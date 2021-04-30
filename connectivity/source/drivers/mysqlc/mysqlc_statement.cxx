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

#include <sal/config.h>

#include "mysqlc_connection.hxx"
#include "mysqlc_propertyids.hxx"
#include "mysqlc_resultset.hxx"
#include "mysqlc_statement.hxx"
#include "mysqlc_general.hxx"

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace connectivity::mysqlc;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
using ::osl::MutexGuard;

OCommonStatement::OCommonStatement(OConnection* _pConnection)
    : OCommonStatement_IBase(m_aMutex)
    , OPropertySetHelper(OCommonStatement_IBase::rBHelper)
    , m_xConnection(_pConnection)
{
}

OCommonStatement::~OCommonStatement() {}

void OCommonStatement::closeResultSet()
{
    if (m_xResultSet.is())
    {
        css::uno::Reference<css::sdbc::XCloseable> xClose(m_xResultSet, UNO_QUERY_THROW);
        xClose->close();
        m_xResultSet.clear();
    }
}

void OCommonStatement::disposing()
{
    MutexGuard aGuard(m_aMutex);

    m_xConnection.clear();
    OCommonStatement_IBase::disposing();
}

Any SAL_CALL OCommonStatement::queryInterface(const Type& rType)
{
    Any aRet = OCommonStatement_IBase::queryInterface(rType);
    if (!aRet.hasValue())
    {
        aRet = OPropertySetHelper::queryInterface(rType);
    }
    return aRet;
}

Sequence<Type> SAL_CALL OCommonStatement::getTypes()
{
    ::cppu::OTypeCollection aTypes(cppu::UnoType<XMultiPropertySet>::get(),
                                   cppu::UnoType<XFastPropertySet>::get(),
                                   cppu::UnoType<XPropertySet>::get());

    return concatSequences(aTypes.getTypes(), OCommonStatement_IBase::getTypes());
}

Sequence<Type> SAL_CALL OStatement::getTypes()
{
    return concatSequences(OStatement_BASE::getTypes(), OCommonStatement::getTypes());
}

void SAL_CALL OCommonStatement::cancel()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);
    // cancel the current sql statement
}

void SAL_CALL OCommonStatement::close()
{
    /*
      We need a block for the checkDisposed call.
      After the check we can call dispose() as we are not under lock ??
    */
    {
        MutexGuard aGuard(m_aMutex);
        checkDisposed(rBHelper.bDisposed);
    }
    dispose();
    closeResultSet();
}

// void SAL_CALL OStatement::clearBatch()
// {
//     mysqlc_sdbc_driver::throwFeatureNotImplementedException("com:sun:star:sdbc:XBatchExecution");
// }

sal_Bool SAL_CALL OStatement::execute(const OUString& sql)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    closeResultSet();
    m_nAffectedRows = -1;

    OString toExec = OUStringToOString(sql, m_xConnection->getConnectionSettings().encoding);

    MYSQL* pMySql = m_xConnection->getMysqlConnection();

    // NOTE: differs from MySQL C API, where mysql_real_escape_string_quote()
    // should be used.
    // toExec = mysqlc_sdbc_driver::escapeSql(toExec);
    int failure = mysql_real_query(pMySql, toExec.getStr(), toExec.getLength());

    if (failure || mysql_errno(pMySql))
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(pMySql), mysql_sqlstate(pMySql),
                                                     mysql_errno(pMySql), *this,
                                                     m_xConnection->getConnectionEncoding());

    return getResult();
}

Reference<XResultSet> SAL_CALL OStatement::executeQuery(const OUString& sql)
{
    bool isRS(execute(sql));
    // if a MySQL error occurred, it was already thrown and the below is not executed
    assert(isRS == m_xResultSet.is());
    if (!isRS)
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(
            "executeQuery called on SQL command that does not return a ResultSet", "02000", 0,
            *this);
    if (!m_xResultSet.is())
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(
            "internal MySQL-SDBC error: executeQuery: no ResultSet after execute() returned true.",
            "02000", 0, *this);

    return m_xResultSet;
}

Reference<XConnection> SAL_CALL OStatement::getConnection()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    // just return our connection here
    return m_xConnection;
}

sal_Int32 SAL_CALL OStatement::getUpdateCount() { return m_nAffectedRows; }

Any SAL_CALL OStatement::queryInterface(const Type& rType)
{
    Any aRet = OCommonStatement::queryInterface(rType);
    if (!aRet.hasValue())
    {
        aRet = OStatement_BASE::queryInterface(rType);
    }
    return aRet;
}

// void SAL_CALL OStatement::addBatch(const OUString&)
// {
//     MutexGuard aGuard(m_aMutex);
//     checkDisposed(rBHelper.bDisposed);

//     mysqlc_sdbc_driver::throwFeatureNotImplementedException("com:sun:star:sdbc:XBatchExecution");
// }

// Sequence<sal_Int32> SAL_CALL OStatement::executeBatch()
// {
//     MutexGuard aGuard(m_aMutex);
//     checkDisposed(rBHelper.bDisposed);

//     mysqlc_sdbc_driver::throwFeatureNotImplementedException("com:sun:star:sdbc:XBatchExecution");
// }

sal_Int32 SAL_CALL OStatement::executeUpdate(const OUString& sql)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    execute(sql);
    return m_nAffectedRows;
}

Reference<XResultSet> SAL_CALL OStatement::getResultSet()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    return m_xResultSet;
}

bool OStatement::getResult()
{
    // all callers already reset that
    assert(!m_xResultSet.is());
    assert(m_nAffectedRows == -1);

    MYSQL* pMySql = m_xConnection->getMysqlConnection();
    MYSQL_RES* pMysqlResult = mysql_store_result(pMySql);
    if (pMysqlResult != nullptr)
    {
        // MariaDB/MySQL will return the number of rows in the ResultSet from mysql_affected_rows();
        // sdbc mandates -1 when the command (query) returns a ResultSet
        assert(m_nAffectedRows == -1);
        m_xResultSet = new OResultSet(*getOwnConnection(), this, pMysqlResult,
                                      m_xConnection->getConnectionEncoding());
        return true;
    }
    else if (mysql_field_count(pMySql) == 0)
    {
        m_nAffectedRows = mysql_affected_rows(pMySql);
        return false;
    }
    else
    {
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(
            "mysql_store_result indicated success and SQL command was supposed to return a "
            "ResultSet, but did not.",
            "02000", 0, *this);
    }
    //unreachable
    assert(false);
    // keep -Werror=return-type happy
    return false;
}

sal_Bool SAL_CALL OStatement::getMoreResults()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    closeResultSet();
    m_nAffectedRows = -1;

    MYSQL* pMySql = m_xConnection->getMysqlConnection();
    int status = mysql_next_result(pMySql);

    if (status > 0 || mysql_errno(pMySql))
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(pMySql), mysql_sqlstate(pMySql),
                                                     mysql_errno(pMySql), *this,
                                                     m_xConnection->getConnectionEncoding());

    if (status == -1)
        return false;

    if (status != 0)
    {
        const OUString errMsg("mysql_next_result returned unexpected value: "
                              + OUString::number(status));
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(errMsg, "02000", 0, *this);
    }

    return getResult();
}

Any SAL_CALL OCommonStatement::getWarnings()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    return makeAny(m_aLastWarning);
}

void SAL_CALL OCommonStatement::clearWarnings()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    m_aLastWarning = SQLWarning();
}

::cppu::IPropertyArrayHelper* OCommonStatement::createArrayHelper() const
{
    // this properties are define by the service statement
    // they must in alphabetic order
    Sequence<Property> aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++]
        = Property("CursorName", PROPERTY_ID_CURSORNAME, cppu::UnoType<OUString>::get(), 0);
    pProperties[nPos++]
        = Property("EscapeProcessing", PROPERTY_ID_ESCAPEPROCESSING, cppu::UnoType<bool>::get(), 0);
    pProperties[nPos++] = Property("FetchDirection", PROPERTY_ID_FETCHDIRECTION,
                                   cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++]
        = Property("FetchSize", PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++]
        = Property("MaxFieldSize", PROPERTY_ID_MAXFIELDSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++]
        = Property("MaxRows", PROPERTY_ID_MAXROWS, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++]
        = Property("QueryTimeOut", PROPERTY_ID_QUERYTIMEOUT, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = Property("ResultSetConcurrency", PROPERTY_ID_RESULTSETCONCURRENCY,
                                   cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++]
        = Property("ResultSetType", PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++]
        = Property("UseBookmarks", PROPERTY_ID_USEBOOKMARKS, cppu::UnoType<bool>::get(), 0);

    return new ::cppu::OPropertyArrayHelper(aProps);
}

::cppu::IPropertyArrayHelper& OCommonStatement::getInfoHelper() { return *getArrayHelper(); }

sal_Bool OCommonStatement::convertFastPropertyValue(Any& /* rConvertedValue */,
                                                    Any& /* rOldValue */, sal_Int32 /* nHandle */,
                                                    const Any& /* rValue */)
{
    // here we have to try to convert
    return false;
}

void OCommonStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& /* rValue */)
{
    // set the value to whatever is necessary
    switch (nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        case PROPERTY_ID_USEBOOKMARKS:
        default:;
    }
}

void OCommonStatement::getFastPropertyValue(Any& _rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            _rValue <<= false;
            break;
        default:;
    }
}

OUString OStatement::getImplementationName() { return "com.sun.star.sdbcx.OStatement"; }

css::uno::Sequence<OUString> OStatement::getSupportedServiceNames()
{
    return { "com.sun.star.sdbc.Statement" };
}

sal_Bool OStatement::supportsService(OUString const& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

void SAL_CALL OCommonStatement::acquire() noexcept { OCommonStatement_IBase::acquire(); }

void SAL_CALL OCommonStatement::release() noexcept { OCommonStatement_IBase::release(); }

void SAL_CALL OStatement::acquire() noexcept { OCommonStatement::acquire(); }

void SAL_CALL OStatement::release() noexcept { OCommonStatement::release(); }

Reference<css::beans::XPropertySetInfo> SAL_CALL OCommonStatement::getPropertySetInfo()
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
