/*************************************************************************
 *
 *  $RCSfile: osl_Mutex.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $  $Date: 2003-08-07 15:10:40 $
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

#ifndef _OSL_MUTEX_CONST_H_
#include <osl_Mutex_Const.h>
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

    printf( "#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf( "%s\n", aString.getStr( ) );
}

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    printf( "#printBool# " );
    ( sal_True == bOk ) ? printf( "YES!\n" ): printf( "NO!\n" );
}

/** pause nSec seconds helper function.
*/
namespace ThreadHelper
{
    void thread_sleep( sal_Int32 _nSec )
    {
        /// print statement in thread process must use fflush() to force display.
        printf( "# wait %d seconds. ", _nSec );
        fflush(stdout);

#ifdef WNT                               //Windows
        Sleep( _nSec * 1000 );
#endif
#if ( defined UNX ) || ( defined OS2 )   //Unix
        sleep( _nSec );
#endif
        printf( "# done\n" );
    }
}

/** print an Error that the thread is not shutdown properly.
*/
void termError( Thread *pThread, OString strThread = "")
{
    if ( pThread->isRunning( ) )
        printf( "# error: %s not terminated.\n", strThread );
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
        termError( this, "IncreaseThread" );
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
        termError( this, "DecreaseThread" );
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
        termError( this, "PutThread" );
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
        for ( sal_Int8 i = 0; i < 5; i++ )
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
        termError( this, "HoldThread" );
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run()
    {
        // block here if the mutex has been acquired
        pMyMutex->acquire( );
        printf( "# Mutex acquired. \n" );
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
        termError( this, "WaitThread" );
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( )
    {
        // block here if the mutex has been acquired
        pMyMutex->acquire( );
        ThreadHelper::thread_sleep( 2 );
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
        termError( this, "GlobalMutexThread" );
    }
protected:
    void SAL_CALL run( )
    {
        // block here if the mutex has been acquired
        Mutex* pGlobalMutex;
        pGlobalMutex = pGlobalMutex->getGlobalMutex( );
        pGlobalMutex->acquire( );
        printf( "# Global Mutex acquired. \n" );
        pGlobalMutex->release( );
    }
};


//--------------------------------------------------------------
namespace osl_Mutex
{

    /** Test of the osl::Mutex::constructor
     */
    class ctor : public CppUnit::TestFixture
    {
    public:
        // initialise your test code values here.
        struct chain m_Data;
        struct resource m_Res;

        void setUp( )
        {
            for ( sal_Int8 i=0; i < BUFFER_SIZE; i++ )
                m_Data.buffer[i] = 0;
            m_Data.pos = 0;

            m_Res.data1 = 0;
            m_Res.data2 = 0;
        }

        void tearDown()
        {
        }

        /** Create two threads to write data to the same buffer, use Mutex to assure
            during one thread write data five times, the other thread should not begin writing.
            the two threads wrote two different datas: their thread ID, so we can check the datas
            in buffer to know the order of the two threads writing
        */
        void ctor_001()
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

            CPPUNIT_ASSERT_MESSAGE("Mutex ctor", bRes == sal_True);

        }

        /** Create two threads to write data to operate on the same number , use Mutex to assure,
            one thread increase data 3 times, the other thread decrease 3 times, store the operate
            result when the first thread complete, if it is interrupt by the other thread, the stored
            number will not be 3.
        */
        void ctor_002()
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

            CPPUNIT_ASSERT_MESSAGE( "test Mutex ctor function: increase and decrease a number 3 times without interrupt.", bRes == sal_True );
        }

        CPPUNIT_TEST_SUITE( ctor );
        CPPUNIT_TEST( ctor_001 );
        CPPUNIT_TEST( ctor_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class ctor


    /** Test of the osl::Mutex::acquire method
     */
    class acquire : public CppUnit::TestFixture
    {
    public:
        // acquire mutex in main thread, and then call acquire again in myThread,
        // the child thread should block, wait 2 secs, it still block.
        // Then release mutex in main thread, the child thread could return from acquire,
        // and go to exec next statement, so could terminate quickly.
        void acquire_001( )
        {
            Mutex aMutex;
            //acquire here
            sal_Bool bRes = aMutex.acquire( );
            // pass the pointer of mutex to child thread
            HoldThread myThread( &aMutex );
            myThread.create( );

            ThreadHelper::thread_sleep( 2 );
            // if acquire in myThread does not work, 2 secs is long enough,
            // myThread should terminate now, and bRes1 should be sal_False
            sal_Bool bRes1 = myThread.isRunning( );

            aMutex.release( );
            ThreadHelper::thread_sleep( 1 );
            // after release mutex, myThread stops blocking and will terminate immediately
            sal_Bool bRes2 = myThread.isRunning( );
            myThread.join( );

            CPPUNIT_ASSERT_MESSAGE( "Mutex acquire",
                bRes == sal_True && bRes1 == sal_True && bRes2 == sal_False );
        }

        //in the same thread, acquire twice should success
        void acquire_002()
        {
            Mutex aMutex;
            //acquire here
            sal_Bool bRes = aMutex.acquire();
            sal_Bool bRes1 = aMutex.acquire();

            sal_Bool bRes2 = aMutex.tryToAcquire();

            aMutex.release();

            CPPUNIT_ASSERT_MESSAGE("Mutex acquire",
                bRes == sal_True && bRes1 == sal_True && bRes2 == sal_True);

        }

        CPPUNIT_TEST_SUITE( acquire );
        CPPUNIT_TEST( acquire_001 );
        CPPUNIT_TEST( acquire_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class acquire


    /** Test of the osl::Mutex::tryToAcquire method
     */
    class tryToAcquire : public CppUnit::TestFixture
    {
    public:
        // First let child thread acquire the mutex, and wait 2 secs, during the 2 secs,
        // in main thread, tryToAcquire mutex should return False
        // then after the child thread terminated, tryToAcquire should return True
        void tryToAcquire_001()
        {
            Mutex aMutex;
            WaitThread myThread(&aMutex);
            myThread.create();

            // ensure the child thread acquire the mutex
            ThreadHelper::thread_sleep(1);

            sal_Bool bRes1 = aMutex.tryToAcquire();

            if (bRes1 == sal_True)
                aMutex.release();
            // wait the child thread terminate
            myThread.join();

            sal_Bool bRes2 = aMutex.tryToAcquire();

            if (bRes2 == sal_True)
                aMutex.release();

        CPPUNIT_ASSERT_MESSAGE("Try to acquire Mutex",
                bRes1 == sal_False && bRes2 == sal_True);
        }

        CPPUNIT_TEST_SUITE(tryToAcquire);
        CPPUNIT_TEST(tryToAcquire_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class tryToAcquire

    /** Test of the osl::Mutex::release method
     */
    class release : public CppUnit::TestFixture
    {
    public:
        /** acquire/release are not used in pairs: after child thread acquired mutex,
            the main thread release it, then any thread could acquire it.
        */
        void release_001()
        {
            Mutex aMutex;
            WaitThread myThread( &aMutex );
            myThread.create( );

            // ensure the child thread acquire the mutex
            ThreadHelper::thread_sleep( 1 );

            sal_Bool bRunning = myThread.isRunning( );
            sal_Bool bRes1 = aMutex.tryToAcquire( );
            // wait the child thread terminate
            myThread.join( );

            sal_Bool bRes2 = aMutex.tryToAcquire( );

            if ( bRes2 == sal_True )
                aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE( "release Mutex: try to aquire before and after the mutex has been released",
                bRes1 == sal_False && bRes2 == sal_True && bRunning == sal_True );

        }

        // how about release twice?
        void release_002()
        {
            Mutex aMutex;
            sal_Bool bRes1 = aMutex.release( );
            sal_Bool bRes2 = aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE( "release Mutex: mutex should not be released without aquire, should not release twice. although the behaviour is still under discussion, this test is passed on (LINUX), not passed on (SOLARIS)&(WINDOWS)",
                bRes1 == sal_False && bRes2 == sal_False );
        }

        CPPUNIT_TEST_SUITE( release );
        CPPUNIT_TEST( release_001 );
        CPPUNIT_TEST( release_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class release



    /** Test of the osl::Mutex::getGlobalMutex method
     */
    class getGlobalMutex : public CppUnit::TestFixture
    {
    public:
        // initialise your test code values here.
        void getGlobalMutex_001()
        {
            Mutex* pGlobalMutex;
            pGlobalMutex = pGlobalMutex->getGlobalMutex();
            pGlobalMutex->acquire();

            GlobalMutexThread myThread;
            myThread.create();

            ThreadHelper::thread_sleep(1);
            sal_Bool bRes1 = myThread.isRunning();

            pGlobalMutex->release();
            ThreadHelper::thread_sleep(1);
            // after release mutex, myThread stops blocking and will terminate immediately
            sal_Bool bRes2 = myThread.isRunning();

            CPPUNIT_ASSERT_MESSAGE("Global Mutex works",
                bRes1 == sal_True && bRes2 == sal_False);
        }

        void getGlobalMutex_002( )
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

            CPPUNIT_ASSERT_MESSAGE( "Global Mutex works: if the code between {} get the different mutex as the former one, it will return false when release.",
                bRes == sal_True );
        }

        CPPUNIT_TEST_SUITE(getGlobalMutex);
        CPPUNIT_TEST(getGlobalMutex_001);
        CPPUNIT_TEST(getGlobalMutex_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class getGlobalMutex

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Mutex::ctor, "osl_Mutex");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Mutex::acquire, "osl_Mutex");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Mutex::tryToAcquire, "osl_Mutex");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Mutex::release, "osl_Mutex");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Mutex::getGlobalMutex, "osl_Mutex");
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
        termError( this, "GuardThread" );
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( )
    {
        // block here if the mutex has been acquired
        MutexGuard aGuard( pMyMutex );
        ThreadHelper::thread_sleep( 2 );
    }
};


namespace osl_Guard
{
    class ctor : public CppUnit::TestFixture
    {
    public:
        // insert your test code here.
        void ctor_001()
        {
            Mutex aMutex;
            GuardThread myThread(&aMutex);
            myThread.create();

            ThreadHelper::thread_sleep(1);
            sal_Bool bRes = aMutex.tryToAcquire();
            // after 1 second, the mutex has been guarded, and the child thread should be running
            sal_Bool bRes1 = myThread.isRunning();

            myThread.join();
            sal_Bool bRes2 = aMutex.tryToAcquire();

            CPPUNIT_ASSERT_MESSAGE("GuardThread constructor",
                bRes == sal_False && bRes1 == sal_True && bRes2 == sal_True);
        }

        void ctor_002( )
        {
            Mutex aMutex;

            /// use reference constructor here
            MutexGuard myGuard( aMutex );

            /// the GuardThread will block here when it is initialised.
            GuardThread myThread( &aMutex );
            myThread.create( );

            /// is it still blocking?
            ThreadHelper::thread_sleep( 2 );
            sal_Bool bRes = myThread.isRunning( );

            /// oh, release him.
            aMutex.release( );
            myThread.join( );

            CPPUNIT_ASSERT_MESSAGE("GuardThread constructor: reference initialization, aquire the mutex before running the thread, then check if it is blocking.",
                bRes == sal_True);
        }

        CPPUNIT_TEST_SUITE(ctor);
        CPPUNIT_TEST(ctor_001);
        CPPUNIT_TEST(ctor_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class ctor

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Guard::ctor, "osl_Guard");
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
        termError( this, "# ClearGuardThread" );
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( )
    {
        // acquire the mutex
        printf( "# ClearGuardThread" );
        ClearableMutexGuard aGuard( pMyMutex );
        ThreadHelper::thread_sleep( 2 );

        // release the mutex
        aGuard.clear( );
        ThreadHelper::thread_sleep( 2 );
    }
};

// -----------------------------------------------------------------------------
namespace osl_ClearableGuard
{

    class ctor : public CppUnit::TestFixture
    {
    public:
        void ctor_001()
        {
            Mutex aMutex;

            /// now, the aMutex has been guarded.
            ClearableMutexGuard myMutexGuard( &aMutex );

            /// it will return sal_False if the aMutex has not been Guarded.
            sal_Bool bRes = aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE("ClearableMutexGuard constructor, test the aquire operation when initilized.",
                bRes == sal_True );
        }

        void ctor_002( )
        {
            Mutex aMutex;

            /// now, the aMutex has been guarded, this time, we use reference constructor.
            ClearableMutexGuard myMutexGuard( aMutex );

            /// it will return sal_False if the aMutex has not been Guarded.
            sal_Bool bRes = aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE("ClearableMutexGuard constructor, test the aquire operation when initilized, we use reference constructor this time.",
                bRes == sal_True );
        }

        CPPUNIT_TEST_SUITE(ctor);
        CPPUNIT_TEST(ctor_001);
        CPPUNIT_TEST(ctor_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class ctor

    class clear : public CppUnit::TestFixture
    {
    public:
        void clear_001()
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
                    break;
            }
            TimeValue aTimeVal_after;
            osl_getSystemTime( &aTimeVal_after );
            sal_Int32 nSec = aTimeVal_after.Seconds - aTimeVal_befor.Seconds;
            // printf("# nSec is %d\n", nSec);

            myThread.join();

            CPPUNIT_ASSERT_MESSAGE("ClearableGuard method: clear",
                nSec < 4 && nSec > 1);
        }

        void clear_002( )
        {
            Mutex aMutex;

            /// now, the aMutex has been guarded.
            ClearableMutexGuard myMutexGuard( &aMutex );

            /// launch the HoldThread, it will be blocked here.
            HoldThread myThread( &aMutex );
            myThread.create( );

            /// is it blocking?
            ThreadHelper::thread_sleep( 2 );
            sal_Bool bRes = myThread.isRunning( );

            /// use clear to release.
            myMutexGuard.clear( );
            myThread.join( );
            sal_Bool bRes1 = myThread.isRunning( );

            CPPUNIT_ASSERT_MESSAGE( "ClearableGuard method: clear, control the HoldThread's running status!",
                ( sal_True == bRes ) && ( sal_False == bRes1 ) );
        }

        CPPUNIT_TEST_SUITE( clear );
        CPPUNIT_TEST( clear_001 );
        CPPUNIT_TEST( clear_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class clear

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_ClearableGuard::ctor, "osl_ClearableGuard" );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_ClearableGuard::clear, "osl_ClearableGuard" );
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
        termError( this, "# ResettableGuard" );
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( )
    {
        // acquire the mutex
        printf( "# ResettableGuard" );
        ResettableMutexGuard aGuard( pMyMutex );
        // release the mutex
        aGuard.clear( );
        ThreadHelper::thread_sleep( 2 );
    }
};

// -----------------------------------------------------------------------------
namespace osl_ResettableGuard
{
    class ctor : public CppUnit::TestFixture
    {
    public:
        void ctor_001()
        {
            Mutex aMutex;

            /// now, the aMutex has been guarded.
            ResettableMutexGuard myMutexGuard( &aMutex );

            /// it will return sal_False if the aMutex has not been Guarded.
            sal_Bool bRes = aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE("ResettableMutexGuard constructor, test the aquire operation when initilized.",
                bRes == sal_True );
        }

        void ctor_002( )
        {
            Mutex aMutex;

            /// now, the aMutex has been guarded, this time, we use reference constructor.
            ResettableMutexGuard myMutexGuard( aMutex );

            /// it will return sal_False if the aMutex has not been Guarded.
            sal_Bool bRes = aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE( "ResettableMutexGuard constructor, test the aquire operation when initilized, we use reference constructor this time.",
                bRes == sal_True );
        }


        CPPUNIT_TEST_SUITE(ctor);
        CPPUNIT_TEST(ctor_001);
        CPPUNIT_TEST(ctor_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class ctor

    class reset : public CppUnit::TestFixture
    {
    public:
        void reset_001( )
        {
            Mutex aMutex;
            ResetGuardThread myThread( &aMutex );
            myThread.create( );
            ResettableMutexGuard myMutexGuard( aMutex );

            /// is it running? and clear done?
            myMutexGuard.clear( );
            ThreadHelper::thread_sleep( 1 );
            sal_Bool bRes = myThread.isRunning( );

            /// if reset is not success, the release will return sal_False
            myMutexGuard.reset( );
            sal_Bool bRes1 = aMutex.release( );
            myThread.join( );

            CPPUNIT_ASSERT_MESSAGE( "ResettableMutexGuard method: reset",
                ( sal_True == bRes ) && ( sal_True == bRes1 ) );
        }

        void reset_002( )
        {
            Mutex aMutex;
            ResettableMutexGuard myMutexGuard( &aMutex );

            /// shouldn't release after clear;
            myMutexGuard.clear( );
            sal_Bool bRes = aMutex.release( );

            /// can release after reset.
            myMutexGuard.reset( );
            sal_Bool bRes1 = aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE( "ResettableMutexGuard method: reset, release after clear and reset, on Solaris, the mutex can be release without aquire, so it can not passed on (SOLARIS), but not the reason for reset_002",
                ( sal_False == bRes ) && ( sal_True == bRes1 ) );
        }

        CPPUNIT_TEST_SUITE(reset);
        CPPUNIT_TEST(reset_001);
#ifdef LINUX
        CPPUNIT_TEST(reset_002);
#endif
        CPPUNIT_TEST_SUITE_END();
    }; // class reset

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_ResettableGuard::ctor, "osl_ResettableGuard");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_ResettableGuard::reset, "osl_ResettableGuard");
} // namespace osl_ResettableGuard

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;
