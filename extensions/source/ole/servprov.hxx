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

#pragma once

#include <functional>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

#include "ole2uno.hxx"
#include "unoconversionutilities.hxx"

using namespace com::sun::star::bridge;
using namespace cppu;
using namespace std;

/// @throws Exception
Reference< XInterface> ConverterProvider_CreateInstance2(  const Reference<XMultiServiceFactory> & xSMgr);
/// @throws Exception
Reference< XInterface> ConverterProvider_CreateInstanceVar1(   const Reference<XMultiServiceFactory> & xSMgr);
/// @throws Exception
Reference<XInterface> OleClient_CreateInstance( const Reference<XMultiServiceFactory> & xSMgr);
/// @throws Exception
Reference<XInterface> OleServer_CreateInstance( const Reference<XMultiServiceFactory> & xSMgr);

/*****************************************************************************

    OneInstanceOleWrapper

    Provides a single UNO object as OLE object.

    Acts as a COM class factory. When IClassFactory::CreateInstance is being called
    then it maps the XInstance member it to a COM object.

*****************************************************************************/

class OneInstanceOleWrapper : public IClassFactory
{
public:

    OneInstanceOleWrapper( const Reference<XMultiServiceFactory>& smgr,
                           std::function<const Reference<XInterface>()> xInstFunction );
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
    std::function<const Reference<XInterface>()> m_xInstFunction;
    DWORD               m_factoryHandle;
    Reference<XBridgeSupplier2> m_bridgeSupplier;
    Reference<XMultiServiceFactory> m_smgr;
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
class OleConverter : public WeakImplHelper<XBridgeSupplier2, XInitialization, css::lang::XServiceInfo>,
                            public UnoConversionUtilities<OleConverter>
{
public:
    explicit OleConverter( const Reference<XMultiServiceFactory>& smgr);
    OleConverter( const  Reference<XMultiServiceFactory>& smgr, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass );
    virtual ~OleConverter() override;

    // XBridgeSupplier2 ---------------------------------------------------

    Any SAL_CALL createBridge(const Any& modelDepObject,
                                const Sequence<sal_Int8>& ProcessId,
                                sal_Int16 sourceModelType,
                                sal_Int16 destModelType) override;

    // XInitialization
    void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // UnoConversionUtilities
    Reference< XInterface > createUnoWrapperInstance() override;
    Reference< XInterface > createComWrapperInstance() override;
protected:

};

// Implementation of the UNO service com.sun.star.bridge.OleObjectFactory.

class OleClient : public WeakImplHelper<XMultiServiceFactory, css::lang::XServiceInfo>,
                  public UnoConversionUtilities<OleClient>
{
public:
    explicit OleClient( const Reference<XMultiServiceFactory>& smgr);
    ~OleClient() override;

    // XMultiServiceFactory
    Reference<XInterface> SAL_CALL createInstance(const OUString& ServiceSpecifier) override;
    Reference<XInterface> SAL_CALL createInstanceWithArguments(const OUString& ServiceSpecifier, const Sequence< Any >& Arguments) override;
    Sequence< OUString >    SAL_CALL getAvailableServiceNames() override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // UnoConversionUtilities
    Reference< XInterface > createUnoWrapperInstance() override;
    Reference< XInterface > createComWrapperInstance() override;

protected:
    Reference<XBridgeSupplier2> m_bridgeSupplier;
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
    explicit OleServer( const Reference<XMultiServiceFactory> &smgr);
    ~OleServer() override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

protected:
    bool provideInstance(std::function<const Reference<XInterface>()> xInstFunction, GUID const * guid);

    list< OneInstanceOleWrapper* > m_wrapperList;
    Reference< XBridgeSupplier2 >   m_bridgeSupplier;

    Reference<XMultiServiceFactory> m_smgr;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
