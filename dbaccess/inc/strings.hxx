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

#include <rtl/ustring.hxx>

// property names

constexpr OUStringLiteral PROPERTY_URL = u"URL";
constexpr OUStringLiteral PROPERTY_INFO = u"Info";
constexpr OUStringLiteral PROPERTY_SETTINGS = u"Settings";
constexpr OUStringLiteral PROPERTY_ISPASSWORDREQUIRED = u"IsPasswordRequired";
constexpr OUStringLiteral PROPERTY_TABLEFILTER = u"TableFilter";
constexpr OUStringLiteral PROPERTY_TABLETYPEFILTER = u"TableTypeFilter";
constexpr OUStringLiteral PROPERTY_NAME = u"Name";
constexpr OUStringLiteral PROPERTY_SCHEMANAME = u"SchemaName";
constexpr OUStringLiteral PROPERTY_CATALOGNAME = u"CatalogName";
constexpr OUStringLiteral PROPERTY_PRIVILEGES = u"Privileges";
constexpr OUStringLiteral PROPERTY_ESCAPE_PROCESSING = u"EscapeProcessing";
constexpr OUStringLiteral PROPERTY_COMMAND = u"Command";
constexpr OUStringLiteral PROPERTY_TYPE = u"Type";
constexpr OUStringLiteral PROPERTY_TYPENAME = u"TypeName";
constexpr OUStringLiteral PROPERTY_PRECISION = u"Precision";
constexpr OUStringLiteral PROPERTY_SCALE = u"Scale";
constexpr OUStringLiteral PROPERTY_ISNULLABLE = u"IsNullable";
constexpr OUStringLiteral PROPERTY_ISAUTOINCREMENT = u"IsAutoIncrement";
constexpr OUStringLiteral PROPERTY_ISROWVERSION = u"IsRowVersion";
constexpr OUStringLiteral PROPERTY_DESCRIPTION = u"Description";
constexpr OUStringLiteral PROPERTY_DEFAULTVALUE = u"DefaultValue";
constexpr OUStringLiteral PROPERTY_NUMBERFORMAT = u"FormatKey";
constexpr OUStringLiteral PROPERTY_QUERYTIMEOUT = u"QueryTimeOut";
constexpr OUStringLiteral PROPERTY_MAXFIELDSIZE = u"MaxFieldSize";
constexpr OUStringLiteral PROPERTY_MAXROWS = u"MaxRows";
constexpr OUStringLiteral PROPERTY_CURSORNAME = u"CursorName";
constexpr OUStringLiteral PROPERTY_RESULTSETCONCURRENCY = u"ResultSetConcurrency";
constexpr OUStringLiteral PROPERTY_RESULTSETTYPE = u"ResultSetType";
constexpr OUStringLiteral PROPERTY_FETCHDIRECTION = u"FetchDirection";
constexpr OUStringLiteral PROPERTY_FETCHSIZE = u"FetchSize";
constexpr OUStringLiteral PROPERTY_USEBOOKMARKS = u"UseBookmarks";
constexpr OUStringLiteral PROPERTY_ISSEARCHABLE = u"IsSearchable";
constexpr OUStringLiteral PROPERTY_ISCURRENCY = u"IsCurrency";
constexpr OUStringLiteral PROPERTY_ISSIGNED = u"IsSigned";
constexpr OUStringLiteral PROPERTY_DISPLAYSIZE = u"DisplaySize";
constexpr OUStringLiteral PROPERTY_LABEL = u"Label";
constexpr OUStringLiteral PROPERTY_ISREADONLY = u"IsReadOnly";
constexpr OUStringLiteral PROPERTY_ISWRITABLE = u"IsWritable";
constexpr OUStringLiteral PROPERTY_ISDEFINITELYWRITABLE = u"IsDefinitelyWritable";
constexpr OUStringLiteral PROPERTY_VALUE = u"Value";
constexpr OUStringLiteral PROPERTY_TABLENAME = u"TableName";
constexpr OUStringLiteral PROPERTY_ISCASESENSITIVE = u"IsCaseSensitive";
constexpr OUStringLiteral PROPERTY_SERVICENAME = u"ServiceName";
constexpr OUStringLiteral PROPERTY_ISBOOKMARKABLE = u"IsBookmarkable";
constexpr OUStringLiteral PROPERTY_CANUPDATEINSERTEDROWS = u"CanUpdateInsertedRows";
#define PROPERTY_ISSET "IsSet"
#define PROPERTY_ISOUTPARAMETER "IsOutParameter"
constexpr OUStringLiteral PROPERTY_NUMBERFORMATSSUPPLIER = u"NumberFormatsSupplier";
#define PROPERTY_ISCALCULATED "IsCalculated"
constexpr OUStringLiteral PROPERTY_DATASOURCENAME = u"DataSourceName";
constexpr OUStringLiteral PROPERTY_DATABASE_LOCATION = u"DatabaseLocation";
constexpr OUStringLiteral PROPERTY_CONNECTION_RESOURCE = u"ConnectionResource";
constexpr OUStringLiteral PROPERTY_CONNECTION_INFO = u"ConnectionInfo";
constexpr OUStringLiteral PROPERTY_RESULT_SET = u"ResultSet";
constexpr OUStringLiteral PROPERTY_SELECTION = u"Selection";
constexpr OUStringLiteral PROPERTY_BOOKMARK_SELECTION = u"BookmarkSelection";
constexpr OUStringLiteral PROPERTY_COLUMN_NAME = u"ColumnName";
constexpr OUStringLiteral PROPERTY_COLUMN = u"Column";
constexpr OUStringLiteral PROPERTY_DATASOURCE = u"DataSource";
constexpr OUStringLiteral PROPERTY_TRANSACTIONISOLATION = u"TransactionIsolation";
constexpr OUStringLiteral PROPERTY_TYPEMAP = u"TypeMap";
constexpr OUStringLiteral PROPERTY_USER = u"User";
constexpr OUStringLiteral PROPERTY_PASSWORD = u"Password";
constexpr OUStringLiteral PROPERTY_COMMAND_TYPE = u"CommandType";
constexpr OUStringLiteral PROPERTY_ACTIVECOMMAND = u"ActiveCommand";
constexpr OUStringLiteral PROPERTY_ACTIVE_CONNECTION = u"ActiveConnection";
constexpr OUStringLiteral PROPERTY_FILTER = u"Filter";
constexpr OUStringLiteral PROPERTY_APPLYFILTER = u"ApplyFilter";
constexpr OUStringLiteral PROPERTY_ORDER = u"Order";
constexpr OUStringLiteral PROPERTY_APPLYORDER = u"ApplyOrder";
constexpr OUStringLiteral PROPERTY_ISMODIFIED = u"IsModified";
constexpr OUStringLiteral PROPERTY_ISNEW = u"IsNew";
constexpr OUStringLiteral PROPERTY_ROWCOUNT = u"RowCount";
constexpr OUStringLiteral PROPERTY_ISROWCOUNTFINAL = u"IsRowCountFinal";
#define PROPERTY_HELPFILENAME "HelpFileName"
constexpr OUStringLiteral PROPERTY_WIDTH = u"Width";
constexpr OUStringLiteral PROPERTY_ROW_HEIGHT = u"RowHeight";
constexpr OUStringLiteral PROPERTY_AUTOGROW = u"AutoGrow";
constexpr OUStringLiteral PROPERTY_FORMATKEY = u"FormatKey";
constexpr OUStringLiteral PROPERTY_ALIGN = u"Align";
constexpr OUStringLiteral PROPERTY_FONT = u"FontDescriptor";
constexpr OUStringLiteral PROPERTY_TEXTCOLOR = u"TextColor";
constexpr OUStringLiteral PROPERTY_BOUNDFIELD = u"BoundField";
constexpr OUStringLiteral PROPERTY_CONTROLSOURCE = u"DataField";
constexpr OUStringLiteral PROPERTY_REALNAME = u"RealName";
#define PROPERTY_ISHIDDEN "IsHidden"
constexpr OUStringLiteral PROPERTY_UPDATE_TABLENAME = u"UpdateTableName";
constexpr OUStringLiteral PROPERTY_UPDATE_SCHEMANAME = u"UpdateSchemaName";
constexpr OUStringLiteral PROPERTY_UPDATE_CATALOGNAME = u"UpdateCatalogName";
constexpr OUStringLiteral PROPERTY_RELATIVEPOSITION = u"RelativePosition";
constexpr OUStringLiteral PROPERTY_CONTROLMODEL = u"ControlModel";
#define PROPERTY_ISASCENDING "IsAscending"
constexpr OUStringLiteral PROPERTY_RELATEDCOLUMN = u"RelatedColumn";
constexpr OUStringLiteral PROPERTY_ISUNIQUE = u"IsUnique";
constexpr OUStringLiteral PROPERTY_ISPRIMARYKEYINDEX = u"IsPrimaryKeyIndex";
constexpr OUStringLiteral PROPERTY_IGNORERESULT = u"IgnoreResult";
constexpr OUStringLiteral PROPERTY_UPDATERULE = u"UpdateRule";
constexpr OUStringLiteral PROPERTY_DELETERULE = u"DeleteRule";
constexpr OUStringLiteral PROPERTY_REFERENCEDTABLE = u"ReferencedTable";
#define PROPERTY_REFERENCEDCOLUMN "ReferencedColumn"
#define PROPERTY_PARENTWINDOW "ParentWindow"
constexpr OUStringLiteral PROPERTY_SQLEXCEPTION = u"SQLException";
constexpr OUStringLiteral PROPERTY_BORDER = u"Border";
#define PROPERTY_THREADSAFE "ThreadSafe"
constexpr OUStringLiteral PROPERTY_HELPTEXT = u"HelpText";
constexpr OUStringLiteral PROPERTY_CONTROLDEFAULT = u"ControlDefault";
constexpr OUStringLiteral PROPERTY_HIDDEN = u"Hidden";
#define PROPERTY_DEFAULTSTATE "DefaultState"
constexpr OUStringLiteral PROPERTY_SUPPRESSVERSIONCL = u"SuppressVersionColumns";
constexpr OUStringLiteral PROPERTY_SHOW_BROWSER = u"ShowBrowser";
constexpr OUStringLiteral PROPERTY_ENABLE_BROWSER = u"EnableBrowser";
constexpr OUStringLiteral PROPERTY_SHOWMENU = u"ShowMenu";
constexpr OUStringLiteral PROPERTY_LAYOUTINFORMATION = u"LayoutInformation";
constexpr OUStringLiteral PROPERTY_CURRENTTABLE = u"CurrentTable";
#define PROPERTY_DATABASENAME "DatabaseName"
#define PROPERTY_CONTROLUSER "ControlUser"
#define PROPERTY_CONTROLPASSWORD "ControlPassword"
#define PROPERTY_CACHESIZE "CacheSize"
constexpr OUStringLiteral PROPERTY_TEXTLINECOLOR = u"TextLineColor";
constexpr OUStringLiteral PROPERTY_TEXTEMPHASIS = u"FontEmphasisMark";
constexpr OUStringLiteral PROPERTY_TEXTRELIEF = u"FontRelief";
#define PROPERTY_DEFAULTTEXT "DefaultText"
#define PROPERTY_EFFECTIVEDEFAULT "EffectiveDefault"
constexpr OUStringLiteral PROPERTY_AUTOINCREMENTCREATION = u"AutoIncrementCreation";
constexpr OUStringLiteral PROPERTY_BOOLEANCOMPARISONMODE = u"BooleanComparisonMode";
constexpr OUStringLiteral PROPERTY_ENABLESQL92CHECK = u"EnableSQL92Check";
constexpr OUStringLiteral PROPERTY_FONTCHARWIDTH = u"FontCharWidth";
constexpr OUStringLiteral PROPERTY_FONTCHARSET = u"FontCharset";
constexpr OUStringLiteral PROPERTY_FONTFAMILY = u"FontFamily";
constexpr OUStringLiteral PROPERTY_FONTHEIGHT = u"FontHeight";
constexpr OUStringLiteral PROPERTY_FONTKERNING = u"FontKerning";
constexpr OUStringLiteral PROPERTY_FONTNAME = u"FontName";
constexpr OUStringLiteral PROPERTY_FONTORIENTATION = u"FontOrientation";
constexpr OUStringLiteral PROPERTY_FONTPITCH = u"FontPitch";
constexpr OUStringLiteral PROPERTY_FONTSLANT = u"FontSlant";
constexpr OUStringLiteral PROPERTY_FONTSTRIKEOUT = u"FontStrikeout";
constexpr OUStringLiteral PROPERTY_FONTSTYLENAME = u"FontStyleName";
constexpr OUStringLiteral PROPERTY_FONTUNDERLINE = u"FontUnderline";
constexpr OUStringLiteral PROPERTY_FONTWEIGHT = u"FontWeight";
constexpr OUStringLiteral PROPERTY_FONTWIDTH = u"FontWidth";
constexpr OUStringLiteral PROPERTY_FONTWORDLINEMODE = u"FontWordLineMode";
constexpr OUStringLiteral PROPERTY_FONTTYPE = u"FontType";
constexpr OUStringLiteral PROPERTY_PERSISTENT_NAME = u"PersistentName";
constexpr OUStringLiteral PROPERTY_EMBEDDEDOBJECT = u"EmbeddedObject";
constexpr OUStringLiteral PROPERTY_ORIGINAL = u"Original";
#define PROPERTY_USECATALOGINSELECT "UseCatalogInSelect"
#define PROPERTY_USESCHEMAINSELECT "UseSchemaInSelect"
#define PROPERTY_OUTERJOINESCAPE "EnableOuterJoinEscape"
constexpr OUStringLiteral PROPERTY_AS_TEMPLATE = u"AsTemplate";
constexpr OUStringLiteral PROPERTY_HAVING_CLAUSE = u"HavingClause";
constexpr OUStringLiteral PROPERTY_GROUP_BY = u"GroupBy";
constexpr OUStringLiteral PROPERTY_EDIT_WIDTH = u"EditWidth";
constexpr OUStringLiteral PROPERTY_SINGLESELECTQUERYCOMPOSER = u"SingleSelectQueryComposer";
constexpr OUStringLiteral PROPERTY_CHANGE_NOTIFICATION_ENABLED
    = u"PropertyChangeNotificationEnabled";

// service names

constexpr OUStringLiteral SERVICE_SDBC_RESULTSET = u"com.sun.star.sdbc.ResultSet";
constexpr OUStringLiteral SERVICE_SDBC_ROWSET = u"com.sun.star.sdbc.RowSet";
constexpr OUStringLiteral SERVICE_SDBC_STATEMENT = u"com.sun.star.sdbc.Statement";
constexpr OUStringLiteral SERVICE_SDBC_PREPAREDSTATEMENT = u"com.sun.star.sdbc.PreparedStatement";
constexpr OUStringLiteral SERVICE_SDBC_CALLABLESTATEMENT = u"com.sun.star.sdbc.CallableStatement";
#define SERVICE_SDBC_CONNECTION "com.sun.star.sdbc.Connection"
constexpr OUStringLiteral SERVICE_SDBCX_CONTAINER = u"com.sun.star.sdbcx.Container";
constexpr OUStringLiteral SERVICE_SDBCX_TABLE = u"com.sun.star.sdbcx.Table";
constexpr OUStringLiteral SERVICE_SDBCX_RESULTSET = u"com.sun.star.sdbcx.ResultSet";
#define SERVICE_SDB_CONNECTION "com.sun.star.sdb.Connection"
constexpr OUStringLiteral SERVICE_SDBCX_COLUMN = u"com.sun.star.sdbcx.Column";
constexpr OUStringLiteral SERVICE_SDBCX_COLUMNDESCRIPTOR = u"com.sun.star.sdbcx.ColumnDescriptor";
constexpr OUStringLiteral SERVICE_SDB_COLUMNSETTINGS = u"com.sun.star.sdb.ColumnSettings";
constexpr OUStringLiteral SERVICE_SDB_RESULTCOLUMN = u"com.sun.star.sdb.ResultColumn";
constexpr OUStringLiteral SERVICE_SDB_DATACOLUMN = u"com.sun.star.sdb.DataColumn";
constexpr OUStringLiteral SERVICE_SDB_DATASOURCE = u"com.sun.star.sdb.DataSource";
constexpr OUStringLiteral SERVICE_SDB_RESULTSET = u"com.sun.star.sdb.ResultSet";
constexpr OUStringLiteral SERVICE_SDB_ROWSET = u"com.sun.star.sdb.RowSet";
constexpr OUStringLiteral SERVICE_SDB_PREPAREDSTATMENT = u"com.sun.star.sdb.PreparedStatement";
constexpr OUStringLiteral SERVICE_SDB_CALLABLESTATEMENT = u"com.sun.star.sdb.CallableStatement";
constexpr OUStringLiteral SERVICE_SDB_SQLQUERYCOMPOSER = u"com.sun.star.sdb.SQLQueryComposer";
constexpr OUStringLiteral SERVICE_SDB_DATASETTINGS = u"com.sun.star.sdb.DefinitionSettings";
constexpr OUStringLiteral SERVICE_SDB_QUERYDESCRIPTOR = u"com.sun.star.sdb.QueryDescriptor";
constexpr OUStringLiteral SERVICE_SDB_QUERY = u"com.sun.star.sdb.Query";
#define SERVICE_SDBCX_COLUMNS "com.sun.star.sdbcx.Columns"
constexpr OUStringLiteral SERVICE_SDBCX_TABLES = u"com.sun.star.sdbcx.Tables";
constexpr OUStringLiteral SERVICE_SDB_QUERIES = u"com.sun.star.sdb.Queries";
#define SERVICE_SDBCX_INDEXCOLUMN "com.sun.star.sdbcx.IndexColumn"
#define SERVICE_SDBCX_KEYCOLUMN "com.sun.star.sdbcx.KeyColumn"
constexpr OUStringLiteral SERVICE_SDB_DOCUMENTDEFINITION = u"com.sun.star.sdb.DocumentDefinition";
#define SERVICE_NAME_FORM "com.sun.star.sdb.Form"
constexpr OUStringLiteral SERVICE_NAME_FORM_COLLECTION = u"com.sun.star.sdb.Forms";
#define SERVICE_NAME_REPORT "com.sun.star.sdb.Report"
constexpr OUStringLiteral SERVICE_NAME_REPORT_COLLECTION = u"com.sun.star.sdb.Reports";
constexpr OUStringLiteral SERVICE_NAME_QUERY_COLLECTION = u"com.sun.star.sdb.Queries";
#define SERVICE_NAME_TABLE_COLLECTION "com.sun.star.sdb.Tables"
constexpr OUStringLiteral SERVICE_NAME_SINGLESELECTQUERYCOMPOSER
    = u"com.sun.star.sdb.SingleSelectQueryComposer";
constexpr OUStringLiteral SERVICE_SDB_APPLICATIONCONTROLLER
    = u"org.openoffice.comp.dbu.OApplicationController";

// info properties
#define INFO_JDBCDRIVERCLASS "JavaDriverClass"
constexpr OUStringLiteral INFO_TEXTFILEEXTENSION = u"Extension";
#define INFO_CHARSET "CharSet"
constexpr OUStringLiteral INFO_TEXTFILEHEADER = u"HeaderLine";
constexpr OUStringLiteral INFO_FIELDDELIMITER = u"FieldDelimiter";
constexpr OUStringLiteral INFO_TEXTDELIMITER = u"StringDelimiter";
constexpr OUStringLiteral INFO_DECIMALDELIMITER = u"DecimalDelimiter";
constexpr OUStringLiteral INFO_THOUSANDSDELIMITER = u"ThousandDelimiter";
constexpr OUStringLiteral INFO_SHOWDELETEDROWS = u"ShowDeleted";
constexpr OUStringLiteral INFO_ALLOWLONGTABLENAMES = u"NoNameLengthLimit";
constexpr OUStringLiteral INFO_ADDITIONALOPTIONS = u"SystemDriverSettings";
#define INFO_AUTORETRIEVEVALUE "AutoRetrievingStatement"
#define INFO_AUTORETRIEVEENABLED "IsAutoRetrievingEnabled"
constexpr OUStringLiteral INFO_APPEND_TABLE_ALIAS = u"AppendTableAliasName";
#define INFO_AS_BEFORE_CORRELATION_NAME "GenerateASBeforeCorrelationName"
#define INFO_FORMS_CHECK_REQUIRED_FIELDS "FormsCheckRequiredFields"
constexpr OUStringLiteral INFO_PARAMETERNAMESUBST = u"ParameterNameSubstitution";
constexpr OUStringLiteral INFO_IGNOREDRIVER_PRIV = u"IgnoreDriverPrivileges";
constexpr OUStringLiteral INFO_USECATALOG = u"UseCatalog";
constexpr OUStringLiteral INFO_CONN_LDAP_BASEDN = u"BaseDN";
constexpr OUStringLiteral INFO_CONN_LDAP_ROWCOUNT = u"MaxRowCount";
constexpr OUStringLiteral INFO_PREVIEW = u"Preview";
constexpr OUStringLiteral INFO_MEDIATYPE = u"MediaType";
#define INFO_ESCAPE_DATETIME "EscapeDateTime"

// other
constexpr OUStringLiteral INFO_POOLURL = u"PoolURL";
#define URL_INTERACTIVE "Interactive"

// URLs

constexpr OUStringLiteral URL_COMPONENT_QUERYDESIGN = u".component:DB/QueryDesign";
constexpr OUStringLiteral URL_COMPONENT_VIEWDESIGN = u".component:DB/ViewDesign";
constexpr OUStringLiteral URL_COMPONENT_TABLEDESIGN = u".component:DB/TableDesign";
constexpr OUStringLiteral URL_COMPONENT_FORMGRIDVIEW = u".component:DB/FormGridView";
constexpr OUStringLiteral URL_COMPONENT_DATASOURCEBROWSER = u".component:DB/DataSourceBrowser";
constexpr OUStringLiteral URL_COMPONENT_RELATIONDESIGN = u".component:DB/RelationDesign";
#define URL_COMPONENT_APPLICATION ".component:DB/Application"
#define URL_COMPONENT_REPORTDESIGN ".component:DB/ReportDesign"

// service names

constexpr OUStringLiteral SERVICE_SDB_DIRECTSQLDIALOG = u"org.openoffice.comp.dbu.DirectSqlDialog";

// other DBU relevant strings

constexpr OUStringLiteral FRAME_NAME_QUERY_PREVIEW = u"QueryPreview";
constexpr OUStringLiteral SERVICE_CONTROLDEFAULT = u"com.sun.star.comp.dbu.OColumnControl";
constexpr OUStringLiteral PROPERTY_QUERYDESIGNVIEW = u"QueryDesignView";
constexpr OUStringLiteral PROPERTY_GRAPHICAL_DESIGN = u"GraphicalDesign";
constexpr OUStringLiteral PROPERTY_HELP_URL = u"HelpURL";

// other DBU properties

#define PROPERTY_TRISTATE "TriState"
#define PROPERTY_ENABLEOUTERJOIN "EnableOuterJoinEscape"
constexpr OUStringLiteral PROPERTY_TABSTOP = u"TabStop";
constexpr OUStringLiteral PROPERTY_DEFAULTCONTROL = u"DefaultControl";
constexpr OUStringLiteral PROPERTY_ENABLED = u"Enabled";
constexpr OUStringLiteral PROPERTY_MOUSE_WHEEL_BEHAVIOR = u"MouseWheelBehavior";

constexpr OUStringLiteral SQLSTATE_GENERAL = u"01000";

#define PROPERTY_APPLYFORMDESIGNMODE "ApplyFormDesignMode"
constexpr OUStringLiteral PROPERTY_IS_FORM = u"IsForm";
constexpr OUStringLiteral PROPERTY_PERSISTENT_PATH = u"PersistentPath";

constexpr OUStringLiteral STR_AUTOTEXTSEPARATORLIST = u"\"\t34\t'\t39";

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
