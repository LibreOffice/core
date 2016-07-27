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

#include "ResultSet.hxx"
#include "ResultSetMetaData.hxx"
#include "Util.hxx"

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbexception.hxx>
#include <propertyids.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
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
                       isc_stmt_handle& aStatementHandle,
                       XSQLDA* pSqlda)
    : OResultSet_BASE(rMutex)
    , OPropertyContainer(OResultSet_BASE::rBHelper)
    , m_bIsBookmarkable(false)
    , m_nFetchSize(1)
    , m_nResultSetType(css::sdbc::ResultSetType::FORWARD_ONLY)
    , m_nFetchDirection(css::sdbc::FetchDirection::FORWARD)
    , m_nResultSetConcurrency(css::sdbc::ResultSetConcurrency::READ_ONLY)
    , m_pConnection(pConnection)
    , m_rMutex(rMutex)
    , m_xStatement(xStatement)
    , m_xMetaData(nullptr)
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
sal_Int32 SAL_CALL OResultSet::getRow() throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow;
}

sal_Bool SAL_CALL OResultSet::next() throw(SQLException, RuntimeException, std::exception)
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
    else if (fetchStat == 100L) // END OF DATASET
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

sal_Bool SAL_CALL OResultSet::previous() throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionNotSupportedSQLException("previous not supported in firebird",
                                                  *this);
    return false;
}

sal_Bool SAL_CALL OResultSet::isLast() throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionNotSupportedSQLException("isLast not supported in firebird",
                                                  *this);
    return false;
}

sal_Bool SAL_CALL OResultSet::isBeforeFirst() throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow == 0;
}

sal_Bool SAL_CALL OResultSet::isAfterLast() throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bIsAfterLastRow;
}

sal_Bool SAL_CALL OResultSet::isFirst() throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow == 1 && !m_bIsAfterLastRow;
}

void SAL_CALL OResultSet::beforeFirst() throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (m_currentRow != 0)
        ::dbtools::throwFunctionNotSupportedSQLException("beforeFirst not supported in firebird",
                                                      *this);
}

void SAL_CALL OResultSet::afterLast() throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (!m_bIsAfterLastRow)
        ::dbtools::throwFunctionNotSupportedSQLException("afterLast not supported in firebird",
                                                      *this);
}

sal_Bool SAL_CALL OResultSet::first() throw(SQLException, RuntimeException, std::exception)
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

sal_Bool SAL_CALL OResultSet::last() throw(SQLException, RuntimeException, std::exception)
{
    // We need to iterate past the last row to know when we've passed the last
    // row, hence we can't actually move to last.
    ::dbtools::throwFunctionNotSupportedSQLException("last not supported in firebird",
                                                  *this);
    return false;
}

sal_Bool SAL_CALL OResultSet::absolute(sal_Int32 aRow) throw(SQLException, RuntimeException, std::exception)
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

sal_Bool SAL_CALL OResultSet::relative(sal_Int32 row) throw(SQLException, RuntimeException, std::exception)
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

void SAL_CALL OResultSet::checkColumnIndex(sal_Int32 nIndex)
    throw (SQLException, RuntimeException)
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

void SAL_CALL OResultSet::checkRowIndex()
    throw (SQLException, RuntimeException)
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

Any SAL_CALL OResultSet::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : OResultSet_BASE::queryInterface(rType);
}

 Sequence<  Type > SAL_CALL OResultSet::getTypes() throw( RuntimeException, std::exception)
{
    return concatSequences(OPropertySetHelper::getTypes(), OResultSet_BASE::getTypes());
}
// ---- XColumnLocate ---------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::findColumn(const OUString& rColumnName)
    throw(SQLException, RuntimeException, std::exception)
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

uno::Reference< XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    (void) columnIndex;
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return nullptr;
}

uno::Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    (void) columnIndex;
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
T OResultSet::retrieveValue(const sal_Int32 nColumnIndex, const ISC_SHORT nType)
{
    if ((m_bWasNull = isNull(nColumnIndex)))
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
    switch (m_pSqlda->sqlvar[nColumnIndex-1].sqltype & ~1)
    {
        case SQL_TEXT:
        case SQL_VARYING:
            return getString(nColumnIndex);
        case SQL_SHORT:
            return getShort(nColumnIndex);
        case SQL_LONG:
            return getInt(nColumnIndex);
        case SQL_FLOAT:
            return getFloat(nColumnIndex);
        case SQL_DOUBLE:
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
            return getLong(nColumnIndex);
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
        ISC_DATE aISCDate = *(reinterpret_cast<ISC_DATE*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata));

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
        ISC_TIME aISCTime = *(reinterpret_cast<ISC_TIME*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata));

        struct tm aCTime;
        isc_decode_sql_time(&aISCTime, &aCTime);

        // first field is nanoseconds -- not supported in firebird or struct tm.
        // last field denotes UTC (true) or unknown (false)
        return Time(0, aCTime.tm_sec, aCTime.tm_min, aCTime.tm_hour, false);
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
        ISC_TIMESTAMP aISCTimestamp = *(reinterpret_cast<ISC_TIMESTAMP*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata));

        struct tm aCTime;
        isc_decode_timestamp(&aISCTimestamp, &aCTime);

        return DateTime(0, //nanoseconds, not supported
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
        sal_uInt16 aLength = *(reinterpret_cast<sal_uInt16*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata));
        return OUString(m_pSqlda->sqlvar[nColumnIndex-1].sqldata + 2,
                        aLength,
                        RTL_TEXTENCODING_UTF8);
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
    if ((m_pSqlda->sqlvar[nColumnIndex-1].sqltype & ~1) == nType)
        return reinterpret_cast<ISC_QUAD*>(m_pSqlda->sqlvar[nColumnIndex-1].sqldata);
    else
        throw SQLException(); // TODO: better exception (can't convert Blob)
}

template <typename T>
T OResultSet::safelyRetrieveValue(const sal_Int32 nColumnIndex, const ISC_SHORT nType)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(nColumnIndex);
    checkRowIndex();

    if ((m_bWasNull = isNull(nColumnIndex)))
        return T();

    return retrieveValue< T >(nColumnIndex, nType);
}

// ---- XRow -----------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::wasNull() throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
}

// ---- XRow: Simple Numerical types ------------------------------------------
sal_Bool SAL_CALL OResultSet::getBoolean(sal_Int32 nColumnIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    // Not a native firebird type hence we always have to convert.
    return safelyRetrieveValue< ORowSetValue >(nColumnIndex);
}

sal_Int8 SAL_CALL OResultSet::getByte(sal_Int32 nColumnIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    // Not a native firebird type hence we always have to convert.
    return safelyRetrieveValue< ORowSetValue >(nColumnIndex);
}

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    (void) columnIndex;
    return Sequence< sal_Int8 >(); // TODO: implement
    //return safelyRetrieveValue(columnIndex);
}

sal_Int16 SAL_CALL OResultSet::getShort(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    return safelyRetrieveValue< sal_Int16 >(columnIndex, SQL_SHORT);
}

sal_Int32 SAL_CALL OResultSet::getInt(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    return safelyRetrieveValue< sal_Int32 >(columnIndex, SQL_LONG);
}

sal_Int64 SAL_CALL OResultSet::getLong(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    return safelyRetrieveValue< sal_Int64 >(columnIndex, SQL_INT64);
}

float SAL_CALL OResultSet::getFloat(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    return safelyRetrieveValue< float >(columnIndex, SQL_FLOAT);
}

double SAL_CALL OResultSet::getDouble(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    return safelyRetrieveValue< double >(columnIndex, SQL_DOUBLE);
}

// ---- XRow: More complex types ----------------------------------------------
OUString SAL_CALL OResultSet::getString(sal_Int32 nIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    return safelyRetrieveValue< OUString >(nIndex);
}

Date SAL_CALL OResultSet::getDate(sal_Int32 nIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    return safelyRetrieveValue< Date >(nIndex, SQL_TYPE_DATE);
}

Time SAL_CALL OResultSet::getTime(sal_Int32 nIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    return safelyRetrieveValue< css::util::Time >(nIndex, SQL_TYPE_TIME);
}

DateTime SAL_CALL OResultSet::getTimestamp(sal_Int32 nIndex)
    throw(SQLException, RuntimeException, std::exception)
{
    return safelyRetrieveValue< DateTime >(nIndex, SQL_TIMESTAMP);
}


uno::Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  ) throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pConnection, m_pSqlda);
    return m_xMetaData;
}

uno::Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    (void) columnIndex;
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return nullptr;
}


uno::Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    (void) columnIndex;
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return nullptr;
}

uno::Reference< XBlob > SAL_CALL OResultSet::getBlob(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException, std::exception)
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


uno::Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    (void) columnIndex;
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return nullptr;
}


Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const uno::Reference< css::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException, std::exception)
{
    (void) columnIndex;
    (void) typeMap;
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return Any();
}


void SAL_CALL OResultSet::close() throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("connectivity.firebird", "close().");

    {
        MutexGuard aGuard(m_rMutex);
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    }
    dispose();
}


uno::Reference< XInterface > SAL_CALL OResultSet::getStatement()
    throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_xStatement;
}
//----- XResultSet: unsupported change detection methods ---------------------
sal_Bool SAL_CALL OResultSet::rowDeleted() throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionNotSupportedSQLException("rowDeleted not supported in firebird",
                                                  *this);
    return false;
}
sal_Bool SAL_CALL OResultSet::rowInserted() throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionNotSupportedSQLException("rowInserted not supported in firebird",
                                                  *this);
    return false;
}

sal_Bool SAL_CALL OResultSet::rowUpdated() throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionNotSupportedSQLException("rowUpdated not supported in firebird",
                                                  *this);
    return false;
}

void SAL_CALL OResultSet::refreshRow() throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionNotSupportedSQLException("refreshRow not supported in firebird",
                                                  *this);
}


void SAL_CALL OResultSet::cancel(  ) throw(RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}

//----- XWarningsSupplier UNSUPPORTED -----------------------------------------
#if 0
void SAL_CALL OResultSet::clearWarnings() throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionNotSupportedSQLException("clearWarnings not supported in firebird",
                                                  *this);
}

Any SAL_CALL OResultSet::getWarnings() throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionNotSupportedSQLException("getWarnings not supported in firebird",
                                                  *this);
    return Any();
}
#endif

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

uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

// ---- XServiceInfo -----------------------------------------------------------
OUString SAL_CALL OResultSet::getImplementationName() throw ( RuntimeException, std::exception)
{
    return OUString("com.sun.star.sdbcx.firebird.ResultSet");
}

Sequence< OUString > SAL_CALL OResultSet::getSupportedServiceNames()
    throw( RuntimeException, std::exception)
{
     Sequence< OUString > aSupported(2);
    aSupported[0] = "com.sun.star.sdbc.ResultSet";
    aSupported[1] = "com.sun.star.sdbcx.ResultSet";
    return aSupported;
}

sal_Bool SAL_CALL OResultSet::supportsService(const OUString& _rServiceName)
    throw( RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
