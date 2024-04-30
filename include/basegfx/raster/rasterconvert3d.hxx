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

#pragma once

#include <config_options.h>
#include <sal/types.h>
#include <cassert>
#include <vector>

#include <osl/diagnose.h>

#include <basegfx/color/bcolor.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class B3DPolygon;
    class B3DPolyPolygon;
}

// interpolators for double precision

namespace basegfx
{
    class ip_single
    {
    private:
        double                                      mfVal;
        double                                      mfInc;

    public:
        ip_single()
        :   mfVal(0.0),
            mfInc(0.0)
        {}

        ip_single(double fVal, double fInc)
        :   mfVal(fVal),
            mfInc(fInc)
        {}

        double getVal() const { return mfVal; }
        double getInc() const { return mfInc; }

        void increment(double fStep) { mfVal += fStep * mfInc; }
    };

    class ip_double
    {
    private:
        ip_single                                   maX;
        ip_single                                   maY;

    public:
        ip_double()
        {}

        ip_double(double fXVal, double fXInc, double fYVal, double fYInc)
        :   maX(fXVal, fXInc),
            maY(fYVal, fYInc)
        {}

        const ip_single& getX() const { return maX; }
        const ip_single& getY() const { return maY; }

        void increment(double fStep) { maX.increment(fStep); maY.increment(fStep); }
    };

    class ip_triple
    {
    private:
        ip_single                                   maX;
        ip_single                                   maY;
        ip_single                                   maZ;

    public:
        ip_triple()
        {}

        ip_triple(double fXVal, double fXInc, double fYVal, double fYInc, double fZVal, double fZInc)
        :   maX(fXVal, fXInc),
            maY(fYVal, fYInc),
            maZ(fZVal, fZInc)
        {}

        const ip_single& getX() const { return maX; }
        const ip_single& getY() const { return maY; }
        const ip_single& getZ() const { return maZ; }

        void increment(double fStep) { maX.increment(fStep); maY.increment(fStep); maZ.increment(fStep); }
    };

    // InterpolatorProvider3D to have a common source for allocating interpolators
    // which may then be addressed using the index to the vectors

    #define SCANLINE_EMPTY_INDEX (0xffffffff)

    class InterpolatorProvider3D
    {
    private:
        ::std::vector< ip_triple >                  maColorInterpolators;
        ::std::vector< ip_triple >                  maNormalInterpolators;
        ::std::vector< ip_double >                  maTextureInterpolators;
        ::std::vector< ip_triple >                  maInverseTextureInterpolators;

    protected:
        sal_uInt32 addColorInterpolator(const BColor& rA, const BColor& rB, double fInvYDelta)
        {
            double aDeltaRed(rB.getRed() - rA.getRed());

            if(fTools::equalZero(aDeltaRed))
            {
                aDeltaRed = 0.0;
            }
            else
            {
                aDeltaRed *= fInvYDelta;
            }

            double aDeltaGreen(rB.getGreen() - rA.getGreen());

            if(fTools::equalZero(aDeltaGreen))
            {
                aDeltaGreen = 0.0;
            }
            else
            {
                aDeltaGreen *= fInvYDelta;
            }

            double aDeltaBlue(rB.getBlue() - rA.getBlue());

            if(fTools::equalZero(aDeltaBlue))
            {
                aDeltaBlue = 0.0;
            }
            else
            {
                aDeltaBlue *= fInvYDelta;
            }

            maColorInterpolators.push_back(
                ip_triple(
                    rA.getRed(), aDeltaRed,
                    rA.getGreen(), aDeltaGreen,
                    rA.getBlue(), aDeltaBlue));

            return (maColorInterpolators.size() - 1);
        }

        sal_uInt32 addNormalInterpolator(const B3DVector& rA, const B3DVector& rB, double fInvYDelta)
        {
            double aDeltaX(rB.getX() - rA.getX());

            if(fTools::equalZero(aDeltaX))
            {
                aDeltaX = 0.0;
            }
            else
            {
                aDeltaX *= fInvYDelta;
            }

            double aDeltaY(rB.getY() - rA.getY());

            if(fTools::equalZero(aDeltaY))
            {
                aDeltaY = 0.0;
            }
            else
            {
                aDeltaY *= fInvYDelta;
            }

            double aDeltaZ(rB.getZ() - rA.getZ());

            if(fTools::equalZero(aDeltaZ))
            {
                aDeltaZ = 0.0;
            }
            else
            {
                aDeltaZ *= fInvYDelta;
            }

            maNormalInterpolators.push_back(
                ip_triple(
                    rA.getX(), aDeltaX,
                    rA.getY(), aDeltaY,
                    rA.getZ(), aDeltaZ));

            return (maNormalInterpolators.size() - 1);
        }

        sal_uInt32 addTextureInterpolator(const B2DPoint& rA, const B2DPoint& rB, double fInvYDelta)
        {
            double aDeltaX(rB.getX() - rA.getX());

            if(fTools::equalZero(aDeltaX))
            {
                aDeltaX = 0.0;
            }
            else
            {
                aDeltaX *= fInvYDelta;
            }

            double aDeltaY(rB.getY() - rA.getY());

            if(fTools::equalZero(aDeltaY))
            {
                aDeltaY = 0.0;
            }
            else
            {
                aDeltaY *= fInvYDelta;
            }

            maTextureInterpolators.push_back(
                ip_double(
                    rA.getX(), aDeltaX,
                    rA.getY(), aDeltaY));

            return (maTextureInterpolators.size() - 1);
        }

        sal_uInt32 addInverseTextureInterpolator(const B2DPoint& rA, const B2DPoint& rB, double fZEyeA, double fZEyeB, double fInvYDelta)
        {
            double fZDelta(fZEyeB - fZEyeA);
            const double fInvZEyeA(fTools::equalZero(fZEyeA) ? fZEyeA : 1.0 / fZEyeA);
            double fInvZEyeB(fInvZEyeA);

            if(fTools::equalZero(fZDelta))
            {
                fZDelta = 0.0;
            }
            else
            {
                fInvZEyeB = fTools::equalZero(fZEyeB) ? fZEyeB : 1.0 / fZEyeB;
                fZDelta = (fInvZEyeB - fInvZEyeA) * fInvYDelta;
            }

            const B2DPoint aInvA(rA * fInvZEyeA);
            const B2DPoint aInvB(rB * fInvZEyeB);
            const double aDeltaX((aInvB.getX() - aInvA.getX()) * fInvYDelta);
            const double aDeltaY((aInvB.getY() - aInvA.getY()) * fInvYDelta);

            maInverseTextureInterpolators.push_back(
                ip_triple(
                    aInvA.getX(), aDeltaX,
                    aInvA.getY(), aDeltaY,
                    fInvZEyeA, fZDelta));

            return (maInverseTextureInterpolators.size() - 1);
        }

        void reset()
        {
            maColorInterpolators.clear();
            maNormalInterpolators.clear();
            maTextureInterpolators.clear();
            maInverseTextureInterpolators.clear();
        }

    public:
        InterpolatorProvider3D() {}

        ::std::vector< ip_triple >& getColorInterpolators() { return maColorInterpolators; }
        ::std::vector< ip_triple >& getNormalInterpolators() { return maNormalInterpolators; }
        ::std::vector< ip_double >& getTextureInterpolators() { return maTextureInterpolators; }
        ::std::vector< ip_triple >& getInverseTextureInterpolators() { return maInverseTextureInterpolators; }
    };

    // RasterConversionLineEntry3D for Rasterconversion of 3D PolyPolygons

    class RasterConversionLineEntry3D
    {
    private:
        ip_single                                   maX;
        ip_single                                   maZ;
        sal_Int32                                   mnY;
        sal_uInt32                                  mnCount;

        sal_uInt32                                  mnColorIndex;
        sal_uInt32                                  mnNormalIndex;
        sal_uInt32                                  mnTextureIndex;
        sal_uInt32                                  mnInverseTextureIndex;

    public:
        RasterConversionLineEntry3D(const double& rfX, const double& rfDeltaX, const double& rfZ, const double& rfDeltaZ, sal_Int32 nY, sal_uInt32 nCount)
        :   maX(rfX, rfDeltaX),
            maZ(rfZ, rfDeltaZ),
            mnY(nY),
            mnCount(nCount),
            mnColorIndex(SCANLINE_EMPTY_INDEX),
            mnNormalIndex(SCANLINE_EMPTY_INDEX),
            mnTextureIndex(SCANLINE_EMPTY_INDEX),
            mnInverseTextureIndex(SCANLINE_EMPTY_INDEX)
        {}

        void setColorIndex(sal_uInt32 nIndex) { mnColorIndex = nIndex; }
        void setNormalIndex(sal_uInt32 nIndex) { mnNormalIndex = nIndex; }
        void setTextureIndex(sal_uInt32 nIndex) { mnTextureIndex = nIndex; }
        void setInverseTextureIndex(sal_uInt32 nIndex) { mnInverseTextureIndex = nIndex; }

        bool operator<(const RasterConversionLineEntry3D& rComp) const
        {
            if(mnY == rComp.mnY)
            {
                return maX.getVal() < rComp.maX.getVal();
            }

            return mnY < rComp.mnY;
        }

        bool decrementRasterConversionLineEntry3D(sal_uInt32 nStep)
        {
            if(nStep >= mnCount)
            {
                return false;
            }
            else
            {
                mnCount -= nStep;
                return true;
            }
        }

        void incrementRasterConversionLineEntry3D(sal_uInt32 nStep, InterpolatorProvider3D& rProvider)
        {
            const double fStep(static_cast<double>(nStep));
            maX.increment(fStep);
            maZ.increment(fStep);
            mnY += nStep;

            if(SCANLINE_EMPTY_INDEX != mnColorIndex)
            {
                rProvider.getColorInterpolators()[mnColorIndex].increment(fStep);
            }

            if(SCANLINE_EMPTY_INDEX != mnNormalIndex)
            {
                rProvider.getNormalInterpolators()[mnNormalIndex].increment(fStep);
            }

            if(SCANLINE_EMPTY_INDEX != mnTextureIndex)
            {
                rProvider.getTextureInterpolators()[mnTextureIndex].increment(fStep);
            }

            if(SCANLINE_EMPTY_INDEX != mnInverseTextureIndex)
            {
                rProvider.getInverseTextureInterpolators()[mnInverseTextureIndex].increment(fStep);
            }
        }

        // data read access
        const ip_single& getX() const { return maX; }
        sal_Int32 getY() const { return mnY; }
        const ip_single& getZ() const { return maZ; }
        sal_uInt32 getColorIndex() const { return mnColorIndex; }
        sal_uInt32 getNormalIndex() const { return mnNormalIndex; }
        sal_uInt32 getTextureIndex() const { return mnTextureIndex; }
        sal_uInt32 getInverseTextureIndex() const { return mnInverseTextureIndex; }
    };

    // the basic RasterConverter itself. Only one method needs to be overridden. The
    // class itself is pure virtual

    class UNLESS_MERGELIBS(BASEGFX_DLLPUBLIC) RasterConverter3D : public InterpolatorProvider3D
    {
    private:
        // the line entries for an area conversion run
        ::std::vector< RasterConversionLineEntry3D >            maLineEntries;

        struct lineComparator
        {
            bool operator()(const RasterConversionLineEntry3D* pA, const RasterConversionLineEntry3D* pB)
            {
                assert(pA && pB && "lineComparator: empty pointer (!)");
                return pA->getX().getVal() < pB->getX().getVal();
            }
        };

        SAL_DLLPRIVATE void addArea(const B3DPolygon& rFill, const B3DHomMatrix* pViewToEye);
        SAL_DLLPRIVATE void addArea(const B3DPolyPolygon& rFill, const B3DHomMatrix* pViewToEye);
        SAL_DLLPRIVATE void addEdge(const B3DPolygon& rFill, sal_uInt32 a, sal_uInt32 b, const B3DHomMatrix* pViewToEye);

        SAL_DLLPRIVATE void rasterconvertB3DArea(sal_Int32 nStartLine, sal_Int32 nStopLine);
        SAL_DLLPRIVATE void rasterconvertB3DEdge(const B3DPolygon& rLine, sal_uInt32 nA, sal_uInt32 nB, sal_Int32 nStartLine, sal_Int32 nStopLine, sal_uInt16 nLineWidth);

        virtual void processLineSpan(const RasterConversionLineEntry3D& rA, const RasterConversionLineEntry3D& rB, sal_Int32 nLine, sal_uInt32 nSpanCount) = 0;

    public:
        RasterConverter3D();
        virtual ~RasterConverter3D();

        void rasterconvertB3DPolyPolygon(const B3DPolyPolygon& rFill, const B3DHomMatrix* pViewToEye, sal_Int32 nStartLine, sal_Int32 nStopLine);
        void rasterconvertB3DPolygon(const B3DPolygon& rLine, sal_Int32 nStartLine, sal_Int32 nStopLine, sal_uInt16 nLineWidth);
    };
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
