/*************************************************************************
 *
 *  $RCSfile: threadpool.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-28 10:46:06 $
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

#include <sal/types.h>
#include <rtl/byteseq.h>

#ifdef __cplusplus
extern "C" {
#endif

/***
 * Thread identifier administration.
 ***/
/**
 * Establishs an association between the current thread an the given thread identifier.
 * There can be only one association at a time. The association must be broken by
 * uno_releaseIdFromCurrentThread.
 * This method is in general called by a bridge, that wants to bind a remote threadId
 * to a new thread.
 *
 * @param pThreadId a byte sequence, that contains the identifier of the current thread.
 * @return true, when the identifier was registered. <br>
 *         false, when the thread has already an identifier. The identifier was not
 *         altered. ( This is in general a bug ). <br>
 **/
sal_Bool SAL_CALL uno_bindIdToCurrentThread( sal_Sequence *pThreadId )
    SAL_THROW_EXTERN_C();


/**
 * Get the identifier of the current thread.
 * If no id has been bound for the thread before, a new one is generated and bound
 * to the thread.
 * For each call to uno_getIdOfCurrentThread, a call to uno_releaseIdFromCurrentThread
 * must be done.
 *
 * @param ppThreadId [out] Contains the (acquired) ThreadId.
 **/
void SAL_CALL uno_getIdOfCurrentThread( sal_Sequence **ppThreadId )
    SAL_THROW_EXTERN_C();


/**
 * If the internal refcount drops to zero, the association betwen threadId and
 * thread is broken.
 **/
void SAL_CALL uno_releaseIdFromCurrentThread()
    SAL_THROW_EXTERN_C();


/**
 * The threadpool
 **/
struct uno_threadpool_Handle;
/**
 * Create a handle for the current thread before entering waiting pool. This method must be
 * called, BEFORE the request is sent (to avoid a race between this thread and an incoming
 * reply).
 * This method shall only be called for synchronous requests.
 *
 * @param nDisposeId An ID, that uniquely identifies a bridge within the
 *                   local process. The pointer to the bridge object should be used.
 * @see uno_threadpool_disposeThreads
 ***/
struct uno_threadpool_Handle * SAL_CALL
uno_threadpool_createHandle( sal_Int64 nDisposeId ) SAL_THROW_EXTERN_C();

/**
 * This method is called to wait for a reply of a previously sent request. This is a
 * blocking method.
 *
 * @param pHandle  the handle that was previously created by uno_threadpool_createHandle.
 * @param ppThreadSpecificData [out] the pointer, that was given by uno_threadpool_reply.
 *                             If the threads for this bridge were disposed,
 *                             *ppThreadSpecificData is null.
 **/
void SAL_CALL
uno_threadpool_enter( struct uno_threadpool_Handle * pHandle , void **ppThreadSpecificData )
    SAL_THROW_EXTERN_C();


/**
 * A request is put into a queue of waiting requests. This method is non-blocking.
 *
 * If the request is synchronous, it is first looked up,
 * if there exists a handle with the given
 * identifier. If this is the case, the thread is woken up and the doRequest
 * function is called with the given pThreadSpecificData. If no handle exists,
 * a new thread is  created and the given threadId is bound to the new thread.
 *
 * If the request is asynchronous, it is put into the queue of asynchronous
 * requests for the current threadid. The requests are always executed in a new
 * thread, even if the thread with the given Id waiting in the pool. No Id is bound
 * to the newly created thread. The responsibilty is left to the bridge ( if it
 * wishes to bind a name).
 *
 * @param pThreadId The Id of thread, that initialized this request. (In general a
 *                  remote threadid).
 * @param pThreadSpecificData The argument, that doRequest will get.
 * @param doRequest The function, that shall be called to execute the request.
 * @param bIsOneway True, if the request is asynchrons. False, if it is synchronous
 *
 **/
void SAL_CALL
uno_threadpool_putRequest( sal_Sequence *pThreadId,
                           void *pThreadSpecificData,
                           void ( SAL_CALL * doRequest ) ( void *pThreadSpecificData ),
                           sal_Bool bIsOneway ) SAL_THROW_EXTERN_C();


/**
 * A reply is put into the threadpool. There MUST be a thread with the given threadId waiting
 * for this reply. This method is non-blocking.
 *
 * @param pThreadSpecificData The pointer, that is returned by uno_threadpool_enter.
 **/
void SAL_CALL
uno_threadpool_putReply( sal_Sequence *pThreadId, void *pThreadSpecificData ) SAL_THROW_EXTERN_C();


/**
 * All threads, that are waiting on handles, that were created with
 * nDisposeId, are forced out of the pool.
 * (@see uno_threadpool_createTicket) These threads will return from
 * uno_threadpool_enter with 0 == *ppThreadSpecificData.
 * Later calls to uno_threadpool_enter with the given disposeId also
 * return immeadiatly.
 *
 * @param nDisposeId Identfies the caller of uno_threadpool_createTicket
 *
 * This function is called i.e. by a bridge, that is forced to dispose itself.
 * When disposing of the bridge has finished, the bridge MUST call
 * uno_threadpool_stopDisposeThreads.
 **/
void SAL_CALL
uno_threadpool_disposeThreads( sal_Int64 nDisposeId ) SAL_THROW_EXTERN_C();


/**
 * Informs the threadpool, that no special treatment is needed for the given nDisposeId.
 * This allows the threadpool to release internal resources
 * and must be called after all threads originated from this bridge have returned.
 * (This can in general be done in the bridge destructor).
 *
 * @param nDisposeId Identifies the caller of uno_threadpool_createTicket
 * @see uno_threadpool_disposeThreads
 **/
void SAL_CALL
uno_threadpool_stopDisposeThreads( sal_Int64 nDisposeId ) SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif
