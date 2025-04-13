/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/ChartColorPaletteType.hxx>
#include <svtools/valueset.hxx>

namespace chart
{
class ChartColorPalettes final : public ValueSet
{
    std::vector<ChartColorPalette> maColorSets;

public:
    ChartColorPalettes()
        : ValueSet(nullptr)
    {
    }

    void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    void UserDraw(const UserDrawEvent& rUserDrawEvent) override;
    void StyleUpdated() override;

    void insert(ChartColorPalette const& rColorSet);
    const ChartColorPalette* getPalette(sal_uInt32 nItem) const;
};
} // end namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
