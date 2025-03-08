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
    REMOVEBORDER      = 0x2000,
    // Never use native border, used to ensure consistency of form controls
    // inside documents across platforms and in pdf/print output
    NONATIVEBORDER    = 0x4000,
};
namespace o3tl
{
    template<> struct typed_flags<WindowBorderStyle> : is_typed_flags<WindowBorderStyle, 0x7033> {};
}

enum class TimeFormat
{
    Hour12, Hour24
};

enum class ExtTimeFieldFormat
{
    /** the first 4 of these are only used by base/dbaccess */
    Short24H, Long24H,
    Short12H, Long12H,
    ShortDuration, LongDuration
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

typedef sal_uInt32 sal_UCS4;    // TODO: this should be moved to rtl

enum class OutDevSupportType
{
    TransparentText     // if alpha in TextColor can be honored
};

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

enum class VclTaskBarStates {
    // Default, no specifics
    Normal,
    // Display some progress defined by SetTaskBarProgress
    Progress,
    // Something going on, but no exact progress scale
    ProgressUnknown,
    // Process on pause
    Paused,
    // Some error did happen
    Error
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
    Error,
    Other
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

enum class WizardButtonFlags : sal_Int16
{
    NONE                = 0x0000,
    NEXT                = 0x0001,
    PREVIOUS            = 0x0002,
    FINISH              = 0x0004,
    CANCEL              = 0x0008,
    HELP                = 0x0010,
};

namespace o3tl
{
    template<> struct typed_flags<WizardButtonFlags> : is_typed_flags<WizardButtonFlags, 0x001f> {};
}

// small, large, size32 force an exact toolbox size for proper alignment
// DontCare will let the toolbox decide about its size
enum class ToolBoxButtonSize
{
    DontCare,
    Small,
    Large,
    Size32,
};

// available button sizes for navigation bar
enum class NavbarButtonSize
{
    Auto,
    Small,
    Large,
    XLarge,
};

namespace vcl
{
    // The exact sizes of the icons in each size grouping are not necessarily
    // the exact size indicated by the name, but the upper limit of their size.
    // e.g. many Size26 icons are often 24x24px and only some 26x26px
    enum class ImageType
    {
        Size16,
        Size26,
        Size32,
        Small = Size16,
        LAST = Size32,
    };

    // Specifies a writing direction-based specialization for an image.
    // Sets do not necessarily provide all possible specializations.
    enum class ImageWritingDirection
    {
        LeftRightTopBottom = 0,
        RightLeftTopBottom,
        DontCare = LeftRightTopBottom,
    };
}

enum class DrawFrameStyle
{
    NONE                     = 0x0000,
    In                       = 0x0001,
    Out                      = 0x0002,
    Group                    = 0x0003,
    DoubleIn                 = 0x0004,
    DoubleOut                = 0x0005,
    NWF                      = 0x0006,
};

// Flags for DrawFrame()
enum class DrawFrameFlags
{
    NONE                     = 0x0000,
    Menu                     = 0x0010,
    WindowBorder             = 0x0020,
    BorderWindowBorder       = 0x0040,
    Mono                     = 0x1000,
    NoDraw                   = 0x8000,
};
namespace o3tl
{
    template<> struct typed_flags<DrawFrameFlags> : is_typed_flags<DrawFrameFlags, 0x9070> {};
}

enum class TxtAlign
{
    Left,
    Center,
    Right
};

enum class FloatWinPopupFlags
{
    NONE                 = 0x000000,
    AllowTearOff         = 0x000001,
    Down                 = 0x000002,
    Up                   = 0x000004,
    Left                 = 0x000008,
    Right                = 0x000010,
    NoKeyClose           = 0x000020,
    AllMouseButtonClose  = 0x000040,
    NoAppFocusClose      = 0x000080,
    NewLevel             = 0x000100,
    NoMouseUpClose       = 0x000200,
    GrabFocus            = 0x000400,
    NoHorzPlacement      = 0x000800,
};
namespace o3tl
{
    template<> struct typed_flags<FloatWinPopupFlags> : is_typed_flags<FloatWinPopupFlags, 0x0fff> {};
}

// Flags for Window::Show()
enum class ShowFlags
{
    NONE                       = 0x0000,
    NoFocusChange              = 0x0002,
    NoActivate                 = 0x0004,
    ForegroundTask             = 0x0008,
};
namespace o3tl
{
    template<> struct typed_flags<ShowFlags> : is_typed_flags<ShowFlags, 0x000e> {};
}

enum class TrackingEventFlags
{
    NONE           = 0x0000,
    Cancel         = 0x0001,
    Key            = 0x0002,
    Focus          = 0x0004,
    Repeat         = 0x0100,
    End            = 0x1000,
};
namespace o3tl
{
    template<> struct typed_flags<TrackingEventFlags> : is_typed_flags<TrackingEventFlags, 0x1107> {};
}

enum class ScrollType
{
    DontKnow,
    LineUp, LineDown,
    PageUp, PageDown,
    Drag
};

#endif // INCLUDED_VCL_VCLENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
