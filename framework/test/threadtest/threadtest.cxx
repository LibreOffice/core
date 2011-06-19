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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/threadhelpbase.hxx>

#include <threadhelp/transactionbase.hxx>
#include <threadhelp/resetableguard.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/transactionguard.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <rtl/random.h>
#include <osl/process.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/time.h>

#include <osl/interlock.h>
#include <osl/thread.hxx>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <stdio.h>

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define LOGFILE             "threadtest.log"
#define STATISTICS_FILE     "threadtest_statistic.csv"

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl       ;
using namespace ::osl       ;
using namespace ::vos       ;
using namespace ::framework ;

//_________________________________________________________________________________________________________________
//  defines
//_________________________________________________________________________________________________________________

/*---------------- Use follow defines to enable/disable some special features of this little test program! -------*/

#define ENABLE_LOG
//#define ENABLE_THREADDELAY
#define ENABLE_REQUESTCOUNT

/*----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_LOG
    #define LOG_SETA_START( NA, NID )                                           \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ::osl::MutexGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp  );                              \
            sLog.append( ": Thread[ "           );                              \
            sLog.append( NID                    );                              \
            sLog.append( " ] call setA( "       );                              \
            sLog.append( NA                     );                              \
            sLog.append( " )\n"                 );                              \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear() )                 \
        }

    #define LOG_SETA_END( NA, EREASON, NID )                                    \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ::osl::MutexGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp  );                              \
            sLog.append( ": Thread[ "           );                              \
            sLog.append( NID                    );                              \
            if( EREASON == E_NOREASON )                                         \
                sLog.append( " ] finish setA( "         );                      \
            else                                                                \
                sLog.append( " ] was refused at setA( ");                       \
            sLog.append( NA     );                                              \
            sLog.append( " )\n" );                                              \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear() )                 \
        }

    #define LOG_GETA_START( NID )                                               \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ::osl::MutexGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp  );                              \
            sLog.append( ": Thread[ "           );                              \
            sLog.append( NID                    );                              \
            sLog.append( " ] call getA()\n"     );                              \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear() )                 \
        }

    #define LOG_GETA_END( NRETURN, EREASON, NID )                               \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ::osl::MutexGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp  );                              \
            sLog.append( ": Thread[ "           );                              \
            sLog.append( NID                    );                              \
            if( EREASON == E_NOREASON )                                         \
                sLog.append( " ] finish getA() with "           );              \
            else                                                                \
                sLog.append( " ] was refused at getA() with "   );              \
            sLog.append( NRETURN    );                                          \
            sLog.append( "\n"       );                                          \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear() )                 \
        }

    #define LOG_WORKA_START( NA, NID )                                          \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ::osl::MutexGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp  );                              \
            sLog.append( ": Thread[ "           );                              \
            sLog.append( NID                    );                              \
            sLog.append( " ] call workA( "      );                              \
            sLog.append( NA                     );                              \
            sLog.append( " )\n"                 );                              \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear() )                 \
        }

    #define LOG_WORKA_END( NRETURN, EREASON, NID )                              \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ::osl::MutexGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp  );                              \
            sLog.append( ": Thread[ "           );                              \
            sLog.append( NID                    );                              \
            if( EREASON == E_NOREASON )                                         \
                sLog.append( " ] finish workA() with "          );              \
            else                                                                \
                sLog.append( " ] was refused at workA() with "  );              \
            sLog.append( NRETURN    );                                          \
            sLog.append( "\n"       );                                          \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear() )                 \
        }

    #define LOG_INITEXCEPTION( SMETHOD, NID )                                   \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ::osl::MutexGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp              );                  \
            sLog.append( ": Thread[ "                       );                  \
            sLog.append( NID                                );                  \
            sLog.append( " ] get EInitException from \""    );                  \
            sLog.append( SMETHOD                            );                  \
            sLog.append( "\"\n"                             );                  \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear() )                 \
        }

    #define LOG_CLOSEEXCEPTION( SMETHOD, NID )                                  \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ::osl::MutexGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp              );                  \
            sLog.append( ": Thread[ "                       );                  \
            sLog.append( NID                                );                  \
            sLog.append( " ] get ECloseException from \""   );                  \
            sLog.append( SMETHOD                            );                  \
            sLog.append( "\"\n"                             );                  \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear() )                 \
        }

    #define LOG_INIT( NA, NID )                                                 \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ::osl::MutexGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp      );                          \
            sLog.append( ": Thread[ "               );                          \
            sLog.append( NID                        );                          \
            sLog.append( " ] initialize me with "   );                          \
            sLog.append( NA                         );                          \
            sLog.append( "\n"                       );                          \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear() )                 \
        }

    #define LOG_CLOSE( NID )                                                    \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ::osl::MutexGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp  );                              \
            sLog.append( ": Thread[ "           );                              \
            sLog.append( NID                    );                              \
            sLog.append( " ] close me\n"        );                              \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear() )                 \
        }
#else
    #define LOG_SETA_START( NA, NID )
    #define LOG_SETA_END( NA, EREASON, NID )
    #define LOG_GETA_START( NID )
    #define LOG_GETA_END( NRETURN, EREASON, NID )
    #define LOG_WORKA_START( NA, NID )
    #define LOG_WORKA_END( NRETURN, EREASON, NID )
    #define LOG_INITEXCEPTION( SMETHOD, NID )
    #define LOG_CLOSEEXCEPTION( SMETHOD, NID )
    #define LOG_INIT( NA, NID )
    #define LOG_CLOSE( NID )
#endif

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

sal_uInt16 getRandomValue()
{
    // Get new random value for thread-sleep!
    // See run() for further informations.
    // Always calculate a new random number.
    sal_uInt16      nValue;
    rtlRandomPool   aPool = rtl_random_createPool();
    rtl_random_getBytes     ( aPool, &nValue, 2 );
    rtl_random_destroyPool  ( aPool             );
    return nValue;
}

/*-************************************************************************************************************//**
    @descr          This class is used from different threads at the same time.
                    We start working after calling init() first(!) ...
                    and finish it by calling close(). It exist two methods for reading/writing an
                    internal variable "A". Another function workA() do both things at the same time.
                    All public methods log information in a file if DO_LOG is defined.

    @attention      Our public base class FaiRWLockBase is a struct with a RWLock as member.
                    This member can be used by guards to safe access at internal variables
                    in interface methods.
                    Another baseclass is the TransactionBase. They support rejection of wrong calls at wrong time.
                    e.g. calls after closing object!
*//*-*************************************************************************************************************/

class ThreadSafeClass : private ThreadHelpBase
                      , private TransactionBase

{
    public:

        ThreadSafeClass ();
        ~ThreadSafeClass();

        // This methods are used from differnt threads
        // to test this class.
        void        init    (   sal_Int32   nA          ,
                                sal_Int32   nThreadID   );
        void        close   (   sal_Int32   nThreadID   );
        void        setA    (   sal_Int32   nA          ,
                                sal_Int32   nThreadID   );
        sal_Int32   getA    (   sal_Int32   nThreadID   );
        sal_Int32   workA   (   sal_Int32   nA          ,
                                sal_Int32   nThreadID   );

        #ifdef ENABLE_REQUESTCOUNT
        // This methods are used for statistics only!
        sal_Int32 getReadCount () { return m_nReadCount;    }
        sal_Int32 getWriteCount() { return m_nWriteCount;   }
        #endif

    private:

        sal_Int32               m_nA            ;   /// test member fro reading/writing

        #ifdef ENABLE_LOG
        ::osl::Mutex            m_aLogMutex     ;   /// mutex to serialize writing log file!
        #endif

        #ifdef ENABLE_REQUESTCOUNT
        oslInterlockedCount     m_nReadCount    ;   /// statistic variables to count read/write requests
        oslInterlockedCount     m_nWriteCount   ;
        #endif
};

//_________________________________________________________________________________________________________________
ThreadSafeClass::ThreadSafeClass()
    :   ThreadHelpBase  (   )
    ,   TransactionBase (   )
    ,   m_nA            ( 0 )
    #ifdef ENABLE_REQUESTCOUNT
    ,   m_nReadCount    ( 0 )
    ,   m_nWriteCount   ( 0 )
    #endif
{
}

//_________________________________________________________________________________________________________________
ThreadSafeClass::~ThreadSafeClass()
{
}

//_________________________________________________________________________________________________________________
void ThreadSafeClass::init( sal_Int32 nA, sal_Int32 nThreadID )
{
    // Look for multiple calls of this method first!
    // Use E_SOFTEXCEPTIONS to disable automaticly throwing of exceptions for some working modes.
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    // Set write lock for setting internal member AND
    // protect changing of working mode!
    WriteGuard aWriteLock( m_aLock );
    LOG_INIT( nA, nThreadID )

    // OK, it must be the first call and we are synchronized with all other threads by using the write lock!
    // Otherwise (e.g. if working mode == E_WORK) we get a exception and follow lines are never called.

    // We can set our member and change the working mode now.
    m_nA = nA;

    aWriteLock.unlock();

    m_aTransactionManager.setWorkingMode( E_WORK );
}

//_________________________________________________________________________________________________________________
void ThreadSafeClass::close( sal_Int32 nThreadID )
{
    // We must look for multiple calls of this method.
    // Try to register this method as a transaction.
    // In combination with E_HARDEXCEPTIONS only working mode E_WORK pass this barrier.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    aTransaction.stop();

    // Change working mode to BEFORECLOSE to enable rejection of normal interface calls
    // and enable SOFTEXCEPTION mode for some impl- or helper methods!
    // Attention: We must stop successful registered transaction first ...
    // because setWorkingMode() blocks and wait for all current existing ones!
    m_aTransactionManager.setWorkingMode( E_BEFORECLOSE );

    // Make it threadsafe.
    // It must be an exclusiv access! => WriteLock!
    WriteGuard aWriteLock( m_aLock );

    LOG_CLOSE( nThreadID )

    // Now we are alone ...
    // All further calls to this object are rejected ...
    // (not all ... some special ones can work by using E_SOFTEXCEPTIONS!)

    // Deinitialize all member and set working mode to E_CLOSE.
    m_nA = 0;

    aWriteLock.unlock();

    m_aTransactionManager.setWorkingMode( E_CLOSE );
}

//_________________________________________________________________________________________________________________
void ThreadSafeClass::setA( sal_Int32 nA, sal_Int32 nThreadID   )
{
    // Register this method as a transaction to prevent code against wrong calls
    // after close() or before init()!
    ERejectReason eReason;
    TransactionGuard aTransaction( m_aTransactionManager, E_NOEXCEPTIONS, &eReason );
    if( eReason == E_NOREASON )
    {
        // Make it threadsafe.
        WriteGuard aWriteLock( m_aLock );

        LOG_SETA_START( nA, nThreadID )

        // This object is ready for working and we have full write access.
        // We can work with our member.
        m_nA = nA;
        #ifdef ENABLE_REQUESTCOUNT
        osl_incrementInterlockedCount( &m_nWriteCount );
        #endif
        LOG_SETA_END( nA, eReason, nThreadID )
    }
}

//_________________________________________________________________________________________________________________
sal_Int32 ThreadSafeClass::getA( sal_Int32 nThreadID )
{
    // Register this method as a transaction to prevent code against wrong calls
    // after close() or before init()!
    sal_Int32           nReturn = 0;
    ERejectReason       eReason;
    TransactionGuard    aTransaction( m_aTransactionManager, E_NOEXCEPTIONS, &eReason );
    if( eReason == E_NOREASON )
    {
        // Make it threadsafe.
        ReadGuard aReadLock( m_aLock );

        LOG_GETA_START( nThreadID )

        // This object is ready for working and we have a read access.
        // We can work with our member.
        nReturn = m_nA;
        #ifdef ENABLE_REQUESTCOUNT
        osl_incrementInterlockedCount( &m_nReadCount );
        #endif
        LOG_GETA_END( nReturn, eReason, nThreadID )
    }
    return nReturn;
}

//_________________________________________________________________________________________________________________
sal_Int32 ThreadSafeClass::workA(   sal_Int32   nA          ,
                                    sal_Int32   nThreadID   )
{
    // Register this method as a transaction to prevent code against wrong calls
    // after close() or before init()!
    sal_Int32           nReturn = 0;
    ERejectReason       eReason;
    TransactionGuard    aTransaction( m_aTransactionManager, E_NOEXCEPTIONS, &eReason );
    if( eReason == E_NOREASON )
    {
        // This method test the downgrade-mechanism of used lock implementation!
        // Make it threadsafe.
        WriteGuard aWriteLock( m_aLock );

        LOG_WORKA_START( nA, nThreadID )
        // We have write access to our member.
        // Set new value.
        m_nA = nA;
        #ifdef ENABLE_REQUESTCOUNT
        osl_incrementInterlockedCount( &m_nWriteCount );
        #endif

        // Downgrade write access to read access and read the set value again.
        // This call can't be rejected - but it can fail!
        aWriteLock.downgrade();
        nReturn = m_nA;
        #ifdef ENABLE_REQUESTCOUNT
        osl_incrementInterlockedCount( &m_nReadCount );
        #endif

        LOG_WORKA_END( nReturn, eReason, nThreadID )
    }
    return nReturn;
}

/*-****************************************************************************************************//**
    @descr  Every thread instance of these class lopp from 0 up to "nLoops".
            He sleep for a random time and work with given test class "pClass" then.
            We use random values for waiting for better results!
            Otherwise all threads are sychron after first 2,3...5 calls - I think!
*//*-*****************************************************************************************************/

class TestThread : public osl::Thread
{
    public:

        TestThread( ThreadSafeClass*    pClass                      ,
                    sal_Int32           nLoops                      ,
                    Condition*          pListener                   ,
                    sal_Bool            bOwner      =   sal_False   );

    private:

           virtual void SAL_CALL    run             ();
           virtual void SAL_CALL    onTerminated    ();

    private:

        ThreadSafeClass*    m_pClass        ;
        sal_Int32           m_nLoops        ;
        sal_Int32           m_nThreadID     ;
        Condition*          m_pListener     ;
        sal_Bool            m_bOwner        ;
};

//_________________________________________________________________________________________________________________
TestThread::TestThread( ThreadSafeClass*    pClass      ,
                        sal_Int32           nLoops      ,
                        Condition*          pListener   ,
                        sal_Bool            bOwner      )
    :   m_pClass    ( pClass    )
    ,   m_nLoops    ( nLoops    )
    ,   m_pListener ( pListener )
    ,   m_bOwner    ( bOwner    )
{
}

//_________________________________________________________________________________________________________________
void SAL_CALL TestThread::run()
{
    // Get ID of this thread.
    // Is used for logging information ...
    m_nThreadID = getCurrentIdentifier();

    // If we are the owner of given pClass
    // we must initialize ... and close
    // it. See at the end of this method too.
    if( m_bOwner == sal_True )
    {
        m_pClass->init( 0, m_nThreadID );
    }

    #ifdef ENABLE_THREADDELAY
    TimeValue   nDelay  ;
    #endif

    sal_Int32   nA      ;

    for( sal_Int32 nCount=0; nCount<m_nLoops; ++nCount )
    {
        // Work with class.
        // Use random to select called method.
        nA = (sal_Int32)getRandomValue();
        if( nA % 3 == 0 )
        {
            m_pClass->setA( nA, m_nThreadID );
        }
        else
        {
            nA = m_pClass->getA( m_nThreadID );
        }
        #ifdef ENABLE_THREADDELAY
        // Sleep - use random value to do that too!
        nDelay.Seconds = 0;
        nDelay.Nanosec = getRandomValue();
        sleep( nDelay );
        #endif
    }

    // Don't forget to "close" teset object if you are the owner!
    if( m_bOwner == sal_True )
    {
        m_pClass->close( m_nThreadID );
    }
}

//_________________________________________________________________________________________________________________
void SAL_CALL TestThread::onTerminated()
{
    // Destroy yourself if you finished.
    // But don't forget to call listener before.
    m_pListener->set();

    m_pClass    = NULL;
    m_pListener = NULL;

    delete this;
}

/*-****************************************************************************************************//**
    @descr  This is our test application.
            We create one ThreadSafeClass object and a lot of threads
            which use it at different times.
*//*-*****************************************************************************************************/

struct ThreadInfo
{
    Condition*  pCondition  ;
    TestThread* pThread     ;
};

class TestApplication : public Application
{
    public:
        void        Main        (                               );
        sal_Int32   measureTime (   sal_Int32   nThreadCount    ,
                                    sal_Int32   nOwner          ,
                                    sal_Int32   nLoops=0        );
};

//_________________________________________________________________________________________________________________
//  definition
//_________________________________________________________________________________________________________________

TestApplication aApplication;

//_________________________________________________________________________________________________________________
// This function start "nThreadCount" threads to use same test class.
// You can specify the owner thread of this test class which start/stop it by using "nOwner". [1..nThreadcount]!
// If you specify "nLoops" different from 0 we use it as loop count for every started thread.
// Otherwise we work with random values.
sal_Int32 TestApplication::measureTime( sal_Int32   nThreadCount    ,
                                           sal_Int32    nOwner          ,
                                          sal_Int32 nLoops          )
{
    // This is the class which should be tested.
    ThreadSafeClass aClass;

    // Create list of threads.
    ThreadInfo* pThreads    =   new ThreadInfo[nThreadCount];
    sal_Int32   nLoopCount  =   nLoops                      ;
    sal_Bool    bOwner      =   sal_False                   ;
    for( sal_Int32 nI=0; nI<nThreadCount; ++nI )
    {
        // If nLoops==0 => we must use random value; otherwise we must use given count ...
        if( nLoops == 0 )
        {
            nLoopCount = getRandomValue();
        }
        // Search owner of class.
        bOwner = sal_False;
        if( nOwner == nI )
        {
            bOwner = sal_True;
        }
        // initialize condition.
        pThreads[nI].pCondition = new Condition;
        // Initialize thread.
        pThreads[nI].pThread = new TestThread( &aClass, nLoopCount, pThreads[nI].pCondition, bOwner );
    }

    // Start clock to get information about used time.
    sal_uInt32  nStartTime  ;
    sal_uInt32  nEndTime    ;

    nStartTime = osl_getGlobalTimer();

    // Start threads ...
    for( nI=0; nI<nThreadCount; ++nI )
    {
        pThreads[nI].pThread->create();
    }

    // Wait for threads ...
    for( nI=0; nI<nThreadCount; ++nI )
    {
        pThreads[nI].pCondition->wait();
        delete pThreads[nI].pCondition;
        pThreads[nI].pCondition = NULL;
        pThreads[nI].pThread    = NULL;
    }

    delete[] pThreads;
    pThreads = NULL;

    nEndTime = osl_getGlobalTimer();

    // Calc used time and return it. [ms]
    return( nEndTime-nStartTime );
}

//_________________________________________________________________________________________________________________
void TestApplication::Main()
{
    sal_Int32 nTestCount    = 0;    /// count of calling "measureTime()"
    sal_Int32 nThreadCount  = 0;    /// count of used threads by "measure..."
    sal_Int32 nLoops        = 0;    /// loop count for every thread
    sal_Int32 nOwner        = 0;    /// number of owner thread

    // Parse command line.
    // Attention: All parameter are required and must exist!
    // syntax: "threadtest.exe <testcount> <threadcount> <loops> <owner>"
    OUString        sArgument   ;
    sal_Int32       nArgument   ;
    sal_Int32       nCount      = osl_getCommandArgCount();

    LOG_ASSERT2( nCount!=4 ,"TestApplication::Main()" , "Wrong argument line detected!")

    for( nArgument=0; nArgument<nCount; ++nArgument )
    {
        osl_getCommandArg( nArgument, &sArgument.pData );
        if( nArgument== 0 ) nTestCount  =sArgument.toInt32();
        if( nArgument== 1 ) nThreadCount=sArgument.toInt32();
        if( nArgument== 2 ) nLoops      =sArgument.toInt32();
        if( nArgument== 3 ) nOwner      =sArgument.toInt32();
    }

    LOG_ASSERT2( nTestCount==0||nThreadCount==0||nLoops==0||nOwner==0,"TestApplication::Main()", "Wrong argument value detected!" )

    // Start test.
    OStringBuffer   sBuf(256);
    sal_Int32       nTime=0;
    sBuf.append( "Nr.\tTime\tThreadCount\tLoops\tOwner\n" );
    for( sal_Int32 nI=1; nI<=nTestCount; ++nI )
    {
        nTime = measureTime( nThreadCount, nOwner, nLoops );
        sBuf.append( nI             );
        sBuf.append( "\t"           );
        sBuf.append( nTime          );
        sBuf.append( "\t"           );
        sBuf.append( nThreadCount   );
        sBuf.append( "\t"           );
        sBuf.append( nLoops         );
        sBuf.append( "\t"           );
        sBuf.append( nOwner         );
        sBuf.append( "\n"           );
    }

    WRITE_LOGFILE( STATISTICS_FILE, sBuf.makeStringAndClear() );
    LOG_ERROR( "TApplication::Main()", "Test finish successful!" )
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
