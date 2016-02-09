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

#include "odbc/OTools.hxx"
#include "odbc/OConnection.hxx"
#include "odbc/ODatabaseMetaData.hxx"
#include "odbc/OFunctions.hxx"
#include "odbc/ODriver.hxx"
#include "odbc/OStatement.hxx"
#include "odbc/OPreparedStatement.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/dbcharset.hxx>
#include <connectivity/FValue.hxx>
#include <comphelper/extract.hxx>
#include <connectivity/dbexception.hxx>

#include <string.h>

using namespace connectivity::odbc;
using namespace connectivity;
using namespace dbtools;


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

OConnection::OConnection(const SQLHANDLE _pDriverHandle,ODBCDriver* _pDriver)
                         : OSubComponent<OConnection, OConnection_BASE>(static_cast<cppu::OWeakObject*>(_pDriver), this)
                         ,m_pDriver(_pDriver)
                         ,m_aConnectionHandle(nullptr)
                         ,m_pDriverHandleCopy(_pDriverHandle)
                         ,m_nStatementCount(0)
                         ,m_bClosed(true)
                         ,m_bUseCatalog(false)
                         ,m_bUseOldDateFormat(false)
                         ,m_bParameterSubstitution(false)
                         ,m_bIgnoreDriverPrivileges(false)
                         ,m_bPreventGetVersionColumns(false)
                         ,m_bReadOnly(true)
{
    m_pDriver->acquire();
}

OConnection::~OConnection()
{
    if(!isClosed(  ))
        close();

    if ( SQL_NULL_HANDLE != m_aConnectionHandle )
    {
        SQLRETURN rc;

        rc = N3SQLDisconnect( m_aConnectionHandle );
        OSL_ENSURE( rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO, "Failure from SQLDisconnect" );

        rc = N3SQLFreeHandle( SQL_HANDLE_DBC, m_aConnectionHandle );
        OSL_ENSURE( rc == SQL_SUCCESS , "Failure from SQLFreeHandle for connection");
        (void) rc;

        m_aConnectionHandle = SQL_NULL_HANDLE;
    }

    m_pDriver->release();
    m_pDriver = nullptr;
}

void SAL_CALL OConnection::release() throw()
{
    relase_ChildImpl();
}

oslGenericFunction OConnection::getOdbcFunction(ODBC3SQLFunctionId _nIndex)  const
{
    OSL_ENSURE(m_pDriver,"OConnection::getOdbcFunction: m_pDriver is null!");
    return m_pDriver->getOdbcFunction(_nIndex);
}

SQLRETURN OConnection::OpenConnection(const OUString& aConnectStr, sal_Int32 nTimeOut, bool bSilent)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_aConnectionHandle == SQL_NULL_HANDLE)
        return -1;

    SQLRETURN nSQLRETURN = 0;
    SDB_ODBC_CHAR szConnStrOut[4096];
    SDB_ODBC_CHAR szConnStrIn[2048];
    SQLSMALLINT cbConnStrOut;
    memset(szConnStrOut,'\0',4096);
    memset(szConnStrIn,'\0',2048);
    OString aConStr(OUStringToOString(aConnectStr,getTextEncoding()));
    memcpy(szConnStrIn, aConStr.getStr(), ::std::min<sal_Int32>((sal_Int32)2048,aConStr.getLength()));

#ifndef MACOSX
    N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_LOGIN_TIMEOUT,reinterpret_cast<SQLPOINTER>(nTimeOut),SQL_IS_UINTEGER);
#else
    (void)nTimeOut; /* WaE */
#endif

#ifdef LINUX
    (void) bSilent;
    nSQLRETURN = N3SQLDriverConnect(m_aConnectionHandle,
                      nullptr,
                      szConnStrIn,
                      (SQLSMALLINT) ::std::min((sal_Int32)2048,aConStr.getLength()),
                      szConnStrOut,
                      (SQLSMALLINT) (sizeof(szConnStrOut)/sizeof(SDB_ODBC_CHAR)) -1,
                      &cbConnStrOut,
                      SQL_DRIVER_NOPROMPT);
    if (nSQLRETURN == SQL_ERROR || nSQLRETURN == SQL_NO_DATA || SQL_SUCCESS_WITH_INFO == nSQLRETURN)
        return nSQLRETURN;
#else

    SQLUSMALLINT nSilent =  bSilent ? SQL_DRIVER_NOPROMPT : SQL_DRIVER_COMPLETE;
    nSQLRETURN = N3SQLDriverConnect(m_aConnectionHandle,
                      nullptr,
                      szConnStrIn,
                      (SQLSMALLINT) ::std::min<sal_Int32>((sal_Int32)2048,aConStr.getLength()),
                      szConnStrOut,
                      (SQLSMALLINT) sizeof szConnStrOut,
                      &cbConnStrOut,
                      nSilent);
    if (nSQLRETURN == SQL_ERROR || nSQLRETURN == SQL_NO_DATA)
        return nSQLRETURN;

    m_bClosed = false;

#endif //LINUX

    try
    {
        OUString aVal;
        OTools::GetInfo(this,m_aConnectionHandle,SQL_DATA_SOURCE_READ_ONLY,aVal,*this,getTextEncoding());
        m_bReadOnly = aVal == "Y";
    }
    catch(Exception&)
    {
        m_bReadOnly = true;
    }
    try
    {
        OUString sVersion;
        OTools::GetInfo(this,m_aConnectionHandle,SQL_DRIVER_ODBC_VER,sVersion,*this,getTextEncoding());
        m_bUseOldDateFormat =  sVersion == "02.50" || sVersion == "02.00";
    }
    catch(Exception&)
    {
    }


    // autocommit is always default

    if (!m_bReadOnly)
        N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_AUTOCOMMIT, reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_ON),SQL_IS_INTEGER);

    return nSQLRETURN;
}

SQLRETURN OConnection::Construct(const OUString& url,const Sequence< PropertyValue >& info)  throw(SQLException, std::exception)
{
    m_aConnectionHandle  = SQL_NULL_HANDLE;
    m_sURL  = url;
    setConnectionInfo(info);

    N3SQLAllocHandle(SQL_HANDLE_DBC,m_pDriverHandleCopy,&m_aConnectionHandle);
    if(m_aConnectionHandle == SQL_NULL_HANDLE)
        throw SQLException();

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+2);
    OUString aDSN("DSN="), aUID, aPWD, aSysDrvSettings;
    aDSN += url.copy(nLen+1);

    const char* pUser       = "user";
    const char* pTimeout    = "Timeout";
    const char* pSilent     = "Silent";
    const char* pPwd        = "password";
    const char* pUseCatalog = "UseCatalog";
    const char* pSysDrv     = "SystemDriverSettings";
    const char* pCharSet    = "CharSet";
    const char* pParaName   = "ParameterNameSubstitution";
    const char* pPrivName   = "IgnoreDriverPrivileges";
    const char* pVerColName = "PreventGetVersionColumns";   // #i60273#
    const char* pRetrieving = "IsAutoRetrievingEnabled";
    const char* pRetriStmt  = "AutoRetrievingStatement";

    sal_Int32 nTimeout = 20;
    bool bSilent = true;
    const PropertyValue *pBegin = info.getConstArray();
    const PropertyValue *pEnd   = pBegin + info.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if( pBegin->Name.equalsAscii(pTimeout))
            OSL_VERIFY( pBegin->Value >>= nTimeout );
        else if( pBegin->Name.equalsAscii(pSilent))
            OSL_VERIFY( pBegin->Value >>= bSilent );
        else if( pBegin->Name.equalsAscii(pPrivName))
            OSL_VERIFY( pBegin->Value >>= m_bIgnoreDriverPrivileges );
        else if( pBegin->Name.equalsAscii(pVerColName))
            OSL_VERIFY( pBegin->Value >>= m_bPreventGetVersionColumns );
        else if( pBegin->Name.equalsAscii(pParaName))
            OSL_VERIFY( pBegin->Value >>= m_bParameterSubstitution );
        else if( pBegin->Name.equalsAscii(pRetrieving))
        {
            bool bAutoRetrievingEnabled = false;
            OSL_VERIFY( pBegin->Value >>= bAutoRetrievingEnabled );
            enableAutoRetrievingEnabled(bAutoRetrievingEnabled);
        }
        else if( pBegin->Name.equalsAscii(pRetriStmt))
        {
            OUString sGeneratedValueStatement;
            OSL_VERIFY( pBegin->Value >>= sGeneratedValueStatement );
            setAutoRetrievingStatement(sGeneratedValueStatement);
        }
        else if( pBegin->Name.equalsAscii(pUser))
        {
            OSL_VERIFY( pBegin->Value >>= aUID );
            aDSN = aDSN + ";UID=" + aUID;
        }
        else if( pBegin->Name.equalsAscii(pPwd))
        {
            OSL_VERIFY( pBegin->Value >>= aPWD );
            aDSN = aDSN + ";PWD=" + aPWD;
        }
        else if( pBegin->Name.equalsAscii(pUseCatalog))
        {
             OSL_VERIFY( pBegin->Value >>= m_bUseCatalog );
        }
        else if( pBegin->Name.equalsAscii(pSysDrv))
        {
            OSL_VERIFY( pBegin->Value >>= aSysDrvSettings );
            aDSN += ";";
            aDSN += aSysDrvSettings;
        }
        else if( pBegin->Name.equalsAscii(pCharSet))
        {
            OUString sIanaName;
            OSL_VERIFY( pBegin->Value >>= sIanaName );

            ::dbtools::OCharsetMap aLookupIanaName;
            ::dbtools::OCharsetMap::const_iterator aLookup = aLookupIanaName.find(sIanaName, ::dbtools::OCharsetMap::IANA());
            if (aLookup != aLookupIanaName.end())
                m_nTextEncoding = (*aLookup).getEncoding();
            else
                m_nTextEncoding = RTL_TEXTENCODING_DONTKNOW;
            if(m_nTextEncoding == RTL_TEXTENCODING_DONTKNOW)
                m_nTextEncoding = osl_getThreadTextEncoding();
        }
    }
    m_sUser = aUID;

    SQLRETURN nSQLRETURN = OpenConnection(aDSN,nTimeout, bSilent);
    if (nSQLRETURN == SQL_ERROR || nSQLRETURN == SQL_NO_DATA)
    {
        OTools::ThrowException(this,nSQLRETURN,m_aConnectionHandle,SQL_HANDLE_DBC,*this,false);
    }
    return nSQLRETURN;
}
// XServiceInfo

IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.odbc.OConnection", "com.sun.star.sdbc.Connection")


Reference< XStatement > SAL_CALL OConnection::createStatement(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference< XStatement > xReturn = new OStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const OUString& sql ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference< XPreparedStatement > xReturn = new OPreparedStatement(this,sql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const OUString& /*sql*/ ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::prepareCall", *this );
    return nullptr;
}

OUString SAL_CALL OConnection::nativeSQL( const OUString& sql ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OString aSql(OUStringToOString(sql.getStr(),getTextEncoding()));
    char pOut[2048];
    SQLINTEGER nOutLen;
    OTools::ThrowException(this,N3SQLNativeSql(m_aConnectionHandle,reinterpret_cast<SDB_ODBC_CHAR *>(const_cast<char *>(aSql.getStr())),aSql.getLength(),reinterpret_cast<SDB_ODBC_CHAR*>(pOut),sizeof pOut - 1,&nOutLen),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    return OUString(pOut,nOutLen,getTextEncoding());
}

void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,N3SQLSetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_AUTOCOMMIT,
                                   reinterpret_cast<SQLPOINTER>((autoCommit) ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF) ,SQL_IS_INTEGER),
                                   m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}

sal_Bool SAL_CALL OConnection::getAutoCommit(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    sal_uInt32 nOption = 0;
    OTools::ThrowException(this,N3SQLGetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_AUTOCOMMIT, &nOption,0,nullptr),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    return nOption == SQL_AUTOCOMMIT_ON ;
}

void SAL_CALL OConnection::commit(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,N3SQLEndTran(SQL_HANDLE_DBC,m_aConnectionHandle,SQL_COMMIT),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}

void SAL_CALL OConnection::rollback(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,N3SQLEndTran(SQL_HANDLE_DBC,m_aConnectionHandle,SQL_ROLLBACK),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}

sal_Bool SAL_CALL OConnection::isClosed(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return OConnection_BASE::rBHelper.bDisposed;
}

Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new ODatabaseMetaData(m_aConnectionHandle,this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

void SAL_CALL OConnection::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,
        N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_ACCESS_MODE,reinterpret_cast< SQLPOINTER >( readOnly ),SQL_IS_INTEGER),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}

sal_Bool SAL_CALL OConnection::isReadOnly() throw(SQLException, RuntimeException, std::exception)
{
    // const member which will initialized only once
    return m_bReadOnly;
}

void SAL_CALL OConnection::setCatalog( const OUString& catalog ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OString aCat(OUStringToOString(catalog.getStr(),getTextEncoding()));
    OTools::ThrowException(this,
        N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_CURRENT_CATALOG,const_cast<char *>(aCat.getStr()),SQL_NTS),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}

OUString SAL_CALL OConnection::getCatalog(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    SQLINTEGER nValueLen;
    char pCat[1024];
    OTools::ThrowException(this,
        N3SQLGetConnectAttr(m_aConnectionHandle,SQL_ATTR_CURRENT_CATALOG,pCat,(sizeof pCat)-1,&nValueLen),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);

    return OUString(pCat,nValueLen,getTextEncoding());
}

void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,N3SQLSetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_TXN_ISOLATION,
                                   reinterpret_cast<SQLPOINTER>(level),SQL_IS_INTEGER),
                                   m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}

sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    sal_Int32 nTxn = 0;
    SQLINTEGER nValueLen;
    OTools::ThrowException(this,
        N3SQLGetConnectAttr(m_aConnectionHandle,SQL_ATTR_TXN_ISOLATION,&nTxn,sizeof nTxn,&nValueLen),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    return nTxn;
}

Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OConnection::getTypeMap(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    return nullptr;
}

void SAL_CALL OConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setTypeMap", *this );
}

// XCloseable
void SAL_CALL OConnection::close(  ) throw(SQLException, RuntimeException, std::exception)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    }
    dispose();
}

// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
    return Any();
}

void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
}

void OConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OConnection_BASE::disposing();

    for (::std::map< SQLHANDLE,OConnection*>::iterator aConIter = m_aConnections.begin();aConIter != m_aConnections.end();++aConIter )
        aConIter->second->dispose();

    ::std::map< SQLHANDLE,OConnection*>().swap(m_aConnections);

    if(!m_bClosed)
        N3SQLDisconnect(m_aConnectionHandle);
    m_bClosed   = true;

    dispose_ChildImpl();
}

OConnection* OConnection::cloneConnection()
{
    return new OConnection(m_pDriverHandleCopy,m_pDriver);
}

SQLHANDLE OConnection::createStatementHandle()
{
    OConnection* pConnectionTemp = this;
    bool bNew = false;
    try
    {
        sal_Int32 nMaxStatements = getMetaData()->getMaxStatements();
        if(nMaxStatements && nMaxStatements <= m_nStatementCount)
        {
            OConnection* pConnection = cloneConnection();
            pConnection->acquire();
            pConnection->Construct(m_sURL,getConnectionInfo());
            pConnectionTemp = pConnection;
            bNew = true;
        }
    }
    catch(SQLException&)
    {
    }

    SQLHANDLE aStatementHandle = SQL_NULL_HANDLE;
    N3SQLAllocHandle(SQL_HANDLE_STMT,pConnectionTemp->getConnection(),&aStatementHandle);
    ++m_nStatementCount;
    if(bNew)
        m_aConnections.insert(::std::map< SQLHANDLE,OConnection*>::value_type(aStatementHandle,pConnectionTemp));

    return aStatementHandle;

}

void OConnection::freeStatementHandle(SQLHANDLE& _pHandle)
{
    if( SQL_NULL_HANDLE == _pHandle )
        return;

    ::std::map< SQLHANDLE,OConnection*>::iterator aFind = m_aConnections.find(_pHandle);

    N3SQLFreeStmt(_pHandle,SQL_RESET_PARAMS);
    N3SQLFreeStmt(_pHandle,SQL_UNBIND);
    N3SQLFreeStmt(_pHandle,SQL_CLOSE);
    N3SQLFreeHandle(SQL_HANDLE_STMT,_pHandle);

    _pHandle = SQL_NULL_HANDLE;

    if(aFind != m_aConnections.end())
    {
        aFind->second->dispose();
        m_aConnections.erase(aFind);
    }
    --m_nStatementCount;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
