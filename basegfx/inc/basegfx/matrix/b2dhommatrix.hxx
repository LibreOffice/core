/*************************************************************************
 *
 *  $RCSfile: b2dhommatrix.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: thb $ $Date: 2003-11-12 12:12:40 $
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

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#define _BGFX_MATRIX_B2DHOMMATRIX_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace basegfx
{
    namespace tuple
    {
        // predeclaration
        class B2DTuple;
    } // end of namespace tuple

    namespace matrix
    {
        // forward declaration
        class Impl2DHomMatrix;

        class B2DHomMatrix
        {
        private:
            Impl2DHomMatrix*                            mpM;

            void implPrepareChange();

        public:
            B2DHomMatrix();
            B2DHomMatrix(const B2DHomMatrix& rMat);
            ~B2DHomMatrix();

            double get(sal_uInt16 nRow, sal_uInt16 nColumn) const;
            void set(sal_uInt16 nRow, sal_uInt16 nColumn, double fValue);

            // Auf Einheitsmatrix zuruecksetzen
            sal_Bool isIdentity() const;
            void identity();

            // Invertierung
            sal_Bool isInvertible() const;
            sal_Bool invert();

            // Normalisierung
            sal_Bool isNormalized() const;
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
            sal_Bool operator==(const B2DHomMatrix& rMat) const;
            sal_Bool operator!=(const B2DHomMatrix& rMat) const;

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
            sal_Bool decompose(tuple::B2DTuple& rScale, tuple::B2DTuple& rTranslate, double& rRotate, double& rShearX) const;
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
    } // end of namespace matrix
} // end of namespace basegfx

#endif // _BGFX_MATRIX_B2DHOMMATRIX_HXX
