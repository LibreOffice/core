/*************************************************************************
 *
 *  $RCSfile: ORealDriver.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-05-15 08:18:32 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
            ORealObdcDriver(){}
        };

        //------------------------------------------------------------------
void* ORealObdcDriver::getOdbcFunction(sal_Int32 _nIndex) const
{
    void* pFunction = NULL;
    switch(_nIndex)
    {
        case ODBC3SQLAllocHandle:
            pFunction = pODBC3SQLAllocHandle;;
            break;
        case ODBC3SQLConnect:
            pFunction = pODBC3SQLConnect;
            break;
        case ODBC3SQLDriverConnect:
            pFunction = pODBC3SQLDriverConnect;
            break;
        case ODBC3SQLBrowseConnect:
            pFunction = pODBC3SQLBrowseConnect;
            break;
        case ODBC3SQLDataSources:
            pFunction = pODBC3SQLDataSources;
            break;
        case ODBC3SQLDrivers:
            pFunction = pODBC3SQLDrivers;
            break;
        case ODBC3SQLGetInfo:

            pFunction = pODBC3SQLGetInfo;
            break;
        case ODBC3SQLGetFunctions:

            pFunction = pODBC3SQLGetFunctions;
            break;
        case ODBC3SQLGetTypeInfo:

            pFunction = pODBC3SQLGetTypeInfo;
            break;
        case ODBC3SQLSetConnectAttr:

            pFunction = pODBC3SQLSetConnectAttr;
            break;
        case ODBC3SQLGetConnectAttr:

            pFunction = pODBC3SQLGetConnectAttr;
            break;
        case ODBC3SQLSetEnvAttr:

            pFunction = pODBC3SQLSetEnvAttr;
            break;
        case ODBC3SQLGetEnvAttr:

            pFunction = pODBC3SQLGetEnvAttr;
            break;
        case ODBC3SQLSetStmtAttr:

            pFunction = pODBC3SQLSetStmtAttr;
            break;
        case ODBC3SQLGetStmtAttr:

            pFunction = pODBC3SQLGetStmtAttr;
            break;
        case ODBC3SQLPrepare:

            pFunction = pODBC3SQLPrepare;
            break;
        case ODBC3SQLBindParameter:

            pFunction = pODBC3SQLBindParameter;
            break;
        case ODBC3SQLSetCursorName:

            pFunction = pODBC3SQLSetCursorName;
            break;
        case ODBC3SQLExecute:

            pFunction = pODBC3SQLExecute;
            break;
        case ODBC3SQLExecDirect:

            pFunction = pODBC3SQLExecDirect;
            break;
        case ODBC3SQLDescribeParam:

            pFunction = pODBC3SQLDescribeParam;
            break;
        case ODBC3SQLNumParams:

            pFunction = pODBC3SQLNumParams;
            break;
        case ODBC3SQLParamData:

            pFunction = pODBC3SQLParamData;
            break;
        case ODBC3SQLPutData:

            pFunction = pODBC3SQLPutData;
            break;
        case ODBC3SQLRowCount:

            pFunction = pODBC3SQLRowCount;
            break;
        case ODBC3SQLNumResultCols:

            pFunction = pODBC3SQLNumResultCols;
            break;
        case ODBC3SQLDescribeCol:

            pFunction = pODBC3SQLDescribeCol;
            break;
        case ODBC3SQLColAttribute:

            pFunction = pODBC3SQLColAttribute;
            break;
        case ODBC3SQLBindCol:

            pFunction = pODBC3SQLBindCol;
            break;
        case ODBC3SQLFetch:

            pFunction = pODBC3SQLFetch;
            break;
        case ODBC3SQLFetchScroll:

            pFunction = pODBC3SQLFetchScroll;
            break;
        case ODBC3SQLGetData:

            pFunction = pODBC3SQLGetData;
            break;
        case ODBC3SQLSetPos:

            pFunction = pODBC3SQLSetPos;
            break;
        case ODBC3SQLBulkOperations:

            pFunction = pODBC3SQLBulkOperations;
            break;
        case ODBC3SQLMoreResults:

            pFunction = pODBC3SQLMoreResults;
            break;
        case ODBC3SQLGetDiagRec:

            pFunction = pODBC3SQLGetDiagRec;
            break;
        case ODBC3SQLColumnPrivileges:

            pFunction = pODBC3SQLColumnPrivileges;
            break;
        case ODBC3SQLColumns:

            pFunction = pODBC3SQLColumns;
            break;
        case ODBC3SQLForeignKeys:

            pFunction = pODBC3SQLForeignKeys;
            break;
        case ODBC3SQLPrimaryKeys:

            pFunction = pODBC3SQLPrimaryKeys;
            break;
        case ODBC3SQLProcedureColumns:

            pFunction = pODBC3SQLProcedureColumns;
            break;
        case ODBC3SQLProcedures:

            pFunction = pODBC3SQLProcedures;
            break;
        case ODBC3SQLSpecialColumns:

            pFunction = pODBC3SQLSpecialColumns;
            break;
        case ODBC3SQLStatistics:

            pFunction = pODBC3SQLStatistics;
            break;
        case ODBC3SQLTablePrivileges:

            pFunction = pODBC3SQLTablePrivileges;
            break;
        case ODBC3SQLTables:

            pFunction = pODBC3SQLTables;
            break;
        case ODBC3SQLFreeStmt:

            pFunction = pODBC3SQLFreeStmt;
            break;
        case ODBC3SQLCloseCursor:

            pFunction = pODBC3SQLCloseCursor;
            break;
        case ODBC3SQLCancel:

            pFunction = pODBC3SQLCancel;
            break;
        case ODBC3SQLEndTran:

            pFunction = pODBC3SQLEndTran;
            break;
        case ODBC3SQLDisconnect:

            pFunction = pODBC3SQLDisconnect;
            break;
        case ODBC3SQLFreeHandle:

            pFunction = pODBC3SQLFreeHandle;
            break;
        case ODBC3SQLGetCursorName:

            pFunction = pODBC3SQLGetCursorName;
            break;
        case ODBC3SQLNativeSql:

            pFunction = pODBC3SQLNativeSql;
            break;
        default:
            OSL_ENSURE(0,"Function unknown!");
    }
    return pFunction;
}

//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL ODBCDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new ORealObdcDriver());
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

