/**
 * khsie003_amalu001_lab7_part5.c
 * Kevin Hsieh, khsie003@ucr.edu
 * Alyza Malunao, amalu001@ucr.edu
 * Section 23
 * Using both rows of the LCD display, design a game where a player controlled character
 * avoids oncoming obstacles. Three buttons are used to operate the game. 
 * Author: student
 * I acknowledge all content contained herein, excluding template or example code,
 * is my own original work.
 **/


#include <avr/io.h>
#include <avr/interrupt.h>
// #include <bit.h>
#include <stdio.h>
#include "header/timer.h"
#include "header/scheduler.h"
#include "header/io.c"
#include "header/usart_ATmega1284.h"

typedef int bool;

#define true 1
#define false 0

#define UP_UPPER 	0x57//'W'
#define UP_LOWER	0x77//'w'
#define LEFT_UPPER 	0x41//'A'
#define LEFT_LOWER 	0x61//'a'
#define DOWN_UPPER 	0x53//'S'
#define DOWN_LOWER 	0x73//'s'
#define RIGHT_UPPER	0x44//'D'
#define RIGHT_LOWER	0x64//'d'
#define START		0x20//'space'
//column select 
#define LEFT        0x80
#define UP 			0x20
#define DOWN 		0x08
#define RIGHT 		0x02
#define ZERO		0x00

/* //notes
	#define C4      261.63
	#define D4      293.66
	#define E4      329.63 
	#define F4      349.23 
	#define G4      392.00 
	#define A4      440.00 
	#define B4      493.88 
	#define C5      523.25 
*/

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

/*** Shared variables ***/
unsigned char input_char = 0x00;
unsigned long score = 0;
unsigned char pause = 0;
bool game = false;
//LED matrix output
// static unsigned char row_sel = 0x81; // sets the pattern displayed on columns
static unsigned char column_sel = 0x55; // grounds column to display pattern
//position of current leds
// unsigned char shift = 0x00;
/***/

//"SONGS"
char* collection[4] = {" MENU", " Saika", " Wing of Piano", " Utoiosphere"};

//song patterns
const unsigned char Saika_keys[] = {ZERO, ZERO, ZERO, LEFT, LEFT, RIGHT, RIGHT, UP, DOWN, UP, DOWN, RIGHT, LEFT, UP, RIGHT, ZERO};
// const unsigned char Saika_notes[] = {};
const unsigned int SONG_LENGTH = 15;
unsigned int n = 0;

unsigned char LEFTNotes		= 0x00;
unsigned char UPNotes 		= 0x00;
unsigned char DOWNNotes 	= 0x00;
unsigned char RIGHTNotes	= 0x00;
static unsigned char curNotes = 0x00;

unsigned long addScore(unsigned long score, unsigned char note){
	if(input_char == UP_LOWER || input_char == UP_UPPER){
		if(note == UP){
			score = score + 10;
		}
	}
	if(input_char == LEFT_LOWER || input_char == LEFT_UPPER){
		if(note == LEFT){
			score = score + 10;
		}
	}
	if(input_char == DOWN_LOWER || input_char == DOWN_UPPER){
		if(note == DOWN){
			score = score + 10;
		}
	}
	if(input_char == RIGHT_LOWER || input_char == RIGHT_UPPER){
		if(note == RIGHT){
			score = score + 10;
		}
	}
	return score;
}

//matrix display
enum SM1_States {sm1_wait, sm1_display1, sm1_display2, sm1_display3, sm1_display4};
int SM1_Tick(int state) { //matrix display
	// === Local Variables ===
	// shift = row_sel & 0xFE;
	curNotes = 0x00;
	// === Transitions ===
	switch (state) { // cycle through each column //transition
		case sm1_wait: 
			if(game) state = sm1_display1;
			else state = sm1_wait;
		break;
		case sm1_display1: 
			if(!game)
				state = sm1_wait;
			else
				state = sm1_display2; 
		break;
		case sm1_display2: 
			if(!game)
				state = sm1_wait;
			else
				state = sm1_display3; 
			break;
		case sm1_display3: 
			if(!game)
				state = sm1_wait;
			else
				state = sm1_display4; 
			break;
		case sm1_display4: 
			if(!game)
				state = sm1_wait;
			else
				state = sm1_display1; 
			break;
		default: state = sm1_wait; break;
	}

	// === Actions ===
	switch (state) { //action
		case sm1_display1:
			column_sel = 0x7F;
			if(game)
				curNotes = LEFTNotes;
			break;
		case sm1_display2:
			column_sel = 0xDF;
			if(game)
				curNotes = UPNotes;
			break;
		case sm1_display3:
			column_sel = 0xF7;
			if(game)
				curNotes = DOWNNotes;
			break;
		case sm1_display4:
			column_sel = 0xFD;
			if(game)
				curNotes = RIGHTNotes;
			break; 
		default: break;
	}
	score = addScore(score,curNotes);
	curNotes = curNotes | 0x01;
	PORTA = curNotes; // PORTA displays column pattern
	PORTC = column_sel; // PORTC selects column to display pattern
	return state;
};



static unsigned short display_line= 1;
static unsigned int   arrow_pos = 1;
//get input from laptop keyboard
enum SM2_States {sm2_title, sm2_menu, sm2_game, sm2_gg};
int SM2_Tick(int state){

	static bool change = false;
	input_char = 0x00;
	if(USART_HasReceived(0))
		input_char = USART_Receive(0);
	if(USART_IsSendReady(0))
		USART_Send(input_char,0);

	switch(state){
		case sm2_title:
			LCD_DisplayString(4," 8BIT DDR      Press Start");
			LCD_Cursor(3);
			LCD_WriteData(0);
			LCD_Cursor(14);
			LCD_WriteData(0);
			// LCD_Cursor(0);
			LCD_Cursor(33);
				
			if(input_char == START)
				state = sm2_menu;
			else
				state = sm2_title;
		break;
		case sm2_menu:
			if(input_char == DOWN_UPPER || input_char == DOWN_LOWER){
				if(arrow_pos == 1)
					arrow_pos = 17;
				else
					display_line = (display_line < 3) ? display_line+1 : display_line;
				change = true;
			}
			if (input_char == UP_UPPER || input_char == UP_LOWER){
				if(arrow_pos == 17 && (display_line == 3 || display_line == 1))
					arrow_pos = 1;
				else
					display_line = (display_line > 1) ? display_line-1 : display_line;
				change = true;
			}
			if(change){
				LCD_ClearScreen();
				LCD_DisplayString(2,collection[display_line-1]);
				LCD_Cursor(16);
				LCD_WriteData(5);
				LCD_DisplayString(18,collection[display_line]);
				LCD_Cursor(32);
				LCD_WriteData(6);
				LCD_Cursor(arrow_pos);
				LCD_WriteData(7);
				LCD_Cursor(33);
			}
			if(input_char == START && (display_line != 1 || arrow_pos != 1)){
				game = true;	
				state = sm2_game;
				change = false;
			}
			else{
				state = sm2_menu;
				change = false;
			}
			input_char = 0x00;
		break;
		case sm2_game:
			if(n < SONG_LENGTH){
				LCD_ClearScreen();

				//display score
				//display character
				state = sm2_game;
			}
			else
				state = sm2_gg;
		break;
		case sm2_gg:
			n = 0;
			game = false;
			LCD_ClearScreen();
			state = sm2_menu;
		break;
		default: 
			state = sm2_title; 
		break;
	}
	// PORTB = LED;
	return state;
}


const short oneFourthSec = 5;
static unsigned short i = 0;
enum SM3_States {sm3_count};
int SM3_Tick(int state){
	switch(state){
		case sm3_count:
			if(i < oneFourthSec){
				i++;
			}
			else{
				n = (n < SONG_LENGTH) ? n+1 : n;

				LEFTNotes = LEFTNotes >> 1;
				UPNotes = UPNotes >> 1;
				DOWNNotes = DOWNNotes >> 1;
				RIGHTNotes = RIGHTNotes >> 1;

				if(Saika_keys[n] == LEFT){
					LEFTNotes = LEFTNotes | 0x80;
				}else if(Saika_keys[n] == UP){
					UPNotes = UPNotes | 0x80;
				}else if(Saika_keys[n] == DOWN){
					DOWNNotes = DOWNNotes | 0x80;
				}else if(Saika_keys[n] == RIGHT){
					RIGHTNotes = RIGHTNotes | 0x80;
				}
				i = 0;
			}

			break;
		default: state = sm3_count; break;
	}

	return state;
}

int main(void)
{
	/*** Set PORTs to correct I/O ***/
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xF0; PORTD = 0x0F;
	/***/


	/*** PERIODs of SMs ***/
	unsigned long int SM1_Tick_calc = 1; //LED Matrix columns
	unsigned long int SM2_Tick_calc = 50; //keyboard input
	unsigned long int SM3_Tick_calc = 100; //LED Matrix rows
	// unsigned long int SM4_Tick_calc = 50;

	/***/

	/*** GCD Calcualtion ***/
	unsigned long int tmpGCD  = 1;
	tmpGCD = findGCD(SM1_Tick_calc, SM1_Tick_calc);
	tmpGCD = findGCD(tmpGCD, SM2_Tick_calc);
	tmpGCD = findGCD(tmpGCD, SM3_Tick_calc);
	// tmpGCD = findGCD(tmpGCD, SM4_Tick_calc);
	/***/

	/*** GCD for all tasks or smallest time unit for tasks ***/
	unsigned long GCD = tmpGCD;
	/***/

	/*** Recalculate GCD periods for scheduler ***/
	unsigned long int SM1_Tick_period = SM1_Tick_calc / GCD;
	unsigned long int SM2_Tick_period = SM2_Tick_calc / GCD;
	unsigned long int SM3_Tick_period = SM3_Tick_calc / GCD;
	// unsigned long int SM4_Tick_period = SM4_Tick_calc / GCD;
	/***/

	/*** List of tasks ***/
	static task task1, task2, task3;//, task4;
	task *tasks[] = {&task1, &task2, &task3};//, &task4};
	const unsigned short numTasks = sizeof(tasks) / sizeof(task*);
	/***/

	// task 1:
	task1.state = -1;
	task1.period = SM1_Tick_period;
	task1.elapsedTime = SM1_Tick_period;
	task1.TickFct = & SM1_Tick;

	// task 2:
	task2.state = -1;
	task2.period = SM2_Tick_period;
	task2.elapsedTime = SM2_Tick_period;
	task2.TickFct = & SM2_Tick;

	//task 3:
	task3.state = -1;
	task3.period = SM3_Tick_period;
	task3.elapsedTime = SM3_Tick_period;
	task3.TickFct = & SM3_Tick;

	//task 4:
	// task4.state = -1;
	// task4.period = SM4_Tick_period;
	// task4.elapsedTime = SM4_Tick_period;
	// task4.TickFct = & SMTick4;

	//set timer and turn it on
	// TimerSet(250);
	TimerSet(GCD);
	TimerOn();

	LCD_init();
	LCD_specialChar();
	// LCD_ClearScreen();
	

	initUSART(0);
	USART_Flush(0);

	// unsigned curState = 1;
	// unsigned curState = sm1_display;
	unsigned short i; //Scheduler for-loop iterator
	while(1) {
		//Scheduler:
		for(i = 0; i < numTasks; i++){
			//Task is ready to tick
			if( tasks[i]->elapsedTime == tasks[i]->period){
				//setting next state for task
				tasks[i]->state=tasks[i]->TickFct(tasks[i]->state);
				//Reset elapsedTime for next Tick
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		// SM1_Tick(curState);
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}

