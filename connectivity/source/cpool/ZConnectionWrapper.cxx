/*************************************************************************
 *
 *  $RCSfile: ZConnectionWrapper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:54:00 $
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
#ifndef _CONNECTIVITY_ZCONNECTIONWEAKWRAPPER_HXX_
#include "ZConnectionWrapper.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

using namespace connectivity;
//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
// --------------------------------------------------------------------------------
OConnectionWeakWrapper::OConnectionWeakWrapper(Reference< XAggregation >& _xConnection)
    : OConnectionWeakWrapper_BASE(m_aMutex)
{
    setDelegation(_xConnection,m_refCount);
    OSL_ENSURE(m_xConnection.is(),"OConnectionWeakWrapper: Connection must be valid!");
}
//-----------------------------------------------------------------------------
OConnectionWeakWrapper::~OConnectionWeakWrapper()
{
}
// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OConnectionWeakWrapper, "com.sun.star.sdbc.drivers.OConnectionWeakWrapper", "com.sun.star.sdbc.Connection")

// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OConnectionWeakWrapper::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->createStatement();
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnectionWeakWrapper::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->prepareStatement(sql);
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnectionWeakWrapper::prepareCall( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->prepareCall(sql);
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnectionWeakWrapper::nativeSQL( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->nativeSQL(sql);
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);

    m_xConnection->setAutoCommit(autoCommit);
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnectionWeakWrapper::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->getAutoCommit();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->commit();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->rollback();
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnectionWeakWrapper::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_xConnection->isClosed();
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OConnectionWeakWrapper::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->getMetaData();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->setReadOnly(readOnly);
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnectionWeakWrapper::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->isReadOnly();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::setCatalog( const ::rtl::OUString& catalog ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->setCatalog(catalog);
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnectionWeakWrapper::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->getCatalog();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->setTransactionIsolation(level);
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OConnectionWeakWrapper::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->getTransactionIsolation();
}
// --------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OConnectionWeakWrapper::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->getTypeMap();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->setTypeMap(typeMap);
}
// --------------------------------------------------------------------------------
// XCloseable
void SAL_CALL OConnectionWeakWrapper::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);

    }
    dispose();
}
//------------------------------------------------------------------------------
void OConnectionWeakWrapper::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OConnectionWeakWrapper_BASE::disposing();
    OConnectionWrapper::disposing();
}
// -----------------------------------------------------------------------------
// com::sun::star::lang::XUnoTunnel
IMPLEMENT_FORWARD_XINTERFACE2(OConnectionWeakWrapper,OConnectionWeakWrapper_BASE,OConnectionWrapper)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(OConnectionWeakWrapper,OConnectionWeakWrapper_BASE,OConnectionWrapper)


