/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <osl/diagnose.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>

#include <vector>
#include <algorithm>


#define SUBDIVIDE_FOR_CUT_TEST_COUNT        (50)



namespace basegfx
{
    namespace
    {
        

        class temporaryPoint
        {
            B2DPoint                            maPoint;        
            sal_uInt32                          mnIndex;        
            double                              mfCut;          

        public:
            temporaryPoint(const B2DPoint& rNewPoint, sal_uInt32 nIndex, double fCut)
            :   maPoint(rNewPoint),
                mnIndex(nIndex),
                mfCut(fCut)
            {
            }

            bool operator<(const temporaryPoint& rComp) const
            {
                if(mnIndex == rComp.mnIndex)
                {
                    return (mfCut < rComp.mfCut);
                }

                return (mnIndex < rComp.mnIndex);
            }

            const B2DPoint& getPoint() const { return maPoint; }
            sal_uInt32 getIndex() const { return mnIndex; }
            double getCut() const { return mfCut; }
        };

        

        typedef ::std::vector< temporaryPoint > temporaryPointVector;

        

        class temporaryPolygonData
        {
            B2DPolygon                              maPolygon;
            B2DRange                                maRange;
            temporaryPointVector                    maPoints;

        public:
            const B2DPolygon& getPolygon() const { return maPolygon; }
            void setPolygon(const B2DPolygon& rNew) { maPolygon = rNew; maRange = tools::getRange(maPolygon); }
            const B2DRange& getRange() const { return maRange; }
            temporaryPointVector& getTemporaryPointVector() { return maPoints; }
        };

        

        B2DPolygon mergeTemporaryPointsAndPolygon(const B2DPolygon& rCandidate, temporaryPointVector& rTempPoints)
        {
            
            
            
            const sal_uInt32 nTempPointCount(rTempPoints.size());

            if(nTempPointCount)
            {
                B2DPolygon aRetval;
                const sal_uInt32 nCount(rCandidate.count());

                if(nCount)
                {
                    
                    ::std::sort(rTempPoints.begin(), rTempPoints.end());

                    
                    B2DCubicBezier aEdge;
                    sal_uInt32 nNewInd(0L);

                    
                    aRetval.append(rCandidate.getB2DPoint(0));

                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        
                        rCandidate.getBezierSegment(a, aEdge);

                        if(aEdge.isBezier())
                        {
                            
                            double fLeftStart(0.0);

                            
                            while(nNewInd < nTempPointCount && rTempPoints[nNewInd].getIndex() == a)
                            {
                                const temporaryPoint& rTempPoint = rTempPoints[nNewInd++];

                                
                                
                                
                                B2DCubicBezier aLeftPart;
                                const double fRelativeSplitPoint((rTempPoint.getCut() - fLeftStart) / (1.0 - fLeftStart));
                                aEdge.split(fRelativeSplitPoint, &aLeftPart, &aEdge);
                                fLeftStart = rTempPoint.getCut();

                                
                                aRetval.appendBezierSegment(aLeftPart.getControlPointA(), aLeftPart.getControlPointB(), rTempPoint.getPoint());
                            }

                            
                            aRetval.appendBezierSegment(aEdge.getControlPointA(), aEdge.getControlPointB(), aEdge.getEndPoint());
                        }
                        else
                        {
                            
                            while(nNewInd < nTempPointCount && rTempPoints[nNewInd].getIndex() == a)
                            {
                                const temporaryPoint& rTempPoint = rTempPoints[nNewInd++];
                                const B2DPoint aNewPoint(rTempPoint.getPoint());

                                
                                if(!aRetval.getB2DPoint(aRetval.count() - 1L).equal(aNewPoint))
                                {
                                    aRetval.append(aNewPoint);
                                }
                            }

                            
                            aRetval.append(aEdge.getEndPoint());
                        }
                    }
                }

                if(rCandidate.isClosed())
                {
                    
                    tools::closeWithGeometryChange(aRetval);
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        

        void adaptAndTransferCutsWithBezierSegment(
            const temporaryPointVector& rPointVector, const B2DPolygon& rPolygon,
            sal_uInt32 nInd, temporaryPointVector& rTempPoints)
        {
            
            
            
            
            const sal_uInt32 nTempPointCount(rPointVector.size());
            const sal_uInt32 nEdgeCount(rPolygon.count() ? rPolygon.count() - 1L : 0L);

            if(nTempPointCount && nEdgeCount)
            {
                for(sal_uInt32 a(0L); a < nTempPointCount; a++)
                {
                    const temporaryPoint& rTempPoint = rPointVector[a];
                    const double fCutPosInPolygon((double)rTempPoint.getIndex() + rTempPoint.getCut());
                    const double fRelativeCutPos(fCutPosInPolygon / (double)nEdgeCount);
                    rTempPoints.push_back(temporaryPoint(rTempPoint.getPoint(), nInd, fRelativeCutPos));
                }
            }
        }

        

    } 
} 



namespace basegfx
{
    namespace
    {
        
        

        void findCuts(const B2DPolygon& rCandidate, temporaryPointVector& rTempPoints);
        void findTouches(const B2DPolygon& rEdgePolygon, const B2DPolygon& rPointPolygon, temporaryPointVector& rTempPoints);
        void findCuts(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB, temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB);

        

        void findEdgeCutsTwoEdges(
            const B2DPoint& rCurrA, const B2DPoint& rNextA,
            const B2DPoint& rCurrB, const B2DPoint& rNextB,
            sal_uInt32 nIndA, sal_uInt32 nIndB,
            temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            
            if(!(rCurrA.equal(rNextA) || rCurrB.equal(rNextB)))
            {
                
                if(!(rCurrB.equal(rCurrA) || rCurrB.equal(rNextA) || rNextB.equal(rCurrA) || rNextB.equal(rNextA)))
                {
                    const B2DVector aVecA(rNextA - rCurrA);
                    const B2DVector aVecB(rNextB - rCurrB);
                    double fCut(aVecA.cross(aVecB));

                    if(!fTools::equalZero(fCut))
                    {
                        const double fZero(0.0);
                        const double fOne(1.0);
                        fCut = (aVecB.getY() * (rCurrB.getX() - rCurrA.getX()) + aVecB.getX() * (rCurrA.getY() - rCurrB.getY())) / fCut;

                        if(fTools::more(fCut, fZero) && fTools::less(fCut, fOne))
                        {
                            
                            double fCut2;

                            
                            if(fabs(aVecB.getX()) > fabs(aVecB.getY()))
                            {
                                fCut2 = (rCurrA.getX() + (fCut * aVecA.getX()) - rCurrB.getX()) / aVecB.getX();
                            }
                            else
                            {
                                fCut2 = (rCurrA.getY() + (fCut * aVecA.getY()) - rCurrB.getY()) / aVecB.getY();
                            }

                            if(fTools::more(fCut2, fZero) && fTools::less(fCut2, fOne))
                            {
                                
                                
                                
                                const B2DPoint aCutPoint(interpolate(rCurrA, rNextA, fCut));
                                rTempPointsA.push_back(temporaryPoint(aCutPoint, nIndA, fCut));
                                rTempPointsB.push_back(temporaryPoint(aCutPoint, nIndB, fCut2));
                            }
                        }
                    }
                }
            }
        }

        

        void findCutsAndTouchesAndCommonForBezier(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB, temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            
            
            
            
            
            
            
            
            
            
            
            
            OSL_ENSURE(!rCandidateA.areControlPointsUsed() && !rCandidateB.areControlPointsUsed(), "findCutsAndTouchesAndCommonForBezier only works with subdivided polygons (!)");
            OSL_ENSURE(!rCandidateA.isClosed() && !rCandidateB.isClosed(), "findCutsAndTouchesAndCommonForBezier only works with opened polygons (!)");
            const sal_uInt32 nPointCountA(rCandidateA.count());
            const sal_uInt32 nPointCountB(rCandidateB.count());

            if(nPointCountA > 1 && nPointCountB > 1)
            {
                const sal_uInt32 nEdgeCountA(nPointCountA - 1);
                const sal_uInt32 nEdgeCountB(nPointCountB - 1);
                B2DPoint aCurrA(rCandidateA.getB2DPoint(0L));

                for(sal_uInt32 a(0L); a < nEdgeCountA; a++)
                {
                    const B2DPoint aNextA(rCandidateA.getB2DPoint(a + 1L));
                    const B2DRange aRangeA(aCurrA, aNextA);
                    B2DPoint aCurrB(rCandidateB.getB2DPoint(0L));

                    for(sal_uInt32 b(0L); b < nEdgeCountB; b++)
                    {
                        const B2DPoint aNextB(rCandidateB.getB2DPoint(b + 1L));
                        const B2DRange aRangeB(aCurrB, aNextB);

                        if(aRangeA.overlaps(aRangeB))
                        {
                            
                            if(!(aCurrA.equal(aNextA) || aCurrB.equal(aNextB)))
                            {
                                const B2DVector aVecA(aNextA - aCurrA);
                                const B2DVector aVecB(aNextB - aCurrB);
                                double fCutA(aVecA.cross(aVecB));

                                if(!fTools::equalZero(fCutA))
                                {
                                    const double fZero(0.0);
                                    const double fOne(1.0);
                                    fCutA = (aVecB.getY() * (aCurrB.getX() - aCurrA.getX()) + aVecB.getX() * (aCurrA.getY() - aCurrB.getY())) / fCutA;

                                    
                                    
                                    if(fTools::moreOrEqual(fCutA, fZero) && fTools::less(fCutA, fOne))
                                    {
                                        
                                        double fCutB;

                                        
                                        if(fabs(aVecB.getX()) > fabs(aVecB.getY()))
                                        {
                                            fCutB = (aCurrA.getX() + (fCutA * aVecA.getX()) - aCurrB.getX()) / aVecB.getX();
                                        }
                                        else
                                        {
                                            fCutB = (aCurrA.getY() + (fCutA * aVecA.getY()) - aCurrB.getY()) / aVecB.getY();
                                        }

                                        
                                        
                                        if(fTools::moreOrEqual(fCutB, fZero) && fTools::less(fCutB, fOne))
                                        {
                                            
                                            
                                            if(fTools::equal(fCutA, fZero))
                                            {
                                                
                                                
                                                if(a)
                                                {
                                                    rTempPointsA.push_back(temporaryPoint(aCurrA, a, 0.0));
                                                }
                                            }
                                            else
                                            {
                                                const B2DPoint aCutPoint(interpolate(aCurrA, aNextA, fCutA));
                                                rTempPointsA.push_back(temporaryPoint(aCutPoint, a, fCutA));
                                            }

                                            
                                            if(fTools::equal(fCutB, fZero))
                                            {
                                                
                                                
                                                if(b)
                                                {
                                                    rTempPointsB.push_back(temporaryPoint(aCurrB, b, 0.0));
                                                }
                                            }
                                            else
                                            {
                                                const B2DPoint aCutPoint(interpolate(aCurrB, aNextB, fCutB));
                                                rTempPointsB.push_back(temporaryPoint(aCutPoint, b, fCutB));
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        
                        aCurrB = aNextB;
                    }

                    
                    aCurrA = aNextA;
                }
            }
        }

        

        void findEdgeCutsBezierAndEdge(
            const B2DCubicBezier& rCubicA,
            const B2DPoint& rCurrB, const B2DPoint& rNextB,
            sal_uInt32 nIndA, sal_uInt32 nIndB,
            temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            
            
            
            B2DPolygon aTempPolygonA;
            B2DPolygon aTempPolygonEdge;
            temporaryPointVector aTempPointVectorA;
            temporaryPointVector aTempPointVectorEdge;

            
            
            aTempPolygonA.reserve(SUBDIVIDE_FOR_CUT_TEST_COUNT + 8);
            aTempPolygonA.append(rCubicA.getStartPoint());
            rCubicA.adaptiveSubdivideByCount(aTempPolygonA, SUBDIVIDE_FOR_CUT_TEST_COUNT);
            aTempPolygonEdge.append(rCurrB);
            aTempPolygonEdge.append(rNextB);

            
            findCutsAndTouchesAndCommonForBezier(aTempPolygonA, aTempPolygonEdge, aTempPointVectorA, aTempPointVectorEdge);

            if(!aTempPointVectorA.empty())
            {
                
                adaptAndTransferCutsWithBezierSegment(aTempPointVectorA, aTempPolygonA, nIndA, rTempPointsA);
            }

            
            for(sal_uInt32 a(0L); a < aTempPointVectorEdge.size(); a++)
            {
                const temporaryPoint& rTempPoint = aTempPointVectorEdge[a];
                rTempPointsB.push_back(temporaryPoint(rTempPoint.getPoint(), nIndB, rTempPoint.getCut()));
            }
        }

        

        void findEdgeCutsTwoBeziers(
            const B2DCubicBezier& rCubicA,
            const B2DCubicBezier& rCubicB,
            sal_uInt32 nIndA, sal_uInt32 nIndB,
            temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            
            
            
            B2DPolygon aTempPolygonA;
            B2DPolygon aTempPolygonB;
            temporaryPointVector aTempPointVectorA;
            temporaryPointVector aTempPointVectorB;

            
            
            aTempPolygonA.reserve(SUBDIVIDE_FOR_CUT_TEST_COUNT + 8);
            aTempPolygonA.append(rCubicA.getStartPoint());
            rCubicA.adaptiveSubdivideByCount(aTempPolygonA, SUBDIVIDE_FOR_CUT_TEST_COUNT);
            aTempPolygonB.reserve(SUBDIVIDE_FOR_CUT_TEST_COUNT + 8);
            aTempPolygonB.append(rCubicB.getStartPoint());
            rCubicB.adaptiveSubdivideByCount(aTempPolygonB, SUBDIVIDE_FOR_CUT_TEST_COUNT);

            
            findCutsAndTouchesAndCommonForBezier(aTempPolygonA, aTempPolygonB, aTempPointVectorA, aTempPointVectorB);

            if(!aTempPointVectorA.empty())
            {
                
                adaptAndTransferCutsWithBezierSegment(aTempPointVectorA, aTempPolygonA, nIndA, rTempPointsA);
            }

            if(!aTempPointVectorB.empty())
            {
                
                adaptAndTransferCutsWithBezierSegment(aTempPointVectorB, aTempPolygonB, nIndB, rTempPointsB);
            }
        }

        

        void findEdgeCutsOneBezier(
            const B2DCubicBezier& rCubicA,
            sal_uInt32 nInd, temporaryPointVector& rTempPoints)
        {
            
            
            double fDummy;
            const bool bHasAnyExtremum = rCubicA.getMinimumExtremumPosition( fDummy );
            if( !bHasAnyExtremum )
                return;

            
            
            
            B2DPolygon aTempPolygon;
            temporaryPointVector aTempPointVector;

            
            
            aTempPolygon.reserve(SUBDIVIDE_FOR_CUT_TEST_COUNT + 8);
            aTempPolygon.append(rCubicA.getStartPoint());
            rCubicA.adaptiveSubdivideByCount(aTempPolygon, SUBDIVIDE_FOR_CUT_TEST_COUNT);
            findCuts(aTempPolygon, aTempPointVector);

            if(!aTempPointVector.empty())
            {
                
                adaptAndTransferCutsWithBezierSegment(aTempPointVector, aTempPolygon, nInd, rTempPoints);
            }
        }

        

        void findCuts(const B2DPolygon& rCandidate, temporaryPointVector& rTempPoints)
        {
            
            
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount)
            {
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1L);

                if(nEdgeCount)
                {
                    const bool bCurvesInvolved(rCandidate.areControlPointsUsed());

                    if(bCurvesInvolved)
                    {
                        B2DCubicBezier aCubicA;
                        B2DCubicBezier aCubicB;

                        for(sal_uInt32 a(0L); a < nEdgeCount - 1L; a++)
                        {
                            rCandidate.getBezierSegment(a, aCubicA);
                            aCubicA.testAndSolveTrivialBezier();
                            const bool bEdgeAIsCurve(aCubicA.isBezier());
                            const B2DRange aRangeA(aCubicA.getRange());

                            if(bEdgeAIsCurve)
                            {
                                
                                findEdgeCutsOneBezier(aCubicA, a, rTempPoints);
                            }

                            for(sal_uInt32 b(a + 1L); b < nEdgeCount; b++)
                            {
                                rCandidate.getBezierSegment(b, aCubicB);
                                aCubicB.testAndSolveTrivialBezier();
                                const B2DRange aRangeB(aCubicB.getRange());

                                
                                
                                bool bOverlap = false;
                                if( b > a+1)
                                    bOverlap = aRangeA.overlaps(aRangeB);
                                else
                                    bOverlap = aRangeA.overlapsMore(aRangeB);
                                if( bOverlap)
                                {
                                    const bool bEdgeBIsCurve(aCubicB.isBezier());
                                    if(bEdgeAIsCurve && bEdgeBIsCurve)
                                    {
                                        
                                        findEdgeCutsTwoBeziers(aCubicA, aCubicB, a, b, rTempPoints, rTempPoints);
                                    }
                                    else if(bEdgeAIsCurve)
                                    {
                                        
                                        findEdgeCutsBezierAndEdge(aCubicA, aCubicB.getStartPoint(), aCubicB.getEndPoint(), a, b, rTempPoints, rTempPoints);
                                    }
                                    else if(bEdgeBIsCurve)
                                    {
                                        
                                        findEdgeCutsBezierAndEdge(aCubicB, aCubicA.getStartPoint(), aCubicA.getEndPoint(), b, a, rTempPoints, rTempPoints);
                                    }
                                    else
                                    {
                                        
                                        findEdgeCutsTwoEdges(aCubicA.getStartPoint(), aCubicA.getEndPoint(), aCubicB.getStartPoint(), aCubicB.getEndPoint(),
                                            a, b, rTempPoints, rTempPoints);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        B2DPoint aCurrA(rCandidate.getB2DPoint(0L));

                        for(sal_uInt32 a(0L); a < nEdgeCount - 1L; a++)
                        {
                            const B2DPoint aNextA(rCandidate.getB2DPoint(a + 1L == nPointCount ? 0L : a + 1L));
                            const B2DRange aRangeA(aCurrA, aNextA);
                            B2DPoint aCurrB(rCandidate.getB2DPoint(a + 1L));

                            for(sal_uInt32 b(a + 1L); b < nEdgeCount; b++)
                            {
                                const B2DPoint aNextB(rCandidate.getB2DPoint(b + 1L == nPointCount ? 0L : b + 1L));
                                const B2DRange aRangeB(aCurrB, aNextB);

                                
                                bool bOverlap = false;
                                if( b > a+1)
                                    bOverlap = aRangeA.overlaps(aRangeB);
                                else
                                    bOverlap = aRangeA.overlapsMore(aRangeB);
                                if( bOverlap)
                                {
                                    findEdgeCutsTwoEdges(aCurrA, aNextA, aCurrB, aNextB, a, b, rTempPoints, rTempPoints);
                                }

                                
                                aCurrB = aNextB;
                            }

                            
                            aCurrA = aNextA;
                        }
                    }
                }
            }
        }

        

    } 
} 



namespace basegfx
{
    namespace
    {
        

        void findTouchesOnEdge(
            const B2DPoint& rCurr, const B2DPoint& rNext, const B2DPolygon& rPointPolygon,
            sal_uInt32 nInd, temporaryPointVector& rTempPoints)
        {
            
            
            const sal_uInt32 nPointCount(rPointPolygon.count());

            if(nPointCount)
            {
                const B2DRange aRange(rCurr, rNext);
                const B2DVector aEdgeVector(rNext - rCurr);
                B2DVector aNormalizedEdgeVector(aEdgeVector);
                aNormalizedEdgeVector.normalize();
                bool bTestUsingX(fabs(aEdgeVector.getX()) > fabs(aEdgeVector.getY()));

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DPoint aTestPoint(rPointPolygon.getB2DPoint(a));

                    if(aRange.isInside(aTestPoint))
                    {
                        if(!aTestPoint.equal(rCurr) && !aTestPoint.equal(rNext))
                        {
                            const B2DVector aTestVector(aTestPoint - rCurr);

                            if(areParallel(aNormalizedEdgeVector, aTestVector))
                            {
                                const double fCut((bTestUsingX)
                                    ? aTestVector.getX() / aEdgeVector.getX()
                                    : aTestVector.getY() / aEdgeVector.getY());
                                const double fZero(0.0);
                                const double fOne(1.0);

                                if(fTools::more(fCut, fZero) && fTools::less(fCut, fOne))
                                {
                                    rTempPoints.push_back(temporaryPoint(aTestPoint, nInd, fCut));
                                }
                            }
                        }
                    }
                }
            }
        }

        

        void findTouchesOnCurve(
            const B2DCubicBezier& rCubicA, const B2DPolygon& rPointPolygon,
            sal_uInt32 nInd, temporaryPointVector& rTempPoints)
        {
            
            
            
            B2DPolygon aTempPolygon;
            temporaryPointVector aTempPointVector;

            
            
            aTempPolygon.reserve(SUBDIVIDE_FOR_CUT_TEST_COUNT + 8);
            aTempPolygon.append(rCubicA.getStartPoint());
            rCubicA.adaptiveSubdivideByCount(aTempPolygon, SUBDIVIDE_FOR_CUT_TEST_COUNT);
            findTouches(aTempPolygon, rPointPolygon, aTempPointVector);

            if(!aTempPointVector.empty())
            {
                
                adaptAndTransferCutsWithBezierSegment(aTempPointVector, aTempPolygon, nInd, rTempPoints);
            }
        }

        

        void findTouches(const B2DPolygon& rEdgePolygon, const B2DPolygon& rPointPolygon, temporaryPointVector& rTempPoints)
        {
            
            
            const sal_uInt32 nPointCount(rPointPolygon.count());
            const sal_uInt32 nEdgePointCount(rEdgePolygon.count());

            if(nPointCount && nEdgePointCount)
            {
                const sal_uInt32 nEdgeCount(rEdgePolygon.isClosed() ? nEdgePointCount : nEdgePointCount - 1L);
                B2DPoint aCurr(rEdgePolygon.getB2DPoint(0));

                for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                {
                    const sal_uInt32 nNextIndex((a + 1) % nEdgePointCount);
                    const B2DPoint aNext(rEdgePolygon.getB2DPoint(nNextIndex));

                    if(!aCurr.equal(aNext))
                    {
                        bool bHandleAsSimpleEdge(true);

                        if(rEdgePolygon.areControlPointsUsed())
                        {
                            const B2DPoint aNextControlPoint(rEdgePolygon.getNextControlPoint(a));
                            const B2DPoint aPrevControlPoint(rEdgePolygon.getPrevControlPoint(nNextIndex));
                            const bool bEdgeIsCurve(!aNextControlPoint.equal(aCurr) || !aPrevControlPoint.equal(aNext));

                            if(bEdgeIsCurve)
                            {
                                bHandleAsSimpleEdge = false;
                                const B2DCubicBezier aCubicA(aCurr, aNextControlPoint, aPrevControlPoint, aNext);
                                findTouchesOnCurve(aCubicA, rPointPolygon, a, rTempPoints);
                            }
                        }

                        if(bHandleAsSimpleEdge)
                        {
                            findTouchesOnEdge(aCurr, aNext, rPointPolygon, a, rTempPoints);
                        }
                    }

                    
                    aCurr = aNext;
                }
            }
        }

        

    } 
} 



namespace basegfx
{
    namespace
    {
        

        void findCuts(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB, temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            
            
            const sal_uInt32 nPointCountA(rCandidateA.count());
            const sal_uInt32 nPointCountB(rCandidateB.count());

            if(nPointCountA && nPointCountB)
            {
                const sal_uInt32 nEdgeCountA(rCandidateA.isClosed() ? nPointCountA : nPointCountA - 1L);
                const sal_uInt32 nEdgeCountB(rCandidateB.isClosed() ? nPointCountB : nPointCountB - 1L);

                if(nEdgeCountA && nEdgeCountB)
                {
                    const bool bCurvesInvolved(rCandidateA.areControlPointsUsed() || rCandidateB.areControlPointsUsed());

                    if(bCurvesInvolved)
                    {
                        B2DCubicBezier aCubicA;
                        B2DCubicBezier aCubicB;

                        for(sal_uInt32 a(0L); a < nEdgeCountA; a++)
                        {
                            rCandidateA.getBezierSegment(a, aCubicA);
                            aCubicA.testAndSolveTrivialBezier();
                            const bool bEdgeAIsCurve(aCubicA.isBezier());
                            const B2DRange aRangeA(aCubicA.getRange());

                            for(sal_uInt32 b(0L); b < nEdgeCountB; b++)
                            {
                                rCandidateB.getBezierSegment(b, aCubicB);
                                aCubicB.testAndSolveTrivialBezier();
                                const B2DRange aRangeB(aCubicB.getRange());

                                
                                bool bOverlap = false;
                                if( b > a+1)
                                    bOverlap = aRangeA.overlaps(aRangeB);
                                else
                                    bOverlap = aRangeA.overlapsMore(aRangeB);
                                if( bOverlap)
                                {
                                    const bool bEdgeBIsCurve(aCubicB.isBezier());
                                    if(bEdgeAIsCurve && bEdgeBIsCurve)
                                    {
                                        
                                        findEdgeCutsTwoBeziers(aCubicA, aCubicB, a, b, rTempPointsA, rTempPointsB);
                                    }
                                    else if(bEdgeAIsCurve)
                                    {
                                        
                                        findEdgeCutsBezierAndEdge(aCubicA, aCubicB.getStartPoint(), aCubicB.getEndPoint(), a, b, rTempPointsA, rTempPointsB);
                                    }
                                    else if(bEdgeBIsCurve)
                                    {
                                        
                                        findEdgeCutsBezierAndEdge(aCubicB, aCubicA.getStartPoint(), aCubicA.getEndPoint(), b, a, rTempPointsB, rTempPointsA);
                                    }
                                    else
                                    {
                                        
                                        findEdgeCutsTwoEdges(aCubicA.getStartPoint(), aCubicA.getEndPoint(), aCubicB.getStartPoint(), aCubicB.getEndPoint(),
                                            a, b, rTempPointsA, rTempPointsB);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        B2DPoint aCurrA(rCandidateA.getB2DPoint(0L));

                        for(sal_uInt32 a(0L); a < nEdgeCountA; a++)
                        {
                            const B2DPoint aNextA(rCandidateA.getB2DPoint(a + 1L == nPointCountA ? 0L : a + 1L));
                            const B2DRange aRangeA(aCurrA, aNextA);
                            B2DPoint aCurrB(rCandidateB.getB2DPoint(0L));

                            for(sal_uInt32 b(0L); b < nEdgeCountB; b++)
                            {
                                const B2DPoint aNextB(rCandidateB.getB2DPoint(b + 1L == nPointCountB ? 0L : b + 1L));
                                const B2DRange aRangeB(aCurrB, aNextB);

                                
                                bool bOverlap = false;
                                if( b > a+1)
                                    bOverlap = aRangeA.overlaps(aRangeB);
                                else
                                    bOverlap = aRangeA.overlapsMore(aRangeB);
                                if( bOverlap)
                                {
                                    
                                    findEdgeCutsTwoEdges(aCurrA, aNextA, aCurrB, aNextB, a, b, rTempPointsA, rTempPointsB);
                                }

                                
                                aCurrB = aNextB;
                            }

                            
                            aCurrA = aNextA;
                        }
                    }
                }
            }
        }

        

    } 
} 



namespace basegfx
{
    namespace tools
    {
        

        B2DPolygon addPointsAtCutsAndTouches(const B2DPolygon& rCandidate)
        {
            if(rCandidate.count())
            {
                temporaryPointVector aTempPoints;

                findTouches(rCandidate, rCandidate, aTempPoints);
                findCuts(rCandidate, aTempPoints);

                return mergeTemporaryPointsAndPolygon(rCandidate, aTempPoints);
            }
            else
            {
                return rCandidate;
            }
        }

        

        B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rCandidate, bool bSelfIntersections)
        {
            const sal_uInt32 nCount(rCandidate.count());

            if(nCount)
            {
                B2DPolyPolygon aRetval;

                if(1L == nCount)
                {
                    if(bSelfIntersections)
                    {
                        
                        aRetval.append(addPointsAtCutsAndTouches(rCandidate.getB2DPolygon(0L)));
                    }
                    else
                    {
                        
                        aRetval = rCandidate;
                    }
                }
                else
                {
                    
                    temporaryPolygonData *pTempData = new temporaryPolygonData[nCount];
                    sal_uInt32 a, b;

                    for(a = 0L; a < nCount; a++)
                    {
                        if(bSelfIntersections)
                        {
                            
                            pTempData[a].setPolygon(addPointsAtCutsAndTouches(rCandidate.getB2DPolygon(a)));
                        }
                        else
                        {
                            
                            pTempData[a].setPolygon(rCandidate.getB2DPolygon(a));
                        }
                    }

                    
                    for(a = 0L; a < nCount; a++)
                    {
                        for(b = 0L; b < nCount; b++)
                        {
                            if(a != b)
                            {
                                
                                if(pTempData[a].getRange().overlaps(pTempData[b].getRange()))
                                {
                                    findTouches(pTempData[a].getPolygon(), pTempData[b].getPolygon(), pTempData[a].getTemporaryPointVector());
                                }
                            }

                            if(a < b)
                            {
                                
                                if(pTempData[a].getRange().overlaps(pTempData[b].getRange()))
                                {
                                    findCuts(pTempData[a].getPolygon(), pTempData[b].getPolygon(), pTempData[a].getTemporaryPointVector(), pTempData[b].getTemporaryPointVector());
                                }
                            }
                        }
                    }

                    
                    for(a = 0L; a < nCount; a++)
                    {
                        aRetval.append(mergeTemporaryPointsAndPolygon(pTempData[a].getPolygon(), pTempData[a].getTemporaryPointVector()));
                    }

                    delete[] pTempData;
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        

        B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate, const B2DPoint& rStart, const B2DPoint& rEnd)
        {
            const sal_uInt32 nCount(rCandidate.count());

            if(nCount && !rStart.equal(rEnd))
            {
                const B2DRange aPolygonRange(rCandidate.getB2DRange());
                const B2DRange aEdgeRange(rStart, rEnd);

                if(aPolygonRange.overlaps(aEdgeRange))
                {
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nCount : nCount - 1);
                    temporaryPointVector aTempPoints;
                    temporaryPointVector aUnusedTempPoints;
                    B2DCubicBezier aCubic;

                    for(sal_uInt32 a(0); a < nEdgeCount; a++)
                    {
                        rCandidate.getBezierSegment(a, aCubic);
                        B2DRange aCubicRange(aCubic.getStartPoint(), aCubic.getEndPoint());

                        if(aCubic.isBezier())
                        {
                            aCubicRange.expand(aCubic.getControlPointA());
                            aCubicRange.expand(aCubic.getControlPointB());

                            if(aCubicRange.overlaps(aEdgeRange))
                            {
                                findEdgeCutsBezierAndEdge(aCubic, rStart, rEnd, a, 0, aTempPoints, aUnusedTempPoints);
                            }
                        }
                        else
                        {
                            if(aCubicRange.overlaps(aEdgeRange))
                            {
                                findEdgeCutsTwoEdges(aCubic.getStartPoint(), aCubic.getEndPoint(), rStart, rEnd, a, 0, aTempPoints, aUnusedTempPoints);
                            }
                        }
                    }

                    return mergeTemporaryPointsAndPolygon(rCandidate, aTempPoints);
                }
            }

            return rCandidate;
        }

        

        B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate, const B2DPolyPolygon& rPolyMask)
        {
            const sal_uInt32 nCountA(rCandidate.count());
            const sal_uInt32 nCountM(rPolyMask.count());

            if(nCountA && nCountM)
            {
                const B2DRange aRangeA(rCandidate.getB2DRange());
                const B2DRange aRangeM(rPolyMask.getB2DRange());

                if(aRangeA.overlaps(aRangeM))
                {
                    const sal_uInt32 nEdgeCountA(rCandidate.isClosed() ? nCountA : nCountA - 1);
                    temporaryPointVector aTempPointsA;
                    temporaryPointVector aUnusedTempPointsB;

                    for(sal_uInt32 m(0); m < nCountM; m++)
                    {
                        const B2DPolygon aMask(rPolyMask.getB2DPolygon(m));
                        const sal_uInt32 nCountB(aMask.count());

                        if(nCountB)
                        {
                            B2DCubicBezier aCubicA;
                            B2DCubicBezier aCubicB;

                            for(sal_uInt32 a(0); a < nEdgeCountA; a++)
                            {
                                rCandidate.getBezierSegment(a, aCubicA);
                                const bool bCubicAIsCurve(aCubicA.isBezier());
                                B2DRange aCubicRangeA(aCubicA.getStartPoint(), aCubicA.getEndPoint());

                                if(bCubicAIsCurve)
                                {
                                    aCubicRangeA.expand(aCubicA.getControlPointA());
                                    aCubicRangeA.expand(aCubicA.getControlPointB());
                                }

                                for(sal_uInt32 b(0); b < nCountB; b++)
                                {
                                    aMask.getBezierSegment(b, aCubicB);
                                    const bool bCubicBIsCurve(aCubicB.isBezier());
                                    B2DRange aCubicRangeB(aCubicB.getStartPoint(), aCubicB.getEndPoint());

                                    if(bCubicBIsCurve)
                                    {
                                        aCubicRangeB.expand(aCubicB.getControlPointA());
                                        aCubicRangeB.expand(aCubicB.getControlPointB());
                                    }

                                    if(aCubicRangeA.overlaps(aCubicRangeB))
                                    {
                                        if(bCubicAIsCurve && bCubicBIsCurve)
                                        {
                                            findEdgeCutsTwoBeziers(aCubicA, aCubicB, a, b, aTempPointsA, aUnusedTempPointsB);
                                        }
                                        else if(bCubicAIsCurve)
                                        {
                                            findEdgeCutsBezierAndEdge(aCubicA, aCubicB.getStartPoint(), aCubicB.getEndPoint(), a, b, aTempPointsA, aUnusedTempPointsB);
                                        }
                                        else if(bCubicBIsCurve)
                                        {
                                            findEdgeCutsBezierAndEdge(aCubicB, aCubicA.getStartPoint(), aCubicA.getEndPoint(), b, a, aUnusedTempPointsB, aTempPointsA);
                                        }
                                        else
                                        {
                                            findEdgeCutsTwoEdges(aCubicA.getStartPoint(), aCubicA.getEndPoint(), aCubicB.getStartPoint(), aCubicB.getEndPoint(), a, b, aTempPointsA, aUnusedTempPointsB);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    return mergeTemporaryPointsAndPolygon(rCandidate, aTempPointsA);
                }
            }

            return rCandidate;
        }

        

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
