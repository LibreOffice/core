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

#ifndef INCLUDED_BASEGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
#define INCLUDED_BASEGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    namespace utils
    {
        /** Solve all crossovers (aka self-intersections) in a polyPolygon.

            This re-layouts all contained polygons so that the result
            will contain only non-cutting polygons. For that reason,
            points will be added at crossover and touch points and the
            single Polygons may be re-combined. The orientations of
            the contained polygons in not changed but used as
            topological information.  Self crossovers of the contained
            sub-polygons are implicitly handled, but to not lose the
            topological information, it may be necessary to remove
            self-intersections of the contained sub-polygons in a
            preparing step and to explicitly correct their
            orientations.
        */
        BASEGFX_DLLPUBLIC B2DPolyPolygon solveCrossovers(const B2DPolyPolygon& rCandidate);

        /** Solve all crossovers (aka self-intersections) in a Polygon

            Same as above, but for single polygons. Result will be
            free of self-intersections. When result contains multiple
            polygons, it may be necessary to rearrange their
            orientations since holes may have been created (possibly use
            correctOrientations).
        */
        BASEGFX_DLLPUBLIC B2DPolyPolygon solveCrossovers(const B2DPolygon& rCandidate);

        /** Strip neutral polygons from PolyPolygon.

            Neutral polygons are ones who's orientation is neutral, so
            normally they have no volume -> just closed paths. A
            polygon with the same positive and negative oriented
            volume is also neutral, so this may not be wanted. It is
            safe to call with self-intersection-free polygons, though
            (that's where it's mostly used).
        */
        BASEGFX_DLLPUBLIC B2DPolyPolygon stripNeutralPolygons(const B2DPolyPolygon& rCandidate);

        /** Remove unnecessary/non-displayed polygons.

            Works only correct with self-intersection-free
            polygons. For each polygon, the depth for the PolyPolygon
            is calculated. The orientation is used to identify holes.
            Start value for holes is -1, for polygons it's zero. Ech
            time a polygon is contained in another one, it's depth is
            increased when inside a polygon, decreased when inside a
            hole. The result is a depth which e.g. is -1 for holes
            outside everything, 1 for a polygon covered by another
            polygon and zero for e.g. holes in a polygon or polygons
            outside everything else.  In the 2nd step, all polygons
            with depth other than zero are removed. If bKeepAboveZero
            is used, all polygons < 1 are removed. The bKeepAboveZero
            mode is useful for clipping, e.g. just append one polygon
            to another and use this mode -> only parts where two
            polygons overlapped will be kept.  In combination with
            correct orientation of the input orientations and the
            SolveCrossover calls this can be combined for logical
            polygon operations or polygon clipping.
        */
        BASEGFX_DLLPUBLIC B2DPolyPolygon stripDispensablePolygons(const B2DPolyPolygon& rCandidate, bool bKeepAboveZero = false);

        /** Emulate nonzero winding rule filling.

            Geometrically convert PolyPolygons which are proposed to
            use nonzero fill rule to a representation where evenodd
            paint will give the same result. To do this all
            intersections and self-intersections get solved (the
            polygons will be rearranged if needed). Then all polygons
            which are inside another one with the same orientation get
            deleted
        */
        BASEGFX_DLLPUBLIC B2DPolyPolygon createNonzeroConform(const B2DPolyPolygon& rCandidate);

        // For convenience: The four basic operations OR, XOR, AND and DIFF for
        // two PolyPolygons. These are combinations of the above methods. To not be forced
        // to do evtl. already done preparations twice, You have to do the operations Yourself.

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

        /// prep for ops - solve self-intersections and intersections, remove neutral parts and check orientations.
        BASEGFX_DLLPUBLIC B2DPolyPolygon prepareForPolygonOperation(const B2DPolygon& rCandidate);
        /// prep for ops - solve self-intersections and intersections, remove neutral parts and check orientations.
        BASEGFX_DLLPUBLIC B2DPolyPolygon prepareForPolygonOperation(const B2DPolyPolygon& rCandidate);

        /// OR: Return all areas where CandidateA or CandidateB exist
        BASEGFX_DLLPUBLIC B2DPolyPolygon solvePolygonOperationOr(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB);

        /// XOR: Return all areas where CandidateA or CandidateB exist, but not both
        BASEGFX_DLLPUBLIC B2DPolyPolygon solvePolygonOperationXor(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB);

        /// AND: Return all areas where CandidateA and CandidateB exist
        BASEGFX_DLLPUBLIC B2DPolyPolygon solvePolygonOperationAnd(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB);

        /// DIFF: Return all areas where CandidateA is not covered by CandidateB (cut B out of A)
        BASEGFX_DLLPUBLIC B2DPolyPolygon solvePolygonOperationDiff(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB);

        /** merge all single PolyPolygons to a single, OR-ed PolyPolygon

            @param rInput
            The source PolyPolygons

            @return A single tools::PolyPolygon containing the Or-merged result
        */
        BASEGFX_DLLPUBLIC B2DPolyPolygon mergeToSinglePolyPolygon(const B2DPolyPolygonVector& rInput);

    } // end of namespace utils
} // end of namespace basegfx


#endif // INCLUDED_BASEGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
