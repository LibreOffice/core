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

#include <GridCoverPrimitive2D.hxx>
#include <sc_primitivetypes2d.hxx>

namespace sc
{
GridCoverPrimitive2D::GridCoverPrimitive2D(std::vector<Area>&& rAreas)
    : maAreas(std::move(rAreas))
{
}

bool GridCoverPrimitive2D::operator==(
    const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive) const
{
    if (!BufferedDecompositionPrimitive2D::operator==(rPrimitive))
        return false;

    return maAreas == static_cast<const GridCoverPrimitive2D&>(rPrimitive).maAreas;
}

basegfx::B2DRange GridCoverPrimitive2D::getB2DRange(
    const drawinglayer::geometry::ViewInformation2D& /*rViewInformation*/) const
{
    basegfx::B2DRange aRange;
    for (const Area& rArea : maAreas)
        aRange.expand(rArea.maRange);
    return aRange;
}

sal_uInt32 GridCoverPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_SC_GRIDCOVERPRIMITIVE2D;
}

drawinglayer::primitive2d::Primitive2DReference GridCoverPrimitive2D::create2DDecomposition(
    const drawinglayer::geometry::ViewInformation2D& /*rViewInformation*/) const
{
    return nullptr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
