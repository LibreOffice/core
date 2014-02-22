/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <config_features.h>

#include "app.hxx"
#include "officeipcthread.hxx"
#include "cmdlineargs.hxx"
#include "dispatchwatcher.hxx"
#include <boost/scoped_ptr.hpp>
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
#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <rtl/process.h>
#include "tools/getprocessworkingdir.hxx"

using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;


const char  *OfficeIPCThread::sc_aShowSequence = "-tofront";
const int OfficeIPCThread::sc_nShSeqLength = 5;

namespace {

#if HAVE_FEATURE_DESKTOP

static char const ARGUMENT_PREFIX[] = "InternalIPC::Arguments";
static char const SEND_ARGUMENTS[] = "InternalIPC::SendArguments";
static char const PROCESSING_DONE[] = "InternalIPC::ProcessingDone";





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
            
        if (end) {
            return str.makeStringAndClear();
        }
    }
}

#endif

}


enum PipeMode
{
    PIPEMODE_DONTKNOW,
    PIPEMODE_CREATED,
    PIPEMODE_CONNECTED
};

namespace desktop
{

namespace {

#if HAVE_FEATURE_DESKTOP

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

    virtual boost::optional< OUString > getCwdUrl() { return m_cwdUrl; }

    virtual bool next(OUString * argument) { return next(argument, true); }

private:
    virtual bool next(OUString * argument, bool prefix) {
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



OUString CreateMD5FromString( const OUString& aMsg )
{
#if (OSL_DEBUG_LEVEL > 2)
    fprintf( stderr, "create md5 from '%s'\n",
             OUStringToOString (aMsg, RTL_TEXTENCODING_UTF8).getStr() );
#endif

    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    if ( handle )
    {
        const sal_uInt8* pData = (const sal_uInt8*)aMsg.getStr();
        sal_uInt32       nSize = ( aMsg.getLength() * sizeof( sal_Unicode ));
        sal_uInt32       nMD5KeyLen = rtl_digest_queryLength( handle );
        sal_uInt8*       pMD5KeyBuffer = new sal_uInt8[ nMD5KeyLen ];

        rtl_digest_init( handle, pData, nSize );
        rtl_digest_update( handle, pData, nSize );
        rtl_digest_get( handle, pMD5KeyBuffer, nMD5KeyLen );
        rtl_digest_destroy( handle );

        
        OUStringBuffer aBuffer( nMD5KeyLen * 2 + 1 );
        for ( sal_uInt32 i = 0; i < nMD5KeyLen; i++ )
            aBuffer.append( (sal_Int32)pMD5KeyBuffer[i], 16 );

        delete [] pMD5KeyBuffer;
        return aBuffer.makeStringAndClear();
    }

    return OUString();
}

class ProcessEventsClass_Impl
{
public:
    DECL_STATIC_LINK( ProcessEventsClass_Impl, CallEvent, void* pEvent );
    DECL_STATIC_LINK( ProcessEventsClass_Impl, ProcessDocumentsEvent, void* pEvent );
};

IMPL_STATIC_LINK_NOINSTANCE( ProcessEventsClass_Impl, CallEvent, void*, pEvent )
{
    
    Desktop::HandleAppEvent( *((ApplicationEvent*)pEvent) );
    delete (ApplicationEvent*)pEvent;
    return 0;
}

IMPL_STATIC_LINK_NOINSTANCE( ProcessEventsClass_Impl, ProcessDocumentsEvent, void*, pEvent )
{
    
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
        OfficeIPCThread::DisableOfficeIPCThread(false);
    return osl_Signal_ActCallNextHdl;
}













OUString SAL_CALL OfficeIPCThreadController::getImplementationName()
throw ( RuntimeException )
{
    return OUString( "com.sun.star.comp.OfficeIPCThreadController" );
}

sal_Bool OfficeIPCThreadController::supportsService(
    OUString const & ServiceName) throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL OfficeIPCThreadController::getSupportedServiceNames()
throw ( RuntimeException )
{
    Sequence< OUString > aSeq( 0 );
    return aSeq;
}


void SAL_CALL OfficeIPCThreadController::disposing( const EventObject& )
throw( RuntimeException )
{
}


void SAL_CALL OfficeIPCThreadController::queryTermination( const EventObject& )
throw( TerminationVetoException, RuntimeException )
{
    
    
    

    if ( OfficeIPCThread::AreRequestsPending() )
        throw TerminationVetoException();
    else
        OfficeIPCThread::SetDowning();
}

void SAL_CALL OfficeIPCThreadController::notifyTermination( const EventObject& )
throw( RuntimeException )
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
    
    
    
    ::osl::MutexGuard   aGuard( GetMutex() );

    if ( pGlobalOfficeIPCThread.is() )
        pGlobalOfficeIPCThread->mbDowning = true;
}

static bool s_bInEnableRequests = false;

void OfficeIPCThread::EnableRequests( bool i_bEnable )
{
    
    ::osl::MutexGuard   aGuard( GetMutex() );

    if ( pGlobalOfficeIPCThread.is() )
    {
        s_bInEnableRequests = true;
        pGlobalOfficeIPCThread->mbRequestsEnabled = i_bEnable;
        if( i_bEnable )
        {
            
            ProcessDocumentsRequest aEmptyReq = ProcessDocumentsRequest( boost::optional< OUString >() );
            
            OfficeIPCThread::ExecuteCmdLineRequests( aEmptyReq );
        }
        s_bInEnableRequests = false;
    }
}

sal_Bool OfficeIPCThread::AreRequestsPending()
{
    
    ::osl::MutexGuard   aGuard( GetMutex() );
    if ( pGlobalOfficeIPCThread.is() )
        return ( pGlobalOfficeIPCThread->mnPendingRequests > 0 );
    else
        return sal_False;
}

void OfficeIPCThread::RequestsCompleted( int nCount )
{
    
    ::osl::MutexGuard   aGuard( GetMutex() );
    if ( pGlobalOfficeIPCThread.is() )
    {
        if ( pGlobalOfficeIPCThread->mnPendingRequests > 0 )
            pGlobalOfficeIPCThread->mnPendingRequests -= nCount;
    }
}

OfficeIPCThread::Status OfficeIPCThread::EnableOfficeIPCThread()
{
#if HAVE_FEATURE_DESKTOP
    ::osl::MutexGuard   aGuard( GetMutex() );

    if( pGlobalOfficeIPCThread.is() )
        return IPC_STATUS_OK;

    OUString aUserInstallPath;
    OUString aDummy;

    rtl::Reference< OfficeIPCThread > pThread(new OfficeIPCThread);

    
    
    ::utl::Bootstrap::PathStatus aLocateResult = ::utl::Bootstrap::locateUserInstallation( aUserInstallPath );
    if ( aLocateResult == ::utl::Bootstrap::PATH_EXISTS || aLocateResult == ::utl::Bootstrap::PATH_VALID)
        aDummy = aUserInstallPath;
    else
    {
        return IPC_STATUS_BOOTSTRAP_ERROR;
    }

    
    
    
    
    

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
        sprintf( szBuffer, "%d", SUPD );
        aUserInstallPathHashCode = OUString( szBuffer, strlen(szBuffer), osl_getThreadTextEncoding() );
    }
    else
        aUserInstallPathHashCode = CreateMD5FromString( aDummy );


    
    if ( aUserInstallPathHashCode.isEmpty() )
        return IPC_STATUS_BOOTSTRAP_ERROR; 

    OUString aPipeIdent( "SingleOfficeIPC_" + aUserInstallPathHashCode );

    PipeMode nPipeMode = PIPEMODE_DONTKNOW;
    do
    {
        osl::Security &rSecurity = Security::get();

        
        if ( pThread->maPipe.create( aPipeIdent.getStr(), osl_Pipe_CREATE, rSecurity ))
        {
            
            nPipeMode = PIPEMODE_CREATED;
        }
        else if( pThread->maPipe.create( aPipeIdent.getStr(), osl_Pipe_OPEN, rSecurity )) 
        {
            osl::StreamPipe aStreamPipe(pThread->maPipe.getHandle());
            if (readStringFromPipe(aStreamPipe) == SEND_ARGUMENTS)
            {
                
                nPipeMode = PIPEMODE_CONNECTED;
            }
            else
            {
                
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

            
            TimeValue aTimeValue;
            aTimeValue.Seconds = 0;
            aTimeValue.Nanosec = 10000000; 
            salhelper::Thread::wait( aTimeValue );
        }

    } while ( nPipeMode == PIPEMODE_DONTKNOW );

    if ( nPipeMode == PIPEMODE_CREATED )
    {
        
        pGlobalOfficeIPCThread = pThread;
        pThread->launch();
    }
    else
    {
        
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
        
        sal_Int32 n = aStreamPipe.write(
            aArguments.getStr(), aArguments.getLength());
        if (n != aArguments.getLength()) {
            SAL_INFO("desktop", "short write: " << n);
            return IPC_STATUS_BOOTSTRAP_ERROR;
        }

        if (readStringFromPipe(aStreamPipe) != PROCESSING_DONE)
        {
            
            return IPC_STATUS_BOOTSTRAP_ERROR;
        }

        return IPC_STATUS_2ND_OFFICE;
    }
#else
    pGlobalOfficeIPCThread = rtl::Reference< OfficeIPCThread >(new OfficeIPCThread);
#endif
    return IPC_STATUS_OK;
}

void OfficeIPCThread::DisableOfficeIPCThread(bool join)
{
#if HAVE_FEATURE_DESKTOP
    osl::ClearableMutexGuard aMutex( GetMutex() );

    if( pGlobalOfficeIPCThread.is() )
    {
        rtl::Reference< OfficeIPCThread > pOfficeIPCThread(
            pGlobalOfficeIPCThread);
        pGlobalOfficeIPCThread.clear();

        pOfficeIPCThread->mbDowning = true;
        pOfficeIPCThread->maPipe.close();

        
        aMutex.clear();

        OfficeIPCThread::SetReady(pOfficeIPCThread);

        
        if (join)
        {
            pOfficeIPCThread->join();
        }
    }
#else
    (void) join;
#endif
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

void OfficeIPCThread::execute()
{
#if HAVE_FEATURE_DESKTOP
    do
    {
        osl::StreamPipe aStreamPipe;
        oslPipeError nError = maPipe.accept( aStreamPipe );


        if( nError == osl_Pipe_E_None )
        {
            
            
            
            
            cReady.wait();

            
            if (!pGlobalOfficeIPCThread.is()) return;

            
            
            osl::ClearableMutexGuard aGuard( GetMutex() );

            if ( mbDowning )
            {
                break;
            }

            
            sal_Int32 n = aStreamPipe.write(
                SEND_ARGUMENTS, SAL_N_ELEMENTS(SEND_ARGUMENTS));
                
            if (n != SAL_N_ELEMENTS(SEND_ARGUMENTS)) {
                SAL_WARN("desktop", "short write: " << n);
                continue;
            }

            OString aArguments = readStringFromPipe(aStreamPipe);

            
            if (aArguments.isEmpty())
                continue;

            boost::scoped_ptr< CommandLineArgs > aCmdLineArgs;
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
                    
                    ApplicationEvent* pAppEvent =
                        new ApplicationEvent(ApplicationEvent::TYPE_QUICKSTART);
                    ImplPostForeignAppEvent( pAppEvent );
                }

                
                std::vector< OUString > const & accept = aCmdLineArgs->
                    GetAccept();
                for (std::vector< OUString >::const_iterator i(accept.begin());
                     i != accept.end(); ++i)
                {
                    ApplicationEvent* pAppEvent = new ApplicationEvent(
                        ApplicationEvent::TYPE_ACCEPT, *i);
                    ImplPostForeignAppEvent( pAppEvent );
                }
                
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

                
                
                pRequest->aPrintList = aCmdLineArgs->GetPrintList();
                bDocRequestSent |= !pRequest->aPrintList.empty();
                pRequest->aPrintToList = aCmdLineArgs->GetPrintToList();
                pRequest->aPrinterName = aCmdLineArgs->GetPrinterName();
                bDocRequestSent |= !( pRequest->aPrintToList.empty() || pRequest->aPrinterName.isEmpty() );

                if ( !rCurrentCmdLineArgs.IsInvisible() )
                {
                    
                    
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

                    

                    
                    
                    
                    
                    if ( aCmdLineArgs->HasModuleParam() && !bDocRequestSent )
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

                        if ( !pRequest->aOpenList.empty() )
                            pRequest->aModule = aOpt.GetFactoryName( eFactory );
                        else
                            pRequest->aOpenList.push_back( aOpt.GetFactoryEmptyDocumentURL( eFactory ) );
                        bDocRequestSent = true;
                    }
                }

                if ( !aCmdLineArgs->IsQuickstart() ) {
                    sal_Bool bShowHelp = sal_False;
                    OUStringBuffer aHelpURLBuffer;
                    if (aCmdLineArgs->IsHelpWriter()) {
                        bShowHelp = sal_True;
                        aHelpURLBuffer.appendAscii("vnd.sun.star.help:
                    } else if (aCmdLineArgs->IsHelpCalc()) {
                        bShowHelp = sal_True;
                        aHelpURLBuffer.appendAscii("vnd.sun.star.help:
                    } else if (aCmdLineArgs->IsHelpDraw()) {
                        bShowHelp = sal_True;
                        aHelpURLBuffer.appendAscii("vnd.sun.star.help:
                    } else if (aCmdLineArgs->IsHelpImpress()) {
                        bShowHelp = sal_True;
                        aHelpURLBuffer.appendAscii("vnd.sun.star.help:
                    } else if (aCmdLineArgs->IsHelpBase()) {
                        bShowHelp = sal_True;
                        aHelpURLBuffer.appendAscii("vnd.sun.star.help:
                    } else if (aCmdLineArgs->IsHelpBasic()) {
                        bShowHelp = sal_True;
                        aHelpURLBuffer.appendAscii("vnd.sun.star.help:
                    } else if (aCmdLineArgs->IsHelpMath()) {
                        bShowHelp = sal_True;
                        aHelpURLBuffer.appendAscii("vnd.sun.star.help:
                    }
                    if (bShowHelp) {
                        aHelpURLBuffer.appendAscii("?Language=");
                        aHelpURLBuffer.append(utl::ConfigManager::getLocale());
#if defined UNX
                        aHelpURLBuffer.appendAscii("&System=UNX");
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
                    
                    
                    if ( aCmdLineArgs->HasModuleParam() )
                    {
                        SvtModuleOptions    aOpt;

                        
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
                    
                    delete pRequest;
                    pRequest = NULL;
                }
                if (aArguments.equalsL(sc_aShowSequence, sc_nShSeqLength) ||
                    aCmdLineArgs->IsEmpty())
                {
                    
                    ApplicationEvent* pAppEvent =
                        new ApplicationEvent(ApplicationEvent::TYPE_APPEAR);
                    ImplPostForeignAppEvent( pAppEvent );
                }
            }

            
            aGuard.clear();
            
            if (bDocRequestSent)
                cProcessed.wait();
            
            n = aStreamPipe.write(
                PROCESSING_DONE, SAL_N_ELEMENTS(PROCESSING_DONE));
                
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
    const OUString& rParamOut )
{
    DispatchWatcher::RequestType nType;
    OUString aParam( rParam );

    if( !rParam.isEmpty() )
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


sal_Bool OfficeIPCThread::ExecuteCmdLineRequests( ProcessDocumentsRequest& aRequest )
{
    
    osl::ClearableMutexGuard aGuard( GetMutex() );

    static DispatchWatcher::DispatchList    aDispatchList;

    OUString aEmpty;
    
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

        
        DispatchWatcher::DispatchList aTempList( aDispatchList );
        aDispatchList.clear();

        aGuard.clear();

        
        bShutdown = pGlobalOfficeIPCThread->mpDispatchWatcher->executeDispatchRequests( aTempList, s_bInEnableRequests );

        
        if (aRequest.pcProcessed != NULL)
            aRequest.pcProcessed->set();
    }

    return bShutdown;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
