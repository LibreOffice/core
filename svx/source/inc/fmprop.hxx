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
#ifndef INCLUDED_SVX_SOURCE_INC_FMPROP_HXX
#define INCLUDED_SVX_SOURCE_INC_FMPROP_HXX

#include <rtl/ustring.hxx>

// PropertyIds, which have an assignment to a PropertyName, could continue
// to be WhichIds in the future -> Itemset
#define FM_ATTR_START           0

#define FM_ATTR_FILTER          ( FM_ATTR_START + 1 )
#define FM_ATTR_FORM_OPERATIONS ( FM_ATTR_START + 2 )

inline constexpr OUString FM_PROP_NAME = u"Name"_ustr;
inline constexpr OUString FM_PROP_CLASSID = u"ClassId"_ustr;
inline constexpr OUString FM_PROP_ALIGN = u"Align"_ustr;
inline constexpr OUString FM_PROP_ROWCOUNT = u"RowCount"_ustr;
inline constexpr OUString FM_PROP_ROWCOUNTFINAL = u"IsRowCountFinal"_ustr;
inline constexpr OUString FM_PROP_FETCHSIZE = u"FetchSize"_ustr;
inline constexpr OUString FM_PROP_VALUE = u"Value"_ustr;
inline constexpr OUString FM_PROP_VALUEMIN = u"ValueMin"_ustr;
inline constexpr OUString FM_PROP_VALUEMAX = u"ValueMax"_ustr;
inline constexpr OUString FM_PROP_VALUESTEP = u"ValueStep"_ustr;
inline constexpr OUString FM_PROP_TEXT = u"Text"_ustr;
inline constexpr OUString FM_PROP_LABEL = u"Label"_ustr;
inline constexpr OUString FM_PROP_NAVIGATION = u"NavigationBarMode"_ustr;
inline constexpr OUString FM_PROP_CYCLE = u"Cycle"_ustr;
inline constexpr OUString FM_PROP_CONTROLSOURCE = u"DataField"_ustr;
inline constexpr OUString FM_PROP_ENABLED = u"Enabled"_ustr;
inline constexpr OUString FM_PROP_SPIN = u"Spin"_ustr;
inline constexpr OUString FM_PROP_READONLY = u"ReadOnly"_ustr;
inline constexpr OUString FM_PROP_FILTER = u"Filter"_ustr;
inline constexpr OUString FM_PROP_AUTOINCREMENT = u"IsAutoIncrement"_ustr;
inline constexpr OUString FM_PROP_WIDTH = u"Width"_ustr;
inline constexpr OUString FM_PROP_SEARCHABLE = u"IsSearchable"_ustr;
inline constexpr OUString FM_PROP_MULTILINE = u"MultiLine"_ustr;
inline constexpr OUString FM_PROP_TARGET_URL = u"TargetURL"_ustr;
inline constexpr OUString FM_PROP_DEFAULTCONTROL = u"DefaultControl"_ustr;
inline constexpr OUString FM_PROP_MAXTEXTLEN = u"MaxTextLen"_ustr;
inline constexpr OUString FM_PROP_DATE = u"Date"_ustr;
inline constexpr OUString FM_PROP_TIME = u"Time"_ustr;
inline constexpr OUString FM_PROP_STATE = u"State"_ustr;
inline constexpr OUString FM_PROP_TRISTATE = u"TriState"_ustr;
inline constexpr OUString FM_PROP_STRINGITEMLIST = u"StringItemList"_ustr;
inline constexpr OUString FM_PROP_DEFAULT_TEXT = u"DefaultText"_ustr;
inline constexpr OUString FM_PROP_DEFAULTCHECKED = u"DefaultState"_ustr;
inline constexpr OUString FM_PROP_DEFAULT_DATE = u"DefaultDate"_ustr;
inline constexpr OUString FM_PROP_DEFAULT_TIME = u"DefaultTime"_ustr;
inline constexpr OUString FM_PROP_DEFAULT_VALUE = u"DefaultValue"_ustr;
inline constexpr OUString FM_PROP_FORMATKEY = u"FormatKey"_ustr;
inline constexpr OUString FM_PROP_FORMATSSUPPLIER = u"FormatsSupplier"_ustr;
inline constexpr OUString FM_PROP_LISTSOURCETYPE = u"ListSourceType"_ustr;
inline constexpr OUString FM_PROP_LISTSOURCE = u"ListSource"_ustr;
inline constexpr OUString FM_PROP_SELECT_SEQ = u"SelectedItems"_ustr;
inline constexpr OUString FM_PROP_VALUE_SEQ = u"ValueItemList"_ustr;
inline constexpr OUString FM_PROP_DEFAULT_SELECT_SEQ = u"DefaultSelection"_ustr;
inline constexpr OUString FM_PROP_DECIMAL_ACCURACY = u"DecimalAccuracy"_ustr;
inline constexpr OUString FM_PROP_EDITMASK = u"EditMask"_ustr;
inline constexpr OUString FM_PROP_ISREADONLY = u"IsReadOnly"_ustr;
inline constexpr OUString FM_PROP_FIELDTYPE = u"Type"_ustr;
inline constexpr OUString FM_PROP_REFVALUE = u"RefValue"_ustr;
inline constexpr OUString FM_PROP_STRICTFORMAT = u"StrictFormat"_ustr;
inline constexpr OUString FM_PROP_DATASOURCE = u"DataSourceName"_ustr;
inline constexpr OUString FM_PROP_LITERALMASK = u"LiteralMask"_ustr;
inline constexpr OUString FM_PROP_SHOWTHOUSANDSEP = u"ShowThousandsSeparator"_ustr;
inline constexpr OUString FM_PROP_CURRENCYSYMBOL = u"CurrencySymbol"_ustr;
inline constexpr OUString FM_PROP_DATEFORMAT = u"DateFormat"_ustr;
inline constexpr OUString FM_PROP_DATEMIN = u"DateMin"_ustr;
inline constexpr OUString FM_PROP_DATEMAX = u"DateMax"_ustr;
inline constexpr OUString FM_PROP_DATE_SHOW_CENTURY = u"DateShowCentury"_ustr;
inline constexpr OUString FM_PROP_TIMEFORMAT = u"TimeFormat"_ustr;
inline constexpr OUString FM_PROP_TIMEMIN = u"TimeMin"_ustr;
inline constexpr OUString FM_PROP_TIMEMAX = u"TimeMax"_ustr;
inline constexpr OUString FM_PROP_LINECOUNT = u"LineCount"_ustr;
inline constexpr OUString FM_PROP_BOUNDCOLUMN = u"BoundColumn"_ustr;
#define FM_PROP_HASNAVIGATION                "HasNavigationBar"
inline constexpr OUString FM_PROP_FONT = u"FontDescriptor"_ustr;
#define FM_PROP_BACKGROUNDCOLOR              "BackgroundColor"
#define FM_PROP_TEXTCOLOR                    "TextColor"
inline constexpr OUString FM_PROP_BORDER = u"Border"_ustr;
inline constexpr OUString FM_PROP_DROPDOWN = u"Dropdown"_ustr;
inline constexpr OUString FM_PROP_ROWHEIGHT = u"RowHeight"_ustr;
inline constexpr OUString FM_PROP_HELPTEXT = u"HelpText"_ustr;
#define FM_PROP_HELPURL                      "HelpURL"
#define FM_PROP_RECORDMARKER                 "HasRecordMarker"
inline constexpr OUString FM_PROP_BOUNDFIELD = u"BoundField"_ustr;
inline constexpr OUString FM_PROP_EFFECTIVE_VALUE = u"EffectiveValue"_ustr;
inline constexpr OUString FM_PROP_EFFECTIVE_DEFAULT = u"EffectiveDefault"_ustr;
inline constexpr OUString FM_PROP_EFFECTIVE_MIN = u"EffectiveMin"_ustr;
inline constexpr OUString FM_PROP_EFFECTIVE_MAX = u"EffectiveMax"_ustr;
inline constexpr OUString FM_PROP_HIDDEN = u"Hidden"_ustr;
inline constexpr OUString FM_PROP_FILTERPROPOSAL = u"UseFilterValueProposal"_ustr;
inline constexpr OUString FM_PROP_FIELDSOURCE = u"FieldSource"_ustr;
inline constexpr OUString FM_PROP_TABLENAME = u"TableName"_ustr;
inline constexpr OUString FM_PROP_CONTROLLABEL = u"LabelControl"_ustr;
inline constexpr OUString FM_PROP_CURSORCOLOR = u"CursorColor"_ustr;
inline constexpr OUString FM_PROP_ALWAYSSHOWCURSOR = u"AlwaysShowCursor"_ustr;
inline constexpr OUString FM_PROP_DISPLAYSYNCHRON = u"DisplayIsSynchron"_ustr;
inline constexpr OUString FM_PROP_ISMODIFIED = u"IsModified"_ustr;
inline constexpr OUString FM_PROP_ISNEW = u"IsNew"_ustr;
inline constexpr OUString FM_PROP_PRIVILEGES = u"Privileges"_ustr;
inline constexpr OUString FM_PROP_COMMAND = u"Command"_ustr;
inline constexpr OUString FM_PROP_COMMANDTYPE = u"CommandType"_ustr;
inline constexpr OUString FM_PROP_RESULTSET_CONCURRENCY = u"ResultSetConcurrency"_ustr;
inline constexpr OUString FM_PROP_INSERTONLY = u"IgnoreResult"_ustr;
inline constexpr OUString FM_PROP_RESULTSET_TYPE = u"ResultSetType"_ustr;
inline constexpr OUString FM_PROP_ESCAPE_PROCESSING = u"EscapeProcessing"_ustr;
inline constexpr OUString FM_PROP_APPLYFILTER = u"ApplyFilter"_ustr;
inline constexpr OUString FM_PROP_ISNULLABLE = u"IsNullable"_ustr;
inline constexpr OUString FM_PROP_ACTIVECOMMAND = u"ActiveCommand"_ustr;
inline constexpr OUString FM_PROP_ISCURRENCY = u"IsCurrency"_ustr;
inline constexpr OUString FM_PROP_URL = u"URL"_ustr;
inline constexpr OUString FM_PROP_ACTIVE_CONNECTION = u"ActiveConnection"_ustr;
inline constexpr OUString FM_PROP_CONTROLSOURCEPROPERTY = u"DataFieldProperty"_ustr;
inline constexpr OUString FM_PROP_REALNAME = u"RealName"_ustr;
inline constexpr OUString FM_PROP_TEXTLINECOLOR = u"TextLineColor"_ustr;
#define FM_PROP_FONTEMPHASISMARK             "FontEmphasisMark"
#define FM_PROP_FONTRELIEF                   "FontRelief"
inline constexpr OUString FM_PROP_ORIENTATION = u"Orientation"_ustr;
inline constexpr OUString FM_PROP_LINEENDFORMAT = u"LineEndFormat"_ustr;
inline constexpr OUString FM_PROP_VISUALEFFECT = u"VisualEffect"_ustr;
inline constexpr OUString FM_PROP_BORDERCOLOR = u"BorderColor"_ustr;
inline constexpr OUString FM_PROP_DYNAMIC_CONTROL_BORDER = u"DynamicControlBorder"_ustr;
inline constexpr OUString FM_PROP_CONTROL_BORDER_COLOR_FOCUS = u"ControlBorderColorOnFocus"_ustr;
inline constexpr OUString FM_PROP_CONTROL_BORDER_COLOR_MOUSE = u"ControlBorderColorOnHover"_ustr;
inline constexpr OUString FM_PROP_CONTROL_BORDER_COLOR_INVALID = u"ControlBorderColorOnInvalid"_ustr;
inline constexpr OUString FM_PROP_BUTTON_TYPE = u"ButtonType"_ustr;
inline constexpr OUString FM_PROP_FORM_OPERATIONS = u"FormOperations"_ustr;
inline constexpr OUString FM_PROP_INPUT_REQUIRED = u"InputRequired"_ustr;
inline constexpr OUString FM_PROP_WRITING_MODE = u"WritingMode"_ustr;
inline constexpr OUString FM_PROP_MOUSE_WHEEL_BEHAVIOR = u"MouseWheelBehavior"_ustr;
inline constexpr OUString FM_PROP_DESCRIPTION = u"Description"_ustr;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
