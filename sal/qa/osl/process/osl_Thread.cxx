/*************************************************************************
 *
 *  $RCSfile: osl_Thread.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 08:35:10 $
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
//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/string.hxx>
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
    T       m_nFlag;
    Mutex   m_aMutex;
public:
    ThreadSafeValue(T n = 0)
            : m_nFlag(n) {}
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
#ifdef WNT              //Windows
        Sleep(_nSec * 1000 );
#endif
#if ( defined UNX ) || ( defined OS2 )  //Unix
        sleep(_nSec);
#endif
        printf("# done\n");
    }
}

// -----------------------------------------------------------------------------
//Tested thread inherit Thread

class myThread : public Thread
{
    ThreadSafeValue<sal_Int32> m_aFlag;
public:
    sal_Int32 getValue() { return m_aFlag.getValue(); }
protected:
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
class OCountThread : public Thread
{
public:
    oslThreadIdentifier m_id;
    Mutex m_Mutex;
    OCountThread()
        {
            m_bSuspending = sal_False;
            m_bWait = sal_False;
            m_nWaitSec = 0;
            m_id = getIdentifier();
        }
    sal_Int32 getValue() { return m_aFlag.getValue(); }
    void setSuspend(sal_Bool bSupd)
        {
            m_bSuspending = bSupd;
        }
    void setWait(sal_Int32 nSec)
        {
            m_nWaitSec = nSec;
            m_bWait = sal_True;
        }

protected:
    ThreadSafeValue<sal_Int32> m_aFlag;
    sal_Bool m_bSuspending;
    sal_Bool m_bWait;
    sal_Int32 m_nWaitSec;

    void SAL_CALL run()
        {
            m_id = getIdentifier();
            //if the thread should terminate, schedule return false
            while (m_aFlag.getValue() < 20 && schedule() == sal_True)
            {
                m_aFlag.addValue(1);
                ThreadHelper::thread_sleep(1);

                if (m_bSuspending == sal_True)
                {
                    suspend();
                    m_bSuspending = sal_False;
                }
                if (m_bWait == sal_True)
                {
                    m_bWait = sal_False;
                    TimeValue nTV;
                    nTV.Seconds = m_nWaitSec;
                    nTV.Nanosec = 0;
                    wait(nTV);
                }
            }
        }
    void SAL_CALL onTerminated()
        {
            printf("# normally terminate this thread %d!\n", m_id);
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

// -----------------------------------------------------------------------------
class OAddThread : public Thread
{
    ThreadSafeValue<sal_Int32> m_aFlag;
public:
    oslThreadIdentifier m_id, m_CurId;
    OAddThread(){}
    sal_Int32 getValue() { return m_aFlag.getValue(); }

protected:
    void SAL_CALL run()
        {
            m_id = getIdentifier();
            m_CurId = getCurrentIdentifier();
            //if the thread should terminate, schedule return false
            while (schedule() == sal_True)
            {
                m_aFlag.addValue(1);
            }
        }
    void SAL_CALL onTerminated()
        {
            printf("# normally terminate this thread %d!\n", m_id);
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

            ALGORITHM:
            After an empty line, here comes the main description of the code entity, Here can be a longer text, examples,
            philosophical considerations and special solution descriptions.
            After this text, only @-tags are allowed. Each following text belongs to the last @-tag.
         */
        void create_001()
            {
                myThread *newthread = new myThread();
                sal_Bool bRes = newthread->create();
                CPPUNIT_ASSERT_MESSAGE("Can not creates a new thread!\n", bRes == sal_True );

                sal_Bool isRunning = newthread->isRunning();
                ThreadHelper::thread_sleep(3);
                sal_Int32 nValue = newthread->getValue();

                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Creates a new thread",
                        nValue == 1 && isRunning == sal_True
                        );
                newthread->join();
                delete newthread;
            }

        void create_002()
            {
                myThread *newthread = new myThread();
                sal_Bool res1 = newthread->create();
                sal_Bool res2 = newthread->create();

                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Creates a new thread: can not create two threads per instance",
                        res1 && !res2
                        );
                newthread->join();
                delete newthread;
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
        printf("# resumeAndWaitThread\n");
        _pThread->resume();
        // ThreadHelper::thread_sleep(1);
        // _pThread->suspend();
        // ThreadHelper::thread_sleep(1);
    }

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


        // insert your test code here.
        void createSuspended_001()
            {
                myThread *newthread = new myThread();
                sal_Bool bRes = newthread->createSuspended();
                CPPUNIT_ASSERT_MESSAGE("Can not creates a new thread!", bRes == sal_True );

                sal_Bool isRunning = newthread->isRunning();
                ThreadHelper::thread_sleep(5);
                sal_Int32 nValue = newthread->getValue();

                resumeAndWaitThread(newthread);

                newthread->terminate();

                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Creates a new suspended thread",
                        nValue == 0 && isRunning
                        );
                printf("#wait for join.\n");
                newthread->join();
                delete newthread;

            }

        void createSuspended_002()
            {
                myThread *newthread = new myThread();
                sal_Bool res1 = newthread->createSuspended();
                sal_Bool res2 = newthread->createSuspended();

                resumeAndWaitThread(newthread);
                newthread->terminate();
                // delete newthread;
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Creates a new thread: can not create two threads per instance",
                        res1 && !res2
                        );
                printf("#wait for join.\n");
                newthread->join();
                delete newthread;
            }

        CPPUNIT_TEST_SUITE(createSuspended);
        CPPUNIT_TEST(createSuspended_001);
        CPPUNIT_TEST(createSuspended_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class createSuspended


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

        void suspend_001()
            {
                OCountThread *aCountThread = new OCountThread();
                sal_Bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

                sal_Int32 nValue = 0;
                while (1)
                {
                    nValue = aCountThread->getValue();    // (1)
                    if (nValue >= 3)
                    {
                        aCountThread->setSuspend(sal_True);          // (2)
                        nValue = aCountThread->getValue();
                        break;
                    }
                }
                ThreadHelper::thread_sleep(3);

                sal_Int32 nLaterValue = aCountThread->getValue();
                //printf("# 6 sec later flag is %d now!\n ", nLaterValue);
                aCountThread->resume();
                aCountThread->terminate();
                //printf("# Suspend: after terminate!\n");

                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Suspend the thread",
                        bRes == sal_True && nValue == nLaterValue
                        );
                aCountThread->join();
                delete aCountThread;
            }

        void suspend_002()
            {

            }

        CPPUNIT_TEST_SUITE(suspend);
        CPPUNIT_TEST(suspend_001);
        CPPUNIT_TEST(suspend_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class suspend


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

        // insert your test code here.
        void resume_001()
            {
                OCountThread *aCountThread = new OCountThread();
                sal_Bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

                sal_Int32 nValue = 0;
                while (1)
                {
                    nValue = aCountThread->getValue();    // (1)
                    if (nValue >= 3)
                    {
                        aCountThread->setSuspend(sal_True);          // (2)
                        nValue = aCountThread->getValue();
                        //printf("# After suspend, flag is %d!\n ", nValue);
                        break;
                    }
                }
                ThreadHelper::thread_sleep(3);
                aCountThread->resume();
                //printf("# resume ends! ");
                nValue = aCountThread->getValue();
                //printf("# flag is %d!\n ", nValue);
                ThreadHelper::thread_sleep(3);
                sal_Int32 nLaterValue = aCountThread->getValue();
                //printf("# 3 sec later flag is %d now!\n ", nLaterValue);
                aCountThread->terminate();
                aCountThread->join();
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Suspend the thread",
                        bRes == sal_True && nLaterValue < 20 &&
                        nLaterValue - nValue > 1 && nLaterValue - nValue <= 3
                        );
                delete aCountThread;

            }

        void resume_002()
            {

            }

        CPPUNIT_TEST_SUITE(resume);
        CPPUNIT_TEST(resume_001);
        CPPUNIT_TEST(resume_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class resume


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

        // insert your test code here.
        void terminate_001()
            {
                OCountThread *aCountThread = new OCountThread();
                sal_Bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

                sal_Int32 nValue = 0;
                while (1)
                {
                    nValue = aCountThread->getValue();
                    if (nValue >= 3)
                    {
                        aCountThread->terminate();
                        break;
                    }
                }
                ThreadHelper::thread_sleep(3);
                nValue = aCountThread->getValue();
                //printf("# flag is %d now!\n ", nValue);
                sal_Bool isRunning = aCountThread->isRunning();
                aCountThread->join();
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Terminate the thread",
                        isRunning == sal_False && nValue == 3
                        );
                delete aCountThread;
            }

        void terminate_002()
            {

            }

        CPPUNIT_TEST_SUITE(terminate);
        CPPUNIT_TEST(terminate_001);
        //CPPUNIT_TEST(terminate_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class terminate


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

        // insert your test code here.
        void join_001()
            {
                OCountThread *aCountThread = new OCountThread();
                sal_Bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

                TimeValue* pTimeVal_befor = new TimeValue;
                osl_getSystemTime( pTimeVal_befor );
                //printf("#join:the system time is %d,%d\n", pTimeVal_befor->Seconds,pTimeVal_befor->Nanosec);

                aCountThread->join();

                //the below line will be executed after aCountThread terminate
                TimeValue* pTimeVal_after = new TimeValue;
                osl_getSystemTime( pTimeVal_after );
                //printf("#join:the system time is %d,%d\n", pTimeVal_after->Seconds,pTimeVal_after->Nanosec);

                sal_uInt32 nSec = pTimeVal_after->Seconds - pTimeVal_befor->Seconds;
                delete pTimeVal_after;
                delete pTimeVal_befor;

                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Join the thread: after the thread terminate",
                        nSec >= 20
                        );
                delete aCountThread;
            }

        void join_002()
            {
                OCountThread *aCountThread = new OCountThread();
                sal_Bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );
                //record the time when the running begin
                TimeValue* pTimeVal_befor = new TimeValue;
                osl_getSystemTime( pTimeVal_befor );
                sal_Int32 nValue = 0;
                while (1)
                {
                    nValue = aCountThread->getValue();
                    if (nValue >= 3)
                    {
                        aCountThread->terminate();
                        break;
                    }
                }

                aCountThread->join();
                //the below line will be executed after aCountThread terminate
                TimeValue* pTimeVal_after = new TimeValue;
                osl_getSystemTime( pTimeVal_after );
                sal_uInt32 nSec = pTimeVal_after->Seconds - pTimeVal_befor->Seconds;
                delete pTimeVal_after;
                delete pTimeVal_befor;
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Join the thread: after thread terminate by another thread",
                        nSec == 3
                        );
                delete aCountThread;
            }

        CPPUNIT_TEST_SUITE(join);
        CPPUNIT_TEST(join_001);
        CPPUNIT_TEST(join_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class join


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

        // insert your test code here.
        void isRunning_001()
            {
                OCountThread *aCountThread = new OCountThread();
                sal_Bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

                sal_Bool bRun = aCountThread->isRunning();

                ThreadHelper::thread_sleep(2);
                aCountThread->terminate();
                aCountThread->join();
                sal_Bool bTer = aCountThread->isRunning();
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Test isRunning",
                        bRun == sal_True && bTer == sal_False
                        );
                delete aCountThread;
            }

        void isRunning_002()
            {
                OCountThread *aCountThread = new OCountThread();
                sal_Bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

                sal_Bool bRunning = aCountThread->isRunning();
                sal_Int32 nValue = 0;
                while (1)
                {
                    nValue = aCountThread->getValue();
                    if (nValue >= 3)
                    {
                        aCountThread->setSuspend(sal_True);
                        break;
                    }
                }
                sal_Bool bRunning_sup = aCountThread->isRunning();
                ThreadHelper::thread_sleep(2);
                aCountThread->resume();
                ThreadHelper::thread_sleep(2);
                sal_Bool bRunning_res = aCountThread->isRunning();
                aCountThread->terminate();
                aCountThread->join();
                sal_Bool bRunning_ter = aCountThread->isRunning();
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Test isRunning",
                        bRes == sal_True && bRunning_sup == sal_True &&
                        bRunning_res == sal_True && bRunning_ter == sal_False
                        );
                delete aCountThread;
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
                OAddThread *pThread = new OAddThread();
                OAddThread *p2Thread = new OAddThread();

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
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Quotient is zero, which means, there exist no right values.",
                        nQuotient != 0
                    );
                double nDeltaPercent = nDelta / nQuotient * 100;

                printf("# Delta value %d, percent %f\n",nDelta, nDeltaPercent);

                CPPUNIT_ASSERT_MESSAGE
                    (
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
                OAddThread *aHighestThread = new OAddThread();
                OAddThread *aAboveNormalThread = new OAddThread();
                OAddThread *aNormalThread = new OAddThread();
                //OAddThread *aBelowNormalThread = new OAddThread();
                //OAddThread *aLowestThread = new OAddThread();

                //Create them and start running at the same time
                aHighestThread->create();
                aHighestThread->setPriority(osl_Thread_PriorityHighest);

                aAboveNormalThread->create();
                aAboveNormalThread->setPriority(osl_Thread_PriorityAboveNormal);

                aNormalThread->create();
                aNormalThread->setPriority(osl_Thread_PriorityNormal);
                /*aBelowNormalThread->create();
                  aBelowNormalThread->setPriority(osl_Thread_PriorityBelowNormal);
                  aLowestThread->create();
                  aLowestThread->setPriority(osl_Thread_PriorityLowest);
                */
                ThreadHelper::thread_sleep(10);
                aHighestThread->terminate();
                aAboveNormalThread->terminate();
                aNormalThread->terminate();
                //aBelowNormalThread->terminate();
                //aLowestThread->terminate();

                sal_Int32 nValueHighest = 0;
                nValueHighest = aHighestThread->getValue();

                sal_Int32 nValueAboveNormal = 0;
                nValueAboveNormal = aAboveNormalThread->getValue();

                sal_Int32 nValueNormal = 0;
                nValueNormal = aNormalThread->getValue();

                sal_Int32 nValueBelowNormal = 0;
                //nValueBelowNormal = aBelowNormalThread->getValue();
                sal_Int32 nValueLowest = 0;
                //nValueLowest = aLowestThread->getValue();
                printf("# After 10 seconds\n");
                printf("# nValue in Highest Prio Thread is %d\n",nValueHighest);
                printf("# nValue in AboveNormal Prio Thread is %d\n",nValueAboveNormal);
                printf("# nValue in Normal Prio Thread is %d\n",nValueNormal);

                ThreadHelper::thread_sleep(2);
                delete aHighestThread;
                delete aAboveNormalThread;
                delete aNormalThread;
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "SetPriority",
                        nValueHighest > nValueAboveNormal &&
                        nValueAboveNormal > nValueNormal
                        );
            }

        void setPriority_003()
            {
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
                aHighestThread->terminate();
                aAboveNormalThread->terminate();
                aNormalThread->terminate();
                aBelowNormalThread->terminate();
                aLowestThread->terminate();

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

                ThreadHelper::thread_sleep(2);
                delete aHighestThread;
                delete aAboveNormalThread;
                delete aNormalThread;
                delete aBelowNormalThread;
                delete aLowestThread;

                CPPUNIT_ASSERT_MESSAGE
                    (
                        "SetPriority",
                        nValueHighest > nValueAboveNormal &&
                        nValueAboveNormal > nValueNormal &&
                        nValueNormal > nValueBelowNormal &&
                        nValueBelowNormal > nValueLowest
                        );

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
                aHighestThread->terminate();
                aHighestThread->join();
                delete aHighestThread;
                CPPUNIT_ASSERT_MESSAGE
                    (
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
                OCountThread* pCountThread2 = new OCountThread;
                pCountThread->create();
                pCountThread2->create();
                pCountThread2->yield();
                oId = pCountThread->getCurrentIdentifier();
                oslThreadIdentifier oIdChild = pCountThread->m_id;
                //printf("# CurrentId is %ld, Child1 id is %ld, Child2 id is %ld\n",oId, oIdChild,pCountThread2->m_id );
                pCountThread->terminate();
                pCountThread->join();
                delete pCountThread;
                pCountThread2->terminate();
                pCountThread2->join();
                delete pCountThread2;
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Get the identifier for the current active thread.",
                        oId != oIdChild
                        );

            }

        void getCurrentIdentifier_002()
            {
                oslThreadIdentifier oId;
                OCountThread* pCountThread = new OCountThread;
                OCountThread* pCountThread2 = new OCountThread;
                pCountThread->create();
                pCountThread2->create();
                pCountThread2->yield();
                oId = pCountThread->getCurrentIdentifier();
                //printf("# CurrentId is %ld, Child1 id is %ld, Child2 id is %ld\n",oId,
                //pCountThread->m_id,pCountThread2->m_id );
                pCountThread->terminate();
                pCountThread->join();
                delete pCountThread;
                pCountThread2->terminate();
                pCountThread2->join();
                delete pCountThread2;
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Get the identifier for the current active thread.",
                        sal_True
                        );
            }

        CPPUNIT_TEST_SUITE(getCurrentIdentifier);
        CPPUNIT_TEST(getCurrentIdentifier_001);
        CPPUNIT_TEST(getCurrentIdentifier_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class getCurrentIdentifier


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

        // insert your test code here.
        void wait_001()
            {
                OCountThread *aCountThread = new OCountThread();
                sal_Bool bRes = aCountThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

                //record the time when the running begin
                TimeValue* pTimeVal_befor = (TimeValue*)malloc( sizeof(TimeValue));
                osl_getSystemTime( pTimeVal_befor );
                //printf("#Wait:the system time is %d,%d\n", pTimeVal_befor->Seconds,pTimeVal_befor->Nanosec);

                sal_Int32 nValue = 0;
                sal_Int32 nWaitSec = 4;
                while (1)
                {
                    nValue = aCountThread->getValue();
                    if (nValue >= 3)
                    {
                        aCountThread->setWait(nWaitSec);
                        ThreadHelper::thread_sleep(nWaitSec + 2);
                        aCountThread->terminate();
                        break;
                    }
                }
                nValue = aCountThread->getValue();
                TimeValue* pTimeVal_after = ( TimeValue* )malloc( sizeof( TimeValue ) );
                osl_getSystemTime( pTimeVal_after );
                //printf("# Wait: the system time is %d,%d\n", pTimeVal_after->Seconds,pTimeVal_after->Nanosec);
                sal_uInt32 nSec = pTimeVal_after->Seconds - pTimeVal_befor->Seconds;
                delete pTimeVal_after;
                //free( pTimeVal_after);
                delete pTimeVal_befor;
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Wait: Blocks the calling thread for the given number of time.",
                        nSec - nValue == nWaitSec
                        );
                aCountThread->join();
                delete aCountThread;
            }

        void wait_002()
            {

            }

        CPPUNIT_TEST_SUITE(wait);
        CPPUNIT_TEST(wait_001);
        CPPUNIT_TEST(wait_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class wait


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

        // insert your test code here.
        void schedule_001()
            {
                OAddThread *aThread = new OAddThread();
                sal_Bool bRes = aThread->create();
                CPPUNIT_ASSERT_MESSAGE ( "Can't start thread!", bRes == sal_True );

                ThreadHelper::thread_sleep(1);
                aThread->terminate();
                TimeValue* pTimeVal_befor = (TimeValue*)malloc( sizeof(TimeValue));
                osl_getSystemTime( pTimeVal_befor );
                sal_Int32 nValue_term = aThread->getValue();
                //printf("# nValue after terminate is %d", nValue_term);
                //printf("# Schedule: the system time is %d,%d\n", pTimeVal_befor->Seconds,pTimeVal_befor->Nanosec);

                aThread->join();
                sal_Int32 nValue_join = aThread->getValue();
                //printf("# nValue after schedule is %d", nValue_join);
                TimeValue* pTimeVal_after = ( TimeValue* )malloc( sizeof( TimeValue ) );
                osl_getSystemTime( pTimeVal_after );
                //printf("# Schedule: the system time is %d,%d\n", pTimeVal_after->Seconds,pTimeVal_after->Nanosec);

                delete pTimeVal_after;
                delete pTimeVal_befor;
                CPPUNIT_ASSERT_MESSAGE
                    (
                        "Schedule: Returns False if the thread should terminate.",
                        nValue_join -  nValue_term <= 1 && nValue_join -  nValue_term >= 0
                        );
                delete aThread;
            }

        void schedule_002()
            {

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
                // CPPUNIT_ASSERT_STUB();
            }

        void ctor_002()
            {
                // CPPUNIT_ASSERT_STUB();
            }

        CPPUNIT_TEST_SUITE(ctors);
        CPPUNIT_TEST(ctor_001);
        CPPUNIT_TEST(ctor_002);
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

        // insert your test code here.
        void setData_001()
            {
                // CPPUNIT_ASSERT_STUB();
            }

        void setData_002()
            {
                // CPPUNIT_ASSERT_STUB();
            }

        CPPUNIT_TEST_SUITE(setData);
        CPPUNIT_TEST(setData_001);
        CPPUNIT_TEST(setData_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class setData


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

        // insert your test code here.
        void getData_001()
            {
                // CPPUNIT_ASSERT_STUB();
            }

        void getData_002()
            {
                // CPPUNIT_ASSERT_STUB();
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

