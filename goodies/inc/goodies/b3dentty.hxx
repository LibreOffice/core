/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dentty.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _B3D_B3DENTITY_HXX
#define _B3D_B3DENTITY_HXX

#include <goodies/b3dcolor.hxx>
#include <goodies/bucket.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>

/*************************************************************************
|*
|* Merkt sich einen Punkt plus Normale plus Texturkoordinate
|* (in Zukunft noch weitere Attribute, bsp. Farbe, Kante, Linienmodus
|*
\************************************************************************/

// predeclarations
class B3dTransformationSet;
namespace basegfx
{
    class B3DHomMatrix;
} // end of namespace basegfx

class B3dEntity
{
private:
    // Data defining this point, it's normal and texture coordinates
    basegfx::B3DPoint       maPoint;
    basegfx::B3DVector  maNormal;
    basegfx::B3DVector  maPlaneNormal;
    basegfx::B2DPoint       maTexCoor;
    B3dColor                maColor;

    // Ist die diesem Punkt folgende Kante sichtbar ?
    // EdgeFlag bool NOT in bitfield to be able to access it from OpenGL driver
    sal_uInt8               mbEdgeFlag;

    // bitfield
    // Gueltigkeit der geometrischen Daten
    unsigned                mbValid         : 1;

    // Wird eine Normale verwendet ?
    unsigned                mbNormalUsed    : 1;

    // Wird eine Texturkoordinate verwendet ?
    unsigned                mbTexCoorUsed   : 1;

    // Sind die geometrischen Daten schon in DeviceKoordinaten
    // umgerechnet ?
    unsigned                mbDeviceCoor    : 1;

public:
    B3dEntity() { Reset(); }

    sal_Bool IsValid() const { return mbValid; }
    void SetValid(sal_Bool bNew=sal_True) { mbValid = bNew; }

    sal_Bool IsNormalUsed() const { return mbNormalUsed; }
    void SetNormalUsed(sal_Bool bNew=sal_True) { mbNormalUsed = bNew; }

    sal_Bool IsTexCoorUsed() const { return mbTexCoorUsed; }
    void SetTexCoorUsed(sal_Bool bNew=sal_True) { mbTexCoorUsed = bNew; }

    sal_Bool IsDeviceCoor() const { return mbDeviceCoor; }
    void SetDeviceCoor(sal_Bool bNew=sal_True) { mbDeviceCoor = bNew; }

    sal_Bool IsEdgeVisible() const { return mbEdgeFlag; }
    void SetEdgeVisible(sal_Bool bNew=sal_True) { mbEdgeFlag = bNew; }

    basegfx::B3DPoint& Point() { return maPoint; }
    const basegfx::B3DPoint& Point() const { return maPoint; }
    basegfx::B3DVector& Normal() { return maNormal; }
    const basegfx::B3DVector& Normal() const { return maNormal; }
    basegfx::B3DVector& PlaneNormal() { return maPlaneNormal; }
    const basegfx::B3DVector& PlaneNormal() const { return maPlaneNormal; }
    basegfx::B2DPoint& TexCoor() { return maTexCoor; }
    const basegfx::B2DPoint& TexCoor() const { return maTexCoor; }
    B3dColor& Color() { return maColor; }
    const B3dColor& Color() const { return maColor; }
    sal_uInt8& EdgeFlag() { return mbEdgeFlag; }
    const sal_uInt8& EdgeFlag() const { return mbEdgeFlag; }

    double GetX() const { return maPoint.getX(); }
    void SetX(double fNew) { maPoint.setX(fNew); }

    double GetY() const { return maPoint.getY(); }
    void SetY(double fNew) { maPoint.setY(fNew); }

    double GetZ() const { return maPoint.getZ(); }
    void SetZ(double fNew) { maPoint.setZ(fNew); }

    void Reset();

    void Copy(B3dEntity& rEnt);
    void ToDeviceCoor(B3dTransformationSet* pSet)
        { if(!IsDeviceCoor()) ImplToDeviceCoor(pSet); }
    void To3DCoor(B3dTransformationSet* pSet)
        { if(IsDeviceCoor()) ImplTo3DCoor(pSet); }

    void ForceEqualBase(B3dTransformationSet* pSet, B3dEntity& rOld);
    void ForceEqualBase(B3dTransformationSet* pSet, B3dEntity& rOld1,
        B3dEntity& rOld2);
    void CalcInBetween(B3dEntity& rOld1,
        B3dEntity& rOld2, double t);
    void CalcMiddle(B3dEntity& rOld1,
        B3dEntity& rOld2);
    void CalcMiddle(B3dEntity& rOld1,
        B3dEntity& rOld2, B3dEntity& rOld3);

    // Eine beliebige Transformation auf die Geometrie anwenden
    void Transform(const basegfx::B3DHomMatrix& rMat);

protected:
    void ImplToDeviceCoor(B3dTransformationSet* pSet);
    void ImplTo3DCoor(B3dTransformationSet* pSet);
};

/*************************************************************************
|*
|* Bucket fuer geometrische Daten
|*
\************************************************************************/

BASE3D_DECL_BUCKET(B3dEntity, Bucket)

#endif          // _B3D_B3DENTITY_HXX
