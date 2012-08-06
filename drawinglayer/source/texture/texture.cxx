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

#include <drawinglayer/texture/texture.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/tools/gradienttools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

//////////////////////////////////////////////////////////////////////////////

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

        void GeoTexSvx::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& /*rMatrices*/)
        {
            // default implementation does nothing
        }

        void GeoTexSvx::modifyBColor(const basegfx::B2DPoint& /*rUV*/, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            // base implementation creates random color (for testing only, may also be pure virtual)
            rBColor.setRed((rand() & 0x7fff) / 32767.0);
            rBColor.setGreen((rand() & 0x7fff) / 32767.0);
            rBColor.setBlue((rand() & 0x7fff) / 32767.0);
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

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        void GeoTexSvxGradient::impAppendMatrix(::std::vector< basegfx::B2DHomMatrix >& rMatrices, const basegfx::B2DRange& rRange)
        {
            basegfx::B2DHomMatrix aNew;
            aNew.set(0, 0, rRange.getWidth());
            aNew.set(1, 1, rRange.getHeight());
            aNew.set(0, 2, rRange.getMinX());
            aNew.set(1, 2, rRange.getMinY());
            rMatrices.push_back(maGradientInfo.maTextureTransform * aNew);
        }

        void GeoTexSvxGradient::impAppendColorsRadial(::std::vector< basegfx::BColor >& rColors)
        {
            if(maGradientInfo.mnSteps)
            {
                rColors.push_back(maStart);

                for(sal_uInt32 a(1L); a < maGradientInfo.mnSteps - 1L; a++)
                {
                    rColors.push_back(interpolate(maStart, maEnd, (double)a / (double)maGradientInfo.mnSteps));
                }

                rColors.push_back(maEnd);
            }
        }

        GeoTexSvxGradient::GeoTexSvxGradient(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder)
        :   maTargetRange(rTargetRange),
            maStart(rStart),
            maEnd(rEnd),
            mfBorder(fBorder)
        {
            maGradientInfo.mnSteps = nSteps;
            maGradientInfo.mfAspectRatio = 1.0;
        }

        GeoTexSvxGradient::~GeoTexSvxGradient()
        {
        }

        bool GeoTexSvxGradient::operator==(const GeoTexSvx& rGeoTexSvx) const
        {
            const GeoTexSvxGradient* pCompare = dynamic_cast< const GeoTexSvxGradient* >(&rGeoTexSvx);
            return (pCompare
                && maGradientInfo.maTextureTransform == pCompare->maGradientInfo.maTextureTransform
                && maTargetRange == pCompare->maTargetRange
                && maGradientInfo.mnSteps == pCompare->maGradientInfo.mnSteps
                && maGradientInfo.mfAspectRatio == pCompare->maGradientInfo.mfAspectRatio
                && mfBorder == pCompare->mfBorder);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientLinear::GeoTexSvxGradientLinear(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fAngle)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            basegfx::tools::createLinearODFGradientInfo(maGradientInfo,
                                                        rTargetRange,
                                                        nSteps,
                                                        fBorder,
                                                        fAngle);
        }

        GeoTexSvxGradientLinear::~GeoTexSvxGradientLinear()
        {
        }

        void GeoTexSvxGradientLinear::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(maGradientInfo.mnSteps)
            {
                const double fStripeWidth(1.0 / maGradientInfo.mnSteps);
                for(sal_uInt32 a(1L); a < maGradientInfo.mnSteps; a++)
                {
                    const basegfx::B2DRange aRect(0.0, fStripeWidth * a, 1.0, 1.0);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientLinear::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            if(maGradientInfo.mnSteps)
            {
                rColors.push_back(maStart);

                for(sal_uInt32 a(1L); a < maGradientInfo.mnSteps; a++)
                {
                    rColors.push_back(interpolate(maStart, maEnd, (double)a / (double)(maGradientInfo.mnSteps + 1L)));
                }
            }
        }

        void GeoTexSvxGradientLinear::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::tools::getLinearGradientAlpha(rUV, maGradientInfo));

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientAxial::GeoTexSvxGradientAxial(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fAngle)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            basegfx::tools::createAxialODFGradientInfo(maGradientInfo,
                                                       rTargetRange,
                                                       nSteps,
                                                       fBorder,
                                                       fAngle);
        }

        GeoTexSvxGradientAxial::~GeoTexSvxGradientAxial()
        {
        }

        void GeoTexSvxGradientAxial::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(maGradientInfo.mnSteps)
            {
                const double fStripeWidth=1.0 / (maGradientInfo.mnSteps - 1L);
                for(sal_uInt32 a(maGradientInfo.mnSteps-1L); a != 0; a--)
                {
                    const basegfx::B2DRange aRect(0, 0, 1.0, fStripeWidth * a);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientAxial::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            if(maGradientInfo.mnSteps)
            {
                rColors.push_back(maEnd);

                for(sal_uInt32 a(1L); a < maGradientInfo.mnSteps; a++)
                {
                    rColors.push_back(interpolate(maEnd, maStart, (double)a / (double)maGradientInfo.mnSteps));
                }
            }
        }

        void GeoTexSvxGradientAxial::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::tools::getAxialGradientAlpha(rUV, maGradientInfo));

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientRadial::GeoTexSvxGradientRadial(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            basegfx::tools::createRadialODFGradientInfo(maGradientInfo,
                                                        rTargetRange,
                                                        basegfx::B2DVector(fOffsetX,fOffsetY),
                                                        nSteps,
                                                        fBorder);
        }

        GeoTexSvxGradientRadial::~GeoTexSvxGradientRadial()
        {
        }

        void GeoTexSvxGradientRadial::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(maGradientInfo.mnSteps)
            {
                const double fStepSize=1.0 / maGradientInfo.mnSteps;
                for(sal_uInt32 a(maGradientInfo.mnSteps-1L); a > 0; a--)
                {
                    const basegfx::B2DRange aRect(0, 0, fStepSize*a, fStepSize*a);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientRadial::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            impAppendColorsRadial(rColors);
        }

        void GeoTexSvxGradientRadial::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::tools::getRadialGradientAlpha(rUV, maGradientInfo));

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientElliptical::GeoTexSvxGradientElliptical(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            basegfx::tools::createEllipticalODFGradientInfo(maGradientInfo,
                                                            rTargetRange,
                                                            basegfx::B2DVector(fOffsetX,fOffsetY),
                                                            nSteps,
                                                            fBorder,
                                                            fAngle);
        }

        GeoTexSvxGradientElliptical::~GeoTexSvxGradientElliptical()
        {
        }

        void GeoTexSvxGradientElliptical::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(maGradientInfo.mnSteps)
            {
                double fWidth(1);
                double fHeight(1);
                double fIncrementX, fIncrementY;

                if(maGradientInfo.mfAspectRatio > 1.0)
                {
                    fIncrementY = fHeight / maGradientInfo.mnSteps;
                    fIncrementX = fIncrementY / maGradientInfo.mfAspectRatio;
                }
                else
                {
                    fIncrementX = fWidth / maGradientInfo.mnSteps;
                    fIncrementY = fIncrementX * maGradientInfo.mfAspectRatio;
                }

                for(sal_uInt32 a(1L); a < maGradientInfo.mnSteps; a++)
                {
                    // next step
                    fWidth  -= fIncrementX;
                    fHeight -= fIncrementY;

                    // create matrix
                    const basegfx::B2DRange aRect(0, 0, fWidth, fHeight);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientElliptical::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            impAppendColorsRadial(rColors);
        }

        void GeoTexSvxGradientElliptical::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::tools::getEllipticalGradientAlpha(rUV, maGradientInfo));

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientSquare::GeoTexSvxGradientSquare(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            basegfx::tools::createSquareODFGradientInfo(maGradientInfo,
                                                        rTargetRange,
                                                        basegfx::B2DVector(fOffsetX,fOffsetY),
                                                        nSteps,
                                                        fBorder,
                                                        fAngle);
        }

        GeoTexSvxGradientSquare::~GeoTexSvxGradientSquare()
        {
        }

        void GeoTexSvxGradientSquare::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(maGradientInfo.mnSteps)
            {
                const double fStepSize=1.0 / maGradientInfo.mnSteps;
                for(sal_uInt32 a(maGradientInfo.mnSteps-1L); a > 0; a--)
                {
                    const basegfx::B2DRange aRect(0, 0, fStepSize*a, fStepSize*a);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientSquare::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            impAppendColorsRadial(rColors);
        }

        void GeoTexSvxGradientSquare::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::tools::getSquareGradientAlpha(rUV, maGradientInfo));

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientRect::GeoTexSvxGradientRect(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            basegfx::tools::createRectangularODFGradientInfo(maGradientInfo,
                                                             rTargetRange,
                                                             basegfx::B2DVector(fOffsetX,fOffsetY),
                                                             nSteps,
                                                             fBorder,
                                                             fAngle);
        }

        GeoTexSvxGradientRect::~GeoTexSvxGradientRect()
        {
        }

        void GeoTexSvxGradientRect::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(maGradientInfo.mnSteps)
            {
                double fWidth(1);
                double fHeight(1);
                double fIncrementX, fIncrementY;

                if(maGradientInfo.mfAspectRatio > 1.0)
                {
                    fIncrementY = fHeight / maGradientInfo.mnSteps;
                    fIncrementX = fIncrementY / maGradientInfo.mfAspectRatio;
                }
                else
                {
                    fIncrementX = fWidth / maGradientInfo.mnSteps;
                    fIncrementY = fIncrementX * maGradientInfo.mfAspectRatio;
                }

                for(sal_uInt32 a(1L); a < maGradientInfo.mnSteps; a++)
                {
                    // next step
                    fWidth  -= fIncrementX;
                    fHeight -= fIncrementY;

                    // create matrix
                    const basegfx::B2DRange aRect(0, 0, fWidth, fHeight);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientRect::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            impAppendColorsRadial(rColors);
        }

        void GeoTexSvxGradientRect::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const double fScaler(basegfx::tools::getRectangularGradientAlpha(rUV, maGradientInfo));

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxHatch::GeoTexSvxHatch(const basegfx::B2DRange& rTargetRange, double fDistance, double fAngle)
        :   mfDistance(0.1),
            mfAngle(fAngle),
            mnSteps(10L)
        {
            double fTargetSizeX(rTargetRange.getWidth());
            double fTargetSizeY(rTargetRange.getHeight());
            double fTargetOffsetX(rTargetRange.getMinX());
            double fTargetOffsetY(rTargetRange.getMinY());

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

                maTextureTransform = basegfx::tools::createRotateAroundPoint(aCenter, fAngle)
                    * maTextureTransform;
            }

            // add object translate
            maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

            // prepare height for texture
            const double fSteps((0.0 != fDistance) ? fTargetSizeY / fDistance : 10.0);
            mnSteps = basegfx::fround(fSteps + 0.5);
            mfDistance = 1.0 / fSteps;

            // build transform from u,v to [0.0 .. 1.0]. As base, use inverse texture transform
            maBackTextureTransform = maTextureTransform;
            maBackTextureTransform.invert();
        }

        GeoTexSvxHatch::~GeoTexSvxHatch()
        {
        }

        bool GeoTexSvxHatch::operator==(const GeoTexSvx& rGeoTexSvx) const
        {
            const GeoTexSvxHatch* pCompare = dynamic_cast< const GeoTexSvxHatch* >(&rGeoTexSvx);
            return (pCompare
                && maTextureTransform == pCompare->maTextureTransform
                && mfDistance == pCompare->mfDistance
                && mfAngle == pCompare->mfAngle
                && mnSteps == pCompare->mnSteps);
        }

        void GeoTexSvxHatch::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            for(sal_uInt32 a(1L); a < mnSteps; a++)
            {
                // create matrix
                const double fOffset(mfDistance * (double)a);
                basegfx::B2DHomMatrix aNew;
                aNew.set(1, 2, fOffset);
                rMatrices.push_back(maTextureTransform * aNew);
            }
        }

        double GeoTexSvxHatch::getDistanceToHatch(const basegfx::B2DPoint& rUV) const
        {
            const basegfx::B2DPoint aCoor(maBackTextureTransform * rUV);
            return fmod(aCoor.getY(), mfDistance);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxTiled::GeoTexSvxTiled(const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize)
        :   maTopLeft(rTopLeft),
            maSize(rSize)
        {
            if(basegfx::fTools::lessOrEqual(maSize.getX(), 0.0))
            {
                maSize.setX(1.0);
            }

            if(basegfx::fTools::lessOrEqual(maSize.getY(), 0.0))
            {
                maSize.setY(1.0);
            }
        }

        GeoTexSvxTiled::~GeoTexSvxTiled()
        {
        }

        bool GeoTexSvxTiled::operator==(const GeoTexSvx& rGeoTexSvx) const
        {
            const GeoTexSvxTiled* pCompare = dynamic_cast< const GeoTexSvxTiled* >(&rGeoTexSvx);
            return (pCompare
                && maTopLeft == pCompare->maTopLeft
                && maSize == pCompare->maSize);
        }

        void GeoTexSvxTiled::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            double fStartX(maTopLeft.getX());
            double fStartY(maTopLeft.getY());

            if(basegfx::fTools::more(fStartX, 0.0))
            {
                fStartX -= (floor(fStartX / maSize.getX()) + 1.0) * maSize.getX();
            }

            if(basegfx::fTools::less(fStartX + maSize.getX(), 0.0))
            {
                fStartX += floor(-fStartX / maSize.getX()) * maSize.getX();
            }

            if(basegfx::fTools::more(fStartY, 0.0))
            {
                fStartY -= (floor(fStartY / maSize.getY()) + 1.0) * maSize.getY();
            }

            if(basegfx::fTools::less(fStartY + maSize.getY(), 0.0))
            {
                fStartY += floor(-fStartY / maSize.getY()) * maSize.getY();
            }

            for(double fPosY(fStartY); basegfx::fTools::less(fPosY, 1.0); fPosY += maSize.getY())
            {
                for(double fPosX(fStartX); basegfx::fTools::less(fPosX, 1.0); fPosX += maSize.getX())
                {
                    basegfx::B2DHomMatrix aNew;

                    aNew.set(0, 0, maSize.getX());
                    aNew.set(1, 1, maSize.getY());
                    aNew.set(0, 2, fPosX);
                    aNew.set(1, 2, fPosY);

                    rMatrices.push_back(aNew);
                }
            }
        }
    } // end of namespace texture
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
