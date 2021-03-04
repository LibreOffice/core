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

#pragma once

#include <connectivity/odbc.hxx>
#include <rtl/ustring.hxx>
#include <osl/module.h>

namespace connectivity
{

//  sal_Bool LoadFunctions(oslModule pODBCso, sal_Bool _bDS=sal_True);
bool LoadLibrary_ODBC3(OUString &_rPath);
//  sal_Bool LoadLibrary_ADABAS(OUString &_rPath);

    // Connecting to a data source
    typedef SQLRETURN  (SQL_API  *T3SQLAllocHandle) (SQLSMALLINT HandleType,SQLHANDLE InputHandle,SQLHANDLE *   OutputHandlePtr);

    #define N3SQLAllocHandle(a,b,c) (*reinterpret_cast<T3SQLAllocHandle>(getOdbcFunction(ODBC3SQLFunctionId::AllocHandle)))(a,b,c)

    typedef SQLRETURN  (SQL_API  *T3SQLConnect) (SQLHDBC ConnectionHandle,SQLCHAR *ServerName,SQLSMALLINT   NameLength1,SQLCHAR *UserName,SQLSMALLINT NameLength2,SQLCHAR *Authentication,SQLSMALLINT NameLength3);

    #define N3SQLConnect(a,b,c,d,e,f,g) (*reinterpret_cast<T3SQLConnect>(getOdbcFunction(ODBC3SQLFunctionId::Connect)))(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLDriverConnect) ( SQLHDBC         ConnectionHandle,
                                                        HWND            WindowHandle,
                                                        SQLCHAR *       InConnectionString,
                                                        SQLSMALLINT     StringLength1,
                                                        SQLCHAR *       OutConnectionString,
                                                        SQLSMALLINT     BufferLength,
                                                        SQLSMALLINT *   StringLength2Ptr,
                                                        SQLUSMALLINT    DriverCompletion);

    #define N3SQLDriverConnect(a,b,c,d,e,f,g,h) (*reinterpret_cast<T3SQLDriverConnect>(getOdbcFunction(ODBC3SQLFunctionId::DriverConnect)))(a,b,c,d,e,f,g,h)

    typedef SQLRETURN  (SQL_API  *T3SQLBrowseConnect) ( SQLHDBC         ConnectionHandle,
                                                        SQLCHAR *       InConnectionString,
                                                        SQLSMALLINT     StringLength1,
                                                        SQLCHAR *       OutConnectionString,
                                                        SQLSMALLINT     BufferLength,
                                                        SQLSMALLINT *   StringLength2Ptr);

    #define N3SQLBrowseConnect(a,b,c,d,e,f) (*reinterpret_cast<T3SQLBrowseConnect>(getOdbcFunction(ODBC3SQLFunctionId::BrowseConnect)))(a,b,c,d,e,f)

    // Obtaining information about a driver and data source
    typedef SQLRETURN  (SQL_API  *T3SQLDataSources) (   SQLHENV         EnvironmentHandle,
                                                        SQLUSMALLINT    Direction,
                                                        SQLCHAR *       ServerName,
                                                        SQLSMALLINT     BufferLength1,
                                                        SQLSMALLINT *   NameLength1Ptr,
                                                        SQLCHAR *       Description,
                                                        SQLSMALLINT     BufferLength2,
                                                        SQLSMALLINT *   NameLength2Ptr);

    #define N3SQLDataSources(a,b,c,d,e,f,g,h) (*reinterpret_cast<T3SQLDataSources>(getOdbcFunction(ODBC3SQLFunctionId::DataSources)))(a,b,c,d,e,f,g,h)

    typedef SQLRETURN  (SQL_API  *T3SQLDrivers) (   SQLHENV         EnvironmentHandle,
                                                    SQLUSMALLINT    Direction,
                                                    SQLCHAR *       DriverDescription,
                                                    SQLSMALLINT     BufferLength1,
                                                    SQLSMALLINT *   DescriptionLengthPtr,
                                                    SQLCHAR *       DriverAttributes,
                                                    SQLSMALLINT     BufferLength2,
                                                    SQLSMALLINT *   AttributesLengthPtr);

    #define N3SQLDrivers(a,b,c,d,e,f,g,h) (*reinterpret_cast<T3SQLDrivers>(getOdbcFunction(ODBC3SQLFunctionId::Drivers)))(a,b,c,d,e,f,g,h)

    typedef SQLRETURN  (SQL_API  *T3SQLGetInfo) (   SQLHDBC         ConnectionHandle,
                                                    SQLUSMALLINT    InfoType,
                                                    SQLPOINTER      InfoValuePtr,
                                                    SQLSMALLINT     BufferLength,
                                                    SQLSMALLINT *   StringLengthPtr);

    #define N3SQLGetInfo(a,b,c,d,e) (*reinterpret_cast<T3SQLGetInfo>(getOdbcFunction(ODBC3SQLFunctionId::GetInfo)))(a,b,c,d,e)

    typedef SQLRETURN  (SQL_API  *T3SQLGetFunctions) (SQLHDBC           ConnectionHandle,
                                                    SQLUSMALLINT    FunctionId,
                                                    SQLUSMALLINT *  SupportedPtr);

    #define N3SQLGetFunctions(a,b,c) (*reinterpret_cast<T3SQLGetFunctions>(getOdbcFunction(ODBC3SQLFunctionId::GetFunctions)))(a,b,c)

    typedef SQLRETURN  (SQL_API  *T3SQLGetTypeInfo) (   SQLHSTMT    StatementHandle,
                                                    SQLSMALLINT     DataType);

    #define N3SQLGetTypeInfo(a,b) (*reinterpret_cast<T3SQLGetTypeInfo>(getOdbcFunction(ODBC3SQLFunctionId::GetTypeInfo)))(a,b)

    // Setting and retrieving driver attributes
    typedef SQLRETURN (SQL_API *T3SQLSetConnectAttr)(SQLHDBC        ConnectionHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      StringLength);

    #define N3SQLSetConnectAttr(a,b,c,d) (*reinterpret_cast<T3SQLSetConnectAttr>(getOdbcFunction(ODBC3SQLFunctionId::SetConnectAttr)))(a,b,c,d)

    typedef SQLRETURN (SQL_API *T3SQLGetConnectAttr) (SQLHDBC       ConnectionHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER*     StringLength);

    #define N3SQLGetConnectAttr(a,b,c,d,e) (*reinterpret_cast<T3SQLGetConnectAttr>(getOdbcFunction(ODBC3SQLFunctionId::GetConnectAttr)))(a,b,c,d,e)


    typedef SQLRETURN (SQL_API *T3SQLSetEnvAttr) (  SQLHENV         EnvironmentHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      StringLength);

    #define N3SQLSetEnvAttr(a,b,c,d) (*reinterpret_cast<T3SQLSetEnvAttr>(getOdbcFunction(ODBC3SQLFunctionId::SetEnvAttr)))(a,b,c,d)

    typedef SQLRETURN (SQL_API *T3SQLGetEnvAttr) (  SQLHENV         EnvironmentHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER*     StringLength);

    #define N3SQLGetEnvAttr(a,b,c,d,e) (*reinterpret_cast<T3SQLGetEnvAttr>(getOdbcFunction(ODBC3SQLFunctionId::GetEnvAttr)))(a,b,c,d,e)


    typedef SQLRETURN (SQL_API *T3SQLSetStmtAttr) ( SQLHSTMT        StatementHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      StringLength);

    #define N3SQLSetStmtAttr(a,b,c,d) (*reinterpret_cast<T3SQLSetStmtAttr>(getOdbcFunction(ODBC3SQLFunctionId::SetStmtAttr)))(a,b,c,d)

    typedef SQLRETURN (SQL_API *T3SQLGetStmtAttr) ( SQLHSTMT        StatementHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER*     StringLength);

    #define N3SQLGetStmtAttr(a,b,c,d,e) (*reinterpret_cast<T3SQLGetStmtAttr>(getOdbcFunction(ODBC3SQLFunctionId::GetStmtAttr)))(a,b,c,d,e)

    // Setting and retrieving descriptor fields
    /*typedef SQLRETURN (SQL_API *T3SQLSetDescField) (SQLHDESC      DescriptorHandle,
                                                    SQLSMALLINT     RecNumber,
                                                    SQLSMALLINT     FieldIdentifier,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength);

    #define N3SQLSetDescField(a,b,c,d,e) (*reinterpret_cast<T3SQLSetDescField>(getOdbcFunction(ODBC3SQLFunctionId::SetDescField)))(a,b,c,d,e)

    typedef SQLRETURN (SQL_API *T3SQLGetDescField) (    SQLHDESC        DescriptorHandle,
                                                    SQLSMALLINT     RecNumber,
                                                    SQLSMALLINT     FieldIdentifier,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER *    StringLengthPtr);

    #define N3SQLGetDescField(a,b,c,d,e,f) (*reinterpret_cast<T3SQLGetDescField>(getOdbcFunction(ODBC3SQLFunctionId::GetDescField)))(a,b,c,d,e,f)


    typedef SQLRETURN (SQL_API *T3SQLGetDescRec) (  SQLHDESC            DescriptorHandle,
                                                    SQLSMALLINT         RecNumber,
                                                    SQLCHAR *           Name,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       StringLengthPtr,
                                                    SQLSMALLINT *       TypePtr,
                                                    SQLSMALLINT *       SubTypePtr,
                                                    SQLLEN *            LengthPtr,
                                                    SQLSMALLINT *       PrecisionPtr,
                                                    SQLSMALLINT *       ScalePtr,
                                                    SQLSMALLINT *       NullablePtr);

    #define N3SQLGetDescRec(a,b,c,d,e,f,g,h,i,j,k) (*reinterpret_cast<T3SQLGetDescRec>(getOdbcFunction(ODBC3SQLFunctionId::GetDescRec)))(a,b,c,d,e,f,g,h,i,j,k)


    typedef SQLRETURN (SQL_API *T3SQLSetDescRec) (  SQLHDESC            DescriptorHandle,
                                                    SQLSMALLINT         RecNumber,
                                                    SQLSMALLINT         Type,
                                                    SQLSMALLINT         SubType,
                                                    SQLLEN              Length,
                                                    SQLSMALLINT         Precision,
                                                    SQLSMALLINT         Scale,
                                                    SQLPOINTER          DataPtr,
                                                    SQLLEN *            StringLengthPtr,
                                                    SQLLEN *            IndicatorPtr);

    #define N3SQLSetDescRec(a,b,c,d,e,f,g,h,i,j) (*reinterpret_cast<T3SQLSetDescRec>(getOdbcFunction(ODBC3SQLFunctionId::SetDescRec)))(a,b,c,d,e,f,g,h,i,j)
    */

    // Preparing SQL requests
    typedef SQLRETURN  (SQL_API  *T3SQLPrepare) (       SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           StatementText,
                                                    SQLINTEGER          TextLength);

    #define N3SQLPrepare(a,b,c) (*reinterpret_cast<T3SQLPrepare>(getOdbcFunction(ODBC3SQLFunctionId::Prepare)))(a,b,c)

    typedef SQLRETURN (SQL_API  *T3SQLBindParameter) (SQLHSTMT          StatementHandle,
                                                    SQLUSMALLINT        ParameterNumber,
                                                    SQLSMALLINT         InputOutputType,
                                                    SQLSMALLINT         ValueType,
                                                    SQLSMALLINT         ParameterType,
                                                    SQLULEN             ColumnSize,
                                                    SQLSMALLINT         DecimalDigits,
                                                    SQLPOINTER          ParameterValuePtr,
                                                    SQLLEN              BufferLength,
                                                    SQLLEN *            StrLen_or_IndPtr);

    #define N3SQLBindParameter(a,b,c,d,e,f,g,h,i,j) (*reinterpret_cast<T3SQLBindParameter>(getOdbcFunction(ODBC3SQLFunctionId::BindParameter)))(a,b,c,d,e,f,g,h,i,j)

    /*typedef SQLRETURN (SQL_API  *T3SQLGetCursorName) (SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CursorName,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       NameLengthPtr);

    #define N3SQLGetCursorName(a,b,c,d) (*reinterpret_cast<T3SQLGetCursorName>(getOdbcFunction(ODBC3SQLFunctionId::GetCursorName)))(a,b,c,d)
    */

    typedef SQLRETURN (SQL_API  *T3SQLSetCursorName) (SQLHSTMT          StatementHandle,
                                                    SQLCHAR *           CursorName,
                                                    SQLSMALLINT         NameLength);

    #define N3SQLSetCursorName(a,b,c) (*reinterpret_cast<T3SQLSetCursorName>(getOdbcFunction(ODBC3SQLFunctionId::SetCursorName)))(a,b,c)

    // Submitting requests
    typedef SQLRETURN  (SQL_API  *T3SQLExecute) (       SQLHSTMT            StatementHandle);

    #define N3SQLExecute(a) (*reinterpret_cast<T3SQLExecute>(getOdbcFunction(ODBC3SQLFunctionId::Execute)))(a)

    typedef SQLRETURN  (SQL_API  *T3SQLExecDirect) (    SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           StatementText,
                                                    SQLINTEGER          TextLength);

    #define N3SQLExecDirect(a,b,c) (*reinterpret_cast<T3SQLExecDirect>(getOdbcFunction(ODBC3SQLFunctionId::ExecDirect)))(a,b,c)

    typedef SQLRETURN (SQL_API  *T3SQLDescribeParam) (SQLHSTMT          StatementHandle,
                                                    SQLUSMALLINT        ParameterNumber,
                                                    SQLSMALLINT *       DataTypePtr,
                                                    SQLULEN *           ParameterSizePtr,
                                                    SQLSMALLINT *       DecimalDigitsPtr,
                                                    SQLSMALLINT *       NullablePtr);

    #define N3SQLDescribeParam(a,b,c,d,e,f) (*reinterpret_cast<T3SQLDescribeParam>(getOdbcFunction(ODBC3SQLFunctionId::DescribeParam)))(a,b,c,d,e,f)

    typedef SQLRETURN  (SQL_API  *T3SQLNumParams) ( SQLHSTMT            StatementHandle,
                                                    SQLSMALLINT *       ParameterCountPtr);

    #define N3SQLNumParams(a,b) (*reinterpret_cast<T3SQLNumParams>(getOdbcFunction(ODBC3SQLFunctionId::NumParams)))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLParamData) ( SQLHSTMT            StatementHandle,
                                                    SQLPOINTER *        ValuePtrPtr);

    #define N3SQLParamData(a,b) (*reinterpret_cast<T3SQLParamData>(getOdbcFunction(ODBC3SQLFunctionId::ParamData)))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLPutData) (       SQLHSTMT            StatementHandle,
                                                    SQLPOINTER          DataPtr,
                                                    SQLLEN              StrLen_or_Ind);

    #define N3SQLPutData(a,b,c) (*reinterpret_cast<T3SQLPutData>(getOdbcFunction(ODBC3SQLFunctionId::PutData)))(a,b,c)

    // Retrieving results and information about results
    typedef SQLRETURN  (SQL_API  *T3SQLRowCount) (  SQLHSTMT            StatementHandle,
                                                    SQLLEN *            RowCountPtr);

    #define N3SQLRowCount(a,b) (*reinterpret_cast<T3SQLRowCount>(getOdbcFunction(ODBC3SQLFunctionId::RowCount)))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLNumResultCols) (SQLHSTMT         StatementHandle,
                                                    SQLSMALLINT *       ColumnCountPtr);

    #define N3SQLNumResultCols(a,b) (*reinterpret_cast<T3SQLNumResultCols>(getOdbcFunction(ODBC3SQLFunctionId::NumResultCols)))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLDescribeCol) (   SQLHSTMT            StatementHandle,
                                                    SQLUSMALLINT        ColumnNumber,
                                                    SQLCHAR *           ColumnName,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       NameLengthPtr,
                                                    SQLSMALLINT *       DataTypePtr,
                                                    SQLULEN *           ColumnSizePtr,
                                                    SQLSMALLINT *       DecimalDigitsPtr,
                                                    SQLSMALLINT *       NullablePtr);

    #define N3SQLDescribeCol(a,b,c,d,e,f,g,h,i) (*reinterpret_cast<T3SQLDescribeCol>(getOdbcFunction(ODBC3SQLFunctionId::DescribeCol)))(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLColAttribute) (SQLHSTMT          StatementHandle,
                                                    SQLUSMALLINT        ColumnNumber,
                                                    SQLUSMALLINT        FieldIdentifier,
                                                    SQLPOINTER          CharacterAttributePtr,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       StringLengthPtr,
                                                    SQLLEN *            NumericAttributePtr);

    #define N3SQLColAttribute(a,b,c,d,e,f,g) (*reinterpret_cast<T3SQLColAttribute>(getOdbcFunction(ODBC3SQLFunctionId::ColAttribute)))(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLBindCol) (       SQLHSTMT            StatementHandle,
                                                    SQLUSMALLINT        ColumnNumber,
                                                    SQLSMALLINT         TargetType,
                                                    SQLPOINTER          TargetValuePtr,
                                                    SQLLEN              BufferLength,
                                                    SQLLEN *            StrLen_or_IndPtr);

    #define N3SQLBindCol(a,b,c,d,e,f) (*reinterpret_cast<T3SQLBindCol>(getOdbcFunction(ODBC3SQLFunctionId::BindCol)))(a,b,c,d,e,f)

    typedef SQLRETURN  (SQL_API  *T3SQLFetch) (     SQLHSTMT            StatementHandle);

    #define N3SQLFetch(a) (*reinterpret_cast<T3SQLFetch>(getOdbcFunction(ODBC3SQLFunctionId::Fetch)))(a)

    typedef SQLRETURN  (SQL_API  *T3SQLFetchScroll) (   SQLHSTMT            StatementHandle,
                                                    SQLSMALLINT         FetchOrientation,
                                                    SQLLEN              FetchOffset);

    #define N3SQLFetchScroll(a,b,c) (*reinterpret_cast<T3SQLFetchScroll>(getOdbcFunction(ODBC3SQLFunctionId::FetchScroll)))(a,b,c)

    typedef SQLRETURN  (SQL_API  *T3SQLGetData) (       SQLHSTMT            StatementHandle,
                                                    SQLUSMALLINT        ColumnNumber,
                                                    SQLSMALLINT         TargetType,
                                                    SQLPOINTER          TargetValuePtr,
                                                    SQLLEN              BufferLength,
                                                    SQLLEN *            StrLen_or_IndPtr);

    #define N3SQLGetData(a,b,c,d,e,f) (*reinterpret_cast<T3SQLGetData>(getOdbcFunction(ODBC3SQLFunctionId::GetData)))(a,b,c,d,e,f)

    typedef SQLRETURN  (SQL_API  *T3SQLSetPos) (        SQLHSTMT            StatementHandle,
                                                    SQLSETPOSIROW       RowNumber,
                                                    SQLUSMALLINT        Operation,
                                                    SQLUSMALLINT        LockType);

    #define N3SQLSetPos(a,b,c,d) (*reinterpret_cast<T3SQLSetPos>(getOdbcFunction(ODBC3SQLFunctionId::SetPos)))(a,b,c,d)

    typedef SQLRETURN  (SQL_API  *T3SQLBulkOperations) (    SQLHSTMT        StatementHandle,
                                                        SQLSMALLINT Operation);

    #define N3SQLBulkOperations(a,b) (*reinterpret_cast<T3SQLBulkOperations>(getOdbcFunction(ODBC3SQLFunctionId::BulkOperations)))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLMoreResults) (   SQLHSTMT            StatementHandle);

    #define N3SQLMoreResults(a) (*reinterpret_cast<T3SQLMoreResults>(getOdbcFunction(ODBC3SQLFunctionId::MoreResults)))(a)

    /*typedef SQLRETURN  (SQL_API  *T3SQLGetDiagField) (SQLSMALLINT         HandleType,
                                                    SQLHANDLE           Handle,
                                                    SQLSMALLINT         RecNumber,
                                                    SQLSMALLINT         DiagIdentifier,
                                                    SQLPOINTER          DiagInfoPtr,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       StringLengthPtr);

    #define N3SQLGetDiagField(a,b,c,d,e,f,g) (*reinterpret_cast<T3SQLGetDiagField>(getOdbcFunction(ODBC3SQLFunctionId::GetDiagField)))(a,b,c,d,e,f,g)*/

    typedef SQLRETURN  (SQL_API  *T3SQLGetDiagRec) (    SQLSMALLINT         HandleType,
                                                    SQLHANDLE           Handle,
                                                    SQLSMALLINT         RecNumber,
                                                    SQLCHAR *           Sqlstate,
                                                    SQLINTEGER *        NativeErrorPtr,
                                                    SQLCHAR *           MessageText,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       TextLengthPtr);


    #define N3SQLGetDiagRec(a,b,c,d,e,f,g,h) (*reinterpret_cast<T3SQLGetDiagRec>(getOdbcFunction(ODBC3SQLFunctionId::GetDiagRec)))(a,b,c,d,e,f,g,h)

    // Obtaining information about the data source's system tables (catalog functions)
    typedef SQLRETURN  (SQL_API  *T3SQLColumnPrivileges) (SQLHSTMT      StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLCHAR *           ColumnName,
                                                    SQLSMALLINT         NameLength4);

    #define N3SQLColumnPrivileges(a,b,c,d,e,f,g,h,i) (*reinterpret_cast<T3SQLColumnPrivileges>(getOdbcFunction(ODBC3SQLFunctionId::ColumnPrivileges)))(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLColumns) (       SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLCHAR *           ColumnName,
                                                    SQLSMALLINT         NameLength4);

    #define N3SQLColumns(a,b,c,d,e,f,g,h,i) (*reinterpret_cast<T3SQLColumns>(getOdbcFunction(ODBC3SQLFunctionId::Columns)))(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLForeignKeys) (   SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           PKCatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           PKSchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           PKTableName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLCHAR *           FKCatalogName,
                                                    SQLSMALLINT         NameLength4,
                                                    SQLCHAR *           FKSchemaName,
                                                    SQLSMALLINT         NameLength5,
                                                    SQLCHAR *           FKTableName,
                                                    SQLSMALLINT         NameLength6);

    #define N3SQLForeignKeys(a,b,c,d,e,f,g,h,i,j,k,l,m) (*reinterpret_cast<T3SQLForeignKeys>(getOdbcFunction(ODBC3SQLFunctionId::ForeignKeys)))(a,b,c,d,e,f,g,h,i,j,k,l,m)

    typedef SQLRETURN  (SQL_API  *T3SQLPrimaryKeys) (   SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3);

    #define N3SQLPrimaryKeys(a,b,c,d,e,f,g) (*reinterpret_cast<T3SQLPrimaryKeys>(getOdbcFunction(ODBC3SQLFunctionId::PrimaryKeys)))(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLProcedureColumns) (SQLHSTMT      StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           ProcName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLCHAR *           ColumnName,
                                                    SQLSMALLINT         NameLength4);

    #define N3SQLProcedureColumns(a,b,c,d,e,f,g,h,i) (*reinterpret_cast<T3SQLProcedureColumns>(getOdbcFunction(ODBC3SQLFunctionId::ProcedureColumns)))(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLProcedures) (    SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           ProcName,
                                                    SQLSMALLINT         NameLength3);

    #define N3SQLProcedures(a,b,c,d,e,f,g) (*reinterpret_cast<T3SQLProcedures>(getOdbcFunction(ODBC3SQLFunctionId::Procedures)))(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLSpecialColumns) (SQLHSTMT            StatementHandle,
                                                    SQLUSMALLINT        IdentifierType,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLUSMALLINT        Scope,
                                                    SQLUSMALLINT        Nullable);

    #define N3SQLSpecialColumns(a,b,c,d,e,f,g,h,i,j) (*reinterpret_cast<T3SQLSpecialColumns>(getOdbcFunction(ODBC3SQLFunctionId::SpecialColumns)))(a,b,c,d,e,f,g,h,i,j)

    typedef SQLRETURN  (SQL_API  *T3SQLStatistics) (    SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLUSMALLINT        Unique,
                                                    SQLUSMALLINT        Reserved);

    #define N3SQLStatistics(a,b,c,d,e,f,g,h,i) (*reinterpret_cast<T3SQLStatistics>(getOdbcFunction(ODBC3SQLFunctionId::Statistics)))(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLTablePrivileges) (SQLHSTMT           StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3);

    #define N3SQLTablePrivileges(a,b,c,d,e,f,g) (*reinterpret_cast<T3SQLTablePrivileges>(getOdbcFunction(ODBC3SQLFunctionId::TablePrivileges)))(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLTables) (    SQLHSTMT                StatementHandle,
                                                    SQLCHAR *               CatalogName,
                                                    SQLSMALLINT             NameLength1,
                                                    SQLCHAR *               SchemaName,
                                                    SQLSMALLINT             NameLength2,
                                                    SQLCHAR *               TableName,
                                                    SQLSMALLINT             NameLength3,
                                                    SQLCHAR *               TableType,
                                                    SQLSMALLINT             NameLength4);

    #define N3SQLTables(a,b,c,d,e,f,g,h,i) (*reinterpret_cast<T3SQLTables>(getOdbcFunction(ODBC3SQLFunctionId::Tables)))(a,b,c,d,e,f,g,h,i)

    // Terminating a statement
    typedef SQLRETURN  (SQL_API  *T3SQLFreeStmt) (  SQLHSTMT                StatementHandle,
                                                    SQLUSMALLINT            Option);

    #define N3SQLFreeStmt(a,b) (*reinterpret_cast<T3SQLFreeStmt>(getOdbcFunction(ODBC3SQLFunctionId::FreeStmt)))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLCloseCursor) (SQLHSTMT           StatementHandle);

    #define N3SQLCloseCursor(a) (*reinterpret_cast<T3SQLCloseCursor>(getOdbcFunction(ODBC3SQLFunctionId::CloseCursor)))(a)

    typedef SQLRETURN  (SQL_API  *T3SQLCancel) (    SQLHSTMT                StatementHandle);

    #define N3SQLCancel(a) (*reinterpret_cast<T3SQLCancel>(getOdbcFunction(ODBC3SQLFunctionId::Cancel)))(a)

    typedef SQLRETURN  (SQL_API  *T3SQLEndTran) (   SQLSMALLINT             HandleType,
                                                    SQLHANDLE               Handle,
                                                    SQLSMALLINT             CompletionType);

    #define N3SQLEndTran(a,b,c) (*reinterpret_cast<T3SQLEndTran>(getOdbcFunction(ODBC3SQLFunctionId::EndTran)))(a,b,c)

    // Terminating a connection
    typedef SQLRETURN  (SQL_API  *T3SQLDisconnect) (SQLHDBC ConnectionHandle);

    #define N3SQLDisconnect(a) (*reinterpret_cast<T3SQLDisconnect>(getOdbcFunction(ODBC3SQLFunctionId::Disconnect)))(a)

    typedef SQLRETURN  (SQL_API  *T3SQLFreeHandle) (SQLSMALLINT             HandleType,
                                                    SQLHANDLE               Handle);

    #define N3SQLFreeHandle(a,b) (*reinterpret_cast<T3SQLFreeHandle>(getOdbcFunction(ODBC3SQLFunctionId::FreeHandle)))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLGetCursorName) ( SQLHSTMT            StatementHandle,
                                                        SQLCHAR *           CursorName,
                                                        SQLSMALLINT         BufferLength,
                                                        SQLSMALLINT*        NameLength2);

    #define N3SQLGetCursorName(a,b,c,d) (*reinterpret_cast<T3SQLGetCursorName>(getOdbcFunction(ODBC3SQLFunctionId::GetCursorName)))(a,b,c,d)

    typedef SQLRETURN  (SQL_API  *T3SQLNativeSql) ( SQLHSTMT                ConnectionHandle,
                                                    SQLCHAR *               InStatementText,
                                                    SQLINTEGER              TextLength1,
                                                    SQLCHAR *               OutStatementText,
                                                    SQLINTEGER              BufferLength,
                                                    SQLINTEGER *            TextLength2Ptr);

    #define N3SQLNativeSql(a,b,c,d,e,f) (*reinterpret_cast<T3SQLNativeSql>(getOdbcFunction(ODBC3SQLFunctionId::NativeSql)))(a,b,c,d,e,f)

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
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
