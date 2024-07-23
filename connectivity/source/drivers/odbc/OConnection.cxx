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

#include <odbc/OTools.hxx>
#include <odbc/OConnection.hxx>
#include <odbc/ODatabaseMetaData.hxx>
#include <odbc/OFunctions.hxx>
#include <odbc/ODriver.hxx>
#include <odbc/OStatement.hxx>
#include <odbc/OPreparedStatement.hxx>
#include <connectivity/dbcharset.hxx>
#include <connectivity/dbexception.hxx>

#include <sal/log.hxx>

#include <string.h>

using namespace connectivity::odbc;
using namespace connectivity;
using namespace dbtools;


using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

OConnection::OConnection(const SQLHANDLE _pDriverHandle,ODBCDriver* _pDriver)
                         :m_xDriver(_pDriver)
                         ,m_aConnectionHandle(nullptr)
                         ,m_pDriverHandleCopy(_pDriverHandle)
                         ,m_nStatementCount(0)
                         ,m_bClosed(false)
                         ,m_bUseCatalog(false)
                         ,m_bUseOldDateFormat(false)
                         ,m_bIgnoreDriverPrivileges(false)
                         ,m_bPreventGetVersionColumns(false)
                         ,m_bReadOnly(true)
{
}

OConnection::~OConnection()
{
    if(!isClosed(  ))
        close();

    if ( SQL_NULL_HANDLE == m_aConnectionHandle )
        return;

    SQLRETURN rc;

    if (!m_bClosed)
    {
        rc = functions().Disconnect(m_aConnectionHandle);
        OSL_ENSURE( rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO, "Failure from SQLDisconnect" );
    }

    rc = functions().FreeHandle(SQL_HANDLE_DBC, m_aConnectionHandle);
    OSL_ENSURE( rc == SQL_SUCCESS , "Failure from SQLFreeHandle for connection");

    m_aConnectionHandle = SQL_NULL_HANDLE;
}

const Functions& OConnection::functions() const
{
    OSL_ENSURE(m_xDriver, "OConnection::getOdbcFunction: m_xDriver is null!");
    return m_xDriver->functions();
}

SQLRETURN OConnection::OpenConnection(const OUString& aConnectStr, sal_Int32 nTimeOut, bool bSilent)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_aConnectionHandle == SQL_NULL_HANDLE)
        return -1;

    SQLRETURN nSQLRETURN = 0;

#ifndef MACOSX
    functions().SetConnectAttr(m_aConnectionHandle,SQL_ATTR_LOGIN_TIMEOUT,reinterpret_cast<SQLPOINTER>(static_cast<sal_IntPtr>(nTimeOut)),SQL_IS_UINTEGER);
#else
    (void)nTimeOut; /* WaE */
#endif

#ifdef LINUX
    bSilent = true;
#endif //LINUX
    SQLUSMALLINT nSilent =  bSilent ? SQL_DRIVER_NOPROMPT : SQL_DRIVER_COMPLETE;

    if (bUseWChar && functions().has(ODBC3SQLFunctionId::DriverConnectW))
    {
        SQLWChars sqlConnectStr(aConnectStr);
        SQLWCHAR szConnStrOut[4096] = {};
        SQLSMALLINT cchConnStrOut;
        nSQLRETURN = functions().DriverConnectW(m_aConnectionHandle,
                          nullptr,
                          sqlConnectStr.get(),
                          sqlConnectStr.cch(),
                          szConnStrOut,
                          std::size(szConnStrOut) - 1,
                          &cchConnStrOut,
                          nSilent);
    }
    else
    {
        SQLChars sqlConnectStr(aConnectStr, getTextEncoding());
        SQLCHAR szConnStrOut[4096] = {};
        SQLSMALLINT cbConnStrOut;
        nSQLRETURN = functions().DriverConnect(m_aConnectionHandle,
                          nullptr,
                          sqlConnectStr.get(),
                          sqlConnectStr.cch(),
                          szConnStrOut,
                          std::size(szConnStrOut) - 1,
                          &cbConnStrOut,
                          nSilent);
    }
#ifdef LINUX
    if (nSQLRETURN == SQL_ERROR || nSQLRETURN == SQL_NO_DATA || SQL_SUCCESS_WITH_INFO == nSQLRETURN)
#else
    if (nSQLRETURN == SQL_ERROR || nSQLRETURN == SQL_NO_DATA)
#endif
        return nSQLRETURN;

    m_bClosed = false;

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
        functions().SetConnectAttr(m_aConnectionHandle,SQL_ATTR_AUTOCOMMIT, reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_ON),SQL_IS_INTEGER);

    return nSQLRETURN;
}

SQLRETURN OConnection::Construct(const OUString& url,const Sequence< PropertyValue >& info)
{
    m_aConnectionHandle  = SQL_NULL_HANDLE;
    m_sURL  = url;
    setConnectionInfo(info);

    functions().AllocHandle(SQL_HANDLE_DBC,m_pDriverHandleCopy,&m_aConnectionHandle);
    if(m_aConnectionHandle == SQL_NULL_HANDLE)
        throw SQLException();

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+2);
    OUString aDSN(u"DSN="_ustr), aUID, aPWD, aSysDrvSettings;
    aDSN += url.subView(nLen+1);

    sal_Int32 nTimeout = 20;
    bool bSilent = true;
    const PropertyValue *pBegin = info.getConstArray();
    const PropertyValue *pEnd   = pBegin + info.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if( pBegin->Name == "Timeout")
        {
            if( ! (pBegin->Value >>= nTimeout) )
                SAL_WARN("connectivity.odbc", "Construct: unable to get property Timeout");
        }
        else if( pBegin->Name == "Silent")
        {
            if( ! (pBegin->Value >>= bSilent) )
                SAL_WARN("connectivity.odbc", "Construct: unable to get property Silent");
        }
        else if( pBegin->Name == "IgnoreDriverPrivileges")
        {
            if( ! (pBegin->Value >>= m_bIgnoreDriverPrivileges) )
                SAL_WARN("connectivity.odbc", "Construct: unable to get property IgnoreDriverPrivileges");
        }
        else if( pBegin->Name == "PreventGetVersionColumns")
        {
            if( ! (pBegin->Value >>= m_bPreventGetVersionColumns) )
                SAL_WARN("connectivity.odbc", "Construct: unable to get property PreventGetVersionColumns");
        }
        else if( pBegin->Name == "IsAutoRetrievingEnabled")
        {
            bool bAutoRetrievingEnabled = false;
            if( ! (pBegin->Value >>= bAutoRetrievingEnabled) )
                SAL_WARN("connectivity.odbc", "Construct: unable to get property IsAutoRetrievingEnabled");
            enableAutoRetrievingEnabled(bAutoRetrievingEnabled);
        }
        else if( pBegin->Name == "AutoRetrievingStatement")
        {
            OUString sGeneratedValueStatement;
            if( ! (pBegin->Value >>= sGeneratedValueStatement) )
                SAL_WARN("connectivity.odbc", "Construct: unable to get property AutoRetrievingStatement");
            setAutoRetrievingStatement(sGeneratedValueStatement);
        }
        else if( pBegin->Name == "user")
        {
            if( ! (pBegin->Value >>= aUID) )
                SAL_WARN("connectivity.odbc", "Construct: unable to get property user");
            aDSN += ";UID=" + aUID;
        }
        else if( pBegin->Name == "password")
        {
            if( ! (pBegin->Value >>= aPWD) )
                SAL_WARN("connectivity.odbc", "Construct: unable to get property password");
            aDSN += ";PWD=" + aPWD;
        }
        else if( pBegin->Name == "UseCatalog")
        {
             if( !( pBegin->Value >>= m_bUseCatalog) )
                SAL_WARN("connectivity.odbc", "Construct: unable to get property UseCatalog");
        }
        else if( pBegin->Name == "SystemDriverSettings")
        {
            if( ! (pBegin->Value >>= aSysDrvSettings) )
                SAL_WARN("connectivity.odbc", "Construct: unable to get property SystemDriverSettings");
            aDSN += ";" + aSysDrvSettings;
        }
        else if( pBegin->Name == "CharSet")
        {
            OUString sIanaName;
            if( ! (pBegin->Value >>= sIanaName) )
                SAL_WARN("connectivity.odbc", "Construct: unable to get property CharSet");

            ::dbtools::OCharsetMap aLookupIanaName;
            ::dbtools::OCharsetMap::const_iterator aLookup = aLookupIanaName.findIanaName(sIanaName);
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

IMPLEMENT_SERVICE_INFO(OConnection, u"com.sun.star.sdbc.drivers.odbc.OConnection"_ustr, u"com.sun.star.sdbc.Connection"_ustr)


Reference< XStatement > SAL_CALL OConnection::createStatement(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference< XStatement > xReturn = new OStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference< XPreparedStatement > xReturn = new OPreparedStatement(this,sql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const OUString& /*sql*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( u"XConnection::prepareCall"_ustr, *this );
}

OUString SAL_CALL OConnection::nativeSQL( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SQLINTEGER nOutLen;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::NativeSqlW))
    {
        SQLWChars nativeSQL(sql);
        SQLWCHAR pOut[2048];
        SQLRETURN ret = functions().NativeSqlW(m_aConnectionHandle,
                                              nativeSQL.get(), nativeSQL.cch(),
                                              pOut, std::size(pOut) - 1, &nOutLen);
        OTools::ThrowException(this, ret, m_aConnectionHandle, SQL_HANDLE_DBC, *this);
        return toUString(pOut, nOutLen);
    }
    else
    {
        SQLChars nativeSQL(sql, getTextEncoding());
        SQLCHAR pOut[2048];
        SQLRETURN ret = functions().NativeSql(m_aConnectionHandle,
                                              nativeSQL.get(), nativeSQL.cch(),
                                              pOut, std::size(pOut) - 1, &nOutLen);
        OTools::ThrowException(this, ret, m_aConnectionHandle, SQL_HANDLE_DBC, *this);
        return toUString(pOut, nOutLen, getTextEncoding());
    }
}

void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    const sal_IntPtr nAutocommit = autoCommit ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF;
    OTools::ThrowException(this,functions().SetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_AUTOCOMMIT,
                                   reinterpret_cast<SQLPOINTER>(nAutocommit) ,SQL_IS_INTEGER),
                                   m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}

sal_Bool SAL_CALL OConnection::getAutoCommit(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    sal_uInt32 nOption = 0;
    OTools::ThrowException(this,functions().GetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_AUTOCOMMIT, &nOption,0,nullptr),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    return nOption == SQL_AUTOCOMMIT_ON ;
}

void SAL_CALL OConnection::commit(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,functions().EndTran(SQL_HANDLE_DBC,m_aConnectionHandle,SQL_COMMIT),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}

void SAL_CALL OConnection::rollback(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,functions().EndTran(SQL_HANDLE_DBC,m_aConnectionHandle,SQL_ROLLBACK),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}

sal_Bool SAL_CALL OConnection::isClosed(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return OConnection_BASE::rBHelper.bDisposed;
}

Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  )
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

void SAL_CALL OConnection::setReadOnly( sal_Bool readOnly )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,
        functions().SetConnectAttr(m_aConnectionHandle,SQL_ATTR_ACCESS_MODE,reinterpret_cast< SQLPOINTER >( readOnly ),SQL_IS_INTEGER),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}

sal_Bool SAL_CALL OConnection::isReadOnly()
{
    // const member which will initialized only once
    return m_bReadOnly;
}

void SAL_CALL OConnection::setCatalog( const OUString& catalog )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    if (bUseWChar && functions().has(ODBC3SQLFunctionId::SetConnectAttrW))
    {
        SQLWChars sqlCatalog(catalog);
        OTools::ThrowException(this,
            functions().SetConnectAttrW(m_aConnectionHandle, SQL_ATTR_CURRENT_CATALOG, sqlCatalog.get(), SQL_NTSL),
            m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    }
    else
    {
        SQLChars sqlCatalog(catalog, getTextEncoding());
        OTools::ThrowException(this,
            functions().SetConnectAttr(m_aConnectionHandle,SQL_ATTR_CURRENT_CATALOG,sqlCatalog.get(),SQL_NTS),
            m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    }
}

OUString SAL_CALL OConnection::getCatalog(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    SQLINTEGER nValueLen;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::GetConnectAttrW))
    {
        SQLWCHAR pCat[1024];
        // SQLGetConnectAttrW gets/returns count of bytes, not characters
        OTools::ThrowException(this,
            functions().GetConnectAttrW(m_aConnectionHandle,SQL_ATTR_CURRENT_CATALOG,pCat,sizeof(pCat)-sizeof(SQLWCHAR),&nValueLen),
            m_aConnectionHandle,SQL_HANDLE_DBC,*this);
        return toUString(pCat, nValueLen / sizeof(SQLWCHAR));
    }
    else
    {
        SQLCHAR pCat[1024];
        OTools::ThrowException(this,
            functions().GetConnectAttr(m_aConnectionHandle,SQL_ATTR_CURRENT_CATALOG,pCat,sizeof(pCat)-1,&nValueLen),
            m_aConnectionHandle,SQL_HANDLE_DBC,*this);

        return toUString(pCat, nValueLen, getTextEncoding());
    }
}

void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 level )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,functions().SetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_TXN_ISOLATION,
                                   reinterpret_cast<SQLPOINTER>(static_cast<sal_IntPtr>(level)),SQL_IS_INTEGER),
                                   m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}

sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    sal_Int32 nTxn = 0;
    SQLINTEGER nValueLen;
    OTools::ThrowException(this,
        functions().GetConnectAttr(m_aConnectionHandle,SQL_ATTR_TXN_ISOLATION,&nTxn,sizeof nTxn,&nValueLen),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    return nTxn;
}

Reference< css::container::XNameAccess > SAL_CALL OConnection::getTypeMap(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    return nullptr;
}

void SAL_CALL OConnection::setTypeMap( const Reference< css::container::XNameAccess >& /*typeMap*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( u"XConnection::setTypeMap"_ustr, *this );
}

// XCloseable
void SAL_CALL OConnection::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    }
    dispose();
}

// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings(  )
{
    return Any();
}

void SAL_CALL OConnection::clearWarnings(  )
{
}

void OConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OConnection_BASE::disposing();

    for (auto const& connection : m_aConnections)
        connection.second->dispose();

    m_aConnections.clear();

    if(!m_bClosed)
        functions().Disconnect(m_aConnectionHandle);
    m_bClosed   = true;
}

SQLHANDLE OConnection::createStatementHandle()
{
    rtl::Reference<OConnection> xConnectionTemp = this;
    bool bNew = false;
    try
    {
        sal_Int32 nMaxStatements = getMetaData()->getMaxStatements();
        if(nMaxStatements && nMaxStatements <= m_nStatementCount)
        {
            rtl::Reference xConnection(new OConnection(m_pDriverHandleCopy,m_xDriver.get()));
            xConnection->Construct(m_sURL,getConnectionInfo());
            xConnectionTemp = xConnection;
            bNew = true;
        }
    }
    catch(SQLException&)
    {
    }

    SQLHANDLE aStatementHandle = SQL_NULL_HANDLE;
    functions().AllocHandle(SQL_HANDLE_STMT,xConnectionTemp->getConnection(),&aStatementHandle);
    ++m_nStatementCount;
    if(bNew)
        m_aConnections.emplace(aStatementHandle,xConnectionTemp);

    return aStatementHandle;

}

void OConnection::freeStatementHandle(SQLHANDLE& _pHandle)
{
    if( SQL_NULL_HANDLE == _pHandle )
        return;

    auto aFind = m_aConnections.find(_pHandle);

    functions().FreeStmt(_pHandle,SQL_RESET_PARAMS);
    functions().FreeStmt(_pHandle,SQL_UNBIND);
    functions().FreeStmt(_pHandle,SQL_CLOSE);
    functions().FreeHandle(SQL_HANDLE_STMT,_pHandle);

    _pHandle = SQL_NULL_HANDLE;

    if(aFind != m_aConnections.end())
    {
        aFind->second->dispose();
        m_aConnections.erase(aFind);
    }
    --m_nStatementCount;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
