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

#include <svx/ChartColorPaletteType.hxx>
#include <svtools/valueset.hxx>

namespace chart
{
class OOO_DLLPUBLIC_CHARTTOOLS ChartColorPalettes final : public ValueSet
{
public:
    typedef Link<const MouseEvent&, void> MouseEventHandler;

private:
    std::vector<ChartColorPalette> maColorSets;
    MouseEventHandler maMouseMoveHdl;

public:
    ChartColorPalettes()
        : ValueSet(nullptr)
    {
    }

    void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    void UserDraw(const UserDrawEvent& rUserDrawEvent) override;
    void StyleUpdated() override;
    bool MouseMove(const MouseEvent& rMEvt) override;

    void insert(ChartColorPalette const& rColorSet);
    const ChartColorPalette* getPalette(sal_uInt32 nItem) const;
    void setMouseMoveHdl(const MouseEventHandler& rLink);
};
} // end namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
