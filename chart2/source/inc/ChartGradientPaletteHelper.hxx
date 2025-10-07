/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "charttoolsdllapi.hxx"
#include <svx/svxdllapi.h>
#include <svx/ChartGradientVariation.hxx>
#include <docmodel/theme/Theme.hxx>
#include <tools/gen.hxx>

#include <array>

class OutputDevice;

namespace chart
{
class OOO_DLLPUBLIC_CHARTTOOLS ChartGradientPaletteHelper
{
public:
    static constexpr size_t PaletteSize = 6;

    ChartGradientPaletteHelper(const std::vector<Color>& aColorSet);

    basegfx::BGradient getGradientSample(ChartGradientVariation eVariation,
                                         ChartGradientType eType) const;
    ChartGradientPalette getGradientPalette(ChartGradientVariation eVariation,
                                            ChartGradientType eType) const;

    static void renderGradientItem(OutputDevice* pDev, const tools::Rectangle& rDrawArea,
                                   const basegfx::BGradient& rGradient,
                                   bool bDrawItemBorder = false);
    static void renderNoGradient(OutputDevice* pDev, const tools::Rectangle& rDrawArea);

private:
    const Color& getSampleColor() const;
    static basegfx::BGradient createItem(const basegfx::BColorStops& rColorStops,
                                         ChartGradientType eType);

    std::vector<Color> maColorSet;
};

} // end namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
