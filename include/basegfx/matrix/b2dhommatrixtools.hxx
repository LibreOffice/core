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

#ifndef _BGFX_MATRIX_B2DHOMMATRIXTOOLS_HXX
#define _BGFX_MATRIX_B2DHOMMATRIXTOOLS_HXX

#include <sal/types.h>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/basegfxdllapi.h>


///////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        /** If the rotation angle is an approximate multiple of pi/2,
            force fSin/fCos to -1/0/1, to maintain orthogonality (which
            might also be advantageous for the other cases, but: for
            multiples of pi/2, the exact values _can_ be attained. It
            would be largely unintuitive, if a 180 degrees rotation
            would introduce slight roundoff errors, instead of exactly
            mirroring the coordinate system)
         */
        BASEGFX_DLLPUBLIC void createSinCosOrthogonal(double& o_rSin, double& rCos, double fRadiant);

        /** Tooling methods for on-the-fly matrix generation e.g. for inline
            multiplications
         */
        BASEGFX_DLLPUBLIC B2DHomMatrix createScaleB2DHomMatrix(double fScaleX, double fScaleY);
        BASEGFX_DLLPUBLIC B2DHomMatrix createShearXB2DHomMatrix(double fShearX);
        BASEGFX_DLLPUBLIC B2DHomMatrix createShearYB2DHomMatrix(double fShearY);
        BASEGFX_DLLPUBLIC B2DHomMatrix createRotateB2DHomMatrix(double fRadiant);
        BASEGFX_DLLPUBLIC B2DHomMatrix createTranslateB2DHomMatrix(double fTranslateX, double fTranslateY);

        /// inline versions for parameters as tuples
        inline B2DHomMatrix createScaleB2DHomMatrix(const B2DTuple& rScale)
        {
            return createScaleB2DHomMatrix(rScale.getX(), rScale.getY());
        }

        inline B2DHomMatrix createTranslateB2DHomMatrix(const B2DTuple& rTranslate)
        {
            return createTranslateB2DHomMatrix(rTranslate.getX(), rTranslate.getY());
        }

        /** Tooling methods for faster completely combined matrix creation
            when scale, shearX, rotation and translation needs to be done in
            exactly that order. It's faster since it direcly calculates
            each matrix value based on a symbolic calculation of the three
            matrix multiplications.
            Inline versions for parameters as tuples added, too.
         */
        BASEGFX_DLLPUBLIC B2DHomMatrix createScaleShearXRotateTranslateB2DHomMatrix(
            double fScaleX, double fScaleY,
            double fShearX,
            double fRadiant,
            double fTranslateX, double fTranslateY);
        inline B2DHomMatrix createScaleShearXRotateTranslateB2DHomMatrix(
            const B2DTuple& rScale,
            double fShearX,
            double fRadiant,
            const B2DTuple& rTranslate)
        {
            return createScaleShearXRotateTranslateB2DHomMatrix(
                rScale.getX(), rScale.getY(),
                fShearX,
                fRadiant,
                rTranslate.getX(), rTranslate.getY());
        }

        BASEGFX_DLLPUBLIC B2DHomMatrix createShearXRotateTranslateB2DHomMatrix(
            double fShearX,
            double fRadiant,
            double fTranslateX, double fTranslateY);
        inline B2DHomMatrix createShearXRotateTranslateB2DHomMatrix(
            double fShearX,
            double fRadiant,
            const B2DTuple& rTranslate)
        {
            return createShearXRotateTranslateB2DHomMatrix(
                fShearX,
                fRadiant,
                rTranslate.getX(), rTranslate.getY());
        }

        BASEGFX_DLLPUBLIC B2DHomMatrix createScaleTranslateB2DHomMatrix(
            double fScaleX, double fScaleY,
            double fTranslateX, double fTranslateY);
        inline B2DHomMatrix createScaleTranslateB2DHomMatrix(
            const B2DTuple& rScale,
            const B2DTuple& rTranslate)
        {
            return createScaleTranslateB2DHomMatrix(
                rScale.getX(), rScale.getY(),
                rTranslate.getX(), rTranslate.getY());
        }

        /// special for the often used case of rotation around a point
        BASEGFX_DLLPUBLIC B2DHomMatrix createRotateAroundPoint(
            double fPointX, double fPointY,
            double fRadiant);
        inline B2DHomMatrix createRotateAroundPoint(
            const B2DTuple& rPoint,
            double fRadiant)
        {
            return createRotateAroundPoint(
                rPoint.getX(), rPoint.getY(),
                fRadiant);
        }

        /// special for the case to map from source range to target range
        BASEGFX_DLLPUBLIC B2DHomMatrix createSourceRangeTargetRangeTransform(
            const B2DRange& rSourceRange,
            const B2DRange& rTargetRange);

    } // end of namespace tools
} // end of namespace basegfx

///////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        class BASEGFX_DLLPUBLIC B2DHomMatrixBufferedDecompose
        {
        private:
            B2DVector              maScale;
            B2DVector              maTranslate;
            double                 mfRotate;
            double                 mfShearX;

        public:
            B2DHomMatrixBufferedDecompose(const B2DHomMatrix& rB2DHomMatrix = B2DHomMatrix())
            :   maScale(),
                maTranslate(),
                mfRotate(0.0),
                mfShearX(0.0)
            {
                rB2DHomMatrix.decompose(maScale, maTranslate, mfRotate, mfShearX);
            }

            // data access
            B2DHomMatrix getB2DHomMatrix() const
            {
                return createScaleShearXRotateTranslateB2DHomMatrix(
                    maScale, mfShearX, mfRotate, maTranslate);
            }

            const B2DVector& getScale() const { return maScale; }
            const B2DVector& getTranslate() const { return maTranslate; }
            double getRotate() const { return mfRotate; }
            double getShearX() const { return mfShearX; }
        };
    } // end of namespace tools
} // end of namespace basegfx

///////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        class BASEGFX_DLLPUBLIC B2DHomMatrixBufferedOnDemandDecompose
        {
        private:
            B2DHomMatrix           maB2DHomMatrix;
            B2DVector              maScale;
            B2DVector              maTranslate;
            double                 mfRotate;
            double                 mfShearX;

            // bitfield
            unsigned               mbDecomposed : 1;

            void impCheckDecompose()
            {
                if(!mbDecomposed)
                {
                    maB2DHomMatrix.decompose(maScale, maTranslate, mfRotate, mfShearX);
                    mbDecomposed = true;
                }
            }

        public:
            B2DHomMatrixBufferedOnDemandDecompose(const B2DHomMatrix& rB2DHomMatrix = B2DHomMatrix())
            :   maB2DHomMatrix(rB2DHomMatrix),
                maScale(),
                maTranslate(),
                mfRotate(0.0),
                mfShearX(0.0),
                mbDecomposed(false)
            {
            }

            // data access
            const B2DHomMatrix& getB2DHomMatrix() const { return maB2DHomMatrix; }
            const B2DVector& getScale() const { const_cast< B2DHomMatrixBufferedOnDemandDecompose* >(this)->impCheckDecompose(); return maScale; }
            const B2DVector& getTranslate() const { const_cast< B2DHomMatrixBufferedOnDemandDecompose* >(this)->impCheckDecompose(); return maTranslate; }
            double getRotate() const { const_cast< B2DHomMatrixBufferedOnDemandDecompose* >(this)->impCheckDecompose(); return mfRotate; }
            double getShearX() const { const_cast< B2DHomMatrixBufferedOnDemandDecompose* >(this)->impCheckDecompose(); return mfShearX; }
        };
    } // end of namespace tools

} // end of namespace basegfx

///////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_MATRIX_B2DHOMMATRIXTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
