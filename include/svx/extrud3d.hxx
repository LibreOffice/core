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

#ifndef INCLUDED_SVX_EXTRUD3D_HXX
#define INCLUDED_SVX_EXTRUD3D_HXX

#include <svx/obj3d.hxx>
#include <svx/svxdllapi.h>

/*************************************************************************
|*
|* 3D extrusion object created from the provided 2D polygon
|*
\************************************************************************/

class SVX_DLLPUBLIC SAL_WARN_UNUSED E3dExtrudeObj : public E3dCompoundObject
{
private:
    // to allow sdr::properties::E3dExtrudeProperties access to SetGeometryValid()
    friend class sdr::properties::E3dExtrudeProperties;

    // geometry, which determines the object
    basegfx::B2DPolyPolygon         maExtrudePolygon;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() override;
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

public:
    TYPEINFO_OVERRIDE();

    E3dExtrudeObj(E3dDefaultAttributes& rDefault, const basegfx::B2DPolyPolygon& rPP, double fDepth);
    E3dExtrudeObj();

    // PercentDiagonal: 0..100, before 0.0..0.5
    sal_uInt16 GetPercentDiagonal() const
        { return static_cast<const SfxUInt16Item&>(GetObjectItemSet().Get(SDRATTR_3DOBJ_PERCENT_DIAGONAL)).GetValue(); }

    // BackScale: 0..100, before 0.0..1.0
    sal_uInt16 GetPercentBackScale() const
        { return static_cast<const SfxUInt16Item&>(GetObjectItemSet().Get(SDRATTR_3DOBJ_BACKSCALE)).GetValue(); }

    // BackScale: 0..100, before 0.0..1.0
    sal_uInt32 GetExtrudeDepth() const
        { return static_cast<const SfxUInt32Item&>(GetObjectItemSet().Get(SDRATTR_3DOBJ_DEPTH)).GetValue(); }

    // #107245# GetSmoothNormals() for bExtrudeSmoothed
    bool GetSmoothNormals() const
        { return static_cast<const Svx3DSmoothNormalsItem&>(GetObjectItemSet().Get(SDRATTR_3DOBJ_SMOOTH_NORMALS)).GetValue(); }

    // #107245# GetSmoothLids() for bExtrudeSmoothFrontBack
    bool GetSmoothLids() const
        { return static_cast<const Svx3DSmoothLidsItem&>(GetObjectItemSet().Get(SDRATTR_3DOBJ_SMOOTH_LIDS)).GetValue(); }

    // #107245# GetCharacterMode() for bExtrudeCharacterMode
    bool GetCharacterMode() const
        { return static_cast<const Svx3DCharacterModeItem&>(GetObjectItemSet().Get(SDRATTR_3DOBJ_CHARACTER_MODE)).GetValue(); }

    // #107245# GetCloseFront() for bExtrudeCloseFront
    bool GetCloseFront() const
        { return static_cast<const Svx3DCloseFrontItem&>(GetObjectItemSet().Get(SDRATTR_3DOBJ_CLOSE_FRONT)).GetValue(); }

    // #107245# GetCloseBack() for bExtrudeCloseBack
    bool GetCloseBack() const
        { return static_cast<const Svx3DCloseBackItem&>(GetObjectItemSet().Get(SDRATTR_3DOBJ_CLOSE_BACK)).GetValue(); }

    virtual sal_uInt16 GetObjIdentifier() const override;

    virtual E3dExtrudeObj* Clone() const override;

    // TakeObjName...() is for the display in the UI (for example "3 frames selected")
    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    // set/get local parameters with geometry regeneration
    void SetExtrudePolygon(const basegfx::B2DPolyPolygon &rNew);
    const basegfx::B2DPolyPolygon &GetExtrudePolygon() const { return maExtrudePolygon; }

    virtual bool IsBreakObjPossible() override;
    virtual SdrAttrObj* GetBreakObj() override;
};

#endif // INCLUDED_SVX_EXTRUD3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
