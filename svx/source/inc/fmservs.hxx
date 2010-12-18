/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_FMSERVS_HXX
#define _SVX_FMSERVS_HXX

#include <svx/svxdllapi.h>

#define FM_NUMBER_FORMATTER             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.NumberFormatter" ) )
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
