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

#include <SparklinePrimitive2D.hxx>
#include <sc_primitivetypes2d.hxx>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>

namespace sc
{
SparklinePrimitive2D::SparklinePrimitive2D(
    SCCOL nColumn, SCROW nRow, const basegfx::B2DRange& rCellRange, OString aGroupId,
    drawinglayer::primitive2d::Primitive2DContainer&& rContent)
    : mnColumn(nColumn)
    , mnRow(nRow)
    , maCellRange(rCellRange)
    , maGroupId(std::move(aGroupId))
    , maContent(std::move(rContent))
{
}

bool SparklinePrimitive2D::operator==(
    const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive) const
{
    if (!BufferedDecompositionPrimitive2D::operator==(rPrimitive))
        return false;

    const auto& rOther = static_cast<const SparklinePrimitive2D&>(rPrimitive);
    return mnColumn == rOther.mnColumn && mnRow == rOther.mnRow && maCellRange == rOther.maCellRange
           && maGroupId == rOther.maGroupId && maContent == rOther.maContent;
}

sal_uInt32 SparklinePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_SC_SPARKLINEPRIMITIVE2D;
}

drawinglayer::primitive2d::Primitive2DReference SparklinePrimitive2D::create2DDecomposition(
    const drawinglayer::geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (maContent.empty())
        return nullptr;
    return new drawinglayer::primitive2d::GroupPrimitive2D(
        drawinglayer::primitive2d::Primitive2DContainer(maContent));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
