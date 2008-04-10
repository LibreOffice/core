/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OFunctions.cxx,v $
 * $Revision: 1.12 $
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

//--------------------------------------------------------------------------
#include "odbc/OFunctions.hxx"
#include <osl/process.h>

// Implib-Definitionen fuer ODBC-DLL/shared library:

namespace connectivity
{
    T3SQLAllocHandle pODBC3SQLAllocHandle;
T3SQLConnect pODBC3SQLConnect;
T3SQLDriverConnect pODBC3SQLDriverConnect;
T3SQLBrowseConnect pODBC3SQLBrowseConnect;
T3SQLDataSources pODBC3SQLDataSources;
T3SQLDrivers pODBC3SQLDrivers;
T3SQLGetInfo pODBC3SQLGetInfo;
T3SQLGetFunctions pODBC3SQLGetFunctions;
T3SQLGetTypeInfo pODBC3SQLGetTypeInfo;
T3SQLSetConnectAttr pODBC3SQLSetConnectAttr;
T3SQLGetConnectAttr pODBC3SQLGetConnectAttr;
T3SQLSetEnvAttr pODBC3SQLSetEnvAttr;
T3SQLGetEnvAttr pODBC3SQLGetEnvAttr;
T3SQLSetStmtAttr pODBC3SQLSetStmtAttr;
T3SQLGetStmtAttr pODBC3SQLGetStmtAttr;
//T3SQLSetDescField pODBC3SQLSetDescField;
//T3SQLGetDescField pODBC3SQLGetDescField;
//T3SQLGetDescRec pODBC3SQLGetDescRec;
//T3SQLSetDescRec pODBC3SQLSetDescRec;
T3SQLPrepare pODBC3SQLPrepare;
T3SQLBindParameter pODBC3SQLBindParameter;
//T3SQLGetCursorName pODBC3SQLGetCursorName;
T3SQLSetCursorName pODBC3SQLSetCursorName;
T3SQLExecute pODBC3SQLExecute;
T3SQLExecDirect pODBC3SQLExecDirect;
//T3SQLNativeSql pODBC3SQLNativeSql;
T3SQLDescribeParam pODBC3SQLDescribeParam;
T3SQLNumParams pODBC3SQLNumParams;
T3SQLParamData pODBC3SQLParamData;
T3SQLPutData pODBC3SQLPutData;
T3SQLRowCount pODBC3SQLRowCount;
T3SQLNumResultCols pODBC3SQLNumResultCols;
T3SQLDescribeCol pODBC3SQLDescribeCol;
T3SQLColAttribute pODBC3SQLColAttribute;
T3SQLBindCol pODBC3SQLBindCol;
T3SQLFetch pODBC3SQLFetch;
T3SQLFetchScroll pODBC3SQLFetchScroll;
T3SQLGetData pODBC3SQLGetData;
T3SQLSetPos pODBC3SQLSetPos;
T3SQLBulkOperations pODBC3SQLBulkOperations;
T3SQLMoreResults pODBC3SQLMoreResults;
//T3SQLGetDiagField pODBC3SQLGetDiagField;
T3SQLGetDiagRec pODBC3SQLGetDiagRec;
T3SQLColumnPrivileges pODBC3SQLColumnPrivileges;
T3SQLColumns pODBC3SQLColumns;
T3SQLForeignKeys pODBC3SQLForeignKeys;
T3SQLPrimaryKeys pODBC3SQLPrimaryKeys;
T3SQLProcedureColumns pODBC3SQLProcedureColumns;
T3SQLProcedures pODBC3SQLProcedures;
T3SQLSpecialColumns pODBC3SQLSpecialColumns;
T3SQLStatistics pODBC3SQLStatistics;
T3SQLTablePrivileges pODBC3SQLTablePrivileges;
T3SQLTables pODBC3SQLTables;
T3SQLFreeStmt pODBC3SQLFreeStmt;
T3SQLCloseCursor pODBC3SQLCloseCursor;
T3SQLCancel pODBC3SQLCancel;
T3SQLEndTran pODBC3SQLEndTran;
T3SQLDisconnect pODBC3SQLDisconnect;
T3SQLFreeHandle pODBC3SQLFreeHandle;
T3SQLGetCursorName pODBC3SQLGetCursorName;
T3SQLNativeSql pODBC3SQLNativeSql;

sal_Bool LoadFunctions(oslModule pODBCso);
// -------------------------------------------------------------------------
// Dynamisches Laden der DLL/shared lib und Adressen der Funktionen besorgen:
// Liefert sal_True bei Erfolg.
sal_Bool LoadLibrary_ODBC3(::rtl::OUString &_rPath)
{
    static sal_Bool bLoaded = sal_False;
    static oslModule pODBCso = NULL;

    if (bLoaded)
        return sal_True;
#ifdef WIN
    _rPath = ::rtl::OUString::createFromAscii("ODBC.DLL");

#endif
#ifdef WNT
    _rPath = ::rtl::OUString::createFromAscii("ODBC32.DLL");
#endif
#ifdef UNX
 #ifdef MACOSX
     _rPath = ::rtl::OUString::createFromAscii("libiodbc.dylib");
 #else
    _rPath = ::rtl::OUString::createFromAscii("libodbc.so.1");
    pODBCso = osl_loadModule( _rPath.pData,SAL_LOADMODULE_NOW );
    if ( !pODBCso )
        _rPath = ::rtl::OUString::createFromAscii("libodbc.so");
 #endif   /* MACOSX */
#endif
#ifdef OS2
    _rPath = ::rtl::OUString::createFromAscii("ODBC");
#endif

    if ( !pODBCso )
        pODBCso = osl_loadModule( _rPath.pData,SAL_LOADMODULE_NOW );
    if( !pODBCso)
#ifdef OS2
    {
        delete pODBCso;
        _rPath = ::rtl::OUString::createFromAscii("WOD402");
        pODBCso = osl_loadModule( _rPath.pData,SAL_LOADMODULE_NOW );
        if( !pODBCso)
            return sal_False;
    }
#else
        return sal_False;
#endif

    return bLoaded = LoadFunctions(pODBCso);
}
// -------------------------------------------------------------------------

sal_Bool LoadFunctions(oslModule pODBCso)
{

    if( ( pODBC3SQLAllocHandle  =   (T3SQLAllocHandle)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLAllocHandle").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLConnect      =   (T3SQLConnect)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLConnect").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLDriverConnect =  (T3SQLDriverConnect)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDriverConnect").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLBrowseConnect =   (T3SQLBrowseConnect)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLBrowseConnect").pData )) == NULL )
        return sal_False;
    if(( pODBC3SQLDataSources   =   (T3SQLDataSources)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDataSources").pData )) == NULL )
        return sal_False;
    if(( pODBC3SQLDrivers       =   (T3SQLDrivers)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDrivers").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetInfo      =   (T3SQLGetInfo)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetInfo").pData )) == NULL )
        return sal_False;
    if(( pODBC3SQLGetFunctions  =   (T3SQLGetFunctions)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetFunctions").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetTypeInfo  =   (T3SQLGetTypeInfo)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetTypeInfo").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetConnectAttr   =   (T3SQLSetConnectAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetConnectAttr").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetConnectAttr   =   (T3SQLGetConnectAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetConnectAttr").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetEnvAttr   =   (T3SQLSetEnvAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetEnvAttr").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetEnvAttr   =   (T3SQLGetEnvAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetEnvAttr").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetStmtAttr  =   (T3SQLSetStmtAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetStmtAttr").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetStmtAttr  =   (T3SQLGetStmtAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetStmtAttr").pData )) == NULL )
        return sal_False;
    /*if( ( pODBC3SQLSetDescField   =   (T3SQLSetDescField)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetDescField").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetDescField =   (T3SQLGetDescField)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDescField").pData )) == NULL )
        return sal_False;*/
    /*if( ( pODBC3SQLGetDescRec =   (T3SQLGetDescRec)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDescRec").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetDescRec   =   (T3SQLSetDescRec)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetDescRec").pData )) == NULL )
        return sal_False;*/
    if( ( pODBC3SQLPrepare      =   (T3SQLPrepare)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLPrepare").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLBindParameter =  (T3SQLBindParameter)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLBindParameter").pData )) == NULL )
        return sal_False;
//  if( ( pODBC3SQLGetCursorName =  (T3SQLGetCursorName)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetCursorName").pData )) == NULL )
//      return sal_False;
    if( ( pODBC3SQLSetCursorName =  (T3SQLSetCursorName)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetCursorName").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLExecute      =   (T3SQLExecute)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLExecute").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLExecDirect   =   (T3SQLExecDirect)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLExecDirect").pData )) == NULL )
        return sal_False;
    /*if( ( pODBC3SQLNativeSql      =   (T3SQLNativeSql)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNativeSql").pData )) == NULL )
        return sal_False;*/
    if( ( pODBC3SQLDescribeParam =   (T3SQLDescribeParam)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDescribeParam").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLNumParams        =   (T3SQLNumParams)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNumParams").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLParamData        =   (T3SQLParamData)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLParamData").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLPutData      =   (T3SQLPutData)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLPutData").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLRowCount     =   (T3SQLRowCount)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLRowCount").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLNumResultCols =  (T3SQLNumResultCols)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNumResultCols").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLDescribeCol  =   (T3SQLDescribeCol)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDescribeCol").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLColAttribute =   (T3SQLColAttribute)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLColAttribute").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLBindCol      =   (T3SQLBindCol)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLBindCol").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFetch            =   (T3SQLFetch)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFetch").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFetchScroll  =   (T3SQLFetchScroll)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFetchScroll").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetData      =   (T3SQLGetData)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetData").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetPos       =   (T3SQLSetPos)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetPos").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLBulkOperations   =   (T3SQLBulkOperations)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLBulkOperations").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLMoreResults  =   (T3SQLMoreResults)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLMoreResults").pData )) == NULL )
        return sal_False;
    /*if( ( pODBC3SQLGetDiagField   =   (T3SQLGetDiagField)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDiagField").pData )) == NULL )
        return sal_False;*/
    if( ( pODBC3SQLGetDiagRec   =   (T3SQLGetDiagRec)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDiagRec").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLColumnPrivileges = (T3SQLColumnPrivileges)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLColumnPrivileges").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLColumns      =   (T3SQLColumns)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLColumns").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLForeignKeys  =   (T3SQLForeignKeys)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLForeignKeys").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLPrimaryKeys  =   (T3SQLPrimaryKeys)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLPrimaryKeys").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLProcedureColumns =  (T3SQLProcedureColumns)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLProcedureColumns").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLProcedures   =   (T3SQLProcedures)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLProcedures").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSpecialColumns =  (T3SQLSpecialColumns)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSpecialColumns").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLStatistics   =   (T3SQLStatistics)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLStatistics").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLTablePrivileges =    (T3SQLTablePrivileges)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLTablePrivileges").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLTables       =   (T3SQLTables)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLTables").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFreeStmt     =   (T3SQLFreeStmt)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFreeStmt").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLCloseCursor  =   (T3SQLCloseCursor)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLCloseCursor").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLCancel       =   (T3SQLCancel)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLCancel").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLEndTran      =   (T3SQLEndTran)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLEndTran").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLDisconnect   =   (T3SQLDisconnect)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDisconnect").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFreeHandle   =   (T3SQLFreeHandle)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFreeHandle").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetCursorName    =   (T3SQLGetCursorName)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetCursorName").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLNativeSql    =   (T3SQLNativeSql)osl_getFunctionSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNativeSql").pData )) == NULL )
        return sal_False;

    return sal_True;
}
// -------------------------------------------------------------------------

}



