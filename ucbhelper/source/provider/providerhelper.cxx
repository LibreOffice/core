/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <boost/unordered_map.hpp>
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

using namespace com::sun::star;

namespace ucbhelper_impl
{

struct equalString
{
    bool operator()(
        const OUString& rKey11, const OUString& rKey22 ) const
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

typedef boost::unordered_map
<
    OUString,
    uno::WeakReference< ucb::XContent >,
    hashString,
    equalString
>
Contents;

struct ContentProviderImplHelper_Impl
{
    uno::Reference< com::sun::star::ucb::XPropertySetRegistry >
        m_xPropertySetRegistry;
    Contents
        m_aContents;
};

} 

namespace ucbhelper {

ContentProviderImplHelper::ContentProviderImplHelper(
    const uno::Reference< uno::XComponentContext >& rxContext )
: m_pImpl( new ucbhelper_impl::ContentProviderImplHelper_Impl ),
  m_xContext( rxContext )
{
}


ContentProviderImplHelper::~ContentProviderImplHelper()
{
    delete m_pImpl;
}

XINTERFACE_IMPL_3( ContentProviderImplHelper,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   com::sun::star::ucb::XContentProvider );

XTYPEPROVIDER_IMPL_3( ContentProviderImplHelper,
                         lang::XTypeProvider,
                         lang::XServiceInfo,
                         com::sun::star::ucb::XContentProvider );


sal_Bool SAL_CALL ContentProviderImplHelper::supportsService(
                                            const OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}


sal_Int32 SAL_CALL ContentProviderImplHelper::compareContentIds(
        const uno::Reference< com::sun::star::ucb::XContentIdentifier >& Id1,
        const uno::Reference< com::sun::star::ucb::XContentIdentifier >& Id2 )
    throw( uno::RuntimeException )
{
    

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
    const uno::Reference< com::sun::star::ucb::XContentIdentifier >&
        Identifier )
{
    return queryExistingContent( Identifier->getContentIdentifier() );
}

rtl::Reference< ContentImplHelper >
ContentProviderImplHelper::queryExistingContent( const OUString& rURL )
{
    osl::MutexGuard aGuard( m_aMutex );

    cleanupRegisteredContents();

    

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

uno::Reference< com::sun::star::ucb::XPropertySetRegistry >
ContentProviderImplHelper::getAdditionalPropertySetRegistry()
{
    

    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_xPropertySetRegistry.is() )
    {
        uno::Reference< com::sun::star::ucb::XPropertySetRegistryFactory >
            xRegFac = com::sun::star::ucb::Store::create( m_xContext );

        
        m_pImpl->m_xPropertySetRegistry
            = xRegFac->createPropertySetRegistry( OUString() );

        OSL_ENSURE( m_pImpl->m_xPropertySetRegistry.is(),
                "ContentProviderImplHelper::getAdditionalPropertySet - "
                "Error opening registry!" );
    }

    return m_pImpl->m_xPropertySetRegistry;
}

uno::Reference< com::sun::star::ucb::XPersistentPropertySet >
ContentProviderImplHelper::getAdditionalPropertySet(
    const OUString& rKey, bool bCreate )
{
    
    getAdditionalPropertySetRegistry();

    if ( m_pImpl->m_xPropertySetRegistry.is() )
    {
        
        return uno::Reference< com::sun::star::ucb::XPersistentPropertySet >(
            m_pImpl->m_xPropertySetRegistry->openPropertySet(
                rKey, bCreate ) );
    }

    return uno::Reference< com::sun::star::ucb::XPersistentPropertySet >();
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
                        aOldKeyWithSlash += OUString( '/' );
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
        
        uno::Reference< com::sun::star::ucb::XPersistentPropertySet > xOldSet
            = getAdditionalPropertySet( rOldKey, false );
        if ( xOldSet.is() )
        {
            
            uno::Reference< container::XNamed > xNamed(
                xOldSet, uno::UNO_QUERY );
            if ( xNamed.is() )
            {
                
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
                        aSrcKeyWithSlash += OUString( '/' );
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
        
        uno::Reference< com::sun::star::ucb::XPersistentPropertySet >
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

        
        uno::Sequence< beans::PropertyValue > aValues
            = xOldPropAccess->getPropertyValues();
        sal_Int32 nCount = aValues.getLength();

        uno::Sequence< beans::Property > aProps
            = xPropSetInfo->getProperties();

        if ( nCount )
        {
            
            uno::Reference< com::sun::star::ucb::XPersistentPropertySet >
                xNewPropSet
                    = getAdditionalPropertySet( rTargetKey, false );
            if ( xNewPropSet.is() )
                return false;

            
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
                        aKeyWithSlash += OUString( '/' );
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
        
        getAdditionalPropertySetRegistry();

        if ( m_pImpl->m_xPropertySetRegistry.is() )
            m_pImpl->m_xPropertySetRegistry->removePropertySet( rKey );
        else
            return false;
    }
    return true;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
