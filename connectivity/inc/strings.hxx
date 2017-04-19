/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_INC_STRINGS_HXX
#define INCLUDED_CONNECTIVITY_INC_STRINGS_HXX

// = log messages for the JDBC driver

#define STR_LOG_DRIVER_CONNECTING_URL           "jdbcBridge: connecting to URL '$1$'"
#define STR_LOG_DRIVER_SUCCESS                  "jdbcBridge: success"
#define STR_LOG_CREATE_STATEMENT                "c$1$: creating statement"
#define STR_LOG_CREATED_STATEMENT_ID            "c$1$: created statement, statement id: s$2$"
#define STR_LOG_PREPARE_STATEMENT               "c$1$: preparing statement: $2$"
#define STR_LOG_PREPARED_STATEMENT_ID           "c$1$: prepared statement, statement id: s$2$"
#define STR_LOG_PREPARE_CALL                    "c$1$: preparing call: $2$"
#define STR_LOG_PREPARED_CALL_ID                "c$1$: prepared call, statement id: s$2$"
#define STR_LOG_NATIVE_SQL                      "c$1$: native SQL: $2$ -> $3$"
#define STR_LOG_LOADING_DRIVER                  "c$1$: attempting to load driver class $2$"
#define STR_LOG_NO_DRIVER_CLASS                 "c$1$: no Java Driver Class was provided"
#define STR_LOG_CONN_SUCCESS                    "c$1$: success"
#define STR_LOG_NO_SYSTEM_CONNECTION            "c$1$: JDBC driver did not provide a JDBC connection"
#define STR_LOG_GOT_JDBC_CONNECTION             "c$1$: obtained a JDBC connection for $2$"
#define STR_LOG_SHUTDOWN_CONNECTION             "c$1$: shutting down connection"
#define STR_LOG_GENERATED_VALUES                "s$1$: retrieving generated values"
#define STR_LOG_GENERATED_VALUES_FALLBACK       "s$1$: getGeneratedValues: falling back to statement: $2$"
#define STR_LOG_EXECUTE_STATEMENT               "s$1$: going to execute: $2$"
#define STR_LOG_EXECUTE_QUERY                   "s$1$: going to execute query: $2$"
#define STR_LOG_CLOSING_STATEMENT               "s$1$: closing/disposing statement"
#define STR_LOG_EXECUTE_UPDATE                  "s$1$: going to execute update: $2$"
#define STR_LOG_UPDATE_COUNT                    "s$1$: update count: $2$"
#define STR_LOG_RESULT_SET_CONCURRENCY          "s$1$: going to set result set concurrency: $2$"
#define STR_LOG_RESULT_SET_TYPE                 "s$1$: going to set result set type: $2$"
#define STR_LOG_FETCH_DIRECTION                 "s$1$: fetch direction: $2$"
#define STR_LOG_FETCH_SIZE                      "s$1$: fetch size: $2$"
#define STR_LOG_SET_ESCAPE_PROCESSING           "s$1$: going to set escape processing: $2$"
#define STR_LOG_EXECUTING_PREPARED              "s$1$: executing previously prepared statement"
#define STR_LOG_EXECUTING_PREPARED_UPDATE       "s$1$: executing previously prepared update statement"
#define STR_LOG_EXECUTING_PREPARED_QUERY        "s$1$: executing previously prepared query"
#define STR_LOG_STRING_PARAMETER                "s$1$: parameter no. $2$: type: string; value: $3$"
#define STR_LOG_BOOLEAN_PARAMETER               "s$1$: parameter no. $2$: type: boolean; value: $3$"
#define STR_LOG_BYTE_PARAMETER                  "s$1$: parameter no. $2$: type: byte; value: $3$"
#define STR_LOG_DATE_PARAMETER                  "s$1$: parameter no. $2$: type: date; value: $3$"
#define STR_LOG_TIME_PARAMETER                  "s$1$: parameter no. $2$: type: time; value: $3$"
#define STR_LOG_TIMESTAMP_PARAMETER             "s$1$: parameter no. $2$: type: timestamp; value: $3$"
#define STR_LOG_DOUBLE_PARAMETER                "s$1$: parameter no. $2$: type: double; value: $3$"
#define STR_LOG_FLOAT_PARAMETER                 "s$1$: parameter no. $2$: type: float; value: $3$"
#define STR_LOG_INT_PARAMETER                   "s$1$: parameter no. $2$: type: int; value: $3$"
#define STR_LOG_LONG_PARAMETER                  "s$1$: parameter no. $2$: type: long; value: $3$"
#define STR_LOG_NULL_PARAMETER                  "s$1$: parameter no. $2$: sql-type: $3$; value: null"
#define STR_LOG_OBJECT_NULL_PARAMETER           "s$1$: parameter no. $2$: setting to null"
#define STR_LOG_SHORT_PARAMETER                 "s$1$: parameter no. $2$: type: short; value: $3$"
#define STR_LOG_BYTES_PARAMETER                 "s$1$: parameter no. $2$: type: byte[]"
#define STR_LOG_CHARSTREAM_PARAMETER            "s$1$: parameter no. $2$: type: character stream"
#define STR_LOG_BINARYSTREAM_PARAMETER          "s$1$: parameter no. $2$: type: binary stream"
#define STR_LOG_CLEAR_PARAMETERS                "s$1$: clearing all parameters"
#define STR_LOG_META_DATA_METHOD                "c$1$: entering XDatabaseMetaData::$2$"
#define STR_LOG_META_DATA_METHOD_ARG1           "c$1$: entering XDatabaseMetaData::$2$( '$3$' )"
#define STR_LOG_META_DATA_METHOD_ARG2           "c$1$: entering XDatabaseMetaData::$2$( '$3$', '$4$' )"
#define STR_LOG_META_DATA_METHOD_ARG3           "c$1$: entering XDatabaseMetaData::$2$( '$3$', '$4$', '$5$' )"
#define STR_LOG_META_DATA_METHOD_ARG4           "c$1$: entering XDatabaseMetaData::$2$( '$3$', '$4$', '$5$', '$6$' )"
#define STR_LOG_META_DATA_RESULT                "c$1$: leaving XDatabaseMetaData::$2$: success-with-result: $3$"
#define STR_LOG_META_DATA_SUCCESS               "c$1$: leaving XDatabaseMetaData::$2$: success"
#define STR_LOG_THROWING_EXCEPTION              "SQLException to be thrown: message: '$1$', SQLState: $2$, ErrorCode: $3$"
#define STR_LOG_SETTING_SYSTEM_PROPERTY         "setting system property \"$1$\" to value \"$2$\""

#define STR_DB_NOT_CONNECTED_STATE              "08003"
#define STR_DATA_CANNOT_SELECT_UNFILTERED_STATE "IM001"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
