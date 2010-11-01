/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b2dmultirange.hxx,v $
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

#ifndef _BGFX_RANGE_B2DRANGECLIPPER_HXX
#define _BGFX_RANGE_B2DRANGECLIPPER_HXX

#include <basegfx/range/b2dpolyrange.hxx>
#include <vector>

namespace basegfx
{
    namespace tools
    {
        /** Extract poly-polygon w/o self-intersections from poly-range

            Similar to the solveCrossovers(const B2DPolyPolygon&)
            method, this one calculates a self-intersection-free
            poly-polygon with the same topology, and encoding
            inside/outsidedness via polygon orientation and layering.
         */
        B2DPolyPolygon solveCrossovers(const std::vector<B2DRange>& rRanges,
                                       const std::vector<B2VectorOrientation>& rOrientations);
    }
}

#endif /* _BGFX_RANGE_B2DRANGECLIPPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
