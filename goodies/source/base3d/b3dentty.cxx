/*************************************************************************
 *
 *  $RCSfile: b3dentty.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    aPoint = rEnt.Point();
    bDeviceCoor = rEnt.IsDeviceCoor();
    bValid = rEnt.IsValid();
    bEdgeFlag = rEnt.IsEdgeVisible();
    aPlaneNormal = rEnt.PlaneNormal();

    if(bNormalUsed = rEnt.IsNormalUsed())
        aNormal = rEnt.Normal();

    if(bTexCoorUsed = rEnt.IsTexCoorUsed())
        aTexCoor = rEnt.TexCoor();

    aColor = rEnt.Color();
}

/*************************************************************************
|*
|* Flags auf Ausgangsposition
|*
\************************************************************************/

void B3dEntity::Reset()
{
    bValid = bNormalUsed = bTexCoorUsed = bDeviceCoor = FALSE;
    bEdgeFlag = TRUE;
}

/*************************************************************************
|*
|* Device Koordinaten des Punktes berechnen
|*
\************************************************************************/

void B3dEntity::ImplToDeviceCoor(B3dTransformationSet* pSet)
{
    if(pSet && !bDeviceCoor)
    {
        const Vector3D& rScale = pSet->GetScale();
        const Vector3D& rTrans = pSet->GetTranslate();

        aPoint.Homogenize();
        aPoint[0] = (aPoint[0] * rScale[0]) + rTrans[0];
        aPoint[1] = (aPoint[1] * rScale[1]) + rTrans[1];
        aPoint[2] = (aPoint[2] * rScale[2]) + rTrans[2];

        bDeviceCoor = TRUE;
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
    if(pSet && bDeviceCoor)
    {
        const Vector3D& rScale = pSet->GetScale();
        const Vector3D& rTrans = pSet->GetTranslate();

        aPoint.Homogenize();
        if(rScale[0] != 0.0)
            aPoint[0] = (aPoint[0] - rTrans[0]) / rScale[0];
        if(rScale[1] != 0.0)
            aPoint[1] = (aPoint[1] - rTrans[1]) / rScale[1];
        if(rScale[2] != 0.0)
            aPoint[2] = (aPoint[2] - rTrans[2]) / rScale[2];

        bDeviceCoor = FALSE;
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
    aPoint.CalcInBetween(rOld1.Point(), rOld2.Point(), t);
    SetValid();

    // PlaneNormal Koordinaten berechnen
    rOld1.PlaneNormal().Normalize();
    rOld2.PlaneNormal().Normalize();
    aPlaneNormal.CalcInBetween(rOld1.PlaneNormal(), rOld2.PlaneNormal(), t);
    aPlaneNormal.Normalize();

    // Vektor berechnen
    if(rOld1.IsNormalUsed() && rOld2.IsNormalUsed())
    {
        rOld1.Normal().Normalize();
        rOld2.Normal().Normalize();
        aNormal.CalcInBetween(rOld1.Normal(), rOld2.Normal(), t);
        aNormal.Normalize();
        SetNormalUsed();
    }

    // Texturkoordinaten berechnen
    if(rOld1.IsTexCoorUsed() && rOld2.IsTexCoorUsed())
    {
        aTexCoor.CalcInBetween(rOld1.TexCoor(), rOld2.TexCoor(), t);
        SetTexCoorUsed();
    }

    // EdgeVisible berechnen
    SetEdgeVisible(rOld1.IsEdgeVisible());

    // Farbe berechnen
    aColor.CalcInBetween(rOld1.Color(), rOld2.Color(), t);
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
    aPoint.CalcMiddle(rOld1.Point(), rOld2.Point());
    SetValid();

    // PlaneNormal Koordinaten berechnen
    rOld1.PlaneNormal().Normalize();
    rOld2.PlaneNormal().Normalize();
    aPlaneNormal.CalcMiddle(rOld1.PlaneNormal(), rOld2.PlaneNormal());
    aPlaneNormal.Normalize();

    // Vektor berechnen
    if(rOld1.IsNormalUsed() && rOld2.IsNormalUsed())
    {
        rOld1.Normal().Normalize();
        rOld2.Normal().Normalize();
        aNormal.CalcMiddle(rOld1.Normal(), rOld2.Normal());
        aNormal.Normalize();
        SetNormalUsed();
    }

    // Texturkoordinaten berechnen
    if(rOld1.IsTexCoorUsed() && rOld2.IsTexCoorUsed())
    {
        aTexCoor.CalcMiddle(rOld1.TexCoor(), rOld2.TexCoor());
        SetTexCoorUsed();
    }

    // EdgeVisible berechnen
    SetEdgeVisible(rOld1.IsEdgeVisible());

    // Farbe berechnen
    aColor.CalcMiddle(rOld1.Color(), rOld2.Color());
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
    aPoint.CalcMiddle(rOld1.Point(), rOld2.Point(), rOld3.Point());
    SetValid();

    // PlaneNormal Koordinaten berechnen
    rOld1.PlaneNormal().Normalize();
    rOld2.PlaneNormal().Normalize();
    rOld3.PlaneNormal().Normalize();
    aPlaneNormal.CalcMiddle(rOld1.PlaneNormal(), rOld2.PlaneNormal(), rOld3.PlaneNormal());
    aPlaneNormal.Normalize();

    // Vektor berechnen
    if(rOld1.IsNormalUsed() && rOld2.IsNormalUsed() && rOld3.IsNormalUsed())
    {
        rOld1.Normal().Normalize();
        rOld2.Normal().Normalize();
        rOld3.Normal().Normalize();
        aNormal.CalcMiddle(rOld1.Normal(), rOld2.Normal(), rOld3.Normal());
        aNormal.Normalize();
        SetNormalUsed();
    }

    // Texturkoordinaten berechnen
    if(rOld1.IsTexCoorUsed() && rOld2.IsTexCoorUsed() && rOld3.IsTexCoorUsed())
    {
        aTexCoor.CalcMiddle(rOld1.TexCoor(), rOld2.TexCoor(), rOld3.TexCoor());
        SetTexCoorUsed();
    }

    // Farbe berechnen
    aColor.CalcMiddle(rOld1.Color(), rOld2.Color(), rOld3.Color());
}

/*************************************************************************
|*
|* Eine beliebige Transformation auf die Geometrie anwenden
|*
\************************************************************************/

void B3dEntity::Transform(const Matrix4D& rMat)
{
    aPoint *= rMat;
    if(bNormalUsed)
        rMat.RotateAndNormalize(aNormal);
}

/*************************************************************************
|*
|* Bucket fuer geometrische Daten
|*
\************************************************************************/

BASE3D_IMPL_BUCKET(B3dEntity, Bucket)


