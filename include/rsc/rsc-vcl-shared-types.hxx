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

#ifndef INCLUDED_RSC_RSC_VCL_SHARED_TYPES_HXX
#define INCLUDED_RSC_RSC_VCL_SHARED_TYPES_HXX

#include <sal/types.h>

enum TimeFieldFormat {TIMEF_NONE, TIMEF_SEC, TIMEF_100TH_SEC, TIMEF_SEC_CS, TimeFieldFormat_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

enum KeyFuncType { KEYFUNC_DONTKNOW, KEYFUNC_NEW, KEYFUNC_OPEN, KEYFUNC_SAVE,
                   KEYFUNC_SAVEAS, KEYFUNC_PRINT, KEYFUNC_CLOSE, KEYFUNC_QUIT,
                   KEYFUNC_CUT, KEYFUNC_COPY, KEYFUNC_PASTE, KEYFUNC_UNDO,
                   KEYFUNC_REDO, KEYFUNC_DELETE, KEYFUNC_REPEAT, KEYFUNC_FIND,
                   KEYFUNC_FINDBACKWARD, KEYFUNC_PROPERTIES, KEYFUNC_FRONT,
                   KeyFuncType_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

enum MenuItemType { MENUITEM_DONTKNOW, MENUITEM_STRING, MENUITEM_IMAGE,
                    MENUITEM_STRINGIMAGE, MENUITEM_SEPARATOR };

typedef sal_uInt16 MenuItemBits;

#define MIB_CHECKABLE           ((MenuItemBits)0x0001)
#define MIB_RADIOCHECK          ((MenuItemBits)0x0002)
#define MIB_AUTOCHECK           ((MenuItemBits)0x0004)
#define MIB_ABOUT               ((MenuItemBits)0x0008)
#define MIB_HELP                ((MenuItemBits)0x0010)
#define MIB_POPUPSELECT         ((MenuItemBits)0x0020)

// These have been said to be a prelimitary (sic) solution since 2007
#define MIB_NOSELECT            ((MenuItemBits)0x0040)
#define MIB_ICON            ((MenuItemBits)0x0080)
#define MIB_TEXT            ((MenuItemBits)0x0100)

typedef sal_uInt16 ToolBoxItemBits;

#define TIB_CHECKABLE           ((ToolBoxItemBits)0x0001)
#define TIB_RADIOCHECK          ((ToolBoxItemBits)0x0002)
#define TIB_AUTOCHECK           ((ToolBoxItemBits)0x0004)
#define TIB_LEFT                ((ToolBoxItemBits)0x0008)
#define TIB_AUTOSIZE            ((ToolBoxItemBits)0x0010)
#define TIB_DROPDOWN            ((ToolBoxItemBits)0x0020)
#define TIB_REPEAT              ((ToolBoxItemBits)0x0040)
#define TIB_DROPDOWNONLY        ((ToolBoxItemBits)0x0080 | TIB_DROPDOWN)
#define TIB_TEXT_ONLY           ((ToolBoxItemBits)0x0100)
#define TIB_ICON_ONLY           ((ToolBoxItemBits)0x0200)
#define TIB_TEXTICON            ((ToolBoxItemBits) TIB_TEXT_ONLY | TIB_ICON_ONLY )

enum ToolBoxItemType { TOOLBOXITEM_DONTKNOW, TOOLBOXITEM_BUTTON,
                       TOOLBOXITEM_SPACE, TOOLBOXITEM_SEPARATOR,
                       TOOLBOXITEM_BREAK };

enum ButtonType { BUTTON_SYMBOL, BUTTON_TEXT, BUTTON_SYMBOLTEXT };

enum class SymbolType : sal_uInt16
{
    DONTKNOW         = 0,
    IMAGE            = 1,
    ARROW_UP         = 2,
    ARROW_DOWN       = 3,
    ARROW_LEFT       = 4,
    ARROW_RIGHT      = 5,
    SPIN_UP          = 6,
    SPIN_DOWN        = 7,
    SPIN_LEFT        = 8,
    SPIN_RIGHT       = 9,
    FIRST            = 10,
    LAST             = 11,
    PREV             = 12,
    NEXT             = 13,
    PAGEUP           = 14,
    PAGEDOWN         = 15,
    PLAY             = 16,
    REVERSEPLAY      = 17,
    RECORD           = 18,
    STOP             = 19,
    PAUSE            = 20,
    WINDSTART        = 21,
    WINDEND          = 22,
    WINDBACKWARD     = 23,
    WINDFORWARD      = 24,
    CLOSE            = 25,
    ROLLUP           = 26,
    ROLLDOWN         = 27,
    CHECKMARK        = 28,
    RADIOCHECKMARK   = 29,
    SPIN_UPDOWN      = 30,
    FLOAT            = 31,
    DOCK             = 32,
    HIDE             = 33,
    HELP             = 34,
    PLUS             = 35,
    MENU             = SymbolType::SPIN_DOWN
};


// Border-Styles fuer SetBorder()
typedef sal_uInt16 WindowBorderStyle;

#define WINDOW_BORDER_NORMAL            ((WindowBorderStyle)0x0001)
#define WINDOW_BORDER_MONO              ((WindowBorderStyle)0x0002)
#define WINDOW_BORDER_MENU              ((WindowBorderStyle)0x0010)
#define WINDOW_BORDER_NWF               ((WindowBorderStyle)0x0020)
#define WINDOW_BORDER_NOBORDER          ((WindowBorderStyle)0x1000)
#define WINDOW_BORDER_REMOVEBORDER      ((WindowBorderStyle)0x2000)

#define KEY_CODE        ((sal_uInt16)0x0FFF)

// Modifier-Tasten
#define KEY_SHIFT       ((sal_uInt16)0x1000)
#define KEY_MOD1        ((sal_uInt16)0x2000)
#define KEY_MOD2        ((sal_uInt16)0x4000)
#define KEY_MOD3        ((sal_uInt16)0x8000)
#define KEY_MODTYPE     ((sal_uInt16)0xF000)
#define KEY_ALLMODTYPE  ((sal_uInt16)0xF000)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
