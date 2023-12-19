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

#include "ole2uno.hxx"
#include "unoconversionutilities.hxx"
#include "servprov.hxx"
#include "unoobjw.hxx"
#include "oleobjw.hxx"

#include <com/sun/star/script/CannotConvertException.hpp>
#include <comphelper/automationinvokedzone.hxx>
#include <comphelper/windowsdebugoutput.hxx>
#include <comphelper/windowserrorstring.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/any.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <officecfg/Office/Common.hxx>
#include <ooo/vba/XHelperInterface.hpp>
#include <sal/log.hxx>

using namespace cppu;
using namespace osl;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::ModelDependent;

#include <initguid.h>

// GUID used since 5.2 ( src569 m)
// {82154420-0FBF-11d4-8313-005004526AB4}
DEFINE_GUID(OID_ServiceManager, 0x82154420, 0xfbf, 0x11d4, 0x83, 0x13, 0x0, 0x50, 0x4, 0x52, 0x6a, 0xb4);

// FIXME: This GUID is just the above OID_ServiceManager with the
// initial part bumped by one. Is that good enough?
// {82154421-0FBF-11d4-8313-005004526AB4}
DEFINE_GUID(OID_LibreOfficeWriterApplication, 0x82154421, 0xfbf, 0x11d4, 0x83, 0x13, 0x0, 0x50, 0x4, 0x52, 0x6a, 0xb4);

// For Calc
// {82154425-0FBF-11d4-8313-005004526AB4}
DEFINE_GUID(OID_LibreOfficeCalcApplication, 0x82154425, 0xfbf, 0x11d4, 0x83, 0x13, 0x0, 0x50, 0x4, 0x52, 0x6a, 0xb4);

OneInstanceOleWrapper::OneInstanceOleWrapper(  const Reference<XMultiServiceFactory>& smgr,
                                               std::function<const Reference<XInterface>()> xInstFunction )
    : m_refCount(0)
    , m_xInstFunction(xInstFunction)
    , m_factoryHandle(0)
    , m_smgr(smgr)
{
    Reference<XInterface> xInt = m_smgr->createInstance("com.sun.star.bridge.oleautomation.BridgeSupplier");

    if (xInt.is())
    {
        Any a= xInt->queryInterface( cppu::UnoType<XBridgeSupplier2>::get() );
        a >>= m_bridgeSupplier;
    }
}

OneInstanceOleWrapper::~OneInstanceOleWrapper()
{
}

bool OneInstanceOleWrapper::registerClass(GUID const * pGuid)
{
    HRESULT hresult;

    o2u_attachCurrentThread();

    hresult = CoRegisterClassObject(
            *pGuid,
            this,
            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
            REGCLS_MULTIPLEUSE,
            &m_factoryHandle);

    SAL_INFO("extensions.olebridge", "CoRegisterClassObject(" << *pGuid << "): " << WindowsErrorStringFromHRESULT(hresult));

    return (hresult == NOERROR);
}

bool OneInstanceOleWrapper::deregisterClass()
{
    return CoRevokeClassObject(m_factoryHandle) == NOERROR;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP OneInstanceOleWrapper::QueryInterface(REFIID riid, void ** ppv)
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

COM_DECLSPEC_NOTHROW STDMETHODIMP_(ULONG) OneInstanceOleWrapper::AddRef()
{
    return osl_atomic_increment( &m_refCount);
}

COM_DECLSPEC_NOTHROW STDMETHODIMP_(ULONG) OneInstanceOleWrapper::Release()
{
    MutexGuard oGuard( Mutex::getGlobalMutex());
    ULONG refCount = --m_refCount;
    if ( m_refCount == 0)
    {
        delete this;
    }

    return refCount;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP OneInstanceOleWrapper::CreateInstance(IUnknown*,
                                                                        REFIID riid, void** ppv)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    SAL_INFO("extensions.olebridge", "OneInstanceOleWrapper::CreateInstance(" << riid << ")");

    if (officecfg::Office::Common::Security::Scripting::DisableOLEAutomation::get())
    {
        return ResultFromScode(E_NOINTERFACE);
    }

    HRESULT ret = ResultFromScode(E_UNEXPECTED);
    const Reference<XInterface>& xInst = m_xInstFunction();
    if (xInst.is())
    {
        Any usrAny(&xInst, cppu::UnoType<decltype(xInst)>::get());
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
                SAL_INFO("extensions.olebridge", "OneInstanceOleWrapper::Createbridge: punkVal=" << pVariant->punkVal);
                ret = pVariant->punkVal->QueryInterface(riid, ppv);
            }

            VariantClear(pVariant);
            CoTaskMemFree(pVariant);
        }
    }

    return ret;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP OneInstanceOleWrapper::LockServer(BOOL /*fLock*/)
{
    return NOERROR;
}

OleConverter::OleConverter( const Reference<XMultiServiceFactory> &smgr):
    UnoConversionUtilities<OleConverter>( smgr)

{
}

// The XMultiServiceFactory is later set by XInitialization
OleConverter::OleConverter( const  Reference<XMultiServiceFactory>& smgr, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass ):
    UnoConversionUtilities<OleConverter>( smgr, unoWrapperClass, comWrapperClass  )

{
}

OleConverter::~OleConverter()
{
}

// XBridgeSupplier --------------------------------------------------------------
Any SAL_CALL OleConverter::createBridge(const Any& modelDepObject,
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

OUString OleConverter::getImplementationName()
{
    return m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL
        ? OUString("com.sun.star.comp.ole.OleConverter2")
        : OUString("com.sun.star.comp.ole.OleConverterVar1");
}

sal_Bool OleConverter::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> OleConverter::getSupportedServiceNames()
{
    if (m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL)
    {
        return css::uno::Sequence<OUString>{
            "com.sun.star.bridge.OleBridgeSupplier2",
            "com.sun.star.bridge.oleautomation.BridgeSupplier"};
    }
    return {"com.sun.star.bridge.OleBridgeSupplierVar1"};
}

// XInitialize ------------------------------------------------------------------------------
// the first argument is an XMultiServiceFactory if at all
void SAL_CALL OleConverter::initialize( const Sequence< Any >& aArguments )
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
Reference< XInterface > OleConverter::createUnoWrapperInstance()
{
    if( m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new InterfaceOleWrapper(
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

Reference< XInterface > OleConverter::createComWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new IUnknownWrapper(
                            m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}

OleClient::OleClient( const Reference<XMultiServiceFactory>& smgr):
    UnoConversionUtilities<OleClient>( smgr)
{
    Reference<XInterface> xInt;// = m_smgr->createInstance(L"com.sun.star.bridge.OleBridgeSupplier2");

    if (xInt.is())
    {
        Any a= xInt->queryInterface(cppu::UnoType<XBridgeSupplier2>::get() );
        a >>= m_bridgeSupplier;
    }
}

OleClient::~OleClient()
{
}

Sequence< OUString >    SAL_CALL OleClient::getAvailableServiceNames()
{
    Sequence< OUString > ret;

    return ret;
}

OUString OleClient::getImplementationName()
{
    return "com.sun.star.comp.ole.OleClient";
}

sal_Bool OleClient::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> OleClient::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{
        "com.sun.star.bridge.OleObjectFactory",
        "com.sun.star.bridge.oleautomation.Factory"};
}

Reference<XInterface> SAL_CALL OleClient::createInstance(const OUString& ServiceSpecifier)
{
    Reference<XInterface>   ret;
    HRESULT         result;
    IUnknown*       pUnknown = nullptr;
    CLSID           classId;

    o2u_attachCurrentThread();

    result = CLSIDFromProgID(
                  o3tl::toW(ServiceSpecifier.getStr()), //Pointer to the ProgID
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

Reference<XInterface> SAL_CALL OleClient::createInstanceWithArguments(const OUString& ServiceSpecifier, const Sequence< Any >& /*Arguments*/)
{
    return createInstance( ServiceSpecifier);
}

// UnoConversionUtilities -----------------------------------------------------------------------------
Reference< XInterface > OleClient::createUnoWrapperInstance()
{
    if( m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new InterfaceOleWrapper(
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
Reference< XInterface > OleClient::createComWrapperInstance( )
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new IUnknownWrapper(
                            m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}

OleServer::OleServer( const Reference<XMultiServiceFactory>& smgr):
    m_smgr( smgr)
{
    Reference<XInterface> xInt = m_smgr->createInstance("com.sun.star.bridge.oleautomation.BridgeSupplier");

    if (xInt.is())
    {
        Any a= xInt->queryInterface( cppu::UnoType<XBridgeSupplier2>::get() );
        a >>= m_bridgeSupplier;
    }

    (void) provideInstance( [&]
                            {
                                return m_smgr;
                            },
                            &OID_ServiceManager );

    (void) provideInstance( [&]
                            {
                                // We want just one SwVbaGlobals for all Automation clients
                                static const Reference<XInterface> xWordGlobals = m_smgr->createInstance("ooo.vba.word.Globals");
                                const Reference<ooo::vba::XHelperInterface> xHelperInterface(xWordGlobals, UNO_QUERY);
                                Any aApplication = xHelperInterface->Application();
                                Reference<XInterface> xApplication;
                                aApplication >>= xApplication;
                                return xApplication;
                            },
                            &OID_LibreOfficeWriterApplication );

    (void) provideInstance( [&]
                            {
                                // Ditto for sc
                                static const Reference<XInterface> xCalcGlobals = m_smgr->createInstance("ooo.vba.excel.Globals");
                                const Reference<ooo::vba::XHelperInterface> xHelperInterface(xCalcGlobals, UNO_QUERY);
                                Any aApplication = xHelperInterface->Application();
                                Reference<XInterface> xApplication;
                                aApplication >>= xApplication;
                                return xApplication;
                            },
                            &OID_LibreOfficeCalcApplication );
}

OleServer::~OleServer()
{
    for (auto const& elem : m_wrapperList)
    {
        elem->deregisterClass();
        elem->Release();
    }
    m_wrapperList.clear();
}

OUString OleServer::getImplementationName()
{
    return "com.sun.star.comp.ole.OleServer";
}

sal_Bool OleServer::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> OleServer::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{
        "com.sun.star.bridge.OleApplicationRegistration",
        "com.sun.star.bridge.oleautomation.ApplicationRegistration"};
}

bool OleServer::provideInstance(std::function<const Reference<XInterface>()> xInstFunction, GUID const * guid)
{
    OneInstanceOleWrapper* pWrapper = new OneInstanceOleWrapper( m_smgr, xInstFunction );

    pWrapper->AddRef();
    m_wrapperList.push_back(pWrapper);

    return pWrapper->registerClass(guid);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
