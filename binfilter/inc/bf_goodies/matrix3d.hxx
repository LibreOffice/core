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

#ifndef _B2D_MATRIX3D_HXX
#define _B2D_MATRIX3D_HXX

#ifndef _B2D_POINT3D_HXX
#include "point3d.hxx"
#endif

/*************************************************************************
|*
|* homogene 4x4 matrix
|*
\************************************************************************/

namespace binfilter {
class Matrix3D
{
protected:
    Point3D						M[3];

public:
    // default: Einheitsmatrix erstellen (Vektoren sind auf 0
    // initialisiert, der recte Spaltenvektor auf 1)
    Matrix3D() { M[0][0] = M[1][1] = 1.0;
                 M[0][2] = M[1][2] = 0.0; }

    // Zeilenvektor zurueckgeben
    Point3D& operator[](int nPos) { return M[nPos]; }
    const Point3D& operator[](int nPos) const { return M[nPos]; }

    // Spaltenvektor zurueckgeben
    Point3D GetColumnVector(int nCol) const
        { return Point3D(M[0][nCol], M[1][nCol], M[2][nCol]); }

    // Auf Einheitsmatrix zuruecksetzen
    void Identity(void);

    // Rotation
    void Rotate(double fAngle);
    void Rotate(double fSin, double fCos );

    // Translation
    void Translate(double fX, double fY);
    void Translate(const Vector2D& aTrans);

    // Skalierung
    void Scale(double fX, double fY);
    void Scale(const Vector2D& aScale);

    // Shearing-Matrices
    void ShearX(double fSx);
    void ShearY(double fSy);

    // Addition, Subtraktion
    Matrix3D&	operator+=	(const Matrix3D&);
    Matrix3D&	operator-=	(const Matrix3D&);
    Matrix3D	operator+ 	(const Matrix3D&) const;
    Matrix3D	operator-	(const Matrix3D&) const;

    // Vergleichsoperatoren
    BOOL		operator==	(const Matrix3D&) const;
    BOOL		operator!=	(const Matrix3D&) const;

    // Multiplikation, Division mit Konstante
    Matrix3D&	operator*=	(double);
    Matrix3D	operator*	(double) const;
    Matrix3D&	operator/=	(double);
    Matrix3D	operator/	(double) const;

    // Matritzenmultiplikation von links auf die lokale
    Matrix3D&	operator*=	(const Matrix3D&);
    Matrix3D	operator*	(const Matrix3D&) const;

    // Operatoren zur Punkttransformation
    friend Point3D	operator*	(const Matrix3D&, const Point3D&);
    friend Point3D&	operator*=	(Point3D& rPnt, const Matrix3D& rMat)
        { return (rPnt = rMat * rPnt); }

    // Operatoren zur Vektortransformation
    friend Vector2D		operator*	(const Matrix3D&, const Vector2D&);
    friend Vector2D&	operator*=	(Vector2D& rVec, const Matrix3D& rMat)
        { return (rVec = rMat * rVec); }

    // Streamoperatoren
    friend SvStream& operator>>(SvStream& rIStream, Matrix3D&);
    friend SvStream& operator<<(SvStream& rOStream, const Matrix3D&);

    // Help routine to decompose given homogen 3x3 matrix to components. A correction of
    // the components is done to avoid inaccuracies.
    BOOL DecomposeAndCorrect(Vector2D& rScale, double& rShear, double& rRotate, Vector2D& rTranslate) const;
};
}//end of namespace binfilter

#endif          // _B2D_MATRIX3D_HXX
