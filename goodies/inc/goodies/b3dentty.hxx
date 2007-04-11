/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dentty.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:32:22 $
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

#ifndef _B3D_B3DENTITY_HXX
#define _B3D_B3DENTITY_HXX

#ifndef _B3D_B3DCOLOR_HXX
#include <goodies/b3dcolor.hxx>
#endif

#ifndef _B3D_BUCKET_HXX
#include <goodies/bucket.hxx>
#endif

#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#include <basegfx/vector/b3dvector.hxx>
#endif

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
