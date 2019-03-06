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

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <functional>

#include "system.hxx"
#include "unixerrnostring.hxx"
#include <string.h>
#if defined(OPENBSD)
#include <sched.h>
#endif
#include <config_options.h>
#include <osl/thread.h>
#include <osl/nlsupport.h>
#include <rtl/textenc.h>
#include <rtl/alloc.h>
#include <sal/log.hxx>
#include <sal/macros.h>
#ifdef ANDROID
#include <jni.h>
#include <android/log.h>
#include <osl/detail/android-bootstrap.h>
#endif

#if defined LINUX && ! defined __FreeBSD_kernel__
#include <sys/syscall.h>
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
 *     - use 'reinterpret_cast<unsigned long>(pthread_t)' as identifier
 *       instead (?)
 *     - if yes, change 'oslThreadIdentifier' to 'intptr_t' or similar
 * (3) 'oslSigAlarmHandler()' (#71232#)
 *     - [Under Solaris we get SIGALRM in e.g. pthread_join which terminates
 *       the process. So we initialize our signal handling module and do
 *       register a SIGALRM Handler which catches and ignores it]
 *     - should this still happen, 'signal.c' needs to be fixed instead.
 *
 ****************************************************************************/

#define THREADIMPL_FLAGS_TERMINATE  0x00001
#define THREADIMPL_FLAGS_STARTUP    0x00002
#define THREADIMPL_FLAGS_SUSPENDED  0x00004
#define THREADIMPL_FLAGS_ACTIVE     0x00008
#define THREADIMPL_FLAGS_ATTACHED   0x00010
#define THREADIMPL_FLAGS_DESTROYED  0x00020

typedef struct osl_thread_impl_st
{
    pthread_t           m_hThread;
    oslThreadIdentifier m_Ident; /* @@@ see TODO @@@ */
    short               m_Flags;
    oslWorkerFunction   m_WorkerFunction;
    void*               m_pData;
    pthread_mutex_t     m_Lock;
    pthread_cond_t      m_Cond;
} Thread_Impl;

struct osl_thread_priority_st
{
    int const m_Highest;
    int const m_Above_Normal;
    int const m_Normal;
    int const m_Below_Normal;
    int const m_Lowest;
};

#define OSL_THREAD_PRIORITY_INITIALIZER { 127, 96, 64, 32, 0 }
static void osl_thread_priority_init_Impl();

struct osl_thread_textencoding_st
{
    pthread_key_t    m_key;     /* key to store thread local text encoding */
    rtl_TextEncoding m_default; /* the default text encoding */
};

#define OSL_THREAD_TEXTENCODING_INITIALIZER { 0, RTL_TEXTENCODING_DONTKNOW }
static void osl_thread_textencoding_init_Impl();

struct osl_thread_global_st
{
    pthread_once_t                    m_once;
    struct osl_thread_priority_st const m_priority;
    struct osl_thread_textencoding_st m_textencoding;
};

static struct osl_thread_global_st g_thread =
{
    PTHREAD_ONCE_INIT,
    OSL_THREAD_PRIORITY_INITIALIZER,
    OSL_THREAD_TEXTENCODING_INITIALIZER
};

static void osl_thread_init_Impl();

static Thread_Impl* osl_thread_construct_Impl();
static void         osl_thread_destruct_Impl (Thread_Impl ** ppImpl);

static void* osl_thread_start_Impl (void * pData);
static void  osl_thread_cleanup_Impl (Thread_Impl * pImpl);

static oslThread osl_thread_create_Impl (
    oslWorkerFunction pWorker, void * pThreadData, short nFlags);

/* @@@ see TODO @@@ */
static oslThreadIdentifier insertThreadId (pthread_t hThread);
static oslThreadIdentifier lookupThreadId (pthread_t hThread);
static void                removeThreadId (pthread_t hThread);

static void osl_thread_init_Impl()
{
    osl_thread_priority_init_Impl();
    osl_thread_textencoding_init_Impl();
}

Thread_Impl* osl_thread_construct_Impl()
{
    Thread_Impl* pImpl = new Thread_Impl;
    memset (pImpl, 0, sizeof(Thread_Impl));

    pthread_mutex_init (&(pImpl->m_Lock), PTHREAD_MUTEXATTR_DEFAULT);
    pthread_cond_init  (&(pImpl->m_Cond), PTHREAD_CONDATTR_DEFAULT);
    return pImpl;
}

static void osl_thread_destruct_Impl (Thread_Impl ** ppImpl)
{
    assert(ppImpl);
    if (*ppImpl)
    {
        pthread_cond_destroy  (&((*ppImpl)->m_Cond));
        pthread_mutex_destroy (&((*ppImpl)->m_Lock));

        delete *ppImpl;
        (*ppImpl) = nullptr;
    }
}

static void osl_thread_cleanup_Impl (Thread_Impl * pImpl)
{
    pthread_t thread;
    bool attached;
    bool destroyed;

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

static void* osl_thread_start_Impl (void* pData)
{
    bool terminate;
    Thread_Impl* pImpl= static_cast<Thread_Impl*>(pData);

    assert(pImpl);

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
        int res = (*lo_get_javavm()).AttachCurrentThread(&env, NULL);
        __android_log_print(ANDROID_LOG_INFO, "LibreOffice", "New sal thread started and attached res=%d", res);
#endif
        /* call worker function */
        pImpl->m_WorkerFunction(pImpl->m_pData);

#ifdef ANDROID
        res = (*lo_get_javavm()).DetachCurrentThread();
        __android_log_print(ANDROID_LOG_INFO, "LibreOffice", "Detached finished sal thread res=%d", res);
#endif
    }

    osl_thread_cleanup_Impl (pImpl);
    return nullptr;
}

static oslThread osl_thread_create_Impl (
    oslWorkerFunction pWorker,
    void*             pThreadData,
    short             nFlags)
{
    Thread_Impl* pImpl;
#if defined OPENBSD || ((defined MACOSX || defined LINUX) && !ENABLE_RUNTIME_OPTIMIZATIONS)
    pthread_attr_t attr;
    size_t stacksize;
#endif
    int nRet=0;

    pImpl = osl_thread_construct_Impl();
    if (!pImpl)
        return nullptr; /* ENOMEM */

    pImpl->m_WorkerFunction = pWorker;
    pImpl->m_pData = pThreadData;
    pImpl->m_Flags = nFlags | THREADIMPL_FLAGS_STARTUP;

    pthread_mutex_lock (&(pImpl->m_Lock));

#if defined OPENBSD || ((defined MACOSX || defined LINUX) && !ENABLE_RUNTIME_OPTIMIZATIONS)
    if (pthread_attr_init(&attr) != 0)
        return nullptr;

#if defined OPENBSD
    stacksize = 262144;
#else
    stacksize = 12 * 1024 * 1024; // 8MB is not enough for ASAN on x86-64
#endif
    if (pthread_attr_setstacksize(&attr, stacksize) != 0) {
        pthread_attr_destroy(&attr);
        return nullptr;
    }
#endif

    if ((nRet = pthread_create (
        &(pImpl->m_hThread),
#if defined OPENBSD || ((defined MACOSX || defined LINUX) && !ENABLE_RUNTIME_OPTIMIZATIONS)
        &attr,
#else
        PTHREAD_ATTR_DEFAULT,
#endif
        osl_thread_start_Impl,
        static_cast<void*>(pImpl))) != 0)
    {
        SAL_WARN(
            "sal.osl",
            "pthread_create failed: " << UnixErrnoString(nRet));

        pthread_mutex_unlock (&(pImpl->m_Lock));
        osl_thread_destruct_Impl (&pImpl);

        return nullptr;
    }

#if defined OPENBSD || ((defined MACOSX || defined LINUX) && !ENABLE_RUNTIME_OPTIMIZATIONS)
    pthread_attr_destroy(&attr);
#endif

    /* wait for change from STARTUP to ACTIVE state */
    while (pImpl->m_Flags & THREADIMPL_FLAGS_STARTUP)
    {
        /* wait until STARTUP flag is cleared */
        pthread_cond_wait (&(pImpl->m_Cond), &(pImpl->m_Lock));
    }

    pthread_mutex_unlock (&(pImpl->m_Lock));

    return static_cast<oslThread>(pImpl);
}

oslThread osl_createThread (
    oslWorkerFunction pWorker,
    void *            pThreadData)
{
    return osl_thread_create_Impl (
        pWorker,
        pThreadData,
        THREADIMPL_FLAGS_ATTACHED);
}

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

void SAL_CALL osl_destroyThread(oslThread Thread)
{
    if (Thread != nullptr) {
        Thread_Impl * impl = static_cast<Thread_Impl *>(Thread);
        bool active;
        pthread_mutex_lock(&impl->m_Lock);
        active = (impl->m_Flags & THREADIMPL_FLAGS_ACTIVE) != 0;
        impl->m_Flags |= THREADIMPL_FLAGS_DESTROYED;
        pthread_mutex_unlock(&impl->m_Lock);
        if (!active) {
            osl_thread_destruct_Impl(&impl);
        }
    }
}

void SAL_CALL osl_resumeThread(oslThread Thread)
{
    Thread_Impl* pImpl= static_cast<Thread_Impl*>(Thread);

    if (!pImpl)
    {
        SAL_WARN("sal.osl", "invalid osl_resumeThread(nullptr) call");
        return; /* EINVAL */
    }

    pthread_mutex_lock (&(pImpl->m_Lock));

    if (pImpl->m_Flags & THREADIMPL_FLAGS_SUSPENDED)
    {
        /* clear SUSPENDED flag */
        pImpl->m_Flags &= ~THREADIMPL_FLAGS_SUSPENDED;
        pthread_cond_signal (&(pImpl->m_Cond));
    }

    pthread_mutex_unlock (&(pImpl->m_Lock));
}

void SAL_CALL osl_suspendThread(oslThread Thread)
{
    Thread_Impl* pImpl= static_cast<Thread_Impl*>(Thread);

    if (!pImpl)
    {
        SAL_WARN("sal.osl", "invalid osl_suspendThread(nullptr) call");
        return; /* EINVAL */
    }

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

sal_Bool SAL_CALL osl_isThreadRunning(const oslThread Thread)
{
    bool active;
    Thread_Impl* pImpl= static_cast<Thread_Impl*>(Thread);

    if (!pImpl)
        return false;

    pthread_mutex_lock (&(pImpl->m_Lock));
    active = ((pImpl->m_Flags & THREADIMPL_FLAGS_ACTIVE) > 0);
    pthread_mutex_unlock (&(pImpl->m_Lock));

    return active;
}

void SAL_CALL osl_joinWithThread(oslThread Thread)
{
    Thread_Impl* pImpl= static_cast<Thread_Impl*>(Thread);

    if (!pImpl)
        return;

    pthread_mutex_lock (&(pImpl->m_Lock));

    pthread_t const thread = pImpl->m_hThread;
    bool const attached = ((pImpl->m_Flags & THREADIMPL_FLAGS_ATTACHED) > 0);

    /* check this only if *this* thread is still attached - if it's not,
       then it could have terminated and another newly created thread could
       have recycled the same id as m_hThread! */
    if (attached && pthread_equal(pthread_self(), pImpl->m_hThread))
    {
        assert(false); /* Win32 implementation would deadlock here! */
        /* self join */
        pthread_mutex_unlock (&(pImpl->m_Lock));
        return; /* EDEADLK */
    }

    pImpl->m_Flags &= ~THREADIMPL_FLAGS_ATTACHED;

    pthread_mutex_unlock (&(pImpl->m_Lock));

    if (attached)
    {
        pthread_join (thread, nullptr);
    }
}

void SAL_CALL osl_terminateThread(oslThread Thread)
{
    Thread_Impl* pImpl= static_cast<Thread_Impl*>(Thread);

    if (!pImpl)
    {
        SAL_WARN("sal.osl", "invalid osl_terminateThread(nullptr) call");
        return; /* EINVAL */
    }

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

sal_Bool SAL_CALL osl_scheduleThread(oslThread Thread)
{
    bool terminate;
    Thread_Impl* pImpl= static_cast<Thread_Impl*>(Thread);

    if (!pImpl)
    {
        SAL_WARN("sal.osl", "invalid osl_scheduleThread(nullptr) call");
        return false; /* EINVAL */
    }

    if (!(pthread_equal (pthread_self(), pImpl->m_hThread)))
    {
        SAL_WARN("sal.osl", "invalid osl_scheduleThread(non-self) call");
        return false; /* EINVAL */
    }

    pthread_mutex_lock (&(pImpl->m_Lock));

    while (pImpl->m_Flags & THREADIMPL_FLAGS_SUSPENDED)
    {
        /* wait until SUSPENDED flag is cleared */
        pthread_cond_wait (&(pImpl->m_Cond), &(pImpl->m_Lock));
    }

    terminate = ((pImpl->m_Flags & THREADIMPL_FLAGS_TERMINATE) > 0);

    pthread_mutex_unlock(&(pImpl->m_Lock));

    return !terminate;
}

void SAL_CALL osl_waitThread(const TimeValue* pDelay)
{
    if (pDelay)
    {
        struct timespec delay;

        SET_TIMESPEC(delay, pDelay->Seconds, pDelay->Nanosec);

        SLEEP_TIMESPEC(delay);
    }
}

/** Yields thread

    @attention Note that POSIX scheduling @em really requires threads to call this
    function, since a thread only reschedules to other thread, when
    it blocks (sleep, blocking I/O) OR calls sched_yield().
*/
void SAL_CALL osl_yieldThread()
{
    sched_yield();
}

void SAL_CALL osl_setThreadName(char const * name)
{
    assert( name );
#if defined LINUX && ! defined __FreeBSD_kernel__
    const int LINUX_THREAD_NAME_MAXLEN = 15;
    if ( strlen( name ) > LINUX_THREAD_NAME_MAXLEN )
        SAL_INFO( "sal.osl", "osl_setThreadName truncated thread name to "
                  << LINUX_THREAD_NAME_MAXLEN << " chars from name '"
                  << name << "'" );
    char shortname[ LINUX_THREAD_NAME_MAXLEN + 1 ];
    shortname[ LINUX_THREAD_NAME_MAXLEN ] = '\0';
    strncpy( shortname, name, LINUX_THREAD_NAME_MAXLEN );
    int err = pthread_setname_np( pthread_self(), shortname );
    if ( 0 != err )
        SAL_WARN("sal.osl", "pthread_setname_np failed with errno " << err);
#elif defined __FreeBSD_kernel__
    pthread_setname_np( pthread_self(), name );
#elif defined MACOSX || defined IOS
    pthread_setname_np( name );
#else
    (void) name;
#endif
}

/* osl_getThreadIdentifier @@@ see TODO @@@ */

struct HashEntry
{
    pthread_t            Handle;
    oslThreadIdentifier  Ident;
    HashEntry *          Next;
};

static HashEntry* HashTable[31];
static const int HashSize = SAL_N_ELEMENTS(HashTable);

static pthread_mutex_t HashLock = PTHREAD_MUTEX_INITIALIZER;

#if ! ((defined LINUX && !defined __FreeBSD_kernel__) || defined MACOSX || defined IOS)
static oslThreadIdentifier LastIdent = 0;
#endif

namespace {

std::size_t HASHID(pthread_t x)
{ return std::hash<pthread_t>()(x) % HashSize; }

}

static oslThreadIdentifier lookupThreadId (pthread_t hThread)
{
    HashEntry *pEntry;

    pthread_mutex_lock(&HashLock);

    pEntry = HashTable[HASHID(hThread)];
    while (pEntry != nullptr)
    {
        if (pthread_equal(pEntry->Handle, hThread))
        {
            pthread_mutex_unlock(&HashLock);
            return pEntry->Ident;
        }
        pEntry = pEntry->Next;
    }

    pthread_mutex_unlock(&HashLock);

    return 0;
}

static oslThreadIdentifier insertThreadId (pthread_t hThread)
{
    HashEntry *pEntry, *pInsert = nullptr;

    pthread_mutex_lock(&HashLock);

    pEntry = HashTable[HASHID(hThread)];

    while (pEntry != nullptr)
    {
        if (pthread_equal(pEntry->Handle, hThread))
            break;

        pInsert = pEntry;
        pEntry = pEntry->Next;
    }

    if (pEntry == nullptr)
    {
        pEntry = static_cast<HashEntry*>(calloc(sizeof(HashEntry), 1));

        pEntry->Handle = hThread;

#if defined LINUX && ! defined __FreeBSD_kernel__
        long lin_tid = syscall(SYS_gettid);
        if (SAL_MAX_UINT32 < static_cast<unsigned long>(lin_tid))
            std::abort();
        pEntry->Ident = static_cast<pid_t>(lin_tid);
#elif defined MACOSX || defined IOS
        // currently the value of pthread_threadid_np is the same then
        // syscall(SYS_thread_selfid), which returns an int as the TID.
        // may change, as the syscall interface was deprecated.
        uint64_t mac_tid;
        pthread_threadid_np(nullptr, &mac_tid);
        if (mac_tid > SAL_MAX_UINT32)
            std::abort();
        pEntry->Ident = mac_tid;
#else
        ++LastIdent;
        if (0 == LastIdent)
            LastIdent = 1;
        pEntry->Ident = LastIdent;
#endif
        if (0 == pEntry->Ident)
            std::abort();

        if (pInsert)
            pInsert->Next = pEntry;
        else
            HashTable[HASHID(hThread)] = pEntry;
    }

    pthread_mutex_unlock(&HashLock);

    return pEntry->Ident;
}

static void removeThreadId (pthread_t hThread)
{
    HashEntry *pEntry, *pRemove = nullptr;

    pthread_mutex_lock(&HashLock);

    pEntry = HashTable[HASHID(hThread)];
    while (pEntry != nullptr)
    {
        if (pthread_equal(pEntry->Handle, hThread))
            break;

        pRemove = pEntry;
        pEntry = pEntry->Next;
    }

    if (pEntry != nullptr)
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
    Thread_Impl* pImpl= static_cast<Thread_Impl*>(Thread);
    oslThreadIdentifier Ident;

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

    return Ident;
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
static void osl_thread_priority_init_Impl()
{
#ifndef NO_PTHREAD_PRIORITY
    struct sched_param param;
    int policy=0;
    int nRet=0;

/* @@@ see TODO: calling thread may not be main thread @@@ */

    if ((nRet = pthread_getschedparam(pthread_self(), &policy, &param)) != 0)
    {
        SAL_WARN(
            "sal.osl",
            "pthread_getschedparam failed: " << UnixErrnoString(nRet));
        return;
    }

#if defined (__sun)
    if ( policy >= _SCHED_NEXT)
    {
        /* mfe: pthread_getschedparam on Solaris has a possible Bug */
        /*      one gets 959917873 as the policy                    */
        /*      so set the policy to a default one                  */
        policy=SCHED_OTHER;
    }
#endif /* __sun */

    if ((nRet = sched_get_priority_min(policy) ) != -1)
    {
        SAL_INFO(
            "sal.osl", "Min Prioriy for policy " << policy << " == " << nRet);
        g_thread.m_priority.m_Lowest=nRet;
    }
    else
    {
        int e = errno;
        SAL_WARN(
            "sal.osl",
            "sched_get_priority_min failed: " << UnixErrnoString(e));
    }

    if ((nRet = sched_get_priority_max(policy) ) != -1)
    {
        SAL_INFO(
            "sal.osl", "Max Prioriy for policy " << policy << " == " << nRet);
        g_thread.m_priority.m_Highest=nRet;
    }
    else
    {
        int e = errno;
        SAL_WARN(
            "sal.osl",
            "sched_get_priority_max failed: " << UnixErrnoString(e));
    }

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
        SAL_WARN(
            "sal.osl",
            "pthread_setschedparam failed: " << UnixErrnoString(nRet));
        SAL_INFO(
            "sal.osl",
            "Thread ID " << pthread_self() << ", Policy " << policy
                << ", Priority " << param.sched_priority);
    }

#endif /* NO_PTHREAD_PRIORITY */
}

/**
    Impl-Notes: contrary to solaris-docu, which claims
    valid priority-levels from 0 .. INT_MAX, only the
    range 0..127 is accepted. (0 lowest, 127 highest)
*/
void SAL_CALL osl_setThreadPriority (
    oslThread         Thread,
    oslThreadPriority Priority)
{
#ifndef NO_PTHREAD_PRIORITY

    struct sched_param Param;
    int policy;
    int nRet;

#endif /* NO_PTHREAD_PRIORITY */

    Thread_Impl* pImpl= static_cast<Thread_Impl*>(Thread);

    if (!pImpl)
    {
        SAL_WARN("sal.osl", "invalid osl_setThreadPriority(nullptr, ...) call");
        return; /* EINVAL */
    }

#ifdef NO_PTHREAD_PRIORITY
    (void) Priority; /* unused */
#else /* NO_PTHREAD_PRIORITY */

    if (pthread_getschedparam(pImpl->m_hThread, &policy, &Param) != 0)
        return; /* ESRCH */

#if defined (__sun)
    if ( policy >= _SCHED_NEXT)
    {
        /* mfe: pthread_getschedparam on Solaris has a possible Bug */
        /*      one gets 959917873 as the policy                   */
        /*      so set the policy to a default one                 */
        policy=SCHED_OTHER;
    }
#endif /* __sun */

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
            SAL_WARN(
                "sal.osl",
                "invalid osl_setThreadPriority(..., osl_Thread_PriorityUnknown)"
                    " call");
            return;

        default:
            SAL_WARN(
                "sal.osl",
                "invalid osl_setThreadPriority(..., " << Priority << ") call");
            return;
    }

    if ((nRet = pthread_setschedparam(pImpl->m_hThread, policy, &Param)) != 0)
    {
        SAL_WARN(
            "sal.osl",
            "pthread_setschedparam failed: " << UnixErrnoString(nRet));
    }

#endif /* NO_PTHREAD_PRIORITY */
}

oslThreadPriority SAL_CALL osl_getThreadPriority(const oslThread Thread)
{
#ifndef NO_PTHREAD_PRIORITY

    struct sched_param Param;
    int Policy;

#endif /* NO_PTHREAD_PRIORITY */

    oslThreadPriority Priority = osl_Thread_PriorityNormal;
    Thread_Impl* pImpl= static_cast<Thread_Impl*>(Thread);

    if (!pImpl)
    {
        SAL_WARN("sal.osl", "invalid osl_getThreadPriority(nullptr) call");
        return osl_Thread_PriorityUnknown; /* EINVAL */
    }

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

struct wrapper_pthread_key
{
    pthread_key_t m_key;
    oslThreadKeyCallbackFunction pfnCallback;
};

oslThreadKey SAL_CALL osl_createThreadKey( oslThreadKeyCallbackFunction pCallback )
{
    wrapper_pthread_key *pKey = static_cast<wrapper_pthread_key*>(malloc(sizeof(wrapper_pthread_key)));

    if (pKey)
    {
        pKey->pfnCallback = pCallback;

        if (pthread_key_create(&(pKey->m_key), pKey->pfnCallback) != 0)
        {
            free(pKey);
            pKey = nullptr;
        }
    }

    return static_cast<oslThreadKey>(pKey);
}

void SAL_CALL osl_destroyThreadKey(oslThreadKey Key)
{
    wrapper_pthread_key *pKey = static_cast<wrapper_pthread_key*>(Key);
    if (pKey)
    {
        pthread_key_delete(pKey->m_key);
        free(pKey);
    }
}

void* SAL_CALL osl_getThreadKeyData(oslThreadKey Key)
{
    wrapper_pthread_key *pKey = static_cast<wrapper_pthread_key*>(Key);
    return pKey ? pthread_getspecific(pKey->m_key) : nullptr;
}

sal_Bool SAL_CALL osl_setThreadKeyData(oslThreadKey Key, void *pData)
{
    bool bRet;
    void *pOldData = nullptr;
    wrapper_pthread_key *pKey = static_cast<wrapper_pthread_key*>(Key);
    if (!pKey)
        return false;

    if (pKey->pfnCallback)
        pOldData = pthread_getspecific(pKey->m_key);

    bRet = (pthread_setspecific(pKey->m_key, pData) == 0);

    if (bRet && pKey->pfnCallback && pOldData)
        pKey->pfnCallback(pOldData);

    return bRet;
}

static void osl_thread_textencoding_init_Impl()
{
    rtl_TextEncoding defaultEncoding;

    /* create thread specific data key */
    pthread_key_create (&(g_thread.m_textencoding.m_key), nullptr);

    /* determine default text encoding */
    defaultEncoding = osl_getTextEncodingFromLocale(nullptr);
    // Tools string functions call abort() on an unknown encoding so ASCII is a
    // meaningful fallback:
    if ( RTL_TEXTENCODING_DONTKNOW == defaultEncoding )
    {
        SAL_WARN("sal.osl", "RTL_TEXTENCODING_DONTKNOW -> _ASCII_US");
        defaultEncoding = RTL_TEXTENCODING_ASCII_US;
    }

    g_thread.m_textencoding.m_default = defaultEncoding;
}

rtl_TextEncoding SAL_CALL osl_getThreadTextEncoding()
{
    rtl_TextEncoding threadEncoding;

    pthread_once (&(g_thread.m_once), osl_thread_init_Impl);

    /* check for thread specific encoding, use default if not set */
    threadEncoding = static_cast<rtl_TextEncoding>(
        reinterpret_cast<sal_uIntPtr>(pthread_getspecific(g_thread.m_textencoding.m_key)));
    if (threadEncoding == 0)
        threadEncoding = g_thread.m_textencoding.m_default;

    return threadEncoding;
}

rtl_TextEncoding osl_setThreadTextEncoding(rtl_TextEncoding Encoding)
{
    rtl_TextEncoding oldThreadEncoding = osl_getThreadTextEncoding();

    /* save encoding in thread local storage */
    pthread_setspecific (
        g_thread.m_textencoding.m_key,
        reinterpret_cast<void*>(static_cast<sal_uIntPtr>(Encoding)));

    return oldThreadEncoding;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
