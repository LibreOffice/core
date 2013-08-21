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
#include <string.h>
#include <osl/diagnose.h>
#include "diagnose_ex.h"
#include "odbc/OPreparedStatement.hxx"
#include "odbc/OBoundParam.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include "odbc/OTools.hxx"
#include "odbc/ODriver.hxx"
#include "odbc/OResultSet.hxx"
#include "odbc/OResultSetMetaData.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include "connectivity/dbtools.hxx"
#include <comphelper/types.hxx>
#include "connectivity/FValue.hxx"
#include "resource/common_res.hrc"
#include "connectivity/sqlparse.hxx"
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/is_same.hpp>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbcx.OPreparedStatement","com.sun.star.sdbc.PreparedStatement");

namespace
{
    // for now, never use wchar,
    // but most of code is prepared to handle it
    // in case we make this configurable
    const bool useWChar = false;
}

OPreparedStatement::OPreparedStatement( OConnection* _pConnection,const OUString& sql)
    :OStatement_BASE2(_pConnection)
    ,numParams(0)
    ,boundParams(NULL)
    ,m_bPrepared(sal_False)
{
    m_sSqlStatement = sql;
    try
    {
        if(_pConnection->isParameterSubstitutionEnabled())
        {
            OSQLParser aParser( comphelper::getComponentContext(_pConnection->getDriver()->getORB()) );
            OUString sErrorMessage;
            OUString sNewSql;
            ::std::auto_ptr<OSQLParseNode> pNode( aParser.parseTree(sErrorMessage,sql) );
            if ( pNode.get() )
            {   // special handling for parameters
                OSQLParseNode::substituteParameterNames(pNode.get());
                pNode->parseNodeToStr( sNewSql, _pConnection );
                m_sSqlStatement = sNewSql;
            }
        }
    }
    catch(Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
Any SAL_CALL OPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE2::queryInterface(rType);
    return aRet.hasValue() ? aRet : OPreparedStatement_BASE::queryInterface(rType);
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OPreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::comphelper::concatSequences(OPreparedStatement_BASE::getTypes(),OStatement_BASE2::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    prepareStatement();
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(getOwnConnection(),m_aStatementHandle);
    return m_xMetaData;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    // Close/clear our result set
    clearMyResultSet ();

    // Reset last warning message

    try {
        clearWarnings ();
        OStatement_BASE2::close();
        FreeParams();
    }
    catch (SQLException &) {
        // If we get an error, ignore
    }

    // Remove this Statement object from the Connection object's
    // list
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    sal_Bool needData = sal_False;

    // Reset warnings

    clearWarnings ();

    // Reset the statement handle, warning and saved Resultset

    reset();

    // Call SQLExecute
    prepareStatement();

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    try
    {
        SQLRETURN nReturn = N3SQLExecute(m_aStatementHandle);

        OTools::ThrowException(m_pConnection,nReturn,m_aStatementHandle,SQL_HANDLE_STMT,*this);
        needData = nReturn == SQL_NEED_DATA;

        // Now loop while more data is needed (i.e. a data-at-
        // execution parameter was given).  For each parameter
        // that needs data, put the data from the input stream.

        while (needData) {

            // Get the parameter number that requires data

            sal_Int32* paramIndex = 0;
            nReturn = N3SQLParamData(m_aStatementHandle,(SQLPOINTER*)&paramIndex);

            // If the parameter index is -1, there is no
            // more data required

            if ( !paramIndex || ( *paramIndex == -1 ) )
                needData = sal_False;
            else
            {
                // Now we have the proper parameter
                // index, get the data from the input
                // stream and do a SQLPutData
                putParamData (*paramIndex);
            }
        }

    }
    catch (const SQLWarning&)
    {
    }

    // Now determine if there is a result set associated with
    // the SQL statement that was executed.  Get the column
    // count, and if it is not zero, there is a result set.


    return getColumnCount() > 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    sal_Int32 numRows = -1;

    prepareStatement();
    // Execute the statement.  If execute returns sal_False, a
    // row count exists.

    if (!execute())
        numRows = getUpdateCount ();
    else
    {
        // No update count was produced (a ResultSet was).  Raise
        // an exception
        m_pConnection->throwGenericSQLException(STR_NO_ROWCOUNT,*this);
    }
    return numRows;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const OUString& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex, DataType::CHAR, invalid_scale, x);
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OPreparedStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet > rs = NULL;

    prepareStatement();

    if (execute())
        rs = getResultSet(sal_False);
    else
    {
        // No ResultSet was produced.  Raise an exception
        m_pConnection->throwGenericSQLException(STR_NO_RESULTSET,*this);
    }
    return rs;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    // Set the parameter as if it were an integer
    setInt (parameterIndex, x ? 1 : 0 );
}
// -------------------------------------------------------------------------
// The MutexGuard must _already_ be taken!
void OPreparedStatement::setParameterPre(sal_Int32 parameterIndex)
{
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    prepareStatement();
    checkParameterIndex(parameterIndex);
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
}
// -------------------------------------------------------------------------

template <typename T> void OPreparedStatement::setScalarParameter(const sal_Int32 parameterIndex, const sal_Int32 i_nType, const SQLULEN i_nColSize, const T i_Value)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    setParameterPre(parameterIndex);

    typedef typename boost::remove_reference< T >::type TnoRef;

    TnoRef *bindBuf = static_cast< TnoRef* >( allocBindBuf(parameterIndex, sizeof(i_Value)) );
    *bindBuf = i_Value;

    setParameter(parameterIndex, i_nType, i_nColSize, invalid_scale, bindBuf, sizeof(i_Value), sizeof(i_Value));
}
// -------------------------------------------------------------------------

void OPreparedStatement::setParameter(const sal_Int32 parameterIndex, const sal_Int32 _nType, const sal_Int16 _nScale, const OUString &_sData)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    setParameterPre(parameterIndex);

    assert (_nType == DataType::VARCHAR || _nType == DataType::CHAR || _nType == DataType::DECIMAL || _nType == DataType::NUMERIC);

    sal_Int32 nCharLen;
    sal_Int32 nByteLen;
    void *pData;
    if (useWChar)
    {
        /*
         * On Windows, wchar is 16 bits (UTF-16 encoding), the ODBC "W" variants functions take UTF-16 encoded strings
         * and character lengths are number of UTF-16 codepoints.
         * Reference: http://msdn.microsoft.com/en-us/library/windows/desktop/ms716246%28v=vs.85%29.aspx
         * ODBC Programmer's reference > Developing Applications > Programming Considerations > Unicode >  Unicode Function Arguments
         *            http://support.microsoft.com/kb/294169
         *
         * UnixODBC can be configured at compile-time so that the "W" variants expect
         * UTF-16 or UTF-32 encoded strings, and character lengths are number of codepoints.
         * However, UTF-16 is the default, what all/most distributions do
         * and the established API that most drivers implement.
         * As wchar is often 32 bits, this differs from C-style strings of wchar!
         *
         * Our internal OUString storage is always UTF-16, so no conversion to do here.
         */
        BOOST_STATIC_ASSERT( sizeof(sal_Unicode) == 2 );
        BOOST_STATIC_ASSERT( sizeof(SQLWCHAR)    == 2 );
        nCharLen = _sData.getLength();
        nByteLen = nCharLen * sizeof(sal_Unicode);
        pData = allocBindBuf(parameterIndex, nByteLen);
        memcpy(pData, _sData.getStr(), nByteLen);
    }
    else
    {
        OString sOData( OUStringToOString(_sData, getOwnConnection()->getTextEncoding()) );
        nCharLen = sOData.getLength();
        nByteLen = nCharLen;
        pData = allocBindBuf(parameterIndex, nByteLen);
        memcpy(pData, sOData.getStr(), nByteLen);
    }

    setParameter( parameterIndex, _nType, nCharLen, _nScale, pData, nByteLen, nByteLen );
}
// -------------------------------------------------------------------------
void OPreparedStatement::setParameter(const sal_Int32 parameterIndex, const sal_Int32 _nType, const Sequence< sal_Int8 > &x)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    setParameterPre(parameterIndex);

    assert(_nType == DataType::BINARY || _nType == DataType::VARBINARY);

    // don't copy the sequence, just point the ODBC directly at the sequence's storage array
    // Why BINARY/Sequence is treated differently than strings (which are copied), I'm not sure
    OSL_VERIFY(allocBindBuf(parameterIndex, 0) == NULL);
    boundParams[parameterIndex-1].setSequence(x); // this ensures that the sequence stays alive

    setParameter( parameterIndex, _nType, x.getLength(), invalid_scale, x.getConstArray(), x.getLength(), x.getLength() );
}
// -------------------------------------------------------------------------
void OPreparedStatement::setParameter(const sal_Int32 parameterIndex, const sal_Int32 _nType, const SQLULEN _nColumnSize, const sal_Int32 _nScale, const void* const _pData, const SQLULEN _nDataLen, const SQLLEN _nDataAllocLen)
{
    SQLSMALLINT fCType, fSqlType;
    OTools::getBindTypes(useWChar, m_pConnection->useOldDateFormat(), OTools::jdbcTypeToOdbc(_nType), fCType, fSqlType);

    SQLLEN *pDataLen=boundParams[parameterIndex-1].getBindLengthBuffer();
    *pDataLen=_nDataLen;

    SQLRETURN nRetcode;
    nRetcode = (*(T3SQLBindParameter)m_pConnection->getOdbcFunction(ODBC3SQLBindParameter))(
                  m_aStatementHandle,
                  // checkParameterIndex guarantees this is safe
                  static_cast<SQLUSMALLINT>(parameterIndex),
                  SQL_PARAM_INPUT,
                  fCType,
                  fSqlType,
                  _nColumnSize,
                  _nScale,
                  // we trust the ODBC driver not to touch it because SQL_PARAM_INPUT
                  const_cast<void*>(_pData),
                  _nDataAllocLen,
                  pDataLen);

    OTools::ThrowException(m_pConnection, nRetcode, m_aStatementHandle, SQL_HANDLE_STMT, *this);
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setByte( const sal_Int32 parameterIndex, const sal_Int8 x ) throw(SQLException, RuntimeException)
{
    setScalarParameter(parameterIndex, DataType::TINYINT, 3, x);
}
// -------------------------------------------------------------------------
// For older compilers (that do not support partial specialisation of class templates)
// uncomment if necessary (safe also on compilers that *do* support partial specialisation)
//BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(DATE_STRUCT);
//BOOST_STATIC_ASSERT((boost::is_same<DATE_STRUCT, boost::remove_reference<DATE_STRUCT&>::type>::value));
void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const Date& aData ) throw(SQLException, RuntimeException)
{
    DATE_STRUCT x(OTools::DateToOdbcDate(aData));
    setScalarParameter<DATE_STRUCT&>(parameterIndex, DataType::DATE, 10, x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const Time& aVal ) throw(SQLException, RuntimeException)
{
    SQLULEN nColSize;
    if(aVal.NanoSeconds == 0)
        nColSize = 8;
    else if(aVal.NanoSeconds % 100000000 == 0)
        nColSize = 10;
    else if(aVal.NanoSeconds % 10000000 == 0)
        nColSize = 11;
    else if(aVal.NanoSeconds % 1000000 == 0)
        nColSize = 12;
    else if(aVal.NanoSeconds % 100000 == 0)
        nColSize = 13;
    else if(aVal.NanoSeconds % 10000 == 0)
        nColSize = 14;
    else if(aVal.NanoSeconds % 1000 == 0)
        nColSize = 15;
    else if(aVal.NanoSeconds % 100 == 0)
        nColSize = 16;
    else if(aVal.NanoSeconds % 10 == 0)
        nColSize = 17;
    else
        nColSize = 18;
    TIME_STRUCT x(OTools::TimeToOdbcTime(aVal));
    setScalarParameter<TIME_STRUCT&>(parameterIndex, DataType::TIME, nColSize, x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const DateTime& aVal ) throw(SQLException, RuntimeException)
{
    SQLULEN nColSize;
    if(aVal.NanoSeconds == 0)
    {
        if (aVal.Seconds == 0)
            nColSize=16;
        else
            nColSize=19;
    }
    else if(aVal.NanoSeconds % 100000000 == 0)
        nColSize = 21;
    else if(aVal.NanoSeconds % 10000000 == 0)
        nColSize = 22;
    else if(aVal.NanoSeconds % 1000000 == 0)
        nColSize = 23;
    else if(aVal.NanoSeconds % 100000 == 0)
        nColSize = 24;
    else if(aVal.NanoSeconds % 10000 == 0)
        nColSize = 25;
    else if(aVal.NanoSeconds % 1000 == 0)
        nColSize = 26;
    else if(aVal.NanoSeconds % 100 == 0)
        nColSize = 27;
    else if(aVal.NanoSeconds % 10 == 0)
        nColSize = 28;
    else
        nColSize = 29;

    TIMESTAMP_STRUCT x(OTools::DateTimeToTimestamp(aVal));
    setScalarParameter<TIMESTAMP_STRUCT&>(parameterIndex, DataType::TIMESTAMP, nColSize, x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    setScalarParameter(parameterIndex, DataType::DOUBLE, 15, x);
}

// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    setScalarParameter(parameterIndex, DataType::FLOAT, 15, x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    setScalarParameter(parameterIndex, DataType::INTEGER, 10, x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    try
    {
        setScalarParameter(parameterIndex, DataType::BIGINT, 19, x);
    }
    catch(SQLException&)
    {
        setString(parameterIndex, ORowSetValue(x));
    }
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, const sal_Int32 _nType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    setParameterPre(parameterIndex);

    OSL_VERIFY(allocBindBuf(parameterIndex, 0) == NULL);
    SQLLEN * const lenBuf = getLengthBuf (parameterIndex);
    *lenBuf = SQL_NULL_DATA;


    SQLSMALLINT fCType;
    SQLSMALLINT fSqlType;

    OTools::getBindTypes(   useWChar,
                            m_pConnection->useOldDateFormat(),
                            OTools::jdbcTypeToOdbc(_nType),
                            fCType,
                            fSqlType);

    SQLRETURN nReturn = N3SQLBindParameter( m_aStatementHandle,
                                            static_cast<SQLUSMALLINT>(parameterIndex),
                                            SQL_PARAM_INPUT,
                                            fCType,
                                            fSqlType,
                                            0,
                                            0,
                                            NULL,
                                            0,
                                            lenBuf
                                            );
    OTools::ThrowException(m_pConnection,nReturn,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
    if ( x.is() )
        setStream(parameterIndex, x->getCharacterStream(), x->length(), DataType::LONGVARCHAR);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
    if ( x.is() )
        setStream(parameterIndex, x->getBinaryStream(), x->length(), DataType::LONGVARBINARY);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setArray( sal_Int32 /*parameterIndex*/, const Reference< XArray >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setArray", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setRef( sal_Int32 /*parameterIndex*/, const Reference< XRef >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setRef", *this );
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    prepareStatement();
    // For each known SQL Type, call the appropriate
        // set routine

    switch (sqlType)
    {
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
            if(x.hasValue())
            {
                OUString sStr;
                x >>= sStr;
                setParameter(parameterIndex, sqlType, scale, sStr);
            }
            else
                setNull(parameterIndex,sqlType);
            break;
        case DataType::DECIMAL:
        case DataType::NUMERIC:
            if(x.hasValue())
            {
                ORowSetValue aValue;
                aValue.fill(x);
                // TODO: make sure that this calls the string overload
                setParameter(parameterIndex, sqlType, scale, aValue);
            }
            else
                setNull(parameterIndex,sqlType);
            break;
        default:
            ::dbtools::setObjectWithInfo(this,parameterIndex,x,sqlType,scale);
        }
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& /*typeName*/ ) throw(SQLException, RuntimeException)
{
    setNull(parameterIndex,sqlType);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    if (!::dbtools::implSetObject(this, parameterIndex, x))
    {   // there is no other setXXX call which can handle the value in x
        throw SQLException();
    }
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    setScalarParameter(parameterIndex, DataType::SMALLINT, 5, x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex, DataType::BINARY, x);
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    // LEM: It is quite unclear to me what the interface here is.
    // The XInputStream provides *bytes*, not characters.
    setStream(parameterIndex, x, length, DataType::LONGVARCHAR);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    setStream(parameterIndex, x, length, DataType::LONGVARBINARY);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    prepareStatement();
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    SQLRETURN nRet = N3SQLFreeStmt (m_aStatementHandle, SQL_RESET_PARAMS);
    nRet = N3SQLFreeStmt (m_aStatementHandle, SQL_UNBIND);
    OSL_UNUSED(nRet);
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::clearBatch(  ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XPreparedBatchExecution::clearBatch", *this );
    //  clearParameters(  );
    //  m_aBatchList.erase();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::addBatch( ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XPreparedBatchExecution::addBatch", *this );
}
// -------------------------------------------------------------------------

Sequence< sal_Int32 > SAL_CALL OPreparedStatement::executeBatch(  ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XPreparedBatchExecution::executeBatch", *this );
    // not reached, but keep -Werror happy
    return Sequence< sal_Int32 > ();
}
// -------------------------------------------------------------------------

//====================================================================
// methods
//====================================================================

//--------------------------------------------------------------------
// initBoundParam
// Initialize the bound parameter objects
//--------------------------------------------------------------------

void OPreparedStatement::initBoundParam () throw(SQLException)
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    // Get the number of parameters
    numParams = 0;
    N3SQLNumParams (m_aStatementHandle,&numParams);

    // There are parameter markers, allocate the bound
    // parameter objects

    if (numParams > 0)
    {
        // Allocate an array of bound parameter objects

        boundParams = new OBoundParam[numParams];

    }
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// allocBindBuf
// Allocate storage for the permanent data buffer for the bound
// parameter.
//--------------------------------------------------------------------

void* OPreparedStatement::allocBindBuf( sal_Int32 index,sal_Int32 bufLen)
{
    void* b = NULL;

    // Sanity check the parameter number

    if ((index >= 1) && (index <= numParams))
    {
        b = boundParams[index - 1].allocBindDataBuffer(bufLen);
    }

    return b;
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// getLengthBuf
// Gets the length buffer for the given parameter index
//--------------------------------------------------------------------

SQLLEN* OPreparedStatement::getLengthBuf (sal_Int32 index)
{
    SQLLEN* b = NULL;

    // Sanity check the parameter number

    if ((index >= 1) &&
        (index <= numParams))
    {
        b = boundParams[index - 1].getBindLengthBuffer ();
    }

    return b;
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// putParamData
// Puts parameter data from a previously bound input stream.  The
// input stream was bound using SQL_LEN_DATA_AT_EXEC.
//--------------------------------------------------------------------

void OPreparedStatement::putParamData (sal_Int32 index) throw(SQLException)
{
    // Sanity check the parameter index
    if ((index < 1) ||
        (index > numParams))
    {
        return;
    }

    // We'll transfer up to MAX_PUT_DATA_LENGTH at a time
    Sequence< sal_Int8 > buf( MAX_PUT_DATA_LENGTH );

    // Get the information about the input stream

    Reference< XInputStream> inputStream =  boundParams[index - 1].getInputStream ();
    if ( !inputStream.is() )
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceString(STR_NO_INPUTSTREAM));
        throw SQLException (sError, *this,OUString(),0,Any());
    }

    sal_Int32 maxBytesLeft = boundParams[index - 1].getInputStreamLen ();

    // Loop while more data from the input stream
    sal_Int32 haveRead = 0;
    try
    {

        do
        {
            sal_Int32 toReadThisRound = ::std::min( MAX_PUT_DATA_LENGTH, maxBytesLeft );

            // Read some data from the input stream
            haveRead = inputStream->readBytes( buf, toReadThisRound );
            OSL_ENSURE( haveRead == buf.getLength(), "OPreparedStatement::putParamData: inconsistency!" );

            if ( !haveRead )
                // no more data in the stream - the given stream length was a maximum which could not be
                // fulfilled by the stream
                break;

            // Put the data
            OSL_ENSURE( m_aStatementHandle, "OPreparedStatement::putParamData: StatementHandle is null!" );
            N3SQLPutData ( m_aStatementHandle, buf.getArray(), buf.getLength() );

            // decrement the number of bytes still needed
            maxBytesLeft -= haveRead;
        }
        while ( maxBytesLeft > 0 );
    }
    catch (const IOException& ex)
    {

        // If an I/O exception was generated, turn
        // it into a SQLException

        throw SQLException(ex.Message,*this,OUString(),0,Any());
    }
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// setStream
// Sets an input stream as a parameter, using the given SQL type
//--------------------------------------------------------------------

void OPreparedStatement::setStream(
                                    sal_Int32 ParameterIndex,
                                    const Reference< XInputStream>& x,
                                    SQLLEN length,
                                    sal_Int32 _nType)
                                    throw(SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    prepareStatement();

    checkParameterIndex(ParameterIndex);
    // Get the buffer needed for the length

    SQLLEN * const lenBuf = getLengthBuf(ParameterIndex);

    // Allocate a new buffer for the parameter data.  This buffer
    // will be returned by SQLParamData (it is set to the parameter
    // number, a sal_Int32)

    sal_Int32* dataBuf = static_cast<sal_Int32*>( allocBindBuf(ParameterIndex, sizeof(ParameterIndex)) );
    *dataBuf = ParameterIndex;

    // Bind the parameter with SQL_LEN_DATA_AT_EXEC
    *lenBuf = SQL_LEN_DATA_AT_EXEC (length);

    SQLSMALLINT fCType, fSqlType;
    OTools::getBindTypes(useWChar, m_pConnection->useOldDateFormat(), OTools::jdbcTypeToOdbc(_nType), fCType, fSqlType);


    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    N3SQLBindParameter(m_aStatementHandle,
                       static_cast<SQLUSMALLINT>(ParameterIndex),
                       SQL_PARAM_INPUT,
                       fCType,
                       fSqlType,
                       length,
                       invalid_scale,
                       dataBuf,
                       sizeof(ParameterIndex),
                       lenBuf);

    // Save the input stream
    boundParams[ParameterIndex - 1].setInputStream (x, length);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void OPreparedStatement::FreeParams()
{
    numParams = 0;
    delete [] boundParams;
    boundParams = NULL;
}
// -------------------------------------------------------------------------
void OPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    try
    {
        switch(nHandle)
        {
            case PROPERTY_ID_RESULTSETCONCURRENCY:
                if(!isPrepared())
                    setResultSetConcurrency(comphelper::getINT32(rValue));
                break;
            case PROPERTY_ID_RESULTSETTYPE:
                if(!isPrepared())
                    setResultSetType(comphelper::getINT32(rValue));
                break;
            case PROPERTY_ID_FETCHDIRECTION:
                if(!isPrepared())
                    setFetchDirection(comphelper::getINT32(rValue));
                break;
            case PROPERTY_ID_USEBOOKMARKS:
                if(!isPrepared())
                    setUsingBookmarks(comphelper::getBOOL(rValue));
                break;
            default:
                OStatement_Base::setFastPropertyValue_NoBroadcast(nHandle,rValue);
        }
    }
    catch(const SQLException&)
    {
        //  throw Exception(e.Message,*this);
    }
}
// -----------------------------------------------------------------------------
void OPreparedStatement::prepareStatement()
{
    if(!isPrepared())
    {
        OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
        OString aSql(OUStringToOString(m_sSqlStatement,getOwnConnection()->getTextEncoding()));
        SQLRETURN nReturn = N3SQLPrepare(m_aStatementHandle,(SDB_ODBC_CHAR *) aSql.getStr(),aSql.getLength());
        OTools::ThrowException(m_pConnection,nReturn,m_aStatementHandle,SQL_HANDLE_STMT,*this);
        m_bPrepared = sal_True;
        initBoundParam();
    }
}
// -----------------------------------------------------------------------------
void OPreparedStatement::checkParameterIndex(sal_Int32 _parameterIndex)
{
    if( _parameterIndex > numParams ||
        _parameterIndex < 1 ||
        _parameterIndex > std::numeric_limits<SQLUSMALLINT>::max() )
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceStringWithSubstitution(STR_WRONG_PARAM_INDEX,
            "$pos$", OUString::number(_parameterIndex),
            "$count$", OUString::number(numParams)
            ));
        SQLException aNext(sError,*this, OUString(),0,Any());

        ::dbtools::throwInvalidIndexException(*this,makeAny(aNext));
    }
}
// -----------------------------------------------------------------------------
OResultSet* OPreparedStatement::createResulSet()
{
    OResultSet* pReturn = new OResultSet(m_aStatementHandle,this);
    pReturn->setMetaData(getMetaData());
    return pReturn;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
