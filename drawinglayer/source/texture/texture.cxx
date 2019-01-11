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

#include <drawinglayer/texture/texture.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <converters.hxx>

namespace drawinglayer
{
    namespace texture
    {
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
    } // end of namespace texture
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradient::GeoTexSvxGradient(
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::BColor& rStart,
            const basegfx::BColor& rEnd,
            double fBorder)
        :   GeoTexSvx(),
            maGradientInfo(),
            maDefinitionRange(rDefinitionRange),
            maStart(rStart),
            maEnd(rEnd),
            mfBorder(fBorder)
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
                && mfBorder == pCompare->mfBorder);
        }
    } // end of namespace texture
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientLinear::GeoTexSvxGradientLinear(
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::B2DRange& rOutputRange,
            const basegfx::BColor& rStart,
            const basegfx::BColor& rEnd,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        :   GeoTexSvxGradient(rDefinitionRange, rStart, rEnd, fBorder),
            mfUnitMinX(0.0),
            mfUnitWidth(1.0),
            mfUnitMaxY(1.0)
        {
            maGradientInfo = basegfx::utils::createLinearODFGradientInfo(
                rDefinitionRange,
                nSteps,
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
            std::vector< B2DHomMatrixAndBColor >& rEntries,
            basegfx::BColor& rOuterColor)
        {
            rOuterColor = maStart;

            if(maGradientInfo.getSteps())
            {
                const double fStripeWidth(1.0 / maGradientInfo.getSteps());
                B2DHomMatrixAndBColor aB2DHomMatrixAndBColor;
                basegfx::B2DHomMatrix aPattern;

                // bring from unit circle [-1, -1, 1, 1] to unit range [0, 0, 1, 1]
                aPattern.scale(0.5, 0.5);
                aPattern.translate(0.5, 0.5);

                // scale and translate in X
                aPattern.scale(mfUnitWidth, 1.0);
                aPattern.translate(mfUnitMinX, 0.0);

                for(sal_uInt32 a(1); a < maGradientInfo.getSteps(); a++)
                {
                    const double fPos(fStripeWidth * a);
                    basegfx::B2DHomMatrix aNew(aPattern);

                    // scale and translate in Y
                    double fHeight(1.0 - fPos);

                    if(a + 1 == maGradientInfo.getSteps() && mfUnitMaxY > 1.0)
                    {
                        fHeight += mfUnitMaxY - 1.0;
                    }

                    aNew.scale(1.0, fHeight);
                    aNew.translate(0.0, fPos);

                    // set at target
                    aB2DHomMatrixAndBColor.maB2DHomMatrix = maGradientInfo.getTextureTransform() * aNew;

                    // interpolate and set color
                    aB2DHomMatrixAndBColor.maBColor = interpolate(maStart, maEnd, double(a) / double(maGradientInfo.getSteps() - 1));

                    rEntries.push_back(aB2DHomMatrixAndBColor);
                }
            }
        }

        void GeoTexSvxGradientLinear::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::utils::getLinearGradientAlpha(rUV, maGradientInfo));

            rBColor = basegfx::interpolate(maStart, maEnd, fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientAxial::GeoTexSvxGradientAxial(
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::B2DRange& rOutputRange,
            const basegfx::BColor& rStart,
            const basegfx::BColor& rEnd,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        :   GeoTexSvxGradient(rDefinitionRange, rStart, rEnd, fBorder),
            mfUnitMinX(0.0),
            mfUnitWidth(1.0)
        {
            maGradientInfo = basegfx::utils::createAxialODFGradientInfo(
                rDefinitionRange,
                nSteps,
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
            std::vector< B2DHomMatrixAndBColor >& rEntries,
            basegfx::BColor& rOuterColor)
        {
            rOuterColor = maEnd;

            if(maGradientInfo.getSteps())
            {
                const double fStripeWidth(1.0 / maGradientInfo.getSteps());
                B2DHomMatrixAndBColor aB2DHomMatrixAndBColor;

                for(sal_uInt32 a(1); a < maGradientInfo.getSteps(); a++)
                {
                    const double fPos(fStripeWidth * a);
                    basegfx::B2DHomMatrix aNew;

                    // bring in X from unit circle [-1, -1, 1, 1] to unit range [0, 0, 1, 1]
                    aNew.scale(0.5, 1.0);
                    aNew.translate(0.5, 0.0);

                    // scale/translate in X
                    aNew.scale(mfUnitWidth, 1.0);
                    aNew.translate(mfUnitMinX, 0.0);

                    // already centered in Y on X-Axis, just scale in Y
                    aNew.scale(1.0, 1.0 - fPos);

                    // set at target
                    aB2DHomMatrixAndBColor.maB2DHomMatrix = maGradientInfo.getTextureTransform() * aNew;

                    // interpolate and set color
                    aB2DHomMatrixAndBColor.maBColor = interpolate(maEnd, maStart, double(a) / double(maGradientInfo.getSteps() - 1));

                    rEntries.push_back(aB2DHomMatrixAndBColor);
                }
            }
        }

        void GeoTexSvxGradientAxial::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::utils::getAxialGradientAlpha(rUV, maGradientInfo));

            rBColor = basegfx::interpolate(maStart, maEnd, fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientRadial::GeoTexSvxGradientRadial(
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::BColor& rStart,
            const basegfx::BColor& rEnd,
            sal_uInt32 nSteps,
            double fBorder,
            double fOffsetX,
            double fOffsetY)
        :   GeoTexSvxGradient(rDefinitionRange, rStart, rEnd, fBorder)
        {
            maGradientInfo = basegfx::utils::createRadialODFGradientInfo(
                rDefinitionRange,
                basegfx::B2DVector(fOffsetX,fOffsetY),
                nSteps,
                fBorder);
        }

        GeoTexSvxGradientRadial::~GeoTexSvxGradientRadial()
        {
        }

        void GeoTexSvxGradientRadial::appendTransformationsAndColors(
            std::vector< B2DHomMatrixAndBColor >& rEntries,
            basegfx::BColor& rOuterColor)
        {
            rOuterColor = maStart;

            if(maGradientInfo.getSteps())
            {
                const double fStepSize(1.0 / maGradientInfo.getSteps());
                B2DHomMatrixAndBColor aB2DHomMatrixAndBColor;

                for(sal_uInt32 a(1); a < maGradientInfo.getSteps(); a++)
                {
                    const double fSize(1.0 - (fStepSize * a));
                    aB2DHomMatrixAndBColor.maB2DHomMatrix = maGradientInfo.getTextureTransform() * basegfx::utils::createScaleB2DHomMatrix(fSize, fSize);
                    aB2DHomMatrixAndBColor.maBColor = interpolate(maStart, maEnd, double(a) / double(maGradientInfo.getSteps() - 1));
                    rEntries.push_back(aB2DHomMatrixAndBColor);
                }
            }
        }

        void GeoTexSvxGradientRadial::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::utils::getRadialGradientAlpha(rUV, maGradientInfo));

            rBColor = basegfx::interpolate(maStart, maEnd, fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientElliptical::GeoTexSvxGradientElliptical(
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::BColor& rStart,
            const basegfx::BColor& rEnd,
            sal_uInt32 nSteps,
            double fBorder,
            double fOffsetX,
            double fOffsetY,
            double fAngle)
        :   GeoTexSvxGradient(rDefinitionRange, rStart, rEnd, fBorder)
        {
            maGradientInfo = basegfx::utils::createEllipticalODFGradientInfo(
                rDefinitionRange,
                basegfx::B2DVector(fOffsetX,fOffsetY),
                nSteps,
                fBorder,
                fAngle);
        }

        GeoTexSvxGradientElliptical::~GeoTexSvxGradientElliptical()
        {
        }

        void GeoTexSvxGradientElliptical::appendTransformationsAndColors(
            std::vector< B2DHomMatrixAndBColor >& rEntries,
            basegfx::BColor& rOuterColor)
        {
            rOuterColor = maStart;

            if(maGradientInfo.getSteps())
            {
                double fWidth(1.0);
                double fHeight(1.0);
                double fIncrementX(0.0);
                double fIncrementY(0.0);

                if(maGradientInfo.getAspectRatio() > 1.0)
                {
                    fIncrementY = fHeight / maGradientInfo.getSteps();
                    fIncrementX = fIncrementY / maGradientInfo.getAspectRatio();
                }
                else
                {
                    fIncrementX = fWidth / maGradientInfo.getSteps();
                    fIncrementY = fIncrementX * maGradientInfo.getAspectRatio();
                }

                B2DHomMatrixAndBColor aB2DHomMatrixAndBColor;

                for(sal_uInt32 a(1); a < maGradientInfo.getSteps(); a++)
                {
                    // next step
                    fWidth -= fIncrementX;
                    fHeight -= fIncrementY;

                    aB2DHomMatrixAndBColor.maB2DHomMatrix = maGradientInfo.getTextureTransform() * basegfx::utils::createScaleB2DHomMatrix(fWidth, fHeight);
                    aB2DHomMatrixAndBColor.maBColor = interpolate(maStart, maEnd, double(a) / double(maGradientInfo.getSteps() - 1));
                    rEntries.push_back(aB2DHomMatrixAndBColor);
                }
            }
        }

        void GeoTexSvxGradientElliptical::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::utils::getEllipticalGradientAlpha(rUV, maGradientInfo));

            rBColor = basegfx::interpolate(maStart, maEnd, fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientSquare::GeoTexSvxGradientSquare(
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::BColor& rStart,
            const basegfx::BColor& rEnd,
            sal_uInt32 nSteps,
            double fBorder,
            double fOffsetX,
            double fOffsetY,
            double fAngle)
        :   GeoTexSvxGradient(rDefinitionRange, rStart, rEnd, fBorder)
        {
            maGradientInfo = basegfx::utils::createSquareODFGradientInfo(
                rDefinitionRange,
                basegfx::B2DVector(fOffsetX,fOffsetY),
                nSteps,
                fBorder,
                fAngle);
        }

        GeoTexSvxGradientSquare::~GeoTexSvxGradientSquare()
        {
        }

        void GeoTexSvxGradientSquare::appendTransformationsAndColors(
            std::vector< B2DHomMatrixAndBColor >& rEntries,
            basegfx::BColor& rOuterColor)
        {
            rOuterColor = maStart;

            if(maGradientInfo.getSteps())
            {
                const double fStepSize(1.0 / maGradientInfo.getSteps());
                B2DHomMatrixAndBColor aB2DHomMatrixAndBColor;

                for(sal_uInt32 a(1); a < maGradientInfo.getSteps(); a++)
                {
                    const double fSize(1.0 - (fStepSize * a));
                    aB2DHomMatrixAndBColor.maB2DHomMatrix = maGradientInfo.getTextureTransform() * basegfx::utils::createScaleB2DHomMatrix(fSize, fSize);
                    aB2DHomMatrixAndBColor.maBColor = interpolate(maStart, maEnd, double(a) / double(maGradientInfo.getSteps() - 1));
                    rEntries.push_back(aB2DHomMatrixAndBColor);
                }
            }
        }

        void GeoTexSvxGradientSquare::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::utils::getSquareGradientAlpha(rUV, maGradientInfo));

            rBColor = basegfx::interpolate(maStart, maEnd, fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientRect::GeoTexSvxGradientRect(
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::BColor& rStart,
            const basegfx::BColor& rEnd,
            sal_uInt32 nSteps,
            double fBorder,
            double fOffsetX,
            double fOffsetY,
            double fAngle)
        :   GeoTexSvxGradient(rDefinitionRange, rStart, rEnd, fBorder)
        {
            maGradientInfo = basegfx::utils::createRectangularODFGradientInfo(
                rDefinitionRange,
                basegfx::B2DVector(fOffsetX,fOffsetY),
                nSteps,
                fBorder,
                fAngle);
        }

        GeoTexSvxGradientRect::~GeoTexSvxGradientRect()
        {
        }

        void GeoTexSvxGradientRect::appendTransformationsAndColors(
            std::vector< B2DHomMatrixAndBColor >& rEntries,
            basegfx::BColor& rOuterColor)
        {
            rOuterColor = maStart;

            if(maGradientInfo.getSteps())
            {
                double fWidth(1.0);
                double fHeight(1.0);
                double fIncrementX(0.0);
                double fIncrementY(0.0);

                if(maGradientInfo.getAspectRatio() > 1.0)
                {
                    fIncrementY = fHeight / maGradientInfo.getSteps();
                    fIncrementX = fIncrementY / maGradientInfo.getAspectRatio();
                }
                else
                {
                    fIncrementX = fWidth / maGradientInfo.getSteps();
                    fIncrementY = fIncrementX * maGradientInfo.getAspectRatio();
                }

                B2DHomMatrixAndBColor aB2DHomMatrixAndBColor;

                for(sal_uInt32 a(1); a < maGradientInfo.getSteps(); a++)
                {
                    // next step
                    fWidth -= fIncrementX;
                    fHeight -= fIncrementY;

                    aB2DHomMatrixAndBColor.maB2DHomMatrix = maGradientInfo.getTextureTransform() * basegfx::utils::createScaleB2DHomMatrix(fWidth, fHeight);
                    aB2DHomMatrixAndBColor.maBColor = interpolate(maStart, maEnd, double(a) / double(maGradientInfo.getSteps() - 1));
                    rEntries.push_back(aB2DHomMatrixAndBColor);
                }
            }
        }

        void GeoTexSvxGradientRect::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::utils::getRectangularGradientAlpha(rUV, maGradientInfo));

            rBColor = basegfx::interpolate(maStart, maEnd, fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxHatch::GeoTexSvxHatch(
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::B2DRange& rOutputRange,
            double fDistance,
            double fAngle)
        :   maOutputRange(rOutputRange),
            maTextureTransform(),
            maBackTextureTransform(),
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
            const basegfx::B2DPoint aCoor(getBackTextureTransform() * rUV);
            return fmod(aCoor.getY(), mfDistance);
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
    } // end of namespace texture
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace texture
    {
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
            return iterateTiles(nullptr);
        }

        void GeoTexSvxTiled::appendTransformations(std::vector< basegfx::B2DHomMatrix >& rMatrices) const
        {
            iterateTiles(&rMatrices);
        }

        sal_Int32 GeoTexSvxTiled::iterateTiles(std::vector< basegfx::B2DHomMatrix >* pMatrices) const
        {
            const double fWidth(maRange.getWidth());
            sal_Int32 nTiles = 0;

            if(!basegfx::fTools::equalZero(fWidth))
            {
                const double fHeight(maRange.getHeight());

                if(!basegfx::fTools::equalZero(fHeight))
                {
                    double fStartX(maRange.getMinX());
                    double fStartY(maRange.getMinY());
                    sal_Int32 nPosX(0);
                    sal_Int32 nPosY(0);

                    if(basegfx::fTools::more(fStartX, 0.0))
                    {
                        const sal_Int32 nDiff(static_cast<sal_Int32>(floor(fStartX / fWidth)) + 1);

                        nPosX -= nDiff;
                        fStartX -= nDiff * fWidth;
                    }

                    if(basegfx::fTools::less(fStartX + fWidth, 0.0))
                    {
                        const sal_Int32 nDiff(static_cast<sal_Int32>(floor(-fStartX / fWidth)));

                        nPosX += nDiff;
                        fStartX += nDiff * fWidth;
                    }

                    if(basegfx::fTools::more(fStartY, 0.0))
                    {
                        const sal_Int32 nDiff(static_cast<sal_Int32>(floor(fStartY / fHeight)) + 1);

                        nPosY -= nDiff;
                        fStartY -= nDiff * fHeight;
                    }

                    if(basegfx::fTools::less(fStartY + fHeight, 0.0))
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
                            {
                                if(pMatrices)
                                {
                                    pMatrices->push_back(
                                        basegfx::utils::createScaleTranslateB2DHomMatrix(
                                            fWidth,
                                            fHeight,
                                            fPosX,
                                            fPosY));
                                }
                                else
                                {
                                    nTiles++;
                                }
                            }
                        }
                    }
                    else
                    {
                        for(double fPosY(fStartY); basegfx::fTools::less(fPosY, 1.0); fPosY += fHeight, nPosY++)
                        {
                            for(double fPosX((nPosY % 2) ? fStartX - fWidth + (mfOffsetX * fWidth) : fStartX);
                                basegfx::fTools::less(fPosX, 1.0); fPosX += fWidth)
                            {
                                if(pMatrices)
                                {
                                    pMatrices->push_back(
                                        basegfx::utils::createScaleTranslateB2DHomMatrix(
                                            fWidth,
                                            fHeight,
                                            fPosX,
                                            fPosY));
                                }
                                else
                                {
                                    nTiles++;
                                }
                            }
                        }
                    }
                }
            }

            return nTiles;
        }
    } // end of namespace texture
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
