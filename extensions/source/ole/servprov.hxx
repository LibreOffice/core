/*************************************************************************
 *
 *  $RCSfile: servprov.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:40 $
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

#ifndef _SERVPROV_HXX
#define _SERVPROV_HXX

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
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

    ProviderOleWrapper_Impl(const Reference<XSingleServiceFactory>& xSFactory, GUID* pGuid);
    ~ProviderOleWrapper_Impl();

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

    OneInstanceOleWrapper_Impl(const Reference<XInterface>& xInst, GUID* pGuid);
    ~OneInstanceOleWrapper_Impl();

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
    OleConverter_Impl2();
    OleConverter_Impl2( sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass );
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
};


/*****************************************************************************

    class declaration OleClient_Impl

    Implementation of the UNO service com.sun.star.bridge.OleObjectFactory.

*****************************************************************************/


class OleClient_Impl : public WeakImplHelper1<XMultiServiceFactory>,
                       public UnoConversionUtilities<OleClient_Impl>
{
public:
    OleClient_Impl();
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
    OleServer_Impl();
    ~OleServer_Impl();

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& aType ) throw(RuntimeException);
    virtual void SAL_CALL acquire(  ) throw (RuntimeException);
    virtual void SAL_CALL release(  ) throw (RuntimeException);

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes( ) throw(RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(RuntimeException);

protected:

    sal_Bool provideService(const Reference<XSingleServiceFactory>& xMulFact, GUID* guid);
    sal_Bool provideInstance(const Reference<XInterface>& xInst, GUID* guid);

    list< IClassFactoryWrapper* > m_wrapperList;
    Reference< XBridgeSupplier2 >   m_bridgeSupplier;
};

} // end namespace
#endif



