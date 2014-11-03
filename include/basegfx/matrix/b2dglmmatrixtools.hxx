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

#ifndef INCLUDED_BASEGFX_MATRIX_B2DGLMMATRIXTOOLS_HXX
#define INCLUDED_BASEGFX_MATRIX_B2DGLMMATRIXTOOLS_HXX

#include <sal/types.h>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/basegfxdllapi.h>
#include <glm/glm.hpp>



namespace basegfx
{
    namespace tools
    {
        /** Tooling methods for on-the-fly matrix generation e.g. for inline
            multiplications
         */
        BASEGFX_DLLPUBLIC glm::mat4 createScaleGlmMatrix(double fScaleX, double fScaleY);
        BASEGFX_DLLPUBLIC glm::mat4 createShearXGlmMatrix(double fShearX);
        BASEGFX_DLLPUBLIC glm::mat4 createShearYGlmMatrix(double fShearY);
        BASEGFX_DLLPUBLIC glm::mat4 createRotateGlmMatrix(double fRadiant);
        BASEGFX_DLLPUBLIC glm::mat4 createTranslateGlmMatrix(double fTranslateX, double fTranslateY);

        /// inline versions for parameters as tuples
        inline glm::mat4 createScaleGlmMatrix(const B2DTuple& rScale)
        {
            return createScaleGlmMatrix(rScale.getX(), rScale.getY());
        }

        inline glm::mat4 createTranslateGlmMatrix(const B2DTuple& rTranslate)
        {
            return createTranslateGlmMatrix(rTranslate.getX(), rTranslate.getY());
        }

        /** Tooling methods for faster completely combined matrix creation
            when scale, shearX, rotation and translation needs to be done in
            exactly that order. It's faster since it direcly calculates
            each matrix value based on a symbolic calculation of the three
            matrix multiplications.
            Inline versions for parameters as tuples added, too.
         */
        BASEGFX_DLLPUBLIC glm::mat4 createScaleShearXRotateTranslateGlmMatrix(
            double fScaleX, double fScaleY,
            double fShearX,
            double fRadiant,
            double fTranslateX, double fTranslateY);
        inline glm::mat4 createScaleShearXRotateTranslateGlmMatrix(
            const B2DTuple& rScale,
            double fShearX,
            double fRadiant,
            const B2DTuple& rTranslate)
        {
            return createScaleShearXRotateTranslateGlmMatrix(
                rScale.getX(), rScale.getY(),
                fShearX,
                fRadiant,
                rTranslate.getX(), rTranslate.getY());
        }

        BASEGFX_DLLPUBLIC glm::mat4 createShearXRotateTranslateGlmMatrix(
            double fShearX,
            double fRadiant,
            double fTranslateX, double fTranslateY);
        inline glm::mat4 createShearXRotateTranslateGlmMatrix(
            double fShearX,
            double fRadiant,
            const B2DTuple& rTranslate)
        {
            return createShearXRotateTranslateGlmMatrix(
                fShearX,
                fRadiant,
                rTranslate.getX(), rTranslate.getY());
        }

        BASEGFX_DLLPUBLIC glm::mat4 createScaleTranslateGlmMatrix(
            double fScaleX, double fScaleY,
            double fTranslateX, double fTranslateY);
        inline glm::mat4 createScaleTranslateGlmMatrix(
            const B2DTuple& rScale,
            const B2DTuple& rTranslate)
        {
            return createScaleTranslateGlmMatrix(
                rScale.getX(), rScale.getY(),
                rTranslate.getX(), rTranslate.getY());
        }

        /// special for the often used case of rotation around a point
        BASEGFX_DLLPUBLIC glm::mat4 createRotateAroundPointGlmMatrix(
            double fPointX, double fPointY,
            double fRadiant);
        inline glm::mat4 createRotateAroundPointGlmMatrix(
            const B2DTuple& rPoint,
            double fRadiant)
        {
            return createRotateAroundPointGlmMatrix(
                rPoint.getX(), rPoint.getY(),
                fRadiant);
        }

        /// special for the case to map from source range to target range
        BASEGFX_DLLPUBLIC glm::mat4 createSourceRangeTargetRangeTransformGlmMatrix(
            const B2DRange& rSourceRange,
            const B2DRange& rTargetRange);

    } // end of namespace tools
} // end of namespace basegfx




#endif // INCLUDED_BASEGFX_MATRIX_GlmMATRIXTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
