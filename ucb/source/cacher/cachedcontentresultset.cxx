/*************************************************************************
 *
 *  $RCSfile: cachedcontentresultset.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-17 10:44:57 $
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

#include <cachedcontentresultset.hxx>

#ifndef _COM_SUN_STAR_SDBC_FETCHDIRECTION_HPP_
#include <com/sun/star/sdbc/FetchDirection.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_FETCHERROR_HPP_
#include <com/sun/star/ucb/FetchError.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace cppu;
using namespace rtl;

#define COMSUNSTARUCBCCRS_DEFAULT_FETCH_SIZE 256
#define COMSUNSTARUCBCCRS_DEFAULT_FETCH_DIRECTION FetchDirection::FORWARD

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//define for getXXX methods of interface XRow
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

//if you change this macro please pay attention to
//function ::getObject, where this is similar implemented

#define XROW_GETXXX( getXXX, Type )                     \
impl_EnsureNotDisposed();                               \
ReacquireableGuard aGuard( m_aMutex );                  \
sal_Int32 nRow = m_nRow;                                \
sal_Int32 nFetchSize = m_nFetchSize;                    \
sal_Int32 nFetchDirection = m_nFetchDirection;          \
if( !m_aCache.hasRow( nRow ) )                          \
{                                                       \
    if( !m_aCache.hasCausedException( nRow ) )          \
{                                                       \
        if( !m_xFetchProvider.is() )                    \
        {                                               \
            OSL_ENSURE( sal_False, "broadcaster was disposed already" );    \
            throw SQLException();                       \
        }                                               \
        aGuard.clear();                                 \
        if( impl_isForwardOnly() )                      \
            applyPositionToOrigin( nRow );              \
                                                        \
        impl_fetchData( nRow, nFetchSize, nFetchDirection ); \
    }                                                   \
    aGuard.reacquire();                                 \
    if( !m_aCache.hasRow( nRow ) )                      \
    {                                                   \
        m_bLastReadWasFromCache = sal_False;            \
        aGuard.clear();                                 \
        applyPositionToOrigin( nRow );                  \
        return m_xRowOrigin->getXXX( columnIndex );     \
    }                                                   \
}                                                       \
const Any& rValue = m_aCache.getAny( nRow, columnIndex );\
Type aRet;                                              \
m_bLastReadWasFromCache = sal_True;                     \
m_bLastCachedReadWasNull = !( rValue >>= aRet );        \
return aRet;

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// CCRS_Cache methoeds.
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

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
    //remind that this row was mapped
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
        for( nCount; nCount--; )
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

    sal_Int32 nTest = rRow.getLength();
    if( nColumnIndex > rRow.getLength() )
        throw SQLException();
    return rRow[nColumnIndex-1];
}

const rtl::OUString& SAL_CALL CachedContentResultSet::CCRS_Cache
    ::getContentIdentifierString( sal_Int32 nRow )
    throw( com::sun::star::uno::RuntimeException )
{
    try
    {
        if( m_xContentIdentifierMapping.is() && !isRowMapped( nRow ) )
        {
            Any& rRow = getRowAny( nRow );
            rtl::OUString aValue;
            rRow >>= aValue;
            rRow <<= m_xContentIdentifierMapping->mapContentIdentifierString( aValue );
            remindMapped( nRow );
        }
        return (* reinterpret_cast< const rtl::OUString * >
                (getRowAny( nRow ).getValue() ));
    }
    catch( SQLException )
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
    catch( SQLException )
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
    catch( SQLException )
    {
        throw RuntimeException();
    }
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// class CCRS_PropertySetInfo
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

class CCRS_PropertySetInfo :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::beans::XPropertySetInfo
{
friend CachedContentResultSet;
    //my Properties
    Sequence< com::sun::star::beans::Property >*
                            m_pProperties;

    //some helping variables ( names for my special properties )
    static rtl::OUString    m_aPropertyNameForCount;
    static rtl::OUString    m_aPropertyNameForFinalCount;
    static rtl::OUString    m_aPropertyNameForFetchSize;
    static rtl::OUString    m_aPropertyNameForFetchDirection;

    long                    m_nFetchSizePropertyHandle;
    long                    m_nFetchDirectionPropertyHandle;

private:
    sal_Int32 SAL_CALL
    impl_getRemainedHandle() const;

    sal_Bool SAL_CALL
    impl_queryProperty(
            const rtl::OUString& rName
            , com::sun::star::beans::Property& rProp ) const;
    sal_Int32 SAL_CALL
    impl_getPos( const rtl::OUString& rName ) const;

    static sal_Bool SAL_CALL
    impl_isMyPropertyName( const rtl::OUString& rName );

public:
    CCRS_PropertySetInfo(   Reference<
            XPropertySetInfo > xPropertySetInfoOrigin );

    virtual ~CCRS_PropertySetInfo();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XPropertySetInfo
    virtual Sequence< com::sun::star::beans::Property > SAL_CALL
    getProperties()
        throw( RuntimeException );

    virtual com::sun::star::beans::Property SAL_CALL
    getPropertyByName( const rtl::OUString& aName )
        throw( com::sun::star::beans::UnknownPropertyException, RuntimeException );

    virtual sal_Bool SAL_CALL
    hasPropertyByName( const rtl::OUString& Name )
        throw( RuntimeException );
};

OUString    CCRS_PropertySetInfo::m_aPropertyNameForCount( OUString::createFromAscii( "RowCount" ) );
OUString    CCRS_PropertySetInfo::m_aPropertyNameForFinalCount( OUString::createFromAscii( "IsRowCountFinal" ) );
OUString    CCRS_PropertySetInfo::m_aPropertyNameForFetchSize( OUString::createFromAscii( "FetchSize" ) );
OUString    CCRS_PropertySetInfo::m_aPropertyNameForFetchDirection( OUString::createFromAscii( "FetchDirection" ) );

CCRS_PropertySetInfo::CCRS_PropertySetInfo(
        Reference< XPropertySetInfo > xInfo )
        : m_pProperties( NULL )
        , m_nFetchSizePropertyHandle( -1 )
        , m_nFetchDirectionPropertyHandle( -1 )
{
    //initialize list of properties:

    // it is required, that the received xInfo contains the two
    // properties with names 'm_aPropertyNameForCount' and
    // 'm_aPropertyNameForFinalCount'

    if( xInfo.is() )
    {
        Sequence<Property> aProps = xInfo->getProperties();
        m_pProperties = new Sequence<Property> ( aProps );
    }
    else
    {
        OSL_ENSURE( sal_False, "The received XPropertySetInfo doesn't contain required properties" );
        m_pProperties = new Sequence<Property>;
    }

    //ensure, that we haven't got the Properties 'FetchSize' and 'Direction' twice:
    sal_Int32 nFetchSize = impl_getPos( m_aPropertyNameForFetchSize );
    sal_Int32 nFetchDirection = impl_getPos( m_aPropertyNameForFetchDirection );
    sal_Int32 nDeleted = 0;
    if( nFetchSize != -1 )
        nDeleted++;
    if( nFetchDirection != -1 )
        nDeleted++;

    Sequence< Property >* pOrigProps = new Sequence<Property> ( *m_pProperties );
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

        if( nFetchSize != -1 )
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

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------
//list all interfaces inclusive baseclasses of interfaces
XINTERFACE_IMPL_2( CCRS_PropertySetInfo
                  , XTypeProvider
                  , XPropertySetInfo
                  );

//--------------------------------------------------------------------------
// XTypeProvider methods.
//--------------------------------------------------------------------------
//list all interfaces exclusive baseclasses
XTYPEPROVIDER_IMPL_2( CCRS_PropertySetInfo
                    , XTypeProvider
                    , XPropertySetInfo
                    );
//--------------------------------------------------------------------------
// XPropertySetInfo methods.
//--------------------------------------------------------------------------
//virtual
Sequence< Property > SAL_CALL CCRS_PropertySetInfo
    ::getProperties() throw( RuntimeException )
{
    return *m_pProperties;
}

//virtual
Property SAL_CALL CCRS_PropertySetInfo
    ::getPropertyByName( const rtl::OUString& aName )
        throw( UnknownPropertyException, RuntimeException )
{
    if ( !aName.getLength() )
        throw UnknownPropertyException();

    Property aProp;
    if ( impl_queryProperty( aName, aProp ) )
        return aProp;

    throw UnknownPropertyException();
}

//virtual
sal_Bool SAL_CALL CCRS_PropertySetInfo
    ::hasPropertyByName( const rtl::OUString& Name )
        throw( RuntimeException )
{
    return ( impl_getPos( Name ) != -1 );
}

//--------------------------------------------------------------------------
// impl_ methods.
//--------------------------------------------------------------------------

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

//static
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
        OSL_ENSURE( sal_False, "Properties not initialized yet" );
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

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// class CachedContentResultSet
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

CachedContentResultSet::CachedContentResultSet( Reference< XResultSet > xOrigin
                , Reference< XContentIdentifierMapping >
                    xContentIdentifierMapping )
                : ContentResultSetWrapper( xOrigin )

                , m_xFetchProvider( NULL )
                , m_xFetchProviderForContentAccess( NULL )
                , m_xContentIdentifierMapping( xContentIdentifierMapping )

                , m_pMyPropSetInfo( NULL )
                , m_xMyPropertySetInfo( NULL )

                , m_nRow( 0 ) // Position is one-based. Zero means: before first element.
                , m_nLastAppliedPos( 0 )
                , m_bAfterLast( sal_False )
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
    //do not delete m_pMyPropSetInfo, cause it is hold via reference
};

//--------------------------------------------------------------------------
// impl_ methods.
//--------------------------------------------------------------------------

sal_Bool SAL_CALL CachedContentResultSet
    ::applyPositionToOrigin( sal_Int32 nRow )
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    //-------------------------------------------------------------------------
    /**
    @returns
        <TRUE/> if the cursor is on a valid row; <FALSE/> if it is off
        the result set.
    */

    ReacquireableGuard aGuard( m_aMutex );
    OSL_ENSURE( nRow >= 0, "only positive values supported" );
    if( !m_xResultSetOrigin.is() )
    {
        OSL_ENSURE( sal_False, "broadcaster was disposed already" );
        return sal_False;
    }
//  OSL_ENSURE( nRow <= m_nKnownCount, "don't step into regions you don't know with this method" );

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
            for( sal_Int32 nM = 0; nN--; nM++ )
            {
                if( !m_xResultSetOrigin->next() )
                    break;
            }

            aGuard.reacquire();
            m_nLastAppliedPos += nM;
            m_bAfterLastApplied = nRow != m_nLastAppliedPos;
            return nRow == m_nLastAppliedPos;
        }

        if( !nRow ) //absolute( 0 ) will throw exception
        {
            m_xResultSetOrigin->beforeFirst();

            aGuard.reacquire();
            m_nLastAppliedPos = 0;
            m_bAfterLastApplied = sal_False;
            return sal_False;
        }
        try
        {
            //move absolute, if !nLastAppliedPos
            //because move relative would throw exception
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
        catch( SQLException& rEx )
        {
            if( !bAfterLastApplied && !bAfterLast && nRow > nLastAppliedPos && impl_isForwardOnly() )
            {
                sal_Int32 nN = nRow - nLastAppliedPos;
                for( sal_Int32 nM = 0; nN--; nM++ )
                {
                    if( !m_xResultSetOrigin->next() )
                        break;
                }

                aGuard.reacquire();
                m_nLastAppliedPos += nM;
                m_bAfterLastApplied = nRow != m_nLastAppliedPos;
                return nRow == m_nLastAppliedPos;
            }
            else
                throw rEx;
        }
    }
    else
        return sal_True;
};

sal_Bool SAL_CALL CachedContentResultSet
    ::applyPositionToOrigin()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    sal_Int32 nRow;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_bAfterLast )
            throw SQLException();
        nRow = m_nRow;
        if( !nRow )
            throw SQLException();
    }
    return applyPositionToOrigin( nRow );
};

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//define for fetching data
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

#define FETCH_XXX( aCache, fetchInterface, fetchMethod )            \
sal_Bool bDirection = !!(                                           \
    nFetchDirection != FetchDirection::REVERSE );                   \
FetchResult aResult =                                               \
    fetchInterface->fetchMethod( nRow, nFetchSize, bDirection );    \
osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );               \
aCache.loadData( aResult );                                         \
sal_Int32 nMax = aCache.getMaxRow();                                \
sal_Int32 nCurCount = m_nKnownCount;                                \
sal_Bool bIsFinalCount = aCache.hasKnownLast();                     \
sal_Bool bCurIsFinalCount = m_bFinalCount;                          \
aGuard.clear();                                                     \
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

    //create PropertyChangeEvent and set value
    PropertyChangeEvent aEvt;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aEvt.Source =  static_cast< XPropertySet * >( this );
        aEvt.Further = sal_False;
        aEvt.OldValue <<= nOld;
        aEvt.NewValue <<= nNew;

        m_nKnownCount = nNew;
    }

    //send PropertyChangeEvent to listeners
    impl_notifyPropertyChangeListeners( aEvt );
}

void SAL_CALL CachedContentResultSet
    ::impl_changeIsRowCountFinal( sal_Bool bOld, sal_Bool bNew )
{
    OSL_ENSURE( !bOld && bNew, "This change is not allowed for IsRowCountFinal" );
    if( ! (!bOld && bNew ) )
        return;

    //create PropertyChangeEvent and set value
    PropertyChangeEvent aEvt;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        aEvt.Source =  static_cast< XPropertySet * >( this );
        aEvt.Further = sal_False;
        aEvt.OldValue <<= bOld;
        aEvt.NewValue <<= bNew;

        m_bFinalCount = bNew;
    }

    //send PropertyChangeEvent to listeners
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


//virtual
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

//--------------------------------------------------------------------------
// XInterface methods. ( inherited )
//--------------------------------------------------------------------------
XINTERFACE_COMMON_IMPL( CachedContentResultSet )

Any SAL_CALL CachedContentResultSet
    ::queryInterface( const Type&  rType )
    throw ( RuntimeException )
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

//--------------------------------------------------------------------------
// XTypeProvider methods.
//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
// XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_NOFACTORY_IMPL_1( CachedContentResultSet,
                    OUString::createFromAscii( "CachedContentResultSet" ),
                    OUString::createFromAscii( CACHED_CONTENT_RESULTSET_SERVICE_NAME ) );

//--------------------------------------------------------------------------
// XPropertySet methods. ( inherited )
//--------------------------------------------------------------------------

// virtual
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
        OSL_ENSURE( sal_False, "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }

    Property aProp = m_pMyPropSetInfo->getPropertyByName( aPropertyName );
        //throws UnknownPropertyException, if so

    if( aProp.Attributes & PropertyAttribute::READONLY )
    {
        //It is assumed, that the properties
        //'RowCount' and 'IsRowCountFinal' are readonly!
        throw IllegalArgumentException();
    }
    if( aProp.Name == CCRS_PropertySetInfo
                        ::m_aPropertyNameForFetchDirection )
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
            aEvt.Further = sal_False;
            aEvt.PropertyHandle = m_pMyPropSetInfo->
                                    m_nFetchDirectionPropertyHandle;
            aEvt.OldValue <<= m_nFetchDirection;
            aEvt.NewValue <<= nNew;

            m_nFetchDirection = nNew;
        }

        //send PropertyChangeEvent to listeners
        impl_notifyPropertyChangeListeners( aEvt );
    }
    else if( aProp.Name == CCRS_PropertySetInfo
                        ::m_aPropertyNameForFetchSize )
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
            aEvt.Further = sal_False;
            aEvt.PropertyHandle = m_pMyPropSetInfo->
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
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( !m_xPropertySetOrigin.is() )
        {
            OSL_ENSURE( sal_False, "broadcaster was disposed already" );
            return;
        }
        m_xPropertySetOrigin->setPropertyValue( aPropertyName, aValue );
    }
}

//--------------------------------------------------------------------------
// virtual
Any SAL_CALL CachedContentResultSet
    ::getPropertyValue( const OUString& rPropertyName )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !getPropertySetInfo().is() )
    {
        OSL_ENSURE( sal_False, "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }

    Property aProp = m_pMyPropSetInfo->getPropertyByName( rPropertyName );
        //throws UnknownPropertyException, if so

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
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            if( !m_xPropertySetOrigin.is() )
            {
                OSL_ENSURE( sal_False, "broadcaster was disposed already" );
                throw UnknownPropertyException();
            }
        }
        aValue = m_xPropertySetOrigin->getPropertyValue( rPropertyName );
    }
    return aValue;
}

//--------------------------------------------------------------------------
// own methods.  ( inherited )
//--------------------------------------------------------------------------

//virtual
void SAL_CALL CachedContentResultSet
    ::impl_disposing( const EventObject& rEventObject )
    throw( RuntimeException )
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
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = sal_False;
    //---------

    if( CCRS_PropertySetInfo
            ::impl_isMyPropertyName( rEvt.PropertyName ) )
    {
        //don't notify foreign events on fetchsize and fetchdirection
        if( aEvt.PropertyName == CCRS_PropertySetInfo
                                ::m_aPropertyNameForFetchSize
        || aEvt.PropertyName == CCRS_PropertySetInfo
                                ::m_aPropertyNameForFetchDirection )
            return;

        //adjust my props 'RowCount' and 'IsRowCountFinal'
        if( aEvt.PropertyName == CCRS_PropertySetInfo
                            ::m_aPropertyNameForCount )
        {//RowCount changed

            //check value
            sal_Int32 nNew;
            if( !( aEvt.NewValue >>= nNew ) )
            {
                OSL_ENSURE( sal_False, "PropertyChangeEvent contains wrong data" );
                return;
            }

            impl_changeRowCount( m_nKnownCount, nNew );
        }
        else if( aEvt.PropertyName == CCRS_PropertySetInfo
                                ::m_aPropertyNameForFinalCount )
        {//IsRowCountFinal changed

            //check value
            sal_Bool bNew;
            if( !( aEvt.NewValue >>= bNew ) )
            {
                OSL_ENSURE( sal_False, "PropertyChangeEvent contains wrong data" );
                return;
            }
            impl_changeIsRowCountFinal( m_bFinalCount, bNew );
        }
        return;
    }

    //-----------
    impl_notifyPropertyChangeListeners( aEvt );
}


//virtual
void SAL_CALL CachedContentResultSet
    ::impl_vetoableChange( const PropertyChangeEvent& rEvt )
    throw( PropertyVetoException,
           RuntimeException )
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
    aEvt.Further = sal_False;

    impl_notifyVetoableChangeListeners( aEvt );
}

//--------------------------------------------------------------------------
// XContentAccess methods. ( inherited ) ( -- position dependent )
//--------------------------------------------------------------------------

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
            OSL_ENSURE( sal_False, "broadcaster was disposed already" );\
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

//--------------------------------------------------------------------------
// virtual
OUString SAL_CALL CachedContentResultSet
    ::queryContentIdentfierString()
    throw( RuntimeException )
{
    XCONTENTACCESS_queryXXX( queryContentIdentfierString, ContentIdentifierString, OUString )
}

//--------------------------------------------------------------------------
// virtual
Reference< XContentIdentifier > SAL_CALL CachedContentResultSet
    ::queryContentIdentifier()
    throw( RuntimeException )
{
    XCONTENTACCESS_queryXXX( queryContentIdentifier, ContentIdentifier, Reference< XContentIdentifier > )
}

//--------------------------------------------------------------------------
// virtual
Reference< XContent > SAL_CALL CachedContentResultSet
    ::queryContent()
    throw( RuntimeException )
{
    XCONTENTACCESS_queryXXX( queryContent, Content, Reference< XContent > )
}

//-----------------------------------------------------------------
// XResultSet methods. ( inherited )
//-----------------------------------------------------------------
//virtual

sal_Bool SAL_CALL CachedContentResultSet
    ::next()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    ReacquireableGuard aGuard( m_aMutex );
    //after last
    if( m_bAfterLast )
        return sal_False;
    //last
    aGuard.clear();
    if( isLast() )
    {
        aGuard.reacquire();
        m_nRow++;
        m_bAfterLast = sal_True;
        return sal_False;
    }
    aGuard.reacquire();
    //known valid position
    if( impl_isKnownValidPosition( m_nRow + 1 ) )
    {
        m_nRow++;
        return sal_True;
    }

    //unknown position
    sal_Int32 nRow = m_nRow;
    aGuard.clear();

    sal_Bool bValid = applyPositionToOrigin( nRow + 1 );

    aGuard.reacquire();
    m_nRow = nRow + 1;
    m_bAfterLast = !bValid;
    return bValid;
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::previous()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( impl_isForwardOnly() )
        throw SQLException();

    ReacquireableGuard aGuard( m_aMutex );
    //before first ?:
    if( !m_bAfterLast && !m_nRow )
        return sal_False;
    //first ?:
    if( !m_bAfterLast && m_nKnownCount && m_nRow == 1 )
    {
        m_nRow--;
        m_bAfterLast = sal_False;
        return sal_False;
    }
    //known valid position ?:
    if( impl_isKnownValidPosition( m_nRow - 1 ) )
    {
        m_nRow--;
        m_bAfterLast = sal_False;
        return sal_True;
    }
    //unknown position:
    sal_Int32 nRow = m_nRow;
    aGuard.clear();

    sal_Bool bValid = applyPositionToOrigin( nRow - 1  );

    aGuard.reacquire();
    m_nRow = nRow - 1;
    m_bAfterLast = sal_False;
    return bValid;
}

//virtual
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
        OSL_ENSURE( sal_False, "broadcaster was disposed already" );
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
        //unknown final count:
        aGuard.clear();

        sal_Bool bValid = m_xResultSetOrigin->absolute( row );

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
        return nCurRow;
    }
    //row > 0:
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
    //unknown new position:
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

//virtual
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
        //known invalid new position:
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
        //unknown new position:
        aGuard.clear();
        sal_Bool bValid = applyPositionToOrigin( nNewRow );

        aGuard.reacquire();
        m_nRow = nNewRow;
        m_bAfterLast = !bValid && nNewRow > 0;
        return bValid;
    }
}


//virtual
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
    //unknown position
    aGuard.clear();

    sal_Bool bValid = applyPositionToOrigin( 1 );

    aGuard.reacquire();
    m_nRow = 1;
    m_bAfterLast = sal_False;
    return bValid;
}

//virtual
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
        return m_nKnownCount;
    }
    //unknown position
    if( !m_xResultSetOrigin.is() )
    {
        OSL_ENSURE( sal_False, "broadcaster was disposed already" );
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
    return nCurRow;
}

//virtual
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

//virtual
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

//virtual
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
        OSL_ENSURE( sal_False, "broadcaster was disposed already" );
        return sal_False;
    }
    aGuard.clear();

    //find out whethter the original resultset contains rows or not
    m_xResultSetOrigin->afterLast();

    aGuard.reacquire();
    m_bAfterLastApplied = sal_True;
    aGuard.clear();

    return m_xResultSetOrigin->isAfterLast();
}

//virtual
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
        OSL_ENSURE( sal_False, "broadcaster was disposed already" );
        return sal_False;
    }
    aGuard.clear();

    //find out whethter the original resultset contains rows or not
    m_xResultSetOrigin->beforeFirst();

    aGuard.reacquire();
    m_bAfterLastApplied = sal_False;
    m_nLastAppliedPos = 0;
    aGuard.clear();

    return m_xResultSetOrigin->isBeforeFirst();
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::isFirst()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    sal_Int32 nRow;
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

    //need to ask origin
    {
        if( applyPositionToOrigin( nRow ) )
            return xResultSetOrigin->isFirst();
        else
            return sal_False;
    }
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::isLast()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    sal_Int32 nRow;
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

    //need to ask origin
    {
        if( applyPositionToOrigin( nRow ) )
            return xResultSetOrigin->isLast();
        else
            return sal_False;
    }
}


//virtual
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

//virtual
void SAL_CALL CachedContentResultSet
    ::refreshRow()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    //the ContentResultSet is static and will not change
    //therefore we don't need to reload anything
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::rowUpdated()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    //the ContentResultSet is static and will not change
    return sal_False;
}
//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::rowInserted()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    //the ContentResultSet is static and will not change
    return sal_False;
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::rowDeleted()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    //the ContentResultSet is static and will not change
    return sal_False;
}

//virtual
Reference< XInterface > SAL_CALL CachedContentResultSet
    ::getStatement()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    //@todo ?return anything
    return Reference< XInterface >();
}

//-----------------------------------------------------------------
// XRow methods. ( inherited )
//-----------------------------------------------------------------

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::wasNull()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_bLastReadWasFromCache )
            return m_bLastCachedReadWasNull;
        if( !m_xRowOrigin.is() )
        {
            OSL_ENSURE( sal_False, "broadcaster was disposed already" );
            return sal_False;
        }
    }
    return m_xRowOrigin->wasNull();
}

//virtual
rtl::OUString SAL_CALL CachedContentResultSet
    ::getString( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getString, OUString );
}

//virtual
sal_Bool SAL_CALL CachedContentResultSet
    ::getBoolean( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBoolean, sal_Bool );
}

//virtual
sal_Int8 SAL_CALL CachedContentResultSet
    ::getByte( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getByte, sal_Int8 );
}

//virtual
sal_Int16 SAL_CALL CachedContentResultSet
    ::getShort( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getShort, sal_Int16 );
}

//virtual
sal_Int32 SAL_CALL CachedContentResultSet
    ::getInt( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getInt, sal_Int32 );
}

//virtual
sal_Int64 SAL_CALL CachedContentResultSet
    ::getLong( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getLong, sal_Int64 );
}

//virtual
float SAL_CALL CachedContentResultSet
    ::getFloat( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getFloat, float );
}

//virtual
double SAL_CALL CachedContentResultSet
    ::getDouble( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getDouble, double );
}

//virtual
Sequence< sal_Int8 > SAL_CALL CachedContentResultSet
    ::getBytes( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBytes, Sequence< sal_Int8 > );
}

//virtual
Date SAL_CALL CachedContentResultSet
    ::getDate( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getDate, Date );
}

//virtual
Time SAL_CALL CachedContentResultSet
    ::getTime( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getTime, Time );
}

//virtual
DateTime SAL_CALL CachedContentResultSet
    ::getTimestamp( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getTimestamp, DateTime );
}

//virtual
Reference< com::sun::star::io::XInputStream >
    SAL_CALL CachedContentResultSet
    ::getBinaryStream( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBinaryStream, Reference< com::sun::star::io::XInputStream > );
}

//virtual
Reference< com::sun::star::io::XInputStream >
    SAL_CALL CachedContentResultSet
    ::getCharacterStream( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getCharacterStream, Reference< com::sun::star::io::XInputStream > );
}

//virtual
Any SAL_CALL CachedContentResultSet
    ::getObject( sal_Int32 columnIndex,
           const Reference<
            com::sun::star::container::XNameAccess >& typeMap )
    throw( SQLException,
           RuntimeException )
{
    //if you change this macro please pay attention to
    //define XROW_GETXXX, where this is similar implemented

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
                OSL_ENSURE( sal_False, "broadcaster was disposed already" );
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
            return m_xRowOrigin->getObject( columnIndex, typeMap );
        }
    }
    //@todo: pay attention to typeMap
    const Any& rValue = m_aCache.getAny( nRow, columnIndex );
    Any aRet;
    m_bLastReadWasFromCache = sal_True;
    m_bLastCachedReadWasNull = !( rValue >>= aRet );
    return aRet;
}

//virtual
Reference< XRef > SAL_CALL CachedContentResultSet
    ::getRef( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getRef, Reference< XRef > );
}

//virtual
Reference< XBlob > SAL_CALL CachedContentResultSet
    ::getBlob( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBlob, Reference< XBlob > );
}

//virtual
Reference< XClob > SAL_CALL CachedContentResultSet
    ::getClob( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getClob, Reference< XClob > );
}

//virtual
Reference< XArray > SAL_CALL CachedContentResultSet
    ::getArray( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getArray, Reference< XArray > );
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// class CachedContentResultSetFactory
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

CachedContentResultSetFactory::CachedContentResultSetFactory(
        const Reference< XMultiServiceFactory > & rSMgr )
{
    m_xSMgr = rSMgr;
}

CachedContentResultSetFactory::~CachedContentResultSetFactory()
{
}

//--------------------------------------------------------------------------
// CachedContentResultSetFactory XInterface methods.
//--------------------------------------------------------------------------

XINTERFACE_IMPL_3( CachedContentResultSetFactory,
                   XTypeProvider,
                   XServiceInfo,
                   XCachedContentResultSetFactory );

//--------------------------------------------------------------------------
// CachedContentResultSetFactory XTypeProvider methods.
//--------------------------------------------------------------------------

XTYPEPROVIDER_IMPL_3( CachedContentResultSetFactory,
                      XTypeProvider,
                         XServiceInfo,
                      XCachedContentResultSetFactory );

//--------------------------------------------------------------------------
// CachedContentResultSetFactory XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_IMPL_1( CachedContentResultSetFactory,
                OUString::createFromAscii( "CachedContentResultSetFactory" ),
                OUString::createFromAscii( CACHED_CONTENT_RESULTSET_FACTORY_NAME ) );

//--------------------------------------------------------------------------
// Service factory implementation.
//--------------------------------------------------------------------------

ONE_INSTANCE_SERVICE_FACTORY_IMPL( CachedContentResultSetFactory );

//--------------------------------------------------------------------------
// CachedContentResultSetFactory XCachedContentResultSetFactory methods.
//--------------------------------------------------------------------------

    //virtual
Reference< XResultSet > SAL_CALL CachedContentResultSetFactory
    ::createCachedContentResultSet(
            const Reference< XResultSet > & xSource,
            const Reference< XContentIdentifierMapping > & xMapping )
            throw( com::sun::star::uno::RuntimeException )
{
    Reference< XResultSet > xRet;
    xRet = new CachedContentResultSet( xSource, xMapping );
    return xRet;
}

