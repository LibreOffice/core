/*************************************************************************
 *
 *  $RCSfile: OStatement.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:23 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONNECTIVITY_ODBC_OSTATEMENT_HXX_
#include "odbc/OStatement.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OCONNECTION_HXX_
#include "odbc/OConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_ORESULTSET_HXX_
#include "odbc/OResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _UTL_PROPERTY_HXX_
#include <unotools/property.hxx>
#endif
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif
#ifndef _UTL_UNO3_HXX_
#include <unotools/uno3.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

#define THROW_SQL(x) \
    OTools::ThrowException(x,m_aStatementHandle,SQL_HANDLE_STMT,*this)

#ifdef DEBUG
#define DEBUG_THROW                 \
    try                                 \
    {                                   \
        THROW_SQL(nRetCode);            \
    }                                   \
    catch(...)                          \
    {                                   \
    }
#endif

using namespace connectivity::odbc;
//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
//------------------------------------------------------------------------------
OStatement_Base::OStatement_Base(OConnection* _pConnection ) :  OStatement_BASE(m_aMutex),
                                                        OPropertySetHelper(OStatement_BASE::rBHelper),
    rBHelper(OStatement_BASE::rBHelper),
    m_pConnection(_pConnection),
    m_pRowStatusArray(0)
{
    m_pConnection->acquire();
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&m_aStatementHandle);
}
//------------------------------------------------------------------------------
void OStatement_Base::disposeResultSet()
{
    // free the cursor if alive
    Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_xResultSet = Reference< XResultSet>();
}
//------------------------------------------------------------------------------
void OStatement_BASE2::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    disposeResultSet();

    if (N3SQLFreeStmt(m_aStatementHandle,SQL_RESET_PARAMS) ||
        N3SQLFreeStmt(m_aStatementHandle,SQL_UNBIND) ||
        N3SQLFreeStmt(m_aStatementHandle,SQL_CLOSE))
        OTools::ThrowException(N3SQLFreeStmt(m_aStatementHandle,SQL_DROP),m_aStatementHandle,SQL_HANDLE_STMT,*this);

    if (m_pConnection)
        m_pConnection->release();

    dispose_ChildImpl();
    OStatement_Base::disposing();
}
//-----------------------------------------------------------------------------
void SAL_CALL OStatement_BASE2::release() throw(RuntimeException)
{
    relase_ChildImpl();
}
//-----------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OStatement_Base::getTypes(  ) throw(RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XPropertySet > *)0 ));

    return ::utl::concatSequences(aTypes.getTypes(),OStatement_BASE::getTypes());
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::cancel(  ) throw(RuntimeException)
{
    OTools::ThrowException(N3SQLCancel(m_aStatementHandle),m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (OStatement_BASE::rBHelper.bDisposed)
            throw DisposedException();
    }
    dispose();
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement::clearBatch(  ) throw(SQLException, RuntimeException)
{

}
// -------------------------------------------------------------------------

void OStatement_Base::reset() throw (SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    clearWarnings ();

    if (m_xResultSet.get().is())
    {
        clearMyResultSet();
        THROW_SQL(N3SQLFreeStmt(m_aStatementHandle, SQL_CLOSE));
    }
}
//--------------------------------------------------------------------
// clearMyResultSet
// If a ResultSet was created for this Statement, close it
//--------------------------------------------------------------------

void OStatement_Base::clearMyResultSet () throw (SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference<XCloseable> xCloseable;
    if(::utl::query_interface(m_xResultSet.get(),xCloseable))
        xCloseable->close();
    m_xResultSet = Reference< XResultSet>();
}
//--------------------------------------------------------------------
sal_Int32 OStatement_Base::getRowCount () throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32   numRows = 0;

    try {
        THROW_SQL(N3SQLRowCount(m_aStatementHandle,&numRows));
    }
    catch (...)
    {
    }
    return numRows;
}
//--------------------------------------------------------------------
// lockIfNecessary
// If the given SQL statement contains a 'FOR UPDATE' clause, change
// the concurrency to lock so that the row can then be updated.  Returns
// true if the concurrency has been changed
//--------------------------------------------------------------------

sal_Bool OStatement_Base::lockIfNecessary (const ::rtl::OUString& sql) throw( SQLException)
{
    sal_Bool rc = sal_False;

    // First, convert the statement to upper case

    ::rtl::OUString sqlStatement = sql.toUpperCase ();

    // Now, look for the FOR UPDATE keywords.  If there is any extra white
    // space between the FOR and UPDATE, this will fail.

    sal_Int32 index = sqlStatement.indexOf(::rtl::OUString::createFromAscii(" FOR UPDATE"));

    // We found it.  Change our concurrency level to ensure that the
    // row can be updated.

    if (index > 0)
    {
        try
        {
            SQLINTEGER nLock = SQL_CONCUR_LOCK;
            THROW_SQL(N3SQLSetStmtAttr(m_aStatementHandle, SQL_CONCURRENCY,(SQLPOINTER)nLock,SQL_IS_UINTEGER));
        }
        catch (SQLWarning& warn)
        {
            // Catch any warnings and place on the warning stack
            setWarning (warn);
        }
        rc = sal_True;
    }

    return rc;
}
//--------------------------------------------------------------------
// setWarning
// Sets the warning
//--------------------------------------------------------------------

void OStatement_Base::setWarning (const SQLWarning &ex) throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_aLastWarning = ex;
}

//--------------------------------------------------------------------
// getColumnCount
// Return the number of columns in the ResultSet
//--------------------------------------------------------------------

sal_Int32 OStatement_Base::getColumnCount () throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int16   numCols = 0;

    try {
        THROW_SQL(N3SQLNumResultCols(m_aStatementHandle,&numCols));
    }
    catch (...)
    {
    }
    return numCols;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OStatement_Base::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ::rtl::OString aSql(::rtl::OUStringToOString(sql,
                        osl_getThreadTextEncoding()
        ));

    sal_Bool hasResultSet = sal_False;
    SQLWarning aWarning;

    // Reset the statement handle and warning

    reset();

    // Check for a 'FOR UPDATE' statement.  If present, change
    // the concurrency to lock

    lockIfNecessary (sql);

    // Call SQLExecDirect

    try {
        THROW_SQL(N3SQLExecDirect(m_aStatementHandle, (SDB_ODBC_CHAR*)aSql.getStr(),aSql.getLength()));
    }
    catch (SQLWarning& ex) {

        // Save pointer to warning and save with ResultSet
        // object once it is created.

        aWarning = ex;
    }

    // Now determine if there is a result set associated with
    // the SQL statement that was executed.  Get the column
    // count, and if it is not zero, there is a result set.

    if (getColumnCount () > 0)
    {
        hasResultSet = sal_True;
    }

    return hasResultSet;
}
//--------------------------------------------------------------------
// getResultSet
// getResultSet returns the current result as a ResultSet.  It
// returns NULL if the current result is not a ResultSet.
//--------------------------------------------------------------------
Reference< XResultSet > OStatement_Base::getResultSet (sal_Bool checkCount) throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if (m_xResultSet.get().is())  // if resultset already retrieved,
    {
        // throw exception to avoid sequence error
        throw SQLException(::rtl::OUString::createFromAscii("Invalid state for getResultSet"),*this,
            ::rtl::OUString(),0,Any());
    }

    OResultSet* pRs = NULL;
    sal_Int32 numCols = 1;

    // If we already know we have result columns, checkCount
    // is false.  This is an optimization to prevent unneeded
    // calls to getColumnCount

    if (checkCount)
        numCols = getColumnCount ();

    // Only return a result set if there are result columns

    if (numCols > 0)
    {
        pRs = new OResultSet(m_aStatementHandle,this);

        // Save a copy of our last result set
        // Changed to save copy at getResultSet.
        //m_xResultSet = rs;
    }
    else
        clearMyResultSet ();

    return pRs;
}
//--------------------------------------------------------------------
// getStmtOption
// Invoke SQLGetStmtOption with the given option.
//--------------------------------------------------------------------

sal_Int32 OStatement_Base::getStmtOption (short fOption) const throw( SQLException)
{
    sal_Int32   result = 0;

    // Reset last warning message

    //  clearWarnings();

    try {
        N3SQLGetStmtAttr(m_aStatementHandle, fOption,&result,SQL_IS_INTEGER,NULL);
    }
    catch (...)
    {
    }
    return result;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OStatement_Base::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XResultSet > xRS = NULL;

    // Execute the statement.  If execute returns true, a result
    // set exists.

    if (execute (sql))
    {
        xRS = getResultSet (sal_False);
    }
    else
    {
        // No ResultSet was produced.  Raise an exception
        throw SQLException(::rtl::OUString::createFromAscii("No ResultSet was produced"),*this,
            ::rtl::OUString(),0,Any());
    }
    return xRS;
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OStatement_Base::getConnection(  ) throw(SQLException, RuntimeException)
{
    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Any SAL_CALL OStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    if(!aRet.hasValue())
        aRet = OStatement_Base::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement::addBatch( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_aBatchList.push_back(sql);
}
// -------------------------------------------------------------------------
Sequence< sal_Int32 > SAL_CALL OStatement::executeBatch(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ::rtl::OString aBatchSql;
    sal_Int32 nLen = 0;
    for(::std::list< ::rtl::OUString>::const_iterator i=m_aBatchList.begin();i != m_aBatchList.end();++i,++nLen)
    {
        aBatchSql += ::rtl::OUStringToOString(*i,
                        osl_getThreadTextEncoding()
            );
        aBatchSql += ";";
    }

    THROW_SQL(N3SQLExecDirect(m_aStatementHandle, (SDB_ODBC_CHAR*)aBatchSql.getStr(),aBatchSql.getLength()));

    Sequence< sal_Int32 > aRet(nLen);
    sal_Int32* pArray = aRet.getArray();
    for(sal_Int32 j=0;j<nLen;++j)
    {
        SQLRETURN nError = N3SQLMoreResults(m_aStatementHandle);
        if(nError == SQL_SUCCESS)
        {
            N3SQLRowCount(m_aStatementHandle,&pArray[j]);
        }
    }
    return aRet;
}
// -------------------------------------------------------------------------


sal_Int32 SAL_CALL OStatement_Base::executeUpdate( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32 numRows = -1;

    // Execute the statement.  If execute returns false, a
    // row count exists.

    if (!execute (sql)) {
        numRows = getUpdateCount();
    }
    else {

        // No update count was produced (a ResultSet was).  Raise
        // an exception

        throw new SQLException (::rtl::OUString::createFromAscii("No row count was produced"),*this,
            ::rtl::OUString(),0,Any());
    }
    return numRows;

}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OStatement_Base::getResultSet(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_xResultSet = getResultSet(sal_True);
    return m_xResultSet;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OStatement_Base::getUpdateCount(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32 rowCount = -1;

    // Only return a row count for SQL statements that did not
    // return a result set.

    if (getColumnCount () == 0)
        rowCount = getRowCount ();

    return rowCount;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OStatement_Base::getMoreResults(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OResultSet* pRS = NULL;
    SQLWarning  warning;
    sal_Bool hasResultSet = sal_False;

    // clear previous warnings

    clearWarnings ();

    // Call SQLMoreResults

    try {
        hasResultSet = N3SQLMoreResults(m_aStatementHandle) == SQL_SUCCESS;
    }
    catch (SQLWarning &ex) {

        // Save pointer to warning and save with ResultSet
        // object once it is created.

        warning = ex;
    }

    // There are more results (it may not be a result set, though)

    if (hasResultSet)
    {

        // Now determine if there is a result set associated
        // with the SQL statement that was executed.  Get the
        // column count, and if it is zero, there is not a
        // result set.

        if (getColumnCount () == 0)
            hasResultSet = sal_False;
    }

    // Set the warning for the statement, if one was generated

    setWarning (warning);

    // Return the result set indicator

    return hasResultSet;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return makeAny(m_aLastWarning);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void SAL_CALL OStatement_Base::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_aLastWarning = SQLWarning();
}
// -------------------------------------------------------------------------
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getQueryTimeOut() const  throw(SQLException, RuntimeException)
{
    return getStmtOption(SQL_ATTR_QUERY_TIMEOUT);
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getMaxRows() const throw(SQLException, RuntimeException)
{
    return getStmtOption(SQL_ATTR_MAX_ROWS);
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getResultSetConcurrency() const throw(SQLException, RuntimeException)
{
    sal_uInt32 nValue;
    SQLRETURN nRetCode = N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_CONCURRENCY,&nValue,SQL_IS_UINTEGER,0);
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getResultSetType() const throw(SQLException, RuntimeException)
{
    sal_uInt32 nValue=0;
    SQLRETURN nRetCode = N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_CURSOR_SENSITIVITY,&nValue,SQL_IS_UINTEGER,0);
    nRetCode = N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_CURSOR_TYPE,&nValue,SQL_IS_UINTEGER,0);
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getFetchDirection() const throw(SQLException, RuntimeException)
{
    sal_uInt32 nValue;
    SQLRETURN nRetCode = N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_CURSOR_TYPE,&nValue,SQL_IS_UINTEGER,0);
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getFetchSize() const throw(SQLException, RuntimeException)
{
    sal_uInt32 nValue;
    SQLRETURN nRetCode = N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_ARRAY_SIZE,&nValue,SQL_IS_UINTEGER,0);
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getMaxFieldSize() const throw(SQLException, RuntimeException)
{
    return getStmtOption(SQL_ATTR_MAX_LENGTH);
}
//------------------------------------------------------------------------------
::rtl::OUString OStatement_Base::getCursorName() const throw(SQLException, RuntimeException)
{
    SQLCHAR pName[258];
    SQLSMALLINT nRealLen = 0;
    SQLRETURN nRetCode = N3SQLGetCursorName(m_aStatementHandle,(SQLCHAR*)pName,256,&nRealLen);
    return ::rtl::OUString::createFromAscii((const char*)pName);
}
//------------------------------------------------------------------------------
void OStatement_Base::setQueryTimeOut(sal_Int32 seconds) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    SQLRETURN nRetCode = N3SQLSetStmtAttr(m_aStatementHandle, SQL_ATTR_QUERY_TIMEOUT,(SQLPOINTER)seconds,SQL_IS_UINTEGER);
#ifdef DEBUG
    DEBUG_THROW
#endif
}
//------------------------------------------------------------------------------
void OStatement_Base::setMaxRows(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    SQLRETURN nRetCode = N3SQLSetStmtAttr(m_aStatementHandle, SQL_ATTR_MAX_ROWS, (SQLPOINTER)_par0,SQL_IS_UINTEGER);
#ifdef DEBUG
    DEBUG_THROW
#endif

}
//------------------------------------------------------------------------------
void OStatement_Base::setResultSetConcurrency(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    SQLINTEGER nSet;
    if(_par0 == ResultSetConcurrency::READ_ONLY)
        nSet = SQL_CONCUR_READ_ONLY;
    else
        nSet = SQL_CONCUR_VALUES;

    THROW_SQL(N3SQLSetStmtAttr(m_aStatementHandle, SQL_ATTR_CONCURRENCY,(SQLPOINTER)nSet,SQL_IS_UINTEGER));

}
//------------------------------------------------------------------------------
void OStatement_Base::setResultSetType(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    SQLRETURN nRetCode = N3SQLSetStmtAttr(m_aStatementHandle, SQL_ATTR_ROW_BIND_TYPE,(SQLPOINTER)SQL_BIND_BY_COLUMN,SQL_IS_UINTEGER);

    SQLUINTEGER nSet;
    switch(_par0)
    {
        case ResultSetType::FORWARD_ONLY:
            nSet =  SQL_UNSPECIFIED;
            break;
        case ResultSetType::SCROLL_INSENSITIVE:
            nSet =  SQL_INSENSITIVE;
            THROW_SQL(N3SQLSetStmtAttr(m_aStatementHandle, SQL_ATTR_CURSOR_TYPE,(SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN,SQL_IS_UINTEGER));
            break;
        case ResultSetType::SCROLL_SENSITIVE:
            nSet = SQL_CURSOR_DYNAMIC;
            if(N3SQLSetStmtAttr(m_aStatementHandle, SQL_ATTR_CURSOR_TYPE,(SQLPOINTER)nSet,SQL_IS_UINTEGER) != SQL_SUCCESS)
            {
                nSet = SQL_CURSOR_KEYSET_DRIVEN;
                THROW_SQL(N3SQLSetStmtAttr(m_aStatementHandle, SQL_ATTR_CURSOR_TYPE,(SQLPOINTER)nSet,SQL_IS_UINTEGER));
            }
            nSet =  SQL_SENSITIVE;
            break;
    }


    if(nRetCode = N3SQLSetStmtAttr(m_aStatementHandle, SQL_ATTR_CURSOR_SENSITIVITY,(SQLPOINTER)nSet,SQL_IS_UINTEGER) != SQL_SUCCESS)
    {
    }
}
//------------------------------------------------------------------------------
void OStatement_Base::setFetchDirection(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    SQLRETURN nRetCode = N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_CURSOR_TYPE,(SQLPOINTER)_par0,SQL_IS_UINTEGER);
}
//------------------------------------------------------------------------------
void OStatement_Base::setFetchSize(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    SQLRETURN nRetCode = N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)_par0,SQL_IS_UINTEGER);

#ifdef DEBUG
    DEBUG_THROW
#endif

    delete m_pRowStatusArray;
    m_pRowStatusArray = new SQLUSMALLINT[_par0];
    nRetCode = N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_STATUS_PTR,m_pRowStatusArray,SQL_IS_POINTER);
#ifdef DEBUG
    DEBUG_THROW
#endif

}
//------------------------------------------------------------------------------
void OStatement_Base::setMaxFieldSize(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    THROW_SQL(N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_MAX_LENGTH,(SQLPOINTER)_par0,SQL_IS_UINTEGER));
}
//------------------------------------------------------------------------------
void OStatement_Base::setCursorName(const ::rtl::OUString &_par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ::rtl::OString aName(::rtl::OUStringToOString(_par0,
                        osl_getThreadTextEncoding()
        ));
    THROW_SQL(N3SQLSetCursorName(m_aStatementHandle,(SDB_ODBC_CHAR*)aName.getStr(),aName.getLength()));
}
// -------------------------------------------------------------------------
sal_Bool OStatement_Base::isUsingBookmarks() const throw(SQLException, RuntimeException)
{
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_uInt32 nValue = SQL_UB_OFF;
    SQLRETURN nRet = N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_USE_BOOKMARKS,&nValue,SQL_IS_UINTEGER,NULL);
    //  THROW_SQL(nRet);
    return nValue != SQL_UB_OFF;
}
// -------------------------------------------------------------------------
void OStatement_Base::setUsingBookmarks(sal_Bool _bUseBookmark) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_uInt32 nValue = _bUseBookmark ? SQL_UB_VARIABLE : SQL_UB_OFF;
    SQLRETURN nRet = N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_USE_BOOKMARKS,(SQLPOINTER)nValue,SQL_IS_UINTEGER);
    THROW_SQL(nRet);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OStatement_Base::createArrayHelper( ) const
{
    Sequence< Property > aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,  ::rtl::OUString);
    DECL_BOOL_PROP0(ESCAPEPROCESSING);
    DECL_PROP0(FETCHDIRECTION,sal_Int32);
    DECL_PROP0(FETCHSIZE,   sal_Int32);
    DECL_PROP0(MAXFIELDSIZE,sal_Int32);
    DECL_PROP0(MAXROWS,     sal_Int32);
    DECL_PROP0(QUERYTIMEOUT,sal_Int32);
    DECL_PROP0(RESULTSETCONCURRENCY,sal_Int32);
    DECL_PROP0(RESULTSETTYPE,sal_Int32);
    DECL_BOOL_PROP0(USEBOOKMARKS);

    return new ::cppu::OPropertyArrayHelper(aProps);
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OStatement_Base::getInfoHelper()
{
    return *const_cast<OStatement_Base*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OStatement_Base::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
            return ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, getQueryTimeOut());
        case PROPERTY_ID_MAXFIELDSIZE:
            return ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, getMaxFieldSize());
        case PROPERTY_ID_MAXROWS:
            return ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, getMaxRows());
        case PROPERTY_ID_CURSORNAME:
            return ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, getCursorName());
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            return ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, getResultSetConcurrency());
        case PROPERTY_ID_RESULTSETTYPE:
            return ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, getResultSetType());
        case PROPERTY_ID_FETCHDIRECTION:
            return ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
        case PROPERTY_ID_FETCHSIZE:
            return ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
        case PROPERTY_ID_ESCAPEPROCESSING:
            //  return ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAsLink);
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            return ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, isUsingBookmarks());

        default:
            ;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void OStatement_Base::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
            setQueryTimeOut(connectivity::getINT32(rValue));
            break;
        case PROPERTY_ID_MAXFIELDSIZE:
            setMaxFieldSize(connectivity::getINT32(rValue));
            break;
        case PROPERTY_ID_MAXROWS:
            setMaxRows(connectivity::getINT32(rValue));
            break;
        case PROPERTY_ID_CURSORNAME:
            setCursorName(connectivity::getString(rValue));
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            setResultSetConcurrency(connectivity::getINT32(rValue));
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            setResultSetType(connectivity::getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            setFetchDirection(connectivity::getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHSIZE:
            setFetchSize(connectivity::getINT32(rValue));
            break;
        case PROPERTY_ID_ESCAPEPROCESSING:
            //  return ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAsLink);
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            setUsingBookmarks(connectivity::getBOOL(rValue));
            break;
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void OStatement_Base::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
            rValue <<= getQueryTimeOut();
            break;
        case PROPERTY_ID_MAXFIELDSIZE:
            rValue <<= getMaxFieldSize();
            break;
        case PROPERTY_ID_MAXROWS:
            rValue <<= getMaxRows();
            break;
        case PROPERTY_ID_CURSORNAME:
            rValue <<= getCursorName();
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            rValue <<= getResultSetConcurrency();
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            rValue <<= getResultSetType();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            rValue <<= getFetchDirection();
            break;
        case PROPERTY_ID_FETCHSIZE:
            rValue <<= getFetchSize();
            break;
        case PROPERTY_ID_ESCAPEPROCESSING:
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            rValue <<= cppu::bool2any(isUsingBookmarks());
            break;
        default:
            ;
    }
}
// -------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbcx.OStatement","com.sun.star.sdbc.Statement");

