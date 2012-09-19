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
#ifndef _SVX_FMSERVS_HXX
#define _SVX_FMSERVS_HXX

#include <svx/svxdllapi.h>

#define FM_COMPONENT_EDIT               rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.Edit" ) )
#define FM_COMPONENT_TEXTFIELD          rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.TextField" ) )
#define FM_COMPONENT_LISTBOX            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.ListBox" ) )
#define FM_COMPONENT_COMBOBOX           rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.ComboBox" ) )
#define FM_COMPONENT_RADIOBUTTON        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.RadioButton" ) )
#define FM_COMPONENT_GROUPBOX           rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.GroupBox" ) )
#define FM_COMPONENT_FIXEDTEXT          rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.FixedText" ) )
#define FM_COMPONENT_COMMANDBUTTON      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.CommandButton" ) )
#define FM_COMPONENT_CHECKBOX           rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.CheckBox" ) )
#define FM_COMPONENT_GRID               rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.Grid" ) )
#define FM_COMPONENT_GRIDCONTROL        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.GridControl" ) )
#define FM_COMPONENT_IMAGEBUTTON        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.ImageButton" ) )
#define FM_COMPONENT_FILECONTROL        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.FileControl" ) )
#define FM_COMPONENT_TIMEFIELD          rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.TimeField" ) )
#define FM_COMPONENT_DATEFIELD          rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.DateField" ) )
#define FM_COMPONENT_NUMERICFIELD       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.NumericField" ) )
#define FM_COMPONENT_CURRENCYFIELD      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.CurrencyField" ) )
#define FM_COMPONENT_PATTERNFIELD       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.PatternField" ) )
#define FM_COMPONENT_FORMATTEDFIELD     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.FormattedField" ) )
#define FM_COMPONENT_HIDDEN             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.Hidden" ) )
#define FM_COMPONENT_HIDDENCONTROL      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.HiddenControl" ) )
#define FM_COMPONENT_IMAGECONTROL       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.component.ImageControl" ) )
#define FM_CONTROL_GRID                 rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.control.Grid" ) )
#define FM_CONTROL_GRIDCONTROL          rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "stardiv.one.form.control.GridControl" ) )
#define FM_FORM_CONTROLLER              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.runtime.FormController" ) )
#define SRV_SDB_CONNECTION              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.Connection" ) )
#define FM_SUN_COMPONENT_FORM           rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.Form" ) )
#define FM_SUN_COMPONENT_TEXTFIELD      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.TextField" ) )
#define FM_SUN_COMPONENT_LISTBOX        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.ListBox" ) )
#define FM_SUN_COMPONENT_COMBOBOX       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.ComboBox" ) )
#define FM_SUN_COMPONENT_RADIOBUTTON    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.RadioButton" ) )
#define FM_SUN_COMPONENT_GROUPBOX       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.GroupBox" ) )
#define FM_SUN_COMPONENT_FIXEDTEXT      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.FixedText" ) )
#define FM_SUN_COMPONENT_COMMANDBUTTON  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.CommandButton" ) )
#define FM_SUN_COMPONENT_CHECKBOX       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.CheckBox" ) )
#define FM_SUN_COMPONENT_GRIDCONTROL    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.GridControl" ) )
#define FM_SUN_COMPONENT_IMAGEBUTTON    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.ImageButton" ) )
#define FM_SUN_COMPONENT_FILECONTROL    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.FileControl" ) )
#define FM_SUN_COMPONENT_TIMEFIELD      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.TimeField" ) )
#define FM_SUN_COMPONENT_DATEFIELD      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.DateField" ) )
#define FM_SUN_COMPONENT_NUMERICFIELD   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.NumericField" ) )
#define FM_SUN_COMPONENT_CURRENCYFIELD  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.CurrencyField" ) )
#define FM_SUN_COMPONENT_PATTERNFIELD   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.PatternField" ) )
#define FM_SUN_COMPONENT_HIDDENCONTROL  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.HiddenControl" ) )
#define FM_SUN_COMPONENT_IMAGECONTROL   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.DatabaseImageControl" ) )
#define FM_SUN_COMPONENT_FORMATTEDFIELD rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.FormattedField" ) )
#define FM_SUN_COMPONENT_SCROLLBAR      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.ScrollBar" ) )
#define FM_SUN_COMPONENT_SPINBUTTON     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.SpinButton" ) )
#define FM_SUN_COMPONENT_NAVIGATIONBAR  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.NavigationToolBar" ) )
#define FM_SUN_CONTROL_GRIDCONTROL      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.control.GridControl" ) )

namespace svxform
{
    SVX_DLLPUBLIC void ImplSmartRegisterUnoServices();
}   // namespace svxform

#endif // _SVX_FMSERVS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
