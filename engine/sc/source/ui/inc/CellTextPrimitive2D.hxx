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

#include "CellTextFlags.hxx"
#include <address.hxx>
#include <basegfx/range/b2drange.hxx>
#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <tools/color.hxx>
#include <editeng/svxenum.hxx>

#include <optional>

namespace sc
{
/** The text of one cell, positioned in twips in the coordinates of the cell area.

    The clip range is the cell, the merged area, or wider when the text overflows into empty
    neighbors. Without a clip range the text is drawn whole. The horizontal alignment is already
    resolved from the context, so it is left, center, right or block, never standard. The text
    range is the area the glyphs of the text cover before clipping, or empty when it is not known
    and has to be measured from the text.
 */
class CellTextPrimitive2D final : public drawinglayer::primitive2d::BufferedDecompositionPrimitive2D
{
public:
    CellTextPrimitive2D(SCCOL nColumn, SCROW nRow, const basegfx::B2DRange& rCellRange,
                        const basegfx::B2DRange& rClipRange, bool bClip,
                        SvxCellHorJustify eHorizontal, SvxCellVerJustify eVertical,
                        CellTextFlags eFlags,
                        drawinglayer::primitive2d::Primitive2DContainer&& rText,
                        const basegfx::B2DRange& rTextRange = basegfx::B2DRange(),
                        const std::optional<Color>& roSurface = std::nullopt);

    SCCOL getColumn() const { return mnColumn; }
    SCROW getRow() const { return mnRow; }
    const basegfx::B2DRange& getCellRange() const { return maCellRange; }
    const basegfx::B2DRange& getClipRange() const { return maClipRange; }
    bool isClip() const { return mbClip; }
    SvxCellHorJustify getHorizontal() const { return meHorizontal; }
    SvxCellVerJustify getVertical() const { return meVertical; }
    CellTextFlags getFlags() const { return meFlags; }
    const drawinglayer::primitive2d::Primitive2DContainer& getText() const { return maText; }
    /// The fill of the cell under the text, when it has one, against which an automatic font
    /// color is chosen. Empty when the colors were chosen already or the cell has no fill.
    const std::optional<Color>& getSurface() const { return moSurface; }

    virtual bool
    operator==(const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive) const override;
    /// The range of the text, cut to the clip range when the text is clipped.
    virtual basegfx::B2DRange
    getB2DRange(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const override;
    virtual sal_uInt32 getPrimitive2DID() const override;

protected:
    virtual drawinglayer::primitive2d::Primitive2DReference create2DDecomposition(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation) const override;

private:
    SCCOL mnColumn;
    SCROW mnRow;
    basegfx::B2DRange maCellRange;
    basegfx::B2DRange maClipRange;
    bool mbClip;
    SvxCellHorJustify meHorizontal;
    SvxCellVerJustify meVertical;
    CellTextFlags meFlags;
    drawinglayer::primitive2d::Primitive2DContainer maText;
    basegfx::B2DRange maTextRange;
    std::optional<Color> moSurface;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
