/*************************************************************************
 *
 *  $RCSfile: thread.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hro $ $Date: 2000-09-29 13:28:02 $
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


/* system headers */
#include "system.h"

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/signal.h>


/*****************************************************************************/
/*  Thread-data structure hidden behind oslThread:  */
/*****************************************************************************/

#define THREADIMPL_FLAGS_TERMINATE  0x00001
#define THREADIMPL_FLAGS_SLEEP      0x00002
#define THREADIMPL_FLAGS_SUSPENDED  0x00004
#define THREADIMPL_FLAGS_ACTIVE     0x00008
#define THREADIMPL_FLAGS_ATTACHED   0x00010
#define THREADIMPL_FLAGS_STARTUP    0x00020

typedef struct _osl_TThreadImpl {
    pthread_t           m_hThread;      /* OS-handle used for all thread-functions */
    sal_uInt16          m_Ident;
    short               m_Flags;
    sal_uInt32          m_Timeout;
    oslWorkerFunction   m_WorkerFunction;
    void*               m_pData;
    pthread_mutex_t     m_HandleLock;
    pthread_cond_t      m_Suspend;
    pthread_mutex_t     m_AccessLock;
} osl_TThreadImpl;

static sal_Bool osl_init_thread= sal_False;

#ifndef NO_PTHREAD_PRIORITY
static int Thread_Prio_Highest      = 127;
static int Thread_Prio_Above_Normal = 96;
static int Thread_Prio_Normal       = 64;
static int Thread_Prio_Below_Normal = 32;
static int Thread_Prio_Lowest       = 0;
#endif

static sal_uInt16 insertThreadId();
static sal_uInt16 lookupThreadId();
static void removeThreadId();
static sal_Bool osl_initThread();
static void oslCleanupFunction(void* pData);
static void* oslWorkerWrapperFunction(void* pData);
static oslThread oslCreateThread(oslWorkerFunction pWorker, void* pThreadData, short nFlags);


#if defined(SOLARIS)

extern void ChangeGlobalInit();

static oslSignalAction oslSigAlarmHandler(void* pData, oslSignalInfo* pInfo)
{
    if ( pInfo == 0 )
    {
        return osl_Signal_ActCallNextHdl;
    }

    if ( pInfo->Signal == osl_Signal_Alarm )
    {
        return osl_Signal_ActIgnore;
    }

    return osl_Signal_ActCallNextHdl;
}

#endif


/*****************************************************************************/
/* ID management */
/*****************************************************************************/

#define HASHID(x)   ((int)PTHREAD_VALUE(x) % HashSize)

typedef struct _HashEntry
{
    pthread_t         Handle;
    sal_uInt16        Ident;
    struct _HashEntry *Next;
} HashEntry;

static HashEntry* HashTable[32];
static int HashSize = sizeof(HashTable) / sizeof(HashEntry*);

static pthread_mutex_t HashLock;

static sal_uInt16 LastIdent = 0;

static sal_uInt16 lookupThreadId()
{
    int       i, n;
    HashEntry *pEntry;
    pthread_t hThread = pthread_self();

    pthread_mutex_lock(&HashLock);

    for (n = 0, i = HASHID(hThread); n < HashSize; n++, i = ++i % HashSize)
    {
        pEntry = HashTable[i];

        while (pEntry != NULL)
        {
            if (pthread_equal(pEntry->Handle, hThread))
            {
                pthread_mutex_unlock(&HashLock);

                return (pEntry->Ident);
            }

            pEntry = pEntry->Next;
        }
    }

    pthread_mutex_unlock(&HashLock);

    return (0);
}

static sal_uInt16 insertThreadId()
{
    HashEntry *pEntry, *pInsert = NULL;
    pthread_t hThread = pthread_self();

    pthread_mutex_lock(&HashLock);

    pEntry = HashTable[HASHID(hThread)];

    while (pEntry != NULL)
    {
        if (pthread_equal(pEntry->Handle, hThread))
            break;

        pInsert = pEntry;
        pEntry = pEntry->Next;
    }

    if (pEntry == NULL)
    {
        pEntry = (HashEntry*) calloc(sizeof(HashEntry), 1);

        pEntry->Handle = hThread;
        pEntry->Ident  = ++LastIdent;

        if (pInsert)
            pInsert->Next = pEntry;
        else
            HashTable[HASHID(hThread)] = pEntry;
    }

    pthread_mutex_unlock(&HashLock);

    return (pEntry->Ident);
}

static void removeThreadId()
{
    pthread_t hThread = pthread_self();
    HashEntry *pEntry, *pRemove = NULL;

    pthread_mutex_lock(&HashLock);

    pEntry = HashTable[HASHID(hThread)];

    while (pEntry != NULL)
    {
        if (pthread_equal(pEntry->Handle, hThread))
            break;

        pRemove = pEntry;
        pEntry = pEntry->Next;
    }

    if (pEntry != NULL)
    {
        if (pRemove)
            pRemove->Next = pEntry->Next;
        else
            HashTable[HASHID(hThread)] = pEntry->Next;

        free(pEntry);
    }

    pthread_mutex_unlock(&HashLock);
}

/*****************************************************************************/
/* oslCleanupFunction */
/*****************************************************************************/
static void oslCleanupFunction(void* pData)
{
    sal_Bool attached;

    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)pData;

    pthread_mutex_lock(&pThreadImpl->m_HandleLock);
    pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_ACTIVE;
    attached = pThreadImpl->m_Flags & THREADIMPL_FLAGS_ATTACHED;
    pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_ATTACHED;
    pthread_mutex_unlock(&pThreadImpl->m_HandleLock);

    removeThreadId();

    if (! attached)
    {
        pthread_cond_destroy(&pThreadImpl->m_Suspend);
        pthread_mutex_destroy(&pThreadImpl->m_HandleLock);
        pthread_mutex_destroy(&pThreadImpl->m_AccessLock);

        /* free memory */
        free(pThreadImpl);
    }
}

/*****************************************************************************/
/* oslWorkerWrapperFunction */
/*****************************************************************************/
static void* oslWorkerWrapperFunction(void* pData)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)pData;


    pthread_mutex_lock(&pThreadImpl->m_AccessLock);

    pthread_cleanup_push(oslCleanupFunction, pData);


    /* Check if thread is started in suspended state */
    pthread_mutex_lock(&pThreadImpl->m_HandleLock);
    pThreadImpl->m_Ident = insertThreadId();
    pThreadImpl->m_Flags |= THREADIMPL_FLAGS_ACTIVE;
    pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_STARTUP;
    pthread_mutex_unlock(&pThreadImpl->m_HandleLock);


    pthread_cond_signal(&pThreadImpl->m_Suspend);


    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SUSPENDED)
    {
        pthread_cond_wait(&pThreadImpl->m_Suspend, &pThreadImpl->m_AccessLock);
    }


    pthread_mutex_unlock(&pThreadImpl->m_AccessLock);


    /* call worker-function with data */
    pThreadImpl->m_WorkerFunction(pThreadImpl->m_pData);

    pthread_cleanup_pop(1);

    return (0);
}

/*****************************************************************************/
/* oslCreateThread */
/*****************************************************************************/
static oslThread oslCreateThread(oslWorkerFunction pWorker,
                                 void* pThreadData,
                                 short nFlags)
{
    osl_TThreadImpl* pThreadImpl;
    int nRet=0;

    /* init threads only once */
    if ( ! osl_init_thread )
        osl_init_thread = osl_initThread();

    /* alloc mem. for our internal data structure */
    pThreadImpl= (osl_TThreadImpl*)malloc(sizeof(osl_TThreadImpl));

    OSL_ASSERT(pThreadImpl);

    if ( pThreadImpl == 0 )
    {
        return 0;
    }

    pThreadImpl->m_WorkerFunction= pWorker;
    pThreadImpl->m_pData= pThreadData;

    pThreadImpl->m_Flags = nFlags;

    pthread_mutex_init(&pThreadImpl->m_HandleLock, PTHREAD_MUTEXATTR_DEFAULT);
    pthread_mutex_init(&pThreadImpl->m_AccessLock, PTHREAD_MUTEXATTR_DEFAULT);
    pthread_cond_init(&pThreadImpl->m_Suspend, PTHREAD_CONDATTR_DEFAULT);

    pthread_mutex_lock(&pThreadImpl->m_AccessLock);

    if ( ( nRet=pthread_create((pthread_t*)&pThreadImpl->m_hThread, /* receives thread data */
                               PTHREAD_ATTR_DEFAULT,
                               oslWorkerWrapperFunction,
                               (void*)pThreadImpl) ) != 0 )
    {
        OSL_TRACE("oslCreateThread failed. Errno: %d; %s\n",
                  nRet, strerror(nRet));

        pthread_cond_destroy(&pThreadImpl->m_Suspend);
        pthread_mutex_destroy(&pThreadImpl->m_HandleLock);
        pthread_mutex_destroy(&pThreadImpl->m_AccessLock);

        free(pThreadImpl);
        return 0;
    }

    pthread_cond_wait(&pThreadImpl->m_Suspend,&pThreadImpl->m_AccessLock);

    pthread_mutex_unlock(&pThreadImpl->m_AccessLock);

#ifdef S390
    pthread_setintrtype(PTHREAD_INTR_CONTROLLED);
    pthread_setintrtype(PTHREAD_INTR_ENABLE);
#endif

    return (oslThread)pThreadImpl;
}

/*****************************************************************************/
/*  osl_initThread
    set the base-priority of the main-thread to
    oslThreadPriorityNormal (64) since 0 (lowest) is
    the system default. This behaviour collides with
    our enum-priority definition (highest..normal..lowest).
    A  normaluser will expect the main-thread of an app.
    to have the "normal" priority.
    We do this by using this static (thus private for this file)
    function (osl_initThread()) to osl_init_thread the static var init.
*/
/*****************************************************************************/
static sal_Bool osl_initThread()
{
#ifndef NO_PTHREAD_PRIORITY
    struct sched_param param;
    int policy=0;
    int nRet=0;
#endif /* NO_PTHREAD_PRIORITY */


#if defined(SOLARIS)

    ChangeGlobalInit();

    /*
     *  mfe: Under Solaris we get SIGALRM in e.g. pthread_join which terminates the process
     *       So we initialize our signal handling module and do register a SIGALRM Handler
     *       which catches and ignores it.
     */
    osl_addSignalHandler(&oslSigAlarmHandler,NULL);

#endif /* SOLARIS */


    pthread_mutex_init(&HashLock, PTHREAD_MUTEXATTR_DEFAULT);

    /* main thread should have first ID */
    insertThreadId();


#ifndef NO_PTHREAD_PRIORITY

    if ( ( nRet = pthread_getschedparam(pthread_self(), &policy, &param)) != 0 )
    {
        OSL_TRACE("failed to get priority of thread [%s]\n",strerror(nRet));
        return sal_False;
    }

    if ( policy >= _SCHED_NEXT)
    {
        /* mfe: pthread_getschedparam on Solaris has a possible Bug */
        /*      one gets 959917873 as the policy                    */
        /*      so set the policy to a default one                  */
        policy=SCHED_OTHER;
    }

    Thread_Prio_Normal       = (Thread_Prio_Lowest + Thread_Prio_Highest) / 2;
    Thread_Prio_Below_Normal = (Thread_Prio_Lowest + Thread_Prio_Normal)  / 2;
    Thread_Prio_Above_Normal = (Thread_Prio_Normal + Thread_Prio_Highest) / 2;

    param.sched_priority= Thread_Prio_Normal;

    if ( ( nRet = pthread_setschedparam(pthread_self(), policy, &param)) != 0 )
    {
        OSL_TRACE("failed to change base priority of thread [%s]\n",strerror(nRet));
        OSL_TRACE("Thread ID '%i', Policy '%i', Priority '%i'\n",pthread_self(),policy,param.sched_priority);
        return sal_False;
    }

#endif /* NO_PTHREAD_PRIORITY */


    return sal_True;
}

/*****************************************************************************/
/* osl_createThread */
/*****************************************************************************/
oslThread osl_createThread(oslWorkerFunction pWorker,
                           void* pThreadData)
{
    return oslCreateThread(pWorker,
                           pThreadData,
                           THREADIMPL_FLAGS_STARTUP   |
                           THREADIMPL_FLAGS_ATTACHED  );
}

/*****************************************************************************/
/* osl_createSuspendedThread */
/*****************************************************************************/
oslThread osl_createSuspendedThread(oslWorkerFunction pWorker,
                                    void* pThreadData)
{
    return oslCreateThread(pWorker,
                           pThreadData,
                           THREADIMPL_FLAGS_STARTUP   |
                           THREADIMPL_FLAGS_ATTACHED  |
                           THREADIMPL_FLAGS_SUSPENDED );
}

/*****************************************************************************/
/* osl_getThreadIdentifier */
/*****************************************************************************/
oslThreadIdentifier SAL_CALL osl_getThreadIdentifier(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* init threads only once */
    if (! osl_init_thread)
        osl_init_thread = osl_initThread();

    if (pThreadImpl != NULL)
        return ((oslThreadIdentifier)pThreadImpl->m_Ident);
    else
    {
        sal_uInt16 Ident = lookupThreadId();

        if (Ident == 0)
            Ident = insertThreadId();

        return ((oslThreadIdentifier)Ident);
    }
}

/*****************************************************************************/
/* osl_destroyThread */
/*****************************************************************************/
void SAL_CALL osl_destroyThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    if (Thread == 0)        /* valid ptr? */
    {
        /* thread already destroyed or not created */
        return;
    }

    /* cancel thread  */
    pthread_cancel(pThreadImpl->m_hThread);
}

/*****************************************************************************/
/* osl_freeThreadHandle */
/*****************************************************************************/
void SAL_CALL osl_freeThreadHandle(oslThread Thread)
{
    sal_Bool attached;
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    if (Thread == 0)        /* valid ptr? */
    {
        /* thread already destroyed or not created */
        return;
    }

    pthread_mutex_lock(&pThreadImpl->m_HandleLock);
    if ( (attached = (pThreadImpl->m_Flags & THREADIMPL_FLAGS_ATTACHED)) > 0 )
    {
        pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_ATTACHED;
        pthread_detach(pThreadImpl->m_hThread);
    }
    pthread_mutex_unlock(&pThreadImpl->m_HandleLock);

    if (! attached)
    {
        pthread_cond_destroy(&pThreadImpl->m_Suspend);
        pthread_mutex_destroy(&pThreadImpl->m_HandleLock);
        pthread_mutex_destroy(&pThreadImpl->m_AccessLock);

        /* free memory */
        free(pThreadImpl);
    }
}

/*****************************************************************************/
/* osl_resumeThread */
/*****************************************************************************/
void SAL_CALL osl_resumeThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    OSL_ASSERT(pThreadImpl);        /* valid ptr? */

    if ( pThreadImpl == 0 )
    {
        return;
    }


    pthread_mutex_lock(&pThreadImpl->m_HandleLock);

    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SUSPENDED)
    {
        pthread_cond_signal(&pThreadImpl->m_Suspend);
    }


    pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_SUSPENDED;

    pthread_mutex_unlock(&pThreadImpl->m_HandleLock);

    return;
}

/*****************************************************************************/
/* osl_suspendThread */
/*****************************************************************************/
void SAL_CALL osl_suspendThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    OSL_ASSERT(pThreadImpl);        /* valid ptr? */

    if ( pThreadImpl == 0 )
    {
        return;
    }

    pthread_mutex_lock(&pThreadImpl->m_HandleLock);

    pThreadImpl->m_Flags |= THREADIMPL_FLAGS_SUSPENDED;

    pthread_mutex_unlock(&pThreadImpl->m_HandleLock);

    /* suspend now if we are ourself */
    if ( pthread_equal( pthread_self(), pThreadImpl->m_hThread ) )
    {
        pthread_mutex_lock(&pThreadImpl->m_AccessLock);
        if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SUSPENDED)
        {
            pthread_cond_wait(&pThreadImpl->m_Suspend, &pThreadImpl->m_AccessLock);
        }
        pthread_mutex_unlock(&pThreadImpl->m_AccessLock);
    }

}

/*****************************************************************************/
/* osl_setThreadPriority */
/*
    Impl-Notes: contrary to solaris-docu, which claims
    valid priority-levels from 0 .. INT_MAX, only the
    range 0..127 is accepted. (0 lowest, 127 highest)
*/
/*****************************************************************************/
void SAL_CALL osl_setThreadPriority(oslThread Thread,
                           oslThreadPriority Priority)
{
#ifndef NO_PTHREAD_PRIORITY

    struct sched_param Param;
    int policy;
    int nRet;

    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;
    OSL_ASSERT(pThreadImpl);        /* valid ptr? */

    if ( pThreadImpl == 0 )
    {
        return;
    }

    if ( ( nRet = pthread_getschedparam(pThreadImpl->m_hThread, &policy, &Param)) != 0 )
    {
        OSL_TRACE("failed to get priority of thread [%s]\n",strerror(nRet));
        return;
    }

    if ( policy >= _SCHED_NEXT)
    {
        /* mfe: pthread_getschedparam on Salris has a possible Bug */
        /*      one gets 959917873 as the policy                   */
        /*      so set the policy to a default one                 */
        policy=SCHED_RR;
    }

    switch(Priority)
    {
        case osl_Thread_PriorityHighest:
            Param.sched_priority= Thread_Prio_Highest;
            break;

        case osl_Thread_PriorityAboveNormal:
            Param.sched_priority= Thread_Prio_Above_Normal;
            break;

        case osl_Thread_PriorityNormal:
            Param.sched_priority= Thread_Prio_Normal;
            break;

        case osl_Thread_PriorityBelowNormal:
            Param.sched_priority= Thread_Prio_Below_Normal;
            break;

        case osl_Thread_PriorityLowest:
            Param.sched_priority= Thread_Prio_Lowest;
            break;

        case osl_Thread_PriorityUnknown:
            OSL_ASSERT(sal_False);      /* only fools try this...*/

            /* let release-version behave friendly */
            return;

        default:
            OSL_ASSERT(sal_False);      /* enum expanded, but forgotten here...*/

            /* let release-version behave friendly */
            return;
    }


    if ( ( nRet = pthread_setschedparam(pThreadImpl->m_hThread, policy, &Param) ) != 0 )
    {
        OSL_TRACE("failed to change thread priority [%s]\n",strerror(nRet));
    }

#endif

    return;
}

/*****************************************************************************/
/* osl_getThreadPriority */
/* see also notes at osl_setPriorityOfThread */
/*****************************************************************************/
oslThreadPriority  SAL_CALL osl_getThreadPriority(const oslThread Thread)
{
#ifndef NO_PTHREAD_PRIORITY

    struct sched_param Param;
    int Policy;
    oslThreadPriority Priority;

    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments ?*/
    if ( (pThreadImpl == 0) ||
        ( pthread_getschedparam(pThreadImpl->m_hThread, &Policy, &Param) != 0 ) )
    {
        return osl_Thread_PriorityUnknown;
    }

    /* map pthread priority to enum */
    if (Param.sched_priority==Thread_Prio_Highest) {
        /* 127 - highest */
        Priority= osl_Thread_PriorityHighest;

    } else if (Param.sched_priority > Thread_Prio_Normal) {
        /* 65..126 - above normal */
        Priority= osl_Thread_PriorityAboveNormal;

    } else if (Param.sched_priority == Thread_Prio_Normal) {
        /* normal */
        Priority= osl_Thread_PriorityNormal;

    } else if (Param.sched_priority > Thread_Prio_Lowest) {
        /* 63..1 -below normal */
        Priority= osl_Thread_PriorityBelowNormal;

    } else if (Param.sched_priority == Thread_Prio_Lowest) {
        /* 0 - lowest */
        Priority= osl_Thread_PriorityLowest;
    }

    return Priority;

#else

    return osl_Thread_PriorityNormal;

#endif
}

/*****************************************************************************/
/* osl_isThreadRunning */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isThreadRunning(const oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments ?*/
    if (pThreadImpl == NULL)
    {
        return sal_False;
    }

    return ((pThreadImpl->m_Flags &
             (THREADIMPL_FLAGS_ACTIVE | THREADIMPL_FLAGS_STARTUP)) != 0);
}

/*****************************************************************************/
/* osl_joinWithThread */
/*****************************************************************************/
void SAL_CALL osl_joinWithThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* thread already destroyed or not created */
    if (pThreadImpl == NULL)
    {
        return;
    }

    /* wait till it is canceled */
    pthread_join(pThreadImpl->m_hThread, NULL);
    return;
}

/*****************************************************************************/
/* osl_sleepThread */
/*****************************************************************************/
oslThreadSleep SAL_CALL osl_sleepThread(oslThread Thread, const TimeValue* pDelay)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments?*/
    if (pThreadImpl == NULL || PTHREAD_VALUE(pThreadImpl->m_hThread)==0 || pDelay==0)
        return osl_Thread_SleepError;

    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SLEEP)
        return osl_Thread_SleepActive;

    if (pthread_equal(pThreadImpl->m_hThread, pthread_self()))
    {
        int ret;
        struct timeval  now;
        struct timespec delay;
        int nOk=0;

        pthread_mutex_lock(&pThreadImpl->m_HandleLock);
        pThreadImpl->m_Timeout = 0;
        pThreadImpl->m_Flags |=  THREADIMPL_FLAGS_SLEEP;
        pthread_mutex_unlock(&pThreadImpl->m_HandleLock);

        gettimeofday(&now, NULL);

        SET_TIMESPEC(delay, now.tv_sec + (unsigned long) pDelay->Seconds,
                      (now.tv_usec * 1000) + (unsigned long) pDelay->Nanosec);

        while ( nOk == 0 )
        {
            pthread_mutex_lock(&pThreadImpl->m_AccessLock);

            ret = pthread_cond_timedwait(&pThreadImpl->m_Suspend, &pThreadImpl->m_AccessLock, &delay);
            if ( ret != EINTR )
            {
                nOk=1;
            }
            pthread_mutex_unlock(&pThreadImpl->m_AccessLock);
        }

        pthread_mutex_lock(&pThreadImpl->m_HandleLock);
        pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_SLEEP;
        pthread_mutex_unlock(&pThreadImpl->m_HandleLock);

        return (ret == ETIME || ret == ETIMEDOUT ) ? osl_Thread_SleepNormal :
                                osl_Thread_SleepCancel;
    }
    else
    {
        pthread_mutex_lock(&pThreadImpl->m_HandleLock);

        pThreadImpl->m_Timeout = pDelay->Seconds*1000+pDelay->Nanosec/1000000;
        pThreadImpl->m_Flags |=  THREADIMPL_FLAGS_SLEEP;

        pthread_mutex_unlock(&pThreadImpl->m_HandleLock);

        return osl_Thread_SleepPending;
    }
}

/*****************************************************************************/
/* osl_awakeThread */
/*****************************************************************************/
sal_Bool SAL_CALL osl_awakeThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments?*/
    if (pThreadImpl==NULL || PTHREAD_VALUE(pThreadImpl->m_hThread)==0)
    {
        /* assume thread is not running */
        return sal_False;
    }

    pthread_mutex_lock(&pThreadImpl->m_HandleLock);

    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SLEEP)
        pthread_cond_signal(&pThreadImpl->m_Suspend);

    pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_SLEEP;

    pthread_mutex_unlock(&pThreadImpl->m_HandleLock);

    return sal_True;
}

/*****************************************************************************/
/* osl_waitThread */
/*****************************************************************************/
void SAL_CALL osl_waitThread(const TimeValue* pDelay)
{
    if (pDelay)
    {
        struct timespec delay;

        SET_TIMESPEC(delay, pDelay->Seconds, pDelay->Nanosec);

        SLEEP_TIMESPEC(delay);
    }
}

/*****************************************************************************/
/* osl_terminateThread */
/*****************************************************************************/
void SAL_CALL osl_terminateThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;


    OSL_ASSERT(pThreadImpl);        /* valid ptr? */

    /* invalid arguments?*/
    if (pThreadImpl==NULL || PTHREAD_VALUE(pThreadImpl->m_hThread)==0)
    {
        /* assume thread is not running */
        return;
    }

    pthread_mutex_lock(&pThreadImpl->m_HandleLock);

    pThreadImpl->m_Flags |= THREADIMPL_FLAGS_TERMINATE;

    if (pThreadImpl->m_Flags & (THREADIMPL_FLAGS_SLEEP | THREADIMPL_FLAGS_SUSPENDED))
    {
        pthread_cond_signal(&pThreadImpl->m_Suspend);
    }

    pthread_mutex_unlock(&pThreadImpl->m_HandleLock);
}

/*****************************************************************************/
/* osl_scheduleThread */
/*****************************************************************************/
sal_Bool SAL_CALL osl_scheduleThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

/*  OSL_ASSERT(pThreadImpl);*/      /* valid ptr? */

    /* invalid arguments?*/
    if (pThreadImpl==0 || PTHREAD_VALUE(pThreadImpl->m_hThread)==0)
    {
        /* assume thread is not running */
        return sal_False;
    }

    OSL_ASSERT(pthread_equal(pThreadImpl->m_hThread, pthread_self()));


    pthread_testcancel();

    pthread_mutex_lock(&pThreadImpl->m_HandleLock);

    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SUSPENDED)
    {
        pthread_mutex_lock(&pThreadImpl->m_AccessLock);
        pthread_cond_wait(&pThreadImpl->m_Suspend, &pThreadImpl->m_AccessLock);
        pthread_mutex_unlock(&pThreadImpl->m_AccessLock);
    }

    pthread_mutex_unlock(&pThreadImpl->m_HandleLock);

    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SLEEP)
    {
        int ret;
        struct timeval  now;
        struct timespec delay;
        int nOk=0;

        gettimeofday(&now, NULL);

        SET_TIMESPEC(delay, now.tv_sec + pThreadImpl->m_Timeout / 1000,
                     now.tv_usec * 1000 + (pThreadImpl->m_Timeout % 1000) * 1000000);

        while ( nOk == 0 )
        {
            pthread_mutex_lock(&pThreadImpl->m_AccessLock);

            ret=pthread_cond_timedwait(&pThreadImpl->m_Suspend, &pThreadImpl->m_AccessLock, &delay);
            if ( ret != EINTR )
            {
                nOk=1;
            }
            pthread_mutex_unlock(&pThreadImpl->m_AccessLock);
        }

        pthread_mutex_lock(&pThreadImpl->m_HandleLock);
        pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_SLEEP;
        pthread_mutex_unlock(&pThreadImpl->m_HandleLock);
    }


    pthread_testcancel();

    return ((pThreadImpl->m_Flags & THREADIMPL_FLAGS_TERMINATE) == 0);
}

/*****************************************************************************/
/* osl_yieldThread */
/*
    Note that POSIX scheduling _really_ requires threads to call this
    functions, since a thread only reschedules to other thread, when
    it blocks (sleep, blocking I/O) OR calls sched_yield().
*/
/*****************************************************************************/
void SAL_CALL osl_yieldThread()
{
    sched_yield();
}

/*****************************************************************************/
/* osl_createThreadKey */
/*****************************************************************************/
oslThreadKey SAL_CALL osl_createThreadKey( oslThreadKeyCallbackFunction pCallback )
{
    pthread_key_t key;

    if (pthread_key_create(&key, pCallback) != 0)
        key = 0;

    return ((oslThreadKey)key);
}

/*****************************************************************************/
/* osl_destroyThreadKey */
/*****************************************************************************/
void SAL_CALL osl_destroyThreadKey(oslThreadKey Key)
{
    pthread_key_delete((pthread_key_t)Key);
}

/*****************************************************************************/
/* osl_getThreadKeyData */
/*****************************************************************************/
void* SAL_CALL osl_getThreadKeyData(oslThreadKey Key)
{
    return (pthread_getspecific((pthread_key_t)Key));
}

/*****************************************************************************/
/* osl_setThreadKeyData */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setThreadKeyData(oslThreadKey Key, void *pData)
{
    return (pthread_setspecific((pthread_key_t)Key, pData) == 0);
}


/*****************************************************************************/
/* osl_getThreadTextEncoding */
/*****************************************************************************/
rtl_TextEncoding SAL_CALL osl_getThreadTextEncoding()
{
    /* FIXME: mfe : osl_getThreadTextEncoding
     *     This function has to return a Thread Specific
     *     Encoding.
     *     It is yet to be implemented.
     */
    return RTL_TEXTENCODING_ISO_8859_1;
}


/*****************************************************************************/
/* osl_setThreadTextEncoding */
/*****************************************************************************/
rtl_TextEncoding osl_setThreadTextEncoding(rtl_TextEncoding Encoding)
{
    /* FIXME: mfe : osl_setThreadTextEncoding
     *     This function has to set a Thread Specific
     *     Encoding.
     *     It is yet to be implemented.
     */

    return RTL_TEXTENCODING_ISO_8859_1;
}


