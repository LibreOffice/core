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
#define VCL_CONTROL_LISTBOX "stardiv.vcl.control.ListBox"
#define VCL_CONTROL_COMBOBOX "stardiv.vcl.control.ComboBox"
#define VCL_CONTROL_RADIOBUTTON "stardiv.vcl.control.RadioButton"
#define VCL_CONTROL_GROUPBOX "stardiv.vcl.control.GroupBox"
#define VCL_CONTROL_FIXEDTEXT "stardiv.vcl.control.FixedText"
#define VCL_CONTROL_COMMANDBUTTON "stardiv.vcl.control.Button"
#define VCL_CONTROL_CHECKBOX "stardiv.vcl.control.CheckBox"
#define VCL_CONTROL_IMAGEBUTTON "stardiv.vcl.control.ImageButton"
#define VCL_CONTROL_FILECONTROL "stardiv.vcl.control.FileControl"
#define VCL_CONTROL_TIMEFIELD "stardiv.vcl.control.TimeField"
#define VCL_CONTROL_DATEFIELD "stardiv.vcl.control.DateField"
#define VCL_CONTROL_NUMERICFIELD "stardiv.vcl.control.NumericField"
#define VCL_CONTROL_CURRENCYFIELD "stardiv.vcl.control.CurrencyField"
#define VCL_CONTROL_PATTERNFIELD "stardiv.vcl.control.PatternField"
#define VCL_CONTROL_FORMATTEDFIELD "stardiv.vcl.control.FormattedField"
#define VCL_CONTROL_IMAGECONTROL "stardiv.vcl.control.ImageControl"

#define VCL_CONTROLMODEL_EDIT             "stardiv.vcl.controlmodel.Edit"
#define VCL_CONTROLMODEL_LISTBOX          "stardiv.vcl.controlmodel.ListBox"
#define VCL_CONTROLMODEL_COMBOBOX         "stardiv.vcl.controlmodel.ComboBox"
#define VCL_CONTROLMODEL_RADIOBUTTON      "stardiv.vcl.controlmodel.RadioButton"
#define VCL_CONTROLMODEL_GROUPBOX         "stardiv.vcl.controlmodel.GroupBox"
#define VCL_CONTROLMODEL_FIXEDTEXT        "stardiv.vcl.controlmodel.FixedText"
#define VCL_CONTROLMODEL_COMMANDBUTTON    "stardiv.vcl.controlmodel.Button"
#define VCL_CONTROLMODEL_CHECKBOX         "stardiv.vcl.controlmodel.CheckBox"
#define VCL_CONTROLMODEL_IMAGEBUTTON      "stardiv.vcl.controlmodel.ImageButton"
#define VCL_CONTROLMODEL_FILECONTROL      "stardiv.vcl.controlmodel.FileControl"
#define VCL_CONTROLMODEL_TIMEFIELD        "stardiv.vcl.controlmodel.TimeField"
#define VCL_CONTROLMODEL_DATEFIELD        "stardiv.vcl.controlmodel.DateField"
#define VCL_CONTROLMODEL_NUMERICFIELD     "stardiv.vcl.controlmodel.NumericField"
#define VCL_CONTROLMODEL_CURRENCYFIELD    "stardiv.vcl.controlmodel.CurrencyField"
#define VCL_CONTROLMODEL_PATTERNFIELD     "stardiv.vcl.controlmodel.PatternField"
#define VCL_CONTROLMODEL_FORMATTEDFIELD   "stardiv.vcl.controlmodel.FormattedField"
#define VCL_CONTROLMODEL_IMAGECONTROL     "stardiv.vcl.controlmodel.ImageControl"

#define VCL_CONTROLMODEL_SCROLLBAR        "com.sun.star.awt.UnoControlScrollBarModel"
#define VCL_CONTROL_SCROLLBAR             "com.sun.star.awt.UnoControlScrollBar"
#define VCL_CONTROLMODEL_SPINBUTTON       "com.sun.star.awt.UnoControlSpinButtonModel"
#define VCL_CONTROL_SPINBUTTON            "com.sun.star.awt.UnoControlSpinButton"

// service names for compatibility

#define FRM_COMPONENT_FORM "stardiv.one.form.component.Form"
#define FRM_COMPONENT_EDIT "stardiv.one.form.component.Edit"           // compatibility
#define FRM_COMPONENT_TEXTFIELD "stardiv.one.form.component.TextField"
#define FRM_COMPONENT_LISTBOX "stardiv.one.form.component.ListBox"
#define FRM_COMPONENT_COMBOBOX "stardiv.one.form.component.ComboBox"
#define FRM_COMPONENT_RADIOBUTTON "stardiv.one.form.component.RadioButton"
#define FRM_COMPONENT_GROUPBOX "stardiv.one.form.component.GroupBox"       // compatibility
#define FRM_COMPONENT_FIXEDTEXT "stardiv.one.form.component.FixedText"     // compatibility
#define FRM_COMPONENT_COMMANDBUTTON "stardiv.one.form.component.CommandButton"
#define FRM_COMPONENT_CHECKBOX "stardiv.one.form.component.CheckBox"
#define FRM_COMPONENT_GRID "stardiv.one.form.component.Grid"           // compatibility
#define FRM_COMPONENT_GRIDCONTROL "stardiv.one.form.component.GridControl"
#define FRM_COMPONENT_IMAGEBUTTON "stardiv.one.form.component.ImageButton"
#define FRM_COMPONENT_FILECONTROL "stardiv.one.form.component.FileControl"
#define FRM_COMPONENT_TIMEFIELD "stardiv.one.form.component.TimeField"
#define FRM_COMPONENT_DATEFIELD "stardiv.one.form.component.DateField"
#define FRM_COMPONENT_NUMERICFIELD "stardiv.one.form.component.NumericField"
#define FRM_COMPONENT_CURRENCYFIELD "stardiv.one.form.component.CurrencyField"
#define FRM_COMPONENT_PATTERNFIELD "stardiv.one.form.component.PatternField"
#define FRM_COMPONENT_HIDDEN "stardiv.one.form.component.Hidden"
#define FRM_COMPONENT_HIDDENCONTROL "stardiv.one.form.component.HiddenControl"
#define FRM_COMPONENT_IMAGECONTROL "stardiv.one.form.component.ImageControl"
#define FRM_COMPONENT_FORMATTEDFIELD "stardiv.one.form.component.FormattedField"

// <compatibility_I>
#define STARDIV_ONE_FORM_CONTROL_COMMANDBUTTON   "stardiv.one.form.control.CommandButton"
#define STARDIV_ONE_FORM_CONTROL_RADIOBUTTON     "stardiv.one.form.control.RadioButton"
#define STARDIV_ONE_FORM_CONTROL_CHECKBOX        "stardiv.one.form.control.CheckBox"
#define STARDIV_ONE_FORM_CONTROL_EDIT            "stardiv.one.form.control.Edit"
#define STARDIV_ONE_FORM_CONTROL_LISTBOX         "stardiv.one.form.control.ListBox"
#define STARDIV_ONE_FORM_CONTROL_COMBOBOX        "stardiv.one.form.control.ComboBox"
#define STARDIV_ONE_FORM_CONTROL_GROUPBOX        "stardiv.one.form.control.GroupBox"
#define STARDIV_ONE_FORM_CONTROL_TEXTFIELD       "stardiv.one.form.control.TextField"
#define STARDIV_ONE_FORM_CONTROL_GRID            "stardiv.one.form.control.Grid"
#define STARDIV_ONE_FORM_CONTROL_IMAGEBUTTON     "stardiv.one.form.control.ImageButton"
#define STARDIV_ONE_FORM_CONTROL_TIMEFIELD       "stardiv.one.form.control.TimeField"
#define STARDIV_ONE_FORM_CONTROL_DATEFIELD       "stardiv.one.form.control.DateField"
#define STARDIV_ONE_FORM_CONTROL_NUMERICFIELD    "stardiv.one.form.control.NumericField"
#define STARDIV_ONE_FORM_CONTROL_CURRENCYFIELD   "stardiv.one.form.control.CurrencyField"
#define STARDIV_ONE_FORM_CONTROL_PATTERNFIELD    "stardiv.one.form.control.PatternField"
#define STARDIV_ONE_FORM_CONTROL_IMAGECONTROL    "stardiv.one.form.control.ImageControl"
#define STARDIV_ONE_FORM_CONTROL_FORMATTEDFIELD  "stardiv.one.form.control.FormattedField"
// </compatibility_I>

// new (sun) service names

#define FRM_SUN_COMPONENT_FORM            "com.sun.star.form.component.Form"
#define FRM_SUN_COMPONENT_HTMLFORM        "com.sun.star.form.component.HTMLForm"
#define FRM_SUN_COMPONENT_DATAFORM        "com.sun.star.form.component.DataForm"
#define FRM_SUN_COMPONENT_TEXTFIELD       "com.sun.star.form.component.TextField"
#define FRM_SUN_COMPONENT_LISTBOX         "com.sun.star.form.component.ListBox"
#define FRM_SUN_COMPONENT_COMBOBOX        "com.sun.star.form.component.ComboBox"
#define FRM_SUN_COMPONENT_RADIOBUTTON     "com.sun.star.form.component.RadioButton"
#define FRM_SUN_COMPONENT_GROUPBOX        "com.sun.star.form.component.GroupBox"
#define FRM_SUN_COMPONENT_FIXEDTEXT       "com.sun.star.form.component.FixedText"
#define FRM_SUN_COMPONENT_COMMANDBUTTON   "com.sun.star.form.component.CommandButton"
#define FRM_SUN_COMPONENT_CHECKBOX        "com.sun.star.form.component.CheckBox"
#define FRM_SUN_COMPONENT_GRIDCONTROL     "com.sun.star.form.component.GridControl"
#define FRM_SUN_COMPONENT_IMAGEBUTTON     "com.sun.star.form.component.ImageButton"
#define FRM_SUN_COMPONENT_FILECONTROL     "com.sun.star.form.component.FileControl"
#define FRM_SUN_COMPONENT_TIMEFIELD       "com.sun.star.form.component.TimeField"
#define FRM_SUN_COMPONENT_DATEFIELD       "com.sun.star.form.component.DateField"
#define FRM_SUN_COMPONENT_NUMERICFIELD    "com.sun.star.form.component.NumericField"
#define FRM_SUN_COMPONENT_CURRENCYFIELD   "com.sun.star.form.component.CurrencyField"
#define FRM_SUN_COMPONENT_PATTERNFIELD    "com.sun.star.form.component.PatternField"
#define FRM_SUN_COMPONENT_HIDDENCONTROL   "com.sun.star.form.component.HiddenControl"
#define FRM_SUN_COMPONENT_FORMATTEDFIELD  "com.sun.star.form.component.FormattedField"
#define FRM_SUN_COMPONENT_SCROLLBAR       "com.sun.star.form.component.ScrollBar"
#define FRM_SUN_COMPONENT_SPINBUTTON      "com.sun.star.form.component.SpinButton"
#define FRM_SUN_COMPONENT_RICHTEXTCONTROL "com.sun.star.form.component.RichTextControl"
#define FRM_SUN_COMPONENT_NAVTOOLBAR      "com.sun.star.form.component.NavigationToolBar"
#define FRM_SUN_COMPONENT_SUBMITBUTTON    "com.sun.star.form.component.SubmitButton"

#define FRM_SUN_COMPONENT_IMAGECONTROL "com.sun.star.form.component.DatabaseImageControl"
#define FRM_SUN_COMPONENT_DATABASE_RADIOBUTTON "com.sun.star.form.component.DatabaseRadioButton"
#define FRM_SUN_COMPONENT_DATABASE_CHECKBOX "com.sun.star.form.component.DatabaseCheckBox"
#define FRM_SUN_COMPONENT_DATABASE_LISTBOX "com.sun.star.form.component.DatabaseListBox"
#define FRM_SUN_COMPONENT_DATABASE_COMBOBOX "com.sun.star.form.component.DatabaseComboBox"
#define FRM_SUN_COMPONENT_DATABASE_FORMATTEDFIELD "com.sun.star.form.component.DatabaseFormattedField"
#define FRM_SUN_COMPONENT_DATABASE_TEXTFIELD "com.sun.star.form.component.DatabaseTextField"
#define FRM_SUN_COMPONENT_DATABASE_DATEFIELD "com.sun.star.form.component.DatabaseDateField"
#define FRM_SUN_COMPONENT_DATABASE_TIMEFIELD "com.sun.star.form.component.DatabaseTimeField"
#define FRM_SUN_COMPONENT_DATABASE_NUMERICFIELD "com.sun.star.form.component.DatabaseNumericField"
#define FRM_SUN_COMPONENT_DATABASE_CURRENCYFIELD "com.sun.star.form.component.DatabaseCurrencyField"
#define FRM_SUN_COMPONENT_DATABASE_PATTERNFIELD "com.sun.star.form.component.DatabasePatternField"

#define FRM_SUN_CONTROL_TEXTFIELD "com.sun.star.form.control.TextField"
#define FRM_SUN_CONTROL_LISTBOX "com.sun.star.form.control.ListBox"
#define FRM_SUN_CONTROL_COMBOBOX "com.sun.star.form.control.ComboBox"
#define FRM_SUN_CONTROL_RADIOBUTTON "com.sun.star.form.control.RadioButton"
#define FRM_SUN_CONTROL_GROUPBOX "com.sun.star.form.control.GroupBox"
#define FRM_SUN_CONTROL_FIXEDTEXT "com.sun.star.form.control.FixedText"
#define FRM_SUN_CONTROL_COMMANDBUTTON "com.sun.star.form.control.CommandButton"
#define FRM_SUN_CONTROL_CHECKBOX "com.sun.star.form.control.CheckBox"
#define FRM_SUN_CONTROL_GRIDCONTROL "com.sun.star.form.control.GridControl"
#define FRM_SUN_CONTROL_IMAGEBUTTON "com.sun.star.form.control.ImageButton"
#define FRM_SUN_CONTROL_TIMEFIELD "com.sun.star.form.control.TimeField"
#define FRM_SUN_CONTROL_DATEFIELD "com.sun.star.form.control.DateField"
#define FRM_SUN_CONTROL_NUMERICFIELD "com.sun.star.form.control.NumericField"
#define FRM_SUN_CONTROL_CURRENCYFIELD "com.sun.star.form.control.CurrencyField"
#define FRM_SUN_CONTROL_PATTERNFIELD "com.sun.star.form.control.PatternField"
#define FRM_SUN_CONTROL_IMAGECONTROL "com.sun.star.form.control.ImageControl"
#define FRM_SUN_CONTROL_FORMATTEDFIELD "com.sun.star.form.control.FormattedField"
#define FRM_SUN_CONTROL_RICHTEXTCONTROL "com.sun.star.form.control.RichTextControl"
#define FRM_SUN_CONTROL_SUBMITBUTTON   "com.sun.star.form.control.SubmitButton"

#define BINDABLE_DATABASE_CHECK_BOX      "com.sun.star.form.binding.BindableDatabaseCheckBox"
#define BINDABLE_DATABASE_COMBO_BOX      "com.sun.star.form.binding.BindableDatabaseComboBox"
#define BINDABLE_DATABASE_FORMATTED_FIELD "com.sun.star.form.binding.BindableDatabaseFormattedField"
#define BINDABLE_DATABASE_LIST_BOX       "com.sun.star.form.binding.BindableDatabaseListBox"
#define BINDABLE_DATABASE_NUMERIC_FIELD  "com.sun.star.form.binding.BindableDatabaseNumericField"
#define BINDABLE_DATABASE_RADIO_BUTTON   "com.sun.star.form.binding.BindableDatabaseRadioButton"
#define BINDABLE_DATABASE_TEXT_FIELD     "com.sun.star.form.binding.BindableDatabaseTextField"
#define BINDABLE_DATABASE_DATE_FIELD     "com.sun.star.form.binding.BindableDatabaseDateField"
#define BINDABLE_DATABASE_TIME_FIELD     "com.sun.star.form.binding.BindableDatabaseTimeField"

#define BINDABLE_CONTROL_MODEL            "com.sun.star.form.binding.BindableControlModel"
#define BINDABLE_INTEGER_VALUE_RANGE      "com.sun.star.form.binding.BindableIntegerValueRange"
#define BINDABLE_DATA_AWARE_CONTROL_MODEL "com.sun.star.form.binding.BindableDataAwareControlModel"
#define DATA_AWARE_CONTROL_MODEL          "com.sun.star.form.binding.DataAwareControlModel"
#define VALIDATABLE_CONTROL_MODEL         "com.sun.star.form.binding.ValidatableControlModel"
#define VALIDATABLE_BINDABLE_CONTROL_MODEL "com.sun.star.form.binding.ValidatableBindableControlModel"

// common

#define FRM_SUN_FORMCOMPONENT "com.sun.star.form.FormComponent"

// misc

#define SRV_AWT_IMAGEPRODUCER "com.sun.star.awt.ImageProducer"

#define SRV_SDB_ROWSET "com.sun.star.sdb.RowSet"
#define SRV_SDB_CONNECTION "com.sun.star.sdb.Connection"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
