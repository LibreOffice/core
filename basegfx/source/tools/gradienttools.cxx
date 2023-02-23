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

#include <basegfx/utils/gradienttools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <algorithm>
#include <cmath>

namespace basegfx
{
    bool ODFGradientInfo::operator==(const ODFGradientInfo& rODFGradientInfo) const
    {
        return getTextureTransform() == rODFGradientInfo.getTextureTransform()
            && getAspectRatio() == rODFGradientInfo.getAspectRatio()
            && getRequestedSteps() == rODFGradientInfo.getRequestedSteps();
    }

    const B2DHomMatrix& ODFGradientInfo::getBackTextureTransform() const
    {
        if(maBackTextureTransform.isIdentity())
        {
            const_cast< ODFGradientInfo* >(this)->maBackTextureTransform = getTextureTransform();
            const_cast< ODFGradientInfo* >(this)->maBackTextureTransform.invert();
        }

        return maBackTextureTransform;
    }

    /** Most of the setup for linear & axial gradient is the same, except
        for the border treatment. Factored out here.
    */
    static ODFGradientInfo init1DGradientInfo(
        const B2DRange& rTargetRange,
        sal_uInt32 nSteps,
        double fBorder,
        double fAngle,
        bool bAxial)
    {
        B2DHomMatrix aTextureTransform;

        fAngle = -fAngle;

        double fTargetSizeX(rTargetRange.getWidth());
        double fTargetSizeY(rTargetRange.getHeight());
        double fTargetOffsetX(rTargetRange.getMinX());
        double fTargetOffsetY(rTargetRange.getMinY());

        // add object expansion
        const bool bAngleUsed(!fTools::equalZero(fAngle));

        if(bAngleUsed)
        {
            const double fAbsCos(fabs(cos(fAngle)));
            const double fAbsSin(fabs(sin(fAngle)));
            const double fNewX(fTargetSizeX * fAbsCos + fTargetSizeY * fAbsSin);
            const double fNewY(fTargetSizeY * fAbsCos + fTargetSizeX * fAbsSin);

            fTargetOffsetX -= (fNewX - fTargetSizeX) / 2.0;
            fTargetOffsetY -= (fNewY - fTargetSizeY) / 2.0;
            fTargetSizeX = fNewX;
            fTargetSizeY = fNewY;
        }

        const double fSizeWithoutBorder(1.0 - fBorder);

        if(bAxial)
        {
            aTextureTransform.scale(1.0, fSizeWithoutBorder * 0.5);
            aTextureTransform.translate(0.0, 0.5);
        }
        else
        {
            if(!fTools::equal(fSizeWithoutBorder, 1.0))
            {
                aTextureTransform.scale(1.0, fSizeWithoutBorder);
                aTextureTransform.translate(0.0, fBorder);
            }
        }

        aTextureTransform.scale(fTargetSizeX, fTargetSizeY);

        // add texture rotate after scale to keep perpendicular angles
        if(bAngleUsed)
        {
            const B2DPoint aCenter(0.5 * fTargetSizeX, 0.5 * fTargetSizeY);

            aTextureTransform *= basegfx::utils::createRotateAroundPoint(aCenter, fAngle);
        }

        // add object translate
        aTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

        // prepare aspect for texture
        const double fAspectRatio(fTools::equalZero(fTargetSizeY) ?  1.0 : fTargetSizeX / fTargetSizeY);

        return ODFGradientInfo(aTextureTransform, fAspectRatio, nSteps);
    }

    /** Most of the setup for radial & ellipsoidal gradient is the same,
        except for the border treatment. Factored out here.
    */
    static ODFGradientInfo initEllipticalGradientInfo(
        const B2DRange& rTargetRange,
        const B2DVector& rOffset,
        sal_uInt32 nSteps,
        double fBorder,
        double fAngle,
        bool bCircular)
    {
        B2DHomMatrix aTextureTransform;

        fAngle = -fAngle;

        double fTargetSizeX(rTargetRange.getWidth());
        double fTargetSizeY(rTargetRange.getHeight());
        double fTargetOffsetX(rTargetRange.getMinX());
        double fTargetOffsetY(rTargetRange.getMinY());

        // add object expansion
        if(bCircular)
        {
            const double fOriginalDiag(std::hypot(fTargetSizeX, fTargetSizeY));

            fTargetOffsetX -= (fOriginalDiag - fTargetSizeX) / 2.0;
            fTargetOffsetY -= (fOriginalDiag - fTargetSizeY) / 2.0;
            fTargetSizeX = fOriginalDiag;
            fTargetSizeY = fOriginalDiag;
        }
        else
        {
            fTargetOffsetX -= ((M_SQRT2 - 1) / 2.0 ) * fTargetSizeX;
            fTargetOffsetY -= ((M_SQRT2 - 1) / 2.0 ) * fTargetSizeY;
            fTargetSizeX = M_SQRT2 * fTargetSizeX;
            fTargetSizeY = M_SQRT2 * fTargetSizeY;
        }

        const double fHalfBorder((1.0 - fBorder) * 0.5);

        aTextureTransform.scale(fHalfBorder, fHalfBorder);
        aTextureTransform.translate(0.5, 0.5);
        aTextureTransform.scale(fTargetSizeX, fTargetSizeY);

        // add texture rotate after scale to keep perpendicular angles
        if(!bCircular && !fTools::equalZero(fAngle))
        {
            const B2DPoint aCenter(0.5 * fTargetSizeX, 0.5 * fTargetSizeY);

            aTextureTransform *= basegfx::utils::createRotateAroundPoint(aCenter, fAngle);
        }

        // add defined offsets after rotation
        if(!fTools::equal(0.5, rOffset.getX()) || !fTools::equal(0.5, rOffset.getY()))
        {
            // use original target size
            fTargetOffsetX += (rOffset.getX() - 0.5) * rTargetRange.getWidth();
            fTargetOffsetY += (rOffset.getY() - 0.5) * rTargetRange.getHeight();
        }

        // add object translate
        aTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

        // prepare aspect for texture
        const double fAspectRatio(fTargetSizeY == 0.0 ? 1.0 : (fTargetSizeX / fTargetSizeY));

        return ODFGradientInfo(aTextureTransform, fAspectRatio, nSteps);
    }

    /** Setup for rect & square gradient is exactly the same. Factored out
        here.
    */
    static ODFGradientInfo initRectGradientInfo(
        const B2DRange& rTargetRange,
        const B2DVector& rOffset,
        sal_uInt32 nSteps,
        double fBorder,
        double fAngle,
        bool bSquare)
    {
        B2DHomMatrix aTextureTransform;

        fAngle = -fAngle;

        double fTargetSizeX(rTargetRange.getWidth());
        double fTargetSizeY(rTargetRange.getHeight());
        double fTargetOffsetX(rTargetRange.getMinX());
        double fTargetOffsetY(rTargetRange.getMinY());

        // add object expansion
        if(bSquare)
        {
            const double fSquareWidth(std::max(fTargetSizeX, fTargetSizeY));

            fTargetOffsetX -= (fSquareWidth - fTargetSizeX) / 2.0;
            fTargetOffsetY -= (fSquareWidth - fTargetSizeY) / 2.0;
            fTargetSizeX = fTargetSizeY = fSquareWidth;
        }

        // add object expansion
        const bool bAngleUsed(!fTools::equalZero(fAngle));

        if(bAngleUsed)
        {
            const double fAbsCos(fabs(cos(fAngle)));
            const double fAbsSin(fabs(sin(fAngle)));
            const double fNewX(fTargetSizeX * fAbsCos + fTargetSizeY * fAbsSin);
            const double fNewY(fTargetSizeY * fAbsCos + fTargetSizeX * fAbsSin);

            fTargetOffsetX -= (fNewX - fTargetSizeX) / 2.0;
            fTargetOffsetY -= (fNewY - fTargetSizeY) / 2.0;
            fTargetSizeX = fNewX;
            fTargetSizeY = fNewY;
        }

        const double fHalfBorder((1.0 - fBorder) * 0.5);

        aTextureTransform.scale(fHalfBorder, fHalfBorder);
        aTextureTransform.translate(0.5, 0.5);
        aTextureTransform.scale(fTargetSizeX, fTargetSizeY);

        // add texture rotate after scale to keep perpendicular angles
        if(bAngleUsed)
        {
            const B2DPoint aCenter(0.5 * fTargetSizeX, 0.5 * fTargetSizeY);

            aTextureTransform *= basegfx::utils::createRotateAroundPoint(aCenter, fAngle);
        }

        // add defined offsets after rotation
        if(!fTools::equal(0.5, rOffset.getX()) || !fTools::equal(0.5, rOffset.getY()))
        {
            // use original target size
            fTargetOffsetX += (rOffset.getX() - 0.5) * rTargetRange.getWidth();
            fTargetOffsetY += (rOffset.getY() - 0.5) * rTargetRange.getHeight();
        }

        // add object translate
        aTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

        // prepare aspect for texture
        const double fAspectRatio(fTargetSizeY == 0.0 ? 1.0 : (fTargetSizeX / fTargetSizeY));

        return ODFGradientInfo(aTextureTransform, fAspectRatio, nSteps);
    }

    namespace utils
    {
        BColor modifyBColor(
            const ColorSteps& rColorSteps,
            double fScaler,
            sal_uInt32 nRequestedSteps)
        {
            // no color at all, done
            if (rColorSteps.empty())
                return BColor();

            // outside range -> at start
            if (fScaler <= 0.0)
                return rColorSteps.front().getColor();

            // outside range -> at end
            if (fScaler >= 1.0)
                return rColorSteps.back().getColor();

            // special case for the 'classic' case with just two colors:
            // we can optimize that and keep the speed/ressources low
            // by avoiding some calculatins and an O(log(N)) array access
            if (2 == rColorSteps.size())
            {
                const basegfx::BColor aCStart(rColorSteps.front().getColor());
                const basegfx::BColor aCEnd(rColorSteps.back().getColor());
                const sal_uInt32 nSteps(
                    calculateNumberOfSteps(
                        nRequestedSteps,
                        aCStart,
                        aCEnd));

                return basegfx::interpolate(
                    aCStart,
                    aCEnd,
                    nSteps > 1 ? floor(fScaler * nSteps) / double(nSteps - 1) : fScaler);
            }

            // access needed spot in sorted array using binary search
            // NOTE: This *seems* slow(er) when developing compared to just
            //       looping/accessing, but that's just due to the extensive
            //       debug test code created by the stl. In a pro version,
            //       all is good/fast as expected
            const auto upperBound(
                std::lower_bound(
                    rColorSteps.begin(),
                    rColorSteps.end(),
                    ColorStep(fScaler),
                    [](const ColorStep& x, const ColorStep& y) { return x.getOffset() < y.getOffset(); }));

            // no upper bound, done
            if (rColorSteps.end() == upperBound)
                return rColorSteps.back().getColor();

            // lower bound is one entry back
            const auto lowerBound(upperBound - 1);

            // no lower bound, done
            if (rColorSteps.end() == lowerBound)
                return rColorSteps.back().getColor();

            // we have lower and upper bound, get colors
            const BColor aCStart(lowerBound->getColor());
            const BColor aCEnd(upperBound->getColor());

            // when there are just two color steps this cannot happen, but when using
            // a range of colors this *may* be used inside the range to represent
            // single-colored regions inside a ColorRange. Use that color & done
            if (aCStart == aCEnd)
                return aCStart;

            // calculate number of steps
            const sal_uInt32 nSteps(
                calculateNumberOfSteps(
                    nRequestedSteps,
                    aCStart,
                    aCEnd));

            // get offsets and scale to new [0.0 .. 1.0] relative range for
            // partial outer range
            const double fOffsetStart(lowerBound->getOffset());
            const double fOffsetEnd(upperBound->getOffset());
            const double fAdaptedScaler((fScaler - fOffsetStart) / (fOffsetEnd - fOffsetStart));

            // interpolate & evtl. apply steps
            return interpolate(
                aCStart,
                aCEnd,
                nSteps > 1 ? floor(fAdaptedScaler * nSteps) / double(nSteps - 1) : fAdaptedScaler);
        }

        sal_uInt32 calculateNumberOfSteps(
            sal_uInt32 nRequestedSteps,
            const BColor& rStart,
            const BColor& rEnd)
        {
            const sal_uInt32 nMaxSteps(sal_uInt32((rStart.getMaximumDistance(rEnd) * 127.5) + 0.5));

            if (0 == nRequestedSteps)
            {
                nRequestedSteps = nMaxSteps;
            }

            if(nRequestedSteps > nMaxSteps)
            {
                nRequestedSteps = nMaxSteps;
            }

            return std::max(sal_uInt32(1), nRequestedSteps);
        }

        ODFGradientInfo createLinearODFGradientInfo(
            const B2DRange& rTargetArea,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        {
            return init1DGradientInfo(
                rTargetArea,
                nSteps,
                fBorder,
                fAngle,
                false);
        }

        ODFGradientInfo createAxialODFGradientInfo(
            const B2DRange& rTargetArea,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        {
            return init1DGradientInfo(
                rTargetArea,
                nSteps,
                fBorder,
                fAngle,
                true);
        }

        ODFGradientInfo createRadialODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder)
        {
            return initEllipticalGradientInfo(
                rTargetArea,
                rOffset,
                nSteps,
                fBorder,
                0.0,
                true);
        }

        ODFGradientInfo createEllipticalODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        {
            return initEllipticalGradientInfo(
                rTargetArea,
                rOffset,
                nSteps,
                fBorder,
                fAngle,
                false);
        }

        ODFGradientInfo createSquareODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        {
            return initRectGradientInfo(
                rTargetArea,
                rOffset,
                nSteps,
                fBorder,
                fAngle,
                true);
        }

        ODFGradientInfo createRectangularODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        {
            return initRectGradientInfo(
                rTargetArea,
                rOffset,
                nSteps,
                fBorder,
                fAngle,
                false);
        }

        double getLinearGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);

            // Ignore X, this is not needed at all for Y-Oriented gradients
            // if(aCoor.getX() < 0.0 || aCoor.getX() > 1.0)
            // {
            //     return 0.0;
            // }

            if(aCoor.getY() <= 0.0)
            {
                return 0.0; // start value for inside
            }

            if(aCoor.getY() >= 1.0)
            {
                return 1.0; // end value for outside
            }

            return aCoor.getY();
        }

        double getAxialGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);

            // Ignore X, this is not needed at all for Y-Oriented gradients
            //if(aCoor.getX() < 0.0 || aCoor.getX() > 1.0)
            //{
            //    return 0.0;
            //}

            const double fAbsY(fabs(aCoor.getY()));

            if(fAbsY >= 1.0)
            {
                return 1.0; // use end value when outside in Y
            }

            return fAbsY;
        }

        double getRadialGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);

            if(aCoor.getX() < -1.0 || aCoor.getX() > 1.0 || aCoor.getY() < -1.0 || aCoor.getY() > 1.0)
            {
                return 0.0;
            }

            const double t(1.0 - std::hypot(aCoor.getX(), aCoor.getY()));
            const sal_uInt32 nSteps(rGradInfo.getRequestedSteps());

            if(nSteps && t < 1.0)
            {
                return floor(t * nSteps) / double(nSteps - 1);
            }

            return t;
        }

        double getEllipticalGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);

            if(aCoor.getX() < -1.0 || aCoor.getX() > 1.0 || aCoor.getY() < -1.0 || aCoor.getY() > 1.0)
            {
                return 0.0;
            }

            double fAspectRatio(rGradInfo.getAspectRatio());
            double t(1.0);

            // MCGR: Similar to getRectangularGradientAlpha (please
            // see there) we need to use aspect ratio here. Due to
            // initEllipticalGradientInfo using M_SQRT2 to make this
            // gradient look 'nicer' this correciton seems not 100%
            // correct, but is close enough for now
            if(fAspectRatio > 1.0)
            {
                t = 1.0 - std::hypot(aCoor.getX() / fAspectRatio, aCoor.getY());
            }
            else if(fAspectRatio > 0.0)
            {
                t = 1.0 - std::hypot(aCoor.getX(), aCoor.getY() * fAspectRatio);
            }

            const sal_uInt32 nSteps(rGradInfo.getRequestedSteps());

            if(nSteps && t < 1.0)
            {
                return floor(t * nSteps) / double(nSteps - 1);
            }

            return t;
        }

        double getSquareGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);
            const double fAbsX(fabs(aCoor.getX()));

            if(fAbsX >= 1.0)
            {
                return 0.0;
            }

            const double fAbsY(fabs(aCoor.getY()));

            if(fAbsY >= 1.0)
            {
                return 0.0;
            }

            const double t(1.0 - std::max(fAbsX, fAbsY));
            const sal_uInt32 nSteps(rGradInfo.getRequestedSteps());

            if(nSteps && t < 1.0)
            {
                return floor(t * nSteps) / double(nSteps - 1);
            }

            return t;
        }

        double getRectangularGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);
            double fAbsX(fabs(aCoor.getX()));

            if(fAbsX >= 1.0)
            {
                return 0.0;
            }

            double fAbsY(fabs(aCoor.getY()));

            if(fAbsY >= 1.0)
            {
                return 0.0;
            }

            // MCGR: Visualiations using the texturing method for
            // displaying gradients (getBackTextureTransform is
            // involved) show wrong results for GradientElliptical
            // and GradientRect, this can be best seen when using
            // less steps, e.g. just four. This thus has influence
            // on cppcanvas (slideshow) and 3D textures, so needs
            // to be corrected.
            // Missing is to use the aspect ratio of the object
            // in this [-1, -1, 1, 1] unified coordinate space
            // after getBackTextureTransform is applied. Optically
            // in the larger direction of the texturing the color
            // step distances are too big *because* we are in that
            // unit range now.
            // To correct that, a kind of 'limo stretching' needs to
            // be applied, adding space around the center
            // proportional to the aspect ratio, so the intuitive
            // idea would be to do
            //
            // fAbsX' = ((fAspectRatio - 1) + fAbsX) / fAspectRatio
            //
            // which scales from the center. This does not work, and
            // after some thoughts it's clear why: It's not the
            // position that needs to be moved (this cannot be
            // changed), but the position *before* that scale has
            // to be determined to get the correct, shifted color
            // for the already 'new' position. Thus, turn around
            // the expression as
            //
            // fAbsX' * fAspectRatio = fAspectRatio - 1 + fAbsX
            // fAbsX' * fAspectRatio - fAspectRatio + 1 = fAbsX
            // fAbsX = (fAbsX' - 1) * fAspectRatio + 1
            //
            // This works and can even be simply adapted for
            // fAspectRatio < 1.0 aka vertical is bigger.
            double fAspectRatio(rGradInfo.getAspectRatio());
            if(fAspectRatio > 1.0)
            {
                fAbsX = ((fAbsX - 1) * fAspectRatio) + 1;
            }
            else if(fAspectRatio > 0.0)
            {
                fAbsY = ((fAbsY - 1) / fAspectRatio) + 1;
            }

            const double t(1.0 - std::max(fAbsX, fAbsY));
            const sal_uInt32 nSteps(rGradInfo.getRequestedSteps());

            if(nSteps && t < 1.0)
            {
                return floor(t * nSteps) / double(nSteps - 1);
            }

            return t;
        }
    } // namespace utils
} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
