/*****************************************************************************************************************
 * Editors: Saleem Griggs-Taylor, Kevin Figurski, Tennison Hoffmann
 * Start Date: 11/11/2021
 * Last modified: 11/11/2021
 * Description: This program is developed to interface an Infrared LED and Reciever with the MSP432
 * to detect a signal, and also to detect when the signal is disrupted.
 *****************************************************************************************************************/

#include "msp.h"
#include <stdio.h>
#include <stdlib.h>

void TA2_N_IRQHandler(void);
void Timer_Init(void);
uint16_t period;
double hertz;
volatile int flag = 0;
/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	P2->SEL0 |= BIT7;               //IR LED
	P2->SEL1 &= ~BIT7;              //timerA0.4 controls this pin
	P2->DIR |= BIT7;

    P6->SEL0 |= BIT7;               //Reciever
    P6->SEL1 &= ~BIT7;              //timerA2.4 controls this pin
	P6->DIR &= ~BIT7;
	P6->REN |= BIT7;
	P6->OUT |= BIT7;

	P1->SEL0 &= ~BIT0;
	P1->SEL1 &= ~BIT0;
	P1->DIR |= BIT0;


	Timer_Init();

    NVIC_EnableIRQ(TA2_N_IRQn);
    __enable_interrupts();

    while(1)
    {
        if (flag == 1){
            P1->OUT ^= BIT0;        //toggle onboard LED
            flag = 0;
        }
    }
}

void TA2_N_IRQHandler(){
    static uint16_t read_1 = 0, read_2 = 0;
    if(TIMER_A2->CCTL[4] & BIT0) // was TA0.4 the interrupt event?
    {
        TIMER_A2->CCTL[4] &= ~BIT0;  //clear interrupt flag;
        read_1 = read_2;
        read_2 = TIMER_A2->CCR[4];
        period = read_2 - read_1;
        hertz = (3000000/(double)period) / 8;
        if ((hertz > 5) & (hertz < 15)){
            flag = 1;
        }
    }
}

void Timer_Init(void){              //Initializes the timer
    TIMER_A0->CTL = 0b1011010100;                      //Count up using smclk, clears TAOR register, /8
    TIMER_A0->CCR[0] = 37500 - 1;                      //10 Hz
    TIMER_A0->CCR[4] = 18750 - 1;                      //50% duty cycle
    TIMER_A0->CCTL[4] = 0b11100000;                    //reset/set mode

    TIMER_A2->CTL = 0b1011100100;  //SMCLK, /8, Continuous Mode, Clear count, no interrupt
    TIMER_A2->CCTL[4] = 0b0100100100010000;  //Capture on rising edge, synchronous, interrupt, CCIxA
}
