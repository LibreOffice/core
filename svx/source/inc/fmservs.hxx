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

namespace com::sun::star::lang { class XMultiServiceFactory; }

#define FM_COMPONENT_EDIT               "stardiv.one.form.component.Edit"
constexpr OUStringLiteral FM_COMPONENT_TEXTFIELD = u"stardiv.one.form.component.TextField";
constexpr OUStringLiteral FM_COMPONENT_LISTBOX = u"stardiv.one.form.component.ListBox";
constexpr OUStringLiteral FM_COMPONENT_COMBOBOX = u"stardiv.one.form.component.ComboBox";
constexpr OUStringLiteral FM_COMPONENT_RADIOBUTTON = u"stardiv.one.form.component.RadioButton";
constexpr OUStringLiteral FM_COMPONENT_GROUPBOX = u"stardiv.one.form.component.GroupBox";
constexpr OUStringLiteral FM_COMPONENT_FIXEDTEXT = u"stardiv.one.form.component.FixedText";
constexpr OUStringLiteral FM_COMPONENT_COMMANDBUTTON = u"stardiv.one.form.component.CommandButton";
constexpr OUStringLiteral FM_COMPONENT_CHECKBOX = u"stardiv.one.form.component.CheckBox";
#define FM_COMPONENT_GRID               "stardiv.one.form.component.Grid"
constexpr OUStringLiteral FM_COMPONENT_GRIDCONTROL = u"stardiv.one.form.component.GridControl";
constexpr OUStringLiteral FM_COMPONENT_IMAGEBUTTON = u"stardiv.one.form.component.ImageButton";
constexpr OUStringLiteral FM_COMPONENT_FILECONTROL = u"stardiv.one.form.component.FileControl";
constexpr OUStringLiteral FM_COMPONENT_TIMEFIELD = u"stardiv.one.form.component.TimeField";
constexpr OUStringLiteral FM_COMPONENT_DATEFIELD = u"stardiv.one.form.component.DateField";
constexpr OUStringLiteral FM_COMPONENT_NUMERICFIELD = u"stardiv.one.form.component.NumericField";
constexpr OUStringLiteral FM_COMPONENT_CURRENCYFIELD = u"stardiv.one.form.component.CurrencyField";
constexpr OUStringLiteral FM_COMPONENT_PATTERNFIELD = u"stardiv.one.form.component.PatternField";
constexpr OUStringLiteral FM_COMPONENT_FORMATTEDFIELD = u"stardiv.one.form.component.FormattedField";
constexpr OUStringLiteral FM_COMPONENT_HIDDEN = u"stardiv.one.form.component.Hidden";
constexpr OUStringLiteral FM_COMPONENT_HIDDENCONTROL = u"stardiv.one.form.component.HiddenControl";
constexpr OUStringLiteral FM_COMPONENT_IMAGECONTROL = u"stardiv.one.form.component.ImageControl";
#define FM_CONTROL_GRID                 "stardiv.one.form.control.Grid"
#define FM_CONTROL_GRIDCONTROL          "stardiv.one.form.control.GridControl"
constexpr OUStringLiteral SRV_SDB_CONNECTION = u"com.sun.star.sdb.Connection";
constexpr OUStringLiteral FM_SUN_COMPONENT_FORM = u"com.sun.star.form.component.Form";
#define FM_SUN_COMPONENT_TEXTFIELD      "com.sun.star.form.component.TextField"
#define FM_SUN_COMPONENT_LISTBOX        "com.sun.star.form.component.ListBox"
#define FM_SUN_COMPONENT_COMBOBOX       "com.sun.star.form.component.ComboBox"
#define FM_SUN_COMPONENT_RADIOBUTTON    "com.sun.star.form.component.RadioButton"
#define FM_SUN_COMPONENT_GROUPBOX       "com.sun.star.form.component.GroupBox"
#define FM_SUN_COMPONENT_FIXEDTEXT      "com.sun.star.form.component.FixedText"
#define FM_SUN_COMPONENT_COMMANDBUTTON  "com.sun.star.form.component.CommandButton"
#define FM_SUN_COMPONENT_CHECKBOX       "com.sun.star.form.component.CheckBox"
constexpr OUStringLiteral FM_SUN_COMPONENT_GRIDCONTROL = u"com.sun.star.form.component.GridControl";
#define FM_SUN_COMPONENT_IMAGEBUTTON    "com.sun.star.form.component.ImageButton"
#define FM_SUN_COMPONENT_FILECONTROL    "com.sun.star.form.component.FileControl"
#define FM_SUN_COMPONENT_TIMEFIELD      "com.sun.star.form.component.TimeField"
#define FM_SUN_COMPONENT_DATEFIELD      "com.sun.star.form.component.DateField"
#define FM_SUN_COMPONENT_NUMERICFIELD   "com.sun.star.form.component.NumericField"
#define FM_SUN_COMPONENT_CURRENCYFIELD  "com.sun.star.form.component.CurrencyField"
#define FM_SUN_COMPONENT_PATTERNFIELD   "com.sun.star.form.component.PatternField"
#define FM_SUN_COMPONENT_HIDDENCONTROL  "com.sun.star.form.component.HiddenControl"
#define FM_SUN_COMPONENT_IMAGECONTROL   "com.sun.star.form.component.DatabaseImageControl"
constexpr OUStringLiteral FM_SUN_COMPONENT_FORMATTEDFIELD = u"com.sun.star.form.component.FormattedField";
constexpr OUStringLiteral FM_SUN_COMPONENT_SCROLLBAR = u"com.sun.star.form.component.ScrollBar";
constexpr OUStringLiteral FM_SUN_COMPONENT_SPINBUTTON = u"com.sun.star.form.component.SpinButton";
constexpr OUStringLiteral FM_SUN_COMPONENT_NAVIGATIONBAR = u"com.sun.star.form.component.NavigationToolBar";
constexpr OUStringLiteral FM_SUN_CONTROL_GRIDCONTROL = u"com.sun.star.form.control.GridControl";

namespace svxform
{
    SVXCORE_DLLPUBLIC void ImplSmartRegisterUnoServices();

    css::uno::Reference<css::uno::XInterface>
    OAddConditionDialog_Create(
        css::uno::Reference<css::lang::XMultiServiceFactory> const &);

    OUString OAddConditionDialog_GetImplementationName();

    css::uno::Sequence<OUString>
    OAddConditionDialog_GetSupportedServiceNames();
}

/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface>
FmXGridControl_NewInstance_Impl(
    css::uno::Reference<css::lang::XMultiServiceFactory> const &);

/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface>
FormController_NewInstance_Impl(
    css::uno::Reference<css::lang::XMultiServiceFactory> const &);

/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface>
LegacyFormController_NewInstance_Impl(
    css::uno::Reference<css::lang::XMultiServiceFactory> const &);

#endif // INCLUDED_SVX_SOURCE_INC_FMSERVS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
