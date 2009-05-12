/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: servprov.cxx,v $
 * $Revision: 1.12 $
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
    return osl_incrementInterlockedCount( &m_refCount);
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
