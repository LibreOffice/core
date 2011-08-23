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

#include "matrix3d.hxx"

#include <tools/debug.hxx>

#include "hmatrix.hxx"

#include "base3d.hxx"

/*************************************************************************
|*
|* Hilfsfunktionen fuer Matrixinvertierung und Determinantenbestimmung
|*
\************************************************************************/

namespace binfilter {
/*************************************************************************
|*
|* Einheitsmatrix herstellen
|*
\************************************************************************/

void Matrix3D::Identity(void)
{
    UINT16 i,j;

    for(i=0;i<3;i++)
    {
        for(j=0;j<3;j++)
        {
            if(i!=j)
                M[i][j] = 0.0;
            else
                M[i][j] = 1.0;
        }
    }
}

/*************************************************************************
|*
|* Rotation mit Winkel (0.0 ... 2PI)
|*
\************************************************************************/

void Matrix3D::Rotate(double fAngle)
{
    Rotate(sin(fAngle),cos(fAngle));
}

/*************************************************************************
|*
|* Rotation mit Sin(),Cos()
|*
\************************************************************************/

void Matrix3D::Rotate(double fSin, double fCos )
{
    Matrix3D aTemp;
    aTemp.M[0][0] = aTemp.M[1][1] = fCos;
    aTemp.M[1][0] = fSin;
    aTemp.M[0][1] = -fSin;
    *this *= aTemp;
}

/*************************************************************************
|*
|* Translationsmatrix aufbauen
|*
\************************************************************************/

void Matrix3D::Translate(double fX, double fY)
{
    Matrix3D aTemp;
    aTemp.M[0][2] = fX;
    aTemp.M[1][2] = fY;
    *this *= aTemp;
}

/*************************************************************************
|*
|* Translationsmatrix aufbauen aus einem Vektor
|*
\************************************************************************/

void Matrix3D::Translate(const Vector2D& rVec)
{
    Translate(rVec.X(), rVec.Y());
}

/*************************************************************************
|*
|* Skalierungsmatrix aufbauen
|*
\************************************************************************/

void Matrix3D::Scale(double fX, double fY)
{
    Matrix3D aTemp;
    aTemp.M[0][0] = fX;
    aTemp.M[1][1] = fY;
    *this *= aTemp;
}

/*************************************************************************
|*
|* Skalierungsmatrix aufbauen aus Vektor
|*
\************************************************************************/

void Matrix3D::Scale(const Vector2D& rVec)
{
    Scale(rVec.X(), rVec.Y());
}

/*************************************************************************
|*
|* Shearing-Matrix for X
|*
\************************************************************************/

void Matrix3D::ShearX(double fSx)
{
    Matrix3D aTemp;
    aTemp.M[0][1] = fSx;
    *this *= aTemp;
}

/*************************************************************************
|*
|* Shearing-Matrix for Y
|*
\************************************************************************/

void Matrix3D::ShearY(double fSy)
{
    Matrix3D aTemp;
    aTemp.M[1][0] = fSy;
    *this *= aTemp;
}

/*************************************************************************
|*
|* Matrixaddition
|*
\************************************************************************/

Matrix3D& Matrix3D::operator+= (const Matrix3D& rMat)
{
    for(UINT16 i=0;i<3;i++)
    {
        for(UINT16 j=0;j<3;j++)
        {
            M[i][j] += rMat[i][j];
        }
    }
    return *this;
}

Matrix3D Matrix3D::operator+ (const Matrix3D& rMat) const
{
    Matrix3D aSum = *this;
    aSum += rMat;
    return aSum;
}

/*************************************************************************
|*
|* Matrixsubtraktion
|*
\************************************************************************/

Matrix3D& Matrix3D::operator-= (const Matrix3D& rMat)
{
    for(UINT16 i=0;i<3;i++)
    {
        for(UINT16 j=0;j<3;j++)
        {
            M[i][j] -= rMat[i][j];
        }
    }
    return *this;
}

Matrix3D Matrix3D::operator- (const Matrix3D& rMat) const
{
    Matrix3D aSum = *this;
    aSum -= rMat;
    return aSum;
}

/*************************************************************************
|*
|* Vergleichsoperatoren
|*
\************************************************************************/

BOOL Matrix3D::operator== (const Matrix3D& rMat) const
{
    for(UINT16 i=0;i<3;i++)
    {
        for(UINT16 j=0;j<3;j++)
        {
            if(M[i][j] != rMat[i][j])
                return FALSE;
        }
    }
    return TRUE;
}

BOOL Matrix3D::operator!=(const Matrix3D& rMat) const
{
    for(UINT16 i=0;i<3;i++)
    {
        for(UINT16 j=0;j<3;j++)
        {
            if(M[i][j] != rMat[i][j])
                return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************************
|*
|* Matrixmultiplikation mit einem Faktor
|*
\************************************************************************/

Matrix3D& Matrix3D::operator*= (double fFactor)
{
    for(UINT16 i=0;i<3;i++)
    {
        for(UINT16 j=0;j<3;j++)
        {
            M[i][j] *= fFactor;
        }
    }
    return *this;
}

Matrix3D Matrix3D::operator* (double fFactor) const
{
    Matrix3D aZwi = *this;
    aZwi *= fFactor;
    return aZwi;
}

/*************************************************************************
|*
|* Matrixdivision durch einen Faktor
|*
\************************************************************************/

Matrix3D& Matrix3D::operator/= (double fFactor)
{
    if(fFactor != 0.0)
    {
        for(UINT16 i=0;i<3;i++)
        {
            for(UINT16 j=0;j<3;j++)
            {
                M[i][j] /= fFactor;
            }
        }
    }
    return *this;
}

Matrix3D Matrix3D::operator/ (double fFactor) const
{
    Matrix3D aZwi = *this;
    aZwi /= fFactor;
    return aZwi;
}

/*************************************************************************
|*
|* Matrixmultiplikation von links, von rechts mittels umgekehrter
|* Parameterangaben zu erreichen
|*
\************************************************************************/

Matrix3D& Matrix3D::operator*= (const Matrix3D& rMat)
{
    // Matrixmultiplikation
    Matrix3D aCopy = *this;
    double fZwi;

    for(UINT16 i=0;i<3;i++)
    {
        for(UINT16 j=0;j<3;j++)
        {
            fZwi = 0.0;
            for(UINT16 k=0;k<3;k++)
            {
                fZwi += aCopy[k][j] * rMat[i][k];
            }
            M[i][j] = fZwi;
        }
    }
    return *this;
}

Matrix3D Matrix3D::operator* (const Matrix3D& rMat) const
{
    Matrix3D aZwi = rMat; // #112587#
    aZwi *= (*this);
    return aZwi;
}

/*************************************************************************
|*
|* Multiplikation Matrix, Punkt
|*
\************************************************************************/

Point3D operator* (const Matrix3D& rMatrix, const Point3D& rPnt)
{
    Point3D aNewPnt;
    UINT16 i,j;
    double fZwi;

    for(i=0;i<3;i++)
    {
        fZwi = 0.0;
        for(j=0;j<3;j++)
        {
            fZwi += rMatrix.M[i][j] * rPnt[j];
        }
        aNewPnt[i] = fZwi;
    }
    return aNewPnt;
}

/*************************************************************************
|*
|* Multiplikation Matrix, Vektor
|*
\************************************************************************/

Vector2D operator* (const Matrix3D& rMatrix, const Vector2D& rVec)
{
    Vector2D aNewVec;
    UINT16 i,j;
    double fZwi;

    for(i=0;i<2;i++)
    {
        fZwi = 0.0;
        for(j=0;j<2;j++)
        {
            fZwi += rMatrix.M[i][j] * rVec[j];
        }
        fZwi += rMatrix.M[i][2];
        aNewVec[i] = fZwi;
    }
    fZwi = rMatrix[2][0] * rVec[0]
         + rMatrix[2][1] * rVec[1]
         + rMatrix[2][2];
    if(fZwi != 1.0 && fZwi != 0.0)
    {
        aNewVec[0] /= fZwi;
        aNewVec[1] /= fZwi;
    }
    return aNewVec;
}

/*************************************************************************
|*
|* Stream-In-Operator fuer Matrix3D
|*
\************************************************************************/

SvStream& operator>>(SvStream& rIStream, Matrix3D& rMatrix3D)
{
    for (int i = 0; i < 3; i++)
        rIStream >> rMatrix3D.M[i];
    return rIStream;
}

/*************************************************************************
|*
|* Stream-Out-Operator fuer Matrix3D
|*
\************************************************************************/

SvStream& operator<<(SvStream& rOStream, const Matrix3D& rMatrix3D)
{
    for (int i = 0; i < 3; i++)
        rOStream << rMatrix3D.M[i];
    return rOStream;
}



/*************************************************************************
|*
|* Help routine to decompose given homogen 3x3 matrix to components. A correction of
|* the components is done to avoid inaccuracies.
|*
\************************************************************************/

BOOL Matrix3D::DecomposeAndCorrect(Vector2D& rScale, 
    double& rShear, double& rRotate, Vector2D& rTranslate) const
{
    // break up homogen 3x3 matrix using homogen 4x4 matrix
    Matrix4D aDecomposeTrans(*this);
    Vector3D aScale;
    Vector3D aShear;
    Vector3D aRotate;
    Vector3D aTranslate;
    if(aDecomposeTrans.Decompose(aScale, aTranslate, aRotate, aShear))
    {
        const double fSmallValue(SMALL_DVALUE);
        
        // handle scale
        if(fabs(aScale.X() - 1.0) < fSmallValue)
            aScale.X() = 1.0;
        if(fabs(aScale.Y() - 1.0) < fSmallValue)
            aScale.Y() = 1.0;
        rScale.X() = aScale.X();
        rScale.Y() = aScale.Y();

        // handle shear
        if(fabs(aShear.X()) < fSmallValue)
            aShear.X() = 0.0;
        rShear = aShear.X();

        // handle rotate
        if(fabs(aRotate.Z()) < fSmallValue)
            aRotate.Z() = 0.0;
        rRotate = aRotate.Z();

        // handle translate
        if(fabs(aTranslate.X()) < fSmallValue)
            aTranslate.X() = 0.0;
        if(fabs(aTranslate.Y()) < fSmallValue)
            aTranslate.Y() = 0.0;
        rTranslate.X() = aTranslate.X();
        rTranslate.Y() = aTranslate.Y();

        return TRUE;
    }
    else
    {
        rScale.X() = rScale.Y() = 10000.0;
        rShear = rRotate = 0.0;
        rTranslate.X() = rTranslate.Y() = 0.0;
        
        return FALSE;
    }
}
}//end of namespace binfilter

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
