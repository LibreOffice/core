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

#include "vector3d.hxx"

#include <tools/stream.hxx>

#include <tools/gen.hxx>

#ifndef _INC_FLOAT
#include <float.h>
#endif

namespace binfilter {
/*************************************************************************
|*
|* Konstruktor aus Point
|*
\************************************************************************/

Vector3D::Vector3D(const Point& rPnt, double fZ)
{
    V[0] = rPnt.X();
    V[1] = rPnt.Y();
    V[2] = fZ;
}

/*************************************************************************
|*
|* Vektorlaenge berechnen
|*
\************************************************************************/

double Vector3D::GetLength(void) const
{
    double fSum = V[0] * V[0] + V[1] * V[1] + V[2] * V[2];
    if(fSum == 0.0 || fSum == 1.0)
        return fSum;
    return sqrt(fSum);
}

/*************************************************************************
|*
|* Laenge der Projektion des Vektors auf die XZ-Ebene berechnen
|*
\************************************************************************/

double Vector3D::GetXZLength(void) const
{
    double fSum = V[0] * V[0] + V[2] * V[2];
    if(fSum == 0.0 || fSum == 1.0)
        return fSum;
    return sqrt(fSum);
}

/*************************************************************************
|*
|* Laenge der Projektion des Vektors auf die YZ-Ebene berechnen
|*
\************************************************************************/

double Vector3D::GetYZLength(void) const
{
    double fSum = V[1] * V[1] + V[2] * V[2];
    if(fSum == 0.0 || fSum == 1.0)
        return fSum;
    return sqrt(fSum);
}

/*************************************************************************
|*
|* Vektor Normalisieren
|*
\************************************************************************/

void Vector3D::Normalize(void)
{
    double fLen = V[0] * V[0] + V[1] * V[1] + V[2] * V[2];

    // nur wirklich rechnen (und Wurzel ziehen), wenn != 0.0
    // und der Faktor nicht schon 1.0, denn dann ist der Vektor
    // bereits normalisiert
    if(fLen == 0.0 || fLen == 1.0)
        return;

    fLen = sqrt(fLen);

    if(fLen == 0.0)
        return;

    V[0] /= fLen;
    V[1] /= fLen;
    V[2] /= fLen;
}

/*************************************************************************
|*
|* Minimum aus diesem und dem uebergebenen Vektor bilden
|*
\************************************************************************/

void Vector3D::Min(const Vector3D& rVec)
{
    if ( V[0] > rVec.V[0] ) V[0] = rVec.V[0];
    if ( V[1] > rVec.V[1] ) V[1] = rVec.V[1];
    if ( V[2] > rVec.V[2] ) V[2] = rVec.V[2];
}

/*************************************************************************
|*
|* Maximum aus diesem und dem uebergebenen Vektor bilden
|*
\************************************************************************/

void Vector3D::Max(const Vector3D& rVec)
{
    if ( V[0] < rVec.V[0] ) V[0] = rVec.V[0];
    if ( V[1] < rVec.V[1] ) V[1] = rVec.V[1];
    if ( V[2] < rVec.V[2] ) V[2] = rVec.V[2];
}

/*************************************************************************
|*
|* Absolutwert
|*
\************************************************************************/

void Vector3D::Abs()
{
    if ( V[0] < 0 ) V[0] = - V[0];
    if ( V[1] < 0 ) V[1] = - V[1];
    if ( V[2] < 0 ) V[2] = - V[2];
}

/*************************************************************************
|*
|* Neue Position an der Stelle t im Parametergebiet des Vektors berechnen
|*
\************************************************************************/

void Vector3D::CalcInBetween(const Vector3D& rOld1, const Vector3D& rOld2, double t)
{
    for(UINT16 i=0;i<3;i++)
    {
        if(rOld2[i] == rOld1[i])
        {
            V[i] = rOld1[i];
        }
        else
        {
            V[i] = ((rOld2[i] - rOld1[i]) * t) + rOld1[i];
        }
    }
}

/*************************************************************************
|*
|* Neue Position in der Mitte des Vektors berechnen
|*
\************************************************************************/

void Vector3D::CalcMiddle(const Vector3D& rOld1, const Vector3D& rOld2)
{
    for(UINT16 i=0;i<3;i++)
    {
        if(rOld2[i] == rOld1[i])
        {
            V[i] = rOld1[i];
        }
        else
        {
            V[i] = (rOld1[i] + rOld2[i]) / 2.0;
        }
    }
}

/*************************************************************************
|*
|* Vektoraddition
|*
\************************************************************************/

Vector3D& Vector3D::operator+= (const Vector3D& rVec)
{
    V[0] += rVec.V[0];
    V[1] += rVec.V[1];
    V[2] += rVec.V[2];
    return *this;
}

/*************************************************************************
|*
|* Vektorsubtraktion
|*
\************************************************************************/

Vector3D& Vector3D::operator-= (const Vector3D& rVec)
{
    V[0] -= rVec.V[0];
    V[1] -= rVec.V[1];
    V[2] -= rVec.V[2];
    return *this;
}

/*************************************************************************
|*
|* Vektoraddition, neuen Vektor erzeugen
|*
\************************************************************************/

Vector3D Vector3D::operator+ (const Vector3D& rVec) const
{
    Vector3D aSum = *this;

    aSum += rVec;
    return aSum;
}

/*************************************************************************
|*
|* Vektorsubtraktion, neuen Vektor erzeugen
|*
\************************************************************************/

Vector3D Vector3D::operator- (const Vector3D& rVec) const
{
    Vector3D aSub = *this;

    aSub -= rVec;
    return aSub;
}

/*************************************************************************
|*
|* Vektornegation
|*
\************************************************************************/

Vector3D Vector3D::operator- (void) const
{
    Vector3D aVec;

    aVec[0] = -V[0];
    aVec[1] = -V[1];
    aVec[2] = -V[2];
    return aVec;
}

/*************************************************************************
|*
|* Kreuzprodukt
|*
\************************************************************************/

Vector3D& Vector3D::operator|=(const Vector3D& rVec)
{
    // this sichern, da Elemente ueberschrieben werden
    Vector3D aVec = *this;
    V[0] = aVec.V[1] * rVec.V[2] - aVec.V[2] * rVec.V[1];
    V[1] = aVec.V[2] * rVec.V[0] - aVec.V[0] * rVec.V[2];
    V[2] = aVec.V[0] * rVec.V[1] - aVec.V[1] * rVec.V[0];
    return *this;
}

Vector3D Vector3D::operator|(const Vector3D& rVec) const
{
    Vector3D aCross = *this;
    aCross |= rVec;
    return aCross;
}

/*************************************************************************
|*
|* Skalarprodukt
|*
\************************************************************************/

double Vector3D::Scalar(const Vector3D& rVec) const
{
    return ((V[0] * rVec.V[0]) + (V[1] * rVec.V[1]) + (V[2] * rVec.V[2]));
}

/*************************************************************************
|*
|* Vektormultiplikation
|*
\************************************************************************/

Vector3D& Vector3D::operator*= (const Vector3D& rVec)
{
    V[0] *= rVec.V[0];
    V[1] *= rVec.V[1];
    V[2] *= rVec.V[2];
    return *this;
}

/*************************************************************************
|*
|* Vektormultiplikation, neuen Vektor erzeugen
|*
\************************************************************************/

Vector3D Vector3D::operator* (const Vector3D& rVec) const
{
    Vector3D aMul = *this;

    aMul *= rVec;
    return aMul;
}

/*************************************************************************
|*
|* Vektordivision
|*
\************************************************************************/

Vector3D& Vector3D::operator/= (const Vector3D& rVec)
{
    if(rVec.V[0] != 0.0)
        V[0] /= rVec.V[0];
    else
        V[0] = DBL_MAX;

    if(rVec.V[1] != 0.0)
        V[1] /= rVec.V[1];
    else
        V[1] = DBL_MAX;

    if(rVec.V[2] != 0.0)
        V[2] /= rVec.V[2];
    else
        V[2] = DBL_MAX;
    return *this;
}

/*************************************************************************
|*
|* Vektordivision, neuen Vektor erzeugen
|*
\************************************************************************/

Vector3D Vector3D::operator/ (const Vector3D& rVec) const
{
    Vector3D aDiv = *this;

    aDiv /= rVec;
    return aDiv;
}

/*************************************************************************
|*
|* Skalarmultiplikation
|*
\************************************************************************/

Vector3D& Vector3D::operator*= (double fFactor)
{
    V[0] *= fFactor;
    V[1] *= fFactor;
    V[2] *= fFactor;
    return *this;
}

/*************************************************************************
|*
|* Skalarmultiplikation, neuen Vektor erzeugen
|*
\************************************************************************/

Vector3D Vector3D::operator* (double fFactor) const
{
    Vector3D aNewVec = *this;

    aNewVec *= fFactor;
    return aNewVec;
}

/*************************************************************************
|*
|* Skalardivision
|*
\************************************************************************/

Vector3D& Vector3D::operator/= (double fDiv)
{
    if (fDiv != 0.0)
    {
        V[0] /= fDiv;
        V[1] /= fDiv;
        V[2] /= fDiv;
    }
    return *this;
}

/*************************************************************************
|*
|* Skalardivision, neuen Vektor erzeugen
|*
\************************************************************************/

Vector3D Vector3D::operator/ (double fDiv) const
{
    Vector3D aNewVec = *this;

    if (fDiv != 0.0) aNewVec /= fDiv;
    return aNewVec;
}

/*************************************************************************
|*
|* Skalarmultiplikation, neuen Vektor erzeugen
|*
\************************************************************************/

BOOL Vector3D::operator==(const Vector3D& rVec) const
{
    return (V[0] == rVec.V[0] && V[1] == rVec.V[1] && V[2] == rVec.V[2] );
}

/*************************************************************************
|*
|* Skalarmultiplikation, neuen Vektor erzeugen
|*
\************************************************************************/

BOOL Vector3D::operator!=(const Vector3D& rVec) const
{
    return (V[0] != rVec.V[0] || V[1] != rVec.V[1] || V[2] != rVec.V[2] );
}

/*************************************************************************
|*
|* Stream-In-Operator fuer Vector3D
|*
\************************************************************************/

SvStream& operator>>(SvStream& rIStream, Vector3D& rVector3D)
{
    for (int i = 0; i < 3; i++)
        rIStream >> rVector3D.V[i];

    return rIStream;
}

/*************************************************************************
|*
|* Stream-Out-Operator fuer Vector3D
|*
\************************************************************************/

SvStream& operator<<(SvStream& rOStream, const Vector3D& rVector3D)
{
    for (int i = 0; i < 3; i++)
        rOStream << rVector3D.V[i];

    return rOStream;
}

}//end of namespace binfilter

// eof



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
