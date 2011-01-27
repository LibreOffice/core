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

#ifndef _BGFX_MATRIX_B2DHOMMATRIXTOOLS_HXX
#define _BGFX_MATRIX_B2DHOMMATRIXTOOLS_HXX

#include <sal/types.h>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dvector.hxx>

namespace rtl { class OUString; }

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
        void createSinCosOrthogonal(double& o_rSin, double& rCos, double fRadiant);

        /** Tooling methods for on-the-fly matrix generation e.g. for inline
            multiplications
         */
        B2DHomMatrix createScaleB2DHomMatrix(double fScaleX, double fScaleY);
        B2DHomMatrix createShearXB2DHomMatrix(double fShearX);
        B2DHomMatrix createShearYB2DHomMatrix(double fShearY);
        B2DHomMatrix createRotateB2DHomMatrix(double fRadiant);
        B2DHomMatrix createTranslateB2DHomMatrix(double fTranslateX, double fTranslateY);

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
        B2DHomMatrix createScaleShearXRotateTranslateB2DHomMatrix(
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

        B2DHomMatrix createShearXRotateTranslateB2DHomMatrix(
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

        B2DHomMatrix createScaleTranslateB2DHomMatrix(
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
        B2DHomMatrix createRotateAroundPoint(
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

    } // end of namespace tools
} // end of namespace basegfx

///////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        class B2DHomMatrixBufferedDecompose
        {
        private:
            B2DVector              maScale;
            B2DVector              maTranslate;
            double                 mfRotate;
            double                 mfShearX;

        public:
            B2DHomMatrixBufferedDecompose(const B2DHomMatrix& rB2DHomMatrix)
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
        class B2DHomMatrixBufferedOnDemandDecompose
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
            B2DHomMatrixBufferedOnDemandDecompose(const B2DHomMatrix& rB2DHomMatrix)
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

    /// Returns a string with svg's "matrix(m00,m10,m01,m11,m02,m12)" representation
    ::rtl::OUString exportToSvg( const B2DHomMatrix& rMatrix );

} // end of namespace basegfx

///////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_MATRIX_B2DHOMMATRIXTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
