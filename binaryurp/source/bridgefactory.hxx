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

#ifndef INCLUDED_BINARYURP_SOURCE_BRIDGEFACTORY_HXX
#define INCLUDED_BINARYURP_SOURCE_BRIDGEFACTORY_HXX

#include "sal/config.h"

#include <list>
#include <map>

#include "boost/noncopyable.hpp"
#include "com/sun/star/bridge/XBridgeFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "cppuhelper/compbase2.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star {
    namespace connection { class XConnection; }
    namespace uno {
        class XComponentContext;
        class XInterface;
    }
} } }

namespace binaryurp {

// That BridgeFactory derives from XComponent appears to be a historic mistake;
// the implementation does not care about a disposed state:

typedef
    cppu::WeakComponentImplHelper2<
        com::sun::star::lang::XServiceInfo,
        com::sun::star::bridge::XBridgeFactory >
    BridgeFactoryBase;

class BridgeFactory:
    private osl::Mutex, public BridgeFactoryBase, private boost::noncopyable
{
public:
    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL static_create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & xContext)
        SAL_THROW((com::sun::star::uno::Exception));

    static rtl::OUString SAL_CALL static_getImplementationName();

    static com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    static_getSupportedServiceNames();

    void removeBridge(
        com::sun::star::uno::Reference< com::sun::star::bridge::XBridge >
            const & bridge);

    using BridgeFactoryBase::acquire;
    using BridgeFactoryBase::release;

private:
    explicit BridgeFactory(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context);

    virtual ~BridgeFactory();

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & ServiceName)
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Reference< com::sun::star::bridge::XBridge >
    SAL_CALL createBridge(
        rtl::OUString const & sName, rtl::OUString const & sProtocol,
        com::sun::star::uno::Reference<
            com::sun::star::connection::XConnection > const & aConnection,
        com::sun::star::uno::Reference<
            com::sun::star::bridge::XInstanceProvider > const &
                anInstanceProvider)
        throw (
            com::sun::star::bridge::BridgeExistsException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Reference< com::sun::star::bridge::XBridge >
    SAL_CALL getBridge(
        rtl::OUString const & sName)
        throw (com::sun::star::uno::RuntimeException);

    virtual
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference< com::sun::star::bridge::XBridge > >
    SAL_CALL getExistingBridges() throw (com::sun::star::uno::RuntimeException);

    typedef
        std::list<
            com::sun::star::uno::Reference< com::sun::star::bridge::XBridge > >
        BridgeList;

    typedef
        std::map<
            rtl::OUString,
            com::sun::star::uno::Reference< com::sun::star::bridge::XBridge > >
        BridgeMap;

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        context_;
    BridgeList unnamed_;
    BridgeMap named_;
};

}

#endif
