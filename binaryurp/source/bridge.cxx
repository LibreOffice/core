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

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/bridge/InvalidProtocolChangeException.hpp"
#include "com/sun/star/bridge/XBridge.hpp"
#include "com/sun/star/bridge/XInstanceProvider.hpp"
#include "com/sun/star/bridge/XProtocolProperties.hpp"
#include "com/sun/star/connection/XConnection.hpp"
#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/EventObject.hpp"
#include "com/sun/star/lang/XEventListener.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/mutex.hxx"
#include "osl/thread.hxx"
#include "rtl/byteseq.hxx"
#include "rtl/oustringostreaminserter.hxx"
#include "rtl/random.h"
#include "rtl/ref.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"
#include "typelib/typedescription.hxx"
#include "uno/dispatcher.hxx"
#include "uno/environment.hxx"
#include "uno/lbnames.h"

#include "binaryany.hxx"
#include "bridge.hxx"
#include "bridgefactory.hxx"
#include "incomingreply.hxx"
#include "lessoperators.hxx"
#include "outgoingrequest.hxx"
#include "outgoingrequests.hxx"
#include "proxy.hxx"
#include "reader.hxx"

namespace binaryurp {

namespace {

namespace css = com::sun::star;

sal_Int32 random() {
    sal_Int32 n;
    rtlRandomPool pool = rtl_random_createPool();
    rtl_random_getBytes(pool, &n, sizeof n);
    rtl_random_destroyPool(pool);
    return n;
}

OUString toString(css::uno::TypeDescription const & type) {
    typelib_TypeDescription * d = type.get();
    assert(d != 0 && d->pTypeName != 0);
    return OUString(d->pTypeName);
}

extern "C" void SAL_CALL freeProxyCallback(
    SAL_UNUSED_PARAMETER uno_ExtEnvironment *, void * pProxy)
{
    assert(pProxy != 0);
    static_cast< Proxy * >(pProxy)->do_free();
}

bool isThread(salhelper::Thread * thread) {
    assert(thread != 0);
    return osl::Thread::getCurrentIdentifier() == thread->getIdentifier();
}

class AttachThread: private boost::noncopyable {
public:
    explicit AttachThread(uno_ThreadPool threadPool);

    ~AttachThread();

    rtl::ByteSequence getTid() throw ();

private:
    uno_ThreadPool threadPool_;
    rtl::ByteSequence tid_;
};

AttachThread::AttachThread(uno_ThreadPool threadPool): threadPool_(threadPool) {
    sal_Sequence * s = 0;
    uno_getIdOfCurrentThread(&s);
    tid_ = rtl::ByteSequence(s, rtl::BYTESEQ_NOACQUIRE);
    uno_threadpool_attach(threadPool_);
}

AttachThread::~AttachThread() {
    uno_threadpool_detach(threadPool_);
    uno_releaseIdFromCurrentThread();
}

rtl::ByteSequence AttachThread::getTid() throw () {
    return tid_;
}

class PopOutgoingRequest: private boost::noncopyable {
public:
    PopOutgoingRequest(
        OutgoingRequests & requests, rtl::ByteSequence const & tid,
        OutgoingRequest const & request);

    ~PopOutgoingRequest();

    void clear();

private:
    OutgoingRequests & requests_;
    rtl::ByteSequence tid_;
    bool cleared_;
};

PopOutgoingRequest::PopOutgoingRequest(
    OutgoingRequests & requests, rtl::ByteSequence const & tid,
    OutgoingRequest const & request):
    requests_(requests), tid_(tid), cleared_(false)
{
    requests_.push(tid_, request);
}

PopOutgoingRequest::~PopOutgoingRequest() {
    if (!cleared_) {
        requests_.pop(tid_);
    }
}

void PopOutgoingRequest::clear() {
    cleared_ = true;
}

}

struct Bridge::SubStub {
    com::sun::star::uno::UnoInterfaceReference object;

    sal_uInt32 references;
};

Bridge::Bridge(
    rtl::Reference< BridgeFactory > const & factory, OUString const & name,
    css::uno::Reference< css::connection::XConnection > const & connection,
    css::uno::Reference< css::bridge::XInstanceProvider > const & provider):
    factory_(factory), name_(name), connection_(connection),
    provider_(provider),
    binaryUno_(OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))),
    cppToBinaryMapping_(
        OUString(
            RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME)),
        OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))),
    binaryToCppMapping_(
        OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO)),
        OUString(
            RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME))),
    protPropTid_(
        reinterpret_cast< sal_Int8 const * >(".UrpProtocolPropertiesTid"),
        RTL_CONSTASCII_LENGTH(".UrpProtocolPropertiesTid")),
    protPropOid_(RTL_CONSTASCII_USTRINGPARAM("UrpProtocolProperties")),
    protPropType_(
        cppu::UnoType<
            css::uno::Reference< css::bridge::XProtocolProperties > >::get()),
    protPropRequest_(
        OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.bridge.XProtocolProperties::requestChange"))),
    protPropCommit_(
        OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.bridge.XProtocolProperties::commitChange"))),
    state_(STATE_INITIAL), threadPool_(0), currentContextMode_(false),
    proxies_(0), calls_(0), normalCall_(false), activeCalls_(0),
    mode_(MODE_REQUESTED)
{
    assert(factory.is() && connection.is());
    if (!binaryUno_.is()) {
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM("URP: no binary UNO environment")),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (!(cppToBinaryMapping_.is() && binaryToCppMapping_.is())) {
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM("URP: no C++ UNO mapping")),
            css::uno::Reference< css::uno::XInterface >());
    }
    passive_.set();
}

void Bridge::start() {
    rtl::Reference< Reader > r(new Reader(this));
    rtl::Reference< Writer > w(new Writer(this));
    {
        osl::MutexGuard g(mutex_);
        assert(
            state_ == STATE_INITIAL && threadPool_ == 0 && !writer_.is() &&
            !reader_.is());
        threadPool_ = uno_threadpool_create();
        assert(threadPool_ != 0);
        reader_ = r;
        writer_ = w;
        state_ = STATE_STARTED;
    }
    // It is important to call reader_->launch() last here; both
    // Writer::execute and Reader::execute can call Bridge::terminate, but
    // Writer::execute is initially blocked in unblocked_.wait() until
    // Reader::execute has called bridge_->sendRequestChangeRequest(), so
    // effectively only reader_->launch() can lead to an early call to
    // Bridge::terminate
    w->launch();
    r->launch();
}

void Bridge::terminate(bool final) {
    uno_ThreadPool tp;
    // Make sure function-local variables (Stubs s, etc.) are destroyed before
    // the final uno_threadpool_destroy/threadPool_ = 0:
    {
        rtl::Reference< Reader > r;
        rtl::Reference< Writer > w;
        bool joinW;
        Listeners ls;
        {
            osl::ClearableMutexGuard g(mutex_);
            switch (state_) {
            case STATE_INITIAL: // via ~Bridge -> dispose -> terminate
            case STATE_FINAL:
                return;
            case STATE_STARTED:
                break;
            case STATE_TERMINATED:
                if (final) {
                    g.clear();
                    terminated_.wait();
                    {
                        osl::MutexGuard g2(mutex_);
                        tp = threadPool_;
                        threadPool_ = 0;
                        assert(!(reader_.is() && isThread(reader_.get())));
                        std::swap(reader_, r);
                        assert(!(writer_.is() && isThread(writer_.get())));
                        std::swap(writer_, w);
                        state_ = STATE_FINAL;
                    }
                    assert(!(r.is() && w.is()));
                    if (r.is()) {
                        r->join();
                    } else if (w.is()) {
                        w->join();
                    }
                    if (tp != 0) {
                        uno_threadpool_destroy(tp);
                    }
                }
                return;
            }
            tp = threadPool_;
            assert(!(final && isThread(reader_.get())));
            if (!isThread(reader_.get())) {
                std::swap(reader_, r);
            }
            w = writer_;
            joinW = !isThread(writer_.get());
            assert(!final || joinW);
            if (joinW) {
                writer_.clear();
            }
            ls.swap(listeners_);
            state_ = final ? STATE_FINAL : STATE_TERMINATED;
        }
        try {
            connection_->close();
        } catch (const css::io::IOException & e) {
            SAL_INFO("binaryurp", "caught IO exception '" << e.Message << '\'');
        }
        assert(w.is());
        w->stop();
        if (r.is()) {
            r->join();
        }
        if (joinW) {
            w->join();
        }
        assert(tp != 0);
        uno_threadpool_dispose(tp);
        Stubs s;
        {
            osl::MutexGuard g(mutex_);
            s.swap(stubs_);
        }
        for (Stubs::iterator i(s.begin()); i != s.end(); ++i) {
            for (Stub::iterator j(i->second.begin()); j != i->second.end(); ++j)
            {
                SAL_INFO(
                    "binaryurp",
                    "stub '" << i->first << "', '" << toString(j->first)
                        << "' still mapped at Bridge::terminate");
                binaryUno_.get()->pExtEnv->revokeInterface(
                    binaryUno_.get()->pExtEnv, j->second.object.get());
            }
        }
        factory_->removeBridge(this);
        for (Listeners::iterator i(ls.begin()); i != ls.end(); ++i) {
            try {
                (*i)->disposing(
                    css::lang::EventObject(
                        static_cast< cppu::OWeakObject * >(this)));
            } catch (const css::uno::RuntimeException & e) {
                SAL_WARN(
                    "binaryurp",
                    "caught runtime exception '" << e.Message << '\'');
            }
        }
    }
    if (final) {
        uno_threadpool_destroy(tp);
    }
    {
        osl::MutexGuard g(mutex_);
        if (final) {
            threadPool_ = 0;
        }
    }
    terminated_.set();
}

css::uno::Reference< css::connection::XConnection > Bridge::getConnection()
    const
{
    return connection_;
}

css::uno::Reference< css::bridge::XInstanceProvider > Bridge::getProvider()
    const
{
    return provider_;
}

css::uno::Mapping & Bridge::getCppToBinaryMapping() {
    return cppToBinaryMapping_;
}

BinaryAny Bridge::mapCppToBinaryAny(css::uno::Any const & cppAny) {
    css::uno::Any in(cppAny);
    BinaryAny out;
    out.~BinaryAny();
    uno_copyAndConvertData(
        out.get(), &in,
        css::uno::TypeDescription(cppu::UnoType< css::uno::Any >::get()).get(),
        cppToBinaryMapping_.get());
    return out;
}

uno_ThreadPool Bridge::getThreadPool() {
    osl::MutexGuard g(mutex_);
    checkDisposed();
    assert(threadPool_ != 0);
    return threadPool_;
}

rtl::Reference< Writer > Bridge::getWriter() {
    osl::MutexGuard g(mutex_);
    checkDisposed();
    assert(writer_.is());
    return writer_;
}

css::uno::UnoInterfaceReference Bridge::registerIncomingInterface(
    OUString const & oid, css::uno::TypeDescription const & type)
{
    assert(type.is());
    if (oid.isEmpty()) {
        return css::uno::UnoInterfaceReference();
    }
    css::uno::UnoInterfaceReference obj(findStub(oid, type));
    if (!obj.is()) {
        binaryUno_.get()->pExtEnv->getRegisteredInterface(
            binaryUno_.get()->pExtEnv,
            reinterpret_cast< void ** >(&obj.m_pUnoI), oid.pData,
            reinterpret_cast< typelib_InterfaceTypeDescription * >(type.get()));
        if (obj.is()) {
            makeReleaseCall(oid, type);
        } else {
            obj.set(new Proxy(this, oid, type), SAL_NO_ACQUIRE);
            {
                osl::MutexGuard g(mutex_);
                assert(proxies_ < std::numeric_limits< std::size_t >::max());
                ++proxies_;
            }
            binaryUno_.get()->pExtEnv->registerProxyInterface(
                binaryUno_.get()->pExtEnv,
                reinterpret_cast< void ** >(&obj.m_pUnoI), &freeProxyCallback,
                oid.pData,
                reinterpret_cast< typelib_InterfaceTypeDescription * >(
                    type.get()));
        }
    }
    return obj;
}

OUString Bridge::registerOutgoingInterface(
    css::uno::UnoInterfaceReference const & object,
    css::uno::TypeDescription const & type)
{
    assert(type.is());
    if (!object.is()) {
        return OUString();
    }
    OUString oid;
    if (!Proxy::isProxy(this, object, &oid)) {
        binaryUno_.get()->pExtEnv->getObjectIdentifier(
            binaryUno_.get()->pExtEnv, &oid.pData, object.get());
        osl::MutexGuard g(mutex_);
        Stubs::iterator i(stubs_.find(oid));
        Stub newStub;
        Stub * stub = i == stubs_.end() ? &newStub : &i->second;
        Stub::iterator j(stub->find(type));
        //TODO: Release sub-stub if it is not successfully sent to remote side
        // (otherwise, stub will leak until terminate()):
        if (j == stub->end()) {
            j = stub->insert(Stub::value_type(type, SubStub())).first;
            if (stub == &newStub) {
                i = stubs_.insert(Stubs::value_type(oid, Stub())).first;
                std::swap(i->second, newStub);
                j = i->second.find(type);
                assert(j !=  i->second.end());
            }
            j->second.object = object;
            j->second.references = 1;
            binaryUno_.get()->pExtEnv->registerInterface(
                binaryUno_.get()->pExtEnv,
                reinterpret_cast< void ** >(&j->second.object.m_pUnoI),
                oid.pData,
                reinterpret_cast< typelib_InterfaceTypeDescription * >(
                    type.get()));
        } else {
            assert(stub != &newStub);
            if (j->second.references == SAL_MAX_UINT32) {
                throw css::uno::RuntimeException(
                    OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "URP: stub reference count overflow")),
                    css::uno::Reference< css::uno::XInterface >());
            }
            ++j->second.references;
        }
    }
    return oid;
}

css::uno::UnoInterfaceReference Bridge::findStub(
    OUString const & oid, css::uno::TypeDescription const & type)
{
    assert(!oid.isEmpty() && type.is());
    osl::MutexGuard g(mutex_);
    Stubs::iterator i(stubs_.find(oid));
    if (i != stubs_.end()) {
        Stub::iterator j(i->second.find(type));
        if (j != i->second.end()) {
            return j->second.object;
        }
        for (j = i->second.begin(); j != i->second.end(); ++j) {
            if (typelib_typedescription_isAssignableFrom(
                    type.get(), j->first.get()))
            {
                return j->second.object;
            }
        }
    }
    return css::uno::UnoInterfaceReference();
}

void Bridge::releaseStub(
    OUString const & oid, css::uno::TypeDescription const & type)
{
    assert(!oid.isEmpty() && type.is());
    css::uno::UnoInterfaceReference obj;
    bool unused;
    {
        osl::MutexGuard g(mutex_);
        Stubs::iterator i(stubs_.find(oid));
        if (i == stubs_.end()) {
            throw css::uno::RuntimeException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM("URP: release unknown stub")),
                css::uno::Reference< css::uno::XInterface >());
        }
        Stub::iterator j(i->second.find(type));
        if (j == i->second.end()) {
            throw css::uno::RuntimeException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM("URP: release unknown stub")),
                css::uno::Reference< css::uno::XInterface >());
        }
        assert(j->second.references > 0);
        --j->second.references;
        if (j->second.references == 0) {
            obj = j->second.object;
            i->second.erase(j);
            if (i->second.empty()) {
                stubs_.erase(i);
            }
        }
        unused = becameUnused();
    }
    if (obj.is()) {
        binaryUno_.get()->pExtEnv->revokeInterface(
            binaryUno_.get()->pExtEnv, obj.get());
    }
    terminateWhenUnused(unused);
}

void Bridge::resurrectProxy(Proxy & proxy) {
    uno_Interface * p = &proxy;
    binaryUno_.get()->pExtEnv->registerProxyInterface(
        binaryUno_.get()->pExtEnv,
        reinterpret_cast< void ** >(&p), &freeProxyCallback,
        proxy.getOid().pData,
        reinterpret_cast< typelib_InterfaceTypeDescription * >(
            proxy.getType().get()));
    assert(p == &proxy);
}

void Bridge::revokeProxy(Proxy & proxy) {
    binaryUno_.get()->pExtEnv->revokeInterface(
        binaryUno_.get()->pExtEnv, &proxy);
}

void Bridge::freeProxy(Proxy & proxy) {
    try {
        makeReleaseCall(proxy.getOid(), proxy.getType());
    } catch (const css::uno::RuntimeException & e) {
        SAL_INFO(
            "binaryurp", "caught runtime exception '" << e.Message << '\'');
    } catch (const std::exception & e) {
        SAL_WARN("binaryurp", "caught C++ exception '" << e.what() << '\'');
    }
    bool unused;
    {
        osl::MutexGuard g(mutex_);
        assert(proxies_ > 0);
        --proxies_;
        unused = becameUnused();
    }
    terminateWhenUnused(unused);
}

void Bridge::incrementCalls(bool normalCall) throw () {
    osl::MutexGuard g(mutex_);
    assert(calls_ < std::numeric_limits< std::size_t >::max());
    ++calls_;
    normalCall_ |= normalCall;
}

void Bridge::decrementCalls() {
    bool unused;
    {
        osl::MutexGuard g(mutex_);
        assert(calls_ > 0);
        --calls_;
        unused = becameUnused();
    }
    terminateWhenUnused(unused);
}

void Bridge::incrementActiveCalls() throw () {
    osl::MutexGuard g(mutex_);
    assert(
        activeCalls_ <= calls_ &&
        activeCalls_ < std::numeric_limits< std::size_t >::max());
    ++activeCalls_;
    passive_.reset();
}

void Bridge::decrementActiveCalls() throw () {
    osl::MutexGuard g(mutex_);
    assert(activeCalls_ <= calls_ && activeCalls_ > 0);
    --activeCalls_;
    if (activeCalls_ == 0) {
        passive_.set();
    }
}

bool Bridge::makeCall(
    OUString const & oid, css::uno::TypeDescription const & member,
    bool setter, std::vector< BinaryAny > const & inArguments,
    BinaryAny * returnValue, std::vector< BinaryAny > * outArguments)
{
    std::auto_ptr< IncomingReply > resp;
    {
        uno_ThreadPool tp = getThreadPool();
        AttachThread att(tp);
        PopOutgoingRequest pop(
            outgoingRequests_, att.getTid(),
            OutgoingRequest(OutgoingRequest::KIND_NORMAL, member, setter));
        sendRequest(
            att.getTid(), oid, css::uno::TypeDescription(), member,
            inArguments);
        pop.clear();
        incrementCalls(true);
        incrementActiveCalls();
        void * job;
        uno_threadpool_enter(tp, &job);
        resp.reset(static_cast< IncomingReply * >(job));
        decrementActiveCalls();
        decrementCalls();
    }
    if (resp.get() == 0) {
        throw css::lang::DisposedException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "Binary URP bridge disposed during call")),
            static_cast< cppu::OWeakObject * >(this));
    }
    *returnValue = resp->returnValue;
    if (!resp->exception) {
        *outArguments = resp->outArguments;
    }
    return resp->exception;
}

void Bridge::sendRequestChangeRequest() {
    assert(mode_ == MODE_REQUESTED);
    random_ = random();
    std::vector< BinaryAny > a;
    a.push_back(
        BinaryAny(
            css::uno::TypeDescription(cppu::UnoType< sal_Int32 >::get()),
            &random_));
    sendProtPropRequest(OutgoingRequest::KIND_REQUEST_CHANGE, a);
}

void Bridge::handleRequestChangeReply(
    bool exception, BinaryAny const & returnValue)
{
    try {
        throwException(exception, returnValue);
    } catch (css::uno::RuntimeException & e) {
        // Before OOo 2.2, Java URP would throw a RuntimeException when
        // receiving a requestChange message (see i#35277 "Java URP: Support
        // Manipulation of Protocol Properties"):
        if (mode_ != MODE_REQUESTED) {
            throw;
        }
        SAL_WARN(
            "binaryurp",
            "requestChange caught RuntimeException \'" << e.Message
                << "' in state 'requested'");
        mode_ = MODE_NORMAL;
        getWriter()->unblock();
        decrementCalls();
        return;
    }
    sal_Int32 n = *static_cast< sal_Int32 * >(
        returnValue.getValue(
            css::uno::TypeDescription(cppu::UnoType< sal_Int32 >::get())));
    sal_Int32 exp = 0;
    switch (mode_) {
    case MODE_REQUESTED:
    case MODE_REPLY_1:
        exp = 1;
        break;
    case MODE_REPLY_MINUS1:
        exp = -1;
        mode_ = MODE_REQUESTED;
        break;
    case MODE_REPLY_0:
        exp = 0;
        mode_ = MODE_WAIT;
        break;
    default:
        assert(false); // this cannot happen
        break;
    }
    if (n != exp) {
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "URP: requestChange reply with unexpected return value"
                    " received")),
            static_cast< cppu::OWeakObject * >(this));
    }
    decrementCalls();
    switch (exp) {
    case -1:
        sendRequestChangeRequest();
        break;
    case 0:
        break;
    case 1:
        sendCommitChangeRequest();
        break;
    default:
        assert(false); // this cannot happen
        break;
    }
}

void Bridge::handleCommitChangeReply(
    bool exception, BinaryAny const & returnValue)
{
    bool ccMode = true;
    try {
        throwException(exception, returnValue);
    } catch (const css::bridge::InvalidProtocolChangeException &) {
        ccMode = false;
    }
    if (ccMode) {
        setCurrentContextMode();
    }
    assert(mode_ == MODE_REQUESTED || mode_ == MODE_REPLY_1);
    mode_ = MODE_NORMAL;
    getWriter()->unblock();
    decrementCalls();
}

void Bridge::handleRequestChangeRequest(
    rtl::ByteSequence const & tid, std::vector< BinaryAny > const & inArguments)
{
    assert(inArguments.size() == 1);
    switch (mode_) {
    case MODE_REQUESTED:
        {
            sal_Int32 n2 = *static_cast< sal_Int32 * >(
                inArguments[0].getValue(
                    css::uno::TypeDescription(
                        cppu::UnoType< sal_Int32 >::get())));
            sal_Int32 ret;
            if (n2 > random_) {
                ret = 1;
                mode_ = MODE_REPLY_0;
            } else if (n2 == random_) {
                ret = -1;
                mode_ = MODE_REPLY_MINUS1;
            } else {
                ret = 0;
                mode_ = MODE_REPLY_1;
            }
            getWriter()->sendDirectReply(
                tid, protPropRequest_, false,
                BinaryAny(
                    css::uno::TypeDescription(
                        cppu::UnoType< sal_Int32 >::get()),
                    &ret),
            std::vector< BinaryAny >());
            break;
        }
    case MODE_NORMAL:
        {
            mode_ = MODE_NORMAL_WAIT;
            sal_Int32 ret = 1;
            getWriter()->queueReply(
                tid, protPropRequest_, false, false,
                BinaryAny(
                    css::uno::TypeDescription(
                        cppu::UnoType< sal_Int32 >::get()),
                    &ret),
            std::vector< BinaryAny >(), false);
            break;
        }
    default:
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "URP: unexpected requestChange request received")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

void Bridge::handleCommitChangeRequest(
    rtl::ByteSequence const & tid, std::vector< BinaryAny > const & inArguments)
{
    bool ccMode = false;
    bool exc = false;
    BinaryAny ret;
    assert(inArguments.size() == 1);
    css::uno::Sequence< css::bridge::ProtocolProperty > s;
    bool ok = (mapBinaryToCppAny(inArguments[0]) >>= s);
    assert(ok);
    (void) ok; // avoid warnings
    for (sal_Int32 i = 0; i != s.getLength(); ++i) {
        if ( s[i].Name == "CurrentContext" )
        {
            ccMode = true;
        } else {
            ccMode = false;
            exc = true;
            ret = mapCppToBinaryAny(
                css::uno::makeAny(
                    css::bridge::InvalidProtocolChangeException(
                        OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "InvalidProtocolChangeException")),
                        css::uno::Reference< css::uno::XInterface >(), s[i],
                        1)));
            break;
        }
    }
    switch (mode_) {
    case MODE_WAIT:
        getWriter()->sendDirectReply(
            tid, protPropCommit_, exc, ret, std::vector< BinaryAny >());
        if (ccMode) {
            setCurrentContextMode();
            mode_ = MODE_NORMAL;
            getWriter()->unblock();
        } else {
            mode_ = MODE_REQUESTED;
            sendRequestChangeRequest();
        }
        break;
    case MODE_NORMAL_WAIT:
        getWriter()->queueReply(
            tid, protPropCommit_, false, false, ret, std::vector< BinaryAny >(),
            ccMode);
        mode_ = MODE_NORMAL;
        break;
    default:
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "URP: unexpected commitChange request received")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

OutgoingRequest Bridge::lastOutgoingRequest(rtl::ByteSequence const & tid) {
    OutgoingRequest req(outgoingRequests_.top(tid));
    outgoingRequests_.pop(tid);
    return req;
}

bool Bridge::isProtocolPropertiesRequest(
    OUString const & oid, css::uno::TypeDescription const & type) const
{
    return oid == protPropOid_ && type.equals(protPropType_);
}

void Bridge::setCurrentContextMode() {
    osl::MutexGuard g(mutex_);
    currentContextMode_ = true;
}

bool Bridge::isCurrentContextMode() {
    osl::MutexGuard g(mutex_);
    return currentContextMode_;
}

Bridge::~Bridge() {
#if OSL_DEBUG_LEVEL > 0
    {
        osl::MutexGuard g(mutex_);
        SAL_WARN_IF(
            state_ == STATE_STARTED || state_ == STATE_TERMINATED, "binaryurp",
            "undisposed bridge, potential deadlock ahead");
    }
#endif
    dispose();
}

css::uno::Reference< css::uno::XInterface > Bridge::getInstance(
    OUString const & sInstanceName) throw (css::uno::RuntimeException)
{
    if (sInstanceName.isEmpty()) {
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "XBridge::getInstance sInstanceName must be non-empty")),
            static_cast< cppu::OWeakObject * >(this));
    }
    for (sal_Int32 i = 0; i != sInstanceName.getLength(); ++i) {
        if (sInstanceName[i] > 0x7F) {
            throw css::io::IOException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "XBridge::getInstance sInstanceName contains non-ASCII"
                        " character")),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
    css::uno::TypeDescription ifc(
        cppu::UnoType< css::uno::Reference< css::uno::XInterface > >::get());
    typelib_TypeDescription * p = ifc.get();
    std::vector< BinaryAny > inArgs;
    inArgs.push_back(
        BinaryAny(
            css::uno::TypeDescription(cppu::UnoType< css::uno::Type >::get()),
            &p));
    BinaryAny ret;
    std::vector< BinaryAny> outArgs;
    bool exc = makeCall(
        sInstanceName,
        css::uno::TypeDescription(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.uno.XInterface::queryInterface"))),
        false, inArgs, &ret, &outArgs);
    throwException(exc, ret);
    return css::uno::Reference< css::uno::XInterface >(
        static_cast< css::uno::XInterface * >(
            binaryToCppMapping_.mapInterface(
                *static_cast< uno_Interface ** >(ret.getValue(ifc)),
                ifc.get())),
        css::uno::UNO_REF_NO_ACQUIRE);
}

OUString Bridge::getName() throw (css::uno::RuntimeException) {
    return name_;
}

OUString Bridge::getDescription() throw (css::uno::RuntimeException) {
    OUStringBuffer b(name_);
    b.append(sal_Unicode(':'));
    b.append(connection_->getDescription());
    return b.makeStringAndClear();
}

void Bridge::dispose() throw (css::uno::RuntimeException) {
    // For terminate(true) not to deadlock, an external protocol must ensure
    // that dispose is not called from a thread pool worker thread (that dispose
    // is never called from the reader or writer thread is already ensured
    // internally):
    terminate(true);
    // OOo expects dispose to not return while there are still remote calls in
    // progress; an external protocol must ensure that dispose is not called
    // from within an incoming or outgoing remote call, as passive_.wait() would
    // otherwise deadlock:
    passive_.wait();
}

void Bridge::addEventListener(
    css::uno::Reference< css::lang::XEventListener > const & xListener)
    throw (css::uno::RuntimeException)
{
    assert(xListener.is());
    {
        osl::MutexGuard g(mutex_);
        assert(state_ != STATE_INITIAL);
        if (state_ == STATE_STARTED) {
            listeners_.push_back(xListener);
            return;
        }
    }
    xListener->disposing(
        css::lang::EventObject(static_cast< cppu::OWeakObject * >(this)));
}

void Bridge::removeEventListener(
    css::uno::Reference< css::lang::XEventListener > const & aListener)
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard g(mutex_);
    Listeners::iterator i(
        std::find(listeners_.begin(), listeners_.end(), aListener));
    if (i != listeners_.end()) {
        listeners_.erase(i);
    }
}

void Bridge::sendCommitChangeRequest() {
    assert(mode_ == MODE_REQUESTED || mode_ == MODE_REPLY_1);
    css::uno::Sequence< css::bridge::ProtocolProperty > s(1);
    s[0].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("CurrentContext"));
    std::vector< BinaryAny > a;
    a.push_back(mapCppToBinaryAny(css::uno::makeAny(s)));
    sendProtPropRequest(OutgoingRequest::KIND_COMMIT_CHANGE, a);
}

void Bridge::sendProtPropRequest(
    OutgoingRequest::Kind kind, std::vector< BinaryAny > const & inArguments)
{
    assert(
        kind == OutgoingRequest::KIND_REQUEST_CHANGE ||
        kind == OutgoingRequest::KIND_COMMIT_CHANGE);
    incrementCalls(false);
    css::uno::TypeDescription member(
        kind == OutgoingRequest::KIND_REQUEST_CHANGE
        ? protPropRequest_ : protPropCommit_);
    PopOutgoingRequest pop(
        outgoingRequests_, protPropTid_, OutgoingRequest(kind, member, false));
    getWriter()->sendDirectRequest(
        protPropTid_, protPropOid_, protPropType_, member, inArguments);
    pop.clear();
}

void Bridge::makeReleaseCall(
    OUString const & oid, css::uno::TypeDescription const & type)
{
    AttachThread att(getThreadPool());
    sendRequest(
        att.getTid(), oid, type,
        css::uno::TypeDescription(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.uno.XInterface::release"))),
        std::vector< BinaryAny >());
}

void Bridge::sendRequest(
    rtl::ByteSequence const & tid, OUString const & oid,
    css::uno::TypeDescription const & type,
    css::uno::TypeDescription const & member,
    std::vector< BinaryAny > const & inArguments)
{
    getWriter()->queueRequest(tid, oid, type, member, inArguments);
}

void Bridge::throwException(bool exception, BinaryAny const & value) {
    if (exception) {
        cppu::throwException(mapBinaryToCppAny(value));
    }
}

css::uno::Any Bridge::mapBinaryToCppAny(BinaryAny const & binaryAny) {
    BinaryAny in(binaryAny);
    css::uno::Any out;
    out.~Any();
    uno_copyAndConvertData(
        &out, in.get(),
        css::uno::TypeDescription(cppu::UnoType< css::uno::Any >::get()).get(),
        binaryToCppMapping_.get());
    return out;
}

bool Bridge::becameUnused() const {
    return stubs_.empty() && proxies_ == 0 && calls_ == 0 && normalCall_;
}

void Bridge::terminateWhenUnused(bool unused) {
    if (unused) {
        // That the current thread considers the bridge unused implies that it
        // is not within an incoming or outgoing remote call (so calling
        // terminate cannot lead to deadlock):
        terminate(false);
    }
}

void Bridge::checkDisposed() {
    assert(state_ != STATE_INITIAL);
    if (state_ != STATE_STARTED) {
        throw css::lang::DisposedException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "Binary URP bridge already disposed")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
