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

#include "Connection.hxx"
#include "PreparedStatement.hxx"
#include "ResultSet.hxx"
#include "ResultSetMetaData.hxx"
#include "Util.hxx"

#include <comphelper/sequence.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>
#include <propertyids.hxx>
#include <time.h>
#include <connectivity/dbtools.hxx>

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

using namespace connectivity::firebird;

using namespace ::comphelper;
using namespace ::osl;

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbcx.firebird.PreparedStatement","com.sun.star.sdbc.PreparedStatement");


OPreparedStatement::OPreparedStatement( Connection* _pConnection,
                                        const OUString& sql)
    :OStatementCommonBase(_pConnection)
    ,m_sSqlStatement(sql)
    ,m_pOutSqlda(nullptr)
    ,m_pInSqlda(nullptr)
{
    SAL_INFO("connectivity.firebird", "OPreparedStatement(). "
             "sql: " << sql);
}

void OPreparedStatement::ensurePrepared()
    throw (SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    if (m_aStatementHandle)
        return;

    ISC_STATUS aErr = 0;

    if (!m_pInSqlda)
    {
        m_pInSqlda = static_cast<XSQLDA*>(calloc(1, XSQLDA_LENGTH(10)));
        m_pInSqlda->version = SQLDA_VERSION1;
        m_pInSqlda->sqln = 10;
    }

    prepareAndDescribeStatement(m_sSqlStatement,
                               m_pOutSqlda,
                               m_pInSqlda);

    aErr = isc_dsql_describe_bind(m_statusVector,
                                  &m_aStatementHandle,
                                  1,
                                  m_pInSqlda);

    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_describe_bind failed");
    }
    else if (m_pInSqlda->sqld > m_pInSqlda->sqln) // Not large enough
    {
        short nItems = m_pInSqlda->sqld;
        free(m_pInSqlda);
        m_pInSqlda = static_cast<XSQLDA*>(calloc(1, XSQLDA_LENGTH(nItems)));
        m_pInSqlda->version = SQLDA_VERSION1;
        m_pInSqlda->sqln = nItems;
        aErr = isc_dsql_describe_bind(m_statusVector,
                                      &m_aStatementHandle,
                                      1,
                                      m_pInSqlda);
        SAL_WARN_IF(aErr, "connectivity.firebird", "isc_dsql_describe_bind failed");
    }

    if (!aErr)
        mallocSQLVAR(m_pInSqlda);
    else
        evaluateStatusVector(m_statusVector, m_sSqlStatement, *this);
}

OPreparedStatement::~OPreparedStatement()
{
}

void SAL_CALL OPreparedStatement::acquire() throw()
{
    OStatementCommonBase::acquire();
}

void SAL_CALL OPreparedStatement::release() throw()
{
    OStatementCommonBase::release();
}

Any SAL_CALL OPreparedStatement::queryInterface(const Type& rType)
    throw(RuntimeException, std::exception)
{
    Any aRet = OStatementCommonBase::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPreparedStatement_Base::queryInterface(rType);
    return aRet;
}

uno::Sequence< Type > SAL_CALL OPreparedStatement::getTypes()
    throw(RuntimeException, std::exception)
{
    return concatSequences(OPreparedStatement_Base::getTypes(),
                           OStatementCommonBase::getTypes());
}

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData()
    throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ensurePrepared();

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pConnection.get()
                                           , m_pOutSqlda);

    return m_xMetaData;
}

void SAL_CALL OPreparedStatement::close() throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    OStatementCommonBase::close();
    if (m_pInSqlda)
    {
        freeSQLVAR(m_pInSqlda);
        free(m_pInSqlda);
        m_pInSqlda = nullptr;
    }
    if (m_pOutSqlda)
    {
        freeSQLVAR(m_pOutSqlda);
        free(m_pOutSqlda);
        m_pOutSqlda = nullptr;
    }
}

void SAL_CALL OPreparedStatement::disposing()
{
    close();
}

void SAL_CALL OPreparedStatement::setString(sal_Int32 nParameterIndex,
                                            const OUString& x)
    throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("connectivity.firebird",
             "setString(" << nParameterIndex << " , " << x << ")");

    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ensurePrepared();

    checkParameterIndex(nParameterIndex);
    setParameterNull(nParameterIndex, false);

    OString str = OUStringToOString(x , RTL_TEXTENCODING_UTF8 );

    XSQLVAR* pVar = m_pInSqlda->sqlvar + (nParameterIndex - 1);

    int dtype = (pVar->sqltype & ~1); // drop flag bit for now

    if (str.getLength() > pVar->sqllen)
        str = str.copy(0, pVar->sqllen);

    switch (dtype) {
    case SQL_VARYING:
    {
        const sal_Int32 max_varchar_len = 0xFFFF;
        // First 2 bytes indicate string size
        if (str.getLength() > max_varchar_len)
        {
            str = str.copy(0, max_varchar_len);
        }
        const short nLength = str.getLength();
        memcpy(pVar->sqldata, &nLength, 2);
        // Actual data
        memcpy(pVar->sqldata + 2, str.getStr(), str.getLength());
        break;
    }
    case SQL_TEXT:
        memcpy(pVar->sqldata, str.getStr(), str.getLength());
        // Fill remainder with spaces
        memset(pVar->sqldata + str.getLength(), ' ', pVar->sqllen - str.getLength());
        break;
    default:
        ::dbtools::throwSQLException(
            "Incorrect type for setString",
            ::dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE,
            *this);
    }
}

Reference< XConnection > SAL_CALL OPreparedStatement::getConnection()
    throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    return Reference<XConnection>(m_pConnection.get());
}

sal_Bool SAL_CALL OPreparedStatement::execute()
    throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("connectivity.firebird", "executeQuery(). "
        "Got called with sql: " <<  m_sSqlStatement);

    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    ensurePrepared();

    ISC_STATUS aErr;

    if (m_xResultSet.is()) // Checks whether we have already run the statement.
    {
        disposeResultSet();
        // Closes the cursor from the last run.
        // This doesn't actually free the statement -- using DSQL_close closes
        // the cursor and keeps the statement, using DSQL_drop frees the statement
        // (and associated cursors).
        aErr = isc_dsql_free_statement(m_statusVector,
                                       &m_aStatementHandle,
                                       DSQL_close);
        if (aErr)
            evaluateStatusVector(m_statusVector,
                                 "isc_dsql_free_statement: close cursor",
                                 *this);
    }

    aErr = isc_dsql_execute(m_statusVector,
                                &m_pConnection->getTransaction(),
                                &m_aStatementHandle,
                                1,
                                m_pInSqlda);
    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_execute failed" );
        evaluateStatusVector(m_statusVector, "isc_dsql_execute", *this);
    }

    m_xResultSet = new OResultSet(m_pConnection.get(),
                                  m_aMutex,
                                  uno::Reference< XInterface >(*this),
                                  m_aStatementHandle,
                                  m_pOutSqlda);

    if (getStatementChangeCount() > 0)
        m_pConnection->notifyDatabaseModified();

    return m_xResultSet.is();
    // TODO: implement handling of multiple ResultSets.
}

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate()
    throw(SQLException, RuntimeException, std::exception)
{
    execute();
    return getStatementChangeCount();
}

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery()
    throw(SQLException, RuntimeException, std::exception)
{
    execute();
    return m_xResultSet;
}

namespace {

/**
 * Take out the number part of a fix point decimal without
 * the information of where is the fracional part from a
 * string representation of a number. (e.g. 54.654 -> 54654)
 */
sal_Int64 toNumericWithoutDecimalPlace(const OUString& sSource)
{
    OUString sNumber(sSource);

    // cut off leading 0 eventually ( eg. 0.567 -> .567)
    (void)sSource.startsWith("0", &sNumber);

    sal_Int32 nDotIndex = sNumber.indexOf((sal_Unicode)'.');

    if( nDotIndex < 0)
    {
        return sNumber.toInt64(); // no dot -> it's an integer
    }
    else
    {
        // remove dot
        OUStringBuffer sBuffer(15);
        if(nDotIndex > 0)
        {
            sBuffer.append(sNumber.copy(0, nDotIndex));
        }
        sBuffer.append(sNumber.copy(nDotIndex + 1));
        return sBuffer.makeStringAndClear().toInt64();
    }
}

}

//----- XParameters -----------------------------------------------------------
void SAL_CALL OPreparedStatement::setNull(sal_Int32 nIndex, sal_Int32 /*nSqlType*/)
    throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ensurePrepared();

    setParameterNull(nIndex);
}

void SAL_CALL OPreparedStatement::setBoolean(sal_Int32 /*nIndex*/, sal_Bool /*bValue*/)
    throw(SQLException, RuntimeException, std::exception)
{
    // FIREBIRD3: will need to be implemented.
    ::dbtools::throwFunctionNotSupportedSQLException("XParameters::setBoolean", *this);
}

template <typename T>
void OPreparedStatement::setValue(sal_Int32 nIndex, T& nValue, ISC_SHORT nType)
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ensurePrepared();

    checkParameterIndex(nIndex);
    setParameterNull(nIndex, false);

    XSQLVAR* pVar = m_pInSqlda->sqlvar + (nIndex - 1);

    if ((pVar->sqltype & ~1) != nType)
    {
       ::dbtools::throwSQLException(
            "Incorrect type for setValue",
            ::dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE,
            *this);
    }

    memcpy(pVar->sqldata, &nValue, sizeof(nValue));
}

void SAL_CALL OPreparedStatement::setByte(sal_Int32 /*nIndex*/, sal_Int8 /*nValue*/)
    throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionNotSupportedSQLException("XParameters::setByte", *this);
}

void SAL_CALL OPreparedStatement::setShort(sal_Int32 nIndex, sal_Int16 nValue)
    throw(SQLException, RuntimeException, std::exception)
{
    setValue< sal_Int16 >(nIndex, nValue, SQL_SHORT);
}

void SAL_CALL OPreparedStatement::setInt(sal_Int32 nIndex, sal_Int32 nValue)
    throw(SQLException, RuntimeException, std::exception)
{
    setValue< sal_Int32 >(nIndex, nValue, SQL_LONG);
}

void SAL_CALL OPreparedStatement::setLong(sal_Int32 nIndex, sal_Int64 nValue)
    throw(SQLException, RuntimeException, std::exception)
{
    setValue< sal_Int64 >(nIndex, nValue, SQL_INT64);
}

void SAL_CALL OPreparedStatement::setFloat(sal_Int32 nIndex, float nValue)
    throw(SQLException, RuntimeException, std::exception)
{
    setValue< float >(nIndex, nValue, SQL_FLOAT);
}

void SAL_CALL OPreparedStatement::setDouble(sal_Int32 nIndex, double nValue)
    throw(SQLException, RuntimeException, std::exception)
{
    setValue< double >(nIndex, nValue, SQL_DOUBLE); // TODO: SQL_D_FLOAT?
}

void SAL_CALL OPreparedStatement::setDate(sal_Int32 nIndex, const Date& rDate)
    throw(SQLException, RuntimeException, std::exception)
{
    struct tm aCTime;
    aCTime.tm_mday = rDate.Day;
    aCTime.tm_mon = rDate.Month -1;
    aCTime.tm_year = rDate.Year -1900;

    ISC_DATE aISCDate;
    isc_encode_sql_date(&aCTime, &aISCDate);

    setValue< ISC_DATE >(nIndex, aISCDate, SQL_TYPE_DATE);
}

void SAL_CALL OPreparedStatement::setTime( sal_Int32 nIndex, const css::util::Time& rTime)
    throw(SQLException, RuntimeException, std::exception)
{
    struct tm aCTime;
    aCTime.tm_sec = rTime.Seconds;
    aCTime.tm_min = rTime.Minutes;
    aCTime.tm_hour = rTime.Hours;

    ISC_TIME aISCTime;
    isc_encode_sql_time(&aCTime, &aISCTime);

    setValue< ISC_TIME >(nIndex, aISCTime, SQL_TYPE_TIME);
}

void SAL_CALL OPreparedStatement::setTimestamp(sal_Int32 nIndex, const DateTime& rTimestamp)
    throw(SQLException, RuntimeException, std::exception)
{
    struct tm aCTime;
    aCTime.tm_sec = rTimestamp.Seconds;
    aCTime.tm_min = rTimestamp.Minutes;
    aCTime.tm_hour = rTimestamp.Hours;
    aCTime.tm_mday = rTimestamp.Day;
    aCTime.tm_mon = rTimestamp.Month - 1;
    aCTime.tm_year = rTimestamp.Year - 1900;

    ISC_TIMESTAMP aISCTimestamp;
    isc_encode_timestamp(&aCTime, &aISCTimestamp);

    setValue< ISC_TIMESTAMP >(nIndex, aISCTimestamp, SQL_TIMESTAMP);
}


// void OPreaparedStatement::set
void OPreparedStatement::openBlobForWriting(isc_blob_handle& rBlobHandle, ISC_QUAD& rBlobId)
{
    ISC_STATUS aErr;

    aErr = isc_create_blob2(m_statusVector,
                            &m_pConnection->getDBHandle(),
                            &m_pConnection->getTransaction(),
                            &rBlobHandle,
                            &rBlobId,
                            0, // Blob parameter buffer length
                            nullptr); // Blob parameter buffer handle

    if (aErr)
    {
        evaluateStatusVector(m_statusVector,
                             "setBlob failed on " + m_sSqlStatement,
                             *this);
        assert(false);
    }
}

void OPreparedStatement::closeBlobAfterWriting(isc_blob_handle& rBlobHandle)
{
    ISC_STATUS aErr;

    aErr = isc_close_blob(m_statusVector,
                          &rBlobHandle);
    if (aErr)
    {
        evaluateStatusVector(m_statusVector,
                             "isc_close_blob failed",
                             *this);
        assert(false);
    }
}

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException, std::exception)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}

void SAL_CALL OPreparedStatement::setBlob(sal_Int32 nParameterIndex,
                                          const Reference< XBlob >& xBlob)
    throw (SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

#if SAL_TYPES_SIZEOFPOINTER == 8
    isc_blob_handle aBlobHandle = 0;
#else
    isc_blob_handle aBlobHandle = nullptr;
#endif
    ISC_QUAD aBlobId;

    openBlobForWriting(aBlobHandle, aBlobId);

    // Max segment size is 2^16 == SAL_MAX_UINT16
    // LEM TODO: SAL_MAX_UINT16 is 2^16-1; this mixup is probably innocuous; to be checked
    sal_uInt64 nDataWritten = 0;
    ISC_STATUS aErr = 0;
    while (xBlob->length() - nDataWritten > 0)
    {
        sal_uInt64 nDataRemaining = xBlob->length() - nDataWritten;
        sal_uInt16 nWriteSize = (nDataRemaining > SAL_MAX_UINT16) ? SAL_MAX_UINT16 : nDataRemaining;
        aErr = isc_put_segment(m_statusVector,
                               &aBlobHandle,
                               nWriteSize,
                               reinterpret_cast<const char*>(xBlob->getBytes(nDataWritten, nWriteSize).getConstArray()));
        nDataWritten += nWriteSize;


        if (aErr)
            break;

    }

    // We need to make sure we close the Blob even if their are errors, hence evaluate
    // errors after closing.
    closeBlobAfterWriting(aBlobHandle);

    if (aErr)
    {
        evaluateStatusVector(m_statusVector,
                             "isc_put_segment failed",
                             *this);
        assert(false);
    }

    setValue< ISC_QUAD >(nParameterIndex, aBlobId, SQL_BLOB);
}


void SAL_CALL OPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x ) throw(SQLException, RuntimeException, std::exception)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}


void SAL_CALL OPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x ) throw(SQLException, RuntimeException, std::exception)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}


void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException, std::exception)
{
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    ensurePrepared();

    checkParameterIndex(parameterIndex);
    setParameterNull(parameterIndex, false);

    XSQLVAR* pVar = m_pInSqlda->sqlvar + (parameterIndex - 1);
    int dType = (pVar->sqltype & ~1); // drop null flag

    if(sqlType == DataType::DECIMAL || sqlType == DataType::NUMERIC)
    {
        double myDouble=0.0;
        OUString myString;
        if( x >>= myDouble )
        {
            myString = OUString::number( myDouble );
        }
        else
        {
            x >>= myString;
        }

        // fill in the number with nulls in fractional part.
        // We need this because  e.g. 0.450 != 0.045 despite
        // their scale is equal
        OUStringBuffer sBuffer(15);
        sBuffer.append(myString);
        if(myString.indexOf('.') != -1) // there is a dot
        {
            for(sal_Int32 i=myString.copy(myString.indexOf('.')+1).getLength(); i<scale;i++)
            {
                sBuffer.append('0');
            }
        }
        else
        {
            for (sal_Int32 i=0; i<scale; i++)
            {
                sBuffer.append('0');
            }
        }
        myString = sBuffer.makeStringAndClear();
        // set value depending on type
        sal_Int16 n16Value = 0;
        sal_Int32 n32Value = 0;
        sal_Int64 n64Value = 0;
        switch(dType)
        {
            case SQL_SHORT:
                n16Value = (sal_Int16) toNumericWithoutDecimalPlace(myString);
                setValue< sal_Int16 >(parameterIndex,
                        n16Value,
                        dType);
                break;
            case SQL_LONG:
            case SQL_DOUBLE: // TODO FIXME 32 bits
                n32Value = (sal_Int32) toNumericWithoutDecimalPlace(myString);
                setValue< sal_Int32 >(parameterIndex,
                        n32Value,
                        dType);
                break;
            case SQL_INT64:
                n64Value = (sal_Int64) toNumericWithoutDecimalPlace(myString);
                setValue< sal_Int64 >(parameterIndex,
                        n64Value,
                        dType);
                break;
            default:
                SAL_WARN("connectivity.firebird",
                        "No Firebird sql type found for numeric or decimal types");
                ::dbtools::setObjectWithInfo(this,parameterIndex,x,sqlType,scale);
        }
    }
    else
    {
        ::dbtools::setObjectWithInfo(this,parameterIndex,x,sqlType,scale);
    }

}


void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException, std::exception)
{
    (void) parameterIndex;
    (void) sqlType;
    (void) typeName;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}


void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException, std::exception)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}

void SAL_CALL OPreparedStatement::setBytes(sal_Int32 nParameterIndex,
                                           const Sequence< sal_Int8 >& xBytes)
    throw (SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

#if SAL_TYPES_SIZEOFPOINTER == 8
    isc_blob_handle aBlobHandle = 0;
#else
    isc_blob_handle aBlobHandle = nullptr;
#endif
    ISC_QUAD aBlobId;

    openBlobForWriting(aBlobHandle, aBlobId);

    // Max segment size is 2^16 == SAL_MAX_UINT16
    sal_uInt64 nDataWritten = 0;
    ISC_STATUS aErr = 0;
    while (xBytes.getLength() - nDataWritten > 0)
    {
        sal_uInt64 nDataRemaining = xBytes.getLength() - nDataWritten;
        sal_uInt16 nWriteSize = (nDataRemaining > SAL_MAX_UINT16) ? SAL_MAX_UINT16 : nDataRemaining;
        aErr = isc_put_segment(m_statusVector,
                               &aBlobHandle,
                               nWriteSize,
                               reinterpret_cast<const char*>(xBytes.getConstArray()) + nDataWritten);
        nDataWritten += nWriteSize;

        if (aErr)
            break;
    }

    // We need to make sure we close the Blob even if their are errors, hence evaluate
    // errors after closing.
    closeBlobAfterWriting(aBlobHandle);

    if (aErr)
    {
        evaluateStatusVector(m_statusVector,
                             "isc_put_segment failed",
                             *this);
        assert(false);
    }

    setValue< ISC_QUAD >(nParameterIndex, aBlobId, SQL_BLOB);
}


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException, std::exception)
{
    (void) parameterIndex;
    (void) x;
    (void) length;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}


void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException, std::exception)
{
    (void) parameterIndex;
    (void) x;
    (void) length;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}


void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException, std::exception)
{
}

// ---- Batch methods -- unsupported -----------------------------------------
void SAL_CALL OPreparedStatement::clearBatch()
    throw(SQLException, RuntimeException, std::exception)
{
    // Unsupported
}

void SAL_CALL OPreparedStatement::addBatch()
    throw(SQLException, RuntimeException, std::exception)
{
    // Unsupported by firebird
}

Sequence< sal_Int32 > SAL_CALL OPreparedStatement::executeBatch()
    throw(SQLException, RuntimeException, std::exception)
{
    // Unsupported by firebird
    return Sequence< sal_Int32 >();
}

void OPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception, std::exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            break;
        default:
            OStatementCommonBase::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }
}

void OPreparedStatement::checkParameterIndex(sal_Int32 nParameterIndex)
    throw(SQLException, RuntimeException)
{
    ensurePrepared();
    if ((nParameterIndex == 0) || (nParameterIndex > m_pInSqlda->sqld))
    {
        ::dbtools::throwSQLException(
            "No column " + OUString::number(nParameterIndex),
            ::dbtools::StandardSQLState::COLUMN_NOT_FOUND,
            *this);
    }
}

void OPreparedStatement::setParameterNull(sal_Int32 nParameterIndex,
                                          bool bSetNull)
{
    XSQLVAR* pVar = m_pInSqlda->sqlvar + (nParameterIndex - 1);
    if (bSetNull)
    {
        pVar->sqltype |= 1;
        *pVar->sqlind = -1;
    }
    else
        *pVar->sqlind = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
