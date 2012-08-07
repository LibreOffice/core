/*************************************************************
 * sqltypes.h
 *
 * This is the lowest level include in unixODBC. It defines
 * the basic types required by unixODBC and is heavily based
 * upon the MS include of the same name (it has to be for
 * binary compatability between drivers developed under different
 * packages).
 *
 * You can include this file directly but it is almost always
 * included indirectly, by including.. for example sqlext.h
 *
 * This include makes no effort to be usefull on any platforms other
 * than Linux (with some exceptions for UNIX in general).
 *
 * !!!DO NOT CONTAMINATE THIS FILE WITH NON-Linux CODE!!!
 *
 *************************************************************/
#ifndef __SQLTYPES_H
#define __SQLTYPES_H

/****************************
 * default to the 3.51 definitions. should define ODBCVER before here if you want an older set of defines
 ***************************/
#ifndef ODBCVER
#define ODBCVER 0x0351
#endif

/*
 * if thi sis set, then use a 4 byte unicode definition, insteead of the 2 bye that MS use
 */

#ifdef SQL_WCHART_CONVERT
/*
 * Use this if you want to use the C/C++ portable definition of  a wide char, wchar_t
 *  Microsoft hardcoded a definition of  unsigned short which may not be compatible with
 *  your platform specific wide char definition.
 */
#include <wchar.h>
#endif

#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SIZEOF_LONG_INT
# define SIZEOF_LONG_INT SAL_TYPES_SIZEOFLONG
#endif
#ifndef ODBCINT64
# define ODBCINT64 sal_Int64
#endif
#ifndef UODBCINT64
# define UODBCINT64 sal_uInt64
#endif

/*
 * this is defined by configure, but will not be on a normal application build
 * the install creates a unixodbc_conf.h file that contains the current build settings
 */

#ifndef SIZEOF_LONG_INT
#include <unixodbc_conf.h>
#endif

#ifndef SIZEOF_LONG_INT
#error "Needs to know how big a long int is to continue!!!"
#endif

/****************************
 * These make up for having no windows.h
 ***************************/
#ifndef ALLREADY_HAVE_WINDOWS_TYPE

#define FAR
#define CALLBACK
#define SQL_API
#define BOOL                int
typedef void*               HWND;
typedef char                CHAR;
#ifdef UNICODE

/*
 * NOTE: The Microsoft unicode define is only for apps that want to use TCHARs and
 *  be able to compile for both unicode and non-unicode with the same source.
 *  This is not recommanded for linux applications and is not supported
 *  by the standard linux string header files.
 */
#ifdef SQL_WCHART_CONVERT
typedef wchar_t             TCHAR;
#else
typedef signed short        TCHAR;
#endif

#else
typedef char                TCHAR;
#endif

#ifndef DONT_TD_VOID
typedef void                VOID;
#endif

typedef unsigned short      WORD;
#if (SIZEOF_LONG_INT == 4)
typedef unsigned long       DWORD;
#else
typedef unsigned int        DWORD;
#endif
typedef unsigned char       BYTE;

#ifdef SQL_WCHART_CONVERT
typedef wchar_t             WCHAR;
#else
typedef unsigned short      WCHAR;
#endif

typedef WCHAR*              LPWSTR;
typedef const char*         LPCSTR;
typedef const WCHAR*        LPCWSTR;
typedef TCHAR*              LPTSTR;
typedef char*               LPSTR;
typedef DWORD*              LPDWORD;

typedef void*               HINSTANCE;

#endif


/****************************
 * standard SQL* data types. use these as much as possible when using ODBC calls/vars
 ***************************/
typedef unsigned char   SQLCHAR;

#if (ODBCVER >= 0x0300)
typedef unsigned char   SQLDATE;
typedef unsigned char   SQLDECIMAL;
typedef double          SQLDOUBLE;
typedef double          SQLFLOAT;
#endif

/*
 * can't use a long it fails on 64 platforms
 */

/*
 * Hopefully by now it should be safe to assume most drivers know about SQLLEN now
 * and the defaukt is now sizeof( SQLLEN ) = 8 on 64 bit platforms
 *
 */

#if (SIZEOF_LONG_INT == 8)
#ifdef BUILD_LEGACY_64_BIT_MODE
typedef int             SQLINTEGER;
typedef unsigned int    SQLUINTEGER;
#define SQLLEN          SQLINTEGER
#define SQLULEN         SQLUINTEGER
#define SQLSETPOSIROW   SQLUSMALLINT
/*
 * These are not supprted on 64bit ODBC according to MS, removed, so use at your peril
 *
 typedef SQLULEN         SQLROWCOUNT;
 typedef SQLULEN         SQLROWSETSIZE;
 typedef SQLULEN         SQLTRANSID;
 typedef SQLLEN          SQLROWOFFSET;
*/
#else
typedef int             SQLINTEGER;
typedef unsigned int    SQLUINTEGER;
typedef long            SQLLEN;
typedef unsigned long   SQLULEN;
typedef unsigned long   SQLSETPOSIROW;
/*
 * These are not supprted on 64bit ODBC according to MS, removed, so use at your peril
 *
 typedef SQLULEN        SQLTRANSID;
 typedef SQLULEN        SQLROWCOUNT;
 typedef SQLUINTEGER    SQLROWSETSIZE;
 typedef SQLLEN         SQLROWOFFSET;
 */
typedef SQLULEN         SQLROWCOUNT;
typedef SQLULEN         SQLROWSETSIZE;
typedef SQLULEN         SQLTRANSID;
typedef SQLLEN          SQLROWOFFSET;
#endif
#else
typedef long            SQLINTEGER;
typedef unsigned long   SQLUINTEGER;
#define SQLLEN          SQLINTEGER
#define SQLULEN         SQLUINTEGER
#define SQLSETPOSIROW   SQLUSMALLINT
typedef SQLULEN         SQLROWCOUNT;
typedef SQLULEN         SQLROWSETSIZE;
typedef SQLULEN         SQLTRANSID;
typedef SQLLEN          SQLROWOFFSET;
#endif

#if (ODBCVER >= 0x0300)
typedef unsigned char   SQLNUMERIC;
#endif

typedef void *          SQLPOINTER;

#if (ODBCVER >= 0x0300)
typedef float           SQLREAL;
#endif

typedef signed short int   SQLSMALLINT;
typedef unsigned short  SQLUSMALLINT;

#if (ODBCVER >= 0x0300)
typedef unsigned char   SQLTIME;
typedef unsigned char   SQLTIMESTAMP;
typedef unsigned char   SQLVARCHAR;
#endif

typedef SQLSMALLINT     SQLRETURN;

#if (ODBCVER >= 0x0300)
typedef void *                  SQLHANDLE;
typedef SQLHANDLE               SQLHENV;
typedef SQLHANDLE               SQLHDBC;
typedef SQLHANDLE               SQLHSTMT;
typedef SQLHANDLE               SQLHDESC;
#else
typedef void *                  SQLHENV;
typedef void *                  SQLHDBC;
typedef void *                  SQLHSTMT;
/*
 * some things like PHP won't build without this
 */
typedef void *                  SQLHANDLE;
#endif

/****************************
 * These are cast into the actual struct that is being passed around. The
 * DriverManager knows what its structs look like and the Driver knows about its
 * structs... the app knows nothing about them... just void*
 * These are deprecated in favour of SQLHENV, SQLHDBC, SQLHSTMT
 ***************************/

#if (ODBCVER >= 0x0300)
typedef SQLHANDLE               HENV;
typedef SQLHANDLE               HDBC;
typedef SQLHANDLE               HSTMT;
#else
typedef void *                  HENV;
typedef void *                  HDBC;
typedef void *                  HSTMT;
#endif


/****************************
 * more basic data types to augment what windows.h provides
 ***************************/
#ifndef ALLREADY_HAVE_WINDOWS_TYPE

typedef unsigned char           UCHAR;
typedef signed char             SCHAR;
typedef SCHAR                   SQLSCHAR;
#if (SIZEOF_LONG_INT == 4)
typedef long int                SDWORD;
typedef unsigned long int       UDWORD;
#else
typedef int                     SDWORD;
typedef unsigned int            UDWORD;
#endif
typedef signed short int        SWORD;
typedef unsigned short int      UWORD;
typedef unsigned int            UINT;
typedef signed long             SLONG;
typedef signed short            SSHORT;
typedef unsigned long           ULONG;
typedef unsigned short          USHORT;
typedef double                  SDOUBLE;
typedef double                  LDOUBLE;
typedef float                   SFLOAT;
typedef void*                   PTR;
typedef signed short            RETCODE;
typedef void*                   SQLHWND;

#endif

/****************************
 * standard structs for working with date/times
 ***************************/
#ifndef __SQLDATE
#define __SQLDATE
typedef struct tagDATE_STRUCT
{
        SQLSMALLINT    year;
        SQLUSMALLINT   month;
        SQLUSMALLINT   day;
} DATE_STRUCT;

#if (ODBCVER >= 0x0300)
typedef DATE_STRUCT SQL_DATE_STRUCT;
#endif

typedef struct tagTIME_STRUCT
{
        SQLUSMALLINT   hour;
        SQLUSMALLINT   minute;
        SQLUSMALLINT   second;
} TIME_STRUCT;

#if (ODBCVER >= 0x0300)
typedef TIME_STRUCT SQL_TIME_STRUCT;
#endif

typedef struct tagTIMESTAMP_STRUCT
{
        SQLSMALLINT    year;
        SQLUSMALLINT   month;
        SQLUSMALLINT   day;
        SQLUSMALLINT   hour;
        SQLUSMALLINT   minute;
        SQLUSMALLINT   second;
        SQLUINTEGER    fraction;
} TIMESTAMP_STRUCT;

#if (ODBCVER >= 0x0300)
typedef TIMESTAMP_STRUCT    SQL_TIMESTAMP_STRUCT;
#endif


#if (ODBCVER >= 0x0300)
typedef enum
{
    SQL_IS_YEAR                     = 1,
    SQL_IS_MONTH                    = 2,
    SQL_IS_DAY                      = 3,
    SQL_IS_HOUR                     = 4,
    SQL_IS_MINUTE                   = 5,
    SQL_IS_SECOND                   = 6,
    SQL_IS_YEAR_TO_MONTH            = 7,
    SQL_IS_DAY_TO_HOUR              = 8,
    SQL_IS_DAY_TO_MINUTE            = 9,
    SQL_IS_DAY_TO_SECOND            = 10,
    SQL_IS_HOUR_TO_MINUTE           = 11,
    SQL_IS_HOUR_TO_SECOND           = 12,
    SQL_IS_MINUTE_TO_SECOND         = 13
} SQLINTERVAL;

#endif

#if (ODBCVER >= 0x0300)
typedef struct tagSQL_YEAR_MONTH
{
        SQLUINTEGER     year;
        SQLUINTEGER     month;
} SQL_YEAR_MONTH_STRUCT;

typedef struct tagSQL_DAY_SECOND
{
        SQLUINTEGER     day;
        SQLUINTEGER     hour;
        SQLUINTEGER     minute;
        SQLUINTEGER     second;
        SQLUINTEGER     fraction;
} SQL_DAY_SECOND_STRUCT;

typedef struct tagSQL_INTERVAL_STRUCT
{
    SQLINTERVAL     interval_type;
    SQLSMALLINT     interval_sign;
    union {
        SQL_YEAR_MONTH_STRUCT       year_month;
        SQL_DAY_SECOND_STRUCT       day_second;
    } intval;

} SQL_INTERVAL_STRUCT;

#endif

#endif

#ifndef ODBCINT64
# if (ODBCVER >= 0x0300)
# if (SIZEOF_LONG_INT == 8)
#   define ODBCINT64        long
#   define UODBCINT64   unsigned long
# else
#  ifdef HAVE_LONG_LONG
#   define ODBCINT64        long long
#   define UODBCINT64   unsigned long long
#  else
/*
 * may fail in some cases, but what else can we do ?
 */
struct __bigint_struct
{
    int             hiword;
    unsigned int    loword;
};
struct __bigint_struct_u
{
    unsigned int    hiword;
    unsigned int    loword;
};
#   define ODBCINT64        struct __bigint_struct
#   define UODBCINT64   struct __bigint_struct_u
#  endif
# endif
#endif
#endif

#ifdef ODBCINT64
typedef ODBCINT64   SQLBIGINT;
#endif
#ifdef UODBCINT64
typedef UODBCINT64  SQLUBIGINT;
#endif


/****************************
 * cursor and bookmark
 ***************************/
#if (ODBCVER >= 0x0300)
#define SQL_MAX_NUMERIC_LEN     16
typedef struct tagSQL_NUMERIC_STRUCT
{
    SQLCHAR     precision;
    SQLSCHAR    scale;
    SQLCHAR     sign;   /* 1=pos 0=neg */
    SQLCHAR     val[SQL_MAX_NUMERIC_LEN];
} SQL_NUMERIC_STRUCT;
#endif

#if (ODBCVER >= 0x0350)
#ifdef GUID_DEFINED
#ifndef ALLREADY_HAVE_WINDOWS_TYPE
typedef GUID    SQLGUID;
#else
typedef struct  tagSQLGUID
{
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[ 8 ];
} SQLGUID;
#endif
#else
typedef struct  tagSQLGUID
{
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[ 8 ];
} SQLGUID;
#endif
#endif

typedef SQLULEN         BOOKMARK;

typedef  WCHAR         SQLWCHAR;

#ifdef UNICODE
typedef SQLWCHAR        SQLTCHAR;
#else
typedef SQLCHAR         SQLTCHAR;
#endif

#ifdef __cplusplus
}
#endif

#endif



