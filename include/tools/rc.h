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
    template<> struct typed_flags<RscWindowFlags> : is_typed_flags<RscWindowFlags, 0x37F> {};
}

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
    HelpText           = 0x020,
    HelpId             = 0x040,
    Menu               = 0x100,
    Disable            = 0x400,
    Command            = 0x800
};
namespace o3tl {
    template<> struct typed_flags<RscMenuItem> : is_typed_flags<RscMenuItem, 0xd6f> {};
}

// For "Image":
enum class RscImageFlags {
    ImageBitmap       = 0x01,
};
namespace o3tl {
    template<> struct typed_flags<RscImageFlags> : is_typed_flags<RscImageFlags, 0x01> {};
}

// For "ImageList":
enum class RscImageListFlags {
    IdList            = 0x08,
    IdCount           = 0x10
};
namespace o3tl {
    template<> struct typed_flags<RscImageListFlags> : is_typed_flags<RscImageListFlags, 0x18> {};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
