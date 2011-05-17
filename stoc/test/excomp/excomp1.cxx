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
#include <cppuhelper/implbase2.hxx>

#include <example/XTest.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace example;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace osl;

using ::rtl::OUString;

#define SERVICENAME1 "example.ExampleComponent1"
#define IMPLNAME1   "example.ExampleComponent1.Impl"

namespace excomp_impl {

//*************************************************************************
// ExampleComponent1Impl
//*************************************************************************
class ExampleComponent1Impl : public WeakImplHelper2< XTest, XServiceInfo >
{
public:
    ExampleComponent1Impl( const Reference<XMultiServiceFactory> & rXSMgr );

    ~ExampleComponent1Impl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XSimpleRegistry
    virtual OUString SAL_CALL getMessage() throw(RuntimeException);

protected:
    Mutex       m_mutex;

    Reference<XMultiServiceFactory> m_xSMgr;
};

//*************************************************************************
ExampleComponent1Impl::ExampleComponent1Impl( const Reference<XMultiServiceFactory> & rXSMgr )
    : m_xSMgr(rXSMgr)
{
}

//*************************************************************************
ExampleComponent1Impl::~ExampleComponent1Impl()
{
}

//*************************************************************************
OUString SAL_CALL ExampleComponent1Impl::getImplementationName(  )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME1) );
}

//*************************************************************************
sal_Bool SAL_CALL ExampleComponent1Impl::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL ExampleComponent1Impl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL ExampleComponent1Impl::getSupportedServiceNames_Static(  )
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME1) );
    return Sequence< OUString >( &aName, 1 );
}

//*************************************************************************
OUString SAL_CALL ExampleComponent1Impl::getMessage() throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return OUString(RTL_CONSTASCII_USTRINGPARAM("Lalelu nur der Mann im Mond schaut zu ..."));
}


//*************************************************************************
Reference<XInterface> SAL_CALL ExampleComponent1_CreateInstance( const Reference<XMultiServiceFactory>& rSMgr )
{
    Reference<XInterface> xRet;

    XTest *pXTest = (XTest*) new ExampleComponent1Impl(rSMgr);

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
    void * /* pServiceManager */ , void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            // ExampleComponent1
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME1 "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL =
                ::excomp_impl::ExampleComponent1Impl::getSupportedServiceNames_Static();
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

    if (rtl_str_compare( pImplName, IMPLNAME1 ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME1) ),
            ::excomp_impl::ExampleComponent1_CreateInstance,
            ::excomp_impl::ExampleComponent1Impl::getSupportedServiceNames_Static() ) );

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
