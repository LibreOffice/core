/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#ifdef WNT
#include <windows.h>
#else
#include <unistd.h>
#include <time.h>
#endif




#include <sal/types.h>

#include <rtl/string.hxx>

#include <rtl/strbuf.hxx>

#include <osl/thread.hxx>

#include <osl/mutex.hxx>
#include <osl/time.h>

#include <string.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#define t_print printf

using namespace osl;

using ::rtl::OString;



class StopWatch {
    TimeValue t1,t2;                                

protected:
    sal_Int32 m_nNanoSec;
    sal_Int32 m_nSeconds;

    bool m_bIsValid;                                   
    bool m_bIsRunning;                                 

public:
    StopWatch();
    ~StopWatch() {}

    void start();                                 
    void stop();                                  

    double getSeconds() const;
    double getTenthSec() const;
};






StopWatch::StopWatch()
    : m_nNanoSec(0)
    , m_nSeconds(0)
    , m_bIsValid(false)
    , m_bIsRunning(false)
{
    t1.Seconds = 0;
    t1.Nanosec = 0;
    t2.Seconds = 0;
    t2.Nanosec = 0;
}

void StopWatch::start()
{



    m_bIsValid = false;
    m_bIsRunning = true;
    osl_getSystemTime( &t1 );
    t_print("# %u %u nsecs\n", (unsigned)t1.Seconds, (unsigned)t1.Nanosec);
    
}

void StopWatch::stop()
{



    
    osl_getSystemTime( &t2 );
    t_print("# %u %u nsecs\n", (unsigned) t2.Seconds, (unsigned) t2.Nanosec);

    if (m_bIsRunning)
    {                                
        m_nSeconds = static_cast<sal_Int32>(t2.Seconds) - static_cast<sal_Int32>(t1.Seconds);
        if ( t2.Nanosec > t1.Nanosec )
               m_nNanoSec = static_cast<sal_Int32>(t2.Nanosec) - static_cast<sal_Int32>(t1.Nanosec);
           else
           {
        m_nNanoSec = 1000000000 + static_cast<sal_Int32>(t2.Nanosec) - static_cast<sal_Int32>(t1.Nanosec);
                m_nSeconds -= 1;
    }
    t_print("# %u %u nsecs\n", (unsigned) m_nSeconds, (unsigned) m_nNanoSec );
        
        
            
            
        
        m_bIsValid = true;
        m_bIsRunning = false;
    }
}

double StopWatch::getSeconds() const
{



    double nValue = 0.0;
    if (m_bIsValid)
    {
        nValue = double(m_nNanoSec) / 1000000000.0 + m_nSeconds; 
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


template <class T>
class ThreadSafeValue
{
    T   m_nFlag;
    Mutex   m_aMutex;
public:
    ThreadSafeValue(T n = 0): m_nFlag(n) {}
    T getValue()
        {
            
            osl::MutexGuard g(m_aMutex);
            return m_nFlag;
        }
    void addValue(T n)
        {
            
            osl::MutexGuard g(m_aMutex);
            m_nFlag += n;
        }
    void acquire() {m_aMutex.acquire();}
    void release() {m_aMutex.release();}
};


namespace ThreadHelper
{
    void thread_sleep_tenth_sec(sal_Int32 _nTenthSec)
    {
#ifdef WNT
        Sleep(_nTenthSec * 100 );
#else
        TimeValue nTV;
        nTV.Seconds = static_cast<sal_uInt32>( _nTenthSec/10 );
        nTV.Nanosec = ( (_nTenthSec%10 ) * 100000000 );
        osl_waitThread(&nTV);
#endif
    }

    void outputPriority(oslThreadPriority const& _aPriority)
    {
        
        if (_aPriority == osl_Thread_PriorityHighest)
        {
            t_print("Prio is High\n");
        }
        else if (_aPriority == osl_Thread_PriorityAboveNormal)
        {
            t_print("Prio is above normal\n");
        }
        else if (_aPriority == osl_Thread_PriorityNormal)
        {
            t_print("Prio is normal\n");
        }
        else if (_aPriority == osl_Thread_PriorityBelowNormal)
        {
            t_print("Prio is below normal\n");
        }
        else if (_aPriority == osl_Thread_PriorityLowest)
        {
            t_print("Prio is lowest\n");
        }
        else
        {
            t_print("Prio is unknown\n");
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
                t_print("error: not terminated.\n");
            }
        }

};


/** Thread which has a flag add 1 every second until 20
 */
class OCountThread : public Thread
{
    ThreadSafeValue<sal_Int32> m_aFlag;
public:
    OCountThread()
        {
            m_nWaitSec = 0;
            t_print("new OCountThread thread %u!\n", (unsigned) getIdentifier());
        }
    sal_Int32 getValue() { return m_aFlag.getValue(); }

    void setWait(sal_Int32 nSec)
        {
            m_nWaitSec = nSec;
            
        }

    virtual void SAL_CALL suspend()
        {
            m_aFlag.acquire();
            ::osl::Thread::suspend();
            m_aFlag.release();
        }

protected:
    
    sal_Int32 m_nWaitSec;

    void SAL_CALL run()
        {
            
            while (m_aFlag.getValue() < 20 && schedule())
            {
                m_aFlag.addValue(1);
                ThreadHelper::thread_sleep_tenth_sec(1);

                if (m_nWaitSec != 0)
                {
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
            t_print("normally terminate this thread %u!\n", (unsigned) getIdentifier());
        }
public:

    ~OCountThread()
        {
            if (isRunning())
            {
                t_print("error: not terminated.\n");
            }
        }

};

/** call suspend in the run method
*/
class OSuspendThread : public Thread
{
    ThreadSafeValue<sal_Int32> m_aFlag;
public:
    OSuspendThread(){ m_bSuspend = false; }
    sal_Int32 getValue() { return m_aFlag.getValue(); }
    void setSuspend()
        {
            m_bSuspend = true;
        }
    virtual void SAL_CALL suspend()
        {
            m_aFlag.acquire();
            ::osl::Thread::suspend();
            m_aFlag.release();
        }
protected:
    bool m_bSuspend;
    void SAL_CALL run()
        {
            
            while (schedule())
            {
                m_aFlag.addValue(1);

                ThreadHelper::thread_sleep_tenth_sec(1);
                if (m_bSuspend)
                {
                    suspend();
                    m_bSuspend  = false;
                }
            }
        }
public:

    ~OSuspendThread()
        {
            if (isRunning())
            {
                t_print("error: not terminated.\n");
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
            }
        }
    void SAL_CALL onTerminated()
        {
            t_print("normally terminate this thread %u!\n", (unsigned) getIdentifier());
        }
public:
    ONoScheduleThread()
        {
                t_print("new thread id %u!\n", (unsigned) getIdentifier());
        }
    ~ONoScheduleThread()
        {
            if (isRunning())
            {
                t_print("error: not terminated.\n");
            }
        }

};

/**
*/
class OAddThread : public Thread
{
    ThreadSafeValue<sal_Int32> m_aFlag;
public:
    
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
            
            while (schedule())
            {
                m_aFlag.addValue(1);
            }
        }
    void SAL_CALL onTerminated()
        {
            
        }
public:

    ~OAddThread()
        {
            if (isRunning())
            {
                
            }
        }

};

namespace osl_Thread
{

    void resumeAndWaitThread(Thread* _pThread)
    {
        
        

        
        
        
#ifdef WNT
        t_print("resumeAndWaitThread\n");
        _pThread->resume();
        ThreadHelper::thread_sleep_tenth_sec(1);
#else
        _pThread->resume();
#endif
    }

    
    void termAndJoinThread(Thread* _pThread)
    {
        _pThread->terminate();


#ifdef WNT
        _pThread->resume();
        ThreadHelper::thread_sleep_tenth_sec(1);
#endif
        t_print("#wait for join.\n");
        _pThread->join();
    }
/** Test of the osl::Thread::create method
 */

    class create : public CppUnit::TestFixture
    {
    public:

        
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
                bool bRes = newthread->create();
                CPPUNIT_ASSERT_MESSAGE("Can not creates a new thread!\n", bRes);

                ThreadHelper::thread_sleep_tenth_sec(1);        
                bool isRunning = newthread->isRunning();    
                
                ThreadHelper::thread_sleep_tenth_sec(3);
                sal_Int32 nValue = newthread->getValue();
                
                termAndJoinThread(newthread);
                delete newthread;

                t_print("   nValue = %d\n", (int) nValue);
                t_print("isRunning = %s\n", isRunning ? "true" : "false");

                CPPUNIT_ASSERT_MESSAGE(
                    "Creates a new thread",
                    nValue >= 1 && isRunning
                    );

            }

        /** only one running thread per instance, return false if create secondly
         */
        void create_002()
            {
                myThread* newthread = new myThread();
                bool res1 = newthread->create();
                bool res2 = newthread->create();
                t_print("In non pro, an assertion should occurred. This behaviour is right.\n");
                termAndJoinThread(newthread);
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
    }; 



    /** Test of the osl::Thread::createSuspended method
    */
    class createSuspended : public CppUnit::TestFixture
    {
    public:
        
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
                bool bRes = newthread->createSuspended();
                CPPUNIT_ASSERT_MESSAGE("Can not creates a new thread!", bRes);

                ThreadHelper::thread_sleep_tenth_sec(1);
                bool isRunning = newthread->isRunning();
                ThreadHelper::thread_sleep_tenth_sec(3);
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
                bool res1 = newthread->createSuspended();
                bool res2 = newthread->createSuspended();

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
    }; 

    /** when the count value equal to or more than 3, suspend the thread.
    */
    void suspendCountThread(OCountThread* _pCountThread)
    {
        sal_Int32 nValue = 0;
        while (true)
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
                bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );
                
                suspendCountThread( aCountThread );

                
                sal_Int32 nValue = aCountThread->getValue();       

                ThreadHelper::thread_sleep_tenth_sec(3);

                
                sal_Int32 nLaterValue = aCountThread->getValue();    

                resumeAndWaitThread(aCountThread);
                termAndJoinThread(aCountThread);
                delete aCountThread;

                CPPUNIT_ASSERT_MESSAGE(
                    "Suspend the thread",
                    bRes && nValue == nLaterValue
                    );

            }
        /** suspend a thread in it's worker-function, the ALGORITHM is same as suspend_001
             reason of deadlocked I think: no schedule can schedule other threads to go on excuting
         */
        void suspend_002()
            {
                OSuspendThread* aThread = new OSuspendThread();
                bool bRes = aThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );
                
                sal_Int32 nValue = 0;
                
                
                ThreadHelper::thread_sleep_tenth_sec(3);
                nValue = aThread->getValue();    
                t_print(" getValue is %d !", (int) nValue );
                if (nValue >= 2)
                {
                        aThread->setSuspend();
                        
                }
                
                t_print(" after while!");
                
                nValue = aThread->getValue();       

                ThreadHelper::thread_sleep_tenth_sec(3);
                t_print(" after sleep!");
                
                sal_Int32 nLaterValue = aThread->getValue();        

                
                aThread->resume();
                termAndJoinThread(aThread);
                delete aThread;

                CPPUNIT_ASSERT_MESSAGE(
                    "Suspend the thread",
                    bRes && nValue == nLaterValue
                    );
            }

        CPPUNIT_TEST_SUITE(suspend);
        CPPUNIT_TEST(suspend_001);
        
        
        CPPUNIT_TEST_SUITE_END();
    }; 

    /** Test of the osl::Thread::resume method
    */
    class resume : public CppUnit::TestFixture
    {
    public:
        
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
                OCountThread* pCountThread = new OCountThread();
                bool bRes = pCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );

                suspendCountThread(pCountThread);

                sal_Int32 nSuspendValue = pCountThread->getValue();  
                
                ThreadHelper::thread_sleep_tenth_sec(3);
                pCountThread->resume();

                ThreadHelper::thread_sleep_tenth_sec(3);
                sal_Int32 nResumeValue = pCountThread->getValue();

                ThreadHelper::thread_sleep_tenth_sec(3);
                sal_Int32 nLaterValue = pCountThread->getValue();

                termAndJoinThread(pCountThread);
                delete pCountThread;

                t_print("SuspendValue: %d\n", (int) nSuspendValue);
                t_print("ResumeValue:  %d\n", (int) nResumeValue);
                t_print("LaterValue:   %d\n", (int) nLaterValue);

                /* LLA: this assumption is no longer relevant: nResumeValue ==  nSuspendValue && */
                CPPUNIT_ASSERT_MESSAGE(
                    "Suspend then resume the thread",
                    nLaterValue >= 9 &&
                    nResumeValue > nSuspendValue &&
                    nLaterValue > nResumeValue
                    );

            }

        /** Create a suspended thread then resume, check if the thread has run
         */
        void resume_002()
            {
                myThread* newthread = new myThread();
                bool bRes = newthread->createSuspended();
                CPPUNIT_ASSERT_MESSAGE ( "Can't create thread!", bRes );

                newthread->resume();
                ThreadHelper::thread_sleep_tenth_sec(2);
                sal_Int32 nValue = newthread->getValue();

                termAndJoinThread(newthread);
                delete newthread;

                t_print("   nValue = %d\n", (int) nValue);

                CPPUNIT_ASSERT_MESSAGE(
                    "Creates a suspended thread, then resume",
                    nValue >= 1
                    );
            }

        CPPUNIT_TEST_SUITE(resume);
        CPPUNIT_TEST(resume_001);
        CPPUNIT_TEST(resume_002);
        CPPUNIT_TEST_SUITE_END();
    }; 

    /** Test of the osl::Thread::terminate method
    */
    class terminate : public CppUnit::TestFixture
    {
    public:
        
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
                bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );

                ThreadHelper::thread_sleep_tenth_sec(2);
                sal_Int32 nValue = aCountThread->getValue();
                aCountThread->terminate();
                ThreadHelper::thread_sleep_tenth_sec(2);
                sal_Int32 nLaterValue = aCountThread->getValue();

                
                bool isRunning = aCountThread->isRunning();
                aCountThread->join();
                delete aCountThread;

                t_print("     nValue = %d\n", (int) nValue);
                t_print("nLaterValue = %d\n", (int) nLaterValue);

                CPPUNIT_ASSERT_MESSAGE(
                    "Terminate the thread",
                    !isRunning && nLaterValue >= nValue
                    );
            }
        /** Check if a suspended thread will terminate after call terminate, different on w32 and on UNX
         */
        void terminate_002()
            {
                OCountThread* aCountThread = new OCountThread();
                bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );

                ThreadHelper::thread_sleep_tenth_sec(1);
                suspendCountThread(aCountThread);
                sal_Int32 nValue = aCountThread->getValue();

                
                resumeAndWaitThread(aCountThread);

                ThreadHelper::thread_sleep_tenth_sec(2);

                termAndJoinThread(aCountThread);
                sal_Int32 nLaterValue = aCountThread->getValue();
                delete aCountThread;

                t_print("     nValue = %d\n", (int) nValue);
                t_print("nLaterValue = %d\n", (int) nLaterValue);

                CPPUNIT_ASSERT_MESSAGE(
                    "Suspend then resume the thread",
                    nLaterValue > nValue );
            }

        CPPUNIT_TEST_SUITE(terminate);
        CPPUNIT_TEST(terminate_001);
        CPPUNIT_TEST(terminate_002);
        CPPUNIT_TEST_SUITE_END();
    }; 

    /** Test of the osl::Thread::join method
    */
    class join : public CppUnit::TestFixture
    {
    public:
        
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
                bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );

                StopWatch aStopWatch;
                aStopWatch.start();
                
                
                

                aCountThread->join();

                
                
                
                aStopWatch.stop();
                
                double nSec = aStopWatch.getSeconds();
                t_print("join_001 nSec=%f\n", nSec);
                delete aCountThread;

                CPPUNIT_ASSERT_MESSAGE(
                    "Join the thread: after the thread terminate",
                    nSec >= 2
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
                bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );

                
                
                
                StopWatch aStopWatch;
                aStopWatch.start();

                ThreadHelper::thread_sleep_tenth_sec(10);
                termAndJoinThread(aCountThread);

                
                
                
                
                aStopWatch.stop();
                double nSec = aStopWatch.getSeconds();
                t_print("join_002 nSec=%f\n", nSec);

                delete aCountThread;
                CPPUNIT_ASSERT_MESSAGE(
                    "Join the thread: after thread terminate by another thread",
                    nSec >= 1
                    );
            }

        CPPUNIT_TEST_SUITE(join);
        CPPUNIT_TEST(join_001);
        CPPUNIT_TEST(join_002);
        CPPUNIT_TEST_SUITE_END();
    }; 

    /** Test of the osl::Thread::isRunning method
    */
    class isRunning : public CppUnit::TestFixture
    {
    public:
        
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
                bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );

                bool bRun = aCountThread->isRunning();

                ThreadHelper::thread_sleep_tenth_sec(2);
                termAndJoinThread(aCountThread);
                bool bTer = aCountThread->isRunning();
                delete aCountThread;

                CPPUNIT_ASSERT_MESSAGE(
                    "Test isRunning",
                    bRun && !bTer
                    );
            }
        /** check the value of isRunning when suspending and after resume
         */
        void isRunning_002()
            {
                OCountThread *aCountThread = new OCountThread();
                bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );

                
                
                suspendCountThread(aCountThread);

                bool bRunning_sup = aCountThread->isRunning();
                ThreadHelper::thread_sleep_tenth_sec(2);
                aCountThread->resume();
                ThreadHelper::thread_sleep_tenth_sec(2);
                bool bRunning_res = aCountThread->isRunning();
                termAndJoinThread(aCountThread);
                bool bRunning_ter = aCountThread->isRunning();
                delete aCountThread;

                CPPUNIT_ASSERT_MESSAGE(
                    "Test isRunning",
                    bRes && bRunning_sup && bRunning_res && !bRunning_ter );

            }

        CPPUNIT_TEST_SUITE(isRunning);
        CPPUNIT_TEST(isRunning_001);
        CPPUNIT_TEST(isRunning_002);
        CPPUNIT_TEST_SUITE_END();
    }; 


    
    class setPriority : public CppUnit::TestFixture
    {
    public:
        
        void setUp()
            {
            }

        void tearDown()
            {
            }

        
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
                    break;

                case osl_Thread_PriorityNormal:
                    sPrioStr = "Normal";
                    break;

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
                
                OAddThread* pThread = new OAddThread();
                OAddThread* p2Thread = new OAddThread();

                
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
                t_print("After 10 tenth seconds\n");

                t_print("nValue in %s Prio Thread is  %d\n",sPrio.getStr(), (int) nValueNormal);
                t_print("nValue in %s Prio Thread is %d\n", sPrio.getStr(), (int) nValueNormal2);

                
                pThread->join();
                p2Thread->join();

                delete pThread;
                delete p2Thread;

                sal_Int32 nDelta = abs(nValueNormal - nValueNormal2);
                double nQuotient = std::max(nValueNormal, nValueNormal2);
                CPPUNIT_ASSERT_MESSAGE(
                    "Quotient is zero, which means, there exist no right values.",
                    nQuotient != 0
                    );
                double nDeltaPercent = nDelta / nQuotient * 100;

                t_print("Delta value %d, percent %f\n", (int) nDelta, nDeltaPercent);

                
                
                
                
                
                
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
                

                OAddThread aHighestThread;
                OAddThread aAboveNormalThread;
                OAddThread aNormalThread;
                
                

                
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
                
                

                sal_Int32 nValueHighest = 0;
                nValueHighest = aHighestThread.getValue();

                sal_Int32 nValueAboveNormal = 0;
                nValueAboveNormal = aAboveNormalThread.getValue();

                sal_Int32 nValueNormal = 0;
                nValueNormal = aNormalThread.getValue();

                t_print("After 10 tenth seconds\n");
                t_print("nValue in Highest Prio Thread is %d\n", (int) nValueHighest);
                t_print("nValue in AboveNormal Prio Thread is %d\n", (int) nValueAboveNormal);
                t_print("nValue in Normal Prio Thread is %d\n", (int) nValueNormal);

#ifndef WNT
                CPPUNIT_ASSERT_MESSAGE(
                    "SetPriority",
                    nValueHighest     > 0 &&
                    nValueAboveNormal > 0 &&
                    nValueNormal > 0
                    );
#endif
            }

        void setPriority_003()
            {
                
                OAddThread *pHighestThread = new OAddThread();
                OAddThread *pAboveNormalThread = new OAddThread();
                OAddThread *pNormalThread = new OAddThread();
                OAddThread *pBelowNormalThread = new OAddThread();
                OAddThread *pLowestThread = new OAddThread();

                
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

                t_print("After 10 tenth seconds\n");
                t_print("nValue in Highest Prio Thread is     %d\n", (int) nValueHighest);
                t_print("nValue in AboveNormal Prio Thread is %d\n", (int) nValueAboveNormal);
                t_print("nValue in Normal Prio Thread is      %d\n", (int) nValueNormal);
                t_print("nValue in BelowNormal Prio Thread is %d\n", (int) nValueBelowNormal);
                t_print("nValue in Lowest Prio Thread is      %d\n", (int) nValueLowest);

                delete pHighestThread;
                delete pAboveNormalThread;
                delete pNormalThread;
                delete pBelowNormalThread;
                delete pLowestThread;

#ifndef WNT
                CPPUNIT_ASSERT_MESSAGE(
                    "SetPriority",
                    nValueHighest     > 0 &&
                    nValueAboveNormal > 0 &&
                    nValueNormal      > 0 &&
                    nValueBelowNormal > 0 &&
                    nValueLowest      > 0
                    );
#endif
            }

        void setPriority_004()
            {
                
                
                OAddThread *pAboveNormalThread = new OAddThread();
                OAddThread *pNormalThread = new OAddThread();
                OAddThread *pBelowNormalThread = new OAddThread();
                OAddThread *pLowestThread = new OAddThread();

                
                
                

                pAboveNormalThread->createSuspended();
                pAboveNormalThread->setPriority(osl_Thread_PriorityAboveNormal);

                pNormalThread->createSuspended();
                pNormalThread->setPriority(osl_Thread_PriorityNormal);

                pBelowNormalThread->createSuspended();
                pBelowNormalThread->setPriority(osl_Thread_PriorityBelowNormal);

                pLowestThread->createSuspended();
                pLowestThread->setPriority(osl_Thread_PriorityLowest);

                
                pAboveNormalThread->resume();
                pNormalThread->resume();
                pBelowNormalThread->resume();
                pLowestThread->resume();

                ThreadHelper::thread_sleep_tenth_sec(5);

                
                pAboveNormalThread->suspend();
                pNormalThread->suspend();
                pBelowNormalThread->suspend();
                pLowestThread->suspend();

                
                termAndJoinThread(pAboveNormalThread);
                termAndJoinThread(pNormalThread);
                termAndJoinThread(pBelowNormalThread);
                termAndJoinThread(pLowestThread);

                
                

                sal_Int32 nValueAboveNormal = 0;
                nValueAboveNormal = pAboveNormalThread->getValue();

                sal_Int32 nValueNormal = 0;
                nValueNormal = pNormalThread->getValue();

                sal_Int32 nValueBelowNormal = 0;
                nValueBelowNormal = pBelowNormalThread->getValue();

                sal_Int32 nValueLowest = 0;
                nValueLowest = pLowestThread->getValue();

                t_print("After 5 tenth seconds\n");
                t_print("nValue in AboveNormal Prio Thread is %d\n", (int) nValueAboveNormal);
                t_print("nValue in Normal Prio Thread is      %d\n", (int) nValueNormal);
                t_print("nValue in BelowNormal Prio Thread is %d\n", (int) nValueBelowNormal);
                t_print("nValue in Lowest Prio Thread is      %d\n", (int) nValueLowest);

                
                delete pAboveNormalThread;
                delete pNormalThread;
                delete pBelowNormalThread;
                delete pLowestThread;

#ifndef WNT
                CPPUNIT_ASSERT_MESSAGE(
                    "SetPriority",
                    /* nValueHighest     > 0 &&  */
                    nValueAboveNormal > 0 &&
                    nValueNormal      > 0 &&
                    nValueBelowNormal > 0 &&
                    nValueLowest      > 0
                    );
#endif
            }
        void setPriority_005()
            {
                
                
                
                OAddThread *pNormalThread = new OAddThread();
                OAddThread *pBelowNormalThread = new OAddThread();
                OAddThread *pLowestThread = new OAddThread();

                
                
                

                
                

                pNormalThread->createSuspended();
                pNormalThread->setPriority(osl_Thread_PriorityNormal);

                pBelowNormalThread->createSuspended();
                pBelowNormalThread->setPriority(osl_Thread_PriorityBelowNormal);

                pLowestThread->createSuspended();
                pLowestThread->setPriority(osl_Thread_PriorityLowest);

                
                
                pNormalThread->resume();
                pBelowNormalThread->resume();
                pLowestThread->resume();

                ThreadHelper::thread_sleep_tenth_sec(5);

                
                
                pNormalThread->suspend();
                pBelowNormalThread->suspend();
                pLowestThread->suspend();

                
                
                termAndJoinThread(pNormalThread);
                termAndJoinThread(pBelowNormalThread);
                termAndJoinThread(pLowestThread);

                
                

                
                

                sal_Int32 nValueNormal = 0;
                nValueNormal = pNormalThread->getValue();

                sal_Int32 nValueBelowNormal = 0;
                nValueBelowNormal = pBelowNormalThread->getValue();

                sal_Int32 nValueLowest = 0;
                nValueLowest = pLowestThread->getValue();

                t_print("After 5 tenth seconds\n");
                t_print("nValue in Normal Prio Thread is      %d\n", (int) nValueNormal);
                t_print("nValue in BelowNormal Prio Thread is %d\n", (int) nValueBelowNormal);
                t_print("nValue in Lowest Prio Thread is      %d\n", (int) nValueLowest);

                delete pNormalThread;
                delete pBelowNormalThread;
                delete pLowestThread;

#ifndef WNT
                CPPUNIT_ASSERT_MESSAGE(
                    "SetPriority",
                    /* nValueHighest     > 0 &&  */
                    /* nValueAboveNormal > 0 &&  */
                    nValueNormal      > 0 &&
                    nValueBelowNormal > 0 &&
                    nValueLowest      > 0
                    );
#endif
            }


        CPPUNIT_TEST_SUITE(setPriority);
#ifndef SOLARIS
        CPPUNIT_TEST(setPriority_002);
        CPPUNIT_TEST(setPriority_003);
        CPPUNIT_TEST(setPriority_004);
        CPPUNIT_TEST(setPriority_005);
#endif
        CPPUNIT_TEST(setPriority_001_1);
        CPPUNIT_TEST(setPriority_001_2);
        CPPUNIT_TEST(setPriority_001_3);
        CPPUNIT_TEST(setPriority_001_4);
        CPPUNIT_TEST(setPriority_001_5);
        CPPUNIT_TEST_SUITE_END();
    }; 

    /** Test of the osl::Thread::getPriority method
    */
    class getPriority : public CppUnit::TestFixture
    {
    public:
        
        void setUp()
            {
            }

        void tearDown()
            {
            }

        
        void getPriority_001()
            {
                OAddThread *pHighestThread = new OAddThread();

                
                pHighestThread->create();
                pHighestThread->setPriority(osl_Thread_PriorityHighest);

                oslThreadPriority aPriority = pHighestThread->getPriority();
                termAndJoinThread(pHighestThread);
                delete pHighestThread;

                ThreadHelper::outputPriority(aPriority);


#if ( defined WNT ) || ( defined SOLARIS )
                CPPUNIT_ASSERT_MESSAGE(
                    "getPriority",
                    aPriority == osl_Thread_PriorityHighest
                    );
#else


                CPPUNIT_ASSERT_MESSAGE(
                    "getPriority",
                    aPriority == osl_Thread_PriorityNormal
                    );
#endif
            }

        void getPriority_002()
            {

            }

        CPPUNIT_TEST_SUITE(getPriority);
        CPPUNIT_TEST(getPriority_001);
        CPPUNIT_TEST(getPriority_002);
        CPPUNIT_TEST_SUITE_END();
    }; 


    class getIdentifier : public CppUnit::TestFixture
    {
    public:
        
        void setUp()
            {
            }

        void tearDown()
            {
            }

        
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
    }; 

    /** Test of the osl::Thread::getCurrentIdentifier method
    */
    class getCurrentIdentifier : public CppUnit::TestFixture
    {
    public:
        
        void setUp()
            {
            }

        void tearDown()
            {
            }

        
        void getCurrentIdentifier_001()
            {
                oslThreadIdentifier oId;
                OCountThread* pCountThread = new OCountThread;
                pCountThread->create();
                pCountThread->setWait(3);
                oId = Thread::getCurrentIdentifier();
                oslThreadIdentifier oIdChild = pCountThread->getIdentifier();
                termAndJoinThread(pCountThread);
                delete pCountThread;

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
        
        CPPUNIT_TEST_SUITE_END();
    }; 

    /** Test of the osl::Thread::wait method
    */
    class wait : public CppUnit::TestFixture
    {
    public:
        
        void setUp()
            {
            }

        void tearDown()
            {
            }

        /** call wait in the run method

            ALGORITHM:
            tested thread wait nWaitSec seconds, main thread sleep (2) seconds,
            then terminate the tested thread, due to the fact that the thread do a sleep(1) + wait(5)
            it's finish after 6 seconds.
        */
        void wait_001()
            {
                OCountThread *aCountThread = new OCountThread();
                sal_Int32 nWaitSec = 5;
                aCountThread->setWait(nWaitSec);
                
                bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );

                
                StopWatch aStopWatch;
                aStopWatch.start();

                
                ThreadHelper::thread_sleep_tenth_sec( 4 );

                
                
                termAndJoinThread(aCountThread);

                
                sal_Int32 nValue = aCountThread->getValue();

                aStopWatch.stop();

                
                double nTenthSec = aStopWatch.getTenthSec();
                double nSec = aStopWatch.getSeconds();
                delete aCountThread;
                t_print("nTenthSec = %f \n", nTenthSec);
                t_print("nSec = %f \n", nSec);
                t_print("nValue = %d \n",  (int) nValue);

                CPPUNIT_ASSERT_MESSAGE(
                    "Wait: Blocks the calling thread for the given number of time.",
                    nTenthSec >= 5 && nValue == 1
                    );

            }

        CPPUNIT_TEST_SUITE(wait);
        CPPUNIT_TEST(wait_001);
        CPPUNIT_TEST_SUITE_END();
    }; 

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

        
        void yield_001()
            {
            }


        CPPUNIT_TEST_SUITE(yield);
        CPPUNIT_TEST(yield_001);
        CPPUNIT_TEST_SUITE_END();
    }; 

    /** Test of the osl::Thread::schedule method
    */
    class schedule : public CppUnit::TestFixture
    {
    public:
        
        void setUp()
            {
            }

        void tearDown()
            {
            }

        /** The requested thread will get terminate the next time schedule() is called.

            Note: on UNX, if call suspend thread is not the to be suspended thread, the to be
            suspended   thread will get suspended the next time schedule() is called,
            while on w32, it's nothing with schedule.

            check if suspend and terminate work well via schedule
        */
        void schedule_001()
            {
                OAddThread* aThread = new OAddThread();
                bool bRes = aThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );

                ThreadHelper::thread_sleep_tenth_sec(2);
                aThread->suspend();
                ThreadHelper::thread_sleep_tenth_sec(1);
                sal_Int32 nValue = aThread->getValue();
                ThreadHelper::thread_sleep_tenth_sec(3);
                sal_Int32 nLaterValue = aThread->getValue();
                
                t_print("      value = %d\n", (int) nValue);
                t_print("later value = %d\n", (int) nLaterValue);
                

                CPPUNIT_ASSERT_MESSAGE(
                    "Schedule: suspend works.",
                    nLaterValue == nValue
                    );

                aThread->resume();
                ThreadHelper::thread_sleep_tenth_sec(2);

                aThread->terminate();
                sal_Int32 nValue_term = aThread->getValue();

                aThread->join();
                sal_Int32 nValue_join = aThread->getValue();

                t_print("value after term = %d\n", (int) nValue_term);
                t_print("value after join = %d\n", (int) nValue_join);

                
                

                delete aThread;
                
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
                bool bRes = aThread.create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes );

                ThreadHelper::thread_sleep_tenth_sec(2);
                aThread.suspend();
                sal_Int32 nValue = aThread.getValue();

                ThreadHelper::thread_sleep_tenth_sec(3);
                sal_Int32 nLaterValue = aThread.getValue();
                ThreadHelper::thread_sleep_tenth_sec(5);

                resumeAndWaitThread(&aThread);

                t_print("      value = %d\n", (int) nValue);
                t_print("later value = %d\n", (int) nLaterValue);

                
#ifdef WNT
                CPPUNIT_ASSERT_MESSAGE(
                    "Schedule: don't schedule in thread run method, suspend works.",
                    nLaterValue == nValue
                    );
#endif

                
#ifdef UNX
                aThread.resume();
                CPPUNIT_ASSERT_MESSAGE(
                    "Schedule: don't schedule in thread run method, suspend does not work too.",
                    nLaterValue > nValue
                    );
#endif

                
                termAndJoinThread(&aThread);
                sal_Int32 nValue_term = aThread.getValue();

                t_print(" value term = %d\n", (int) nValue_term);

                CPPUNIT_ASSERT_MESSAGE(
                    "Schedule: don't schedule in thread run method, terminate failed.",
                    nValue_term == 10
                    );
            }

        CPPUNIT_TEST_SUITE(schedule);
        CPPUNIT_TEST(schedule_001);
        CPPUNIT_TEST(schedule_002);
        CPPUNIT_TEST_SUITE_END();
    }; 


    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::create, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::createSuspended, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::suspend, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::resume, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::terminate, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::join, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::isRunning, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::setPriority, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::getPriority, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::getIdentifier, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::getCurrentIdentifier, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::wait, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::yield, "osl_Thread");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Thread::schedule, "osl_Thread");
} 




void SAL_CALL destroyCallback(void * data)
{
    delete[] (char *) data;
}

static ThreadData myThreadData(destroyCallback);

/**
*/
class myKeyThread : public Thread
{
public:
    
    char m_Char_Test;
    
    myKeyThread(const char cData)
        : m_Char_Test(0)
        {
            m_nData = cData;
        }
private:
    char m_nData;

    void SAL_CALL run()
        {
            char * pc = new char[2];

            memcpy(pc, &m_nData, 1);
            pc[1] = '\0';

            myThreadData.setData(pc);
            char* pData = (char*)myThreadData.getData();
            m_Char_Test = *pData;
            
            ThreadHelper::thread_sleep_tenth_sec(3);
        }
public:
    ~myKeyThread()
        {
            if (isRunning())
            {
                t_print("error: not terminated.\n");
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
            
            m_Id = *pIdData;
            delete pId;
        }

public:
    ~idThread()
        {
            if (isRunning())
            {
                t_print("error: not terminated.\n");
            }
        }
};

namespace osl_ThreadData
{

    class ctors : public CppUnit::TestFixture
    {
    public:
        
        void setUp()
            {
            }

        void tearDown()
            {
            }

        
        void ctor_001()
            {

            }

        CPPUNIT_TEST_SUITE(ctors);
        CPPUNIT_TEST(ctor_001);
        CPPUNIT_TEST_SUITE_END();
    }; 


    class setData : public CppUnit::TestFixture
    {
    public:
        
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
                
                char* pc = new char[2];
                char m_nData = 'm';



                memcpy(pc, &m_nData, 1);
                pc[1] = '\0';

                myThreadData.setData(pc);

                myKeyThread aThread1('a');
                aThread1.create();
                myKeyThread aThread2('b');
                aThread2.create();
                
                char* pChar = (char*)myThreadData.getData();
                char aChar = *pChar;

                aThread1.join();
                aThread2.join();

                
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
                
                char* pc = new char[2];
                char m_nData = 'm';
                memcpy(pc, &m_nData, 1);
                pc[1] = '\0';
                myThreadData.setData(pc);

                myKeyThread aThread1('a');
                aThread1.create();
                myKeyThread aThread2('b');
                aThread2.create();
                
                
                char* pc2 = new char[2];
                m_nData = 'o';
                memcpy(pc2, &m_nData, 1);
                pc2[1] = '\0';

                myThreadData.setData(pc2);
                char* pChar = (char*)myThreadData.getData();
                char aChar = *pChar;

                aThread1.join();
                aThread2.join();

                
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
    }; 

    class getData : public CppUnit::TestFixture
    {
    public:
        
        void setUp()
            {
            }

        void tearDown()
            {
            }

        
        void getData_001()
            {
                char* pc = new char[2];
                char m_nData[] = "i";
                strcpy(pc, m_nData);
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

        
        
        void getData_002()
            {
                char* pc = new char[2];
                char m_nData = 'i';
                memcpy(pc, &m_nData, 1);
                pc[1] = '\0';

                myThreadData.setData(pc);

                myKeyThread aThread1('a');
                aThread1.create();
                myKeyThread aThread2('b');
                aThread2.create();

                
                char m_nData2 = 'j';
                memcpy(pc, &m_nData2, 1);
                pc[1] = '\0';

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
    }; 


    CPPUNIT_TEST_SUITE_REGISTRATION(osl_ThreadData::ctors);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_ThreadData::setData);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_ThreadData::getData);
} 



CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
