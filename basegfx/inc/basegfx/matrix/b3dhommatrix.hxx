/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dhommatrix.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 09:54:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#define _BGFX_MATRIX_B3DHOMMATRIX_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef INCLUDED_O3TL_COW_WRAPPER_HXX
#include <o3tl/cow_wrapper.hxx>
#endif

namespace basegfx
{
    class B3DTuple;
    class Impl3DHomMatrix;

    class B3DHomMatrix
    {
    public:
        typedef o3tl::cow_wrapper< Impl3DHomMatrix > ImplType;

    private:
        ImplType                                     mpImpl;

    public:
        B3DHomMatrix();
        B3DHomMatrix(const B3DHomMatrix& rMat);
        ~B3DHomMatrix();

        /// unshare this matrix with all internally shared instances
        void makeUnique();

        double get(sal_uInt16 nRow, sal_uInt16 nColumn) const;
        void set(sal_uInt16 nRow, sal_uInt16 nColumn, double fValue);

        // test if last line is default to see if last line needs to be
        // involved in calculations
        bool isLastLineDefault() const;

        bool isIdentity() const;
        /// Reset to the identity matrix
        void identity();

        bool isInvertible() const;
        /// Invert the matrix (if possible)
        bool invert();

        bool isNormalized() const;
        /// Normalize (i.e. force w=1) the matrix
        void normalize();

        /// Calc the matrix determinant
        double determinant() const;

        /// Calc the matrix trace
        double trace() const;

        /// Transpose the matrix
        void transpose();

        /// Rotation
        void rotate(double fAngleX,double fAngleY,double fAngleZ);

        /// Translation
        void translate(double fX, double fY, double fZ);

        /// Scaling
        void scale(double fX, double fY, double fZ);

        // Shearing-Matrices
        void shearXY(double fSx, double fSy);
        void shearYZ(double fSy, double fSz);
        void shearXZ(double fSx, double fSz);

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

    // addition, subtraction
    inline B3DHomMatrix operator+(const B3DHomMatrix& rMatA, const B3DHomMatrix& rMatB)
    {
        B3DHomMatrix aSum(rMatA);
        aSum += rMatB;
        return aSum;
    }

    inline B3DHomMatrix operator-(const B3DHomMatrix& rMatA, const B3DHomMatrix& rMatB)
    {
        B3DHomMatrix aDiv(rMatA);
        aDiv -= rMatB;
        return aDiv;
    }

    // multiplication, division by constant value
    inline B3DHomMatrix operator*(const B3DHomMatrix& rMat, double fValue)
    {
        B3DHomMatrix aNew(rMat);
        aNew *= fValue;
        return aNew;
    }

    inline B3DHomMatrix operator/(const B3DHomMatrix& rMat, double fValue)
    {
        B3DHomMatrix aNew(rMat);
        aNew *= 1.0 / fValue;
        return aNew;
    }

    inline B3DHomMatrix operator*(const B3DHomMatrix& rMatA, const B3DHomMatrix& rMatB)
    {
        B3DHomMatrix aMul(rMatB);
        aMul *= rMatA;
        return aMul;
    }
} // end of namespace basegfx

#endif /* _BGFX_MATRIX_B3DHOMMATRIX_HXX */
