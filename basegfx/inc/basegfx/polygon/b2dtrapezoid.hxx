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
#include <basegfx/polygon/b2dPolypolygon.hxx>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // class to hold a single trapezoid
    class B2DTrapezoid
    {
    private:
        // geometry data
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
        // convert SourcePolyPolygon to trapezoids
        B2DTrapezoidVector trapezoidSubdivide(const B2DPolyPolygon& rSourcePolyPolygon);

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_POLYGON_B2DTRAPEZOID_HXX */
