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

#include "ResultSet.hxx"
#include "ResultSetMetaData.hxx"
#include "Util.hxx"

#include <cmath>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbexception.hxx>
#include <propertyids.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <time.h>
#include <TConnection.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>

using namespace ::comphelper;
using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::cppu;
using namespace ::dbtools;
using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;

OResultSet::OResultSet(Connection* pConnection,
                       ::osl::Mutex& rMutex,
                       const uno::Reference< XInterface >& xStatement,
                       isc_stmt_handle aStatementHandle,
                       XSQLDA* pSqlda )
    : OResultSet_BASE(rMutex)
    , OPropertyContainer(OResultSet_BASE::rBHelper)
    , m_bIsBookmarkable(false)
    , m_nFetchSize(1)
    , m_nResultSetType(css::sdbc::ResultSetType::FORWARD_ONLY)
    , m_nFetchDirection(css::sdbc::FetchDirection::FORWARD)
    , m_nResultSetConcurrency(css::sdbc::ResultSetConcurrency::UPDATABLE)
    , m_pConnection(pConnection)
    , m_rMutex(rMutex)
    , m_xStatement(xStatement)
    , m_pSqlda(pSqlda)
    , m_statementHandle(aStatementHandle)
    , m_bWasNull(false)
    , m_currentRow(0)
    , m_bIsAfterLastRow(false)
    , m_fieldCount(pSqlda? pSqlda->sqld : 0)
{
    SAL_INFO("connectivity.firebird", "OResultSet().");
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),
                     PROPERTY_ID_ISBOOKMARKABLE,
                     PropertyAttribute::READONLY,
                     &m_bIsBookmarkable,
                     cppu::UnoType<decltype(m_bIsBookmarkable)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
                     PROPERTY_ID_FETCHSIZE,
                     PropertyAttribute::READONLY,
                     &m_nFetchSize,
                     cppu::UnoType<decltype(m_nFetchSize)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
                     PROPERTY_ID_RESULTSETTYPE,
                     PropertyAttribute::READONLY,
                     &m_nResultSetType,
                     cppu::UnoType<decltype(m_nResultSetType)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
                     PROPERTY_ID_FETCHDIRECTION,
                     PropertyAttribute::READONLY,
                     &m_nFetchDirection,
                     cppu::UnoType<decltype(m_nFetchDirection)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
                     PROPERTY_ID_RESULTSETCONCURRENCY,
                     PropertyAttribute::READONLY,
                     &m_nResultSetConcurrency,
                     cppu::UnoType<decltype(m_nResultSetConcurrency)>::get());

    if (!pSqlda)
        return; // TODO: what?

}

OResultSet::~OResultSet()
{
}

// ---- XResultSet -- Row retrieval methods ------------------------------------
sal_Int32 SAL_CALL OResultSet::getRow()
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow;
}

sal_Bool SAL_CALL OResultSet::next()
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_currentRow++;

    ISC_STATUS fetchStat = isc_dsql_fetch(m_statusVector,
                               &m_statementHandle,
                               1,
                               m_pSqlda);
    if (fetchStat == 0)         // SUCCESSFUL
    {
        return true;
    }
    else if (fetchStat == 100) // END OF DATASET
    {
        m_bIsAfterLastRow = true;
        return false;
    }
    else
    {
        SAL_WARN("connectivity.firebird", "Error when fetching data");
        // Throws sql exception as appropriate
        evaluateStatusVector(m_statusVector, "isc_dsql_fetch", *this);
        return false;
    }
}

sal_Bool SAL_CALL OResultSet::previous()
{
    ::dbtools::throwFunctionNotSupportedSQLException("previous not supported in firebird",
                                                  *this);
    return false;
}

sal_Bool SAL_CALL OResultSet::isLast()
{
    ::dbtools::throwFunctionNotSupportedSQLException("isLast not supported in firebird",
                                                  *this);
    return false;
}

sal_Bool SAL_CALL OResultSet::isBeforeFirst()
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow == 0;
}

sal_Bool SAL_CALL OResultSet::isAfterLast()
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bIsAfterLastRow;
}

sal_Bool SAL_CALL OResultSet::isFirst()
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow == 1 && !m_bIsAfterLastRow;
}

void SAL_CALL OResultSet::beforeFirst()
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (m_currentRow != 0)
        ::dbtools::throwFunctionNotSupportedSQLException("beforeFirst not supported in firebird",
                                                      *this);
}

void SAL_CALL OResultSet::afterLast()
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (!m_bIsAfterLastRow)
        ::dbtools::throwFunctionNotSupportedSQLException("afterLast not supported in firebird",
                                                      *this);
}

sal_Bool SAL_CALL OResultSet::first()
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (m_currentRow == 0)
    {
        return next();
    }
    else if (m_currentRow == 1 && !m_bIsAfterLastRow)
    {
        return true;
    }
    else
    {
        ::dbtools::throwFunctionNotSupportedSQLException("first not supported in firebird",
                                                      *this);
        return false;
    }
}

sal_Bool SAL_CALL OResultSet::last()
{
    // We need to iterate past the last row to know when we've passed the last
    // row, hence we can't actually move to last.
    ::dbtools::throwFunctionNotSupportedSQLException("last not supported in firebird",
                                                  *this);
    return false;
}

sal_Bool SAL_CALL OResultSet::absolute(sal_Int32 aRow)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (aRow > m_currentRow)
    {
        sal_Int32 aIterations = aRow - m_currentRow;
        return relative(aIterations);
    }
    else
    {
        ::dbtools::throwFunctionNotSupportedSQLException("absolute not supported in firebird",
                                                      *this);
        return false;
    }
}

sal_Bool SAL_CALL OResultSet::relative(sal_Int32 row)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (row > 0)
    {
        while (row--)
        {
            if (!next())
                return false;
        }
        return true;
    }
    else
    {
        ::dbtools::throwFunctionNotSupportedSQLException("relative not supported in firebird",
                                                      *this);
        return false;
    }
}

void OResultSet::checkColumnIndex(sal_Int32 nIndex)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if( nIndex < 1 || nIndex > m_fieldCount )
    {
        ::dbtools::throwSQLException(
            "No column " + OUString::number(nIndex),
            ::dbtools::StandardSQLState::COLUMN_NOT_FOUND,
            *this);
    }
}

void OResultSet::checkRowIndex()
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if((m_currentRow < 1) || m_bIsAfterLastRow)
    {
        ::dbtools::throwSQLException(
            "Invalid Row",
            ::dbtools::StandardSQLState::INVALID_CURSOR_POSITION,
            *this);
    }
}

Any SAL_CALL OResultSet::queryInterface( const Type & rType )
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : OResultSet_BASE::queryInterface(rType);
}

 Sequence<  Type > SAL_CALL OResultSet::getTypes()
{
    return concatSequences(OPropertySetHelper::getTypes(), OResultSet_BASE::getTypes());
}
// ---- XColumnLocate ---------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::findColumn(const OUString& rColumnName)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    uno::Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i;

    for(i = 1; i<=nLen; ++i)
    {
        // We assume case sensitive, otherwise you'd have to test
        // xMeta->isCaseSensitive and use qualsIgnoreAsciiCase as needed.
        if (rColumnName == xMeta->getColumnName(i))
            return i;
    }

    ::dbtools::throwInvalidColumnException(rColumnName, *this);
    assert(false);
    return 0; // Never reached
}

uno::Reference< XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 )
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return nullptr;
}

uno::Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 )
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return nullptr;
}

// ---- Internal Utilities ---------------------------------------------------
bool OResultSet::isNull(const sal_Int32 nColumnIndex)
{
    assert(nColumnIndex <= m_fieldCount);
    XSQLVAR* pVar = m_pSqlda->sqlvar;

    if (pVar[nColumnIndex-1].sqltype & 1) // Indicates column may contain null
    {
        if (*pVar[nColumnIndex-1].sqlind == -1)
            return true;
    }
    return false;
}

template <typename T>
OUString OResultSet::makeNumericString(const sal_Int32 nColumnIndex)
{
    //  minus because firebird stores scale as a negative number
    int nDecimalCount = -(m_pSqlda->sqlvar[nColumnIndex-1].sqlscale);
    if(nDecimalCount < 0)
    {
        // scale should be always positive
        assert(false);
        return OUString();
    }

    OUStringBuffer sRetBuffer;
    T nAllDigits = *reinterpret_cast<T*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata);
    sal_Int64 nDecimalCountExp = pow10Integer(nDecimalCount);

    if(nAllDigits < 0)
    {
        sRetBuffer.append('-');
        nAllDigits = -nAllDigits; // abs
    }

    sRetBuffer.append(static_cast<sal_Int64>(nAllDigits / nDecimalCountExp) );
    if( nDecimalCount > 0)
    {
        sRetBuffer.append('.');

        sal_Int64 nFractionalPart = nAllDigits % nDecimalCountExp;

        int iCount = 0; // digit count
        sal_Int64 nFracTemp = nFractionalPart;
        while(nFracTemp>0)
        {
            nFracTemp /= 10;
            iCount++;
        }

        int nMissingNulls = nDecimalCount - iCount;

        // append nulls after dot and before nFractionalPart
        for(int i=0; i<nMissingNulls; i++)
        {
            sRetBuffer.append('0');
        }

        // the rest
        sRetBuffer.append(nFractionalPart);
    }

    return sRetBuffer.makeStringAndClear();
}

template <typename T>
T OResultSet::retrieveValue(const sal_Int32 nColumnIndex, const ISC_SHORT nType)
{
    m_bWasNull = isNull(nColumnIndex);
    if (m_bWasNull)
        return T();

    if ((m_pSqlda->sqlvar[nColumnIndex-1].sqltype & ~1) == nType)
        return *reinterpret_cast<T*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata);
    else
        return retrieveValue< ORowSetValue >(nColumnIndex, 0);
}

template <>
ORowSetValue OResultSet::retrieveValue(const sal_Int32 nColumnIndex, const ISC_SHORT /*nType*/)
{
    // See http://wiki.openoffice.org/wiki/Documentation/DevGuide/Database/Using_the_getXXX_Methods
    // (bottom of page) for a chart of possible conversions, we should allow all
    // of these -- Blob/Clob will probably need some specialist handling especially
    // w.r.t. to generating Strings for them.
    //
    // Basically we just have to map to the correct direct request and
    // ORowSetValue does the rest for us here.
    int nSqlSubType = m_pSqlda->sqlvar[nColumnIndex-1].sqlsubtype;

    // TODO Firebird 3.0 does not set subtype (i.e. set to 0) for computed numeric/decimal value.
    // It may change in the future.
    // Imply numeric data type when subtype is 0 and scale is negative
    if( nSqlSubType == 0 && m_pSqlda->sqlvar[nColumnIndex-1].sqlscale < 0 )
        nSqlSubType = 1;

    switch (m_pSqlda->sqlvar[nColumnIndex-1].sqltype & ~1)
    {
        case SQL_TEXT:
        case SQL_VARYING:
            return getString(nColumnIndex);
        case SQL_SHORT:
            if(nSqlSubType == 1 || nSqlSubType == 2) //numeric or decimal
                return getString(nColumnIndex);
            return getShort(nColumnIndex);
        case SQL_LONG:
            if(nSqlSubType == 1 || nSqlSubType == 2) //numeric or decimal
                return getString(nColumnIndex);
            return getInt(nColumnIndex);
        case SQL_FLOAT:
            return getFloat(nColumnIndex);
        case SQL_DOUBLE:
            if(nSqlSubType == 1 || nSqlSubType == 2) //numeric or decimal
                return getString(nColumnIndex);
            return getDouble(nColumnIndex);
        case SQL_D_FLOAT:
            return getFloat(nColumnIndex);
        case SQL_TIMESTAMP:
            return getTimestamp(nColumnIndex);
        case SQL_TYPE_TIME:
            return getTime(nColumnIndex);
        case SQL_TYPE_DATE:
            return getDate(nColumnIndex);
        case SQL_INT64:
            if(nSqlSubType == 1 || nSqlSubType == 2) //numeric or decimal
                return getString(nColumnIndex);
            return getLong(nColumnIndex);
        case SQL_BOOLEAN:
            return ORowSetValue(bool(getBoolean(nColumnIndex)));
        case SQL_BLOB:
        case SQL_NULL:
        case SQL_QUAD:
        case SQL_ARRAY:
            // TODO: these are all invalid conversions, so maybe we should
            // throw an exception?
            return ORowSetValue();
        default:
            assert(false);
            return ORowSetValue();
    }
}

template <>
Date OResultSet::retrieveValue(const sal_Int32 nColumnIndex, const ISC_SHORT /*nType*/)
{
    if ((m_pSqlda->sqlvar[nColumnIndex-1].sqltype & ~1) == SQL_TYPE_DATE)
    {
        ISC_DATE aISCDate = *reinterpret_cast<ISC_DATE*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata);

        struct tm aCTime;
        isc_decode_sql_date(&aISCDate, &aCTime);

        return Date(aCTime.tm_mday, aCTime.tm_mon + 1, aCTime.tm_year + 1900);
    }
    else
    {
        return retrieveValue< ORowSetValue >(nColumnIndex, 0);
    }
}

template <>
Time OResultSet::retrieveValue(const sal_Int32 nColumnIndex, const ISC_SHORT /*nType*/)
{
    if ((m_pSqlda->sqlvar[nColumnIndex-1].sqltype & ~1) == SQL_TYPE_TIME)
    {
        ISC_TIME aISCTime = *reinterpret_cast<ISC_TIME*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata);

        struct tm aCTime;
        isc_decode_sql_time(&aISCTime, &aCTime);

        // First field is nanoseconds.
        // last field denotes UTC (true) or unknown (false)
        // Here we "know" that ISC_TIME is simply in units of seconds/ISC_TIME_SECONDS_PRECISION
        // with no other funkiness, so we can get the fractional seconds easily.
        return Time((aISCTime % ISC_TIME_SECONDS_PRECISION) * (1000000000 / ISC_TIME_SECONDS_PRECISION),
                    aCTime.tm_sec, aCTime.tm_min, aCTime.tm_hour, false);
    }
    else
    {
        return retrieveValue< ORowSetValue >(nColumnIndex, 0);
    }
}

template <>
DateTime OResultSet::retrieveValue(const sal_Int32 nColumnIndex, const ISC_SHORT /*nType*/)
{
    if ((m_pSqlda->sqlvar[nColumnIndex-1].sqltype & ~1) == SQL_TIMESTAMP)
    {
        ISC_TIMESTAMP aISCTimestamp = *reinterpret_cast<ISC_TIMESTAMP*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata);

        struct tm aCTime;
        isc_decode_timestamp(&aISCTimestamp, &aCTime);

        // Ditto here, see comment in previous function about ISC_TIME and ISC_TIME_SECONDS_PRECISION.
        return DateTime((aISCTimestamp.timestamp_time % ISC_TIME_SECONDS_PRECISION) * (1000000000 / ISC_TIME_SECONDS_PRECISION), //nanoseconds
                        aCTime.tm_sec,
                        aCTime.tm_min,
                        aCTime.tm_hour,
                        aCTime.tm_mday,
                        aCTime.tm_mon + 1, // tm is from 0 to 11
                        aCTime.tm_year + 1900, //tm_year is the years since 1900
                        false); // denotes UTC (true), or unknown (false)
    }
    else
    {
        return retrieveValue< ORowSetValue >(nColumnIndex, 0);
    }
}

template <>
OUString OResultSet::retrieveValue(const sal_Int32 nColumnIndex, const ISC_SHORT /*nType*/)
{
    // &~1 to remove the "can contain NULL" indicator
    int aSqlType = m_pSqlda->sqlvar[nColumnIndex-1].sqltype & ~1;
    int aSqlSubType = m_pSqlda->sqlvar[nColumnIndex-1].sqlsubtype;
    if (aSqlType == SQL_TEXT )
    {
        return OUString(m_pSqlda->sqlvar[nColumnIndex-1].sqldata,
                        m_pSqlda->sqlvar[nColumnIndex-1].sqllen,
                        RTL_TEXTENCODING_UTF8);
    }
    else if (aSqlType == SQL_VARYING)
    {
        // First 2 bytes are a short containing the length of the string
        // No idea if sqllen is still valid here?
        sal_uInt16 aLength = *reinterpret_cast<sal_uInt16*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata);
        return OUString(m_pSqlda->sqlvar[nColumnIndex-1].sqldata + 2,
                        aLength,
                        RTL_TEXTENCODING_UTF8);
    }
    else if ((aSqlType == SQL_SHORT || aSqlType == SQL_LONG ||
              aSqlType == SQL_DOUBLE || aSqlType == SQL_INT64)
          && (aSqlSubType == 1 ||
              aSqlSubType == 2 ||
              (aSqlSubType == 0 && m_pSqlda->sqlvar[nColumnIndex-1].sqlscale < 0) ) )
    {
        // decimal and numeric types
        switch(aSqlType)
        {
            case SQL_SHORT:
                return makeNumericString<sal_Int16>(nColumnIndex);
            case SQL_LONG:
                return makeNumericString<sal_Int32>(nColumnIndex);
            case SQL_DOUBLE:
                // TODO FIXME 64 bits?
            case SQL_INT64:
                return makeNumericString<sal_Int64>(nColumnIndex);
            default:
                assert(false);
                return OUString(); // never reached
        }
    }
    else if(aSqlType == SQL_BLOB && aSqlSubType == static_cast<short>(BlobSubtype::Clob) )
    {
        uno::Reference<XClob> xClob = getClob(nColumnIndex);
        return xClob->getSubString( 0, xClob->length() );
    }
    else
    {
        return retrieveValue< ORowSetValue >(nColumnIndex, 0);
    }
}

template <>
ISC_QUAD* OResultSet::retrieveValue(const sal_Int32 nColumnIndex, const ISC_SHORT nType)
{
    // TODO: this is probably wrong
    if ((m_pSqlda->sqlvar[nColumnIndex-1].sqltype & ~1) != nType)
        throw SQLException(); // TODO: better exception (can't convert Blob)

    return reinterpret_cast<ISC_QUAD*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata);
}

template <typename T>
T OResultSet::safelyRetrieveValue(const sal_Int32 nColumnIndex, const ISC_SHORT nType)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(nColumnIndex);
    checkRowIndex();

    m_bWasNull = isNull(nColumnIndex);
    if (m_bWasNull)
        return T();

    return retrieveValue< T >(nColumnIndex, nType);
}

// ---- XRow -----------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::wasNull()
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
}

// ---- XRow: Simple Numerical types ------------------------------------------
sal_Bool SAL_CALL OResultSet::getBoolean(sal_Int32 nColumnIndex)
{
    return safelyRetrieveValue< bool >(nColumnIndex, SQL_BOOLEAN);
}

sal_Int8 SAL_CALL OResultSet::getByte(sal_Int32 nColumnIndex)
{
    // Not a native firebird type hence we always have to convert.
    return safelyRetrieveValue< ORowSetValue >(nColumnIndex);
}

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes(sal_Int32)
{
    return Sequence< sal_Int8 >(); // TODO: implement
    //return safelyRetrieveValue(columnIndex);
}

sal_Int16 SAL_CALL OResultSet::getShort(sal_Int32 columnIndex)
{
    return safelyRetrieveValue< sal_Int16 >(columnIndex, SQL_SHORT);
}

sal_Int32 SAL_CALL OResultSet::getInt(sal_Int32 columnIndex)
{
    return safelyRetrieveValue< sal_Int32 >(columnIndex, SQL_LONG);
}

sal_Int64 SAL_CALL OResultSet::getLong(sal_Int32 columnIndex)
{
    return safelyRetrieveValue< sal_Int64 >(columnIndex, SQL_INT64);
}

float SAL_CALL OResultSet::getFloat(sal_Int32 columnIndex)
{
    return safelyRetrieveValue< float >(columnIndex, SQL_FLOAT);
}

double SAL_CALL OResultSet::getDouble(sal_Int32 columnIndex)
{
    return safelyRetrieveValue< double >(columnIndex, SQL_DOUBLE);
}

// ---- XRow: More complex types ----------------------------------------------
OUString SAL_CALL OResultSet::getString(sal_Int32 nIndex)
{
    return safelyRetrieveValue< OUString >(nIndex);
}

Date SAL_CALL OResultSet::getDate(sal_Int32 nIndex)
{
    return safelyRetrieveValue< Date >(nIndex, SQL_TYPE_DATE);
}

Time SAL_CALL OResultSet::getTime(sal_Int32 nIndex)
{
    return safelyRetrieveValue< css::util::Time >(nIndex, SQL_TYPE_TIME);
}

DateTime SAL_CALL OResultSet::getTimestamp(sal_Int32 nIndex)
{
    return safelyRetrieveValue< DateTime >(nIndex, SQL_TIMESTAMP);
}


uno::Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  )
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pConnection
                                           , m_pSqlda);
    return m_xMetaData;
}

uno::Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 )
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return nullptr;
}


uno::Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    int aSqlSubType = m_pSqlda->sqlvar[columnIndex-1].sqlsubtype;

    SAL_WARN_IF(aSqlSubType != 1,
        "connectivity.firebird", "wrong subtype, not a textual blob");

    ISC_QUAD* pBlobID = safelyRetrieveValue< ISC_QUAD* >(columnIndex, SQL_BLOB);
    if (!pBlobID)
        return nullptr;
    return m_pConnection->createClob(pBlobID);
}

uno::Reference< XBlob > SAL_CALL OResultSet::getBlob(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // TODO: CLOB etc. should be valid here too, but we probably want some more
    // cleverness around this.
    ISC_QUAD* pBlobID = safelyRetrieveValue< ISC_QUAD* >(columnIndex, SQL_BLOB);
    if (!pBlobID)
        return nullptr;
    return m_pConnection->createBlob(pBlobID);
}


uno::Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 )
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return nullptr;
}


Any SAL_CALL OResultSet::getObject( sal_Int32, const uno::Reference< css::container::XNameAccess >& )
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return Any();
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
            sBuffer.append(std::u16string_view(sNumber).substr(0, nDotIndex));
        }
        sBuffer.append(std::u16string_view(sNumber).substr(nDotIndex + 1));
        return sBuffer.makeStringAndClear().toInt64();
    }
}

}
// void OPreaparedStatement::set
void OResultSet::openBlobForWriting(isc_blob_handle& rBlobHandle, ISC_QUAD& rBlobId)
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
                             "setBlob failed",
                             *this);
        assert(false);
    }
}

void OResultSet::closeBlobAfterWriting(isc_blob_handle& rBlobHandle)
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

// XResultSetUpdate
void OResultSet::insertRow()
{
}

void OResultSet::updateRow()
{
}

void OResultSet::deleteRow()
{
}

void OResultSet::cancelRowUpdates()
{
}

void OResultSet::moveToInsertRow()
{
}

void OResultSet::moveToCurrentRow()
{
}

// XRowUpdate
void OResultSet::updateParameterNull(sal_Int32 nParameterIndex,
                                          bool bSetNull)
{
    XSQLVAR* pVar = m_pSqlda->sqlvar + (nParameterIndex - 1);
    if (bSetNull)
    {
        pVar->sqltype |= 1;
        *pVar->sqlind = -1;
    }
    else
        *pVar->sqlind = 0;
}


void OResultSet::updateClob( sal_Int32 column, const OUString& rStr )
{
    MutexGuard aGuard( m_rMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    checkRowIndex();

#if SAL_TYPES_SIZEOFPOINTER == 8
    isc_blob_handle aBlobHandle = 0;
#else
    isc_blob_handle aBlobHandle = nullptr;
#endif
    ISC_QUAD aBlobId;

    openBlobForWriting(aBlobHandle, aBlobId);

    OString sData = OUStringToOString(
            rStr,
            RTL_TEXTENCODING_UTF8);
    ISC_STATUS aErr = isc_put_segment( m_statusVector,
                            &aBlobHandle,
                            sData.getLength(),
                            sData.getStr() );

    // We need to make sure we close the Blob even if there are errors, hence evaluate
    // errors after closing.
    closeBlobAfterWriting(aBlobHandle);

    if (aErr)
    {
        evaluateStatusVector(m_statusVector,
                             "isc_put_segment failed",
                             *this);
        assert(false);
    }

    updateValue< ISC_QUAD >(column, aBlobId, SQL_BLOB);
}

template <typename T>
void OResultSet::updateValue(sal_Int32 column, const T& nValue, ISC_SHORT nType)
{
    MutexGuard aGuard( m_rMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    checkRowIndex();

    updateParameterNull(column, false);
    XSQLVAR* pVar = m_pSqlda->sqlvar + (column - 1);

    if ((pVar->sqltype & ~1) != nType)
    {
       ::dbtools::throwSQLException(
            "Incorrect type for setValue",
            ::dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE,
            *this);
    }

    memcpy(pVar->sqldata, &nValue, sizeof(nValue));
}

void SAL_CALL OResultSet::updateNull(sal_Int32 column)
{
    MutexGuard aGuard( m_rMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    checkRowIndex();
    updateParameterNull(column, true);
}

void SAL_CALL OResultSet::updateBoolean(sal_Int32 column, sal_Bool x)
{
    updateValue< sal_Bool >(column, x, SQL_BOOLEAN);
}

void SAL_CALL OResultSet::updateByte(sal_Int32 column, sal_Int8 x)
{
    // there's no TINYINT or equivalent on Firebird,
    // so do the same as updateShort
    updateValue< sal_Int16 >(column, x, SQL_SHORT);
}

void SAL_CALL OResultSet::updateShort(sal_Int32 column, sal_Int16 x)
{
    updateValue< sal_Int16 >(column, x, SQL_SHORT);
}

void SAL_CALL OResultSet::updateInt(sal_Int32 column, sal_Int32 x)
{
    updateValue< sal_Int32 >(column, x, SQL_LONG);
}

void SAL_CALL OResultSet::updateLong(sal_Int32 column, sal_Int64 x)
{
    updateValue< sal_Int64 >(column, x, SQL_INT64);
}

void SAL_CALL OResultSet::updateFloat(sal_Int32 column, float x)
{
    updateValue< float >(column, x, SQL_FLOAT);
}

void SAL_CALL OResultSet::updateDouble(sal_Int32 column, double x)
{
    MutexGuard aGuard( m_rMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    checkRowIndex();

    XSQLVAR* pVar = m_pSqlda->sqlvar + (column - 1);
    short dType = (pVar->sqltype & ~1); // drop flag bit for now
    short dSubType = pVar->sqlsubtype;
    // Assume it is a sub type of a number.
    if(dSubType < 0 || dSubType > 2)
    {
        ::dbtools::throwSQLException(
            "Incorrect number sub type",
            ::dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE,
            *this);
    }
    // firebird stores scale as a negative number
    ColumnTypeInfo columnType{ dType, dSubType,
        static_cast<short>(-pVar->sqlscale) };

    // Caller might try to set an integer type here. It makes sense to convert
    // it instead of throwing an error.
    switch(columnType.getSdbcType())
    {
        case DataType::SMALLINT:
            updateValue< sal_Int16 >(column,
                    static_cast<sal_Int16>(x),
                    dType);
            break;
        case DataType::INTEGER:
            updateValue< sal_Int32 >(column,
                    static_cast<sal_Int32>(x),
                    dType);
            break;
        case DataType::BIGINT:
            updateValue< sal_Int64 >(column,
                    static_cast<sal_Int64>(x),
                    dType);
            break;
        case DataType::NUMERIC:
        case DataType::DECIMAL:
            // take decimal places into account, later on they are removed in makeNumericString
            updateNumericObject(column, css::uno::Any{x}, columnType.getScale());
            break;
        default:
            updateValue< double >(column, x, SQL_DOUBLE); // TODO: SQL_D_FLOAT?
    }
}

void SAL_CALL OResultSet::updateString(sal_Int32 column, const OUString& x)
{
    MutexGuard aGuard( m_rMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    checkRowIndex();
    updateParameterNull(column, false);

    OString str = OUStringToOString(x , RTL_TEXTENCODING_UTF8 );

    XSQLVAR* pVar = m_pSqlda->sqlvar + (column - 1);

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
        const auto nLength = str.getLength();
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
    case SQL_BLOB: // Clob
        assert( pVar->sqlsubtype == static_cast<short>(BlobSubtype::Clob) );
        updateClob(column, x );
        break;
    default:
        ::dbtools::throwSQLException(
            "Incorrect type for setString",
            ::dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE,
            *this);
    }
}

void SAL_CALL OResultSet::updateBytes(sal_Int32 column, const css::uno::Sequence<sal_Int8>& x)
{
    MutexGuard aGuard( m_rMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    checkRowIndex();

    XSQLVAR* pVar = m_pSqlda->sqlvar + (column - 1);
    int dType = (pVar->sqltype & ~1); // drop flag bit for now

    if( dType == SQL_BLOB )
    {
#if SAL_TYPES_SIZEOFPOINTER == 8
        isc_blob_handle aBlobHandle = 0;
#else
        isc_blob_handle aBlobHandle = nullptr;
#endif
        ISC_QUAD aBlobId;

        openBlobForWriting(aBlobHandle, aBlobId);

        ISC_STATUS aErr = 0;
        const sal_Int32 nBytesLen = x.getLength();
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
                                       reinterpret_cast<const char*>(x.getConstArray()) + nDataWritten);
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
                                 "isc_put_segment failed",
                                 *this);
            assert(false);
        }

        updateValue< ISC_QUAD >(column, aBlobId, SQL_BLOB);
    }
    else if( dType == SQL_VARYING )
    {
            updateParameterNull(column, false);
            const sal_Int32 nMaxSize = 0xFFFF;
            Sequence<sal_Int8> xCopy(x);
            if (xCopy.getLength() > nMaxSize)
            {
                xCopy.realloc( nMaxSize );
            }
            const auto nSize = xCopy.getLength();
            // 8000 corresponds to value from lcl_addDefaultParameters
            // in dbaccess/source/filter/hsqldb/createparser.cxx
            if (nSize > 8000)
            {
                free(pVar->sqldata);
                pVar->sqldata = static_cast<char *>(malloc(sizeof(char) * nSize + 2));
            }
            // First 2 bytes indicate string size
            memcpy(pVar->sqldata, &nSize, 2);
            // Actual data
            memcpy(pVar->sqldata + 2, xCopy.getConstArray(), nSize);
    }
    else if( dType == SQL_TEXT )
    {
            updateParameterNull(column, false);
            memcpy(pVar->sqldata, x.getConstArray(), x.getLength() );
            // Fill remainder with spaces
            memset(pVar->sqldata + x.getLength(), 0, pVar->sqllen - x.getLength());
    }
    else
    {
        ::dbtools::throwSQLException(
            "Incorrect type for setBytes",
            ::dbtools::StandardSQLState::INVALID_SQL_DATA_TYPE,
            *this);
    }
}

void SAL_CALL OResultSet::updateDate(sal_Int32 column, const css::util::Date& x)
{
    struct tm aCTime;
    aCTime.tm_mday = x.Day;
    aCTime.tm_mon = x.Month -1;
    aCTime.tm_year = x.Year -1900;

    ISC_DATE aISCDate;
    isc_encode_sql_date(&aCTime, &aISCDate);

    updateValue< ISC_DATE >(column, aISCDate, SQL_TYPE_DATE);
}

void SAL_CALL OResultSet::updateTime(sal_Int32 column, const css::util::Time& x)
{
    struct tm aCTime;
    aCTime.tm_sec = x.Seconds;
    aCTime.tm_min = x.Minutes;
    aCTime.tm_hour = x.Hours;

    ISC_TIME aISCTime;
    isc_encode_sql_time(&aCTime, &aISCTime);

    // Here we "know" that ISC_TIME is simply in units of seconds/ISC_TIME_SECONDS_PRECISION with no
    // other funkiness, so we can simply add the fraction of a second.
    aISCTime += x.NanoSeconds / (1000000000 / ISC_TIME_SECONDS_PRECISION);

    updateValue< ISC_TIME >(column, aISCTime, SQL_TYPE_TIME);
}

void SAL_CALL OResultSet::updateTimestamp(sal_Int32 column, const css::util::DateTime& x)
{
    struct tm aCTime;
    aCTime.tm_sec = x.Seconds;
    aCTime.tm_min = x.Minutes;
    aCTime.tm_hour = x.Hours;
    aCTime.tm_mday = x.Day;
    aCTime.tm_mon = x.Month - 1;
    aCTime.tm_year = x.Year - 1900;

    ISC_TIMESTAMP aISCTimestamp;
    isc_encode_timestamp(&aCTime, &aISCTimestamp);

    // As in previous function
    aISCTimestamp.timestamp_time += x.NanoSeconds / (1000000000 / ISC_TIME_SECONDS_PRECISION);

    updateValue< ISC_TIMESTAMP >(column, aISCTimestamp, SQL_TIMESTAMP);
}

void SAL_CALL OResultSet::updateBinaryStream(sal_Int32 column,
                                     const css::uno::Reference<css::io::XInputStream>&,
                                     sal_Int32)
{
// TODO ?
    MutexGuard aGuard( m_rMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    checkRowIndex();
}

void SAL_CALL OResultSet::updateCharacterStream(sal_Int32 column,
                                        const css::uno::Reference<css::io::XInputStream>&,
                                        sal_Int32)
{
// TODO ?
    MutexGuard aGuard( m_rMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    checkRowIndex();
}

void SAL_CALL OResultSet::updateObject(sal_Int32 column, const css::uno::Any&)
{
// TODO ?
    MutexGuard aGuard( m_rMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    checkRowIndex();
}

void SAL_CALL OResultSet::updateNumericObject(sal_Int32 column, const css::uno::Any& x, sal_Int32 scale)
{
    MutexGuard aGuard( m_rMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    checkRowIndex();

    updateParameterNull(column, false);

    XSQLVAR* pVar = m_pSqlda->sqlvar + (column - 1);
    int dType = (pVar->sqltype & ~1); // drop null flag

    double dbValue =0.0;
    OUString sValue;
    if( x >>= dbValue )
    {
        // truncate and round to 'scale' number of decimal places
        sValue = OUString::number( std::floor((dbValue * pow10Integer(scale)) + .5) / pow10Integer(scale) );
    }
    else
    {
        x >>= sValue;
    }

    // fill in the number with nulls in fractional part.
    // We need this because  e.g. 0.450 != 0.045 despite
    // their scale is equal
    OUStringBuffer sBuffer(15);
    sBuffer.append(sValue);
    if(sValue.indexOf('.') != -1) // there is a dot
    {
        for(sal_Int32 i=sValue.copy(sValue.indexOf('.')+1).getLength(); i<scale;i++)
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

    sValue = sBuffer.makeStringAndClear();
    switch(dType)
    {
        case SQL_SHORT:
            updateValue< sal_Int16 >(column,
                    static_cast<sal_Int16>( toNumericWithoutDecimalPlace(sValue) ),
                    dType);
            break;
        case SQL_LONG:
        case SQL_DOUBLE:
            updateValue< sal_Int32 >(column,
                    static_cast<sal_Int32>( toNumericWithoutDecimalPlace(sValue) ),
                    dType);
            break;
        case SQL_INT64:
            updateValue< sal_Int64 >(column,
                    toNumericWithoutDecimalPlace(sValue),
                    dType);
            break;
        default:
            SAL_WARN("connectivity.firebird",
                    "No Firebird sql type found for numeric or decimal types");
            assert(false);
    }
}


void SAL_CALL OResultSet::close()
{
    SAL_INFO("connectivity.firebird", "close().");

    {
        MutexGuard aGuard(m_rMutex);
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    }
    dispose();
}


uno::Reference< XInterface > SAL_CALL OResultSet::getStatement()
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_xStatement;
}
//----- XResultSet: unsupported change detection methods ---------------------
sal_Bool SAL_CALL OResultSet::rowDeleted()
{
    ::dbtools::throwFunctionNotSupportedSQLException("rowDeleted not supported in firebird",
                                                  *this);
    return false;
}
sal_Bool SAL_CALL OResultSet::rowInserted()
{
    ::dbtools::throwFunctionNotSupportedSQLException("rowInserted not supported in firebird",
                                                  *this);
    return false;
}

sal_Bool SAL_CALL OResultSet::rowUpdated()
{
    ::dbtools::throwFunctionNotSupportedSQLException("rowUpdated not supported in firebird",
                                                  *this);
    return false;
}

void SAL_CALL OResultSet::refreshRow()
{
    ::dbtools::throwFunctionNotSupportedSQLException("refreshRow not supported in firebird",
                                                  *this);
}


void SAL_CALL OResultSet::cancel(  )
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}

//----- OIdPropertyArrayUsageHelper ------------------------------------------
IPropertyArrayHelper* OResultSet::createArrayHelper() const
{
    Sequence< Property > aProperties;
    describeProperties(aProperties);
    return new ::cppu::OPropertyArrayHelper(aProperties);
}

IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

void SAL_CALL OResultSet::acquire() throw()
{
    OResultSet_BASE::acquire();
}

void SAL_CALL OResultSet::release() throw()
{
    OResultSet_BASE::release();
}

uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

// ---- XServiceInfo -----------------------------------------------------------
OUString SAL_CALL OResultSet::getImplementationName()
{
    return "com.sun.star.sdbcx.firebird.ResultSet";
}

Sequence< OUString > SAL_CALL OResultSet::getSupportedServiceNames()
{
    return {"com.sun.star.sdbc.ResultSet","com.sun.star.sdbcx.ResultSet"};
}

sal_Bool SAL_CALL OResultSet::supportsService(const OUString& _rServiceName)
{
    return cppu::supportsService(this, _rServiceName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
