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
    inline constexpr OUStringLiteral PROPERTY_CLASSID = u"ClassId";
    inline constexpr OUStringLiteral PROPERTY_ECHOCHAR = u"EchoChar";
    inline constexpr OUStringLiteral PROPERTY_MULTILINE = u"MultiLine";
    inline constexpr OUStringLiteral PROPERTY_NAME = u"Name";
    inline constexpr OUStringLiteral PROPERTY_GRAPHIC = u"Graphic";
    inline constexpr OUStringLiteral PROPERTY_LABEL = u"Label";
    inline constexpr OUStringLiteral PROPERTY_TARGETFRAME = u"TargetFrame";
    inline constexpr OUStringLiteral PROPERTY_TARGETURL = u"TargetURL";
    inline constexpr OUStringLiteral PROPERTY_TITLE = u"Tag";
    inline constexpr OUStringLiteral PROPERTY_DROPDOWN = u"Dropdown";
    inline constexpr OUStringLiteral PROPERTY_PRINTABLE = u"Printable";
    inline constexpr OUStringLiteral PROPERTY_READONLY = u"ReadOnly";
    inline constexpr OUStringLiteral PROPERTY_DEFAULT_STATE = u"DefaultState";
    inline constexpr OUStringLiteral PROPERTY_TABSTOP = u"Tabstop";
    inline constexpr OUStringLiteral PROPERTY_STATE = u"State";
    inline constexpr OUStringLiteral PROPERTY_ENABLED = u"Enabled";
    inline constexpr OUStringLiteral PROPERTY_ENABLEVISIBLE = u"EnableVisible";
    inline constexpr OUStringLiteral PROPERTY_MAXTEXTLENGTH  = u"MaxTextLen";
    inline constexpr OUStringLiteral PROPERTY_LINECOUNT = u"LineCount";
    inline constexpr OUStringLiteral PROPERTY_TABINDEX = u"TabIndex";
    inline constexpr OUStringLiteral PROPERTY_COMMAND = u"Command";
    inline constexpr OUStringLiteral PROPERTY_DATASOURCENAME = u"DataSourceName";
    inline constexpr OUStringLiteral PROPERTY_FILTER = u"Filter";
    inline constexpr OUStringLiteral PROPERTY_ORDER = u"Order";
    inline constexpr OUStringLiteral PROPERTY_ALLOWDELETES = u"AllowDeletes";
    inline constexpr OUStringLiteral PROPERTY_ALLOWINSERTS = u"AllowInserts";
    inline constexpr OUStringLiteral PROPERTY_ALLOWUPDATES = u"AllowUpdates";
    inline constexpr OUStringLiteral PROPERTY_APPLYFILTER = u"ApplyFilter";
    inline constexpr OUStringLiteral PROPERTY_ESCAPEPROCESSING = u"EscapeProcessing";
    inline constexpr OUStringLiteral PROPERTY_IGNORERESULT = u"IgnoreResult";
    inline constexpr OUStringLiteral PROPERTY_SUBMIT_ENCODING = u"SubmitEncoding";
    inline constexpr OUStringLiteral PROPERTY_SUBMIT_METHOD = u"SubmitMethod";
    inline constexpr OUStringLiteral PROPERTY_COMMAND_TYPE = u"CommandType";
    inline constexpr OUStringLiteral PROPERTY_NAVIGATION = u"NavigationBarMode";
    inline constexpr OUStringLiteral PROPERTY_CYCLE = u"Cycle";
    inline constexpr OUStringLiteral PROPERTY_BUTTONTYPE = u"ButtonType";
    inline constexpr OUStringLiteral PROPERTY_DATAFIELD = u"DataField";
    inline constexpr OUStringLiteral PROPERTY_BOUNDCOLUMN = u"BoundColumn";
    inline constexpr OUStringLiteral PROPERTY_EMPTY_IS_NULL = u"ConvertEmptyToNull";
    inline constexpr OUStringLiteral PROPERTY_INPUT_REQUIRED = u"InputRequired";
    inline constexpr OUStringLiteral PROPERTY_LISTSOURCE = u"ListSource";
    inline constexpr OUStringLiteral PROPERTY_LISTSOURCETYPE = u"ListSourceType";
    inline constexpr OUStringLiteral PROPERTY_ECHO_CHAR = u"EchoChar";
    inline constexpr OUStringLiteral PROPERTY_STRICTFORMAT = u"StrictFormat";
    inline constexpr OUStringLiteral PROPERTY_AUTOCOMPLETE = u"Autocomplete";
    inline constexpr OUStringLiteral PROPERTY_MULTISELECTION = u"MultiSelection";
    inline constexpr OUStringLiteral PROPERTY_DEFAULTBUTTON = u"DefaultButton";
    inline constexpr OUStringLiteral PROPERTY_TRISTATE = u"TriState";
    inline constexpr OUStringLiteral PROPERTY_CONTROLLABEL = u"LabelControl";
    inline constexpr OUStringLiteral PROPERTY_STRING_ITEM_LIST = u"StringItemList";
    inline constexpr OUStringLiteral PROPERTY_VALUE_SEQ = u"ValueItemList";
    inline constexpr OUStringLiteral PROPERTY_DEFAULT_SELECT_SEQ = u"DefaultSelection";
    inline constexpr OUStringLiteral PROPERTY_SELECT_SEQ = u"SelectedItems";
    inline constexpr OUStringLiteral PROPERTY_DATE_MIN = u"DateMin";
    inline constexpr OUStringLiteral PROPERTY_DATE_MAX = u"DateMax";
    inline constexpr OUStringLiteral PROPERTY_TIME_MIN = u"TimeMin";
    inline constexpr OUStringLiteral PROPERTY_TIME_MAX = u"TimeMax";
    #define PROPERTY_VALUE_MIN "ValueMin"
    #define PROPERTY_VALUE_MAX "ValueMax"
    #define PROPERTY_EFFECTIVE_MIN "EffectiveMin"
    #define PROPERTY_EFFECTIVE_MAX "EffectiveMax"
    inline constexpr OUStringLiteral PROPERTY_DEFAULT_DATE = u"DefaultDate";
    inline constexpr OUStringLiteral PROPERTY_DATE = u"Date";
    inline constexpr OUStringLiteral PROPERTY_DEFAULT_TIME = u"DefaultTime";
    inline constexpr OUStringLiteral PROPERTY_TIME = u"Time";
    #define PROPERTY_DEFAULT_VALUE "DefaultValue"
    #define PROPERTY_VALUE "Value"
    #define PROPERTY_HIDDEN_VALUE "HiddenValue"
    #define PROPERTY_DEFAULT_TEXT "DefaultText"
    #define PROPERTY_TEXT "Text"
    #define PROPERTY_EFFECTIVE_VALUE "EffectiveValue"
    #define PROPERTY_EFFECTIVE_DEFAULT "EffectiveDefault"
    #define PROPERTY_REFVALUE "RefValue"
    inline constexpr OUStringLiteral PROPERTY_URL = u"URL";
    inline constexpr OUStringLiteral PROPERTY_FONT = u"FontDescriptor";
    inline constexpr OUStringLiteral PROPERTY_BACKGROUNDCOLOR  = u"BackgroundColor";
    inline constexpr OUStringLiteral PROPERTY_MASTERFIELDS = u"MasterFields";
    inline constexpr OUStringLiteral PROPERTY_DETAILFIELDS = u"DetailFields";
    inline constexpr OUStringLiteral PROPERTY_COLUMNSERVICENAME = u"ColumnServiceName";
    inline constexpr OUStringLiteral PROPERTY_FORMATKEY = u"FormatKey";
    inline constexpr OUStringLiteral PROPERTY_ALIGN = u"Align";
    inline constexpr OUStringLiteral PROPERTY_BORDER = u"Border";
    inline constexpr OUStringLiteral PROPERTY_AUTOCONTROLFOCUS = u"AutomaticControlFocus";
    inline constexpr OUStringLiteral PROPERTY_APPLYDESIGNMODE = u"ApplyFormDesignMode";
    inline constexpr OUStringLiteral PROPERTY_FORMATSSUPPLIER = u"FormatsSupplier";
    inline constexpr OUStringLiteral PROPERTY_LOCALE = u"Locale";
    inline constexpr OUStringLiteral PROPERTY_FORMATSTRING = u"FormatString";
    inline constexpr OUStringLiteral PROPERTY_DATEFORMAT = u"DateFormat";
    inline constexpr OUStringLiteral PROPERTY_TIMEFORMAT = u"TimeFormat";
    inline constexpr OUStringLiteral PROPERTY_PERSISTENCE_MAXTEXTLENGTH = u"PersistenceMaxTextLength";
    #define PROPERTY_SCROLLVALUE_MIN "ScrollValueMin"
    #define PROPERTY_SCROLLVALUE_MAX "ScrollValueMax"
    #define PROPERTY_SCROLLVALUE "ScrollValue"
    #define PROPERTY_SCROLLVALUE_DEFAULT "DefaultScrollValue"
    inline constexpr OUStringLiteral PROPERTY_LINE_INCREMENT = u"LineIncrement";
    inline constexpr OUStringLiteral PROPERTY_BLOCK_INCREMENT = u"BlockIncrement";
    inline constexpr OUStringLiteral PROPERTY_REPEAT_DELAY = u"RepeatDelay";
    #define PROPERTY_SPINVALUE "SpinValue"
    #define PROPERTY_SPINVALUE_MIN "SpinValueMin"
    #define PROPERTY_SPINVALUE_MAX "SpinValueMax"
    #define PROPERTY_DEFAULT_SPINVALUE "DefaultSpinValue"
    inline constexpr OUStringLiteral PROPERTY_SPIN_INCREMENT = u"SpinIncrement";
    inline constexpr OUStringLiteral PROPERTY_ORIENTATION = u"Orientation";
    inline constexpr OUStringLiteral PROPERTY_TOGGLE = u"Toggle";
    inline constexpr OUStringLiteral PROPERTY_FOCUS_ON_CLICK = u"FocusOnClick";
    inline constexpr OUStringLiteral PROPERTY_VISUAL_EFFECT = u"VisualEffect";
    inline constexpr OUStringLiteral PROPERTY_IMAGE_POSITION = u"ImagePosition";
    inline constexpr OUStringLiteral PROPERTY_IMAGE_ALIGN = u"ImageAlign";
    #define PROPERTY_SCALE_IMAGE "ScaleImage"
    inline constexpr OUStringLiteral PROPERTY_GROUP_NAME  = u"GroupName";

    inline constexpr OUStringLiteral PROPERTY_BOUND_CELL = u"BoundCell";
    inline constexpr OUStringLiteral PROPERTY_LIST_CELL_RANGE = u"CellRange";
    inline constexpr OUStringLiteral PROPERTY_ADDRESS = u"Address";
    inline constexpr OUStringLiteral PROPERTY_FILE_REPRESENTATION = u"PersistentRepresentation";
    inline constexpr OUStringLiteral PROPERTY_RICH_TEXT = u"RichText";

    // services
    inline constexpr OUStringLiteral SERVICE_SPREADSHEET_DOCUMENT = u"com.sun.star.sheet.SpreadsheetDocument";
    inline constexpr OUStringLiteral SERVICE_CELLVALUEBINDING  = u"com.sun.star.table.CellValueBinding";
    inline constexpr OUStringLiteral SERVICE_LISTINDEXCELLBINDING  = u"com.sun.star.table.ListPositionCellBinding";
    inline constexpr OUStringLiteral SERVICE_CELLRANGELISTSOURCE = u"com.sun.star.table.CellRangeListSource";
    inline constexpr OUStringLiteral SERVICE_ADDRESS_CONVERSION  = u"com.sun.star.table.CellAddressConversion";
    inline constexpr OUStringLiteral SERVICE_RANGEADDRESS_CONVERSION = u"com.sun.star.table.CellRangeAddressConversion";

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
    inline constexpr OUStringLiteral SERVICE_FORM = u"com.sun.star.form.component.Form";
    inline constexpr OUStringLiteral SERVICE_EDIT = u"com.sun.star.form.component.TextField";
    inline constexpr OUStringLiteral SERVICE_LISTBOX = u"com.sun.star.form.component.ListBox";
    inline constexpr OUStringLiteral SERVICE_COMBOBOX = u"com.sun.star.form.component.ComboBox";
    inline constexpr OUStringLiteral SERVICE_RADIOBUTTON = u"com.sun.star.form.component.RadioButton";
    inline constexpr OUStringLiteral SERVICE_GROUPBOX = u"com.sun.star.form.component.GroupBox";
    inline constexpr OUStringLiteral SERVICE_FIXEDTEXT = u"com.sun.star.form.component.FixedText";
    inline constexpr OUStringLiteral SERVICE_COMMANDBUTTON = u"com.sun.star.form.component.CommandButton";
    inline constexpr OUStringLiteral SERVICE_CHECKBOX = u"com.sun.star.form.component.CheckBox";
    inline constexpr OUStringLiteral SERVICE_GRID = u"com.sun.star.form.component.GridControl";
    inline constexpr OUStringLiteral SERVICE_IMAGEBUTTON = u"com.sun.star.form.component.ImageButton";
    inline constexpr OUStringLiteral SERVICE_FILECONTROL = u"com.sun.star.form.component.FileControl";
    inline constexpr OUStringLiteral SERVICE_TIMEFIELD = u"com.sun.star.form.component.TimeField";
    inline constexpr OUStringLiteral SERVICE_DATEFIELD = u"com.sun.star.form.component.DateField";
    inline constexpr OUStringLiteral SERVICE_NUMERICFIELD = u"com.sun.star.form.component.NumericField";
    inline constexpr OUStringLiteral SERVICE_CURRENCYFIELD = u"com.sun.star.form.component.CurrencyField";
    inline constexpr OUStringLiteral SERVICE_PATTERNFIELD = u"com.sun.star.form.component.PatternField";
    inline constexpr OUStringLiteral SERVICE_HIDDENCONTROL = u"com.sun.star.form.component.HiddenControl";
    inline constexpr OUStringLiteral SERVICE_IMAGECONTROL = u"com.sun.star.form.component.DatabaseImageControl";
    inline constexpr OUStringLiteral SERVICE_FORMATTEDFIELD = u"com.sun.star.form.component.FormattedField";

    // various strings
    #define EVENT_NAME_SEPARATOR "::"
    inline constexpr OUStringLiteral EVENT_TYPE = u"EventType";
    inline constexpr OUStringLiteral EVENT_LIBRARY = u"Library";
    inline constexpr OUStringLiteral EVENT_LOCALMACRONAME = u"MacroName";
    inline constexpr OUStringLiteral EVENT_SCRIPTURL = u"Script";
    inline constexpr OUStringLiteral EVENT_STAROFFICE = u"StarOffice";
    #define EVENT_STARBASIC "StarBasic"
    inline constexpr OUStringLiteral EVENT_APPLICATION = u"application";

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
