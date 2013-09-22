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

#include "system.h"
#include <sal/log.hxx>
#include <sal/types.h>

#include <osl/conditn.h>
#include <osl/time.h>


typedef struct _oslConditionImpl
{
    pthread_cond_t  m_Condition;
    pthread_mutex_t m_Lock;
    sal_Bool            m_State;
} oslConditionImpl;


oslCondition SAL_CALL osl_createCondition()
{
      oslConditionImpl* pCond;
    int nRet=0;

    pCond = (oslConditionImpl*) malloc(sizeof(oslConditionImpl));

    if ( pCond == 0 )
    {
        SAL_WARN("sal.osl", "std::bad_alloc in C");
        return 0;
    }

    pCond->m_State = sal_False;

    /* init condition variable with default attr. (PTHREAD_PROCESS_PRIVAT) */
    nRet =  pthread_cond_init(&pCond->m_Condition, PTHREAD_CONDATTR_DEFAULT);
    if ( nRet != 0 )
    {
        SAL_WARN(
            "sal.osl",
            "pthread_cond_init failed, errno " << nRet << ", \""
                << strerror(nRet) << '"');

        free(pCond);
        return 0;
    }

    nRet = pthread_mutex_init(&pCond->m_Lock, PTHREAD_MUTEXATTR_DEFAULT);
    if ( nRet != 0 )
    {
        SAL_WARN(
            "sal.osl",
            "pthread_mutex_init failed, errno " << nRet << ", \""
                << strerror(nRet) << '"');

        nRet = pthread_cond_destroy(&pCond->m_Condition);
        SAL_WARN_IF(
            nRet != 0, "sal.osl",
            "pthread_cond_destroy failed, errno " << nRet << ", \""
                << strerror(nRet) << '"');

        free(pCond);
        pCond = 0;
    }

    return (oslCondition)pCond;
}

void SAL_CALL osl_destroyCondition(oslCondition Condition)
{
    oslConditionImpl* pCond;
    int nRet = 0;

    if ( Condition )
    {
        pCond = (oslConditionImpl*)Condition;

        nRet = pthread_cond_destroy(&pCond->m_Condition);
        SAL_WARN_IF(
            nRet != 0, "sal.osl",
            "pthread_cond_destroy failed, errno " << nRet << ", \""
                << strerror(nRet) << '"');
        nRet = pthread_mutex_destroy(&pCond->m_Lock);
        SAL_WARN_IF(
            nRet != 0, "sal.osl",
            "pthread_mutex_destroy failed, errno " << nRet << ", \""
                << strerror(nRet) << '"');

        free(Condition);
    }

    return;
}

sal_Bool SAL_CALL osl_setCondition(oslCondition Condition)
{
   oslConditionImpl* pCond;
   int nRet=0;

   assert(Condition);
   pCond = (oslConditionImpl*)Condition;

   if ( pCond == 0 )
   {
       return sal_False;
   }

   nRet = pthread_mutex_lock(&pCond->m_Lock);
   if ( nRet != 0 )
   {
       SAL_WARN(
           "sal.osl",
           "pthread_mutex_lock failed, errno " << nRet << ", \""
               << strerror(nRet) << '"');
       return sal_False;
   }

   pCond->m_State = sal_True;
   nRet = pthread_cond_broadcast(&pCond->m_Condition);
   if ( nRet != 0 )
   {
       SAL_WARN(
           "sal.osl",
           "pthread_cond_broadcast failed, errno " << nRet << ", \""
               << strerror(nRet) << '"');
       // try to unlock the mutex
       pthread_mutex_unlock(&pCond->m_Lock);
       return sal_False;
   }

   nRet = pthread_mutex_unlock(&pCond->m_Lock);
   if ( nRet != 0 )
   {
       SAL_WARN(
           "sal.osl",
           "pthread_mutex_unlock failed, errno " << nRet << ", \""
               << strerror(nRet) << '"');
       return sal_False;
   }

   return sal_True;

}

sal_Bool SAL_CALL osl_resetCondition(oslCondition Condition)
{
    oslConditionImpl* pCond;
    int nRet=0;

    assert(Condition);

    pCond = (oslConditionImpl*)Condition;

    if ( pCond == 0 )
    {
        return sal_False;
    }

    nRet = pthread_mutex_lock(&pCond->m_Lock);
    if ( nRet != 0 )
    {
        SAL_WARN(
            "sal.osl",
            "pthread_mutex_lock failed, errno " << nRet << ", \""
                << strerror(nRet) << '"');
        return sal_False;
    }

    pCond->m_State = sal_False;

    nRet = pthread_mutex_unlock(&pCond->m_Lock);
    if ( nRet != 0 )
    {
       SAL_WARN(
           "sal.osl", "pthread_mutex_unlock failed, errno " << nRet <<", \""
               << strerror(nRet) << '"');
        return sal_False;
    }

    return sal_True;
}

oslConditionResult SAL_CALL osl_waitCondition(oslCondition Condition, const TimeValue* pTimeout)
{
    oslConditionImpl* pCond;
    int nRet=0;
    oslConditionResult Result = osl_cond_result_ok;

    assert(Condition);
    pCond = (oslConditionImpl*)Condition;

    if ( pCond == 0 )
    {
        return osl_cond_result_error;
    }

    nRet = pthread_mutex_lock(&pCond->m_Lock);
    if ( nRet != 0 )
    {
        SAL_WARN(
            "sal.osl", "pthread_mutex_lock failed, errno " << nRet <<", \""
                << strerror(nRet) << '"');
        return osl_cond_result_error;
    }

    if ( pTimeout )
    {
        if ( ! pCond->m_State )
        {
            int                 ret;
            struct timeval      tp;
            struct timespec     to;

            gettimeofday(&tp, NULL);

            SET_TIMESPEC( to, tp.tv_sec + pTimeout->Seconds,
                              tp.tv_usec * 1000 + pTimeout->Nanosec );

            /* spurious wake up prevention */
            do
            {
                ret = pthread_cond_timedwait(&pCond->m_Condition, &pCond->m_Lock, &to);
                if ( ret != 0 )
                {
                    if ( ret == ETIME || ret == ETIMEDOUT )
                    {
                        Result = osl_cond_result_timeout;
                        nRet = pthread_mutex_unlock(&pCond->m_Lock);
                        SAL_WARN_IF(
                            nRet != 0, "sal.osl",
                            "pthread_mutex_unlock failed, errno " << nRet
                                << ", \"" << strerror(nRet) << '"');

                        return Result;
                    }
                    else if ( ret != EINTR )
                    {
                        Result = osl_cond_result_error;
                        nRet = pthread_mutex_unlock(&pCond->m_Lock);
                        SAL_WARN_IF(
                            nRet != 0, "sal.osl",
                            "pthread_mutex_unlock failed, errno " << nRet
                                << ", \"" << strerror(nRet) << '"');
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
                SAL_WARN(
                    "sal.osl",
                    "pthread_cond_wait failed, errno " << nRet << ", \""
                        << strerror(nRet) << '"');
                Result = osl_cond_result_error;
                nRet = pthread_mutex_unlock(&pCond->m_Lock);
                SAL_WARN_IF(
                    nRet != 0, "sal.osl",
                    "pthread_mutex_unlock failed, errno " << nRet << ", \""
                        << strerror(nRet) << '"');

                return Result;
            }
        }
    }

    nRet = pthread_mutex_unlock(&pCond->m_Lock);
    SAL_WARN_IF(
        nRet != 0, "sal.osl",
        "pthread_mutex_unlock failed, errno " << nRet << ", \""
            << strerror(nRet) << '"');

    return Result;
}

sal_Bool SAL_CALL osl_checkCondition(oslCondition Condition)
{
    sal_Bool State;
    oslConditionImpl* pCond;
    int nRet=0;

    assert(Condition);
    pCond = (oslConditionImpl*)Condition;

    if ( pCond == 0 )
    {
        return sal_False;
    }

    nRet = pthread_mutex_lock(&pCond->m_Lock);
    SAL_WARN_IF(
        nRet != 0, "sal.osl",
        "pthread_mutex_lock failed, errno " << nRet << ", \"" << strerror(nRet)
            << '"');

    State = pCond->m_State;

    nRet = pthread_mutex_unlock(&pCond->m_Lock);
    SAL_WARN_IF(
        nRet != 0, "sal.osl",
        "pthread_mutex_unlock failed, errno " << nRet << ", \""
            << strerror(nRet) << '"');

    return State;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
