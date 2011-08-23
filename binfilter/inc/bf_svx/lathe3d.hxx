/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _E3D_LATHE3D_HXX
#define _E3D_LATHE3D_HXX

#include <bf_svx/obj3d.hxx>

#include <bf_svx/poly3d.hxx>
namespace binfilter {

/*************************************************************************
|*
|* 3D-Rotationsobjekt aus uebergebenem 2D-Polygon erzeugen
|*
|* Das aPolyPoly3D wird in nHSegments-Schritten um die Achse rotiert.
|* nVSegments gibt die Anzahl der Linien von aPolyPoly3D an und stellt damit
|* quasi eine vertikale Segmentierung dar.
|*
\************************************************************************/

class E3dLatheObj : public E3dCompoundObject
{
    // Partcodes fuer Wireframe-Generierung: Standard oder Deckelflaeche
    enum { LATHE_PART_STD = 1, LATHE_PART_COVER = 2 };
    PolyPolygon3D	aPolyPoly3D;
    double			fLatheScale;

    // #78972#
    PolyPolygon3D	maLinePolyPolygon;

    // #107245# unsigned		bLatheSmoothed				: 1;
    // #107245# unsigned		bLatheSmoothFrontBack		: 1;
    // #107245# unsigned		bLatheCharacterMode			: 1;
    // #107245# unsigned		bLatheCloseFront			: 1;
    // #107245# unsigned		bLatheCloseBack				: 1;

 protected:
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

    PolyPolygon3D CreateLathePolyPoly(PolyPolygon3D& rPoly3D, long nVSegs);
    Polygon3D CreateLathePoly(Polygon3D& rPoly3D, long nVSegs);

 public:
    TYPEINFO();
    E3dLatheObj(E3dDefaultAttributes& rDefault, const PolyPolygon& rPoly);
    E3dLatheObj();

    // HorizontalSegments:
    sal_Int32 GetHorizontalSegments() const 
        { return ((const Svx3DHorizontalSegmentsItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_HORZ_SEGS)).GetValue(); }

    // VerticalSegments:
    sal_Int32 GetVerticalSegments() const 
        { return ((const Svx3DVerticalSegmentsItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_VERT_SEGS)).GetValue(); }

    // PercentDiagonal: 0..100, before 0.0..0.5
    sal_uInt16 GetPercentDiagonal() const 
        { return ((const Svx3DPercentDiagonalItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_PERCENT_DIAGONAL)).GetValue(); }

    // BackScale: 0..100, before 0.0..1.0
    sal_uInt16 GetBackScale() const 
        { return ((const Svx3DBackscaleItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_BACKSCALE)).GetValue(); }

    // EndAngle: 0..10000
    sal_uInt32 GetEndAngle() const 
        { return ((const Svx3DEndAngleItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_END_ANGLE)).GetValue(); }

    // #107245# GetSmoothNormals() for bLatheSmoothed
    sal_Bool GetSmoothNormals() const 
        { return ((const Svx3DSmoothNormalsItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_SMOOTH_NORMALS)).GetValue(); }

    // #107245# GetSmoothLids() for bLatheSmoothFrontBack
    sal_Bool GetSmoothLids() const 
        { return ((const Svx3DSmoothLidsItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_SMOOTH_LIDS)).GetValue(); }

    // #107245# GetCharacterMode() for bLatheCharacterMode
    sal_Bool GetCharacterMode() const 
        { return ((const Svx3DCharacterModeItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_CHARACTER_MODE)).GetValue(); }

    // #107245# GetCloseFront() for bLatheCloseFront
    sal_Bool GetCloseFront() const 
        { return ((const Svx3DCloseFrontItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_CLOSE_FRONT)).GetValue(); }

    // #107245# GetCloseBack() for bLatheCloseBack
    sal_Bool GetCloseBack() const 
        { return ((const Svx3DCloseBackItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_CLOSE_BACK)).GetValue(); }

    virtual UINT16 GetObjIdentifier() const;

    virtual void CreateWireframe(Polygon3D& rWirePoly, const Matrix4D* pTf = NULL,
        E3dDragDetail eDetail = E3DDETAIL_DEFAULT);

    const   PolyPolygon3D& GetPolyPolygon() const { return aPolyPoly3D; }

    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);


    // Geometrieerzeugung
    virtual void CreateGeometry();

    // Give out simple line geometry

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".

    // Lokale Parameter setzen/lesen mit Geometrieneuerzeugung
    void SetPolyPoly3D(const PolyPolygon3D& rNew);
    const PolyPolygon3D& GetPolyPoly3D() { return aPolyPoly3D; }

    double GetLatheScale() const { return fLatheScale; }

    // #107245# 
    // void SetLatheSmoothed(BOOL bNew);
    // BOOL GetLatheSmoothed() const { return bLatheSmoothed; }
    // void SetLatheSmoothFrontBack(BOOL bNew);
    // BOOL GetLatheSmoothFrontBack() const { return bLatheSmoothFrontBack; }
    // void SetLatheCharacterMode(BOOL bNew);
    // BOOL GetLatheCharacterMode() const { return bLatheCharacterMode; }
    // void SetLatheCloseFront(BOOL bNew);
    // BOOL GetLatheCloseFront() const { return bLatheCloseFront; }
    // void SetLatheCloseBack(BOOL bNew);
    // BOOL GetLatheCloseBack() const { return bLatheCloseBack; }

    // private support routines for ItemSet access. NULL pointer means clear item.
    virtual void PostItemChange(const sal_uInt16 nWhich);

    // Aufbrechen
};

}//end of namespace binfilter
#endif			// _E3D_LATHE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
