/*
 *  sqltypes.h
 *
 *  $Id$
 *
 *  ODBC typedefs
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

#ifndef _SQLTYPES_H
#define _SQLTYPES_H

/*
 *  Set default specification to  ODBC 3.51
 */
#ifndef ODBCVER
#define ODBCVER		0x0351
#endif

/*
 *  Include Windows style defines and typedefs on Unix
 */
#ifndef _IODBCUNIX_H
#include "iodbcunix.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*
 *  Environment specific definitions
 */
#ifndef EXPORT
#define EXPORT
#endif

#ifdef WIN32
#define SQL_API	__stdcall
#else
#define SQL_API
#endif


/*
 *  API declaration data types
 */
typedef unsigned char		SQLCHAR;
typedef signed short		SQLSMALLINT;
typedef unsigned short		SQLUSMALLINT;
#if (SIZEOF_LONG == 8)
typedef signed int		SQLINTEGER;
typedef unsigned int		SQLUINTEGER;
#else
typedef signed long		SQLINTEGER;
typedef unsigned long		SQLUINTEGER;
#endif
typedef void *              	SQLPOINTER;

#if (ODBCVER >= 0x0300)
typedef signed char		SQLSCHAR;
typedef unsigned char		SQLDATE;
typedef unsigned char		SQLDECIMAL;
typedef unsigned char		SQLNUMERIC;
typedef double			SQLDOUBLE;
typedef double			SQLFLOAT;
typedef float			SQLREAL;
typedef unsigned char		SQLTIME;
typedef unsigned char		SQLTIMESTAMP;
typedef unsigned char		SQLVARCHAR;
#endif	/* ODBCVER >= 0x0300 */


/*
 *  New Win64 datatypes
 */
#ifdef _WIN64
typedef INT64			SQLLEN;
typedef UINT64			SQLULEN;
typedef UINT64			SQLSETPOSIROW;
#elif defined(STRICT_ODBC_TYPES)
typedef long			SQLLEN;
typedef unsigned long		SQLULEN;
typedef unsigned short		SQLSETPOSIROW;
#else
#define SQLLEN			long
#define SQLULEN 		unsigned long
#define SQLSETPOSIROW		unsigned short
#endif


/*
 *  Backward compatibility with older platform sdks
 */
typedef SQLULEN 		SQLROWCOUNT;
typedef SQLULEN 		SQLROWSETSIZE;
typedef SQLULEN 		SQLTRANSID;
typedef SQLLEN			SQLROWOFFSET;


/*
 *  Generic pointer types
 */
typedef void *              	PTR;
typedef void *			SQLHANDLE;


/*
 *  Handles
 */
typedef void *			HENV;
typedef void *			HDBC;
typedef void *			HSTMT;

typedef SQLHANDLE		SQLHENV;
typedef SQLHANDLE		SQLHDBC;
typedef SQLHANDLE		SQLHSTMT;
#if (ODBCVER >= 0x0300)
typedef SQLHANDLE		SQLHDESC;
#endif	/* ODBCVER >= 0x0300 */


/*
 *  Window Handle
 */
#if defined(WIN32) || defined (_WIN64) || defined(OS2)
typedef HWND			SQLHWND;
#elif defined(macintosh)
#include <Dialogs.h>
typedef WindowPtr 		HWND;
typedef HWND 			SQLHWND;
#else
typedef SQLPOINTER 		HWND;
typedef SQLPOINTER 		SQLHWND;
#endif


/*
 *  SQL portable types for C
 */
typedef unsigned char		UCHAR;
typedef signed char		SCHAR;
typedef short int		SWORD;
typedef unsigned short int	UWORD;
typedef long int		SDWORD;
typedef unsigned long int	UDWORD;

typedef signed short		SSHORT;
typedef unsigned short		USHORT;
typedef signed long		SLONG;
typedef unsigned long		ULONG;
typedef float			SFLOAT;
typedef double			SDOUBLE;
typedef double			LDOUBLE;


/*
 *  Return type for functions
 */
typedef signed short		RETCODE;
typedef SQLSMALLINT		SQLRETURN;


/*
 *  SQL portable types for C - DATA, TIME, TIMESTAMP, and BOOKMARK
 */
typedef SQLULEN			BOOKMARK;


typedef struct tagDATE_STRUCT
  {
    SQLSMALLINT year;
    SQLUSMALLINT month;
    SQLUSMALLINT day;
  }
DATE_STRUCT;

#if (ODBCVER >= 0x0300)
typedef DATE_STRUCT		SQL_DATE_STRUCT;
#endif	/* ODBCVER >= 0x0300 */


typedef struct tagTIME_STRUCT
  {
    SQLUSMALLINT hour;
    SQLUSMALLINT minute;
    SQLUSMALLINT second;
  }
TIME_STRUCT;

#if (ODBCVER >= 0x0300)
typedef TIME_STRUCT		SQL_TIME_STRUCT;
#endif	/* ODBCVER >= 0x0300 */


typedef struct tagTIMESTAMP_STRUCT
  {
    SQLSMALLINT year;
    SQLUSMALLINT month;
    SQLUSMALLINT day;
    SQLUSMALLINT hour;
    SQLUSMALLINT minute;
    SQLUSMALLINT second;
    SQLUINTEGER fraction;
  }
TIMESTAMP_STRUCT;

#if (ODBCVER >= 0x0300)
typedef TIMESTAMP_STRUCT	SQL_TIMESTAMP_STRUCT;
#endif	/* ODBCVER >= 0x0300 */


/*
 *  Enumeration for DATETIME_INTERVAL_SUBCODE values for interval data types
 *
 *  These values are from SQL-92
 */
#if (ODBCVER >= 0x0300)
typedef enum
  {
    SQL_IS_YEAR			= 1,
    SQL_IS_MONTH		= 2,
    SQL_IS_DAY			= 3,
    SQL_IS_HOUR			= 4,
    SQL_IS_MINUTE		= 5,
    SQL_IS_SECOND		= 6,
    SQL_IS_YEAR_TO_MONTH	= 7,
    SQL_IS_DAY_TO_HOUR		= 8,
    SQL_IS_DAY_TO_MINUTE	= 9,
    SQL_IS_DAY_TO_SECOND	= 10,
    SQL_IS_HOUR_TO_MINUTE	= 11,
    SQL_IS_HOUR_TO_SECOND	= 12,
    SQL_IS_MINUTE_TO_SECOND	= 13
  }
SQLINTERVAL;


typedef struct tagSQL_YEAR_MONTH
  {
    SQLUINTEGER year;
    SQLUINTEGER month;
  }
SQL_YEAR_MONTH_STRUCT;


typedef struct tagSQL_DAY_SECOND
  {
    SQLUINTEGER day;
    SQLUINTEGER hour;
    SQLUINTEGER minute;
    SQLUINTEGER second;
    SQLUINTEGER fraction;
  }
SQL_DAY_SECOND_STRUCT;


typedef struct tagSQL_INTERVAL_STRUCT
  {
    SQLINTERVAL interval_type;
    SQLSMALLINT interval_sign;
    union
      {
	SQL_YEAR_MONTH_STRUCT year_month;
	SQL_DAY_SECOND_STRUCT day_second;
      }
    intval;
  }
SQL_INTERVAL_STRUCT;
#endif	/* ODBCVER >= 0x0300 */


/*
 *  The ODBC C types for SQL_C_SBIGINT and SQL_C_UBIGINT
 */
#if (ODBCVER >= 0x0300)

#if defined(_MSC_VER) && (_MSC_VER >= 900)
#  define ODBCINT64 		__int64
#endif

#ifndef ODBCINT64
# if (SIZEOF_LONG == 8)
#   define ODBCINT64		long
# else
#   define ODBCINT64		long long
# endif
#endif /* ODBCINT64 */

#if defined (ODBCINT64)
typedef signed   ODBCINT64	SQLBIGINT;
typedef unsigned ODBCINT64	SQLUBIGINT;
#endif	/* ODBCINT64 */

#endif	/* ODBCVER >= 0x0300 */


/*
 *  The internal representation of the numeric data type
 */
#if (ODBCVER >= 0x0300)
#define SQL_MAX_NUMERIC_LEN	16
typedef struct tagSQL_NUMERIC_STRUCT
  {
    SQLCHAR	precision;
    SQLSCHAR	scale;
    SQLCHAR	sign;		/* 0 for negative, 1 for positive */
    SQLCHAR	val[SQL_MAX_NUMERIC_LEN];
  }
SQL_NUMERIC_STRUCT;
#endif	/* ODBCVER >= 0x0300 */


#if (ODBCVER >= 0x0350)
#ifdef GUID_DEFINED
typedef GUID 			SQLGUID;
#else
typedef struct tagSQLGUID
  {
    unsigned int	Data1;
    unsigned short	Data2;
    unsigned short	Data3;
    unsigned char	Data4[8];	/* BYTE */
  }
SQLGUID;
#endif	/* GUID_DEFINED */
#endif	/* ODBCVER >= 0x0350 */


#if defined(WIN32)
typedef unsigned short SQLWCHAR;
#else
#  include <stdlib.h>

#  if defined(__cplusplus)		|| \
      defined(_WCHAR_T)			|| \
      defined(_WCHAR_T_DEFINED)		|| \
      defined(_WCHAR_T_DEFINED_)	|| \
      defined(_WCHAR_T_DECLARED)	|| \
      defined(_BSD_WCHAR_T_DEFINED_)	|| \
      defined(_BSD_WCHAR_T_)		|| \
      defined(_BSD_CT_RUNE_T_)      || \
      defined(__DEFINED_wchar_t)
typedef wchar_t SQLWCHAR;
#  else
#    error Please make sure your system supports the wchar_t type
#  endif
#endif /* WIN32 */


#ifdef UNICODE
typedef SQLWCHAR        	SQLTCHAR;
#else
typedef SQLCHAR         	SQLTCHAR;
#endif  /* UNICODE */

#ifdef __cplusplus
}
#endif

#endif	/* _SQLTYPES_H */
