/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b2dpolygontriangulator.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BGFX_POLYGON_B2DTRAPEZOID_HXX
#define _BGFX_POLYGON_B2DTRAPEZOID_HXX

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // class to hold a single trapezoid
    class B2DTrapezoid
    {
    private:
        // Geometry data. YValues are down-oriented, this means bottom should
        // be bigger than top to be below it. The constructor implementation
        // guarantees:
        //
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

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // convert SourcePolyPolygon to trapezoids. The trapezoids will be appended to
        // ro_Result. ro_Result will not be cleared. If SourcePolyPolygon contains curves,
        // it's default AdaptiveSubdivision will be used.
        // CAUTION: Trapezoids are oreintation-dependent in the sense that the upper and lower
        // lines have to be parallel to the X-Axis, thus this subdivision is NOT simply usable
        // for primitive decompositions. To use it, the shear and rotate parts of the
        // involved transformations HAVE to be taken into account.
        void trapezoidSubdivide(
            B2DTrapezoidVector& ro_Result,
            const B2DPolyPolygon& rSourcePolyPolygon);

        // directly create trapezoids from given edge. Depending on the given geometry,
        // none up to three trapezoids will be created
        void createLineTrapezoidFromEdge(
            B2DTrapezoidVector& ro_Result,
            const B2DPoint& rPointA,
            const B2DPoint& rPointB,
            double fLineWidth = 1.0);

        // create trapezoids for all edges of the given polygon. The closed state of
        // the polygon is taken into account. If curves are contaned, the default
        // AdaptiveSubdivision will be used.
        void createLineTrapezoidFromB2DPolygon(
            B2DTrapezoidVector& ro_Result,
            const B2DPolygon& rPolygon,
            double fLineWidth = 1.0);

        // create trapezoids for all edges of the given polyPolygon. The closed state of
        // the PolyPolygon is taken into account. If curves are contaned, the default
        // AdaptiveSubdivision will be used.
        void createLineTrapezoidFromB2DPolyPolygon(
            B2DTrapezoidVector& ro_Result,
            const B2DPolyPolygon& rPolyPolygon,
            double fLineWidth = 1.0);

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_POLYGON_B2DTRAPEZOID_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
