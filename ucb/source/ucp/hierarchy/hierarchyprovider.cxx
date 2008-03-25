/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hierarchyprovider.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 14:50:44 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 - XInitialization::initialize does not work any longer!

 *************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XOFFICEINSTALLATIONDIRECTORIES_HPP_
#include <com/sun/star/util/XOfficeInstallationDirectories.hpp>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif

#ifndef _HIERARCHYPROVIDER_HXX
#include "hierarchyprovider.hxx"
#endif
#ifndef _HIERARCHYCONTENT_HXX
#include "hierarchycontent.hxx"
#endif
#ifndef _HIERARCHYURI_HXX
#include "hierarchyuri.hxx"
#endif

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
                     rtl::OUString::createFromAscii(
                         "com.sun.star.comp.ucb.HierarchyContentProvider" ),
                     rtl::OUString::createFromAscii(
                         HIERARCHY_CONTENT_PROVIDER_SERVICE_NAME ) );

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
#if 0
    if ( aArguments.getLength() > 0 )
    {
         // Extract config provider from service init args.
         aArguments[ 0 ] >>= m_xConfigProvider;

        OSL_ENSURE( m_xConfigProvider.is(),
                    "HierarchyContentProvider::initialize - "
                    "No config provider!" );
    }
#else
    if ( aArguments.getLength() > 0 )
        OSL_ENSURE( false,
                    "HierarchyContentProvider::initialize : not supported!" );
#endif
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
//            OSL_ENSURE( sal_False,
//                        "HierarchyContentProvider::getConfigProvider - "
//                        "caught exception!" );
        }

        OSL_ENSURE( sal_False,
                    "HierarchyContentProvider::getConfigProvider - "
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
                OSL_ENSURE( sal_False,
                    "HierarchyContentProvider::getRootConfigReadNameAccess - "
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
                    aProperty.Name
                        = rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM( "nodepath" ) );
                    aProperty.Value <<= rtl::OUString(); // root path
                    aArguments[ 0 ] <<= aProperty;

                    (*it).second.bTriedToGetRootReadAccess = true;

                    (*it).second.xRootReadAccess
                        = uno::Reference< container::XHierarchicalNameAccess >(
                            xConfigProv->createInstanceWithArguments(
                                rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "com.sun.star.ucb."
                                        "HierarchyDataReadAccess" ) ),
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

                OSL_ENSURE( sal_False,
                    "HierarchyContentProvider::getRootConfigReadNameAccess - "
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

            uno::Reference< uno::XComponentContext > xCtx;
            uno::Reference< beans::XPropertySet > xPropSet(
                m_xSMgr, uno::UNO_QUERY );
            if ( xPropSet.is() )
            {
                xPropSet->getPropertyValue(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) )
                >>= xCtx;
            }

            OSL_ENSURE( xCtx.is(),
                        "Unable to obtain component context from "
                        "service manager!" );

            if ( xCtx.is() )
            {
                xCtx->getValueByName(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "/singletons/"
                        "com.sun.star.util.theOfficeInstallationDirectories" ) ) )
                >>= m_xOfficeInstDirs;

// Be silent. singleton only available in an Office environment.
//                OSL_ENSURE( m_xOfficeInstDirs.is(),
//                            "Unable to obtain office directories singleton!" );
            }
        }
    }
    return m_xOfficeInstDirs;
}

