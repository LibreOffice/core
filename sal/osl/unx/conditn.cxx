/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/config.h"

#include <assert.h>

#include "system.hxx"
#include <sal/log.hxx>
#include <sal/types.h>

#include <osl/conditn.h>
#include <osl/time.h>

typedef struct _oslConditionImpl
{
    pthread_cond_t  m_Condition;
    pthread_mutex_t m_Lock;
    bool            m_State;
} oslConditionImpl;

oslCondition SAL_CALL osl_createCondition()
{
    oslConditionImpl* pCond;
    int nRet=0;

    pCond = static_cast<oslConditionImpl*>(malloc(sizeof(oslConditionImpl)));

    if ( pCond == nullptr )
    {
        return nullptr;
    }

    pCond->m_State = false;

    /* init condition variable with default attr. (PTHREAD_PROCESS_PRIVAT) */
    nRet =  pthread_cond_init(&pCond->m_Condition, PTHREAD_CONDATTR_DEFAULT);
    if ( nRet != 0 )
    {
        SAL_WARN( "sal.osl.condition", "pthread_cond_init failed: " << strerror(nRet) );

        free(pCond);
        return nullptr;
    }

    nRet = pthread_mutex_init(&pCond->m_Lock, PTHREAD_MUTEXATTR_DEFAULT);
    if ( nRet != 0 )
    {
        SAL_WARN( "sal.osl.condition", "pthread_mutex_init failed: " << strerror(nRet) );

        nRet = pthread_cond_destroy(&pCond->m_Condition);
        SAL_WARN_IF( nRet != 0, "sal.osl.condition", "pthread_cond_destroy failed: " << strerror(nRet) );

        free(pCond);
        pCond = nullptr;
    }

    SAL_INFO( "sal.osl.condition", "osl_createCondition(): " << pCond );

    return static_cast<oslCondition>(pCond);
}

void SAL_CALL osl_destroyCondition(oslCondition Condition)
{
    oslConditionImpl* pCond;

    pCond = static_cast<oslConditionImpl*>(Condition);

    SAL_INFO( "sal.osl.condition", "osl_destroyCondition(" << pCond << ")" );

    if ( pCond )
    {
        int nRet = pthread_cond_destroy(&pCond->m_Condition);
        SAL_WARN_IF( nRet != 0, "sal.osl.condition", "pthread_cond_destroy failed: " << strerror(nRet) );
        nRet = pthread_mutex_destroy(&pCond->m_Lock);
        SAL_WARN_IF( nRet != 0, "sal.osl.condition", "pthread_mutex_destroy failed: " << strerror(nRet) );

        free(Condition);
    }

    return;
}

sal_Bool SAL_CALL osl_setCondition(oslCondition Condition)
{
   oslConditionImpl* pCond;
   int nRet=0;

   assert(Condition);
   pCond = static_cast<oslConditionImpl*>(Condition);

   nRet = pthread_mutex_lock(&pCond->m_Lock);
   if ( nRet != 0 )
   {
       SAL_WARN( "sal.osl.condition", "osl_setCondition(" << pCond << "): pthread_mutex_lock failed: " << strerror(nRet) );
       return sal_False;
   }

   pCond->m_State = true;
   nRet = pthread_cond_broadcast(&pCond->m_Condition);
   if ( nRet != 0 )
   {
       SAL_WARN( "sal.osl.condition", "osl_setCondition(" << pCond << "): pthread_cond_broadcast failed: " << strerror(nRet) );
       // try to unlock the mutex
       pthread_mutex_unlock(&pCond->m_Lock);
       return sal_False;
   }

   nRet = pthread_mutex_unlock(&pCond->m_Lock);
   if ( nRet != 0 )
   {
       SAL_WARN( "sal.osl.condition", "osl_setCondition(" << pCond << "): pthread_mutex_unlock failed: " << strerror(nRet) );
       return sal_False;
   }

   SAL_INFO( "sal.osl.condition", "osl_setCondition(" << pCond << ")" );

   return sal_True;

}

sal_Bool SAL_CALL osl_resetCondition(oslCondition Condition)
{
    oslConditionImpl* pCond;
    int nRet=0;

    assert(Condition);

    pCond = static_cast<oslConditionImpl*>(Condition);

    nRet = pthread_mutex_lock(&pCond->m_Lock);
    if ( nRet != 0 )
    {
        SAL_WARN( "sal.osl.condition", "osl_resetCondition(" << pCond << "): pthread_mutex_lock failed: " << strerror(nRet) );
        return sal_False;
    }

    pCond->m_State = false;

    nRet = pthread_mutex_unlock(&pCond->m_Lock);
    if ( nRet != 0 )
    {
        SAL_WARN( "sal.osl.condition", "osl_resetCondition(" << pCond << "): pthread_mutex_unlock failed: " << strerror(nRet) );
        return sal_False;
    }

    SAL_INFO( "sal.osl.condition", "osl_resetCondition(" << pCond << ")" );

    return sal_True;
}

oslConditionResult SAL_CALL osl_waitCondition(oslCondition Condition, const TimeValue* pTimeout)
{
    oslConditionImpl* pCond;
    int nRet=0;
    oslConditionResult Result = osl_cond_result_ok;

    assert(Condition);
    pCond = static_cast<oslConditionImpl*>(Condition);

    SAL_INFO( "sal.osl.condition", "osl_waitCondition(" << pCond << ")" );

    nRet = pthread_mutex_lock(&pCond->m_Lock);
    if ( nRet != 0 )
    {
        SAL_WARN( "sal.osl.condition", "osl_waitCondition(" << pCond << "): pthread_mutex_lock failed: " << strerror(nRet) );
        return osl_cond_result_error;
    }

    if ( pTimeout )
    {
        if ( ! pCond->m_State )
        {
            struct timeval      tp;
            struct timespec     to;

            gettimeofday(&tp, nullptr);

            SET_TIMESPEC( to, tp.tv_sec + pTimeout->Seconds,
                              tp.tv_usec * 1000 + pTimeout->Nanosec );

            /* spurious wake up prevention */
            do
            {
                const int ret = pthread_cond_timedwait(&pCond->m_Condition, &pCond->m_Lock, &to);
                if ( ret != 0 )
                {
                    if ( ret == ETIME || ret == ETIMEDOUT )
                    {
                        Result = osl_cond_result_timeout;
                        nRet = pthread_mutex_unlock(&pCond->m_Lock);
                        SAL_WARN_IF( nRet != 0, "sal.osl.condition", "osl_waitCondition(" << pCond << "): pthread_mutex_unlock failed: " << strerror(nRet) );

                        return Result;
                    }
                    else if ( ret != EINTR )
                    {
                        Result = osl_cond_result_error;
                        nRet = pthread_mutex_unlock(&pCond->m_Lock);
                        SAL_WARN_IF( nRet != 0, "sal.osl.condition", "osl_waitCondition(" << pCond << "): pthread_mutex_unlock failed: " << strerror(nRet) );
                        return Result;
                    }
                }
            }
            while ( !pCond->m_State );
        }
    }
    else
    {
        while ( !pCond->m_State )
        {
            nRet = pthread_cond_wait(&pCond->m_Condition, &pCond->m_Lock);
            if ( nRet != 0 )
            {
                SAL_WARN( "sal.osl.condition", "osl_waitCondition(" << pCond << "): pthread_cond_wait failed: " << strerror(nRet) );
                Result = osl_cond_result_error;
                nRet = pthread_mutex_unlock(&pCond->m_Lock);
                SAL_WARN_IF( nRet != 0, "sal.osl.condition", "osl_waitCondition(" << pCond << "): pthread_mutex_unlock failed: " << strerror(nRet) );

                return Result;
            }
        }
    }

    nRet = pthread_mutex_unlock(&pCond->m_Lock);
    SAL_WARN_IF( nRet != 0, "sal.osl.condition", "osl_waitCondition(" << pCond << "): pthread_mutex_unlock failed: " << strerror(nRet) );

    SAL_INFO( "sal.osl.condition", "osl_waitCondition(" << pCond << "): " << (Result == osl_cond_result_ok ? "OK" : "ERROR") );

    return Result;
}

sal_Bool SAL_CALL osl_checkCondition(oslCondition Condition)
{
    bool State;
    oslConditionImpl* pCond;
    int nRet=0;

    assert(Condition);
    pCond = static_cast<oslConditionImpl*>(Condition);

    nRet = pthread_mutex_lock(&pCond->m_Lock);
    SAL_WARN_IF( nRet != 0, "sal.osl.condition", "osl_checkCondition(" << pCond << "): pthread_mutex_lock failed: " << strerror(nRet) );

    State = pCond->m_State;

    nRet = pthread_mutex_unlock(&pCond->m_Lock);
    SAL_WARN_IF( nRet != 0, "sal.osl.condition", "osl_checkCondition(" << pCond << "): pthread_mutex_unlock failed: " << strerror(nRet) );

    SAL_INFO( "sal.osl.condition", "osl_checkCondition(" << pCond << "): " << (State ? "YES" : "NO") );

    return State;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
