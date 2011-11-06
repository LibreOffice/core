/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


