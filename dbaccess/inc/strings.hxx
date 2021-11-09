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

inline constexpr OUStringLiteral PROPERTY_URL = u"URL";
inline constexpr OUStringLiteral PROPERTY_INFO = u"Info";
inline constexpr OUStringLiteral PROPERTY_SETTINGS = u"Settings";
inline constexpr OUStringLiteral PROPERTY_ISPASSWORDREQUIRED = u"IsPasswordRequired";
inline constexpr OUStringLiteral PROPERTY_TABLEFILTER = u"TableFilter";
inline constexpr OUStringLiteral PROPERTY_TABLETYPEFILTER = u"TableTypeFilter";
inline constexpr OUStringLiteral PROPERTY_NAME = u"Name";
inline constexpr OUStringLiteral PROPERTY_SCHEMANAME = u"SchemaName";
inline constexpr OUStringLiteral PROPERTY_CATALOGNAME = u"CatalogName";
inline constexpr OUStringLiteral PROPERTY_PRIVILEGES = u"Privileges";
inline constexpr OUStringLiteral PROPERTY_ESCAPE_PROCESSING = u"EscapeProcessing";
inline constexpr OUStringLiteral PROPERTY_COMMAND = u"Command";
inline constexpr OUStringLiteral PROPERTY_TYPE = u"Type";
inline constexpr OUStringLiteral PROPERTY_TYPENAME = u"TypeName";
inline constexpr OUStringLiteral PROPERTY_PRECISION = u"Precision";
inline constexpr OUStringLiteral PROPERTY_SCALE = u"Scale";
inline constexpr OUStringLiteral PROPERTY_ISNULLABLE = u"IsNullable";
inline constexpr OUStringLiteral PROPERTY_ISAUTOINCREMENT = u"IsAutoIncrement";
inline constexpr OUStringLiteral PROPERTY_ISROWVERSION = u"IsRowVersion";
inline constexpr OUStringLiteral PROPERTY_DESCRIPTION = u"Description";
inline constexpr OUStringLiteral PROPERTY_DEFAULTVALUE = u"DefaultValue";
inline constexpr OUStringLiteral PROPERTY_NUMBERFORMAT = u"FormatKey";
inline constexpr OUStringLiteral PROPERTY_QUERYTIMEOUT = u"QueryTimeOut";
inline constexpr OUStringLiteral PROPERTY_MAXFIELDSIZE = u"MaxFieldSize";
inline constexpr OUStringLiteral PROPERTY_MAXROWS = u"MaxRows";
inline constexpr OUStringLiteral PROPERTY_CURSORNAME = u"CursorName";
inline constexpr OUStringLiteral PROPERTY_RESULTSETCONCURRENCY = u"ResultSetConcurrency";
inline constexpr OUStringLiteral PROPERTY_RESULTSETTYPE = u"ResultSetType";
inline constexpr OUStringLiteral PROPERTY_FETCHDIRECTION = u"FetchDirection";
inline constexpr OUStringLiteral PROPERTY_FETCHSIZE = u"FetchSize";
inline constexpr OUStringLiteral PROPERTY_USEBOOKMARKS = u"UseBookmarks";
inline constexpr OUStringLiteral PROPERTY_ISSEARCHABLE = u"IsSearchable";
inline constexpr OUStringLiteral PROPERTY_ISCURRENCY = u"IsCurrency";
inline constexpr OUStringLiteral PROPERTY_ISSIGNED = u"IsSigned";
inline constexpr OUStringLiteral PROPERTY_DISPLAYSIZE = u"DisplaySize";
inline constexpr OUStringLiteral PROPERTY_LABEL = u"Label";
inline constexpr OUStringLiteral PROPERTY_ISREADONLY = u"IsReadOnly";
inline constexpr OUStringLiteral PROPERTY_ISWRITABLE = u"IsWritable";
inline constexpr OUStringLiteral PROPERTY_ISDEFINITELYWRITABLE = u"IsDefinitelyWritable";
inline constexpr OUStringLiteral PROPERTY_VALUE = u"Value";
inline constexpr OUStringLiteral PROPERTY_TABLENAME = u"TableName";
inline constexpr OUStringLiteral PROPERTY_ISCASESENSITIVE = u"IsCaseSensitive";
inline constexpr OUStringLiteral PROPERTY_SERVICENAME = u"ServiceName";
inline constexpr OUStringLiteral PROPERTY_ISBOOKMARKABLE = u"IsBookmarkable";
inline constexpr OUStringLiteral PROPERTY_CANUPDATEINSERTEDROWS = u"CanUpdateInsertedRows";
#define PROPERTY_ISSET "IsSet"
#define PROPERTY_ISOUTPARAMETER "IsOutParameter"
inline constexpr OUStringLiteral PROPERTY_NUMBERFORMATSSUPPLIER = u"NumberFormatsSupplier";
#define PROPERTY_ISCALCULATED "IsCalculated"
inline constexpr OUStringLiteral PROPERTY_DATASOURCENAME = u"DataSourceName";
inline constexpr OUStringLiteral PROPERTY_DATABASE_LOCATION = u"DatabaseLocation";
inline constexpr OUStringLiteral PROPERTY_CONNECTION_RESOURCE = u"ConnectionResource";
inline constexpr OUStringLiteral PROPERTY_CONNECTION_INFO = u"ConnectionInfo";
inline constexpr OUStringLiteral PROPERTY_RESULT_SET = u"ResultSet";
inline constexpr OUStringLiteral PROPERTY_SELECTION = u"Selection";
inline constexpr OUStringLiteral PROPERTY_BOOKMARK_SELECTION = u"BookmarkSelection";
inline constexpr OUStringLiteral PROPERTY_COLUMN_NAME = u"ColumnName";
inline constexpr OUStringLiteral PROPERTY_COLUMN = u"Column";
inline constexpr OUStringLiteral PROPERTY_DATASOURCE = u"DataSource";
inline constexpr OUStringLiteral PROPERTY_TRANSACTIONISOLATION = u"TransactionIsolation";
inline constexpr OUStringLiteral PROPERTY_TYPEMAP = u"TypeMap";
inline constexpr OUStringLiteral PROPERTY_USER = u"User";
inline constexpr OUStringLiteral PROPERTY_PASSWORD = u"Password";
inline constexpr OUStringLiteral PROPERTY_COMMAND_TYPE = u"CommandType";
inline constexpr OUStringLiteral PROPERTY_ACTIVECOMMAND = u"ActiveCommand";
inline constexpr OUStringLiteral PROPERTY_ACTIVE_CONNECTION = u"ActiveConnection";
inline constexpr OUStringLiteral PROPERTY_FILTER = u"Filter";
inline constexpr OUStringLiteral PROPERTY_APPLYFILTER = u"ApplyFilter";
inline constexpr OUStringLiteral PROPERTY_ORDER = u"Order";
inline constexpr OUStringLiteral PROPERTY_APPLYORDER = u"ApplyOrder";
inline constexpr OUStringLiteral PROPERTY_ISMODIFIED = u"IsModified";
inline constexpr OUStringLiteral PROPERTY_ISNEW = u"IsNew";
inline constexpr OUStringLiteral PROPERTY_ROWCOUNT = u"RowCount";
inline constexpr OUStringLiteral PROPERTY_ISROWCOUNTFINAL = u"IsRowCountFinal";
#define PROPERTY_HELPFILENAME "HelpFileName"
inline constexpr OUStringLiteral PROPERTY_WIDTH = u"Width";
inline constexpr OUStringLiteral PROPERTY_ROW_HEIGHT = u"RowHeight";
inline constexpr OUStringLiteral PROPERTY_AUTOGROW = u"AutoGrow";
inline constexpr OUStringLiteral PROPERTY_FORMATKEY = u"FormatKey";
inline constexpr OUStringLiteral PROPERTY_ALIGN = u"Align";
inline constexpr OUStringLiteral PROPERTY_FONT = u"FontDescriptor";
inline constexpr OUStringLiteral PROPERTY_TEXTCOLOR = u"TextColor";
inline constexpr OUStringLiteral PROPERTY_BOUNDFIELD = u"BoundField";
inline constexpr OUStringLiteral PROPERTY_CONTROLSOURCE = u"DataField";
inline constexpr OUStringLiteral PROPERTY_REALNAME = u"RealName";
#define PROPERTY_ISHIDDEN "IsHidden"
inline constexpr OUStringLiteral PROPERTY_UPDATE_TABLENAME = u"UpdateTableName";
inline constexpr OUStringLiteral PROPERTY_UPDATE_SCHEMANAME = u"UpdateSchemaName";
inline constexpr OUStringLiteral PROPERTY_UPDATE_CATALOGNAME = u"UpdateCatalogName";
inline constexpr OUStringLiteral PROPERTY_RELATIVEPOSITION = u"RelativePosition";
inline constexpr OUStringLiteral PROPERTY_CONTROLMODEL = u"ControlModel";
#define PROPERTY_ISASCENDING "IsAscending"
inline constexpr OUStringLiteral PROPERTY_RELATEDCOLUMN = u"RelatedColumn";
inline constexpr OUStringLiteral PROPERTY_ISUNIQUE = u"IsUnique";
inline constexpr OUStringLiteral PROPERTY_ISPRIMARYKEYINDEX = u"IsPrimaryKeyIndex";
inline constexpr OUStringLiteral PROPERTY_IGNORERESULT = u"IgnoreResult";
inline constexpr OUStringLiteral PROPERTY_UPDATERULE = u"UpdateRule";
inline constexpr OUStringLiteral PROPERTY_DELETERULE = u"DeleteRule";
inline constexpr OUStringLiteral PROPERTY_REFERENCEDTABLE = u"ReferencedTable";
#define PROPERTY_REFERENCEDCOLUMN "ReferencedColumn"
#define PROPERTY_PARENTWINDOW "ParentWindow"
inline constexpr OUStringLiteral PROPERTY_SQLEXCEPTION = u"SQLException";
inline constexpr OUStringLiteral PROPERTY_BORDER = u"Border";
#define PROPERTY_THREADSAFE "ThreadSafe"
inline constexpr OUStringLiteral PROPERTY_HELPTEXT = u"HelpText";
inline constexpr OUStringLiteral PROPERTY_CONTROLDEFAULT = u"ControlDefault";
inline constexpr OUStringLiteral PROPERTY_HIDDEN = u"Hidden";
inline constexpr OUStringLiteral PROPERTY_DEFAULTSTATE = u"DefaultState";
inline constexpr OUStringLiteral PROPERTY_SUPPRESSVERSIONCL = u"SuppressVersionColumns";
inline constexpr OUStringLiteral PROPERTY_SHOW_BROWSER = u"ShowBrowser";
inline constexpr OUStringLiteral PROPERTY_ENABLE_BROWSER = u"EnableBrowser";
inline constexpr OUStringLiteral PROPERTY_SHOWMENU = u"ShowMenu";
inline constexpr OUStringLiteral PROPERTY_LAYOUTINFORMATION = u"LayoutInformation";
inline constexpr OUStringLiteral PROPERTY_CURRENTTABLE = u"CurrentTable";
#define PROPERTY_DATABASENAME "DatabaseName"
#define PROPERTY_CONTROLUSER "ControlUser"
#define PROPERTY_CONTROLPASSWORD "ControlPassword"
#define PROPERTY_CACHESIZE "CacheSize"
inline constexpr OUStringLiteral PROPERTY_TEXTLINECOLOR = u"TextLineColor";
inline constexpr OUStringLiteral PROPERTY_TEXTEMPHASIS = u"FontEmphasisMark";
inline constexpr OUStringLiteral PROPERTY_TEXTRELIEF = u"FontRelief";
inline constexpr OUStringLiteral PROPERTY_DEFAULTTEXT = u"DefaultText";
inline constexpr OUStringLiteral PROPERTY_EFFECTIVEDEFAULT = u"EffectiveDefault";
inline constexpr OUStringLiteral PROPERTY_AUTOINCREMENTCREATION = u"AutoIncrementCreation";
inline constexpr OUStringLiteral PROPERTY_BOOLEANCOMPARISONMODE = u"BooleanComparisonMode";
inline constexpr OUStringLiteral PROPERTY_ENABLESQL92CHECK = u"EnableSQL92Check";
inline constexpr OUStringLiteral PROPERTY_FONTCHARWIDTH = u"FontCharWidth";
inline constexpr OUStringLiteral PROPERTY_FONTCHARSET = u"FontCharset";
inline constexpr OUStringLiteral PROPERTY_FONTFAMILY = u"FontFamily";
inline constexpr OUStringLiteral PROPERTY_FONTHEIGHT = u"FontHeight";
inline constexpr OUStringLiteral PROPERTY_FONTKERNING = u"FontKerning";
inline constexpr OUStringLiteral PROPERTY_FONTNAME = u"FontName";
inline constexpr OUStringLiteral PROPERTY_FONTORIENTATION = u"FontOrientation";
inline constexpr OUStringLiteral PROPERTY_FONTPITCH = u"FontPitch";
inline constexpr OUStringLiteral PROPERTY_FONTSLANT = u"FontSlant";
inline constexpr OUStringLiteral PROPERTY_FONTSTRIKEOUT = u"FontStrikeout";
inline constexpr OUStringLiteral PROPERTY_FONTSTYLENAME = u"FontStyleName";
inline constexpr OUStringLiteral PROPERTY_FONTUNDERLINE = u"FontUnderline";
inline constexpr OUStringLiteral PROPERTY_FONTWEIGHT = u"FontWeight";
inline constexpr OUStringLiteral PROPERTY_FONTWIDTH = u"FontWidth";
inline constexpr OUStringLiteral PROPERTY_FONTWORDLINEMODE = u"FontWordLineMode";
inline constexpr OUStringLiteral PROPERTY_FONTTYPE = u"FontType";
inline constexpr OUStringLiteral PROPERTY_PERSISTENT_NAME = u"PersistentName";
inline constexpr OUStringLiteral PROPERTY_EMBEDDEDOBJECT = u"EmbeddedObject";
inline constexpr OUStringLiteral PROPERTY_ORIGINAL = u"Original";
inline constexpr OUStringLiteral PROPERTY_USECATALOGINSELECT = u"UseCatalogInSelect";
inline constexpr OUStringLiteral PROPERTY_USESCHEMAINSELECT = u"UseSchemaInSelect";
#define PROPERTY_OUTERJOINESCAPE "EnableOuterJoinEscape"
inline constexpr OUStringLiteral PROPERTY_AS_TEMPLATE = u"AsTemplate";
inline constexpr OUStringLiteral PROPERTY_HAVING_CLAUSE = u"HavingClause";
inline constexpr OUStringLiteral PROPERTY_GROUP_BY = u"GroupBy";
inline constexpr OUStringLiteral PROPERTY_EDIT_WIDTH = u"EditWidth";
inline constexpr OUStringLiteral PROPERTY_SINGLESELECTQUERYCOMPOSER = u"SingleSelectQueryComposer";
inline constexpr OUStringLiteral PROPERTY_CHANGE_NOTIFICATION_ENABLED
    = u"PropertyChangeNotificationEnabled";

// service names

inline constexpr OUStringLiteral SERVICE_SDBC_RESULTSET = u"com.sun.star.sdbc.ResultSet";
inline constexpr OUStringLiteral SERVICE_SDBC_ROWSET = u"com.sun.star.sdbc.RowSet";
inline constexpr OUStringLiteral SERVICE_SDBC_STATEMENT = u"com.sun.star.sdbc.Statement";
inline constexpr OUStringLiteral SERVICE_SDBC_PREPAREDSTATEMENT
    = u"com.sun.star.sdbc.PreparedStatement";
inline constexpr OUStringLiteral SERVICE_SDBC_CALLABLESTATEMENT
    = u"com.sun.star.sdbc.CallableStatement";
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
inline constexpr OUStringLiteral SERVICE_SDBCX_TABLES = u"com.sun.star.sdbcx.Tables";
inline constexpr OUStringLiteral SERVICE_SDB_QUERIES = u"com.sun.star.sdb.Queries";
inline constexpr OUStringLiteral SERVICE_SDB_DOCUMENTDEFINITION
    = u"com.sun.star.sdb.DocumentDefinition";
inline constexpr OUStringLiteral SERVICE_NAME_FORM_COLLECTION = u"com.sun.star.sdb.Forms";
inline constexpr OUStringLiteral SERVICE_NAME_REPORT_COLLECTION = u"com.sun.star.sdb.Reports";
inline constexpr OUStringLiteral SERVICE_NAME_QUERY_COLLECTION = u"com.sun.star.sdb.Queries";
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

// URLs

inline constexpr OUStringLiteral URL_COMPONENT_QUERYDESIGN = u".component:DB/QueryDesign";
inline constexpr OUStringLiteral URL_COMPONENT_VIEWDESIGN = u".component:DB/ViewDesign";
inline constexpr OUStringLiteral URL_COMPONENT_TABLEDESIGN = u".component:DB/TableDesign";
inline constexpr OUStringLiteral URL_COMPONENT_FORMGRIDVIEW = u".component:DB/FormGridView";
inline constexpr OUStringLiteral URL_COMPONENT_DATASOURCEBROWSER
    = u".component:DB/DataSourceBrowser";
inline constexpr OUStringLiteral URL_COMPONENT_RELATIONDESIGN = u".component:DB/RelationDesign";
#define URL_COMPONENT_REPORTDESIGN ".component:DB/ReportDesign"

// service names

inline constexpr OUStringLiteral SERVICE_SDB_DIRECTSQLDIALOG
    = u"org.openoffice.comp.dbu.DirectSqlDialog";

// other DBU relevant strings

inline constexpr OUStringLiteral PROPERTY_QUERYDESIGNVIEW = u"QueryDesignView";
inline constexpr OUStringLiteral PROPERTY_GRAPHICAL_DESIGN = u"GraphicalDesign";
inline constexpr OUStringLiteral PROPERTY_HELP_URL = u"HelpURL";
inline constexpr OUStringLiteral FRAME_NAME_QUERY_PREVIEW = u"QueryPreview";
inline constexpr OUStringLiteral SERVICE_CONTROLDEFAULT = u"com.sun.star.comp.dbu.OColumnControl";

// other DBU properties

#define PROPERTY_TRISTATE "TriState"
inline constexpr OUStringLiteral PROPERTY_ENABLEOUTERJOIN = u"EnableOuterJoinEscape";
inline constexpr OUStringLiteral PROPERTY_TABSTOP = u"TabStop";
inline constexpr OUStringLiteral PROPERTY_DEFAULTCONTROL = u"DefaultControl";
inline constexpr OUStringLiteral PROPERTY_ENABLED = u"Enabled";
inline constexpr OUStringLiteral PROPERTY_MOUSE_WHEEL_BEHAVIOR = u"MouseWheelBehavior";

inline constexpr OUStringLiteral SQLSTATE_GENERAL = u"01000";

#define PROPERTY_APPLYFORMDESIGNMODE "ApplyFormDesignMode"
inline constexpr OUStringLiteral PROPERTY_IS_FORM = u"IsForm";
inline constexpr OUStringLiteral PROPERTY_PERSISTENT_PATH = u"PersistentPath";

inline constexpr OUStringLiteral STR_AUTOTEXTSEPARATORLIST = u"\"\t34\t'\t39";

inline constexpr OUStringLiteral PROPERTY_CHAR_STRIKEOUT = u"CharStrikeout";
inline constexpr OUStringLiteral PROPERTY_CHAR_UNDERLINE = u"CharUnderline";
inline constexpr OUStringLiteral PROPERTY_CHAR_UNDERLINE_COLOR = u"CharUnderlineColor";
inline constexpr OUStringLiteral PROPERTY_CHAR_UNDERLINE_HAS_COLOR = u"CharUnderlineHasColor";

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
