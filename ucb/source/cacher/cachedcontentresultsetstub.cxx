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


#include "cachedcontentresultsetstub.hxx"
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/ucb/FetchError.hpp>
#include <osl/diagnose.h>
#include <cppuhelper/queryinterface.hxx>
#include <ucbhelper/macros.hxx>

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;


CachedContentResultSetStub::CachedContentResultSetStub( Reference< XResultSet > const & xOrigin )
                : ContentResultSetWrapper( xOrigin )
                , m_nColumnCount( 0 )
                , m_bColumnCountCached( false )
                , m_bNeedToPropagateFetchSize( true )
                , m_bFirstFetchSizePropagationDone( false )
                , m_nLastFetchSize( 1 )//this value is not important at all
                , m_bLastFetchDirection( true )//this value is not important at all
                , m_aPropertyNameForFetchSize( u"FetchSize"_ustr )
                , m_aPropertyNameForFetchDirection( u"FetchDirection"_ustr )
{
    impl_init();
}

CachedContentResultSetStub::~CachedContentResultSetStub()
{
    impl_deinit();
}


// XInterface methods.
void SAL_CALL CachedContentResultSetStub::acquire()
    noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL CachedContentResultSetStub::release()
    noexcept
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


//virtual, only called from ContentResultSetWrapperListener
void CachedContentResultSetStub
    ::impl_propertyChange( const PropertyChangeEvent& rEvt )
{
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

    //don't notify events on fetchsize and fetchdirection to the above CachedContentResultSet
    //because it will ignore them anyway and we can save this remote calls
    if(    rEvt.PropertyName == m_aPropertyNameForFetchSize
        || rEvt.PropertyName == m_aPropertyNameForFetchDirection )
        return;

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = false;

    impl_notifyPropertyChangeListeners( aGuard, aEvt );
}


//virtual, only called from ContentResultSetWrapperListener
void CachedContentResultSetStub
    ::impl_vetoableChange( const PropertyChangeEvent& rEvt )
{
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

    //don't notify events on fetchsize and fetchdirection to the above CachedContentResultSet
    //because it will ignore them anyway and we can save this remote calls
    if(    rEvt.PropertyName == m_aPropertyNameForFetchSize
        || rEvt.PropertyName == m_aPropertyNameForFetchDirection )
        return;

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = false;

    impl_notifyVetoableChangeListeners( aGuard, aEvt );
}


// XTypeProvider methods.


XTYPEPROVIDER_COMMON_IMPL( CachedContentResultSetStub )
//list all interfaces exclusive baseclasses
Sequence< Type > SAL_CALL CachedContentResultSetStub
    ::getTypes()
{
    static Sequence<Type> ourTypes(
                {   CPPU_TYPE_REF( XTypeProvider ),
                    CPPU_TYPE_REF( XServiceInfo ),
                    CPPU_TYPE_REF( XComponent ),
                    CPPU_TYPE_REF( XCloseable ),
                    CPPU_TYPE_REF( XResultSetMetaDataSupplier ),
                    CPPU_TYPE_REF( XPropertySet ),
                    CPPU_TYPE_REF( XPropertyChangeListener ),
                    CPPU_TYPE_REF( XVetoableChangeListener ),
                    CPPU_TYPE_REF( XResultSet ),
                    CPPU_TYPE_REF( XContentAccess ),
                    CPPU_TYPE_REF( XRow ),
                    CPPU_TYPE_REF( XFetchProvider ),
                    CPPU_TYPE_REF( XFetchProviderForContentAccess ) } );

    return ourTypes;
}


// XServiceInfo methods.

OUString SAL_CALL CachedContentResultSetStub::getImplementationName()
{
    return u"com.sun.star.comp.ucb.CachedContentResultSetStub"_ustr;
}

sal_Bool SAL_CALL CachedContentResultSetStub::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL CachedContentResultSetStub::getSupportedServiceNames()
{
    return { u"com.sun.star.ucb.CachedContentResultSetStub"_ustr };
}



// XFetchProvider methods.


FetchResult CachedContentResultSetStub::impl_fetchHelper(
        std::unique_lock<std::mutex>& rGuard,
        sal_Int32 nRowStartPosition, sal_Int32 nRowCount, bool bDirection,
        std::function<void( std::unique_lock<std::mutex>&, css::uno::Any& rRowContent)> impl_loadRow)
{
    impl_EnsureNotDisposed(rGuard);
    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    impl_propagateFetchSizeAndDirection( rGuard, nRowCount, bDirection );
    FetchResult aRet;
    aRet.StartIndex = nRowStartPosition;
    aRet.Orientation = bDirection;
    aRet.FetchError = FetchError::SUCCESS; /*ENDOFDATA, EXCEPTION*/
    sal_Int32 nOldOriginal_Pos = m_xResultSetOrigin->getRow();
    if( impl_isForwardOnly(rGuard) )
    {
        if( nOldOriginal_Pos != nRowStartPosition )
        {
            /*@todo*/
            aRet.FetchError = FetchError::EXCEPTION;
            return aRet;
        }
        if( nRowCount != 1 )
            aRet.FetchError = FetchError::EXCEPTION;

        aRet.Rows.realloc( 1 );

        try
        {
            impl_loadRow( rGuard, aRet.Rows.getArray()[0] );
        }
        catch( SQLException& )
        {
            aRet.Rows.realloc( 0 );
            aRet.FetchError = FetchError::EXCEPTION;
            return aRet;
        }
        return aRet;
    }
    aRet.Rows.realloc( nRowCount );
    auto pRows = aRet.Rows.getArray();
    bool bOldOriginal_AfterLast = false;
    if( !nOldOriginal_Pos )
        bOldOriginal_AfterLast = m_xResultSetOrigin->isAfterLast();
    sal_Int32 nN = 1;
    try
    {
        bool bValidNewPos = false;
        try
        {
            /*if( nOldOriginal_Pos != nRowStartPosition )*/
            bValidNewPos = m_xResultSetOrigin->absolute( nRowStartPosition );
        }
        catch( SQLException& )
        {
            aRet.Rows.realloc( 0 );
            aRet.FetchError = FetchError::EXCEPTION;
            return aRet;
        }
        if( !bValidNewPos )
        {
            aRet.Rows.realloc( 0 );
            aRet.FetchError = FetchError::EXCEPTION;

            /*restore old position*/
            if( nOldOriginal_Pos )
                m_xResultSetOrigin->absolute( nOldOriginal_Pos );
            else if( bOldOriginal_AfterLast )
                m_xResultSetOrigin->afterLast();
            else
                m_xResultSetOrigin->beforeFirst();

            return aRet;
        }
        for( ; nN <= nRowCount; )
        {
            impl_loadRow( rGuard, pRows[nN-1] );
            nN++;
            if( nN <= nRowCount )
            {
                if( bDirection )
                {
                    if( !m_xResultSetOrigin->next() )
                    {
                        aRet.Rows.realloc( nN-1 );
                        aRet.FetchError = FetchError::ENDOFDATA;
                        break;
                    }
                }
                else
                {
                    if( !m_xResultSetOrigin->previous() )
                    {
                        aRet.Rows.realloc( nN-1 );
                        aRet.FetchError = FetchError::ENDOFDATA;
                        break;
                    }
                }
            }
        }
    }
    catch( SQLException& )
    {
        aRet.Rows.realloc( nN-1 );
        aRet.FetchError = FetchError::EXCEPTION;
    }
    /*restore old position*/
    if( nOldOriginal_Pos )
        m_xResultSetOrigin->absolute( nOldOriginal_Pos );
    else if( bOldOriginal_AfterLast )
        m_xResultSetOrigin->afterLast();
    else
        m_xResultSetOrigin->beforeFirst();
    return aRet;
}

FetchResult SAL_CALL CachedContentResultSetStub
    ::fetch( sal_Int32 nRowStartPosition
    , sal_Int32 nRowCount, sal_Bool bDirection )
{
    std::unique_lock aGuard(m_aMutex);
    impl_init_xRowOrigin(aGuard);
    return impl_fetchHelper( aGuard, nRowStartPosition, nRowCount, bDirection,
        [&](std::unique_lock<std::mutex>& rGuard, css::uno::Any& rRowContent)
        { return impl_getCurrentRowContent(rGuard, rRowContent, m_xRowOrigin); });
}

sal_Int32 CachedContentResultSetStub
    ::impl_getColumnCount(std::unique_lock<std::mutex>& /*rGuard*/)
{
    sal_Int32 nCount;
    bool bCached;
    nCount = m_nColumnCount;
    bCached = m_bColumnCountCached;
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
    m_nColumnCount = nCount;
    m_bColumnCountCached = true;
    return m_nColumnCount;
}

void CachedContentResultSetStub
    ::impl_getCurrentRowContent( std::unique_lock<std::mutex>& rGuard, Any& rRowContent
        , const Reference< XRow >& xRow )
{
    sal_Int32 nCount = impl_getColumnCount(rGuard);

    Sequence< Any > aContent( nCount );
    auto aContentRange = asNonConstRange(aContent);
    for( sal_Int32 nN = 1; nN <= nCount; nN++ )
    {
        aContentRange[nN-1] = xRow->getObject( nN, nullptr );
    }

    rRowContent <<= aContent;
}

void CachedContentResultSetStub
    ::impl_propagateFetchSizeAndDirection( std::unique_lock<std::mutex>& rGuard, sal_Int32 nFetchSize, bool bFetchDirection )
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
    bNeedAction             = m_bNeedToPropagateFetchSize;
    nLastSize               = m_nLastFetchSize;
    bLastDirection          = m_bLastFetchDirection;
    bFirstPropagationDone   = m_bFirstFetchSizePropagationDone;
    if( !bNeedAction )
        return;

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
            m_bNeedToPropagateFetchSize = false;
            return;
        }
    }

    bool bSetSize       = ( nLastSize       !=nFetchSize        ) || !bFirstPropagationDone;
    bool bSetDirection  = ( bLastDirection  !=bFetchDirection   ) || !bFirstPropagationDone;

    m_bFirstFetchSizePropagationDone = true;
    m_nLastFetchSize        = nFetchSize;
    m_bLastFetchDirection   = bFetchDirection;

    if( bSetSize )
    {
        Any aValue;
        aValue <<= nFetchSize;
        try
        {
            setPropertyValueImpl( rGuard, m_aPropertyNameForFetchSize, aValue );
        }
        catch( css::uno::Exception& ) {}
    }
    if( !bSetDirection )
        return;

    sal_Int32 nFetchDirection = FetchDirection::FORWARD;
    if( !bFetchDirection )
        nFetchDirection = FetchDirection::REVERSE;
    Any aValue;
    aValue <<= nFetchDirection;
    try
    {
        setPropertyValueImpl( rGuard, m_aPropertyNameForFetchDirection, aValue );
    }
    catch( css::uno::Exception& ) {}
}


// XFetchProviderForContentAccess methods.


void CachedContentResultSetStub
    ::impl_getCurrentContentIdentifierString( std::unique_lock<std::mutex>& /*rGuard*/, Any& rAny
        , const Reference< XContentAccess >& xContentAccess )
{
     rAny <<= xContentAccess->queryContentIdentifierString();
}

void CachedContentResultSetStub
    ::impl_getCurrentContentIdentifier( std::unique_lock<std::mutex>& /*rGuard*/, Any& rAny
        , const Reference< XContentAccess >& xContentAccess )
{
     rAny <<= xContentAccess->queryContentIdentifier();
}

void CachedContentResultSetStub
    ::impl_getCurrentContent( std::unique_lock<std::mutex>& /*rGuard*/, Any& rAny
        , const Reference< XContentAccess >& xContentAccess )
{
     rAny <<= xContentAccess->queryContent();
}

//virtual
FetchResult SAL_CALL CachedContentResultSetStub
    ::fetchContentIdentifierStrings( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
{
    std::unique_lock aGuard( m_aMutex );
    impl_init_xContentAccessOrigin(aGuard);
    return impl_fetchHelper( aGuard, nRowStartPosition, nRowCount, bDirection,
        [&](std::unique_lock<std::mutex>& rGuard, css::uno::Any& rRowContent)
        { return impl_getCurrentContentIdentifierString(rGuard, rRowContent, m_xContentAccessOrigin); });
}

//virtual
FetchResult SAL_CALL CachedContentResultSetStub
    ::fetchContentIdentifiers( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
{
    std::unique_lock aGuard( m_aMutex );
    impl_init_xContentAccessOrigin(aGuard);
    return impl_fetchHelper( aGuard, nRowStartPosition, nRowCount, bDirection,
        [&](std::unique_lock<std::mutex>& rGuard, css::uno::Any& rRowContent)
        { return impl_getCurrentContentIdentifier(rGuard, rRowContent, m_xContentAccessOrigin); });
}

//virtual
FetchResult SAL_CALL CachedContentResultSetStub
    ::fetchContents( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
{
    std::unique_lock aGuard( m_aMutex );
    impl_init_xContentAccessOrigin(aGuard);
    return impl_fetchHelper( aGuard, nRowStartPosition, nRowCount, bDirection,
        [&](std::unique_lock<std::mutex>& rGuard, css::uno::Any& rRowContent)
        { return impl_getCurrentContent(rGuard, rRowContent, m_xContentAccessOrigin); });
}




CachedContentResultSetStubFactory::CachedContentResultSetStubFactory()
{
}

CachedContentResultSetStubFactory::~CachedContentResultSetStubFactory()
{
}


// CachedContentResultSetStubFactory XServiceInfo methods.

OUString SAL_CALL CachedContentResultSetStubFactory::getImplementationName()
{
    return u"com.sun.star.comp.ucb.CachedContentResultSetStubFactory"_ustr;
}
sal_Bool SAL_CALL CachedContentResultSetStubFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}
css::uno::Sequence< OUString > SAL_CALL CachedContentResultSetStubFactory::getSupportedServiceNames()
{
    return { u"com.sun.star.ucb.CachedContentResultSetStubFactory"_ustr };
}

// Service factory implementation.


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_CachedContentResultSetStubFactory_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new CachedContentResultSetStubFactory());
}




// CachedContentResultSetStubFactory XCachedContentResultSetStubFactory methods.


    //virtual
Reference< XResultSet > SAL_CALL CachedContentResultSetStubFactory
    ::createCachedContentResultSetStub(
            const Reference< XResultSet > & xSource )
{
    if( xSource.is() )
    {
        Reference< XResultSet > xRet = new CachedContentResultSetStub( xSource );
        return xRet;
    }
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
