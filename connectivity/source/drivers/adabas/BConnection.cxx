/*************************************************************************
 *
 *  $RCSfile: BConnection.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:08:03 $
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
#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#include "adabas/BConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_BDRIVER_HXX_
#include "adabas/BDriver.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#include "odbc/OFunctions.hxx"
#endif
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_ODATABASEMETADATA_HXX_
#include "adabas/BDatabaseMetaData.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

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
}
//-----------------------------------------------------------------------------
SQLRETURN OAdabasConnection::Construct( const ::rtl::OUString& url,const Sequence< PropertyValue >& info) throw(SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    osl_incrementInterlockedCount( &m_refCount );

    m_aConnectionHandle  = SQL_NULL_HANDLE;

    // Connection allozieren
    N3SQLAllocHandle(SQL_HANDLE_DBC,m_pDriverHandleCopy,&m_aConnectionHandle);
    if(m_aConnectionHandle == SQL_NULL_HANDLE)
        throw SQLException();

    const PropertyValue *pBegin = info.getConstArray();
    const PropertyValue *pEnd   = pBegin + info.getLength();
    ::rtl::OUString aAdminUid,aAdminPwd;

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aDSN(url.copy(nLen+1)),aUID,aPWD;
    sal_Int32 nTimeout = 20;
    for(;pBegin != pEnd;++pBegin)
    {
        if(!pBegin->Name.compareToAscii("CTRLUSER"))
            pBegin->Value >>= aAdminUid;
        else if(!pBegin->Name.compareToAscii("CTRLPWD"))
            pBegin->Value >>= aAdminPwd;
        else if(!pBegin->Name.compareToAscii("Timeout"))
            pBegin->Value >>= nTimeout;
        else if(!pBegin->Name.compareToAscii("user"))
            pBegin->Value >>= aUID;
        else if(!pBegin->Name.compareToAscii("password"))
            pBegin->Value >>= aPWD;
    }

    SQLRETURN nSQLRETURN = OpenConnection(aDSN,nTimeout, aUID,aPWD);

    osl_decrementInterlockedCount( &m_refCount );
    return nSQLRETURN;
}
//-----------------------------------------------------------------------------
SQLRETURN OAdabasConnection::OpenConnection(const ::rtl::OUString& aConnectStr,sal_Int32 nTimeOut, const ::rtl::OUString& _uid,const ::rtl::OUString& _pwd)
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

    ::rtl::OString aConStr(::rtl::OUStringToOString(aConnectStr,osl_getThreadTextEncoding()));
    ::rtl::OString aUID(::rtl::OUStringToOString(_uid,osl_getThreadTextEncoding()));
    ::rtl::OString aPWD(::rtl::OUStringToOString(_pwd,osl_getThreadTextEncoding()));
    memcpy(szDSN, (SDB_ODBC_CHAR*) aConStr.getStr(), ::std::min<sal_Int32>((sal_Int32)2048,aConStr.getLength()));
    memcpy(szUID, (SDB_ODBC_CHAR*) aUID.getStr(), ::std::min<sal_Int32>((sal_Int32)20,aUID.getLength()));
    memcpy(szPWD, (SDB_ODBC_CHAR*) aPWD.getStr(), ::std::min<sal_Int32>((sal_Int32)20,aPWD.getLength()));



    N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_LOGIN_TIMEOUT,(SQLPOINTER)nTimeOut,SQL_IS_INTEGER);
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

#ifndef MAC
    // autocoomit ist immer default

    N3SQLSetConnectAttr(m_aConnectionHandle,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_ON,SQL_IS_INTEGER);
#endif
    buildTypeInfo();

    return nSQLRETURN;
}

//------------------------------------------------------------------------------
void OAdabasConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference< XComponent > xComp2(m_xCatalog.get(), UNO_QUERY);
    if(xComp2.is())
        xComp2->dispose();

    m_xCatalog = Reference< XTablesSupplier >();

    OConnection_BASE2::disposing();
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< XTablesSupplier > OAdabasConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!m_xCatalog.get().is())
    {
        OAdabasCatalog *pCat = new OAdabasCatalog(m_aConnectionHandle,this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OAdabasConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE2::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new OAdabasDatabaseMetaData(m_aConnectionHandle,this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}
//------------------------------------------------------------------------------
sal_Bool OAdabasConnection::isStarted()
{
    return sal_True;

}

