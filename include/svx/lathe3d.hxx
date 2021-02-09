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

#ifndef INCLUDED_SVX_LATHE3D_HXX
#define INCLUDED_SVX_LATHE3D_HXX

#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <svx/obj3d.hxx>
#include <svx/svxdllapi.h>
#include <svx/svddef.hxx>
#include <svx/svx3ditems.hxx>

class E3dDefaultAttributes;

/*************************************************************************
|*
|* Create a 3D rotation object from a passed 2D polygon
|*
|* The aPolyPoly3D is rotated around its axis in nHSegments steps.
|* nVSegments contains the number of lines of aPolyPoly3D and therefore
|* is effectively a vertical segmentation.
|*
\************************************************************************/

class SVXCORE_DLLPUBLIC E3dLatheObj final : public E3dCompoundObject
{
    basegfx::B2DPolyPolygon maPolyPoly2D;

    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;
    void SetDefaultAttributes(const E3dDefaultAttributes& rDefault);

private:
    // protected destructor - due to final, make private
    virtual ~E3dLatheObj() override;

public:
    E3dLatheObj(
        SdrModel& rSdrModel,
        const E3dDefaultAttributes& rDefault,
        const basegfx::B2DPolyPolygon& rPoly2D);
    E3dLatheObj(SdrModel& rSdrModel, E3dLatheObj const & rSource);
    E3dLatheObj(SdrModel& rSdrModel);

    // HorizontalSegments:
    sal_uInt32 GetHorizontalSegments() const
        { return GetObjectItemSet().Get(SDRATTR_3DOBJ_HORZ_SEGS).GetValue(); }

    // VerticalSegments:
    sal_uInt32 GetVerticalSegments() const
        { return GetObjectItemSet().Get(SDRATTR_3DOBJ_VERT_SEGS).GetValue(); }

    // PercentDiagonal: 0..100, before 0.0..0.5
    sal_uInt16 GetPercentDiagonal() const
        { return GetObjectItemSet().Get(SDRATTR_3DOBJ_PERCENT_DIAGONAL).GetValue(); }

    // BackScale: 0..100, before 0.0..1.0
    sal_uInt16 GetBackScale() const
        { return GetObjectItemSet().Get(SDRATTR_3DOBJ_BACKSCALE).GetValue(); }

    // EndAngle: 0..10000
    sal_uInt32 GetEndAngle() const
        { return GetObjectItemSet().Get(SDRATTR_3DOBJ_END_ANGLE).GetValue(); }

    // #107245# GetSmoothNormals() for bLatheSmoothed
    bool GetSmoothNormals() const
        { return GetObjectItemSet().Get(SDRATTR_3DOBJ_SMOOTH_NORMALS).GetValue(); }

    // #107245# GetSmoothLids() for bLatheSmoothFrontBack
    bool GetSmoothLids() const
        { return GetObjectItemSet().Get(SDRATTR_3DOBJ_SMOOTH_LIDS).GetValue(); }

    // #107245# GetCharacterMode() for bLatheCharacterMode
    bool GetCharacterMode() const
        { return GetObjectItemSet().Get(SDRATTR_3DOBJ_CHARACTER_MODE).GetValue(); }

    // #107245# GetCloseFront() for bLatheCloseFront
    bool GetCloseFront() const
        { return GetObjectItemSet().Get(SDRATTR_3DOBJ_CLOSE_FRONT).GetValue(); }

    // #107245# GetCloseBack() for bLatheCloseBack
    bool GetCloseBack() const
        { return GetObjectItemSet().Get(SDRATTR_3DOBJ_CLOSE_BACK).GetValue(); }

    virtual SdrObjKind GetObjIdentifier() const override;

    virtual E3dLatheObj* CloneSdrObject(SdrModel& rTargetModel) const override;

    virtual SdrObjectUniquePtr DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    // TakeObjName...() is for the display in the UI, for example "3 frames selected".
    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    // set/get local parameters with geometry recreation
    void SetPolyPoly2D(const basegfx::B2DPolyPolygon& rNew);
    const basegfx::B2DPolyPolygon& GetPolyPoly2D() const { return maPolyPoly2D; }

    // break up
    virtual bool IsBreakObjPossible() override;
    virtual std::unique_ptr<SdrAttrObj,SdrObjectFreeOp> GetBreakObj() override;
};

#endif // INCLUDED_SVX_LATHE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
