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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHICFILTER_IDXF_DXFVEC_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHICFILTER_IDXF_DXFVEC_HXX

#include <sal/types.h>
#include <vcl/lineinfo.hxx>

class Point;

class DXFLineInfo {
public:
    LineStyle       eStyle;
    sal_Int32       nDashCount;
    double          fDashLen;
    sal_Int32       nDotCount;
    double          fDotLen;
    double          fDistance;

    DXFLineInfo() :
        eStyle(LineStyle::Solid),
        nDashCount(0),
        fDashLen(0),
        nDotCount(0),
        fDotLen(0),
        fDistance(0) {}
};


//---------------------------- DXFVector ---------------------------------------

// common 3D vector with doubles

class DXFVector {

public:

    double fx,fy,fz; // public ! - why not?

    inline DXFVector(double fX=0.0, double fY=0.0, double fZ=0.0);

    // summation/subtraktion:
    DXFVector & operator += (const DXFVector & rV);
    DXFVector   operator +  (const DXFVector & rV) const;
    DXFVector   operator -  (const DXFVector & rV) const;

    // vector product
    DXFVector   operator *  (const DXFVector & rV) const;

    // scalar product:
    double SProd(const DXFVector & rV) const;

    // multiplication with scalar:
    DXFVector & operator *= (double fs);
    DXFVector   operator *  (double fs) const;

    // length:
    double Abs() const;

    // vector with same direction and a length of 1:
    DXFVector Unit() const;

    // equivalence or net:
    bool operator == (const DXFVector & rV) const;
};


//---------------------------- DXFTransform ------------------------------------

// a transformation matrice specialized for our problem

class DXFTransform {

public:

    DXFTransform();
        // destination coordinate = source coordinate

    DXFTransform(double fScaleX, double fScaleY, double fScaleZ,
                 const DXFVector & rShift);
        // dest coordinate = translate(scale(source coordinate))

    DXFTransform(double fScaleX, double fScaleY, double fScaleZ,
                 double fRotAngle,
                 const DXFVector & rShift);
        // dest coordinate = translate(rotate(scale(source coordinate)))
        // rotation around z-axis, fRotAngle in degrees.

    DXFTransform(const DXFVector & rExtrusion);
        // Transformation "ECS->WCS" via "Entity Extrusion Direction"
        // ant the "Arbitrary Axis Algorithm"
        // (See DXF-Docu from AutoDesk)

    DXFTransform(const DXFVector & rViewDir, const DXFVector & rViewTarget);
        // Transformation object space->picture space on the basis of direction
        // destination point of a viewport
        // (See DXF-Docu from AutoDesk: VPORT)

    DXFTransform(const DXFTransform & rT1, const DXFTransform & rT2);
        // destination coordinate = rT2(rT1(source coordinate))


    void Transform(const DXFVector & rSrc, DXFVector & rTgt) const;
        // Transformation from DXFVector to DXFVector

    void Transform(const DXFVector & rSrc, Point & rTgt) const;
        // Transformation from DXFVector to SvPoint

    void TransDir(const DXFVector & rSrc, DXFVector & rTgt) const;
        // Transformation of a relative vector (so no translation)

    bool TransCircleToEllipse(double fRadius, double & rEx, double & rEy) const;
        // Attempt to transform a circle (in xy plane) so that it results
        // in an aligned ellipse. If the does not work because an ellipse of
        // arbitrary position would be created, sal_False is returned.
        // (The center point will not be transformed, use Transform(..))

    double CalcRotAngle() const;
        // Calculates the rotation angle around z-axis (in degrees)

    bool Mirror() const;
        // Returns sal_True, if the matrice represents a left-handed coordinate system

    LineInfo Transform(const DXFLineInfo& aDXFLineInfo) const;
        // Transform to LineInfo

private:
    DXFVector aMX;
    DXFVector aMY;
    DXFVector aMZ;
    DXFVector aMP;
};


//------------------------------- inlines --------------------------------------


inline DXFVector::DXFVector(double fX, double fY, double fZ)
{
    fx=fX; fy=fY; fz=fZ;
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


inline bool DXFVector::operator == (const DXFVector & rV) const
{
    if (fx==rV.fx && fy==rV.fy && fz==rV.fz) return true;
    else return false;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
