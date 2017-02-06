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


#include <cachedcontentresultsetstub.hxx>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/ucb/FetchError.hpp>
#include <osl/diagnose.h>

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace cppu;


CachedContentResultSetStub::CachedContentResultSetStub( Reference< XResultSet > const & xOrigin )
                : ContentResultSetWrapper( xOrigin )
                , m_nColumnCount( 0 )
                , m_bColumnCountCached( false )
                , m_bNeedToPropagateFetchSize( true )
                , m_bFirstFetchSizePropagationDone( false )
                , m_nLastFetchSize( 1 )//this value is not important at all
                , m_bLastFetchDirection( true )//this value is not important at all
                , m_aPropertyNameForFetchSize( OUString("FetchSize") )
                , m_aPropertyNameForFetchDirection( OUString("FetchDirection") )
{
    impl_init();
}

CachedContentResultSetStub::~CachedContentResultSetStub()
{
    impl_deinit();
}


// XInterface methods.
void SAL_CALL CachedContentResultSetStub::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL CachedContentResultSetStub::release()
    throw()
{
    OWeakObject::release();
}

Any SAL_CALL CachedContentResultSetStub
    ::queryInterface( const Type&  rType )
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


// own methods.  ( inherited )


//virtual
void SAL_CALL CachedContentResultSetStub
    ::impl_propertyChange( const PropertyChangeEvent& rEvt )
{
    impl_EnsureNotDisposed();

    //don't notify events on fetchsize and fetchdirection to the above CachedContentResultSet
    //because it will ignore them anyway and we can save this remote calls
    if(    rEvt.PropertyName == m_aPropertyNameForFetchSize
        || rEvt.PropertyName == m_aPropertyNameForFetchDirection )
        return;

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = false;

    impl_notifyPropertyChangeListeners( aEvt );
}


//virtual
void SAL_CALL CachedContentResultSetStub
    ::impl_vetoableChange( const PropertyChangeEvent& rEvt )
{
    impl_EnsureNotDisposed();

    //don't notify events on fetchsize and fetchdirection to the above CachedContentResultSet
    //because it will ignore them anyway and we can save this remote calls
    if(    rEvt.PropertyName == m_aPropertyNameForFetchSize
        || rEvt.PropertyName == m_aPropertyNameForFetchDirection )
        return;

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = false;

    impl_notifyVetoableChangeListeners( aEvt );
}


// XTypeProvider methods.


XTYPEPROVIDER_COMMON_IMPL( CachedContentResultSetStub )
//list all interfaces exclusive baseclasses
Sequence< Type > SAL_CALL CachedContentResultSetStub
    ::getTypes()
{
    static Sequence< Type >* pTypes = nullptr;
    if( !pTypes )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pTypes )
        {
            pTypes = new Sequence< Type >(13);
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


// XServiceInfo methods.

OUString SAL_CALL CachedContentResultSetStub::getImplementationName()
{
    return OUString( "com.sun.star.comp.ucb.CachedContentResultSetStub" );
}

sal_Bool SAL_CALL CachedContentResultSetStub::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL CachedContentResultSetStub::getSupportedServiceNames()
{
    return { CACHED_CRS_STUB_SERVICE_NAME };
}



// XFetchProvider methods.


#define FETCH_XXX( impl_loadRow, loadInterface ) \
impl_EnsureNotDisposed(); \
if( !m_xResultSetOrigin.is() ) \
{ \
    OSL_FAIL( "broadcaster was disposed already" ); \
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
bool bOldOriginal_AfterLast = false; \
if( !nOldOriginal_Pos ) \
    bOldOriginal_AfterLast = m_xResultSetOrigin->isAfterLast(); \
sal_Int32 nN = 1; \
bool bValidNewPos = false; \
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
{
    impl_init_xRowOrigin();
    FETCH_XXX( impl_getCurrentRowContent, m_xRowOrigin );
}

sal_Int32 SAL_CALL CachedContentResultSetStub
    ::impl_getColumnCount()
{
    sal_Int32 nCount;
    bool bCached;
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
            OSL_FAIL( "couldn't determine the column count" );
            nCount = 0;
        }
    }
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_nColumnCount = nCount;
    m_bColumnCountCached = true;
    return m_nColumnCount;
}

void SAL_CALL CachedContentResultSetStub
    ::impl_getCurrentRowContent( Any& rRowContent
        , const Reference< XRow >& xRow )
{
    sal_Int32 nCount = impl_getColumnCount();

    Sequence< Any > aContent( nCount );
    for( sal_Int32 nN = 1; nN <= nCount; nN++ )
    {
        aContent[nN-1] = xRow->getObject( nN, nullptr );
    }

    rRowContent <<= aContent;
}

void SAL_CALL CachedContentResultSetStub
    ::impl_propagateFetchSizeAndDirection( sal_Int32 nFetchSize, bool bFetchDirection )
{
    //this is done only for the case, that there is another CachedContentResultSet in the chain of underlying ResultSets

    //we do not propagate the property 'FetchSize' or 'FetchDirection' via 'setPropertyValue' from the above CachedContentResultSet to save remote calls

    //if the underlying ResultSet has a property FetchSize and FetchDirection,
    //we will set these properties, if the new given parameters are different from the last ones

    if( !m_bNeedToPropagateFetchSize )
        return;

    bool bNeedAction;
    sal_Int32 nLastSize;
    bool bLastDirection;
    bool bFirstPropagationDone;
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
            && bFirstPropagationDone )
            return;

        if(!bFirstPropagationDone)
        {
            //check whether the properties 'FetchSize' and 'FetchDirection' do exist

            Reference< XPropertySetInfo > xPropertySetInfo = getPropertySetInfo();
            bool bHasSize = xPropertySetInfo->hasPropertyByName( m_aPropertyNameForFetchSize );
            bool bHasDirection = xPropertySetInfo->hasPropertyByName( m_aPropertyNameForFetchDirection );

            if(!bHasSize || !bHasDirection)
            {
                osl::Guard< osl::Mutex > aGuard( m_aMutex );
                m_bNeedToPropagateFetchSize = false;
                return;
            }
        }

        bool bSetSize       = ( nLastSize       !=nFetchSize        ) || !bFirstPropagationDone;
        bool bSetDirection  = ( bLastDirection  !=bFetchDirection   ) || !bFirstPropagationDone;

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_bFirstFetchSizePropagationDone = true;
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
            catch( css::uno::Exception& ) {}
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
            catch( css::uno::Exception& ) {}
        }

    }
}


// XFetchProviderForContentAccess methods.


void SAL_CALL CachedContentResultSetStub
    ::impl_getCurrentContentIdentifierString( Any& rAny
        , const Reference< XContentAccess >& xContentAccess )
{
     rAny <<= xContentAccess->queryContentIdentifierString();
}

void SAL_CALL CachedContentResultSetStub
    ::impl_getCurrentContentIdentifier( Any& rAny
        , const Reference< XContentAccess >& xContentAccess )
{
     rAny <<= xContentAccess->queryContentIdentifier();
}

void SAL_CALL CachedContentResultSetStub
    ::impl_getCurrentContent( Any& rAny
        , const Reference< XContentAccess >& xContentAccess )
{
     rAny <<= xContentAccess->queryContent();
}

//virtual
FetchResult SAL_CALL CachedContentResultSetStub
    ::fetchContentIdentifierStrings( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
{
    impl_init_xContentAccessOrigin();
    FETCH_XXX( impl_getCurrentContentIdentifierString, m_xContentAccessOrigin );
}

//virtual
FetchResult SAL_CALL CachedContentResultSetStub
    ::fetchContentIdentifiers( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
{
    impl_init_xContentAccessOrigin();
    FETCH_XXX( impl_getCurrentContentIdentifier, m_xContentAccessOrigin );
}

//virtual
FetchResult SAL_CALL CachedContentResultSetStub
    ::fetchContents( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
{
    impl_init_xContentAccessOrigin();
    FETCH_XXX( impl_getCurrentContent, m_xContentAccessOrigin );
}


// class CachedContentResultSetStubFactory


CachedContentResultSetStubFactory::CachedContentResultSetStubFactory(
        const Reference< XMultiServiceFactory > & rSMgr )
{
    m_xSMgr = rSMgr;
}

CachedContentResultSetStubFactory::~CachedContentResultSetStubFactory()
{
}


// CachedContentResultSetStubFactory XInterface methods.
void SAL_CALL CachedContentResultSetStubFactory::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL CachedContentResultSetStubFactory::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL CachedContentResultSetStubFactory::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< XTypeProvider* >(this)),
                                               (static_cast< XServiceInfo* >(this)),
                                               (static_cast< XCachedContentResultSetStubFactory* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// CachedContentResultSetStubFactory XTypeProvider methods.


XTYPEPROVIDER_IMPL_3( CachedContentResultSetStubFactory,
                      XTypeProvider,
                         XServiceInfo,
                      XCachedContentResultSetStubFactory );


// CachedContentResultSetStubFactory XServiceInfo methods.

XSERVICEINFO_COMMOM_IMPL( CachedContentResultSetStubFactory,
                          OUString( "com.sun.star.comp.ucb.CachedContentResultSetStubFactory" ) )
/// @throws css::uno::Exception
static css::uno::Reference< css::uno::XInterface > SAL_CALL
CachedContentResultSetStubFactory_CreateInstance( const css::uno::Reference< css::lang::XMultiServiceFactory> & rSMgr )
{
    css::lang::XServiceInfo* pX =
        static_cast<css::lang::XServiceInfo*>(new CachedContentResultSetStubFactory( rSMgr ));
    return css::uno::Reference< css::uno::XInterface >::query( pX );
}
css::uno::Sequence< OUString >
CachedContentResultSetStubFactory::getSupportedServiceNames_Static()
{
    return { CACHED_CRS_STUB_FACTORY_NAME };
}

// Service factory implementation.


ONE_INSTANCE_SERVICE_FACTORY_IMPL( CachedContentResultSetStubFactory );


// CachedContentResultSetStubFactory XCachedContentResultSetStubFactory methods.


    //virtual
Reference< XResultSet > SAL_CALL CachedContentResultSetStubFactory
    ::createCachedContentResultSetStub(
            const Reference< XResultSet > & xSource )
{
    if( xSource.is() )
    {
        Reference< XResultSet > xRet;
        xRet = new CachedContentResultSetStub( xSource );
        return xRet;
    }
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
