/*************************************************************************
 *
 *  $RCSfile: thread.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:13 $
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

#ifndef _OSL_THREAD_H_
#define _OSL_THREAD_H_

#ifndef _OSL_TYPES_H_
#   include <osl/types.h>
#endif

#ifndef _RTL_TEXTENC_H_
#   include <rtl/textenc.h>
#endif

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


typedef enum
{
    osl_Thread_SleepNormal,
    osl_Thread_SleepCancel,
    osl_Thread_SleepPending,
    osl_Thread_SleepActive,
    osl_Thread_SleepError,
    osl_Thread_SleepUnknown,                /* don't use to set */
    osl_Thread_Sleep_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslThreadSleep;

typedef sal_uInt32 oslThreadIdentifier;

typedef sal_uInt32 oslThreadKey;

/** Create the thread, using the function-ptr pWorker as
    its main (worker) function. This functions receives in
    its void* parameter the value supplied by pThreadData.
    Once the OS-structures are initialized,the thread starts
    running.
    @return 0 if creation failed, otherwise a handle to the thread
*/
oslThread SAL_CALL osl_createThread(oslWorkerFunction pWorker, void* pThreadData);

/** Create the thread, using the function-ptr pWorker as
    its main (worker) function. This functions receives in
    its void* parameter the value supplied by pThreadData.
    The thread will be created, but it won't start running.
    To wake-up the thread, use resume().
    @return 0 if creation failed, otherwise a handle to the thread
*/
oslThread SAL_CALL osl_createSuspendedThread(oslWorkerFunction pWorker, void* pThreadData);

/** Get the identifier for the specified thread or if parameter
    Thread is NULL of the current active thread.
    @return identifier of the thread
*/
oslThreadIdentifier SAL_CALL osl_getThreadIdentifier(oslThread Thread);

/** Forcefully abort the thread, if it is still running.
    Then release the OS-structures and our thread data-structure.
    If Thread is NULL, the function won't do anything.
*/
void SAL_CALL osl_destroyThread(oslThread Thread);

/** Release our thread data-structure.
    If Thread is NULL, the function won't do anything.
    Note that we do not interfere with the actual running of
    the thread, we just free up the memory needed by the handle.
*/
void SAL_CALL osl_freeThreadHandle(oslThread Thread);

/** Wake-up a thread that was suspended with suspend() or
    createSuspended(). The oslThread must be valid!
*/
void SAL_CALL osl_resumeThread(oslThread Thread);

/** Suspend the execution of the thread. If you want the thread
    to continue, call resume(). The oslThread must be valid!
*/
void SAL_CALL osl_suspendThread(oslThread Thread);

/** Changes the threads priority.
    The oslThread must be valid!
*/
void SAL_CALL osl_setThreadPriority(oslThread Thread, oslThreadPriority Priority);

/** Retrieves the threads priority.
    Returns oslThreadPriorityUnknown for invalid Thread-argument or
    terminated thread. (I.e.: The oslThread might be invalid.)
*/
oslThreadPriority SAL_CALL osl_getThreadPriority(const oslThread Thread);

/** Returns True if the thread was created and has not terminated yet.
    Note that according to this definition a "running" thread might be
    suspended! Also returns False is Thread is NULL.
*/
sal_Bool SAL_CALL osl_isThreadRunning(const oslThread Thread);

/** Blocks the calling thread until Thread has terminated.
    Returns immediately if Thread is NULL.
*/
void SAL_CALL osl_joinWithThread(oslThread Thread);

/** Blocks the calling thread at least for the given number of
    of time. Returns False if the sleep is aborted by a call
    to awakeThread.
*/
oslThreadSleep SAL_CALL osl_sleepThread(oslThread Thread, const TimeValue* pDelay);

/** Awake a sleeping thread. Returns False if at least one of
    the handles is invalid or the thread is not sleeping.
*/
sal_Bool SAL_CALL osl_awakeThread(oslThread Thread);

/** Blocks the calling thread at least for the given number
    of time.
*/
void SAL_CALL osl_waitThread(const TimeValue* pDelay);

/** The requested thread will get terminate the next time
    scheduleThread() is called.
*/
void SAL_CALL osl_terminateThread(oslThread Thread);

/** Offers the rest of the threads time-slice to the OS.
    scheduleThread() should be called in the working loop
    of the thread, so any other thread could also get the
    processor. Returns False if the thread should terminate, so
    the thread could free any allocated resources.
*/
sal_Bool SAL_CALL osl_scheduleThread(oslThread Thread);

/** Offers the rest of the threads time-slice to the OS.
    Under POSIX you _need_ to yield(), otherwise, since the
    threads are not preempted during execution, NO other thread
    (even with higher priority) gets the processor. Control is
    only given to another thread if the current thread blocks
    or uses yield().
*/
void SAL_CALL osl_yieldThread(void);

/** Create a key to an associated thread local storage pointer. */
oslThreadKey SAL_CALL osl_createThreadKey(void);

/** Destroy a key to an associated thread local storage pointer. */
void SAL_CALL osl_destroyThreadKey(oslThreadKey Key);

/** Get to key associated thread specific data. */
void* SAL_CALL osl_getThreadKeyData(oslThreadKey Key);

/** Set to key associated thread specific data. */
sal_Bool SAL_CALL osl_setThreadKeyData(oslThreadKey Key, void *pData);

/** Get the current thread local text encoding. */
rtl_TextEncoding SAL_CALL osl_getThreadTextEncoding();

/** Set the thread local text encoding.
    @return the old text encoding.
*/
rtl_TextEncoding SAL_CALL osl_setThreadTextEncoding(rtl_TextEncoding Encoding);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_THREAD_H_ */

/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.9  2000/09/18 14:28:49  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.8  2000/07/31 09:34:24  mfe
*    changed declaration of setthreadtextenconding
*
*    Revision 1.7  2000/07/27 20:23:46  patrick.luby
*    Set osl_setThreadTextEncoding prototype to match function definition in
*    osl/unx/thread.c file.
*
*    Revision 1.6  2000/05/29 16:45:16  hro
*    SRC591: Explicite SAL_CALL calling convention
*
*    Revision 1.5  2000/05/17 14:50:15  mfe
*    comments upgraded
*
*    Revision 1.4  2000/03/27 09:16:51  hro
*    UNICODE New osl_getThreadTextEncoding
*
*    Revision 1.3  1999/12/22 13:38:17  mfe
*    some clean up
*
*    Revision 1.2  1999/10/27 15:02:08  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.1  1999/08/05 10:18:20  jsc
*    verschoben aus osl
*
*    Revision 1.15  1999/08/05 11:14:44  jsc
*    verschoben in root inc-Verzeichnis
*
*    Revision 1.14  1999/03/19 09:05:58  jsc
*    #60455# osl_isCurrentThresad entfernt
*
*    Revision 1.13  1999/01/20 18:53:41  jsc
*    #61011# Typumstellung
*
*    Revision 1.12  1998/11/05 11:13:16  rh
*    #59037# Thread local storage implementation
*
*    Revision 1.11  1998/03/13 15:07:33  rh
*    Cleanup of enum chaos and implemntation of pipes
*
*    Revision 1.10  1998/02/26 18:35:27  rh
*    Avoid unix zombies and rename oslThreadHandle to oslThread
*
*    Revision 1.9  1998/02/16 19:34:52  rh
*    Cleanup of ports, integration of Size_t, features for process
*
*    Revision 1.8  1997/10/16 16:35:51  rh
*    *** empty log message ***
*
*    Revision 1.7  1997/10/16 16:26:08  rh
*    *** empty log message ***
*
*    Revision 1.6  1997/09/09 12:03:30  fm
*    *** empty log message ***
*
*    Revision 1.5  1997/08/22 14:47:38  rh
*    TimeValue inserted
*
*    Revision 1.4  1997/07/22 14:29:30  rh
*    process added
*
*    Revision 1.3  1997/07/17 11:02:29  rh
*    Header adapted and profile added
*
*    Revision 1.2  1997/07/14 09:09:12  rh
*    Adaptions for killable sleeps
*
*    Revision 1.1  1997/06/19 13:10:13  bho
*    first version of OSL.
*
*************************************************************************/

