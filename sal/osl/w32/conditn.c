/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conditn.c,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:05:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "system.h"

#include <osl/conditn.h>
#include <osl/diagnose.h>

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

/*
    under WIN32, we use the void* oslCondition
    as a WIN32 HANDLE (which is also a 32-bit value)
*/

/*****************************************************************************/
/* osl_createCondition */
/*****************************************************************************/
oslCondition SAL_CALL osl_createCondition(void)
{
    oslCondition Condition;

    Condition= (oslCondition)CreateEvent(0,         /* no security */
                                         sal_True,      /* manual reset */
                                         sal_False,     /* initial state not signaled */
                                         0);        /* automatic name */

    return Condition;

}

/*****************************************************************************/
/* osl_destroyCondition */
/*****************************************************************************/
void SAL_CALL osl_destroyCondition(oslCondition Condition)
{
    if(Condition)
    {
        OSL_VERIFY(CloseHandle(Condition));
    }
}

/*****************************************************************************/
/* osl_setCondition */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setCondition(oslCondition Condition)
{
    OSL_ASSERT(Condition);

    return (sal_Bool)(SetEvent((HANDLE)Condition) != FALSE);
}

/*****************************************************************************/
/* osl_resetCondition */
/*****************************************************************************/
sal_Bool SAL_CALL osl_resetCondition(oslCondition Condition)
{
    OSL_ASSERT(Condition);

    return (sal_Bool)(ResetEvent((HANDLE)Condition) != FALSE);
}

/*****************************************************************************/
/* osl_waitCondition */
/*****************************************************************************/
oslConditionResult SAL_CALL osl_waitCondition(oslCondition Condition,
                                     const TimeValue* pTimeout)
{
    DWORD timeout;

    OSL_ASSERT(Condition);

    if (pTimeout)
        timeout = pTimeout->Seconds * 1000 + pTimeout->Nanosec / 1000000L;
    else
        timeout = INFINITE;

    switch ( WaitForSingleObject( (HANDLE)Condition, timeout )  )
    {
        case WAIT_OBJECT_0:
            return (osl_cond_result_ok);

        case WAIT_TIMEOUT:
            return (osl_cond_result_timeout);

        default:
            return (osl_cond_result_error);
    }
}

/*****************************************************************************/
/* osl_checkCondition */
/*****************************************************************************/
sal_Bool SAL_CALL osl_checkCondition(oslCondition Condition)
{
    OSL_ASSERT(Condition);

    return (sal_Bool)(WaitForSingleObject((HANDLE)Condition, 0) == WAIT_OBJECT_0);
}

