/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: provprox.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:16:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _PROVPROX_HXX
#include "provprox.hxx"
#endif

using namespace rtl;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

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
                     OUString::createFromAscii(
                         "com.sun.star.comp.ucb.UcbContentProviderProxyFactory" ),
                     OUString::createFromAscii(
                         PROVIDER_FACTORY_SERVICE_NAME ) );

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
                            OUString::createFromAscii(
                                "com.sun.star.comp.ucb.UcbContentProviderProxy" ),
                            OUString::createFromAscii(
                             PROVIDER_PROXY_SERVICE_NAME ) );

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

    // OSL_ENSURE( sal_False,
    // "UcbContentProviderProxy::compareContentIds - No provider!" );

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
                OSL_ENSURE( sal_False,
                    "UcbContentProviderProxy::deregisterInstance - "
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
                    OSL_ENSURE( sal_False,
                        "UcbContentProviderProxy::getContentProvider - "
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
