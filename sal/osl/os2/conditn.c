/*************************************************************************
 *
 *  $RCSfile: conditn.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-14 09:48:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "system.h"

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif


#include <osl/conditn.h>
#include <osl/types.h>
#include <osl/diagnose.h>

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

