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
#include <osl_Semaphore_Const.h>

using namespace osl;
using namespace rtl;


//------------------------------------------------------------------------
// helper functions and classes
//------------------------------------------------------------------------

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    t_print("#printBool# " );
    ( sal_True == bOk ) ? t_print("YES!\n" ): t_print("NO!\n" );
}

/** print a UNI_CODE String.
*/
inline void printUString( const ::rtl::OUString & str )
{
    rtl::OString aString;

    t_print("#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    t_print("%s\n", aString.getStr( ) );
}

/** wait _nSec seconds.
*/
void thread_sleep( sal_Int32 _nSec )
{
    /// print statement in thread process must use fflush() to force display.
    // t_print("wait %d seconds. ", _nSec );
    // fflush( stdout );

#ifdef WNT                               //Windows
    Sleep( _nSec * 1000 );
#endif
#if ( defined UNX ) || ( defined OS2 )   //Unix
    sleep( _nSec );
#endif
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

/** thread for testing Semaphore acquire.
 */
class HoldThread : public Thread
{
public:
    //get the Semaphores to operate
    HoldThread( ::osl::Semaphore& Sem ): MySem( Sem ) { }

    ~HoldThread( )
    {
        CPPUNIT_ASSERT_MESSAGE( "#HoldThread does not shutdown properly.\n", sal_False == this -> isRunning( ) );
    }
protected:
    ::osl::Semaphore& MySem;

    void SAL_CALL run()
    {
        // block here if it tries to decrease below zero.
        MySem.acquire( );
        t_print("Semaphore acquired. \n" );
        MySem.release( );
    }
};

/** thread for testing Semaphore release and tryToAcquire.
 */
class WaitThread : public Thread
{
public:
    //get the Semaphore pointer to operate
    WaitThread( ::osl::Semaphore& Sem ): MySem( Sem ) { }

    ~WaitThread( )
    {
        CPPUNIT_ASSERT_MESSAGE( "WaitThread does not shutdown properly.\n", sal_False == this -> isRunning( ) );
    }
protected:
    Semaphore& MySem;

    void SAL_CALL run( )
    {
        // block here if the semaphore has been acquired
        MySem.acquire( );
        thread_sleep_tenth_sec( 2 );
        MySem.release( );
    }
};

/** thread for producer-consumer model.
 */
#define BSIZE 50
class SemBuffer
{
public:
    sal_Int32 Buf[BSIZE];
    ::osl::Semaphore& aSemOccupied;
    ::osl::Semaphore& aSemEmpty;
    ::osl::Mutex& aMutex;

    SemBuffer( ::osl::Semaphore& Sem, ::osl::Semaphore& Sem1, ::osl::Mutex& Mut )
            :aSemOccupied( Sem ), aSemEmpty( Sem1 ), aMutex( Mut )
    {
        for ( sal_Int8 iCount=0; iCount < BSIZE; iCount++ )
            Buf[iCount] = 0;
    }
};

class WriterThread : public Thread
{
public:
    //get the Semaphores to operate
    WriterThread( SemBuffer& aSemBuffer ): MySemBuffer( aSemBuffer ){ }

    ~WriterThread( )
    {
        CPPUNIT_ASSERT_MESSAGE( "WriterThread does not shutdown properly.\n", sal_False == this -> isRunning( ) );
    }
protected:
    SemBuffer& MySemBuffer;

    void SAL_CALL run( )
    {
        for ( sal_Int32 iCount = 0; iCount < BSIZE; iCount++ )
        {
            MySemBuffer.aSemEmpty.acquire( ) ;
            MySemBuffer.aMutex.acquire( ) ;
            MySemBuffer.Buf[iCount] = iCount;
            MySemBuffer.aMutex.release( ) ;
            MySemBuffer.aSemOccupied.release( ) ;
        }
    }
};

class ReaderThread : public Thread
{
public:
    //get the Semaphores to operate
    ReaderThread( SemBuffer& aSemBuffer ): MySemBuffer( aSemBuffer ){ }

    ~ReaderThread( )
    {
        CPPUNIT_ASSERT_MESSAGE( "ReaderThread does not shutdown properly.\n", sal_False == this -> isRunning( ) );
    }

protected:
    SemBuffer& MySemBuffer;

    void SAL_CALL run( )
    {
        for ( sal_Int32 iCount = 0; iCount < BSIZE; iCount++ )
        {
            MySemBuffer.aSemOccupied.acquire( ) ;
            MySemBuffer.aMutex.acquire( ) ;
            MySemBuffer.Buf[iCount] *= MySemBuffer.Buf[iCount];
            MySemBuffer.aMutex.release( ) ;
            MySemBuffer.aSemEmpty.release( ) ;
        }
    }
};



//------------------------------------------------------------------------
// test code start here
//------------------------------------------------------------------------

namespace osl_Semaphore
{

    /** testing the method:
        Semaphore(sal_uInt32 initialCount);
    */
    class ctors : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void ctors_001( )
        {
            ::osl::Semaphore aSem(sal_uInt32(0));
            bRes = aSem.tryToAcquire( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create semaphore with initialCount = 0. the first acquire will block.",
                                    sal_False == bRes );
        }

        void ctors_002( )
        {
            ::osl::Semaphore aSem(sal_uInt32(1));
            bRes = aSem.tryToAcquire( );
            if ( sal_True == bRes )
                aSem.release( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create semaphore with initialCount = 1. the first acquire will not block.",
                                    sal_True == bRes );
        }

        void ctors_003( )
        {
            ::osl::Semaphore aSem(sal_uInt32(1));
            bRes = aSem.tryToAcquire( );
            bRes1 = aSem.tryToAcquire( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create semaphore with initialCount = 1. acquire twice will cause block.",
                                    sal_True == bRes && sal_False == bRes1);
        }

        void ctors_004( )
        {
            oslSemaphore hSem = new ::osl::Semaphore(sal_uInt32(1));

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test return value of the constructor, it should not be NULL.",
                                    NULL != hSem );
        }

        void ctors_005( )
        {
            ::osl::Semaphore aSemaphore(sal_uInt32(2));
            bRes =  aSemaphore.tryToAcquire( )&&
                    aSemaphore.tryToAcquire( )&&
                    !aSemaphore.tryToAcquire( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create semaphore with initialCount = 2. guess what behaviour will the semaphore act like.",
                                    sal_True == bRes );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST( ctors_002 );
        CPPUNIT_TEST( ctors_003 );
        CPPUNIT_TEST( ctors_004 );
        CPPUNIT_TEST( ctors_005 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class ctors


    /** testing the method:
        sal_Bool acquire();
    */
    class acquire : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1, bRes2;

        /**  tester comment:
            acquire semaphore in main thread, and then call acquire again in myThread,
            the child thread should block, wait 2 secs, it still block. Then release
            semaphore in main thread, the child thread could return from acquire, and
            go to exec next statement, so could terminate quickly.
        */

        void acquire_001( )
        {
            // launch the child thread
            ::osl::Semaphore aSemaphore(1);
            bRes = aSemaphore.acquire( );
            HoldThread myThread( aSemaphore );
            myThread.create( );

            // if acquire in myThread does not work, 2 secs is long enough,
            // myThread should terminate now, and bRes1 should be sal_False
            thread_sleep_tenth_sec( 2 );
            bRes1 = myThread.isRunning( );

            // after release semaphore, myThread stops blocking and will terminate immediately
            aSemaphore.release( );
            thread_sleep_tenth_sec( 1 );
            bRes2 = myThread.isRunning( );
            myThread.join( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test acquire of Semaphore.",
                bRes == sal_True && bRes1 == sal_True && bRes2 == sal_False );
        }

        /**  tester comment:
            launch 3 thread for testing acquirement inter-process.
        */

        void acquire_002( )
        {
            // launch three child threads
            ::osl::Semaphore aSemaphore(1);
            bRes = aSemaphore.acquire( );
            HoldThread myThread1( aSemaphore );
            myThread1.create( );
            HoldThread myThread2( aSemaphore );
            myThread2.create( );
            HoldThread myThread3( aSemaphore );
            myThread3.create( );

            // if acquire in myThread does not work, 2 secs is long enough,
            thread_sleep_tenth_sec( 2 );
            bRes1 = myThread1.isRunning( ) && myThread2.isRunning( ) && myThread3.isRunning( );

            // after release semaphore, myThread stops blocking and will terminate immediately
            aSemaphore.release( );
            thread_sleep_tenth_sec( 1 );
            bRes2 = myThread1.isRunning( ) || myThread2.isRunning( ) || myThread3.isRunning( );
            myThread1.join( );
            myThread2.join( );
            myThread3.join( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test acquire of Semaphore in multithreaded environment.",
                bRes == sal_True && bRes1 == sal_True && bRes2 == sal_False );
        }



        /**  tester comment:
            launch 3 thread for testing acquirement inter-process. in this test,
            we use two threads as producer and consumer, operate together on an
            array which is initialized to 0 for every item. producer takes action
            as follow:
                    p(A), p(M), Buf[i]=i, v(M), v(B).
            consumer's action is like:
                    p(B), p(M), Buf[i]=Buf[i]*Buf[i], v(M), v(A).
            consumer must operate on the array after producer does sequetially,
            otherwise, the array will contain items remain zero after all threads
            terminate. array will be filled with index^2 in the end.
        */

        void acquire_003( )
        {
            // initialization.
            ::osl::Semaphore aSemOccupied( sal_uInt32(0) );
            ::osl::Semaphore aSemEmpty( BSIZE );
            ::osl::Mutex aMutex;

            // launch two threads.
            SemBuffer aBuffer( aSemOccupied, aSemEmpty, aMutex );
            WriterThread myThread1( aBuffer );
            ReaderThread myThread2( aBuffer );
            myThread1.create( );
            myThread2.create( );

            myThread1.join( );
            myThread2.join( );

            bRes = sal_True;
            for ( sal_Int32 iCount = 0; iCount < BSIZE; iCount++ )
                bRes = bRes && ( aBuffer.Buf[iCount] == iCount*iCount );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test acquire of Semaphore using Producer-Consumer model.",
                sal_True == bRes );
        }

        CPPUNIT_TEST_SUITE( acquire );
        CPPUNIT_TEST( acquire_001 );
        CPPUNIT_TEST( acquire_002 );
        CPPUNIT_TEST( acquire_003 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class acquire


    /** testing the method:
        sal_Bool tryToAcquire();
    */
    class tryToAcquire : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1, bRes2;
        /**  tester comment:
            First let child thread acquire the semaphore, and wait 2 secs, during the 2 secs,
            in main thread, tryToAcquire semaphore should return False, then after the
            child thread terminated, tryToAcquire should return True
        */
        void tryToAcquire_001()
        {
            ::osl::Semaphore aSemaphore(1);
            WaitThread myThread( aSemaphore );
            myThread.create();

            // ensure the child thread acquire the semaphore
            thread_sleep_tenth_sec(1);
            bRes1 = aSemaphore.tryToAcquire();

            if (bRes1 == sal_True)
                aSemaphore.release();
            // wait the child thread terminate
            myThread.join();

            bRes2 = aSemaphore.tryToAcquire();
            if (bRes2 == sal_True)
                aSemaphore.release();

            CPPUNIT_ASSERT_MESSAGE("#test comment#: Try to acquire Semaphore",
                    bRes1 == sal_False && bRes2 == sal_True);
        }

        void tryToAcquire_002()
        {
            ::osl::Semaphore aSem(1);
            bRes = aSem.tryToAcquire( );
            bRes1 = aSem.tryToAcquire( );


            CPPUNIT_ASSERT_MESSAGE("#test comment#: Try to acquire Semaphore twice should block.",
                    sal_True == bRes && sal_False == bRes1);
        }

        CPPUNIT_TEST_SUITE( tryToAcquire );
        CPPUNIT_TEST( tryToAcquire_001 );
        CPPUNIT_TEST( tryToAcquire_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class tryToAcquire


    /** testing the method:
        sal_Bool release();
    */
    class release : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1, bRes2, bRunning;
        sal_Int32 nCount;
        /** acquire/release are not used in pairs: after child thread acquired semaphore,
            the main thread release it, then any thread could acquire it.
        */
        void release_001()
        {
            Semaphore aSemaphore(1);
            WaitThread myThread( aSemaphore );
            myThread.create( );

            // ensure the child thread acquire the mutex
            thread_sleep_tenth_sec( 1 );

            bRunning = myThread.isRunning( );
            bRes1 = aSemaphore.tryToAcquire( );
            // wait the child thread terminate
            myThread.join( );

            bRes2 = aSemaphore.tryToAcquire( );
            if ( bRes2 == sal_True )
                aSemaphore.release( );

            CPPUNIT_ASSERT_MESSAGE( "release Semaphore: try to aquire before and after the semaphore has been released",
                bRes1 == sal_False && bRes2 == sal_True && bRunning == sal_True );

        }

        void release_002()
        {
            Semaphore aSemaphore(sal_uInt32(0));
            bRes1 = sal_True;
            for ( nCount = 0; nCount < 10; nCount++, aSemaphore.release( ) ) { }
            for ( nCount = 0; nCount < 10; nCount++, bRes1 = bRes1 && aSemaphore.tryToAcquire( ) ) { }
            bRes = aSemaphore.tryToAcquire( );

            CPPUNIT_ASSERT_MESSAGE( "release Semaphore: release ten times and acquire eleven times.",
                sal_False == bRes && sal_True == bRes1);
        }

        CPPUNIT_TEST_SUITE( release );
        CPPUNIT_TEST( release_001 );
        CPPUNIT_TEST( release_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class release

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Semaphore::ctors, "osl_Semaphore");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Semaphore::acquire, "osl_Semaphore");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Semaphore::tryToAcquire, "osl_Semaphore");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Semaphore::release, "osl_Semaphore");
// -----------------------------------------------------------------------------

} // namespace osl_Semaphore


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;
