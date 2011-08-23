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

#ifndef _DXFVEC_HXX
#define _DXFVEC_HXX

#include <tools/gen.hxx>
#include <vcl/lineinfo.hxx>

class DXFLineInfo {
public:
    LineStyle		eStyle;
    double			fWidth;
    sal_Int32		nDashCount;
    double			fDashLen;
    sal_Int32		nDotCount;
    double			fDotLen;
    double			fDistance;

    DXFLineInfo() :
        eStyle(LINE_SOLID),
        fWidth(0),
        nDashCount(0),
        fDashLen(0),
        nDotCount(0),
        fDotLen(0),
        fDistance(0) {}

    DXFLineInfo(const DXFLineInfo& x) :
        eStyle(x.eStyle),
        fWidth(x.fWidth),
        nDashCount(x.nDashCount),
        fDashLen(x.fDashLen),
        nDotCount(x.nDotCount),
        fDotLen(x.fDotLen),
        fDistance(x.fDistance) {}

};


//------------------------------------------------------------------------------
//---------------------------- DXFVector ---------------------------------------
//------------------------------------------------------------------------------
// Allgemeiner 3D-Vektor mit double

class DXFVector {

public:

    double fx,fy,fz; // public ! - Warum nicht ?

    inline DXFVector(double fX=0.0, double fY=0.0, double fZ=0.0);
    inline DXFVector(const DXFVector & rV);

    // Addition/Subtraktion:
    DXFVector & operator += (const DXFVector & rV);
    DXFVector   operator +  (const DXFVector & rV) const;
    DXFVector & operator -= (const DXFVector & rV);
    DXFVector   operator -  (const DXFVector & rV) const;

    // Vektorprodukt
    DXFVector   operator *  (const DXFVector & rV) const;

    // Skalarprodukt:
    double SProd(const DXFVector & rV) const;

    // Multiplikation mit Skalar:
    DXFVector & operator *= (double fs);
    DXFVector   operator *  (double fs) const;

    // Laenge:
    double Abs() const;

    // Vektor gleicher Richtung und der Laenge 1:
    DXFVector Unit() const;

    // Aequivalenz oder nicht:
    BOOL operator == (const DXFVector & rV) const;
    BOOL operator != (const DXFVector & rV) const;
};

//------------------------------------------------------------------------------
//---------------------------- DXFTransform ------------------------------------
//------------------------------------------------------------------------------
// Eine Transformationsmatrix, spezialisiert auf unser Problem

class DXFTransform {

public:

    DXFTransform();
        // Zielkoordinate = Quellkoordinate

    DXFTransform(double fScaleX, double fScaleY, double fScaleZ,
                 const DXFVector & rShift);
        // Zielkoordinate = Verschoben(Skaliert(Quellkoorinate))

    DXFTransform(double fScaleX, double fScaleY, double fScaleZ,
                 double fRotAngle,
                 const DXFVector & rShift);
        // Zielkoordinate = Verschoben(Gedreht(Skaliert(Quellkoorinate)))
        // Drehung geshieht um die Z-Achse, fRotAngle in Grad.

    DXFTransform(const DXFVector & rExtrusion);
        // Transformation "ECS->WCS" per "Entity Extrusion Direction"
        // und dem "Arbitrary Axis Algorithm"
        // (Siehe DXF-Docu von AutoDesk)

    DXFTransform(const DXFVector & rViewDir, const DXFVector & rViewTarget);
        // Transformation Objektraum->Bildraum anhand von Richtung und
        // Zielpunkt eines ViewPort.
        // (siehe DXF-Docu von AutoDesk: VPORT)

    DXFTransform(const DXFTransform & rT1, const DXFTransform & rT2);
        // Zielkoordinate = rT2(rT1(Quellkoorinate))


    void Transform(const DXFVector & rSrc, DXFVector & rTgt) const;
        // Transformation DXFVector nach DXFVector

    void Transform(const DXFVector & rSrc, Point & rTgt) const;
        // Transformation DXFVector nach SvPoint

    void TransDir(const DXFVector & rSrc, DXFVector & rTgt) const;
        // Transformation eines relativen Vektors (also kein Verschiebung)

    BOOL TransCircleToEllipse(double fRadius, double & rEx, double & rEy) const;
        // Versucht, einen Kreis (in der XY-Ebene) zu transformieren, so dass eine
        // ausgerichtete Ellipse entsteht. Wenn das nicht geht, weil Ellipse
        // in belibieger Lage entstehen wuerde, wird FALSE geliefert.
        // (Der Mittelpunkt wird hiermit nicht transformiert, nehme Transform(..))

    ULONG TransLineWidth(double fW) const;
        // Transformiert die Liniendicke (so gut es geht)

    double CalcRotAngle() const;
        // Ermittelt den Rotationswinkel um die Z-Achse (in Grad)

    BOOL Mirror() const;
        // Liefert TRUE, wenn die Matrix ein Linkssystem bildet

    LineInfo Transform(const DXFLineInfo& aDXFLineInfo) const;
        // Transform to LineInfo

private:
    DXFVector aMX;
    DXFVector aMY;
    DXFVector aMZ;
    DXFVector aMP;
};

//------------------------------------------------------------------------------
//------------------------------- inlines --------------------------------------
//------------------------------------------------------------------------------


inline DXFVector::DXFVector(double fX, double fY, double fZ)
{
    fx=fX; fy=fY; fz=fZ;
}


inline DXFVector::DXFVector(const DXFVector & rV)
{
    fx=rV.fx; fy=rV.fy; fz=rV.fz;
}


inline DXFVector & DXFVector::operator += (const DXFVector & rV)
{
    fx+=rV.fx; fy+=rV.fy; fz+=rV.fz;
    return *this;
}


inline DXFVector DXFVector::operator + (const DXFVector & rV) const
{
    return DXFVector(fx+rV.fx, fy+rV.fy, fz+rV.fz);
}


inline DXFVector & DXFVector::operator -= (const DXFVector & rV)
{
    fx-=rV.fx; fy-=rV.fy; fz-=rV.fz;
    return *this;
}


inline DXFVector DXFVector::operator - (const DXFVector & rV) const
{
    return DXFVector(fx-rV.fx, fy-rV.fy, fz-rV.fz);
}


inline DXFVector DXFVector::operator *  (const DXFVector & rV) const
{
    return DXFVector(
        fy * rV.fz - fz * rV.fy,
        fz * rV.fx - fx * rV.fz,
        fx * rV.fy - fy * rV.fx
    );
}


inline double DXFVector::SProd(const DXFVector & rV) const
{
    return fx*rV.fx + fy*rV.fy + fz*rV.fz;
}


inline DXFVector & DXFVector::operator *= (double fs)
{
    fx*=fs; fy*=fs; fz*=fs;
    return *this;
}


inline DXFVector DXFVector::operator * (double fs) const
{
    return DXFVector(fx*fs,fy*fs,fz*fs);
}


inline BOOL DXFVector::operator == (const DXFVector & rV) const
{
    if (fx==rV.fx && fy==rV.fy && fz==rV.fz) return TRUE;
    else return FALSE;
}


inline BOOL DXFVector::operator != (const DXFVector & rV) const
{
    if (fx!=rV.fx || fy!=rV.fy || fz!=rV.fz) return TRUE;
    else return FALSE;
}

#endif
