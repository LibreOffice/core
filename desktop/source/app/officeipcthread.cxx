/*************************************************************************
 *
 *  $RCSfile: officeipcthread.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: cd $ $Date: 2001-12-04 16:05:32 $
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

#include "app.hxx"
#include "officeipcthread.hxx"
#include "cmdlineargs.hxx"
#include "dispatchwatcher.hxx"
#include <stdio.h>

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif

using namespace vos;
using namespace rtl;
using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;


#define TERMINATION_SEQUENCE "InternalIPC::TerminateThread"
#define TERMINATION_LENGTH 28

#define SHOW_SEQUENCE   "-show"
#define SHOW_LENGTH     5

// Type of pipe we use
enum PipeMode
{
    PIPEMODE_DONTKNOW,
    PIPEMODE_CREATED,
    PIPEMODE_CONNECTED
};

String GetURL_Impl( const String& rName );

namespace desktop
{

OfficeIPCThread*    OfficeIPCThread::pGlobalOfficeIPCThread = 0;
OSecurity           OfficeIPCThread::maSecurity;
::osl::Mutex*       OfficeIPCThread::pOfficeIPCThreadMutex = 0;

class ImplForeignAppEventClass
{
public:
    DECL_STATIC_LINK( ImplForeignAppEventClass, CallEvent, void* pEvent );
};

void HandleAppEvent( const ApplicationEvent& rAppEvent );
IMPL_STATIC_LINK( ImplForeignAppEventClass, CallEvent, void*, pEvent )
{
    // Application events are processed by the Desktop::HandleAppEvent implementation.
    ApplicationEvent* pAppEvent = (ApplicationEvent*)pEvent;

    if ( pAppEvent->GetEvent().CompareTo( "OPENPRINTCMDLINE" ) == COMPARE_EQUAL )
    {
        // Special application event to execute OPEN, PRINT and PRINTTO requests from the command line
        ::rtl::OUString aPrintToList;
        ::rtl::OUString aPrinterName;
        ::rtl::OUString aOpenList( pAppEvent->GetSenderAppName() );
        ::rtl::OUString aPrintList( pAppEvent->GetData() );
        ApplicationAddress aAppAddress;

        aAppAddress = pAppEvent->GetAppAddress();
        aPrintToList = aAppAddress.GetHost();
        aPrinterName = aAppAddress.GetDisplay();

        OfficeIPCThread* pIPCThread = OfficeIPCThread::GetOfficeIPCThread();
        if ( pIPCThread )
            pIPCThread->ExecuteCmdLineRequests( aOpenList, aPrintList, aPrintToList, aPrinterName );
    }
    else
    {
        Desktop::HandleAppEvent( *((ApplicationEvent*)pEvent) );
    }

    delete (ApplicationEvent*)pEvent;
    return 0;
}

void ImplPostForeignAppEvent( ApplicationEvent* pEvent )
{
    Application::PostUserEvent( STATIC_LINK( NULL, ImplForeignAppEventClass, CallEvent ), pEvent );
}

OSignalHandler::TSignalAction SAL_CALL SalMainPipeExchangeSignalHandler::signal(TSignalInfo *pInfo)
{
    if( pInfo->Signal == osl_Signal_Terminate )
        OfficeIPCThread::DisableOfficeIPCThread();
    return (TAction_CallNextHandler);
}

// ----------------------------------------------------------------------------

// The OfficeIPCThreadController implementation is a bookkeeper for all pending requests
// that were created by the OfficeIPCThread. The requests are waiting to be processed by
// our framework loadComponentFromURL function (e.g. open/print request).
// During shutdown the framework is asking OfficeIPCThreadController about pending requests.
// If there are pending requests framework has to stop the shutdown process. It is waiting
// for these requests because framework is not able to handle shutdown and open a document
// concurrently.


// XServiceInfo
OUString SAL_CALL OfficeIPCThreadController::getImplementationName()
throw ( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.OfficeIPCThreadController" ));
}

sal_Bool SAL_CALL OfficeIPCThreadController::supportsService( const OUString& ServiceName )
throw ( RuntimeException )
{
    return sal_False;
}

Sequence< OUString > SAL_CALL OfficeIPCThreadController::getSupportedServiceNames()
throw ( RuntimeException )
{
    Sequence< OUString > aSeq( 0 );
    return aSeq;
}

// XEventListener
void SAL_CALL OfficeIPCThreadController::disposing( const EventObject& Source )
throw( RuntimeException )
{
}

// XTerminateListener
void SAL_CALL OfficeIPCThreadController::queryTermination( const EventObject& aEvent )
throw( TerminationVetoException, RuntimeException )
{
    // Desktop ask about pending request through our office ipc pipe. We have to
    // be sure that no pending request is waiting because framework is not able to
    // handle shutdown and open a document concurrently.

    if ( OfficeIPCThread::AreRequestsPending() )
        throw TerminationVetoException();
    else
        OfficeIPCThread::BlockAllRequests();
}

void SAL_CALL OfficeIPCThreadController::notifyTermination( const EventObject& aEvent )
throw( RuntimeException )
{
}

// ----------------------------------------------------------------------------

::osl::Mutex&   OfficeIPCThread::GetMutex()
{
    // Get or create our mutex for thread-saftey
    if ( !pOfficeIPCThreadMutex )
    {
        ::osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pOfficeIPCThreadMutex )
            pOfficeIPCThreadMutex = new osl::Mutex;
    }

    return *pOfficeIPCThreadMutex;
}

OfficeIPCThread* OfficeIPCThread::GetOfficeIPCThread()
{
    // Return the one and only OfficeIPCThread pointer
    ::osl::MutexGuard   aGuard( GetMutex() );
    return pGlobalOfficeIPCThread;
}

void OfficeIPCThread::BlockAllRequests()
{
    // We have the order to block all incoming requests. Framework
    // wants to shutdown and we have to make sure that no loading/printing
    // requests are executed anymore.
    ::osl::MutexGuard   aGuard( GetMutex() );

    if ( pGlobalOfficeIPCThread )
        pGlobalOfficeIPCThread->mbBlockRequests = sal_True;
}

sal_Bool OfficeIPCThread::AreRequestsPending()
{
    // Give info about pending requests
    ::osl::MutexGuard   aGuard( GetMutex() );
    if ( pGlobalOfficeIPCThread )
        return ( pGlobalOfficeIPCThread->mnPendingRequests > 0 );
    else
        return sal_False;
}

void OfficeIPCThread::RequestsCompleted( int nCount )
{
    // Remove nCount pending requests from our internal counter
    ::osl::MutexGuard   aGuard( GetMutex() );
    if ( pGlobalOfficeIPCThread )
    {
        if ( pGlobalOfficeIPCThread->mnPendingRequests > 0 )
            pGlobalOfficeIPCThread->mnPendingRequests -= nCount;
    }
}

OfficeIPCThread::Status OfficeIPCThread::EnableOfficeIPCThread()
{
    ::osl::MutexGuard   aGuard( GetMutex() );

    if( pGlobalOfficeIPCThread )
        return IPC_STATUS_OK;

    ::rtl::OUString aOfficeInstallPath;
    ::rtl::OUString aUserInstallPath;
    ::rtl::OUString aLastIniFile;
    ::rtl::OUString aDummy;

    ::vos::OStartupInfo aInfo;
    OfficeIPCThread* pThread = new OfficeIPCThread;

    pThread->maPipeIdent = OUString( RTL_CONSTASCII_USTRINGPARAM( "SingleOfficeIPC_" ) );

    // The name of the named pipe is created with the hashcode of the user installation directory (without /user). We have to retrieve
    // this information from a unotools implementation.
    ::utl::Bootstrap::PathStatus aLocateResult = ::utl::Bootstrap::locateUserInstallation( aUserInstallPath );
    if ( aLocateResult == ::utl::Bootstrap::PATH_EXISTS )
        aDummy = aUserInstallPath;
    else
    {
        delete pThread;
        return IPC_STATUS_BOOTSTRAP_ERROR;
    }

    // Try to  determine if we are the first office or not! This should prevent multiple
    // access to the user directory !
    // First we try to create our pipe if this fails we try to connect. We have to do this
    // in a loop because the the other office can crash or shutdown between createPipe
    // and connectPipe!!
    pThread->maPipeIdent = pThread->maPipeIdent + OUString::valueOf( (sal_Int32)aDummy.hashCode() );

    PipeMode nPipeMode = PIPEMODE_DONTKNOW;
    do
    {
        // Try to create pipe
        if ( pThread->maPipe.create( pThread->maPipeIdent.getStr(), OPipe::TOption_Create, maSecurity ))
        {
            // Pipe created
            nPipeMode = PIPEMODE_CREATED;
        }
        else if( pThread->maPipe.create( pThread->maPipeIdent.getStr(), OPipe::TOption_Open, maSecurity )) // Creation not successfull, now we try to connect
        {
            // Pipe connected to first office
            nPipeMode = PIPEMODE_CONNECTED;
        }
        else
        {
            // Wait for second office to be ready
            TimeValue aTimeValue;
            aTimeValue.Seconds = 0;
            aTimeValue.Nanosec = 10000000; // 10ms
            osl::Thread::wait( aTimeValue );
        }

    } while ( nPipeMode == PIPEMODE_DONTKNOW );

    if ( nPipeMode == PIPEMODE_CREATED )
    {
        // Seems we are the one and only, so start listening thread
        pGlobalOfficeIPCThread = pThread;
        pThread->create(); // starts thread
    }
    else
    {
        // Seems another office is running. Pipe arguments to it and self terminate
        pThread->maStreamPipe = pThread->maPipe;

        sal_Bool bWaitBeforeClose = sal_False;
        ByteString aArguments;
        ULONG nCount = aInfo.getCommandArgCount();

        if ( nCount == 0 )
        {
            // Use default argument so the first office can distinguish between a real second
            // office and another program that check the existence of the the pipe!!

            aArguments += ByteString( "-show" );
        }
        else
        {
            for( ULONG i=0; i < nCount; i++ )
            {
                aInfo.getCommandArg( i, aDummy );
                // Make absolute pathes from relative ones!
                // It's neccessary to use current working directory of THESE office instance and not of
                // currently running once, which get these information by using pipe.
                // Otherwhise relativ pathes are not right for his environment ...
                if( aDummy.indexOf('-',0) != 0 )
                {
                    bWaitBeforeClose = sal_True;
                    aDummy = GetURL_Impl( aDummy );
                }
                aArguments += ByteString( String( aDummy ), osl_getThreadTextEncoding() );
                aArguments += '|';
            }
        }

        pThread->maStreamPipe.write( aArguments.GetBuffer(), aArguments.Len() );
        delete pThread;

#ifdef UNX
        if ( bWaitBeforeClose )
        {
            // Fix for bug #95361#
            // We are waiting before office shutdown itself. Netscape
            // deletes temporary files after the responsible application
            // exited. The running office must have time to open the file
            // before Netscape can delete it!!
            // We have to find a better way to handle this kind of problem
            // in the future.
            TimeValue aTimeValue;
            aTimeValue.Seconds = 5;
            aTimeValue.Nanosec = 0; // 5sec
            osl::Thread::wait( aTimeValue );
        }
#endif

        return IPC_STATUS_2ND_OFFICE;
    }

    return IPC_STATUS_OK;
}

void OfficeIPCThread::DisableOfficeIPCThread()
{
    osl::ClearableMutexGuard aMutex( GetMutex() );

    if( pGlobalOfficeIPCThread && !( pGlobalOfficeIPCThread->mbShutdownInProgress ))
    {
        pGlobalOfficeIPCThread->mbShutdownInProgress = sal_True;

        // send thread a termination message
        // this is done so the subsequent join will not hang
        // because the thread hangs in accept of pipe
        OPipe Pipe( pGlobalOfficeIPCThread->maPipeIdent, OPipe::TOption_Open, maSecurity );
        Pipe.send( TERMINATION_SEQUENCE, TERMINATION_LENGTH );

        // close the pipe so that the streampipe on the other
        // side produces EOF
        Pipe.close();

        // release mutex to avoid deadlocks
        aMutex.clear();

        // exit gracefully and join
        pGlobalOfficeIPCThread->join();

        {
            // acquire mutex again to delete and reset global pointer threadsafe!
            osl::MutexGuard aGuard( GetMutex() );
            delete pGlobalOfficeIPCThread;
            pGlobalOfficeIPCThread = 0;
        }
    }
}

OfficeIPCThread::OfficeIPCThread() :
    mbBlockRequests( sal_False ),
    mnPendingRequests( 0 ),
    mpDispatchWatcher( 0 ),
    mbShutdownInProgress( sal_False )
{
}

OfficeIPCThread::~OfficeIPCThread()
{
    ::osl::ClearableMutexGuard  aGuard( GetMutex() );

    if ( mpDispatchWatcher )
        mpDispatchWatcher->release();
    maPipe.close();
    maStreamPipe.close();
    pGlobalOfficeIPCThread = 0;
}

void SAL_CALL OfficeIPCThread::run()
{
    while (schedule())
    {
        OPipe::TPipeError
            nError = maPipe.accept( maStreamPipe );

        {
            osl::ClearableMutexGuard aGuard( GetMutex() );

            if( nError == OStreamPipe::E_None )
            {
                ByteString aArguments;
                char pBuf[ 2049 ];
                int nBytes;
                do
                {
                    nBytes = maStreamPipe.read( pBuf, 2048 );
                    pBuf[ nBytes ] = 0;
                    aArguments += pBuf;
                } while( ! maStreamPipe.isEof() || nBytes > 0 );
                maStreamPipe.close();

                // #90717# Is this a lookup message from another application? if so, ignore
                if ( aArguments.Len() == 0 )
                    continue;

                // is this a termination message ? if so, terminate
                if(( aArguments.CompareTo( TERMINATION_SEQUENCE,
                                          TERMINATION_LENGTH ) == COMPARE_EQUAL ) ||
                    mbBlockRequests )
                    return;

                ::rtl::OUString aOpenList;
                ::rtl::OUString aPrintList;
                ::rtl::OUString aPrintToList;
                ::rtl::OUString aPrinter;
                String          aEmpty;
                CommandLineArgs aCmdLineArgs( OUString( aArguments.GetBuffer(), aArguments.Len(), gsl_getSystemTextEncoding() ));

                if ( aCmdLineArgs.IsQuickstart() )
                {
                    // we have to use application event, because we have to start quickstart service in main thread!!
                    ApplicationEvent* pAppEvent =
                        new ApplicationEvent( aEmpty, aEmpty,
                                              "QUICKSTART", aEmpty );
                    ImplPostForeignAppEvent( pAppEvent );
                }

                aCmdLineArgs.GetOpenList( aOpenList );
                aCmdLineArgs.GetPrintList( aPrintList );
                aCmdLineArgs.GetPrintToList( aPrintToList );

                // send requests to dispatch watcher
                if ( aOpenList.getLength() > 0 ||
                     aPrintList.getLength() > 0 ||
                     ( aPrintToList.getLength() > 0 && aPrinter.getLength() > 0 ))
                 {
                    // use application address to transport print to data
                    ApplicationAddress aAppAddress( aPrintToList, aPrinter, aEmpty );

                    ApplicationEvent* pAppEvent =
                        new ApplicationEvent( aOpenList, aEmpty, "OPENPRINTCMDLINE", aPrintList );

                    ImplPostForeignAppEvent( pAppEvent );
                }

                if (( aArguments.CompareTo( SHOW_SEQUENCE, SHOW_LENGTH ) == COMPARE_EQUAL ) ||
                    ( aPrintList.getLength() == 0 && aOpenList.getLength() == 0 ) )
                {
                    // no document was send, just bring Office to front
                    ApplicationEvent* pAppEvent =
                        new ApplicationEvent( aEmpty, aEmpty,
                                              "APPEAR", aEmpty );
                    ImplPostForeignAppEvent( pAppEvent );
                }
            }
            else
            {
#if defined DEBUG || defined DBG_UTIL
                fprintf( stderr, "Error on accept: %d\n", (int)nError );
#endif
                TimeValue tval;
                tval.Seconds = 1;
                tval.Nanosec = 0;
                sleep( tval );
            }
        }
    }
}

void OfficeIPCThread::ExecuteCmdLineRequests(
    const ::rtl::OUString& aOpenList,
    const ::rtl::OUString& aPrintList,
    const ::rtl::OUString& aPrintToList,
    const ::rtl::OUString& aPrinterName )
{
    ::rtl::OUString                 aEmpty;
    DispatchWatcher::DispatchList   aDispatchList;

    if ( aOpenList.getLength() > 0 )
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = aOpenList.getToken( 0, APPEVENT_PARAM_DELIMITER, nIndex );
            if ( aToken.getLength() > 0 )
                aDispatchList.push_back(
                    DispatchWatcher::DispatchRequest( DispatchWatcher::REQUEST_OPEN, aToken, aEmpty ));
        }
        while ( nIndex >= 0 );
    }

    if ( aPrintList.getLength() > 0 )
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = aPrintList.getToken( 0, APPEVENT_PARAM_DELIMITER, nIndex );
            if ( aToken.getLength() > 0 )
                aDispatchList.push_back(
                    DispatchWatcher::DispatchRequest( DispatchWatcher::REQUEST_PRINT, aToken, aEmpty ));
        }
        while ( nIndex >= 0 );
    }

    if ( aPrintToList.getLength() > 0 && aPrinterName.getLength() > 0 )
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = aPrintToList.getToken( 0, APPEVENT_PARAM_DELIMITER, nIndex );
            if ( aToken.getLength() > 0 )
                aDispatchList.push_back(
                    DispatchWatcher::DispatchRequest( DispatchWatcher::REQUEST_PRINTTO, aToken, aPrinterName ));
        }
        while ( nIndex >= 0 );
    }

    osl::ClearableMutexGuard aGuard( GetMutex() );

    if ( pGlobalOfficeIPCThread )
    {
        pGlobalOfficeIPCThread->mnPendingRequests += aDispatchList.size();
        if ( !pGlobalOfficeIPCThread->mpDispatchWatcher )
        {
            pGlobalOfficeIPCThread->mpDispatchWatcher = DispatchWatcher::GetDispatchWatcher();
            pGlobalOfficeIPCThread->mpDispatchWatcher->acquire();
        }

        aGuard.clear();
        pGlobalOfficeIPCThread->mpDispatchWatcher->executeDispatchRequests( aDispatchList );
    }
}

}
