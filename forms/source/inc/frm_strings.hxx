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

#pragma once

#include <rtl/ustring.hxx>

//- properties

inline constexpr OUStringLiteral PROPERTY_TABINDEX = u"TabIndex";
inline constexpr OUStringLiteral PROPERTY_TAG = u"Tag";
inline constexpr OUStringLiteral PROPERTY_NAME = u"Name";
inline constexpr OUStringLiteral PROPERTY_GROUP_NAME = u"GroupName";
inline constexpr OUStringLiteral PROPERTY_CLASSID = u"ClassId";
inline constexpr OUStringLiteral PROPERTY_FETCHSIZE = u"FetchSize";
inline constexpr OUStringLiteral PROPERTY_VALUE = u"Value";
inline constexpr OUStringLiteral PROPERTY_TEXT = u"Text";
inline constexpr OUStringLiteral PROPERTY_LABEL = u"Label";
inline constexpr OUStringLiteral PROPERTY_NAVIGATION = u"NavigationBarMode";
inline constexpr OUStringLiteral PROPERTY_HASNAVIGATION = u"HasNavigationBar";
inline constexpr OUStringLiteral PROPERTY_CYCLE = u"Cycle";
inline constexpr OUStringLiteral PROPERTY_CONTROLSOURCE = u"DataField";
inline constexpr OUStringLiteral PROPERTY_ENABLED = u"Enabled";
inline constexpr OUStringLiteral PROPERTY_ENABLEVISIBLE = u"EnableVisible";
inline constexpr OUStringLiteral PROPERTY_READONLY = u"ReadOnly";
inline constexpr OUStringLiteral PROPERTY_RELEVANT = u"Relevant";
inline constexpr OUStringLiteral PROPERTY_ISREADONLY = u"IsReadOnly";
inline constexpr OUStringLiteral PROPERTY_FILTER = u"Filter";
inline constexpr OUStringLiteral PROPERTY_HAVINGCLAUSE = u"HavingClause";
inline constexpr OUStringLiteral PROPERTY_WIDTH = u"Width";
inline constexpr OUStringLiteral PROPERTY_SEARCHABLE = u"IsSearchable";
inline constexpr OUStringLiteral PROPERTY_MULTILINE = u"MultiLine";
inline constexpr OUStringLiteral PROPERTY_TARGET_URL = u"TargetURL";
inline constexpr OUStringLiteral PROPERTY_TARGET_FRAME = u"TargetFrame";
inline constexpr OUStringLiteral PROPERTY_DEFAULTCONTROL = u"DefaultControl";
inline constexpr OUStringLiteral PROPERTY_MAXTEXTLEN = u"MaxTextLen";
inline constexpr OUStringLiteral PROPERTY_EDITMASK = u"EditMask";
inline constexpr OUStringLiteral PROPERTY_SIZE = u"Size";
inline constexpr OUStringLiteral PROPERTY_SPIN = u"Spin";
inline constexpr OUStringLiteral PROPERTY_DATE = u"Date";
inline constexpr OUStringLiteral PROPERTY_TIME = u"Time";
inline constexpr OUStringLiteral PROPERTY_STATE = u"State";
inline constexpr OUStringLiteral PROPERTY_TRISTATE = u"TriState";
inline constexpr OUStringLiteral PROPERTY_HIDDEN_VALUE = u"HiddenValue";
inline constexpr OUStringLiteral PROPERTY_BUTTONTYPE = u"ButtonType";
inline constexpr OUStringLiteral PROPERTY_STRINGITEMLIST = u"StringItemList";
inline constexpr OUStringLiteral PROPERTY_TYPEDITEMLIST = u"TypedItemList";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_TEXT = u"DefaultText";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_STATE = u"DefaultState";
inline constexpr OUStringLiteral PROPERTY_FORMATKEY = u"FormatKey";
inline constexpr OUStringLiteral PROPERTY_FORMATSSUPPLIER = u"FormatsSupplier";
inline constexpr OUStringLiteral PROPERTY_SUBMIT_ACTION = u"SubmitAction";
inline constexpr OUStringLiteral PROPERTY_SUBMIT_TARGET = u"SubmitTarget";
inline constexpr OUStringLiteral PROPERTY_SUBMIT_METHOD = u"SubmitMethod";
inline constexpr OUStringLiteral PROPERTY_SUBMIT_ENCODING = u"SubmitEncoding";
inline constexpr OUStringLiteral PROPERTY_IMAGE_URL = u"ImageURL";
inline constexpr OUStringLiteral PROPERTY_GRAPHIC = u"Graphic";
inline constexpr OUStringLiteral PROPERTY_IMAGE_POSITION = u"ImagePosition";
inline constexpr OUStringLiteral PROPERTY_EMPTY_IS_NULL = u"ConvertEmptyToNull";
inline constexpr OUStringLiteral PROPERTY_LISTSOURCETYPE = u"ListSourceType";
inline constexpr OUStringLiteral PROPERTY_LISTSOURCE = u"ListSource";
inline constexpr OUStringLiteral PROPERTY_SELECT_SEQ = u"SelectedItems";
inline constexpr OUStringLiteral PROPERTY_VALUE_SEQ = u"ValueItemList";
inline constexpr OUStringLiteral PROPERTY_SELECT_VALUE_SEQ = u"SelectedValues";
inline constexpr OUStringLiteral PROPERTY_SELECT_VALUE = u"SelectedValue";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_SELECT_SEQ = u"DefaultSelection";
inline constexpr OUStringLiteral PROPERTY_MULTISELECTION = u"MultiSelection";
inline constexpr OUStringLiteral PROPERTY_ALIGN = u"Align";
inline constexpr OUStringLiteral PROPERTY_VERTICAL_ALIGN = u"VerticalAlign";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_DATE = u"DefaultDate";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_TIME = u"DefaultTime";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_VALUE = u"DefaultValue";
inline constexpr OUStringLiteral PROPERTY_DECIMAL_ACCURACY = u"DecimalAccuracy";
inline constexpr OUStringLiteral PROPERTY_FIELDTYPE = u"Type";
inline constexpr OUStringLiteral PROPERTY_DECIMALS = u"Decimals";
inline constexpr OUStringLiteral PROPERTY_REFVALUE = u"RefValue";
inline constexpr OUStringLiteral PROPERTY_UNCHECKED_REFVALUE = u"SecondaryRefValue";
inline constexpr OUStringLiteral PROPERTY_VALUEMIN = u"ValueMin";
inline constexpr OUStringLiteral PROPERTY_VALUEMAX = u"ValueMax";
inline constexpr OUStringLiteral PROPERTY_STRICTFORMAT = u"StrictFormat";
inline constexpr OUStringLiteral PROPERTY_ALLOWADDITIONS = u"AllowInserts";
inline constexpr OUStringLiteral PROPERTY_ALLOWEDITS = u"AllowUpdates";
inline constexpr OUStringLiteral PROPERTY_ALLOWDELETIONS = u"AllowDeletes";
inline constexpr OUStringLiteral PROPERTY_MASTERFIELDS = u"MasterFields";
inline constexpr OUStringLiteral PROPERTY_ISPASSTHROUGH = u"IsPassThrough";
inline constexpr OUStringLiteral PROPERTY_QUERY = u"Query";
inline constexpr OUStringLiteral PROPERTY_LITERALMASK = u"LiteralMask";
inline constexpr OUStringLiteral PROPERTY_VALUESTEP = u"ValueStep";
inline constexpr OUStringLiteral PROPERTY_SHOWTHOUSANDSEP = u"ShowThousandsSeparator";
inline constexpr OUStringLiteral PROPERTY_CURRENCYSYMBOL = u"CurrencySymbol";
inline constexpr OUStringLiteral PROPERTY_DATEFORMAT = u"DateFormat";
inline constexpr OUStringLiteral PROPERTY_DATEMIN = u"DateMin";
inline constexpr OUStringLiteral PROPERTY_DATEMAX = u"DateMax";
inline constexpr OUStringLiteral PROPERTY_DATE_SHOW_CENTURY = u"DateShowCentury";
inline constexpr OUStringLiteral PROPERTY_TIMEFORMAT = u"TimeFormat";
inline constexpr OUStringLiteral PROPERTY_TIMEMIN = u"TimeMin";
inline constexpr OUStringLiteral PROPERTY_TIMEMAX = u"TimeMax";
inline constexpr OUStringLiteral PROPERTY_LINECOUNT = u"LineCount";
inline constexpr OUStringLiteral PROPERTY_BOUNDCOLUMN = u"BoundColumn";
inline constexpr OUStringLiteral PROPERTY_FONT = u"FontDescriptor";
inline constexpr OUStringLiteral PROPERTY_FILLCOLOR = u"FillColor";
inline constexpr OUStringLiteral PROPERTY_LINECOLOR = u"LineColor";
inline constexpr OUStringLiteral PROPERTY_DROPDOWN = u"Dropdown";
inline constexpr OUStringLiteral PROPERTY_HSCROLL = u"HScroll";
inline constexpr OUStringLiteral PROPERTY_VSCROLL = u"VScroll";
inline constexpr OUStringLiteral PROPERTY_TABSTOP = u"Tabstop";
inline constexpr OUStringLiteral PROPERTY_AUTOCOMPLETE = u"Autocomplete";
inline constexpr OUStringLiteral PROPERTY_HARDLINEBREAKS = u"HardLineBreaks";
inline constexpr OUStringLiteral PROPERTY_PRINTABLE = u"Printable";
inline constexpr OUStringLiteral PROPERTY_ECHO_CHAR = u"EchoChar";
inline constexpr OUStringLiteral PROPERTY_ROWHEIGHT = u"RowHeight";
inline constexpr OUStringLiteral PROPERTY_HELPTEXT = u"HelpText";
inline constexpr OUStringLiteral PROPERTY_FONT_NAME = u"FontName";
inline constexpr OUStringLiteral PROPERTY_FONT_STYLENAME = u"FontStyleName";
inline constexpr OUStringLiteral PROPERTY_FONT_FAMILY = u"FontFamily";
inline constexpr OUStringLiteral PROPERTY_FONT_CHARSET = u"FontCharset";
inline constexpr OUStringLiteral PROPERTY_FONT_HEIGHT = u"FontHeight";
inline constexpr OUStringLiteral PROPERTY_FONT_WEIGHT = u"FontWeight";
inline constexpr OUStringLiteral PROPERTY_FONT_SLANT = u"FontSlant";
inline constexpr OUStringLiteral PROPERTY_FONT_UNDERLINE = u"FontUnderline";
inline constexpr OUStringLiteral PROPERTY_FONT_WORDLINEMODE = u"FontWordLineMode";
inline constexpr OUStringLiteral PROPERTY_FONT_STRIKEOUT = u"FontStrikeout";
inline constexpr OUStringLiteral PROPERTY_FONTEMPHASISMARK = u"FontEmphasisMark";
inline constexpr OUStringLiteral PROPERTY_FONTRELIEF = u"FontRelief";
inline constexpr OUStringLiteral PROPERTY_FONT_CHARWIDTH = u"FontCharWidth";
inline constexpr OUStringLiteral PROPERTY_FONT_KERNING = u"FontKerning";
inline constexpr OUStringLiteral PROPERTY_FONT_ORIENTATION = u"FontOrientation";
inline constexpr OUStringLiteral PROPERTY_FONT_PITCH = u"FontPitch";
inline constexpr OUStringLiteral PROPERTY_FONT_TYPE = u"FontType";
inline constexpr OUStringLiteral PROPERTY_FONT_WIDTH = u"FontWidth";
inline constexpr OUStringLiteral PROPERTY_HELPURL = u"HelpURL";
inline constexpr OUStringLiteral PROPERTY_RECORDMARKER = u"HasRecordMarker";
inline constexpr OUStringLiteral PROPERTY_BOUNDFIELD = u"BoundField";
inline constexpr OUStringLiteral PROPERTY_INPUT_REQUIRED = u"InputRequired";
inline constexpr OUStringLiteral PROPERTY_TREATASNUMERIC = u"TreatAsNumber";
inline constexpr OUStringLiteral PROPERTY_EFFECTIVE_VALUE = u"EffectiveValue";
inline constexpr OUStringLiteral PROPERTY_EFFECTIVE_DEFAULT = u"EffectiveDefault";
inline constexpr OUStringLiteral PROPERTY_EFFECTIVE_MIN = u"EffectiveMin";
inline constexpr OUStringLiteral PROPERTY_EFFECTIVE_MAX = u"EffectiveMax";
inline constexpr OUStringLiteral PROPERTY_HIDDEN = u"Hidden";
inline constexpr OUStringLiteral PROPERTY_FILTERPROPOSAL = u"UseFilterValueProposal";
inline constexpr OUStringLiteral PROPERTY_FIELDSOURCE = u"FieldSource";
inline constexpr OUStringLiteral PROPERTY_TABLENAME = u"TableName";
inline constexpr OUStringLiteral PROPERTY_CONTROLLABEL = u"LabelControl";
inline constexpr OUStringLiteral PROPERTY_CURRSYM_POSITION = u"PrependCurrencySymbol";
inline constexpr OUStringLiteral PROPERTY_CURSORCOLOR = u"CursorColor";
inline constexpr OUStringLiteral PROPERTY_ALWAYSSHOWCURSOR = u"AlwaysShowCursor";
inline constexpr OUStringLiteral PROPERTY_DISPLAYSYNCHRON = u"DisplayIsSynchron";
inline constexpr OUStringLiteral PROPERTY_TEXTCOLOR = u"TextColor";
inline constexpr OUStringLiteral PROPERTY_DELAY = u"RepeatDelay";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_SCROLL_VALUE = u"DefaultScrollValue";
inline constexpr OUStringLiteral PROPERTY_SCROLL_VALUE = u"ScrollValue";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_SPIN_VALUE = u"DefaultSpinValue";
inline constexpr OUStringLiteral PROPERTY_SPIN_VALUE = u"SpinValue";
inline constexpr OUStringLiteral PROPERTY_REFERENCE_DEVICE = u"ReferenceDevice";
inline constexpr OUStringLiteral PROPERTY_ISMODIFIED = u"IsModified";
inline constexpr OUStringLiteral PROPERTY_ISNEW = u"IsNew";
inline constexpr OUStringLiteral PROPERTY_PRIVILEGES = u"Privileges";
inline constexpr OUStringLiteral PROPERTY_COMMAND = u"Command";
inline constexpr OUStringLiteral PROPERTY_COMMANDTYPE = u"CommandType";
inline constexpr OUStringLiteral PROPERTY_RESULTSET_CONCURRENCY = u"ResultSetConcurrency";
inline constexpr OUStringLiteral PROPERTY_INSERTONLY = u"IgnoreResult";
inline constexpr OUStringLiteral PROPERTY_RESULTSET_TYPE = u"ResultSetType";
inline constexpr OUStringLiteral PROPERTY_ESCAPE_PROCESSING = u"EscapeProcessing";
inline constexpr OUStringLiteral PROPERTY_APPLYFILTER = u"ApplyFilter";
inline constexpr OUStringLiteral PROPERTY_ROWCOUNT = u"RowCount";
inline constexpr OUStringLiteral PROPERTY_ROWCOUNTFINAL = u"IsRowCountFinal";

inline constexpr OUStringLiteral PROPERTY_ISNULLABLE = u"IsNullable";
inline constexpr OUStringLiteral PROPERTY_ACTIVECOMMAND = u"ActiveCommand";
inline constexpr OUStringLiteral PROPERTY_ISCURRENCY = u"IsCurrency";
inline constexpr OUStringLiteral PROPERTY_URL = u"URL";
inline constexpr OUStringLiteral PROPERTY_TITLE = u"Title";
inline constexpr OUStringLiteral PROPERTY_ACTIVE_CONNECTION = u"ActiveConnection";
inline constexpr OUStringLiteral PROPERTY_SCALE = u"Scale";
inline constexpr OUStringLiteral PROPERTY_SORT = u"Order";
inline constexpr OUStringLiteral PROPERTY_DATASOURCE = u"DataSourceName";
inline constexpr OUStringLiteral PROPERTY_DETAILFIELDS = u"DetailFields";

inline constexpr OUStringLiteral PROPERTY_COLUMNSERVICENAME = u"ColumnServiceName";
inline constexpr OUStringLiteral PROPERTY_REALNAME = u"RealName";
inline constexpr OUStringLiteral PROPERTY_CONTROLSOURCEPROPERTY = u"DataFieldProperty";
inline constexpr OUStringLiteral PROPERTY_USER = u"User";
inline constexpr OUStringLiteral PROPERTY_PASSWORD = u"Password";
inline constexpr OUStringLiteral PROPERTY_DISPATCHURLINTERNAL = u"DispatchURLInternal";
inline constexpr OUStringLiteral PROPERTY_PERSISTENCE_MAXTEXTLENGTH = u"PersistenceMaxTextLength";
inline constexpr OUStringLiteral PROPERTY_RICH_TEXT = u"RichText";
inline constexpr OUStringLiteral PROPERTY_ENFORCE_FORMAT = u"EnforceFormat";
inline constexpr OUStringLiteral PROPERTY_LINEEND_FORMAT = u"LineEndFormat";
inline constexpr OUStringLiteral PROPERTY_WRITING_MODE = u"WritingMode";
inline constexpr OUStringLiteral PROPERTY_CONTEXT_WRITING_MODE = u"ContextWritingMode";

inline constexpr OUStringLiteral PROPERTY_NATIVE_LOOK = u"NativeWidgetLook";
inline constexpr OUStringLiteral PROPERTY_BORDER = u"Border";
inline constexpr OUStringLiteral PROPERTY_BORDERCOLOR = u"BorderColor";
inline constexpr OUStringLiteral PROPERTY_BACKGROUNDCOLOR = u"BackgroundColor";
inline constexpr OUStringLiteral PROPERTY_ICONSIZE = u"IconSize";
inline constexpr OUStringLiteral PROPERTY_TEXTLINECOLOR = u"TextLineColor";
inline constexpr OUStringLiteral PROPERTY_HIDEINACTIVESELECTION = u"HideInactiveSelection";
inline constexpr OUStringLiteral PROPERTY_HIGHLIGHT_COLOR = u"HighlightColor";
inline constexpr OUStringLiteral PROPERTY_HIGHLIGHT_TEXT_COLOR = u"HighlightTextColor";

inline constexpr OUStringLiteral PROPERTY_SHOW_POSITION = u"ShowPosition";
inline constexpr OUStringLiteral PROPERTY_SHOW_NAVIGATION = u"ShowNavigation";
inline constexpr OUStringLiteral PROPERTY_SHOW_RECORDACTIONS = u"ShowRecordActions";
inline constexpr OUStringLiteral PROPERTY_SHOW_FILTERSORT = u"ShowFilterSort";

inline constexpr OUStringLiteral PROPERTY_XSD_WHITESPACE = u"WhiteSpace";
inline constexpr OUStringLiteral PROPERTY_XSD_PATTERN = u"Pattern";
inline constexpr OUStringLiteral PROPERTY_XSD_LENGTH = u"Length";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_LENGTH = u"MinLength";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_LENGTH = u"MaxLength";
inline constexpr OUStringLiteral PROPERTY_XSD_TOTAL_DIGITS = u"TotalDigits";
inline constexpr OUStringLiteral PROPERTY_XSD_FRACTION_DIGITS = u"FractionDigits";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_INT = u"MaxInclusiveInt";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_INT = u"MaxExclusiveInt";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_INT = u"MinInclusiveInt";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_INT = u"MinExclusiveInt";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_DOUBLE = u"MaxInclusiveDouble";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_DOUBLE = u"MaxExclusiveDouble";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_DOUBLE = u"MinInclusiveDouble";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_DOUBLE = u"MinExclusiveDouble";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_DATE = u"MaxInclusiveDate";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_DATE = u"MaxExclusiveDate";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_DATE = u"MinInclusiveDate";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_DATE = u"MinExclusiveDate";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_TIME = u"MaxInclusiveTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_TIME = u"MaxExclusiveTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_TIME = u"MinInclusiveTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_TIME = u"MinExclusiveTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_DATE_TIME = u"MaxInclusiveDateTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_DATE_TIME = u"MaxExclusiveDateTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_DATE_TIME = u"MinInclusiveDateTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_DATE_TIME = u"MinExclusiveDateTime";
inline constexpr OUStringLiteral PROPERTY_XSD_IS_BASIC = u"IsBasic";
inline constexpr OUStringLiteral PROPERTY_XSD_TYPE_CLASS = u"TypeClass";

inline constexpr OUStringLiteral PROPERTY_DYNAMIC_CONTROL_BORDER = u"DynamicControlBorder";
inline constexpr OUStringLiteral PROPERTY_CONTROL_BORDER_COLOR_FOCUS = u"ControlBorderColorOnFocus";
inline constexpr OUStringLiteral PROPERTY_CONTROL_BORDER_COLOR_MOUSE = u"ControlBorderColorOnHover";
inline constexpr OUStringLiteral PROPERTY_CONTROL_BORDER_COLOR_INVALID = u"ControlBorderColorOnInvalid";
inline constexpr OUStringLiteral PROPERTY_GENERATEVBAEVENTS = u"GenerateVbaEvents";
inline constexpr OUStringLiteral PROPERTY_CONTROL_TYPE_IN_MSO = u"ControlTypeinMSO";
inline constexpr OUStringLiteral PROPERTY_OBJ_ID_IN_MSO = u"ObjIDinMSO";


//- URLs

#define URL_FORM_POSITION     ".uno:FormController/positionForm"
#define URL_FORM_RECORDCOUNT  ".uno:FormController/RecordCount"
#define URL_RECORD_FIRST      ".uno:FormController/moveToFirst"
#define URL_RECORD_PREV       ".uno:FormController/moveToPrev"
#define URL_RECORD_NEXT       ".uno:FormController/moveToNext"
#define URL_RECORD_LAST       ".uno:FormController/moveToLast"
#define URL_RECORD_SAVE       ".uno:FormController/saveRecord"
#define URL_RECORD_UNDO       ".uno:FormController/undoRecord"
#define URL_RECORD_NEW        ".uno:FormController/moveToNew"
#define URL_RECORD_DELETE     ".uno:FormController/deleteRecord"
#define URL_FORM_REFRESH      ".uno:FormController/refreshForm"
#define URL_FORM_REFRESH_CURRENT_CONTROL ".uno:FormController/refreshCurrentControl"

#define URL_FORM_SORT_UP      ".uno:FormController/sortUp"
#define URL_FORM_SORT_DOWN    ".uno:FormController/sortDown"
#define URL_FORM_SORT         ".uno:FormController/sort"
#define URL_FORM_AUTO_FILTER  ".uno:FormController/autoFilter"
#define URL_FORM_FILTER       ".uno:FormController/filter"
#define URL_FORM_APPLY_FILTER ".uno:FormController/applyFilter"
#define URL_FORM_REMOVE_FILTER ".uno:FormController/removeFilterOrder"


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
