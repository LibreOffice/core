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

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#define _BGFX_MATRIX_B2DHOMMATRIX_HXX

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>

namespace basegfx
{
    class B2DTuple;
    class Impl2DHomMatrix;

    class B2DHomMatrix
    {
    public:
        typedef o3tl::cow_wrapper< Impl2DHomMatrix > ImplType;

    private:
        ImplType                                     mpImpl;

    public:
        B2DHomMatrix();
        B2DHomMatrix(const B2DHomMatrix& rMat);
        ~B2DHomMatrix();

        /** constructor to allow setting all needed values for a 3x2 matrix at once. The
            parameter f_0x1 e.g. is the same as using set(0, 1, f)
         */
        B2DHomMatrix(double f_0x0, double f_0x1, double f_0x2, double f_1x0, double f_1x1, double f_1x2);

        /// unshare this matrix with all internally shared instances
        void makeUnique();

        double get(sal_uInt16 nRow, sal_uInt16 nColumn) const;
        void set(sal_uInt16 nRow, sal_uInt16 nColumn, double fValue);

        /** allow setting all needed values for a 3x2 matrix in one call. The
            parameter f_0x1 e.g. is the same as using set(0, 1, f)
         */
        void set3x2(double f_0x0, double f_0x1, double f_0x2, double f_1x0, double f_1x1, double f_1x2);

        // test if last line is default to see if last line needs to be
        // involved in calculations
        bool isLastLineDefault() const;

        // Auf Einheitsmatrix zuruecksetzen
        bool isIdentity() const;
        void identity();

        // Invertierung
        bool isInvertible() const;
        bool invert();

        // Normalisierung
        bool isNormalized() const;
        void normalize();

        // Determinante
        double determinant() const;

        // Trace
        double trace() const;

        // Transpose
        void transpose();

        // Rotation
        void rotate(double fRadiant);

        // Translation
        void translate(double fX, double fY);

        // Skalierung
        void scale(double fX, double fY);

        // Shearing-Matrices
        void shearX(double fSx);
        void shearY(double fSy);

        // Addition, Subtraktion
        B2DHomMatrix& operator+=(const B2DHomMatrix& rMat);
        B2DHomMatrix& operator-=(const B2DHomMatrix& rMat);

        // Vergleichsoperatoren
        bool operator==(const B2DHomMatrix& rMat) const;
        bool operator!=(const B2DHomMatrix& rMat) const;

        // Multiplikation, Division mit Konstante
        B2DHomMatrix& operator*=(double fValue);
        B2DHomMatrix& operator/=(double fValue);

        // Matritzenmultiplikation von links auf die lokale
        B2DHomMatrix& operator*=(const B2DHomMatrix& rMat);

        // assignment operator
        B2DHomMatrix& operator=(const B2DHomMatrix& rMat);

        // Help routine to decompose given homogen 3x3 matrix to components. A correction of
        // the components is done to avoid inaccuracies.
        // Zerlegung
        bool decompose(B2DTuple& rScale, B2DTuple& rTranslate, double& rRotate, double& rShearX) const;
    };

    // Addition, Subtraktion
    inline B2DHomMatrix operator+(const B2DHomMatrix& rMatA, const B2DHomMatrix& rMatB)
    {
        B2DHomMatrix aSum(rMatA);
        aSum += rMatB;
        return aSum;
    }

    inline B2DHomMatrix operator-(const B2DHomMatrix& rMatA, const B2DHomMatrix& rMatB)
    {
        B2DHomMatrix aDiv(rMatA);
        aDiv -= rMatB;
        return aDiv;
    }

    // Multiplikation, Division mit Konstante
    inline B2DHomMatrix operator*(const B2DHomMatrix& rMat, double fValue)
    {
        B2DHomMatrix aNew(rMat);
        aNew *= fValue;
        return aNew;
    }

    inline B2DHomMatrix operator/(const B2DHomMatrix& rMat, double fValue)
    {
        B2DHomMatrix aNew(rMat);
        aNew *= 1.0 / fValue;
        return aNew;
    }

    inline B2DHomMatrix operator*(const B2DHomMatrix& rMatA, const B2DHomMatrix& rMatB)
    {
        B2DHomMatrix aMul(rMatB);
        aMul *= rMatA;
        return aMul;
    }
} // end of namespace basegfx

#endif /* _BGFX_MATRIX_B2DHOMMATRIX_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
