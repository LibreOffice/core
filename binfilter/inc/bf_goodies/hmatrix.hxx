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

#ifndef _B3D_HMATRIX_HXX
#define _B3D_HMATRIX_HXX

#include "point4d.hxx"

namespace binfilter {

//////////////////////////////////////////////////////////////////////////
// external declarations
class Matrix3D;

/*************************************************************************
|*
|* homogene 4x4 matrix
|*
\************************************************************************/

class Matrix4D
{
private:
    // Hilfsfunktionen fuer Matrixinvertierung und Determinantenbestimmung
    BOOL Ludcmp(UINT16 nIndex[], INT16& nParity);
    void Lubksb(UINT16 nIndex[], Point4D& aVec);

protected:
    Point4D		M[4];

public:
    // default: Einheitsmatrix erstellen (Vektoren sind auf 0
    // initialisiert, der recte Spaltenvektor auf 1)
    Matrix4D() { M[0][0] = M[1][1] = M[2][2] = 1.0;
                 M[0][3] = M[1][3] = M[2][3] = 0.0; }

    // constructor using homogen 3x3 matrix
    Matrix4D(const Matrix3D& rMat);

    // set to given homogen 3x3 matrix
    //void SetMatrix(const Matrix3D& rMat);

    // Zeilenvektor zurueckgeben
    Point4D& operator[](int nPos) { return M[nPos]; }
    const Point4D& operator[](int nPos) const { return M[nPos]; }

    // Spaltenvektor zurueckgeben
    Point4D GetColumnVector(int nCol) const
        { return Point4D(M[0][nCol], M[1][nCol], M[2][nCol], M[3][nCol]); }

    // Auf Einheitsmatrix zuruecksetzen
    void Identity(void);

    // Invertierung
    BOOL Invert();

    // Zerlegung
    BOOL Decompose(Vector3D& rScale, Vector3D& rTranslate, Vector3D& rRotate, Vector3D& rShear) const;

    // Determinante
    double Determinant();

    // Transpose
    void Transpose();

    // Rotation
    void RotateX(double fAngle);
    void RotateY(double fAngle);
    void RotateZ(double fAngle);
    void RotateX(double fSin, double fCos );
    void RotateY(double fSin, double fCos );
    void RotateZ(double fSin, double fCos );

    // Translation
    void Translate(double fX, double fY, double fZ );
    void Translate(const Vector3D& aTrans);
    void TranslateZ(double fValue);

    // Skalierung
    void Scale(double fX, double fY, double fZ);
    void Scale(const Vector3D& aScale);

    // ModelViewMatrix (ViewOrientationMatrix) fuer die Umwandlung
    // ObjectCoordinates in EyeCoordinates
#if defined( ICC ) || defined( GCC )
    void Orientation(Point4D aVRP = Point4D(0.0,0.0,1.0),
        Vector3D aVPN = Vector3D(0.0,0.0,1.0),
        Vector3D aVUP = Vector3D(0.0,1.0,0.0));
#else
    void Orientation(Point4D& aVRP = Point4D(0.0,0.0,1.0),
        Vector3D& aVPN = Vector3D(0.0,0.0,1.0),
        Vector3D& aVUP = Vector3D(0.0,1.0,0.0));
#endif  
    // Projektionsmatritzen fuer die Umwandlung von EyeCoordinates
    // auf ClipCoordinates
    void Frustum(double fLeft = -1.0, double fRight = 1.0,
        double fBottom = -1.0, double fTop = 1.0,
        double fNear = 0.001, double fFar = 1.0);
    void Ortho(double fLeft = -1.0, double fRight = 1.0,
        double fBottom = -1.0, double fTop = 1.0,
        double fNear = 0.0, double fFar = 1.0);

    // Addition, Subtraktion
    Matrix4D&	operator+=	(const Matrix4D&);
    Matrix4D&	operator-=	(const Matrix4D&);
    Matrix4D	operator+ 	(const Matrix4D&) const;
    Matrix4D	operator-	(const Matrix4D&) const;

    // Vergleichsoperatoren
    BOOL		operator==	(const Matrix4D&) const;
    BOOL		operator!=	(const Matrix4D&) const;

    // Multiplikation, Division mit Konstante
    Matrix4D&	operator*=	(double);
    Matrix4D	operator*	(double) const;
    Matrix4D&	operator/=	(double);
    Matrix4D	operator/	(double) const;

    // Matritzenmultiplikation von links auf die lokale
    Matrix4D&	operator*=	(const Matrix4D&);
    Matrix4D	operator*	(const Matrix4D&) const;

    // Operatoren zur Punkttransformation
    friend Point4D	operator*	(const Matrix4D&, const Point4D&);
    friend Point4D&	operator*=	(Point4D& rPnt, const Matrix4D& rMat)
        { return (rPnt = rMat * rPnt); }

    // Operatoren zur Vektortransformation
    friend Vector3D		operator*	(const Matrix4D&, const Vector3D&);
    friend Vector3D&	operator*=	(Vector3D& rVec, const Matrix4D& rMat)
        { return (rVec = rMat * rVec); }

    // Streamoperatoren
    friend SvStream& operator>>(SvStream& rIStream, Matrix4D&);
    friend SvStream& operator<<(SvStream& rOStream, const Matrix4D&);
};
}//end of namespace binfilter


#endif          // _B3D_HMATRIX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
