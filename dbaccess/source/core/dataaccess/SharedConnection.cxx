/*************************************************************************
 *
 *  $RCSfile: SharedConnection.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:06:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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


