/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef COMPHELPER_POSIX_COMPAT_H
#define COMPHELPER_POSIX_COMPAT_H

#ifdef WNT

typedef HANDLE pthread_mutex_t ;
typedef int pthread_mutexattr_t;

typedef struct pthread_cond_t
{
    int nb_waiters;
    int broadcasting;
    CRITICAL_SECTION local_access;
    HANDLE pending;
    HANDLE done;
}pthread_cond_t;

/*
 * pthread emulation for windaube
 */
static inline int pthread_mutex_init(pthread_mutex_t* mutex, void* pattrib)
{
    *mutex = CreateMutex(NULL, FALSE, NULL);
    return 0;
}

static inline int pthread_mutex_destroy(pthread_mutex_t* mutex)
{
    CloseHandle(*mutex);
    return 0;
}

static inline int pthread_mutex_unlock(pthread_mutex_t* mutex)
{
    ReleaseMutex(*mutex);
    return 0;
}

static inline int pthread_mutex_trylock(pthread_mutex_t* mutex)
{
    int rc = 0;

    if(WaitForSingleObject(*mutex, 0) == WAIT_TIMEOUT)
    {
        rc = EBUSY;
    }
    return rc;
}

static inline int pthread_mutex_lock(pthread_mutex_t* mutex)
{
    WaitForSingleObject(*mutex, INFINITE);
    return 0;
}

static inline int pthread_mutexattr_init(pthread_mutexattr_t* pmutexAttr)
{
    *pmutexAttr = 0;
    return 0;
}

#define pthread_mutexattr_setpshared(pmutexAttr, val) 0

static inline int pthread_condattr_init(pthread_condattr_t* condattr)
{
    *condattr = 0;
    return 0;
}

static inline int pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr)
{
int rc = 0;

    cond->nb_waiters = 0;
    cond->broadcasting = 0;
    InitializeCriticalSection(&cond->local_access);
    cond->pending = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
    if(cond->pending == NULL)
    {
        rc = GetLastError();
    }
    else
    {
        cond->done = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(cond->done == NULL)
        {
            rc = GetLastError();
            CloseHandle(cond->pending);
        }
    }

    return rc;
}

static inline int pthread_cond_destroy(pthread_cond_t* cond)
{
int rc = 0;

    if(cond)
    {
        if(cond->pending)
        {
            CloseHandle(cond->pending);
            cond->pending = NULL;
        }
        if(cond->done)
        {
            CloseHandle(cond->done);
            cond->done = 0;
        }
        DeleteCriticalSection(&cond->local_access);
    }
    return rc;
}

static inline int pthread_cond_signal(pthread_cond_t* cond)
{
int waiters = 0;

    EnterCriticalSection (&cond->local_access);
    waiters = cond->nb_waiters;
    LeaveCriticalSection (&cond->local_access);
    if(waiters)
    {
        ReleaseSemaphore (cond->pending, 1, 0);
    }
    return 0;
}

static inline int pthread_cond_broadcast(pthread_cond_t* cond)
{
    EnterCriticalSection (&cond->local_access);

    if (cond->nb_waiters > 0)
    {
        cond->broadcasting = 1;
        ReleaseSemaphore (cond->pending, cond->nb_waiters, 0);
        LeaveCriticalSection (&cond->local_access);

        WaitForSingleObject (cond->done, INFINITE);
        cond->broadcasting = 0;
    }
    else
    {
        LeaveCriticalSection (&cond->local_access);
    }
    return 0;
}

static inline int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex)
{
int is_last;

    EnterCriticalSection (&cond->local_access);
    cond->nb_waiters += 1;
    LeaveCriticalSection (&cond->local_access);

    SignalObjectAndWait (*mutex, cond->pending, INFINITE, FALSE);

    EnterCriticalSection (&cond->local_access);
    cond->nb_waiters -= 1;
    is_last = cond->broadcasting && (cond->nb_waiters == 0);
    LeaveCriticalSection (&cond->local_access);

    if(is_last)
    {
        SignalObjectAndWait(cond->done, *mutex, INFINITE, FALSE);
    }
    else
    {
        WaitForSingleObject(*mutex, INFINITE);
    }
    return 0;
}

static inline int pthread_cond_timedwait(pthread_cond_t* cond,
                                         pthread_mutex_t* mutex,
                                         const struct timespec*  abstime)
{
int is_last;
struct timeval current_time;
DWORD milliseconds;

    gettimeofday(&current_time, NULL);
    milliseconds = (DWORD)(abstime->tv_sec - current_time.tv_sec);
    milliseconds *= 1000;

    EnterCriticalSection (&cond->local_access);
    cond->nb_waiters += 1;
    LeaveCriticalSection (&cond->local_access);

    SignalObjectAndWait (*mutex, cond->pending, milliseconds, FALSE);

    EnterCriticalSection (&cond->local_access);
    cond->nb_waiters -= 1;
    is_last = cond->broadcasting && (cond->nb_waiters == 0);
    LeaveCriticalSection (&cond->local_access);

    if(is_last)
    {
        SignalObjectAndWait(cond->done, *mutex, INFINITE, FALSE);
    }
    else
    {
        WaitForSingleObject(*mutex, INFINITE);
    }
    return 0;
}
#endif /* WNT */
#endif /* COMPHELPER_POSIX_COMPAT_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
