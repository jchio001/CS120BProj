/*  [cslogin]_lab2_part3.c - 2/02/15
 *  Name & E-mail:  - Jonathan Chiou (jchio001@ucr.edu)
 *  CS Login: jchio001
 *  Lab Section: 23
 *  Assignment: Project
 *  Exercise Description:
 *  
 *  
 *  I acknowledge all content contained herein, excluding template or example 
 *  code, is my own original work.
 */
   
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
//#include "C:\Users\Jonathan\Documents\Atmel Studio\HeaderFiles\io.c"
#include "C:\Users\student\Documents\Atmel Studio\header files\io.c"
   
volatile unsigned char TimerFlag = 0; // ISR raises, main() lowers
   
// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
   
void TimerOn() {
    // AVR timer/counter controller register TCCR1
    // bit3 = 0: CTC mode (clear timer on compare)
    // bit2bit1bit0=011: pre-scaler /64
    // 00001011: 0x0B
    // SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
    // Thus, TCNT1 register will count at 125,000 ticks/s
    TCCR1B = 0x0B;
    // AVR output compare register OCR1A.
    // Timer interrupt will be generated when TCNT1==OCR1A
    // We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
    // So when TCNT1 register equals 125,
    // 1 ms has passed. Thus, we compare to 125.
    OCR1A = 125;// AVR timer interrupt mask register
    // bit1: OCIE1A -- enables compare match interrupt
    TIMSK1 = 0x02;
    //Initialize avr counter
    TCNT1=0;
    // TimerISR will be called every _avr_timer_cntcurr milliseconds
    _avr_timer_cntcurr = _avr_timer_M;
    //Enable global interrupts: 0x80: 1000000
    SREG |= 0x80;
}
   
void TimerOff() {
    // bit3bit1bit0=000: timer off
    TCCR1B = 0x00;
}

typedef struct _task {
	//Task's current state, period, and the time elapsed
	// since the last tick
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	//Task tick function
	int (*TickFct)(int);
} task;

task tasks[2];

const unsigned char tasksNum = 2;
const unsigned long tasksPeriodGCD = 100;
const unsigned long period1 = 100;
const unsigned long period2 = 100;

void TimerISR() {
    unsigned char i;
	for (i = 0; i < tasksNum; ++i) { // Heart of the scheduler code
		if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
}
   
ISR(TIMER1_COMPA_vect) {
    // CPU automatically calls when TCNT1 == OCR1
    // (every 1 ms per TimerOn settings)
    // Count down to 0 rather than up to TOP (results in a more efficient comparison)
    _avr_timer_cntcurr--;
    if (_avr_timer_cntcurr == 0) {
        // Call the ISR that the user uses
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}
   
void TimerSet(unsigned long M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
   
unsigned char GetBit(unsigned char x, unsigned char k) {
    return ((x & (0x01 << k)) != 0);
}


void set_PWM(double frequency) {
	
	
	// Keeps track of the currently set frequency
	// Will only update the registers when the frequency
	// changes, plays music uninterrupted.
	static double current_frequency;
	if (frequency != current_frequency) {

		if (!frequency) TCCR3B &= 0x08; //stops timer/counter
		else TCCR3B |= 0x03; // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) OCR3A = 0xFFFF;
		
		// prevents OCR3A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) OCR3A = 0x0000;
		
		// set OCR3A based on desired frequency
		else OCR3A = (short)(8000000 / (128 * frequency)) - 1;

		TCNT3 = 0; // resets counter
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	// in Free Running Mode, a new conversion will trigger
	// whenever the previous conversion completes.
}

//custom characters

const uint8_t bop[8] PROGMEM = {
	0x00,
	0x04,
	0x0A,
	0x15,
	0x15,
	0x0A,
	0x04,
	0x00,
};

const uint8_t twist[8] PROGMEM = {
	0x1D,
	0x1A,
	0x15,
	0x1A,
	0x15,
	0x0B,
	0x17,
	0x00,
};

const uint8_t pull[8] PROGMEM = {
	0x00,
	0x18,
	0x1C,
	0x0F,
	0x07,
	0x07,
	0x00,
	0x00,
};

const uint8_t spin[8] PROGMEM = {
	0x00,
	0x0F,
	0x10,
	0x17,
	0x15,
	0x11,
	0x0E,
	0x00,
};

const uint8_t flick[8] PROGMEM = {
	0x18,
	0x0C,
	0x06,
	0x03,
	0x03,
	0x06,
	0x0C,
	0x00,
};

void LCD_DefineChar(uint8_t char_code, const uint8_t *custom_char){
	uint8_t a = (char_code<<3)|0x40;
	uint8_t pcc;
	uint16_t i;	
	for (i=0; i<8; i++){
		pcc=pgm_read_byte(&custom_char[i]);
		LCD_WriteCommand(a++);
		LCD_WriteData(pcc);
	}
}

void set_up_chars() {	
	LCD_DefineChar(0, pull);
	LCD_DefineChar(1, twist);	
	LCD_DefineChar(2, bop);
	LCD_DefineChar(3, spin);
	LCD_DefineChar(4, flick);
}

unsigned char button;
unsigned char score = 0;
unsigned long high_score = 0;
unsigned char curPos = 1; //curPos of the cursor on the menu
unsigned char seed = 25; //used for RNG
unsigned char input_time = 20;
unsigned char whatodo;
unsigned char seqPos = 0; //which sequence position are we on?

//double notes[] = {261.63, 293.66, 329.63, 349.23, 392.00};
unsigned char *seq;
unsigned char tick_cnt = 0;
//Using pseudo-random number generation. How some games create randomness is
//that they have predetermined levels, and the only randomness there is when
//the game is starting.

void writeTwist(unsigned char pos) {
	LCD_Cursor(pos);
	LCD_WriteData('T');
	LCD_Cursor(pos + 1);
	LCD_WriteData('w');
	LCD_Cursor(pos + 2);
	LCD_WriteData('i');
	LCD_Cursor(pos + 3);
	LCD_WriteData('s');
	LCD_Cursor(pos + 4);
	LCD_WriteData('t');
	LCD_Cursor(pos + 5);
	LCD_WriteData(' ');
	LCD_Cursor(pos + 6);
	LCD_WriteData('I');
	LCD_Cursor(pos + 7);
	LCD_WriteData('t');
}

void writePull(unsigned char pos) {
	LCD_Cursor(pos);
	LCD_WriteData('P');
	LCD_Cursor(pos + 1);
	LCD_WriteData('u');
	LCD_Cursor(pos + 2);
	LCD_WriteData('l');
	LCD_Cursor(pos + 3);
	LCD_WriteData('l');
	LCD_Cursor(pos + 4);
	LCD_WriteData(' ');
	LCD_Cursor(pos + 5);
	LCD_WriteData('I');
	LCD_Cursor(pos + 6);
	LCD_WriteData('t');
}

void writeBop(unsigned char pos) {
	LCD_Cursor(pos);
	LCD_WriteData('B');
	LCD_Cursor(pos + 1);
	LCD_WriteData('o');
	LCD_Cursor(pos + 2);
	LCD_WriteData('p');
	LCD_Cursor(pos + 3);
	LCD_WriteData(' ');
	LCD_Cursor(pos + 4);
	LCD_WriteData('I');
	LCD_Cursor(pos + 5);
	LCD_WriteData('t');
}

void writeSpin(unsigned char pos) {
	LCD_Cursor(pos);
	LCD_WriteData('S');
	LCD_Cursor(pos + 1);
	LCD_WriteData('p');
	LCD_Cursor(pos + 2);
	LCD_WriteData('i');
	LCD_Cursor(pos + 3);
	LCD_WriteData('n');
	LCD_Cursor(pos + 4);
	LCD_WriteData(' ');
	LCD_Cursor(pos + 5);
	LCD_WriteData('I');
	LCD_Cursor(pos + 6);
	LCD_WriteData('t');
}

void writeFlick(unsigned char pos) {
	LCD_Cursor(pos);
	LCD_WriteData('F');
	LCD_Cursor(pos + 1);
	LCD_WriteData('l');
	LCD_Cursor(pos + 2);
	LCD_WriteData('i');
	LCD_Cursor(pos + 3);
	LCD_WriteData('c');
	LCD_Cursor(pos + 4);
	LCD_WriteData('k');
	LCD_Cursor(pos + 5);
	LCD_WriteData(' ');
	LCD_Cursor(pos + 6);
	LCD_WriteData('I');
	LCD_Cursor(pos + 7);
	LCD_WriteData('t');
}

void writeStart() {
	LCD_Cursor(2);
	LCD_WriteData('S');
	LCD_Cursor(3);
	LCD_WriteData('t');
	LCD_Cursor(4);
	LCD_WriteData('a');
	LCD_Cursor(5);
	LCD_WriteData('r');
	LCD_Cursor(6);
	LCD_WriteData('t');
}

void output_fnc(int seed) {
	LCD_ClearScreen();
	switch (seed) {
		case 0:
			LCD_Cursor(1);
			LCD_WriteData(0);
			writePull(2);
			break;
		case 1:
			LCD_Cursor(1);
			LCD_WriteData(1);
			writeTwist(2);			
			break;
		case 2:
			LCD_Cursor(1);
			LCD_WriteData(2);
			writeBop(2);
			break;
		case 3:
			LCD_Cursor(1);
			LCD_WriteData(3);
			writeSpin(2);
			break;
		case 4:
			LCD_Cursor(1);
			LCD_WriteData(4);
			writeFlick(2);
			break;
		default:
			break;
	}
	
}

void writeScore(unsigned char pos) {
	LCD_Cursor(pos);
	LCD_WriteData('H');
	LCD_Cursor(pos + 1);
	LCD_WriteData('i');
	LCD_Cursor(pos + 2);
	LCD_WriteData('g');
	LCD_Cursor(pos + 3);
	LCD_WriteData('h');
	LCD_Cursor(pos + 4);
	LCD_WriteData(' ');
	
	LCD_Cursor(pos + 5);
	LCD_WriteData('S');
	LCD_Cursor(pos + 6);
	LCD_WriteData('c');
	LCD_Cursor(pos + 7);
	LCD_WriteData('o');
	LCD_Cursor(pos + 8);
	LCD_WriteData('r');
	LCD_Cursor(pos + 9);
	LCD_WriteData('e');
}

void writeEndScore() {
	LCD_Cursor(17);
	LCD_WriteData('S');
	LCD_Cursor(18);
	LCD_WriteData('c');
	LCD_Cursor(19);
	LCD_WriteData('o');
	LCD_Cursor(20);
	LCD_WriteData('r');
	LCD_Cursor(21);
	LCD_WriteData('e');
	
	unsigned long leftover;
	unsigned char digit0 = (high_score % 10);	
	leftover = score / 10;	
	unsigned char digit1 = (leftover % 10);
	leftover = leftover / 10;
	unsigned char digit2 = (leftover % 10);
		
	LCD_Cursor(12);
	LCD_WriteData(':');
	LCD_Cursor(23);
	LCD_WriteData(' ');
	
	LCD_Cursor(24);	
	LCD_WriteData(digit2 + '0');
	LCD_Cursor(25);
	LCD_WriteData(digit1 + '0');
	LCD_Cursor(26);
	LCD_WriteData(digit0 + '0');
}

void writeHighScore() {
	//assuming we don't have a score greater than 1k
	unsigned long leftover;
	unsigned char digit0 = (high_score % 10);	
	leftover = high_score / 10;	
	unsigned char digit1 = (leftover % 10);
	leftover = leftover / 10;
	unsigned char digit2 = (leftover % 10);
	
	LCD_Cursor(1);
	writeScore(1);
	LCD_Cursor(11);
	LCD_WriteData(':');
	LCD_Cursor(12);
	LCD_WriteData(' ');
	
	LCD_Cursor(13);	
	LCD_WriteData(digit2 + '0');
	LCD_Cursor(14);
	LCD_WriteData(digit1 + '0');
	LCD_Cursor(15);
	LCD_WriteData(digit0 + '0');
	
}

enum States {Start, Disp_Menu, Menu, Wait, Wait2, Wait3, Display, Game_On, Pause, Game_Over, High_Score} state;
int tick(int state) {
	switch (state) {
		case Start:
			state = Disp_Menu;
			break;
		case Disp_Menu:
			state = Menu;			
			break;		
		case Menu:
			if ((button == (0x04)) && (curPos == 1)) {				
				state = Wait;				
			}
			else if ((button == (0x04)) && (curPos == 17)) {				
				state = Wait2;				
			}
			else
				state = Menu;			
			break;
		case Wait:
			if (button == (0x04)) {
				state = Wait;
			}
			else {
				LCD_ClearScreen();				
				state = Display;
			}
			break;
		case Wait2:
			if (button == 0x04)
				state = Wait2;
			else {
				LCD_ClearScreen();
				writeHighScore();
				state = High_Score;
			}
			break;	
		case Display:
			state = Game_On;						
			break;		
		case Game_On:
			++tick_cnt;
			if (tick_cnt <= input_time) {
				if (button == (0x01 << whatodo)) {
					tick_cnt = 0;
					PORTB = 0x00;
					LCD_ClearScreen();
					++score;
					state = Pause;					
				}
				else if (button != 0x00) {
					tick_cnt = 0;
					PORTB = 0x00;
					LCD_ClearScreen();
					LCD_DisplayString(1, "Game Over");
					state = Game_Over;
				}
				else
					state = Game_On;
			}
			else {
				tick_cnt = 0;
				PORTB = 0x00;
				LCD_ClearScreen();
				LCD_DisplayString(1, "Game Over");
				state = Game_Over;
			}
			break;
		case Pause:
			++tick_cnt;
			if (tick_cnt > 10) {
				tick_cnt = 0;
				if ((score > 0) && (score % 10 == 0)) {
					if (input_time > 10)
						--input_time;
				}
				state = Display;
			}
			break;
		case Game_Over:
			++tick_cnt;
			if (tick_cnt > 20) {
				curPos = 1;				
				tick_cnt = 0;								
				LCD_ClearScreen();
				input_time = 20;
				if (score > high_score) {
					high_score = score;
					LCD_ClearScreen();
					writeHighScore();
					state = High_Score;
				}
				else
					state = Disp_Menu;
				
				score = 0;
			}
			break;
		case High_Score:			
			if (button == 0x04)
				state = Wait3;			
			else
				state = High_Score;
			break;
		case Wait3:
			if (button == 0x04)
				state = Wait3;
			else {				
				state = Disp_Menu;
			}
		default:
			state = Start;
			break;					
	}
	
	switch (state) {
		case Disp_Menu:
			LCD_ClearScreen();
			curPos = 1;								
			LCD_Cursor(1);
			writeStart();
			writeScore(18);
			LCD_Cursor(1);
			break;
		case Menu:
			if (button == 0x08) {
				curPos = 1;
				LCD_Cursor(1);				
			}
			else if (button == 0x10) {
				curPos = 17;
				LCD_Cursor(17);
			}
			break;
		case Display:
			whatodo = seed % 5;
			output_fnc(seed % 5);		
		case Game_On:			
			break;					
		default:
			break;		
	}
	return state;
}

enum States1 {Start1, Seed} seed_state;	
int seed_tick(int state) {
	switch (state) {
		case Start1:
		state = Seed;
		break;
		case Seed:
		break;
		default:
		state = Start1;
		break;
	}
	
	switch (state) {
		case Seed:
			++seed;
			if (seed > 50)
				seed = 1;
			break;
		default:
			break;
	}
	
	return state;
	
}

unsigned long int findGCD (unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}

   
int main(void)
{		
	unsigned char i = 0;		
	tasks[i].state = -1;
	tasks[i].period = period1;
	tasks[i].elapsedTime = period1;
	tasks[i].TickFct = &tick;
	
	++i;
	tasks[i].state = -1;
	tasks[i].period = period2;
	tasks[i].elapsedTime = period2;
	tasks[i].TickFct = &seed_tick;
		
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00; // LCD data lines
    DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	set_up_chars();
    LCD_init();
    state = Start;
	ADC_init();
	unsigned short min_value = 250;
	unsigned short x = ADC; // Value of ADC register now stored in variable x.
	unsigned short my_short = 0x0000;
    TimerSet(tasksPeriodGCD);
    TimerOn();	
    while(1)		
    {       
		x = ADC;
		my_short = x;
		button = ~PINA;
		if (my_short > min_value)
			SetBit(button, 0, 1);
		else
			SetBit(button, 0, 0);
		//Sleep();
	}
	
}