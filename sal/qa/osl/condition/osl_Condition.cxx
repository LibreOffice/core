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

#include <osl_Condition_Const.h>
#include <stdlib.h>

using namespace osl;

enum ConditionType
{
    thread_type_set,
    thread_type_reset,
    thread_type_wait,
    thread_type_check
};

/** thread for testing Condition.
 */
class ConditionThread : public Thread
{
public:
    //get the Condition to operate
    ConditionThread( ::osl::Condition& Con, ConditionType tType): m_MyCon( Con ), m_MyType( tType ) { }

    virtual ~ConditionThread( )
    {
        // LLA: do not throw in DTors!
        // LLA: CPPUNIT_ASSERT_MESSAGE( "#ConditionThread does not shutdown properly.\n", sal_False == this -> isRunning( ) );
    }
protected:
    ::osl::Condition& m_MyCon;
    ConditionType m_MyType;

    void SAL_CALL run() override
    {
        switch ( m_MyType )
        {
            case thread_type_wait:
                m_MyCon.wait(); break;
            case thread_type_set:
                m_MyCon.set(); break;
            case thread_type_reset:
                m_MyCon.reset(); break;
            default:
                break;
        }
    }
};

// test code start here

namespace osl_Condition
{

    /** testing the method:
        Condition()
    */
    class ctors : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void ctors_001( )
        {
            ::osl::Condition aCond;
            bRes = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create a condition its initial check state should be sal_False.",
                                    !bRes );
        }

        void ctors_002( )
        {
            ::osl::Condition aCond;
            aCond.set( );
            bRes = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create a condition and set it.",
                                    bRes );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST( ctors_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class ctors

    /** testing the method:
        void set()
    */
    class set : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1, bRes2;

        void set_001( )
        {
            ::osl::Condition aCond;
            aCond.set( );
            bRes = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: check state should be sal_True after set.",
                                    bRes );
        }

        void set_002( )
        {
            ::osl::Condition aCond;
            ConditionThread myThread1( aCond, thread_type_wait );
            myThread1.create();
            bRes = myThread1.isRunning( );

            ConditionThread myThread2( aCond, thread_type_set );
            myThread2.create();

            myThread1.join( );
            bRes1 = myThread1.isRunning( );
            bRes2 = aCond.check( );
            myThread2.join( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: use one thread to set the condition in order to release another thread.",
                                    bRes && !bRes1 && bRes2 );
        }

        CPPUNIT_TEST_SUITE( set );
        CPPUNIT_TEST( set_001 );
        CPPUNIT_TEST( set_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class set

    /** testing the method:
        void reset()
    */
    class reset : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1, bRes2;

        void reset_001( )
        {
            ::osl::Condition aCond;
            aCond.reset( );

            ConditionThread myThread( aCond, thread_type_wait );
            myThread.create();
            bRes = myThread.isRunning( );
            bRes2 = aCond.check( );

            aCond.set( );
            myThread.join( );
            bRes1 = myThread.isRunning( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: wait will cause a reset thread block, use set to release it.",
                                    bRes && !bRes1 && !bRes2 );
        }

        void reset_002( )
        {
            ::osl::Condition aCond;
            aCond.reset( );
            bRes = aCond.check( );
            aCond.set( );
            bRes1 = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create a condition and reset/set it.",
                                    !bRes && bRes1 );
        }

        CPPUNIT_TEST_SUITE( reset );
        CPPUNIT_TEST( reset_001 );
        CPPUNIT_TEST( reset_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class reset

    /** testing the method:
        Result wait(const TimeValue *pTimeout = 0)
    */
    class wait : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1, bRes2;
        TimeValue *tv1;

        void setUp( ) override
        {
            tv1 = new TimeValue;
            tv1->Seconds = 1;
            tv1->Nanosec = 0;

        }

        void tearDown( ) override
        {
            delete tv1;
        }

        void wait_001( )
        {
            ::osl::Condition cond1;
            ::osl::Condition cond2;
            ::osl::Condition cond3;

            cond1.set();
            cond2.set();

            osl::Condition::Result r1=cond1.wait(tv1);
            osl::Condition::Result r2=cond2.wait();
            osl::Condition::Result r3=cond3.wait(tv1);

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test three types of wait.",
                                    (r1 == ::osl::Condition::result_ok) &&
                                    (r2 == ::osl::Condition::result_ok) &&
                                    (r3 == ::osl::Condition::result_timeout) );
        }

        void wait_002( )
        {
            ::osl::Condition aCond;
            ::osl::Condition::Result wRes, wRes1;

            aCond.reset( );
            bRes = aCond.check( );
            wRes = aCond.wait( tv1 );

            aCond.set( );
            wRes1 = aCond.wait( tv1 );
            bRes1 = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: wait a condition after set/reset.",
                                    !bRes && bRes1 &&
                                    ( ::osl::Condition::result_timeout == wRes ) &&
                                    ( ::osl::Condition::result_ok == wRes1 ) );
        }

        CPPUNIT_TEST_SUITE( wait );
        CPPUNIT_TEST( wait_001 );
        CPPUNIT_TEST( wait_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class wait

    /** testing the method:
        sal_Bool check()
    */
    class check : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1, bRes2;

        void check_001( )
        {
            ::osl::Condition aCond;

            aCond.reset( );
            bRes = aCond.check( );
            aCond.set( );
            bRes1 = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: check the condition states.",
                                    !bRes && bRes1 );
        }

        void check_002( )
        {
            ::osl::Condition aCond;
            aCond.reset( );

            ConditionThread myThread( aCond, thread_type_set );
            myThread.create( );
            myThread.join( );
            bRes = aCond.check( );

            ConditionThread myThread1( aCond, thread_type_reset );
            myThread1.create( );
            myThread1.join( );
            bRes1 = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: use threads to set/reset Condition and check it in main routine.",
                                    bRes && !bRes1 );
        }

        CPPUNIT_TEST_SUITE( check );
        CPPUNIT_TEST( check_001 );
        CPPUNIT_TEST( check_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class check

CPPUNIT_TEST_SUITE_REGISTRATION(osl_Condition::ctors);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Condition::set);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Condition::reset);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Condition::wait);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Condition::check);

} // namespace osl_Condition

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
