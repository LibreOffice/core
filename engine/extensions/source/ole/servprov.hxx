/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <functional>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

#include "ole2uno.hxx"
#include "unoconversionutilities.hxx"

/// @throws Exception
css::uno::Reference< css::uno::XInterface> ConverterProvider_CreateInstance2(  const css::uno::Reference<css::lang::XMultiServiceFactory> & xSMgr);
/// @throws Exception
css::uno::Reference< css::uno::XInterface> ConverterProvider_CreateInstanceVar1(   const css::uno::Reference<css::lang::XMultiServiceFactory> & xSMgr);
/// @throws Exception
css::uno::Reference<css::uno::XInterface> OleClient_CreateInstance( const css::uno::Reference<css::lang::XMultiServiceFactory> & xSMgr);
/// @throws Exception
css::uno::Reference<css::uno::XInterface> OleServer_CreateInstance( const css::uno::Reference<css::lang::XMultiServiceFactory> & xSMgr);

/*****************************************************************************

    OneInstanceOleWrapper

    Provides a single UNO object as OLE object.

    Acts as a COM class factory. When IClassFactory::CreateInstance is being called
    then it maps the XInstance member it to a COM object.

*****************************************************************************/

class OneInstanceOleWrapper : public IClassFactory
{
public:

    OneInstanceOleWrapper( const css::uno::Reference<css::lang::XMultiServiceFactory>& smgr,
                           std::function<const css::uno::Reference<css::uno::XInterface>()> xInstFunction );
    virtual ~OneInstanceOleWrapper();

    bool registerClass(GUID const * pGuid);
    bool deregisterClass();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

    /* IClassFactory methods */
    STDMETHOD(CreateInstance)(IUnknown* punkOuter, REFIID riid, void** ppv) override;
    STDMETHOD(LockServer)(BOOL fLock) override;

protected:
    oslInterlockedCount m_refCount;
    std::function<const css::uno::Reference<css::uno::XInterface>()> m_xInstFunction;
    DWORD               m_factoryHandle;
    css::uno::Reference<css::bridge::XBridgeSupplier2> m_bridgeSupplier;
    css::uno::Reference<css::lang::XMultiServiceFactory> m_smgr;
};

// Implementation of the UNO service com.sun.star.bridge.OleBridgeSupplier2.

// This class realizes the service com.sun.star.bridge.OleBridgeSupplier2 and
// com.sun.star.bridge.OleBridgeSupplierVar1. The class implements XBridgeSupplier2 which
// interface does not need a Machine Id in its createBridge function anymore,
// If a UNO interface is to be converted then the member m_nUnoWrapperClass determines
// what wrapper class is to be used. There are currently InterfaceOleWrapper and
// UnoObjectWrapperRemoteOpt. The first is used for the OleBridgeSupplier2 and the
// latter for OleBridgeSupplierVar1.
// The m_nComWrapperClass specifies the class which is used as wrapper for COM interfaces.
// Currently there is only one class available (IUnknownWrapper).
class OleConverter : public cppu::WeakImplHelper<css::bridge::XBridgeSupplier2, css::lang::XInitialization, css::lang::XServiceInfo>,
                            public UnoConversionUtilities<OleConverter>
{
public:
    explicit OleConverter( const css::uno::Reference<css::lang::XMultiServiceFactory>& smgr);
    OleConverter( const  css::uno::Reference<css::lang::XMultiServiceFactory>& smgr, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass );
    virtual ~OleConverter() override;

    // XBridgeSupplier2 ---------------------------------------------------

    cpo::uno::Any SAL_CALL createBridge(const cpo::uno::Any& modelDepObject,
                                const css::uno::Sequence<sal_Int8>& ProcessId,
                                sal_Int16 sourceModelType,
                                sal_Int16 destModelType) override;

    // XInitialization
    void SAL_CALL initialize( const css::uno::Sequence< cpo::uno::Any >& aArguments ) override;

    OUString SAL_CALL getImplementationName() override;

    bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // UnoConversionUtilities
    css::uno::Reference< css::uno::XInterface > createUnoWrapperInstance() override;
    css::uno::Reference< css::uno::XInterface > createComWrapperInstance() override;
protected:

};

// Implementation of the UNO service com.sun.star.bridge.OleObjectFactory.

class OleClient : public cppu::WeakImplHelper<css::lang::XMultiServiceFactory, css::lang::XServiceInfo>,
                  public UnoConversionUtilities<OleClient>
{
public:
    explicit OleClient( const css::uno::Reference<css::lang::XMultiServiceFactory>& smgr);
    ~OleClient() override;

    // XMultiServiceFactory
    css::uno::Reference<css::uno::XInterface> SAL_CALL createInstance(const OUString& ServiceSpecifier) override;
    css::uno::Reference<css::uno::XInterface> SAL_CALL createInstanceWithArguments(const OUString& ServiceSpecifier, const css::uno::Sequence< cpo::uno::Any >& Arguments) override;
    css::uno::Sequence< OUString >    SAL_CALL getAvailableServiceNames() override;

    OUString SAL_CALL getImplementationName() override;

    bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // UnoConversionUtilities
    css::uno::Reference< css::uno::XInterface > createUnoWrapperInstance() override;
    css::uno::Reference< css::uno::XInterface > createComWrapperInstance() override;

protected:
    css::uno::Reference<css::bridge::XBridgeSupplier2> m_bridgeSupplier;
};

/*****************************************************************************

    OleServer

    Implementation of the UNO service com.sun.star.bridge.OleApplicationRegistration.
    Register the calling application as OLE automation server for
    standard OLE object. The objects will be registered while instantiating
    this implementation and deregistered, if this implementation is destroyed.

*****************************************************************************/

class OleServer : public cppu::WeakImplHelper<css::lang::XServiceInfo>
{
public:
    explicit OleServer( const css::uno::Reference<css::lang::XMultiServiceFactory> &smgr);
    ~OleServer() override;

    OUString SAL_CALL getImplementationName() override;

    bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

protected:
    bool provideInstance(std::function<const css::uno::Reference<css::uno::XInterface>()> xInstFunction, GUID const * guid);

    std::list< OneInstanceOleWrapper* > m_wrapperList;
    css::uno::Reference< css::bridge::XBridgeSupplier2 >   m_bridgeSupplier;

    css::uno::Reference<css::lang::XMultiServiceFactory> m_smgr;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
