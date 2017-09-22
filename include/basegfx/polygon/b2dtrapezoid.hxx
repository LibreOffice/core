/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_BASEGFX_POLYGON_B2DTRAPEZOID_HXX
#define INCLUDED_BASEGFX_POLYGON_B2DTRAPEZOID_HXX

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    // class to hold a single trapezoid
    class BASEGFX_DLLPUBLIC B2DTrapezoid
    {
    private:
        // Geometry data. YValues are down-oriented, this means bottom should
        // be bigger than top to be below it. The constructor implementation
        // guarantees:

        // - mfBottomY >= mfTopY
        // - mfTopXRight >= mfTopXLeft
        // - mfBottomXRight >= mfBottomXLeft
        double              mfTopXLeft;
        double              mfTopXRight;
        double              mfTopY;
        double              mfBottomXLeft;
        double              mfBottomXRight;
        double              mfBottomY;

    public:
        // constructor
        B2DTrapezoid(
            const double& rfTopXLeft,
            const double& rfTopXRight,
            const double& rfTopY,
            const double& rfBottomXLeft,
            const double& rfBottomXRight,
            const double& rfBottomY);

        // data read access
        const double& getTopXLeft() const { return mfTopXLeft; }
        const double& getTopXRight() const { return mfTopXRight; }
        const double& getTopY() const { return mfTopY; }
        const double& getBottomXLeft() const { return mfBottomXLeft; }
        const double& getBottomXRight() const { return mfBottomXRight; }
        const double& getBottomY() const { return mfBottomY; }

        // convenience method to get content as Polygon
        B2DPolygon getB2DPolygon() const;
    };

    typedef ::std::vector< B2DTrapezoid > B2DTrapezoidVector;

} // end of namespace basegfx


namespace basegfx
{
    namespace utils
    {
        // convert SourcePolyPolygon to trapezoids. The trapezoids will be appended to
        // ro_Result. ro_Result will not be cleared. If SourcePolyPolygon contains curves,
        // it's default AdaptiveSubdivision will be used.
        // CAUTION: Trapezoids are orientation-dependent in the sense that the upper and lower
        // lines have to be parallel to the X-Axis, thus this subdivision is NOT simply usable
        // for primitive decompositions. To use it, the shear and rotate parts of the
        // involved transformations HAVE to be taken into account.
        BASEGFX_DLLPUBLIC void trapezoidSubdivide(
            B2DTrapezoidVector& ro_Result,
            const B2DPolyPolygon& rSourcePolyPolygon);

        // directly create trapezoids from given edge. Depending on the given geometry,
        // none up to three trapezoids will be created
        BASEGFX_DLLPUBLIC void createLineTrapezoidFromEdge(
            B2DTrapezoidVector& ro_Result,
            const B2DPoint& rPointA,
            const B2DPoint& rPointB,
            double fLineWidth);

        // create trapezoids for all edges of the given polygon. The closed state of
        // the polygon is taken into account. If curves are contained, the default
        // AdaptiveSubdivision will be used.
        BASEGFX_DLLPUBLIC void createLineTrapezoidFromB2DPolygon(
            B2DTrapezoidVector& ro_Result,
            const B2DPolygon& rPolygon,
            double fLineWidth);

    } // end of namespace utils
} // end of namespace basegfx


#endif // INCLUDED_BASEGFX_POLYGON_B2DTRAPEZOID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
