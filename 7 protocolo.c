#include <18f4520.h>
#device adc = 10
#fuses XT, PUT, NODEBUG , NOBROWNOUT , NOPROTECT , NOLVP
#use delay(clock=4000000)
#include <ds1307.c>
#use rs232(baud=9600, bits = 8 , parity = N ,xmit=PIN_C6,rcv=PIN_C7)
//#use I2c(master,sda=pin_c4,scl=pin_c3)
#use delay(clock=4000000)

#include <LCD.C>
#USE FAST_IO(B)
#USE FAST_IO(C)
#USE FAST_IO(D)


//--------------Protocolo
char v[8];

int cont=0, op=0,con=0,flag=0;
int ver,hor=0,min=0,seg=0,dia=0,mes=0,ano=0,diasem=0;
int meses[12]={31,28,31,30,31,30,31,31,30,31,30,31};
int act, oc;//Protocolo de Cadencia
//--------------Protocolo

boolean timer=false;

int bandera=0,c;
double medicion=0.0, Proof=2.5;
double temperatura=0, temperatura2=0, resolucion1=.48828, resolucion2=.48828, div1=1, div2=1, resolucion3=.48828, div3=1,off1=0,off2=0,off3=0,off=-2;
int posicion=0, valorElegido2=0, valorElegido3=0;
int mask=0xF0, nivel=0, ok=0, valorElegido=0, banVenti=0, cancel=0, banMAs=0, x=1, banMenos=0, posicion2=0, lim1=34, lim2=44, lim3=49, lim4=100;
int sensorElegido=1;
int contadorLec=0, Lect=0;
void mostrar(int);
void sensor1(int);
void sensor2(int);
void sensorLuz(int);
void ventilador(int);
void resolucion(int);
void cadencia(int);
void offset(int);
void ventiladorConfig(int);
void configurarResolucion(int, int);
void Offs(int, double);
void mostrarNivel1(int);
void mostrarNivel2(int);
void mostrarNivel3(int);
void mostrarNivel4(int);

#INT_TIMER0
void TIMER0_isr(){
   set_timer0(0x00);
   flag++;
   if(flag==7){
      timer=true;
      flag=0;
      }
      contadorLec++;
   if(contadorLec==3){
         contadorLec=0;
         Lect=1;
   }
}

#int_rda
void serial_isr(){
   while(kbhit()){
      v[cont]=getc();
      cont++;
      if(cont==8)
         cont=0;
   }
   op=1;
}

#INT_RB
void isr_intrb(){
act=input_b();
       if((act& 0xE0)==0xE0)// si es 11100000
          oc=1; //Sera suma
       if((act& 0xD0)==0xD0)// si es 11010000
          oc=2; //Sera resta
       if((act& 0xB0)==0xB0)// si es 10110000
          oc=3; //Sera multiplicacion
       if((act& 0x70)==0x70)// si es 01110000
          oc=4; //Sera division
       if((act& 0xF0)==0xF0)
          oc=0;
}

void main(){
setup_adc(ADC_CLOCK_INTERNAL);
   setup_adc_ports(AN0_TO_AN1);
   enable_interrupts(int_rda);
   setup_timer_0(rtcc_internal|rtcc_div_2);
   enable_interrupts(int_timer0);
   enable_interrupts(int_rb);
   setup_CCP1(ccp_pwm);
   setup_timer_2(T2_DIV_BY_4,249,1);
   enable_interrupts(GLOBAL);


   i2c_stop();
   i2c_start();
   i2c_write(0xD0);      // WR to RTC
   i2c_write(0x00);      // REG 0
   i2c_start();
   i2c_write(0xD1);
   ver=i2c_read();
   i2c_stop();

   set_tris_b(0xF0);
   set_tris_c(0xF0);
   set_tris_d(0x00);
   set_tris_a(0x00);

   lcd_init();


   ver&=0x80;
   if(ver==0x80)
      setTime(0,0,0,0,0,0,0);
   while(true){
         getDate(dia,mes,ano,diasem); //Siempre obtenemos los valores del DS1307
         getTime(hor,min,seg);

         lcd_gotoxy(1,1);
         printf(lcd_putc,"Lunes:%1.0f",(double)hor);
         lcd_gotoxy(9,1);
         printf(lcd_putc,":%1.0f",(double)min);
         lcd_gotoxy(12,1);
         printf(lcd_putc,":%1.0f",(double)seg);
         //----------------
         lcd_gotoxy(1,2);
         printf(lcd_putc,"%1.0f",(double)dia);
         lcd_gotoxy(5,2);
         printf(lcd_putc,"/%1.0f",(double)mes);
         lcd_gotoxy(8,2);
         printf(lcd_putc,"/%1.0f",(double)ano);

         switch(oc) {
      case 1:
            dia++;
            if(dia<31){
                     dia=1;
            }

      break;
      case 2:
               mes++;
                  if(mes<=12){

                     printf("Se ha cambiado la hora por el valor Deseado\r");
                  }
                  if(mes>12){
                  mes=01;

                  }


      break;
      case 3:
               hor++;
                  if(hor<=24){

                     printf("Se ha cambiado la hora por el valor Deseado\r");
                  }
                  if(hor>24){
                  hor=01;

                  }
      break;
   case 4:
            min++;
                  if(min<=60){

                     printf("Se ha cambiado la hora por el valor Deseado\r");
                  }
                  if(min>60){
                  min=01;

                  }
      break;
         }
          setTime(dia,mes,ano,diasem,hor,min,seg);
   }
}

