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


#include <math.h>
#include "dxfvec.hxx"
#include <tools/gen.hxx>


//---------------------------- DXFVector ---------------------------------------


double DXFVector::Abs() const
{
    return std::hypot(fx, fy, fz);
}


DXFVector DXFVector::Unit() const
{
    double flen;

    flen=Abs();
    if (flen!=0) return (*this)*(1.0/flen);
    else return DXFVector(1.0,0.0,0.0);
}


//---------------------------- DXFTransform ------------------------------------


DXFTransform::DXFTransform() :
    aMX(1.0, 0.0, 0.0),
    aMY(0.0, 1.0, 0.0),
    aMZ(0.0, 0.0, 1.0),
    aMP(0.0, 0.0, 0.0)
{
}


DXFTransform::DXFTransform(double fScaleX, double fScaleY, double fScaleZ,
                           const DXFVector & rShift) :
    aMX(fScaleX, 0.0, 0.0),
    aMY(0.0, fScaleY, 0.0),
    aMZ(0.0, 0.0, fScaleZ),
    aMP(rShift)
{
}


DXFTransform::DXFTransform(double fScaleX, double fScaleY, double fScaleZ,
                           double fRotAngle,
                           const DXFVector & rShift) :
    aMX(0.0, 0.0, 0.0),
    aMY(0.0, 0.0, 0.0),
    aMZ(0.0, 0.0, fScaleZ),
    aMP(rShift)
{
    aMX.fx=cos(basegfx::deg2rad(fRotAngle));
    aMX.fy=sin(basegfx::deg2rad(fRotAngle));
    aMY.fx=-aMX.fy;
    aMY.fy=aMX.fx;
    aMX*=fScaleX;
    aMY*=fScaleY;
}


DXFTransform::DXFTransform(const DXFVector & rExtrusion) :
    aMP(0.0, 0.0, 0.0)
{
    // 'Arbitrary Axis Algorithm' (cf. DXF documentation by Autodesk)
    if ( fabs(rExtrusion.fx) < 1.0/64.0 && fabs(rExtrusion.fy) < 1.0/64.0) {
        aMX = DXFVector(0.0, 1.0, 0.0) * rExtrusion;
    }
    else {
        aMX = DXFVector(0.0, 0.0, 1.0) * rExtrusion;
    }
    aMX=aMX.Unit();
    aMY=(rExtrusion*aMX).Unit();
    aMZ=rExtrusion.Unit();
}


DXFTransform::DXFTransform(const DXFVector & rViewDir, const DXFVector & rViewTarget)
{
    DXFVector aV;

    aV=rViewDir.Unit();
    aMX.fz=aV.fx;
    aMY.fz=aV.fy;
    aMZ.fz=aV.fz;

    aMZ.fx=0;
    if (aV.fx==0) aMY.fx=0; else aMY.fx=sqrt(1/(1+aV.fy*aV.fy/(aV.fx*aV.fx)));
    aMX.fx=sqrt(1-aMY.fx*aMY.fx);
    if (aV.fx*aV.fy*aMY.fx>0) aMX.fx=-aMX.fx;

    aV=aV*DXFVector(aMX.fx,aMY.fx,aMZ.fx);
    aMX.fy=aV.fx;
    aMY.fy=aV.fy;
    aMZ.fy=aV.fz;

    if (aMZ.fy<0) {
        aMX.fy=-aMX.fy;
        aMY.fy=-aMY.fy;
        aMZ.fy=-aMZ.fy;
        aMX.fx=-aMX.fx;
        aMY.fx=-aMY.fx;
    }

    aV=DXFVector(0,0,0)-rViewTarget;
    aMP.fx = aV.fx * aMX.fx + aV.fy * aMY.fx + aV.fz * aMZ.fx;
    aMP.fy = aV.fx * aMX.fy + aV.fy * aMY.fy + aV.fz * aMZ.fy;
    aMP.fz = aV.fx * aMX.fz + aV.fy * aMY.fz + aV.fz * aMZ.fz;
}


DXFTransform::DXFTransform(const DXFTransform & rT1, const DXFTransform & rT2)
{
    rT2.TransDir(rT1.aMX,aMX);
    rT2.TransDir(rT1.aMY,aMY);
    rT2.TransDir(rT1.aMZ,aMZ);
    rT2.Transform(rT1.aMP,aMP);
}


void DXFTransform::Transform(const DXFVector & rSrc, DXFVector & rTgt) const
{
    rTgt.fx = rSrc.fx * aMX.fx + rSrc.fy * aMY.fx + rSrc.fz * aMZ.fx + aMP.fx;
    rTgt.fy = rSrc.fx * aMX.fy + rSrc.fy * aMY.fy + rSrc.fz * aMZ.fy + aMP.fy;
    rTgt.fz = rSrc.fx * aMX.fz + rSrc.fy * aMY.fz + rSrc.fz * aMZ.fz + aMP.fz;
}


void DXFTransform::Transform(const DXFVector & rSrc, Point & rTgt) const
{
    rTgt.setX(static_cast<tools::Long>( rSrc.fx * aMX.fx + rSrc.fy * aMY.fx + rSrc.fz * aMZ.fx + aMP.fx + 0.5 ) );
    rTgt.setY(static_cast<tools::Long>( rSrc.fx * aMX.fy + rSrc.fy * aMY.fy + rSrc.fz * aMZ.fy + aMP.fy + 0.5 ) );
}


void DXFTransform::TransDir(const DXFVector & rSrc, DXFVector & rTgt) const
{
    rTgt.fx = rSrc.fx * aMX.fx + rSrc.fy * aMY.fx + rSrc.fz * aMZ.fx;
    rTgt.fy = rSrc.fx * aMX.fy + rSrc.fy * aMY.fy + rSrc.fz * aMZ.fy;
    rTgt.fz = rSrc.fx * aMX.fz + rSrc.fy * aMY.fz + rSrc.fz * aMZ.fz;
}


bool DXFTransform::TransCircleToEllipse(double fRadius, double & rEx, double & rEy) const
{
    double fMXAbs=aMX.Abs();
    double fMYAbs=aMY.Abs();
    double fNearNull=(fMXAbs+fMYAbs)*0.001;

    if (fabs(aMX.fy)<=fNearNull && fabs(aMX.fz)<=fNearNull &&
        fabs(aMY.fx)<=fNearNull && fabs(aMY.fz)<=fNearNull)
    {
        rEx=fabs(aMX.fx*fRadius);
        rEy=fabs(aMY.fy*fRadius);
        return true;
    }
    else if (fabs(aMX.fx)<=fNearNull && fabs(aMX.fz)<=fNearNull &&
             fabs(aMY.fy)<=fNearNull && fabs(aMY.fz)<=fNearNull)
    {
        rEx=fabs(aMY.fx*fRadius);
        rEy=fabs(aMX.fy*fRadius);
        return true;
    }
    else if (fabs(fMXAbs-fMYAbs)<=fNearNull &&
             fabs(aMX.fz)<=fNearNull && fabs(aMY.fz)<=fNearNull)
    {
        rEx=rEy=fabs(((fMXAbs+fMYAbs)/2)*fRadius);
        return true;
    }
    else return false;
}

LineInfo DXFTransform::Transform(const DXFLineInfo& aDXFLineInfo) const
{
    double fex,fey,scale;

    fex=std::hypot(aMX.fx, aMX.fy);
    fey=std::hypot(aMY.fx, aMY.fy);
    scale = (fex+fey)/2.0;

    LineInfo aLineInfo;

    aLineInfo.SetStyle( aDXFLineInfo.eStyle );
    aLineInfo.SetWidth( 0 );
    aLineInfo.SetDashCount( static_cast< sal_uInt16 >( aDXFLineInfo.nDashCount ) );
    aLineInfo.SetDashLen( aDXFLineInfo.fDashLen * scale );
    aLineInfo.SetDotCount( static_cast< sal_uInt16 >( aDXFLineInfo.nDotCount ) );
    aLineInfo.SetDotLen( aDXFLineInfo.fDotLen * scale );
    aLineInfo.SetDistance( aDXFLineInfo.fDistance * scale );

    if ( aLineInfo.GetDashCount() > 0 && aLineInfo.GetDashLen() == 0 )
        aLineInfo.SetDashLen(1);

    if ( aLineInfo.GetDotCount() > 0 && aLineInfo.GetDotLen() == 0 )
        aLineInfo.SetDotLen(1);

    return aLineInfo;
}

double DXFTransform::CalcRotAngle() const
{
    return basegfx::rad2deg(atan2(aMX.fy,aMX.fx));
}

bool DXFTransform::Mirror() const
{
    return aMZ.SProd(aMX*aMY)<0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
