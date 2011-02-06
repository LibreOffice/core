/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------
#include <osl_Condition_Const.h>
#include <stdlib.h>

using namespace osl;
using namespace rtl;


//------------------------------------------------------------------------
// helper functions and classes
//------------------------------------------------------------------------

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    printf("#printBool# " );
    ( sal_True == bOk ) ? printf("TRUE!\n" ): printf("FALSE!\n" );
}

/** print a UNI_CODE String.
*/
inline void printUString( const ::rtl::OUString & str )
{
    rtl::OString aString;

    printf("#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf("%s\n", aString.getStr( ) );
}

/** wait _nSec seconds.
*/
void thread_sleep( sal_Int32 _nSec )
{
    /// print statement in thread process must use fflush() to force display.
    printf("# wait %d seconds. ", (int) _nSec );
    fflush( stdout );

#ifdef WNT                               //Windows
    Sleep( _nSec * 1000 );
#endif
#if ( defined UNX ) || ( defined OS2 )   //Unix
    sleep( _nSec );
#endif
    printf("# done\n" );
}

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

    ~ConditionThread( )
    {
        // LLA: do not throw in DTors!
        // LLA: CPPUNIT_ASSERT_MESSAGE( "#ConditionThread does not shutdown properly.\n", sal_False == this -> isRunning( ) );
    }
protected:
    ::osl::Condition& m_MyCon;
    ConditionType m_MyType;

    void SAL_CALL run()
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


//------------------------------------------------------------------------
// test code start here
//------------------------------------------------------------------------

namespace osl_Condition
{

    /** testing the method:
        Condition()
    */
    class ctors : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void ctors_001( )
        {
            ::osl::Condition aCond;
            bRes = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create a condition its initial check state should be sal_False.",
                                    sal_False == bRes );
        }

        void ctors_002( )
        {
            ::osl::Condition aCond;
            aCond.set( );
            bRes = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create a condition and set it.",
                                    sal_True == bRes );
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
        sal_Bool bRes, bRes1, bRes2;

        void set_001( )
        {
            ::osl::Condition aCond;
            aCond.set( );
            bRes = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: check state should be sal_True after set.",
                                    sal_True == bRes );
        }

        void set_002( )
        {
            ::osl::Condition aCond;
            ConditionThread myThread1( aCond, thread_type_wait );
            myThread1.create();
            bRes = myThread1.isRunning( );

            ConditionThread myThread2( aCond, thread_type_set );
            myThread2.create();
            thread_sleep(1);
            bRes1 = myThread1.isRunning( );
            bRes2 = aCond.check( );

            myThread1.join( );
            myThread2.join( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: use one thread to set the condition in order to release another thread.",
                                    sal_True == bRes && sal_False == bRes1 && sal_True == bRes2 );
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
        sal_Bool bRes, bRes1, bRes2;

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
                                    sal_True == bRes && sal_False == bRes1 && sal_False == bRes2 );
        }

        void reset_002( )
        {
            ::osl::Condition aCond;
            aCond.reset( );
            bRes = aCond.check( );
            aCond.set( );
            bRes1 = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create a condition and reset/set it.",
                                    ( sal_False == bRes && sal_True == bRes1 ) );
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
        sal_Bool bRes, bRes1, bRes2;
        TimeValue *tv1;

        void setUp( )
        {
            tv1 = (TimeValue*)malloc(sizeof(TimeValue));
            tv1->Seconds = 1;

        }

        void tearDown( )
        {
            free( tv1 );
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
fprintf(stderr,"%d %d %d\n",r1,r2,r3);
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test three types of wait.",
                                    (cond1.wait(tv1) == ::osl::Condition::result_ok) &&
                                    (cond2.wait() == ::osl::Condition::result_ok) &&
                                    (cond3.wait(tv1) == ::osl::Condition::result_timeout) );

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
                                    ( sal_False == bRes ) && ( sal_True == bRes1 ) &&
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
        sal_Bool bRes, bRes1, bRes2;

        void check_001( )
        {
            ::osl::Condition aCond;

            aCond.reset( );
            bRes = aCond.check( );
            aCond.set( );
            bRes1 = aCond.check( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: check the condition states.",
                                    ( sal_False == bRes && sal_True == bRes1 ) );
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
                                    ( sal_True == bRes && sal_False == bRes1 ) );
        }

        CPPUNIT_TEST_SUITE( check );
        CPPUNIT_TEST( check_001 );
        CPPUNIT_TEST( check_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class check


// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Condition::ctors);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Condition::set);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Condition::reset);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Condition::wait);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Condition::check);
// -----------------------------------------------------------------------------

} // namespace osl_Condition


// -----------------------------------------------------------------------------

CPPUNIT_PLUGIN_IMPLEMENT();


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
