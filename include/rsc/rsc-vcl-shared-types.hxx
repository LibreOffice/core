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

#ifndef _RSC_RSC_VCL_SHARED_TYPES_HXX
#define _RSC_RSC_VCL_SHARED_TYPES_HXX

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

typedef sal_uInt16 SymbolType;

#define SYMBOL_DONTKNOW         ((SymbolType)0)
#define SYMBOL_IMAGE            ((SymbolType)1)
#define SYMBOL_ARROW_UP         ((SymbolType)2)
#define SYMBOL_ARROW_DOWN       ((SymbolType)3)
#define SYMBOL_ARROW_LEFT       ((SymbolType)4)
#define SYMBOL_ARROW_RIGHT      ((SymbolType)5)
#define SYMBOL_SPIN_UP          ((SymbolType)6)
#define SYMBOL_SPIN_DOWN        ((SymbolType)7)
#define SYMBOL_SPIN_LEFT        ((SymbolType)8)
#define SYMBOL_SPIN_RIGHT       ((SymbolType)9)
#define SYMBOL_FIRST            ((SymbolType)10)
#define SYMBOL_LAST             ((SymbolType)11)
#define SYMBOL_PREV             ((SymbolType)12)
#define SYMBOL_NEXT             ((SymbolType)13)
#define SYMBOL_PAGEUP           ((SymbolType)14)
#define SYMBOL_PAGEDOWN         ((SymbolType)15)
#define SYMBOL_PLAY             ((SymbolType)16)
#define SYMBOL_REVERSEPLAY      ((SymbolType)17)
#define SYMBOL_RECORD           ((SymbolType)18)
#define SYMBOL_STOP             ((SymbolType)19)
#define SYMBOL_PAUSE            ((SymbolType)20)
#define SYMBOL_WINDSTART        ((SymbolType)21)
#define SYMBOL_WINDEND          ((SymbolType)22)
#define SYMBOL_WINDBACKWARD     ((SymbolType)23)
#define SYMBOL_WINDFORWARD      ((SymbolType)24)
#define SYMBOL_CLOSE            ((SymbolType)25)
#define SYMBOL_ROLLUP           ((SymbolType)26)
#define SYMBOL_ROLLDOWN         ((SymbolType)27)
#define SYMBOL_CHECKMARK        ((SymbolType)28)
#define SYMBOL_RADIOCHECKMARK   ((SymbolType)29)
#define SYMBOL_SPIN_UPDOWN      ((SymbolType)30)
#define SYMBOL_FLOAT            ((SymbolType)31)
#define SYMBOL_DOCK             ((SymbolType)32)
#define SYMBOL_HIDE             ((SymbolType)33)
#define SYMBOL_HELP             ((SymbolType)34)
#define SYMBOL_PLUS             ((SymbolType)35)
#define SYMBOL_MENU             SYMBOL_SPIN_DOWN

#define SYMBOL_NOSYMBOL         (SYMBOL_DONTKNOW)

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
