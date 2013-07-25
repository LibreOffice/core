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

#ifndef _E3D_EXTRUD3D_HXX
#define _E3D_EXTRUD3D_HXX

#include <svx/obj3d.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* 3D extrusion object created from the provided 2D polygon
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dExtrudeObj : public E3dCompoundObject
{
private:
    // to allow sdr::properties::E3dExtrudeProperties access to SetGeometryValid()
    friend class sdr::properties::E3dExtrudeProperties;

    // geometry, which determines the object
    basegfx::B2DPolyPolygon         maExtrudePolygon;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

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

    virtual sal_uInt16 GetObjIdentifier() const;

    virtual E3dExtrudeObj* Clone() const;

    // TakeObjName...() is for the display in the UI (for example "3 frames selected")
    virtual OUString TakeObjNameSingul() const;
    virtual OUString TakeObjNamePlural() const;

    // set/get local parameters with geometry regeneration
    void SetExtrudePolygon(const basegfx::B2DPolyPolygon &rNew);
    const basegfx::B2DPolyPolygon &GetExtrudePolygon() { return maExtrudePolygon; }

    virtual bool IsBreakObjPossible();
    virtual SdrAttrObj* GetBreakObj();
};

#endif          // _E3D_EXTRUD3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
