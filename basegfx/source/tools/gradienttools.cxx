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

namespace basegfx
{
    bool ODFGradientInfo::operator==(const ODFGradientInfo& rODFGradientInfo) const
    {
        return getTextureTransform() == rODFGradientInfo.getTextureTransform()
            && getAspectRatio() == rODFGradientInfo.getAspectRatio()
            && getSteps() == rODFGradientInfo.getSteps();
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
    ODFGradientInfo init1DGradientInfo(
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
    ODFGradientInfo initEllipticalGradientInfo(
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
            const double fOriginalDiag(sqrt((fTargetSizeX * fTargetSizeX) + (fTargetSizeY * fTargetSizeY)));

            fTargetOffsetX -= (fOriginalDiag - fTargetSizeX) / 2.0;
            fTargetOffsetY -= (fOriginalDiag - fTargetSizeY) / 2.0;
            fTargetSizeX = fOriginalDiag;
            fTargetSizeY = fOriginalDiag;
        }
        else
        {
            fTargetOffsetX -= (0.4142 / 2.0 ) * fTargetSizeX;
            fTargetOffsetY -= (0.4142 / 2.0 ) * fTargetSizeY;
            fTargetSizeX = 1.4142 * fTargetSizeX;
            fTargetSizeY = 1.4142 * fTargetSizeY;
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
    ODFGradientInfo initRectGradientInfo(
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
            // use scaled target size
            fTargetOffsetX += (rOffset.getX() - 0.5) * fTargetSizeX;
            fTargetOffsetY += (rOffset.getY() - 0.5) * fTargetSizeY;
        }

        // add object translate
        aTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

        // prepare aspect for texture
        const double fAspectRatio(fTargetSizeY == 0.0 ? 1.0 : (fTargetSizeX / fTargetSizeY));

        return ODFGradientInfo(aTextureTransform, fAspectRatio, nSteps);
    }

    namespace utils
    {
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

            // Ignore Y, this is not needed at all for Y-Oriented gradients
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

            const sal_uInt32 nSteps(rGradInfo.getSteps());

            if(nSteps)
            {
                return floor(aCoor.getY() * nSteps) / double(nSteps - 1);
            }

            return aCoor.getY();
        }

        double getAxialGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);

            // Ignore Y, this is not needed at all for Y-Oriented gradients
            //if(aCoor.getX() < 0.0 || aCoor.getX() > 1.0)
            //{
            //    return 0.0;
            //}

            const double fAbsY(fabs(aCoor.getY()));

            if(fAbsY >= 1.0)
            {
                return 1.0; // use end value when outside in Y
            }

            const sal_uInt32 nSteps(rGradInfo.getSteps());

            if(nSteps)
            {
                return floor(fAbsY * nSteps) / double(nSteps - 1);
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

            const double t(1.0 - sqrt(aCoor.getX() * aCoor.getX() + aCoor.getY() * aCoor.getY()));
            const sal_uInt32 nSteps(rGradInfo.getSteps());

            if(nSteps && t < 1.0)
            {
                return floor(t * nSteps) / double(nSteps - 1);
            }

            return t;
        }

        double getEllipticalGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            return getRadialGradientAlpha(rUV, rGradInfo); // only matrix setup differs
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
            const sal_uInt32 nSteps(rGradInfo.getSteps());

            if(nSteps && t < 1.0)
            {
                return floor(t * nSteps) / double(nSteps - 1);
            }

            return t;
        }

        double getRectangularGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            return getSquareGradientAlpha(rUV, rGradInfo); // only matrix setup differs
        }
    } // namespace utils
} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
