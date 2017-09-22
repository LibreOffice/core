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

#include <drawinglayer/primitive3d/sdrextrudelathetools3d.hxx>

#include <osl/diagnose.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <basegfx/range/b3drange.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <numeric>


// decomposition helpers for extrude/lathe (rotation) objects

namespace
{

    // common helpers

    basegfx::B2DPolyPolygon impScalePolyPolygonOnCenter(
        const basegfx::B2DPolyPolygon& rSource,
        double fScale)
    {
        basegfx::B2DPolyPolygon aRetval(rSource);

        if(!basegfx::fTools::equalZero(fScale))
        {
            const basegfx::B2DRange aRange(basegfx::utils::getRange(rSource));
            const basegfx::B2DPoint aCenter(aRange.getCenter());
            basegfx::B2DHomMatrix aTrans;

            aTrans.translate(-aCenter.getX(), -aCenter.getY());
            aTrans.scale(fScale, fScale);
            aTrans.translate(aCenter.getX(), aCenter.getY());
            aRetval.transform(aTrans);
        }

        return aRetval;
    }

    void impGetOuterPolyPolygon(
        basegfx::B2DPolyPolygon& rPolygon,
        basegfx::B2DPolyPolygon& rOuterPolyPolygon,
        double fOffset,
        bool bCharacterMode)
    {
        rOuterPolyPolygon = rPolygon;

        if(basegfx::fTools::more(fOffset, 0.0))
        {
            if(bCharacterMode)
            {
                // grow the outside polygon and scale all polygons to original size. This is done
                // to avoid a shrink which potentially would lead to self-intersections, but changes
                // the original polygon -> not a precision step, so e.g. not usable for charts
                const basegfx::B2DRange aRange(basegfx::utils::getRange(rPolygon));
                rPolygon = basegfx::utils::growInNormalDirection(rPolygon, fOffset);
                const basegfx::B2DRange aGrownRange(basegfx::utils::getRange(rPolygon));
                const double fScaleX(basegfx::fTools::equalZero(aGrownRange.getWidth()) ? 1.0 : aRange.getWidth() / aGrownRange.getWidth());
                const double fScaleY(basegfx::fTools::equalZero(aGrownRange.getHeight())? 1.0 : aRange.getHeight() / aGrownRange.getHeight());
                basegfx::B2DHomMatrix aScaleTrans;

                aScaleTrans.translate(-aGrownRange.getMinX(), -aGrownRange.getMinY());
                aScaleTrans.scale(fScaleX, fScaleY);
                aScaleTrans.translate(aRange.getMinX(), aRange.getMinY());
                rPolygon.transform(aScaleTrans);
                rOuterPolyPolygon.transform(aScaleTrans);
            }
            else
            {
                // use more precision, shrink the outer polygons. Since this may lead to self-intersections,
                // some kind of correction should be applied here after that step
                rOuterPolyPolygon = basegfx::utils::growInNormalDirection(rPolygon, -fOffset);
                // basegfx::utils::correctGrowShrinkPolygonPair(rPolygon, rOuterPolyPolygon);
            }
        }
    }

    void impAddInBetweenFill(
        basegfx::B3DPolyPolygon& rTarget,
        const basegfx::B3DPolyPolygon& rPolA,
        const basegfx::B3DPolyPolygon& rPolB,
        double fTexVerStart,
        double fTexVerStop,
        bool bCreateNormals,
        bool bCreateTextureCoordinates)
    {
        OSL_ENSURE(rPolA.count() == rPolB.count(), "impAddInBetweenFill: unequally sized polygons (!)");
        const sal_uInt32 nPolygonCount(std::min(rPolA.count(), rPolB.count()));

        for(sal_uInt32 a(0); a < nPolygonCount; a++)
        {
            const basegfx::B3DPolygon aSubA(rPolA.getB3DPolygon(a));
            const basegfx::B3DPolygon aSubB(rPolB.getB3DPolygon(a));
            OSL_ENSURE(aSubA.count() == aSubB.count(), "impAddInBetweenFill: unequally sized polygons (!)");
            const sal_uInt32 nPointCount(std::min(aSubA.count(), aSubB.count()));

            if(nPointCount)
            {
                const sal_uInt32 nEdgeCount(aSubA.isClosed() ? nPointCount : nPointCount - 1);
                double fTexHorMultiplicatorA(0.0), fTexHorMultiplicatorB(0.0);
                double fPolygonPosA(0.0), fPolygonPosB(0.0);

                if(bCreateTextureCoordinates)
                {
                    const double fPolygonLengthA(basegfx::utils::getLength(aSubA));
                    fTexHorMultiplicatorA = basegfx::fTools::equalZero(fPolygonLengthA) ? 1.0 : 1.0 / fPolygonLengthA;

                    const double fPolygonLengthB(basegfx::utils::getLength(aSubB));
                    fTexHorMultiplicatorB = basegfx::fTools::equalZero(fPolygonLengthB) ? 1.0 : 1.0 / fPolygonLengthB;
                }

                for(sal_uInt32 b(0); b < nEdgeCount; b++)
                {
                    const sal_uInt32 nIndexA(b);
                    const sal_uInt32 nIndexB((b + 1) % nPointCount);

                    const basegfx::B3DPoint aStartA(aSubA.getB3DPoint(nIndexA));
                    const basegfx::B3DPoint aEndA(aSubA.getB3DPoint(nIndexB));
                    const basegfx::B3DPoint aStartB(aSubB.getB3DPoint(nIndexA));
                    const basegfx::B3DPoint aEndB(aSubB.getB3DPoint(nIndexB));

                    basegfx::B3DPolygon aNew;
                    aNew.setClosed(true);

                    aNew.append(aStartA);
                    aNew.append(aStartB);
                    aNew.append(aEndB);
                    aNew.append(aEndA);

                    if(bCreateNormals)
                    {
                        aNew.setNormal(0, aSubA.getNormal(nIndexA));
                        aNew.setNormal(1, aSubB.getNormal(nIndexA));
                        aNew.setNormal(2, aSubB.getNormal(nIndexB));
                        aNew.setNormal(3, aSubA.getNormal(nIndexB));
                    }

                    if(bCreateTextureCoordinates)
                    {
                        const double fRelTexAL(fPolygonPosA * fTexHorMultiplicatorA);
                        const double fEdgeLengthA(basegfx::B3DVector(aEndA - aStartA).getLength());
                        fPolygonPosA += fEdgeLengthA;
                        const double fRelTexAR(fPolygonPosA * fTexHorMultiplicatorA);

                        const double fRelTexBL(fPolygonPosB * fTexHorMultiplicatorB);
                        const double fEdgeLengthB(basegfx::B3DVector(aEndB - aStartB).getLength());
                        fPolygonPosB += fEdgeLengthB;
                        const double fRelTexBR(fPolygonPosB * fTexHorMultiplicatorB);

                        aNew.setTextureCoordinate(0, basegfx::B2DPoint(fRelTexAL, fTexVerStart));
                        aNew.setTextureCoordinate(1, basegfx::B2DPoint(fRelTexBL, fTexVerStop));
                        aNew.setTextureCoordinate(2, basegfx::B2DPoint(fRelTexBR, fTexVerStop));
                        aNew.setTextureCoordinate(3, basegfx::B2DPoint(fRelTexAR, fTexVerStart));
                    }

                    rTarget.append(aNew);
                }
            }
        }
    }

    void impSetNormal(
        basegfx::B3DPolyPolygon& rCandidate,
        const basegfx::B3DVector& rNormal)
    {
        for(sal_uInt32 a(0); a < rCandidate.count(); a++)
        {
            basegfx::B3DPolygon aSub(rCandidate.getB3DPolygon(a));

            for(sal_uInt32 b(0); b < aSub.count(); b++)
            {
                aSub.setNormal(b, rNormal);
            }

            rCandidate.setB3DPolygon(a, aSub);
        }
    }

    void impCreateInBetweenNormals(
        basegfx::B3DPolyPolygon& rPolA,
        basegfx::B3DPolyPolygon& rPolB,
        bool bSmoothHorizontalNormals)
    {
        OSL_ENSURE(rPolA.count() == rPolB.count(), "sdrExtrudePrimitive3D: unequally sized polygons (!)");
        const sal_uInt32 nPolygonCount(std::min(rPolA.count(), rPolB.count()));

        for(sal_uInt32 a(0); a < nPolygonCount; a++)
        {
            basegfx::B3DPolygon aSubA(rPolA.getB3DPolygon(a));
            basegfx::B3DPolygon aSubB(rPolB.getB3DPolygon(a));
            OSL_ENSURE(aSubA.count() == aSubB.count(), "sdrExtrudePrimitive3D: unequally sized polygons (!)");
            const sal_uInt32 nPointCount(std::min(aSubA.count(), aSubB.count()));

            if(nPointCount)
            {
                basegfx::B3DPoint aPrevA(aSubA.getB3DPoint(nPointCount - 1));
                basegfx::B3DPoint aCurrA(aSubA.getB3DPoint(0));
                const bool bClosed(aSubA.isClosed());

                for(sal_uInt32 b(0); b < nPointCount; b++)
                {
                    const sal_uInt32 nIndNext((b + 1) % nPointCount);
                    const basegfx::B3DPoint aNextA(aSubA.getB3DPoint(nIndNext));
                    const basegfx::B3DPoint aCurrB(aSubB.getB3DPoint(b));

                    // vector to back
                    basegfx::B3DVector aDepth(aCurrB - aCurrA);
                    aDepth.normalize();

                    if(aDepth.equalZero())
                    {
                        // no difference, try to get depth from next point
                        const basegfx::B3DPoint aNextB(aSubB.getB3DPoint(nIndNext));
                        aDepth = aNextB - aNextA;
                        aDepth.normalize();
                    }

                    // vector to left (correct for non-closed lines)
                    const bool bFirstAndNotClosed(!bClosed && 0 == b);
                    basegfx::B3DVector aLeft(bFirstAndNotClosed ? aCurrA - aNextA : aPrevA - aCurrA);
                    aLeft.normalize();

                    // create left normal
                    const basegfx::B3DVector aNormalLeft(aDepth.getPerpendicular(aLeft));

                    if(bSmoothHorizontalNormals)
                    {
                        // vector to right (correct for non-closed lines)
                        const bool bLastAndNotClosed(!bClosed && b + 1 == nPointCount);
                        basegfx::B3DVector aRight(bLastAndNotClosed ? aCurrA - aPrevA : aNextA - aCurrA);
                        aRight.normalize();

                        // create right normal
                        const basegfx::B3DVector aNormalRight(aRight.getPerpendicular(aDepth));

                        // create smoothed in-between normal
                        basegfx::B3DVector aNewNormal(aNormalLeft + aNormalRight);
                        aNewNormal.normalize();

                        // set as new normal at polygons
                        aSubA.setNormal(b, aNewNormal);
                        aSubB.setNormal(b, aNewNormal);
                    }
                    else
                    {
                        // set aNormalLeft as new normal at polygons
                        aSubA.setNormal(b, aNormalLeft);
                        aSubB.setNormal(b, aNormalLeft);
                    }

                    // prepare next step
                    aPrevA = aCurrA;
                    aCurrA = aNextA;
                }

                rPolA.setB3DPolygon(a, aSubA);
                rPolB.setB3DPolygon(a, aSubB);
            }
        }
    }

    void impMixNormals(
        basegfx::B3DPolyPolygon& rPolA,
        const basegfx::B3DPolyPolygon& rPolB,
        double fWeightA)
    {
        const double fWeightB(1.0 - fWeightA);
        OSL_ENSURE(rPolA.count() == rPolB.count(), "sdrExtrudePrimitive3D: unequally sized polygons (!)");
        const sal_uInt32 nPolygonCount(std::min(rPolA.count(), rPolB.count()));

        for(sal_uInt32 a(0); a < nPolygonCount; a++)
        {
            basegfx::B3DPolygon aSubA(rPolA.getB3DPolygon(a));
            const basegfx::B3DPolygon aSubB(rPolB.getB3DPolygon(a));
            OSL_ENSURE(aSubA.count() == aSubB.count(), "sdrExtrudePrimitive3D: unequally sized polygons (!)");
            const sal_uInt32 nPointCount(std::min(aSubA.count(), aSubB.count()));

            for(sal_uInt32 b(0); b < nPointCount; b++)
            {
                const basegfx::B3DVector aVA(aSubA.getNormal(b) * fWeightA);
                const basegfx::B3DVector aVB(aSubB.getNormal(b) * fWeightB);
                basegfx::B3DVector aVNew(aVA + aVB);
                aVNew.normalize();
                aSubA.setNormal(b, aVNew);
            }

            rPolA.setB3DPolygon(a, aSubA);
        }
    }

    bool impHasCutWith(const basegfx::B2DPolygon& rPoly, const basegfx::B2DPoint& rStart, const basegfx::B2DPoint& rEnd)
    {
        // polygon is closed, one of the points is a member
        const sal_uInt32 nPointCount(rPoly.count());

        if(nPointCount)
        {
            basegfx::B2DPoint aCurrent(rPoly.getB2DPoint(0));
            const basegfx::B2DVector aVector(rEnd - rStart);

            for(sal_uInt32 a(0); a < nPointCount; a++)
            {
                const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                const basegfx::B2DPoint aNext(rPoly.getB2DPoint(nNextIndex));
                const basegfx::B2DVector aEdgeVector(aNext - aCurrent);

                if(basegfx::utils::findCut(
                    rStart, aVector,
                    aCurrent, aEdgeVector) != CutFlagValue::NONE)
                {
                    return true;
                }

                aCurrent = aNext;
            }
        }

        return false;
    }
} // end of anonymous namespace


namespace drawinglayer
{
    namespace primitive3d
    {
        void createLatheSlices(
            Slice3DVector& rSliceVector,
            const basegfx::B2DPolyPolygon& rSource,
            double fBackScale,
            double fDiagonal,
            double fRotation,
            sal_uInt32 nSteps,
            bool bCharacterMode,
            bool bCloseFront,
            bool bCloseBack)
        {
            if(basegfx::fTools::equalZero(fRotation) || 0 == nSteps)
            {
                // no rotation or no steps, just one plane
                rSliceVector.emplace_back(rSource, basegfx::B3DHomMatrix());
            }
            else
            {
                const bool bBackScale(!basegfx::fTools::equal(fBackScale, 1.0));
                const bool bClosedRotation(!bBackScale && basegfx::fTools::equal(fRotation, F_2PI));
                basegfx::B2DPolyPolygon aFront(rSource);
                basegfx::B2DPolyPolygon aBack(rSource);
                basegfx::B3DHomMatrix aTransformBack;
                basegfx::B2DPolyPolygon aOuterBack;

                if(bClosedRotation)
                {
                    bCloseFront = bCloseBack = false;
                }

                if(bBackScale)
                {
                    // avoid null zoom
                    if(basegfx::fTools::equalZero(fBackScale))
                    {
                        fBackScale = 0.000001;
                    }

                    // back is scaled compared to front, create scaled version
                    aBack = impScalePolyPolygonOnCenter(aBack, fBackScale);
                }

                if(bCloseFront || bCloseBack)
                {
                    const basegfx::B2DRange aBaseRange(basegfx::utils::getRange(aFront));
                    const double fOuterLength(aBaseRange.getMaxX() * fRotation);
                    const double fInnerLength(aBaseRange.getMinX() * fRotation);
                    const double fAverageLength((fOuterLength + fInnerLength) * 0.5);

                    if(bCloseFront)
                    {
                        const double fOffsetLen((fAverageLength / 12.0) * fDiagonal);
                        basegfx::B2DPolyPolygon aOuterFront;
                        impGetOuterPolyPolygon(aFront, aOuterFront, fOffsetLen, bCharacterMode);
                        basegfx::B3DHomMatrix aTransform;
                        aTransform.translate(0.0, 0.0, fOffsetLen);
                        rSliceVector.emplace_back(aOuterFront, aTransform, SLICETYPE3D_FRONTCAP);
                    }

                    if(bCloseBack)
                    {
                        const double fOffsetLen((fAverageLength / 12.0) * fDiagonal);
                        impGetOuterPolyPolygon(aBack, aOuterBack, fOffsetLen, bCharacterMode);
                        aTransformBack.translate(0.0, 0.0, -fOffsetLen);
                        aTransformBack.rotate(0.0, fRotation, 0.0);
                    }
                }

                // add start polygon (a = 0)
                if(!bClosedRotation)
                {
                    rSliceVector.emplace_back(aFront, basegfx::B3DHomMatrix());
                }

                // create segments (a + 1 .. nSteps)
                const double fStepSize(1.0 / (double)nSteps);

                for(sal_uInt32 a(0); a < nSteps; a++)
                {
                    const double fStep((double)(a + 1) * fStepSize);
                    basegfx::B2DPolyPolygon aNewPoly(bBackScale ? basegfx::utils::interpolate(aFront, aBack, fStep) : aFront);
                    basegfx::B3DHomMatrix aNewMat;
                    aNewMat.rotate(0.0, fRotation * fStep, 0.0);
                    rSliceVector.emplace_back(aNewPoly, aNewMat);
                }

                if(bCloseBack)
                {
                    rSliceVector.emplace_back(aOuterBack, aTransformBack, SLICETYPE3D_BACKCAP);
                }
            }
        }

        void createExtrudeSlices(
            Slice3DVector& rSliceVector,
            const basegfx::B2DPolyPolygon& rSource,
            double fBackScale,
            double fDiagonal,
            double fDepth,
            bool bCharacterMode,
            bool bCloseFront,
            bool bCloseBack)
        {
            if(basegfx::fTools::equalZero(fDepth))
            {
                // no depth, just one plane
                rSliceVector.emplace_back(rSource, basegfx::B3DHomMatrix());
            }
            else
            {
                // there is depth, create Polygons for front,back and their default depth positions
                basegfx::B2DPolyPolygon aFront(rSource);
                basegfx::B2DPolyPolygon aBack(rSource);
                const bool bBackScale(!basegfx::fTools::equal(fBackScale, 1.0));
                double fZFront(fDepth); // default depth for aFront
                double fZBack(0.0); // default depth for aBack
                basegfx::B2DPolyPolygon aOuterBack;

                if(bBackScale)
                {
                    // avoid null zoom
                    if(basegfx::fTools::equalZero(fBackScale))
                    {
                        fBackScale = 0.000001;
                    }

                    // aFront is scaled compared to aBack, create scaled version
                    aFront = impScalePolyPolygonOnCenter(aFront, fBackScale);
                }

                if(bCloseFront)
                {
                    const double fOffset(fDepth * fDiagonal * 0.5);
                    fZFront = fDepth - fOffset;
                    basegfx::B2DPolyPolygon aOuterFront;
                    impGetOuterPolyPolygon(aFront, aOuterFront, fOffset, bCharacterMode);
                    basegfx::B3DHomMatrix aTransformFront;
                    aTransformFront.translate(0.0, 0.0, fDepth);
                    rSliceVector.emplace_back(aOuterFront, aTransformFront, SLICETYPE3D_FRONTCAP);
                }

                if(bCloseBack)
                {
                    const double fOffset(fDepth * fDiagonal * 0.5);
                    fZBack = fOffset;
                    impGetOuterPolyPolygon(aBack, aOuterBack, fOffset, bCharacterMode);
                }

                // add front and back polygons at evtl. changed depths
                {
                    basegfx::B3DHomMatrix aTransformA, aTransformB;

                    aTransformA.translate(0.0, 0.0, fZFront);
                    rSliceVector.emplace_back(aFront, aTransformA);

                    aTransformB.translate(0.0, 0.0, fZBack);
                    rSliceVector.emplace_back(aBack, aTransformB);
                }

                if(bCloseBack)
                {
                    rSliceVector.emplace_back(aOuterBack, basegfx::B3DHomMatrix(), SLICETYPE3D_BACKCAP);
                }
            }
        }

        basegfx::B3DPolyPolygon extractHorizontalLinesFromSlice(const Slice3DVector& rSliceVector, bool bCloseHorLines)
        {
            basegfx::B3DPolyPolygon aRetval;
            const sal_uInt32 nNumSlices(rSliceVector.size());

            if(nNumSlices)
            {
                const sal_uInt32 nSlideSubPolygonCount(rSliceVector[0].getB3DPolyPolygon().count());

                for(sal_uInt32 b(0); b < nSlideSubPolygonCount; b++)
                {
                    const sal_uInt32 nSubPolygonPointCount(rSliceVector[0].getB3DPolyPolygon().getB3DPolygon(b).count());

                    for(sal_uInt32 c(0); c < nSubPolygonPointCount; c++)
                    {
                        basegfx::B3DPolygon aNew;

                        for(sal_uInt32 d(0); d < nNumSlices; d++)
                        {
                            const bool bSamePolygonCount(nSlideSubPolygonCount == rSliceVector[d].getB3DPolyPolygon().count());
                            const bool bSamePointCount(nSubPolygonPointCount == rSliceVector[d].getB3DPolyPolygon().getB3DPolygon(b).count());

                            if(bSamePolygonCount && bSamePointCount)
                            {
                                aNew.append(rSliceVector[d].getB3DPolyPolygon().getB3DPolygon(b).getB3DPoint(c));
                            }
                            else
                            {
                                OSL_ENSURE(bSamePolygonCount, "Slice tools::PolyPolygon with different Polygon count (!)");
                                OSL_ENSURE(bSamePointCount, "Slice Polygon with different point count (!)");
                            }
                        }

                        aNew.setClosed(bCloseHorLines);
                        aRetval.append(aNew);
                    }
                }
            }

            return aRetval;
        }

        basegfx::B3DPolyPolygon  extractVerticalLinesFromSlice(const Slice3DVector& rSliceVector)
        {
            basegfx::B3DPolyPolygon aRetval;
            const sal_uInt32 nNumSlices(rSliceVector.size());

            for(sal_uInt32 a(0); a < nNumSlices; a++)
            {
                aRetval.append(rSliceVector[a].getB3DPolyPolygon());
            }

            return aRetval;
        }

        void extractPlanesFromSlice(
            std::vector< basegfx::B3DPolyPolygon >& rFill,
            const Slice3DVector& rSliceVector,
            bool bCreateNormals,
            bool bSmoothHorizontalNormals,
            bool bSmoothNormals,
            bool bSmoothLids,
            bool bClosed,
            double fSmoothNormalsMix,
            double fSmoothLidsMix,
            bool bCreateTextureCoordinates,
            const basegfx::B2DHomMatrix& rTexTransform)
        {
            const sal_uInt32 nNumSlices(rSliceVector.size());

            if(nNumSlices)
            {
                // common parameters
                const sal_uInt32 nLoopCount(bClosed ? nNumSlices : nNumSlices - 1);
                basegfx::B3DPolyPolygon aEdgeRounding;
                sal_uInt32 a;

                // texture parameters
                double fInvTexHeight(1.0);
                std::vector<double> aTexHeightArray;
                basegfx::B3DRange aTexRangeFront;
                basegfx::B3DRange aTexRangeBack;

                if(bCreateTextureCoordinates)
                {
                    aTexRangeFront = basegfx::utils::getRange(rSliceVector[0].getB3DPolyPolygon());
                    aTexRangeBack = basegfx::utils::getRange(rSliceVector[nNumSlices - 1].getB3DPolyPolygon());

                    if(aTexRangeBack.getDepth() > aTexRangeBack.getWidth())
                    {
                        // last polygon is rotated so that depth is bigger than width, exchange X and Z
                        // for making applyDefaultTextureCoordinatesParallel use Z instead of X for
                        // horizontal texture coordinate
                        aTexRangeBack = basegfx::B3DRange(
                            aTexRangeBack.getMinZ(), aTexRangeBack.getMinY(), aTexRangeBack.getMinX(),
                            aTexRangeBack.getMaxZ(), aTexRangeBack.getMaxY(), aTexRangeBack.getMaxX());
                    }

                    basegfx::B3DPoint aCenter(basegfx::utils::getRange(rSliceVector[0].getB3DPolyPolygon()).getCenter());

                    for(a = 0; a < nLoopCount; a++)
                    {
                        const basegfx::B3DPoint aNextCenter(basegfx::utils::getRange(rSliceVector[(a + 1) % nNumSlices].getB3DPolyPolygon()).getCenter());
                        const double fLength(basegfx::B3DVector(aNextCenter - aCenter).getLength());
                        aTexHeightArray.push_back(fLength);
                        aCenter = aNextCenter;
                    }

                    const double fTexHeight(std::accumulate(aTexHeightArray.begin(), aTexHeightArray.end(), 0.0));

                    if(!basegfx::fTools::equalZero(fTexHeight))
                    {
                        fInvTexHeight = 1.0 / fTexHeight;
                    }
                }

                if(nLoopCount)
                {
                    double fTexHeightPos(0.0);
                    for(a = 0; a < nLoopCount; a++)
                    {
                        const Slice3D& rSliceA(rSliceVector[a]);
                        const Slice3D& rSliceB(rSliceVector[(a + 1) % nNumSlices]);
                        const bool bAcceptPair(SLICETYPE3D_REGULAR == rSliceA.getSliceType() && SLICETYPE3D_REGULAR == rSliceB.getSliceType());
                        basegfx::B3DPolyPolygon aPolA(rSliceA.getB3DPolyPolygon());
                        basegfx::B3DPolyPolygon aPolB(rSliceB.getB3DPolyPolygon());

                        if(bAcceptPair)
                        {
                            if(bCreateNormals)
                            {
                                impCreateInBetweenNormals(aPolB, aPolA, bSmoothHorizontalNormals);
                            }

                            {
                                const sal_uInt32 nIndPrev((a + nNumSlices - 1) % nNumSlices);
                                const Slice3D& rSlicePrev(rSliceVector[nIndPrev]);
                                basegfx::B3DPolyPolygon aPrev(rSlicePrev.getB3DPolyPolygon());
                                basegfx::B3DPolyPolygon aPolAA(rSliceA.getB3DPolyPolygon());

                                if(SLICETYPE3D_FRONTCAP == rSlicePrev.getSliceType())
                                {
                                    basegfx::B3DPolyPolygon aFront(rSlicePrev.getB3DPolyPolygon());
                                    const bool bHasSlant(aPolAA != aPrev);

                                    if(bCreateTextureCoordinates)
                                    {
                                        aFront = basegfx::utils::applyDefaultTextureCoordinatesParallel(aFront, aTexRangeFront);
                                    }

                                    if(bCreateNormals)
                                    {
                                        basegfx::B3DVector aNormal(0.0, 0.0, -1.0);

                                        if(aFront.count())
                                        {
                                            aNormal = -aFront.getB3DPolygon(0).getNormal();
                                        }

                                        impSetNormal(aFront, aNormal);

                                        if(bHasSlant)
                                        {
                                            impCreateInBetweenNormals(aPolAA, aPrev, bSmoothHorizontalNormals);

                                            if(bSmoothNormals)
                                            {
                                                // smooth and copy
                                                impMixNormals(aPolA, aPolAA, fSmoothNormalsMix);
                                                aPolAA = aPolA;
                                            }
                                            else
                                            {
                                                // take over from surface
                                                aPolAA = aPolA;
                                            }

                                            if(bSmoothLids)
                                            {
                                                // smooth and copy
                                                impMixNormals(aFront, aPrev, fSmoothLidsMix);
                                                aPrev = aFront;
                                            }
                                            else
                                            {
                                                // take over from front
                                                aPrev = aFront;
                                            }
                                        }
                                        else
                                        {
                                            if(bSmoothNormals)
                                            {
                                                // smooth
                                                impMixNormals(aPolA, aFront, fSmoothNormalsMix);
                                            }

                                            if(bSmoothLids)
                                            {
                                                // smooth and copy
                                                impMixNormals(aFront, aPolA, fSmoothLidsMix);
                                                aPolA = aFront;
                                            }
                                        }
                                    }

                                    if(bHasSlant)
                                    {
                                        double fTexStart{};
                                        double fTexStop{};
                                        if(bCreateTextureCoordinates)
                                        {
                                            fTexStart = fTexHeightPos * fInvTexHeight;
                                            fTexStop = (fTexHeightPos - aTexHeightArray[(a + nLoopCount - 1) % nLoopCount]) * fInvTexHeight;
                                        }

                                        impAddInBetweenFill(aEdgeRounding, aPolAA, aPrev, fTexStart, fTexStop, bCreateNormals, bCreateTextureCoordinates);
                                    }

                                    aFront.flip();
                                    rFill.push_back(aFront);
                                }
                                else
                                {
                                    if(bCreateNormals && bSmoothNormals && (nIndPrev != a + 1))
                                    {
                                        impCreateInBetweenNormals(aPolAA, aPrev, bSmoothHorizontalNormals);
                                        impMixNormals(aPolA, aPolAA, 0.5);
                                    }
                                }
                            }

                            {
                                const sal_uInt32 nIndNext((a + 2) % nNumSlices);
                                const Slice3D& rSliceNext(rSliceVector[nIndNext]);
                                basegfx::B3DPolyPolygon aNext(rSliceNext.getB3DPolyPolygon());
                                basegfx::B3DPolyPolygon aPolBB(rSliceB.getB3DPolyPolygon());

                                if(SLICETYPE3D_BACKCAP == rSliceNext.getSliceType())
                                {
                                    basegfx::B3DPolyPolygon aBack(rSliceNext.getB3DPolyPolygon());
                                    const bool bHasSlant(aPolBB != aNext);

                                    if(bCreateTextureCoordinates)
                                    {
                                        aBack = basegfx::utils::applyDefaultTextureCoordinatesParallel(aBack, aTexRangeBack);
                                    }

                                    if(bCreateNormals)
                                    {
                                        const basegfx::B3DVector aNormal(aBack.count() ? aBack.getB3DPolygon(0).getNormal() : basegfx::B3DVector(0.0, 0.0, 1.0));
                                        impSetNormal(aBack, aNormal);

                                        if(bHasSlant)
                                        {
                                            impCreateInBetweenNormals(aNext, aPolBB, bSmoothHorizontalNormals);

                                            if(bSmoothNormals)
                                            {
                                                // smooth and copy
                                                impMixNormals(aPolB, aPolBB, fSmoothNormalsMix);
                                                aPolBB = aPolB;
                                            }
                                            else
                                            {
                                                // take over from surface
                                                aPolBB = aPolB;
                                            }

                                            if(bSmoothLids)
                                            {
                                                // smooth and copy
                                                impMixNormals(aBack, aNext, fSmoothLidsMix);
                                                aNext = aBack;
                                            }
                                            else
                                            {
                                                // take over from back
                                                aNext = aBack;
                                            }
                                        }
                                        else
                                        {
                                            if(bSmoothNormals)
                                            {
                                                // smooth
                                                impMixNormals(aPolB, aBack, fSmoothNormalsMix);
                                            }

                                            if(bSmoothLids)
                                            {
                                                // smooth and copy
                                                impMixNormals(aBack, aPolB, fSmoothLidsMix);
                                                aPolB = aBack;
                                            }
                                        }
                                    }

                                    if(bHasSlant)
                                    {
                                        double fTexStart{};
                                        double fTexStop{};
                                        if(bCreateTextureCoordinates)
                                        {
                                            fTexStart = (fTexHeightPos + aTexHeightArray[a] + aTexHeightArray[(a + 1) % nLoopCount]) * fInvTexHeight;
                                            fTexStop = (fTexHeightPos + aTexHeightArray[a]) * fInvTexHeight;
                                        }

                                        impAddInBetweenFill(aEdgeRounding, aNext, aPolBB, fTexStart, fTexStop, bCreateNormals, bCreateTextureCoordinates);
                                    }

                                    rFill.push_back(aBack);
                                }
                                else
                                {
                                    if(bCreateNormals && bSmoothNormals && (nIndNext != a))
                                    {
                                        impCreateInBetweenNormals(aNext, aPolBB, bSmoothHorizontalNormals);
                                        impMixNormals(aPolB, aPolBB, 0.5);
                                    }
                                }
                            }

                            double fTexStart{};
                            double fTexStop{};
                            if(bCreateTextureCoordinates)
                            {
                                fTexStart = (fTexHeightPos + aTexHeightArray[a]) * fInvTexHeight;
                                fTexStop = fTexHeightPos * fInvTexHeight;
                            }

                            impAddInBetweenFill(aEdgeRounding, aPolB, aPolA, fTexStart, fTexStop, bCreateNormals, bCreateTextureCoordinates);
                        }

                        if(bCreateTextureCoordinates)
                        {
                            fTexHeightPos += aTexHeightArray[a];
                        }
                    }
                }
                else
                {
                    // no loop, but a single slice (1 == nNumSlices), create a filling from the single
                    // front plane
                    const Slice3D& rSlice(rSliceVector[0]);
                    basegfx::B3DPolyPolygon aFront(rSlice.getB3DPolyPolygon());

                    if(bCreateTextureCoordinates)
                    {
                        aFront = basegfx::utils::applyDefaultTextureCoordinatesParallel(aFront, aTexRangeFront);
                    }

                    if(bCreateNormals)
                    {
                        basegfx::B3DVector aNormal(0.0, 0.0, -1.0);

                        if(aFront.count())
                        {
                            aNormal = -aFront.getB3DPolygon(0).getNormal();
                        }

                        impSetNormal(aFront, aNormal);
                    }

                    aFront.flip();
                    rFill.push_back(aFront);
                }

                if(bCreateTextureCoordinates)
                {
                    aEdgeRounding.transformTextureCoordinates(rTexTransform);
                }

                for(a = 0; a < aEdgeRounding.count(); a++)
                {
                    rFill.emplace_back(aEdgeRounding.getB3DPolygon(a));
                }
            }
        }

        void createReducedOutlines(
            const geometry::ViewInformation3D& rViewInformation,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B3DPolygon& rLoopA,
            const basegfx::B3DPolygon& rLoopB,
            basegfx::B3DPolyPolygon& rTarget)
        {
            const sal_uInt32 nPointCount(rLoopA.count());

            // with identical polygons there are no outlines
            if(rLoopA != rLoopB)
            {
                if(nPointCount && nPointCount == rLoopB.count())
                {
                    const basegfx::B3DHomMatrix aObjectTransform(rViewInformation.getObjectToView() * rObjectTransform);
                    const basegfx::B2DPolygon a2DLoopA(basegfx::utils::createB2DPolygonFromB3DPolygon(rLoopA, aObjectTransform));
                    const basegfx::B2DPolygon a2DLoopB(basegfx::utils::createB2DPolygonFromB3DPolygon(rLoopB, aObjectTransform));
                    const basegfx::B2DPoint a2DCenterA(a2DLoopA.getB2DRange().getCenter());
                    const basegfx::B2DPoint a2DCenterB(a2DLoopB.getB2DRange().getCenter());

                    // without detectable Y-Axis there are no outlines
                    if(!a2DCenterA.equal(a2DCenterB))
                    {
                        // search for outmost left and right inter-loop-edges which do not cut the loops
                        const basegfx::B2DPoint aCommonCenter(basegfx::average(a2DCenterA, a2DCenterB));
                        const basegfx::B2DVector aAxisVector(a2DCenterA - a2DCenterB);
                        double fMaxLeft(0.0);
                        double fMaxRight(0.0);
                        sal_uInt32 nIndexLeft(0);
                        sal_uInt32 nIndexRight(0);

                        for(sal_uInt32 a(0); a < nPointCount; a++)
                        {
                            const basegfx::B2DPoint aStart(a2DLoopA.getB2DPoint(a));
                            const basegfx::B2DPoint aEnd(a2DLoopB.getB2DPoint(a));
                            const basegfx::B2DPoint aMiddle(basegfx::average(aStart, aEnd));

                            if(!basegfx::utils::isInside(a2DLoopA, aMiddle))
                            {
                                if(!basegfx::utils::isInside(a2DLoopB, aMiddle))
                                {
                                    if(!impHasCutWith(a2DLoopA, aStart, aEnd))
                                    {
                                        if(!impHasCutWith(a2DLoopB, aStart, aEnd))
                                        {
                                            const basegfx::B2DVector aCandidateVector(aMiddle - aCommonCenter);
                                            const double fCross(aCandidateVector.cross(aAxisVector));
                                            const double fDistance(aCandidateVector.getLength());

                                            if(fCross > 0.0)
                                            {
                                                if(fDistance > fMaxLeft)
                                                {
                                                    fMaxLeft = fDistance;
                                                    nIndexLeft = a;
                                                }
                                            }
                                            else if(fCross < 0.0)
                                            {
                                                if(fDistance > fMaxRight)
                                                {
                                                    fMaxRight = fDistance;
                                                    nIndexRight = a;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if(fMaxLeft != 0.0)
                        {
                            basegfx::B3DPolygon aToBeAdded;
                            aToBeAdded.append(rLoopA.getB3DPoint(nIndexLeft));
                            aToBeAdded.append(rLoopB.getB3DPoint(nIndexLeft));
                            rTarget.append(aToBeAdded);
                        }

                        if(fMaxRight != 0.0)
                        {
                            basegfx::B3DPolygon aToBeAdded;
                            aToBeAdded.append(rLoopA.getB3DPoint(nIndexRight));
                            aToBeAdded.append(rLoopB.getB3DPoint(nIndexRight));
                            rTarget.append(aToBeAdded);
                        }
                    }
                }
            }
        }

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
