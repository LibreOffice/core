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

// For all menu resources:
enum class RscMenu {
    Items                  = 0x01,
    Text                   = 0x02,
};
namespace o3tl {
    template<> struct typed_flags<RscMenu> : is_typed_flags<RscMenu, 0x03> {};
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
