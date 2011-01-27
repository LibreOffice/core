/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
#define _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // Solve all crossovers in a polyPolygon. This re-layouts all contained polygons so that the
        // result will contain only non-cutting polygons. For that reason, points will be added at
        // crossover and touch points and the single Polygons may be re-combined. The orientations
        // of the contained polygons in not changed but used as topological information.
        // Self crossovers of the contained sub-polygons are implicitely handled, but to not lose
        // the topological information, it may be necessary to remove self-intersections of the
        // contained sub-polygons in a preparing step and to explicitely correct their orientations.
        B2DPolyPolygon solveCrossovers(const B2DPolyPolygon& rCandidate);

        // Version for single polygons. This is for solving self-intersections. Result will be free of
        // crossovers. When result contains multiple polygons, it may be necessary to rearrange their
        // orientations since holes may have been created (use correctOrientations eventually).
        B2DPolyPolygon solveCrossovers(const B2DPolygon& rCandidate);

        // Neutral polygons will be stripped. Neutral polygons are ones who's orientation is
        // neutral, so normally they have no volume -> just closed paths. A polygon with the same
        // positive and negative oriented volume is also neutral, so this may not be wanted. It is
        // safe to call with crossover-free polygons, though (that's where it's mostly used).
        B2DPolyPolygon stripNeutralPolygons(const B2DPolyPolygon& rCandidate);

        // Remove not necessary polygons. Works only correct with crossover-free polygons. For each
        // polygon, the depth for the PolyPolygon is calculated. The orientation is used to identify holes.
        // Start value for holes is -1, for polygons it's zero. Ech time a polygon is contained in another one,
        // it's depth is increased when inside a polygon, decreased when inside a hole. The result is a depth
        // which e.g. is -1 for holes outside everything, 1 for a polygon covered by another polygon and zero
        // for e.g. holes in a polygon or polygons outside everythig else.
        // In the 2nd step, all polygons with depth other than zero are removed. If bKeepAboveZero is used,
        // all polygons < 1 are removed. The bKeepAboveZero mode is useful for clipping, e.g. just append
        // one polygon to another and use this mode -> only parts where two polygons overlapped will be kept.
        // In combination with correct orientation of the input orientations and the SolveCrossover calls this
        // can be combined for logical polygon operations or polygon clipping.
        B2DPolyPolygon stripDispensablePolygons(const B2DPolyPolygon& rCandidate, bool bKeepAboveZero = false);

        // For convenience: The four basic operations OR, XOR, AND and DIFF for
        // two PolyPolygons. These are combinations of the above methods. To not be forced
        // to do evtl. already done preparations twice, You have to do the operations Yourself.
        //
        // A source preparation consists of preparing it to be seen as XOR-Rule PolyPolygon,
        // so it is freed of intersections, self-intersections and the orientations are corrected.
        // Important is that it will define the same areas as before, but is intersection-free.
        // As an example think about a single polygon looping in itself and having holes. To
        // topologically correctly handle this, it is necessary to remove all intersections and
        // to correct the orientations. The orientation of the isolated holes e.g. will be negative.
        // Topologically it is necessary to prepare each polygon which is seen as entity. It is
        // not sufficient just to concatenate them and prepare the result, this may be topologically
        // different since the simple concatenation will be seen as XOR. To work correctly, You
        // may need to OR those polygons.

        // Preparations: solve self-intersections and intersections, remove neutral
        // parts and correct orientations.
        B2DPolyPolygon prepareForPolygonOperation(const B2DPolygon& rCandidate);
        B2DPolyPolygon prepareForPolygonOperation(const B2DPolyPolygon& rCandidate);

        // OR: Return all areas where CandidateA or CandidateB exist
        B2DPolyPolygon solvePolygonOperationOr(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB);

        // XOR: Return all areas where CandidateA or CandidateB exist, but not both
        B2DPolyPolygon solvePolygonOperationXor(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB);

        // AND: Return all areas where CandidateA and CandidateB exist
        B2DPolyPolygon solvePolygonOperationAnd(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB);

        // DIFF: Return all areas where CandidateA is not covered by CandidateB (cut B out of A)
        B2DPolyPolygon solvePolygonOperationDiff(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB);

        /** merge all single PolyPolygons to a single, OR-ed PolyPolygon

            @param rInput
            The source PolyPolygons

            @return A single PolyPolygon containing the Or-merged result
        */
        B2DPolyPolygon mergeToSinglePolyPolygon(const std::vector< basegfx::B2DPolyPolygon >& rInput);

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////


#endif /* _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
