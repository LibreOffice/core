/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "CellMarkKind.hxx"
#include <address.hxx>
#include <basegfx/range/b2drange.hxx>
#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <tools/color.hxx>

namespace sc
{
/** A mark on one cell, such as the formula indicator or the drop-down button of an autofilter.

    The cell range is in twips. The mark size is in pixels, and the pixel size is the twips per
    pixel at the drawing zoom. The error mark grows by a pixel for each zoom step of 1.2, and the
    other marks stay the same on screen at every zoom. The active state means an autofilter button
    has a filter or a sort applied.
 */
class CellMarkPrimitive2D final : public drawinglayer::primitive2d::BufferedDecompositionPrimitive2D
{
public:
    CellMarkPrimitive2D(CellMarkKind eKind, SCCOL nColumn, SCROW nRow,
                        const basegfx::B2DRange& rCellRange, const Color& rColor, double fPixelSize,
                        bool bActive = false);

    CellMarkKind getKind() const { return meKind; }
    SCCOL getColumn() const { return mnColumn; }
    SCROW getRow() const { return mnRow; }
    const basegfx::B2DRange& getCellRange() const { return maCellRange; }
    const Color& getColor() const { return maColor; }
    double getPixelSize() const { return mfPixelSize; }
    bool isActive() const { return mbActive; }

    virtual bool
    operator==(const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive) const override;
    virtual sal_uInt32 getPrimitive2DID() const override;

protected:
    virtual drawinglayer::primitive2d::Primitive2DReference create2DDecomposition(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation) const override;

private:
    CellMarkKind meKind;
    SCCOL mnColumn;
    SCROW mnRow;
    basegfx::B2DRange maCellRange;
    Color maColor;
    double mfPixelSize;
    bool mbActive;
    /// The fill, the outline and the arrow of a drop-down button, from the style settings at the
    /// time the mark was made. They are COL_TRANSPARENT for the other kinds of marks.
    Color maButtonFace;
    Color maButtonOutline;
    Color maButtonArrow;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
