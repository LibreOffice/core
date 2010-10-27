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

#ifndef _SVX_FMGLOB_HXX
#define _SVX_FMGLOB_HXX

#include <tools/solar.h>
#include <svx/svdobj.hxx>
#include <com/sun/star/form/FormComponentType.hpp>

const sal_uInt32 FmFormInventor = sal_uInt32('F')*0x00000001+
                              sal_uInt32('M')*0x00000100+
                              sal_uInt32('0')*0x00010000+
                              sal_uInt32('1')*0x01000000;

const sal_uInt16 OBJ_FM_CONTROL         =   ::com::sun::star::form::FormComponentType::CONTROL;
                                                                    // for form components
const sal_uInt16 OBJ_FM_EDIT            =   ::com::sun::star::form::FormComponentType::TEXTFIELD;
const sal_uInt16 OBJ_FM_BUTTON          =   ::com::sun::star::form::FormComponentType::COMMANDBUTTON;
const sal_uInt16 OBJ_FM_FIXEDTEXT       =   ::com::sun::star::form::FormComponentType::FIXEDTEXT;
const sal_uInt16 OBJ_FM_LISTBOX         =   ::com::sun::star::form::FormComponentType::LISTBOX;
const sal_uInt16 OBJ_FM_CHECKBOX        =   ::com::sun::star::form::FormComponentType::CHECKBOX;
const sal_uInt16 OBJ_FM_COMBOBOX        =   ::com::sun::star::form::FormComponentType::COMBOBOX;
const sal_uInt16 OBJ_FM_RADIOBUTTON     =   ::com::sun::star::form::FormComponentType::RADIOBUTTON;
const sal_uInt16 OBJ_FM_GROUPBOX        =   ::com::sun::star::form::FormComponentType::GROUPBOX;
const sal_uInt16 OBJ_FM_GRID            =   ::com::sun::star::form::FormComponentType::GRIDCONTROL;
const sal_uInt16 OBJ_FM_IMAGEBUTTON     =   ::com::sun::star::form::FormComponentType::IMAGEBUTTON;
const sal_uInt16 OBJ_FM_FILECONTROL     =   ::com::sun::star::form::FormComponentType::FILECONTROL;
const sal_uInt16 OBJ_FM_DATEFIELD       =   ::com::sun::star::form::FormComponentType::DATEFIELD;
const sal_uInt16 OBJ_FM_TIMEFIELD       =   ::com::sun::star::form::FormComponentType::TIMEFIELD;
const sal_uInt16 OBJ_FM_NUMERICFIELD    =   ::com::sun::star::form::FormComponentType::NUMERICFIELD;
const sal_uInt16 OBJ_FM_CURRENCYFIELD   =   ::com::sun::star::form::FormComponentType::CURRENCYFIELD;
const sal_uInt16 OBJ_FM_PATTERNFIELD    =   ::com::sun::star::form::FormComponentType::PATTERNFIELD;
const sal_uInt16 OBJ_FM_HIDDEN          =   ::com::sun::star::form::FormComponentType::HIDDENCONTROL;
const sal_uInt16 OBJ_FM_IMAGECONTROL    =   ::com::sun::star::form::FormComponentType::IMAGECONTROL;
const sal_uInt16 OBJ_FM_FORMATTEDFIELD  =   ::com::sun::star::form::FormComponentType::PATTERNFIELD + 1;
const sal_uInt16 OBJ_FM_SCROLLBAR       =   ::com::sun::star::form::FormComponentType::PATTERNFIELD + 2;
const sal_uInt16 OBJ_FM_SPINBUTTON      =   ::com::sun::star::form::FormComponentType::PATTERNFIELD + 3;
const sal_uInt16 OBJ_FM_NAVIGATIONBAR   =   ::com::sun::star::form::FormComponentType::PATTERNFIELD + 4;

#endif      // _FM_FMGLOB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
