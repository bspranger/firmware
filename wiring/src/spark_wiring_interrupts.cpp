/**
 ******************************************************************************
 * @file    spark_wiring_interrupts.cpp
 * @author  Mohit Bhoite
 * @version V1.0.0
 * @date    13-March-2013
 * @brief   Wrapper for wiring interrupts
 ******************************************************************************
  Copyright (c) 2013-2015 Particle Industries, Inc.  All rights reserved.
  Copyright (c) 2004-05 Hernando Barragan
  Modified 24 November 2006 by David A. Mellis
  Modified 1 August 2010 by Mark Sproul

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#include "spark_wiring_interrupts.h"

static wiring_interrupt_handler_t* handlers[16];

wiring_interrupt_handler_t* allocate_handler(uint16_t pin, wiring_interrupt_handler_t& fn)
{
    delete handlers[pin];
    return handlers[pin] = new wiring_interrupt_handler_t(fn);
}

void call_wiring_interrupt_handler(void* data)
{
    wiring_interrupt_handler_t* handler = (wiring_interrupt_handler_t*)data;
    (*handler)();
}

void call_raw_interrupt_handler(void* data)
{
    raw_interrupt_handler_t handler = raw_interrupt_handler_t(data);
    handler();
}

/*******************************************************************************
 * Function Name  : attachInterrupt
 * Description    : Arduino compatible function to attach hardware interrupts to
						        the Core pins
 * Input          : pin number, user function name and interrupt mode
 * Output         : None.
 * Return         : true if function handler was allocated, false otherwise.
 *******************************************************************************/

bool attachInterrupt(uint16_t pin, wiring_interrupt_handler_t fn, InterruptMode mode)
{
#if Wiring_Cellular == 1
  /* safety check that prevents users from attaching an interrupt to D7
   * which is shared with BATT_INT_PC13 for power management */
  if (pin == D7) return false;
#endif
    HAL_Interrupts_Detach(pin);
    wiring_interrupt_handler_t* handler = allocate_handler(pin, fn);
    if (handler) {
        HAL_Interrupts_Attach(pin, call_wiring_interrupt_handler, handler, mode, NULL);
    }
    return handler!=NULL;
}

bool attachInterrupt(uint16_t pin, raw_interrupt_handler_t handler, InterruptMode mode)
{
#if Wiring_Cellular == 1
  /* safety check that prevents users from attaching an interrupt to D7
   * which is shared with BATT_INT_PC13 for power management */
  if (pin == D7) return false;
#endif
    HAL_Interrupts_Detach(pin);
    HAL_Interrupts_Attach(pin, call_raw_interrupt_handler, (void*)handler, mode, NULL);
    return true;
}


/*******************************************************************************
 * Function Name  : detachInterrupt
 * Description    : Arduino compatible function to detach hardware interrupts that
						        were asssigned previously using attachInterrupt
 * Input          : pin number to which the interrupt was attached
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void detachInterrupt(uint16_t pin)
{
#if Wiring_Cellular == 1
    /* safety check that prevents users from detaching an interrupt from
     * BATT_INT_PC13 for power management which is shared with D7 */
    if (pin == D7) return;
#endif
    HAL_Interrupts_Detach(pin);
    delete handlers[pin];
    handlers[pin] = NULL;
}

/*******************************************************************************
 * Function Name  : noInterrupts
 * Description    : Disable all external interrupts
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void noInterrupts(void)
{
  //Only disable the interrupts that are exposed to the user
  HAL_Interrupts_Disable_All();
}


/*******************************************************************************
 * Function Name  : interrupts
 * Description    : Enable all external interrupts
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void interrupts(void)
{
  //Only enable the interrupts that are exposed to the user
  HAL_Interrupts_Enable_All();
}

/*
 * System Interrupts
 */
bool attachSystemInterrupt(hal_irq_t irq, wiring_interrupt_handler_t handler)
{
    HAL_InterruptCallback callback;
    callback.handler = call_wiring_interrupt_handler;
    wiring_interrupt_handler_t& h = handler;
    callback.data = new wiring_interrupt_handler_t(h);
    return HAL_Set_System_Interrupt_Handler(irq, &callback, NULL, NULL);
}

/**
 * Removes all registered handlers from the given system interrupt.
 * @param irq   The interrupt from which all handlers are removed.
 * @return {@code true} if handlers were removed.
 */
bool detachSystemInterrupt(hal_irq_t irq)
{
    return HAL_Set_System_Interrupt_Handler(irq, NULL, NULL, NULL);
}
