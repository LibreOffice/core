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
#include <com/sun/star/awt/Gradient2.hpp>

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
        /* Tooling method to convert UNO API data to ColorStops
           This will try to extract ColorStop data from the given
           Any, so if it's of type awt::Gradient2 that data will be
           extracted, converted and copied into the given ColorStops.
        */
        void fillColorStopsFromAny(ColorStops& rColorStops, const css::uno::Any& rVal)
        {
            css::awt::Gradient2 aGradient2;
            if (!(rVal >>= aGradient2))
                return;

            const sal_Int32 nLen(aGradient2.ColorStops.getLength());

            if (0 == nLen)
                return;

            // we have ColorStops
            rColorStops.clear();
            rColorStops.reserve(nLen);
            const css::awt::ColorStop* pSourceColorStop(aGradient2.ColorStops.getConstArray());

            for (sal_Int32 a(0); a < nLen; a++, pSourceColorStop++)
            {
                rColorStops.emplace_back(
                    pSourceColorStop->StopOffset,
                    BColor(pSourceColorStop->StopColor.Red, pSourceColorStop->StopColor.Green, pSourceColorStop->StopColor.Blue));
            }
        }

        /* Tooling method to fill a awt::ColorStopSequence with
           the data from the given ColorStops. This is used in
           UNO API implementations.
        */
        void fillColorStopSequenceFromColorStops(css::awt::ColorStopSequence& rColorStopSequence, const ColorStops& rColorStops)
        {
            // fill ColorStops to extended Gradient2
            rColorStopSequence.realloc(rColorStops.size());
            css::awt::ColorStop* pTargetColorStop(rColorStopSequence.getArray());

            for (const auto& candidate : rColorStops)
            {
                pTargetColorStop->StopOffset = candidate.getStopOffset();
                pTargetColorStop->StopColor = css::rendering::RGBColor(
                    candidate.getStopColor().getRed(),
                    candidate.getStopColor().getGreen(),
                    candidate.getStopColor().getBlue());
                pTargetColorStop++;
            }
        }

        /* Tooling method that allows to replace the StartColor in a
           vector of ColorStops. A vector in 'ordered state' is expected,
           so you may use/have used sortAndCorrectColorStops, see below.
           This method is for convenience & backwards compatibility, please
           think about handling multi-colored gradients directly.
        */
        void replaceStartColor(ColorStops& rColorStops, const BColor& rStart)
        {
            ColorStops::iterator a1stNonStartColor(rColorStops.begin());

            // search for highest existing non-StartColor
            while (a1stNonStartColor != rColorStops.end() && basegfx::fTools::lessOrEqual(a1stNonStartColor->getStopOffset(), 0.0))
                a1stNonStartColor++;

            // create new ColorStops by 1st adding new one and then all
            // non-StartColor entries
            ColorStops aNewColorStops;

            aNewColorStops.reserve(rColorStops.size() + 1);
            aNewColorStops.emplace_back(0.0, rStart);
            aNewColorStops.insert(aNewColorStops.end(), a1stNonStartColor, rColorStops.end());

            // assign & done
            rColorStops = aNewColorStops;
        }

        /* Tooling method that allows to replace the EndColor in a
           vector of ColorStops. A vector in 'ordered state' is expected,
           so you may use/have used sortAndCorrectColorStops, see below.
           This method is for convenience & backwards compatibility, please
           think about handling multi-colored gradients directly.
        */
        void replaceEndColor(ColorStops& rColorStops, const BColor& rEnd)
        {
            // erase all evtl. existing EndColor(s)
            while (!rColorStops.empty() && basegfx::fTools::moreOrEqual(rColorStops.back().getStopOffset(), 1.0))
                rColorStops.pop_back();

            // add at the end of existing ColorStops
            rColorStops.emplace_back(1.0, rEnd);
        }

        // Tooling method to quickly create a ColorStop vector for a given set of Start/EndColor
        ColorStops createColorStopsFromStartEndColor(const BColor& rStart, const BColor& rEnd)
        {
            return ColorStops {
                ColorStop(0.0, rStart),
                ColorStop(1.0, rEnd) };
        }

        /* Tooling method to guarantee sort and correctness for
           the given ColorStops vector.
           A vector fulfilling these conditions is called to be
           in 'ordered state'.

           At return, the following conditions are guaranteed:
           - contains no ColorStops with offset < 0.0 (will
             be removed)
           - contains no ColorStops with offset > 1.0 (will
             be removed)
           - contains no two ColorStops with identical offsets
             (will be removed, 1st one/smallest offset wins
             which is also given by sort tooling)
           - will be sorted from lowest offset to highest
           - if all colors are the same, the content will
             be reduced to a single entry with offset 0.0
             (force to StartColor)

           Some more notes:
           - It can happen that the result is empty
           - It is allowed to have consecutive entries with
             the same color, this represents single-color
             regions inside the gradient
           - A entry with 0.0 is not required or forced, so
             no 'StartColor' is technically required
           - A entry with 1.0 is not required or forced, so
             no 'EndColor' is technically required

           All this is done in one run (sort + O(N)) without
           creating a copy of the data in any form
        */
        void sortAndCorrectColorStops(ColorStops& rColorStops)
        {
            // no content, we are done
            if (rColorStops.empty())
                return;

            if (1 == rColorStops.size())
            {
                // no gradient at all, but preserve given color
                // and force it to be the StartColor
                rColorStops[0] = ColorStop(0.0, rColorStops[0].getStopColor());
            }

            // start with sorting the input data. Remember that
            // this preserves the order of equal entries, where
            // equal is defined here by offset (see use operator==)
            std::sort(rColorStops.begin(), rColorStops.end());

            // prepare status values
            bool bSameColorInit(false);
            bool bAllTheSameColor(true);
            basegfx::BColor aFirstColor;
            size_t write(0);

            // use the paradigm of a band machine with two heads, read
            // and write with write <= read all the time. Step over the
            // data using read and check for valid entry. If valid, decide
            // how to keep it
            for (size_t read(0); read < rColorStops.size(); read++)
            {
                // get offset of entry at read position
                const double rOff(rColorStops[read].getStopOffset());

                // step over < 0 values
                if (basegfx::fTools::less(rOff, 0.0))
                    continue;

                // step over > 1 values; even break, since all following
                // entries will also be bigger due to being sorted, so done
                if (basegfx::fTools::more(rOff, 1.0))
                    break;

                // entry is valid value at read position

                // check/init for all-the-same color
                if(bSameColorInit)
                {
                    // already initialized, compare
                    bAllTheSameColor = bAllTheSameColor && aFirstColor == rColorStops[read].getStopColor();
                }
                else
                {
                    // do initialize, remember 1st valid color
                    bSameColorInit = true;
                    aFirstColor = rColorStops[read].getStopColor();
                }

                // copy if write target is empty (write at start) or when
                // write target is different to read
                if (0 == write || rOff != rColorStops[write-1].getStopOffset())
                {
                    if (write != read)
                    {
                        // copy read to write backwards to close gaps
                        rColorStops[write] = rColorStops[read];
                    }

                    // always forward write position
                    write++;
                }
            }

            // correct size when length is reduced. write is always at
            // last used position + 1
            if (rColorStops.size() > write)
            {
                rColorStops.resize(write);
            }

            if (bSameColorInit && bAllTheSameColor && rColorStops.size() > 1)
            {
                // id all-the-same color is detected, reset to single
                // entry, but also force to StartColor and preserve the color
                rColorStops.resize(1);
                rColorStops[0] = ColorStop(0.0, aFirstColor);
            }
        }

        BColor modifyBColor(
            const ColorStops& rColorStops,
            double fScaler,
            sal_uInt32 nRequestedSteps)
        {
            // no color at all, done
            if (rColorStops.empty())
                return BColor();

            // outside range -> at start
            if (fScaler <= 0.0)
                return rColorStops.front().getStopColor();

            // outside range -> at end
            if (fScaler >= 1.0)
                return rColorStops.back().getStopColor();

            // special case for the 'classic' case with just two colors:
            // we can optimize that and keep the speed/resources low
            // by avoiding some calculations and an O(log(N)) array access
            if (2 == rColorStops.size())
            {
                const basegfx::BColor aCStart(rColorStops.front().getStopColor());
                const basegfx::BColor aCEnd(rColorStops.back().getStopColor());
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
                    rColorStops.begin(),
                    rColorStops.end(),
                    ColorStop(fScaler),
                    [](const ColorStop& x, const ColorStop& y) { return x.getStopOffset() < y.getStopOffset(); }));

            // no upper bound, done
            if (rColorStops.end() == upperBound)
                return rColorStops.back().getStopColor();

            // lower bound is one entry back
            const auto lowerBound(upperBound - 1);

            // no lower bound, done
            if (rColorStops.end() == lowerBound)
                return rColorStops.back().getStopColor();

            // we have lower and upper bound, get colors
            const BColor aCStart(lowerBound->getStopColor());
            const BColor aCEnd(upperBound->getStopColor());

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
            const double fOffsetStart(lowerBound->getStopOffset());
            const double fOffsetEnd(upperBound->getStopOffset());
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

            return 1.0 - std::hypot(aCoor.getX(), aCoor.getY());
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
            // gradient look 'nicer' this correction seems not 100%
            // correct, but is close enough for now
            if(fAspectRatio > 1.0)
            {
                t = 1.0 - std::hypot(aCoor.getX() / fAspectRatio, aCoor.getY());
            }
            else if(fAspectRatio > 0.0)
            {
                t = 1.0 - std::hypot(aCoor.getX(), aCoor.getY() * fAspectRatio);
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

            return 1.0 - std::max(fAbsX, fAbsY);
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

            // MCGR: Visualizations using the texturing method for
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

            return 1.0 - std::max(fAbsX, fAbsY);
        }
    } // namespace utils
} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
