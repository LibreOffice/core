/*************************************************************************
 *
 *  $RCSfile: xmlEnums.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:21:11 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
        XML_TOK_DOC_STYLES
    };
    enum XMLDatabaseToken
    {
        XML_TOK_DATASOURCE,
        XML_TOK_FORMS,
        XML_TOK_REPORTS,
        XML_TOK_QUERIES,
        XML_TOK_TABLES
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
        XML_TOK_FONT_CHARSET
    };
    enum XMLLogin
    {
        XML_TOK_USER_NAME,
        XML_TOK_IS_PASSWORD_REQUIRED
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
        XML_TOK_COLUMN_VISIBILITY
    };
// -----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
#endif // DBA_XMLENUMS_HXX
