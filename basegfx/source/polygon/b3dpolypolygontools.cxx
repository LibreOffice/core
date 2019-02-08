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

#include <osl/diagnose.h>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <basegfx/range/b3drange.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>

// predefines
#define nMinSegments sal_uInt32(1)
#define nMaxSegments sal_uInt32(512)

namespace basegfx
{
    namespace utils
    {
        // B3DPolyPolygon tools
        B3DRange getRange(const B3DPolyPolygon& rCandidate)
        {
            B3DRange aRetval;
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0); a < nPolygonCount; a++)
            {
                const B3DPolygon& aCandidate = rCandidate.getB3DPolygon(a);
                aRetval.expand(getRange(aCandidate));
            }

            return aRetval;
        }

        B3DPolyPolygon const & createUnitCubePolyPolygon()
        {
            static auto const singleton = [] {
                    B3DPolyPolygon aRetval;
                    B3DPolygon aTemp;
                    aTemp.append(B3DPoint(0.0, 0.0, 1.0));
                    aTemp.append(B3DPoint(0.0, 1.0, 1.0));
                    aTemp.append(B3DPoint(1.0, 1.0, 1.0));
                    aTemp.append(B3DPoint(1.0, 0.0, 1.0));
                    aTemp.setClosed(true);
                    aRetval.append(aTemp);

                    aTemp.clear();
                    aTemp.append(B3DPoint(0.0, 0.0, 0.0));
                    aTemp.append(B3DPoint(0.0, 1.0, 0.0));
                    aTemp.append(B3DPoint(1.0, 1.0, 0.0));
                    aTemp.append(B3DPoint(1.0, 0.0, 0.0));
                    aTemp.setClosed(true);
                    aRetval.append(aTemp);

                    aTemp.clear();
                    aTemp.append(B3DPoint(0.0, 0.0, 0.0));
                    aTemp.append(B3DPoint(0.0, 0.0, 1.0));
                    aRetval.append(aTemp);

                    aTemp.clear();
                    aTemp.append(B3DPoint(0.0, 1.0, 0.0));
                    aTemp.append(B3DPoint(0.0, 1.0, 1.0));
                    aRetval.append(aTemp);

                    aTemp.clear();
                    aTemp.append(B3DPoint(1.0, 1.0, 0.0));
                    aTemp.append(B3DPoint(1.0, 1.0, 1.0));
                    aRetval.append(aTemp);

                    aTemp.clear();
                    aTemp.append(B3DPoint(1.0, 0.0, 0.0));
                    aTemp.append(B3DPoint(1.0, 0.0, 1.0));
                    aRetval.append(aTemp);
                    return aRetval;
                }();
            return singleton;
        }

        B3DPolyPolygon const & createUnitCubeFillPolyPolygon()
        {
            static auto const singleton = [] {
                    B3DPolyPolygon aRetval;
                    B3DPolygon aTemp;

                    // all points
                    const B3DPoint A(0.0, 0.0, 0.0);
                    const B3DPoint B(0.0, 1.0, 0.0);
                    const B3DPoint C(1.0, 1.0, 0.0);
                    const B3DPoint D(1.0, 0.0, 0.0);
                    const B3DPoint E(0.0, 0.0, 1.0);
                    const B3DPoint F(0.0, 1.0, 1.0);
                    const B3DPoint G(1.0, 1.0, 1.0);
                    const B3DPoint H(1.0, 0.0, 1.0);

                    // create bottom
                    aTemp.append(D);
                    aTemp.append(A);
                    aTemp.append(E);
                    aTemp.append(H);
                    aTemp.setClosed(true);
                    aRetval.append(aTemp);

                    // create front
                    aTemp.clear();
                    aTemp.append(B);
                    aTemp.append(A);
                    aTemp.append(D);
                    aTemp.append(C);
                    aTemp.setClosed(true);
                    aRetval.append(aTemp);

                    // create left
                    aTemp.clear();
                    aTemp.append(E);
                    aTemp.append(A);
                    aTemp.append(B);
                    aTemp.append(F);
                    aTemp.setClosed(true);
                    aRetval.append(aTemp);

                    // create top
                    aTemp.clear();
                    aTemp.append(C);
                    aTemp.append(G);
                    aTemp.append(F);
                    aTemp.append(B);
                    aTemp.setClosed(true);
                    aRetval.append(aTemp);

                    // create right
                    aTemp.clear();
                    aTemp.append(H);
                    aTemp.append(G);
                    aTemp.append(C);
                    aTemp.append(D);
                    aTemp.setClosed(true);
                    aRetval.append(aTemp);

                    // create back
                    aTemp.clear();
                    aTemp.append(F);
                    aTemp.append(G);
                    aTemp.append(H);
                    aTemp.append(E);
                    aTemp.setClosed(true);
                    aRetval.append(aTemp);
                    return aRetval;
                }();
            return singleton;
        }

        B3DPolyPolygon createCubePolyPolygonFromB3DRange( const B3DRange& rRange)
        {
            B3DPolyPolygon aRetval;

            if(!rRange.isEmpty())
            {
                aRetval = createUnitCubePolyPolygon();
                B3DHomMatrix aTrans;
                aTrans.scale(rRange.getWidth(), rRange.getHeight(), rRange.getDepth());
                aTrans.translate(rRange.getMinX(), rRange.getMinY(), rRange.getMinZ());
                aRetval.transform(aTrans);
                aRetval.removeDoublePoints();
            }

            return aRetval;
        }

        B3DPolyPolygon createCubeFillPolyPolygonFromB3DRange( const B3DRange& rRange)
        {
            B3DPolyPolygon aRetval;

            if(!rRange.isEmpty())
            {
                aRetval = createUnitCubeFillPolyPolygon();
                B3DHomMatrix aTrans;
                aTrans.scale(rRange.getWidth(), rRange.getHeight(), rRange.getDepth());
                aTrans.translate(rRange.getMinX(), rRange.getMinY(), rRange.getMinZ());
                aRetval.transform(aTrans);
                aRetval.removeDoublePoints();
            }

            return aRetval;
        }

        // helper for getting the 3D Point from given cartesian coordinates. fHor is defined from
        // [F_PI2 .. -F_PI2], fVer from [0.0 .. F_2PI]
        static B3DPoint getPointFromCartesian(double fHor, double fVer)
        {
            const double fCosVer(cos(fVer));
            return B3DPoint(fCosVer * cos(fHor), sin(fVer), fCosVer * -sin(fHor));
        }

        B3DPolyPolygon createUnitSpherePolyPolygon(
            sal_uInt32 nHorSeg, sal_uInt32 nVerSeg,
            double fVerStart, double fVerStop,
            double fHorStart, double fHorStop)
        {
            B3DPolyPolygon aRetval;
            sal_uInt32 a, b;

            if(!nHorSeg)
            {
                nHorSeg = fround(fabs(fHorStop - fHorStart) / (F_2PI / 24.0));
            }

            // min/max limitations
            nHorSeg = std::min(nMaxSegments, std::max(nMinSegments, nHorSeg));

            if(!nVerSeg)
            {
                nVerSeg = fround(fabs(fVerStop - fVerStart) / (F_2PI / 24.0));
            }

            // min/max limitations
            nVerSeg = std::min(nMaxSegments, std::max(nMinSegments, nVerSeg));

            // create constants
            const double fVerDiffPerStep((fVerStop - fVerStart) / static_cast<double>(nVerSeg));
            const double fHorDiffPerStep((fHorStop - fHorStart) / static_cast<double>(nHorSeg));
            bool bHorClosed(fTools::equal(fHorStop - fHorStart, F_2PI));
            bool bVerFromTop(fTools::equal(fVerStart, F_PI2));
            bool bVerToBottom(fTools::equal(fVerStop, -F_PI2));

            // create horizontal rings
            const sal_uInt32 nLoopVerInit(bVerFromTop ? 1 : 0);
            const sal_uInt32 nLoopVerLimit(bVerToBottom ? nVerSeg : nVerSeg + 1);
            const sal_uInt32 nLoopHorLimit(bHorClosed ? nHorSeg : nHorSeg + 1);

            for(a = nLoopVerInit; a < nLoopVerLimit; a++)
            {
                const double fVer(fVerStart + (static_cast<double>(a) * fVerDiffPerStep));
                B3DPolygon aNew;

                for(b = 0; b < nLoopHorLimit; b++)
                {
                    const double fHor(fHorStart + (static_cast<double>(b) * fHorDiffPerStep));
                    aNew.append(getPointFromCartesian(fHor, fVer));
                }

                aNew.setClosed(bHorClosed);
                aRetval.append(aNew);
            }

            // create vertical half-rings
            for(a = 0; a < nLoopHorLimit; a++)
            {
                const double fHor(fHorStart + (static_cast<double>(a) * fHorDiffPerStep));
                B3DPolygon aNew;

                if(bVerFromTop)
                {
                    aNew.append(B3DPoint(0.0, 1.0, 0.0));
                }

                for(b = nLoopVerInit; b < nLoopVerLimit; b++)
                {
                    const double fVer(fVerStart + (static_cast<double>(b) * fVerDiffPerStep));
                    aNew.append(getPointFromCartesian(fHor, fVer));
                }

                if(bVerToBottom)
                {
                    aNew.append(B3DPoint(0.0, -1.0, 0.0));
                }

                aRetval.append(aNew);
            }

            return aRetval;
        }

        B3DPolyPolygon createSpherePolyPolygonFromB3DRange( const B3DRange& rRange,
            sal_uInt32 nHorSeg, sal_uInt32 nVerSeg,
            double fVerStart, double fVerStop,
            double fHorStart, double fHorStop)
        {
            B3DPolyPolygon aRetval(createUnitSpherePolyPolygon(nHorSeg, nVerSeg, fVerStart, fVerStop, fHorStart, fHorStop));

            if(aRetval.count())
            {
                // move and scale whole construct which is now in [-1.0 .. 1.0] in all directions
                B3DHomMatrix aTrans;
                aTrans.translate(1.0, 1.0, 1.0);
                aTrans.scale(rRange.getWidth() / 2.0, rRange.getHeight() / 2.0, rRange.getDepth() / 2.0);
                aTrans.translate(rRange.getMinX(), rRange.getMinY(), rRange.getMinZ());
                aRetval.transform(aTrans);
            }

            return aRetval;
        }

        B3DPolyPolygon createUnitSphereFillPolyPolygon(
            sal_uInt32 nHorSeg, sal_uInt32 nVerSeg,
            bool bNormals,
            double fVerStart, double fVerStop,
            double fHorStart, double fHorStop)
        {
            B3DPolyPolygon aRetval;

            if(!nHorSeg)
            {
                nHorSeg = fround(fabs(fHorStop - fHorStart) / (F_2PI / 24.0));
            }

            // min/max limitations
            nHorSeg = std::min(nMaxSegments, std::max(nMinSegments, nHorSeg));

            if(!nVerSeg)
            {
                nVerSeg = fround(fabs(fVerStop - fVerStart) / (F_2PI / 24.0));
            }

            // min/max limitations
            nVerSeg = std::min(nMaxSegments, std::max(nMinSegments, nVerSeg));

            // vertical loop
            for(sal_uInt32 a(0); a < nVerSeg; a++)
            {
                const double fVer1(fVerStart + (((fVerStop - fVerStart) * a) / nVerSeg));
                const double fVer2(fVerStart + (((fVerStop - fVerStart) * (a + 1)) / nVerSeg));

                // horizontal loop
                for(sal_uInt32 b(0); b < nHorSeg; b++)
                {
                    const double fHor1(fHorStart + (((fHorStop - fHorStart) * b) / nHorSeg));
                    const double fHor2(fHorStart + (((fHorStop - fHorStart) * (b + 1)) / nHorSeg));
                    B3DPolygon aNew;

                    aNew.append(getPointFromCartesian(fHor1, fVer1));
                    aNew.append(getPointFromCartesian(fHor2, fVer1));
                    aNew.append(getPointFromCartesian(fHor2, fVer2));
                    aNew.append(getPointFromCartesian(fHor1, fVer2));

                    if(bNormals)
                    {
                        for(sal_uInt32 c(0); c < aNew.count(); c++)
                        {
                            aNew.setNormal(c, ::basegfx::B3DVector(aNew.getB3DPoint(c)));
                        }
                    }

                    aNew.setClosed(true);
                    aRetval.append(aNew);
                }
            }

            return aRetval;
        }

        B3DPolyPolygon createSphereFillPolyPolygonFromB3DRange( const B3DRange& rRange,
            sal_uInt32 nHorSeg, sal_uInt32 nVerSeg,
            bool bNormals,
            double fVerStart, double fVerStop,
            double fHorStart, double fHorStop)
        {
            B3DPolyPolygon aRetval(createUnitSphereFillPolyPolygon(nHorSeg, nVerSeg, bNormals, fVerStart, fVerStop, fHorStart, fHorStop));

            if(aRetval.count())
            {
                // move and scale whole construct which is now in [-1.0 .. 1.0] in all directions
                B3DHomMatrix aTrans;
                aTrans.translate(1.0, 1.0, 1.0);
                aTrans.scale(rRange.getWidth() / 2.0, rRange.getHeight() / 2.0, rRange.getDepth() / 2.0);
                aTrans.translate(rRange.getMinX(), rRange.getMinY(), rRange.getMinZ());
                aRetval.transform(aTrans);
            }

            return aRetval;
        }

        B3DPolyPolygon applyDefaultNormalsSphere( const B3DPolyPolygon& rCandidate, const B3DPoint& rCenter)
        {
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0); a < rCandidate.count(); a++)
            {
                aRetval.append(applyDefaultNormalsSphere(rCandidate.getB3DPolygon(a), rCenter));
            }

            return aRetval;
        }

        B3DPolyPolygon invertNormals( const B3DPolyPolygon& rCandidate)
        {
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0); a < rCandidate.count(); a++)
            {
                aRetval.append(invertNormals(rCandidate.getB3DPolygon(a)));
            }

            return aRetval;
        }

        B3DPolyPolygon applyDefaultTextureCoordinatesParallel( const B3DPolyPolygon& rCandidate, const B3DRange& rRange, bool bChangeX, bool bChangeY)
        {
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0); a < rCandidate.count(); a++)
            {
                aRetval.append(applyDefaultTextureCoordinatesParallel(rCandidate.getB3DPolygon(a), rRange, bChangeX, bChangeY));
            }

            return aRetval;
        }

        B3DPolyPolygon applyDefaultTextureCoordinatesSphere( const B3DPolyPolygon& rCandidate, const B3DPoint& rCenter, bool bChangeX, bool bChangeY)
        {
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0); a < rCandidate.count(); a++)
            {
                aRetval.append(applyDefaultTextureCoordinatesSphere(rCandidate.getB3DPolygon(a), rCenter, bChangeX, bChangeY));
            }

            return aRetval;
        }

        bool isInside(const B3DPolyPolygon& rCandidate, const B3DPoint& rPoint)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());

            if(nPolygonCount == 1)
            {
                return isInside(rCandidate.getB3DPolygon(0), rPoint, false/*bWithBorder*/);
            }
            else
            {
                sal_Int32 nInsideCount(0);

                for(sal_uInt32 a(0); a < nPolygonCount; a++)
                {
                    const B3DPolygon& aPolygon(rCandidate.getB3DPolygon(a));
                    const bool bInside(isInside(aPolygon, rPoint, false/*bWithBorder*/));

                    if(bInside)
                    {
                        nInsideCount++;
                    }
                }

                return (nInsideCount % 2);
            }
        }

/// converters for css::drawing::PolyPolygonShape3D
        B3DPolyPolygon UnoPolyPolygonShape3DToB3DPolyPolygon(
            const css::drawing::PolyPolygonShape3D& rPolyPolygonShape3DSource)
        {
            B3DPolyPolygon aRetval;
            const sal_Int32 nOuterSequenceCount(rPolyPolygonShape3DSource.SequenceX.getLength());

            if(nOuterSequenceCount)
            {
                OSL_ENSURE(nOuterSequenceCount == rPolyPolygonShape3DSource.SequenceY.getLength()
                    && nOuterSequenceCount == rPolyPolygonShape3DSource.SequenceZ.getLength(),
                    "UnoPolyPolygonShape3DToB3DPolygon: Not all double sequences have the same length (!)");

                const css::drawing::DoubleSequence* pInnerSequenceX = rPolyPolygonShape3DSource.SequenceX.getConstArray();
                const css::drawing::DoubleSequence* pInnerSequenceY = rPolyPolygonShape3DSource.SequenceY.getConstArray();
                const css::drawing::DoubleSequence* pInnerSequenceZ = rPolyPolygonShape3DSource.SequenceZ.getConstArray();

                for(sal_Int32 a(0); a < nOuterSequenceCount; a++)
                {
                    basegfx::B3DPolygon aNewPolygon;
                    const sal_Int32 nInnerSequenceCount(pInnerSequenceX->getLength());
                    OSL_ENSURE(nInnerSequenceCount == pInnerSequenceY->getLength()
                        && nInnerSequenceCount == pInnerSequenceZ->getLength(),
                        "UnoPolyPolygonShape3DToB3DPolygon: Not all double sequences have the same length (!)");

                    const double* pArrayX = pInnerSequenceX->getConstArray();
                    const double* pArrayY = pInnerSequenceY->getConstArray();
                    const double* pArrayZ = pInnerSequenceZ->getConstArray();

                    for(sal_Int32 b(0); b < nInnerSequenceCount; b++)
                    {
                        aNewPolygon.append(basegfx::B3DPoint(*pArrayX++,*pArrayY++,*pArrayZ++));
                    }

                    pInnerSequenceX++;
                    pInnerSequenceY++;
                    pInnerSequenceZ++;

                    // #i101520# correction is needed for imported polygons of old format,
                    // see callers
                    basegfx::utils::checkClosed(aNewPolygon);

                    aRetval.append(aNewPolygon);
                }
            }

            return aRetval;
        }

        void B3DPolyPolygonToUnoPolyPolygonShape3D(
            const B3DPolyPolygon& rPolyPolygonSource,
            css::drawing::PolyPolygonShape3D& rPolyPolygonShape3DRetval)
        {
            const sal_uInt32 nPolygonCount(rPolyPolygonSource.count());

            if(nPolygonCount)
            {
                rPolyPolygonShape3DRetval.SequenceX.realloc(nPolygonCount);
                rPolyPolygonShape3DRetval.SequenceY.realloc(nPolygonCount);
                rPolyPolygonShape3DRetval.SequenceZ.realloc(nPolygonCount);

                css::drawing::DoubleSequence* pOuterSequenceX = rPolyPolygonShape3DRetval.SequenceX.getArray();
                css::drawing::DoubleSequence* pOuterSequenceY = rPolyPolygonShape3DRetval.SequenceY.getArray();
                css::drawing::DoubleSequence* pOuterSequenceZ = rPolyPolygonShape3DRetval.SequenceZ.getArray();

                for(sal_uInt32 a(0); a < nPolygonCount; a++)
                {
                    const basegfx::B3DPolygon& aPoly(rPolyPolygonSource.getB3DPolygon(a));
                    const sal_uInt32 nPointCount(aPoly.count());

                    if(nPointCount)
                    {
                        const bool bIsClosed(aPoly.isClosed());
                        const sal_uInt32 nTargetCount(bIsClosed ? nPointCount + 1 : nPointCount);
                        pOuterSequenceX->realloc(nTargetCount);
                        pOuterSequenceY->realloc(nTargetCount);
                        pOuterSequenceZ->realloc(nTargetCount);

                        double* pInnerSequenceX = pOuterSequenceX->getArray();
                        double* pInnerSequenceY = pOuterSequenceY->getArray();
                        double* pInnerSequenceZ = pOuterSequenceZ->getArray();

                        for(sal_uInt32 b(0); b < nPointCount; b++)
                        {
                            const basegfx::B3DPoint aPoint(aPoly.getB3DPoint(b));

                            *pInnerSequenceX++ = aPoint.getX();
                            *pInnerSequenceY++ = aPoint.getY();
                            *pInnerSequenceZ++ = aPoint.getZ();
                        }

                        if(bIsClosed)
                        {
                            const basegfx::B3DPoint aPoint(aPoly.getB3DPoint(0));

                            *pInnerSequenceX++ = aPoint.getX();
                            *pInnerSequenceY++ = aPoint.getY();
                            *pInnerSequenceZ++ = aPoint.getZ();
                        }
                    }
                    else
                    {
                        pOuterSequenceX->realloc(0);
                        pOuterSequenceY->realloc(0);
                        pOuterSequenceZ->realloc(0);
                    }

                    pOuterSequenceX++;
                    pOuterSequenceY++;
                    pOuterSequenceZ++;
                }
            }
            else
            {
                rPolyPolygonShape3DRetval.SequenceX.realloc(0);
                rPolyPolygonShape3DRetval.SequenceY.realloc(0);
                rPolyPolygonShape3DRetval.SequenceZ.realloc(0);
            }
        }

    } // end of namespace utils
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
