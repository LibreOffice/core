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

namespace xmloff
{

    // properties
    constexpr OUStringLiteral PROPERTY_CLASSID = u"ClassId";
    constexpr OUStringLiteral PROPERTY_ECHOCHAR = u"EchoChar";
    constexpr OUStringLiteral PROPERTY_MULTILINE = u"MultiLine";
    constexpr OUStringLiteral PROPERTY_NAME = u"Name";
    constexpr OUStringLiteral PROPERTY_GRAPHIC = u"Graphic";
    constexpr OUStringLiteral PROPERTY_LABEL = u"Label";
    constexpr OUStringLiteral PROPERTY_TARGETFRAME = u"TargetFrame";
    constexpr OUStringLiteral PROPERTY_TARGETURL = u"TargetURL";
    constexpr OUStringLiteral PROPERTY_TITLE = u"Tag";
    constexpr OUStringLiteral PROPERTY_DROPDOWN = u"Dropdown";
    constexpr OUStringLiteral PROPERTY_PRINTABLE = u"Printable";
    constexpr OUStringLiteral PROPERTY_READONLY = u"ReadOnly";
    constexpr OUStringLiteral PROPERTY_DEFAULT_STATE = u"DefaultState";
    constexpr OUStringLiteral PROPERTY_TABSTOP = u"Tabstop";
    constexpr OUStringLiteral PROPERTY_STATE = u"State";
    constexpr OUStringLiteral PROPERTY_ENABLED = u"Enabled";
    constexpr OUStringLiteral PROPERTY_ENABLEVISIBLE = u"EnableVisible";
    constexpr OUStringLiteral PROPERTY_MAXTEXTLENGTH  = u"MaxTextLen";
    constexpr OUStringLiteral PROPERTY_LINECOUNT = u"LineCount";
    constexpr OUStringLiteral PROPERTY_TABINDEX = u"TabIndex";
    constexpr OUStringLiteral PROPERTY_COMMAND = u"Command";
    constexpr OUStringLiteral PROPERTY_DATASOURCENAME = u"DataSourceName";
    constexpr OUStringLiteral PROPERTY_FILTER = u"Filter";
    constexpr OUStringLiteral PROPERTY_ORDER = u"Order";
    constexpr OUStringLiteral PROPERTY_ALLOWDELETES = u"AllowDeletes";
    constexpr OUStringLiteral PROPERTY_ALLOWINSERTS = u"AllowInserts";
    constexpr OUStringLiteral PROPERTY_ALLOWUPDATES = u"AllowUpdates";
    constexpr OUStringLiteral PROPERTY_APPLYFILTER = u"ApplyFilter";
    constexpr OUStringLiteral PROPERTY_ESCAPEPROCESSING = u"EscapeProcessing";
    constexpr OUStringLiteral PROPERTY_IGNORERESULT = u"IgnoreResult";
    constexpr OUStringLiteral PROPERTY_SUBMIT_ENCODING = u"SubmitEncoding";
    constexpr OUStringLiteral PROPERTY_SUBMIT_METHOD = u"SubmitMethod";
    constexpr OUStringLiteral PROPERTY_COMMAND_TYPE = u"CommandType";
    constexpr OUStringLiteral PROPERTY_NAVIGATION = u"NavigationBarMode";
    constexpr OUStringLiteral PROPERTY_CYCLE = u"Cycle";
    constexpr OUStringLiteral PROPERTY_BUTTONTYPE = u"ButtonType";
    constexpr OUStringLiteral PROPERTY_DATAFIELD = u"DataField";
    constexpr OUStringLiteral PROPERTY_BOUNDCOLUMN = u"BoundColumn";
    constexpr OUStringLiteral PROPERTY_EMPTY_IS_NULL = u"ConvertEmptyToNull";
    constexpr OUStringLiteral PROPERTY_INPUT_REQUIRED = u"InputRequired";
    constexpr OUStringLiteral PROPERTY_LISTSOURCE = u"ListSource";
    constexpr OUStringLiteral PROPERTY_LISTSOURCETYPE = u"ListSourceType";
    constexpr OUStringLiteral PROPERTY_ECHO_CHAR = u"EchoChar";
    constexpr OUStringLiteral PROPERTY_STRICTFORMAT = u"StrictFormat";
    constexpr OUStringLiteral PROPERTY_AUTOCOMPLETE = u"Autocomplete";
    constexpr OUStringLiteral PROPERTY_MULTISELECTION = u"MultiSelection";
    constexpr OUStringLiteral PROPERTY_DEFAULTBUTTON = u"DefaultButton";
    constexpr OUStringLiteral PROPERTY_TRISTATE = u"TriState";
    constexpr OUStringLiteral PROPERTY_CONTROLLABEL = u"LabelControl";
    constexpr OUStringLiteral PROPERTY_STRING_ITEM_LIST = u"StringItemList";
    constexpr OUStringLiteral PROPERTY_VALUE_SEQ = u"ValueItemList";
    constexpr OUStringLiteral PROPERTY_DEFAULT_SELECT_SEQ = u"DefaultSelection";
    constexpr OUStringLiteral PROPERTY_SELECT_SEQ = u"SelectedItems";
    constexpr OUStringLiteral PROPERTY_DATE_MIN = u"DateMin";
    constexpr OUStringLiteral PROPERTY_DATE_MAX = u"DateMax";
    constexpr OUStringLiteral PROPERTY_TIME_MIN = u"TimeMin";
    constexpr OUStringLiteral PROPERTY_TIME_MAX = u"TimeMax";
    #define PROPERTY_VALUE_MIN "ValueMin"
    #define PROPERTY_VALUE_MAX "ValueMax"
    #define PROPERTY_EFFECTIVE_MIN "EffectiveMin"
    #define PROPERTY_EFFECTIVE_MAX "EffectiveMax"
    constexpr OUStringLiteral PROPERTY_DEFAULT_DATE = u"DefaultDate";
    constexpr OUStringLiteral PROPERTY_DATE = u"Date";
    constexpr OUStringLiteral PROPERTY_DEFAULT_TIME = u"DefaultTime";
    constexpr OUStringLiteral PROPERTY_TIME = u"Time";
    #define PROPERTY_DEFAULT_VALUE "DefaultValue"
    #define PROPERTY_VALUE "Value"
    #define PROPERTY_HIDDEN_VALUE "HiddenValue"
    #define PROPERTY_DEFAULT_TEXT "DefaultText"
    #define PROPERTY_TEXT "Text"
    #define PROPERTY_EFFECTIVE_VALUE "EffectiveValue"
    #define PROPERTY_EFFECTIVE_DEFAULT "EffectiveDefault"
    #define PROPERTY_REFVALUE "RefValue"
    constexpr OUStringLiteral PROPERTY_URL = u"URL";
    constexpr OUStringLiteral PROPERTY_FONT = u"FontDescriptor";
    constexpr OUStringLiteral PROPERTY_BACKGROUNDCOLOR  = u"BackgroundColor";
    constexpr OUStringLiteral PROPERTY_MASTERFIELDS = u"MasterFields";
    constexpr OUStringLiteral PROPERTY_DETAILFIELDS = u"DetailFields";
    constexpr OUStringLiteral PROPERTY_COLUMNSERVICENAME = u"ColumnServiceName";
    constexpr OUStringLiteral PROPERTY_FORMATKEY = u"FormatKey";
    constexpr OUStringLiteral PROPERTY_ALIGN = u"Align";
    constexpr OUStringLiteral PROPERTY_BORDER = u"Border";
    constexpr OUStringLiteral PROPERTY_AUTOCONTROLFOCUS = u"AutomaticControlFocus";
    constexpr OUStringLiteral PROPERTY_APPLYDESIGNMODE = u"ApplyFormDesignMode";
    constexpr OUStringLiteral PROPERTY_FORMATSSUPPLIER = u"FormatsSupplier";
    constexpr OUStringLiteral PROPERTY_LOCALE = u"Locale";
    constexpr OUStringLiteral PROPERTY_FORMATSTRING = u"FormatString";
    constexpr OUStringLiteral PROPERTY_DATEFORMAT = u"DateFormat";
    constexpr OUStringLiteral PROPERTY_TIMEFORMAT = u"TimeFormat";
    constexpr OUStringLiteral PROPERTY_PERSISTENCE_MAXTEXTLENGTH = u"PersistenceMaxTextLength";
    #define PROPERTY_SCROLLVALUE_MIN "ScrollValueMin"
    #define PROPERTY_SCROLLVALUE_MAX "ScrollValueMax"
    #define PROPERTY_SCROLLVALUE "ScrollValue"
    #define PROPERTY_SCROLLVALUE_DEFAULT "DefaultScrollValue"
    constexpr OUStringLiteral PROPERTY_LINE_INCREMENT = u"LineIncrement";
    constexpr OUStringLiteral PROPERTY_BLOCK_INCREMENT = u"BlockIncrement";
    constexpr OUStringLiteral PROPERTY_REPEAT_DELAY = u"RepeatDelay";
    #define PROPERTY_SPINVALUE "SpinValue"
    #define PROPERTY_SPINVALUE_MIN "SpinValueMin"
    #define PROPERTY_SPINVALUE_MAX "SpinValueMax"
    #define PROPERTY_DEFAULT_SPINVALUE "DefaultSpinValue"
    constexpr OUStringLiteral PROPERTY_SPIN_INCREMENT = u"SpinIncrement";
    constexpr OUStringLiteral PROPERTY_ORIENTATION = u"Orientation";
    constexpr OUStringLiteral PROPERTY_TOGGLE = u"Toggle";
    constexpr OUStringLiteral PROPERTY_FOCUS_ON_CLICK = u"FocusOnClick";
    constexpr OUStringLiteral PROPERTY_VISUAL_EFFECT = u"VisualEffect";
    constexpr OUStringLiteral PROPERTY_IMAGE_POSITION = u"ImagePosition";
    constexpr OUStringLiteral PROPERTY_IMAGE_ALIGN = u"ImageAlign";
    #define PROPERTY_SCALE_IMAGE "ScaleImage"
    constexpr OUStringLiteral PROPERTY_GROUP_NAME  = u"GroupName";

    constexpr OUStringLiteral PROPERTY_BOUND_CELL = u"BoundCell";
    constexpr OUStringLiteral PROPERTY_LIST_CELL_RANGE = u"CellRange";
    constexpr OUStringLiteral PROPERTY_ADDRESS = u"Address";
    constexpr OUStringLiteral PROPERTY_FILE_REPRESENTATION = u"PersistentRepresentation";
    constexpr OUStringLiteral PROPERTY_RICH_TEXT = u"RichText";

    // services
    constexpr OUStringLiteral SERVICE_SPREADSHEET_DOCUMENT = u"com.sun.star.sheet.SpreadsheetDocument";
    constexpr OUStringLiteral SERVICE_CELLVALUEBINDING  = u"com.sun.star.table.CellValueBinding";
    constexpr OUStringLiteral SERVICE_LISTINDEXCELLBINDING  = u"com.sun.star.table.ListPositionCellBinding";
    constexpr OUStringLiteral SERVICE_CELLRANGELISTSOURCE = u"com.sun.star.table.CellRangeListSource";
    constexpr OUStringLiteral SERVICE_ADDRESS_CONVERSION  = u"com.sun.star.table.CellAddressConversion";
    constexpr OUStringLiteral SERVICE_RANGEADDRESS_CONVERSION = u"com.sun.star.table.CellRangeAddressConversion";

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
    constexpr OUStringLiteral SERVICE_FORM = u"com.sun.star.form.component.Form";
    constexpr OUStringLiteral SERVICE_EDIT = u"com.sun.star.form.component.TextField";
    constexpr OUStringLiteral SERVICE_LISTBOX = u"com.sun.star.form.component.ListBox";
    constexpr OUStringLiteral SERVICE_COMBOBOX = u"com.sun.star.form.component.ComboBox";
    constexpr OUStringLiteral SERVICE_RADIOBUTTON = u"com.sun.star.form.component.RadioButton";
    constexpr OUStringLiteral SERVICE_GROUPBOX = u"com.sun.star.form.component.GroupBox";
    constexpr OUStringLiteral SERVICE_FIXEDTEXT = u"com.sun.star.form.component.FixedText";
    constexpr OUStringLiteral SERVICE_COMMANDBUTTON = u"com.sun.star.form.component.CommandButton";
    constexpr OUStringLiteral SERVICE_CHECKBOX = u"com.sun.star.form.component.CheckBox";
    constexpr OUStringLiteral SERVICE_GRID = u"com.sun.star.form.component.GridControl";
    constexpr OUStringLiteral SERVICE_IMAGEBUTTON = u"com.sun.star.form.component.ImageButton";
    constexpr OUStringLiteral SERVICE_FILECONTROL = u"com.sun.star.form.component.FileControl";
    constexpr OUStringLiteral SERVICE_TIMEFIELD = u"com.sun.star.form.component.TimeField";
    constexpr OUStringLiteral SERVICE_DATEFIELD = u"com.sun.star.form.component.DateField";
    constexpr OUStringLiteral SERVICE_NUMERICFIELD = u"com.sun.star.form.component.NumericField";
    constexpr OUStringLiteral SERVICE_CURRENCYFIELD = u"com.sun.star.form.component.CurrencyField";
    constexpr OUStringLiteral SERVICE_PATTERNFIELD = u"com.sun.star.form.component.PatternField";
    constexpr OUStringLiteral SERVICE_HIDDENCONTROL = u"com.sun.star.form.component.HiddenControl";
    constexpr OUStringLiteral SERVICE_IMAGECONTROL = u"com.sun.star.form.component.DatabaseImageControl";
    constexpr OUStringLiteral SERVICE_FORMATTEDFIELD = u"com.sun.star.form.component.FormattedField";

    // various strings
    #define EVENT_NAME_SEPARATOR "::"
    constexpr OUStringLiteral EVENT_TYPE = u"EventType";
    constexpr OUStringLiteral EVENT_LIBRARY = u"Library";
    constexpr OUStringLiteral EVENT_LOCALMACRONAME = u"MacroName";
    constexpr OUStringLiteral EVENT_SCRIPTURL = u"Script";
    constexpr OUStringLiteral EVENT_STAROFFICE = u"StarOffice";
    #define EVENT_STARBASIC "StarBasic"
    constexpr OUStringLiteral EVENT_APPLICATION = u"application";

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
