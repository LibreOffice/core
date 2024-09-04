/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>
#include <cmath>

#include "Connection.hxx"
#include "PreparedStatement.hxx"
#include "ResultSet.hxx"
#include "ResultSetMetaData.hxx"
#include "Util.hxx"

#include <comphelper/sequence.hxx>
#include <connectivity/dbexception.hxx>
#include <propertyids.hxx>
#include <connectivity/dbtools.hxx>
#include <sal/log.hxx>

#include <com/sun/star/sdbc/DataType.hpp>

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

IMPLEMENT_SERVICE_INFO(OPreparedStatement,u"com.sun.star.sdbcx.firebird.PreparedStatement"_ustr,u"com.sun.star.sdbc.PreparedStatement"_ustr);

constexpr size_t MAX_SIZE_SEGMENT = 65535; // max value of a segment of CLOB, if we want more than 65535 bytes, we need more segments


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
        assert(m_pInSqlda && "Don't handle OOM conditions");
        m_pInSqlda->version = SQLDA_VERSION1;
        m_pInSqlda->sqln = 10;
    }

    prepareAndDescribeStatement(m_sSqlStatement, m_pOutSqlda);

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
        assert(m_pInSqlda && "Don't handle OOM conditions");
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

void SAL_CALL OPreparedStatement::acquire() noexcept
{
    OStatementCommonBase::acquire();
}

void SAL_CALL OPreparedStatement::release() noexcept
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
                                            const OUString& sInput)
{
    SAL_INFO("connectivity.firebird",
             "setString(" << nParameterIndex << " , " << sInput << ")");

    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ensurePrepared();

    checkParameterIndex(nParameterIndex);
    setParameterNull(nParameterIndex, false);

    XSQLVAR* pVar = m_pInSqlda->sqlvar + (nParameterIndex - 1);
    ColumnTypeInfo columnType(*pVar);

    switch (auto sdbcType = columnType.getSdbcType()) {
    case DataType::VARCHAR:
    case DataType::CHAR:
    {
        OString str = OUStringToOString(sInput, RTL_TEXTENCODING_UTF8);
        const ISC_SHORT nLength = std::min(str.getLength(), static_cast<sal_Int32>(pVar->sqllen));
        int offset = 0;
        if (sdbcType == DataType::VARCHAR)
        {
            // First 2 bytes indicate string size
            static_assert(sizeof(nLength) == 2, "must match dest memcpy len");
            memcpy(pVar->sqldata, &nLength, 2);
            offset = 2;
        }
        // Actual data
        memcpy(pVar->sqldata + offset, str.getStr(), nLength);
        if (sdbcType == DataType::CHAR)
        {
            // Fill remainder with spaces
            memset(pVar->sqldata + offset + nLength, ' ', pVar->sqllen - nLength);
        }
        break;
    }
    case DataType::CLOB:
        setClob(nParameterIndex, sInput );
        break;
    case DataType::SMALLINT:
    {
        sal_Int32 int32Value = sInput.toInt32();
        if ( (int32Value < std::numeric_limits<sal_Int16>::min()) ||
             (int32Value > std::numeric_limits<sal_Int16>::max()) )
        {
            ::dbtools::throwSQLException(
                u"Value out of range for SQL_SHORT type"_ustr,
                ::dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE,
                *this);
        }
        setShort(nParameterIndex, int32Value);
        break;
    }
    case DataType::INTEGER:
    {
        sal_Int32 int32Value = sInput.toInt32();
        setInt(nParameterIndex, int32Value);
        break;
    }
    case DataType::BIGINT:
    {
        sal_Int64 int64Value = sInput.toInt64();
        setLong(nParameterIndex, int64Value);
        break;
    }
    case DataType::FLOAT:
    {
        float floatValue = sInput.toFloat();
        setFloat(nParameterIndex, floatValue);
        break;
    }
    case DataType::DOUBLE:
        setDouble(nParameterIndex, sInput.toDouble());
        break;
    case DataType::NUMERIC:
    case DataType::DECIMAL:
        return setObjectWithInfo(nParameterIndex, Any{ sInput }, sdbcType, 0);
        break;
    case DataType::BOOLEAN:
    {
        bool boolValue = sInput.toBoolean();
        setBoolean(nParameterIndex, boolValue);
        break;
    }
    case DataType::SQLNULL:
    {
        // See https://www.firebirdsql.org/file/documentation/html/en/refdocs/fblangref25/firebird-25-language-reference.html#fblangref25-datatypes-special-sqlnull
        pVar->sqldata = nullptr;
        break;
    }
    default:
        ::dbtools::throwSQLException(
            u"Incorrect type for setString"_ustr,
            ::dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE,
            *this);
    }
}

Reference< XConnection > SAL_CALL OPreparedStatement::getConnection()
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    return m_pConnection;
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
                    u"isc_dsql_free_statement: close cursor");
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
        evaluateStatusVector(m_statusVector, u"isc_dsql_execute", *this);
    }

    m_xResultSet = new OResultSet(m_pConnection.get(),
                                  m_aMutex,
                                  uno::Reference< XInterface >(*this),
                                  m_aStatementHandle,
                                  m_pOutSqlda);

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

sal_Int64 toPowOf10AndRound(double n, int powOf10)
{
    static constexpr sal_Int64 powers[] = {
        1,
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
        1000000000,
        10000000000,
        100000000000,
        1000000000000,
        10000000000000,
        100000000000000,
        1000000000000000,
        10000000000000000,
        100000000000000000,
        1000000000000000000,
    };
    powOf10 = std::clamp(powOf10, 0, int(std::size(powers) - 1));
    return n * powers[powOf10] + (n >= 0 ? 0.5 : -0.5);
}

/**
 * Take out the number part of a fix point decimal without
 * the information of where is the fractional part from a
 * string representation of a number. (e.g. 54.654 -> 54654)
 */
sal_Int64 toNumericWithoutDecimalPlace(const Any& x, sal_Int32 scale)
{
    if (double value = 0; x >>= value)
        return toPowOf10AndRound(value, scale);

    // Can't use conversion of string to double, because it could be not representable in double

    OUString s;
    x >>= s;
    std::u16string_view num(o3tl::trim(s));
    size_t end = num.starts_with('-') ? 1 : 0;
    for (bool seenDot = false; end < num.size(); ++end)
    {
        if (num[end] == '.')
        {
            if (seenDot)
                break;
            seenDot = true;
        }
        else if (!rtl::isAsciiDigit(num[end]))
            break;
    }
    num = num.substr(0, end);

    // fill in the number with nulls in fractional part.
    // We need this because  e.g. 0.450 != 0.045 despite
    // their scale is equal
    OUStringBuffer buffer(num);
    if (auto dotPos = num.find('.'); dotPos != std::u16string_view::npos) // there is a dot
    {
        scale -= num.substr(dotPos + 1).size();
        buffer.remove(dotPos, 1);
        if (scale < 0)
        {
            const sal_Int32 n = std::min(buffer.getLength(), -scale);
            buffer.truncate(buffer.getLength() - n);
            scale = 0;
        }
    }
    for (sal_Int32 i = 0; i < scale; ++i)
        buffer.append('0');

    return OUString::unacquired(buffer).toInt64();
}

double toDouble(const Any& x)
{
    if (double value = 0; x >>= value)
        return value;
    OUString s;
    x >>= s;
    return s.toDouble();
}
}

//----- XParameters -----------------------------------------------------------
void SAL_CALL OPreparedStatement::setNull(sal_Int32 nIndex, sal_Int32 /*nSqlType*/)
{
    MutexGuard aGuard( m_aMutex );
    ensurePrepared();

    checkParameterIndex(nIndex);
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
    ensurePrepared();

    checkParameterIndex(nIndex);
    setParameterNull(nIndex, false);

    XSQLVAR* pVar = m_pInSqlda->sqlvar + (nIndex - 1);

    if ((pVar->sqltype & ~1) != nType)
    {
       ::dbtools::throwSQLException(
            u"Incorrect type for setValue"_ustr,
            ::dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE,
            *this);
    }

    memcpy(pVar->sqldata, &nValue, sizeof(nValue));
}

// Integral type setters convert transparently to bigger types

void SAL_CALL OPreparedStatement::setByte(sal_Int32 nIndex, sal_Int8 nValue)
{
    // there's no TINYINT or equivalent on Firebird,
    // so do the same as setShort
    setShort(nIndex, nValue);
}

void SAL_CALL OPreparedStatement::setShort(sal_Int32 nIndex, sal_Int16 nValue)
{
    MutexGuard aGuard(m_aMutex);
    ensurePrepared();

    ColumnTypeInfo columnType{ m_pInSqlda, nIndex };
    switch (columnType.getSdbcType())
    {
        case DataType::INTEGER:
            return setValue<sal_Int32>(nIndex, nValue, columnType.getType());
        case DataType::BIGINT:
            return setValue<sal_Int64>(nIndex, nValue, columnType.getType());
        case DataType::FLOAT:
            return setValue<float>(nIndex, nValue, columnType.getType());
        case DataType::DOUBLE:
            return setValue<double>(nIndex, nValue, columnType.getType());
    }
    setValue< sal_Int16 >(nIndex, nValue, SQL_SHORT);
}

void SAL_CALL OPreparedStatement::setInt(sal_Int32 nIndex, sal_Int32 nValue)
{
    MutexGuard aGuard(m_aMutex);
    ensurePrepared();

    ColumnTypeInfo columnType{ m_pInSqlda, nIndex };
    switch (columnType.getSdbcType())
    {
        case DataType::BIGINT:
            return setValue<sal_Int64>(nIndex, nValue, columnType.getType());
        case DataType::DOUBLE:
            return setValue<double>(nIndex, nValue, columnType.getType());
    }
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
    MutexGuard aGuard( m_aMutex );
    ensurePrepared();

    ColumnTypeInfo columnType{ m_pInSqlda, nIndex };
    // Assume it is a sub type of a number.
    if (columnType.getSubType() < 0 || columnType.getSubType() > 2)
    {
        ::dbtools::throwSQLException(
            u"Incorrect number sub type"_ustr,
            ::dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE,
            *this);
    }

    // Caller might try to set an integer type here. It makes sense to convert
    // it instead of throwing an error.
    switch(auto sdbcType = columnType.getSdbcType())
    {
        case DataType::SMALLINT:
            return setValue(nIndex, static_cast<sal_Int16>(nValue), columnType.getType());
        case DataType::INTEGER:
            return setValue(nIndex, static_cast<sal_Int32>(nValue), columnType.getType());
        case DataType::BIGINT:
            return setValue(nIndex, static_cast<sal_Int64>(nValue), columnType.getType());
        case DataType::NUMERIC:
        case DataType::DECIMAL:
            return setObjectWithInfo(nIndex, Any{ nValue }, sdbcType, 0);
        // TODO: SQL_D_FLOAT?
    }
    setValue<double>(nIndex, nValue, SQL_DOUBLE);
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

    // Here we "know" that ISC_TIME is simply in units of seconds/ISC_TIME_SECONDS_PRECISION with no
    // other funkiness, so we can simply add the fraction of a second.
    aISCTime += rTime.NanoSeconds / (1000000000 / ISC_TIME_SECONDS_PRECISION);

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

    // As in previous function
    aISCTimestamp.timestamp_time += rTimestamp.NanoSeconds / (1000000000 / ISC_TIME_SECONDS_PRECISION);

    setValue< ISC_TIMESTAMP >(nIndex, aISCTimestamp, SQL_TIMESTAMP);
}


// void OPreparedStatement::set
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
                             Concat2View("setBlob failed on " + m_sSqlStatement),
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
                u"isc_close_blob failed",
                *this);
        assert(false);
    }
}

void SAL_CALL OPreparedStatement::setClob(sal_Int32 nParameterIndex, const Reference< XClob >& xClob )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    // value-initialization: isc_blob_handle may be either a pointer of an integer
    isc_blob_handle aBlobHandle{};
    ISC_QUAD aBlobId;

    openBlobForWriting(aBlobHandle, aBlobId);


    // Max segment size is 2^16 == SAL_MAX_UINT16
    // SAL_MAX_UINT16 / 4 is surely enough for UTF-8
    // TODO apply max segment size to character encoding
    sal_Int64 nCharWritten = 1; // XClob is indexed from 1
    ISC_STATUS aErr = 0;
    sal_Int64 nLen = xClob->length();
    while ( nLen >= nCharWritten )
    {
        sal_Int64 nCharRemain = nLen - nCharWritten + 1;
        constexpr sal_uInt16 MAX_SIZE = SAL_MAX_UINT16 / 4;
        sal_uInt16 nWriteSize = std::min<sal_Int64>(nCharRemain, MAX_SIZE);
        OString sData = OUStringToOString(
                xClob->getSubString(nCharWritten, nWriteSize),
                RTL_TEXTENCODING_UTF8);
        aErr = isc_put_segment( m_statusVector,
                &aBlobHandle,
                sData.getLength(),
                sData.getStr() );
        nCharWritten += nWriteSize;

        if (aErr)
            break;
    }

    // We need to make sure we close the Blob even if there are errors, hence evaluate
    // errors after closing.
    closeBlobAfterWriting(aBlobHandle);

    if (aErr)
    {
        evaluateStatusVector(m_statusVector,
                u"isc_put_segment failed",
                *this);
        assert(false);
    }

    setValue< ISC_QUAD >(nParameterIndex, aBlobId, SQL_BLOB);
}

void OPreparedStatement::setClob(sal_Int32 nParameterIndex, std::u16string_view rStr)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    checkParameterIndex(nParameterIndex);

    // value-initialization: isc_blob_handle may be either a pointer of an integer
    isc_blob_handle aBlobHandle{};
    ISC_QUAD aBlobId;

    openBlobForWriting(aBlobHandle, aBlobId);

    OString sData = OUStringToOString(
            rStr,
            RTL_TEXTENCODING_UTF8);
    size_t nDataSize = sData.getLength();
    ISC_STATUS aErr = 0;
    // we can't store  more than MAX_SIZE_SEGMENT in a segment
    if (nDataSize <= MAX_SIZE_SEGMENT)
    {
        aErr = isc_put_segment( m_statusVector,
                            &aBlobHandle,
                            sData.getLength(),
                            sData.getStr() );
    }
    else
    {
        // if we need more, let's split the input and first let's calculate the nb of entire chunks needed
        size_t nNbEntireChunks = nDataSize / MAX_SIZE_SEGMENT;
        for (size_t i = 0; i < nNbEntireChunks; ++i)
        {
            OString strCurrentChunk = sData.copy(i * MAX_SIZE_SEGMENT, MAX_SIZE_SEGMENT);
            aErr = isc_put_segment( m_statusVector,
                            &aBlobHandle,
                            strCurrentChunk.getLength(),
                            strCurrentChunk.getStr() );
            if (aErr)
                break;
        }
        size_t nRemainingBytes = nDataSize - (nNbEntireChunks * MAX_SIZE_SEGMENT);
        if (nRemainingBytes && !aErr)
        {
            // then copy the remaining
            OString strCurrentChunk = sData.copy(nNbEntireChunks * MAX_SIZE_SEGMENT, nRemainingBytes);
            aErr = isc_put_segment( m_statusVector,
                            &aBlobHandle,
                            strCurrentChunk.getLength(),
                            strCurrentChunk.getStr() );
        }
    }

    // We need to make sure we close the Blob even if there are errors, hence evaluate
    // errors after closing.
    closeBlobAfterWriting(aBlobHandle);

    if (aErr)
    {
        evaluateStatusVector(m_statusVector,
                             u"isc_put_segment failed",
                             *this);
        assert(false);
    }

    setValue< ISC_QUAD >(nParameterIndex, aBlobId, SQL_BLOB);
}

void SAL_CALL OPreparedStatement::setBlob(sal_Int32 nParameterIndex,
                                          const Reference< XBlob >& xBlob)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    checkParameterIndex(nParameterIndex);

    // value-initialization: isc_blob_handle may be either a pointer of an integer
    isc_blob_handle aBlobHandle{};
    ISC_QUAD aBlobId;

    openBlobForWriting(aBlobHandle, aBlobId);

    ISC_STATUS aErr = 0;
    const sal_Int64 nBlobLen = xBlob->length();
    if (nBlobLen > 0)
    {
        // Max write size is 0xFFFF == SAL_MAX_UINT16
        sal_uInt64 nDataWritten = 0;
        while (sal::static_int_cast<sal_uInt64>(nBlobLen) > nDataWritten)
        {
            sal_uInt64 nDataRemaining = nBlobLen - nDataWritten;
            sal_uInt16 nWriteSize = std::min(nDataRemaining, sal_uInt64(SAL_MAX_UINT16));
            aErr = isc_put_segment(m_statusVector,
                                   &aBlobHandle,
                                   nWriteSize,
                                   reinterpret_cast<const char*>(xBlob->getBytes(nDataWritten, nWriteSize).getConstArray()));
            nDataWritten += nWriteSize;

            if (aErr)
                break;
        }
    }

    // We need to make sure we close the Blob even if there are errors, hence evaluate
    // errors after closing.
    closeBlobAfterWriting(aBlobHandle);

    if (aErr)
    {
        evaluateStatusVector(m_statusVector,
                             u"isc_put_segment failed",
                             *this);
        assert(false);
    }

    setValue< ISC_QUAD >(nParameterIndex, aBlobId, SQL_BLOB);
}


void SAL_CALL OPreparedStatement::setArray( sal_Int32 nIndex, const Reference< XArray >& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    checkParameterIndex(nIndex);
}


void SAL_CALL OPreparedStatement::setRef( sal_Int32 nIndex, const Reference< XRef >& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    checkParameterIndex(nIndex);
}


void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale )
{
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    ensurePrepared();

    checkParameterIndex(parameterIndex);
    setParameterNull(parameterIndex, false);

    ColumnTypeInfo columnType{ m_pInSqlda, parameterIndex };
    int dType = columnType.getType() & ~1; // drop null flag

    if(sqlType == DataType::DECIMAL || sqlType == DataType::NUMERIC)
    {
        switch(dType)
        {
            case SQL_SHORT:
                return setValue(
                    parameterIndex,
                    static_cast<sal_Int16>(toNumericWithoutDecimalPlace(x, columnType.getScale())),
                    dType);
            case SQL_LONG:
                return setValue(
                    parameterIndex,
                    static_cast<sal_Int32>(toNumericWithoutDecimalPlace(x, columnType.getScale())),
                    dType);
            case SQL_INT64:
                return setValue(parameterIndex,
                                toNumericWithoutDecimalPlace(x, columnType.getScale()), dType);
            case SQL_DOUBLE:
                return setValue(parameterIndex, toDouble(x), dType);
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


void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 nIndex, sal_Int32, const OUString& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    checkParameterIndex(nIndex);
}


void SAL_CALL OPreparedStatement::setObject( sal_Int32 nIndex, const Any& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    checkParameterIndex(nIndex);
}

void SAL_CALL OPreparedStatement::setBytes(sal_Int32 nParameterIndex,
                                           const Sequence< sal_Int8 >& xBytes)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    checkParameterIndex(nParameterIndex);

    XSQLVAR* pVar = m_pInSqlda->sqlvar + (nParameterIndex - 1);
    int dType = (pVar->sqltype & ~1); // drop flag bit for now

    if( dType == SQL_BLOB )
    {
        // value-initialization: isc_blob_handle may be either a pointer of an integer
        isc_blob_handle aBlobHandle{};
        ISC_QUAD aBlobId;

        openBlobForWriting(aBlobHandle, aBlobId);

        ISC_STATUS aErr = 0;
        const sal_Int32 nBytesLen = xBytes.getLength();
        if (nBytesLen > 0)
        {
            // Max write size is 0xFFFF == SAL_MAX_UINT16
            sal_uInt32 nDataWritten = 0;
            while (sal::static_int_cast<sal_uInt32>(nBytesLen) > nDataWritten)
            {
                sal_uInt32 nDataRemaining = nBytesLen - nDataWritten;
                sal_uInt16 nWriteSize = std::min(nDataRemaining, sal_uInt32(SAL_MAX_UINT16));
                aErr = isc_put_segment(m_statusVector,
                                       &aBlobHandle,
                                       nWriteSize,
                                       reinterpret_cast<const char*>(xBytes.getConstArray()) + nDataWritten);
                nDataWritten += nWriteSize;

                if (aErr)
                    break;
            }
        }

        // We need to make sure we close the Blob even if there are errors, hence evaluate
        // errors after closing.
        closeBlobAfterWriting(aBlobHandle);

        if (aErr)
        {
            evaluateStatusVector(m_statusVector,
                                 u"isc_put_segment failed",
                                 *this);
            assert(false);
        }

        setValue< ISC_QUAD >(nParameterIndex, aBlobId, SQL_BLOB);
    }
    else if( dType == SQL_VARYING )
    {
            setParameterNull(nParameterIndex, false);
            const sal_Int32 nMaxSize = 0xFFFF;
            const sal_uInt16 nSize = std::min(xBytes.getLength(), nMaxSize);
            // 8000 corresponds to value from lcl_addDefaultParameters
            // in dbaccess/source/filter/hsqldb/createparser.cxx
            if (nSize > 8000)
            {
                free(pVar->sqldata);
                pVar->sqldata = static_cast<char *>(malloc(sizeof(char) * nSize + 2));
            }
            static_assert(sizeof(nSize) == 2, "must match dest memcpy len");
            // First 2 bytes indicate string size
            memcpy(pVar->sqldata, &nSize, 2);
            // Actual data
            memcpy(pVar->sqldata + 2, xBytes.getConstArray(), nSize);
    }
    else if( dType == SQL_TEXT )
    {
            if (pVar->sqllen < xBytes.getLength())
                dbtools::throwSQLException(u"Data too big for this field"_ustr,
                                           dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE, *this);
            setParameterNull(nParameterIndex, false);
            memcpy(pVar->sqldata, xBytes.getConstArray(), xBytes.getLength() );
            // Fill remainder with zeroes
            memset(pVar->sqldata + xBytes.getLength(), 0, pVar->sqllen - xBytes.getLength());
    }
    else
    {
        ::dbtools::throwSQLException(
            u"Incorrect type for setBytes"_ustr,
            ::dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE,
            *this);
    }
}


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 nIndex, const Reference< css::io::XInputStream >&, sal_Int32 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    checkParameterIndex(nIndex);
}


void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 nIndex, const Reference< css::io::XInputStream >&, sal_Int32 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    checkParameterIndex(nIndex);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
