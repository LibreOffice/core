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

#include <sal/config.h>

#include <algorithm>
#include <limits>

#include <texture/texture.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <comphelper/random.hxx>

namespace drawinglayer::texture
{
        namespace
        {
            double getRandomColorRange()
            {
                return comphelper::rng::uniform_real_distribution(0.0, nextafter(1.0, DBL_MAX));
            }
        }

        GeoTexSvx::GeoTexSvx()
        {
        }

        GeoTexSvx::~GeoTexSvx()
        {
        }

        bool GeoTexSvx::operator==(const GeoTexSvx& /*rGeoTexSvx*/) const
        {
            // default implementation says yes (no data -> no difference)
            return true;
        }

        void GeoTexSvx::modifyBColor(const basegfx::B2DPoint& /*rUV*/, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            // base implementation creates random color (for testing only, may also be pure virtual)
            rBColor.setRed(getRandomColorRange());
            rBColor.setGreen(getRandomColorRange());
            rBColor.setBlue(getRandomColorRange());
        }

        void GeoTexSvx::modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const
        {
            // base implementation uses inverse of luminance of solved color (for testing only, may also be pure virtual)
            basegfx::BColor aBaseColor;
            modifyBColor(rUV, aBaseColor, rfOpacity);
            rfOpacity = 1.0 - aBaseColor.luminance();
        }


        GeoTexSvxGradient::GeoTexSvxGradient(
            const basegfx::B2DRange& rDefinitionRange,
            sal_uInt32 nRequestedSteps,
            const basegfx::BColorStops& rColorStops,
            double fBorder)
        : maDefinitionRange(rDefinitionRange)
        , mnRequestedSteps(nRequestedSteps)
        , mnColorStops(rColorStops)
        , mfBorder(fBorder)
        , maLastColorStopRange()
        {
        }

        GeoTexSvxGradient::~GeoTexSvxGradient()
        {
        }

        bool GeoTexSvxGradient::operator==(const GeoTexSvx& rGeoTexSvx) const
        {
            const GeoTexSvxGradient* pCompare = dynamic_cast< const GeoTexSvxGradient* >(&rGeoTexSvx);

            return (pCompare
                && maGradientInfo == pCompare->maGradientInfo
                && maDefinitionRange == pCompare->maDefinitionRange
                && mnRequestedSteps == pCompare->mnRequestedSteps
                && mnColorStops == pCompare->mnColorStops
                && mfBorder == pCompare->mfBorder);
        }

        GeoTexSvxGradientLinear::GeoTexSvxGradientLinear(
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::B2DRange& rOutputRange,
            sal_uInt32 nRequestedSteps,
            const basegfx::BColorStops& rColorStops,
            double fBorder,
            double fAngle)
        : GeoTexSvxGradient(rDefinitionRange, nRequestedSteps, rColorStops, fBorder)
        , mfUnitMinX(0.0)
        , mfUnitWidth(1.0)
        , mfUnitMaxY(1.0)
        {
            maGradientInfo = basegfx::utils::createLinearODFGradientInfo(
                rDefinitionRange,
                nRequestedSteps,
                fBorder,
                fAngle);

            if(rDefinitionRange != rOutputRange)
            {
                basegfx::B2DRange aInvOutputRange(rOutputRange);

                aInvOutputRange.transform(maGradientInfo.getBackTextureTransform());
                mfUnitMinX = aInvOutputRange.getMinX();
                mfUnitWidth = aInvOutputRange.getWidth();
                mfUnitMaxY = aInvOutputRange.getMaxY();
            }
        }

        GeoTexSvxGradientLinear::~GeoTexSvxGradientLinear()
        {
        }

        void GeoTexSvxGradientLinear::appendTransformationsAndColors(
            std::function<void(const basegfx::B2DHomMatrix& rMatrix, const basegfx::BColor& rColor)> aCallback)
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // only one color, done
            if (mnColorStops.size() < 2)
                return;

            // check if we need last-ColorStop-correction
            const bool bPenultimateUsed(mnColorStops.checkPenultimate());

            if (bPenultimateUsed)
            {
                // Here we need to temporarily add a ColorStop entry with the
                // same color as the last entry to correctly 'close' the
                // created gradient geometry.
                // The simplest way is to temporarily add an entry to the local
                // ColorStops for this at 1.0 (using same color)
                mnColorStops.emplace_back(1.0, mnColorStops.back().getStopColor());
            }

            // prepare unit range transform
            basegfx::B2DHomMatrix aPattern;

            // bring from unit circle [-1, -1, 1, 1] to unit range [0, 0, 1, 1]
            aPattern.scale(0.5, 0.5);
            aPattern.translate(0.5, 0.5);

            // scale and translate in X
            aPattern.scale(mfUnitWidth, 1.0);
            aPattern.translate(mfUnitMinX, 0.0);

            // outer loop over ColorStops, each is from cs_l to cs_r
            for (auto cs_l(mnColorStops.begin()), cs_r(cs_l + 1); cs_r != mnColorStops.end(); cs_l++, cs_r++)
            {
                // get offsets
                const double fOffsetStart(cs_l->getStopOffset());
                const double fOffsetEnd(cs_r->getStopOffset());

                // same offset, empty BColorStopRange, continue with next step
                if (basegfx::fTools::equal(fOffsetStart, fOffsetEnd))
                    continue;

                // get colors & calculate steps
                const basegfx::BColor aCStart(cs_l->getStopColor());
                const basegfx::BColor aCEnd(cs_r->getStopColor());
                const sal_uInt32 nSteps(basegfx::utils::calculateNumberOfSteps(
                    maGradientInfo.getRequestedSteps(), aCStart, aCEnd));

                // calculate StripeWidth
                // nSteps is >= 1, see getRequestedSteps, so no check needed here
                const double fStripeWidth((fOffsetEnd - fOffsetStart) / nSteps);

                // for the 1st color range we do not need to create the 1st step
                // since it will be equal to StartColor and thus OuterColor, so
                // will be painted by the 1st, always-created background polygon
                // colored using OuterColor.
                // We *need* to create this though for all 'inner' color ranges
                // to get a correct start
                const sal_uInt32 nStartInnerLoop(cs_l == mnColorStops.begin() ? 1 : 0);

                for (sal_uInt32 innerLoop(nStartInnerLoop); innerLoop < nSteps; innerLoop++)
                {
                    // calculate pos in Y
                    const double fPos(fOffsetStart + (fStripeWidth * innerLoop));

                    // scale and translate in Y. For GradientLinear we always have
                    // the full height
                    double fHeight(1.0 - fPos);

                    if (mfUnitMaxY > 1.0)
                    {
                        // extend when difference between definition and OutputRange exists
                        fHeight += mfUnitMaxY - 1.0;
                    }

                    basegfx::B2DHomMatrix aNew(aPattern);
                    aNew.scale(1.0, fHeight);
                    aNew.translate(0.0, fPos);

                    // set and add at target
                    aCallback(
                        maGradientInfo.getTextureTransform() * aNew,
                        1 == nSteps ? aCStart : basegfx::BColor(interpolate(aCStart, aCEnd, double(innerLoop) / double(nSteps - 1))));
                }
            }

            if (bPenultimateUsed)
            {
                // correct temporary change
                mnColorStops.pop_back();
            }
        }

        void GeoTexSvxGradientLinear::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // just single color, done
            if (mnColorStops.size() < 2)
            {
                rBColor = mnColorStops.front().getStopColor();
                return;
            }

            // texture-back-transform X/Y -> t [0.0..1.0] and determine color
            const double fScaler(basegfx::utils::getLinearGradientAlpha(rUV, maGradientInfo));
            rBColor = mnColorStops.getInterpolatedBColor(fScaler, mnRequestedSteps, maLastColorStopRange);
        }

        GeoTexSvxGradientAxial::GeoTexSvxGradientAxial(
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::B2DRange& rOutputRange,
            sal_uInt32 nRequestedSteps,
            const basegfx::BColorStops& rColorStops,
            double fBorder,
            double fAngle)
        : GeoTexSvxGradient(rDefinitionRange, nRequestedSteps, rColorStops, fBorder)
        , mfUnitMinX(0.0)
        , mfUnitWidth(1.0)
        {
            // ARGH! GradientAxial uses the ColorStops in reverse order compared
            // with the other gradients. Either stay 'thinking reverse' for the
            // rest of time or adapt it here and go in same order as the other five,
            // so unifications/tooling will be possible
            mnColorStops.reverseColorStops();

            maGradientInfo = basegfx::utils::createAxialODFGradientInfo(
                rDefinitionRange,
                nRequestedSteps,
                fBorder,
                fAngle);

            if(rDefinitionRange != rOutputRange)
            {
                basegfx::B2DRange aInvOutputRange(rOutputRange);

                aInvOutputRange.transform(maGradientInfo.getBackTextureTransform());
                mfUnitMinX = aInvOutputRange.getMinX();
                mfUnitWidth = aInvOutputRange.getWidth();
            }
        }

        GeoTexSvxGradientAxial::~GeoTexSvxGradientAxial()
        {
        }

        void GeoTexSvxGradientAxial::appendTransformationsAndColors(
            std::function<void(const basegfx::B2DHomMatrix& rMatrix, const basegfx::BColor& rColor)> aCallback)
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // only one color, done
            if (mnColorStops.size() < 2)
                return;

            // check if we need last-ColorStop-correction
            const bool bPenultimateUsed(mnColorStops.checkPenultimate());

            if (bPenultimateUsed)
            {
                // temporarily add a ColorStop entry
                mnColorStops.emplace_back(1.0, mnColorStops.back().getStopColor());
            }

            // prepare unit range transform
            basegfx::B2DHomMatrix aPattern;

            // bring in X from unit circle [-1, -1, 1, 1] to unit range [0, 0, 1, 1]
            aPattern.scale(0.5, 1.0);
            aPattern.translate(0.5, 0.0);

            // scale/translate in X
            aPattern.scale(mfUnitWidth, 1.0);
            aPattern.translate(mfUnitMinX, 0.0);

            // outer loop over ColorStops, each is from cs_l to cs_r
            for (auto cs_l(mnColorStops.begin()), cs_r(cs_l + 1); cs_r != mnColorStops.end(); cs_l++, cs_r++)
            {
                // get offsets
                const double fOffsetStart(cs_l->getStopOffset());
                const double fOffsetEnd(cs_r->getStopOffset());

                // same offset, empty BColorStopRange, continue with next step
                if (basegfx::fTools::equal(fOffsetStart, fOffsetEnd))
                    continue;

                // get colors & calculate steps
                const basegfx::BColor aCStart(cs_l->getStopColor());
                const basegfx::BColor aCEnd(cs_r->getStopColor());
                const sal_uInt32 nSteps(basegfx::utils::calculateNumberOfSteps(
                    maGradientInfo.getRequestedSteps(), aCStart, aCEnd));

                // calculate StripeWidth
                // nSteps is >= 1, see getRequestedSteps, so no check needed here
                const double fStripeWidth((fOffsetEnd - fOffsetStart) / nSteps);

                // for the 1st color range we do not need to create the 1st step, see above
                const sal_uInt32 nStartInnerLoop(cs_l == mnColorStops.begin() ? 1 : 0);

                for (sal_uInt32 innerLoop(nStartInnerLoop); innerLoop < nSteps; innerLoop++)
                {
                    // calculate pos in Y
                    const double fPos(fOffsetStart + (fStripeWidth * innerLoop));

                    // already centered in Y on X-Axis, just scale in Y
                    basegfx::B2DHomMatrix aNew(aPattern);
                    aNew.scale(1.0, 1.0 - fPos);

                    // set and add at target
                    aCallback(
                        maGradientInfo.getTextureTransform() * aNew,
                        1 == nSteps ? aCStart : basegfx::BColor(interpolate(aCStart, aCEnd, double(innerLoop) / double(nSteps - 1))));
                }
            }

            if (bPenultimateUsed)
            {
                // correct temporary change
                mnColorStops.pop_back();
            }
        }

        void GeoTexSvxGradientAxial::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // just single color, done
            if (mnColorStops.size() < 2)
            {
                // we use the reverse ColorSteps here, so use front value
                rBColor = mnColorStops.front().getStopColor();
                return;
            }

            // texture-back-transform X/Y -> t [0.0..1.0] and determine color
            const double fScaler(basegfx::utils::getAxialGradientAlpha(rUV, maGradientInfo));

                // we use the reverse ColorSteps here, so mirror scaler value
            rBColor = mnColorStops.getInterpolatedBColor(1.0 - fScaler, mnRequestedSteps, maLastColorStopRange);
        }


        GeoTexSvxGradientRadial::GeoTexSvxGradientRadial(
            const basegfx::B2DRange& rDefinitionRange,
            sal_uInt32 nRequestedSteps,
            const basegfx::BColorStops& rColorStops,
            double fBorder,
            double fOffsetX,
            double fOffsetY)
        : GeoTexSvxGradient(rDefinitionRange, nRequestedSteps, rColorStops, fBorder)
        {
            maGradientInfo = basegfx::utils::createRadialODFGradientInfo(
                rDefinitionRange,
                basegfx::B2DVector(fOffsetX,fOffsetY),
                nRequestedSteps,
                fBorder);
        }

        GeoTexSvxGradientRadial::~GeoTexSvxGradientRadial()
        {
        }

        void GeoTexSvxGradientRadial::appendTransformationsAndColors(
            std::function<void(const basegfx::B2DHomMatrix& rMatrix, const basegfx::BColor& rColor)> aCallback)
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // only one color, done
            if (mnColorStops.size() < 2)
                return;

            // check if we need last-ColorStop-correction
            const bool bPenultimateUsed(mnColorStops.checkPenultimate());

            if (bPenultimateUsed)
            {
                // temporarily add a ColorStop entry
                mnColorStops.emplace_back(1.0, mnColorStops.back().getStopColor());
            }

            // outer loop over ColorStops, each is from cs_l to cs_r
            for (auto cs_l(mnColorStops.begin()), cs_r(cs_l + 1); cs_r != mnColorStops.end(); cs_l++, cs_r++)
            {
                // get offsets
                const double fOffsetStart(cs_l->getStopOffset());
                const double fOffsetEnd(cs_r->getStopOffset());

                // same offset, empty BColorStopRange, continue with next step
                if (basegfx::fTools::equal(fOffsetStart, fOffsetEnd))
                    continue;

                // get colors & calculate steps
                const basegfx::BColor aCStart(cs_l->getStopColor());
                const basegfx::BColor aCEnd(cs_r->getStopColor());
                const sal_uInt32 nSteps(basegfx::utils::calculateNumberOfSteps(
                    maGradientInfo.getRequestedSteps(), aCStart, aCEnd));

                // calculate StripeWidth
                const double fStripeWidth((fOffsetEnd - fOffsetStart) / nSteps);

                // get correct start for inner loop (see above)
                const sal_uInt32 nStartInnerLoop(cs_l == mnColorStops.begin() ? 1 : 0);

                for (sal_uInt32 innerLoop(nStartInnerLoop); innerLoop < nSteps; innerLoop++)
                {
                    // calculate size/radius
                    const double fSize(1.0 - (fOffsetStart + (fStripeWidth * innerLoop)));

                    // set and add at target
                    aCallback(
                        maGradientInfo.getTextureTransform() * basegfx::utils::createScaleB2DHomMatrix(fSize, fSize),
                        1 == nSteps ? aCStart : basegfx::BColor(interpolate(aCStart, aCEnd, double(innerLoop) / double(nSteps - 1))));
                }
            }

            if (bPenultimateUsed)
            {
                // correct temporary change
                mnColorStops.pop_back();
            }
        }

        void GeoTexSvxGradientRadial::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // just single color, done
            if (mnColorStops.size() < 2)
            {
                rBColor = mnColorStops.front().getStopColor();
                return;
            }

            // texture-back-transform X/Y -> t [0.0..1.0] and determine color
            const double fScaler(basegfx::utils::getRadialGradientAlpha(rUV, maGradientInfo));
            rBColor = mnColorStops.getInterpolatedBColor(fScaler, mnRequestedSteps, maLastColorStopRange);
        }


        GeoTexSvxGradientElliptical::GeoTexSvxGradientElliptical(
            const basegfx::B2DRange& rDefinitionRange,
            sal_uInt32 nRequestedSteps,
            const basegfx::BColorStops& rColorStops,
            double fBorder,
            double fOffsetX,
            double fOffsetY,
            double fAngle)
        : GeoTexSvxGradient(rDefinitionRange, nRequestedSteps, rColorStops, fBorder)
        {
            maGradientInfo = basegfx::utils::createEllipticalODFGradientInfo(
                rDefinitionRange,
                basegfx::B2DVector(fOffsetX,fOffsetY),
                nRequestedSteps,
                fBorder,
                fAngle);
        }

        GeoTexSvxGradientElliptical::~GeoTexSvxGradientElliptical()
        {
        }

        void GeoTexSvxGradientElliptical::appendTransformationsAndColors(
            std::function<void(const basegfx::B2DHomMatrix& rMatrix, const basegfx::BColor& rColor)> aCallback)
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // only one color, done
            if (mnColorStops.size() < 2)
                return;

            // check if we need last-ColorStop-correction
            const bool bPenultimateUsed(mnColorStops.checkPenultimate());

            if (bPenultimateUsed)
            {
                // temporarily add a ColorStop entry
                mnColorStops.emplace_back(1.0, mnColorStops.back().getStopColor());
            }

            // prepare vars dependent on aspect ratio
            const double fAR(maGradientInfo.getAspectRatio());
            const bool bMTO(fAR > 1.0);

            // outer loop over ColorStops, each is from cs_l to cs_r
            for (auto cs_l(mnColorStops.begin()), cs_r(cs_l + 1); cs_r != mnColorStops.end(); cs_l++, cs_r++)
            {
                // get offsets
                const double fOffsetStart(cs_l->getStopOffset());
                const double fOffsetEnd(cs_r->getStopOffset());

                // same offset, empty BColorStopRange, continue with next step
                if (basegfx::fTools::equal(fOffsetStart, fOffsetEnd))
                    continue;

                // get colors & calculate steps
                const basegfx::BColor aCStart(cs_l->getStopColor());
                const basegfx::BColor aCEnd(cs_r->getStopColor());
                const sal_uInt32 nSteps(basegfx::utils::calculateNumberOfSteps(
                    maGradientInfo.getRequestedSteps(), aCStart, aCEnd));

                // calculate StripeWidth
                const double fStripeWidth((fOffsetEnd - fOffsetStart) / nSteps);

                // get correct start for inner loop (see above)
                const sal_uInt32 nStartInnerLoop(cs_l == mnColorStops.begin() ? 1 : 0);

                for (sal_uInt32 innerLoop(nStartInnerLoop); innerLoop < nSteps; innerLoop++)
                {
                    // calculate offset position for entry
                    const double fSize(fOffsetStart + (fStripeWidth * innerLoop));

                    // set and add at target
                    aCallback(
                        maGradientInfo.getTextureTransform()
                        * basegfx::utils::createScaleB2DHomMatrix(
                            1.0 - (bMTO ? fSize / fAR : fSize),
                            1.0 - (bMTO ? fSize : fSize * fAR)),
                        1 == nSteps ? aCStart : basegfx::BColor(interpolate(aCStart, aCEnd, double(innerLoop) / double(nSteps - 1))));
                }
            }

            if (bPenultimateUsed)
            {
                // correct temporary change
                mnColorStops.pop_back();
            }
        }

        void GeoTexSvxGradientElliptical::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // just single color, done
            if (mnColorStops.size() < 2)
            {
                rBColor = mnColorStops.front().getStopColor();
                return;
            }

            // texture-back-transform X/Y -> t [0.0..1.0] and determine color
            const double fScaler(basegfx::utils::getEllipticalGradientAlpha(rUV, maGradientInfo));
            rBColor = mnColorStops.getInterpolatedBColor(fScaler, mnRequestedSteps, maLastColorStopRange);
        }


        GeoTexSvxGradientSquare::GeoTexSvxGradientSquare(
            const basegfx::B2DRange& rDefinitionRange,
            sal_uInt32 nRequestedSteps,
            const basegfx::BColorStops& rColorStops,
            double fBorder,
            double fOffsetX,
            double fOffsetY,
            double fAngle)
        : GeoTexSvxGradient(rDefinitionRange, nRequestedSteps, rColorStops, fBorder)
        {
            maGradientInfo = basegfx::utils::createSquareODFGradientInfo(
                rDefinitionRange,
                basegfx::B2DVector(fOffsetX,fOffsetY),
                nRequestedSteps,
                fBorder,
                fAngle);
        }

        GeoTexSvxGradientSquare::~GeoTexSvxGradientSquare()
        {
        }

        void GeoTexSvxGradientSquare::appendTransformationsAndColors(
            std::function<void(const basegfx::B2DHomMatrix& rMatrix, const basegfx::BColor& rColor)> aCallback)
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // only one color, done
            if (mnColorStops.size() < 2)
                return;

            // check if we need last-ColorStop-correction
            const bool bPenultimateUsed(mnColorStops.checkPenultimate());

            if (bPenultimateUsed)
            {
                // temporarily add a ColorStop entry
                mnColorStops.emplace_back(1.0, mnColorStops.back().getStopColor());
            }

            // outer loop over ColorStops, each is from cs_l to cs_r
            for (auto cs_l(mnColorStops.begin()), cs_r(cs_l + 1); cs_r != mnColorStops.end(); cs_l++, cs_r++)
            {
                // get offsets
                const double fOffsetStart(cs_l->getStopOffset());
                const double fOffsetEnd(cs_r->getStopOffset());

                // same offset, empty BColorStopRange, continue with next step
                if (basegfx::fTools::equal(fOffsetStart, fOffsetEnd))
                    continue;

                // get colors & calculate steps
                const basegfx::BColor aCStart(cs_l->getStopColor());
                const basegfx::BColor aCEnd(cs_r->getStopColor());
                const sal_uInt32 nSteps(basegfx::utils::calculateNumberOfSteps(
                    maGradientInfo.getRequestedSteps(), aCStart, aCEnd));

                // calculate StripeWidth
                const double fStripeWidth((fOffsetEnd - fOffsetStart) / nSteps);

                // get correct start for inner loop (see above)
                const sal_uInt32 nStartInnerLoop(cs_l == mnColorStops.begin() ? 1 : 0);

                for (sal_uInt32 innerLoop(nStartInnerLoop); innerLoop < nSteps; innerLoop++)
                {
                    // calculate size/radius
                    const double fSize(1.0 - (fOffsetStart + (fStripeWidth * innerLoop)));

                    // set and add at target
                    aCallback(
                        maGradientInfo.getTextureTransform() * basegfx::utils::createScaleB2DHomMatrix(fSize, fSize),
                        1 == nSteps ? aCStart : basegfx::BColor(interpolate(aCStart, aCEnd, double(innerLoop) / double(nSteps - 1))));
                }
            }

            if (bPenultimateUsed)
            {
                // correct temporary change
                mnColorStops.pop_back();
            }
        }

        void GeoTexSvxGradientSquare::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // just single color, done
            if (mnColorStops.size() < 2)
            {
                rBColor = mnColorStops.front().getStopColor();
                return;
            }

            // texture-back-transform X/Y -> t [0.0..1.0] and determine color
            const double fScaler(basegfx::utils::getSquareGradientAlpha(rUV, maGradientInfo));
            rBColor = mnColorStops.getInterpolatedBColor(fScaler, mnRequestedSteps, maLastColorStopRange);
        }


        GeoTexSvxGradientRect::GeoTexSvxGradientRect(
            const basegfx::B2DRange& rDefinitionRange,
            sal_uInt32 nRequestedSteps,
            const basegfx::BColorStops& rColorStops,
            double fBorder,
            double fOffsetX,
            double fOffsetY,
            double fAngle)
        : GeoTexSvxGradient(rDefinitionRange, nRequestedSteps, rColorStops, fBorder)
        {
            maGradientInfo = basegfx::utils::createRectangularODFGradientInfo(
                rDefinitionRange,
                basegfx::B2DVector(fOffsetX,fOffsetY),
                nRequestedSteps,
                fBorder,
                fAngle);
        }

        GeoTexSvxGradientRect::~GeoTexSvxGradientRect()
        {
        }

        void GeoTexSvxGradientRect::appendTransformationsAndColors(
            std::function<void(const basegfx::B2DHomMatrix& rMatrix, const basegfx::BColor& rColor)> aCallback)
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // only one color, done
            if (mnColorStops.size() < 2)
                return;

            // check if we need last-ColorStop-correction
            const bool bPenultimateUsed(mnColorStops.checkPenultimate());

            if (bPenultimateUsed)
            {
                // temporarily add a ColorStop entry
                mnColorStops.emplace_back(1.0, mnColorStops.back().getStopColor());
            }

            // prepare vars dependent on aspect ratio
            const double fAR(maGradientInfo.getAspectRatio());
            const bool bMTO(fAR > 1.0);

            // outer loop over ColorStops, each is from cs_l to cs_r
            for (auto cs_l(mnColorStops.begin()), cs_r(cs_l + 1); cs_r != mnColorStops.end(); cs_l++, cs_r++)
            {
                // get offsets
                const double fOffsetStart(cs_l->getStopOffset());
                const double fOffsetEnd(cs_r->getStopOffset());

                // same offset, empty BColorStopRange, continue with next step
                if (basegfx::fTools::equal(fOffsetStart, fOffsetEnd))
                    continue;

                // get colors & calculate steps
                const basegfx::BColor aCStart(cs_l->getStopColor());
                const basegfx::BColor aCEnd(cs_r->getStopColor());
                const sal_uInt32 nSteps(basegfx::utils::calculateNumberOfSteps(
                    maGradientInfo.getRequestedSteps(), aCStart, aCEnd));

                // calculate StripeWidth
                const double fStripeWidth((fOffsetEnd - fOffsetStart) / nSteps);

                // get correct start for inner loop (see above)
                const sal_uInt32 nStartInnerLoop(cs_l == mnColorStops.begin() ? 1 : 0);

                for (sal_uInt32 innerLoop(nStartInnerLoop); innerLoop < nSteps; innerLoop++)
                {
                    // calculate offset position for entry
                    const double fSize(fOffsetStart + (fStripeWidth * innerLoop));

                    // set and add at target
                    aCallback(
                        maGradientInfo.getTextureTransform()
                        * basegfx::utils::createScaleB2DHomMatrix(
                            1.0 - (bMTO ? fSize / fAR : fSize),
                            1.0 - (bMTO ? fSize : fSize * fAR)),
                        1 == nSteps ? aCStart : basegfx::BColor(interpolate(aCStart, aCEnd, double(innerLoop) / double(nSteps - 1))));
                }
            }

            if (bPenultimateUsed)
            {
                // correct temporary change
                mnColorStops.pop_back();
            }
        }

        void GeoTexSvxGradientRect::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            // no color at all, done
            if (mnColorStops.empty())
                return;

            // just single color, done
            if (mnColorStops.size() < 2)
            {
                rBColor = mnColorStops.front().getStopColor();
                return;
            }

            // texture-back-transform X/Y -> t [0.0..1.0] and determine color
            const double fScaler(basegfx::utils::getRectangularGradientAlpha(rUV, maGradientInfo));
            rBColor = mnColorStops.getInterpolatedBColor(fScaler, mnRequestedSteps, maLastColorStopRange);
        }


        GeoTexSvxHatch::GeoTexSvxHatch(
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::B2DRange& rOutputRange,
            double fDistance,
            double fAngle)
        :   maOutputRange(rOutputRange),
            mfDistance(0.1),
            mfAngle(fAngle),
            mnSteps(10),
            mbDefinitionRangeEqualsOutputRange(rDefinitionRange == rOutputRange)
        {
            double fTargetSizeX(rDefinitionRange.getWidth());
            double fTargetSizeY(rDefinitionRange.getHeight());
            double fTargetOffsetX(rDefinitionRange.getMinX());
            double fTargetOffsetY(rDefinitionRange.getMinY());

            fAngle = -fAngle;

            // add object expansion
            if(0.0 != fAngle)
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

            // add object scale before rotate
            maTextureTransform.scale(fTargetSizeX, fTargetSizeY);

            // add texture rotate after scale to keep perpendicular angles
            if(0.0 != fAngle)
            {
                basegfx::B2DPoint aCenter(0.5, 0.5);
                aCenter *= maTextureTransform;

                maTextureTransform = basegfx::utils::createRotateAroundPoint(aCenter, fAngle)
                    * maTextureTransform;
            }

            // add object translate
            maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

            // prepare height for texture
            const double fSteps((0.0 != fDistance) ? fTargetSizeY / fDistance : 10.0);
            mnSteps = basegfx::fround(fSteps + 0.5);
            mfDistance = 1.0 / fSteps;
        }

        GeoTexSvxHatch::~GeoTexSvxHatch()
        {
        }

        bool GeoTexSvxHatch::operator==(const GeoTexSvx& rGeoTexSvx) const
        {
            const GeoTexSvxHatch* pCompare = dynamic_cast< const GeoTexSvxHatch* >(&rGeoTexSvx);
            return (pCompare
                && maOutputRange == pCompare->maOutputRange
                && maTextureTransform == pCompare->maTextureTransform
                && mfDistance == pCompare->mfDistance
                && mfAngle == pCompare->mfAngle
                && mnSteps == pCompare->mnSteps);
        }

        void GeoTexSvxHatch::appendTransformations(std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(mbDefinitionRangeEqualsOutputRange)
            {
                // simple hatch where the definition area equals the output area
                for(sal_uInt32 a(1); a < mnSteps; a++)
                {
                    // create matrix
                    const double fOffset(mfDistance * static_cast<double>(a));
                    basegfx::B2DHomMatrix aNew;
                    aNew.set(1, 2, fOffset);
                    rMatrices.push_back(maTextureTransform * aNew);
                }
            }
            else
            {
                // output area is different from definition area, back-transform to get
                // the output area in unit coordinates and fill this with hatch lines
                // using the settings derived from the definition area
                basegfx::B2DRange aBackUnitRange(maOutputRange);

                aBackUnitRange.transform(getBackTextureTransform());

                // calculate vertical start value and a security maximum integer value to avoid death loops
                double fStart(basegfx::snapToNearestMultiple(aBackUnitRange.getMinY(), mfDistance));
                const sal_uInt32 nNeededIntegerSteps(basegfx::fround((aBackUnitRange.getHeight() / mfDistance) + 0.5));
                sal_uInt32 nMaxIntegerSteps(std::min(nNeededIntegerSteps, sal_uInt32(10000)));

                while(fStart < aBackUnitRange.getMaxY() && nMaxIntegerSteps)
                {
                    // create new transform for
                    basegfx::B2DHomMatrix aNew;

                    // adapt x scale and position
                    //aNew.scale(aBackUnitRange.getWidth(), 1.0);
                    //aNew.translate(aBackUnitRange.getMinX(), 0.0);
                    aNew.set(0, 0, aBackUnitRange.getWidth());
                    aNew.set(0, 2, aBackUnitRange.getMinX());

                    // adapt y position to current step
                    aNew.set(1, 2, fStart);
                    //aNew.translate(0.0, fStart);

                    // add new transformation
                    rMatrices.push_back(maTextureTransform * aNew);

                    // next step
                    fStart += mfDistance;
                    nMaxIntegerSteps--;
                }
            }
        }

        double GeoTexSvxHatch::getDistanceToHatch(const basegfx::B2DPoint& rUV) const
        {
            // the below is an inlined and optimised version of
            //     const basegfx::B2DPoint aCoor(getBackTextureTransform() * rUV);
            //     return fmod(aCoor.getY(), mfDistance);

            const basegfx::B2DHomMatrix& rMat = getBackTextureTransform();
            double fX = rUV.getX();
            double fY = rUV.getY();

            double fTempY(
                rMat.get(1, 0) * fX +
                rMat.get(1, 1) * fY +
                rMat.get(1, 2));

            return fmod(fTempY, mfDistance);
        }

        const basegfx::B2DHomMatrix& GeoTexSvxHatch::getBackTextureTransform() const
        {
            if(maBackTextureTransform.isIdentity())
            {
                const_cast< GeoTexSvxHatch* >(this)->maBackTextureTransform = maTextureTransform;
                const_cast< GeoTexSvxHatch* >(this)->maBackTextureTransform.invert();
            }

            return maBackTextureTransform;
        }


        GeoTexSvxTiled::GeoTexSvxTiled(
            const basegfx::B2DRange& rRange,
            double fOffsetX,
            double fOffsetY)
        :   maRange(rRange),
            mfOffsetX(std::clamp(fOffsetX, 0.0, 1.0)),
            mfOffsetY(std::clamp(fOffsetY, 0.0, 1.0))
        {
            if(!basegfx::fTools::equalZero(mfOffsetX))
            {
                mfOffsetY = 0.0;
            }
        }

        GeoTexSvxTiled::~GeoTexSvxTiled()
        {
        }

        bool GeoTexSvxTiled::operator==(const GeoTexSvx& rGeoTexSvx) const
        {
            const GeoTexSvxTiled* pCompare = dynamic_cast< const GeoTexSvxTiled* >(&rGeoTexSvx);

            return (pCompare
                && maRange == pCompare->maRange
                && mfOffsetX == pCompare->mfOffsetX
                && mfOffsetY == pCompare->mfOffsetY);
        }

        sal_uInt32 GeoTexSvxTiled::getNumberOfTiles() const
        {
            sal_Int32 nTiles = 0;
            iterateTiles([&](double, double) { ++nTiles; });
            return nTiles;
        }

        void GeoTexSvxTiled::appendTransformations(std::vector< basegfx::B2DHomMatrix >& rMatrices) const
        {
            const double fWidth(maRange.getWidth());
            const double fHeight(maRange.getHeight());
            iterateTiles([&](double fPosX, double fPosY) {
                rMatrices.push_back(basegfx::utils::createScaleTranslateB2DHomMatrix(
                                        fWidth,
                                        fHeight,
                                        fPosX,
                                        fPosY));
                });
        }

        void GeoTexSvxTiled::iterateTiles(std::function<void(double fPosX, double fPosY)> aFunc) const
        {
            const double fWidth(maRange.getWidth());

            if(basegfx::fTools::equalZero(fWidth))
                return;

            const double fHeight(maRange.getHeight());

            if(basegfx::fTools::equalZero(fHeight))
                return;

            double fStartX(maRange.getMinX());
            double fStartY(maRange.getMinY());
            sal_Int32 nPosX(0);
            sal_Int32 nPosY(0);

            if(fStartX > 0.0)
            {
                const sal_Int32 nDiff(static_cast<sal_Int32>(floor(fStartX / fWidth)) + 1);

                nPosX -= nDiff;
                fStartX -= nDiff * fWidth;
            }

            if((fStartX + fWidth) < 0.0)
            {
                const sal_Int32 nDiff(static_cast<sal_Int32>(floor(-fStartX / fWidth)));

                nPosX += nDiff;
                fStartX += nDiff * fWidth;
            }

            if(fStartY > 0.0)
            {
                const sal_Int32 nDiff(static_cast<sal_Int32>(floor(fStartY / fHeight)) + 1);

                nPosY -= nDiff;
                fStartY -= nDiff * fHeight;
            }

            if((fStartY + fHeight) < 0.0)
            {
                const sal_Int32 nDiff(static_cast<sal_Int32>(floor(-fStartY / fHeight)));

                nPosY += nDiff;
                fStartY += nDiff * fHeight;
            }

            if(!basegfx::fTools::equalZero(mfOffsetY))
            {
                for(double fPosX(fStartX); basegfx::fTools::less(fPosX, 1.0); fPosX += fWidth, nPosX++)
                {
                    for(double fPosY((nPosX % 2) ? fStartY - fHeight + (mfOffsetY * fHeight) : fStartY);
                        basegfx::fTools::less(fPosY, 1.0); fPosY += fHeight)
                        aFunc(fPosX, fPosY);
                }
            }
            else
            {
                for(double fPosY(fStartY); basegfx::fTools::less(fPosY, 1.0); fPosY += fHeight, nPosY++)
                {
                    for(double fPosX((nPosY % 2) ? fStartX - fWidth + (mfOffsetX * fWidth) : fStartX);
                        basegfx::fTools::less(fPosX, 1.0); fPosX += fWidth)
                        aFunc(fPosX, fPosY);
                }
            }

        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
