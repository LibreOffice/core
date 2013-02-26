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

#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#define _XMLOFF_FORMS_STRINGS_HXX_

#include <comphelper/string.hxx>

//.........................................................................
namespace xmloff
{
    using comphelper::string::ConstAsciiString;
//.........................................................................

#ifndef XMLFORM_IMPLEMENT_STRINGS
    #define XMLFORM_CONSTASCII_STRING(ident, string) extern const ConstAsciiString ident
#else
#define XMLFORM_CONSTASCII_STRING(ident, string) extern const ConstAsciiString ident = {RTL_CONSTASCII_STRINGPARAM(string)}
#endif

    //============================================================
    //= string constants
    //============================================================

    // properties
    XMLFORM_CONSTASCII_STRING( PROPERTY_CLASSID,            "ClassId" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ECHOCHAR,           "EchoChar" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_MULTILINE,          "MultiLine" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_NAME,               "Name" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_IMAGEURL,           "ImageURL" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_LABEL,              "Label" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_TARGETFRAME,        "TargetFrame" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_TARGETURL,          "TargetURL" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_TITLE,              "Tag" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_DROPDOWN,           "Dropdown" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_PRINTABLE,          "Printable" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_READONLY,           "ReadOnly" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_STATE,      "DefaultState" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_TABSTOP,            "Tabstop" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_STATE,              "State" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ENABLED,            "Enabled" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ENABLEVISIBLE,      "EnableVisible" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_MAXTEXTLENGTH,      "MaxTextLen" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_LINECOUNT,          "LineCount" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_TABINDEX,           "TabIndex" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_COMMAND,            "Command" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_DATASOURCENAME,     "DataSourceName" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_FILTER,             "Filter" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ORDER,              "Order" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ALLOWDELETES,       "AllowDeletes" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ALLOWINSERTS,       "AllowInserts" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ALLOWUPDATES,       "AllowUpdates" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_APPLYFILTER,        "ApplyFilter" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ESCAPEPROCESSING,   "EscapeProcessing" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_IGNORERESULT,       "IgnoreResult" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_SUBMIT_ENCODING,    "SubmitEncoding" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_SUBMIT_METHOD,      "SubmitMethod" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_COMMAND_TYPE,       "CommandType" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_NAVIGATION,         "NavigationBarMode" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_CYCLE,              "Cycle" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_BUTTONTYPE,         "ButtonType" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_DATAFIELD,          "DataField" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_BOUNDCOLUMN,        "BoundColumn");
    XMLFORM_CONSTASCII_STRING( PROPERTY_EMPTY_IS_NULL,      "ConvertEmptyToNull");
    XMLFORM_CONSTASCII_STRING( PROPERTY_INPUT_REQUIRED,     "InputRequired");
    XMLFORM_CONSTASCII_STRING( PROPERTY_LISTSOURCE,         "ListSource");
    XMLFORM_CONSTASCII_STRING( PROPERTY_LISTSOURCETYPE,     "ListSourceType");
    XMLFORM_CONSTASCII_STRING( PROPERTY_ECHO_CHAR,          "EchoChar");
    XMLFORM_CONSTASCII_STRING( PROPERTY_STRICTFORMAT,       "StrictFormat");
    XMLFORM_CONSTASCII_STRING( PROPERTY_AUTOCOMPLETE,       "Autocomplete");
    XMLFORM_CONSTASCII_STRING( PROPERTY_MULTISELECTION,     "MultiSelection");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULTBUTTON,      "DefaultButton");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TRISTATE,           "TriState");
    XMLFORM_CONSTASCII_STRING( PROPERTY_CONTROLLABEL,       "LabelControl");
    XMLFORM_CONSTASCII_STRING( PROPERTY_STRING_ITEM_LIST,   "StringItemList");
    XMLFORM_CONSTASCII_STRING( PROPERTY_VALUE_SEQ,          "ValueItemList");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_SELECT_SEQ, "DefaultSelection");
    XMLFORM_CONSTASCII_STRING( PROPERTY_SELECT_SEQ,         "SelectedItems");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DATE_MIN,           "DateMin");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DATE_MAX,           "DateMax");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TIME_MIN,           "TimeMin");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TIME_MAX,           "TimeMax");
    XMLFORM_CONSTASCII_STRING( PROPERTY_VALUE_MIN,          "ValueMin");
    XMLFORM_CONSTASCII_STRING( PROPERTY_VALUE_MAX,          "ValueMax");
    XMLFORM_CONSTASCII_STRING( PROPERTY_EFFECTIVE_MIN,      "EffectiveMin");
    XMLFORM_CONSTASCII_STRING( PROPERTY_EFFECTIVE_MAX,      "EffectiveMax");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_DATE,       "DefaultDate");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DATE,               "Date");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_TIME,       "DefaultTime");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TIME,               "Time");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_VALUE,      "DefaultValue");
    XMLFORM_CONSTASCII_STRING( PROPERTY_VALUE,              "Value");
    XMLFORM_CONSTASCII_STRING( PROPERTY_HIDDEN_VALUE,       "HiddenValue");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_TEXT,       "DefaultText");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TEXT,               "Text");
    XMLFORM_CONSTASCII_STRING( PROPERTY_EFFECTIVE_VALUE,    "EffectiveValue");
    XMLFORM_CONSTASCII_STRING( PROPERTY_EFFECTIVE_DEFAULT,  "EffectiveDefault");
    XMLFORM_CONSTASCII_STRING( PROPERTY_REFVALUE,           "RefValue");
    XMLFORM_CONSTASCII_STRING( PROPERTY_URL,                "URL");
    XMLFORM_CONSTASCII_STRING( PROPERTY_FONT,               "FontDescriptor");
    XMLFORM_CONSTASCII_STRING( PROPERTY_BACKGROUNDCOLOR,    "BackgroundColor");
    XMLFORM_CONSTASCII_STRING( PROPERTY_MASTERFIELDS,       "MasterFields");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DETAILFIELDS,       "DetailFields");
    XMLFORM_CONSTASCII_STRING( PROPERTY_COLUMNSERVICENAME,  "ColumnServiceName");
    XMLFORM_CONSTASCII_STRING( PROPERTY_FORMATKEY,          "FormatKey");
    XMLFORM_CONSTASCII_STRING( PROPERTY_ALIGN,              "Align");
    XMLFORM_CONSTASCII_STRING( PROPERTY_BORDER,             "Border");
    XMLFORM_CONSTASCII_STRING( PROPERTY_AUTOCONTROLFOCUS,   "AutomaticControlFocus");
    XMLFORM_CONSTASCII_STRING( PROPERTY_APPLYDESIGNMODE,    "ApplyFormDesignMode");
    XMLFORM_CONSTASCII_STRING( PROPERTY_FORMATSSUPPLIER,    "FormatsSupplier");
    XMLFORM_CONSTASCII_STRING( PROPERTY_LOCALE,             "Locale");
    XMLFORM_CONSTASCII_STRING( PROPERTY_FORMATSTRING,       "FormatString");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DATEFORMAT,         "DateFormat");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TIMEFORMAT,         "TimeFormat");
    XMLFORM_CONSTASCII_STRING( PROPERTY_PERSISTENCE_MAXTEXTLENGTH,  "PersistenceMaxTextLength");
    XMLFORM_CONSTASCII_STRING( PROPERTY_SCROLLVALUE_MIN,    "ScrollValueMin");
    XMLFORM_CONSTASCII_STRING( PROPERTY_SCROLLVALUE_MAX,    "ScrollValueMax");
    XMLFORM_CONSTASCII_STRING( PROPERTY_SCROLLVALUE,        "ScrollValue");
    XMLFORM_CONSTASCII_STRING( PROPERTY_SCROLLVALUE_DEFAULT,"DefaultScrollValue");
    XMLFORM_CONSTASCII_STRING( PROPERTY_LINE_INCREMENT,     "LineIncrement");
    XMLFORM_CONSTASCII_STRING( PROPERTY_BLOCK_INCREMENT,    "BlockIncrement");
    XMLFORM_CONSTASCII_STRING( PROPERTY_REPEAT_DELAY,       "RepeatDelay");
    XMLFORM_CONSTASCII_STRING( PROPERTY_SPINVALUE,          "SpinValue");
    XMLFORM_CONSTASCII_STRING( PROPERTY_SPINVALUE_MIN,      "SpinValueMin");
    XMLFORM_CONSTASCII_STRING( PROPERTY_SPINVALUE_MAX,      "SpinValueMax");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_SPINVALUE,  "DefaultSpinValue");
    XMLFORM_CONSTASCII_STRING( PROPERTY_SPIN_INCREMENT,     "SpinIncrement");
    XMLFORM_CONSTASCII_STRING( PROPERTY_ORIENTATION,        "Orientation");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TOGGLE,             "Toggle");
    XMLFORM_CONSTASCII_STRING( PROPERTY_FOCUS_ON_CLICK,     "FocusOnClick");
    XMLFORM_CONSTASCII_STRING( PROPERTY_VISUAL_EFFECT,      "VisualEffect");
    XMLFORM_CONSTASCII_STRING( PROPERTY_IMAGE_POSITION,     "ImagePosition");
    XMLFORM_CONSTASCII_STRING( PROPERTY_IMAGE_ALIGN,        "ImageAlign");
    XMLFORM_CONSTASCII_STRING( PROPERTY_SCALE_IMAGE,        "ScaleImage");
    XMLFORM_CONSTASCII_STRING( PROPERTY_GROUP_NAME,         "GroupName");

    XMLFORM_CONSTASCII_STRING( PROPERTY_BOUND_CELL,         "BoundCell");
    XMLFORM_CONSTASCII_STRING( PROPERTY_LIST_CELL_RANGE,    "CellRange");
    XMLFORM_CONSTASCII_STRING( PROPERTY_ADDRESS,            "Address");
    XMLFORM_CONSTASCII_STRING( PROPERTY_FILE_REPRESENTATION,"PersistentRepresentation");
    XMLFORM_CONSTASCII_STRING( PROPERTY_RICH_TEXT,          "RichText");

    // services
    XMLFORM_CONSTASCII_STRING( SERVICE_FORMSCOLLECTION,         "com.sun.star.form.Forms" );
    XMLFORM_CONSTASCII_STRING( SERVICE_SPREADSHEET_DOCUMENT,    "com.sun.star.sheet.SpreadsheetDocument");
    XMLFORM_CONSTASCII_STRING( SERVICE_CELLVALUEBINDING,        "com.sun.star.table.CellValueBinding" );
    XMLFORM_CONSTASCII_STRING( SERVICE_LISTINDEXCELLBINDING,    "com.sun.star.table.ListPositionCellBinding" );
    XMLFORM_CONSTASCII_STRING( SERVICE_CELLRANGELISTSOURCE,     "com.sun.star.table.CellRangeListSource" );
    XMLFORM_CONSTASCII_STRING( SERVICE_ADDRESS_CONVERSION,      "com.sun.star.table.CellAddressConversion");
    XMLFORM_CONSTASCII_STRING( SERVICE_RANGEADDRESS_CONVERSION, "com.sun.star.table.CellRangeAddressConversion");

    // old service names (compatibility)
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_FORM,           "stardiv.one.form.component.Form");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_EDIT,           "stardiv.one.form.component.Edit");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_LISTBOX,        "stardiv.one.form.component.ListBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_COMBOBOX,       "stardiv.one.form.component.ComboBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_RADIOBUTTON,    "stardiv.one.form.component.RadioButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_GROUPBOX,       "stardiv.one.form.component.GroupBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_FIXEDTEXT,      "stardiv.one.form.component.FixedText");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_COMMANDBUTTON,  "stardiv.one.form.component.CommandButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_CHECKBOX,       "stardiv.one.form.component.CheckBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_GRID,           "stardiv.one.form.component.Grid");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_IMAGEBUTTON,    "stardiv.one.form.component.ImageButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_FILECONTROL,    "stardiv.one.form.component.FileControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_TIMEFIELD,      "stardiv.one.form.component.TimeField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_DATEFIELD,      "stardiv.one.form.component.DateField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_NUMERICFIELD,   "stardiv.one.form.component.NumericField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_CURRENCYFIELD,  "stardiv.one.form.component.CurrencyField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_PATTERNFIELD,   "stardiv.one.form.component.PatternField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_HIDDENCONTROL,  "stardiv.one.form.component.Hidden");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_IMAGECONTROL,   "stardiv.one.form.component.ImageControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_FORMATTEDFIELD, "stardiv.one.form.component.FormattedField");

    // new service names, the old ones are translated into this new ones
    XMLFORM_CONSTASCII_STRING( SERVICE_FORM,            "com.sun.star.form.component.Form");
    XMLFORM_CONSTASCII_STRING( SERVICE_EDIT,            "com.sun.star.form.component.TextField");
    XMLFORM_CONSTASCII_STRING( SERVICE_LISTBOX,         "com.sun.star.form.component.ListBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_COMBOBOX,        "com.sun.star.form.component.ComboBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_RADIOBUTTON,     "com.sun.star.form.component.RadioButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_GROUPBOX,        "com.sun.star.form.component.GroupBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_FIXEDTEXT,       "com.sun.star.form.component.FixedText");
    XMLFORM_CONSTASCII_STRING( SERVICE_COMMANDBUTTON,   "com.sun.star.form.component.CommandButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_CHECKBOX,        "com.sun.star.form.component.CheckBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_GRID,            "com.sun.star.form.component.GridControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_IMAGEBUTTON,     "com.sun.star.form.component.ImageButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_FILECONTROL,     "com.sun.star.form.component.FileControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_TIMEFIELD,       "com.sun.star.form.component.TimeField");
    XMLFORM_CONSTASCII_STRING( SERVICE_DATEFIELD,       "com.sun.star.form.component.DateField");
    XMLFORM_CONSTASCII_STRING( SERVICE_NUMERICFIELD,    "com.sun.star.form.component.NumericField");
    XMLFORM_CONSTASCII_STRING( SERVICE_CURRENCYFIELD,   "com.sun.star.form.component.CurrencyField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PATTERNFIELD,    "com.sun.star.form.component.PatternField");
    XMLFORM_CONSTASCII_STRING( SERVICE_HIDDENCONTROL,   "com.sun.star.form.component.HiddenControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_IMAGECONTROL,    "com.sun.star.form.component.DatabaseImageControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_FORMATTEDFIELD,  "com.sun.star.form.component.FormattedField" );

    // various strings
    XMLFORM_CONSTASCII_STRING( EVENT_NAME_SEPARATOR, "::" );
    XMLFORM_CONSTASCII_STRING( EVENT_TYPE, "EventType" );
    XMLFORM_CONSTASCII_STRING( EVENT_LIBRARY, "Library" );
    XMLFORM_CONSTASCII_STRING( EVENT_LOCALMACRONAME, "MacroName" );
    XMLFORM_CONSTASCII_STRING( EVENT_SCRIPTURL, "Script" );
    XMLFORM_CONSTASCII_STRING( EVENT_STAROFFICE, "StarOffice" );
    XMLFORM_CONSTASCII_STRING( EVENT_STARBASIC, "StarBasic" );
    XMLFORM_CONSTASCII_STRING( EVENT_APPLICATION, "application" );

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_STRINGS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
