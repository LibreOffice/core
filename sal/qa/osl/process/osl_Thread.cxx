/*************************************************************************
 *
 *  $RCSfile: osl_Thread.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $  $Date: 2003-05-27 14:19:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *     - GNU Lesser General Public License Version 2.1
 *     - Sun Industry Standards Source License Version 1.1
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
//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#include <cppunit/simpleheader.hxx>

using namespace osl;
using namespace rtl;

#ifdef UNX
#include <unistd.h>
#endif
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
};

// -----------------------------------------------------------------------------
namespace ThreadHelper
{
void thread_sleep(sal_Int32 _nSec)
{
    printf("# wait %d seconds. ", _nSec);
    fflush(stdout);
    #ifdef WNT      //Windows
        Sleep(_nSec * 1000 );
    #endif
    #if ( defined UNX ) || ( defined OS2 )  //Unix
        sleep(_nSec);
    #endif
    printf("# done\n");
}
}

/** Simple thread for testing Thread-create.

    Just add 1 of value 0, and after running, result is 1.
 */
class myThread : public Thread
{
public:
    sal_Int32 getValue() { return m_aFlag.getValue(); }
protected:
    /** guarded value which initialized 0

    @see ThreadSafeValue
    */
    ThreadSafeValue<sal_Int32> m_aFlag;
    void SAL_CALL run()
    {
        m_aFlag.addValue(1);
    }
public:

    ~myThread()
    {
        if (isRunning())
        {
        printf("# error: not terminated.\n");
        }
    }

};

// -----------------------------------------------------------------------------
/** Thread which has a flag add 1 every second until 20
*/
class OCountThread : public Thread
{
public:
    OCountThread()
    {
        //m_bWait = sal_False;
        m_nWaitSec = 0;
        //m_id = getIdentifier();
    }
    sal_Int32 getValue() { return m_aFlag.getValue(); }

    void setWait(sal_Int32 nSec)
    {
        m_nWaitSec = nSec;
        //m_bWait = sal_True;
    }

protected:
    ThreadSafeValue<sal_Int32> m_aFlag;
    //sal_Bool m_bWait;
    sal_Int32 m_nWaitSec;

    void SAL_CALL run()
    {
        /// if the thread should terminate, schedule return false
        while (m_aFlag.getValue() < 20 && schedule() == sal_True)
        {
        m_aFlag.addValue(1);
        ThreadHelper::thread_sleep(1);

        if (m_nWaitSec != 0)
        {
            //m_bWait = sal_False;
            TimeValue nTV;
            nTV.Seconds = m_nWaitSec;
            nTV.Nanosec = 0;
            wait(nTV);
            m_nWaitSec = 0;
        }
        }
    }
    void SAL_CALL onTerminated()
    {
        printf("# normally terminate this thread %d!\n", getIdentifier());
    }
public:

    ~OCountThread()
    {
        if (isRunning())
        {
        printf("# error: not terminated.\n");
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
protected:
    sal_Bool m_bSuspend;
    void SAL_CALL run()
    {
        //if the thread should terminate, schedule return false
        while (schedule() == sal_True)
        {
        m_aFlag.addValue(1);
        ThreadHelper::thread_sleep(1);
        if (m_bSuspend == sal_True)
        {
             suspend();
             m_bSuspend = sal_False;
        }
        }
    }
public:

    ~OSuspendThread()
    {
        if (isRunning())
        {
        printf("# error: not terminated.\n");
        }
    }

};

/** no call schedule in the run method
*/
class ONoScheduleThread : public Thread
{
    ThreadSafeValue<sal_Int32> m_aFlag;
public:
    ONoScheduleThread(){}
    sal_Int32 getValue() { return m_aFlag.getValue(); }

protected:
    void SAL_CALL run()
    {
        while (m_aFlag.getValue() < 10)
        {
        m_aFlag.addValue(1);
                ThreadHelper::thread_sleep(1);
        }
    }
    void SAL_CALL onTerminated()
    {
        printf("# normally terminate this thread %d!\n", getIdentifier());
    }
public:

    ~ONoScheduleThread()
    {
        if (isRunning())
        {
        printf("# error: not terminated.\n");
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
        printf("# normally terminate this thread %d!\n", getIdentifier());
    }
public:

    ~OAddThread()
    {
        if (isRunning())
        {
        printf("# error: not terminated.\n");
        }
    }

};

namespace osl_Thread
{
/** Test of the osl::Thread::create method
*/

    class create : public CppUnit::TestFixture
    {
    public:

    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }

    /** Simple create a thread.

        Create a simple thread, it just does add 1 to value(which initialized 0),
        if the thread run, the value should be 1.
     */
    void create_001()
    {
        myThread* newthread = new myThread();
        sal_Bool bRes = newthread->create();
        CPPUNIT_ASSERT_MESSAGE("Can not creates a new thread!\n", bRes == sal_True );

        sal_Bool isRunning = newthread->isRunning();
        /// wait for the new thread to assure it has run
        ThreadHelper::thread_sleep(3);
        sal_Int32 nValue = newthread->getValue();
        /// to assure the new thread has terminated
        newthread->join();
        delete newthread;

        CPPUNIT_ASSERT_MESSAGE(
            "Creates a new thread",
            nValue == 1 && isRunning == sal_True
            );

    }

    /** only one running thread per instance, return false if create secondly
    */
    void create_002()
    {
        myThread* newthread = new myThread();
        sal_Bool res1 = newthread->create();
        sal_Bool res2 = newthread->create();

        newthread->join();
        delete newthread;

        CPPUNIT_ASSERT_MESSAGE(
            "Creates a new thread: can not create two threads per instance",
            res1 && !res2
            );

    }

    CPPUNIT_TEST_SUITE(create);
    CPPUNIT_TEST(create_001);
    CPPUNIT_TEST(create_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class create


    void resumeAndWaitThread(Thread* _pThread)
    {
        // This functions starts a thread, wait a second and suspends the thread
        // Due to the fact, that a suspend and never run thread never really exists.

        // Note: on UNX, after createSuspended, and then terminate the thread, it performs well;
        // while on Windows, after createSuspended, the thread can not terminate, wait endlessly,
        // so here call resume at first, then call terminate.
        #ifdef WNT
            printf("# resumeAndWaitThread\n");
            _pThread->resume();
        #endif
        // ThreadHelper::thread_sleep(1);
        // _pThread->suspend();
        // ThreadHelper::thread_sleep(1);
    }

    // kill a running thread and join it, if it has terminated, do nothing
    void termAndJoinThread(Thread* _pThread)
    {
        _pThread->terminate();
        printf("#wait for join.\n");
        _pThread->join();
    }

    /** Test of the osl::Thread::createSuspended method
    */
    class createSuspended : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }

    /** Create a suspended thread, use the same class as create_001

        after create, wait enough time, check the value, if it's still the initial value, pass
    */
    void createSuspended_001()
        {
        myThread* newthread = new myThread();
        sal_Bool bRes = newthread->createSuspended();
        CPPUNIT_ASSERT_MESSAGE("Can not creates a new thread!", bRes == sal_True );

        sal_Bool isRunning = newthread->isRunning();
        ThreadHelper::thread_sleep(5);
        sal_Int32 nValue = newthread->getValue();

        resumeAndWaitThread(newthread);

        termAndJoinThread(newthread);
        delete newthread;

        CPPUNIT_ASSERT_MESSAGE(
            "Creates a new suspended thread",
            nValue == 0 && isRunning
            );
        }

    void createSuspended_002()
        {
        myThread* newthread = new myThread();
        sal_Bool res1 = newthread->createSuspended();
        sal_Bool res2 = newthread->createSuspended();

        resumeAndWaitThread(newthread);

        termAndJoinThread(newthread);

        delete newthread;

        CPPUNIT_ASSERT_MESSAGE(
            "Creates a new thread: can not create two threads per instance",
            res1 && !res2
            );
        }

    CPPUNIT_TEST_SUITE(createSuspended);
    CPPUNIT_TEST(createSuspended_001);
    CPPUNIT_TEST(createSuspended_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class createSuspended

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
    class suspend : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    /** Use a thread which has a flag added 1 every second

        ALGORITHM:
        create the thread, after running special time, record value of flag, then suspend it,
        wait a long time, check the flag, if it remains unchanged during suspending
    */
    void suspend_001()
    {
        OCountThread* aCountThread = new OCountThread();
        sal_Bool bRes = aCountThread->create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );
        // the thread run for some seconds, but not terminate
        suspendCountThread(aCountThread);

        // the value just after calling suspend
        sal_Int32 nValue = aCountThread->getValue();       // (2)

        ThreadHelper::thread_sleep(3);

        // the value after waiting 3 seconds
        sal_Int32 nLaterValue = aCountThread->getValue();    // (3)

        resumeAndWaitThread(aCountThread);
        termAndJoinThread(aCountThread);
        delete aCountThread;

        CPPUNIT_ASSERT_MESSAGE(
            "Suspend the thread",
            bRes == sal_True && nValue == nLaterValue
        );

    }
    /** suspend a thread in it's worker-function, the ALGORITHM is same as suspend_001
    */
    void suspend_002()
    {
        OSuspendThread* aThread = new OSuspendThread();
        sal_Bool bRes = aThread->create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );
        // first the thread run for some seconds, but not terminate
        sal_Int32 nValue = 0;
        while (1)
        {
            nValue = aThread->getValue();    // (1)
            if (nValue >= 3)
            {
                aThread->setSuspend();
                break;
            }
        }
        // the value just after calling suspend
        nValue = aThread->getValue();       // (2)

        ThreadHelper::thread_sleep(3);

        // the value after waiting 3 seconds
        sal_Int32 nLaterValue = aThread->getValue();        // (3)

        resumeAndWaitThread(aThread);
        termAndJoinThread(aThread);
        delete aThread;

        CPPUNIT_ASSERT_MESSAGE(
            "Suspend the thread",
            bRes == sal_True && nValue == nLaterValue
        );
    }

    CPPUNIT_TEST_SUITE(suspend);
    CPPUNIT_TEST(suspend_001);
    CPPUNIT_TEST(suspend_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class suspend

    /** Test of the osl::Thread::resume method
    */
    class resume : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    /** check if the thread run samely as usual after suspend and resume

        ALGORITHM:
        compare the values before and after suspend, they should be same,
        then compare values before and after resume, the difference should be same as the sleep seconds number
    */
    void resume_001()
    {
        OCountThread* aCountThread = new OCountThread();
        sal_Bool bRes = aCountThread->create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

        suspendCountThread(aCountThread);

        sal_Int32 nSuspendValue = aCountThread->getValue();  // (2)
        // suspend for 3 seconds
        ThreadHelper::thread_sleep(3);
        aCountThread->resume();

        sal_Int32 nResumeValue = aCountThread->getValue();
        ThreadHelper::thread_sleep(3);
        sal_Int32 nLaterValue = aCountThread->getValue();

        termAndJoinThread(aCountThread);
        delete aCountThread;

        CPPUNIT_ASSERT_MESSAGE(
            "Suspend then resume the thread",
            bRes == sal_True && nLaterValue <= 6 && nResumeValue == nSuspendValue &&
            nLaterValue - nResumeValue > 1 && nLaterValue - nResumeValue <= 3
        );

    }

        /** Create a suspended thread then resume, check if the thread has run
        */
    void resume_002()
    {
        myThread* newthread = new myThread();
        sal_Bool bRes = newthread->createSuspended();
        CPPUNIT_ASSERT_MESSAGE ( "Can't create thread!", bRes == sal_True );

        newthread->resume();
        ThreadHelper::thread_sleep(2);
        sal_Int32 nValue = newthread->getValue();
        delete newthread;

        CPPUNIT_ASSERT_MESSAGE(
            "Creates a suspended thread, then resume",
            nValue == 1
        );
    }

    CPPUNIT_TEST_SUITE(resume);
    CPPUNIT_TEST(resume_001);
    CPPUNIT_TEST(resume_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class resume

    /** Test of the osl::Thread::terminate method
    */
    class terminate : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    /** Check after call terminate if the running thread running go on executing

        ALGORITHM:
        before and after call terminate, the values should be the same
    */
    void terminate_001()
    {
        OCountThread* aCountThread = new OCountThread();
        sal_Bool bRes = aCountThread->create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

        ThreadHelper::thread_sleep(2);
        sal_Int32 nValue = aCountThread->getValue();
        aCountThread->terminate();
        ThreadHelper::thread_sleep(2);
        sal_Int32 nLaterValue = aCountThread->getValue();

        // isRunning should be false after terminate
        sal_Bool isRunning = aCountThread->isRunning();
        aCountThread->join();
        delete aCountThread;

        CPPUNIT_ASSERT_MESSAGE(
            "Terminate the thread",
            isRunning == sal_False && nValue == nLaterValue
        );
    }
    /** Check if a suspended thread will terminate after call terminate, different on w32 and on UNX
    */
    void terminate_002()
    {
        OCountThread* aCountThread = new OCountThread();
        sal_Bool bRes = aCountThread->create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

        suspendCountThread(aCountThread);
        sal_Int32 nValue = aCountThread->getValue();

        // seems a suspended thread can not be terminated on W32, while on Solaris can
        resumeAndWaitThread(aCountThread);

        termAndJoinThread(aCountThread);
        sal_Int32 nLaterValue = aCountThread->getValue();
        delete aCountThread;

        CPPUNIT_ASSERT_MESSAGE(
            "Suspend then resume the thread",
            nLaterValue == nValue );
        }

    CPPUNIT_TEST_SUITE(terminate);
    CPPUNIT_TEST(terminate_001);
    CPPUNIT_TEST(terminate_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class terminate

    /** Test of the osl::Thread::join method
    */
    class join : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    /** Check after call terminate if the thread running function will not go on executing

        the next statement after join will not exec before the thread terminate
        ALGORITHM:
        recode system time at the beginning of the thread run, call join, then record system time again,
        the difference of the two time should be equal or more than 20 seconds, the CountThead normally terminate
    */
    void join_001()
    {
        OCountThread *aCountThread = new OCountThread();
        sal_Bool bRes = aCountThread->create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

        TimeValue aTimeVal_befor;
        osl_getSystemTime( &aTimeVal_befor );
        //printf("#join:the system time is %d,%d\n", pTimeVal_befor->Seconds,pTimeVal_befor->Nanosec);

        aCountThread->join();

        //the below line will be executed after aCountThread terminate
        TimeValue aTimeVal_after;
        osl_getSystemTime( &aTimeVal_after );
        sal_uInt32 nSec = aTimeVal_after.Seconds - aTimeVal_befor.Seconds;

        delete aCountThread;

        CPPUNIT_ASSERT_MESSAGE(
            "Join the thread: after the thread terminate",
            nSec >= 20
        );

    }
    /** after terminated by another thread, join exited immediately

        ALGORITHM:
        terminate the thread when value>=3, call join, check the beginning time and time after join,
        the difference should be 3 seconds, join costs little time
    */
    void join_002()
    {
        OCountThread *aCountThread = new OCountThread();
        sal_Bool bRes = aCountThread->create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

        //record the time when the running begin
        TimeValue aTimeVal_befor;
        osl_getSystemTime( &aTimeVal_befor );

        ThreadHelper::thread_sleep(3);
        termAndJoinThread(aCountThread);

        //the below line will be executed after aCountThread terminate
        TimeValue aTimeVal_after;
        osl_getSystemTime( &aTimeVal_after );
        sal_uInt32 nSec = aTimeVal_after.Seconds - aTimeVal_befor.Seconds;
        delete aCountThread;
        CPPUNIT_ASSERT_MESSAGE(
            "Join the thread: after thread terminate by another thread",
            nSec == 3
        );
    }

    CPPUNIT_TEST_SUITE(join);
    CPPUNIT_TEST(join_001);
    CPPUNIT_TEST(join_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class join

    /** Test of the osl::Thread::isRunning method
    */
    class isRunnung : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    /**
    */
    void isRunning_001()
    {
        OCountThread *aCountThread = new OCountThread();
        sal_Bool bRes = aCountThread->create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

        sal_Bool bRun = aCountThread->isRunning();

        ThreadHelper::thread_sleep(2);
        termAndJoinThread(aCountThread);
        sal_Bool bTer = aCountThread->isRunning();
        delete aCountThread;

        CPPUNIT_ASSERT_MESSAGE(
            "Test isRunning",
            bRun == sal_True && bTer == sal_False
        );
    }
    /** check the value of isRunning when suspending and after resume
    */
    void isRunning_002()
    {
        OCountThread *aCountThread = new OCountThread();
        sal_Bool bRes = aCountThread->create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

        sal_Bool bRunning = aCountThread->isRunning();
        sal_Int32 nValue = 0;
        suspendCountThread(aCountThread);

        sal_Bool bRunning_sup = aCountThread->isRunning();
        ThreadHelper::thread_sleep(2);
        aCountThread->resume();
        ThreadHelper::thread_sleep(2);
        sal_Bool bRunning_res = aCountThread->isRunning();
        termAndJoinThread(aCountThread);
        sal_Bool bRunning_ter = aCountThread->isRunning();
        delete aCountThread;

        CPPUNIT_ASSERT_MESSAGE(
            "Test isRunning",
            bRes == sal_True && bRunning_sup == sal_True &&
            bRunning_res == sal_True && bRunning_ter == sal_False
        );

    }

    CPPUNIT_TEST_SUITE(isRunnung);
    CPPUNIT_TEST(isRunning_001);
    CPPUNIT_TEST(isRunning_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class isRunnung


    /// check osl::Thread::setPriority
    class setPriority : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
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

        ThreadHelper::thread_sleep(10);

        pThread->terminate();
        p2Thread->terminate();

        sal_Int32 nValueNormal = 0;
        nValueNormal = pThread->getValue();

        sal_Int32 nValueNormal2 = 0;
        nValueNormal2 = p2Thread->getValue();

        rtl::OString sPrio = getPrioName(_aPriority);
        printf("# After 10 seconds\n");

        printf("# nValue in %s Prio Thread is  %d\n",sPrio.getStr(), nValueNormal);
        printf("# nValue in %s Prio Thread is %d\n", sPrio.getStr(), nValueNormal2);

        ThreadHelper::thread_sleep(1);
        delete pThread;
        delete p2Thread;

        sal_Int32 nDelta = abs(nValueNormal - nValueNormal2);
        double nQuotient = std::max(nValueNormal, nValueNormal2);
        CPPUNIT_ASSERT_MESSAGE(
            "Quotient is zero, which means, there exist no right values.",
            nQuotient != 0
            );
        double nDeltaPercent = nDelta / nQuotient * 100;

        printf("# Delta value %d, percent %f\n",nDelta, nDeltaPercent);

        CPPUNIT_ASSERT_MESSAGE(
            "Run 2 normal threads, the count diff more than 5 percent.",
            nDeltaPercent <= 5
            );
        }

    void setPriority_001_1()
        {
        check2Threads(osl_Thread_PriorityHighest);
        }
    void setPriority_001_2()
        {
        check2Threads(osl_Thread_PriorityAboveNormal);
        }
    void setPriority_001_3()
        {
        check2Threads(osl_Thread_PriorityNormal);
        }
    void setPriority_001_4()
        {
        check2Threads(osl_Thread_PriorityBelowNormal);
        }
    void setPriority_001_5()
        {
        check2Threads(osl_Thread_PriorityLowest);
        }

    void setPriority_002()
        {
        // initial 5 threads with different priorities
        OAddThread aHighestThread;
        OAddThread aAboveNormalThread;
        OAddThread aNormalThread;
        //OAddThread *aBelowNormalThread = new OAddThread();
        //OAddThread *aLowestThread = new OAddThread();

        //Create them and start running at the same time
        aHighestThread.create();
        aHighestThread.setPriority(osl_Thread_PriorityHighest);

        aAboveNormalThread.create();
        aAboveNormalThread.setPriority(osl_Thread_PriorityAboveNormal);

        aNormalThread.create();
        aNormalThread.setPriority(osl_Thread_PriorityNormal);
        /*aBelowNormalThread->create();
          aBelowNormalThread->setPriority(osl_Thread_PriorityBelowNormal);
          aLowestThread->create();
          aLowestThread->setPriority(osl_Thread_PriorityLowest);
        */
        ThreadHelper::thread_sleep(10);
                termAndJoinThread(&aHighestThread);
                termAndJoinThread(&aAboveNormalThread);
                termAndJoinThread(&aNormalThread);
        //aBelowNormalThread->terminate();
        //aLowestThread->terminate();

        sal_Int32 nValueHighest = 0;
        nValueHighest = aHighestThread.getValue();

        sal_Int32 nValueAboveNormal = 0;
        nValueAboveNormal = aAboveNormalThread.getValue();

        sal_Int32 nValueNormal = 0;
        nValueNormal = aNormalThread.getValue();

        sal_Int32 nValueBelowNormal = 0;
        //nValueBelowNormal = aBelowNormalThread->getValue();
        sal_Int32 nValueLowest = 0;
        //nValueLowest = aLowestThread->getValue();
        printf("# After 10 seconds\n");
        printf("# nValue in Highest Prio Thread is %d\n",nValueHighest);
        printf("# nValue in AboveNormal Prio Thread is %d\n",nValueAboveNormal);
        printf("# nValue in Normal Prio Thread is %d\n",nValueNormal);

        CPPUNIT_ASSERT_MESSAGE(
            "SetPriority",
            nValueHighest >= nValueAboveNormal &&
            nValueAboveNormal >= nValueNormal
            );
        }

    void setPriority_003()
        {
                #ifndef SOLARIS
        // initial 5 threads with different priorities
        OAddThread *aHighestThread = new OAddThread();
        OAddThread *aAboveNormalThread = new OAddThread();
        OAddThread *aNormalThread = new OAddThread();
        OAddThread *aBelowNormalThread = new OAddThread();
        OAddThread *aLowestThread = new OAddThread();

        //Create them and start running at the same time
        aHighestThread->create();
        aHighestThread->setPriority(osl_Thread_PriorityHighest);

        aAboveNormalThread->create();
        aAboveNormalThread->setPriority(osl_Thread_PriorityAboveNormal);

        aNormalThread->create();
        aNormalThread->setPriority(osl_Thread_PriorityNormal);
        aBelowNormalThread->create();
        aBelowNormalThread->setPriority(osl_Thread_PriorityBelowNormal);
        aLowestThread->create();
        aLowestThread->setPriority(osl_Thread_PriorityLowest);

        ThreadHelper::thread_sleep(10);
        termAndJoinThread(aHighestThread);
        termAndJoinThread(aAboveNormalThread);
        termAndJoinThread(aNormalThread);
        termAndJoinThread(aBelowNormalThread);
        termAndJoinThread(aLowestThread);

        sal_Int32 nValueHighest = 0;
        nValueHighest = aHighestThread->getValue();

        sal_Int32 nValueAboveNormal = 0;
        nValueAboveNormal = aAboveNormalThread->getValue();

        sal_Int32 nValueNormal = 0;
        nValueNormal = aNormalThread->getValue();

        sal_Int32 nValueBelowNormal = 0;
        nValueBelowNormal = aBelowNormalThread->getValue();

        sal_Int32 nValueLowest = 0;
        nValueLowest = aLowestThread->getValue();

        printf("# After 10 seconds\n");
        printf("# nValue in Highest Prio Thread is     %d\n",nValueHighest);
        printf("# nValue in AboveNormal Prio Thread is %d\n",nValueAboveNormal);
        printf("# nValue in Normal Prio Thread is      %d\n",nValueNormal);
        printf("# nValue in BelowNormal Prio Thread is %d\n",nValueBelowNormal);
        printf("# nValue in Lowest Prio Thread is      %d\n",nValueLowest);

        delete aHighestThread;
        delete aAboveNormalThread;
        delete aNormalThread;
        delete aBelowNormalThread;
        delete aLowestThread;

        CPPUNIT_ASSERT_MESSAGE(
            "SetPriority",
            nValueHighest > nValueAboveNormal &&
            nValueAboveNormal > nValueNormal &&
            nValueNormal > nValueBelowNormal &&
            nValueBelowNormal > nValueLowest
            );
                #endif

        }


    CPPUNIT_TEST_SUITE(setPriority);
    CPPUNIT_TEST(setPriority_001_1);
    CPPUNIT_TEST(setPriority_001_2);
    CPPUNIT_TEST(setPriority_001_3);
    CPPUNIT_TEST(setPriority_001_4);
    CPPUNIT_TEST(setPriority_001_5);
    CPPUNIT_TEST(setPriority_002);
    CPPUNIT_TEST(setPriority_003);
    CPPUNIT_TEST_SUITE_END();
    }; // class setPriority

    /** Test of the osl::Thread::getPriority method
    */
    class getPriority : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }

    // insert your test code here.
    void getPriority_001()
        {
        OAddThread *aHighestThread = new OAddThread();

        //Create them and start running at the same time
        aHighestThread->create();
        aHighestThread->setPriority(osl_Thread_PriorityHighest);

        oslThreadPriority aPriority = aHighestThread->getPriority();
        termAndJoinThread(aHighestThread);
        delete aHighestThread;

        CPPUNIT_ASSERT_MESSAGE(
            "getPriority",
            aPriority == osl_Thread_PriorityHighest
            );

        }

    void getPriority_002()
        {

        }

    CPPUNIT_TEST_SUITE(getPriority);
    CPPUNIT_TEST(getPriority_001);
    CPPUNIT_TEST(getPriority_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class getPriority


    class getIdentifier : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }

    // insert your test code here.
    void getIdentifier_001()
        {

        }

    void getIdentifier_002()
        {

        }

    CPPUNIT_TEST_SUITE(getIdentifier);
    CPPUNIT_TEST(getIdentifier_001);
    CPPUNIT_TEST(getIdentifier_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class getIdentifier

    /** Test of the osl::Thread::getCurrentIdentifier method
    */
    class getCurrentIdentifier : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }

    // insert your test code here.
    void getCurrentIdentifier_001()
        {
        oslThreadIdentifier oId;
        OCountThread* pCountThread = new OCountThread;
        //OCountThread* pCountThread2 = new OCountThread;
        pCountThread->create();
        //pCountThread2->create();
        pCountThread->setWait(3);
        oId = pCountThread->getCurrentIdentifier();
        oslThreadIdentifier oIdChild = pCountThread->getIdentifier();
        //printf("# CurrentId is %ld, Child1 id is %ld, Child2 id is %ld\n",oId, oIdChild,pCountThread2->m_id );
        termAndJoinThread(pCountThread);
        delete pCountThread;
        //termAndJoinThread(pCountThread2);
        //delete pCountThread2;

        CPPUNIT_ASSERT_MESSAGE(
            "Get the identifier for the current active thread.",
            oId != oIdChild
            );

        }

    void getCurrentIdentifier_002()
        {
        }

    CPPUNIT_TEST_SUITE(getCurrentIdentifier);
    CPPUNIT_TEST(getCurrentIdentifier_001);
    CPPUNIT_TEST(getCurrentIdentifier_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class getCurrentIdentifier

    /** Test of the osl::Thread::wait method
    */
    class wait : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }

    /** call wait in the run method

        ALGORITHM:
        tested thread wait nWaitSec seconds, main thread sleep (nWaitSec + 2) seconds,
        then terminate the tested thread, and the time cost of the tested thread minus
        current value should equals to nWaitSec.
    */
    void wait_001()
    {
            OCountThread *aCountThread = new OCountThread();
        sal_Bool bRes = aCountThread->create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

            //record the time when the running begin
            TimeValue aTimeVal_befor;
            osl_getSystemTime( &aTimeVal_befor );

            sal_Int32 nValue = 0;
            sal_Int32 nWaitSec = 5;
            while (1)
            {
                nValue = aCountThread->getValue();
                if (nValue >= 2)
                {
                    //wait
                    aCountThread->setWait(nWaitSec);
                    break;
                }
            }
            ThreadHelper::thread_sleep(nWaitSec + 3);
            termAndJoinThread(aCountThread);

            nValue = aCountThread->getValue();
            TimeValue aTimeVal_after;
            osl_getSystemTime( &aTimeVal_after );

            sal_uInt32 nSec = aTimeVal_after.Seconds - aTimeVal_befor.Seconds;
            delete aCountThread;
            printf("nSec = %d \n", nSec);
            printf("nValue = %d \n", nValue);

            CPPUNIT_ASSERT_MESSAGE(
                "Wait: Blocks the calling thread for the given number of time.",
                nSec - nValue >= nWaitSec
        );

        }
    /** wait then terminate the thread

        ALGORITHM:
        wait nWaitSec seconds, and terminate when the wait does not finish
        Windows & UNX: thread terminates immediatlly
    */
    void wait_002()
    {
        OCountThread aThread;
        sal_Bool bRes = aThread.create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

        TimeValue aTimeVal_befor;
        osl_getSystemTime( &aTimeVal_befor );

        sal_Int32 nWaitSec = 3;
        aThread.setWait(nWaitSec);

        termAndJoinThread(&aThread);
        sal_Int32 nValue = aThread.getValue();

        TimeValue aTimeVal_after;
        osl_getSystemTime( &aTimeVal_after );

        sal_Int32 nSec = aTimeVal_after.Seconds - aTimeVal_befor.Seconds;
        CPPUNIT_ASSERT_MESSAGE(
            "Wait: Blocks the calling thread for the given number of time.",
            nSec == 0 && nValue == 0
        );
    }

    CPPUNIT_TEST_SUITE(wait);
    CPPUNIT_TEST(wait_001);
    CPPUNIT_TEST(wait_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class wait

    /** osl::Thread::yield method: can not design good test scenario to test up to now
    */
    class yield : public CppUnit::TestFixture
    {
    public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

    // insert your test code here.
    void yield_001()
    {
    }


    CPPUNIT_TEST_SUITE(yield);
    CPPUNIT_TEST(yield_001);
    CPPUNIT_TEST_SUITE_END();
    }; // class yield

    /** Test of the osl::Thread::schedule method
    */
    class schedule : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    /** The requested thread will get terminate the next time schedule() is called.

        Note: on UNX, if call suspend thread is not the to be suspended thread, the to be suspended thread will get suspended the next time schedule() is called,
        while on w32, it's nothing with schedule.

        check if suspend and terminate work well via schedule
    */
    void schedule_001()
    {
        OAddThread* aThread = new OAddThread();
        sal_Bool bRes = aThread->create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

        ThreadHelper::thread_sleep(2);
        aThread->suspend();
        sal_Int32 nValue = aThread->getValue();
        ThreadHelper::thread_sleep(3);
        sal_Int32 nLaterValue = aThread->getValue();
        resumeAndWaitThread(aThread);

        CPPUNIT_ASSERT_MESSAGE(
            "Schedule: suspend works.",
            nLaterValue == nValue
        );

        aThread->terminate();
        sal_Int32 nValue_term = aThread->getValue();

        aThread->join();
        sal_Int32 nValue_join = aThread->getValue();
        delete aThread;
        //check if thread really terminate after call terminate, if join immediatlly return
        CPPUNIT_ASSERT_MESSAGE(
            "Schedule: Returns False if the thread should terminate.",
            nValue_join -  nValue_term <= 1 && nValue_join -  nValue_term >= 0
        );

    }

    /** design a thread that has not call schedule in the workfunction--run method
    */
    void schedule_002()
    {
        ONoScheduleThread aThread;
        sal_Bool bRes = aThread.create();
        CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

        ThreadHelper::thread_sleep(2);
        aThread.suspend();
        sal_Int32 nValue = aThread.getValue();

        ThreadHelper::thread_sleep(3);
        sal_Int32 nLaterValue = aThread.getValue();
        resumeAndWaitThread(&aThread);
        //On windows, suspend works, so the values are same
        #ifdef WNT
        CPPUNIT_ASSERT_MESSAGE(
            "Schedule: don't schedule in thread run method, suspend works.",
            nLaterValue -  nValue == 0
        );
        #endif

        //On UNX, suspend does not work, so the difference of the values equals to sleep seconds number
        #ifdef UNX
        CPPUNIT_ASSERT_MESSAGE(
            "Schedule: don't schedule in thread run method, suspend does not work too.",
            nLaterValue -  nValue == 3
        );
        #endif

        // terminate will not work if no schedule in thread's work function
        termAndJoinThread(&aThread);
        sal_Int32 nValue_term = aThread.getValue();

        CPPUNIT_ASSERT_MESSAGE(
            "Schedule: don't schedule in thread run method, terminate failed.",
            nValue_term == 10
        );
    }

    CPPUNIT_TEST_SUITE(schedule);
    CPPUNIT_TEST(schedule_001);
    CPPUNIT_TEST(schedule_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class schedule

// -----------------------------------------------------------------------------
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::create, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::createSuspended, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::suspend, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::resume, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::terminate, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::join, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::isRunnung, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::setPriority, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::getPriority, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::getIdentifier, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::getCurrentIdentifier, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::wait, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::yield, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::schedule, "osl_Thread");
} // namespace osl_Thread


// -----------------------------------------------------------------------------
// destroy function when the binding thread terminate
void SAL_CALL destroyCallback(void * data)
{
   printf("# destroying local data %s\n", (char *) data);
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
        strcpy(pc, &m_nData);
        myThreadData.setData(pc);
        char* pData = (char*)myThreadData.getData();
        m_Char_Test = *pData;
        // wait for long time to check the data value in main thread
        ThreadHelper::thread_sleep(3);
    }
public:
     ~myKeyThread()
    {
        if (isRunning())
        {
        printf("# error: not terminated.\n");
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
            printf("# error: not terminated.\n");
        }
    }
};

namespace osl_ThreadData
{

    class ctors : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }

    // insert your test code here.
    void ctor_001()
        {

        }

    CPPUNIT_TEST_SUITE(ctors);
    CPPUNIT_TEST(ctor_001);
    CPPUNIT_TEST_SUITE_END();
    }; // class ctors


    class setData : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }

    /** the same instance of the class can have different values in different threads
    */
    void setData_001()
        {
            idThread aThread1;
            aThread1.create();
            idThread aThread2;
            aThread2.create();

            aThread1.join();
            aThread2.join();

            oslThreadIdentifier aThreadId1 = aThread1.getIdentifier();
            oslThreadIdentifier aThreadId2 = aThread2.getIdentifier();

            CPPUNIT_ASSERT_MESSAGE(
                "ThreadData setData: ",
                aThread1.m_Id == aThreadId1 && aThread2.m_Id == aThreadId2
                );

        }

    void setData_002()
        {
            // at first, set the data a value
            char* pc = new char[2];
            char m_nData = 'm';
            strcpy(pc, &m_nData);
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

            CPPUNIT_ASSERT_MESSAGE(
                "ThreadData setData: ",
                cData1 == 'a' && cData2 == 'b' && aChar == 'm'
                );

        }
    /** setData the second time, and then getData
    */
    void setData_003()
        {
            // at first, set the data a value
            char* pc = new char[2];
            char m_nData = 'm';
            strcpy(pc, &m_nData);
            myThreadData.setData(pc);

            myKeyThread aThread1('a');
            aThread1.create();
            myKeyThread aThread2('b');
            aThread2.create();
            // aThread1 and aThread2 should have not terminated yet
            // setData the second time
            char* pc2 = new char[2];
            m_nData = 'o';
            strcpy(pc2, &m_nData);
            myThreadData.setData(pc2);
            char* pChar = (char*)myThreadData.getData();
            char aChar = *pChar;

            aThread1.join();
            aThread2.join();

            // the saved thread data of aThread1 & aThread2, different
            char cData1 = aThread1.m_Char_Test;
            char cData2 = aThread2.m_Char_Test;

            CPPUNIT_ASSERT_MESSAGE(
                "ThreadData setData: ",
                cData1 == 'a' && cData2 == 'b' && aChar == 'o'
                );

        }

    CPPUNIT_TEST_SUITE(setData);
    CPPUNIT_TEST(setData_001);
    CPPUNIT_TEST(setData_002);
    CPPUNIT_TEST(setData_003);
    CPPUNIT_TEST_SUITE_END();
    }; // class setData

    //sal_Bool buildTwoThreads(char)

    class getData : public CppUnit::TestFixture
    {
    public:
    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }

    // After setData in child threads, get Data in the main thread, should be independent
    void getData_001()
        {
            char* pc = new char[2];
            char m_nData[] = "i";
            strcpy(pc, m_nData);
            printf("# pc %s\n", pc);
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

            CPPUNIT_ASSERT_MESSAGE(
                "ThreadData setData: ",
                cData1 == 'c' && cData2 == 'd' && aChar == 'i'
                );


        }

    // setData then change the value in the address data pointer points,
    // and then getData, should get the new value
    void getData_002()
        {
            char* pc = new char[2];
            char m_nData = 'i';
            strcpy(pc, &m_nData);
            printf("# pc %s\n", pc);
            myThreadData.setData(pc);

            myKeyThread aThread1('a');
            aThread1.create();
            myKeyThread aThread2('b');
            aThread2.create();

            // change the value which pc points
            char m_nData2 = 'j';
            strcpy(pc, &m_nData2);
            //printf("# pc %s\n", pc);
            void* pChar = myThreadData.getData();
            char aChar = *(char*)pChar;

            aThread1.join();
            aThread2.join();

            char cData1 = aThread1.m_Char_Test;
            char cData2 = aThread2.m_Char_Test;

            CPPUNIT_ASSERT_MESSAGE(
                "ThreadData setData: ",
                cData1 == 'a' && cData2 == 'b' && aChar == 'j'
                );

        }

    CPPUNIT_TEST_SUITE(getData);
    CPPUNIT_TEST(getData_001);
    CPPUNIT_TEST(getData_002);
    CPPUNIT_TEST_SUITE_END();
    }; // class getData

// -----------------------------------------------------------------------------
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_ThreadData::ctors, "osl_ThreadData");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_ThreadData::setData, "osl_ThreadData");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_ThreadData::getData, "osl_ThreadData");
} // namespace osl_ThreadData

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;

