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

#include <memory>
#include "stringresource.hxx"
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/TextInputStream.hpp>
#include <com/sun/star/io/TextOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/resource/MissingResourceException.hpp>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include <osl/diagnose.h>
#include <rtl/tencinfo.h>
#include <rtl/ustrbuf.hxx>
#include <tools/urlobj.hxx>
#include <i18nlangtag/languagetag.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::container;


namespace stringresource
{


// mutex


::osl::Mutex& getMutex()
{
    static ::osl::Mutex s_aMutex;

    return s_aMutex;
}


// StringResourceImpl

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
scripting_StringResourcePersistenceImpl_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new StringResourcePersistenceImpl(context));
}


StringResourceImpl::StringResourceImpl( const Reference< XComponentContext >& rxContext )
    : m_xContext( rxContext )
    , m_pCurrentLocaleItem( nullptr )
    , m_pDefaultLocaleItem( nullptr )
    , m_bDefaultModified( false )
    , m_aListenerContainer( getMutex() )
    , m_bModified( false )
    , m_bReadOnly( false )
    , m_nNextUniqueNumericId( UNIQUE_NUMBER_NEEDS_INITIALISATION )
{
}


StringResourceImpl::~StringResourceImpl()
{
}


// XServiceInfo

OUString StringResourceImpl::getImplementationName(  )
{
    return "com.sun.star.comp.scripting.StringResource";
}

sal_Bool StringResourceImpl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > StringResourceImpl::getSupportedServiceNames(  )
{
    return { "com.sun.star.resource.StringResource" };
}


// XModifyBroadcaster

void StringResourceImpl::addModifyListener( const Reference< XModifyListener >& aListener )
{
    if( !aListener.is() )
        throw RuntimeException();

    ::osl::MutexGuard aGuard( getMutex() );
    m_aListenerContainer.addInterface( Reference<XInterface>( aListener, UNO_QUERY ) );
}

void StringResourceImpl::removeModifyListener( const Reference< XModifyListener >& aListener )
{
    if( !aListener.is() )
        throw RuntimeException();

    ::osl::MutexGuard aGuard( getMutex() );
    m_aListenerContainer.removeInterface( Reference<XInterface>( aListener, UNO_QUERY ) );
}


// XStringResourceResolver

OUString StringResourceImpl::implResolveString
    ( const OUString& ResourceID, LocaleItem* pLocaleItem )
{
    OUString aRetStr;
    bool bSuccess = false;
    if( pLocaleItem != nullptr && loadLocale( pLocaleItem ) )
    {
        IdToStringMap::iterator it = pLocaleItem->m_aIdToStringMap.find( ResourceID );
        if( it != pLocaleItem->m_aIdToStringMap.end() )
        {
            aRetStr = (*it).second;
            bSuccess = true;
        }
    }
    if( !bSuccess )
    {
        throw css::resource::MissingResourceException( "StringResourceImpl: No entry for ResourceID: " + ResourceID );
    }
    return aRetStr;
}

OUString StringResourceImpl::resolveString( const OUString& ResourceID )
{
    ::osl::MutexGuard aGuard( getMutex() );
    return implResolveString( ResourceID, m_pCurrentLocaleItem );
}

OUString StringResourceImpl::resolveStringForLocale( const OUString& ResourceID, const Locale& locale )
{
    ::osl::MutexGuard aGuard( getMutex() );
    LocaleItem* pLocaleItem = getItemForLocale( locale, false );
    return implResolveString( ResourceID, pLocaleItem );
}

bool StringResourceImpl::implHasEntryForId( const OUString& ResourceID, LocaleItem* pLocaleItem )
{
    bool bSuccess = false;
    if( pLocaleItem != nullptr && loadLocale( pLocaleItem ) )
    {
        IdToStringMap::iterator it = pLocaleItem->m_aIdToStringMap.find( ResourceID );
        if( it != pLocaleItem->m_aIdToStringMap.end() )
            bSuccess = true;
    }
    return bSuccess;
}

sal_Bool StringResourceImpl::hasEntryForId( const OUString& ResourceID )
{
    ::osl::MutexGuard aGuard( getMutex() );
    return implHasEntryForId( ResourceID, m_pCurrentLocaleItem );
}

sal_Bool StringResourceImpl::hasEntryForIdAndLocale( const OUString& ResourceID,
    const Locale& locale )
{
    ::osl::MutexGuard aGuard( getMutex() );
    LocaleItem* pLocaleItem = getItemForLocale( locale, false );
    return implHasEntryForId( ResourceID, pLocaleItem );
}

Sequence< OUString > StringResourceImpl::implGetResourceIDs( LocaleItem* pLocaleItem )
{
    Sequence< OUString > aIDSeq( 0 );
    if( pLocaleItem && loadLocale( pLocaleItem ) )
    {
        const IdToStringMap& rHashMap = pLocaleItem->m_aIdToStringMap;
        sal_Int32 nResourceIDCount = rHashMap.size();
        aIDSeq.realloc( nResourceIDCount );
        OUString* pStrings = aIDSeq.getArray();

        int iTarget = 0;
        for( const auto& rEntry : rHashMap )
        {
            OUString aStr = rEntry.first;
            pStrings[iTarget] = aStr;
            iTarget++;
        }
    }
    return aIDSeq;
}

Sequence< OUString > StringResourceImpl::getResourceIDsForLocale
    ( const Locale& locale )
{
    ::osl::MutexGuard aGuard( getMutex() );
    LocaleItem* pLocaleItem = getItemForLocale( locale, false );
    return implGetResourceIDs( pLocaleItem );
}

Sequence< OUString > StringResourceImpl::getResourceIDs(  )
{
    ::osl::MutexGuard aGuard( getMutex() );
    return implGetResourceIDs( m_pCurrentLocaleItem );
}

Locale StringResourceImpl::getCurrentLocale()
{
    ::osl::MutexGuard aGuard( getMutex() );

    Locale aRetLocale;
    if( m_pCurrentLocaleItem != nullptr )
        aRetLocale = m_pCurrentLocaleItem->m_locale;
    return aRetLocale;
}

Locale StringResourceImpl::getDefaultLocale(  )
{
    ::osl::MutexGuard aGuard( getMutex() );

    Locale aRetLocale;
    if( m_pDefaultLocaleItem != nullptr )
        aRetLocale = m_pDefaultLocaleItem->m_locale;
    return aRetLocale;
}

Sequence< Locale > StringResourceImpl::getLocales(  )
{
    ::osl::MutexGuard aGuard( getMutex() );

    sal_Int32 nSize = m_aLocaleItemVector.size();
    Sequence< Locale > aLocalSeq( nSize );
    Locale* pLocales = aLocalSeq.getArray();
    int iTarget = 0;
    for( const auto& pLocaleItem : m_aLocaleItemVector )
    {
        pLocales[iTarget] = pLocaleItem->m_locale;
        iTarget++;
    }
    return aLocalSeq;
}


// XStringResourceManager

void StringResourceImpl::implCheckReadOnly( const char* pExceptionMsg )
{
    if( m_bReadOnly )
    {
        OUString errorMsg = OUString::createFromAscii( pExceptionMsg );
        throw NoSupportException( errorMsg );
    }
}

sal_Bool StringResourceImpl::isReadOnly()
{
    return m_bReadOnly;
}

void StringResourceImpl::implSetCurrentLocale( const Locale& locale,
    bool FindClosestMatch, bool bUseDefaultIfNoMatch )
{
    ::osl::MutexGuard aGuard( getMutex() );

    LocaleItem* pLocaleItem = nullptr;
    if( FindClosestMatch )
        pLocaleItem = getClosestMatchItemForLocale( locale );
    else
        pLocaleItem = getItemForLocale( locale, true );

    if( pLocaleItem == nullptr && bUseDefaultIfNoMatch )
        pLocaleItem = m_pDefaultLocaleItem;

    if( pLocaleItem != nullptr )
    {
        (void)loadLocale( pLocaleItem );
        m_pCurrentLocaleItem = pLocaleItem;

        // Only notify without modifying
        implNotifyListeners();
    }
}

void StringResourceImpl::setCurrentLocale( const Locale& locale, sal_Bool FindClosestMatch )
{
    implSetCurrentLocale( locale, FindClosestMatch, false/*bUseDefaultIfNoMatch*/ );
}

void StringResourceImpl::setDefaultLocale( const Locale& locale )
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::setDefaultLocale(): Read only" );

    LocaleItem* pLocaleItem = getItemForLocale( locale, true );
    if( pLocaleItem && pLocaleItem != m_pDefaultLocaleItem )
    {
        if( m_pDefaultLocaleItem )
        {
            m_aChangedDefaultLocaleVector.push_back(
                std::make_unique<LocaleItem>( m_pDefaultLocaleItem->m_locale ) );
        }

        m_pDefaultLocaleItem = pLocaleItem;
        m_bDefaultModified = true;
        implModified();
    }
}

void StringResourceImpl::implSetString( const OUString& ResourceID,
    const OUString& Str, LocaleItem* pLocaleItem )
{
    if( !(pLocaleItem != nullptr && loadLocale( pLocaleItem )) )
        return;

    IdToStringMap& rHashMap = pLocaleItem->m_aIdToStringMap;

    IdToStringMap::iterator it = rHashMap.find( ResourceID );
    bool bNew = ( it == rHashMap.end() );
    if( bNew )
    {
        IdToIndexMap& rIndexMap = pLocaleItem->m_aIdToIndexMap;
        rIndexMap[ ResourceID ] = pLocaleItem->m_nNextIndex++;
        implScanIdForNumber( ResourceID );
    }
    rHashMap[ ResourceID ] = Str;
    pLocaleItem->m_bModified = true;
    implModified();
}

void StringResourceImpl::setString( const OUString& ResourceID, const OUString& Str )
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::setString(): Read only" );
    implSetString( ResourceID, Str, m_pCurrentLocaleItem );
}

void StringResourceImpl::setStringForLocale
    ( const OUString& ResourceID, const OUString& Str, const Locale& locale )
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::setStringForLocale(): Read only" );
    LocaleItem* pLocaleItem = getItemForLocale( locale, false );
    implSetString( ResourceID, Str, pLocaleItem );
}

void StringResourceImpl::implRemoveId( const OUString& ResourceID, LocaleItem* pLocaleItem )
{
    if( pLocaleItem != nullptr && loadLocale( pLocaleItem ) )
    {
        IdToStringMap& rHashMap = pLocaleItem->m_aIdToStringMap;
        IdToStringMap::iterator it = rHashMap.find( ResourceID );
        if( it == rHashMap.end() )
        {
            throw css::resource::MissingResourceException( "StringResourceImpl: No entries for ResourceID: " + ResourceID );
        }
        rHashMap.erase( it );
        pLocaleItem->m_bModified = true;
        implModified();
    }
}

void StringResourceImpl::removeId( const OUString& ResourceID )
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::removeId(): Read only" );
    implRemoveId( ResourceID, m_pCurrentLocaleItem );
}

void StringResourceImpl::removeIdForLocale( const OUString& ResourceID, const Locale& locale )
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::removeIdForLocale(): Read only" );
    LocaleItem* pLocaleItem = getItemForLocale( locale, false );
    implRemoveId( ResourceID, pLocaleItem );
}

void StringResourceImpl::newLocale( const Locale& locale )
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::newLocale(): Read only" );

    if( getItemForLocale( locale, false ) != nullptr )
    {
        throw ElementExistException( "StringResourceImpl: locale already exists" );
    }

    // TODO?: Check if locale is valid? How?
    //if (!bValid)
    //{
    //    OUString errorMsg("StringResourceImpl: Invalid locale");
    //    throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 0 );
    //}

    LocaleItem* pLocaleItem = new LocaleItem( locale );
    m_aLocaleItemVector.emplace_back( pLocaleItem );
    pLocaleItem->m_bModified = true;

    // Copy strings from default locale
    LocaleItem* pCopyFromItem = m_pDefaultLocaleItem;
    if( pCopyFromItem == nullptr )
        pCopyFromItem = m_pCurrentLocaleItem;
    if( pCopyFromItem != nullptr && loadLocale( pCopyFromItem ) )
    {
        const IdToStringMap& rSourceMap = pCopyFromItem->m_aIdToStringMap;
        IdToStringMap& rTargetMap = pLocaleItem->m_aIdToStringMap;
        for( const auto& rEntry : rSourceMap )
        {
            OUString aId  = rEntry.first;
            OUString aStr = rEntry.second;
            rTargetMap[ aId ] = aStr;
        }

        const IdToIndexMap& rSourceIndexMap = pCopyFromItem->m_aIdToIndexMap;
        IdToIndexMap& rTargetIndexMap = pLocaleItem->m_aIdToIndexMap;
        for( const auto& rIndex : rSourceIndexMap )
        {
            OUString aId = rIndex.first;
            sal_Int32 nIndex = rIndex.second;
            rTargetIndexMap[ aId ] = nIndex;
        }
        pLocaleItem->m_nNextIndex = pCopyFromItem->m_nNextIndex;
    }

    if( m_pCurrentLocaleItem == nullptr )
        m_pCurrentLocaleItem = pLocaleItem;

    if( m_pDefaultLocaleItem == nullptr )
    {
        m_pDefaultLocaleItem = pLocaleItem;
        m_bDefaultModified = true;
    }

    implModified();
}

void StringResourceImpl::removeLocale( const Locale& locale )
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::removeLocale(): Read only" );

    LocaleItem* pRemoveItem = getItemForLocale( locale, true );
    if( !pRemoveItem )
        return;

    // Last locale?
    sal_Int32 nLocaleCount = m_aLocaleItemVector.size();
    if( nLocaleCount > 1 )
    {
        if( m_pCurrentLocaleItem == pRemoveItem ||
            m_pDefaultLocaleItem  == pRemoveItem )
        {
            LocaleItem* pFallbackItem = nullptr;
            for( const auto& pLocaleItem : m_aLocaleItemVector )
            {
                if( pLocaleItem.get() != pRemoveItem )
                {
                    pFallbackItem = pLocaleItem.get();
                    break;
                }
            }
            if( m_pCurrentLocaleItem == pRemoveItem )
            {
                setCurrentLocale( pFallbackItem->m_locale, false/*FindClosestMatch*/ );
            }
            if( m_pDefaultLocaleItem == pRemoveItem )
            {
                setDefaultLocale( pFallbackItem->m_locale );
            }
        }
    }
    auto it = std::find_if(m_aLocaleItemVector.begin(), m_aLocaleItemVector.end(),
        [&pRemoveItem](const std::unique_ptr<LocaleItem>& rxItem) { return rxItem.get() == pRemoveItem; });
    if (it == m_aLocaleItemVector.end())
        return;

    // Remember locale item to delete file while storing
    m_aDeletedLocaleItemVector.push_back( std::move(*it) );

    // Last locale?
    if( nLocaleCount == 1 )
    {
        m_nNextUniqueNumericId = 0;
        if( m_pDefaultLocaleItem )
        {
            m_aChangedDefaultLocaleVector.push_back(
                    std::make_unique<LocaleItem>( m_pDefaultLocaleItem->m_locale ) );
        }
        m_pCurrentLocaleItem = nullptr;
        m_pDefaultLocaleItem = nullptr;
    }

    m_aLocaleItemVector.erase( it );

    implModified();
}

void StringResourceImpl::implScanIdForNumber( const OUString& ResourceID )
{
    const sal_Unicode* pSrc = ResourceID.getStr();
    sal_Int32 nLen = ResourceID.getLength();

    sal_Int32 nNumber = 0;
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
    {
        sal_Unicode c = pSrc[i];
        if( c >= '0' && c <= '9' )
        {
            sal_uInt16 nDigitVal = c - '0';
            nNumber = 10*nNumber + nDigitVal;
        }
        else
            break;
    }

    if( m_nNextUniqueNumericId < nNumber + 1 )
        m_nNextUniqueNumericId = nNumber + 1;
}

sal_Int32 StringResourceImpl::getUniqueNumericId(  )
{
    if( m_nNextUniqueNumericId == UNIQUE_NUMBER_NEEDS_INITIALISATION )
    {
        implLoadAllLocales();
        m_nNextUniqueNumericId = 0;
    }

    if( m_nNextUniqueNumericId < UNIQUE_NUMBER_NEEDS_INITIALISATION )
    {
        throw NoSupportException( "getUniqueNumericId: Extended sal_Int32 range" );
    }
    return m_nNextUniqueNumericId;
}


// Private helper methods

LocaleItem* StringResourceImpl::getItemForLocale
    ( const Locale& locale, bool bException )
{
    LocaleItem* pRetItem = nullptr;

    // Search for locale
    for( auto& pLocaleItem : m_aLocaleItemVector )
    {
        if( pLocaleItem )
        {
            Locale& cmp_locale = pLocaleItem->m_locale;
            if( cmp_locale.Language == locale.Language &&
                cmp_locale.Country  == locale.Country &&
                cmp_locale.Variant  == locale.Variant )
            {
                pRetItem = pLocaleItem.get();
                break;
            }
        }
    }

    if( pRetItem == nullptr && bException )
    {
        throw IllegalArgumentException( "StringResourceImpl: Invalid locale", Reference< XInterface >(), 0 );
    }
    return pRetItem;
}

// Returns the LocaleItem for a given locale, if it exists, otherwise NULL.
// This method performs a closest match search, at least the language must match.
LocaleItem* StringResourceImpl::getClosestMatchItemForLocale( const Locale& locale )
{
    LocaleItem* pRetItem = nullptr;

    ::std::vector< Locale > aLocales( m_aLocaleItemVector.size());
    size_t i = 0;
    for( const auto& pLocaleItem : m_aLocaleItemVector )
    {
        aLocales[i] = (pLocaleItem ? pLocaleItem->m_locale : Locale());
        ++i;
    }
    ::std::vector< Locale >::const_iterator iFound( LanguageTag::getMatchingFallback( aLocales, locale));
    if (iFound != aLocales.end())
        pRetItem = (m_aLocaleItemVector.begin() + (iFound - aLocales.begin()))->get();

    return pRetItem;
}

void StringResourceImpl::implModified()
{
    m_bModified = true;
    implNotifyListeners();
}

void StringResourceImpl::implNotifyListeners()
{
    EventObject aEvent;
    aEvent.Source = static_cast< XInterface* >( static_cast<OWeakObject*>(this) );

    ::comphelper::OInterfaceIteratorHelper2 it( m_aListenerContainer );
    while( it.hasMoreElements() )
    {
        Reference< XInterface > xIface = it.next();
        Reference< XModifyListener > xListener( xIface, UNO_QUERY );
        try
        {
            xListener->modified( aEvent );
        }
        catch(RuntimeException&)
        {
            it.remove();
        }
    }
}


// Loading

bool StringResourceImpl::loadLocale( LocaleItem* )
{
    // Base implementation has nothing to load
    return true;
}

void StringResourceImpl::implLoadAllLocales()
{
    // Base implementation has nothing to load
}


// StringResourcePersistenceImpl


StringResourcePersistenceImpl::StringResourcePersistenceImpl( const Reference< XComponentContext >& rxContext )
    : StringResourcePersistenceImpl_BASE( rxContext )
{
}


StringResourcePersistenceImpl::~StringResourcePersistenceImpl()
{
}


// XServiceInfo


OUString StringResourcePersistenceImpl::getImplementationName(  )
{
    return "com.sun.star.comp.scripting.StringResource";
}


sal_Bool StringResourcePersistenceImpl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}


Sequence< OUString > StringResourcePersistenceImpl::getSupportedServiceNames(  )
{
    return StringResourceImpl::getSupportedServiceNames();
}


// XInitialization base functionality for derived classes


const char aNameBaseDefaultStr[] = "strings";

void StringResourcePersistenceImpl::implInitializeCommonParameters
    ( const Sequence< Any >& aArguments )
{
    bool bReadOnlyOk = (aArguments[1] >>= m_bReadOnly);
    if( !bReadOnlyOk )
    {
        throw IllegalArgumentException( "XInitialization::initialize: Expected ReadOnly flag", Reference< XInterface >(), 1 );
    }

    css::lang::Locale aCurrentLocale;
    bool bLocaleOk = (aArguments[2] >>= aCurrentLocale);
    if( !bLocaleOk )
    {
        throw IllegalArgumentException( "XInitialization::initialize: Expected Locale", Reference< XInterface >(), 2 );
    }

    bool bNameBaseOk = (aArguments[3] >>= m_aNameBase);
    if( !bNameBaseOk )
    {
        throw IllegalArgumentException( "XInitialization::initialize: Expected NameBase string", Reference< XInterface >(), 3 );
    }
    if( m_aNameBase.isEmpty() )
        m_aNameBase = aNameBaseDefaultStr;

    bool bCommentOk = (aArguments[4] >>= m_aComment);
    if( !bCommentOk )
    {
        throw IllegalArgumentException( "XInitialization::initialize: Expected Comment string", Reference< XInterface >(), 4 );
    }

    implScanLocales();

    implSetCurrentLocale( aCurrentLocale, true/*FindClosestMatch*/, true/*bUseDefaultIfNoMatch*/ );
}


// Forwarding calls to base class

// XModifyBroadcaster
void StringResourcePersistenceImpl::addModifyListener( const Reference< XModifyListener >& aListener )
{
    StringResourceImpl::addModifyListener( aListener );
}
void StringResourcePersistenceImpl::removeModifyListener( const Reference< XModifyListener >& aListener )
{
    StringResourceImpl::removeModifyListener( aListener );
}

// XStringResourceResolver
OUString StringResourcePersistenceImpl::resolveString( const OUString& ResourceID )
{
    return StringResourceImpl::resolveString( ResourceID ) ;
}
OUString StringResourcePersistenceImpl::resolveStringForLocale( const OUString& ResourceID, const Locale& locale )
{
    return StringResourceImpl::resolveStringForLocale( ResourceID, locale );
}
sal_Bool StringResourcePersistenceImpl::hasEntryForId( const OUString& ResourceID )
{
    return StringResourceImpl::hasEntryForId( ResourceID ) ;
}
sal_Bool StringResourcePersistenceImpl::hasEntryForIdAndLocale( const OUString& ResourceID,
    const Locale& locale )
{
    return StringResourceImpl::hasEntryForIdAndLocale( ResourceID, locale );
}
Locale StringResourcePersistenceImpl::getCurrentLocale()
{
    return StringResourceImpl::getCurrentLocale();
}
Locale StringResourcePersistenceImpl::getDefaultLocale(  )
{
    return StringResourceImpl::getDefaultLocale();
}
Sequence< Locale > StringResourcePersistenceImpl::getLocales(  )
{
    return StringResourceImpl::getLocales();
}

// XStringResourceManager
sal_Bool StringResourcePersistenceImpl::isReadOnly()
{
    return StringResourceImpl::isReadOnly();
}
void StringResourcePersistenceImpl::setCurrentLocale( const Locale& locale, sal_Bool FindClosestMatch )
{
    StringResourceImpl::setCurrentLocale( locale, FindClosestMatch );
}
void StringResourcePersistenceImpl::setDefaultLocale( const Locale& locale )
{
    StringResourceImpl::setDefaultLocale( locale );
}
Sequence< OUString > StringResourcePersistenceImpl::getResourceIDs(  )
{
    return StringResourceImpl::getResourceIDs();
}
void StringResourcePersistenceImpl::setString( const OUString& ResourceID, const OUString& Str )
{
    StringResourceImpl::setString( ResourceID, Str );
}
void StringResourcePersistenceImpl::setStringForLocale
    ( const OUString& ResourceID, const OUString& Str, const Locale& locale )
{
    StringResourceImpl::setStringForLocale( ResourceID, Str, locale );
}
Sequence< OUString > StringResourcePersistenceImpl::getResourceIDsForLocale
    ( const Locale& locale )
{
    return StringResourceImpl::getResourceIDsForLocale( locale );
}
void StringResourcePersistenceImpl::removeId( const OUString& ResourceID )
{
    StringResourceImpl::removeId( ResourceID );
}
void StringResourcePersistenceImpl::removeIdForLocale( const OUString& ResourceID, const Locale& locale )
{
    StringResourceImpl::removeIdForLocale( ResourceID, locale );
}
void StringResourcePersistenceImpl::newLocale( const Locale& locale )
{
    StringResourceImpl::newLocale( locale );
}
void StringResourcePersistenceImpl::removeLocale( const Locale& locale )
{
    StringResourceImpl::removeLocale( locale );
}
sal_Int32 StringResourcePersistenceImpl::getUniqueNumericId(  )
{
    return StringResourceImpl::getUniqueNumericId();
}


// XStringResourcePersistence

void StringResourcePersistenceImpl::store()
{
}

sal_Bool StringResourcePersistenceImpl::isModified(  )
{
    ::osl::MutexGuard aGuard( getMutex() );

    return m_bModified;
}

void StringResourcePersistenceImpl::setComment( const OUString& Comment )
{
    m_aComment = Comment;
}

void StringResourcePersistenceImpl::storeToStorage( const Reference< XStorage >& Storage,
    const OUString& NameBase, const OUString& Comment )
{
    ::osl::MutexGuard aGuard( getMutex() );

    implStoreAtStorage( NameBase, Comment, Storage, false/*bUsedForStore*/, true/*bStoreAll*/ );
}

void StringResourcePersistenceImpl::implStoreAtStorage
(
    const OUString& aNameBase,
    const OUString& aComment,
    const Reference< css::embed::XStorage >& Storage,
    bool bUsedForStore,
    bool bStoreAll
)
{
    // Delete files for deleted locales
    if( bUsedForStore )
    {
        for( auto& pLocaleItem : m_aDeletedLocaleItemVector )
        {
            if( pLocaleItem )
            {
                OUString aStreamName = implGetFileNameForLocaleItem( pLocaleItem.get(), m_aNameBase ) + ".properties";

                try
                {
                    Storage->removeElement( aStreamName );
                }
                catch( Exception& )
                {}

                pLocaleItem.reset();
            }
        }
        m_aDeletedLocaleItemVector.clear();
    }

    for( auto& pLocaleItem : m_aLocaleItemVector )
    {
        if( pLocaleItem != nullptr && (bStoreAll || pLocaleItem->m_bModified) &&
            loadLocale( pLocaleItem.get() ) )
        {
            OUString aStreamName = implGetFileNameForLocaleItem( pLocaleItem.get(), aNameBase ) + ".properties";

            Reference< io::XStream > xElementStream =
                    Storage->openStreamElement( aStreamName, ElementModes::READWRITE );

            uno::Reference< beans::XPropertySet > xProps( xElementStream, uno::UNO_QUERY );
            OSL_ENSURE( xProps.is(), "The StorageStream must implement XPropertySet interface!" );
            if ( xProps.is() )
            {
                OUString aPropName("MediaType");
                xProps->setPropertyValue( aPropName, uno::makeAny( OUString("text/plain") ) );

                aPropName = "UseCommonStoragePasswordEncryption";
                xProps->setPropertyValue( aPropName, uno::makeAny( true ) );
            }

            Reference< io::XOutputStream > xOutputStream = xElementStream->getOutputStream();
            if( xOutputStream.is() )
                implWritePropertiesFile( pLocaleItem.get(), xOutputStream, aComment );
            xOutputStream->closeOutput();

            if( bUsedForStore )
                pLocaleItem->m_bModified = false;
        }
    }

    // Delete files for changed defaults
    if( bUsedForStore )
    {
        for( auto& pLocaleItem : m_aChangedDefaultLocaleVector )
        {
            OUString aStreamName = implGetFileNameForLocaleItem( pLocaleItem.get(), m_aNameBase ) + ".default";

            try
            {
                Storage->removeElement( aStreamName );
            }
            catch( Exception& )
            {}

            pLocaleItem.reset();
        }
        m_aChangedDefaultLocaleVector.clear();
    }

    // Default locale
    if( !(m_pDefaultLocaleItem != nullptr && (bStoreAll || m_bDefaultModified)) )
        return;

    OUString aStreamName = implGetFileNameForLocaleItem( m_pDefaultLocaleItem, aNameBase ) + ".default";

    Reference< io::XStream > xElementStream =
            Storage->openStreamElement( aStreamName, ElementModes::READWRITE );

    // Only create stream without content
    Reference< io::XOutputStream > xOutputStream = xElementStream->getOutputStream();
    xOutputStream->closeOutput();

    if( bUsedForStore )
        m_bDefaultModified = false;
}

void StringResourcePersistenceImpl::storeToURL( const OUString& URL,
    const OUString& NameBase, const OUString& Comment,
    const Reference< css::task::XInteractionHandler >& Handler )
{
    ::osl::MutexGuard aGuard( getMutex() );

    Reference< ucb::XSimpleFileAccess3 > xFileAccess = ucb::SimpleFileAccess::create(m_xContext);
    if( xFileAccess.is() && Handler.is() )
        xFileAccess->setInteractionHandler( Handler );

    implStoreAtLocation( URL, NameBase, Comment, xFileAccess, false/*bUsedForStore*/, true/*bStoreAll*/ );
}

void StringResourcePersistenceImpl::implKillRemovedLocaleFiles
(
    const OUString& Location,
    const OUString& aNameBase,
    const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xFileAccess
)
{
    // Delete files for deleted locales
    for( auto& pLocaleItem : m_aDeletedLocaleItemVector )
    {
        if( pLocaleItem )
        {
            OUString aCompleteFileName =
                implGetPathForLocaleItem( pLocaleItem.get(), aNameBase, Location );
            if( xFileAccess->exists( aCompleteFileName ) )
                xFileAccess->kill( aCompleteFileName );

            pLocaleItem.reset();
        }
    }
    m_aDeletedLocaleItemVector.clear();
}

void StringResourcePersistenceImpl::implKillChangedDefaultFiles
(
    const OUString& Location,
    const OUString& aNameBase,
    const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xFileAccess
)
{
    // Delete files for changed defaults
    for( auto& pLocaleItem : m_aChangedDefaultLocaleVector )
    {
        OUString aCompleteFileName =
            implGetPathForLocaleItem( pLocaleItem.get(), aNameBase, Location, true );
        if( xFileAccess->exists( aCompleteFileName ) )
            xFileAccess->kill( aCompleteFileName );
        pLocaleItem.reset();
    }
    m_aChangedDefaultLocaleVector.clear();
}

void StringResourcePersistenceImpl::implStoreAtLocation
(
    const OUString& Location,
    const OUString& aNameBase,
    const OUString& aComment,
    const Reference< ucb::XSimpleFileAccess3 >& xFileAccess,
    bool bUsedForStore,
    bool bStoreAll,
    bool bKillAll
)
{
    // Delete files for deleted locales
    if( bUsedForStore || bKillAll )
        implKillRemovedLocaleFiles( Location, aNameBase, xFileAccess );

    for( auto& pLocaleItem : m_aLocaleItemVector )
    {
        if( pLocaleItem != nullptr && (bStoreAll || bKillAll || pLocaleItem->m_bModified) &&
            loadLocale( pLocaleItem.get() ) )
        {
            OUString aCompleteFileName =
                implGetPathForLocaleItem( pLocaleItem.get(), aNameBase, Location );
            if( xFileAccess->exists( aCompleteFileName ) )
                xFileAccess->kill( aCompleteFileName );

            if( !bKillAll )
            {
                // Create Output stream
                Reference< io::XOutputStream > xOutputStream = xFileAccess->openFileWrite( aCompleteFileName );
                if( xOutputStream.is() )
                {
                    implWritePropertiesFile( pLocaleItem.get(), xOutputStream, aComment );
                    xOutputStream->closeOutput();
                }
                if( bUsedForStore )
                    pLocaleItem->m_bModified = false;
            }
        }
    }

    // Delete files for changed defaults
    if( bUsedForStore || bKillAll )
        implKillChangedDefaultFiles( Location, aNameBase, xFileAccess );

    // Default locale
    if( !(m_pDefaultLocaleItem != nullptr && (bStoreAll || bKillAll || m_bDefaultModified)) )
        return;

    OUString aCompleteFileName =
        implGetPathForLocaleItem( m_pDefaultLocaleItem, aNameBase, Location, true );
    if( xFileAccess->exists( aCompleteFileName ) )
        xFileAccess->kill( aCompleteFileName );

    if( !bKillAll )
    {
        // Create Output stream
        Reference< io::XOutputStream > xOutputStream = xFileAccess->openFileWrite( aCompleteFileName );
        if( xOutputStream.is() )
            xOutputStream->closeOutput();

        if( bUsedForStore )
            m_bDefaultModified = false;
    }
}


// BinaryOutput, helper class for exportBinary

class BinaryOutput
{
    Reference< XComponentContext >          m_xContext;
    Reference< XInterface >                 m_xTempFile;
    Reference< io::XOutputStream >          m_xOutputStream;

public:
    explicit BinaryOutput( Reference< XComponentContext > const & xContext );

    const Reference< io::XOutputStream >& getOutputStream() const
        { return m_xOutputStream; }

    Sequence< ::sal_Int8 > closeAndGetData();

    // Template to be used with sal_Int16 and sal_Unicode
    template< class T >
    void write16BitInt( T n );
    void writeInt16( sal_Int16 n )
        { write16BitInt( n ); }
    void writeUnicodeChar( sal_Unicode n )
        { write16BitInt( n ); }
    void writeInt32( sal_Int32 n );
    void writeString( const OUString& aStr );
};

BinaryOutput::BinaryOutput( Reference< XComponentContext > const & xContext )
        : m_xContext( xContext )
{
    m_xTempFile = io::TempFile::create( m_xContext );
    m_xOutputStream.set( m_xTempFile, UNO_QUERY_THROW );
}

template< class T >
void BinaryOutput::write16BitInt( T n )
{
    if( !m_xOutputStream.is() )
        return;

    Sequence< sal_Int8 > aSeq( 2 );
    sal_Int8* p = aSeq.getArray();

    sal_Int8 nLow  = sal_Int8( n & 0xff );
    sal_Int8 nHigh = sal_Int8( n >> 8 );

    p[0] = nLow;
    p[1] = nHigh;
    m_xOutputStream->writeBytes( aSeq );
}

void BinaryOutput::writeInt32( sal_Int32 n )
{
    if( !m_xOutputStream.is() )
        return;

    Sequence< sal_Int8 > aSeq( 4 );
    sal_Int8* p = aSeq.getArray();

    for( sal_Int16 i = 0 ; i < 4 ; i++ )
    {
        p[i] = sal_Int8( n & 0xff );
        n >>= 8;
    }
    m_xOutputStream->writeBytes( aSeq );
}

void BinaryOutput::writeString( const OUString& aStr )
{
    sal_Int32 nLen = aStr.getLength();
    const sal_Unicode* pStr = aStr.getStr();

    for( sal_Int32 i = 0 ; i < nLen ; i++ )
        writeUnicodeChar( pStr[i] );

    writeUnicodeChar( 0 );
}

Sequence< ::sal_Int8 > BinaryOutput::closeAndGetData()
{
    Sequence< ::sal_Int8 > aRetSeq;
    if( !m_xOutputStream.is() )
        return aRetSeq;

    m_xOutputStream->closeOutput();

    Reference< io::XSeekable> xSeekable( m_xTempFile, UNO_QUERY );
    if( !xSeekable.is() )
        return aRetSeq;

    sal_Int32 nSize = static_cast<sal_Int32>(xSeekable->getPosition());

    Reference< io::XInputStream> xInputStream( m_xTempFile, UNO_QUERY );
    if( !xInputStream.is() )
        return aRetSeq;

    xSeekable->seek( 0 );
    sal_Int32 nRead = xInputStream->readBytes( aRetSeq, nSize );
    OSL_ENSURE( nRead == nSize, "BinaryOutput::closeAndGetData: nRead != nSize" );

    return aRetSeq;
}


// Binary format:

// Header
// Byte         Content
// 0 + 1        sal_Int16:  Version, currently 0, low byte first
// 2 + 3        sal_Int16:  Locale count = n, low byte first
// 4 + 5        sal_Int16:  Default Locale position in Locale list, == n if none
// 6 - 7        sal_Int32:  Start index locale block 0, lowest byte first
// (n-1) *      sal_Int32:  Start index locale block 1 to n, lowest byte first
// 6 + 4*n      sal_Int32:  "Start index" non existing locale block n+1,
//                          marks the first invalid index, kind of EOF

// Locale block
// All strings are stored as 2-Byte-0 terminated sequence
// of 16 bit Unicode characters, each with low byte first
// Empty strings only contain the 2-Byte-0

// Members of com.sun.star.lang.Locale
// with l1 = Locale.Language.getLength()
// with l2 = Locale.Country.getLength()
// with l3 = Locale.Variant.getLength()
// pos0 = 0                     Locale.Language
// pos1 = 2 * (l1 + 1)          Locale.Country
// pos2 = pos1 + 2 * (l2 + 1)   Locale.Variant
// pos3 = pos2 + 2 * (l3 + 1)
// pos3                         Properties file written by implWritePropertiesFile

Sequence< sal_Int8 > StringResourcePersistenceImpl::exportBinary(  )
{
    BinaryOutput aOut( m_xContext );

    sal_Int32 nLocaleCount = m_aLocaleItemVector.size();
    std::vector<Sequence< sal_Int8 >> aLocaleDataSeq(nLocaleCount);

    sal_Int32 iLocale = 0;
    sal_Int32 iDefault = 0;
    for( auto& pLocaleItem : m_aLocaleItemVector )
    {
        if( pLocaleItem != nullptr && loadLocale( pLocaleItem.get() ) )
        {
            if( m_pDefaultLocaleItem == pLocaleItem.get() )
                iDefault = iLocale;

            BinaryOutput aLocaleOut( m_xContext );
            implWriteLocaleBinary( pLocaleItem.get(), aLocaleOut );

            aLocaleDataSeq[iLocale] = aLocaleOut.closeAndGetData();
        }
        ++iLocale;
    }

    // Write header
    sal_Int16 nLocaleCount16 = static_cast<sal_Int16>(nLocaleCount);
    sal_Int16 iDefault16 = static_cast<sal_Int16>(iDefault);
    aOut.writeInt16( 0 ); // nVersion
    aOut.writeInt16( nLocaleCount16 );
    aOut.writeInt16( iDefault16 );

    // Write data positions
    sal_Int32 nDataPos = 6 + 4 * (nLocaleCount + 1);
    for( iLocale = 0; iLocale < nLocaleCount; iLocale++ )
    {
        aOut.writeInt32( nDataPos );

        Sequence< sal_Int8 >& rSeq = aLocaleDataSeq[iLocale];
        sal_Int32 nSeqLen = rSeq.getLength();
        nDataPos += nSeqLen;
    }
    // Write final position
    aOut.writeInt32( nDataPos );

    // Write data
    Reference< io::XOutputStream > xOutputStream = aOut.getOutputStream();
    if( xOutputStream.is() )
    {
        for( iLocale = 0; iLocale < nLocaleCount; iLocale++ )
        {
            Sequence< sal_Int8 >& rSeq = aLocaleDataSeq[iLocale];
            xOutputStream->writeBytes( rSeq );
        }
    }

    Sequence< sal_Int8 > aRetSeq = aOut.closeAndGetData();
    return aRetSeq;
}

void StringResourcePersistenceImpl::implWriteLocaleBinary
    ( LocaleItem* pLocaleItem, BinaryOutput& rOut )
{
    Reference< io::XOutputStream > xOutputStream = rOut.getOutputStream();
    if( !xOutputStream.is() )
        return;

    Locale& rLocale = pLocaleItem->m_locale;
    rOut.writeString( rLocale.Language );
    rOut.writeString( rLocale.Country );
    rOut.writeString( rLocale.Variant );
    implWritePropertiesFile( pLocaleItem, xOutputStream, m_aComment );
}


// BinaryOutput, helper class for exportBinary

namespace {

class BinaryInput
{
    Sequence< sal_Int8 >                    m_aData;
    Reference< XComponentContext >          m_xContext;

    const sal_Int8*                         m_pData;
    sal_Int32                               m_nCurPos;
    sal_Int32                               m_nSize;

public:
    BinaryInput( const Sequence< ::sal_Int8 >& aData, Reference< XComponentContext > const & xContext );

    Reference< io::XInputStream > getInputStreamForSection( sal_Int32 nSize );

    void seek( sal_Int32 nPos );
    sal_Int32 getPosition() const
        { return m_nCurPos; }

    sal_Int16 readInt16();
    sal_Int32 readInt32();
    sal_Unicode readUnicodeChar();
    OUString readString();
};

}

BinaryInput::BinaryInput( const Sequence< ::sal_Int8 >& aData, Reference< XComponentContext > const & xContext )
        : m_aData( aData )
        , m_xContext( xContext )
{
    m_pData = m_aData.getConstArray();
    m_nCurPos = 0;
    m_nSize = m_aData.getLength();
}

Reference< io::XInputStream > BinaryInput::getInputStreamForSection( sal_Int32 nSize )
{
    Reference< io::XInputStream > xIn;
    if( m_nCurPos + nSize <= m_nSize )
    {
        Reference< io::XOutputStream > xTempOut( io::TempFile::create(m_xContext), UNO_QUERY_THROW );
        Sequence< sal_Int8 > aSection( m_pData + m_nCurPos, nSize );
        xTempOut->writeBytes( aSection );

        Reference< io::XSeekable> xSeekable( xTempOut, UNO_QUERY );
        if( xSeekable.is() )
            xSeekable->seek( 0 );

        xIn.set( xTempOut, UNO_QUERY );
    }
    else
        OSL_FAIL( "BinaryInput::getInputStreamForSection(): Read past end" );

    return xIn;
}

void BinaryInput::seek( sal_Int32 nPos )
{
    if( nPos <= m_nSize )
        m_nCurPos = nPos;
    else
        OSL_FAIL( "BinaryInput::seek(): Position past end" );
}


sal_Int16 BinaryInput::readInt16()
{
    sal_Int16 nRet = 0;
    if( m_nCurPos + 2 <= m_nSize )
    {
        nRet = nRet + sal_Int16( sal_uInt8( m_pData[m_nCurPos++] ) );
        nRet += 256 * sal_Int16( sal_uInt8( m_pData[m_nCurPos++] ) );
    }
    else
        OSL_FAIL( "BinaryInput::readInt16(): Read past end" );

    return nRet;
}

sal_Int32 BinaryInput::readInt32()
{
    sal_Int32 nRet = 0;
    if( m_nCurPos + 4 <= m_nSize )
    {
        sal_Int32 nFactor = 1;
        for( sal_Int16 i = 0; i < 4; i++ )
        {
            nRet += sal_uInt8( m_pData[m_nCurPos++] ) * nFactor;
            nFactor *= 256;
        }
    }
    else
        OSL_FAIL( "BinaryInput::readInt32(): Read past end" );

    return nRet;
}

sal_Unicode BinaryInput::readUnicodeChar()
{
    sal_uInt16 nRet = 0;
    if( m_nCurPos + 2 <= m_nSize )
    {
        nRet = nRet + sal_uInt8( m_pData[m_nCurPos++] );
        nRet += 256 * sal_uInt8( m_pData[m_nCurPos++] );
    }
    else
        OSL_FAIL( "BinaryInput::readUnicodeChar(): Read past end" );

    sal_Unicode cRet = nRet;
    return cRet;
}

OUString BinaryInput::readString()
{
    OUStringBuffer aBuf;
    sal_Unicode c;
    do
    {
        c = readUnicodeChar();
        if( c != 0 )
            aBuf.append( c );
    }
    while( c != 0 );

    OUString aRetStr = aBuf.makeStringAndClear();
    return aRetStr;
}

void StringResourcePersistenceImpl::importBinary( const Sequence< ::sal_Int8 >& Data )
{
    // Init: Remove all locales
    sal_Int32 nOldLocaleCount = 0;
    do
    {
        Sequence< Locale > aLocaleSeq = getLocales();
        nOldLocaleCount = aLocaleSeq.getLength();
        if( nOldLocaleCount > 0 )
        {
            Locale aLocale = aLocaleSeq[0];
            removeLocale( aLocale );
        }
    }
    while( nOldLocaleCount > 0 );

    // Import data
    BinaryInput aIn( Data, m_xContext );

    aIn.readInt16(); // version
    sal_Int32 nLocaleCount = aIn.readInt16();
    sal_Int32 iDefault = aIn.readInt16();

    std::unique_ptr<sal_Int32[]> pPositions( new sal_Int32[nLocaleCount + 1] );
    for( sal_Int32 i = 0; i < nLocaleCount + 1; i++ )
        pPositions[i] = aIn.readInt32();

    // Import locales
    LocaleItem* pUseAsDefaultItem = nullptr;
    for( sal_Int32 i = 0; i < nLocaleCount; i++ )
    {
        sal_Int32 nPos = pPositions[i];
        aIn.seek( nPos );

        Locale aLocale;
        aLocale.Language = aIn.readString();
        aLocale.Country = aIn.readString();
        aLocale.Variant = aIn.readString();

        sal_Int32 nAfterStringPos = aIn.getPosition();
        sal_Int32 nSize = pPositions[i+1] - nAfterStringPos;
        Reference< io::XInputStream > xInput = aIn.getInputStreamForSection( nSize );
        if( xInput.is() )
        {
            LocaleItem* pLocaleItem = new LocaleItem( aLocale );
            if( iDefault == i )
                pUseAsDefaultItem = pLocaleItem;
            m_aLocaleItemVector.emplace_back( pLocaleItem );
            implReadPropertiesFile( pLocaleItem, xInput );
        }
    }

    if( pUseAsDefaultItem != nullptr )
        setDefaultLocale( pUseAsDefaultItem->m_locale );
}


// Private helper methods

static bool checkNamingSceme( const OUString& aName, const OUString& aNameBase,
                       Locale& aLocale )
{
    bool bSuccess = false;

    sal_Int32 nNameLen = aName.getLength();
    sal_Int32 nNameBaseLen = aNameBase.getLength();

    // Name has to start with NameBase followed
    // by a '_' and at least one more character
    if( aName.startsWith( aNameBase ) && nNameBaseLen < nNameLen-1 &&
        aName[nNameBaseLen] == '_' )
    {
        bSuccess = true;

        /* FIXME-BCP47: this uses '_' underscore character as separator and
         * also appends Variant, which can't be blindly changed as it would
         * violate the naming scheme in use. */

        sal_Int32 iStart = nNameBaseLen + 1;
        sal_Int32 iNext_ = aName.indexOf( '_', iStart );
        if( iNext_ != -1 && iNext_ < nNameLen-1 )
        {
            aLocale.Language = aName.copy( iStart, iNext_ - iStart );

            iStart = iNext_ + 1;
            iNext_ = aName.indexOf( '_', iStart );
            if( iNext_ != -1 && iNext_ < nNameLen-1 )
            {
                aLocale.Country = aName.copy( iStart, iNext_ - iStart );
                aLocale.Variant = aName.copy( iNext_ + 1 );
            }
            else
                aLocale.Country = aName.copy( iStart );
        }
        else
            aLocale.Language = aName.copy( iStart );
    }
    return bSuccess;
}

void StringResourcePersistenceImpl::implLoadAllLocales()
{
    for( auto& pLocaleItem : m_aLocaleItemVector )
        if( pLocaleItem )
            loadLocale( pLocaleItem.get() );
}

// Scan locale properties files helper
void StringResourcePersistenceImpl::implScanLocaleNames( const Sequence< OUString >& aContentSeq )
{
    Locale aDefaultLocale;
    bool bDefaultFound = false;

    for( const OUString& aCompleteName : aContentSeq )
    {
        OUString aPureName;
        OUString aExtension;
        sal_Int32 iDot = aCompleteName.lastIndexOf( '.' );
        sal_Int32 iSlash = aCompleteName.lastIndexOf( '/' );
        if( iDot != -1 && iDot > iSlash)
        {
            sal_Int32 iCopyFrom = (iSlash != -1) ? iSlash + 1 : 0;
            aPureName = aCompleteName.copy( iCopyFrom, iDot-iCopyFrom );
            aExtension = aCompleteName.copy( iDot + 1 );
        }

        if ( aExtension == "properties" )
        {
            //OUString aName = aInetObj.getBase();
            Locale aLocale;

            if( checkNamingSceme( aPureName, m_aNameBase, aLocale ) )
            {
                LocaleItem* pLocaleItem = new LocaleItem( aLocale, false );
                m_aLocaleItemVector.emplace_back( pLocaleItem );

                if( m_pCurrentLocaleItem == nullptr )
                    m_pCurrentLocaleItem = pLocaleItem;

                if( m_pDefaultLocaleItem == nullptr )
                {
                    m_pDefaultLocaleItem = pLocaleItem;
                    m_bDefaultModified = true;
                }
            }
        }
        else if( !bDefaultFound && aExtension == "default" )
        {
            if( checkNamingSceme( aPureName, m_aNameBase, aDefaultLocale ) )
                bDefaultFound = true;
        }
    }
    if( bDefaultFound )
    {
        LocaleItem* pLocaleItem = getItemForLocale( aDefaultLocale, false );
        if( pLocaleItem )
        {
            m_pDefaultLocaleItem = pLocaleItem;
            m_bDefaultModified = false;
        }
    }
}

// Scan locale properties files
void StringResourcePersistenceImpl::implScanLocales()
{
    // Dummy implementation, method not called for this
    // base class, but pure virtual not possible-
}

bool StringResourcePersistenceImpl::loadLocale( LocaleItem* pLocaleItem )
{
    bool bSuccess = false;

    OSL_ENSURE( pLocaleItem, "StringResourcePersistenceImpl::loadLocale(): pLocaleItem == NULL" );
    if( pLocaleItem )
    {
        if( pLocaleItem->m_bLoaded )
        {
            bSuccess = true;
        }
        else
        {
            bSuccess = implLoadLocale( pLocaleItem );
            pLocaleItem->m_bLoaded = true;      // = bSuccess??? -> leads to more tries
        }
    }
    return bSuccess;
}

bool StringResourcePersistenceImpl::implLoadLocale( LocaleItem* )
{
    // Dummy implementation, method not called for this
    // base class, but pure virtual not possible-
    return false;
}

static OUString implGetNameScemeForLocaleItem( const LocaleItem* pLocaleItem )
{
    /* FIXME-BCP47: this uses '_' underscore character as separator and
     * also appends Variant, which can't be blindly changed as it would
     * violate the naming scheme in use. */

    static const char aUnder[] = "_";

    OSL_ENSURE( pLocaleItem,
        "StringResourcePersistenceImpl::implGetNameScemeForLocaleItem(): pLocaleItem == NULL" );
    Locale aLocale = pLocaleItem->m_locale;

    OUString aRetStr = aUnder + aLocale.Language;

    OUString aCountry  = aLocale.Country;
    if( !aCountry.isEmpty() )
    {
        aRetStr += aUnder + aCountry;
    }

    OUString aVariant  = aLocale.Variant;
    if( !aVariant.isEmpty() )
    {
        aRetStr += aUnder + aVariant;
    }
    return aRetStr;
}

OUString StringResourcePersistenceImpl::implGetFileNameForLocaleItem
    ( LocaleItem const * pLocaleItem, const OUString& aNameBase )
{
    OUString aFileName = aNameBase;
    if( aFileName.isEmpty() )
        aFileName = aNameBaseDefaultStr;

    aFileName += implGetNameScemeForLocaleItem( pLocaleItem );
    return aFileName;
}

OUString StringResourcePersistenceImpl::implGetPathForLocaleItem
    ( LocaleItem const * pLocaleItem, const OUString& aNameBase,
      const OUString& aLocation, bool bDefaultFile )
{
    OUString aFileName = implGetFileNameForLocaleItem( pLocaleItem, aNameBase );
    INetURLObject aInetObj( aLocation );
    aInetObj.insertName( aFileName, true, INetURLObject::LAST_SEGMENT, INetURLObject::EncodeMechanism::All );
    if( bDefaultFile )
        aInetObj.setExtension( "default" );
    else
        aInetObj.setExtension( "properties" );
    OUString aCompleteFileName = aInetObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    return aCompleteFileName;
}

// White space according to Java property files specification in
// http://java.sun.com/j2se/1.4.2/docs/api/java/util/Properties.html#load(java.io.InputStream)
static bool isWhiteSpace( sal_Unicode c )
{
    bool bWhite = ( c == 0x0020 ||      // space
                    c == 0x0009 ||      // tab
                    c == 0x000a ||      // line feed, not always handled by TextInputStream
                    c == 0x000d ||      // carriage return, not always handled by TextInputStream
                    c == 0x000C );      // form feed
    return bWhite;
}

static void skipWhites( const sal_Unicode* pBuf, sal_Int32 nLen, sal_Int32& ri )
{
    while( ri < nLen )
    {
        if( !isWhiteSpace( pBuf[ri] ) )
            break;
        ri++;
    }
}

static bool isHexDigit( sal_Unicode c, sal_uInt16& nDigitVal )
{
    bool bRet = true;
    if( c >= '0' && c <= '9' )
        nDigitVal = c - '0';
    else if( c >= 'a' && c <= 'f' )
        nDigitVal = c - 'a' + 10;
    else if( c >= 'A' && c <= 'F' )
        nDigitVal = c - 'A' + 10;
    else
        bRet = false;
    return bRet;
}

static sal_Unicode getEscapeChar( const sal_Unicode* pBuf, sal_Int32 nLen, sal_Int32& ri )
{
    sal_Int32 i = ri;

    sal_Unicode cRet = 0;
    sal_Unicode c = pBuf[i];
    switch( c )
    {
        case 't':
            cRet = 0x0009;
            break;
        case 'n':
            cRet = 0x000a;
            break;
        case 'f':
            cRet = 0x000c;
            break;
        case 'r':
            cRet = 0x000d;
            break;
        case '\\':
            cRet = '\\';
            break;
        case 'u':
        {
            // Skip multiple u
            i++;
            while( i < nLen && pBuf[i] == 'u' )
                i++;

            // Process hex digits
            sal_Int32 nDigitCount = 0;
            sal_uInt16 nDigitVal;
            while( i < nLen && isHexDigit( pBuf[i], nDigitVal ) )
            {
                cRet = 16 * cRet + nDigitVal;

                nDigitCount++;
                if( nDigitCount == 4 )
                {
                    // Write back position
                    ri = i;
                    break;
                }
                i++;
            }
            break;
        }
        default:
            cRet = c;
    }

    return cRet;
}

static void CheckContinueInNextLine( const Reference< io::XTextInputStream2 >& xTextInputStream,
    OUString& aLine, bool& bEscapePending, const sal_Unicode*& pBuf,
    sal_Int32& nLen, sal_Int32& i )
{
    if( !(i == nLen && bEscapePending) )
        return;

    bEscapePending = false;

    if( !xTextInputStream->isEOF() )
    {
        aLine = xTextInputStream->readLine();
        nLen = aLine.getLength();
        pBuf = aLine.getStr();
        i = 0;

        skipWhites( pBuf, nLen, i );
    }
}

bool StringResourcePersistenceImpl::implReadPropertiesFile
    ( LocaleItem* pLocaleItem, const Reference< io::XInputStream >& xInputStream )
{
    if( !xInputStream.is() || pLocaleItem == nullptr )
        return false;

    Reference< io::XTextInputStream2 > xTextInputStream = io::TextInputStream::create( m_xContext );

    xTextInputStream->setInputStream( xInputStream );

    OUString aEncodingStr = OUString::createFromAscii
        ( rtl_getMimeCharsetFromTextEncoding( RTL_TEXTENCODING_ISO_8859_1 ) );
    xTextInputStream->setEncoding( aEncodingStr );

    OUString aLine;
    while( !xTextInputStream->isEOF() )
    {
        aLine = xTextInputStream->readLine();

        sal_Int32 nLen = aLine.getLength();
        if( 0 == nLen )
            continue;
        const sal_Unicode* pBuf = aLine.getStr();
        OUStringBuffer aBuf;
        sal_Unicode c = 0;
        sal_Int32 i = 0;

        skipWhites( pBuf, nLen, i );
        if( i == nLen )
            continue;   // line contains only white spaces

        // Comment?
        c = pBuf[i];
        if( c == '#' || c == '!' )
            continue;

        // Scan key
        OUString aResourceID;
        bool bEscapePending = false;
        bool bStrComplete = false;
        while( i < nLen && !bStrComplete )
        {
            c = pBuf[i];
            if( bEscapePending )
            {
                aBuf.append( getEscapeChar( pBuf, nLen, i ) );
                bEscapePending = false;
            }
            else
            {
                if( c == '\\' )
                {
                    bEscapePending = true;
                }
                else
                {
                    if( c == ':' || c == '=' || isWhiteSpace( c ) )
                        bStrComplete = true;
                    else
                        aBuf.append( c );
                }
            }
            i++;

            CheckContinueInNextLine( xTextInputStream, aLine, bEscapePending, pBuf, nLen, i );
            if( i == nLen )
                bStrComplete = true;

            if( bStrComplete )
                aResourceID = aBuf.makeStringAndClear();
        }

        // Ignore lines with empty keys
        if( aResourceID.isEmpty() )
            continue;

        // Scan value
        skipWhites( pBuf, nLen, i );

        OUString aValueStr;
        bEscapePending = false;
        bStrComplete = false;
        while( i < nLen && !bStrComplete )
        {
            c = pBuf[i];
            if( c == 0x000a || c == 0x000d )    // line feed/carriage return, not always handled by TextInputStream
            {
                i++;
            }
            else
            {
                if( bEscapePending )
                {
                    aBuf.append( getEscapeChar( pBuf, nLen, i ) );
                    bEscapePending = false;
                }
                else if( c == '\\' )
                    bEscapePending = true;
                else
                    aBuf.append( c );
                i++;

                CheckContinueInNextLine( xTextInputStream, aLine, bEscapePending, pBuf, nLen, i );
            }
            if( i == nLen )
                bStrComplete = true;

            if( bStrComplete )
                aValueStr = aBuf.makeStringAndClear();
        }

        // Push into table
        pLocaleItem->m_aIdToStringMap[ aResourceID ] = aValueStr;
        implScanIdForNumber( aResourceID );
        IdToIndexMap& rIndexMap = pLocaleItem->m_aIdToIndexMap;
        rIndexMap[ aResourceID ] = pLocaleItem->m_nNextIndex++;
    }

    return true;
}


static sal_Unicode getHexCharForDigit( sal_uInt16 nDigitVal )
{
    sal_Unicode cRet = ( nDigitVal < 10 ) ? ('0' + nDigitVal) : ('a' + (nDigitVal-10));
    return cRet;
}

static void implWriteCharToBuffer( OUStringBuffer& aBuf, sal_Unicode cu, bool bKey )
{
    if( cu == '\\' )
    {
        aBuf.append( '\\' );
        aBuf.append( '\\' );
    }
    else if( cu == 0x000a )
    {
        aBuf.append( '\\' );
        aBuf.append( 'n' );
    }
    else if( cu == 0x000d )
    {
        aBuf.append( '\\' );
        aBuf.append( 'r' );
    }
    else if( bKey && cu == '=' )
    {
        aBuf.append( '\\' );
        aBuf.append( '=' );
    }
    else if( bKey && cu == ':' )
    {
        aBuf.append( '\\' );
        aBuf.append( ':' );
    }
    // ISO/IEC 8859-1 range according to:
    // http://en.wikipedia.org/wiki/ISO/IEC_8859-1
    else if( cu >= 0x20 && cu <= 0x7e )
    //TODO: Check why (cu >= 0xa0 && cu <= 0xFF)
    //is encoded in sample properties files
    //else if( (cu >= 0x20 && cu <= 0x7e) ||
    //       (cu >= 0xa0 && cu <= 0xFF) )
    {
        aBuf.append( cu );
    }
    else
    {
        // Unicode encoding
        aBuf.append( '\\' );
        aBuf.append( 'u' );

        sal_uInt16 nVal = cu;
        for( sal_uInt16 i = 0 ; i < 4 ; i++ )
        {
            sal_uInt16 nDigit = nVal / 0x1000;
            nVal -= nDigit * 0x1000;
            nVal *= 0x10;
            aBuf.append( getHexCharForDigit( nDigit ) );
        }
    }
}

static void implWriteStringWithEncoding( const OUString& aStr,
    Reference< io::XTextOutputStream2 > const & xTextOutputStream, bool bKey )
{
    static const sal_Unicode cLineFeed = 0xa;

    OUStringBuffer aBuf;
    sal_Int32 nLen = aStr.getLength();
    const sal_Unicode* pSrc = aStr.getStr();
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
    {
        sal_Unicode cu = pSrc[i];
        implWriteCharToBuffer( aBuf, cu, bKey );
        // TODO?: split long lines
    }
    if( !bKey )
        aBuf.append( cLineFeed );

    OUString aWriteStr = aBuf.makeStringAndClear();
    xTextOutputStream->writeString( aWriteStr );
}

bool StringResourcePersistenceImpl::implWritePropertiesFile( LocaleItem const * pLocaleItem,
    const Reference< io::XOutputStream >& xOutputStream, const OUString& aComment )
{
    if( !xOutputStream.is() || pLocaleItem == nullptr )
        return false;

    bool bSuccess = false;
    Reference< io::XTextOutputStream2 > xTextOutputStream = io::TextOutputStream::create(m_xContext);

    xTextOutputStream->setOutputStream( xOutputStream );

    OUString aEncodingStr = OUString::createFromAscii
        ( rtl_getMimeCharsetFromTextEncoding( RTL_TEXTENCODING_ISO_8859_1 ) );
    xTextOutputStream->setEncoding( aEncodingStr );

    xTextOutputStream->writeString( aComment );
    xTextOutputStream->writeString( "\n" );

    const IdToStringMap& rHashMap = pLocaleItem->m_aIdToStringMap;
    if( !rHashMap.empty() )
    {
        // Sort ids according to read order
        const IdToIndexMap& rIndexMap = pLocaleItem->m_aIdToIndexMap;

        // Find max/min index
        auto itMinMax = std::minmax_element(rIndexMap.begin(), rIndexMap.end(),
            [](const IdToIndexMap::value_type& a, const IdToIndexMap::value_type& b) { return a.second < b.second; });
        sal_Int32 nMinIndex = itMinMax.first->second;
        sal_Int32 nMaxIndex = itMinMax.second->second;
        sal_Int32 nTabSize = nMaxIndex - nMinIndex + 1;

        // Create sorted array of pointers to the id strings
        std::unique_ptr<const OUString*[]> pIdPtrs( new const OUString*[nTabSize] );
        for(sal_Int32 i = 0 ; i < nTabSize ; i++ )
            pIdPtrs[i] = nullptr;
        for( const auto& rIndex : rIndexMap )
        {
            sal_Int32 nIndex = rIndex.second;
            pIdPtrs[nIndex - nMinIndex] = &(rIndex.first);
        }

        // Write lines in correct order
        for(sal_Int32 i = 0 ; i < nTabSize ; i++ )
        {
            const OUString* pStr = pIdPtrs[i];
            if( pStr != nullptr )
            {
                OUString aResourceID = *pStr;
                IdToStringMap::const_iterator it = rHashMap.find( aResourceID );
                if( it != rHashMap.end() )
                {
                    implWriteStringWithEncoding( aResourceID, xTextOutputStream, true );
                    xTextOutputStream->writeString( "=" );
                    OUString aValStr = (*it).second;
                    implWriteStringWithEncoding( aValStr, xTextOutputStream, false );
                }
            }
        }
    }

    bSuccess = true;

    return bSuccess;
}


// StringResourceWithStorageImpl

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
scripting_StringResourceWithStorageImpl_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new StringResourceWithStorageImpl(context));
}


StringResourceWithStorageImpl::StringResourceWithStorageImpl( const Reference< XComponentContext >& rxContext )
    : StringResourceWithStorageImpl_BASE( rxContext )
    , m_bStorageChanged( false )
{
}


StringResourceWithStorageImpl::~StringResourceWithStorageImpl()
{
}


// XServiceInfo


OUString StringResourceWithStorageImpl::getImplementationName(  )
{
    return "com.sun.star.comp.scripting.StringResourceWithStorage";
}

sal_Bool StringResourceWithStorageImpl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > StringResourceWithStorageImpl::getSupportedServiceNames(  )
{
    return { "com.sun.star.resource.StringResourceWithStorage" };
}


// XInitialization


void StringResourceWithStorageImpl::initialize( const Sequence< Any >& aArguments )
{
    ::osl::MutexGuard aGuard( getMutex() );

    if ( aArguments.getLength() != 5 )
    {
        throw RuntimeException(
            "StringResourceWithStorageImpl::initialize: invalid number of arguments!" );
    }

    bool bOk = (aArguments[0] >>= m_xStorage);
    if( bOk && !m_xStorage.is() )
        bOk = false;

    if( !bOk )
    {
        throw IllegalArgumentException( "StringResourceWithStorageImpl::initialize: invalid storage", Reference< XInterface >(), 0 );
    }

    implInitializeCommonParameters( aArguments );
}


// Forwarding calls to base class

// XModifyBroadcaster
void StringResourceWithStorageImpl::addModifyListener( const Reference< XModifyListener >& aListener )
{
    StringResourceImpl::addModifyListener( aListener );
}
void StringResourceWithStorageImpl::removeModifyListener( const Reference< XModifyListener >& aListener )
{
    StringResourceImpl::removeModifyListener( aListener );
}

// XStringResourceResolver
OUString StringResourceWithStorageImpl::resolveString( const OUString& ResourceID )
{
    return StringResourceImpl::resolveString( ResourceID ) ;
}
OUString StringResourceWithStorageImpl::resolveStringForLocale( const OUString& ResourceID, const Locale& locale )
{
    return StringResourceImpl::resolveStringForLocale( ResourceID, locale );
}
sal_Bool StringResourceWithStorageImpl::hasEntryForId( const OUString& ResourceID )
{
    return StringResourceImpl::hasEntryForId( ResourceID ) ;
}
sal_Bool StringResourceWithStorageImpl::hasEntryForIdAndLocale( const OUString& ResourceID,
    const Locale& locale )
{
    return StringResourceImpl::hasEntryForIdAndLocale( ResourceID, locale );
}
Sequence< OUString > StringResourceWithStorageImpl::getResourceIDs(  )
{
    return StringResourceImpl::getResourceIDs();
}
Sequence< OUString > StringResourceWithStorageImpl::getResourceIDsForLocale
    ( const Locale& locale )
{
    return StringResourceImpl::getResourceIDsForLocale( locale );
}
Locale StringResourceWithStorageImpl::getCurrentLocale()
{
    return StringResourceImpl::getCurrentLocale();
}
Locale StringResourceWithStorageImpl::getDefaultLocale(  )
{
    return StringResourceImpl::getDefaultLocale();
}
Sequence< Locale > StringResourceWithStorageImpl::getLocales(  )
{
    return StringResourceImpl::getLocales();
}

// XStringResourceManager
sal_Bool StringResourceWithStorageImpl::isReadOnly()
{
    return StringResourceImpl::isReadOnly();
}
void StringResourceWithStorageImpl::setCurrentLocale( const Locale& locale, sal_Bool FindClosestMatch )
{
    StringResourceImpl::setCurrentLocale( locale, FindClosestMatch );
}
void StringResourceWithStorageImpl::setDefaultLocale( const Locale& locale )
{
    StringResourceImpl::setDefaultLocale( locale );
}
void StringResourceWithStorageImpl::setString( const OUString& ResourceID, const OUString& Str )
{
    StringResourceImpl::setString( ResourceID, Str );
}
void StringResourceWithStorageImpl::setStringForLocale
    ( const OUString& ResourceID, const OUString& Str, const Locale& locale )
{
    StringResourceImpl::setStringForLocale( ResourceID, Str, locale );
}
void StringResourceWithStorageImpl::removeId( const OUString& ResourceID )
{
    StringResourceImpl::removeId( ResourceID );
}
void StringResourceWithStorageImpl::removeIdForLocale( const OUString& ResourceID, const Locale& locale )
{
    StringResourceImpl::removeIdForLocale( ResourceID, locale );
}
void StringResourceWithStorageImpl::newLocale( const Locale& locale )
{
    StringResourceImpl::newLocale( locale );
}
void StringResourceWithStorageImpl::removeLocale( const Locale& locale )
{
    StringResourceImpl::removeLocale( locale );
}
sal_Int32 StringResourceWithStorageImpl::getUniqueNumericId(  )
{
    return StringResourceImpl::getUniqueNumericId();
}

// XStringResourcePersistence
void StringResourceWithStorageImpl::store()
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceWithStorageImpl::store(): Read only" );

    bool bStoreAll = m_bStorageChanged;
    m_bStorageChanged = false;
    if( !m_bModified && !bStoreAll )
        return;

    implStoreAtStorage( m_aNameBase, m_aComment, m_xStorage, true/*bUsedForStore*/, bStoreAll );
    m_bModified = false;
}

sal_Bool StringResourceWithStorageImpl::isModified(  )
{
    return StringResourcePersistenceImpl::isModified();
}
void StringResourceWithStorageImpl::setComment( const OUString& Comment )
{
    StringResourcePersistenceImpl::setComment( Comment );
}
void StringResourceWithStorageImpl::storeToStorage( const Reference< XStorage >& Storage,
    const OUString& NameBase, const OUString& Comment )
{
    StringResourcePersistenceImpl::storeToStorage( Storage, NameBase, Comment );
}
void StringResourceWithStorageImpl::storeToURL( const OUString& URL,
    const OUString& NameBase, const OUString& Comment,
    const Reference< css::task::XInteractionHandler >& Handler )
{
    StringResourcePersistenceImpl::storeToURL( URL, NameBase, Comment, Handler );
}
Sequence< ::sal_Int8 > StringResourceWithStorageImpl::exportBinary(  )
{
    return StringResourcePersistenceImpl::exportBinary();
}
void StringResourceWithStorageImpl::importBinary( const Sequence< ::sal_Int8 >& Data )
{
    StringResourcePersistenceImpl::importBinary( Data );
}


// XStringResourceWithStorage

void StringResourceWithStorageImpl::storeAsStorage( const Reference< XStorage >& Storage )
{
    setStorage( Storage );
    store();
}

void StringResourceWithStorageImpl::setStorage( const Reference< XStorage >& Storage )
{
    ::osl::MutexGuard aGuard( getMutex() );

    if( !Storage.is() )
    {
        throw IllegalArgumentException( "StringResourceWithStorageImpl::setStorage: invalid storage", Reference< XInterface >(), 0 );
    }

    implLoadAllLocales();

    m_xStorage = Storage;
    m_bStorageChanged = true;
}


// Private helper methods


// Scan locale properties files
void StringResourceWithStorageImpl::implScanLocales()
{
    if( m_xStorage.is() )
    {
        Sequence< OUString > aContentSeq = m_xStorage->getElementNames();
        implScanLocaleNames( aContentSeq );
    }

    implLoadAllLocales();
}

// Loading
bool StringResourceWithStorageImpl::implLoadLocale( LocaleItem* pLocaleItem )
{
    bool bSuccess = false;
    try
    {
        OUString aStreamName = implGetFileNameForLocaleItem( pLocaleItem, m_aNameBase ) + ".properties";

        Reference< io::XStream > xElementStream =
            m_xStorage->openStreamElement( aStreamName, ElementModes::READ );

        if( xElementStream.is() )
        {
            Reference< io::XInputStream > xInputStream = xElementStream->getInputStream();
            if( xInputStream.is() )
            {
                bSuccess = StringResourcePersistenceImpl::implReadPropertiesFile( pLocaleItem, xInputStream );
                xInputStream->closeInput();
            }
        }
    }
    catch( uno::Exception& )
    {}

    return bSuccess;
}


// StringResourceWithLocationImpl


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
scripting_StringResourceWithLocationImpl_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new StringResourceWithLocationImpl(context));
}



StringResourceWithLocationImpl::StringResourceWithLocationImpl( const Reference< XComponentContext >& rxContext )
    : StringResourceWithLocationImpl_BASE( rxContext )
    , m_bLocationChanged( false )
{
}


StringResourceWithLocationImpl::~StringResourceWithLocationImpl()
{
}


// XServiceInfo


OUString StringResourceWithLocationImpl::getImplementationName(  )
{
    return "com.sun.star.comp.scripting.StringResourceWithLocation";
}

sal_Bool StringResourceWithLocationImpl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > StringResourceWithLocationImpl::getSupportedServiceNames(  )
{
    return { "com.sun.star.resource.StringResourceWithLocation" };
}


// XInitialization


void StringResourceWithLocationImpl::initialize( const Sequence< Any >& aArguments )
{
    ::osl::MutexGuard aGuard( getMutex() );

    if ( aArguments.getLength() != 6 )
    {
        throw RuntimeException(
            "XInitialization::initialize: invalid number of arguments!" );
    }

    bool bOk = (aArguments[0] >>= m_aLocation);
    sal_Int32 nLen = m_aLocation.getLength();
    if( bOk && nLen == 0 )
    {
        bOk = false;
    }
    else
    {
        if( m_aLocation[nLen - 1] != '/' )
            m_aLocation += "/";
    }

    if( !bOk )
    {
        throw IllegalArgumentException( "XInitialization::initialize: invalid URL", Reference< XInterface >(), 0 );
    }


    bOk = (aArguments[5] >>= m_xInteractionHandler);
    if( !bOk )
    {
        throw IllegalArgumentException( "StringResourceWithStorageImpl::initialize: invalid type", Reference< XInterface >(), 5 );
    }

    implInitializeCommonParameters( aArguments );
}


// Forwarding calls to base class

// XModifyBroadcaster
void StringResourceWithLocationImpl::addModifyListener( const Reference< XModifyListener >& aListener )
{
    StringResourceImpl::addModifyListener( aListener );
}
void StringResourceWithLocationImpl::removeModifyListener( const Reference< XModifyListener >& aListener )
{
    StringResourceImpl::removeModifyListener( aListener );
}

// XStringResourceResolver
OUString StringResourceWithLocationImpl::resolveString( const OUString& ResourceID )
{
    return StringResourceImpl::resolveString( ResourceID ) ;
}
OUString StringResourceWithLocationImpl::resolveStringForLocale( const OUString& ResourceID, const Locale& locale )
{
    return StringResourceImpl::resolveStringForLocale( ResourceID, locale );
}
sal_Bool StringResourceWithLocationImpl::hasEntryForId( const OUString& ResourceID )
{
    return StringResourceImpl::hasEntryForId( ResourceID ) ;
}
sal_Bool StringResourceWithLocationImpl::hasEntryForIdAndLocale( const OUString& ResourceID,
    const Locale& locale )
{
    return StringResourceImpl::hasEntryForIdAndLocale( ResourceID, locale );
}
Sequence< OUString > StringResourceWithLocationImpl::getResourceIDs(  )
{
    return StringResourceImpl::getResourceIDs();
}
Sequence< OUString > StringResourceWithLocationImpl::getResourceIDsForLocale
    ( const Locale& locale )
{
    return StringResourceImpl::getResourceIDsForLocale( locale );
}
Locale StringResourceWithLocationImpl::getCurrentLocale()
{
    return StringResourceImpl::getCurrentLocale();
}
Locale StringResourceWithLocationImpl::getDefaultLocale(  )
{
    return StringResourceImpl::getDefaultLocale();
}
Sequence< Locale > StringResourceWithLocationImpl::getLocales(  )
{
    return StringResourceImpl::getLocales();
}

// XStringResourceManager
sal_Bool StringResourceWithLocationImpl::isReadOnly()
{
    return StringResourceImpl::isReadOnly();
}
void StringResourceWithLocationImpl::setCurrentLocale( const Locale& locale, sal_Bool FindClosestMatch )
{
    StringResourceImpl::setCurrentLocale( locale, FindClosestMatch );
}
void StringResourceWithLocationImpl::setDefaultLocale( const Locale& locale )
{
    StringResourceImpl::setDefaultLocale( locale );
}
void StringResourceWithLocationImpl::setString( const OUString& ResourceID, const OUString& Str )
{
    StringResourceImpl::setString( ResourceID, Str );
}
void StringResourceWithLocationImpl::setStringForLocale
    ( const OUString& ResourceID, const OUString& Str, const Locale& locale )
{
    StringResourceImpl::setStringForLocale( ResourceID, Str, locale );
}
void StringResourceWithLocationImpl::removeId( const OUString& ResourceID )
{
    StringResourceImpl::removeId( ResourceID );
}
void StringResourceWithLocationImpl::removeIdForLocale( const OUString& ResourceID, const Locale& locale )
{
    StringResourceImpl::removeIdForLocale( ResourceID, locale );
}
void StringResourceWithLocationImpl::newLocale( const Locale& locale )
{
    StringResourceImpl::newLocale( locale );
}
void StringResourceWithLocationImpl::removeLocale( const Locale& locale )
{
    StringResourceImpl::removeLocale( locale );
}
sal_Int32 StringResourceWithLocationImpl::getUniqueNumericId(  )
{
    return StringResourceImpl::getUniqueNumericId();
}

// XStringResourcePersistence
void StringResourceWithLocationImpl::store()
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceWithLocationImpl::store(): Read only" );

    bool bStoreAll = m_bLocationChanged;
    m_bLocationChanged = false;
    if( !m_bModified && !bStoreAll )
        return;

    Reference< ucb::XSimpleFileAccess3 > xFileAccess = getFileAccess();
    implStoreAtLocation( m_aLocation, m_aNameBase, m_aComment,
        xFileAccess, true/*bUsedForStore*/, bStoreAll );
    m_bModified = false;
}

sal_Bool StringResourceWithLocationImpl::isModified(  )
{
    return StringResourcePersistenceImpl::isModified();
}
void StringResourceWithLocationImpl::setComment( const OUString& Comment )
{
    StringResourcePersistenceImpl::setComment( Comment );
}
void StringResourceWithLocationImpl::storeToStorage( const Reference< XStorage >& Storage,
    const OUString& NameBase, const OUString& Comment )
{
    StringResourcePersistenceImpl::storeToStorage( Storage, NameBase, Comment );
}
void StringResourceWithLocationImpl::storeToURL( const OUString& URL,
    const OUString& NameBase, const OUString& Comment,
    const Reference< css::task::XInteractionHandler >& Handler )
{
    StringResourcePersistenceImpl::storeToURL( URL, NameBase, Comment, Handler );
}
Sequence< ::sal_Int8 > StringResourceWithLocationImpl::exportBinary(  )
{
    return StringResourcePersistenceImpl::exportBinary();
}
void StringResourceWithLocationImpl::importBinary( const Sequence< ::sal_Int8 >& Data )
{
    StringResourcePersistenceImpl::importBinary( Data );
}


// XStringResourceWithLocation

// XStringResourceWithLocation
void StringResourceWithLocationImpl::storeAsURL( const OUString& URL )
{
    setURL( URL );
    store();
}

void StringResourceWithLocationImpl::setURL( const OUString& URL )
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceWithLocationImpl::setURL(): Read only" );

    sal_Int32 nLen = URL.getLength();
    if( nLen == 0 )
    {
        throw IllegalArgumentException( "StringResourceWithLocationImpl::setURL: invalid URL", Reference< XInterface >(), 0 );
    }

    implLoadAllLocales();

    // Delete files at old location
    implStoreAtLocation( m_aLocation, m_aNameBase, m_aComment,
        getFileAccess(), false/*bUsedForStore*/, false/*bStoreAll*/, true/*bKillAll*/ );

    m_aLocation = URL;
    m_bLocationChanged = true;
}


// Private helper methods


// Scan locale properties files
void StringResourceWithLocationImpl::implScanLocales()
{
    const Reference< ucb::XSimpleFileAccess3 > xFileAccess = getFileAccess();
    if( xFileAccess.is() && xFileAccess->isFolder( m_aLocation ) )
    {
        Sequence< OUString > aContentSeq = xFileAccess->getFolderContents( m_aLocation, false );
        implScanLocaleNames( aContentSeq );
    }
}

// Loading
bool StringResourceWithLocationImpl::implLoadLocale( LocaleItem* pLocaleItem )
{
    bool bSuccess = false;

    const Reference< ucb::XSimpleFileAccess3 > xFileAccess = getFileAccess();
    if( xFileAccess.is() )
    {
        OUString aCompleteFileName =
            implGetPathForLocaleItem( pLocaleItem, m_aNameBase, m_aLocation );

        Reference< io::XInputStream > xInputStream;
        try
        {
            xInputStream = xFileAccess->openFileRead( aCompleteFileName );
        }
        catch( Exception& )
        {}
        if( xInputStream.is() )
        {
            bSuccess = StringResourcePersistenceImpl::implReadPropertiesFile( pLocaleItem, xInputStream );
            xInputStream->closeInput();
        }
    }

    return bSuccess;
}

const Reference< ucb::XSimpleFileAccess3 > & StringResourceWithLocationImpl::getFileAccess()
{
    ::osl::MutexGuard aGuard( getMutex() );

    if( !m_xSFI.is() )
    {
        m_xSFI = ucb::SimpleFileAccess::create(m_xContext);

        if( m_xSFI.is() && m_xInteractionHandler.is() )
            m_xSFI->setInteractionHandler( m_xInteractionHandler );
    }
    return m_xSFI;
}

}   // namespace stringresource


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
