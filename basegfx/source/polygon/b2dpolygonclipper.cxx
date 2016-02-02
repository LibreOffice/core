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

#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <osl/diagnose.h>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/tools/rectcliptools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

namespace basegfx
{
    namespace tools
    {
        B2DPolyPolygon clipPolygonOnParallelAxis(const B2DPolygon& rCandidate, bool bParallelToXAxis, bool bAboveAxis, double fValueOnOtherAxis, bool bStroke)
        {
            B2DPolyPolygon aRetval;

            if(rCandidate.count())
            {
                const B2DRange aCandidateRange(getRange(rCandidate));

                if(bParallelToXAxis && fTools::moreOrEqual(aCandidateRange.getMinY(), fValueOnOtherAxis))
                {
                    // completely above and on the clip line. also true for curves.
                    if(bAboveAxis)
                    {
                        // add completely
                        aRetval.append(rCandidate);
                    }
                }
                else if(bParallelToXAxis && fTools::lessOrEqual(aCandidateRange.getMaxY(), fValueOnOtherAxis))
                {
                    // completely below and on the clip line. also true for curves.
                    if(!bAboveAxis)
                    {
                        // add completely
                        aRetval.append(rCandidate);
                    }
                }
                else if(!bParallelToXAxis && fTools::moreOrEqual(aCandidateRange.getMinX(), fValueOnOtherAxis))
                {
                    // completely right of and on the clip line. also true for curves.
                    if(bAboveAxis)
                    {
                        // add completely
                        aRetval.append(rCandidate);
                    }
                }
                else if(!bParallelToXAxis && fTools::lessOrEqual(aCandidateRange.getMaxX(), fValueOnOtherAxis))
                {
                    // completely left of and on the clip line. also true for curves.
                    if(!bAboveAxis)
                    {
                        // add completely
                        aRetval.append(rCandidate);
                    }
                }
                else
                {
                    // add cuts with axis to polygon, including bezier segments
                    // Build edge to cut with. Make it a little big longer than needed for
                    // numerical stability. We want to cut against the edge seen as endless
                    // ray here, but addPointsAtCuts() will limit itself to the
                    // edge's range ]0.0 .. 1.0[.
                    const double fSmallExtension((aCandidateRange.getWidth() + aCandidateRange.getHeight()) * (0.5 * 0.1));
                    const B2DPoint aStart(
                        bParallelToXAxis ? aCandidateRange.getMinX() - fSmallExtension : fValueOnOtherAxis,
                        bParallelToXAxis ? fValueOnOtherAxis : aCandidateRange.getMinY() - fSmallExtension);
                    const B2DPoint aEnd(
                        bParallelToXAxis ? aCandidateRange.getMaxX() + fSmallExtension : fValueOnOtherAxis,
                        bParallelToXAxis ? fValueOnOtherAxis : aCandidateRange.getMaxY() + fSmallExtension);
                    const B2DPolygon aCandidate(addPointsAtCuts(rCandidate, aStart, aEnd));
                    const sal_uInt32 nPointCount(aCandidate.count());
                    const sal_uInt32 nEdgeCount(aCandidate.isClosed() ? nPointCount : nPointCount - 1L);
                    B2DCubicBezier aEdge;
                    B2DPolygon aRun;

                    for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                    {
                        aCandidate.getBezierSegment(a, aEdge);
                        const B2DPoint aTestPoint(aEdge.interpolatePoint(0.5));
                        const bool bInside(bParallelToXAxis ?
                            fTools::moreOrEqual(aTestPoint.getY(), fValueOnOtherAxis) == bAboveAxis :
                            fTools::moreOrEqual(aTestPoint.getX(), fValueOnOtherAxis) == bAboveAxis);

                        if(bInside)
                        {
                            if(!aRun.count() || !aRun.getB2DPoint(aRun.count() - 1).equal(aEdge.getStartPoint()))
                            {
                                aRun.append(aEdge.getStartPoint());
                            }

                            if(aEdge.isBezier())
                            {
                                aRun.appendBezierSegment(aEdge.getControlPointA(), aEdge.getControlPointB(), aEdge.getEndPoint());
                            }
                            else
                            {
                                aRun.append(aEdge.getEndPoint());
                            }
                        }
                        else
                        {
                            if(bStroke && aRun.count())
                            {
                                aRetval.append(aRun);
                                aRun.clear();
                            }
                        }
                    }

                    if(aRun.count())
                    {
                        if(bStroke)
                        {
                            // try to merge this last and first polygon; they may have been
                            // the former polygon's start/end point
                            if(aRetval.count())
                            {
                                const B2DPolygon aStartPolygon(aRetval.getB2DPolygon(0));

                                if(aStartPolygon.count() && aStartPolygon.getB2DPoint(0).equal(aRun.getB2DPoint(aRun.count() - 1)))
                                {
                                    // append start polygon to aRun, remove from result set
                                    aRun.append(aStartPolygon); aRun.removeDoublePoints();
                                    aRetval.remove(0);
                                }
                            }

                            aRetval.append(aRun);
                        }
                        else
                        {
                            // set closed flag and correct last point (which is added double now).
                            closeWithGeometryChange(aRun);
                            aRetval.append(aRun);
                        }
                    }
                }
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolyPolygonOnParallelAxis(const B2DPolyPolygon& rCandidate, bool bParallelToXAxis, bool bAboveAxis, double fValueOnOtherAxis, bool bStroke)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                const B2DPolyPolygon aClippedPolyPolygon(clipPolygonOnParallelAxis(rCandidate.getB2DPolygon(a), bParallelToXAxis, bAboveAxis, fValueOnOtherAxis, bStroke));

                if(aClippedPolyPolygon.count())
                {
                    aRetval.append(aClippedPolyPolygon);
                }
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolygonOnRange(const B2DPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke)
        {
            const sal_uInt32 nCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            if(!nCount)
            {
                // source is empty
                return aRetval;
            }

            if(rRange.isEmpty())
            {
                if(bInside)
                {
                    // nothing is inside an empty range
                    return aRetval;
                }
                else
                {
                    // everything is outside an empty range
                    return B2DPolyPolygon(rCandidate);
                }
            }

            const B2DRange aCandidateRange(getRange(rCandidate));

            if(rRange.isInside(aCandidateRange))
            {
                  // candidate is completely inside given range
                if(bInside)
                {
                    // nothing to do
                    return B2DPolyPolygon(rCandidate);
                }
                else
                {
                    // nothing is outside, then
                    return aRetval;
                }
            }

            if(!bInside)
            {
                // cutting off the outer parts of filled polygons at parallel
                // lines to the axes is only possible for the inner part, not for
                // the outer part which means cutting a hole into the original polygon.
                // This is because the inner part is a logical AND-operation of
                // the four implied half-planes, but the outer part is not.
                // It is possible for strokes, but with creating unnecessary extra
                // cuts, so using clipPolygonOnPolyPolygon is better there, too.
                // This needs to be done with the topology knowlegde and is unfurtunately
                // more expensive, too.
                const B2DPolygon aClip(createPolygonFromRect(rRange));

                return clipPolygonOnPolyPolygon(rCandidate, B2DPolyPolygon(aClip), bInside, bStroke);
            }

            // clip against the four axes of the range
            // against X-Axis, lower value
            aRetval = clipPolygonOnParallelAxis(rCandidate, true, bInside, rRange.getMinY(), bStroke);

            if(aRetval.count())
            {
                // against Y-Axis, lower value
                if(1L == aRetval.count())
                {
                    aRetval = clipPolygonOnParallelAxis(aRetval.getB2DPolygon(0L), false, bInside, rRange.getMinX(), bStroke);
                }
                else
                {
                    aRetval = clipPolyPolygonOnParallelAxis(aRetval, false, bInside, rRange.getMinX(), bStroke);
                }

                if(aRetval.count())
                {
                    // against X-Axis, higher value
                    if(1L == aRetval.count())
                    {
                        aRetval = clipPolygonOnParallelAxis(aRetval.getB2DPolygon(0L), true, !bInside, rRange.getMaxY(), bStroke);
                    }
                    else
                    {
                        aRetval = clipPolyPolygonOnParallelAxis(aRetval, true, !bInside, rRange.getMaxY(), bStroke);
                    }

                    if(aRetval.count())
                    {
                        // against Y-Axis, higher value
                        if(1L == aRetval.count())
                        {
                            aRetval = clipPolygonOnParallelAxis(aRetval.getB2DPolygon(0L), false, !bInside, rRange.getMaxX(), bStroke);
                        }
                        else
                        {
                            aRetval = clipPolyPolygonOnParallelAxis(aRetval, false, !bInside, rRange.getMaxX(), bStroke);
                        }
                    }
                }
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolyPolygonOnRange(const B2DPolyPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            if(!nPolygonCount)
            {
                // source is empty
                return aRetval;
            }

            if(rRange.isEmpty())
            {
                if(bInside)
                {
                    // nothing is inside an empty range
                    return aRetval;
                }
                else
                {
                    // everything is outside an empty range
                    return rCandidate;
                }
            }

            if(bInside)
            {
                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    const B2DPolyPolygon aClippedPolyPolygon(clipPolygonOnRange(rCandidate.getB2DPolygon(a), rRange, bInside, bStroke));

                    if(aClippedPolyPolygon.count())
                    {
                        aRetval.append(aClippedPolyPolygon);
                    }
                }
            }
            else
            {
                // for details, see comment in clipPolygonOnRange for the "cutting off
                // the outer parts of filled polygons at parallel lines" explanations
                const B2DPolygon aClip(createPolygonFromRect(rRange));

                return clipPolyPolygonOnPolyPolygon(rCandidate, B2DPolyPolygon(aClip), bInside, bStroke);
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolyPolygonOnPolyPolygon(const B2DPolyPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bInside, bool bStroke)
        {
            B2DPolyPolygon aRetval;

            if(rCandidate.count() && rClip.count())
            {
                // one or both are no rectangle - go the hard way and clip PolyPolygon
                // against PolyPolygon...
                if(bStroke)
                {
                    // line clipping, create line snippets by first adding all cut points and
                    // then marching along the edges and detecting if they are inside or outside
                    // the clip polygon
                    for(sal_uInt32 a(0); a < rCandidate.count(); a++)
                    {
                        // add cuts with clip to polygon, including bezier segments
                        const B2DPolygon aCandidate(addPointsAtCuts(rCandidate.getB2DPolygon(a), rClip));
                        const sal_uInt32 nPointCount(aCandidate.count());
                        const sal_uInt32 nEdgeCount(aCandidate.isClosed() ? nPointCount : nPointCount - 1L);
                        B2DCubicBezier aEdge;
                        B2DPolygon aRun;

                        for(sal_uInt32 b(0); b < nEdgeCount; b++)
                        {
                            aCandidate.getBezierSegment(b, aEdge);
                            const B2DPoint aTestPoint(aEdge.interpolatePoint(0.5));
                            const bool bIsInside(tools::isInside(rClip, aTestPoint) == bInside);

                            if(bIsInside)
                            {
                                if(!aRun.count())
                                {
                                    aRun.append(aEdge.getStartPoint());
                                }

                                if(aEdge.isBezier())
                                {
                                    aRun.appendBezierSegment(aEdge.getControlPointA(), aEdge.getControlPointB(), aEdge.getEndPoint());
                                }
                                else
                                {
                                    aRun.append(aEdge.getEndPoint());
                                }
                            }
                            else
                            {
                                if(aRun.count())
                                {
                                    aRetval.append(aRun);
                                    aRun.clear();
                                }
                            }
                        }

                        if(aRun.count())
                        {
                            // try to merge this last and first polygon; they may have been
                            // the former polygon's start/end point
                            if(aRetval.count())
                            {
                                const B2DPolygon aStartPolygon(aRetval.getB2DPolygon(0));

                                if(aStartPolygon.count() && aStartPolygon.getB2DPoint(0).equal(aRun.getB2DPoint(aRun.count() - 1)))
                                {
                                    // append start polygon to aRun, remove from result set
                                    aRun.append(aStartPolygon); aRun.removeDoublePoints();
                                    aRetval.remove(0);
                                }
                            }

                            aRetval.append(aRun);
                        }
                    }
                }
                else
                {
                    // check for simplification with ranges if !bStroke (handling as stroke is more simple),
                    // but also only when bInside, else the simplification may lead to recursive calls (see
                    // calls to clipPolyPolygonOnPolyPolygon in clipPolyPolygonOnRange and clipPolygonOnRange)
                    if(bInside)
                    {
                        // #i125349# detect if both given PolyPolygons are indeed ranges
                        bool bBothRectangle(false);

                        if(basegfx::tools::isRectangle(rCandidate))
                        {
                            if(basegfx::tools::isRectangle(rClip))
                            {
                                // both are ranges
                                bBothRectangle = true;
                            }
                            else
                            {
                                // rCandidate is rectangle -> clip rClip on rRectangle, use the much
                                // cheaper and numerically more stable clipping against a range
                                // This simplification (exchanging content and clip) is valid
                                // since we do a logical AND operation
                                return clipPolyPolygonOnRange(rClip, rCandidate.getB2DRange(), bInside, bStroke);
                            }
                        }
                        else if(basegfx::tools::isRectangle(rClip))
                        {
                            if(basegfx::tools::isRectangle(rCandidate))
                            {
                                // both are ranges
                                bBothRectangle = true;
                            }
                            else
                            {
                                // rClip is rectangle -> clip rCandidate on rRectangle, use the much
                                // cheaper and numerically more stable clipping against a range
                                return clipPolyPolygonOnRange(rCandidate, rClip.getB2DRange(), bInside, bStroke);
                            }
                        }

                        if(bBothRectangle)
                        {
                            // both are rectangle
                            if(rCandidate.getB2DRange().equal(rClip.getB2DRange()))
                            {
                                // if both are equal -> no change
                                return rCandidate;
                            }
                            else
                            {
                                // not equal -> create new intersection from both ranges,
                                // but much cheaper based on the ranges
                                basegfx::B2DRange aIntersectionRange(rCandidate.getB2DRange());

                                aIntersectionRange.intersect(rClip.getB2DRange());

                                if(aIntersectionRange.isEmpty())
                                {
                                    // no common IntersectionRange -> the clip will be empty
                                    return B2DPolyPolygon();
                                }
                                else
                                {
                                    // use common aIntersectionRange as result, convert
                                    // to expected tools::PolyPolygon form
                                    return basegfx::B2DPolyPolygon(
                                        basegfx::tools::createPolygonFromRect(aIntersectionRange));
                                }
                            }
                        }
                    }

                    // area clipping
                    B2DPolyPolygon aMergePolyPolygonA(rClip);

                    // First solve all polygon-self and polygon-polygon intersections.
                    // Also get rid of some not-needed polygons (neutral, no area -> when
                    // no intersections, these are tubes).
                    // Now it is possible to correct the orientations in the cut-free
                    // polygons to values corresponding to painting the tools::PolyPolygon with
                    // a XOR-WindingRule.
                    aMergePolyPolygonA = solveCrossovers(aMergePolyPolygonA);
                    aMergePolyPolygonA = stripNeutralPolygons(aMergePolyPolygonA);
                    aMergePolyPolygonA = correctOrientations(aMergePolyPolygonA);

                    if(!bInside)
                    {
                        // if we want to get the outside of the clip polygon, make
                        // it a 'Hole' in topological sense
                        aMergePolyPolygonA.flip();
                    }

                    B2DPolyPolygon aMergePolyPolygonB(rCandidate);

                    // prepare 2nd source polygon in same way
                    aMergePolyPolygonB = solveCrossovers(aMergePolyPolygonB);
                    aMergePolyPolygonB = stripNeutralPolygons(aMergePolyPolygonB);
                    aMergePolyPolygonB = correctOrientations(aMergePolyPolygonB);

                    // to clip against each other, concatenate and solve all
                    // polygon-polygon crossovers. polygon-self do not need to
                    // be solved again, they were solved in the preparation.
                    aRetval.append(aMergePolyPolygonA);
                    aRetval.append(aMergePolyPolygonB);
                    aRetval = solveCrossovers(aRetval);

                    // now remove neutral polygons (closed, but no area). In a last
                    // step throw away all polygons which have a depth of less than 1
                    // which means there was no logical AND at their position. For the
                    // not-inside solution, the clip was flipped to define it as 'Hole',
                    // so the removal rule is different here; remove all with a depth
                    // of less than 0 (aka holes).
                    aRetval = stripNeutralPolygons(aRetval);
                    aRetval = stripDispensablePolygons(aRetval, bInside);
                }
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolygonOnPolyPolygon(const B2DPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bInside, bool bStroke)
        {
            B2DPolyPolygon aRetval;

            if(rCandidate.count() && rClip.count())
            {
                aRetval = clipPolyPolygonOnPolyPolygon(B2DPolyPolygon(rCandidate), rClip, bInside, bStroke);
            }

            return aRetval;
        }

        /*
        * let a plane be defined as
        *
        *     v.n+d=0
        *
        * and a ray be defined as
        *
        *     a+(b-a)*t=0
        *
        * substitute and rearranging yields
        *
        *     t = -(a.n+d)/(n.(b-a))
        *
        * if the denominator is zero, the line is either
        * contained in the plane or parallel to the plane.
        * in either case, there is no intersection.
        * if numerator and denominator are both zero, the
        * ray is contained in the plane.
        *
        */
        struct scissor_plane {
            double nx,ny;           // plane normal
            double d;               // [-] minimum distance from origin
            sal_uInt32 clipmask;    // clipping mask, e.g. 1000 1000
        };

        /*
        *
        * polygon clipping rules  (straight out of Foley and Van Dam)
        * ===========================================================
        * current   |next       |emit
        * ____________________________________
        * inside    |inside     |next
        * inside    |outside    |intersect with clip plane
        * outside   |outside    |nothing
        * outside   |inside     |intersect with clip plane follwed by next
        *
        */
        sal_uInt32 scissorLineSegment( ::basegfx::B2DPoint           *in_vertex,    // input buffer
                                       sal_uInt32                     in_count,     // number of verts in input buffer
                                       ::basegfx::B2DPoint           *out_vertex,   // output buffer
                                       scissor_plane                 *pPlane,       // scissoring plane
                                       const ::basegfx::B2DRectangle &rR )          // clipping rectangle
        {

            sal_uInt32 out_count=0;

            // process all the verts
            for(sal_uInt32 i=0; i<in_count; i++) {

                // vertices are relative to the coordinate
                // system defined by the rectangle.
                ::basegfx::B2DPoint *curr = &in_vertex[i];
                ::basegfx::B2DPoint *next = &in_vertex[(i+1)%in_count];

                // perform clipping judgement & mask against current plane.
                sal_uInt32 clip = pPlane->clipmask & ((getCohenSutherlandClipFlags(*curr,rR)<<4)|getCohenSutherlandClipFlags(*next,rR));

                if(clip==0) { // both verts are inside
                    out_vertex[out_count++] = *next;
                }
                else if((clip&0x0f) && (clip&0xf0)) { // both verts are outside
                }
                else if((clip&0x0f) && (clip&0xf0)==0) { // curr is inside, next is outside

                    // direction vector from 'current' to 'next', *not* normalized
                    // to bring 't' into the [0<=x<=1] intervall.
                    ::basegfx::B2DPoint dir((*next)-(*curr));

                    double denominator = ( pPlane->nx*dir.getX() +
                                        pPlane->ny*dir.getY() );
                    double numerator = ( pPlane->nx*curr->getX() +
                                        pPlane->ny*curr->getY() +
                                        pPlane->d );
                    double t = -numerator/denominator;

                    // calculate the actual point of intersection
                    ::basegfx::B2DPoint intersection( curr->getX()+t*dir.getX(),
                                                    curr->getY()+t*dir.getY() );

                    out_vertex[out_count++] = intersection;
                }
                else if((clip&0x0f)==0 && (clip&0xf0)) { // curr is outside, next is inside

                    // direction vector from 'current' to 'next', *not* normalized
                    // to bring 't' into the [0<=x<=1] intervall.
                    ::basegfx::B2DPoint dir((*next)-(*curr));

                    double denominator = ( pPlane->nx*dir.getX() +
                                        pPlane->ny*dir.getY() );
                    double numerator = ( pPlane->nx*curr->getX() +
                                        pPlane->ny*curr->getY() +
                                        pPlane->d );
                    double t = -numerator/denominator;

                    // calculate the actual point of intersection
                    ::basegfx::B2DPoint intersection( curr->getX()+t*dir.getX(),
                                                    curr->getY()+t*dir.getY() );

                    out_vertex[out_count++] = intersection;
                    out_vertex[out_count++] = *next;
                }
            }

            return out_count;
        }

        B2DPolygon clipTriangleListOnRange( const B2DPolygon& rCandidate,
                                            const B2DRange&   rRange )
        {
            B2DPolygon aResult;

            if( !(rCandidate.count()%3) )
            {
                const int scissor_plane_count = 4;

                scissor_plane sp[scissor_plane_count];

                sp[0].nx = +1.0;
                sp[0].ny = +0.0;
                sp[0].d = -(rRange.getMinX());
                sp[0].clipmask = (RectClipFlags::LEFT << 4) | RectClipFlags::LEFT; // 0001 0001
                sp[1].nx = -1.0;
                sp[1].ny = +0.0;
                sp[1].d = +(rRange.getMaxX());
                sp[1].clipmask = (RectClipFlags::RIGHT << 4) | RectClipFlags::RIGHT; // 0010 0010
                sp[2].nx = +0.0;
                sp[2].ny = +1.0;
                sp[2].d = -(rRange.getMinY());
                sp[2].clipmask = (RectClipFlags::TOP << 4) | RectClipFlags::TOP; // 0100 0100
                sp[3].nx = +0.0;
                sp[3].ny = -1.0;
                sp[3].d = +(rRange.getMaxY());
                sp[3].clipmask = (RectClipFlags::BOTTOM << 4) | RectClipFlags::BOTTOM; // 1000 1000

                // retrieve the number of vertices of the triangulated polygon
                const sal_uInt32 nVertexCount = rCandidate.count();

                if(nVertexCount)
                {
                    // Upper bound for the maximal number of vertices when intersecting an
                    // axis-aligned rectangle with a triangle in E2

                    // The rectangle and the triangle are in general position, and have 4 and 3
                    // vertices, respectively.

                    //   Lemma: Since the rectangle is a convex polygon ( see
                    //   http://mathworld.wolfram.com/ConvexPolygon.html for a definition), and
                    //   has no holes, it follows that any straight line will intersect the
                    //   rectangle's border line at utmost two times (with the usual
                    //   tie-breaking rule, if the intersection exactly hits an already existing
                    //   rectangle vertex, that this intersection is only attributed to one of
                    //   the adjoining edges). Thus, having a rectangle intersected with
                    //   a half-plane (one side of a straight line denotes 'inside', the
                    //   other 'outside') will at utmost add _one_  vertex to the resulting
                    //   intersection polygon (adding two intersection vertices, and removing at
                    //   least one rectangle vertex):

                    //         *
                    //     +--+-----------------+
                    //     | *                  |
                    //     |*                   |
                    //     +                    |
                    //    *|                    |
                    //   * |                    |
                    //     +--------------------+

                    //   Proof: If the straight line intersects the rectangle two
                    //   times, it does so for distinct edges, i.e. the intersection has
                    //   minimally one of the rectangle's vertices on either side of the straight
                    //   line (but maybe more). Thus, the intersection with a half-plane has
                    //   minimally _one_ rectangle vertex removed from the resulting clip
                    //   polygon, and therefore, a clip against a half-plane has the net effect
                    //   of adding at utmost _one_ vertex to the resulting clip polygon.

                    // Theorem: The intersection of a rectangle and a triangle results in a
                    // polygon with at utmost 7 vertices.

                    // Proof: The inside of the triangle can be described as the consecutive
                    // intersection with three half-planes. Together with the lemma above, this
                    // results in at utmost 3 additional vertices added to the already existing 4
                    // rectangle vertices.

                    // This upper bound is attained with the following example configuration:

                    //                               *
                    //                             ***
                    //                           ** *
                    //                         **  *
                    //                       **   *
                    //                     **    *
                    //                   **     *
                    //                 **      *
                    //               **       *
                    //             **        *
                    //           **         *
                    //     ----*2--------3 *
                    //     | **          |*
                    //     1*            4
                    //   **|            *|
                    // **  |           * |
                    //   **|          *  |
                    //     7*        *   |
                    //     --*6-----5-----
                    //         **  *
                    //           **

                    // As we need to scissor all triangles against the
                    // output rectangle we employ an output buffer for the
                    // resulting vertices.  the question is how large this
                    // buffer needs to be compared to the number of
                    // incoming vertices.  this buffer needs to hold at
                    // most the number of original vertices times '7'. see
                    // figure above for an example.  scissoring triangles
                    // with the cohen-sutherland line clipping algorithm
                    // as implemented here will result in a triangle fan
                    // which will be rendered as separate triangles to
                    // avoid pipeline stalls for each scissored
                    // triangle. creating separate triangles from a
                    // triangle fan produces (n-2)*3 vertices where n is
                    // the number of vertices of the original triangle
                    // fan.  for the maximum number of 7 vertices of
                    // resulting triangle fans we therefore need 15 times
                    // the number of original vertices.

                    //const size_t nBufferSize = sizeof(vertex)*(nVertexCount*16);
                    //vertex *pVertices = (vertex*)alloca(nBufferSize);
                    //sal_uInt32 nNumOutput = 0;

                    // we need to clip this triangle against the output rectangle
                    // to ensure that the resulting texture coordinates are in
                    // the valid range from [0<=st<=1]. under normal circustances
                    // we could use the BORDERCOLOR renderstate but some cards
                    // seem to ignore this feature.
                    ::basegfx::B2DPoint stack[3];
                    unsigned int clipflag = 0;

                    for(sal_uInt32 nIndex=0; nIndex<nVertexCount; ++nIndex)
                    {
                        // rotate stack
                        stack[0] = stack[1];
                        stack[1] = stack[2];
                        stack[2] = rCandidate.getB2DPoint(nIndex);

                        // clipping judgement
                        clipflag |= unsigned(!(rRange.isInside(stack[2])));

                        if(nIndex > 1)
                        {
                            // consume vertices until a single separate triangle has been visited.
                            if(!((nIndex+1)%3))
                            {
                                // if any of the last three vertices was outside
                                // we need to scissor against the destination rectangle
                                if(clipflag & 7)
                                {
                                    ::basegfx::B2DPoint buf0[16];
                                    ::basegfx::B2DPoint buf1[16];

                                    sal_uInt32 vertex_count = 3;

                                    // clip against all 4 planes passing the result of
                                    // each plane as the input to the next using a double buffer
                                    vertex_count = scissorLineSegment(stack,vertex_count,buf1,&sp[0],rRange);
                                    vertex_count = scissorLineSegment(buf1,vertex_count,buf0,&sp[1],rRange);
                                    vertex_count = scissorLineSegment(buf0,vertex_count,buf1,&sp[2],rRange);
                                    vertex_count = scissorLineSegment(buf1,vertex_count,buf0,&sp[3],rRange);

                                    if(vertex_count >= 3)
                                    {
                                        // convert triangle fan back to triangle list.
                                        ::basegfx::B2DPoint v0(buf0[0]);
                                        ::basegfx::B2DPoint v1(buf0[1]);
                                        for(sal_uInt32 i=2; i<vertex_count; ++i)
                                        {
                                            ::basegfx::B2DPoint v2(buf0[i]);
                                            aResult.append(v0);
                                            aResult.append(v1);
                                            aResult.append(v2);
                                            v1 = v2;
                                        }
                                    }
                                }
                                else
                                {
                                    // the last triangle has not been altered, simply copy to result
                                    for(sal_uInt32 i=0; i<3; ++i)
                                        aResult.append(stack[i]);
                                }
                            }
                        }

                        clipflag <<= 1;
                    }
                }
            }

            return aResult;
        }

    } // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
