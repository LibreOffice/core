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
#include "gtest/gtest.h"
#include <osl_Mutex_Const.h>

#ifdef WNT
#include <tools/prewin.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tools/postwin.h>
#endif

using namespace osl;
using namespace rtl;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

/** print a UNI_CODE String.
*/
inline void printUString( const ::rtl::OUString & str )
{
    rtl::OString aString;

    printf("#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf("%s\n", aString.getStr( ) );
}

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    printf("#printBool# " );
    ( sal_True == bOk ) ? printf("YES!\n" ): printf("NO!\n" );
}

/** pause nSec seconds helper function.
*/
namespace ThreadHelper
{
    void thread_sleep( sal_Int32 _nSec )
    {
        /// print statement in thread process must use fflush() to force display.
        // t_print("# wait %d seconds. ", _nSec );
        fflush(stdout);

#ifdef WNT                               //Windows
        Sleep( _nSec * 1000 );
#endif
#if ( defined UNX ) || ( defined OS2 )   //Unix
        sleep( _nSec );
#endif
        // printf("# done\n" );
    }
    void thread_sleep_tenth_sec(sal_Int32 _nTenthSec)
     {
#ifdef WNT      //Windows
            Sleep(_nTenthSec * 100 );
#endif
#if ( defined UNX ) || ( defined OS2 )  //Unix
            TimeValue nTV;
            nTV.Seconds = static_cast<sal_uInt32>( _nTenthSec/10 );
            nTV.Nanosec = ( (_nTenthSec%10 ) * 100000000 );
            osl_waitThread(&nTV);
#endif
    }
}


//------------------------------------------------------------------------
// Beginning of the test cases for osl_Mutex class
//------------------------------------------------------------------------


/** mutually exclusive data
*/
struct resource {
    sal_Int32   data1;
    sal_Int32   data2;
    Mutex       lock;
};

/** IncreaseThread provide data.
*/
class IncreaseThread : public Thread
{
public:
    IncreaseThread( struct resource *pData ): pResource( pData ) { }

    ~IncreaseThread( )
    {
        EXPECT_TRUE(sal_False == this -> isRunning( )) << "#IncreaseThread does not shutdown properly.\n";
    }
protected:
    struct resource *pResource;

    void SAL_CALL run( )
    {
        pResource->lock.acquire( );
        for( sal_Int8 i = 0; i < 3; i++ )
        {
            pResource->data1++;
            yield( );  //yield() give CPU time to other thread, other thread if not block, they will change the data;
        }
        if ( pResource->data2 == 0 )
            pResource->data2 = ( pResource->data1 > 0 ? pResource->data1 : 0 - pResource->data1 );
        pResource->lock.release();
    }
};

/** DecreaseThread consume data.
*/
class DecreaseThread : public Thread
{
public:
    DecreaseThread( struct resource *pData ): pResource( pData ) { }

    ~DecreaseThread( )
    {
        EXPECT_TRUE(sal_False == this -> isRunning( )) << "#DecreaseThread does not shutdown properly.\n";
    }
protected:
    struct resource *pResource;

    void SAL_CALL run( )
    {
        pResource->lock.acquire( );
        for( sal_Int8 i = 0; i < 3; i++ )
        {
            pResource->data1--;
            yield( );  //yield() give CPU time to other thread, other thread if not block, they will change the data;
        }
        if ( pResource->data2 == 0 )
            pResource->data2 = ( pResource->data1 > 0 ? pResource->data1 : 0 - pResource->data1 );
        pResource->lock.release();
    }
};


/** chain structure used in Threads as critical resource
*/
struct chain {
    sal_Int32   buffer[ BUFFER_SIZE ];
    Mutex       lock;
    sal_Int8    pos;
};

/** PutThread write to the chain structure in a mutex manner.
*/
class PutThread : public Thread
{
public:
    //get the struct pointer to write data to buffer
    PutThread( struct chain* pData ): pChain( pData ) { }

    ~PutThread( )
    {
        EXPECT_TRUE(sal_False == this -> isRunning( )) << "#PutThread does not shutdown properly.\n";
    }
protected:
    struct chain* pChain;

    void SAL_CALL run( )
    {
        //block here if the mutex has been acquired
        pChain->lock.acquire( );

        //current position in buffer to write
        sal_Int8 nPos = pChain->pos;
        oslThreadIdentifier oId = getIdentifier( );
        //write data
                sal_Int8 i;
        for ( i = 0; i < 5; i++ )
        {
            pChain->buffer[ nPos + i ] = oId;
            yield( );
        }
        //revise the position
        pChain->pos = nPos + i;

        //finish writing, release the mutex
        pChain->lock.release();
    }
};

/** thread for testing Mutex acquire.
 */
class HoldThread : public Thread
{
public:
    //get the Mutex pointer to operate
    HoldThread( Mutex* pMutex ): pMyMutex( pMutex ) { }

    ~HoldThread( )
    {
        EXPECT_TRUE(sal_False == this -> isRunning( )) << "#HoldThread does not shutdown properly.\n";
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run()
    {
        // block here if the mutex has been acquired
        pMyMutex->acquire( );
        printf("# Mutex acquired. \n" );
        pMyMutex->release( );
    }
};

class WaitThread : public Thread
{
public:
    //get the Mutex pointer to operate
    WaitThread( Mutex* pMutex ): pMyMutex( pMutex ) { }

    ~WaitThread( )
    {
        EXPECT_TRUE(sal_False == this -> isRunning( )) << "#WaitThread does not shutdown properly.\n";
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( )
    {
        // block here if the mutex has been acquired
        pMyMutex->acquire( );
        ThreadHelper::thread_sleep_tenth_sec( 2 );
        pMyMutex->release( );
    }
};

/** thread for testing getGlobalMutex.
 */
class GlobalMutexThread : public Thread
{
public:
    //get the Mutex pointer to operate
    GlobalMutexThread( ){ }

    ~GlobalMutexThread( )
    {
        EXPECT_TRUE(sal_False == this -> isRunning( )) << "#GlobalMutexThread does not shutdown properly.\n";
    }
protected:
    void SAL_CALL run( )
    {
        // block here if the mutex has been acquired
        Mutex* pGlobalMutex;
        pGlobalMutex = pGlobalMutex->getGlobalMutex( );
        pGlobalMutex->acquire( );
        printf("# Global Mutex acquired. \n" );
        pGlobalMutex->release( );
    }
};


//--------------------------------------------------------------
namespace osl_Mutex
{

    /** Test of the osl::Mutex::constructor
     */
    class MutexConstructor : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        struct chain m_Data;
        struct resource m_Res;

        void SetUp( )
        {
            for ( sal_Int8 i=0; i < BUFFER_SIZE; i++ )
                m_Data.buffer[i] = 0;
            m_Data.pos = 0;

            m_Res.data1 = 0;
            m_Res.data2 = 0;
        }

        void TearDown()
        {
        }
    }; // class ctor

    /** Create two threads to write data to the same buffer, use Mutex to assure
        during one thread write data five times, the other thread should not begin writing.
        the two threads wrote two different datas: their thread ID, so we can check the datas
        in buffer to know the order of the two threads writing
    */
    TEST_F(MutexConstructor, ctor_001)
    {
        PutThread myThread1( &m_Data );
        PutThread myThread2( &m_Data );

        myThread1.create( );
        myThread2.create( );

        //wait until the two threads terminate
        myThread1.join( );
        myThread2.join( );

        sal_Bool bRes = sal_False;

        // every 5 datas should the same
        // LLA: this is not a good check, it's too fix
        if (m_Data.buffer[0] == m_Data.buffer[1] &&
            m_Data.buffer[1] == m_Data.buffer[2] &&
            m_Data.buffer[2] == m_Data.buffer[3] &&
            m_Data.buffer[3] == m_Data.buffer[4] &&
            m_Data.buffer[5] == m_Data.buffer[6] &&
            m_Data.buffer[6] == m_Data.buffer[7] &&
            m_Data.buffer[7] == m_Data.buffer[8] &&
            m_Data.buffer[8] == m_Data.buffer[9])
            bRes = sal_True;

        /*for (sal_Int8 i=0; i<BUFFER_SIZE; i++)
            printf("#data in buffer is %d\n", m_Data.buffer[i]);
        */

        ASSERT_TRUE(bRes == sal_True) << "Mutex ctor";

    }

    /** Create two threads to write data to operate on the same number , use Mutex to assure,
        one thread increase data 3 times, the other thread decrease 3 times, store the operate
        result when the first thread complete, if it is interrupt by the other thread, the stored
        number will not be 3.
    */
    TEST_F(MutexConstructor, ctor_002)
    {
        IncreaseThread myThread1( &m_Res );
        DecreaseThread myThread2( &m_Res );

        myThread1.create( );
        myThread2.create( );

        //wait until the two threads terminate
        myThread1.join( );
        myThread2.join( );

        sal_Bool bRes = sal_False;

        // every 5 datas should the same
        if ( ( m_Res.data1 == 0 ) && ( m_Res.data2 == 3 ) )
            bRes = sal_True;

        ASSERT_TRUE(bRes == sal_True) << "test Mutex ctor function: increase and decrease a number 3 times without interrupt.";
    }


    /** Test of the osl::Mutex::acquire method
     */
    class acquire : public ::testing::Test
    {
    public:
    }; // class acquire

    // acquire mutex in main thread, and then call acquire again in myThread,
    // the child thread should block, wait 2 secs, it still block.
    // Then release mutex in main thread, the child thread could return from acquire,
    // and go to exec next statement, so could terminate quickly.
    TEST_F(acquire, acquire_001 )
    {
        Mutex aMutex;
        //acquire here
        sal_Bool bRes = aMutex.acquire( );
        // pass the pointer of mutex to child thread
        HoldThread myThread( &aMutex );
        myThread.create( );

        ThreadHelper::thread_sleep_tenth_sec( 2 );
        // if acquire in myThread does not work, 2 secs is long enough,
        // myThread should terminate now, and bRes1 should be sal_False
        sal_Bool bRes1 = myThread.isRunning( );

        aMutex.release( );
        ThreadHelper::thread_sleep_tenth_sec( 1 );
        // after release mutex, myThread stops blocking and will terminate immediately
        sal_Bool bRes2 = myThread.isRunning( );
        myThread.join( );

        ASSERT_TRUE(bRes == sal_True && bRes1 == sal_True && bRes2 == sal_False) << "Mutex acquire";
    }

    //in the same thread, acquire twice should success
    TEST_F(acquire, acquire_002)
    {
        Mutex aMutex;
        //acquire here
        sal_Bool bRes = aMutex.acquire();
        sal_Bool bRes1 = aMutex.acquire();

        sal_Bool bRes2 = aMutex.tryToAcquire();

        aMutex.release();

        ASSERT_TRUE(bRes == sal_True && bRes1 == sal_True && bRes2 == sal_True) << "Mutex acquire";

    }


    /** Test of the osl::Mutex::tryToAcquire method
     */
    class tryToAcquire : public ::testing::Test
    {
    public:
    }; // class tryToAcquire

    // First let child thread acquire the mutex, and wait 2 secs, during the 2 secs,
    // in main thread, tryToAcquire mutex should return False
    // then after the child thread terminated, tryToAcquire should return True
    TEST_F(tryToAcquire, tryToAcquire_001)
    {
        Mutex aMutex;
        WaitThread myThread(&aMutex);
        myThread.create();

        // ensure the child thread acquire the mutex
        ThreadHelper::thread_sleep_tenth_sec(1);

        sal_Bool bRes1 = aMutex.tryToAcquire();

        if (bRes1 == sal_True)
            aMutex.release();
        // wait the child thread terminate
        myThread.join();

        sal_Bool bRes2 = aMutex.tryToAcquire();

        if (bRes2 == sal_True)
            aMutex.release();

    ASSERT_TRUE(bRes1 == sal_False && bRes2 == sal_True) << "Try to acquire Mutex";
    }


    /** Test of the osl::Mutex::release method
     */
    class release : public ::testing::Test
    {
    public:
    }; // class release

    /** acquire/release are not used in pairs: after child thread acquired mutex,
        the main thread release it, then any thread could acquire it.
    */
    TEST_F(release, release_001)
    {
        Mutex aMutex;
        WaitThread myThread( &aMutex );
        myThread.create( );

        // ensure the child thread acquire the mutex
        ThreadHelper::thread_sleep_tenth_sec( 1 );

        sal_Bool bRunning = myThread.isRunning( );
        sal_Bool bRes1 = aMutex.tryToAcquire( );
        // wait the child thread terminate
        myThread.join( );

        sal_Bool bRes2 = aMutex.tryToAcquire( );

        if ( bRes2 == sal_True )
            aMutex.release( );

        ASSERT_TRUE(bRes1 == sal_False && bRes2 == sal_True && bRunning == sal_True) << "release Mutex: try to aquire before and after the mutex has been released";

    }

    // how about release twice?
    TEST_F(release, release_002)
    {
// LLA: is this a real test?
#if 0
        Mutex aMutex;
        sal_Bool bRes1 = aMutex.release( );
        sal_Bool bRes2 = aMutex.release( );

        ASSERT_TRUE(bRes1 == sal_False && bRes2 == sal_False) << "release Mutex: mutex should not be released without aquire, should not release twice. although the behaviour is still under discussion, this test is passed on (LINUX), not passed on (SOLARIS)&(WINDOWS)";
#endif
    }


    /** Test of the osl::Mutex::getGlobalMutex method
     */
    class getGlobalMutex : public ::testing::Test
    {
    public:
    }; // class getGlobalMutex

    // initialise your test code values here.
    TEST_F(getGlobalMutex, getGlobalMutex_001)
    {
        Mutex* pGlobalMutex;
        pGlobalMutex = pGlobalMutex->getGlobalMutex();
        pGlobalMutex->acquire();

        GlobalMutexThread myThread;
        myThread.create();

        ThreadHelper::thread_sleep_tenth_sec(1);
        sal_Bool bRes1 = myThread.isRunning();

        pGlobalMutex->release();
        ThreadHelper::thread_sleep_tenth_sec(1);
        // after release mutex, myThread stops blocking and will terminate immediately
        sal_Bool bRes2 = myThread.isRunning();

        ASSERT_TRUE(bRes1 == sal_True && bRes2 == sal_False) << "Global Mutex works";
    }

    TEST_F(getGlobalMutex, getGlobalMutex_002 )
    {
        sal_Bool bRes;

        Mutex *pGlobalMutex;
        pGlobalMutex = pGlobalMutex->getGlobalMutex( );
        pGlobalMutex->acquire( );
        {
            Mutex *pGlobalMutex1;
            pGlobalMutex1 = pGlobalMutex1->getGlobalMutex( );
            bRes = pGlobalMutex1->release( );
        }

        ASSERT_TRUE(bRes == sal_True) << "Global Mutex works: if the code between {} get the different mutex as the former one, it will return false when release.";
    }

} // namespace osl_Mutex


//------------------------------------------------------------------------
// Beginning of the test cases for osl_Guard class
//------------------------------------------------------------------------

class GuardThread : public Thread
{
public:
    //get the Mutex pointer to operate
    GuardThread( Mutex* pMutex ): pMyMutex( pMutex ) { }

    ~GuardThread( )
    {
        EXPECT_TRUE(sal_False == this -> isRunning( )) << "#GuardThread does not shutdown properly.\n";
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( )
    {
        // block here if the mutex has been acquired
        MutexGuard aGuard( pMyMutex );
        ThreadHelper::thread_sleep_tenth_sec( 2 );
    }
};


namespace osl_Guard
{
    class GuardThreadConstructor : public ::testing::Test
    {
    public:
    }; // class ctor

    // insert your test code here.
    TEST_F(GuardThreadConstructor, ctor_001)
    {
        Mutex aMutex;
        GuardThread myThread(&aMutex);
        myThread.create();

        ThreadHelper::thread_sleep_tenth_sec(1);
        sal_Bool bRes = aMutex.tryToAcquire();
        // after 1 second, the mutex has been guarded, and the child thread should be running
        sal_Bool bRes1 = myThread.isRunning();

        myThread.join();
        sal_Bool bRes2 = aMutex.tryToAcquire();

        ASSERT_TRUE(bRes == sal_False && bRes1 == sal_True && bRes2 == sal_True) << "GuardThread constructor";
    }

    TEST_F(GuardThreadConstructor, ctor_002 )
    {
        Mutex aMutex;

        /// use reference constructor here
        MutexGuard myGuard( aMutex );

        /// the GuardThread will block here when it is initialised.
        GuardThread myThread( &aMutex );
        myThread.create( );

        /// is it still blocking?
        ThreadHelper::thread_sleep_tenth_sec( 2 );
        sal_Bool bRes = myThread.isRunning( );

        /// oh, release him.
        aMutex.release( );
        myThread.join( );

        ASSERT_TRUE(bRes == sal_True) << "GuardThread constructor: reference initialization, aquire the mutex before running the thread, then check if it is blocking.";
    }

} // namespace osl_Guard


//------------------------------------------------------------------------
// Beginning of the test cases for osl_ClearableGuard class
//------------------------------------------------------------------------

/** Thread for test ClearableGuard
 */
class ClearGuardThread : public Thread
{
public:
    //get the Mutex pointer to operate
    ClearGuardThread( Mutex* pMutex ): pMyMutex( pMutex ) {}

    ~ClearGuardThread( )
    {
        EXPECT_TRUE(sal_False == this -> isRunning( )) << "#ClearGuardThread does not shutdown properly.\n";
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( )
    {
        // acquire the mutex
        // printf("# ClearGuardThread" );
        ClearableMutexGuard aGuard( pMyMutex );
        ThreadHelper::thread_sleep( 5 );

        // release the mutex
        aGuard.clear( );
        ThreadHelper::thread_sleep( 2 );
    }
};

// -----------------------------------------------------------------------------
namespace osl_ClearableGuard
{

    class ClearableGuardConstructor : public ::testing::Test
    {
    public:
    }; // class ctor

    TEST_F(ClearableGuardConstructor, ctor_001)
    {
        Mutex aMutex;

        /// now, the aMutex has been guarded.
        ClearableMutexGuard myMutexGuard( &aMutex );

        /// it will return sal_False if the aMutex has not been Guarded.
        sal_Bool bRes = aMutex.release( );

        ASSERT_TRUE(bRes == sal_True) << "ClearableMutexGuard constructor, test the aquire operation when initilized.";
    }

    TEST_F(ClearableGuardConstructor, ctor_002 )
    {
        Mutex aMutex;

        /// now, the aMutex has been guarded, this time, we use reference constructor.
        ClearableMutexGuard myMutexGuard( aMutex );

        /// it will return sal_False if the aMutex has not been Guarded.
        sal_Bool bRes = aMutex.release( );

        ASSERT_TRUE(bRes == sal_True) << "ClearableMutexGuard constructor, test the aquire operation when initilized, we use reference constructor this time.";
    }

    class clear : public ::testing::Test
    {
    public:
    }; // class clear

    TEST_F(clear, clear_001)
    {
        Mutex aMutex;
        ClearGuardThread myThread(&aMutex);
        myThread.create();

        TimeValue aTimeVal_befor;
        osl_getSystemTime( &aTimeVal_befor );
        // wait 1 second to assure the child thread has begun
        ThreadHelper::thread_sleep(1);

        while (1)
        {
            if (aMutex.tryToAcquire() == sal_True)
            {
                break;
            }
            ThreadHelper::thread_sleep(1);
        }
        TimeValue aTimeVal_after;
        osl_getSystemTime( &aTimeVal_after );
        sal_Int32 nSec = aTimeVal_after.Seconds - aTimeVal_befor.Seconds;
        printf("nSec is %" SAL_PRIdINT32"\n", nSec);

        myThread.join();

        ASSERT_TRUE(nSec < 7 && nSec > 1) << "ClearableGuard method: clear";
    }

    TEST_F(clear, clear_002 )
    {
        Mutex aMutex;

        /// now, the aMutex has been guarded.
        ClearableMutexGuard myMutexGuard( &aMutex );

        /// launch the HoldThread, it will be blocked here.
        HoldThread myThread( &aMutex );
        myThread.create( );

        /// is it blocking?
        ThreadHelper::thread_sleep_tenth_sec( 4 );
        sal_Bool bRes = myThread.isRunning( );

        /// use clear to release.
        myMutexGuard.clear( );
        myThread.join( );
        sal_Bool bRes1 = myThread.isRunning( );

        ASSERT_TRUE(( sal_True == bRes ) && ( sal_False == bRes1 )) << "ClearableGuard method: clear, control the HoldThread's running status!";
    }

} // namespace osl_ClearableGuard


//------------------------------------------------------------------------
// Beginning of the test cases for osl_ResettableGuard class
//------------------------------------------------------------------------

/** Thread for test ResettableGuard
 */
class ResetGuardThread : public Thread
{
public:
    //get the Mutex pointer to operate
    ResetGuardThread( Mutex* pMutex ): pMyMutex( pMutex ) {}

    ~ResetGuardThread( )
    {
        EXPECT_TRUE(sal_False == this -> isRunning( )) << "#ResetGuardThread does not shutdown properly.\n";
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( )
    {
        // acquire the mutex
        printf("# ResettableGuard\n" );
        ResettableMutexGuard aGuard( pMyMutex );
        // release the mutex
        aGuard.clear( );
        ThreadHelper::thread_sleep_tenth_sec( 2 );
    }
};

// -----------------------------------------------------------------------------
namespace osl_ResettableGuard
{
    class ctor : public ::testing::Test
    {
    public:
    }; // class ctor

    TEST_F(ctor, ctor_001)
    {
        Mutex aMutex;

        /// now, the aMutex has been guarded.
        ResettableMutexGuard myMutexGuard( &aMutex );

        /// it will return sal_False if the aMutex has not been Guarded.
        sal_Bool bRes = aMutex.release( );

        ASSERT_TRUE(bRes == sal_True) << "ResettableMutexGuard constructor, test the aquire operation when initilized.";
    }

    TEST_F(ctor, ctor_002 )
    {
        Mutex aMutex;

        /// now, the aMutex has been guarded, this time, we use reference constructor.
        ResettableMutexGuard myMutexGuard( aMutex );

        /// it will return sal_False if the aMutex has not been Guarded.
        sal_Bool bRes = aMutex.release( );

        ASSERT_TRUE(bRes == sal_True) << "ResettableMutexGuard constructor, test the aquire operation when initilized, we use reference constructor this time.";
    }


    class reset : public ::testing::Test
    {
    public:
    }; // class reset


    TEST_F(reset, reset_001 )
    {
        Mutex aMutex;
        ResetGuardThread myThread( &aMutex );
        ResettableMutexGuard myMutexGuard( aMutex );
        myThread.create( );

        /// is it running? and clear done?
        sal_Bool bRes = myThread.isRunning( );
        myMutexGuard.clear( );
        ThreadHelper::thread_sleep_tenth_sec( 1 );

        /// if reset is not success, the release will return sal_False
        myMutexGuard.reset( );
        sal_Bool bRes1 = aMutex.release( );
        myThread.join( );

        ASSERT_TRUE(( sal_True == bRes ) && ( sal_True == bRes1 )) << "ResettableMutexGuard method: reset";
    }

    TEST_F(reset, reset_002 )
    {
#ifdef LINUX
        Mutex aMutex;
        ResettableMutexGuard myMutexGuard( &aMutex );

        /// shouldn't release after clear;
        myMutexGuard.clear( );
        sal_Bool bRes = aMutex.release( );

        /// can release after reset.
        myMutexGuard.reset( );
        sal_Bool bRes1 = aMutex.release( );

        ASSERT_TRUE(( sal_False == bRes ) && ( sal_True == bRes1 )) << "ResettableMutexGuard method: reset, release after clear and reset, on Solaris, the mutex can be release without aquire, so it can not passed on (SOLARIS), but not the reason for reset_002";
#endif
    }

} // namespace osl_ResettableGuard

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// The following sets variables for GNU EMACS
// Local Variables:
// tab-width:4
// End:
