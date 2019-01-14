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

#ifndef INCLUDED_BINARYURP_SOURCE_BRIDGE_HXX
#define INCLUDED_BINARYURP_SOURCE_BRIDGE_HXX

#include <sal/config.h>

#include <cstddef>
#include <map>
#include <vector>

#include <com/sun/star/bridge/XBridge.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <uno/environment.hxx>
#include <uno/mapping.hxx>
#include <uno/threadpool.h>

#include "outgoingrequest.hxx"
#include "outgoingrequests.hxx"
#include "writer.hxx"

namespace binaryurp {
    class BinaryAny;
    class BridgeFactory;
    class Proxy;
    class Reader;
}
namespace com::sun::star {
    namespace bridge { class XInstanceProvider; }
    namespace connection { class XConnection; }
    namespace lang { class XEventListener; }
    namespace uno {
        class Any;
        class TypeDescription;
        class UnoInterfaceReference;
        class XInterface;
    }
}
namespace rtl { class ByteSequence; }

namespace binaryurp {

class Bridge:
    public cppu::WeakImplHelper<
        com::sun::star::bridge::XBridge, com::sun::star::lang::XComponent >
{
public:
    Bridge(
        rtl::Reference< BridgeFactory > const & factory,
        OUString const & name,
        com::sun::star::uno::Reference<
            com::sun::star::connection::XConnection > const & connection,
        com::sun::star::uno::Reference<
            com::sun::star::bridge::XInstanceProvider > const & provider);

    void start();

    // Internally waits for all incoming and outgoing remote calls to terminate,
    // so must not be called from within such a call; when final is true, also
    // joins all remaining threads (reader, writer, and worker threads from the
    // thread pool), so must not be called with final set to true from such a
    // thread:
    void terminate(bool final);

    const com::sun::star::uno::Reference< com::sun::star::connection::XConnection >&
    getConnection() const { return connection_;}

    const com::sun::star::uno::Reference< com::sun::star::bridge::XInstanceProvider >&
    getProvider() const { return provider_;}

    com::sun::star::uno::Mapping & getCppToBinaryMapping() { return cppToBinaryMapping_;}

    BinaryAny mapCppToBinaryAny(com::sun::star::uno::Any const & cppAny);

    uno_ThreadPool getThreadPool();

    rtl::Reference< Writer > getWriter();

    com::sun::star::uno::UnoInterfaceReference registerIncomingInterface(
        OUString const & oid,
        com::sun::star::uno::TypeDescription const & type);

    OUString registerOutgoingInterface(
        com::sun::star::uno::UnoInterfaceReference const & object,
        com::sun::star::uno::TypeDescription const & type);

    com::sun::star::uno::UnoInterfaceReference findStub(
        OUString const & oid,
        com::sun::star::uno::TypeDescription const & type);

    void releaseStub(
        OUString const & oid,
        com::sun::star::uno::TypeDescription const & type);

    void resurrectProxy(Proxy & proxy);

    void revokeProxy(Proxy & proxy);

    void freeProxy(Proxy & proxy);

    void incrementCalls(bool normalCall) throw ();

    void decrementCalls();

    void incrementActiveCalls() throw ();

    void decrementActiveCalls() throw ();

    bool makeCall(
        OUString const & oid,
        com::sun::star::uno::TypeDescription const & member, bool setter,
        std::vector< BinaryAny > const & inArguments, BinaryAny * returnValue,
        std::vector< BinaryAny > * outArguments);

    // Only called from reader_ thread:
    void sendRequestChangeRequest();

    // Only called from reader_ thread:
    void handleRequestChangeReply(
        bool exception, BinaryAny const & returnValue);

    // Only called from reader_ thread:
    void handleCommitChangeReply(bool exception, BinaryAny const & returnValue);

    // Only called from reader_ thread:
    void handleRequestChangeRequest(
        rtl::ByteSequence const & tid,
        std::vector< BinaryAny > const & inArguments);

    // Only called from reader_ thread:
    void handleCommitChangeRequest(
        rtl::ByteSequence const & tid,
        std::vector< BinaryAny > const & inArguments);

    OutgoingRequest lastOutgoingRequest(rtl::ByteSequence const & tid);

    bool isProtocolPropertiesRequest(
        OUString const & oid,
        com::sun::star::uno::TypeDescription const & type) const;

    void setCurrentContextMode();

    bool isCurrentContextMode();

private:
    Bridge(const Bridge&) = delete;
    Bridge& operator=(const Bridge&) = delete;

    virtual ~Bridge() override;

    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL getInstance(OUString const & sInstanceName) override;

    virtual OUString SAL_CALL getName() override;

    virtual OUString SAL_CALL getDescription() override;

    virtual void SAL_CALL dispose() override;

    virtual void SAL_CALL addEventListener(
        com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >
            const & xListener) override;

    virtual void SAL_CALL removeEventListener(
        com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >
            const & aListener) override;

    // Only called from reader_ thread:
    void sendCommitChangeRequest();

    // Only called from reader_ thread:
    void sendProtPropRequest(
        OutgoingRequest::Kind kind,
        std::vector< BinaryAny > const & inArguments);

    void makeReleaseCall(
        OUString const & oid,
        com::sun::star::uno::TypeDescription const & type);

    void sendRequest(
        rtl::ByteSequence const & tid, OUString const & oid,
        com::sun::star::uno::TypeDescription const & type,
        com::sun::star::uno::TypeDescription const & member,
        std::vector< BinaryAny > const & inArguments);

    void throwException(bool exception, BinaryAny const & value);

    com::sun::star::uno::Any mapBinaryToCppAny(BinaryAny const & binaryAny);

    bool becameUnused() const;

    void terminateWhenUnused(bool unused);

    // Must only be called with mutex_ locked:
    void checkDisposed();

    typedef
        std::vector<
            com::sun::star::uno::Reference<
                com::sun::star::lang::XEventListener > >
        Listeners;

    struct SubStub;

    typedef std::map< com::sun::star::uno::TypeDescription, SubStub > Stub;

    typedef std::map< OUString, Stub > Stubs;

    enum State { STATE_INITIAL, STATE_STARTED, STATE_TERMINATED, STATE_FINAL };

    enum Mode {
        MODE_REQUESTED, MODE_REPLY_MINUS1, MODE_REPLY_0, MODE_REPLY_1,
        MODE_WAIT, MODE_NORMAL, MODE_NORMAL_WAIT };

    rtl::Reference< BridgeFactory > factory_;
    OUString name_;
    com::sun::star::uno::Reference< com::sun::star::connection::XConnection >
        connection_;
    com::sun::star::uno::Reference< com::sun::star::bridge::XInstanceProvider >
        provider_;
    com::sun::star::uno::Environment binaryUno_;
    com::sun::star::uno::Mapping cppToBinaryMapping_;
    com::sun::star::uno::Mapping binaryToCppMapping_;
    rtl::ByteSequence protPropTid_;
    OUString protPropOid_;
    com::sun::star::uno::TypeDescription protPropType_;
    com::sun::star::uno::TypeDescription protPropRequest_;
    com::sun::star::uno::TypeDescription protPropCommit_;
    OutgoingRequests outgoingRequests_;
    osl::Condition passive_;
        // to guarantee that passive_ is eventually set (to avoid deadlock, see
        // dispose), activeCalls_ only counts those calls for which it can be
        // guaranteed that incrementActiveCalls is indeed followed by
        // decrementActiveCalls, without an intervening exception
    osl::Condition terminated_;

    osl::Mutex mutex_;
    State state_;
    Listeners listeners_;
    uno_ThreadPool threadPool_;
    rtl::Reference< Writer > writer_;
    rtl::Reference< Reader > reader_;
    bool currentContextMode_;
    Stubs stubs_;
    std::size_t proxies_;
    std::size_t calls_;
    bool normalCall_;
    std::size_t activeCalls_;

    // Only accessed from reader_ thread:
    Mode mode_;
    sal_Int32 random_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
