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

#define WINDOW_XYMAPMODE                0x0001
#define WINDOW_X                        0x0002
#define WINDOW_Y                        0x0004
#define WINDOW_WHMAPMODE                0x0008
#define WINDOW_WIDTH                    0x0010
#define WINDOW_HEIGHT                   0x0020
#define WINDOW_TEXT                     0x0040
#define WINDOW_HELPTEXT                 0x0080
#define WINDOW_QUICKTEXT                0x0100
#define WINDOW_HELPID                   0x0200

// For "WorkWindow" resources:

#define WORKWIN_SHOWNORMAL              0
#define WORKWIN_SHOWMINIMIZED           1
#define WORKWIN_SHOWMAXIMIZED           2

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

#define METRICFORMATTER_UNIT            0x01
#define METRICFORMATTER_CUSTOMUNITTEXT  0x02

#define NUMERICFIELD_FIRST              0x01
#define NUMERICFIELD_LAST               0x02
#define NUMERICFIELD_SPINSIZE           0x04

#define METRICFIELD_FIRST               0x01
#define METRICFIELD_LAST                0x02
#define METRICFIELD_SPINSIZE            0x04

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
#define RSC_TOOLBOX_BUTTONTYPE          0x01
#define RSC_TOOLBOX_ALIGN               0x02
#define RSC_TOOLBOX_LINECOUNT           0x04
#define RSC_TOOLBOX_FLOATLINES          0x08
#define RSC_TOOLBOX_CUSTOMIZE           0x10
#define RSC_TOOLBOX_MENUSTRINGS         0x20
#define RSC_TOOLBOX_ITEMIMAGELIST       0x40
#define RSC_TOOLBOX_ITEMLIST            0x80

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
#define RSC_IMAGE_IMAGEBITMAP           0x01
#define RSC_IMAGE_MASKBITMAP            0x02
#define RSC_IMAGE_MASKCOLOR             0x04

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
