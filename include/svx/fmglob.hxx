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

#ifndef INCLUDED_SVX_FMGLOB_HXX
#define INCLUDED_SVX_FMGLOB_HXX

#include <com/sun/star/form/FormComponentType.hpp>
#include <svx/svdobj.hxx>

const sal_uInt32 FmFormInventor = sal_uInt32('F')*0x00000001+
                              sal_uInt32('M')*0x00000100+
                              sal_uInt32('0')*0x00010000+
                              sal_uInt32('1')*0x01000000;

const sal_uInt16 OBJ_FM_CONTROL         =   css::form::FormComponentType::CONTROL;
                                                                    // for form components
const sal_uInt16 OBJ_FM_EDIT            =   css::form::FormComponentType::TEXTFIELD;
const sal_uInt16 OBJ_FM_BUTTON          =   css::form::FormComponentType::COMMANDBUTTON;
const sal_uInt16 OBJ_FM_FIXEDTEXT       =   css::form::FormComponentType::FIXEDTEXT;
const sal_uInt16 OBJ_FM_LISTBOX         =   css::form::FormComponentType::LISTBOX;
const sal_uInt16 OBJ_FM_CHECKBOX        =   css::form::FormComponentType::CHECKBOX;
const sal_uInt16 OBJ_FM_COMBOBOX        =   css::form::FormComponentType::COMBOBOX;
const sal_uInt16 OBJ_FM_RADIOBUTTON     =   css::form::FormComponentType::RADIOBUTTON;
const sal_uInt16 OBJ_FM_GROUPBOX        =   css::form::FormComponentType::GROUPBOX;
const sal_uInt16 OBJ_FM_GRID            =   css::form::FormComponentType::GRIDCONTROL;
const sal_uInt16 OBJ_FM_IMAGEBUTTON     =   css::form::FormComponentType::IMAGEBUTTON;
const sal_uInt16 OBJ_FM_FILECONTROL     =   css::form::FormComponentType::FILECONTROL;
const sal_uInt16 OBJ_FM_DATEFIELD       =   css::form::FormComponentType::DATEFIELD;
const sal_uInt16 OBJ_FM_TIMEFIELD       =   css::form::FormComponentType::TIMEFIELD;
const sal_uInt16 OBJ_FM_NUMERICFIELD    =   css::form::FormComponentType::NUMERICFIELD;
const sal_uInt16 OBJ_FM_CURRENCYFIELD   =   css::form::FormComponentType::CURRENCYFIELD;
const sal_uInt16 OBJ_FM_PATTERNFIELD    =   css::form::FormComponentType::PATTERNFIELD;
const sal_uInt16 OBJ_FM_HIDDEN          =   css::form::FormComponentType::HIDDENCONTROL;
const sal_uInt16 OBJ_FM_IMAGECONTROL    =   css::form::FormComponentType::IMAGECONTROL;
const sal_uInt16 OBJ_FM_FORMATTEDFIELD  =   css::form::FormComponentType::PATTERNFIELD + 1;
const sal_uInt16 OBJ_FM_SCROLLBAR       =   css::form::FormComponentType::PATTERNFIELD + 2;
const sal_uInt16 OBJ_FM_SPINBUTTON      =   css::form::FormComponentType::PATTERNFIELD + 3;
const sal_uInt16 OBJ_FM_NAVIGATIONBAR   =   css::form::FormComponentType::PATTERNFIELD + 4;

#endif      // _FM_FMGLOB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
