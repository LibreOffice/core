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
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include "osl_Mutex_Const.h"

using namespace osl;

/** pause nSec seconds helper function.
*/
namespace ThreadHelper
{
    static void thread_sleep_tenth_sec(sal_uInt32 _nTenthSec)
    {
        osl::Thread::wait(std::chrono::milliseconds(_nTenthSec * 100));
    }
    static void thread_sleep( sal_uInt32 _nSec )
    {
        /// print statement in thread process must use fflush() to force display.
        // t_print("# wait %d seconds. ", _nSec );
        fflush(stdout);

        thread_sleep_tenth_sec( _nSec * 10 );
        // printf("# done\n" );
    }
}

// Beginning of the test cases for osl_Mutex class

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
    explicit IncreaseThread( struct resource *pData ): pResource( pData ) { }

    virtual ~IncreaseThread( ) override
    {
        CPPUNIT_ASSERT_MESSAGE( "#IncreaseThread does not shutdown properly.\n", !isRunning( ) );
    }
protected:
    struct resource *pResource;

    void SAL_CALL run( ) override
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
    explicit DecreaseThread( struct resource *pData ): pResource( pData ) { }

    virtual ~DecreaseThread( ) override
    {
        CPPUNIT_ASSERT_MESSAGE( "#DecreaseThread does not shutdown properly.\n", !isRunning( ) );
    }
protected:
    struct resource *pResource;

    void SAL_CALL run( ) override
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
    explicit PutThread( struct chain* pData ): pChain( pData ) { }

    virtual ~PutThread( ) override
    {
        CPPUNIT_ASSERT_MESSAGE( "#PutThread does not shutdown properly.\n", !isRunning( ) );
    }
protected:
    struct chain* pChain;

    void SAL_CALL run( ) override
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
    explicit HoldThread( Mutex* pMutex ): pMyMutex( pMutex ) { }

    virtual ~HoldThread( ) override
    {
        CPPUNIT_ASSERT_MESSAGE( "#HoldThread does not shutdown properly.\n", !isRunning( ) );
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run() override
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
    explicit WaitThread( Mutex* pMutex ): pMyMutex( pMutex ) { }

    virtual ~WaitThread( ) override
    {
        CPPUNIT_ASSERT_MESSAGE( "#WaitThread does not shutdown properly.\n", !isRunning( ) );
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( ) override
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

    virtual ~GlobalMutexThread( ) override
    {
        CPPUNIT_ASSERT_MESSAGE( "#GlobalMutexThread does not shutdown properly.\n", !isRunning( ) );
    }
protected:
    void SAL_CALL run( ) override
    {
        // block here if the mutex has been acquired
        Mutex* pGlobalMutex;
        pGlobalMutex = Mutex::getGlobalMutex( );
        pGlobalMutex->acquire( );
        printf("# Global Mutex acquired. \n" );
        pGlobalMutex->release( );
    }
};

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

        void setUp( ) override
        {
            for ( sal_Int8 i=0; i < BUFFER_SIZE; i++ )
                m_Data.buffer[i] = 0;
            m_Data.pos = 0;

            m_Res.data1 = 0;
            m_Res.data2 = 0;
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

            bool bRes = false;

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
                bRes = true;

            /*for (sal_Int8 i=0; i<BUFFER_SIZE; i++)
                printf("#data in buffer is %d\n", m_Data.buffer[i]);
            */

            CPPUNIT_ASSERT_MESSAGE("Mutex ctor", bRes);

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

            bool bRes = false;

            // every 5 datas should the same
            if ( ( m_Res.data1 == 0 ) && ( m_Res.data2 == 3 ) )
                bRes = true;

            CPPUNIT_ASSERT_MESSAGE( "test Mutex ctor function: increase and decrease a number 3 times without interrupt.", bRes );
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
            bool bRes = aMutex.acquire( );
            // pass the pointer of mutex to child thread
            HoldThread myThread( &aMutex );
            myThread.create( );

            ThreadHelper::thread_sleep_tenth_sec( 2 );
            // if acquire in myThread does not work, 2 secs is long enough,
            // myThread should terminate now, and bRes1 should be sal_False
            bool bRes1 = myThread.isRunning( );

            aMutex.release( );
            ThreadHelper::thread_sleep_tenth_sec( 1 );
            // after release mutex, myThread stops blocking and will terminate immediately
            bool bRes2 = myThread.isRunning( );
            myThread.join( );

            CPPUNIT_ASSERT_MESSAGE( "Mutex acquire", bRes );
            CPPUNIT_ASSERT_MESSAGE( "Mutex acquire", bRes1 );
            CPPUNIT_ASSERT_MESSAGE( "Mutex acquire", !bRes2 );
        }

        //in the same thread, acquire twice should success
        void acquire_002()
        {
            Mutex aMutex;
            //acquire here
            bool bRes = aMutex.acquire();
            bool bRes1 = aMutex.acquire();

            bool bRes2 = aMutex.tryToAcquire();

            aMutex.release();

            CPPUNIT_ASSERT_MESSAGE("Mutex acquire", bRes);
            CPPUNIT_ASSERT_MESSAGE("Mutex acquire", bRes1);
            CPPUNIT_ASSERT_MESSAGE("Mutex acquire", bRes2);

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
            ThreadHelper::thread_sleep_tenth_sec(1);

            bool bRes1 = aMutex.tryToAcquire();

            if (bRes1)
                aMutex.release();
            // wait the child thread terminate
            myThread.join();

            bool bRes2 = aMutex.tryToAcquire();

            if (bRes2)
                aMutex.release();

            CPPUNIT_ASSERT_MESSAGE("Try to acquire Mutex", !bRes1);
            CPPUNIT_ASSERT_MESSAGE("Try to acquire Mutex", bRes2);
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
            ThreadHelper::thread_sleep_tenth_sec( 1 );

            bool bRunning = myThread.isRunning( );
            bool bRes1 = aMutex.tryToAcquire( );
            // wait the child thread terminate
            myThread.join( );

            bool bRes2 = aMutex.tryToAcquire( );

            if ( bRes2 )
                aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE( "release Mutex: try to acquire before and after the mutex has been released",
                !bRes1 );
            CPPUNIT_ASSERT_MESSAGE( "release Mutex: try to acquire before and after the mutex has been released",
                bRes2 );
            CPPUNIT_ASSERT_MESSAGE( "release Mutex: try to acquire before and after the mutex has been released",
                bRunning );

        }

        // how about release twice?
        void release_002()
        {
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
            pGlobalMutex = Mutex::getGlobalMutex();
            pGlobalMutex->acquire();

            GlobalMutexThread myThread;
            myThread.create();

            ThreadHelper::thread_sleep_tenth_sec(1);
            bool bRes1 = myThread.isRunning();

            pGlobalMutex->release();
            ThreadHelper::thread_sleep_tenth_sec(1);
            // after release mutex, myThread stops blocking and will terminate immediately
            bool bRes2 = myThread.isRunning();

            CPPUNIT_ASSERT_MESSAGE("Global Mutex works", bRes1);
            CPPUNIT_ASSERT_MESSAGE("Global Mutex works", !bRes2);
        }

        void getGlobalMutex_002( )
        {
            bool bRes;

            Mutex *pGlobalMutex;
            pGlobalMutex = Mutex::getGlobalMutex( );
            pGlobalMutex->acquire( );
            {
                Mutex *pGlobalMutex1;
                pGlobalMutex1 = Mutex::getGlobalMutex( );
                bRes = pGlobalMutex1->release( );
            }

            CPPUNIT_ASSERT_MESSAGE( "Global Mutex works: if the code between {} get the different mutex as the former one, it will return false when release.",
                bRes );
        }

        CPPUNIT_TEST_SUITE(getGlobalMutex);
        CPPUNIT_TEST(getGlobalMutex_001);
        CPPUNIT_TEST(getGlobalMutex_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class getGlobalMutex

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Mutex::ctor, "osl_Mutex");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Mutex::acquire, "osl_Mutex");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Mutex::tryToAcquire, "osl_Mutex");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Mutex::release, "osl_Mutex");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Mutex::getGlobalMutex, "osl_Mutex");
} // namespace osl_Mutex

// Beginning of the test cases for osl_Guard class

class GuardThread : public Thread
{
public:
    //get the Mutex pointer to operate
    explicit GuardThread( Mutex* pMutex ): pMyMutex( pMutex ) { }

    virtual ~GuardThread( ) override
    {
        CPPUNIT_ASSERT_MESSAGE( "#GuardThread does not shutdown properly.\n", !isRunning( ) );
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( ) override
    {
        // block here if the mutex has been acquired
        MutexGuard aGuard( pMyMutex );
        ThreadHelper::thread_sleep_tenth_sec( 2 );
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

            ThreadHelper::thread_sleep_tenth_sec(1);
            bool bRes = aMutex.tryToAcquire();
            // after 1 second, the mutex has been guarded, and the child thread should be running
            bool bRes1 = myThread.isRunning();

            myThread.join();
            bool bRes2 = aMutex.tryToAcquire();

            CPPUNIT_ASSERT_MESSAGE("GuardThread constructor",
                !bRes);
            CPPUNIT_ASSERT_MESSAGE("GuardThread constructor",
                bRes1);
            CPPUNIT_ASSERT_MESSAGE("GuardThread constructor",
                bRes2);
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
            ThreadHelper::thread_sleep_tenth_sec( 2 );
            bool bRes = myThread.isRunning( );

            /// oh, release him.
            aMutex.release( );
            myThread.join( );

            CPPUNIT_ASSERT_MESSAGE("GuardThread constructor: reference initialization, acquire the mutex before running the thread, then check if it is blocking.",
                bRes);
        }

        CPPUNIT_TEST_SUITE(ctor);
        CPPUNIT_TEST(ctor_001);
        CPPUNIT_TEST(ctor_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class ctor

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Guard::ctor, "osl_Guard");
} // namespace osl_Guard

// Beginning of the test cases for osl_ClearableGuard class

/** Thread for test ClearableGuard
 */
class ClearGuardThread : public Thread
{
public:
    //get the Mutex pointer to operate
    explicit ClearGuardThread( Mutex* pMutex ): pMyMutex( pMutex ) {}

    virtual ~ClearGuardThread( ) override
    {
        CPPUNIT_ASSERT_MESSAGE( "#ClearGuardThread does not shutdown properly.\n", !isRunning( ) );
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( ) override
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
            bool bRes = aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE("ClearableMutexGuard constructor, test the acquire operation when initialized.",
                bRes);
        }

        void ctor_002( )
        {
            Mutex aMutex;

            /// now, the aMutex has been guarded, this time, we use reference constructor.
            ClearableMutexGuard myMutexGuard( aMutex );

            /// it will return sal_False if the aMutex has not been Guarded.
            bool bRes = aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE("ClearableMutexGuard constructor, test the acquire operation when initialized, we use reference constructor this time.",
                bRes);
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

            while (true)
            {
                if (aMutex.tryToAcquire())
                {
                    break;
                }
                ThreadHelper::thread_sleep(1);
            }
            TimeValue aTimeVal_after;
            osl_getSystemTime( &aTimeVal_after );
            sal_Int32 nSec = aTimeVal_after.Seconds - aTimeVal_befor.Seconds;
            printf("nSec is %" SAL_PRIdINT32 "\n", nSec);

            myThread.join();

            CPPUNIT_ASSERT_MESSAGE("ClearableGuard method: clear",
                nSec < 7);
            CPPUNIT_ASSERT_MESSAGE("ClearableGuard method: clear",
                nSec > 1);
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
            ThreadHelper::thread_sleep_tenth_sec( 4 );
            bool bRes = myThread.isRunning( );

            /// use clear to release.
            myMutexGuard.clear( );
            myThread.join( );
            bool bRes1 = myThread.isRunning( );

            CPPUNIT_ASSERT_MESSAGE( "ClearableGuard method: clear, control the HoldThread's running status!",
                bRes );
            CPPUNIT_ASSERT_MESSAGE( "ClearableGuard method: clear, control the HoldThread's running status!",
                !bRes1 );
        }

        CPPUNIT_TEST_SUITE( clear );
        CPPUNIT_TEST( clear_001 );
        CPPUNIT_TEST( clear_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class clear

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_ClearableGuard::ctor, "osl_ClearableGuard" );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_ClearableGuard::clear, "osl_ClearableGuard" );
} // namespace osl_ClearableGuard

// Beginning of the test cases for osl_ResettableGuard class

/** Thread for test ResettableGuard
 */
class ResetGuardThread : public Thread
{
public:
    //get the Mutex pointer to operate
    explicit ResetGuardThread( Mutex* pMutex ): pMyMutex( pMutex ) {}

    virtual ~ResetGuardThread( ) override
    {
        CPPUNIT_ASSERT_MESSAGE( "#ResetGuardThread does not shutdown properly.\n", !isRunning( ) );
    }
protected:
    Mutex* pMyMutex;

    void SAL_CALL run( ) override
    {
        // acquire the mutex
        printf("# ResettableGuard\n" );
        ResettableMutexGuard aGuard( pMyMutex );
        // release the mutex
        aGuard.clear( );
        ThreadHelper::thread_sleep_tenth_sec( 2 );
    }
};

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
            bool bRes = aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE("ResettableMutexGuard constructor, test the acquire operation when initialized.",
                bRes);
        }

        void ctor_002( )
        {
            Mutex aMutex;

            /// now, the aMutex has been guarded, this time, we use reference constructor.
            ResettableMutexGuard myMutexGuard( aMutex );

            /// it will return sal_False if the aMutex has not been Guarded.
            bool bRes = aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE( "ResettableMutexGuard constructor, test the acquire operation when initialized, we use reference constructor this time.",
                bRes);
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
            ResettableMutexGuard myMutexGuard( aMutex );
            myThread.create( );

            /// is it running? and clear done?
            bool bRes = myThread.isRunning( );
            myMutexGuard.clear( );
            ThreadHelper::thread_sleep_tenth_sec( 1 );

            /// if reset is not success, the release will return sal_False
            myMutexGuard.reset( );
            bool bRes1 = aMutex.release( );
            myThread.join( );

            CPPUNIT_ASSERT_MESSAGE( "ResettableMutexGuard method: reset",
                bRes );
            CPPUNIT_ASSERT_MESSAGE( "ResettableMutexGuard method: reset",
                bRes1 );
        }

#ifdef LINUX
        void reset_002( )
        {
            Mutex aMutex;
            ResettableMutexGuard myMutexGuard( &aMutex );

            /// shouldn't release after clear;
            myMutexGuard.clear( );
            bool bRes = aMutex.release( );

            /// can release after reset.
            myMutexGuard.reset( );
            bool bRes1 = aMutex.release( );

            CPPUNIT_ASSERT_MESSAGE( "ResettableMutexGuard method: reset, release after clear and reset, on Solaris, the mutex can be release without acquire, so it can not passed on (SOLARIS), but not the reason for reset_002",
                !bRes );
            CPPUNIT_ASSERT_MESSAGE( "ResettableMutexGuard method: reset, release after clear and reset, on Solaris, the mutex can be release without acquire, so it can not passed on (SOLARIS), but not the reason for reset_002",
                bRes1 );
        }
#endif

        CPPUNIT_TEST_SUITE(reset);
        CPPUNIT_TEST(reset_001);
#ifdef LINUX
        CPPUNIT_TEST(reset_002);
#endif
        CPPUNIT_TEST_SUITE_END();
    }; // class reset

CPPUNIT_TEST_SUITE_REGISTRATION(osl_ResettableGuard::ctor);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_ResettableGuard::reset);
} // namespace osl_ResettableGuard

// The following sets variables for GNU EMACS
// Local Variables:
// tab-width:4
// End:

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
