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
#ifdef _MSC_VER
#pragma warning(disable : 4917 4555)
#endif

#include "stdafx.h"
#include "servprov.hxx"
#include "embeddoc.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/thread.h>

using namespace com::sun::star;

const GUID* guidList[ SUPPORTED_FACTORIES_NUM ] = {
    &OID_WriterTextServer,
    &OID_WriterOASISTextServer,
    &OID_CalcServer,
    &OID_CalcOASISServer,
    &OID_DrawingServer,
    &OID_DrawingOASISServer,
    &OID_PresentationServer,
    &OID_PresentationOASISServer,
    &OID_MathServer,
    &OID_MathOASISServer
};

class CurThreadData
{
    public:
        CurThreadData();
        virtual ~CurThreadData();

        sal_Bool SAL_CALL setData(void *pData);

        void* SAL_CALL getData();

    protected:
        oslThreadKey m_hKey;
};

CurThreadData::CurThreadData()
{
    m_hKey = osl_createThreadKey( (oslThreadKeyCallbackFunction)NULL );
}

CurThreadData::~CurThreadData()
{
    osl_destroyThreadKey(m_hKey);
}

sal_Bool CurThreadData::setData(void *pData)
{
    OSL_ENSURE( m_hKey, "No thread key!\n" );
    return (osl_setThreadKeyData(m_hKey, pData));
}

void *CurThreadData::getData()
{
    OSL_ENSURE( m_hKey, "No thread key!\n" );
    return (osl_getThreadKeyData(m_hKey));
}

void o2u_attachCurrentThread()
{
    static CurThreadData oleThreadData;

    if ( oleThreadData.getData() != 0 )
    {
        HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (!SUCCEEDED(hr))
        {   // FIXME: is it a problem that this ends up in STA currently?
            assert(RPC_E_CHANGED_MODE == hr);
            SAL_INFO("embedserv.ole",
                    "CoInitializeEx fail: probably thread is in STA already?");
        }
        oleThreadData.setData((void*)sal_True);
    }
}



// EmbedServer_Impl

EmbedServer_Impl::EmbedServer_Impl( const uno::Reference<lang::XMultiServiceFactory>& xFactory):
    m_xFactory( xFactory)
{
    for( int nInd = 0; nInd < SUPPORTED_FACTORIES_NUM; nInd++ )
    {
        m_pOLEFactories[nInd] = new EmbedProviderFactory_Impl( m_xFactory, guidList[nInd] );
        m_pOLEFactories[nInd]->registerClass();
    }
}

EmbedServer_Impl::~EmbedServer_Impl()
{
    for( int nInd = 0; nInd < SUPPORTED_FACTORIES_NUM; nInd++ )
    {
        if ( m_pOLEFactories[nInd] )
            m_pOLEFactories[nInd]->deregisterClass();
    }
}

OUString EmbedServer_Impl::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.ole.EmbedServer");
}

sal_Bool EmbedServer_Impl::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> EmbedServer_Impl::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<OUString>{
        "com.sun.star.document.OleEmbeddedServerRegistration"};
}

// EmbedProviderFactory_Impl

EmbedProviderFactory_Impl::EmbedProviderFactory_Impl(const uno::Reference<lang::XMultiServiceFactory>& xFactory, const GUID* pGuid)
    : m_refCount( 0L )
    , m_guid( *pGuid )
    , m_xFactory( xFactory )
{
}

EmbedProviderFactory_Impl::~EmbedProviderFactory_Impl()
{
}

sal_Bool EmbedProviderFactory_Impl::registerClass()
{
    HRESULT hresult;

    o2u_attachCurrentThread();

    hresult = CoRegisterClassObject(
            m_guid,
            this,
            CLSCTX_LOCAL_SERVER,
            REGCLS_MULTIPLEUSE,
            &m_factoryHandle);

    return (hresult == NOERROR);
}

sal_Bool EmbedProviderFactory_Impl::deregisterClass()
{
    HRESULT hresult = CoRevokeClassObject( m_factoryHandle );

    return (hresult == NOERROR);
}

STDMETHODIMP EmbedProviderFactory_Impl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppv = (IUnknown*) (IClassFactory*) this;
        return NOERROR;
    }
    else if (IsEqualIID(riid, IID_IClassFactory))
    {
        AddRef();
        *ppv = (IClassFactory*) this;
        return NOERROR;
    }

    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) EmbedProviderFactory_Impl::AddRef()
{
    return osl_atomic_increment( &m_refCount);
}

STDMETHODIMP_(ULONG) EmbedProviderFactory_Impl::Release()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex());
    sal_Int32 nCount = --m_refCount;
    if ( nCount == 0 )
    {
        delete this;
    }

    return nCount;
}

STDMETHODIMP EmbedProviderFactory_Impl::CreateInstance(IUnknown FAR* punkOuter,
                                                       REFIID riid,
                                                       void FAR* FAR* ppv)
{
    punkOuter = NULL;

    IUnknown* pEmbedDocument = (IUnknown*)(IPersistStorage*)( new EmbedDocument_Impl( m_xFactory, &m_guid ) );

    return pEmbedDocument->QueryInterface( riid, ppv );
}

STDMETHODIMP EmbedProviderFactory_Impl::LockServer( int /*fLock*/ )
{
    return NOERROR;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
