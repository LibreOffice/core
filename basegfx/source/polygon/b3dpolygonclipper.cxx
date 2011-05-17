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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"

#include <basegfx/polygon/b3dpolygonclipper.hxx>
#include <osl/diagnose.h>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/range/b3drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        inline bool impIsInside(const B3DPoint& rCandidate, double fPlaneOffset, tools::B3DOrientation ePlaneOrthogonal)
        {
            if(tools::B3DORIENTATION_X == ePlaneOrthogonal)
            {
                return fTools::moreOrEqual(rCandidate.getX(), fPlaneOffset);
            }
            else if(tools::B3DORIENTATION_Y == ePlaneOrthogonal)
            {
                return fTools::moreOrEqual(rCandidate.getY(), fPlaneOffset);
            }
            else
            {
                return fTools::moreOrEqual(rCandidate.getZ(), fPlaneOffset);
            }
        }

        inline double impGetCut(const B3DPoint& rCurrent, const B3DPoint& rNext, double fPlaneOffset, tools::B3DOrientation ePlaneOrthogonal)
        {
            if(tools::B3DORIENTATION_X == ePlaneOrthogonal)
            {
                return ((fPlaneOffset - rCurrent.getX())/(rNext.getX() - rCurrent.getX()));
            }
            else if(tools::B3DORIENTATION_Y == ePlaneOrthogonal)
            {
                return ((fPlaneOffset - rCurrent.getY())/(rNext.getY() - rCurrent.getY()));
            }
            else
            {
                return ((fPlaneOffset - rCurrent.getZ())/(rNext.getZ() - rCurrent.getZ()));
            }
        }

        void impAppendCopy(B3DPolygon& rDest, const B3DPolygon& rSource, sal_uInt32 nIndex)
        {
            rDest.append(rSource.getB3DPoint(nIndex));

            if(rSource.areBColorsUsed())
            {
                rDest.setBColor(rDest.count() - 1L, rSource.getBColor(nIndex));
            }

            if(rSource.areNormalsUsed())
            {
                rDest.setNormal(rDest.count() - 1L, rSource.getNormal(nIndex));
            }

            if(rSource.areTextureCoordinatesUsed())
            {
                rDest.setTextureCoordinate(rDest.count() - 1L, rSource.getTextureCoordinate(nIndex));
            }
        }

        void impAppendInterpolate(B3DPolygon& rDest, const B3DPolygon& rSource, sal_uInt32 nIndA, sal_uInt32 nIndB, double fCut)
        {
            const B3DPoint aCurrPoint(rSource.getB3DPoint(nIndA));
            const B3DPoint aNextPoint(rSource.getB3DPoint(nIndB));
            rDest.append(interpolate(aCurrPoint, aNextPoint, fCut));

            if(rSource.areBColorsUsed())
            {
                const BColor aCurrBColor(rSource.getBColor(nIndA));
                const BColor aNextBColor(rSource.getBColor(nIndB));
                rDest.setBColor(rDest.count() - 1L, interpolate(aCurrBColor, aNextBColor, fCut));
            }

            if(rSource.areNormalsUsed())
            {
                const B3DVector aCurrVector(rSource.getNormal(nIndA));
                const B3DVector aNextVector(rSource.getNormal(nIndB));
                rDest.setNormal(rDest.count() - 1L, interpolate(aCurrVector, aNextVector, fCut));
            }

            if(rSource.areTextureCoordinatesUsed())
            {
                const B2DPoint aCurrTxCo(rSource.getTextureCoordinate(nIndA));
                const B2DPoint aNextTxCo(rSource.getTextureCoordinate(nIndB));
                rDest.setTextureCoordinate(rDest.count() - 1L, interpolate(aCurrTxCo, aNextTxCo, fCut));
            }
        }
    }
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        B3DPolyPolygon clipPolygonOnOrthogonalPlane(const B3DPolygon& rCandidate, B3DOrientation ePlaneOrthogonal, bool bClipPositive, double fPlaneOffset, bool bStroke)
        {
            B3DPolyPolygon aRetval;

            if(rCandidate.count())
            {
                const B3DRange aCandidateRange(getRange(rCandidate));

                if(B3DORIENTATION_X == ePlaneOrthogonal && fTools::moreOrEqual(aCandidateRange.getMinX(), fPlaneOffset))
                {
                    // completely above and on the clip plane.
                    if(bClipPositive)
                    {
                        // add completely
                        aRetval.append(rCandidate);
                    }
                }
                else if(B3DORIENTATION_X == ePlaneOrthogonal && fTools::lessOrEqual(aCandidateRange.getMaxX(), fPlaneOffset))
                {
                    // completely below and on the clip plane.
                    if(!bClipPositive)
                    {
                        // add completely
                        aRetval.append(rCandidate);
                    }
                }
                else if(B3DORIENTATION_Y == ePlaneOrthogonal && fTools::moreOrEqual(aCandidateRange.getMinY(), fPlaneOffset))
                {
                    // completely above and on the clip plane.
                    if(bClipPositive)
                    {
                        // add completely
                        aRetval.append(rCandidate);
                    }
                }
                else if(B3DORIENTATION_Y == ePlaneOrthogonal && fTools::lessOrEqual(aCandidateRange.getMaxY(), fPlaneOffset))
                {
                    // completely below and on the clip plane.
                    if(!bClipPositive)
                    {
                        // add completely
                        aRetval.append(rCandidate);
                    }
                }
                else if(B3DORIENTATION_Z == ePlaneOrthogonal && fTools::moreOrEqual(aCandidateRange.getMinZ(), fPlaneOffset))
                {
                    // completely above and on the clip plane.
                    if(bClipPositive)
                    {
                        // add completely
                        aRetval.append(rCandidate);
                    }
                }
                else if(B3DORIENTATION_Z == ePlaneOrthogonal && fTools::lessOrEqual(aCandidateRange.getMaxZ(), fPlaneOffset))
                {
                    // completely below and on the clip plane.
                    if(!bClipPositive)
                    {
                        // add completely
                        aRetval.append(rCandidate);
                    }
                }
                else
                {
                    // prepare loop(s)
                    B3DPolygon aNewPolygon;
                    B3DPoint aCurrent(rCandidate.getB3DPoint(0L));
                    const sal_uInt32 nPointCount(rCandidate.count());
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1L);
                    bool bCurrentInside(impIsInside(aCurrent, fPlaneOffset, ePlaneOrthogonal) == bClipPositive);

                    if(bCurrentInside)
                    {
                        impAppendCopy(aNewPolygon, rCandidate, 0L);
                    }

                    if(bStroke)
                    {
                        // open polygon, create clipped line snippets.
                        for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                        {
                            // get next point data
                            const sal_uInt32 nNextIndex((a + 1L == nPointCount) ? 0L : a + 1L);
                            const B3DPoint aNext(rCandidate.getB3DPoint(nNextIndex));
                            const bool bNextInside(impIsInside(aNext, fPlaneOffset, ePlaneOrthogonal) == bClipPositive);

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
                                const double fCut(impGetCut(aCurrent, aNext, fPlaneOffset, ePlaneOrthogonal));
                                impAppendInterpolate(aNewPolygon, rCandidate, a, nNextIndex, fCut);

                                // pepare next step
                                bCurrentInside = bNextInside;
                            }

                            if(bNextInside)
                            {
                                impAppendCopy(aNewPolygon, rCandidate, nNextIndex);
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
                            const B3DPoint aNext(rCandidate.getB3DPoint(nNextIndex));
                            const bool bNextInside(impIsInside(aNext, fPlaneOffset, ePlaneOrthogonal) == bClipPositive);

                            if(bCurrentInside != bNextInside)
                            {
                                // calculate and add cut point
                                const double fCut(impGetCut(aCurrent, aNext, fPlaneOffset, ePlaneOrthogonal));
                                impAppendInterpolate(aNewPolygon, rCandidate, a, nNextIndex, fCut);

                                // pepare next step
                                bCurrentInside = bNextInside;
                            }

                            if(bNextInside && nNextIndex)
                            {
                                impAppendCopy(aNewPolygon, rCandidate, nNextIndex);
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

        B3DPolyPolygon clipPolyPolygonOnOrthogonalPlane(const B3DPolyPolygon& rCandidate, B3DOrientation ePlaneOrthogonal, bool bClipPositive, double fPlaneOffset, bool bStroke)
        {
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(clipPolygonOnOrthogonalPlane(rCandidate.getB3DPolygon(a), ePlaneOrthogonal, bClipPositive, fPlaneOffset, bStroke));
            }

            return aRetval;
        }

        B3DPolyPolygon clipPolyPolygonOnRange(const B3DPolyPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke)
        {
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(clipPolygonOnRange(rCandidate.getB3DPolygon(a), rRange, bInside, bStroke));
            }

            return aRetval;
        }

        B3DPolyPolygon clipPolygonOnRange(const B3DPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke)
        {
            B3DPolyPolygon aRetval;

            if(rRange.isEmpty())
            {
                // clipping against an empty range. Nothing is inside an empty range, so the polygon
                // is outside the range. So only return if not inside is wanted
                if(!bInside && rCandidate.count())
                {
                    aRetval.append(rCandidate);
                }
            }
            else if(rCandidate.count())
            {
                const B3DRange aCandidateRange3D(getRange(rCandidate));
                const B2DRange aCandidateRange(
                    aCandidateRange3D.getMinX(), aCandidateRange3D.getMinY(),
                    aCandidateRange3D.getMaxX(), aCandidateRange3D.getMaxY());

                if(rRange.isInside(aCandidateRange))
                {
                    // candidate is completely inside given range, nothing to do. Is also true with curves.
                    if(bInside)
                    {
                        aRetval.append(rCandidate);
                    }
                }
                else if(!rRange.overlaps(aCandidateRange))
                {
                    // candidate is completely outside given range, nothing to do. Is also true with curves.
                    if(!bInside)
                    {
                        aRetval.append(rCandidate);
                    }
                }
                else
                {
                    // clip against the six planes of the range
                    // against lower X
                    aRetval = clipPolygonOnOrthogonalPlane(rCandidate, tools::B3DORIENTATION_X, bInside, rRange.getMinX(), bStroke);

                    if(aRetval.count())
                    {
                        // against lower Y
                        if(1L == aRetval.count())
                        {
                            aRetval = clipPolygonOnOrthogonalPlane(aRetval.getB3DPolygon(0L), tools::B3DORIENTATION_Y, bInside, rRange.getMinY(), bStroke);
                        }
                        else
                        {
                            aRetval = clipPolyPolygonOnOrthogonalPlane(aRetval, tools::B3DORIENTATION_Y, bInside, rRange.getMinY(), bStroke);
                        }

                        if(aRetval.count())
                        {
                            // against higher X
                            if(1L == aRetval.count())
                            {
                                aRetval = clipPolygonOnOrthogonalPlane(aRetval.getB3DPolygon(0L), tools::B3DORIENTATION_X, !bInside, rRange.getMaxX(), bStroke);
                            }
                            else
                            {
                                aRetval = clipPolyPolygonOnOrthogonalPlane(aRetval, tools::B3DORIENTATION_X, !bInside, rRange.getMaxX(), bStroke);
                            }

                            if(aRetval.count())
                            {
                                // against higher Y
                                if(1L == aRetval.count())
                                {
                                    aRetval = clipPolygonOnOrthogonalPlane(aRetval.getB3DPolygon(0L), tools::B3DORIENTATION_Y, !bInside, rRange.getMaxY(), bStroke);
                                }
                                else
                                {
                                    aRetval = clipPolyPolygonOnOrthogonalPlane(aRetval, tools::B3DORIENTATION_Y, !bInside, rRange.getMaxY(), bStroke);
                                }
                            }
                        }
                    }
                }
            }

            return aRetval;
        }

        B3DPolyPolygon clipPolyPolygonOnRange(const B3DPolyPolygon& rCandidate, const B3DRange& rRange, bool bInside, bool bStroke)
        {
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(clipPolygonOnRange(rCandidate.getB3DPolygon(a), rRange, bInside, bStroke));
            }

            return aRetval;
        }

        B3DPolyPolygon clipPolygonOnRange(const B3DPolygon& rCandidate, const B3DRange& rRange, bool bInside, bool bStroke)
        {
            B3DPolyPolygon aRetval;

            if(rRange.isEmpty())
            {
                // clipping against an empty range. Nothing is inside an empty range, so the polygon
                // is outside the range. So only return if not inside is wanted
                if(!bInside && rCandidate.count())
                {
                    aRetval.append(rCandidate);
                }
            }
            else if(rCandidate.count())
            {
                const B3DRange aCandidateRange(getRange(rCandidate));

                if(rRange.isInside(aCandidateRange))
                {
                    // candidate is completely inside given range, nothing to do. Is also true with curves.
                    if(bInside)
                    {
                        aRetval.append(rCandidate);
                    }
                }
                else if(!rRange.overlaps(aCandidateRange))
                {
                    // candidate is completely outside given range, nothing to do. Is also true with curves.
                    if(!bInside)
                    {
                        aRetval.append(rCandidate);
                    }
                }
                else
                {
                    // clip against X,Y first and see if there's something left
                    const B2DRange aCandidateRange2D(rRange.getMinX(), rRange.getMinY(), rRange.getMaxX(), rRange.getMaxY());
                    aRetval = clipPolygonOnRange(rCandidate, aCandidateRange2D, bInside, bStroke);

                    if(aRetval.count())
                    {
                        // against lower Z
                        if(1L == aRetval.count())
                        {
                            aRetval = clipPolygonOnOrthogonalPlane(aRetval.getB3DPolygon(0L), tools::B3DORIENTATION_Z, bInside, rRange.getMinZ(), bStroke);
                        }
                        else
                        {
                            aRetval = clipPolyPolygonOnOrthogonalPlane(aRetval, tools::B3DORIENTATION_Z, bInside, rRange.getMinZ(), bStroke);
                        }

                        if(aRetval.count())
                        {
                            // against higher Z
                            if(1L == aRetval.count())
                            {
                                aRetval = clipPolygonOnOrthogonalPlane(aRetval.getB3DPolygon(0L), tools::B3DORIENTATION_Z, !bInside, rRange.getMaxZ(), bStroke);
                            }
                            else
                            {
                                aRetval = clipPolyPolygonOnOrthogonalPlane(aRetval, tools::B3DORIENTATION_Z, !bInside, rRange.getMaxZ(), bStroke);
                            }
                        }
                    }
                }
            }

            return aRetval;
        }

        B3DPolyPolygon clipPolygonOnPlane(const B3DPolygon& rCandidate, const B3DPoint& rPointOnPlane, const B3DVector& rPlaneNormal, bool bClipPositive, bool bStroke)
        {
            B3DPolyPolygon aRetval;

            if(rPlaneNormal.equalZero())
            {
                // not really a plane definition, return polygon
                aRetval.append(rCandidate);
            }
            else if(rCandidate.count())
            {
                // build transform to project planeNormal on X-Axis and pointOnPlane to null point
                B3DHomMatrix aMatrixTransform;
                aMatrixTransform.translate(-rPointOnPlane.getX(), -rPointOnPlane.getY(), -rPointOnPlane.getZ());
                const double fRotInXY(atan2(rPlaneNormal.getY(), rPlaneNormal.getX()));
                const double fRotInXZ(atan2(-rPlaneNormal.getZ(), rPlaneNormal.getXYLength()));
                if(!fTools::equalZero(fRotInXY) || !fTools::equalZero(fRotInXZ))
                {
                    aMatrixTransform.rotate(0.0, fRotInXZ, fRotInXY);
                }

                // transform polygon to clip scenario
                B3DPolygon aCandidate(rCandidate);
                aCandidate.transform(aMatrixTransform);

                // clip on YZ plane
                aRetval = clipPolygonOnOrthogonalPlane(aCandidate, tools::B3DORIENTATION_X, bClipPositive, 0.0, bStroke);

                if(aRetval.count())
                {
                    // if there is a result, it needs to be transformed back
                    aMatrixTransform.invert();
                    aRetval.transform(aMatrixTransform);
                }
            }

            return aRetval;
        }

        B3DPolyPolygon clipPolyPolygonOnPlane(const B3DPolyPolygon& rCandidate, const B3DPoint& rPointOnPlane, const B3DVector& rPlaneNormal, bool bClipPositive, bool bStroke)
        {
            B3DPolyPolygon aRetval;

            if(rPlaneNormal.equalZero())
            {
                // not really a plane definition, return polygon
                aRetval = rCandidate;
            }
            else if(rCandidate.count())
            {
                // build transform to project planeNormal on X-Axis and pointOnPlane to null point
                B3DHomMatrix aMatrixTransform;
                aMatrixTransform.translate(-rPointOnPlane.getX(), -rPointOnPlane.getY(), -rPointOnPlane.getZ());
                const double fRotInXY(atan2(rPlaneNormal.getY(), rPlaneNormal.getX()));
                const double fRotInXZ(atan2(-rPlaneNormal.getZ(), rPlaneNormal.getXYLength()));
                if(!fTools::equalZero(fRotInXY) || !fTools::equalZero(fRotInXZ))
                {
                    aMatrixTransform.rotate(0.0, fRotInXZ, fRotInXY);
                }

                // transform polygon to clip scenario
                aRetval = rCandidate;
                aRetval.transform(aMatrixTransform);

                // clip on YZ plane
                aRetval = clipPolyPolygonOnOrthogonalPlane(aRetval, tools::B3DORIENTATION_X, bClipPositive, 0.0, bStroke);

                if(aRetval.count())
                {
                    // if there is a result, it needs to be transformed back
                    aMatrixTransform.invert();
                    aRetval.transform(aMatrixTransform);
                }
            }

            return aRetval;
        }

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
