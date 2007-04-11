/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: extrud3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:48:29 $
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

#ifndef _E3D_EXTRUD3D_HXX
#define _E3D_EXTRUD3D_HXX

#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

/*************************************************************************
|*
|* 3D-Extrusionsobjekt aus uebergebenem 2D-Polygon erzeugen
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dExtrudeObj : public E3dCompoundObject
{
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    // to allow sdr::properties::E3dExtrudeProperties access to SetGeometryValid()
    friend class sdr::properties::E3dExtrudeProperties;

private:
    // Geometrie, die dieses Objekt bestimmt
    basegfx::B2DPolyPolygon         maExtrudePolygon;

    // #78972#
    basegfx::B3DPolyPolygon         maLinePolyPolygon;

protected:
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);
    basegfx::B3DPolyPolygon GetFrontSide();
    basegfx::B3DPolyPolygon GetBackSide(const basegfx::B3DPolyPolygon& rFrontSide);

public:
    TYPEINFO();

    E3dExtrudeObj(E3dDefaultAttributes& rDefault, const basegfx::B2DPolyPolygon& rPP, double fDepth);
    E3dExtrudeObj();

    // PercentDiagonal: 0..100, before 0.0..0.5
    sal_uInt16 GetPercentDiagonal() const
        { return ((const Svx3DPercentDiagonalItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_PERCENT_DIAGONAL)).GetValue(); }

    // BackScale: 0..100, before 0.0..1.0
    sal_uInt16 GetPercentBackScale() const
        { return ((const Svx3DBackscaleItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_BACKSCALE)).GetValue(); }

    // BackScale: 0..100, before 0.0..1.0
    sal_uInt32 GetExtrudeDepth() const
        { return ((const Svx3DDepthItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_DEPTH)).GetValue(); }

    // #107245# GetSmoothNormals() for bExtrudeSmoothed
    sal_Bool GetSmoothNormals() const
        { return ((const Svx3DSmoothNormalsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_SMOOTH_NORMALS)).GetValue(); }

    // #107245# GetSmoothLids() for bExtrudeSmoothFrontBack
    sal_Bool GetSmoothLids() const
        { return ((const Svx3DSmoothLidsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_SMOOTH_LIDS)).GetValue(); }

    // #107245# GetCharacterMode() for bExtrudeCharacterMode
    sal_Bool GetCharacterMode() const
        { return ((const Svx3DCharacterModeItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_CHARACTER_MODE)).GetValue(); }

    // #107245# GetCloseFront() for bExtrudeCloseFront
    sal_Bool GetCloseFront() const
        { return ((const Svx3DCloseFrontItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_CLOSE_FRONT)).GetValue(); }

    // #107245# GetCloseBack() for bExtrudeCloseBack
    sal_Bool GetCloseBack() const
        { return ((const Svx3DCloseBackItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_CLOSE_BACK)).GetValue(); }

    virtual UINT16 GetObjIdentifier() const;

    virtual void operator=(const SdrObject&);

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    // Geometrieerzeugung
    virtual void CreateGeometry();

    // Give out simple line geometry
    virtual basegfx::B3DPolyPolygon Get3DLineGeometry() const;

    // Lokale Parameter setzen/lesen mit Geometrieneuerzeugung
    void SetExtrudePolygon(const basegfx::B2DPolyPolygon &rNew);
    const basegfx::B2DPolyPolygon &GetExtrudePolygon() { return maExtrudePolygon; }

    // Aufbrechen
    virtual BOOL IsBreakObjPossible();
    virtual SdrAttrObj* GetBreakObj();
};

#endif          // _E3D_EXTRUD3D_HXX

