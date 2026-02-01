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
#include <basegfx/utils/bgradient.hxx>
#include <vector>

constexpr OUString sUnoChartGradientPalette = u".uno:ChartGradientPalette"_ustr;
constexpr ::Color ChartGradientPresetDefaultColor(0x14, 0x5F, 0x82); // dark blue
constexpr ::Color ChartGradientPresetInvalidColor(ColorTransparency, 0xFF, 0x00, 0x00, 0x00);

typedef std::vector<basegfx::BGradient> ChartGradientPalette;

enum class ChartGradientVariation
{
    Unknown,
    LightVariation,
    DarkVariation,
};

enum class ChartGradientType
{
    TopLeftToBottomRight = 0,
    LinearDown,
    TopRightToBottomLeft,
    FromBottomRightCorner,
    FromBottomLeftCorner,
    LinearRight,
    FromCenter,
    LinearLeft,
    FromTopRightCorner,
    FromTopLeftCorner,
    BottomLeftToTopRight,
    LinearUp,
    BottomRightToTopLeft,
    LAST = BottomRightToTopLeft,
    Invalid = LAST + 1
};

struct ChartGradientProperties
{
    css::awt::GradientStyle style;
    sal_Int16 angle;
    sal_uInt16 offsetX;
    sal_uInt16 offsetY;
};

struct ChartGradientPaletteLayout
{
    static constexpr tools::Long ItemSize = 48;
    static constexpr tools::Long ItemBorder = 2;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
