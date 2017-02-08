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

#include <vector>

#ifdef __MINGW32__
#define INITGUID
#include <initguid.h>
#else
#include "ole2uno.hxx"
#include "unoconversionutilities.hxx"
#endif
#include "servprov.hxx"
#include "unoobjw.hxx"
#include "oleobjw.hxx"
#include <com/sun/star/script/CannotConvertException.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/any.hxx>

using namespace cppu;
using namespace osl;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::ModelDependent;


namespace ole_adapter
{

#include <initguid.h>

// GUID used since 5.2 ( src569 m)
// {82154420-0FBF-11d4-8313-005004526AB4}
DEFINE_GUID(OID_ServiceManager, 0x82154420, 0xfbf, 0x11d4, 0x83, 0x13, 0x0, 0x50, 0x4, 0x52, 0x6a, 0xb4);

/*****************************************************************************

    class implementation ProviderOleWrapper_Impl

*****************************************************************************/

ProviderOleWrapper_Impl::ProviderOleWrapper_Impl(const Reference<XMultiServiceFactory>& smgr,
                                                 const Reference<XSingleServiceFactory>& xSFact, GUID* pGuid)
    : m_xSingleServiceFactory(xSFact),
      m_smgr( smgr)
{
    m_guid = *pGuid;

    Reference<XInterface> xInt = smgr->createInstance(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.bridge.oleautomation.BridgeSupplier"));

    if (xInt.is())
    {
        Any a= xInt->queryInterface( cppu::UnoType<XBridgeSupplier2>::get() );
        a >>= m_bridgeSupplier;

    }
}

ProviderOleWrapper_Impl::~ProviderOleWrapper_Impl()
{
}

bool ProviderOleWrapper_Impl::registerClass()
{
    HRESULT hresult;

    o2u_attachCurrentThread();

    hresult = CoRegisterClassObject(
            m_guid,
            this,
            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
            REGCLS_MULTIPLEUSE,
            &m_factoryHandle);

    return (hresult == NOERROR);
}

bool ProviderOleWrapper_Impl::deregisterClass()
{
    HRESULT hresult = CoRevokeClassObject(m_factoryHandle);

    return (hresult == NOERROR);
}

STDMETHODIMP ProviderOleWrapper_Impl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppv = static_cast<IUnknown*>(static_cast<IClassFactory*>(this));
        return NOERROR;
    }
    else if (IsEqualIID(riid, IID_IClassFactory))
    {
        AddRef();
        *ppv = static_cast<IClassFactory*>(this);
        return NOERROR;
    }

    *ppv = nullptr;
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) ProviderOleWrapper_Impl::AddRef()
{
    return osl_atomic_increment( &m_refCount);
}

STDMETHODIMP_(ULONG) ProviderOleWrapper_Impl::Release()
{
    MutexGuard aGuard( Mutex::getGlobalMutex());
    ULONG refCount = --m_refCount;
    if (m_refCount == 0)
    {
        delete this;
    }

    return refCount;
}

STDMETHODIMP ProviderOleWrapper_Impl::CreateInstance(IUnknown FAR* punkOuter,
                                                     REFIID riid,
                                                     void FAR* FAR* ppv)
{
    HRESULT ret = ResultFromScode(E_UNEXPECTED);
    punkOuter = nullptr;

    Reference<XInterface> xInstance;

    if (m_xSingleServiceFactory.is())
    {
        xInstance = m_xSingleServiceFactory->createInstance();

        if (xInstance.is())
        {
            Any usrAny(&xInstance, cppu::UnoType<decltype(xInstance)>::get());

            sal_uInt8 arId[16];
            rtl_getGlobalProcessId( arId );
            Any oleAny = m_bridgeSupplier->createBridge(usrAny,
                                        Sequence<sal_Int8>(reinterpret_cast<sal_Int8*>(arId), 16),
                                        UNO,
                                        OLE);


            if (auto v = o3tl::tryAccess<sal_uIntPtr>(oleAny))
            {
                VARIANT* pVariant = reinterpret_cast<VARIANT*>(*v);

                if (pVariant->vt == VT_DISPATCH)
                {
                    ret = pVariant->pdispVal->QueryInterface(riid, ppv);
                }

                VariantClear(pVariant);
                CoTaskMemFree(pVariant);
            }
        }
    }

    return ret;
}

STDMETHODIMP ProviderOleWrapper_Impl::LockServer(int /*fLock*/)
{
    return NOERROR;
}

/*****************************************************************************

    class implementation OneInstanceOleWrapper_Impl

*****************************************************************************/

OneInstanceOleWrapper_Impl::OneInstanceOleWrapper_Impl(  const Reference<XMultiServiceFactory>& smgr,
                                                         const Reference<XInterface>& xInst,
                                                         GUID* pGuid )
    : m_refCount(0)
    , m_xInst(xInst)
    , m_factoryHandle(0)
    , m_smgr(smgr)
{
    m_guid = *pGuid;

    Reference<XInterface> xInt = m_smgr->createInstance(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.bridge.oleautomation.BridgeSupplier"));

    if (xInt.is())
    {
        Any a= xInt->queryInterface( cppu::UnoType<XBridgeSupplier2>::get() );
        a >>= m_bridgeSupplier;
    }
}

OneInstanceOleWrapper_Impl::~OneInstanceOleWrapper_Impl()
{
}

bool OneInstanceOleWrapper_Impl::registerClass()
{
    HRESULT hresult;

    o2u_attachCurrentThread();

    hresult = CoRegisterClassObject(
            m_guid,
            this,
            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
            REGCLS_MULTIPLEUSE,
            &m_factoryHandle);

    return (hresult == NOERROR);
}

bool OneInstanceOleWrapper_Impl::deregisterClass()
{
    HRESULT hresult1 = NOERROR;

    HRESULT hresult2 = CoRevokeClassObject(m_factoryHandle);

    return (hresult1 == NOERROR && hresult2 == NOERROR);
}

STDMETHODIMP OneInstanceOleWrapper_Impl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppv = static_cast<IUnknown*>(static_cast<IClassFactory*>(this));
        return NOERROR;
    }
    else if (IsEqualIID(riid, IID_IClassFactory))
    {
        AddRef();
        *ppv = static_cast<IClassFactory*>(this);
        return NOERROR;
    }

    *ppv = nullptr;
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) OneInstanceOleWrapper_Impl::AddRef()
{
    return osl_atomic_increment( &m_refCount);
}

STDMETHODIMP_(ULONG) OneInstanceOleWrapper_Impl::Release()
{
    MutexGuard oGuard( Mutex::getGlobalMutex());
    ULONG refCount = --m_refCount;
    if ( m_refCount == 0)
    {
        delete this;
    }

    return refCount;
}

STDMETHODIMP OneInstanceOleWrapper_Impl::CreateInstance(IUnknown FAR* punkOuter,
                                                         REFIID riid,
                                                         void FAR* FAR* ppv)
{
    HRESULT ret = ResultFromScode(E_UNEXPECTED);
    punkOuter = nullptr;

    if (m_xInst.is())
    {
        Any usrAny(&m_xInst, cppu::UnoType<decltype(m_xInst)>::get());
        sal_uInt8 arId[16];
        rtl_getGlobalProcessId( arId);
        Any oleAny = m_bridgeSupplier->createBridge(usrAny,
                                        Sequence<sal_Int8>( reinterpret_cast<sal_Int8*>(arId), 16),
                                        UNO,
                                        OLE);


        if (auto v = o3tl::tryAccess<sal_uIntPtr>(oleAny))
        {
            VARIANT* pVariant = reinterpret_cast<VARIANT*>(*v);

            if ((pVariant->vt == VT_UNKNOWN) || (pVariant->vt == VT_DISPATCH))
            {
                ret = pVariant->punkVal->QueryInterface(riid, ppv);
            }

            VariantClear(pVariant);
            CoTaskMemFree(pVariant);
        }
    }

    return ret;
}

STDMETHODIMP OneInstanceOleWrapper_Impl::LockServer(int /*fLock*/)
{
    return NOERROR;
}


/*****************************************************************************

    class implementation OleConverter_Impl2

*****************************************************************************/

OleConverter_Impl2::OleConverter_Impl2( const Reference<XMultiServiceFactory> &smgr):
    UnoConversionUtilities<OleConverter_Impl2>( smgr)

{
}

// The XMultiServiceFactory is later set by XInitialization
OleConverter_Impl2::OleConverter_Impl2( const  Reference<XMultiServiceFactory>& smgr, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass ):
    UnoConversionUtilities<OleConverter_Impl2>( smgr, unoWrapperClass, comWrapperClass  )

{
}

OleConverter_Impl2::~OleConverter_Impl2()
{
}

// XBridgeSupplier --------------------------------------------------------------
Any SAL_CALL OleConverter_Impl2::createBridge(const Any& modelDepObject,
                                       const Sequence< sal_Int8 >& ProcessId,
                                       sal_Int16 sourceModelType,
                                       sal_Int16 destModelType)
{
    Any ret;
    sal_uInt8 arId[16];
    rtl_getGlobalProcessId( arId );

    Sequence< sal_Int8 > seqProcessId( reinterpret_cast<sal_Int8*>(arId), 16);

    if ( seqProcessId == ProcessId)
    {
        if (sourceModelType == UNO)
        {
            if (destModelType == UNO)
            {
                // same model -> copy value only
                ret = modelDepObject;
            }
            else if (destModelType == OLE)
            {
                // convert UNO any into variant
                VARIANT* pVariant = static_cast<VARIANT*>(CoTaskMemAlloc(sizeof(VARIANT)));
                VariantInit( pVariant);
                try
                {
                    anyToVariant( pVariant, modelDepObject);
                }
                catch(...)
                {
                    CoTaskMemFree(pVariant);
                    throw IllegalArgumentException();
                }
                ret.setValue(static_cast<void*>(&pVariant), cppu::UnoType<sal_uIntPtr>::get());
            }
            else
                throw IllegalArgumentException();
        }
        else if (sourceModelType == OLE)
        {
            auto v = o3tl::tryAccess<sal_uIntPtr>(modelDepObject);
            if (!v)
            {
                throw IllegalArgumentException();
            }
            else if (destModelType == OLE)
            {
                // same model -> copy value only
                VARIANT* pVariant = static_cast<VARIANT*>(CoTaskMemAlloc(sizeof(VARIANT)));

                if (NOERROR != VariantCopy(pVariant, reinterpret_cast<VARIANT*>(*v)))
                {
                    CoTaskMemFree(pVariant);
                    throw(IllegalArgumentException());
                }
                else
                {
                    ret.setValue(static_cast<void*>(&pVariant), cppu::UnoType<sal_uIntPtr>::get());
                }
            }
            else if (destModelType == UNO)
            {
                // convert variant into UNO any
                VARIANT* pVariant = reinterpret_cast<VARIANT*>(*v);
                try
                {
                    variantToAny(pVariant, ret);
                }
                catch (const CannotConvertException & e)
                {
                    throw IllegalArgumentException(
                        e.Message, nullptr, -1);
                }
            }
            else
                throw IllegalArgumentException();

        }
        else
            throw IllegalArgumentException();
    }

    return ret;
}

OUString OleConverter_Impl2::getImplementationName()
{
    return m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL
        ? OUString("com.sun.star.comp.ole.OleConverter2")
        : OUString("com.sun.star.comp.ole.OleConverterVar1");
}

sal_Bool OleConverter_Impl2::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> OleConverter_Impl2::getSupportedServiceNames()
{
    if (m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL)
    {
        return css::uno::Sequence<OUString>{
            "com.sun.star.bridge.OleBridgeSupplier2",
            "com.sun.star.bridge.oleautomation.BridgeSupplier"};
    }
    return css::uno::Sequence<OUString>{
        "com.sun.star.bridge.OleBridgeSupplierVar1"};
}

// XInitialize ------------------------------------------------------------------------------
// the first argument is an XMultiServiceFactory if at all
void SAL_CALL OleConverter_Impl2::initialize( const Sequence< Any >& aArguments )
{
    if( aArguments.getLength() == 1 && aArguments[0].getValueTypeClass() == TypeClass_INTERFACE)
    {
        Reference < XInterface > xInt;
        aArguments[0] >>= xInt;
        Reference <XMultiServiceFactory> xMulti( xInt, UNO_QUERY);
        m_smgrRemote= xMulti;
    }
}

// UnoConversionUtilities -------------------------------------------------------------------
Reference< XInterface > OleConverter_Impl2::createUnoWrapperInstance()
{
    if( m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new InterfaceOleWrapper_Impl(
                                m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else if( m_nUnoWrapperClass == UNO_OBJECT_WRAPPER_REMOTE_OPT)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new UnoObjectWrapperRemoteOpt(
                                m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else
        return Reference<XInterface>();
}

Reference< XInterface > OleConverter_Impl2::createComWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new IUnknownWrapper_Impl(
                            m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}


/*****************************************************************************

    class implementation OleClient_Impl

*****************************************************************************/

OleClient_Impl::OleClient_Impl( const Reference<XMultiServiceFactory>& smgr):
    UnoConversionUtilities<OleClient_Impl>( smgr)
{
    Reference<XInterface> xInt;// = m_smgr->createInstance(L"com.sun.star.bridge.OleBridgeSupplier2");

    if (xInt.is())
    {
        Any a= xInt->queryInterface(cppu::UnoType<XBridgeSupplier2>::get() );
        a >>= m_bridgeSupplier;
    }
}

OleClient_Impl::~OleClient_Impl()
{
}

Sequence< OUString >    SAL_CALL OleClient_Impl::getAvailableServiceNames()
{
    Sequence< OUString > ret;

    return ret;
}

OUString OleClient_Impl::getImplementationName()
{
    return OUString("com.sun.star.comp.ole.OleClient");
}

sal_Bool OleClient_Impl::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> OleClient_Impl::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{
        "com.sun.star.bridge.OleObjectFactory",
        "com.sun.star.bridge.oleautomation.Factory"};
}

Reference<XInterface> SAL_CALL OleClient_Impl::createInstance(const OUString& ServiceSpecifier)
{
    Reference<XInterface>   ret;
    HRESULT         result;
    IUnknown*       pUnknown = nullptr;
    CLSID           classId;

    o2u_attachCurrentThread();

    result = CLSIDFromProgID(
                  reinterpret_cast<LPCWSTR>(ServiceSpecifier.getStr()),     //Pointer to the ProgID
                  &classId);                        //Pointer to the CLSID


    if (result == NOERROR)
    {
        result = CoCreateInstance(
                      classId,              //Class identifier (CLSID) of the object
                      nullptr,              //Pointer to whether object is or isn't part of an aggregate
                      CLSCTX_SERVER,  //Context for running executable code
                      IID_IUnknown,         //Reference to the identifier of the interface
                      reinterpret_cast<void**>(&pUnknown)); //Address of output variable that receives
                                                  // the interface pointer requested in riid
    }

    if (pUnknown != nullptr)
    {
        Any any;
        CComVariant variant;

        V_VT(&variant) = VT_UNKNOWN;
        V_UNKNOWN(&variant) = pUnknown;
        // AddRef for Variant
        pUnknown->AddRef();

        // When the object is wrapped, then its refcount is increased
        variantToAny(&variant, any);
        if (any.getValueTypeClass() == TypeClass_INTERFACE)
        {
            any >>= ret;
        }
        pUnknown->Release(); // CoCreateInstance
    }

    return ret;
}

Reference<XInterface> SAL_CALL OleClient_Impl::createInstanceWithArguments(const OUString& ServiceSpecifier, const Sequence< Any >& /*Arguments*/)
{
    return createInstance( ServiceSpecifier);
}

// UnoConversionUtilities -----------------------------------------------------------------------------
Reference< XInterface > OleClient_Impl::createUnoWrapperInstance()
{
    if( m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new InterfaceOleWrapper_Impl(
                                m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else if( m_nUnoWrapperClass == UNO_OBJECT_WRAPPER_REMOTE_OPT)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new UnoObjectWrapperRemoteOpt(
                                m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else
        return Reference< XInterface>();
}
// UnoConversionUtilities -----------------------------------------------------------------------------
Reference< XInterface > OleClient_Impl::createComWrapperInstance( )
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new IUnknownWrapper_Impl(
                            m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}


/*****************************************************************************

    class implementation OleServer_Impl

*****************************************************************************/

OleServer_Impl::OleServer_Impl( const Reference<XMultiServiceFactory>& smgr):
    m_smgr( smgr)
{
    Reference<XInterface> xInt = m_smgr->createInstance(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.bridge.oleautomation.BridgeSupplier"));

    if (xInt.is())
    {
        Any a= xInt->queryInterface( cppu::UnoType<XBridgeSupplier2>::get() );
        a >>= m_bridgeSupplier;
    }

    bool ret = provideInstance( m_smgr, const_cast<GUID*>(&OID_ServiceManager) );
    (void)ret;
}

OleServer_Impl::~OleServer_Impl()
{
    while (!m_wrapperList.empty())
    {
        (*m_wrapperList.begin())->deregisterClass();
        (*m_wrapperList.begin())->Release();
        m_wrapperList.pop_front();
    }
}

OUString OleServer_Impl::getImplementationName()
{
    return OUString("com.sun.star.comp.ole.OleServer");
}

sal_Bool OleServer_Impl::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> OleServer_Impl::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{
        "com.sun.star.bridge.OleApplicationRegistration",
        "com.sun.star.bridge.oleautomation.ApplicationRegistration"};
}

bool OleServer_Impl::provideService(const Reference<XSingleServiceFactory>& xSFact, GUID* guid)
{
    IClassFactoryWrapper* pFac = new ProviderOleWrapper_Impl( m_smgr, xSFact, guid);

    pFac->AddRef();

    m_wrapperList.push_back(pFac);

    return pFac->registerClass();
}

bool OleServer_Impl::provideInstance(const Reference<XInterface>& xInst, GUID* guid)
{
    IClassFactoryWrapper* pFac =
        new OneInstanceOleWrapper_Impl( m_smgr, xInst, guid );

    pFac->AddRef();
    m_wrapperList.push_back(pFac);

    return pFac->registerClass();
}


} // end namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
