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

#ifndef INCLUDED_BASEGFX_MATRIX_B3DHOMMATRIX_HXX
#define INCLUDED_BASEGFX_MATRIX_B3DHOMMATRIX_HXX

#include <sal/types.h>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <o3tl/cow_wrapper.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class B3DTuple;
    class Impl3DHomMatrix;

    class BASEGFX_DLLPUBLIC B3DHomMatrix
    {
    public:
        typedef o3tl::cow_wrapper< Impl3DHomMatrix > ImplType;

    private:
        ImplType                                     mpImpl;

    public:
        B3DHomMatrix();
        B3DHomMatrix(const B3DHomMatrix& rMat);
        ~B3DHomMatrix();

        double get(sal_uInt16 nRow, sal_uInt16 nColumn) const;
        void set(sal_uInt16 nRow, sal_uInt16 nColumn, double fValue);

        // test if last line is default to see if last line needs to be
        // involved in calculations
        bool isLastLineDefault() const;

        bool isIdentity() const;
        /// Reset to the identity matrix
        void identity();

        /// Invert the matrix (if possible)
        bool invert();

        /// Calc the matrix determinant
        double determinant() const;

        /// Rotation
        void rotate(double fAngleX,double fAngleY,double fAngleZ);

        /// Translation
        void translate(double fX, double fY, double fZ);

        /// Scaling
        void scale(double fX, double fY, double fZ);

        // Shearing-Matrices
        void shearXY(double fSx, double fSy);
        void shearXZ(double fSx, double fSz);

        // Projection matrices, used for converting between eye and
        // clip coordinates
        void frustum(double fLeft = -1.0, double fRight = 1.0,
            double fBottom = -1.0, double fTop = 1.0,
            double fNear = 0.001, double fFar = 1.0);

        void ortho(double fLeft = -1.0, double fRight = 1.0,
            double fBottom = -1.0, double fTop = 1.0,
            double fNear = 0.0, double fFar = 1.0);

        // build orientation matrix
        void orientation(
            const B3DPoint& rVRP = B3DPoint(0.0,0.0,1.0),
            B3DVector aVPN = B3DVector(0.0,0.0,1.0),
            B3DVector aVUV = B3DVector(0.0,1.0,0.0));

        // addition, subtraction
        B3DHomMatrix& operator+=(const B3DHomMatrix& rMat);
        B3DHomMatrix& operator-=(const B3DHomMatrix& rMat);

        // comparison
        bool operator==(const B3DHomMatrix& rMat) const;
        bool operator!=(const B3DHomMatrix& rMat) const;

        // multiplication, division by constant value
        B3DHomMatrix& operator*=(double fValue);
        B3DHomMatrix& operator/=(double fValue);

        // matrix multiplication (from the left)
        B3DHomMatrix& operator*=(const B3DHomMatrix& rMat);

        // assignment operator
        B3DHomMatrix& operator=(const B3DHomMatrix& rMat);

        // decomposition
        bool decompose(B3DTuple& rScale, B3DTuple& rTranslate, B3DTuple& rRotate, B3DTuple& rShear) const;
    };

    inline B3DHomMatrix operator*(const B3DHomMatrix& rMatA, const B3DHomMatrix& rMatB)
    {
        B3DHomMatrix aMul(rMatB);
        aMul *= rMatA;
        return aMul;
    }
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_MATRIX_B3DHOMMATRIX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
