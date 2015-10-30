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


#include <sal/main.h>
#include <osl/diagnose.h>
#include <rtl/alloc.h>
#include <uno/environment.hxx>
#include <uno/lbnames.h>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>

#include <stdio.h>


using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace css::uno;


typedef WeakImplHelper< lang::XServiceInfo,
                         XCurrentContext,
                         reflection::XProxyFactory > t_impl;


class TargetObject : public t_impl
{
public:
    static int s_obj;

    virtual ~TargetObject() {
        --s_obj;
        OSL_TRACE( "~TargetObject()" );
    }
    TargetObject()
        { ++s_obj; }

    Any SAL_CALL queryInterface( Type const & type )
        throw (RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException)
        { return OUString("target"); }
    virtual sal_Bool SAL_CALL supportsService( const OUString & /*rServiceName*/ )
        throw (RuntimeException)
        { return sal_False; }
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException)
        { return Sequence< OUString >(); }
    // XProxyFactory
    virtual Reference< XAggregation > SAL_CALL createProxy(
        const Reference< XInterface > & xTarget ) throw (RuntimeException)
        { return Reference< XAggregation >( xTarget, UNO_QUERY ); }
    // XCurrentContext
    virtual Any SAL_CALL getValueByName( OUString const & name )
        throw (RuntimeException)
        { return makeAny( name ); }
};


Any TargetObject::queryInterface( Type const & type )
    throw (RuntimeException)
{
    Any ret( t_impl::queryInterface( type ) );
    if (ret.hasValue())
        return ret;
    throw lang::DisposedException(
        OUString( "my test exception" ),
        static_cast< OWeakObject * >(this) );
}

int TargetObject::s_obj = 0;



class TestMaster : public WeakImplHelper< lang::XServiceInfo >
{
    Reference< XAggregation > m_xProxyTarget;
    Reference<lang::XServiceInfo> m_xOtherProxyTargetBeforeSetDelegator;

    inline TestMaster() { ++s_obj; }
public:
    static int s_obj;
    static Reference< XInterface > create(
        Reference< reflection::XProxyFactory > const & xProxyFac );
    static Reference< XInterface > create(
        Reference< XInterface > const & xTarget,
        Reference< reflection::XProxyFactory > const & xProxyFac );

    virtual ~TestMaster() {
        --s_obj;
        OSL_TRACE( "~TestMaster()" );
    }

    virtual Any SAL_CALL queryInterface( const Type & rType )
        throw (RuntimeException)
    {
        Any aRet(
            WeakImplHelper< lang::XServiceInfo >::queryInterface( rType ) );
        if (aRet.hasValue())
            return aRet;
        return m_xProxyTarget->queryAggregation( rType );
    }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException)
        { return OUString("master"); }
    virtual sal_Bool SAL_CALL supportsService( const OUString & /*rServiceName*/ )
        throw (RuntimeException)
        { return sal_False; }
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException)
        { return Sequence< OUString >(); }
};

int TestMaster::s_obj = 0;


Reference< XInterface > TestMaster::create(
    Reference< XInterface > const & xTarget,
    Reference< reflection::XProxyFactory > const & xProxyFac )
{
    TestMaster * that = new TestMaster;
    Reference< XInterface > xRet( static_cast< OWeakObject * >( that ) );
    {
        Reference< XAggregation > xAgg( xProxyFac->createProxy( xTarget ) );
        // ownership take over
        that->m_xProxyTarget.set( xAgg, UNO_QUERY_THROW );
        that->m_xOtherProxyTargetBeforeSetDelegator.set(
            that->m_xProxyTarget, UNO_QUERY );
    }
    that->m_xProxyTarget->setDelegator( xRet );
    return xRet;
}

Reference< XInterface > TestMaster::create(
    Reference< reflection::XProxyFactory > const & xProxyFac )
{
    return create(
        static_cast< OWeakObject * >( new TargetObject ), xProxyFac );
}


static void test_proxyfac_(
    Reference< XInterface > const & xMaster, OUString const & test,
    Reference< reflection::XProxyFactory > const & /*xProxyFac*/ )
{
    (void)test;
    Reference< lang::XServiceInfo > xMaster_XServiceInfo(
        xMaster, UNO_QUERY_THROW );
    OSL_ASSERT( xMaster_XServiceInfo->getImplementationName().equals( test ) );

    Reference< reflection::XProxyFactory > xTarget( xMaster, UNO_QUERY_THROW );
    Reference< XCurrentContext > xTarget_XCurrentContext(
        xTarget, UNO_QUERY_THROW );
    Reference< XCurrentContext > xMaster_XCurrentContext(
        xMaster, UNO_QUERY_THROW );

    OSL_ASSERT(
        xTarget_XCurrentContext->getValueByName( test ) == makeAny( test ) );
    OSL_ASSERT(
        xMaster_XCurrentContext->getValueByName( test ) == makeAny( test ) );

    Reference< XAggregation > xFakeAgg( xTarget->createProxy( xTarget ) );
    if (xFakeAgg.is())
    {
        OSL_ASSERT( xTarget == xFakeAgg );
        OSL_ASSERT( xMaster == xFakeAgg );
    }

    Reference< lang::XServiceInfo > xTarget_XServiceInfo(
        xTarget, UNO_QUERY_THROW );
    OSL_ASSERT( xTarget_XServiceInfo->getImplementationName().equals( test ) );
    Reference< lang::XServiceInfo > xTarget_XServiceInfo2(
        xTarget, UNO_QUERY_THROW );
    OSL_ASSERT( xTarget_XServiceInfo2.get() == xTarget_XServiceInfo.get() );

    OSL_ASSERT( xTarget == xTarget_XCurrentContext );
    OSL_ASSERT( xTarget_XCurrentContext == xMaster );
    OSL_ASSERT(
        xTarget_XCurrentContext.get() == xMaster_XCurrentContext.get() );
    OSL_ASSERT( xTarget_XCurrentContext == xMaster );
    OSL_ASSERT( xTarget == xMaster );
    OSL_ASSERT( xTarget_XServiceInfo.get() == xMaster_XServiceInfo.get() );
    OSL_ASSERT( xTarget_XServiceInfo == xMaster );
    OSL_ASSERT( xMaster_XServiceInfo == xMaster );

    try
    {
        Reference< registry::XRegistryKey >(
            xMaster, UNO_QUERY_THROW );
    }
    catch (const lang::DisposedException & exc)
    {
        if ( exc.Message != "my test exception" )
            throw;
    }
}

static void test_proxyfac(
    Reference< XInterface > const & xMaster, OUString const & test,
    Reference< reflection::XProxyFactory > const & xProxyFac )
{
    test_proxyfac_( xMaster, test, xProxyFac );
    // proxy the proxy...
    Reference< XInterface > xNew( TestMaster::create( xMaster, xProxyFac ) );
    test_proxyfac_(
        xNew, OUString( "master" ), xProxyFac );
}

SAL_IMPLEMENT_MAIN()
{
    bool success = true;

    Environment cpp_env;
    OUString cpp( CPPU_CURRENT_LANGUAGE_BINDING_NAME );
    uno_getEnvironment(
        reinterpret_cast< uno_Environment ** >( &cpp_env ),
        cpp.pData, 0 );
    OSL_ENSURE( cpp_env.is(), "### cannot get C++ uno env!" );

    {
        Reference< lang::XMultiServiceFactory > xMgr(
            createRegistryServiceFactory(
                OUString( "stoctest.rdb" ) ) );

        try
        {
            Reference< registry::XImplementationRegistration > xImplReg(
                xMgr->createInstance( "com.sun.star.registry.ImplementationRegistration" ),
                UNO_QUERY );
            OSL_ENSURE( xImplReg.is(), "### no impl reg!" );

            OUString aLibName(
                "proxyfac.uno" SAL_DLLEXTENSION );
            xImplReg->registerImplementation(
                OUString( "com.sun.star.loader.SharedLibrary" ),
                aLibName, Reference< registry::XSimpleRegistry >() );

            Reference< reflection::XProxyFactory > xProxyFac(
                xMgr->createInstance(
                    OUString(
                        "com.sun.star.reflection.ProxyFactory") ),
                UNO_QUERY_THROW );

            Reference< XAggregation > x(
                xProxyFac->createProxy(
                    static_cast< OWeakObject * >( new TargetObject ) ) );
            // no call

            {
            Reference< XInterface > xMaster( TestMaster::create( xProxyFac ) );
            test_proxyfac(
                xMaster,
                OUString( "master" ),
                xProxyFac );
            }
            {
            Reference< XInterface > xMaster( TestMaster::create( xProxyFac ) );
            // no call
            }

            {
            Reference< XInterface > xMaster( TestMaster::create( xProxyFac ) );
            Reference< reflection::XProxyFactory > xSlave_lives_alone(
                xMaster, UNO_QUERY_THROW );
            xMaster.clear();
            test_proxyfac(
                xSlave_lives_alone,
                OUString( "master" ),
                xProxyFac );
            uno_dumpEnvironment( stdout, cpp_env.get(), 0 );
            }
            {
            Reference< XInterface > xMaster( TestMaster::create( xProxyFac ) );
            Reference< reflection::XProxyFactory > xSlave_lives_alone(
                xMaster, UNO_QUERY_THROW );
            // no call
            }

            test_proxyfac(
                xProxyFac->createProxy(
                    static_cast< OWeakObject * >( new TargetObject ) ),
                OUString( "target" ),
                xProxyFac );
            uno_dumpEnvironment( stdout, cpp_env.get(), 0 );
        }
        catch (const Exception & rExc)
        {
            (void)rExc;
            OSL_ENSURE(
                ! __FILE__,
                OUStringToOString(
                    rExc.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            success = false;
        }


        Reference< lang::XComponent > xComp;
        Reference< beans::XPropertySet >(
            xMgr, UNO_QUERY_THROW )->getPropertyValue(
                OUString( "DefaultContext" ) )
                    >>= xComp;
        xComp->dispose();
    }

    if (TestMaster::s_obj != 0)
        fprintf( stderr, "TestMaster objects: %d\n", TestMaster::s_obj );
    if (TargetObject::s_obj != 0)
        fprintf( stderr, "TargetObject objects: %d\n", TargetObject::s_obj );

    uno_dumpEnvironment( stdout, cpp_env.get(), 0 );
    void ** ppInterfaces;
    sal_Int32 len;
    uno_ExtEnvironment * env = cpp_env.get()->pExtEnv;
    (*env->getRegisteredInterfaces)(
        env, &ppInterfaces, &len, rtl_allocateMemory );
    if (len != 0)
        fprintf( stderr, "%d registered C++ interfaces left!\n", len );

    success &= (TestMaster::s_obj == 0 &&
                TargetObject::s_obj == 0 &&
                len == 0);
    if (success)
    {
        printf( "testproxyfac succeeded.\n" );
        return 0;
    }
    else
    {
        fprintf( stderr, "testproxyfac failed!\n" );
        return 1;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
