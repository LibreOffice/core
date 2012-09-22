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
#if defined(_MSC_VER) && (_MSC_VER > 1310)
#pragma warning(disable : 4917 4555)
#endif

#include "stdafx.h"
#include "servprov.hxx"
#include "embeddoc.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/typeprovider.hxx>
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


// CoInitializeEx *
typedef DECLSPEC_IMPORT HRESULT (STDAPICALLTYPE *ptrCoInitEx)( LPVOID, DWORD);
// CoInitialize *
typedef DECLSPEC_IMPORT HRESULT (STDAPICALLTYPE *ptrCoInit)( LPVOID);

void o2u_attachCurrentThread()
{
    static CurThreadData oleThreadData;

    if ( oleThreadData.getData() != 0 )
    {
        HINSTANCE inst= LoadLibrary( _T("ole32.dll"));
        if( inst )
        {
            HRESULT hr;
            ptrCoInitEx initFuncEx= (ptrCoInitEx)GetProcAddress( inst, _T("CoInitializeEx"));
            if( initFuncEx)
                hr= initFuncEx( NULL, COINIT_MULTITHREADED);
            else
            {
                ptrCoInit initFunc= (ptrCoInit)GetProcAddress( inst,_T("CoInitialize"));
                if( initFunc)
                    hr= initFunc( NULL);
            }
        }
        oleThreadData.setData((void*)sal_True);
    }
}


//===============================================================================
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

// XInterface --------------------------------------------------
uno::Any SAL_CALL
EmbedServer_Impl::queryInterface(
    const uno::Type& aType )
    throw(
        uno::RuntimeException
    )
{
    uno::Any a=
        ::cppu::queryInterface(
            aType, static_cast<lang::XTypeProvider*>(this));
    if( a == uno::Any())
        return OWeakObject::queryInterface( aType);
    else
        return a;
}

void SAL_CALL EmbedServer_Impl::acquire(  ) throw(uno::RuntimeException)
{
    OWeakObject::acquire();
}

void SAL_CALL EmbedServer_Impl::release(  ) throw (uno::RuntimeException)
{
    OWeakObject::release();
}


// XTypeProvider --------------------------------------------------
uno::Sequence< uno::Type > SAL_CALL
EmbedServer_Impl::getTypes( )
    throw(
        uno::RuntimeException
    )
{
    static ::cppu::OTypeCollection *pCollection = 0;
    if( ! pCollection )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! pCollection )
        {
            static ::cppu::OTypeCollection collection(
                getCppuType(
                    reinterpret_cast<uno::Reference< uno::XWeak>*>(0)),
                getCppuType(
                    reinterpret_cast<
                    uno::Reference< lang::XTypeProvider>*>(0)));
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL EmbedServer_Impl::getImplementationId() throw(uno::RuntimeException)
{
    static ::cppu::OImplementationId *pId = 0;
    if( ! pId )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! pId )
        {
            static ::cppu::OImplementationId id( sal_False );
            pId = &id;
        }
    }
    return (*pId).getImplementationId();
}

//===============================================================================
// EmbedProviderFactory_Impl

EmbedProviderFactory_Impl::EmbedProviderFactory_Impl(const uno::Reference<lang::XMultiServiceFactory>& xFactory, const GUID* pGuid)
    : m_refCount( 0L )
    , m_xFactory( xFactory )
    , m_guid( *pGuid )
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

// Fix strange warnings about some
// ATL::CAxHostWindow::QueryInterface|AddRef|Releae functions.
// warning C4505: 'xxx' : unreferenced local function has been removed
#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
