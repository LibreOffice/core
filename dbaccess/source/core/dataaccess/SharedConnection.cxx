/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBA_CORE_SHARED_CONNECTION_HXX
#include "SharedConnection.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


namespace dbaccess
{
    using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
//  using namespace ::com::sun::star::reflection;
using namespace connectivity;

DBG_NAME(OSharedConnection)
OSharedConnection::OSharedConnection(Reference< XAggregation >& _rxProxyConnection)
            : OSharedConnection_BASE(m_aMutex)
{
    DBG_CTOR(OSharedConnection,NULL);
    setDelegation(_rxProxyConnection,m_refCount);
}
// -----------------------------------------------------------------------------
OSharedConnection::~OSharedConnection()
{
    DBG_DTOR(OSharedConnection,NULL);
}
// -----------------------------------------------------------------------------
void SAL_CALL OSharedConnection::disposing(void)
{
    OSharedConnection_BASE::disposing();
    OConnectionWrapper::disposing();
}
// -----------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OSharedConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    return m_xConnection->createStatement();
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OSharedConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    return m_xConnection->prepareStatement(sql);
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OSharedConnection::prepareCall( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    return m_xConnection->prepareCall(sql);
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSharedConnection::nativeSQL( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    return m_xConnection->nativeSQL(sql);
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OSharedConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    return m_xConnection->getAutoCommit();
}
// --------------------------------------------------------------------------------
void SAL_CALL OSharedConnection::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    m_xConnection->commit();
}
// --------------------------------------------------------------------------------
void SAL_CALL OSharedConnection::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    m_xConnection->rollback();
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OSharedConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xConnection.is() )
        return sal_True;

    return m_xConnection->isClosed();
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OSharedConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    return m_xConnection->getMetaData();
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OSharedConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    return m_xConnection->isReadOnly();
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSharedConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    return m_xConnection->getCatalog();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OSharedConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    return m_xConnection->getTransactionIsolation();
}
// --------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OSharedConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);


    return m_xConnection->getTypeMap();
}
// -----------------------------------------------------------------------------

//........................................................................
}   // namespace dbaccess
//........................................................................


