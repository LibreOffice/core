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

namespace xmloff
{

    // properties
    inline constexpr OUString PROPERTY_CLASSID = u"ClassId"_ustr;
    inline constexpr OUString PROPERTY_ECHOCHAR = u"EchoChar"_ustr;
    inline constexpr OUString PROPERTY_MULTILINE = u"MultiLine"_ustr;
    inline constexpr OUString PROPERTY_NAME = u"Name"_ustr;
    inline constexpr OUString PROPERTY_GRAPHIC = u"Graphic"_ustr;
    inline constexpr OUString PROPERTY_LABEL = u"Label"_ustr;
    inline constexpr OUString PROPERTY_TARGETFRAME = u"TargetFrame"_ustr;
    inline constexpr OUString PROPERTY_TARGETURL = u"TargetURL"_ustr;
    inline constexpr OUString PROPERTY_TITLE = u"Tag"_ustr;
    inline constexpr OUString PROPERTY_DROPDOWN = u"Dropdown"_ustr;
    inline constexpr OUString PROPERTY_PRINTABLE = u"Printable"_ustr;
    inline constexpr OUString PROPERTY_READONLY = u"ReadOnly"_ustr;
    inline constexpr OUString PROPERTY_DEFAULT_STATE = u"DefaultState"_ustr;
    inline constexpr OUString PROPERTY_TABSTOP = u"Tabstop"_ustr;
    inline constexpr OUString PROPERTY_STATE = u"State"_ustr;
    inline constexpr OUString PROPERTY_ENABLED = u"Enabled"_ustr;
    inline constexpr OUString PROPERTY_ENABLEVISIBLE = u"EnableVisible"_ustr;
    inline constexpr OUString PROPERTY_MAXTEXTLENGTH = u"MaxTextLen"_ustr;
    inline constexpr OUString PROPERTY_LINECOUNT = u"LineCount"_ustr;
    inline constexpr OUString PROPERTY_TABINDEX = u"TabIndex"_ustr;
    inline constexpr OUString PROPERTY_COMMAND = u"Command"_ustr;
    inline constexpr OUString PROPERTY_DATASOURCENAME = u"DataSourceName"_ustr;
    inline constexpr OUString PROPERTY_FILTER = u"Filter"_ustr;
    inline constexpr OUString PROPERTY_ORDER = u"Order"_ustr;
    inline constexpr OUString PROPERTY_ALLOWDELETES = u"AllowDeletes"_ustr;
    inline constexpr OUString PROPERTY_ALLOWINSERTS = u"AllowInserts"_ustr;
    inline constexpr OUString PROPERTY_ALLOWUPDATES = u"AllowUpdates"_ustr;
    inline constexpr OUString PROPERTY_APPLYFILTER = u"ApplyFilter"_ustr;
    inline constexpr OUString PROPERTY_ESCAPEPROCESSING = u"EscapeProcessing"_ustr;
    inline constexpr OUString PROPERTY_IGNORERESULT = u"IgnoreResult"_ustr;
    inline constexpr OUString PROPERTY_SUBMIT_ENCODING = u"SubmitEncoding"_ustr;
    inline constexpr OUString PROPERTY_SUBMIT_METHOD = u"SubmitMethod"_ustr;
    inline constexpr OUString PROPERTY_COMMAND_TYPE = u"CommandType"_ustr;
    inline constexpr OUString PROPERTY_NAVIGATION = u"NavigationBarMode"_ustr;
    inline constexpr OUString PROPERTY_CYCLE = u"Cycle"_ustr;
    inline constexpr OUString PROPERTY_BUTTONTYPE = u"ButtonType"_ustr;
    inline constexpr OUString PROPERTY_DATAFIELD = u"DataField"_ustr;
    inline constexpr OUString PROPERTY_BOUNDCOLUMN = u"BoundColumn"_ustr;
    inline constexpr OUString PROPERTY_EMPTY_IS_NULL = u"ConvertEmptyToNull"_ustr;
    inline constexpr OUString PROPERTY_INPUT_REQUIRED = u"InputRequired"_ustr;
    inline constexpr OUString PROPERTY_LISTSOURCE = u"ListSource"_ustr;
    inline constexpr OUString PROPERTY_LISTSOURCETYPE = u"ListSourceType"_ustr;
    inline constexpr OUString PROPERTY_ECHO_CHAR = u"EchoChar"_ustr;
    inline constexpr OUString PROPERTY_STRICTFORMAT = u"StrictFormat"_ustr;
    inline constexpr OUString PROPERTY_AUTOCOMPLETE = u"Autocomplete"_ustr;
    inline constexpr OUString PROPERTY_MULTISELECTION = u"MultiSelection"_ustr;
    inline constexpr OUString PROPERTY_DEFAULTBUTTON = u"DefaultButton"_ustr;
    inline constexpr OUString PROPERTY_TRISTATE = u"TriState"_ustr;
    inline constexpr OUString PROPERTY_CONTROLLABEL = u"LabelControl"_ustr;
    inline constexpr OUString PROPERTY_STRING_ITEM_LIST = u"StringItemList"_ustr;
    inline constexpr OUString PROPERTY_VALUE_SEQ = u"ValueItemList"_ustr;
    inline constexpr OUString PROPERTY_DEFAULT_SELECT_SEQ = u"DefaultSelection"_ustr;
    inline constexpr OUString PROPERTY_SELECT_SEQ = u"SelectedItems"_ustr;
    inline constexpr OUString PROPERTY_DATE_MIN = u"DateMin"_ustr;
    inline constexpr OUString PROPERTY_DATE_MAX = u"DateMax"_ustr;
    inline constexpr OUString PROPERTY_TIME_MIN = u"TimeMin"_ustr;
    inline constexpr OUString PROPERTY_TIME_MAX = u"TimeMax"_ustr;
    inline constexpr OUString PROPERTY_VALUE_MIN = u"ValueMin"_ustr;
    inline constexpr OUString PROPERTY_VALUE_MAX = u"ValueMax"_ustr;
    inline constexpr OUString PROPERTY_EFFECTIVE_MIN = u"EffectiveMin"_ustr;
    inline constexpr OUString PROPERTY_EFFECTIVE_MAX = u"EffectiveMax"_ustr;
    inline constexpr OUString PROPERTY_DEFAULT_DATE = u"DefaultDate"_ustr;
    inline constexpr OUString PROPERTY_DATE = u"Date"_ustr;
    inline constexpr OUString PROPERTY_DEFAULT_TIME = u"DefaultTime"_ustr;
    inline constexpr OUString PROPERTY_TIME = u"Time"_ustr;
    inline constexpr OUString PROPERTY_DEFAULT_VALUE = u"DefaultValue"_ustr;
    inline constexpr OUString PROPERTY_VALUE = u"Value"_ustr;
    inline constexpr OUString PROPERTY_HIDDEN_VALUE = u"HiddenValue"_ustr;
    inline constexpr OUString PROPERTY_DEFAULT_TEXT = u"DefaultText"_ustr;
    inline constexpr OUString PROPERTY_TEXT = u"Text"_ustr;
    inline constexpr OUString PROPERTY_EFFECTIVE_VALUE = u"EffectiveValue"_ustr;
    inline constexpr OUString PROPERTY_EFFECTIVE_DEFAULT = u"EffectiveDefault"_ustr;
    inline constexpr OUString PROPERTY_REFVALUE = u"RefValue"_ustr;
    inline constexpr OUString PROPERTY_URL = u"URL"_ustr;
    inline constexpr OUString PROPERTY_FONT = u"FontDescriptor"_ustr;
    inline constexpr OUString PROPERTY_BACKGROUNDCOLOR = u"BackgroundColor"_ustr;
    inline constexpr OUString PROPERTY_MASTERFIELDS = u"MasterFields"_ustr;
    inline constexpr OUString PROPERTY_DETAILFIELDS = u"DetailFields"_ustr;
    inline constexpr OUString PROPERTY_COLUMNSERVICENAME = u"ColumnServiceName"_ustr;
    inline constexpr OUString PROPERTY_FORMATKEY = u"FormatKey"_ustr;
    inline constexpr OUString PROPERTY_ALIGN = u"Align"_ustr;
    inline constexpr OUString PROPERTY_BORDER = u"Border"_ustr;
    inline constexpr OUString PROPERTY_AUTOCONTROLFOCUS = u"AutomaticControlFocus"_ustr;
    inline constexpr OUString PROPERTY_APPLYDESIGNMODE = u"ApplyFormDesignMode"_ustr;
    inline constexpr OUString PROPERTY_FORMATSSUPPLIER = u"FormatsSupplier"_ustr;
    inline constexpr OUString PROPERTY_LOCALE = u"Locale"_ustr;
    inline constexpr OUString PROPERTY_FORMATSTRING = u"FormatString"_ustr;
    inline constexpr OUString PROPERTY_DATEFORMAT = u"DateFormat"_ustr;
    inline constexpr OUString PROPERTY_TIMEFORMAT = u"TimeFormat"_ustr;
    inline constexpr OUString PROPERTY_PERSISTENCE_MAXTEXTLENGTH = u"PersistenceMaxTextLength"_ustr;
    inline constexpr OUString PROPERTY_SCROLLVALUE_MIN = u"ScrollValueMin"_ustr;
    inline constexpr OUString PROPERTY_SCROLLVALUE_MAX = u"ScrollValueMax"_ustr;
    inline constexpr OUString PROPERTY_SCROLLVALUE = u"ScrollValue"_ustr;
    inline constexpr OUString PROPERTY_SCROLLVALUE_DEFAULT = u"DefaultScrollValue"_ustr;
    inline constexpr OUString PROPERTY_LINE_INCREMENT = u"LineIncrement"_ustr;
    inline constexpr OUString PROPERTY_BLOCK_INCREMENT = u"BlockIncrement"_ustr;
    inline constexpr OUString PROPERTY_REPEAT_DELAY = u"RepeatDelay"_ustr;
    inline constexpr OUString PROPERTY_SPINVALUE = u"SpinValue"_ustr;
    inline constexpr OUString PROPERTY_SPINVALUE_MIN = u"SpinValueMin"_ustr;
    inline constexpr OUString PROPERTY_SPINVALUE_MAX = u"SpinValueMax"_ustr;
    inline constexpr OUString PROPERTY_DEFAULT_SPINVALUE = u"DefaultSpinValue"_ustr;
    inline constexpr OUString PROPERTY_SPIN_INCREMENT = u"SpinIncrement"_ustr;
    inline constexpr OUString PROPERTY_ORIENTATION = u"Orientation"_ustr;
    inline constexpr OUString PROPERTY_TOGGLE = u"Toggle"_ustr;
    inline constexpr OUString PROPERTY_FOCUS_ON_CLICK = u"FocusOnClick"_ustr;
    inline constexpr OUString PROPERTY_VISUAL_EFFECT = u"VisualEffect"_ustr;
    inline constexpr OUString PROPERTY_IMAGE_POSITION = u"ImagePosition"_ustr;
    inline constexpr OUString PROPERTY_IMAGE_ALIGN = u"ImageAlign"_ustr;
    inline constexpr OUString PROPERTY_GROUP_NAME = u"GroupName"_ustr;

    inline constexpr OUString PROPERTY_BOUND_CELL = u"BoundCell"_ustr;
    inline constexpr OUString PROPERTY_LIST_CELL_RANGE = u"CellRange"_ustr;
    inline constexpr OUString PROPERTY_ADDRESS = u"Address"_ustr;
    inline constexpr OUString PROPERTY_FILE_REPRESENTATION = u"PersistentRepresentation"_ustr;
    inline constexpr OUString PROPERTY_RICH_TEXT = u"RichText"_ustr;

    // services
    inline constexpr OUString SERVICE_SPREADSHEET_DOCUMENT = u"com.sun.star.sheet.SpreadsheetDocument"_ustr;
    inline constexpr OUString SERVICE_CELLVALUEBINDING  = u"com.sun.star.table.CellValueBinding"_ustr;
    inline constexpr OUString SERVICE_LISTINDEXCELLBINDING  = u"com.sun.star.table.ListPositionCellBinding"_ustr;
    inline constexpr OUString SERVICE_CELLRANGELISTSOURCE = u"com.sun.star.table.CellRangeListSource"_ustr;
    inline constexpr OUString SERVICE_ADDRESS_CONVERSION  = u"com.sun.star.table.CellAddressConversion"_ustr;
    inline constexpr OUString SERVICE_RANGEADDRESS_CONVERSION = u"com.sun.star.table.CellRangeAddressConversion"_ustr;

    // old service names (compatibility)
    #define SERVICE_PERSISTENT_COMPONENT_FORM "stardiv.one.form.component.Form"
    #define SERVICE_PERSISTENT_COMPONENT_EDIT "stardiv.one.form.component.Edit"
    #define SERVICE_PERSISTENT_COMPONENT_LISTBOX "stardiv.one.form.component.ListBox"
    #define SERVICE_PERSISTENT_COMPONENT_COMBOBOX "stardiv.one.form.component.ComboBox"
    #define SERVICE_PERSISTENT_COMPONENT_RADIOBUTTON "stardiv.one.form.component.RadioButton"
    #define SERVICE_PERSISTENT_COMPONENT_GROUPBOX "stardiv.one.form.component.GroupBox"
    #define SERVICE_PERSISTENT_COMPONENT_FIXEDTEXT "stardiv.one.form.component.FixedText"
    #define SERVICE_PERSISTENT_COMPONENT_COMMANDBUTTON "stardiv.one.form.component.CommandButton"
    #define SERVICE_PERSISTENT_COMPONENT_CHECKBOX "stardiv.one.form.component.CheckBox"
    #define SERVICE_PERSISTENT_COMPONENT_GRID "stardiv.one.form.component.Grid"
    #define SERVICE_PERSISTENT_COMPONENT_IMAGEBUTTON "stardiv.one.form.component.ImageButton"
    #define SERVICE_PERSISTENT_COMPONENT_FILECONTROL "stardiv.one.form.component.FileControl"
    #define SERVICE_PERSISTENT_COMPONENT_TIMEFIELD "stardiv.one.form.component.TimeField"
    #define SERVICE_PERSISTENT_COMPONENT_DATEFIELD "stardiv.one.form.component.DateField"
    #define SERVICE_PERSISTENT_COMPONENT_NUMERICFIELD "stardiv.one.form.component.NumericField"
    #define SERVICE_PERSISTENT_COMPONENT_CURRENCYFIELD "stardiv.one.form.component.CurrencyField"
    #define SERVICE_PERSISTENT_COMPONENT_PATTERNFIELD "stardiv.one.form.component.PatternField"
    #define SERVICE_PERSISTENT_COMPONENT_HIDDENCONTROL "stardiv.one.form.component.Hidden"
    #define SERVICE_PERSISTENT_COMPONENT_IMAGECONTROL "stardiv.one.form.component.ImageControl"
    #define SERVICE_PERSISTENT_COMPONENT_FORMATTEDFIELD "stardiv.one.form.component.FormattedField"

    // new service names, the old ones are translated into this new ones
    inline constexpr OUString SERVICE_FORM = u"com.sun.star.form.component.Form"_ustr;
    inline constexpr OUString SERVICE_EDIT = u"com.sun.star.form.component.TextField"_ustr;
    inline constexpr OUString SERVICE_LISTBOX = u"com.sun.star.form.component.ListBox"_ustr;
    inline constexpr OUString SERVICE_COMBOBOX = u"com.sun.star.form.component.ComboBox"_ustr;
    inline constexpr OUString SERVICE_RADIOBUTTON = u"com.sun.star.form.component.RadioButton"_ustr;
    inline constexpr OUString SERVICE_GROUPBOX = u"com.sun.star.form.component.GroupBox"_ustr;
    inline constexpr OUString SERVICE_FIXEDTEXT = u"com.sun.star.form.component.FixedText"_ustr;
    inline constexpr OUString SERVICE_COMMANDBUTTON = u"com.sun.star.form.component.CommandButton"_ustr;
    inline constexpr OUString SERVICE_CHECKBOX = u"com.sun.star.form.component.CheckBox"_ustr;
    inline constexpr OUString SERVICE_GRID = u"com.sun.star.form.component.GridControl"_ustr;
    inline constexpr OUString SERVICE_IMAGEBUTTON = u"com.sun.star.form.component.ImageButton"_ustr;
    inline constexpr OUString SERVICE_FILECONTROL = u"com.sun.star.form.component.FileControl"_ustr;
    inline constexpr OUString SERVICE_TIMEFIELD = u"com.sun.star.form.component.TimeField"_ustr;
    inline constexpr OUString SERVICE_DATEFIELD = u"com.sun.star.form.component.DateField"_ustr;
    inline constexpr OUString SERVICE_NUMERICFIELD = u"com.sun.star.form.component.NumericField"_ustr;
    inline constexpr OUString SERVICE_CURRENCYFIELD = u"com.sun.star.form.component.CurrencyField"_ustr;
    inline constexpr OUString SERVICE_PATTERNFIELD = u"com.sun.star.form.component.PatternField"_ustr;
    inline constexpr OUString SERVICE_HIDDENCONTROL = u"com.sun.star.form.component.HiddenControl"_ustr;
    inline constexpr OUString SERVICE_IMAGECONTROL = u"com.sun.star.form.component.DatabaseImageControl"_ustr;
    inline constexpr OUString SERVICE_FORMATTEDFIELD = u"com.sun.star.form.component.FormattedField"_ustr;

    // various strings
    #define EVENT_NAME_SEPARATOR "::"
    inline constexpr OUString EVENT_TYPE = u"EventType"_ustr;
    inline constexpr OUString EVENT_LIBRARY = u"Library"_ustr;
    inline constexpr OUString EVENT_LOCALMACRONAME = u"MacroName"_ustr;
    inline constexpr OUString EVENT_SCRIPTURL = u"Script"_ustr;
    inline constexpr OUString EVENT_STAROFFICE = u"StarOffice"_ustr;
    #define EVENT_STARBASIC "StarBasic"
    inline constexpr OUString EVENT_APPLICATION = u"application"_ustr;

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
