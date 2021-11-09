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

inline constexpr OUStringLiteral FM_PROP_NAME = u"Name";
inline constexpr OUStringLiteral FM_PROP_CLASSID = u"ClassId";
inline constexpr OUStringLiteral FM_PROP_ALIGN = u"Align";
inline constexpr OUStringLiteral FM_PROP_ROWCOUNT = u"RowCount";
inline constexpr OUStringLiteral FM_PROP_ROWCOUNTFINAL = u"IsRowCountFinal";
inline constexpr OUStringLiteral FM_PROP_FETCHSIZE = u"FetchSize";
inline constexpr OUStringLiteral FM_PROP_VALUE = u"Value";
inline constexpr OUStringLiteral FM_PROP_VALUEMIN = u"ValueMin";
inline constexpr OUStringLiteral FM_PROP_VALUEMAX = u"ValueMax";
inline constexpr OUStringLiteral FM_PROP_VALUESTEP = u"ValueStep";
inline constexpr OUStringLiteral FM_PROP_TEXT = u"Text";
inline constexpr OUStringLiteral FM_PROP_LABEL = u"Label";
inline constexpr OUStringLiteral FM_PROP_NAVIGATION = u"NavigationBarMode";
inline constexpr OUStringLiteral FM_PROP_CYCLE = u"Cycle";
inline constexpr OUStringLiteral FM_PROP_CONTROLSOURCE = u"DataField";
inline constexpr OUStringLiteral FM_PROP_ENABLED = u"Enabled";
inline constexpr OUStringLiteral FM_PROP_SPIN = u"Spin";
inline constexpr OUStringLiteral FM_PROP_READONLY = u"ReadOnly";
inline constexpr OUStringLiteral FM_PROP_FILTER = u"Filter";
inline constexpr OUStringLiteral FM_PROP_AUTOINCREMENT = u"IsAutoIncrement";
inline constexpr OUStringLiteral FM_PROP_WIDTH = u"Width";
inline constexpr OUStringLiteral FM_PROP_SEARCHABLE = u"IsSearchable";
inline constexpr OUStringLiteral FM_PROP_MULTILINE = u"MultiLine";
inline constexpr OUStringLiteral FM_PROP_TARGET_URL = u"TargetURL";
inline constexpr OUStringLiteral FM_PROP_DEFAULTCONTROL = u"DefaultControl";
inline constexpr OUStringLiteral FM_PROP_MAXTEXTLEN = u"MaxTextLen";
inline constexpr OUStringLiteral FM_PROP_DATE = u"Date";
inline constexpr OUStringLiteral FM_PROP_TIME = u"Time";
inline constexpr OUStringLiteral FM_PROP_STATE = u"State";
inline constexpr OUStringLiteral FM_PROP_TRISTATE = u"TriState";
inline constexpr OUStringLiteral FM_PROP_STRINGITEMLIST = u"StringItemList";
inline constexpr OUStringLiteral FM_PROP_DEFAULT_TEXT = u"DefaultText";
inline constexpr OUStringLiteral FM_PROP_DEFAULTCHECKED = u"DefaultState";
inline constexpr OUStringLiteral FM_PROP_DEFAULT_DATE = u"DefaultDate";
inline constexpr OUStringLiteral FM_PROP_DEFAULT_TIME = u"DefaultTime";
inline constexpr OUStringLiteral FM_PROP_DEFAULT_VALUE = u"DefaultValue";
inline constexpr OUStringLiteral FM_PROP_FORMATKEY = u"FormatKey";
inline constexpr OUStringLiteral FM_PROP_FORMATSSUPPLIER = u"FormatsSupplier";
inline constexpr OUStringLiteral FM_PROP_LISTSOURCETYPE = u"ListSourceType";
inline constexpr OUStringLiteral FM_PROP_LISTSOURCE = u"ListSource";
inline constexpr OUStringLiteral FM_PROP_SELECT_SEQ = u"SelectedItems";
inline constexpr OUStringLiteral FM_PROP_VALUE_SEQ = u"ValueItemList";
inline constexpr OUStringLiteral FM_PROP_DEFAULT_SELECT_SEQ = u"DefaultSelection";
inline constexpr OUStringLiteral FM_PROP_DECIMAL_ACCURACY = u"DecimalAccuracy";
inline constexpr OUStringLiteral FM_PROP_EDITMASK = u"EditMask";
inline constexpr OUStringLiteral FM_PROP_ISREADONLY = u"IsReadOnly";
inline constexpr OUStringLiteral FM_PROP_FIELDTYPE = u"Type";
inline constexpr OUStringLiteral FM_PROP_REFVALUE = u"RefValue";
inline constexpr OUStringLiteral FM_PROP_STRICTFORMAT = u"StrictFormat";
inline constexpr OUStringLiteral FM_PROP_DATASOURCE = u"DataSourceName";
inline constexpr OUStringLiteral FM_PROP_LITERALMASK = u"LiteralMask";
inline constexpr OUStringLiteral FM_PROP_SHOWTHOUSANDSEP = u"ShowThousandsSeparator";
inline constexpr OUStringLiteral FM_PROP_CURRENCYSYMBOL = u"CurrencySymbol";
inline constexpr OUStringLiteral FM_PROP_DATEFORMAT = u"DateFormat";
inline constexpr OUStringLiteral FM_PROP_DATEMIN = u"DateMin";
inline constexpr OUStringLiteral FM_PROP_DATEMAX = u"DateMax";
inline constexpr OUStringLiteral FM_PROP_DATE_SHOW_CENTURY = u"DateShowCentury";
inline constexpr OUStringLiteral FM_PROP_TIMEFORMAT = u"TimeFormat";
inline constexpr OUStringLiteral FM_PROP_TIMEMIN = u"TimeMin";
inline constexpr OUStringLiteral FM_PROP_TIMEMAX = u"TimeMax";
inline constexpr OUStringLiteral FM_PROP_LINECOUNT = u"LineCount";
inline constexpr OUStringLiteral FM_PROP_BOUNDCOLUMN = u"BoundColumn";
#define FM_PROP_HASNAVIGATION                "HasNavigationBar"
inline constexpr OUStringLiteral FM_PROP_FONT = u"FontDescriptor";
#define FM_PROP_BACKGROUNDCOLOR              "BackgroundColor"
#define FM_PROP_TEXTCOLOR                    "TextColor"
inline constexpr OUStringLiteral FM_PROP_BORDER = u"Border";
inline constexpr OUStringLiteral FM_PROP_DROPDOWN = u"Dropdown";
inline constexpr OUStringLiteral FM_PROP_ROWHEIGHT = u"RowHeight";
inline constexpr OUStringLiteral FM_PROP_HELPTEXT = u"HelpText";
#define FM_PROP_HELPURL                      "HelpURL"
#define FM_PROP_RECORDMARKER                 "HasRecordMarker"
inline constexpr OUStringLiteral FM_PROP_BOUNDFIELD = u"BoundField";
inline constexpr OUStringLiteral FM_PROP_EFFECTIVE_VALUE = u"EffectiveValue";
inline constexpr OUStringLiteral FM_PROP_EFFECTIVE_DEFAULT = u"EffectiveDefault";
inline constexpr OUStringLiteral FM_PROP_EFFECTIVE_MIN = u"EffectiveMin";
inline constexpr OUStringLiteral FM_PROP_EFFECTIVE_MAX = u"EffectiveMax";
inline constexpr OUStringLiteral FM_PROP_HIDDEN = u"Hidden";
inline constexpr OUStringLiteral FM_PROP_FILTERPROPOSAL = u"UseFilterValueProposal";
inline constexpr OUStringLiteral FM_PROP_FIELDSOURCE = u"FieldSource";
inline constexpr OUStringLiteral FM_PROP_TABLENAME = u"TableName";
inline constexpr OUStringLiteral FM_PROP_CONTROLLABEL = u"LabelControl";
inline constexpr OUStringLiteral FM_PROP_CURSORCOLOR = u"CursorColor";
inline constexpr OUStringLiteral FM_PROP_ALWAYSSHOWCURSOR = u"AlwaysShowCursor";
inline constexpr OUStringLiteral FM_PROP_DISPLAYSYNCHRON = u"DisplayIsSynchron";
inline constexpr OUStringLiteral FM_PROP_ISMODIFIED = u"IsModified";
inline constexpr OUStringLiteral FM_PROP_ISNEW = u"IsNew";
inline constexpr OUStringLiteral FM_PROP_PRIVILEGES = u"Privileges";
inline constexpr OUStringLiteral FM_PROP_COMMAND = u"Command";
inline constexpr OUStringLiteral FM_PROP_COMMANDTYPE = u"CommandType";
inline constexpr OUStringLiteral FM_PROP_RESULTSET_CONCURRENCY = u"ResultSetConcurrency";
inline constexpr OUStringLiteral FM_PROP_INSERTONLY = u"IgnoreResult";
inline constexpr OUStringLiteral FM_PROP_RESULTSET_TYPE = u"ResultSetType";
inline constexpr OUStringLiteral FM_PROP_ESCAPE_PROCESSING = u"EscapeProcessing";
inline constexpr OUStringLiteral FM_PROP_APPLYFILTER = u"ApplyFilter";
inline constexpr OUStringLiteral FM_PROP_ISNULLABLE = u"IsNullable";
inline constexpr OUStringLiteral FM_PROP_ACTIVECOMMAND = u"ActiveCommand";
inline constexpr OUStringLiteral FM_PROP_ISCURRENCY = u"IsCurrency";
inline constexpr OUStringLiteral FM_PROP_URL = u"URL";
inline constexpr OUStringLiteral FM_PROP_ACTIVE_CONNECTION = u"ActiveConnection";
inline constexpr OUStringLiteral FM_PROP_CONTROLSOURCEPROPERTY = u"DataFieldProperty";
inline constexpr OUStringLiteral FM_PROP_REALNAME = u"RealName";
inline constexpr OUStringLiteral FM_PROP_TEXTLINECOLOR = u"TextLineColor";
#define FM_PROP_FONTEMPHASISMARK             "FontEmphasisMark"
#define FM_PROP_FONTRELIEF                   "FontRelief"
inline constexpr OUStringLiteral FM_PROP_ORIENTATION = u"Orientation";
inline constexpr OUStringLiteral FM_PROP_LINEENDFORMAT = u"LineEndFormat";
inline constexpr OUStringLiteral FM_PROP_VISUALEFFECT = u"VisualEffect";
inline constexpr OUStringLiteral FM_PROP_BORDERCOLOR = u"BorderColor";
inline constexpr OUStringLiteral FM_PROP_DYNAMIC_CONTROL_BORDER = u"DynamicControlBorder";
inline constexpr OUStringLiteral FM_PROP_CONTROL_BORDER_COLOR_FOCUS = u"ControlBorderColorOnFocus";
inline constexpr OUStringLiteral FM_PROP_CONTROL_BORDER_COLOR_MOUSE = u"ControlBorderColorOnHover";
inline constexpr OUStringLiteral FM_PROP_CONTROL_BORDER_COLOR_INVALID = u"ControlBorderColorOnInvalid";
inline constexpr OUStringLiteral FM_PROP_BUTTON_TYPE = u"ButtonType";
inline constexpr OUStringLiteral FM_PROP_FORM_OPERATIONS = u"FormOperations";
inline constexpr OUStringLiteral FM_PROP_INPUT_REQUIRED = u"InputRequired";
inline constexpr OUStringLiteral FM_PROP_WRITING_MODE = u"WritingMode";
inline constexpr OUStringLiteral FM_PROP_MOUSE_WHEEL_BEHAVIOR = u"MouseWheelBehavior";
inline constexpr OUStringLiteral FM_PROP_DESCRIPTION = u"Description";

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
