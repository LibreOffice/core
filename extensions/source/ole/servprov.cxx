/*************************************************************************
 *
 *  $RCSfile: servprov.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2000-10-12 13:04:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <vos/mutex.hxx>

#include "ole2uno.hxx"
#include "unoconversionutilities.hxx"
#include "servprov.hxx"
#include "unoobjw.hxx"
#include "oleobjw.hxx"


using namespace std;
using namespace cppu;
using namespace rtl;
using namespace osl;
using namespace vos;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::ModelDependent;



namespace ole_adapter
{

#include <initguid.h>
// prior 5.2 ( src569 ver m)
// {3ECF78F0-B149-11D2-8EBE-00105AD848AF}
//DEFINE_GUID(OID_ServiceManager, 0x3ECF78F0, 0xB149, 0x11d2, 0x8E, 0xBE, 0x00, 0x10, 0x5A, 0xD8, 0x48, 0xAF);

// GUID used since 5.2 ( src569 m)
// {82154420-0FBF-11d4-8313-005004526AB4}
DEFINE_GUID(OID_ServiceManager, 0x82154420, 0xfbf, 0x11d4, 0x83, 0x13, 0x0, 0x50, 0x4, 0x52, 0x6a, 0xb4);



static  Reference<XMultiServiceFactory> xMultiServiceFactory;
static Reference<XRegistryKey>      xRegistryKey;

Reference<XMultiServiceFactory> o2u_getMultiServiceFactory()
{
    return xMultiServiceFactory;
}

Reference<XRegistryKey> o2u_getRegistryKey()
{
    return xRegistryKey;
}

extern OMutex globalWrapperMutex;
Reference<XSingleServiceFactory> getInvocationFactory()
{
    OGuard aGuard(globalWrapperMutex);
    static Reference<XSingleServiceFactory> factory= Reference<XSingleServiceFactory>(o2u_getMultiServiceFactory()->createInstance( INVOCATION_SERVICE), UNO_QUERY);
    return factory;
}

/*****************************************************************************

    class implementation ProviderOleWrapper_Impl

*****************************************************************************/

ProviderOleWrapper_Impl::ProviderOleWrapper_Impl(const Reference<XSingleServiceFactory>& xSFact, GUID* pGuid)
    : m_xSingleServiceFactory(xSFact)
{
    m_guid = *pGuid;

    Reference<XInterface> xInt = o2u_getMultiServiceFactory()->createInstance(L"com.sun.star.bridge.OleBridgeSupplier2");

    if (xInt.is())
    {
        Any a= xInt->queryInterface( ::getCppuType( reinterpret_cast<
                                                  Reference< XBridgeSupplier2>* >(0)));
        a >>= m_bridgeSupplier;

    }
}

ProviderOleWrapper_Impl::~ProviderOleWrapper_Impl()
{
}

sal_Bool ProviderOleWrapper_Impl::registerClass()
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

sal_Bool ProviderOleWrapper_Impl::deregisterClass()
{
    HRESULT hresult = CoRevokeClassObject(m_factoryHandle);

    return (hresult == NOERROR);
}

STDMETHODIMP ProviderOleWrapper_Impl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
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

STDMETHODIMP_(ULONG) ProviderOleWrapper_Impl::AddRef()
{
    return osl_incrementInterlockedCount( &m_refCount);
}

STDMETHODIMP_(ULONG) ProviderOleWrapper_Impl::Release()
{
    MutexGuard aGuard( Mutex::getGlobalMutex());
    m_refCount--;
    if (m_refCount == 0)
    {
        delete this;
    }

    return m_refCount;
}

STDMETHODIMP ProviderOleWrapper_Impl::CreateInstance(IUnknown FAR* punkOuter,
                                                     REFIID riid,
                                                     void FAR* FAR* ppv)
{
    HRESULT ret = ResultFromScode(E_UNEXPECTED);
    punkOuter = NULL;

    Reference<XInterface> xInstance;

    if (m_xSingleServiceFactory.is())
    {
        xInstance = m_xSingleServiceFactory->createInstance();

        if (xInstance.is())
        {
            Any usrAny(&xInstance, getCppuType( & xInstance));

            sal_uInt8 arId[16];
            rtl_getGlobalProcessId( arId );
            Any oleAny = m_bridgeSupplier->createBridge(usrAny,
                                        Sequence<sal_Int8>((sal_Int8*)arId, 16),
                                        UNO,
                                        OLE);


            if (oleAny.getValueTypeClass() == getCppuType( (sal_uInt32 *)0).getTypeClass())
            {
                VARIANT* pVariant = (VARIANT*)oleAny.getValue();

                if ((pVariant->vt == VT_UNKNOWN) || (pVariant->vt == VT_DISPATCH))
                {
                    ret = pVariant->punkVal->QueryInterface(riid, ppv);
                }

                VariantClear(pVariant);
                CoTaskMemFree(pVariant);
            }
        }
    }

    return ret;
}

STDMETHODIMP ProviderOleWrapper_Impl::LockServer(int fLock)
{
    return NOERROR;
}

/*****************************************************************************

    class implementation OneInstanceOleWrapper_Impl

*****************************************************************************/

OneInstanceOleWrapper_Impl::OneInstanceOleWrapper_Impl(const Reference<XInterface>& xInst, GUID* pGuid)
    : m_xInst(xInst)
{
    m_guid = *pGuid;

    Reference<XInterface> xInt = o2u_getMultiServiceFactory()->createInstance(L"com.sun.star.bridge.OleBridgeSupplier2");

    if (xInt.is())
    {
        Any a= xInt->queryInterface( getCppuType(
            reinterpret_cast< Reference<XBridgeSupplier2>*>(0)));
        a >>= m_bridgeSupplier;
    }
}

OneInstanceOleWrapper_Impl::~OneInstanceOleWrapper_Impl()
{
}

sal_Bool OneInstanceOleWrapper_Impl::registerClass()
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

sal_Bool OneInstanceOleWrapper_Impl::deregisterClass()
{
    HRESULT hresult = CoRevokeClassObject(m_factoryHandle);

    return (hresult == NOERROR);
}

STDMETHODIMP OneInstanceOleWrapper_Impl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
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

STDMETHODIMP_(ULONG) OneInstanceOleWrapper_Impl::AddRef()
{
    return osl_incrementInterlockedCount( &m_refCount);
}

STDMETHODIMP_(ULONG) OneInstanceOleWrapper_Impl::Release()
{

    MutexGuard oGuard( Mutex::getGlobalMutex());
    m_refCount--;
    if ( m_refCount == 0)
    {
        delete this;
    }

    return m_refCount;
}

STDMETHODIMP OneInstanceOleWrapper_Impl::CreateInstance(IUnknown FAR* punkOuter,
                                                         REFIID riid,
                                                         void FAR* FAR* ppv)
{
    HRESULT ret = ResultFromScode(E_UNEXPECTED);
    punkOuter = NULL;

    if (m_xInst.is())
    {
        Any usrAny(&m_xInst, getCppuType( &m_xInst));
        sal_uInt8 arId[16];
        rtl_getGlobalProcessId( arId);
        Any oleAny = m_bridgeSupplier->createBridge(usrAny,
                                        Sequence<sal_Int8>( (sal_Int8*)arId, 16),
                                        UNO,
                                        OLE);


        if (oleAny.getValueTypeClass() == TypeClass_UNSIGNED_LONG)
        {
            VARIANT* pVariant = *(VARIANT**)oleAny.getValue();

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

STDMETHODIMP OneInstanceOleWrapper_Impl::LockServer(int fLock)
{
    return NOERROR;
}


/*****************************************************************************

    class implementation OleConverter_Impl2

*****************************************************************************/

OleConverter_Impl2::OleConverter_Impl2()

{
}

// The XMultiServiceFactory is later set by XInitialization
OleConverter_Impl2::OleConverter_Impl2( sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass ):
                        UnoConversionUtilities<OleConverter_Impl2>( unoWrapperClass, comWrapperClass  )

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
                                       throw (IllegalArgumentException,
                                                   RuntimeException )
{
    Any ret;
    sal_uInt8 arId[16];
    rtl_getGlobalProcessId( arId );

    Sequence< sal_Int8 > seqProcessId( (sal_Int8*)arId, 16);

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
                VARIANT* pVariant = (VARIANT*) CoTaskMemAlloc(sizeof(VARIANT));
                VariantInit( pVariant);
                if (! anyToVariant( pVariant, modelDepObject))
                {
                    CoTaskMemFree(pVariant);
                    throw IllegalArgumentException();
                }
                else
                {
                    ret.setValue((void*) &pVariant, getCppuType((sal_uInt32*)0));
                }
            }
            else
                throw IllegalArgumentException();
        }
        else if (sourceModelType == OLE)
        {
            if (modelDepObject.getValueType() != getCppuType((sal_uInt32*)0))
            {
                throw IllegalArgumentException();
            }
            else if (destModelType == OLE)
            {
                // same model -> copy value only
                VARIANT* pVariant = (VARIANT*) CoTaskMemAlloc(sizeof(VARIANT));

                if (NOERROR != VariantCopy(pVariant, (VARIANT*)modelDepObject.getValue()))
                {
                    CoTaskMemFree(pVariant);
                    throw(IllegalArgumentException());
                }
                else
                {
                    ret.setValue((void*) &pVariant, getCppuType((sal_uInt32*)0));
                }
            }
            else if (destModelType == UNO)
            {
                // convert variant into UNO any
                VARIANT* pVariant = (VARIANT*)modelDepObject.getValue();

                if (!variantToAny(pVariant, ret))
                {
                    throw IllegalArgumentException();
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


// XInitialize ------------------------------------------------------------------------------
// the first argument is an XMultiServiceFactory if at all
void SAL_CALL OleConverter_Impl2::initialize( const Sequence< Any >& aArguments )
                throw(Exception, RuntimeException)
{
    if( aArguments.getLength() == 1 && aArguments[0].getValueTypeClass() == TypeClass_INTERFACE)
    {
        Reference < XInterface > xInt;
        aArguments[0] >>= xInt;
        Reference <XMultiServiceFactory> xMulti( xInt, UNO_QUERY);
        m_xMultiServiceFactory= xMulti;
    }
}

// UnoConversionUtilities -------------------------------------------------------------------
Reference< XInterface > OleConverter_Impl2::createUnoWrapperInstance()
{
    if( m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new InterfaceOleWrapper_Impl(
                                m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else if( m_nUnoWrapperClass == UNO_OBJECT_WRAPPER_REMOTE_OPT)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new UnoObjectWrapperRemoteOpt(
                                m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else
        return Reference<XInterface>();
}

Reference< XInterface > OleConverter_Impl2::createComWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new IUnknownWrapper_Impl(
                            m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}



/*****************************************************************************

    class implementation OleClient_Impl

*****************************************************************************/

OleClient_Impl::OleClient_Impl()
{
    Reference<XInterface> xInt = o2u_getMultiServiceFactory()->createInstance(L"com.sun.star.bridge.OleBridgeSupplier2");

    if (xInt.is())
    {
        Any a= xInt->queryInterface(getCppuType(
            reinterpret_cast<Reference<XBridgeSupplier2>*>(0)));
        a >>= m_bridgeSupplier;
    }
}

OleClient_Impl::~OleClient_Impl()
{
}

Sequence< OUString >    SAL_CALL OleClient_Impl::getAvailableServiceNames() throw( RuntimeException )
{
    Sequence< OUString > ret;

    return ret;
}


OUString OleClient_Impl::getImplementationName()
{
    return OUString(L"com.sun.star.comp.ole.OleClient");
}

Reference<XInterface> SAL_CALL OleClient_Impl::createInstance(const OUString& ServiceSpecifier) throw (Exception, RuntimeException )
{
    Reference<XInterface>   ret;
    HRESULT         result;
    IUnknown*       pUnknown = NULL;
    CLSID           classId;

    o2u_attachCurrentThread();

    result = CLSIDFromProgID(
                  ServiceSpecifier.getStr(),    //Pointer to the ProgID
                  &classId);                        //Pointer to the CLSID


    if (result == NOERROR)
    {
        result = CoCreateInstance(
                      classId,              //Class identifier (CLSID) of the object
                      NULL,                 //Pointer to whether object is or isn't part of an aggregate
                      CLSCTX_ALL,  //Context for running executable code
                      IID_IUnknown,         //Reference to the identifier of the interface
                      (void**)&pUnknown);   //Address of output variable that receives
                                                  // the interface pointer requested in riid
    }

    if (pUnknown != NULL)
    {
        Any any;
        VARIANT variant;

        VariantInit(&variant);

        V_VT(&variant) = VT_UNKNOWN;
        V_UNKNOWN(&variant) = pUnknown;

        pUnknown->AddRef();

        if (variantToAny(&variant, any))
        {
            if (any.getValueTypeClass() == TypeClass_INTERFACE)
            {
                ret = *(Reference<XInterface>*)any.getValue();
            }
        }

        VariantClear(&variant);
    }

    return ret;
}

Reference<XInterface> SAL_CALL OleClient_Impl::createInstanceWithArguments(const OUString& ServiceSpecifier, const Sequence< Any >& Arguments) throw (Exception, RuntimeException)
{
    Reference<XInterface>   ret;
    HRESULT         result;
    IUnknown*       pUnknown = NULL;
    CLSID           classId;

    o2u_attachCurrentThread();

    result = CLSIDFromProgID(
                      ServiceSpecifier.getStr(),    //Pointer to the ProgID
                      &classId);                        //Pointer to the CLSID


    if (result == NOERROR)
    {
        result = CoCreateInstance(
                      classId,              //Class identifier (CLSID) of the object
                      NULL,                 //Pointer to whether object is or isn't part of an aggregate
                      CLSCTX_LOCAL_SERVER,  //Context for running executable code
                      IID_IUnknown,         //Reference to the identifier of the interface
                      (void**)&pUnknown);   //Address of output variable that receives
                                                  // the interface pointer requested in riid
    }

    if (pUnknown != NULL)
    {
        Any any;
        VARIANT variant;

        VariantInit(&variant);

        V_VT(&variant) = VT_UNKNOWN;
        V_UNKNOWN(&variant) = pUnknown;

        pUnknown->AddRef();

        if (variantToAny(&variant, any))
        {
            if (any.getValueTypeClass() == TypeClass_INTERFACE)
            {
                ret = *(Reference<XInterface>*)any.getValue();
            }
        }

        VariantClear(&variant);
    }

    return ret;
}

// UnoConversionUtilities -----------------------------------------------------------------------------
Reference< XInterface > OleClient_Impl::createUnoWrapperInstance()
{
    if( m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new InterfaceOleWrapper_Impl(
                                m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else if( m_nUnoWrapperClass == UNO_OBJECT_WRAPPER_REMOTE_OPT)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new UnoObjectWrapperRemoteOpt(
                                m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else
        return Reference< XInterface>();
}
// UnoConversionUtilities -----------------------------------------------------------------------------
Reference< XInterface > OleClient_Impl::createComWrapperInstance( )
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new IUnknownWrapper_Impl(
                            m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}



/*****************************************************************************

    class implementation OleServer_Impl

*****************************************************************************/

OleServer_Impl::OleServer_Impl()
{
    Reference<XInterface> xInt = o2u_getMultiServiceFactory()->createInstance(L"com.sun.star.bridge.OleBridgeSupplier2");

    if (xInt.is())
    {
        Any a= xInt->queryInterface( getCppuType(
            reinterpret_cast< Reference<XBridgeSupplier2>*>(0)));
        a >>= m_bridgeSupplier;
    }

    sal_Bool ret = provideInstance(o2u_getMultiServiceFactory(), (GUID*)&OID_ServiceManager);
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
// XInterface --------------------------------------------------
Any SAL_CALL OleServer_Impl::queryInterface( const Type& aType ) throw(RuntimeException)
{
    Any a= ::cppu::queryInterface( aType, static_cast<XTypeProvider*>(this));
    if( a == Any())
        return OWeakObject::queryInterface( aType);
    else
        return a;
}
void SAL_CALL OleServer_Impl::acquire(  ) throw(RuntimeException)
{
    OWeakObject::acquire();
}
void SAL_CALL OleServer_Impl::release(  ) throw (RuntimeException)
{
    OWeakObject::release();
}


// XTypeProvider --------------------------------------------------
Sequence< Type > SAL_CALL OleServer_Impl::getTypes( ) throw(RuntimeException)
{
    static OTypeCollection *pCollection = 0;
    if( ! pCollection )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pCollection )
        {
            static OTypeCollection collection(
                getCppuType(reinterpret_cast< Reference< XWeak>*>(0)),
                getCppuType(reinterpret_cast< Reference< XTypeProvider>*>(0)) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}
Sequence< sal_Int8 > SAL_CALL OleServer_Impl::getImplementationId() throw(RuntimeException)
{
    static OImplementationId *pId = 0;
    if( ! pId )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pId )
        {
            static OImplementationId id( sal_False );
            pId = &id;
        }
    }
    return (*pId).getImplementationId();
}


sal_Bool OleServer_Impl::provideService(const Reference<XSingleServiceFactory>& xSFact, GUID* guid)
{
    sal_Bool ret = FALSE;

    IClassFactoryWrapper* pFac = new ProviderOleWrapper_Impl(xSFact, guid);

    pFac->AddRef();

    m_wrapperList.push_back(pFac);

    return pFac->registerClass();
}

sal_Bool OleServer_Impl::provideInstance(const Reference<XInterface>& xInst, GUID* guid)
{
    sal_Bool    ret = FALSE;

    IClassFactoryWrapper* pFac = new OneInstanceOleWrapper_Impl(xInst, guid);

    pFac->AddRef();
    m_wrapperList.push_back(pFac);

    return pFac->registerClass();
}



/*****************************************************************************

    functions to create the service providers

*****************************************************************************/

Reference<XSingleServiceFactory>    o2u_getConverterProvider2(const Reference<XMultiServiceFactory>&    xMan,
                                                const Reference<XRegistryKey>&  xKey)
{
    static Reference<XSingleServiceFactory> ret = NULL;

    if (!ret.is())
    {
        if (!xMultiServiceFactory.is())     xMultiServiceFactory = xMan;

        Sequence<OUString> seqServiceNames;
        ret = createSingleFactory( xMan, L"com.sun.star.comp.ole.OleConverter2",
            ConverterProvider_CreateInstance2, seqServiceNames );
    }
    return ret;
}

Reference<XSingleServiceFactory>    o2u_getConverterProviderVar1(const Reference<XMultiServiceFactory>& xMan,
                                                const Reference<XRegistryKey>&  xKey)
{
    static Reference<XSingleServiceFactory> ret = NULL;

    if (!ret.is())
    {
        if (!xMultiServiceFactory.is())     xMultiServiceFactory = xMan;

        Sequence<OUString> seqServiceNames;
        ret = createSingleFactory( xMan, L"com.sun.star.comp.ole.OleConverterVar1",
            ConverterProvider_CreateInstanceVar1, seqServiceNames );
    }
    return ret;
}
Reference<XSingleServiceFactory>    o2u_getClientProvider(const Reference<XMultiServiceFactory>& xMan,
                                          const Reference<XRegistryKey>&    xKey)
{
    static Reference<XSingleServiceFactory> ret = NULL;

    if (!ret.is())
    {
        if (!xMultiServiceFactory.is())     xMultiServiceFactory = xMan;
        Sequence<OUString> seqServiceNames;
        ret = createSingleFactory( xMan, L"com.sun.star.comp.ole.OleClient",
            OleClient_CreateInstance, seqServiceNames);
    }

    return ret;
}

Reference<XSingleServiceFactory>    o2u_getServerProvider(const Reference<XMultiServiceFactory>& xMan,
                                          const Reference<XRegistryKey>&    xKey)
{
    static Reference<XSingleServiceFactory> ret = NULL;

    if (!ret.is())
    {
        if (!xMultiServiceFactory.is())     xMultiServiceFactory = xMan;
        Sequence<OUString> seqServiceNames;
        ret = createOneInstanceFactory( xMan, L"com.sun.star.comp.ole.OleServer",
            OleServer_CreateInstance, seqServiceNames);
    }

    return ret;
}
// Creator functions --------------------

Reference<XInterface> SAL_CALL ConverterProvider_CreateInstance2(   const Reference<XMultiServiceFactory> & xSMgr)
                            throw(Exception)
{
    Reference<XInterface> xService = *new OleConverter_Impl2();
    return xService;
}

Reference<XInterface> SAL_CALL ConverterProvider_CreateInstanceVar1(    const Reference<XMultiServiceFactory> & xSMgr)
                            throw(Exception)
{
    Reference<XInterface> xService = *new OleConverter_Impl2( UNO_OBJECT_WRAPPER_REMOTE_OPT, IUNKNOWN_WRAPPER_IMPL);
    return xService;
}

Reference<XInterface> SAL_CALL OleClient_CreateInstance( const Reference<XMultiServiceFactory> & xSMgr)
                            throw(Exception)
{
    Reference<XInterface> xService = *new OleClient_Impl();
    return xService;
}

Reference<XInterface> SAL_CALL OleServer_CreateInstance( const Reference<XMultiServiceFactory> & xSMgr)
                            throw (Exception)
{
    Reference<XInterface > xService = *new OleServer_Impl();
    return xService;
}


} // end namespace
