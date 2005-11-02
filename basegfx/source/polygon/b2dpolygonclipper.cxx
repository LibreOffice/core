/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolygonclipper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:57:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#endif

#ifndef _BGFX_POLYGON_CUTANDTOUCH_HXX
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_CURVE_B2DCUBICBEZIER_HXX
#include <basegfx/curve/b2dcubicbezier.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

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
                    // prepare loop(s)
                    OSL_ENSURE(!rCandidate.areControlPointsUsed(), "clipPolygonOnParallelAxis: ATM works not for curves (!)");
                    B2DPolygon aNewPolygon;
                    B2DPoint aCurrent(rCandidate.getB2DPoint(0L));
                    bool bCurrentInside(bParallelToXAxis ?
                        fTools::moreOrEqual(aCurrent.getY(), fValueOnOtherAxis) == bAboveAxis :
                        fTools::moreOrEqual(aCurrent.getX(), fValueOnOtherAxis) == bAboveAxis);
                    const sal_uInt32 nPointCount(rCandidate.count());
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1L);

                    if(bCurrentInside)
                    {
                        aNewPolygon.append(aCurrent);
                    }

                    if(bStroke)
                    {
                        // open polygon, create clipped line snippets.
                        for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                        {
                            // get next point data
                            const sal_uInt32 nNextIndex((a + 1L == nPointCount) ? 0L : a + 1L);
                            const B2DPoint aNext(rCandidate.getB2DPoint(nNextIndex));
                            const bool bNextInside(bParallelToXAxis ?
                                fTools::moreOrEqual(aNext.getY(), fValueOnOtherAxis) == bAboveAxis :
                                fTools::moreOrEqual(aNext.getX(), fValueOnOtherAxis) == bAboveAxis);

                            if(bCurrentInside != bNextInside)
                            {
                                // change inside/outside
                                if(bNextInside)
                                {
                                    // entering, finish existing and start new line polygon
                                    if(aNewPolygon.count() > 1L)
                                    {
                                        aRetval.append(aNewPolygon);
                                    }

                                    aNewPolygon.clear();
                                }

                                // calculate and add cut point
                                if(bParallelToXAxis)
                                {
                                    const double fNewX(aCurrent.getX() - (((aCurrent.getY() - fValueOnOtherAxis) * (aNext.getX() - aCurrent.getX()) / (aNext.getY() - aCurrent.getY()))));
                                    aNewPolygon.append(B2DPoint(fNewX, fValueOnOtherAxis));
                                }
                                else
                                {
                                    const double fNewY(aCurrent.getY() - (((aCurrent.getX() - fValueOnOtherAxis) * (aNext.getY() - aCurrent.getY()) / (aNext.getX() - aCurrent.getX()))));
                                    aNewPolygon.append(B2DPoint(fValueOnOtherAxis, fNewY));
                                }

                                // pepare next step
                                bCurrentInside = bNextInside;
                            }

                            if(bNextInside)
                            {
                                aNewPolygon.append(aNext);
                            }

                            // pepare next step
                            aCurrent = aNext;
                        }

                        if(aNewPolygon.count() > 1L)
                        {
                            aRetval.append(aNewPolygon);
                        }
                    }
                    else
                    {
                        // closed polygon, create single clipped closed polygon
                        for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                        {
                            // get next point data, use offset
                            const sal_uInt32 nNextIndex((a + 1L == nPointCount) ? 0L : a + 1L);
                            const B2DPoint aNext(rCandidate.getB2DPoint(nNextIndex));
                            const bool bNextInside(bParallelToXAxis ?
                                fTools::moreOrEqual(aNext.getY(), fValueOnOtherAxis) == bAboveAxis :
                                fTools::moreOrEqual(aNext.getX(), fValueOnOtherAxis) == bAboveAxis);

                            if(bCurrentInside != bNextInside)
                            {
                                // change inside/outside, calculate and add cut point
                                if(bParallelToXAxis)
                                {
                                    const double fNewX(aCurrent.getX() - (((aCurrent.getY() - fValueOnOtherAxis) * (aNext.getX() - aCurrent.getX()) / (aNext.getY() - aCurrent.getY()))));
                                    aNewPolygon.append(B2DPoint(fNewX, fValueOnOtherAxis));
                                }
                                else
                                {
                                    const double fNewY(aCurrent.getY() - (((aCurrent.getX() - fValueOnOtherAxis) * (aNext.getY() - aCurrent.getY()) / (aNext.getX() - aCurrent.getX()))));
                                    aNewPolygon.append(B2DPoint(fValueOnOtherAxis, fNewY));
                                }

                                // pepare next step
                                bCurrentInside = bNextInside;
                            }

                            if(bNextInside && nNextIndex)
                            {
                                aNewPolygon.append(aNext);
                            }

                            // pepare next step
                            aCurrent = aNext;
                        }

                        if(aNewPolygon.count() > 2L)
                        {
                            aNewPolygon.setClosed(true);
                            aRetval.append(aNewPolygon);
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
                B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                aRetval.append(clipPolygonOnParallelAxis(aCandidate, bParallelToXAxis, bAboveAxis, fValueOnOtherAxis, bStroke));
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolygonOnRange(const B2DPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke)
        {
            B2DPolyPolygon aRetval;

            if(rRange.isEmpty())
            {
                // clipping against an empty range. Nothing is inside an empty range, so the polygon
                // is outside the range. So only return if not inside is wanted
                if(!bInside && rCandidate.count())
                {
                    aRetval.append(rCandidate);
                }
            }
            if(rCandidate.count())
            {
                const B2DRange aCandidateRange(getRange(rCandidate));

                if(rRange.isInside(aCandidateRange))
                {
                    // candidate is completely inside given range, nothing to do. Is also true with curves.
                    if(bInside)
                    {
                        aRetval.append(rCandidate);
                    }
                }
                else
                {
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
                }
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolyPolygonOnRange(const B2DPolyPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                aRetval.append(clipPolygonOnRange(aCandidate, rRange, bInside, bStroke));
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolygonOnEdge(const B2DPolygon& rCandidate, const B2DPoint& rPointA, const B2DPoint& rPointB, bool bAbove, bool bStroke)
        {
            B2DPolyPolygon aRetval;

            if(rPointA.equal(rPointB))
            {
                // edge has no length, return polygon
                aRetval.append(rCandidate);
            }
            else if(rCandidate.count())
            {
                const B2DVector aEdge(rPointB - rPointA);
                B2DHomMatrix aMatrixTransform;
                B2DPolygon aCandidate(rCandidate);

                // translate and rotate polygon so that given edge is on x axis
                aMatrixTransform.translate(-rPointA.getX(), -rPointA.getY());
                aMatrixTransform.rotate(-atan2(aEdge.getY(), aEdge.getX()));
                aCandidate.transform(aMatrixTransform);

                // call clip method on X-Axis
                aRetval = clipPolygonOnParallelAxis(aCandidate, true, bAbove, 0.0, bStroke);

                if(aRetval.count())
                {
                    // if there is a result, it needs to be transformed back
                    aMatrixTransform.invert();
                    aRetval.transform(aMatrixTransform);
                }
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolyPolygonOnEdge(const B2DPolyPolygon& rCandidate, const B2DPoint& rPointA, const B2DPoint& rPointB, bool bAbove, bool bStroke)
        {
            B2DPolyPolygon aRetval;

            if(rPointA.equal(rPointB))
            {
                // edge has no length, return polygon
                aRetval = rCandidate;
            }
            else if(rCandidate.count())
            {
                const B2DVector aEdge(rPointB - rPointA);
                B2DHomMatrix aMatrixTransform;
                B2DPolyPolygon aCandidate(rCandidate);

                // translate and rotate polygon so that given edge is on x axis
                aMatrixTransform.translate(-rPointA.getX(), -rPointA.getY());
                aMatrixTransform.rotate(-atan2(aEdge.getY(), aEdge.getX()));
                aCandidate.transform(aMatrixTransform);

                // call clip method on X-Axis
                aRetval = clipPolyPolygonOnParallelAxis(aCandidate, true, bAbove, 0.0, bStroke);

                if(aRetval.count())
                {
                    // if there is a result, it needs to be transformed back
                    aMatrixTransform.invert();
                    aRetval.transform(aMatrixTransform);
                }
            }

            return aRetval;
        }

        //////////////////////////////////////////////////////////////////////////////

        B2DPolyPolygon clipPolyPolygonOnPolyPolygon(const B2DPolyPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bStroke, bool bInvert)
        {
            B2DPolyPolygon aRetval;

            if(rCandidate.count() && rClip.count())
            {
                if(bStroke)
                {
                    // line clipping, create line snippets
                    for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                    {
                        // get candidate and add cuts and touches with rClip to aCandidate
                        const B2DPolygon aCandidate(addPointsAtCutsAndTouches(rClip, rCandidate.getB2DPolygon(a)));
                        const sal_uInt32 nPointCount(aCandidate.count());

                        if(nPointCount)
                        {
                            const sal_uInt32 nEdgeCount(aCandidate.isClosed() ? nPointCount : nPointCount - 1L);
                            B2DPolygon aRun;
                            B2DPoint aCurrent(aCandidate.getB2DPoint(0L));

                            for(sal_uInt32 b(0L); b < nEdgeCount; b++)
                            {
                                B2DVector aControlVectorA;
                                B2DVector aControlVectorB;
                                bool bCurveEdge(false);

                                if(aCandidate.areControlPointsUsed())
                                {
                                    aControlVectorA = aCandidate.getControlVectorA(b);
                                    aControlVectorB = aCandidate.getControlVectorB(b);
                                    bCurveEdge = !(aControlVectorA.equalZero() && aControlVectorB.equalZero());
                                }

                                const sal_uInt32 nNextIndex((b + 1L == nPointCount) ? 0L : b + 1L);
                                const B2DPoint aNext(aCandidate.getB2DPoint(nNextIndex));
                                B2DPoint aComparePoint;

                                if(bCurveEdge)
                                {
                                    B2DCubicBezier aCubicBezier(aCurrent, aControlVectorA, aControlVectorB, aNext);
                                    aComparePoint = aCubicBezier.interpolatePoint(0.5);
                                }
                                else
                                {
                                    aComparePoint = average(aCurrent, aNext);
                                }

                                const bool bInside(isInside(rClip, aComparePoint) != bInvert);

                                if(bInside)
                                {
                                    if(!aRun.count())
                                    {
                                        aRun.append(aCurrent);

                                        if(bCurveEdge)
                                        {
                                            const sal_uInt32 nNextRunIndex(aRun.count() - 1L);
                                            aRun.setControlVectorA(nNextRunIndex, aControlVectorA);
                                            aRun.setControlVectorB(nNextRunIndex, aControlVectorB);
                                        }
                                    }

                                    aRun.append(aNext);

                                    if(bCurveEdge)
                                    {
                                        const sal_uInt32 nNextRunIndex(aRun.count() - 1L);
                                        aRun.setControlVectorA(nNextRunIndex, aCandidate.getControlVectorA(nNextIndex));
                                        aRun.setControlVectorB(nNextRunIndex, aCandidate.getControlVectorB(nNextIndex));
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

                                // prepare next step
                                aCurrent = aNext;
                            }

                            if(aRun.count())
                            {
                                aRetval.append(aRun);
                            }
                        }
                    }
                }
                else
                {
                    // area clipping
                    B2DPolyPolygon aMergePolyPolygonA(rClip);
                    aMergePolyPolygonA = SolveCrossovers(aMergePolyPolygonA);
                    aMergePolyPolygonA = StripNeutralPolygons(aMergePolyPolygonA);
                    aMergePolyPolygonA = StripDispensablePolygons(aMergePolyPolygonA);

                    if(bInvert)
                    {
                        aMergePolyPolygonA.flip();
                    }

                    B2DPolyPolygon aMergePolyPolygonB(rCandidate);
                    aMergePolyPolygonB = SolveCrossovers(aMergePolyPolygonB);
                    aMergePolyPolygonB = StripNeutralPolygons(aMergePolyPolygonB);
                    aMergePolyPolygonB = StripDispensablePolygons(aMergePolyPolygonB);

                    aRetval.append(aMergePolyPolygonA);
                    aRetval.append(aMergePolyPolygonB);
                    aRetval = SolveCrossovers(aRetval, false);
                    aRetval = StripNeutralPolygons(aRetval);
                    aRetval = StripDispensablePolygons(aRetval, !bInvert);
                }
            }

            return aRetval;
        }

        //////////////////////////////////////////////////////////////////////////////

        B2DPolyPolygon clipPolygonOnPolyPolygon(const B2DPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bStroke, bool bInvert)
        {
            B2DPolyPolygon aRetval;

            if(rCandidate.count() && rClip.count())
            {
                aRetval = clipPolyPolygonOnPolyPolygon(B2DPolyPolygon(rCandidate), rClip, bStroke, bInvert);
            }

            return aRetval;
        }

        //////////////////////////////////////////////////////////////////////////////

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

        inline sal_uInt32 getClipFlags( const ::basegfx::B2DPoint&     rV,
                                        const ::basegfx::B2DRectangle& rR )
        {
            // maxY | minY | maxX | minX
            sal_uInt32 clip  = (rV.getX() < rR.getMinX()) << 0;
                    clip |= (rV.getX() > rR.getMaxX()) << 1;
                    clip |= (rV.getY() < rR.getMinY()) << 2;
                    clip |= (rV.getY() > rR.getMaxY()) << 3;
            return clip;
        }

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
                                    sal_uInt32                    in_count,     // number of verts in input buffer
                                    ::basegfx::B2DPoint          *out_vertex,   // output buffer
                                    scissor_plane                *pPlane,       // scissoring plane
                                    const ::basegfx::B2DRectangle &rR )         // clipping rectangle
        {
            ::basegfx::B2DPoint *curr;
            ::basegfx::B2DPoint *next;

            sal_uInt32 out_count=0;

            // process all the verts
            for(sal_uInt32 i=0; i<in_count; i++) {

                // vertices are relative to the coordinate
                // system defined by the rectangle.
                curr = &in_vertex[i];
                next = &in_vertex[(i+1)%in_count];

                // perform clipping judgement & mask against current plane.
                sal_uInt32 clip = pPlane->clipmask & ((getClipFlags(*curr,rR)<<4)|getClipFlags(*next,rR));

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
                sp[0].clipmask = 0x11; // 0001 0001
                sp[1].nx = -1.0;
                sp[1].ny = +0.0;
                sp[1].d = +(rRange.getMaxX());
                sp[1].clipmask = 0x22; // 0010 0010
                sp[2].nx = +0.0;
                sp[2].ny = +1.0;
                sp[2].d = -(rRange.getMinY());
                sp[2].clipmask = 0x44; // 0100 0100
                sp[3].nx = +0.0;
                sp[3].ny = -1.0;
                sp[3].d = +(rRange.getMaxY());
                sp[3].clipmask = 0x88; // 1000 1000

                // retrieve the number of vertices of the triangulated polygon
                const sal_uInt32 nVertexCount = rCandidate.count();

                if(nVertexCount)
                {
                    ////////////////////////////////////////////////////////////////////////
                    ////////////////////////////////////////////////////////////////////////
                    ////////////////////////////////////////////////////////////////////////
                    //
                    // Upper bound for the maximal number of vertices when intersecting an
                    // axis-aligned rectangle with a triangle in E2
                    //
                    // The rectangle and the triangle are in general position, and have 4 and 3
                    // vertices, respectively.
                    //
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
                    //
                    //         *
                    //     +--+-----------------+
                    //     | *                  |
                    //     |*                   |
                    //     +                    |
                    //    *|                    |
                    //   * |                    |
                    //     +--------------------+
                    //
                    //   Proof: If the straight line intersects the rectangle two
                    //   times, it does so for distinct edges, i.e. the intersection has
                    //   minimally one of the rectangle's vertices on either side of the straight
                    //   line (but maybe more). Thus, the intersection with a half-plane has
                    //   minimally _one_ rectangle vertex removed from the resulting clip
                    //   polygon, and therefore, a clip against a half-plane has the net effect
                    //   of adding at utmost _one_ vertex to the resulting clip polygon.
                    //
                    // Theorem: The intersection of a rectangle and a triangle results in a
                    // polygon with at utmost 7 vertices.
                    //
                    // Proof: The inside of the triangle can be described as the consecutive
                    // intersection with three half-planes. Together with the lemma above, this
                    // results in at utmost 3 additional vertices added to the already existing 4
                    // rectangle vertices.
                    //
                    // This upper bound is attained with the following example configuration:
                    //
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
                    //
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
                    //
                    ////////////////////////////////////////////////////////////////////////
                    ////////////////////////////////////////////////////////////////////////
                    ////////////////////////////////////////////////////////////////////////

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
                        clipflag |= !(rRange.isInside(stack[2]));

                        if(nIndex > 1)
                        {
                            // consume vertices until a single seperate triangle has been visited.
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

        //////////////////////////////////////////////////////////////////////////////

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

// eof
