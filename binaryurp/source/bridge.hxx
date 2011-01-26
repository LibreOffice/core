/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
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

#ifndef INCLUDED_BINARYURP_SOURCE_BRIDGE_HXX
#define INCLUDED_BINARYURP_SOURCE_BRIDGE_HXX

#include "sal/config.h"

#include <cstddef>
#include <list>
#include <map>
#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/bridge/XBridge.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "cppuhelper/implbase2.hxx"
#include "osl/conditn.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/environment.hxx"
#include "uno/mapping.hxx"
#include "uno/threadpool.h"

#include "outgoingrequest.hxx"
#include "outgoingrequests.hxx"
#include "writer.hxx"

namespace binaryurp {
    class BinaryAny;
    class BridgeFactory;
    class Proxy;
    class Reader;
}
namespace com { namespace sun { namespace star {
    namespace bridge { class XInstanceProvider; }
    namespace connection { class XConnection; }
    namespace lang { class XEventListener; }
    namespace uno {
        class Any;
        class TypeDescription;
        class UnoInterfaceReference;
        class XInterface;
    }
} } }
namespace rtl { class ByteSequence; }

namespace binaryurp {

class Bridge:
    public cppu::WeakImplHelper2<
        com::sun::star::bridge::XBridge, com::sun::star::lang::XComponent >,
    private boost::noncopyable
{
public:
    Bridge(
        rtl::Reference< BridgeFactory > const & factory,
        rtl::OUString const & name,
        com::sun::star::uno::Reference<
            com::sun::star::connection::XConnection > const & connection,
        com::sun::star::uno::Reference<
            com::sun::star::bridge::XInstanceProvider > const & provider);

    void start();

    // Internally waits for all incoming and outgoing remote calls to terminate,
    // so must not be called from within such a call:
    void terminate();

    com::sun::star::uno::Reference< com::sun::star::connection::XConnection >
    getConnection() const;

    com::sun::star::uno::Reference< com::sun::star::bridge::XInstanceProvider >
    getProvider() const;

    com::sun::star::uno::Mapping & getCppToBinaryMapping();

    BinaryAny mapCppToBinaryAny(com::sun::star::uno::Any const & cppAny);

    uno_ThreadPool getThreadPool() const;

    rtl::Reference< Writer > getWriter();

    com::sun::star::uno::UnoInterfaceReference registerIncomingInterface(
        rtl::OUString const & oid,
        com::sun::star::uno::TypeDescription const & type);

    rtl::OUString registerOutgoingInterface(
        com::sun::star::uno::UnoInterfaceReference const & object,
        com::sun::star::uno::TypeDescription const & type);

    com::sun::star::uno::UnoInterfaceReference findStub(
        rtl::OUString const & oid,
        com::sun::star::uno::TypeDescription const & type);

    void releaseStub(
        rtl::OUString const & oid,
        com::sun::star::uno::TypeDescription const & type);

    void resurrectProxy(Proxy & proxy);

    void revokeProxy(Proxy & proxy);

    void freeProxy(Proxy & proxy);

    void incrementCalls(bool normalCall) throw ();

    void decrementCalls();

    void incrementActiveCalls() throw ();

    void decrementActiveCalls() throw ();

    bool makeCall(
        rtl::OUString const & oid,
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
        rtl::OUString const & oid,
        com::sun::star::uno::TypeDescription const & type) const;

    void setCurrentContextMode();

    bool isCurrentContextMode();

private:
    virtual ~Bridge();

    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL getInstance(rtl::OUString const & sInstanceName)
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getName()
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getDescription()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL dispose()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addEventListener(
        com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >
            const & xListener)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeEventListener(
        com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >
            const & aListener)
        throw (com::sun::star::uno::RuntimeException);

    // Only called from reader_ thread:
    void sendCommitChangeRequest();

    // Only called from reader_ thread:
    void sendProtPropRequest(
        OutgoingRequest::Kind kind,
        std::vector< BinaryAny > const & inArguments);

    void makeReleaseCall(
        rtl::OUString const & oid,
        com::sun::star::uno::TypeDescription const & type);

    void sendRequest(
        rtl::ByteSequence const & tid, rtl::OUString const & oid,
        com::sun::star::uno::TypeDescription const & type,
        com::sun::star::uno::TypeDescription const & member,
        std::vector< BinaryAny > const & inArguments);

    void throwException(bool exception, BinaryAny const & value);

    com::sun::star::uno::Any mapBinaryToCppAny(BinaryAny const & binaryAny);

    bool becameUnused() const;

    void terminateWhenUnused(bool unused);

    typedef
        std::list<
            com::sun::star::uno::Reference<
                com::sun::star::lang::XEventListener > >
        Listeners;

    struct SubStub;

    typedef std::map< com::sun::star::uno::TypeDescription, SubStub > Stub;

    typedef std::map< rtl::OUString, Stub > Stubs;

    enum Mode {
        MODE_REQUESTED, MODE_REPLY_MINUS1, MODE_REPLY_0, MODE_REPLY_1,
        MODE_WAIT, MODE_NORMAL, MODE_NORMAL_WAIT };

    rtl::Reference< BridgeFactory > factory_;
    rtl::OUString name_;
    com::sun::star::uno::Reference< com::sun::star::connection::XConnection >
        connection_;
    com::sun::star::uno::Reference< com::sun::star::bridge::XInstanceProvider >
        provider_;
    com::sun::star::uno::Environment binaryUno_;
    com::sun::star::uno::Mapping cppToBinaryMapping_;
    com::sun::star::uno::Mapping binaryToCppMapping_;
    rtl::ByteSequence protPropTid_;
    rtl::OUString protPropOid_;
    com::sun::star::uno::TypeDescription protPropType_;
    com::sun::star::uno::TypeDescription protPropRequest_;
    com::sun::star::uno::TypeDescription protPropCommit_;
    uno_ThreadPool threadPool_;
    OutgoingRequests outgoingRequests_;

    osl::Mutex mutex_;
    Listeners listeners_;
    rtl::Reference< Writer > writer_;
    rtl::Reference< Reader > reader_;
    bool currentContextMode_;
    Stubs stubs_;
    std::size_t proxies_;
    std::size_t calls_;
    bool normalCall_;
    std::size_t activeCalls_;
    osl::Condition passive_;
        // to guarantee that passive_ is eventually set (to avoid deadlock, see
        // dispose), activeCalls_ only counts those calls for which it can be
        // guaranteed that incrementActiveCalls is indeed followed by
        // decrementActiveCalls, without an intervening exception
    bool terminated_;

    // Only accessed from reader_ thread:
    Mode mode_;
    sal_Int32 random_;
};

}

#endif
