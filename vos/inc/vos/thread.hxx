/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _VOS_THREAD_HXX_
#define _VOS_THREAD_HXX_

#   include <vos/types.hxx>
#   include <vos/object.hxx>
#   include <osl/thread.h>
#   include <osl/conditn.h>
#   include <vos/runnable.hxx>

#include <osl/time.h>

namespace vos
{

extern "C" typedef void ThreadWorkerFunction_impl(void *);
ThreadWorkerFunction_impl threadWorkerFunction_impl;

/** OThread is an objectoriented interface for threads.
    This class should be the base class for all objects using threads. The
    main working function is the run() method and should be overriden in the
    derived class. To support soft termination of a thread, yield() should
    be called in regular intervalls and the return value should be checked.
    If yield returned False the run method should return.

    @author  Bernd Hofner
    @version 1.0
*/

class OThread : public vos::IRunnable,
                public vos::OObject
{

    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OThread, vos));

    oslCondition m_aCondition;

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
        TSleep_Normal,
        TSleep_Cancel,
        TSleep_Pending,
        TSleep_Active,
        TSleep_Error,
        TSleep_Unknown
    };

    typedef oslThreadIdentifier TThreadIdentifier;

    /// Constructor
    OThread();

    /// Destructor kills thread if necessary
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

    friend void threadWorkerFunction_impl(void *);
};

class OThreadData : public vos::OObject
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OThreadData, vos));

public:
    /// Create a thread specific local data key
    OThreadData( oslThreadKeyCallbackFunction = 0 );

    /// Destroy a thread specific local data key
    virtual ~OThreadData();

    /** Set the data associated with the data key.
        @returns True if operation was successful
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

}

#endif // _VOS_THREAD_HXX_

