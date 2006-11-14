/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dentty.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 16:07:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_goodies.hxx"

#ifndef _B3D_B3DENTITY_HXX
#include "b3dentty.hxx"
#endif

#ifndef _B3D_B3DCOMMN_HXX
#include "b3dcommn.hxx"
#endif

#ifndef _B3D_B3DTRANS_HXX
#include "b3dtrans.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

/*************************************************************************
|*
|* Kopieren eine 3DEntity
|*
\************************************************************************/

void B3dEntity::Copy(B3dEntity& rEnt)
{
    maPoint = rEnt.maPoint;
    mbDeviceCoor = rEnt.IsDeviceCoor();
    mbValid = rEnt.IsValid();
    mbEdgeFlag = rEnt.mbEdgeFlag;
    maPlaneNormal = rEnt.maPlaneNormal;

    mbNormalUsed = rEnt.IsNormalUsed();
    if( mbNormalUsed )
        maNormal = rEnt.maNormal;

    mbTexCoorUsed = rEnt.IsTexCoorUsed();
    if( mbTexCoorUsed )
        maTexCoor = rEnt.maTexCoor;

    maColor = rEnt.maColor;
}

/*************************************************************************
|*
|* Flags auf Ausgangsposition
|*
\************************************************************************/

void B3dEntity::Reset()
{
    mbValid = mbNormalUsed = mbTexCoorUsed = mbDeviceCoor = sal_False;
    mbEdgeFlag = sal_True;
}

/*************************************************************************
|*
|* Device Koordinaten des Punktes berechnen
|*
\************************************************************************/

void B3dEntity::ImplToDeviceCoor(B3dTransformationSet* pSet)
{
    if(pSet && !mbDeviceCoor)
    {
        const basegfx::B3DVector& rScale = pSet->GetScale();
        const basegfx::B3DVector& rTrans = pSet->GetTranslate();

        maPoint.setX((maPoint.getX() * rScale.getX()) + rTrans.getX());
        maPoint.setY((maPoint.getY() * rScale.getY()) + rTrans.getY());
        maPoint.setZ((maPoint.getZ() * rScale.getZ()) + rTrans.getZ());

        mbDeviceCoor = sal_True;
    }
}

/*************************************************************************
|*
|* aus Device Koordinaten des Punktes 3D Koor im canonical view volume
|* berechnen
|*
\************************************************************************/

void B3dEntity::ImplTo3DCoor(B3dTransformationSet* pSet)
{
    if(pSet && mbDeviceCoor)
    {
        const basegfx::B3DVector& rScale = pSet->GetScale();
        const basegfx::B3DVector& rTrans = pSet->GetTranslate();

        if(rScale.getX() != 0.0)
            maPoint.setX((maPoint.getX() - rTrans.getX()) / rScale.getX());
        if(rScale.getY() != 0.0)
            maPoint.setY((maPoint.getY() - rTrans.getY()) / rScale.getY());
        if(rScale.getZ() != 0.0)
            maPoint.setZ((maPoint.getZ() - rTrans.getZ()) / rScale.getZ());

        mbDeviceCoor = sal_False;
    }
}

/*************************************************************************
|*
|* Garantiere eine gemeinsame Datenbasis (ClipKoordinaten oder
|* Devicekoordinaten)
|*
\************************************************************************/

void B3dEntity::ForceEqualBase(B3dTransformationSet* pSet, B3dEntity& rOld)
{
    if(IsDeviceCoor() && rOld.IsDeviceCoor())
    {
        SetDeviceCoor();
    }
    else
    {
        if(IsDeviceCoor())
            To3DCoor(pSet);
        if(rOld.IsDeviceCoor())
            rOld.To3DCoor(pSet);
    }
}

/*************************************************************************
|*
|* Garantiere eine gemeinsame Datenbasis (ClipKoordinaten oder
|* Devicekoordinaten)
|*
\************************************************************************/

void B3dEntity::ForceEqualBase(B3dTransformationSet* pSet, B3dEntity& rOld1,
    B3dEntity& rOld2)
{
    if(!IsDeviceCoor() && rOld1.IsDeviceCoor() && rOld2.IsDeviceCoor())
    {
        if(IsDeviceCoor())
            To3DCoor(pSet);
        if(rOld1.IsDeviceCoor())
            rOld1.To3DCoor(pSet);
        if(rOld2.IsDeviceCoor())
            rOld2.To3DCoor(pSet);
    }
}

/*************************************************************************
|*
|* Neuen Punkt an der stelle t des parametrisierten Vektors rOld1, rOld2
|* berechnen und fuellen
|*
\************************************************************************/

void B3dEntity::CalcInBetween(B3dEntity& rOld1, B3dEntity& rOld2, double t)
{
    // DeviceCoor der ersten Quelle benutzen, die Basis sollte
    // vorher abgeglichen sein
    SetDeviceCoor(rOld1.IsDeviceCoor());

    // Punktkoordinaten berechnen
    maPoint = interpolate(rOld1.Point(), rOld2.Point(), t);
    SetValid();

    // PlaneNormal Koordinaten berechnen
    rOld1.PlaneNormal().normalize();
    rOld2.PlaneNormal().normalize();
    maPlaneNormal = interpolate(rOld1.PlaneNormal(), rOld2.PlaneNormal(), t);
    maPlaneNormal.normalize();

    // Vektor berechnen
    if(rOld1.IsNormalUsed() && rOld2.IsNormalUsed())
    {
        rOld1.Normal().normalize();
        rOld2.Normal().normalize();
        maNormal = interpolate(rOld1.Normal(), rOld2.Normal(), t);
        maNormal.normalize();
        SetNormalUsed();
    }

    // Texturkoordinaten berechnen
    if(rOld1.IsTexCoorUsed() && rOld2.IsTexCoorUsed())
    {
        maTexCoor = interpolate(rOld1.TexCoor(), rOld2.TexCoor(), t);
        SetTexCoorUsed();
    }

    // EdgeVisible berechnen
    SetEdgeVisible(rOld1.IsEdgeVisible());

    // Farbe berechnen
    maColor.CalcInBetween(rOld1.Color(), rOld2.Color(), t);
}

/*************************************************************************
|*
|* Neuen Punkt in der Mitte des parametrisierten Vektors rOld1, rOld2
|* berechnen und fuellen
|*
\************************************************************************/

void B3dEntity::CalcMiddle(B3dEntity& rOld1, B3dEntity& rOld2)
{
    // DeviceCoor der ersten Quelle benutzen, die Basis sollte
    // vorher abgeglichen sein
    SetDeviceCoor(rOld1.IsDeviceCoor());

    // Punktkoordinaten berechnen
    maPoint = average(rOld1.Point(), rOld2.Point());
    SetValid();

    // PlaneNormal Koordinaten berechnen
    rOld1.PlaneNormal().normalize();
    rOld2.PlaneNormal().normalize();
    maPlaneNormal = average(rOld1.PlaneNormal(), rOld2.PlaneNormal());
    maPlaneNormal.normalize();

    // Vektor berechnen
    if(rOld1.IsNormalUsed() && rOld2.IsNormalUsed())
    {
        rOld1.Normal().normalize();
        rOld2.Normal().normalize();
        maNormal = average(rOld1.Normal(), rOld2.Normal());
        maNormal.normalize();
        SetNormalUsed();
    }

    // Texturkoordinaten berechnen
    if(rOld1.IsTexCoorUsed() && rOld2.IsTexCoorUsed())
    {
        maTexCoor = average(rOld1.TexCoor(), rOld2.TexCoor());
        SetTexCoorUsed();
    }

    // EdgeVisible berechnen
    SetEdgeVisible(rOld1.IsEdgeVisible());

    // Farbe berechnen
    maColor.CalcMiddle(rOld1.Color(), rOld2.Color());
}

/*************************************************************************
|*
|* Neuen Punkt in der Mitte des Dreiecks rOld1, rOld2, rOld3
|* berechnen und fuellen
|*
\************************************************************************/

void B3dEntity::CalcMiddle(B3dEntity& rOld1, B3dEntity& rOld2,
    B3dEntity& rOld3)
{
    // DeviceCoor der ersten Quelle benutzen, die Basis sollte
    // vorher abgeglichen sein
    SetDeviceCoor(rOld1.IsDeviceCoor());

    // Punktkoordinaten berechnen
    maPoint = average(rOld1.Point(), rOld2.Point(), rOld3.Point());
    SetValid();

    // PlaneNormal Koordinaten berechnen
    rOld1.PlaneNormal().normalize();
    rOld2.PlaneNormal().normalize();
    rOld3.PlaneNormal().normalize();
    maPlaneNormal = average(rOld1.PlaneNormal(), rOld2.PlaneNormal(), rOld3.PlaneNormal());
    maPlaneNormal.normalize();

    // Vektor berechnen
    if(rOld1.IsNormalUsed() && rOld2.IsNormalUsed() && rOld3.IsNormalUsed())
    {
        rOld1.Normal().normalize();
        rOld2.Normal().normalize();
        rOld3.Normal().normalize();
        maNormal = average(rOld1.Normal(), rOld2.Normal(), rOld3.Normal());
        maNormal.normalize();
        SetNormalUsed();
    }

    // Texturkoordinaten berechnen
    if(rOld1.IsTexCoorUsed() && rOld2.IsTexCoorUsed() && rOld3.IsTexCoorUsed())
    {
        maTexCoor = average(rOld1.TexCoor(), rOld2.TexCoor(), rOld3.TexCoor());
        SetTexCoorUsed();
    }

    // Farbe berechnen
    maColor.CalcMiddle(rOld1.Color(), rOld2.Color(), rOld3.Color());
}

/*************************************************************************
|*
|* Eine beliebige Transformation auf die Geometrie anwenden
|*
\************************************************************************/

void B3dEntity::Transform(const basegfx::B3DHomMatrix& rMat)
{
    maPoint *= rMat;

    if(mbNormalUsed)
    {
        maNormal *= rMat; // RotateAndNormalize
        maNormal.normalize();
    }
}

/*************************************************************************
|*
|* Bucket fuer geometrische Daten
|*
\************************************************************************/

BASE3D_IMPL_BUCKET(B3dEntity, Bucket)

// eof
