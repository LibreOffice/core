/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/primitive2d/ViewDependentTools.hxx>
#include <basegfx/utils/tools.hxx>

namespace drawinglayer::primitive2d
{
void calculateDiscreteVisibleRanges(const geometry::ViewInformation2D& rViewInformation,
                                    const basegfx::B2DHomMatrix& rObjectTransformation,
                                    basegfx::B2DRange& rDiscreteRange,
                                    basegfx::B2DRange& rVisibleDiscreteRange,
                                    basegfx::B2DRange& rUnitVisibleRange)
{
    // use unit range and transform to discrete coordinates
    rDiscreteRange = basegfx::B2DRange(0.0, 0.0, 1.0, 1.0);
    rDiscreteRange.transform(rViewInformation.getObjectToViewTransformation()
                             * rObjectTransformation);

    // clip against discrete viewport (if set)
    rVisibleDiscreteRange = rDiscreteRange;

    if (!rViewInformation.getViewport().isEmpty())
    {
        rVisibleDiscreteRange.intersect(rViewInformation.getDiscreteViewport());
    }

    if (rVisibleDiscreteRange.isEmpty())
    {
        rUnitVisibleRange = rVisibleDiscreteRange;
    }
    else
    {
        // create UnitVisibleRange with values [0.0 .. 1.0] describing
        // the relative position of rVisibleDiscreteRange inside rDiscreteRange
        const double fDiscreteScaleFactorX(basegfx::fTools::equalZero(rDiscreteRange.getWidth())
                                               ? 1.0
                                               : 1.0 / rDiscreteRange.getWidth());
        const double fDiscreteScaleFactorY(basegfx::fTools::equalZero(rDiscreteRange.getHeight())
                                               ? 1.0
                                               : 1.0 / rDiscreteRange.getHeight());

        const double fMinX(
            basegfx::fTools::equal(rVisibleDiscreteRange.getMinX(), rDiscreteRange.getMinX())
                ? 0.0
                : (rVisibleDiscreteRange.getMinX() - rDiscreteRange.getMinX())
                      * fDiscreteScaleFactorX);
        const double fMinY(
            basegfx::fTools::equal(rVisibleDiscreteRange.getMinY(), rDiscreteRange.getMinY())
                ? 0.0
                : (rVisibleDiscreteRange.getMinY() - rDiscreteRange.getMinY())
                      * fDiscreteScaleFactorY);

        const double fMaxX(
            basegfx::fTools::equal(rVisibleDiscreteRange.getMaxX(), rDiscreteRange.getMaxX())
                ? 1.0
                : (rVisibleDiscreteRange.getMaxX() - rDiscreteRange.getMinX())
                      * fDiscreteScaleFactorX);
        const double fMaxY(
            basegfx::fTools::equal(rVisibleDiscreteRange.getMaxY(), rDiscreteRange.getMaxY())
                ? 1.0
                : (rVisibleDiscreteRange.getMaxY() - rDiscreteRange.getMinY())
                      * fDiscreteScaleFactorY);

        rUnitVisibleRange = basegfx::B2DRange(fMinX, fMinY, fMaxX, fMaxY);
    }
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
