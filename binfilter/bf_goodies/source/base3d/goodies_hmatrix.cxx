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
#include "hmatrix.hxx"
#endif

#ifndef _B3D_BASE3D_HXX
#include "base3d.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _B2D_MATRIX3D_HXX
#include "matrix3d.hxx"
#endif

namespace binfilter {
/*************************************************************************
|*
|* constructor using homogen 3x3 matrix
|*
\************************************************************************/
 
Matrix4D::Matrix4D(const Matrix3D& rMat)
{
    // vectors are initialized to 0.0, right column to 1.0
    // set not-yet right inited values except to be copied ones
    M[2][2] = 1.0;
    M[2][3] = 0.0;

    // copy upper left 2x2
    M[0][0] = rMat[0][0];
    M[0][1] = rMat[0][1];
    M[1][0] = rMat[1][0];
    M[1][1] = rMat[1][1];

    // copy translate
    M[0][3] = rMat[0][2];
    M[1][3] = rMat[1][2];
}

/*************************************************************************
|*
|* set to given homogen 3x3 matrix
|*
\************************************************************************/
 
//void Matrix4D::SetMatrix(const Matrix3D& rMat)
//{
//	// reset non-overwritten parts
//	M[0][2] = M[1][2] = M[2][0] = M[2][1] = M[2][2] = M[3][0] = M[3][1] = M[3][2] = 0.0;
//	M[2][2] = M[3][3] = 1.0;
//
//	// copy upper left 2x2
//	M[0][0] = rMat[0][0];
//	M[0][1] = rMat[0][1];
//	M[1][0] = rMat[1][0];
//	M[1][1] = rMat[1][1];
//
//	// copy translate
//	M[0][3] = rMat[0][2];
//	M[1][3] = rMat[1][2];
//}

/*************************************************************************
|*
|* Hilfsfunktionen fuer Matrixinvertierung und Determinantenbestimmung
|*
\************************************************************************/

BOOL Matrix4D::Ludcmp(UINT16 nIndex[], INT16& nParity)
{
    double fBig;
    double fTemp;
    double fStorage[4];
    double fSum;
    double fDum;
    UINT16 i,j,k,imax = 0;

    nParity = 1;

    // Ermittle das Maximum jeder Zeile. Falls eine Zeile
    // leer ist, breche ab. Matrix ist dann nicht invertierbar.
    for(i=0;i<4;i++)
    {
        fBig = 0.0;
        for(j=0;j<4;j++)
        {
            fTemp = fabs(M[i][j]);
            if(fTemp > fBig)
                fBig = fTemp;
        }
        if(fBig == 0.0)
            return FALSE;
        fStorage[i] = 1.0 / fBig;
    }
    // beginne mit dem normalisieren
    for(j=0;j<4;j++)
    {
        for(i=0;i<j;i++)
        {
            fSum = M[i][j];
            for(k=0;k<i;k++)
            {
                fSum -= M[i][k] * M[k][j];
            }
            M[i][j] = fSum;
        }
        fBig = 0.0;
        for(i=j;i<4;i++)
        {
            fSum = M[i][j];
            for(k=0L;k<j;k++)
            {
                fSum -= M[i][k] * M[k][j];
            }
            M[i][j] = fSum;
            fDum = fStorage[i] * fabs(fSum);
            if(fDum >= fBig)
            {
                fBig = fDum;
                imax = i;
            }
        }
        if(j != imax)
        {
            for(k=0;k<4;k++)
            {
                fDum = M[imax][k];
                M[imax][k] = M[j][k];
                M[j][k] = fDum;
            }
            nParity = -nParity;
            fStorage[imax] = fStorage[j];
        }
        nIndex[j] = imax;

        // here the failure of precision occurs
        if(fabs(M[j][j]) == 0.0)
            return FALSE;

        if(j != 3)
        {
            fDum = 1.0 / M[j][j];
            for(i=j+1;i<4;i++)
            {
                M[i][j] *= fDum;
            }
        }
    }
    return TRUE;
}

/*************************************************************************
|*
|* Hilfsfunktionen fuer Matrixinvertierung und Determinantenbestimmung
|*
\************************************************************************/

void Matrix4D::Lubksb(UINT16 nIndex[], Point4D& rPnt)
{
    UINT16 j,ip;
    INT16 i,ii = -1;
    double fSum;

    for(i=0;i<4;i++)
    {
        ip = nIndex[i];
        fSum = rPnt[ip];
        rPnt[ip] = rPnt[i];
        if(ii >= 0)
        {
            for(j=ii;j<i;j++)
            {
                fSum -= M[i][j] * rPnt[j];
            }
        }
        else if(fSum != 0.0)
        {
            ii = i;
        }
        rPnt[i] = fSum;
    }
    for(i=3;i>=0;i--)
    {
        fSum = rPnt[i];
        for(j=i+1;j<4;j++)
        {
            fSum -= M[i][j] * rPnt[j];
        }
        if(M[i][i] != 0.0)
            rPnt[i] = fSum / M[i][i];
    }
}

/*************************************************************************
|*
|* Einheitsmatrix herstellen
|*
\************************************************************************/

void Matrix4D::Identity(void)
{
    UINT16 i,j;

    for(i=0;i<4;i++)
    {
        for(j=0;j<4;j++)
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
|* Inverse Matrix bilden
|*
\************************************************************************/

BOOL Matrix4D::Invert()
{
    Matrix4D aWork = *this;
    UINT16 nIndex[4];
    INT16 nParity;
    UINT16 i;

    if(!aWork.Ludcmp(nIndex, nParity))
        return FALSE;

    Matrix4D aInverse;
    for(i=0;i<4;i++)
    {
        // Zeile expandieren
        aWork.Lubksb(nIndex, aInverse[i]);
    }
    // transponieren
    aInverse.Transpose();

    // kopieren
    *this = aInverse;

    return TRUE;
}

/*************************************************************************
|*
|* Liefert die Determinante dieser Matrix
|*
\************************************************************************/

double Matrix4D::Determinant()
{
    Matrix4D aWork = *this;
    UINT16 nIndex[4];
    INT16 nParity;
    UINT16 i;
    double fRetval = 0.0;

    if(aWork.Ludcmp(nIndex, nParity))
    {
        fRetval = (double)nParity;
        for(i=0;i<4;i++)
            fRetval *= aWork[i][i];
    }
    return fRetval;
}

/*************************************************************************
|*
|* Transponiert diese Matrix
|*
\************************************************************************/

void Matrix4D::Transpose()
{
    UINT16 i,j;
    double fTemp;

    for(i=0;i<3;i++)
    {
        for(j=i+1;j<4;j++)
        {
            fTemp = M[i][j];
            M[i][j] = M[j][i];
            M[j][i] = fTemp;
        }
    }
}

/*************************************************************************
|*
|* Rotation um X-Achse mit Winkel (0.0 ... 2PI)
|*
\************************************************************************/

void Matrix4D::RotateX(double fAngle)
{
    RotateX(sin(fAngle),cos(fAngle));
}

/*************************************************************************
|*
|* Rotation um Y-Achse mit Winkel (0.0 ... 2PI)
|*
\************************************************************************/

void Matrix4D::RotateY(double fAngle)
{
    RotateY(sin(fAngle),cos(fAngle));
}

/*************************************************************************
|*
|* Rotation um Z-Achse mit Winkel (0.0 ... 2PI)
|*
\************************************************************************/

void Matrix4D::RotateZ(double fAngle)
{
    RotateZ(sin(fAngle),cos(fAngle));
}

/*************************************************************************
|*
|* Rotation um X-Achse mit Sin(),Cos()
|*
\************************************************************************/

void Matrix4D::RotateX(double fSin, double fCos )
{
    Matrix4D aTemp;
    aTemp.M[1][1] = aTemp.M[2][2] = fCos;
    aTemp.M[2][1] = fSin;
    aTemp.M[1][2] = -fSin;
    *this *= aTemp;
}

/*************************************************************************
|*
|* Rotation um Y-Achse mit Sin(),Cos()
|*
\************************************************************************/

void Matrix4D::RotateY(double fSin, double fCos )
{
    Matrix4D aTemp;
    aTemp.M[0][0] = aTemp.M[2][2] = fCos;
    aTemp.M[0][2] = fSin;
    aTemp.M[2][0] = -fSin;
    *this *= aTemp;
}

/*************************************************************************
|*
|* Rotation um Z-Achse mit Sin(),Cos()
|*
\************************************************************************/

void Matrix4D::RotateZ(double fSin, double fCos )
{
    Matrix4D aTemp;
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

void Matrix4D::Translate(double fX, double fY, double fZ )
{
    Matrix4D aTemp;
    aTemp.M[0][3] = fX;
    aTemp.M[1][3] = fY;
    aTemp.M[2][3] = fZ;
    *this *= aTemp;
}

/*************************************************************************
|*
|* Translationsmatrix aufbauen aus einem Vektor
|*
\************************************************************************/

void Matrix4D::Translate(const Vector3D& rVec)
{
    Translate(rVec.X(),rVec.Y(),rVec.Z());
}

/*************************************************************************
|*
|* Translationsmatrix nur in Z
|*
\************************************************************************/

void Matrix4D::TranslateZ(double fValue)
{
    Matrix4D aTemp;
    aTemp.M[2][3] = fValue;
    *this *= aTemp;
}

/*************************************************************************
|*
|* Skalierungsmatrix aufbauen
|*
\************************************************************************/

void Matrix4D::Scale(double fX, double fY, double fZ)
{
    Matrix4D aTemp;
    aTemp.M[0][0] = fX;
    aTemp.M[1][1] = fY;
    aTemp.M[2][2] = fZ;
    *this *= aTemp;
}

/*************************************************************************
|*
|* Skalierungsmatrix aufbauen aus Vektor
|*
\************************************************************************/

void Matrix4D::Scale(const Vector3D& rVec)
{
    Scale(rVec.X(),rVec.Y(),rVec.Z());
}

/*************************************************************************
|*
|* Decomposition of a Matrix
|* NO perspective Information allowed here!
|*
\************************************************************************/

BOOL Matrix4D::Decompose(
    Vector3D& rScale, Vector3D& rTranslate,
    Vector3D& rRotate, Vector3D& rShear) const
{
    BOOL bSuccess(FALSE);

    // NO perspective Infos!
    if(M[3][0] == 0.0 && M[3][1] == 0.0 && M[3][2] == 0.0)
    {
        // Determinante darf nict NULL sein
        if(((Matrix4D*)this)->Determinant() != 0.0)
        {
            // Translation isolieren
            rTranslate[0] = M[0][3];
            rTranslate[1] = M[1][3];
            rTranslate[2] = M[2][3];

            // Skalierung und Shear holen
            Vector3D aCol0(M[0][0], M[1][0], M[2][0]);
            Vector3D aCol1(M[0][1], M[1][1], M[2][1]);
            Vector3D aCol2(M[0][2], M[1][2], M[2][2]);
            Vector3D aZwi;

            // ScaleX holen
            rScale[0] = aCol0.GetLength();
            aCol0.Normalize();

            // ShearXY holen
            rShear[0] = aCol0.Scalar(aCol1);
            if(fabs(rShear[0]) > SMALL_DVALUE)
            {
                aZwi[0] = aCol1[0] - rShear[0] * aCol0[0];
                aZwi[1] = aCol1[1] - rShear[0] * aCol0[1];
                aZwi[2] = aCol1[2] - rShear[0] * aCol0[2];
                aCol1 = aZwi;
            }
            else
            {
                rShear[0] = 0.0;
            }

            // Scale Y holen
            rScale[1] = aCol1.GetLength();
            aCol1.Normalize();

            if(rShear[0] != 0.0)
                rShear[0] /= rScale[1];

            // Shear XZ holen
            rShear[1] = aCol0.Scalar(aCol2);
            if(fabs(rShear[1]) > SMALL_DVALUE)
            {
                aZwi[0] = aCol2[0] - rShear[1] * aCol0[0];
                aZwi[1] = aCol2[1] - rShear[1] * aCol0[1];
                aZwi[2] = aCol2[2] - rShear[1] * aCol0[2];
                aCol2 = aZwi;
            }
            else
            {
                rShear[1] = 0.0;
            }

            // Shear YZ holen
            rShear[2] = aCol1.Scalar(aCol2);
            if(fabs(rShear[2]) > SMALL_DVALUE)
            {
                aZwi[0] = aCol2[0] - rShear[2] * aCol1[0];
                aZwi[1] = aCol2[1] - rShear[2] * aCol1[1];
                aZwi[2] = aCol2[2] - rShear[2] * aCol1[2];
                aCol2 = aZwi;
            }
            else
            {
                rShear[2] = 0.0;
            }

            // Scale Z holen
            rScale[2] = aCol2.GetLength();
            aCol2.Normalize();

            if(rShear[1] != 0.0)
                rShear[1] /= rScale[2];

            if(rShear[2] != 0.0)
                rShear[2] /= rScale[2];

            // Coordinate system flip?
            if(aCol0.Scalar(aCol1|aCol2) < 0.0)
            {
                rScale = -rScale;
                aCol0 = -aCol0;
                aCol1 = -aCol1;
                aCol2 = -aCol2;
            }

            // Rotationen holen
            rRotate[1] = asin(-aCol0[2]);

            if(fabs(cos(rRotate[1])) > SMALL_DVALUE)
            {
                rRotate[0] = atan2(aCol1[2], aCol2[2]);
                rRotate[2] = atan2(aCol0[1], aCol0[0]);
            }
            else
            {
                rRotate[0] = atan2(aCol1[0], aCol1[1]);
                rRotate[2] = 0.0;
            }

            bSuccess = TRUE;
        }
    }
    return bSuccess;
}

/*************************************************************************
|*
|* ViewOrientationMatrix
|*
\************************************************************************/

#if defined ( ICC ) || defined( GCC )
void Matrix4D::Orientation(Point4D aVRP, Vector3D aVPN, Vector3D aVUP)
#else
void Matrix4D::Orientation(Point4D& aVRP, Vector3D& aVPN, Vector3D& aVUP)
#endif
{
    // Translation um -VRP
    aVRP.Homogenize();
    Translate(-aVRP[0],-aVRP[1],-aVRP[2]);

    // Rotationen bilden
    aVUP.Normalize();
    aVPN.Normalize();

    Vector3D aRx = aVUP;
    Vector3D aRy = aVPN;

    // x-Achse als Senkrechte aus aVUP und aVPN bilden
    aRx |= aRy;
    aRx.Normalize();

    // dann y-Achse dazu senkrecht stellen
    aRy |= aRx;
    aRy.Normalize();

    // die berechneten Vektoren beschreiben die Zeilenvektoren der
    // notwendigen Rotationsmatrix. Diese muessen also nur noch
    // eingetragen werden, um die Rotation zu erzeugen.
    Matrix4D aTemp;

    aTemp[0][0] = aRx[0];
    aTemp[0][1] = aRx[1];
    aTemp[0][2] = aRx[2];

    aTemp[1][0] = aRy[0];
    aTemp[1][1] = aRy[1];
    aTemp[1][2] = aRy[2];

    aTemp[2][0] = aVPN[0];
    aTemp[2][1] = aVPN[1];
    aTemp[2][2] = aVPN[2];

    *this *= aTemp;
}

/*************************************************************************
|*
|* Projektionsmatrix fuer perspektivische Projektion
|*
\************************************************************************/

void Matrix4D::Frustum(double fLeft, double fRight, double fBottom,
    double fTop, double fNear, double fFar)
{
    if(!(fNear > 0.0))
    {
        fNear = 0.001;
    }
    if(!(fFar > 0.0))
    {
        fFar = 1.0;
    }
    if(fNear == fFar)
    {
        fFar = fNear + 1.0;
    }
    if(fLeft == fRight)
    {
        fLeft -= 1.0;
        fRight += 1.0;
    }
    if(fTop == fBottom)
    {
        fBottom -= 1.0;
        fTop += 1.0;
    }
    Matrix4D aTemp;

    aTemp.M[0][0] = 2.0 * fNear / (fRight - fLeft);
    aTemp.M[1][1] = 2.0 * fNear / (fTop - fBottom);
    aTemp.M[0][2] = (fRight + fLeft) / (fRight - fLeft);
    aTemp.M[1][2] = (fTop + fBottom) / (fTop - fBottom);
    aTemp.M[2][2] = -1.0 * ((fFar + fNear) / (fFar - fNear));
    aTemp.M[3][2] = -1.0;
    aTemp.M[2][3] = -1.0 * ((2.0 * fFar * fNear) / (fFar - fNear));
    aTemp.M[3][3] = 0.0;

    *this *= aTemp;
}

/*************************************************************************
|*
|* Projektionsmatrix fuer orthogonale (parallele)  Projektion
|*
\************************************************************************/

void Matrix4D::Ortho(double fLeft, double fRight, double fBottom,
    double fTop, double fNear, double fFar)
{
    if(fNear == fFar)
    {
        DBG_ERROR("Near and far clipping plane in Ortho definition are identical");
        fFar = fNear + 1.0;
    }
    if(fLeft == fRight)
    {
        DBG_ERROR("Left and right in Ortho definition are identical");
        fLeft -= 1.0;
        fRight += 1.0;
    }
    if(fTop == fBottom)
    {
        DBG_ERROR("Top and bottom in Ortho definition are identical");
        fBottom -= 1.0;
        fTop += 1.0;
    }
    Matrix4D aTemp;

    aTemp.M[0][0] = 2.0 / (fRight - fLeft);
    aTemp.M[1][1] = 2.0 / (fTop - fBottom);
    aTemp.M[2][2] = -1.0 * (2.0 / (fFar - fNear));
    aTemp.M[0][3] = -1.0 * ((fRight + fLeft) / (fRight - fLeft));
    aTemp.M[1][3] = -1.0 * ((fTop + fBottom) / (fTop - fBottom));
    aTemp.M[2][3] = -1.0 * ((fFar + fNear) / (fFar - fNear));

    *this *= aTemp;
}

/*************************************************************************
|*
|* Matrixaddition
|*
\************************************************************************/

Matrix4D& Matrix4D::operator+= (const Matrix4D& rMat)
{
    for(UINT16 i=0;i<4;i++)
    {
        for(UINT16 j=0;j<4;j++)
        {
            M[i][j] += rMat[i][j];
        }
    }
    return *this;
}

Matrix4D Matrix4D::operator+ (const Matrix4D& rMat) const
{
    Matrix4D aSum = *this;
    aSum += rMat;
    return aSum;
}

/*************************************************************************
|*
|* Matrixsubtraktion
|*
\************************************************************************/

Matrix4D& Matrix4D::operator-= (const Matrix4D& rMat)
{
    for(UINT16 i=0;i<4;i++)
    {
        for(UINT16 j=0;j<4;j++)
        {
            M[i][j] -= rMat[i][j];
        }
    }
    return *this;
}

Matrix4D Matrix4D::operator- (const Matrix4D& rMat) const
{
    Matrix4D aSum = *this;
    aSum -= rMat;
    return aSum;
}

/*************************************************************************
|*
|* Vergleichsoperatoren
|*
\************************************************************************/

BOOL Matrix4D::operator== (const Matrix4D& rMat) const
{
    for(UINT16 i=0;i<4;i++)
    {
        for(UINT16 j=0;j<4;j++)
        {
            if(M[i][j] != rMat[i][j])
                return FALSE;
        }
    }
    return TRUE;
}

BOOL Matrix4D::operator!=(const Matrix4D& rMat) const
{
    for(UINT16 i=0;i<4;i++)
    {
        for(UINT16 j=0;j<4;j++)
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

Matrix4D& Matrix4D::operator*= (double fFactor)
{
    for(UINT16 i=0;i<4;i++)
    {
        for(UINT16 j=0;j<4;j++)
        {
            M[i][j] *= fFactor;
        }
    }
    return *this;
}

Matrix4D Matrix4D::operator* (double fFactor) const
{
    Matrix4D aZwi = *this;
    aZwi *= fFactor;
    return aZwi;
}

/*************************************************************************
|*
|* Matrixdivision durch einen Faktor
|*
\************************************************************************/

Matrix4D& Matrix4D::operator/= (double fFactor)
{
    if(fFactor != 0.0)
    {
        for(UINT16 i=0;i<4;i++)
        {
            for(UINT16 j=0;j<4;j++)
            {
                M[i][j] /= fFactor;
            }
        }
    }
    return *this;
}

Matrix4D Matrix4D::operator/ (double fFactor) const
{
    Matrix4D aZwi = *this;
    aZwi /= fFactor;
    return aZwi;
}

/*************************************************************************
|*
|* Matrixmultiplikation von links, von rechts mittels umgekehrter
|* Parameterangaben zu erreichen
|*
\************************************************************************/

Matrix4D& Matrix4D::operator*= (const Matrix4D& rMat)
{
    // Matrixmultiplikation
    Matrix4D aCopy = *this;
    double fZwi;

    for(UINT16 i=0;i<4;i++)
    {
        for(UINT16 j=0;j<4;j++)
        {
            fZwi = 0.0;
            for(UINT16 k=0;k<4;k++)
            {
                fZwi += aCopy[k][j] * rMat[i][k];
            }
            M[i][j] = fZwi;
        }
    }
    return *this;
}

Matrix4D Matrix4D::operator* (const Matrix4D& rMat) const
{
    Matrix4D aZwi = rMat; // #112587#
    aZwi *= (*this);
    return aZwi;
}

/*************************************************************************
|*
|* Multiplikation Matrix, Punkt
|*
\************************************************************************/

Point4D operator* (const Matrix4D& rMatrix, const Point4D& rPnt)
{
    Point4D aNewPnt;
    UINT16 i,j;
    double fZwi;

    for(i=0;i<4;i++)
    {
        fZwi = 0.0;
        for(j=0;j<4;j++)
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

Vector3D operator* (const Matrix4D& rMatrix, const Vector3D& rVec)
{
    Vector3D aNewVec;
    UINT16 i,j;
    double fZwi;

    for(i=0;i<3;i++)
    {
        fZwi = 0.0;
        for(j=0;j<3;j++)
        {
            fZwi += rMatrix.M[i][j] * rVec[j];
        }
        fZwi += rMatrix.M[i][3];
        aNewVec[i] = fZwi;
    }
    fZwi = rMatrix[3][0] * rVec[0]
         + rMatrix[3][1] * rVec[1]
         + rMatrix[3][2] * rVec[2]
         + rMatrix[3][3];
    if(fZwi != 1.0 && fZwi != 0.0)
    {
        aNewVec[0] /= fZwi;
        aNewVec[1] /= fZwi;
        aNewVec[2] /= fZwi;
    }
    return aNewVec;
}

/*************************************************************************
|*
|* Stream-In-Operator fuer Matrix4D
|*
\************************************************************************/

SvStream& operator>>(SvStream& rIStream, Matrix4D& rMatrix4D)
{
    for (int i = 0; i < 4; i++)
        rIStream >> rMatrix4D.M[i];
    return rIStream;
}

/*************************************************************************
|*
|* Stream-Out-Operator fuer Matrix4D
|*
\************************************************************************/

SvStream& operator<<(SvStream& rOStream, const Matrix4D& rMatrix4D)
{
    for (int i = 0; i < 4; i++)
        rOStream << rMatrix4D.M[i];
    return rOStream;
}


}//end of namespace binfilter

// eof
