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

#ifndef INCLUDED_EMBEDSERV_SOURCE_INC_SERVPROV_HXX
#define INCLUDED_EMBEDSERV_SOURCE_INC_SERVPROV_HXX

#include "common.h"
#include <oleidl.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>


class EmbedProviderFactory_Impl;

class EmbedServer_Impl: public cppu::WeakImplHelper<css::lang::XServiceInfo>
{
public:
    EmbedServer_Impl( const css::uno::Reference< css::lang::XMultiServiceFactory > &xFactory );
    virtual ~EmbedServer_Impl();

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:

    CComPtr< EmbedProviderFactory_Impl > m_pOLEFactories[ SUPPORTED_FACTORIES_NUM ];
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;
};

class EmbedProviderFactory_Impl : public IClassFactory
{
public:

    EmbedProviderFactory_Impl( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory, const GUID* pGuid);
    virtual ~EmbedProviderFactory_Impl();

    sal_Bool registerClass();
    sal_Bool deregisterClass();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    /* IClassFactory methods */
    STDMETHOD(CreateInstance)(IUnknown FAR* punkOuter, REFIID riid, void FAR* FAR* ppv);
    STDMETHOD(LockServer)(int fLock);

protected:

    oslInterlockedCount m_refCount;
    GUID                m_guid;
    DWORD               m_factoryHandle;

    css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
