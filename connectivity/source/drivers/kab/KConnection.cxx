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


#include "KConnection.hxx"
#include "KDatabaseMetaData.hxx"
#include "KStatement.hxx"
#include "KPreparedStatement.hxx"
#include "KCatalog.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <shell/kde_headers.h>

using namespace connectivity::kab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

IMPLEMENT_SERVICE_INFO(KabConnection, "com.sun.star.sdbc.drivers.KabConnection", "com.sun.star.sdbc.Connection")

KabConnection::KabConnection(
    css::uno::Reference<css::uno::XComponentContext> const & componentContext,
    css::uno::Reference<css::sdbc::XDriver> const & driver)
         : OMetaConnection_BASE(m_aMutex),
         OSubComponent<KabConnection, KabConnection_BASE>(driver, this),
         m_xMetaData(NULL),
         m_pAddressBook(NULL),
         m_xComponentContext(componentContext)
{}

KabConnection::~KabConnection()
{
    if (!isClosed())
        close();
}

void SAL_CALL KabConnection::release() throw()
{
    relase_ChildImpl();
}

//TODO: is doing this after the ctor, and the manual ref counting really
// necessary?
void KabConnection::construct()
{
    osl_atomic_increment( &m_refCount );

    // create a KDE address book object
    m_pAddressBook = KABC::StdAddressBook::self();
    KABC::StdAddressBook::setAutomaticSave(false);

    osl_atomic_decrement( &m_refCount );
}
// XServiceInfo

Reference< XStatement > SAL_CALL KabConnection::createStatement(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    // create a statement
    // the statement can only be executed once
    Reference< XStatement > xReturn = new KabStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL KabConnection::prepareStatement( const OUString& _sSql ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    // create a statement
    // the statement can only be executed more than once
    Reference< XPreparedStatement > xReturn = new KabPreparedStatement(this, _sSql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL KabConnection::prepareCall( const OUString& ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    // not implemented yet :-) a task to do
    return NULL;
}

OUString SAL_CALL KabConnection::nativeSQL( const OUString& _sSql ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // when you need to transform SQL92 to you driver specific you can do it here

    return _sSql;
}

void SAL_CALL KabConnection::setAutoCommit( sal_Bool ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);
    // here you  have to set your commit mode please have a look at the jdbc documentation to get a clear explanation
}

sal_Bool SAL_CALL KabConnection::getAutoCommit(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);
    // you have to distinguish which if you are in autocommit mode or not
    // at normal case true should be fine here

    return sal_True;
}

void SAL_CALL KabConnection::commit(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    // when you database does support transactions you should commit here
}

void SAL_CALL KabConnection::rollback(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    // same as commit but for the other case
}

sal_Bool SAL_CALL KabConnection::isClosed(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // just simple -> we are closed when we are disposed, that means someone called dispose(); (XComponent)
    return KabConnection_BASE::rBHelper.bDisposed;
}

Reference< XDatabaseMetaData > SAL_CALL KabConnection::getMetaData(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    // here we have to create the class with biggest interface
    // The answer is 42 :-)
    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if (!xMetaData.is())
    {
        xMetaData = new KabDatabaseMetaData(this); // need the connection because it can return it
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

void SAL_CALL KabConnection::setReadOnly( sal_Bool ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    // set you connection to readonly
}

sal_Bool SAL_CALL KabConnection::isReadOnly(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    // return if your connection to readonly
    return sal_False;
}

void SAL_CALL KabConnection::setCatalog( const OUString& ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    // if your database doesn't work with catalogs you go to next method otherwise you know what to do
}

OUString SAL_CALL KabConnection::getCatalog(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);


    // return your current catalog
    return OUString();
}

void SAL_CALL KabConnection::setTransactionIsolation( sal_Int32 ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    // set your isolation level
    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
}

sal_Int32 SAL_CALL KabConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);


    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
    return TransactionIsolation::NONE;
}

Reference< ::com::sun::star::container::XNameAccess > SAL_CALL KabConnection::getTypeMap(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    // if your driver has special database types you can return it here

    return NULL;
}

void SAL_CALL KabConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& ) throw(SQLException, RuntimeException, std::exception)
{
    // the other way around
}

// XCloseable
void SAL_CALL KabConnection::close(  ) throw(SQLException, RuntimeException, std::exception)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(KabConnection_BASE::rBHelper.bDisposed);
    }
    dispose();
}

// XWarningsSupplier
Any SAL_CALL KabConnection::getWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
    // when you collected some warnings -> return it
    return Any();
}

void SAL_CALL KabConnection::clearWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
    // you should clear your collected warnings here
}

void KabConnection::disposing()
{
    // we noticed that we should be destroied in near future so we have to dispose our statements
    ::osl::MutexGuard aGuard(m_aMutex);

    for (OWeakRefArray::iterator i = m_aStatements.begin(); m_aStatements.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aStatements.clear();

    if (m_pAddressBook != NULL)
    {
        KABC::StdAddressBook::close();
        m_pAddressBook = NULL;
    }

    m_xMetaData = ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData>();

    dispose_ChildImpl();
    KabConnection_BASE::disposing();
}

Reference< XTablesSupplier > SAL_CALL KabConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XTablesSupplier > xTab = m_xCatalog;
    if (!m_xCatalog.is())
    {
        KabCatalog *pCat = new KabCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}

::KABC::AddressBook* KabConnection::getAddressBook() const
{
    return m_pAddressBook;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
createKabConnection(
    css::uno::Reference<css::uno::XComponentContext> const & componentContext,
    css::uno::Reference<css::sdbc::XDriver> const & driver)
{
    rtl::Reference<KabConnection> con(
        new KabConnection(componentContext, driver));
    con->construct();
    return cppu::acquire(con.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
