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

#include "ORealDriver.hxx"
#include "odbc/ODriver.hxx"
#include "odbc/OTools.hxx"
#include "odbc/OFunctions.hxx"

namespace connectivity
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


    namespace odbc
    {
        class ORealObdcDriver : public ODBCDriver
        {
        protected:
            virtual oslGenericFunction  getOdbcFunction(ODBC3SQLFunctionId _nIndex)  const override;
            virtual SQLHANDLE   EnvironmentHandle(OUString &_rPath) override;
        public:
            explicit ORealObdcDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) : ODBCDriver(_rxFactory) {}
        };


oslGenericFunction ORealObdcDriver::getOdbcFunction(ODBC3SQLFunctionId _nIndex) const
{
    oslGenericFunction pFunction = nullptr;
    switch(_nIndex)
    {
        case ODBC3SQLFunctionId::AllocHandle:
            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLAllocHandle);
            break;
        case ODBC3SQLFunctionId::Connect:
            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLConnect);
            break;
        case ODBC3SQLFunctionId::DriverConnect:
            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLDriverConnect);
            break;
        case ODBC3SQLFunctionId::BrowseConnect:
            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLBrowseConnect);
            break;
        case ODBC3SQLFunctionId::DataSources:
            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLDataSources);
            break;
        case ODBC3SQLFunctionId::Drivers:
            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLDrivers);
            break;
        case ODBC3SQLFunctionId::GetInfo:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLGetInfo);
            break;
        case ODBC3SQLFunctionId::GetFunctions:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLGetFunctions);
            break;
        case ODBC3SQLFunctionId::GetTypeInfo:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLGetTypeInfo);
            break;
        case ODBC3SQLFunctionId::SetConnectAttr:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLSetConnectAttr);
            break;
        case ODBC3SQLFunctionId::GetConnectAttr:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLGetConnectAttr);
            break;
        case ODBC3SQLFunctionId::SetEnvAttr:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLSetEnvAttr);
            break;
        case ODBC3SQLFunctionId::GetEnvAttr:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLGetEnvAttr);
            break;
        case ODBC3SQLFunctionId::SetStmtAttr:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLSetStmtAttr);
            break;
        case ODBC3SQLFunctionId::GetStmtAttr:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLGetStmtAttr);
            break;
        case ODBC3SQLFunctionId::Prepare:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLPrepare);
            break;
        case ODBC3SQLFunctionId::BindParameter:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLBindParameter);
            break;
        case ODBC3SQLFunctionId::SetCursorName:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLSetCursorName);
            break;
        case ODBC3SQLFunctionId::Execute:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLExecute);
            break;
        case ODBC3SQLFunctionId::ExecDirect:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLExecDirect);
            break;
        case ODBC3SQLFunctionId::DescribeParam:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLDescribeParam);
            break;
        case ODBC3SQLFunctionId::NumParams:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLNumParams);
            break;
        case ODBC3SQLFunctionId::ParamData:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLParamData);
            break;
        case ODBC3SQLFunctionId::PutData:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLPutData);
            break;
        case ODBC3SQLFunctionId::RowCount:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLRowCount);
            break;
        case ODBC3SQLFunctionId::NumResultCols:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLNumResultCols);
            break;
        case ODBC3SQLFunctionId::DescribeCol:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLDescribeCol);
            break;
        case ODBC3SQLFunctionId::ColAttribute:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLColAttribute);
            break;
        case ODBC3SQLFunctionId::BindCol:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLBindCol);
            break;
        case ODBC3SQLFunctionId::Fetch:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLFetch);
            break;
        case ODBC3SQLFunctionId::FetchScroll:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLFetchScroll);
            break;
        case ODBC3SQLFunctionId::GetData:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLGetData);
            break;
        case ODBC3SQLFunctionId::SetPos:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLSetPos);
            break;
        case ODBC3SQLFunctionId::BulkOperations:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLBulkOperations);
            break;
        case ODBC3SQLFunctionId::MoreResults:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLMoreResults);
            break;
        case ODBC3SQLFunctionId::GetDiagRec:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLGetDiagRec);
            break;
        case ODBC3SQLFunctionId::ColumnPrivileges:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLColumnPrivileges);
            break;
        case ODBC3SQLFunctionId::Columns:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLColumns);
            break;
        case ODBC3SQLFunctionId::ForeignKeys:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLForeignKeys);
            break;
        case ODBC3SQLFunctionId::PrimaryKeys:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLPrimaryKeys);
            break;
        case ODBC3SQLFunctionId::ProcedureColumns:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLProcedureColumns);
            break;
        case ODBC3SQLFunctionId::Procedures:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLProcedures);
            break;
        case ODBC3SQLFunctionId::SpecialColumns:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLSpecialColumns);
            break;
        case ODBC3SQLFunctionId::Statistics:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLStatistics);
            break;
        case ODBC3SQLFunctionId::TablePrivileges:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLTablePrivileges);
            break;
        case ODBC3SQLFunctionId::Tables:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLTables);
            break;
        case ODBC3SQLFunctionId::FreeStmt:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLFreeStmt);
            break;
        case ODBC3SQLFunctionId::CloseCursor:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLCloseCursor);
            break;
        case ODBC3SQLFunctionId::Cancel:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLCancel);
            break;
        case ODBC3SQLFunctionId::EndTran:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLEndTran);
            break;
        case ODBC3SQLFunctionId::Disconnect:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLDisconnect);
            break;
        case ODBC3SQLFunctionId::FreeHandle:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLFreeHandle);
            break;
        case ODBC3SQLFunctionId::GetCursorName:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLGetCursorName);
            break;
        case ODBC3SQLFunctionId::NativeSql:

            pFunction = reinterpret_cast<oslGenericFunction>(pODBC3SQLNativeSql);
            break;
        default:
            OSL_FAIL("Function unknown!");
    }
    return pFunction;
}


::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL ODBCDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new ORealObdcDriver(_rxFactory));
}

// ODBC Environment (common for all Connections):
SQLHANDLE ORealObdcDriver::EnvironmentHandle(OUString &_rPath)
{
    // Is (for this instance) already a Environment made?
    if (!m_pDriverHandle)
    {
        SQLHANDLE h = SQL_NULL_HANDLE;
        // allocate Environment

        // load ODBC-DLL now:
        if (!LoadLibrary_ODBC3(_rPath) || N3SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&h) != SQL_SUCCESS)
            return SQL_NULL_HANDLE;

        // Save in global Structure
        m_pDriverHandle = h;
        N3SQLSetEnvAttr(h, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), SQL_IS_UINTEGER);
        //N3SQLSetEnvAttr(h, SQL_ATTR_CONNECTION_POOLING,(SQLPOINTER) SQL_CP_ONE_PER_HENV, SQL_IS_INTEGER);
    }

    return m_pDriverHandle;
}


    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
