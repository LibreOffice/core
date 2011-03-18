/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
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
#include "diagnose_ex.h"
#include <connectivity/dbexception.hxx>

#include <string.h>

using namespace connectivity::odbc;
using namespace connectivity;
using namespace dbtools;

//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
// --------------------------------------------------------------------------------
OConnection::OConnection(const SQLHANDLE _pDriverHandle,ODBCDriver* _pDriver)
                         : OSubComponent<OConnection, OConnection_BASE>((::cppu::OWeakObject*)_pDriver, this)
                         ,m_pDriver(_pDriver)
                         ,m_pDriverHandleCopy(_pDriverHandle)
                         ,m_nStatementCount(0)
                         ,m_bClosed(sal_True)
                         ,m_bUseCatalog(sal_False)
                         ,m_bUseOldDateFormat(sal_False)
                         ,m_bParameterSubstitution(sal_False)
                         ,m_bIgnoreDriverPrivileges(sal_False)
                         ,m_bPreventGetVersionColumns(sal_False)
                         ,m_bReadOnly(sal_True)
{
    m_pDriver->acquire();
}
//-----------------------------------------------------------------------------
OConnection::~OConnection()
{
    if(!isClosed(  ))
        close();

    if ( SQL_NULL_HANDLE != m_aConnectionHandle )
        N3SQLFreeHandle( SQL_HANDLE_DBC, m_aConnectionHandle );
    m_aConnectionHandle = SQL_NULL_HANDLE;

    m_pDriver->release();
    m_pDriver = NULL;
}
//-----------------------------------------------------------------------------
void SAL_CALL OConnection::release() throw()
{
    relase_ChildImpl();
}
// -----------------------------------------------------------------------------
oslGenericFunction OConnection::getOdbcFunction(sal_Int32 _nIndex)  const
{
    OSL_ENSURE(m_pDriver,"OConnection::getOdbcFunction: m_pDriver is null!");
    return m_pDriver->getOdbcFunction(_nIndex);
}
//-----------------------------------------------------------------------------
SQLRETURN OConnection::OpenConnection(const ::rtl::OUString& aConnectStr,sal_Int32 nTimeOut, sal_Bool bSilent)
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
    ::rtl::OString aConStr(::rtl::OUStringToOString(aConnectStr,getTextEncoding()));
    memcpy(szConnStrIn, (SDB_ODBC_CHAR*) aConStr.getStr(), ::std::min<sal_Int32>((sal_Int32)2048,aConStr.getLength()));

#ifndef MACOSX
    N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_LOGIN_TIMEOUT,(SQLPOINTER)(sal_IntPtr)nTimeOut,SQL_IS_UINTEGER);
    // Verbindung aufbauen
#endif

#ifdef LINUX
    OSL_UNUSED( bSilent );
    nSQLRETURN = N3SQLDriverConnect(m_aConnectionHandle,
                      NULL,
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
                      NULL,
                      szConnStrIn,
                      (SQLSMALLINT) ::std::min<sal_Int32>((sal_Int32)2048,aConStr.getLength()),
                      szConnStrOut,
                      (SQLSMALLINT) sizeof szConnStrOut,
                      &cbConnStrOut,
                      nSilent);
    if (nSQLRETURN == SQL_ERROR || nSQLRETURN == SQL_NO_DATA)
        return nSQLRETURN;

    m_bClosed = sal_False;

#endif //LINUX

    try
    {
        ::rtl::OUString aVal;
        OTools::GetInfo(this,m_aConnectionHandle,SQL_DATA_SOURCE_READ_ONLY,aVal,*this,getTextEncoding());
        m_bReadOnly = !aVal.compareToAscii("Y");
    }
    catch(Exception&)
    {
        m_bReadOnly = sal_True;
    }
    try
    {
        ::rtl::OUString sVersion;
        OTools::GetInfo(this,m_aConnectionHandle,SQL_DRIVER_ODBC_VER,sVersion,*this,getTextEncoding());
        m_bUseOldDateFormat =  sVersion == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("02.50")) || sVersion == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("02.00"));
    }
    catch(Exception&)
    {
    }


    // autocoomit ist immer default

    if (!m_bReadOnly)
        N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_ON,SQL_IS_INTEGER);

    return nSQLRETURN;
}
//-----------------------------------------------------------------------------
SQLRETURN OConnection::Construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)  throw(SQLException)
{
    m_aConnectionHandle  = SQL_NULL_HANDLE;
    m_sURL  = url;
    setConnectionInfo(info);

    // Connection allozieren
    N3SQLAllocHandle(SQL_HANDLE_DBC,m_pDriverHandleCopy,&m_aConnectionHandle);
    if(m_aConnectionHandle == SQL_NULL_HANDLE)
        throw SQLException();

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aDSN(RTL_CONSTASCII_USTRINGPARAM("DSN=")), aUID, aPWD, aSysDrvSettings;
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
    sal_Bool bSilent = sal_True;
    const PropertyValue *pBegin = info.getConstArray();
    const PropertyValue *pEnd   = pBegin + info.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!pBegin->Name.compareToAscii(pTimeout))
            OSL_VERIFY( pBegin->Value >>= nTimeout );
        else if(!pBegin->Name.compareToAscii(pSilent))
            OSL_VERIFY( pBegin->Value >>= bSilent );
        else if(!pBegin->Name.compareToAscii(pPrivName))
            OSL_VERIFY( pBegin->Value >>= m_bIgnoreDriverPrivileges );
        else if(!pBegin->Name.compareToAscii(pVerColName))
            OSL_VERIFY( pBegin->Value >>= m_bPreventGetVersionColumns );
        else if(!pBegin->Name.compareToAscii(pParaName))
            OSL_VERIFY( pBegin->Value >>= m_bParameterSubstitution );
        else if(!pBegin->Name.compareToAscii(pRetrieving))
        {
            sal_Bool bAutoRetrievingEnabled = sal_False;
            OSL_VERIFY( pBegin->Value >>= bAutoRetrievingEnabled );
            enableAutoRetrievingEnabled(bAutoRetrievingEnabled);
        }
        else if(!pBegin->Name.compareToAscii(pRetriStmt))
        {
            ::rtl::OUString sGeneratedValueStatement;
            OSL_VERIFY( pBegin->Value >>= sGeneratedValueStatement );
            setAutoRetrievingStatement(sGeneratedValueStatement);
        }
        else if(!pBegin->Name.compareToAscii(pUser))
        {
            OSL_VERIFY( pBegin->Value >>= aUID );
            aDSN = aDSN + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";UID=")) + aUID;
        }
        else if(!pBegin->Name.compareToAscii(pPwd))
        {
            OSL_VERIFY( pBegin->Value >>= aPWD );
            aDSN = aDSN + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";PWD=")) + aPWD;
        }
        else if(!pBegin->Name.compareToAscii(pUseCatalog))
        {
             OSL_VERIFY( pBegin->Value >>= m_bUseCatalog );
        }
        else if(!pBegin->Name.compareToAscii(pSysDrv))
        {
            OSL_VERIFY( pBegin->Value >>= aSysDrvSettings );
            aDSN += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";"));
            aDSN += aSysDrvSettings;
        }
        else if(0 == pBegin->Name.compareToAscii(pCharSet))
        {
            ::rtl::OUString sIanaName;
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
        OTools::ThrowException(this,nSQLRETURN,m_aConnectionHandle,SQL_HANDLE_DBC,*this,sal_False);
    }
    return nSQLRETURN;
}
// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.odbc.OConnection", "com.sun.star.sdbc.Connection")

// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference< XStatement > xReturn = new OStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference< XPreparedStatement > xReturn = new OPreparedStatement(this,sql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const ::rtl::OUString& /*sql*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::prepareCall", *this );
    return NULL;
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::nativeSQL( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OString aSql(::rtl::OUStringToOString(sql.getStr(),getTextEncoding()));
    char pOut[2048];
    SQLINTEGER nOutLen;
    OTools::ThrowException(this,N3SQLNativeSql(m_aConnectionHandle,(SDB_ODBC_CHAR*)aSql.getStr(),aSql.getLength(),(SDB_ODBC_CHAR*)pOut,sizeof pOut - 1,&nOutLen),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    return ::rtl::OUString(pOut,nOutLen,getTextEncoding());
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,N3SQLSetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_AUTOCOMMIT,
                                   (SQLPOINTER)((autoCommit) ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF) ,SQL_IS_INTEGER),
                                   m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    sal_uInt32 nOption = 0;
    OTools::ThrowException(this,N3SQLGetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_AUTOCOMMIT, &nOption,0,0),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    return nOption == SQL_AUTOCOMMIT_ON ;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,N3SQLEndTran(SQL_HANDLE_DBC,m_aConnectionHandle,SQL_COMMIT),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,N3SQLEndTran(SQL_HANDLE_DBC,m_aConnectionHandle,SQL_ROLLBACK),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return OConnection_BASE::rBHelper.bDisposed;
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  ) throw(SQLException, RuntimeException)
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
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,
        N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_ACCESS_MODE,reinterpret_cast< SQLPOINTER >( readOnly ),SQL_IS_INTEGER),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isReadOnly() throw(SQLException, RuntimeException)
{
    // const member which will initialized only once
    return m_bReadOnly;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setCatalog( const ::rtl::OUString& catalog ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    ::rtl::OString aCat(::rtl::OUStringToOString(catalog.getStr(),getTextEncoding()));
    OTools::ThrowException(this,
        N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_CURRENT_CATALOG,(SDB_ODBC_CHAR*)aCat.getStr(),SQL_NTS),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    sal_Int32 nValueLen;
    char pCat[1024];
    OTools::ThrowException(this,
        N3SQLGetConnectAttr(m_aConnectionHandle,SQL_ATTR_CURRENT_CATALOG,(SDB_ODBC_CHAR*)pCat,(sizeof pCat)-1,&nValueLen),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);

    return ::rtl::OUString(pCat,nValueLen,getTextEncoding());
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OTools::ThrowException(this,N3SQLSetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_TXN_ISOLATION,
                                   (SQLPOINTER)(sal_IntPtr)level,SQL_IS_INTEGER),
                                   m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
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
// --------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    return NULL;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setTypeMap", *this );
}
// --------------------------------------------------------------------------------
// XCloseable
void SAL_CALL OConnection::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// --------------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
//--------------------------------------------------------------------
void OConnection::buildTypeInfo() throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XResultSet> xRs = getMetaData ()->getTypeInfo ();
    if(xRs.is())
    {
        Reference< XRow> xRow(xRs,UNO_QUERY);
        // Information for a single SQL type

        ::connectivity::ORowSetValue aValue;
        ::std::vector<sal_Int32> aTypes;
        Reference<XResultSetMetaData> xResultSetMetaData = Reference<XResultSetMetaDataSupplier>(xRs,UNO_QUERY)->getMetaData();
        sal_Int32 nCount = xResultSetMetaData->getColumnCount();
        // Loop on the result set until we reach end of file
        while (xRs->next ())
        {
            OTypeInfo aInfo;
            sal_Int32 nPos = 1;
            if ( aTypes.empty() )
            {
                if ( nCount < 1 )
                    nCount = 18;
                aTypes.reserve(nCount+1);
                aTypes.push_back(-1);
                for (sal_Int32 j = 1; j <= nCount ; ++j)
                    aTypes.push_back(xResultSetMetaData->getColumnType(j));
            }

            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.aTypeName     = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.nType         = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.nPrecision        = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.aLiteralPrefix    = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.aLiteralSuffix    = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.aCreateParams = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.bNullable     = (sal_Int32)aValue == ColumnValue::NULLABLE;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.bCaseSensitive    = (sal_Bool)aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.nSearchType       = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.bUnsigned     = (sal_Bool)aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.bCurrency     = (sal_Bool)aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.bAutoIncrement    = (sal_Bool)aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.aLocalTypeName    = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.nMinimumScale = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],xRow);
            aInfo.nMaximumScale = aValue;
            if ( nCount >= 18 )
            {
                nPos = 18;
                aValue.fill(nPos,aTypes[nPos],xRow);
                aInfo.nNumPrecRadix = aValue;
            }

            // check if values are less than zero like it happens in a oracle jdbc driver
            if( aInfo.nPrecision < 0)
                aInfo.nPrecision = 0;
            if( aInfo.nMinimumScale < 0)
                aInfo.nMinimumScale = 0;
            if( aInfo.nMaximumScale < 0)
                aInfo.nMaximumScale = 0;
            if( aInfo.nNumPrecRadix < 0)
                aInfo.nNumPrecRadix = 10;

            // Now that we have the type info, save it
            // in the Hashtable if we don't already have an
            // entry for this SQL type.

            m_aTypeInfo.push_back(aInfo);
        }

        // Close the result set/statement.

        Reference< XCloseable> xClose(xRs,UNO_QUERY);
        if(xClose.is())
            xClose->close();
    }
}
//------------------------------------------------------------------------------
void OConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OConnection_BASE::disposing();

    for (::std::map< SQLHANDLE,OConnection*>::iterator aConIter = m_aConnections.begin();aConIter != m_aConnections.end();++aConIter )
        aConIter->second->dispose();

    ::std::map< SQLHANDLE,OConnection*>().swap(m_aConnections);

    if(!m_bClosed)
        N3SQLDisconnect(m_aConnectionHandle);
    m_bClosed   = sal_True;

    dispose_ChildImpl();
}
// -----------------------------------------------------------------------------
OConnection* OConnection::cloneConnection()
{
    return new OConnection(m_pDriverHandleCopy,m_pDriver);
}
// -----------------------------------------------------------------------------
SQLHANDLE OConnection::createStatementHandle()
{
    OConnection* pConnectionTemp = this;
    sal_Bool bNew = sal_False;
    try
    {
        sal_Int32 nMaxStatements = getMetaData()->getMaxStatements();
        if(nMaxStatements && nMaxStatements <= m_nStatementCount)
        {
            OConnection* pConnection = cloneConnection();
            pConnection->acquire();
            pConnection->Construct(m_sURL,getConnectionInfo());
            pConnectionTemp = pConnection;
            bNew = sal_True;
        }
    }
    catch(SQLException&)
    {
    }

    SQLHANDLE aStatementHandle = SQL_NULL_HANDLE;
    SQLRETURN nRetcode = N3SQLAllocHandle(SQL_HANDLE_STMT,pConnectionTemp->getConnection(),&aStatementHandle);
    OSL_UNUSED( nRetcode );
    ++m_nStatementCount;
    if(bNew)
        m_aConnections.insert(::std::map< SQLHANDLE,OConnection*>::value_type(aStatementHandle,pConnectionTemp));

    return aStatementHandle;

}
// -----------------------------------------------------------------------------
void OConnection::freeStatementHandle(SQLHANDLE& _pHandle)
{
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
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
