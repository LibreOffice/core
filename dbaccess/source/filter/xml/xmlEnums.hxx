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
#ifndef DBA_XMLENUMS_HXX
#define DBA_XMLENUMS_HXX

#define PROGRESS_BAR_STEP 20

namespace dbaxml
{
    enum XMLDocTokens
    {
        XML_TOK_DOC_AUTOSTYLES,
        XML_TOK_DOC_SETTINGS,
        XML_TOK_DOC_DATABASE,
        XML_TOK_DOC_STYLES,
        XML_TOK_DOC_SCRIPT
    };
    enum XMLDatabaseToken
    {
        XML_TOK_DATASOURCE,
        XML_TOK_FORMS,
        XML_TOK_REPORTS,
        XML_TOK_QUERIES,
        XML_TOK_TABLES,
        XML_TOK_SCHEMA_DEFINITION
    };
    enum XMLDataSource
    {
        XML_TOK_CONNECTION_RESOURCE,
        XML_TOK_SUPPRESS_VERSION_COLUMNS,
        XML_TOK_JAVA_DRIVER_CLASS,
        XML_TOK_EXTENSION,
        XML_TOK_IS_FIRST_ROW_HEADER_LINE,
        XML_TOK_SHOW_DELETED,
        XML_TOK_IS_TABLE_NAME_LENGTH_LIMITED,
        XML_TOK_SYSTEM_DRIVER_SETTINGS,
        XML_TOK_ENABLE_SQL92_CHECK,
        XML_TOK_APPEND_TABLE_ALIAS_NAME,
        XML_TOK_PARAMETER_NAME_SUBSTITUTION,
        XML_TOK_IGNORE_DRIVER_PRIVILEGES,
        XML_TOK_BOOLEAN_COMPARISON_MODE,
        XML_TOK_USE_CATALOG,
        XML_TOK_BASE_DN,
        XML_TOK_MAX_ROW_COUNT,
        XML_TOK_LOGIN,
        XML_TOK_TABLE_FILTER,
        XML_TOK_TABLE_TYPE_FILTER,
        XML_TOK_AUTO_INCREMENT,
        XML_TOK_DELIMITER,
        XML_TOK_DATA_SOURCE_SETTINGS,
        XML_TOK_FONT_CHARSET,
        XML_TOK_ENCODING,
        XML_TOK_DATABASE_DESCRIPTION,
        XML_TOK_COMPOUND_DATABASE,
        XML_TOK_DB_HREF,
        XML_TOK_MEDIA_TYPE,
        XML_TOK_DB_TYPE,
        XML_TOK_HOSTNAME,
        XML_TOK_PORT,
        XML_TOK_LOCAL_SOCKET,
        XML_TOK_DATABASE_NAME,
        XML_TOK_CONNECTION_DATA,
        XML_TOK_DRIVER_SETTINGS,
        XML_TOK_JAVA_CLASSPATH,
        XML_TOK_CHARACTER_SET,
        XML_TOK_APPLICATION_CONNECTION_SETTINGS
    };
    enum XMLDatabaseDescription
    {
        XML_TOK_FILE_BASED_DATABASE,
        XML_TOK_SERVER_DATABASE
    };
    enum XMLLogin
    {
        XML_TOK_USER_NAME,
        XML_TOK_IS_PASSWORD_REQUIRED,
        XML_TOK_USE_SYSTEM_USER,
        XML_TOK_LOGIN_TIMEOUT
    };
    enum XMLDataSourceInfo
    {
        XML_TOK_STRING,
        XML_TOK_FIELD,
        XML_TOK_DECIMAL,
        XML_TOK_THOUSAND,
        XML_TOK_ADDITIONAL_COLUMN_STATEMENT,
        XML_TOK_ROW_RETRIEVING_STATEMENT,
        XML_TOK_DATA_SOURCE_SETTING,
        XML_TOK_DATA_SOURCE_SETTING_VALUE,
        XML_TOK_DATA_SOURCE_SETTING_IS_LIST,
        XML_TOK_DATA_SOURCE_SETTING_TYPE,
        XML_TOK_DATA_SOURCE_SETTING_NAME
    };
    enum XMLDocuments
    {
        XML_TOK_COMPONENT,
        XML_TOK_COMPONENT_COLLECTION,
        XML_TOK_QUERY_COLLECTION,
        XML_TOK_QUERY,
        XML_TOK_TABLE,
        XML_TOK_COLUMN
    };
    enum XMLComponent
    {
        XML_TOK_HREF    ,
        XML_TOK_TYPE    ,
        XML_TOK_SHOW    ,
        XML_TOK_ACTUATE ,
        XML_TOK_AS_TEMPLATE ,
        XML_TOK_COMPONENT_NAME
    };
    enum XMLType
    {
        XML_TYPE_FORMS,
        XML_TYPE_REPORTS,
        XML_TYPE_QUERIES,
        XML_TYPE_TABLES
    };
    enum XMLQueryTable
    {
        XML_TOK_QUERY_NAME,
        XML_TOK_COMMAND,
        XML_TOK_ESCAPE_PROCESSING,
        XML_TOK_FILTER_STATEMENT,
        XML_TOK_ORDER_STATEMENT,
        XML_TOK_UPDATE_TABLE,
        XML_TOK_CATALOG_NAME,
        XML_TOK_SCHEMA_NAME,
        XML_TOK_STYLE_NAME,
        XML_TOK_APPLY_FILTER,
        XML_TOK_APPLY_ORDER,
        XML_TOK_COLUMNS
    };
    enum XMLColumn
    {
        XML_TOK_COLUMN_NAME,
        XML_TOK_COLUMN_STYLE_NAME,
        XML_TOK_COLUMN_HELP_MESSAGE,
        XML_TOK_COLUMN_VISIBILITY,
        XML_TOK_COLUMN_DEFAULT_VALUE,
        XML_TOK_COLUMN_TYPE_NAME,
        XML_TOK_COLUMN_VISIBLE,
        XML_TOK_DEFAULT_CELL_STYLE_NAME
    };
} // namespace dbaxml
#endif // DBA_XMLENUMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
