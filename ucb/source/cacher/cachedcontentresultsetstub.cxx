/*************************************************************************
 *
 *  $RCSfile: cachedcontentresultsetstub.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: iha $ $Date: 2001-03-23 11:00:45 $
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

#include <cachedcontentresultsetstub.hxx>

#ifndef _COM_SUN_STAR_SDBC_FETCHDIRECTION_HPP_
#include <com/sun/star/sdbc/FetchDirection.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_FETCHERROR_HPP_
#include <com/sun/star/ucb/FetchError.hpp>
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

CachedContentResultSetStub::CachedContentResultSetStub( Reference< XResultSet > xOrigin )
                : ContentResultSetWrapper( xOrigin )
                , m_nColumnCount( 0 )
                , m_bColumnCountCached( sal_False )
                , m_bNeedToPropagateFetchSize( sal_True )
                , m_bFirstFetchSizePropagationDone( sal_False )
                , m_nLastFetchSize( 1 )//this value is not important at all
                , m_bLastFetchDirection( sal_True )//this value is not important at all
                , m_aPropertyNameForFetchSize( OUString::createFromAscii( "FetchSize" ) )
                , m_aPropertyNameForFetchDirection( OUString::createFromAscii( "FetchDirection" ) )
{
    impl_init();
}

CachedContentResultSetStub::~CachedContentResultSetStub()
{
    impl_deinit();
}

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------
XINTERFACE_COMMON_IMPL( CachedContentResultSetStub )

Any SAL_CALL CachedContentResultSetStub
    ::queryInterface( const Type&  rType )
    throw ( RuntimeException )
{
    //list all interfaces inclusive baseclasses of interfaces

    Any aRet = ContentResultSetWrapper::queryInterface( rType );
    if( aRet.hasValue() )
        return aRet;

    aRet = cppu::queryInterface( rType
                , static_cast< XTypeProvider* >( this )
                , static_cast< XServiceInfo* >( this )
                , static_cast< XFetchProvider* >( this )
                , static_cast< XFetchProviderForContentAccess* >( this )
                );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//--------------------------------------------------------------------------
// own methods.  ( inherited )
//--------------------------------------------------------------------------

//virtual
void SAL_CALL CachedContentResultSetStub
    ::impl_propertyChange( const PropertyChangeEvent& rEvt )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    //don't notify events on fetchsize and fetchdirection to the above CachedContentResultSet
    //because it will ignore them anyway and we can save this remote calls
    if(    rEvt.PropertyName == m_aPropertyNameForFetchSize
        || rEvt.PropertyName == m_aPropertyNameForFetchDirection )
        return;

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = sal_False;

    impl_notifyPropertyChangeListeners( aEvt );
}


//virtual
void SAL_CALL CachedContentResultSetStub
    ::impl_vetoableChange( const PropertyChangeEvent& rEvt )
    throw( PropertyVetoException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    //don't notify events on fetchsize and fetchdirection to the above CachedContentResultSet
    //because it will ignore them anyway and we can save this remote calls
    if(    rEvt.PropertyName == m_aPropertyNameForFetchSize
        || rEvt.PropertyName == m_aPropertyNameForFetchDirection )
        return;

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = sal_False;

    impl_notifyVetoableChangeListeners( aEvt );
}

//--------------------------------------------------------------------------
// XTypeProvider methods.
//--------------------------------------------------------------------------

XTYPEPROVIDER_COMMON_IMPL( CachedContentResultSetStub )
//list all interfaces exclusive baseclasses
Sequence< Type > SAL_CALL CachedContentResultSetStub
    ::getTypes()
    throw( RuntimeException )
{
    static Sequence< Type >* pTypes = NULL;
    if( !pTypes )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pTypes )
        {
            pTypes->realloc( 13 );
            (*pTypes)[0] = CPPU_TYPE_REF( XTypeProvider );
            (*pTypes)[1] = CPPU_TYPE_REF( XServiceInfo );
            (*pTypes)[2] = CPPU_TYPE_REF( XComponent );
            (*pTypes)[3] = CPPU_TYPE_REF( XCloseable );
            (*pTypes)[4] = CPPU_TYPE_REF( XResultSetMetaDataSupplier );
            (*pTypes)[5] = CPPU_TYPE_REF( XPropertySet );
            (*pTypes)[6] = CPPU_TYPE_REF( XPropertyChangeListener );
            (*pTypes)[7] = CPPU_TYPE_REF( XVetoableChangeListener );
            (*pTypes)[8] = CPPU_TYPE_REF( XResultSet );
            (*pTypes)[9] = CPPU_TYPE_REF( XContentAccess );
            (*pTypes)[10] = CPPU_TYPE_REF( XRow );
            (*pTypes)[11] = CPPU_TYPE_REF( XFetchProvider );
            (*pTypes)[12] = CPPU_TYPE_REF( XFetchProviderForContentAccess );
        }
    }
    return *pTypes;
}

//--------------------------------------------------------------------------
// XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_NOFACTORY_IMPL_1( CachedContentResultSetStub,
                    OUString::createFromAscii( "CachedContentResultSetStub" ),
                    OUString::createFromAscii( CACHED_CRS_STUB_SERVICE_NAME ) );

//-----------------------------------------------------------------
// XFetchProvider methods.
//-----------------------------------------------------------------

#define FETCH_XXX( impl_loadRow, loadInterface ) \
impl_EnsureNotDisposed(); \
if( !m_xResultSetOrigin.is() ) \
{ \
    OSL_ENSURE( sal_False, "broadcaster was disposed already" ); \
    throw RuntimeException(); \
} \
impl_propagateFetchSizeAndDirection( nRowCount, bDirection ); \
FetchResult aRet; \
aRet.StartIndex = nRowStartPosition; \
aRet.Orientation = bDirection; \
aRet.FetchError = FetchError::SUCCESS; /*ENDOFDATA, EXCEPTION*/ \
sal_Int32 nOldOriginal_Pos = m_xResultSetOrigin->getRow(); \
if( impl_isForwardOnly() ) \
{ \
    if( nOldOriginal_Pos != nRowStartPosition ) \
    { \
        /*@todo*/ \
        aRet.FetchError = FetchError::EXCEPTION; \
        return aRet; \
    } \
    if( nRowCount != 1 ) \
        aRet.FetchError = FetchError::EXCEPTION; \
 \
    aRet.Rows.realloc( 1 ); \
 \
    try \
    { \
        impl_loadRow( aRet.Rows[0], loadInterface ); \
    } \
    catch( SQLException& ) \
    { \
        aRet.Rows.realloc( 0 ); \
        aRet.FetchError = FetchError::EXCEPTION; \
        return aRet; \
    } \
    return aRet; \
} \
aRet.Rows.realloc( nRowCount ); \
sal_Bool bOldOriginal_AfterLast = sal_False; \
if( !nOldOriginal_Pos ) \
    bOldOriginal_AfterLast = m_xResultSetOrigin->isAfterLast(); \
sal_Int32 nN = 1; \
sal_Bool bValidNewPos = sal_False; \
try \
{ \
    try \
    { \
        /*if( nOldOriginal_Pos != nRowStartPosition )*/ \
        bValidNewPos = m_xResultSetOrigin->absolute( nRowStartPosition ); \
    } \
    catch( SQLException& ) \
    { \
        aRet.Rows.realloc( 0 ); \
        aRet.FetchError = FetchError::EXCEPTION; \
        return aRet; \
    } \
    if( !bValidNewPos ) \
    { \
        aRet.Rows.realloc( 0 ); \
        aRet.FetchError = FetchError::EXCEPTION; \
 \
        /*restore old position*/ \
        if( nOldOriginal_Pos ) \
            m_xResultSetOrigin->absolute( nOldOriginal_Pos ); \
        else if( bOldOriginal_AfterLast ) \
            m_xResultSetOrigin->afterLast(); \
        else \
            m_xResultSetOrigin->beforeFirst(); \
 \
        return aRet; \
    } \
    for( ; nN <= nRowCount; ) \
    { \
        impl_loadRow( aRet.Rows[nN-1], loadInterface ); \
        nN++; \
        if( nN <= nRowCount ) \
        { \
            if( bDirection ) \
            { \
                if( !m_xResultSetOrigin->next() ) \
                { \
                    aRet.Rows.realloc( nN-1 ); \
                    aRet.FetchError = FetchError::ENDOFDATA; \
                    break; \
                } \
            } \
            else \
            { \
                if( !m_xResultSetOrigin->previous() ) \
                { \
                    aRet.Rows.realloc( nN-1 ); \
                    aRet.FetchError = FetchError::ENDOFDATA; \
                    break; \
                } \
            } \
        } \
    } \
} \
catch( SQLException& ) \
{ \
    aRet.Rows.realloc( nN-1 ); \
    aRet.FetchError = FetchError::EXCEPTION; \
} \
/*restore old position*/ \
if( nOldOriginal_Pos ) \
    m_xResultSetOrigin->absolute( nOldOriginal_Pos ); \
else if( bOldOriginal_AfterLast ) \
    m_xResultSetOrigin->afterLast(); \
else \
    m_xResultSetOrigin->beforeFirst(); \
return aRet;

FetchResult SAL_CALL CachedContentResultSetStub
    ::fetch( sal_Int32 nRowStartPosition
    , sal_Int32 nRowCount, sal_Bool bDirection )
    throw( RuntimeException )
{
    impl_init_xRowOrigin();
    FETCH_XXX( impl_getCurrentRowContent, m_xRowOrigin );
}

sal_Int32 SAL_CALL CachedContentResultSetStub
    ::impl_getColumnCount()
{
    sal_Int32 nCount;
    sal_Bool bCached;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        nCount = m_nColumnCount;
        bCached = m_bColumnCountCached;
    }
    if( !bCached )
    {
        try
        {
            Reference< XResultSetMetaData > xMetaData = getMetaData();
            if( xMetaData.is() )
                nCount = xMetaData->getColumnCount();
        }
        catch( SQLException& )
        {
            OSL_ENSURE( sal_False, "couldn't determine the column count" );
            nCount = 0;
        }
    }
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_nColumnCount = nCount;
    m_bColumnCountCached = sal_True;
    return m_nColumnCount;
}

void SAL_CALL CachedContentResultSetStub
    ::impl_getCurrentRowContent( Any& rRowContent
        , Reference< XRow > xRow )
        throw ( SQLException, RuntimeException )
{
    sal_Int32 nCount = impl_getColumnCount();

    Sequence< Any > aContent( nCount );
    for( sal_Int32 nN = 1; nN <= nCount; nN++ )
    {
        aContent[nN-1] = xRow->getObject( nN, NULL );
    }

    rRowContent <<= aContent;
}

void SAL_CALL CachedContentResultSetStub
    ::impl_propagateFetchSizeAndDirection( sal_Int32 nFetchSize, sal_Bool bFetchDirection )
        throw ( RuntimeException )
{
    //this is done only for the case, that there is another CachedContentResultSet in the chain of underlying ResulSets

    //we do not propagate the property 'FetchSize' or 'FetchDirection' via 'setPropertyValue' from the above CachedContentResultSet to save remote calls

    //if the underlying ResultSet has a property FetchSize and FetchDirection,
    //we will set these properties, if the new given parameters are different from the last ones

    if( !m_bNeedToPropagateFetchSize )
        return;

    sal_Bool bNeedAction;
    sal_Int32 nLastSize;
    sal_Bool bLastDirection;
    sal_Bool bFirstPropagationDone;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        bNeedAction             = m_bNeedToPropagateFetchSize;
        nLastSize               = m_nLastFetchSize;
        bLastDirection          = m_bLastFetchDirection;
        bFirstPropagationDone   = m_bFirstFetchSizePropagationDone;
    }
    if( bNeedAction )
    {
        if( nLastSize == nFetchSize
            && bLastDirection == bFetchDirection
            && bFirstPropagationDone == sal_True )
            return;

        if(!bFirstPropagationDone)
        {
            //check wether the properties 'FetchSize' and 'FetchDirection' do exist

            Reference< XPropertySetInfo > xPropertySetInfo = getPropertySetInfo();
            sal_Bool bHasSize = xPropertySetInfo->hasPropertyByName( m_aPropertyNameForFetchSize );
            sal_Bool bHasDirection = xPropertySetInfo->hasPropertyByName( m_aPropertyNameForFetchDirection );

            if(!bHasSize || !bHasDirection)
            {
                osl::Guard< osl::Mutex > aGuard( m_aMutex );
                m_bNeedToPropagateFetchSize = sal_False;
                return;
            }
        }

        sal_Bool bSetSize       = ( nLastSize       !=nFetchSize        ) || !bFirstPropagationDone;
        sal_Bool bSetDirection  = ( bLastDirection  !=bFetchDirection   ) || !bFirstPropagationDone;

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_bFirstFetchSizePropagationDone = sal_True;
            m_nLastFetchSize        = nFetchSize;
            m_bLastFetchDirection   = bFetchDirection;
        }

        if( bSetSize )
        {
            Any aValue;
            aValue <<= nFetchSize;
            try
            {
                setPropertyValue( m_aPropertyNameForFetchSize, aValue );
            }
            catch( com::sun::star::uno::Exception& ) {}
        }
        if( bSetDirection )
        {
            sal_Int32 nFetchDirection = FetchDirection::FORWARD;
            if( !bFetchDirection )
                nFetchDirection = FetchDirection::REVERSE;
            Any aValue;
            aValue <<= nFetchDirection;
            try
            {
                setPropertyValue( m_aPropertyNameForFetchDirection, aValue );
            }
            catch( com::sun::star::uno::Exception& ) {}
        }

    }
}

//-----------------------------------------------------------------
// XFetchProviderForContentAccess methods.
//-----------------------------------------------------------------

void SAL_CALL CachedContentResultSetStub
    ::impl_getCurrentContentIdentifierString( Any& rAny
        , Reference< XContentAccess > xContentAccess )
        throw ( RuntimeException )
{
     rAny <<= xContentAccess->queryContentIdentifierString();
}

void SAL_CALL CachedContentResultSetStub
    ::impl_getCurrentContentIdentifier( Any& rAny
        , Reference< XContentAccess > xContentAccess )
        throw ( RuntimeException )
{
     rAny <<= xContentAccess->queryContentIdentifier();
}

void SAL_CALL CachedContentResultSetStub
    ::impl_getCurrentContent( Any& rAny
        , Reference< XContentAccess > xContentAccess )
        throw ( RuntimeException )
{
     rAny <<= xContentAccess->queryContent();
}

//virtual
FetchResult SAL_CALL CachedContentResultSetStub
    ::fetchContentIdentifierStrings( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException )
{
    impl_init_xContentAccessOrigin();
    FETCH_XXX( impl_getCurrentContentIdentifierString, m_xContentAccessOrigin );
}

//virtual
FetchResult SAL_CALL CachedContentResultSetStub
    ::fetchContentIdentifiers( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException )
{
    impl_init_xContentAccessOrigin();
    FETCH_XXX( impl_getCurrentContentIdentifier, m_xContentAccessOrigin );
}

//virtual
FetchResult SAL_CALL CachedContentResultSetStub
    ::fetchContents( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException )
{
    impl_init_xContentAccessOrigin();
    FETCH_XXX( impl_getCurrentContent, m_xContentAccessOrigin );
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// class CachedContentResultSetStubFactory
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

CachedContentResultSetStubFactory::CachedContentResultSetStubFactory(
        const Reference< XMultiServiceFactory > & rSMgr )
{
    m_xSMgr = rSMgr;
}

CachedContentResultSetStubFactory::~CachedContentResultSetStubFactory()
{
}

//--------------------------------------------------------------------------
// CachedContentResultSetStubFactory XInterface methods.
//--------------------------------------------------------------------------

XINTERFACE_IMPL_3( CachedContentResultSetStubFactory,
                   XTypeProvider,
                   XServiceInfo,
                   XCachedContentResultSetStubFactory );

//--------------------------------------------------------------------------
// CachedContentResultSetStubFactory XTypeProvider methods.
//--------------------------------------------------------------------------

XTYPEPROVIDER_IMPL_3( CachedContentResultSetStubFactory,
                      XTypeProvider,
                         XServiceInfo,
                      XCachedContentResultSetStubFactory );

//--------------------------------------------------------------------------
// CachedContentResultSetStubFactory XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_IMPL_1( CachedContentResultSetStubFactory,
                OUString::createFromAscii( "CachedContentResultSetStubFactory" ),
                OUString::createFromAscii( CACHED_CRS_STUB_FACTORY_NAME ) );

//--------------------------------------------------------------------------
// Service factory implementation.
//--------------------------------------------------------------------------

ONE_INSTANCE_SERVICE_FACTORY_IMPL( CachedContentResultSetStubFactory );

//--------------------------------------------------------------------------
// CachedContentResultSetStubFactory XCachedContentResultSetStubFactory methods.
//--------------------------------------------------------------------------

    //virtual
Reference< XResultSet > SAL_CALL CachedContentResultSetStubFactory
    ::createCachedContentResultSetStub(
            const Reference< XResultSet > & xSource )
            throw( RuntimeException )
{
    Reference< XResultSet > xRet;
    xRet = new CachedContentResultSetStub( xSource );
    return xRet;
}


