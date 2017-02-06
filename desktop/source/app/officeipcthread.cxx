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

#include <sal/config.h>

#include <config_dbus.h>
#include <config_features.h>

#include "app.hxx"
#include "officeipcthread.hxx"
#include "cmdlineargs.hxx"
#include "dispatchwatcher.hxx"
#include <stdio.h>
#include <com/sun/star/frame/TerminationVetoException.hpp>
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
#include <rtl/strbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <rtl/process.h>
#include <tools/getprocessworkingdir.hxx>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <memory>

#if ENABLE_DBUS
#include <dbus/dbus.h>
#endif

using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;

namespace {

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
            SAL_INFO("desktop.app", "read empty string");
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
            auto s = str.makeStringAndClear();
            SAL_INFO("desktop.app", "read <" << s << ">");
            return s;
        }
    }
}

}

namespace desktop
{

namespace {

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

    virtual boost::optional< OUString > getCwdUrl() override { return m_cwdUrl; }

    virtual bool next(OUString * argument) override { return next(argument, true); }

private:
    bool next(OUString * argument, bool prefix) {
        OSL_ASSERT(argument != nullptr);
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

}

rtl::Reference< RequestHandler > RequestHandler::pGlobal;

// Turns a string in aMsg such as file:///home/foo/.libreoffice/3
// Into a hex string of well known length ff132a86...
OUString CreateMD5FromString( const OUString& aMsg )
{
    SAL_INFO("desktop.app", "create md5 from '" << aMsg << "'");

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
    DECL_STATIC_LINK( ProcessEventsClass_Impl, CallEvent, void*, void );
    DECL_STATIC_LINK( ProcessEventsClass_Impl, ProcessDocumentsEvent, void*, void );
};

IMPL_STATIC_LINK( ProcessEventsClass_Impl, CallEvent, void*, pEvent, void )
{
    // Application events are processed by the Desktop::HandleAppEvent implementation.
    Desktop::HandleAppEvent( *static_cast<ApplicationEvent*>(pEvent) );
    delete static_cast<ApplicationEvent*>(pEvent);
}

IMPL_STATIC_LINK( ProcessEventsClass_Impl, ProcessDocumentsEvent, void*, pEvent, void )
{
    // Documents requests are processed by the RequestHandler implementation
    ProcessDocumentsRequest* pDocsRequest = static_cast<ProcessDocumentsRequest*>(pEvent);
    RequestHandler::ExecuteCmdLineRequests(*pDocsRequest, false);
    delete pDocsRequest;
}

void ImplPostForeignAppEvent( ApplicationEvent* pEvent )
{
    Application::PostUserEvent( LINK( nullptr, ProcessEventsClass_Impl, CallEvent ), pEvent );
}

void ImplPostProcessDocumentsEvent( ProcessDocumentsRequest* pEvent )
{
    Application::PostUserEvent( LINK( nullptr, ProcessEventsClass_Impl, ProcessDocumentsEvent ), pEvent );
}

oslSignalAction SAL_CALL SalMainPipeExchangeSignal_impl(void* /*pData*/, oslSignalInfo* pInfo)
{
    if( pInfo->Signal == osl_Signal_Terminate )
        RequestHandler::SetDowning();
    return osl_Signal_ActCallNextHdl;
}


// The RequestHandlerController implementation is a bookkeeper for all pending requests
// that were created by the RequestHandler. The requests are waiting to be processed by
// our framework loadComponentFromURL function (e.g. open/print request).
// During shutdown the framework is asking RequestHandlerController about pending requests.
// If there are pending requests framework has to stop the shutdown process. It is waiting
// for these requests because framework is not able to handle shutdown and open a document
// concurrently.


// XServiceInfo
OUString SAL_CALL RequestHandlerController::getImplementationName()
{
    return OUString( "com.sun.star.comp.RequestHandlerController" );
}

sal_Bool RequestHandlerController::supportsService(
    OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL RequestHandlerController::getSupportedServiceNames()
{
    Sequence< OUString > aSeq( 0 );
    return aSeq;
}

// XEventListener
void SAL_CALL RequestHandlerController::disposing( const EventObject& )
{
}

// XTerminateListener
void SAL_CALL RequestHandlerController::queryTermination( const EventObject& )
{
    // Desktop ask about pending request through our office ipc pipe. We have to
    // be sure that no pending request is waiting because framework is not able to
    // handle shutdown and open a document concurrently.

    if ( RequestHandler::AreRequestsPending() )
        throw TerminationVetoException();
    else
        RequestHandler::SetDowning();
}

void SAL_CALL RequestHandlerController::notifyTermination( const EventObject& )
{
}

class IpcThread: public salhelper::Thread {
public:
    void start(RequestHandler * handler) {
        m_handler = handler;
        launch();
    }

    virtual void close() = 0;

protected:
    explicit IpcThread(char const * name): Thread(name), m_handler(nullptr) {}

    virtual ~IpcThread() override {}

    bool process(OString const & arguments, bool * waitProcessed);

    RequestHandler * m_handler;
};

class PipeIpcThread: public IpcThread {
public:
    static RequestHandler::Status enable(rtl::Reference<IpcThread> * thread);

private:
    explicit PipeIpcThread(osl::Pipe const & pipe):
        IpcThread("PipeIPC"), pipe_(pipe)
    {}

    virtual ~PipeIpcThread() override {}

    void execute() override;

    void close() override { pipe_.close(); }

    osl::Pipe pipe_;
};

#if ENABLE_DBUS

namespace {

struct DbusConnectionHolder {
    explicit DbusConnectionHolder(DBusConnection * theConnection):
        connection(theConnection)
    {}

    DbusConnectionHolder(DbusConnectionHolder && other): connection(nullptr)
    { std::swap(connection, other.connection); }

    ~DbusConnectionHolder() {
        if (connection != nullptr) {
            dbus_connection_close(connection);
            dbus_connection_unref(connection);
        }
    }

    DBusConnection * connection;
};

struct DbusMessageHolder {
    explicit DbusMessageHolder(DBusMessage * theMessage): message(theMessage) {}

    ~DbusMessageHolder() { clear(); }

    void clear() {
        if (message != nullptr) {
            dbus_message_unref(message);
        }
        message = nullptr;
    }

    DBusMessage * message;

private:
    DbusMessageHolder(DbusMessageHolder &) = delete;
    void operator =(DbusMessageHolder) = delete;
};

}

class DbusIpcThread: public IpcThread {
public:
    static RequestHandler::Status enable(rtl::Reference<IpcThread> * thread);

private:
    explicit DbusIpcThread(DbusConnectionHolder && connection):
        IpcThread("DbusIPC"), connection_(std::move(connection))
    {}

    virtual ~DbusIpcThread() override {}

    void execute() override;

    void close() override;

    DbusConnectionHolder connection_;
};

RequestHandler::Status DbusIpcThread::enable(rtl::Reference<IpcThread> * thread)
{
    assert(thread != nullptr);
    if (!dbus_threads_init_default()) {
        SAL_WARN("desktop.app", "dbus_threads_init_default failed");
        return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR;
    }
    DBusError e;
    dbus_error_init(&e);
    DbusConnectionHolder con(dbus_bus_get_private(DBUS_BUS_SESSION, &e));
    assert((con.connection == nullptr) == bool(dbus_error_is_set(&e)));
    if (con.connection == nullptr) {
        SAL_WARN(
            "desktop.app",
            "dbus_bus_get_private failed with: " << e.name << ": "
                << e.message);
        dbus_error_free(&e);
        return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR;
    }
    for (;;) {
        int n = dbus_bus_request_name(
            con.connection, "org.libreoffice.LibreOfficeIpc0",
            DBUS_NAME_FLAG_DO_NOT_QUEUE, &e);
        assert((n == -1) == bool(dbus_error_is_set(&e)));
        switch (n) {
        case -1:
            SAL_WARN(
                "desktop.app",
                "dbus_bus_request_name failed with: " << e.name << ": "
                << e.message);
            dbus_error_free(&e);
            return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR;
        case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER:
            *thread = new DbusIpcThread(std::move(con));
            return RequestHandler::IPC_STATUS_OK;
        case DBUS_REQUEST_NAME_REPLY_EXISTS:
            {
                OStringBuffer buf(ARGUMENT_PREFIX);
                OUString arg;
                if (!(tools::getProcessWorkingDir(arg)
                      && addArgument(buf, '1', arg)))
                {
                    buf.append('0');
                }
                sal_uInt32 narg = rtl_getAppCommandArgCount();
                for (sal_uInt32 i = 0; i != narg; ++i) {
                    rtl_getAppCommandArg(i, &arg.pData);
                    if (!addArgument(buf, ',', arg)) {
                        return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR;
                    }
                }
                char const * argstr = buf.getStr();
                DbusMessageHolder msg(
                    dbus_message_new_method_call(
                        "org.libreoffice.LibreOfficeIpc0",
                        "/org/libreoffice/LibreOfficeIpc0",
                        "org.libreoffice.LibreOfficeIpcIfc0", "Execute"));
                if (msg.message == nullptr) {
                    SAL_WARN(
                        "desktop.app", "dbus_message_new_method_call failed");
                    return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR;
                }
                DBusMessageIter it;
                dbus_message_iter_init_append(msg.message, &it);
                if (!dbus_message_iter_append_basic(
                        &it, DBUS_TYPE_STRING, &argstr))
                {
                    SAL_WARN(
                        "desktop.app", "dbus_message_iter_append_basic failed");
                    return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR;
                }
                DbusMessageHolder repl(
                    dbus_connection_send_with_reply_and_block(
                        con.connection, msg.message, 0x7FFFFFFF, &e));
                assert(
                    (repl.message == nullptr) == bool(dbus_error_is_set(&e)));
                if (repl.message == nullptr) {
                    SAL_INFO(
                        "desktop.app",
                        "dbus_connection_send_with_reply_and_block failed"
                            " with: " << e.name << ": " << e.message);
                    dbus_error_free(&e);
                    break;
                }
                return RequestHandler::IPC_STATUS_2ND_OFFICE;
            }
        case DBUS_REQUEST_NAME_REPLY_IN_QUEUE:
        case DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER:
            SAL_WARN(
                "desktop.app",
                "dbus_bus_request_name failed with unexpected " << +n);
            return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR;
        default:
            for (;;) std::abort();
        }
    }
}

void DbusIpcThread::execute()
{
    assert(m_handler != nullptr);
    m_handler->cReady.wait();
    for (;;) {
        {
            osl::MutexGuard g(RequestHandler::GetMutex());
            if (m_handler->mState == RequestHandler::State::Downing) {
                break;
            }
        }
        dbus_connection_read_write_dispatch(connection_.connection, -1);
        DbusMessageHolder msg(
            dbus_connection_pop_message(connection_.connection));
        if (msg.message == nullptr) {
            continue;
        }
        if (dbus_message_is_method_call(
                msg.message, "org.libreoffice.LibreOfficeIpcIfc0", "Close"))
        {
            break;
        }
        if (!dbus_message_is_method_call(
                msg.message, "org.libreoffice.LibreOfficeIpcIfc0", "Execute"))
        {
            SAL_INFO("desktop.app", "unknown DBus message ignored");
            continue;
        }
        DBusMessageIter it;
        if (!dbus_message_iter_init(msg.message, &it)) {
            SAL_WARN("desktop.app", "DBus message without argument ignored");
            continue;
        }
        if (dbus_message_iter_get_arg_type(&it) != DBUS_TYPE_STRING) {
            SAL_WARN(
                "desktop.app", "DBus message with non-string argument ignored");
            continue;
        }
        char const * argstr;
        dbus_message_iter_get_basic(&it, &argstr);
        bool waitProcessed = false;
        {
            osl::MutexGuard g(RequestHandler::GetMutex());
            if (!process(argstr, &waitProcessed)) {
                continue;
            }
        }
        if (waitProcessed) {
            m_handler->cProcessed.wait();
        }
        DbusMessageHolder repl(dbus_message_new_method_return(msg.message));
        if (repl.message == nullptr) {
            SAL_WARN("desktop.app", "dbus_message_new_method_return failed");
            continue;
        }
        dbus_uint32_t serial = 0;
        if (!dbus_connection_send(
                connection_.connection, repl.message, &serial)) {
            SAL_WARN("desktop.app", "dbus_connection_send failed");
            continue;
        }
        dbus_connection_flush(connection_.connection);
    }
}

void DbusIpcThread::close() {
    assert(connection_.connection != nullptr);
    DBusError e;
    dbus_error_init(&e);
    {
        // Let DbusIpcThread::execute return from dbus_connection_read_write;
        // for now, just abort on failure (the process would otherwise block,
        // with DbusIpcThread::execute hanging in dbus_connection_read_write);
        // this apparently needs a more DBus-y design anyway:
        DbusConnectionHolder con(dbus_bus_get_private(DBUS_BUS_SESSION, &e));
        assert((con.connection == nullptr) == bool(dbus_error_is_set(&e)));
        if (con.connection == nullptr) {
            SAL_WARN(
                "desktop.app",
                "dbus_bus_get_private failed with: " << e.name << ": "
                    << e.message);
            dbus_error_free(&e);
            std::abort();
        }
        DbusMessageHolder msg(
            dbus_message_new_method_call(
                "org.libreoffice.LibreOfficeIpc0",
                "/org/libreoffice/LibreOfficeIpc0",
                "org.libreoffice.LibreOfficeIpcIfc0", "Close"));
        if (msg.message == nullptr) {
            SAL_WARN("desktop.app", "dbus_message_new_method_call failed");
            std::abort();
        }
        if (!dbus_connection_send(con.connection, msg.message, nullptr))
        {
            SAL_WARN("desktop.app", "dbus_connection_send failed");
            std::abort();
        }
        dbus_connection_flush(con.connection);
    }
    int n = dbus_bus_release_name(
        connection_.connection, "org.libreoffice.LibreOfficeIpc0", &e);
    assert((n == -1) == bool(dbus_error_is_set(&e)));
    switch (n) {
    case -1:
        SAL_WARN(
            "desktop.app",
            "dbus_bus_release_name failed with: " << e.name << ": "
                << e.message);
        dbus_error_free(&e);
        break;
    case DBUS_RELEASE_NAME_REPLY_RELEASED:
        break;
    case DBUS_RELEASE_NAME_REPLY_NOT_OWNER:
    case DBUS_RELEASE_NAME_REPLY_NON_EXISTENT:
        SAL_WARN(
            "desktop.app",
            "dbus_bus_release_name failed with unexpected " << +n);
        break;
    default:
        for (;;) std::abort();
    }
}

#endif

namespace
{
    class theRequestHandlerMutex
        : public rtl::Static<osl::Mutex, theRequestHandlerMutex> {};
}

::osl::Mutex& RequestHandler::GetMutex()
{
    return theRequestHandlerMutex::get();
}

void RequestHandler::SetDowning()
{
    // We have the order to block all incoming requests. Framework
    // wants to shutdown and we have to make sure that no loading/printing
    // requests are executed anymore.
    ::osl::MutexGuard   aGuard( GetMutex() );

    if ( pGlobal.is() )
        pGlobal->mState = State::Downing;
}

void RequestHandler::EnableRequests()
{
    // switch between just queueing the requests and executing them
    ::osl::MutexGuard   aGuard( GetMutex() );

    if ( pGlobal.is() )
    {
        if (pGlobal->mState != State::Downing) {
            pGlobal->mState = State::RequestsEnabled;
        }
        // hit the compiler over the head
        ProcessDocumentsRequest aEmptyReq = ProcessDocumentsRequest( boost::optional< OUString >() );
        // trigger already queued requests
        RequestHandler::ExecuteCmdLineRequests(aEmptyReq, true);
    }
}

bool RequestHandler::AreRequestsPending()
{
    // Give info about pending requests
    ::osl::MutexGuard   aGuard( GetMutex() );
    if ( pGlobal.is() )
        return ( pGlobal->mnPendingRequests > 0 );
    else
        return false;
}

void RequestHandler::RequestsCompleted()
{
    // Remove nCount pending requests from our internal counter
    ::osl::MutexGuard   aGuard( GetMutex() );
    if ( pGlobal.is() )
    {
        if ( pGlobal->mnPendingRequests > 0 )
            pGlobal->mnPendingRequests --;
    }
}

RequestHandler::Status RequestHandler::Enable(bool ipc)
{
    ::osl::MutexGuard   aGuard( GetMutex() );

    if( pGlobal.is() )
        return IPC_STATUS_OK;

#if !HAVE_FEATURE_DESKTOP || HAVE_FEATURE_MACOSX_SANDBOX
    ipc = false;
#endif

    if (!ipc) {
        pGlobal = new RequestHandler;
        return IPC_STATUS_OK;
    }

    enum class Kind { Pipe, Dbus };
    Kind kind = Kind::Pipe;
#if ENABLE_DBUS
    if (std::getenv("LIBO_FLATPAK") != nullptr) {
        kind = Kind::Dbus;
    }
#endif
    rtl::Reference<IpcThread> thread;
    Status stat = Status(); // silence bogus potentially-uninitialized warnings
    switch (kind) {
    case Kind::Pipe:
        stat = PipeIpcThread::enable(&thread);
        break;
    case Kind::Dbus:
#if ENABLE_DBUS
        stat = DbusIpcThread::enable(&thread);
        break;
#endif
    default:
        assert(false);
    }
    assert(thread.is() == (stat == IPC_STATUS_OK));
    if (stat == IPC_STATUS_OK) {
        pGlobal = new RequestHandler;
        pGlobal->mIpcThread = thread;
        pGlobal->mIpcThread->start(pGlobal.get());
    }
    return stat;
}

RequestHandler::Status PipeIpcThread::enable(rtl::Reference<IpcThread> * thread)
{
    assert(thread != nullptr);

    // The name of the named pipe is created with the hashcode of the user installation directory (without /user). We have to retrieve
    // this information from a unotools implementation.
    OUString aUserInstallPath;
    ::utl::Bootstrap::PathStatus aLocateResult = ::utl::Bootstrap::locateUserInstallation( aUserInstallPath );
    if (aLocateResult != utl::Bootstrap::PATH_EXISTS
        && aLocateResult != utl::Bootstrap::PATH_VALID)
    {
        return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR;
    }

    // Try to  determine if we are the first office or not! This should prevent multiple
    // access to the user directory !
    // First we try to create our pipe if this fails we try to connect. We have to do this
    // in a loop because the other office can crash or shutdown between createPipe
    // and connectPipe!!
    auto aUserInstallPathHashCode = CreateMD5FromString(aUserInstallPath);

    // Check result to create a hash code from the user install path
    if ( aUserInstallPathHashCode.isEmpty() )
        return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR; // Something completely broken, we cannot create a valid hash code!

    osl::Pipe pipe;
    enum PipeMode
    {
        PIPEMODE_DONTKNOW,
        PIPEMODE_CREATED,
        PIPEMODE_CONNECTED
    };
    PipeMode nPipeMode = PIPEMODE_DONTKNOW;

    OUString aPipeIdent( "SingleOfficeIPC_" + aUserInstallPathHashCode );
    do
    {
        osl::Security security;

        // Try to create pipe
        if ( pipe.create( aPipeIdent.getStr(), osl_Pipe_CREATE, security ))
        {
            // Pipe created
            nPipeMode = PIPEMODE_CREATED;
        }
        else if( pipe.create( aPipeIdent.getStr(), osl_Pipe_OPEN, security )) // Creation not successful, now we try to connect
        {
            osl::StreamPipe aStreamPipe(pipe.getHandle());
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
            oslPipeError eReason = pipe.getError();
            if ((eReason == osl_Pipe_E_ConnectionRefused) || (eReason == osl_Pipe_E_invalidError))
                return RequestHandler::IPC_STATUS_PIPE_ERROR;

            // Wait for second office to be ready
            TimeValue aTimeValue;
            aTimeValue.Seconds = 0;
            aTimeValue.Nanosec = 10000000; // 10ms
            salhelper::Thread::wait( aTimeValue );
        }

    } while ( nPipeMode == PIPEMODE_DONTKNOW );

    if ( nPipeMode == PIPEMODE_CREATED )
    {
        // Seems we are the one and only, so create listening thread
        *thread = new PipeIpcThread(pipe);
        return RequestHandler::IPC_STATUS_OK;
    }
    else
    {
        // Seems another office is running. Pipe arguments to it and self terminate
        osl::StreamPipe aStreamPipe(pipe.getHandle());

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
            rtl_getAppCommandArg( i, &aUserInstallPath.pData );
            if (!addArgument(aArguments, ',', aUserInstallPath)) {
                return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR;
            }
        }
        aArguments.append('\0');
        // finally, write the string onto the pipe
        SAL_INFO("desktop.app", "writing <" << aArguments.getStr() << ">");
        sal_Int32 n = aStreamPipe.write(
            aArguments.getStr(), aArguments.getLength());
        if (n != aArguments.getLength()) {
            SAL_INFO("desktop.app", "short write: " << n);
            return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR;
        }

        if (readStringFromPipe(aStreamPipe) != PROCESSING_DONE)
        {
            // something went wrong
            return RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR;
        }

        return RequestHandler::IPC_STATUS_2ND_OFFICE;
    }
}

void RequestHandler::Disable()
{
    osl::ClearableMutexGuard aMutex( GetMutex() );

    if( pGlobal.is() )
    {
        rtl::Reference< RequestHandler > handler(pGlobal);
        pGlobal.clear();

        handler->mState = State::Downing;
        if (handler->mIpcThread.is()) {
            handler->mIpcThread->close();
        }

        // release mutex to avoid deadlocks
        aMutex.clear();

        handler->cReady.set();

        // exit gracefully and join
        if (handler->mIpcThread.is())
        {
            handler->mIpcThread->join();
            handler->mIpcThread.clear();
        }
    }
}

RequestHandler::RequestHandler() :
    mState( State::Starting ),
    mnPendingRequests( 0 )
{
}

RequestHandler::~RequestHandler()
{
    assert(!mIpcThread.is());
}

void RequestHandler::SetReady()
{
    osl::MutexGuard g(GetMutex());
    if (pGlobal.is())
    {
        pGlobal->cReady.set();
    }
}

void RequestHandler::WaitForReady()
{
    rtl::Reference<RequestHandler> t;
    {
        osl::MutexGuard g(GetMutex());
        t =  pGlobal;
    }
    if (t.is())
    {
        t->cReady.wait();
    }
}

bool IpcThread::process(OString const & arguments, bool * waitProcessed) {
    assert(waitProcessed != nullptr);

    std::unique_ptr< CommandLineArgs > aCmdLineArgs;
    try
    {
        Parser p(arguments);
        aCmdLineArgs.reset( new CommandLineArgs( p ) );
    }
    catch ( const CommandLineArgs::Supplier::Exception & )
    {
        SAL_WARN("desktop.app", "Error in received command line arguments");
        return false;
    }

    bool bDocRequestSent = false;

    OUString aUnknown( aCmdLineArgs->GetUnknown() );
    if ( !aUnknown.isEmpty() || aCmdLineArgs->IsHelp() )
    {
        ApplicationEvent* pAppEvent =
            new ApplicationEvent(ApplicationEvent::Type::Help, aUnknown);
        ImplPostForeignAppEvent( pAppEvent );
    }
    else if ( aCmdLineArgs->IsVersion() )
    {
        ApplicationEvent* pAppEvent =
            new ApplicationEvent(ApplicationEvent::Type::Version);
        ImplPostForeignAppEvent( pAppEvent );
    }
    else
    {
        const CommandLineArgs &rCurrentCmdLineArgs = Desktop::GetCommandLineArgs();

        if ( aCmdLineArgs->IsQuickstart() )
        {
            // we have to use application event, because we have to start quickstart service in main thread!!
            ApplicationEvent* pAppEvent =
                new ApplicationEvent(ApplicationEvent::Type::QuickStart);
            ImplPostForeignAppEvent( pAppEvent );
        }

        // handle request for acceptor
        std::vector< OUString > const & accept = aCmdLineArgs->GetAccept();
        for (std::vector< OUString >::const_iterator i(accept.begin());
             i != accept.end(); ++i)
        {
            ApplicationEvent* pAppEvent = new ApplicationEvent(
                ApplicationEvent::Type::Accept, *i);
            ImplPostForeignAppEvent( pAppEvent );
        }
        // handle acceptor removal
        std::vector< OUString > const & unaccept = aCmdLineArgs->GetUnaccept();
        for (std::vector< OUString >::const_iterator i(unaccept.begin());
             i != unaccept.end(); ++i)
        {
            ApplicationEvent* pAppEvent = new ApplicationEvent(
                ApplicationEvent::Type::Unaccept, *i);
            ImplPostForeignAppEvent( pAppEvent );
        }

        ProcessDocumentsRequest* pRequest = new ProcessDocumentsRequest(
            aCmdLineArgs->getCwdUrl());
        m_handler->cProcessed.reset();
        pRequest->pcProcessed = &m_handler->cProcessed;

        // Print requests are not dependent on the --invisible cmdline argument as they are
        // loaded with the "hidden" flag! So they are always checked.
        pRequest->aPrintList = aCmdLineArgs->GetPrintList();
        bDocRequestSent |= !pRequest->aPrintList.empty();
        pRequest->aPrintToList = aCmdLineArgs->GetPrintToList();
        pRequest->aPrinterName = aCmdLineArgs->GetPrinterName();
        bDocRequestSent |= !( pRequest->aPrintToList.empty() || pRequest->aPrinterName.isEmpty() );
        pRequest->aConversionList = aCmdLineArgs->GetConversionList();
        pRequest->aConversionParams = aCmdLineArgs->GetConversionParams();
        pRequest->aConversionOut = aCmdLineArgs->GetConversionOut();
        pRequest->aInFilter = aCmdLineArgs->GetInFilter();
        pRequest->bTextCat = aCmdLineArgs->IsTextCat();
        bDocRequestSent |= !pRequest->aConversionList.empty();

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
                aHelpURLBuffer.append("&System=WIN");
#endif
                ApplicationEvent* pAppEvent = new ApplicationEvent(
                    ApplicationEvent::Type::OpenHelpUrl,
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
            pRequest = nullptr;
        }
        if (aCmdLineArgs->IsEmpty())
        {
            // no document was sent, just bring Office to front
            ApplicationEvent* pAppEvent =
                new ApplicationEvent(ApplicationEvent::Type::Appear);
            ImplPostForeignAppEvent( pAppEvent );
        }
    }
    *waitProcessed = bDocRequestSent;
    return true;
}

void PipeIpcThread::execute()
{
    assert(m_handler != nullptr);
    do
    {
        osl::StreamPipe aStreamPipe;
        oslPipeError nError = pipe_.accept( aStreamPipe );


        if( nError == osl_Pipe_E_None )
        {
            // if we receive a request while the office is displaying some dialog or error during
            // bootstrap, that dialogs event loop might get events that are dispatched by this thread
            // we have to wait for cReady to be set by the real main loop.
            // only requests that don't dispatch events may be processed before cReady is set.
            m_handler->cReady.wait();

            // we might have decided to shutdown while we were sleeping
            if (!RequestHandler::pGlobal.is()) return;

            // only lock the mutex when processing starts, othewise we deadlock when the office goes
            // down during wait
            osl::ClearableMutexGuard aGuard( RequestHandler::GetMutex() );

            if (m_handler->mState == RequestHandler::State::Downing)
            {
                break;
            }

            // notify client we're ready to process its args:
            SAL_INFO("desktop.app", "writing <" << SEND_ARGUMENTS << ">");
            sal_Int32 n = aStreamPipe.write(
                SEND_ARGUMENTS, SAL_N_ELEMENTS(SEND_ARGUMENTS));
                // incl. terminating NUL
            if (n != SAL_N_ELEMENTS(SEND_ARGUMENTS)) {
                SAL_WARN("desktop.app", "short write: " << n);
                continue;
            }

            OString aArguments = readStringFromPipe(aStreamPipe);

            // Is this a lookup message from another application? if so, ignore
            if (aArguments.isEmpty())
                continue;

            bool waitProcessed = false;
            if (!process(aArguments, &waitProcessed)) {
                continue;
            }

            // we don't need the mutex any longer...
            aGuard.clear();
            // wait for processing to finish
            if (waitProcessed)
                m_handler->cProcessed.wait();
            // processing finished, inform the requesting end:
            SAL_INFO("desktop.app", "writing <" << PROCESSING_DONE << ">");
            n = aStreamPipe.write(
                PROCESSING_DONE, SAL_N_ELEMENTS(PROCESSING_DONE));
                // incl. terminating NUL
            if (n != SAL_N_ELEMENTS(PROCESSING_DONE)) {
                SAL_WARN("desktop.app", "short write: " << n);
                continue;
            }
        }
        else
        {
            {
                osl::MutexGuard aGuard( RequestHandler::GetMutex() );
                if (m_handler->mState == RequestHandler::State::Downing)
                {
                    break;
                }
            }

            SAL_WARN( "desktop.app", "Error on accept: " << (int)nError);
            TimeValue tval;
            tval.Seconds = 1;
            tval.Nanosec = 0;
            salhelper::Thread::wait( tval );
        }
    } while( schedule() );
}

static void AddToDispatchList(
    std::vector<DispatchWatcher::DispatchRequest>& rDispatchList,
    boost::optional< OUString > const & cwdUrl,
    std::vector< OUString > const & aRequestList,
    DispatchWatcher::RequestType nType,
    const OUString& aParam,
    const OUString& aFactory )
{
    for (std::vector< OUString >::const_iterator i(aRequestList.begin());
         i != aRequestList.end(); ++i)
    {
        rDispatchList.push_back({nType, *i, cwdUrl, aParam, aFactory});
    }
}

static void AddConversionsToDispatchList(
    std::vector<DispatchWatcher::DispatchRequest>& rDispatchList,
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
    if (cwdUrl)
    {
        aPWD = *cwdUrl;
    }
    else
    {
        ::tools::getProcessWorkingDir( aPWD );
    }

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
        rDispatchList.push_back({nType, *i, cwdUrl, aParam, rFactory});
    }
}


bool RequestHandler::ExecuteCmdLineRequests(
    ProcessDocumentsRequest& aRequest, bool noTerminate)
{
    // protect the dispatch list
    osl::ClearableMutexGuard aGuard( GetMutex() );

    static std::vector<DispatchWatcher::DispatchRequest> aDispatchList;

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

    if ( pGlobal.is() )
    {
        if( ! pGlobal->AreRequestsEnabled() )
            return bShutdown;

        pGlobal->mnPendingRequests += aDispatchList.size();
        if ( !pGlobal->mpDispatchWatcher.is() )
        {
            pGlobal->mpDispatchWatcher = new DispatchWatcher;
        }
        rtl::Reference<DispatchWatcher> dispatchWatcher(
            pGlobal->mpDispatchWatcher);

        // copy for execute
        std::vector<DispatchWatcher::DispatchRequest> aTempList( aDispatchList );
        aDispatchList.clear();

        aGuard.clear();

        // Execute dispatch requests
        bShutdown = dispatchWatcher->executeDispatchRequests( aTempList, noTerminate);

        // set processed flag
        if (aRequest.pcProcessed != nullptr)
            aRequest.pcProcessed->set();
    }

    return bShutdown;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
