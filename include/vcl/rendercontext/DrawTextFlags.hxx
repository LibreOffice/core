/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#pragma once

#include <o3tl/typed_flags_set.hxx>

enum class DrawTextFlags
{
    NONE = 0x00000000,
    Disable = 0x00000001,
    Mnemonic = 0x00000002,
    Mono = 0x00000004,
    Clip = 0x00000008,
    Left = 0x00000010,
    Center = 0x00000020,
    Right = 0x00000040,
    Top = 0x00000080,
    VCenter = 0x00000100,
    Bottom = 0x00000200,
    EndEllipsis = 0x00000400,
    PathEllipsis = 0x00000800,
    MultiLine = 0x00001000,
    WordBreak = 0x00002000,
    NewsEllipsis = 0x00004000,
    WordBreakHyphenation = 0x00008000 | WordBreak,
    CenterEllipsis = 0x00010000,
    HideMnemonic = 0x00020000,
};
namespace o3tl
{
template <> struct typed_flags<DrawTextFlags> : is_typed_flags<DrawTextFlags, 0x3ffff>
{
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
