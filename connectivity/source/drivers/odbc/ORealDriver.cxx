/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ORealDriver.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:35:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ODBC_ODRIVER_HXX_
#include "odbc/ODriver.hxx"
#endif
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#include "odbc/OFunctions.hxx"
#endif

namespace connectivity
{
    sal_Bool LoadFunctions(oslModule pODBCso);
    sal_Bool LoadLibrary_ODBC3(::rtl::OUString &_rPath);
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


    namespace odbc
    {
        class ORealObdcDriver : public ODBCDriver
        {
        protected:
            virtual void* getOdbcFunction(sal_Int32 _nIndex)  const;
            virtual SQLHANDLE   EnvironmentHandle(::rtl::OUString &_rPath);
        public:
            ORealObdcDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) : ODBCDriver(_rxFactory) {}
        };

        //------------------------------------------------------------------
void* ORealObdcDriver::getOdbcFunction(sal_Int32 _nIndex) const
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

//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL ODBCDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new ORealObdcDriver(_rxFactory));
}
// -----------------------------------------------------------------------------
// ODBC Environment (gemeinsam fuer alle Connections):
SQLHANDLE ORealObdcDriver::EnvironmentHandle(::rtl::OUString &_rPath)
{
    // Ist (fuer diese Instanz) bereits ein Environment erzeugt worden?
    if (!m_pDriverHandle)
    {
        SQLHANDLE h = SQL_NULL_HANDLE;
        // Environment allozieren

        // ODBC-DLL jetzt laden:
        if (!LoadLibrary_ODBC3(_rPath) || N3SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&h) != SQL_SUCCESS)
            return SQL_NULL_HANDLE;

        // In globaler Struktur merken ...
        m_pDriverHandle = h;
        SQLRETURN nError = N3SQLSetEnvAttr(h, SQL_ATTR_ODBC_VERSION,(SQLPOINTER) SQL_OV_ODBC3, SQL_IS_UINTEGER);
        //N3SQLSetEnvAttr(h, SQL_ATTR_CONNECTION_POOLING,(SQLPOINTER) SQL_CP_ONE_PER_HENV, SQL_IS_INTEGER);
    }

    return m_pDriverHandle;
}
// -----------------------------------------------------------------------------

    }
}

