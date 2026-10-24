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

#include <address.hxx>
#include <basegfx/range/b2drange.hxx>
#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <rtl/string.hxx>

namespace sc
{
/** The sparkline of one cell. The group id is the GUID of its sparkline group. The lines, columns
    and markers are in twips, inside the cell range.
 */
class SparklinePrimitive2D final
    : public drawinglayer::primitive2d::BufferedDecompositionPrimitive2D
{
public:
    SparklinePrimitive2D(SCCOL nColumn, SCROW nRow, const basegfx::B2DRange& rCellRange,
                         OString aGroupId,
                         drawinglayer::primitive2d::Primitive2DContainer&& rContent);

    SCCOL getColumn() const { return mnColumn; }
    SCROW getRow() const { return mnRow; }
    const basegfx::B2DRange& getCellRange() const { return maCellRange; }
    const OString& getGroupId() const { return maGroupId; }

    virtual bool
    operator==(const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive) const override;
    virtual sal_uInt32 getPrimitive2DID() const override;

protected:
    virtual drawinglayer::primitive2d::Primitive2DReference create2DDecomposition(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation) const override;

private:
    SCCOL mnColumn;
    SCROW mnRow;
    basegfx::B2DRange maCellRange;
    OString maGroupId;
    drawinglayer::primitive2d::Primitive2DContainer maContent;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
