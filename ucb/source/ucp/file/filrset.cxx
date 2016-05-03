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

#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#include "filid.hxx"
#include "shell.hxx"
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

XResultSet_impl::XResultSet_impl( shell* pMyShell,
                                  const OUString& aUnqPath,
                                  sal_Int32 OpenMode,
                                  const uno::Sequence< beans::Property >& seq,
                                  const uno::Sequence< ucb::NumberedSortingInfo >& seqSort )
    : m_pMyShell( pMyShell )
    , m_xProvider( pMyShell->m_pProvider )
    , m_nRow( -1 )
    , m_nWasNull ( false )
    , m_nOpenMode( OpenMode )
    , m_bRowCountFinal( false )
    , m_aBaseDirectory( aUnqPath )
    , m_aFolder( aUnqPath )
    , m_sProperty( seq )
    , m_sSortingInfo( seqSort )
    , m_pDisposeEventListeners( nullptr )
    , m_pRowCountListeners( nullptr )
    , m_pIsFinalListeners( nullptr )
    , m_bStatic( false )
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

    m_pMyShell->registerNotifier( m_aBaseDirectory,this );
}


XResultSet_impl::~XResultSet_impl()
{
    m_pMyShell->deregisterNotifier( m_aBaseDirectory,this );

    if( m_nIsOpen )
        m_aFolder.close();

    delete m_pDisposeEventListeners;
    delete m_pRowCountListeners;
    delete m_pIsFinalListeners;
}


void SAL_CALL
XResultSet_impl::disposing( const lang::EventObject& )
    throw( uno::RuntimeException, std::exception )
{
    // To do, but what
}


void SAL_CALL
XResultSet_impl::addEventListener(
    const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( ! m_pDisposeEventListeners )
        m_pDisposeEventListeners =
            new cppu::OInterfaceContainerHelper( m_aEventListenerMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL
XResultSet_impl::removeEventListener(
    const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


void SAL_CALL
XResultSet_impl::dispose()
    throw( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    lang::EventObject aEvt;
    aEvt.Source = static_cast< lang::XComponent * >( this );

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }
    if( m_pRowCountListeners && m_pRowCountListeners->getLength() )
    {
        m_pRowCountListeners->disposeAndClear( aEvt );
    }
    if( m_pIsFinalListeners && m_pIsFinalListeners->getLength() )
    {
        m_pIsFinalListeners->disposeAndClear( aEvt );
    }
}


void XResultSet_impl::rowCountChanged()
{
    sal_Int32 aOldValue,aNewValue;
    uno::Sequence< uno::Reference< uno::XInterface > > seq;
    {
        osl::MutexGuard aGuard( m_aMutex );
        if( m_pRowCountListeners )
            seq = m_pRowCountListeners->getElements();
        aNewValue = m_aItems.size();
        aOldValue = aNewValue-1;
    }
    beans::PropertyChangeEvent aEv;
    aEv.PropertyName = "RowCount";
    aEv.Further = false;
    aEv.PropertyHandle = -1;
    aEv.OldValue <<= aOldValue;
    aEv.NewValue <<= aNewValue;
    for( sal_Int32 i = 0; i < seq.getLength(); ++i )
    {
        uno::Reference< beans::XPropertyChangeListener > listener(
            seq[i], uno::UNO_QUERY );
        if( listener.is() )
            listener->propertyChange( aEv );
    }
}


void XResultSet_impl::isFinalChanged()
{
    uno::Sequence< uno::Reference< XInterface > > seq;
    {
        osl::MutexGuard aGuard( m_aMutex );
        if( m_pIsFinalListeners )
            seq = m_pIsFinalListeners->getElements();
        m_bRowCountFinal = true;
    }
    beans::PropertyChangeEvent aEv;
    aEv.PropertyName = "IsRowCountFinal";
    aEv.Further = false;
    aEv.PropertyHandle = -1;
    bool fval = false;
    bool tval = true;
    aEv.OldValue <<= fval;
    aEv.NewValue <<= tval;
    for( sal_Int32 i = 0; i < seq.getLength(); ++i )
    {
        uno::Reference< beans::XPropertyChangeListener > listener(
            seq[i], uno::UNO_QUERY );
        if( listener.is() )
            listener->propertyChange( aEv );
    }
}


bool SAL_CALL
XResultSet_impl::OneMore(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException,
           std::exception )
{
    if( ! m_nIsOpen )
        return false;

    osl::FileBase::RC err;
    bool IsRegular;
    OUString aUnqPath;
    osl::DirectoryItem  m_aDirIte;
    uno::Reference< sdbc::XRow > aRow;

    while( true )
    {
        err = m_aFolder.getNextItem( m_aDirIte );

        if( err == osl::FileBase::E_NOENT || err == osl::FileBase::E_INVAL )
        {
            m_aFolder.close();
            isFinalChanged();
            return ( m_nIsOpen = false );
        }
        else if( err == osl::FileBase::E_None )
        {
            aRow = m_pMyShell->getv(
                this, m_sProperty, m_aDirIte, aUnqPath, IsRegular );

            if( m_nOpenMode == ucb::OpenMode::DOCUMENTS && IsRegular )
            {
                osl::MutexGuard aGuard( m_aMutex );
                m_aItems.push_back( aRow );
                m_aIdents.push_back(
                    uno::Reference< ucb::XContentIdentifier >() );
                m_aUnqPath.push_back( aUnqPath );
                rowCountChanged();
                return true;

            }
            else if( m_nOpenMode == ucb::OpenMode::DOCUMENTS && ! IsRegular )
            {
                continue;
            }
            else if( m_nOpenMode == ucb::OpenMode::FOLDERS && ! IsRegular )
            {
                osl::MutexGuard aGuard( m_aMutex );
                m_aItems.push_back( aRow );
                m_aIdents.push_back(
                    uno::Reference< ucb::XContentIdentifier >() );
                m_aUnqPath.push_back( aUnqPath );
                rowCountChanged();
                return true;
            }
            else if( m_nOpenMode == ucb::OpenMode::FOLDERS && IsRegular )
            {
                continue;
            }
            else
            {
                osl::MutexGuard aGuard( m_aMutex );
                m_aItems.push_back( aRow );
                m_aIdents.push_back(
                    uno::Reference< ucb::XContentIdentifier >() );
                m_aUnqPath.push_back( aUnqPath );
                rowCountChanged();
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
XResultSet_impl::next(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    bool test;
    if( ++m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) ) test = true;
    else
        test = OneMore();
    return test;
}


sal_Bool SAL_CALL
XResultSet_impl::isBeforeFirst(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return m_nRow == -1;
}


sal_Bool SAL_CALL
XResultSet_impl::isAfterLast(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return m_nRow >= sal::static_int_cast<sal_Int32>(m_aItems.size());   // Cannot happen, if m_aFolder.isOpen()
}


sal_Bool SAL_CALL
XResultSet_impl::isFirst(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return m_nRow == 0;
}


sal_Bool SAL_CALL
XResultSet_impl::isLast(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    if( m_nRow ==  sal::static_int_cast<sal_Int32>(m_aItems.size()) - 1 )
        return ! OneMore();
    else
        return false;
}


void SAL_CALL
XResultSet_impl::beforeFirst(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    m_nRow = -1;
}


void SAL_CALL
XResultSet_impl::afterLast(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    m_nRow = sal::static_int_cast<sal_Int32>(m_aItems.size());
    while( OneMore() )
        ++m_nRow;
}


sal_Bool SAL_CALL
XResultSet_impl::first(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    m_nRow = -1;
    return next();
}


sal_Bool SAL_CALL
XResultSet_impl::last(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    m_nRow = sal::static_int_cast<sal_Int32>(m_aItems.size()) - 1;
    while( OneMore() )
        ++m_nRow;
    return true;
}


sal_Int32 SAL_CALL
XResultSet_impl::getRow(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    // Test, whether behind last row
    if( -1 == m_nRow || m_nRow >= sal::static_int_cast<sal_Int32>(m_aItems.size()) )
        return 0;
    else
        return m_nRow+1;
}


sal_Bool SAL_CALL XResultSet_impl::absolute( sal_Int32 row )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception)
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
XResultSet_impl::relative(
    sal_Int32 row )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
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
XResultSet_impl::previous(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    if( m_nRow > sal::static_int_cast<sal_Int32>(m_aItems.size()) )
        m_nRow = sal::static_int_cast<sal_Int32>(m_aItems.size());  // Correct Handling of afterLast
    if( 0 <= m_nRow ) -- m_nRow;

    return 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size());
}


void SAL_CALL
XResultSet_impl::refreshRow(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    // get the row from the filesystem
    return;
}


sal_Bool SAL_CALL
XResultSet_impl::rowUpdated(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return false;
}

sal_Bool SAL_CALL
XResultSet_impl::rowInserted(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return false;
}

sal_Bool SAL_CALL
XResultSet_impl::rowDeleted(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return false;
}


uno::Reference< uno::XInterface > SAL_CALL
XResultSet_impl::getStatement(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return uno::Reference< uno::XInterface >();
}


// XCloseable

void SAL_CALL
XResultSet_impl::close(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    if( m_nIsOpen )
    {
        m_aFolder.close();
        isFinalChanged();
        osl::MutexGuard aGuard( m_aMutex );
        m_nIsOpen = false;
    }
}


OUString SAL_CALL
XResultSet_impl::queryContentIdentifierString(
    void )
    throw( uno::RuntimeException, std::exception )
{
    uno::Reference< ucb::XContentIdentifier > xContentId
        = queryContentIdentifier();

    if( xContentId.is() )
        return xContentId->getContentIdentifier();
    else
        return OUString();
}


uno::Reference< ucb::XContentIdentifier > SAL_CALL
XResultSet_impl::queryContentIdentifier(
    void )
    throw( uno::RuntimeException, std::exception )
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
XResultSet_impl::queryContent(
    void )
    throw( uno::RuntimeException, std::exception )
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
    throw( ucb::ListenerAlreadySetException,
           uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_xListener.is() )
        throw ucb::ListenerAlreadySetException( THROW_WHERE );

    return uno::Reference< sdbc::XResultSet >( this );
}


// virtual
void SAL_CALL
XResultSet_impl::setListener(
    const uno::Reference< ucb::XDynamicResultSetListener >& Listener )
    throw( ucb::ListenerAlreadySetException,
           uno::RuntimeException, std::exception )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );

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
    aGuard.clear();

    Listener->notify(
        ucb::ListEvent(
            static_cast< cppu::OWeakObject * >( this ), aActions ) );
}


// virtual
void SAL_CALL
XResultSet_impl::connectToCache(
    const uno::Reference< ucb::XDynamicResultSet > & xCache )
    throw( ucb::ListenerAlreadySetException,
           ucb::AlreadyInitializedException,
           ucb::ServiceNotFoundException,
           uno::RuntimeException, std::exception )
{
    if( m_xListener.is() )
        throw ucb::ListenerAlreadySetException( THROW_WHERE );
    if( m_bStatic )
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
  throw( uno::RuntimeException, std::exception )
{
    // Never set ucb::ContentResultSetCapability::SORTED
    //  - Underlying content cannot provide sorted data...
    return 0;
}

// XResultSetMetaDataSupplier
uno::Reference< sdbc::XResultSetMetaData > SAL_CALL
XResultSet_impl::getMetaData(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    for ( sal_Int32 n = 0; n < m_sProperty.getLength(); ++n )
    {
        if ( m_sProperty.getConstArray()[ n ].Name == "Title" )
        {
            // @@@ #82177# - Determine correct value!
            bool bCaseSensitiveChildren = false;

            std::vector< ::ucbhelper::ResultSetColumnData >
                                    aColumnData( m_sProperty.getLength() );
            aColumnData[ n ].isCaseSensitive = bCaseSensitiveChildren;

            ::ucbhelper::ResultSetMetaData* p =
                new ::ucbhelper::ResultSetMetaData(
                    m_pMyShell->m_xContext,
                    m_sProperty,
                    aColumnData );
            return uno::Reference< sdbc::XResultSetMetaData >( p );
        }
    }

    ::ucbhelper::ResultSetMetaData* p =
            new ::ucbhelper::ResultSetMetaData( m_pMyShell->m_xContext, m_sProperty );
    return uno::Reference< sdbc::XResultSetMetaData >( p );
}


// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL
XResultSet_impl::getPropertySetInfo()
    throw( uno::RuntimeException, std::exception)
{

    uno::Sequence< beans::Property > seq(2);
    seq[0].Name = "RowCount";
    seq[0].Handle = -1;
    seq[0].Type = cppu::UnoType<sal_Int32>::get();
    seq[0].Attributes = beans::PropertyAttribute::READONLY;

    seq[0].Name = "IsRowCountFinal";
    seq[0].Handle = -1;
    seq[0].Type = cppu::UnoType<sal_Bool>::get();
    seq[0].Attributes = beans::PropertyAttribute::READONLY;

    XPropertySetInfo_impl* p = new XPropertySetInfo_impl( m_pMyShell,
                                                          seq );
    return uno::Reference< beans::XPropertySetInfo > ( p );
}


void SAL_CALL XResultSet_impl::setPropertyValue(
    const OUString& aPropertyName, const uno::Any& )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
    if( aPropertyName == "IsRowCountFinal" ||
        aPropertyName == "RowCount" )
        return;
    throw beans::UnknownPropertyException( THROW_WHERE );
}


uno::Any SAL_CALL XResultSet_impl::getPropertyValue(
    const OUString& PropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
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
        throw beans::UnknownPropertyException( THROW_WHERE );
}


void SAL_CALL XResultSet_impl::addPropertyChangeListener(
    const OUString& aPropertyName,
    const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
    if( aPropertyName == "IsRowCountFinal" )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( ! m_pIsFinalListeners )
            m_pIsFinalListeners =
                new cppu::OInterfaceContainerHelper( m_aEventListenerMutex );

        m_pIsFinalListeners->addInterface( xListener );
    }
    else if ( aPropertyName == "RowCount" )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( ! m_pRowCountListeners )
            m_pRowCountListeners =
                new cppu::OInterfaceContainerHelper( m_aEventListenerMutex );
        m_pRowCountListeners->addInterface( xListener );
    }
    else
        throw beans::UnknownPropertyException( THROW_WHERE );
}


void SAL_CALL XResultSet_impl::removePropertyChangeListener(
    const OUString& aPropertyName,
    const uno::Reference< beans::XPropertyChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
    if( aPropertyName == "IsRowCountFinal" &&
        m_pIsFinalListeners )
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_pIsFinalListeners->removeInterface( aListener );
    }
    else if ( aPropertyName == "RowCount" &&
              m_pRowCountListeners )
    {
        osl::MutexGuard aGuard( m_aMutex );

        m_pRowCountListeners->removeInterface( aListener );
    }
    else
        throw beans::UnknownPropertyException( THROW_WHERE );
}

void SAL_CALL XResultSet_impl::addVetoableChangeListener(
    const OUString&,
    const uno::Reference< beans::XVetoableChangeListener >& )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
}


void SAL_CALL XResultSet_impl::removeVetoableChangeListener(
    const OUString&,
    const uno::Reference< beans::XVetoableChangeListener >& )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
