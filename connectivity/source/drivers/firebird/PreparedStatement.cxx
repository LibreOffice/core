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
{
    Any aRet = OStatementCommonBase::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPreparedStatement_Base::queryInterface(rType);
    return aRet;
}

uno::Sequence< Type > SAL_CALL OPreparedStatement::getTypes()
{
    return concatSequences(OPreparedStatement_Base::getTypes(),
                           OStatementCommonBase::getTypes());
}

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ensurePrepared();

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pConnection.get()
                                           , m_pOutSqlda);

    return m_xMetaData;
}

void SAL_CALL OPreparedStatement::close()
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
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    return Reference<XConnection>(m_pConnection.get());
}

sal_Bool SAL_CALL OPreparedStatement::execute()
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
        {
            // Do not throw error. Trying to close a closed cursor is not a
            // critical mistake.
            OUString sErrMsg  = StatusVectorToString(m_statusVector,
                    "isc_dsql_free_statement: close cursor");
            SAL_WARN("connectivity.firebird", sErrMsg);
        }
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
{
    execute();
    return getStatementChangeCount();
}

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery()
{
    execute();
    return m_xResultSet;
}

namespace {

/**
 * Take out the number part of a fix point decimal without
 * the information of where is the fractional part from a
 * string representation of a number. (e.g. 54.654 -> 54654)
 */
sal_Int64 toNumericWithoutDecimalPlace(const OUString& sSource)
{
    OUString sNumber(sSource);

    // cut off leading 0 eventually ( eg. 0.567 -> .567)
    (void)sSource.startsWith("0", &sNumber);

    sal_Int32 nDotIndex = sNumber.indexOf('.');

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
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ensurePrepared();

    setParameterNull(nIndex);
}

void SAL_CALL OPreparedStatement::setBoolean(sal_Int32 nIndex, sal_Bool bValue)
{
    setValue< sal_Bool >(nIndex, bValue, SQL_BOOLEAN);
}

template <typename T>
void OPreparedStatement::setValue(sal_Int32 nIndex, const T& nValue, ISC_SHORT nType)
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

void SAL_CALL OPreparedStatement::setByte(sal_Int32 nIndex, sal_Int8 nValue)
{
    // there's no TINYINT or equivalent on Firebird,
    // so do the same as setShort
    setValue< sal_Int16 >(nIndex, nValue, SQL_SHORT);
}

void SAL_CALL OPreparedStatement::setShort(sal_Int32 nIndex, sal_Int16 nValue)
{
    setValue< sal_Int16 >(nIndex, nValue, SQL_SHORT);
}

void SAL_CALL OPreparedStatement::setInt(sal_Int32 nIndex, sal_Int32 nValue)
{
    setValue< sal_Int32 >(nIndex, nValue, SQL_LONG);
}

void SAL_CALL OPreparedStatement::setLong(sal_Int32 nIndex, sal_Int64 nValue)
{
    setValue< sal_Int64 >(nIndex, nValue, SQL_INT64);
}

void SAL_CALL OPreparedStatement::setFloat(sal_Int32 nIndex, float nValue)
{
    setValue< float >(nIndex, nValue, SQL_FLOAT);
}

void SAL_CALL OPreparedStatement::setDouble(sal_Int32 nIndex, double nValue)
{
    setValue< double >(nIndex, nValue, SQL_DOUBLE); // TODO: SQL_D_FLOAT?
}

void SAL_CALL OPreparedStatement::setDate(sal_Int32 nIndex, const Date& rDate)
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

void SAL_CALL OPreparedStatement::setClob( sal_Int32, const Reference< XClob >& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}

void SAL_CALL OPreparedStatement::setBlob(sal_Int32 nParameterIndex,
                                          const Reference< XBlob >& xBlob)
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


void SAL_CALL OPreparedStatement::setArray( sal_Int32, const Reference< XArray >& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}


void SAL_CALL OPreparedStatement::setRef( sal_Int32, const Reference< XRef >& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}


void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale )
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
                n64Value = toNumericWithoutDecimalPlace(myString);
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


void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32, sal_Int32, const ::rtl::OUString& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}


void SAL_CALL OPreparedStatement::setObject( sal_Int32, const Any& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}

void SAL_CALL OPreparedStatement::setBytes(sal_Int32 nParameterIndex,
                                           const Sequence< sal_Int8 >& xBytes)
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


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32, const Reference< css::io::XInputStream >&, sal_Int32 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}


void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32, const Reference< css::io::XInputStream >&, sal_Int32 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}


void SAL_CALL OPreparedStatement::clearParameters(  )
{
}

// ---- Batch methods -- unsupported -----------------------------------------
void SAL_CALL OPreparedStatement::clearBatch()
{
    // Unsupported
}

void SAL_CALL OPreparedStatement::addBatch()
{
    // Unsupported by firebird
}

Sequence< sal_Int32 > SAL_CALL OPreparedStatement::executeBatch()
{
    // Unsupported by firebird
    return Sequence< sal_Int32 >();
}

void OPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
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
