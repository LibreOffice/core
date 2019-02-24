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

#include <memory>
#include "mysqlc_connection.hxx"
#include "mysqlc_databasemetadata.hxx"

#include "mysqlc_driver.hxx"
#include "mysqlc_statement.hxx"
#include "mysqlc_preparedstatement.hxx"
#include "mysqlc_general.hxx"

#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <osl/module.hxx>
#include <osl/thread.h>
#include <osl/file.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>

using namespace connectivity::mysqlc;

#include <stdio.h>

using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using ::osl::MutexGuard;

#define MYSQLC_URI_PREFIX "sdbc:mysqlc:"

namespace
{
void lcl_executeUpdate(MYSQL* pMySql, const OString& sql)
{
    mysql_real_query(pMySql, sql.getStr(), sql.getLength());
    // TODO handle error
}
}

OConnection::OConnection(MysqlCDriver& _rDriver)
    : OMetaConnection_BASE(m_aMutex)
    , m_mysql()
    , m_xMetaData(nullptr)
    , m_xDriver(&_rDriver)
{
}

OConnection::~OConnection()
{
    if (!isClosed())
    {
        close();
    }
}

void OConnection::construct(const OUString& url, const Sequence<PropertyValue>& info)
{
    MutexGuard aGuard(m_aMutex);

    mysql_library_init(0, nullptr, nullptr);
    mysql_init(&m_mysql);

    // use TCP as connection
    mysql_protocol_type protocol = MYSQL_PROTOCOL_TCP;
    mysql_options(&m_mysql, MYSQL_OPT_PROTOCOL, &protocol);

    sal_Int32 nIndex;
    OUString token;
    OUString aHostName("localhost");
    sal_Int32 nPort = 3306;
    OUString aDbName;

    m_settings.encoding = MysqlCDriver::getDefaultEncoding();

    // parse url. Url has the following format:
    // external server: sdbc:mysqlc:[hostname]:[port]/[dbname]

    if (url.startsWith(MYSQLC_URI_PREFIX))
    {
        nIndex = 12;
    }
    else
    {
        // sdbc:mysql:mysqlc:[hostname]:[port]/[dbname]
        nIndex = 18;
    }

    token = url.getToken(0, '/', nIndex);
    if (!token.isEmpty())
    {
        sal_Int32 nIndex1 = 0;
        OUString hostandport = token.getToken(0, ':', nIndex1);
        if (!hostandport.isEmpty())
        {
            aHostName = hostandport;
            hostandport = token.getToken(0, ':', nIndex1);
            if (!hostandport.isEmpty() && nIndex1)
            {
                nPort = hostandport.toInt32();
            }
            token = url.getToken(0, '/', nIndex);
            if (!token.isEmpty() && nIndex)
            {
                aDbName = token;
            }
        }
    }

    // get user and password for mysql connection
    const PropertyValue* pIter = info.getConstArray();
    const PropertyValue* pEnd = pIter + info.getLength();
    OUString aUser, aPass, sUnixSocket, sNamedPipe;
    bool unixSocketPassed = false;
    bool namedPipePassed = false;

    m_settings.connectionURL = url;
    for (; pIter != pEnd; ++pIter)
    {
        if (pIter->Name == "user")
        {
            OSL_VERIFY(pIter->Value >>= aUser);
        }
        else if (pIter->Name == "password")
        {
            OSL_VERIFY(pIter->Value >>= aPass);
        }
        else if (pIter->Name == "LocalSocket")
        {
            OSL_VERIFY(pIter->Value >>= sUnixSocket);
            unixSocketPassed = !sUnixSocket.isEmpty();
        }
        else if (pIter->Name == "NamedPipe")
        {
            OSL_VERIFY(pIter->Value >>= sNamedPipe);
            namedPipePassed = !sNamedPipe.isEmpty();
        }
        else if (pIter->Name == "PublicConnectionURL")
        {
            OSL_VERIFY(pIter->Value >>= m_settings.connectionURL);
        }
        else if (pIter->Name == "NewURL")
        { // legacy name for "PublicConnectionURL"
            OSL_VERIFY(pIter->Value >>= m_settings.connectionURL);
        }
    }

    OString host_str = OUStringToOString(aHostName, m_settings.encoding);
    OString user_str = OUStringToOString(aUser, m_settings.encoding);
    OString pass_str = OUStringToOString(aPass, m_settings.encoding);
    OString schema_str = OUStringToOString(aDbName, m_settings.encoding);
    OString socket_str;
    if (unixSocketPassed)
    {
        socket_str = OUStringToOString(sUnixSocket, m_settings.encoding);
    }
    else if (namedPipePassed)
    {
        socket_str = OUStringToOString(sNamedPipe, m_settings.encoding);
    }

    // flags can also be passed as last parameter
    if (!mysql_real_connect(&m_mysql, host_str.getStr(), user_str.getStr(), pass_str.getStr(),
                            schema_str.getStr(), nPort, socket_str.getStr(), 0))
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(&m_mysql), mysql_errno(&m_mysql),
                                                     *this, getConnectionEncoding());

    // Check if the server is 4.1 or above
    if (getMysqlVersion() < 40100)
    {
        throw SQLException("MariaDB LibreOffice Connector requires MySQL Server 4.1 or above",
                           *this, OUString(), 0, Any());
    }

    lcl_executeUpdate(&m_mysql, OString{ "SET session sql_mode='ANSI_QUOTES'" });
    lcl_executeUpdate(&m_mysql, OString{ "SET NAMES utf8" });
}

OUString OConnection::getImplementationName()
{
    return OUString("com.sun.star.sdbc.drivers.mysqlc.OConnection");
}

css::uno::Sequence<OUString> OConnection::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> s(1);
    s[0] = "com.sun.star.sdbc.Connection";
    return s;
}

sal_Bool OConnection::supportsService(OUString const& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Reference<XStatement> SAL_CALL OConnection::createStatement()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // create a statement
    Reference<XStatement> xReturn;
    // the statement can only be executed once
    xReturn = new OStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));

    return xReturn;
}

Reference<XPreparedStatement> SAL_CALL OConnection::prepareStatement(const OUString& _sSql)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);
    const OString sSqlStatement
        = OUStringToOString(_sSql, getConnectionEncoding()); // FIXME transform statement ?

    MYSQL_STMT* pStmt = mysql_stmt_init(&m_mysql);
    mysql_stmt_prepare(pStmt, sSqlStatement.getStr(), sSqlStatement.getLength());

    unsigned int nErrorNum = mysql_errno(&m_mysql);
    if (nErrorNum != 0)
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(&m_mysql), nErrorNum, *this,
                                                     getConnectionEncoding());

    Reference<XPreparedStatement> xStatement;
    xStatement = new OPreparedStatement(this, pStmt);
    m_aStatements.push_back(WeakReferenceHelper(xStatement));
    return xStatement;
}

Reference<XPreparedStatement> SAL_CALL OConnection::prepareCall(const OUString& /*_sSql*/)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OConnection::prepareCall", *this);
    return Reference<XPreparedStatement>();
}

OUString SAL_CALL OConnection::nativeSQL(const OUString& /*_sSql*/)
{
    MutexGuard aGuard(m_aMutex);

    // const OUString sSqlStatement = transFormPreparedStatement( _sSql );
    // TODO
    return OUString();
}

void SAL_CALL OConnection::setAutoCommit(sal_Bool autoCommit)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);
    if (!mysql_autocommit(&m_mysql, autoCommit))
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(&m_mysql), mysql_errno(&m_mysql),
                                                     *this, getConnectionEncoding());
}

sal_Bool SAL_CALL OConnection::getAutoCommit()
{
    // you have to distinguish which if you are in autocommit mode or not
    // at normal case true should be fine here

    // TODO use SELECT @@autocommit query for that
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    return false;
}

void SAL_CALL OConnection::commit()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    if (!mysql_commit(&m_mysql))
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(&m_mysql), mysql_errno(&m_mysql),
                                                     *this, getConnectionEncoding());
}

void SAL_CALL OConnection::rollback()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    if (!mysql_rollback(&m_mysql))
        mysqlc_sdbc_driver::throwSQLExceptionWithMsg(mysql_error(&m_mysql), mysql_errno(&m_mysql),
                                                     *this, getConnectionEncoding());
}

sal_Bool SAL_CALL OConnection::isClosed()
{
    MutexGuard aGuard(m_aMutex);

    // just simple -> we are close when we are disposed that means someone called dispose(); (XComponent)
    return OConnection_BASE::rBHelper.bDisposed;
}

Reference<XDatabaseMetaData> SAL_CALL OConnection::getMetaData()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference<XDatabaseMetaData> xMetaData = m_xMetaData;
    if (!xMetaData.is())
    {
        xMetaData = new ODatabaseMetaData(*this, &m_mysql);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

void SAL_CALL OConnection::setReadOnly(sal_Bool readOnly)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    m_settings.readOnly = readOnly;
}

sal_Bool SAL_CALL OConnection::isReadOnly()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // return if your connection to readonly
    return m_settings.readOnly;
}

void SAL_CALL OConnection::setCatalog(const OUString& /*catalog*/)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // TODO How?
}

OUString SAL_CALL OConnection::getCatalog()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // TODO How?
    return OUString{};
}

void SAL_CALL OConnection::setTransactionIsolation(sal_Int32 /*level*/)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // TODO
}

sal_Int32 SAL_CALL OConnection::getTransactionIsolation()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    return 0; // TODO
}

Reference<XNameAccess> SAL_CALL OConnection::getTypeMap()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference<XNameAccess> t = m_typeMap;
    return t;
}

void SAL_CALL OConnection::setTypeMap(const Reference<XNameAccess>& typeMap)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    m_typeMap = typeMap;
}

// XCloseable
void SAL_CALL OConnection::close()
{
    /*
      we need block, because the mutex is a local variable,
      which will guard the block
    */
    {
        // we just dispose us
        MutexGuard aGuard(m_aMutex);
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);
    }
    mysql_close(&m_mysql);
    mysql_library_end();
    dispose();
}

// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings()
{
    Any x = Any();
    // when you collected some warnings -> return it
    return x;
}

void SAL_CALL OConnection::clearWarnings()
{
    // you should clear your collected warnings here#
}

void OConnection::disposing()
{
    // we noticed that we should be destroyed in near future so we have to dispose our statements
    MutexGuard aGuard(m_aMutex);

    for (auto const& statement : m_aStatements)
    {
        Reference<XComponent> xComp(statement.get(), UNO_QUERY);
        if (xComp.is())
        {
            xComp->dispose();
        }
    }
    m_aStatements.clear();

    m_xMetaData = WeakReference<XDatabaseMetaData>();

    OConnection_BASE::disposing();
}

sal_Int32 OConnection::getMysqlVersion()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    unsigned long version = mysql_get_server_version(&m_mysql);
    return static_cast<sal_Int32>(version);
}

OUString OConnection::transFormPreparedStatement(const OUString& _sSQL)
{
    OUString sSqlStatement = _sSQL;
    if (!m_xParameterSubstitution.is())
    {
        try
        {
            Sequence<Any> aArgs(1);
            Reference<XConnection> xCon = this;
            aArgs[0] <<= NamedValue("ActiveConnection", makeAny(xCon));

            m_xParameterSubstitution.set(
                m_xDriver->getFactory()->createInstanceWithArguments(
                    "org.openoffice.comp.helper.ParameterSubstitution", aArgs),
                UNO_QUERY);
        }
        catch (const Exception&)
        {
        }
    }
    if (m_xParameterSubstitution.is())
    {
        try
        {
            sSqlStatement = m_xParameterSubstitution->substituteVariables(sSqlStatement, true);
        }
        catch (const Exception&)
        {
        }
    }
    return sSqlStatement;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
