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

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <rtl/strbuf.hxx>
#include "provprox.hxx"
#include <com/sun/star/lang/XInitialization.hpp>

using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

using ::rtl::OUString;

//=========================================================================
//=========================================================================
//
// UcbContentProviderProxyFactory Implementation.
//
//=========================================================================
//=========================================================================

UcbContentProviderProxyFactory::UcbContentProviderProxyFactory(
                        const Reference< XMultiServiceFactory >& rxSMgr )
: m_xSMgr( rxSMgr )
{
}

//=========================================================================
// virtual
UcbContentProviderProxyFactory::~UcbContentProviderProxyFactory()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_3( UcbContentProviderProxyFactory,
                   XTypeProvider,
                   XServiceInfo,
                   XContentProviderFactory );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( UcbContentProviderProxyFactory,
                          XTypeProvider,
                      XServiceInfo,
                          XContentProviderFactory );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1( UcbContentProviderProxyFactory,
                     OUString( "com.sun.star.comp.ucb.UcbContentProviderProxyFactory" ),
                     OUString( PROVIDER_FACTORY_SERVICE_NAME ) );

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL( UcbContentProviderProxyFactory );

//=========================================================================
//
// XContentProviderFactory methods.
//
//=========================================================================

// virtual
Reference< XContentProvider > SAL_CALL
UcbContentProviderProxyFactory::createContentProvider(
                                                const OUString& Service )
    throw( RuntimeException )
{
    return Reference< XContentProvider >(
                        new UcbContentProviderProxy( m_xSMgr, Service ) );
}

//=========================================================================
//=========================================================================
//
// UcbContentProviderProxy Implementation.
//
//=========================================================================
//=========================================================================

UcbContentProviderProxy::UcbContentProviderProxy(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        const OUString& Service )
: m_aService( Service ),
  m_bReplace( sal_False ),
  m_bRegister( sal_False ),
  m_xSMgr( rxSMgr )
{
}

//=========================================================================
// virtual
UcbContentProviderProxy::~UcbContentProviderProxy()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_COMMON_IMPL( UcbContentProviderProxy );

//============================================================================
// virtual
Any SAL_CALL
UcbContentProviderProxy::queryInterface( const Type & rType )
    throw ( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                static_cast< XTypeProvider * >( this ),
                static_cast< XServiceInfo * >( this ),
                static_cast< XContentProvider * >( this ),
                static_cast< XParameterizedContentProvider * >( this ),
                static_cast< XContentProviderSupplier * >( this ) );

    if ( !aRet.hasValue() )
        aRet = OWeakObject::queryInterface( rType );

    if ( !aRet.hasValue() )
    {
        // Get original provider an forward the call...
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        Reference< XContentProvider > xProvider = getContentProvider();
        if ( xProvider.is() )
            aRet = xProvider->queryInterface( rType );
    }

    return aRet;
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_COMMON_IMPL( UcbContentProviderProxy );

//=========================================================================

Sequence< Type > SAL_CALL UcbContentProviderProxy::getTypes()                                                           \
    throw( RuntimeException )
{
    // Get original provider an forward the call...
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    Reference< XTypeProvider > xProvider( getContentProvider(), UNO_QUERY );
    if ( xProvider.is() )
    {
        return xProvider->getTypes();
    }
    else
    {
        static cppu::OTypeCollection collection(
            CPPU_TYPE_REF( XTypeProvider ),
            CPPU_TYPE_REF( XServiceInfo ),
            CPPU_TYPE_REF( XContentProvider ),
            CPPU_TYPE_REF( XParameterizedContentProvider ),
            CPPU_TYPE_REF( XContentProviderSupplier ) );
        return collection.getTypes();
    }
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_NOFACTORY_IMPL_1( UcbContentProviderProxy,
                            OUString( "com.sun.star.comp.ucb.UcbContentProviderProxy" ),
                            OUString( PROVIDER_PROXY_SERVICE_NAME ) );

//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

// virtual
Reference< XContent > SAL_CALL UcbContentProviderProxy::queryContent(
                        const Reference< XContentIdentifier >& Identifier )
    throw( IllegalIdentifierException,
           RuntimeException )
{
    // Get original provider an forward the call...

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    Reference< XContentProvider > xProvider = getContentProvider();
    if ( xProvider.is() )
        return xProvider->queryContent( Identifier );

    return Reference< XContent >();
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL UcbContentProviderProxy::compareContentIds(
                       const Reference< XContentIdentifier >& Id1,
                       const Reference< XContentIdentifier >& Id2 )
    throw( RuntimeException )
{
    // Get original provider an forward the call...

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    Reference< XContentProvider > xProvider = getContentProvider();
    if ( xProvider.is() )
        return xProvider->compareContentIds( Id1, Id2 );

    // OSL_FAIL( // "UcbContentProviderProxy::compareContentIds - No provider!" );

    // @@@ What else?
    return 0;
}

//=========================================================================
//
// XParameterizedContentProvider methods.
//
//=========================================================================

// virtual
Reference< XContentProvider > SAL_CALL
UcbContentProviderProxy::registerInstance( const OUString& Template,
                                             const OUString& Arguments,
                                             sal_Bool ReplaceExisting )
    throw( IllegalArgumentException,
           RuntimeException )
{
    // Just remember that this method was called ( and the params ).

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_bRegister )
    {
//      m_xTargetProvider = 0;
        m_aTemplate  = Template;
        m_aArguments = Arguments;
        m_bReplace   = ReplaceExisting;

        m_bRegister  = sal_True;
    }
    return this;
}

//=========================================================================
// virtual
Reference< XContentProvider > SAL_CALL
UcbContentProviderProxy::deregisterInstance( const OUString& Template,
                                             const OUString& Arguments )
    throw( IllegalArgumentException,
           RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // registerInstance called at proxy and at original?
    if ( m_bRegister && m_xTargetProvider.is() )
    {
        m_bRegister       = sal_False;
        m_xTargetProvider = 0;

        Reference< XParameterizedContentProvider >
                                xParamProvider( m_xProvider, UNO_QUERY );
        if ( xParamProvider.is() )
        {
            try
            {
                xParamProvider->deregisterInstance( Template, Arguments );
            }
            catch ( IllegalIdentifierException const & )
            {
                OSL_FAIL( "UcbContentProviderProxy::deregisterInstance - "
                    "Caught IllegalIdentifierException!" );
            }
        }
    }

    return this;
}

//=========================================================================
//
// XContentProviderSupplier methods.
//
//=========================================================================

// virtual
Reference< XContentProvider > SAL_CALL
UcbContentProviderProxy::getContentProvider()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if ( !m_xProvider.is() )
    {
        try
        {
            m_xProvider
                = Reference< XContentProvider >(
                      m_xSMgr->createInstance( m_aService ), UNO_QUERY );
            if ( m_aArguments == "NoConfig" )
            {
                Reference<XInitialization> xInit(m_xProvider,UNO_QUERY);
                if(xInit.is()) {
                    Sequence<Any> aArgs(1);
                    aArgs[0] <<= m_aArguments;
                    xInit->initialize(aArgs);
                }
            }
        }
        catch ( RuntimeException const & )
        {
            throw;
        }
        catch ( Exception const & )
        {
        }

        // registerInstance called at proxy, but not yet at original?
        if ( m_xProvider.is() && m_bRegister )
        {
            Reference< XParameterizedContentProvider >
                xParamProvider( m_xProvider, UNO_QUERY );
            if ( xParamProvider.is() )
            {
                try
                {
                    m_xTargetProvider
                        = xParamProvider->registerInstance( m_aTemplate,
                                                            m_aArguments,
                                                            m_bReplace );
                }
                catch ( IllegalIdentifierException const & )
                {
                    OSL_FAIL( "UcbContentProviderProxy::getContentProvider - "
                        "Caught IllegalIdentifierException!" );
                }

                OSL_ENSURE( m_xTargetProvider.is(),
                    "UcbContentProviderProxy::getContentProvider - "
                    "No provider!" );
            }
        }
        if ( !m_xTargetProvider.is() )
            m_xTargetProvider = m_xProvider;
    }

    OSL_ENSURE( m_xProvider.is(),
        rtl::OStringBuffer("UcbContentProviderProxy::getContentProvider - No provider for '").append(rtl::OUStringToOString(m_aService, osl_getThreadTextEncoding())).append(".").getStr() );
    return m_xTargetProvider;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
