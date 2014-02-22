/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>



namespace basegfx
{
    namespace tools
    {
        void createSinCosOrthogonal(double& o_rSin, double& o_rCos, double fRadiant)
        {
            if( fTools::equalZero( fmod( fRadiant, F_PI2 ) ) )
            {
                
                const sal_Int32 nQuad(
                    (4 + fround( 4/F_2PI*fmod( fRadiant, F_2PI ) )) % 4 );
                switch( nQuad )
                {
                    case 0: 
                        o_rSin = 0.0;
                        o_rCos = 1.0;
                        break;

                    case 1: 
                        o_rSin = 1.0;
                        o_rCos = 0.0;
                        break;

                    case 2: 
                        o_rSin = 0.0;
                        o_rCos = -1.0;
                        break;

                    case 3: 
                        o_rSin = -1.0;
                        o_rCos = 0.0;
                        break;

                    default:
                        OSL_FAIL( "createSinCos: Impossible case reached" );
                }
            }
            else
            {
                
                
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
                
                return createShearXRotateTranslateB2DHomMatrix(fShearX, fRadiant, fTranslateX, fTranslateY);
            }
            else
            {
                
                if(fTools::equalZero(fShearX))
                {
                    
                    if(fTools::equalZero(fRadiant))
                    {
                        
                        return createScaleTranslateB2DHomMatrix(fScaleX, fScaleY, fTranslateX, fTranslateY);
                    }
                    else
                    {
                        
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
                    
                    if(fTools::equalZero(fRadiant))
                    {
                        
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
                
                if(fTools::equalZero(fRadiant))
                {
                    
                    return createTranslateB2DHomMatrix(fTranslateX, fTranslateY);
                }
                else
                {
                    
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
                
                if(fTools::equalZero(fRadiant))
                {
                    
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
                
                return createTranslateB2DHomMatrix(fTranslateX, fTranslateY);
            }
            else
            {
                
                if(fTools::equalZero(fTranslateX) && fTools::equalZero(fTranslateY))
                {
                    
                    B2DHomMatrix aRetval;

                    aRetval.set(0, 0, fScaleX);
                    aRetval.set(1, 1, fScaleY);

                    return aRetval;
                }
                else
                {
                    
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

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
