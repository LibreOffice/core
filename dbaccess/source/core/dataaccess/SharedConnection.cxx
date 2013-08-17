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

#include "SharedConnection.hxx"
#include <tools/debug.hxx>


namespace dbaccess
{
    using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace connectivity;

DBG_NAME(OSharedConnection)
OSharedConnection::OSharedConnection(Reference< XAggregation >& _rxProxyConnection)
            : OSharedConnection_BASE(m_aMutex)
{
    DBG_CTOR(OSharedConnection,NULL);
    setDelegation(_rxProxyConnection,m_refCount);
}

OSharedConnection::~OSharedConnection()
{
    DBG_DTOR(OSharedConnection,NULL);
}

void SAL_CALL OSharedConnection::disposing(void)
{
    OSharedConnection_BASE::disposing();
    OConnectionWrapper::disposing();
}

Reference< XStatement > SAL_CALL OSharedConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    return m_xConnection->createStatement();
}

Reference< XPreparedStatement > SAL_CALL OSharedConnection::prepareStatement( const OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    return m_xConnection->prepareStatement(sql);
}

Reference< XPreparedStatement > SAL_CALL OSharedConnection::prepareCall( const OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    return m_xConnection->prepareCall(sql);
}

OUString SAL_CALL OSharedConnection::nativeSQL( const OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    return m_xConnection->nativeSQL(sql);
}

sal_Bool SAL_CALL OSharedConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    return m_xConnection->getAutoCommit();
}

void SAL_CALL OSharedConnection::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    m_xConnection->commit();
}

void SAL_CALL OSharedConnection::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    m_xConnection->rollback();
}

sal_Bool SAL_CALL OSharedConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xConnection.is() )
        return sal_True;

    return m_xConnection->isClosed();
}

Reference< XDatabaseMetaData > SAL_CALL OSharedConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    return m_xConnection->getMetaData();
}

sal_Bool SAL_CALL OSharedConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    return m_xConnection->isReadOnly();
}

OUString SAL_CALL OSharedConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    return m_xConnection->getCatalog();
}

sal_Int32 SAL_CALL OSharedConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    return m_xConnection->getTransactionIsolation();
}

Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OSharedConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    return m_xConnection->getTypeMap();
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
