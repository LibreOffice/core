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

#include <odbc/OFunctions.hxx>
#include <osl/process.h>

// Implib definitions for ODBC-DLL/shared library:

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

static bool LoadFunctions(oslModule pODBCso);

// Take care of Dynamically loading of the DLL/shared lib and Addresses:
// Returns sal_True at success
bool LoadLibrary_ODBC3(OUString &_rPath)
{
    static bool bLoaded = false;
    static oslModule pODBCso = nullptr;

    if (bLoaded)
        return true;
#ifndef DISABLE_DYNLOADING
#ifdef _WIN32
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
#endif
    if( !pODBCso)
        return false;

    return bLoaded = LoadFunctions(pODBCso);
}


bool LoadFunctions(oslModule pODBCso)
{

    if( ( pODBC3SQLAllocHandle  =   reinterpret_cast<T3SQLAllocHandle>(osl_getFunctionSymbol(pODBCso, OUString("SQLAllocHandle").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLConnect      =   reinterpret_cast<T3SQLConnect>(osl_getFunctionSymbol(pODBCso, OUString("SQLConnect").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLDriverConnect =  reinterpret_cast<T3SQLDriverConnect>(osl_getFunctionSymbol(pODBCso, OUString("SQLDriverConnect").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLBrowseConnect =  reinterpret_cast<T3SQLBrowseConnect>(osl_getFunctionSymbol(pODBCso, OUString("SQLBrowseConnect").pData ))) == nullptr )
        return false;
    if(( pODBC3SQLDataSources   =   reinterpret_cast<T3SQLDataSources>(osl_getFunctionSymbol(pODBCso, OUString("SQLDataSources").pData ))) == nullptr )
        return false;
    if(( pODBC3SQLDrivers       =   reinterpret_cast<T3SQLDrivers>(osl_getFunctionSymbol(pODBCso, OUString("SQLDrivers").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLGetInfo      =   reinterpret_cast<T3SQLGetInfo>(osl_getFunctionSymbol(pODBCso, OUString("SQLGetInfo").pData ))) == nullptr )
        return false;
    if(( pODBC3SQLGetFunctions  =   reinterpret_cast<T3SQLGetFunctions>(osl_getFunctionSymbol(pODBCso, OUString("SQLGetFunctions").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLGetTypeInfo  =   reinterpret_cast<T3SQLGetTypeInfo>(osl_getFunctionSymbol(pODBCso, OUString("SQLGetTypeInfo").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLSetConnectAttr = reinterpret_cast<T3SQLSetConnectAttr>(osl_getFunctionSymbol(pODBCso, OUString("SQLSetConnectAttr").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLGetConnectAttr = reinterpret_cast<T3SQLGetConnectAttr>(osl_getFunctionSymbol(pODBCso, OUString("SQLGetConnectAttr").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLSetEnvAttr   =   reinterpret_cast<T3SQLSetEnvAttr>(osl_getFunctionSymbol(pODBCso, OUString("SQLSetEnvAttr").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLGetEnvAttr   =   reinterpret_cast<T3SQLGetEnvAttr>(osl_getFunctionSymbol(pODBCso, OUString("SQLGetEnvAttr").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLSetStmtAttr  =   reinterpret_cast<T3SQLSetStmtAttr>(osl_getFunctionSymbol(pODBCso, OUString("SQLSetStmtAttr").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLGetStmtAttr  =   reinterpret_cast<T3SQLGetStmtAttr>(osl_getFunctionSymbol(pODBCso, OUString("SQLGetStmtAttr").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLPrepare      =   reinterpret_cast<T3SQLPrepare>(osl_getFunctionSymbol(pODBCso, OUString("SQLPrepare").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLBindParameter =  reinterpret_cast<T3SQLBindParameter>(osl_getFunctionSymbol(pODBCso, OUString("SQLBindParameter").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLSetCursorName =  reinterpret_cast<T3SQLSetCursorName>(osl_getFunctionSymbol(pODBCso, OUString("SQLSetCursorName").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLExecute      =   reinterpret_cast<T3SQLExecute>(osl_getFunctionSymbol(pODBCso, OUString("SQLExecute").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLExecDirect   =   reinterpret_cast<T3SQLExecDirect>(osl_getFunctionSymbol(pODBCso, OUString("SQLExecDirect").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLDescribeParam =  reinterpret_cast<T3SQLDescribeParam>(osl_getFunctionSymbol(pODBCso, OUString("SQLDescribeParam").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLNumParams    =   reinterpret_cast<T3SQLNumParams>(osl_getFunctionSymbol(pODBCso, OUString("SQLNumParams").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLParamData    =   reinterpret_cast<T3SQLParamData>(osl_getFunctionSymbol(pODBCso, OUString("SQLParamData").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLPutData      =   reinterpret_cast<T3SQLPutData>(osl_getFunctionSymbol(pODBCso, OUString("SQLPutData").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLRowCount     =   reinterpret_cast<T3SQLRowCount>(osl_getFunctionSymbol(pODBCso, OUString("SQLRowCount").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLNumResultCols =  reinterpret_cast<T3SQLNumResultCols>(osl_getFunctionSymbol(pODBCso, OUString("SQLNumResultCols").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLDescribeCol  =   reinterpret_cast<T3SQLDescribeCol>(osl_getFunctionSymbol(pODBCso, OUString("SQLDescribeCol").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLColAttribute =   reinterpret_cast<T3SQLColAttribute>(osl_getFunctionSymbol(pODBCso, OUString("SQLColAttribute").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLBindCol      =   reinterpret_cast<T3SQLBindCol>(osl_getFunctionSymbol(pODBCso, OUString("SQLBindCol").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLFetch        =   reinterpret_cast<T3SQLFetch>(osl_getFunctionSymbol(pODBCso, OUString("SQLFetch").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLFetchScroll  =   reinterpret_cast<T3SQLFetchScroll>(osl_getFunctionSymbol(pODBCso, OUString("SQLFetchScroll").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLGetData      =   reinterpret_cast<T3SQLGetData>(osl_getFunctionSymbol(pODBCso, OUString("SQLGetData").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLSetPos       =   reinterpret_cast<T3SQLSetPos>(osl_getFunctionSymbol(pODBCso, OUString("SQLSetPos").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLBulkOperations = reinterpret_cast<T3SQLBulkOperations>(osl_getFunctionSymbol(pODBCso, OUString("SQLBulkOperations").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLMoreResults  =   reinterpret_cast<T3SQLMoreResults>(osl_getFunctionSymbol(pODBCso, OUString("SQLMoreResults").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLGetDiagRec   =   reinterpret_cast<T3SQLGetDiagRec>(osl_getFunctionSymbol(pODBCso, OUString("SQLGetDiagRec").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLColumnPrivileges = reinterpret_cast<T3SQLColumnPrivileges>(osl_getFunctionSymbol(pODBCso, OUString("SQLColumnPrivileges").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLColumns      =   reinterpret_cast<T3SQLColumns>(osl_getFunctionSymbol(pODBCso, OUString("SQLColumns").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLForeignKeys  =   reinterpret_cast<T3SQLForeignKeys>(osl_getFunctionSymbol(pODBCso, OUString("SQLForeignKeys").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLPrimaryKeys  =   reinterpret_cast<T3SQLPrimaryKeys>(osl_getFunctionSymbol(pODBCso, OUString("SQLPrimaryKeys").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLProcedureColumns = reinterpret_cast<T3SQLProcedureColumns>(osl_getFunctionSymbol(pODBCso, OUString("SQLProcedureColumns").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLProcedures   =   reinterpret_cast<T3SQLProcedures>(osl_getFunctionSymbol(pODBCso, OUString("SQLProcedures").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLSpecialColumns = reinterpret_cast<T3SQLSpecialColumns>(osl_getFunctionSymbol(pODBCso, OUString("SQLSpecialColumns").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLStatistics   =   reinterpret_cast<T3SQLStatistics>(osl_getFunctionSymbol(pODBCso, OUString("SQLStatistics").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLTablePrivileges = reinterpret_cast<T3SQLTablePrivileges>(osl_getFunctionSymbol(pODBCso, OUString("SQLTablePrivileges").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLTables       =   reinterpret_cast<T3SQLTables>(osl_getFunctionSymbol(pODBCso, OUString("SQLTables").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLFreeStmt     =   reinterpret_cast<T3SQLFreeStmt>(osl_getFunctionSymbol(pODBCso, OUString("SQLFreeStmt").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLCloseCursor  =   reinterpret_cast<T3SQLCloseCursor>(osl_getFunctionSymbol(pODBCso, OUString("SQLCloseCursor").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLCancel       =   reinterpret_cast<T3SQLCancel>(osl_getFunctionSymbol(pODBCso, OUString("SQLCancel").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLEndTran      =   reinterpret_cast<T3SQLEndTran>(osl_getFunctionSymbol(pODBCso, OUString("SQLEndTran").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLDisconnect   =   reinterpret_cast<T3SQLDisconnect>(osl_getFunctionSymbol(pODBCso, OUString("SQLDisconnect").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLFreeHandle   =   reinterpret_cast<T3SQLFreeHandle>(osl_getFunctionSymbol(pODBCso, OUString("SQLFreeHandle").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLGetCursorName =  reinterpret_cast<T3SQLGetCursorName>(osl_getFunctionSymbol(pODBCso, OUString("SQLGetCursorName").pData ))) == nullptr )
        return false;
    if( ( pODBC3SQLNativeSql    =   reinterpret_cast<T3SQLNativeSql>(osl_getFunctionSymbol(pODBCso, OUString("SQLNativeSql").pData ))) == nullptr )
        return false;

    return true;
}


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
