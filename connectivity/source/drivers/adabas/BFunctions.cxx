/*************************************************************************
 *
 *  $RCSfile: BFunctions.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:55:51 $
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
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

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
        _rPath += ::rtl::OUString::createFromAscii("/lib/");
#endif
        rtl_uString_release(pPath);
    }
    else
    {
        _rPath = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The variable DBROOT is not set."));
        return sal_False;
    }
#if defined(WIN) || defined(WNT)
    _rPath += ::rtl::OUString::createFromAscii("SQLOD32.DLL");
#elif ( defined(SOLARIS) && defined(SPARC)) || defined(LINUX)
    _rPath += ::rtl::OUString::createFromAscii("odbclib.so");
#elif defined(MACOSX)
    _rPath += ::rtl::OUString::createFromAscii("odbclib.dylib");
#else
    return sal_False;
#endif

    pODBCso = osl_loadModule( _rPath.pData,SAL_LOADMODULE_NOW );
    if( !pODBCso)
        return sal_False;


    return bLoaded = LoadFunctions(pODBCso);
}
// -------------------------------------------------------------------------

sal_Bool LoadFunctions(oslModule pODBCso)
{

    if( ( pODBC3SQLAllocHandle  =   (T3SQLAllocHandle)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLAllocHandle").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLConnect      =   (T3SQLConnect)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLConnect").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLDriverConnect =  (T3SQLDriverConnect)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDriverConnect").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLBrowseConnect =   (T3SQLBrowseConnect)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLBrowseConnect").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetInfo      =   (T3SQLGetInfo)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetInfo").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetTypeInfo  =   (T3SQLGetTypeInfo)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetTypeInfo").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetConnectAttr   =   (T3SQLSetConnectAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetConnectAttr").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetConnectAttr   =   (T3SQLGetConnectAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetConnectAttr").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetEnvAttr   =   (T3SQLSetEnvAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetEnvAttr").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetEnvAttr   =   (T3SQLGetEnvAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetEnvAttr").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetStmtAttr  =   (T3SQLSetStmtAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetStmtAttr").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetStmtAttr  =   (T3SQLGetStmtAttr)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetStmtAttr").pData )) == NULL )
        return sal_False;
    /*if( ( pODBC3SQLSetDescField   =   (T3SQLSetDescField)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetDescField").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetDescField =   (T3SQLGetDescField)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDescField").pData )) == NULL )
        return sal_False;*/
    /*if( ( pODBC3SQLGetDescRec =   (T3SQLGetDescRec)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDescRec").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetDescRec   =   (T3SQLSetDescRec)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetDescRec").pData )) == NULL )
        return sal_False;*/
    if( ( pODBC3SQLPrepare      =   (T3SQLPrepare)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLPrepare").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLBindParameter =  (T3SQLBindParameter)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLBindParameter").pData )) == NULL )
        return sal_False;
//  if( ( pODBC3SQLGetCursorName =  (T3SQLGetCursorName)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetCursorName").pData )) == NULL )
//      return sal_False;
    if( ( pODBC3SQLSetCursorName =  (T3SQLSetCursorName)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetCursorName").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLExecute      =   (T3SQLExecute)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLExecute").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLExecDirect   =   (T3SQLExecDirect)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLExecDirect").pData )) == NULL )
        return sal_False;
    /*if( ( pODBC3SQLNativeSql      =   (T3SQLNativeSql)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNativeSql").pData )) == NULL )
        return sal_False;*/
    if( ( pODBC3SQLDescribeParam =   (T3SQLDescribeParam)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDescribeParam").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLNumParams        =   (T3SQLNumParams)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNumParams").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLParamData        =   (T3SQLParamData)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLParamData").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLPutData      =   (T3SQLPutData)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLPutData").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLRowCount     =   (T3SQLRowCount)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLRowCount").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLNumResultCols =  (T3SQLNumResultCols)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNumResultCols").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLDescribeCol  =   (T3SQLDescribeCol)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDescribeCol").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLColAttribute =   (T3SQLColAttribute)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLColAttribute").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLBindCol      =   (T3SQLBindCol)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLBindCol").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFetch            =   (T3SQLFetch)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFetch").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFetchScroll  =   (T3SQLFetchScroll)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFetchScroll").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetData      =   (T3SQLGetData)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetData").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSetPos       =   (T3SQLSetPos)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSetPos").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLMoreResults  =   (T3SQLMoreResults)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLMoreResults").pData )) == NULL )
        return sal_False;
    /*if( ( pODBC3SQLGetDiagField   =   (T3SQLGetDiagField)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDiagField").pData )) == NULL )
        return sal_False;*/
    if( ( pODBC3SQLGetDiagRec   =   (T3SQLGetDiagRec)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetDiagRec").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLColumnPrivileges = (T3SQLColumnPrivileges)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLColumnPrivileges").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLColumns      =   (T3SQLColumns)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLColumns").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLForeignKeys  =   (T3SQLForeignKeys)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLForeignKeys").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLPrimaryKeys  =   (T3SQLPrimaryKeys)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLPrimaryKeys").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLProcedureColumns =  (T3SQLProcedureColumns)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLProcedureColumns").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLProcedures   =   (T3SQLProcedures)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLProcedures").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLSpecialColumns =  (T3SQLSpecialColumns)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLSpecialColumns").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLStatistics   =   (T3SQLStatistics)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLStatistics").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLTablePrivileges =    (T3SQLTablePrivileges)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLTablePrivileges").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLTables       =   (T3SQLTables)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLTables").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFreeStmt     =   (T3SQLFreeStmt)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFreeStmt").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLCloseCursor  =   (T3SQLCloseCursor)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLCloseCursor").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLCancel       =   (T3SQLCancel)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLCancel").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLEndTran      =   (T3SQLEndTran)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLEndTran").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLDisconnect   =   (T3SQLDisconnect)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLDisconnect").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLFreeHandle   =   (T3SQLFreeHandle)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLFreeHandle").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLGetCursorName    =   (T3SQLGetCursorName)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLGetCursorName").pData )) == NULL )
        return sal_False;
    if( ( pODBC3SQLNativeSql    =   (T3SQLNativeSql)osl_getSymbol(pODBCso, ::rtl::OUString::createFromAscii("SQLNativeSql").pData )) == NULL )
        return sal_False;

    return sal_True;
}
// -------------------------------------------------------------------------
    }
}

