/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BResultSet.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:10:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CONNECTIVITY_ADABAS_RESULTSET_HXX
#include "adabas/BResultSet.hxx"
#endif
#ifndef CONNECTIVITY_ADABAS_RESULTSETMETADATA_HXX
#include "adabas/BResultSetMetaData.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif

using namespace connectivity::adabas;
using namespace connectivity::odbc;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

// comment: all this movement methods are needed because adabas doesn't support a SQLGetData call when
// the cursor was moved with a call of SQLFetchScroll. So when this is fixed by adabas we can remove this damn thing.


sal_Bool SAL_CALL OAdabasResultSet::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    //  m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_NEXT,0);
    m_nCurrentFetchState = N3SQLFetch(m_aStatementHandle);

    if(m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO)
        ++m_nRowPos;

    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    return m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OAdabasResultSet::first(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    // don't ask why !
    N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_OFF,SQL_IS_UINTEGER);
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_FIRST,0);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
    {
        m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_PRIOR,0);
        N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_ON,SQL_IS_UINTEGER);
        m_nCurrentFetchState = N3SQLFetch(m_aStatementHandle);
    }

    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    bRet = ( m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO );
    if ( bRet )
        m_nRowPos = 1;
    return bRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OAdabasResultSet::last(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_OFF,SQL_IS_UINTEGER);
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_LAST,0);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
    {
        m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_PRIOR,0);
        N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_ON,SQL_IS_UINTEGER);
        m_nCurrentFetchState = N3SQLFetch(m_aStatementHandle);
    }

    m_bEOF = sal_True;
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    // here I know definitely that I stand on the last record
    return m_bLastRecord = (m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OAdabasResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_OFF,SQL_IS_UINTEGER);

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_ABSOLUTE,row);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
    {
        m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_PRIOR,0);
        N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_ON,SQL_IS_UINTEGER);
        m_nCurrentFetchState = N3SQLFetch(m_aStatementHandle);
    }

    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    if(bRet)
        m_nRowPos = row;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OAdabasResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_OFF,SQL_IS_UINTEGER);
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_RELATIVE,row);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
    {
        m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_PRIOR,0);
        N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_ON,SQL_IS_UINTEGER);
        m_nCurrentFetchState = N3SQLFetch(m_aStatementHandle);
    }

    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    if(bRet)
        m_nRowPos += row;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OAdabasResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_OFF,SQL_IS_UINTEGER);
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_PRIOR,0);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
    {
        m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_PRIOR,0);
        N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_ON,SQL_IS_UINTEGER);
        m_nCurrentFetchState = N3SQLFetch(m_aStatementHandle);
    }

    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    if(bRet || m_nCurrentFetchState == SQL_NO_DATA)
        --m_nRowPos;
    return bRet;
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdabasResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    //  SQLRETURN nRet = N3SQLSetPos(m_aStatementHandle,1,SQL_REFRESH,SQL_LOCK_NO_CHANGE);
    N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_OFF,SQL_IS_UINTEGER);
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_RELATIVE,0);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
    {
        m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_PRIOR,0);
        N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_RETRIEVE_DATA,(SQLPOINTER)SQL_RD_ON,SQL_IS_UINTEGER);
        m_nCurrentFetchState = N3SQLFetch(m_aStatementHandle);
    }
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -----------------------------------------------------------------------------
Reference< XResultSetMetaData > SAL_CALL OAdabasResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OAdabasResultSetMetaData(m_pStatement->getOwnConnection(),m_aStatementHandle,m_aSelectColumns);
    return m_xMetaData;
}
// -----------------------------------------------------------------------------






