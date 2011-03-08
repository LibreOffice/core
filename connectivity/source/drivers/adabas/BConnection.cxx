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
#include <cppuhelper/typeprovider.hxx>
#include "adabas/BConnection.hxx"
#include "adabas/BDriver.hxx"
#include "adabas/BCatalog.hxx"
#include "odbc/OFunctions.hxx"
#include "odbc/OTools.hxx"
#include "adabas/BDatabaseMetaData.hxx"
#include "adabas/BStatement.hxx"
#include "adabas/BPreparedStatement.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/dbcharset.hxx>
#include "connectivity/sqliterator.hxx"
#include <connectivity/sqlparse.hxx>

#include <string.h>

using namespace connectivity::adabas;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


//------------------------------------------------------------------------------
namespace starlang  = ::com::sun::star::lang;
// --------------------------------------------------------------------------------
OAdabasConnection::OAdabasConnection(const SQLHANDLE _pDriverHandle, connectivity::odbc::ODBCDriver*        _pDriver)
                                                 : OConnection_BASE2(_pDriverHandle,_pDriver)
{
    m_bUseOldDateFormat = sal_True;
}
//-----------------------------------------------------------------------------
SQLRETURN OAdabasConnection::Construct( const ::rtl::OUString& url,const Sequence< PropertyValue >& info) throw(SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_aConnectionHandle  = SQL_NULL_HANDLE;
    setURL(url);
    setConnectionInfo(info);

    // Connection allozieren
    N3SQLAllocHandle(SQL_HANDLE_DBC,m_pDriverHandleCopy,&m_aConnectionHandle);
    if(m_aConnectionHandle == SQL_NULL_HANDLE)
        throw SQLException();

    const PropertyValue *pBegin = info.getConstArray();
    const PropertyValue *pEnd   = pBegin + info.getLength();
    ::rtl::OUString sHostName;

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aDSN(url.copy(nLen+1)),aUID,aPWD;
    sal_Int32 nTimeout = 20;
    for(;pBegin != pEnd;++pBegin)
    {
        if ( !pBegin->Name.compareToAscii("Timeout") )
            pBegin->Value >>= nTimeout;
        else if(!pBegin->Name.compareToAscii("user"))
            pBegin->Value >>= aUID;
        else if(!pBegin->Name.compareToAscii("password"))
            pBegin->Value >>= aPWD;
        else if(!pBegin->Name.compareToAscii("HostName"))
            pBegin->Value >>= sHostName;
        else if(0 == pBegin->Name.compareToAscii("CharSet"))
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

    if ( sHostName.getLength() )
        aDSN = sHostName + rtl::OUString(static_cast<sal_Unicode>(':')) + aDSN;
    SQLRETURN nSQLRETURN = openConnectionWithAuth(aDSN,nTimeout, aUID,aPWD);

    return nSQLRETURN;
}
//-----------------------------------------------------------------------------
SQLRETURN OAdabasConnection::openConnectionWithAuth(const ::rtl::OUString& aConnectStr,sal_Int32 nTimeOut, const ::rtl::OUString& _uid,const ::rtl::OUString& _pwd)
{
    if (m_aConnectionHandle == SQL_NULL_HANDLE)
        return -1;

    SQLRETURN nSQLRETURN = 0;
    SDB_ODBC_CHAR szDSN[4096];
    SDB_ODBC_CHAR szUID[20];
    SDB_ODBC_CHAR szPWD[20];

    memset(szDSN,'\0',4096);
    memset(szUID,'\0',20);
    memset(szPWD,'\0',20);

    ::rtl::OString aConStr(::rtl::OUStringToOString(aConnectStr,getTextEncoding()));
    ::rtl::OString aUID(::rtl::OUStringToOString(_uid,getTextEncoding()));
    ::rtl::OString aPWD(::rtl::OUStringToOString(_pwd,getTextEncoding()));
    memcpy(szDSN, (SDB_ODBC_CHAR*) aConStr.getStr(), ::std::min<sal_Int32>((sal_Int32)2048,aConStr.getLength()));
    memcpy(szUID, (SDB_ODBC_CHAR*) aUID.getStr(), ::std::min<sal_Int32>((sal_Int32)20,aUID.getLength()));
    memcpy(szPWD, (SDB_ODBC_CHAR*) aPWD.getStr(), ::std::min<sal_Int32>((sal_Int32)20,aPWD.getLength()));



    N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_LOGIN_TIMEOUT,(SQLPOINTER)(sal_IntPtr)nTimeOut,SQL_IS_INTEGER);
    // Verbindung aufbauen

    nSQLRETURN = N3SQLConnect(m_aConnectionHandle,
                      szDSN,
                      (SQLSMALLINT) ::std::min<sal_Int32>((sal_Int32)2048,aConStr.getLength()),
                      szUID,
                      (SQLSMALLINT) ::std::min<sal_Int32>((sal_Int32)20,aUID.getLength()),
                      szPWD,
                      (SQLSMALLINT) ::std::min<sal_Int32>((sal_Int32)20,aPWD.getLength()));
    if (nSQLRETURN == SQL_ERROR || nSQLRETURN == SQL_NO_DATA)
        return nSQLRETURN;

    m_bClosed = sal_False;

    // autocoomit ist immer default

    N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_ON,SQL_IS_INTEGER);

    return nSQLRETURN;
}

//------------------------------------------------------------------------------
void OAdabasConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference< XTablesSupplier > xTableSupplier(m_xCatalog);
    ::comphelper::disposeComponent(xTableSupplier);

    m_xCatalog = WeakReference< XTablesSupplier >();

    OConnection_BASE2::disposing();
}
//------------------------------------------------------------------------------
Reference< XTablesSupplier > OAdabasConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!xTab.is())
    {
        xTab = new OAdabasCatalog(m_aConnectionHandle,this);
        m_xCatalog = xTab;
    }
    return xTab;
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OAdabasConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE2::rBHelper.bDisposed);


    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new OAdabasDatabaseMetaData(m_aConnectionHandle,this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}
// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OAdabasConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE2::rBHelper.bDisposed);

    Reference< XStatement > xReturn = new OAdabasStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OAdabasConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE2::rBHelper.bDisposed);

    Reference< XPreparedStatement > xReturn = new OAdabasPreparedStatement(this,sql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL OAdabasConnection::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw (::com::sun::star::uno::RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OConnection_BASE2::getSomething(rId);
}
// -----------------------------------------------------------------------------
Sequence< sal_Int8 > OAdabasConnection::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}
// -----------------------------------------------------------------------------
::connectivity::odbc::OConnection* OAdabasConnection::cloneConnection()
{
    return new OAdabasConnection(m_pDriverHandleCopy,m_pDriver);
}
// -----------------------------------------------------------------------------
::rtl::Reference<OSQLColumns> OAdabasConnection::createSelectColumns(const ::rtl::OUString& _rSql)
{
    ::rtl::Reference<OSQLColumns> aRet;
    OSQLParser aParser(getDriver()->getORB());
    ::rtl::OUString sErrorMessage;
    OSQLParseNode* pNode = aParser.parseTree(sErrorMessage,_rSql);
    if(pNode)
    {
        Reference< XTablesSupplier> xCata = createCatalog();
        OSQLParseTreeIterator aParseIter(this, xCata->getTables(),
                                        aParser, pNode);
        aParseIter.traverseAll();
        aRet = aParseIter.getSelectColumns();
    }
    return aRet;
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
