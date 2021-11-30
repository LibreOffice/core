/*
 *  sql.h
 *
 *  $Id$
 *
 *  ODBC defines
 *
 *  The iODBC driver manager.
 *
 *  Copyright (C) 1995 Ke Jin <kejin@empress.com>
 *  Copyright (C) 1996-2021 OpenLink Software <iodbc@openlinksw.com>
 *  All Rights Reserved.
 *
 *  This software is released under the terms of either of the following
 *  licenses:
 *
 *      - GNU Library General Public License (see LICENSE.LGPL)
 *      - The BSD License (see LICENSE.BSD).
 *
 *  Note that the only valid version of the LGPL license as far as this
 *  project is concerned is the original GNU Library General Public License
 *  Version 2, dated June 1991.
 *
 *  While not mandated by the BSD license, any patches you make to the
 *  iODBC source code may be contributed back into the iODBC project
 *  at your discretion. Contributions will benefit the Open Source and
 *  Data Access community as a whole. Submissions may be made at:
 *
 *      http://www.iodbc.org
 *
 *
 *  GNU Library Generic Public License Version 2
 *  ============================================
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; only
 *  Version 2 of the License dated June 1991.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 *  The BSD License
 *  ===============
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. Neither the name of OpenLink Software Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL OPENLINK OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SQL_H
#define _SQL_H

/*
 *  Set default specification to ODBC 3.51
 */
#ifndef ODBCVER
#define ODBCVER		0x0351
#endif

#ifndef _SQLTYPES_H
#include "sqltypes.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Useful Constants
 */
#define SQL_MAX_MESSAGE_LENGTH			512


/*
 *  Handle types
 */
#if (ODBCVER >= 0x0300)
#define SQL_HANDLE_ENV				1
#define SQL_HANDLE_DBC				2
#define SQL_HANDLE_STMT				3
#define SQL_HANDLE_DESC				4
#endif	/* ODBCVER >= 0x0300 */


/*
 *  Function return codes
 */
#define SQL_SUCCESS				0
#define SQL_SUCCESS_WITH_INFO			1
#define SQL_STILL_EXECUTING			2
#define SQL_ERROR				(-1)
#define SQL_INVALID_HANDLE			(-2)
#define SQL_NEED_DATA				99
#if (ODBCVER >= 0x0300)
#define SQL_NO_DATA				100
#endif	/* ODBCVER >= 0x0300 */


/*
 *  Test for SQL_SUCCESS or SQL_SUCCESS_WITH_INFO
 */
#define SQL_SUCCEEDED(rc)			(((rc) & (~1)) == 0)


/*
 *  Special length values
 */
#define SQL_NULL_DATA				(-1)
#define SQL_DATA_AT_EXEC			(-2)


/*
 *  Flags for null-terminated strings
 */
#define SQL_NTS					(-3)
#define SQL_NTSL				(-3L)


/*
 *  Standard SQL datatypes, using ANSI type numbering
 */
#define SQL_UNKNOWN_TYPE			0
#define SQL_CHAR				1
#define SQL_NUMERIC				2
#define SQL_DECIMAL				3
#define SQL_INTEGER				4
#define SQL_SMALLINT				5
#define SQL_FLOAT				6
#define SQL_REAL				7
#define SQL_DOUBLE				8
#if (ODBCVER >= 0x0300)
#define SQL_DATETIME				9
#endif	/* ODBCVER >= 0x0300 */
#define SQL_VARCHAR				12


/*
 *  SQLGetTypeInfo request for all data types
 */
#define SQL_ALL_TYPES				0


/*
 *  Statement attribute values for date/time data types
 */
#if (ODBCVER >= 0x0300)
#define SQL_TYPE_DATE				91
#define SQL_TYPE_TIME				92
#define SQL_TYPE_TIMESTAMP			93
#endif	/* ODBCVER >= 0x0300 */


/*
 *  Date/Time length constants
 */
#if (ODBCVER >= 0x0300)
#define SQL_DATE_LEN				10
#define SQL_TIME_LEN				8	/* add P+1 if prec >0 */
#define SQL_TIMESTAMP_LEN			19	/* add P+1 if prec >0 */
#endif	/* ODBCVER >= 0x0300 */


/*
 *  NULL status constants
 */
#define SQL_NO_NULLS				0
#define SQL_NULLABLE				1
#define SQL_NULLABLE_UNKNOWN			2


/*
 *  NULL Handles
 */
#define SQL_NULL_HENV				0
#define SQL_NULL_HDBC				0
#define SQL_NULL_HSTMT				0
#if (ODBCVER >= 0x0300)
#define SQL_NULL_HDESC				0
#endif	/* ODBCVER >= 0x0300 */


/*
 *  NULL handle for parent argument to SQLAllocHandle when allocating
 *  a SQLHENV
 */
#if (ODBCVER >= 0x0300)
#define SQL_NULL_HANDLE				0L
#endif	/* ODBCVER >= 0x0300 */


/*
 *  CLI option values
 */
#if (ODBCVER >= 0x0300)
#define SQL_FALSE				0
#define SQL_TRUE				1
#endif	/* ODBCVER >= 0x0300 */


/*
 *  Default conversion code for SQLBindCol(), SQLBindParam() and SQLGetData()
 */
#if (ODBCVER >= 0x0300)
#define SQL_DEFAULT				99
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLDataSources/SQLFetchScroll - FetchOrientation
 */
#define SQL_FETCH_NEXT				1
#define SQL_FETCH_FIRST				2


/*
 *  SQLFetchScroll - FetchOrientation
 */
#define SQL_FETCH_LAST				3
#define SQL_FETCH_PRIOR				4
#define SQL_FETCH_ABSOLUTE			5
#define SQL_FETCH_RELATIVE			6


/*
 *  SQLFreeStmt
 */
#define SQL_CLOSE				0
#define SQL_DROP				1
#define SQL_UNBIND				2
#define SQL_RESET_PARAMS			3


/*
 *  SQLGetConnectAttr - connection attributes
 */
#if (ODBCVER >= 0x0300)
#define SQL_ATTR_AUTO_IPD			10001
#define SQL_ATTR_METADATA_ID			10014
#endif	/* ODBCVER >= 0x0300 */


/*
 *   SQLGetData() code indicating that the application row descriptor
 *   specifies the data type
 */
#if (ODBCVER >= 0x0300)
#define SQL_ARD_TYPE				(-99)
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetDescField - identifiers of fields in the SQL descriptor
 */
#if (ODBCVER >= 0x0300)
#define SQL_DESC_COUNT				1001
#define SQL_DESC_TYPE				1002
#define SQL_DESC_LENGTH				1003
#define SQL_DESC_OCTET_LENGTH_PTR		1004
#define SQL_DESC_PRECISION			1005
#define SQL_DESC_SCALE				1006
#define SQL_DESC_DATETIME_INTERVAL_CODE		1007
#define SQL_DESC_NULLABLE			1008
#define SQL_DESC_INDICATOR_PTR			1009
#define SQL_DESC_DATA_PTR			1010
#define SQL_DESC_NAME				1011
#define SQL_DESC_UNNAMED			1012
#define SQL_DESC_OCTET_LENGTH			1013
#define SQL_DESC_ALLOC_TYPE			1099
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetDescField - SQL_DESC_ALLOC_TYPE
 */
#if (ODBCVER >= 0x0300)
#define SQL_DESC_ALLOC_AUTO			1
#define SQL_DESC_ALLOC_USER			2
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetDescField - SQL_DESC_DATETIME_INTERVAL_CODE
 */
#if (ODBCVER >= 0x0300)
#define SQL_CODE_DATE				1
#define SQL_CODE_TIME				2
#define SQL_CODE_TIMESTAMP			3
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetDescField - SQL_DESC_UNNAMED
 */
#if (ODBCVER >= 0x0300)
#define SQL_NAMED				0
#define SQL_UNNAMED				1
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetDiagField - identifiers of fields in the diagnostics area
 */
#if (ODBCVER >= 0x0300)
#define SQL_DIAG_RETURNCODE			1
#define SQL_DIAG_NUMBER				2
#define SQL_DIAG_ROW_COUNT			3
#define SQL_DIAG_SQLSTATE			4
#define SQL_DIAG_NATIVE				5
#define SQL_DIAG_MESSAGE_TEXT			6
#define SQL_DIAG_DYNAMIC_FUNCTION		7
#define SQL_DIAG_CLASS_ORIGIN			8
#define SQL_DIAG_SUBCLASS_ORIGIN		9
#define SQL_DIAG_CONNECTION_NAME		10
#define SQL_DIAG_SERVER_NAME			11
#define SQL_DIAG_DYNAMIC_FUNCTION_CODE		12
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetDiagField - SQL_DIAG_DYNAMIC_FUNCTION_CODE
 */
#if (ODBCVER >= 0x0300)
#define SQL_DIAG_ALTER_DOMAIN			3
#define SQL_DIAG_ALTER_TABLE			4
#define SQL_DIAG_CALL				7
#define SQL_DIAG_CREATE_ASSERTION		6
#define SQL_DIAG_CREATE_CHARACTER_SET		8
#define SQL_DIAG_CREATE_COLLATION		10
#define SQL_DIAG_CREATE_DOMAIN			23
#define SQL_DIAG_CREATE_INDEX			(-1)
#define SQL_DIAG_CREATE_SCHEMA			64
#define SQL_DIAG_CREATE_TABLE			77
#define SQL_DIAG_CREATE_TRANSLATION		79
#define SQL_DIAG_CREATE_VIEW			84
#define SQL_DIAG_DELETE_WHERE			19
#define	SQL_DIAG_DROP_ASSERTION			24
#define	SQL_DIAG_DROP_CHARACTER_SET		25
#define	SQL_DIAG_DROP_COLLATION			26
#define	SQL_DIAG_DROP_DOMAIN			27
#define SQL_DIAG_DROP_INDEX			(-2)
#define SQL_DIAG_DROP_SCHEMA			31
#define SQL_DIAG_DROP_TABLE			32
#define SQL_DIAG_DROP_TRANSLATION		33
#define SQL_DIAG_DROP_VIEW			36
#define SQL_DIAG_DYNAMIC_DELETE_CURSOR		38
#define SQL_DIAG_DYNAMIC_UPDATE_CURSOR		81
#define SQL_DIAG_GRANT				48
#define SQL_DIAG_INSERT				50
#define SQL_DIAG_REVOKE				59
#define SQL_DIAG_SELECT_CURSOR			85
#define SQL_DIAG_UNKNOWN_STATEMENT		0
#define SQL_DIAG_UPDATE_WHERE			82
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetEnvAttr - environment attribute
 */
#if (ODBCVER >= 0x0300)
#define SQL_ATTR_OUTPUT_NTS			10001
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetFunctions
 */
#define SQL_API_SQLALLOCCONNECT			1
#define SQL_API_SQLALLOCENV			2
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLALLOCHANDLE			1001
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLALLOCSTMT			3
#define SQL_API_SQLBINDCOL			4
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLBINDPARAM			1002
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLCANCEL			5
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLCLOSECURSOR			1003
#define SQL_API_SQLCOLATTRIBUTE			6
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLCOLUMNS			40
#define SQL_API_SQLCONNECT			7
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLCOPYDESC			1004
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLDATASOURCES			57
#define SQL_API_SQLDESCRIBECOL			8
#define SQL_API_SQLDISCONNECT			9
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLENDTRAN			1005
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLERROR			10
#define SQL_API_SQLEXECDIRECT			11
#define SQL_API_SQLEXECUTE			12
#define SQL_API_SQLFETCH			13
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLFETCHSCROLL			1021
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLFREECONNECT			14
#define SQL_API_SQLFREEENV			15
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLFREEHANDLE			1006
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLFREESTMT			16
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLGETCONNECTATTR		1007
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLGETCONNECTOPTION		42
#define SQL_API_SQLGETCURSORNAME		17
#define SQL_API_SQLGETDATA			43
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLGETDESCFIELD			1008
#define SQL_API_SQLGETDESCREC			1009
#define SQL_API_SQLGETDIAGFIELD			1010
#define SQL_API_SQLGETDIAGREC			1011
#define SQL_API_SQLGETENVATTR			1012
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLGETFUNCTIONS			44
#define SQL_API_SQLGETINFO			45
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLGETSTMTATTR			1014
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLGETSTMTOPTION		46
#define SQL_API_SQLGETTYPEINFO			47
#define SQL_API_SQLNUMRESULTCOLS		18
#define SQL_API_SQLPARAMDATA			48
#define SQL_API_SQLPREPARE			19
#define SQL_API_SQLPUTDATA			49
#define SQL_API_SQLROWCOUNT			20
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLSETCONNECTATTR		1016
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLSETCONNECTOPTION		50
#define SQL_API_SQLSETCURSORNAME		21
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLSETDESCFIELD			1017
#define SQL_API_SQLSETDESCREC			1018
#define SQL_API_SQLSETENVATTR			1019
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLSETPARAM			22
#if (ODBCVER >= 0x0300)
#define SQL_API_SQLSETSTMTATTR			1020
#endif	/* ODBCVER >= 0x0300 */
#define SQL_API_SQLSETSTMTOPTION		51
#define SQL_API_SQLSPECIALCOLUMNS		52
#define SQL_API_SQLSTATISTICS			53
#define SQL_API_SQLTABLES			54
#define SQL_API_SQLTRANSACT			23


/*
 *  SQLGetInfo
 */
#if (ODBCVER >= 0x0300)
#define SQL_MAX_DRIVER_CONNECTIONS		0
#define SQL_MAXIMUM_DRIVER_CONNECTIONS		SQL_MAX_DRIVER_CONNECTIONS
#define SQL_MAX_CONCURRENT_ACTIVITIES		1
#define SQL_MAXIMUM_CONCURRENT_ACTIVITIES	SQL_MAX_CONCURRENT_ACTIVITIES
#endif	/* ODBCVER >= 0x0300 */
#define SQL_DATA_SOURCE_NAME			2
#define SQL_FETCH_DIRECTION			8
#define SQL_SERVER_NAME				13
#define SQL_SEARCH_PATTERN_ESCAPE		14
#define SQL_DBMS_NAME				17
#define SQL_DBMS_VER				18
#define SQL_ACCESSIBLE_TABLES			19
#define SQL_ACCESSIBLE_PROCEDURES		20
#define SQL_CURSOR_COMMIT_BEHAVIOR		23
#define SQL_DATA_SOURCE_READ_ONLY		25
#define SQL_DEFAULT_TXN_ISOLATION		26
#define SQL_IDENTIFIER_CASE			28
#define SQL_IDENTIFIER_QUOTE_CHAR		29
#define SQL_MAX_COLUMN_NAME_LEN			30
#define SQL_MAXIMUM_COLUMN_NAME_LENGTH		SQL_MAX_COLUMN_NAME_LEN
#define SQL_MAX_CURSOR_NAME_LEN			31
#define SQL_MAXIMUM_CURSOR_NAME_LENGTH		SQL_MAX_CURSOR_NAME_LEN
#define SQL_MAX_SCHEMA_NAME_LEN			32
#define SQL_MAXIMUM_SCHEMA_NAME_LENGTH		SQL_MAX_SCHEMA_NAME_LEN
#define SQL_MAX_CATALOG_NAME_LEN		34
#define SQL_MAXIMUM_CATALOG_NAME_LENGTH		SQL_MAX_CATALOG_NAME_LEN
#define SQL_MAX_TABLE_NAME_LEN			35
#define SQL_SCROLL_CONCURRENCY			43
#define SQL_TXN_CAPABLE				46
#define SQL_TRANSACTION_CAPABLE			SQL_TXN_CAPABLE
#define SQL_USER_NAME				47
#define SQL_TXN_ISOLATION_OPTION		72
#define SQL_TRANSACTION_ISOLATION_OPTION	SQL_TXN_ISOLATION_OPTION
#define SQL_INTEGRITY				73
#define SQL_GETDATA_EXTENSIONS			81
#define SQL_NULL_COLLATION			85
#define SQL_ALTER_TABLE				86
#define SQL_ORDER_BY_COLUMNS_IN_SELECT		90
#define SQL_SPECIAL_CHARACTERS			94
#define SQL_MAX_COLUMNS_IN_GROUP_BY		97
#define SQL_MAXIMUM_COLUMNS_IN_GROUP_BY		SQL_MAX_COLUMNS_IN_GROUP_BY
#define SQL_MAX_COLUMNS_IN_INDEX		98
#define SQL_MAXIMUM_COLUMNS_IN_INDEX		SQL_MAX_COLUMNS_IN_INDEX
#define SQL_MAX_COLUMNS_IN_ORDER_BY		99
#define SQL_MAXIMUM_COLUMNS_IN_ORDER_BY		SQL_MAX_COLUMNS_IN_ORDER_BY
#define SQL_MAX_COLUMNS_IN_SELECT		100
#define SQL_MAXIMUM_COLUMNS_IN_SELECT		SQL_MAX_COLUMNS_IN_SELECT
#define SQL_MAX_COLUMNS_IN_TABLE		101
#define SQL_MAX_INDEX_SIZE			102
#define SQL_MAXIMUM_INDEX_SIZE			SQL_MAX_INDEX_SIZE
#define SQL_MAX_ROW_SIZE			104
#define SQL_MAXIMUM_ROW_SIZE			SQL_MAX_ROW_SIZE
#define SQL_MAX_STATEMENT_LEN			105
#define SQL_MAXIMUM_STATEMENT_LENGTH		SQL_MAX_STATEMENT_LEN
#define SQL_MAX_TABLES_IN_SELECT		106
#define SQL_MAXIMUM_TABLES_IN_SELECT		SQL_MAX_TABLES_IN_SELECT
#define SQL_MAX_USER_NAME_LEN			107
#define SQL_MAXIMUM_USER_NAME_LENGTH		SQL_MAX_USER_NAME_LEN
#if (ODBCVER >= 0x0300)
#define SQL_OJ_CAPABILITIES			115
#define SQL_OUTER_JOIN_CAPABILITIES		SQL_OJ_CAPABILITIES
#endif	/* ODBCVER >= 0x0300 */

#if (ODBCVER >= 0x0300)
#define SQL_XOPEN_CLI_YEAR			10000
#define SQL_CURSOR_SENSITIVITY			10001
#define SQL_DESCRIBE_PARAMETER			10002
#define SQL_CATALOG_NAME			10003
#define SQL_COLLATION_SEQ			10004
#define SQL_MAX_IDENTIFIER_LEN			10005
#define SQL_MAXIMUM_IDENTIFIER_LENGTH		SQL_MAX_IDENTIFIER_LEN
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetInfo - SQL_ALTER_TABLE
 */
#if (ODBCVER >= 0x0200)
#define SQL_AT_ADD_COLUMN			0x00000001L
#define SQL_AT_DROP_COLUMN			0x00000002L
#endif	/* ODBCVER >= 0x0300 */

#if (ODBCVER >= 0x0300)
#define SQL_AT_ADD_CONSTRAINT			0x00000008L
/*
 * The following bitmasks are ODBC extensions and defined in sqlext.h:
 *
 * #define SQL_AT_COLUMN_SINGLE				0x00000020L
 * #define SQL_AT_ADD_COLUMN_DEFAULT			0x00000040L
 * #define SQL_AT_ADD_COLUMN_COLLATION			0x00000080L
 * #define SQL_AT_SET_COLUMN_DEFAULT			0x00000100L
 * #define SQL_AT_DROP_COLUMN_DEFAULT			0x00000200L
 * #define SQL_AT_DROP_COLUMN_CASCADE			0x00000400L
 * #define SQL_AT_DROP_COLUMN_RESTRICT			0x00000800L
 * #define SQL_AT_ADD_TABLE_CONSTRAINT			0x00001000L
 * #define SQL_AT_DROP_TABLE_CONSTRAINT_CASCADE		0x00002000L
 * #define SQL_AT_DROP_TABLE_CONSTRAINT_RESTRICT	0x00004000L
 * #define SQL_AT_CONSTRAINT_NAME_DEFINITION		0x00008000L
 * #define SQL_AT_CONSTRAINT_INITIALLY_DEFERRED		0x00010000L
 * #define SQL_AT_CONSTRAINT_INITIALLY_IMMEDIATE	0x00020000L
 * #define SQL_AT_CONSTRAINT_DEFERRABLE			0x00040000L
 * #define SQL_AT_CONSTRAINT_NON_DEFERRABLE		0x00080000L
 */
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetInfo - SQL_ASYNC_MODE
 */
#if (ODBCVER >= 0x0300)
#define SQL_AM_NONE				0
#define SQL_AM_CONNECTION			1
#define SQL_AM_STATEMENT			2
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetInfo - SQL_CURSOR_COMMIT_BEHAVIOR
 */
#define SQL_CB_DELETE				0
#define SQL_CB_CLOSE				1
#define SQL_CB_PRESERVE				2


/*
 *  SQLGetInfo - SQL_FETCH_DIRECTION
 */
#define SQL_FD_FETCH_NEXT			0x00000001L
#define SQL_FD_FETCH_FIRST			0x00000002L
#define SQL_FD_FETCH_LAST			0x00000004L
#define SQL_FD_FETCH_PRIOR			0x00000008L
#define SQL_FD_FETCH_ABSOLUTE			0x00000010L
#define SQL_FD_FETCH_RELATIVE			0x00000020L


/*
 *  SQLGetInfo - SQL_GETDATA_EXTENSIONS
 */
#define SQL_GD_ANY_COLUMN			0x00000001L
#define SQL_GD_ANY_ORDER			0x00000002L


/*
 *  SQLGetInfo - SQL_IDENTIFIER_CASE
 */
#define SQL_IC_UPPER				1
#define SQL_IC_LOWER				2
#define SQL_IC_SENSITIVE			3
#define SQL_IC_MIXED				4


/*
 *  SQLGetInfo - SQL_NULL_COLLATION
 */
#define SQL_NC_HIGH				0
#define SQL_NC_LOW				1


/*
 *  SQLGetInfo - SQL_OJ_CAPABILITIES
 */
#if (ODBCVER >= 0x0201)
#define SQL_OJ_LEFT				0x00000001L
#define SQL_OJ_RIGHT				0x00000002L
#define SQL_OJ_FULL				0x00000004L
#define SQL_OJ_NESTED				0x00000008L
#define SQL_OJ_NOT_ORDERED			0x00000010L
#define SQL_OJ_INNER				0x00000020L
#define SQL_OJ_ALL_COMPARISON_OPS		0x00000040L
#endif


/*
 *  SQLGetInfo - SQL_SCROLL_CONCURRENCY
 */
#define SQL_SCCO_READ_ONLY			0x00000001L
#define SQL_SCCO_LOCK				0x00000002L
#define SQL_SCCO_OPT_ROWVER			0x00000004L
#define SQL_SCCO_OPT_VALUES			0x00000008L


/*
 *  SQLGetInfo - SQL_TXN_CAPABLE
 */
#define SQL_TC_NONE				0
#define SQL_TC_DML				1
#define SQL_TC_ALL				2
#define SQL_TC_DDL_COMMIT			3
#define SQL_TC_DDL_IGNORE			4


/*
 *  SQLGetInfo - SQL_TXN_ISOLATION_OPTION
 */
#define SQL_TXN_READ_UNCOMMITTED		0x00000001L
#define SQL_TRANSACTION_READ_UNCOMMITTED	SQL_TXN_READ_UNCOMMITTED
#define SQL_TXN_READ_COMMITTED			0x00000002L
#define SQL_TRANSACTION_READ_COMMITTED		SQL_TXN_READ_COMMITTED
#define SQL_TXN_REPEATABLE_READ			0x00000004L
#define SQL_TRANSACTION_REPEATABLE_READ		SQL_TXN_REPEATABLE_READ
#define SQL_TXN_SERIALIZABLE			0x00000008L
#define SQL_TRANSACTION_SERIALIZABLE		SQL_TXN_SERIALIZABLE


/*
 *  SQLGetStmtAttr - statement attributes
 */
#if (ODBCVER >= 0x0300)
#define SQL_ATTR_APP_ROW_DESC			10010
#define SQL_ATTR_APP_PARAM_DESC			10011
#define SQL_ATTR_IMP_ROW_DESC			10012
#define SQL_ATTR_IMP_PARAM_DESC			10013
#define SQL_ATTR_CURSOR_SCROLLABLE		(-1)
#define SQL_ATTR_CURSOR_SENSITIVITY		(-2)
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetStmtAttr - SQL_ATTR_CURSOR_SCROLLABLE
 */
#if (ODBCVER >= 0x0300)
#define SQL_NONSCROLLABLE			0
#define SQL_SCROLLABLE				1
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetStmtAttr - SQL_ATTR_CURSOR_SENSITIVITY
 */
#if (ODBCVER >= 0x0300)
#define SQL_UNSPECIFIED				0
#define SQL_INSENSITIVE				1
#define SQL_SENSITIVE				2
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLGetTypeInfo - SEARCHABLE
 */
#if (ODBCVER >= 0x0300)
#define SQL_PRED_NONE				0
#define SQL_PRED_CHAR				1
#define SQL_PRED_BASIC				2
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLSpecialColumns - Column scopes
 */
#define SQL_SCOPE_CURROW			0
#define SQL_SCOPE_TRANSACTION			1
#define SQL_SCOPE_SESSION			2


/*
 *  SQLSpecialColumns - PSEUDO_COLUMN
 */
#define SQL_PC_UNKNOWN				0
#if (ODBCVER >= 0x0300)
#define SQL_PC_NON_PSEUDO			1
#endif	/* ODBCVER >= 0x0300 */
#define SQL_PC_PSEUDO				2


/*
 *  SQLSpecialColumns - IdentifierType
 */
#if (ODBCVER >= 0x0300)
#define SQL_ROW_IDENTIFIER			1
#endif	/* ODBCVER >= 0x0300 */


/*
 *  SQLStatistics - fUnique
 */
#define SQL_INDEX_UNIQUE			0
#define SQL_INDEX_ALL				1


/*
 *  SQLStatistics - TYPE
 */
#define SQL_INDEX_CLUSTERED			1
#define SQL_INDEX_HASHED			2
#define SQL_INDEX_OTHER				3


/*
 *  SQLTransact/SQLEndTran
 */
#define SQL_COMMIT				0
#define SQL_ROLLBACK				1


/*
 *  Function Prototypes
 */
SQLRETURN SQL_API SQLAllocConnect (
    SQLHENV		  EnvironmentHandle,
    SQLHDBC		* ConnectionHandle);

SQLRETURN SQL_API SQLAllocEnv (
    SQLHENV		* EnvironmentHandle);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLAllocHandle (
    SQLSMALLINT		  HandleType,
    SQLHANDLE		  InputHandle,
    SQLHANDLE		* OutputHandle);
#endif	/* ODBCVER >= 0x0300 */

SQLRETURN SQL_API SQLAllocStmt (
    SQLHDBC		  ConnectionHandle,
    SQLHSTMT		* StatementHandle);

SQLRETURN SQL_API SQLBindCol (
    SQLHSTMT		  StatementHandle,
    SQLUSMALLINT	  ColumnNumber,
    SQLSMALLINT		  TargetType,
    SQLPOINTER		  TargetValue,
    SQLLEN		  BufferLength,
    SQLLEN		* StrLen_or_Ind);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLBindParam (
    SQLHSTMT		  StatementHandle,
    SQLUSMALLINT	  ParameterNumber,
    SQLSMALLINT		  ValueType,
    SQLSMALLINT		  ParameterType,
    SQLULEN		  LengthPrecision,
    SQLSMALLINT		  ParameterScale,
    SQLPOINTER		  ParameterValue,
    SQLLEN		* StrLen_or_Ind);
#endif

SQLRETURN SQL_API SQLCancel (
    SQLHSTMT		  StatementHandle);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLCloseCursor (
    SQLHSTMT		  StatementHandle);

/*
 *  Using SQLLEN * instead of SQLPOINTER for NumericAttribute,
 *  makes the prototype the same as SQLColAttributes (deprecated)
 *  and clearer for 64bit ports
 */
SQLRETURN SQL_API SQLColAttribute (
    SQLHSTMT		  StatementHandle,
    SQLUSMALLINT	  ColumnNumber,
    SQLUSMALLINT	  FieldIdentifier,
    SQLPOINTER		  CharacterAttribute,
    SQLSMALLINT		  BufferLength,
    SQLSMALLINT		* StringLength,
    SQLLEN		* NumericAttribute);
#endif

SQLRETURN SQL_API SQLColumns (
    SQLHSTMT		  StatementHandle,
    SQLCHAR		* CatalogName,
    SQLSMALLINT		  NameLength1,
    SQLCHAR		* SchemaName,
    SQLSMALLINT		  NameLength2,
    SQLCHAR		* TableName,
    SQLSMALLINT		  NameLength3,
    SQLCHAR		* ColumnName,
    SQLSMALLINT		  NameLength4);

SQLRETURN SQL_API SQLConnect (
    SQLHDBC		  ConnectionHandle,
    SQLCHAR		* ServerName,
    SQLSMALLINT		  NameLength1,
    SQLCHAR		* UserName,
    SQLSMALLINT		  NameLength2,
    SQLCHAR		* Authentication,
    SQLSMALLINT		  NameLength3);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLCopyDesc (
    SQLHDESC		  SourceDescHandle,
    SQLHDESC		  TargetDescHandle);
#endif

SQLRETURN SQL_API SQLDataSources (
    SQLHENV		  EnvironmentHandle,
    SQLUSMALLINT	  Direction,
    SQLCHAR		* ServerName,
    SQLSMALLINT		  BufferLength1,
    SQLSMALLINT		* NameLength1,
    SQLCHAR		* Description,
    SQLSMALLINT		  BufferLength2,
    SQLSMALLINT		* NameLength2);

SQLRETURN SQL_API SQLDescribeCol (
    SQLHSTMT		  StatementHandle,
    SQLUSMALLINT	  ColumnNumber,
    SQLCHAR		* ColumnName,
    SQLSMALLINT		  BufferLength,
    SQLSMALLINT		* NameLength,
    SQLSMALLINT		* DataType,
    SQLULEN		* ColumnSize,
    SQLSMALLINT		* DecimalDigits,
    SQLSMALLINT		* Nullable);

SQLRETURN SQL_API SQLDisconnect (
    SQLHDBC		  ConnectionHandle);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLEndTran (
    SQLSMALLINT		  HandleType,
    SQLHANDLE		  Handle,
    SQLSMALLINT		  CompletionType);
#endif

SQLRETURN SQL_API SQLError (
    SQLHENV		  EnvironmentHandle,
    SQLHDBC		  ConnectionHandle,
    SQLHSTMT		  StatementHandle,
    SQLCHAR		* Sqlstate,
    SQLINTEGER		* NativeError,
    SQLCHAR		* MessageText,
    SQLSMALLINT		  BufferLength,
    SQLSMALLINT		* TextLength);

SQLRETURN SQL_API SQLExecDirect (
    SQLHSTMT		  StatementHandle,
    SQLCHAR		* StatementText,
    SQLINTEGER		  TextLength);

SQLRETURN SQL_API SQLExecute (
    SQLHSTMT		  StatementHandle);

SQLRETURN SQL_API SQLFetch (
    SQLHSTMT		  StatementHandle);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLFetchScroll (
    SQLHSTMT		  StatementHandle,
    SQLSMALLINT		  FetchOrientation,
    SQLLEN		  FetchOffset);
#endif

SQLRETURN SQL_API SQLFreeConnect (
    SQLHDBC		  ConnectionHandle);

SQLRETURN SQL_API SQLFreeEnv (
    SQLHENV		  EnvironmentHandle);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLFreeHandle (
    SQLSMALLINT		  HandleType,
    SQLHANDLE		  Handle);
#endif

SQLRETURN SQL_API SQLFreeStmt (
    SQLHSTMT		  StatementHandle,
    SQLUSMALLINT	  Option);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLGetConnectAttr (
    SQLHDBC		  ConnectionHandle,
    SQLINTEGER		  Attribute,
    SQLPOINTER		  Value,
    SQLINTEGER		  BufferLength,
    SQLINTEGER		* StringLength);
#endif

SQLRETURN SQL_API SQLGetConnectOption (
    SQLHDBC		  ConnectionHandle,
    SQLUSMALLINT	  Option,
    SQLPOINTER		  Value);

SQLRETURN SQL_API SQLGetCursorName (
    SQLHSTMT		  StatementHandle,
    SQLCHAR		* CursorName,
    SQLSMALLINT		  BufferLength,
    SQLSMALLINT		* NameLength);

SQLRETURN SQL_API SQLGetData (
    SQLHSTMT		  StatementHandle,
    SQLUSMALLINT	  ColumnNumber,
    SQLSMALLINT		  TargetType,
    SQLPOINTER		  TargetValue,
    SQLLEN		  BufferLength,
    SQLLEN		* StrLen_or_Ind);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLGetDescField (
    SQLHDESC		  DescriptorHandle,
    SQLSMALLINT		  RecNumber,
    SQLSMALLINT		  FieldIdentifier,
    SQLPOINTER		  Value,
    SQLINTEGER		  BufferLength,
    SQLINTEGER		* StringLength);

SQLRETURN SQL_API SQLGetDescRec (
    SQLHDESC		  DescriptorHandle,
    SQLSMALLINT		  RecNumber,
    SQLCHAR		* Name,
    SQLSMALLINT		  BufferLength,
    SQLSMALLINT		* StringLength,
    SQLSMALLINT		* Type,
    SQLSMALLINT		* SubType,
    SQLLEN		* Length,
    SQLSMALLINT		* Precision,
    SQLSMALLINT		* Scale,
    SQLSMALLINT		* Nullable);

SQLRETURN SQL_API SQLGetDiagField (
    SQLSMALLINT		  HandleType,
    SQLHANDLE		  Handle,
    SQLSMALLINT		  RecNumber,
    SQLSMALLINT		  DiagIdentifier,
    SQLPOINTER		  DiagInfo,
    SQLSMALLINT		  BufferLength,
    SQLSMALLINT		* StringLength);

SQLRETURN SQL_API SQLGetDiagRec (
    SQLSMALLINT		  HandleType,
    SQLHANDLE		  Handle,
    SQLSMALLINT		  RecNumber,
    SQLCHAR		* Sqlstate,
    SQLINTEGER		* NativeError,
    SQLCHAR		* MessageText,
    SQLSMALLINT		  BufferLength,
    SQLSMALLINT		* TextLength);

SQLRETURN SQL_API SQLGetEnvAttr (
    SQLHENV		  EnvironmentHandle,
    SQLINTEGER		  Attribute,
    SQLPOINTER		  Value,
    SQLINTEGER		  BufferLength,
    SQLINTEGER		* StringLength);
#endif /* ODBCVER >= 0x0300 */

SQLRETURN SQL_API SQLGetFunctions (
    SQLHDBC		ConnectionHandle,
    SQLUSMALLINT	FunctionId,
    SQLUSMALLINT *	Supported);

SQLRETURN SQL_API SQLGetInfo (
    SQLHDBC		  ConnectionHandle,
    SQLUSMALLINT	  InfoType,
    SQLPOINTER		  InfoValue,
    SQLSMALLINT		  BufferLength,
    SQLSMALLINT		* StringLength);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLGetStmtAttr (
    SQLHSTMT		  StatementHandle,
    SQLINTEGER		  Attribute,
    SQLPOINTER		  Value,
    SQLINTEGER		  BufferLength,
    SQLINTEGER		* StringLength);
#endif /* ODBCVER >= 0x0300 */

SQLRETURN SQL_API SQLGetStmtOption (
    SQLHSTMT		  StatementHandle,
    SQLUSMALLINT	  Option,
    SQLPOINTER		  Value);

SQLRETURN SQL_API SQLGetTypeInfo (
    SQLHSTMT		  StatementHandle,
    SQLSMALLINT		  DataType);

SQLRETURN SQL_API SQLNumResultCols (
    SQLHSTMT		  StatementHandle,
    SQLSMALLINT		* ColumnCount);

SQLRETURN SQL_API SQLParamData (
    SQLHSTMT		  StatementHandle,
    SQLPOINTER		* Value);

SQLRETURN SQL_API SQLPrepare (
    SQLHSTMT		  StatementHandle,
    SQLCHAR		* StatementText,
    SQLINTEGER		  TextLength);

SQLRETURN SQL_API SQLPutData (
    SQLHSTMT		  StatementHandle,
    SQLPOINTER		  Data,
    SQLLEN		  StrLen_or_Ind);

SQLRETURN SQL_API SQLRowCount (
    SQLHSTMT		  StatementHandle,
    SQLLEN		* RowCount);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLSetConnectAttr (
    SQLHDBC		  ConnectionHandle,
    SQLINTEGER		  Attribute,
    SQLPOINTER		  Value,
    SQLINTEGER		  StringLength);
#endif /* ODBCVER >= 0x0300 */

SQLRETURN SQL_API SQLSetConnectOption (
    SQLHDBC		  ConnectionHandle,
    SQLUSMALLINT	  Option,
    SQLULEN		  Value);

SQLRETURN SQL_API SQLSetCursorName (
    SQLHSTMT		  StatementHandle,
    SQLCHAR		* CursorName,
    SQLSMALLINT		  NameLength);

#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLSetDescField (
    SQLHDESC		  DescriptorHandle,
    SQLSMALLINT		  RecNumber,
    SQLSMALLINT		  FieldIdentifier,
    SQLPOINTER		  Value,
    SQLINTEGER		  BufferLength);

SQLRETURN SQL_API SQLSetDescRec (
    SQLHDESC		  DescriptorHandle,
    SQLSMALLINT		  RecNumber,
    SQLSMALLINT		  Type,
    SQLSMALLINT		  SubType,
    SQLLEN		  Length,
    SQLSMALLINT		  Precision,
    SQLSMALLINT		  Scale,
    SQLPOINTER		  Data,
    SQLLEN		* StringLength,
    SQLLEN		* Indicator);

SQLRETURN SQL_API SQLSetEnvAttr (
    SQLHENV		  EnvironmentHandle,
    SQLINTEGER		  Attribute,
    SQLPOINTER		  Value,
    SQLINTEGER		  StringLength);
#endif /* ODBCVER >= 0x0300 */


#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLSetStmtAttr (
    SQLHSTMT		  StatementHandle,
    SQLINTEGER		  Attribute,
    SQLPOINTER		  Value,
    SQLINTEGER		  StringLength);
#endif

SQLRETURN SQL_API SQLSetStmtOption (
    SQLHSTMT		  StatementHandle,
    SQLUSMALLINT	  Option,
    SQLULEN		  Value);

SQLRETURN SQL_API SQLSpecialColumns (
    SQLHSTMT		  StatementHandle,
    SQLUSMALLINT	  IdentifierType,
    SQLCHAR		* CatalogName,
    SQLSMALLINT		  NameLength1,
    SQLCHAR		* SchemaName,
    SQLSMALLINT		  NameLength2,
    SQLCHAR		* TableName,
    SQLSMALLINT		  NameLength3,
    SQLUSMALLINT	  Scope,
    SQLUSMALLINT	  Nullable);

SQLRETURN SQL_API SQLStatistics (
    SQLHSTMT		  StatementHandle,
    SQLCHAR		* CatalogName,
    SQLSMALLINT		  NameLength1,
    SQLCHAR		* SchemaName,
    SQLSMALLINT		  NameLength2,
    SQLCHAR		* TableName,
    SQLSMALLINT		  NameLength3,
    SQLUSMALLINT	  Unique,
    SQLUSMALLINT	  Reserved);

SQLRETURN SQL_API SQLTables (
    SQLHSTMT		  StatementHandle,
    SQLCHAR		* CatalogName,
    SQLSMALLINT		  NameLength1,
    SQLCHAR		* SchemaName,
    SQLSMALLINT		  NameLength2,
    SQLCHAR		* TableName,
    SQLSMALLINT		  NameLength3,
    SQLCHAR		* TableType,
    SQLSMALLINT		  NameLength4);

SQLRETURN SQL_API SQLTransact (
    SQLHENV		  EnvironmentHandle,
    SQLHDBC		  ConnectionHandle,
    SQLUSMALLINT	  CompletionType);


/*
 *  Depreciated ODBC 1.0 function - Use SQLBindParameter
 */
SQLRETURN SQL_API SQLSetParam (
    SQLHSTMT		  StatementHandle,
    SQLUSMALLINT	  ParameterNumber,
    SQLSMALLINT		  ValueType,
    SQLSMALLINT		  ParameterType,
    SQLULEN		  LengthPrecision,
    SQLSMALLINT		  ParameterScale,
    SQLPOINTER		  ParameterValue,
    SQLLEN		* StrLen_or_Ind);

#ifdef __cplusplus
}
#endif

#endif	 /* _SQL_H */
