/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dhommatrix.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:25:05 $
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

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#define _BGFX_MATRIX_B2DHOMMATRIX_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace basegfx
{
    // predeclaration
    class B2DTuple;

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
