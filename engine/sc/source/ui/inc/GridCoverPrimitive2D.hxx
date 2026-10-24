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

#include <vector>

namespace sc
{
/** The areas of a cell block without grid lines: merged cells, and empty cells that the text of a
    neighbor overflows into. Each area is given as a cell range and a rectangle in twips. The grid
    is not part of the content, so this primitive draws nothing.
 */
class GridCoverPrimitive2D final
    : public drawinglayer::primitive2d::BufferedDecompositionPrimitive2D
{
public:
    struct Area
    {
        SCCOL mnStartColumn;
        SCROW mnStartRow;
        SCCOL mnEndColumn;
        SCROW mnEndRow;
        basegfx::B2DRange maRange;

        bool operator==(const Area&) const = default;
    };

    explicit GridCoverPrimitive2D(std::vector<Area>&& rAreas);

    const std::vector<Area>& getAreas() const { return maAreas; }

    virtual bool
    operator==(const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive) const override;
    virtual basegfx::B2DRange
    getB2DRange(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const override;
    virtual sal_uInt32 getPrimitive2DID() const override;

protected:
    virtual drawinglayer::primitive2d::Primitive2DReference create2DDecomposition(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation) const override;

private:
    std::vector<Area> maAreas;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
