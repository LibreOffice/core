/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: servprov.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _SERVPROV_HXX
#define _SERVPROV_HXX

#include <com/sun/star/lang/XInitialization.hpp>
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#include "ole2uno.hxx"
#include "unoconversionutilities.hxx"


using namespace com::sun::star::bridge;
using namespace cppu;
using namespace std;

namespace ole_adapter
{
Reference< XInterface> SAL_CALL ConverterProvider_CreateInstance(   const Reference<XMultiServiceFactory> & xSMgr)
                            throw( Exception);
Reference< XInterface> SAL_CALL ConverterProvider_CreateInstance2(  const Reference<XMultiServiceFactory> & xSMgr)
                            throw( Exception);
Reference< XInterface> SAL_CALL ConverterProvider_CreateInstanceVar1(   const Reference<XMultiServiceFactory> & xSMgr)
                            throw( Exception);
Reference<XInterface> SAL_CALL OleClient_CreateInstance( const Reference<XMultiServiceFactory> & xSMgr)
                            throw( Exception);
Reference<XInterface> SAL_CALL OleServer_CreateInstance( const Reference<XMultiServiceFactory> & xSMgr)
                            throw( Exception);
/*****************************************************************************

    class declaration IClassFactoryWrapper

    Specify abstract helper methods on class factories, which provide
    UNO objects. These methods are used by objects of class OleServer_Impl,
    to handle the OLE registration of different class factories.

*****************************************************************************/

class IClassFactoryWrapper : public IClassFactory
{
public:

    virtual sal_Bool registerClass() = 0;
    virtual sal_Bool deregisterClass() = 0;
};

/*****************************************************************************

    class declaration ProviderOleWrapper_Impl

    Provides an UNO service provider as OLE class factory. Handle the
    OLE registration by overloading the abstract methods from
    IClassFactoryWrapper.

    Acts as a COM class factory. When IClassFactory::CreateInstance is being called
    then it creates an service by help of the XSingleServiceFactory member and maps
    maps it to a COM object.

*****************************************************************************/

class ProviderOleWrapper_Impl : public IClassFactoryWrapper
{
public:

    ProviderOleWrapper_Impl( const Reference<XMultiServiceFactory>& smgr,
                             const Reference<XSingleServiceFactory>& xSFactory, GUID* pGuid);
    virtual ~ProviderOleWrapper_Impl();

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
    Reference<XSingleServiceFactory> m_xSingleServiceFactory;
    GUID                m_guid;
    DWORD               m_factoryHandle;
    Reference<XBridgeSupplier2> m_bridgeSupplier;
    Reference<XMultiServiceFactory> m_smgr;
};

/*****************************************************************************

    class declaration OneInstanceOleWrapper_Impl

    Provides an single UNO object as OLE object. Handle the
    OLE registration by overloading the abstract methods from
    IClassFactoryWrapper.

      Acts as a COM class factory. When IClassFactory::CreateInstance is being called
    then it maps the XInstance member it to a COM object.

*****************************************************************************/

class OneInstanceOleWrapper_Impl : public IClassFactoryWrapper
{
public:

    OneInstanceOleWrapper_Impl( const Reference<XMultiServiceFactory>& smgr, const Reference<XInterface>& xInst, GUID* pGuid, sal_Bool bAsApplication );
    virtual ~OneInstanceOleWrapper_Impl();

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

    //ORefCount         m_refCount;
    oslInterlockedCount m_refCount;
    Reference<XInterface>       m_xInst;
    GUID                m_guid;
    DWORD               m_factoryHandle;
    Reference<XBridgeSupplier2> m_bridgeSupplier;
    Reference<XMultiServiceFactory> m_smgr;
    unsigned long       m_nApplRegHandle;
    sal_Bool            m_bAsApplication;
};

/*****************************************************************************

    class declaration OleConverter_Impl2

    Implementation of the UNO service com.sun.star.bridge.OleBridgeSupplier2.

*****************************************************************************/

// This class realizes the service com.sun.star.bridge.OleBridgeSupplier2 and
// com.sun.star.bridge.OleBridgeSupplierVar1. The class implements XBridgeSupplier2
// instead of XBridgeSuppplier as done by class OleConverter_Impl. The XBridgeSupplier2
// interface does not need a Maschine Id in its createBridge function anymore,
// If an UNO interface is to be converted then the member m_nUnoWrapperClass determines
// what wrapper class is to be used. There are currently InterfaceOleWrapper_Impl and
// UnoObjectWrapperRemoteOpt. The first is used for the OleBridgeSupplier2 and the
// latter for OleBridgeSupplierVar1.
// The m_nComWrapperClass specifies the class which is used as wrapper for COM interfaces.
// Currently there is only one class available ( IUnknownWrapper_Impl).
class OleConverter_Impl2 : public WeakImplHelper2<XBridgeSupplier2, XInitialization>,
                            public UnoConversionUtilities<OleConverter_Impl2>
{
public:
    OleConverter_Impl2( const Reference<XMultiServiceFactory>& smgr);
    OleConverter_Impl2( const  Reference<XMultiServiceFactory>& smgr, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass );
    virtual ~OleConverter_Impl2();

    // XBridgeSupplier2 ---------------------------------------------------

    virtual Any SAL_CALL createBridge(const Any& modelDepObject,
                                const Sequence<sal_Int8>& ProcessId,
                                sal_Int16 sourceModelType,
                                sal_Int16 destModelType)
            throw (IllegalArgumentException, RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException);

    // Abstract struct UnoConversionUtilities
    virtual Reference< XInterface > createUnoWrapperInstance();
    virtual Reference< XInterface > createComWrapperInstance();
protected:

};


/*****************************************************************************

    class declaration OleClient_Impl

    Implementation of the UNO service com.sun.star.bridge.OleObjectFactory.

*****************************************************************************/


class OleClient_Impl : public WeakImplHelper1<XMultiServiceFactory>,
                       public UnoConversionUtilities<OleClient_Impl>
{
public:
    OleClient_Impl( const Reference<XMultiServiceFactory>& smgr);
    ~OleClient_Impl();

    // XMultiServiceFactory
    virtual Reference<XInterface> SAL_CALL createInstance(const OUString& ServiceSpecifier) throw( Exception, RuntimeException);
    virtual Reference<XInterface> SAL_CALL createInstanceWithArguments(const OUString& ServiceSpecifier, const Sequence< Any >& Arguments) throw (Exception, RuntimeException);
    Sequence< OUString >    SAL_CALL getAvailableServiceNames() throw (RuntimeException);

    // Abstract struct UnoConversionUtilities
    virtual Reference< XInterface > createUnoWrapperInstance();
    virtual Reference< XInterface > createComWrapperInstance();


    OUString getImplementationName();
protected:
    Reference<XBridgeSupplier2> m_bridgeSupplier;
};

/*****************************************************************************

    class declaration OleServer_Impl

    Implementation of the UNO service com.sun.star.bridge.OleApplicationRegistration.
    Register the calling application as OLE automation server for
    standard OLE object. The objects will be registered while instanciating
    this implementation and deregistrated, if this implementation is destroyed.

*****************************************************************************/

class OleServer_Impl : public OWeakObject, XTypeProvider
{
public:
    OleServer_Impl( const Reference<XMultiServiceFactory> &smgr);
    ~OleServer_Impl();

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& aType ) throw(RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes( ) throw(RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(RuntimeException);

protected:

    sal_Bool provideService(const Reference<XSingleServiceFactory>& xMulFact, GUID* guid);
    sal_Bool provideInstance(const Reference<XInterface>& xInst, GUID* guid, sal_Bool bAsApplication );

    list< IClassFactoryWrapper* > m_wrapperList;
    Reference< XBridgeSupplier2 >   m_bridgeSupplier;

    Reference<XMultiServiceFactory> m_smgr;
};

} // end namespace
#endif



