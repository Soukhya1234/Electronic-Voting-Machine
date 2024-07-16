#ifndef F_CPU
#define F_CPU 1000000 // defining the crystal frequency attached
#endif

#include <avr/io.h> // header to enable data flow control over pins
#include <util/delay.h> // header to enable delay function in program
#include <string.h> // header for string functions
#include <stdlib.h> // header for itoa function
#include <stdio.h> // header for snprintf function

#define E 5 // define enable pin
#define RS 6 // define register selection pin

void send_a_command(unsigned char command);
void send_a_character(unsigned char character);
void send_a_string(const char *string);
void update_display(int16_t COUNTA, int16_t COUNTB, int16_t COUNTC, int16_t COUNTD);
void display_winner(int16_t COUNTA, int16_t COUNTB, int16_t COUNTC, int16_t COUNTD);
void buzz(); // Function to activate the buzzer
void indicate_delay(uint8_t state); // Function to indicate the delay period

int main(void) {
	DDRA = 0xFF; // set PORTA as output for LCD
	DDRD = 0xFF; // set PORTD as output for LCD control
	DDRB = 0x00; // set PORTB as input for buttons
	PORTB = 0xFF; // Enable internal pull-up resistors on PORTB
	DDRC = 0xFF; // Set PORTC as output for LEDs
	DDRB |= (1 << PB7); // Set PB7 as output for buzzer

	PORTB &= ~(1 << PB7); // Ensure buzzer is off initially
	PORTC &= ~(1 << PC0); // Ensure delay indication LED is off initially

	_delay_ms(50); // delay for 50 ms

	int16_t COUNTA = 0; // person1 votes storing memory
	int16_t COUNTB = 0; // person2 votes storing memory
	int16_t COUNTC = 0; // person3 votes storing memory
	int16_t COUNTD = 0; // person4 votes storing memory
	uint8_t reset_display = 0; // flag for reset display

	send_a_command(0x01); // Clear Screen 0x01 = 00000001
	_delay_ms(50);
	send_a_command(0x38); // 8bit mode
	_delay_ms(50);
	send_a_command(0b00001111); // LCD ON, cursor ON

	// Display welcome message
	send_a_command(0x80); // First line
	send_a_string("Welcome!");
	_delay_ms(3000); // display for 3 seconds

	send_a_command(0x01); // Clear Screen

	// Display A, B, C, D in the desired format
	send_a_command(0x80); // First line
	send_a_string("A        B");
	send_a_command(0xC0); // Second line
	send_a_string("C        D");

	uint8_t delay_active = 0; // flag for delay

	while (1) {
		if (!delay_active) { // check if delay is not active
			if (bit_is_clear(PINB, 0)) { // button 1 pressed
				COUNTA += 1;
				buzz(); // Activate buzzer
				indicate_delay(1); // Turn on delay indicator LED
				delay_active = 1; // activate delay
			}
			if (bit_is_clear(PINB, 1)) { // button 2 pressed
				COUNTB += 1;
				buzz(); // Activate buzzer
				indicate_delay(1); // Turn on delay indicator LED
				delay_active = 1; // activate delay
			}
			if (bit_is_clear(PINB, 2)) { // button 3 pressed
				COUNTC += 1;
				buzz(); // Activate buzzer
				indicate_delay(1); // Turn on delay indicator LED
				delay_active = 1; // activate delay
			}
			if (bit_is_clear(PINB, 3)) { // button 4 pressed
				COUNTD += 1;
				buzz(); // Activate buzzer
				indicate_delay(1); // Turn on delay indicator LED
				delay_active = 1; // activate delay
			}
			if (bit_is_clear(PINB, 4)) { // reset button pressed
				if (reset_display == 0) { // Display winner
					display_winner(COUNTA, COUNTB, COUNTC, COUNTD);
					reset_display = 1;
					} else if (reset_display == 1) { // Display counts
					update_display(COUNTA, COUNTB, COUNTC, COUNTD);
					reset_display = 2;
					} else { // Clear counts
					COUNTA = COUNTB = COUNTC = COUNTD = 0; // reset counts
					update_display(COUNTA, COUNTB, COUNTC, COUNTD);
					reset_display = 0;
				}
				buzz(); // Activate buzzer
				indicate_delay(1); // Turn on delay indicator LED
				delay_active = 1; // activate delay
			}

			if (delay_active) {
				_delay_ms(5000); // 5 second delay
				indicate_delay(0); // Turn off delay indicator LED
				delay_active = 0; // deactivate delay
			}
		}
	}
}

void send_a_command(unsigned char command) {
	PORTA = command;
	PORTD &= ~(1 << RS); // RS = 0, command mode
	PORTD |= (1 << E); // E = 1, enable
	_delay_ms(1);
	PORTD &= ~(1 << E); // E = 0, disable
	_delay_ms(1);
	PORTA = 0;
}

void send_a_character(unsigned char character) {
	PORTA = character;
	PORTD |= (1 << RS); // RS = 1, data mode
	PORTD |= (1 << E); // E = 1, enable
	_delay_ms(1);
	PORTD &= ~(1 << E); // E = 0, disable
	_delay_ms(1);
	PORTA = 0;
}

void send_a_string(const char *string) {
	while (*string) {
		send_a_character(*string++);
	}
}

void update_display(int16_t COUNTA, int16_t COUNTB, int16_t COUNTC, int16_t COUNTD) {
	char SHOWA[16], SHOWB[16], SHOWC[16], SHOWD[16];

	send_a_command(0x80); // First line
	send_a_string("A=");
	itoa(COUNTA, SHOWA, 10);
	send_a_string(SHOWA);
	send_a_string("     B=");
	itoa(COUNTB, SHOWB, 10);
	send_a_string(SHOWB);

	send_a_command(0xC0); // Second line
	send_a_string("C=");
	itoa(COUNTC, SHOWC, 10);
	send_a_string(SHOWC);
	send_a_string("     D=");
	itoa(COUNTD, SHOWD, 10);
	send_a_string(SHOWD);
}

void display_winner(int16_t COUNTA, int16_t COUNTB, int16_t COUNTC, int16_t COUNTD) {
	char winner_message[16];
	int16_t max_votes = COUNTA;
	char winner = 'A';

	if (COUNTB > max_votes) {
		max_votes = COUNTB;
		winner = 'B';
	}
	if (COUNTC > max_votes) {
		max_votes = COUNTC;
		winner = 'C';
	}
	if (COUNTD > max_votes) {
		max_votes = COUNTD;
		winner = 'D';
	}

	snprintf(winner_message, 16, "Winner: %c", winner);
	send_a_command(0x01); // Clear Screen
	send_a_command(0x80); // First line
	send_a_string(winner_message);
}

void buzz() {
	PORTB |= (1 << PB7); // Turn on buzzer
	_delay_ms(100); // Buzz for 100 ms
	PORTB &= ~(1 << PB7); // Turn off buzzer
}

void indicate_delay(uint8_t state) {
	if (state) {
		PORTC |= (1 << PC0); // Turn on delay indicator LED
		} else {
		PORTC &= ~(1 << PC0); // Turn off delay indicator LED
	}
}