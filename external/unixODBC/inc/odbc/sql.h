/**************************************************
 * sql.h
 *
 * These should be consistent with the MS version.
 *
 **************************************************/
#pragma once



/****************************
 * default to 3.51 declare something else before here and you get a whole new ball of wax
 ***************************/
#pragma once
 0x0351
#endif

#pragma once
#include "odbc/sqltypes.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************
 * some ret values
 ***************************/
             (-1)
          (-2)
                0
      1
#if (ODBCVER >= 0x0300)
              100
#endif
                 (-1)
        (-2)
        2
             99
(rc) (((rc)&(~1))==0)

/****************************
 * use these to indicate string termination to some function
 ***************************/
                   (-3)
                  (-3L)

/* maximum message length */
   512

/* date/time length constants */
#if (ODBCVER >= 0x0300)
           10
            8  /* add P+1 if precision is nonzero */
      19  /* add P+1 if precision is nonzero */
#endif

/* handle type identifiers */
#if (ODBCVER >= 0x0300)
             1
             2
            3
            4
#endif

/* environment attribute */
#if (ODBCVER >= 0x0300)
    10001
#endif

/* connection attributes */
#if (ODBCVER >= 0x0300)
      10001
   10014


/* statement attributes */
#if (ODBCVER >= 0x0300)
       10010
     10011
       10012
     10013
  (-1)
 (-2)
#endif

/* SQL_ATTR_CURSOR_SCROLLABLE values */
#if (ODBCVER >= 0x0300)
           0
              1


/* identifiers of fields in the SQL descriptor */
#if (ODBCVER >= 0x0300)
                  1001
                   1002
                 1003
       1004
              1005
                  1006
 1007
               1008
          1009
               1010
                   1011
                1012
           1013
             1099
#endif

/* identifiers of fields in the diagnostics area */
#if (ODBCVER >= 0x0300)
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
#endif

/* dynamic function codes */
#if (ODBCVER >= 0x0300)
           3
            4
                   7
       6
   8
       10
          23
          (-1)
          64
          77
     79
           84
          19
         24
     25
         26
            27
            (-2)
            31
            32
      33
             36
 38
 81
                 48
                50
                59
         85
      0
          82


/* SQL data type codes */
    0
            1
         2
         3
         4
        5
           6
            7
          8
#if (ODBCVER >= 0x0300)
        9
#endif
        12

/* One-parameter shortcuts for date/time data types */
#if (ODBCVER >= 0x0300)
      91
      92
 93
#endif

/* Statement attribute values for cursor sensitivity */
#if (ODBCVER >= 0x0300)
     0
     1
       2
#endif

/* GetTypeInfo() request for all data types */
       0

/* Default conversion code for SQLBindCol(), SQLBindParam() and SQLGetData() */
#if (ODBCVER >= 0x0300)
        99
#endif

/* SQLGetData() code indicating that the application row descriptor
 * specifies the data type
 */
#if (ODBCVER >= 0x0300)
      (-99)
#endif

/* SQL date/time type subcodes */
#if (ODBCVER >= 0x0300)
       1
       2
  3
#endif

/* CLI option values */
#if (ODBCVER >= 0x0300)
           0
            1
#endif

/* values of NULLABLE field in descriptor */
        0
        1

/* Value returned by SQLGetTypeInfo() to denote that it is
 * not known whether or not a data type supports null values.
 */
  2

/* Values returned by SQLGetTypeInfo() to show WHERE clause
 * supported
 */
#if (ODBCVER >= 0x0300)
     0
     1
    2
#endif

/* values of UNNAMED field in descriptor */
#if (ODBCVER >= 0x0300)
           0
         1
#endif

/* values of ALLOC_TYPE field in descriptor */
#if (ODBCVER >= 0x0300)
 1
 2
#endif

/* FreeStmt() options */
           0
            1
          2
    3

/* Codes used for FetchOrientation in SQLFetchScroll(),
   and in SQLDataSources()
*/
      1
     2

/* Other codes used for FetchOrientation in SQLFetchScroll() */
      3
     4
  5
  6

/* SQLEndTran() options */
          0
        1

/* null handles returned by SQLAllocHandle() */
       0
       0
      0
#if (ODBCVER >= 0x0300)
      0
       0
#endif

/* null handle used in place of parent handle when allocating HENV */
#if (ODBCVER >= 0x0300)
     NULL
#endif

/* Values that may appear in the result set of SQLSpecialColumns() */
    0
 1
   2

      0
#if (ODBCVER >= 0x0300)
   1
#endif
       2

/* Reserved value for the IdentifierType argument of SQLSpecialColumns() */
#if (ODBCVER >= 0x0300)
  1
#endif

/* Reserved values for UNIQUE argument of SQLStatistics() */
    0
       1

/* Values that may appear in the result set of SQLStatistics() */
 1
    2
     3

/* SQLGetFunctions() values to identify ODBC APIs */
         1
             2
#if (ODBCVER >= 0x0300)
       1001
#endif
            3
              4
#if (ODBCVER >= 0x0300)
         1002
#endif
               5
#if (ODBCVER >= 0x0300)
       1003
         6
#endif
             40
              7
#if (ODBCVER >= 0x0300)
          1004
#endif
         57
          8
           9
#if (ODBCVER >= 0x0300)
           1005
#endif
               10
          11
             12
               13
#if (ODBCVER >= 0x0300)
       1021
#endif
         14
             15
#if (ODBCVER >= 0x0300)
        1006
#endif
            16
#if (ODBCVER >= 0x0300)
    1007
#endif
    42
       17
             43
#if (ODBCVER >= 0x0300)
      1008
        1009
      1010
        1011
        1012
#endif
        44
             45
#if (ODBCVER >= 0x0300)
       1014
#endif
       46
         47
       18
           48
             19
             49
            20
#if (ODBCVER >= 0x0300)
    1016
#endif
    50
       21
#if (ODBCVER >= 0x0300)
      1017
        1018
        1019
#endif
            22
#if (ODBCVER >= 0x0300)
       1020
#endif
       51
      52
          53
              54
            23

/* Information requested by SQLGetInfo() */
#if (ODBCVER >= 0x0300)
           0
      SQL_MAX_DRIVER_CONNECTIONS
        1
   SQL_MAX_CONCURRENT_ACTIVITIES
#endif
                 2
                  8
                     13
           14
                       17
                        18
               19
           20
          23
           25
           26
                 28
           29
             30
      SQL_MAX_COLUMN_NAME_LEN
             31
      SQL_MAX_CURSOR_NAME_LEN
             32
      SQL_MAX_SCHEMA_NAME_LEN
            34
     SQL_MAX_CATALOG_NAME_LEN
              35
              43
                     46
             SQL_TXN_CAPABLE
                       47
            72
    SQL_TXN_ISOLATION_OPTION
                       73
              81
                  85
                     86
      90
              94
         97
     SQL_MAX_COLUMNS_IN_GROUP_BY
            98
        SQL_MAX_COLUMNS_IN_INDEX
         99
     SQL_MAX_COLUMNS_IN_ORDER_BY
          100
      SQL_MAX_COLUMNS_IN_SELECT
           101
                 102
             SQL_MAX_INDEX_SIZE
                   104
               SQL_MAX_ROW_SIZE
              105
       SQL_MAX_STATEMENT_LEN
           106
       SQL_MAX_TABLES_IN_SELECT
              107
       SQL_MAX_USER_NAME_LEN
#if (ODBCVER >= 0x0300)
                115
        SQL_OJ_CAPABILITIES


#if (ODBCVER >= 0x0300)
               10000
           10001
           10002
                 10003
                10004
           10005
    SQL_MAX_IDENTIFIER_LEN


/* SQL_ALTER_TABLE bitmasks */
#if (ODBCVER >= 0x0200)
                       0x00000001L
                      0x00000002L


#if (ODBCVER >= 0x0300)
                   0x00000008L

/* The following bitmasks are ODBC extensions and defined in sqlext.h
*                    0x00000020L
*               0x00000040L
*             0x00000080L
*               0x00000100L
*              0x00000200L
*              0x00000400L
*             0x00000800L
*                0x00001000L
*       0x00002000L
*      0x00004000L
*          0x00008000L
*       0x00010000L
*      0x00020000L
*               0x00040000L
*           0x00080000L
*/



/* SQL_ASYNC_MODE values */
#if (ODBCVER >= 0x0300)
                         0
                   1
                    2
#endif

/* SQL_CURSOR_COMMIT_BEHAVIOR values */
                       0
                        1
                     2

/* SQL_FETCH_DIRECTION bitmasks */
                   0x00000001L
                  0x00000002L
                   0x00000004L
                  0x00000008L
               0x00000010L
               0x00000020L

/* SQL_GETDATA_EXTENSIONS bitmasks */
                   0x00000001L
                    0x00000002L

/* SQL_IDENTIFIER_CASE values */
                        1
                        2
                    3
                        4

/* SQL_OJ_CAPABILITIES bitmasks */
/* NB: this means 'outer join', not what  you may be thinking */


#if (ODBCVER >= 0x0201)
                         0x00000001L
                        0x00000002L
                         0x00000004L
                       0x00000008L
                  0x00000010L
                        0x00000020L
           0x00000040L
#endif

/* SQL_SCROLL_CONCURRENCY bitmasks */
                  0x00000001L
                       0x00000002L
                 0x00000004L
                 0x00000008L

/* SQL_TXN_CAPABLE values */
                         0
                          1
                          2
                   3
                   4

/* SQL_TXN_ISOLATION_OPTION bitmasks */
            0x00000001L
    SQL_TXN_READ_UNCOMMITTED
              0x00000002L
      SQL_TXN_READ_COMMITTED
             0x00000004L
     SQL_TXN_REPEATABLE_READ
                0x00000008L
        SQL_TXN_SERIALIZABLE

/* SQL_NULL_COLLATION values */
                         0
                          1

    SQLRETURN  SQL_API SQLAllocConnect(SQLHENV EnvironmentHandle,
                                       SQLHDBC *ConnectionHandle);

    SQLRETURN  SQL_API SQLAllocEnv(SQLHENV *EnvironmentHandle);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLAllocHandle(SQLSMALLINT HandleType,
                                      SQLHANDLE InputHandle, SQLHANDLE *OutputHandle);
#endif

    SQLRETURN  SQL_API SQLAllocStmt(SQLHDBC ConnectionHandle,
                                    SQLHSTMT *StatementHandle);

    SQLRETURN  SQL_API SQLBindCol(SQLHSTMT StatementHandle,
                                  SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType,
                                  SQLPOINTER TargetValue, SQLLEN BufferLength,
                                  SQLLEN *StrLen_or_Ind);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLBindParam(SQLHSTMT StatementHandle,
                                    SQLUSMALLINT ParameterNumber, SQLSMALLINT ValueType,
                                    SQLSMALLINT ParameterType, SQLULEN LengthPrecision,
                                    SQLSMALLINT ParameterScale, SQLPOINTER ParameterValue,
                                    SQLLEN *StrLen_or_Ind);
#endif

    SQLRETURN  SQL_API SQLCancel(SQLHSTMT StatementHandle);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLCloseCursor(SQLHSTMT StatementHandle);

    SQLRETURN  SQL_API SQLColAttribute(SQLHSTMT StatementHandle,
                                        SQLUSMALLINT ColumnNumber, SQLUSMALLINT FieldIdentifier,
                                        SQLPOINTER CharacterAttribute, SQLSMALLINT BufferLength,
                                        SQLSMALLINT *StringLength, SQLLEN *NumericAttribute
                                        /* spec says (SQLPOINTER) not (SQLEN*) - PAH */ );
                                        /* Ms now say SQLLEN* http://msdn.microsoft.com/library/en-us/odbc/htm/dasdkodbcoverview_64bit.asp - NG */

#endif


    SQLRETURN  SQL_API SQLColumns(SQLHSTMT StatementHandle,
                                  SQLCHAR *CatalogName, SQLSMALLINT NameLength1,
                                  SQLCHAR *SchemaName, SQLSMALLINT NameLength2,
                                  SQLCHAR *TableName, SQLSMALLINT NameLength3,
                                  SQLCHAR *ColumnName, SQLSMALLINT NameLength4);


    SQLRETURN  SQL_API SQLConnect(SQLHDBC ConnectionHandle,
                                  SQLCHAR *ServerName, SQLSMALLINT NameLength1,
                                  SQLCHAR *UserName, SQLSMALLINT NameLength2,
                                  SQLCHAR *Authentication, SQLSMALLINT NameLength3);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLCopyDesc(SQLHDESC SourceDescHandle,
                                   SQLHDESC TargetDescHandle);
#endif

    SQLRETURN  SQL_API SQLDataSources(SQLHENV EnvironmentHandle,
                                      SQLUSMALLINT Direction, SQLCHAR *ServerName,
                                      SQLSMALLINT BufferLength1, SQLSMALLINT *NameLength1,
                                      SQLCHAR *Description, SQLSMALLINT BufferLength2,
                                      SQLSMALLINT *NameLength2);

    SQLRETURN  SQL_API SQLDescribeCol(SQLHSTMT StatementHandle,
                                      SQLUSMALLINT ColumnNumber, SQLCHAR *ColumnName,
                                      SQLSMALLINT BufferLength, SQLSMALLINT *NameLength,
                                      SQLSMALLINT *DataType, SQLULEN *ColumnSize,
                                      SQLSMALLINT *DecimalDigits, SQLSMALLINT *Nullable);

    SQLRETURN  SQL_API SQLDisconnect(SQLHDBC ConnectionHandle);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLEndTran(SQLSMALLINT HandleType, SQLHANDLE Handle,
                                  SQLSMALLINT CompletionType);
#endif

    SQLRETURN  SQL_API SQLError(SQLHENV EnvironmentHandle,
                                SQLHDBC ConnectionHandle, SQLHSTMT StatementHandle,
                                SQLCHAR *Sqlstate, SQLINTEGER *NativeError,
                                SQLCHAR *MessageText, SQLSMALLINT BufferLength,
                                SQLSMALLINT *TextLength);

    SQLRETURN  SQL_API SQLExecDirect(SQLHSTMT StatementHandle,
                                     SQLCHAR *StatementText, SQLINTEGER TextLength);

    SQLRETURN  SQL_API SQLExecute(SQLHSTMT StatementHandle);

    SQLRETURN  SQL_API SQLFetch(SQLHSTMT StatementHandle);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLFetchScroll(SQLHSTMT StatementHandle,
                                      SQLSMALLINT FetchOrientation, SQLLEN FetchOffset);
#endif

    SQLRETURN  SQL_API SQLFreeConnect(SQLHDBC ConnectionHandle);

    SQLRETURN  SQL_API SQLFreeEnv(SQLHENV EnvironmentHandle);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLFreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle);
#endif

    SQLRETURN  SQL_API SQLFreeStmt(SQLHSTMT StatementHandle,
                                   SQLUSMALLINT Option);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLGetConnectAttr(SQLHDBC ConnectionHandle,
                                         SQLINTEGER Attribute, SQLPOINTER Value,
                                         SQLINTEGER BufferLength, SQLINTEGER *StringLength);
#endif

    SQLRETURN  SQL_API SQLGetConnectOption(SQLHDBC ConnectionHandle,
                                           SQLUSMALLINT Option, SQLPOINTER Value);

    SQLRETURN  SQL_API SQLGetCursorName(SQLHSTMT StatementHandle,
                                        SQLCHAR *CursorName, SQLSMALLINT BufferLength,
                                        SQLSMALLINT *NameLength);

    SQLRETURN  SQL_API SQLGetData(SQLHSTMT StatementHandle,
                                  SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType,
                                  SQLPOINTER TargetValue, SQLLEN BufferLength,
                                  SQLLEN *StrLen_or_Ind);

#if (ODBCVER >= 0x0300)
    SQLRETURN   SQLGetDescField(SQLHDESC DescriptorHandle,
                                SQLSMALLINT RecNumber, SQLSMALLINT FieldIdentifier,
                                SQLPOINTER Value, SQLINTEGER BufferLength,
                                SQLINTEGER *StringLength);

    SQLRETURN  SQL_API SQLGetDescRec(SQLHDESC DescriptorHandle,
                                     SQLSMALLINT RecNumber, SQLCHAR *Name,
                                     SQLSMALLINT BufferLength, SQLSMALLINT *StringLength,
                                     SQLSMALLINT *Type, SQLSMALLINT *SubType,
                                     SQLLEN *Length, SQLSMALLINT *Precision,
                                     SQLSMALLINT *Scale, SQLSMALLINT *Nullable);

    SQLRETURN  SQL_API SQLGetDiagField(SQLSMALLINT HandleType, SQLHANDLE Handle,
                                       SQLSMALLINT RecNumber, SQLSMALLINT DiagIdentifier,
                                       SQLPOINTER DiagInfo, SQLSMALLINT BufferLength,
                                       SQLSMALLINT *StringLength);

    SQLRETURN  SQL_API SQLGetDiagRec(SQLSMALLINT HandleType, SQLHANDLE Handle,
                                     SQLSMALLINT RecNumber, SQLCHAR *Sqlstate,
                                     SQLINTEGER *NativeError, SQLCHAR *MessageText,
                                     SQLSMALLINT BufferLength, SQLSMALLINT *TextLength);

    SQLRETURN  SQL_API SQLGetEnvAttr(SQLHENV EnvironmentHandle,
                                     SQLINTEGER Attribute, SQLPOINTER Value,
                                     SQLINTEGER BufferLength, SQLINTEGER *StringLength);


    SQLRETURN  SQL_API SQLGetFunctions(SQLHDBC ConnectionHandle,
                                       SQLUSMALLINT FunctionId, SQLUSMALLINT *Supported);

    SQLRETURN  SQL_API SQLGetInfo(SQLHDBC ConnectionHandle,
                                  SQLUSMALLINT InfoType, SQLPOINTER InfoValue,
                                  SQLSMALLINT BufferLength, SQLSMALLINT *StringLength);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLGetStmtAttr(SQLHSTMT StatementHandle,
                                      SQLINTEGER Attribute, SQLPOINTER Value,
                                      SQLINTEGER BufferLength, SQLINTEGER *StringLength);


    SQLRETURN  SQL_API SQLGetStmtOption(SQLHSTMT StatementHandle,
                                        SQLUSMALLINT Option, SQLPOINTER Value);

    SQLRETURN  SQL_API SQLGetTypeInfo(SQLHSTMT StatementHandle,
                                      SQLSMALLINT DataType);

    SQLRETURN  SQL_API SQLNumResultCols(SQLHSTMT StatementHandle,
                                        SQLSMALLINT *ColumnCount);

    SQLRETURN  SQL_API SQLParamData(SQLHSTMT StatementHandle,
                                    SQLPOINTER *Value);

    SQLRETURN  SQL_API SQLPrepare(SQLHSTMT StatementHandle,
                                  SQLCHAR *StatementText, SQLINTEGER TextLength);

    SQLRETURN  SQL_API SQLPutData(SQLHSTMT StatementHandle,
                                  SQLPOINTER Data, SQLLEN StrLen_or_Ind);

    SQLRETURN  SQL_API SQLRowCount(SQLHSTMT StatementHandle,
                                   SQLLEN *RowCount);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLSetConnectAttr(SQLHDBC ConnectionHandle,
                                         SQLINTEGER Attribute, SQLPOINTER Value,
                                         SQLINTEGER StringLength);


    SQLRETURN  SQL_API SQLSetConnectOption(SQLHDBC ConnectionHandle,
                                           SQLUSMALLINT Option, SQLULEN Value);

    SQLRETURN  SQL_API SQLSetCursorName(SQLHSTMT StatementHandle,
                                        SQLCHAR *CursorName, SQLSMALLINT NameLength);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLSetDescField(SQLHDESC DescriptorHandle,
                                       SQLSMALLINT RecNumber, SQLSMALLINT FieldIdentifier,
                                       SQLPOINTER Value, SQLINTEGER BufferLength);

    SQLRETURN  SQL_API SQLSetDescRec(SQLHDESC DescriptorHandle,
                                     SQLSMALLINT RecNumber, SQLSMALLINT Type,
                                     SQLSMALLINT SubType, SQLLEN Length,
                                     SQLSMALLINT Precision, SQLSMALLINT Scale,
                                     SQLPOINTER Data, SQLLEN *StringLength,
                                     SQLLEN *Indicator);

    SQLRETURN  SQL_API SQLSetEnvAttr(SQLHENV EnvironmentHandle,
                                     SQLINTEGER Attribute, SQLPOINTER Value,
                                     SQLINTEGER StringLength);


    SQLRETURN  SQL_API SQLSetParam(SQLHSTMT StatementHandle,
                                   SQLUSMALLINT ParameterNumber, SQLSMALLINT ValueType,
                                   SQLSMALLINT ParameterType, SQLULEN LengthPrecision,
                                   SQLSMALLINT ParameterScale, SQLPOINTER ParameterValue,
                                   SQLLEN *StrLen_or_Ind);

#if (ODBCVER >= 0x0300)
    SQLRETURN  SQL_API SQLSetStmtAttr(SQLHSTMT StatementHandle,
                                      SQLINTEGER Attribute, SQLPOINTER Value,
                                      SQLINTEGER StringLength);
#endif

    SQLRETURN  SQL_API SQLSetStmtOption(SQLHSTMT StatementHandle,
                                        SQLUSMALLINT Option, SQLULEN Value);

    SQLRETURN  SQL_API SQLSpecialColumns(SQLHSTMT StatementHandle,
                                         SQLUSMALLINT IdentifierType, SQLCHAR *CatalogName,
                                         SQLSMALLINT NameLength1, SQLCHAR *SchemaName,
                                         SQLSMALLINT NameLength2, SQLCHAR *TableName,
                                         SQLSMALLINT NameLength3, SQLUSMALLINT Scope,
                                         SQLUSMALLINT Nullable);

    SQLRETURN  SQL_API SQLStatistics(SQLHSTMT StatementHandle,
                                     SQLCHAR *CatalogName, SQLSMALLINT NameLength1,
                                     SQLCHAR *SchemaName, SQLSMALLINT NameLength2,
                                     SQLCHAR *TableName, SQLSMALLINT NameLength3,
                                     SQLUSMALLINT Unique, SQLUSMALLINT Reserved);

    SQLRETURN   SQL_API SQLTables(SQLHSTMT StatementHandle,
                                  SQLCHAR *CatalogName, SQLSMALLINT NameLength1,
                                  SQLCHAR *SchemaName, SQLSMALLINT NameLength2,
                                  SQLCHAR *TableName, SQLSMALLINT NameLength3,
                                  SQLCHAR *TableType, SQLSMALLINT NameLength4);

    SQLRETURN  SQL_API SQLTransact(SQLHENV EnvironmentHandle,
                                   SQLHDBC ConnectionHandle, SQLUSMALLINT CompletionType);

#ifdef __cplusplus
}
#endif
#endif
