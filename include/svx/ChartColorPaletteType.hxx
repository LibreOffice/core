/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <tools/long.hxx>
#include <tools/color.hxx>
#include <array>

constexpr OUString sUnoChartColorPalette = u".uno:ChartColorPalette"_ustr;

constexpr size_t ChartColorPaletteSize = 6;

typedef std::array<Color, ChartColorPaletteSize> ChartColorPalette;

enum class ChartColorPaletteType : sal_Int32
{
    Unknown = -1,
    Colorful = 0,
    Monochromatic = 1,
};

struct ChartColorPaletteLayout
{
    static constexpr tools::Long ItemSize = 24;
    static constexpr tools::Long ItemBorder = 4;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
