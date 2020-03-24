#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xscutimer.h"
#include "sleep.h"
#include "xscugic.h"

static void timer1_interrupt_handler(void *CallBackRef);

int InterruptCounter= 0;
	/**
	 * The XScuTimer driver instance data. The user is required to allocate a
	 * variable of this type for every timer device in the system.
	 * A pointer to a variable of this type is then passed to the driver API
	 * functions.
	 */
XScuTimer Timer1;

int main()
{
	int Status;
	int Timer1_Value;

	/**
	 * This typedef contains configuration information for the device.
	 */

	XScuTimer_Config * Timer1_config;

	XScuGic Gic1;
	XScuGic_Config *Gic1_Config;

	/**
	* Lookup the device configuration based on the unique device ID. The table
	* contains the configuration info for each device in the system.
	**/

	Timer1_config = XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID);

	/**
	* CfgInitialize a specific interrupt controller instance/driver
	**/
	Status = XScuTimer_CfgInitialize(&Timer1, Timer1_config, Timer1_config->BaseAddr);
	/** Write to the timer load register. This will also update the
	* timer counter register with the new value. This macro can be used to
	* change the time-out value.
	**/

	//XScuTimer_LoadTimer(&Timer1,1000000000);

	Gic1_Config = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	Status = XScuGic_CfgInitialize(&Gic1, Gic1_Config, Gic1_Config->CpuBaseAddress);

	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, &Gic1);

	Status = XScuGic_Connect(&Gic1, XPAR_SCUTIMER_INTR, (Xil_ExceptionHandler)timer1_interrupt_handler, (void *) &Timer1);

	XScuGic_Enable(&Gic1, XPAR_SCUTIMER_INTR);

	XScuTimer_EnableInterrupt(&Timer1);

	Xil_ExceptionEnable();

	// Load the timer with a value that represents one second of real time
	// HINT: The SCU Timer is clocked at half the frequency of the CPU.
	XScuTimer_LoadTimer(&Timer1, XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2);

	XScuTimer_EnableAutoReload(&Timer1);

	// Start The Timer
	XScuTimer_Start(&Timer1);

    init_platform();

    printf("Timer Demo Application...\n\r");

    while(1)
    {
    	usleep_A9(500);
    	Timer1_Value = XScuTimer_GetCounterValue(&Timer1);
    	if(Timer1_Value <= XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ / 4)
    	{
    		printf("Timervalue: %d\n\r",Timer1_Value);
    		printf("Timer1 value reached...\n\r");
    		Timer1_Value =-1;
    	}
    	else
    	{
    		// For debugging you can uncomment the following line ;-)
    		//printf("Timervalue: %d\n\r",Timer1_Value);
    	}
    }

    Xil_ExceptionDisable();
    XScuGic_Disconnect(&Gic1, XPAR_SCUTIMER_INTR);
    cleanup_platform();
    return 0;
}
static void timer1_interrupt_handler(void *CallBackRef)
{
	XScuTimer *my_Timer_LOCAL = (XScuTimer *) CallBackRef;

	if (XScuTimer_IsExpired(&Timer1))
		{
			// Clear the interrupt flag in the timer, so we don't service
			// the same interrupt twice.
			XScuTimer_ClearInterruptStatus(my_Timer_LOCAL);
			// Print something to the UART to show that we're in the interrupt handler
			printf("** Interrupt Handler by Timer1!\n\r");
		}
}