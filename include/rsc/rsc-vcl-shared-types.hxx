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
#include <o3tl/typed_flags_set.hxx>

enum class TimeFieldFormat : sal_Int32 { F_NONE, F_SEC, F_100TH_SEC, F_SEC_CS };

enum class KeyFuncType : sal_Int32 { DONTKNOW, NEW, OPEN, SAVE,
                   SAVEAS, PRINT, CLOSE, QUIT,
                   CUT, COPY, PASTE, UNDO,
                   REDO, DELETE, REPEAT, FIND,
                   FINDBACKWARD, PROPERTIES, FRONT };

enum class MenuItemType { DONTKNOW, STRING, IMAGE, STRINGIMAGE, SEPARATOR };

enum class MenuItemBits : sal_Int16
{
    NONE                = 0x0000,
    CHECKABLE           = 0x0001,
    RADIOCHECK          = 0x0002,
    AUTOCHECK           = 0x0004,
    ABOUT               = 0x0008,
    HELP                = 0x0010,
    POPUPSELECT         = 0x0020,
    // These have been said to be a prelimitary (sic) solution since 2007
    NOSELECT            = 0x0040,
    ICON                = 0x0080,
    TEXT                = 0x0100,
    BOLD                = 0x0200
};
namespace o3tl
{
    template<> struct typed_flags<MenuItemBits> : is_typed_flags<MenuItemBits, 0x1ff> {};
}

enum class ToolBoxItemBits
{
    NONE                = 0x0000,
    CHECKABLE           = 0x0001,
    RADIOCHECK          = 0x0002,
    AUTOCHECK           = 0x0004,
    LEFT                = 0x0008,
    AUTOSIZE            = 0x0010,
    DROPDOWN            = 0x0020,
    REPEAT              = 0x0040,
    DROPDOWNONLY        = 0x00a0, // 0x0080 | DROPDOWN
    TEXT_ONLY           = 0x0100,
    ICON_ONLY           = 0x0200,
    TEXTICON            = 0x0300 // TEXT_ONLY | ICON_ONLY
};
namespace o3tl
{
    template<> struct typed_flags<ToolBoxItemBits> : is_typed_flags<ToolBoxItemBits, 0x3ff> {};
}

enum class ToolBoxItemType { DONTKNOW, BUTTON, SPACE, SEPARATOR, BREAK };

enum class ButtonType { SYMBOLONLY, TEXT, SYMBOLTEXT };

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
enum class WindowBorderStyle : sal_Int16
{
    NONE              = 0x0000,
    NORMAL            = 0x0001,
    MONO              = 0x0002,
    MENU              = 0x0010,
    NWF               = 0x0020,
    NOBORDER          = 0x1000,
    REMOVEBORDER      = 0x2000
};
namespace o3tl
{
    template<> struct typed_flags<WindowBorderStyle> : is_typed_flags<WindowBorderStyle, 0x3033> {};
}

#define KEY_CODE_MASK       ((sal_uInt16)0x0FFF)

// Modifier-Tasten
#define KEY_SHIFT           ((sal_uInt16)0x1000)
#define KEY_MOD1            ((sal_uInt16)0x2000)
#define KEY_MOD2            ((sal_uInt16)0x4000)
#define KEY_MOD3            ((sal_uInt16)0x8000)
#define KEY_MODIFIERS_MASK  ((sal_uInt16)0xF000)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
