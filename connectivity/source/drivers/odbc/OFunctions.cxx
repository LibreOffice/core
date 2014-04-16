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
T3SQLPrepare pODBC3SQLPrepare;
T3SQLBindParameter pODBC3SQLBindParameter;
T3SQLSetCursorName pODBC3SQLSetCursorName;
T3SQLExecute pODBC3SQLExecute;
T3SQLExecDirect pODBC3SQLExecDirect;
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

bool LoadFunctions(oslModule pODBCso);

// Take care of Dynamicly loading of the DLL/shared lib and Addresses:
// Returns sal_True at success
bool LoadLibrary_ODBC3(OUString &_rPath)
{
    static bool bLoaded = false;
    static oslModule pODBCso = NULL;

    if (bLoaded)
        return true;
#ifdef WNT
    _rPath = "ODBC32.DLL";
#endif
#ifdef UNX
 #ifdef MACOSX
     _rPath = "libiodbc.dylib";
 #else
    _rPath = "libodbc.so.1";
    pODBCso = osl_loadModule( _rPath.pData,SAL_LOADMODULE_NOW );
    if ( !pODBCso )
        _rPath = "libodbc.so";
 #endif   /* MACOSX */
#endif

    if ( !pODBCso )
        pODBCso = osl_loadModule( _rPath.pData,SAL_LOADMODULE_NOW );
    if( !pODBCso)
        return false;

    return bLoaded = LoadFunctions(pODBCso);
}


bool LoadFunctions(oslModule pODBCso)
{

    if( ( pODBC3SQLAllocHandle  =   (T3SQLAllocHandle)osl_getFunctionSymbol(pODBCso, OUString("SQLAllocHandle").pData )) == NULL )
        return false;
    if( ( pODBC3SQLConnect      =   (T3SQLConnect)osl_getFunctionSymbol(pODBCso, OUString("SQLConnect").pData )) == NULL )
        return false;
    if( ( pODBC3SQLDriverConnect =  (T3SQLDriverConnect)osl_getFunctionSymbol(pODBCso, OUString("SQLDriverConnect").pData )) == NULL )
        return false;
    if( ( pODBC3SQLBrowseConnect =   (T3SQLBrowseConnect)osl_getFunctionSymbol(pODBCso, OUString("SQLBrowseConnect").pData )) == NULL )
        return false;
    if(( pODBC3SQLDataSources   =   (T3SQLDataSources)osl_getFunctionSymbol(pODBCso, OUString("SQLDataSources").pData )) == NULL )
        return false;
    if(( pODBC3SQLDrivers       =   (T3SQLDrivers)osl_getFunctionSymbol(pODBCso, OUString("SQLDrivers").pData )) == NULL )
        return false;
    if( ( pODBC3SQLGetInfo      =   (T3SQLGetInfo)osl_getFunctionSymbol(pODBCso, OUString("SQLGetInfo").pData )) == NULL )
        return false;
    if(( pODBC3SQLGetFunctions  =   (T3SQLGetFunctions)osl_getFunctionSymbol(pODBCso, OUString("SQLGetFunctions").pData )) == NULL )
        return false;
    if( ( pODBC3SQLGetTypeInfo  =   (T3SQLGetTypeInfo)osl_getFunctionSymbol(pODBCso, OUString("SQLGetTypeInfo").pData )) == NULL )
        return false;
    if( ( pODBC3SQLSetConnectAttr   =   (T3SQLSetConnectAttr)osl_getFunctionSymbol(pODBCso, OUString("SQLSetConnectAttr").pData )) == NULL )
        return false;
    if( ( pODBC3SQLGetConnectAttr   =   (T3SQLGetConnectAttr)osl_getFunctionSymbol(pODBCso, OUString("SQLGetConnectAttr").pData )) == NULL )
        return false;
    if( ( pODBC3SQLSetEnvAttr   =   (T3SQLSetEnvAttr)osl_getFunctionSymbol(pODBCso, OUString("SQLSetEnvAttr").pData )) == NULL )
        return false;
    if( ( pODBC3SQLGetEnvAttr   =   (T3SQLGetEnvAttr)osl_getFunctionSymbol(pODBCso, OUString("SQLGetEnvAttr").pData )) == NULL )
        return false;
    if( ( pODBC3SQLSetStmtAttr  =   (T3SQLSetStmtAttr)osl_getFunctionSymbol(pODBCso, OUString("SQLSetStmtAttr").pData )) == NULL )
        return false;
    if( ( pODBC3SQLGetStmtAttr  =   (T3SQLGetStmtAttr)osl_getFunctionSymbol(pODBCso, OUString("SQLGetStmtAttr").pData )) == NULL )
        return false;
    if( ( pODBC3SQLPrepare      =   (T3SQLPrepare)osl_getFunctionSymbol(pODBCso, OUString("SQLPrepare").pData )) == NULL )
        return false;
    if( ( pODBC3SQLBindParameter =  (T3SQLBindParameter)osl_getFunctionSymbol(pODBCso, OUString("SQLBindParameter").pData )) == NULL )
        return false;
    if( ( pODBC3SQLSetCursorName =  (T3SQLSetCursorName)osl_getFunctionSymbol(pODBCso, OUString("SQLSetCursorName").pData )) == NULL )
        return false;
    if( ( pODBC3SQLExecute      =   (T3SQLExecute)osl_getFunctionSymbol(pODBCso, OUString("SQLExecute").pData )) == NULL )
        return false;
    if( ( pODBC3SQLExecDirect   =   (T3SQLExecDirect)osl_getFunctionSymbol(pODBCso, OUString("SQLExecDirect").pData )) == NULL )
        return false;
    if( ( pODBC3SQLDescribeParam =   (T3SQLDescribeParam)osl_getFunctionSymbol(pODBCso, OUString("SQLDescribeParam").pData )) == NULL )
        return false;
    if( ( pODBC3SQLNumParams        =   (T3SQLNumParams)osl_getFunctionSymbol(pODBCso, OUString("SQLNumParams").pData )) == NULL )
        return false;
    if( ( pODBC3SQLParamData        =   (T3SQLParamData)osl_getFunctionSymbol(pODBCso, OUString("SQLParamData").pData )) == NULL )
        return false;
    if( ( pODBC3SQLPutData      =   (T3SQLPutData)osl_getFunctionSymbol(pODBCso, OUString("SQLPutData").pData )) == NULL )
        return false;
    if( ( pODBC3SQLRowCount     =   (T3SQLRowCount)osl_getFunctionSymbol(pODBCso, OUString("SQLRowCount").pData )) == NULL )
        return false;
    if( ( pODBC3SQLNumResultCols =  (T3SQLNumResultCols)osl_getFunctionSymbol(pODBCso, OUString("SQLNumResultCols").pData )) == NULL )
        return false;
    if( ( pODBC3SQLDescribeCol  =   (T3SQLDescribeCol)osl_getFunctionSymbol(pODBCso, OUString("SQLDescribeCol").pData )) == NULL )
        return false;
    if( ( pODBC3SQLColAttribute =   (T3SQLColAttribute)osl_getFunctionSymbol(pODBCso, OUString("SQLColAttribute").pData )) == NULL )
        return false;
    if( ( pODBC3SQLBindCol      =   (T3SQLBindCol)osl_getFunctionSymbol(pODBCso, OUString("SQLBindCol").pData )) == NULL )
        return false;
    if( ( pODBC3SQLFetch            =   (T3SQLFetch)osl_getFunctionSymbol(pODBCso, OUString("SQLFetch").pData )) == NULL )
        return false;
    if( ( pODBC3SQLFetchScroll  =   (T3SQLFetchScroll)osl_getFunctionSymbol(pODBCso, OUString("SQLFetchScroll").pData )) == NULL )
        return false;
    if( ( pODBC3SQLGetData      =   (T3SQLGetData)osl_getFunctionSymbol(pODBCso, OUString("SQLGetData").pData )) == NULL )
        return false;
    if( ( pODBC3SQLSetPos       =   (T3SQLSetPos)osl_getFunctionSymbol(pODBCso, OUString("SQLSetPos").pData )) == NULL )
        return false;
    if( ( pODBC3SQLBulkOperations   =   (T3SQLBulkOperations)osl_getFunctionSymbol(pODBCso, OUString("SQLBulkOperations").pData )) == NULL )
        return false;
    if( ( pODBC3SQLMoreResults  =   (T3SQLMoreResults)osl_getFunctionSymbol(pODBCso, OUString("SQLMoreResults").pData )) == NULL )
        return false;
    if( ( pODBC3SQLGetDiagRec   =   (T3SQLGetDiagRec)osl_getFunctionSymbol(pODBCso, OUString("SQLGetDiagRec").pData )) == NULL )
        return false;
    if( ( pODBC3SQLColumnPrivileges = (T3SQLColumnPrivileges)osl_getFunctionSymbol(pODBCso, OUString("SQLColumnPrivileges").pData )) == NULL )
        return false;
    if( ( pODBC3SQLColumns      =   (T3SQLColumns)osl_getFunctionSymbol(pODBCso, OUString("SQLColumns").pData )) == NULL )
        return false;
    if( ( pODBC3SQLForeignKeys  =   (T3SQLForeignKeys)osl_getFunctionSymbol(pODBCso, OUString("SQLForeignKeys").pData )) == NULL )
        return false;
    if( ( pODBC3SQLPrimaryKeys  =   (T3SQLPrimaryKeys)osl_getFunctionSymbol(pODBCso, OUString("SQLPrimaryKeys").pData )) == NULL )
        return false;
    if( ( pODBC3SQLProcedureColumns =  (T3SQLProcedureColumns)osl_getFunctionSymbol(pODBCso, OUString("SQLProcedureColumns").pData )) == NULL )
        return false;
    if( ( pODBC3SQLProcedures   =   (T3SQLProcedures)osl_getFunctionSymbol(pODBCso, OUString("SQLProcedures").pData )) == NULL )
        return false;
    if( ( pODBC3SQLSpecialColumns =  (T3SQLSpecialColumns)osl_getFunctionSymbol(pODBCso, OUString("SQLSpecialColumns").pData )) == NULL )
        return false;
    if( ( pODBC3SQLStatistics   =   (T3SQLStatistics)osl_getFunctionSymbol(pODBCso, OUString("SQLStatistics").pData )) == NULL )
        return false;
    if( ( pODBC3SQLTablePrivileges =    (T3SQLTablePrivileges)osl_getFunctionSymbol(pODBCso, OUString("SQLTablePrivileges").pData )) == NULL )
        return false;
    if( ( pODBC3SQLTables       =   (T3SQLTables)osl_getFunctionSymbol(pODBCso, OUString("SQLTables").pData )) == NULL )
        return false;
    if( ( pODBC3SQLFreeStmt     =   (T3SQLFreeStmt)osl_getFunctionSymbol(pODBCso, OUString("SQLFreeStmt").pData )) == NULL )
        return false;
    if( ( pODBC3SQLCloseCursor  =   (T3SQLCloseCursor)osl_getFunctionSymbol(pODBCso, OUString("SQLCloseCursor").pData )) == NULL )
        return false;
    if( ( pODBC3SQLCancel       =   (T3SQLCancel)osl_getFunctionSymbol(pODBCso, OUString("SQLCancel").pData )) == NULL )
        return false;
    if( ( pODBC3SQLEndTran      =   (T3SQLEndTran)osl_getFunctionSymbol(pODBCso, OUString("SQLEndTran").pData )) == NULL )
        return false;
    if( ( pODBC3SQLDisconnect   =   (T3SQLDisconnect)osl_getFunctionSymbol(pODBCso, OUString("SQLDisconnect").pData )) == NULL )
        return false;
    if( ( pODBC3SQLFreeHandle   =   (T3SQLFreeHandle)osl_getFunctionSymbol(pODBCso, OUString("SQLFreeHandle").pData )) == NULL )
        return false;
    if( ( pODBC3SQLGetCursorName    =   (T3SQLGetCursorName)osl_getFunctionSymbol(pODBCso, OUString("SQLGetCursorName").pData )) == NULL )
        return false;
    if( ( pODBC3SQLNativeSql    =   (T3SQLNativeSql)osl_getFunctionSymbol(pODBCso, OUString("SQLNativeSql").pData )) == NULL )
        return false;

    return true;
}


}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
