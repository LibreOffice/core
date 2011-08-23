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

#ifndef _B3D_POINT4D_HXX
#include "point4d.hxx"
#endif

namespace binfilter {
/*************************************************************************
|*
|* Konstruktor aus Vector3D
|*
\************************************************************************/

Point4D::Point4D(const Vector3D& rPnt, double fW )
{
    V[0] = rPnt.X();
    V[1] = rPnt.Y();
    V[2] = rPnt.Z();
    V[3] = fW;
}

/*************************************************************************
|*
|* Punkt Korrigieren (in homogene Koorinaten wandeln)
|*
\************************************************************************/

void Point4D::ImplHomogenize(void)
{
    if(V[3] != 0.0)
    {
        V[0] /= V[3];
        V[1] /= V[3];
        V[2] /= V[3];
        V[3] = 1.0;
    }
}

/*************************************************************************
|*
|* Punkt als Vector3D zurueckliefern. Dazu wird er Homogenisiert, wonach
|* die vierte Dimension (W) keine Rolle mehr spielt. An dieser Stelle
|* wird ausnahmsweise mal gecastet, aber zumindest ein const
|* zurueckgegeben.
|*
\************************************************************************/

const Vector3D& Point4D::GetVector3D() const
{
    ((Point4D* const)this)->Homogenize();
    return *((Vector3D*)this);
}

/*************************************************************************
|*
|* Neuen Punkt an der Stelle t im Parametergebiet der Linie berechnen
|*
\************************************************************************/

void Point4D::CalcInBetween(Point4D& rOld1, Point4D& rOld2, double t)
{
    // Punktkoordinaten berechnen
    for(UINT16 i=0;i<4;i++)
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
|* Neuen Punkt in der Mitte der Linie berechnen
|*
\************************************************************************/

void Point4D::CalcMiddle(Point4D& rOld1, Point4D& rOld2)
{
    // Punktkoordinaten berechnen
    for(UINT16 i=0;i<4;i++)
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
|* Punktaddition
|*
\************************************************************************/

Point4D& Point4D::operator+= (const Point4D& rPnt)
{
    if(rPnt[3] == 1.0)
    {
        if(V[3] == 1.0)
        {
            V[0] = V[0] + rPnt[0];
            V[1] = V[1] + rPnt[1];
            V[2] = V[2] + rPnt[2];
        }
        else
        {
            V[0] = V[0] + rPnt[0] * V[3];
            V[1] = V[1] + rPnt[1] * V[3];
            V[2] = V[2] + rPnt[2] * V[3];
        }
    }
    else
    {
        if(V[3] == 1.0)
        {
            V[0] = V[0] * rPnt[3] + rPnt[0];
            V[1] = V[1] * rPnt[3] + rPnt[1];
            V[2] = V[2] * rPnt[3] + rPnt[2];
            V[3] = rPnt[3];
        }
        else
        {
            V[0] = V[0] * rPnt[3] + rPnt[0] * V[3];
            V[1] = V[1] * rPnt[3] + rPnt[1] * V[3];
            V[2] = V[2] * rPnt[3] + rPnt[2] * V[3];
            V[3] = V[3] * rPnt[3];
        }
    }
    return *this;
}

/*************************************************************************
|*
|* Punktsubtraktion
|*
\************************************************************************/

Point4D& Point4D::operator-= (const Point4D& rPnt)
{
    if(rPnt[3] == 1.0)
    {
        if(V[3] == 1.0)
        {
            V[0] = V[0] - rPnt[0];
            V[1] = V[1] - rPnt[1];
            V[2] = V[2] - rPnt[2];
        }
        else
        {
            V[0] = V[0] - rPnt[0] * V[3];
            V[1] = V[1] - rPnt[1] * V[3];
            V[2] = V[2] - rPnt[2] * V[3];
        }
    }
    else
    {
        if(V[3] == 1.0)
        {
            V[0] = V[0] * rPnt[3] - rPnt[0];
            V[1] = V[1] * rPnt[3] - rPnt[1];
            V[2] = V[2] * rPnt[3] - rPnt[2];
            V[3] = rPnt[3];
        }
        else
        {
            V[0] = V[0] * rPnt[3] - rPnt[0] * V[3];
            V[1] = V[1] * rPnt[3] - rPnt[1] * V[3];
            V[2] = V[2] * rPnt[3] - rPnt[2] * V[3];
            V[3] = V[3] * rPnt[3];
        }
    }
    return *this;
}

/*************************************************************************
|*
|* Punktaddition, neuen Vektor erzeugen
|*
\************************************************************************/

Point4D Point4D::operator+ (const Point4D& rPnt) const
{
    Point4D aSum = *this;
    aSum += rPnt;
    return aSum;
}

/*************************************************************************
|*
|* Punktsubtraktion, neuen Vektor erzeugen
|*
\************************************************************************/

Point4D Point4D::operator- (const Point4D& rPnt) const
{
    Point4D aSub = *this;
    aSub -= rPnt;
    return aSub;
}

/*************************************************************************
|*
|* Punktnegation
|*
\************************************************************************/

Point4D Point4D::operator- (void) const
{
    Point4D aPnt = *this;
    aPnt[3] = -V[3];
    return aPnt;
}

/*************************************************************************
|*
|* Punktmultiplikation
|*
\************************************************************************/

Point4D& Point4D::operator*= (const Point4D& rPnt)
{
    V[0] = V[0] * rPnt[0];
    V[1] = V[1] * rPnt[1];
    V[2] = V[2] * rPnt[2];
    V[3] = V[3] * rPnt[3];
    return *this;
}

/*************************************************************************
|*
|* Punktdivision
|*
\************************************************************************/

Point4D& Point4D::operator/= (const Point4D& rPnt)
{
    if(rPnt[0] != 0.0 && rPnt[1] != 0.0
        && rPnt[2] != 0.0 && rPnt[3] != 0.0)
    {
        V[0] = V[0] / rPnt[0];
        V[1] = V[1] / rPnt[1];
        V[2] = V[2] / rPnt[2];
        V[3] = V[3] / rPnt[3];
    }
    return *this;
}

/*************************************************************************
|*
|* Punktmultiplikation, neuen Punkt erzeugen
|*
\************************************************************************/

Point4D Point4D::operator* (const Point4D& rPnt) const
{
    Point4D aSum = *this;
    aSum *= rPnt;
    return aSum;
}

/*************************************************************************
|*
|* Punktdivision, neuen Punkt erzeugen
|*
\************************************************************************/

Point4D Point4D::operator/ (const Point4D& rPnt) const
{
    Point4D aSub = *this;
    aSub /= rPnt;
    return aSub;
}

/*************************************************************************
|*
|* Punkttranslation
|*
\************************************************************************/

Point4D& Point4D::operator+= (const Vector3D& rVec)
{
    if(V[3] == 1.0)
    {
        V[0] = V[0] + rVec[0];
        V[1] = V[1] + rVec[1];
        V[2] = V[2] + rVec[2];
    }
    else
    {
        V[0] = V[0] + rVec[0] * V[3];
        V[1] = V[1] + rVec[1] * V[3];
        V[2] = V[2] + rVec[2] * V[3];
    }
    return *this;
}

/*************************************************************************
|*
|* Punkttranslation um negativen Vektor
|*
\************************************************************************/

Point4D& Point4D::operator-= (const Vector3D& rVec)
{
    if(V[3] == 1.0)
    {
        V[0] = V[0] - rVec[0];
        V[1] = V[1] - rVec[1];
        V[2] = V[2] - rVec[2];
    }
    else
    {
        V[0] = V[0] - rVec[0] * V[3];
        V[1] = V[1] - rVec[1] * V[3];
        V[2] = V[2] - rVec[2] * V[3];
    }
    return *this;
}

/*************************************************************************
|*
|* Punkttranslation, neuen Punkt erzeugen
|*
\************************************************************************/

Point4D Point4D::operator+ (const Vector3D& rVec) const
{
    Point4D aSum = *this;
    aSum += rVec;
    return aSum;
}

/*************************************************************************
|*
|* Punkttranslation um negativen Vektor, neuen Punkt erzeugen
|*
\************************************************************************/

Point4D Point4D::operator- (const Vector3D& rVec) const
{
    Point4D aSum = *this;
    aSum -= rVec;
    return aSum;
}

/*************************************************************************
|*
|* Multiplikation mit Faktor
|*
\************************************************************************/

Point4D& Point4D::operator*= (double fFactor)
{
    V[3] /= fFactor;
    return *this;
}

/*************************************************************************
|*
|* Multiplikation mit Faktor, neuen Punkt erzeugen
|*
\************************************************************************/

Point4D Point4D::operator* (double fFactor) const
{
    Point4D aNewPnt = *this;
    aNewPnt.W() /= fFactor;
    return aNewPnt;
}

/*************************************************************************
|*
|* Division mit Faktor
|*
\************************************************************************/

Point4D& Point4D::operator/= (double fDiv)
{
    if (fDiv != 0.0)
        V[3] *= fDiv;
    return *this;
}

/*************************************************************************
|*
|* Division mit Faktor, neuen Punkt erzeugen
|*
\************************************************************************/

Point4D Point4D::operator/ (double fDiv) const
{
    Point4D aNewPnt = *this;
    if (fDiv != 0.0)
        aNewPnt.W() *= fDiv;
    return aNewPnt;
}

/*************************************************************************
|*
|* Gleichheit
|*
\************************************************************************/

BOOL Point4D::operator==(const Point4D& rPnt) const
{
    if(rPnt[3] == 1.0)
    {
        if(V[3] == 1.0)
        {
            if(V[0] == rPnt[0])
                if(V[1] == rPnt[1])
                    if(V[2] == rPnt[2])
                        return TRUE;
        }
        else
        {
            if(V[0] == V[3] * rPnt[0])
                if(V[1] == V[3] * rPnt[1])
                    if(V[2] == V[3] * rPnt[2])
                        return TRUE;
        }
    }
    else
    {
        if(V[3] == 1.0)
        {
            if(V[0] * rPnt[3] == rPnt[0])
                if(V[1] * rPnt[3] == rPnt[1])
                    if(V[2] * rPnt[3] == rPnt[2])
                        return TRUE;
        }
        else
        {
            if(V[0] * rPnt[3] == V[3] * rPnt[0])
                if(V[1] * rPnt[3] == V[3] * rPnt[1])
                    if(V[2] * rPnt[3] == V[3] * rPnt[2])
                        return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************************
|*
|* Ungleichheit
|*
\************************************************************************/

BOOL Point4D::operator!=(const Point4D& rPnt) const
{
    if(rPnt[3] == 1.0)
    {
        if(V[3] == 1.0)
        {
            if(V[0] == rPnt[0])
                if(V[1] == rPnt[1])
                    if(V[2] == rPnt[2])
                        return FALSE;
        }
        else
        {
            if(V[0] == V[3] * rPnt[0])
                if(V[1] == V[3] * rPnt[1])
                    if(V[2] == V[3] * rPnt[2])
                        return FALSE;
        }
    }
    else
    {
        if(V[3] == 1.0)
        {
            if(V[0] * rPnt[3] == rPnt[0])
                if(V[1] * rPnt[3] == rPnt[1])
                    if(V[2] * rPnt[3] == rPnt[2])
                        return FALSE;
        }
        else
        {
            if(V[0] * rPnt[3] == V[3] * rPnt[0])
                if(V[1] * rPnt[3] == V[3] * rPnt[1])
                    if(V[2] * rPnt[3] == V[3] * rPnt[2])
                        return FALSE;
        }
    }
    return TRUE;
}

/*************************************************************************
|*
|* Stream-In-Operator fuer Point4D
|*
\************************************************************************/

SvStream& operator>>(SvStream& rIStream, Point4D& rPoint4D)
{
    for (int i = 0; i < 4; i++)
        rIStream >> rPoint4D.V[i];

    return rIStream;
}

/*************************************************************************
|*
|* Stream-Out-Operator fuer Point4D
|*
\************************************************************************/

SvStream& operator<<(SvStream& rOStream, const Point4D& rPoint4D)
{
    for (int i = 0; i < 4; i++)
        rOStream << rPoint4D.V[i];

    return rOStream;
}
}//end of namespace binfilter

// eof
