/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: conditn.c,v $
 * $Revision: 1.6 $
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
oslCondition SAL_CALL osl_createCondition()
{
    HEV hevCondition;
    APIRET rc;

    rc = DosCreateEventSem( NULL,       /* unnamed semaphore */
                            &hevCondition,  /* pointer to variable */
                                                   /* for the sem-handle */
                            DC_SEM_SHARED,  /* shared semaphore */
                            FALSE );         /* initial state is posted */

    if( rc == NO_ERROR )
        return (oslCondition)hevCondition;
    else
        return NULL;
}

/*****************************************************************************/
/* osl_destroyCondition */
/*****************************************************************************/
void SAL_CALL osl_destroyCondition(oslCondition Condition)
{
    if( Condition )
        DosCloseEventSem( (HEV) Condition );
}

/*****************************************************************************/
/* osl_setCondition */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setCondition(oslCondition Condition)
{
    OSL_ASSERT(Condition);

    return DosPostEventSem((HEV)Condition) == NO_ERROR;
}

/*****************************************************************************/
/* osl_resetCondition */
/*****************************************************************************/
sal_Bool SAL_CALL osl_resetCondition(oslCondition Condition)
{
    ULONG ulPostCount;

    OSL_ASSERT(Condition);

    return DosResetEventSem((HEV)Condition, &ulPostCount) == NO_ERROR;
}

/*****************************************************************************/
/* osl_waitCondition */
/*****************************************************************************/
oslConditionResult SAL_CALL osl_waitCondition(oslCondition Condition, const TimeValue * pTimeout )
{
    long nTimeout;
    APIRET rc;
    OSL_ASSERT(Condition);

    if( pTimeout )
        nTimeout = pTimeout->Seconds * 1000 + pTimeout->Nanosec / 1000000;
    else
        nTimeout = SEM_INDEFINITE_WAIT;

    rc = DosWaitEventSem((HEV)Condition, nTimeout );
    if( rc == ERROR_TIMEOUT )
        return osl_cond_result_timeout;
    if( rc != NO_ERROR )
        return osl_cond_result_error;

    return osl_cond_result_ok;
}

/*****************************************************************************/
/* osl_checkCondition */
/*****************************************************************************/
sal_Bool SAL_CALL osl_checkCondition(oslCondition Condition)
{
    OSL_ASSERT(Condition);

    return( DosWaitEventSem((HEV)Condition, SEM_IMMEDIATE_RETURN) == NO_ERROR);
}

