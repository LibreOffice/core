/*************************************************************************
 *
 *  $RCSfile: officeipcthread.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: cd $ $Date: 2001-08-07 11:25:00 $
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

using namespace vos;
using namespace rtl;
using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;


#define TERMINATION_SEQUENCE "InternalIPC::TerminateThread"
#define TERMINATION_LENGTH 28

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
    Desktop::HandleAppEvent( *((ApplicationEvent*)pEvent) );
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

    // look if we are the first Office: try to open the pipe
    // this should prevent multiple access to the user directory !
    pThread->maPipeIdent = pThread->maPipeIdent + OUString::valueOf( (sal_Int32)aDummy.hashCode() );
    if( pThread->maPipe.create( pThread->maPipeIdent.getStr(), OPipe::TOption_Open, maSecurity ) )
    {
        pThread->maStreamPipe = pThread->maPipe;

        // seems another office is running. pipe arguments to it
        // and self terminate
        ByteString aArguments;
        ULONG nCount = aInfo.getCommandArgCount();
        for( ULONG i=0; i < nCount; i++ )
        {
            aInfo.getCommandArg( i, aDummy );
            aArguments += ByteString( String( aDummy ), osl_getThreadTextEncoding() );
            aArguments += '|';
        }

        pThread->maStreamPipe.write( aArguments.GetBuffer(), aArguments.Len() );
        delete pThread;
        return IPC_STATUS_2ND_OFFICE;
    }

    // seems we are the one and only, so start listening thread
    pThread->maPipe = OPipe( pThread->maPipeIdent, OPipe::TOption_Create, maSecurity );
    if( pThread->maPipe.isValid() )
    {
        pGlobalOfficeIPCThread = pThread;
        pThread->create(); // starts thread
    }

    return IPC_STATUS_OK;
}

void OfficeIPCThread::DisableOfficeIPCThread()
{
    ::osl::ClearableMutexGuard  aGuard( GetMutex() );

    if( pGlobalOfficeIPCThread )
    {
        // send thread a termination message
        // this is done so the subsequent join will not hang
        // because the thread hangs in accept of pipe
        OPipe Pipe( pGlobalOfficeIPCThread->maPipeIdent, OPipe::TOption_Open, maSecurity );
        Pipe.send( TERMINATION_SEQUENCE, TERMINATION_LENGTH );
        Pipe.close();
        // close the pipe so that the streampipe on the other
        // side produces EOF
        aGuard.clear();

        // exit gracefully
        pGlobalOfficeIPCThread->join();
        delete pGlobalOfficeIPCThread;
        pGlobalOfficeIPCThread = 0;
    }
}

OfficeIPCThread::OfficeIPCThread() :
    mbBlockRequests( sal_False ),
    mnPendingRequests( 0 )
{
}

OfficeIPCThread::~OfficeIPCThread()
{
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
            osl::MutexGuard aMutexGuard( GetMutex() );

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

                // is this a termination message ? if so, terminate
                if(( aArguments.CompareTo( TERMINATION_SEQUENCE,
                                          TERMINATION_LENGTH ) == COMPARE_EQUAL ) ||
                    mbBlockRequests )
                    return;

                ::rtl::OUString aOpenList;
                ::rtl::OUString aPrintList;
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

                if( aOpenList.getLength() )
                {
                    // open file(s)

                    ApplicationEvent* pAppEvent =
                        new ApplicationEvent( aEmpty, aEmpty,
                                              APPEVENT_OPEN_STRING, aOpenList );
                    ImplPostForeignAppEvent( pAppEvent );

                    // We are sending a open request to the office. We have to increase our pending request counter!
                    ++mnPendingRequests;
                }

                if ( aPrintList.getLength() )
                {
                    // print file(s)
                    ApplicationEvent* pAppEvent =
                        new ApplicationEvent( aEmpty, aEmpty,
                                              APPEVENT_PRINT_STRING, aPrintList );
                    ImplPostForeignAppEvent( pAppEvent );

                    // We are sending a print request to the office. We have to increase our pending request counter!
                    ++mnPendingRequests;
                }

                if ( aPrintList.getLength() == 0 && aOpenList.getLength() == 0 )
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

}
