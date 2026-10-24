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

#include <CellTextPrimitive2D.hxx>
#include <sc_primitivetypes2d.hxx>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>

namespace sc
{
CellTextPrimitive2D::CellTextPrimitive2D(SCCOL nColumn, SCROW nRow,
                                         const basegfx::B2DRange& rCellRange,
                                         const basegfx::B2DRange& rClipRange, bool bClip,
                                         SvxCellHorJustify eHorizontal, SvxCellVerJustify eVertical,
                                         CellTextFlags eFlags,
                                         drawinglayer::primitive2d::Primitive2DContainer&& rText,
                                         const basegfx::B2DRange& rTextRange,
                                         const std::optional<Color>& roSurface)
    : mnColumn(nColumn)
    , mnRow(nRow)
    , maCellRange(rCellRange)
    , maClipRange(rClipRange)
    , mbClip(bClip)
    , meHorizontal(eHorizontal)
    , meVertical(eVertical)
    , meFlags(eFlags)
    , maText(std::move(rText))
    , maTextRange(rTextRange)
    , moSurface(roSurface)
{
}

bool CellTextPrimitive2D::operator==(
    const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive) const
{
    if (!BufferedDecompositionPrimitive2D::operator==(rPrimitive))
        return false;

    const auto& rOther = static_cast<const CellTextPrimitive2D&>(rPrimitive);
    return mnColumn == rOther.mnColumn && mnRow == rOther.mnRow && maCellRange == rOther.maCellRange
           && maClipRange == rOther.maClipRange && mbClip == rOther.mbClip
           && meHorizontal == rOther.meHorizontal && meVertical == rOther.meVertical
           && meFlags == rOther.meFlags && maText == rOther.maText
           && maTextRange == rOther.maTextRange;
}

basegfx::B2DRange CellTextPrimitive2D::getB2DRange(
    const drawinglayer::geometry::ViewInformation2D& rViewInformation) const
{
    basegfx::B2DRange aRange(maTextRange.isEmpty() ? maText.getB2DRange(rViewInformation)
                                                   : maTextRange);
    if (mbClip)
        aRange.intersect(maClipRange);
    return aRange;
}

sal_uInt32 CellTextPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_SC_CELLTEXTPRIMITIVE2D;
}

drawinglayer::primitive2d::Primitive2DReference CellTextPrimitive2D::create2DDecomposition(
    const drawinglayer::geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (maText.empty())
        return nullptr;

    // Text that is not clipped is its single portion as it is, without a group around it.
    if (!mbClip && maText.size() == 1)
        return maText[0];

    drawinglayer::primitive2d::Primitive2DContainer aText(maText);
    if (!mbClip)
        return new drawinglayer::primitive2d::GroupPrimitive2D(std::move(aText));

    return new drawinglayer::primitive2d::MaskPrimitive2D(
        basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(maClipRange)),
        std::move(aText));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
