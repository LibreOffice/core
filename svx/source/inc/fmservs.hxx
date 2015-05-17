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
#ifndef INCLUDED_SVX_SOURCE_INC_FMSERVS_HXX
#define INCLUDED_SVX_SOURCE_INC_FMSERVS_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <svx/svxdllapi.h>

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

#define FM_COMPONENT_EDIT               "stardiv.one.form.component.Edit"
#define FM_COMPONENT_TEXTFIELD          "stardiv.one.form.component.TextField"
#define FM_COMPONENT_LISTBOX            "stardiv.one.form.component.ListBox"
#define FM_COMPONENT_COMBOBOX           "stardiv.one.form.component.ComboBox"
#define FM_COMPONENT_RADIOBUTTON        "stardiv.one.form.component.RadioButton"
#define FM_COMPONENT_GROUPBOX           "stardiv.one.form.component.GroupBox"
#define FM_COMPONENT_FIXEDTEXT          "stardiv.one.form.component.FixedText"
#define FM_COMPONENT_COMMANDBUTTON      "stardiv.one.form.component.CommandButton"
#define FM_COMPONENT_CHECKBOX           "stardiv.one.form.component.CheckBox"
#define FM_COMPONENT_GRID               "stardiv.one.form.component.Grid"
#define FM_COMPONENT_GRIDCONTROL        "stardiv.one.form.component.GridControl"
#define FM_COMPONENT_IMAGEBUTTON        "stardiv.one.form.component.ImageButton"
#define FM_COMPONENT_FILECONTROL        "stardiv.one.form.component.FileControl"
#define FM_COMPONENT_TIMEFIELD          "stardiv.one.form.component.TimeField"
#define FM_COMPONENT_DATEFIELD          "stardiv.one.form.component.DateField"
#define FM_COMPONENT_NUMERICFIELD       "stardiv.one.form.component.NumericField"
#define FM_COMPONENT_CURRENCYFIELD      "stardiv.one.form.component.CurrencyField"
#define FM_COMPONENT_PATTERNFIELD       "stardiv.one.form.component.PatternField"
#define FM_COMPONENT_FORMATTEDFIELD     "stardiv.one.form.component.FormattedField"
#define FM_COMPONENT_HIDDEN             "stardiv.one.form.component.Hidden"
#define FM_COMPONENT_HIDDENCONTROL      "stardiv.one.form.component.HiddenControl"
#define FM_COMPONENT_IMAGECONTROL       "stardiv.one.form.component.ImageControl"
#define FM_CONTROL_GRID                 "stardiv.one.form.control.Grid"
#define FM_CONTROL_GRIDCONTROL          "stardiv.one.form.control.GridControl"
#define SRV_SDB_CONNECTION              "com.sun.star.sdb.Connection"
#define FM_SUN_COMPONENT_FORM           "com.sun.star.form.component.Form"
#define FM_SUN_COMPONENT_TEXTFIELD      "com.sun.star.form.component.TextField"
#define FM_SUN_COMPONENT_LISTBOX        "com.sun.star.form.component.ListBox"
#define FM_SUN_COMPONENT_COMBOBOX       "com.sun.star.form.component.ComboBox"
#define FM_SUN_COMPONENT_RADIOBUTTON    "com.sun.star.form.component.RadioButton"
#define FM_SUN_COMPONENT_GROUPBOX       "com.sun.star.form.component.GroupBox"
#define FM_SUN_COMPONENT_FIXEDTEXT      "com.sun.star.form.component.FixedText"
#define FM_SUN_COMPONENT_COMMANDBUTTON  "com.sun.star.form.component.CommandButton"
#define FM_SUN_COMPONENT_CHECKBOX       "com.sun.star.form.component.CheckBox"
#define FM_SUN_COMPONENT_GRIDCONTROL    "com.sun.star.form.component.GridControl"
#define FM_SUN_COMPONENT_IMAGEBUTTON    "com.sun.star.form.component.ImageButton"
#define FM_SUN_COMPONENT_FILECONTROL    "com.sun.star.form.component.FileControl"
#define FM_SUN_COMPONENT_TIMEFIELD      "com.sun.star.form.component.TimeField"
#define FM_SUN_COMPONENT_DATEFIELD      "com.sun.star.form.component.DateField"
#define FM_SUN_COMPONENT_NUMERICFIELD   "com.sun.star.form.component.NumericField"
#define FM_SUN_COMPONENT_CURRENCYFIELD  "com.sun.star.form.component.CurrencyField"
#define FM_SUN_COMPONENT_PATTERNFIELD   "com.sun.star.form.component.PatternField"
#define FM_SUN_COMPONENT_HIDDENCONTROL  "com.sun.star.form.component.HiddenControl"
#define FM_SUN_COMPONENT_IMAGECONTROL   "com.sun.star.form.component.DatabaseImageControl"
#define FM_SUN_COMPONENT_FORMATTEDFIELD "com.sun.star.form.component.FormattedField"
#define FM_SUN_COMPONENT_SCROLLBAR      "com.sun.star.form.component.ScrollBar"
#define FM_SUN_COMPONENT_SPINBUTTON     "com.sun.star.form.component.SpinButton"
#define FM_SUN_COMPONENT_NAVIGATIONBAR  "com.sun.star.form.component.NavigationToolBar"
#define FM_SUN_CONTROL_GRIDCONTROL      "com.sun.star.form.control.GridControl"

namespace svxform
{
    SVX_DLLPUBLIC void ImplSmartRegisterUnoServices();

    css::uno::Reference<css::uno::XInterface> SAL_CALL
    OAddConditionDialog_Create(
        css::uno::Reference<css::lang::XMultiServiceFactory> const &);

    OUString SAL_CALL OAddConditionDialog_GetImplementationName();

    css::uno::Sequence<OUString> SAL_CALL
    OAddConditionDialog_GetSupportedServiceNames();
}

css::uno::Reference<css::uno::XInterface> SAL_CALL
FmXGridControl_NewInstance_Impl(
    css::uno::Reference<css::lang::XMultiServiceFactory> const &)
    throw (css::uno::Exception);

css::uno::Reference<css::uno::XInterface> SAL_CALL
FormController_NewInstance_Impl(
    css::uno::Reference<css::lang::XMultiServiceFactory> const &)
    throw (css::uno::Exception);

css::uno::Reference<css::uno::XInterface> SAL_CALL
LegacyFormController_NewInstance_Impl(
    css::uno::Reference<css::lang::XMultiServiceFactory> const &)
    throw (css::uno::Exception);

#endif // INCLUDED_SVX_SOURCE_INC_FMSERVS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
