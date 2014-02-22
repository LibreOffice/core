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


#include "MacabConnection.hxx"
#include "MacabAddressBook.hxx"
#include "MacabDatabaseMetaData.hxx"
#include "MacabStatement.hxx"
#include "MacabPreparedStatement.hxx"
#include "MacabDriver.hxx"
#include "MacabCatalog.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>

using namespace connectivity::macab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

IMPLEMENT_SERVICE_INFO(MacabConnection, "com.sun.star.sdbc.drivers.MacabConnection", "com.sun.star.sdbc.Connection")

MacabConnection::MacabConnection(MacabDriver*   _pDriver)
         : OSubComponent<MacabConnection, MacabConnection_BASE>((::cppu::OWeakObject*)_pDriver, this),
         m_pAddressBook(NULL),
         m_pDriver(_pDriver)
{
    m_pDriver->acquire();
}

MacabConnection::~MacabConnection()
{
    if (!isClosed())
        close();

    m_pDriver->release();
    m_pDriver = NULL;
}

void SAL_CALL MacabConnection::release() throw()
{
    relase_ChildImpl();
}

void MacabConnection::construct(const OUString&, const Sequence< PropertyValue >&) throw(SQLException)
{
    osl_atomic_increment( &m_refCount );

    
    m_pAddressBook = new MacabAddressBook();

    osl_atomic_decrement( &m_refCount );
}


Reference< XStatement > SAL_CALL MacabConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    
    
    Reference< XStatement > xReturn = new MacabStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL MacabConnection::prepareStatement( const OUString& _sSql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    
    
    Reference< XPreparedStatement > xReturn = new MacabPreparedStatement(this, _sSql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL MacabConnection::prepareCall( const OUString& ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    
    return NULL;
}

OUString SAL_CALL MacabConnection::nativeSQL( const OUString& _sSql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    

    return _sSql;
}

void SAL_CALL MacabConnection::setAutoCommit( sal_Bool ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);
    
}

sal_Bool SAL_CALL MacabConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);
    
    

    return sal_True;
}

void SAL_CALL MacabConnection::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    
}

void SAL_CALL MacabConnection::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    
}

sal_Bool SAL_CALL MacabConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    
    return MacabConnection_BASE::rBHelper.bDisposed;
}

Reference< XDatabaseMetaData > SAL_CALL MacabConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    
    
    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if (!xMetaData.is())
    {
        xMetaData = new MacabDatabaseMetaData(this); 
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

void SAL_CALL MacabConnection::setReadOnly( sal_Bool ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    
}

sal_Bool SAL_CALL MacabConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    
    return sal_False;
}

void SAL_CALL MacabConnection::setCatalog( const OUString& ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    
}

OUString SAL_CALL MacabConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);


    
    return OUString();
}

void SAL_CALL MacabConnection::setTransactionIsolation( sal_Int32 ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    
    
}

sal_Int32 SAL_CALL MacabConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);


    
    return TransactionIsolation::NONE;
}

Reference< ::com::sun::star::container::XNameAccess > SAL_CALL MacabConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    

    return NULL;
}

void SAL_CALL MacabConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& ) throw(SQLException, RuntimeException)
{
    
}


void SAL_CALL MacabConnection::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);
    }
    dispose();
}


Any SAL_CALL MacabConnection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    
    return Any();
}

void SAL_CALL MacabConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    
}

void MacabConnection::disposing()
{
    
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
        delete m_pAddressBook;
        m_pAddressBook = NULL;
    }

    m_xMetaData = ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData>();

    dispose_ChildImpl();
    MacabConnection_BASE::disposing();
}

Reference< XTablesSupplier > SAL_CALL MacabConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XTablesSupplier > xTab = m_xCatalog;
    if (!m_xCatalog.is())
    {
        MacabCatalog *pCat = new MacabCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}

MacabAddressBook* MacabConnection::getAddressBook() const
{
    return m_pAddressBook;
}

extern "C" SAL_DLLPUBLIC_EXPORT void*  SAL_CALL createMacabConnection( void* _pDriver )
{
    MacabConnection* pConnection = new MacabConnection( static_cast< MacabDriver* >( _pDriver ) );
    
    pConnection->acquire();
    return pConnection;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
