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
#include <sal/log.hxx>

#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/ucb/ListenerAlreadySetException.hpp>
#include <com/sun/star/ucb/ServiceNotFoundException.hpp>
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#include "filid.hxx"
#include "filtask.hxx"
#include "filprp.hxx"
#include "filrset.hxx"
#include <com/sun/star/ucb/OpenMode.hpp>
#include "prov.hxx"
#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#include <com/sun/star/ucb/CachedDynamicResultSetStubFactory.hpp>
#include <ucbhelper/resultsetmetadata.hxx>

using namespace fileaccess;
using namespace com::sun::star;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

XResultSet_impl::XResultSet_impl( TaskManager* pMyShell,
                                  const OUString& aUnqPath,
                                  sal_Int32 OpenMode,
                                  const uno::Sequence< beans::Property >& seq,
                                  const uno::Sequence< ucb::NumberedSortingInfo >& seqSort )
    : m_pMyShell( pMyShell )
    , m_nRow( -1 )
    , m_nWasNull ( false )
    , m_nOpenMode( OpenMode )
    , m_bRowCountFinal( false )
    , m_aBaseDirectory( aUnqPath )
    , m_aFolder( aUnqPath )
    , m_sProperty( seq )
    , m_sSortingInfo( seqSort )
    , m_nErrorCode( TASKHANDLER_NO_ERROR )
    , m_nMinorErrorCode( TASKHANDLER_NO_ERROR )
{
    osl::FileBase::RC err = m_aFolder.open();
    if(  err != osl::FileBase::E_None )
    {
        m_nIsOpen = false;
        m_aFolder.close();

        m_nErrorCode = TASKHANDLING_OPEN_FOR_DIRECTORYLISTING;
        m_nMinorErrorCode = err;
    }
    else
        m_nIsOpen = true;
}


XResultSet_impl::~XResultSet_impl()
{
    if( m_nIsOpen )
        m_aFolder.close();
}


void SAL_CALL
XResultSet_impl::disposing( const lang::EventObject& )
{
    // To do, but what
}


void SAL_CALL
XResultSet_impl::addEventListener(
    const uno::Reference< lang::XEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aDisposeEventListeners.addInterface( aGuard, Listener );
}


void SAL_CALL
XResultSet_impl::removeEventListener(
    const uno::Reference< lang::XEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aDisposeEventListeners.removeInterface( aGuard, Listener );
}


void SAL_CALL
XResultSet_impl::dispose()
{
    std::unique_lock aGuard( m_aMutex );

    lang::EventObject aEvt;
    aEvt.Source = static_cast< lang::XComponent * >( this );

    m_aDisposeEventListeners.disposeAndClear( aGuard, aEvt );
    m_aRowCountListeners.disposeAndClear( aGuard, aEvt );
    m_aIsFinalListeners.disposeAndClear( aGuard, aEvt );
}


void XResultSet_impl::rowCountChanged(std::unique_lock<std::mutex>& rGuard)
{
    sal_Int32 aOldValue,aNewValue;
    std::vector< uno::Reference< beans::XPropertyChangeListener > > seq = m_aRowCountListeners.getElements(rGuard);
    aNewValue = m_aItems.size();
    aOldValue = aNewValue-1;
    beans::PropertyChangeEvent aEv;
    aEv.PropertyName = "RowCount";
    aEv.Further = false;
    aEv.PropertyHandle = -1;
    aEv.OldValue <<= aOldValue;
    aEv.NewValue <<= aNewValue;
    for( const auto& listener : seq )
        listener->propertyChange( aEv );
}


void XResultSet_impl::isFinalChanged()
{
    std::vector< uno::Reference< beans::XPropertyChangeListener > > seq;
    {
        std::unique_lock aGuard( m_aMutex );
        seq = m_aIsFinalListeners.getElements(aGuard);
        m_bRowCountFinal = true;
    }
    beans::PropertyChangeEvent aEv;
    aEv.PropertyName = "IsRowCountFinal";
    aEv.Further = false;
    aEv.PropertyHandle = -1;
    aEv.OldValue <<= false;
    aEv.NewValue <<= true;
    for( const auto& listener : seq )
        listener->propertyChange( aEv );
}


bool
XResultSet_impl::OneMore()
{
    if( ! m_nIsOpen )
        return false;

    osl::FileBase::RC err;
    bool IsRegular;
    OUString aUnqPath;
    osl::DirectoryItem  aDirIte;
    uno::Reference< sdbc::XRow > aRow;

    while( true )
    {
        err = m_aFolder.getNextItem( aDirIte );

        if( err == osl::FileBase::E_NOENT || err == osl::FileBase::E_INVAL )
        {
            m_aFolder.close();
            isFinalChanged();
            m_nIsOpen = false;
            return m_nIsOpen;
        }
        else if( err == osl::FileBase::E_None )
        {
            if (!m_pMyShell->getv( m_sProperty, aDirIte, aUnqPath, IsRegular, aRow ))
            {
                SAL_WARN(
                    "ucb.ucp.file",
                    "getting dir item in <" << m_aBaseDirectory << "> failed");
                continue;
            }

            if( m_nOpenMode == ucb::OpenMode::DOCUMENTS && IsRegular )
            {
                std::unique_lock aGuard( m_aMutex );
                m_aItems.push_back( aRow );
                m_aIdents.emplace_back( );
                m_aUnqPath.push_back( aUnqPath );
                rowCountChanged(aGuard);
                return true;

            }
            else if( m_nOpenMode == ucb::OpenMode::DOCUMENTS && ! IsRegular )
            {
                continue;
            }
            else if( m_nOpenMode == ucb::OpenMode::FOLDERS && ! IsRegular )
            {
                std::unique_lock aGuard( m_aMutex );
                m_aItems.push_back( aRow );
                m_aIdents.emplace_back( );
                m_aUnqPath.push_back( aUnqPath );
                rowCountChanged(aGuard);
                return true;
            }
            else if( m_nOpenMode == ucb::OpenMode::FOLDERS && IsRegular )
            {
                continue;
            }
            else
            {
                std::unique_lock aGuard( m_aMutex );
                m_aItems.push_back( aRow );
                m_aIdents.emplace_back( );
                m_aUnqPath.push_back( aUnqPath );
                rowCountChanged(aGuard);
                return true;
            }
        }
        else  // error fetching anything
        {
            throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
        }
    }
}


sal_Bool SAL_CALL
XResultSet_impl::next()
{
    bool test;
    if( ++m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) ) test = true;
    else
        test = OneMore();
    return test;
}


sal_Bool SAL_CALL
XResultSet_impl::isBeforeFirst()
{
    return m_nRow == -1;
}


sal_Bool SAL_CALL
XResultSet_impl::isAfterLast()
{
    return m_nRow >= sal::static_int_cast<sal_Int32>(m_aItems.size());   // Cannot happen, if m_aFolder.isOpen()
}


sal_Bool SAL_CALL
XResultSet_impl::isFirst()
{
    return m_nRow == 0;
}


sal_Bool SAL_CALL
XResultSet_impl::isLast()
{
    if( m_nRow ==  sal::static_int_cast<sal_Int32>(m_aItems.size()) - 1 )
        return ! OneMore();
    else
        return false;
}


void SAL_CALL
XResultSet_impl::beforeFirst()
{
    m_nRow = -1;
}


void SAL_CALL
XResultSet_impl::afterLast()
{
    m_nRow = sal::static_int_cast<sal_Int32>(m_aItems.size());
    while( OneMore() )
        ++m_nRow;
}


sal_Bool SAL_CALL
XResultSet_impl::first()
{
    m_nRow = -1;
    return next();
}


sal_Bool SAL_CALL
XResultSet_impl::last()
{
    m_nRow = sal::static_int_cast<sal_Int32>(m_aItems.size()) - 1;
    while( OneMore() )
        ++m_nRow;
    return true;
}


sal_Int32 SAL_CALL
XResultSet_impl::getRow()
{
    // Test, whether behind last row
    if( -1 == m_nRow || m_nRow >= sal::static_int_cast<sal_Int32>(m_aItems.size()) )
        return 0;
    else
        return m_nRow+1;
}


sal_Bool SAL_CALL XResultSet_impl::absolute( sal_Int32 row )
{
    if( row >= 0 )
    {
        m_nRow = row - 1;
        if( row >= sal::static_int_cast<sal_Int32>(m_aItems.size()) )
            while( row-- && OneMore() )
                ;
    }
    else
    {
        last();
        m_nRow += ( row + 1 );
        if( m_nRow < -1 )
            m_nRow = -1;
    }

    return 0<= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size());
}


sal_Bool SAL_CALL
XResultSet_impl::relative( sal_Int32 row )
{
    if( isAfterLast() || isBeforeFirst() )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    if( row > 0 )
        while( row-- ) next();
    else if( row < 0 )
        while( row++ && m_nRow > - 1 ) previous();

    return 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size());
}


sal_Bool SAL_CALL
XResultSet_impl::previous()
{
    if( m_nRow > sal::static_int_cast<sal_Int32>(m_aItems.size()) )
        m_nRow = sal::static_int_cast<sal_Int32>(m_aItems.size());  // Correct Handling of afterLast
    if( 0 <= m_nRow ) -- m_nRow;

    return 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size());
}


void SAL_CALL
XResultSet_impl::refreshRow()
{
    // get the row from the filesystem
}


sal_Bool SAL_CALL
XResultSet_impl::rowUpdated()
{
    return false;
}

sal_Bool SAL_CALL
XResultSet_impl::rowInserted()
{
    return false;
}

sal_Bool SAL_CALL
XResultSet_impl::rowDeleted()
{
    return false;
}


uno::Reference< uno::XInterface > SAL_CALL
XResultSet_impl::getStatement()
{
    return uno::Reference< uno::XInterface >();
}


// XCloseable

void SAL_CALL
XResultSet_impl::close()
{
    if( m_nIsOpen )
    {
        m_aFolder.close();
        isFinalChanged();
        std::unique_lock aGuard( m_aMutex );
        m_nIsOpen = false;
    }
}


OUString SAL_CALL
XResultSet_impl::queryContentIdentifierString()
{
    uno::Reference< ucb::XContentIdentifier > xContentId
        = queryContentIdentifier();

    if( xContentId.is() )
        return xContentId->getContentIdentifier();
    else
        return OUString();
}


uno::Reference< ucb::XContentIdentifier > SAL_CALL
XResultSet_impl::queryContentIdentifier()
{
    if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
    {
        if( ! m_aIdents[m_nRow].is() )
        {
            m_aIdents[m_nRow].set( new FileContentIdentifier( m_aUnqPath[  m_nRow ] ) );
        }
        return m_aIdents[m_nRow];
    }
    return uno::Reference< ucb::XContentIdentifier >();
}


uno::Reference< ucb::XContent > SAL_CALL
XResultSet_impl::queryContent()
{
    if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
        return m_pMyShell->m_pProvider->queryContent( queryContentIdentifier() );
    else
        return uno::Reference< ucb::XContent >();
}


// XDynamicResultSet


// virtual
uno::Reference< sdbc::XResultSet > SAL_CALL
XResultSet_impl::getStaticResultSet()
{
    std::unique_lock aGuard( m_aMutex );

    if ( m_xListener.is() )
        throw ucb::ListenerAlreadySetException( THROW_WHERE );

    return uno::Reference< sdbc::XResultSet >( this );
}


// virtual
void SAL_CALL
XResultSet_impl::setListener(
    const uno::Reference< ucb::XDynamicResultSetListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    if ( m_xListener.is() )
        throw ucb::ListenerAlreadySetException( THROW_WHERE );

    m_xListener = Listener;


    // Create "welcome event" and send it to listener.


    // Note: We only have the implementation for a static result set at the
    //       moment (src590). The dynamic result sets passed to the listener
    //       are a fake. This implementation will never call "notify" at the
    //       listener to propagate any changes!!!

    uno::Any aInfo;
    aInfo <<= ucb::WelcomeDynamicResultSetStruct( this, /* "old" */
                                                  this /* "new" */ );

    uno::Sequence< ucb::ListAction > aActions( 1 );
    aActions.getArray()[ 0 ] = ucb::ListAction( 0, // Position; not used
                                                0, // Count; not used
                                                ucb::ListActionType::WELCOME,
                                                aInfo );
    aGuard.unlock();

    Listener->notify(
        ucb::ListEvent(
            getXWeak(), aActions ) );
}


// virtual
void SAL_CALL
XResultSet_impl::connectToCache(
    const uno::Reference< ucb::XDynamicResultSet > & xCache )
{
    if( m_xListener.is() )
        throw ucb::ListenerAlreadySetException( THROW_WHERE );

    uno::Reference< ucb::XSourceInitialization > xTarget(
        xCache, uno::UNO_QUERY );
    if( xTarget.is() && m_pMyShell->m_xContext.is() )
    {
        uno::Reference< ucb::XCachedDynamicResultSetStubFactory > xStubFactory;
        try
        {
            xStubFactory
                = ucb::CachedDynamicResultSetStubFactory::create(
                    m_pMyShell->m_xContext );
        }
        catch ( uno::Exception const & )
        {
        }

        if( xStubFactory.is() )
        {
            xStubFactory->connectToCache(
                this, xCache,m_sSortingInfo, nullptr );
            return;
        }
    }
    throw ucb::ServiceNotFoundException( THROW_WHERE );
}


// virtual
sal_Int16 SAL_CALL
XResultSet_impl::getCapabilities()
{
    // Never set ucb::ContentResultSetCapability::SORTED
    //  - Underlying content cannot provide sorted data...
    return 0;
}

// XResultSetMetaDataSupplier
uno::Reference< sdbc::XResultSetMetaData > SAL_CALL
XResultSet_impl::getMetaData()
{
    auto pProp = std::find_if(std::cbegin(m_sProperty), std::cend(m_sProperty),
        [](const beans::Property& rProp) { return rProp.Name == "Title"; });
    if (pProp != std::cend(m_sProperty))
    {
        std::vector< ::ucbhelper::ResultSetColumnData >
                                aColumnData( m_sProperty.getLength() );
        auto n = std::distance(std::cbegin(m_sProperty), pProp);
        // @@@ #82177# - Determine correct value!
        aColumnData[ n ].isCaseSensitive = false;

        return new ::ucbhelper::ResultSetMetaData(
                m_pMyShell->m_xContext,
                m_sProperty,
                std::move(aColumnData) );
    }

    return new ::ucbhelper::ResultSetMetaData( m_pMyShell->m_xContext, m_sProperty );
}


// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL
XResultSet_impl::getPropertySetInfo()
{

    uno::Sequence< beans::Property > seq
    {
        { u"RowCount"_ustr, -1, cppu::UnoType<sal_Int32>::get(), beans::PropertyAttribute::READONLY },
        { u"IsRowCountFinal"_ustr, -1, cppu::UnoType<sal_Bool>::get(), beans::PropertyAttribute::READONLY }
    };

    return new XPropertySetInfo_impl( m_pMyShell, seq );
}


void SAL_CALL XResultSet_impl::setPropertyValue(
    const OUString& aPropertyName, const uno::Any& )
{
    if( aPropertyName == "IsRowCountFinal" ||
        aPropertyName == "RowCount" )
        return;
    throw beans::UnknownPropertyException( aPropertyName );
}


uno::Any SAL_CALL XResultSet_impl::getPropertyValue(
    const OUString& PropertyName )
{
    if( PropertyName == "IsRowCountFinal" )
    {
        return uno::Any(m_bRowCountFinal);
    }
    else if ( PropertyName == "RowCount" )
    {
        sal_Int32 count = sal::static_int_cast<sal_Int32>(m_aItems.size());
        return uno::Any(count);
    }
    else
        throw beans::UnknownPropertyException( PropertyName );
}


void SAL_CALL XResultSet_impl::addPropertyChangeListener(
    const OUString& aPropertyName,
    const uno::Reference< beans::XPropertyChangeListener >& xListener )
{
    if( aPropertyName == "IsRowCountFinal" )
    {
        std::unique_lock aGuard( m_aMutex );

        m_aIsFinalListeners.addInterface( aGuard, xListener );
    }
    else if ( aPropertyName == "RowCount" )
    {
        std::unique_lock aGuard( m_aMutex );

        m_aRowCountListeners.addInterface( aGuard, xListener );
    }
    else
        throw beans::UnknownPropertyException( aPropertyName );
}


void SAL_CALL XResultSet_impl::removePropertyChangeListener(
    const OUString& aPropertyName,
    const uno::Reference< beans::XPropertyChangeListener >& aListener )
{
    if( aPropertyName == "IsRowCountFinal" )
    {
        std::unique_lock aGuard( m_aMutex );

        m_aIsFinalListeners.removeInterface( aGuard, aListener );
    }
    else if ( aPropertyName == "RowCount" )
    {
        std::unique_lock aGuard( m_aMutex );

        m_aRowCountListeners.removeInterface( aGuard, aListener );
    }
    else
        throw beans::UnknownPropertyException( aPropertyName );
}

void SAL_CALL XResultSet_impl::addVetoableChangeListener(
    const OUString&,
    const uno::Reference< beans::XVetoableChangeListener >& )
{
}


void SAL_CALL XResultSet_impl::removeVetoableChangeListener(
    const OUString&,
    const uno::Reference< beans::XVetoableChangeListener >& )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
