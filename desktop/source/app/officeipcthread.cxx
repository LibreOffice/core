/*************************************************************************
 *
 *  $RCSfile: officeipcthread.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: mav $ $Date: 2002-07-25 12:13:30 $
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

#ifdef SOLARIS
#include "officeipcmanager.hxx"
#ifndef _VOS_SECURITY_HXX_
#include <vos/security.hxx>
#endif
#endif

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
#ifndef _RTL_DIGEST_H_
#include <rtl/digest.h>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
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
#ifdef SOLARIS
     PIPEMODE_CONNECTED_TO_PARENT,
#endif
    PIPEMODE_CONNECTED
};

String GetURL_Impl( const String& rName );

extern desktop::CommandLineArgs*    GetCommandLineArgs();

namespace desktop
{

OfficeIPCThread*    OfficeIPCThread::pGlobalOfficeIPCThread = 0;
#ifndef SOLARIS
OSecurity           OfficeIPCThread::maSecurity;
#endif
::osl::Mutex*       OfficeIPCThread::pOfficeIPCThreadMutex = 0;


String CreateMD5FromString( const OUString& aMsg )
{
    // PRE: aStr "file"
    // BACK: Str "ababab....0f" Hexcode String

    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    if ( handle > 0 )
    {
        const sal_uInt8* pData = (const sal_uInt8*)aMsg.getStr();
        sal_uInt32       nSize = ( aMsg.getLength() * sizeof( sal_Unicode ));
        sal_uInt32       nMD5KeyLen = rtl_digest_queryLength( handle );
        sal_uInt8*       pMD5KeyBuffer = new sal_uInt8[ nMD5KeyLen ];

        rtl_digest_init( handle, pData, nSize );
        rtl_digest_update( handle, pData, nSize );
        rtl_digest_get( handle, pMD5KeyBuffer, nMD5KeyLen );
        rtl_digest_destroy( handle );

        // Create hex-value string from the MD5 value to keep the string size minimal
        OUStringBuffer aBuffer( nMD5KeyLen * 2 + 1 );
        for ( sal_uInt32 i = 0; i < nMD5KeyLen; i++ )
            aBuffer.append( (sal_Int32)pMD5KeyBuffer[i], 16 );

        delete [] pMD5KeyBuffer;
        return aBuffer.makeStringAndClear();
    }

    return String();
}

class ProcessEventsClass_Impl
{
public:
    DECL_STATIC_LINK( ProcessEventsClass_Impl, CallEvent, void* pEvent );
    DECL_STATIC_LINK( ProcessEventsClass_Impl, ProcessDocumentsEvent, void* pEvent );
};

IMPL_STATIC_LINK( ProcessEventsClass_Impl, CallEvent, void*, pEvent )
{
    // Application events are processed by the Desktop::HandleAppEvent implementation.
    Desktop::HandleAppEvent( *((ApplicationEvent*)pEvent) );
    delete (ApplicationEvent*)pEvent;
    return 0;
}

IMPL_STATIC_LINK( ProcessEventsClass_Impl, ProcessDocumentsEvent, void*, pEvent )
{
    // Documents requests are processed by the OfficeIPCThread implementation
    ProcessDocumentsRequest* pDocsRequest = (ProcessDocumentsRequest*)pEvent;

    if ( pDocsRequest )
    {
        OfficeIPCThread::ExecuteCmdLineRequests( *pDocsRequest );
        delete pDocsRequest;
    }
    return 0;
}

void ImplPostForeignAppEvent( ApplicationEvent* pEvent )
{
    Application::PostUserEvent( STATIC_LINK( NULL, ProcessEventsClass_Impl, CallEvent ), pEvent );
}

void ImplPostProcessDocumentsEvent( ProcessDocumentsRequest* pEvent )
{
    Application::PostUserEvent( STATIC_LINK( NULL, ProcessEventsClass_Impl, ProcessDocumentsEvent ), pEvent );
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

#ifdef SOLARIS
OfficeIPCThread::Status OfficeIPCThread::EnableOfficeIPCThread(
                                            sal_Bool useParent )
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

    pThread->maPipeIdent = OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "SingleOfficeIPC_" ) );

    // The name of the named pipe is created with the hashcode of the user
    // installation directory (without /user). We have to retrieve
    // this information from a unotools implementation.
    ::utl::Bootstrap::PathStatus aLocateResult =
        ::utl::Bootstrap::locateUserInstallation( aUserInstallPath );
    if ( aLocateResult == ::utl::Bootstrap::PATH_EXISTS )
        aDummy = aUserInstallPath;
    else
    {
        delete pThread;
        return IPC_STATUS_BOOTSTRAP_ERROR;
    }

    // Try to determine if we are the first office or not! This should prevent
    // multiple access to the user directory !
    // First we try to create our pipe if this fails we try to connect. We have
    // to do this in a loop because the the other office can crash or shutdown
    // between createPipe and connectPipe!!
    pThread->maPipeIdent = pThread->maPipeIdent +
                           OUString::valueOf( (sal_Int32)aDummy.hashCode() );

    PipeMode nPipeMode = PIPEMODE_DONTKNOW;
    // Warning: Temporarily removed loop below until I have a better mechanism
    // Try to create pipe
    if ( pThread->maPipe.create(
            pThread->maPipeIdent.getStr(),
            OPipe::TOption_Open,
            OSecurity() ) )
    {
        nPipeMode = PIPEMODE_CONNECTED;
    }
    else if ( useParent && OfficeIPCManager::ParentExists() )
    {
        nPipeMode = PIPEMODE_CONNECTED_TO_PARENT;
    }
    else if( pThread->maPipe.create(
                pThread->maPipeIdent.getStr(),
                OPipe::TOption_Create,
                OSecurity() ) )
    {
        nPipeMode = PIPEMODE_CREATED;
    }

    if ( nPipeMode == PIPEMODE_CREATED )
    {
        // Seems we are the one and only, so start listening thread
        pGlobalOfficeIPCThread = pThread;
        pThread->create(); // starts thread
    }
    else
    {
        // Seems another office is running. Pipe arguments to it and self
        // terminate
        pThread->maStreamPipe = pThread->maPipe;

        sal_Bool bWaitBeforeClose = sal_False;
        ByteString aArguments;
        ULONG nCount = aInfo.getCommandArgCount();

        if ( nCount > 0 )
        {
            sal_Bool    bPrintTo = sal_False;
            OUString    aPrintToCmd( RTL_CONSTASCII_USTRINGPARAM( "-pt" ));
            for( ULONG i=0; i < nCount; i++ )
            {
                aInfo.getCommandArg( i, aDummy );
                // Make absolute pathes from relative ones!
                // It's neccessary to use current working directory of THESE
                // office instance and not of
                // currently running once, which get these information by using
                // pipe.
                // Otherwhise relativ pathes are not right for his environment ...
                if( aDummy.indexOf('-',0) != 0 )
                {
                    bWaitBeforeClose = sal_True;
                    if ( !bPrintTo )
                        aDummy = GetURL_Impl( aDummy );
                    bPrintTo = sal_False;
                }
                else if ( aDummy == OfficeIPCManager::GetDisplayArgument() )
                {
                    i ++;
                    nCount -= 2;
                    continue;
                }
                else if ( aDummy == OfficeIPCManager::GetMasterArgument() )
                {
                    continue;
                }
                else
                {
                    if ( aDummy.equalsIgnoreAsciiCase( aPrintToCmd ))
                        bPrintTo = sal_True;
                    else
                        bPrintTo = sal_False;
                }

                aArguments += ByteString( String( aDummy ),
                                          osl_getThreadTextEncoding() );
                aArguments += '|';
            }
        }

        if ( nCount == 0 )
        {
            // Use default argument so the first office can distinguish between
            // a real second
            // office and another program that check the existence of the the
            // pipe!!

            aArguments += ByteString( "-show" );
        }

        if ( nPipeMode == PIPEMODE_CONNECTED_TO_PARENT )
        {
            OfficeIPCManager::SendParentRequest( aArguments.GetBuffer(),
                                                 aArguments.Len() );
        }
        else
        {
            pThread->maStreamPipe.write( aArguments.GetBuffer(),
                                         aArguments.Len() );
        }
        delete pThread;

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
        return IPC_STATUS_2ND_OFFICE;
    }
    return IPC_STATUS_OK;
}
#else // SOLARIS
OfficeIPCThread::Status OfficeIPCThread::EnableOfficeIPCThread()
{
    ::osl::MutexGuard   aGuard( GetMutex() );

    if( pGlobalOfficeIPCThread )
        return IPC_STATUS_OK;

    ::rtl::OUString aUserInstallPath;
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
    OUString aUserInstallPathHashCode = CreateMD5FromString( aDummy );

    // Check result to create a hash code from the user install path
    if ( aUserInstallPathHashCode.getLength() == 0 )
        return IPC_STATUS_BOOTSTRAP_ERROR; // Something completely broken, we cannot create a valid hash code!

    pThread->maPipeIdent = pThread->maPipeIdent + aUserInstallPathHashCode;

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
            sal_Bool    bPrintTo = sal_False;
            OUString    aPrintToCmd( RTL_CONSTASCII_USTRINGPARAM( "-pt" ));
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
                    if ( !bPrintTo )
                        aDummy = GetURL_Impl( aDummy );
                    bPrintTo = sal_False;
                }
                else
                {
                    if ( aDummy.equalsIgnoreAsciiCase( aPrintToCmd ))
                        bPrintTo = sal_True;
                    else
                        bPrintTo = sal_False;
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
#endif // SOLARIS


void OfficeIPCThread::DisableOfficeIPCThread()
{
    osl::ClearableMutexGuard aMutex( GetMutex() );

    if( pGlobalOfficeIPCThread && !( pGlobalOfficeIPCThread->mbShutdownInProgress ))
    {
        pGlobalOfficeIPCThread->mbShutdownInProgress = sal_True;

        // send thread a termination message
        // this is done so the subsequent join will not hang
        // because the thread hangs in accept of pipe
#ifdef SOLARIS
        OPipe Pipe( pGlobalOfficeIPCThread->maPipeIdent, OPipe::TOption_Open, OSecurity() );
#else
        OPipe Pipe( pGlobalOfficeIPCThread->maPipeIdent, OPipe::TOption_Open, maSecurity );
#endif
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

                String              aEmpty;
                CommandLineArgs     aCmdLineArgs( OUString( aArguments.GetBuffer(), aArguments.Len(), gsl_getSystemTextEncoding() ));
                CommandLineArgs*    pCurrentCmdLineArgs = GetCommandLineArgs();

                if ( aCmdLineArgs.IsQuickstart() )
                {
                    // we have to use application event, because we have to start quickstart service in main thread!!
                    ApplicationEvent* pAppEvent =
                        new ApplicationEvent( aEmpty, aEmpty,
                                              "QUICKSTART", aEmpty );
                    ImplPostForeignAppEvent( pAppEvent );
                }

                sal_Bool bDocRequestSent = sal_False;
                ProcessDocumentsRequest* pRequest = new ProcessDocumentsRequest;

                // Print requests are not dependent on the -invisible cmdline argument as they are
                // loaded with the "hidden" flag! So they are always checked.
                bDocRequestSent |= aCmdLineArgs.GetPrintList( pRequest->aPrintList );
                bDocRequestSent |= ( aCmdLineArgs.GetPrintToList( pRequest->aPrintToList ) &&
                                     aCmdLineArgs.GetPrinterName( pRequest->aPrinterName )      );

                if ( !pCurrentCmdLineArgs->IsInvisible() )
                {
                    // Read cmdline args that can open/create documents. As they would open a window
                    // they are only allowed if the "-invisible" is currently not used!
                    bDocRequestSent |= aCmdLineArgs.GetOpenList( pRequest->aOpenList );
                    bDocRequestSent |= aCmdLineArgs.GetForceOpenList( pRequest->aForceOpenList );
                    bDocRequestSent |= aCmdLineArgs.GetForceNewList( pRequest->aForceNewList );
                }

                if ( bDocRequestSent )
                 {
                    // Send requests to dispatch watcher if we have at least one. The receiver
                    // is responsible to delete the request after processing it.
                    ImplPostProcessDocumentsEvent( pRequest );
                }
                else
                {
                    // delete not used request again
                    delete pRequest;
                }

                if (( aArguments.CompareTo( SHOW_SEQUENCE, SHOW_LENGTH ) == COMPARE_EQUAL ) ||
                      !bDocRequestSent )
                {
                    // no document was sent, just bring Office to front
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

void AddToDispatchList(
    DispatchWatcher::DispatchList& rDispatchList,
    const OUString& aRequestList,
    DispatchWatcher::RequestType nType,
    const OUString& aParam )
{
    if ( aRequestList.getLength() > 0 )
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = aRequestList.getToken( 0, APPEVENT_PARAM_DELIMITER, nIndex );
            if ( aToken.getLength() > 0 )
                rDispatchList.push_back(
                    DispatchWatcher::DispatchRequest( nType, aToken, aParam ));
        }
        while ( nIndex >= 0 );
    }
}

void OfficeIPCThread::ExecuteCmdLineRequests( const ProcessDocumentsRequest& aRequest )
{
    ::rtl::OUString                 aEmpty;
    DispatchWatcher::DispatchList   aDispatchList;

    // Create dispatch list for dispatch watcher
    AddToDispatchList( aDispatchList, aRequest.aOpenList, DispatchWatcher::REQUEST_OPEN, aEmpty );
    AddToDispatchList( aDispatchList, aRequest.aPrintList, DispatchWatcher::REQUEST_PRINT, aEmpty );
    AddToDispatchList( aDispatchList, aRequest.aPrintToList, DispatchWatcher::REQUEST_PRINTTO, aRequest.aPrinterName );
    AddToDispatchList( aDispatchList, aRequest.aForceOpenList, DispatchWatcher::REQUEST_FORCEOPEN, aEmpty );
    AddToDispatchList( aDispatchList, aRequest.aForceNewList, DispatchWatcher::REQUEST_FORCENEW, aEmpty );

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

        // Execute dispatch requests
        pGlobalOfficeIPCThread->mpDispatchWatcher->executeDispatchRequests( aDispatchList );
    }
}

}
