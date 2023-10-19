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

inline constexpr OUString VCL_CONTROL_LISTBOX = u"stardiv.vcl.control.ListBox"_ustr;
inline constexpr OUString VCL_CONTROL_COMBOBOX = u"stardiv.vcl.control.ComboBox"_ustr;
inline constexpr OUString VCL_CONTROL_RADIOBUTTON = u"stardiv.vcl.control.RadioButton"_ustr;
inline constexpr OUString VCL_CONTROL_GROUPBOX = u"stardiv.vcl.control.GroupBox"_ustr;
inline constexpr OUString VCL_CONTROL_COMMANDBUTTON = u"stardiv.vcl.control.Button"_ustr;
inline constexpr OUString VCL_CONTROL_CHECKBOX = u"stardiv.vcl.control.CheckBox"_ustr;
inline constexpr OUString VCL_CONTROL_IMAGEBUTTON = u"stardiv.vcl.control.ImageButton"_ustr;
inline constexpr OUString VCL_CONTROL_TIMEFIELD = u"stardiv.vcl.control.TimeField"_ustr;
inline constexpr OUString VCL_CONTROL_DATEFIELD = u"stardiv.vcl.control.DateField"_ustr;
inline constexpr OUString VCL_CONTROL_NUMERICFIELD = u"stardiv.vcl.control.NumericField"_ustr;
inline constexpr OUString VCL_CONTROL_CURRENCYFIELD = u"stardiv.vcl.control.CurrencyField"_ustr;
inline constexpr OUString VCL_CONTROL_PATTERNFIELD = u"stardiv.vcl.control.PatternField"_ustr;
inline constexpr OUString VCL_CONTROL_FORMATTEDFIELD = u"stardiv.vcl.control.FormattedField"_ustr;
inline constexpr OUString VCL_CONTROL_IMAGECONTROL = u"stardiv.vcl.control.ImageControl"_ustr;

inline constexpr OUString VCL_CONTROLMODEL_EDIT = u"stardiv.vcl.controlmodel.Edit"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_LISTBOX = u"stardiv.vcl.controlmodel.ListBox"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_COMBOBOX = u"stardiv.vcl.controlmodel.ComboBox"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_RADIOBUTTON = u"stardiv.vcl.controlmodel.RadioButton"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_GROUPBOX = u"stardiv.vcl.controlmodel.GroupBox"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_FIXEDTEXT = u"stardiv.vcl.controlmodel.FixedText"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_COMMANDBUTTON = u"stardiv.vcl.controlmodel.Button"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_CHECKBOX = u"stardiv.vcl.controlmodel.CheckBox"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_IMAGEBUTTON = u"stardiv.vcl.controlmodel.ImageButton"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_FILECONTROL = u"stardiv.vcl.controlmodel.FileControl"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_TIMEFIELD = u"stardiv.vcl.controlmodel.TimeField"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_DATEFIELD = u"stardiv.vcl.controlmodel.DateField"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_NUMERICFIELD = u"stardiv.vcl.controlmodel.NumericField"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_CURRENCYFIELD = u"stardiv.vcl.controlmodel.CurrencyField"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_PATTERNFIELD = u"stardiv.vcl.controlmodel.PatternField"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_FORMATTEDFIELD = u"stardiv.vcl.controlmodel.FormattedField"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_IMAGECONTROL = u"stardiv.vcl.controlmodel.ImageControl"_ustr;

inline constexpr OUString VCL_CONTROLMODEL_SCROLLBAR = u"com.sun.star.awt.UnoControlScrollBarModel"_ustr;
inline constexpr OUString VCL_CONTROL_SCROLLBAR = u"com.sun.star.awt.UnoControlScrollBar"_ustr;
inline constexpr OUString VCL_CONTROLMODEL_SPINBUTTON = u"com.sun.star.awt.UnoControlSpinButtonModel"_ustr;
inline constexpr OUString VCL_CONTROL_SPINBUTTON = u"com.sun.star.awt.UnoControlSpinButton"_ustr;

// service names for compatibility

inline constexpr OUString FRM_COMPONENT_FORM = u"stardiv.one.form.component.Form"_ustr;
inline constexpr OUString FRM_COMPONENT_EDIT = u"stardiv.one.form.component.Edit"_ustr; // compatibility
inline constexpr OUString FRM_COMPONENT_TEXTFIELD = u"stardiv.one.form.component.TextField"_ustr;
inline constexpr OUString FRM_COMPONENT_LISTBOX = u"stardiv.one.form.component.ListBox"_ustr;
inline constexpr OUString FRM_COMPONENT_COMBOBOX = u"stardiv.one.form.component.ComboBox"_ustr;
inline constexpr OUString FRM_COMPONENT_RADIOBUTTON = u"stardiv.one.form.component.RadioButton"_ustr;
inline constexpr OUString FRM_COMPONENT_GROUPBOX = u"stardiv.one.form.component.GroupBox"_ustr; // compatibility
inline constexpr OUString FRM_COMPONENT_FIXEDTEXT = u"stardiv.one.form.component.FixedText"_ustr; // compatibility
inline constexpr OUString FRM_COMPONENT_COMMANDBUTTON = u"stardiv.one.form.component.CommandButton"_ustr;
inline constexpr OUString FRM_COMPONENT_CHECKBOX = u"stardiv.one.form.component.CheckBox"_ustr;
inline constexpr OUString FRM_COMPONENT_GRID = u"stardiv.one.form.component.Grid"_ustr; // compatibility
inline constexpr OUString FRM_COMPONENT_GRIDCONTROL = u"stardiv.one.form.component.GridControl"_ustr;
inline constexpr OUString FRM_COMPONENT_IMAGEBUTTON = u"stardiv.one.form.component.ImageButton"_ustr;
inline constexpr OUString FRM_COMPONENT_FILECONTROL = u"stardiv.one.form.component.FileControl"_ustr;
inline constexpr OUString FRM_COMPONENT_TIMEFIELD = u"stardiv.one.form.component.TimeField"_ustr;
inline constexpr OUString FRM_COMPONENT_DATEFIELD = u"stardiv.one.form.component.DateField"_ustr;
inline constexpr OUString FRM_COMPONENT_NUMERICFIELD = u"stardiv.one.form.component.NumericField"_ustr;
inline constexpr OUString FRM_COMPONENT_CURRENCYFIELD = u"stardiv.one.form.component.CurrencyField"_ustr;
inline constexpr OUString FRM_COMPONENT_PATTERNFIELD = u"stardiv.one.form.component.PatternField"_ustr;
inline constexpr OUString FRM_COMPONENT_HIDDEN = u"stardiv.one.form.component.Hidden"_ustr;
inline constexpr OUString FRM_COMPONENT_HIDDENCONTROL = u"stardiv.one.form.component.HiddenControl"_ustr;
inline constexpr OUString FRM_COMPONENT_IMAGECONTROL = u"stardiv.one.form.component.ImageControl"_ustr;
inline constexpr OUString FRM_COMPONENT_FORMATTEDFIELD = u"stardiv.one.form.component.FormattedField"_ustr;

// <compatibility_I>
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_COMMANDBUTTON = u"stardiv.one.form.control.CommandButton"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_RADIOBUTTON = u"stardiv.one.form.control.RadioButton"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_CHECKBOX = u"stardiv.one.form.control.CheckBox"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_EDIT = u"stardiv.one.form.control.Edit"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_LISTBOX = u"stardiv.one.form.control.ListBox"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_COMBOBOX = u"stardiv.one.form.control.ComboBox"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_GROUPBOX = u"stardiv.one.form.control.GroupBox"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_TEXTFIELD = u"stardiv.one.form.control.TextField"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_GRID = u"stardiv.one.form.control.Grid"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_IMAGEBUTTON = u"stardiv.one.form.control.ImageButton"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_TIMEFIELD = u"stardiv.one.form.control.TimeField"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_DATEFIELD = u"stardiv.one.form.control.DateField"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_NUMERICFIELD = u"stardiv.one.form.control.NumericField"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_CURRENCYFIELD = u"stardiv.one.form.control.CurrencyField"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_PATTERNFIELD = u"stardiv.one.form.control.PatternField"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_IMAGECONTROL = u"stardiv.one.form.control.ImageControl"_ustr;
inline constexpr OUString STARDIV_ONE_FORM_CONTROL_FORMATTEDFIELD = u"stardiv.one.form.control.FormattedField"_ustr;
// </compatibility_I>

// new (sun) service names

inline constexpr OUString FRM_SUN_COMPONENT_FORM = u"com.sun.star.form.component.Form"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_HTMLFORM = u"com.sun.star.form.component.HTMLForm"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATAFORM = u"com.sun.star.form.component.DataForm"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_TEXTFIELD = u"com.sun.star.form.component.TextField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_LISTBOX = u"com.sun.star.form.component.ListBox"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_COMBOBOX = u"com.sun.star.form.component.ComboBox"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_RADIOBUTTON = u"com.sun.star.form.component.RadioButton"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_GROUPBOX = u"com.sun.star.form.component.GroupBox"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_FIXEDTEXT = u"com.sun.star.form.component.FixedText"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_COMMANDBUTTON = u"com.sun.star.form.component.CommandButton"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_CHECKBOX = u"com.sun.star.form.component.CheckBox"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_GRIDCONTROL = u"com.sun.star.form.component.GridControl"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_IMAGEBUTTON = u"com.sun.star.form.component.ImageButton"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_FILECONTROL = u"com.sun.star.form.component.FileControl"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_TIMEFIELD = u"com.sun.star.form.component.TimeField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATEFIELD = u"com.sun.star.form.component.DateField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_NUMERICFIELD = u"com.sun.star.form.component.NumericField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_CURRENCYFIELD = u"com.sun.star.form.component.CurrencyField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_PATTERNFIELD = u"com.sun.star.form.component.PatternField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_HIDDENCONTROL = u"com.sun.star.form.component.HiddenControl"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_FORMATTEDFIELD = u"com.sun.star.form.component.FormattedField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_SCROLLBAR = u"com.sun.star.form.component.ScrollBar"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_SPINBUTTON = u"com.sun.star.form.component.SpinButton"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_RICHTEXTCONTROL = u"com.sun.star.form.component.RichTextControl"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_NAVTOOLBAR = u"com.sun.star.form.component.NavigationToolBar"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_SUBMITBUTTON = u"com.sun.star.form.component.SubmitButton"_ustr;

inline constexpr OUString FRM_SUN_COMPONENT_IMAGECONTROL = u"com.sun.star.form.component.DatabaseImageControl"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATABASE_RADIOBUTTON = u"com.sun.star.form.component.DatabaseRadioButton"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATABASE_CHECKBOX = u"com.sun.star.form.component.DatabaseCheckBox"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATABASE_LISTBOX = u"com.sun.star.form.component.DatabaseListBox"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATABASE_COMBOBOX = u"com.sun.star.form.component.DatabaseComboBox"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATABASE_FORMATTEDFIELD = u"com.sun.star.form.component.DatabaseFormattedField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATABASE_TEXTFIELD = u"com.sun.star.form.component.DatabaseTextField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATABASE_DATEFIELD = u"com.sun.star.form.component.DatabaseDateField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATABASE_TIMEFIELD = u"com.sun.star.form.component.DatabaseTimeField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATABASE_NUMERICFIELD = u"com.sun.star.form.component.DatabaseNumericField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATABASE_CURRENCYFIELD = u"com.sun.star.form.component.DatabaseCurrencyField"_ustr;
inline constexpr OUString FRM_SUN_COMPONENT_DATABASE_PATTERNFIELD = u"com.sun.star.form.component.DatabasePatternField"_ustr;

inline constexpr OUString FRM_SUN_CONTROL_TEXTFIELD = u"com.sun.star.form.control.TextField"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_LISTBOX = u"com.sun.star.form.control.ListBox"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_COMBOBOX = u"com.sun.star.form.control.ComboBox"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_RADIOBUTTON = u"com.sun.star.form.control.RadioButton"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_GROUPBOX = u"com.sun.star.form.control.GroupBox"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_COMMANDBUTTON = u"com.sun.star.form.control.CommandButton"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_CHECKBOX = u"com.sun.star.form.control.CheckBox"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_GRIDCONTROL = u"com.sun.star.form.control.GridControl"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_IMAGEBUTTON = u"com.sun.star.form.control.ImageButton"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_TIMEFIELD = u"com.sun.star.form.control.TimeField"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_DATEFIELD = u"com.sun.star.form.control.DateField"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_NUMERICFIELD = u"com.sun.star.form.control.NumericField"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_CURRENCYFIELD = u"com.sun.star.form.control.CurrencyField"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_PATTERNFIELD = u"com.sun.star.form.control.PatternField"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_IMAGECONTROL = u"com.sun.star.form.control.ImageControl"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_FORMATTEDFIELD = u"com.sun.star.form.control.FormattedField"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_RICHTEXTCONTROL = u"com.sun.star.form.control.RichTextControl"_ustr;
inline constexpr OUString FRM_SUN_CONTROL_SUBMITBUTTON = u"com.sun.star.form.control.SubmitButton"_ustr;

inline constexpr OUString BINDABLE_DATABASE_CHECK_BOX = u"com.sun.star.form.binding.BindableDatabaseCheckBox"_ustr;
inline constexpr OUString BINDABLE_DATABASE_COMBO_BOX = u"com.sun.star.form.binding.BindableDatabaseComboBox"_ustr;
inline constexpr OUString BINDABLE_DATABASE_FORMATTED_FIELD = u"com.sun.star.form.binding.BindableDatabaseFormattedField"_ustr;
inline constexpr OUString BINDABLE_DATABASE_LIST_BOX = u"com.sun.star.form.binding.BindableDatabaseListBox"_ustr;
inline constexpr OUString BINDABLE_DATABASE_NUMERIC_FIELD = u"com.sun.star.form.binding.BindableDatabaseNumericField"_ustr;
inline constexpr OUString BINDABLE_DATABASE_RADIO_BUTTON = u"com.sun.star.form.binding.BindableDatabaseRadioButton"_ustr;
inline constexpr OUString BINDABLE_DATABASE_TEXT_FIELD = u"com.sun.star.form.binding.BindableDatabaseTextField"_ustr;
inline constexpr OUString BINDABLE_DATABASE_DATE_FIELD = u"com.sun.star.form.binding.BindableDatabaseDateField"_ustr;
inline constexpr OUString BINDABLE_DATABASE_TIME_FIELD = u"com.sun.star.form.binding.BindableDatabaseTimeField"_ustr;

inline constexpr OUString BINDABLE_CONTROL_MODEL = u"com.sun.star.form.binding.BindableControlModel"_ustr;
inline constexpr OUString BINDABLE_INTEGER_VALUE_RANGE = u"com.sun.star.form.binding.BindableIntegerValueRange"_ustr;
inline constexpr OUString BINDABLE_DATA_AWARE_CONTROL_MODEL = u"com.sun.star.form.binding.BindableDataAwareControlModel"_ustr;
inline constexpr OUString DATA_AWARE_CONTROL_MODEL = u"com.sun.star.form.binding.DataAwareControlModel"_ustr;
inline constexpr OUString VALIDATABLE_CONTROL_MODEL = u"com.sun.star.form.binding.ValidatableControlModel"_ustr;
inline constexpr OUString VALIDATABLE_BINDABLE_CONTROL_MODEL = u"com.sun.star.form.binding.ValidatableBindableControlModel"_ustr;

// common

inline constexpr OUString FRM_SUN_FORMCOMPONENT = u"com.sun.star.form.FormComponent"_ustr;

// misc

inline constexpr OUString SRV_SDB_ROWSET = u"com.sun.star.sdb.RowSet"_ustr;
inline constexpr OUString SRV_SDB_CONNECTION = u"com.sun.star.sdb.Connection"_ustr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
