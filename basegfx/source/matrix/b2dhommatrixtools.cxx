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

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/range/b2drange.hxx>

#include <osl/diagnose.h>

namespace basegfx
{
    namespace utils
    {
        void createSinCosOrthogonal(double& o_rSin, double& o_rCos, double fRadiant)
        {
            if( fTools::equalZero( fmod( fRadiant, F_PI2 ) ) )
            {
                // determine quadrant
                const sal_Int32 nQuad(
                    (4 + fround( 4/F_2PI*fmod( fRadiant, F_2PI ) )) % 4 );
                switch( nQuad )
                {
                    case 0: // -2pi,0,2pi
                        o_rSin = 0.0;
                        o_rCos = 1.0;
                        break;

                    case 1: // -3/2pi,1/2pi
                        o_rSin = 1.0;
                        o_rCos = 0.0;
                        break;

                    case 2: // -pi,pi
                        o_rSin = 0.0;
                        o_rCos = -1.0;
                        break;

                    case 3: // -1/2pi,3/2pi
                        o_rSin = -1.0;
                        o_rCos = 0.0;
                        break;

                    default:
                        OSL_FAIL( "createSinCos: Impossible case reached" );
                }
            }
            else
            {
                // TODO(P1): Maybe use glibc's sincos here (though
                // that's kinda non-portable...)
                o_rSin = sin(fRadiant);
                o_rCos = cos(fRadiant);
            }
        }

        B2DHomMatrix createScaleB2DHomMatrix(double fScaleX, double fScaleY)
        {
            B2DHomMatrix aRetval;
            const double fOne(1.0);

            if(!fTools::equal(fScaleX, fOne))
            {
                aRetval.set(0, 0, fScaleX);
            }

            if(!fTools::equal(fScaleY, fOne))
            {
                aRetval.set(1, 1, fScaleY);
            }

            return aRetval;
        }

        B2DHomMatrix createShearXB2DHomMatrix(double fShearX)
        {
            B2DHomMatrix aRetval;

            if(!fTools::equalZero(fShearX))
            {
                aRetval.set(0, 1, fShearX);
            }

            return aRetval;
        }

        B2DHomMatrix createShearYB2DHomMatrix(double fShearY)
        {
            B2DHomMatrix aRetval;

            if(!fTools::equalZero(fShearY))
            {
                aRetval.set(1, 0, fShearY);
            }

            return aRetval;
        }

        B2DHomMatrix createRotateB2DHomMatrix(double fRadiant)
        {
            B2DHomMatrix aRetval;

            if(!fTools::equalZero(fRadiant))
            {
                double fSin(0.0);
                double fCos(1.0);

                createSinCosOrthogonal(fSin, fCos, fRadiant);
                aRetval.set(0, 0, fCos);
                aRetval.set(1, 1, fCos);
                aRetval.set(1, 0, fSin);
                aRetval.set(0, 1, -fSin);
            }

            return aRetval;
        }

        B2DHomMatrix createTranslateB2DHomMatrix(double fTranslateX, double fTranslateY)
        {
            B2DHomMatrix aRetval;

            if(!(fTools::equalZero(fTranslateX) && fTools::equalZero(fTranslateY)))
            {
                aRetval.set(0, 2, fTranslateX);
                aRetval.set(1, 2, fTranslateY);
            }

            return aRetval;
        }

        B2DHomMatrix createScaleShearXRotateTranslateB2DHomMatrix(
            double fScaleX, double fScaleY,
            double fShearX,
            double fRadiant,
            double fTranslateX, double fTranslateY)
        {
            const double fOne(1.0);

            if(fTools::equal(fScaleX, fOne) && fTools::equal(fScaleY, fOne))
            {
                /// no scale, take shortcut
                return createShearXRotateTranslateB2DHomMatrix(fShearX, fRadiant, fTranslateX, fTranslateY);
            }
            else
            {
                /// scale used
                if(fTools::equalZero(fShearX))
                {
                    /// no shear
                    if(fTools::equalZero(fRadiant))
                    {
                        /// no rotate, take shortcut
                        return createScaleTranslateB2DHomMatrix(fScaleX, fScaleY, fTranslateX, fTranslateY);
                    }
                    else
                    {
                        /// rotate and scale used, no shear
                        double fSin(0.0);
                        double fCos(1.0);

                        createSinCosOrthogonal(fSin, fCos, fRadiant);

                        B2DHomMatrix aRetval(
                            /* Row 0, Column 0 */ fCos * fScaleX,
                            /* Row 0, Column 1 */ fScaleY * -fSin,
                            /* Row 0, Column 2 */ fTranslateX,
                            /* Row 1, Column 0 */ fSin * fScaleX,
                            /* Row 1, Column 1 */ fScaleY * fCos,
                            /* Row 1, Column 2 */ fTranslateY);

                        return aRetval;
                    }
                }
                else
                {
                    /// scale and shear used
                    if(fTools::equalZero(fRadiant))
                    {
                        /// scale and shear, but no rotate
                        B2DHomMatrix aRetval(
                            /* Row 0, Column 0 */ fScaleX,
                            /* Row 0, Column 1 */ fScaleY * fShearX,
                            /* Row 0, Column 2 */ fTranslateX,
                            /* Row 1, Column 0 */ 0.0,
                            /* Row 1, Column 1 */ fScaleY,
                            /* Row 1, Column 2 */ fTranslateY);

                        return aRetval;
                    }
                    else
                    {
                        /// scale, shear and rotate used
                        double fSin(0.0);
                        double fCos(1.0);

                        createSinCosOrthogonal(fSin, fCos, fRadiant);

                        B2DHomMatrix aRetval(
                            /* Row 0, Column 0 */ fCos * fScaleX,
                            /* Row 0, Column 1 */ fScaleY * ((fCos * fShearX) - fSin),
                            /* Row 0, Column 2 */ fTranslateX,
                            /* Row 1, Column 0 */ fSin * fScaleX,
                            /* Row 1, Column 1 */ fScaleY * ((fSin * fShearX) + fCos),
                            /* Row 1, Column 2 */ fTranslateY);

                        return aRetval;
                    }
                }
            }
        }

        B2DHomMatrix createShearXRotateTranslateB2DHomMatrix(
            double fShearX,
            double fRadiant,
            double fTranslateX, double fTranslateY)
        {
            if(fTools::equalZero(fShearX))
            {
                /// no shear
                if(fTools::equalZero(fRadiant))
                {
                    /// no shear, no rotate, take shortcut
                    return createTranslateB2DHomMatrix(fTranslateX, fTranslateY);
                }
                else
                {
                    /// no shear, but rotate used
                    double fSin(0.0);
                    double fCos(1.0);

                    createSinCosOrthogonal(fSin, fCos, fRadiant);

                    B2DHomMatrix aRetval(
                        /* Row 0, Column 0 */ fCos,
                        /* Row 0, Column 1 */ -fSin,
                        /* Row 0, Column 2 */ fTranslateX,
                        /* Row 1, Column 0 */ fSin,
                        /* Row 1, Column 1 */ fCos,
                        /* Row 1, Column 2 */ fTranslateY);

                    return aRetval;
                }
            }
            else
            {
                /// shear used
                if(fTools::equalZero(fRadiant))
                {
                    /// no rotate, but shear used
                    B2DHomMatrix aRetval(
                        /* Row 0, Column 0 */ 1.0,
                        /* Row 0, Column 1 */ fShearX,
                        /* Row 0, Column 2 */ fTranslateX,
                        /* Row 1, Column 0 */ 0.0,
                        /* Row 1, Column 1 */ 1.0,
                        /* Row 1, Column 2 */ fTranslateY);

                    return aRetval;
                }
                else
                {
                    /// shear and rotate used
                    double fSin(0.0);
                    double fCos(1.0);

                    createSinCosOrthogonal(fSin, fCos, fRadiant);

                    B2DHomMatrix aRetval(
                        /* Row 0, Column 0 */ fCos,
                        /* Row 0, Column 1 */ (fCos * fShearX) - fSin,
                        /* Row 0, Column 2 */ fTranslateX,
                        /* Row 1, Column 0 */ fSin,
                        /* Row 1, Column 1 */ (fSin * fShearX) + fCos,
                        /* Row 1, Column 2 */ fTranslateY);

                    return aRetval;
                }
            }
        }

        B2DHomMatrix createScaleTranslateB2DHomMatrix(
            double fScaleX, double fScaleY,
            double fTranslateX, double fTranslateY)
        {
            const double fOne(1.0);

            if(fTools::equal(fScaleX, fOne) && fTools::equal(fScaleY, fOne))
            {
                /// no scale, take shortcut
                return createTranslateB2DHomMatrix(fTranslateX, fTranslateY);
            }
            else
            {
                /// scale used
                if(fTools::equalZero(fTranslateX) && fTools::equalZero(fTranslateY))
                {
                    /// no translate, but scale.
                    B2DHomMatrix aRetval;

                    aRetval.set(0, 0, fScaleX);
                    aRetval.set(1, 1, fScaleY);

                    return aRetval;
                }
                else
                {
                    /// translate and scale
                    B2DHomMatrix aRetval(
                        /* Row 0, Column 0 */ fScaleX,
                        /* Row 0, Column 1 */ 0.0,
                        /* Row 0, Column 2 */ fTranslateX,
                        /* Row 1, Column 0 */ 0.0,
                        /* Row 1, Column 1 */ fScaleY,
                        /* Row 1, Column 2 */ fTranslateY);

                    return aRetval;
                }
            }
        }

        B2DHomMatrix createRotateAroundPoint(
            double fPointX, double fPointY,
            double fRadiant)
        {
            B2DHomMatrix aRetval;

            if(!fTools::equalZero(fRadiant))
            {
                double fSin(0.0);
                double fCos(1.0);

                createSinCosOrthogonal(fSin, fCos, fRadiant);

                aRetval.set3x2(
                    /* Row 0, Column 0 */ fCos,
                    /* Row 0, Column 1 */ -fSin,
                    /* Row 0, Column 2 */ (fPointX * (1.0 - fCos)) + (fSin * fPointY),
                    /* Row 1, Column 0 */ fSin,
                    /* Row 1, Column 1 */ fCos,
                    /* Row 1, Column 2 */ (fPointY * (1.0 - fCos)) - (fSin * fPointX));
            }

            return aRetval;
        }

        BASEGFX_DLLPUBLIC B2DHomMatrix createRotateAroundCenterKeepAspectRatioStayInsideRange(
            const basegfx::B2DRange& rTargetRange,
            double fRotate)
        {
            basegfx::B2DHomMatrix aRetval;

            // RotGrfFlyFrame: Create a transformation that maps the range inside of itself
            // so that it fits, takes as much space as possible and keeps the aspect ratio
            if(0.0 != fRotate)
            {
                // Fit rotated graphic to center of available space, keeping page ratio:
                // Adapt scaling ratio of unit object and rotate it
                aRetval.scale(1.0, rTargetRange.getHeight() / rTargetRange.getWidth());
                aRetval.rotate(fRotate);

                // get the range to see where we are in unit coordinates
                basegfx::B2DRange aFullRange(0.0, 0.0, 1.0, 1.0);
                aFullRange.transform(aRetval);

                // detect needed scales in X/Y and choose the smallest for staying inside the
                // available space while keeping aspect ratio of the source
                const double fScaleX(rTargetRange.getWidth() / aFullRange.getWidth());
                const double fScaleY(rTargetRange.getHeight() / aFullRange.getHeight());
                const double fScaleMin(std::min(fScaleX, fScaleY));

                // TopLeft to zero, then scale, then move to center of available space
                aRetval.translate(-aFullRange.getMinX(), -aFullRange.getMinY());
                aRetval.scale(fScaleMin, fScaleMin);
                aRetval.translate(
                    rTargetRange.getCenterX() - (0.5 * fScaleMin * aFullRange.getWidth()),
                    rTargetRange.getCenterY() - (0.5 * fScaleMin * aFullRange.getHeight()));
            }
            else
            {
                // just scale/translate needed
                aRetval *= createScaleTranslateB2DHomMatrix(
                    rTargetRange.getRange(),
                    rTargetRange.getMinimum());
            }

            return aRetval;
        }

        /// special for the case to map from source range to target range
        B2DHomMatrix createSourceRangeTargetRangeTransform(
            const B2DRange& rSourceRange,
            const B2DRange& rTargetRange)
        {
            B2DHomMatrix aRetval;

            if(&rSourceRange == &rTargetRange)
            {
                return aRetval;
            }

            if(!fTools::equalZero(rSourceRange.getMinX()) || !fTools::equalZero(rSourceRange.getMinY()))
            {
                aRetval.set(0, 2, -rSourceRange.getMinX());
                aRetval.set(1, 2, -rSourceRange.getMinY());
            }

            const double fSourceW(rSourceRange.getWidth());
            const double fSourceH(rSourceRange.getHeight());
            const bool bDivX(!fTools::equalZero(fSourceW) && !fTools::equal(fSourceW, 1.0));
            const bool bDivY(!fTools::equalZero(fSourceH) && !fTools::equal(fSourceH, 1.0));
            const double fScaleX(bDivX ? rTargetRange.getWidth() / fSourceW : rTargetRange.getWidth());
            const double fScaleY(bDivY ? rTargetRange.getHeight() / fSourceH : rTargetRange.getHeight());

            if(!fTools::equalZero(fScaleX) || !fTools::equalZero(fScaleY))
            {
                aRetval.scale(fScaleX, fScaleY);
            }

            if(!fTools::equalZero(rTargetRange.getMinX()) || !fTools::equalZero(rTargetRange.getMinY()))
            {
                aRetval.translate(
                    rTargetRange.getMinX(),
                    rTargetRange.getMinY());
            }

            return aRetval;
        }

        B2DHomMatrix createCoordinateSystemTransform(
            const B2DPoint& rOrigin,
            const B2DVector& rX,
            const B2DVector& rY)
        {
            return basegfx::B2DHomMatrix(
                rX.getX(), rY.getX(), rOrigin.getX(),
                rX.getY(), rY.getY(), rOrigin.getY());
        }

        B2DTuple getColumn(const B2DHomMatrix& rMatrix, sal_uInt16 nCol)
        {
            return B2DTuple(rMatrix.get(0, nCol), rMatrix.get(1, nCol));
        }
    } // end of namespace utils
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
