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


#include <cachedcontentresultset.hxx>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/ucb/FetchError.hpp>
#include <com/sun/star/ucb/ResultSetException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <comphelper/processfactory.hxx>

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




template<typename T> T CachedContentResultSet::rowOriginGet(
    T (SAL_CALL css::sdbc::XRow::* f)(sal_Int32), sal_Int32 columnIndex)
{
    impl_EnsureNotDisposed();
    ReacquireableGuard aGuard( m_aMutex );
    sal_Int32 nRow = m_nRow;
    sal_Int32 nFetchSize = m_nFetchSize;
    sal_Int32 nFetchDirection = m_nFetchDirection;
    if( !m_aCache.hasRow( nRow ) )
    {
        if( !m_aCache.hasCausedException( nRow ) )
        {
            if( !m_xFetchProvider.is() )
            {
                OSL_FAIL( "broadcaster was disposed already" );
                throw SQLException();
            }
            aGuard.clear();
            if( impl_isForwardOnly() )
                applyPositionToOrigin( nRow );

            impl_fetchData( nRow, nFetchSize, nFetchDirection );
        }
        aGuard.reacquire();
        if( !m_aCache.hasRow( nRow ) )
        {
            m_bLastReadWasFromCache = sal_False;
            aGuard.clear();
            applyPositionToOrigin( nRow );
            impl_init_xRowOrigin();
            return (m_xRowOrigin.get()->*f)( columnIndex );
        }
    }
    const Any& rValue = m_aCache.getAny( nRow, columnIndex );
    T aRet = T();
    m_bLastReadWasFromCache = sal_True;
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
                    getCppuType( static_cast<
                                 const T * >( 0 ) ) );
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







CachedContentResultSet::CCRS_Cache::CCRS_Cache(
    const Reference< XContentIdentifierMapping > & xMapping )
    : m_pResult( NULL )
    , m_xContentIdentifierMapping( xMapping )
    , m_pMappedReminder( NULL )
{
}

CachedContentResultSet::CCRS_Cache::~CCRS_Cache()
{
    delete m_pResult;
}

void SAL_CALL CachedContentResultSet::CCRS_Cache
    ::clear()
{
    if( m_pResult )
    {
        delete m_pResult;
        m_pResult = NULL;
    }
    clearMappedReminder();
}

void SAL_CALL CachedContentResultSet::CCRS_Cache
    ::loadData( const FetchResult& rResult )
{
    clear();
    m_pResult = new FetchResult( rResult );
}

sal_Bool SAL_CALL CachedContentResultSet::CCRS_Cache
    ::hasRow( sal_Int32 row )
{
    if( !m_pResult )
        return sal_False;
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

sal_Bool SAL_CALL CachedContentResultSet::CCRS_Cache
    ::hasKnownLast()
{
    if( !m_pResult )
        return sal_False;

    if( ( m_pResult->FetchError & FetchError::ENDOFDATA )
        && m_pResult->Orientation
        && m_pResult->Rows.getLength() )
        return sal_True;

    return sal_False;
}

sal_Bool SAL_CALL CachedContentResultSet::CCRS_Cache
    ::hasCausedException( sal_Int32 nRow )
{
    if( !m_pResult )
        return sal_False;
    if( !( m_pResult->FetchError & FetchError::EXCEPTION ) )
        return sal_False;

    long nEnd = m_pResult->StartIndex;
    if( m_pResult->Orientation )
        nEnd += m_pResult->Rows.getLength();

    return nRow == nEnd+1;
}

Any& SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getRowAny( sal_Int32 nRow )
    throw( SQLException,
    RuntimeException )
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
    
    if( !m_pResult )
        return;
    long nDiff = nRow - m_pResult->StartIndex;
    if( nDiff < 0 )
        nDiff *= -1;
    Sequence< sal_Bool >* pMappedReminder = getMappedReminder();
    if( nDiff < pMappedReminder->getLength() )
        (*pMappedReminder)[nDiff] = sal_True;
}

sal_Bool SAL_CALL CachedContentResultSet::CCRS_Cache
    ::isRowMapped( sal_Int32 nRow )
{
    if( !m_pMappedReminder || !m_pResult )
        return sal_False;
    long nDiff = nRow - m_pResult->StartIndex;
    if( nDiff < 0 )
        nDiff *= -1;
    if( nDiff < m_pMappedReminder->getLength() )
        return (*m_pMappedReminder)[nDiff];
    return sal_False;
}

void SAL_CALL CachedContentResultSet::CCRS_Cache
    ::clearMappedReminder()
{
    delete m_pMappedReminder;
    m_pMappedReminder = NULL;
}

Sequence< sal_Bool >* SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getMappedReminder()
{
    if( !m_pMappedReminder )
    {
        sal_Int32 nCount = m_pResult->Rows.getLength();
        m_pMappedReminder = new Sequence< sal_Bool >( nCount );
        for( ;nCount; nCount-- )
            (*m_pMappedReminder)[nCount] = sal_False;
    }
    return m_pMappedReminder;
}

const Any& SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getAny( sal_Int32 nRow, sal_Int32 nColumnIndex )
    throw( SQLException,
    RuntimeException )
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
    const Sequence< Any >& rRow =
        (* reinterpret_cast< const Sequence< Any > * >
        (getRowAny( nRow ).getValue() ));

    if( nColumnIndex > rRow.getLength() )
        throw SQLException();
    return rRow[nColumnIndex-1];
}

const OUString& SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getContentIdentifierString( sal_Int32 nRow )
    throw( com::sun::star::uno::RuntimeException )
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
        return (* reinterpret_cast< const OUString * >
                (getRowAny( nRow ).getValue() ));
    }
    catch(const SQLException&)
    {
        throw RuntimeException();
    }
}

const Reference< XContentIdentifier >& SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getContentIdentifier( sal_Int32 nRow )
    throw( com::sun::star::uno::RuntimeException )
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
        return (* reinterpret_cast< const Reference< XContentIdentifier > * >
                (getRowAny( nRow ).getValue() ));
    }
    catch(const SQLException&)
    {
        throw RuntimeException();
    }
}

const Reference< XContent >& SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getContent( sal_Int32 nRow )
    throw( com::sun::star::uno::RuntimeException )
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
        return (* reinterpret_cast< const Reference< XContent > * >
                (getRowAny( nRow ).getValue() ));
    }
    catch (const SQLException&)
    {
        throw RuntimeException();
    }
}







class CCRS_PropertySetInfo :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::beans::XPropertySetInfo
{
    friend class CachedContentResultSet;

    
    Sequence< com::sun::star::beans::Property >*
                            m_pProperties;

    
    static OUString m_aPropertyNameForCount;
    static OUString m_aPropertyNameForFinalCount;
    static OUString m_aPropertyNameForFetchSize;
    static OUString m_aPropertyNameForFetchDirection;

    long                    m_nFetchSizePropertyHandle;
    long                    m_nFetchDirectionPropertyHandle;

private:
    sal_Int32 SAL_CALL
    impl_getRemainedHandle() const;

    sal_Bool SAL_CALL
    impl_queryProperty(
            const OUString& rName
            , com::sun::star::beans::Property& rProp ) const;
    sal_Int32 SAL_CALL
    impl_getPos( const OUString& rName ) const;

    static sal_Bool SAL_CALL
    impl_isMyPropertyName( const OUString& rName );

public:
    CCRS_PropertySetInfo(   Reference<
            XPropertySetInfo > xPropertySetInfoOrigin );

    virtual ~CCRS_PropertySetInfo();

    
    XINTERFACE_DECL()

    
    XTYPEPROVIDER_DECL()

    
    virtual Sequence< com::sun::star::beans::Property > SAL_CALL
    getProperties()
        throw( RuntimeException );

    virtual com::sun::star::beans::Property SAL_CALL
    getPropertyByName( const OUString& aName )
        throw( com::sun::star::beans::UnknownPropertyException, RuntimeException );

    virtual sal_Bool SAL_CALL
    hasPropertyByName( const OUString& Name )
        throw( RuntimeException );
};

OUString    CCRS_PropertySetInfo::m_aPropertyNameForCount( "RowCount" );
OUString    CCRS_PropertySetInfo::m_aPropertyNameForFinalCount( "IsRowCountFinal" );
OUString    CCRS_PropertySetInfo::m_aPropertyNameForFetchSize( "FetchSize" );
OUString    CCRS_PropertySetInfo::m_aPropertyNameForFetchDirection( "FetchDirection" );

CCRS_PropertySetInfo::CCRS_PropertySetInfo(
        Reference< XPropertySetInfo > xInfo )
        : m_pProperties( NULL )
        , m_nFetchSizePropertyHandle( -1 )
        , m_nFetchDirectionPropertyHandle( -1 )
{
    

    
    
    

    if( xInfo.is() )
    {
        Sequence<Property> aProps = xInfo->getProperties();
        m_pProperties = new Sequence<Property> ( aProps );
    }
    else
    {
        OSL_FAIL( "The received XPropertySetInfo doesn't contain required properties" );
        m_pProperties = new Sequence<Property>;
    }

    
    sal_Int32 nFetchSize = impl_getPos( m_aPropertyNameForFetchSize );
    sal_Int32 nFetchDirection = impl_getPos( m_aPropertyNameForFetchDirection );
    sal_Int32 nDeleted = 0;
    if( nFetchSize != -1 )
        nDeleted++;
    if( nFetchDirection != -1 )
        nDeleted++;

    Sequence< Property >* pOrigProps = new Sequence<Property> ( *m_pProperties );
    sal_Int32 nOrigProps = pOrigProps->getLength();

    m_pProperties->realloc( nOrigProps + 2 - nDeleted );
    for( sal_Int32 n = 0, m = 0; n < nOrigProps; n++, m++ )
    {
        if( n == nFetchSize || n == nFetchDirection )
            m--;
        else
            (*m_pProperties)[ m ] = (*pOrigProps)[ n ];
    }
    {
        Property& rMyProp = (*m_pProperties)[ nOrigProps - nDeleted ];
        rMyProp.Name = m_aPropertyNameForFetchSize;
        rMyProp.Type = getCppuType( static_cast< const sal_Int32 * >( 0 ) );
        rMyProp.Attributes = PropertyAttribute::BOUND | PropertyAttribute::MAYBEDEFAULT;

        if( nFetchSize != -1 )
            m_nFetchSizePropertyHandle = (*pOrigProps)[nFetchSize].Handle;
        else
            m_nFetchSizePropertyHandle = impl_getRemainedHandle();

        rMyProp.Handle = m_nFetchSizePropertyHandle;

    }
    {
        Property& rMyProp = (*m_pProperties)[ nOrigProps - nDeleted + 1 ];
        rMyProp.Name = m_aPropertyNameForFetchDirection;
        rMyProp.Type = getCppuType( static_cast< const sal_Bool * >( 0 ) );
        rMyProp.Attributes = PropertyAttribute::BOUND | PropertyAttribute::MAYBEDEFAULT;

        if( nFetchDirection != -1 )
            m_nFetchDirectionPropertyHandle = (*pOrigProps)[nFetchDirection].Handle;
        else
            m_nFetchDirectionPropertyHandle = impl_getRemainedHandle();

        m_nFetchDirectionPropertyHandle = rMyProp.Handle;
    }
    delete pOrigProps;
}

CCRS_PropertySetInfo::~CCRS_PropertySetInfo()
{
    delete m_pProperties;
}





XINTERFACE_IMPL_2( CCRS_PropertySetInfo
                  , XTypeProvider
                  , XPropertySetInfo
                  );





XTYPEPROVIDER_IMPL_2( CCRS_PropertySetInfo
                    , XTypeProvider
                    , XPropertySetInfo
                    );




Sequence< Property > SAL_CALL CCRS_PropertySetInfo
    ::getProperties() throw( RuntimeException )
{
    return *m_pProperties;
}


Property SAL_CALL CCRS_PropertySetInfo
    ::getPropertyByName( const OUString& aName )
        throw( UnknownPropertyException, RuntimeException )
{
    if ( aName.isEmpty() )
        throw UnknownPropertyException();

    Property aProp;
    if ( impl_queryProperty( aName, aProp ) )
        return aProp;

    throw UnknownPropertyException();
}


sal_Bool SAL_CALL CCRS_PropertySetInfo
    ::hasPropertyByName( const OUString& Name )
        throw( RuntimeException )
{
    return ( impl_getPos( Name ) != -1 );
}





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

sal_Bool SAL_CALL CCRS_PropertySetInfo
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

            return sal_True;
        }
    }
    return sal_False;
}


sal_Bool SAL_CALL CCRS_PropertySetInfo
        ::impl_isMyPropertyName( const OUString& rPropertyName )
{
    return ( rPropertyName == m_aPropertyNameForCount
    || rPropertyName == m_aPropertyNameForFinalCount
    || rPropertyName == m_aPropertyNameForFetchSize
    || rPropertyName == m_aPropertyNameForFetchDirection );
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
    sal_Bool bFound = sal_True;
    while( bFound )
    {
        bFound = sal_False;
        for( sal_Int32 nN = m_pProperties->getLength(); nN--; )
        {
            if( nHandle == (*m_pProperties)[nN].Handle )
            {
                bFound = sal_True;
                nHandle++;
                break;
            }
        }
    }
    return nHandle;
}







CachedContentResultSet::CachedContentResultSet(
                  const Reference< XComponentContext > & rxContext
                , const Reference< XResultSet > & xOrigin
                , const Reference< XContentIdentifierMapping > &
                    xContentIdentifierMapping )
                : ContentResultSetWrapper( xOrigin )

                , m_xContext( rxContext )
                , m_xFetchProvider( NULL )
                , m_xFetchProviderForContentAccess( NULL )

                , m_xMyPropertySetInfo( NULL )
                , m_pMyPropSetInfo( NULL )

                , m_xContentIdentifierMapping( xContentIdentifierMapping )
                , m_nRow( 0 ) 
                , m_bAfterLast( sal_False )
                , m_nLastAppliedPos( 0 )
                , m_bAfterLastApplied( sal_False )
                , m_nKnownCount( 0 )
                , m_bFinalCount( sal_False )
                , m_nFetchSize(
                    COMSUNSTARUCBCCRS_DEFAULT_FETCH_SIZE )
                , m_nFetchDirection(
                    COMSUNSTARUCBCCRS_DEFAULT_FETCH_DIRECTION )

                , m_bLastReadWasFromCache( sal_False )
                , m_bLastCachedReadWasNull( sal_True )
                , m_aCache( m_xContentIdentifierMapping )
                , m_aCacheContentIdentifierString( m_xContentIdentifierMapping )
                , m_aCacheContentIdentifier( m_xContentIdentifierMapping )
                , m_aCacheContent( m_xContentIdentifierMapping )
                , m_bTriedToGetTypeConverter( sal_False )
                , m_xTypeConverter( NULL )
{
    m_xFetchProvider = Reference< XFetchProvider >( m_xResultSetOrigin, UNO_QUERY );
    OSL_ENSURE( m_xFetchProvider.is(), "interface XFetchProvider is required" );

    m_xFetchProviderForContentAccess = Reference< XFetchProviderForContentAccess >( m_xResultSetOrigin, UNO_QUERY );
    OSL_ENSURE( m_xFetchProviderForContentAccess.is(), "interface XFetchProviderForContentAccess is required" );

    impl_init();
};

CachedContentResultSet::~CachedContentResultSet()
{
    impl_deinit();
    
};





sal_Bool SAL_CALL CachedContentResultSet
    ::applyPositionToOrigin( sal_Int32 nRow )
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    
    /**
    @returns
        <TRUE/> if the cursor is on a valid row; <FALSE/> if it is off
        the result set.
    */

    ReacquireableGuard aGuard( m_aMutex );
    OSL_ENSURE( nRow >= 0, "only positive values supported" );
    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return sal_False;
    }


    sal_Int32 nLastAppliedPos = m_nLastAppliedPos;
    sal_Bool bAfterLastApplied = m_bAfterLastApplied;
    sal_Bool bAfterLast = m_bAfterLast;
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

            aGuard.reacquire();
            m_nLastAppliedPos += nM;
            m_bAfterLastApplied = nRow != m_nLastAppliedPos;
            return nRow == m_nLastAppliedPos;
        }

        if( !nRow ) 
        {
            m_xResultSetOrigin->beforeFirst();

            aGuard.reacquire();
            m_nLastAppliedPos = 0;
            m_bAfterLastApplied = sal_False;
            return sal_False;
        }
        try
        {
            
            
            if( !nLastAppliedPos || bAfterLast || bAfterLastApplied )
            {
                sal_Bool bValid = m_xResultSetOrigin->absolute( nRow );

                aGuard.reacquire();
                m_nLastAppliedPos = nRow;
                m_bAfterLastApplied = !bValid;
                return bValid;
            }
            else
            {
                sal_Bool bValid = m_xResultSetOrigin->relative( nRow - nLastAppliedPos );

                aGuard.reacquire();
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

                aGuard.reacquire();
                m_nLastAppliedPos += nM;
                m_bAfterLastApplied = nRow != m_nLastAppliedPos;
            }
            else
                throw;
        }

        return nRow == m_nLastAppliedPos;
    }
    return sal_True;
};







#define FETCH_XXX( aCache, fetchInterface, fetchMethod )            \
sal_Bool bDirection = !!(                                           \
    nFetchDirection != FetchDirection::REVERSE );                   \
FetchResult aResult =                                               \
    fetchInterface->fetchMethod( nRow, nFetchSize, bDirection );    \
osl::ClearableGuard< osl::Mutex > aGuard2( m_aMutex );              \
aCache.loadData( aResult );                                         \
sal_Int32 nMax = aCache.getMaxRow();                                \
sal_Int32 nCurCount = m_nKnownCount;                                \
sal_Bool bIsFinalCount = aCache.hasKnownLast();                     \
sal_Bool bCurIsFinalCount = m_bFinalCount;                          \
aGuard2.clear();                                                    \
if( nMax > nCurCount )                                              \
    impl_changeRowCount( nCurCount, nMax );                         \
if( bIsFinalCount && !bCurIsFinalCount )                            \
    impl_changeIsRowCountFinal( bCurIsFinalCount, bIsFinalCount );

void SAL_CALL CachedContentResultSet
    ::impl_fetchData( sal_Int32 nRow
        , sal_Int32 nFetchSize, sal_Int32 nFetchDirection )
        throw( com::sun::star::uno::RuntimeException )
{
    FETCH_XXX( m_aCache, m_xFetchProvider, fetch );
}

void SAL_CALL CachedContentResultSet
    ::impl_changeRowCount( sal_Int32 nOld, sal_Int32 nNew )
{
    OSL_ENSURE( nNew > nOld, "RowCount only can grow" );
    if( nNew <= nOld )
        return;

    
    PropertyChangeEvent aEvt;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aEvt.Source =  static_cast< XPropertySet * >( this );
        aEvt.Further = sal_False;
        aEvt.OldValue <<= nOld;
        aEvt.NewValue <<= nNew;

        m_nKnownCount = nNew;
    }

    
    impl_notifyPropertyChangeListeners( aEvt );
}

void SAL_CALL CachedContentResultSet
    ::impl_changeIsRowCountFinal( sal_Bool bOld, sal_Bool bNew )
{
    OSL_ENSURE( !bOld && bNew, "This change is not allowed for IsRowCountFinal" );
    if( ! (!bOld && bNew ) )
        return;

    
    PropertyChangeEvent aEvt;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aEvt.Source =  static_cast< XPropertySet * >( this );
        aEvt.Further = sal_False;
        aEvt.OldValue <<= bOld;
        aEvt.NewValue <<= bNew;

        m_bFinalCount = bNew;
    }

    
    impl_notifyPropertyChangeListeners( aEvt );
}

sal_Bool SAL_CALL CachedContentResultSet
    ::impl_isKnownValidPosition( sal_Int32 nRow )
{
    return m_nKnownCount && nRow
            && nRow <= m_nKnownCount;
}

sal_Bool SAL_CALL CachedContentResultSet
    ::impl_isKnownInvalidPosition( sal_Int32 nRow )
{
    if( !nRow )
        return sal_True;
    if( !m_bFinalCount )
        return sal_False;
    return nRow > m_nKnownCount;
}



void SAL_CALL CachedContentResultSet
    ::impl_initPropertySetInfo()
{
    ContentResultSetWrapper::impl_initPropertySetInfo();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if( m_pMyPropSetInfo )
        return;
    m_pMyPropSetInfo = new CCRS_PropertySetInfo( m_xPropertySetInfo );
    m_xMyPropertySetInfo = m_pMyPropSetInfo;
    m_xPropertySetInfo = m_xMyPropertySetInfo;
}




XINTERFACE_COMMON_IMPL( CachedContentResultSet )

Any SAL_CALL CachedContentResultSet
    ::queryInterface( const Type&  rType )
    throw ( RuntimeException )
{
    

    Any aRet = ContentResultSetWrapper::queryInterface( rType );
    if( aRet.hasValue() )
        return aRet;

    aRet = cppu::queryInterface( rType,
                static_cast< XTypeProvider* >( this ),
                static_cast< XServiceInfo* >( this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}





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





XSERVICEINFO_NOFACTORY_IMPL_1( CachedContentResultSet,
                               OUString(
                            "com.sun.star.comp.ucb.CachedContentResultSet" ),
                            OUString(
                            CACHED_CONTENT_RESULTSET_SERVICE_NAME ) );






void SAL_CALL CachedContentResultSet
    ::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !getPropertySetInfo().is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }

    Property aProp = m_pMyPropSetInfo->getPropertyByName( aPropertyName );
        

    if( aProp.Attributes & PropertyAttribute::READONLY )
    {
        
        
        throw IllegalArgumentException();
    }
    if( aProp.Name == CCRS_PropertySetInfo
                        ::m_aPropertyNameForFetchDirection )
    {
        
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

        
        PropertyChangeEvent aEvt;
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            aEvt.Source =  static_cast< XPropertySet * >( this );
            aEvt.PropertyName = aPropertyName;
            aEvt.Further = sal_False;
            aEvt.PropertyHandle = m_pMyPropSetInfo->
                                    m_nFetchDirectionPropertyHandle;
            aEvt.OldValue <<= m_nFetchDirection;
            aEvt.NewValue <<= nNew;

            m_nFetchDirection = nNew;
        }

        
        impl_notifyPropertyChangeListeners( aEvt );
    }
    else if( aProp.Name == CCRS_PropertySetInfo
                        ::m_aPropertyNameForFetchSize )
    {
        
        sal_Int32 nNew;
        if( !( aValue >>= nNew ) )
        {
            throw IllegalArgumentException();
        }

        if( nNew < 0 )
        {
            nNew = COMSUNSTARUCBCCRS_DEFAULT_FETCH_SIZE;
        }

        
        PropertyChangeEvent aEvt;
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            aEvt.Source =  static_cast< XPropertySet * >( this );
            aEvt.PropertyName = aPropertyName;
            aEvt.Further = sal_False;
            aEvt.PropertyHandle = m_pMyPropSetInfo->
                                    m_nFetchSizePropertyHandle;
            aEvt.OldValue <<= m_nFetchSize;
            aEvt.NewValue <<= nNew;

            m_nFetchSize = nNew;
        }

        
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



Any SAL_CALL CachedContentResultSet
    ::getPropertyValue( const OUString& rPropertyName )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !getPropertySetInfo().is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }

    Property aProp = m_pMyPropSetInfo->getPropertyByName( rPropertyName );
        

    Any aValue;
    if( rPropertyName == CCRS_PropertySetInfo
                        ::m_aPropertyNameForCount )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aValue <<= m_nKnownCount;
    }
    else if( rPropertyName == CCRS_PropertySetInfo
                            ::m_aPropertyNameForFinalCount )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aValue <<= m_bFinalCount;
    }
    else if( rPropertyName == CCRS_PropertySetInfo
                            ::m_aPropertyNameForFetchSize )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aValue <<= m_nFetchSize;
    }
    else if( rPropertyName == CCRS_PropertySetInfo
                            ::m_aPropertyNameForFetchDirection )
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






void SAL_CALL CachedContentResultSet
    ::impl_disposing( const EventObject& rEventObject )
    throw( RuntimeException )
{
    {
        impl_EnsureNotDisposed();
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        
        m_xFetchProvider.clear();
        m_xFetchProviderForContentAccess.clear();
    }
    ContentResultSetWrapper::impl_disposing( rEventObject );
}


void SAL_CALL CachedContentResultSet
    ::impl_propertyChange( const PropertyChangeEvent& rEvt )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = sal_False;
    

    if( CCRS_PropertySetInfo
            ::impl_isMyPropertyName( rEvt.PropertyName ) )
    {
        
        if( aEvt.PropertyName == CCRS_PropertySetInfo
                                ::m_aPropertyNameForFetchSize
        || aEvt.PropertyName == CCRS_PropertySetInfo
                                ::m_aPropertyNameForFetchDirection )
            return;

        
        if( aEvt.PropertyName == CCRS_PropertySetInfo
                            ::m_aPropertyNameForCount )
        {

            
            sal_Int32 nNew = 0;
            if( !( aEvt.NewValue >>= nNew ) )
            {
                OSL_FAIL( "PropertyChangeEvent contains wrong data" );
                return;
            }

            impl_changeRowCount( m_nKnownCount, nNew );
        }
        else if( aEvt.PropertyName == CCRS_PropertySetInfo
                                ::m_aPropertyNameForFinalCount )
        {

            
            sal_Bool bNew = sal_False;
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



void SAL_CALL CachedContentResultSet
    ::impl_vetoableChange( const PropertyChangeEvent& rEvt )
    throw( PropertyVetoException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    
    if( CCRS_PropertySetInfo
            ::impl_isMyPropertyName( rEvt.PropertyName ) )
    {
        return;
    }


    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = sal_False;

    impl_notifyVetoableChangeListeners( aEvt );
}





#define XCONTENTACCESS_queryXXX( queryXXX, XXX, TYPE )              \
impl_EnsureNotDisposed();                                   \
ReacquireableGuard aGuard( m_aMutex );                      \
sal_Int32 nRow = m_nRow;                                    \
sal_Int32 nFetchSize = m_nFetchSize;                        \
sal_Int32 nFetchDirection = m_nFetchDirection;              \
if( !m_aCache##XXX.hasRow( nRow ) )                         \
{                                                           \
    if( !m_aCache##XXX.hasCausedException( nRow ) )         \
{                                                           \
        if( !m_xFetchProviderForContentAccess.is() )        \
        {                                                   \
            OSL_FAIL( "broadcaster was disposed already" );\
            throw RuntimeException();                       \
        }                                                   \
        aGuard.clear();                                     \
        if( impl_isForwardOnly() )                          \
            applyPositionToOrigin( nRow );                  \
                                                            \
        FETCH_XXX( m_aCache##XXX, m_xFetchProviderForContentAccess, fetch##XXX##s ); \
    }                                                       \
    aGuard.reacquire();                                     \
    if( !m_aCache##XXX.hasRow( nRow ) )                     \
    {                                                       \
        aGuard.clear();                                     \
        applyPositionToOrigin( nRow );                      \
        TYPE aRet = ContentResultSetWrapper::queryXXX();    \
        if( m_xContentIdentifierMapping.is() )              \
            return m_xContentIdentifierMapping->map##XXX( aRet );\
        return aRet;                                        \
    }                                                       \
}                                                           \
return m_aCache##XXX.get##XXX( nRow );



OUString SAL_CALL CachedContentResultSet
    ::queryContentIdentifierString()
    throw( RuntimeException )
{
    XCONTENTACCESS_queryXXX( queryContentIdentifierString, ContentIdentifierString, OUString )
}



Reference< XContentIdentifier > SAL_CALL CachedContentResultSet
    ::queryContentIdentifier()
    throw( RuntimeException )
{
    XCONTENTACCESS_queryXXX( queryContentIdentifier, ContentIdentifier, Reference< XContentIdentifier > )
}



Reference< XContent > SAL_CALL CachedContentResultSet
    ::queryContent()
    throw( RuntimeException )
{
    XCONTENTACCESS_queryXXX( queryContent, Content, Reference< XContent > )
}






sal_Bool SAL_CALL CachedContentResultSet
    ::next()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    ReacquireableGuard aGuard( m_aMutex );
    
    if( m_bAfterLast )
        return sal_False;
    
    aGuard.clear();
    if( isLast() )
    {
        aGuard.reacquire();
        m_nRow++;
        m_bAfterLast = sal_True;
        return sal_False;
    }
    aGuard.reacquire();
    
    if( impl_isKnownValidPosition( m_nRow + 1 ) )
    {
        m_nRow++;
        return sal_True;
    }

    
    sal_Int32 nRow = m_nRow;
    aGuard.clear();

    sal_Bool bValid = applyPositionToOrigin( nRow + 1 );

    aGuard.reacquire();
    m_nRow = nRow + 1;
    m_bAfterLast = !bValid;
    return bValid;
}


sal_Bool SAL_CALL CachedContentResultSet
    ::previous()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    ReacquireableGuard aGuard( m_aMutex );
    
    if( !m_bAfterLast && !m_nRow )
        return sal_False;
    
    if( !m_bAfterLast && m_nKnownCount && m_nRow == 1 )
    {
        m_nRow--;
        m_bAfterLast = sal_False;
        return sal_False;
    }
    
    if( impl_isKnownValidPosition( m_nRow - 1 ) )
    {
        m_nRow--;
        m_bAfterLast = sal_False;
        return sal_True;
    }
    
    sal_Int32 nRow = m_nRow;
    aGuard.clear();

    sal_Bool bValid = applyPositionToOrigin( nRow - 1  );

    aGuard.reacquire();
    m_nRow = nRow - 1;
    m_bAfterLast = sal_False;
    return bValid;
}


sal_Bool SAL_CALL CachedContentResultSet
    ::absolute( sal_Int32 row )
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !row )
        throw SQLException();

    if( impl_isForwardOnly() )
        throw SQLException();

    ReacquireableGuard aGuard( m_aMutex );

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return sal_False;
    }
    if( row < 0 )
    {
        if( m_bFinalCount )
        {
            sal_Int32 nNewRow = m_nKnownCount + 1 + row;
            sal_Bool bValid = sal_True;
            if( nNewRow <= 0 )
            {
                nNewRow = 0;
                bValid = sal_False;
            }
            m_nRow = nNewRow;
            m_bAfterLast = sal_False;
            return bValid;
        }
        
        aGuard.clear();

        
        
        
        sal_Bool bValid;
        try
        {
            bValid = m_xResultSetOrigin->absolute( row );
        }
        catch (const ResultSetException&)
        {
            throw;
        }

        aGuard.reacquire();
        if( m_bFinalCount )
        {
            sal_Int32 nNewRow = m_nKnownCount + 1 + row;
            if( nNewRow < 0 )
                nNewRow = 0;
            m_nLastAppliedPos = nNewRow;
            m_nRow = nNewRow;
            m_bAfterLastApplied = m_bAfterLast = sal_False;
            return bValid;
        }
        aGuard.clear();

        sal_Int32 nCurRow = m_xResultSetOrigin->getRow();

        aGuard.reacquire();
        m_nLastAppliedPos = nCurRow;
        m_nRow = nCurRow;
        m_bAfterLast = sal_False;
        return nCurRow != 0;
    }
    
    if( m_bFinalCount )
    {
        if( row > m_nKnownCount )
        {
            m_nRow = m_nKnownCount + 1;
            m_bAfterLast = sal_True;
            return sal_False;
        }
        m_nRow = row;
        m_bAfterLast = sal_False;
        return sal_True;
    }
    
    aGuard.clear();

    sal_Bool bValid = m_xResultSetOrigin->absolute( row );

    aGuard.reacquire();
    if( m_bFinalCount )
    {
        sal_Int32 nNewRow = row;
        if( nNewRow > m_nKnownCount )
        {
            nNewRow = m_nKnownCount + 1;
            m_bAfterLastApplied = m_bAfterLast = sal_True;
        }
        else
            m_bAfterLastApplied = m_bAfterLast = sal_False;

        m_nLastAppliedPos = nNewRow;
        m_nRow = nNewRow;
        return bValid;
    }
    aGuard.clear();

    sal_Int32 nCurRow = m_xResultSetOrigin->getRow();
    sal_Bool bIsAfterLast = m_xResultSetOrigin->isAfterLast();

    aGuard.reacquire();
    m_nLastAppliedPos = nCurRow;
    m_nRow = nCurRow;
    m_bAfterLastApplied = m_bAfterLast = bIsAfterLast;
    return nCurRow && !bIsAfterLast;
}


sal_Bool SAL_CALL CachedContentResultSet
    ::relative( sal_Int32 rows )
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    ReacquireableGuard aGuard( m_aMutex );
    if( m_bAfterLast || impl_isKnownInvalidPosition( m_nRow ) )
        throw SQLException();

    if( !rows )
        return sal_True;

    sal_Int32 nNewRow = m_nRow + rows;
        if( nNewRow < 0 )
            nNewRow = 0;

    if( impl_isKnownValidPosition( nNewRow ) )
    {
        m_nRow = nNewRow;
        m_bAfterLast = sal_False;
        return sal_True;
    }
    else
    {
        
        if( nNewRow == 0 )
        {
            m_bAfterLast = sal_False;
            m_nRow = 0;
            return sal_False;
        }
        if( m_bFinalCount && nNewRow > m_nKnownCount )
        {
            m_bAfterLast = sal_True;
            m_nRow = m_nKnownCount + 1;
            return sal_False;
        }
        
        aGuard.clear();
        sal_Bool bValid = applyPositionToOrigin( nNewRow );

        aGuard.reacquire();
        m_nRow = nNewRow;
        m_bAfterLast = !bValid && nNewRow > 0;
        return bValid;
    }
}



sal_Bool SAL_CALL CachedContentResultSet
    ::first()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    ReacquireableGuard aGuard( m_aMutex );
    if( impl_isKnownValidPosition( 1 ) )
    {
        m_nRow = 1;
        m_bAfterLast = sal_False;
        return sal_True;
    }
    if( impl_isKnownInvalidPosition( 1 ) )
    {
        m_nRow = 1;
        m_bAfterLast = sal_False;
        return sal_False;
    }
    
    aGuard.clear();

    sal_Bool bValid = applyPositionToOrigin( 1 );

    aGuard.reacquire();
    m_nRow = 1;
    m_bAfterLast = sal_False;
    return bValid;
}


sal_Bool SAL_CALL CachedContentResultSet
    ::last()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    ReacquireableGuard aGuard( m_aMutex );
    if( m_bFinalCount )
    {
        m_nRow = m_nKnownCount;
        m_bAfterLast = sal_False;
        return m_nKnownCount != 0;
    }
    
    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return sal_False;
    }
    aGuard.clear();

    sal_Bool bValid = m_xResultSetOrigin->last();

    aGuard.reacquire();
    m_bAfterLastApplied = m_bAfterLast = sal_False;
    if( m_bFinalCount )
    {
        m_nLastAppliedPos = m_nKnownCount;
        m_nRow = m_nKnownCount;
        return bValid;
    }
    aGuard.clear();

    sal_Int32 nCurRow = m_xResultSetOrigin->getRow();

    aGuard.reacquire();
    m_nLastAppliedPos = nCurRow;
    m_nRow = nCurRow;
    OSL_ENSURE( nCurRow >= m_nKnownCount, "position of last row < known Count, that could not be" );
    m_nKnownCount = nCurRow;
    m_bFinalCount = sal_True;
    return nCurRow != 0;
}


void SAL_CALL CachedContentResultSet
    ::beforeFirst()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_nRow = 0;
    m_bAfterLast = sal_False;
}


void SAL_CALL CachedContentResultSet
    ::afterLast()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_nRow = 1;
    m_bAfterLast = sal_True;
}


sal_Bool SAL_CALL CachedContentResultSet
    ::isAfterLast()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    ReacquireableGuard aGuard( m_aMutex );
    if( !m_bAfterLast )
        return sal_False;
    if( m_nKnownCount )
        return m_bAfterLast;
    if( m_bFinalCount )
        return sal_False;

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return sal_False;
    }
    aGuard.clear();

    
    m_xResultSetOrigin->afterLast();

    aGuard.reacquire();
    m_bAfterLastApplied = sal_True;
    aGuard.clear();

    return m_xResultSetOrigin->isAfterLast();
}


sal_Bool SAL_CALL CachedContentResultSet
    ::isBeforeFirst()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    ReacquireableGuard aGuard( m_aMutex );
    if( m_bAfterLast )
        return sal_False;
    if( m_nRow )
        return sal_False;
    if( m_nKnownCount )
        return !m_nRow;
    if( m_bFinalCount )
        return sal_False;

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return sal_False;
    }
    aGuard.clear();

    
    m_xResultSetOrigin->beforeFirst();

    aGuard.reacquire();
    m_bAfterLastApplied = sal_False;
    m_nLastAppliedPos = 0;
    aGuard.clear();

    return m_xResultSetOrigin->isBeforeFirst();
}


sal_Bool SAL_CALL CachedContentResultSet
    ::isFirst()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    sal_Int32 nRow = 0;
    Reference< XResultSet > xResultSetOrigin;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_bAfterLast )
            return sal_False;
        if( m_nRow != 1 )
            return sal_False;
        if( m_nKnownCount )
            return m_nRow == 1;
        if( m_bFinalCount )
            return sal_False;

        nRow = m_nRow;
        xResultSetOrigin = m_xResultSetOrigin;
    }

    
    {
        if( applyPositionToOrigin( nRow ) )
            return xResultSetOrigin->isFirst();
        else
            return sal_False;
    }
}


sal_Bool SAL_CALL CachedContentResultSet
    ::isLast()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    sal_Int32 nRow = 0;
    Reference< XResultSet > xResultSetOrigin;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_bAfterLast )
            return sal_False;
        if( m_nRow < m_nKnownCount )
            return sal_False;
        if( m_bFinalCount )
            return m_nKnownCount && m_nRow == m_nKnownCount;

        nRow = m_nRow;
        xResultSetOrigin = m_xResultSetOrigin;
    }

    
    {
        if( applyPositionToOrigin( nRow ) )
            return xResultSetOrigin->isLast();
        else
            return sal_False;
    }
}



sal_Int32 SAL_CALL CachedContentResultSet
    ::getRow()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if( m_bAfterLast )
        return 0;
    return m_nRow;
}


void SAL_CALL CachedContentResultSet
    ::refreshRow()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    
    
}


sal_Bool SAL_CALL CachedContentResultSet
    ::rowUpdated()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    
    return sal_False;
}

sal_Bool SAL_CALL CachedContentResultSet
    ::rowInserted()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    
    return sal_False;
}


sal_Bool SAL_CALL CachedContentResultSet
    ::rowDeleted()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    
    return sal_False;
}


Reference< XInterface > SAL_CALL CachedContentResultSet
    ::getStatement()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    
    return Reference< XInterface >();
}






sal_Bool SAL_CALL CachedContentResultSet
    ::wasNull()
    throw( SQLException,
           RuntimeException )
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
            return sal_False;
        }
    }
    return m_xRowOrigin->wasNull();
}


OUString SAL_CALL CachedContentResultSet
    ::getString( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet<OUString>(&css::sdbc::XRow::getString, columnIndex);
}


sal_Bool SAL_CALL CachedContentResultSet
    ::getBoolean( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet<sal_Bool>(&css::sdbc::XRow::getBoolean, columnIndex);
}


sal_Int8 SAL_CALL CachedContentResultSet
    ::getByte( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet<sal_Int8>(&css::sdbc::XRow::getByte, columnIndex);
}


sal_Int16 SAL_CALL CachedContentResultSet
    ::getShort( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet<sal_Int16>(&css::sdbc::XRow::getShort, columnIndex);
}


sal_Int32 SAL_CALL CachedContentResultSet
    ::getInt( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet<sal_Int32>(&css::sdbc::XRow::getInt, columnIndex);
}


sal_Int64 SAL_CALL CachedContentResultSet
    ::getLong( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet<sal_Int64>(&css::sdbc::XRow::getLong, columnIndex);
}


float SAL_CALL CachedContentResultSet
    ::getFloat( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet<float>(&css::sdbc::XRow::getFloat, columnIndex);
}


double SAL_CALL CachedContentResultSet
    ::getDouble( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet<double>(&css::sdbc::XRow::getDouble, columnIndex);
}


Sequence< sal_Int8 > SAL_CALL CachedContentResultSet
    ::getBytes( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet< css::uno::Sequence<sal_Int8> >(
        &css::sdbc::XRow::getBytes, columnIndex);
}


Date SAL_CALL CachedContentResultSet
    ::getDate( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet<css::util::Date>(
        &css::sdbc::XRow::getDate, columnIndex);
}


Time SAL_CALL CachedContentResultSet
    ::getTime( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet<css::util::Time>(
        &css::sdbc::XRow::getTime, columnIndex);
}


DateTime SAL_CALL CachedContentResultSet
    ::getTimestamp( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet<css::util::DateTime>(
        &css::sdbc::XRow::getTimestamp, columnIndex);
}


Reference< com::sun::star::io::XInputStream >
    SAL_CALL CachedContentResultSet
    ::getBinaryStream( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet< css::uno::Reference<css::io::XInputStream> >(
        &css::sdbc::XRow::getBinaryStream, columnIndex);
}


Reference< com::sun::star::io::XInputStream >
    SAL_CALL CachedContentResultSet
    ::getCharacterStream( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet< css::uno::Reference<css::io::XInputStream> >(
        &css::sdbc::XRow::getCharacterStream, columnIndex);
}


Any SAL_CALL CachedContentResultSet
    ::getObject( sal_Int32 columnIndex,
           const Reference<
            com::sun::star::container::XNameAccess >& typeMap )
    throw( SQLException,
           RuntimeException )
{
    
    

    ReacquireableGuard aGuard( m_aMutex );
    sal_Int32 nRow = m_nRow;
    sal_Int32 nFetchSize = m_nFetchSize;
    sal_Int32 nFetchDirection = m_nFetchDirection;
    if( !m_aCache.hasRow( nRow ) )
    {
        if( !m_aCache.hasCausedException( nRow ) )
        {
            if( !m_xFetchProvider.is() )
            {
                OSL_FAIL( "broadcaster was disposed already" );
                return Any();
            }
            aGuard.clear();

            impl_fetchData( nRow, nFetchSize, nFetchDirection );
        }
        aGuard.reacquire();
        if( !m_aCache.hasRow( nRow ) )
        {
            m_bLastReadWasFromCache = sal_False;
            aGuard.clear();
            applyPositionToOrigin( nRow );
            impl_init_xRowOrigin();
            return m_xRowOrigin->getObject( columnIndex, typeMap );
        }
    }
    
    const Any& rValue = m_aCache.getAny( nRow, columnIndex );
    Any aRet;
    m_bLastReadWasFromCache = sal_True;
    m_bLastCachedReadWasNull = !( rValue >>= aRet );
    return aRet;
}


Reference< XRef > SAL_CALL CachedContentResultSet
    ::getRef( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet< css::uno::Reference<css::sdbc::XRef> >(
        &css::sdbc::XRow::getRef, columnIndex);
}


Reference< XBlob > SAL_CALL CachedContentResultSet
    ::getBlob( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet< css::uno::Reference<css::sdbc::XBlob> >(
        &css::sdbc::XRow::getBlob, columnIndex);
}


Reference< XClob > SAL_CALL CachedContentResultSet
    ::getClob( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet< css::uno::Reference<css::sdbc::XClob> >(
        &css::sdbc::XRow::getClob, columnIndex);
}


Reference< XArray > SAL_CALL CachedContentResultSet
    ::getArray( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    return rowOriginGet< css::uno::Reference<css::sdbc::XArray> >(
        &css::sdbc::XRow::getArray, columnIndex);
}





const Reference< XTypeConverter >& CachedContentResultSet::getTypeConverter()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_bTriedToGetTypeConverter && !m_xTypeConverter.is() )
    {
        m_bTriedToGetTypeConverter = sal_True;
        m_xTypeConverter = Reference< XTypeConverter >( Converter::create(m_xContext) );

        OSL_ENSURE( m_xTypeConverter.is(),
                    "PropertyValueSet::getTypeConverter() - "
                    "Service 'com.sun.star.script.Converter' n/a!" );
    }
    return m_xTypeConverter;
}







CachedContentResultSetFactory::CachedContentResultSetFactory(
        const Reference< XComponentContext > & rxContext )
{
    m_xContext = rxContext;
}

CachedContentResultSetFactory::~CachedContentResultSetFactory()
{
}





XINTERFACE_IMPL_3( CachedContentResultSetFactory,
                   XTypeProvider,
                   XServiceInfo,
                   XCachedContentResultSetFactory );





XTYPEPROVIDER_IMPL_3( CachedContentResultSetFactory,
                      XTypeProvider,
                         XServiceInfo,
                      XCachedContentResultSetFactory );





XSERVICEINFO_IMPL_1_CTX( CachedContentResultSetFactory,
                     OUString( "com.sun.star.comp.ucb.CachedContentResultSetFactory" ),
                     OUString( CACHED_CONTENT_RESULTSET_FACTORY_NAME ) );





ONE_INSTANCE_SERVICE_FACTORY_IMPL( CachedContentResultSetFactory );





    
Reference< XResultSet > SAL_CALL CachedContentResultSetFactory
    ::createCachedContentResultSet(
            const Reference< XResultSet > & xSource,
            const Reference< XContentIdentifierMapping > & xMapping )
            throw( com::sun::star::uno::RuntimeException )
{
    Reference< XResultSet > xRet;
    xRet = new CachedContentResultSet( m_xContext, xSource, xMapping );
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
