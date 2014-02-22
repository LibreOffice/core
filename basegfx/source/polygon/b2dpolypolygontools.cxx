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

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <osl/diagnose.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <numeric>



namespace basegfx
{
    namespace tools
    {
        B2DPolyPolygon correctOrientations(const B2DPolyPolygon& rCandidate)
        {
            B2DPolyPolygon aRetval(rCandidate);
            const sal_uInt32 nCount(aRetval.count());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));
                const B2VectorOrientation aOrientation(tools::getOrientation(aCandidate));
                sal_uInt32 nDepth(0L);

                for(sal_uInt32 b(0L); b < nCount; b++)
                {
                    if(b != a)
                    {
                        const B2DPolygon aCompare(rCandidate.getB2DPolygon(b));

                        if(tools::isInside(aCompare, aCandidate, true))
                        {
                            nDepth++;
                        }
                    }
                }

                const bool bShallBeHole(1L == (nDepth & 0x00000001));
                const bool bIsHole(ORIENTATION_NEGATIVE == aOrientation);

                if(bShallBeHole != bIsHole && ORIENTATION_NEUTRAL != aOrientation)
                {
                    B2DPolygon aFlipped(aCandidate);
                    aFlipped.flip();
                    aRetval.setB2DPolygon(a, aFlipped);
                }
            }

            return aRetval;
        }

        B2DPolyPolygon correctOutmostPolygon(const B2DPolyPolygon& rCandidate)
        {
            const sal_uInt32 nCount(rCandidate.count());

            if(nCount > 1L)
            {
                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));
                    sal_uInt32 nDepth(0L);

                    for(sal_uInt32 b(0L); b < nCount; b++)
                    {
                        if(b != a)
                        {
                            const B2DPolygon aCompare(rCandidate.getB2DPolygon(b));

                            if(tools::isInside(aCompare, aCandidate, true))
                            {
                                nDepth++;
                            }
                        }
                    }

                    if(!nDepth)
                    {
                        B2DPolyPolygon aRetval(rCandidate);

                        if(a != 0L)
                        {
                            
                            aRetval.setB2DPolygon(0L, aCandidate);
                            aRetval.setB2DPolygon(a, rCandidate.getB2DPolygon(0L));
                        }

                        
                        return aRetval;
                    }
                }
            }

            return rCandidate;
        }

        B2DPolyPolygon adaptiveSubdivideByDistance(const B2DPolyPolygon& rCandidate, double fDistanceBound)
        {
            if(rCandidate.areControlPointsUsed())
            {
                const sal_uInt32 nPolygonCount(rCandidate.count());
                B2DPolyPolygon aRetval;

                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                    if(aCandidate.areControlPointsUsed())
                    {
                        aRetval.append(tools::adaptiveSubdivideByDistance(aCandidate, fDistanceBound));
                    }
                    else
                    {
                        aRetval.append(aCandidate);
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolyPolygon adaptiveSubdivideByAngle(const B2DPolyPolygon& rCandidate, double fAngleBound)
        {
            if(rCandidate.areControlPointsUsed())
            {
                const sal_uInt32 nPolygonCount(rCandidate.count());
                B2DPolyPolygon aRetval;

                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                    if(aCandidate.areControlPointsUsed())
                    {
                        aRetval.append(tools::adaptiveSubdivideByAngle(aCandidate, fAngleBound));
                    }
                    else
                    {
                        aRetval.append(aCandidate);
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolyPolygon adaptiveSubdivideByCount(const B2DPolyPolygon& rCandidate, sal_uInt32 nCount)
        {
            if(rCandidate.areControlPointsUsed())
            {
                const sal_uInt32 nPolygonCount(rCandidate.count());
                B2DPolyPolygon aRetval;

                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                    if(aCandidate.areControlPointsUsed())
                    {
                        aRetval.append(tools::adaptiveSubdivideByCount(aCandidate, nCount));
                    }
                    else
                    {
                        aRetval.append(aCandidate);
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        bool isInside(const B2DPolyPolygon& rCandidate, const B2DPoint& rPoint, bool bWithBorder)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());

            if(1L == nPolygonCount)
            {
                return isInside(rCandidate.getB2DPolygon(0L), rPoint, bWithBorder);
            }
            else
            {
                sal_Int32 nInsideCount(0L);

                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    const B2DPolygon aPolygon(rCandidate.getB2DPolygon(a));
                    const bool bInside(isInside(aPolygon, rPoint, bWithBorder));

                    if(bInside)
                    {
                        nInsideCount++;
                    }
                }

                return (nInsideCount % 2L);
            }
        }

        B2DRange getRange(const B2DPolyPolygon& rCandidate)
        {
            B2DRange aRetval;
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B2DPolygon aCandidate = rCandidate.getB2DPolygon(a);
                aRetval.expand(tools::getRange(aCandidate));
            }

            return aRetval;
        }

        double getSignedArea(const B2DPolyPolygon& rCandidate)
        {
            double fRetval(0.0);
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                fRetval += tools::getSignedArea(aCandidate);
            }

            return fRetval;
        }

        double getArea(const B2DPolyPolygon& rCandidate)
        {
            return fabs(getSignedArea(rCandidate));
        }

        void applyLineDashing(const B2DPolyPolygon& rCandidate, const ::std::vector<double>& rDotDashArray, B2DPolyPolygon* pLineTarget, B2DPolyPolygon* pGapTarget, double fFullDashDotLen)
        {
            if(0.0 == fFullDashDotLen && rDotDashArray.size())
            {
                
                fFullDashDotLen = ::std::accumulate(rDotDashArray.begin(), rDotDashArray.end(), 0.0);
            }

            if(rCandidate.count() && fFullDashDotLen > 0.0)
            {
                B2DPolyPolygon aLineTarget, aGapTarget;

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                    applyLineDashing(
                        aCandidate,
                        rDotDashArray,
                        pLineTarget ? &aLineTarget : 0,
                        pGapTarget ? &aGapTarget : 0,
                        fFullDashDotLen);

                    if(pLineTarget)
                    {
                        pLineTarget->append(aLineTarget);
                    }

                    if(pGapTarget)
                    {
                        pGapTarget->append(aGapTarget);
                    }
                }
            }
        }

        bool isInEpsilonRange(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPosition, double fDistance)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                if(isInEpsilonRange(aCandidate, rTestPosition, fDistance))
                {
                    return true;
                }
            }

            return false;
        }

        B3DPolyPolygon createB3DPolyPolygonFromB2DPolyPolygon(const B2DPolyPolygon& rCandidate, double fZCoordinate)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                aRetval.append(createB3DPolygonFromB2DPolygon(aCandidate, fZCoordinate));
            }

            return aRetval;
        }

        B2DPolyPolygon createB2DPolyPolygonFromB3DPolyPolygon(const B3DPolyPolygon& rCandidate, const B3DHomMatrix& rMat)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B3DPolygon aCandidate(rCandidate.getB3DPolygon(a));

                aRetval.append(createB2DPolygonFromB3DPolygon(aCandidate, rMat));
            }

            return aRetval;
        }

        double getSmallestDistancePointToPolyPolygon(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPoint, sal_uInt32& rPolygonIndex, sal_uInt32& rEdgeIndex, double& rCut)
        {
            double fRetval(DBL_MAX);
            const double fZero(0.0);
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));
                sal_uInt32 nNewEdgeIndex;
                double fNewCut;
                const double fNewDistance(getSmallestDistancePointToPolygon(aCandidate, rTestPoint, nNewEdgeIndex, fNewCut));

                if(DBL_MAX == fRetval || fNewDistance < fRetval)
                {
                    fRetval = fNewDistance;
                    rPolygonIndex = a;
                    rEdgeIndex = nNewEdgeIndex;
                    rCut = fNewCut;

                    if(fTools::equal(fRetval, fZero))
                    {
                        
                        fRetval = 0.0;
                        break;
                    }
                }
            }

            return fRetval;
        }

        B2DPolyPolygon distort(const B2DPolyPolygon& rCandidate, const B2DRange& rOriginal, const B2DPoint& rTopLeft, const B2DPoint& rTopRight, const B2DPoint& rBottomLeft, const B2DPoint& rBottomRight)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                aRetval.append(distort(aCandidate, rOriginal, rTopLeft, rTopRight, rBottomLeft, rBottomRight));
            }

            return aRetval;
        }

        B2DPolyPolygon expandToCurve(const B2DPolyPolygon& rCandidate)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                aRetval.append(expandToCurve(aCandidate));
            }

            return aRetval;
        }

        B2DPolyPolygon growInNormalDirection(const B2DPolyPolygon& rCandidate, double fValue)
        {
            if(0.0 != fValue)
            {
                B2DPolyPolygon aRetval;

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    aRetval.append(growInNormalDirection(rCandidate.getB2DPolygon(a), fValue));
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        void correctGrowShrinkPolygonPair(SAL_UNUSED_PARAMETER B2DPolyPolygon& /*rOriginal*/, SAL_UNUSED_PARAMETER B2DPolyPolygon& /*rGrown*/)
        {
            
        }

        B2DPolyPolygon reSegmentPolyPolygon(const B2DPolyPolygon& rCandidate, sal_uInt32 nSegments)
        {
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(reSegmentPolygon(rCandidate.getB2DPolygon(a), nSegments));
            }

            return aRetval;
        }

        B2DPolyPolygon interpolate(const B2DPolyPolygon& rOld1, const B2DPolyPolygon& rOld2, double t)
        {
            OSL_ENSURE(rOld1.count() == rOld2.count(), "B2DPolyPolygon interpolate: Different geometry (!)");
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rOld1.count(); a++)
            {
                aRetval.append(interpolate(rOld1.getB2DPolygon(a), rOld2.getB2DPolygon(a), t));
            }

            return aRetval;
        }

        bool isRectangle( const B2DPolyPolygon& rPoly )
        {
            
            if( rPoly.count() != 1 )
                return false;

            return isRectangle( rPoly.getB2DPolygon(0) );
        }

        
        B2DPolyPolygon simplifyCurveSegments(const B2DPolyPolygon& rCandidate)
        {
            if(rCandidate.areControlPointsUsed())
            {
                B2DPolyPolygon aRetval;

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    aRetval.append(simplifyCurveSegments(rCandidate.getB2DPolygon(a)));
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolyPolygon snapPointsOfHorizontalOrVerticalEdges(const B2DPolyPolygon& rCandidate)
        {
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(snapPointsOfHorizontalOrVerticalEdges(rCandidate.getB2DPolygon(a)));
            }

            return aRetval;
        }

        bool containsOnlyHorizontalAndVerticalEdges(const B2DPolyPolygon& rCandidate)
        {
            if(rCandidate.areControlPointsUsed())
            {
                return false;
            }

            for(sal_uInt32 a(0); a < rCandidate.count(); a++)
            {
                if(!containsOnlyHorizontalAndVerticalEdges(rCandidate.getB2DPolygon(a)))
                {
                    return false;
                }
            }

            return true;
        }

        B2DPolyPolygon createSevenSegmentPolyPolygon(sal_Char nNumber, bool bLitSegments)
        {
            
            
            const double fTotalSize=1.0;
            const double fPosMiddleSegment=0.6;
            const double fSegmentEndChopHoriz=0.08;
            const double fSegmentEndChopVert =0.04;
            
            

            const double fLeft=0.0;
            const double fRight=fTotalSize;
            const double fTop=0.0;
            const double fMiddle=fPosMiddleSegment;
            const double fBottom=fTotalSize;

            
            //
            
            //
            
            
            
            
            
            //
            static const double corners[] =
            {
                fLeft,  fTop,
                fRight, fTop,
                fLeft,  fMiddle,
                fRight, fMiddle,
                fLeft,  fBottom,
                fRight, fBottom
            };

            
            //
            
            
            
            
            
            
            
            
            
            //
            static const int numbers[] =
            {
                1, 1, 1, 0, 1, 1, 1, 
                0, 0, 1, 0, 0, 1, 0, 
                1, 0, 1, 1, 1, 0, 1, 
                1, 0, 1, 1, 0, 1, 1, 
                0, 1, 1, 1, 0, 1, 0, 
                1, 1, 0, 1, 0, 1, 1, 
                1, 1, 0, 1, 1, 1, 1, 
                1, 0, 1, 0, 0, 1, 0, 
                1, 1, 1, 1, 1, 1, 1, 
                1, 1, 1, 1, 0, 1, 1, 
                0, 0, 0, 1, 0, 0, 0, 
                1, 1, 0, 1, 1, 0, 1, 
            };

            
            static const int index2corner[] =
            {
                0, 2,  
                0, 4,  
                2, 6,  
                4, 6,  
                4, 8,  
                6, 10, 
                8, 10, 
            };

            B2DPolyPolygon aRes;
            if( nNumber == '-' )
            {
                nNumber = 10;
            }
            else if( nNumber == 'E' )
            {
                nNumber = 11;
            }
            else if( nNumber == '.' )
            {
                if( bLitSegments )
                    aRes.append(createPolygonFromCircle(B2DPoint(fTotalSize/2, fTotalSize),
                                                        fSegmentEndChopHoriz));
                return aRes;
            }
            else
            {
                nNumber=clamp<sal_uInt32>(nNumber,'0','9') - '0';
            }

            B2DPolygon aCurrSegment;
            const size_t sliceSize=SAL_N_ELEMENTS(numbers)/12;
            const int* pCurrSegment=numbers + nNumber*sliceSize;
            for( size_t i=0; i<sliceSize; i++, pCurrSegment++)
            {
                if( !(*pCurrSegment ^ int(bLitSegments)) )
                {
                    const size_t j=2*i;
                    aCurrSegment.clear();
                    B2DPoint start(corners[index2corner[j]],
                                   corners[index2corner[j]+1]  );
                    B2DPoint end  (corners[index2corner[j+1]],
                                   corners[index2corner[j+1]+1]);

                    if( start.getX() == end.getX() )
                    {
                        start.setY(start.getY()+fSegmentEndChopVert);
                        end.setY(end.getY()-fSegmentEndChopVert);
                    }
                    else
                    {
                        start.setX(start.getX()+fSegmentEndChopHoriz);
                        end.setX(end.getX()-fSegmentEndChopHoriz);
                    }

                    aCurrSegment.append(start);
                    aCurrSegment.append(end);
                }
                aRes.append(aCurrSegment);
            }

            return aRes;
        }

        
        

        B2DPolyPolygon UnoPointSequenceSequenceToB2DPolyPolygon(
            const com::sun::star::drawing::PointSequenceSequence& rPointSequenceSequenceSource,
            bool bCheckClosed)
        {
            B2DPolyPolygon aRetval;
            const com::sun::star::drawing::PointSequence* pPointSequence = rPointSequenceSequenceSource.getConstArray();
            const com::sun::star::drawing::PointSequence* pPointSeqEnd = pPointSequence + rPointSequenceSequenceSource.getLength();

            for(;pPointSequence != pPointSeqEnd; pPointSequence++)
            {
                const B2DPolygon aNewPolygon = UnoPointSequenceToB2DPolygon(*pPointSequence, bCheckClosed);
                aRetval.append(aNewPolygon);
            }

            return aRetval;
        }

        void B2DPolyPolygonToUnoPointSequenceSequence(
            const B2DPolyPolygon& rPolyPolygon,
            com::sun::star::drawing::PointSequenceSequence& rPointSequenceSequenceRetval)
        {
            const sal_uInt32 nCount(rPolyPolygon.count());

            if(nCount)
            {
                rPointSequenceSequenceRetval.realloc(nCount);
                com::sun::star::drawing::PointSequence* pPointSequence = rPointSequenceSequenceRetval.getArray();

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    const B2DPolygon aPolygon(rPolyPolygon.getB2DPolygon(a));

                    B2DPolygonToUnoPointSequence(aPolygon, *pPointSequence);
                    pPointSequence++;
                }
            }
            else
            {
                rPointSequenceSequenceRetval.realloc(0);
            }
        }

        
        

        B2DPolyPolygon UnoPolyPolygonBezierCoordsToB2DPolyPolygon(
            const com::sun::star::drawing::PolyPolygonBezierCoords& rPolyPolygonBezierCoordsSource,
            bool bCheckClosed)
        {
            B2DPolyPolygon aRetval;
            const sal_uInt32 nSequenceCount((sal_uInt32)rPolyPolygonBezierCoordsSource.Coordinates.getLength());

            if(nSequenceCount)
            {
                OSL_ENSURE(nSequenceCount == (sal_uInt32)rPolyPolygonBezierCoordsSource.Flags.getLength(),
                    "UnoPolyPolygonBezierCoordsToB2DPolyPolygon: unequal number of Points and Flags (!)");
                const com::sun::star::drawing::PointSequence* pPointSequence = rPolyPolygonBezierCoordsSource.Coordinates.getConstArray();
                const com::sun::star::drawing::FlagSequence* pFlagSequence = rPolyPolygonBezierCoordsSource.Flags.getConstArray();

                for(sal_uInt32 a(0); a < nSequenceCount; a++)
                {
                    const B2DPolygon aNewPolygon(UnoPolygonBezierCoordsToB2DPolygon(
                        *pPointSequence,
                        *pFlagSequence,
                        bCheckClosed));

                    pPointSequence++;
                    pFlagSequence++;
                    aRetval.append(aNewPolygon);
                }
            }

            return aRetval;
        }

        void B2DPolyPolygonToUnoPolyPolygonBezierCoords(
            const B2DPolyPolygon& rPolyPolygon,
            com::sun::star::drawing::PolyPolygonBezierCoords& rPolyPolygonBezierCoordsRetval)
        {
            const sal_uInt32 nCount(rPolyPolygon.count());

            if(nCount)
            {
                
                rPolyPolygonBezierCoordsRetval.Coordinates.realloc((sal_Int32)nCount);
                rPolyPolygonBezierCoordsRetval.Flags.realloc((sal_Int32)nCount);

                
                com::sun::star::drawing::PointSequence* pPointSequence = rPolyPolygonBezierCoordsRetval.Coordinates.getArray();
                com::sun::star::drawing::FlagSequence*  pFlagSequence = rPolyPolygonBezierCoordsRetval.Flags.getArray();

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    const B2DPolygon aSource(rPolyPolygon.getB2DPolygon(a));

                    B2DPolygonToUnoPolygonBezierCoords(
                        aSource,
                        *pPointSequence,
                        *pFlagSequence);
                    pPointSequence++;
                    pFlagSequence++;
                }
            }
            else
            {
                rPolyPolygonBezierCoordsRetval.Coordinates.realloc(0);
                rPolyPolygonBezierCoordsRetval.Flags.realloc(0);
            }
        }

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
