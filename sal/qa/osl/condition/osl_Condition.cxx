 /*************************************************************************
 *
 *  $RCSfile: osl_Condition.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $  $Date: 2003-09-08 13:20:34 $
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

#ifndef _OSL_CONDITION_CONST_H_
#include <osl_Condition_Const.h>
#endif

using namespace osl;
using namespace rtl;


//------------------------------------------------------------------------
// helper functions and classes
//------------------------------------------------------------------------

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    printf( "#printBool# " );
    ( sal_True == bOk ) ? printf( "TRUE!\n" ): printf( "FALSE!\n" );
}

/** print a UNI_CODE String.
*/
inline void printUString( const ::rtl::OUString & str )
{
    rtl::OString aString;

    printf( "#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf( "%s\n", aString.getStr( ) );
}

/** wait _nSec seconds.
*/
void thread_sleep( sal_Int32 _nSec )
{
    /// print statement in thread process must use fflush() to force display.
    printf( "# wait %d seconds. ", _nSec );
    fflush( stdout );

#ifdef WNT                               //Windows
    Sleep( _nSec * 1000 );
#endif
#if ( defined UNX ) || ( defined OS2 )   //Unix
    sleep( _nSec );
#endif
    printf( "# done\n" );
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
        CPPUNIT_ASSERT_MESSAGE( "#ConditionThread does not shutdown properly.\n", sal_False == this -> isRunning( ) );
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
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Condition::ctors, "osl_Condition");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Condition::set, "osl_Condition");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Condition::reset, "osl_Condition");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Condition::wait, "osl_Condition");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Condition::check, "osl_Condition");
// -----------------------------------------------------------------------------

} // namespace osl_Condition


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;
