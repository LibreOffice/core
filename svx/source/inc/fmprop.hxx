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

constexpr OUStringLiteral FM_PROP_NAME = u"Name";
constexpr OUStringLiteral FM_PROP_CLASSID = u"ClassId";
constexpr OUStringLiteral FM_PROP_ALIGN = u"Align";
constexpr OUStringLiteral FM_PROP_ROWCOUNT = u"RowCount";
constexpr OUStringLiteral FM_PROP_ROWCOUNTFINAL = u"IsRowCountFinal";
constexpr OUStringLiteral FM_PROP_FETCHSIZE = u"FetchSize";
constexpr OUStringLiteral FM_PROP_VALUE = u"Value";
constexpr OUStringLiteral FM_PROP_VALUEMIN = u"ValueMin";
constexpr OUStringLiteral FM_PROP_VALUEMAX = u"ValueMax";
constexpr OUStringLiteral FM_PROP_VALUESTEP = u"ValueStep";
constexpr OUStringLiteral FM_PROP_TEXT = u"Text";
constexpr OUStringLiteral FM_PROP_LABEL = u"Label";
constexpr OUStringLiteral FM_PROP_NAVIGATION = u"NavigationBarMode";
constexpr OUStringLiteral FM_PROP_CYCLE = u"Cycle";
constexpr OUStringLiteral FM_PROP_CONTROLSOURCE = u"DataField";
constexpr OUStringLiteral FM_PROP_ENABLED = u"Enabled";
constexpr OUStringLiteral FM_PROP_SPIN = u"Spin";
constexpr OUStringLiteral FM_PROP_READONLY = u"ReadOnly";
constexpr OUStringLiteral FM_PROP_FILTER = u"Filter";
constexpr OUStringLiteral FM_PROP_AUTOINCREMENT = u"IsAutoIncrement";
constexpr OUStringLiteral FM_PROP_WIDTH = u"Width";
constexpr OUStringLiteral FM_PROP_SEARCHABLE = u"IsSearchable";
constexpr OUStringLiteral FM_PROP_MULTILINE = u"MultiLine";
constexpr OUStringLiteral FM_PROP_TARGET_URL = u"TargetURL";
constexpr OUStringLiteral FM_PROP_DEFAULTCONTROL = u"DefaultControl";
constexpr OUStringLiteral FM_PROP_MAXTEXTLEN = u"MaxTextLen";
constexpr OUStringLiteral FM_PROP_DATE = u"Date";
constexpr OUStringLiteral FM_PROP_TIME = u"Time";
constexpr OUStringLiteral FM_PROP_STATE = u"State";
constexpr OUStringLiteral FM_PROP_TRISTATE = u"TriState";
constexpr OUStringLiteral FM_PROP_STRINGITEMLIST = u"StringItemList";
constexpr OUStringLiteral FM_PROP_DEFAULT_TEXT = u"DefaultText";
constexpr OUStringLiteral FM_PROP_DEFAULTCHECKED = u"DefaultState";
constexpr OUStringLiteral FM_PROP_DEFAULT_DATE = u"DefaultDate";
constexpr OUStringLiteral FM_PROP_DEFAULT_TIME = u"DefaultTime";
constexpr OUStringLiteral FM_PROP_DEFAULT_VALUE = u"DefaultValue";
constexpr OUStringLiteral FM_PROP_FORMATKEY = u"FormatKey";
constexpr OUStringLiteral FM_PROP_FORMATSSUPPLIER = u"FormatsSupplier";
#define FM_PROP_LISTSOURCETYPE               "ListSourceType"
#define FM_PROP_LISTSOURCE                   "ListSource"
constexpr OUStringLiteral FM_PROP_SELECT_SEQ = u"SelectedItems";
constexpr OUStringLiteral FM_PROP_VALUE_SEQ = u"ValueItemList";
constexpr OUStringLiteral FM_PROP_DEFAULT_SELECT_SEQ = u"DefaultSelection";
constexpr OUStringLiteral FM_PROP_DECIMAL_ACCURACY = u"DecimalAccuracy";
constexpr OUStringLiteral FM_PROP_EDITMASK = u"EditMask";
constexpr OUStringLiteral FM_PROP_ISREADONLY = u"IsReadOnly";
constexpr OUStringLiteral FM_PROP_FIELDTYPE = u"Type";
constexpr OUStringLiteral FM_PROP_REFVALUE = u"RefValue";
constexpr OUStringLiteral FM_PROP_STRICTFORMAT = u"StrictFormat";
constexpr OUStringLiteral FM_PROP_DATASOURCE = u"DataSourceName";
constexpr OUStringLiteral FM_PROP_LITERALMASK = u"LiteralMask";
constexpr OUStringLiteral FM_PROP_SHOWTHOUSANDSEP = u"ShowThousandsSeparator";
constexpr OUStringLiteral FM_PROP_CURRENCYSYMBOL = u"CurrencySymbol";
constexpr OUStringLiteral FM_PROP_DATEFORMAT = u"DateFormat";
constexpr OUStringLiteral FM_PROP_DATEMIN = u"DateMin";
constexpr OUStringLiteral FM_PROP_DATEMAX = u"DateMax";
constexpr OUStringLiteral FM_PROP_DATE_SHOW_CENTURY = u"DateShowCentury";
constexpr OUStringLiteral FM_PROP_TIMEFORMAT = u"TimeFormat";
constexpr OUStringLiteral FM_PROP_TIMEMIN = u"TimeMin";
constexpr OUStringLiteral FM_PROP_TIMEMAX = u"TimeMax";
constexpr OUStringLiteral FM_PROP_LINECOUNT = u"LineCount";
#define FM_PROP_BOUNDCOLUMN                  "BoundColumn"
#define FM_PROP_HASNAVIGATION                "HasNavigationBar"
constexpr OUStringLiteral FM_PROP_FONT = u"FontDescriptor";
#define FM_PROP_BACKGROUNDCOLOR              "BackgroundColor"
#define FM_PROP_TEXTCOLOR                    "TextColor"
constexpr OUStringLiteral FM_PROP_BORDER = u"Border";
constexpr OUStringLiteral FM_PROP_DROPDOWN = u"Dropdown";
constexpr OUStringLiteral FM_PROP_ROWHEIGHT = u"RowHeight";
constexpr OUStringLiteral FM_PROP_HELPTEXT = u"HelpText";
#define FM_PROP_HELPURL                      "HelpURL"
#define FM_PROP_RECORDMARKER                 "HasRecordMarker"
constexpr OUStringLiteral FM_PROP_BOUNDFIELD = u"BoundField";
constexpr OUStringLiteral FM_PROP_EFFECTIVE_VALUE = u"EffectiveValue";
constexpr OUStringLiteral FM_PROP_EFFECTIVE_DEFAULT = u"EffectiveDefault";
constexpr OUStringLiteral FM_PROP_EFFECTIVE_MIN = u"EffectiveMin";
constexpr OUStringLiteral FM_PROP_EFFECTIVE_MAX = u"EffectiveMax";
constexpr OUStringLiteral FM_PROP_HIDDEN = u"Hidden";
constexpr OUStringLiteral FM_PROP_FILTERPROPOSAL = u"UseFilterValueProposal";
constexpr OUStringLiteral FM_PROP_FIELDSOURCE = u"FieldSource";
constexpr OUStringLiteral FM_PROP_TABLENAME = u"TableName";
constexpr OUStringLiteral FM_PROP_CONTROLLABEL = u"LabelControl";
constexpr OUStringLiteral FM_PROP_CURSORCOLOR = u"CursorColor";
constexpr OUStringLiteral FM_PROP_ALWAYSSHOWCURSOR = u"AlwaysShowCursor";
constexpr OUStringLiteral FM_PROP_DISPLAYSYNCHRON = u"DisplayIsSynchron";
constexpr OUStringLiteral FM_PROP_ISMODIFIED = u"IsModified";
constexpr OUStringLiteral FM_PROP_ISNEW = u"IsNew";
constexpr OUStringLiteral FM_PROP_PRIVILEGES = u"Privileges";
constexpr OUStringLiteral FM_PROP_COMMAND = u"Command";
constexpr OUStringLiteral FM_PROP_COMMANDTYPE = u"CommandType";
constexpr OUStringLiteral FM_PROP_RESULTSET_CONCURRENCY = u"ResultSetConcurrency";
constexpr OUStringLiteral FM_PROP_INSERTONLY = u"IgnoreResult";
constexpr OUStringLiteral FM_PROP_RESULTSET_TYPE = u"ResultSetType";
constexpr OUStringLiteral FM_PROP_ESCAPE_PROCESSING = u"EscapeProcessing";
constexpr OUStringLiteral FM_PROP_APPLYFILTER = u"ApplyFilter";
constexpr OUStringLiteral FM_PROP_ISNULLABLE = u"IsNullable";
constexpr OUStringLiteral FM_PROP_ACTIVECOMMAND = u"ActiveCommand";
constexpr OUStringLiteral FM_PROP_ISCURRENCY = u"IsCurrency";
constexpr OUStringLiteral FM_PROP_URL = u"URL";
constexpr OUStringLiteral FM_PROP_ACTIVE_CONNECTION = u"ActiveConnection";
constexpr OUStringLiteral FM_PROP_CONTROLSOURCEPROPERTY = u"DataFieldProperty";
constexpr OUStringLiteral FM_PROP_REALNAME = u"RealName";
constexpr OUStringLiteral FM_PROP_TEXTLINECOLOR = u"TextLineColor";
#define FM_PROP_FONTEMPHASISMARK             "FontEmphasisMark"
#define FM_PROP_FONTRELIEF                   "FontRelief"
constexpr OUStringLiteral FM_PROP_ORIENTATION = u"Orientation";
constexpr OUStringLiteral FM_PROP_LINEENDFORMAT = u"LineEndFormat";
constexpr OUStringLiteral FM_PROP_VISUALEFFECT = u"VisualEffect";
constexpr OUStringLiteral FM_PROP_BORDERCOLOR = u"BorderColor";
constexpr OUStringLiteral FM_PROP_DYNAMIC_CONTROL_BORDER = u"DynamicControlBorder";
constexpr OUStringLiteral FM_PROP_CONTROL_BORDER_COLOR_FOCUS = u"ControlBorderColorOnFocus";
constexpr OUStringLiteral FM_PROP_CONTROL_BORDER_COLOR_MOUSE = u"ControlBorderColorOnHover";
constexpr OUStringLiteral FM_PROP_CONTROL_BORDER_COLOR_INVALID = u"ControlBorderColorOnInvalid";
constexpr OUStringLiteral FM_PROP_BUTTON_TYPE = u"ButtonType";
constexpr OUStringLiteral FM_PROP_FORM_OPERATIONS = u"FormOperations";
constexpr OUStringLiteral FM_PROP_INPUT_REQUIRED = u"InputRequired";
constexpr OUStringLiteral FM_PROP_WRITING_MODE = u"WritingMode";
constexpr OUStringLiteral FM_PROP_MOUSE_WHEEL_BEHAVIOR = u"MouseWheelBehavior";
constexpr OUStringLiteral FM_PROP_DESCRIPTION = u"Description";

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
