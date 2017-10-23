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

#ifndef INCLUDED_UNO_THREADPOOL_H
#define INCLUDED_UNO_THREADPOOL_H

#include "cppu/cppudllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***
 * Thread identifier administration.
 ***/
/**
  Establishes an association between the current thread and the given thread identifier.
  There can be only one association at a time. The association must be broken by
  uno_releaseIdFromCurrentThread().
  This method is in general called by a bridge, that wants to bind a remote threadId
  to a new thread.

  @param pThreadId a byte sequence, that contains the identifier of the current thread.
  @return true, when the identifier was registered.
          false, when the thread has already an identifier. The identifier was not
          altered. ( This is in general a bug ).

  @see uno_releaseIdFromCurrentThread()
 */
CPPU_DLLPUBLIC sal_Bool SAL_CALL uno_bindIdToCurrentThread( sal_Sequence *pThreadId )
    SAL_THROW_EXTERN_C();


/**
  Get the identifier of the current thread.
  If no id has been bound for the thread before, a new one is generated and bound
  to the thread.
  For each call to uno_getIdOfCurrentThread(), a call to uno_releaseIdFromCurrentThread()
  must be done.

  @param ppThreadId [out] Contains the (acquired) ThreadId.
  @see uno_releaseIdFromCurrentThread()
 */
CPPU_DLLPUBLIC void SAL_CALL uno_getIdOfCurrentThread( sal_Sequence **ppThreadId )
    SAL_THROW_EXTERN_C();


/**
  If the internal refcount drops to zero, the association between threadId and
  thread is broken.
 */
CPPU_DLLPUBLIC void SAL_CALL uno_releaseIdFromCurrentThread()
    SAL_THROW_EXTERN_C();


struct _uno_ThreadPool;
typedef struct _uno_ThreadPool * uno_ThreadPool;

/**
  Creates a threadpool handle. Typically each remote bridge instances creates one
  handle.
 */
CPPU_DLLPUBLIC uno_ThreadPool SAL_CALL
uno_threadpool_create() SAL_THROW_EXTERN_C();


/**
  Makes the current thread known to the threadpool. This function must be
  called, BEFORE uno_threadpool_enter() is called and BEFORE a job for this
  thread is put into the threadpool (avoid a race between this thread and
  an incoming request/reply).
  For every call to uno_threadpool_attach, a corresponding call to
  uno_threadpool_detach must be done.

  @param hPool The bridge threadpool handle previously created by uno_threadpool_create.

*/
CPPU_DLLPUBLIC void SAL_CALL
uno_threadpool_attach( uno_ThreadPool hPool ) SAL_THROW_EXTERN_C();

/**
  This method is called to wait for a reply of a previously sent request. This is a
  blocking method. uno_threadpool_attach() must have been called before.

  @param hPool the handle that was previously created by uno_threadpool_create().
  @param ppJob [out] the pointer, that was given by uno_threadpool_putJob
  0, when uno_threadpool_dispose() was the reason to fall off from threadpool.
  @see uno_threadpool_dispose()
 **/
CPPU_DLLPUBLIC void SAL_CALL
uno_threadpool_enter( uno_ThreadPool hPool , void **ppJob )
    SAL_THROW_EXTERN_C();

/**
   Detaches the current thread from the threadpool. Must be called for
   every call to uno_threadpool_attach.
  @param hPool the handle that was previously created by uno_threadpool_create().
*/
CPPU_DLLPUBLIC void SAL_CALL
uno_threadpool_detach( uno_ThreadPool hPool ) SAL_THROW_EXTERN_C();

/**
  Puts a job into the pool. A job may either be a request or a reply
  (replies have a 0 in the doRequest parameter). This function is non-blocking.

  A request may either be synchronous or asynchronous.
  If the request is synchronous, it is first looked up,
  if there exists a handle with the given
  identifier. If this is the case, the thread is woken up and the doRequest
  function is called with the given pJob. If no handle exists,
  a new thread is created and the given threadId is bound to the new thread.

  If the request is asynchronous, it is put into the queue of asynchronous
  requests for the current threadid. The requests are always executed in a new
  thread, even if the thread with the given id is waiting in the pool. No id is bound
  to the newly created thread. The responsibilty is left to the bridge ( if it
  wishes to bind a name).

  If pJob is a reply, there MUST be a thread with the given threadId waiting
  for this reply.

  @param hPool the handle that was previously created by uno_threadpool_create().
  @param pThreadId The Id of the thread, that initialized this request. (In general a
                   remote threadid).
  @param pJob The argument, that doRequest will get or that will be returned by
                   uno_threadpool_enter().
  @param doRequest The function, that shall be called to execute the request.
                   0 if pJob is a reply.
  @param bIsOneway True, if the request is asynchronous. False, if it is synchronous.
                   Set to sal_False, if pJob is a reply.
 */
CPPU_DLLPUBLIC void SAL_CALL
uno_threadpool_putJob(
    uno_ThreadPool hPool,
    sal_Sequence *pThreadId,
    void *pJob,
    void ( SAL_CALL * doRequest ) ( void *pThreadSpecificData ),
    sal_Bool bIsOneway ) SAL_THROW_EXTERN_C();

/**
  All threads, that are waiting on the hPool handle, are forced out of the pool.
  The threads waiting with uno_threadpool_enter() will return with *ppJob == 0

  Later calls to uno_threadpool_enter() using the hPool handle will also
  return immediately with *ppJob == 0.

  @param hPool The handle to be disposed.

  This function is called i.e. by a bridge, that is forced to dispose itself.
 */
CPPU_DLLPUBLIC void SAL_CALL
uno_threadpool_dispose( uno_ThreadPool hPool ) SAL_THROW_EXTERN_C();


/** Releases the previously with uno_threadpool_create() created handle.
    The handle thus becomes invalid. It is an error to use the handle after
    uno_threadpool_destroy().

    A call to uno_threadpool_destroy can synchronously join on spawned worker
    threads, so this function must never be called from such a worker thread.

    @see uno_threadpool_create()
 */
CPPU_DLLPUBLIC void SAL_CALL
uno_threadpool_destroy( uno_ThreadPool hPool ) SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_UNO_THREADPOOL_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
