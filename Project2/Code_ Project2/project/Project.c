/*===================================================================================================
 Name        : Project2
 Author      : Mina Adel Kadess
 Date        : 12-9-2024
 Description : Stop watch with Dual Mode (count up and Count down) Using ATmega32 and Seven-Segment Display
 ===================================================================================================
 */
#include<avr/io.h>
#include<avr/interrupt.h>
#include <util/delay.h>

unsigned char ButtonState = 0; // flag control the mode
unsigned char Seconds = 0;
unsigned char Minutes = 0;
unsigned char Hours = 0;
unsigned char Flag_seconds_dec_btn;
unsigned char Flag_seconds_inc_btn;
unsigned char Flag_minutes_dec_btn;
unsigned char Flag_minutes_inc_btn;
unsigned char Flag_hours_dec_btn;
unsigned char Flag_hours_inc_btn;
/*======================================================================================================*/
// Function TIMER1
void TIMER1_CTC_INIT() {
	/* F_CPU=16Mhz , Pre-scaler=1024
	 *  F_Timer=15625 HZ ,T(timer)=1\15625
	 *  1 Second = 1000000 us
	 *  for 1 second the compare match value will be 15625
	 */
	/* start timer from 0 */
	TCNT1 = 0;
	/*Force Output Compare for Compare unit A*/
	TCCR1A = (1 << FOC1A);
	/* WGM12 = 1 => using mode 4 in timer1 (CTC) and compare match in OCR1A
	 *(CS10 = 1 && CS12 = 1) => pre-scaler 1024
	 */
	TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS12);
	/*Set compare value to 15625*/
	OCR1A = 15625;

	/* enable output compare A match interrupt */
	TIMSK |= (1 << OCIE1A);

	/* enable global interrupt */
	SREG |= (1 << 7);
}

//ISRTIMER1
ISR(TIMER1_COMPA_vect) {
	/*After Compare interrupt seconds++*/
	if (!ButtonState) {
		/*Buzzer off*/
		PORTD &= ~(1 << PD0);

		//increment Seconds
		Seconds++;
		/*1 minutes = 60 second*/
		if (Seconds == 60) {
			//increment Minutes
			Minutes++;
			Seconds = 0;
		}
		/*1 hour = 60 minute*/
		if (Minutes == 60) {
			//increment Hours
			Hours++;
			Minutes = 0;
		}
		/*	Reached the maximum count of counter*/
		if (Hours == 100) {
			Hours = 0;
			Minutes = 0;
			Seconds = 0;
		}
	}

	else {
		//start decrement from seconds
		if (Seconds == 0) {
			if (Minutes == 0) {
				if (Hours == 0) {
					/*If everything is zero, keep it at zero
					 and the buzzer will turn on*/
					PORTD |= (1 << PD0);
				} else {
					/*Decrement hours and reset minutes and seconds*/
					Hours--;
					Minutes = 59;
					Seconds = 59;
				}
			} else {
				/*Decrement minutes and reset seconds*/
				Minutes--;
				Seconds = 59;
			}
		}

		else {
			/*Decrement seconds*/
			Seconds--;
		}
	}
}
/*======================================================================================================*/
//function INT0
void INT0_Init() {
	/*ISC01->1 ISC00->0 ---> The falling edge of INT0*/
	MCUCR |= (1 << ISC01);
	/* external interrupt request 0 enable */
	GICR |= (1 << INT0);
	/* make PD2 as input */
	DDRD &= ~(1 << PD2);
	/* internal pull-up resistor for PD2 */
	PORTD |= (1 << PD2);
}

//ISR INT1
ISR(INT0_vect) {
	/*Reset the timer*/
	TCNT1 = 0;
	Seconds = 0;
	Minutes = 0;
	Hours = 0;
}
/*======================================================================================================*/
//function INT1
void INT1_Init() {
	/* ISC11-> 1 ISC10-> 1 ---> The rising edge of INT1*/
	MCUCR |= (1 << ISC11) | (1 << ISC10);
	/* external interrupt request 1 enable */
	GICR |= (1 << INT1);
	/* make PD3 as input */
	DDRD &= ~(1 << PD3);
}

//ISR INT1
ISR(INT1_vect) {
	/*Pause Timer 1 by setting the prescaler to 0 (no clock source)*/
	TCCR1B &= ~(1 << CS10) & ~(1 << CS12);

}
/*======================================================================================================*/
//function INT2
void INT2_Init() {
	/* (ISC2 = 0) => falling edge configuration */
	MCUCSR &= ~(1 << ISC2);
	/* external interrupt request 2 enable */
	GICR |= (1 << INT2);
	/* make PB2 as input*/
	DDRB &= ~(1 << PB2);
	/* internal pull-up resistor for PB2 */
	PORTB |= (1 << PB2);
}

//ISR INT2
ISR(INT2_vect) {
	/*Resume Timer 1 with prescaler 1024*/
	TCCR1B |= (1 << CS10) | (1 << CS12);
}
/*======================================================================================================*/
int main() {

	unsigned char Flag = 0;  // button flag

	/*make the PIN 0,1,3,4,5,6 in port B as input*/
	DDRB &= ~(1 << PB0) & ~(1 << PB1) & ~(1 << PB3) & ~(1 << PB2) & ~(1 << PB5)
			& ~(1 << PB6);
	/*Enable internal pull up resistor in pin 0,1,3,4,5,6*/
	PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB3) | (1 << PB4) | (1 << PB5)
			| (1 << PB6);
	/*At the begging the Red led is on because the start is up counter*/
	PORTD |= (1 << PD4);
	/*make PD4 and PD5 and PD0 output pins*/
	DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD0);

	/*make first 4-pins in PORTC as output for 7447 decoder*/
	DDRC |= 0x0F;
	/* put 0 value as initial in first 4-pins in PORTC */
	PORTC &= 0xF0;
	/* define first 6-pins in PORTA as output as the enable/disable pins for the six 7-segments */

	DDRA |= 0x3F;   //used it because used multiplexed 7 segment
	/* put 0 value as initial in first 6-pins in PORTA  (1100 0000)*/
	PORTA &= 0xC0;

	TIMER1_CTC_INIT();
	INT0_Init();
	INT1_Init();
	INT2_Init();

	/*make PB7 input pin*/
	DDRB &= ~(1 << PB7);
	PORTB |= (1 << PB7);

	while (1) {
		/*================================================================================================*/
		// Check if the button state has changed from not pressed to pressed
		if (!(PINB & (1 << PB7))) // check if the push button at PB7 is pressed or not
		{

			if (Flag == 0) {
				//if the push button at PB7 is pressed convert from count up mode to count down mode
				ButtonState = !ButtonState;
				/*Resume Timer 1 with prescaler 1024*/
//					TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS12);
				if (ButtonState)
				{
					/*turn off the count up led*/
					PORTD &= ~(1 << PD4);
					/*turn on the count down led*/
					PORTD |= (1 << PD5);
				}
				else
				{
					/*Red LED for count up is on*/
					PORTD |= (1 << PD4);
					/*Yellow LED for down count is off*/
					PORTD &= ~(1 << PD5);
				}
				//set the button flag value to 1 to not enter here again until the button is released.
				Flag = 1;
			}

		}
		else
		{
			// button is released reset the button flag to value 0 again.
			Flag = 0;
		}

		/*===============================================================================================*/

		/*Check the decrement button for seconds ====================================================*/
		if (!(PINB & (1 << PB5))) {
			if (Flag_seconds_dec_btn == 0 && Seconds > 0) {
				Seconds--;
				Flag_seconds_dec_btn = 1;
			}
		} else {
			Flag_seconds_dec_btn = 0;
		}

		/*Check the increment button for seconds ====================================================*/
		if (!(PINB & (1 << PB6))) {
			if (Flag_seconds_inc_btn == 0) {
				Seconds++;
				Flag_seconds_inc_btn = 1;
			}
		} else {
			Flag_seconds_inc_btn = 0;
		}

		/*Check the decrement button for minutes====================================================*/
		if (!(PINB & (1 << PB3))) {
			if (Flag_minutes_dec_btn == 0 && Minutes > 0) {
				Minutes--;
				Flag_minutes_dec_btn = 1;
			}
		} else {
			Flag_minutes_dec_btn = 0;
		}

		/*Check the increment button for Minutes ====================================================*/
		if (!(PINB & (1 << PB4))) {
			if (Flag_minutes_inc_btn == 0) {
				Minutes++;
				Flag_minutes_inc_btn = 1;
			}
		} else {
			Flag_minutes_inc_btn = 0;
		}
		/*Check the decrement button for hours ====================================================*/
		if (!(PINB & (1 << PB0))) {
			if (Flag_hours_dec_btn == 0 && Hours > 0) {
				Hours--;
				Flag_hours_dec_btn = 1;
			}
		} else {
			Flag_hours_dec_btn = 0;
		}
		/*Check the increment button for hours ====================================================*/
		if (!(PINB & (1 << PB1))) {
			if (Flag_hours_inc_btn == 0) {
				Hours++;
				Flag_hours_inc_btn = 1;
			}
		} else {
			Flag_hours_inc_btn = 0;
		}

		/*===============================================================================================*/
		/*First 7-Segment*/
		/* enable first 7-segment for first digit in seconds */
		PORTA = 0x20;  //(0010 0000)
		/* put the value of first digit in seconds in first 4-pins in PORTC */
		/* Used ( % ) to  enable  first digit in seconds */
		PORTC = (PORTC & 0xF0) | ((Seconds % 10) & 0x0F);
		/* small delay the show the number multiplexing method */
		_delay_ms(2);

		/*================================================================================================*/
		/*Second 7-segment*/
		/*enable second 7-segment for second digit in seconds*/
		PORTA = 0x10;   //(0001 0000)
		/* put the value of second digit in seconds in first 4-pins in PORTC */
		/* Used ( / ) to  enable  second  digit in seconds */
		PORTC = (PORTC & 0xF0) | ((Seconds / 10) & 0x0F);
		/* small delay the show the number multiplexing method */
		_delay_ms(2);

		/*================================================================================================*/
		/*Third 7-Segment*/
		/*enable third 7-segment for first digit in minutes*/
		PORTA = 0x08;     //(0000 1000)
		/*put the value of the first digit in minutes in first 4-pins in PORTC*/
		/* Used ( % ) to  enable  first digit in seconds */
		PORTC = (PORTC & 0xF0) | ((Minutes % 10) & 0x0F);
		/* small delay the show the number multiplexing method */
		_delay_ms(2);

		/*================================================================================================*/
		/*Fourth 7-segment*/
		/*enable fourth 7-segment for second digit in minutes*/
		PORTA = 0x04;       //(0000 0100)
		/*put the value of the second digit in minutes in first 4-pins in PORTC*/
		/* Used ( / ) to  enable  second  digit in seconds */
		PORTC = (PORTC & 0xF0) | ((Minutes / 10) & 0x0F);
		/* small delay the show the number multiplexing method */
		_delay_ms(2);

		/*=================================================================================================*/
		/*Fifth 7-segment*/
		/*enable Fifth 7-segment for first digit in hours*/
		PORTA = 0x02;        //(0000 0010)
		/*put the value of the first digit in hours in first 4-pins in PORTC*/
		/* Used ( % ) to  enable  first digit in seconds */
		PORTC = (PORTC & 0xF0) | ((Hours % 10) & 0x0F);
		/* small delay the show the number multiplexing method */
		_delay_ms(2);

		/*=================================================================================================*/
		/*Sixth 7-segment*/
		/*enable sixth 7-segment for second digit in hours*/
		PORTA = 0x01;      //(0000 0001)
		/*put the value of the second digit in minutes in first 4-pins in PORTC*/
		/* Used ( / ) to  enable  second  digit in seconds */
		PORTC = (PORTC & 0xF0) | ((Hours / 10) & 0x0F);
		/* small delay the show the number multiplexing method */
		_delay_ms(2);

	}
}
