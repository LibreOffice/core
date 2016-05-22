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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"
//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------
#include <sal/types.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/strbuf.hxx>
#endif

#ifndef _OSL_THREAD_HXX
#include <osl/thread.hxx>
#endif

#ifndef _OSL_MUTEX_HXX
#include <osl/mutex.hxx>
#endif
#include <osl/time.h>

#include "gtest/gtest.h"

using namespace osl;
using namespace rtl;

#ifdef UNX
#include <unistd.h>
#include <time.h>
#endif
// -----------------------------------------------------------------------------
// Kleine Stopuhr
class StopWatch {
    TimeValue t1,t2;                                // Start und Stopzeit

protected:
    sal_Int32 m_nNanoSec;
    sal_Int32 m_nSeconds;

    bool m_bIsValid;                                   // TRUE, wenn gestartet und gestoppt
    bool m_bIsRunning;                                 // TRUE, wenn gestartet.

public:
    StopWatch();
    ~StopWatch() {}

    void start();                                 // Startet Timer
    void stop();                                  // Stoppt Timer

    double getSeconds() const;
    double getTenthSec() const;
};

// ================================= Stop Watch =================================

// Eine kleine Stop-Uhr fuer den internen Gebrauch.
// (c) Lars Langhans 29.12.1996 22:10

StopWatch::StopWatch():m_bIsValid(false),m_bIsRunning(false) {}

void StopWatch::start()
{
// pre: %
// post: Start Timer

    m_bIsValid = false;
    m_bIsRunning = true;
    osl_getSystemTime( &t1 );
    printf("# %d %d nsecs\n", t1.Seconds, t1.Nanosec);
    // gettimeofday(&t1, 0);
}

void StopWatch::stop()
{
// pre: Timer should be started
// post: Timer will stopped

    // gettimeofday(&t2, 0);                         // Timer ausfragen
    osl_getSystemTime( &t2 );
    printf("# %d %d nsecs\n", t2.Seconds, t2.Nanosec);

    if (m_bIsRunning)
    {                                // check ob gestartet.
// LLA: old         m_nNanoSec = static_cast<sal_Int32>(t2.Nanosec) - static_cast<sal_Int32>(t1.Nanosec);
// LLA: old         m_nSeconds = static_cast<sal_Int32>(t2.Seconds) - static_cast<sal_Int32>(t1.Seconds);
// LLA: old         if (m_nNanoSec < 0)
// LLA: old         {
// LLA: old             m_nNanoSec += 1000000000;
// LLA: old             m_nSeconds -= 1;
// LLA: old         }
        //m_nNanoSec = t2.Nanosec - t1.Nanosec;
        m_nSeconds = static_cast<sal_Int32>(t2.Seconds) - static_cast<sal_Int32>(t1.Seconds);
        if ( t2.Nanosec > t1.Nanosec )
               m_nNanoSec = static_cast<sal_Int32>(t2.Nanosec) - static_cast<sal_Int32>(t1.Nanosec);
           else
           {
        m_nNanoSec = 1000000000 + static_cast<sal_Int32>(t2.Nanosec) - static_cast<sal_Int32>(t1.Nanosec);
                m_nSeconds -= 1;
    }
    printf("# %d %d nsecs\n", m_nSeconds, m_nNanoSec );
        //if (m_nNanoSec < 0)
        //{
            //m_nNanoSec += 1000000000;
            //m_nSeconds -= 1;
        //}
        m_bIsValid = true;
        m_bIsRunning = false;
    }
}

double StopWatch::getSeconds() const
{
// pre: gueltig = TRUE
// BACK: Zeit in Sekunden.

    double nValue = 0.0;
    if (m_bIsValid)
    {
        nValue = double(m_nNanoSec) / 1000000000.0 + m_nSeconds; // milli micro nano
    }
    return nValue;
}

double StopWatch::getTenthSec() const
{
    double nValue = 0.0;
    if (m_bIsValid)
    {
        nValue = double(m_nNanoSec) / 100000000.0 + m_nSeconds * 10;
    }
    return nValue ;
}

// -----------------------------------------------------------------------------
template <class T>
class ThreadSafeValue
{
    T   m_nFlag;
    Mutex   m_aMutex;
public:
    ThreadSafeValue(T n = 0): m_nFlag(n) {}
    T getValue()
        {
            //block if already acquired by another thread.
            osl::MutexGuard g(m_aMutex);
            return m_nFlag;
        }
    void addValue(T n)
        {
            //only one thread operate on the flag.
            osl::MutexGuard g(m_aMutex);
            m_nFlag += n;
        }
    void acquire() {m_aMutex.acquire();}
    void release() {m_aMutex.release();}
};

// -----------------------------------------------------------------------------
namespace ThreadHelper
{
    // typedef enum {
    //     QUIET=1,
    //     VERBOSE
    // } eSleepVerboseMode;

    void thread_sleep_tenth_sec(sal_Int32 _nTenthSec/*, eSleepVerboseMode nVerbose = VERBOSE*/)
    {
        // if (nVerbose == VERBOSE)
        // {
        //     printf("wait %d tenth seconds. ", _nTenthSec );
        //     fflush(stdout);
        // }
#ifdef WNT      //Windows
        Sleep(_nTenthSec * 100 );
#endif
#if ( defined UNX ) || ( defined OS2 )  //Unix
        TimeValue nTV;
        nTV.Seconds = static_cast<sal_uInt32>( _nTenthSec/10 );
        nTV.Nanosec = ( (_nTenthSec%10 ) * 100000000 );
        osl_waitThread(&nTV);
#endif
        // if (nVerbose == VERBOSE)
        // {
        //     printf("done\n");
        // }
    }

    void outputPriority(oslThreadPriority const& _aPriority)
    {
        // LLA: output the priority
        if (_aPriority == osl_Thread_PriorityHighest)
        {
            printf("Prio is High\n");
        }
        else if (_aPriority == osl_Thread_PriorityAboveNormal)
        {
            printf("Prio is above normal\n");
        }
        else if (_aPriority == osl_Thread_PriorityNormal)
        {
            printf("Prio is normal\n");
        }
        else if (_aPriority == osl_Thread_PriorityBelowNormal)
        {
            printf("Prio is below normal\n");
        }
        else if (_aPriority == osl_Thread_PriorityLowest)
        {
            printf("Prio is lowest\n");
        }
        else
        {
            printf("Prio is unknown\n");
        }
    }
}

/** Simple thread for testing Thread-create.

    Just add 1 of value 0, and after running, result is 1.
 */
class myThread : public Thread
{
    ThreadSafeValue<sal_Int32> m_aFlag;
public:
    sal_Int32 getValue() { return m_aFlag.getValue(); }
protected:
    /** guarded value which initialized 0

        @see ThreadSafeValue
    */
    void SAL_CALL run()
        {
            while(schedule())
            {
                m_aFlag.addValue(1);
                ThreadHelper::thread_sleep_tenth_sec(1);
            }
        }

public:

    virtual void SAL_CALL suspend()
        {
            m_aFlag.acquire();
            ::osl::Thread::suspend();
            m_aFlag.release();
        }

    ~myThread()
        {
            if (isRunning())
            {
                printf("error: not terminated.\n");
            }
        }

};

// -----------------------------------------------------------------------------
/** Thread which has a flag add 1 every second until 20
 */
class OCountThread : public Thread
{
    ThreadSafeValue<sal_Int32> m_aFlag;
public:
    OCountThread()
        {
            m_nWaitSec = 0;
            printf("new OCountThread thread %d!\n", getIdentifier());
        }
    sal_Int32 getValue() { return m_aFlag.getValue(); }

    void setWait(sal_Int32 nSec)
        {
            m_nWaitSec = nSec;
            //m_bWait = sal_True;
        }

    virtual void SAL_CALL suspend()
        {
            m_aFlag.acquire();
            ::osl::Thread::suspend();
            m_aFlag.release();
        }

protected:
    //sal_Bool m_bWait;
    sal_Int32 m_nWaitSec;

    void SAL_CALL run()
        {
            /// if the thread should terminate, schedule return false
            while (m_aFlag.getValue() < 20 && schedule() == sal_True)
            {
                m_aFlag.addValue(1);
                ThreadHelper::thread_sleep_tenth_sec(1);
                // TimeValue nTV;
                // nTV.Seconds = 1;
                // nTV.Nanosec = 0;
                // wait(nTV);

                if (m_nWaitSec != 0)
                {
                    //ThreadHelper::thread_sleep_tenth_sec(m_nWaitSec * 10);
                    TimeValue nTV;
                    nTV.Seconds = m_nWaitSec / 10 ;
                    nTV.Nanosec = ( m_nWaitSec%10 ) * 100000000 ;
                    wait( nTV );
                    m_nWaitSec = 0;
                }
            }
        }
    void SAL_CALL onTerminated()
        {
            printf("normally terminate this thread %d!\n", getIdentifier());
        }
public:

    ~OCountThread()
        {
            if (isRunning())
            {
                printf("error: not terminated.\n");
            }
        }

};

/** call suspend in the run method
*/
class OSuspendThread : public Thread
{
    ThreadSafeValue<sal_Int32> m_aFlag;
public:
    OSuspendThread(){ m_bSuspend = sal_False; }
    sal_Int32 getValue() { return m_aFlag.getValue(); }
    void setSuspend()
        {
            m_bSuspend = sal_True;
        }
    virtual void SAL_CALL suspend()
        {
            m_aFlag.acquire();
            ::osl::Thread::suspend();
            m_aFlag.release();
        }
protected:
    sal_Bool m_bSuspend;
    void SAL_CALL run()
        {
            //if the thread should terminate, schedule return false
            while (schedule() == sal_True)
            {
                m_aFlag.addValue(1);

                ThreadHelper::thread_sleep_tenth_sec(1);
                // m_bWait =    sal_False;
                // TimeValue nTV;
                // nTV.Seconds = 1;
                // nTV.Nanosec = 0;
                // wait(nTV);
                if (m_bSuspend == sal_True)
                {
                    suspend();
                    m_bSuspend  = sal_False;
                }
            }
        }
public:

    ~OSuspendThread()
        {
            if (isRunning())
            {
                printf("error: not terminated.\n");
            }
        }

};

/** no call schedule in the run method
*/
class ONoScheduleThread : public Thread
{
    ThreadSafeValue<sal_Int32> m_aFlag;
public:
    sal_Int32 getValue() { return m_aFlag.getValue(); }

    virtual void SAL_CALL suspend()
        {
            m_aFlag.acquire();
            ::osl::Thread::suspend();
            m_aFlag.release();
        }
protected:
    void SAL_CALL run()
        {
            while (m_aFlag.getValue() < 10)
            {
                m_aFlag.addValue(1);
                ThreadHelper::thread_sleep_tenth_sec(1);
                // TimeValue nTV;
                // nTV.Seconds = 1;
                // nTV.Nanosec = 0;
                // wait(nTV);
            }
        }
    void SAL_CALL onTerminated()
        {
            printf("normally terminate this thread %d!\n", getIdentifier());
        }
public:
    ONoScheduleThread()
        {
                printf("new thread id %d!\n", getIdentifier());
        }
    ~ONoScheduleThread()
        {
            if (isRunning())
            {
                printf("error: not terminated.\n");
            }
        }

};

/**
*/
class OAddThread : public Thread
{
    ThreadSafeValue<sal_Int32> m_aFlag;
public:
    //oslThreadIdentifier m_id, m_CurId;
    OAddThread(){}
    sal_Int32 getValue() { return m_aFlag.getValue(); }

    virtual void SAL_CALL suspend()
        {
            m_aFlag.acquire();
            ::osl::Thread::suspend();
            m_aFlag.release();
        }
protected:
    void SAL_CALL run()
        {
            //if the thread should terminate, schedule return false
            while (schedule() == sal_True)
            {
                m_aFlag.addValue(1);
            }
        }
    void SAL_CALL onTerminated()
        {
            // printf("normally terminate this thread %d!\n", getIdentifier());
        }
public:

    ~OAddThread()
        {
            if (isRunning())
            {
                // printf("error: not terminated.\n");
            }
        }

};

namespace osl_Thread
{

    void resumeAndWaitThread(Thread* _pThread)
    {
        // This functions starts a thread, wait a second and suspends the thread
        // Due to the fact, that a suspend and never run thread never really exists.

        // Note: on UNX, after createSuspended, and then terminate the thread, it performs well;
        // while on Windows, after createSuspended, the thread can not terminate, wait endlessly,
        // so here call resume at first, then call terminate.
#ifdef WNT
        printf("resumeAndWaitThread\n");
        _pThread->resume();
        ThreadHelper::thread_sleep_tenth_sec(1);
#else
        _pThread->resume();
#endif
        // ThreadHelper::thread_sleep_tenth_sec(1);
        // _pThread->suspend();
        // ThreadHelper::thread_sleep_tenth_sec(1);
    }

    // kill a running thread and join it, if it has terminated, do nothing
    void termAndJoinThread(Thread* _pThread)
    {
        _pThread->terminate();

// LLA: Windows feature???, a suspended thread can not terminated, so we have to weak it up
#ifdef WNT
        _pThread->resume();
        ThreadHelper::thread_sleep_tenth_sec(1);
#endif
        printf("#wait for join.\n");
        _pThread->join();
    }
/** Test of the osl::Thread::create method
 */

    class create : public ::testing::Test
    {
    public:

        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class create


    /** Simple create a thread.

        Create a simple thread, it just does add 1 to value(which initialized 0),
        if the thread run, the value should be 1.
    */
    TEST_F(create, create_001)
        {
            myThread* newthread = new myThread();
            sal_Bool bRes = newthread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can not creates a new thread!\n";

            ThreadHelper::thread_sleep_tenth_sec(1);        // wait short
            sal_Bool isRunning = newthread->isRunning();    // check if thread is running
            /// wait for the new thread to assure it has run
            ThreadHelper::thread_sleep_tenth_sec(3);
            sal_Int32 nValue = newthread->getValue();
            /// to assure the new thread has terminated
            termAndJoinThread(newthread);
            delete newthread;

            printf("   nValue = %d\n", nValue);
            printf("isRunning = %d\n", isRunning);

            ASSERT_TRUE(nValue >= 1 && isRunning == sal_True) << "Creates a new thread";

        }

    /** only one running thread per instance, return false if create secondly
     */
    TEST_F(create, create_002)
        {
            myThread* newthread = new myThread();
            sal_Bool res1 = newthread->create();
            sal_Bool res2 = newthread->create();
            printf("In non pro, an assertion should occurred. This behaviour is right.\n");
            termAndJoinThread(newthread);
            delete newthread;

            ASSERT_TRUE(res1 && !res2) << "Creates a new thread: can not create two threads per instance";

        }


    /** Test of the osl::Thread::createSuspended method
    */
    class createSuspended : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class createSuspended

    /** Create a suspended thread, use the same class as create_001

        after create, wait enough time, check the value, if it's still the initial value, pass
    */
    TEST_F(createSuspended, createSuspended_001)
        {
            myThread* newthread = new myThread();
            sal_Bool bRes = newthread->createSuspended();
            ASSERT_TRUE(bRes == sal_True) << "Can not creates a new thread!";

            ThreadHelper::thread_sleep_tenth_sec(1);
            sal_Bool isRunning = newthread->isRunning();
            ThreadHelper::thread_sleep_tenth_sec(3);
            sal_Int32 nValue = newthread->getValue();

            resumeAndWaitThread(newthread);

            termAndJoinThread(newthread);
            delete newthread;

            ASSERT_TRUE(nValue == 0 && isRunning) << "Creates a new suspended thread";
        }
    // LLA: Deadlocked!!!
    TEST_F(createSuspended, createSuspended_002)
        {
            myThread* newthread = new myThread();
            sal_Bool res1 = newthread->createSuspended();
            sal_Bool res2 = newthread->createSuspended();

            resumeAndWaitThread(newthread);

            termAndJoinThread(newthread);

            delete newthread;

            ASSERT_TRUE(res1 && !res2) << "Creates a new thread: can not create two threads per instance";
        }


    /** when the count value equal to or more than 3, suspend the thread.
    */
    void suspendCountThread(OCountThread* _pCountThread)
    {
        sal_Int32 nValue = 0;
        while (1)
        {
            nValue = _pCountThread->getValue();
            if (nValue >= 3)
            {
                _pCountThread->suspend();
                break;
            }
        }
    }

    /** Test of the osl::Thread::suspend method
    */
    class suspend : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class suspend

    /** Use a thread which has a flag added 1 every second

        ALGORITHM:
        create the thread, after running special time, record value of flag, then suspend it,
        wait a long time, check the flag, if it remains unchanged during suspending
    */
    TEST_F(suspend, suspend_001)
        {
            OCountThread* aCountThread = new OCountThread();
            sal_Bool bRes = aCountThread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";
            // the thread run for some seconds, but not terminate
            suspendCountThread( aCountThread );

            // the value just after calling suspend
            sal_Int32 nValue = aCountThread->getValue();       // (2)

            ThreadHelper::thread_sleep_tenth_sec(3);

            // the value after waiting 3 seconds
            sal_Int32 nLaterValue = aCountThread->getValue();    // (3)

            resumeAndWaitThread(aCountThread);
            termAndJoinThread(aCountThread);
            delete aCountThread;

            ASSERT_TRUE(bRes == sal_True && nValue == nLaterValue) << "Suspend the thread";

        }
    /** suspend a thread in it's worker-function, the ALGORITHM is same as suspend_001
         reason of deadlocked I think: no schedule can schedule other threads to go on excuting
     */
    TEST_F(suspend, suspend_002)
        {
#if 0
            OSuspendThread* aThread = new OSuspendThread();
            sal_Bool bRes = aThread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";
            // first the thread run for some seconds, but not terminate
            sal_Int32 nValue = 0;
            //while (1)
            //{
            ThreadHelper::thread_sleep_tenth_sec(3);
            nValue = aThread->getValue();    // (1)
            printf(" getValue is %d !", nValue );
            if (nValue >= 2)
            {
                    aThread->setSuspend();
                    //break;
            }
            //}
            printf(" after while!");
            // the value just after calling suspend
            nValue = aThread->getValue();       // (2)

            ThreadHelper::thread_sleep_tenth_sec(3);
            printf(" after sleep!");
            // the value after waiting 3 seconds
            sal_Int32 nLaterValue = aThread->getValue();        // (3)

            //resumeAndWaitThread(aThread);
            aThread->resume();
            termAndJoinThread(aThread);
            delete aThread;

            ASSERT_TRUE(bRes == sal_True && nValue == nLaterValue) << "Suspend the thread";
#endif
        }


    /** Test of the osl::Thread::resume method
    */
    class resume : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class resume

    /** check if the thread run samely as usual after suspend and resume

        ALGORITHM:
        compare the values before and after suspend, they should be same,
        then compare values before and after resume, the difference should be same as the sleep seconds number
    */
    TEST_F(resume, resume_001)
        {
            OCountThread* pCountThread = new OCountThread();
            sal_Bool bRes = pCountThread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";

            suspendCountThread(pCountThread);

            sal_Int32 nSuspendValue = pCountThread->getValue();  // (2)
            // suspend for 3 seconds
            ThreadHelper::thread_sleep_tenth_sec(3);
            pCountThread->resume();

            ThreadHelper::thread_sleep_tenth_sec(3);
            sal_Int32 nResumeValue = pCountThread->getValue();

            ThreadHelper::thread_sleep_tenth_sec(3);
            sal_Int32 nLaterValue = pCountThread->getValue();

            termAndJoinThread(pCountThread);
            delete pCountThread;

            printf("SuspendValue: %d\n", nSuspendValue);
            printf("ResumeValue:  %d\n", nResumeValue);
            printf("LaterValue:   %d\n", nLaterValue);

            /* LLA: this assumption is no longer relevant: nResumeValue ==  nSuspendValue && */
            ASSERT_TRUE(nLaterValue >= 9 &&
                nResumeValue > nSuspendValue &&
                nLaterValue > nResumeValue) << "Suspend then resume the thread";

        }

    /** Create a suspended thread then resume, check if the thread has run
     */
    TEST_F(resume, resume_002)
        {
            myThread* newthread = new myThread();
            sal_Bool bRes = newthread->createSuspended();
            ASSERT_TRUE(bRes == sal_True) << "Can't create thread!";

            newthread->resume();
            ThreadHelper::thread_sleep_tenth_sec(2);
            sal_Int32 nValue = newthread->getValue();

            termAndJoinThread(newthread);
            delete newthread;

            printf("   nValue = %d\n", nValue);

            ASSERT_TRUE(nValue >= 1) << "Creates a suspended thread, then resume";
        }

    /** Test of the osl::Thread::terminate method
    */
    class terminate : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class terminate

    /** Check after call terminate if the running thread running go on executing

        ALGORITHM:
        before and after call terminate, the values should be the same
    */
    TEST_F(terminate, terminate_001)
        {
            OCountThread* aCountThread = new OCountThread();
            sal_Bool bRes = aCountThread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";

            ThreadHelper::thread_sleep_tenth_sec(2);
            sal_Int32 nValue = aCountThread->getValue();
            aCountThread->terminate();
            ThreadHelper::thread_sleep_tenth_sec(2);
            sal_Int32 nLaterValue = aCountThread->getValue();

            // isRunning should be false after terminate
            sal_Bool isRunning = aCountThread->isRunning();
            aCountThread->join();
            delete aCountThread;

            printf("     nValue = %d\n", nValue);
            printf("nLaterValue = %d\n", nLaterValue);

            ASSERT_TRUE(isRunning == sal_False && nLaterValue >= nValue) << "Terminate the thread";
        }
    /** Check if a suspended thread will terminate after call terminate, different on w32 and on UNX
     */
    TEST_F(terminate, terminate_002)
        {
            OCountThread* aCountThread = new OCountThread();
            sal_Bool bRes = aCountThread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";

            ThreadHelper::thread_sleep_tenth_sec(1);
            suspendCountThread(aCountThread);
            sal_Int32 nValue = aCountThread->getValue();

            // seems a suspended thread can not be terminated on W32, while on Solaris can
            resumeAndWaitThread(aCountThread);

            ThreadHelper::thread_sleep_tenth_sec(2);

            termAndJoinThread(aCountThread);
            sal_Int32 nLaterValue = aCountThread->getValue();
            delete aCountThread;

            printf("     nValue = %d\n", nValue);
            printf("nLaterValue = %d\n", nLaterValue);

            ASSERT_TRUE(nLaterValue > nValue) << "Suspend then resume the thread";
        }


    /** Test of the osl::Thread::join method
    */
    class join : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class join

    /** Check after call terminate if the thread running function will not go on executing

        the next statement after join will not exec before the thread terminate
        ALGORITHM:
        recode system time at the beginning of the thread run, call join, then record system time again,
        the difference of the two time should be equal or more than 20 seconds, the CountThead normally terminate
    */
    TEST_F(join, join_001)
        {
            OCountThread *aCountThread = new OCountThread();
            sal_Bool bRes = aCountThread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";

            StopWatch aStopWatch;
            aStopWatch.start();
            // TimeValue aTimeVal_befor;
            // osl_getSystemTime( &aTimeVal_befor );
            //printf("#join:the system time is %d,%d\n", pTimeVal_befor->Seconds,pTimeVal_befor->Nanosec);

            aCountThread->join();

            //the below line will be executed after aCountThread terminate
            // TimeValue aTimeVal_after;
            // osl_getSystemTime( &aTimeVal_after );
            aStopWatch.stop();
            // sal_uInt32 nSec  = aTimeVal_after.Seconds - aTimeVal_befor.Seconds;
            double nSec = aStopWatch.getSeconds();
            printf("join_001 nSec=%f\n", nSec);
            delete aCountThread;

            ASSERT_TRUE(nSec >= 2) << "Join the thread: after the thread terminate";

        }
    /** after terminated by another thread, join exited immediately

        ALGORITHM:
        terminate the thread when value>=3, call join, check the beginning time and time after join,
        the difference should be 3 seconds, join costs little time
    */
    TEST_F(join, join_002)
        {
            OCountThread *aCountThread = new OCountThread();
            sal_Bool bRes = aCountThread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";

            //record the time when the running begin
            // TimeValue aTimeVal_befor;
            // osl_getSystemTime( &aTimeVal_befor );
            StopWatch aStopWatch;
            aStopWatch.start();

            ThreadHelper::thread_sleep_tenth_sec(10);
            termAndJoinThread(aCountThread);

            //the below line will be executed after aCountThread terminate
            // TimeValue aTimeVal_after;
            // osl_getSystemTime( &aTimeVal_after );
            // sal_uInt32 nSec  = aTimeVal_after.Seconds - aTimeVal_befor.Seconds;
            aStopWatch.stop();
            double nSec = aStopWatch.getSeconds();
            printf("join_002 nSec=%f\n", nSec);

            delete aCountThread;
            ASSERT_TRUE(nSec >= 1) << "Join the thread: after thread terminate by another thread";
        }

    /** Test of the osl::Thread::isRunning method
    */
    class isRunning : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class isRunning

    /**
     */
    TEST_F(isRunning, isRunning_001)
        {
            OCountThread *aCountThread = new OCountThread();
            sal_Bool bRes = aCountThread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";

            sal_Bool bRun = aCountThread->isRunning();

            ThreadHelper::thread_sleep_tenth_sec(2);
            termAndJoinThread(aCountThread);
            sal_Bool bTer = aCountThread->isRunning();
            delete aCountThread;

            ASSERT_TRUE(bRun == sal_True && bTer == sal_False) << "Test isRunning";
        }
    /** check the value of isRunning when suspending and after resume
     */
    TEST_F(isRunning, isRunning_002)
        {
            OCountThread *aCountThread = new OCountThread();
            sal_Bool bRes = aCountThread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";

            // sal_Bool bRunning = aCountThread->isRunning();
            // sal_Int32 nValue = 0;
            suspendCountThread(aCountThread);

            sal_Bool bRunning_sup = aCountThread->isRunning();
            ThreadHelper::thread_sleep_tenth_sec(2);
            aCountThread->resume();
            ThreadHelper::thread_sleep_tenth_sec(2);
            sal_Bool bRunning_res = aCountThread->isRunning();
            termAndJoinThread(aCountThread);
            sal_Bool bRunning_ter = aCountThread->isRunning();
            delete aCountThread;

            ASSERT_TRUE(bRes == sal_True &&
                bRunning_sup == sal_True &&
                bRunning_res == sal_True &&
                bRunning_ter == sal_False) << "Test isRunning";

        }

    /// check osl::Thread::setPriority
    class setPriority : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }

        // insert your test code here.
        rtl::OString getPrioName(oslThreadPriority _aPriority)
            {
                rtl::OString sPrioStr;
                switch (_aPriority)
                {
                case osl_Thread_PriorityHighest:
                    sPrioStr = "Highest";
                    break;

                case osl_Thread_PriorityAboveNormal:
                    sPrioStr = "AboveNormal";

                case osl_Thread_PriorityNormal:
                    sPrioStr = "Normal";

                case osl_Thread_PriorityBelowNormal:
                    sPrioStr = "BelowNormal";
                    break;

                case osl_Thread_PriorityLowest:
                    sPrioStr = "Lowest";
                    break;
                default:
                    sPrioStr = "unknown";
                }
                return sPrioStr;
            }


        /** check 2 threads.

            ALGORITHM:
            Here the function should show, that 2 different threads,
            which only increase a value, should run at the same time with same prio.
            The test fails, if the difference between the two values is more than 5%
            but IMHO this isn't a failure, it's only a feature of the OS.
        */

        void check2Threads(oslThreadPriority _aPriority)
            {
                // initial 5 threads with different priorities
                OAddThread* pThread = new OAddThread();
                OAddThread* p2Thread = new OAddThread();

                //Create them and start running at the same time
                pThread->create();
                pThread->setPriority(_aPriority);
                p2Thread->create();
                p2Thread->setPriority(_aPriority);

                ThreadHelper::thread_sleep_tenth_sec(5);

                pThread->terminate();
                p2Thread->terminate();

                sal_Int32 nValueNormal = 0;
                nValueNormal = pThread->getValue();

                sal_Int32 nValueNormal2 = 0;
                nValueNormal2 = p2Thread->getValue();

                rtl::OString sPrio = getPrioName(_aPriority);
                printf("After 10 tenth seconds\n");

                printf("nValue in %s Prio Thread is  %d\n",sPrio.getStr(), nValueNormal);
                printf("nValue in %s Prio Thread is %d\n", sPrio.getStr(), nValueNormal2);

                // ThreadHelper::thread_sleep_tenth_sec(1);
                pThread->join();
                p2Thread->join();

                delete pThread;
                delete p2Thread;

                sal_Int32 nDelta = abs(nValueNormal - nValueNormal2);
                double nQuotient = std::max(nValueNormal, nValueNormal2);
                ASSERT_TRUE(nQuotient != 0) << "Quotient is zero, which means, there exist no right values.";
                double nDeltaPercent = nDelta / nQuotient * 100;

                printf("Delta value %d, percent %f\n",nDelta, nDeltaPercent);

                // LLA: it's not a bug if the current OS is not able to handle thread scheduling right and good.
                // like Windows XP
                // LLA: ASSERT_TRUE(// LLA:     nDeltaPercent <= 5
                // LLA:) << // LLA:     "Run 2 normal threads, the count diff more than 5 percent.";
            }
    }; // class setPriority

    TEST_F(setPriority, setPriority_001_1)
        {
            check2Threads(osl_Thread_PriorityHighest);
        }
    TEST_F(setPriority, setPriority_001_2)
        {
            check2Threads(osl_Thread_PriorityAboveNormal);
        }
    TEST_F(setPriority, setPriority_001_3)
        {
            check2Threads(osl_Thread_PriorityNormal);
        }
    TEST_F(setPriority, setPriority_001_4)
        {
            check2Threads(osl_Thread_PriorityBelowNormal);
        }
    TEST_F(setPriority, setPriority_001_5)
        {
            check2Threads(osl_Thread_PriorityLowest);
        }

#ifndef SOLARIS
    TEST_F(setPriority, setPriority_002)
        {
            // initial 5 threads with different priorities

            OAddThread aHighestThread;
            OAddThread aAboveNormalThread;
            OAddThread aNormalThread;
            //OAddThread *aBelowNormalThread = new OAddThread();
            //OAddThread *aLowestThread = new OAddThread();

            //Create them and start running at the same time
            aHighestThread.createSuspended();
            aHighestThread.setPriority(osl_Thread_PriorityHighest);

            aAboveNormalThread.createSuspended();
            aAboveNormalThread.setPriority(osl_Thread_PriorityAboveNormal);

            aNormalThread.createSuspended();
            aNormalThread.setPriority(osl_Thread_PriorityNormal);
            /*aBelowNormalThread->create();
              aBelowNormalThread->setPriority(osl_Thread_PriorityBelowNormal);
              aLowestThread->create();
              aLowestThread->setPriority(osl_Thread_PriorityLowest);
            */

            aHighestThread.resume();
            aAboveNormalThread.resume();
            aNormalThread.resume();

            ThreadHelper::thread_sleep_tenth_sec(5);

            aHighestThread.suspend();
            aAboveNormalThread.suspend();
            aNormalThread.suspend();

            termAndJoinThread(&aNormalThread);
            termAndJoinThread(&aAboveNormalThread);
            termAndJoinThread(&aHighestThread);
            //aBelowNormalThread->terminate();
            //aLowestThread->terminate();

            sal_Int32 nValueHighest = 0;
            nValueHighest = aHighestThread.getValue();

            sal_Int32 nValueAboveNormal = 0;
            nValueAboveNormal = aAboveNormalThread.getValue();

            sal_Int32 nValueNormal = 0;
            nValueNormal = aNormalThread.getValue();

            // sal_Int32 nValueBelowNormal = 0;
            //nValueBelowNormal = aBelowNormalThread->getValue();
            // sal_Int32 nValueLowest = 0;
            //nValueLowest = aLowestThread->getValue();
            printf("After 10 tenth seconds\n");
            printf("nValue in Highest Prio Thread is %d\n",nValueHighest);
            printf("nValue in AboveNormal Prio Thread is %d\n",nValueAboveNormal);
            printf("nValue in Normal Prio Thread is %d\n",nValueNormal);

            // LLA: this is not a save test, so we only check if all values not zero
            // LLA: ASSERT_TRUE(// LLA:     nValueHighest >= nValueAboveNormal &&
            // LLA:     nValueAboveNormal >= nValueNormal &&
            // LLA:     nValueNormal > 0
            // LLA:) << // LLA:     "SetPriority";

// LLA: windows let starve threads with lower priority
#ifndef WNT
            ASSERT_TRUE(nValueHighest     > 0 &&
                nValueAboveNormal > 0 &&
                nValueNormal > 0) << "SetPriority";
#endif
        }

    TEST_F(setPriority, setPriority_003)
        {
            // initial 5 threads with different priorities
            OAddThread *pHighestThread = new OAddThread();
            OAddThread *pAboveNormalThread = new OAddThread();
            OAddThread *pNormalThread = new OAddThread();
            OAddThread *pBelowNormalThread = new OAddThread();
            OAddThread *pLowestThread = new OAddThread();

            //Create them and start running at the same time
            pHighestThread->createSuspended();
            pHighestThread->setPriority(osl_Thread_PriorityHighest);

            pAboveNormalThread->createSuspended();
            pAboveNormalThread->setPriority(osl_Thread_PriorityAboveNormal);

            pNormalThread->createSuspended();
            pNormalThread->setPriority(osl_Thread_PriorityNormal);

            pBelowNormalThread->createSuspended();
            pBelowNormalThread->setPriority(osl_Thread_PriorityBelowNormal);

            pLowestThread->createSuspended();
            pLowestThread->setPriority(osl_Thread_PriorityLowest);

            pHighestThread->resume();
            pAboveNormalThread->resume();
            pNormalThread->resume();
            pBelowNormalThread->resume();
            pLowestThread->resume();

            ThreadHelper::thread_sleep_tenth_sec(5);

            pHighestThread->suspend();
            pAboveNormalThread->suspend();
            pNormalThread->suspend();
            pBelowNormalThread->suspend();
            pLowestThread->suspend();

            termAndJoinThread(pHighestThread);
            termAndJoinThread(pAboveNormalThread);
            termAndJoinThread(pNormalThread);
            termAndJoinThread(pBelowNormalThread);
            termAndJoinThread(pLowestThread);

            sal_Int32 nValueHighest = 0;
            nValueHighest = pHighestThread->getValue();

            sal_Int32 nValueAboveNormal = 0;
            nValueAboveNormal = pAboveNormalThread->getValue();

            sal_Int32 nValueNormal = 0;
            nValueNormal = pNormalThread->getValue();

            sal_Int32 nValueBelowNormal = 0;
            nValueBelowNormal = pBelowNormalThread->getValue();

            sal_Int32 nValueLowest = 0;
            nValueLowest = pLowestThread->getValue();

            printf("After 10 tenth seconds\n");
            printf("nValue in Highest Prio Thread is     %d\n",nValueHighest);
            printf("nValue in AboveNormal Prio Thread is %d\n",nValueAboveNormal);
            printf("nValue in Normal Prio Thread is      %d\n",nValueNormal);
            printf("nValue in BelowNormal Prio Thread is %d\n",nValueBelowNormal);
            printf("nValue in Lowest Prio Thread is      %d\n",nValueLowest);

            delete pHighestThread;
            delete pAboveNormalThread;
            delete pNormalThread;
            delete pBelowNormalThread;
            delete pLowestThread;

            // LLA: this is not a save test, so we only check if all values not zero
            // LLA: ASSERT_TRUE(// LLA:     nValueHighest > nValueAboveNormal &&
            // LLA:     nValueAboveNormal > nValueNormal &&
            // LLA:     nValueNormal > nValueBelowNormal &&
            // LLA:     nValueBelowNormal > nValueLowest &&
            // LLA:     nValueLowest > 0
            // LLA:) << // LLA:     "SetPriority";

// LLA: windows let starve threads with lower priority
#ifndef WNT
            ASSERT_TRUE(nValueHighest     > 0 &&
                nValueAboveNormal > 0 &&
                nValueNormal      > 0 &&
                nValueBelowNormal > 0 &&
                nValueLowest      > 0) << "SetPriority";
#endif
        }

    TEST_F(setPriority, setPriority_004)
        {
            // initial 5 threads with different priorities
            // OAddThread *pHighestThread = new OAddThread();
            OAddThread *pAboveNormalThread = new OAddThread();
            OAddThread *pNormalThread = new OAddThread();
            OAddThread *pBelowNormalThread = new OAddThread();
            OAddThread *pLowestThread = new OAddThread();

            //Create them and start running at the same time
            // pHighestThread->createSuspended();
            // pHighestThread->setPriority(osl_Thread_PriorityHighest);

            pAboveNormalThread->createSuspended();
            pAboveNormalThread->setPriority(osl_Thread_PriorityAboveNormal);

            pNormalThread->createSuspended();
            pNormalThread->setPriority(osl_Thread_PriorityNormal);

            pBelowNormalThread->createSuspended();
            pBelowNormalThread->setPriority(osl_Thread_PriorityBelowNormal);

            pLowestThread->createSuspended();
            pLowestThread->setPriority(osl_Thread_PriorityLowest);

            // pHighestThread->resume();
            pAboveNormalThread->resume();
            pNormalThread->resume();
            pBelowNormalThread->resume();
            pLowestThread->resume();

            ThreadHelper::thread_sleep_tenth_sec(5);

            // pHighestThread->suspend();
            pAboveNormalThread->suspend();
            pNormalThread->suspend();
            pBelowNormalThread->suspend();
            pLowestThread->suspend();

            // termAndJoinThread(pHighestThread);
            termAndJoinThread(pAboveNormalThread);
            termAndJoinThread(pNormalThread);
            termAndJoinThread(pBelowNormalThread);
            termAndJoinThread(pLowestThread);

            // sal_Int32 nValueHighest  = 0;
            // nValueHighest =  pHighestThread->getValue();

            sal_Int32 nValueAboveNormal = 0;
            nValueAboveNormal = pAboveNormalThread->getValue();

            sal_Int32 nValueNormal = 0;
            nValueNormal = pNormalThread->getValue();

            sal_Int32 nValueBelowNormal = 0;
            nValueBelowNormal = pBelowNormalThread->getValue();

            sal_Int32 nValueLowest = 0;
            nValueLowest = pLowestThread->getValue();

            printf("After 5 tenth seconds\n");
            // printf("nValue in Highest Prio Thread  is     %d\n",nValueHighest);
            printf("nValue in AboveNormal Prio Thread is %d\n",nValueAboveNormal);
            printf("nValue in Normal Prio Thread is      %d\n",nValueNormal);
            printf("nValue in BelowNormal Prio Thread is %d\n",nValueBelowNormal);
            printf("nValue in Lowest Prio Thread is      %d\n",nValueLowest);

            // delete pHighestThread;
            delete pAboveNormalThread;
            delete pNormalThread;
            delete pBelowNormalThread;
            delete pLowestThread;

            // LLA: this is not a save test, so we only check if all values not zero
            // LLA: ASSERT_TRUE(// LLA:     nValueHighest > nValueAboveNormal &&
            // LLA:     nValueAboveNormal > nValueNormal &&
            // LLA:     nValueNormal > nValueBelowNormal &&
            // LLA:     nValueBelowNormal > nValueLowest &&
            // LLA:     nValueLowest > 0
            // LLA:) << // LLA:     "SetPriority";

// LLA: windows let starve threads with lower priority
#ifndef WNT
            ASSERT_TRUE(/* nValueHighest     > 0 &&  */
                nValueAboveNormal > 0 &&
                nValueNormal      > 0 &&
                nValueBelowNormal > 0 &&
                nValueLowest      > 0) << "SetPriority";
#endif
        }
    TEST_F(setPriority, setPriority_005)
        {
            // initial 5 threads with different priorities
            // OAddThread *pHighestThread = new OAddThread();
            // OAddThread *pAboveNormalThread = new OAddThread();
            OAddThread *pNormalThread = new OAddThread();
            OAddThread *pBelowNormalThread = new OAddThread();
            OAddThread *pLowestThread = new OAddThread();

            //Create them and start running at the same time
            // pHighestThread->createSuspended();
            // pHighestThread->setPriority(osl_Thread_PriorityHighest);

            // pAboveNormalThread->createSuspended();
            // pAboveNormalThread->setPriority(osl_Thread_PriorityAboveNormal);

            pNormalThread->createSuspended();
            pNormalThread->setPriority(osl_Thread_PriorityNormal);

            pBelowNormalThread->createSuspended();
            pBelowNormalThread->setPriority(osl_Thread_PriorityBelowNormal);

            pLowestThread->createSuspended();
            pLowestThread->setPriority(osl_Thread_PriorityLowest);

            // pHighestThread->resume();
            // pAboveNormalThread->resume();
            pNormalThread->resume();
            pBelowNormalThread->resume();
            pLowestThread->resume();

            ThreadHelper::thread_sleep_tenth_sec(5);

            // pHighestThread->suspend();
            // pAboveNormalThread->suspend();
            pNormalThread->suspend();
            pBelowNormalThread->suspend();
            pLowestThread->suspend();

            // termAndJoinThread(pHighestThread);
            // termAndJoinThread(pAboveNormalThread);
            termAndJoinThread(pNormalThread);
            termAndJoinThread(pBelowNormalThread);
            termAndJoinThread(pLowestThread);

            // sal_Int32 nValueHighest  = 0;
            // nValueHighest =  pHighestThread->getValue();

            // sal_Int32 nValueAboveNormal = 0;
            // nValueAboveNormal = pAboveNormalThread->getValue();

            sal_Int32 nValueNormal = 0;
            nValueNormal = pNormalThread->getValue();

            sal_Int32 nValueBelowNormal = 0;
            nValueBelowNormal = pBelowNormalThread->getValue();

            sal_Int32 nValueLowest = 0;
            nValueLowest = pLowestThread->getValue();

            printf("After 5 tenth seconds\n");
            // printf("nValue in Highest Prio Thread  is     %d\n",nValueHighest);
            // printf("nValue in AboveNormal  Prio Thread is %d\n",nValueAboveNormal);
            printf("nValue in Normal Prio Thread is      %d\n",nValueNormal);
            printf("nValue in BelowNormal Prio Thread is %d\n",nValueBelowNormal);
            printf("nValue in Lowest Prio Thread is      %d\n",nValueLowest);

            // delete pHighestThread;
            // delete pAboveNormalThread;
            delete pNormalThread;
            delete pBelowNormalThread;
            delete pLowestThread;

            // LLA: this is not a save test, so we only check if all values not zero
            // LLA: ASSERT_TRUE(// LLA:     nValueHighest > nValueAboveNormal &&
            // LLA:     nValueAboveNormal > nValueNormal &&
            // LLA:     nValueNormal > nValueBelowNormal &&
            // LLA:     nValueBelowNormal > nValueLowest &&
            // LLA:     nValueLowest > 0
            // LLA:) << // LLA:     "SetPriority";

// LLA: windows let starve threads with lower priority
#ifndef WNT
            ASSERT_TRUE(/* nValueHighest     > 0 &&  */
                /* nValueAboveNormal > 0 &&  */
                nValueNormal      > 0 &&
                nValueBelowNormal > 0 &&
                nValueLowest      > 0) << "SetPriority";
#endif
        }
#endif // SOLARIS


    /** Test of the osl::Thread::getPriority method
    */
    class getPriority : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class getPriority

    TEST_F(getPriority, getPriority_001)
        {
            OAddThread *pHighestThread = new OAddThread();

            //Create them and start running at the same time
            pHighestThread->create();
            pHighestThread->setPriority(osl_Thread_PriorityHighest);

            oslThreadPriority aPriority = pHighestThread->getPriority();
            termAndJoinThread(pHighestThread);
            delete pHighestThread;

            ThreadHelper::outputPriority(aPriority);

// LLA: Priority settings may not work within some OS versions.
#if ( defined WNT ) || ( defined SOLARIS )
            ASSERT_TRUE(aPriority == osl_Thread_PriorityHighest) << "getPriority";
#else
// LLA: Linux
// NO_PTHREAD_PRIORITY ???
            ASSERT_TRUE(aPriority == osl_Thread_PriorityNormal) << "getPriority";
#endif
        }

    TEST_F(getPriority, getPriority_002)
        {

        }


    class getIdentifier : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class getIdentifier

    // insert your test code here.
    TEST_F(getIdentifier, getIdentifier_001)
        {

        }

    TEST_F(getIdentifier, getIdentifier_002)
        {

        }

    /** Test of the osl::Thread::getCurrentIdentifier method
    */
    class getCurrentIdentifier : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class getCurrentIdentifier

    // insert your test code here.
    TEST_F(getCurrentIdentifier, getCurrentIdentifier_001)
        {
            oslThreadIdentifier oId;
            OCountThread* pCountThread = new OCountThread;
            //OCountThread* pCountThread2 = new OCountThread;
            pCountThread->create();
            //pCountThread2->create();
            pCountThread->setWait(3);
            oId = Thread::getCurrentIdentifier();
            oslThreadIdentifier oIdChild = pCountThread->getIdentifier();
            //printf("CurrentId is %ld, Child1 id is %ld, Child2 id is %ld\n",oId, oIdChild,pCountThread2->m_id );
            termAndJoinThread(pCountThread);
            delete pCountThread;
            //termAndJoinThread(pCountThread2);
            //delete pCountThread2;

            ASSERT_TRUE(oId != oIdChild) << "Get the identifier for the current active thread.";

        }

    TEST_F(getCurrentIdentifier, getCurrentIdentifier_002)
        {
        }


    /** Test of the osl::Thread::wait method
    */
    class wait : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class wait


    /** call wait in the run method

        ALGORITHM:
        tested thread wait nWaitSec seconds, main thread sleep (2) seconds,
        then terminate the tested thread, due to the fact that the thread do a sleep(1) + wait(5)
        it's finish after 6 seconds.
    */
    TEST_F(wait, wait_001)
        {
            OCountThread *aCountThread = new OCountThread();
            sal_Int32 nWaitSec = 5;
            aCountThread->setWait(nWaitSec);
            // thread runs at least 5 seconds.
            sal_Bool bRes = aCountThread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";

            //record the time when the running begin
            StopWatch aStopWatch;
            aStopWatch.start();

            // wait a little bit, to let the thread the time, to start
            ThreadHelper::thread_sleep_tenth_sec( 4 );

            // if wait works,
            // this function returns, after 4 sec. later
            termAndJoinThread(aCountThread);

            // value should be one.
            sal_Int32 nValue = aCountThread->getValue();

            aStopWatch.stop();

            // sal_uInt32 nSec  = aTimeVal_after.Seconds - aTimeVal_befor.Seconds;
            double nTenthSec = aStopWatch.getTenthSec();
            double nSec = aStopWatch.getSeconds();
            delete aCountThread;
            printf("nTenthSec = %f \n", nTenthSec);
            printf("nSec = %f \n", nSec);
            printf("nValue = %d \n", nValue);

            ASSERT_TRUE(nTenthSec >= 5 && nValue == 1) << "Wait: Blocks the calling thread for the given number of time.";

        }
// LLA: wait_001 does the same.
// LLA:         /** wait then terminate the thread
// LLA:
// LLA:         ALGORITHM:
// LLA:         wait nWaitSec seconds, and terminate when the wait does not finish
// LLA:         Windows & UNX: thread terminates immediatlly
// LLA:     */
// LLA:     TEST_F(wait, wait_002)
// LLA:     {
// LLA:         OCountThread aThread;
// LLA:
// LLA:         sal_Int32 nWaitSec = 3;
// LLA:         aThread.setWait(nWaitSec);
// LLA:
// LLA:         sal_Bool bRes = aThread.create();
// LLA:         ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";
// LLA:
// LLA:         StopWatch aStopWatch;
// LLA:         // TimeValue aTimeVal_befor;
// LLA:         // osl_getSystemTime( &aTimeVal_befor );
// LLA:         aStopWatch.start();
// LLA:
// LLA:         termAndJoinThread(&aThread);
// LLA:         sal_Int32 nValue = aThread.getValue();
// LLA:
// LLA:         // TimeValue aTimeVal_after;
// LLA:         // osl_getSystemTime( &aTimeVal_after );
// LLA:         aStopWatch.stop();
// LLA:         // sal_Int32 nSec = aTimeVal_after.Seconds  - aTimeVal_befor.Seconds;
// LLA:         double nSec = aStopWatch.getSeconds();
// LLA:         printf("sec=%f\n", nSec);
// LLA:         printf("nValue = %d\n", nValue);
// LLA:
// LLA:         ASSERT_TRUE(// LLA:             nSec < 1 && nValue == 0
// LLA:) << // LLA:             "Wait: Blocks the calling thread for the given number of time.";
// LLA:     }
    /** osl::Thread::yield method: can not design good test scenario to test up to now
    */
    class yield : public ::testing::Test
    {
    public:
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class yield

    // insert your test code here.
    TEST_F(yield, yield_001)
        {
        }

    /** Test of the osl::Thread::schedule method
    */
    class schedule : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class schedule

    /** The requested thread will get terminate the next time schedule() is called.

        Note: on UNX, if call suspend thread is not the to be suspended thread, the to be
        suspended   thread will get suspended the next time schedule() is called,
        while on w32, it's nothing with schedule.

        check if suspend and terminate work well via schedule
    */
    TEST_F(schedule, schedule_001)
        {
            OAddThread* aThread = new OAddThread();
            sal_Bool bRes = aThread->create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";

            ThreadHelper::thread_sleep_tenth_sec(2);
            aThread->suspend();
            ThreadHelper::thread_sleep_tenth_sec(1);
            sal_Int32 nValue = aThread->getValue();
            ThreadHelper::thread_sleep_tenth_sec(3);
            sal_Int32 nLaterValue = aThread->getValue();
            // resumeAndWaitThread(aThread);
            printf("      value = %d\n", nValue);
            printf("later value = %d\n", nLaterValue);
            // if value and latervalue not equal, than the thread would not suspended

            ASSERT_TRUE(nLaterValue == nValue) << "Schedule: suspend works.";

            aThread->resume();
            ThreadHelper::thread_sleep_tenth_sec(2);

            aThread->terminate();
            sal_Int32 nValue_term = aThread->getValue();

            aThread->join();
            sal_Int32 nValue_join = aThread->getValue();

            printf("value after term = %d\n", nValue_term);
            printf("value after join = %d\n", nValue_join);

            // nValue_term and nValue_join should be the same
            // but should be differ from nValue

            delete aThread;
            //check if thread really terminate after call terminate, if join immediatlly return
            ASSERT_TRUE(nValue_join -  nValue_term <= 1 && nValue_join -  nValue_term >= 0) << "Schedule: Returns False if the thread should terminate.";

        }

    /** design a thread that has not call schedule in the workfunction--run method
     */
    TEST_F(schedule, schedule_002)
        {
            ONoScheduleThread aThread; // this thread runs 10 sec. (no schedule() used)
            sal_Bool bRes = aThread.create();
            ASSERT_TRUE(bRes == sal_True) << "Can't start thread!";

            ThreadHelper::thread_sleep_tenth_sec(2);
            aThread.suspend();
            sal_Int32 nValue = aThread.getValue();

            ThreadHelper::thread_sleep_tenth_sec(3);
            sal_Int32 nLaterValue = aThread.getValue();
            ThreadHelper::thread_sleep_tenth_sec(5);

            resumeAndWaitThread(&aThread);

            printf("      value = %d\n", nValue);
            printf("later value = %d\n", nLaterValue);

            //On windows, suspend works, so the values are same
#ifdef WNT
            ASSERT_TRUE(nLaterValue == nValue) << "Schedule: don't schedule in thread run method, suspend works.";
#endif

            //On UNX, suspend does not work, so the difference of the values equals to sleep seconds number
#ifdef UNX
            aThread.resume();
            ASSERT_TRUE(nLaterValue > nValue) << "Schedule: don't schedule in thread run method, suspend does not work too.";
#endif

            // terminate will not work if no schedule in thread's work function
            termAndJoinThread(&aThread);
            sal_Int32 nValue_term = aThread.getValue();

            printf(" value term = %d\n", nValue_term);

            ASSERT_TRUE(nValue_term == 10) << "Schedule: don't schedule in thread run method, terminate failed.";
        }

} // namespace osl_Thread


// -----------------------------------------------------------------------------
// destroy function when the binding thread terminate
void SAL_CALL destroyCallback(void * data)
{
    printf("destroying local data %s\n", (char *) data);
    delete[] (char *) data;
}

static ThreadData myThreadData(destroyCallback);

/**
*/
class myKeyThread : public Thread
{
public:
    // a public char member for test result checking
    char m_Char_Test;
    // for pass thread-special data to thread
    myKeyThread(const char cData)
        {
            m_nData = cData;
        }
private:
    char m_nData;

    void SAL_CALL run()
        {
            char * pc = new char[2];
//      strcpy(pc, &m_nData);
            memcpy(pc, &m_nData, 1);
            pc[1] = '\0';

            myThreadData.setData(pc);
            char* pData = (char*)myThreadData.getData();
            m_Char_Test = *pData;
            // wait for long time to check the data value in main thread
            ThreadHelper::thread_sleep_tenth_sec(3);
        }
public:
    ~myKeyThread()
        {
            if (isRunning())
            {
                printf("error: not terminated.\n");
            }
        }
};

static ThreadData idData;

class idThread: public Thread
{
public:
    oslThreadIdentifier m_Id;
private:
    void SAL_CALL run()
        {
            oslThreadIdentifier* pId = new oslThreadIdentifier;
            *pId = getIdentifier();
            idData.setData(pId);
            oslThreadIdentifier* pIdData = (oslThreadIdentifier*)idData.getData();
            //printf("Thread %d has Data %d\n", getIdentifier(), *pIdData);
            m_Id = *pIdData;
            delete pId;
        }

public:
    ~idThread()
        {
            if (isRunning())
            {
                printf("error: not terminated.\n");
            }
        }
};

namespace osl_ThreadData
{

    class ctors : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class ctors


    // insert your test code here.
    TEST_F(ctors, ctor_001)
        {

        }

    class setData : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class setData

    /** the same instance of the class can have different values in different threads
     */
    TEST_F(setData, setData_001)
        {
            idThread aThread1;
            aThread1.create();
            idThread aThread2;
            aThread2.create();

            aThread1.join();
            aThread2.join();

            oslThreadIdentifier aThreadId1 = aThread1.getIdentifier();
            oslThreadIdentifier aThreadId2 = aThread2.getIdentifier();

            ASSERT_TRUE(aThread1.m_Id == aThreadId1 && aThread2.m_Id == aThreadId2) << "ThreadData setData: ";

        }

    TEST_F(setData, setData_002)
        {
            // at first, set the data a value
            char* pc = new char[2];
            char m_nData = 'm';
// LLA: this is a copy functions only and really only for \0 terminated strings
//      m_nData is not a string, it's a character
//          strcpy(pc, &m_nData);
            memcpy(pc, &m_nData, 1);
            pc[1] = '\0';

            myThreadData.setData(pc);

            myKeyThread aThread1('a');
            aThread1.create();
            myKeyThread aThread2('b');
            aThread2.create();
            // aThread1 and aThread2 should have not terminated yet, check current data, not 'a' 'b'
            char* pChar = (char*)myThreadData.getData();
            char aChar = *pChar;

            aThread1.join();
            aThread2.join();

            // the saved thread data of aThread1 & aThread2, different
            char cData1 = aThread1.m_Char_Test;
            char cData2 = aThread2.m_Char_Test;

            ASSERT_TRUE(cData1 == 'a' && cData2 == 'b' && aChar == 'm') << "ThreadData setData: ";

        }
    /** setData the second time, and then getData
     */
    TEST_F(setData, setData_003)
        {
            // at first, set the data a value
            char* pc = new char[2];
            char m_nData = 'm';
//          strcpy(pc, &m_nData);
            memcpy(pc, &m_nData, 1);
            pc[1] = '\0';
            myThreadData.setData(pc);

            myKeyThread aThread1('a');
            aThread1.create();
            myKeyThread aThread2('b');
            aThread2.create();
            // aThread1 and aThread2 should have not terminated yet
            // setData the second time
            char* pc2 = new char[2];
            m_nData = 'o';
//          strcpy(pc2, &m_nData);
            memcpy(pc2, &m_nData, 1);
            pc2[1] = '\0';

            myThreadData.setData(pc2);
            char* pChar = (char*)myThreadData.getData();
            char aChar = *pChar;

            aThread1.join();
            aThread2.join();

            // the saved thread data of aThread1 & aThread2, different
            char cData1 = aThread1.m_Char_Test;
            char cData2 = aThread2.m_Char_Test;

            ASSERT_TRUE(cData1 == 'a' && cData2 == 'b' && aChar == 'o') << "ThreadData setData: ";

        }

    //sal_Bool buildTwoThreads(char)

    class getData : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    }; // class getData

    // After setData in child threads, get Data in the main thread, should be independent
    TEST_F(getData, getData_001)
        {
            char* pc = new char[2];
            char m_nData[] = "i";
            strcpy(pc, m_nData);
            printf("pc %s\n", pc);
            myThreadData.setData(pc);

            myKeyThread aThread1('c');
            aThread1.create();
            myKeyThread aThread2('d');
            aThread2.create();

            aThread1.join();
            aThread2.join();

            char cData1 = aThread1.m_Char_Test;
            char cData2 = aThread2.m_Char_Test;

            char* pChar = (char*)myThreadData.getData();
            char aChar = *pChar;

            ASSERT_TRUE(cData1 == 'c' && cData2 == 'd' && aChar == 'i') << "ThreadData setData: ";

        }

    // setData then change the value in the address data pointer points,
    // and then getData, should get the new value
    TEST_F(getData, getData_002)
        {
            char* pc = new char[2];
            char m_nData = 'i';
//          strcpy(pc, &m_nData);
            memcpy(pc, &m_nData, 1);
            pc[1] = '\0';
//          strncpy(pc, &m_nData, sizeof(char);

            printf("pc %s\n", pc);
            myThreadData.setData(pc);

            myKeyThread aThread1('a');
            aThread1.create();
            myKeyThread aThread2('b');
            aThread2.create();

            // change the value which pc points
            char m_nData2 = 'j';
            // strcpy(pc, &m_nData2);
            memcpy(pc, &m_nData2, 1);
            pc[1] = '\0';

            //printf("pc %s\n", pc);
            void* pChar = myThreadData.getData();
            char aChar = *(char*)pChar;

            aThread1.join();
            aThread2.join();

            char cData1 = aThread1.m_Char_Test;
            char cData2 = aThread2.m_Char_Test;

            ASSERT_TRUE(cData1 == 'a' && cData2 == 'b' && aChar == 'j') << "ThreadData setData: ";

        }


} // namespace osl_ThreadData

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
