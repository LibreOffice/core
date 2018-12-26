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

#ifndef INCLUDED_VCL_VCLENUM_HXX
#define INCLUDED_VCL_VCLENUM_HXX

#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>

enum class SelectionMode { NONE, Single, Range, Multiple };

enum class TimeFieldFormat : sal_Int32 { F_NONE, F_SEC, F_SEC_CS };

enum class MenuItemType { DONTKNOW, STRING, IMAGE, STRINGIMAGE, SEPARATOR };

enum class MenuItemBits : sal_Int16
{
    NONE                = 0x0000,
    CHECKABLE           = 0x0001,
    RADIOCHECK          = 0x0002,
    AUTOCHECK           = 0x0004,
    HELP                = 0x0010,
    POPUPSELECT         = 0x0020,
    // These have been said to be a preliminary (sic) solution since 2007
    NOSELECT            = 0x0040,
    ICON                = 0x0080,
    TEXT                = 0x0100,
};
namespace o3tl
{
    template<> struct typed_flags<MenuItemBits> : is_typed_flags<MenuItemBits, 0x1f7> {};
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
    ICON_ONLY           = 0x0200
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
    STOP             = 19,
    CLOSE            = 25,
    ROLLUP           = 26,
    ROLLDOWN         = 27,
    CHECKMARK        = 28,
    RADIOCHECKMARK   = 29,
    FLOAT            = 31,
    DOCK             = 32,
    HIDE             = 33,
    HELP             = 34,
    PLUS             = 35,
    MENU             = SymbolType::SPIN_DOWN
};


// Border styles for SetBorder()
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

enum class WindowStateMask {
    NONE             = 0x0000,
    X                = 0x0001,
    Y                = 0x0002,
    Width            = 0x0004,
    Height           = 0x0008,
    State            = 0x0010,
    Minimized        = 0x0020,
    MaximizedX       = 0x0100,
    MaximizedY       = 0x0200,
    MaximizedWidth   = 0x0400,
    MaximizedHeight  = 0x0800,
    Pos              = X | Y,
    All              = X | Y | Width | Height | MaximizedX | MaximizedY | MaximizedWidth | MaximizedHeight | State | Minimized
};
namespace o3tl
{
    template<> struct typed_flags<WindowStateMask> : is_typed_flags<WindowStateMask, 0x0f3f> {};
}

enum class TimeFormat
{
    Hour12, Hour24
};

enum class ExtTimeFieldFormat
{
    Short24H, Long24H
};

enum class ExtDateFieldFormat
{
    SystemShort, SystemShortYY, SystemShortYYYY,
    SystemLong,
    ShortDDMMYY, ShortMMDDYY, ShortYYMMDD,
    ShortDDMMYYYY, ShortMMDDYYYY, ShortYYYYMMDD,
    ShortYYMMDD_DIN5008, ShortYYYYMMDD_DIN5008,
    FORCE_EQUAL_SIZE=SAL_MAX_ENUM
};

// this appears to be a direct copy of css::awt::GradientStyle
enum class GradientStyle
{
    Linear = 0,
    Axial = 1,
    Radial = 2,
    Elliptical = 3,
    Square = 4,
    Rect = 5,
    FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

enum class HatchStyle
{
    Single = 0,
    Double = 1,
    Triple = 2,
    FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

enum class LineStyle
{
    NONE = 0,
    Solid = 1,
    Dash = 2,
    FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

enum class RasterOp { OverPaint, Xor, N0, N1, Invert };

typedef sal_uInt32 sal_UCS4;    // TODO: this should be moved to rtl

enum class OutDevSupportType { TransparentRect, B2DDraw };

struct ItalicMatrix
{
    double xx, xy, yx, yy;
    ItalicMatrix() : xx(1), xy(0), yx(0), yy(1) {}
};

inline bool operator ==(const ItalicMatrix& a, const ItalicMatrix& b)
{
    return a.xx == b.xx && a.xy == b.xy && a.yx == b.yx && a.yy == b.yy;
}

inline bool operator !=(const ItalicMatrix& a, const ItalicMatrix& b)
{
    return !(a == b);
}

enum class VclAlign
{
    Fill,
    Start,
    End,
    Center
};

enum class VclPackType
{
    Start = 0,
    End = 1,
    LAST = End
};

// Return Values from Dialog::Execute
//!!! in case of changes adjust /basic/source/runtime/methods.cxx msgbox

enum VclResponseType
{
    RET_CANCEL  = 0,
    RET_OK      = 1,
    RET_YES     = 2,
    RET_NO      = 3,
    RET_RETRY   = 4,
    RET_IGNORE  = 5,
    RET_CLOSE   = 7,
    RET_HELP    = 10
};

enum class VclButtonsType
{
    NONE,
    Ok,
    Close,
    Cancel,
    YesNo,
    OkCancel
};

enum class VclMessageType
{
    Info,
    Warning,
    Question,
    Error
};

enum class VclSizeGroupMode
{
    NONE,
    Horizontal,
    Vertical,
    Both
};

enum class VclPolicyType
{
    ALWAYS,
    AUTOMATIC,
    NEVER
};

#endif // INCLUDED_VCL_VCLENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
