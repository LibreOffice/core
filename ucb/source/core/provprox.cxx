/*************************************************************************
 *
 *  $RCSfile: provprox.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:52:48 $
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
                                     "UcbContentProviderProxyFactory" ),
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

XTYPEPROVIDER_IMPL_5( UcbContentProviderProxy,
                          XTypeProvider,
                      XServiceInfo,
                      XContentProvider,
                      XParameterizedContentProvider,
                         XContentProviderSupplier );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_NOFACTORY_IMPL_1( UcbContentProviderProxy,
                                OUString::createFromAscii(
                                                "UcbContentProviderProxy" ),
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

    OSL_ENSURE( sal_False,
                "UcbContentProviderProxy::compareContentIds - No provider!" );

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
                return xParamProvider->deregisterInstance( Template,
                                                                Arguments );
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
