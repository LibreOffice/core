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

#include <basegfx/tools/gradienttools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

namespace basegfx
{
    /** Most of the setup for linear & axial gradient is the same, except
        for the border treatment. Factored out here.
    */
    static void init1DGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                   const B2DRange&  rTargetRange,
                                   sal_uInt32       nSteps,
                                   double           fBorder,
                                   double           fAngle,
                                   bool             bAxial)
    {
        o_rGradientInfo.maTextureTransform.identity();
        o_rGradientInfo.maBackTextureTransform.identity();
        o_rGradientInfo.mnSteps = nSteps;

        fAngle = -fAngle;

        double fTargetSizeX(rTargetRange.getWidth());
        double fTargetSizeY(rTargetRange.getHeight());
        double fTargetOffsetX(rTargetRange.getMinX());
        double fTargetOffsetY(rTargetRange.getMinY());

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

        const double fSizeWithoutBorder=1.0 - fBorder;
        if( bAxial )
        {
            o_rGradientInfo.maTextureTransform.scale(1.0, fSizeWithoutBorder * .5);
            o_rGradientInfo.maTextureTransform.translate(0.0, 0.5);
        }
        else
        {
            if(!fTools::equal(fSizeWithoutBorder, 1.0))
            {
                o_rGradientInfo.maTextureTransform.scale(1.0, fSizeWithoutBorder);
                o_rGradientInfo.maTextureTransform.translate(0.0, fBorder);
            }
        }

        o_rGradientInfo.maTextureTransform.scale(fTargetSizeX, fTargetSizeY);

        // add texture rotate after scale to keep perpendicular angles
        if(0.0 != fAngle)
        {
            const B2DPoint aCenter(0.5*fTargetSizeX,
                                   0.5*fTargetSizeY);
            o_rGradientInfo.maTextureTransform *=
                basegfx::tools::createRotateAroundPoint(aCenter, fAngle);
        }

        // add object translate
        o_rGradientInfo.maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

        // prepare aspect for texture
        o_rGradientInfo.mfAspectRatio = (0.0 != fTargetSizeY) ?  fTargetSizeX / fTargetSizeY : 1.0;

        // build transform from u,v to [0.0 .. 1.0].
        o_rGradientInfo.maBackTextureTransform = o_rGradientInfo.maTextureTransform;
        o_rGradientInfo.maBackTextureTransform.invert();
    }

    /** Most of the setup for radial & ellipsoidal gradient is the same,
        except for the border treatment. Factored out here.
    */
    static void initEllipticalGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                           const B2DRange&  rTargetRange,
                                           const B2DVector& rOffset,
                                           sal_uInt32       nSteps,
                                           double           fBorder,
                                           double           fAngle,
                                           bool             bCircular)
    {
        o_rGradientInfo.maTextureTransform.identity();
        o_rGradientInfo.maBackTextureTransform.identity();
        o_rGradientInfo.mnSteps = nSteps;

        fAngle = -fAngle;

        double fTargetSizeX(rTargetRange.getWidth());
        double fTargetSizeY(rTargetRange.getHeight());
        double fTargetOffsetX(rTargetRange.getMinX());
        double fTargetOffsetY(rTargetRange.getMinY());

        // add object expansion
        if( bCircular )
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
        o_rGradientInfo.maTextureTransform.scale(fHalfBorder, fHalfBorder);

        o_rGradientInfo.maTextureTransform.translate(0.5, 0.5);
        o_rGradientInfo.maTextureTransform.scale(fTargetSizeX, fTargetSizeY);

        // add texture rotate after scale to keep perpendicular angles
        if( !bCircular && 0.0 != fAngle)
        {
            const B2DPoint aCenter(0.5*fTargetSizeX,
                                   0.5*fTargetSizeY);
            o_rGradientInfo.maTextureTransform *=
                basegfx::tools::createRotateAroundPoint(aCenter, fAngle);
        }

        // add defined offsets after rotation
        if(0.5 != rOffset.getX() || 0.5 != rOffset.getY())
        {
            // use original target size
            fTargetOffsetX += (rOffset.getX() - 0.5) * rTargetRange.getWidth();
            fTargetOffsetY += (rOffset.getY() - 0.5) * rTargetRange.getHeight();
        }

        // add object translate
        o_rGradientInfo.maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

        // prepare aspect for texture
        o_rGradientInfo.mfAspectRatio = (0.0 != fTargetSizeY) ?  fTargetSizeX / fTargetSizeY : 1.0;

        // build transform from u,v to [0.0 .. 1.0].
        o_rGradientInfo.maBackTextureTransform = o_rGradientInfo.maTextureTransform;
        o_rGradientInfo.maBackTextureTransform.invert();
    }

    /** Setup for rect & square gradient is exactly the same. Factored out
        here.
    */
    static void initRectGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                     const B2DRange&  rTargetRange,
                                     const B2DVector& rOffset,
                                     sal_uInt32       nSteps,
                                     double           fBorder,
                                     double           fAngle,
                                     bool             bSquare)
    {
        o_rGradientInfo.maTextureTransform.identity();
        o_rGradientInfo.maBackTextureTransform.identity();
        o_rGradientInfo.mnSteps = nSteps;

        fAngle = -fAngle;

        double fTargetSizeX(rTargetRange.getWidth());
        double fTargetSizeY(rTargetRange.getHeight());
        double fTargetOffsetX(rTargetRange.getMinX());
        double fTargetOffsetY(rTargetRange.getMinY());

        // add object expansion
        if(bSquare)
        {
            const double fSquareWidth((fTargetSizeX>fTargetSizeY) ? fTargetSizeX : fTargetSizeY);
            fTargetOffsetX = fTargetOffsetX + fTargetSizeX / 2.0 - fSquareWidth / 2.0;
            fTargetOffsetY = fTargetOffsetY + fTargetSizeY / 2.0 - fSquareWidth / 2.0;
            fTargetSizeX = fSquareWidth;
            fTargetSizeY = fSquareWidth;
        }

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

        const double fHalfBorder((1.0 - fBorder) * 0.5);
        o_rGradientInfo.maTextureTransform.scale(fHalfBorder, fHalfBorder);

        o_rGradientInfo.maTextureTransform.translate(0.5, 0.5);
        o_rGradientInfo.maTextureTransform.scale(fTargetSizeX, fTargetSizeY);

        // add texture rotate after scale to keep perpendicular angles
        if(0.0 != fAngle)
        {
            const B2DPoint aCenter(0.5*fTargetSizeX,
                                   0.5*fTargetSizeY);
            o_rGradientInfo.maTextureTransform *=
                basegfx::tools::createRotateAroundPoint(aCenter, fAngle);
        }

        // add defined offsets after rotation
        if(0.5 != rOffset.getX() || 0.5 != rOffset.getY())
        {
            // use scaled target size
            fTargetOffsetX += (rOffset.getX() - 0.5) * fTargetSizeX;
            fTargetOffsetY += (rOffset.getY() - 0.5) * fTargetSizeY;
        }

        // add object translate
        o_rGradientInfo.maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

        // prepare aspect for texture
        o_rGradientInfo.mfAspectRatio = (0.0 != fTargetSizeY) ? fTargetSizeX / fTargetSizeY : 1.0;

        // build transform from u,v to [0.0 .. 1.0]. As base, use inverse texture transform
        o_rGradientInfo.maBackTextureTransform = o_rGradientInfo.maTextureTransform;
        o_rGradientInfo.maBackTextureTransform.invert();
    }

    namespace tools
    {
        ODFGradientInfo& createLinearODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                     const B2DRange&  rTargetArea,
                                                     sal_uInt32       nSteps,
                                                     double           fBorder,
                                                     double           fAngle)
        {
            init1DGradientInfo(o_rGradientInfo,
                               rTargetArea,
                               nSteps,
                               fBorder,
                               fAngle,
                               false);
            return o_rGradientInfo;
        }

        ODFGradientInfo& createAxialODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                    const B2DRange&  rTargetArea,
                                                    sal_uInt32       nSteps,
                                                    double           fBorder,
                                                    double           fAngle)
        {
            init1DGradientInfo(o_rGradientInfo,
                               rTargetArea,
                               nSteps,
                               fBorder,
                               fAngle,
                               true);
            return o_rGradientInfo;
        }

        ODFGradientInfo& createRadialODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                     const B2DRange&  rTargetArea,
                                                     const B2DVector& rOffset,
                                                     sal_uInt32       nSteps,
                                                     double           fBorder)
        {
            initEllipticalGradientInfo(o_rGradientInfo,
                                       rTargetArea,
                                       rOffset,
                                       nSteps,
                                       fBorder,
                                       0.0,
                                       true);
            return o_rGradientInfo;
        }

        ODFGradientInfo& createEllipticalODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                         const B2DRange&  rTargetArea,
                                                         const B2DVector& rOffset,
                                                         sal_uInt32       nSteps,
                                                         double           fBorder,
                                                         double           fAngle)
        {
            initEllipticalGradientInfo(o_rGradientInfo,
                                       rTargetArea,
                                       rOffset,
                                       nSteps,
                                       fBorder,
                                       fAngle,
                                       false);
            return o_rGradientInfo;
        }

        ODFGradientInfo& createSquareODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                     const B2DRange&  rTargetArea,
                                                     const B2DVector& rOffset,
                                                     sal_uInt32       nSteps,
                                                     double           fBorder,
                                                     double           fAngle)
        {
            initRectGradientInfo(o_rGradientInfo,
                                 rTargetArea,
                                 rOffset,
                                 nSteps,
                                 fBorder,
                                 fAngle,
                                 true);
            return o_rGradientInfo;
        }

        ODFGradientInfo& createRectangularODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                          const B2DRange&  rTargetArea,
                                                          const B2DVector& rOffset,
                                                          sal_uInt32       nSteps,
                                                          double           fBorder,
                                                          double           fAngle)
        {
            initRectGradientInfo(o_rGradientInfo,
                                 rTargetArea,
                                 rOffset,
                                 nSteps,
                                 fBorder,
                                 fAngle,
                                 false);
            return o_rGradientInfo;
        }

    } // namespace tools

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
