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
#include <osl_Condition_Const.h>

#ifdef WNT
#include <Windows.h>
#endif

#include "gtest/gtest.h"

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
    printf("# wait %d seconds. ", _nSec );
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
    TEST(Sal_Test_Condition, ctors_001) {
        ::osl::Condition aCond;
        sal_Bool bRes = aCond.check( );

        // #test comment#: create a condition its initial check state should be sal_False.
        ASSERT_TRUE( !bRes );
    }

    TEST(Sal_Test_Condition, ctors_002) {
        ::osl::Condition aCond;
        aCond.set( );
        sal_Bool bRes = aCond.check( );

        // #test comment#: create a condition and set it.
        ASSERT_TRUE( bRes );
    }


    /** testing the method:
        void set()
    */
    TEST(Sal_Test_Condition, set_001) {
        ::osl::Condition aCond;
        aCond.set( );
        sal_Bool bRes = aCond.check( );

        // #test comment#: check state should be sal_True after set.
        ASSERT_TRUE( bRes );
    }

    TEST(Sal_Test_Condition, set_002) {
        ::osl::Condition aCond;
        ConditionThread myThread1( aCond, thread_type_wait );
        myThread1.create();
        sal_Bool bRes = myThread1.isRunning( );

        ConditionThread myThread2( aCond, thread_type_set );
        myThread2.create();
        thread_sleep(1);
        sal_Bool bRes1 = myThread1.isRunning( );
        sal_Bool bRes2 = aCond.check( );

        myThread1.join( );
        myThread2.join( );

        // #test comment#: use one thread to set the condition in order to release another thread."
        ASSERT_TRUE( bRes && !bRes1 && bRes2 );
    }


    /** testing the method:
        void reset()
    */
    TEST(Sal_Test_Condition, reset_001) {
        ::osl::Condition aCond;
        aCond.reset( );

        ConditionThread myThread( aCond, thread_type_wait );
        myThread.create();
        sal_Bool bRes = myThread.isRunning( );
        sal_Bool bRes2 = aCond.check( );

        aCond.set( );
        myThread.join( );
        sal_Bool bRes1 = myThread.isRunning( );

        // #test comment#: wait will cause a reset thread block, use set to release it.
        ASSERT_TRUE( bRes && !bRes1 && !bRes2 );
    }

    TEST(Sal_Test_Condition, reset_002) {
        ::osl::Condition aCond;
        aCond.reset( );
        sal_Bool bRes = aCond.check( );
        aCond.set( );
        sal_Bool bRes1 = aCond.check( );

        // #test comment#: create a condition and reset/set it.
        ASSERT_TRUE( sal_False == bRes && sal_True == bRes1 );
    }


    /** testing the method:
        Result wait(const TimeValue *pTimeout = 0)
    */
    TEST(Sal_Test_Condition, wait_001) {
        TimeValue tv1 = {1,0};

        ::osl::Condition cond1;
        ::osl::Condition cond2;
        ::osl::Condition cond3;

        cond1.set();
        cond2.set();

        osl::Condition::Result r1=cond1.wait(&tv1);
        osl::Condition::Result r2=cond2.wait();
        osl::Condition::Result r3=cond3.wait(&tv1);
        fprintf(stderr,"%d %d %d\n",r1,r2,r3);

        // #test comment#: test three types of wait.
        ASSERT_TRUE( cond1.wait(&tv1) == ::osl::Condition::result_ok );
        ASSERT_TRUE( cond2.wait() == ::osl::Condition::result_ok );
        ASSERT_TRUE( cond3.wait(&tv1) == ::osl::Condition::result_timeout );
    }

    TEST(Sal_Test_Condition, wait_002) {
        TimeValue tv1 = {1,0};

        ::osl::Condition aCond;
        ::osl::Condition::Result wRes, wRes1;

        aCond.reset( );
        sal_Bool bRes = aCond.check( );
        wRes = aCond.wait( &tv1 );

        aCond.set( );
        wRes1 = aCond.wait( &tv1 );
        sal_Bool bRes1 = aCond.check( );

        // #test comment#: wait a condition after set/reset.
        ASSERT_TRUE( !bRes );
        ASSERT_TRUE( bRes1 );
        ASSERT_TRUE( ::osl::Condition::result_timeout == wRes );
        ASSERT_TRUE( ::osl::Condition::result_ok == wRes1 );
    }


    /** testing the method:
        sal_Bool check()
    */
    TEST(Sal_Test_Condition, check_001) {
        ::osl::Condition aCond;
        aCond.reset( );
        sal_Bool bRes = aCond.check( );
        aCond.set( );
        sal_Bool bRes1 = aCond.check( );

        // #test comment#: check the condition states.
        ASSERT_TRUE( !bRes && bRes1 );
    }

    TEST(Sal_Test_Condition, check_002) {
        ::osl::Condition aCond;
        aCond.reset( );

        ConditionThread myThread( aCond, thread_type_set );
        myThread.create( );
        myThread.join( );
        sal_Bool bRes = aCond.check( );

        ConditionThread myThread1( aCond, thread_type_reset );
        myThread1.create( );
        myThread1.join( );
        sal_Bool bRes1 = aCond.check( );

        // #test comment#: use threads to set/reset Condition and check it in main routine.
        ASSERT_TRUE( bRes );
        ASSERT_TRUE( !bRes1 );
    }

} // namespace osl_Condition


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
