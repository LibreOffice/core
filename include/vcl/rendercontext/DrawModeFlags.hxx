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

#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>

enum class DrawModeFlags : sal_uInt32
{
    Default = 0x00000000,
    BlackLine = 0x00000001,
    BlackFill = 0x00000002,
    BlackText = 0x00000004,
    BlackBitmap = 0x00000008,
    BlackGradient = 0x00000010,
    GrayLine = 0x00000020,
    GrayFill = 0x00000040,
    GrayText = 0x00000080,
    GrayBitmap = 0x00000100,
    GrayGradient = 0x00000200,
    NoFill = 0x00000400,
    WhiteLine = 0x00000800,
    WhiteFill = 0x00001000,
    WhiteText = 0x00002000,
    WhiteBitmap = 0x00004000,
    WhiteGradient = 0x00008000,
    SettingsLine = 0x00010000,
    SettingsFill = 0x00020000,
    SettingsText = 0x00040000,
    SettingsGradient = 0x00080000,
    SettingsForSelection = 0x00100000,
    NoTransparency = 0x00200000,
};
namespace o3tl
{
template <> struct typed_flags<DrawModeFlags> : is_typed_flags<DrawModeFlags, 0x3fffff>
{
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
