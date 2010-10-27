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

#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#define _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_

#include "odbc/OFunctiondefs.hxx"
#include <rtl/ustring.hxx>
#include <osl/module.h>

namespace connectivity
{

//  sal_Bool LoadFunctions(oslModule pODBCso, sal_Bool _bDS=sal_True);
//  sal_Bool LoadLibrary_ODBC3(::rtl::OUString &_rPath);
//  sal_Bool LoadLibrary_ADABAS(::rtl::OUString &_rPath);

    // Connecting to a data source
    typedef SQLRETURN  (SQL_API  *T3SQLAllocHandle) (SQLSMALLINT HandleType,SQLHANDLE InputHandle,SQLHANDLE *   OutputHandlePtr);

    #define N3SQLAllocHandle(a,b,c) (*(T3SQLAllocHandle)getOdbcFunction(ODBC3SQLAllocHandle))(a,b,c)

    typedef SQLRETURN  (SQL_API  *T3SQLConnect) (SQLHDBC ConnectionHandle,SQLCHAR *ServerName,SQLSMALLINT   NameLength1,SQLCHAR *UserName,SQLSMALLINT NameLength2,SQLCHAR *Authentication,SQLSMALLINT NameLength3);

    #define N3SQLConnect(a,b,c,d,e,f,g) (*(T3SQLConnect)getOdbcFunction(ODBC3SQLConnect))(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLDriverConnect) ( SQLHDBC         ConnectionHandle,
                                                        HWND            WindowHandle,
                                                        SQLCHAR *       InConnectionString,
                                                        SQLSMALLINT     StringLength1,
                                                        SQLCHAR *       OutConnectionString,
                                                        SQLSMALLINT     BufferLength,
                                                        SQLSMALLINT *   StringLength2Ptr,
                                                        SQLUSMALLINT    DriverCompletion);

    #define N3SQLDriverConnect(a,b,c,d,e,f,g,h) (*(T3SQLDriverConnect)getOdbcFunction(ODBC3SQLDriverConnect))(a,b,c,d,e,f,g,h)

    typedef SQLRETURN  (SQL_API  *T3SQLBrowseConnect) ( SQLHDBC         ConnectionHandle,
                                                        SQLCHAR *       InConnectionString,
                                                        SQLSMALLINT     StringLength1,
                                                        SQLCHAR *       OutConnectionString,
                                                        SQLSMALLINT     BufferLength,
                                                        SQLSMALLINT *   StringLength2Ptr);

    #define N3SQLBrowseConnect(a,b,c,d,e,f) (*(T3SQLBrowseConnect)getOdbcFunction(ODBC3SQLBrowseConnect))(a,b,c,d,e,f)

    // Obtaining information about a driver and data source
    typedef SQLRETURN  (SQL_API  *T3SQLDataSources) (   SQLHENV         EnvironmentHandle,
                                                        SQLUSMALLINT    Direction,
                                                        SQLCHAR *       ServerName,
                                                        SQLSMALLINT     BufferLength1,
                                                        SQLSMALLINT *   NameLength1Ptr,
                                                        SQLCHAR *       Description,
                                                        SQLSMALLINT     BufferLength2,
                                                        SQLSMALLINT *   NameLength2Ptr);

    #define N3SQLDataSources(a,b,c,d,e,f,g,h) (*(T3SQLDataSources)getOdbcFunction(ODBC3SQLDataSources))(a,b,c,d,e,f,g,h)

    typedef SQLRETURN  (SQL_API  *T3SQLDrivers) (   SQLHENV         EnvironmentHandle,
                                                    SQLUSMALLINT    Direction,
                                                    SQLCHAR *       DriverDescription,
                                                    SQLSMALLINT     BufferLength1,
                                                    SQLSMALLINT *   DescriptionLengthPtr,
                                                    SQLCHAR *       DriverAttributes,
                                                    SQLSMALLINT     BufferLength2,
                                                    SQLSMALLINT *   AttributesLengthPtr);

    #define N3SQLDrivers(a,b,c,d,e,f,g,h) (*(T3SQLDrivers)getOdbcFunction(ODBC3SQLDrivers))(a,b,c,d,e,f,g,h)

    typedef SQLRETURN  (SQL_API  *T3SQLGetInfo) (   SQLHDBC         ConnectionHandle,
                                                    SQLUSMALLINT    InfoType,
                                                    SQLPOINTER      InfoValuePtr,
                                                    SQLSMALLINT     BufferLength,
                                                    SQLSMALLINT *   StringLengthPtr);

    #define N3SQLGetInfo(a,b,c,d,e) (*(T3SQLGetInfo)getOdbcFunction(ODBC3SQLGetInfo))(a,b,c,d,e)

    typedef SQLRETURN  (SQL_API  *T3SQLGetFunctions) (SQLHDBC           ConnectionHandle,
                                                    SQLUSMALLINT    FunctionId,
                                                    SQLUSMALLINT *  SupportedPtr);

    #define N3SQLGetFunctions(a,b,c) (*(T3SQLGetFunctions)getOdbcFunction(ODBC3SQLGetFunctions))(a,b,c)

    typedef SQLRETURN  (SQL_API  *T3SQLGetTypeInfo) (   SQLHSTMT    StatementHandle,
                                                    SQLSMALLINT     DataType);

    #define N3SQLGetTypeInfo(a,b) (*(T3SQLGetTypeInfo)getOdbcFunction(ODBC3SQLGetTypeInfo))(a,b)

    // Setting and retrieving driver attributes
    typedef SQLRETURN (SQL_API *T3SQLSetConnectAttr)(SQLHDBC        ConnectionHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      StringLength);

    #define N3SQLSetConnectAttr(a,b,c,d) (*(T3SQLSetConnectAttr)getOdbcFunction(ODBC3SQLSetConnectAttr))(a,b,c,d)

    typedef SQLRETURN (SQL_API *T3SQLGetConnectAttr) (SQLHDBC       ConnectionHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER*     StringLength);

    #define N3SQLGetConnectAttr(a,b,c,d,e) (*(T3SQLGetConnectAttr)getOdbcFunction(ODBC3SQLGetConnectAttr))(a,b,c,d,e)


    typedef SQLRETURN (SQL_API *T3SQLSetEnvAttr) (  SQLHENV         EnvironmentHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      StringLength);

    #define N3SQLSetEnvAttr(a,b,c,d) (*(T3SQLSetEnvAttr)getOdbcFunction(ODBC3SQLSetEnvAttr))(a,b,c,d)

    typedef SQLRETURN (SQL_API *T3SQLGetEnvAttr) (  SQLHENV         EnvironmentHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER*     StringLength);

    #define N3SQLGetEnvAttr(a,b,c,d,e) (*(T3SQLGetEnvAttr)getOdbcFunction(ODBC3SQLGetEnvAttr))(a,b,c,d,e)


    typedef SQLRETURN (SQL_API *T3SQLSetStmtAttr) ( SQLHSTMT        StatementHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      StringLength);

    #define N3SQLSetStmtAttr(a,b,c,d) (*(T3SQLSetStmtAttr)getOdbcFunction(ODBC3SQLSetStmtAttr))(a,b,c,d)

    typedef SQLRETURN (SQL_API *T3SQLGetStmtAttr) ( SQLHSTMT        StatementHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER*     StringLength);

    #define N3SQLGetStmtAttr(a,b,c,d,e) (*(T3SQLGetStmtAttr)getOdbcFunction(ODBC3SQLGetStmtAttr))(a,b,c,d,e)

    // Setting and retrieving descriptor fields
    /*typedef SQLRETURN (SQL_API *T3SQLSetDescField) (SQLHDESC      DescriptorHandle,
                                                    SQLSMALLINT     RecNumber,
                                                    SQLSMALLINT     FieldIdentifier,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength);

    #define N3SQLSetDescField(a,b,c,d,e) (*(T3SQLSetDescField)getOdbcFunction(ODBC3SQLSetDescField))(a,b,c,d,e)

    typedef SQLRETURN (SQL_API *T3SQLGetDescField) (    SQLHDESC        DescriptorHandle,
                                                    SQLSMALLINT     RecNumber,
                                                    SQLSMALLINT     FieldIdentifier,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER *    StringLengthPtr);

    #define N3SQLGetDescField(a,b,c,d,e,f) (*(T3SQLGetDescField)getOdbcFunction(ODBC3SQLGetDescField))(a,b,c,d,e,f)


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

    #define N3SQLGetDescRec(a,b,c,d,e,f,g,h,i,j,k) (*(T3SQLGetDescRec)getOdbcFunction(ODBC3SQLGetDescRec))(a,b,c,d,e,f,g,h,i,j,k)


    typedef SQLRETURN (SQL_API *T3SQLSetDescRec) (  SQLHDESC            DescriptorHandle,
                                                    SQLSMALLINT         RecNumber,
                                                    SQLSMALLINT         Type,
                                                    SQLSMALLINT         SubType,
                                                    SQLINTEGER          Length,
                                                    SQLSMALLINT         Precision,
                                                    SQLSMALLINT         Scale,
                                                    SQLPOINTER          DataPtr,
                                                    SQLLEN *            StringLengthPtr,
                                                    SQLLEN *            IndicatorPtr);

    #define N3SQLSetDescRec(a,b,c,d,e,f,g,h,i,j) (*(T3SQLSetDescRec)getOdbcFunction(ODBC3SQLSetDescRec))(a,b,c,d,e,f,g,h,i,j)
    */

    // Preparing SQL requests
    typedef SQLRETURN  (SQL_API  *T3SQLPrepare) (       SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           StatementText,
                                                    SQLINTEGER          TextLength);

    #define N3SQLPrepare(a,b,c) (*(T3SQLPrepare)getOdbcFunction(ODBC3SQLPrepare))(a,b,c)

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

    #define N3SQLBindParameter(a,b,c,d,e,f,g,h,i,j) (*(T3SQLBindParameter)getOdbcFunction(ODBC3SQLBindParameter))(a,b,c,d,e,f,g,h,i,j)

    /*typedef SQLRETURN (SQL_API  *T3SQLGetCursorName) (SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CursorName,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       NameLengthPtr);

    #define N3SQLGetCursorName(a,b,c,d) (*(T3SQLGetCursorName)getOdbcFunction(ODBC3SQLGetCursorName))(a,b,c,d)
    */

    typedef SQLRETURN (SQL_API  *T3SQLSetCursorName) (SQLHSTMT          StatementHandle,
                                                    SQLCHAR *           CursorName,
                                                    SQLSMALLINT         NameLength);

    #define N3SQLSetCursorName(a,b,c) (*(T3SQLSetCursorName)getOdbcFunction(ODBC3SQLSetCursorName))(a,b,c)

    // Submitting requests
    typedef SQLRETURN  (SQL_API  *T3SQLExecute) (       SQLHSTMT            StatementHandle);

    #define N3SQLExecute(a) (*(T3SQLExecute)getOdbcFunction(ODBC3SQLExecute))(a)

    typedef SQLRETURN  (SQL_API  *T3SQLExecDirect) (    SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           StatementText,
                                                    SQLINTEGER          TextLength);

    #define N3SQLExecDirect(a,b,c) (*(T3SQLExecDirect)getOdbcFunction(ODBC3SQLExecDirect))(a,b,c)

    /*typedef SQLRETURN  (SQL_API  *T3SQLNativeSql) (   SQLHDBC             ConnectionHandle,
                                                    SQLCHAR *           InStatementText,
                                                    SQLINTEGER          TextLength1,
                                                    SQLCHAR *           utStatementText,
                                                    SQLINTEGER          BufferLength,
                                                    SQLINTEGER *        TextLength2Ptr);

    #define N3SQLNativeSql(a,b,c,d,e,f) (*(T3SQLNativeSql)getOdbcFunction(ODBC3SQLNativeSql))(a,b,c,d,e,f)*/

    typedef SQLRETURN (SQL_API  *T3SQLDescribeParam) (SQLHSTMT          StatementHandle,
                                                    SQLUSMALLINT        ParameterNumber,
                                                    SQLSMALLINT *       DataTypePtr,
                                                    SQLULEN *           ParameterSizePtr,
                                                    SQLSMALLINT *       DecimalDigitsPtr,
                                                    SQLSMALLINT *       NullablePtr);

    #define N3SQLDescribeParam(a,b,c,d,e,f) (*(T3SQLDescribeParam)getOdbcFunction(ODBC3SQLDescribeParam))(a,b,c,d,e,f)

    typedef SQLRETURN  (SQL_API  *T3SQLNumParams) ( SQLHSTMT            StatementHandle,
                                                    SQLSMALLINT *       ParameterCountPtr);

    #define N3SQLNumParams(a,b) (*(T3SQLNumParams)getOdbcFunction(ODBC3SQLNumParams))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLParamData) ( SQLHSTMT            StatementHandle,
                                                    SQLPOINTER *        ValuePtrPtr);

    #define N3SQLParamData(a,b) (*(T3SQLParamData)getOdbcFunction(ODBC3SQLParamData))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLPutData) (       SQLHSTMT            StatementHandle,
                                                    SQLPOINTER          DataPtr,
                                                    SQLLEN              StrLen_or_Ind);

    #define N3SQLPutData(a,b,c) (*(T3SQLPutData)getOdbcFunction(ODBC3SQLPutData))(a,b,c)

    // Retrieving results and information about results
    typedef SQLRETURN  (SQL_API  *T3SQLRowCount) (  SQLHSTMT            StatementHandle,
                                                    SQLLEN *            RowCountPtr);

    #define N3SQLRowCount(a,b) (*(T3SQLRowCount)getOdbcFunction(ODBC3SQLRowCount))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLNumResultCols) (SQLHSTMT         StatementHandle,
                                                    SQLSMALLINT *       ColumnCountPtr);

    #define N3SQLNumResultCols(a,b) (*(T3SQLNumResultCols)getOdbcFunction(ODBC3SQLNumResultCols))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLDescribeCol) (   SQLHSTMT            StatementHandle,
                                                    SQLUSMALLINT        ColumnNumber,
                                                    SQLCHAR *           ColumnName,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       NameLengthPtr,
                                                    SQLSMALLINT *       DataTypePtr,
                                                    SQLULEN *           ColumnSizePtr,
                                                    SQLSMALLINT *       DecimalDigitsPtr,
                                                    SQLSMALLINT *       NullablePtr);

    #define N3SQLDescribeCol(a,b,c,d,e,f,g,h,i) (*(T3SQLDescribeCol)getOdbcFunction(ODBC3SQLDescribeCol))(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLColAttribute) (SQLHSTMT          StatementHandle,
                                                    SQLUSMALLINT        ColumnNumber,
                                                    SQLUSMALLINT        FieldIdentifier,
                                                    SQLPOINTER          CharacterAttributePtr,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       StringLengthPtr,
                                                    SQLLEN *            NumericAttributePtr);

    #define N3SQLColAttribute(a,b,c,d,e,f,g) (*(T3SQLColAttribute)getOdbcFunction(ODBC3SQLColAttribute))(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLBindCol) (       SQLHSTMT            StatementHandle,
                                                    SQLUSMALLINT        ColumnNumber,
                                                    SQLSMALLINT         TargetType,
                                                    SQLPOINTER          TargetValuePtr,
                                                    SQLLEN              BufferLength,
                                                    SQLLEN *            StrLen_or_IndPtr);

    #define N3SQLBindCol(a,b,c,d,e,f) (*(T3SQLBindCol)getOdbcFunction(ODBC3SQLBindCol))(a,b,c,d,e,f)

    typedef SQLRETURN  (SQL_API  *T3SQLFetch) (     SQLHSTMT            StatementHandle);

    #define N3SQLFetch(a) (*(T3SQLFetch)getOdbcFunction(ODBC3SQLFetch))(a)

    typedef SQLRETURN  (SQL_API  *T3SQLFetchScroll) (   SQLHSTMT            StatementHandle,
                                                    SQLSMALLINT         FetchOrientation,
                                                    SQLLEN              FetchOffset);

    #define N3SQLFetchScroll(a,b,c) (*(T3SQLFetchScroll)getOdbcFunction(ODBC3SQLFetchScroll))(a,b,c)

    typedef SQLRETURN  (SQL_API  *T3SQLGetData) (       SQLHSTMT            StatementHandle,
                                                    SQLUSMALLINT        ColumnNumber,
                                                    SQLSMALLINT         TargetType,
                                                    SQLPOINTER          TargetValuePtr,
                                                    SQLLEN              BufferLength,
                                                    SQLLEN *            StrLen_or_IndPtr);

    #define N3SQLGetData(a,b,c,d,e,f) (*(T3SQLGetData)getOdbcFunction(ODBC3SQLGetData))(a,b,c,d,e,f)

    typedef SQLRETURN  (SQL_API  *T3SQLSetPos) (        SQLHSTMT            StatementHandle,
                                                    SQLSETPOSIROW       RowNumber,
                                                    SQLUSMALLINT        Operation,
                                                    SQLUSMALLINT        LockType);

    #define N3SQLSetPos(a,b,c,d) (*(T3SQLSetPos)getOdbcFunction(ODBC3SQLSetPos))(a,b,c,d)

    typedef SQLRETURN  (SQL_API  *T3SQLBulkOperations) (    SQLHSTMT        StatementHandle,
                                                        SQLSMALLINT Operation);

    #define N3SQLBulkOperations(a,b) (*(T3SQLBulkOperations)getOdbcFunction(ODBC3SQLBulkOperations))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLMoreResults) (   SQLHSTMT            StatementHandle);

    #define N3SQLMoreResults(a) (*(T3SQLMoreResults)getOdbcFunction(ODBC3SQLMoreResults))(a)

    /*typedef SQLRETURN  (SQL_API  *T3SQLGetDiagField) (SQLSMALLINT         HandleType,
                                                    SQLHANDLE           Handle,
                                                    SQLSMALLINT         RecNumber,
                                                    SQLSMALLINT         DiagIdentifier,
                                                    SQLPOINTER          DiagInfoPtr,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       StringLengthPtr);

    #define N3SQLGetDiagField(a,b,c,d,e,f,g) (*(T3SQLGetDiagField)getOdbcFunction(ODBC3SQLGetDiagField))(a,b,c,d,e,f,g)*/

    typedef SQLRETURN  (SQL_API  *T3SQLGetDiagRec) (    SQLSMALLINT         HandleType,
                                                    SQLHANDLE           Handle,
                                                    SQLSMALLINT         RecNumber,
                                                    SQLCHAR *           Sqlstate,
                                                    SQLINTEGER *        NativeErrorPtr,
                                                    SQLCHAR *           MessageText,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       TextLengthPtr);


    #define N3SQLGetDiagRec(a,b,c,d,e,f,g,h) (*(T3SQLGetDiagRec)getOdbcFunction(ODBC3SQLGetDiagRec))(a,b,c,d,e,f,g,h)

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

    #define N3SQLColumnPrivileges(a,b,c,d,e,f,g,h,i) (*(T3SQLColumnPrivileges)getOdbcFunction(ODBC3SQLColumnPrivileges))(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLColumns) (       SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLCHAR *           ColumnName,
                                                    SQLSMALLINT         NameLength4);

    #define N3SQLColumns(a,b,c,d,e,f,g,h,i) (*(T3SQLColumns)getOdbcFunction(ODBC3SQLColumns))(a,b,c,d,e,f,g,h,i)

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

    #define N3SQLForeignKeys(a,b,c,d,e,f,g,h,i,j,k,l,m) (*(T3SQLForeignKeys)getOdbcFunction(ODBC3SQLForeignKeys))(a,b,c,d,e,f,g,h,i,j,k,l,m)

    typedef SQLRETURN  (SQL_API  *T3SQLPrimaryKeys) (   SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3);

    #define N3SQLPrimaryKeys(a,b,c,d,e,f,g) (*(T3SQLPrimaryKeys)getOdbcFunction(ODBC3SQLPrimaryKeys))(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLProcedureColumns) (SQLHSTMT      StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           ProcName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLCHAR *           ColumnName,
                                                    SQLSMALLINT         NameLength4);

    #define N3SQLProcedureColumns(a,b,c,d,e,f,g,h,i) (*(T3SQLProcedureColumns)getOdbcFunction(ODBC3SQLProcedureColumns))(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLProcedures) (    SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           ProcName,
                                                    SQLSMALLINT         NameLength3);

    #define N3SQLProcedures(a,b,c,d,e,f,g) (*(T3SQLProcedures)getOdbcFunction(ODBC3SQLProcedures))(a,b,c,d,e,f,g)

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

    #define N3SQLSpecialColumns(a,b,c,d,e,f,g,h,i,j) (*(T3SQLSpecialColumns)getOdbcFunction(ODBC3SQLSpecialColumns))(a,b,c,d,e,f,g,h,i,j)

    typedef SQLRETURN  (SQL_API  *T3SQLStatistics) (    SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLUSMALLINT        Unique,
                                                    SQLUSMALLINT        Reserved);

    #define N3SQLStatistics(a,b,c,d,e,f,g,h,i) (*(T3SQLStatistics)getOdbcFunction(ODBC3SQLStatistics))(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLTablePrivileges) (SQLHSTMT           StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3);

    #define N3SQLTablePrivileges(a,b,c,d,e,f,g) (*(T3SQLTablePrivileges)getOdbcFunction(ODBC3SQLTablePrivileges))(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLTables) (    SQLHSTMT                StatementHandle,
                                                    SQLCHAR *               CatalogName,
                                                    SQLSMALLINT             NameLength1,
                                                    SQLCHAR *               SchemaName,
                                                    SQLSMALLINT             NameLength2,
                                                    SQLCHAR *               TableName,
                                                    SQLSMALLINT             NameLength3,
                                                    SQLCHAR *               TableType,
                                                    SQLSMALLINT             NameLength4);

    #define N3SQLTables(a,b,c,d,e,f,g,h,i) (*(T3SQLTables)getOdbcFunction(ODBC3SQLTables))(a,b,c,d,e,f,g,h,i)

    // Terminating a statement
    typedef SQLRETURN  (SQL_API  *T3SQLFreeStmt) (  SQLHSTMT                StatementHandle,
                                                    SQLUSMALLINT            Option);

    #define N3SQLFreeStmt(a,b) (*(T3SQLFreeStmt)getOdbcFunction(ODBC3SQLFreeStmt))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLCloseCursor) (SQLHSTMT           StatementHandle);

    #define N3SQLCloseCursor(a) (*(T3SQLCloseCursor)getOdbcFunction(ODBC3SQLCloseCursor))(a)

    typedef SQLRETURN  (SQL_API  *T3SQLCancel) (    SQLHSTMT                StatementHandle);

    #define N3SQLCancel(a) (*(T3SQLCancel)getOdbcFunction(ODBC3SQLCancel))(a)

    typedef SQLRETURN  (SQL_API  *T3SQLEndTran) (   SQLSMALLINT             HandleType,
                                                    SQLHANDLE               Handle,
                                                    SQLSMALLINT             CompletionType);

    #define N3SQLEndTran(a,b,c) (*(T3SQLEndTran)getOdbcFunction(ODBC3SQLEndTran))(a,b,c)

    // Terminating a connection
    typedef SQLRETURN  (SQL_API  *T3SQLDisconnect) (SQLHDBC ConnectionHandle);

    #define N3SQLDisconnect(a) (*(T3SQLDisconnect)getOdbcFunction(ODBC3SQLDisconnect))(a)

    typedef SQLRETURN  (SQL_API  *T3SQLFreeHandle) (SQLSMALLINT             HandleType,
                                                    SQLHANDLE               Handle);

    #define N3SQLFreeHandle(a,b) (*(T3SQLFreeHandle)getOdbcFunction(ODBC3SQLFreeHandle))(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLGetCursorName) ( SQLHSTMT            StatementHandle,
                                                        SQLCHAR *           CursorName,
                                                        SQLSMALLINT         BufferLength,
                                                        SQLSMALLINT*        NameLength2);

    #define N3SQLGetCursorName(a,b,c,d) (*(T3SQLGetCursorName)getOdbcFunction(ODBC3SQLGetCursorName))(a,b,c,d)

    typedef SQLRETURN  (SQL_API  *T3SQLNativeSql) ( SQLHSTMT                ConnectionHandle,
                                                    SQLCHAR *               InStatementText,
                                                    SQLINTEGER              TextLength1,
                                                    SQLCHAR *               OutStatementText,
                                                    SQLINTEGER              BufferLength,
                                                    SQLINTEGER *            TextLength2Ptr);

    #define N3SQLNativeSql(a,b,c,d,e,f) (*(T3SQLNativeSql)getOdbcFunction(ODBC3SQLNativeSql))(a,b,c,d,e,f)
}

#endif // _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
