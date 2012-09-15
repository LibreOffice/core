/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _OSL_THREAD_H_
#define _OSL_THREAD_H_

#include "sal/config.h"

#include "osl/time.h"
#include "rtl/textenc.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
    Opaque data type for threads. As with all other osl-handles
    you can initialize and/or test it to/for 0.
*/
typedef void* oslThread;

/** the function-ptr. representing the threads worker-function.
*/
typedef void (SAL_CALL *oslWorkerFunction)(void*);

/** levels of thread-priority
    Note that oslThreadPriorityUnknown might be returned
    by getPriorityOfThread() (e.g. when it is terminated),
    but mustn't be used with setPriority()!
*/
typedef enum
{
    osl_Thread_PriorityHighest,
    osl_Thread_PriorityAboveNormal,
    osl_Thread_PriorityNormal,
    osl_Thread_PriorityBelowNormal,
    osl_Thread_PriorityLowest,
    osl_Thread_PriorityUnknown,         /* don't use to set */
    osl_Thread_Priority_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslThreadPriority;


typedef sal_uInt32 oslThreadIdentifier;

typedef void* oslThreadKey;

/** Create the thread, using the function-ptr pWorker as
    its main (worker) function. This functions receives in
    its void* parameter the value supplied by pThreadData.
    Once the OS-structures are initialized,the thread starts
    running.
    @return 0 if creation failed, otherwise a handle to the thread
*/
SAL_DLLPUBLIC oslThread SAL_CALL osl_createThread(oslWorkerFunction pWorker, void* pThreadData);

/** Create the thread, using the function-ptr pWorker as
    its main (worker) function. This functions receives in
    its void* parameter the value supplied by pThreadData.
    The thread will be created, but it won't start running.
    To wake-up the thread, use resume().
    @return 0 if creation failed, otherwise a handle to the thread
*/
SAL_DLLPUBLIC oslThread SAL_CALL osl_createSuspendedThread(oslWorkerFunction pWorker, void* pThreadData);

/** Get the identifier for the specified thread or if parameter
    Thread is NULL of the current active thread.
    @return identifier of the thread
*/
SAL_DLLPUBLIC oslThreadIdentifier SAL_CALL osl_getThreadIdentifier(oslThread Thread);

/** Release the thread handle.
    If Thread is NULL, the function won't do anything.
    Note that we do not interfere with the actual running of
    the thread, we just free up the memory needed by the handle.
*/
SAL_DLLPUBLIC void SAL_CALL osl_destroyThread(oslThread Thread);

/** Wake-up a thread that was suspended with suspend() or
    createSuspended(). The oslThread must be valid!
*/
SAL_DLLPUBLIC void SAL_CALL osl_resumeThread(oslThread Thread);

/** Suspend the execution of the thread. If you want the thread
    to continue, call resume(). The oslThread must be valid!
*/
SAL_DLLPUBLIC void SAL_CALL osl_suspendThread(oslThread Thread);

/** Changes the threads priority.
    The oslThread must be valid!
*/
SAL_DLLPUBLIC void SAL_CALL osl_setThreadPriority(oslThread Thread, oslThreadPriority Priority);

/** Retrieves the threads priority.
    Returns oslThreadPriorityUnknown for invalid Thread-argument or
    terminated thread. (I.e.: The oslThread might be invalid.)
*/
SAL_DLLPUBLIC oslThreadPriority SAL_CALL osl_getThreadPriority(const oslThread Thread);

/** Returns True if the thread was created and has not terminated yet.
    Note that according to this definition a "running" thread might be
    suspended! Also returns False is Thread is NULL.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_isThreadRunning(const oslThread Thread);

/** Blocks the calling thread until Thread has terminated.
    Returns immediately if Thread is NULL.
*/
SAL_DLLPUBLIC void SAL_CALL osl_joinWithThread(oslThread Thread);

/** Blocks the calling thread at least for the given number
    of time.
*/
SAL_DLLPUBLIC void SAL_CALL osl_waitThread(const TimeValue* pDelay);

/** The requested thread will get terminate the next time
    scheduleThread() is called.
*/
SAL_DLLPUBLIC void SAL_CALL osl_terminateThread(oslThread Thread);

/** Offers the rest of the threads time-slice to the OS.
    scheduleThread() should be called in the working loop
    of the thread, so any other thread could also get the
    processor. Returns False if the thread should terminate, so
    the thread could free any allocated resources.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_scheduleThread(oslThread Thread);

/** Offers the rest of the threads time-slice to the OS.
    Under POSIX you _need_ to yield(), otherwise, since the
    threads are not preempted during execution, NO other thread
    (even with higher priority) gets the processor. Control is
    only given to another thread if the current thread blocks
    or uses yield().
*/
SAL_DLLPUBLIC void SAL_CALL osl_yieldThread(void);

/** Attempts to set the name of the current thread.

    The name of a thread is usually evaluated for debugging purposes.  Not all
    platforms support this.  On Linux, a set thread name can be observed with
    "ps -L".  On Windows with the Microsoft compiler, a thread name set while a
    debugger is attached can be observed within the debugger.

    @param name  the name of the thread; must not be null; on Linux, only the
    first 16 characters are used
*/
SAL_DLLPUBLIC void SAL_CALL osl_setThreadName(char const * name);

/* Callback when data stored in a thread key is no longer needed */

typedef void (SAL_CALL *oslThreadKeyCallbackFunction)(void *);

/** Create a key to an associated thread local storage pointer. */
SAL_DLLPUBLIC oslThreadKey SAL_CALL osl_createThreadKey(oslThreadKeyCallbackFunction pCallback);

/** Destroy a key to an associated thread local storage pointer. */
SAL_DLLPUBLIC void SAL_CALL osl_destroyThreadKey(oslThreadKey Key);

/** Get to key associated thread specific data. */
SAL_DLLPUBLIC void* SAL_CALL osl_getThreadKeyData(oslThreadKey Key);

/** Set to key associated thread specific data. */
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_setThreadKeyData(oslThreadKey Key, void *pData);

/** Get the current thread local text encoding. */
SAL_DLLPUBLIC rtl_TextEncoding SAL_CALL osl_getThreadTextEncoding(void);

/** Set the thread local text encoding.
    @return the old text encoding.
*/
SAL_DLLPUBLIC rtl_TextEncoding SAL_CALL osl_setThreadTextEncoding(rtl_TextEncoding Encoding);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_THREAD_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
