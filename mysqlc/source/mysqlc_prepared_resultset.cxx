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
#include "mysqlc_resultset.hxx"
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

#include <cppconn/resultset.h>
#include <cppconn/resultset_metadata.h>

#include <stdio.h>

rtl::OUString SAL_CALL OResultSet::getImplementationName()
{
    return rtl::OUString("com.sun.star.sdbcx.mysqlc.ResultSet");
}

uno::Sequence<rtl::OUString> SAL_CALL OResultSet::getSupportedServiceNames()
{
    uno::Sequence<rtl::OUString> aSupported(2);
    aSupported[0] = "com.sun.star.sdbc.ResultSet";
    aSupported[1] = "com.sun.star.sdbcx.ResultSet";
    return aSupported;
}

sal_Bool SAL_CALL OResultSet::supportsService(const rtl::OUString& _rServiceName)
{
    return cppu::supportsService(this, _rServiceName);
}

OResultSet::OResultSet(OConnection& rConn, OPreparedStatement* pPrepared, MYSQL_STMT* pStmt)
    : OResultSet_BASE(m_aMutex)
    , OPropertySetHelper(OResultSet_BASE::rBHelper)
    , m_rConnection(rConn)
    , m_aStatement(static_cast<OWeakObject*>(pPrepared))
    , m_xMetaData(nullptr)
    , m_pResult(pResult)
    , m_encoding(rConn.getConnectionEncoding())
{
    m_nFieldCount = mysql_stmt_field_count(pStmt);
}

OResultSet::~OResultSet() {}

void OResultSet::disposing()
{
    OPropertySetHelper::disposing();

    MutexGuard aGuard(m_aMutex);

    m_aStatement = nullptr;
    m_xMetaData = nullptr;
}

Any SAL_CALL OResultSet::queryInterface(const Type& rType)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if (!aRet.hasValue())
    {
        aRet = OResultSet_BASE::queryInterface(rType);
    }
    return aRet;
}

uno::Sequence<Type> SAL_CALL OResultSet::getTypes()
{
    OTypeCollection aTypes(cppu::UnoType<XMultiPropertySet>::get(),
                           cppu::UnoType<XFastPropertySet>::get(),
                           cppu::UnoType<XPropertySet>::get());

    return concatSequences(aTypes.getTypes(), OResultSet_BASE::getTypes());
}

sal_Int32 SAL_CALL OResultSet::findColumn(const rtl::OUString& columnName)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    MYSQL_FIELD* pFields = mysql_fetch_field(m_pResult);
    for (unsigned int i = 0; i < m_rData.size(); ++i)
    {
        if (columnName.equalsIgnoreAsciiCaseAscii(pFields[i].name))
            return i + 1; // sdbc indexes from 1
    }

    throw SQLException("The column name '" + columnName + "' is not valid.", *this,
                       rtl::OUString("42S22"), 0, Any());
}

uno::Reference<XInputStream> SAL_CALL OResultSet::getBinaryStream(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getBinaryStream", *this);
    return nullptr;
}

uno::Reference<XInputStream> SAL_CALL OResultSet::getCharacterStream(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getCharacterStream",
                                                            *this);
    return nullptr;
}

sal_Bool SAL_CALL OResultSet::getBoolean(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (m_aData.at(column - 1).is_null)
        return false;
    return *reinterpret_cast<bool*>(m_aData.at(column - 1).buffer);
}

uno::Sequence<sal_Int8> SAL_CALL OResultSet::getBytes(sal_Int32 column)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);

    if (m_aData.at(column - 1).is_null)
        return uno::Sequence<sal_Int8>();

    return uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const*>(m_aData[column - 1].buffer),
                                   m_aData[column - 1].buffer_length);
}

Date SAL_CALL OResultSet::getDate(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (m_aData.at(column - 1).is_null)
        return Date{};

    const MYSQL_TIME* pTime = reinterpret_cast<MYSQL_TIME*>(m_aData.at(column - 1).buffer);

    assert(pTime != nullptr);

    Date d;
    d.Year = pTime->year;
    d.Month = pTime->month;
    d.Day = pTime->day;
    return d;
}

double SAL_CALL OResultSet::getDouble(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (m_aData.at(column - 1).is_null)
        return double{};

    return *reinterpret_cast<double*>(m_aData.at(column - 1).buffer);
}

float SAL_CALL OResultSet::getFloat(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (m_aData.at(column - 1).is_null)
        return float{};

    return *reinterpret_cast<float*>(m_aData.at(column - 1).buffer);
}

sal_Int32 SAL_CALL OResultSet::getInt(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    if (m_aData.at(column - 1).is_null)
        return sal_Int32{};

    return reinterpret_cast<sal_Int32*>(m_aData.at(column - 1).buffer);
}

sal_Int32 SAL_CALL OResultSet::getRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return static_cast<sal_Int32>(mysql_field_tell(m_pResult));
}

sal_Int64 SAL_CALL OResultSet::getLong(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (m_aData.at(column - 1).is_null)
        return sal_Int64{};

    return reinterpret_cast<sal_Int64*>(m_aData.at(column - 1).buffer);
}

uno::Reference<XResultSetMetaData> SAL_CALL OResultSet::getMetaData()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    if (!m_xMetaData.is())
    {
        m_xMetaData = new OResultSetMetaData(m_rConnection, m_pResult, m_encoding);
    }
    return m_xMetaData;
}

uno::Reference<XArray> SAL_CALL OResultSet::getArray(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getArray", *this);
    return nullptr;
}

uno::Reference<XClob> SAL_CALL OResultSet::getClob(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getClob", *this);
    return nullptr;
}

uno::Reference<XBlob> SAL_CALL OResultSet::getBlob(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getBlob", *this);
    return nullptr;
}

uno::Reference<XRef> SAL_CALL OResultSet::getRef(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getRef", *this);
    return nullptr;
}

Any SAL_CALL OResultSet::getObject(sal_Int32 column,
                                   const uno::Reference<XNameAccess>& /* typeMap */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    Any aRet = Any();

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getObject", *this);
    return aRet;
}

sal_Int16 SAL_CALL OResultSet::getShort(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    if (m_aData.at(column - 1).is_null)
        return sal_Int16{};

    return reinterpret_cast<sal_Int16*>(m_aData.at(column - 1).buffer);
}

rtl::OUString SAL_CALL OResultSet::getString(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    if (m_aData.at(column - 1).is_null)
        return OUString{};

    const char* sStr = reinterpret_cast<const char*>(m_aData.at(column - 1).buffer);

    return rtl::OUString(sStr, m_aData.at(column - 1).buffer_length, m_encoding);
}

Time SAL_CALL OResultSet::getTime(sal_Int32 column)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);

    if (m_aData.at(column - 1).is_null)
        return Time{};

    const MYSQL_TIME* pTime = reinterpret_cast<MYSQL_TIME*>(m_aData.at(column - 1).buffer);

    assert(pTime != nullptr);

    Time t;
    t.Hours = pTime->hour;
    t.Minutes = pTime->minute;
    t.Seconds = pTime->second;
    return t;
}

DateTime SAL_CALL OResultSet::getTimestamp(sal_Int32 column)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);

    if (m_aData.at(column - 1).is_null)
        return DateTime{};

    const MYSQL_TIME* pTime = reinterpret_cast<MYSQL_TIME*>(m_aData.at(column - 1).buffer);

    assert(pTime != nullptr);

    DateTime t;
    t.Year = pTime->year;
    t.Month = pTime->month;
    t.Day = pTime->day;
    t.Hours = pTime->hour;
    t.Minutes = pTime->minute;
    t.Seconds = pTime->second;
    return t;

    if (!pValue)
        return DateTime{};

    // YY-MM-DD HH:MM:SS
    std::vector<rtl::OUString> dateAndTime = string::split(
        rtl::OUString{ pValue, static_cast<sal_Int32>(m_aLengths[column - 1]), m_encoding }, u' ');

    auto dateParts = string::split(dateAndTime.at(0), u'-');
    auto timeParts = string::split(dateAndTime.at(1), u':');

    DateTime dt;

    dt.Year = dateParts.at(0).toUInt32();
    dt.Month = dateParts.at(1).toUInt32();
    dt.Day = dateParts.at(2).toUInt32();
    dt.Hours = timeParts.at(0).toUInt32();
    dt.Minutes = timeParts.at(1).toUInt32();
    dt.Seconds = timeParts.at(2).toUInt32();
    return dt;
}

sal_Bool SAL_CALL OResultSet::isBeforeFirst()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nCurrentField == 0;
}

sal_Bool SAL_CALL OResultSet::isAfterLast()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nCurrentField >= static_cast<sal_Int32>(fieldCount);
}

sal_Bool SAL_CALL OResultSet::isFirst()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nCurrentField == 1 && !isAfterLast();
}

sal_Bool SAL_CALL OResultSet::isLast()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return mysql_field_tell(m_pResult) == fieldCount;
}

void SAL_CALL OResultSet::beforeFirst()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::beforeFirst", *this);
}

void SAL_CALL OResultSet::afterLast()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::afterLast", *this);
}

void SAL_CALL OResultSet::close()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    mysql_free_result(m_pResult);
    dispose();
}

sal_Bool SAL_CALL OResultSet::first()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    mysql_stmt_data_seek(m_pResult, 0);
    next();

    return true;
}

sal_Bool SAL_CALL OResultSet::last()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    mysql_stmt_data_seek(m_pResult, fieldCount - 1);
    next();

    return true;
}

sal_Bool SAL_CALL OResultSet::absolute(sal_Int32 row)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nFields = static_cast<sal_Int32>(fieldCount);
    sal_Int32 nToGo = row < 0 ? nFields - row : row - 1;

    if (nToGo >= nFields)
        nToGo = nFields - 1;
    if (nToGo < 0)
        nToGo = 0;

    mysql_stmt_data_seek(m_pResult, nToGo);
    next();

    return true;
}

sal_Bool SAL_CALL OResultSet::relative(sal_Int32 row)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nFields = static_cast<sal_Int32>(fieldCount);
    if (row == 0)
        return true;

    sal_Int32 nToGo = m_nCurrentField + row;
    if (nToGo >= nFields)
        nToGo = nFields - 1;
    if (nToGo < 0)
        nToGo = 0;

    mysql_stmt_data_seek(m_pResult, nToGo);
    next();

    return true;
}

sal_Bool SAL_CALL OResultSet::previous()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (m_nCurrentField <= 1)
        return false;

    mysql_stmt_data_seek(m_pResult, m_nCurrentField - 2);
    next();
    return true;
}

uno::Reference<uno::XInterface> SAL_CALL OResultSet::getStatement()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_aStatement.get();
}

sal_Bool SAL_CALL OResultSet::rowDeleted()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL OResultSet::rowInserted()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL OResultSet::rowUpdated()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL OResultSet::next()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    unsigned errorNum = mysql_errno(m_pMysql);
    if (errorNum)
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(m_pMysql), errorNum, *this,
                                                     m_encoding);

    m_pResult = mysql_stmt_result_metadata(m_pStmt);

    m_aData.clear();
    MYSQL_FIELD* aFields = mysql_fetch_fields(m_pResult);
    for (sal_Int32 i = 0; i < m_nFieldCount; ++i)
    {
        m_aData.push_back(MYSQL_BIND{});
        m_aMetaData.push_back(RowMetaData{});

        m_aData.back().is_null = &m_aMetaData.back().is_null;
        m_aData.back().length = &m_aMetaData.back().length;
        m_aData.back().error = &m_aMetaData.back().error;
        m_aData.back().buffer = nullptr;
        m_aData.back().buffer_type = aFields[i].type;
    }
    mysql_stmt_bind_result(
        m_pStmt, m_aData.data();
        int failure = mysql_stmt_fetch(m_pStmt); // determine buffer lengths

        if (failure == 1) {
            mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(m_pMysql),
                                                         mysql_errno(m_pMysql), *this, m_encoding);
        } else if (failure == MYSQL_NO_DATA) return false;

        m_nCurrentField = mysql_stmt_row_tell(m_pStmt);

        for (size_t i = 0; i < m_nFieldCount; ++i))
    {
        if (m_aMetaData[i].length > 0)
        {
            m_aData[i].buffer = malloc(m_aMetaData[i].length);
            m_aData[i].buffer_length = m_aMetaData[i].length;

            // fetch actual data
            mysql_stmt_fetch_column(m_pStmt, &m_aData[i], i, 0);
        }
    }

    return true;
}

sal_Bool SAL_CALL OResultSet::wasNull()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_aRow == nullptr;
}

void SAL_CALL OResultSet::cancel()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL OResultSet::clearWarnings() {}

Any SAL_CALL OResultSet::getWarnings()
{
    Any aRet = Any();
    return aRet;
}

void SAL_CALL OResultSet::insertRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    // you only have to implement this if you want to insert new rows
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::insertRow", *this);
}

void SAL_CALL OResultSet::updateRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // only when you allow updates
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateRow", *this);
}

void SAL_CALL OResultSet::deleteRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::deleteRow", *this);
}

void SAL_CALL OResultSet::cancelRowUpdates()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::cancelRowUpdates", *this);
}

void SAL_CALL OResultSet::moveToInsertRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // only when you allow insert's
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::moveToInsertRow", *this);
}

void SAL_CALL OResultSet::moveToCurrentRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL OResultSet::updateNull(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateNull", *this);
}

void SAL_CALL OResultSet::updateBoolean(sal_Int32 column, sal_Bool /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateBoolean", *this);
}

void SAL_CALL OResultSet::updateByte(sal_Int32 column, sal_Int8 /* x */)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateByte", *this);
}

void SAL_CALL OResultSet::updateShort(sal_Int32 column, sal_Int16 /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateShort", *this);
}

void SAL_CALL OResultSet::updateInt(sal_Int32 column, sal_Int32 /* x */)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateInt", *this);
}

void SAL_CALL OResultSet::updateLong(sal_Int32 column, sal_Int64 /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateLong", *this);
}

void SAL_CALL OResultSet::updateFloat(sal_Int32 column, float /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateFloat", *this);
}

void SAL_CALL OResultSet::updateDouble(sal_Int32 column, double /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateDouble", *this);
}

void SAL_CALL OResultSet::updateString(sal_Int32 column, const rtl::OUString& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateString", *this);
}

void SAL_CALL OResultSet::updateBytes(sal_Int32 column, const uno::Sequence<sal_Int8>& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateBytes", *this);
}

void SAL_CALL OResultSet::updateDate(sal_Int32 column, const Date& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateDate", *this);
}

void SAL_CALL OResultSet::updateTime(sal_Int32 column, const Time& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateTime", *this);
}

void SAL_CALL OResultSet::updateTimestamp(sal_Int32 column, const DateTime& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateTimestamp", *this);
}

void SAL_CALL OResultSet::updateBinaryStream(sal_Int32 column,
                                             const uno::Reference<XInputStream>& /* x */,
                                             sal_Int32 /* length */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateBinaryStream",
                                                            *this);
}

void SAL_CALL OResultSet::updateCharacterStream(sal_Int32 column,
                                                const uno::Reference<XInputStream>& /* x */,
                                                sal_Int32 /* length */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateCharacterStream",
                                                            *this);
}

void SAL_CALL OResultSet::refreshRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::refreshRow", *this);
}

void SAL_CALL OResultSet::updateObject(sal_Int32 column, const Any& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateObject", *this);
}

void SAL_CALL OResultSet::updateNumericObject(sal_Int32 column, const Any& /* x */,
                                              sal_Int32 /* scale */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateNumericObject",
                                                            *this);
}

// XRowLocate
Any SAL_CALL OResultSet::getBookmark()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    Any aRet = Any();

    // if you don't want to support bookmark you must remove the XRowLocate interface
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getBookmark", *this);

    return aRet;
}

sal_Bool SAL_CALL OResultSet::moveToBookmark(const Any& /* bookmark */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL OResultSet::moveRelativeToBookmark(const Any& /* bookmark */,
                                                     sal_Int32 /* rows */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::moveRelativeToBookmark",
                                                            *this);
    return false;
}

sal_Int32 SAL_CALL OResultSet::compareBookmarks(const Any& /* n1 */, const Any& /* n2 */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::compareBookmarks", *this);

    return CompareBookmark::NOT_EQUAL;
}

sal_Bool SAL_CALL OResultSet::hasOrderedBookmarks() { return false; }

sal_Int32 SAL_CALL OResultSet::hashBookmark(const Any& /* bookmark */)
{
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::hashBookmark", *this);
    return 0;
}

// XDeleteRows
uno::Sequence<sal_Int32> SAL_CALL OResultSet::deleteRows(const uno::Sequence<Any>& /* rows */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    uno::Sequence<sal_Int32> aRet = uno::Sequence<sal_Int32>();

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::deleteRows", *this);
    return aRet;
}

IPropertyArrayHelper* OResultSet::createArrayHelper() const
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

IPropertyArrayHelper& OResultSet::getInfoHelper() { return *getArrayHelper(); }

sal_Bool OResultSet::convertFastPropertyValue(Any& /* rConvertedValue */, Any& /* rOldValue */,
                                              sal_Int32 nHandle, const Any& /* rValue */)
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

void OResultSet::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& /* rValue */)
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

void OResultSet::getFastPropertyValue(Any& _rValue, sal_Int32 nHandle) const
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

void SAL_CALL OResultSet::acquire() throw() { OResultSet_BASE::acquire(); }

void SAL_CALL OResultSet::release() throw() { OResultSet_BASE::release(); }

css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL OResultSet::getPropertySetInfo()
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

void OResultSet::checkColumnIndex(sal_Int32 index)
{
    if (index < 1 || index > static_cast<int>(fieldCount))
    {
        /* static object for efficiency or thread safety is a problem ? */
        throw SQLException("index out of range", *this, rtl::OUString(), 1, Any());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
