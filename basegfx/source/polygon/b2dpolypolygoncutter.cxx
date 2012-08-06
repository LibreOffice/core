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

#include <osl/diagnose.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        //////////////////////////////////////////////////////////////////////////////

        struct StripHelper
        {
            B2DRange                                maRange;
            sal_Int32                               mnDepth;
            B2VectorOrientation                     meOrinetation;
        };

        //////////////////////////////////////////////////////////////////////////////

        struct PN
        {
        public:
            B2DPoint                maPoint;
            sal_uInt32              mnI;
            sal_uInt32              mnIP;
            sal_uInt32              mnIN;
        };

        //////////////////////////////////////////////////////////////////////////////

        struct VN
        {
        public:
            B2DVector               maPrev;
            B2DVector               maNext;

            // to have the correct curve segments in the crossover checks,
            // it is necessary to keep the original next vectors, too. Else,
            // it may happen to use a already switched next vector which
            // would interpolate the wrong comparison point
            B2DVector               maOriginalNext;
        };

        //////////////////////////////////////////////////////////////////////////////

        struct SN
        {
        public:
            PN*                     mpPN;

            bool operator<(const SN& rComp) const
            {
                if(fTools::equal(mpPN->maPoint.getX(), rComp.mpPN->maPoint.getX()))
                {
                    if(fTools::equal(mpPN->maPoint.getY(), rComp.mpPN->maPoint.getY()))
                    {
                        return (mpPN->mnI < rComp.mpPN->mnI);
                    }
                    else
                    {
                        return fTools::less(mpPN->maPoint.getY(), rComp.mpPN->maPoint.getY());
                    }
                }
                else
                {
                    return fTools::less(mpPN->maPoint.getX(), rComp.mpPN->maPoint.getX());
                }
            }
        };

        //////////////////////////////////////////////////////////////////////////////

        typedef ::std::vector< PN > PNV;
        typedef ::std::vector< VN > VNV;
        typedef ::std::vector< SN > SNV;

        //////////////////////////////////////////////////////////////////////////////

        class solver
        {
        private:
            const B2DPolyPolygon    maOriginal;
            PNV                     maPNV;
            VNV                     maVNV;
            SNV                     maSNV;

            unsigned                mbIsCurve : 1;
            unsigned                mbChanged : 1;

            void impAddPolygon(const sal_uInt32 aPos, const B2DPolygon& rGeometry)
            {
                const sal_uInt32 nCount(rGeometry.count());
                PN aNewPN;
                VN aNewVN;
                SN aNewSN;

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    const B2DPoint aPoint(rGeometry.getB2DPoint(a));
                    aNewPN.maPoint = aPoint;
                    aNewPN.mnI = aPos + a;
                    aNewPN.mnIP = aPos + ((a != 0) ? a - 1 : nCount - 1);
                    aNewPN.mnIN = aPos + ((a + 1 == nCount) ? 0 : a + 1);
                    maPNV.push_back(aNewPN);

                    if(mbIsCurve)
                    {
                        aNewVN.maPrev = rGeometry.getPrevControlPoint(a) - aPoint;
                        aNewVN.maNext = rGeometry.getNextControlPoint(a) - aPoint;
                        aNewVN.maOriginalNext = aNewVN.maNext;
                        maVNV.push_back(aNewVN);
                    }

                    aNewSN.mpPN = &maPNV[maPNV.size() - 1];
                    maSNV.push_back(aNewSN);
                }
            }

            bool impLeftOfEdges(const B2DVector& rVecA, const B2DVector& rVecB, const B2DVector& rTest)
            {
                // tests if rTest is left of both directed line segments along the line -rVecA, rVecB. Test is
                // with border.
                if(rVecA.cross(rVecB) > 0.0)
                {
                    // b is left turn seen from a, test if Test is left of both and so inside (left is seeen as inside)
                    const bool bBoolA(fTools::moreOrEqual(rVecA.cross(rTest), 0.0));
                    const bool bBoolB(fTools::lessOrEqual(rVecB.cross(rTest), 0.0));

                    return (bBoolA && bBoolB);
                }
                else
                {
                    // b is right turn seen from a, test if Test is right of both and so outside (left is seeen as inside)
                    const bool bBoolA(fTools::lessOrEqual(rVecA.cross(rTest), 0.0));
                    const bool bBoolB(fTools::moreOrEqual(rVecB.cross(rTest), 0.0));

                    return (!(bBoolA && bBoolB));
                }
            }

            void impSwitchNext(PN& rPNa, PN& rPNb)
            {
                ::std::swap(rPNa.mnIN, rPNb.mnIN);

                if(mbIsCurve)
                {
                    VN& rVNa = maVNV[rPNa.mnI];
                    VN& rVNb = maVNV[rPNb.mnI];

                    ::std::swap(rVNa.maNext, rVNb.maNext);
                }

                if(!mbChanged)
                {
                    mbChanged = true;
                }
            }

            B2DCubicBezier createSegment(const PN& rPN, bool bPrev) const
            {
                const B2DPoint& rStart(rPN.maPoint);
                const B2DPoint& rEnd(maPNV[bPrev ? rPN.mnIP : rPN.mnIN].maPoint);
                const B2DVector& rCPA(bPrev ? maVNV[rPN.mnI].maPrev : maVNV[rPN.mnI].maNext);
                // Use maOriginalNext, not maNext to create the original (yet unchanged)
                // curve segment. Otherwise, this segment would NOT ne correct.
                const B2DVector& rCPB(bPrev ? maVNV[maPNV[rPN.mnIP].mnI].maOriginalNext : maVNV[maPNV[rPN.mnIN].mnI].maPrev);

                return B2DCubicBezier(rStart, rStart + rCPA, rEnd + rCPB, rEnd);
            }

            void impHandleCommon(PN& rPNa, PN& rPNb)
            {
                if(mbIsCurve)
                {
                    const B2DCubicBezier aNextA(createSegment(rPNa, false));
                    const B2DCubicBezier aPrevA(createSegment(rPNa, true));

                    if(aNextA.equal(aPrevA))
                    {
                        // deadend on A (identical edge)
                        return;
                    }

                    const B2DCubicBezier aNextB(createSegment(rPNb, false));
                    const B2DCubicBezier aPrevB(createSegment(rPNb, true));

                    if(aNextB.equal(aPrevB))
                    {
                        // deadend on B (identical edge)
                        return;
                    }

                    if(aPrevA.equal(aPrevB))
                    {
                        // common edge in same direction
                        return;
                    }
                    else if(aPrevA.equal(aNextB))
                    {
                        // common edge in opposite direction
                        if(aNextA.equal(aPrevB))
                        {
                            // common edge in opposite direction continues
                            return;
                        }
                        else
                        {
                            // common edge in opposite direction leave
                            impSwitchNext(rPNa, rPNb);
                        }
                    }
                    else if(aNextA.equal(aNextB))
                    {
                        // common edge in same direction enter
                        // search leave edge
                        PN* pPNa2 = &maPNV[rPNa.mnIN];
                        PN* pPNb2 = &maPNV[rPNb.mnIN];
                        bool bOnEdge(true);

                        do
                        {
                            const B2DCubicBezier aNextA2(createSegment(*pPNa2, false));
                            const B2DCubicBezier aNextB2(createSegment(*pPNb2, false));

                            if(aNextA2.equal(aNextB2))
                            {
                                pPNa2 = &maPNV[pPNa2->mnIN];
                                pPNb2 = &maPNV[pPNb2->mnIN];
                            }
                            else
                            {
                                bOnEdge = false;
                            }
                        }
                        while(bOnEdge && pPNa2 != &rPNa && pPNb2 != &rPNb);

                        if(bOnEdge)
                        {
                            // loop over two identical polygon paths
                            return;
                        }
                        else
                        {
                            // enter at rPNa, rPNb; leave at pPNa2, pPNb2. No common edges
                            // at enter/leave. Check for crossover.
                            const B2DVector aPrevCA(aPrevA.interpolatePoint(0.5) - aPrevA.getStartPoint());
                            const B2DVector aNextCA(aNextA.interpolatePoint(0.5) - aNextA.getStartPoint());
                            const B2DVector aPrevCB(aPrevB.interpolatePoint(0.5) - aPrevB.getStartPoint());
                            const bool bEnter(impLeftOfEdges(aPrevCA, aNextCA, aPrevCB));

                            const B2DCubicBezier aNextA2(createSegment(*pPNa2, false));
                            const B2DCubicBezier aPrevA2(createSegment(*pPNa2, true));
                            const B2DCubicBezier aNextB2(createSegment(*pPNb2, false));
                            const B2DVector aPrevCA2(aPrevA2.interpolatePoint(0.5) - aPrevA2.getStartPoint());
                            const B2DVector aNextCA2(aNextA2.interpolatePoint(0.5) - aNextA2.getStartPoint());
                            const B2DVector aNextCB2(aNextB2.interpolatePoint(0.5) - aNextB2.getStartPoint());
                            const bool bLeave(impLeftOfEdges(aPrevCA2, aNextCA2, aNextCB2));

                            if(bEnter != bLeave)
                            {
                                // crossover
                                impSwitchNext(rPNa, rPNb);
                            }
                        }
                    }
                    else if(aNextA.equal(aPrevB))
                    {
                        // common edge in opposite direction enter
                        impSwitchNext(rPNa, rPNb);
                    }
                    else
                    {
                        // no common edges, check for crossover
                        const B2DVector aPrevCA(aPrevA.interpolatePoint(0.5) - aPrevA.getStartPoint());
                        const B2DVector aNextCA(aNextA.interpolatePoint(0.5) - aNextA.getStartPoint());
                        const B2DVector aPrevCB(aPrevB.interpolatePoint(0.5) - aPrevB.getStartPoint());
                        const B2DVector aNextCB(aNextB.interpolatePoint(0.5) - aNextB.getStartPoint());

                        const bool bEnter(impLeftOfEdges(aPrevCA, aNextCA, aPrevCB));
                        const bool bLeave(impLeftOfEdges(aPrevCA, aNextCA, aNextCB));

                        if(bEnter != bLeave)
                        {
                            // crossover
                            impSwitchNext(rPNa, rPNb);
                        }
                    }
                }
                else
                {
                    const B2DPoint& rNextA(maPNV[rPNa.mnIN].maPoint);
                    const B2DPoint& rPrevA(maPNV[rPNa.mnIP].maPoint);

                    if(rNextA.equal(rPrevA))
                    {
                        // deadend on A
                        return;
                    }

                    const B2DPoint& rNextB(maPNV[rPNb.mnIN].maPoint);
                    const B2DPoint& rPrevB(maPNV[rPNb.mnIP].maPoint);

                    if(rNextB.equal(rPrevB))
                    {
                        // deadend on B
                        return;
                    }

                    if(rPrevA.equal(rPrevB))
                    {
                        // common edge in same direction
                        return;
                    }
                    else if(rPrevA.equal(rNextB))
                    {
                        // common edge in opposite direction
                        if(rNextA.equal(rPrevB))
                        {
                            // common edge in opposite direction continues
                            return;
                        }
                        else
                        {
                            // common edge in opposite direction leave
                            impSwitchNext(rPNa, rPNb);
                        }
                    }
                    else if(rNextA.equal(rNextB))
                    {
                        // common edge in same direction enter
                        // search leave edge
                        PN* pPNa2 = &maPNV[rPNa.mnIN];
                        PN* pPNb2 = &maPNV[rPNb.mnIN];
                        bool bOnEdge(true);

                        do
                        {
                            const B2DPoint& rNextA2(maPNV[pPNa2->mnIN].maPoint);
                            const B2DPoint& rNextB2(maPNV[pPNb2->mnIN].maPoint);

                            if(rNextA2.equal(rNextB2))
                            {
                                pPNa2 = &maPNV[pPNa2->mnIN];
                                pPNb2 = &maPNV[pPNb2->mnIN];
                            }
                            else
                            {
                                bOnEdge = false;
                            }
                        }
                        while(bOnEdge && pPNa2 != &rPNa && pPNb2 != &rPNb);

                        if(bOnEdge)
                        {
                            // loop over two identical polygon paths
                            return;
                        }
                        else
                        {
                            // enter at rPNa, rPNb; leave at pPNa2, pPNb2. No common edges
                            // at enter/leave. Check for crossover.
                            const B2DPoint& aPointE(rPNa.maPoint);
                            const B2DVector aPrevAE(rPrevA - aPointE);
                            const B2DVector aNextAE(rNextA - aPointE);
                            const B2DVector aPrevBE(rPrevB - aPointE);

                            const B2DPoint& aPointL(pPNa2->maPoint);
                            const B2DVector aPrevAL(maPNV[pPNa2->mnIP].maPoint - aPointL);
                            const B2DVector aNextAL(maPNV[pPNa2->mnIN].maPoint - aPointL);
                            const B2DVector aNextBL(maPNV[pPNb2->mnIN].maPoint - aPointL);

                            const bool bEnter(impLeftOfEdges(aPrevAE, aNextAE, aPrevBE));
                            const bool bLeave(impLeftOfEdges(aPrevAL, aNextAL, aNextBL));

                            if(bEnter != bLeave)
                            {
                                // crossover; switch start or end
                                impSwitchNext(rPNa, rPNb);
                            }
                        }
                    }
                    else if(rNextA.equal(rPrevB))
                    {
                        // common edge in opposite direction enter
                        impSwitchNext(rPNa, rPNb);
                    }
                    else
                    {
                        // no common edges, check for crossover
                        const B2DPoint& aPoint(rPNa.maPoint);
                        const B2DVector aPrevA(rPrevA - aPoint);
                        const B2DVector aNextA(rNextA - aPoint);
                        const B2DVector aPrevB(rPrevB - aPoint);
                        const B2DVector aNextB(rNextB - aPoint);

                        const bool bEnter(impLeftOfEdges(aPrevA, aNextA, aPrevB));
                        const bool bLeave(impLeftOfEdges(aPrevA, aNextA, aNextB));

                        if(bEnter != bLeave)
                        {
                            // crossover
                            impSwitchNext(rPNa, rPNb);
                        }
                    }
                }
            }

            void impSolve()
            {
                // sort by point to identify common nodes
                ::std::sort(maSNV.begin(), maSNV.end());

                // handle common nodes
                const sal_uInt32 nNodeCount(maSNV.size());

                for(sal_uInt32 a(0); a < nNodeCount - 1; a++)
                {
                    // test a before using it, not after. Also use nPointCount instead of aSortNodes.size()
                    PN& rPNb = *(maSNV[a].mpPN);

                    for(sal_uInt32 b(a + 1); b < nNodeCount && rPNb.maPoint.equal(maSNV[b].mpPN->maPoint); b++)
                    {
                        impHandleCommon(rPNb, *maSNV[b].mpPN);
                    }
                }
            }

        public:
            explicit solver(const B2DPolygon& rOriginal)
            :   maOriginal(B2DPolyPolygon(rOriginal)),
                mbIsCurve(false),
                mbChanged(false)
            {
                const sal_uInt32 nOriginalCount(rOriginal.count());

                if(nOriginalCount)
                {
                    B2DPolygon aGeometry(tools::addPointsAtCutsAndTouches(rOriginal));
                    aGeometry.removeDoublePoints();
                    aGeometry = tools::simplifyCurveSegments(aGeometry);
                    mbIsCurve = aGeometry.areControlPointsUsed();

                    const sal_uInt32 nPointCount(aGeometry.count());

                    // If it's not a pezier polygon, at least four points are needed to create
                    // a self-intersection. If it's a bezier polygon, the minimum point number
                    // is two, since with a single point You get a curve, but no self-intersection
                    if(nPointCount > 3 || (nPointCount > 1 && mbIsCurve))
                    {
                        // reserve space in point, control and sort vector.
                        maSNV.reserve(nPointCount);
                        maPNV.reserve(nPointCount);
                        maVNV.reserve(mbIsCurve ? nPointCount : 0);

                        // fill data
                        impAddPolygon(0, aGeometry);

                        // solve common nodes
                        impSolve();
                    }
                }
            }

            explicit solver(const B2DPolyPolygon& rOriginal)
            :   maOriginal(rOriginal),
                mbIsCurve(false),
                mbChanged(false)
            {
                sal_uInt32 nOriginalCount(maOriginal.count());

                if(nOriginalCount)
                {
                    B2DPolyPolygon aGeometry(tools::addPointsAtCutsAndTouches(maOriginal, true));
                    aGeometry.removeDoublePoints();
                    aGeometry = tools::simplifyCurveSegments(aGeometry);
                    mbIsCurve = aGeometry.areControlPointsUsed();
                    nOriginalCount = aGeometry.count();

                    if(nOriginalCount)
                    {
                        sal_uInt32 nPointCount(0);
                        sal_uInt32 a(0);

                        // count points
                        for(a = 0; a < nOriginalCount; a++)
                        {
                            const B2DPolygon aCandidate(aGeometry.getB2DPolygon(a));
                            const sal_uInt32 nCandCount(aCandidate.count());

                            // If it's not a bezier curve, at least three points would be needed to have a
                            // topological relevant (not empty) polygon. Since its not known here if trivial
                            // edges (dead ends) will be kept or sorted out, add non-bezier polygons with
                            // more than one point.
                            // For bezier curves, the minimum for defining an area is also one.
                            if(nCandCount)
                            {
                                nPointCount += nCandCount;
                            }
                        }

                        if(nPointCount)
                        {
                            // reserve space in point, control and sort vector.
                            maSNV.reserve(nPointCount);
                            maPNV.reserve(nPointCount);
                            maVNV.reserve(mbIsCurve ? nPointCount : 0);

                            // fill data
                            sal_uInt32 nInsertIndex(0);

                            for(a = 0; a < nOriginalCount; a++)
                            {
                                const B2DPolygon aCandidate(aGeometry.getB2DPolygon(a));
                                const sal_uInt32 nCandCount(aCandidate.count());

                                // use same condition as above, the data vector is
                                // pre-allocated
                                if(nCandCount)
                                {
                                    impAddPolygon(nInsertIndex, aCandidate);
                                    nInsertIndex += nCandCount;
                                }
                            }

                            // solve common nodes
                            impSolve();
                        }
                    }
                }
            }

            B2DPolyPolygon getB2DPolyPolygon()
            {
                if(mbChanged)
                {
                    B2DPolyPolygon aRetval;
                    const sal_uInt32 nCount(maPNV.size());
                    sal_uInt32 nCountdown(nCount);

                    for(sal_uInt32 a(0); nCountdown && a < nCount; a++)
                    {
                        PN& rPN = maPNV[a];

                        if(SAL_MAX_UINT32 != rPN.mnI)
                        {
                            // unused node, start new part polygon
                            B2DPolygon aNewPart;
                            PN* pPNCurr = &rPN;

                            do
                            {
                                const B2DPoint& rPoint = pPNCurr->maPoint;
                                aNewPart.append(rPoint);

                                if(mbIsCurve)
                                {
                                    const VN& rVNCurr = maVNV[pPNCurr->mnI];

                                    if(!rVNCurr.maPrev.equalZero())
                                    {
                                        aNewPart.setPrevControlPoint(aNewPart.count() - 1, rPoint + rVNCurr.maPrev);
                                    }

                                    if(!rVNCurr.maNext.equalZero())
                                    {
                                        aNewPart.setNextControlPoint(aNewPart.count() - 1, rPoint + rVNCurr.maNext);
                                    }
                                }

                                pPNCurr->mnI = SAL_MAX_UINT32;
                                nCountdown--;
                                pPNCurr = &(maPNV[pPNCurr->mnIN]);
                            }
                            while(pPNCurr != &rPN && SAL_MAX_UINT32 != pPNCurr->mnI);

                            // close and add
                            aNewPart.setClosed(true);
                            aRetval.append(aNewPart);
                        }
                    }

                    return aRetval;
                }
                else
                {
                    // no change, return original
                    return maOriginal;
                }
            }
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

        B2DPolyPolygon solveCrossovers(const B2DPolyPolygon& rCandidate)
        {
            if(rCandidate.count() > 1L)
            {
                solver aSolver(rCandidate);
                return aSolver.getB2DPolyPolygon();
            }
            else
            {
                return rCandidate;
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        B2DPolyPolygon stripNeutralPolygons(const B2DPolyPolygon& rCandidate)
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

        B2DPolyPolygon stripDispensablePolygons(const B2DPolyPolygon& rCandidate, bool bKeepAboveZero)
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
                    ::std::vector< StripHelper > aHelpers;
                    aHelpers.resize(nCount);

                    for(a = 0L; a < nCount; a++)
                    {
                        const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));
                        StripHelper* pNewHelper = &(aHelpers[a]);
                        pNewHelper->maRange = tools::getRange(aCandidate);
                        pNewHelper->meOrinetation = tools::getOrientation(aCandidate);
                        pNewHelper->mnDepth = (ORIENTATION_NEGATIVE == pNewHelper->meOrinetation ? -1L : 0L);
                    }

                    for(a = 0L; a < nCount - 1L; a++)
                    {
                        const B2DPolygon aCandA(rCandidate.getB2DPolygon(a));
                        StripHelper& rHelperA = aHelpers[a];

                        for(b = a + 1L; b < nCount; b++)
                        {
                            const B2DPolygon aCandB(rCandidate.getB2DPolygon(b));
                            StripHelper& rHelperB = aHelpers[b];
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
                        const StripHelper& rHelper = aHelpers[a];
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

        B2DPolyPolygon prepareForPolygonOperation(const B2DPolygon& rCandidate)
        {
            solver aSolver(rCandidate);
            B2DPolyPolygon aRetval(stripNeutralPolygons(aSolver.getB2DPolyPolygon()));

            return correctOrientations(aRetval);
        }

        B2DPolyPolygon prepareForPolygonOperation(const B2DPolyPolygon& rCandidate)
        {
            solver aSolver(rCandidate);
            B2DPolyPolygon aRetval(stripNeutralPolygons(aSolver.getB2DPolyPolygon()));

            return correctOrientations(aRetval);
        }

        B2DPolyPolygon solvePolygonOperationOr(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB)
        {
            if(!rCandidateA.count())
            {
                return rCandidateB;
            }
            else if(!rCandidateB.count())
            {
                return rCandidateA;
            }
            else
            {
                // concatenate polygons, solve crossovers and throw away all sub-polygons
                // which have a depth other than 0.
                B2DPolyPolygon aRetval(rCandidateA);

                aRetval.append(rCandidateB);
                aRetval = solveCrossovers(aRetval);
                aRetval = stripNeutralPolygons(aRetval);

                return stripDispensablePolygons(aRetval, false);
            }
        }

        B2DPolyPolygon solvePolygonOperationXor(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB)
        {
            if(!rCandidateA.count())
            {
                return rCandidateB;
            }
            else if(!rCandidateB.count())
            {
                return rCandidateA;
            }
            else
            {
                // XOR is pretty simple: By definition it is the simple concatenation of
                // the single polygons since we imply XOR fill rule. Make it intersection-free
                // and correct orientations
                B2DPolyPolygon aRetval(rCandidateA);

                aRetval.append(rCandidateB);
                aRetval = solveCrossovers(aRetval);
                aRetval = stripNeutralPolygons(aRetval);

                return correctOrientations(aRetval);
            }
        }

        B2DPolyPolygon solvePolygonOperationAnd(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB)
        {
            if(!rCandidateA.count())
            {
                return B2DPolyPolygon();
            }
            else if(!rCandidateB.count())
            {
                return B2DPolyPolygon();
            }
            else
            {
                // concatenate polygons, solve crossovers and throw away all sub-polygons
                // with a depth of < 1. This means to keep all polygons where at least two
                // polygons do overlap.
                B2DPolyPolygon aRetval(rCandidateA);

                aRetval.append(rCandidateB);
                aRetval = solveCrossovers(aRetval);
                aRetval = stripNeutralPolygons(aRetval);

                return stripDispensablePolygons(aRetval, true);
            }
        }

        B2DPolyPolygon solvePolygonOperationDiff(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB)
        {
            if(!rCandidateA.count())
            {
                return B2DPolyPolygon();
            }
            else if(!rCandidateB.count())
            {
                return rCandidateA;
            }
            else
            {
                // Make B topologically to holes and append to A
                B2DPolyPolygon aRetval(rCandidateB);

                aRetval.flip();
                aRetval.append(rCandidateA);

                // solve crossovers and throw away all sub-polygons which have a
                // depth other than 0.
                aRetval = basegfx::tools::solveCrossovers(aRetval);
                aRetval = basegfx::tools::stripNeutralPolygons(aRetval);

                return basegfx::tools::stripDispensablePolygons(aRetval, false);
            }
        }

        B2DPolyPolygon mergeToSinglePolyPolygon(const std::vector< basegfx::B2DPolyPolygon >& rInput)
        {
            std::vector< basegfx::B2DPolyPolygon > aInput(rInput);

            // first step: prepareForPolygonOperation and simple merge of non-overlapping
            // PolyPolygons for speedup; this is possible for the wanted OR-operation
            if(!aInput.empty())
            {
                std::vector< basegfx::B2DPolyPolygon > aResult;
                aResult.reserve(aInput.size());

                for(sal_uInt32 a(0); a < aInput.size(); a++)
                {
                    const basegfx::B2DPolyPolygon aCandidate(prepareForPolygonOperation(aInput[a]));

                    if(!aResult.empty())
                    {
                        const B2DRange aCandidateRange(aCandidate.getB2DRange());
                        bool bCouldMergeSimple(false);

                        for(sal_uInt32 b(0); !bCouldMergeSimple && b < aResult.size(); b++)
                        {
                            basegfx::B2DPolyPolygon aTarget(aResult[b]);
                            const B2DRange aTargetRange(aTarget.getB2DRange());

                            if(!aCandidateRange.overlaps(aTargetRange))
                            {
                                aTarget.append(aCandidate);
                                aResult[b] = aTarget;
                                bCouldMergeSimple = true;
                            }
                        }

                        if(!bCouldMergeSimple)
                        {
                            aResult.push_back(aCandidate);
                        }
                    }
                    else
                    {
                        aResult.push_back(aCandidate);
                    }
                }

                aInput = aResult;
            }

            // second step: melt pairwise to a single PolyPolygon
            while(aInput.size() > 1)
            {
                std::vector< basegfx::B2DPolyPolygon > aResult;
                aResult.reserve((aInput.size() / 2) + 1);

                for(sal_uInt32 a(0); a < aInput.size(); a += 2)
                {
                    if(a + 1 < aInput.size())
                    {
                        // a pair for processing
                        aResult.push_back(solvePolygonOperationOr(aInput[a], aInput[a + 1]));
                    }
                    else
                    {
                        // last single PolyPolygon; copy to target to not lose it
                        aResult.push_back(aInput[a]);
                    }
                }

                aInput = aResult;
            }

            // third step: get result
            if(1 == aInput.size())
            {
                return aInput[0];
            }

            return B2DPolyPolygon();
        }

        //////////////////////////////////////////////////////////////////////////////

    } // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
