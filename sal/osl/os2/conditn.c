/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

