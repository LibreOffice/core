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

#include "system.h"
#include <string.h>
#if defined(OPENBSD)
#include <sched.h>
#endif
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/nlsupport.h>
#include <rtl/textenc.h>
#include <rtl/alloc.h>
#include <sal/macros.h>
#ifdef ANDROID
#include <jni.h>
#include <android/log.h>
#include <osl/detail/android-bootstrap.h>
#endif

#if defined LINUX && ! defined __FreeBSD_kernel__
#include <sys/prctl.h>
#ifndef PR_SET_NAME
#define PR_SET_NAME 15
#endif
#endif

/****************************************************************************
 * @@@ TODO @@@
 *
 * (1) 'osl_thread_priority_init_Impl()'
 *     - insane assumption that initializing caller is main thread
 *     - use _POSIX_THREAD_PRIORITY_SCHEDULING, not NO_PTHREAD_PRIORITY (?)
 *     - POSIX doesn't require defined prio's for SCHED_OTHER (!)
 *     - use SCHED_RR instead of SCHED_OTHER for defined behaviour (?)
 * (2) 'oslThreadIdentifier' and '{insert|remove|lookup}ThreadId()'
 *     - cannot reliably be applied to 'alien' threads;
 *     - memory leak for 'alien' thread 'HashEntry's;
 *     - use 'PTHREAD_VALUE(pthread_t)' as identifier instead (?)
 *     - if yes, change 'oslThreadIdentifier' to 'intptr_t' or similar
 * (3) 'oslSigAlarmHandler()' (#71232#)
 *     - [Under Solaris we get SIGALRM in e.g. pthread_join which terminates
 *       the process. So we initialize our signal handling module and do
 *       register a SIGALRM Handler which catches and ignores it]
 *     - should this still happen, 'signal.c' needs to be fixed instead.
 *
 ****************************************************************************/

/*****************************************************************************/
/*  Internal data structures and functions */
/*****************************************************************************/

#define THREADIMPL_FLAGS_TERMINATE  0x00001
#define THREADIMPL_FLAGS_STARTUP    0x00002
#define THREADIMPL_FLAGS_SUSPENDED  0x00004
#define THREADIMPL_FLAGS_ACTIVE     0x00008
#define THREADIMPL_FLAGS_ATTACHED   0x00010
#define THREADIMPL_FLAGS_DESTROYED  0x00020

typedef struct osl_thread_impl_st
{
    pthread_t           m_hThread;
    sal_uInt16          m_Ident; /* @@@ see TODO @@@ */
    short               m_Flags;
    oslWorkerFunction   m_WorkerFunction;
    void*               m_pData;
    pthread_mutex_t     m_Lock;
    pthread_cond_t      m_Cond;
} Thread_Impl;

struct osl_thread_priority_st
{
    int m_Highest;
    int m_Above_Normal;
    int m_Normal;
    int m_Below_Normal;
    int m_Lowest;
};

#define OSL_THREAD_PRIORITY_INITIALIZER { 127, 96, 64, 32, 0 }
static void osl_thread_priority_init_Impl (void);

struct osl_thread_textencoding_st
{
    pthread_key_t    m_key;     /* key to store thread local text encoding */
    rtl_TextEncoding m_default; /* the default text encoding */
};

#define OSL_THREAD_TEXTENCODING_INITIALIZER { 0, RTL_TEXTENCODING_DONTKNOW }
static void osl_thread_textencoding_init_Impl (void);

struct osl_thread_global_st
{
    pthread_once_t                    m_once;
    struct osl_thread_priority_st     m_priority;
    struct osl_thread_textencoding_st m_textencoding;
};

static struct osl_thread_global_st g_thread =
{
    PTHREAD_ONCE_INIT,
    OSL_THREAD_PRIORITY_INITIALIZER,
    OSL_THREAD_TEXTENCODING_INITIALIZER
};

static void osl_thread_init_Impl (void);

static Thread_Impl* osl_thread_construct_Impl (void);
static void         osl_thread_destruct_Impl (Thread_Impl ** ppImpl);

static void* osl_thread_start_Impl (void * pData);
static void  osl_thread_cleanup_Impl (Thread_Impl * pImpl);

static oslThread osl_thread_create_Impl (
    oslWorkerFunction pWorker, void * pThreadData, short nFlags);

/* @@@ see TODO @@@ */
static sal_uInt16 insertThreadId (pthread_t hThread);
static sal_uInt16 lookupThreadId (pthread_t hThread);
static void       removeThreadId (pthread_t hThread);

/*****************************************************************************/
/* osl_thread_init_Impl */
/*****************************************************************************/
static void osl_thread_init_Impl (void)
{
    osl_thread_priority_init_Impl();
    osl_thread_textencoding_init_Impl();
}

/*****************************************************************************/
/* osl_thread_construct_Impl */
/*****************************************************************************/
Thread_Impl* osl_thread_construct_Impl (void)
{
    Thread_Impl* pImpl = malloc (sizeof(Thread_Impl));
    if (pImpl)
    {
        memset (pImpl, 0, sizeof(Thread_Impl));

        pthread_mutex_init (&(pImpl->m_Lock), PTHREAD_MUTEXATTR_DEFAULT);
        pthread_cond_init  (&(pImpl->m_Cond), PTHREAD_CONDATTR_DEFAULT);
    }
    return (pImpl);
}

/*****************************************************************************/
/* osl_thread_destruct_Impl */
/*****************************************************************************/
static void osl_thread_destruct_Impl (Thread_Impl ** ppImpl)
{
    OSL_ASSERT(ppImpl);
    if (*ppImpl)
    {
        pthread_cond_destroy  (&((*ppImpl)->m_Cond));
        pthread_mutex_destroy (&((*ppImpl)->m_Lock));

        free (*ppImpl);
        (*ppImpl) = 0;
    }
}

/*****************************************************************************/
/* osl_thread_cleanup_Impl */
/*****************************************************************************/
static void osl_thread_cleanup_Impl (Thread_Impl * pImpl)
{
    pthread_t thread;
    int attached;
    int destroyed;

    pthread_mutex_lock (&(pImpl->m_Lock));

    thread = pImpl->m_hThread;
    attached = (pImpl->m_Flags & THREADIMPL_FLAGS_ATTACHED) != 0;
    destroyed = (pImpl->m_Flags & THREADIMPL_FLAGS_DESTROYED) != 0;
    pImpl->m_Flags &= ~(THREADIMPL_FLAGS_ACTIVE | THREADIMPL_FLAGS_ATTACHED);

    pthread_mutex_unlock (&(pImpl->m_Lock));

    /* release oslThreadIdentifier @@@ see TODO @@@ */
    removeThreadId (thread);

    if (attached)
    {
        pthread_detach (thread);
    }

    if (destroyed)
    {
        osl_thread_destruct_Impl (&pImpl);
    }
}

/*****************************************************************************/
/* osl_thread_start_Impl */
/*****************************************************************************/
static void* osl_thread_start_Impl (void* pData)
{
    int terminate;
    Thread_Impl* pImpl= (Thread_Impl*)pData;

    OSL_ASSERT(pImpl);

    pthread_mutex_lock (&(pImpl->m_Lock));

    /* request oslThreadIdentifier @@@ see TODO @@@ */
    pImpl->m_Ident = insertThreadId (pImpl->m_hThread);

    /* signal change from STARTUP to ACTIVE state */
    pImpl->m_Flags &= ~THREADIMPL_FLAGS_STARTUP;
    pImpl->m_Flags |=  THREADIMPL_FLAGS_ACTIVE;
    pthread_cond_signal (&(pImpl->m_Cond));

    /* Check if thread is started in SUSPENDED state */
    while (pImpl->m_Flags & THREADIMPL_FLAGS_SUSPENDED)
    {
        /* wait until SUSPENDED flag is cleared */
        pthread_cond_wait (&(pImpl->m_Cond), &(pImpl->m_Lock));
    }

    /* check for SUSPENDED to TERMINATE state change */
    terminate = ((pImpl->m_Flags & THREADIMPL_FLAGS_TERMINATE) > 0);

    pthread_mutex_unlock (&(pImpl->m_Lock));

    if (!terminate)
    {
#ifdef ANDROID
        JNIEnv* env = 0;
        int res = (*lo_get_javavm())->AttachCurrentThread(lo_get_javavm(), &env, NULL);
        __android_log_print(ANDROID_LOG_INFO, "LibreOffice", "New sal thread started and attached res=%d", res);
#endif
        /* call worker function */
        pImpl->m_WorkerFunction(pImpl->m_pData);

#ifdef ANDROID
        res = (*lo_get_javavm())->DetachCurrentThread(lo_get_javavm());
        __android_log_print(ANDROID_LOG_INFO, "LibreOffice", "Detached finished sal thread res=%d", res);
#endif
    }

    osl_thread_cleanup_Impl (pImpl);
    return (0);
}

/*****************************************************************************/
/* osl_thread_create_Impl */
/*****************************************************************************/
static oslThread osl_thread_create_Impl (
    oslWorkerFunction pWorker,
    void*             pThreadData,
    short             nFlags)
{
    Thread_Impl* pImpl;
#if defined(OPENBSD)
    pthread_attr_t attr;
#endif
    int nRet=0;

    pImpl = osl_thread_construct_Impl();
    if (!pImpl)
        return (0); /* ENOMEM */

    pImpl->m_WorkerFunction = pWorker;
    pImpl->m_pData = pThreadData;
    pImpl->m_Flags = nFlags | THREADIMPL_FLAGS_STARTUP;

    pthread_mutex_lock (&(pImpl->m_Lock));

#if defined(OPENBSD)
    if (pthread_attr_init(&attr) != 0)
        return (0);

    if (pthread_attr_setstacksize(&attr, 262144) != 0) {
        pthread_attr_destroy(&attr);
        return (0);
    }
#endif

    if ((nRet = pthread_create (
        &(pImpl->m_hThread),
#if defined(OPENBSD)
        &attr,
#else
        PTHREAD_ATTR_DEFAULT,
#endif
        osl_thread_start_Impl,
        (void*)(pImpl))) != 0)
    {
        OSL_TRACE("osl_thread_create_Impl(): errno: %d, %s\n",
                  nRet, strerror(nRet));

        pthread_mutex_unlock (&(pImpl->m_Lock));
        osl_thread_destruct_Impl (&pImpl);

        return (0);
    }

#if defined(OPENBSD)
    pthread_attr_destroy(&attr);
#endif

    /* wait for change from STARTUP to ACTIVE state */
    while (pImpl->m_Flags & THREADIMPL_FLAGS_STARTUP)
    {
        /* wait until STARTUP flag is cleared */
        pthread_cond_wait (&(pImpl->m_Cond), &(pImpl->m_Lock));
    }

    pthread_mutex_unlock (&(pImpl->m_Lock));

    return ((oslThread)(pImpl));
}

/*****************************************************************************/
/* osl_createThread */
/*****************************************************************************/
oslThread osl_createThread (
    oslWorkerFunction pWorker,
    void *            pThreadData)
{
    return osl_thread_create_Impl (
        pWorker,
        pThreadData,
        THREADIMPL_FLAGS_ATTACHED);
}

/*****************************************************************************/
/* osl_createSuspendedThread */
/*****************************************************************************/
oslThread osl_createSuspendedThread (
    oslWorkerFunction pWorker,
    void *            pThreadData)
{
    return osl_thread_create_Impl (
        pWorker,
        pThreadData,
        THREADIMPL_FLAGS_ATTACHED |
        THREADIMPL_FLAGS_SUSPENDED );
}

/*****************************************************************************/
/* osl_destroyThread */
/*****************************************************************************/
void SAL_CALL osl_destroyThread(oslThread Thread)
{
    if (Thread != NULL) {
        Thread_Impl * impl = (Thread_Impl *) Thread;
        int active;
        pthread_mutex_lock(&impl->m_Lock);
        active = (impl->m_Flags & THREADIMPL_FLAGS_ACTIVE) != 0;
        impl->m_Flags |= THREADIMPL_FLAGS_DESTROYED;
        pthread_mutex_unlock(&impl->m_Lock);
        if (!active) {
            osl_thread_destruct_Impl(&impl);
        }
    }
}

/*****************************************************************************/
/* osl_resumeThread */
/*****************************************************************************/
void SAL_CALL osl_resumeThread(oslThread Thread)
{
    Thread_Impl* pImpl= (Thread_Impl*)Thread;

    OSL_ASSERT(pImpl);
    if (!pImpl)
        return; /* EINVAL */

    pthread_mutex_lock (&(pImpl->m_Lock));

    if (pImpl->m_Flags & THREADIMPL_FLAGS_SUSPENDED)
    {
        /* clear SUSPENDED flag */
        pImpl->m_Flags &= ~THREADIMPL_FLAGS_SUSPENDED;
        pthread_cond_signal (&(pImpl->m_Cond));
    }

    pthread_mutex_unlock (&(pImpl->m_Lock));
}

/*****************************************************************************/
/* osl_suspendThread */
/*****************************************************************************/
void SAL_CALL osl_suspendThread(oslThread Thread)
{
    Thread_Impl* pImpl= (Thread_Impl*)Thread;

    OSL_ASSERT(pImpl);
    if (!pImpl)
        return; /* EINVAL */

    pthread_mutex_lock (&(pImpl->m_Lock));

    pImpl->m_Flags |= THREADIMPL_FLAGS_SUSPENDED;

    if (pthread_equal (pthread_self(), pImpl->m_hThread))
    {
        /* self suspend */
        while (pImpl->m_Flags & THREADIMPL_FLAGS_SUSPENDED)
        {
            /* wait until SUSPENDED flag is cleared */
            pthread_cond_wait (&(pImpl->m_Cond), &(pImpl->m_Lock));
        }
    }

    pthread_mutex_unlock (&(pImpl->m_Lock));
}

/*****************************************************************************/
/* osl_isThreadRunning */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isThreadRunning(const oslThread Thread)
{
    sal_Bool active;
    Thread_Impl* pImpl= (Thread_Impl*)Thread;

    if (!pImpl)
        return sal_False;

    pthread_mutex_lock (&(pImpl->m_Lock));
    active = ((pImpl->m_Flags & THREADIMPL_FLAGS_ACTIVE) > 0);
    pthread_mutex_unlock (&(pImpl->m_Lock));

    return (active);
}

/*****************************************************************************/
/* osl_joinWithThread */
/*****************************************************************************/
void SAL_CALL osl_joinWithThread(oslThread Thread)
{
    pthread_t thread;
    int attached;
    Thread_Impl* pImpl= (Thread_Impl*)Thread;

    if (!pImpl)
        return;

    pthread_mutex_lock (&(pImpl->m_Lock));

    if (pthread_equal (pthread_self(), pImpl->m_hThread))
    {
        /* self join */
        pthread_mutex_unlock (&(pImpl->m_Lock));
        return; /* EDEADLK */
    }

    thread = pImpl->m_hThread;
    attached = ((pImpl->m_Flags & THREADIMPL_FLAGS_ATTACHED) > 0);
    pImpl->m_Flags &= ~THREADIMPL_FLAGS_ATTACHED;

    pthread_mutex_unlock (&(pImpl->m_Lock));

    if (attached)
    {
        pthread_join (thread, NULL);
    }
}

/*****************************************************************************/
/* osl_terminateThread */
/*****************************************************************************/
void SAL_CALL osl_terminateThread(oslThread Thread)
{
    Thread_Impl* pImpl= (Thread_Impl*)Thread;

    OSL_ASSERT(pImpl);
    if (!pImpl)
        return; /* EINVAL */

    pthread_mutex_lock (&(pImpl->m_Lock));

    if (pImpl->m_Flags & THREADIMPL_FLAGS_SUSPENDED)
    {
        /* clear SUSPENDED flag */
        pImpl->m_Flags &= ~THREADIMPL_FLAGS_SUSPENDED;
        pthread_cond_signal (&(pImpl->m_Cond));
    }

    pImpl->m_Flags |= THREADIMPL_FLAGS_TERMINATE;

    pthread_mutex_unlock (&(pImpl->m_Lock));
}

/*****************************************************************************/
/* osl_scheduleThread */
/*****************************************************************************/
sal_Bool SAL_CALL osl_scheduleThread(oslThread Thread)
{
    int terminate;
    Thread_Impl* pImpl= (Thread_Impl*)Thread;

    OSL_ASSERT(pImpl);
    if (!pImpl)
        return sal_False; /* EINVAL */

    OSL_ASSERT(pthread_equal (pthread_self(), pImpl->m_hThread));
    if (!(pthread_equal (pthread_self(), pImpl->m_hThread)))
        return sal_False; /* EINVAL */

    pthread_mutex_lock (&(pImpl->m_Lock));

    while (pImpl->m_Flags & THREADIMPL_FLAGS_SUSPENDED)
    {
        /* wait until SUSPENDED flag is cleared */
        pthread_cond_wait (&(pImpl->m_Cond), &(pImpl->m_Lock));
    }

    terminate = ((pImpl->m_Flags & THREADIMPL_FLAGS_TERMINATE) > 0);

    pthread_mutex_unlock(&(pImpl->m_Lock));

    return (terminate == 0);
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

void SAL_CALL osl_setThreadName(char const * name) {
#if defined LINUX && ! defined __FreeBSD_kernel__
    if (prctl(PR_SET_NAME, (unsigned long) name, 0, 0, 0) != 0) {
        OSL_TRACE(
            "%s prctl(PR_SET_NAME) failed with errno %d", OSL_LOG_PREFIX,
            errno);
    }
#else
    (void) name;
#endif
}

/*****************************************************************************/
/* osl_getThreadIdentifier @@@ see TODO @@@ */
/*****************************************************************************/

#define HASHID(x) ((unsigned int)PTHREAD_VALUE(x) % HashSize)

typedef struct _HashEntry
{
    pthread_t         Handle;
    sal_uInt16        Ident;
    struct _HashEntry *Next;
} HashEntry;

static HashEntry* HashTable[31];
static int HashSize = SAL_N_ELEMENTS(HashTable);

static pthread_mutex_t HashLock = PTHREAD_MUTEX_INITIALIZER;

static sal_uInt16 LastIdent = 0;

static sal_uInt16 lookupThreadId (pthread_t hThread)
{
    HashEntry *pEntry;

    pthread_mutex_lock(&HashLock);

        pEntry = HashTable[HASHID(hThread)];
        while (pEntry != NULL)
        {
            if (pthread_equal(pEntry->Handle, hThread))
            {
                pthread_mutex_unlock(&HashLock);
                return (pEntry->Ident);
            }
            pEntry = pEntry->Next;
        }

    pthread_mutex_unlock(&HashLock);

    return (0);
}

static sal_uInt16 insertThreadId (pthread_t hThread)
{
    HashEntry *pEntry, *pInsert = NULL;

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

static void removeThreadId (pthread_t hThread)
{
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

oslThreadIdentifier SAL_CALL osl_getThreadIdentifier(oslThread Thread)
{
    Thread_Impl* pImpl= (Thread_Impl*)Thread;
    sal_uInt16   Ident;

    if (pImpl)
        Ident = pImpl->m_Ident;
    else
    {
        /* current thread */
        pthread_t current = pthread_self();

        Ident = lookupThreadId (current);
        if (Ident == 0)
            /* @@@ see TODO: alien pthread_self() @@@ */
            Ident = insertThreadId (current);
    }

    return ((oslThreadIdentifier)(Ident));
}

/*****************************************************************************
    @@@ see TODO @@@
    osl_thread_priority_init_Impl

    set the base-priority of the main-thread to
    oslThreadPriorityNormal (64) since 0 (lowest) is
    the system default. This behaviour collides with
    our enum-priority definition (highest..normal..lowest).
    A  normaluser will expect the main-thread of an app.
    to have the "normal" priority.

*****************************************************************************/
static void osl_thread_priority_init_Impl (void)
{
#ifndef NO_PTHREAD_PRIORITY
    struct sched_param param;
    int policy=0;
    int nRet=0;

/* @@@ see TODO: calling thread may not be main thread @@@ */

    if ((nRet = pthread_getschedparam(pthread_self(), &policy, &param)) != 0)
    {
        OSL_TRACE("failed to get priority of thread [%s]",strerror(nRet));
        return;
    }

#if defined (SOLARIS)
    if ( policy >= _SCHED_NEXT)
    {
        /* mfe: pthread_getschedparam on Solaris has a possible Bug */
        /*      one gets 959917873 as the policy                    */
        /*      so set the policy to a default one                  */
        policy=SCHED_OTHER;
    }
#endif /* SOLARIS */

    if ((nRet = sched_get_priority_min(policy) ) != -1)
    {
        OSL_TRACE("Min Prioriy for policy '%i' == '%i'",policy,nRet);
        g_thread.m_priority.m_Lowest=nRet;
    }
#if OSL_DEBUG_LEVEL > 1
    else
    {
        fprintf(stderr,"failed to get min sched param [%s]\n",strerror(errno));
    }
#endif /* OSL_DEBUG_LEVEL */

    if ((nRet = sched_get_priority_max(policy) ) != -1)
    {
        OSL_TRACE("Max Prioriy for policy '%i' == '%i'",policy,nRet);
        g_thread.m_priority.m_Highest=nRet;
    }
#if OSL_DEBUG_LEVEL > 1
    else
    {
        fprintf(stderr,"failed to get max sched param [%s]\n",strerror(errno));
    }
#endif /* OSL_DEBUG_LEVEL */

    g_thread.m_priority.m_Normal =
        (g_thread.m_priority.m_Lowest + g_thread.m_priority.m_Highest) / 2;
    g_thread.m_priority.m_Below_Normal =
        (g_thread.m_priority.m_Lowest + g_thread.m_priority.m_Normal)  / 2;
    g_thread.m_priority.m_Above_Normal =
        (g_thread.m_priority.m_Normal + g_thread.m_priority.m_Highest) / 2;

/* @@@ set prio of calling (not main) thread (?) @@@ */

    param.sched_priority= g_thread.m_priority.m_Normal;

    if ((nRet = pthread_setschedparam(pthread_self(), policy, &param)) != 0)
    {
        OSL_TRACE("failed to change base priority of thread [%s]",strerror(nRet));
        OSL_TRACE("Thread ID '%i', Policy '%i', Priority '%i'\n",pthread_self(),policy,param.sched_priority);
    }

#endif /* NO_PTHREAD_PRIORITY */
}

/*****************************************************************************/
/* osl_setThreadPriority */
/*
    Impl-Notes: contrary to solaris-docu, which claims
    valid priority-levels from 0 .. INT_MAX, only the
    range 0..127 is accepted. (0 lowest, 127 highest)
*/
/*****************************************************************************/
void SAL_CALL osl_setThreadPriority (
    oslThread         Thread,
    oslThreadPriority Priority)
{
#ifndef NO_PTHREAD_PRIORITY

    struct sched_param Param;
    int policy;
    int nRet;

#endif /* NO_PTHREAD_PRIORITY */

    Thread_Impl* pImpl= (Thread_Impl*)Thread;

    OSL_ASSERT(pImpl);
    if (!pImpl)
        return; /* EINVAL */

#ifdef NO_PTHREAD_PRIORITY
    (void) Priority; /* unused */
#else /* NO_PTHREAD_PRIORITY */

    if (pthread_getschedparam(pImpl->m_hThread, &policy, &Param) != 0)
        return; /* ESRCH */

#if defined (SOLARIS)
    if ( policy >= _SCHED_NEXT)
    {
        /* mfe: pthread_getschedparam on Solaris has a possible Bug */
        /*      one gets 959917873 as the policy                   */
        /*      so set the policy to a default one                 */
        policy=SCHED_OTHER;
    }
#endif /* SOLARIS */

    pthread_once (&(g_thread.m_once), osl_thread_init_Impl);

    switch(Priority)
    {
        case osl_Thread_PriorityHighest:
            Param.sched_priority= g_thread.m_priority.m_Highest;
            break;

        case osl_Thread_PriorityAboveNormal:
            Param.sched_priority= g_thread.m_priority.m_Above_Normal;
            break;

        case osl_Thread_PriorityNormal:
            Param.sched_priority= g_thread.m_priority.m_Normal;
            break;

        case osl_Thread_PriorityBelowNormal:
            Param.sched_priority= g_thread.m_priority.m_Below_Normal;
            break;

        case osl_Thread_PriorityLowest:
            Param.sched_priority= g_thread.m_priority.m_Lowest;
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

    if ((nRet = pthread_setschedparam(pImpl->m_hThread, policy, &Param)) != 0)
    {
        OSL_TRACE("failed to change thread priority [%s]",strerror(nRet));
    }

#endif /* NO_PTHREAD_PRIORITY */
}

/*****************************************************************************/
/* osl_getThreadPriority */
/*****************************************************************************/
oslThreadPriority SAL_CALL osl_getThreadPriority(const oslThread Thread)
{
#ifndef NO_PTHREAD_PRIORITY

    struct sched_param Param;
    int Policy;

#endif /* NO_PTHREAD_PRIORITY */

    oslThreadPriority Priority = osl_Thread_PriorityNormal;
    Thread_Impl* pImpl= (Thread_Impl*)Thread;

    OSL_ASSERT(pImpl);
    if (!pImpl)
        return osl_Thread_PriorityUnknown; /* EINVAL */

#ifndef NO_PTHREAD_PRIORITY

    if (pthread_getschedparam(pImpl->m_hThread, &Policy, &Param) != 0)
        return osl_Thread_PriorityUnknown; /* ESRCH */

    pthread_once (&(g_thread.m_once), osl_thread_init_Impl);

    /* map pthread priority to enum */
    if (Param.sched_priority==g_thread.m_priority.m_Highest)
    {
        /* 127 - highest */
        Priority= osl_Thread_PriorityHighest;
    }
    else if (Param.sched_priority > g_thread.m_priority.m_Normal)
    {
        /* 65..126 - above normal */
        Priority= osl_Thread_PriorityAboveNormal;
    }
    else if (Param.sched_priority == g_thread.m_priority.m_Normal)
    {
        /* normal */
        Priority= osl_Thread_PriorityNormal;
    }
    else if (Param.sched_priority > g_thread.m_priority.m_Lowest)
    {
        /* 63..1 -below normal */
        Priority= osl_Thread_PriorityBelowNormal;
    }
    else if (Param.sched_priority == g_thread.m_priority.m_Lowest)
    {
        /* 0 - lowest */
        Priority= osl_Thread_PriorityLowest;
    }
    else
    {
        /* unknown */
        Priority= osl_Thread_PriorityUnknown;
    }

#endif /* NO_PTHREAD_PRIORITY */

    return Priority;
}

typedef struct _wrapper_pthread_key
{
    pthread_key_t m_key;
    oslThreadKeyCallbackFunction pfnCallback;
} wrapper_pthread_key;

/*****************************************************************************/
/* osl_createThreadKey */
/*****************************************************************************/
oslThreadKey SAL_CALL osl_createThreadKey( oslThreadKeyCallbackFunction pCallback )
{
    wrapper_pthread_key *pKey = (wrapper_pthread_key*)rtl_allocateMemory(sizeof(wrapper_pthread_key));

    if (pKey)
    {
        pKey->pfnCallback = pCallback;

        if (pthread_key_create(&(pKey->m_key), pKey->pfnCallback) != 0)
        {
            rtl_freeMemory(pKey);
            pKey = 0;
        }
    }

    return ((oslThreadKey)pKey);
}

/*****************************************************************************/
/* osl_destroyThreadKey */
/*****************************************************************************/
void SAL_CALL osl_destroyThreadKey(oslThreadKey Key)
{
    wrapper_pthread_key *pKey = (wrapper_pthread_key*)Key;
    if (pKey)
    {
        pthread_key_delete(pKey->m_key);
        rtl_freeMemory(pKey);
    }
}

/*****************************************************************************/
/* osl_getThreadKeyData */
/*****************************************************************************/
void* SAL_CALL osl_getThreadKeyData(oslThreadKey Key)
{
    wrapper_pthread_key *pKey = (wrapper_pthread_key*)Key;
    return pKey ? pthread_getspecific(pKey->m_key) : NULL;
}

/*****************************************************************************/
/* osl_setThreadKeyData */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setThreadKeyData(oslThreadKey Key, void *pData)
{
    sal_Bool bRet;
    void *pOldData = NULL;
    wrapper_pthread_key *pKey = (wrapper_pthread_key*)Key;
    if (!pKey)
        return sal_False;

    if (pKey->pfnCallback)
        pOldData = pthread_getspecific(pKey->m_key);

    bRet = (pthread_setspecific(pKey->m_key, pData) == 0);

    if (bRet && pKey->pfnCallback && pOldData)
        pKey->pfnCallback(pOldData);

    return bRet;
}

/*****************************************************************************/
/* Thread Local Text Encoding */
/*****************************************************************************/
static void osl_thread_textencoding_init_Impl (void)
{
    rtl_TextEncoding defaultEncoding;
    const char *     pszEncoding;

    /* create thread specific data key */
    pthread_key_create (&(g_thread.m_textencoding.m_key), NULL);

    /* determine default text encoding */
    pszEncoding = getenv ("SOLAR_USER_RTL_TEXTENCODING");
    if (pszEncoding)
        defaultEncoding = atoi(pszEncoding);
    else
        defaultEncoding = osl_getTextEncodingFromLocale(NULL);

    OSL_ASSERT(defaultEncoding != RTL_TEXTENCODING_DONTKNOW);

    /*
    Tools string functions call abort() on an unknown encoding so ASCII
    is a meaningfull fallback regardless whether the assertion makes sense.
    */

    if ( RTL_TEXTENCODING_DONTKNOW == defaultEncoding )
        defaultEncoding = RTL_TEXTENCODING_ASCII_US;

    g_thread.m_textencoding.m_default = defaultEncoding;
}

/*****************************************************************************/
/* osl_getThreadTextEncoding */
/*****************************************************************************/
rtl_TextEncoding SAL_CALL osl_getThreadTextEncoding()
{
    rtl_TextEncoding threadEncoding;

    pthread_once (&(g_thread.m_once), osl_thread_init_Impl);

    /* check for thread specific encoding, use default if not set */
    threadEncoding = SAL_INT_CAST(
        rtl_TextEncoding,
        (sal_uIntPtr) pthread_getspecific(g_thread.m_textencoding.m_key));
    if (0 == threadEncoding)
        threadEncoding = g_thread.m_textencoding.m_default;

    return threadEncoding;
}

/*****************************************************************************/
/* osl_setThreadTextEncoding */
/*****************************************************************************/
rtl_TextEncoding osl_setThreadTextEncoding(rtl_TextEncoding Encoding)
{
    rtl_TextEncoding oldThreadEncoding = osl_getThreadTextEncoding();

    /* save encoding in thread local storage */
    pthread_setspecific (
        g_thread.m_textencoding.m_key,
        (void*) SAL_INT_CAST(sal_uIntPtr, Encoding));

    return oldThreadEncoding;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
