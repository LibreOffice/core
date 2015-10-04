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


// starting the executable:
// -env:UNO_CFG_URL=local;<absolute_path>..\\..\\test\\cfg_data;<absolute_path>\\cfg_update
// -env:UNO_TYPES=cpputest.rdb

#include <sal/main.h>

#include <stdio.h>

#include <rtl/strbuf.hxx>

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/registry/XImplementationRegistration.hpp>


using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace cfg_test
{


static Sequence< OUString > impl0_getSupportedServiceNames()
{
    OUString str("com.sun.star.bootstrap.TestComponent0");
    return Sequence< OUString >( &str, 1 );
}

static OUString impl0_getImplementationName()
{
    return OUString("com.sun.star.comp.bootstrap.TestComponent0");
}

static Sequence< OUString > impl1_getSupportedServiceNames()
{
    OUString str("com.sun.star.bootstrap.TestComponent1");
    return Sequence< OUString >( &str, 1 );
}

static OUString impl1_getImplementationName()
{
    return OUString("com.sun.star.comp.bootstrap.TestComponent1");
}


class ServiceImpl0
    : public WeakImplHelper < lang::XServiceInfo, lang::XInitialization >
{
    Reference< XComponentContext > m_xContext;

public:
    explicit ServiceImpl0( Reference< XComponentContext > const & xContext );

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& rArgs ) throw (Exception, RuntimeException);

    // XServiceInfo
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw (RuntimeException);
};

ServiceImpl0::ServiceImpl0( Reference< XComponentContext > const & xContext )
    : m_xContext( xContext )
{
    sal_Int32 n;
    OUString val;

    // service properties
    OSL_VERIFY( m_xContext->getValueByName(
        "/services/com.sun.star.bootstrap.TestComponent0/context-properties/serviceprop0" ) >>= n );
    OSL_VERIFY( n == 13 );
    OSL_VERIFY( m_xContext->getValueByName(
        "/services/com.sun.star.bootstrap.TestComponent0/context-properties/serviceprop1" ) >>= val );
    OSL_VERIFY( val == "value of serviceprop1" );
    // impl properties
    OSL_VERIFY( m_xContext->getValueByName(
        "/implementations/com.sun.star.comp.bootstrap.TestComponent0/context-properties/implprop0" ) >>= n );
    OSL_VERIFY( n == 15 );
    OSL_VERIFY( m_xContext->getValueByName(
        "/implementations/com.sun.star.comp.bootstrap.TestComponent0/context-properties/implprop1" ) >>= val );
    OSL_VERIFY( val == "value of implprop1" );
}
// XInitialization

void ServiceImpl0::initialize( const Sequence< Any >& rArgs )
    throw (Exception, RuntimeException)
{
    // check args
    OUString val;
    OSL_VERIFY( rArgs.getLength() == 3 );
    OSL_VERIFY( rArgs[ 0 ] >>= val );
    OSL_VERIFY( val == "first argument" );
    OSL_VERIFY( rArgs[ 1 ] >>= val );
    OSL_VERIFY( val == "second argument" );
    OSL_VERIFY( rArgs[ 2 ] >>= val );
    OSL_VERIFY( val == "third argument" );
}
// XServiceInfo

OUString ServiceImpl0::getImplementationName()
    throw(css::uno::RuntimeException)
{
    return impl0_getImplementationName();
}

Sequence< OUString > ServiceImpl0::getSupportedServiceNames()
    throw(css::uno::RuntimeException)
{
    return impl0_getSupportedServiceNames();
}

sal_Bool ServiceImpl0::supportsService( const OUString & rServiceName )
    throw(css::uno::RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}


class ServiceImpl1 : public ServiceImpl0
{
public:
    explicit ServiceImpl1( Reference< XComponentContext > const & xContext )
        : ServiceImpl0( xContext )
    {
    }

    // XServiceInfo
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
};

OUString ServiceImpl1::getImplementationName()
    throw(css::uno::RuntimeException)
{
    return impl1_getImplementationName();
}

Sequence< OUString > ServiceImpl1::getSupportedServiceNames()
    throw(css::uno::RuntimeException)
{
    return impl1_getSupportedServiceNames();
}


static Reference< XInterface > SAL_CALL ServiceImpl0_create(
    Reference< XComponentContext > const & xContext )
{
    return (OWeakObject *)new ServiceImpl0( xContext );
}

static Reference< XInterface > SAL_CALL ServiceImpl1_create(
    Reference< XComponentContext > const & xContext )
{
    return (OWeakObject *)new ServiceImpl1( xContext );
}

} // namespace cfg_test

static const struct ImplementationEntry g_entries[] =
{
    {
        ::cfg_test::ServiceImpl0_create, ::cfg_test::impl0_getImplementationName,
        ::cfg_test::impl0_getSupportedServiceNames, createSingleComponentFactory,
        0, 0
    },
    {
        ::cfg_test::ServiceImpl1_create, ::cfg_test::impl1_getImplementationName,
        ::cfg_test::impl1_getSupportedServiceNames, createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

// component exports
extern "C"
{

sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return component_writeInfoHelper(
        pServiceManager, pRegistryKey, g_entries );
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey , g_entries );
}
}






SAL_IMPLEMENT_MAIN()
{
    try
    {
        Reference< XComponentContext > xContext( defaultBootstrap_InitialComponentContext() );
        Reference< lang::XMultiComponentFactory > xMgr( xContext->getServiceManager() );

        // show what is in context
        xContext->getValueByName( "dump_maps" );

        sal_Int32 n(0);
        OSL_VERIFY( xContext->getValueByName( "/global-context-properties/TestValue" ) >>= n );
        ::fprintf( stderr, "> n=%d\n", n );

        Reference< XInterface > x;
        OSL_VERIFY( !(xContext->getValueByName( "/singletons/my_converter" ) >>= x) );
        OSL_VERIFY( xContext->getValueByName( "/singletons/com.sun.star.script.theConverter" ) >>= x );
        OSL_VERIFY( xContext->getValueByName( "/singletons/com.sun.star.bootstrap.theTestComponent0" ) >>= x );

        ::fprintf( stderr, "> registering service...\n");
#if defined(SAL_W32)
        OUString libName( "cfg_test.dll" );
#elif defined(SAL_UNX)
        OUString libName( "libcfg_test.so" );
#endif
        Reference< registry::XImplementationRegistration > xImplReg( xMgr->createInstanceWithContext(
            "com.sun.star.registry.ImplementationRegistration", xContext ), UNO_QUERY );
        OSL_ENSURE( xImplReg.is(), "### no impl reg!" );
        xImplReg->registerImplementation(
            "com.sun.star.loader.SharedLibrary", libName,
            Reference< registry::XSimpleRegistry >() );

        OSL_VERIFY( (x = xMgr->createInstanceWithContext( "com.sun.star.bootstrap.TestComponent0", xContext )).is() );
        OSL_VERIFY( (x = xMgr->createInstanceWithContext( "com.sun.star.bootstrap.TestComponent1", xContext )).is() );

        Reference< lang::XComponent > xComp( xContext, UNO_QUERY );
        if (xComp.is())
        {
            xComp->dispose();
        }
        return 0;
    }
    catch (Exception & exc)
    {
        OString str( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        ::fprintf( stderr, "# caught exception: %s\n", str.getStr() );
        return 1;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
