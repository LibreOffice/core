/*************************************************************************
 *
 *  $RCSfile: thread.c,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: mfe $ $Date: 2001-02-14 17:41:16 $
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


/* MFE: just for the encoding stuff */

#include <rtl/memory.h>
#include <rtl/string.h>
#include <rtl/tencinfo.h>
#include <locale.h>
#ifndef MACOSX
#include <langinfo.h>
#endif

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

        ++ LastIdent;

        if ( LastIdent == 0 )
            LastIdent = 1;

        pEntry->Ident  = LastIdent;

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

#if defined (SOLARIS)
    if ( policy >= _SCHED_NEXT)
    {
        /* mfe: pthread_getschedparam on Solaris has a possible Bug */
        /*      one gets 959917873 as the policy                    */
        /*      so set the policy to a default one                  */
        policy=SCHED_OTHER;
    }
#endif

    if ( ( nRet = sched_get_priority_min(policy) ) >= 0 )
    {
        OSL_TRACE("Min Prioriy for policy '%i' == '%i'\n",policy,nRet);
        Thread_Prio_Lowest=nRet;
    }
#if defined(DEBUG)
    else
    {
        fprintf(stderr,"failed to get min sched param [%s]\n",strerror(errno));
    }
#endif

    if ( ( nRet = sched_get_priority_max(policy) ) >= 0 )
    {
        OSL_TRACE("Max Prioriy for policy '%i' == '%i'\n",policy,nRet);
        Thread_Prio_Highest=nRet;
    }
#if defined(DEBUG)
    else
    {
        fprintf(stderr,"failed to get max sched param [%s]\n",strerror(errno));
    }
#endif

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
#if defined (SOLARIS)
    if ( policy >= _SCHED_NEXT)
    {
        /* mfe: pthread_getschedparam on Salris has a possible Bug */
        /*      one gets 959917873 as the policy                   */
        /*      so set the policy to a default one                 */
        policy=SCHED_RR;
    }
#endif /* SOLARIS */

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
            /* enum expanded, but forgotten here...*/
            OSL_ENSURE(sal_False,"osl_setThreadPriority : unknown priority\n");

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
    oslThreadPriority Priority = osl_Thread_PriorityUnknown;

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



/*
 *   MFE :
 *   And now ... the beautyful textencoding functions
 *   absolutely in the right place (grrr ...)
 */

/*
 *  FIXME: this has to be merged, better transfered, with/from the tools
 *         (but to the RIGHT place!!!
 */


static rtl_TextEncoding eImplCharSet = RTL_TEXTENCODING_DONTKNOW;
static rtl_TextEncoding eImplDefaultCharSet = RTL_TEXTENCODING_MS_1252;

#if defined(NETBSD) || defined(SCO)
static rtl_TextEncoding GetSystemCharSetFromSystemLanguage();
static rtl_TextEncoding GetSystemCharSetFromLocale( const char* pLocaleString );
static rtl_TextEncoding GetSystemCharSetFromEnvironment();
#endif /* if def NETBSD || SCO */

#ifdef UNX

typedef struct {
    const char              *key;
    const rtl_TextEncoding   value;
} _pair;

rtl_TextEncoding GetSystemCharsetFromNLLanginfo();
static int _pair_compare (const char *key, const _pair *pair);
static const _pair* _pair_search (const char *key, const _pair *base, unsigned int member );
#endif


static void osl_setSystemTextEncoding( rtl_TextEncoding eEncoding );
static rtl_TextEncoding osl_getSystemTextEncoding();
static int _pair_compare (const char *key, const _pair *pair);
static const _pair* _pair_search (const char *key, const _pair *base, unsigned int member );

/*****************************************************************************/
/* osl_getThreadTextEncoding */
/*****************************************************************************/
rtl_TextEncoding SAL_CALL osl_getThreadTextEncoding()
{
    rtl_TextEncoding Encoding;
    Encoding =  osl_getSystemTextEncoding();
/*    OSL_TRACE("osl_getThreadTextEncoding : returning  '%i",Encoding);*/
    return Encoding;
}


/*****************************************************************************/
/* osl_setThreadTextEncoding */
/***c**************************************************************************/
rtl_TextEncoding osl_setThreadTextEncoding(rtl_TextEncoding Encoding)
{
    rtl_TextEncoding aOldEncoding = eImplCharSet;
    osl_setSystemTextEncoding(Encoding);

    return aOldEncoding;
}


static void osl_setSystemTextEncoding( rtl_TextEncoding eEncoding )
{
    eImplCharSet = eEncoding;
}

static rtl_TextEncoding osl_getSystemTextEncoding()
{
    rtl_TextEncoding nTextEncoding;

    if ( eImplCharSet == RTL_TEXTENCODING_DONTKNOW )
    {
        const char* pName = getenv("LC_CHARSET");
        if ( pName )
        {
            eImplCharSet = rtl_getTextEncodingFromUnixCharset( pName );
            OSL_TRACE("osl_getSystemTextEncoding : encoding is '%i'",eImplCharSet);
            if ( eImplCharSet != RTL_TEXTENCODING_DONTKNOW )
                return eImplCharSet;
        }


#if defined(NETBSD) || defined(SCO)
        nTextEncoding = GetSystemCharSetFromEnvironment();
        if (nTextEncoding == RTL_TEXTENCODING_DONTKNOW)
            nTextEncoding = GetSystemCharSetFromSystemLanguage();
#elif defined(LINUX) || defined(SOLARIS)
        nTextEncoding = GetSystemCharsetFromNLLanginfo();
#elif defined(MACOSX)
        nTextEncoding = RTL_TEXTENCODING_DONTKNOW;
#endif
        if ( nTextEncoding == RTL_TEXTENCODING_DONTKNOW )
            nTextEncoding = RTL_TEXTENCODING_ISO_8859_1;
        eImplCharSet = nTextEncoding;

        OSL_TRACE("osl_getSystemTextEncoding is now '%i'",eImplCharSet);

#ifdef MAC
#error "Now we need a implementation on the mac, because we support now more charsets"
#endif
    }

    if ( eImplCharSet == RTL_TEXTENCODING_DONTKNOW )
        return eImplDefaultCharSet;
    else
        return eImplCharSet;
}


#if defined(NETBSD) || defined(SCO)

/*
 *  gather from system language to what charset may adequate,
 *  in general only simple languages are taken into concern.
 *  but chinese and chinese-traditional are distinguished
 */

static rtl_TextEncoding GetSystemCharSetFromSystemLanguage()
{
    #define LANGUAGE_NULL LANGUAGE_NONE
    struct LanguagePool
    {
        const CharSet       nCharSet;  /* charset which belongs to planguage */
        const LanguageType *pLanguage; /* list of langs, */
                                       /* terminate with language_null */
    };

    const LanguageType pLangISO_8859_1 [] = {
        LANGUAGE_AFRIKAANS, LANGUAGE_BASQUE,    LANGUAGE_CATALAN,
        LANGUAGE_DANISH,    LANGUAGE_DUTCH,     LANGUAGE_ENGLISH,
        LANGUAGE_FINNISH,   LANGUAGE_FAEROESE,  LANGUAGE_FRENCH,
        LANGUAGE_GERMAN,    LANGUAGE_ICELANDIC, LANGUAGE_ITALIAN,
        LANGUAGE_NORWEGIAN, LANGUAGE_PORTUGUESE,LANGUAGE_SPANISH,
        LANGUAGE_SWEDISH,   LANGUAGE_NULL
    };
    const LanguageType pLangISO_8859_2 [] = {
        LANGUAGE_ALBANIAN,  LANGUAGE_CROATIAN,  LANGUAGE_CZECH,
        LANGUAGE_HUNGARIAN, LANGUAGE_POLISH,    LANGUAGE_ROMANIAN,
        LANGUAGE_SLOVAK,    LANGUAGE_SLOVENIAN, LANGUAGE_NULL
    };
    const LanguageType pLangISO_8859_4 [] = {
        LANGUAGE_ESTONIAN,  LANGUAGE_LATVIAN,   LANGUAGE_LITHUANIAN,
        LANGUAGE_NULL
    };
    const LanguageType pLangISO_8859_5 [] = {
        LANGUAGE_BELARUSIAN,LANGUAGE_BULGARIAN, LANGUAGE_MACEDONIAN,
        LANGUAGE_RUSSIAN,   LANGUAGE_UKRAINIAN, LANGUAGE_NULL
    };
    const LanguageType pLangISO_8859_6 [] = {
        LANGUAGE_ARABIC,    LANGUAGE_FARSI,     LANGUAGE_NULL
    };
    const LanguageType pLangISO_8859_7 [] = {
        LANGUAGE_GREEK,     LANGUAGE_NULL
    };
    const LanguageType pLangISO_8859_8 [] = {
        LANGUAGE_HEBREW,    LANGUAGE_NULL
    };
    const LanguageType pLangISO_8859_9 [] = {
        LANGUAGE_TURKISH,   LANGUAGE_NULL
    };
    const LanguageType pLangJapaneseEUC [] = {
        LANGUAGE_JAPANESE,  LANGUAGE_NULL
    };
    const LanguageType pLangChineseEUC [] = {
        LANGUAGE_CHINESE,   LANGUAGE_NULL
    };
    const LanguageType pLangTaiwanese [] = {
        LANGUAGE_CHINESE_TRADITIONAL, LANGUAGE_NULL
    };
#if (0)
    const LanguageType pTheseLangIDontKnow [] = {
        LANGUAGE_INDONESIAN,LANGUAGE_KOREAN, LANGUAGE_KOREAN_JOHAB,
        LANGUAGE_THAI,      LANGUAGE_VIETNAMESE,
        LANGUAGE_MALAY,     LANGUAGE_RHAETO_ROMAN,
        LANGUAGE_SORBIAN,   LANGUAGE_URDU
    };
#endif

    const LanguagePool aLanguagePool [] = {
        { RTL_TEXTENCODING_ISO_8859_1,  pLangISO_8859_1     },
        { RTL_TEXTENCODING_ISO_8859_2,  pLangISO_8859_2     },
        { RTL_TEXTENCODING_ISO_8859_4,  pLangISO_8859_4     },
        { RTL_TEXTENCODING_ISO_8859_5,  pLangISO_8859_5     },
        { RTL_TEXTENCODING_ISO_8859_6,  pLangISO_8859_6     },
        { RTL_TEXTENCODING_ISO_8859_7,  pLangISO_8859_7     },
        { RTL_TEXTENCODING_ISO_8859_8,  pLangISO_8859_8     },
        { RTL_TEXTENCODING_ISO_8859_9,  pLangISO_8859_9     },
        { RTL_TEXTENCODING_EUC_CN,      pLangChineseEUC     },
        { RTL_TEXTENCODING_EUC_JP,      pLangJapaneseEUC    },
        { RTL_TEXTENCODING_EUC_TW,      pLangTaiwanese      }
    };

    /* get the system language, be indefferent to cultural finenesses */
    LanguageType nFullLanguage    = GetSystemLanguage(
                                                INTERNATIONAL_SYSTEM_DEFAULT );
    LanguageType nSimpleLanguage  = nFullLanguage;
    if ( nSimpleLanguage != LANGUAGE_CHINESE_TRADITIONAL )
        nSimpleLanguage = International::GetNeutralLanguage( nFullLanguage );

    /* loop through the list of language lists */
    /* and try to match the system language */
    const int nPoolElements = sizeof(aLanguagePool) / sizeof(LanguagePool);
    for ( int i = 0; i < nPoolElements; i++ )
    {
        for (int j = 0; aLanguagePool[ i ].pLanguage[ j ] != LANGUAGE_NULL; j++)
        {
            if ( aLanguagePool[ i ].pLanguage[ j ] == nSimpleLanguage )
                return aLanguagePool[ i ].nCharSet;
        }
    }

    return RTL_TEXTENCODING_DONTKNOW;
}

/*
 *  check if there is a charset qualifier at the end of the given locale string
 *  e.g. de.ISO8859-15 or de.ISO8859-15@euro which strongly indicates what
 *  charset to use
 */
static rtl_TextEncoding GetSystemCharSetFromLocale( const char* pLocaleString )
{
    struct LocaleExtension {
        const sal_Char*         pName;
        const int               nNameLen;
        const rtl_TextEncoding  nCharSet;
    };

    #define RTL_TEXTENCODING_EUC RTL_TEXTENCODING_EUC_JP
    LocaleExtension const pLocaleExtension[] = {
        { "iso8859-15", sizeof("iso8859-15")- 1, RTL_TEXTENCODING_ISO_8859_15 },
        { "iso8859-1",  sizeof("iso8859-1") - 1, RTL_TEXTENCODING_ISO_8859_1  },
        { "iso8859-2",  sizeof("iso8859-2") - 1, RTL_TEXTENCODING_ISO_8859_2  },
        { "iso8859-3",  sizeof("iso8859-3") - 1, RTL_TEXTENCODING_ISO_8859_3  },
        { "iso8859-4",  sizeof("iso8859-4") - 1, RTL_TEXTENCODING_ISO_8859_4  },
        { "iso8859-5",  sizeof("iso8859-5") - 1, RTL_TEXTENCODING_ISO_8859_5  },
        { "iso8859-6",  sizeof("iso8859-6") - 1, RTL_TEXTENCODING_ISO_8859_6  },
        { "iso8859-7",  sizeof("iso8859-7") - 1, RTL_TEXTENCODING_ISO_8859_7  },
        { "iso8859-8",  sizeof("iso8859-8") - 1, RTL_TEXTENCODING_ISO_8859_8  },
        { "iso8859-9",  sizeof("iso8859-9") - 1, RTL_TEXTENCODING_ISO_8859_9  },
        { "utf-7",      sizeof("utf-7")     - 1, RTL_TEXTENCODING_UTF7      },
        { "utf-8",      sizeof("utf-8")     - 1, RTL_TEXTENCODING_UTF8      },
        { "utf-16",     sizeof("utf-16")    - 1, RTL_TEXTENCODING_UNICODE   },
        { "euc",        sizeof("euc")       - 1, RTL_TEXTENCODING_EUC       },
        { "pck",        sizeof("pck")       - 1, RTL_TEXTENCODING_MS_932    },
        { "koi8-r",     sizeof("koi8-r")    - 1, RTL_TEXTENCODING_KOI8_R    },
        { "big5",       sizeof("big5")      - 1, RTL_TEXTENCODING_BIG5      },
#if (0)
        { "sun_eu_greek",sizeof("sun_eu_greek")-1, RTL_TEXTENCODING_DONTKNOW },
#endif
        { NULL,         0,                       RTL_TEXTENCODING_DONTKNOW  }
    };

    /* get the charset qualifier */
    const char* pLocaleCharset;
    pLocaleCharset = strchr( pLocaleString, '.' );
    if ( pLocaleCharset == NULL || *(++pLocaleCharset) == '\0' )
        return RTL_TEXTENCODING_DONTKNOW;

    /* loop through the list, */
    /* compare the charset qualifier with the pName list member */
    for ( int i = 0; pLocaleExtension[ i ].pName != NULL; i++ )
    {
        if ( strncasecmp(pLocaleExtension[ i ].pName, pLocaleCharset,
                         pLocaleExtension[ i ].nNameLen) == 0 )
        {
            if ( pLocaleExtension[ i ].nCharSet != RTL_TEXTENCODING_EUC )
            {
                /* direct match */
                return pLocaleExtension[ i ].nCharSet;
            }
            else
            {
                /* special handling for japanese/chinese/thaiwanese euc */
                LanguageType nSimpleLanguage;

                nSimpleLanguage = GetSystemLanguage();
                if ( nSimpleLanguage != LANGUAGE_CHINESE_TRADITIONAL )
                    nSimpleLanguage = International::GetNeutralLanguage( nSimpleLanguage );
                if ( nSimpleLanguage == LANGUAGE_CHINESE_TRADITIONAL )
                    return RTL_TEXTENCODING_EUC_TW;
                if ( nSimpleLanguage == LANGUAGE_CHINESE )
                    return RTL_TEXTENCODING_EUC_CN;
                if ( nSimpleLanguage == LANGUAGE_JAPANESE )
                    return RTL_TEXTENCODING_EUC_JP;
            }
        }
    }

    /* nothing valid found */
    return RTL_TEXTENCODING_DONTKNOW;
}

/* evaluate the environment to guess what charset to choose */

static rtl_TextEncoding GetSystemCharSetFromEnvironment()
{
    const char* pEnvironmentPtr;

    /*
     *  check for encoding specification in public environment
     *  variables lc_ctype or lang, this does not check language itself
     *  but any charset extension
     */
    pEnvironmentPtr = getenv( "LANG" );
    if ( pEnvironmentPtr == NULL )
        pEnvironmentPtr = getenv( "LC_CTYPE" );
    if ( pEnvironmentPtr == NULL )
        return RTL_TEXTENCODING_DONTKNOW;
    return GetSystemCharSetFromLocale( pEnvironmentPtr );
}

#endif /* ifdef NETBSD || SCO */



#if defined(LINUX) || defined(SOLARIS)

/*
 * rtl_getTextEncodingFromLanguage maps from nl_langinfo(CODESET) to
 * rtl_textencoding defines. nl_langinfo() is supported only on Linux
 * and Solaris. This routine is SLOW because of the setlocale call, so
 * grab the result and cache it.
 * XXX this code has the usual mt problems aligned with setlocale() XXX
 */

#ifdef LINUX
#if !defined(CODESET)
#define CODESET _NL_CTYPE_CODESET_NAME
#endif
#endif


#if defined(SOLARIS)

const _pair _nl_language_list[] = {
    { "5601",           RTL_TEXTENCODING_EUC_KR         }, /* ko_KR.EUC */
    { "646",            RTL_TEXTENCODING_ISO_8859_1     }, /* fake: ASCII_US */
    { "ANSI-1251",      RTL_TEXTENCODING_MS_1251        }, /* ru_RU.ANSI1251 */
    { "BIG5",           RTL_TEXTENCODING_BIG5           },
    { "CNS11643",       RTL_TEXTENCODING_EUC_TW         }, /* zh_TW.EUC */
    { "EUCJP",          RTL_TEXTENCODING_EUC_JP         },
    { "GB2312",         RTL_TEXTENCODING_EUC_CN         }, /* zh_CN.EUC */
    { "GBK",            RTL_TEXTENCODING_GBK            }, /* zh_CN.GBK */
    { "ISO8859-1",      RTL_TEXTENCODING_ISO_8859_1     },
    { "ISO8859-13",     RTL_TEXTENCODING_DONTKNOW       }, /* lt_LT lv_LV */
    { "ISO8859-14",     RTL_TEXTENCODING_ISO_8859_14    },
    { "ISO8859-15",     RTL_TEXTENCODING_ISO_8859_15    },
    { "ISO8859-2",      RTL_TEXTENCODING_ISO_8859_2     },
    { "ISO8859-3",      RTL_TEXTENCODING_ISO_8859_3     },
    { "ISO8859-4",      RTL_TEXTENCODING_ISO_8859_4     },
    { "ISO8859-5",      RTL_TEXTENCODING_ISO_8859_5     },
    { "ISO8859-6",      RTL_TEXTENCODING_ISO_8859_6     },
    { "ISO8859-7",      RTL_TEXTENCODING_ISO_8859_7     },
    { "ISO8859-8",      RTL_TEXTENCODING_ISO_8859_8     },
    { "ISO8859-9",      RTL_TEXTENCODING_ISO_8859_9     },
    { "KOI8-R",         RTL_TEXTENCODING_KOI8_R         },
    { "PCK",            RTL_TEXTENCODING_MS_932         },
    { "SUN_EU_GREEK",   RTL_TEXTENCODING_ISO_8859_7     }, /* 8859-7 + Euro */
    { "TIS620.2533",    RTL_TEXTENCODING_MS_874         }, /* th_TH.TIS620 */
    { "UTF-8",          RTL_TEXTENCODING_UTF8           }
};

/* XXX MS-874 is an extension to tis620, so this is not
 * really equivalent */

#elif defined(LINUX)

const _pair _nl_language_list[] = {
    { "ANSI_X3.110-1983",   RTL_TEXTENCODING_DONTKNOW },/* ISO-IR-99 NAPLPS */
    { "ANSI_X3.4-1968", RTL_TEXTENCODING_ISO_8859_1 },  /* fake: ASCII_US */
    { "ASMO_449",       RTL_TEXTENCODING_DONTKNOW },    /* ISO_9036 ARABIC7 */
    { "BALTIC",         RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-179 */
    { "BS_4730",        RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-4 ISO646-GB */
    { "BS_VIEWDATA",    RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-47 */
    { "CP1250",         RTL_TEXTENCODING_MS_1250 },     /* MS-EE */
    { "CP1251",         RTL_TEXTENCODING_MS_1251 },     /* MS-CYRL */
    { "CP1252",         RTL_TEXTENCODING_MS_1252 },     /* MS-ANSI */
    { "CP1253",         RTL_TEXTENCODING_MS_1253 },     /* MS-GREEK */
    { "CP1254",         RTL_TEXTENCODING_MS_1254 },     /* MS-TURK */
    { "CP1255",         RTL_TEXTENCODING_MS_1255 },     /* MS-HEBR */
    { "CP1256",         RTL_TEXTENCODING_MS_1256 },     /* MS-ARAB */
    { "CP1257",         RTL_TEXTENCODING_MS_1257 },     /* WINBALTRIM */
    { "CSA_Z243.4-1985-1",  RTL_TEXTENCODING_DONTKNOW },/* ISO-IR-121 */
    { "CSA_Z243.4-1985-2",  RTL_TEXTENCODING_DONTKNOW },/* ISO-IR-122 CSA7-2 */
    { "CSA_Z243.4-1985-GR", RTL_TEXTENCODING_DONTKNOW },/* ISO-IR-123 */
    { "CSN_369103",     RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-139 */
    { "CWI",            RTL_TEXTENCODING_DONTKNOW },    /* CWI-2 CP-HU */
    { "DEC-MCS",        RTL_TEXTENCODING_DONTKNOW },    /* DEC */
    { "DIN_66003",      RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-21 */
    { "DS_2089",        RTL_TEXTENCODING_DONTKNOW },    /* DS2089 ISO646-DK */
    { "EBCDIC-AT-DE",   RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-AT-DE-A", RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-CA-FR",   RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-DK-NO",   RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-DK-NO-A", RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-ES",      RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-ES-A",    RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-ES-S",    RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-FI-SE",   RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-FI-SE-A", RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-FR",      RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-IS-FRISS",    RTL_TEXTENCODING_DONTKNOW },/*  FRISS */
    { "EBCDIC-IT",      RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-PT",      RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-UK",      RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-US",      RTL_TEXTENCODING_DONTKNOW },
    { "ECMA-CYRILLIC",  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-111 */
    { "ES",             RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-17 */
    { "ES2",            RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-85 */
    { "GB_1988-80",     RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-57 */
    { "GOST_19768-74",  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-153 */
    { "GREEK-CCITT",    RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-150 */
    { "GREEK7",         RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-88 */
    { "GREEK7-OLD",     RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-18 */
    { "HP-ROMAN8",      RTL_TEXTENCODING_DONTKNOW },    /* ROMAN8 R8 */
    { "IBM037",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-[US|CA|WT] */
    { "IBM038",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-INT CP038 */
    { "IBM1004",        RTL_TEXTENCODING_DONTKNOW },    /* CP1004 OS2LATIN1 */
    { "IBM1026",        RTL_TEXTENCODING_DONTKNOW },    /* CP1026 1026 */
    { "IBM1047",        RTL_TEXTENCODING_DONTKNOW },    /* CP1047 1047 */
    { "IBM256",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-INT1 */
    { "IBM273",         RTL_TEXTENCODING_DONTKNOW },    /* CP273 */
    { "IBM274",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-BE CP274 */
    { "IBM275",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-BR CP275 */
    { "IBM277",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CP-[DK|NO] */
    { "IBM278",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CP-[FISE]*/
    { "IBM280",         RTL_TEXTENCODING_DONTKNOW },    /* CP280 EBCDIC-CP-IT*/
    { "IBM281",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-JP-E CP281 */
    { "IBM284",         RTL_TEXTENCODING_DONTKNOW },    /* CP284 EBCDIC-CP-ES */
    { "IBM285",         RTL_TEXTENCODING_DONTKNOW },    /* CP285 EBCDIC-CP-GB */
    { "IBM290",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-JP-KANA */
    { "IBM297",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CP-FR */
    { "IBM420",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CP-AR1 */
    { "IBM423",         RTL_TEXTENCODING_DONTKNOW },    /* CP423 EBCDIC-CP-GR */
    { "IBM424",         RTL_TEXTENCODING_DONTKNOW },    /* CP424 EBCDIC-CP-HE */
    { "IBM437",         RTL_TEXTENCODING_IBM_437 },     /* CP437 437 */
    { "IBM500",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CP-[BE|CH] */
    { "IBM850",         RTL_TEXTENCODING_IBM_850 },     /* CP850 850 */
    { "IBM851",         RTL_TEXTENCODING_DONTKNOW },    /* CP851 851 */
    { "IBM852",         RTL_TEXTENCODING_IBM_852 },     /* CP852 852 */
    { "IBM855",         RTL_TEXTENCODING_IBM_855 },     /* CP855 855 */
    { "IBM857",         RTL_TEXTENCODING_IBM_857 },     /* CP857 857 */
    { "IBM860",         RTL_TEXTENCODING_IBM_860 },     /* CP860 860 */
    { "IBM861",         RTL_TEXTENCODING_IBM_861 },     /* CP861 861 CP-IS */
    { "IBM862",         RTL_TEXTENCODING_IBM_862 },     /* CP862 862 */
    { "IBM863",         RTL_TEXTENCODING_IBM_863 },     /* CP863 863 */
    { "IBM864",         RTL_TEXTENCODING_IBM_864 },     /* CP864 */
    { "IBM865",         RTL_TEXTENCODING_IBM_865 },     /* CP865 865 */
    { "IBM866",         RTL_TEXTENCODING_IBM_866 },     /* CP866 866 */
    { "IBM868",         RTL_TEXTENCODING_DONTKNOW },    /* CP868 CP-AR */
    { "IBM869",         RTL_TEXTENCODING_IBM_869 },     /* CP869 869 CP-GR */
    { "IBM870",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-[ROECE|YU] */
    { "IBM871",         RTL_TEXTENCODING_DONTKNOW },    /* CP871 EBCDIC-CP-IS */
    { "IBM875",         RTL_TEXTENCODING_DONTKNOW },    /* CP875 EBCDIC-GREEK */
    { "IBM880",         RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CYRILLIC */
    { "IBM891",         RTL_TEXTENCODING_DONTKNOW },    /* CP891 */
    { "IBM903",         RTL_TEXTENCODING_DONTKNOW },    /* CP903 */
    { "IBM904",         RTL_TEXTENCODING_DONTKNOW },    /* CP904 904 */
    { "IBM905",         RTL_TEXTENCODING_DONTKNOW },    /* CP905 EBCDIC-CP-TR */
    { "IBM918",         RTL_TEXTENCODING_DONTKNOW },    /* CP918 EBCDIC-AR2 */
    { "IEC_P27-1",      RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-143 */
    { "INIS",           RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-49 */
    { "INIS-8",         RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-50 */
    { "INIS-CYRILLIC",  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-51 */
    { "INVARIANT",      RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-170 */
    { "ISO-8859-1",     RTL_TEXTENCODING_ISO_8859_1 },  /* ISO-IR-100 CP819 */
    { "ISO-8859-10",    RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-157 LATIN6 */
    { "ISO-8859-13",    RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-179 LATIN7 */
    { "ISO-8859-14",    RTL_TEXTENCODING_ISO_8859_14 }, /* LATIN8 L8 */
    { "ISO-8859-15",    RTL_TEXTENCODING_ISO_8859_15 },
    { "ISO-8859-2",     RTL_TEXTENCODING_ISO_8859_2 },  /* LATIN2 L2 */
    { "ISO-8859-3",     RTL_TEXTENCODING_ISO_8859_3 },  /* LATIN3 L3 */
    { "ISO-8859-4",     RTL_TEXTENCODING_ISO_8859_4 },  /* LATIN4 L4 */
    { "ISO-8859-5",     RTL_TEXTENCODING_ISO_8859_5 },  /* CYRILLIC */
    { "ISO-8859-6",     RTL_TEXTENCODING_ISO_8859_6 },  /* ECMA-114 ARABIC */
    { "ISO-8859-7",     RTL_TEXTENCODING_ISO_8859_7 },  /* ECMA-118 GREEK8 */
    { "ISO-8859-8",     RTL_TEXTENCODING_ISO_8859_8 },  /* ISO_8859-8 HEBREW */
    { "ISO-8859-9",     RTL_TEXTENCODING_ISO_8859_9 },  /* ISO_8859-9 LATIN5 */
    { "ISO-IR-90",      RTL_TEXTENCODING_DONTKNOW },    /* ISO_6937-2:1983 */
    { "ISO_10367-BOX",  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-155 */
    { "ISO_2033-1983",  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-98 E13B */
    { "ISO_5427",       RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-37 KOI-7 */
    { "ISO_5427-EXT",   RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-54  */
    { "ISO_5428",       RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-55 */
    { "ISO_646.BASIC",  RTL_TEXTENCODING_ASCII_US },    /* REF */
    { "ISO_646.IRV",    RTL_TEXTENCODING_ASCII_US },    /* ISO-IR-2 IRV */
    { "ISO_646.IRV:1983",   RTL_TEXTENCODING_ISO_8859_1 },/* fake: ASCII_US,
                                                           used for "C" locale*/
    { "ISO_6937",       RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-156 ISO6937*/
    { "ISO_6937-2-25",  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-152 */
    { "ISO_6937-2-ADD", RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-142 */
    { "ISO_8859-SUPP",  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-154 */
    { "IT",             RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-15  */
    { "JIS_C6220-1969-JP",  RTL_TEXTENCODING_DONTKNOW },/* KATAKANA X0201-7 */
    { "JIS_C6220-1969-RO",  RTL_TEXTENCODING_DONTKNOW }, /* ISO-IR-14 */
    { "JIS_C6229-1984-A",   RTL_TEXTENCODING_DONTKNOW }, /* ISO-IR-91 */
    { "JIS_C6229-1984-B",   RTL_TEXTENCODING_DONTKNOW }, /* ISO-IR-92 */
    { "JIS_C6229-1984-B-ADD",   RTL_TEXTENCODING_DONTKNOW }, /* ISO-IR-93 */
    { "JIS_C6229-1984-HAND",    RTL_TEXTENCODING_DONTKNOW }, /* ISO-IR-94 */
    { "JIS_C6229-1984-HAND-ADD",RTL_TEXTENCODING_DONTKNOW }, /* ISO-IR-95 */
    { "JIS_C6229-1984-KANA",    RTL_TEXTENCODING_DONTKNOW }, /* ISO-IR-96 */
    { "JIS_X0201",      RTL_TEXTENCODING_DONTKNOW },    /* X0201 */
    { "JUS_I.B1.002",   RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-141 */
    { "JUS_I.B1.003-MAC",   RTL_TEXTENCODING_DONTKNOW },/* MACEDONIAN */
    { "JUS_I.B1.003-SERB",  RTL_TEXTENCODING_DONTKNOW },/* ISO-IR-146 SERBIAN */
    { "KOI-8",          RTL_TEXTENCODING_DONTKNOW },
    { "KOI8-R",         RTL_TEXTENCODING_KOI8_R },
    { "KOI8-U",         RTL_TEXTENCODING_DONTKNOW },
    { "KSC5636",        RTL_TEXTENCODING_DONTKNOW },    /* ISO646-KR */
    { "LATIN-GREEK",    RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-19 */
    { "LATIN-GREEK-1",  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-27 */
    { "MAC-IS",         RTL_TEXTENCODING_APPLE_ROMAN },
    { "MAC-UK",         RTL_TEXTENCODING_APPLE_ROMAN },
    { "MACINTOSH",      RTL_TEXTENCODING_APPLE_ROMAN }, /* MAC */
    { "MSZ_7795.3",     RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-86 */
    { "NATS-DANO",      RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-9-1 */
    { "NATS-DANO-ADD",  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-9-2 */
    { "NATS-SEFI",      RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-8-1 */
    { "NATS-SEFI-ADD",  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-8-2 */
    { "NC_NC00-10",     RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-151 */
    { "NEXTSTEP",       RTL_TEXTENCODING_DONTKNOW },    /* NEXT */
    { "NF_Z_62-010",    RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-69 */
    { "NF_Z_62-010_(1973)", RTL_TEXTENCODING_DONTKNOW },/* ISO-IR-25 */
    { "NS_4551-1",      RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-60 */
    { "NS_4551-2",      RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-61 */
    { "PT",             RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-16 */
    { "PT2",            RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-84 */
    { "SAMI",           RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-158 */
    { "SEN_850200_B",   RTL_TEXTENCODING_DONTKNOW },    /* ISO646-[FI|SE] */
    { "SEN_850200_C",   RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-11 */
    { "T.101-G2",       RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-128 */
    { "T.61-7BIT",      RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-102 */
    { "T.61-8BIT",      RTL_TEXTENCODING_DONTKNOW },    /* T.61 ISO-IR-103 */
    { "UTF-8",          RTL_TEXTENCODING_UTF8 },        /* ISO-10646/UTF-8 */
    { "VIDEOTEX-SUPPL", RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-70 */
    { "WIN-SAMI-2",     RTL_TEXTENCODING_DONTKNOW }     /* WS2 */
};
#endif /* ifdef LINUX */

static pthread_mutex_t aLocalMutex = PTHREAD_MUTEX_INITIALIZER;

rtl_TextEncoding
GetSystemCharsetFromNLLanginfo()
{
    const _pair *language=0;
    char  codeset_buf[64];

    char *ctype_locale = 0;
    char *codeset      = 0;

    /*
     *   basic thread safeness
     */
    pthread_mutex_lock(&aLocalMutex);

    /* get the charset as indicated by the LC_CTYPE locale */
    ctype_locale = setlocale( LC_CTYPE, "" );
    codeset    = nl_langinfo( CODESET );

    if ( codeset != NULL )
    {
        /* get codeset into mt save memory */
        rtl_copyMemory( codeset_buf, codeset, sizeof(codeset_buf) );
        codeset = codeset_buf;
    }

    /* restore the original value of locale */
    if ( ctype_locale != NULL )
        setlocale( LC_CTYPE, ctype_locale );

    /* search the codeset in our language list */
    if ( codeset != NULL )
    {
        const unsigned int members = sizeof(_nl_language_list) / sizeof(_pair);
        language = _pair_search (codeset, _nl_language_list, members);
    }

    /* a matching item in our list provides a mapping from codeset to
     * rtl-codeset */
    if ( language != NULL )
    {
        pthread_mutex_unlock(&aLocalMutex);
        return language->value;
    }

    pthread_mutex_unlock(&aLocalMutex);

    return RTL_TEXTENCODING_DONTKNOW;
}
#endif /* ifdef LINUX || SOLARIS */


static int
_pair_compare (const char *key, const _pair *pair)
{
    int result = rtl_str_compareIgnoreCase( key, pair->key );
    return result;
}

static const _pair*
_pair_search (const char *key, const _pair *base, unsigned int member )
{
    unsigned int lower = 0;
    unsigned int upper = member;
    unsigned int current;
    int comparison;

    /* check for validity of input */
    if ( (key == NULL) || (base == NULL) || (member == 0) )
        return NULL;

    /* binary search */
    while ( lower < upper )
    {
        current = (lower + upper) / 2;
        comparison = _pair_compare( key, base + current );
        if (comparison < 0)
            upper = current;
        else
        if (comparison > 0)
            lower = current + 1;
        else
            return base + current;
    }

    return NULL;
}
