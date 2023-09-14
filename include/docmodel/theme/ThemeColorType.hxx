/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sal/types.h>

namespace model
{
/// Offsets into the color list of a theme.
enum class ThemeColorType : sal_Int32
{
    Unknown = -1,
    Dark1 = 0,
    Light1 = 1,
    Dark2 = 2,
    Light2 = 3,
    Accent1 = 4,
    Accent2 = 5,
    Accent3 = 6,
    Accent4 = 7,
    Accent5 = 8,
    Accent6 = 9,
    Hyperlink = 10,
    FollowedHyperlink = 11,
    LAST = FollowedHyperlink
};

enum class ThemeColorUsage
{
    Unknown = 0,
    Text,
    Background
};

constexpr ThemeColorType convertToThemeColorType(sal_Int32 nIndex)
{
    if (nIndex < 0 || nIndex > 11)
        return ThemeColorType::Unknown;
    return static_cast<ThemeColorType>(nIndex);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
