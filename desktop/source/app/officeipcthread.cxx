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
#include "precompiled_desktop.hxx"

#include "app.hxx"
#include "officeipcthread.hxx"
#include "cmdlineargs.hxx"
#include "dispatchwatcher.hxx"
#include <memory>
#include <stdio.h>
#include <osl/process.h>
#include <unotools/bootstrap.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <unotools/configmgr.hxx>
#include <osl/thread.hxx>
#include <rtl/digest.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <osl/conditn.hxx>
#include <unotools/moduleoptions.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include "rtl/process.h"
#include "tools/getprocessworkingdir.hxx"

using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

const char  *OfficeIPCThread::sc_aTerminationSequence = "InternalIPC::TerminateThread";
const int OfficeIPCThread::sc_nTSeqLength = 28;
const char  *OfficeIPCThread::sc_aShowSequence = "-tofront";
const int OfficeIPCThread::sc_nShSeqLength = 5;
const char  *OfficeIPCThread::sc_aConfirmationSequence = "InternalIPC::ProcessingDone";
const int OfficeIPCThread::sc_nCSeqLength = 27;

namespace { static char const ARGUMENT_PREFIX[] = "InternalIPC::Arguments"; }

// Type of pipe we use
enum PipeMode
{
    PIPEMODE_DONTKNOW,
    PIPEMODE_CREATED,
    PIPEMODE_CONNECTED
};

namespace desktop
{

namespace {

class Parser: public CommandLineArgs::Supplier {
public:
    explicit Parser(rtl::OString const & input): m_input(input) {
        if (!m_input.match(ARGUMENT_PREFIX) ||
            m_input.getLength() == RTL_CONSTASCII_LENGTH(ARGUMENT_PREFIX))
        {
            throw CommandLineArgs::Supplier::Exception();
        }
        m_index = RTL_CONSTASCII_LENGTH(ARGUMENT_PREFIX);
        switch (m_input[m_index++]) {
        case '0':
            break;
        case '1':
            {
                rtl::OUString url;
                if (!next(&url, false)) {
                    throw CommandLineArgs::Supplier::Exception();
                }
                m_cwdUrl.reset(url);
                break;
            }
        case '2':
            {
                rtl::OUString path;
                if (!next(&path, false)) {
                    throw CommandLineArgs::Supplier::Exception();
                }
                rtl::OUString url;
                if (osl::FileBase::getFileURLFromSystemPath(path, url) ==
                    osl::FileBase::E_None)
                {
                    m_cwdUrl.reset(url);
                }
                break;
            }
        default:
            throw CommandLineArgs::Supplier::Exception();
        }
    }

    virtual ~Parser() {}

    virtual boost::optional< rtl::OUString > getCwdUrl() { return m_cwdUrl; }

    virtual bool next(rtl::OUString * argument) { return next(argument, true); }

private:
    virtual bool next(rtl::OUString * argument, bool prefix) {
        OSL_ASSERT(argument != NULL);
        if (m_index < m_input.getLength()) {
            if (prefix) {
                if (m_input[m_index] != ',') {
                    throw CommandLineArgs::Supplier::Exception();
                }
                ++m_index;
            }
            rtl::OStringBuffer b;
            while (m_index < m_input.getLength()) {
                char c = m_input[m_index];
                if (c == ',') {
                    break;
                }
                ++m_index;
                if (c == '\\') {
                    if (m_index < m_input.getLength()) {
                        c = m_input[m_index++];
                        switch (c) {
                        case '0':
                            c = '\0';
                            break;
                        case ',':
                        case '\\':
                            break;
                        default:
                            throw CommandLineArgs::Supplier::Exception();
                        }
                    } else {
                        throw CommandLineArgs::Supplier::Exception();
                    }
                }
                b.append(c);
            }
            rtl::OString b2(b.makeStringAndClear());
            if (!rtl_convertStringToUString(
                    &argument->pData, b2.getStr(), b2.getLength(),
                    RTL_TEXTENCODING_UTF8,
                    (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
                     RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
                     RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
            {
                throw CommandLineArgs::Supplier::Exception();
            }
            return true;
        } else {
            return false;
        }
    }

    boost::optional< rtl::OUString > m_cwdUrl;
    rtl::OString m_input;
    sal_Int32 m_index;
};

bool addArgument(
    ByteString * arguments, char prefix, rtl::OUString const & argument)
{
    rtl::OString utf8;
    if (!argument.convertToString(
            &utf8, RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        return false;
    }
    *arguments += prefix;
    for (sal_Int32 i = 0; i < utf8.getLength(); ++i) {
        char c = utf8[i];
        switch (c) {
        case '\0':
            *arguments += "\\0";
            break;
        case ',':
            *arguments += "\\,";
            break;
        case '\\':
            *arguments += "\\\\";
            break;
        default:
            *arguments += c;
            break;
        }
    }
    return true;
}

}

OfficeIPCThread*    OfficeIPCThread::pGlobalOfficeIPCThread = 0;
    namespace { struct Security : public rtl::Static<osl::Security, Security> {}; }
::osl::Mutex*       OfficeIPCThread::pOfficeIPCThreadMutex = 0;

// Turns a string in aMsg such as file:///home/foo/.libreoffice/3
// Into a hex string of well known length ff132a86...
String CreateMD5FromString( const OUString& aMsg )
{
#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
    fprintf (stderr, "create md5 frim '%s'\n",
             (const sal_Char *)rtl::OUStringToOString (aMsg, RTL_TEXTENCODING_UTF8));
#endif

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

IMPL_STATIC_LINK_NOINSTANCE( ProcessEventsClass_Impl, CallEvent, void*, pEvent )
{
    // Application events are processed by the Desktop::HandleAppEvent implementation.
    Desktop::HandleAppEvent( *((ApplicationEvent*)pEvent) );
    delete (ApplicationEvent*)pEvent;
    return 0;
}

IMPL_STATIC_LINK_NOINSTANCE( ProcessEventsClass_Impl, ProcessDocumentsEvent, void*, pEvent )
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

oslSignalAction SAL_CALL SalMainPipeExchangeSignal_impl(void* /*pData*/, oslSignalInfo* pInfo)
{
    if( pInfo->Signal == osl_Signal_Terminate )
        OfficeIPCThread::DisableOfficeIPCThread();
    return osl_Signal_ActCallNextHdl;
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

sal_Bool SAL_CALL OfficeIPCThreadController::supportsService( const OUString& )
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
void SAL_CALL OfficeIPCThreadController::disposing( const EventObject& )
throw( RuntimeException )
{
}

// XTerminateListener
void SAL_CALL OfficeIPCThreadController::queryTermination( const EventObject& )
throw( TerminationVetoException, RuntimeException )
{
    // Desktop ask about pending request through our office ipc pipe. We have to
    // be sure that no pending request is waiting because framework is not able to
    // handle shutdown and open a document concurrently.

    if ( OfficeIPCThread::AreRequestsPending() )
        throw TerminationVetoException();
    else
        OfficeIPCThread::SetDowning();
}

void SAL_CALL OfficeIPCThreadController::notifyTermination( const EventObject& )
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

void OfficeIPCThread::SetDowning()
{
    // We have the order to block all incoming requests. Framework
    // wants to shutdown and we have to make sure that no loading/printing
    // requests are executed anymore.
    ::osl::MutexGuard   aGuard( GetMutex() );

    if ( pGlobalOfficeIPCThread )
        pGlobalOfficeIPCThread->mbDowning = true;
}

static bool s_bInEnableRequests = false;

void OfficeIPCThread::EnableRequests( bool i_bEnable )
{
    // switch between just queueing the requests and executing them
    ::osl::MutexGuard   aGuard( GetMutex() );

    if ( pGlobalOfficeIPCThread )
    {
        s_bInEnableRequests = true;
        pGlobalOfficeIPCThread->mbRequestsEnabled = i_bEnable;
        if( i_bEnable )
        {
            // hit the compiler over the head
            ProcessDocumentsRequest aEmptyReq = ProcessDocumentsRequest( boost::optional< rtl::OUString >() );
            // trigger already queued requests
            OfficeIPCThread::ExecuteCmdLineRequests( aEmptyReq );
        }
        s_bInEnableRequests = false;
    }
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

    ::rtl::OUString aUserInstallPath;
    ::rtl::OUString aDummy;

    OfficeIPCThread* pThread = new OfficeIPCThread;

    pThread->maPipeIdent = OUString( RTL_CONSTASCII_USTRINGPARAM( "SingleOfficeIPC_" ) );

    // The name of the named pipe is created with the hashcode of the user installation directory (without /user). We have to retrieve
    // this information from a unotools implementation.
    ::utl::Bootstrap::PathStatus aLocateResult = ::utl::Bootstrap::locateUserInstallation( aUserInstallPath );
    if ( aLocateResult == ::utl::Bootstrap::PATH_EXISTS || aLocateResult == ::utl::Bootstrap::PATH_VALID)
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

    OUString            aIniName;

    osl_getExecutableFile( &aIniName.pData );

    sal_uInt32     lastIndex = aIniName.lastIndexOf('/');
    if ( lastIndex > 0 )
    {
        aIniName    = aIniName.copy( 0, lastIndex+1 );
        aIniName    += OUString( RTL_CONSTASCII_USTRINGPARAM( "perftune" ));
#if defined(WNT) || defined(OS2)
        aIniName    += OUString( RTL_CONSTASCII_USTRINGPARAM( ".ini" ));
#else
        aIniName    += OUString( RTL_CONSTASCII_USTRINGPARAM( "rc" ));
#endif
    }

    ::rtl::Bootstrap aPerfTuneIniFile( aIniName );

    OUString aDefault( RTL_CONSTASCII_USTRINGPARAM( "0" ));
    OUString aPreloadData;

    aPerfTuneIniFile.getFrom( OUString( RTL_CONSTASCII_USTRINGPARAM( "FastPipeCommunication" )), aPreloadData, aDefault );


    OUString aUserInstallPathHashCode;

    if ( aPreloadData.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "1" ) ))
    {
        sal_Char    szBuffer[32];
        sprintf( szBuffer, "%d", SUPD );
        aUserInstallPathHashCode = OUString( szBuffer, strlen(szBuffer), osl_getThreadTextEncoding() );
    }
    else
        aUserInstallPathHashCode = CreateMD5FromString( aDummy );


    // Check result to create a hash code from the user install path
    if ( aUserInstallPathHashCode.getLength() == 0 )
        return IPC_STATUS_BOOTSTRAP_ERROR; // Something completely broken, we cannot create a valid hash code!

    pThread->maPipeIdent = pThread->maPipeIdent + aUserInstallPathHashCode;

    PipeMode nPipeMode = PIPEMODE_DONTKNOW;
    do
    {
        osl::Security &rSecurity = Security::get();
        // Try to create pipe
        if ( pThread->maPipe.create( pThread->maPipeIdent.getStr(), osl_Pipe_CREATE, rSecurity ))
        {
            // Pipe created
            nPipeMode = PIPEMODE_CREATED;
        }
        else if( pThread->maPipe.create( pThread->maPipeIdent.getStr(), osl_Pipe_OPEN, rSecurity )) // Creation not successfull, now we try to connect
        {
            // Pipe connected to first office
            nPipeMode = PIPEMODE_CONNECTED;
        }
        else
        {
            oslPipeError eReason = pThread->maPipe.getError();
            if ((eReason == osl_Pipe_E_ConnectionRefused) || (eReason == osl_Pipe_E_invalidError))
                return IPC_STATUS_BOOTSTRAP_ERROR;

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
        osl::StreamPipe aStreamPipe(pThread->maPipe.getHandle());

        ByteString aArguments(RTL_CONSTASCII_STRINGPARAM(ARGUMENT_PREFIX));
        rtl::OUString cwdUrl;
        if (!(tools::getProcessWorkingDir(cwdUrl) &&
              addArgument(&aArguments, '1', cwdUrl)))
        {
            aArguments += '0';
        }
        sal_uInt32 nCount = rtl_getAppCommandArgCount();
        for( sal_uInt32 i=0; i < nCount; i++ )
        {
            rtl_getAppCommandArg( i, &aDummy.pData );
            if (!addArgument(&aArguments, ',', aDummy)) {
                return IPC_STATUS_BOOTSTRAP_ERROR;
            }
        }
        // finally, write the string onto the pipe
        aStreamPipe.write( aArguments.GetBuffer(), aArguments.Len() );
        aStreamPipe.write( "\0", 1 );

        // wait for confirmation #95361# #95425#
        ByteString aToken(sc_aConfirmationSequence);
        char *aReceiveBuffer = new char[aToken.Len()+1];
        int n = aStreamPipe.read( aReceiveBuffer, aToken.Len() );
        aReceiveBuffer[n]='\0';

        delete pThread;
        if (aToken.CompareTo(aReceiveBuffer)!= COMPARE_EQUAL) {
            // something went wrong
            delete[] aReceiveBuffer;
            return IPC_STATUS_BOOTSTRAP_ERROR;
        } else {
            delete[] aReceiveBuffer;
            return IPC_STATUS_2ND_OFFICE;
        }
    }

    return IPC_STATUS_OK;
}

void OfficeIPCThread::DisableOfficeIPCThread()
{
    osl::ClearableMutexGuard aMutex( GetMutex() );

    if( pGlobalOfficeIPCThread )
    {
        OfficeIPCThread *pOfficeIPCThread = pGlobalOfficeIPCThread;
        pGlobalOfficeIPCThread = 0;

        // send thread a termination message
        // this is done so the subsequent join will not hang
        // because the thread hangs in accept of pipe
        osl::StreamPipe aPipe ( pOfficeIPCThread->maPipeIdent, osl_Pipe_OPEN, Security::get() );
        //Pipe.send( TERMINATION_SEQUENCE, TERMINATION_LENGTH );
        if (aPipe.is())
        {
            aPipe.send( sc_aTerminationSequence, sc_nTSeqLength+1 ); // also send 0-byte

            // close the pipe so that the streampipe on the other
            // side produces EOF
            aPipe.close();
        }

        // release mutex to avoid deadlocks
        aMutex.clear();

        OfficeIPCThread::SetReady(pOfficeIPCThread);

        // exit gracefully and join
        pOfficeIPCThread->join();
        delete pOfficeIPCThread;


    }
}

OfficeIPCThread::OfficeIPCThread() :
    mbDowning( false ),
    mbRequestsEnabled( false ),
    mnPendingRequests( 0 ),
    mpDispatchWatcher( 0 )
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

static void AddURLToStringList( const rtl::OUString& aURL, rtl::OUString& aStringList )
{
    if ( aStringList.getLength() )
        aStringList += ::rtl::OUString::valueOf( (sal_Unicode)APPEVENT_PARAM_DELIMITER );
    aStringList += aURL;
}

void OfficeIPCThread::SetReady(OfficeIPCThread* pThread)
{
    if (pThread == NULL) pThread = pGlobalOfficeIPCThread;
    if (pThread != NULL)
    {
        pThread->cReady.set();
    }
}

void SAL_CALL OfficeIPCThread::run()
{
    do
    {
        oslPipeError nError = maPipe.accept( maStreamPipe );


        if( nError == osl_Pipe_E_None )
        {

            // #111143# and others:
            // if we receive a request while the office is displaying some dialog or error during
            // bootstrap, that dialogs event loop might get events that are dispatched by this thread
            // we have to wait for cReady to be set by the real main loop.
            // only reqests that dont dispatch events may be processed before cReady is set.
            cReady.wait();

            // we might have decided to shutdown while we were sleeping
            if (!pGlobalOfficeIPCThread) return;

            // only lock the mutex when processing starts, othewise we deadlock when the office goes
            // down during wait
            osl::ClearableMutexGuard aGuard( GetMutex() );

            ByteString aArguments;
            // test byte by byte
            const int nBufSz = 2048;
            char pBuf[nBufSz];
            int nBytes = 0;
            int nResult = 0;
            // read into pBuf until '\0' is read or read-error
            while ((nResult=maStreamPipe.recv( pBuf+nBytes, nBufSz-nBytes))>0) {
                nBytes += nResult;
                if (pBuf[nBytes-1]=='\0') {
                    aArguments += pBuf;
                    break;
                }
            }
            // don't close pipe ...

            // #90717# Is this a lookup message from another application? if so, ignore
            if ( aArguments.Len() == 0 )
                continue;

            // is this a termination message ? if so, terminate
            if(( aArguments.CompareTo( sc_aTerminationSequence, sc_nTSeqLength ) == COMPARE_EQUAL ) ||
                    mbDowning ) return;
            String           aEmpty;
            std::auto_ptr< CommandLineArgs > aCmdLineArgs;
            try
            {
                Parser p( aArguments );
                aCmdLineArgs.reset( new CommandLineArgs( p ) );
            }
            catch ( CommandLineArgs::Supplier::Exception & )
            {
#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
                fprintf( stderr, "Error in received command line arguments\n" );
#endif
                continue;
            }
            CommandLineArgs *pCurrentCmdLineArgs = Desktop::GetCommandLineArgs();

            if ( aCmdLineArgs->IsQuickstart() )
            {
                // we have to use application event, because we have to start quickstart service in main thread!!
                ApplicationEvent* pAppEvent =
                    new ApplicationEvent( aEmpty, aEmpty,
                                            "QUICKSTART", aEmpty );
                ImplPostForeignAppEvent( pAppEvent );
            }

            // handle request for acceptor
            OUString aAcceptString;
            if ( aCmdLineArgs->GetAcceptString(aAcceptString) && Desktop::CheckOEM()) {
                ApplicationEvent* pAppEvent =
                    new ApplicationEvent( aEmpty, aEmpty,
                                          "ACCEPT", aAcceptString );
                ImplPostForeignAppEvent( pAppEvent );
            }
            // handle acceptor removal
            OUString aUnAcceptString;
            if ( aCmdLineArgs->GetUnAcceptString(aUnAcceptString) ) {
                ApplicationEvent* pAppEvent =
                    new ApplicationEvent( aEmpty, aEmpty,
                                         "UNACCEPT", aUnAcceptString );
                ImplPostForeignAppEvent( pAppEvent );
            }

#ifndef UNX
            // only in non-unix version, we need to handle a -help request
            // in a running instance in order to display  the command line help
            if ( aCmdLineArgs->IsHelp() ) {
                ApplicationEvent* pAppEvent =
                    new ApplicationEvent( aEmpty, aEmpty, "HELP", aEmpty );
                ImplPostForeignAppEvent( pAppEvent );
            }
#endif

            sal_Bool bDocRequestSent = sal_False;
            ProcessDocumentsRequest* pRequest = new ProcessDocumentsRequest(
                aCmdLineArgs->getCwdUrl());
            cProcessed.reset();
            pRequest->pcProcessed = &cProcessed;

            // Print requests are not dependent on the -invisible cmdline argument as they are
            // loaded with the "hidden" flag! So they are always checked.
            bDocRequestSent |= aCmdLineArgs->GetPrintList( pRequest->aPrintList );
            bDocRequestSent |= ( aCmdLineArgs->GetPrintToList( pRequest->aPrintToList ) &&
                                    aCmdLineArgs->GetPrinterName( pRequest->aPrinterName )      );

            if ( !pCurrentCmdLineArgs->IsInvisible() )
            {
                // Read cmdline args that can open/create documents. As they would open a window
                // they are only allowed if the "-invisible" is currently not used!
                bDocRequestSent |= aCmdLineArgs->GetOpenList( pRequest->aOpenList );
                bDocRequestSent |= aCmdLineArgs->GetViewList( pRequest->aViewList );
                bDocRequestSent |= aCmdLineArgs->GetStartList( pRequest->aStartList );
                bDocRequestSent |= aCmdLineArgs->GetForceOpenList( pRequest->aForceOpenList );
                bDocRequestSent |= aCmdLineArgs->GetForceNewList( pRequest->aForceNewList );

                // Special command line args to create an empty document for a given module

                // #i18338# (lo)
                // we only do this if no document was specified on the command line,
                // since this would be inconsistent with the the behaviour of
                // the first process, see OpenClients() (call to OpenDefault()) in app.cxx
                if ( aCmdLineArgs->HasModuleParam() && Desktop::CheckOEM() && (!bDocRequestSent))
                {
                    SvtModuleOptions aOpt;
                    SvtModuleOptions::EFactory eFactory = SvtModuleOptions::E_WRITER;
                    if ( aCmdLineArgs->IsWriter() )
                        eFactory = SvtModuleOptions::E_WRITER;
                    else if ( aCmdLineArgs->IsCalc() )
                        eFactory = SvtModuleOptions::E_CALC;
                    else if ( aCmdLineArgs->IsDraw() )
                        eFactory = SvtModuleOptions::E_DRAW;
                    else if ( aCmdLineArgs->IsImpress() )
                        eFactory = SvtModuleOptions::E_IMPRESS;
                    else if ( aCmdLineArgs->IsBase() )
                        eFactory = SvtModuleOptions::E_DATABASE;
                    else if ( aCmdLineArgs->IsMath() )
                        eFactory = SvtModuleOptions::E_MATH;
                    else if ( aCmdLineArgs->IsGlobal() )
                        eFactory = SvtModuleOptions::E_WRITERGLOBAL;
                    else if ( aCmdLineArgs->IsWeb() )
                        eFactory = SvtModuleOptions::E_WRITERWEB;

                    if ( pRequest->aOpenList.getLength() )
                        pRequest->aModule = aOpt.GetFactoryName( eFactory );
                    else
                        AddURLToStringList( aOpt.GetFactoryEmptyDocumentURL( eFactory ), pRequest->aOpenList );
                    bDocRequestSent = sal_True;
                }
            }

            if (!aCmdLineArgs->IsQuickstart() && Desktop::CheckOEM()) {
                sal_Bool bShowHelp = sal_False;
                rtl::OUStringBuffer aHelpURLBuffer;
                if (aCmdLineArgs->IsHelpWriter()) {
                    bShowHelp = sal_True;
                    aHelpURLBuffer.appendAscii("vnd.sun.star.help://swriter/start");
                } else if (aCmdLineArgs->IsHelpCalc()) {
                    bShowHelp = sal_True;
                    aHelpURLBuffer.appendAscii("vnd.sun.star.help://scalc/start");
                } else if (aCmdLineArgs->IsHelpDraw()) {
                    bShowHelp = sal_True;
                    aHelpURLBuffer.appendAscii("vnd.sun.star.help://sdraw/start");
                } else if (aCmdLineArgs->IsHelpImpress()) {
                    bShowHelp = sal_True;
                    aHelpURLBuffer.appendAscii("vnd.sun.star.help://simpress/start");
                } else if (aCmdLineArgs->IsHelpBase()) {
                    bShowHelp = sal_True;
                    aHelpURLBuffer.appendAscii("vnd.sun.star.help://sdatabase/start");
                } else if (aCmdLineArgs->IsHelpBasic()) {
                    bShowHelp = sal_True;
                    aHelpURLBuffer.appendAscii("vnd.sun.star.help://sbasic/start");
                } else if (aCmdLineArgs->IsHelpMath()) {
                    bShowHelp = sal_True;
                    aHelpURLBuffer.appendAscii("vnd.sun.star.help://smath/start");
                }
                if (bShowHelp) {
                    Any aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::LOCALE );
                    rtl::OUString aTmp;
                    aRet >>= aTmp;
                    aHelpURLBuffer.appendAscii("?Language=");
                    aHelpURLBuffer.append(aTmp);
#if defined UNX
                    aHelpURLBuffer.appendAscii("&System=UNX");
#elif defined WNT
                    aHelpURLBuffer.appendAscii("&System=WIN");
#elif defined OS2
                    aHelpURLBuffer.appendAscii("&System=OS2");
#endif
                    ApplicationEvent* pAppEvent =
                        new ApplicationEvent( aEmpty, aEmpty,
                                              "OPENHELPURL", aHelpURLBuffer.makeStringAndClear());
                    ImplPostForeignAppEvent( pAppEvent );
                }
            }

            if ( bDocRequestSent && Desktop::CheckOEM())
             {
                // Send requests to dispatch watcher if we have at least one. The receiver
                // is responsible to delete the request after processing it.
                if ( aCmdLineArgs->HasModuleParam() )
                {
                    SvtModuleOptions    aOpt;

                    // Support command line parameters to start a module (as preselection)
                    if ( aCmdLineArgs->IsWriter() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
                        pRequest->aModule = aOpt.GetFactoryName( SvtModuleOptions::E_WRITER );
                    else if ( aCmdLineArgs->IsCalc() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
                        pRequest->aModule = aOpt.GetFactoryName( SvtModuleOptions::E_CALC );
                    else if ( aCmdLineArgs->IsImpress() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
                        pRequest->aModule= aOpt.GetFactoryName( SvtModuleOptions::E_IMPRESS );
                    else if ( aCmdLineArgs->IsDraw() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
                        pRequest->aModule= aOpt.GetFactoryName( SvtModuleOptions::E_DRAW );
                }


                ImplPostProcessDocumentsEvent( pRequest );
            }
            else
            {
                // delete not used request again
                delete pRequest;
                pRequest = NULL;
            }
            if (( aArguments.CompareTo( sc_aShowSequence, sc_nShSeqLength ) == COMPARE_EQUAL ) ||
                aCmdLineArgs->IsEmpty() )
            {
                // no document was sent, just bring Office to front
                ApplicationEvent* pAppEvent =
                        new ApplicationEvent( aEmpty, aEmpty, "APPEAR", aEmpty );
                ImplPostForeignAppEvent( pAppEvent );
            }

            // we don't need the mutex any longer...
            aGuard.clear();
            // wait for processing to finish
            if (bDocRequestSent)
                cProcessed.wait();
            // processing finished, inform the requesting end
            nBytes = 0;
            while (
                   (nResult = maStreamPipe.send(sc_aConfirmationSequence+nBytes, sc_nCSeqLength-nBytes))>0 &&
                   ((nBytes += nResult) < sc_nCSeqLength) ) ;
        }
        else
        {
#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
            fprintf( stderr, "Error on accept: %d\n", (int)nError );
#endif
            TimeValue tval;
            tval.Seconds = 1;
            tval.Nanosec = 0;
            wait( tval );
        }
    } while( schedule() );
}

static void AddToDispatchList(
    DispatchWatcher::DispatchList& rDispatchList,
    boost::optional< rtl::OUString > const & cwdUrl,
    const OUString& aRequestList,
    DispatchWatcher::RequestType nType,
    const OUString& aParam,
    const OUString& aFactory )
{
    if ( aRequestList.getLength() > 0 )
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = aRequestList.getToken( 0, APPEVENT_PARAM_DELIMITER, nIndex );
            if ( aToken.getLength() > 0 )
                rDispatchList.push_back(
                    DispatchWatcher::DispatchRequest( nType, aToken, cwdUrl, aParam, aFactory ));
        }
        while ( nIndex >= 0 );
    }
}

static void AddConversionsToDispatchList(
    DispatchWatcher::DispatchList& rDispatchList,
    boost::optional< rtl::OUString > const & cwdUrl,
    const OUString& rRequestList,
    const OUString& rParam,
    const OUString& rPrinterName,
    const OUString& rFactory,
    const OUString& rParamOut )
{
    DispatchWatcher::RequestType nType;
    OUString aParam( rParam );

    if( rParam.getLength() )
    {
        nType = DispatchWatcher::REQUEST_CONVERSION;
        aParam = rParam;
    }
    else
    {
        nType = DispatchWatcher::REQUEST_BATCHPRINT;
        aParam = rPrinterName;
    }

    OUString aOutDir( rParamOut.trim() );
    ::rtl::OUString aPWD;
    ::tools::getProcessWorkingDir( aPWD );

    if( !::osl::FileBase::getAbsoluteFileURL( aPWD, rParamOut, aOutDir ) )
        ::osl::FileBase::getSystemPathFromFileURL( aOutDir, aOutDir );

    if( rParamOut.trim().getLength() )
    {
        aParam += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";"));
        aParam += aOutDir;
    }
    else
    {
        ::osl::FileBase::getSystemPathFromFileURL( aPWD, aPWD );
        aParam += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ";" )) + aPWD;
    }

    if ( rRequestList.getLength() > 0 )
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = rRequestList.getToken( 0, APPEVENT_PARAM_DELIMITER, nIndex );
            if ( aToken.getLength() > 0 )
                rDispatchList.push_back(
                    DispatchWatcher::DispatchRequest( nType, aToken, cwdUrl, aParam, rFactory ));
        }
        while ( nIndex >= 0 );
    }
}


sal_Bool OfficeIPCThread::ExecuteCmdLineRequests( ProcessDocumentsRequest& aRequest )
{
    // protect the dispatch list
    osl::ClearableMutexGuard aGuard( GetMutex() );

    static DispatchWatcher::DispatchList    aDispatchList;

    rtl::OUString aEmpty;
    // Create dispatch list for dispatch watcher
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aInFilter, DispatchWatcher::REQUEST_INFILTER, aEmpty, aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aOpenList, DispatchWatcher::REQUEST_OPEN, aEmpty, aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aViewList, DispatchWatcher::REQUEST_VIEW, aEmpty, aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aStartList, DispatchWatcher::REQUEST_START, aEmpty, aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aPrintList, DispatchWatcher::REQUEST_PRINT, aEmpty, aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aPrintToList, DispatchWatcher::REQUEST_PRINTTO, aRequest.aPrinterName, aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aForceOpenList, DispatchWatcher::REQUEST_FORCEOPEN, aEmpty, aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aForceNewList, DispatchWatcher::REQUEST_FORCENEW, aEmpty, aRequest.aModule );
    AddConversionsToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aConversionList, aRequest.aConversionParams, aRequest.aPrinterName, aRequest.aModule, aRequest.aConversionOut );
    sal_Bool bShutdown( sal_False );

    if ( pGlobalOfficeIPCThread )
    {
        if( ! pGlobalOfficeIPCThread->AreRequestsEnabled() )
            return bShutdown;

        pGlobalOfficeIPCThread->mnPendingRequests += aDispatchList.size();
        if ( !pGlobalOfficeIPCThread->mpDispatchWatcher )
        {
            pGlobalOfficeIPCThread->mpDispatchWatcher = DispatchWatcher::GetDispatchWatcher();
            pGlobalOfficeIPCThread->mpDispatchWatcher->acquire();
        }

        // copy for execute
        DispatchWatcher::DispatchList aTempList( aDispatchList );
        aDispatchList.clear();

        aGuard.clear();

        // Execute dispatch requests
        bShutdown = pGlobalOfficeIPCThread->mpDispatchWatcher->executeDispatchRequests( aTempList, s_bInEnableRequests );

        // set processed flag
        if (aRequest.pcProcessed != NULL)
            aRequest.pcProcessed->set();
    }

    return bShutdown;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
