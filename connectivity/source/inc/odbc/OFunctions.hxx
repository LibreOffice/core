/*************************************************************************
 *
 *  $RCSfile: OFunctions.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:27 $
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

#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#define _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_

#ifndef _CONNECTIVITY_OFUNCTIONDEFS_HXX_
#include "odbc/OFunctiondefs.hxx"
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif

namespace connectivity
{

    sal_Bool LoadFunctions(oslModule pODBCso, sal_Bool _bDS=sal_True);
    sal_Bool LoadLibrary_ODBC3(::rtl::OUString &_rPath);
    sal_Bool LoadLibrary_ADABAS(::rtl::OUString &_rPath);

    // Connecting to a data source
    typedef SQLRETURN  (SQL_API  *T3SQLAllocHandle) (SQLSMALLINT HandleType,SQLHANDLE InputHandle,SQLHANDLE *   OutputHandlePtr);
    extern T3SQLAllocHandle pODBC3SQLAllocHandle;
    #define N3SQLAllocHandle(a,b,c) (*pODBC3SQLAllocHandle)(a,b,c)

    typedef SQLRETURN  (SQL_API  *T3SQLConnect) (SQLHDBC ConnectionHandle,SQLCHAR *ServerName,SQLSMALLINT   NameLength1,SQLCHAR *UserName,SQLSMALLINT NameLength2,SQLCHAR *Authentication,SQLSMALLINT NameLength3);
    extern T3SQLConnect pODBC3SQLConnect;
    #define N3SQLConnect(a,b,c,d,e,f,g) (*pODBC3SQLConnect)(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLDriverConnect) ( SQLHDBC         ConnectionHandle,
                                                        HWND            WindowHandle,
                                                        SQLCHAR *       InConnectionString,
                                                        SQLSMALLINT     StringLength1,
                                                        SQLCHAR *       OutConnectionString,
                                                        SQLSMALLINT     BufferLength,
                                                        SQLSMALLINT *   StringLength2Ptr,
                                                        SQLUSMALLINT    DriverCompletion);
    extern T3SQLDriverConnect pODBC3SQLDriverConnect;
    #define N3SQLDriverConnect(a,b,c,d,e,f,g,h) (*pODBC3SQLDriverConnect)(a,b,c,d,e,f,g,h)

    typedef SQLRETURN  (SQL_API  *T3SQLBrowseConnect) ( SQLHDBC         ConnectionHandle,
                                                        SQLCHAR *       InConnectionString,
                                                        SQLSMALLINT     StringLength1,
                                                        SQLCHAR *       OutConnectionString,
                                                        SQLSMALLINT     BufferLength,
                                                        SQLSMALLINT *   StringLength2Ptr);
    extern T3SQLBrowseConnect pODBC3SQLBrowseConnect;
    #define N3SQLBrowseConnect(a,b,c,d,e,f) (*pODBC3SQLBrowseConnect)(a,b,c,d,e,f)

    // Obtaining information about a driver and data source
    typedef SQLRETURN  (SQL_API  *T3SQLDataSources) (   SQLHENV         EnvironmentHandle,
                                                        SQLUSMALLINT    Direction,
                                                        SQLCHAR *       ServerName,
                                                        SQLSMALLINT     BufferLength1,
                                                        SQLSMALLINT *   NameLength1Ptr,
                                                        SQLCHAR *       Description,
                                                        SQLSMALLINT     BufferLength2,
                                                        SQLSMALLINT *   NameLength2Ptr);
    extern T3SQLDataSources pODBC3SQLDataSources;
    #define N3SQLDataSources(a,b,c,d,e,f,g,h) (*pODBC3SQLDataSources)(a,b,c,d,e,f,g,h)

    typedef SQLRETURN  (SQL_API  *T3SQLDrivers) (   SQLHENV         EnvironmentHandle,
                                                    SQLUSMALLINT    Direction,
                                                    SQLCHAR *       DriverDescription,
                                                    SQLSMALLINT     BufferLength1,
                                                    SQLSMALLINT *   DescriptionLengthPtr,
                                                    SQLCHAR *       DriverAttributes,
                                                    SQLSMALLINT     BufferLength2,
                                                    SQLSMALLINT *   AttributesLengthPtr);
    extern T3SQLDrivers pODBC3SQLDrivers;
    #define N3SQLDrivers(a,b,c,d,e,f,g,h) (*pODBC3SQLDrivers)(a,b,c,d,e,f,g,h)

    typedef SQLRETURN  (SQL_API  *T3SQLGetInfo) (   SQLHDBC         ConnectionHandle,
                                                    SQLUSMALLINT    InfoType,
                                                    SQLPOINTER      InfoValuePtr,
                                                    SQLSMALLINT     BufferLength,
                                                    SQLSMALLINT *   StringLengthPtr);
    extern T3SQLGetInfo pODBC3SQLGetInfo;
    #define N3SQLGetInfo(a,b,c,d,e) (*pODBC3SQLGetInfo)(a,b,c,d,e)

    typedef SQLRETURN  (SQL_API  *T3SQLGetFunctions) (SQLHDBC           ConnectionHandle,
                                                    SQLUSMALLINT    FunctionId,
                                                    SQLUSMALLINT *  SupportedPtr);
    extern T3SQLGetFunctions pODBC3SQLGetFunctions;
    #define N3SQLGetFunctions(a,b,c) (*pODBC3SQLGetFunctions)(a,b,c)

    typedef SQLRETURN  (SQL_API  *T3SQLGetTypeInfo) (   SQLHSTMT    StatementHandle,
                                                    SQLSMALLINT     DataType);
    extern T3SQLGetTypeInfo pODBC3SQLGetTypeInfo;
    #define N3SQLGetTypeInfo(a,b) (*pODBC3SQLGetTypeInfo)(a,b)

    // Setting and retrieving driver attributes
    typedef SQLRETURN (SQL_API *T3SQLSetConnectAttr)(SQLHDBC        ConnectionHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      StringLength);
    extern T3SQLSetConnectAttr pODBC3SQLSetConnectAttr;
    #define N3SQLSetConnectAttr(a,b,c,d) (*pODBC3SQLSetConnectAttr)(a,b,c,d)

    typedef SQLRETURN (SQL_API *T3SQLGetConnectAttr) (SQLHDBC       ConnectionHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER*     StringLength);
    extern T3SQLGetConnectAttr pODBC3SQLGetConnectAttr;
    #define N3SQLGetConnectAttr(a,b,c,d,e) (*pODBC3SQLGetConnectAttr)(a,b,c,d,e)


    typedef SQLRETURN (SQL_API *T3SQLSetEnvAttr) (  SQLHENV         EnvironmentHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      StringLength);
    extern T3SQLSetEnvAttr pODBC3SQLSetEnvAttr;
    #define N3SQLSetEnvAttr(a,b,c,d) (*pODBC3SQLSetEnvAttr)(a,b,c,d)

    typedef SQLRETURN (SQL_API *T3SQLGetEnvAttr) (  SQLHENV         EnvironmentHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER*     StringLength);
    extern T3SQLGetEnvAttr pODBC3SQLGetEnvAttr;
    #define N3SQLGetEnvAttr(a,b,c,d,e) (*pODBC3SQLGetEnvAttr)(a,b,c,d,e)


    typedef SQLRETURN (SQL_API *T3SQLSetStmtAttr) ( SQLHSTMT        StatementHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      StringLength);
    extern T3SQLSetStmtAttr pODBC3SQLSetStmtAttr;
    #define N3SQLSetStmtAttr(a,b,c,d) (*pODBC3SQLSetStmtAttr)(a,b,c,d)

    typedef SQLRETURN (SQL_API *T3SQLGetStmtAttr) ( SQLHSTMT        StatementHandle,
                                                    SQLINTEGER      Attribute,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER*     StringLength);
    extern T3SQLGetStmtAttr pODBC3SQLGetStmtAttr;
    #define N3SQLGetStmtAttr(a,b,c,d,e) (*pODBC3SQLGetStmtAttr)(a,b,c,d,e)

    // Setting and retrieving descriptor fields
    /*typedef SQLRETURN (SQL_API *T3SQLSetDescField) (SQLHDESC      DescriptorHandle,
                                                    SQLSMALLINT     RecNumber,
                                                    SQLSMALLINT     FieldIdentifier,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength);
    extern T3SQLSetDescField pODBC3SQLSetDescField;
    #define N3SQLSetDescField(a,b,c,d,e) (*pODBC3SQLSetDescField)(a,b,c,d,e)

    typedef SQLRETURN (SQL_API *T3SQLGetDescField) (    SQLHDESC        DescriptorHandle,
                                                    SQLSMALLINT     RecNumber,
                                                    SQLSMALLINT     FieldIdentifier,
                                                    SQLPOINTER      ValuePtr,
                                                    SQLINTEGER      BufferLength,
                                                    SQLINTEGER *    StringLengthPtr);
    extern T3SQLGetDescField pODBC3SQLGetDescField;
    #define N3SQLGetDescField(a,b,c,d,e,f) (*pODBC3SQLGetDescField)(a,b,c,d,e,f)


    typedef SQLRETURN (SQL_API *T3SQLGetDescRec) (  SQLHDESC            DescriptorHandle,
                                                    SQLSMALLINT         RecNumber,
                                                    SQLCHAR *           Name,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       StringLengthPtr,
                                                    SQLSMALLINT *       TypePtr,
                                                    SQLSMALLINT *       SubTypePtr,
                                                    SQLINTEGER *        LengthPtr,
                                                    SQLSMALLINT *       PrecisionPtr,
                                                    SQLSMALLINT *       ScalePtr,
                                                    SQLSMALLINT *       NullablePtr);
    extern T3SQLGetDescRec pODBC3SQLGetDescRec;
    #define N3SQLGetDescRec(a,b,c,d,e,f,g,h,i,j,k) (*pODBC3SQLGetDescRec)(a,b,c,d,e,f,g,h,i,j,k)


    typedef SQLRETURN (SQL_API *T3SQLSetDescRec) (  SQLHDESC            DescriptorHandle,
                                                    SQLSMALLINT         RecNumber,
                                                    SQLSMALLINT         Type,
                                                    SQLSMALLINT         SubType,
                                                    SQLINTEGER          Length,
                                                    SQLSMALLINT         Precision,
                                                    SQLSMALLINT         Scale,
                                                    SQLPOINTER          DataPtr,
                                                    SQLINTEGER *        StringLengthPtr,
                                                    SQLINTEGER *        IndicatorPtr);
    extern T3SQLSetDescRec pODBC3SQLSetDescRec;
    #define N3SQLSetDescRec(a,b,c,d,e,f,g,h,i,j) (*pODBC3SQLSetDescRec)(a,b,c,d,e,f,g,h,i,j)
    */

    // Preparing SQL requests
    typedef SQLRETURN  (SQL_API  *T3SQLPrepare) (       SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           StatementText,
                                                    SQLINTEGER          TextLength);
    extern T3SQLPrepare pODBC3SQLPrepare;
    #define N3SQLPrepare(a,b,c) (*pODBC3SQLPrepare)(a,b,c)

    typedef SQLRETURN (SQL_API  *T3SQLBindParameter) (SQLHSTMT          StatementHandle,
                                                    SQLUSMALLINT        ParameterNumber,
                                                    SQLSMALLINT         InputOutputType,
                                                    SQLSMALLINT         ValueType,
                                                    SQLSMALLINT         ParameterType,
                                                    SQLUINTEGER         ColumnSize,
                                                    SQLSMALLINT         DecimalDigits,
                                                    SQLPOINTER          ParameterValuePtr,
                                                    SQLINTEGER          BufferLength,
                                                    SQLINTEGER *        StrLen_or_IndPtr);
    extern T3SQLBindParameter pODBC3SQLBindParameter;
    #define N3SQLBindParameter(a,b,c,d,e,f,g,h,i,j) (*pODBC3SQLBindParameter)(a,b,c,d,e,f,g,h,i,j)

    /*typedef SQLRETURN (SQL_API  *T3SQLGetCursorName) (SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CursorName,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       NameLengthPtr);
    extern T3SQLGetCursorName pODBC3SQLGetCursorName;
    #define N3SQLGetCursorName(a,b,c,d) (*pODBC3SQLGetCursorName)(a,b,c,d)
    */

    typedef SQLRETURN (SQL_API  *T3SQLSetCursorName) (SQLHSTMT          StatementHandle,
                                                    SQLCHAR *           CursorName,
                                                    SQLSMALLINT         NameLength);
    extern T3SQLSetCursorName pODBC3SQLSetCursorName;
    #define N3SQLSetCursorName(a,b,c) (*pODBC3SQLSetCursorName)(a,b,c)

    // Submitting requests
    typedef SQLRETURN  (SQL_API  *T3SQLExecute) (       SQLHSTMT            StatementHandle);
    extern T3SQLExecute pODBC3SQLExecute;
    #define N3SQLExecute(a) (*pODBC3SQLExecute)(a)

    typedef SQLRETURN  (SQL_API  *T3SQLExecDirect) (    SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           StatementText,
                                                    SQLINTEGER          TextLength);
    extern T3SQLExecDirect pODBC3SQLExecDirect;
    #define N3SQLExecDirect(a,b,c) (*pODBC3SQLExecDirect)(a,b,c)

    /*typedef SQLRETURN  (SQL_API  *T3SQLNativeSql) (   SQLHDBC             ConnectionHandle,
                                                    SQLCHAR *           InStatementText,
                                                    SQLINTEGER          TextLength1,
                                                    SQLCHAR *           utStatementText,
                                                    SQLINTEGER          BufferLength,
                                                    SQLINTEGER *        TextLength2Ptr);
    extern T3SQLNativeSql pODBC3SQLNativeSql;
    #define N3SQLNativeSql(a,b,c,d,e,f) (*pODBC3SQLNativeSql)(a,b,c,d,e,f)*/

    typedef SQLRETURN (SQL_API  *T3SQLDescribeParam) (SQLHSTMT          StatementHandle,
                                                    SQLUSMALLINT        ParameterNumber,
                                                    SQLSMALLINT *       DataTypePtr,
                                                    SQLUINTEGER *       ParameterSizePtr,
                                                    SQLSMALLINT *       DecimalDigitsPtr,
                                                    SQLSMALLINT *       NullablePtr);
    extern T3SQLDescribeParam pODBC3SQLDescribeParam;
    #define N3SQLDescribeParam(a,b,c,d,e,f) (*pODBC3SQLDescribeParam)(a,b,c,d,e,f)

    typedef SQLRETURN  (SQL_API  *T3SQLNumParams) ( SQLHSTMT            StatementHandle,
                                                    SQLSMALLINT *       ParameterCountPtr);
    extern T3SQLNumParams pODBC3SQLNumParams;
    #define N3SQLNumParams(a,b) (*pODBC3SQLNumParams)(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLParamData) ( SQLHSTMT            StatementHandle,
                                                    SQLPOINTER *        ValuePtrPtr);
    extern T3SQLParamData pODBC3SQLParamData;
    #define N3SQLParamData(a,b) (*pODBC3SQLParamData)(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLPutData) (       SQLHSTMT            StatementHandle,
                                                    SQLPOINTER          DataPtr,
                                                    SQLINTEGER          StrLen_or_Ind);
    extern T3SQLPutData pODBC3SQLPutData;
    #define N3SQLPutData(a,b,c) (*pODBC3SQLPutData)(a,b,c)

    // Retrieving results and information about results
    typedef SQLRETURN  (SQL_API  *T3SQLRowCount) (  SQLHSTMT            StatementHandle,
                                                    SQLINTEGER *        RowCountPtr);
    extern T3SQLRowCount pODBC3SQLRowCount;
    #define N3SQLRowCount(a,b) (*pODBC3SQLRowCount)(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLNumResultCols) (SQLHSTMT         StatementHandle,
                                                    SQLSMALLINT *       ColumnCountPtr);
    extern T3SQLNumResultCols pODBC3SQLNumResultCols;
    #define N3SQLNumResultCols(a,b) (*pODBC3SQLNumResultCols)(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLDescribeCol) (   SQLHSTMT            StatementHandle,
                                                    SQLSMALLINT         ColumnNumber,
                                                    SQLCHAR *           ColumnName,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       NameLengthPtr,
                                                    SQLSMALLINT *       DataTypePtr,
                                                    SQLUINTEGER *       ColumnSizePtr,
                                                    SQLSMALLINT *       DecimalDigitsPtr,
                                                    SQLSMALLINT *       NullablePtr);
    extern T3SQLDescribeCol pODBC3SQLDescribeCol;
    #define N3SQLDescribeCol(a,b,c,d,e,f,g,h,i) (*pODBC3SQLDescribeCol)(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLColAttribute) (SQLHSTMT          StatementHandle,
                                                    SQLUSMALLINT        ColumnNumber,
                                                    SQLUSMALLINT        FieldIdentifier,
                                                    SQLPOINTER          CharacterAttributePtr,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       StringLengthPtr,
                                                    SQLPOINTER          NumericAttributePtr);
    extern T3SQLColAttribute pODBC3SQLColAttribute;
    #define N3SQLColAttribute(a,b,c,d,e,f,g) (*pODBC3SQLColAttribute)(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLBindCol) (       SQLHSTMT            StatementHandle,
                                                    SQLUSMALLINT        ColumnNumber,
                                                    SQLSMALLINT         TargetType,
                                                    SQLPOINTER          TargetValuePtr,
                                                    SQLINTEGER          BufferLength,
                                                    SQLINTEGER *        StrLen_or_IndPtr);
    extern T3SQLBindCol pODBC3SQLBindCol;
    #define N3SQLBindCol(a,b,c,d,e,f) (*pODBC3SQLBindCol)(a,b,c,d,e,f)

    typedef SQLRETURN  (SQL_API  *T3SQLFetch) (     SQLHSTMT            StatementHandle);
    extern T3SQLFetch pODBC3SQLFetch;
    #define N3SQLFetch(a) (*pODBC3SQLFetch)(a)

    typedef SQLRETURN  (SQL_API  *T3SQLFetchScroll) (   SQLHSTMT            StatementHandle,
                                                    SQLSMALLINT         FetchOrientation,
                                                    SQLINTEGER          FetchOffset);
    extern T3SQLFetchScroll pODBC3SQLFetchScroll;
    #define N3SQLFetchScroll(a,b,c) (*pODBC3SQLFetchScroll)(a,b,c)

    typedef SQLRETURN  (SQL_API  *T3SQLGetData) (       SQLHSTMT            StatementHandle,
                                                    SQLUSMALLINT        ColumnNumber,
                                                    SQLSMALLINT         TargetType,
                                                    SQLPOINTER          TargetValuePtr,
                                                    SQLINTEGER          BufferLength,
                                                    SQLINTEGER *        StrLen_or_IndPtr);
    extern T3SQLGetData pODBC3SQLGetData;
    #define N3SQLGetData(a,b,c,d,e,f) (*pODBC3SQLGetData)(a,b,c,d,e,f)

    typedef SQLRETURN  (SQL_API  *T3SQLSetPos) (        SQLHSTMT            StatementHandle,
                                                    SQLUSMALLINT        RowNumber,
                                                    SQLUSMALLINT        Operation,
                                                    SQLUSMALLINT        LockType);
    extern T3SQLSetPos pODBC3SQLSetPos;
    #define N3SQLSetPos(a,b,c,d) (*pODBC3SQLSetPos)(a,b,c,d)

    typedef SQLRETURN  (SQL_API  *T3SQLBulkOperations) (    SQLHSTMT        StatementHandle,
                                                        SQLUSMALLINT    Operation);
    extern T3SQLBulkOperations pODBC3SQLBulkOperations;
    #define N3SQLBulkOperations(a,b) (*pODBC3SQLBulkOperations)(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLMoreResults) (   SQLHSTMT            StatementHandle);
    extern T3SQLMoreResults pODBC3SQLMoreResults;
    #define N3SQLMoreResults(a) (*pODBC3SQLMoreResults)(a)

    /*typedef SQLRETURN  (SQL_API  *T3SQLGetDiagField) (SQLSMALLINT         HandleType,
                                                    SQLHANDLE           Handle,
                                                    SQLSMALLINT         RecNumber,
                                                    SQLSMALLINT         DiagIdentifier,
                                                    SQLPOINTER          DiagInfoPtr,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       StringLengthPtr);
    extern T3SQLGetDiagField pODBC3SQLGetDiagField;
    #define N3SQLGetDiagField(a,b,c,d,e,f,g) (*pODBC3SQLGetDiagField)(a,b,c,d,e,f,g)*/

    typedef SQLRETURN  (SQL_API  *T3SQLGetDiagRec) (    SQLSMALLINT         HandleType,
                                                    SQLHANDLE           Handle,
                                                    SQLSMALLINT         RecNumber,
                                                    SQLCHAR *           Sqlstate,
                                                    SQLINTEGER *        NativeErrorPtr,
                                                    SQLCHAR *           MessageText,
                                                    SQLSMALLINT         BufferLength,
                                                    SQLSMALLINT *       TextLengthPtr);

    extern T3SQLGetDiagRec pODBC3SQLGetDiagRec;
    #define N3SQLGetDiagRec(a,b,c,d,e,f,g,h) (*pODBC3SQLGetDiagRec)(a,b,c,d,e,f,g,h)

    // Obtaining information about the data source’s system tables (catalog functions)
    typedef SQLRETURN  (SQL_API  *T3SQLColumnPrivileges) (SQLHSTMT      StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLCHAR *           ColumnName,
                                                    SQLSMALLINT         NameLength4);
    extern T3SQLColumnPrivileges pODBC3SQLColumnPrivileges;
    #define N3SQLColumnPrivileges(a,b,c,d,e,f,g,h,i) (*pODBC3SQLColumnPrivileges)(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLColumns) (       SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLCHAR *           ColumnName,
                                                    SQLSMALLINT         NameLength4);
    extern T3SQLColumns pODBC3SQLColumns;
    #define N3SQLColumns(a,b,c,d,e,f,g,h,i) (*pODBC3SQLColumns)(a,b,c,d,e,f,g,h,i)

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
    extern T3SQLForeignKeys pODBC3SQLForeignKeys;
    #define N3SQLForeignKeys(a,b,c,d,e,f,g,h,i,j,k,l,m) (*pODBC3SQLForeignKeys)(a,b,c,d,e,f,g,h,i,j,k,l,m)

    typedef SQLRETURN  (SQL_API  *T3SQLPrimaryKeys) (   SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3);
    extern T3SQLPrimaryKeys pODBC3SQLPrimaryKeys;
    #define N3SQLPrimaryKeys(a,b,c,d,e,f,g) (*pODBC3SQLPrimaryKeys)(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLProcedureColumns) (SQLHSTMT      StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           ProcName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLCHAR *           ColumnName,
                                                    SQLSMALLINT         NameLength4);
    extern T3SQLProcedureColumns pODBC3SQLProcedureColumns;
    #define N3SQLProcedureColumns(a,b,c,d,e,f,g,h,i) (*pODBC3SQLProcedureColumns)(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLProcedures) (    SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           ProcName,
                                                    SQLSMALLINT         NameLength3);
    extern T3SQLProcedures pODBC3SQLProcedures;
    #define N3SQLProcedures(a,b,c,d,e,f,g) (*pODBC3SQLProcedures)(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLSpecialColumns) (SQLHSTMT            StatementHandle,
                                                    SQLSMALLINT         IdentifierType,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLSMALLINT         Scope,
                                                    SQLSMALLINT         Nullable);
    extern T3SQLSpecialColumns pODBC3SQLSpecialColumns;
    #define N3SQLSpecialColumns(a,b,c,d,e,f,g,h,i,j) (*pODBC3SQLSpecialColumns)(a,b,c,d,e,f,g,h,i,j)

    typedef SQLRETURN  (SQL_API  *T3SQLStatistics) (    SQLHSTMT            StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3,
                                                    SQLUSMALLINT        Unique,
                                                    SQLUSMALLINT        Reserved);
    extern T3SQLStatistics pODBC3SQLStatistics;
    #define N3SQLStatistics(a,b,c,d,e,f,g,h,i) (*pODBC3SQLStatistics)(a,b,c,d,e,f,g,h,i)

    typedef SQLRETURN  (SQL_API  *T3SQLTablePrivileges) (SQLHSTMT           StatementHandle,
                                                    SQLCHAR *           CatalogName,
                                                    SQLSMALLINT         NameLength1,
                                                    SQLCHAR *           SchemaName,
                                                    SQLSMALLINT         NameLength2,
                                                    SQLCHAR *           TableName,
                                                    SQLSMALLINT         NameLength3);
    extern T3SQLTablePrivileges pODBC3SQLTablePrivileges;
    #define N3SQLTablePrivileges(a,b,c,d,e,f,g) (*pODBC3SQLTablePrivileges)(a,b,c,d,e,f,g)

    typedef SQLRETURN  (SQL_API  *T3SQLTables) (    SQLHSTMT                StatementHandle,
                                                    SQLCHAR *               CatalogName,
                                                    SQLSMALLINT             NameLength1,
                                                    SQLCHAR *               SchemaName,
                                                    SQLSMALLINT             NameLength2,
                                                    SQLCHAR *               TableName,
                                                    SQLSMALLINT             NameLength3,
                                                    SQLCHAR *               TableType,
                                                    SQLSMALLINT             NameLength4);
    extern T3SQLTables pODBC3SQLTables;
    #define N3SQLTables(a,b,c,d,e,f,g,h,i) (*pODBC3SQLTables)(a,b,c,d,e,f,g,h,i)

    // Terminating a statement
    typedef SQLRETURN  (SQL_API  *T3SQLFreeStmt) (  SQLHSTMT                StatementHandle,
                                                    SQLUSMALLINT            Option);
    extern T3SQLFreeStmt pODBC3SQLFreeStmt;
    #define N3SQLFreeStmt(a,b) (*pODBC3SQLFreeStmt)(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLCloseCursor) (SQLHSTMT           StatementHandle);
    extern T3SQLCloseCursor pODBC3SQLCloseCursor;
    #define N3SQLCloseCursor(a) (*pODBC3SQLCloseCursor)(a)

    typedef SQLRETURN  (SQL_API  *T3SQLCancel) (    SQLHSTMT                StatementHandle);
    extern T3SQLCancel pODBC3SQLCancel;
    #define N3SQLCancel(a) (*pODBC3SQLCancel)(a)

    typedef SQLRETURN  (SQL_API  *T3SQLEndTran) (   SQLSMALLINT             HandleType,
                                                    SQLHANDLE               Handle,
                                                    SQLSMALLINT             CompletionType);
    extern T3SQLEndTran pODBC3SQLEndTran;
    #define N3SQLEndTran(a,b,c) (*pODBC3SQLEndTran)(a,b,c)

    // Terminating a connection
    typedef SQLRETURN  (SQL_API  *T3SQLDisconnect) (SQLHDBC ConnectionHandle);
    extern T3SQLDisconnect pODBC3SQLDisconnect;
    #define N3SQLDisconnect(a) (*pODBC3SQLDisconnect)(a)

    typedef SQLRETURN  (SQL_API  *T3SQLFreeHandle) (SQLSMALLINT             HandleType,
                                                    SQLHANDLE               Handle);
    extern T3SQLFreeHandle pODBC3SQLFreeHandle;
    #define N3SQLFreeHandle(a,b) (*pODBC3SQLFreeHandle)(a,b)

    typedef SQLRETURN  (SQL_API  *T3SQLGetCursorName) ( SQLHSTMT            StatementHandle,
                                                        SQLCHAR *           CursorName,
                                                        SQLSMALLINT         BufferLength,
                                                        SQLSMALLINT*        NameLength2);
    extern T3SQLGetCursorName pODBC3SQLGetCursorName;
    #define N3SQLGetCursorName(a,b,c,d) (*pODBC3SQLGetCursorName)(a,b,c,d)

    typedef SQLRETURN  (SQL_API  *T3SQLNativeSql) ( SQLHSTMT                ConnectionHandle,
                                                    SQLCHAR *               InStatementText,
                                                    SQLINTEGER              TextLength1,
                                                    SQLCHAR *               OutStatementText,
                                                    SQLINTEGER              BufferLength,
                                                    SQLINTEGER *            TextLength2Ptr);
    extern T3SQLNativeSql pODBC3SQLNativeSql;
    #define N3SQLNativeSql(a,b,c,d,e,f) (*pODBC3SQLNativeSql)(a,b,c,d,e,f)
}

#endif // _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_


