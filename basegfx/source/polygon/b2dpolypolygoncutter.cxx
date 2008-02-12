/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolypolygoncutter.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:25:20 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_CUTANDTOUCH_HXX
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        //////////////////////////////////////////////////////////////////////////////

        bool impLeftOfEdges(const B2DPoint& rPrev, const B2DPoint& rCurr, const B2DPoint& rNext, const B2DPoint& rTest)
        {
            // tests if rTest is left of both directed line segments along the line rPrev, rCurr, rNext. Test is
            // with border, so for rTest on border or rTest == rCtrr, true is returned, too.
            const B2DVector aVecA(rCurr - rPrev); // A is edge vector from prev to curr
            const B2DVector aVecB(rNext - rCurr); // B is edge vector from curr to next
            const B2DVector aVecTest(rTest - rCurr); // testpoint seen as vector, too

            if(aVecA.cross(aVecB) < 0.0)
            {
                // b is left turn seen from a, test if Test is left of both and so inside (left is seeen as inside)
                const bool bBoolA(fTools::lessOrEqual(aVecA.cross(aVecTest), 0.0));
                const bool bBoolB(fTools::lessOrEqual(aVecB.cross(aVecTest), 0.0));
                return (bBoolA && bBoolB);
            }
            else
            {
                // b is right turn seen from a, test if Test is right of both and so outside (left is seeen as inside)
                // #i81852# border is included (see above), use moreOrEqual(), not more()
                const bool bBoolA(fTools::moreOrEqual(aVecA.cross(aVecTest), 0.0));
                const bool bBoolB(fTools::moreOrEqual(aVecB.cross(aVecTest), 0.0));
                return (!(bBoolA && bBoolB));
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        struct impSortNode
        {
            B2DPoint                                maPoint;
            sal_uInt32                              mnIndex;

            // sort operator to be able to sort on coordinates to later see common points
            bool operator<(const impSortNode& rComp) const
            {
                if(fTools::equal(maPoint.getX(), rComp.maPoint.getX()))
                {
                    if(fTools::equal(maPoint.getY(), rComp.maPoint.getY()))
                    {
                        return (mnIndex < rComp.mnIndex);
                    }
                    else
                    {
                        return fTools::less(maPoint.getY(), rComp.maPoint.getY());
                    }
                }
                else
                {
                    return fTools::less(maPoint.getX(), rComp.maPoint.getX());
                }
            }
        };

        //////////////////////////////////////////////////////////////////////////////

        enum CommonPointType
        {
            COMMON_IS_PARALLEL,             // C0: parallel in one direction
            COMMON_IS_PARALLEL_OPPOSITE,    // C2: parallel opposite directions
            COMMON_IS_LEAVE,                // C1: A leaving B
            COMMON_IS_LEAVE_OPPOSITE,       // C3: A leaving B in opposite direction
            COMMON_IS_ENTER,                // C4: A entering B
            COMMON_IS_ENTER_OPPOSITE,       // C5: A entering B in opposite direction
            COMMON_IS_TOUCH,                // C6: A touching B
            COMMON_IS_CROSS,                // C7: A crossing B
            COMMON_IS_DEADEND               // C8: one or both are a deadend, so it's only a touch
        };

        CommonPointType impGetCommonPointType(const B2DPoint& rPoint, const B2DPoint& rPrevA, const B2DPoint& rNextA, const B2DPoint& rPrevB, const B2DPoint& rNextB)
        {
            if(rPrevA.equal(rNextA) || rPrevB.equal(rNextB))
            {
                return COMMON_IS_DEADEND;
            }
            else if(rPrevA.equal(rPrevB))
            {
                if(rNextA.equal(rNextB))
                {
                    return COMMON_IS_PARALLEL;
                }
                else
                {
                    return COMMON_IS_LEAVE;
                }
            }
            else if(rPrevA.equal(rNextB))
            {
                if(rNextA.equal(rPrevB))
                {
                    return COMMON_IS_PARALLEL_OPPOSITE;
                }
                else
                {
                    return COMMON_IS_LEAVE_OPPOSITE;
                }
            }
            else if(rNextA.equal(rNextB))
            {
                return COMMON_IS_ENTER;
            }
            else if(rNextA.equal(rPrevB))
            {
                return COMMON_IS_ENTER_OPPOSITE;
            }
            else
            {
                // C7, C8: check for crossover
                const bool bSideOfPrevB(impLeftOfEdges(rPrevA, rPoint, rNextA, rPrevB));
                const bool bSideOfNextB(impLeftOfEdges(rPrevA, rPoint, rNextA, rNextB));

                if(bSideOfPrevB == bSideOfNextB)
                {
                    return COMMON_IS_TOUCH;
                }
                else
                {
                    return COMMON_IS_CROSS;
                }
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        struct impPolyPolygonPointNode
        {
            // B2DPolyPolygon related indices
            sal_uInt32                              mnPoint;        // index of point in polygon
            sal_uInt32                              mnPoly;         // index of polygon in polyPolygon

            // impPolyPolygonPointNode related indices
            sal_uInt32                              mnSelf;         // my own PointNode index in whole point array
            sal_uInt32                              mnPrev;         // index to prev PointNode in whole point array
            sal_uInt32                              mnNext;         // index to next PointNode in whole point array
            sal_uInt32                              mnNextControl;  // index to PointNode holding NextControlPoint (normally same as mnSelf)

            // bitfield
            unsigned                                mbUsed : 1;     // used flag for later extraction
        };

        typedef ::std::vector< impPolyPolygonPointNode > impPolyPolygonPointVector;

        //////////////////////////////////////////////////////////////////////////////

        void impSwitchNext(impPolyPolygonPointNode& rCandA, impPolyPolygonPointNode& rCandB, impPolyPolygonPointVector& rPointVector)
        {
            // get next nodes
            impPolyPolygonPointNode& rNextA = rPointVector[rCandA.mnNext];
            impPolyPolygonPointNode& rNextB = rPointVector[rCandB.mnNext];

            // switch prev/next indices
            rCandA.mnNext = rNextB.mnSelf;
            rNextB.mnPrev = rCandA.mnSelf;
            rCandB.mnNext = rNextA.mnSelf;
            rNextA.mnPrev = rCandB.mnSelf;

            // switch NextControl indices to follow the correct curve
            ::std::swap(rCandA.mnNextControl, rCandB.mnNextControl);
        }

        //////////////////////////////////////////////////////////////////////////////

        B2DPoint impGetB2DPoint(const impPolyPolygonPointNode& rNode, const B2DPolyPolygon& rPolyPolygon)
        {
            return rPolyPolygon.getB2DPolygon(rNode.mnPoly).getB2DPoint(rNode.mnPoint);
        }

    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        //////////////////////////////////////////////////////////////////////////////

        class impPolygonCrossoverSolver
        {
            const B2DPolygon&                           maOriginal;
            B2DPolygon                                  maGeometry;
            impPolyPolygonPointVector                   maPointVector;

            // bitfield
            unsigned                                    mbChanged : 1;

            void impHandleCommon(impPolyPolygonPointNode& rCandA, impPolyPolygonPointNode& rCandB)
            {
                const B2DPoint aPoint(maGeometry.getB2DPoint(rCandA.mnSelf));
                B2DPoint aPrevA(maGeometry.getB2DPoint(rCandA.mnPrev));
                B2DPoint aNextA(maGeometry.getB2DPoint(rCandA.mnNext));
                B2DPoint aPrevB(maGeometry.getB2DPoint(rCandB.mnPrev));
                B2DPoint aNextB(maGeometry.getB2DPoint(rCandB.mnNext));

                if(maGeometry.areControlPointsUsed())
                {
                    // #i81852# Of course control point vectors need to be used for self-intersections, too
                    const B2DPoint aCandidatePrevA(maGeometry.getPrevControlPoint(rCandA.mnPoint));
                    const B2DPoint aCandidatePrevB(maGeometry.getPrevControlPoint(rCandB.mnPoint));
                    const impPolyPolygonPointNode& rNextControlA = maPointVector[rCandA.mnNextControl];
                    const B2DPoint aCandidateNextA(maGeometry.getNextControlPoint(rNextControlA.mnPoint));
                    const impPolyPolygonPointNode& rNextControlB = maPointVector[rCandB.mnNextControl];
                    const B2DPoint aCandidateNextB(maGeometry.getNextControlPoint(rNextControlB.mnPoint));

                    if(!aCandidatePrevA.equal(aPoint))
                    {
                        aPrevA = aCandidatePrevA;
                    }

                    if(!aCandidatePrevB.equal(aPoint))
                    {
                        aPrevB = aCandidatePrevB;
                    }

                    if(!aCandidateNextA.equal(aPoint))
                    {
                        aNextA = aCandidateNextA;
                    }

                    if(!aCandidateNextB.equal(aPoint))
                    {
                        aNextB = aCandidateNextB;
                    }
                }

                const CommonPointType aType(impGetCommonPointType(aPoint, aPrevA, aNextA, aPrevB, aNextB));

                switch(aType)
                {
                    case COMMON_IS_LEAVE : // A leaving B
                    case COMMON_IS_LEAVE_OPPOSITE : // A leaving B in opposite direction
                    case COMMON_IS_ENTER : // A entering B
                    case COMMON_IS_ENTER_OPPOSITE : // A entering B in opposite direction
                    case COMMON_IS_CROSS : // A crossing B
                    {
                        impSwitchNext(rCandA, rCandB, maPointVector);
                        mbChanged = true;
                        break;
                    }
                    case COMMON_IS_PARALLEL:
                    case COMMON_IS_PARALLEL_OPPOSITE:
                    case COMMON_IS_TOUCH:
                    case COMMON_IS_DEADEND:
                        break;
                }
            }

            void impBuildGraph()
            {
                sal_uInt32 a;

                // prepare input: create all selfcuts and touches. After
                // this step, there will be no cut or touch inside edges, only at points.
                // Also remove double points to always have a edge length and direction.
                maGeometry = tools::addPointsAtCutsAndTouches(maOriginal);
                maGeometry.removeDoublePoints();

                // create space in point and sort vector.
                const sal_uInt32 nCount(maGeometry.count());
                ::std::vector< impSortNode > aSortNodes;
                maPointVector.resize(nCount);
                aSortNodes.resize(nCount);

                // fill data to points and sort vector
                for(a = 0L; a < nCount; a++)
                {
                    impPolyPolygonPointNode& rNewPointNode = maPointVector[a];
                    rNewPointNode.mnSelf = a;
                    rNewPointNode.mnNextControl = a;
                    rNewPointNode.mnPoint = a;
                    rNewPointNode.mnPoly = 0L;
                    rNewPointNode.mnPrev = (a != 0L) ? a - 1L : nCount - 1L;
                    rNewPointNode.mnNext = (a + 1L == nCount) ? 0L : a + 1L;
                    rNewPointNode.mbUsed = false;

                    impSortNode& rNewSortNode = aSortNodes[a];
                    rNewSortNode.maPoint = maGeometry.getB2DPoint(a);
                    rNewSortNode.mnIndex = a;
                }

                // sort by point to identify common nodes
                ::std::sort(aSortNodes.begin(), aSortNodes.end());

                // handle common nodes
                for(a = 0L; a < nCount; a++)
                {
                    // #129701# test b before using it, not after. Also use nCount instead of aSortNodes.size()
                    for(sal_uInt32 b(a + 1L); b < nCount && aSortNodes[a].maPoint.equal(aSortNodes[b].maPoint); b++)
                    {
                        impHandleCommon(maPointVector[aSortNodes[a].mnIndex], maPointVector[aSortNodes[b].mnIndex]);
                    }
                }
            }

        public:
            impPolygonCrossoverSolver(const B2DPolygon& rPolygon)
            :   maOriginal(rPolygon),
                mbChanged(false)
            {
                if(maOriginal.count())
                {
                    impBuildGraph();
                }
            }

            B2DPolyPolygon getB2DPolyPolygon()
            {
                if(mbChanged)
                {
                    B2DPolyPolygon aRetval;
                    sal_uInt32 nPointsUsed(0L);

                    for(sal_uInt32 a(0L); nPointsUsed != maGeometry.count() && a < maPointVector.size(); a++)
                    {
                        impPolyPolygonPointNode& rNode = maPointVector[a];

                        if(!rNode.mbUsed)
                        {
                            B2DPolygon aNew;
                            sal_uInt32 nCurr(rNode.mnSelf);
                            const bool bControlPointsUsed(maGeometry.areControlPointsUsed());

                            do
                            {
                                // get and add point
                                impPolyPolygonPointNode& rCandidate = maPointVector[nCurr];
                                const B2DPoint aNewPoint(maGeometry.getB2DPoint(rCandidate.mnPoint));
                                aNew.append(aNewPoint);

                                if(bControlPointsUsed)
                                {
                                    // apply control points to added point
                                    const sal_uInt32 nIndex(aNew.count() - 1);
                                    const impPolyPolygonPointNode& rNextControl = maPointVector[rCandidate.mnNextControl];

                                    aNew.setControlPoints(nIndex,
                                        maGeometry.getPrevControlPoint(rCandidate.mnPoint),
                                        maGeometry.getNextControlPoint(rNextControl.mnPoint));
                                }

                                // check if last two edges build a dead end and maybe removed
                                const sal_uInt32 nNewPointCount(aNew.count());

                                if(nNewPointCount > 2 && aNew.getB2DPoint(nNewPointCount - 3).equal(aNewPoint))
                                {
                                    if(bControlPointsUsed)
                                    {
                                        // check if edges are the same including control points
                                        if(aNew.getPrevControlPoint(nNewPointCount - 2).equal(aNew.getNextControlPoint(nNewPointCount - 2)))
                                        {
                                            if(aNew.getPrevControlPoint(nNewPointCount - 1).equal(aNew.getNextControlPoint(nNewPointCount - 3)))
                                            {
                                                // rescue nextControlPoint and remove
                                                aNew.setNextControlPoint(nNewPointCount - 3, aNew.getNextControlPoint(nNewPointCount - 1));
                                                aNew.remove(nNewPointCount - 2, 2);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        // edges are the same, remove
                                        aNew.remove(nNewPointCount - 2, 2);
                                    }
                                }

                                // mark as used and go to next
                                nPointsUsed++;
                                rCandidate.mbUsed = true;
                                nCurr = rCandidate.mnNext;
                            }
                            while(nCurr != rNode.mnSelf);

                            if(aNew.count())
                            {
                                aNew.setClosed(true);
                                aRetval.append(aNew);
                            }
                        }
                    }

                    return aRetval;
                }
                else
                {
                    return B2DPolyPolygon(maOriginal);
                }
            }
        };

        //////////////////////////////////////////////////////////////////////////////

    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        //////////////////////////////////////////////////////////////////////////////

        class impPolyPolygonCrossoverSolver
        {
            const B2DPolyPolygon&                       maOriginal;
            B2DPolyPolygon                              maGeometry;
            sal_uInt32                                  mnPointCount;
            impPolyPolygonPointVector                   maPointVector;

            // bitfield
            unsigned                                    mbChanged : 1;

            void impHandleLeaving(impPolyPolygonPointNode& rCandidateA, impPolyPolygonPointNode& rCandidateB, bool bOpposite, bool bSideOfLeave)
            {
                // go back on A and look for node entering B
                sal_uInt32 nIndexA(rCandidateA.mnSelf);
                sal_uInt32 nIndexB(rCandidateB.mnSelf);
                bool bOnCommonEdge(true);

                // go along common edge as long as we are on common edge, backward on
                // A and depending on bOpposite along B. Since we are on a leave, there must
                // exist an entering node, so the loop will end.
                while(bOnCommonEdge)
                {
                    const sal_uInt32 nCandA(maPointVector[nIndexA].mnPrev);
                    const sal_uInt32 nCandB((bOpposite) ? maPointVector[nIndexB].mnNext : maPointVector[nIndexB].mnPrev);
                    const impPolyPolygonPointNode& rCandA = maPointVector[nCandA];
                    const impPolyPolygonPointNode& rCandB = maPointVector[nCandB];
                    const B2DPoint aPointA(impGetB2DPoint(rCandA, maGeometry));
                    const B2DPoint aPointB(impGetB2DPoint(rCandB, maGeometry));

                    if(aPointA.equal(aPointB))
                    {
                        // continue going along common edge
                        nIndexA = nCandA;
                        nIndexB = nCandB;
                    }
                    else
                    {
                        // done
                        bOnCommonEdge = false;
                    }
                }

                // now we have the last common edge in (nIndexA, nIndexB) which must be the
                // entering edge. Get the point values from there for the crossover test
                impPolyPolygonPointNode& rEnterCandA = maPointVector[nIndexA];
                impPolyPolygonPointNode& rEnterCandB = maPointVector[nIndexB];

                if(bOpposite)
                {
                    // #i81852# switch at enter and leave, make the common edge(s) an own neutral polygon
                    // Always switch at bOpposite, no need to figure out enter/leave
                    impSwitchNext(rEnterCandA, rEnterCandB, maPointVector);
                    impSwitchNext(rCandidateA, rCandidateB, maPointVector);

                    // set changed flag
                    mbChanged = true;
                }
                else
                {
                    // #i81852# enter/leave needs to be figured out. Calculate points for test
                    const B2DPoint aPoint(impGetB2DPoint(rEnterCandA, maGeometry));
                    B2DPoint aPrevA(impGetB2DPoint(maPointVector[rEnterCandA.mnPrev], maGeometry));
                    B2DPoint aNextA(impGetB2DPoint(maPointVector[rEnterCandA.mnNext], maGeometry));
                    B2DPoint aNextB(impGetB2DPoint(maPointVector[rEnterCandB.mnNext], maGeometry));

                    if(maGeometry.areControlPointsUsed())
                    {
                        // #i81852# of course also these points need to be adapted to control vectors
                        const B2DPoint aCandidatePrevA(maGeometry.getB2DPolygon(rEnterCandA.mnPoly).getPrevControlPoint(rEnterCandA.mnPoint));
                        const impPolyPolygonPointNode& rNextControlA = maPointVector[rEnterCandA.mnNextControl];
                        const B2DPoint aCandidateNextA(maGeometry.getB2DPolygon(rNextControlA.mnPoly).getNextControlPoint(rNextControlA.mnPoint));
                        const impPolyPolygonPointNode& rNextControlB = maPointVector[rEnterCandB.mnNextControl];
                        const B2DPoint aCandidateNextB(maGeometry.getB2DPolygon(rNextControlB.mnPoly).getNextControlPoint(rNextControlB.mnPoint));

                        if(!aCandidatePrevA.equal(aPoint))
                        {
                            aPrevA = aCandidatePrevA;
                        }

                        if(!aCandidateNextA.equal(aPoint))
                        {
                            aNextA = aCandidateNextA;
                        }

                        if(!aCandidateNextB.equal(aPoint))
                        {
                            aNextB = aCandidateNextB;
                        }
                    }

                    const bool bSideOfEnter(impLeftOfEdges(aPrevA, aPoint, aNextA, aNextB));

                    if(bSideOfLeave != bSideOfEnter)
                    {
                        // crossover, needs to be solved
                        // switch at leave
                        impSwitchNext(rCandidateA, rCandidateB, maPointVector);

                        // set changed flag
                        mbChanged = true;
                    }
                }
            }

            void impHandleCommon(impPolyPolygonPointNode& rCandA, impPolyPolygonPointNode& rCandB)
            {
                const B2DPoint aPoint(impGetB2DPoint(rCandA, maGeometry));
                B2DPoint aPrevA(impGetB2DPoint(maPointVector[rCandA.mnPrev], maGeometry));
                B2DPoint aNextA(impGetB2DPoint(maPointVector[rCandA.mnNext], maGeometry));
                B2DPoint aPrevB(impGetB2DPoint(maPointVector[rCandB.mnPrev], maGeometry));
                B2DPoint aNextB(impGetB2DPoint(maPointVector[rCandB.mnNext], maGeometry));

                if(maGeometry.areControlPointsUsed())
                {
                    // get the control points for the common points and use them if they are different. This will use
                    // the tangent for the touch/crossover calculation
                    const B2DPoint aCandidatePrevA(maGeometry.getB2DPolygon(rCandA.mnPoly).getPrevControlPoint(rCandA.mnPoint));
                    const B2DPoint aCandidatePrevB(maGeometry.getB2DPolygon(rCandB.mnPoly).getPrevControlPoint(rCandB.mnPoint));
                    const impPolyPolygonPointNode& rNextControlA = maPointVector[rCandA.mnNextControl];
                    const B2DPoint aCandidateNextA(maGeometry.getB2DPolygon(rNextControlA.mnPoly).getNextControlPoint(rNextControlA.mnPoint));
                    const impPolyPolygonPointNode& rNextControlB = maPointVector[rCandB.mnNextControl];
                    const B2DPoint aCandidateNextB(maGeometry.getB2DPolygon(rNextControlB.mnPoly).getNextControlPoint(rNextControlB.mnPoint));

                    if(!aCandidatePrevA.equal(aPoint))
                    {
                        aPrevA = aCandidatePrevA;
                    }

                    if(!aCandidatePrevB.equal(aPoint))
                    {
                        aPrevB = aCandidatePrevB;
                    }

                    if(!aCandidateNextA.equal(aPoint))
                    {
                        aNextA = aCandidateNextA;
                    }

                    if(!aCandidateNextB.equal(aPoint))
                    {
                        aNextB = aCandidateNextB;
                    }
                }

                const CommonPointType aType(impGetCommonPointType(aPoint, aPrevA, aNextA, aPrevB, aNextB));

                switch(aType)
                {
                    case COMMON_IS_LEAVE : // A leaving B
                    {
                        impHandleLeaving(rCandA, rCandB, false, impLeftOfEdges(aPrevA, aPoint, aNextA, aNextB));
                        break;
                    }
                    case COMMON_IS_LEAVE_OPPOSITE : // A leaving B in opposite direction
                    {
                        // #i81852# Since opposite will always be switched, it is not necessary to calculate
                        // the leave value for evaluation of cross/tutch
                        impHandleLeaving(rCandA, rCandB, true, true);
                        break;
                    }
                    case COMMON_IS_CROSS : // A crossing B
                    {
                        impSwitchNext(rCandA, rCandB, maPointVector);
                        mbChanged = true;
                        break;
                    }
                    case COMMON_IS_PARALLEL:
                    case COMMON_IS_PARALLEL_OPPOSITE:
                    case COMMON_IS_ENTER:
                    case COMMON_IS_ENTER_OPPOSITE:
                    case COMMON_IS_TOUCH:
                    case COMMON_IS_DEADEND:
                        break;
                }
            }

            void impBuildGraph()
            {
                sal_uInt32 a, b, c;

                // prepare input: create all selfcuts and touches. After
                // this step, there will be no cut or touch inside edges, only at points.
                // Self-intersections are not handled, should have been done outside this
                // helper already.
                // Also remove double points to always have a edge length and direction.
                maGeometry = tools::addPointsAtCutsAndTouches(maOriginal, false);
                maGeometry.removeDoublePoints();

                // get mnPointCount
                for(a = 0L; a < maGeometry.count(); a++)
                {
                    mnPointCount += maGeometry.getB2DPolygon(a).count();
                }

                // create space in point and sort vector.
                ::std::vector< impSortNode > aSortNodes;
                maPointVector.resize(mnPointCount);
                aSortNodes.resize(mnPointCount);

                // fill data to points and sort vector
                for(a = c = 0L; a < maGeometry.count(); a++)
                {
                    const B2DPolygon aPartGeometry(maGeometry.getB2DPolygon(a));
                    const sal_uInt32 nPartCount(aPartGeometry.count());
                    const sal_uInt32 nNewPolyStart(c);

                    for(b = 0L; b < nPartCount; b++, c++)
                    {
                        impPolyPolygonPointNode& rNewPointNode = maPointVector[c];
                        rNewPointNode.mnSelf = c;
                        rNewPointNode.mnNextControl = c;
                        rNewPointNode.mnPoint = b;
                        rNewPointNode.mnPoly = a;
                        rNewPointNode.mnNext = nNewPolyStart + ((b + 1L == nPartCount) ? 0L : b + 1L);
                        rNewPointNode.mnPrev = nNewPolyStart + ((b != 0L) ? b - 1L : nPartCount - 1L);
                        rNewPointNode.mbUsed = false;

                        impSortNode& rNewSortNode = aSortNodes[c];
                        rNewSortNode.maPoint = aPartGeometry.getB2DPoint(b);
                        rNewSortNode.mnIndex = c;
                    }
                }

                // sort by point to identify common nodes
                ::std::sort(aSortNodes.begin(), aSortNodes.end());

                // handle common nodes
                for(a = 0L; a < mnPointCount - 1L; a++)
                {
                    // #129701# test b before using it, not after. Also use mnPointCount instead of aSortNodes.size()
                    for(b = a + 1L; b < mnPointCount && aSortNodes[a].maPoint.equal(aSortNodes[b].maPoint); b++)
                    {
                        impHandleCommon(maPointVector[aSortNodes[a].mnIndex], maPointVector[aSortNodes[b].mnIndex]);
                    }
                }
            }

        public:
            impPolyPolygonCrossoverSolver(const B2DPolyPolygon& rPolyPolygon)
            :   maOriginal(rPolyPolygon),
                mnPointCount(0L),
                mbChanged(false)
            {
                if(maOriginal.count())
                {
                    impBuildGraph();
                }
            }

            B2DPolyPolygon getB2DPolyPolygon()
            {
                if(mbChanged)
                {
                    B2DPolyPolygon aRetval;
                    sal_uInt32 nPointsUsed(0L);

                    for(sal_uInt32 a(0L); nPointsUsed != mnPointCount && a < maPointVector.size(); a++)
                    {
                        impPolyPolygonPointNode& rNode = maPointVector[a];

                        if(!rNode.mbUsed)
                        {
                            B2DPolygon aNew;
                            sal_uInt32 nCurr(rNode.mnSelf);
                            const bool bControlPointsUsed(maGeometry.areControlPointsUsed());

                            do
                            {
                                // get and add point
                                impPolyPolygonPointNode& rCandidate = maPointVector[nCurr];
                                const B2DPoint aNewPoint(impGetB2DPoint(rCandidate, maGeometry));
                                aNew.append(aNewPoint);

                                if(bControlPointsUsed)
                                {
                                    // apply control points to added point
                                    const sal_uInt32 nIndex(aNew.count() - 1);
                                    const impPolyPolygonPointNode& rNextControl = maPointVector[rCandidate.mnNextControl];

                                    aNew.setControlPoints(nIndex,
                                        maGeometry.getB2DPolygon(rCandidate.mnPoly).getPrevControlPoint(rCandidate.mnPoint),
                                        maGeometry.getB2DPolygon(rNextControl.mnPoly).getNextControlPoint(rNextControl.mnPoint));
                                }

                                // check if last two edges build a dead end and maybe removed
                                const sal_uInt32 nNewPointCount(aNew.count());

                                if(nNewPointCount > 2 && aNew.getB2DPoint(nNewPointCount - 3).equal(aNewPoint))
                                {
                                    if(bControlPointsUsed)
                                    {
                                        // check if edges are the same including control points
                                        if(aNew.getPrevControlPoint(nNewPointCount - 2).equal(aNew.getNextControlPoint(nNewPointCount - 2)))
                                        {
                                            if(aNew.getPrevControlPoint(nNewPointCount - 1).equal(aNew.getNextControlPoint(nNewPointCount - 3)))
                                            {
                                                // rescue nextControlPoint and remove
                                                aNew.setNextControlPoint(nNewPointCount - 3, aNew.getNextControlPoint(nNewPointCount - 1));
                                                aNew.remove(nNewPointCount - 2, 2);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        // edges are the same, remove
                                        aNew.remove(nNewPointCount - 2, 2);
                                    }
                                }

                                // mark as used and go to next
                                rCandidate.mbUsed = true;
                                nCurr = rCandidate.mnNext;
                            }
                            while(nCurr != rNode.mnSelf);

                            if(aNew.count())
                            {
                                aNew.setClosed(true);
                                aRetval.append(aNew);
                            }
                        }
                    }

                    return aRetval;
                }
                else
                {
                    return B2DPolyPolygon(maOriginal);
                }
            }
        };

        //////////////////////////////////////////////////////////////////////////////

        struct impStripHelper
        {
            B2DRange                                maRange;
            sal_Int32                               mnDepth;
            B2VectorOrientation                     meOrinetation;
        };

        //////////////////////////////////////////////////////////////////////////////

    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        //////////////////////////////////////////////////////////////////////////////

        B2DPolyPolygon SolveCrossovers(const B2DPolyPolygon& rCandidate, bool bSelfCrossovers)
        {
            B2DPolyPolygon aCandidate;

            if(bSelfCrossovers)
            {
                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    aCandidate.append(SolveCrossovers(rCandidate.getB2DPolygon(a)));
                }
            }
            else
            {
                aCandidate = rCandidate;
            }

            if(aCandidate.count() > 1L)
            {
                impPolyPolygonCrossoverSolver aSolver(aCandidate);
                aCandidate = aSolver.getB2DPolyPolygon();
            }

            return aCandidate;
        }

        //////////////////////////////////////////////////////////////////////////////

        B2DPolyPolygon SolveCrossovers(const B2DPolygon& rCandidate)
        {
            impPolygonCrossoverSolver aSolver(rCandidate);
            return aSolver.getB2DPolyPolygon();
        }

        //////////////////////////////////////////////////////////////////////////////

        B2DPolyPolygon StripNeutralPolygons(const B2DPolyPolygon& rCandidate)
        {
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                if(ORIENTATION_NEUTRAL != tools::getOrientation(aCandidate))
                {
                    aRetval.append(aCandidate);
                }
            }

            return aRetval;
        }

        //////////////////////////////////////////////////////////////////////////////

        B2DPolyPolygon StripDispensablePolygons(const B2DPolyPolygon& rCandidate, bool bKeepAboveZero)
        {
            const sal_uInt32 nCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            if(nCount)
            {
                if(nCount == 1L)
                {
                    if(!bKeepAboveZero && ORIENTATION_POSITIVE == tools::getOrientation(rCandidate.getB2DPolygon(0L)))
                    {
                        aRetval = rCandidate;
                    }
                }
                else
                {
                    sal_uInt32 a, b;
                    ::std::vector< impStripHelper > aHelpers;
                    aHelpers.resize(nCount);

                    for(a = 0L; a < nCount; a++)
                    {
                        const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));
                        impStripHelper* pNewHelper = &(aHelpers[a]);
                        pNewHelper->maRange = tools::getRange(aCandidate);
                        pNewHelper->meOrinetation = tools::getOrientation(aCandidate);
                        pNewHelper->mnDepth = (ORIENTATION_NEGATIVE == pNewHelper->meOrinetation ? -1L : 0L);
                    }

                    for(a = 0L; a < nCount - 1L; a++)
                    {
                        const B2DPolygon aCandA(rCandidate.getB2DPolygon(a));
                        impStripHelper& rHelperA = aHelpers[a];

                        for(b = a + 1L; b < nCount; b++)
                        {
                            const B2DPolygon aCandB(rCandidate.getB2DPolygon(b));
                            impStripHelper& rHelperB = aHelpers[b];
                            const bool bAInB(rHelperB.maRange.isInside(rHelperA.maRange) && tools::isInside(aCandB, aCandA, true));
                            const bool bBInA(rHelperA.maRange.isInside(rHelperB.maRange) && tools::isInside(aCandA, aCandB, true));

                            if(bAInB && bBInA)
                            {
                                // congruent
                                if(rHelperA.meOrinetation == rHelperB.meOrinetation)
                                {
                                    // two polys or two holes. Lower one of them to get one of them out of the way.
                                    // Since each will be contained in the other one, both will be increased, too.
                                    // So, for lowering, increase only one of them
                                    rHelperA.mnDepth++;
                                }
                                else
                                {
                                    // poly and hole. They neutralize, so get rid of both. Move securely below zero.
                                    rHelperA.mnDepth = -((sal_Int32)nCount);
                                    rHelperB.mnDepth = -((sal_Int32)nCount);
                                }
                            }
                            else
                            {
                                if(bAInB)
                                {
                                    if(ORIENTATION_NEGATIVE == rHelperB.meOrinetation)
                                    {
                                        rHelperA.mnDepth--;
                                    }
                                    else
                                    {
                                        rHelperA.mnDepth++;
                                    }
                                }
                                else if(bBInA)
                                {
                                    if(ORIENTATION_NEGATIVE == rHelperA.meOrinetation)
                                    {
                                        rHelperB.mnDepth--;
                                    }
                                    else
                                    {
                                        rHelperB.mnDepth++;
                                    }
                                }
                            }
                        }
                    }

                    for(a = 0L; a < nCount; a++)
                    {
                        const impStripHelper& rHelper = aHelpers[a];
                        bool bAcceptEntry(bKeepAboveZero ? 1L <= rHelper.mnDepth : 0L == rHelper.mnDepth);

                        if(bAcceptEntry)
                        {
                            aRetval.append(rCandidate.getB2DPolygon(a));
                        }
                    }
                }
            }

            return aRetval;
        }

        //////////////////////////////////////////////////////////////////////////////

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
