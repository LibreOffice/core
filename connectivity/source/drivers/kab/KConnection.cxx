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


#include "KConnection.hxx"
#include "KDatabaseMetaData.hxx"
#include "KStatement.hxx"
#include "KPreparedStatement.hxx"
#include "KDriver.hxx"
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

KabConnection::KabConnection(KabDriver* _pDriver)
         : OMetaConnection_BASE(m_aMutex),
         OSubComponent<KabConnection, KabConnection_BASE>((::cppu::OWeakObject*)_pDriver, this),
         m_xMetaData(NULL),
         m_pAddressBook(NULL),
         m_pDriver(_pDriver)
{
    m_pDriver->acquire();
}

KabConnection::~KabConnection()
{
    if (!isClosed())
        close();

    m_pDriver->release();
    m_pDriver = NULL;
}

void SAL_CALL KabConnection::release() throw()
{
    relase_ChildImpl();
}

void KabConnection::construct(const OUString&, const Sequence< PropertyValue >&) throw(SQLException)
{
    osl_atomic_increment( &m_refCount );

    
    m_pAddressBook = KABC::StdAddressBook::self();
    m_pAddressBook->setAutomaticSave(false);



    osl_atomic_decrement( &m_refCount );
}


Reference< XStatement > SAL_CALL KabConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    
    
    Reference< XStatement > xReturn = new KabStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL KabConnection::prepareStatement( const OUString& _sSql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    
    
    Reference< XPreparedStatement > xReturn = new KabPreparedStatement(this, _sSql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL KabConnection::prepareCall( const OUString& ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    
    return NULL;
}

OUString SAL_CALL KabConnection::nativeSQL( const OUString& _sSql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    

    return _sSql;
}

void SAL_CALL KabConnection::setAutoCommit( sal_Bool ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);
    
}

sal_Bool SAL_CALL KabConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);
    
    

    return sal_True;
}

void SAL_CALL KabConnection::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    
}

void SAL_CALL KabConnection::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    
}

sal_Bool SAL_CALL KabConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    
    return KabConnection_BASE::rBHelper.bDisposed;
}

Reference< XDatabaseMetaData > SAL_CALL KabConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    
    
    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if (!xMetaData.is())
    {
        xMetaData = new KabDatabaseMetaData(this); 
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

void SAL_CALL KabConnection::setReadOnly( sal_Bool ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    
}

sal_Bool SAL_CALL KabConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    
    return sal_False;
}

void SAL_CALL KabConnection::setCatalog( const OUString& ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    
}

OUString SAL_CALL KabConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);


    
    return OUString();
}

void SAL_CALL KabConnection::setTransactionIsolation( sal_Int32 ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    
    
}

sal_Int32 SAL_CALL KabConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);


    
    return TransactionIsolation::NONE;
}

Reference< ::com::sun::star::container::XNameAccess > SAL_CALL KabConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabConnection_BASE::rBHelper.bDisposed);

    

    return NULL;
}

void SAL_CALL KabConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& ) throw(SQLException, RuntimeException)
{
    
}


void SAL_CALL KabConnection::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(KabConnection_BASE::rBHelper.bDisposed);
    }
    dispose();
}


Any SAL_CALL KabConnection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    
    return Any();
}

void SAL_CALL KabConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    
}

void KabConnection::disposing()
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
        m_pAddressBook->close();
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

extern "C" SAL_DLLPUBLIC_EXPORT void*  SAL_CALL createKabConnection( void* _pDriver )
{
    KabConnection* pConnection = new KabConnection( static_cast< KabDriver* >( _pDriver ) );
    
    pConnection->acquire();
    return pConnection;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
