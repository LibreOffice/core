/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/alloc.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <example/XTest.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace example;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace osl;

using ::rtl::OUString;

#define SERVICENAME2 "example.ExampleComponent2"
#define IMPLNAME2   "example.ExampleComponent2.Impl"

namespace excomp2_impl {

//*************************************************************************
// ExampleComponent2Impl
//*************************************************************************
class ExampleComponent2Impl : public OWeakObject
                            , public XTypeProvider
                            , public XServiceInfo
                            , public XTest
{
public:
    ExampleComponent2Impl( const Reference<XMultiServiceFactory> & rXSMgr );

    ~ExampleComponent2Impl();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw()
        { OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw()
        { OWeakObject::release(); }

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XTest
    virtual OUString SAL_CALL getMessage() throw(RuntimeException);

protected:
    Mutex       m_mutex;

    Reference<XMultiServiceFactory> m_xSMgr;
};

//*************************************************************************
ExampleComponent2Impl::ExampleComponent2Impl( const Reference<XMultiServiceFactory> & rXSMgr )
    : m_xSMgr(rXSMgr)
{
}

//*************************************************************************
ExampleComponent2Impl::~ExampleComponent2Impl()
{
}

//*************************************************************************
Any SAL_CALL ExampleComponent2Impl::queryInterface( const ::com::sun::star::uno::Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType,
                                      static_cast< XTypeProvider * >( this ),
                                      static_cast< XServiceInfo * >( this ),
                                        static_cast< XTest * >( this ) );
    if ( aRet.hasValue() )
        return aRet;

    return OWeakObject::queryInterface( rType );
}

//*************************************************************************
Sequence< Type > SAL_CALL ExampleComponent2Impl::getTypes()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OTypeCollection * pTypes = 0;
    if (! pTypes)
    {
        MutexGuard aGuard( m_mutex );
        if (! pTypes)
        {
            static OTypeCollection aTypes(
                ::getCppuType( (const Reference< XInterface > *)0 ),
                ::getCppuType( (const Reference< XWeak > *)0 ),
                ::getCppuType( (const Reference< XTypeProvider > *)0 ),
                ::getCppuType( (const Reference< XServiceInfo > *)0 ),
                ::getCppuType( (const Reference< XTest > *)0 ) );
            pTypes = &aTypes;
        }
    }
    return pTypes->getTypes();
}

//*************************************************************************
Sequence< sal_Int8 > SAL_CALL ExampleComponent2Impl::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( m_mutex );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

//*************************************************************************
OUString SAL_CALL ExampleComponent2Impl::getImplementationName(  )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME2) );
}

//*************************************************************************
sal_Bool SAL_CALL ExampleComponent2Impl::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL ExampleComponent2Impl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL ExampleComponent2Impl::getSupportedServiceNames_Static(  )
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME2) );
    return Sequence< OUString >( &aName, 1 );
}

//*************************************************************************
OUString SAL_CALL ExampleComponent2Impl::getMessage() throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return OUString(RTL_CONSTASCII_USTRINGPARAM("Alle meine Entchen schwimmen auf dem See, schwimmen auf dem See ..."));
}


//*************************************************************************
Reference<XInterface> SAL_CALL ExampleComponent2_CreateInstance( const Reference<XMultiServiceFactory>& rSMgr )
{
    Reference<XInterface> xRet;

    XTest *pXTest = (XTest*) new ExampleComponent2Impl(rSMgr);

    if (pXTest)
    {
        xRet = Reference< XInterface >::query(pXTest);
    }

    return xRet;
}


} // excomp_impl


extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * /* pServiceManager */, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            // ExampleComponent2
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME2 "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL =
                ::excomp2_impl::ExampleComponent2Impl::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_FAIL( "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /* pRegistryKey */ )
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, IMPLNAME2 ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME2) ),
            ::excomp2_impl::ExampleComponent2_CreateInstance,
            ::excomp2_impl::ExampleComponent2Impl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
