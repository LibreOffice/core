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

#ifndef INCLUDED_BASEGFX_MATRIX_B2DHOMMATRIX_HXX
#define INCLUDED_BASEGFX_MATRIX_B2DHOMMATRIX_HXX

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class B2DTuple;
    class Impl2DHomMatrix;

    class BASEGFX_DLLPUBLIC B2DHomMatrix
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

        double get(sal_uInt16 nRow, sal_uInt16 nColumn) const;
        void set(sal_uInt16 nRow, sal_uInt16 nColumn, double fValue);

        /** allow setting all needed values for a 3x2 matrix in one call. The
            parameter f_0x1 e.g. is the same as using set(0, 1, f)
         */
        void set3x2(double f_0x0, double f_0x1, double f_0x2, double f_1x0, double f_1x1, double f_1x2);

        // test if last line is default to see if last line needs to be
        // involved in calculations
        bool isLastLineDefault() const;

        // reset to a standard matrix
        bool isIdentity() const;
        void identity();

        bool isInvertible() const;
        bool invert();

        void rotate(double fRadiant);

        void translate(double fX, double fY);

        void scale(double fX, double fY);

        // Shearing-Matrices
        void shearX(double fSx);
        void shearY(double fSy);

        B2DHomMatrix& operator+=(const B2DHomMatrix& rMat);
        B2DHomMatrix& operator-=(const B2DHomMatrix& rMat);

        bool operator==(const B2DHomMatrix& rMat) const;
        bool operator!=(const B2DHomMatrix& rMat) const;

        B2DHomMatrix& operator*=(double fValue);
        B2DHomMatrix& operator/=(double fValue);

        // matrix multiplication from the left to the local
        B2DHomMatrix& operator*=(const B2DHomMatrix& rMat);

        // assignment operator
        B2DHomMatrix& operator=(const B2DHomMatrix& rMat);

        // Help routine to decompose given homogen 3x3 matrix to components. A correction of
        // the components is done to avoid inaccuracies.
        bool decompose(B2DTuple& rScale, B2DTuple& rTranslate, double& rRotate, double& rShearX) const;
    };

    inline B2DHomMatrix operator*(const B2DHomMatrix& rMatA, const B2DHomMatrix& rMatB)
    {
        B2DHomMatrix aMul(rMatB);
        aMul *= rMatA;
        return aMul;
    }
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_MATRIX_B2DHOMMATRIX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
