#include <stdlib.h>
#include "dorobo32.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "digital.h"
#include "adc.h"
#include "motor.h"
#include "stm32f0xx_hal.h"
#include "mxconstants.h"
#include "fft.h"

//Defining motors
#define RIGHT   0
#define LEFT    1
#define BACK    2

//Global variable for motor speeed
int setspeed=0;
int flag=0;

//Defining microswitch
uint32_t leftswitch =0;
uint32_t rightswitch=0;
uint32_t leftdist= 0;
uint32_t rightdist= 0;

//IR
uint32_t IRLeft, IRRight = 0;

//Function declaration
void bot();
void Switchtest();
void Distance();
static void IR(void *pvParameters);
//static void botIR(void *pvParameters);

void gobackward();
void goleft();
void goright();
void goforward();
void stop();
void gofwdleft();
void gofwdright();
void gobwdright();
void gobwdleft();




int main()
{
  //microswitch pin configuration
  digital_configure_pin(DD_PIN_PC8, DD_CFG_INPUT_PULLUP);
  digital_configure_pin(DD_PIN_PD15, DD_CFG_INPUT_PULLUP);
  //IR detector pin configuration
  digital_configure_pin(DD_PIN_PF10, DD_CFG_INPUT_PULLUP);
  digital_configure_pin(DD_PIN_PF9, DD_CFG_INPUT_PULLUP);

  //Initialising
    dorobo_init();			//Call dorobo_init() function to initialize HAL, Clocks, Timers etc.	
    adc_init();
    motor_init();

    //xTaskCreate(dorobo, "dorobo1", 512, NULL, 2, NULL); // main task for robo implementation
	  //xTaskCreate(Switchtest, "Switchtest1", 256, NULL, 2, NULL);	//create micro switch task
    //xTaskCreate(Distance, "Distance1", 256, NULL, 2, NULL); //create distance sensor task
      xTaskCreate(IR, "IRdetect", 256, NULL, 2, NULL);
    //xTaskCreate(motor, "motordrive", 256, NULL, 2, NULL);
   // xTaskCreate(bot, "bot11", 512, NULL, 2, NULL);
    //xTaskCreate(botIR, "bot12", 256, NULL, 2, NULL);

   vTaskStartScheduler();	//start the freertos scheduler

	return 0;				//should not be reached!
}



// Function definition for Microswitch
void Switchtest()
{


  while(1)
  {

    //set speed for motor rotation
      setspeed=40;

      //get the microswitch pin satatus
      leftswitch= digital_get_pin(DD_PIN_PD15);
      rightswitch= digital_get_pin(DD_PIN_PC8);
      tracef("switchleft = %d", leftswitch);
      tracef("switchright = %d", rightswitch);

      // Behavior for motor based on the microswitch condition

      if(leftswitch==0 && rightswitch == 1)
      {

        gobackward();
        delay(500);
        goleft();
        delay(1250);
      }
    else if(leftswitch==1 && rightswitch == 0)
      {

        gobackward();
        delay(1000);
        goright();
        delay(1250);

      }
    else if(leftswitch==0 && rightswitch == 0)
      {
        gobackward();
        delay(500);
        gofwdright();
        delay(2000);
     }
    else
     {
        goforward();

     }


}
}



// Function definition for Distance Sensor
void Distance()
{

  while(1)
  {
    setspeed = 40;
    leftdist= adc_get_value(DA_ADC_CHANNEL2);
    rightdist= adc_get_value(DA_ADC_CHANNEL3);

  //tracef("DistanceRight : %d",rightdist);
  //delay(500);
 //tracef("DistanceLeft : %d",leftdist);
 //vTaskDelay(40);
if(leftdist>=750 && rightdist<1400)
{
        tracef("Left detected!!  DistanceLeft : %d",leftdist);

        goright();
        delay(1250);

}
else if (rightdist>=1400 && leftdist<750)
{
        tracef("Right detected!!!  DistanceRight : %d",rightdist);

        goleft();
        delay(1250);


}

else if (rightdist>=750 && leftdist>=1400)
        {
                tracef("DistanceRight : %d",rightdist);
                tracef("DistanceLeft : %d",leftdist);
                tracef("Object in front");

                gobackward();
                delay(1500);
                goleft();
}
else
  goforward();
        //tracef("Distance2 : %d",rightdist);


        vTaskDelay(40);        //delay the task for 20 ticks (1 ticks = 50 ms)*/
  }
}

// Function definition for IR detector

static void IR(void *pvParameters)
{

while(1)
{
bool leftsamp, rightsamp = 0;
setspeed =30;

ft_start_sampling(DD_PIN_PF10);
delay(100);
rightsamp = ft_is_sampling_finished();


if(rightsamp)
{
  IRRight = ft_get_transform(DFT_FREQ100);
  delay(150);
  tracef("distanceRight= %ld", IRRight);
}

ft_start_sampling(DD_PIN_PF9);
delay(100);
leftsamp = ft_is_sampling_finished();

if(leftsamp)
{
  IRLeft = ft_get_transform(DFT_FREQ100);
  delay(150);
  tracef("distanceLeft= %ld", IRLeft);
}


bot();

if(IRLeft>1000 && IRRight==0 && (flag!=1))
{
  goleft();
  delay(1000);
  tracef("Left IR detected");
}

else if(IRRight>1000 && IRLeft==0  && (flag!=2))
{
  goright();
  delay(1000);
  tracef("Right IR detected");
}
else
{
  goforward();
   delay(1000);
   tracef("BOTH IR detected");
}


}

}

void goforward()
        {
            motor_set( RIGHT,setspeed);
            motor_set(LEFT,-setspeed);
            motor_set(BACK,0);
        }

void gobackward()
        {
              motor_set(RIGHT,-setspeed);
              motor_set(LEFT,setspeed);
              motor_set(BACK,0);
        }

void goleft()
     {
           motor_set(RIGHT,setspeed);
           motor_set(LEFT,setspeed);
           motor_set(BACK,0);

     }

void goright()
  {
          motor_set(RIGHT,-setspeed);
          motor_set(LEFT,-setspeed);
          motor_set(BACK,0);

  }

void gofwdleft()
  {
          motor_set(RIGHT,setspeed);
          motor_set(LEFT,0);
          motor_set(BACK,-setspeed);

  }

 void gofwdright()
  {
          motor_set(RIGHT,0);
          motor_set(LEFT,-setspeed);
          motor_set(BACK,setspeed);

  }

 void gobwdright()
  {
          motor_set(RIGHT,-setspeed);
          motor_set(LEFT,0);
          motor_set(BACK,setspeed);

  }

 void gobwdleft()
  {
          motor_set(RIGHT,0);
          motor_set(LEFT,setspeed);
          motor_set(BACK,-setspeed);

  }
 void stop()
   {
           motor_set(RIGHT,10);
           motor_set(LEFT,10);
           motor_set(BACK,10);

   }

void bot()
 {


  // while(1)
  // {
     setspeed = 30;
         leftdist= adc_get_value(DA_ADC_CHANNEL2);
         rightdist= adc_get_value(DA_ADC_CHANNEL3);
     leftswitch= digital_get_pin(DD_PIN_PD15);
          rightswitch= digital_get_pin(DD_PIN_PC8);

          if((leftdist>=1900 && rightdist<900) || (leftswitch==0 && rightswitch == 1))
          {


            tracef("Left detected!!  DistanceLeft : %d",leftdist);

                  gobackward();
                                           delay(500);
                                           gofwdright();
                  delay(1200);
                  //gofwdleft();
                  //delay(500);
                  tracef("switchleft = %d", leftswitch);
                        tracef("switchright = %d", rightswitch);
                        tracef("l1");
                        flag=1;



          }
          else if ((rightdist>=900 && leftdist<1900) || (leftswitch==1 && rightswitch == 0))
          {

                  tracef("Right detected!!!  DistanceRight : %d",rightdist);

                  gobackward();
                                           delay(500);
                                           gofwdleft();
                  delay(500);
                  //gofwdright();
                                    //delay(500);
                  tracef("switchleft = %d", leftswitch);
                        tracef("switchright = %d", rightswitch);
                        tracef("l2");
                        flag=2;



          }

          else if ((rightdist>=900 && leftdist>=1900)||(leftswitch==0 && rightswitch == 0))
                  {

                          tracef("DistanceRight : %d",rightdist);
                          tracef("DistanceLeft : %d",leftdist);
                          tracef("Object in front");

                          gobackward();
                          delay(3000);
                          goleft();
                          delay(500);
                          tracef("switchleft = %d", leftswitch);
                                tracef("switchright = %d", rightswitch);
                                tracef("l3");
                                flag=3;

          }
          else

            goforward();
          flag=0;

          tracef("switchleft = %d", leftswitch);
                tracef("switchright = %d", rightswitch);
                tracef("l4");

                  //tracef("Distance2 : %d",rightdist);

 //}
 }

 /*static void botIR(void *pvParameters)
  {


    while(1)
    {
      bool leftsamp, rightsamp = 0;
      setspeed =30;

      ft_start_sampling(DD_PIN_PF10);
      delay(100);
      rightsamp = ft_is_sampling_finished();


      if(rightsamp)
      {
        IRRight = ft_get_transform(DFT_FREQ100);
        delay(150);
        tracef("distanceRight= %ld", IRRight);
      }

      ft_start_sampling(DD_PIN_PF9);
      delay(100);
      leftsamp = ft_is_sampling_finished();

      if(leftsamp)
      {
        IRLeft = ft_get_transform(DFT_FREQ100);
        delay(150);
        tracef("distanceLeft= %ld", IRLeft);
      }



      if((IRRight>1000 && IRLeft==0) && (rightdist<900))
      {
        gofwdright();
        delay(1000);
        tracef("Right IR detected");
      }
      else if((IRLeft>1000 && IRRight==0) && (leftdist<1900))
    {
      gofwdleft();
      delay(1000);

    }
      else if((IRLeft>1000 && IRRight>1000) && (rightdist>=900 && leftdist>=1900))
      {
        gobackward();
        delay(3000);
        gofwdleft();
        delay(1000);
      }
      else

        goforward();

  }
  }
*/
