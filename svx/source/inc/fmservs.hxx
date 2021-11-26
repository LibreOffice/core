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

inline constexpr OUStringLiteral FM_COMPONENT_EDIT = u"stardiv.one.form.component.Edit";
inline constexpr OUStringLiteral FM_COMPONENT_TEXTFIELD = u"stardiv.one.form.component.TextField";
inline constexpr OUStringLiteral FM_COMPONENT_LISTBOX = u"stardiv.one.form.component.ListBox";
inline constexpr OUStringLiteral FM_COMPONENT_COMBOBOX = u"stardiv.one.form.component.ComboBox";
inline constexpr OUStringLiteral FM_COMPONENT_RADIOBUTTON = u"stardiv.one.form.component.RadioButton";
inline constexpr OUStringLiteral FM_COMPONENT_GROUPBOX = u"stardiv.one.form.component.GroupBox";
inline constexpr OUStringLiteral FM_COMPONENT_FIXEDTEXT = u"stardiv.one.form.component.FixedText";
inline constexpr OUStringLiteral FM_COMPONENT_COMMANDBUTTON = u"stardiv.one.form.component.CommandButton";
inline constexpr OUStringLiteral FM_COMPONENT_CHECKBOX = u"stardiv.one.form.component.CheckBox";
inline constexpr OUStringLiteral FM_COMPONENT_GRID = u"stardiv.one.form.component.Grid";
inline constexpr OUStringLiteral FM_COMPONENT_GRIDCONTROL = u"stardiv.one.form.component.GridControl";
inline constexpr OUStringLiteral FM_COMPONENT_IMAGEBUTTON = u"stardiv.one.form.component.ImageButton";
inline constexpr OUStringLiteral FM_COMPONENT_FILECONTROL = u"stardiv.one.form.component.FileControl";
inline constexpr OUStringLiteral FM_COMPONENT_TIMEFIELD = u"stardiv.one.form.component.TimeField";
inline constexpr OUStringLiteral FM_COMPONENT_DATEFIELD = u"stardiv.one.form.component.DateField";
inline constexpr OUStringLiteral FM_COMPONENT_NUMERICFIELD = u"stardiv.one.form.component.NumericField";
inline constexpr OUStringLiteral FM_COMPONENT_CURRENCYFIELD = u"stardiv.one.form.component.CurrencyField";
inline constexpr OUStringLiteral FM_COMPONENT_PATTERNFIELD = u"stardiv.one.form.component.PatternField";
inline constexpr OUStringLiteral FM_COMPONENT_FORMATTEDFIELD = u"stardiv.one.form.component.FormattedField";
inline constexpr OUStringLiteral FM_COMPONENT_HIDDEN = u"stardiv.one.form.component.Hidden";
inline constexpr OUStringLiteral FM_COMPONENT_HIDDENCONTROL = u"stardiv.one.form.component.HiddenControl";
inline constexpr OUStringLiteral FM_COMPONENT_IMAGECONTROL = u"stardiv.one.form.component.ImageControl";
inline constexpr OUStringLiteral FM_CONTROL_GRID = u"stardiv.one.form.control.Grid";
inline constexpr OUStringLiteral FM_CONTROL_GRIDCONTROL = u"stardiv.one.form.control.GridControl";
inline constexpr OUStringLiteral SRV_SDB_CONNECTION = u"com.sun.star.sdb.Connection";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_FORM = u"com.sun.star.form.component.Form";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_TEXTFIELD = u"com.sun.star.form.component.TextField";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_LISTBOX = u"com.sun.star.form.component.ListBox";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_COMBOBOX = u"com.sun.star.form.component.ComboBox";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_RADIOBUTTON = u"com.sun.star.form.component.RadioButton";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_GROUPBOX = u"com.sun.star.form.component.GroupBox";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_FIXEDTEXT = u"com.sun.star.form.component.FixedText";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_COMMANDBUTTON = u"com.sun.star.form.component.CommandButton";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_CHECKBOX = u"com.sun.star.form.component.CheckBox";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_GRIDCONTROL = u"com.sun.star.form.component.GridControl";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_IMAGEBUTTON = u"com.sun.star.form.component.ImageButton";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_FILECONTROL = u"com.sun.star.form.component.FileControl";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_TIMEFIELD = u"com.sun.star.form.component.TimeField";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_DATEFIELD = u"com.sun.star.form.component.DateField";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_NUMERICFIELD = u"com.sun.star.form.component.NumericField";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_CURRENCYFIELD = u"com.sun.star.form.component.CurrencyField";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_PATTERNFIELD = u"com.sun.star.form.component.PatternField";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_HIDDENCONTROL = u"com.sun.star.form.component.HiddenControl";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_IMAGECONTROL = u"com.sun.star.form.component.DatabaseImageControl";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_FORMATTEDFIELD = u"com.sun.star.form.component.FormattedField";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_SCROLLBAR = u"com.sun.star.form.component.ScrollBar";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_SPINBUTTON = u"com.sun.star.form.component.SpinButton";
inline constexpr OUStringLiteral FM_SUN_COMPONENT_NAVIGATIONBAR = u"com.sun.star.form.component.NavigationToolBar";
inline constexpr OUStringLiteral FM_SUN_CONTROL_GRIDCONTROL = u"com.sun.star.form.control.GridControl";

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
