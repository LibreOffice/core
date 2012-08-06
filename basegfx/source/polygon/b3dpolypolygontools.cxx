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

#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <basegfx/range/b3drange.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <numeric>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <osl/mutex.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // B3DPolyPolygon tools
        B3DRange getRange(const B3DPolyPolygon& rCandidate)
        {
            B3DRange aRetval;
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B3DPolygon aCandidate = rCandidate.getB3DPolygon(a);
                aRetval.expand(getRange(aCandidate));
            }

            return aRetval;
        }

        B3DPolyPolygon createUnitCubePolyPolygon()
        {
            static B3DPolyPolygon aRetval;
            ::osl::Mutex m_mutex;

            if(!aRetval.count())
            {
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
            }

            return aRetval;
        }

        B3DPolyPolygon createUnitCubeFillPolyPolygon()
        {
            static B3DPolyPolygon aRetval;
            ::osl::Mutex m_mutex;

            if(!aRetval.count())
            {
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
            }

            return aRetval;
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

        // helper for getting the 3D Point from given cartesian coordiantes. fVer is defined from
        // [F_PI2 .. -F_PI2], fHor from [0.0 .. F_2PI]
        inline B3DPoint getPointFromCartesian(double fVer, double fHor)
        {
            const double fCosHor(cos(fHor));
            return B3DPoint(fCosHor * cos(fVer), sin(fHor), fCosHor * -sin(fVer));
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

            if(!nHorSeg)
            {
                nHorSeg = 1L;
            }

            if(!nVerSeg)
            {
                nVerSeg = fround(fabs(fVerStop - fVerStart) / (F_2PI / 24.0));
            }

            if(!nVerSeg)
            {
                nVerSeg = 1L;
            }

            // create constants
            const double fVerDiffPerStep((fVerStop - fVerStart) / (double)nVerSeg);
            const double fHorDiffPerStep((fHorStop - fHorStart) / (double)nHorSeg);
            bool bHorClosed(fTools::equal(fHorStop - fHorStart, F_2PI));
            bool bVerFromTop(fTools::equal(fVerStart, F_PI2));
            bool bVerToBottom(fTools::equal(fVerStop, -F_PI2));

            // create horizontal rings
            const sal_uInt32 nLoopVerInit(bVerFromTop ? 1L : 0L);
            const sal_uInt32 nLoopVerLimit(bVerToBottom ? nVerSeg : nVerSeg + 1L);
            const sal_uInt32 nLoopHorLimit(bHorClosed ? nHorSeg : nHorSeg + 1L);

            for(a = nLoopVerInit; a < nLoopVerLimit; a++)
            {
                const double fVer(fVerStart + ((double)(a) * fVerDiffPerStep));
                B3DPolygon aNew;

                for(b = 0L; b < nLoopHorLimit; b++)
                {
                    const double fHor(fHorStart + ((double)(b) * fHorDiffPerStep));
                    aNew.append(getPointFromCartesian(fHor, fVer));
                }

                aNew.setClosed(bHorClosed);
                aRetval.append(aNew);
            }

            // create vertical half-rings
            for(a = 0L; a < nLoopHorLimit; a++)
            {
                const double fHor(fHorStart + ((double)(a) * fHorDiffPerStep));
                B3DPolygon aNew;

                if(bVerFromTop)
                {
                    aNew.append(B3DPoint(0.0, 1.0, 0.0));
                }

                for(b = nLoopVerInit; b < nLoopVerLimit; b++)
                {
                    const double fVer(fVerStart + ((double)(b) * fVerDiffPerStep));
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

            if(!nHorSeg)
            {
                nHorSeg = 1L;
            }

            if(!nVerSeg)
            {
                nVerSeg = fround(fabs(fVerStop - fVerStart) / (F_2PI / 24.0));
            }

            if(!nVerSeg)
            {
                nVerSeg = 1L;
            }

            // vertical loop
            for(sal_uInt32 a(0L); a < nVerSeg; a++)
            {
                const double fVer(fVerStart + (((fVerStop - fVerStart) * a) / nVerSeg));
                const double fVer2(fVerStart + (((fVerStop - fVerStart) * (a + 1)) / nVerSeg));

                // horizontal loop
                for(sal_uInt32 b(0L); b < nHorSeg; b++)
                {
                    const double fHor(fHorStart + (((fHorStop - fHorStart) * b) / nHorSeg));
                    const double fHor2(fHorStart + (((fHorStop - fHorStart) * (b + 1)) / nHorSeg));
                    B3DPolygon aNew;

                    aNew.append(getPointFromCartesian(fHor, fVer));
                    aNew.append(getPointFromCartesian(fHor2, fVer));
                    aNew.append(getPointFromCartesian(fHor2, fVer2));
                    aNew.append(getPointFromCartesian(fHor, fVer2));

                    if(bNormals)
                    {
                        for(sal_uInt32 c(0L); c < aNew.count(); c++)
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

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(applyDefaultNormalsSphere(rCandidate.getB3DPolygon(a), rCenter));
            }

            return aRetval;
        }

        B3DPolyPolygon invertNormals( const B3DPolyPolygon& rCandidate)
        {
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(invertNormals(rCandidate.getB3DPolygon(a)));
            }

            return aRetval;
        }

        B3DPolyPolygon applyDefaultTextureCoordinatesParallel( const B3DPolyPolygon& rCandidate, const B3DRange& rRange, bool bChangeX, bool bChangeY)
        {
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(applyDefaultTextureCoordinatesParallel(rCandidate.getB3DPolygon(a), rRange, bChangeX, bChangeY));
            }

            return aRetval;
        }

        B3DPolyPolygon applyDefaultTextureCoordinatesSphere( const B3DPolyPolygon& rCandidate, const B3DPoint& rCenter, bool bChangeX, bool bChangeY)
        {
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(applyDefaultTextureCoordinatesSphere(rCandidate.getB3DPolygon(a), rCenter, bChangeX, bChangeY));
            }

            return aRetval;
        }

        bool isInside(const B3DPolyPolygon& rCandidate, const B3DPoint& rPoint, bool bWithBorder)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());

            if(1L == nPolygonCount)
            {
                return isInside(rCandidate.getB3DPolygon(0), rPoint, bWithBorder);
            }
            else
            {
                sal_Int32 nInsideCount(0);

                for(sal_uInt32 a(0); a < nPolygonCount; a++)
                {
                    const B3DPolygon aPolygon(rCandidate.getB3DPolygon(a));
                    const bool bInside(isInside(aPolygon, rPoint, bWithBorder));

                    if(bInside)
                    {
                        nInsideCount++;
                    }
                }

                return (nInsideCount % 2L);
            }
        }

    } // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
