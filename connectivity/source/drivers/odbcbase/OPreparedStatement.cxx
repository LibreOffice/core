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
#include <comphelper/sequence.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include "connectivity/dbtools.hxx"
#include <comphelper/types.hxx>
#include "connectivity/FValue.hxx"
#include "resource/common_res.hrc"
#include "connectivity/sqlparse.hxx"

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


OPreparedStatement::OPreparedStatement( OConnection* _pConnection,const ::rtl::OUString& sql)
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
            OSQLParser aParser(_pConnection->getDriver()->getORB());
            ::rtl::OUString sErrorMessage;
            ::rtl::OUString sNewSql;
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

    // Now loop while more data is needed (i.e. a data-at-
    // execution parameter was given).  For each parameter
    // that needs data, put the data from the input stream.

    while (needData) {

        // Get the parameter number that requires data

        sal_Int32* paramIndex = 0;
        N3SQLParamData (m_aStatementHandle,(SQLPOINTER*)&paramIndex);

        // If the parameter index is -1, there is no more
        // data required

        if (*paramIndex == -1) {
            needData = sal_False;
        }
        else {
            // Now we have the proper parameter index,
            // get the data from the input stream
            // and do a SQLPutData
            putParamData(*paramIndex);
        }
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

void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    ::rtl::OString aString(::rtl::OUStringToOString(x,getOwnConnection()->getTextEncoding()));
    setParameter(parameterIndex,DataType::CHAR,aString.getLength(),(void*)&x);
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
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    sal_Int32 value = 0;

    // If the parameter is sal_True, set the value to 1
    if (x) {
        value = 1;
    }

    // Set the parameter as if it were an integer
    setInt (parameterIndex, value);
}
// -------------------------------------------------------------------------
void OPreparedStatement::setParameter(sal_Int32 parameterIndex,sal_Int32 _nType,sal_Int32 _nSize,void* _pData)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    prepareStatement();
    // Allocate a buffer to be used in binding.  This will be
        // a 'permanent' buffer that the bridge will fill in with
        // the bound data in native format.


    checkParameterIndex(parameterIndex);
    sal_Int32 nRealSize = _nSize;
    SQLSMALLINT fSqlType = static_cast<SQLSMALLINT>(OTools::jdbcTypeToOdbc(_nType));
    switch(fSqlType)
    {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_DECIMAL:
        case SQL_NUMERIC:
            ++nRealSize;
            break;
        case SQL_BINARY:
        case SQL_VARBINARY:
            nRealSize=1;    //dummy buffer, binary data isn't copied
            break;
        default:
            break;
    }

    sal_Int8* bindBuf = allocBindBuf(parameterIndex, nRealSize);

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    OTools::bindParameter(  m_pConnection,
                            m_aStatementHandle,
                            parameterIndex,
                            bindBuf,
                            getLengthBuf(parameterIndex),
                            fSqlType,
                            sal_False,
                            m_pConnection->useOldDateFormat(),
                            _pData,
                            (Reference <XInterface>)*this,
                            getOwnConnection()->getTextEncoding());
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,DataType::TINYINT,sizeof(sal_Int8),&x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const Date& aData ) throw(SQLException, RuntimeException)
{
    DATE_STRUCT x = OTools::DateToOdbcDate(aData);
    setParameter(parameterIndex,DataType::DATE,sizeof(DATE_STRUCT),&x);
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const Time& aVal ) throw(SQLException, RuntimeException)
{
    TIME_STRUCT x = OTools::TimeToOdbcTime(aVal);
    setParameter(parameterIndex,DataType::TIME,sizeof(TIME_STRUCT),&x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const DateTime& aVal ) throw(SQLException, RuntimeException)
{
    TIMESTAMP_STRUCT x = OTools::DateTimeToTimestamp(aVal);
    setParameter(parameterIndex,DataType::TIMESTAMP,sizeof(TIMESTAMP_STRUCT),&x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,DataType::DOUBLE,sizeof(double),&x);
}

// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,DataType::FLOAT,sizeof(float),&x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,DataType::INTEGER,sizeof(sal_Int32),&x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    try
    {
        setParameter(parameterIndex,DataType::BIGINT,sizeof(sal_Int64),&x);
    }
    catch(SQLException&)
    {
        setString(parameterIndex,ORowSetValue(x));
    }
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    prepareStatement();
    // Get the buffer needed for the length
    checkParameterIndex(parameterIndex);

    sal_Int8* lenBuf = getLengthBuf (parameterIndex);
    *(SQLLEN*)lenBuf = SQL_NULL_DATA;


    SQLLEN prec = 0;
    SQLULEN nColumnSize = 0;
    if (sqlType == SQL_CHAR || sqlType == SQL_VARCHAR || sqlType == SQL_LONGVARCHAR)
    {
        prec = 1;
        nColumnSize = 1;
    }

    SQLSMALLINT fCType = 0;
    SQLSMALLINT fSqlType = 0;

    SQLSMALLINT nDecimalDigits = 0;
    OTools::getBindTypes(   sal_False,
                            m_pConnection->useOldDateFormat(),
                            (SQLSMALLINT)sqlType,
                            fCType,
                            fSqlType);

    SQLRETURN nReturn = N3SQLBindParameter( m_aStatementHandle,
                                            (SQLUSMALLINT)parameterIndex,
                                            (SQLSMALLINT)SQL_PARAM_INPUT,
                                            fCType,
                                            fSqlType,
                                            nColumnSize,
                                            nDecimalDigits,
                                            NULL,
                                            prec,
                                            (SQLLEN*)lenBuf
                                            );
    OTools::ThrowException(m_pConnection,nReturn,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
    if ( x.is() )
        setStream(parameterIndex, x->getCharacterStream(), (SQLLEN)x->length(), DataType::LONGVARCHAR);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
    if ( x.is() )
        setStream(parameterIndex, x->getBinaryStream(), (SQLLEN)x->length(), DataType::LONGVARCHAR);
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
void OPreparedStatement::setDecimal( sal_Int32 parameterIndex, const ::rtl::OUString& x )
{
    ::rtl::OString aString(::rtl::OUStringToOString(x,getOwnConnection()->getTextEncoding()));
    setParameter(parameterIndex,DataType::DECIMAL,aString.getLength(),(void*)&x);
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
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
            if(x.hasValue())
            {
                ::rtl::OUString sStr;
                x >>= sStr;
                ::rtl::OString aString(::rtl::OUStringToOString(sStr,getOwnConnection()->getTextEncoding()));
                setParameter(parameterIndex,sqlType,aString.getLength(),&aString);
            }
            else
                setNull(parameterIndex,sqlType);
            break;
        case DataType::DECIMAL:
            {
                ORowSetValue aValue;
                aValue.fill(x);
                setDecimal(parameterIndex,aValue);
            }
            break;
        case DataType::NUMERIC:
            {
                ORowSetValue aValue;
                aValue.fill(x);
                setString(parameterIndex,aValue);
            }
            break;
        default:
            ::dbtools::setObjectWithInfo(this,parameterIndex,x,sqlType,scale);
        }
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& /*typeName*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

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
    setParameter(parameterIndex,DataType::SMALLINT,sizeof(sal_Int16),&x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,DataType::BINARY,x.getLength(),(void*)&x);
    boundParams[parameterIndex-1].setSequence(x); // this assures that the sequence stays alive
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
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
    prepareStatement();
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    SQLRETURN nRet = N3SQLFreeStmt (m_aStatementHandle, SQL_RESET_PARAMS);
    nRet = N3SQLFreeStmt (m_aStatementHandle, SQL_UNBIND);
    OSL_UNUSED(nRet);
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::clearBatch(  ) throw(SQLException, RuntimeException)
{
    //  clearParameters(  );
    //  m_aBatchList.erase();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::addBatch( ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

Sequence< sal_Int32 > SAL_CALL OPreparedStatement::executeBatch(  ) throw(SQLException, RuntimeException)
{
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

        // initialize each bound parameter

        for (sal_Int32 i = 0; i < numParams; i++)
        {
            boundParams[i].initialize ();
        }
    }
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// allocBindBuf
// Allocate storage for the permanent data buffer for the bound
// parameter.
//--------------------------------------------------------------------

sal_Int8* OPreparedStatement::allocBindBuf( sal_Int32 index,sal_Int32 bufLen)
{
    sal_Int8* b = NULL;

    // Sanity check the parameter number

    if ((index >= 1) &&
        (index <= numParams) && bufLen > 0 )
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

sal_Int8* OPreparedStatement::getLengthBuf (sal_Int32 index)
{
    sal_Int8* b = NULL;

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
        const ::rtl::OUString sError( aResources.getResourceString(STR_NO_INPUTSTREAM));
        throw SQLException (sError, *this,::rtl::OUString(),0,Any());
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

        throw SQLException(ex.Message,*this,::rtl::OUString(),0,Any());
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
                                    sal_Int32 SQLtype)
                                    throw(SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    prepareStatement();

    checkParameterIndex(ParameterIndex);
    // Get the buffer needed for the length

    sal_Int8* lenBuf = getLengthBuf(ParameterIndex);

    // Allocate a new buffer for the parameter data.  This buffer
    // will be returned by SQLParamData (it is set to the parameter
    // number, a 4-sal_Int8 integer)

    sal_Int8* dataBuf = allocBindBuf (ParameterIndex, 4);

    // Bind the parameter with SQL_LEN_DATA_AT_EXEC
    SQLSMALLINT   Ctype = SQL_C_CHAR;
    SQLLEN  atExec = SQL_LEN_DATA_AT_EXEC (length);
    memcpy (dataBuf, &ParameterIndex, sizeof(ParameterIndex));
    memcpy (lenBuf, &atExec, sizeof (atExec));

    if ((SQLtype == SQL_BINARY) || (SQLtype == SQL_VARBINARY) || (SQLtype == SQL_LONGVARBINARY))
        Ctype = SQL_C_BINARY;


    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    N3SQLBindParameter(m_aStatementHandle,
                        (SQLUSMALLINT)ParameterIndex,
                        (SQLUSMALLINT)SQL_PARAM_INPUT,
                        Ctype,
                        (SQLSMALLINT)SQLtype,
                        (SQLULEN)length,
                        0,
                        dataBuf,
                        sizeof(ParameterIndex),
                        (SQLLEN*)lenBuf);

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
        ::rtl::OString aSql(::rtl::OUStringToOString(m_sSqlStatement,getOwnConnection()->getTextEncoding()));
        SQLRETURN nReturn = N3SQLPrepare(m_aStatementHandle,(SDB_ODBC_CHAR *) aSql.getStr(),aSql.getLength());
        OTools::ThrowException(m_pConnection,nReturn,m_aStatementHandle,SQL_HANDLE_STMT,*this);
        m_bPrepared = sal_True;
        initBoundParam();
    }
}
// -----------------------------------------------------------------------------
void OPreparedStatement::checkParameterIndex(sal_Int32 _parameterIndex)
{
    if( !_parameterIndex || _parameterIndex > numParams)
    {
        ::connectivity::SharedResources aResources;
        const ::rtl::OUString sError( aResources.getResourceStringWithSubstitution(STR_WRONG_PARAM_INDEX,
            "$pos$", ::rtl::OUString::valueOf(_parameterIndex),
            "$count$", ::rtl::OUString::valueOf((sal_Int32)numParams)
            ));
        SQLException aNext(sError,*this, ::rtl::OUString(),0,Any());

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
