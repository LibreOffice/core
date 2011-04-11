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
#include "precompiled_ucb.hxx"
#include <osl/diagnose.h>
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
                     OUString(RTL_CONSTASCII_USTRINGPARAM(
                         "com.sun.star.comp.ucb.UcbContentProviderProxyFactory" )),
                     OUString(RTL_CONSTASCII_USTRINGPARAM(
                         PROVIDER_FACTORY_SERVICE_NAME )) );

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
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.comp.ucb.UcbContentProviderProxy" )),
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                             PROVIDER_PROXY_SERVICE_NAME )) );

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
            if(m_aArguments.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("NoConfig")))
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
                "UcbContentProviderProxy::getContentProvider - No provider!" );
    return m_xTargetProvider;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
