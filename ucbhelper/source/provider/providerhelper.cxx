/*************************************************************************
 *
 *  $RCSfile: providerhelper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef __HASH_MAP__
#include <stl/hash_map>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROPERTYSETREGISTRYFACTORY_HPP_
#include <com/sun/star/ucb/XPropertySetRegistryFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROPERTYSETREGISTRY_HPP_
#include <com/sun/star/ucb/XPropertySetRegistry.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif
#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif

#define PROPSET_REGISTRY_FILE_NAME "dynprops.rdb"

using namespace rtl;
using namespace com::sun::star::container;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace ucb;

namespace ucb_impl
{

//=========================================================================
//
// Contents.
//
//=========================================================================

struct equalString
{
    bool operator()( const OUString& rKey11, const OUString& rKey22 ) const
      {
          return !!( rKey11 == rKey22 );
      }
};

struct hashString
{
    size_t operator()( const OUString & rName ) const
    {
        return rName.hashCode();
    }
};

typedef std::hash_map
<
    OUString,
    ContentImplHelper*,
    hashString,
    equalString
>
Contents;

//=========================================================================
//
// struct ContentProviderImplHelper_Impl.
//
//=========================================================================

struct ContentProviderImplHelper_Impl
{
    Reference< XPropertySetRegistry > m_xPropertySetRegistry;
    Contents m_aContents;
};

} // namespace ucb_impl

using namespace ucb_impl;

//=========================================================================
//=========================================================================
//
// ContentProviderImplHelper Implementation.
//
//=========================================================================
//=========================================================================

ContentProviderImplHelper::ContentProviderImplHelper(
                        const Reference< XMultiServiceFactory >& rXSMgr )
: m_xSMgr( rXSMgr ),
  m_pImpl( new ucb_impl::ContentProviderImplHelper_Impl )
{
}

//=========================================================================
// virtual
ContentProviderImplHelper::~ContentProviderImplHelper()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_3( ContentProviderImplHelper,
                   XTypeProvider,
                   XServiceInfo,
                   XContentProvider );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( ContentProviderImplHelper,
                         XTypeProvider,
                         XServiceInfo,
                         XContentProvider );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

/*

 Pure virtual. Must be implemented by derived classes!

// virtual
OUString SAL_CALL ContentProviderImplHelper::getImplementationName()
    throw( RuntimeException )
{
}

//=========================================================================
// virtual
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
ContentProviderImplHelper::getSupportedServiceNames()
    throw( RuntimeException )
{
}

*/

//=========================================================================
// virtual
sal_Bool SAL_CALL ContentProviderImplHelper::supportsService(
                                            const OUString& ServiceName )
    throw( RuntimeException )
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString* pArray = aSNL.getConstArray();
    for ( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
    {
        if ( pArray[ i ] == ServiceName )
            return sal_True;
    }

    return sal_False;
}

//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

/*

 Pure virtual. Must be implemented by derived classes!

// virtual
Reference< XContent > SAL_CALL ContentProviderImplHelper::queryContent(
                        const Reference< XContentIdentifier >& Identifier )
    throw( IllegalIdentifierException, RuntimeException )
{
}

*/

//=========================================================================
// virtual
sal_Int32 SAL_CALL ContentProviderImplHelper::compareContentIds(
                                const Reference< XContentIdentifier >& Id1,
                                const Reference< XContentIdentifier >& Id2 )
    throw( RuntimeException )
{
    // Simply do a string compare.

    OUString aURL1( Id1->getContentIdentifier() );
    OUString aURL2( Id2->getContentIdentifier() );

    return aURL1.compareTo( aURL2 );;
}

//=========================================================================
//
// Non-interface methods
//
//=========================================================================

void ContentProviderImplHelper::addContent(
                                    ::ucb::ContentImplHelper* pContent )
{
    vos::OGuard aGuard( m_aMutex );

    const OUString aURL( pContent->getIdentifier()->getContentIdentifier() );

#ifdef _DEBUG
    Contents::const_iterator it = m_pImpl->m_aContents.find( aURL );
    VOS_ENSURE( it == m_pImpl->m_aContents.end(),
                "ContentProviderImplHelper::addContent - Already registered!" );
#endif

    m_pImpl->m_aContents[ aURL ] = pContent;
}

//=========================================================================
void ContentProviderImplHelper::removeContent(
                                    ::ucb::ContentImplHelper* pContent )
{
    vos::OGuard aGuard( m_aMutex );

    const OUString aURL( pContent->getIdentifier()->getContentIdentifier() );
    removeContent( aURL );
}

//=========================================================================
void ContentProviderImplHelper::removeContent( const OUString& rURL )
{
    vos::OGuard aGuard( m_aMutex );

    Contents::iterator it = m_pImpl->m_aContents.find( rURL );

//  VOS_ENSURE( it != m_pImpl->m_aContents.end(),
//              "ContentProviderImplHelper::removeContent - Not registered!" );

    if ( it != m_pImpl->m_aContents.end() )
        m_pImpl->m_aContents.erase( it );
}

//=========================================================================
vos::ORef< ContentImplHelper >
ContentProviderImplHelper::queryExistingContent(
                        const Reference< XContentIdentifier >& Identifier )
{
    return queryExistingContent( Identifier->getContentIdentifier() );
}

//=========================================================================
vos::ORef< ContentImplHelper >
ContentProviderImplHelper::queryExistingContent( const OUString& rURL )
{
    vos::OGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...

    Contents::const_iterator it = m_pImpl->m_aContents.find( rURL );
    if ( it != m_pImpl->m_aContents.end() )
    {
        // Already there. Return it.
        return vos::ORef< ContentImplHelper >( (*it).second );
    }

    return vos::ORef< ContentImplHelper >();
}

//=========================================================================
void ContentProviderImplHelper::queryExistingContents(
                                            ContentRefList& rContents )
{
    vos::OGuard aGuard( m_aMutex );

    Contents::const_iterator it  = m_pImpl->m_aContents.begin();
    Contents::const_iterator end = m_pImpl->m_aContents.end();

    while ( it != end )
    {
        rContents.push_back( ContentImplHelperRef( (*it).second ) );
        ++it;
    }
}

//=========================================================================
Reference< XPropertySetRegistry >
ContentProviderImplHelper::getAdditionalPropertySetRegistry()
{
    // Get propertyset registry.

    vos::OGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_xPropertySetRegistry.is() )
    {
        Reference< XPropertySetRegistryFactory > xRegFac(
                m_xSMgr->createInstance(
                    OUString::createFromAscii( "com.sun.star.ucb.Store" ) ),
                UNO_QUERY );

        VOS_ENSURE( xRegFac.is(),
                    "ContentProviderImplHelper::getAdditionalPropertySet - "
                    "No UCB-Store service!" );

        Reference< XSimpleRegistry > xCfgReg(
                m_xSMgr->createInstance(
                    OUString::createFromAscii(
                               "com.sun.star.config.SpecialConfigManager" ) ),
                UNO_QUERY );

        VOS_ENSURE( xCfgReg.is(),
                    "ContentProviderImplHelper::getAdditionalPropertySet - "
                    "No ConfigManager service!" );

        Reference< XConfigManager > xCfgMgr;
        if ( xCfgReg.is() )
            xCfgMgr = Reference< XConfigManager >( xCfgReg, UNO_QUERY );

        VOS_ENSURE( xCfgMgr.is(),
                    "ContentProviderImplHelper::getAdditionalPropertySet - "
                    "No XConfigManager!" );

        if ( xRegFac.is() && xCfgReg.is() && xCfgMgr.is() )
        {
            OUString aRegURL;

            try
            {
                Reference< XRegistryKey > xRegistryKey( xCfgReg->getRootKey() );
                if ( xRegistryKey.is() )
                    xRegistryKey = xRegistryKey->openKey(
                        OUString::createFromAscii( "Directories/Storage-Dir") );

                if ( xRegistryKey.is() )
                    aRegURL = xRegistryKey->getStringValue();
            }
            catch ( InvalidRegistryException& )
            {
            }

            aRegURL = xCfgMgr->substituteVariables( aRegURL );

            VOS_ENSURE( aRegURL.getLength(),
                        "ContentProviderImplHelper::getAdditionalPropertySet - "
                        "Unable to get Store-directory!" );

            if ( aRegURL.getLength() )
            {
                // Convert path to a URL.
                OUString aUNCPath;
                osl::FileBase::RC
                eErr = osl::FileBase::normalizePath( aRegURL, aUNCPath );

                if ( eErr == osl::FileBase::E_None )
                {
                    eErr = osl::FileBase::getFileURLFromNormalizedPath(
                                                        aUNCPath, aRegURL );
                    if ( eErr == osl::FileBase::E_None )
                    {
                        OUString aLastChar(
                                    aRegURL.copy( aRegURL.getLength() - 1 ) );
                        OUString aTmpOWStr = OUString::createFromAscii ( "/" );
                        if ( aLastChar != aTmpOWStr )
                            aRegURL += aTmpOWStr;

                        aRegURL += OUString::createFromAscii(
                                                PROPSET_REGISTRY_FILE_NAME );

                        // Open/create a registry.
                        m_pImpl->m_xPropertySetRegistry
                            = xRegFac->createPropertySetRegistry( aRegURL );
                    }
                }

                VOS_ENSURE( m_pImpl->m_xPropertySetRegistry.is(),
                    "ContentProviderImplHelper::getAdditionalPropertySet - "
                    "Error opening registry!" );
            }
        }
    }

    return m_pImpl->m_xPropertySetRegistry;
}


//=========================================================================
Reference< XPersistentPropertySet >
ContentProviderImplHelper::getAdditionalPropertySet( const OUString& rKey,
                                                     sal_Bool bCreate )
{
    // Get propertyset registry.
    getAdditionalPropertySetRegistry();

    if ( m_pImpl->m_xPropertySetRegistry.is() )
    {
        // Open/create persistent property set.
        return Reference< XPersistentPropertySet >(
                      m_pImpl->m_xPropertySetRegistry->openPropertySet(
                                                            rKey, bCreate ) );
    }

    return Reference< XPersistentPropertySet >();
}

//=========================================================================
sal_Bool ContentProviderImplHelper::renameAdditionalPropertySet(
                                                      const OUString& rOldKey,
                                                      const OUString& rNewKey,
                                                      sal_Bool bRecursive )
{
    vos::OGuard aGuard( m_aMutex );

    if ( bRecursive )
    {
        // Get propertyset registry.
        getAdditionalPropertySetRegistry();

        if ( m_pImpl->m_xPropertySetRegistry.is() )
        {
            Reference< XNameAccess > xNameAccess(
                            m_pImpl->m_xPropertySetRegistry, UNO_QUERY );
            if ( xNameAccess.is() )
            {
                Sequence< OUString > aKeys = xNameAccess->getElementNames();
                sal_Int32 nCount = aKeys.getLength();
                if ( nCount > 0 )
                {
                    const OUString* pKeys = aKeys.getConstArray();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        const OUString& rKey = pKeys[ n ];
                        if ( rKey.compareTo(
                                        rOldKey, rOldKey.getLength() ) == 0 )
                        {
                            OUString aNewKey
                                = rKey.replaceAt(
                                    0, rOldKey.getLength(), rNewKey );
                            if ( !renameAdditionalPropertySet(
                                    rKey, aNewKey, sal_False ) )
                                return sal_False;
                        }
                    }
                }
            }
            else
                return sal_False;
        }
        else
            return sal_False;
    }
    else
    {
        // Get old property set, if exists.
        Reference< XPersistentPropertySet > xOldSet
                        = getAdditionalPropertySet( rOldKey, sal_False );
        if ( xOldSet.is() )
        {
            // Rename property set.
            Reference< XNamed > xNamed( xOldSet, UNO_QUERY );
            if ( xNamed.is() )
            {
                // ??? throws no exceptions and has no return value ???
                xNamed->setName( rNewKey );
            }
            else
                return sal_False;
        }
    }
    return sal_True;
}

//=========================================================================
sal_Bool ContentProviderImplHelper::removeAdditionalPropertySet(
                                                      const OUString& rKey,
                                                      sal_Bool bRecursive )
{
    vos::OGuard aGuard( m_aMutex );

    if ( bRecursive )
    {
        // Get propertyset registry.
        getAdditionalPropertySetRegistry();

        if ( m_pImpl->m_xPropertySetRegistry.is() )
        {
            Reference< XNameAccess > xNameAccess(
                            m_pImpl->m_xPropertySetRegistry, UNO_QUERY );
            if ( xNameAccess.is() )
            {
                Sequence< OUString > aKeys = xNameAccess->getElementNames();
                sal_Int32 nCount = aKeys.getLength();
                if ( nCount > 0 )
                {
                    const OUString* pKeys = aKeys.getConstArray();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        const OUString& rCurrKey = pKeys[ n ];
                        if ( rCurrKey.compareTo( rKey, rKey.getLength() ) == 0 )
                        {
                            if ( !removeAdditionalPropertySet(
                                                        rCurrKey, sal_False ) )
                                return sal_False;
                        }
                    }
                }
            }
            else
                return sal_False;
        }
        else
            return sal_False;
    }
    else
    {
        // Get propertyset registry.
        getAdditionalPropertySetRegistry();

        if ( m_pImpl->m_xPropertySetRegistry.is() )
            m_pImpl->m_xPropertySetRegistry->removePropertySet( rKey );
        else
            return sal_False;
    }
    return sal_True;
}

