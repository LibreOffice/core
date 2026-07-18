/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include "MacabConnection.hxx"
#include "MacabAddressBook.hxx"
#include "MacabDatabaseMetaData.hxx"
#include "MacabStatement.hxx"
#include "MacabPreparedStatement.hxx"
#include "MacabDriver.hxx"
#include "MacabCatalog.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <cppuhelper/weak.hxx>

using namespace connectivity::macab;
using namespace com::sun::star::uno;
using namespace cpo::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

IMPLEMENT_SERVICE_INFO(MacabConnection, "com.sun.star.sdbc.drivers.MacabConnection", "com.sun.star.sdbc.Connection")

MacabConnection::MacabConnection(MacabDriver*   _pDriver)
         : m_pAddressBook(nullptr),
         m_pDriver(_pDriver)
{
    m_pDriver->acquire();
}

MacabConnection::~MacabConnection()
{
    if (!doIsClosed())
        doClose();

    m_pDriver->release();
    m_pDriver = nullptr;
}

void MacabConnection::construct(const OUString&, const Sequence< PropertyValue >&)
{
    osl_atomic_increment( &m_refCount );

    // get the macOS shared address book
    m_pAddressBook = new MacabAddressBook();

    osl_atomic_decrement( &m_refCount );
}
// XServiceInfo

Reference< XStatement > MacabConnection::createStatement(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // create a statement
    // the statement can only be executed once
    Reference< XStatement > xReturn = new MacabStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > MacabConnection::prepareStatement( const OUString& _sSql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // create a statement
    // the statement can only be executed more than once
    Reference< XPreparedStatement > xReturn = new MacabPreparedStatement(this, _sSql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > MacabConnection::prepareCall( const OUString& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // not implemented yet :-) a task to do
    return nullptr;
}

OUString MacabConnection::nativeSQL( const OUString& _sSql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // when you need to transform SQL92 to you driver specific you can do it here

    return _sSql;
}

void MacabConnection::setAutoCommit( bool )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);
    // here you  have to set your commit mode please have a look at the jdbc documentation to get a clear explanation
}

bool MacabConnection::getAutoCommit(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);
    // you have to distinguish which if you are in autocommit mode or not
    // at normal case true should be fine here

    return true;
}

void MacabConnection::commit(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // when you database does support transactions you should commit here
}

void MacabConnection::rollback(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // same as commit but for the other case
}

bool MacabConnection::isClosed(  )
{
    return doIsClosed();
}

bool MacabConnection::doIsClosed()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // just simple -> we are closed when we are disposed, that means someone called dispose(); (XComponent)
    return MacabConnection_BASE::rBHelper.bDisposed;
}

Reference< XDatabaseMetaData > MacabConnection::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // here we have to create the class with biggest interface
    // The answer is 42 :-)
    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if (!xMetaData.is())
    {
        xMetaData = new MacabDatabaseMetaData(this); // need the connection because it can return it
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

void MacabConnection::setReadOnly( bool )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // set you connection to readonly
}

bool MacabConnection::isReadOnly(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // return if your connection to readonly
    return false;
}

void MacabConnection::setCatalog( const OUString& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // if your database doesn't work with catalogs you go to next method otherwise you know what to do
}

OUString MacabConnection::getCatalog(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);


    // return your current catalog
    return OUString();
}

void MacabConnection::setTransactionIsolation( sal_Int32 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // set your isolation level
    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
}

sal_Int32 MacabConnection::getTransactionIsolation(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);


    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
    return TransactionIsolation::NONE;
}

Reference< css::container::XNameAccess > MacabConnection::getTypeMap(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // if your driver has special database types you can return it here

    return nullptr;
}

void MacabConnection::setTypeMap( const Reference< css::container::XNameAccess >& )
{
    // the other way around
}

// XCloseable
void MacabConnection::close(  )
{
    doClose();
}

void MacabConnection::doClose()
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);
    }
    dispose();
}

// XWarningsSupplier
Any MacabConnection::getWarnings(  )
{
    // when you collected some warnings -> return it
    return Any();
}

void MacabConnection::clearWarnings(  )
{
    // you should clear your collected warnings here
}

void MacabConnection::disposing()
{
    // we noticed that we should be destroyed in near future so we have to dispose our statements
    ::osl::MutexGuard aGuard(m_aMutex);

    for (auto& rxStatement : m_aStatements)
    {
        Reference< XComponent > xComp(rxStatement.get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aStatements.clear();

    if (m_pAddressBook != nullptr)
    {
        delete m_pAddressBook;
        m_pAddressBook = nullptr;
    }

    m_xMetaData.clear();

    MacabConnection_BASE::disposing();
}

Reference< XTablesSupplier > MacabConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XTablesSupplier > xTab = m_xCatalog;
    if (!m_xCatalog.is())
    {
        xTab = new MacabCatalog(this);
        m_xCatalog = xTab;
    }
    return xTab;
}

MacabAddressBook* MacabConnection::getAddressBook() const
{
    return m_pAddressBook;
}

extern "C" SAL_DLLPUBLIC_EXPORT void* createMacabConnection( void* _pDriver )
{
    // by definition, the pointer crossing library boundaries as void ptr is acquired once
    return cppu::acquire(new MacabConnection( static_cast< MacabDriver* >( _pDriver ) ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
