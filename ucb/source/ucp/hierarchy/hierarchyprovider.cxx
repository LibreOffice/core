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


/**************************************************************************
                                TODO
 **************************************************************************

 - XInitialization::initialize does not work any longer!

 *************************************************************************/
#include <osl/diagnose.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XOfficeInstallationDirectories.hpp>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include "hierarchyprovider.hxx"
#include "hierarchycontent.hxx"
#include "hierarchyuri.hxx"

#include "../inc/urihelper.hxx"

using namespace com::sun::star;
using namespace hierarchy_ucp;

//=========================================================================
//=========================================================================
//
// HierarchyContentProvider Implementation.
//
//=========================================================================
//=========================================================================

HierarchyContentProvider::HierarchyContentProvider(
            const uno::Reference< lang::XMultiServiceFactory >& rXSMgr )
: ::ucbhelper::ContentProviderImplHelper( rXSMgr )
{
}

//=========================================================================
// virtual
HierarchyContentProvider::~HierarchyContentProvider()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_4( HierarchyContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   ucb::XContentProvider,
                   lang::XInitialization );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_4( HierarchyContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider,
                      lang::XInitialization );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1( HierarchyContentProvider,
                     rtl::OUString( "com.sun.star.comp.ucb.HierarchyContentProvider" ),
                     rtl::OUString( HIERARCHY_CONTENT_PROVIDER_SERVICE_NAME ) );

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL( HierarchyContentProvider );

//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

// virtual
uno::Reference< ucb::XContent > SAL_CALL
HierarchyContentProvider::queryContent(
        const uno::Reference< ucb::XContentIdentifier >& Identifier )
    throw( ucb::IllegalIdentifierException, uno::RuntimeException )
{
    HierarchyUri aUri( Identifier->getContentIdentifier() );
    if ( !aUri.isValid() )
        throw ucb::IllegalIdentifierException();

    // Encode URL and create new Id. This may "correct" user-typed-in URL's.
    uno::Reference< ucb::XContentIdentifier > xCanonicId
        = new ::ucbhelper::ContentIdentifier( m_xSMgr,
                                              ::ucb_impl::urihelper::encodeURI(
                                                  aUri.getUri() ) );
    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).get();
    if ( xContent.is() )
        return xContent;

    // Create a new content.
    xContent = HierarchyContent::create( m_xSMgr, this, xCanonicId );
    registerNewContent( xContent );

    if ( xContent.is() && !xContent->getIdentifier().is() )
        throw ucb::IllegalIdentifierException();

    return xContent;
}

//=========================================================================
//
// XInitialization methods.
//
//=========================================================================

// virtual
void SAL_CALL HierarchyContentProvider::initialize(
                                const uno::Sequence< uno::Any >& aArguments )
    throw( uno::Exception, uno::RuntimeException )
{
    if ( aArguments.getLength() > 0 )
        OSL_FAIL( "HierarchyContentProvider::initialize : not supported!" );
}

//=========================================================================
//
//  Non-interface methods.
//
//=========================================================================

uno::Reference< lang::XMultiServiceFactory >
HierarchyContentProvider::getConfigProvider(
                                const rtl::OUString & rServiceSpecifier )
{
    osl::MutexGuard aGuard( m_aMutex );
    ConfigProviderMap::iterator it = m_aConfigProviderMap.find(
                                                    rServiceSpecifier );
    if ( it == m_aConfigProviderMap.end() )
    {
        try
        {
            ConfigProviderMapEntry aEntry;
            aEntry.xConfigProvider
                = uno::Reference< lang::XMultiServiceFactory >(
                                m_xSMgr->createInstance( rServiceSpecifier ),
                                uno::UNO_QUERY );

            if ( aEntry.xConfigProvider.is() )
            {
                m_aConfigProviderMap[ rServiceSpecifier ] = aEntry;
                return aEntry.xConfigProvider;
            }
        }
        catch ( uno::Exception const & )
        {
//            OSL_FAIL( //                        "HierarchyContentProvider::getConfigProvider - "
//                        "caught exception!" );
        }

        OSL_FAIL( "HierarchyContentProvider::getConfigProvider - "
                    "No config provider!" );

        return uno::Reference< lang::XMultiServiceFactory >();
    }

    return (*it).second.xConfigProvider;
}

//=========================================================================
uno::Reference< container::XHierarchicalNameAccess >
HierarchyContentProvider::getRootConfigReadNameAccess(
                                const rtl::OUString & rServiceSpecifier )
{
    osl::MutexGuard aGuard( m_aMutex );
    ConfigProviderMap::iterator it = m_aConfigProviderMap.find(
                                                    rServiceSpecifier );
    if ( it != m_aConfigProviderMap.end() )
    {
        if ( !( (*it).second.xRootReadAccess.is() ) )
        {
            if ( (*it).second.bTriedToGetRootReadAccess ) // #82494#
            {
                OSL_FAIL( "HierarchyContentProvider::getRootConfigReadNameAccess - "
                    "Unable to read any config data! -> #82494#" );
                return uno::Reference< container::XHierarchicalNameAccess >();
            }

            try
            {
                uno::Reference< lang::XMultiServiceFactory > xConfigProv
                    = getConfigProvider( rServiceSpecifier );

                if ( xConfigProv.is() )
                {
                    uno::Sequence< uno::Any > aArguments( 1 );
                    beans::PropertyValue      aProperty;
                    aProperty.Name = rtl::OUString( "nodepath"  );
                    aProperty.Value <<= rtl::OUString(); // root path
                    aArguments[ 0 ] <<= aProperty;

                    (*it).second.bTriedToGetRootReadAccess = true;

                    (*it).second.xRootReadAccess
                        = uno::Reference< container::XHierarchicalNameAccess >(
                            xConfigProv->createInstanceWithArguments(
                                rtl::OUString( "com.sun.star.ucb.HierarchyDataReadAccess"  ),
                                aArguments ),
                            uno::UNO_QUERY );
                }
            }
            catch ( uno::RuntimeException const & )
            {
                throw;
            }
            catch ( uno::Exception const & )
            {
                // createInstance, createInstanceWithArguments

                OSL_FAIL( "HierarchyContentProvider::getRootConfigReadNameAccess - "
                    "caught Exception!" );
            }
        }
    }

    return (*it).second.xRootReadAccess;
}

//=========================================================================
uno::Reference< util::XOfficeInstallationDirectories >
HierarchyContentProvider::getOfficeInstallationDirectories()
{
    if ( !m_xOfficeInstDirs.is() )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xOfficeInstDirs.is() )
        {
            OSL_ENSURE( m_xSMgr.is(), "No service manager!" );

            uno::Reference< uno::XComponentContext > xCtx(
                comphelper::getComponentContext( m_xSMgr ) );

            xCtx->getValueByName(
                rtl::OUString( "/singletons/com.sun.star.util.theOfficeInstallationDirectories"  ) )
                >>= m_xOfficeInstDirs;

// Be silent. singleton only available in an Office environment.
//          OSL_ENSURE( m_xOfficeInstDirs.is(),
//                      "Unable to obtain office directories singleton!" );
        }
    }
    return m_xOfficeInstDirs;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
