/*************************************************************************
 *
 *  $RCSfile: threadtest.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RWLOCKBASE_HXX_
#include <threadhelp/rwlockbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_RANDOM_H_
#include <rtl/random.h>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlock.h>
#endif

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

//#define ENABLE_LOG
//#define ENABLE_THREADDELAY
//#define ENABLE_REQUESTCOUNT

/*----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_LOG
    #define LOG_SETA_START( NA, NID )                                           \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ResetableGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp  );                              \
            sLog.append( ": Thread[ "           );                              \
            sLog.append( NID                    );                              \
            sLog.append( " ] call setA( "       );                              \
            sLog.append( NA                     );                              \
            sLog.append( " )\n"                 );                              \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear().getStr() )        \
        }

    #define LOG_SETA_END( NA, EREASON, NID )                                    \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ResetableGuard aLogGuard( m_aLogMutex );                            \
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
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear().getStr() )        \
        }

    #define LOG_GETA_START( NID )                                               \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ResetableGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp  );                              \
            sLog.append( ": Thread[ "           );                              \
            sLog.append( NID                    );                              \
            sLog.append( " ] call getA()\n"     );                              \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear().getStr() )        \
        }

    #define LOG_GETA_END( NRETURN, EREASON, NID )                               \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ResetableGuard aLogGuard( m_aLogMutex );                            \
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
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear().getStr() )        \
        }

    #define LOG_WORKA_START( NA, NID )                                          \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ResetableGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp  );                              \
            sLog.append( ": Thread[ "           );                              \
            sLog.append( NID                    );                              \
            sLog.append( " ] call workA( "      );                              \
            sLog.append( NA                     );                              \
            sLog.append( " )\n"                 );                              \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear().getStr() )        \
        }

    #define LOG_WORKA_END( NRETURN, EREASON, NID )                              \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ResetableGuard aLogGuard( m_aLogMutex );                            \
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
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear().getStr() )        \
        }

    #define LOG_INITEXCEPTION( SMETHOD, NID )                                   \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ResetableGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp              );                  \
            sLog.append( ": Thread[ "                       );                  \
            sLog.append( NID                                );                  \
            sLog.append( " ] get EInitException from \""    );                  \
            sLog.append( SMETHOD                            );                  \
            sLog.append( "\"\n"                             );                  \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear().getStr() )        \
        }

    #define LOG_CLOSEEXCEPTION( SMETHOD, NID )                                  \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ResetableGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp              );                  \
            sLog.append( ": Thread[ "                       );                  \
            sLog.append( NID                                );                  \
            sLog.append( " ] get ECloseException from \""   );                  \
            sLog.append( SMETHOD                            );                  \
            sLog.append( "\"\n"                             );                  \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear().getStr() )        \
        }

    #define LOG_INIT( NA, NID )                                                 \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ResetableGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp      );                          \
            sLog.append( ": Thread[ "               );                          \
            sLog.append( NID                        );                          \
            sLog.append( " ] initialize me with "   );                          \
            sLog.append( NA                         );                          \
            sLog.append( "\n"                       );                          \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear().getStr() )        \
        }

    #define LOG_CLOSE( NID )                                                    \
        {                                                                       \
            sal_uInt32 nTimeStamp = osl_getGlobalTimer();                       \
            ResetableGuard aLogGuard( m_aLogMutex );                            \
            OStringBuffer sLog(256);                                            \
            sLog.append( (sal_Int32)nTimeStamp  );                              \
            sLog.append( ": Thread[ "           );                              \
            sLog.append( NID                    );                              \
            sLog.append( " ] close me\n"        );                              \
            WRITE_LOGFILE( LOGFILE, sLog.makeStringAndClear().getStr() )        \
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
*//*-*************************************************************************************************************/

class ThreadSafeClass : private FairRWLockBase
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
    :   FairRWLockBase  (   )   /// This struct "export" a public lock member, which can be used by our methods!
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
    LOG_INIT( nA, nThreadID )
    // Best place to initialize internal member is before you call
    // setWorkingMode() ... but if somewhere call this function
    // more then ones ...
    // => check current mode before!
    switch( m_aLock.getWorkingMode() )
    {
        case E_INIT     :   {
                                // OK - This is the first call of init().
                                // Set new value and change mode then.
                                m_nA = nA;
                                m_aLock.setWorkingMode( E_WORK );
                                // After that it's not a good idea to work with internal member
                                // iwthout using the lock!
                            }
                            break;
        case E_WORK     :   {
                                LOG_ERROR( "ThreadSafeClass::init()", "Don't call this method more then ones!" )
                            }
                            break;
        case E_CLOSE    :   LOG_CLOSEEXCEPTION( "init()", nThreadID )
                            break;
    }
}

//_________________________________________________________________________________________________________________
void ThreadSafeClass::close( sal_Int32 nThreadID )
{
    LOG_CLOSE( nThreadID )
    // First get current working mode to prevent us against
    // multiple calls of this method or calls at wrong time!
    switch( m_aLock.getWorkingMode() )
    {
        case E_INIT     :   LOG_INITEXCEPTION( "close()", nThreadID )
                            //throw Exception();
                            break;
        case E_CLOSE    :   LOG_CLOSEEXCEPTION( "close()", nThreadID )
                            //throw Exception();
                            break;
        case E_WORK     :   {
                                // This is the only accepted mode for this method.
                                // Now we should change it to E_CLOSE before we do something
                                // with our internal member!!!
                                // This call will block till all current reader and writer are gone!
                                m_aLock.setWorkingMode( E_CLOSE );
                                // Now we are alone ...
                                // All further calls to this object are rejected.
                                m_nA = 0;
                            }
                            break;
    }
}

//_________________________________________________________________________________________________________________
void ThreadSafeClass::setA( sal_Int32 nA, sal_Int32 nThreadID   )
{
    LOG_SETA_START( nA, nThreadID )

    // Try to set a write lock ... but look for
    // rejected calls. We must react for.
    ERefusalReason  eReason;
    WriteGuard      aGuard( m_aLock, eReason );
    switch( eReason )
    {
        case E_UNINITIALIZED    :   {
                                        // This object isn't initialized => not ready for working!
                                        // We should throw an exception or do nothing.
                                        LOG_INITEXCEPTION( "setA()", nThreadID )
                                        //throw Exception();
                                    }
                                    break;
        case E_CLOSED           :   {
                                        // This object is closed => not ready for working!
                                        // We should throw an exception or do nothing.
                                        LOG_CLOSEEXCEPTION( "setA()", nThreadID )
                                        //throw Exception();
                                    }
                                    break;
        case E_NOREASON         :   {
                                        // This object is ready for working and we have full write access.
                                        // We can work with our member.
                                        m_nA = nA;
                                        #ifdef ENABLE_REQUESTCOUNT
                                        osl_incrementInterlockedCount( &m_nWriteCount );
                                        #endif
                                    }
                                    break;
    }

    LOG_SETA_END( nA, eReason, nThreadID )
}

//_________________________________________________________________________________________________________________
sal_Int32 ThreadSafeClass::getA( sal_Int32 nThreadID )
{
    LOG_GETA_START( nThreadID )

    // Try to set a read lock ... but look for
    // rejected calls. We must react for.
    // Define a default return value for this case.
    sal_Int32       nReturn = 0;
    ERefusalReason  eReason;
    ReadGuard       aGuard( m_aLock, eReason );
    switch( eReason )
    {
        case E_UNINITIALIZED    :   {
                                        LOG_INITEXCEPTION( "getA()", nThreadID )
                                        //throw Exception();
                                    }
                                    break;
        case E_CLOSED           :   {
                                        LOG_CLOSEEXCEPTION( "getA()", nThreadID )
                                        //throw Exception();
                                    }
                                    break;
        case E_NOREASON         :   {
                                        nReturn = m_nA;
                                        #ifdef ENABLE_REQUESTCOUNT
                                        osl_incrementInterlockedCount( &m_nReadCount );
                                        #endif
                                    }
                                    break;
    }

    LOG_GETA_END( nReturn, eReason, nThreadID )

    return nReturn;
}

//_________________________________________________________________________________________________________________
sal_Int32 ThreadSafeClass::workA(   sal_Int32   nA          ,
                                    sal_Int32   nThreadID   )
{
    LOG_WORKA_START( nA, nThreadID )
    // This method test the downgrade-mechanism of used lock implementation!

    // Try to set a write lock first ... but look for
    // rejected calls. We must react for.
    // Define a default return value for this case.
    sal_Int32       nReturn = 0;
    ERefusalReason  eReason;
    WriteGuard      aGuard( m_aLock, eReason );
    switch( eReason )
    {
        case E_UNINITIALIZED    :   {
                                        LOG_INITEXCEPTION( "workA()", nThreadID )
                                        //throw Exception();
                                    }
                                    break;
        case E_CLOSED           :   {
                                        LOG_CLOSEEXCEPTION( "workA()", nThreadID )
                                        //throw Exception();
                                    }
                                    break;
        case E_NOREASON         :   {
                                        // We have write access to our member.
                                        // Set new value.
                                        m_nA = nA;
                                        #ifdef ENABLE_REQUESTCOUNT
                                        osl_incrementInterlockedCount( &m_nWriteCount );
                                        #endif

                                        // Downgrade write access to read access and read the set value again.
                                        // This call can't be rejected - but it can fail!
                                        aGuard.downgrade();
                                        nReturn = m_nA;
                                        #ifdef ENABLE_REQUESTCOUNT
                                        osl_incrementInterlockedCount( &m_nReadCount );
                                        #endif
                                    }
                                    break;
    }

    LOG_WORKA_END( nReturn, eReason, nThreadID )
    return nReturn;
}

/*-****************************************************************************************************//**
    @descr  Every thread instance of these class lopp from 0 up to "nLoops".
            He sleep for a random time and work with given test class "pClass" then.
            We use random values for waiting for better results!
            Otherwise all threads are sychron after first 2,3...5 calls - I think!
*//*-*****************************************************************************************************/

class TestThread : public OThread
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
        if( nA % 5 == 0 )
        {
            nA = m_pClass->workA( nA, m_nThreadID );
        }
        else
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
    for( sal_Int32 nI=1; nI<=nThreadCount; ++nI )
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
    for( nI=1; nI<=nThreadCount; ++nI )
    {
        pThreads[nI].pThread->create();
    }

    // Wait for threads ...
    for( nI=1; nI<=nThreadCount; ++nI )
    {
        pThreads[nI].pCondition->wait();
        delete pThreads[nI].pCondition;
        pThreads[nI].pCondition = NULL;
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
    OStartupInfo    aInfo       ;
    OUString        sArgument   ;
    sal_Int32       nArgument   ;
    sal_Int32       nCount      = aInfo.getCommandArgCount();

    LOG_ASSERT2( nCount!=4 ,"TestApplication::Main()" , "Wrong argument line detected!")

    for( nArgument=0; nArgument<nCount; ++nArgument )
    {
        aInfo.getCommandArg( nArgument, sArgument );
        if( nArgument== 0 ) nTestCount  =sArgument.toInt32();
        if( nArgument== 1 ) nThreadCount=sArgument.toInt32();
        if( nArgument== 2 ) nLoops      =sArgument.toInt32();
        if( nArgument== 3 ) nOwner      =sArgument.toInt32();
    }

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

    WRITE_LOGFILE( STATISTICS_FILE, sBuf.makeStringAndClear().getStr() );
    LOG_ERROR( "TApplication::Main()", "Test finish successful!" )
}
