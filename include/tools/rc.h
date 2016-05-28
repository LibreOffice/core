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

#ifndef INCLUDED_TOOLS_RC_H
#define INCLUDED_TOOLS_RC_H

#include <tools/rcid.h>
#include <o3tl/typed_flags_set.hxx>

// Defines for all "Window" resources.
// (Here are all window styles that are not in WinBits)
#define RSC_EXTRAMAPUNIT                0xFFFF // do not change that value!


// Attributes in *.src files
enum class RSWND {
    NONE         = 0x00,
    DISABLED     = 0x01, // "Disable" attribute
    CLIENTSIZE   = 0x02  // "OutputSize" attribute
};
namespace o3tl {
    template<> struct typed_flags<RSWND> : is_typed_flags<RSWND, 0x03> {};
}

enum class RscWindowFlags {
    XYMapMode                = 0x0001,
    X                        = 0x0002,
    Y                        = 0x0004,
    WHMapMode                = 0x0008,
    Width                    = 0x0010,
    Height                   = 0x0020,
    Text                     = 0x0040,
    QuickText                = 0x0100,
    HelpId                   = 0x0200
};
namespace o3tl {
    template<> struct typed_flags<RscWindowFlags> : is_typed_flags<RscWindowFlags, 0x07> {};
}

// For "FixedImage" resource:
#define RSC_FIXEDIMAGE_IMAGE            0x0001

// For all menu resources:
enum class RscMenu {
    Items                  = 0x01,
    Text                   = 0x02,
    DefaultItemId          = 0x04
};
namespace o3tl {
    template<> struct typed_flags<RscMenu> : is_typed_flags<RscMenu, 0x07> {};
}

// "MenuItem" resource options:
enum class RscMenuItem {
    Separator          = 0x001,
    Id                 = 0x002,
    Status             = 0x004,
    Text               = 0x008,
    Bitmap             = 0x010,
    HelpText           = 0x020,
    HelpId             = 0x040,
    Menu               = 0x100,
    Disable            = 0x400,
    Command            = 0x800
};
namespace o3tl {
    template<> struct typed_flags<RscMenuItem> : is_typed_flags<RscMenuItem, 0xd7f> {};
}

// For "Field" resources:

enum class RscNumFormatterFlags {
    Min            = 0x01,
    Max            = 0x02,
    StrictFormat   = 0x04,
    DecimalDigits  = 0x10,
    Value          = 0x20,
};
namespace o3tl {
    template<> struct typed_flags<RscNumFormatterFlags> : is_typed_flags<RscNumFormatterFlags, 0x37> {};
}

enum class RscMetricFormatterFlags {
    Unit            = 0x01,
    CustomUnitText  = 0x02
};
namespace o3tl {
    template<> struct typed_flags<RscMetricFormatterFlags> : is_typed_flags<RscMetricFormatterFlags, 0x03> {};
}

#define NUMERICFIELD_FIRST              0x01
#define NUMERICFIELD_LAST               0x02
#define NUMERICFIELD_SPINSIZE           0x04

enum class RscMetricFieldFlags {
    First               = 0x01,
    Last                = 0x02,
    SpinSize            = 0x04
};
namespace o3tl {
    template<> struct typed_flags<RscMetricFieldFlags> : is_typed_flags<RscMetricFieldFlags, 0x07> {};
}

// For "ToolBoxItem" resources:
#define RSC_TOOLBOXITEM_ID              0x0001
#define RSC_TOOLBOXITEM_TYPE            0x0002
#define RSC_TOOLBOXITEM_STATUS          0x0004
#define RSC_TOOLBOXITEM_HELPID          0x0008
#define RSC_TOOLBOXITEM_TEXT            0x0010
#define RSC_TOOLBOXITEM_HELPTEXT        0x0020
#define RSC_TOOLBOXITEM_BITMAP          0x0040
#define RSC_TOOLBOXITEM_IMAGE           0x0080
#define RSC_TOOLBOXITEM_DISABLE         0x0100
#define RSC_TOOLBOXITEM_STATE           0x0200
#define RSC_TOOLBOXITEM_HIDE            0x0400
#define RSC_TOOLBOXITEM_COMMAND         0x0800

// For "ToolBox" resources:
enum class RscToolboxFlags {
    ButtonType          = 0x01,
    Align               = 0x02,
    LineCount           = 0x04,
    ItemList            = 0x80
};
namespace o3tl {
    template<> struct typed_flags<RscToolboxFlags> : is_typed_flags<RscToolboxFlags, 0x87> {};
}

// For "DockingWindow" resources:
#define RSC_DOCKINGWINDOW_XYMAPMODE     0x01
#define RSC_DOCKINGWINDOW_X             0x02
#define RSC_DOCKINGWINDOW_Y             0x04
#define RSC_DOCKINGWINDOW_FLOATING      0x08

// For "ImageButtons":
enum class RscImageButtonFlags {
    Image           = 0x01,
    Symbol          = 0x02,
    State           = 0x04
};
namespace o3tl {
    template<> struct typed_flags<RscImageButtonFlags> : is_typed_flags<RscImageButtonFlags, 0x07> {};
}

// For "Image":
enum class RscImageFlags {
    ImageBitmap       = 0x01,
    MaskColor         = 0x04
};
namespace o3tl {
    template<> struct typed_flags<RscImageFlags> : is_typed_flags<RscImageFlags, 0x05> {};
}

// For "ImageList":
enum class RscImageListFlags {
    MaskColor         = 0x04,
    IdList            = 0x08,
    IdCount           = 0x10
};
namespace o3tl {
    template<> struct typed_flags<RscImageListFlags> : is_typed_flags<RscImageListFlags, 0x1c> {};
}

// FIXME obsolete, should be removed by MM
#define RSC_COLOR                       (RSC_NOTYPE + 0x16)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
