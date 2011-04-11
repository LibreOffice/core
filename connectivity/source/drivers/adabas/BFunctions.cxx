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

//--------------------------------------------------------------------------
#include "odbc/OFunctions.hxx"
#include <osl/process.h>

// Implib-Definitionen fuer ODBC-DLL/shared library:

namespace connectivity
{
    namespace adabas
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
sal_Bool LoadLibrary_ADABAS(::rtl::OUString &_rPath)
{
    static sal_Bool bLoaded = sal_False;
    static oslModule pODBCso = NULL;

    if (bLoaded)
        return sal_True;

    rtl_uString* pPath = NULL;
    ::rtl::OUString sTemp(RTL_CONSTASCII_USTRINGPARAM("DBROOT"));
    if ( osl_getEnvironment(sTemp.pData,&pPath) == osl_Process_E_None && pPath )
    {

#if ( defined(SOLARIS) && defined(SPARC)) || defined(LINUX) || defined(MACOSX)
        _rPath = ::rtl::OUString(pPath);
        _rPath += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/lib/"));
#endif
        rtl_uString_release(pPath);
    }
    else
    {
        _rPath = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The variable DBROOT is not set."));
        return sal_False;
    }
    const sal_Char* pLibraryAsciiName = NULL;

#if defined(WNT)
    pLibraryAsciiName = "SQLOD32.DLL";
#elif ( defined(SOLARIS) && defined(SPARC)) || defined(LINUX)
    pLibraryAsciiName = "odbclib.so";
#elif defined(MACOSX)
    pLibraryAsciiName = "odbclib.dylib";
#endif
    if ( !pLibraryAsciiName )
        return sal_False;

    _rPath += ::rtl::OUString::createFromAscii( pLibraryAsciiName );
    pODBCso = osl_loadModule( _rPath.pData,SAL_LOADMODULE_NOW );
    if( !pODBCso)
        return sal_False;


    return bLoaded = LoadFunctions(pODBCso);
}
// -------------------------------------------------------------------------

sal_Bool LoadFunctions(oslModule pODBCso)
{

    if( ( pODBC3SQLAllocHandle  =   (T3SQLAllocHandle)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLAllocHandle")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLConnect      =   (T3SQLConnect)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLConnect")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLDriverConnect =  (T3SQLDriverConnect)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLDriverConnect")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLBrowseConnect =   (T3SQLBrowseConnect)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLBrowseConnect")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetInfo      =   (T3SQLGetInfo)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLGetInfo")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetTypeInfo  =   (T3SQLGetTypeInfo)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLGetTypeInfo")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetConnectAttr   =   (T3SQLSetConnectAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLSetConnectAttr")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetConnectAttr   =   (T3SQLGetConnectAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLGetConnectAttr")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetEnvAttr   =   (T3SQLSetEnvAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLSetEnvAttr")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetEnvAttr   =   (T3SQLGetEnvAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLGetEnvAttr")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetStmtAttr  =   (T3SQLSetStmtAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLSetStmtAttr")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetStmtAttr  =   (T3SQLGetStmtAttr)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLGetStmtAttr")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLPrepare      =   (T3SQLPrepare)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLPrepare")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLBindParameter =  (T3SQLBindParameter)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLBindParameter")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetCursorName =  (T3SQLSetCursorName)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLSetCursorName")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLExecute      =   (T3SQLExecute)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLExecute")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLExecDirect   =   (T3SQLExecDirect)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLExecDirect")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLDescribeParam =   (T3SQLDescribeParam)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLDescribeParam")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLNumParams        =   (T3SQLNumParams)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLNumParams")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLParamData        =   (T3SQLParamData)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLParamData")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLPutData      =   (T3SQLPutData)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLPutData")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLRowCount     =   (T3SQLRowCount)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLRowCount")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLNumResultCols =  (T3SQLNumResultCols)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLNumResultCols")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLDescribeCol  =   (T3SQLDescribeCol)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLDescribeCol")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLColAttribute =   (T3SQLColAttribute)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLColAttribute")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLBindCol      =   (T3SQLBindCol)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLBindCol")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFetch            =   (T3SQLFetch)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLFetch")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFetchScroll  =   (T3SQLFetchScroll)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLFetchScroll")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetData      =   (T3SQLGetData)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLGetData")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetPos       =   (T3SQLSetPos)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLSetPos")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLMoreResults  =   (T3SQLMoreResults)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLMoreResults")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetDiagRec   =   (T3SQLGetDiagRec)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLGetDiagRec")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLColumnPrivileges = (T3SQLColumnPrivileges)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLColumnPrivileges")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLColumns      =   (T3SQLColumns)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLColumns")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLForeignKeys  =   (T3SQLForeignKeys)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLForeignKeys")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLPrimaryKeys  =   (T3SQLPrimaryKeys)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLPrimaryKeys")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLProcedureColumns =  (T3SQLProcedureColumns)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLProcedureColumns")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLProcedures   =   (T3SQLProcedures)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLProcedures")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSpecialColumns =  (T3SQLSpecialColumns)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLSpecialColumns")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLStatistics   =   (T3SQLStatistics)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLStatistics")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLTablePrivileges =    (T3SQLTablePrivileges)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLTablePrivileges")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLTables       =   (T3SQLTables)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLTables")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFreeStmt     =   (T3SQLFreeStmt)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLFreeStmt")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLCloseCursor  =   (T3SQLCloseCursor)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLCloseCursor")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLCancel       =   (T3SQLCancel)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLCancel")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLEndTran      =   (T3SQLEndTran)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLEndTran")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLDisconnect   =   (T3SQLDisconnect)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLDisconnect")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFreeHandle   =   (T3SQLFreeHandle)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLFreeHandle")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetCursorName    =   (T3SQLGetCursorName)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLGetCursorName")).pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLNativeSql    =   (T3SQLNativeSql)osl_getFunctionSymbol(pODBCso, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQLNativeSql")).pData )) == NULL )
        return sal_False;

    return sal_True;
}
// -------------------------------------------------------------------------
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
