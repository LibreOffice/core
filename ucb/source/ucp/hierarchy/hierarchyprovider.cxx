/*************************************************************************
 *
 *  $RCSfile: hierarchyprovider.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kso $ $Date: 2001-04-05 09:48:49 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
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

using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace rtl;

using namespace hierarchy_ucp;

//=========================================================================
//=========================================================================
//
// HierarchyContentProvider Implementation.
//
//=========================================================================
//=========================================================================

HierarchyContentProvider::HierarchyContentProvider(
                        const Reference< XMultiServiceFactory >& rXSMgr )
: ::ucb::ContentProviderImplHelper( rXSMgr ),
  m_bTriedToGetRootReadAccess( sal_False )
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
                   XTypeProvider,
                   XServiceInfo,
                   XContentProvider,
                   XInitialization );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_4( HierarchyContentProvider,
                         XTypeProvider,
                         XServiceInfo,
                         XContentProvider,
                      XInitialization );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1( HierarchyContentProvider,
                     OUString::createFromAscii(
                         "com.sun.star.comp.ucb.HierarchyContentProvider" ),
                     OUString::createFromAscii(
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
Reference< XContent > SAL_CALL HierarchyContentProvider::queryContent(
                        const Reference< XContentIdentifier >& Identifier )
    throw( IllegalIdentifierException, RuntimeException )
{
    vos::OGuard aGuard( m_aMutex );

    // Check URL scheme...

    OUString aScheme( OUString::createFromAscii( HIERARCHY_URL_SCHEME ) );
    if ( !Identifier->getContentProviderScheme().equalsIgnoreCase( aScheme ) )
        throw IllegalIdentifierException();

    // Check URL. Must be at least the URL of the Root Folder.
    OUString aId = Identifier->getContentIdentifier();
    if ( aId.compareToAscii( HIERARCHY_ROOT_FOLDER_URL,
                             HIERARCHY_ROOT_FOLDER_URL_LENGTH ) != 0 )
        throw IllegalIdentifierException();

    // Encode URL and create new Id. This may "correct" user-typed-in URL's.
    Reference< XContentIdentifier > xCanonicId =
                new ::ucb::ContentIdentifier( m_xSMgr, encodeURL( aId ) );

    // Check, if a content with given id already exists...
    Reference< XContent > xContent
        = queryExistingContent( xCanonicId ).getBodyPtr();
    if ( xContent.is() )
        return xContent;

    // Create a new content. Note that the content will insert itself
    // into m_pContents by calling addContent(...) from it's ctor.

    xContent = HierarchyContent::create( m_xSMgr, this, xCanonicId );

    if ( xContent.is() && !xContent->getIdentifier().is() )
        throw IllegalIdentifierException();

    return xContent;
}

//=========================================================================
//
// XInitialization methods.
//
//=========================================================================

// virtual
void SAL_CALL HierarchyContentProvider::initialize(
                                    const Sequence< Any >& aArguments )
    throw( Exception, RuntimeException )
{
    if ( aArguments.getLength() > 0 )
    {
         // Extract config provider from service init args.
         aArguments[ 0 ] >>= m_xConfigProvider;

        VOS_ENSURE( m_xConfigProvider.is(),
                    "HierarchyContentProvider::initialize - "
                    "No config provider!" );
    }
}

//=========================================================================
//
//  Non-interface methods.
//
//=========================================================================

Reference< XMultiServiceFactory > HierarchyContentProvider::getConfigProvider()
{
    if ( !m_xConfigProvider.is() )
    {
        vos::OGuard aGuard( m_aMutex );
        if ( !m_xConfigProvider.is() )
        {
            try
            {
               m_xConfigProvider = Reference< XMultiServiceFactory >(
                    m_xSMgr->createInstance(
                        OUString::createFromAscii( "com.sun.star.configuration."
                                                   "ConfigurationProvider" ) ),
                    UNO_QUERY );

                VOS_ENSURE( m_xConfigProvider.is(),
                            "HierarchyContentProvider::getConfigProvider - "
                            "No config provider!" );
            }
            catch ( Exception& )
            {
                VOS_ENSURE( sal_False,
                               "HierarchyContentProvider::getConfigProvider - "
                               "caught exception!" );
            }
        }
    }

    return m_xConfigProvider;
}

//=========================================================================
Reference< XHierarchicalNameAccess >
HierarchyContentProvider::getRootConfigReadNameAccess()
{
    if ( !m_xRootConfigReadNameAccess.is() )
    {
        vos::OGuard aGuard( m_aMutex );
        if ( !m_xRootConfigReadNameAccess.is() )
        {
            if ( m_bTriedToGetRootReadAccess ) // #82494#
            {
                OSL_ENSURE( sal_False,
                    "HierarchyContentProvider::getRootConfigReadNameAccess - "
                    "Unable to read any config data! -> #82494#" );
                return Reference< XHierarchicalNameAccess >();
            }

            try
            {
                getConfigProvider();

                if ( m_xConfigProvider.is() )
                {
                    Sequence< Any > aArguments( 1 );
                    aArguments[ 0 ] <<= OUString::createFromAscii(
                                        "/org.openoffice.ucb.Hierarchy/Root" );

                    m_bTriedToGetRootReadAccess = sal_True;

                    m_xRootConfigReadNameAccess
                        = Reference< XHierarchicalNameAccess >(
                            m_xConfigProvider->createInstanceWithArguments(
                                OUString::createFromAscii(
                                    "com.sun.star.configuration."
                                    "ConfigurationAccess" ),
                                aArguments ),
                            UNO_QUERY );
                }
            }
            catch ( RuntimeException& )
            {
                throw;
            }
            catch ( Exception& )
            {
                // createInstance, createInstanceWithArguments

                VOS_ENSURE( sal_False,
                    "HierarchyContentProvider::getRootConfigReadNameAccess - "
                    "caught Exception!" );
            }
        }
    }

    return m_xRootConfigReadNameAccess;
}

