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

#pragma once

#include <sal/config.h>

#include <ostream>

#include <sal/types.h>
#include <basegfx/basegfxdllapi.h>
#include <array>

namespace basegfx
{
    class B2DTuple;

    class SAL_WARN_UNUSED BASEGFX_DLLPUBLIC B2DHomMatrix
    {
    private:
        // Since this is a graphics matrix, the last row is always 0 0 1, so we don't bother to store it.
        std::array<std::array<double, 3>, 2> mfValues {
            std::array<double, 3>{ 1.0, 0.0, 0.0 },
            std::array<double, 3>{ 0.0, 1.0, 0.0 } };

    public:
        constexpr B2DHomMatrix() = default;

        /** constructor to allow setting all needed values for a 3x2 matrix at once. The
            parameter f_0x1 e.g. is the same as using set(0, 1, f)
         */
        constexpr B2DHomMatrix(double f_0x0, double f_0x1, double f_0x2, double f_1x0, double f_1x1, double f_1x2)
        {
            mfValues[0][0] = f_0x0;
            mfValues[0][1] = f_0x1;
            mfValues[0][2] = f_0x2;
            mfValues[1][0] = f_1x0;
            mfValues[1][1] = f_1x1;
            mfValues[1][2] = f_1x2;
        }

        /** Convenience creator for declaration of the matrix that is commonly
            used by web standards (SVG, CSS, HTML).

           Values a,b,c,d,e,f represent the following values in the matrix:
              [a,c,e]      [a,c,e]
              [b,d,f]  or  [b,d,f]
                           [0,0,1]

        */
        static B2DHomMatrix abcdef(double da, double db, double dc, double dd, double de, double df)
        {
            return B2DHomMatrix(da, dc, de, db, dd, df);
        }

        // Convenience accessor for value at 0,0 position in the matrix
        double a() const { return get(0,0); }
        // Convenience accessor for value at 1,0 position in the matrix
        double b() const { return get(1,0); }
        // Convenience accessor for value at 0,1 position in the matrix
        double c() const { return get(0,1); }
        // Convenience accessor for value at 1,1 position in the matrix
        double d() const { return get(1,1); }
        // Convenience accessor for value at 0,2 position in the matrix
        double e() const { return get(0,2); }
        // Convenience accessor for value at 1,2 position in the matrix
        double f() const { return get(1,2); }

        double get(sal_uInt16 nRow, sal_uInt16 nColumn) const
        {
            return mfValues[nRow][nColumn];
        }

        void set(sal_uInt16 nRow, sal_uInt16 nColumn, double fValue)
        {
            mfValues[nRow][nColumn] = fValue;
        }

        /** allow setting all needed values for a 3x2 matrix in one call. The
            parameter f_0x1 e.g. is the same as using set(0, 1, f)
         */
        void set3x2(double f_0x0, double f_0x1, double f_0x2, double f_1x0, double f_1x1, double f_1x2);

        // reset to a standard matrix
        bool isIdentity() const;
        void identity();

        bool isInvertible() const;
        bool invert();

        void rotate(double fRadiant);

        void translate(double fX, double fY);
        void translate(const B2DTuple& rTuple);

        void scale(double fX, double fY);
        void scale(const B2DTuple& rTuple);

        // Shearing-Matrices
        void shearX(double fSx);
        void shearY(double fSy);

        bool operator==(const B2DHomMatrix& rMat) const;

        // matrix multiplication from the left to the local
        B2DHomMatrix& operator*=(const B2DHomMatrix& rMat);

        /**
         * Help routine to decompose given homogen 3x3 matrix to components. A correction of the
         * components is done to avoid inaccuracies.
         *
         * See basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix() for the opposite, to
         * compose a homogen 3x3 matrix from components.
         */
        bool decompose(B2DTuple& rScale, B2DTuple& rTranslate, double& rRotate, double& rShearX) const;

    private:
        void computeAdjoint(double (&dst)[6]) const;
        double computeDeterminant(double (&dst)[6]) const;
        void doMulMatrix(const B2DHomMatrix& rMat);
    };

    inline B2DHomMatrix operator*(const B2DHomMatrix& rMatA, const B2DHomMatrix& rMatB)
    {
        B2DHomMatrix aMul(rMatB);
        aMul *= rMatA;
        return aMul;
    }

    template<typename charT, typename traits>
    std::basic_ostream<charT, traits> & operator <<(
        std::basic_ostream<charT, traits> & stream, B2DHomMatrix const & matrix)
    {
        return stream
            << '[' << matrix.get(0, 0) << ' ' << matrix.get(0, 1) << ' '
            << matrix.get(0, 2) << "; " << matrix.get(1, 0) << ' '
            << matrix.get(1, 1) << ' ' << matrix.get(1, 2) << ']';
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
