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

// include files

#include <sal/types.h>

#include <osl/thread.hxx>
#include <osl/time.h>

#include <rtl/instance.hxx>
#include <rtl/ustring.hxx>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#ifdef WNT
#include <windows.h>
#else
#include <unistd.h>
#include <time.h>
#endif

#define CONST_TEST_STRING "gregorian"

namespace {
struct Gregorian : public rtl::StaticWithInit<rtl::OUString, Gregorian> {
    const rtl::OUString operator () () {
        return rtl::OUString( CONST_TEST_STRING );
    }
};
}

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
#if ( defined UNX )
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
}

/** Simple thread for testing Thread-create.
 * Just add 1 of value 0, and after running, result is 1.
 */
class OGetThread : public osl::Thread
{
    sal_Int32 m_nOK;
    sal_Int32 m_nFails;

    rtl::OUString m_sConstStr;
public:
    OGetThread()
            :m_nOK(0),
             m_nFails(0)
        {
            m_sConstStr = CONST_TEST_STRING;
        }

    sal_Int32 getOK() { return m_nOK; }
    sal_Int32 getFails() {return m_nFails;}

protected:

    /** guarded value which initialized 0

        @see ThreadSafeValue
    */
    void SAL_CALL run() override
        {
            while(schedule())
            {
                rtl::OUString aStr = Gregorian::get();
                if (aStr.equals(m_sConstStr))
                {
                    m_nOK++;
                }
                else
                {
                    m_nFails++;
                }
                ThreadHelper::thread_sleep_tenth_sec(1);
            }
        }

public:

    virtual void SAL_CALL suspend() override
        {
            ::osl::Thread::suspend();
        }

    virtual ~OGetThread()
        {
            if (isRunning())
            {
                printf("error: not terminated.\n");
            }
        }
};

namespace rtl_DoubleLocking
{

/** Test of the osl::Thread::create method
 */

    class getValue : public CppUnit::TestFixture
    {
    public:

        // initialise your test code values here.
        void setUp() override
            {
            }

        void tearDown() override
            {
            }

        void getValue_001()
            {
                rtl::OUString aStr = Gregorian::get();

                CPPUNIT_ASSERT_MESSAGE(
                    "Gregorian::get() failed, wrong value expected.",
                    !aStr.isEmpty()
                    );
            }

        /** check 2 threads.

            ALGORITHM:
            Here the function should show, that 2 different threads,
            which only increase a value, should run at the same time with same prio.
            The test fails, if the difference between the two values is more than 5%
            but IMHO this isn't a failure, it's only a feature of the OS.
        */

        void getValue_002()
            {
                // initial 5 threads with different priorities
                OGetThread* pThread = new OGetThread();
                OGetThread* p2Thread = new OGetThread();

                //Create them and start running at the same time
                pThread->create();
                p2Thread->create();

                ThreadHelper::thread_sleep_tenth_sec(5);

                pThread->terminate();
                p2Thread->terminate();

                pThread->join();
                p2Thread->join();

                sal_Int32 nValueOK = 0;
                nValueOK = pThread->getOK();

                sal_Int32 nValueOK2 = 0;
                nValueOK2 = p2Thread->getOK();

#if OSL_DEBUG_LEVEL > 2
                printf("Value in Thread #1 is %" SAL_PRIdINT32 "\n", nValueOK);
                printf("Value in Thread #2 is %" SAL_PRIdINT32 "\n", nValueOK2);
#else
                (void)nValueOK2;
#endif

                sal_Int32 nValueFails = 0;
                nValueFails = pThread->getFails();

                sal_Int32 nValueFails2 = 0;
                nValueFails2 = p2Thread->getFails();

#if OSL_DEBUG_LEVEL > 2
                printf("Fails in Thread #1 is %" SAL_PRIdINT32 "\n", nValueFails);
                printf("Fails in Thread #2 is %" SAL_PRIdINT32 "\n", nValueFails2);
#endif

                delete pThread;
                delete p2Thread;

                CPPUNIT_ASSERT_MESSAGE(
                    "getValue() failed, wrong value expected.",
                    nValueOK != 0 && nValueFails == 0 && nValueFails2 == 0
                    );
            }

        CPPUNIT_TEST_SUITE(getValue);
        CPPUNIT_TEST(getValue_001);
        CPPUNIT_TEST(getValue_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class create

    CPPUNIT_TEST_SUITE_REGISTRATION(rtl_DoubleLocking::getValue);
} // namespace rtl_DoubleLocking

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
