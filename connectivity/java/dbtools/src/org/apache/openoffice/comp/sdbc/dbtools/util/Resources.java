/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package org.apache.openoffice.comp.sdbc.dbtools.util;

public class Resources {
    public static final int STR_COMMON_BASE = 1200;
    // = resource ids for log messages
    public static final int STR_LOG_MESSAGE_BASE = 10000;
    public static final int STR_JDBC_LOG_MESSAGE_BASE = STR_LOG_MESSAGE_BASE + 500;

    public static final int STR_STRING_LENGTH_EXCEEDED = (STR_COMMON_BASE + 1);
    public static final int STR_CANNOT_CONVERT_STRING = (STR_COMMON_BASE + 2);
    public static final int STR_URI_SYNTAX_ERROR = (STR_COMMON_BASE + 3);
    public static final int STR_COULD_NOT_LOAD_FILE = (STR_COMMON_BASE + 4);
    public static final int STR_QUERY_TOO_COMPLEX = (STR_COMMON_BASE + 5);
    public static final int STR_OPERATOR_TOO_COMPLEX = (STR_COMMON_BASE + 6);
    public static final int STR_QUERY_INVALID_LIKE_COLUMN = (STR_COMMON_BASE + 7);
    public static final int STR_QUERY_INVALID_LIKE_STRING = (STR_COMMON_BASE + 8);
    public static final int STR_QUERY_NOT_LIKE_TOO_COMPLEX = (STR_COMMON_BASE + 9);
    public static final int STR_QUERY_LIKE_WILDCARD = (STR_COMMON_BASE + 10);
    public static final int STR_QUERY_LIKE_WILDCARD_MANY = (STR_COMMON_BASE + 11);
    public static final int STR_INVALID_COLUMNNAME = (STR_COMMON_BASE + 12);
    public static final int STR_NO_CLASSNAME = (STR_COMMON_BASE + 13);
    public static final int STR_NO_CLASSNAME_PATH = (STR_COMMON_BASE + 14);
    public static final int STR_UNKNOWN_PARA_TYPE = (STR_COMMON_BASE + 15);
    public static final int STR_INVALID_COLUMN_SELECTION = (STR_COMMON_BASE + 16);
    public static final int STR_PARA_ONLY_PREPARED = (STR_COMMON_BASE + 17);
    public static final int STR_COLUMN_NOT_UPDATEABLE = (STR_COMMON_BASE + 18);
    public static final int STR_ROW_ALREADY_DELETED = (STR_COMMON_BASE + 19);
    public static final int STR_UNKNOWN_COLUMN_TYPE = (STR_COMMON_BASE + 20);
    public static final int STR_FORMULA_WRONG = (STR_COMMON_BASE + 21);
    public static final int STR_NO_JAVA = (STR_COMMON_BASE + 22);
    public static final int STR_NO_RESULTSET = (STR_COMMON_BASE + 23);
    public static final int STR_NO_ROWCOUNT = (STR_COMMON_BASE + 24);
    public static final int STR_ERRORMSG_SEQUENCE = (STR_COMMON_BASE + 25);
    public static final int STR_INVALID_INDEX = (STR_COMMON_BASE + 26);
    public static final int STR_UNSUPPORTED_FUNCTION = (STR_COMMON_BASE + 27);
    public static final int STR_UNSUPPORTED_FEATURE = (STR_COMMON_BASE + 28);
    public static final int STR_UNKNOWN_COLUMN_NAME = (STR_COMMON_BASE + 29);
    public static final int STR_INVALID_PARA_COUNT = (STR_COMMON_BASE + 30);
    public static final int STR_PRIVILEGE_NOT_GRANTED = (STR_COMMON_BASE + 31);
    public static final int STR_PRIVILEGE_NOT_REVOKED = (STR_COMMON_BASE + 32);
    public static final int STR_INVALID_BOOKMARK = (STR_COMMON_BASE + 33);
    public static final int STR_NO_ELEMENT_NAME = (STR_COMMON_BASE + 34);
    public static final int STR_NO_INPUTSTREAM = (STR_COMMON_BASE + 35);
    public static final int STR_INPUTSTREAM_WRONG_LEN = (STR_COMMON_BASE + 36);
    public static final int STR_WRONG_PARAM_INDEX = (STR_COMMON_BASE + 37);
    public static final int STR_NO_CONNECTION_GIVEN = (STR_COMMON_BASE + 38);

    public static final int STR_LOG_DRIVER_CONNECTING_URL      = ( STR_JDBC_LOG_MESSAGE_BASE +  1 );
    public static final int STR_LOG_DRIVER_SUCCESS             = ( STR_JDBC_LOG_MESSAGE_BASE +  2 );
    public static final int STR_LOG_CREATE_STATEMENT           = ( STR_JDBC_LOG_MESSAGE_BASE +  3 );
    public static final int STR_LOG_CREATED_STATEMENT_ID       = ( STR_JDBC_LOG_MESSAGE_BASE +  4 );
    public static final int STR_LOG_PREPARE_STATEMENT          = ( STR_JDBC_LOG_MESSAGE_BASE +  5 );
    public static final int STR_LOG_PREPARED_STATEMENT_ID      = ( STR_JDBC_LOG_MESSAGE_BASE +  6 );
    public static final int STR_LOG_PREPARE_CALL               = ( STR_JDBC_LOG_MESSAGE_BASE +  7 );
    public static final int STR_LOG_PREPARED_CALL_ID           = ( STR_JDBC_LOG_MESSAGE_BASE +  8 );
    public static final int STR_LOG_NATIVE_SQL                 = ( STR_JDBC_LOG_MESSAGE_BASE +  9 );
    public static final int STR_LOG_LOADING_DRIVER             = ( STR_JDBC_LOG_MESSAGE_BASE + 10 );
    public static final int STR_LOG_NO_DRIVER_CLASS            = ( STR_JDBC_LOG_MESSAGE_BASE + 11 );
    public static final int STR_LOG_CONN_SUCCESS               = ( STR_JDBC_LOG_MESSAGE_BASE + 12 );
    public static final int STR_LOG_NO_SYSTEM_CONNECTION       = ( STR_JDBC_LOG_MESSAGE_BASE + 13 );
    public static final int STR_LOG_GOT_JDBC_CONNECTION        = ( STR_JDBC_LOG_MESSAGE_BASE + 14 );
    public static final int STR_LOG_SHUTDOWN_CONNECTION        = ( STR_JDBC_LOG_MESSAGE_BASE + 15 );
    public static final int STR_LOG_GENERATED_VALUES            =( STR_JDBC_LOG_MESSAGE_BASE + 16 );
    public static final int STR_LOG_GENERATED_VALUES_FALLBACK  = ( STR_JDBC_LOG_MESSAGE_BASE + 17 );
    public static final int STR_LOG_EXECUTE_STATEMENT          = ( STR_JDBC_LOG_MESSAGE_BASE + 18 );
    public static final int STR_LOG_EXECUTE_QUERY              = ( STR_JDBC_LOG_MESSAGE_BASE + 19 );
    public static final int STR_LOG_CLOSING_STATEMENT          = ( STR_JDBC_LOG_MESSAGE_BASE + 20 );
    public static final int STR_LOG_EXECUTE_UPDATE             = ( STR_JDBC_LOG_MESSAGE_BASE + 21 );
    public static final int STR_LOG_UPDATE_COUNT               = ( STR_JDBC_LOG_MESSAGE_BASE + 22 );
    public static final int STR_LOG_RESULT_SET_CONCURRENCY     = ( STR_JDBC_LOG_MESSAGE_BASE + 23 );
    public static final int STR_LOG_RESULT_SET_TYPE            = ( STR_JDBC_LOG_MESSAGE_BASE + 24 );
    public static final int STR_LOG_FETCH_DIRECTION            = ( STR_JDBC_LOG_MESSAGE_BASE + 25 );
    public static final int STR_LOG_FETCH_SIZE                 = ( STR_JDBC_LOG_MESSAGE_BASE + 26 );
    public static final int STR_LOG_SET_ESCAPE_PROCESSING      = ( STR_JDBC_LOG_MESSAGE_BASE + 27 );
    public static final int STR_LOG_EXECUTING_PREPARED         = ( STR_JDBC_LOG_MESSAGE_BASE + 28 );
    public static final int STR_LOG_EXECUTING_PREPARED_UPDATE  = ( STR_JDBC_LOG_MESSAGE_BASE + 29 );
    public static final int STR_LOG_EXECUTING_PREPARED_QUERY   = ( STR_JDBC_LOG_MESSAGE_BASE + 30 );
    public static final int STR_LOG_STRING_PARAMETER           = ( STR_JDBC_LOG_MESSAGE_BASE + 31 );
    public static final int STR_LOG_BOOLEAN_PARAMETER          = ( STR_JDBC_LOG_MESSAGE_BASE + 32 );
    public static final int STR_LOG_BYTE_PARAMETER             = ( STR_JDBC_LOG_MESSAGE_BASE + 33 );
    public static final int STR_LOG_DATE_PARAMETER             = ( STR_JDBC_LOG_MESSAGE_BASE + 34 );
    public static final int STR_LOG_TIME_PARAMETER             = ( STR_JDBC_LOG_MESSAGE_BASE + 35 );
    public static final int STR_LOG_TIMESTAMP_PARAMETER        = ( STR_JDBC_LOG_MESSAGE_BASE + 36 );
    public static final int STR_LOG_DOUBLE_PARAMETER           = ( STR_JDBC_LOG_MESSAGE_BASE + 37 );
    public static final int STR_LOG_FLOAT_PARAMETER            = ( STR_JDBC_LOG_MESSAGE_BASE + 38 );
    public static final int STR_LOG_INT_PARAMETER              = ( STR_JDBC_LOG_MESSAGE_BASE + 39 );
    public static final int STR_LOG_LONG_PARAMETER             = ( STR_JDBC_LOG_MESSAGE_BASE + 40 );
    public static final int STR_LOG_NULL_PARAMETER             = ( STR_JDBC_LOG_MESSAGE_BASE + 41 );
    public static final int STR_LOG_OBJECT_NULL_PARAMETER      = ( STR_JDBC_LOG_MESSAGE_BASE + 42 );
    public static final int STR_LOG_SHORT_PARAMETER            = ( STR_JDBC_LOG_MESSAGE_BASE + 43 );
    public static final int STR_LOG_BYTES_PARAMETER            = ( STR_JDBC_LOG_MESSAGE_BASE + 44 );
    public static final int STR_LOG_CHARSTREAM_PARAMETER       = ( STR_JDBC_LOG_MESSAGE_BASE + 45 );
    public static final int STR_LOG_BINARYSTREAM_PARAMETER     = ( STR_JDBC_LOG_MESSAGE_BASE + 46 );
    public static final int STR_LOG_CLEAR_PARAMETERS           = ( STR_JDBC_LOG_MESSAGE_BASE + 47 );
    public static final int STR_LOG_META_DATA_METHOD           = ( STR_JDBC_LOG_MESSAGE_BASE + 48 );
    public static final int STR_LOG_META_DATA_METHOD_ARG1      = ( STR_JDBC_LOG_MESSAGE_BASE + 49 );
    public static final int STR_LOG_META_DATA_METHOD_ARG2      = ( STR_JDBC_LOG_MESSAGE_BASE + 50 );
    public static final int STR_LOG_META_DATA_METHOD_ARG3      = ( STR_JDBC_LOG_MESSAGE_BASE + 51 );
    public static final int STR_LOG_META_DATA_METHOD_ARG4      = ( STR_JDBC_LOG_MESSAGE_BASE + 52 );
    public static final int STR_LOG_META_DATA_RESULT           = ( STR_JDBC_LOG_MESSAGE_BASE + 53 );
    public static final int STR_LOG_META_DATA_SUCCESS          = ( STR_JDBC_LOG_MESSAGE_BASE + 54 );
    public static final int STR_LOG_THROWING_EXCEPTION         = ( STR_JDBC_LOG_MESSAGE_BASE + 55 );
    public static final int STR_LOG_SETTING_SYSTEM_PROPERTY    = ( STR_JDBC_LOG_MESSAGE_BASE + 56 );
}
