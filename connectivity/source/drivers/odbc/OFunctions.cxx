/*************************************************************************
 *
 *  $RCSfile: OFunctions.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:23 $
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

//--------------------------------------------------------------------------
#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#include "odbc/OFunctions.hxx"
#endif

// Implib-Definitionen fuer ODBC-DLL/shared library:

using namespace connectivity;

// -------------------------------------------------------------------------
sal_Bool connectivity::LoadLibrary_ADABAS(::rtl::OUString &_rPath)
{
    static sal_Bool bLoaded = sal_False;
    static oslModule pODBCso = NULL;

    if (bLoaded)
        return TRUE;

    char *pPath =getenv("DBROOT");
    if(pPath)
    {

#if ( defined(SOLARIS) && defined(SPARC)) || defined(LINUX)
        _rPath = ::rtl::OUString::createFromAscii(pPath);
        _rPath += ::rtl::OUString::createFromAscii("/lib/");
#endif
    }
#if defined(WIN) || defined(WNT)
    _rPath += ::rtl::OUString::createFromAscii("SQLOD32.DLL");
#elif ( defined(SOLARIS) && defined(SPARC)) || defined(LINUX)
    _rPath += ::rtl::OUString::createFromAscii("odbclib.so");
#else
    return sal_False;
#endif

    pODBCso = osl_loadModule( _rPath.pData,SAL_LOADMODULE_NOW );
    if( !pODBCso)
        return sal_False;


    return bLoaded = LoadFunctions(pODBCso,sal_False);
}
// -------------------------------------------------------------------------
// Dynamisches Laden der DLL/shared lib und Adressen der Funktionen besorgen:
// Liefert TRUE bei Erfolg.
sal_Bool connectivity::LoadLibrary_ODBC3(::rtl::OUString &_rPath)
{
    static sal_Bool bLoaded = sal_False;
    static oslModule pODBCso = NULL;

    if (bLoaded)
        return TRUE;
#ifdef WIN
    _rPath = ::rtl::OUString::createFromAscii("ODBC.DLL");

#endif
#ifdef WNT
    _rPath = ::rtl::OUString::createFromAscii("ODBC32.DLL");
#endif
#ifdef UNX
    _rPath = ::rtl::OUString::createFromAscii("libodbc.so");
#endif
#ifdef OS2
    _rPath = ::rtl::OUString::createFromAscii("ODBC");
#endif

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

    return bLoaded = connectivity::LoadFunctions(pODBCso);
}
// -------------------------------------------------------------------------

sal_Bool connectivity::LoadFunctions(oslModule pODBCso, sal_Bool _bDS)
{

    if( ( connectivity::pODBC3SQLAllocHandle    =   (T3SQLAllocHandle)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLAllocHandle").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLConnect        =   (T3SQLConnect)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLConnect").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLDriverConnect =    (T3SQLDriverConnect)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDriverConnect").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLBrowseConnect =   (T3SQLBrowseConnect)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLBrowseConnect").pData )) == NULL )
        return sal_False;
    if(_bDS && ( connectivity::pODBC3SQLDataSources =   (T3SQLDataSources)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDataSources").pData )) == NULL )
        return sal_False;
    if(_bDS &&  ( connectivity::pODBC3SQLDrivers        =   (T3SQLDrivers)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDrivers").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLGetInfo        =   (T3SQLGetInfo)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetInfo").pData )) == NULL )
        return sal_False;
    if(_bDS && ( connectivity::pODBC3SQLGetFunctions    =   (T3SQLGetFunctions)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetFunctions").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLGetTypeInfo    =   (T3SQLGetTypeInfo)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetTypeInfo").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLSetConnectAttr =   (T3SQLSetConnectAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetConnectAttr").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLGetConnectAttr =   (T3SQLGetConnectAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetConnectAttr").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLSetEnvAttr =   (T3SQLSetEnvAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetEnvAttr").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLGetEnvAttr =   (T3SQLGetEnvAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetEnvAttr").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLSetStmtAttr    =   (T3SQLSetStmtAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetStmtAttr").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLGetStmtAttr    =   (T3SQLGetStmtAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetStmtAttr").pData )) == NULL )
        return sal_False;
    /*if( ( connectivity::pODBC3SQLSetDescField =   (T3SQLSetDescField)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetDescField").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLGetDescField   =   (T3SQLGetDescField)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDescField").pData )) == NULL )
        return sal_False;*/
    /*if( ( connectivity::pODBC3SQLGetDescRec   =   (T3SQLGetDescRec)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDescRec").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLSetDescRec =   (T3SQLSetDescRec)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetDescRec").pData )) == NULL )
        return sal_False;*/
    if( ( connectivity::pODBC3SQLPrepare        =   (T3SQLPrepare)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLPrepare").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLBindParameter =    (T3SQLBindParameter)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLBindParameter").pData )) == NULL )
        return sal_False;
//  if( ( connectivity::pODBC3SQLGetCursorName =    (T3SQLGetCursorName)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetCursorName").pData )) == NULL )
//      return sal_False;
    if( ( connectivity::pODBC3SQLSetCursorName =    (T3SQLSetCursorName)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetCursorName").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLExecute        =   (T3SQLExecute)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLExecute").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLExecDirect =   (T3SQLExecDirect)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLExecDirect").pData )) == NULL )
        return sal_False;
    /*if( ( connectivity::pODBC3SQLNativeSql        =   (T3SQLNativeSql)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNativeSql").pData )) == NULL )
        return sal_False;*/
    if( ( connectivity::pODBC3SQLDescribeParam =   (T3SQLDescribeParam)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDescribeParam").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLNumParams      =   (T3SQLNumParams)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNumParams").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLParamData      =   (T3SQLParamData)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLParamData").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLPutData        =   (T3SQLPutData)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLPutData").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLRowCount       =   (T3SQLRowCount)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLRowCount").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLNumResultCols =    (T3SQLNumResultCols)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNumResultCols").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLDescribeCol    =   (T3SQLDescribeCol)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDescribeCol").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLColAttribute = (T3SQLColAttribute)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLColAttribute").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLBindCol        =   (T3SQLBindCol)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLBindCol").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLFetch          =   (T3SQLFetch)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFetch").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLFetchScroll    =   (T3SQLFetchScroll)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFetchScroll").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLGetData        =   (T3SQLGetData)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetData").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLSetPos     =   (T3SQLSetPos)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetPos").pData )) == NULL )
        return sal_False;
    if( _bDS && ( connectivity::pODBC3SQLBulkOperations =   (T3SQLBulkOperations)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLBulkOperations").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLMoreResults    =   (T3SQLMoreResults)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLMoreResults").pData )) == NULL )
        return sal_False;
    /*if( ( connectivity::pODBC3SQLGetDiagField =   (T3SQLGetDiagField)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDiagField").pData )) == NULL )
        return sal_False;*/
    if( ( connectivity::pODBC3SQLGetDiagRec =   (T3SQLGetDiagRec)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDiagRec").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLColumnPrivileges = (T3SQLColumnPrivileges)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLColumnPrivileges").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLColumns        =   (T3SQLColumns)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLColumns").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLForeignKeys    =   (T3SQLForeignKeys)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLForeignKeys").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLPrimaryKeys    =   (T3SQLPrimaryKeys)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLPrimaryKeys").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLProcedureColumns =  (T3SQLProcedureColumns)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLProcedureColumns").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLProcedures =   (T3SQLProcedures)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLProcedures").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLSpecialColumns =  (T3SQLSpecialColumns)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSpecialColumns").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLStatistics =   (T3SQLStatistics)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLStatistics").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLTablePrivileges =  (T3SQLTablePrivileges)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLTablePrivileges").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLTables     =   (T3SQLTables)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLTables").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLFreeStmt       =   (T3SQLFreeStmt)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFreeStmt").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLCloseCursor    =   (T3SQLCloseCursor)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLCloseCursor").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLCancel     =   (T3SQLCancel)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLCancel").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLEndTran        =   (T3SQLEndTran)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLEndTran").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLDisconnect =   (T3SQLDisconnect)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDisconnect").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLFreeHandle =   (T3SQLFreeHandle)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFreeHandle").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLGetCursorName  =   (T3SQLGetCursorName)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetCursorName").pData )) == NULL )
        return sal_False;
    if( ( connectivity::pODBC3SQLNativeSql  =   (T3SQLNativeSql)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNativeSql").pData )) == NULL )
        return sal_False;

    return sal_True;
}
// -------------------------------------------------------------------------

connectivity::T3SQLAllocHandle connectivity::pODBC3SQLAllocHandle;
connectivity::T3SQLConnect connectivity::pODBC3SQLConnect;
connectivity::T3SQLDriverConnect connectivity::pODBC3SQLDriverConnect;
connectivity::T3SQLBrowseConnect connectivity::pODBC3SQLBrowseConnect;
connectivity::T3SQLDataSources connectivity::pODBC3SQLDataSources;
connectivity::T3SQLDrivers connectivity::pODBC3SQLDrivers;
connectivity::T3SQLGetInfo connectivity::pODBC3SQLGetInfo;
connectivity::T3SQLGetFunctions connectivity::pODBC3SQLGetFunctions;
connectivity::T3SQLGetTypeInfo connectivity::pODBC3SQLGetTypeInfo;
connectivity::T3SQLSetConnectAttr connectivity::pODBC3SQLSetConnectAttr;
connectivity::T3SQLGetConnectAttr connectivity::pODBC3SQLGetConnectAttr;
connectivity::T3SQLSetEnvAttr connectivity::pODBC3SQLSetEnvAttr;
connectivity::T3SQLGetEnvAttr connectivity::pODBC3SQLGetEnvAttr;
connectivity::T3SQLSetStmtAttr connectivity::pODBC3SQLSetStmtAttr;
connectivity::T3SQLGetStmtAttr connectivity::pODBC3SQLGetStmtAttr;
//connectivity::T3SQLSetDescField connectivity::pODBC3SQLSetDescField;
//connectivity::T3SQLGetDescField connectivity::pODBC3SQLGetDescField;
//connectivity::T3SQLGetDescRec connectivity::pODBC3SQLGetDescRec;
//connectivity::T3SQLSetDescRec connectivity::pODBC3SQLSetDescRec;
connectivity::T3SQLPrepare connectivity::pODBC3SQLPrepare;
connectivity::T3SQLBindParameter connectivity::pODBC3SQLBindParameter;
//connectivity::T3SQLGetCursorName connectivity::pODBC3SQLGetCursorName;
connectivity::T3SQLSetCursorName connectivity::pODBC3SQLSetCursorName;
connectivity::T3SQLExecute connectivity::pODBC3SQLExecute;
connectivity::T3SQLExecDirect connectivity::pODBC3SQLExecDirect;
//connectivity::T3SQLNativeSql connectivity::pODBC3SQLNativeSql;
connectivity::T3SQLDescribeParam connectivity::pODBC3SQLDescribeParam;
connectivity::T3SQLNumParams connectivity::pODBC3SQLNumParams;
connectivity::T3SQLParamData connectivity::pODBC3SQLParamData;
connectivity::T3SQLPutData connectivity::pODBC3SQLPutData;
connectivity::T3SQLRowCount connectivity::pODBC3SQLRowCount;
connectivity::T3SQLNumResultCols connectivity::pODBC3SQLNumResultCols;
connectivity::T3SQLDescribeCol connectivity::pODBC3SQLDescribeCol;
connectivity::T3SQLColAttribute connectivity::pODBC3SQLColAttribute;
connectivity::T3SQLBindCol connectivity::pODBC3SQLBindCol;
connectivity::T3SQLFetch connectivity::pODBC3SQLFetch;
connectivity::T3SQLFetchScroll connectivity::pODBC3SQLFetchScroll;
connectivity::T3SQLGetData connectivity::pODBC3SQLGetData;
connectivity::T3SQLSetPos connectivity::pODBC3SQLSetPos;
connectivity::T3SQLBulkOperations connectivity::pODBC3SQLBulkOperations;
connectivity::T3SQLMoreResults connectivity::pODBC3SQLMoreResults;
//connectivity::T3SQLGetDiagField connectivity::pODBC3SQLGetDiagField;
connectivity::T3SQLGetDiagRec connectivity::pODBC3SQLGetDiagRec;
connectivity::T3SQLColumnPrivileges connectivity::pODBC3SQLColumnPrivileges;
connectivity::T3SQLColumns connectivity::pODBC3SQLColumns;
connectivity::T3SQLForeignKeys connectivity::pODBC3SQLForeignKeys;
connectivity::T3SQLPrimaryKeys connectivity::pODBC3SQLPrimaryKeys;
connectivity::T3SQLProcedureColumns connectivity::pODBC3SQLProcedureColumns;
connectivity::T3SQLProcedures connectivity::pODBC3SQLProcedures;
connectivity::T3SQLSpecialColumns connectivity::pODBC3SQLSpecialColumns;
connectivity::T3SQLStatistics connectivity::pODBC3SQLStatistics;
connectivity::T3SQLTablePrivileges connectivity::pODBC3SQLTablePrivileges;
connectivity::T3SQLTables connectivity::pODBC3SQLTables;
connectivity::T3SQLFreeStmt connectivity::pODBC3SQLFreeStmt;
connectivity::T3SQLCloseCursor connectivity::pODBC3SQLCloseCursor;
connectivity::T3SQLCancel connectivity::pODBC3SQLCancel;
connectivity::T3SQLEndTran connectivity::pODBC3SQLEndTran;
connectivity::T3SQLDisconnect connectivity::pODBC3SQLDisconnect;
connectivity::T3SQLFreeHandle connectivity::pODBC3SQLFreeHandle;
connectivity::T3SQLGetCursorName connectivity::pODBC3SQLGetCursorName;
connectivity::T3SQLNativeSql connectivity::pODBC3SQLNativeSql;


