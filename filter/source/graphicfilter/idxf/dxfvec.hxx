/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _DXFVEC_HXX
#define _DXFVEC_HXX

#include <tools/gen.hxx>
#include <vcl/lineinfo.hxx>

class DXFLineInfo {
public:
    LineStyle       eStyle;
    double          fWidth;
    sal_Int32       nDashCount;
    double          fDashLen;
    sal_Int32       nDotCount;
    double          fDotLen;
    double          fDistance;

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

    double fx,fy,fz; // public ! - why not?

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

    // length:
    double Abs() const;

    // Vektor gleicher Richtung und der Laenge 1:
    DXFVector Unit() const;

    // Aequivalenz oder nicht:
    sal_Bool operator == (const DXFVector & rV) const;
    sal_Bool operator != (const DXFVector & rV) const;
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

    sal_Bool TransCircleToEllipse(double fRadius, double & rEx, double & rEy) const;
        // Versucht, einen Kreis (in der XY-Ebene) zu transformieren, so dass eine
        // ausgerichtete Ellipse entsteht. Wenn das nicht geht, weil Ellipse
        // in belibieger Lage entstehen wuerde, wird sal_False geliefert.
        // (Der Mittelpunkt wird hiermit nicht transformiert, nehme Transform(..))

    sal_uLong TransLineWidth(double fW) const;
        // Transformiert die Liniendicke (so gut es geht)

    double CalcRotAngle() const;
        // Ermittelt den Rotationswinkel um die Z-Achse (in Grad)

    sal_Bool Mirror() const;
        // Liefert sal_True, wenn die Matrix ein Linkssystem bildet

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


inline sal_Bool DXFVector::operator == (const DXFVector & rV) const
{
    if (fx==rV.fx && fy==rV.fy && fz==rV.fz) return sal_True;
    else return sal_False;
}


inline sal_Bool DXFVector::operator != (const DXFVector & rV) const
{
    if (fx!=rV.fx || fy!=rV.fy || fz!=rV.fz) return sal_True;
    else return sal_False;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
