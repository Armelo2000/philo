/*
 * Testat_RUM_SoSe_2023_v2.cpp
 *
 * Created: 20.08.2023 20:42:29
 * Author : uCRemote10
 */ 

#ifndef F_CPU
#define F_CPU 16000000LU
#endif
#include "Basics.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "OSKernel.h"
#include "BinarySemaphor.h"
#include "ADConverter.h"
#include "LCD.h"
#include "Timer.h"
#include "DigiPort.h"

#define button_0 (0b00000001)
#define button_1 (0b00000010)
#define button_2 (0b00000100)
#define button_3 (0b00001000)
#define button_4 (0b00010000)
#define N (5)
#define LEFT (i+N-1)%N
#define RIGHT (i+1)%N


//Objekte
void run_philo_task(uint8_t i);
static ADConverter adc(AD_CHAN_0);
static uint8_t rv, waitTime;
static DelayHandler dh (TC1, use_ms, multi_tasking);
static uint8_t phil_1id, phil_2id, phil_3id, phil_4id, phil_5id, lcd_id, taste_id;
static DigiPortIRPT keys (PK, SET_IN_PORT, SET_ACTIVE_LOW);
static LCD display(PC, LCD_Type_40x4, DISPLAY_ON | CURSOR_OFF | BLINK_OFF | WRAPPING_ON );
static DigiPortRaw leds (PA, SET_OUT_PORT);
static volatile bool ist_button_0 = false;
static volatile bool ist_button_1 = false;
static volatile bool ist_button_2 = false;
static volatile bool ist_button_3 = false;
static volatile bool ist_button_4 = false;
static volatile uint8_t a=0, b=0, c=0, d=0, e=0;


typedef struct chopstick {
	uint8_t index;
	BinarySemaphor semaphor;
	} chopstick_t;
	
typedef enum _eState{
	OFF = 0,
	HUNGRIG,
	THINKING,
	WARTET,
	EATING
}eState;

typedef struct t_Position{
	uint8_t line;
	uint8_t col;
	uint8_t size;

}position_t;
	
typedef struct Philo{
	uint8_t id;
	uint8_t waitTime;
	eState state;
	chopstick_t* leftChopStick;
	chopstick_t* rightChopStick;
	volatile bool voher_gegessen;	
	volatile uint8_t essen_zahl;
	
	position_t lcd_position_philo;
	position_t lcd_position_state;
	position_t lcd_position_count;
	
}philo_t;

philo_t Phil_1, Phil_2, Phil_3, Phil_4, Phil_5;
philo_t* Phil[5]= {&Phil_1, &Phil_2, &Phil_3, &Phil_4, &Phil_5};

chopstick_t gabel_1, gabel_2, gabel_3, gabel_4, gabel_5;
chopstick_t* stick[5] = {&gabel_1, &gabel_2, &gabel_3, &gabel_4, &gabel_5};
	
static void get_random_seed() {
		rv = 0;
		rv |= (0x01 & adc.get_value());
		rv |= (0x01 & adc.get_value()) << 1;
		rv |= (0x01 & adc.get_value()) << 2;
		rv |= (0x01 & adc.get_value()) << 3;
	}

static uint8_t random()
	{
		#define mask (0x0f)
		rv ^= rv << 4;
		rv ^= rv >> 3;
		rv ^= rv << 1;
		return 1+ (mask & rv);
	}

void Init_Philosopher(){
	
	uint8_t i;
	for(i=0; i<5; i++){
		Phil[i]->id = i+1;
		Phil[i]->state = THINKING;
		Phil[i]->leftChopStick = stick[i];
		Phil[i]->rightChopStick = stick[(i+1)%5];
		Phil[i]->voher_gegessen = false;
		Phil[i]->essen_zahl =0;
		Phil[i]->waitTime=random();

		Phil[i]->lcd_position_philo.line = 0;
		Phil[i]->lcd_position_philo.col = (i+1)*7;
		Phil[i]->lcd_position_philo.size = 2;
		
		Phil[i]->lcd_position_state.line = 1;
		Phil[i]->lcd_position_state.col = (i+1)*7;
		Phil[i]->lcd_position_state.size = 5;
		
		Phil[i]->lcd_position_count.line = 2;
		Phil[i]->lcd_position_count.col = (i+1)*7;
		Phil[i]->lcd_position_count.size = 5;
		
	}
	
}

/*void initChopSticks(){
	uint8_t i;
	for(i=0; i<5; i++){
		 stick[i]->index = i+1;
	}
}*/

void test(uint8_t i);
void schon_gegessen(void);
void phil_1_task(void);
void phil_2_task(void);
void phil_3_task(void);
void phil_4_task(void);
void phil_5_task(void);
void lcd_task (void);
void Taste_task(void);

//static uint8_t demo_Id;
void demoTask(void){
	
	static uint8_t count = 0;
	
	while(1){
		CRITICAL_SECTION { 
			if(count > 99) count = 0;
			display.set_pos(0, 30);
			display.write_number(count++);
		
		}
		yield();
	}
}

int main(void)
{
	get_random_seed();
	
	Init_Philosopher();
	leds.off();
	phil_1id = task_insert ( phil_1_task);
	phil_2id = task_insert ( phil_2_task);
	phil_3id = task_insert ( phil_3_task);
	phil_4id = task_insert ( phil_4_task);
	phil_5id = task_insert ( phil_5_task);
	taste_id = task_insert (Taste_task);
	lcd_id = task_insert (lcd_task);
	
	
	
   kernel(Simple);
	
}



void phil_1_task(void)
{
	while(1){
		if (a==1)
		{
			if (Phil[0]->leftChopStick->semaphor.release()== false)
			{
				Phil[0]->leftChopStick->semaphor.release();
			}
			
			if (Phil[0]->rightChopStick->semaphor.release()== false)
			{
				Phil[0]->rightChopStick->semaphor.release();
			}
			
			Phil[0]->state=OFF;
			deactivate(phil_1id);
		}
		
		else if (a==2)
		{
			a= 0;
			activate(phil_1id);
		}
		
		else {
			run_philo_task(3);
			schon_gegessen();
		}	
		
		yield();
	}
}
	
void phil_2_task(void)
{
	while(1){
		if (b==1)
		{
			if (Phil[1]->leftChopStick->semaphor.release()== false)
			{
				Phil[1]->leftChopStick->semaphor.release();
			}
			
			if (Phil[1]->rightChopStick->semaphor.release()== false)
			{
				Phil[1]->rightChopStick->semaphor.release();
			}
			
			Phil[1]->state=OFF;
			deactivate(phil_2id);
		}
		
		else if (b==2)
		{
			b= 0;
			activate(phil_2id);
		}
		
	   else {
		 run_philo_task(1);
		 schon_gegessen();
	 }
		
		yield();
	}	
}

void phil_3_task(void)
{
	while(1){
		if (c==1)
		{
			if (Phil[2]->leftChopStick->semaphor.release()== false)
			{
				Phil[2]->leftChopStick->semaphor.release();
			}
			
			if (Phil[2]->rightChopStick->semaphor.release()== false)
			{
				Phil[2]->rightChopStick->semaphor.release();
			}
			
			Phil[2]->state=OFF;
			deactivate(phil_3id);
		}
		
		else if (c==2)
		{
			c= 0;
			activate(phil_3id);
		}
		
	  else {
		  run_philo_task(2);
		  schon_gegessen();
	  }
		
		yield();
	}
}

void phil_4_task(void)
{
	while(1){
		if (d==1)
		{
			if (Phil[3]->leftChopStick->semaphor.release()== false)
			{
				Phil[3]->leftChopStick->semaphor.release();
			}
			
			if (Phil[3]->rightChopStick->semaphor.release()== false)
			{
				Phil[3]->rightChopStick->semaphor.release();
			}
			
			Phil[3]->state=OFF;
			deactivate(phil_4id);
		}
		
		else if (d==2)
		{
			d= 0;
			activate(phil_4id);
		}
		
		else {
			run_philo_task(3);
			schon_gegessen();
		}
		
		yield();
	}
}

void phil_5_task(void)
{
	while(1){
		if (e==1)
		{
			if (Phil[4]->leftChopStick->semaphor.release()== false)
			{
				Phil[4]->leftChopStick->semaphor.release();
			}
			
			if (Phil[4]->rightChopStick->semaphor.release()== false)
			{
				Phil[4]->rightChopStick->semaphor.release();
			}
			
			Phil[4]->state=OFF;
			deactivate(phil_5id);
		}
		
		else if (e==2)
		{
			e= 0;
			activate(phil_5id);
		}
		
		else {
			run_philo_task(3);
			schon_gegessen();
		}
		
		yield();
	}
}
	
void test(uint8_t i){
	Phil[i]->waitTime=random();
	if(Phil[i]->state == HUNGRIG && Phil[LEFT]->state != EATING && Phil[RIGHT]->state != EATING && Phil[i]->voher_gegessen== false){
		Phil[i]->leftChopStick->semaphor.aquire();
		if (Phil[i]->leftChopStick->semaphor.aquire()){
			Phil[i]->rightChopStick->semaphor.aquire();
			if (Phil[i]->rightChopStick->semaphor.aquire()){
			Phil[i]->state= EATING;
			Phil[i]->essen_zahl++;
			if(Phil[i]->essen_zahl==1000){
				Phil[i]->essen_zahl=0;
			}
			Phil[i]->voher_gegessen=1;
			dh.wait(Phil[i]->waitTime*100);
			Phil[i]->rightChopStick->semaphor.release();
			Phil[i]->leftChopStick->semaphor.release();
			Phil[i]->state=THINKING;
			
			}
			else Phil[i]->state=WARTET;
		}
		else Phil[i]->state=WARTET;
		
	}
	else Phil[i]->state=WARTET;
}	
void run_philo_task(uint8_t i){
	Phil[i]->waitTime=random();
	Phil[i]->state=THINKING;
	dh.wait(waitTime*250);
	Phil[i]->state= HUNGRIG;
	test(i);
}
void schon_gegessen(void){
	if(Phil[0]->voher_gegessen==1 && Phil[1]->voher_gegessen==1 && Phil[2]->voher_gegessen==1 && Phil[3]->voher_gegessen==1 && Phil[4]->voher_gegessen==1){
		for(uint8_t k=0;k<5;k++){Phil[k]->voher_gegessen=false;}
	}
}

void Taste_task(void) {
	
	while (1){
		
		switch(keys.read_raw()) {
			
			case button_0:
			
			ist_button_0 = true;
			a++;
			leds.on(button_0);
			break;
			
			case button_1:
			
			ist_button_1 = true;
			b++;
			leds.on(button_1);
			break;
			
			case button_2:
			
			ist_button_2 = true;
			c++;
			leds.on(button_2);
			break;
			
			case button_3:
			
			ist_button_3 = true;
			d++;
			leds.on(button_3);
			break;
			
			case button_4:
			
			ist_button_4 = true;
			e++;
			leds.on(button_4);
			break;
			
			default:
			break;
			
		}
		yield();
	
	}	
}

void lcd_task (void) {
	
	uint8_t g=0;
	
	while(1){
		CRITICAL_SECTION { 
		
		for(uint8_t i=0; i<5; i++){
			if(i==5){
				if(Phil[i]->state==THINKING){
					
					display.set_pos(i-1,23);
					display.write_SRAM_text(" Ph.");
					display.write_number(i+1,1);
					display.write_SRAM_text(": think ");
					display.write_number(Phil[i]->essen_zahl,3,'0');
					
				}
				
				if(Phil[i]->state==HUNGRIG){
					
					display.set_pos(i-1,23);
					display.write_SRAM_text(" Ph.");
					display.write_number(i+1,1);
					display.write_SRAM_text(": hungrig ");
					display.write_number(Phil[i]->essen_zahl,3,'0');
					
				}
				
				if(Phil[i]->state==WARTET){
					
					display.set_pos(i-1,23);
					display.write_SRAM_text(" Ph.");
					display.write_number(i+1,1);
					display.write_SRAM_text(": wartet ");
					display.write_number(Phil[i]->essen_zahl,3,'0');
					
				}
				if(Phil[i]->state==EATING){
					display.set_pos(i-1,23);
					display.write_SRAM_text(" Ph.");
					display.write_number(i+1,1);
					display.write_SRAM_text(": eat ");
					display.write_number(Phil[i]->essen_zahl,3,'0');
				}
				if(Phil[i]->state==OFF){
					display.set_pos(i-1,23);
					display.write_SRAM_text(" Ph.");
					display.write_number(i+1,1);
					display.write_SRAM_text(": off ");
					display.write_number(Phil[i]->essen_zahl,3,'0');
				}
				
				g++;
				
				
			}
			
		else	{if(Phil[i]->state==THINKING){
				
				display.set_pos(i,i-g);
				display.write_SRAM_text("Ph.");
				display.write_number(i+1,1);
				display.write_SRAM_text(": think ");
				display.write_number(Phil[i]->essen_zahl,3,'0');
				
			}
			
			if(Phil[i]->state==HUNGRIG){
				
				display.set_pos(i,i-g);
				display.write_SRAM_text("Ph.");
				display.write_number(i+1,1);
				display.write_SRAM_text(": hungrig ");
				display.write_number(Phil[i]->essen_zahl,3,'0');
				
			}
			
			if(Phil[i]->state==WARTET){
				
				display.set_pos(i,i-g);
				display.write_SRAM_text("Ph.");
				display.write_number(i+1,1);
				display.write_SRAM_text(": wartet ");
				display.write_number(Phil[i]->essen_zahl,3,'0');
				
			}
			if(Phil[i]->state==EATING){
				display.set_pos(i,i-g);
				display.write_SRAM_text("Ph.");
				display.write_number(i+1,1);
				display.write_SRAM_text(": eat ");
				display.write_number(Phil[i]->essen_zahl,3,'0');
				}
			if(Phil[i]->state==OFF){
				display.set_pos(i,i-g);
				display.write_SRAM_text("Ph.");
				display.write_number(i+1,1);
				display.write_SRAM_text(": off ");
				display.write_number(Phil[i]->essen_zahl,3,'0');
				}
			
			g++;
		}
		
		}
		//yield();
	}
	yield();
 }
	
	
}


/*******************************************************************
 ************************** LCD Display ****************************
 *
 *  Philo:	P1		P2		P3		P4		P5
 *	Do:		OFF		OFF		OFF		OFF		OFF
 *	Count:	0		0		0		0		0
 *
 *******************************************************************/
void initDisplay(){
	uint8_t xPos = 0;
	uint8_t yPos = 0;
	const uint8_t space = 2;
	
	display.set_pos(0, 0);
	display.write_SRAM_text("Philo:");
	display.set_pos(1, 0);
	display.write_SRAM_text("DO:");
	display.set_pos(2, 0);
	display.write_SRAM_text("Time:");
	
	yPos += 7; //Size of "Philo:"
	yPos += space;
	//Save the following position for the philos:
	/*
	 * P1 		-> xPos = 0, yPos = 7
	 * OFF		-> xPos = 1, yPos = 7
	 * 0		-> xPos = 2, yPos = 7
	 **********************************
	 * P2 		-> xPos = 0, yPos = 14
	 * OFF		-> xPos = 1, yPos = 14
	 * 0		-> xPos = 2, yPos = 14
	 **********************************	
	 * P3 		-> xPos = 0, yPos = 21
	 * OFF		-> xPos = 1, yPos = 21
	 * 0		-> xPos = 2, yPos = 21
	 **********************************
	 * P4 		-> xPos = 0, yPos = 28
	 * OFF		-> xPos = 1, yPos = 28
	 * 0		-> xPos = 2, yPos = 28
	 **********************************
	 * P1 		-> xPos = 0, yPos = 35
	 * OFF		-> xPos = 1, yPos = 35
	 * 0		-> xPos = 2, yPos = 35
	 *********************************
	*/
	
	// Philosoph 1
	display.set_pos(0, 7);
	display.write_SRAM_text("P1");
	display.set_pos(1, 7);
	display.write_SRAM_text("OFF");
	display.set_pos(2, 7);
	display.write_number(0);
	
	// Philosoph 2
	display.set_pos(0, 14);
	display.write_SRAM_text("P2");
	display.set_pos(1, 14);
	display.write_SRAM_text("OFF");
	display.set_pos(2, 14);
	display.write_number(0);
	
	// Philosoph 3
	display.set_pos(0, 21);
	display.write_SRAM_text("P3");
	display.set_pos(1, 21);
	display.write_SRAM_text("OFF");
	display.set_pos(2, 21);
	display.write_number(0);

	// Philosoph 4
	display.set_pos(0, 28);
	display.write_SRAM_text("P4");
	display.set_pos(1, 28);
	display.write_SRAM_text("OFF");
	display.set_pos(2, 28);
	display.write_number(0);
	
	// Philosoph 5
	display.set_pos(0, 35);
	display.write_SRAM_text("P5");
	display.set_pos(1, 35);
	display.write_SRAM_text("OFF");
	display.set_pos(2, 35);
	display.write_number(0);	
}
