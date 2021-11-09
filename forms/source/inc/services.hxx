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

#define AWT_CONTROL_TEXTFIELD "com.sun.star.awt.TextField"

#define VCL_CONTROL_EDIT "stardiv.vcl.control.Edit"
inline constexpr OUStringLiteral VCL_CONTROL_LISTBOX = u"stardiv.vcl.control.ListBox";
inline constexpr OUStringLiteral VCL_CONTROL_COMBOBOX = u"stardiv.vcl.control.ComboBox";
inline constexpr OUStringLiteral VCL_CONTROL_RADIOBUTTON = u"stardiv.vcl.control.RadioButton";
inline constexpr OUStringLiteral VCL_CONTROL_GROUPBOX = u"stardiv.vcl.control.GroupBox";
#define VCL_CONTROL_FIXEDTEXT "stardiv.vcl.control.FixedText"
inline constexpr OUStringLiteral VCL_CONTROL_COMMANDBUTTON = u"stardiv.vcl.control.Button";
inline constexpr OUStringLiteral VCL_CONTROL_CHECKBOX = u"stardiv.vcl.control.CheckBox";
inline constexpr OUStringLiteral VCL_CONTROL_IMAGEBUTTON = u"stardiv.vcl.control.ImageButton";
#define VCL_CONTROL_FILECONTROL "stardiv.vcl.control.FileControl"
inline constexpr OUStringLiteral VCL_CONTROL_TIMEFIELD = u"stardiv.vcl.control.TimeField";
inline constexpr OUStringLiteral VCL_CONTROL_DATEFIELD = u"stardiv.vcl.control.DateField";
inline constexpr OUStringLiteral VCL_CONTROL_NUMERICFIELD = u"stardiv.vcl.control.NumericField";
inline constexpr OUStringLiteral VCL_CONTROL_CURRENCYFIELD = u"stardiv.vcl.control.CurrencyField";
inline constexpr OUStringLiteral VCL_CONTROL_PATTERNFIELD = u"stardiv.vcl.control.PatternField";
inline constexpr OUStringLiteral VCL_CONTROL_FORMATTEDFIELD = u"stardiv.vcl.control.FormattedField";
inline constexpr OUStringLiteral VCL_CONTROL_IMAGECONTROL = u"stardiv.vcl.control.ImageControl";

inline constexpr OUStringLiteral VCL_CONTROLMODEL_EDIT = u"stardiv.vcl.controlmodel.Edit";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_LISTBOX = u"stardiv.vcl.controlmodel.ListBox";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_COMBOBOX = u"stardiv.vcl.controlmodel.ComboBox";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_RADIOBUTTON = u"stardiv.vcl.controlmodel.RadioButton";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_GROUPBOX = u"stardiv.vcl.controlmodel.GroupBox";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_FIXEDTEXT = u"stardiv.vcl.controlmodel.FixedText";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_COMMANDBUTTON = u"stardiv.vcl.controlmodel.Button";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_CHECKBOX = u"stardiv.vcl.controlmodel.CheckBox";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_IMAGEBUTTON = u"stardiv.vcl.controlmodel.ImageButton";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_FILECONTROL = u"stardiv.vcl.controlmodel.FileControl";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_TIMEFIELD = u"stardiv.vcl.controlmodel.TimeField";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_DATEFIELD = u"stardiv.vcl.controlmodel.DateField";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_NUMERICFIELD = u"stardiv.vcl.controlmodel.NumericField";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_CURRENCYFIELD = u"stardiv.vcl.controlmodel.CurrencyField";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_PATTERNFIELD = u"stardiv.vcl.controlmodel.PatternField";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_FORMATTEDFIELD = u"stardiv.vcl.controlmodel.FormattedField";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_IMAGECONTROL = u"stardiv.vcl.controlmodel.ImageControl";

inline constexpr OUStringLiteral VCL_CONTROLMODEL_SCROLLBAR = u"com.sun.star.awt.UnoControlScrollBarModel";
inline constexpr OUStringLiteral VCL_CONTROL_SCROLLBAR = u"com.sun.star.awt.UnoControlScrollBar";
inline constexpr OUStringLiteral VCL_CONTROLMODEL_SPINBUTTON = u"com.sun.star.awt.UnoControlSpinButtonModel";
inline constexpr OUStringLiteral VCL_CONTROL_SPINBUTTON = u"com.sun.star.awt.UnoControlSpinButton";

// service names for compatibility

inline constexpr OUStringLiteral FRM_COMPONENT_FORM = u"stardiv.one.form.component.Form";
inline constexpr OUStringLiteral FRM_COMPONENT_EDIT = u"stardiv.one.form.component.Edit"; // compatibility
inline constexpr OUStringLiteral FRM_COMPONENT_TEXTFIELD = u"stardiv.one.form.component.TextField";
inline constexpr OUStringLiteral FRM_COMPONENT_LISTBOX = u"stardiv.one.form.component.ListBox";
inline constexpr OUStringLiteral FRM_COMPONENT_COMBOBOX = u"stardiv.one.form.component.ComboBox";
inline constexpr OUStringLiteral FRM_COMPONENT_RADIOBUTTON = u"stardiv.one.form.component.RadioButton";
inline constexpr OUStringLiteral FRM_COMPONENT_GROUPBOX = u"stardiv.one.form.component.GroupBox"; // compatibility
inline constexpr OUStringLiteral FRM_COMPONENT_FIXEDTEXT = u"stardiv.one.form.component.FixedText"; // compatibility
inline constexpr OUStringLiteral FRM_COMPONENT_COMMANDBUTTON = u"stardiv.one.form.component.CommandButton";
inline constexpr OUStringLiteral FRM_COMPONENT_CHECKBOX = u"stardiv.one.form.component.CheckBox";
inline constexpr OUStringLiteral FRM_COMPONENT_GRID = u"stardiv.one.form.component.Grid"; // compatibility
inline constexpr OUStringLiteral FRM_COMPONENT_GRIDCONTROL = u"stardiv.one.form.component.GridControl";
inline constexpr OUStringLiteral FRM_COMPONENT_IMAGEBUTTON = u"stardiv.one.form.component.ImageButton";
inline constexpr OUStringLiteral FRM_COMPONENT_FILECONTROL = u"stardiv.one.form.component.FileControl";
inline constexpr OUStringLiteral FRM_COMPONENT_TIMEFIELD = u"stardiv.one.form.component.TimeField";
inline constexpr OUStringLiteral FRM_COMPONENT_DATEFIELD = u"stardiv.one.form.component.DateField";
inline constexpr OUStringLiteral FRM_COMPONENT_NUMERICFIELD = u"stardiv.one.form.component.NumericField";
inline constexpr OUStringLiteral FRM_COMPONENT_CURRENCYFIELD = u"stardiv.one.form.component.CurrencyField";
inline constexpr OUStringLiteral FRM_COMPONENT_PATTERNFIELD = u"stardiv.one.form.component.PatternField";
inline constexpr OUStringLiteral FRM_COMPONENT_HIDDEN = u"stardiv.one.form.component.Hidden";
inline constexpr OUStringLiteral FRM_COMPONENT_HIDDENCONTROL = u"stardiv.one.form.component.HiddenControl";
inline constexpr OUStringLiteral FRM_COMPONENT_IMAGECONTROL = u"stardiv.one.form.component.ImageControl";
inline constexpr OUStringLiteral FRM_COMPONENT_FORMATTEDFIELD = u"stardiv.one.form.component.FormattedField";

// <compatibility_I>
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_COMMANDBUTTON = u"stardiv.one.form.control.CommandButton";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_RADIOBUTTON = u"stardiv.one.form.control.RadioButton";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_CHECKBOX = u"stardiv.one.form.control.CheckBox";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_EDIT = u"stardiv.one.form.control.Edit";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_LISTBOX = u"stardiv.one.form.control.ListBox";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_COMBOBOX = u"stardiv.one.form.control.ComboBox";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_GROUPBOX = u"stardiv.one.form.control.GroupBox";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_TEXTFIELD = u"stardiv.one.form.control.TextField";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_GRID = u"stardiv.one.form.control.Grid";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_IMAGEBUTTON = u"stardiv.one.form.control.ImageButton";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_TIMEFIELD = u"stardiv.one.form.control.TimeField";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_DATEFIELD = u"stardiv.one.form.control.DateField";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_NUMERICFIELD = u"stardiv.one.form.control.NumericField";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_CURRENCYFIELD = u"stardiv.one.form.control.CurrencyField";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_PATTERNFIELD = u"stardiv.one.form.control.PatternField";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_IMAGECONTROL = u"stardiv.one.form.control.ImageControl";
inline constexpr OUStringLiteral STARDIV_ONE_FORM_CONTROL_FORMATTEDFIELD = u"stardiv.one.form.control.FormattedField";
// </compatibility_I>

// new (sun) service names

inline constexpr OUStringLiteral FRM_SUN_COMPONENT_FORM = u"com.sun.star.form.component.Form";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_HTMLFORM = u"com.sun.star.form.component.HTMLForm";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATAFORM = u"com.sun.star.form.component.DataForm";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_TEXTFIELD = u"com.sun.star.form.component.TextField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_LISTBOX = u"com.sun.star.form.component.ListBox";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_COMBOBOX = u"com.sun.star.form.component.ComboBox";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_RADIOBUTTON = u"com.sun.star.form.component.RadioButton";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_GROUPBOX = u"com.sun.star.form.component.GroupBox";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_FIXEDTEXT = u"com.sun.star.form.component.FixedText";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_COMMANDBUTTON = u"com.sun.star.form.component.CommandButton";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_CHECKBOX = u"com.sun.star.form.component.CheckBox";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_GRIDCONTROL = u"com.sun.star.form.component.GridControl";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_IMAGEBUTTON = u"com.sun.star.form.component.ImageButton";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_FILECONTROL = u"com.sun.star.form.component.FileControl";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_TIMEFIELD = u"com.sun.star.form.component.TimeField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATEFIELD = u"com.sun.star.form.component.DateField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_NUMERICFIELD = u"com.sun.star.form.component.NumericField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_CURRENCYFIELD = u"com.sun.star.form.component.CurrencyField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_PATTERNFIELD = u"com.sun.star.form.component.PatternField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_HIDDENCONTROL = u"com.sun.star.form.component.HiddenControl";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_FORMATTEDFIELD = u"com.sun.star.form.component.FormattedField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_SCROLLBAR = u"com.sun.star.form.component.ScrollBar";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_SPINBUTTON = u"com.sun.star.form.component.SpinButton";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_RICHTEXTCONTROL = u"com.sun.star.form.component.RichTextControl";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_NAVTOOLBAR = u"com.sun.star.form.component.NavigationToolBar";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_SUBMITBUTTON = u"com.sun.star.form.component.SubmitButton";

inline constexpr OUStringLiteral FRM_SUN_COMPONENT_IMAGECONTROL = u"com.sun.star.form.component.DatabaseImageControl";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATABASE_RADIOBUTTON = u"com.sun.star.form.component.DatabaseRadioButton";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATABASE_CHECKBOX = u"com.sun.star.form.component.DatabaseCheckBox";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATABASE_LISTBOX = u"com.sun.star.form.component.DatabaseListBox";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATABASE_COMBOBOX = u"com.sun.star.form.component.DatabaseComboBox";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATABASE_FORMATTEDFIELD = u"com.sun.star.form.component.DatabaseFormattedField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATABASE_TEXTFIELD = u"com.sun.star.form.component.DatabaseTextField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATABASE_DATEFIELD = u"com.sun.star.form.component.DatabaseDateField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATABASE_TIMEFIELD = u"com.sun.star.form.component.DatabaseTimeField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATABASE_NUMERICFIELD = u"com.sun.star.form.component.DatabaseNumericField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATABASE_CURRENCYFIELD = u"com.sun.star.form.component.DatabaseCurrencyField";
inline constexpr OUStringLiteral FRM_SUN_COMPONENT_DATABASE_PATTERNFIELD = u"com.sun.star.form.component.DatabasePatternField";

inline constexpr OUStringLiteral FRM_SUN_CONTROL_TEXTFIELD = u"com.sun.star.form.control.TextField";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_LISTBOX = u"com.sun.star.form.control.ListBox";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_COMBOBOX = u"com.sun.star.form.control.ComboBox";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_RADIOBUTTON = u"com.sun.star.form.control.RadioButton";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_GROUPBOX = u"com.sun.star.form.control.GroupBox";
#define FRM_SUN_CONTROL_FIXEDTEXT "com.sun.star.form.control.FixedText"
inline constexpr OUStringLiteral FRM_SUN_CONTROL_COMMANDBUTTON = u"com.sun.star.form.control.CommandButton";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_CHECKBOX = u"com.sun.star.form.control.CheckBox";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_GRIDCONTROL = u"com.sun.star.form.control.GridControl";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_IMAGEBUTTON = u"com.sun.star.form.control.ImageButton";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_TIMEFIELD = u"com.sun.star.form.control.TimeField";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_DATEFIELD = u"com.sun.star.form.control.DateField";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_NUMERICFIELD = u"com.sun.star.form.control.NumericField";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_CURRENCYFIELD = u"com.sun.star.form.control.CurrencyField";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_PATTERNFIELD = u"com.sun.star.form.control.PatternField";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_IMAGECONTROL = u"com.sun.star.form.control.ImageControl";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_FORMATTEDFIELD = u"com.sun.star.form.control.FormattedField";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_RICHTEXTCONTROL = u"com.sun.star.form.control.RichTextControl";
inline constexpr OUStringLiteral FRM_SUN_CONTROL_SUBMITBUTTON = u"com.sun.star.form.control.SubmitButton";

inline constexpr OUStringLiteral BINDABLE_DATABASE_CHECK_BOX = u"com.sun.star.form.binding.BindableDatabaseCheckBox";
inline constexpr OUStringLiteral BINDABLE_DATABASE_COMBO_BOX = u"com.sun.star.form.binding.BindableDatabaseComboBox";
inline constexpr OUStringLiteral BINDABLE_DATABASE_FORMATTED_FIELD = u"com.sun.star.form.binding.BindableDatabaseFormattedField";
inline constexpr OUStringLiteral BINDABLE_DATABASE_LIST_BOX = u"com.sun.star.form.binding.BindableDatabaseListBox";
inline constexpr OUStringLiteral BINDABLE_DATABASE_NUMERIC_FIELD = u"com.sun.star.form.binding.BindableDatabaseNumericField";
inline constexpr OUStringLiteral BINDABLE_DATABASE_RADIO_BUTTON = u"com.sun.star.form.binding.BindableDatabaseRadioButton";
inline constexpr OUStringLiteral BINDABLE_DATABASE_TEXT_FIELD = u"com.sun.star.form.binding.BindableDatabaseTextField";
inline constexpr OUStringLiteral BINDABLE_DATABASE_DATE_FIELD = u"com.sun.star.form.binding.BindableDatabaseDateField";
inline constexpr OUStringLiteral BINDABLE_DATABASE_TIME_FIELD = u"com.sun.star.form.binding.BindableDatabaseTimeField";

inline constexpr OUStringLiteral BINDABLE_CONTROL_MODEL = u"com.sun.star.form.binding.BindableControlModel";
inline constexpr OUStringLiteral BINDABLE_INTEGER_VALUE_RANGE = u"com.sun.star.form.binding.BindableIntegerValueRange";
inline constexpr OUStringLiteral BINDABLE_DATA_AWARE_CONTROL_MODEL = u"com.sun.star.form.binding.BindableDataAwareControlModel";
inline constexpr OUStringLiteral DATA_AWARE_CONTROL_MODEL = u"com.sun.star.form.binding.DataAwareControlModel";
inline constexpr OUStringLiteral VALIDATABLE_CONTROL_MODEL = u"com.sun.star.form.binding.ValidatableControlModel";
inline constexpr OUStringLiteral VALIDATABLE_BINDABLE_CONTROL_MODEL = u"com.sun.star.form.binding.ValidatableBindableControlModel";

// common

inline constexpr OUStringLiteral FRM_SUN_FORMCOMPONENT = u"com.sun.star.form.FormComponent";

// misc

#define SRV_AWT_IMAGEPRODUCER "com.sun.star.awt.ImageProducer"

inline constexpr OUStringLiteral SRV_SDB_ROWSET = u"com.sun.star.sdb.RowSet";
inline constexpr OUStringLiteral SRV_SDB_CONNECTION = u"com.sun.star.sdb.Connection";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
