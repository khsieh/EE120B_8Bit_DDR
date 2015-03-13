#include <avr/io.h>
#include <avr/interrupt.h>
// #include <bit.h>
#include <stdio.h>
#include "header/timer.h"
#include "header/scheduler.h"
#include "header/io.c"
#include "header/usart_ATmega1284.h"

/*** SET/GET Bit***/
//x is the word to be modified, k-th bit to modify, b is the new bit
unsigned char SetBit(unsigned char x, unsigned char k, unsigned b){
	return(b?x|(0x01<<k):x&~(0x01<<k) );
}

//x is the word to access, k-th bit to get
unsigned char GetBit(unsigned char x, unsigned char k){
	return((x&(0x01<<k))!=0);
}
/***/


enum SM1_States {sm1_display};
int SM1_Tick(int curState){
	unsigned char LED = PORTB;
	switch(curState){
		case sm1_display:

			break;
		default:
			curState = sm1_display;
			break;
	}

	switch(curState){
		case sm1_display:
			if(USART_HasReceived(0)){
				LED = USART_Receive(0);
			}
			break;
		default:
			break;
	}
	PORTB = LED;
	return curState;
}

int main(void)
{
	/*** Set PORTs to correct I/O ***/
	DDRA = 0xFF; PORTA = 0x00;
	// PORTB set to output, outputs init 0s
	DDRB = 0xFF; PORTB = 0x00;
	// PC7..4 outputs init 0s, PC3..0 inputs init 1s
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0x00; PORTD = 0xFF;
	/***/

	//set timer and turn it on
	// TimerSet(250);
	// TimerSet(GCD);
	// TimerOn();
	initUSART(0);

	unsigned curState = sm1_display;
	while(1) {
		SM1_Tick(curState);
	// 	while(!TimerFlag);
	// 	TimerFlag = 0;
	}
	return 0;
}