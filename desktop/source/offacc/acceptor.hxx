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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/connection/XAcceptor.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/bridge/XInstanceProvider.hpp>
#include <com/sun/star/bridge/XBridgeFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <rtl/logfile.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <comphelper/weakbag.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <osl/thread.hxx>


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::registry;

namespace desktop {

class  Acceptor
    : public ::cppu::WeakImplHelper2<XServiceInfo, XInitialization>
{
private:
    static const sal_Char *serviceName;
    static const sal_Char *implementationName;
    static const sal_Char *supportedServiceNames[];

    static Mutex m_aMutex;

    oslThread m_thread;
    comphelper::WeakBag< com::sun::star::bridge::XBridge > m_bridges;

    Condition m_cEnable;

    Reference< XMultiServiceFactory > m_rSMgr;
    Reference< XInterface >           m_rContext;
    Reference< XAcceptor >            m_rAcceptor;
    Reference< XBridgeFactory >       m_rBridgeFactory;

    OUString m_aAcceptString;
    OUString m_aConnectString;
    OUString m_aProtocol;

    sal_Bool m_bInit;
    bool m_bDying;

public:
    Acceptor( const Reference< XMultiServiceFactory >& aFactory );
    virtual ~Acceptor();

    void SAL_CALL run();

    // XService info
    static  OUString                    impl_getImplementationName();
    virtual OUString           SAL_CALL getImplementationName()
        throw (RuntimeException);
    static  Sequence<OUString>          impl_getSupportedServiceNames();
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);
    virtual sal_Bool           SAL_CALL supportsService( const OUString& aName )
        throw (RuntimeException);

    // XInitialize
    virtual void SAL_CALL initialize( const Sequence<Any>& aArguments )
        throw ( Exception );

    static  Reference<XInterface> impl_getInstance( const Reference< XMultiServiceFactory >& aFactory );
};

class AccInstanceProvider : public ::cppu::WeakImplHelper1<XInstanceProvider>
{
private:
    Reference<XMultiServiceFactory> m_rSMgr;
    Reference<XConnection> m_rConnection;

public:
    AccInstanceProvider(const Reference< XMultiServiceFactory >& aFactory,
                        const Reference< XConnection >& rConnection);
    virtual ~AccInstanceProvider();

    // XInstanceProvider
    virtual Reference<XInterface> SAL_CALL getInstance (const OUString& aName )
        throw ( NoSuchElementException );
};


} //namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
