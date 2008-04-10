/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dpolypolygontools.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#define _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>

#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // predefinitions
    class B3DPolyPolygon;
    class B3DRange;

    namespace tools
    {
        // B3DPolyPolygon tools

        // get size of PolyPolygon. Control vectors are included in that ranges.
        ::basegfx::B3DRange getRange(const ::basegfx::B3DPolyPolygon& rCandidate);

        // Apply Line Dashing. This cuts every contained PolyPolygon into line pieces
        // which are inserted as single polygons into the result.
        // If fFullDashDotLen is not given it will be calculated from the given
        // raDashDotArray.
        ::basegfx::B3DPolyPolygon applyLineDashing(const ::basegfx::B3DPolyPolygon& rCandidate, const ::std::vector<double>& raDashDotArray, double fFullDashDotLen = 0.0);

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX */
