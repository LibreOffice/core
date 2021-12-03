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

// property names

#define PROPERTY_URL "URL"
#define PROPERTY_INFO "Info"
#define PROPERTY_SETTINGS "Settings"
#define PROPERTY_ISPASSWORDREQUIRED "IsPasswordRequired"
#define PROPERTY_TABLEFILTER "TableFilter"
#define PROPERTY_TABLETYPEFILTER "TableTypeFilter"
#define PROPERTY_NAME "Name"
#define PROPERTY_SCHEMANAME "SchemaName"
#define PROPERTY_CATALOGNAME "CatalogName"
#define PROPERTY_PRIVILEGES "Privileges"
#define PROPERTY_ESCAPE_PROCESSING "EscapeProcessing"
#define PROPERTY_COMMAND "Command"
#define PROPERTY_TYPE "Type"
#define PROPERTY_TYPENAME "TypeName"
#define PROPERTY_PRECISION "Precision"
#define PROPERTY_SCALE "Scale"
#define PROPERTY_ISNULLABLE "IsNullable"
#define PROPERTY_ISAUTOINCREMENT "IsAutoIncrement"
#define PROPERTY_ISROWVERSION "IsRowVersion"
#define PROPERTY_DESCRIPTION "Description"
#define PROPERTY_DEFAULTVALUE "DefaultValue"
#define PROPERTY_NUMBERFORMAT "FormatKey"
#define PROPERTY_QUERYTIMEOUT "QueryTimeOut"
#define PROPERTY_MAXFIELDSIZE "MaxFieldSize"
#define PROPERTY_MAXROWS "MaxRows"
#define PROPERTY_CURSORNAME "CursorName"
#define PROPERTY_RESULTSETCONCURRENCY "ResultSetConcurrency"
#define PROPERTY_RESULTSETTYPE "ResultSetType"
#define PROPERTY_FETCHDIRECTION "FetchDirection"
#define PROPERTY_FETCHSIZE "FetchSize"
#define PROPERTY_USEBOOKMARKS "UseBookmarks"
#define PROPERTY_ISSEARCHABLE "IsSearchable"
#define PROPERTY_ISCURRENCY "IsCurrency"
#define PROPERTY_ISSIGNED "IsSigned"
#define PROPERTY_DISPLAYSIZE "DisplaySize"
#define PROPERTY_LABEL "Label"
#define PROPERTY_ISREADONLY "IsReadOnly"
#define PROPERTY_ISWRITABLE "IsWritable"
#define PROPERTY_ISDEFINITELYWRITABLE "IsDefinitelyWritable"
#define PROPERTY_VALUE "Value"
#define PROPERTY_TABLENAME "TableName"
#define PROPERTY_ISCASESENSITIVE "IsCaseSensitive"
#define PROPERTY_SERVICENAME "ServiceName"
#define PROPERTY_ISBOOKMARKABLE "IsBookmarkable"
#define PROPERTY_CANUPDATEINSERTEDROWS "CanUpdateInsertedRows"
#define PROPERTY_ISSET "IsSet"
#define PROPERTY_ISOUTPARAMETER "IsOutParameter"
#define PROPERTY_NUMBERFORMATSSUPPLIER "NumberFormatsSupplier"
#define PROPERTY_ISCALCULATED "IsCalculated"
#define PROPERTY_DATASOURCENAME "DataSourceName"
#define PROPERTY_DATABASE_LOCATION "DatabaseLocation"
#define PROPERTY_CONNECTION_RESOURCE "ConnectionResource"
#define PROPERTY_CONNECTION_INFO "ConnectionInfo"
#define PROPERTY_RESULT_SET "ResultSet"
#define PROPERTY_SELECTION "Selection"
#define PROPERTY_BOOKMARK_SELECTION "BookmarkSelection"
#define PROPERTY_COLUMN_NAME "ColumnName"
#define PROPERTY_COLUMN "Column"
#define PROPERTY_DATASOURCE "DataSource"
#define PROPERTY_TRANSACTIONISOLATION "TransactionIsolation"
#define PROPERTY_TYPEMAP "TypeMap"
#define PROPERTY_USER "User"
#define PROPERTY_PASSWORD "Password"
#define PROPERTY_COMMAND_TYPE "CommandType"
#define PROPERTY_ACTIVECOMMAND "ActiveCommand"
#define PROPERTY_ACTIVE_CONNECTION "ActiveConnection"
#define PROPERTY_FILTER "Filter"
#define PROPERTY_APPLYFILTER "ApplyFilter"
#define PROPERTY_ORDER "Order"
#define PROPERTY_APPLYORDER "ApplyOrder"
#define PROPERTY_ISMODIFIED "IsModified"
#define PROPERTY_ISNEW "IsNew"
#define PROPERTY_ROWCOUNT "RowCount"
#define PROPERTY_ISROWCOUNTFINAL "IsRowCountFinal"
#define PROPERTY_HELPFILENAME "HelpFileName"
#define PROPERTY_WIDTH "Width"
#define PROPERTY_ROW_HEIGHT "RowHeight"
#define PROPERTY_AUTOGROW "AutoGrow"
#define PROPERTY_FORMATKEY "FormatKey"
#define PROPERTY_ALIGN "Align"
#define PROPERTY_FONT "FontDescriptor"
#define PROPERTY_TEXTCOLOR "TextColor"
#define PROPERTY_BOUNDFIELD "BoundField"
#define PROPERTY_CONTROLSOURCE "DataField"
#define PROPERTY_REALNAME "RealName"
#define PROPERTY_ISHIDDEN "IsHidden"
#define PROPERTY_UPDATE_TABLENAME "UpdateTableName"
#define PROPERTY_UPDATE_SCHEMANAME "UpdateSchemaName"
#define PROPERTY_UPDATE_CATALOGNAME "UpdateCatalogName"
#define PROPERTY_RELATIVEPOSITION "RelativePosition"
#define PROPERTY_CONTROLMODEL "ControlModel"
#define PROPERTY_ISASCENDING "IsAscending"
#define PROPERTY_RELATEDCOLUMN "RelatedColumn"
#define PROPERTY_ISUNIQUE "IsUnique"
#define PROPERTY_ISPRIMARYKEYINDEX "IsPrimaryKeyIndex"
#define PROPERTY_IGNORERESULT "IgnoreResult"
#define PROPERTY_UPDATERULE "UpdateRule"
#define PROPERTY_DELETERULE "DeleteRule"
#define PROPERTY_REFERENCEDTABLE "ReferencedTable"
#define PROPERTY_REFERENCEDCOLUMN "ReferencedColumn"
#define PROPERTY_PARENTWINDOW "ParentWindow"
#define PROPERTY_SQLEXCEPTION "SQLException"
#define PROPERTY_BORDER "Border"
#define PROPERTY_THREADSAFE "ThreadSafe"
#define PROPERTY_HELPTEXT "HelpText"
#define PROPERTY_CONTROLDEFAULT "ControlDefault"
#define PROPERTY_HIDDEN "Hidden"
#define PROPERTY_DEFAULTSTATE "DefaultState"
#define PROPERTY_SUPPRESSVERSIONCL "SuppressVersionColumns"
#define PROPERTY_SHOW_BROWSER "ShowBrowser"
#define PROPERTY_ENABLE_BROWSER "EnableBrowser"
#define PROPERTY_SHOWMENU "ShowMenu"
#define PROPERTY_LAYOUTINFORMATION "LayoutInformation"
#define PROPERTY_CURRENTTABLE "CurrentTable"
#define PROPERTY_DATABASENAME "DatabaseName"
#define PROPERTY_CONTROLUSER "ControlUser"
#define PROPERTY_CONTROLPASSWORD "ControlPassword"
#define PROPERTY_CACHESIZE "CacheSize"
#define PROPERTY_TEXTLINECOLOR "TextLineColor"
#define PROPERTY_TEXTEMPHASIS "FontEmphasisMark"
#define PROPERTY_TEXTRELIEF "FontRelief"
#define PROPERTY_DEFAULTTEXT "DefaultText"
#define PROPERTY_EFFECTIVEDEFAULT "EffectiveDefault"
#define PROPERTY_AUTOINCREMENTCREATION "AutoIncrementCreation"
#define PROPERTY_BOOLEANCOMPARISONMODE "BooleanComparisonMode"
#define PROPERTY_ENABLESQL92CHECK "EnableSQL92Check"
#define PROPERTY_FONTCHARWIDTH "FontCharWidth"
#define PROPERTY_FONTCHARSET "FontCharset"
#define PROPERTY_FONTFAMILY "FontFamily"
#define PROPERTY_FONTHEIGHT "FontHeight"
#define PROPERTY_FONTKERNING "FontKerning"
#define PROPERTY_FONTNAME "FontName"
#define PROPERTY_FONTORIENTATION "FontOrientation"
#define PROPERTY_FONTPITCH "FontPitch"
#define PROPERTY_FONTSLANT "FontSlant"
#define PROPERTY_FONTSTRIKEOUT "FontStrikeout"
#define PROPERTY_FONTSTYLENAME "FontStyleName"
#define PROPERTY_FONTUNDERLINE "FontUnderline"
#define PROPERTY_FONTWEIGHT "FontWeight"
#define PROPERTY_FONTWIDTH "FontWidth"
#define PROPERTY_FONTWORDLINEMODE "FontWordLineMode"
#define PROPERTY_FONTTYPE "FontType"
#define PROPERTY_PERSISTENT_NAME "PersistentName"
#define PROPERTY_EMBEDDEDOBJECT "EmbeddedObject"
#define PROPERTY_ORIGINAL "Original"
#define PROPERTY_USECATALOGINSELECT "UseCatalogInSelect"
#define PROPERTY_USESCHEMAINSELECT "UseSchemaInSelect"
#define PROPERTY_OUTERJOINESCAPE "EnableOuterJoinEscape"
#define PROPERTY_AS_TEMPLATE "AsTemplate"
#define PROPERTY_HAVING_CLAUSE "HavingClause"
#define PROPERTY_GROUP_BY "GroupBy"
#define PROPERTY_EDIT_WIDTH "EditWidth"
#define PROPERTY_SINGLESELECTQUERYCOMPOSER "SingleSelectQueryComposer"
#define PROPERTY_CHANGE_NOTIFICATION_ENABLED "PropertyChangeNotificationEnabled"

// service names

inline constexpr OUStringLiteral SERVICE_SDBC_RESULTSET = u"com.sun.star.sdbc.ResultSet";
inline constexpr OUStringLiteral SERVICE_SDBC_ROWSET = u"com.sun.star.sdbc.RowSet";
inline constexpr OUStringLiteral SERVICE_SDBC_STATEMENT = u"com.sun.star.sdbc.Statement";
inline constexpr OUStringLiteral SERVICE_SDBC_PREPAREDSTATEMENT
    = u"com.sun.star.sdbc.PreparedStatement";
inline constexpr OUStringLiteral SERVICE_SDBC_CALLABLESTATEMENT
    = u"com.sun.star.sdbc.CallableStatement";
#define SERVICE_SDBC_CONNECTION "com.sun.star.sdbc.Connection"
inline constexpr OUStringLiteral SERVICE_SDBCX_CONTAINER = u"com.sun.star.sdbcx.Container";
inline constexpr OUStringLiteral SERVICE_SDBCX_TABLE = u"com.sun.star.sdbcx.Table";
inline constexpr OUStringLiteral SERVICE_SDBCX_RESULTSET = u"com.sun.star.sdbcx.ResultSet";
inline constexpr OUStringLiteral SERVICE_SDB_CONNECTION = u"com.sun.star.sdb.Connection";
inline constexpr OUStringLiteral SERVICE_SDBCX_COLUMN = u"com.sun.star.sdbcx.Column";
inline constexpr OUStringLiteral SERVICE_SDBCX_COLUMNDESCRIPTOR
    = u"com.sun.star.sdbcx.ColumnDescriptor";
inline constexpr OUStringLiteral SERVICE_SDB_COLUMNSETTINGS = u"com.sun.star.sdb.ColumnSettings";
inline constexpr OUStringLiteral SERVICE_SDB_RESULTCOLUMN = u"com.sun.star.sdb.ResultColumn";
inline constexpr OUStringLiteral SERVICE_SDB_DATACOLUMN = u"com.sun.star.sdb.DataColumn";
inline constexpr OUStringLiteral SERVICE_SDB_DATASOURCE = u"com.sun.star.sdb.DataSource";
inline constexpr OUStringLiteral SERVICE_SDB_RESULTSET = u"com.sun.star.sdb.ResultSet";
inline constexpr OUStringLiteral SERVICE_SDB_ROWSET = u"com.sun.star.sdb.RowSet";
inline constexpr OUStringLiteral SERVICE_SDB_PREPAREDSTATMENT
    = u"com.sun.star.sdb.PreparedStatement";
inline constexpr OUStringLiteral SERVICE_SDB_CALLABLESTATEMENT
    = u"com.sun.star.sdb.CallableStatement";
inline constexpr OUStringLiteral SERVICE_SDB_SQLQUERYCOMPOSER
    = u"com.sun.star.sdb.SQLQueryComposer";
inline constexpr OUStringLiteral SERVICE_SDB_DATASETTINGS = u"com.sun.star.sdb.DefinitionSettings";
inline constexpr OUStringLiteral SERVICE_SDB_QUERYDESCRIPTOR = u"com.sun.star.sdb.QueryDescriptor";
inline constexpr OUStringLiteral SERVICE_SDB_QUERY = u"com.sun.star.sdb.Query";
#define SERVICE_SDBCX_COLUMNS "com.sun.star.sdbcx.Columns"
inline constexpr OUStringLiteral SERVICE_SDBCX_TABLES = u"com.sun.star.sdbcx.Tables";
inline constexpr OUStringLiteral SERVICE_SDB_QUERIES = u"com.sun.star.sdb.Queries";
#define SERVICE_SDBCX_INDEXCOLUMN "com.sun.star.sdbcx.IndexColumn"
#define SERVICE_SDBCX_KEYCOLUMN "com.sun.star.sdbcx.KeyColumn"
inline constexpr OUStringLiteral SERVICE_SDB_DOCUMENTDEFINITION
    = u"com.sun.star.sdb.DocumentDefinition";
#define SERVICE_NAME_FORM "com.sun.star.sdb.Form"
inline constexpr OUStringLiteral SERVICE_NAME_FORM_COLLECTION = u"com.sun.star.sdb.Forms";
#define SERVICE_NAME_REPORT "com.sun.star.sdb.Report"
inline constexpr OUStringLiteral SERVICE_NAME_REPORT_COLLECTION = u"com.sun.star.sdb.Reports";
inline constexpr OUStringLiteral SERVICE_NAME_QUERY_COLLECTION = u"com.sun.star.sdb.Queries";
#define SERVICE_NAME_TABLE_COLLECTION "com.sun.star.sdb.Tables"
inline constexpr OUStringLiteral SERVICE_NAME_SINGLESELECTQUERYCOMPOSER
    = u"com.sun.star.sdb.SingleSelectQueryComposer";
inline constexpr OUStringLiteral SERVICE_SDB_APPLICATIONCONTROLLER
    = u"org.openoffice.comp.dbu.OApplicationController";

// info properties
inline constexpr OUStringLiteral INFO_JDBCDRIVERCLASS = u"JavaDriverClass";
inline constexpr OUStringLiteral INFO_TEXTFILEEXTENSION = u"Extension";
inline constexpr OUStringLiteral INFO_CHARSET = u"CharSet";
inline constexpr OUStringLiteral INFO_TEXTFILEHEADER = u"HeaderLine";
inline constexpr OUStringLiteral INFO_FIELDDELIMITER = u"FieldDelimiter";
inline constexpr OUStringLiteral INFO_TEXTDELIMITER = u"StringDelimiter";
inline constexpr OUStringLiteral INFO_DECIMALDELIMITER = u"DecimalDelimiter";
inline constexpr OUStringLiteral INFO_THOUSANDSDELIMITER = u"ThousandDelimiter";
inline constexpr OUStringLiteral INFO_SHOWDELETEDROWS = u"ShowDeleted";
inline constexpr OUStringLiteral INFO_ALLOWLONGTABLENAMES = u"NoNameLengthLimit";
inline constexpr OUStringLiteral INFO_ADDITIONALOPTIONS = u"SystemDriverSettings";
inline constexpr OUStringLiteral INFO_AUTORETRIEVEVALUE = u"AutoRetrievingStatement";
inline constexpr OUStringLiteral INFO_AUTORETRIEVEENABLED = u"IsAutoRetrievingEnabled";
inline constexpr OUStringLiteral INFO_APPEND_TABLE_ALIAS = u"AppendTableAliasName";
inline constexpr OUStringLiteral INFO_AS_BEFORE_CORRELATION_NAME
    = u"GenerateASBeforeCorrelationName";
inline constexpr OUStringLiteral INFO_FORMS_CHECK_REQUIRED_FIELDS = u"FormsCheckRequiredFields";
inline constexpr OUStringLiteral INFO_PARAMETERNAMESUBST = u"ParameterNameSubstitution";
inline constexpr OUStringLiteral INFO_IGNOREDRIVER_PRIV = u"IgnoreDriverPrivileges";
inline constexpr OUStringLiteral INFO_USECATALOG = u"UseCatalog";
inline constexpr OUStringLiteral INFO_CONN_LDAP_BASEDN = u"BaseDN";
inline constexpr OUStringLiteral INFO_CONN_LDAP_ROWCOUNT = u"MaxRowCount";
inline constexpr OUStringLiteral INFO_PREVIEW = u"Preview";
inline constexpr OUStringLiteral INFO_MEDIATYPE = u"MediaType";
inline constexpr OUStringLiteral INFO_ESCAPE_DATETIME = u"EscapeDateTime";

// other
inline constexpr OUStringLiteral INFO_POOLURL = u"PoolURL";
#define URL_INTERACTIVE "Interactive"

// URLs

inline constexpr OUStringLiteral URL_COMPONENT_QUERYDESIGN = u".component:DB/QueryDesign";
inline constexpr OUStringLiteral URL_COMPONENT_VIEWDESIGN = u".component:DB/ViewDesign";
inline constexpr OUStringLiteral URL_COMPONENT_TABLEDESIGN = u".component:DB/TableDesign";
inline constexpr OUStringLiteral URL_COMPONENT_FORMGRIDVIEW = u".component:DB/FormGridView";
inline constexpr OUStringLiteral URL_COMPONENT_DATASOURCEBROWSER
    = u".component:DB/DataSourceBrowser";
inline constexpr OUStringLiteral URL_COMPONENT_RELATIONDESIGN = u".component:DB/RelationDesign";
#define URL_COMPONENT_APPLICATION ".component:DB/Application"
#define URL_COMPONENT_REPORTDESIGN ".component:DB/ReportDesign"

// service names

inline constexpr OUStringLiteral SERVICE_SDB_DIRECTSQLDIALOG
    = u"org.openoffice.comp.dbu.DirectSqlDialog";

// other DBU relevant strings

#define PROPERTY_QUERYDESIGNVIEW "QueryDesignView"
#define PROPERTY_GRAPHICAL_DESIGN "GraphicalDesign"
#define PROPERTY_HELP_URL "HelpURL"
inline constexpr OUStringLiteral FRAME_NAME_QUERY_PREVIEW = u"QueryPreview";
inline constexpr OUStringLiteral SERVICE_CONTROLDEFAULT = u"com.sun.star.comp.dbu.OColumnControl";

// other DBU properties

#define PROPERTY_TRISTATE "TriState"
#define PROPERTY_ENABLEOUTERJOIN "EnableOuterJoinEscape"
#define PROPERTY_TABSTOP "TabStop"
#define PROPERTY_DEFAULTCONTROL "DefaultControl"
#define PROPERTY_ENABLED "Enabled"
#define PROPERTY_MOUSE_WHEEL_BEHAVIOR "MouseWheelBehavior"

inline constexpr OUStringLiteral SQLSTATE_GENERAL = u"01000";

#define PROPERTY_APPLYFORMDESIGNMODE "ApplyFormDesignMode"
#define PROPERTY_IS_FORM "IsForm"
#define PROPERTY_PERSISTENT_PATH "PersistentPath"

inline constexpr OUStringLiteral STR_AUTOTEXTSEPARATORLIST = u"\"\t34\t'\t39";

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
