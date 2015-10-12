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

#include <config_features.h>
#include <config_version.h>

#include "app.hxx"
#include "officeipcthread.hxx"
#include "cmdlineargs.hxx"
#include "dispatchwatcher.hxx"
#include <stdio.h>
#include <osl/process.h>
#include <sal/log.hxx>
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
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <rtl/process.h>
#include <tools/getprocessworkingdir.hxx>
#include <memory>

using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;

namespace {

#if HAVE_FEATURE_DESKTOP || defined(ANDROID)

static char const ARGUMENT_PREFIX[] = "InternalIPC::Arguments";
static char const SEND_ARGUMENTS[] = "InternalIPC::SendArguments";
static char const PROCESSING_DONE[] = "InternalIPC::ProcessingDone";

// Receives packets from the pipe until a packet ends in a NUL character (that
// will not be included in the returned string) or it cannot read anything (due
// to error or closed pipe, in which case an empty string will be returned to
// signal failure):
OString readStringFromPipe(osl::StreamPipe & pipe) {
    for (OStringBuffer str;;) {
        char buf[1024];
        sal_Int32 n = pipe.recv(buf, SAL_N_ELEMENTS(buf));
        if (n <= 0) {
            return "";
        }
        bool end = false;
        if (buf[n - 1] == '\0') {
            end = true;
            --n;
        }
        str.append(buf, n);
            //TODO: how does OStringBuffer.append handle overflow?
        if (end) {
            return str.makeStringAndClear();
        }
    }
}

#endif

}

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

#if HAVE_FEATURE_DESKTOP || defined(ANDROID)

class Parser: public CommandLineArgs::Supplier {
public:
    explicit Parser(OString const & input): m_input(input) {
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
                OUString url;
                if (!next(&url, false)) {
                    throw CommandLineArgs::Supplier::Exception();
                }
                m_cwdUrl.reset(url);
                break;
            }
        case '2':
            {
                OUString path;
                if (!next(&path, false)) {
                    throw CommandLineArgs::Supplier::Exception();
                }
                OUString url;
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

    virtual boost::optional< OUString > getCwdUrl() override { return m_cwdUrl; }

    virtual bool next(OUString * argument) override { return next(argument, true); }

private:
    bool next(OUString * argument, bool prefix) {
        OSL_ASSERT(argument != NULL);
        if (m_index < m_input.getLength()) {
            if (prefix) {
                if (m_input[m_index] != ',') {
                    throw CommandLineArgs::Supplier::Exception();
                }
                ++m_index;
            }
            OStringBuffer b;
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
            OString b2(b.makeStringAndClear());
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

    boost::optional< OUString > m_cwdUrl;
    OString m_input;
    sal_Int32 m_index;
};

bool addArgument(OStringBuffer &rArguments, char prefix,
    const OUString &rArgument)
{
    OString utf8;
    if (!rArgument.convertToString(
            &utf8, RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        return false;
    }
    rArguments.append(prefix);
    for (sal_Int32 i = 0; i < utf8.getLength(); ++i) {
        char c = utf8[i];
        switch (c) {
        case '\0':
            rArguments.append("\\0");
            break;
        case ',':
            rArguments.append("\\,");
            break;
        case '\\':
            rArguments.append("\\\\");
            break;
        default:
            rArguments.append(c);
            break;
        }
    }
    return true;
}

#endif

}

rtl::Reference< OfficeIPCThread > OfficeIPCThread::pGlobalOfficeIPCThread;
    namespace { struct Security : public rtl::Static<osl::Security, Security> {}; }

// Turns a string in aMsg such as file:///home/foo/.libreoffice/3
// Into a hex string of well known length ff132a86...
OUString CreateMD5FromString( const OUString& aMsg )
{
#if (OSL_DEBUG_LEVEL > 2)
    fprintf( stderr, "create md5 from '%s'\n",
             OUStringToOString (aMsg, RTL_TEXTENCODING_UTF8).getStr() );
#endif

    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    if ( handle )
    {
        const sal_uInt8* pData = reinterpret_cast<const sal_uInt8*>(aMsg.getStr());
        sal_uInt32       nSize = ( aMsg.getLength() * sizeof( sal_Unicode ));
        sal_uInt32       nMD5KeyLen = rtl_digest_queryLength( handle );
        std::unique_ptr<sal_uInt8[]> pMD5KeyBuffer(new sal_uInt8[ nMD5KeyLen ]);

        rtl_digest_init( handle, pData, nSize );
        rtl_digest_update( handle, pData, nSize );
        rtl_digest_get( handle, pMD5KeyBuffer.get(), nMD5KeyLen );
        rtl_digest_destroy( handle );

        // Create hex-value string from the MD5 value to keep the string size minimal
        OUStringBuffer aBuffer( nMD5KeyLen * 2 + 1 );
        for ( sal_uInt32 i = 0; i < nMD5KeyLen; i++ )
            aBuffer.append( (sal_Int32)pMD5KeyBuffer[i], 16 );

        return aBuffer.makeStringAndClear();
    }

    return OUString();
}

class ProcessEventsClass_Impl
{
public:
    DECL_STATIC_LINK_TYPED( ProcessEventsClass_Impl, CallEvent, void*, void );
    DECL_STATIC_LINK_TYPED( ProcessEventsClass_Impl, ProcessDocumentsEvent, void*, void );
};

IMPL_STATIC_LINK_TYPED( ProcessEventsClass_Impl, CallEvent, void*, pEvent, void )
{
    // Application events are processed by the Desktop::HandleAppEvent implementation.
    Desktop::HandleAppEvent( *static_cast<ApplicationEvent*>(pEvent) );
    delete static_cast<ApplicationEvent*>(pEvent);
}

IMPL_STATIC_LINK_TYPED( ProcessEventsClass_Impl, ProcessDocumentsEvent, void*, pEvent, void )
{
    // Documents requests are processed by the OfficeIPCThread implementation
    ProcessDocumentsRequest* pDocsRequest = static_cast<ProcessDocumentsRequest*>(pEvent);

    if ( pDocsRequest )
    {
        OfficeIPCThread::ExecuteCmdLineRequests( *pDocsRequest );
        delete pDocsRequest;
    }
}

void ImplPostForeignAppEvent( ApplicationEvent* pEvent )
{
    Application::PostUserEvent( LINK( NULL, ProcessEventsClass_Impl, CallEvent ), pEvent );
}

void ImplPostProcessDocumentsEvent( ProcessDocumentsRequest* pEvent )
{
    Application::PostUserEvent( LINK( NULL, ProcessEventsClass_Impl, ProcessDocumentsEvent ), pEvent );
}

oslSignalAction SAL_CALL SalMainPipeExchangeSignal_impl(void* /*pData*/, oslSignalInfo* pInfo)
{
    if( pInfo->Signal == osl_Signal_Terminate )
        OfficeIPCThread::DisableOfficeIPCThread(false);
    return osl_Signal_ActCallNextHdl;
}



// The OfficeIPCThreadController implementation is a bookkeeper for all pending requests
// that were created by the OfficeIPCThread. The requests are waiting to be processed by
// our framework loadComponentFromURL function (e.g. open/print request).
// During shutdown the framework is asking OfficeIPCThreadController about pending requests.
// If there are pending requests framework has to stop the shutdown process. It is waiting
// for these requests because framework is not able to handle shutdown and open a document
// concurrently.


// XServiceInfo
OUString SAL_CALL OfficeIPCThreadController::getImplementationName()
throw ( RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.OfficeIPCThreadController" );
}

sal_Bool OfficeIPCThreadController::supportsService(
    OUString const & ServiceName) throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL OfficeIPCThreadController::getSupportedServiceNames()
throw ( RuntimeException, std::exception )
{
    Sequence< OUString > aSeq( 0 );
    return aSeq;
}

// XEventListener
void SAL_CALL OfficeIPCThreadController::disposing( const EventObject& )
throw( RuntimeException, std::exception )
{
}

// XTerminateListener
void SAL_CALL OfficeIPCThreadController::queryTermination( const EventObject& )
throw( TerminationVetoException, RuntimeException, std::exception )
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
throw( RuntimeException, std::exception )
{
}

namespace
{
    class theOfficeIPCThreadMutex
        : public rtl::Static<osl::Mutex, theOfficeIPCThreadMutex> {};
}

::osl::Mutex& OfficeIPCThread::GetMutex()
{
    return theOfficeIPCThreadMutex::get();
}

void OfficeIPCThread::SetDowning()
{
    // We have the order to block all incoming requests. Framework
    // wants to shutdown and we have to make sure that no loading/printing
    // requests are executed anymore.
    ::osl::MutexGuard   aGuard( GetMutex() );

    if ( pGlobalOfficeIPCThread.is() )
        pGlobalOfficeIPCThread->mbDowning = true;
}

static bool s_bInEnableRequests = false;

void OfficeIPCThread::EnableRequests( bool i_bEnable )
{
    // switch between just queueing the requests and executing them
    ::osl::MutexGuard   aGuard( GetMutex() );

    if ( pGlobalOfficeIPCThread.is() )
    {
        s_bInEnableRequests = true;
        pGlobalOfficeIPCThread->mbRequestsEnabled = i_bEnable;
        if( i_bEnable )
        {
            // hit the compiler over the head
            ProcessDocumentsRequest aEmptyReq = ProcessDocumentsRequest( boost::optional< OUString >() );
            // trigger already queued requests
            OfficeIPCThread::ExecuteCmdLineRequests( aEmptyReq );
        }
        s_bInEnableRequests = false;
    }
}

bool OfficeIPCThread::AreRequestsPending()
{
    // Give info about pending requests
    ::osl::MutexGuard   aGuard( GetMutex() );
    if ( pGlobalOfficeIPCThread.is() )
        return ( pGlobalOfficeIPCThread->mnPendingRequests > 0 );
    else
        return false;
}

void OfficeIPCThread::RequestsCompleted( int nCount )
{
    // Remove nCount pending requests from our internal counter
    ::osl::MutexGuard   aGuard( GetMutex() );
    if ( pGlobalOfficeIPCThread.is() )
    {
        if ( pGlobalOfficeIPCThread->mnPendingRequests > 0 )
            pGlobalOfficeIPCThread->mnPendingRequests -= nCount;
    }
}

OfficeIPCThread::Status OfficeIPCThread::EnableOfficeIPCThread()
{
    ::osl::MutexGuard   aGuard( GetMutex() );

    if( pGlobalOfficeIPCThread.is() )
        return IPC_STATUS_OK;

#if HAVE_FEATURE_DESKTOP || defined(ANDROID)
    OUString aUserInstallPath;
    OUString aDummy;

    rtl::Reference< OfficeIPCThread > pThread(new OfficeIPCThread);

    PipeMode nPipeMode = PIPEMODE_DONTKNOW;

#ifndef ANDROID // On Android it might be that we still for some reason need the pipe?

    // In LibreOfficeKit-based programs we want to be totally independent from any other LibreOffice
    // instance or LOKit-using program. Certainly no need for any IPC pipes by definition, as we
    // don't have any reason to do any IPC. Why we even call this EnableOfficeIPCThread function
    // from LibreOfficeKit's lo_initialize() I am not completely sure, but that code, and this, is
    // such horrible crack that I don't want to change it too much.

    if (comphelper::LibreOfficeKit::isActive())
    {
        // Setting nPipeMode to PIPEMODE_CREATED causes the trivial path to be taken below, starting
        // the listening thread. (Which will immediately finish, see the execute() function, but what
        // the heck...)
        nPipeMode = PIPEMODE_CREATED;
    }
    else
#endif
    {
        // The name of the named pipe is created with the hashcode of the user installation directory (without /user). We have to retrieve
        // this information from a unotools implementation.
        ::utl::Bootstrap::PathStatus aLocateResult = ::utl::Bootstrap::locateUserInstallation( aUserInstallPath );
        if ( aLocateResult == ::utl::Bootstrap::PATH_EXISTS || aLocateResult == ::utl::Bootstrap::PATH_VALID)
            aDummy = aUserInstallPath;
        else
        {
            return IPC_STATUS_BOOTSTRAP_ERROR;
        }

        // Try to  determine if we are the first office or not! This should prevent multiple
        // access to the user directory !
        // First we try to create our pipe if this fails we try to connect. We have to do this
        // in a loop because the other office can crash or shutdown between createPipe
        // and connectPipe!!

        OUString            aIniName;

        osl_getExecutableFile( &aIniName.pData );

        sal_uInt32     lastIndex = aIniName.lastIndexOf('/');
        if ( lastIndex > 0 )
        {
            aIniName    = aIniName.copy( 0, lastIndex+1 );
            aIniName    += "perftune";
    #if defined(WNT)
            aIniName    += ".ini";
    #else
            aIniName    += "rc";
    #endif
        }

        ::rtl::Bootstrap aPerfTuneIniFile( aIniName );

        OUString aDefault( "0" );
        OUString aPreloadData;

        aPerfTuneIniFile.getFrom( OUString( "FastPipeCommunication" ), aPreloadData, aDefault );


        OUString aUserInstallPathHashCode;

        if ( aPreloadData == "1" )
        {
            sal_Char    szBuffer[32];
            sprintf( szBuffer, "%d", LIBO_VERSION_MAJOR * 10000 + LIBO_VERSION_MINOR * 100 + LIBO_VERSION_MICRO * 1 );
            aUserInstallPathHashCode = OUString( szBuffer, strlen(szBuffer), osl_getThreadTextEncoding() );
        }
        else
            aUserInstallPathHashCode = CreateMD5FromString( aDummy );


        // Check result to create a hash code from the user install path
        if ( aUserInstallPathHashCode.isEmpty() )
            return IPC_STATUS_BOOTSTRAP_ERROR; // Something completely broken, we cannot create a valid hash code!

        OUString aPipeIdent( "SingleOfficeIPC_" + aUserInstallPathHashCode );

        do
        {
            osl::Security &rSecurity = Security::get();

            // Try to create pipe
            if ( pThread->maPipe.create( aPipeIdent.getStr(), osl_Pipe_CREATE, rSecurity ))
            {
                // Pipe created
                nPipeMode = PIPEMODE_CREATED;
            }
            else if( pThread->maPipe.create( aPipeIdent.getStr(), osl_Pipe_OPEN, rSecurity )) // Creation not successful, now we try to connect
            {
                osl::StreamPipe aStreamPipe(pThread->maPipe.getHandle());
                if (readStringFromPipe(aStreamPipe) == SEND_ARGUMENTS)
                {
                    // Pipe connected to first office
                    nPipeMode = PIPEMODE_CONNECTED;
                }
                else
                {
                    // Pipe connection failed (other office exited or crashed)
                    TimeValue tval;
                    tval.Seconds = 0;
                    tval.Nanosec = 500000000;
                    salhelper::Thread::wait( tval );
                }
            }
            else
            {
                oslPipeError eReason = pThread->maPipe.getError();
                if ((eReason == osl_Pipe_E_ConnectionRefused) || (eReason == osl_Pipe_E_invalidError))
                    return IPC_STATUS_PIPE_ERROR;

                // Wait for second office to be ready
                TimeValue aTimeValue;
                aTimeValue.Seconds = 0;
                aTimeValue.Nanosec = 10000000; // 10ms
                salhelper::Thread::wait( aTimeValue );
            }

        } while ( nPipeMode == PIPEMODE_DONTKNOW );
    }

    if ( nPipeMode == PIPEMODE_CREATED )
    {
        // Seems we are the one and only, so start listening thread
        pGlobalOfficeIPCThread = pThread;
        pThread->launch();
    }
    else
    {
        // Seems another office is running. Pipe arguments to it and self terminate
        osl::StreamPipe aStreamPipe(pThread->maPipe.getHandle());

        OStringBuffer aArguments(ARGUMENT_PREFIX);
        OUString cwdUrl;
        if (!(tools::getProcessWorkingDir(cwdUrl) &&
              addArgument(aArguments, '1', cwdUrl)))
        {
            aArguments.append('0');
        }
        sal_uInt32 nCount = rtl_getAppCommandArgCount();
        for( sal_uInt32 i=0; i < nCount; i++ )
        {
            rtl_getAppCommandArg( i, &aDummy.pData );
            if (!addArgument(aArguments, ',', aDummy)) {
                return IPC_STATUS_BOOTSTRAP_ERROR;
            }
        }
        aArguments.append('\0');
        // finally, write the string onto the pipe
        sal_Int32 n = aStreamPipe.write(
            aArguments.getStr(), aArguments.getLength());
        if (n != aArguments.getLength()) {
            SAL_INFO("desktop", "short write: " << n);
            return IPC_STATUS_BOOTSTRAP_ERROR;
        }

        if (readStringFromPipe(aStreamPipe) != PROCESSING_DONE)
        {
            // something went wrong
            return IPC_STATUS_BOOTSTRAP_ERROR;
        }

        return IPC_STATUS_2ND_OFFICE;
    }
#else
    rtl::Reference< OfficeIPCThread > pThread(new OfficeIPCThread);
    pGlobalOfficeIPCThread = pThread;
    pThread->launch();
#endif
    return IPC_STATUS_OK;
}

void OfficeIPCThread::DisableOfficeIPCThread(bool join)
{
    osl::ClearableMutexGuard aMutex( GetMutex() );

    if( pGlobalOfficeIPCThread.is() )
    {
        rtl::Reference< OfficeIPCThread > pOfficeIPCThread(
            pGlobalOfficeIPCThread);
        pGlobalOfficeIPCThread.clear();

        pOfficeIPCThread->mbDowning = true;
        pOfficeIPCThread->maPipe.close();

        // release mutex to avoid deadlocks
        aMutex.clear();

        OfficeIPCThread::SetReady(pOfficeIPCThread);

        // exit gracefully and join
        if (join)
        {
            pOfficeIPCThread->join();
        }
    }
}

OfficeIPCThread::OfficeIPCThread() :
    Thread( "OfficeIPCThread" ),
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
    pGlobalOfficeIPCThread.clear();
}

void OfficeIPCThread::SetReady(
    rtl::Reference< OfficeIPCThread > const & pThread)
{
    rtl::Reference< OfficeIPCThread > const & t(
        pThread.is() ? pThread : pGlobalOfficeIPCThread);
    if (t.is())
    {
        t->cReady.set();
    }
}

void OfficeIPCThread::WaitForReady(
    rtl::Reference< OfficeIPCThread > const & pThread)

{
    rtl::Reference< OfficeIPCThread > const & t(
        pThread.is() ? pThread : pGlobalOfficeIPCThread);
    if (t.is())
    {
        t->cReady.wait();
    }
}

bool OfficeIPCThread::IsEnabled()
{
    return pGlobalOfficeIPCThread.is();
}

void OfficeIPCThread::execute()
{
#if HAVE_FEATURE_DESKTOP || defined(ANDROID)

#ifndef ANDROID
    if (comphelper::LibreOfficeKit::isActive())
        return;
#endif

    do
    {
        osl::StreamPipe aStreamPipe;
        oslPipeError nError = maPipe.accept( aStreamPipe );


        if( nError == osl_Pipe_E_None )
        {
            // if we receive a request while the office is displaying some dialog or error during
            // bootstrap, that dialogs event loop might get events that are dispatched by this thread
            // we have to wait for cReady to be set by the real main loop.
            // only requests that don't dispatch events may be processed before cReady is set.
            cReady.wait();

            // we might have decided to shutdown while we were sleeping
            if (!pGlobalOfficeIPCThread.is()) return;

            // only lock the mutex when processing starts, othewise we deadlock when the office goes
            // down during wait
            osl::ClearableMutexGuard aGuard( GetMutex() );

            if ( mbDowning )
            {
                break;
            }

            // notify client we're ready to process its args:
            sal_Int32 n = aStreamPipe.write(
                SEND_ARGUMENTS, SAL_N_ELEMENTS(SEND_ARGUMENTS));
                // incl. terminating NUL
            if (n != SAL_N_ELEMENTS(SEND_ARGUMENTS)) {
                SAL_WARN("desktop", "short write: " << n);
                continue;
            }

            OString aArguments = readStringFromPipe(aStreamPipe);

            // Is this a lookup message from another application? if so, ignore
            if (aArguments.isEmpty())
                continue;

            std::unique_ptr< CommandLineArgs > aCmdLineArgs;
            try
            {
                Parser p(aArguments);
                aCmdLineArgs.reset( new CommandLineArgs( p ) );
            }
            catch ( const CommandLineArgs::Supplier::Exception & )
            {
#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
                fprintf( stderr, "Error in received command line arguments\n" );
#endif
                continue;
            }

            bool bDocRequestSent = false;

            OUString aUnknown( aCmdLineArgs->GetUnknown() );
            if ( !aUnknown.isEmpty() || aCmdLineArgs->IsHelp() )
            {
                ApplicationEvent* pAppEvent =
                    new ApplicationEvent(ApplicationEvent::TYPE_HELP, aUnknown);
                ImplPostForeignAppEvent( pAppEvent );
            }
            else if ( aCmdLineArgs->IsVersion() )
            {
                ApplicationEvent* pAppEvent =
                    new ApplicationEvent(ApplicationEvent::TYPE_VERSION);
                ImplPostForeignAppEvent( pAppEvent );
            }
            else
            {
                const CommandLineArgs &rCurrentCmdLineArgs = Desktop::GetCommandLineArgs();

                if ( aCmdLineArgs->IsQuickstart() )
                {
                    // we have to use application event, because we have to start quickstart service in main thread!!
                    ApplicationEvent* pAppEvent =
                        new ApplicationEvent(ApplicationEvent::TYPE_QUICKSTART);
                    ImplPostForeignAppEvent( pAppEvent );
                }

                // handle request for acceptor
                std::vector< OUString > const & accept = aCmdLineArgs->
                    GetAccept();
                for (std::vector< OUString >::const_iterator i(accept.begin());
                     i != accept.end(); ++i)
                {
                    ApplicationEvent* pAppEvent = new ApplicationEvent(
                        ApplicationEvent::TYPE_ACCEPT, *i);
                    ImplPostForeignAppEvent( pAppEvent );
                }
                // handle acceptor removal
                std::vector< OUString > const & unaccept = aCmdLineArgs->
                    GetUnaccept();
                for (std::vector< OUString >::const_iterator i(
                         unaccept.begin());
                     i != unaccept.end(); ++i)
                {
                    ApplicationEvent* pAppEvent = new ApplicationEvent(
                        ApplicationEvent::TYPE_UNACCEPT, *i);
                    ImplPostForeignAppEvent( pAppEvent );
                }

                ProcessDocumentsRequest* pRequest = new ProcessDocumentsRequest(
                    aCmdLineArgs->getCwdUrl());
                cProcessed.reset();
                pRequest->pcProcessed = &cProcessed;

                // Print requests are not dependent on the --invisible cmdline argument as they are
                // loaded with the "hidden" flag! So they are always checked.
                pRequest->aPrintList = aCmdLineArgs->GetPrintList();
                bDocRequestSent |= !pRequest->aPrintList.empty();
                pRequest->aPrintToList = aCmdLineArgs->GetPrintToList();
                pRequest->aPrinterName = aCmdLineArgs->GetPrinterName();
                bDocRequestSent |= !( pRequest->aPrintToList.empty() || pRequest->aPrinterName.isEmpty() );

                if ( !rCurrentCmdLineArgs.IsInvisible() )
                {
                    // Read cmdline args that can open/create documents. As they would open a window
                    // they are only allowed if the "--invisible" is currently not used!
                    pRequest->aOpenList = aCmdLineArgs->GetOpenList();
                    bDocRequestSent |= !pRequest->aOpenList.empty();
                    pRequest->aViewList = aCmdLineArgs->GetViewList();
                    bDocRequestSent |= !pRequest->aViewList.empty();
                    pRequest->aStartList = aCmdLineArgs->GetStartList();
                    bDocRequestSent |= !pRequest->aStartList.empty();
                    pRequest->aForceOpenList = aCmdLineArgs->GetForceOpenList();
                    bDocRequestSent |= !pRequest->aForceOpenList.empty();
                    pRequest->aForceNewList = aCmdLineArgs->GetForceNewList();
                    bDocRequestSent |= !pRequest->aForceNewList.empty();

                    // Special command line args to create an empty document for a given module

                    // #i18338# (lo)
                    // we only do this if no document was specified on the command line,
                    // since this would be inconsistent with the behaviour of
                    // the first process, see OpenClients() (call to OpenDefault()) in app.cxx
                    if ( aCmdLineArgs->HasModuleParam() && !bDocRequestSent )
                    {
                        SvtModuleOptions aOpt;
                        SvtModuleOptions::EFactory eFactory = SvtModuleOptions::EFactory::WRITER;
                        if ( aCmdLineArgs->IsWriter() )
                            eFactory = SvtModuleOptions::EFactory::WRITER;
                        else if ( aCmdLineArgs->IsCalc() )
                            eFactory = SvtModuleOptions::EFactory::CALC;
                        else if ( aCmdLineArgs->IsDraw() )
                            eFactory = SvtModuleOptions::EFactory::DRAW;
                        else if ( aCmdLineArgs->IsImpress() )
                            eFactory = SvtModuleOptions::EFactory::IMPRESS;
                        else if ( aCmdLineArgs->IsBase() )
                            eFactory = SvtModuleOptions::EFactory::DATABASE;
                        else if ( aCmdLineArgs->IsMath() )
                            eFactory = SvtModuleOptions::EFactory::MATH;
                        else if ( aCmdLineArgs->IsGlobal() )
                            eFactory = SvtModuleOptions::EFactory::WRITERGLOBAL;
                        else if ( aCmdLineArgs->IsWeb() )
                            eFactory = SvtModuleOptions::EFactory::WRITERWEB;

                        if ( !pRequest->aOpenList.empty() )
                            pRequest->aModule = aOpt.GetFactoryName( eFactory );
                        else
                            pRequest->aOpenList.push_back( aOpt.GetFactoryEmptyDocumentURL( eFactory ) );
                        bDocRequestSent = true;
                    }
                }

                if ( !aCmdLineArgs->IsQuickstart() ) {
                    bool bShowHelp = false;
                    OUStringBuffer aHelpURLBuffer;
                    if (aCmdLineArgs->IsHelpWriter()) {
                        bShowHelp = true;
                        aHelpURLBuffer.append("vnd.sun.star.help://swriter/start");
                    } else if (aCmdLineArgs->IsHelpCalc()) {
                        bShowHelp = true;
                        aHelpURLBuffer.append("vnd.sun.star.help://scalc/start");
                    } else if (aCmdLineArgs->IsHelpDraw()) {
                        bShowHelp = true;
                        aHelpURLBuffer.append("vnd.sun.star.help://sdraw/start");
                    } else if (aCmdLineArgs->IsHelpImpress()) {
                        bShowHelp = true;
                        aHelpURLBuffer.append("vnd.sun.star.help://simpress/start");
                    } else if (aCmdLineArgs->IsHelpBase()) {
                        bShowHelp = true;
                        aHelpURLBuffer.append("vnd.sun.star.help://sdatabase/start");
                    } else if (aCmdLineArgs->IsHelpBasic()) {
                        bShowHelp = true;
                        aHelpURLBuffer.append("vnd.sun.star.help://sbasic/start");
                    } else if (aCmdLineArgs->IsHelpMath()) {
                        bShowHelp = true;
                        aHelpURLBuffer.append("vnd.sun.star.help://smath/start");
                    }
                    if (bShowHelp) {
                        aHelpURLBuffer.append("?Language=");
                        aHelpURLBuffer.append(utl::ConfigManager::getLocale());
#if defined UNX
                        aHelpURLBuffer.append("&System=UNX");
#elif defined WNT
                        aHelpURLBuffer.appendAscii("&System=WIN");
#endif
                        ApplicationEvent* pAppEvent = new ApplicationEvent(
                            ApplicationEvent::TYPE_OPENHELPURL,
                            aHelpURLBuffer.makeStringAndClear());
                        ImplPostForeignAppEvent( pAppEvent );
                    }
                }

                if ( bDocRequestSent )
                {
                    // Send requests to dispatch watcher if we have at least one. The receiver
                    // is responsible to delete the request after processing it.
                    if ( aCmdLineArgs->HasModuleParam() )
                    {
                        SvtModuleOptions    aOpt;

                        // Support command line parameters to start a module (as preselection)
                        if ( aCmdLineArgs->IsWriter() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
                            pRequest->aModule = aOpt.GetFactoryName( SvtModuleOptions::EFactory::WRITER );
                        else if ( aCmdLineArgs->IsCalc() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
                            pRequest->aModule = aOpt.GetFactoryName( SvtModuleOptions::EFactory::CALC );
                        else if ( aCmdLineArgs->IsImpress() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
                            pRequest->aModule= aOpt.GetFactoryName( SvtModuleOptions::EFactory::IMPRESS );
                        else if ( aCmdLineArgs->IsDraw() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) )
                            pRequest->aModule= aOpt.GetFactoryName( SvtModuleOptions::EFactory::DRAW );
                    }

                    ImplPostProcessDocumentsEvent( pRequest );
                }
                else
                {
                    // delete not used request again
                    delete pRequest;
                    pRequest = NULL;
                }
                if (aArguments.equalsL(RTL_CONSTASCII_STRINGPARAM("-tofront")) ||
                    aCmdLineArgs->IsEmpty())
                {
                    // no document was sent, just bring Office to front
                    ApplicationEvent* pAppEvent =
                        new ApplicationEvent(ApplicationEvent::TYPE_APPEAR);
                    ImplPostForeignAppEvent( pAppEvent );
                }
            }

            // we don't need the mutex any longer...
            aGuard.clear();
            // wait for processing to finish
            if (bDocRequestSent)
                cProcessed.wait();
            // processing finished, inform the requesting end:
            n = aStreamPipe.write(
                PROCESSING_DONE, SAL_N_ELEMENTS(PROCESSING_DONE));
                // incl. terminating NUL
            if (n != SAL_N_ELEMENTS(PROCESSING_DONE)) {
                SAL_WARN("desktop", "short write: " << n);
                continue;
            }
        }
        else
        {
            {
                osl::MutexGuard aGuard( GetMutex() );
                if ( mbDowning )
                {
                    break;
                }
            }

#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
            fprintf( stderr, "Error on accept: %d\n", (int)nError );
#endif
            TimeValue tval;
            tval.Seconds = 1;
            tval.Nanosec = 0;
            salhelper::Thread::wait( tval );
        }
    } while( schedule() );
#endif
}

static void AddToDispatchList(
    DispatchWatcher::DispatchList& rDispatchList,
    boost::optional< OUString > const & cwdUrl,
    std::vector< OUString > const & aRequestList,
    DispatchWatcher::RequestType nType,
    const OUString& aParam,
    const OUString& aFactory )
{
    for (std::vector< OUString >::const_iterator i(aRequestList.begin());
         i != aRequestList.end(); ++i)
    {
        rDispatchList.push_back(
            DispatchWatcher::DispatchRequest( nType, *i, cwdUrl, aParam, aFactory ));
    }
}

static void AddConversionsToDispatchList(
    DispatchWatcher::DispatchList& rDispatchList,
    boost::optional< OUString > const & cwdUrl,
    std::vector< OUString > const & rRequestList,
    const OUString& rParam,
    const OUString& rPrinterName,
    const OUString& rFactory,
    const OUString& rParamOut,
    const bool isTextCat )
{
    DispatchWatcher::RequestType nType;
    OUString aParam( rParam );

    if( !rParam.isEmpty() )
    {
        nType = ( isTextCat ) ? DispatchWatcher::REQUEST_CAT : DispatchWatcher::REQUEST_CONVERSION;
        aParam = rParam;
    }
    else
    {
        nType = DispatchWatcher::REQUEST_BATCHPRINT;
        aParam = rPrinterName;
    }

    OUString aOutDir( rParamOut.trim() );
    OUString aPWD;
    ::tools::getProcessWorkingDir( aPWD );

    if( !::osl::FileBase::getAbsoluteFileURL( aPWD, rParamOut, aOutDir ) )
        ::osl::FileBase::getSystemPathFromFileURL( aOutDir, aOutDir );

    if( !rParamOut.trim().isEmpty() )
    {
        aParam += ";";
        aParam += aOutDir;
    }
    else
    {
        ::osl::FileBase::getSystemPathFromFileURL( aPWD, aPWD );
        aParam += ";" + aPWD;
    }

    for (std::vector< OUString >::const_iterator i(rRequestList.begin());
         i != rRequestList.end(); ++i)
    {
        rDispatchList.push_back(
            DispatchWatcher::DispatchRequest( nType, *i, cwdUrl, aParam, rFactory ));
    }
}


bool OfficeIPCThread::ExecuteCmdLineRequests( ProcessDocumentsRequest& aRequest )
{
    // protect the dispatch list
    osl::ClearableMutexGuard aGuard( GetMutex() );

    static DispatchWatcher::DispatchList    aDispatchList;

    // Create dispatch list for dispatch watcher
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aInFilter, DispatchWatcher::REQUEST_INFILTER, "", aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aOpenList, DispatchWatcher::REQUEST_OPEN, "", aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aViewList, DispatchWatcher::REQUEST_VIEW, "", aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aStartList, DispatchWatcher::REQUEST_START, "", aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aPrintList, DispatchWatcher::REQUEST_PRINT, "", aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aPrintToList, DispatchWatcher::REQUEST_PRINTTO, aRequest.aPrinterName, aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aForceOpenList, DispatchWatcher::REQUEST_FORCEOPEN, "", aRequest.aModule );
    AddToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aForceNewList, DispatchWatcher::REQUEST_FORCENEW, "", aRequest.aModule );
    AddConversionsToDispatchList( aDispatchList, aRequest.aCwdUrl, aRequest.aConversionList, aRequest.aConversionParams, aRequest.aPrinterName, aRequest.aModule, aRequest.aConversionOut, aRequest.bTextCat );
    bool bShutdown( false );

    if ( pGlobalOfficeIPCThread.is() )
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
