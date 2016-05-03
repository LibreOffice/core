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

#include <comphelper/processfactory.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#include <ucbhelper/resultsetmetadata.hxx>
#include "ftpresultsetbase.hxx"

using namespace ftp;
using namespace com::sun::star;

ResultSetBase::ResultSetBase(
    const uno::Reference< uno::XComponentContext >&  rxContext,
    const uno::Reference< ucb::XContentProvider >&  xProvider,
    const uno::Sequence< beans::Property >& seq,
    const uno::Sequence< ucb::NumberedSortingInfo >& seqSort )
    : m_xContext( rxContext ),
      m_xProvider( xProvider ),
      m_nRow( -1 ),
      m_nWasNull( true ),
      m_bRowCountFinal( true ),
      m_sProperty( seq ),
      m_sSortingInfo( seqSort ),
      m_pDisposeEventListeners( nullptr ),
      m_pRowCountListeners( nullptr ),
      m_pIsFinalListeners( nullptr )
{
}

ResultSetBase::~ResultSetBase()
{
    delete m_pIsFinalListeners;
    delete m_pRowCountListeners;
    delete m_pDisposeEventListeners;
}


// XInterface

void SAL_CALL
ResultSetBase::acquire(
    void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
ResultSetBase::release(
    void )
    throw()
{
    OWeakObject::release();
}


uno::Any SAL_CALL
ResultSetBase::queryInterface(
    const uno::Type& rType )
    throw( uno::RuntimeException, std::exception )
{
    uno::Any aRet = cppu::queryInterface(
        rType,
        (static_cast< lang::XComponent* >(this)),
        (static_cast< sdbc::XRow* >(this)),
        (static_cast< sdbc::XResultSet* >(this)),
        (static_cast< sdbc::XResultSetMetaDataSupplier* >(this)),
        (static_cast< beans::XPropertySet* >(this)),
        (static_cast< ucb::XContentAccess* >(this)) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


// XComponent


void SAL_CALL
ResultSetBase::addEventListener(
    const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( ! m_pDisposeEventListeners )
        m_pDisposeEventListeners =
            new cppu::OInterfaceContainerHelper( m_aMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL
ResultSetBase::removeEventListener(
    const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


void SAL_CALL
ResultSetBase::dispose()
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


//  XResultSet

sal_Bool SAL_CALL
ResultSetBase::next(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    bool test;
    if( ++m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
        test = true;
    else
        test = false;
    return test;
}


sal_Bool SAL_CALL
ResultSetBase::isBeforeFirst(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return m_nRow == -1;
}


sal_Bool SAL_CALL
ResultSetBase::isAfterLast(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return m_nRow >= sal::static_int_cast<sal_Int32>(m_aItems.size());   // Cannot happen, if m_aFolder.isOpen()
}


sal_Bool SAL_CALL
ResultSetBase::isFirst(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return m_nRow == 0;
}


sal_Bool SAL_CALL
ResultSetBase::isLast(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    if( m_nRow ==  sal::static_int_cast<sal_Int32>(m_aItems.size()) - 1 )
        return true;
    else
        return false;
}


void SAL_CALL
ResultSetBase::beforeFirst(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    m_nRow = -1;
}


void SAL_CALL
ResultSetBase::afterLast(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    m_nRow = m_aItems.size();
}


sal_Bool SAL_CALL
ResultSetBase::first(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    m_nRow = -1;
    return next();
}


sal_Bool SAL_CALL
ResultSetBase::last(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    m_nRow = m_aItems.size() - 1;
    return true;
}


sal_Int32 SAL_CALL
ResultSetBase::getRow(
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


sal_Bool SAL_CALL ResultSetBase::absolute( sal_Int32 row )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception)
{
    if( row >= 0 )
        m_nRow = row - 1;
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
ResultSetBase::relative(
    sal_Int32 row )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    if( isAfterLast() || isBeforeFirst() )
        throw sdbc::SQLException();

    if( row > 0 )
        while( row-- )
            next();
    else if( row < 0 )
        while( row++ && m_nRow > - 1 )
            previous();

    return 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size());
}


sal_Bool SAL_CALL
ResultSetBase::previous(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
    if( m_nRow > sal::static_int_cast<sal_Int32>(m_aItems.size()) )
        m_nRow = m_aItems.size();  // Correct Handling of afterLast
    if( 0 <= m_nRow ) -- m_nRow;

    return 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size());
}


void SAL_CALL
ResultSetBase::refreshRow(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
}


sal_Bool SAL_CALL
ResultSetBase::rowUpdated(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return false;
}

sal_Bool SAL_CALL
ResultSetBase::rowInserted(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return false;
}

sal_Bool SAL_CALL
ResultSetBase::rowDeleted(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    return false;
}


uno::Reference< uno::XInterface > SAL_CALL
ResultSetBase::getStatement(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    uno::Reference< uno::XInterface > test( nullptr );
    return test;
}


// XCloseable

void SAL_CALL
ResultSetBase::close(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception)
{
}


OUString SAL_CALL
ResultSetBase::queryContentIdentifierString(
    void )
    throw( uno::RuntimeException, std::exception )
{
    if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
        return m_aPath[m_nRow];
    else
        return OUString();
}


uno::Reference< ucb::XContentIdentifier > SAL_CALL
ResultSetBase::queryContentIdentifier(
    void
)
    throw(
        uno::RuntimeException, std::exception
    )
{
    if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
    {
        if(!m_aIdents[m_nRow].is()) {
            OUString url = queryContentIdentifierString();
            if(!url.isEmpty() )
                m_aIdents[m_nRow] =
                    uno::Reference< ucb::XContentIdentifier >(
                        new ::ucbhelper::ContentIdentifier(url) );
        }
        return m_aIdents[m_nRow];
    }

    return uno::Reference<ucb::XContentIdentifier>();
}


uno::Reference< ucb::XContent > SAL_CALL
ResultSetBase::queryContent(
    void )
    throw( uno::RuntimeException, std::exception )
{
    if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
        return m_xProvider->queryContent(queryContentIdentifier());
    else
        return uno::Reference< ucb::XContent >();
}


class XPropertySetInfoImpl
    : public cppu::OWeakObject,
      public beans::XPropertySetInfo
{
public:

    explicit XPropertySetInfoImpl( const uno::Sequence< beans::Property >& aSeq )
        : m_aSeq( aSeq )
    {
    }

    void SAL_CALL acquire()
        throw() override
    {
        OWeakObject::acquire();
    }


    void SAL_CALL release()
        throw() override
    {
        OWeakObject::release();
    }

    uno::Any SAL_CALL queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException, std::exception ) override
    {
        uno::Any aRet = cppu::queryInterface(
            rType,
            (static_cast< beans::XPropertySetInfo* >(this)) );
        return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
    }

    uno::Sequence< beans::Property > SAL_CALL getProperties()
        throw( uno::RuntimeException, std::exception ) override
    {
        return m_aSeq;
    }

    beans::Property SAL_CALL getPropertyByName( const OUString& aName )
        throw( beans::UnknownPropertyException,
               uno::RuntimeException, std::exception) override
    {
        for( int i = 0; i < m_aSeq.getLength(); ++i )
            if( aName == m_aSeq[i].Name )
                return m_aSeq[i];
        throw beans::UnknownPropertyException();
    }

    sal_Bool SAL_CALL hasPropertyByName( const OUString& Name )
        throw( uno::RuntimeException, std::exception ) override
    {
        for( int i = 0; i < m_aSeq.getLength(); ++i )
            if( Name == m_aSeq[i].Name )
                return true;
        return false;
    }

private:

    uno::Sequence< beans::Property > m_aSeq;
};


// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL
ResultSetBase::getPropertySetInfo()
    throw( uno::RuntimeException, std::exception)
{
    uno::Sequence< beans::Property > seq(2);
    seq[0].Name = "RowCount";
    seq[0].Handle = -1;
    seq[0].Type = cppu::UnoType<sal_Int32>::get();
    seq[0].Attributes = beans::PropertyAttribute::READONLY;

    seq[1].Name = "IsRowCountFinal";
    seq[1].Handle = -1;
    seq[1].Type = cppu::UnoType<sal_Bool>::get();
    seq[1].Attributes = beans::PropertyAttribute::READONLY;

    //t
    return uno::Reference< beans::XPropertySetInfo > (
        new XPropertySetInfoImpl( seq ) );
}


void SAL_CALL ResultSetBase::setPropertyValue(
    const OUString& aPropertyName, const uno::Any& /*aValue*/ )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
    if( aPropertyName == "IsRowCountFinal" ||
        aPropertyName == "RowCount" )
        return;

    throw beans::UnknownPropertyException();
}


uno::Any SAL_CALL ResultSetBase::getPropertyValue(
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
        sal_Int32 count = m_aItems.size();
        return uno::Any(count);
    }
    else
        throw beans::UnknownPropertyException();
}


void SAL_CALL ResultSetBase::addPropertyChangeListener(
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
                new cppu::OInterfaceContainerHelper( m_aMutex );

        m_pIsFinalListeners->addInterface( xListener );
    }
    else if ( aPropertyName == "RowCount" )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( ! m_pRowCountListeners )
            m_pRowCountListeners =
                new cppu::OInterfaceContainerHelper( m_aMutex );
        m_pRowCountListeners->addInterface( xListener );
    }
    else
        throw beans::UnknownPropertyException();
}


void SAL_CALL ResultSetBase::removePropertyChangeListener(
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
        throw beans::UnknownPropertyException();
}


void SAL_CALL ResultSetBase::addVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
}


void SAL_CALL ResultSetBase::removeVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
}


// XResultSetMetaDataSupplier
uno::Reference< sdbc::XResultSetMetaData > SAL_CALL
ResultSetBase::getMetaData(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException, std::exception )
{
    ::ucbhelper::ResultSetMetaData* p =
          new ::ucbhelper::ResultSetMetaData( m_xContext, m_sProperty );
    return uno::Reference< sdbc::XResultSetMetaData >( p );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
