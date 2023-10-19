/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>

// = log messages for the JDBC driver

#define STR_LOG_DRIVER_CONNECTING_URL           "jdbcBridge: connecting to URL '$1$'"
inline constexpr OUString STR_LOG_DRIVER_SUCCESS = u"jdbcBridge: success"_ustr;
inline constexpr OUString STR_LOG_CREATE_STATEMENT = u"c$1$: creating statement"_ustr;
inline constexpr OUString STR_LOG_CREATED_STATEMENT_ID = u"c$1$: created statement, statement id: s$2$"_ustr;
inline constexpr OUString STR_LOG_PREPARE_STATEMENT = u"c$1$: preparing statement: $2$"_ustr;
inline constexpr OUString STR_LOG_PREPARED_STATEMENT_ID = u"c$1$: prepared statement, statement id: s$2$"_ustr;
inline constexpr OUString STR_LOG_PREPARE_CALL = u"c$1$: preparing call: $2$"_ustr;
inline constexpr OUString STR_LOG_PREPARED_CALL_ID = u"c$1$: prepared call, statement id: s$2$"_ustr;
inline constexpr OUString STR_LOG_NATIVE_SQL = u"c$1$: native SQL: $2$ -> $3$"_ustr;
inline constexpr OUString STR_LOG_LOADING_DRIVER = u"c$1$: attempting to load driver class $2$"_ustr;
inline constexpr OUString STR_LOG_NO_DRIVER_CLASS = u"c$1$: no Java Driver Class was provided"_ustr;
inline constexpr OUString STR_LOG_CONN_SUCCESS = u"c$1$: success"_ustr;
inline constexpr OUString STR_LOG_NO_SYSTEM_CONNECTION = u"c$1$: JDBC driver did not provide a JDBC connection"_ustr;
inline constexpr OUString STR_LOG_GOT_JDBC_CONNECTION = u"c$1$: obtained a JDBC connection for $2$"_ustr;
inline constexpr OUString STR_LOG_SHUTDOWN_CONNECTION = u"c$1$: shutting down connection"_ustr;
inline constexpr OUString STR_LOG_GENERATED_VALUES = u"s$1$: retrieving generated values"_ustr;
inline constexpr OUString STR_LOG_GENERATED_VALUES_FALLBACK = u"s$1$: getGeneratedValues: falling back to statement: $2$"_ustr;
inline constexpr OUString STR_LOG_EXECUTE_STATEMENT = u"s$1$: going to execute: $2$"_ustr;
inline constexpr OUString STR_LOG_EXECUTE_QUERY = u"s$1$: going to execute query: $2$"_ustr;
inline constexpr OUString STR_LOG_CLOSING_STATEMENT = u"s$1$: closing/disposing statement"_ustr;
inline constexpr OUString STR_LOG_EXECUTE_UPDATE = u"s$1$: going to execute update: $2$"_ustr;
inline constexpr OUString STR_LOG_UPDATE_COUNT = u"s$1$: update count: $2$"_ustr;
inline constexpr OUString STR_LOG_RESULT_SET_CONCURRENCY = u"s$1$: going to set result set concurrency: $2$"_ustr;
inline constexpr OUString STR_LOG_RESULT_SET_TYPE = u"s$1$: going to set result set type: $2$"_ustr;
inline constexpr OUString STR_LOG_FETCH_DIRECTION = u"s$1$: fetch direction: $2$"_ustr;
inline constexpr OUString STR_LOG_FETCH_SIZE = u"s$1$: fetch size: $2$"_ustr;
inline constexpr OUString STR_LOG_SET_ESCAPE_PROCESSING = u"s$1$: going to set escape processing: $2$"_ustr;
inline constexpr OUString STR_LOG_EXECUTING_PREPARED = u"s$1$: executing previously prepared statement"_ustr;
inline constexpr OUString STR_LOG_EXECUTING_PREPARED_UPDATE = u"s$1$: executing previously prepared update statement"_ustr;
inline constexpr OUString STR_LOG_EXECUTING_PREPARED_QUERY = u"s$1$: executing previously prepared query"_ustr;
inline constexpr OUString STR_LOG_STRING_PARAMETER = u"s$1$: parameter no. $2$: type: string; value: $3$"_ustr;
inline constexpr OUString STR_LOG_BOOLEAN_PARAMETER = u"s$1$: parameter no. $2$: type: boolean; value: $3$"_ustr;
inline constexpr OUString STR_LOG_BYTE_PARAMETER = u"s$1$: parameter no. $2$: type: byte; value: $3$"_ustr;
inline constexpr OUString STR_LOG_DATE_PARAMETER = u"s$1$: parameter no. $2$: type: date; value: $3$"_ustr;
inline constexpr OUString STR_LOG_TIME_PARAMETER = u"s$1$: parameter no. $2$: type: time; value: $3$"_ustr;
inline constexpr OUString STR_LOG_TIMESTAMP_PARAMETER = u"s$1$: parameter no. $2$: type: timestamp; value: $3$"_ustr;
inline constexpr OUString STR_LOG_DOUBLE_PARAMETER = u"s$1$: parameter no. $2$: type: double; value: $3$"_ustr;
inline constexpr OUString STR_LOG_FLOAT_PARAMETER = u"s$1$: parameter no. $2$: type: float; value: $3$"_ustr;
inline constexpr OUString STR_LOG_INT_PARAMETER = u"s$1$: parameter no. $2$: type: int; value: $3$"_ustr;
inline constexpr OUString STR_LOG_LONG_PARAMETER = u"s$1$: parameter no. $2$: type: long; value: $3$"_ustr;
inline constexpr OUString STR_LOG_NULL_PARAMETER = u"s$1$: parameter no. $2$: sql-type: $3$; value: null"_ustr;
inline constexpr OUString STR_LOG_OBJECT_NULL_PARAMETER = u"s$1$: parameter no. $2$: setting to null"_ustr;
inline constexpr OUString STR_LOG_SHORT_PARAMETER = u"s$1$: parameter no. $2$: type: short; value: $3$"_ustr;
inline constexpr OUString STR_LOG_BYTES_PARAMETER = u"s$1$: parameter no. $2$: type: byte[]"_ustr;
inline constexpr OUString STR_LOG_CHARSTREAM_PARAMETER = u"s$1$: parameter no. $2$: type: character stream"_ustr;
inline constexpr OUString STR_LOG_BINARYSTREAM_PARAMETER = u"s$1$: parameter no. $2$: type: binary stream"_ustr;
inline constexpr OUString STR_LOG_CLEAR_PARAMETERS = u"s$1$: clearing all parameters"_ustr;
inline constexpr OUString STR_LOG_META_DATA_METHOD = u"c$1$: entering XDatabaseMetaData::$2$"_ustr;
inline constexpr OUString STR_LOG_META_DATA_METHOD_ARG1 = u"c$1$: entering XDatabaseMetaData::$2$( '$3$' )"_ustr;
inline constexpr OUString STR_LOG_META_DATA_METHOD_ARG2 = u"c$1$: entering XDatabaseMetaData::$2$( '$3$', '$4$' )"_ustr;
inline constexpr OUString STR_LOG_META_DATA_METHOD_ARG3 = u"c$1$: entering XDatabaseMetaData::$2$( '$3$', '$4$', '$5$' )"_ustr;
inline constexpr OUString STR_LOG_META_DATA_METHOD_ARG4 = u"c$1$: entering XDatabaseMetaData::$2$( '$3$', '$4$', '$5$', '$6$' )"_ustr;
inline constexpr OUString STR_LOG_META_DATA_RESULT = u"c$1$: leaving XDatabaseMetaData::$2$: success-with-result: $3$"_ustr;
inline constexpr OUString STR_LOG_META_DATA_SUCCESS = u"c$1$: leaving XDatabaseMetaData::$2$: success"_ustr;
#define STR_LOG_THROWING_EXCEPTION              "SQLException to be thrown: message: '$1$', SQLState: $2$, ErrorCode: $3$"
inline constexpr OUString STR_LOG_SETTING_SYSTEM_PROPERTY = u"setting system property \"$1$\" to value \"$2$\""_ustr;

inline constexpr OUString STR_DB_NOT_CONNECTED_STATE = u"08003"_ustr;
inline constexpr OUString STR_DATA_CANNOT_SELECT_UNFILTERED_STATE = u"IM001"_ustr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
