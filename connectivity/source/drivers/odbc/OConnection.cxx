/*************************************************************************
 *
 *  $RCSfile: OConnection.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-24 16:27:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OCONNECTION_HXX_
#include "odbc/OConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_ODATABASEMETADATA_HXX_
#include "odbc/ODatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_RESOURCE_HRC_
#include "Resource.hrc"
#endif
#ifndef _CONNECTIVITY_MODULECONTEXT_HXX_
#include "ModuleContext.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#include "odbc/OFunctions.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_ODRIVER_HXX_
#include "odbc/ODriver.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OSTATEMENT_HXX_
#include "odbc/OStatement.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OPREPAREDSTATEMENT_HXX_
#include "odbc/OPreparedStatement.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

using namespace connectivity::odbc;
using namespace connectivity::dbtools;

//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
// --------------------------------------------------------------------------------
OConnection::OConnection(const SQLHANDLE _pDriverHandle,ODBCDriver* _pDriver)
                         : OConnection_BASE(m_aMutex),
                         OSubComponent<OConnection>((::cppu::OWeakObject*)_pDriver, this),
                         m_pDriverHandleCopy(_pDriverHandle),
                         m_pDriver(_pDriver),
                         m_bClosed(sal_False),
                         m_xMetaData(NULL)
{
    ModuleContext::AddRef();
}
//-----------------------------------------------------------------------------
OConnection::~OConnection()
{
    if(!isClosed(  ))
        close();
    ModuleContext::ReleaseRef();
}
//-----------------------------------------------------------------------------
void SAL_CALL OConnection::release() throw(RuntimeException)
{
    relase_ChildImpl();
}

//-----------------------------------------------------------------------------
SQLRETURN OConnection::OpenConnection(const ::rtl::OUString& aConnectStr,sal_Int32 nTimeOut, sal_Bool bSilent)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (m_aConnectionHandle == SQL_NULL_HANDLE)
        return -1;

    sal_Bool bReadOnly; //weil Methode statisch hier noch einmal ein lokales bReadOnly
    SQLRETURN nSQLRETURN = 0;
    SDB_ODBC_CHAR szConnStrOut[4096];
    SDB_ODBC_CHAR szConnStrIn[2048];
    SWORD cbConnStrOut;
    memset(szConnStrOut,'\0',4096);
    memset(szConnStrIn,'\0',2048);
    ::rtl::OString aConStr(::rtl::OUStringToOString(aConnectStr,osl_getThreadTextEncoding()));
    memcpy(szConnStrIn, (SDB_ODBC_CHAR*) aConStr.getStr(), ::std::min<sal_Int32>((sal_Int32)2048,aConStr.getLength()));


    N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_LOGIN_TIMEOUT,(SQLPOINTER)nTimeOut,SQL_IS_INTEGER);
    // Verbindung aufbauen

#ifdef LINUX

    nSQLRETURN = N3SQLDriverConnect(m_aConnectionHandle,
                      NULL,
                      szConnStrIn,
                      (SQLSMALLINT) ::std::min((sal_Int32)2048,aConStr.getLength()),
                      szConnStrOut,
                      (SQLSMALLINT) sizeof szConnStrOut,
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

#endif //LINUX

    try
    {
        ::rtl::OUString aVal;
        OTools::GetInfo(m_aConnectionHandle,SQL_DATA_SOURCE_READ_ONLY,aVal,*this);
        bReadOnly = !aVal.compareToAscii("Y");
    }
    catch(...)
    {
        bReadOnly = sal_True;
    }

#ifndef MAC
    // autocoomit ist immer default

    if (!bReadOnly)
        N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_ON,SQL_IS_INTEGER);
#endif

    return nSQLRETURN;
}
//-----------------------------------------------------------------------------
SQLRETURN OConnection::Construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)  throw(SQLException)
{
    osl_incrementInterlockedCount( &m_refCount );
    m_aConnectionHandle  = SQL_NULL_HANDLE;

    // Connection allozieren
    N3SQLAllocHandle(SQL_HANDLE_DBC,m_pDriverHandleCopy,&m_aConnectionHandle);
    if(m_aConnectionHandle == SQL_NULL_HANDLE)
        throw SQLException();

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aDSN(url.copy(nLen+1)),aUID,aPWD;

    sal_Int32 nTimeout = 20;
    sal_Bool bSilent = sal_True;
    const PropertyValue *pBegin = info.getConstArray();
    const PropertyValue *pEnd   = pBegin + info.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!pBegin->Name.compareToAscii("Timeout"))
            pBegin->Value >>= nTimeout;
        else if(!pBegin->Name.compareToAscii("Silent"))
            pBegin->Value >>= bSilent;
        else if(!pBegin->Name.compareToAscii("user"))
        {
            pBegin->Value >>= aUID;
            aDSN = aDSN + ::rtl::OUString::createFromAscii(";UID=") + aUID;
        }
        else if(!pBegin->Name.compareToAscii("password"))
        {
            pBegin->Value >>= aPWD;
            aDSN = aDSN + ::rtl::OUString::createFromAscii(";PWD=") + aPWD;
        }
    }

    SQLRETURN nSQLRETURN = OpenConnection(aDSN,nTimeout, bSilent);
    if (nSQLRETURN == SQL_ERROR || nSQLRETURN == SQL_NO_DATA)
    {
        OTools::ThrowException(nSQLRETURN,m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    }
    else if(SQL_SUCCESS_WITH_INFO == nSQLRETURN) // this driver does not support odbc3
    {
    }
    osl_decrementInterlockedCount( &m_refCount );
    return nSQLRETURN;
}
// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.odbc.OConnection", "com.sun.star.sdbc.Connection")

// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();
    Reference< XStatement > xReturn = new OStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aTypeInfo.empty())
        buildTypeInfo();

    Reference< XPreparedStatement > xReturn = new OPreparedStatement(this,m_aTypeInfo,sql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();
    return NULL;
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::nativeSQL( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OString aSql(::rtl::OUStringToOString(sql.getStr(),osl_getThreadTextEncoding()));
    char pOut[2048];
    SQLINTEGER nOutLen;
    OTools::ThrowException(N3SQLNativeSql(m_aConnectionHandle,(SDB_ODBC_CHAR*)aSql.getStr(),aSql.getLength(),(SDB_ODBC_CHAR*)pOut,2048,&nOutLen),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    return ::rtl::OUString(pOut,nOutLen,osl_getThreadTextEncoding());
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OTools::ThrowException(N3SQLSetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_AUTOCOMMIT,
                                   (SQLPOINTER)((autoCommit) ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF) ,SQL_IS_INTEGER),
                                   m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_uInt32 nOption = 0;
    OTools::ThrowException(N3SQLGetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_AUTOCOMMIT, &nOption,0,0),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    return nOption == SQL_AUTOCOMMIT_ON ;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OTools::ThrowException(N3SQLEndTran(SQL_HANDLE_DBC,m_aConnectionHandle,SQL_COMMIT),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OTools::ThrowException(N3SQLEndTran(SQL_HANDLE_DBC,m_aConnectionHandle,SQL_ROLLBACK),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
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
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_xMetaData.is())
        m_xMetaData = new ODatabaseMetaData(m_aConnectionHandle,this);

    return m_xMetaData;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OTools::ThrowException(
        N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_ACCESS_MODE,(SQLPOINTER)readOnly,SQL_IS_INTEGER),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_DATA_SOURCE_READ_ONLY,aValue,*this);
    return aValue == ::rtl::OUString::createFromAscii("Y");
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setCatalog( const ::rtl::OUString& catalog ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ::rtl::OString aCat(::rtl::OUStringToOString(catalog.getStr(),osl_getThreadTextEncoding()));
    OTools::ThrowException(
        N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_CURRENT_CATALOG,(SDB_ODBC_CHAR*)aCat.getStr(),SQL_NTS),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32 nValueLen;
    char pCat[1024];
    OTools::ThrowException(
        N3SQLGetConnectAttr(m_aConnectionHandle,SQL_ATTR_CURRENT_CATALOG,(SDB_ODBC_CHAR*)pCat,1024,&nValueLen),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);

    return ::rtl::OUString(pCat,nValueLen,osl_getThreadTextEncoding());
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OTools::ThrowException(N3SQLSetConnectAttr(m_aConnectionHandle,
                                   SQL_ATTR_TXN_ISOLATION,
                                   (SQLPOINTER)level,SQL_IS_INTEGER),
                                   m_aConnectionHandle,SQL_HANDLE_DBC,*this);
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32 nTxn = 0;
    SQLINTEGER nValueLen;
    OTools::ThrowException(
        N3SQLGetConnectAttr(m_aConnectionHandle,SQL_ATTR_TXN_ISOLATION,&nTxn,sizeof nTxn,&nValueLen),
        m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    return nTxn;
}
// --------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return NULL;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
}
// --------------------------------------------------------------------------------
// XCloseable
void SAL_CALL OConnection::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (OConnection_BASE::rBHelper.bDisposed)
            throw DisposedException();
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
    Reference< XRow> xRow(xRs,UNO_QUERY);
    // Information for a single SQL type

    // Loop on the result set until we reach end of file

    while (xRs->next ())
    {
        OTypeInfo aInfo;
        aInfo.aTypeName         = xRow->getString   (1);
        aInfo.nType             = xRow->getShort    (2);
        aInfo.nPrecision        = xRow->getInt      (3);
        aInfo.aLiteralPrefix    = xRow->getString   (4);
        aInfo.aLiteralSuffix    = xRow->getString   (5);
        aInfo.aCreateParams     = xRow->getString   (6);
        aInfo.bNullable         = xRow->getBoolean  (7) == ColumnValue::NULLABLE;
        aInfo.bCaseSensitive    = xRow->getBoolean  (8);
        aInfo.nSearchType       = xRow->getShort    (9);
        aInfo.bUnsigned         = xRow->getBoolean  (10);
        aInfo.bCurrency         = xRow->getBoolean  (11);
        aInfo.bAutoIncrement    = xRow->getBoolean  (12);
        aInfo.aLocalTypeName    = xRow->getString   (13);
        aInfo.nMinimumScale     = xRow->getShort    (14);
        aInfo.nMaximumScale     = xRow->getShort    (15);
        aInfo.nNumPrecRadix     = xRow->getInt      (18);



        // Now that we have the type info, save it
        // in the Hashtable if we don't already have an
        // entry for this SQL type.

        m_aTypeInfo.push_back(aInfo);
    }

    // Close the result set/statement.

    Reference< XCloseable> xClose(xRs,UNO_QUERY);
    xClose->close();
}
//------------------------------------------------------------------------------
void OConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);


    //  m_aTables.disposing();
    for (OWeakRefArray::iterator i = m_aStatements.begin(); m_aStatements.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aStatements.clear();

    OTools::ThrowException(N3SQLDisconnect(m_aConnectionHandle),m_aConnectionHandle,SQL_HANDLE_DBC,*this);
    m_bClosed = sal_True;
    m_xMetaData = NULL;

    dispose_ChildImpl();
    OConnection_BASE::disposing();

//  for (OWeakRefArray::iterator j = m_aComposers.begin(); m_aComposers.end() != j; j++)
//  {
//      Reference< XComponent > xComp(j->get(), UNO_QUERY);
//      if (xComp.is())
//          xComp->dispose();
//  }
//  m_aComposers.clear();
}


