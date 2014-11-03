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

#include <basegfx/matrix/b2dglmmatrixtools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

namespace basegfx
{
    namespace tools
    {

        glm::mat4 createScaleGlmMatrix(double fScaleX, double fScaleY)
        {
            glm::mat4 aRetval;
            const double fOne(1.0);

            if(!fTools::equal(fScaleX, fOne))
            {
                aRetval[0][0] = fScaleX;
            }

            if(!fTools::equal(fScaleY, fOne))
            {
                aRetval[1][1] = fScaleY;
            }

            return aRetval;
        }

        glm::mat4 createShearXGlmMatrix(double fShearX)
        {
            glm::mat4 aRetval;

            if(!fTools::equalZero(fShearX))
            {
                aRetval[1][0] = fShearX;
            }

            return aRetval;
        }

        glm::mat4 createShearYGlmMatrix(double fShearY)
        {
            glm::mat4 aRetval;

            if(!fTools::equalZero(fShearY))
            {
                aRetval[0][1] = fShearY;
            }

            return aRetval;
        }

        glm::mat4 createRotateGlmMatrix(double fRadiant)
        {
            glm::mat4 aRetval;

            if(!fTools::equalZero(fRadiant))
            {
                double fSin(0.0);
                double fCos(1.0);

                createSinCosOrthogonal(fSin, fCos, fRadiant);
                aRetval[0][0] = fCos;
                aRetval[1][1] = fCos;
                aRetval[0][1] = fSin;
                aRetval[1][0] = -fSin;
            }

            return aRetval;
        }

        glm::mat4 createTranslateGlmMatrix(double fTranslateX, double fTranslateY)
        {
            glm::mat4 aRetval;

            if(!(fTools::equalZero(fTranslateX) && fTools::equalZero(fTranslateY)))
            {
                aRetval[2][0] = fTranslateX;
                aRetval[2][1] = fTranslateY;
            }

            return aRetval;
        }

        glm::mat4 createScaleShearXRotateTranslateGlmMatrix(
            double fScaleX, double fScaleY,
            double fShearX,
            double fRadiant,
            double fTranslateX, double fTranslateY)
        {
            const double fOne(1.0);

            if(fTools::equal(fScaleX, fOne) && fTools::equal(fScaleY, fOne))
            {
                /// no scale, take shortcut
                return createShearXRotateTranslateGlmMatrix(fShearX, fRadiant, fTranslateX, fTranslateY);
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
                        return createScaleTranslateGlmMatrix(fScaleX, fScaleY, fTranslateX, fTranslateY);
                    }
                    else
                    {
                        /// rotate and scale used, no shear
                        double fSin(0.0);
                        double fCos(1.0);

                        createSinCosOrthogonal(fSin, fCos, fRadiant);

                        glm::mat4 aRetval;
                        aRetval[0][0] = fCos * fScaleX;
                        aRetval[1][0] = fScaleY * -fSin;
                        aRetval[2][0] = fTranslateX;
                        aRetval[0][1] = fSin * fScaleX;
                        aRetval[1][1] = fScaleY * fCos;
                        aRetval[2][1] = fTranslateY;

                        return aRetval;
                    }
                }
                else
                {
                    /// scale and shear used
                    if(fTools::equalZero(fRadiant))
                    {
                        /// scale and shear, but no rotate
                        glm::mat4 aRetval;
                        aRetval[0][0] = fScaleX;
                        aRetval[1][0] = fScaleY * fShearX;
                        aRetval[2][0] = fTranslateX;
                        aRetval[0][1] = 0.0;
                        aRetval[1][1] = fScaleY;
                        aRetval[2][1] = fTranslateY;

                        return aRetval;
                    }
                    else
                    {
                        /// scale, shear and rotate used
                        double fSin(0.0);
                        double fCos(1.0);

                        createSinCosOrthogonal(fSin, fCos, fRadiant);

                        glm::mat4 aRetval;
                        aRetval[0][0] = fCos * fScaleX;
                        aRetval[1][0] = fScaleY * ((fCos * fShearX) - fSin);
                        aRetval[2][0] = fTranslateX;
                        aRetval[0][1] = fSin * fScaleX;
                        aRetval[1][1] = fScaleY * ((fSin * fShearX) + fCos);
                        aRetval[2][1] = fTranslateY;

                        return aRetval;
                    }
                }
            }
        }

        glm::mat4  createShearXRotateTranslateGlmMatrix(
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
                    return createTranslateGlmMatrix(fTranslateX, fTranslateY);
                }
                else
                {
                    /// no shear, but rotate used
                    double fSin(0.0);
                    double fCos(1.0);

                    createSinCosOrthogonal(fSin, fCos, fRadiant);

                    glm::mat4 aRetval;
                    aRetval[0][0] = fCos;
                    aRetval[1][0] = -fSin;
                    aRetval[2][0] = fTranslateX;
                    aRetval[0][1] = fSin;
                    aRetval[1][1] = fCos;
                    aRetval[2][1] = fTranslateY;

                    return aRetval;
                }
            }
            else
            {
                /// shear used
                if(fTools::equalZero(fRadiant))
                {
                    /// no rotate, but shear used
                    glm::mat4 aRetval;
                    aRetval[0][0] = 1.0;
                    aRetval[1][0] = fShearX;
                    aRetval[2][0] = fTranslateX;
                    aRetval[0][1] = 0.0;
                    aRetval[1][1] = 1.0;
                    aRetval[2][1] = fTranslateY;

                    return aRetval;
                }
                else
                {
                    /// shear and rotate used
                    double fSin(0.0);
                    double fCos(1.0);

                    createSinCosOrthogonal(fSin, fCos, fRadiant);

                    glm::mat4 aRetval;
                    aRetval[0][0] = fCos;
                    aRetval[1][0] = (fCos * fShearX) - fSin;
                    aRetval[2][0] = fTranslateX;
                    aRetval[0][1] = fSin;
                    aRetval[1][1] = (fSin * fShearX) + fCos;
                    aRetval[2][1] = fTranslateY;

                    return aRetval;
                }
            }
        }

        glm::mat4  createScaleTranslateGlmMatrix(
            double fScaleX, double fScaleY,
            double fTranslateX, double fTranslateY)
        {
            const double fOne(1.0);

            if(fTools::equal(fScaleX, fOne) && fTools::equal(fScaleY, fOne))
            {
                /// no scale, take shortcut
                return createTranslateGlmMatrix(fTranslateX, fTranslateY);
            }
            else
            {
                /// scale used
                if(fTools::equalZero(fTranslateX) && fTools::equalZero(fTranslateY))
                {
                    /// no translate, but scale.
                    glm::mat4 aRetval;

                    aRetval[0][0] = fScaleX;
                    aRetval[1][1] = fScaleY;

                    return aRetval;
                }
                else
                {
                    /// translate and scale
                    glm::mat4 aRetval;
                    aRetval[0][0] = fScaleX;
                    aRetval[1][0] = 0.0;
                    aRetval[2][0] = fTranslateX;
                    aRetval[0][1] = 0.0;
                    aRetval[1][1] = fScaleY;
                    aRetval[2][1] = fTranslateY;

                    return aRetval;
                }
            }
        }

        glm::mat4  createRotateAroundPointGlmMatrix(
            double fPointX, double fPointY,
            double fRadiant)
        {
            glm::mat4 aRetval;

            if(!fTools::equalZero(fRadiant))
            {
                double fSin(0.0);
                double fCos(1.0);

                createSinCosOrthogonal(fSin, fCos, fRadiant);
                aRetval[0][0] = fCos;
                aRetval[1][0] = -fSin;
                aRetval[2][0] = (fPointX * (1.0 - fCos)) + (fSin * fPointY);
                aRetval[0][1] = fSin;
                aRetval[1][1] = fCos;
                aRetval[2][1] = (fPointY * (1.0 - fCos)) - (fSin * fPointX);
            }

            return aRetval;
        }

        /// special for the case to map from source range to target range
        glm::mat4  createSourceRangeTargetRangeTransformGlmMatrix(
            const B2DRange& rSourceRange,
            const B2DRange& rTargetRange)
        {
            glm::mat4 aRetval;

            if(&rSourceRange == &rTargetRange)
            {
                return aRetval;
            }

            if(!fTools::equalZero(rSourceRange.getMinX()) || !fTools::equalZero(rSourceRange.getMinY()))
            {
                aRetval[2][0] = -rSourceRange.getMinX();
                aRetval[2][1] = -rSourceRange.getMinY();
            }

            const double fSourceW(rSourceRange.getWidth());
            const double fSourceH(rSourceRange.getHeight());
            const bool bDivX(!fTools::equalZero(fSourceW) && !fTools::equal(fSourceW, 1.0));
            const bool bDivY(!fTools::equalZero(fSourceH) && !fTools::equal(fSourceH, 1.0));
            const double fScaleX(bDivX ? rTargetRange.getWidth() / fSourceW : rTargetRange.getWidth());
            const double fScaleY(bDivY ? rTargetRange.getHeight() / fSourceH : rTargetRange.getHeight());

            if(!fTools::equalZero(fScaleX) || !fTools::equalZero(fScaleY))
            {
                aRetval[0][0] = fScaleX;
                aRetval[1][1] = fScaleY;
            }

            if(!fTools::equalZero(rTargetRange.getMinX()) || !fTools::equalZero(rTargetRange.getMinY()))
            {
                aRetval[0][0] *= rTargetRange.getMinX();
                aRetval[1][1] *= rTargetRange.getMinY();
            }

            return aRetval;
        }

    } // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
