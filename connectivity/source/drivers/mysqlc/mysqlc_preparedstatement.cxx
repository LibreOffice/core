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

#include "mysqlc_general.hxx"
#include "mysqlc_prepared_resultset.hxx"
#include "mysqlc_preparedstatement.hxx"
#include "mysqlc_propertyids.hxx"
#include "mysqlc_resultsetmetadata.hxx"

#include <sal/log.hxx>

#include <com/sun/star/sdbc/DataType.hpp>

#include <stdio.h>

using namespace connectivity::mysqlc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
using ::osl::MutexGuard;

OUString OPreparedStatement::getImplementationName()
{
    return "com.sun.star.sdbcx.mysqlc.PreparedStatement";
}

css::uno::Sequence<OUString> OPreparedStatement::getSupportedServiceNames()
{
    return { "com.sun.star.sdbc.PreparedStatement" };
}

sal_Bool OPreparedStatement::supportsService(OUString const& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

OPreparedStatement::OPreparedStatement(OConnection* _pConnection, MYSQL_STMT* pStmt)
    : OCommonStatement(_pConnection)
    , m_pStmt(pStmt)
{
    m_paramCount = mysql_stmt_param_count(m_pStmt);
    m_binds.reserve(m_paramCount);
    m_bindMetas.reserve(m_paramCount);
    for (unsigned i = 0; i < m_paramCount; ++i)
    {
        m_binds.push_back(MYSQL_BIND{});
        m_bindMetas.push_back(BindMetaData{});
        m_binds.back().is_null = &m_bindMetas.back().is_null;
        m_binds.back().length = &m_bindMetas.back().length;
        m_binds.back().buffer = nullptr;
    }
}

OPreparedStatement::~OPreparedStatement() {}

void SAL_CALL OPreparedStatement::acquire() noexcept { OCommonStatement::acquire(); }

void SAL_CALL OPreparedStatement::release() noexcept { OCommonStatement::release(); }

Any SAL_CALL OPreparedStatement::queryInterface(const Type& rType)
{
    Any aRet = OCommonStatement::queryInterface(rType);
    if (!aRet.hasValue())
    {
        aRet = OPreparedStatement_BASE::queryInterface(rType);
    }
    return aRet;
}

Sequence<Type> SAL_CALL OPreparedStatement::getTypes()
{
    return concatSequences(OPreparedStatement_BASE::getTypes(), OCommonStatement::getTypes());
}

Reference<XResultSetMetaData> SAL_CALL OPreparedStatement::getMetaData()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    if (!m_xMetaData.is())
    {
        MYSQL_RES* pRes = mysql_stmt_result_metadata(m_pStmt);
        // TODO warning or error if no meta data.
        m_xMetaData = new OResultSetMetaData(*m_xConnection, pRes);
    }
    return m_xMetaData;
}

void SAL_CALL OPreparedStatement::close()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    if (mysql_stmt_close(m_pStmt))
    {
        SAL_WARN("connectivity.mysqlc", "failed to close mysql prepared statement");
    }
    m_pStmt = nullptr; // it's deallocated already
    clearWarnings();
    clearParameters();
    OCommonStatement::close();
}

sal_Bool SAL_CALL OPreparedStatement::execute()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    if (!m_binds.empty() && mysql_stmt_bind_param(m_pStmt, m_binds.data()))
    {
        MYSQL* pMysql = m_xConnection->getMysqlConnection();
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_stmt_error(m_pStmt),
                                                     mysql_sqlstate(pMysql), mysql_errno(pMysql),
                                                     *this, m_xConnection->getConnectionEncoding());
    }

    int nFail = mysql_stmt_execute(m_pStmt);
    if (nFail != 0)
    {
        MYSQL* pMysql = m_xConnection->getMysqlConnection();
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_stmt_error(m_pStmt),
                                                     mysql_sqlstate(pMysql), mysql_errno(pMysql),
                                                     *this, m_xConnection->getConnectionEncoding());
    }

    return !nFail;
}

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    if (!m_binds.empty() && mysql_stmt_bind_param(m_pStmt, m_binds.data()))
    {
        MYSQL* pMysql = m_xConnection->getMysqlConnection();
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_stmt_error(m_pStmt),
                                                     mysql_sqlstate(pMysql), mysql_errno(pMysql),
                                                     *this, m_xConnection->getConnectionEncoding());
    }

    int nFail = mysql_stmt_execute(m_pStmt);

    if (nFail != 0)
    {
        MYSQL* pMysql = m_xConnection->getMysqlConnection();
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_stmt_error(m_pStmt),
                                                     mysql_sqlstate(pMysql), mysql_errno(pMysql),
                                                     *this, m_xConnection->getConnectionEncoding());
    }

    sal_Int32 affectedRows = mysql_stmt_affected_rows(m_pStmt);
    return affectedRows;
}

void SAL_CALL OPreparedStatement::setString(sal_Int32 parameter, const OUString& x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    OString stringie(OUStringToOString(x, m_xConnection->getConnectionEncoding()));
    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_STRING;
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, stringie.getStr(), MYSQL_TYPE_STRING,
                                    stringie.getLength());
    m_bindMetas[nIndex].is_null = false;
    m_bindMetas[nIndex].length = stringie.getLength();
}

Reference<XConnection> SAL_CALL OPreparedStatement::getConnection()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    return m_xConnection;
}

Reference<XResultSet> SAL_CALL OPreparedStatement::executeQuery()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    if (!m_binds.empty() && mysql_stmt_bind_param(m_pStmt, m_binds.data()))
    {
        MYSQL* pMysql = m_xConnection->getMysqlConnection();
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_stmt_error(m_pStmt),
                                                     mysql_sqlstate(pMysql), mysql_errno(pMysql),
                                                     *this, m_xConnection->getConnectionEncoding());
    }

    int nFail = mysql_stmt_execute(m_pStmt);

    if (nFail != 0)
    {
        MYSQL* pMysql = m_xConnection->getMysqlConnection();
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_stmt_error(m_pStmt),
                                                     mysql_sqlstate(pMysql), mysql_errno(pMysql),
                                                     *this, m_xConnection->getConnectionEncoding());
    }

    Reference<XResultSet> xResultSet = new OPreparedResultSet(*m_xConnection, this, m_pStmt);
    return xResultSet;
}

void SAL_CALL OPreparedStatement::setBoolean(sal_Int32 parameter, sal_Bool x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_TINY;
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &x, MYSQL_TYPE_TINY);
    m_bindMetas[nIndex].is_null = false;
}

void SAL_CALL OPreparedStatement::setByte(sal_Int32 parameter, sal_Int8 x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_TINY;
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &x, MYSQL_TYPE_TINY);
    m_bindMetas[nIndex].is_null = false;
}

void SAL_CALL OPreparedStatement::setDate(sal_Int32 parameter, const Date& aData)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    MYSQL_TIME my_time = {};

    my_time.year = aData.Year;
    my_time.month = aData.Month;
    my_time.day = aData.Day;

    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_DATE;
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &my_time, MYSQL_TYPE_DATE);
    m_bindMetas[nIndex].is_null = false;
}

void SAL_CALL OPreparedStatement::setTime(sal_Int32 parameter, const Time& aVal)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    MYSQL_TIME my_time = {};

    my_time.hour = aVal.Hours;
    my_time.minute = aVal.Minutes;
    my_time.second = aVal.Seconds;

    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_TIME;
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &my_time, MYSQL_TYPE_TIME);
    m_bindMetas[nIndex].is_null = false;
}

void SAL_CALL OPreparedStatement::setTimestamp(sal_Int32 parameter, const DateTime& aVal)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    MYSQL_TIME my_time = {};

    my_time.hour = aVal.Hours;
    my_time.minute = aVal.Minutes;
    my_time.second = aVal.Seconds;
    my_time.year = aVal.Year;
    my_time.month = aVal.Month;
    my_time.day = aVal.Day;

    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_DATETIME;
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &my_time, MYSQL_TYPE_DATETIME);
    m_bindMetas[nIndex].is_null = false;
}

void SAL_CALL OPreparedStatement::setDouble(sal_Int32 parameter, double x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_DOUBLE;
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &x, MYSQL_TYPE_DOUBLE);
    m_bindMetas[nIndex].is_null = false;
}

void SAL_CALL OPreparedStatement::setFloat(sal_Int32 parameter, float x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_FLOAT;
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &x, MYSQL_TYPE_FLOAT);
    m_bindMetas[nIndex].is_null = false;
}

void SAL_CALL OPreparedStatement::setInt(sal_Int32 parameter, sal_Int32 x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_LONG;
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &x, MYSQL_TYPE_LONG);
    m_bindMetas[nIndex].is_null = false;
}

void SAL_CALL OPreparedStatement::setLong(sal_Int32 parameter, sal_Int64 aVal)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_LONGLONG;
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &aVal, MYSQL_TYPE_LONGLONG);
    m_bindMetas[nIndex].is_null = false;
}

void SAL_CALL OPreparedStatement::setNull(sal_Int32 parameter, sal_Int32 /*sqlType*/)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    const sal_Int32 nIndex = parameter - 1;
    m_bindMetas[nIndex].is_null = true;
    free(m_binds[nIndex].buffer);
    m_binds[nIndex].buffer = nullptr;
}

void SAL_CALL OPreparedStatement::setClob(sal_Int32 parameter, const Reference<XClob>& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setClob", *this);
}

void SAL_CALL OPreparedStatement::setBlob(sal_Int32 parameter, const Reference<XBlob>& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setBlob", *this);
}

void SAL_CALL OPreparedStatement::setArray(sal_Int32 parameter, const Reference<XArray>& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setArray", *this);
}

void SAL_CALL OPreparedStatement::setRef(sal_Int32 parameter, const Reference<XRef>& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setRef", *this);
}

void SAL_CALL OPreparedStatement::setObjectWithInfo(sal_Int32 parameterIndex, const Any& value,
                                                    sal_Int32 targetSqlType, sal_Int32 /* scale */)
{
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkParameterIndex(parameterIndex);

    const sal_Int32 nIndex = parameterIndex - 1;
    if (!value.hasValue())
    {
        free(m_binds[nIndex].buffer);
        m_binds[nIndex].buffer = nullptr;
        m_bindMetas[parameterIndex - 1].is_null = true;
        return;
    }

    switch (targetSqlType)
    {
        case DataType::DECIMAL:
        case DataType::NUMERIC:
        {
            double nValue(0.0);
            OUString sValue;
            if (value >>= nValue)
            {
                setDouble(parameterIndex, nValue);
                break;
            }
            else if (value >>= sValue)
            {
                OString sAscii
                    = OUStringToOString(sValue, getOwnConnection()->getConnectionEncoding());
                std::stringstream sStream{ sAscii.getStr() };
                sStream >> nValue;
                m_binds[nIndex].buffer_type = MYSQL_TYPE_DOUBLE;
                mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &nValue, MYSQL_TYPE_DOUBLE,
                                                sValue.getLength());
                m_bindMetas[nIndex].is_null = false;
                break;
            }

            [[fallthrough]];
        }

            // TODO other types

        default:
            mysqlc_sdbc_driver::throwInvalidArgumentException(
                "OPreparedStatement::setObjectWithInfo", *this);
            break;
    }
}

void SAL_CALL OPreparedStatement::setObjectNull(sal_Int32 parameter, sal_Int32 /* sqlType */,
                                                const OUString& /* typeName */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setObjectNull",
                                                            *this);
}

void SAL_CALL OPreparedStatement::setObject(sal_Int32 parameter, const Any& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setObject", *this);
}

void SAL_CALL OPreparedStatement::setShort(sal_Int32 parameter, sal_Int16 x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_SHORT;
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &x, MYSQL_TYPE_SHORT);
    m_bindMetas[nIndex].is_null = false;
}

void SAL_CALL OPreparedStatement::setBytes(sal_Int32 parameter, const Sequence<sal_Int8>& x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    const sal_Int32 nIndex = parameter - 1;
    m_binds[nIndex].buffer_type = MYSQL_TYPE_BLOB; // FIXME
    mysqlc_sdbc_driver::resetSqlVar(&m_binds[nIndex].buffer, &x, MYSQL_TYPE_BLOB);
    m_bindMetas[nIndex].is_null = false;
}

void SAL_CALL OPreparedStatement::setCharacterStream(sal_Int32 parameter,
                                                     const Reference<XInputStream>& /* x */,
                                                     sal_Int32 /* length */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException(
        "OPreparedStatement::setCharacterStream", *this);
}

void SAL_CALL OPreparedStatement::setBinaryStream(sal_Int32 parameter,
                                                  const Reference<XInputStream>& /* x */,
                                                  sal_Int32 /* length */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setBinaryStream",
                                                            *this);
}

void SAL_CALL OPreparedStatement::clearParameters()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    for (size_t i = 0; i < m_binds.size(); ++i)
    {
        m_bindMetas[i].is_null = true;
        free(m_binds[i].buffer);
        m_binds[i].buffer = nullptr;
    }
}

// void SAL_CALL OPreparedStatement::clearBatch()
// {
//     mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::clearBatch",
//                                                             *this);
// }

// void SAL_CALL OPreparedStatement::addBatch()
// {
//     mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::addBatch", *this);
// }

// Sequence<sal_Int32> SAL_CALL OPreparedStatement::executeBatch() {
//     mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::executeBatch", *this);
// }

void OPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue)
{
    switch (nHandle)
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
            /* XXX: Recursion ?? */
            OPreparedStatement::setFastPropertyValue_NoBroadcast(nHandle, rValue);
    }
}

void OPreparedStatement::checkParameterIndex(sal_Int32 column)
{
    if (column < 1 || column > static_cast<sal_Int32>(m_paramCount))
    {
        throw SQLException("Parameter index out of range", *this, OUString(), 1, Any());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
