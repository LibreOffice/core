/*************************************************************************
 *
 *  $RCSfile: thread.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-14 10:01:09 $
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

#ifndef _VOS_THREAD_HXX_
#define _VOS_THREAD_HXX_

#ifndef _VOS_TYPES_HXX_
#   include <vos/types.hxx>
#endif
#ifndef _VOS_OBJECT_HXX_
#   include <vos/object.hxx>
#endif
#ifndef _OSL_THREAD_H_
#   include <osl/thread.h>
#endif
#ifndef _VOS_RUNNABLE_HXX_
#   include <vos/runnable.hxx>
#endif

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

extern void SAL_CALL _OThread_WorkerFunction(void* pthis);

#ifdef _USE_NAMESPACE
namespace vos
{
#endif

#if defined ( _USE_NAMESPACE ) && defined ( UNX )
void SAL_CALL _cpp_OThread_WorkerFunction(void* pthis);
#endif

/** OThread is an objectoriented interface for threads.
    This class should be the base class for all objects using threads. The
    main working function is the run() method and should be overriden in the
    derived class. To support soft termination of a thread, yield() should
    be called in regular intervalls and the return value should be checked.
    If yield returned False the run method should return.

    @author  Bernd Hofner
    @version 1.0
*/

class OThread : public NAMESPACE_VOS(IRunnable),
                public NAMESPACE_VOS(OObject)
{

    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OThread, vos));

public:
    /** priority of thread.
    */
    enum TThreadPriority
    {
        TPriority_Highest     = osl_Thread_PriorityHighest,
        TPriority_AboveNormal = osl_Thread_PriorityAboveNormal,
        TPriority_Normal      = osl_Thread_PriorityNormal,
        TPriority_BelowNormal = osl_Thread_PriorityBelowNormal,
        TPriority_Lowest      = osl_Thread_PriorityLowest,
        TPriority_Unknown     = osl_Thread_PriorityUnknown
    };

    /**
    */
    enum TThreadSleep
    {
        TSleep_Normal  = osl_Thread_SleepNormal,
        TSleep_Cancel  = osl_Thread_SleepCancel,
        TSleep_Pending = osl_Thread_SleepPending,
        TSleep_Active  = osl_Thread_SleepActive,
        TSleep_Error   = osl_Thread_SleepError,
        TSleep_Unknown = osl_Thread_SleepUnknown
    };

    typedef oslThreadIdentifier TThreadIdentifier;

    /// Constructor
    OThread();

    /// Destructor kills thread if neccessary
    virtual ~OThread();

    /** Create running instance of a thread.
        @returns True if thread could be created.
    */
    sal_Bool SAL_CALL create();

    /** Create suspended instance of a thread.
        @returns True if thread could be created.
    */
    sal_Bool SAL_CALL createSuspended();

    /// Suspend a runnng thread
    void SAL_CALL suspend();

    /// Resume a suspended thread
    void SAL_CALL resume();

    /** Tries to kill the thread.
        will not block and might not succeed when run() won't heed isTerminationRequested().
    */
    virtual void SAL_CALL terminate();

    /// Kill thread hard and block until it is actually gone
    virtual void SAL_CALL kill();

    /// Block till thread is terminated
    void SAL_CALL join();

    /// Check if thread is running.
    sal_Bool SAL_CALL isRunning();

    /** Change thread priority.
        The valid priority levels are:
    <ul>
        <li>ThreadPriorityHighest,
        <li>ThreadPriorityAboveNormal,
        <li>ThreadPriorityNormal,
        <li>ThreadPriorityBelowNormal,
        <li>ThreadPriorityLowest,
    </ul>
    */
    void SAL_CALL setPriority(TThreadPriority Priority);

    /** Query thread priority.
        Valid return values are:
    <ul>
        <li>ThreadPriorityHighest,
        <li>ThreadPriorityAboveNormal,
        <li>ThreadPriorityNormal,
        <li>ThreadPriorityBelowNormal,
        <li>ThreadPriorityLowest,
        <li>ThreadPriorityUnknown (returned if thread is killed)
    </ul>
    */
    TThreadPriority SAL_CALL getPriority();

    TThreadIdentifier SAL_CALL getIdentifier() const;

    static TThreadIdentifier SAL_CALL getCurrentIdentifier();

    /** Let thread sleep a specified amout of time.
        @param Delay specifies the number of time to sleep.
    */
    TThreadSleep SAL_CALL sleep(const TimeValue& Delay);

    /** Awake the sleeping thread.
        @returns False if at least one of the handles is invalid
        or the thread is not sleeping.
    */
    sal_Bool SAL_CALL awake();

    /** Let current thread wait a specified amout of time.
        @param Delay specifies the number of time
        to wait. Note, if you need to interrupt the waiting operation
        use sleep instead.
    */
    static void SAL_CALL wait(const TimeValue& Delay);

    /** Reschedules threads.
        Call within your loop if you
        want other threads offer some processing time.
        This method is static, so it might be used by the
        main-thread.
    */
    static void SAL_CALL yield();

protected:

    /// Working method which should be overridden.
    virtual void SAL_CALL run() = 0;

    /** Checks if thread should terminate.
        isTerminationRequested() will return True if someone called
        terminate().
        @return True if thread should terminate, False if he can continue.
    */
    virtual sal_Bool SAL_CALL schedule();

    /** Called when run() is done.
        You might want to override it to do some cleanup.
    */
    virtual void SAL_CALL onTerminated();

protected:
    oslThread m_hThread;
    sal_Bool   m_bTerminating;

#if defined ( _USE_NAMESPACE ) && defined ( WNT ) && !defined(GCC)
    friend static void ::_OThread_WorkerFunction(void* pthis);
#elif defined ( _USE_NAMESPACE )
    friend void _cpp_OThread_WorkerFunction(void* pthis);
#else
    friend void _OThread_WorkerFunction(void* pthis);
#endif


};

class OThreadData : public NAMESPACE_VOS(OObject)
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OThreadData, vos));

public:
    /// Create a thread specific local data key
    OThreadData( oslThreadKeyCallbackFunction = 0 );

    /// Destroy a thread specific local data key
    virtual ~OThreadData();

    /** Set the data associated with the data key.
        @returns True if operation was successfull
    */
    sal_Bool SAL_CALL setData(void *pData);

    /** Get the data associated with the data key.
        @returns The data asscoitaed with the data key or
        NULL if no data was set
    */
    void* SAL_CALL getData();

protected:
    oslThreadKey m_hKey;
};

#ifdef _USE_NAMESPACE
}
#endif

#endif // _VOS_THREAD_HXX_

