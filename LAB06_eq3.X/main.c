////+++++++++++++++++++++++++++++++++++++| LIBRARIES / HEADERS |+++++++++++++++++++++++++++++++++++++
#include "device_config.h"
#include <stdint.h>
#include <math.h>

//+++++++++++++++++++++++++++++++++++++| DIRECTIVES |+++++++++++++++++++++++++++++++++++++
#define _XTAL_FREQ 8000000
#define SWEEP_FREQ 300
#define LCD_DATA_R          PORTD
#define LCD_DATA_W          LATD
#define LCD_DATA_DIR        TRISD
#define LCD_RS              LATCbits.LATC2
#define LCD_RS_DIR          TRISCbits.TRISC2
#define LCD_RW              LATCbits.LATC1
#define LCD_RW_DIR          TRISCbits.TRISC1
#define LCD_E               LATCbits.LATC0
#define LCD_E_DIR           TRISCbits.TRISC0
 char counter ;
 char carry ;
 char rem ;
 int result ;
 int array[3] ;
 int address;
//+++++++++++++++++++++++++++++++++++++| DATA TYPES |+++++++++++++++++++++++++++++++++++++
enum por_ACDC {digital, analog};          // digital = 0, analog = 1

////+++++++++++++++++++++++++++++++++++++| ISRs |+++++++++++++++++++++++++++++++++++++
//// ISR for high priority
//void __interrupt ( high_priority ) high_isr( void );
//// ISR for low priority
//void __interrupt ( low_priority ) low_isr( void ); 

//+++++++++++++++++++++++++++++++++++++| FUNCTION DECLARATIONS |+++++++++++++++++++++++++++++++++++++
void portsInit(void);
char key_scanner(void);
void screen_disp(char key);
void LCD_rdy(void);
void LCD_init(void);
void LCD_cmd(char cx);
void send2LCD(char xy, char ad);

//+++++++++++++++++++++++++++++++++++++| MAIN |+++++++++++++++++++++++++++++++++++++
void main(void){
    portsInit();
    while(1){
      LCD_init ();
      counter = 0 ;
      carry = 0 ;
      rem = 0 ;
      result = 0 ;
      array[3] = {0,0,0} ;
      address = 0x8F;
      while (1){
        char key = key_scanner();
        if (key != 'x'){
            screen_disp(key);
        }
        if (key == 60){break;}
      }
    } 
}

//+++++++++++++++++++++++++++++++++++++| FUNCTIONS |+++++++++++++++++++++++++++++++++++++
void portsInit(void){
    ANSELA = digital;   // Set port A as Digital for keypad driving
    TRISA  = 0xF0;      // For Port A, set pins 4 to 7 as inputs (columns), and pins 0 to 3 as outputs (rows)
    ANSELC = digital;   // Set port B as Digital for 7 segment cathode selection (only 4 pins used)  
    TRISC  = 0x00;      // For Port B, set pins as outputs for cathode selection
    ANSELD = digital;   // Set port D as Digital for 7 segment anodes
    TRISD  = 0x00;      // for Port D, set all pins as outputs for 7 segment anodes
    OSCCON = 0x74;      // Set the internal oscillator to 8MHz and stable
}


char key_scanner(void){
    LATAbits.LA0 = 0;
    LATAbits.LA1 = 1;
    LATAbits.LA2 = 1;
    LATAbits.LA3 = 1;
    __delay_ms(SWEEP_FREQ);
    if      (PORTAbits.RA4 == 0) {__delay_ms(SWEEP_FREQ); return 1;}
    else if (PORTAbits.RA5 == 0) {__delay_ms(SWEEP_FREQ); return 2;}
    else if (PORTAbits.RA6 == 0) {__delay_ms(SWEEP_FREQ); return 3;}
    else if (PORTAbits.RA7 == 0) {__delay_ms(SWEEP_FREQ); return 43;}
    LATAbits.LA0 = 1;
    LATAbits.LA1 = 0;
    LATAbits.LA2 = 1;
    LATAbits.LA3 = 1;
    __delay_ms(SWEEP_FREQ);
    if      (PORTAbits.RA4 == 0) {__delay_ms(SWEEP_FREQ); return 4;}
    else if (PORTAbits.RA5 == 0) {__delay_ms(SWEEP_FREQ); return 5;}
    else if (PORTAbits.RA6 == 0) {__delay_ms(SWEEP_FREQ); return 6;}
    else if (PORTAbits.RA7 == 0) {__delay_ms(SWEEP_FREQ); return 45;}
    LATAbits.LA0 = 1;
    LATAbits.LA1 = 1;
    LATAbits.LA2 = 0;
    LATAbits.LA3 = 1;
    __delay_ms(SWEEP_FREQ);
    if      (PORTAbits.RA4 == 0) {__delay_ms(SWEEP_FREQ); return 7;}
    else if (PORTAbits.RA5 == 0) {__delay_ms(SWEEP_FREQ); return 8;}
    else if (PORTAbits.RA6 == 0) {__delay_ms(SWEEP_FREQ); return 9;}
    else if (PORTAbits.RA7 == 0) {__delay_ms(SWEEP_FREQ); return 42;}
    LATAbits.LA0 = 1;
    LATAbits.LA1 = 1;
    LATAbits.LA2 = 1;
    LATAbits.LA3 = 0;
    __delay_ms(SWEEP_FREQ);
    if      (PORTAbits.RA4 == 0) {__delay_ms(SWEEP_FREQ); return 61;}
    else if (PORTAbits.RA5 == 0) {__delay_ms(SWEEP_FREQ); return 0;}
    else if (PORTAbits.RA6 == 0) {__delay_ms(SWEEP_FREQ); return 60;}
    else if (PORTAbits.RA7 == 0) {__delay_ms(SWEEP_FREQ); return 47;}   
    else return 'x';
}
// Function to wait until the LCD is not busy
void LCD_rdy(void){
    char test;
    // configure LCD data bus for input
    LCD_DATA_DIR = 0b11111111;
    test = 0x80;
    while(test){
        LCD_RS = 0;         // select IR register
        LCD_RW = 1;         // set READ mode
        LCD_E  = 1;         // setup to clock data
        test = LCD_DATA_R;
        Nop();
        LCD_E = 0;          // complete the READ cycle
        test &= 0x80;       // check BUSY FLAG 
    }
}
// LCD initialization function
void LCD_init(void){
    LATC = 0;               // Make sure LCD control port is low
    LCD_E_DIR = 0;          // Set Enable as output
    LCD_RS_DIR = 0;         // Set RS as output 
    LCD_RW_DIR = 0;         // Set R/W as output
    LCD_cmd(0x38);          // Display to 2x40
    LCD_cmd(0x0F);          // Display on, cursor on and blinking
    LCD_cmd(0x01);          // Clear display and move cursor home
}
// Send command to the LCD
void LCD_cmd(char cx) {
    //LCD_rdy();              // wait until LCD is ready
    LCD_RS = 0;             // select IR register
    LCD_RW = 0;             // set WRITE mode
    LCD_E  = 1;             // set to clock data
    Nop();
    LCD_DATA_W = cx;        // send out command
    Nop();                  // No operation (small delay to lengthen E pulse)
    LCD_E = 0;              // complete external write cycle
}
// Function to display data on the LCD
void send2LCD(char xy, char ad){
    LCD_cmd (ad);
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_E  = 1;
    LCD_DATA_W = xy;
    Nop();
    Nop();
    LCD_E  = 0;
}
    
void screen_disp(char key){
 
    array[counter]=key;
    counter = counter+1;
    if (key<50){
        key = key+48;
    }
   
    send2LCD(key, address);
    address= address-1;
        if(key==61) {
           if (array[1] == 43){
                result = array[0]+array[2];
                carry = result /10;
                rem = result%10;
                
                send2LCD(carry, 0xC1) ;
                send2LCD (rem, 0xC0) ;
           }
           if (array[1] == 45){
                result = array[0]-array[2];
                carry = result /10;
                rem = result%10;
                send2LCD(carry, 0xC1) ;
                send2LCD (rem, 0xC0) ;
           }
           if (array[1] == 42){
                result = array[0]*array[2];
                carry = result /10;
                rem = result%10;
                send2LCD(carry, 0xC1) ;
                send2LCD (rem, 0xC0) ;
           }
           if (array[1] == 47){
                result = array[0]/array[2];
                carry = result /10;
                rem = result%10;
                send2LCD(carry, 0xC1) ;
                send2LCD (rem, 0xC0) ;
           }
      
       
    }


}
