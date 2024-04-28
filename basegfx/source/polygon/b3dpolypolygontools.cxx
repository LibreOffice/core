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

namespace basegfx::utils
{
        // B3DPolyPolygon tools
        B3DRange getRange(const B3DPolyPolygon& rCandidate)
        {
            B3DRange aRetval;

            for(const auto& rPolygon : rCandidate )
            {
                aRetval.expand(getRange(rPolygon));
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
        // [M_PI_2 .. -M_PI_2], fVer from [0.0 .. 2PI]
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
                nHorSeg = fround(fabs(fHorStop - fHorStart) / (M_PI / 12.0));
            }

            // min/max limitations
            nHorSeg = std::clamp(nHorSeg, nMinSegments, nMaxSegments);

            if(!nVerSeg)
            {
                nVerSeg = fround(fabs(fVerStop - fVerStart) / (M_PI / 12.0));
            }

            // min/max limitations
            nVerSeg = std::clamp(nVerSeg, nMinSegments, nMaxSegments);

            // create constants
            const double fVerDiffPerStep((fVerStop - fVerStart) / static_cast<double>(nVerSeg));
            const double fHorDiffPerStep((fHorStop - fHorStart) / static_cast<double>(nHorSeg));
            bool bHorClosed(fTools::equal(fHorStop - fHorStart, 2 * M_PI));
            bool bVerFromTop(fTools::equal(fVerStart, M_PI_2));
            bool bVerToBottom(fTools::equal(fVerStop, -M_PI_2));

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
                nHorSeg = fround(fabs(fHorStop - fHorStart) / (M_PI / 12.0));
            }

            // min/max limitations
            nHorSeg = std::clamp(nHorSeg, nMinSegments, nMaxSegments);

            if(!nVerSeg)
            {
                nVerSeg = fround(fabs(fVerStop - fVerStart) / (M_PI / 12.0));
            }

            // min/max limitations
            nVerSeg = std::clamp(nVerSeg, nMinSegments, nMaxSegments);

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

            for( const auto& rB3DPolygon : rCandidate)
            {
                aRetval.append(applyDefaultNormalsSphere(rB3DPolygon, rCenter));
            }

            return aRetval;
        }

        B3DPolyPolygon invertNormals( const B3DPolyPolygon& rCandidate)
        {
            B3DPolyPolygon aRetval;

            for( const auto& rB3DPolygon : rCandidate )
            {
                aRetval.append(invertNormals(rB3DPolygon));
            }

            return aRetval;
        }

        B3DPolyPolygon applyDefaultTextureCoordinatesParallel( const B3DPolyPolygon& rCandidate, const B3DRange& rRange, bool bChangeX, bool bChangeY)
        {
            B3DPolyPolygon aRetval;

            for( const auto& rB3DPolygon : rCandidate)
            {
                aRetval.append(applyDefaultTextureCoordinatesParallel(rB3DPolygon, rRange, bChangeX, bChangeY));
            }

            return aRetval;
        }

        B3DPolyPolygon applyDefaultTextureCoordinatesSphere( const B3DPolyPolygon& rCandidate, const B3DPoint& rCenter, bool bChangeX, bool bChangeY)
        {
            B3DPolyPolygon aRetval;

            for( const auto& rB3DPolygon : rCandidate )
            {
                aRetval.append(applyDefaultTextureCoordinatesSphere(rB3DPolygon, rCenter, bChangeX, bChangeY));
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

                for(const auto& rPolygon : rCandidate )
                {
                    const bool bInside(isInside(rPolygon, rPoint, false/*bWithBorder*/));

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
                assert(nOuterSequenceCount == rPolyPolygonShape3DSource.SequenceY.getLength()
                           && nOuterSequenceCount
                                  == rPolyPolygonShape3DSource.SequenceZ.getLength()&&
                       "UnoPolyPolygonShape3DToB3DPolygon: Not all double sequences have the same "
                       "length (!)" );

                for(sal_Int32 a(0); a < nOuterSequenceCount; a++)
                {
                    basegfx::B3DPolygon aNewPolygon;

                    auto& rInnerSequenceX = rPolyPolygonShape3DSource.SequenceX[a];
                    auto& rInnerSequenceY = rPolyPolygonShape3DSource.SequenceY[a];
                    auto& rInnerSequenceZ = rPolyPolygonShape3DSource.SequenceZ[a];

                    const sal_Int32 nInnerSequenceCount(rInnerSequenceX.getLength());
                    assert(nInnerSequenceCount == rInnerSequenceY.getLength()
                           && nInnerSequenceCount == rInnerSequenceZ.getLength()
                           && "UnoPolyPolygonShape3DToB3DPolygon: Not all double sequences have "
                              "the same length (!)");

                    for(sal_Int32 b(0); b < nInnerSequenceCount; b++)
                    {
                        aNewPolygon.append(basegfx::B3DPoint(rInnerSequenceX[b], rInnerSequenceY[b], rInnerSequenceZ[b]));
                    }

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

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
