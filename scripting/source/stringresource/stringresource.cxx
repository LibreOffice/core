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

#include "stringresource.hxx"
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XTextInputStream.hpp>
#include <com/sun/star/io/XTextOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>


#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <tools/urlobj.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::container;


//.........................................................................
namespace stringresource
{
//.........................................................................

// =============================================================================
// mutex
// =============================================================================

::osl::Mutex& getMutex()
{
    static ::osl::Mutex* s_pMutex = 0;
    if ( !s_pMutex )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !s_pMutex )
        {
            static ::osl::Mutex s_aMutex;
            s_pMutex = &s_aMutex;
        }
    }
    return *s_pMutex;
}


// =============================================================================
// StringResourceImpl
// =============================================================================

// component operations
static Sequence< ::rtl::OUString > getSupportedServiceNames_StringResourceImpl()
{
    Sequence< ::rtl::OUString > names(1);
    names[0] = ::rtl::OUString( "com.sun.star.resource.StringResource" );
    return names;
}

static ::rtl::OUString getImplementationName_StringResourceImpl()
{
    return ::rtl::OUString( "com.sun.star.comp.scripting.StringResource" );
}

static Reference< XInterface > SAL_CALL create_StringResourceImpl(
    Reference< XComponentContext > const & xContext )
    SAL_THROW(())
{
    return static_cast< ::cppu::OWeakObject * >( new StringResourcePersistenceImpl( xContext ) );
}


// =============================================================================

StringResourceImpl::StringResourceImpl( const Reference< XComponentContext >& rxContext )
    : m_xContext( rxContext )
    , m_pCurrentLocaleItem( NULL )
    , m_pDefaultLocaleItem( NULL )
    , m_bDefaultModified( false )
    , m_aListenerContainer( getMutex() )
    , m_bModified( false )
    , m_bReadOnly( false )
    , m_nNextUniqueNumericId( UNIQUE_NUMBER_NEEDS_INITIALISATION )
{
}

// =============================================================================

StringResourceImpl::~StringResourceImpl()
{
    for( LocaleItemVectorIt it = m_aLocaleItemVector.begin(); it != m_aLocaleItemVector.end(); ++it )
    {
        LocaleItem* pLocaleItem = *it;
        delete pLocaleItem;
    }

    for( LocaleItemVectorIt it = m_aDeletedLocaleItemVector.begin(); it != m_aDeletedLocaleItemVector.end(); ++it )
    {
        LocaleItem* pLocaleItem = *it;
        delete pLocaleItem;
    }
}


// =============================================================================
// XServiceInfo

::rtl::OUString StringResourceImpl::getImplementationName(  ) throw (RuntimeException)
{
    return getImplementationName_StringResourceImpl();
}

sal_Bool StringResourceImpl::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
    const ::rtl::OUString* pNames = aNames.getConstArray();
    const ::rtl::OUString* pEnd = pNames + aNames.getLength();
    for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
        ;

    return pNames != pEnd;
}

Sequence< ::rtl::OUString > StringResourceImpl::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return getSupportedServiceNames_StringResourceImpl();
}


// =============================================================================
// XModifyBroadcaster

void StringResourceImpl::addModifyListener( const Reference< XModifyListener >& aListener )
    throw (RuntimeException)
{
    if( !aListener.is() )
        throw RuntimeException();

    ::osl::MutexGuard aGuard( getMutex() );
    Reference< XInterface > xIface( aListener, UNO_QUERY );
    m_aListenerContainer.addInterface( xIface );
}

void StringResourceImpl::removeModifyListener( const Reference< XModifyListener >& aListener )
    throw (RuntimeException)
{
    if( !aListener.is() )
        throw RuntimeException();

    ::osl::MutexGuard aGuard( getMutex() );
    Reference< XInterface > xIface( aListener, UNO_QUERY );
    m_aListenerContainer.removeInterface( xIface );
}


// =============================================================================
// XStringResourceResolver

::rtl::OUString StringResourceImpl::implResolveString
    ( const ::rtl::OUString& ResourceID, LocaleItem* pLocaleItem )
        throw (::com::sun::star::resource::MissingResourceException)
{
    ::rtl::OUString aRetStr;
    bool bSuccess = false;
    if( pLocaleItem != NULL && loadLocale( pLocaleItem ) )
    {
        IdToStringMap::iterator it = pLocaleItem->m_aIdToStringMap.find( ResourceID );
        if( !( it == pLocaleItem->m_aIdToStringMap.end() ) )
        {
            aRetStr = (*it).second;
            bSuccess = true;
        }
    }
    if( !bSuccess )
    {
        ::rtl::OUString errorMsg("StringResourceImpl: No entry for ResourceID: ");
        errorMsg.concat( ResourceID );
        throw ::com::sun::star::resource::MissingResourceException( errorMsg, Reference< XInterface >() );
    }
    return aRetStr;
}

::rtl::OUString StringResourceImpl::resolveString( const ::rtl::OUString& ResourceID )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    return implResolveString( ResourceID, m_pCurrentLocaleItem );
}

::rtl::OUString StringResourceImpl::resolveStringForLocale( const ::rtl::OUString& ResourceID, const Locale& locale )
    throw ( ::com::sun::star::resource::MissingResourceException, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    LocaleItem* pLocaleItem = getItemForLocale( locale, false );
    return implResolveString( ResourceID, pLocaleItem );
}

sal_Bool StringResourceImpl::implHasEntryForId( const ::rtl::OUString& ResourceID, LocaleItem* pLocaleItem )
{
    bool bSuccess = false;
    if( pLocaleItem != NULL && loadLocale( pLocaleItem ) )
    {
        IdToStringMap::iterator it = pLocaleItem->m_aIdToStringMap.find( ResourceID );
        if( !( it == pLocaleItem->m_aIdToStringMap.end() ) )
            bSuccess = true;
    }
    return bSuccess;
}

sal_Bool StringResourceImpl::hasEntryForId( const ::rtl::OUString& ResourceID )
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    return implHasEntryForId( ResourceID, m_pCurrentLocaleItem );
}

sal_Bool StringResourceImpl::hasEntryForIdAndLocale( const ::rtl::OUString& ResourceID,
    const Locale& locale )
        throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    LocaleItem* pLocaleItem = getItemForLocale( locale, false );
    return implHasEntryForId( ResourceID, pLocaleItem );
}

Sequence< ::rtl::OUString > StringResourceImpl::implGetResourceIDs( LocaleItem* pLocaleItem )
{
    Sequence< ::rtl::OUString > aIDSeq( 0 );
    if( pLocaleItem && loadLocale( pLocaleItem ) )
    {
        const IdToStringMap& rHashMap = pLocaleItem->m_aIdToStringMap;
        sal_Int32 nResourceIDCount = rHashMap.size();
        aIDSeq.realloc( nResourceIDCount );
        ::rtl::OUString* pStrings = aIDSeq.getArray();

        IdToStringMap::const_iterator it;
        int iTarget = 0;
        for( it = rHashMap.begin(); it != rHashMap.end(); ++it )
        {
            ::rtl::OUString aStr = (*it).first;
            pStrings[iTarget] = aStr;
            iTarget++;
        }
    }
    return aIDSeq;
}

Sequence< ::rtl::OUString > StringResourceImpl::getResourceIDsForLocale
    ( const Locale& locale ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    LocaleItem* pLocaleItem = getItemForLocale( locale, false );
    return implGetResourceIDs( pLocaleItem );
}

Sequence< ::rtl::OUString > StringResourceImpl::getResourceIDs(  )
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    return implGetResourceIDs( m_pCurrentLocaleItem );
}

Locale StringResourceImpl::getCurrentLocale()
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );

    Locale aRetLocale;
    if( m_pCurrentLocaleItem != NULL )
        aRetLocale = m_pCurrentLocaleItem->m_locale;
    return aRetLocale;
}

Locale StringResourceImpl::getDefaultLocale(  )
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );

    Locale aRetLocale;
    if( m_pDefaultLocaleItem != NULL )
        aRetLocale = m_pDefaultLocaleItem->m_locale;
    return aRetLocale;
}

Sequence< Locale > StringResourceImpl::getLocales(  )
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );

    sal_Int32 nSize = m_aLocaleItemVector.size();
    Sequence< Locale > aLocalSeq( nSize );
    Locale* pLocales = aLocalSeq.getArray();
    int iTarget = 0;
    for( LocaleItemVectorConstIt it = m_aLocaleItemVector.begin(); it != m_aLocaleItemVector.end(); ++it )
    {
        LocaleItem* pLocaleItem = *it;
        pLocales[iTarget] = pLocaleItem->m_locale;
        iTarget++;
    }
    return aLocalSeq;
}


// =============================================================================
// XStringResourceManager

void StringResourceImpl::implCheckReadOnly( const sal_Char* pExceptionMsg )
    throw (NoSupportException)
{
    if( m_bReadOnly )
    {
        ::rtl::OUString errorMsg = ::rtl::OUString::createFromAscii( pExceptionMsg );
        throw NoSupportException( errorMsg, Reference< XInterface >() );
    }
}

sal_Bool StringResourceImpl::isReadOnly()
    throw (RuntimeException)
{
    return m_bReadOnly;
}

void StringResourceImpl::implSetCurrentLocale( const Locale& locale,
    sal_Bool FindClosestMatch, sal_Bool bUseDefaultIfNoMatch )
        throw (IllegalArgumentException, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );

    LocaleItem* pLocaleItem = NULL;
    if( FindClosestMatch )
        pLocaleItem = getClosestMatchItemForLocale( locale );
    else
        pLocaleItem = getItemForLocale( locale, true );

    if( pLocaleItem == NULL && bUseDefaultIfNoMatch )
        pLocaleItem = m_pDefaultLocaleItem;

    if( pLocaleItem != NULL )
    {
        loadLocale( pLocaleItem );
        m_pCurrentLocaleItem = pLocaleItem;

        // Only notify without modifying
        implNotifyListeners();
    }
}

void StringResourceImpl::setCurrentLocale( const Locale& locale, sal_Bool FindClosestMatch )
    throw (IllegalArgumentException, RuntimeException)
{
    sal_Bool bUseDefaultIfNoMatch = false;
    implSetCurrentLocale( locale, FindClosestMatch, bUseDefaultIfNoMatch );
}

void StringResourceImpl::setDefaultLocale( const Locale& locale )
    throw (IllegalArgumentException, RuntimeException,NoSupportException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::setDefaultLocale(): Read only" );

    LocaleItem* pLocaleItem = getItemForLocale( locale, true );
    if( pLocaleItem && pLocaleItem != m_pDefaultLocaleItem )
    {
        if( m_pDefaultLocaleItem )
        {
            LocaleItem* pChangedDefaultLocaleItem = new LocaleItem( m_pDefaultLocaleItem->m_locale );
            m_aChangedDefaultLocaleVector.push_back( pChangedDefaultLocaleItem );
        }

        m_pDefaultLocaleItem = pLocaleItem;
        m_bDefaultModified = true;
        implModified();
    }
}

void StringResourceImpl::implSetString( const ::rtl::OUString& ResourceID,
    const ::rtl::OUString& Str, LocaleItem* pLocaleItem )
{
    if( pLocaleItem != NULL && loadLocale( pLocaleItem ) )
    {
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
}

void StringResourceImpl::setString( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str )
    throw (NoSupportException, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::setString(): Read only" );
    implSetString( ResourceID, Str, m_pCurrentLocaleItem );
}

void StringResourceImpl::setStringForLocale
    ( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str, const Locale& locale )
        throw (NoSupportException, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::setStringForLocale(): Read only" );
    LocaleItem* pLocaleItem = getItemForLocale( locale, false );
    implSetString( ResourceID, Str, pLocaleItem );
}

void StringResourceImpl::implRemoveId( const ::rtl::OUString& ResourceID, LocaleItem* pLocaleItem )
    throw (::com::sun::star::resource::MissingResourceException)
{
    if( pLocaleItem != NULL && loadLocale( pLocaleItem ) )
    {
        IdToStringMap& rHashMap = pLocaleItem->m_aIdToStringMap;
        IdToStringMap::iterator it = rHashMap.find( ResourceID );
        if( it == rHashMap.end() )
        {
            ::rtl::OUString errorMsg("StringResourceImpl: No entries for ResourceID: ");
            errorMsg.concat( ResourceID );
            throw ::com::sun::star::resource::MissingResourceException( errorMsg, Reference< XInterface >() );
        }
        rHashMap.erase( it );
        pLocaleItem->m_bModified = true;
        implModified();
    }
}

void StringResourceImpl::removeId( const ::rtl::OUString& ResourceID )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException, NoSupportException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::removeId(): Read only" );
    implRemoveId( ResourceID, m_pCurrentLocaleItem );
}

void StringResourceImpl::removeIdForLocale( const ::rtl::OUString& ResourceID, const Locale& locale )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException, NoSupportException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::removeIdForLocale(): Read only" );
    LocaleItem* pLocaleItem = getItemForLocale( locale, false );
    implRemoveId( ResourceID, pLocaleItem );
}

void StringResourceImpl::newLocale( const Locale& locale )
    throw (ElementExistException, IllegalArgumentException, RuntimeException, NoSupportException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::newLocale(): Read only" );

    if( getItemForLocale( locale, false ) != NULL )
    {
        ::rtl::OUString errorMsg("StringResourceImpl: locale already exists");
        throw ElementExistException( errorMsg, Reference< XInterface >() );
    }

    // TODO?: Check if locale is valid? How?
    bool bValid = true;
    if( bValid )
    {
        LocaleItem* pLocaleItem = new LocaleItem( locale );
        m_aLocaleItemVector.push_back( pLocaleItem );
        pLocaleItem->m_bModified = true;

        // Copy strings from default locale
        LocaleItem* pCopyFromItem = m_pDefaultLocaleItem;
        if( pCopyFromItem == NULL )
            pCopyFromItem = m_pCurrentLocaleItem;
        if( pCopyFromItem != NULL && loadLocale( pCopyFromItem ) )
        {
            const IdToStringMap& rSourceMap = pCopyFromItem->m_aIdToStringMap;
            IdToStringMap& rTargetMap = pLocaleItem->m_aIdToStringMap;
            IdToStringMap::const_iterator it;
            for( it = rSourceMap.begin(); it != rSourceMap.end(); ++it )
            {
                ::rtl::OUString aId  = (*it).first;
                ::rtl::OUString aStr = (*it).second;
                rTargetMap[ aId ] = aStr;
            }

            const IdToIndexMap& rSourceIndexMap = pCopyFromItem->m_aIdToIndexMap;
            IdToIndexMap& rTargetIndexMap = pLocaleItem->m_aIdToIndexMap;
            IdToIndexMap::const_iterator it_index;
            for( it_index = rSourceIndexMap.begin(); it_index != rSourceIndexMap.end(); ++it_index )
            {
                ::rtl::OUString aId  = (*it_index).first;
                sal_Int32 nIndex = (*it_index).second;
                rTargetIndexMap[ aId ] = nIndex;
            }
            pLocaleItem->m_nNextIndex = pCopyFromItem->m_nNextIndex;
        }

        if( m_pCurrentLocaleItem == NULL )
            m_pCurrentLocaleItem = pLocaleItem;

        if( m_pDefaultLocaleItem == NULL )
        {
            m_pDefaultLocaleItem = pLocaleItem;
            m_bDefaultModified = true;
        }

        implModified();
    }
    else
    {
        ::rtl::OUString errorMsg("StringResourceImpl: Invalid locale");
        throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 0 );
    }
}

void StringResourceImpl::removeLocale( const Locale& locale )
    throw (IllegalArgumentException, RuntimeException, NoSupportException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceImpl::removeLocale(): Read only" );

    LocaleItem* pRemoveItem = getItemForLocale( locale, true );
    if( pRemoveItem )
    {
        // Last locale?
        sal_Int32 nLocaleCount = m_aLocaleItemVector.size();
        if( nLocaleCount > 1 )
        {
            LocaleItem* pFallbackItem = NULL;
            if( m_pCurrentLocaleItem == pRemoveItem ||
                m_pDefaultLocaleItem  == pRemoveItem )
            {
                for( LocaleItemVectorIt it = m_aLocaleItemVector.begin(); it != m_aLocaleItemVector.end(); ++it )
                {
                    LocaleItem* pLocaleItem = *it;
                    if( pLocaleItem != pRemoveItem )
                    {
                        pFallbackItem = pLocaleItem;
                        break;
                    }
                }
                if( m_pCurrentLocaleItem == pRemoveItem )
                {
                    sal_Bool FindClosestMatch = false;
                    setCurrentLocale( pFallbackItem->m_locale, FindClosestMatch );
                }
                if( m_pDefaultLocaleItem == pRemoveItem )
                {
                    setDefaultLocale( pFallbackItem->m_locale );
                }
            }
        }
        for( LocaleItemVectorIt it = m_aLocaleItemVector.begin(); it != m_aLocaleItemVector.end(); ++it )
        {
            LocaleItem* pLocaleItem = *it;
            if( pLocaleItem == pRemoveItem )
            {
                // Remember locale item to delete file while storing
                m_aDeletedLocaleItemVector.push_back( pLocaleItem );

                // Last locale?
                if( nLocaleCount == 1 )
                {
                    m_nNextUniqueNumericId = 0;
                    if( m_pDefaultLocaleItem )
                    {
                        LocaleItem* pChangedDefaultLocaleItem = new LocaleItem( m_pDefaultLocaleItem->m_locale );
                        m_aChangedDefaultLocaleVector.push_back( pChangedDefaultLocaleItem );
                    }
                    m_pCurrentLocaleItem = NULL;
                    m_pDefaultLocaleItem = NULL;
                }

                m_aLocaleItemVector.erase( it );

                implModified();
                break;
            }
        }
    }
}

void StringResourceImpl::implScanIdForNumber( const ::rtl::OUString& ResourceID )
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
    throw (RuntimeException, NoSupportException)
{
    if( m_nNextUniqueNumericId == UNIQUE_NUMBER_NEEDS_INITIALISATION )
    {
        implLoadAllLocales();
        m_nNextUniqueNumericId = 0;
    }

    if( m_nNextUniqueNumericId < UNIQUE_NUMBER_NEEDS_INITIALISATION )
    {
        ::rtl::OUString errorMsg("getUniqueNumericId: Extended sal_Int32 range");
        throw NoSupportException( errorMsg, Reference< XInterface >() );
    }
    return m_nNextUniqueNumericId;
}


// =============================================================================
// Private helper methods

LocaleItem* StringResourceImpl::getItemForLocale
    ( const Locale& locale, sal_Bool bException )
        throw (::com::sun::star::lang::IllegalArgumentException)
{
    LocaleItem* pRetItem = NULL;

    // Search for locale
    for( LocaleItemVectorConstIt it = m_aLocaleItemVector.begin(); it != m_aLocaleItemVector.end(); ++it )
    {
        LocaleItem* pLocaleItem = *it;
        if( pLocaleItem )
        {
            Locale& cmp_locale = pLocaleItem->m_locale;
            if( cmp_locale.Language == locale.Language &&
                cmp_locale.Country  == locale.Country &&
                cmp_locale.Variant  == locale.Variant )
            {
                pRetItem = pLocaleItem;
                break;
            }
        }
    }

    if( pRetItem == NULL && bException )
    {
        ::rtl::OUString errorMsg("StringResourceImpl: Invalid locale");
        throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 0 );
    }
    return pRetItem;
}

// Returns the LocalItem for a given locale, if it exists, otherwise NULL
// This method performes a closest match search, at least the language must match
LocaleItem* StringResourceImpl::getClosestMatchItemForLocale( const Locale& locale )
{
    LocaleItem* pRetItem = NULL;

    // Search for locale
    for( sal_Int32 iPass = 0 ; iPass <= 2 ; ++iPass )
    {
        for( LocaleItemVectorConstIt it = m_aLocaleItemVector.begin(); it != m_aLocaleItemVector.end(); ++it )
        {
            LocaleItem* pLocaleItem = *it;
            if( pLocaleItem )
            {
                Locale& cmp_locale = pLocaleItem->m_locale;
                if( cmp_locale.Language == locale.Language &&
                    (iPass > 1 || cmp_locale.Country  == locale.Country) &&
                    (iPass > 0 || cmp_locale.Variant  == locale.Variant) )
                {
                    pRetItem = pLocaleItem;
                    break;
                }
            }
        }
        if( pRetItem )
            break;
    }

    return pRetItem;
}

void StringResourceImpl::implModified( void )
{
    m_bModified = true;
    implNotifyListeners();
}

void StringResourceImpl::implNotifyListeners( void )
{
    EventObject aEvent;
    aEvent.Source = static_cast< XInterface* >( (OWeakObject*)this );

    ::cppu::OInterfaceIteratorHelper it( m_aListenerContainer );
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


// =============================================================================
// Loading

bool StringResourceImpl::loadLocale( LocaleItem* pLocaleItem )
{
    // Base implementation has nothing to load
    (void)pLocaleItem;
    return true;
}

void StringResourceImpl::implLoadAllLocales( void )
{
    // Base implementation has nothing to load
}


Reference< XMultiComponentFactory > StringResourceImpl::getMultiComponentFactory( void )
{
    ::osl::MutexGuard aGuard( getMutex() );

    if( !m_xMCF.is() )
    {
        Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager(), UNO_QUERY );
        if( !xSMgr.is() )
        {
            throw RuntimeException(
                ::rtl::OUString( "StringResourceImpl::getMultiComponentFactory: Couldn't instantiate MultiComponentFactory"  ),
                    Reference< XInterface >() );
        }
        m_xMCF = xSMgr;
    }
    return m_xMCF;
}


// =============================================================================
// StringResourcePersistenceImpl
// =============================================================================

StringResourcePersistenceImpl::StringResourcePersistenceImpl( const Reference< XComponentContext >& rxContext )
    : StringResourcePersistenceImpl_BASE( rxContext )
{
}

// -----------------------------------------------------------------------------

StringResourcePersistenceImpl::~StringResourcePersistenceImpl()
{
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString StringResourcePersistenceImpl::getImplementationName(  )
    throw (RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM
        ( "com.sun.star.comp.scripting.StringResourceWithLocation") );
}

// -----------------------------------------------------------------------------

sal_Bool StringResourcePersistenceImpl::supportsService( const ::rtl::OUString& rServiceName )
    throw (RuntimeException)
{
    return StringResourceImpl::supportsService( rServiceName );
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > StringResourcePersistenceImpl::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return StringResourceImpl::getSupportedServiceNames();
}

// -----------------------------------------------------------------------------
// XInitialization base functionality for derived classes
// -----------------------------------------------------------------------------

static ::rtl::OUString aNameBaseDefaultStr("strings");

void StringResourcePersistenceImpl::implInitializeCommonParameters
    ( const Sequence< Any >& aArguments )
        throw (Exception, RuntimeException)
{
    bool bReadOnlyOk = (aArguments[1] >>= m_bReadOnly);
    if( !bReadOnlyOk )
    {
        ::rtl::OUString errorMsg("XInitialization::initialize: Expected ReadOnly flag");
        throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 1 );
    }

    com::sun::star::lang::Locale aCurrentLocale;
    bool bLocaleOk = (aArguments[2] >>= aCurrentLocale);
    if( !bLocaleOk )
    {
        ::rtl::OUString errorMsg("XInitialization::initialize: Expected Locale");
        throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 2 );
    }

    bool bNameBaseOk = (aArguments[3] >>= m_aNameBase);
    if( !bNameBaseOk )
    {
        ::rtl::OUString errorMsg("XInitialization::initialize: Expected NameBase string");
        throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 3 );
    }
    if( m_aNameBase.isEmpty() )
        m_aNameBase = aNameBaseDefaultStr;

    bool bCommentOk = (aArguments[4] >>= m_aComment);
    if( !bCommentOk )
    {
        ::rtl::OUString errorMsg("XInitialization::initialize: Expected Comment string");
        throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 4 );
    }

    implScanLocales();

    sal_Bool FindClosestMatch = true;
    sal_Bool bUseDefaultIfNoMatch = true;
    implSetCurrentLocale( aCurrentLocale, FindClosestMatch, bUseDefaultIfNoMatch );
}

// -----------------------------------------------------------------------------
// Forwarding calls to base class

// XModifyBroadcaster
void StringResourcePersistenceImpl::addModifyListener( const Reference< XModifyListener >& aListener )
    throw (RuntimeException)
{
    StringResourceImpl::addModifyListener( aListener );
}
void StringResourcePersistenceImpl::removeModifyListener( const Reference< XModifyListener >& aListener )
    throw (RuntimeException)
{
    StringResourceImpl::removeModifyListener( aListener );
}

// XStringResourceResolver
::rtl::OUString StringResourcePersistenceImpl::resolveString( const ::rtl::OUString& ResourceID )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException)
{
    return StringResourceImpl::resolveString( ResourceID ) ;
}
::rtl::OUString StringResourcePersistenceImpl::resolveStringForLocale( const ::rtl::OUString& ResourceID, const Locale& locale )
    throw ( ::com::sun::star::resource::MissingResourceException, RuntimeException)
{
    return StringResourceImpl::resolveStringForLocale( ResourceID, locale );
}
sal_Bool StringResourcePersistenceImpl::hasEntryForId( const ::rtl::OUString& ResourceID )
    throw (RuntimeException)
{
    return StringResourceImpl::hasEntryForId( ResourceID ) ;
}
sal_Bool StringResourcePersistenceImpl::hasEntryForIdAndLocale( const ::rtl::OUString& ResourceID,
    const Locale& locale )
        throw (RuntimeException)
{
    return StringResourceImpl::hasEntryForIdAndLocale( ResourceID, locale );
}
Locale StringResourcePersistenceImpl::getCurrentLocale()
    throw (RuntimeException)
{
    return StringResourceImpl::getCurrentLocale();
}
Locale StringResourcePersistenceImpl::getDefaultLocale(  )
    throw (RuntimeException)
{
    return StringResourceImpl::getDefaultLocale();
}
Sequence< Locale > StringResourcePersistenceImpl::getLocales(  )
    throw (RuntimeException)
{
    return StringResourceImpl::getLocales();
}

// XStringResourceManager
sal_Bool StringResourcePersistenceImpl::isReadOnly()
    throw (RuntimeException)
{
    return StringResourceImpl::isReadOnly();
}
void StringResourcePersistenceImpl::setCurrentLocale( const Locale& locale, sal_Bool FindClosestMatch )
    throw (IllegalArgumentException, RuntimeException)
{
    StringResourceImpl::setCurrentLocale( locale, FindClosestMatch );
}
void StringResourcePersistenceImpl::setDefaultLocale( const Locale& locale )
    throw (IllegalArgumentException, RuntimeException,NoSupportException)
{
    StringResourceImpl::setDefaultLocale( locale );
}
Sequence< ::rtl::OUString > StringResourcePersistenceImpl::getResourceIDs(  )
    throw (RuntimeException)
{
    return StringResourceImpl::getResourceIDs();
}
void StringResourcePersistenceImpl::setString( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str )
    throw (NoSupportException, RuntimeException)
{
    StringResourceImpl::setString( ResourceID, Str );
}
void StringResourcePersistenceImpl::setStringForLocale
    ( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str, const Locale& locale )
        throw (NoSupportException, RuntimeException)
{
    StringResourceImpl::setStringForLocale( ResourceID, Str, locale );
}
Sequence< ::rtl::OUString > StringResourcePersistenceImpl::getResourceIDsForLocale
    ( const Locale& locale ) throw (::com::sun::star::uno::RuntimeException)
{
    return StringResourceImpl::getResourceIDsForLocale( locale );
}
void StringResourcePersistenceImpl::removeId( const ::rtl::OUString& ResourceID )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException, NoSupportException)
{
    StringResourceImpl::removeId( ResourceID );
}
void StringResourcePersistenceImpl::removeIdForLocale( const ::rtl::OUString& ResourceID, const Locale& locale )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException, NoSupportException)
{
    StringResourceImpl::removeIdForLocale( ResourceID, locale );
}
void StringResourcePersistenceImpl::newLocale( const Locale& locale )
    throw (ElementExistException, IllegalArgumentException, RuntimeException, NoSupportException)
{
    StringResourceImpl::newLocale( locale );
}
void StringResourcePersistenceImpl::removeLocale( const Locale& locale )
    throw (IllegalArgumentException, RuntimeException, NoSupportException)
{
    StringResourceImpl::removeLocale( locale );
}
sal_Int32 StringResourcePersistenceImpl::getUniqueNumericId(  )
    throw (RuntimeException, NoSupportException)
{
    return StringResourceImpl::getUniqueNumericId();
}

// -----------------------------------------------------------------------------
// XStringResourcePersistence

void StringResourcePersistenceImpl::store()
    throw (NoSupportException, Exception, RuntimeException)
{
}

sal_Bool StringResourcePersistenceImpl::isModified(  )
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );

    return m_bModified;
}

void StringResourcePersistenceImpl::setComment( const ::rtl::OUString& Comment )
    throw (::com::sun::star::uno::RuntimeException)
{
    m_aComment = Comment;
}

void StringResourcePersistenceImpl::storeToStorage( const Reference< XStorage >& Storage,
    const ::rtl::OUString& NameBase, const ::rtl::OUString& Comment )
        throw (Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );

    bool bUsedForStore = false;
    bool bStoreAll = true;
    implStoreAtStorage( NameBase, Comment, Storage, bUsedForStore, bStoreAll );
}

void StringResourcePersistenceImpl::implStoreAtStorage
(
    const ::rtl::OUString& aNameBase,
    const ::rtl::OUString& aComment,
    const Reference< ::com::sun::star::embed::XStorage >& Storage,
    bool bUsedForStore,
    bool bStoreAll
)
    throw (Exception, RuntimeException)
{
    // Delete files for deleted locales
    if( bUsedForStore )
    {
        while( m_aDeletedLocaleItemVector.size() > 0 )
        {
            LocaleItemVectorIt it = m_aDeletedLocaleItemVector.begin();
            LocaleItem* pLocaleItem = *it;
            if( pLocaleItem != NULL )
            {
                ::rtl::OUString aStreamName = implGetFileNameForLocaleItem( pLocaleItem, m_aNameBase );
                aStreamName += ::rtl::OUString(".properties");

                try
                {
                    Storage->removeElement( aStreamName );
                }
                catch( Exception& )
                {}

                m_aDeletedLocaleItemVector.erase( it );
                delete pLocaleItem;
            }
        }
    }

    for( LocaleItemVectorConstIt it = m_aLocaleItemVector.begin(); it != m_aLocaleItemVector.end(); ++it )
    {
        LocaleItem* pLocaleItem = *it;
        if( pLocaleItem != NULL && (bStoreAll || pLocaleItem->m_bModified) &&
            loadLocale( pLocaleItem ) )
        {
            ::rtl::OUString aStreamName = implGetFileNameForLocaleItem( pLocaleItem, aNameBase );
            aStreamName += ::rtl::OUString(".properties");

            Reference< io::XStream > xElementStream =
                    Storage->openStreamElement( aStreamName, ElementModes::READWRITE );

            ::rtl::OUString aPropName("MediaType");
            ::rtl::OUString aMime("text/plain");

            uno::Reference< beans::XPropertySet > xProps( xElementStream, uno::UNO_QUERY );
            OSL_ENSURE( xProps.is(), "The StorageStream must implement XPropertySet interface!\n" );
            if ( xProps.is() )
            {
                xProps->setPropertyValue( aPropName, uno::makeAny( aMime ) );

                aPropName = ::rtl::OUString("UseCommonStoragePasswordEncryption");
                xProps->setPropertyValue( aPropName, uno::makeAny( sal_True ) );
            }

            Reference< io::XOutputStream > xOutputStream = xElementStream->getOutputStream();
            if( xOutputStream.is() )
                implWritePropertiesFile( pLocaleItem, xOutputStream, aComment );
            xOutputStream->closeOutput();

            if( bUsedForStore )
                pLocaleItem->m_bModified = false;
        }
    }

    // Delete files for changed defaults
    if( bUsedForStore )
    {
        for( LocaleItemVectorIt it = m_aChangedDefaultLocaleVector.begin();
             it != m_aChangedDefaultLocaleVector.end(); ++it )
        {
            LocaleItem* pLocaleItem = *it;
            if( pLocaleItem != NULL )
            {
                ::rtl::OUString aStreamName = implGetFileNameForLocaleItem( pLocaleItem, m_aNameBase );
                aStreamName += ::rtl::OUString(".default");

                try
                {
                    Storage->removeElement( aStreamName );
                }
                catch( Exception& )
                {}

                delete pLocaleItem;
            }
        }
        m_aChangedDefaultLocaleVector.clear();
    }

    // Default locale
    if( m_pDefaultLocaleItem != NULL && (bStoreAll || m_bDefaultModified) )
    {
        ::rtl::OUString aStreamName = implGetFileNameForLocaleItem( m_pDefaultLocaleItem, aNameBase );
        aStreamName += ::rtl::OUString(".default");

        Reference< io::XStream > xElementStream =
                Storage->openStreamElement( aStreamName, ElementModes::READWRITE );

        ::rtl::OUString aPropName("MediaType");
        ::rtl::OUString aMime("text/plain");

        // Only create stream without content
        Reference< io::XOutputStream > xOutputStream = xElementStream->getOutputStream();
        xOutputStream->closeOutput();

        if( bUsedForStore )
            m_bDefaultModified = false;
    }
}

void StringResourcePersistenceImpl::storeToURL( const ::rtl::OUString& URL,
    const ::rtl::OUString& NameBase, const ::rtl::OUString& Comment,
    const Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
        throw (Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );

    bool bUsedForStore = false;
    bool bStoreAll = true;

    Reference< ucb::XSimpleFileAccess2 > xFileAccess = ucb::SimpleFileAccess::create(m_xContext);
    if( xFileAccess.is() && Handler.is() )
        xFileAccess->setInteractionHandler( Handler );

    implStoreAtLocation( URL, NameBase, Comment, xFileAccess, bUsedForStore, bStoreAll );
}

void StringResourcePersistenceImpl::implKillRemovedLocaleFiles
(
    const ::rtl::OUString& Location,
    const ::rtl::OUString& aNameBase,
    const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess2 >& xFileAccess
)
    throw (Exception, RuntimeException)
{
    // Delete files for deleted locales
    while( m_aDeletedLocaleItemVector.size() > 0 )
    {
        LocaleItemVectorIt it = m_aDeletedLocaleItemVector.begin();
        LocaleItem* pLocaleItem = *it;
        if( pLocaleItem != NULL )
        {
            ::rtl::OUString aCompleteFileName =
                implGetPathForLocaleItem( pLocaleItem, aNameBase, Location );
            if( xFileAccess->exists( aCompleteFileName ) )
                xFileAccess->kill( aCompleteFileName );

            m_aDeletedLocaleItemVector.erase( it );
            delete pLocaleItem;
        }
    }
}

void StringResourcePersistenceImpl::implKillChangedDefaultFiles
(
    const ::rtl::OUString& Location,
    const ::rtl::OUString& aNameBase,
    const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess2 >& xFileAccess
)
    throw (Exception, RuntimeException)
{
    // Delete files for changed defaults
    for( LocaleItemVectorIt it = m_aChangedDefaultLocaleVector.begin();
         it != m_aChangedDefaultLocaleVector.end(); ++it )
    {
        LocaleItem* pLocaleItem = *it;
        if( pLocaleItem != NULL )
        {
            ::rtl::OUString aCompleteFileName =
                implGetPathForLocaleItem( pLocaleItem, aNameBase, Location, true );
            if( xFileAccess->exists( aCompleteFileName ) )
                xFileAccess->kill( aCompleteFileName );

            delete pLocaleItem;
        }
    }
    m_aChangedDefaultLocaleVector.clear();
}

void StringResourcePersistenceImpl::implStoreAtLocation
(
    const ::rtl::OUString& Location,
    const ::rtl::OUString& aNameBase,
    const ::rtl::OUString& aComment,
    const Reference< ucb::XSimpleFileAccess2 >& xFileAccess,
    bool bUsedForStore,
    bool bStoreAll,
    bool bKillAll
)
    throw (Exception, RuntimeException)
{
    // Delete files for deleted locales
    if( bUsedForStore || bKillAll )
        implKillRemovedLocaleFiles( Location, aNameBase, xFileAccess );

    for( LocaleItemVectorConstIt it = m_aLocaleItemVector.begin(); it != m_aLocaleItemVector.end(); ++it )
    {
        LocaleItem* pLocaleItem = *it;
        if( pLocaleItem != NULL && (bStoreAll || bKillAll || pLocaleItem->m_bModified) &&
            loadLocale( pLocaleItem ) )
        {
            ::rtl::OUString aCompleteFileName =
                implGetPathForLocaleItem( pLocaleItem, aNameBase, Location );
            if( xFileAccess->exists( aCompleteFileName ) )
                xFileAccess->kill( aCompleteFileName );

            if( !bKillAll )
            {
                // Create Output stream
                Reference< io::XOutputStream > xOutputStream = xFileAccess->openFileWrite( aCompleteFileName );
                if( xOutputStream.is() )
                {
                    implWritePropertiesFile( pLocaleItem, xOutputStream, aComment );
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
    if( m_pDefaultLocaleItem != NULL && (bStoreAll || bKillAll || m_bDefaultModified) )
    {
        ::rtl::OUString aCompleteFileName =
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
}


// -----------------------------------------------------------------------------
// BinaryOutput, helper class for exportBinary

class BinaryOutput
{
    Reference< XMultiComponentFactory >     m_xMCF;
    Reference< XComponentContext >          m_xContext;
    Reference< XInterface >                 m_xTempFile;
    Reference< io::XOutputStream >          m_xOutputStream;

public:
    BinaryOutput( Reference< XMultiComponentFactory > xMCF,
        Reference< XComponentContext > xContext );

    Reference< io::XOutputStream > getOutputStream() const
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
    void writeString( const ::rtl::OUString& aStr );
};

BinaryOutput::BinaryOutput( Reference< XMultiComponentFactory > xMCF,
    Reference< XComponentContext > xContext )
        : m_xMCF( xMCF )
        , m_xContext( xContext )
{
    m_xTempFile = io::TempFile::create( m_xContext );
    m_xOutputStream = Reference< io::XOutputStream >( m_xTempFile, UNO_QUERY_THROW );
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

void BinaryOutput::writeString( const ::rtl::OUString& aStr )
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

    sal_Int32 nSize = (sal_Int32)xSeekable->getPosition();

    Reference< io::XInputStream> xInputStream( m_xTempFile, UNO_QUERY );
    if( !xInputStream.is() )
        return aRetSeq;

    xSeekable->seek( 0 );
    sal_Int32 nRead = xInputStream->readBytes( aRetSeq, nSize );
    (void)nRead;
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
    throw (RuntimeException)
{
    Reference< XMultiComponentFactory > xMCF = getMultiComponentFactory();
    BinaryOutput aOut( xMCF, m_xContext );

    sal_Int32 nLocaleCount = m_aLocaleItemVector.size();
    Sequence< sal_Int8 >* pLocaleDataSeq = new Sequence< sal_Int8 >[ nLocaleCount ];

    sal_Int32 iLocale = 0;
    sal_Int32 iDefault = 0;
    for( LocaleItemVectorConstIt it = m_aLocaleItemVector.begin();
         it != m_aLocaleItemVector.end(); ++it,++iLocale )
    {
        LocaleItem* pLocaleItem = *it;
        if( pLocaleItem != NULL && loadLocale( pLocaleItem ) )
        {
            if( m_pDefaultLocaleItem == pLocaleItem )
                iDefault = iLocale;

            BinaryOutput aLocaleOut( m_xMCF, m_xContext );
            implWriteLocaleBinary( pLocaleItem, aLocaleOut );

            pLocaleDataSeq[iLocale] = aLocaleOut.closeAndGetData();
        }
    }

    // Write header
    sal_Int16 nVersion = 0;
    sal_Int16 nLocaleCount16 = (sal_Int16)nLocaleCount;
    sal_Int16 iDefault16 = (sal_Int16)iDefault;
    aOut.writeInt16( nVersion );
    aOut.writeInt16( nLocaleCount16 );
    aOut.writeInt16( iDefault16 );

    // Write data positions
    sal_Int32 nDataPos = 6 + 4 * (nLocaleCount + 1);
    for( iLocale = 0; iLocale < nLocaleCount; iLocale++ )
    {
        aOut.writeInt32( nDataPos );

        Sequence< sal_Int8 >& rSeq = pLocaleDataSeq[iLocale];
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
            Sequence< sal_Int8 >& rSeq = pLocaleDataSeq[iLocale];
            xOutputStream->writeBytes( rSeq );
        }
    }

    delete[] pLocaleDataSeq;

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

// -----------------------------------------------------------------------------
// BinaryOutput, helper class for exportBinary

class BinaryInput
{
    Sequence< sal_Int8 >                    m_aData;
    Reference< XMultiComponentFactory >     m_xMCF;
    Reference< XComponentContext >          m_xContext;

    const sal_Int8*                         m_pData;
    sal_Int32                               m_nCurPos;
    sal_Int32                               m_nSize;

public:
    BinaryInput( Sequence< ::sal_Int8 > aData, Reference< XMultiComponentFactory > xMCF,
        Reference< XComponentContext > xContext );

    Reference< io::XInputStream > getInputStreamForSection( sal_Int32 nSize );

    void seek( sal_Int32 nPos );
    sal_Int32 getPosition( void ) const
        { return m_nCurPos; }

    sal_Int16 readInt16( void );
    sal_Int32 readInt32( void );
    sal_Unicode readUnicodeChar( void );
    ::rtl::OUString readString( void );
};

BinaryInput::BinaryInput( Sequence< ::sal_Int8 > aData, Reference< XMultiComponentFactory > xMCF,
    Reference< XComponentContext > xContext )
        : m_aData( aData )
        , m_xMCF( xMCF )
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

        xIn = Reference< io::XInputStream>( xTempOut, UNO_QUERY );
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


sal_Int16 BinaryInput::readInt16( void )
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

sal_Int32 BinaryInput::readInt32( void )
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

sal_Unicode BinaryInput::readUnicodeChar( void )
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

::rtl::OUString BinaryInput::readString( void )
{
    ::rtl::OUStringBuffer aBuf;
    sal_Unicode c;
    do
    {
        c = readUnicodeChar();
        if( c != 0 )
            aBuf.append( c );
    }
    while( c != 0 );

    ::rtl::OUString aRetStr = aBuf.makeStringAndClear();
    return aRetStr;
}

void StringResourcePersistenceImpl::importBinary( const Sequence< ::sal_Int8 >& Data )
    throw (IllegalArgumentException, RuntimeException)
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
    Reference< XMultiComponentFactory > xMCF = getMultiComponentFactory();
    BinaryInput aIn( Data, xMCF, m_xContext );

    sal_Int32 nVersion = aIn.readInt16();
    (void)nVersion;
    sal_Int32 nLocaleCount = aIn.readInt16();
    sal_Int32 iDefault = aIn.readInt16();
    (void)iDefault;

    sal_Int32* pPositions = new sal_Int32[nLocaleCount + 1];
    for( sal_Int32 i = 0; i < nLocaleCount + 1; i++ )
        pPositions[i] = aIn.readInt32();

    // Import locales
    LocaleItem* pUseAsDefaultItem = NULL;
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
            m_aLocaleItemVector.push_back( pLocaleItem );
            implReadPropertiesFile( pLocaleItem, xInput );
        }
    }

    if( pUseAsDefaultItem != NULL )
        setDefaultLocale( pUseAsDefaultItem->m_locale );

    delete[] pPositions;
}


// =============================================================================
// Private helper methods

bool checkNamingSceme( const ::rtl::OUString& aName, const ::rtl::OUString& aNameBase,
                       Locale& aLocale )
{
    bool bSuccess = false;

    sal_Int32 nNameLen = aName.getLength();
    sal_Int32 nNameBaseLen = aNameBase.getLength();

    // Name has to start with NameBase followed
    // by a '_' and at least one more character
    if( aName.indexOf( aNameBase ) == 0 && nNameBaseLen < nNameLen-1 &&
        aName.getStr()[nNameBaseLen] == '_' )
    {
        bSuccess = true;

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

void StringResourcePersistenceImpl::implLoadAllLocales( void )
{
    for( LocaleItemVectorIt it = m_aLocaleItemVector.begin(); it != m_aLocaleItemVector.end(); ++it )
    {
        LocaleItem* pLocaleItem = *it;
        if( pLocaleItem != NULL )
            loadLocale( pLocaleItem );
    }
}

// Scan locale properties files helper
void StringResourcePersistenceImpl::implScanLocaleNames( const Sequence< ::rtl::OUString >& aContentSeq )
{
    Locale aDefaultLocale;
    bool bDefaultFound = false;

    sal_Int32 nCount = aContentSeq.getLength();
    const ::rtl::OUString* pFiles = aContentSeq.getConstArray();
    for( int i = 0 ; i < nCount ; i++ )
    {
        ::rtl::OUString aCompleteName = pFiles[i];
        rtl::OUString aPureName;
        rtl::OUString aExtension;
        sal_Int32 iDot = aCompleteName.lastIndexOf( '.' );
        sal_Int32 iSlash = aCompleteName.lastIndexOf( '/' );
        if( iDot != -1 )
        {
            sal_Int32 iCopyFrom = (iSlash != -1) ? iSlash + 1 : 0;
            aPureName = aCompleteName.copy( iCopyFrom, iDot-iCopyFrom );
            aExtension = aCompleteName.copy( iDot + 1 );
        }

        if ( aExtension == "properties" )
        {
            //rtl::OUString aName = aInetObj.getBase();
            Locale aLocale;

            if( checkNamingSceme( aPureName, m_aNameBase, aLocale ) )
            {
                LocaleItem* pLocaleItem = new LocaleItem( aLocale, false );
                m_aLocaleItemVector.push_back( pLocaleItem );

                if( m_pCurrentLocaleItem == NULL )
                    m_pCurrentLocaleItem = pLocaleItem;

                if( m_pDefaultLocaleItem == NULL )
                {
                    m_pDefaultLocaleItem = pLocaleItem;
                    m_bDefaultModified = true;
                }
            }
        }
        else if( !bDefaultFound && aExtension == "default" )
        {
            //rtl::OUString aName = aInetObj.getBase();
            Locale aLocale;

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
void StringResourcePersistenceImpl::implScanLocales( void )
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

::rtl::OUString implGetNameScemeForLocaleItem( const LocaleItem* pLocaleItem )
{
    static ::rtl::OUString aUnder("_");

    OSL_ENSURE( pLocaleItem,
        "StringResourcePersistenceImpl::implGetNameScemeForLocaleItem(): pLocaleItem == NULL" );
    Locale aLocale = pLocaleItem->m_locale;

    ::rtl::OUString aRetStr = aUnder;
    aRetStr += aLocale.Language;

    ::rtl::OUString aCountry  = aLocale.Country;
    if( !aCountry.isEmpty() )
    {
        aRetStr += aUnder;
        aRetStr += aCountry;
    }

    ::rtl::OUString aVariant  = aLocale.Variant;
    if( !aVariant.isEmpty() )
    {
        aRetStr += aUnder;
        aRetStr += aVariant;
    }
    return aRetStr;
}

::rtl::OUString StringResourcePersistenceImpl::implGetFileNameForLocaleItem
    ( LocaleItem* pLocaleItem, const ::rtl::OUString& aNameBase )
{
    ::rtl::OUString aFileName = aNameBase;
    if( aFileName.isEmpty() )
        aFileName = aNameBaseDefaultStr;

    aFileName += implGetNameScemeForLocaleItem( pLocaleItem );
    return aFileName;
}

::rtl::OUString StringResourcePersistenceImpl::implGetPathForLocaleItem
    ( LocaleItem* pLocaleItem, const ::rtl::OUString& aNameBase,
      const ::rtl::OUString& aLocation, bool bDefaultFile )
{
    ::rtl::OUString aFileName = implGetFileNameForLocaleItem( pLocaleItem, aNameBase );
    INetURLObject aInetObj( aLocation );
    aInetObj.insertName( aFileName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
    if( bDefaultFile )
        aInetObj.setExtension( ::rtl::OUString( "default" ) );
    else
        aInetObj.setExtension( ::rtl::OUString( "properties" ) );
    ::rtl::OUString aCompleteFileName = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
    return aCompleteFileName;
}

// White space according to Java property files specification in
// http://java.sun.com/j2se/1.4.2/docs/api/java/util/Properties.html#load(java.io.InputStream)
inline bool isWhiteSpace( sal_Unicode c )
{
    bool bWhite = ( c == 0x0020 ||      // space
                    c == 0x0009 ||      // tab
                    c == 0x000a ||      // line feed, not always handled by TextInputStream
                    c == 0x000d ||      // carriage return, not always handled by TextInputStream
                    c == 0x000C );      // form feed
    return bWhite;
}

inline void skipWhites( const sal_Unicode* pBuf, sal_Int32 nLen, sal_Int32& ri )
{
    while( ri < nLen )
    {
        if( !isWhiteSpace( pBuf[ri] ) )
            break;
        ri++;
    }
}

inline bool isHexDigit( sal_Unicode c, sal_uInt16& nDigitVal )
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

sal_Unicode getEscapeChar( const sal_Unicode* pBuf, sal_Int32 nLen, sal_Int32& ri )
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

void CheckContinueInNextLine( Reference< io::XTextInputStream > xTextInputStream,
    ::rtl::OUString& aLine, bool& bEscapePending, const sal_Unicode*& pBuf,
    sal_Int32& nLen, sal_Int32& i )
{
    if( i == nLen && bEscapePending )
    {
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
}

bool StringResourcePersistenceImpl::implReadPropertiesFile
    ( LocaleItem* pLocaleItem, const Reference< io::XInputStream >& xInputStream )
{
    if( !xInputStream.is() || pLocaleItem == NULL )
        return false;

    bool bSuccess = false;
    Reference< XMultiComponentFactory > xMCF = getMultiComponentFactory();
    Reference< io::XTextInputStream > xTextInputStream( xMCF->createInstanceWithContext
        ( ::rtl::OUString("com.sun.star.io.TextInputStream"), m_xContext ), UNO_QUERY );

    if( xTextInputStream.is() )
    {
        Reference< io::XActiveDataSink> xActiveDataSink( xTextInputStream, UNO_QUERY );
        if( xActiveDataSink.is() )
        {
            bSuccess = true;

            xActiveDataSink->setInputStream( xInputStream );

            ::rtl::OUString aEncodingStr = ::rtl::OUString::createFromAscii
                ( rtl_getMimeCharsetFromTextEncoding( RTL_TEXTENCODING_ISO_8859_1 ) );
            xTextInputStream->setEncoding( aEncodingStr );

            ::rtl::OUString aLine;
            while( !xTextInputStream->isEOF() )
            {
                aLine = xTextInputStream->readLine();

                sal_Int32 nLen = aLine.getLength();
                if( 0 == nLen )
                    continue;
                const sal_Unicode* pBuf = aLine.getStr();
                ::rtl::OUStringBuffer aBuf;
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
                ::rtl::OUString aResourceID;
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

                ::rtl::OUString aValueStr;
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
        }
    }

    return bSuccess;
}


inline sal_Unicode getHexCharForDigit( sal_uInt16 nDigitVal )
{
    sal_Unicode cRet = ( nDigitVal < 10 ) ? ('0' + nDigitVal) : ('a' + (nDigitVal-10));
    return cRet;
}

void implWriteCharToBuffer( ::rtl::OUStringBuffer& aBuf, sal_Unicode cu, bool bKey )
{
    if( cu == '\\' )
    {
        aBuf.append( (sal_Unicode)'\\' );
        aBuf.append( (sal_Unicode)'\\' );
    }
    else if( cu == 0x000a )
    {
        aBuf.append( (sal_Unicode)'\\' );
        aBuf.append( (sal_Unicode)'n' );
    }
    else if( cu == 0x000d )
    {
        aBuf.append( (sal_Unicode)'\\' );
        aBuf.append( (sal_Unicode)'r' );
    }
    else if( bKey && cu == '=' )
    {
        aBuf.append( (sal_Unicode)'\\' );
        aBuf.append( (sal_Unicode)'=' );
    }
    else if( bKey && cu == ':' )
    {
        aBuf.append( (sal_Unicode)'\\' );
        aBuf.append( (sal_Unicode)':' );
    }
    // ISO/IEC 8859-1 range according to:
    // http://en.wikipedia.org/wiki/ISO/IEC_8859-1
    else if( (cu >= 0x20 && cu <= 0x7e) )
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
        aBuf.append( (sal_Unicode)'\\' );
        aBuf.append( (sal_Unicode)'u' );

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

void implWriteStringWithEncoding( const ::rtl::OUString& aStr,
    Reference< io::XTextOutputStream > xTextOutputStream, bool bKey )
{
    static sal_Unicode cLineFeed = 0xa;

    (void)aStr;
    (void)xTextOutputStream;

    ::rtl::OUStringBuffer aBuf;
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

    ::rtl::OUString aWriteStr = aBuf.makeStringAndClear();
    xTextOutputStream->writeString( aWriteStr );
}

bool StringResourcePersistenceImpl::implWritePropertiesFile( LocaleItem* pLocaleItem,
    const Reference< io::XOutputStream >& xOutputStream, const ::rtl::OUString& aComment )
{
    static ::rtl::OUString aAssignmentStr("=");
    static ::rtl::OUString aLineFeedStr("\n");

    if( !xOutputStream.is() || pLocaleItem == NULL )
        return false;

    bool bSuccess = false;
    Reference< XMultiComponentFactory > xMCF = getMultiComponentFactory();
    Reference< io::XTextOutputStream > xTextOutputStream( xMCF->createInstanceWithContext
        ( ::rtl::OUString("com.sun.star.io.TextOutputStream"), m_xContext ), UNO_QUERY );

    if( xTextOutputStream.is() )
    {
        Reference< io::XActiveDataSource> xActiveDataSource( xTextOutputStream, UNO_QUERY );
        if( xActiveDataSource.is() )
        {
            xActiveDataSource->setOutputStream( xOutputStream );

            ::rtl::OUString aEncodingStr = ::rtl::OUString::createFromAscii
                ( rtl_getMimeCharsetFromTextEncoding( RTL_TEXTENCODING_ISO_8859_1 ) );
            xTextOutputStream->setEncoding( aEncodingStr );

            xTextOutputStream->writeString( aComment );
            xTextOutputStream->writeString( aLineFeedStr );

            const IdToStringMap& rHashMap = pLocaleItem->m_aIdToStringMap;
            if( rHashMap.size() > 0 )
            {
                // Sort ids according to read order
                const IdToIndexMap& rIndexMap = pLocaleItem->m_aIdToIndexMap;
                IdToIndexMap::const_iterator it_index;

                // Find max/min index
                sal_Int32 nMinIndex = -1;
                sal_Int32 nMaxIndex = -1;
                for( it_index = rIndexMap.begin(); it_index != rIndexMap.end(); ++it_index )
                {
                    sal_Int32 nIndex = (*it_index).second;
                    if( nMinIndex > nIndex || nMinIndex == -1 )
                        nMinIndex = nIndex;
                    if( nMaxIndex < nIndex )
                        nMaxIndex = nIndex;
                }
                sal_Int32 nTabSize = nMaxIndex - nMinIndex + 1;

                // Create sorted array of pointers to the id strings
                const ::rtl::OUString** pIdPtrs = new const ::rtl::OUString*[nTabSize];
                sal_Int32 i;
                for( i = 0 ; i < nTabSize ; i++ )
                    pIdPtrs[i] = NULL;
                for( it_index = rIndexMap.begin(); it_index != rIndexMap.end(); ++it_index )
                {
                    sal_Int32 nIndex = (*it_index).second;
                    pIdPtrs[nIndex - nMinIndex] = &((*it_index).first);
                }

                // Write lines in correct order
                for( i = 0 ; i < nTabSize ; i++ )
                {
                    const ::rtl::OUString* pStr = pIdPtrs[i];
                    if( pStr != NULL )
                    {
                        ::rtl::OUString aResourceID = *pStr;
                        IdToStringMap::const_iterator it = rHashMap.find( aResourceID );
                        if( !( it == rHashMap.end() ) )
                        {
                            implWriteStringWithEncoding( aResourceID, xTextOutputStream, true );
                            xTextOutputStream->writeString( aAssignmentStr );
                            ::rtl::OUString aValStr = (*it).second;
                            implWriteStringWithEncoding( aValStr, xTextOutputStream, false );
                        }
                    }
                }

                delete pIdPtrs;
            }

            bSuccess = true;
        }
    }
    return bSuccess;
}


// =============================================================================
// StringResourceWithStorageImpl
// =============================================================================

// component operations
static Sequence< ::rtl::OUString > getSupportedServiceNames_StringResourceWithStorageImpl()
{
    Sequence< ::rtl::OUString > names(1);
    names[0] = ::rtl::OUString( "com.sun.star.resource.StringResourceWithStorage" );
    return names;
}

static ::rtl::OUString getImplementationName_StringResourceWithStorageImpl()
{
    return ::rtl::OUString( "com.sun.star.comp.scripting.StringResourceWithStorage" );
}

static Reference< XInterface > SAL_CALL create_StringResourceWithStorageImpl(
    Reference< XComponentContext > const & xContext )
    SAL_THROW(())
{
    return static_cast< ::cppu::OWeakObject * >( new StringResourceWithStorageImpl( xContext ) );
}

// -----------------------------------------------------------------------------

StringResourceWithStorageImpl::StringResourceWithStorageImpl( const Reference< XComponentContext >& rxContext )
    : StringResourceWithStorageImpl_BASE( rxContext )
    , m_bStorageChanged( false )
{
}

// -----------------------------------------------------------------------------

StringResourceWithStorageImpl::~StringResourceWithStorageImpl()
{
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString StringResourceWithStorageImpl::getImplementationName(  ) throw (RuntimeException)
{
    return getImplementationName_StringResourceWithStorageImpl();
}

// -----------------------------------------------------------------------------

sal_Bool StringResourceWithStorageImpl::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
    const ::rtl::OUString* pNames = aNames.getConstArray();
    const ::rtl::OUString* pEnd = pNames + aNames.getLength();
    for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
        ;

    return pNames != pEnd;
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > StringResourceWithStorageImpl::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return getSupportedServiceNames_StringResourceWithStorageImpl();
}

// -----------------------------------------------------------------------------
// XInitialization
// -----------------------------------------------------------------------------

void StringResourceWithStorageImpl::initialize( const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );

    if ( aArguments.getLength() != 5 )
    {
        throw RuntimeException(
            ::rtl::OUString( "StringResourceWithStorageImpl::initialize: invalid number of arguments!"  ),
            Reference< XInterface >() );
    }

    bool bOk = (aArguments[0] >>= m_xStorage);
    if( bOk && !m_xStorage.is() )
        bOk = false;

    if( !bOk )
    {
        ::rtl::OUString errorMsg("StringResourceWithStorageImpl::initialize: invalid storage");
        throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 0 );
    }

    implInitializeCommonParameters( aArguments );
}

// -----------------------------------------------------------------------------
// Forwarding calls to base class

// XModifyBroadcaster
void StringResourceWithStorageImpl::addModifyListener( const Reference< XModifyListener >& aListener )
    throw (RuntimeException)
{
    StringResourceImpl::addModifyListener( aListener );
}
void StringResourceWithStorageImpl::removeModifyListener( const Reference< XModifyListener >& aListener )
    throw (RuntimeException)
{
    StringResourceImpl::removeModifyListener( aListener );
}

// XStringResourceResolver
::rtl::OUString StringResourceWithStorageImpl::resolveString( const ::rtl::OUString& ResourceID )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException)
{
    return StringResourceImpl::resolveString( ResourceID ) ;
}
::rtl::OUString StringResourceWithStorageImpl::resolveStringForLocale( const ::rtl::OUString& ResourceID, const Locale& locale )
    throw ( ::com::sun::star::resource::MissingResourceException, RuntimeException)
{
    return StringResourceImpl::resolveStringForLocale( ResourceID, locale );
}
sal_Bool StringResourceWithStorageImpl::hasEntryForId( const ::rtl::OUString& ResourceID )
    throw (RuntimeException)
{
    return StringResourceImpl::hasEntryForId( ResourceID ) ;
}
sal_Bool StringResourceWithStorageImpl::hasEntryForIdAndLocale( const ::rtl::OUString& ResourceID,
    const Locale& locale )
        throw (RuntimeException)
{
    return StringResourceImpl::hasEntryForIdAndLocale( ResourceID, locale );
}
Sequence< ::rtl::OUString > StringResourceWithStorageImpl::getResourceIDs(  )
    throw (RuntimeException)
{
    return StringResourceImpl::getResourceIDs();
}
Sequence< ::rtl::OUString > StringResourceWithStorageImpl::getResourceIDsForLocale
    ( const Locale& locale ) throw (::com::sun::star::uno::RuntimeException)
{
    return StringResourceImpl::getResourceIDsForLocale( locale );
}
Locale StringResourceWithStorageImpl::getCurrentLocale()
    throw (RuntimeException)
{
    return StringResourceImpl::getCurrentLocale();
}
Locale StringResourceWithStorageImpl::getDefaultLocale(  )
    throw (RuntimeException)
{
    return StringResourceImpl::getDefaultLocale();
}
Sequence< Locale > StringResourceWithStorageImpl::getLocales(  )
    throw (RuntimeException)
{
    return StringResourceImpl::getLocales();
}

// XStringResourceManager
sal_Bool StringResourceWithStorageImpl::isReadOnly()
    throw (RuntimeException)
{
    return StringResourceImpl::isReadOnly();
}
void StringResourceWithStorageImpl::setCurrentLocale( const Locale& locale, sal_Bool FindClosestMatch )
    throw (IllegalArgumentException, RuntimeException)
{
    StringResourceImpl::setCurrentLocale( locale, FindClosestMatch );
}
void StringResourceWithStorageImpl::setDefaultLocale( const Locale& locale )
    throw (IllegalArgumentException, RuntimeException,NoSupportException)
{
    StringResourceImpl::setDefaultLocale( locale );
}
void StringResourceWithStorageImpl::setString( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str )
    throw (NoSupportException, RuntimeException)
{
    StringResourceImpl::setString( ResourceID, Str );
}
void StringResourceWithStorageImpl::setStringForLocale
    ( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str, const Locale& locale )
        throw (NoSupportException, RuntimeException)
{
    StringResourceImpl::setStringForLocale( ResourceID, Str, locale );
}
void StringResourceWithStorageImpl::removeId( const ::rtl::OUString& ResourceID )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException, NoSupportException)
{
    StringResourceImpl::removeId( ResourceID );
}
void StringResourceWithStorageImpl::removeIdForLocale( const ::rtl::OUString& ResourceID, const Locale& locale )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException, NoSupportException)
{
    StringResourceImpl::removeIdForLocale( ResourceID, locale );
}
void StringResourceWithStorageImpl::newLocale( const Locale& locale )
    throw (ElementExistException, IllegalArgumentException, RuntimeException, NoSupportException)
{
    StringResourceImpl::newLocale( locale );
}
void StringResourceWithStorageImpl::removeLocale( const Locale& locale )
    throw (IllegalArgumentException, RuntimeException, NoSupportException)
{
    StringResourceImpl::removeLocale( locale );
}
sal_Int32 StringResourceWithStorageImpl::getUniqueNumericId(  )
    throw (RuntimeException, NoSupportException)
{
    return StringResourceImpl::getUniqueNumericId();
}

// XStringResourcePersistence
void StringResourceWithStorageImpl::store()
    throw (NoSupportException, Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceWithStorageImpl::store(): Read only" );

    bool bUsedForStore = true;
    bool bStoreAll = m_bStorageChanged;
    m_bStorageChanged = false;
    if( !m_bModified && !bStoreAll )
        return;

    implStoreAtStorage( m_aNameBase, m_aComment, m_xStorage, bUsedForStore, bStoreAll );
    m_bModified = false;
}

sal_Bool StringResourceWithStorageImpl::isModified(  )
    throw (RuntimeException)
{
    return StringResourcePersistenceImpl::isModified();
}
void StringResourceWithStorageImpl::setComment( const ::rtl::OUString& Comment )
    throw (::com::sun::star::uno::RuntimeException)
{
    StringResourcePersistenceImpl::setComment( Comment );
}
void StringResourceWithStorageImpl::storeToStorage( const Reference< XStorage >& Storage,
    const ::rtl::OUString& NameBase, const ::rtl::OUString& Comment )
        throw (Exception, RuntimeException)
{
    StringResourcePersistenceImpl::storeToStorage( Storage, NameBase, Comment );
}
void StringResourceWithStorageImpl::storeToURL( const ::rtl::OUString& URL,
    const ::rtl::OUString& NameBase, const ::rtl::OUString& Comment,
    const Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
        throw (Exception, RuntimeException)
{
    StringResourcePersistenceImpl::storeToURL( URL, NameBase, Comment, Handler );
}
Sequence< ::sal_Int8 > StringResourceWithStorageImpl::exportBinary(  )
    throw (RuntimeException)
{
    return StringResourcePersistenceImpl::exportBinary();
}
void StringResourceWithStorageImpl::importBinary( const Sequence< ::sal_Int8 >& Data )
    throw (IllegalArgumentException, RuntimeException)
{
    StringResourcePersistenceImpl::importBinary( Data );
}

// -----------------------------------------------------------------------------
// XStringResourceWithStorage

void StringResourceWithStorageImpl::storeAsStorage( const Reference< XStorage >& Storage )
    throw (Exception, RuntimeException)
{
    setStorage( Storage );
    store();
}

void StringResourceWithStorageImpl::setStorage( const Reference< XStorage >& Storage )
    throw (IllegalArgumentException, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );

    if( !Storage.is() )
    {
        ::rtl::OUString errorMsg(RTL_CONSTASCII_USTRINGPARAM
            ( "StringResourceWithStorageImpl::setStorage: invalid storage" ));
        throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 0 );
    }

    implLoadAllLocales();

    m_xStorage = Storage;
    m_bStorageChanged = true;
}


// =============================================================================
// Private helper methods
// =============================================================================

// Scan locale properties files
void StringResourceWithStorageImpl::implScanLocales( void )
{
    Reference< container::XNameAccess > xNameAccess( m_xStorage, UNO_QUERY );
    if( xNameAccess.is() )
    {
        Sequence< ::rtl::OUString > aContentSeq = xNameAccess->getElementNames();
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
        ::rtl::OUString aStreamName = implGetFileNameForLocaleItem( pLocaleItem, m_aNameBase );
        aStreamName += ::rtl::OUString( ".properties" );

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


// =============================================================================
// StringResourceWithLocationImpl
// =============================================================================

// component operations
static Sequence< ::rtl::OUString > getSupportedServiceNames_StringResourceWithLocationImpl()
{
    Sequence< ::rtl::OUString > names(1);
    names[0] = ::rtl::OUString( "com.sun.star.resource.StringResourceWithLocation" );
    return names;
}

static ::rtl::OUString getImplementationName_StringResourceWithLocationImpl()
{
    return ::rtl::OUString( "com.sun.star.comp.scripting.StringResourceWithLocation" );
}

static Reference< XInterface > SAL_CALL create_StringResourceWithLocationImpl(
    Reference< XComponentContext > const & xContext )
    SAL_THROW(())
{
    return static_cast< ::cppu::OWeakObject * >( new StringResourceWithLocationImpl( xContext ) );
}

// -----------------------------------------------------------------------------

StringResourceWithLocationImpl::StringResourceWithLocationImpl( const Reference< XComponentContext >& rxContext )
    : StringResourceWithLocationImpl_BASE( rxContext )
    , m_bLocationChanged( false )
{
}

// -----------------------------------------------------------------------------

StringResourceWithLocationImpl::~StringResourceWithLocationImpl()
{
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString StringResourceWithLocationImpl::getImplementationName(  ) throw (RuntimeException)
{
    return getImplementationName_StringResourceWithLocationImpl();
}

// -----------------------------------------------------------------------------

sal_Bool StringResourceWithLocationImpl::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
    const ::rtl::OUString* pNames = aNames.getConstArray();
    const ::rtl::OUString* pEnd = pNames + aNames.getLength();
    for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
        ;

    return pNames != pEnd;
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > StringResourceWithLocationImpl::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return getSupportedServiceNames_StringResourceWithLocationImpl();
}

// -----------------------------------------------------------------------------
// XInitialization
// -----------------------------------------------------------------------------

void StringResourceWithLocationImpl::initialize( const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );

    if ( aArguments.getLength() != 6 )
    {
        throw RuntimeException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM
                ( "XInitialization::initialize: invalid number of arguments!" ) ),
            Reference< XInterface >() );
    }

    bool bOk = (aArguments[0] >>= m_aLocation);
    sal_Int32 nLen = m_aLocation.getLength();
    if( bOk && nLen == 0 )
    {
        bOk = false;
    }
    else
    {
        if( m_aLocation.getStr()[nLen - 1] != '/' )
            m_aLocation += ::rtl::OUString("/");
    }

    if( !bOk )
    {
        ::rtl::OUString errorMsg("XInitialization::initialize: invalid URL");
        throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 0 );
    }


    bOk = (aArguments[5] >>= m_xInteractionHandler);
    if( !bOk )
    {
        ::rtl::OUString errorMsg("StringResourceWithStorageImpl::initialize: invalid type");
        throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 5 );
    }

    implInitializeCommonParameters( aArguments );
}

// -----------------------------------------------------------------------------
// Forwarding calls to base class

// XModifyBroadcaster
void StringResourceWithLocationImpl::addModifyListener( const Reference< XModifyListener >& aListener )
    throw (RuntimeException)
{
    StringResourceImpl::addModifyListener( aListener );
}
void StringResourceWithLocationImpl::removeModifyListener( const Reference< XModifyListener >& aListener )
    throw (RuntimeException)
{
    StringResourceImpl::removeModifyListener( aListener );
}

// XStringResourceResolver
::rtl::OUString StringResourceWithLocationImpl::resolveString( const ::rtl::OUString& ResourceID )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException)
{
    return StringResourceImpl::resolveString( ResourceID ) ;
}
::rtl::OUString StringResourceWithLocationImpl::resolveStringForLocale( const ::rtl::OUString& ResourceID, const Locale& locale )
    throw ( ::com::sun::star::resource::MissingResourceException, RuntimeException)
{
    return StringResourceImpl::resolveStringForLocale( ResourceID, locale );
}
sal_Bool StringResourceWithLocationImpl::hasEntryForId( const ::rtl::OUString& ResourceID )
    throw (RuntimeException)
{
    return StringResourceImpl::hasEntryForId( ResourceID ) ;
}
sal_Bool StringResourceWithLocationImpl::hasEntryForIdAndLocale( const ::rtl::OUString& ResourceID,
    const Locale& locale )
        throw (RuntimeException)
{
    return StringResourceImpl::hasEntryForIdAndLocale( ResourceID, locale );
}
Sequence< ::rtl::OUString > StringResourceWithLocationImpl::getResourceIDs(  )
    throw (RuntimeException)
{
    return StringResourceImpl::getResourceIDs();
}
Sequence< ::rtl::OUString > StringResourceWithLocationImpl::getResourceIDsForLocale
    ( const Locale& locale ) throw (::com::sun::star::uno::RuntimeException)
{
    return StringResourceImpl::getResourceIDsForLocale( locale );
}
Locale StringResourceWithLocationImpl::getCurrentLocale()
    throw (RuntimeException)
{
    return StringResourceImpl::getCurrentLocale();
}
Locale StringResourceWithLocationImpl::getDefaultLocale(  )
    throw (RuntimeException)
{
    return StringResourceImpl::getDefaultLocale();
}
Sequence< Locale > StringResourceWithLocationImpl::getLocales(  )
    throw (RuntimeException)
{
    return StringResourceImpl::getLocales();
}

// XStringResourceManager
sal_Bool StringResourceWithLocationImpl::isReadOnly()
    throw (RuntimeException)
{
    return StringResourceImpl::isReadOnly();
}
void StringResourceWithLocationImpl::setCurrentLocale( const Locale& locale, sal_Bool FindClosestMatch )
    throw (IllegalArgumentException, RuntimeException)
{
    StringResourceImpl::setCurrentLocale( locale, FindClosestMatch );
}
void StringResourceWithLocationImpl::setDefaultLocale( const Locale& locale )
    throw (IllegalArgumentException, RuntimeException,NoSupportException)
{
    StringResourceImpl::setDefaultLocale( locale );
}
void StringResourceWithLocationImpl::setString( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str )
    throw (NoSupportException, RuntimeException)
{
    StringResourceImpl::setString( ResourceID, Str );
}
void StringResourceWithLocationImpl::setStringForLocale
    ( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str, const Locale& locale )
        throw (NoSupportException, RuntimeException)
{
    StringResourceImpl::setStringForLocale( ResourceID, Str, locale );
}
void StringResourceWithLocationImpl::removeId( const ::rtl::OUString& ResourceID )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException, NoSupportException)
{
    StringResourceImpl::removeId( ResourceID );
}
void StringResourceWithLocationImpl::removeIdForLocale( const ::rtl::OUString& ResourceID, const Locale& locale )
    throw (::com::sun::star::resource::MissingResourceException, RuntimeException, NoSupportException)
{
    StringResourceImpl::removeIdForLocale( ResourceID, locale );
}
void StringResourceWithLocationImpl::newLocale( const Locale& locale )
    throw (ElementExistException, IllegalArgumentException, RuntimeException, NoSupportException)
{
    StringResourceImpl::newLocale( locale );
}
void StringResourceWithLocationImpl::removeLocale( const Locale& locale )
    throw (IllegalArgumentException, RuntimeException, NoSupportException)
{
    StringResourceImpl::removeLocale( locale );
}
sal_Int32 StringResourceWithLocationImpl::getUniqueNumericId(  )
    throw (RuntimeException, NoSupportException)
{
    return StringResourceImpl::getUniqueNumericId();
}

// XStringResourcePersistence
void StringResourceWithLocationImpl::store()
    throw (NoSupportException, Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceWithLocationImpl::store(): Read only" );

    bool bUsedForStore = true;
    bool bStoreAll = m_bLocationChanged;
    m_bLocationChanged = false;
    if( !m_bModified && !bStoreAll )
        return;

    Reference< ucb::XSimpleFileAccess2 > xFileAccess = getFileAccess();
    implStoreAtLocation( m_aLocation, m_aNameBase, m_aComment,
        xFileAccess, bUsedForStore, bStoreAll );
    m_bModified = false;
}

sal_Bool StringResourceWithLocationImpl::isModified(  )
    throw (RuntimeException)
{
    return StringResourcePersistenceImpl::isModified();
}
void StringResourceWithLocationImpl::setComment( const ::rtl::OUString& Comment )
    throw (::com::sun::star::uno::RuntimeException)
{
    StringResourcePersistenceImpl::setComment( Comment );
}
void StringResourceWithLocationImpl::storeToStorage( const Reference< XStorage >& Storage,
    const ::rtl::OUString& NameBase, const ::rtl::OUString& Comment )
        throw (Exception, RuntimeException)
{
    StringResourcePersistenceImpl::storeToStorage( Storage, NameBase, Comment );
}
void StringResourceWithLocationImpl::storeToURL( const ::rtl::OUString& URL,
    const ::rtl::OUString& NameBase, const ::rtl::OUString& Comment,
    const Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
        throw (Exception, RuntimeException)
{
    StringResourcePersistenceImpl::storeToURL( URL, NameBase, Comment, Handler );
}
Sequence< ::sal_Int8 > StringResourceWithLocationImpl::exportBinary(  )
    throw (RuntimeException)
{
    return StringResourcePersistenceImpl::exportBinary();
}
void StringResourceWithLocationImpl::importBinary( const Sequence< ::sal_Int8 >& Data )
    throw (IllegalArgumentException, RuntimeException)
{
    StringResourcePersistenceImpl::importBinary( Data );
}

// -----------------------------------------------------------------------------
// XStringResourceWithLocation

// XStringResourceWithLocation
void StringResourceWithLocationImpl::storeAsURL( const ::rtl::OUString& URL )
    throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    setURL( URL );
    store();
}

void StringResourceWithLocationImpl::setURL( const ::rtl::OUString& URL )
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    implCheckReadOnly( "StringResourceWithLocationImpl::setURL(): Read only" );

    sal_Int32 nLen = URL.getLength();
    if( nLen == 0 )
    {
        ::rtl::OUString errorMsg(RTL_CONSTASCII_USTRINGPARAM
            ( "StringResourceWithLocationImpl::setURL: invalid URL" ));
        throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 0 );
    }

    implLoadAllLocales();

    // Delete files at old location
    bool bUsedForStore = false;
    bool bStoreAll = false;
    bool bKillAll = true;
    implStoreAtLocation( m_aLocation, m_aNameBase, m_aComment,
        getFileAccess(), bUsedForStore, bStoreAll, bKillAll );

    m_aLocation = URL;
    m_bLocationChanged = true;
}


// =============================================================================
// Private helper methods
// =============================================================================

// Scan locale properties files
void StringResourceWithLocationImpl::implScanLocales( void )
{
    const Reference< ucb::XSimpleFileAccess2 > xFileAccess = getFileAccess();
    if( xFileAccess.is() && xFileAccess->isFolder( m_aLocation ) )
    {
        Sequence< ::rtl::OUString > aContentSeq = xFileAccess->getFolderContents( m_aLocation, false );
        implScanLocaleNames( aContentSeq );
    }
}

// Loading
bool StringResourceWithLocationImpl::implLoadLocale( LocaleItem* pLocaleItem )
{
    bool bSuccess = false;

    const Reference< ucb::XSimpleFileAccess2 > xFileAccess = getFileAccess();
    if( xFileAccess.is() )
    {
        ::rtl::OUString aCompleteFileName =
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

const Reference< ucb::XSimpleFileAccess2 > StringResourceWithLocationImpl::getFileAccess( void )
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


// =============================================================================
// component export operations
// =============================================================================

static struct ::cppu::ImplementationEntry s_component_entries [] =
{
    {
        create_StringResourceImpl, getImplementationName_StringResourceImpl,
        getSupportedServiceNames_StringResourceImpl,
        ::cppu::createSingleComponentFactory,
        0, 0
    },
    {
        create_StringResourceWithLocationImpl, getImplementationName_StringResourceWithLocationImpl,
        getSupportedServiceNames_StringResourceWithLocationImpl,
        ::cppu::createSingleComponentFactory,
        0, 0
    },
    {
        create_StringResourceWithStorageImpl, getImplementationName_StringResourceWithStorageImpl,
        getSupportedServiceNames_StringResourceWithStorageImpl,
        ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};


//.........................................................................
}   // namespace dlgprov
//.........................................................................


// =============================================================================
// component exports
// =============================================================================

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
        const sal_Char * pImplName, lang::XMultiServiceFactory * pServiceManager,
        registry::XRegistryKey * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, ::stringresource::s_component_entries );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
