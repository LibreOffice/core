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

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
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
    } // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
