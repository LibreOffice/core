/*************************************************************************
 *
 *  $RCSfile: BDriver.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:55:39 $
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

#ifndef _CONNECTIVITY_ADABAS_BDRIVER_HXX_
#include "adabas/BDriver.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#include "adabas/BConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#include "odbc/OFunctions.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif

using namespace connectivity;
namespace connectivity
{
    namespace adabas
    {
        // extern declaration of the function pointer
    extern T3SQLAllocHandle pODBC3SQLAllocHandle;
    extern T3SQLConnect pODBC3SQLConnect;
    extern T3SQLDriverConnect pODBC3SQLDriverConnect;
    extern T3SQLBrowseConnect pODBC3SQLBrowseConnect;
    extern T3SQLDataSources pODBC3SQLDataSources;
    extern T3SQLDrivers pODBC3SQLDrivers;
    extern T3SQLGetInfo pODBC3SQLGetInfo;
    extern T3SQLGetFunctions pODBC3SQLGetFunctions;
    extern T3SQLGetTypeInfo pODBC3SQLGetTypeInfo;
    extern T3SQLSetConnectAttr pODBC3SQLSetConnectAttr;
    extern T3SQLGetConnectAttr pODBC3SQLGetConnectAttr;
    extern T3SQLSetEnvAttr pODBC3SQLSetEnvAttr;
    extern T3SQLGetEnvAttr pODBC3SQLGetEnvAttr;
    extern T3SQLSetStmtAttr pODBC3SQLSetStmtAttr;
    extern T3SQLGetStmtAttr pODBC3SQLGetStmtAttr;
    //extern T3SQLSetDescField pODBC3SQLSetDescField;
    //extern T3SQLGetDescField pODBC3SQLGetDescField;
    //extern T3SQLGetDescRec pODBC3SQLGetDescRec;
    //extern T3SQLSetDescRec pODBC3SQLSetDescRec;
    extern T3SQLPrepare pODBC3SQLPrepare;
    extern T3SQLBindParameter pODBC3SQLBindParameter;
    //extern T3SQLGetCursorName pODBC3SQLGetCursorName;
    extern T3SQLSetCursorName pODBC3SQLSetCursorName;
    extern T3SQLExecute pODBC3SQLExecute;
    extern T3SQLExecDirect pODBC3SQLExecDirect;
    //extern T3SQLNativeSql pODBC3SQLNativeSql;
    extern T3SQLDescribeParam pODBC3SQLDescribeParam;
    extern T3SQLNumParams pODBC3SQLNumParams;
    extern T3SQLParamData pODBC3SQLParamData;
    extern T3SQLPutData pODBC3SQLPutData;
    extern T3SQLRowCount pODBC3SQLRowCount;
    extern T3SQLNumResultCols pODBC3SQLNumResultCols;
    extern T3SQLDescribeCol pODBC3SQLDescribeCol;
    extern T3SQLColAttribute pODBC3SQLColAttribute;
    extern T3SQLBindCol pODBC3SQLBindCol;
    extern T3SQLFetch pODBC3SQLFetch;
    extern T3SQLFetchScroll pODBC3SQLFetchScroll;
    extern T3SQLGetData pODBC3SQLGetData;
    extern T3SQLSetPos pODBC3SQLSetPos;
    extern T3SQLBulkOperations pODBC3SQLBulkOperations;
    extern T3SQLMoreResults pODBC3SQLMoreResults;
    //extern T3SQLGetDiagField pODBC3SQLGetDiagField;
    extern T3SQLGetDiagRec pODBC3SQLGetDiagRec;
    extern T3SQLColumnPrivileges pODBC3SQLColumnPrivileges;
    extern T3SQLColumns pODBC3SQLColumns;
    extern T3SQLForeignKeys pODBC3SQLForeignKeys;
    extern T3SQLPrimaryKeys pODBC3SQLPrimaryKeys;
    extern T3SQLProcedureColumns pODBC3SQLProcedureColumns;
    extern T3SQLProcedures pODBC3SQLProcedures;
    extern T3SQLSpecialColumns pODBC3SQLSpecialColumns;
    extern T3SQLStatistics pODBC3SQLStatistics;
    extern T3SQLTablePrivileges pODBC3SQLTablePrivileges;
    extern T3SQLTables pODBC3SQLTables;
    extern T3SQLFreeStmt pODBC3SQLFreeStmt;
    extern T3SQLCloseCursor pODBC3SQLCloseCursor;
    extern T3SQLCancel pODBC3SQLCancel;
    extern T3SQLEndTran pODBC3SQLEndTran;
    extern T3SQLDisconnect pODBC3SQLDisconnect;
    extern T3SQLFreeHandle pODBC3SQLFreeHandle;
    extern T3SQLGetCursorName pODBC3SQLGetCursorName;
    extern T3SQLNativeSql pODBC3SQLNativeSql;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

    sal_Bool LoadFunctions(oslModule pODBCso);
    sal_Bool LoadLibrary_ADABAS(::rtl::OUString &_rPath);

// --------------------------------------------------------------------------------
ODriver::ODriver(const Reference< XMultiServiceFactory >& _rxFactory) : ODBCDriver(_rxFactory)
{
}
//------------------------------------------------------------------------------
void ODriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ODriver_BASE::disposing();
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString ODriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.sdbcx.adabas.ODriver");
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");
    aSNS[1] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.Driver");
    return aSNS;
}
//------------------------------------------------------------------
::rtl::OUString SAL_CALL ODriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------
sal_Bool SAL_CALL ODriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------
Any SAL_CALL ODriver::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType, static_cast<XDataDefinitionSupplier*>(this));
    return aRet.hasValue() ? aRet : ODriver_BASE::queryInterface(rType);
}
//------------------------------------------------------------------
Reference< XInterface >  SAL_CALL ODriver_CreateInstance(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFac) throw( Exception )
{
    return *(new ODriver(_rxFac));
}
// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( ! acceptsURL(url) )
        return NULL;

    if(!m_pDriverHandle)
    {
        ::rtl::OUString aPath;
        if(!EnvironmentHandle(aPath))
            throw SQLException(aPath,*this,::rtl::OUString(),1000,Any());
    }
    OAdabasConnection* pCon = new OAdabasConnection(m_pDriverHandle,this);
    Reference< XConnection > xCon = pCon;
    SQLRETURN nSQLRETURN = pCon->Construct(url,info);

    if (nSQLRETURN == SQL_ERROR || nSQLRETURN == SQL_NO_DATA)
    {
        odbc::OTools::ThrowException(pCon,nSQLRETURN,pCon->getConnection(),SQL_HANDLE_DBC,*this);
    }
    else if(SQL_SUCCESS_WITH_INFO == nSQLRETURN) // this driver does not support odbc3
    {
    }

    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}

// --------------------------------------------------------------------------------
sal_Bool SAL_CALL ODriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    return (!url.compareTo(::rtl::OUString::createFromAscii("sdbc:adabas:"),12));
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL ODriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& info) throw(SQLException, RuntimeException)
{
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet of the database."))
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        return Sequence< DriverPropertyInfo >(aDriverInfo.begin(),aDriverInfo.size());
    }

    ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid URL!")) ,*this);
    return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODriver::getMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// --------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// ODBC Environment (gemeinsam fuer alle Connections):
SQLHANDLE ODriver::EnvironmentHandle(::rtl::OUString &_rPath)
{
    // Ist (fuer diese Instanz) bereits ein Environment erzeugt worden?
    if (!m_pDriverHandle)
    {
        SQLHANDLE h = SQL_NULL_HANDLE;
        // Environment allozieren

        // ODBC-DLL jetzt laden:
        if (! LoadLibrary_ADABAS(_rPath))
            return SQL_NULL_HANDLE;

        if (N3SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&h) != SQL_SUCCESS)
            return SQL_NULL_HANDLE;

        // In globaler Struktur merken ...
        m_pDriverHandle = h;
        SQLRETURN nError = N3SQLSetEnvAttr(h, SQL_ATTR_ODBC_VERSION,(SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER);
        //N3SQLSetEnvAttr(h, SQL_ATTR_CONNECTION_POOLING,(SQLPOINTER) SQL_CP_ONE_PER_HENV, SQL_IS_INTEGER);
    }

    return m_pDriverHandle;
}
// --------------------------------------------------------------------------------
// XDataDefinitionSupplier
Reference< XTablesSupplier > SAL_CALL ODriver::getDataDefinitionByConnection( const Reference< ::com::sun::star::sdbc::XConnection >& connection ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XTablesSupplier > xTab;
    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(connection,UNO_QUERY);
    if ( xTunnel.is() )
    {

        OAdabasConnection* pConnection = NULL;
        OAdabasConnection* pSearchConnection = (OAdabasConnection*)xTunnel->getSomething(OAdabasConnection::getUnoTunnelImplementationId());
        for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
        {
            if ( (OAdabasConnection*) Reference< XConnection >::query(i->get().get()).get() == pSearchConnection )
            {
                pConnection = pSearchConnection;
                break;
            }
        }


        if ( pConnection )
            xTab = pConnection->createCatalog();
    }
    return xTab;
}

// --------------------------------------------------------------------------------
Reference< XTablesSupplier > SAL_CALL ODriver::getDataDefinitionByURL( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    if ( ! acceptsURL(url) )
        ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid URL!")) ,*this);

    return getDataDefinitionByConnection(connect(url,info));
}
// -----------------------------------------------------------------------------
void SAL_CALL ODriver::acquire() throw()
{
    ODriver_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODriver::release() throw()
{
    ODriver_BASE::release();
}
ODriver::~ODriver()
{
}
// -----------------------------------------------------------------------------
void* ODriver::getOdbcFunction(sal_Int32 _nIndex) const
{
    void* pFunction = NULL;
    switch(_nIndex)
    {
        case ODBC3SQLAllocHandle:
            pFunction = (void*)pODBC3SQLAllocHandle;;
            break;
        case ODBC3SQLConnect:
            pFunction = (void*)pODBC3SQLConnect;
            break;
        case ODBC3SQLDriverConnect:
            pFunction = (void*)pODBC3SQLDriverConnect;
            break;
        case ODBC3SQLBrowseConnect:
            pFunction = (void*)pODBC3SQLBrowseConnect;
            break;
        case ODBC3SQLDataSources:
            pFunction = (void*)pODBC3SQLDataSources;
            break;
        case ODBC3SQLDrivers:
            pFunction = (void*)pODBC3SQLDrivers;
            break;
        case ODBC3SQLGetInfo:

            pFunction = (void*)pODBC3SQLGetInfo;
            break;
        case ODBC3SQLGetFunctions:

            pFunction = (void*)pODBC3SQLGetFunctions;
            break;
        case ODBC3SQLGetTypeInfo:

            pFunction = (void*)pODBC3SQLGetTypeInfo;
            break;
        case ODBC3SQLSetConnectAttr:

            pFunction = (void*)pODBC3SQLSetConnectAttr;
            break;
        case ODBC3SQLGetConnectAttr:

            pFunction = (void*)pODBC3SQLGetConnectAttr;
            break;
        case ODBC3SQLSetEnvAttr:

            pFunction = (void*)pODBC3SQLSetEnvAttr;
            break;
        case ODBC3SQLGetEnvAttr:

            pFunction = (void*)pODBC3SQLGetEnvAttr;
            break;
        case ODBC3SQLSetStmtAttr:

            pFunction = (void*)pODBC3SQLSetStmtAttr;
            break;
        case ODBC3SQLGetStmtAttr:

            pFunction = (void*)pODBC3SQLGetStmtAttr;
            break;
        case ODBC3SQLPrepare:

            pFunction = (void*)pODBC3SQLPrepare;
            break;
        case ODBC3SQLBindParameter:

            pFunction = (void*)pODBC3SQLBindParameter;
            break;
        case ODBC3SQLSetCursorName:

            pFunction = (void*)pODBC3SQLSetCursorName;
            break;
        case ODBC3SQLExecute:

            pFunction = (void*)pODBC3SQLExecute;
            break;
        case ODBC3SQLExecDirect:

            pFunction = (void*)pODBC3SQLExecDirect;
            break;
        case ODBC3SQLDescribeParam:

            pFunction = (void*)pODBC3SQLDescribeParam;
            break;
        case ODBC3SQLNumParams:

            pFunction = (void*)pODBC3SQLNumParams;
            break;
        case ODBC3SQLParamData:

            pFunction = (void*)pODBC3SQLParamData;
            break;
        case ODBC3SQLPutData:

            pFunction = (void*)pODBC3SQLPutData;
            break;
        case ODBC3SQLRowCount:

            pFunction = (void*)pODBC3SQLRowCount;
            break;
        case ODBC3SQLNumResultCols:

            pFunction = (void*)pODBC3SQLNumResultCols;
            break;
        case ODBC3SQLDescribeCol:

            pFunction = (void*)pODBC3SQLDescribeCol;
            break;
        case ODBC3SQLColAttribute:

            pFunction = (void*)pODBC3SQLColAttribute;
            break;
        case ODBC3SQLBindCol:

            pFunction = (void*)pODBC3SQLBindCol;
            break;
        case ODBC3SQLFetch:

            pFunction = (void*)pODBC3SQLFetch;
            break;
        case ODBC3SQLFetchScroll:

            pFunction = (void*)pODBC3SQLFetchScroll;
            break;
        case ODBC3SQLGetData:

            pFunction = (void*)pODBC3SQLGetData;
            break;
        case ODBC3SQLSetPos:

            pFunction = (void*)pODBC3SQLSetPos;
            break;
        case ODBC3SQLBulkOperations:

            pFunction = (void*)pODBC3SQLBulkOperations;
            break;
        case ODBC3SQLMoreResults:

            pFunction = (void*)pODBC3SQLMoreResults;
            break;
        case ODBC3SQLGetDiagRec:

            pFunction = (void*)pODBC3SQLGetDiagRec;
            break;
        case ODBC3SQLColumnPrivileges:

            pFunction = (void*)pODBC3SQLColumnPrivileges;
            break;
        case ODBC3SQLColumns:

            pFunction = (void*)pODBC3SQLColumns;
            break;
        case ODBC3SQLForeignKeys:

            pFunction = (void*)pODBC3SQLForeignKeys;
            break;
        case ODBC3SQLPrimaryKeys:

            pFunction = (void*)pODBC3SQLPrimaryKeys;
            break;
        case ODBC3SQLProcedureColumns:

            pFunction = (void*)pODBC3SQLProcedureColumns;
            break;
        case ODBC3SQLProcedures:

            pFunction = (void*)pODBC3SQLProcedures;
            break;
        case ODBC3SQLSpecialColumns:

            pFunction = (void*)pODBC3SQLSpecialColumns;
            break;
        case ODBC3SQLStatistics:

            pFunction = (void*)pODBC3SQLStatistics;
            break;
        case ODBC3SQLTablePrivileges:

            pFunction = (void*)pODBC3SQLTablePrivileges;
            break;
        case ODBC3SQLTables:

            pFunction = (void*)pODBC3SQLTables;
            break;
        case ODBC3SQLFreeStmt:

            pFunction = (void*)pODBC3SQLFreeStmt;
            break;
        case ODBC3SQLCloseCursor:

            pFunction = (void*)pODBC3SQLCloseCursor;
            break;
        case ODBC3SQLCancel:

            pFunction = (void*)pODBC3SQLCancel;
            break;
        case ODBC3SQLEndTran:

            pFunction = (void*)pODBC3SQLEndTran;
            break;
        case ODBC3SQLDisconnect:

            pFunction = (void*)pODBC3SQLDisconnect;
            break;
        case ODBC3SQLFreeHandle:

            pFunction = (void*)pODBC3SQLFreeHandle;
            break;
        case ODBC3SQLGetCursorName:

            pFunction = (void*)pODBC3SQLGetCursorName;
            break;
        case ODBC3SQLNativeSql:

            pFunction = (void*)pODBC3SQLNativeSql;
            break;
        default:
            OSL_ENSURE(0,"Function unknown!");
    }
    return pFunction;
}
// -----------------------------------------------------------------------------
    }
}
// -----------------------------------------------------------------------------




