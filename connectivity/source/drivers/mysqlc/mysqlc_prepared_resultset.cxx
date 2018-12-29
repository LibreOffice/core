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

#include "mysqlc_propertyids.hxx"
#include "mysqlc_general.hxx"
#include "mysqlc_prepared_resultset.hxx"
#include "mysqlc_resultsetmetadata.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>

using namespace rtl;
#include <comphelper/string.hxx>

#include <cstdlib>

using namespace connectivity::mysqlc;
using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
using namespace ::comphelper;
using ::osl::MutexGuard;

#include <stdio.h>

rtl::OUString SAL_CALL OPreparedResultSet::getImplementationName()
{
    return rtl::OUString("com.sun.star.sdbcx.mysqlc.ResultSet");
}

uno::Sequence<rtl::OUString> SAL_CALL OPreparedResultSet::getSupportedServiceNames()
{
    uno::Sequence<rtl::OUString> aSupported(2);
    aSupported[0] = "com.sun.star.sdbc.ResultSet";
    aSupported[1] = "com.sun.star.sdbcx.ResultSet";
    return aSupported;
}

sal_Bool SAL_CALL OPreparedResultSet::supportsService(const rtl::OUString& _rServiceName)
{
    return cppu::supportsService(this, _rServiceName);
}
OPreparedResultSet::OPreparedResultSet(OConnection& rConn, OPreparedStatement* pPrepared,
                                       MYSQL_STMT* pStmt)
    : OPreparedResultSet_BASE(m_aMutex)
    , OPropertySetHelper(OPreparedResultSet_BASE::rBHelper)
    , m_rConnection(rConn)
    , m_aStatement(static_cast<OWeakObject*>(pPrepared))
    , m_xMetaData(nullptr)
    , m_pStmt(pStmt)
    , m_encoding(rConn.getConnectionEncoding())
{
    m_nFieldCount = mysql_stmt_field_count(pStmt);
    m_pResult = mysql_stmt_result_metadata(m_pStmt);
    m_aFields = mysql_fetch_fields(m_pResult);
}

void OPreparedResultSet::disposing()
{
    OPropertySetHelper::disposing();

    MutexGuard aGuard(m_aMutex);

    m_aStatement = nullptr;
    m_xMetaData = nullptr;
}

Any SAL_CALL OPreparedResultSet::queryInterface(const Type& rType)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if (!aRet.hasValue())
    {
        aRet = OPreparedResultSet_BASE::queryInterface(rType);
    }
    return aRet;
}

uno::Sequence<Type> SAL_CALL OPreparedResultSet::getTypes()
{
    OTypeCollection aTypes(cppu::UnoType<XMultiPropertySet>::get(),
                           cppu::UnoType<XFastPropertySet>::get(),
                           cppu::UnoType<XPropertySet>::get());

    return concatSequences(aTypes.getTypes(), OPreparedResultSet_BASE::getTypes());
}

sal_Int32 SAL_CALL OPreparedResultSet::findColumn(const rtl::OUString& columnName)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    MYSQL_FIELD* pFields = mysql_fetch_fields(m_pResult);
    for (sal_Int32 i = 0; i < m_nFieldCount; ++i)
    {
        if (columnName.equalsIgnoreAsciiCaseAscii(pFields[i].name))
            return i + 1; // sdbc indexes from 1
    }

    throw SQLException("The column name '" + columnName + "' is not valid.", *this, "42S22", 0,
                       Any());
}

uno::Reference<XInputStream> SAL_CALL OPreparedResultSet::getBinaryStream(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::getBinaryStream",
                                                            *this);
    return nullptr;
}

uno::Reference<XInputStream> SAL_CALL OPreparedResultSet::getCharacterStream(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException(
        "OPreparedResultSet::getCharacterStream", *this);
    return nullptr;
}

sal_Bool SAL_CALL OPreparedResultSet::getBoolean(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (*m_aData[column - 1].is_null)
    {
        m_bWasNull = true;
        return false;
    }
    m_bWasNull = false;
    return *reinterpret_cast<bool*>(m_aData[column - 1].buffer);
}

sal_Int8 SAL_CALL OPreparedResultSet::getByte(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (*m_aData[column - 1].is_null)
    {
        m_bWasNull = true;
        return 0;
    }
    m_bWasNull = false;
    return *reinterpret_cast<sal_Int8*>(m_aData[column - 1].buffer);
}

uno::Sequence<sal_Int8> SAL_CALL OPreparedResultSet::getBytes(sal_Int32 column)
{
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);

    if (*m_aData[column - 1].is_null)
    {
        m_bWasNull = true;
        return uno::Sequence<sal_Int8>();
    }
    m_bWasNull = false;

    return uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const*>(m_aData[column - 1].buffer),
                                   *m_aData[column - 1].length);
}

Date SAL_CALL OPreparedResultSet::getDate(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (*m_aData[column - 1].is_null)
    {
        m_bWasNull = true;
        return Date{}; // TODO init
    }
    m_bWasNull = false;

    const MYSQL_TIME* pTime = reinterpret_cast<MYSQL_TIME*>(m_aData[column - 1].buffer);

    assert(pTime != nullptr);

    Date d;
    d.Year = pTime->year;
    d.Month = pTime->month;
    d.Day = pTime->day;
    return d;
}

double SAL_CALL OPreparedResultSet::getDouble(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (*m_aData[column - 1].is_null)
    {
        m_bWasNull = true;
        return double{};
    }
    m_bWasNull = false;

    if (m_aFields[column - 1].type == MYSQL_TYPE_FLOAT)
        return *reinterpret_cast<float*>(m_aData[column - 1].buffer);

    return *reinterpret_cast<double*>(m_aData[column - 1].buffer);
}

float SAL_CALL OPreparedResultSet::getFloat(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (*m_aData[column - 1].is_null)
    {
        m_bWasNull = true;
        return float{};
    }
    m_bWasNull = false;

    return *reinterpret_cast<float*>(m_aData[column - 1].buffer);
}

sal_Int32 SAL_CALL OPreparedResultSet::getInt(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    if (*m_aData[column - 1].is_null)
    {
        m_bWasNull = true;
        return sal_Int32{};
    }
    m_bWasNull = false;

    return *reinterpret_cast<sal_Int32*>(m_aData[column - 1].buffer);
}

sal_Int32 SAL_CALL OPreparedResultSet::getRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    return static_cast<sal_Int32>(mysql_field_tell(m_pResult));
}

sal_Int64 SAL_CALL OPreparedResultSet::getLong(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (*m_aData[column - 1].is_null)
        return sal_Int64{};

    return *reinterpret_cast<sal_Int64*>(m_aData[column - 1].buffer);
}

uno::Reference<XResultSetMetaData> SAL_CALL OPreparedResultSet::getMetaData()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    if (!m_xMetaData.is())
    {
        m_xMetaData = new OResultSetMetaData(m_rConnection, m_pResult);
    }
    return m_xMetaData;
}

uno::Reference<XArray> SAL_CALL OPreparedResultSet::getArray(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::getArray", *this);
    return nullptr;
}

uno::Reference<XClob> SAL_CALL OPreparedResultSet::getClob(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::getClob", *this);
    return nullptr;
}

uno::Reference<XBlob> SAL_CALL OPreparedResultSet::getBlob(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::getBlob", *this);
    return nullptr;
}

uno::Reference<XRef> SAL_CALL OPreparedResultSet::getRef(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::getRef", *this);
    return nullptr;
}

Any SAL_CALL OPreparedResultSet::getObject(sal_Int32 column,
                                           const uno::Reference<XNameAccess>& /* typeMap */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    Any aRet = Any();

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::getObject", *this);
    return aRet;
}

sal_Int16 SAL_CALL OPreparedResultSet::getShort(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (*m_aData[column - 1].is_null)
    {
        m_bWasNull = true;
        return sal_Int16{};
    }
    m_bWasNull = false;

    return *reinterpret_cast<sal_Int16*>(m_aData[column - 1].buffer);
}

rtl::OUString SAL_CALL OPreparedResultSet::getString(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    if (*m_aData[column - 1].is_null)
    {
        m_bWasNull = true;
        return OUString{};
    }
    m_bWasNull = false;

    if (m_aFields[column - 1].type == MYSQL_TYPE_BIT)
    {
        if (*reinterpret_cast<sal_Int8*>(m_aData[column - 1].buffer) != 0)
            return OUString{ "YES" };
        return OUString{ "NO" };
    }

    const char* sStr = reinterpret_cast<const char*>(m_aData[column - 1].buffer);

    OUString sReturn = rtl::OUString(sStr, *m_aData[column - 1].length, m_encoding);
    return sReturn;
}

Time SAL_CALL OPreparedResultSet::getTime(sal_Int32 column)
{
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);

    if (*m_aData[column - 1].is_null)
    {
        m_bWasNull = true;
        return Time{}; // TODO init
    }
    m_bWasNull = false;

    const MYSQL_TIME* pTime = reinterpret_cast<MYSQL_TIME*>(m_aData[column - 1].buffer);

    assert(pTime != nullptr);

    Time t;
    t.Hours = pTime->hour;
    t.Minutes = pTime->minute;
    t.Seconds = pTime->second;
    return t;
}

DateTime SAL_CALL OPreparedResultSet::getTimestamp(sal_Int32 column)
{
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);

    if (*m_aData[column - 1].is_null)
    {
        m_bWasNull = true;
        return DateTime{};
    }
    m_bWasNull = false;

    const MYSQL_TIME* pTime = reinterpret_cast<MYSQL_TIME*>(m_aData[column - 1].buffer);

    assert(pTime != nullptr);

    DateTime t;
    t.Year = pTime->year;
    t.Month = pTime->month;
    t.Day = pTime->day;
    t.Hours = pTime->hour;
    t.Minutes = pTime->minute;
    t.Seconds = pTime->second;
    return t;
}

sal_Bool SAL_CALL OPreparedResultSet::isBeforeFirst()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    return m_nCurrentField == 0;
}

sal_Bool SAL_CALL OPreparedResultSet::isAfterLast()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    return m_nCurrentField >= m_nFieldCount;
}

sal_Bool SAL_CALL OPreparedResultSet::isFirst()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    return m_nCurrentField == 1 && !isAfterLast();
}

sal_Bool SAL_CALL OPreparedResultSet::isLast()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    return mysql_field_tell(m_pResult) == static_cast<unsigned>(m_nFieldCount);
}

void SAL_CALL OPreparedResultSet::beforeFirst()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::beforeFirst",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::afterLast()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::afterLast", *this);
}

void SAL_CALL OPreparedResultSet::close()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    m_aData.reset();
    m_aMetaData.reset();

    if (m_pResult)
        mysql_free_result(m_pResult);
    mysql_stmt_free_result(m_pStmt);
    dispose();
}

sal_Bool SAL_CALL OPreparedResultSet::first()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    mysql_stmt_data_seek(m_pStmt, 0);
    next();

    return true;
}

sal_Bool SAL_CALL OPreparedResultSet::last()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    mysql_stmt_data_seek(m_pStmt, m_nFieldCount - 1);
    next();

    return true;
}

sal_Bool SAL_CALL OPreparedResultSet::absolute(sal_Int32 row)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nFields = static_cast<sal_Int32>(m_nFieldCount);
    sal_Int32 nToGo = row < 0 ? nFields - row : row - 1;

    if (nToGo >= nFields)
        nToGo = nFields - 1;
    if (nToGo < 0)
        nToGo = 0;

    mysql_stmt_data_seek(m_pStmt, nToGo);
    next();

    return true;
}

sal_Bool SAL_CALL OPreparedResultSet::relative(sal_Int32 row)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nFields = static_cast<sal_Int32>(m_nFieldCount);
    if (row == 0)
        return true;

    sal_Int32 nToGo = m_nCurrentField + row;
    if (nToGo >= nFields)
        nToGo = nFields - 1;
    if (nToGo < 0)
        nToGo = 0;

    mysql_stmt_data_seek(m_pStmt, nToGo);
    next();
    m_nCurrentField += row;

    return true;
}

sal_Bool SAL_CALL OPreparedResultSet::previous()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    if (m_nCurrentField <= 1)
        return false;

    mysql_stmt_data_seek(m_pStmt, m_nCurrentField - 2);
    next();
    --m_nFieldCount;
    return true;
}

uno::Reference<uno::XInterface> SAL_CALL OPreparedResultSet::getStatement()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    return m_aStatement.get();
}

sal_Bool SAL_CALL OPreparedResultSet::rowDeleted()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL OPreparedResultSet::rowInserted()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL OPreparedResultSet::rowUpdated()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL OPreparedResultSet::next()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    bool bFirstRun = false;
    // allocate array if it does not exist
    if (m_aData == nullptr)
    {
        bFirstRun = true;
        m_aData.reset(new MYSQL_BIND[m_nFieldCount]);
        memset(m_aData.get(), 0, m_nFieldCount * sizeof(MYSQL_BIND));
        m_aMetaData.reset(new BindMetaData[m_nFieldCount]);
    }
    for (sal_Int32 i = 0; i < m_nFieldCount; ++i)
    {
        m_aMetaData[i].is_null = 0;
        m_aMetaData[i].length = 0l;
        m_aMetaData[i].error = 0;

        m_aData[i].is_null = &m_aMetaData[i].is_null;
        m_aData[i].buffer_length = m_aFields[i].type == MYSQL_TYPE_BLOB ? 0 : m_aFields[i].length;
        m_aData[i].length = &m_aMetaData[i].length;
        m_aData[i].error = &m_aMetaData[i].error;
        m_aData[i].buffer = nullptr;
        m_aData[i].buffer_type = m_aFields[i].type;

        // allocates memory, if it is a fixed size type. If not then nullptr
        mysqlc_sdbc_driver::allocateSqlVar(&m_aData[i].buffer, m_aData[i].buffer_type,
                                           m_aFields[i].length);
    }
    mysql_stmt_bind_result(m_pStmt, m_aData.get());
    if (bFirstRun)
        mysql_stmt_store_result(m_pStmt);
    int failure = mysql_stmt_fetch(m_pStmt);

    for (sal_Int32 i = 0; i < m_nFieldCount; ++i)
    {
        if (*m_aData[i].error)
        {
            // expected if we have a BLOB, as buffer_length is set to 0. We want to
            // fetch it piece by piece
            // see https://bugs.mysql.com/file.php?id=12361&bug_id=33086
            if (m_aData[i].buffer == nullptr)
            {
                m_aData[i].buffer_length = *m_aData[i].length;
                m_aData[i].buffer = malloc(*m_aData[i].length);
                mysql_stmt_fetch_column(m_pStmt, &m_aData[i], i, 0);
            }
        }
    }

    if (failure == 1)
    {
        MYSQL* pMysql = m_rConnection.getMysqlConnection();
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(pMysql), mysql_errno(pMysql),
                                                     *this, m_encoding);
    }
    else if (failure == MYSQL_NO_DATA)
        return false;

    // current field cannot be asked as a number. We have to keep track it
    // manually.
    m_nCurrentField += 1;

    return true;
}

sal_Bool SAL_CALL OPreparedResultSet::wasNull()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
}

void SAL_CALL OPreparedResultSet::cancel()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL OPreparedResultSet::clearWarnings() {}

Any SAL_CALL OPreparedResultSet::getWarnings()
{
    Any aRet = Any();
    return aRet;
}

void SAL_CALL OPreparedResultSet::insertRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    // you only have to implement this if you want to insert new rows
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::insertRow", *this);
}

void SAL_CALL OPreparedResultSet::updateRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    // only when you allow updates
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateRow", *this);
}

void SAL_CALL OPreparedResultSet::deleteRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::deleteRow", *this);
}

void SAL_CALL OPreparedResultSet::cancelRowUpdates()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::cancelRowUpdates",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::moveToInsertRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    // only when you allow insert's
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::moveToInsertRow",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::moveToCurrentRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL OPreparedResultSet::updateNull(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateNull",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateBoolean(sal_Int32 column, sal_Bool /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateBoolean",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateByte(sal_Int32 column, sal_Int8 /* x */)
{
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateByte",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateShort(sal_Int32 column, sal_Int16 /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateShort",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateInt(sal_Int32 column, sal_Int32 /* x */)
{
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateInt", *this);
}

void SAL_CALL OPreparedResultSet::updateLong(sal_Int32 column, sal_Int64 /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateLong",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateFloat(sal_Int32 column, float /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateFloat",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateDouble(sal_Int32 column, double /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateDouble",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateString(sal_Int32 column, const rtl::OUString& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateString",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateBytes(sal_Int32 column,
                                              const uno::Sequence<sal_Int8>& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateBytes",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateDate(sal_Int32 column, const Date& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateDate",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateTime(sal_Int32 column, const Time& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateTime",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateTimestamp(sal_Int32 column, const DateTime& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateTimestamp",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateBinaryStream(sal_Int32 column,
                                                     const uno::Reference<XInputStream>& /* x */,
                                                     sal_Int32 /* length */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException(
        "OPreparedResultSet::updateBinaryStream", *this);
}

void SAL_CALL OPreparedResultSet::updateCharacterStream(sal_Int32 column,
                                                        const uno::Reference<XInputStream>& /* x */,
                                                        sal_Int32 /* length */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException(
        "OPreparedResultSet::updateCharacterStream", *this);
}

void SAL_CALL OPreparedResultSet::refreshRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::refreshRow",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateObject(sal_Int32 column, const Any& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::updateObject",
                                                            *this);
}

void SAL_CALL OPreparedResultSet::updateNumericObject(sal_Int32 column, const Any& /* x */,
                                                      sal_Int32 /* scale */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException(
        "OPreparedResultSet::updateNumericObject", *this);
}

// XRowLocate
Any SAL_CALL OPreparedResultSet::getBookmark()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    Any aRet = Any();

    // if you don't want to support bookmark you must remove the XRowLocate interface
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::getBookmark",
                                                            *this);

    return aRet;
}

sal_Bool SAL_CALL OPreparedResultSet::moveToBookmark(const Any& /* bookmark */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL OPreparedResultSet::moveRelativeToBookmark(const Any& /* bookmark */,
                                                             sal_Int32 /* rows */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException(
        "OPreparedResultSet::moveRelativeToBookmark", *this);
    return false;
}

sal_Int32 SAL_CALL OPreparedResultSet::compareBookmarks(const Any& /* n1 */, const Any& /* n2 */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::compareBookmarks",
                                                            *this);

    return CompareBookmark::NOT_EQUAL;
}

sal_Bool SAL_CALL OPreparedResultSet::hasOrderedBookmarks() { return false; }

sal_Int32 SAL_CALL OPreparedResultSet::hashBookmark(const Any& /* bookmark */)
{
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::hashBookmark",
                                                            *this);
    return 0;
}

// XDeleteRows
uno::Sequence<sal_Int32>
    SAL_CALL OPreparedResultSet::deleteRows(const uno::Sequence<Any>& /* rows */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedResultSet_BASE::rBHelper.bDisposed);
    uno::Sequence<sal_Int32> aRet = uno::Sequence<sal_Int32>();

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedResultSet::deleteRows",
                                                            *this);
    return aRet;
}

IPropertyArrayHelper* OPreparedResultSet::createArrayHelper() const
{
    uno::Sequence<Property> aProps(5);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++] = Property("FetchDirection", PROPERTY_ID_FETCHDIRECTION,
                                   cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++]
        = Property("FetchSize", PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = Property("IsBookmarkable", PROPERTY_ID_ISBOOKMARKABLE,
                                   cppu::UnoType<bool>::get(), PropertyAttribute::READONLY);
    pProperties[nPos++] = Property("ResultSetConcurrency", PROPERTY_ID_RESULTSETCONCURRENCY,
                                   cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);
    pProperties[nPos++] = Property("ResultSetType", PROPERTY_ID_RESULTSETTYPE,
                                   cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);

    return new OPropertyArrayHelper(aProps);
}

IPropertyArrayHelper& OPreparedResultSet::getInfoHelper() { return *getArrayHelper(); }

sal_Bool OPreparedResultSet::convertFastPropertyValue(Any& /* rConvertedValue */,
                                                      Any& /* rOldValue */, sal_Int32 nHandle,
                                                      const Any& /* rValue */)
{
    switch (nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw css::lang::IllegalArgumentException();
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        default:;
    }
    return false;
}

void OPreparedResultSet::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,
                                                          const Any& /* rValue */)
{
    switch (nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw uno::Exception("cannot set prop " + rtl::OUString::number(nHandle), nullptr);
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_FETCHSIZE:
            break;
        default:;
    }
}

void OPreparedResultSet::getFastPropertyValue(Any& _rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
            _rValue <<= false;
            break;
        case PROPERTY_ID_CURSORNAME:
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            _rValue <<= ResultSetConcurrency::READ_ONLY;
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            _rValue <<= ResultSetType::SCROLL_INSENSITIVE;
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            _rValue <<= FetchDirection::FORWARD;
            break;
        case PROPERTY_ID_FETCHSIZE:
            _rValue <<= sal_Int32(50);
            break;
            ;
        default:;
    }
}

void SAL_CALL OPreparedResultSet::acquire() throw() { OPreparedResultSet_BASE::acquire(); }

void SAL_CALL OPreparedResultSet::release() throw() { OPreparedResultSet_BASE::release(); }

css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL OPreparedResultSet::getPropertySetInfo()
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

void OPreparedResultSet::checkColumnIndex(sal_Int32 index)
{
    if (!m_aData)
        throw SQLException("Cursor out of range", *this, rtl::OUString(), 1, Any());
    if (index < 1 || index > static_cast<int>(m_nFieldCount))
    {
        /* static object for efficiency or thread safety is a problem ? */
        throw SQLException("index out of range", *this, rtl::OUString(), 1, Any());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
