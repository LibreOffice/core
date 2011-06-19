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

#include <svx/obj3d.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* Create a 3D rotation object from a passed 2D polygon
|*
|* The aPolyPoly3D is rotated around its axis in nHSegments steps.
|* nVSegments contains the number of lines of aPolyPoly3D and therefore
|* is effectively a vertical segmentation.
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dLatheObj : public E3dCompoundObject
{
private:
    // Part codes for Wireframe generation: standard oder cover surface
    enum { LATHE_PART_STD = 1, LATHE_PART_COVER = 2 };
    basegfx::B2DPolyPolygon maPolyPoly2D;

 protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

 public:
    TYPEINFO();
    E3dLatheObj(E3dDefaultAttributes& rDefault, const basegfx::B2DPolyPolygon rPoly2D);
    E3dLatheObj();

    // HorizontalSegments:
    sal_uInt32 GetHorizontalSegments() const
        { return ((const Svx3DHorizontalSegmentsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_HORZ_SEGS)).GetValue(); }

    // VerticalSegments:
    sal_uInt32 GetVerticalSegments() const
        { return ((const Svx3DVerticalSegmentsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_VERT_SEGS)).GetValue(); }

    // PercentDiagonal: 0..100, before 0.0..0.5
    sal_uInt16 GetPercentDiagonal() const
        { return ((const Svx3DPercentDiagonalItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_PERCENT_DIAGONAL)).GetValue(); }

    // BackScale: 0..100, before 0.0..1.0
    sal_uInt16 GetBackScale() const
        { return ((const Svx3DBackscaleItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_BACKSCALE)).GetValue(); }

    // EndAngle: 0..10000
    sal_uInt32 GetEndAngle() const
        { return ((const Svx3DEndAngleItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_END_ANGLE)).GetValue(); }

    // #107245# GetSmoothNormals() for bLatheSmoothed
    sal_Bool GetSmoothNormals() const
        { return ((const Svx3DSmoothNormalsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_SMOOTH_NORMALS)).GetValue(); }

    // #107245# GetSmoothLids() for bLatheSmoothFrontBack
    sal_Bool GetSmoothLids() const
        { return ((const Svx3DSmoothLidsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_SMOOTH_LIDS)).GetValue(); }

    // #107245# GetCharacterMode() for bLatheCharacterMode
    sal_Bool GetCharacterMode() const
        { return ((const Svx3DCharacterModeItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_CHARACTER_MODE)).GetValue(); }

    // #107245# GetCloseFront() for bLatheCloseFront
    sal_Bool GetCloseFront() const
        { return ((const Svx3DCloseFrontItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_CLOSE_FRONT)).GetValue(); }

    // #107245# GetCloseBack() for bLatheCloseBack
    sal_Bool GetCloseBack() const
        { return ((const Svx3DCloseBackItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_CLOSE_BACK)).GetValue(); }

    virtual sal_uInt16 GetObjIdentifier() const;
    void    ReSegment(sal_uInt32 nHSegs, sal_uInt32 nVSegs);

    virtual E3dLatheObj* Clone() const;

    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier) const;

    // TakeObjName...() is for the display in the UI, for example "3 frames selected".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    // set/get local parameters with geometry recreation
    void SetPolyPoly2D(const basegfx::B2DPolyPolygon& rNew);
    const basegfx::B2DPolyPolygon& GetPolyPoly2D() { return maPolyPoly2D; }

    // break up
    virtual sal_Bool IsBreakObjPossible();
    virtual SdrAttrObj* GetBreakObj();
};

#endif          // _E3D_LATHE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
