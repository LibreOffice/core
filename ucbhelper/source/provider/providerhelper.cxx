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

#include <sal/config.h>

#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/ucb/Store.hpp>
#include <com/sun/star/ucb/XPropertySetRegistry.hpp>
#include <com/sun/star/ucb/XPropertySetRegistryFactory.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <ucbhelper/contenthelper.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>

#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "cppuhelper/weakref.hxx"

#include <unordered_map>

using namespace com::sun::star;

namespace ucbhelper_impl
{

typedef std::unordered_map
<
    OUString,
    uno::WeakReference< ucb::XContent >,
    OUStringHash
>
Contents;

struct ContentProviderImplHelper_Impl
{
    uno::Reference< css::ucb::XPropertySetRegistry >  m_xPropertySetRegistry;
    Contents                                          m_aContents;
};

} // namespace ucbhelper_impl

namespace ucbhelper {

ContentProviderImplHelper::ContentProviderImplHelper(
    const uno::Reference< uno::XComponentContext >& rxContext )
: m_pImpl( new ucbhelper_impl::ContentProviderImplHelper_Impl ),
  m_xContext( rxContext )
{
}

// virtual
ContentProviderImplHelper::~ContentProviderImplHelper()
{
}

// XInterface
void SAL_CALL ContentProviderImplHelper::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ContentProviderImplHelper::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ContentProviderImplHelper::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< lang::XTypeProvider* >(this)),
                                               (static_cast< lang::XServiceInfo* >(this)),
                                               (static_cast< css::ucb::XContentProvider* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

XTYPEPROVIDER_IMPL_3( ContentProviderImplHelper,
                         lang::XTypeProvider,
                         lang::XServiceInfo,
                         css::ucb::XContentProvider );

// virtual
sal_Bool SAL_CALL ContentProviderImplHelper::supportsService(
                                            const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

// virtual
sal_Int32 SAL_CALL ContentProviderImplHelper::compareContentIds(
        const uno::Reference< css::ucb::XContentIdentifier >& Id1,
        const uno::Reference< css::ucb::XContentIdentifier >& Id2 )
{
    // Simply do a string compare.

    OUString aURL1( Id1->getContentIdentifier() );
    OUString aURL2( Id2->getContentIdentifier() );

    return aURL1.compareTo( aURL2 );
}

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

void ContentProviderImplHelper::removeContent( ContentImplHelper* pContent )
{
    osl::MutexGuard aGuard( m_aMutex );

    cleanupRegisteredContents();

    const OUString aURL(
        pContent->getIdentifier()->getContentIdentifier() );

    ucbhelper_impl::Contents::iterator it = m_pImpl->m_aContents.find( aURL );

    if ( it != m_pImpl->m_aContents.end() )
        m_pImpl->m_aContents.erase( it );
}

rtl::Reference< ContentImplHelper >
ContentProviderImplHelper::queryExistingContent(
    const uno::Reference< css::ucb::XContentIdentifier >& Identifier )
{
    return queryExistingContent( Identifier->getContentIdentifier() );
}

rtl::Reference< ContentImplHelper >
ContentProviderImplHelper::queryExistingContent( const OUString& rURL )
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

void ContentProviderImplHelper::registerNewContent(
    const uno::Reference< ucb::XContent > & xContent )
{
    if ( xContent.is() )
    {
        osl::MutexGuard aGuard( m_aMutex );

        cleanupRegisteredContents();

        const OUString aURL(
            xContent->getIdentifier()->getContentIdentifier() );
        ucbhelper_impl::Contents::const_iterator it
            = m_pImpl->m_aContents.find( aURL );
        if ( it == m_pImpl->m_aContents.end() )
            m_pImpl->m_aContents[ aURL ] = xContent;
    }
}

uno::Reference< css::ucb::XPropertySetRegistry >
ContentProviderImplHelper::getAdditionalPropertySetRegistry()
{
    // Get propertyset registry.

    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_xPropertySetRegistry.is() )
    {
        uno::Reference< css::ucb::XPropertySetRegistryFactory >
            xRegFac = css::ucb::Store::create( m_xContext );

        // Open/create a registry.
        m_pImpl->m_xPropertySetRegistry
            = xRegFac->createPropertySetRegistry( OUString() );

        OSL_ENSURE( m_pImpl->m_xPropertySetRegistry.is(),
                "ContentProviderImplHelper::getAdditionalPropertySet - "
                "Error opening registry!" );
    }

    return m_pImpl->m_xPropertySetRegistry;
}

uno::Reference< css::ucb::XPersistentPropertySet >
ContentProviderImplHelper::getAdditionalPropertySet(
    const OUString& rKey, bool bCreate )
{
    // Get propertyset registry.
    getAdditionalPropertySetRegistry();

    if ( m_pImpl->m_xPropertySetRegistry.is() )
    {
        // Open/create persistent property set.
        return uno::Reference< css::ucb::XPersistentPropertySet >(
            m_pImpl->m_xPropertySetRegistry->openPropertySet(
                rKey, bCreate ) );
    }

    return uno::Reference< css::ucb::XPersistentPropertySet >();
}

bool ContentProviderImplHelper::renameAdditionalPropertySet(
    const OUString& rOldKey,
    const OUString& rNewKey,
    bool bRecursive )
{
    if ( rOldKey == rNewKey )
        return true;

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
                uno::Sequence< OUString > aKeys
                    = xNameAccess->getElementNames();
                sal_Int32 nCount = aKeys.getLength();
                if ( nCount > 0 )
                {
                    OUString aOldKeyWithSlash = rOldKey;
                    OUString aOldKeyWithoutSlash;
                    if ( !aOldKeyWithSlash.endsWith("/") )
                    {
                        aOldKeyWithSlash += "/";
                        aOldKeyWithoutSlash = rOldKey;
                    }
                    else if ( !rOldKey.isEmpty() )
                        aOldKeyWithoutSlash
                            = rOldKey.copy( 0, rOldKey.getLength() - 1 );

                    const OUString* pKeys = aKeys.getConstArray();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        const OUString& rKey = pKeys[ n ];
                        if ( rKey.compareTo(
                                 aOldKeyWithSlash,
                                 aOldKeyWithSlash.getLength() ) == 0
                             || rKey.equals( aOldKeyWithoutSlash ) )
                        {
                            OUString aNewKey
                                = rKey.replaceAt(
                                    0, rOldKey.getLength(), rNewKey );
                            if ( !renameAdditionalPropertySet(
                                    rKey, aNewKey, false ) )
                                return false;
                        }
                    }
                }
            }
            else
                return false;
        }
        else
            return false;
    }
    else
    {
        // Get old property set, if exists.
        uno::Reference< css::ucb::XPersistentPropertySet > xOldSet
            = getAdditionalPropertySet( rOldKey, false );
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
                return false;
        }
    }
    return true;
}

bool ContentProviderImplHelper::copyAdditionalPropertySet(
    const OUString& rSourceKey,
    const OUString& rTargetKey,
    bool bRecursive )
{
    if ( rSourceKey == rTargetKey )
        return true;

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
                uno::Sequence< OUString > aKeys
                    = xNameAccess->getElementNames();
                sal_Int32 nCount = aKeys.getLength();
                if ( nCount > 0 )
                {
                    OUString aSrcKeyWithSlash = rSourceKey;
                    OUString aSrcKeyWithoutSlash;
                    if ( !aSrcKeyWithSlash.endsWith("/") )
                    {
                        aSrcKeyWithSlash += "/";
                        aSrcKeyWithoutSlash = rSourceKey;
                    }
                    else if ( !rSourceKey.isEmpty() )
                        aSrcKeyWithoutSlash = rSourceKey.copy(
                            0, rSourceKey.getLength() - 1 );

                    const OUString* pKeys = aKeys.getConstArray();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        const OUString& rKey = pKeys[ n ];
                        if ( rKey.compareTo(
                                 aSrcKeyWithSlash,
                                 aSrcKeyWithSlash.getLength() ) == 0
                             || rKey.equals( aSrcKeyWithoutSlash ) )
                        {
                            OUString aNewKey
                                = rKey.replaceAt(
                                    0, rSourceKey.getLength(), rTargetKey );
                            if ( !copyAdditionalPropertySet(
                                    rKey, aNewKey, false ) )
                                return false;
                        }
                    }
                }
            }
            else
                return false;
        }
        else
            return false;
    }
    else
    {
        // Get old property set, if exists.
        uno::Reference< css::ucb::XPersistentPropertySet >
            xOldPropSet = getAdditionalPropertySet( rSourceKey, false );
        if ( !xOldPropSet.is() )
            return false;

        uno::Reference< beans::XPropertySetInfo > xPropSetInfo
            = xOldPropSet->getPropertySetInfo();
        if ( !xPropSetInfo.is() )
            return false;

        uno::Reference< beans::XPropertyAccess > xOldPropAccess(
            xOldPropSet, uno::UNO_QUERY );
        if ( !xOldPropAccess.is() )
            return false;

        // Obtain all values from old set.
        uno::Sequence< beans::PropertyValue > aValues
            = xOldPropAccess->getPropertyValues();
        sal_Int32 nCount = aValues.getLength();

        uno::Sequence< beans::Property > aProps
            = xPropSetInfo->getProperties();

        if ( nCount )
        {
            // Fail, if property set with new key already exists.
            uno::Reference< css::ucb::XPersistentPropertySet >
                xNewPropSet
                    = getAdditionalPropertySet( rTargetKey, false );
            if ( xNewPropSet.is() )
                return false;

            // Create new, empty set.
            xNewPropSet = getAdditionalPropertySet( rTargetKey, true );
            if ( !xNewPropSet.is() )
                return false;

            uno::Reference< beans::XPropertyContainer > xNewPropContainer(
                xNewPropSet, uno::UNO_QUERY );
            if ( !xNewPropContainer.is() )
                return false;

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
    return true;
}

bool ContentProviderImplHelper::removeAdditionalPropertySet(
    const OUString& rKey, bool bRecursive )
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
                uno::Sequence< OUString > aKeys
                    = xNameAccess->getElementNames();
                sal_Int32 nCount = aKeys.getLength();
                if ( nCount > 0 )
                {
                    OUString aKeyWithSlash = rKey;
                    OUString aKeyWithoutSlash;
                    if ( !aKeyWithSlash.endsWith("/") )
                    {
                        aKeyWithSlash += "/";
                        aKeyWithoutSlash = rKey;
                    }
                    else if ( !rKey.isEmpty() )
                        aKeyWithoutSlash
                            = rKey.copy( 0, rKey.getLength() - 1 );

                    const OUString* pKeys = aKeys.getConstArray();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        const OUString& rCurrKey = pKeys[ n ];
                        if ( rCurrKey.compareTo(
                                 aKeyWithSlash,
                                 aKeyWithSlash.getLength() ) == 0
                             || rCurrKey.equals( aKeyWithoutSlash ) )
                        {
                            if ( !removeAdditionalPropertySet(
                                     rCurrKey, false ) )
                                return false;
                        }
                    }
                }
            }
            else
                return false;
        }
        else
            return false;
    }
    else
    {
        // Get propertyset registry.
        getAdditionalPropertySetRegistry();

        if ( m_pImpl->m_xPropertySetRegistry.is() )
            m_pImpl->m_xPropertySetRegistry->removePropertySet( rKey );
        else
            return false;
    }
    return true;
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
