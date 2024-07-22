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

#include <sal/config.h>

#include <connectivity/odbc.hxx>
#include <odbc/OTools.hxx>

namespace connectivity::odbc
{
// All ODBC wrapper method signatures must match respective SQL* ODBC API functions exactly
class Functions
{
public:
    virtual bool has(ODBC3SQLFunctionId id) const = 0;

    virtual SQLRETURN AllocHandle(SQLSMALLINT HandleType,
                                  SQLHANDLE InputHandle,
                                  SQLHANDLE* OutputHandlePtr) const = 0;
    virtual SQLRETURN DriverConnect(SQLHDBC ConnectionHandle,
                                    HWND WindowHandle,
                                    SQLCHAR* InConnectionString,
                                    SQLSMALLINT StringLength1,
                                    SQLCHAR* OutConnectionString,
                                    SQLSMALLINT BufferLength,
                                    SQLSMALLINT* StringLength2Ptr,
                                    SQLUSMALLINT DriverCompletion) const = 0;
    // Obtaining information about a driver and data source
    virtual SQLRETURN GetInfo(SQLHDBC ConnectionHandle,
                              SQLUSMALLINT InfoType,
                              SQLPOINTER InfoValuePtr,
                              SQLSMALLINT BufferLength,
                              SQLSMALLINT* StringLengthPtr) const = 0;
    virtual SQLRETURN GetFunctions(SQLHDBC ConnectionHandle,
                                   SQLUSMALLINT FunctionId,
                                   SQLUSMALLINT* SupportedPtr) const = 0;
    virtual SQLRETURN GetTypeInfo(SQLHSTMT StatementHandle, SQLSMALLINT DataType) const = 0;
    // Setting and retrieving driver attributes
    virtual SQLRETURN SetConnectAttr(SQLHDBC ConnectionHandle,
                                     SQLINTEGER Attribute,
                                     SQLPOINTER ValuePtr,
                                     SQLINTEGER StringLength) const = 0;
    virtual SQLRETURN GetConnectAttr(SQLHDBC ConnectionHandle,
                                     SQLINTEGER Attribute,
                                     SQLPOINTER ValuePtr,
                                     SQLINTEGER BufferLength,
                                     SQLINTEGER* StringLength) const = 0;
    virtual SQLRETURN SetEnvAttr(SQLHENV EnvironmentHandle,
                                 SQLINTEGER Attribute,
                                 SQLPOINTER ValuePtr,
                                 SQLINTEGER StringLength) const = 0;
    virtual SQLRETURN GetEnvAttr(SQLHENV EnvironmentHandle,
                                 SQLINTEGER Attribute,
                                 SQLPOINTER ValuePtr,
                                 SQLINTEGER BufferLength,
                                 SQLINTEGER* StringLength) const = 0;
    virtual SQLRETURN SetStmtAttr(SQLHSTMT StatementHandle,
                                  SQLINTEGER Attribute,
                                  SQLPOINTER ValuePtr,
                                  SQLINTEGER StringLength) const = 0;
    virtual SQLRETURN GetStmtAttr(SQLHSTMT StatementHandle,
                                  SQLINTEGER Attribute,
                                  SQLPOINTER ValuePtr,
                                  SQLINTEGER BufferLength,
                                  SQLINTEGER* StringLength) const = 0;
    // Setting and retrieving descriptor fields
    /*
    virtual SQLRETURN SetDescField(SQLHDESC DescriptorHandle,
                                   SQLSMALLINT RecNumber,
                                   SQLSMALLINT FieldIdentifier,
                                   SQLPOINTER ValuePtr,
                                   SQLINTEGER BufferLength) const = 0;
    virtual SQLRETURN GetDescField(SQLHDESC DescriptorHandle,
                                   SQLSMALLINT RecNumber,
                                   SQLSMALLINT FieldIdentifier,
                                   SQLPOINTER ValuePtr,
                                   SQLINTEGER BufferLength,
                                   SQLINTEGER* StringLengthPtr) const = 0;
    virtual SQLRETURN GetDescRec(SQLHDESC DescriptorHandle,
                                 SQLSMALLINT RecNumber,
                                 SQLCHAR* Name,
                                 SQLSMALLINT BufferLength,
                                 SQLSMALLINT* StringLengthPtr,
                                 SQLSMALLINT* TypePtr,
                                 SQLSMALLINT* SubTypePtr,
                                 SQLLEN* LengthPtr,
                                 SQLSMALLINT* PrecisionPtr,
                                 SQLSMALLINT* ScalePtr,
                                 SQLSMALLINT* NullablePtr) const = 0;
    virtual SQLRETURN SetDescRec(SQLHDESC DescriptorHandle,
                                 SQLSMALLINT RecNumber,
                                 SQLSMALLINT Type,
                                 SQLSMALLINT SubType,
                                 SQLLEN Length,
                                 SQLSMALLINT Precision,
                                 SQLSMALLINT Scale,
                                 SQLPOINTER DataPtr,
                                 SQLLEN* StringLengthPtr,
                                 SQLLEN* IndicatorPtr) const = 0;
    */
    // Preparing SQL requests
    virtual SQLRETURN Prepare(SQLHSTMT StatementHandle,
                              SQLCHAR* StatementText,
                              SQLINTEGER TextLength) const = 0;
    virtual SQLRETURN BindParameter(SQLHSTMT StatementHandle,
                                    SQLUSMALLINT ParameterNumber,
                                    SQLSMALLINT InputOutputType,
                                    SQLSMALLINT ValueType,
                                    SQLSMALLINT ParameterType,
                                    SQLULEN ColumnSize,
                                    SQLSMALLINT DecimalDigits,
                                    SQLPOINTER ParameterValuePtr,
                                    SQLLEN BufferLength,
                                    SQLLEN* StrLen_or_IndPtr) const = 0;
    /*
    virtual SQLRETURN GetCursorName(SQLHSTMT StatementHandle,
                                    SQLCHAR* CursorName,
                                    SQLSMALLINT BufferLength,
                                    SQLSMALLINT* NameLengthPtr) const = 0;
    */
    virtual SQLRETURN SetCursorName(SQLHSTMT StatementHandle,
                                    SQLCHAR* CursorName,
                                    SQLSMALLINT NameLength) const = 0;
    // Submitting requests
    virtual SQLRETURN Execute(SQLHSTMT StatementHandle) const = 0;
    virtual SQLRETURN ExecDirect(SQLHSTMT StatementHandle,
                                 SQLCHAR* StatementText,
                                 SQLINTEGER TextLength) const = 0;
    virtual SQLRETURN DescribeParam(SQLHSTMT StatementHandle,
                                    SQLUSMALLINT ParameterNumber,
                                    SQLSMALLINT* DataTypePtr,
                                    SQLULEN* ParameterSizePtr,
                                    SQLSMALLINT* DecimalDigitsPtr,
                                    SQLSMALLINT* NullablePtr) const = 0;
    virtual SQLRETURN NumParams(SQLHSTMT StatementHandle, SQLSMALLINT* ParameterCountPtr) const = 0;
    virtual SQLRETURN ParamData(SQLHSTMT StatementHandle, SQLPOINTER* ValuePtrPtr) const = 0;
    virtual SQLRETURN PutData(SQLHSTMT StatementHandle,
                              SQLPOINTER DataPtr,
                              SQLLEN StrLen_or_Ind) const = 0;
    // Retrieving results and information about results
    virtual SQLRETURN RowCount(SQLHSTMT StatementHandle, SQLLEN* RowCountPtr) const = 0;
    virtual SQLRETURN NumResultCols(SQLHSTMT StatementHandle, SQLSMALLINT* ColumnCountPtr) const = 0;
    virtual SQLRETURN ColAttribute(SQLHSTMT StatementHandle,
                                   SQLUSMALLINT ColumnNumber,
                                   SQLUSMALLINT FieldIdentifier,
                                   SQLPOINTER CharacterAttributePtr,
                                   SQLSMALLINT BufferLength,
                                   SQLSMALLINT* StringLengthPtr,
                                   SQLLEN* NumericAttributePtr) const = 0;
    virtual SQLRETURN BindCol(SQLHSTMT StatementHandle,
                              SQLUSMALLINT ColumnNumber,
                              SQLSMALLINT TargetType,
                              SQLPOINTER TargetValuePtr,
                              SQLLEN BufferLength,
                              SQLLEN* StrLen_or_IndPtr) const = 0;
    virtual SQLRETURN Fetch(SQLHSTMT StatementHandle) const = 0;
    virtual SQLRETURN FetchScroll(SQLHSTMT StatementHandle,
                                  SQLSMALLINT FetchOrientation,
                                  SQLLEN FetchOffset) const = 0;
    virtual SQLRETURN GetData(SQLHSTMT StatementHandle,
                              SQLUSMALLINT ColumnNumber,
                              SQLSMALLINT TargetType,
                              SQLPOINTER TargetValuePtr,
                              SQLLEN BufferLength,
                              SQLLEN* StrLen_or_IndPtr) const = 0;
    virtual SQLRETURN SetPos(SQLHSTMT StatementHandle,
                             SQLSETPOSIROW RowNumber,
                             SQLUSMALLINT Operation,
                             SQLUSMALLINT LockType) const = 0;
    virtual SQLRETURN BulkOperations(SQLHSTMT StatementHandle, SQLSMALLINT Operation) const = 0;
    virtual SQLRETURN MoreResults(SQLHSTMT StatementHandle) const = 0;
    /*
    virtual SQLRETURN GetDiagField(SQLSMALLINT HandleType,
                                   SQLHANDLE Handle,
                                   SQLSMALLINT RecNumber,
                                   SQLSMALLINT DiagIdentifier,
                                   SQLPOINTER DiagInfoPtr,
                                   SQLSMALLINT BufferLength,
                                   SQLSMALLINT* StringLengthPtr) const = 0;
    */
    virtual SQLRETURN GetDiagRec(SQLSMALLINT HandleType,
                                 SQLHANDLE Handle,
                                 SQLSMALLINT RecNumber,
                                 SQLCHAR* Sqlstate,
                                 SQLINTEGER* NativeErrorPtr,
                                 SQLCHAR* MessageText,
                                 SQLSMALLINT BufferLength,
                                 SQLSMALLINT* TextLengthPtr) const = 0;
    // Obtaining information about the data source's system tables (catalog functions)
    virtual SQLRETURN ColumnPrivileges(SQLHSTMT StatementHandle,
                                       SQLCHAR* CatalogName,
                                       SQLSMALLINT NameLength1,
                                       SQLCHAR* SchemaName,
                                       SQLSMALLINT NameLength2,
                                       SQLCHAR* TableName,
                                       SQLSMALLINT NameLength3,
                                       SQLCHAR* ColumnName,
                                       SQLSMALLINT NameLength4) const = 0;
    virtual SQLRETURN Columns(SQLHSTMT StatementHandle,
                              SQLCHAR* CatalogName,
                              SQLSMALLINT NameLength1,
                              SQLCHAR* SchemaName,
                              SQLSMALLINT NameLength2,
                              SQLCHAR* TableName,
                              SQLSMALLINT NameLength3,
                              SQLCHAR* ColumnName,
                              SQLSMALLINT NameLength4) const = 0;
    virtual SQLRETURN ForeignKeys(SQLHSTMT StatementHandle,
                                  SQLCHAR* PKCatalogName,
                                  SQLSMALLINT NameLength1,
                                  SQLCHAR* PKSchemaName,
                                  SQLSMALLINT NameLength2,
                                  SQLCHAR* PKTableName,
                                  SQLSMALLINT NameLength3,
                                  SQLCHAR* FKCatalogName,
                                  SQLSMALLINT NameLength4,
                                  SQLCHAR* FKSchemaName,
                                  SQLSMALLINT NameLength5,
                                  SQLCHAR* FKTableName,
                                  SQLSMALLINT NameLength6) const = 0;
    virtual SQLRETURN PrimaryKeys(SQLHSTMT StatementHandle,
                                  SQLCHAR* CatalogName,
                                  SQLSMALLINT NameLength1,
                                  SQLCHAR* SchemaName,
                                  SQLSMALLINT NameLength2,
                                  SQLCHAR* TableName,
                                  SQLSMALLINT NameLength3) const = 0;
    virtual SQLRETURN ProcedureColumns(SQLHSTMT StatementHandle,
                                       SQLCHAR* CatalogName,
                                       SQLSMALLINT NameLength1,
                                       SQLCHAR* SchemaName,
                                       SQLSMALLINT NameLength2,
                                       SQLCHAR* ProcName,
                                       SQLSMALLINT NameLength3,
                                       SQLCHAR* ColumnName,
                                       SQLSMALLINT NameLength4) const = 0;
    virtual SQLRETURN Procedures(SQLHSTMT StatementHandle,
                                 SQLCHAR* CatalogName,
                                 SQLSMALLINT NameLength1,
                                 SQLCHAR* SchemaName,
                                 SQLSMALLINT NameLength2,
                                 SQLCHAR* ProcName,
                                 SQLSMALLINT NameLength3) const = 0;
    virtual SQLRETURN SpecialColumns(SQLHSTMT StatementHandle,
                                     SQLUSMALLINT IdentifierType,
                                     SQLCHAR* CatalogName,
                                     SQLSMALLINT NameLength1,
                                     SQLCHAR* SchemaName,
                                     SQLSMALLINT NameLength2,
                                     SQLCHAR* TableName,
                                     SQLSMALLINT NameLength3,
                                     SQLUSMALLINT Scope,
                                     SQLUSMALLINT Nullable) const = 0;
    virtual SQLRETURN Statistics(SQLHSTMT StatementHandle,
                                 SQLCHAR* CatalogName,
                                 SQLSMALLINT NameLength1,
                                 SQLCHAR* SchemaName,
                                 SQLSMALLINT NameLength2,
                                 SQLCHAR* TableName,
                                 SQLSMALLINT NameLength3,
                                 SQLUSMALLINT Unique,
                                 SQLUSMALLINT Reserved) const = 0;
    virtual SQLRETURN TablePrivileges(SQLHSTMT StatementHandle,
                                      SQLCHAR* CatalogName,
                                      SQLSMALLINT NameLength1,
                                      SQLCHAR* SchemaName,
                                      SQLSMALLINT NameLength2,
                                      SQLCHAR* TableName,
                                      SQLSMALLINT NameLength3) const = 0;
    virtual SQLRETURN Tables(SQLHSTMT StatementHandle,
                             SQLCHAR* CatalogName,
                             SQLSMALLINT NameLength1,
                             SQLCHAR* SchemaName,
                             SQLSMALLINT NameLength2,
                             SQLCHAR* TableName,
                             SQLSMALLINT NameLength3,
                             SQLCHAR* TableType,
                             SQLSMALLINT NameLength4) const = 0;
    // Terminating a statement
    virtual SQLRETURN FreeStmt(SQLHSTMT StatementHandle, SQLUSMALLINT Option) const = 0;
    virtual SQLRETURN CloseCursor(SQLHSTMT StatementHandle) const = 0;
    virtual SQLRETURN Cancel(SQLHSTMT StatementHandle) const = 0;
    virtual SQLRETURN EndTran(SQLSMALLINT HandleType,
                              SQLHANDLE Handle,
                              SQLSMALLINT CompletionType) const = 0;
    // Terminating a connection
    virtual SQLRETURN Disconnect(SQLHDBC ConnectionHandle) const = 0;
    virtual SQLRETURN FreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle) const = 0;
    virtual SQLRETURN GetCursorName(SQLHSTMT StatementHandle,
                                    SQLCHAR* CursorName,
                                    SQLSMALLINT BufferLength,
                                    SQLSMALLINT* NameLength2) const = 0;
    virtual SQLRETURN NativeSql(SQLHDBC ConnectionHandle,
                                SQLCHAR* InStatementText,
                                SQLINTEGER TextLength1,
                                SQLCHAR* OutStatementText,
                                SQLINTEGER BufferLength,
                                SQLINTEGER* TextLength2Ptr) const = 0;
};
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
