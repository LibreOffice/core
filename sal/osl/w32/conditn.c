/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: conditn.c,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "system.h"

#include <osl/conditn.h>
#include <osl/diagnose.h>
#include <osl/time.h>

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

