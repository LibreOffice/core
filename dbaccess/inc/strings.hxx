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

inline constexpr OUString PROPERTY_URL(u"URL"_ustr);
inline constexpr OUString PROPERTY_INFO(u"Info"_ustr);
inline constexpr OUString PROPERTY_SETTINGS(u"Settings"_ustr);
inline constexpr OUString PROPERTY_ISPASSWORDREQUIRED(u"IsPasswordRequired"_ustr);
inline constexpr OUString PROPERTY_TABLEFILTER(u"TableFilter"_ustr);
inline constexpr OUString PROPERTY_TABLETYPEFILTER(u"TableTypeFilter"_ustr);
inline constexpr OUString PROPERTY_NAME(u"Name"_ustr);
inline constexpr OUString PROPERTY_SCHEMANAME(u"SchemaName"_ustr);
inline constexpr OUString PROPERTY_CATALOGNAME(u"CatalogName"_ustr);
inline constexpr OUString PROPERTY_PRIVILEGES(u"Privileges"_ustr);
inline constexpr OUString PROPERTY_ESCAPE_PROCESSING(u"EscapeProcessing"_ustr);
inline constexpr OUString PROPERTY_COMMAND(u"Command"_ustr);
inline constexpr OUString PROPERTY_TYPE(u"Type"_ustr);
inline constexpr OUString PROPERTY_TYPENAME(u"TypeName"_ustr);
inline constexpr OUString PROPERTY_PRECISION(u"Precision"_ustr);
inline constexpr OUString PROPERTY_SCALE(u"Scale"_ustr);
inline constexpr OUString PROPERTY_ISNULLABLE(u"IsNullable"_ustr);
inline constexpr OUString PROPERTY_ISAUTOINCREMENT(u"IsAutoIncrement"_ustr);
inline constexpr OUString PROPERTY_ISROWVERSION(u"IsRowVersion"_ustr);
inline constexpr OUString PROPERTY_DESCRIPTION(u"Description"_ustr);
inline constexpr OUString PROPERTY_DEFAULTVALUE(u"DefaultValue"_ustr);
inline constexpr OUString PROPERTY_NUMBERFORMAT(u"FormatKey"_ustr);
inline constexpr OUString PROPERTY_QUERYTIMEOUT(u"QueryTimeOut"_ustr);
inline constexpr OUString PROPERTY_MAXFIELDSIZE(u"MaxFieldSize"_ustr);
inline constexpr OUString PROPERTY_MAXROWS(u"MaxRows"_ustr);
inline constexpr OUString PROPERTY_CURSORNAME(u"CursorName"_ustr);
inline constexpr OUString PROPERTY_RESULTSETCONCURRENCY(u"ResultSetConcurrency"_ustr);
inline constexpr OUString PROPERTY_RESULTSETTYPE(u"ResultSetType"_ustr);
inline constexpr OUString PROPERTY_FETCHDIRECTION(u"FetchDirection"_ustr);
inline constexpr OUString PROPERTY_FETCHSIZE(u"FetchSize"_ustr);
inline constexpr OUString PROPERTY_USEBOOKMARKS(u"UseBookmarks"_ustr);
inline constexpr OUString PROPERTY_ISSEARCHABLE(u"IsSearchable"_ustr);
inline constexpr OUString PROPERTY_ISCURRENCY(u"IsCurrency"_ustr);
inline constexpr OUString PROPERTY_ISSIGNED(u"IsSigned"_ustr);
inline constexpr OUString PROPERTY_DISPLAYSIZE(u"DisplaySize"_ustr);
inline constexpr OUString PROPERTY_LABEL(u"Label"_ustr);
inline constexpr OUString PROPERTY_ISREADONLY(u"IsReadOnly"_ustr);
inline constexpr OUString PROPERTY_ISWRITABLE(u"IsWritable"_ustr);
inline constexpr OUString PROPERTY_ISDEFINITELYWRITABLE(u"IsDefinitelyWritable"_ustr);
inline constexpr OUString PROPERTY_VALUE(u"Value"_ustr);
inline constexpr OUString PROPERTY_TABLENAME(u"TableName"_ustr);
inline constexpr OUString PROPERTY_ISCASESENSITIVE(u"IsCaseSensitive"_ustr);
inline constexpr OUString PROPERTY_SERVICENAME(u"ServiceName"_ustr);
inline constexpr OUString PROPERTY_ISBOOKMARKABLE(u"IsBookmarkable"_ustr);
inline constexpr OUString PROPERTY_CANUPDATEINSERTEDROWS(u"CanUpdateInsertedRows"_ustr);
inline constexpr OUString PROPERTY_NUMBERFORMATSSUPPLIER(u"NumberFormatsSupplier"_ustr);
inline constexpr OUString PROPERTY_DATASOURCENAME(u"DataSourceName"_ustr);
inline constexpr OUString PROPERTY_DATABASE_LOCATION(u"DatabaseLocation"_ustr);
inline constexpr OUString PROPERTY_CONNECTION_RESOURCE(u"ConnectionResource"_ustr);
inline constexpr OUString PROPERTY_CONNECTION_INFO(u"ConnectionInfo"_ustr);
inline constexpr OUString PROPERTY_RESULT_SET(u"ResultSet"_ustr);
inline constexpr OUString PROPERTY_SELECTION(u"Selection"_ustr);
inline constexpr OUString PROPERTY_BOOKMARK_SELECTION(u"BookmarkSelection"_ustr);
inline constexpr OUString PROPERTY_COLUMN_NAME(u"ColumnName"_ustr);
inline constexpr OUString PROPERTY_COLUMN(u"Column"_ustr);
inline constexpr OUString PROPERTY_DATASOURCE(u"DataSource"_ustr);
inline constexpr OUString PROPERTY_TRANSACTIONISOLATION(u"TransactionIsolation"_ustr);
inline constexpr OUString PROPERTY_TYPEMAP(u"TypeMap"_ustr);
inline constexpr OUString PROPERTY_USER(u"User"_ustr);
inline constexpr OUString PROPERTY_PASSWORD(u"Password"_ustr);
inline constexpr OUString PROPERTY_COMMAND_TYPE(u"CommandType"_ustr);
inline constexpr OUString PROPERTY_ACTIVECOMMAND(u"ActiveCommand"_ustr);
inline constexpr OUString PROPERTY_ACTIVE_CONNECTION(u"ActiveConnection"_ustr);
inline constexpr OUString PROPERTY_FILTER(u"Filter"_ustr);
inline constexpr OUString PROPERTY_APPLYFILTER(u"ApplyFilter"_ustr);
inline constexpr OUString PROPERTY_ORDER(u"Order"_ustr);
inline constexpr OUString PROPERTY_APPLYORDER(u"ApplyOrder"_ustr);
inline constexpr OUString PROPERTY_ISMODIFIED(u"IsModified"_ustr);
inline constexpr OUString PROPERTY_ISNEW(u"IsNew"_ustr);
inline constexpr OUString PROPERTY_ROWCOUNT(u"RowCount"_ustr);
inline constexpr OUString PROPERTY_ISROWCOUNTFINAL(u"IsRowCountFinal"_ustr);
inline constexpr OUString PROPERTY_WIDTH(u"Width"_ustr);
inline constexpr OUString PROPERTY_ROW_HEIGHT(u"RowHeight"_ustr);
inline constexpr OUString PROPERTY_AUTOGROW(u"AutoGrow"_ustr);
inline constexpr OUString PROPERTY_FORMATKEY(u"FormatKey"_ustr);
inline constexpr OUString PROPERTY_ALIGN(u"Align"_ustr);
inline constexpr OUString PROPERTY_FONT(u"FontDescriptor"_ustr);
inline constexpr OUString PROPERTY_TEXTCOLOR(u"TextColor"_ustr);
inline constexpr OUString PROPERTY_BOUNDFIELD(u"BoundField"_ustr);
inline constexpr OUString PROPERTY_CONTROLSOURCE(u"DataField"_ustr);
inline constexpr OUString PROPERTY_REALNAME(u"RealName"_ustr);
inline constexpr OUString PROPERTY_UPDATE_TABLENAME(u"UpdateTableName"_ustr);
inline constexpr OUString PROPERTY_UPDATE_SCHEMANAME(u"UpdateSchemaName"_ustr);
inline constexpr OUString PROPERTY_UPDATE_CATALOGNAME(u"UpdateCatalogName"_ustr);
inline constexpr OUString PROPERTY_RELATIVEPOSITION(u"RelativePosition"_ustr);
inline constexpr OUString PROPERTY_CONTROLMODEL(u"ControlModel"_ustr);
inline constexpr OUString PROPERTY_RELATEDCOLUMN(u"RelatedColumn"_ustr);
inline constexpr OUString PROPERTY_ISUNIQUE(u"IsUnique"_ustr);
inline constexpr OUString PROPERTY_ISPRIMARYKEYINDEX(u"IsPrimaryKeyIndex"_ustr);
inline constexpr OUString PROPERTY_IGNORERESULT(u"IgnoreResult"_ustr);
inline constexpr OUString PROPERTY_UPDATERULE(u"UpdateRule"_ustr);
inline constexpr OUString PROPERTY_DELETERULE(u"DeleteRule"_ustr);
inline constexpr OUString PROPERTY_REFERENCEDTABLE(u"ReferencedTable"_ustr);
inline constexpr OUString PROPERTY_SQLEXCEPTION(u"SQLException"_ustr);
inline constexpr OUString PROPERTY_BORDER(u"Border"_ustr);
inline constexpr OUString PROPERTY_HELPTEXT(u"HelpText"_ustr);
inline constexpr OUString PROPERTY_CONTROLDEFAULT(u"ControlDefault"_ustr);
inline constexpr OUString PROPERTY_HIDDEN(u"Hidden"_ustr);
inline constexpr OUString PROPERTY_DEFAULTSTATE(u"DefaultState"_ustr);
inline constexpr OUString PROPERTY_SUPPRESSVERSIONCL(u"SuppressVersionColumns"_ustr);
inline constexpr OUString PROPERTY_SHOW_BROWSER(u"ShowBrowser"_ustr);
inline constexpr OUString PROPERTY_ENABLE_BROWSER(u"EnableBrowser"_ustr);
inline constexpr OUString PROPERTY_SHOWMENU(u"ShowMenu"_ustr);
inline constexpr OUString PROPERTY_LAYOUTINFORMATION(u"LayoutInformation"_ustr);
inline constexpr OUString PROPERTY_CURRENTTABLE(u"CurrentTable"_ustr);
inline constexpr OUString PROPERTY_TEXTLINECOLOR(u"TextLineColor"_ustr);
inline constexpr OUString PROPERTY_TEXTEMPHASIS(u"FontEmphasisMark"_ustr);
inline constexpr OUString PROPERTY_TEXTRELIEF(u"FontRelief"_ustr);
inline constexpr OUString PROPERTY_DEFAULTTEXT(u"DefaultText"_ustr);
inline constexpr OUString PROPERTY_EFFECTIVEDEFAULT(u"EffectiveDefault"_ustr);
inline constexpr OUString PROPERTY_AUTOINCREMENTCREATION(u"AutoIncrementCreation"_ustr);
inline constexpr OUString PROPERTY_BOOLEANCOMPARISONMODE(u"BooleanComparisonMode"_ustr);
inline constexpr OUString PROPERTY_ENABLESQL92CHECK(u"EnableSQL92Check"_ustr);
inline constexpr OUString PROPERTY_FONTCHARWIDTH(u"FontCharWidth"_ustr);
inline constexpr OUString PROPERTY_FONTCHARSET(u"FontCharset"_ustr);
inline constexpr OUString PROPERTY_FONTFAMILY(u"FontFamily"_ustr);
inline constexpr OUString PROPERTY_FONTHEIGHT(u"FontHeight"_ustr);
inline constexpr OUString PROPERTY_FONTKERNING(u"FontKerning"_ustr);
inline constexpr OUString PROPERTY_FONTNAME(u"FontName"_ustr);
inline constexpr OUString PROPERTY_FONTORIENTATION(u"FontOrientation"_ustr);
inline constexpr OUString PROPERTY_FONTPITCH(u"FontPitch"_ustr);
inline constexpr OUString PROPERTY_FONTSLANT(u"FontSlant"_ustr);
inline constexpr OUString PROPERTY_FONTSTRIKEOUT(u"FontStrikeout"_ustr);
inline constexpr OUString PROPERTY_FONTSTYLENAME(u"FontStyleName"_ustr);
inline constexpr OUString PROPERTY_FONTUNDERLINE(u"FontUnderline"_ustr);
inline constexpr OUString PROPERTY_FONTWEIGHT(u"FontWeight"_ustr);
inline constexpr OUString PROPERTY_FONTWIDTH(u"FontWidth"_ustr);
inline constexpr OUString PROPERTY_FONTWORDLINEMODE(u"FontWordLineMode"_ustr);
inline constexpr OUString PROPERTY_FONTTYPE(u"FontType"_ustr);
inline constexpr OUString PROPERTY_PERSISTENT_NAME(u"PersistentName"_ustr);
inline constexpr OUString PROPERTY_EMBEDDEDOBJECT(u"EmbeddedObject"_ustr);
inline constexpr OUString PROPERTY_ORIGINAL(u"Original"_ustr);
inline constexpr OUString PROPERTY_USECATALOGINSELECT(u"UseCatalogInSelect"_ustr);
inline constexpr OUString PROPERTY_USESCHEMAINSELECT(u"UseSchemaInSelect"_ustr);
inline constexpr OUString PROPERTY_OUTERJOINESCAPE(u"EnableOuterJoinEscape"_ustr);
inline constexpr OUString PROPERTY_AS_TEMPLATE(u"AsTemplate"_ustr);
inline constexpr OUString PROPERTY_HAVING_CLAUSE(u"HavingClause"_ustr);
inline constexpr OUString PROPERTY_GROUP_BY(u"GroupBy"_ustr);
inline constexpr OUString PROPERTY_EDIT_WIDTH(u"EditWidth"_ustr);
inline constexpr OUString PROPERTY_SINGLESELECTQUERYCOMPOSER(u"SingleSelectQueryComposer"_ustr);
inline constexpr OUString
    PROPERTY_CHANGE_NOTIFICATION_ENABLED(u"PropertyChangeNotificationEnabled"_ustr);
inline constexpr OUString PROPERTY_CHAR_STRIKEOUT(u"CharStrikeout"_ustr);
inline constexpr OUString PROPERTY_CHAR_UNDERLINE(u"CharUnderline"_ustr);
inline constexpr OUString PROPERTY_CHAR_UNDERLINE_COLOR(u"CharUnderlineColor"_ustr);
inline constexpr OUString PROPERTY_CHAR_UNDERLINE_HAS_COLOR(u"CharUnderlineHasColor"_ustr);

// service names

inline constexpr OUString SERVICE_SDBC_RESULTSET = u"com.sun.star.sdbc.ResultSet"_ustr;
inline constexpr OUString SERVICE_SDBC_ROWSET = u"com.sun.star.sdbc.RowSet"_ustr;
inline constexpr OUString SERVICE_SDBC_STATEMENT = u"com.sun.star.sdbc.Statement"_ustr;
inline constexpr OUString SERVICE_SDBC_PREPAREDSTATEMENT
    = u"com.sun.star.sdbc.PreparedStatement"_ustr;
inline constexpr OUString SERVICE_SDBC_CALLABLESTATEMENT
    = u"com.sun.star.sdbc.CallableStatement"_ustr;
inline constexpr OUString SERVICE_SDBCX_CONTAINER = u"com.sun.star.sdbcx.Container"_ustr;
inline constexpr OUString SERVICE_SDBCX_TABLE = u"com.sun.star.sdbcx.Table"_ustr;
inline constexpr OUString SERVICE_SDBCX_RESULTSET = u"com.sun.star.sdbcx.ResultSet"_ustr;
inline constexpr OUString SERVICE_SDB_CONNECTION = u"com.sun.star.sdb.Connection"_ustr;
inline constexpr OUString SERVICE_SDBCX_COLUMN = u"com.sun.star.sdbcx.Column"_ustr;
inline constexpr OUString SERVICE_SDBCX_COLUMNDESCRIPTOR
    = u"com.sun.star.sdbcx.ColumnDescriptor"_ustr;
inline constexpr OUString SERVICE_SDB_COLUMNSETTINGS = u"com.sun.star.sdb.ColumnSettings"_ustr;
inline constexpr OUString SERVICE_SDB_RESULTCOLUMN = u"com.sun.star.sdb.ResultColumn"_ustr;
inline constexpr OUString SERVICE_SDB_DATACOLUMN = u"com.sun.star.sdb.DataColumn"_ustr;
inline constexpr OUString SERVICE_SDB_DATASOURCE = u"com.sun.star.sdb.DataSource"_ustr;
inline constexpr OUString SERVICE_SDB_RESULTSET = u"com.sun.star.sdb.ResultSet"_ustr;
inline constexpr OUString SERVICE_SDB_ROWSET = u"com.sun.star.sdb.RowSet"_ustr;
inline constexpr OUString SERVICE_SDB_PREPAREDSTATEMENT
    = u"com.sun.star.sdb.PreparedStatement"_ustr;
inline constexpr OUString SERVICE_SDB_CALLABLESTATEMENT
    = u"com.sun.star.sdb.CallableStatement"_ustr;
inline constexpr OUString SERVICE_SDB_SQLQUERYCOMPOSER = u"com.sun.star.sdb.SQLQueryComposer"_ustr;
inline constexpr OUString SERVICE_SDB_DATASETTINGS = u"com.sun.star.sdb.DefinitionSettings"_ustr;
inline constexpr OUString SERVICE_SDB_QUERYDESCRIPTOR = u"com.sun.star.sdb.QueryDescriptor"_ustr;
inline constexpr OUString SERVICE_SDB_QUERY = u"com.sun.star.sdb.Query"_ustr;
inline constexpr OUString SERVICE_SDBCX_TABLES = u"com.sun.star.sdbcx.Tables"_ustr;
inline constexpr OUString SERVICE_SDB_QUERIES = u"com.sun.star.sdb.Queries"_ustr;
inline constexpr OUString SERVICE_SDB_DOCUMENTDEFINITION
    = u"com.sun.star.sdb.DocumentDefinition"_ustr;
inline constexpr OUString SERVICE_NAME_FORM_COLLECTION = u"com.sun.star.sdb.Forms"_ustr;
inline constexpr OUString SERVICE_NAME_REPORT_COLLECTION = u"com.sun.star.sdb.Reports"_ustr;
inline constexpr OUString SERVICE_NAME_QUERY_COLLECTION = u"com.sun.star.sdb.Queries"_ustr;
inline constexpr OUString SERVICE_NAME_SINGLESELECTQUERYCOMPOSER
    = u"com.sun.star.sdb.SingleSelectQueryComposer"_ustr;
inline constexpr OUString SERVICE_SDB_APPLICATIONCONTROLLER
    = u"org.openoffice.comp.dbu.OApplicationController"_ustr;

// info properties
inline constexpr OUString INFO_JDBCDRIVERCLASS = u"JavaDriverClass"_ustr;
inline constexpr OUString INFO_TEXTFILEEXTENSION = u"Extension"_ustr;
inline constexpr OUString INFO_CHARSET = u"CharSet"_ustr;
inline constexpr OUString INFO_TEXTFILEHEADER = u"HeaderLine"_ustr;
inline constexpr OUString INFO_FIELDDELIMITER = u"FieldDelimiter"_ustr;
inline constexpr OUString INFO_TEXTDELIMITER = u"StringDelimiter"_ustr;
inline constexpr OUString INFO_DECIMALDELIMITER = u"DecimalDelimiter"_ustr;
inline constexpr OUString INFO_THOUSANDSDELIMITER = u"ThousandDelimiter"_ustr;
inline constexpr OUString INFO_SHOWDELETEDROWS = u"ShowDeleted"_ustr;
inline constexpr OUString INFO_ALLOWLONGTABLENAMES = u"NoNameLengthLimit"_ustr;
inline constexpr OUString INFO_ADDITIONALOPTIONS = u"SystemDriverSettings"_ustr;
inline constexpr OUString INFO_AUTORETRIEVEVALUE = u"AutoRetrievingStatement"_ustr;
inline constexpr OUString INFO_AUTORETRIEVEENABLED = u"IsAutoRetrievingEnabled"_ustr;
inline constexpr OUString INFO_APPEND_TABLE_ALIAS = u"AppendTableAliasName"_ustr;
inline constexpr OUString INFO_AS_BEFORE_CORRELATION_NAME = u"GenerateASBeforeCorrelationName"_ustr;
inline constexpr OUString INFO_FORMS_CHECK_REQUIRED_FIELDS = u"FormsCheckRequiredFields"_ustr;
inline constexpr OUString INFO_PARAMETERNAMESUBST = u"ParameterNameSubstitution"_ustr;
inline constexpr OUString INFO_IGNOREDRIVER_PRIV = u"IgnoreDriverPrivileges"_ustr;
inline constexpr OUString INFO_USECATALOG = u"UseCatalog"_ustr;
inline constexpr OUString INFO_CONN_LDAP_BASEDN = u"BaseDN"_ustr;
inline constexpr OUString INFO_CONN_LDAP_ROWCOUNT = u"MaxRowCount"_ustr;
inline constexpr OUString INFO_PREVIEW = u"Preview"_ustr;
inline constexpr OUString INFO_MEDIATYPE = u"MediaType"_ustr;
inline constexpr OUString INFO_ESCAPE_DATETIME = u"EscapeDateTime"_ustr;

// other
inline constexpr OUString INFO_POOLURL = u"PoolURL"_ustr;

// URLs

inline constexpr OUString URL_COMPONENT_QUERYDESIGN = u".component:DB/QueryDesign"_ustr;
inline constexpr OUString URL_COMPONENT_VIEWDESIGN = u".component:DB/ViewDesign"_ustr;
inline constexpr OUString URL_COMPONENT_TABLEDESIGN = u".component:DB/TableDesign"_ustr;
inline constexpr OUString URL_COMPONENT_FORMGRIDVIEW = u".component:DB/FormGridView"_ustr;
inline constexpr OUString URL_COMPONENT_DATASOURCEBROWSER = u".component:DB/DataSourceBrowser"_ustr;
inline constexpr OUString URL_COMPONENT_RELATIONDESIGN = u".component:DB/RelationDesign"_ustr;
inline constexpr OUString URL_COMPONENT_REPORTDESIGN(u".component:DB/ReportDesign"_ustr);

// service names

inline constexpr OUString SERVICE_SDB_DIRECTSQLDIALOG
    = u"org.openoffice.comp.dbu.DirectSqlDialog"_ustr;

// other DBU relevant strings

inline constexpr OUString PROPERTY_QUERYDESIGNVIEW(u"QueryDesignView"_ustr);
inline constexpr OUString PROPERTY_GRAPHICAL_DESIGN(u"GraphicalDesign"_ustr);
inline constexpr OUString PROPERTY_HELP_URL(u"HelpURL"_ustr);
inline constexpr OUString FRAME_NAME_QUERY_PREVIEW = u"QueryPreview"_ustr;
inline constexpr OUString SERVICE_CONTROLDEFAULT = u"com.sun.star.comp.dbu.OColumnControl"_ustr;

// other DBU properties

inline constexpr OUString PROPERTY_ENABLEOUTERJOIN(u"EnableOuterJoinEscape"_ustr);
inline constexpr OUString PROPERTY_TABSTOP2(u"TabStop"_ustr);
inline constexpr OUString PROPERTY_DEFAULTCONTROL(u"DefaultControl"_ustr);
inline constexpr OUString PROPERTY_ENABLED(u"Enabled"_ustr);
inline constexpr OUString PROPERTY_MOUSE_WHEEL_BEHAVIOR(u"MouseWheelBehavior"_ustr);

inline constexpr OUString SQLSTATE_GENERAL = u"01000"_ustr;

inline constexpr OUString PROPERTY_IS_FORM(u"IsForm"_ustr);
inline constexpr OUString PROPERTY_PERSISTENT_PATH(u"PersistentPath"_ustr);

inline constexpr OUString STR_AUTOTEXTSEPARATORLIST = u"\"\t34\t'\t39"_ustr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
