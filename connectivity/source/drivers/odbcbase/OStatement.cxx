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


#include <stdio.h>
#include <osl/diagnose.h>
#include "odbc/OStatement.hxx"
#include "odbc/OConnection.hxx"
#include "odbc/OResultSet.hxx"
#include <comphelper/property.hxx>
#include "odbc/OTools.hxx"
#include <comphelper/uno3.hxx>
#include <osl/thread.h>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include "diagnose_ex.h"
#include <algorithm>
#include "resource/common_res.hrc"
#include "connectivity/dbexception.hxx"

using namespace ::comphelper;

#define THROW_SQL(x) \
    OTools::ThrowException(m_pConnection,x,m_aStatementHandle,SQL_HANDLE_STMT,*this)



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
OStatement_Base::OStatement_Base(OConnection* _pConnection )
    :OStatement_BASE(m_aMutex)
    ,OPropertySetHelper(OStatement_BASE::rBHelper)
    ,m_pConnection(_pConnection)
    ,m_aStatementHandle(SQL_NULL_HANDLE)
    ,m_pRowStatusArray(0)
    ,rBHelper(OStatement_BASE::rBHelper)
{
    osl_incrementInterlockedCount( &m_refCount );
    m_pConnection->acquire();
    m_aStatementHandle = m_pConnection->createStatementHandle();

    //setMaxFieldSize(0);
    // Don't do this. By ODBC spec, "0" is the default for the SQL_ATTR_MAX_LENGTH attribute. We once introduced
    // this line since an PostgreSQL ODBC driver had a default other than 0. However, current drivers (at least 8.3
    // and later) have a proper default of 0, so there should be no need anymore.
    // On the other hand, the NotesSQL driver (IBM's ODBC driver for the Lotus Notes series) wrongly interprets
    // "0" as "0", whereas the ODBC spec says it should in fact mean "unlimited".
    // So, removing this line seems to be the best option for now.
    // If we ever again encounter a ODBC driver which needs this option, then we should introduce a data source
    // setting for it, instead of unconditionally doing it.

    osl_decrementInterlockedCount( &m_refCount );
}
// -----------------------------------------------------------------------------
OStatement_Base::~OStatement_Base()
{
    OSL_ENSURE(!m_aStatementHandle,"Sohould ne null here!");
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
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::disposing(void)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    disposeResultSet();
    ::comphelper::disposeComponent(m_xGeneratedStatement);

    OSL_ENSURE(m_aStatementHandle,"OStatement_BASE2::disposing: StatementHandle is null!");
    if (m_pConnection)
    {
        m_pConnection->freeStatementHandle(m_aStatementHandle);
        m_pConnection->release();
        m_pConnection = NULL;
    }
    OSL_ENSURE(!m_aStatementHandle,"Sohould ne null here!");

    OStatement_BASE::disposing();
}
//------------------------------------------------------------------------------
void OStatement_BASE2::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    dispose_ChildImpl();
    OStatement_Base::disposing();
}
//-----------------------------------------------------------------------------
void SAL_CALL OStatement_BASE2::release() throw()
{
    relase_ChildImpl();
}
//-----------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if ( m_pConnection && !m_pConnection->isAutoRetrievingEnabled() && rType == ::getCppuType( (const Reference< XGeneratedResultSet > *)0 ) )
        return Any();
    Any aRet = OStatement_BASE::queryInterface(rType);
    return aRet.hasValue() ? aRet : OPropertySetHelper::queryInterface(rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OStatement_Base::getTypes(  ) throw(RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XPropertySet > *)0 ));
    Sequence< Type > aOldTypes = OStatement_BASE::getTypes();
    if ( m_pConnection && !m_pConnection->isAutoRetrievingEnabled() )
    {
        ::std::remove(aOldTypes.getArray(),aOldTypes.getArray() + aOldTypes.getLength(),
                        ::getCppuType( (const Reference< XGeneratedResultSet > *)0 ));
        aOldTypes.realloc(aOldTypes.getLength() - 1);
    }

    return ::comphelper::concatSequences(aTypes.getTypes(),aOldTypes);
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OStatement_Base::getGeneratedValues(  ) throw (SQLException, RuntimeException)
{
    OSL_ENSURE( m_pConnection && m_pConnection->isAutoRetrievingEnabled(),"Illegal call here. isAutoRetrievingEnabled is false!");
    Reference< XResultSet > xRes;
    if ( m_pConnection )
    {
        ::rtl::OUString sStmt = m_pConnection->getTransformedGeneratedStatement(m_sSqlStatement);
        if ( !sStmt.isEmpty() )
        {
            ::comphelper::disposeComponent(m_xGeneratedStatement);
            m_xGeneratedStatement = m_pConnection->createStatement();
            xRes = m_xGeneratedStatement->executeQuery(sStmt);
        }
    }
    return xRes;
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    OTools::ThrowException(m_pConnection,N3SQLCancel(m_aStatementHandle),m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OStatement_BASE::rBHelper.bDisposed);

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
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    clearWarnings ();

    if (m_xResultSet.get().is())
    {
        clearMyResultSet();
    }
    if(m_aStatementHandle)
    {
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
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    try
    {
        Reference<XCloseable> xCloseable;
        if ( ::comphelper::query_interface( m_xResultSet.get(), xCloseable ) )
            xCloseable->close();
    }
    catch( const DisposedException& ) { }

    m_xResultSet = Reference< XResultSet >();
}
//--------------------------------------------------------------------
SQLLEN OStatement_Base::getRowCount () throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    SQLLEN numRows = 0;

    try {
        THROW_SQL(N3SQLRowCount(m_aStatementHandle,&numRows));
    }
    catch (const SQLException&)
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

    ::rtl::OUString sqlStatement = sql.toAsciiUpperCase ();

    // Now, look for the FOR UPDATE keywords.  If there is any extra white
    // space between the FOR and UPDATE, this will fail.

    sal_Int32 index = sqlStatement.indexOf(" FOR UPDATE");

    // We found it.  Change our concurrency level to ensure that the
    // row can be updated.

    if (index > 0)
    {
        OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
        try
        {
            THROW_SQL((setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CONCURRENCY, SQL_CONCUR_LOCK)));
        }
        catch (const SQLWarning& warn)
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
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aLastWarning = ex;
}

//--------------------------------------------------------------------
// getColumnCount
// Return the number of columns in the ResultSet
//--------------------------------------------------------------------

sal_Int32 OStatement_Base::getColumnCount () throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    sal_Int16   numCols = 0;
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");

    try {
        THROW_SQL(N3SQLNumResultCols(m_aStatementHandle,&numCols));
    }
    catch (const SQLException&)
    {
    }
    return numCols;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OStatement_Base::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    printf("--- OStatement_Base::execute\n");
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    m_sSqlStatement = sql;


    ::rtl::OString aSql(::rtl::OUStringToOString(sql,getOwnConnection()->getTextEncoding()));

    sal_Bool hasResultSet = sal_False;
    SQLWarning aWarning;

    // Reset the statement handle and warning

    reset();

    // Check for a 'FOR UPDATE' statement.  If present, change
    // the concurrency to lock

    lockIfNecessary (sql);

    // Call SQLExecDirect
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");

    try {
        THROW_SQL(N3SQLExecDirect(m_aStatementHandle, (SDB_ODBC_CHAR*)aSql.getStr(),aSql.getLength()));
    }
    catch (const SQLWarning& ex) {

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
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    if (m_xResultSet.get().is())  // if resultset already retrieved,
    {
        // throw exception to avoid sequence error
        ::dbtools::throwFunctionSequenceException(*this,Any());
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
        OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
        pRs = createResulSet();
        pRs->construct();

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

template < typename T, SQLINTEGER BufferLength > T OStatement_Base::getStmtOption (SQLINTEGER fOption, T dflt) const
{
    T result (dflt);
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    N3SQLGetStmtAttr(m_aStatementHandle, fOption, &result, BufferLength, NULL);
    return result;
}
template < typename T, SQLINTEGER BufferLength > SQLRETURN OStatement_Base::setStmtOption (SQLINTEGER fOption, T value) const
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    SQLPOINTER sv = reinterpret_cast<SQLPOINTER>(value);
    return N3SQLSetStmtAttr(m_aStatementHandle, fOption, sv, BufferLength);
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OStatement_Base::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    Reference< XResultSet > xRS = NULL;

    // Execute the statement.  If execute returns true, a result
    // set exists.

    if (execute (sql))
    {
        xRS = getResultSet (sal_False);
        m_xResultSet = xRS;
    }
    else
    {
        // No ResultSet was produced.  Raise an exception
        m_pConnection->throwGenericSQLException(STR_NO_RESULTSET,*this);
    }
    return xRS;
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OStatement_Base::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Any SAL_CALL OStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    return aRet.hasValue() ? aRet : OStatement_Base::queryInterface(rType);
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement::addBatch( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aBatchList.push_back(sql);
}
// -------------------------------------------------------------------------
Sequence< sal_Int32 > SAL_CALL OStatement::executeBatch(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    ::rtl::OString aBatchSql;
    sal_Int32 nLen = 0;
    for(::std::list< ::rtl::OUString>::const_iterator i=m_aBatchList.begin();i != m_aBatchList.end();++i,++nLen)
    {
        aBatchSql += ::rtl::OUStringToOString(*i,getOwnConnection()->getTextEncoding());
        aBatchSql += ";";
    }

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    THROW_SQL(N3SQLExecDirect(m_aStatementHandle, (SDB_ODBC_CHAR*)aBatchSql.getStr(),aBatchSql.getLength()));

    Sequence< sal_Int32 > aRet(nLen);
    sal_Int32* pArray = aRet.getArray();
    for(sal_Int32 j=0;j<nLen;++j)
    {
        SQLRETURN nError = N3SQLMoreResults(m_aStatementHandle);
        if(nError == SQL_SUCCESS)
        {
            SQLLEN nRowCount=0;
            N3SQLRowCount(m_aStatementHandle,&nRowCount);
            pArray[j] = nRowCount;
        }
    }
    return aRet;
}
// -------------------------------------------------------------------------


sal_Int32 SAL_CALL OStatement_Base::executeUpdate( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    sal_Int32 numRows = -1;

    // Execute the statement.  If execute returns false, a
    // row count exists.

    if (!execute (sql)) {
        numRows = getUpdateCount();
    }
    else {

        // No update count was produced (a ResultSet was).  Raise
        // an exception

        ::connectivity::SharedResources aResources;
        const ::rtl::OUString sError( aResources.getResourceString(STR_NO_ROWCOUNT));
        throw SQLException (sError, *this,::rtl::OUString(),0,Any());
    }
    return numRows;

}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OStatement_Base::getResultSet(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_xResultSet = getResultSet(sal_True);
    return m_xResultSet;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OStatement_Base::getUpdateCount(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


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
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    SQLWarning  warning;
    sal_Bool hasResultSet = sal_False;

    // clear previous warnings

    clearWarnings ();

    // Call SQLMoreResults
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");

    try {
        hasResultSet = N3SQLMoreResults(m_aStatementHandle) == SQL_SUCCESS;
    }
    catch (const SQLWarning &ex) {

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
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return makeAny(m_aLastWarning);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void SAL_CALL OStatement_Base::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aLastWarning = SQLWarning();
}
// -------------------------------------------------------------------------
//------------------------------------------------------------------------------
sal_Int64 OStatement_Base::getQueryTimeOut() const
{
    return getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_QUERY_TIMEOUT);
}
//------------------------------------------------------------------------------
sal_Int64 OStatement_Base::getMaxRows() const
{
    return getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_MAX_ROWS);
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getResultSetConcurrency() const
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    SQLULEN nValue (getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CONCURRENCY));
    if(nValue == SQL_CONCUR_READ_ONLY)
        nValue = ResultSetConcurrency::READ_ONLY;
    else
        nValue = ResultSetConcurrency::UPDATABLE;
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getResultSetType() const
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    SQLULEN nValue (getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_TYPE, SQL_CURSOR_FORWARD_ONLY));
    switch(nValue)
    {
        case SQL_CURSOR_FORWARD_ONLY:
            nValue = ResultSetType::FORWARD_ONLY;
            break;
        case SQL_CURSOR_KEYSET_DRIVEN:
        case SQL_CURSOR_STATIC:
            nValue = ResultSetType::SCROLL_INSENSITIVE;
            break;
        case SQL_CURSOR_DYNAMIC:
            nValue = ResultSetType::SCROLL_SENSITIVE;
            break;
        default:
            OSL_FAIL("Unknown ODBC Cursor Type");
    }

    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getFetchDirection() const
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    SQLULEN nValue (getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_SCROLLABLE));
    switch(nValue)
    {
        case SQL_SCROLLABLE:
            nValue = FetchDirection::REVERSE;
            break;
        default:
            nValue = FetchDirection::FORWARD;
            break;
    }

    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getFetchSize() const
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    return getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_ROW_ARRAY_SIZE);
}
//------------------------------------------------------------------------------
sal_Int64 OStatement_Base::getMaxFieldSize() const
{
    return getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_MAX_LENGTH);
}
//------------------------------------------------------------------------------
::rtl::OUString OStatement_Base::getCursorName() const
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    SQLCHAR pName[258];
    SQLSMALLINT nRealLen = 0;
    SQLRETURN nRetCode = N3SQLGetCursorName(m_aStatementHandle,(SQLCHAR*)pName,256,&nRealLen);
    OSL_UNUSED( nRetCode );
    return ::rtl::OUString::createFromAscii((const char*)pName);
}
//------------------------------------------------------------------------------
void OStatement_Base::setQueryTimeOut(sal_Int64 seconds)
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_QUERY_TIMEOUT,seconds);
}
//------------------------------------------------------------------------------
void OStatement_Base::setMaxRows(sal_Int64 _par0)
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_MAX_ROWS, _par0);
}
//------------------------------------------------------------------------------
void OStatement_Base::setResultSetConcurrency(sal_Int32 _par0)
{
    SQLULEN nSet;
    if(_par0 == ResultSetConcurrency::READ_ONLY)
        nSet = SQL_CONCUR_READ_ONLY;
    else
        nSet = SQL_CONCUR_VALUES;

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CONCURRENCY, nSet);
}
//------------------------------------------------------------------------------
void OStatement_Base::setResultSetType(sal_Int32 _par0)
{

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_ROW_BIND_TYPE, SQL_BIND_BY_COLUMN);

    sal_Bool bUseBookmark = isUsingBookmarks();
    SQLULEN nSet( SQL_UNSPECIFIED );
    switch(_par0)
    {
        case ResultSetType::FORWARD_ONLY:
            nSet =  SQL_UNSPECIFIED;
            break;
        case ResultSetType::SCROLL_INSENSITIVE:
            nSet =  SQL_INSENSITIVE;
            setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_TYPE, SQL_CURSOR_KEYSET_DRIVEN);
            break;
        case ResultSetType::SCROLL_SENSITIVE:
            if(bUseBookmark)
            {
                SQLUINTEGER nCurProp = getCursorProperties(SQL_CURSOR_DYNAMIC,sal_True);
                if((nCurProp & SQL_CA1_BOOKMARK) != SQL_CA1_BOOKMARK) // check if bookmark for this type isn't supported
                { // we have to test the next one
                    nCurProp = getCursorProperties(SQL_CURSOR_KEYSET_DRIVEN,sal_True);
                    sal_Bool bNotBookmarks = ((nCurProp & SQL_CA1_BOOKMARK) != SQL_CA1_BOOKMARK);
                    nCurProp = getCursorProperties(SQL_CURSOR_KEYSET_DRIVEN,sal_False);
                    nSet = SQL_CURSOR_KEYSET_DRIVEN;
                    if( bNotBookmarks ||
                        ((nCurProp & SQL_CA2_SENSITIVITY_DELETIONS) != SQL_CA2_SENSITIVITY_DELETIONS) ||
                        ((nCurProp & SQL_CA2_SENSITIVITY_ADDITIONS) != SQL_CA2_SENSITIVITY_ADDITIONS))
                    {
                        // bookmarks for keyset isn't supported so reset bookmark setting
                        setUsingBookmarks(sal_False);
                        nSet = SQL_CURSOR_DYNAMIC;
                    }
                }
                else
                    nSet = SQL_CURSOR_DYNAMIC;
            }
            else
                nSet = SQL_CURSOR_DYNAMIC;
            if( setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_TYPE, nSet) != SQL_SUCCESS )
            {
                setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_TYPE, SQL_CURSOR_KEYSET_DRIVEN);
            }
            nSet =  SQL_SENSITIVE;
            break;
        default:
            OSL_FAIL( "OStatement_Base::setResultSetType: invalid result set type!" );
            break;
    }


    setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_SENSITIVITY, nSet);
}
//------------------------------------------------------------------------------
void OStatement_Base::setEscapeProcessing( const sal_Bool _bEscapeProc )
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    SQLULEN nEscapeProc( _bEscapeProc ? SQL_NOSCAN_OFF : SQL_NOSCAN_ON );
    setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_NOSCAN, nEscapeProc);
}

//------------------------------------------------------------------------------
void OStatement_Base::setFetchDirection(sal_Int32 _par0)
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    if(_par0 == FetchDirection::FORWARD)
    {
        setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_SCROLLABLE, SQL_NONSCROLLABLE);
    }
    else if(_par0 == FetchDirection::REVERSE)
    {
        setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_SCROLLABLE, SQL_SCROLLABLE);
    }
}
//------------------------------------------------------------------------------
void OStatement_Base::setFetchSize(sal_Int32 _par0)
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    OSL_ENSURE(_par0>0,"Illegal fetch size!");
    if ( _par0 > 0 )
    {
        setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_ROW_ARRAY_SIZE, _par0);

        if (m_pRowStatusArray)
            delete[] m_pRowStatusArray;
        m_pRowStatusArray = new SQLUSMALLINT[_par0];
        setStmtOption<SQLUSMALLINT*, SQL_IS_POINTER>(SQL_ATTR_ROW_STATUS_PTR, m_pRowStatusArray);
    }
}
//------------------------------------------------------------------------------
void OStatement_Base::setMaxFieldSize(sal_Int64 _par0)
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_MAX_LENGTH, _par0);
}
//------------------------------------------------------------------------------
void OStatement_Base::setCursorName(const ::rtl::OUString &_par0)
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    ::rtl::OString aName(::rtl::OUStringToOString(_par0,getOwnConnection()->getTextEncoding()));
    N3SQLSetCursorName(m_aStatementHandle,(SDB_ODBC_CHAR*)aName.getStr(),(SQLSMALLINT)aName.getLength());
}
// -------------------------------------------------------------------------
sal_Bool OStatement_Base::isUsingBookmarks() const
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    return SQL_UB_OFF != getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_USE_BOOKMARKS, SQL_UB_OFF);
}
// -------------------------------------------------------------------------
sal_Bool OStatement_Base::getEscapeProcessing() const
{
    OSL_ENSURE( m_aStatementHandle, "StatementHandle is null!" );
    return SQL_NOSCAN_OFF == getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_USE_BOOKMARKS, SQL_NOSCAN_OFF);;
}
// -------------------------------------------------------------------------
void OStatement_Base::setUsingBookmarks(sal_Bool _bUseBookmark)
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    SQLULEN nValue = _bUseBookmark ? SQL_UB_VARIABLE : SQL_UB_OFF;
    setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_USE_BOOKMARKS, nValue);
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
    DECL_PROP0(MAXFIELDSIZE,sal_Int64);
    DECL_PROP0(MAXROWS,     sal_Int64);
    DECL_PROP0(QUERYTIMEOUT,sal_Int64);
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
    sal_Bool bConverted = sal_False;
    try
    {
        switch(nHandle)
        {
            case PROPERTY_ID_QUERYTIMEOUT:
                bConverted = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getQueryTimeOut());
                break;

            case PROPERTY_ID_MAXFIELDSIZE:
                bConverted = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getMaxFieldSize());
                break;

            case PROPERTY_ID_MAXROWS:
                bConverted = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getMaxRows());
                break;

            case PROPERTY_ID_CURSORNAME:
                bConverted = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getCursorName());
                break;

            case PROPERTY_ID_RESULTSETCONCURRENCY:
                bConverted = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getResultSetConcurrency());
                break;

            case PROPERTY_ID_RESULTSETTYPE:
                bConverted = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getResultSetType());
                break;

            case PROPERTY_ID_FETCHDIRECTION:
                bConverted = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
                break;

            case PROPERTY_ID_FETCHSIZE:
                bConverted = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
                break;

            case PROPERTY_ID_USEBOOKMARKS:
                bConverted = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, isUsingBookmarks());
                break;

            case PROPERTY_ID_ESCAPEPROCESSING:
                bConverted = ::comphelper::tryPropertyValue( rConvertedValue, rOldValue, rValue, getEscapeProcessing() );
                break;

        }
    }
    catch(const SQLException&)
    {
        //  throw Exception(e.Message,*this);
    }
    return bConverted;
}
// -------------------------------------------------------------------------
void OStatement_Base::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    try
    {
        switch(nHandle)
        {
            case PROPERTY_ID_QUERYTIMEOUT:
                setQueryTimeOut(comphelper::getINT64(rValue));
                break;
            case PROPERTY_ID_MAXFIELDSIZE:
                setMaxFieldSize(comphelper::getINT64(rValue));
                break;
            case PROPERTY_ID_MAXROWS:
                setMaxRows(comphelper::getINT64(rValue));
                break;
            case PROPERTY_ID_CURSORNAME:
                setCursorName(comphelper::getString(rValue));
                break;
            case PROPERTY_ID_RESULTSETCONCURRENCY:
                setResultSetConcurrency(comphelper::getINT32(rValue));
                break;
            case PROPERTY_ID_RESULTSETTYPE:
                setResultSetType(comphelper::getINT32(rValue));
                break;
            case PROPERTY_ID_FETCHDIRECTION:
                setFetchDirection(comphelper::getINT32(rValue));
                break;
            case PROPERTY_ID_FETCHSIZE:
                setFetchSize(comphelper::getINT32(rValue));
                break;
            case PROPERTY_ID_USEBOOKMARKS:
                setUsingBookmarks(comphelper::getBOOL(rValue));
                break;
            case PROPERTY_ID_ESCAPEPROCESSING:
                setEscapeProcessing( ::comphelper::getBOOL( rValue ) );
                break;
            default:
                OSL_FAIL( "OStatement_Base::setFastPropertyValue_NoBroadcast: what property?" );
                break;
        }
    }
    catch(const SQLException& )
    {
        //  throw Exception(e.Message,*this);
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
        case PROPERTY_ID_USEBOOKMARKS:
            rValue <<= isUsingBookmarks();
            break;
        case PROPERTY_ID_ESCAPEPROCESSING:
            rValue <<= getEscapeProcessing();
            break;
        default:
            OSL_FAIL( "OStatement_Base::getFastPropertyValue: what property?" );
            break;
    }
}
// -------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbcx.OStatement","com.sun.star.sdbc.Statement");
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::acquire() throw()
{
    OStatement_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::release() throw()
{
    OStatement_BASE::release();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
OResultSet* OStatement_Base::createResulSet()
{
    return new OResultSet(m_aStatementHandle,this);
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OStatement_Base::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
SQLUINTEGER OStatement_Base::getCursorProperties(SQLINTEGER _nCursorType,sal_Bool bFirst)
{
    SQLUINTEGER nValueLen = 0;
    try
    {
        SQLUSMALLINT nAskFor = SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2;
        if(SQL_CURSOR_KEYSET_DRIVEN == _nCursorType)
            nAskFor = bFirst ? SQL_KEYSET_CURSOR_ATTRIBUTES1 : SQL_KEYSET_CURSOR_ATTRIBUTES2;
        else if(SQL_CURSOR_STATIC  == _nCursorType)
            nAskFor = bFirst ? SQL_STATIC_CURSOR_ATTRIBUTES1 : SQL_STATIC_CURSOR_ATTRIBUTES2;
        else if(SQL_CURSOR_FORWARD_ONLY == _nCursorType)
            nAskFor = bFirst ? SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1 : SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2;
        else if(SQL_CURSOR_DYNAMIC == _nCursorType)
            nAskFor = bFirst ? SQL_DYNAMIC_CURSOR_ATTRIBUTES1 : SQL_DYNAMIC_CURSOR_ATTRIBUTES2;


        OTools::GetInfo(getOwnConnection(),getConnectionHandle(),nAskFor,nValueLen,NULL);
    }
    catch(const Exception&)
    { // we don't want our result destroy here
        nValueLen = 0;
    }
    return nValueLen;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
