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

inline constexpr OUString PROPERTY_TABINDEX = u"TabIndex"_ustr;
inline constexpr OUString PROPERTY_TAG = u"Tag"_ustr;
inline constexpr OUString PROPERTY_NAME = u"Name"_ustr;
inline constexpr OUString PROPERTY_GROUP_NAME = u"GroupName"_ustr;
inline constexpr OUString PROPERTY_CLASSID = u"ClassId"_ustr;
inline constexpr OUString PROPERTY_FETCHSIZE = u"FetchSize"_ustr;
inline constexpr OUString PROPERTY_VALUE = u"Value"_ustr;
inline constexpr OUString PROPERTY_TEXT = u"Text"_ustr;
inline constexpr OUString PROPERTY_LABEL = u"Label"_ustr;
inline constexpr OUString PROPERTY_NAVIGATION = u"NavigationBarMode"_ustr;
inline constexpr OUString PROPERTY_HASNAVIGATION = u"HasNavigationBar"_ustr;
inline constexpr OUString PROPERTY_CYCLE = u"Cycle"_ustr;
inline constexpr OUString PROPERTY_CONTROLSOURCE = u"DataField"_ustr;
inline constexpr OUString PROPERTY_ENABLED = u"Enabled"_ustr;
inline constexpr OUString PROPERTY_ENABLEVISIBLE = u"EnableVisible"_ustr;
inline constexpr OUString PROPERTY_READONLY = u"ReadOnly"_ustr;
inline constexpr OUString PROPERTY_RELEVANT = u"Relevant"_ustr;
inline constexpr OUString PROPERTY_ISREADONLY = u"IsReadOnly"_ustr;
inline constexpr OUString PROPERTY_FILTER = u"Filter"_ustr;
inline constexpr OUString PROPERTY_HAVINGCLAUSE = u"HavingClause"_ustr;
inline constexpr OUString PROPERTY_WIDTH = u"Width"_ustr;
inline constexpr OUString PROPERTY_SEARCHABLE = u"IsSearchable"_ustr;
inline constexpr OUString PROPERTY_MULTILINE = u"MultiLine"_ustr;
inline constexpr OUString PROPERTY_TARGET_URL = u"TargetURL"_ustr;
inline constexpr OUString PROPERTY_TARGET_FRAME = u"TargetFrame"_ustr;
inline constexpr OUString PROPERTY_DEFAULTCONTROL = u"DefaultControl"_ustr;
inline constexpr OUString PROPERTY_MAXTEXTLEN = u"MaxTextLen"_ustr;
inline constexpr OUString PROPERTY_EDITMASK = u"EditMask"_ustr;
inline constexpr OUString PROPERTY_SIZE = u"Size"_ustr;
inline constexpr OUString PROPERTY_SPIN = u"Spin"_ustr;
inline constexpr OUString PROPERTY_DATE = u"Date"_ustr;
inline constexpr OUString PROPERTY_TIME = u"Time"_ustr;
inline constexpr OUString PROPERTY_STATE = u"State"_ustr;
inline constexpr OUString PROPERTY_TRISTATE = u"TriState"_ustr;
inline constexpr OUString PROPERTY_HIDDEN_VALUE = u"HiddenValue"_ustr;
inline constexpr OUString PROPERTY_BUTTONTYPE = u"ButtonType"_ustr;
inline constexpr OUString PROPERTY_STRINGITEMLIST = u"StringItemList"_ustr;
inline constexpr OUString PROPERTY_TYPEDITEMLIST = u"TypedItemList"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_TEXT = u"DefaultText"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_STATE = u"DefaultState"_ustr;
inline constexpr OUString PROPERTY_FORMATKEY = u"FormatKey"_ustr;
inline constexpr OUString PROPERTY_FORMATSSUPPLIER = u"FormatsSupplier"_ustr;
inline constexpr OUString PROPERTY_SUBMIT_ACTION = u"SubmitAction"_ustr;
inline constexpr OUString PROPERTY_SUBMIT_TARGET = u"SubmitTarget"_ustr;
inline constexpr OUString PROPERTY_SUBMIT_METHOD = u"SubmitMethod"_ustr;
inline constexpr OUString PROPERTY_SUBMIT_ENCODING = u"SubmitEncoding"_ustr;
inline constexpr OUString PROPERTY_IMAGE_URL = u"ImageURL"_ustr;
inline constexpr OUString PROPERTY_GRAPHIC = u"Graphic"_ustr;
inline constexpr OUString PROPERTY_IMAGE_POSITION = u"ImagePosition"_ustr;
inline constexpr OUString PROPERTY_EMPTY_IS_NULL = u"ConvertEmptyToNull"_ustr;
inline constexpr OUString PROPERTY_LISTSOURCETYPE = u"ListSourceType"_ustr;
inline constexpr OUString PROPERTY_LISTSOURCE = u"ListSource"_ustr;
inline constexpr OUString PROPERTY_SELECT_SEQ = u"SelectedItems"_ustr;
inline constexpr OUString PROPERTY_VALUE_SEQ = u"ValueItemList"_ustr;
inline constexpr OUString PROPERTY_SELECT_VALUE_SEQ = u"SelectedValues"_ustr;
inline constexpr OUString PROPERTY_SELECT_VALUE = u"SelectedValue"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_SELECT_SEQ = u"DefaultSelection"_ustr;
inline constexpr OUString PROPERTY_MULTISELECTION = u"MultiSelection"_ustr;
inline constexpr OUString PROPERTY_ALIGN = u"Align"_ustr;
inline constexpr OUString PROPERTY_VERTICAL_ALIGN = u"VerticalAlign"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_DATE = u"DefaultDate"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_TIME = u"DefaultTime"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_VALUE = u"DefaultValue"_ustr;
inline constexpr OUString PROPERTY_DECIMAL_ACCURACY = u"DecimalAccuracy"_ustr;
inline constexpr OUString PROPERTY_FIELDTYPE = u"Type"_ustr;
inline constexpr OUString PROPERTY_DECIMALS = u"Decimals"_ustr;
inline constexpr OUString PROPERTY_REFVALUE = u"RefValue"_ustr;
inline constexpr OUString PROPERTY_UNCHECKED_REFVALUE = u"SecondaryRefValue"_ustr;
inline constexpr OUString PROPERTY_VALUEMIN = u"ValueMin"_ustr;
inline constexpr OUString PROPERTY_VALUEMAX = u"ValueMax"_ustr;
inline constexpr OUString PROPERTY_STRICTFORMAT = u"StrictFormat"_ustr;
inline constexpr OUString PROPERTY_ALLOWADDITIONS = u"AllowInserts"_ustr;
inline constexpr OUString PROPERTY_ALLOWEDITS = u"AllowUpdates"_ustr;
inline constexpr OUString PROPERTY_ALLOWDELETIONS = u"AllowDeletes"_ustr;
inline constexpr OUString PROPERTY_MASTERFIELDS = u"MasterFields"_ustr;
inline constexpr OUString PROPERTY_ISPASSTHROUGH = u"IsPassThrough"_ustr;
inline constexpr OUString PROPERTY_QUERY = u"Query"_ustr;
inline constexpr OUString PROPERTY_LITERALMASK = u"LiteralMask"_ustr;
inline constexpr OUString PROPERTY_VALUESTEP = u"ValueStep"_ustr;
inline constexpr OUString PROPERTY_SHOWTHOUSANDSEP = u"ShowThousandsSeparator"_ustr;
inline constexpr OUString PROPERTY_CURRENCYSYMBOL = u"CurrencySymbol"_ustr;
inline constexpr OUString PROPERTY_DATEFORMAT = u"DateFormat"_ustr;
inline constexpr OUString PROPERTY_DATEMIN = u"DateMin"_ustr;
inline constexpr OUString PROPERTY_DATEMAX = u"DateMax"_ustr;
inline constexpr OUString PROPERTY_DATE_SHOW_CENTURY = u"DateShowCentury"_ustr;
inline constexpr OUString PROPERTY_TIMEFORMAT = u"TimeFormat"_ustr;
inline constexpr OUString PROPERTY_TIMEMIN = u"TimeMin"_ustr;
inline constexpr OUString PROPERTY_TIMEMAX = u"TimeMax"_ustr;
inline constexpr OUString PROPERTY_LINECOUNT = u"LineCount"_ustr;
inline constexpr OUString PROPERTY_BOUNDCOLUMN = u"BoundColumn"_ustr;
inline constexpr OUString PROPERTY_FONT = u"FontDescriptor"_ustr;
inline constexpr OUString PROPERTY_FILLCOLOR = u"FillColor"_ustr;
inline constexpr OUString PROPERTY_LINECOLOR = u"LineColor"_ustr;
inline constexpr OUString PROPERTY_DROPDOWN = u"Dropdown"_ustr;
inline constexpr OUString PROPERTY_HSCROLL = u"HScroll"_ustr;
inline constexpr OUString PROPERTY_VSCROLL = u"VScroll"_ustr;
inline constexpr OUString PROPERTY_TABSTOP = u"Tabstop"_ustr;
inline constexpr OUString PROPERTY_AUTOCOMPLETE = u"Autocomplete"_ustr;
inline constexpr OUString PROPERTY_HARDLINEBREAKS = u"HardLineBreaks"_ustr;
inline constexpr OUString PROPERTY_PRINTABLE = u"Printable"_ustr;
inline constexpr OUString PROPERTY_ECHO_CHAR = u"EchoChar"_ustr;
inline constexpr OUString PROPERTY_ROWHEIGHT = u"RowHeight"_ustr;
inline constexpr OUString PROPERTY_HELPTEXT = u"HelpText"_ustr;
inline constexpr OUString PROPERTY_FONT_NAME = u"FontName"_ustr;
inline constexpr OUString PROPERTY_FONT_STYLENAME = u"FontStyleName"_ustr;
inline constexpr OUString PROPERTY_FONT_FAMILY = u"FontFamily"_ustr;
inline constexpr OUString PROPERTY_FONT_CHARSET = u"FontCharset"_ustr;
inline constexpr OUString PROPERTY_FONT_HEIGHT = u"FontHeight"_ustr;
inline constexpr OUString PROPERTY_FONT_WEIGHT = u"FontWeight"_ustr;
inline constexpr OUString PROPERTY_FONT_SLANT = u"FontSlant"_ustr;
inline constexpr OUString PROPERTY_FONT_UNDERLINE = u"FontUnderline"_ustr;
inline constexpr OUString PROPERTY_FONT_WORDLINEMODE = u"FontWordLineMode"_ustr;
inline constexpr OUString PROPERTY_FONT_STRIKEOUT = u"FontStrikeout"_ustr;
inline constexpr OUString PROPERTY_FONTEMPHASISMARK = u"FontEmphasisMark"_ustr;
inline constexpr OUString PROPERTY_FONTRELIEF = u"FontRelief"_ustr;
inline constexpr OUString PROPERTY_FONT_CHARWIDTH = u"FontCharWidth"_ustr;
inline constexpr OUString PROPERTY_FONT_KERNING = u"FontKerning"_ustr;
inline constexpr OUString PROPERTY_FONT_ORIENTATION = u"FontOrientation"_ustr;
inline constexpr OUString PROPERTY_FONT_PITCH = u"FontPitch"_ustr;
inline constexpr OUString PROPERTY_FONT_TYPE = u"FontType"_ustr;
inline constexpr OUString PROPERTY_FONT_WIDTH = u"FontWidth"_ustr;
inline constexpr OUString PROPERTY_HELPURL = u"HelpURL"_ustr;
inline constexpr OUString PROPERTY_RECORDMARKER = u"HasRecordMarker"_ustr;
inline constexpr OUString PROPERTY_BOUNDFIELD = u"BoundField"_ustr;
inline constexpr OUString PROPERTY_INPUT_REQUIRED = u"InputRequired"_ustr;
inline constexpr OUString PROPERTY_TREATASNUMERIC = u"TreatAsNumber"_ustr;
inline constexpr OUString PROPERTY_EFFECTIVE_VALUE = u"EffectiveValue"_ustr;
inline constexpr OUString PROPERTY_EFFECTIVE_DEFAULT = u"EffectiveDefault"_ustr;
inline constexpr OUString PROPERTY_EFFECTIVE_MIN = u"EffectiveMin"_ustr;
inline constexpr OUString PROPERTY_EFFECTIVE_MAX = u"EffectiveMax"_ustr;
inline constexpr OUString PROPERTY_HIDDEN = u"Hidden"_ustr;
inline constexpr OUString PROPERTY_FILTERPROPOSAL = u"UseFilterValueProposal"_ustr;
inline constexpr OUString PROPERTY_FIELDSOURCE = u"FieldSource"_ustr;
inline constexpr OUString PROPERTY_TABLENAME = u"TableName"_ustr;
inline constexpr OUString PROPERTY_CONTROLLABEL = u"LabelControl"_ustr;
inline constexpr OUString PROPERTY_CURRSYM_POSITION = u"PrependCurrencySymbol"_ustr;
inline constexpr OUString PROPERTY_CURSORCOLOR = u"CursorColor"_ustr;
inline constexpr OUString PROPERTY_ALWAYSSHOWCURSOR = u"AlwaysShowCursor"_ustr;
inline constexpr OUString PROPERTY_DISPLAYSYNCHRON = u"DisplayIsSynchron"_ustr;
inline constexpr OUString PROPERTY_TEXTCOLOR = u"TextColor"_ustr;
inline constexpr OUString PROPERTY_DELAY = u"RepeatDelay"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_SCROLL_VALUE = u"DefaultScrollValue"_ustr;
inline constexpr OUString PROPERTY_SCROLL_VALUE = u"ScrollValue"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_SPIN_VALUE = u"DefaultSpinValue"_ustr;
inline constexpr OUString PROPERTY_SPIN_VALUE = u"SpinValue"_ustr;
inline constexpr OUString PROPERTY_REFERENCE_DEVICE = u"ReferenceDevice"_ustr;
inline constexpr OUString PROPERTY_ISMODIFIED = u"IsModified"_ustr;
inline constexpr OUString PROPERTY_ISNEW = u"IsNew"_ustr;
inline constexpr OUString PROPERTY_PRIVILEGES = u"Privileges"_ustr;
inline constexpr OUString PROPERTY_COMMAND = u"Command"_ustr;
inline constexpr OUString PROPERTY_COMMANDTYPE = u"CommandType"_ustr;
inline constexpr OUString PROPERTY_RESULTSET_CONCURRENCY = u"ResultSetConcurrency"_ustr;
inline constexpr OUString PROPERTY_INSERTONLY = u"IgnoreResult"_ustr;
inline constexpr OUString PROPERTY_RESULTSET_TYPE = u"ResultSetType"_ustr;
inline constexpr OUString PROPERTY_ESCAPE_PROCESSING = u"EscapeProcessing"_ustr;
inline constexpr OUString PROPERTY_APPLYFILTER = u"ApplyFilter"_ustr;
inline constexpr OUString PROPERTY_ROWCOUNT = u"RowCount"_ustr;
inline constexpr OUString PROPERTY_ROWCOUNTFINAL = u"IsRowCountFinal"_ustr;

inline constexpr OUString PROPERTY_ISNULLABLE = u"IsNullable"_ustr;
inline constexpr OUString PROPERTY_ACTIVECOMMAND = u"ActiveCommand"_ustr;
inline constexpr OUString PROPERTY_ISCURRENCY = u"IsCurrency"_ustr;
inline constexpr OUString PROPERTY_URL = u"URL"_ustr;
inline constexpr OUString PROPERTY_TITLE = u"Title"_ustr;
inline constexpr OUString PROPERTY_ACTIVE_CONNECTION = u"ActiveConnection"_ustr;
inline constexpr OUString PROPERTY_SCALE = u"Scale"_ustr;
inline constexpr OUString PROPERTY_SORT = u"Order"_ustr;
inline constexpr OUString PROPERTY_DATASOURCE = u"DataSourceName"_ustr;
inline constexpr OUString PROPERTY_DETAILFIELDS = u"DetailFields"_ustr;

inline constexpr OUString PROPERTY_COLUMNSERVICENAME = u"ColumnServiceName"_ustr;
inline constexpr OUString PROPERTY_REALNAME = u"RealName"_ustr;
inline constexpr OUString PROPERTY_CONTROLSOURCEPROPERTY = u"DataFieldProperty"_ustr;
inline constexpr OUString PROPERTY_USER = u"User"_ustr;
inline constexpr OUString PROPERTY_PASSWORD = u"Password"_ustr;
inline constexpr OUString PROPERTY_DISPATCHURLINTERNAL = u"DispatchURLInternal"_ustr;
inline constexpr OUString PROPERTY_PERSISTENCE_MAXTEXTLENGTH = u"PersistenceMaxTextLength"_ustr;
inline constexpr OUString PROPERTY_RICH_TEXT = u"RichText"_ustr;
inline constexpr OUString PROPERTY_ENFORCE_FORMAT = u"EnforceFormat"_ustr;
inline constexpr OUString PROPERTY_LINEEND_FORMAT = u"LineEndFormat"_ustr;
inline constexpr OUString PROPERTY_WRITING_MODE = u"WritingMode"_ustr;
inline constexpr OUString PROPERTY_CONTEXT_WRITING_MODE = u"ContextWritingMode"_ustr;

inline constexpr OUString PROPERTY_NATIVE_LOOK = u"NativeWidgetLook"_ustr;
inline constexpr OUString PROPERTY_BORDER = u"Border"_ustr;
inline constexpr OUString PROPERTY_BORDERCOLOR = u"BorderColor"_ustr;
inline constexpr OUString PROPERTY_BACKGROUNDCOLOR = u"BackgroundColor"_ustr;
inline constexpr OUString PROPERTY_ICONSIZE = u"IconSize"_ustr;
inline constexpr OUString PROPERTY_TEXTLINECOLOR = u"TextLineColor"_ustr;
inline constexpr OUString PROPERTY_HIDEINACTIVESELECTION = u"HideInactiveSelection"_ustr;

inline constexpr OUString PROPERTY_STANDARD_THEME = u"StandardTheme"_ustr;

inline constexpr OUString PROPERTY_SHOW_POSITION = u"ShowPosition"_ustr;
inline constexpr OUString PROPERTY_SHOW_NAVIGATION = u"ShowNavigation"_ustr;
inline constexpr OUString PROPERTY_SHOW_RECORDACTIONS = u"ShowRecordActions"_ustr;
inline constexpr OUString PROPERTY_SHOW_FILTERSORT = u"ShowFilterSort"_ustr;

inline constexpr OUString PROPERTY_XSD_WHITESPACE = u"WhiteSpace"_ustr;
inline constexpr OUString PROPERTY_XSD_PATTERN = u"Pattern"_ustr;
inline constexpr OUString PROPERTY_XSD_LENGTH = u"Length"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_LENGTH = u"MinLength"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_LENGTH = u"MaxLength"_ustr;
inline constexpr OUString PROPERTY_XSD_TOTAL_DIGITS = u"TotalDigits"_ustr;
inline constexpr OUString PROPERTY_XSD_FRACTION_DIGITS = u"FractionDigits"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_INCLUSIVE_INT = u"MaxInclusiveInt"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_EXCLUSIVE_INT = u"MaxExclusiveInt"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_INCLUSIVE_INT = u"MinInclusiveInt"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_EXCLUSIVE_INT = u"MinExclusiveInt"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_INCLUSIVE_DOUBLE = u"MaxInclusiveDouble"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_EXCLUSIVE_DOUBLE = u"MaxExclusiveDouble"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_INCLUSIVE_DOUBLE = u"MinInclusiveDouble"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_EXCLUSIVE_DOUBLE = u"MinExclusiveDouble"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_INCLUSIVE_DATE = u"MaxInclusiveDate"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_EXCLUSIVE_DATE = u"MaxExclusiveDate"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_INCLUSIVE_DATE = u"MinInclusiveDate"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_EXCLUSIVE_DATE = u"MinExclusiveDate"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_INCLUSIVE_TIME = u"MaxInclusiveTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_EXCLUSIVE_TIME = u"MaxExclusiveTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_INCLUSIVE_TIME = u"MinInclusiveTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_EXCLUSIVE_TIME = u"MinExclusiveTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_INCLUSIVE_DATE_TIME = u"MaxInclusiveDateTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_EXCLUSIVE_DATE_TIME = u"MaxExclusiveDateTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_INCLUSIVE_DATE_TIME = u"MinInclusiveDateTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_EXCLUSIVE_DATE_TIME = u"MinExclusiveDateTime"_ustr;
inline constexpr OUString PROPERTY_XSD_IS_BASIC = u"IsBasic"_ustr;
inline constexpr OUString PROPERTY_XSD_TYPE_CLASS = u"TypeClass"_ustr;

inline constexpr OUString PROPERTY_DYNAMIC_CONTROL_BORDER = u"DynamicControlBorder"_ustr;
inline constexpr OUString PROPERTY_CONTROL_BORDER_COLOR_FOCUS = u"ControlBorderColorOnFocus"_ustr;
inline constexpr OUString PROPERTY_CONTROL_BORDER_COLOR_MOUSE = u"ControlBorderColorOnHover"_ustr;
inline constexpr OUString PROPERTY_CONTROL_BORDER_COLOR_INVALID = u"ControlBorderColorOnInvalid"_ustr;
inline constexpr OUString PROPERTY_GENERATEVBAEVENTS = u"GenerateVbaEvents"_ustr;
inline constexpr OUString PROPERTY_CONTROL_TYPE_IN_MSO = u"ControlTypeinMSO"_ustr;
inline constexpr OUString PROPERTY_OBJ_ID_IN_MSO = u"ObjIDinMSO"_ustr;


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
