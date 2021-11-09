/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DBACCESS_INC_STRINGS_HXX
#define INCLUDED_DBACCESS_INC_STRINGS_HXX

// property names

constexpr OUStringLiteral PROPERTY_URL = u"URL";
#define PROPERTY_INFO "Info"
#define PROPERTY_SETTINGS "Settings"
#define PROPERTY_ISPASSWORDREQUIRED "IsPasswordRequired"
#define PROPERTY_TABLEFILTER "TableFilter"
#define PROPERTY_TABLETYPEFILTER "TableTypeFilter"
constexpr OUStringLiteral PROPERTY_NAME = u"Name";
constexpr OUStringLiteral PROPERTY_SCHEMANAME = u"SchemaName";
constexpr OUStringLiteral PROPERTY_CATALOGNAME = u"CatalogName";
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
constexpr OUStringLiteral PROPERTY_NUMBERFORMAT = u"FormatKey";
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
constexpr OUStringLiteral PROPERTY_TABLENAME = u"TableName";
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
constexpr OUStringLiteral PROPERTY_USER = u"User";
#define PROPERTY_PASSWORD "Password"
#define PROPERTY_COMMAND_TYPE "CommandType"
#define PROPERTY_ACTIVECOMMAND "ActiveCommand"
#define PROPERTY_ACTIVE_CONNECTION "ActiveConnection"
constexpr OUStringLiteral PROPERTY_FILTER = u"Filter";
constexpr OUStringLiteral PROPERTY_APPLYFILTER = u"ApplyFilter";
constexpr OUStringLiteral PROPERTY_ORDER = u"Order";
#define PROPERTY_APPLYORDER "ApplyOrder"
#define PROPERTY_ISMODIFIED "IsModified"
#define PROPERTY_ISNEW "IsNew"
#define PROPERTY_ROWCOUNT "RowCount"
#define PROPERTY_ISROWCOUNTFINAL "IsRowCountFinal"
#define PROPERTY_HELPFILENAME "HelpFileName"
constexpr OUStringLiteral PROPERTY_WIDTH = u"Width";
constexpr OUStringLiteral PROPERTY_ROW_HEIGHT = u"RowHeight";
#define PROPERTY_AUTOGROW "AutoGrow"
#define PROPERTY_FORMATKEY "FormatKey"
constexpr OUStringLiteral PROPERTY_ALIGN = u"Align";
constexpr OUStringLiteral PROPERTY_FONT = u"FontDescriptor";
constexpr OUStringLiteral PROPERTY_TEXTCOLOR = u"TextColor";
#define PROPERTY_BOUNDFIELD "BoundField"
#define PROPERTY_CONTROLSOURCE "DataField"
constexpr OUStringLiteral PROPERTY_REALNAME = u"RealName";
#define PROPERTY_ISHIDDEN "IsHidden"
constexpr OUStringLiteral PROPERTY_UPDATE_TABLENAME = u"UpdateTableName";
constexpr OUStringLiteral PROPERTY_UPDATE_SCHEMANAME = u"UpdateSchemaName";
constexpr OUStringLiteral PROPERTY_UPDATE_CATALOGNAME = u"UpdateCatalogName";
constexpr OUStringLiteral PROPERTY_RELATIVEPOSITION = u"RelativePosition";
constexpr OUStringLiteral PROPERTY_CONTROLMODEL = u"ControlModel";
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
constexpr OUStringLiteral PROPERTY_HELPTEXT = u"HelpText";
constexpr OUStringLiteral PROPERTY_CONTROLDEFAULT = u"ControlDefault";
constexpr OUStringLiteral PROPERTY_HIDDEN = u"Hidden";
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
constexpr OUStringLiteral PROPERTY_TEXTLINECOLOR = u"TextLineColor";
constexpr OUStringLiteral PROPERTY_TEXTEMPHASIS = u"FontEmphasisMark";
constexpr OUStringLiteral PROPERTY_TEXTRELIEF = u"FontRelief";
#define PROPERTY_DEFAULTTEXT "DefaultText"
#define PROPERTY_EFFECTIVEDEFAULT "EffectiveDefault"
#define PROPERTY_AUTOINCREMENTCREATION "AutoIncrementCreation"
constexpr OUStringLiteral PROPERTY_BOOLEANCOMPARISONMODE = u"BooleanComparisonMode";
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
constexpr OUStringLiteral PROPERTY_HAVING_CLAUSE = u"HavingClause";
constexpr OUStringLiteral PROPERTY_GROUP_BY = u"GroupBy";
#define PROPERTY_EDIT_WIDTH "EditWidth"
#define PROPERTY_SINGLESELECTQUERYCOMPOSER "SingleSelectQueryComposer"
#define PROPERTY_CHANGE_NOTIFICATION_ENABLED "PropertyChangeNotificationEnabled"

// service names

#define SERVICE_SDBC_RESULTSET "com.sun.star.sdbc.ResultSet"
#define SERVICE_SDBC_ROWSET "com.sun.star.sdbc.RowSet"
#define SERVICE_SDBC_STATEMENT "com.sun.star.sdbc.Statement"
#define SERVICE_SDBC_PREPAREDSTATEMENT "com.sun.star.sdbc.PreparedStatement"
#define SERVICE_SDBC_CALLABLESTATEMENT "com.sun.star.sdbc.CallableStatement"
#define SERVICE_SDBC_CONNECTION "com.sun.star.sdbc.Connection"
#define SERVICE_SDBCX_CONTAINER "com.sun.star.sdbcx.Container"
#define SERVICE_SDBCX_TABLE "com.sun.star.sdbcx.Table"
#define SERVICE_SDBCX_RESULTSET "com.sun.star.sdbcx.ResultSet"
#define SERVICE_SDB_CONNECTION "com.sun.star.sdb.Connection"
constexpr OUStringLiteral SERVICE_SDBCX_COLUMN = u"com.sun.star.sdbcx.Column";
constexpr OUStringLiteral SERVICE_SDBCX_COLUMNDESCRIPTOR = u"com.sun.star.sdbcx.ColumnDescriptor";
#define SERVICE_SDB_COLUMNSETTINGS "com.sun.star.sdb.ColumnSettings"
#define SERVICE_SDB_RESULTCOLUMN "com.sun.star.sdb.ResultColumn"
#define SERVICE_SDB_DATACOLUMN "com.sun.star.sdb.DataColumn"
#define SERVICE_SDB_DATASOURCE "com.sun.star.sdb.DataSource"
#define SERVICE_SDB_RESULTSET "com.sun.star.sdb.ResultSet"
#define SERVICE_SDB_ROWSET "com.sun.star.sdb.RowSet"
#define SERVICE_SDB_PREPAREDSTATMENT "com.sun.star.sdb.PreparedStatement"
#define SERVICE_SDB_CALLABLESTATEMENT "com.sun.star.sdb.CallableStatement"
#define SERVICE_SDB_SQLQUERYCOMPOSER "com.sun.star.sdb.SQLQueryComposer"
#define SERVICE_SDB_DATASETTINGS "com.sun.star.sdb.DefinitionSettings"
#define SERVICE_SDB_QUERYDESCRIPTOR "com.sun.star.sdb.QueryDescriptor"
#define SERVICE_SDB_QUERY "com.sun.star.sdb.Query"
#define SERVICE_SDBCX_COLUMNS "com.sun.star.sdbcx.Columns"
#define SERVICE_SDBCX_TABLES "com.sun.star.sdbcx.Tables"
#define SERVICE_SDB_QUERIES "com.sun.star.sdb.Queries"
#define SERVICE_SDBCX_INDEXCOLUMN "com.sun.star.sdbcx.IndexColumn"
#define SERVICE_SDBCX_KEYCOLUMN "com.sun.star.sdbcx.KeyColumn"
constexpr OUStringLiteral SERVICE_SDB_DOCUMENTDEFINITION = u"com.sun.star.sdb.DocumentDefinition";
#define SERVICE_NAME_FORM "com.sun.star.sdb.Form"
constexpr OUStringLiteral SERVICE_NAME_FORM_COLLECTION = u"com.sun.star.sdb.Forms";
#define SERVICE_NAME_REPORT "com.sun.star.sdb.Report"
constexpr OUStringLiteral SERVICE_NAME_REPORT_COLLECTION = u"com.sun.star.sdb.Reports";
#define SERVICE_NAME_QUERY_COLLECTION "com.sun.star.sdb.Queries"
#define SERVICE_NAME_TABLE_COLLECTION "com.sun.star.sdb.Tables"
#define SERVICE_NAME_SINGLESELECTQUERYCOMPOSER "com.sun.star.sdb.SingleSelectQueryComposer"
#define SERVICE_SDB_APPLICATIONCONTROLLER "org.openoffice.comp.dbu.OApplicationController"

// info properties
#define INFO_JDBCDRIVERCLASS "JavaDriverClass"
#define INFO_TEXTFILEEXTENSION "Extension"
#define INFO_CHARSET "CharSet"
#define INFO_TEXTFILEHEADER "HeaderLine"
#define INFO_FIELDDELIMITER "FieldDelimiter"
#define INFO_TEXTDELIMITER "StringDelimiter"
#define INFO_DECIMALDELIMITER "DecimalDelimiter"
#define INFO_THOUSANDSDELIMITER "ThousandDelimiter"
#define INFO_SHOWDELETEDROWS "ShowDeleted"
#define INFO_ALLOWLONGTABLENAMES "NoNameLengthLimit"
#define INFO_ADDITIONALOPTIONS "SystemDriverSettings"
#define INFO_AUTORETRIEVEVALUE "AutoRetrievingStatement"
#define INFO_AUTORETRIEVEENABLED "IsAutoRetrievingEnabled"
#define INFO_APPEND_TABLE_ALIAS "AppendTableAliasName"
#define INFO_AS_BEFORE_CORRELATION_NAME "GenerateASBeforeCorrelationName"
#define INFO_FORMS_CHECK_REQUIRED_FIELDS "FormsCheckRequiredFields"
#define INFO_PARAMETERNAMESUBST "ParameterNameSubstitution"
#define INFO_IGNOREDRIVER_PRIV "IgnoreDriverPrivileges"
#define INFO_USECATALOG "UseCatalog"
#define INFO_CONN_LDAP_BASEDN "BaseDN"
#define INFO_CONN_LDAP_ROWCOUNT "MaxRowCount"
#define INFO_PREVIEW "Preview"
#define INFO_MEDIATYPE "MediaType"
#define INFO_ESCAPE_DATETIME "EscapeDateTime"

// other
#define INFO_POOLURL "PoolURL"
#define URL_INTERACTIVE "Interactive"

// URLs

constexpr OUStringLiteral URL_COMPONENT_QUERYDESIGN = u".component:DB/QueryDesign";
constexpr OUStringLiteral URL_COMPONENT_VIEWDESIGN = u".component:DB/ViewDesign";
constexpr OUStringLiteral URL_COMPONENT_TABLEDESIGN = u".component:DB/TableDesign";
#define URL_COMPONENT_FORMGRIDVIEW ".component:DB/FormGridView"
constexpr OUStringLiteral URL_COMPONENT_DATASOURCEBROWSER = u".component:DB/DataSourceBrowser";
constexpr OUStringLiteral URL_COMPONENT_RELATIONDESIGN = u".component:DB/RelationDesign";
#define URL_COMPONENT_APPLICATION ".component:DB/Application"
#define URL_COMPONENT_REPORTDESIGN ".component:DB/ReportDesign"

// service names

#define SERVICE_SDB_DIRECTSQLDIALOG "org.openoffice.comp.dbu.DirectSqlDialog"

// other DBU relevant strings

constexpr OUStringLiteral FRAME_NAME_QUERY_PREVIEW = u"QueryPreview";
constexpr OUStringLiteral SERVICE_CONTROLDEFAULT = u"com.sun.star.comp.dbu.OColumnControl";
#define PROPERTY_QUERYDESIGNVIEW "QueryDesignView"
#define PROPERTY_GRAPHICAL_DESIGN "GraphicalDesign"
#define PROPERTY_HELP_URL "HelpURL"

// other DBU properties

#define PROPERTY_TRISTATE "TriState"
#define PROPERTY_ENABLEOUTERJOIN "EnableOuterJoinEscape"
#define PROPERTY_TABSTOP "TabStop"
#define PROPERTY_DEFAULTCONTROL "DefaultControl"
#define PROPERTY_ENABLED "Enabled"
#define PROPERTY_MOUSE_WHEEL_BEHAVIOR "MouseWheelBehavior"

#define SQLSTATE_GENERAL "01000"

#define PROPERTY_APPLYFORMDESIGNMODE "ApplyFormDesignMode"
#define PROPERTY_IS_FORM "IsForm"
#define PROPERTY_PERSISTENT_PATH "PersistentPath"

constexpr OUStringLiteral STR_AUTOTEXTSEPARATORLIST = u"\"\t34\t'\t39";

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
