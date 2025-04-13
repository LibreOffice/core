/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/svxdllapi.h>
#include <svx/ChartColorPaletteType.hxx>
#include <docmodel/theme/Theme.hxx>
#include <tools/gen.hxx>

#include <array>

class OutputDevice;

namespace chart
{
class ChartColorPaletteHelper
{
public:
    static constexpr size_t ColorfulPaletteSize = 4;
    static constexpr size_t MonotonicPaletteSize = 6;

    explicit ChartColorPaletteHelper(const std::shared_ptr<model::Theme>& pTheme);

    ChartColorPalette getColorPalette(ChartColorPaletteType eType, sal_uInt32 nIndex) const;
    static void renderColorPalette(OutputDevice* pDev, const tools::Rectangle& rDrawArea,
                                   const ChartColorPalette& rColorSet,
                                   bool bDrawItemBorder = false);
    static void renderNoPalette(OutputDevice* pDev, const tools::Rectangle& rDrawArea);

private:
    void createBasePaletteFromTheme(const std::shared_ptr<model::Theme>& pTheme);
    ChartColorPalette createColorfulPalette(sal_uInt32 nIndex) const;
    ChartColorPalette createColorfulPaletteImpl(size_t nIdx1, size_t nIdx2, size_t nIdx3) const;
    ChartColorPalette createMonotonicPalette(sal_uInt32 nIndex) const;

    ChartColorPalette mBasePalette;
};
} // end namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
