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

#include <memory>
#include <stdio.h>

#include "mysqlc_connection.hxx"
#include "mysqlc_propertyids.hxx"
#include "mysqlc_resultset.hxx"
#include "mysqlc_statement.hxx"
#include "mysqlc_general.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <osl/thread.h>

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

void OCommonStatement::disposeResultSet()
{
    // free the cursor if alive
    if (m_xResultSet.is())
    {
        m_xResultSet.clear();
        m_pMysqlResult = nullptr; // it is freed by XResultSet
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
    disposeResultSet();
}

void SAL_CALL OStatement::clearBatch()
{
    // if you support batches clear it here
}

sal_Bool SAL_CALL OCommonStatement::execute(const OUString& sql)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);
    const OUString sSqlStatement = m_xConnection->transFormPreparedStatement(sql);

    OString toExec
        = OUStringToOString(sSqlStatement, m_xConnection->getConnectionSettings().encoding);

    MYSQL* pMySql = m_xConnection->getMysqlConnection();

    // NOTE: differs from MySQL C API, where mysql_real_escape_string_quote()
    // should be used.
    // toExec = mysqlc_sdbc_driver::escapeSql(toExec);
    int failure = mysql_real_query(pMySql, toExec.getStr(), toExec.getLength());

    if (failure)
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(pMySql), mysql_errno(pMySql),
                                                     *this, m_xConnection->getConnectionEncoding());
    m_nAffectedRows = mysql_affected_rows(pMySql);

    return !failure;
}

Reference<XResultSet> SAL_CALL OCommonStatement::executeQuery(const OUString& sql)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);
    const OUString sSqlStatement = sql; // TODO m_xConnection->transFormPreparedStatement( sql );
    OString toExec
        = OUStringToOString(sSqlStatement, m_xConnection->getConnectionSettings().encoding);

    MYSQL* pMySql = m_xConnection->getMysqlConnection();
    // toExec = mysqlc_sdbc_driver::escapeSql(toExec);
    int failure = mysql_real_query(pMySql, toExec.getStr(), toExec.getLength());
    if (failure)
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(pMySql), mysql_errno(pMySql),
                                                     *this, m_xConnection->getConnectionEncoding());

    m_pMysqlResult = mysql_store_result(pMySql);
    if (m_pMysqlResult == nullptr)
    {
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(pMySql), mysql_errno(pMySql),
                                                     *this, m_xConnection->getConnectionEncoding());
    }

    m_xResultSet = new OResultSet(*getOwnConnection(), this, m_pMysqlResult,
                                  m_xConnection->getConnectionEncoding());
    return m_xResultSet;
}

Reference<XConnection> SAL_CALL OCommonStatement::getConnection()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    // just return our connection here
    return m_xConnection.get();
}

sal_Int32 SAL_CALL OCommonStatement::getUpdateCount() { return m_nAffectedRows; }

Any SAL_CALL OStatement::queryInterface(const Type& rType)
{
    Any aRet = ::cppu::queryInterface(rType, static_cast<XBatchExecution*>(this));
    if (!aRet.hasValue())
    {
        aRet = OCommonStatement::queryInterface(rType);
    }
    return aRet;
}

void SAL_CALL OStatement::addBatch(const OUString&)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);
}

Sequence<sal_Int32> SAL_CALL OStatement::executeBatch()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    Sequence<sal_Int32> aRet = Sequence<sal_Int32>();
    return aRet;
}

sal_Int32 SAL_CALL OCommonStatement::executeUpdate(const OUString& sql)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    execute(sql);
    return m_nAffectedRows;
}

Reference<XResultSet> SAL_CALL OCommonStatement::getResultSet()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    return m_xResultSet;
}

sal_Bool SAL_CALL OCommonStatement::getMoreResults()
{
    return false; // TODO IMPL
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

OUString OStatement::getImplementationName() { return OUString("com.sun.star.sdbcx.OStatement"); }

css::uno::Sequence<OUString> OStatement::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> s(1);
    s[0] = "com.sun.star.sdbc.Statement";
    return s;
}

sal_Bool OStatement::supportsService(OUString const& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

void SAL_CALL OCommonStatement::acquire() throw() { OCommonStatement_IBase::acquire(); }

void SAL_CALL OCommonStatement::release() throw() { OCommonStatement_IBase::release(); }

void SAL_CALL OStatement::acquire() throw() { OCommonStatement::acquire(); }

void SAL_CALL OStatement::release() throw() { OCommonStatement::release(); }

Reference<css::beans::XPropertySetInfo> SAL_CALL OCommonStatement::getPropertySetInfo()
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
