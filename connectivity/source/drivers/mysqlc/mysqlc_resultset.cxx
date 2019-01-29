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
#include <comphelper/seqstream.hxx>

#include <sal/log.hxx>

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
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace ::comphelper;
using ::osl::MutexGuard;

#include <stdio.h>

namespace
{
// copied from string misc, it should be replaced when library is not an
// extension anymore
std::vector<OString> lcl_split(const OString& rStr, char cSeparator)
{
    std::vector<OString> vec;
    sal_Int32 idx = 0;
    do
    {
        OString kw = rStr.getToken(0, cSeparator, idx);
        kw = kw.trim();
        if (!kw.isEmpty())
        {
            vec.push_back(kw);
        }
    } while (idx >= 0);
    return vec;
}
}

void OResultSet::checkRowIndex()
{
    if (m_nRowPosition < 0 || m_nRowPosition >= m_nRowCount)
    {
        throw SQLException("Cursor position out of range", *this, OUString(), 1, Any());
    }
}

bool OResultSet::checkNull(sal_Int32 column)
{
    if (m_aRows[m_nRowPosition][column - 1].isEmpty())
    {
        m_bWasNull = true;
        return true;
    }
    m_bWasNull = false;
    return false;
}

OUString SAL_CALL OResultSet::getImplementationName()
{
    return OUString("com.sun.star.sdbcx.mysqlc.ResultSet");
}

uno::Sequence<OUString> SAL_CALL OResultSet::getSupportedServiceNames()
{
    uno::Sequence<OUString> aSupported(2);
    aSupported[0] = "com.sun.star.sdbc.ResultSet";
    aSupported[1] = "com.sun.star.sdbcx.ResultSet";
    return aSupported;
}

sal_Bool SAL_CALL OResultSet::supportsService(const OUString& _rServiceName)
{
    return cppu::supportsService(this, _rServiceName);
}

OResultSet::OResultSet(OConnection& rConn, OCommonStatement* pStmt, MYSQL_RES* pResult,
                       rtl_TextEncoding _encoding)
    : OResultSet_BASE(m_aMutex)
    , OPropertySetHelper(OResultSet_BASE::rBHelper)
    , m_pMysql(rConn.getMysqlConnection())
    , m_aStatement(static_cast<OWeakObject*>(pStmt))
    , m_pResult(pResult)
    , m_encoding(_encoding)
{
    m_xMetaData = new OResultSetMetaData(rConn, m_pResult);
}

void OResultSet::ensureResultFetched()
{
    if (!m_bResultFetched)
    {
        fetchResult();
    }
}

void OResultSet::ensureFieldInfoFetched()
{
    if (m_bResultFetched)
        return; // already fetched

    // it works only if result set is produced via mysql_store_result
    // TODO ensure that
    m_nRowCount = mysql_num_rows(m_pResult);

    if (!m_aFields.empty())
        return;
    unsigned nFieldCount = mysql_num_fields(m_pResult);
    MYSQL_FIELD* pFields = mysql_fetch_fields(m_pResult);
    m_aFields.reserve(nFieldCount);
    for (unsigned i = 0; i < nFieldCount; ++i)
        m_aFields.push_back(OUString{
            pFields[i].name, static_cast<sal_Int32>(strlen(pFields[i].name)), m_encoding });
}

void OResultSet::fetchResult()
{
    // Mysql C API does not allow simultaneously opened result sets, but sdbc does.
    // Because of that we need to fetch all of the data ASAP
    ensureFieldInfoFetched();

    // fetch all the data
    m_aRows.reserve(m_nRowCount);

    for (sal_Int32 row = 0; row < m_nRowCount; ++row)
    {
        MYSQL_ROW data = mysql_fetch_row(m_pResult);
        unsigned long* lengths = mysql_fetch_lengths(m_pResult);
        m_aRows.push_back(DataFields{});
        // MYSQL_ROW is char**, array of strings
        for (std::size_t col = 0; col < m_aFields.size(); ++col)
        {
            m_aRows.back().push_back(OString{ data[col], static_cast<sal_Int32>(lengths[col]) });
        }
    }
    unsigned errorNum = mysql_errno(m_pMysql);
    if (errorNum)
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(m_pMysql), errorNum, *this,
                                                     m_encoding);
    m_bResultFetched = true;
    mysql_free_result(m_pResult);
}

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

sal_Int32 SAL_CALL OResultSet::findColumn(const OUString& columnName)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ensureFieldInfoFetched();

    for (std::size_t i = 0; i < m_aFields.size(); ++i)
    {
        if (columnName.equalsIgnoreAsciiCase(m_aFields[i]))
            return static_cast<sal_Int32>(i) + 1; // sdbc indexes from 1
    }

    throw SQLException("The column name '" + columnName + "' is not valid.", *this, "42S22", 0,
                       Any());
}

uno::Reference<XInputStream> SAL_CALL OResultSet::getBinaryStream(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);
    if (checkNull(column))
        return nullptr;

    OString sVal = m_aRows[m_nRowPosition][column - 1];
    return new SequenceInputStream{ uno::Sequence<sal_Int8>(
        reinterpret_cast<sal_Int8 const*>(sVal.getStr()), getDataLength(column)) };
}

uno::Reference<XInputStream> SAL_CALL OResultSet::getCharacterStream(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getCharacterStream",
                                                            *this);
    return nullptr;
}

sal_Bool SAL_CALL OResultSet::getBoolean(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);
    if (checkNull(column))
        return false;

    OString sVal = m_aRows[m_nRowPosition][column - 1];
    return sVal.toInt32() != 0;
}

sal_Int8 SAL_CALL OResultSet::getByte(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);
    if (checkNull(column))
        return 0;

    OString sVal = m_aRows[m_nRowPosition][column - 1];

    return static_cast<sal_Int8>(sVal.toInt32());
}

uno::Sequence<sal_Int8> SAL_CALL OResultSet::getBytes(sal_Int32 column)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkBordersAndEnsureFetched(column);
    OString sVal = m_aRows[m_nRowPosition][column - 1];
    if (checkNull(column))
        return uno::Sequence<sal_Int8>();

    return uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const*>(sVal.getStr()),
                                   getDataLength(column));
}

Date SAL_CALL OResultSet::getDate(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);

    Date d;

    if (checkNull(column))
        return d;

    OString dateStr = m_aRows[m_nRowPosition][column - 1];

    OString dateString(dateStr);
    OString token;
    sal_Int32 nIndex = 0, i = 0;
    do
    {
        token = dateString.getToken(0, '-', nIndex);
        switch (i)
        {
            case 0:
                d.Year = static_cast<sal_uInt16>(token.toUInt32());
                break;
            case 1:
                d.Month = static_cast<sal_uInt16>(token.toUInt32());
                break;
            case 2:
                d.Day = static_cast<sal_uInt16>(token.toUInt32());
                break;
            default:;
        }
        i++;
    } while (nIndex >= 0);
    return d;
}

double SAL_CALL OResultSet::getDouble(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);

    if (checkNull(column))
        return 0.0;

    OString sVal = m_aRows[m_nRowPosition][column - 1];
    return sVal.toDouble();
}

float SAL_CALL OResultSet::getFloat(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);
    if (checkNull(column))
        return 0.0f;

    OString sVal = m_aRows[m_nRowPosition][column - 1];
    return sVal.toFloat();
}

sal_Int32 SAL_CALL OResultSet::getInt(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);
    if (checkNull(column))
        return 0;

    OString sVal = m_aRows[m_nRowPosition][column - 1];
    return sVal.toInt32();
}

sal_Int32 SAL_CALL OResultSet::getRow()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nRowPosition + 1; // indexed from 1
}

sal_Int64 SAL_CALL OResultSet::getLong(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);
    if (checkNull(column))
        return 0LL;

    OString sVal = m_aRows[m_nRowPosition][column - 1];
    return sVal.toInt64();
}

uno::Reference<XResultSetMetaData> SAL_CALL OResultSet::getMetaData()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_xMetaData;
}

uno::Reference<XArray> SAL_CALL OResultSet::getArray(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getArray", *this);
    return nullptr;
}

uno::Reference<XClob> SAL_CALL OResultSet::getClob(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getClob", *this);
    return nullptr;
}

uno::Reference<XBlob> SAL_CALL OResultSet::getBlob(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getBlob", *this);
    return nullptr;
}

uno::Reference<XRef> SAL_CALL OResultSet::getRef(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getRef", *this);
    return nullptr;
}

Any SAL_CALL OResultSet::getObject(sal_Int32 column,
                                   const uno::Reference<XNameAccess>& /* typeMap */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);

    Any aRet = Any();

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getObject", *this);
    return aRet;
}

sal_Int16 SAL_CALL OResultSet::getShort(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);
    if (checkNull(column))
        return 0;

    OString sVal = m_aRows[m_nRowPosition][column - 1];
    return sVal.toInt32();
}

OUString SAL_CALL OResultSet::getString(sal_Int32 column)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkBordersAndEnsureFetched(column);
    if (checkNull(column))
        return rtl::OUString{};

    OString sVal = m_aRows[m_nRowPosition][column - 1];
    return OStringToOUString(sVal, m_encoding);
}

Time SAL_CALL OResultSet::getTime(sal_Int32 column)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkBordersAndEnsureFetched(column);

    Time t;
    if (checkNull(column))
        return t;

    OString sVal = m_aRows[m_nRowPosition][column - 1];
    OString timeString{ sVal.getStr(), getDataLength(column) };
    OString token;
    sal_Int32 nIndex, i = 0;

    nIndex = timeString.indexOf(' ') + 1;
    do
    {
        token = timeString.getToken(0, ':', nIndex);
        switch (i)
        {
            case 0:
                t.Hours = static_cast<sal_uInt16>(token.toUInt32());
                break;
            case 1:
                t.Minutes = static_cast<sal_uInt16>(token.toUInt32());
                break;
            case 2:
                t.Seconds = static_cast<sal_uInt16>(token.toUInt32());
                break;
        }
        i++;
    } while (nIndex >= 0);

    return t;
}

DateTime SAL_CALL OResultSet::getTimestamp(sal_Int32 column)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkBordersAndEnsureFetched(column);

    if (checkNull(column))
        return DateTime{};

    OString sVal = m_aRows[m_nRowPosition][column - 1];

    // YY-MM-DD HH:MM:SS
    std::vector<OString> dateAndTime
        = lcl_split(OString{ sVal.getStr(), getDataLength(column) }, ' ');

    auto dateParts = lcl_split(dateAndTime.at(0), '-');
    auto timeParts = lcl_split(dateAndTime.at(1), ':');

    if (dateParts.size() < 2 || timeParts.size() < 2)
        throw SQLException("Timestamp has a wrong format", *this, OUString(), 1, Any());

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

    return m_nRowPosition < 0;
}

sal_Bool SAL_CALL OResultSet::isAfterLast()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ensureFieldInfoFetched();

    return m_nRowPosition >= m_nRowCount;
}

sal_Bool SAL_CALL OResultSet::isFirst()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ensureFieldInfoFetched();

    return m_nRowPosition == 0 && !isAfterLast();
}

sal_Bool SAL_CALL OResultSet::isLast()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ensureFieldInfoFetched();

    return m_nRowPosition == m_nRowCount - 1;
}

void SAL_CALL OResultSet::beforeFirst()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    m_nRowPosition = -1;
}

void SAL_CALL OResultSet::afterLast()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ensureFieldInfoFetched();
    m_nRowPosition = m_nRowCount;
}

void SAL_CALL OResultSet::close()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_pResult = nullptr;
    dispose();
}

sal_Bool SAL_CALL OResultSet::first()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    m_nRowPosition = 0;

    return true;
}

sal_Bool SAL_CALL OResultSet::last()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ensureFieldInfoFetched();
    m_nRowPosition = m_nRowCount - 1;

    return true;
}

sal_Bool SAL_CALL OResultSet::absolute(sal_Int32 row)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ensureFieldInfoFetched();

    sal_Int32 nToGo = row < 0 ? (m_nRowCount - 1) - row : row - 1;

    if (nToGo >= m_nRowCount)
        nToGo = m_nRowCount - 1;
    if (nToGo < 0)
        nToGo = 0;

    m_nRowPosition = nToGo;

    return true;
}

sal_Bool SAL_CALL OResultSet::relative(sal_Int32 row)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ensureFieldInfoFetched();

    if (row == 0)
        return true;

    sal_Int32 nToGo = m_nRowPosition + row;
    if (nToGo >= m_nRowCount)
        nToGo = m_nRowCount - 1;
    if (nToGo < 0)
        nToGo = 0;

    m_nRowPosition = nToGo;

    return true;
}

sal_Bool SAL_CALL OResultSet::previous()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (m_nRowPosition == 0)
    {
        m_nRowPosition--;
        return false;
    }
    else if (m_nRowPosition < 0)
    {
        return false;
    }

    m_nRowPosition--;
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
    ensureFieldInfoFetched();
    if (m_nRowPosition + 1 > m_nRowCount) // afterlast
        return false;
    if (m_nRowPosition + 1 == m_nRowCount) // last
    {
        // return false but take it to afterlast anyway
        ++m_nRowPosition;
        return false;
    }
    ++m_nRowPosition;
    return true;
}

sal_Bool SAL_CALL OResultSet::wasNull()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
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
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateNull", *this);
}

void SAL_CALL OResultSet::updateBoolean(sal_Int32 column, sal_Bool /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateBoolean", *this);
}

void SAL_CALL OResultSet::updateByte(sal_Int32 column, sal_Int8 /* x */)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateByte", *this);
}

void SAL_CALL OResultSet::updateShort(sal_Int32 column, sal_Int16 /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateShort", *this);
}

void SAL_CALL OResultSet::updateInt(sal_Int32 column, sal_Int32 /* x */)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateInt", *this);
}

void SAL_CALL OResultSet::updateLong(sal_Int32 column, sal_Int64 /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateLong", *this);
}

void SAL_CALL OResultSet::updateFloat(sal_Int32 column, float /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateFloat", *this);
}

void SAL_CALL OResultSet::updateDouble(sal_Int32 column, double /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateDouble", *this);
}

void SAL_CALL OResultSet::updateString(sal_Int32 column, const OUString& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateString", *this);
}

void SAL_CALL OResultSet::updateBytes(sal_Int32 column, const uno::Sequence<sal_Int8>& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateBytes", *this);
}

void SAL_CALL OResultSet::updateDate(sal_Int32 column, const Date& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateDate", *this);
}

void SAL_CALL OResultSet::updateTime(sal_Int32 column, const Time& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateTime", *this);
}

void SAL_CALL OResultSet::updateTimestamp(sal_Int32 column, const DateTime& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateTimestamp", *this);
}

void SAL_CALL OResultSet::updateBinaryStream(sal_Int32 column,
                                             const uno::Reference<XInputStream>& /* x */,
                                             sal_Int32 /* length */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
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
    checkRowIndex();
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
    checkRowIndex();
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateObject", *this);
}

void SAL_CALL OResultSet::updateNumericObject(sal_Int32 column, const Any& /* x */,
                                              sal_Int32 /* scale */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    checkRowIndex();
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
            throw uno::Exception("cannot set prop " + OUString::number(nHandle), nullptr);
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
    if (index < 1 || index > static_cast<int>(m_aFields.size()))
    {
        /* static object for efficiency or thread safety is a problem ? */
        throw SQLException("index out of range", *this, OUString(), 1, Any());
    }
}

void OResultSet::checkBordersAndEnsureFetched(sal_Int32 index)
{
    ensureResultFetched();
    checkColumnIndex(index);
    checkRowIndex();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
