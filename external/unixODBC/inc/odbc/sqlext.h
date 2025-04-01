/*****************************************************
 * sqlext.h
 *
 * These should be consistent with the MS version.
 *
 *****************************************************/
#pragma once



/* BEGIN - unixODBC ONLY (programs like ODBCConfig and DataManager use these) */

/* COLUMNS IN SQLTables() RESULT SET */
 1
 2
 3
 4
 5

/* COLUMNS IN SQLColumns() RESULT SET */
 1
 2
 3
 4
 5
 6
 7
 8
 9
 10
 11
 12
 13
 14
 15
 16
 17
 18

/* END - unixODBC ONLY */

#pragma once
#include "odbc/sql.h"
#endif

#ifdef __cplusplus
extern "C" {                         /* Assume C declarations for C++ */
#endif

/* generally useful constants */
     3        /* Major version of specification  */
     52       /* Minor version of specification  */
   "03.52"   /* String constant for version */

   5   /* size of SQLSTATE */
  32  /* maximum data source name size */

    256

/* return code SQL_NO_DATA_FOUND is the same as SQL_NO_DATA */
#if (ODBCVER < 0x0300)
   100
#else
   SQL_NO_DATA
#endif

/* an end handle type */
#if (ODBCVER >= 0x0300)
     5


/* env attribute */
#if (ODBCVER >= 0x0300)
               200
         201
                   202

/* unixODBC additions */
           65001
           65002
           65003


#if (ODBCVER >= 0x0300)
/* values for SQL_ATTR_CONNECTION_POOLING */
                          0UL
               1UL
                 2UL
                      SQL_CP_OFF

/* values for SQL_ATTR_CP_MATCH */
                 0UL
                1UL
                SQL_CP_STRICT_MATCH

/* values for SQL_ATTR_ODBC_VERSION */
                        2UL
                        3UL


/* connection attributes */
                 101
                  102
               103
                   104
               105
               106
            107
               108
           109
                110
                  111
                 112

/* connection attributes with new names */
#if (ODBCVER >= 0x0300)
        SQL_ACCESS_MODE
         SQL_AUTOCOMMIT
 113
    SQL_CURRENT_QUALIFIER
    114
      1207
       1208
      SQL_LOGIN_TIMEOUT
       SQL_ODBC_CURSORS
        SQL_PACKET_SIZE
         SQL_QUIET_MODE
              SQL_OPT_TRACE
          SQL_OPT_TRACEFILE
      SQL_TRANSLATE_DLL
   SQL_TRANSLATE_OPTION
      SQL_TXN_ISOLATION


    1209    /* GetConnectAttr only */

   1028    /* Driver threading level */

#if (ODBCVER >= 0x0351)
/*  ODBC Driver Manager sets this connection attribute to a unicode driver
    (which supports SQLConnectW) when the application is an ANSI application
    (which calls SQLConnect, SQLDriverConnect, or SQLBrowseConnect).
    This is SetConnectAttr only and application does not set this attribute
    This attribute was introduced because some unicode driver's some APIs may
    need to behave differently on ANSI or Unicode applications. A unicode
    driver, which  has same behavior for both ANSI or Unicode applications,
    should return SQL_ERROR when the driver manager sets this connection
    attribute. When a unicode driver returns SQL_SUCCESS on this attribute,
    the driver manager treates ANSI and Unicode connections differently in
    connection pooling.
*/
           115
#endif

/* SQL_CONNECT_OPT_DRVR_START is not meaningful for 3.0 driver */
#if (ODBCVER < 0x0300)
      1000


#if (ODBCVER < 0x0300)
                SQL_PACKET_SIZE
                SQL_ACCESS_MODE


/* SQL_ACCESS_MODE options */
             0UL
              1UL
                SQL_MODE_READ_WRITE

/* SQL_AUTOCOMMIT options */
              0UL
               1UL
          SQL_AUTOCOMMIT_ON

/* SQL_LOGIN_TIMEOUT options */
       15UL

/* SQL_OPT_TRACE options */
               0UL
                1UL
           SQL_OPT_TRACE_OFF
      "/tmp/SQL.LOG"

/* SQL_ODBC_CURSORS options */
           0UL
                1UL
              2UL
                 SQL_CUR_USE_DRIVER

#if (ODBCVER >= 0x0300)
/* values for SQL_ATTR_DISCONNECT_BEHAVIOR */
           0UL
               1UL
                  SQL_DB_RETURN_TO_POOL

/* values for SQL_ATTR_ENLIST_IN_DTC */
                    0L


/* values for SQL_ATTR_CONNECTION_DEAD */
                 1L      /* Connection is closed/dead */
                0L      /* Connection is open/available */

/* values for SQL_ATTR_ANSI_APP */
#if (ODBCVER >= 0x0351)
                 1L  /* the application is an ANSI app */
                    0L  /* the application is a Unicode app */
#endif

/* statement attributes */
       0
            1
              2
          3
        4   /* same as SQL_ATTR_ASYNC_ENABLE */
           5
         6
         7
         8
         9
     10
       11
       12
        13      /*      GetStmtOption Only */
          14      /*      GetStmtOption Only */

/* statement attributes for ODBC 3.0 */
#if (ODBCVER >= 0x0300)
               4
                SQL_CONCURRENCY
                SQL_CURSOR_TYPE
            15
         16
                SQL_KEYSET_SIZE
                 SQL_MAX_LENGTH
                   SQL_MAX_ROWS
                     SQL_NOSCAN
      17
            18
        19
           20
       21
              22
              SQL_QUERY_TIMEOUT
              SQL_RETRIEVE_DATA
        23
              SQL_BIND_TYPE
                 SQL_ROW_NUMBER      /*GetStmtAttr*/
          24
             25
           26
             27
            SQL_SIMULATE_CURSOR
              SQL_USE_BOOKMARKS



#if (ODBCVER < 0x0300)
                SQL_ROW_NUMBER
    SQL_QUERY_TIMEOUT


/* New defines for SEARCHABLE column in SQLGetTypeInfo */

#if (ODBCVER >= 0x0300)
       SQL_LIKE_ONLY
      SQL_ALL_EXCEPT_LIKE




/* whether an attribute is a pointer or not */
#if (ODBCVER >= 0x0300)
                          (-4)
                         (-5)
                          (-6)
                        (-7)
                         (-8)


/* the value of SQL_ATTR_PARAM_BIND_TYPE */
#if (ODBCVER >= 0x0300)
            0UL
         SQL_PARAM_BIND_BY_COLUMN


/* SQL_QUERY_TIMEOUT options */
       0UL

/* SQL_MAX_ROWS options */
            0UL

/* SQL_NOSCAN options */
                  0UL     /*      1.0 FALSE */
                   1UL     /*      1.0 TRUE */
              SQL_NOSCAN_OFF

/* SQL_MAX_LENGTH options */
          0UL

/* values for SQL_ATTR_ASYNC_ENABLE */
            0UL
             1UL
        SQL_ASYNC_ENABLE_OFF

/* SQL_BIND_TYPE options */
              0UL
           SQL_BIND_BY_COLUMN  /* Default value */

/* SQL_CONCURRENCY options */
            1
                 2
               3
               4
              SQL_CONCUR_READ_ONLY /* Default value */

/* SQL_CURSOR_TYPE options */
         0UL
        1UL
              2UL
               3UL
         SQL_CURSOR_FORWARD_ONLY /* Default value */

/* SQL_ROWSET_SIZE options */
         1UL

/* SQL_KEYSET_SIZE options */
         0UL

/* SQL_SIMULATE_CURSOR options */
               0UL
               1UL
                   2UL

/* SQL_RETRIEVE_DATA options */
                      0UL
                       1UL
                  SQL_RD_ON

/* SQL_USE_BOOKMARKS options */
                      0UL
                       01UL
                  SQL_UB_OFF

/* New values for SQL_USE_BOOKMARKS attribute */
#if (ODBCVER >= 0x0300)
                    SQL_UB_ON
                 2UL


/* extended descriptor field */
#if (ODBCVER >= 0x0300)
                     20
               21
              SQL_COLUMN_AUTO_INCREMENT
               22
                23
                24
                      25
                 SQL_COLUMN_CASE_SENSITIVE
                   SQL_COLUMN_QUALIFIER_NAME
                   SQL_COLUMN_TYPE
    26
                   SQL_COLUMN_DISPLAY_SIZE
               SQL_COLUMN_MONEY
                          SQL_COLUMN_LABEL
                 27
                 28
                29
                  30
                  31
                 32
                 33
             34
#if (ODBCVER >= 0x0350)
                         35

                    SQL_COLUMN_OWNER_NAME
                     SQL_COLUMN_SEARCHABLE
                      SQL_COLUMN_TYPE_NAME
                     SQL_COLUMN_TABLE_NAME
                       SQL_COLUMN_UNSIGNED
                      SQL_COLUMN_UPDATABLE



/* defines for diagnostics fields */
#if (ODBCVER >= 0x0300)
           (-1249)
                 (-1248)
              (-1247)


/* SQL extended datatypes */
                                9
#if (ODBCVER >= 0x0300)
                            10

                                10
                           11
                         (-1)
                              (-2)
                           (-3)
                       (-4)
                              (-5)
                             (-6)
                                 (-7)
#if (ODBCVER >= 0x0350)
                (-11)


#if (ODBCVER >= 0x0300)
/* interval code */
               1
              2
                3
               4
             5
             6
          7
            8
          9
          10
         11
         12
       13

                   (100 + SQL_CODE_YEAR)
                  (100 + SQL_CODE_MONTH)
                    (100 + SQL_CODE_DAY)
                   (100 + SQL_CODE_HOUR)
                 (100 + SQL_CODE_MINUTE)
                 (100 + SQL_CODE_SECOND)
          (100 + SQL_CODE_YEAR_TO_MONTH)
            (100 + SQL_CODE_DAY_TO_HOUR)
          (100 + SQL_CODE_DAY_TO_MINUTE)
          (100 + SQL_CODE_DAY_TO_SECOND)
         (100 + SQL_CODE_HOUR_TO_MINUTE)
         (100 + SQL_CODE_HOUR_TO_SECOND)
       (100 + SQL_CODE_MINUTE_TO_SECOND)

#else
                       (-80)
                      (-81)
              (-82)
                        (-83)
                       (-84)
                     (-85)
                     (-86)
                (-87)
              (-88)
              (-89)
             (-90)
             (-91)
           (-92)



#if (ODBCVER <= 0x0300)
                             (-95)
                     (-96)
                 (-97)
                        SQL_UNICODE
#else
/* The previous definitions for SQL_UNICODE_ are historical and obsolete */

             SQL_WCHAR
     SQL_WVARCHAR
 SQL_WLONGVARCHAR
        SQL_WCHAR
#endif

#if (ODBCVER < 0x0300)
                   SQL_INTERVAL_YEAR
                     SQL_UNICODE_LONGVARCHAR


/* C datatype to SQL datatype mapping      SQL types
                                           ------------------- */
    SQL_CHAR             /* CHAR, VARCHAR, DECIMAL, NUMERIC */
    SQL_INTEGER          /* INTEGER                      */
   SQL_SMALLINT         /* SMALLINT                     */
   SQL_REAL             /* REAL                         */
  SQL_DOUBLE           /* FLOAT, DOUBLE                */
#if (ODBCVER >= 0x0300)
       SQL_NUMERIC

 99

       (-20)
     (-22)

/* C datatype to SQL datatype mapping */
       SQL_DATE
       SQL_TIME
  SQL_TIMESTAMP
#if (ODBCVER >= 0x0300)
                 SQL_TYPE_DATE
                 SQL_TYPE_TIME
            SQL_TYPE_TIMESTAMP
             SQL_INTERVAL_YEAR
            SQL_INTERVAL_MONTH
              SQL_INTERVAL_DAY
             SQL_INTERVAL_HOUR
           SQL_INTERVAL_MINUTE
           SQL_INTERVAL_SECOND
    SQL_INTERVAL_YEAR_TO_MONTH
      SQL_INTERVAL_DAY_TO_HOUR
    SQL_INTERVAL_DAY_TO_MINUTE
    SQL_INTERVAL_DAY_TO_SECOND
   SQL_INTERVAL_HOUR_TO_MINUTE
   SQL_INTERVAL_HOUR_TO_SECOND
 SQL_INTERVAL_MINUTE_TO_SECOND

     SQL_BINARY
        SQL_BIT
#if (ODBCVER >= 0x0300)
   (SQL_BIGINT+SQL_SIGNED_OFFSET)     /* SIGNED BIGINT */
   (SQL_BIGINT+SQL_UNSIGNED_OFFSET)   /* UNSIGNED BIGINT */

    SQL_TINYINT
      (SQL_C_LONG+SQL_SIGNED_OFFSET)    /* SIGNED INTEGER  */
     (SQL_C_SHORT+SQL_SIGNED_OFFSET)   /* SIGNED SMALLINT */
   (SQL_TINYINT+SQL_SIGNED_OFFSET)   /* SIGNED TINYINT  */
      (SQL_C_LONG+SQL_UNSIGNED_OFFSET)  /* UNSIGNED INTEGER*/
     (SQL_C_SHORT+SQL_UNSIGNED_OFFSET) /* UNSIGNED SMALLINT*/
   (SQL_TINYINT+SQL_UNSIGNED_OFFSET) /* UNSIGNED TINYINT*/

#if (ODBCVER >= 0x0300) && (SIZEOF_LONG_INT == 8) && !defined(BUILD_LEGACY_64_BIT_MODE)
   SQL_C_UBIGINT                     /* BOOKMARK        */
#else
   SQL_C_ULONG                       /* BOOKMARK        */
#endif

#if (ODBCVER >= 0x0350)
  SQL_GUID


                   0
#if (ODBCVER < 0x0300)
                    SQL_BIT
                    SQL_VARCHAR
#endif

#if (ODBCVER >= 0x0300)
       SQL_C_BINARY


/* define for SQL_DIAG_ROW_NUMBER and SQL_DIAG_COLUMN_NUMBER */
#if (ODBCVER >= 0x0300)
                       (-1)
                    (-1)
                  (-2)
               (-2)
#endif

/* SQLBindParameter extensions */
            (-5)
                   (-6)
#if (ODBCVER >= 0x0300)
           SQL_IGNORE

  (-100)
(length) (-(length)+SQL_LEN_DATA_AT_EXEC_OFFSET)

/* binary length for driver specific attributes */
   (-100)
(length)  (-(length)+SQL_LEN_BINARY_ATTR_OFFSET)

/* Defines used by Driver Manager when mapping SQLSetParam to SQLBindParameter
*/
           SQL_PARAM_INPUT_OUTPUT
           (-1L)

/* SQLColAttributes defines */
                0
                 1
                 2
               3
            4
                5
         6
             7
             8
                9
            10
       11
       12
           13
            14
           15
           16
       17
                18
              SQL_COLUMN_LABEL
#if (ODBCVER < 0x0300)
         1000


              SQL_COLUMN_COUNT

/* SQLColAttributes subdefines for SQL_COLUMN_UPDATABLE */
               0
                  1
      2

/* SQLColAttributes subdefines for SQL_COLUMN_SEARCHABLE */
/* These are also used by SQLGetInfo                     */
                0
                   1
             2
                  3
             SQL_SEARCHABLE


/* Special return values for SQLGetData */
                    (-4)

/********************************************/
/* SQLGetFunctions: additional values for   */
/* Function to represent functions that    */
/* are not in the X/Open spec.              */
/********************************************/

#if (ODBCVER >= 0x0300)
   73
   24

    72
    55
    6
 56
    58
    41
          71
    59
      60
      61
        62
        63
     64
      65
 66
       67
           68
 69
  70

/*-------------------------------------------*/
/* SQL_EXT_API_LAST is not useful with ODBC  */
/* version 3.0 because some of the values    */
/* from X/Open are in the 10000 range.       */
/*-------------------------------------------*/

#if (ODBCVER < 0x0300)
            SQL_API_SQLBINDPARAMETER
           23
           40
 (SQL_EXT_API_LAST-SQL_EXT_API_START+1)
#endif

/*--------------------------------------------*/
/* SQL_API_ALL_FUNCTIONS returns an array     */
/* of 'booleans' representing whether a       */
/* function is implemented by the driver.     */
/*                                            */
/* CAUTION: Only functions defined in ODBC    */
/* version 2.0 and earlier are returned, the  */
/* new high-range function numbers defined by */
/* X/Open break this scheme.   See the new    */
/* method -- SQL_API_ODBC3_ALL_FUNCTIONS      */
/*--------------------------------------------*/

       0       /* See CAUTION above */

/*----------------------------------------------*/
/* 2.X drivers export a dummy function with     */
/* ordinal number SQL_API_LOADBYORDINAL to speed*/
/* loading under the windows operating system.  */
/*                      */
/* CAUTION: Loading by ordinal is not supported */
/* for 3.0 and above drivers.           */
/*----------------------------------------------*/

       199     /* See CAUTION above */

/*----------------------------------------------*/
/* SQL_API_ODBC3_ALL_FUNCTIONS                  */
/* This returns a bitmap, which allows us to    */
/* handle the higher-valued function numbers.   */
/* Use  SQL_FUNC_EXISTS(bitmap,function_number) */
/* to determine if the function exists.         */
/*----------------------------------------------*/


#if (ODBCVER >= 0x0300)
 999
    250     /* array of 250 words */


(pfExists, uwAPI) ((*(((UWORD*) (pfExists)) + ((uwAPI) >> 4)) & (1 << ((uwAPI) & 0x000F)) ) ? SQL_TRUE : SQL_FALSE )




/************************************************/
/* Extended definitions for SQLGetInfo          */
/************************************************/

/*---------------------------------*/
/* Values in ODBC 2.0 that are not */
/* in the X/Open spec              */
/*---------------------------------*/

                       0
               0  /* MAX_DRIVER_CONNECTIONS */
                1  /* MAX_CONCURRENT_ACTIVITIES */
                      3
                      4
                     5
                      6
                       7
             9
                        10
                     11
        12
            15
                      21
            22
        24
          27
              32  /* MAX_SCHEMA_NAME_LEN */
          33
          34  /* MAX_CATALOG_NAME_LEN */
                36
             37
                     38
                      39
                  40
        41
                  42
                  44
                      45
               48
               49
                50
                51
              52
                  53
                  54
                     55
                    56
                    57
                 58
                  59
                   60
                 61
             62
                 63
                    64
                65
                    66
               67
                 68
               69
                 70
           71
                    173
                73      /* SQL_INTEGRITY */
                74
            75
                     76
                 77
                      78
                  79
           80
            82
              83
                      84
                    87
                        88
                        89
                     91
                 92
          93
                      95
                           96
      103
            108
          109
         110
              111
          112
              113
              114

#if (ODBCVER >= 0x0201 && ODBCVER < 0x0300)
#pragma once
         65003  /* Temp value until ODBC 3.0 */
#endif


/*----------------------------------------------*/
/* SQL_INFO_LAST and SQL_INFO_DRIVER_START are  */
/* not useful anymore, because  X/Open has      */
/* values in the 10000 range.   You             */
/* must contact X/Open directly to get a range  */
/* of numbers for driver-specific values.       */
/*----------------------------------------------*/

#if (ODBCVER < 0x0300)
                       SQL_QUALIFIER_LOCATION
               1000


/*-----------------------------------------------*/
/* ODBC 3.0 SQLGetInfo values that are not part  */
/* of the X/Open standard at this time.   X/Open */
/* standard values are in sql.h.                 */
/*-----------------------------------------------*/

#if (ODBCVER >= 0x0300)
                 116
                        117

                     118
                   119

                          10021   /* new X/Open spec */
                     120
                       121
                    SQL_QUALIFIER_LOCATION
              SQL_QUALIFIER_NAME_SEPARATOR
                        SQL_QUALIFIER_TERM
                       SQL_QUALIFIER_USAGE
                       122
           123
         124
                125
                    126
                    127
                128
                    129
                       130
                       131
                        132
                  133
                         134
                        135
                      136
                  137
                      138
                         139
                         140
                          141
                    142
                           143
          144
          145
     146
     147
                      148
                   149
           150
           151
     10022   /* new X/Open spec */
          152
              153
                 154
                         SQL_OWNER_TERM
                        SQL_OWNER_USAGE
            155
       156
       157
                         158
       159
                    160
     161
                        162
         163
              164
             165
            166
           167
           168

                 169
                           170
                              171
                    172
                     SQL_UNION


                 1750

/* SQL_ALTER_TABLE bitmasks */
#if (ODBCVER >= 0x0300)
/* the following 5 bitmasks are defined in sql.h
*                      0x00000001L
*                     0x00000002L
*                  0x00000008L
*/
                0x00000020L
               0x00000040L
             0x00000080L
               0x00000100L
              0x00000200L
              0x00000400L
             0x00000800L
             0x00001000L
    0x00002000L
   0x00004000L
       0x00008000L
    0x00010000L
   0x00020000L
            0x00040000L
        0x00080000L


/* SQL_CONVERT_*  return value bitmasks */

                        0x00000001L
                     0x00000002L
                     0x00000004L
                     0x00000008L
                    0x00000010L
                       0x00000020L
                        0x00000040L
                      0x00000080L
                     0x00000100L
                 0x00000200L
                      0x00000400L
                   0x00000800L
                         0x00001000L
                     0x00002000L
                      0x00004000L
                        0x00008000L
                        0x00010000L
                   0x00020000L
               0x00040000L
#if (ODBCVER >= 0x0300)
         0x00080000L
           0x00100000L
                       0x00200000L
                0x00400000L
                    0x00800000L
                        0x01000000L




/* SQL_CONVERT_FUNCTIONS functions */
                  0x00000001L
#if (ODBCVER >= 0x0300)
                     0x00000002L



/* SQL_STRING_FUNCTIONS functions */

                   0x00000001L
                   0x00000002L
                     0x00000004L
                    0x00000008L
                   0x00000010L
                   0x00000020L
                    0x00000040L
                   0x00000080L
                  0x00000100L
                    0x00000200L
                    0x00000400L
                0x00000800L
                    0x00001000L
                    0x00002000L
                     0x00004000L
               0x00008000L
                 0x00010000L
                  0x00020000L
                    0x00040000L
#if (ODBCVER >= 0x0300)
               0x00080000L
              0x00100000L
         0x00200000L
             0x00400000L
                 0x00800000L


/* SQL_SQL92_STRING_FUNCTIONS */
#if (ODBCVER >= 0x0300)
                     0x00000001L
                       0x00000002L
                       0x00000004L
                   0x00000008L
                   0x00000010L
                   0x00000020L
                0x00000040L
               0x00000080L


/* SQL_NUMERIC_FUNCTIONS functions */

                      0x00000001L
                     0x00000002L
                     0x00000004L
                     0x00000008L
                    0x00000010L
                  0x00000020L
                      0x00000040L
                      0x00000080L
                      0x00000100L
                    0x00000200L
                      0x00000400L
                      0x00000800L
                     0x00001000L
                      0x00002000L
                     0x00004000L
                      0x00008000L
                       0x00010000L
                     0x00020000L
                  0x00040000L
                    0x00080000L
                    0x00100000L
                  0x00200000L
                    0x00400000L
                 0x00800000L

/* SQL_SQL92_NUMERIC_VALUE_FUNCTIONS */
#if (ODBCVER >= 0x0300)
                 0x00000001L
                0x00000002L
           0x00000004L
                    0x00000008L
               0x00000010L
                   0x00000020L


/* SQL_TIMEDATE_FUNCTIONS functions */

                       0x00000001L
                   0x00000002L
                0x00000004L
                 0x00000008L
                 0x00000010L
                     0x00000020L
                   0x00000040L
                      0x00000080L
                      0x00000100L
                   0x00000200L
                      0x00000400L
                    0x00000800L
                    0x00001000L
              0x00002000L
             0x00004000L
                   0x00008000L
                 0x00010000L
#if (ODBCVER >= 0x0300)
              0x00020000L
              0x00040000L
         0x00080000L
                   0x00100000L


/* SQL_SQL92_DATETIME_FUNCTIONS */
#if (ODBCVER >= 0x0300)
                0x00000001L
                0x00000002L
           0x00000004L


/* SQL_SYSTEM_FUNCTIONS functions */

                 0x00000001L
                   0x00000002L
                   0x00000004L

/* SQL_TIMEDATE_ADD_INTERVALS and SQL_TIMEDATE_DIFF_INTERVALS functions */

              0x00000001L
                   0x00000002L
                   0x00000004L
                     0x00000008L
                      0x00000010L
                     0x00000020L
                    0x00000040L
                  0x00000080L
                     0x00000100L

/* bitmasks for SQL_DYNAMIC_CURSOR_ATTRIBUTES1,
 * SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1,
 * SQL_KEYSET_CURSOR_ATTRIBUTES1, and SQL_STATIC_CURSOR_ATTRIBUTES1
 */
#if (ODBCVER >= 0x0300)
/* supported SQLFetchScroll FetchOrientation's */
                        0x00000001L
                    0x00000002L
                    0x00000004L
                    0x00000008L

/* supported SQLSetPos LockType's */
              0x00000040L
              0x00000080L
                 0x00000100L

/* supported SQLSetPos Operations */
                0x00000200L
                  0x00000400L
                  0x00000800L
                 0x00001000L

/* positioned updates and deletes */
           0x00002000L
           0x00004000L
           0x00008000L

/* supported SQLBulkOperations operations */
                    0x00010000L
     0x00020000L
     0x00040000L
      0x00080000L


/* bitmasks for SQL_DYNAMIC_CURSOR_ATTRIBUTES2,
 * SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2,
 * SQL_KEYSET_CURSOR_ATTRIBUTES2, and SQL_STATIC_CURSOR_ATTRIBUTES2
 */
#if (ODBCVER >= 0x0300)
/* supported values for SQL_ATTR_SCROLL_CONCURRENCY */
       0x00000001L
            0x00000002L
      0x00000004L
      0x00000008L

/* sensitivity of the cursor to its own inserts, deletes, and updates */
       0x00000010L
       0x00000020L
         0x00000040L

/* semantics of SQL_ATTR_MAX_ROWS */
             0x00000080L
             0x00000100L
             0x00000200L
             0x00000400L
            0x00000800L
        (SQL_CA2_MAX_ROWS_SELECT | SQL_CA2_MAX_ROWS_INSERT | SQL_CA2_MAX_ROWS_DELETE | SQL_CA2_MAX_ROWS_UPDATE | SQL_CA2_MAX_ROWS_CATALOG)

/* semantics of SQL_DIAG_CURSOR_ROW_COUNT */
                   0x00001000L
             0x00002000L

/* the kinds of positioned statements that can be simulated */
         0x00004000L
         0x00008000L
             0x00010000L


/* SQL_ODBC_API_CONFORMANCE values */

                        0x0000
                      0x0001
                      0x0002

/* SQL_ODBC_SAG_CLI_CONFORMANCE values */

              0x0000
                  0x0001

/* SQL_ODBC_SQL_CONFORMANCE values */

                     0x0000
                        0x0001
                    0x0002


/* SQL_CONCAT_NULL_BEHAVIOR values */

                         0x0000
                     0x0001

/* SQL_SCROLL_OPTIONS masks */

                 0x00000001L
                0x00000002L
                      0x00000004L
                        0x00000008L
                       0x00000010L

/* SQL_FETCH_DIRECTION masks */

/* SQL_FETCH_RESUME is no longer supported
                 0x00000040L
*/
               0x00000080L

/* SQL_TXN_ISOLATION_OPTION masks */
/* SQL_TXN_VERSIONING is no longer supported
                  0x00000010L
*/

/* SQL_CORRELATION_NAME values */

                         0x0000
                    0x0001
                          0x0002

/* SQL_NON_NULLABLE_COLUMNS values */

                        0x0000
                    0x0001

/* SQL_NULL_COLLATION values */

                        0x0002
                          0x0004

/* SQL_FILE_USAGE values */

              0x0000
                      0x0001
                  0x0002
                    SQL_FILE_QUALIFIER  /* ODBC 3.0 */


/* SQL_GETDATA_EXTENSIONS values */

                        0x00000004L
                        0x00000008L

/* SQL_POSITIONED_STATEMENTS masks */

            0x00000001L
            0x00000002L
            0x00000004L

/* SQL_GROUP_BY values */

                0x0000
       0x0001
     0x0002
                  0x0003
#if (ODBCVER >= 0x0300)
                      0x0004



/* SQL_OWNER_USAGE masks */

               0x00000001L
         0x00000002L
             0x00000004L
             0x00000008L
         0x00000010L

/* SQL_SCHEMA_USAGE masks */
#if (ODBCVER >= 0x0300)
           SQL_OU_DML_STATEMENTS
     SQL_OU_PROCEDURE_INVOCATION
         SQL_OU_TABLE_DEFINITION
         SQL_OU_INDEX_DEFINITION
     SQL_OU_PRIVILEGE_DEFINITION


/* SQL_QUALIFIER_USAGE masks */

               0x00000001L
         0x00000002L
             0x00000004L
             0x00000008L
         0x00000010L

#if (ODBCVER >= 0x0300)
/* SQL_CATALOG_USAGE masks */
           SQL_QU_DML_STATEMENTS
     SQL_QU_PROCEDURE_INVOCATION
         SQL_QU_TABLE_DEFINITION
         SQL_QU_INDEX_DEFINITION
     SQL_QU_PRIVILEGE_DEFINITION


/* SQL_SUBQUERIES masks */

                   0x00000001L
                       0x00000002L
                           0x00000004L
                   0x00000008L
        0x00000010L

/* SQL_UNION masks */

                         0x00000001L
                     0x00000002L

/* SQL_BOOKMARK_PERSISTENCE values */

                        0x00000001L
                       0x00000002L
                         0x00000004L
                  0x00000008L
                       0x00000010L
                  0x00000020L
                       0x00000040L

/* SQL_STATIC_SENSITIVITY values */

                    0x00000001L
                    0x00000002L
                      0x00000004L

/* SQL_VIEW values */
                  0x00000001L
                 0x00000002L
                     0x00000004L
                        0x00000008L

/* SQL_LOCK_TYPES masks */

                   0x00000001L
                   0x00000002L
                      0x00000004L

/* SQL_POS_OPERATIONS masks */

                    0x00000001L
                     0x00000002L
                      0x00000004L
                      0x00000008L
                         0x00000010L

/* SQL_QUALIFIER_LOCATION values */

                        0x0001
                          0x0002

/* Here start return values for ODBC 3.0 SQLGetInfo */

#if (ODBCVER >= 0x0300)
/* SQL_AGGREGATE_FUNCTIONS bitmasks */
                      0x00000001L
                    0x00000002L
                      0x00000004L
                      0x00000008L
                      0x00000010L
                 0x00000020L
                      0x00000040L

/* SQL_SQL_CONFORMANCE bit masks */
              0x00000001L
   0x00000002L
       0x00000004L
               0x00000008L

/* SQL_DATETIME_LITERALS masks */
                       0x00000001L
                       0x00000002L
                  0x00000004L
              0x00000008L
             0x00000010L
               0x00000020L
              0x00000040L
            0x00000080L
            0x00000100L
     0x00000200L
       0x00000400L
     0x00000800L
     0x00001000L
    0x00002000L
    0x00004000L
  0x00008000L

/* SQL_CATALOG_LOCATION values */
                        SQL_QL_START
                          SQL_QL_END

/* values for SQL_BATCH_ROW_COUNT */
          0x0000001
            0x0000002
           0x0000004

/* bitmasks for SQL_BATCH_SUPPORT */
              0x00000001L
           0x00000002L
                  0x00000004L
               0x00000008L

/* Values for SQL_PARAM_ARRAY_ROW_COUNTS getinfo */
      1
   2

/* values for SQL_PARAM_ARRAY_SELECTS */
               1
            2
           3

/* Bitmasks for SQL_INDEX_KEYWORDS */
                         0x00000000L
                          0x00000001L
                         0x00000002L
                          (SQL_IK_ASC | SQL_IK_DESC)

/* Bitmasks for SQL_INFO_SCHEMA_VIEWS */

                  0x00000001L
              0x00000002L
           0x00000004L
                  0x00000008L
         0x00000010L
           0x00000020L
                     0x00000040L
     0x00000080L
      0x00000100L
          0x00000200L
                     0x00000400L
            0x00000800L
     0x00001000L
                    0x00002000L
               0x00004000L
           0x00008000L
            0x00010000L
                      0x00020000L
                0x00040000L
            0x00080000L
           0x00100000L
            0x00200000L
                       0x00400000L

/* Bitmasks for SQL_ASYNC_MODE */

         0
   1
    2

/* Bitmasks for SQL_ALTER_DOMAIN */
           0x00000001L
                0x00000002L
               0x00000004L
                   0x00000008L
                  0x00000010L
    0x00000020L
   0x00000040L
            0x00000080L
        0x00000100L


/* SQL_CREATE_SCHEMA bitmasks */
                0x00000001L
                0x00000002L
        0x00000004L

/* SQL_CREATE_TRANSLATION bitmasks */
          0x00000001L

/* SQL_CREATE_ASSERTION bitmasks */
                 0x00000001L
    0x00000010L
   0x00000020L
            0x00000040L
        0x00000080L

/* SQL_CREATE_CHARACTER_SET bitmasks */
        0x00000001L
              0x00000002L
           0x00000004L

/* SQL_CREATE_COLLATION bitmasks */
           0x00000001L

/* SQL_CREATE_DOMAIN bitmasks */
                   0x00000001L
                         0x00000002L
                      0x00000004L
                       0x00000008L
      0x00000010L
   0x00000020L
  0x00000040L
           0x00000080L
       0x00000100L

/* SQL_CREATE_TABLE bitmasks */
                     0x00000001L
                  0x00000002L
                    0x00000004L
                 0x00000008L
                  0x00000010L
    0x00000020L
   0x00000040L
            0x00000080L
        0x00000100L
                0x00000200L
                   0x00000400L
                 0x00000800L
                 0x00001000L
       0x00002000L

/* SQL_DDL_INDEX bitmasks */
                     0x00000001L
                       0x00000002L

/* SQL_DROP_COLLATION bitmasks */
                   0x00000001L

/* SQL_DROP_DOMAIN bitmasks */
                      0x00000001L
                         0x00000002L
                          0x00000004L

/* SQL_DROP_SCHEMA bitmasks */
                      0x00000001L
                         0x00000002L
                          0x00000004L

/* SQL_DROP_CHARACTER_SET bitmasks */
              0x00000001L

/* SQL_DROP_ASSERTION bitmasks */
                   0x00000001L

/* SQL_DROP_TABLE bitmasks */
                       0x00000001L
                         0x00000002L
                          0x00000004L

/* SQL_DROP_TRANSLATION bitmasks */
                0x00000001L

/* SQL_DROP_VIEW bitmasks */
                        0x00000001L
                         0x00000002L
                          0x00000004L

/* SQL_INSERT_STATEMENT bitmasks */
                  0x00000001L
                  0x00000002L
                      0x00000004L

/* SQL_ODBC_INTERFACE_CONFORMANCE values */
                            1UL
                          2UL
                          3UL

/* SQL_SQL92_FOREIGN_KEY_DELETE_RULE bitmasks */
                        0x00000001L
                      0x00000002L
                    0x00000004L
                       0x00000008L

/* SQL_SQL92_FOREIGN_KEY_UPDATE_RULE bitmasks */
                        0x00000001L
                      0x00000002L
                    0x00000004L
                       0x00000008L

/* SQL_SQL92_GRANT  bitmasks */
                  0x00000001L
           0x00000002L
               0x00000004L
             0x00000008L
                0x00000010L
                     0x00000020L
                     0x00000040L
                    0x00000080L
                 0x00000100L
                0x00000200L
                     0x00000400L
                     0x00000800L
                    0x00001000L

/* SQL_SQL92_PREDICATES bitmasks */
                           0x00000001L
                        0x00000002L
                           0x00000004L
                       0x00000008L
                    0x00000010L
                0x00000020L
             0x00000040L
                         0x00000080L
                           0x00000100L
                             0x00000200L
                               0x00000400L
                          0x00000800L
                       0x00001000L
            0x00002000L

/* SQL_SQL92_RELATIONAL_JOIN_OPERATORS bitmasks */
           0x00000001L
                     0x00000002L
                    0x00000004L
                0x00000008L
                     0x00000010L
                 0x00000020L
                0x00000040L
                   0x00000080L
               0x00000100L
                     0x00000200L

/* SQL_SQL92_REVOKE bitmasks */
                  0x00000001L
           0x00000002L
               0x00000004L
             0x00000008L
                 0x00000010L
                          0x00000020L
                         0x00000040L
                     0x00000080L
                     0x00000100L
                    0x00000200L
                 0x00000400L
                0x00000800L
                     0x00001000L
                     0x00002000L
                    0x00004000L

/* SQL_SQL92_ROW_VALUE_CONSTRUCTOR bitmasks */
               0x00000001L
                           0x00000002L
                        0x00000004L
                   0x00000008L

/* SQL_SQL92_VALUE_EXPRESSIONS bitmasks */
                            0x00000001L
                            0x00000002L
                        0x00000004L
                          0x00000008L

/* SQL_STANDARD_CLI_CONFORMANCE bitmasks */
              0x00000001L
                       0x00000002L

/* SQL_UNION_STATEMENT bitmasks */
                            SQL_U_UNION
                        SQL_U_UNION_ALL



/* SQL_DTC_TRANSITION_COST bitmasks */
                0x00000001L
              0x00000002L

/* additional SQLDataSources fetch directions */
#if (ODBCVER >= 0x0300)
                31
              32



/* Defines for SQLSetPos */
            0

/* Operations in SQLSetPos */
                 0               /*      1.0 FALSE */
                  1               /*      1.0 TRUE */
                   2
                   3

/* Operations in SQLBulkOperations */
                      4
         SQL_ADD
#if (ODBCVER >= 0x0300)
       5
       6
        7



/* Lock options in SQLSetPos */
           0               /*      1.0 FALSE */
           1               /*      1.0 TRUE */
              2

       SQL_LOCK_UNLOCK

/* Macros for SQLSetPos */
(hstmt,irow) SQLSetPos(hstmt,irow,SQL_POSITION,SQL_LOCK_NO_CHANGE)
(hstmt,irow,fLock) SQLSetPos(hstmt,irow,SQL_POSITION,fLock)
(hstmt,irow,fLock) SQLSetPos(hstmt,irow,SQL_REFRESH,fLock)
(hstmt,irow) SQLSetPos(hstmt,irow,SQL_UPDATE,SQL_LOCK_NO_CHANGE)
(hstmt,irow) SQLSetPos(hstmt,irow,SQL_DELETE,SQL_LOCK_NO_CHANGE)
(hstmt,irow) SQLSetPos(hstmt,irow,SQL_ADD,SQL_LOCK_NO_CHANGE)

/* Column types and scopes in SQLSpecialColumns.  */
                  1
                      2

/* Defines for SQLSpecialColumns (returned in the result set)
   SQL_PC_UNKNOWN and SQL_PC_PSEUDO are defined in sql.h */
               1

/* Defines for SQLStatistics */
                       0
                      1

/* Defines for SQLStatistics (returned in the result set)
   SQL_INDEX_CLUSTERED, SQL_INDEX_HASHED, and SQL_INDEX_OTHER are
   defined in sql.h */
                  0


/* Defines for SQLTables */
#if (ODBCVER >= 0x0300)
                "%"
                 "%"
             "%"


/* Options for SQLDriverConnect */
             0
             1
               2
    3

SQLRETURN SQL_API SQLDriverConnect(
    SQLHDBC            hdbc,
    SQLHWND            hwnd,
    SQLCHAR           *szConnStrIn,
    SQLSMALLINT        cbConnStrIn,
    SQLCHAR           *szConnStrOut,
    SQLSMALLINT        cbConnStrOutMax,
    SQLSMALLINT       *pcbConnStrOut,
    SQLUSMALLINT       fDriverCompletion);

/* Level 2 Functions                             */

/* SQLExtendedFetch "fFetchType" values */
               8

/* SQLExtendedFetch "rgfRowStatus" element values */
                  0
                  1
                  2
                    3
                    4
                    5
#if (ODBCVER >= 0x0300)
        6
                  0
                   1
#endif

/* value for SQL_DESC_ARRAY_STATUS_PTR */
#if (ODBCVER >= 0x0300)
               0
     6
                 5
                7
      1

               0
                1


/* Defines for SQLForeignKeys (UPDATE_RULE and DELETE_RULE) */
                      0
                     1
                     2
#if (ODBCVER >= 0x0250)
            3
          4


#if (ODBCVER >= 0x0300)
/* Note that the following are in a different column of SQLForeignKeys than */
/* the previous #defines.   These are for DEFERRABILITY.                    */

          5
         6
          7



/* Defines for SQLBindParameter and
                           SQLProcedureColumns (returned in the result set) */
           0
                  1
           2
                   3
                 4
                 5

/* Defines for SQLProcedures (returned in the result set) */
                   0
                 1
                  2

/*      This define is too large for RC */
 "ABSOLUTE,ACTION,ADA,ADD,ALL,ALLOCATE,ALTER,AND,ANY,ARE,AS,"\
"ASC,ASSERTION,AT,AUTHORIZATION,AVG,"\
"BEGIN,BETWEEN,BIT,BIT_LENGTH,BOTH,BY,CASCADE,CASCADED,CASE,CAST,CATALOG,"\
"CHAR,CHAR_LENGTH,CHARACTER,CHARACTER_LENGTH,CHECK,CLOSE,COALESCE,"\
"COLLATE,COLLATION,COLUMN,COMMIT,CONNECT,CONNECTION,CONSTRAINT,"\
"CONSTRAINTS,CONTINUE,CONVERT,CORRESPONDING,COUNT,CREATE,CROSS,CURRENT,"\
"CURRENT_DATE,CURRENT_TIME,CURRENT_TIMESTAMP,CURRENT_USER,CURSOR,"\
"DATE,DAY,DEALLOCATE,DEC,DECIMAL,DECLARE,DEFAULT,DEFERRABLE,"\
"DEFERRED,DELETE,DESC,DESCRIBE,DESCRIPTOR,DIAGNOSTICS,DISCONNECT,"\
"DISTINCT,DOMAIN,DOUBLE,DROP,"\
"ELSE,END,END-EXEC,ESCAPE,EXCEPT,EXCEPTION,EXEC,EXECUTE,"\
"EXISTS,EXTERNAL,EXTRACT,"\
"FALSE,FETCH,FIRST,FLOAT,FOR,FOREIGN,FORTRAN,FOUND,FROM,FULL,"\
"GET,GLOBAL,GO,GOTO,GRANT,GROUP,HAVING,HOUR,"\
"IDENTITY,IMMEDIATE,IN,INCLUDE,INDEX,INDICATOR,INITIALLY,INNER,"\
"INPUT,INSENSITIVE,INSERT,INT,INTEGER,INTERSECT,INTERVAL,INTO,IS,ISOLATION,"\
"JOIN,KEY,LANGUAGE,LAST,LEADING,LEFT,LEVEL,LIKE,LOCAL,LOWER,"\
"MATCH,MAX,MIN,MINUTE,MODULE,MONTH,"\
"NAMES,NATIONAL,NATURAL,NCHAR,NEXT,NO,NONE,NOT,NULL,NULLIF,NUMERIC,"\
"OCTET_LENGTH,OF,ON,ONLY,OPEN,OPTION,OR,ORDER,OUTER,OUTPUT,OVERLAPS,"\
"PAD,PARTIAL,PASCAL,PLI,POSITION,PRECISION,PREPARE,PRESERVE,"\
"PRIMARY,PRIOR,PRIVILEGES,PROCEDURE,PUBLIC,"\
"READ,REAL,REFERENCES,RELATIVE,RESTRICT,REVOKE,RIGHT,ROLLBACK,ROWS"\
"SCHEMA,SCROLL,SECOND,SECTION,SELECT,SESSION,SESSION_USER,SET,SIZE,"\
"SMALLINT,SOME,SPACE,SQL,SQLCA,SQLCODE,SQLERROR,SQLSTATE,SQLWARNING,"\
"SUBSTRING,SUM,SYSTEM_USER,"\
"TABLE,TEMPORARY,THEN,TIME,TIMESTAMP,TIMEZONE_HOUR,TIMEZONE_MINUTE,"\
"TO,TRAILING,TRANSACTION,TRANSLATE,TRANSLATION,TRIM,TRUE,"\
"UNION,UNIQUE,UNKNOWN,UPDATE,UPPER,USAGE,USER,USING,"\
"VALUE,VALUES,VARCHAR,VARYING,VIEW,WHEN,WHENEVER,WHERE,WITH,WORK,WRITE,"\
"YEAR,ZONE"

SQLRETURN SQL_API SQLBrowseConnect(
    SQLHDBC            hdbc,
    SQLCHAR           *szConnStrIn,
    SQLSMALLINT        cbConnStrIn,
    SQLCHAR           *szConnStrOut,
    SQLSMALLINT        cbConnStrOutMax,
    SQLSMALLINT       *pcbConnStrOut);

#if (ODBCVER >= 0x0300)
SQLRETURN   SQL_API SQLBulkOperations(
    SQLHSTMT            StatementHandle,
    SQLSMALLINT         Operation);


SQLRETURN SQL_API SQLColAttributes(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       icol,
    SQLUSMALLINT       fDescType,
    SQLPOINTER         rgbDesc,
    SQLSMALLINT        cbDescMax,
    SQLSMALLINT       *pcbDesc,
    SQLLEN            *pfDesc);

SQLRETURN SQL_API SQLColumnPrivileges(
    SQLHSTMT           hstmt,
    SQLCHAR           *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR           *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR           *szTableName,
    SQLSMALLINT        cbTableName,
    SQLCHAR           *szColumnName,
    SQLSMALLINT        cbColumnName);

SQLRETURN SQL_API SQLDescribeParam(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       ipar,
    SQLSMALLINT       *pfSqlType,
    SQLULEN           *pcbParamDef,
    SQLSMALLINT       *pibScale,
    SQLSMALLINT       *pfNullable);

SQLRETURN SQL_API SQLExtendedFetch(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       fFetchType,
    SQLLEN             irow,
    SQLULEN            *pcrow,
    SQLUSMALLINT       *rgfRowStatus);

SQLRETURN SQL_API SQLForeignKeys(
    SQLHSTMT           hstmt,
    SQLCHAR           *szPkCatalogName,
    SQLSMALLINT        cbPkCatalogName,
    SQLCHAR           *szPkSchemaName,
    SQLSMALLINT        cbPkSchemaName,
    SQLCHAR           *szPkTableName,
    SQLSMALLINT        cbPkTableName,
    SQLCHAR           *szFkCatalogName,
    SQLSMALLINT        cbFkCatalogName,
    SQLCHAR           *szFkSchemaName,
    SQLSMALLINT        cbFkSchemaName,
    SQLCHAR           *szFkTableName,
    SQLSMALLINT        cbFkTableName);

SQLRETURN SQL_API SQLMoreResults(
    SQLHSTMT           hstmt);

SQLRETURN SQL_API SQLNativeSql(
    SQLHDBC            hdbc,
    SQLCHAR           *szSqlStrIn,
    SQLINTEGER         cbSqlStrIn,
    SQLCHAR           *szSqlStr,
    SQLINTEGER         cbSqlStrMax,
    SQLINTEGER        *pcbSqlStr);

SQLRETURN SQL_API SQLNumParams(
    SQLHSTMT           hstmt,
    SQLSMALLINT       *pcpar);

SQLRETURN SQL_API SQLParamOptions(
    SQLHSTMT           hstmt,
    SQLULEN            crow,
    SQLULEN            *pirow);

SQLRETURN SQL_API SQLPrimaryKeys(
    SQLHSTMT           hstmt,
    SQLCHAR           *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR           *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR           *szTableName,
    SQLSMALLINT        cbTableName);

SQLRETURN SQL_API SQLProcedureColumns(
    SQLHSTMT           hstmt,
    SQLCHAR           *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR           *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR           *szProcName,
    SQLSMALLINT        cbProcName,
    SQLCHAR           *szColumnName,
    SQLSMALLINT        cbColumnName);

SQLRETURN SQL_API SQLProcedures(
    SQLHSTMT           hstmt,
    SQLCHAR           *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR           *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR           *szProcName,
    SQLSMALLINT        cbProcName);

SQLRETURN SQL_API SQLSetPos(
    SQLHSTMT           hstmt,
    SQLSETPOSIROW      irow,
    SQLUSMALLINT       fOption,
    SQLUSMALLINT       fLock);

SQLRETURN SQL_API SQLTablePrivileges(
    SQLHSTMT           hstmt,
    SQLCHAR           *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR           *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR           *szTableName,
    SQLSMALLINT        cbTableName);

SQLRETURN SQL_API SQLDrivers(
    SQLHENV            henv,
    SQLUSMALLINT       fDirection,
    SQLCHAR           *szDriverDesc,
    SQLSMALLINT        cbDriverDescMax,
    SQLSMALLINT       *pcbDriverDesc,
    SQLCHAR           *szDriverAttributes,
    SQLSMALLINT        cbDrvrAttrMax,
    SQLSMALLINT       *pcbDrvrAttr);

SQLRETURN SQL_API SQLBindParameter(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       ipar,
    SQLSMALLINT        fParamType,
    SQLSMALLINT        fCType,
    SQLSMALLINT        fSqlType,
    SQLULEN            cbColDef,
    SQLSMALLINT        ibScale,
    SQLPOINTER         rgbValue,
    SQLLEN             cbValueMax,
    SQLLEN            *pcbValue);

/*---------------------------------------------------------*/
/* SQLAllocHandleStd is implemented to make SQLAllocHandle */
/* compatible with X/Open standard.  an application should */
/* not call SQLAllocHandleStd directly                     */
/*---------------------------------------------------------*/
#ifdef ODBC_STD
  SQLAllocHandleStd
(phenv)  SQLAllocHandleStd(SQL_HANDLE_ENV, SQL_NULL_HANDLE, phenv)

/* Internal type subcodes */
                        SQL_CODE_YEAR
                       SQL_CODE_MONTH
                         SQL_CODE_DAY
                        SQL_CODE_HOUR
                      SQL_CODE_MINUTE
                      SQL_CODE_SECOND
               SQL_CODE_YEAR_TO_MONTH
                 SQL_CODE_DAY_TO_HOUR
               SQL_CODE_DAY_TO_MINUTE
               SQL_CODE_DAY_TO_SECOND
              SQL_CODE_HOUR_TO_MINUTE
              SQL_CODE_HOUR_TO_SECOND
            SQL_CODE_MINUTE_TO_SECOND


#if (ODBCVER >= 0x0300)
SQLRETURN SQL_API SQLAllocHandleStd(
    SQLSMALLINT     fHandleType,
    SQLHANDLE       hInput,
    SQLHANDLE      *phOutput);
#endif

/*      Deprecated defines from prior versions of ODBC */
               16    /* Use SQLGetConnectOption/SQL_CURRENT_QUALIFIER */
               SQL_FD_FETCH_PRIOR
                  SQL_FETCH_PRIOR
            SQL_CONCUR_ROWVER
          SQL_SCCO_OPT_ROWVER
                   SQL_CB_DELETE
                   SQL_CB_DELETE
                    SQL_CB_CLOSE
                    SQL_CB_CLOSE
                 SQL_CB_PRESERVE
                 SQL_CB_PRESERVE
/* SQL_FETCH_RESUME is not supported by 2.0+ drivers
                7
*/
         0L    /*-SQL_CURSOR_FORWARD_ONLY */
        (-1L) /*-SQL_CURSOR_KEYSET_DRIVEN */
              (-2L) /*-SQL_CURSOR_DYNAMIC */
               (-3L) /*-SQL_CURSOR_STATIC */

/*      Deprecated functions from prior versions of ODBC */
SQLRETURN SQL_API SQLSetScrollOptions(    /*      Use SQLSetStmtOptions */
    SQLHSTMT           hstmt,
    SQLUSMALLINT       fConcurrency,
    SQLLEN             crowKeyset,
    SQLUSMALLINT       crowRowset);

/*!
 * \defgroup    Tracing.
 *
 *              unixODBC implements a slight variation of the tracing mechanism used
 *              on MS platforms. The unixODBC method loses the ability to produce trace
 *              output for invalid handles but gains the following;
 *
 *              - better concurrency
 *              - allows tracing to be turned on/off and configured at finer granularity
 *              - hopefully; better performance
 *
 *              unixODBC provides a cross-platform helper library called 'trace' and an
 *              example/default trace plugin called 'odbctrac'. Those writing an ODBC
 *              driver can use the 'trace' helper library (a static library). Those wanting
 *              to create custom trace output can implement a different version of the
 *              'odbctrac' plugin.
 *
 *              The text file driver (odbctxt) included with unixODBC is an example of a
 *              driver using the 'trace' helper library.
 *
 *              The 'trace' library and the example plugin 'odbctrac' are designed to be
 *              portable on all platforms where unixODBC is available and on MS platforms.
 *              This will allow drivers using 'trace' and 'odbctrac' plugin to equilly
 *              portable. On MS platforms - this compliments traditional tracing (mostly
 *              just used by the Driver Manager).
 *
 * \sa          trace
 *              odbctxt
 *              odbctrac
 */
/*@{*/
 1000                                  /*!< Version of trace API                               */
#ifdef UNICODE
RETCODE TraceOpenLogFile(SQLPOINTER,LPWSTR,LPWSTR,DWORD);   /*!< open a trace log file                              */
#else
RETCODE TraceOpenLogFile(SQLPOINTER,LPSTR,LPSTR,DWORD);     /*!< open a trace log file                              */
#endif
RETCODE TraceCloseLogFile(SQLPOINTER);                      /*!< Request to close a trace log                       */
SQLRETURN TraceReturn(SQLPOINTER,SQLRETURN);                /*!< Call to produce trace output upon function return. */
#ifdef __cplusplus
DWORD    TraceVersion();                                    /*!< Returns trace API version                          */
#else
DWORD    TraceVersion(VOID);                                /*!< Returns trace API version                          */
#endif

/* Functions for Visual Studio Analyzer*/
/* to turn on/off tracing or VS events, call TraceVSControl by setting or clearing the following bits  */
            0x00000001L
   0x00000002L

RETCODE TraceVSControl(DWORD);

/* the flags in ODBC_VS_ARGS */
    0x00000001L /* the argument is unicode */
    0x00000002L /* the correlation is unicode */
        0x00000004L /* RetCode field is set */
       0x00000008L /* Stop firing visual studio analyzer events */

typedef struct tagODBC_VS_ARGS {
#ifdef GUID_DEFINED
    const GUID  *pguidEvent;    /* the GUID for event */
#else
    const void  *pguidEvent;    /* the GUID for event */
#endif
    DWORD   dwFlags;        /* flags for the call */
    union {
        WCHAR   *wszArg;
        CHAR    *szArg;
    }u1;
    union {
        WCHAR   *wszCorrelation;
        CHAR    *szCorrelation;
    }u2;
    RETCODE RetCode;
} ODBC_VS_ARGS, *PODBC_VS_ARGS;

VOID    FireVSDebugEvent(PODBC_VS_ARGS);
/*@}*/

#ifdef __cplusplus
}
#endif

/*
 * connection pooling retry times
 */

int ODBCSetTryWaitValue ( DWORD dwValue );
#ifdef __cplusplus
DWORD ODBCGetTryWaitValue ( );
#else
DWORD ODBCGetTryWaitValue ( VOID );
#endif

#pragma once
#include "odbc/sqlucode.h"
#endif

#endif
