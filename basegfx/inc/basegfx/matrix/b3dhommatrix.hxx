/*************************************************************************
 *
 *  $RCSfile: b3dhommatrix.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-05 12:25:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#define _BGFX_MATRIX_B3DHOMMATRIX_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

// predeclaration
class Impl3DHomMatrix;

namespace basegfx
{
    namespace tuple
    {
        // predeclaration
        class B3DTuple;
    } // end of namespace tuple

    namespace matrix
    {
        class B3DHomMatrix
        {
        private:
            Impl3DHomMatrix*                            mpM;

            void implPrepareChange();

        public:
            B3DHomMatrix();
            B3DHomMatrix(const B3DHomMatrix& rMat);
            ~B3DHomMatrix();

            double get(sal_uInt16 nRow, sal_uInt16 nColumn) const;
            void set(sal_uInt16 nRow, sal_uInt16 nColumn, double fValue);

            sal_Bool isIdentity() const;
            /// Reset to the identity matrix
            void identity();

            sal_Bool isInvertible() const;
            /// Invert the matrix (if possible)
            sal_Bool invert();

            sal_Bool isNormalized() const;
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

            // Projection matrices, used for converting between eye and
            // clip coordinates
            void frustum(double fLeft = -1.0, double fRight = 1.0,
                double fBottom = -1.0, double fTop = 1.0,
                double fNear = 0.001, double fFar = 1.0);

            void ortho(double fLeft = -1.0, double fRight = 1.0,
                double fBottom = -1.0, double fTop = 1.0,
                double fNear = 0.0, double fFar = 1.0);

            // addition, subtraction
            B3DHomMatrix& operator+=(const B3DHomMatrix& rMat);
            B3DHomMatrix& operator-=(const B3DHomMatrix& rMat);

            // comparison
            sal_Bool operator==(const B3DHomMatrix& rMat) const;
            sal_Bool operator!=(const B3DHomMatrix& rMat) const;

            // multiplication, division by constant value
            B3DHomMatrix& operator*=(double fValue);
            B3DHomMatrix& operator/=(double fValue);

            // matrix multiplication (from the left)
            B3DHomMatrix& operator*=(const B3DHomMatrix& rMat);

            // assignment operator
            B3DHomMatrix& operator=(const B3DHomMatrix& rMat);

            // decomposition
            sal_Bool decompose(tuple::B3DTuple& rScale, tuple::B3DTuple& rTranslate, tuple::B3DTuple& rRotate, tuple::B3DTuple& rShear) const;
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
    } // end of namespace matrix
} // end of namespace basegfx

#endif // _BGFX_MATRIX_B3DHOMMATRIX_HXX
