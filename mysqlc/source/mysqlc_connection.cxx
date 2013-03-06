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

#include "mysqlc_connection.hxx"
#include "mysqlc_databasemetadata.hxx"


#include "mysqlc_driver.hxx"
#include "mysqlc_statement.hxx"
#include "mysqlc_preparedstatement.hxx"
#include "mysqlc_general.hxx"

#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/metadata.h>
#include <cppconn/exception.h>

#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <osl/module.hxx>
#include <osl/thread.h>
#include <osl/file.h>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>

using namespace connectivity::mysqlc;

#include <stdio.h>

//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using ::osl::MutexGuard;
using ::rtl::OUString;


#define MYSQLC_URI_PREFIX "sdbc:mysqlc:"


/* {{{ OConnection::OConnection() -I- */
OConnection::OConnection(MysqlCDriver& _rDriver, sql::Driver * _cppDriver)
    :OMetaConnection_BASE(m_aMutex)
    ,OSubComponent<OConnection, OConnection_BASE>((::cppu::OWeakObject*)&_rDriver, this)
    ,m_xMetaData(NULL)
    ,m_rDriver(_rDriver)
    ,cppDriver(_cppDriver)
    ,m_bClosed(sal_False)
    ,m_bUseCatalog(sal_False)
    ,m_bUseOldDateFormat(sal_False)
{
    OSL_TRACE("OConnection::OConnection");
    m_rDriver.acquire();
}
/* }}} */


/* {{{ OConnection::OConnection() -I- */
OConnection::~OConnection()
{
    OSL_TRACE("OConnection::~OConnection");
    if (!isClosed()) {
        close();
    }
    m_rDriver.release();
}
/* }}} */


/* {{{ OConnection::release() -I- */
void SAL_CALL OConnection::release()
    throw()
{
    OSL_TRACE("OConnection::release");
    relase_ChildImpl();
}
/* }}} */

#ifndef SYSTEM_MARIADB
    extern "C" { void SAL_CALL thisModule() {} }
#endif

/* {{{ OConnection::construct() -I- */
void OConnection::construct(const OUString& url, const Sequence< PropertyValue >& info)
    throw(SQLException)
{
    OSL_TRACE("OConnection::construct");
    MutexGuard aGuard(m_aMutex);

    sal_Int32 nIndex;
    sal_Bool  bEmbedded = sal_False;
    OUString token;
    OUString aHostName("localhost");
    sal_Int32 nPort = 3306;
    OUString aDbName;

    m_settings.encoding = m_rDriver.getDefaultEncoding();
    m_settings.quoteIdentifier = OUString();

    // parse url. Url has the following format:
    // external server: sdbc:mysqlc:[hostname]:[port]/[dbname]

    if (!url.compareTo(OUString(MYSQLC_URI_PREFIX), sizeof(MYSQLC_URI_PREFIX)-1)) {
        nIndex = 12;
    } else {
        bEmbedded = sal_True;
        nIndex = 20;
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OConnection::construct (embedded MySQL)", *this);
    }

    token = url.getToken(0, '/', nIndex);
    if (!token.isEmpty()) {
        sal_Int32 nIndex1 = 0;
        OUString hostandport = token.getToken(0,':', nIndex1);
        if (!hostandport.isEmpty()) {
            aHostName = hostandport;
            hostandport = token.getToken(0, ':', nIndex1);
            if (!hostandport.isEmpty() && nIndex1) {
                nPort = hostandport.toInt32();
            }
            token = url.getToken(0, '/', nIndex);
            if (!token.isEmpty() && nIndex) {
                aDbName = token;
            }
        }
    }

    // get user and password for mysql connection
    const PropertyValue *pIter  = info.getConstArray();
    const PropertyValue *pEnd   = pIter + info.getLength();
    OUString aUser, aPass, sUnixSocket, sNamedPipe;
    bool unixSocketPassed = false;
    bool namedPipePassed = false;

    m_settings.connectionURL = url;
    for (;pIter != pEnd;++pIter) {
        if (!pIter->Name.compareToAscii("user")) {
            OSL_VERIFY( pIter->Value >>= aUser );
        } else if (!pIter->Name.compareToAscii("password")) {
            OSL_VERIFY( pIter->Value >>= aPass );
        } else if (!pIter->Name.compareToAscii("LocalSocket")) {
            OSL_VERIFY( pIter->Value >>= sUnixSocket );
            unixSocketPassed = true;
        } else if (!pIter->Name.compareToAscii("NamedPipe")) {
            OSL_VERIFY( pIter->Value >>= sNamedPipe );
            namedPipePassed = true;
        } else if ( !pIter->Name.compareToAscii("PublicConnectionURL")) {
            OSL_VERIFY( pIter->Value >>= m_settings.connectionURL );
        } else if ( !pIter->Name.compareToAscii("NewURL")) {    // legacy name for "PublicConnectionURL"
            OSL_VERIFY( pIter->Value >>= m_settings.connectionURL );
        }
    }

    if (bEmbedded == sal_False) {
        try {
            sql::ConnectOptionsMap connProps;
            std::string host_str = OUStringToOString(aHostName, m_settings.encoding).getStr();
            std::string user_str = OUStringToOString(aUser, m_settings.encoding).getStr();
            std::string pass_str = OUStringToOString(aPass, m_settings.encoding).getStr();
            std::string schema_str = OUStringToOString(aDbName, m_settings.encoding).getStr();
            connProps["hostName"] = sql::ConnectPropertyVal(host_str);
            connProps["userName"] = sql::ConnectPropertyVal(user_str);
            connProps["password"] = sql::ConnectPropertyVal(pass_str);
            connProps["schema"] = sql::ConnectPropertyVal(schema_str);
            connProps["port"] = sql::ConnectPropertyVal((int)(nPort));
            if (unixSocketPassed) {
                sql::SQLString socket_str = OUStringToOString(sUnixSocket, m_settings.encoding).getStr();
                connProps["socket"] = socket_str;
            } else if (namedPipePassed) {
                sql::SQLString pipe_str = OUStringToOString(sNamedPipe, m_settings.encoding).getStr();
                connProps["socket"] = pipe_str;
            }

#ifndef SYSTEM_MARIADB
            ::rtl::OUString sMySQLClientLib( MYSQL_LIB  );

            ::rtl::OUString moduleBase;
            OSL_VERIFY( ::osl::Module::getUrlFromAddress( &thisModule, moduleBase ) );
            ::rtl::OUString sMySQLClientLibURL;
            try
            {
                sMySQLClientLibURL = ::rtl::Uri::convertRelToAbs( moduleBase, sMySQLClientLib.pData );
            }
            catch ( const ::rtl::MalformedUriException& e )
            {
                (void)e; // silence compiler
            #if OSL_DEBUG_LEVEL > 0
                ::rtl::OString sMessage( "OConnection::construct: malformed URI: " );
                sMessage += ::rtl::OUStringToOString( e.getMessage(), osl_getThreadTextEncoding() );
                OSL_FAIL( sMessage.getStr() );
            #endif
            }

            ::rtl::OUString sMySQLClientLibPath;
            osl_getSystemPathFromFileURL( sMySQLClientLibURL.pData, &sMySQLClientLibPath.pData );

            sql::SQLString mysqlLib = ::rtl::OUStringToOString( sMySQLClientLibPath, osl_getThreadTextEncoding() ).getStr();
            connProps["clientlib"] = mysqlLib;

            OSL_TRACE("clientlib=%s", mysqlLib.c_str());
#endif

            OSL_TRACE("hostName=%s", host_str.c_str());
            OSL_TRACE("port=%i", int(nPort));
            OSL_TRACE("userName=%s", user_str.c_str());
            OSL_TRACE("password=%s", pass_str.c_str());
            OSL_TRACE("schema=%s", schema_str.c_str());

            m_settings.cppConnection.reset(cppDriver->connect(connProps));
        } catch (const sql::SQLException &e) {
            mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
        }
    } else {
        // TODO: support for embedded server
    }

    m_settings.schema = aDbName;
    OSL_TRACE("%s", OUStringToOString(m_settings.schema, getConnectionEncoding()).getStr());

    // Check if the server is 4.1 or above
    if (this->getMysqlVersion() < 40100) {
        throw SQLException(
            ::rtl::OUString( "MySQL Connector/OO.org requires MySQL Server 4.1 or above"  ),
            *this,
            ::rtl::OUString(),
            0,
            Any());
    }
    std::auto_ptr<sql::Statement> stmt(m_settings.cppConnection->createStatement());
    stmt->executeUpdate("SET session sql_mode='ANSI_QUOTES'");
    stmt->executeUpdate("SET NAMES utf8");
}
/* }}} */


// XServiceInfo
IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.mysqlc.OConnection", "com.sun.star.sdbc.Connection")


/* {{{ OConnection::createStatement() -I- */
Reference< XStatement > SAL_CALL OConnection::createStatement()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::createStatement");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // create a statement
    Reference< XStatement > xReturn;
    // the statement can only be executed once
    try {
        xReturn = new OStatement(this, m_settings.cppConnection->createStatement());
        m_aStatements.push_back(WeakReferenceHelper(xReturn));
        return xReturn;
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
    return xReturn;
}
/* }}} */


/* {{{ OConnection::createStatement() -I- */
Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement(const OUString& _sSql)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::prepareStatement");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);
    const ::rtl::OUString sSqlStatement = transFormPreparedStatement( _sSql );

    Reference< XPreparedStatement > xStatement;
    try {
        // create a statement
        // the statement can only be executed more than once
        xStatement = new OPreparedStatement(this,
                    m_settings.cppConnection->prepareStatement(OUStringToOString(sSqlStatement, getConnectionEncoding()).getStr()));
        m_aStatements.push_back( WeakReferenceHelper( xStatement ) );
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
    return xStatement;
}
/* }}} */


/* {{{ OConnection::prepareCall() -U- */
Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall(const OUString& /*_sSql*/ )
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::prepareCall");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OConnection::prepareCall", *this);
    return Reference< XPreparedStatement >();
}
/* }}} */


/* {{{ OConnection::nativeSQL() -I- */
OUString SAL_CALL OConnection::nativeSQL(const OUString& _sSql)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::nativeSQL");
    MutexGuard aGuard(m_aMutex);

    const ::rtl::OUString sSqlStatement = transFormPreparedStatement( _sSql );
    ::rtl::OUString sNativeSQL;
    try {
        sNativeSQL = mysqlc_sdbc_driver::convert(m_settings.cppConnection->nativeSQL(mysqlc_sdbc_driver::convert(sSqlStatement, getConnectionEncoding())),
                                                                                getConnectionEncoding());
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
    return sNativeSQL;
}
/* }}} */


/* {{{ OConnection::setAutoCommit() -I- */
void SAL_CALL OConnection::setAutoCommit(sal_Bool autoCommit)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::setAutoCommit");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);
    try {
        m_settings.cppConnection->setAutoCommit(autoCommit == sal_True? true:false);
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OConnection::getAutoCommit() -I- */
sal_Bool SAL_CALL OConnection::getAutoCommit()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::getAutoCommit");
    // you have to distinguish which if you are in autocommit mode or not
    // at normal case true should be fine here

    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    sal_Bool autoCommit = sal_False;
    try {
        autoCommit = m_settings.cppConnection->getAutoCommit() == true ? sal_True : sal_False;
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
    return autoCommit;
}
/* }}} */


/* {{{ OConnection::commit() -I- */
void SAL_CALL OConnection::commit()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::commit");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);
    try {
        m_settings.cppConnection->commit();
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OConnection::rollback() -I- */
void SAL_CALL OConnection::rollback()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::rollback");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);
    try {
        m_settings.cppConnection->rollback();
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OConnection::isClosed() -I- */
sal_Bool SAL_CALL OConnection::isClosed()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::isClosed");
    MutexGuard aGuard(m_aMutex);

    // just simple -> we are close when we are disposed that means someone called dispose(); (XComponent)
    return (OConnection_BASE::rBHelper.bDisposed);
}
/* }}} */


/* {{{ OConnection::createStatement() -I- */
Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::getMetaData");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if (!xMetaData.is()) {
        try {
            xMetaData = new ODatabaseMetaData(*this); // need the connection because it can return it
        } catch (const sql::SQLException & e) {
            mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
        }
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}
/* }}} */


/* {{{ OConnection::createStatement() -I- */
void SAL_CALL OConnection::setReadOnly(sal_Bool readOnly)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::setReadOnly");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    m_settings.readOnly = readOnly;
}
/* }}} */


/* {{{ OConnection::createStatement() -I- */
sal_Bool SAL_CALL OConnection::isReadOnly()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::isReadOnly");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // return if your connection to readonly
    return (m_settings.readOnly);
}
/* }}} */


/* {{{ OConnection::createStatement() -I- */
void SAL_CALL OConnection::setCatalog(const OUString& catalog)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::setCatalog");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    try {
//      m_settings.cppConnection->setCatalog(OUStringToOString(catalog, m_settings.encoding).getStr());
        m_settings.cppConnection->setSchema(OUStringToOString(catalog, getConnectionEncoding()).getStr());
    } catch (sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OConnection::createStatement() -I- */
OUString SAL_CALL OConnection::getCatalog()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::getCatalog");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    OUString catalog;
    try {
        catalog = mysqlc_sdbc_driver::convert(m_settings.cppConnection->getSchema(), getConnectionEncoding());
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
    return catalog;
}
/* }}} */


/* {{{ OConnection::createStatement() -I- */
void SAL_CALL OConnection::setTransactionIsolation(sal_Int32 level)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::setTransactionIsolation");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    sql::enum_transaction_isolation cpplevel = sql::TRANSACTION_SERIALIZABLE;

    switch (level) {
        case TransactionIsolation::READ_UNCOMMITTED:
            cpplevel = sql::TRANSACTION_READ_UNCOMMITTED;
            break;
        case TransactionIsolation::READ_COMMITTED:
            cpplevel = sql::TRANSACTION_READ_COMMITTED;
            break;
        case TransactionIsolation::REPEATABLE_READ:
            cpplevel = sql::TRANSACTION_REPEATABLE_READ;
            break;
        case TransactionIsolation::SERIALIZABLE:
            cpplevel = sql::TRANSACTION_SERIALIZABLE;
            break;
        case TransactionIsolation::NONE:
            cpplevel = sql::TRANSACTION_SERIALIZABLE;
            break;
        default:;
            /* XXX: Exception ?? */
    }
    try {
        m_settings.cppConnection->setTransactionIsolation(cpplevel);
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OConnection::createStatement() -I- */
sal_Int32 SAL_CALL OConnection::getTransactionIsolation()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::getTransactionIsolation");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    try {
        switch (m_settings.cppConnection->getTransactionIsolation()) {
            case sql::TRANSACTION_SERIALIZABLE:     return TransactionIsolation::SERIALIZABLE;
            case sql::TRANSACTION_REPEATABLE_READ:  return TransactionIsolation::REPEATABLE_READ;
            case sql::TRANSACTION_READ_COMMITTED:   return TransactionIsolation::READ_COMMITTED;
            case sql::TRANSACTION_READ_UNCOMMITTED: return TransactionIsolation::READ_UNCOMMITTED;
            default:
                ;
        }
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
    return TransactionIsolation::NONE;
}
/* }}} */


/* {{{ OConnection::getTypeMap() -I- */
Reference<XNameAccess> SAL_CALL OConnection::getTypeMap()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::getTypeMap");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference<XNameAccess > t;
    {
        t = m_typeMap;
    }
    return (t);
}
/* }}} */


/* {{{ OConnection::setTypeMap() -I- */
void SAL_CALL OConnection::setTypeMap(const Reference<XNameAccess >& typeMap)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::setTypeMap");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    m_typeMap = typeMap;
}
/* }}} */


// XCloseable
/* {{{ OConnection::close() -I- */
void SAL_CALL OConnection::close()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::close");
    /*
      we need block, because the mutex is a local variable,
      which will guard the block
    */
    {
        // we just dispose us
        MutexGuard aGuard(m_aMutex);
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);
    }
    dispose();
}
/* }}} */


// XWarningsSupplier
/* {{{ OConnection::getWarnings() -I- */
Any SAL_CALL OConnection::getWarnings()
    throw(SQLException, RuntimeException)
{
    Any x = Any();
    OSL_TRACE("OConnection::getWarnings");
    // when you collected some warnings -> return it
    return x;
}
/* }}} */


/* {{{ OConnection::clearWarnings() -I- */
void SAL_CALL OConnection::clearWarnings()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::clearWarnings");
    // you should clear your collected warnings here#
}
/* }}} */


/* {{{ OConnection::buildTypeInfo() -I- */
void OConnection::buildTypeInfo()
    throw(SQLException)
{
    OSL_TRACE("OConnection::buildTypeInfo");
}
/* }}} */


/* {{{ OConnection::disposing() -I- */
void OConnection::disposing()
{
    OSL_TRACE("OConnection::disposing");
    // we noticed that we should be destroied in near future so we have to dispose our statements
    MutexGuard aGuard(m_aMutex);

    for (OWeakRefArray::iterator i = m_aStatements.begin(); i != m_aStatements.end() ; ++i) {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is()) {
            xComp->dispose();
        }
    }
    m_aStatements.clear();

    m_bClosed   = sal_True;
    m_xMetaData = WeakReference< XDatabaseMetaData >();

    dispose_ChildImpl();
    OConnection_BASE::disposing();
}
/* }}} */


/* ToDo - upcast the connection to MySQL_Connection and use ::getSessionVariable() */

/* {{{ OConnection::getMysqlVariable() -I- */
OUString OConnection::getMysqlVariable(const char *varname)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::getMysqlVariable");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    OUString ret;
    ::rtl::OUStringBuffer aStatement;
    aStatement.appendAscii( "SHOW SESSION VARIABLES LIKE '" );
    aStatement.appendAscii( varname );
    aStatement.append( sal_Unicode( '\'' ) );

    try {
        XStatement * stmt = new OStatement(this, m_settings.cppConnection->createStatement());
        Reference< XResultSet > rs = stmt->executeQuery( aStatement.makeStringAndClear() );
        if (rs.is() && rs->next()) {
            Reference< XRow > xRow(rs, UNO_QUERY);
            ret = xRow->getString(2);
        }
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }

    return ret;
}
/* }}} */


/* {{{ OConnection::getMysqlVersion() -I- */
sal_Int32 OConnection::getMysqlVersion()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OConnection::getMysqlVersion");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    sal_Int32 version(0);
    try {
        version = 10000 * m_settings.cppConnection->getMetaData()->getDatabaseMajorVersion();
        version += 100 * m_settings.cppConnection->getMetaData()->getDatabaseMinorVersion();
        version += m_settings.cppConnection->getMetaData()->getDatabasePatchVersion();
    } catch (const sql::SQLException & e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getConnectionEncoding());
    }
    return version;
}
/* }}} */


/* {{{ OConnection::sdbcColumnType() -I- */
// TODO: Not used
//sal_Int32 OConnection::sdbcColumnType(OUString typeName)
//{
//  OSL_TRACE("OConnection::sdbcColumnType");
//  int i = 0;
//  while (mysqlc_types[i].typeName) {
//      if (OUString::createFromAscii(mysqlc_types[i].typeName).equals(
//          typeName.toAsciiUpperCase()))
//      {
//          return mysqlc_types[i].dataType;
//      }
//      i++;
//  }
//  return 0;
//}
// -----------------------------------------------------------------------------
::rtl::OUString OConnection::transFormPreparedStatement(const ::rtl::OUString& _sSQL)
{
    ::rtl::OUString sSqlStatement = _sSQL;
    if ( !m_xParameterSubstitution.is() ) {
        try {
            Sequence< Any > aArgs(1);
            Reference< XConnection> xCon = this;
            aArgs[0] <<= NamedValue(::rtl::OUString("ActiveConnection"), makeAny(xCon));

            m_xParameterSubstitution.set(m_rDriver.getFactory()->createInstanceWithArguments(::rtl::OUString("org.openoffice.comp.helper.ParameterSubstitution"),aArgs),UNO_QUERY);
        } catch(const Exception&) {}
    }
    if ( m_xParameterSubstitution.is() ) {
        try {
            sSqlStatement = m_xParameterSubstitution->substituteVariables(sSqlStatement,sal_True);
        } catch(const Exception&) { }
    }
    return sSqlStatement;
}

/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
