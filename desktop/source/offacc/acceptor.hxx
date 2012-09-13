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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/connection/XAcceptor.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/bridge/XInstanceProvider.hpp>
#include <com/sun/star/bridge/XBridgeFactory2.hpp>
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
    Reference< XComponentContext >    m_rContext;
    Reference< XAcceptor >            m_rAcceptor;
    Reference< XBridgeFactory2 >      m_rBridgeFactory;

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
