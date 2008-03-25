/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: providerhelper.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:26:51 $
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
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <hash_map>

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
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

#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "cppuhelper/weakref.hxx"

#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif
#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif

using namespace com::sun::star;

namespace ucbhelper_impl
{

//=========================================================================
//
// Contents.
//
//=========================================================================

struct equalString
{
    bool operator()(
        const rtl::OUString& rKey11, const rtl::OUString& rKey22 ) const
      {
          return !!( rKey11 == rKey22 );
      }
};

struct hashString
{
    size_t operator()( const rtl::OUString & rName ) const
    {
        return rName.hashCode();
    }
};

typedef std::hash_map
<
    rtl::OUString,
    uno::WeakReference< ucb::XContent >,
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
    uno::Reference< com::sun::star::ucb::XPropertySetRegistry >
        m_xPropertySetRegistry;
    Contents
        m_aContents;
};

} // namespace ucbhelper_impl

//=========================================================================
//=========================================================================
//
// ContentProviderImplHelper Implementation.
//
//=========================================================================
//=========================================================================

namespace ucbhelper {

ContentProviderImplHelper::ContentProviderImplHelper(
    const uno::Reference< lang::XMultiServiceFactory >& rXSMgr )
: m_pImpl( new ucbhelper_impl::ContentProviderImplHelper_Impl ),
  m_xSMgr( rXSMgr )
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
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   com::sun::star::ucb::XContentProvider );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( ContentProviderImplHelper,
                         lang::XTypeProvider,
                         lang::XServiceInfo,
                         com::sun::star::ucb::XContentProvider );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
sal_Bool SAL_CALL ContentProviderImplHelper::supportsService(
                                            const rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSNL = getSupportedServiceNames();
    const rtl::OUString* pArray = aSNL.getConstArray();
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

// virtual
sal_Int32 SAL_CALL ContentProviderImplHelper::compareContentIds(
        const uno::Reference< com::sun::star::ucb::XContentIdentifier >& Id1,
        const uno::Reference< com::sun::star::ucb::XContentIdentifier >& Id2 )
    throw( uno::RuntimeException )
{
    // Simply do a string compare.

    rtl::OUString aURL1( Id1->getContentIdentifier() );
    rtl::OUString aURL2( Id2->getContentIdentifier() );

    return aURL1.compareTo( aURL2 );;
}

//=========================================================================
//
// Non-interface methods
//
//=========================================================================

void ContentProviderImplHelper::cleanupRegisteredContents()
{
    osl::MutexGuard aGuard( m_aMutex );

    ucbhelper_impl::Contents::iterator it
        = m_pImpl->m_aContents.begin();
    while( it != m_pImpl->m_aContents.end() )
    {
        uno::Reference< ucb::XContent > xContent( (*it).second );
        if ( !xContent.is() )
        {
            ucbhelper_impl::Contents::iterator tmp = it;
            ++it;
            m_pImpl->m_aContents.erase( tmp );
        }
        else
        {
            ++it;
        }
    }
}

//=========================================================================

void ContentProviderImplHelper::removeContent( ContentImplHelper* pContent )
{
    osl::MutexGuard aGuard( m_aMutex );

    cleanupRegisteredContents();

    const rtl::OUString aURL(
        pContent->getIdentifier()->getContentIdentifier() );

    ucbhelper_impl::Contents::iterator it = m_pImpl->m_aContents.find( aURL );

    if ( it != m_pImpl->m_aContents.end() )
        m_pImpl->m_aContents.erase( it );
}

//=========================================================================
rtl::Reference< ContentImplHelper >
ContentProviderImplHelper::queryExistingContent(
    const uno::Reference< com::sun::star::ucb::XContentIdentifier >&
        Identifier )
{
    return queryExistingContent( Identifier->getContentIdentifier() );
}

//=========================================================================
rtl::Reference< ContentImplHelper >
ContentProviderImplHelper::queryExistingContent( const rtl::OUString& rURL )
{
    osl::MutexGuard aGuard( m_aMutex );

    cleanupRegisteredContents();

    // Check, if a content with given id already exists...

    ucbhelper_impl::Contents::const_iterator it
        = m_pImpl->m_aContents.find( rURL );
    if ( it != m_pImpl->m_aContents.end() )
    {
        uno::Reference< ucb::XContent > xContent( (*it).second );
        if ( xContent.is() )
        {
            return rtl::Reference< ContentImplHelper >(
                static_cast< ContentImplHelper * >( xContent.get() ) );
        }
    }
    return rtl::Reference< ContentImplHelper >();
}

//=========================================================================
void ContentProviderImplHelper::queryExistingContents(
        ContentRefList& rContents )
{
    osl::MutexGuard aGuard( m_aMutex );

    cleanupRegisteredContents();

    ucbhelper_impl::Contents::const_iterator it
        = m_pImpl->m_aContents.begin();
    ucbhelper_impl::Contents::const_iterator end
        = m_pImpl->m_aContents.end();

    while ( it != end )
    {
        uno::Reference< ucb::XContent > xContent( (*it).second );
        if ( xContent.is() )
        {
            rContents.push_back(
                rtl::Reference< ContentImplHelper >(
                    static_cast< ContentImplHelper * >( xContent.get() ) ) );
        }
        ++it;
    }
}

//=========================================================================
void ContentProviderImplHelper::registerNewContent(
    const uno::Reference< ucb::XContent > & xContent )
{
    if ( xContent.is() )
    {
        osl::MutexGuard aGuard( m_aMutex );

        cleanupRegisteredContents();

        const rtl::OUString aURL(
            xContent->getIdentifier()->getContentIdentifier() );
        ucbhelper_impl::Contents::const_iterator it
            = m_pImpl->m_aContents.find( aURL );
        if ( it == m_pImpl->m_aContents.end() )
            m_pImpl->m_aContents[ aURL ] = xContent;
    }
}

//=========================================================================
uno::Reference< com::sun::star::ucb::XPropertySetRegistry >
ContentProviderImplHelper::getAdditionalPropertySetRegistry()
{
    // Get propertyset registry.

    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_xPropertySetRegistry.is() )
    {
        uno::Reference< com::sun::star::ucb::XPropertySetRegistryFactory >
            xRegFac(
                m_xSMgr->createInstance(
                    rtl::OUString::createFromAscii(
                        "com.sun.star.ucb.Store" ) ),
                uno::UNO_QUERY );

        OSL_ENSURE( xRegFac.is(),
                    "ContentProviderImplHelper::getAdditionalPropertySet - "
                    "No UCB-Store service!" );

        if ( xRegFac.is() )
        {
            // Open/create a registry.
            m_pImpl->m_xPropertySetRegistry
                = xRegFac->createPropertySetRegistry( rtl::OUString() );

            OSL_ENSURE( m_pImpl->m_xPropertySetRegistry.is(),
                    "ContentProviderImplHelper::getAdditionalPropertySet - "
                    "Error opening registry!" );
        }
    }

    return m_pImpl->m_xPropertySetRegistry;
}


//=========================================================================
uno::Reference< com::sun::star::ucb::XPersistentPropertySet >
ContentProviderImplHelper::getAdditionalPropertySet(
    const rtl::OUString& rKey, sal_Bool bCreate )
{
    // Get propertyset registry.
    getAdditionalPropertySetRegistry();

    if ( m_pImpl->m_xPropertySetRegistry.is() )
    {
        // Open/create persistent property set.
        return uno::Reference< com::sun::star::ucb::XPersistentPropertySet >(
            m_pImpl->m_xPropertySetRegistry->openPropertySet(
                rKey, bCreate ) );
    }

    return uno::Reference< com::sun::star::ucb::XPersistentPropertySet >();
}

//=========================================================================
sal_Bool ContentProviderImplHelper::renameAdditionalPropertySet(
    const rtl::OUString& rOldKey,
    const rtl::OUString& rNewKey,
    sal_Bool bRecursive )
{
    if ( rOldKey == rNewKey )
        return sal_True;

    osl::MutexGuard aGuard( m_aMutex );

    if ( bRecursive )
    {
        // Get propertyset registry.
        getAdditionalPropertySetRegistry();

        if ( m_pImpl->m_xPropertySetRegistry.is() )
        {
            uno::Reference< container::XNameAccess > xNameAccess(
                m_pImpl->m_xPropertySetRegistry, uno::UNO_QUERY );
            if ( xNameAccess.is() )
            {
                uno::Sequence< rtl::OUString > aKeys
                    = xNameAccess->getElementNames();
                sal_Int32 nCount = aKeys.getLength();
                if ( nCount > 0 )
                {
                    rtl::OUString aOldKeyWithSlash = rOldKey;
                    rtl::OUString aOldKeyWithoutSlash;
                    if ( aOldKeyWithSlash.lastIndexOf(
                             sal_Unicode('/')
                                 != aOldKeyWithSlash.getLength() - 1 ) )
                    {
                        aOldKeyWithSlash += rtl::OUString( sal_Unicode('/') );
                        aOldKeyWithoutSlash = rOldKey;
                    }
                    else if ( rOldKey.getLength() )
                        aOldKeyWithoutSlash
                            = rOldKey.copy( 0, rOldKey.getLength() - 1 );

                    const rtl::OUString* pKeys = aKeys.getConstArray();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        const rtl::OUString& rKey = pKeys[ n ];
                        if ( rKey.compareTo(
                                 aOldKeyWithSlash,
                                 aOldKeyWithSlash.getLength() ) == 0
                             || rKey.equals( aOldKeyWithoutSlash ) )
                        {
                            rtl::OUString aNewKey
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
        uno::Reference< com::sun::star::ucb::XPersistentPropertySet > xOldSet
            = getAdditionalPropertySet( rOldKey, sal_False );
        if ( xOldSet.is() )
        {
            // Rename property set.
            uno::Reference< container::XNamed > xNamed(
                xOldSet, uno::UNO_QUERY );
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
sal_Bool ContentProviderImplHelper::copyAdditionalPropertySet(
    const rtl::OUString& rSourceKey,
    const rtl::OUString& rTargetKey,
    sal_Bool bRecursive )
{
    if ( rSourceKey == rTargetKey )
        return sal_True;

    osl::MutexGuard aGuard( m_aMutex );

    if ( bRecursive )
    {
        // Get propertyset registry.
        getAdditionalPropertySetRegistry();

        if ( m_pImpl->m_xPropertySetRegistry.is() )
        {
            uno::Reference< container::XNameAccess > xNameAccess(
                m_pImpl->m_xPropertySetRegistry, uno::UNO_QUERY );
            if ( xNameAccess.is() )
            {
                uno::Sequence< rtl::OUString > aKeys
                    = xNameAccess->getElementNames();
                sal_Int32 nCount = aKeys.getLength();
                if ( nCount > 0 )
                {
                    rtl::OUString aSrcKeyWithSlash = rSourceKey;
                    rtl::OUString aSrcKeyWithoutSlash;
                    if ( aSrcKeyWithSlash.lastIndexOf(
                             sal_Unicode('/')
                             != aSrcKeyWithSlash.getLength() - 1 ) )
                    {
                        aSrcKeyWithSlash += rtl::OUString( sal_Unicode('/') );
                        aSrcKeyWithoutSlash = rSourceKey;
                    }
                    else if ( rSourceKey.getLength() )
                        aSrcKeyWithoutSlash = rSourceKey.copy(
                            0, rSourceKey.getLength() - 1 );

                    const rtl::OUString* pKeys = aKeys.getConstArray();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        const rtl::OUString& rKey = pKeys[ n ];
                        if ( rKey.compareTo(
                                 aSrcKeyWithSlash,
                                 aSrcKeyWithSlash.getLength() ) == 0
                             || rKey.equals( aSrcKeyWithoutSlash ) )
                        {
                            rtl::OUString aNewKey
                                = rKey.replaceAt(
                                    0, rSourceKey.getLength(), rTargetKey );
                            if ( !copyAdditionalPropertySet(
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
        uno::Reference< com::sun::star::ucb::XPersistentPropertySet >
            xOldPropSet = getAdditionalPropertySet( rSourceKey, sal_False );
        if ( !xOldPropSet.is() )
            return sal_False;

        uno::Reference< beans::XPropertySetInfo > xPropSetInfo
            = xOldPropSet->getPropertySetInfo();
        if ( !xPropSetInfo.is() )
            return sal_False;

        uno::Reference< beans::XPropertyAccess > xOldPropAccess(
            xOldPropSet, uno::UNO_QUERY );
        if ( !xOldPropAccess.is() )
            return sal_False;

        // Obtain all values from old set.
        uno::Sequence< beans::PropertyValue > aValues
            = xOldPropAccess->getPropertyValues();
        sal_Int32 nCount = aValues.getLength();

        uno::Sequence< beans::Property > aProps
            = xPropSetInfo->getProperties();

        if ( nCount )
        {
            // Fail, if property set with new key already exists.
            uno::Reference< com::sun::star::ucb::XPersistentPropertySet >
                xNewPropSet
                    = getAdditionalPropertySet( rTargetKey, sal_False );
            if ( xNewPropSet.is() )
                return sal_False;

            // Create new, empty set.
            xNewPropSet = getAdditionalPropertySet( rTargetKey, sal_True );
            if ( !xNewPropSet.is() )
                return sal_False;

            uno::Reference< beans::XPropertyContainer > xNewPropContainer(
                xNewPropSet, uno::UNO_QUERY );
            if ( !xNewPropContainer.is() )
                return sal_False;

            for ( sal_Int32 n = 0; n < nCount; ++n )
            {
                const beans::PropertyValue& rValue = aValues[ n ];

                sal_Int16 nAttribs = 0;
                for ( sal_Int32 m = 0; m < aProps.getLength(); ++m )
                {
                    if ( aProps[ m ].Name == rValue.Name )
                    {
                        nAttribs = aProps[ m ].Attributes;
                        break;
                    }
                }

                try
                {
                    xNewPropContainer->addProperty(
                        rValue.Name, nAttribs, rValue.Value );
                }
                catch ( beans::PropertyExistException & )
                {
                }
                   catch ( beans::IllegalTypeException & )
                {
                }
                catch ( lang::IllegalArgumentException & )
                {
                }
            }
        }
    }
    return sal_True;
}

//=========================================================================
sal_Bool ContentProviderImplHelper::removeAdditionalPropertySet(
    const rtl::OUString& rKey, sal_Bool bRecursive )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( bRecursive )
    {
        // Get propertyset registry.
        getAdditionalPropertySetRegistry();

        if ( m_pImpl->m_xPropertySetRegistry.is() )
        {
            uno::Reference< container::XNameAccess > xNameAccess(
                m_pImpl->m_xPropertySetRegistry, uno::UNO_QUERY );
            if ( xNameAccess.is() )
            {
                uno::Sequence< rtl::OUString > aKeys
                    = xNameAccess->getElementNames();
                sal_Int32 nCount = aKeys.getLength();
                if ( nCount > 0 )
                {
                    rtl::OUString aKeyWithSlash = rKey;
                    rtl::OUString aKeyWithoutSlash;
                    if ( aKeyWithSlash.lastIndexOf(
                             sal_Unicode('/')
                             != aKeyWithSlash.getLength() - 1 ) )
                    {
                        aKeyWithSlash += rtl::OUString( (sal_Unicode)'/' );
                        aKeyWithoutSlash = rKey;
                    }
                    else if ( rKey.getLength() )
                        aKeyWithoutSlash
                            = rKey.copy( 0, rKey.getLength() - 1 );

                    const rtl::OUString* pKeys = aKeys.getConstArray();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        const rtl::OUString& rCurrKey = pKeys[ n ];
                        if ( rCurrKey.compareTo(
                                 aKeyWithSlash,
                                 aKeyWithSlash.getLength() ) == 0
                             || rCurrKey.equals( aKeyWithoutSlash ) )
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

} // namespace ucbhelper
