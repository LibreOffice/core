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


#include <cachedcontentresultset.hxx>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/ucb/FetchError.hpp>
#include <com/sun/star/ucb/ResultSetException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <rtl/ustring.hxx>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <memory>

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace cppu;


#define COMSUNSTARUCBCCRS_DEFAULT_FETCH_SIZE 256
#define COMSUNSTARUCBCCRS_DEFAULT_FETCH_DIRECTION FetchDirection::FORWARD

//if you change this function template please pay attention to
//function getObject, where this is similar implemented

template<typename T> T CachedContentResultSet::rowOriginGet(
    T (SAL_CALL css::sdbc::XRow::* f)(sal_Int32), sal_Int32 columnIndex)
{
    impl_EnsureNotDisposed();
    osl::ResettableMutexGuard aGuard(m_aMutex);
    sal_Int32 nRow = m_nRow;
    sal_Int32 nFetchSize = m_nFetchSize;
    sal_Int32 nFetchDirection = m_nFetchDirection;
    if( !m_aCache.hasRow( nRow ) )
    {
        bool isCleared = false;
        if( !m_aCache.hasCausedException( nRow ) )
        {
            if( !m_xFetchProvider.is() )
            {
                OSL_FAIL( "broadcaster was disposed already" );
                throw SQLException();
            }
            aGuard.clear();
            isCleared = true;
            if( impl_isForwardOnly() )
                applyPositionToOrigin( nRow );

            impl_fetchData( nRow, nFetchSize, nFetchDirection );
        }
        if (isCleared)
        {
            aGuard.reset();
        }
        if( !m_aCache.hasRow( nRow ) )
        {
            m_bLastReadWasFromCache = false;
            aGuard.clear();
            applyPositionToOrigin( nRow );
            impl_init_xRowOrigin();
            return (m_xRowOrigin.get()->*f)( columnIndex );
        }
    }
    const Any& rValue = m_aCache.getAny( nRow, columnIndex );
    T aRet = T();
    m_bLastReadWasFromCache = true;
    m_bLastCachedReadWasNull = !( rValue >>= aRet );
    /* Last chance. Try type converter service... */
    if ( m_bLastCachedReadWasNull && rValue.hasValue() )
    {
        Reference< XTypeConverter > xConverter = getTypeConverter();
        if ( xConverter.is() )
        {
            try
            {
                Any aConvAny = xConverter->convertTo(
                    rValue,
                    cppu::UnoType<T>::get() );
                m_bLastCachedReadWasNull = !( aConvAny >>= aRet );
            }
            catch (const IllegalArgumentException&)
            {
            }
            catch (const CannotConvertException&)
            {
            }
        }
    }
    return aRet;
}


// CCRS_Cache methods


CachedContentResultSet::CCRS_Cache::CCRS_Cache(
    const Reference< XContentIdentifierMapping > & xMapping )
    : m_pResult( nullptr )
    , m_xContentIdentifierMapping( xMapping )
    , m_pMappedReminder( nullptr )
{
}

CachedContentResultSet::CCRS_Cache::~CCRS_Cache()
{
}

void SAL_CALL CachedContentResultSet::CCRS_Cache
    ::clear()
{
    m_pResult.reset();
    clearMappedReminder();
}

void SAL_CALL CachedContentResultSet::CCRS_Cache
    ::loadData( const FetchResult& rResult )
{
    clear();
    m_pResult.reset( new FetchResult( rResult ) );
}

bool SAL_CALL CachedContentResultSet::CCRS_Cache
    ::hasRow( sal_Int32 row )
{
    if( !m_pResult )
        return false;
    long nStart = m_pResult->StartIndex;
    long nEnd = nStart;
    if( m_pResult->Orientation )
        nEnd += m_pResult->Rows.getLength() - 1;
    else
        nStart -= m_pResult->Rows.getLength() + 1;

    return nStart <= row && row <= nEnd;
}

sal_Int32 SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getMaxRow()
{
    if( !m_pResult )
        return 0;
    long nEnd = m_pResult->StartIndex;
    if( m_pResult->Orientation )
        return nEnd += m_pResult->Rows.getLength() - 1;
    else
        return nEnd;
}

bool SAL_CALL CachedContentResultSet::CCRS_Cache
    ::hasKnownLast()
{
    if( !m_pResult )
        return false;

    if( ( m_pResult->FetchError & FetchError::ENDOFDATA )
        && m_pResult->Orientation
        && m_pResult->Rows.getLength() )
        return true;

    return false;
}

bool SAL_CALL CachedContentResultSet::CCRS_Cache
    ::hasCausedException( sal_Int32 nRow )
{
    if( !m_pResult )
        return false;
    if( !( m_pResult->FetchError & FetchError::EXCEPTION ) )
        return false;

    long nEnd = m_pResult->StartIndex;
    if( m_pResult->Orientation )
        nEnd += m_pResult->Rows.getLength();

    return nRow == nEnd+1;
}

Any& SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getRowAny( sal_Int32 nRow )
{
    if( !nRow )
        throw SQLException();
    if( !m_pResult )
        throw SQLException();
    if( !hasRow( nRow ) )
        throw SQLException();

    long nDiff = nRow - m_pResult->StartIndex;
    if( nDiff < 0 )
        nDiff *= -1;

    return (m_pResult->Rows)[nDiff];
}

void SAL_CALL CachedContentResultSet::CCRS_Cache
    ::remindMapped( sal_Int32 nRow )
{
    //remind that this row was mapped
    if( !m_pResult )
        return;
    long nDiff = nRow - m_pResult->StartIndex;
    if( nDiff < 0 )
        nDiff *= -1;
    Sequence< sal_Bool >* pMappedReminder = getMappedReminder();
    if( nDiff < pMappedReminder->getLength() )
        (*pMappedReminder)[nDiff] = true;
}

bool SAL_CALL CachedContentResultSet::CCRS_Cache
    ::isRowMapped( sal_Int32 nRow )
{
    if( !m_pMappedReminder || !m_pResult )
        return false;
    long nDiff = nRow - m_pResult->StartIndex;
    if( nDiff < 0 )
        nDiff *= -1;
    if( nDiff < m_pMappedReminder->getLength() )
        return (*m_pMappedReminder)[nDiff];
    return false;
}

void SAL_CALL CachedContentResultSet::CCRS_Cache
    ::clearMappedReminder()
{
    delete m_pMappedReminder;
    m_pMappedReminder = nullptr;
}

Sequence< sal_Bool >* SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getMappedReminder()
{
    if( !m_pMappedReminder )
    {
        sal_Int32 nCount = m_pResult->Rows.getLength();
        m_pMappedReminder = new Sequence< sal_Bool >( nCount );
        for( ;nCount; nCount-- )
            (*m_pMappedReminder)[nCount] = false;
    }
    return m_pMappedReminder;
}

const Any& SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getAny( sal_Int32 nRow, sal_Int32 nColumnIndex )
{
    if( !nColumnIndex )
        throw SQLException();
    if( m_xContentIdentifierMapping.is() && !isRowMapped( nRow ) )
    {
        Any& rRow = getRowAny( nRow );
        Sequence< Any > aValue;
        rRow >>= aValue;
        if( m_xContentIdentifierMapping->mapRow( aValue ) )
        {
            rRow <<= aValue;
            remindMapped( nRow );
        }
        else
            m_xContentIdentifierMapping.clear();
    }
    auto & rowAny = getRowAny(nRow);
    auto rRow = o3tl::doAccess<Sequence<Any>>(rowAny);

    if( nColumnIndex > rRow->getLength() )
        throw SQLException();
    return (*rRow)[nColumnIndex-1];
}

OUString SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getContentIdentifierString( sal_Int32 nRow )
{
    try
    {
        if( m_xContentIdentifierMapping.is() && !isRowMapped( nRow ) )
        {
            Any& rRow = getRowAny( nRow );
            OUString aValue;
            rRow >>= aValue;
            rRow <<= m_xContentIdentifierMapping->mapContentIdentifierString( aValue );
            remindMapped( nRow );
        }
        return *o3tl::doAccess<OUString>(getRowAny(nRow));
    }
    catch(const SQLException&)
    {
        throw RuntimeException();
    }
}

Reference< XContentIdentifier > SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getContentIdentifier( sal_Int32 nRow )
{
    try
    {
        if( m_xContentIdentifierMapping.is() && !isRowMapped( nRow ) )
        {
            Any& rRow = getRowAny( nRow );
            Reference< XContentIdentifier > aValue;
            rRow >>= aValue;
            rRow <<= m_xContentIdentifierMapping->mapContentIdentifier( aValue );
            remindMapped( nRow );
        }
        return *o3tl::doAccess<Reference<XContentIdentifier>>(getRowAny(nRow));
    }
    catch(const SQLException&)
    {
        throw RuntimeException();
    }
}

Reference< XContent > SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getContent( sal_Int32 nRow )
{
    try
    {
        if( m_xContentIdentifierMapping.is() && !isRowMapped( nRow ) )
        {
            Any& rRow = getRowAny( nRow );
            Reference< XContent > aValue;
            rRow >>= aValue;
            rRow <<= m_xContentIdentifierMapping->mapContent( aValue );
            remindMapped( nRow );
        }
        return *o3tl::doAccess<Reference<XContent>>(getRowAny(nRow));
    }
    catch (const SQLException&)
    {
        throw RuntimeException();
    }
}


// class CCRS_PropertySetInfo


class CCRS_PropertySetInfo :
                public cppu::OWeakObject,
                public css::lang::XTypeProvider,
                public css::beans::XPropertySetInfo
{
    friend class CachedContentResultSet;

    //my Properties
    std::unique_ptr<Sequence< css::beans::Property >>
                            m_pProperties;

    long                    m_nFetchSizePropertyHandle;
    long                    m_nFetchDirectionPropertyHandle;

private:
    sal_Int32 SAL_CALL
    impl_getRemainedHandle() const;

    bool SAL_CALL
    impl_queryProperty(
            const OUString& rName
            , css::beans::Property& rProp ) const;
    sal_Int32 SAL_CALL
    impl_getPos( const OUString& rName ) const;

    static bool SAL_CALL
    impl_isMyPropertyName( const OUString& rName );

public:
    explicit CCRS_PropertySetInfo(   Reference<
            XPropertySetInfo > const & xPropertySetInfoOrigin );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    // XPropertySetInfo
    virtual Sequence< css::beans::Property > SAL_CALL
    getProperties() override;

    virtual css::beans::Property SAL_CALL
    getPropertyByName( const OUString& aName ) override;

    virtual sal_Bool SAL_CALL
    hasPropertyByName( const OUString& Name ) override;
};

//some helping variables ( names for my special properties )
static const char g_sPropertyNameForCount[] = "RowCount";
static const char g_sPropertyNameForFinalCount[] = "IsRowCountFinal";
static const char g_sPropertyNameForFetchSize[] = "FetchSize";
static const char g_sPropertyNameForFetchDirection[] = "FetchDirection";

CCRS_PropertySetInfo::CCRS_PropertySetInfo(
        Reference< XPropertySetInfo > const & xInfo )
        : m_pProperties( nullptr )
        , m_nFetchSizePropertyHandle( -1 )
        , m_nFetchDirectionPropertyHandle( -1 )
{
    //initialize list of properties:

    // it is required, that the received xInfo contains the two
    // properties with names 'g_sPropertyNameForCount' and
    // 'g_sPropertyNameForFinalCount'

    if( xInfo.is() )
    {
        Sequence<Property> aProps = xInfo->getProperties();
        m_pProperties.reset( new Sequence<Property> ( aProps ) );
    }
    else
    {
        OSL_FAIL( "The received XPropertySetInfo doesn't contain required properties" );
        m_pProperties.reset( new Sequence<Property> );
    }

    //ensure, that we haven't got the Properties 'FetchSize' and 'Direction' twice:
    sal_Int32 nFetchSize = impl_getPos( g_sPropertyNameForFetchSize );
    sal_Int32 nFetchDirection = impl_getPos( g_sPropertyNameForFetchDirection );
    sal_Int32 nDeleted = 0;
    if( nFetchSize != -1 )
        nDeleted++;
    if( nFetchDirection != -1 )
        nDeleted++;

    std::unique_ptr<Sequence< Property > > pOrigProps(new Sequence<Property> ( *m_pProperties ));
    sal_Int32 nOrigProps = pOrigProps->getLength();

    m_pProperties->realloc( nOrigProps + 2 - nDeleted );//note that nDeleted is <= 2
    for( sal_Int32 n = 0, m = 0; n < nOrigProps; n++, m++ )
    {
        if( n == nFetchSize || n == nFetchDirection )
            m--;
        else
            (*m_pProperties)[ m ] = (*pOrigProps)[ n ];
    }
    {
        Property& rMyProp = (*m_pProperties)[ nOrigProps - nDeleted ];
        rMyProp.Name = g_sPropertyNameForFetchSize;
        rMyProp.Type = cppu::UnoType<sal_Int32>::get();
        rMyProp.Attributes = PropertyAttribute::BOUND | PropertyAttribute::MAYBEDEFAULT;

        if( nFetchSize != -1 )
            m_nFetchSizePropertyHandle = (*pOrigProps)[nFetchSize].Handle;
        else
            m_nFetchSizePropertyHandle = impl_getRemainedHandle();

        rMyProp.Handle = m_nFetchSizePropertyHandle;

    }
    {
        Property& rMyProp = (*m_pProperties)[ nOrigProps - nDeleted + 1 ];
        rMyProp.Name = g_sPropertyNameForFetchDirection;
        rMyProp.Type = cppu::UnoType<sal_Bool>::get();
        rMyProp.Attributes = PropertyAttribute::BOUND | PropertyAttribute::MAYBEDEFAULT;

        if( nFetchDirection != -1 )
            m_nFetchDirectionPropertyHandle = (*pOrigProps)[nFetchDirection].Handle;
        else
            m_nFetchDirectionPropertyHandle = impl_getRemainedHandle();

        m_nFetchDirectionPropertyHandle = rMyProp.Handle;
    }
}

// XInterface methods.

void SAL_CALL CCRS_PropertySetInfo::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL CCRS_PropertySetInfo::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL CCRS_PropertySetInfo::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< XTypeProvider* >(this)),
                                               (static_cast< XPropertySetInfo* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.

//list all interfaces exclusive baseclasses
XTYPEPROVIDER_IMPL_2( CCRS_PropertySetInfo
                    , XTypeProvider
                    , XPropertySetInfo
                    );

// XPropertySetInfo methods.

//virtual
Sequence< Property > SAL_CALL CCRS_PropertySetInfo
    ::getProperties()
{
    return *m_pProperties;
}

//virtual
Property SAL_CALL CCRS_PropertySetInfo
    ::getPropertyByName( const OUString& aName )
{
    if ( aName.isEmpty() )
        throw UnknownPropertyException();

    Property aProp;
    if ( impl_queryProperty( aName, aProp ) )
        return aProp;

    throw UnknownPropertyException();
}

//virtual
sal_Bool SAL_CALL CCRS_PropertySetInfo
    ::hasPropertyByName( const OUString& Name )
{
    return ( impl_getPos( Name ) != -1 );
}


// impl_ methods.


sal_Int32 SAL_CALL CCRS_PropertySetInfo
            ::impl_getPos( const OUString& rName ) const
{
    for( sal_Int32 nN = m_pProperties->getLength(); nN--; )
    {
        const Property& rMyProp = (*m_pProperties)[nN];
        if( rMyProp.Name == rName )
            return nN;
    }
    return -1;
}

bool SAL_CALL CCRS_PropertySetInfo
        ::impl_queryProperty( const OUString& rName, Property& rProp ) const
{
    for( sal_Int32 nN = m_pProperties->getLength(); nN--; )
    {
        const Property& rMyProp = (*m_pProperties)[nN];
        if( rMyProp.Name == rName )
        {
            rProp.Name = rMyProp.Name;
            rProp.Handle = rMyProp.Handle;
            rProp.Type = rMyProp.Type;
            rProp.Attributes = rMyProp.Attributes;

            return true;
        }
    }
    return false;
}

//static
bool SAL_CALL CCRS_PropertySetInfo
        ::impl_isMyPropertyName( const OUString& rPropertyName )
{
    return ( rPropertyName == g_sPropertyNameForCount
    || rPropertyName == g_sPropertyNameForFinalCount
    || rPropertyName == g_sPropertyNameForFetchSize
    || rPropertyName == g_sPropertyNameForFetchDirection );
}

sal_Int32 SAL_CALL CCRS_PropertySetInfo
            ::impl_getRemainedHandle( ) const
{
    sal_Int32 nHandle = 1;

    if( !m_pProperties )
    {
        OSL_FAIL( "Properties not initialized yet" );
        return nHandle;
    }
    bool bFound = true;
    while( bFound )
    {
        bFound = false;
        for( sal_Int32 nN = m_pProperties->getLength(); nN--; )
        {
            if( nHandle == (*m_pProperties)[nN].Handle )
            {
                bFound = true;
                nHandle++;
                break;
            }
        }
    }
    return nHandle;
}


// class CachedContentResultSet


CachedContentResultSet::CachedContentResultSet(
                  const Reference< XComponentContext > & rxContext
                , const Reference< XResultSet > & xOrigin
                , const Reference< XContentIdentifierMapping > &
                    xContentIdentifierMapping )
                : ContentResultSetWrapper( xOrigin )

                , m_xContext( rxContext )
                , m_xFetchProvider( nullptr )
                , m_xFetchProviderForContentAccess( nullptr )

                , m_xContentIdentifierMapping( xContentIdentifierMapping )
                , m_nRow( 0 ) // Position is one-based. Zero means: before first element.
                , m_bAfterLast( false )
                , m_nLastAppliedPos( 0 )
                , m_bAfterLastApplied( false )
                , m_nKnownCount( 0 )
                , m_bFinalCount( false )
                , m_nFetchSize(
                    COMSUNSTARUCBCCRS_DEFAULT_FETCH_SIZE )
                , m_nFetchDirection(
                    COMSUNSTARUCBCCRS_DEFAULT_FETCH_DIRECTION )

                , m_bLastReadWasFromCache( false )
                , m_bLastCachedReadWasNull( true )
                , m_aCache( m_xContentIdentifierMapping )
                , m_aCacheContentIdentifierString( m_xContentIdentifierMapping )
                , m_aCacheContentIdentifier( m_xContentIdentifierMapping )
                , m_aCacheContent( m_xContentIdentifierMapping )
                , m_bTriedToGetTypeConverter( false )
                , m_xTypeConverter( nullptr )
{
    m_xFetchProvider.set( m_xResultSetOrigin, UNO_QUERY );
    OSL_ENSURE( m_xFetchProvider.is(), "interface XFetchProvider is required" );

    m_xFetchProviderForContentAccess.set( m_xResultSetOrigin, UNO_QUERY );
    OSL_ENSURE( m_xFetchProviderForContentAccess.is(), "interface XFetchProviderForContentAccess is required" );

    impl_init();
};

CachedContentResultSet::~CachedContentResultSet()
{
    impl_deinit();
    //do not delete m_pMyPropSetInfo, cause it is hold via reference
};


// impl_ methods.


bool SAL_CALL CachedContentResultSet
    ::applyPositionToOrigin( sal_Int32 nRow )
{
    impl_EnsureNotDisposed();

    /**
    @returns
        <TRUE/> if the cursor is on a valid row; <FALSE/> if it is off
        the result set.
    */

    osl::ResettableMutexGuard aGuard(m_aMutex);
    OSL_ENSURE( nRow >= 0, "only positive values supported" );
    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return false;
    }
//  OSL_ENSURE( nRow <= m_nKnownCount, "don't step into regions you don't know with this method" );

    sal_Int32 nLastAppliedPos = m_nLastAppliedPos;
    bool bAfterLastApplied = m_bAfterLastApplied;
    bool bAfterLast = m_bAfterLast;
    sal_Int32 nForwardOnly = m_nForwardOnly;

    aGuard.clear();

    if( bAfterLastApplied || nLastAppliedPos != nRow )
    {
        if( nForwardOnly == 1 )
        {
            if( bAfterLastApplied || bAfterLast || !nRow || nRow < nLastAppliedPos )
                throw SQLException();

            sal_Int32 nN = nRow - nLastAppliedPos;
            sal_Int32 nM;
            for( nM = 0; nN--; nM++ )
            {
                if( !m_xResultSetOrigin->next() )
                    break;
            }

            aGuard.reset();
            m_nLastAppliedPos += nM;
            m_bAfterLastApplied = nRow != m_nLastAppliedPos;
            return nRow == m_nLastAppliedPos;
        }

        if( !nRow ) //absolute( 0 ) will throw exception
        {
            m_xResultSetOrigin->beforeFirst();

            aGuard.reset();
            m_nLastAppliedPos = 0;
            m_bAfterLastApplied = false;
            return false;
        }
        try
        {
            //move absolute, if !nLastAppliedPos
            //because move relative would throw exception
            if( !nLastAppliedPos || bAfterLast || bAfterLastApplied )
            {
                bool bValid = m_xResultSetOrigin->absolute( nRow );

                aGuard.reset();
                m_nLastAppliedPos = nRow;
                m_bAfterLastApplied = !bValid;
                return bValid;
            }
            else
            {
                bool bValid = m_xResultSetOrigin->relative( nRow - nLastAppliedPos );

                aGuard.reset();
                m_nLastAppliedPos += ( nRow - nLastAppliedPos );
                m_bAfterLastApplied = !bValid;
                return bValid;
            }
        }
        catch (const SQLException&)
        {
            if( !bAfterLastApplied && !bAfterLast && nRow > nLastAppliedPos && impl_isForwardOnly() )
            {
                sal_Int32 nN = nRow - nLastAppliedPos;
                sal_Int32 nM;
                for( nM = 0; nN--; nM++ )
                {
                    if( !m_xResultSetOrigin->next() )
                        break;
                }

                aGuard.reset();
                m_nLastAppliedPos += nM;
                m_bAfterLastApplied = nRow != m_nLastAppliedPos;
            }
            else
                throw;
        }

        return nRow == m_nLastAppliedPos;
    }
    return true;
};


//define for fetching data


#define FETCH_XXX( aCache, fetchInterface, fetchMethod )            \
bool bDirection = !!(                                           \
    nFetchDirection != FetchDirection::REVERSE );                   \
FetchResult aResult =                                               \
    fetchInterface->fetchMethod( nRow, nFetchSize, bDirection );    \
osl::ClearableGuard< osl::Mutex > aGuard2( m_aMutex );              \
aCache.loadData( aResult );                                         \
sal_Int32 nMax = aCache.getMaxRow();                                \
sal_Int32 nCurCount = m_nKnownCount;                                \
bool bIsFinalCount = aCache.hasKnownLast();                     \
bool bCurIsFinalCount = m_bFinalCount;                          \
aGuard2.clear();                                                    \
if( nMax > nCurCount )                                              \
    impl_changeRowCount( nCurCount, nMax );                         \
if( bIsFinalCount && !bCurIsFinalCount )                            \
    impl_changeIsRowCountFinal( bCurIsFinalCount, bIsFinalCount );

void SAL_CALL CachedContentResultSet
    ::impl_fetchData( sal_Int32 nRow
        , sal_Int32 nFetchSize, sal_Int32 nFetchDirection )
{
    FETCH_XXX( m_aCache, m_xFetchProvider, fetch );
}

void SAL_CALL CachedContentResultSet
    ::impl_changeRowCount( sal_Int32 nOld, sal_Int32 nNew )
{
    OSL_ENSURE( nNew > nOld, "RowCount only can grow" );
    if( nNew <= nOld )
        return;

    //create PropertyChangeEvent and set value
    PropertyChangeEvent aEvt;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aEvt.Source =  static_cast< XPropertySet * >( this );
        aEvt.Further = false;
        aEvt.OldValue <<= nOld;
        aEvt.NewValue <<= nNew;

        m_nKnownCount = nNew;
    }

    //send PropertyChangeEvent to listeners
    impl_notifyPropertyChangeListeners( aEvt );
}

void SAL_CALL CachedContentResultSet
    ::impl_changeIsRowCountFinal( bool bOld, bool bNew )
{
    OSL_ENSURE( !bOld && bNew, "This change is not allowed for IsRowCountFinal" );
    if( ! (!bOld && bNew ) )
        return;

    //create PropertyChangeEvent and set value
    PropertyChangeEvent aEvt;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aEvt.Source =  static_cast< XPropertySet * >( this );
        aEvt.Further = false;
        aEvt.OldValue <<= bOld;
        aEvt.NewValue <<= bNew;

        m_bFinalCount = bNew;
    }

    //send PropertyChangeEvent to listeners
    impl_notifyPropertyChangeListeners( aEvt );
}

bool SAL_CALL CachedContentResultSet
    ::impl_isKnownValidPosition( sal_Int32 nRow )
{
    return m_nKnownCount && nRow
            && nRow <= m_nKnownCount;
}

bool SAL_CALL CachedContentResultSet
    ::impl_isKnownInvalidPosition( sal_Int32 nRow )
{
    if( !nRow )
        return true;
    if( !m_bFinalCount )
        return false;
    return nRow > m_nKnownCount;
}


//virtual
void SAL_CALL CachedContentResultSet
    ::impl_initPropertySetInfo()
{
    ContentResultSetWrapper::impl_initPropertySetInfo();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if( m_xMyPropertySetInfo.is() )
        return;
    m_xMyPropertySetInfo = new CCRS_PropertySetInfo( m_xPropertySetInfo );
    m_xPropertySetInfo = m_xMyPropertySetInfo.get();
}


// XInterface methods.
void SAL_CALL CachedContentResultSet::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL CachedContentResultSet::release()
    throw()
{
    OWeakObject::release();
}

Any SAL_CALL CachedContentResultSet
    ::queryInterface( const Type&  rType )
{
    //list all interfaces inclusive baseclasses of interfaces

    Any aRet = ContentResultSetWrapper::queryInterface( rType );
    if( aRet.hasValue() )
        return aRet;

    aRet = cppu::queryInterface( rType,
                static_cast< XTypeProvider* >( this ),
                static_cast< XServiceInfo* >( this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


// XTypeProvider methods.

//list all interfaces exclusive baseclasses
XTYPEPROVIDER_IMPL_11( CachedContentResultSet
                    , XTypeProvider
                    , XServiceInfo
                    , XComponent
                    , XCloseable
                    , XResultSetMetaDataSupplier
                    , XPropertySet

                    , XPropertyChangeListener
                    , XVetoableChangeListener

                    , XContentAccess

                    , XResultSet
                    , XRow );


// XServiceInfo methods.

OUString SAL_CALL CachedContentResultSet::getImplementationName()
{
    return OUString( "com.sun.star.comp.ucb.CachedContentResultSet" );
}

sal_Bool SAL_CALL CachedContentResultSet::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL CachedContentResultSet::getSupportedServiceNames()
{
    return { CACHED_CONTENT_RESULTSET_SERVICE_NAME };
}



// XPropertySet methods. ( inherited )


// virtual
void SAL_CALL CachedContentResultSet
    ::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
{
    impl_EnsureNotDisposed();

    if( !getPropertySetInfo().is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }

    Property aProp = m_xMyPropertySetInfo->getPropertyByName( aPropertyName );
        //throws UnknownPropertyException, if so

    if( aProp.Attributes & PropertyAttribute::READONLY )
    {
        //It is assumed, that the properties
        //'RowCount' and 'IsRowCountFinal' are readonly!
        throw IllegalArgumentException();
    }
    if( aProp.Name == g_sPropertyNameForFetchDirection )
    {
        //check value
        sal_Int32 nNew;
        if( !( aValue >>= nNew ) )
        {
            throw IllegalArgumentException();
        }

        if( nNew == FetchDirection::UNKNOWN )
        {
            nNew = COMSUNSTARUCBCCRS_DEFAULT_FETCH_DIRECTION;
        }
        else if( !( nNew == FetchDirection::FORWARD
                || nNew == FetchDirection::REVERSE ) )
        {
            throw IllegalArgumentException();
        }

        //create PropertyChangeEvent and set value
        PropertyChangeEvent aEvt;
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            aEvt.Source =  static_cast< XPropertySet * >( this );
            aEvt.PropertyName = aPropertyName;
            aEvt.Further = false;
            aEvt.PropertyHandle = m_xMyPropertySetInfo->
                                    m_nFetchDirectionPropertyHandle;
            aEvt.OldValue <<= m_nFetchDirection;
            aEvt.NewValue <<= nNew;

            m_nFetchDirection = nNew;
        }

        //send PropertyChangeEvent to listeners
        impl_notifyPropertyChangeListeners( aEvt );
    }
    else if( aProp.Name == g_sPropertyNameForFetchSize )
    {
        //check value
        sal_Int32 nNew;
        if( !( aValue >>= nNew ) )
        {
            throw IllegalArgumentException();
        }

        if( nNew < 0 )
        {
            nNew = COMSUNSTARUCBCCRS_DEFAULT_FETCH_SIZE;
        }

        //create PropertyChangeEvent and set value
        PropertyChangeEvent aEvt;
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            aEvt.Source =  static_cast< XPropertySet * >( this );
            aEvt.PropertyName = aPropertyName;
            aEvt.Further = false;
            aEvt.PropertyHandle = m_xMyPropertySetInfo->
                                    m_nFetchSizePropertyHandle;
            aEvt.OldValue <<= m_nFetchSize;
            aEvt.NewValue <<= nNew;

            m_nFetchSize = nNew;
        }

        //send PropertyChangeEvent to listeners
        impl_notifyPropertyChangeListeners( aEvt );
    }
    else
    {
        impl_init_xPropertySetOrigin();
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            if( !m_xPropertySetOrigin.is() )
            {
                OSL_FAIL( "broadcaster was disposed already" );
                return;
            }
        }
        m_xPropertySetOrigin->setPropertyValue( aPropertyName, aValue );
    }
}


// virtual
Any SAL_CALL CachedContentResultSet
    ::getPropertyValue( const OUString& rPropertyName )
{
    impl_EnsureNotDisposed();

    if( !getPropertySetInfo().is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }

    m_xMyPropertySetInfo->getPropertyByName( rPropertyName );
        //throws UnknownPropertyException, if so

    Any aValue;
    if( rPropertyName == g_sPropertyNameForCount )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aValue <<= m_nKnownCount;
    }
    else if( rPropertyName == g_sPropertyNameForFinalCount )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aValue <<= m_bFinalCount;
    }
    else if( rPropertyName == g_sPropertyNameForFetchSize )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aValue <<= m_nFetchSize;
    }
    else if( rPropertyName == g_sPropertyNameForFetchDirection )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aValue <<= m_nFetchDirection;
    }
    else
    {
        impl_init_xPropertySetOrigin();
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            if( !m_xPropertySetOrigin.is() )
            {
                OSL_FAIL( "broadcaster was disposed already" );
                throw UnknownPropertyException();
            }
        }
        aValue = m_xPropertySetOrigin->getPropertyValue( rPropertyName );
    }
    return aValue;
}


// own methods.  ( inherited )


//virtual
void SAL_CALL CachedContentResultSet
    ::impl_disposing( const EventObject& rEventObject )
{
    {
        impl_EnsureNotDisposed();
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        //release all references to the broadcaster:
        m_xFetchProvider.clear();
        m_xFetchProviderForContentAccess.clear();
    }
    ContentResultSetWrapper::impl_disposing( rEventObject );
}

//virtual
void SAL_CALL CachedContentResultSet
    ::impl_propertyChange( const PropertyChangeEvent& rEvt )
{
    impl_EnsureNotDisposed();

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = false;


    if( CCRS_PropertySetInfo
            ::impl_isMyPropertyName( rEvt.PropertyName ) )
    {
        //don't notify foreign events on fetchsize and fetchdirection
        if( aEvt.PropertyName == g_sPropertyNameForFetchSize
        || aEvt.PropertyName == g_sPropertyNameForFetchDirection )
            return;

        //adjust my props 'RowCount' and 'IsRowCountFinal'
        if( aEvt.PropertyName == g_sPropertyNameForCount )
        {//RowCount changed

            //check value
            sal_Int32 nNew = 0;
            if( !( aEvt.NewValue >>= nNew ) )
            {
                OSL_FAIL( "PropertyChangeEvent contains wrong data" );
                return;
            }

            impl_changeRowCount( m_nKnownCount, nNew );
        }
        else if( aEvt.PropertyName == g_sPropertyNameForFinalCount )
        {//IsRowCountFinal changed

            //check value
            bool bNew = false;
            if( !( aEvt.NewValue >>= bNew ) )
            {
                OSL_FAIL( "PropertyChangeEvent contains wrong data" );
                return;
            }
            impl_changeIsRowCountFinal( m_bFinalCount, bNew );
        }
        return;
    }


    impl_notifyPropertyChangeListeners( aEvt );
}


//virtual
void SAL_CALL CachedContentResultSet
    ::impl_vetoableChange( const PropertyChangeEvent& rEvt )
{
    impl_EnsureNotDisposed();

    //don't notify events on my properties, cause they are not vetoable
    if( CCRS_PropertySetInfo
            ::impl_isMyPropertyName( rEvt.PropertyName ) )
    {
        return;
    }


    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = false;

    impl_notifyVetoableChangeListeners( aEvt );
}


// XContentAccess methods. ( inherited ) ( -- position dependent )


#define XCONTENTACCESS_queryXXX( queryXXX, XXX, TYPE )      \
impl_EnsureNotDisposed();                                   \
osl::ResettableMutexGuard aGuard(m_aMutex);                 \
sal_Int32 nRow = m_nRow;                                    \
sal_Int32 nFetchSize = m_nFetchSize;                        \
sal_Int32 nFetchDirection = m_nFetchDirection;              \
if( !m_aCache##XXX.hasRow( nRow ) )                         \
{                                                           \
    try                                                     \
    {                                                       \
        bool isCleared = false;                             \
        if( !m_aCache##XXX.hasCausedException( nRow ) )     \
        {                                                   \
            if( !m_xFetchProviderForContentAccess.is() )    \
            {                                               \
                OSL_FAIL( "broadcaster was disposed already" ); \
                throw RuntimeException();                   \
            }                                               \
            aGuard.clear();                                 \
            isCleared = true;                               \
            if( impl_isForwardOnly() )                      \
                applyPositionToOrigin( nRow );              \
                                                            \
            FETCH_XXX( m_aCache##XXX, m_xFetchProviderForContentAccess, fetch##XXX##s ); \
        }                                                   \
        if (isCleared)                                      \
        {                                                   \
            aGuard.reset();                                 \
        }                                                   \
        if( !m_aCache##XXX.hasRow( nRow ) )                 \
        {                                                   \
            aGuard.clear();                                 \
            applyPositionToOrigin( nRow );                  \
            TYPE aRet = ContentResultSetWrapper::queryXXX();\
            if( m_xContentIdentifierMapping.is() )          \
                return m_xContentIdentifierMapping->map##XXX( aRet );\
            return aRet;                                    \
        }                                                   \
    }                                                       \
    catch (const RuntimeException&)                         \
    {                                                       \
        throw;                                              \
    }                                                       \
    catch (const Exception& e)                              \
    {                                                       \
        Any a(cppu::getCaughtException());                  \
        throw WrappedTargetRuntimeException(                \
            "wrapped Exception " + e.Message,               \
            Reference<XInterface>(), a);                    \
    }                                                       \
}                                                           \
return m_aCache##XXX.get##XXX( nRow );

// virtual
OUString SAL_CALL CachedContentResultSet
    ::queryContentIdentifierString()
{
    XCONTENTACCESS_queryXXX( queryContentIdentifierString, ContentIdentifierString, OUString )
}


// virtual
Reference< XContentIdentifier > SAL_CALL CachedContentResultSet
    ::queryContentIdentifier()
{
    XCONTENTACCESS_queryXXX( queryContentIdentifier, ContentIdentifier, Reference< XContentIdentifier > )
}


// virtual
Reference< XContent > SAL_CALL CachedContentResultSet
    ::queryContent()
{
    XCONTENTACCESS_queryXXX( queryContent, Content, Reference< XContent > )
}


// XResultSet methods. ( inherited )

//virtual

sal_Bool SAL_CALL CachedContentResultSet
    ::next()
{
    impl_EnsureNotDisposed();

    osl::ResettableMutexGuard aGuard(m_aMutex);
    //after last
    if( m_bAfterLast )
        return false;
    //last
    aGuard.clear();
    if( isLast() )
    {
        aGuard.reset();
        m_nRow++;
        m_bAfterLast = true;
        return false;
    }
    aGuard.reset();
    //known valid position
    if( impl_isKnownValidPosition( m_nRow + 1 ) )
    {
        m_nRow++;
        return true;
    }

    //unknown position
    sal_Int32 nRow = m_nRow;
    aGuard.clear();

    bool bValid = applyPositionToOrigin( nRow + 1 );

    aGuard.reset();
    m_nRow = nRow + 1;
    m_bAfterLast = !bValid;
    return bValid;
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::previous()
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    osl::ResettableMutexGuard aGuard(m_aMutex);
    //before first ?:
    if( !m_bAfterLast && !m_nRow )
        return false;
    //first ?:
    if( !m_bAfterLast && m_nKnownCount && m_nRow == 1 )
    {
        m_nRow--;
        m_bAfterLast = false;
        return false;
    }
    //known valid position ?:
    if( impl_isKnownValidPosition( m_nRow - 1 ) )
    {
        m_nRow--;
        m_bAfterLast = false;
        return true;
    }
    //unknown position:
    sal_Int32 nRow = m_nRow;
    aGuard.clear();

    bool bValid = applyPositionToOrigin( nRow - 1  );

    aGuard.reset();
    m_nRow = nRow - 1;
    m_bAfterLast = false;
    return bValid;
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::absolute( sal_Int32 row )
{
    impl_EnsureNotDisposed();

    if( !row )
        throw SQLException();

    if( impl_isForwardOnly() )
        throw SQLException();

    osl::ResettableMutexGuard aGuard(m_aMutex);

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return false;
    }
    if( row < 0 )
    {
        if( m_bFinalCount )
        {
            sal_Int32 nNewRow = m_nKnownCount + 1 + row;
            bool bValid = true;
            if( nNewRow <= 0 )
            {
                nNewRow = 0;
                bValid = false;
            }
            m_nRow = nNewRow;
            m_bAfterLast = false;
            return bValid;
        }
        //unknown final count:
        aGuard.clear();

        // Solaris has problems catching or propagating derived exceptions
        // when only the base class is known, so make ResultSetException
        // (derived from SQLException) known here:
        bool bValid;
        try
        {
            bValid = m_xResultSetOrigin->absolute( row );
        }
        catch (const ResultSetException&)
        {
            throw;
        }

        aGuard.reset();
        if( m_bFinalCount )
        {
            sal_Int32 nNewRow = m_nKnownCount + 1 + row;
            if( nNewRow < 0 )
                nNewRow = 0;
            m_nLastAppliedPos = nNewRow;
            m_nRow = nNewRow;
            m_bAfterLastApplied = m_bAfterLast = false;
            return bValid;
        }
        aGuard.clear();

        sal_Int32 nCurRow = m_xResultSetOrigin->getRow();

        aGuard.reset();
        m_nLastAppliedPos = nCurRow;
        m_nRow = nCurRow;
        m_bAfterLast = false;
        return nCurRow != 0;
    }
    //row > 0:
    if( m_bFinalCount )
    {
        if( row > m_nKnownCount )
        {
            m_nRow = m_nKnownCount + 1;
            m_bAfterLast = true;
            return false;
        }
        m_nRow = row;
        m_bAfterLast = false;
        return true;
    }
    //unknown new position:
    aGuard.clear();

    bool bValid = m_xResultSetOrigin->absolute( row );

    aGuard.reset();
    if( m_bFinalCount )
    {
        sal_Int32 nNewRow = row;
        if( nNewRow > m_nKnownCount )
        {
            nNewRow = m_nKnownCount + 1;
            m_bAfterLastApplied = m_bAfterLast = true;
        }
        else
            m_bAfterLastApplied = m_bAfterLast = false;

        m_nLastAppliedPos = nNewRow;
        m_nRow = nNewRow;
        return bValid;
    }
    aGuard.clear();

    sal_Int32 nCurRow = m_xResultSetOrigin->getRow();
    bool bIsAfterLast = m_xResultSetOrigin->isAfterLast();

    aGuard.reset();
    m_nLastAppliedPos = nCurRow;
    m_nRow = nCurRow;
    m_bAfterLastApplied = m_bAfterLast = bIsAfterLast;
    return nCurRow && !bIsAfterLast;
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::relative( sal_Int32 rows )
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    osl::ResettableMutexGuard aGuard(m_aMutex);
    if( m_bAfterLast || impl_isKnownInvalidPosition( m_nRow ) )
        throw SQLException();

    if( !rows )
        return true;

    sal_Int32 nNewRow = m_nRow + rows;
        if( nNewRow < 0 )
            nNewRow = 0;

    if( impl_isKnownValidPosition( nNewRow ) )
    {
        m_nRow = nNewRow;
        m_bAfterLast = false;
        return true;
    }
    else
    {
        //known invalid new position:
        if( nNewRow == 0 )
        {
            m_bAfterLast = false;
            m_nRow = 0;
            return false;
        }
        if( m_bFinalCount && nNewRow > m_nKnownCount )
        {
            m_bAfterLast = true;
            m_nRow = m_nKnownCount + 1;
            return false;
        }
        //unknown new position:
        aGuard.clear();
        bool bValid = applyPositionToOrigin( nNewRow );

        aGuard.reset();
        m_nRow = nNewRow;
        m_bAfterLast = !bValid && nNewRow > 0;
        return bValid;
    }
}


//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::first()
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    osl::ResettableMutexGuard aGuard(m_aMutex);
    if( impl_isKnownValidPosition( 1 ) )
    {
        m_nRow = 1;
        m_bAfterLast = false;
        return true;
    }
    if( impl_isKnownInvalidPosition( 1 ) )
    {
        m_nRow = 1;
        m_bAfterLast = false;
        return false;
    }
    //unknown position
    aGuard.clear();

    bool bValid = applyPositionToOrigin( 1 );

    aGuard.reset();
    m_nRow = 1;
    m_bAfterLast = false;
    return bValid;
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::last()
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    osl::ResettableMutexGuard aGuard(m_aMutex);
    if( m_bFinalCount )
    {
        m_nRow = m_nKnownCount;
        m_bAfterLast = false;
        return m_nKnownCount != 0;
    }
    //unknown position
    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return false;
    }
    aGuard.clear();

    bool bValid = m_xResultSetOrigin->last();

    aGuard.reset();
    m_bAfterLastApplied = m_bAfterLast = false;
    if( m_bFinalCount )
    {
        m_nLastAppliedPos = m_nKnownCount;
        m_nRow = m_nKnownCount;
        return bValid;
    }
    aGuard.clear();

    sal_Int32 nCurRow = m_xResultSetOrigin->getRow();

    aGuard.reset();
    m_nLastAppliedPos = nCurRow;
    m_nRow = nCurRow;
    OSL_ENSURE( nCurRow >= m_nKnownCount, "position of last row < known Count, that could not be" );
    m_nKnownCount = nCurRow;
    m_bFinalCount = true;
    return nCurRow != 0;
}

//virtual
void SAL_CALL CachedContentResultSet
    ::beforeFirst()
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_nRow = 0;
    m_bAfterLast = false;
}

//virtual
void SAL_CALL CachedContentResultSet
    ::afterLast()
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_nRow = 1;
    m_bAfterLast = true;
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::isAfterLast()
{
    impl_EnsureNotDisposed();

    osl::ResettableMutexGuard aGuard(m_aMutex);
    if( !m_bAfterLast )
        return false;
    if( m_nKnownCount )
        return m_bAfterLast;
    if( m_bFinalCount )
        return false;

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return false;
    }
    aGuard.clear();

    //find out whethter the original resultset contains rows or not
    m_xResultSetOrigin->afterLast();

    aGuard.reset();
    m_bAfterLastApplied = true;
    aGuard.clear();

    return m_xResultSetOrigin->isAfterLast();
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::isBeforeFirst()
{
    impl_EnsureNotDisposed();

    osl::ResettableMutexGuard aGuard(m_aMutex);
    if( m_bAfterLast )
        return false;
    if( m_nRow )
        return false;
    if( m_nKnownCount )
        return !m_nRow;
    if( m_bFinalCount )
        return false;

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return false;
    }
    aGuard.clear();

    //find out whethter the original resultset contains rows or not
    m_xResultSetOrigin->beforeFirst();

    aGuard.reset();
    m_bAfterLastApplied = false;
    m_nLastAppliedPos = 0;
    aGuard.clear();

    return m_xResultSetOrigin->isBeforeFirst();
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::isFirst()
{
    impl_EnsureNotDisposed();

    sal_Int32 nRow = 0;
    Reference< XResultSet > xResultSetOrigin;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_bAfterLast )
            return false;
        if( m_nRow != 1 )
            return false;
        if( m_nKnownCount )
            return m_nRow == 1;
        if( m_bFinalCount )
            return false;

        nRow = m_nRow;
        xResultSetOrigin = m_xResultSetOrigin;
    }

    //need to ask origin
    {
        if( applyPositionToOrigin( nRow ) )
            return xResultSetOrigin->isFirst();
        else
            return false;
    }
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::isLast()
{
    impl_EnsureNotDisposed();

    sal_Int32 nRow = 0;
    Reference< XResultSet > xResultSetOrigin;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_bAfterLast )
            return false;
        if( m_nRow < m_nKnownCount )
            return false;
        if( m_bFinalCount )
            return m_nKnownCount && m_nRow == m_nKnownCount;

        nRow = m_nRow;
        xResultSetOrigin = m_xResultSetOrigin;
    }

    //need to ask origin
    {
        if( applyPositionToOrigin( nRow ) )
            return xResultSetOrigin->isLast();
        else
            return false;
    }
}


//virtual
sal_Int32 SAL_CALL CachedContentResultSet
    ::getRow()
{
    impl_EnsureNotDisposed();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if( m_bAfterLast )
        return 0;
    return m_nRow;
}

//virtual
void SAL_CALL CachedContentResultSet
    ::refreshRow()
{
    impl_EnsureNotDisposed();

    //the ContentResultSet is static and will not change
    //therefore we don't need to reload anything
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::rowUpdated()
{
    impl_EnsureNotDisposed();

    //the ContentResultSet is static and will not change
    return false;
}
//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::rowInserted()
{
    impl_EnsureNotDisposed();

    //the ContentResultSet is static and will not change
    return false;
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::rowDeleted()
{
    impl_EnsureNotDisposed();

    //the ContentResultSet is static and will not change
    return false;
}

//virtual
Reference< XInterface > SAL_CALL CachedContentResultSet
    ::getStatement()
{
    impl_EnsureNotDisposed();
    //@todo ?return anything
    return Reference< XInterface >();
}


// XRow methods. ( inherited )


//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::wasNull()
{
    impl_EnsureNotDisposed();
    impl_init_xRowOrigin();
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_bLastReadWasFromCache )
            return m_bLastCachedReadWasNull;
        if( !m_xRowOrigin.is() )
        {
            OSL_FAIL( "broadcaster was disposed already" );
            return false;
        }
    }
    return m_xRowOrigin->wasNull();
}

//virtual
OUString SAL_CALL CachedContentResultSet
    ::getString( sal_Int32 columnIndex )
{
    return rowOriginGet<OUString>(&css::sdbc::XRow::getString, columnIndex);
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::getBoolean( sal_Int32 columnIndex )
{
    return rowOriginGet<sal_Bool>(&css::sdbc::XRow::getBoolean, columnIndex);
}

//virtual
sal_Int8 SAL_CALL CachedContentResultSet
    ::getByte( sal_Int32 columnIndex )
{
    return rowOriginGet<sal_Int8>(&css::sdbc::XRow::getByte, columnIndex);
}

//virtual
sal_Int16 SAL_CALL CachedContentResultSet
    ::getShort( sal_Int32 columnIndex )
{
    return rowOriginGet<sal_Int16>(&css::sdbc::XRow::getShort, columnIndex);
}

//virtual
sal_Int32 SAL_CALL CachedContentResultSet
    ::getInt( sal_Int32 columnIndex )
{
    return rowOriginGet<sal_Int32>(&css::sdbc::XRow::getInt, columnIndex);
}

//virtual
sal_Int64 SAL_CALL CachedContentResultSet
    ::getLong( sal_Int32 columnIndex )
{
    return rowOriginGet<sal_Int64>(&css::sdbc::XRow::getLong, columnIndex);
}

//virtual
float SAL_CALL CachedContentResultSet
    ::getFloat( sal_Int32 columnIndex )
{
    return rowOriginGet<float>(&css::sdbc::XRow::getFloat, columnIndex);
}

//virtual
double SAL_CALL CachedContentResultSet
    ::getDouble( sal_Int32 columnIndex )
{
    return rowOriginGet<double>(&css::sdbc::XRow::getDouble, columnIndex);
}

//virtual
Sequence< sal_Int8 > SAL_CALL CachedContentResultSet
    ::getBytes( sal_Int32 columnIndex )
{
    return rowOriginGet< css::uno::Sequence<sal_Int8> >(
        &css::sdbc::XRow::getBytes, columnIndex);
}

//virtual
Date SAL_CALL CachedContentResultSet
    ::getDate( sal_Int32 columnIndex )
{
    return rowOriginGet<css::util::Date>(
        &css::sdbc::XRow::getDate, columnIndex);
}

//virtual
Time SAL_CALL CachedContentResultSet
    ::getTime( sal_Int32 columnIndex )
{
    return rowOriginGet<css::util::Time>(
        &css::sdbc::XRow::getTime, columnIndex);
}

//virtual
DateTime SAL_CALL CachedContentResultSet
    ::getTimestamp( sal_Int32 columnIndex )
{
    return rowOriginGet<css::util::DateTime>(
        &css::sdbc::XRow::getTimestamp, columnIndex);
}

//virtual
Reference< css::io::XInputStream >
    SAL_CALL CachedContentResultSet
    ::getBinaryStream( sal_Int32 columnIndex )
{
    return rowOriginGet< css::uno::Reference<css::io::XInputStream> >(
        &css::sdbc::XRow::getBinaryStream, columnIndex);
}

//virtual
Reference< css::io::XInputStream >
    SAL_CALL CachedContentResultSet
    ::getCharacterStream( sal_Int32 columnIndex )
{
    return rowOriginGet< css::uno::Reference<css::io::XInputStream> >(
        &css::sdbc::XRow::getCharacterStream, columnIndex);
}

//virtual
Any SAL_CALL CachedContentResultSet
    ::getObject( sal_Int32 columnIndex,
           const Reference<
            css::container::XNameAccess >& typeMap )
{
    //if you change this function please pay attention to
    //function template rowOriginGet, where this is similar implemented

    osl::ResettableMutexGuard aGuard(m_aMutex);
    sal_Int32 nRow = m_nRow;
    sal_Int32 nFetchSize = m_nFetchSize;
    sal_Int32 nFetchDirection = m_nFetchDirection;
    if( !m_aCache.hasRow( nRow ) )
    {
        bool isCleared = false;
        if( !m_aCache.hasCausedException( nRow ) )
        {
            if( !m_xFetchProvider.is() )
            {
                OSL_FAIL( "broadcaster was disposed already" );
                return Any();
            }
            isCleared = true;
            aGuard.clear();

            impl_fetchData( nRow, nFetchSize, nFetchDirection );
        }
        if (isCleared)
        {
            aGuard.reset();
        }
        if( !m_aCache.hasRow( nRow ) )
        {
            m_bLastReadWasFromCache = false;
            aGuard.clear();
            applyPositionToOrigin( nRow );
            impl_init_xRowOrigin();
            return m_xRowOrigin->getObject( columnIndex, typeMap );
        }
    }
    //@todo: pay attention to typeMap
    const Any& rValue = m_aCache.getAny( nRow, columnIndex );
    m_bLastReadWasFromCache = true;
    m_bLastCachedReadWasNull = !rValue.hasValue();
    return rValue;
}

//virtual
Reference< XRef > SAL_CALL CachedContentResultSet
    ::getRef( sal_Int32 columnIndex )
{
    return rowOriginGet< css::uno::Reference<css::sdbc::XRef> >(
        &css::sdbc::XRow::getRef, columnIndex);
}

//virtual
Reference< XBlob > SAL_CALL CachedContentResultSet
    ::getBlob( sal_Int32 columnIndex )
{
    return rowOriginGet< css::uno::Reference<css::sdbc::XBlob> >(
        &css::sdbc::XRow::getBlob, columnIndex);
}

//virtual
Reference< XClob > SAL_CALL CachedContentResultSet
    ::getClob( sal_Int32 columnIndex )
{
    return rowOriginGet< css::uno::Reference<css::sdbc::XClob> >(
        &css::sdbc::XRow::getClob, columnIndex);
}

//virtual
Reference< XArray > SAL_CALL CachedContentResultSet
    ::getArray( sal_Int32 columnIndex )
{
    return rowOriginGet< css::uno::Reference<css::sdbc::XArray> >(
        &css::sdbc::XRow::getArray, columnIndex);
}


// Type Converter Support


const Reference< XTypeConverter >& CachedContentResultSet::getTypeConverter()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_bTriedToGetTypeConverter && !m_xTypeConverter.is() )
    {
        m_bTriedToGetTypeConverter = true;
        m_xTypeConverter.set( Converter::create(m_xContext) );

        OSL_ENSURE( m_xTypeConverter.is(),
                    "PropertyValueSet::getTypeConverter() - "
                    "Service 'com.sun.star.script.Converter' n/a!" );
    }
    return m_xTypeConverter;
}


// class CachedContentResultSetFactory


CachedContentResultSetFactory::CachedContentResultSetFactory(
        const Reference< XComponentContext > & rxContext )
{
    m_xContext = rxContext;
}

CachedContentResultSetFactory::~CachedContentResultSetFactory()
{
}


// CachedContentResultSetFactory XInterface methods.
void SAL_CALL CachedContentResultSetFactory::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL CachedContentResultSetFactory::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL CachedContentResultSetFactory::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< XTypeProvider* >(this)),
                                               (static_cast< XServiceInfo* >(this)),
                                               (static_cast< XCachedContentResultSetFactory* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// CachedContentResultSetFactory XTypeProvider methods.


XTYPEPROVIDER_IMPL_3( CachedContentResultSetFactory,
                      XTypeProvider,
                         XServiceInfo,
                      XCachedContentResultSetFactory );


// CachedContentResultSetFactory XServiceInfo methods.

XSERVICEINFO_COMMOM_IMPL( CachedContentResultSetFactory,
                          OUString( "com.sun.star.comp.ucb.CachedContentResultSetFactory" ) )
/// @throws css::uno::Exception
static css::uno::Reference< css::uno::XInterface > SAL_CALL
CachedContentResultSetFactory_CreateInstance( const css::uno::Reference< css::lang::XMultiServiceFactory> & rSMgr )
{
    css::lang::XServiceInfo* pX =
        static_cast<css::lang::XServiceInfo*>(new CachedContentResultSetFactory( ucbhelper::getComponentContext(rSMgr) ));
    return css::uno::Reference< css::uno::XInterface >::query( pX );
}

css::uno::Sequence< OUString >
CachedContentResultSetFactory::getSupportedServiceNames_Static()
{
    css::uno::Sequence< OUString > aSNS { CACHED_CONTENT_RESULTSET_FACTORY_NAME };
    return aSNS;
}

// Service factory implementation.


ONE_INSTANCE_SERVICE_FACTORY_IMPL( CachedContentResultSetFactory );


// CachedContentResultSetFactory XCachedContentResultSetFactory methods.


    //virtual
Reference< XResultSet > SAL_CALL CachedContentResultSetFactory
    ::createCachedContentResultSet(
            const Reference< XResultSet > & xSource,
            const Reference< XContentIdentifierMapping > & xMapping )
{
    Reference< XResultSet > xRet;
    xRet = new CachedContentResultSet( m_xContext, xSource, xMapping );
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
