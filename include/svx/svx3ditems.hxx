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
#ifndef INCLUDED_SVX_SVX3DITEMS_HXX
#define INCLUDED_SVX_SVX3DITEMS_HXX

#include <svl/intitem.hxx>
#include <svl/eitem.hxx>


#include <editeng/colritem.hxx>
#include <svx/e3ditem.hxx>
#include <svx/viewpt3d.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdllapi.h>

// Svx3D _3DOBJ_ Items
inline SfxUInt16Item makeSvx3DPercentDiagonalItem(sal_uInt16 nVal) {
    return SfxUInt16Item(SDRATTR_3DOBJ_PERCENT_DIAGONAL, nVal);
}

inline SfxUInt16Item makeSvx3DBackscaleItem(sal_uInt16 nVal) {
    return SfxUInt16Item(SDRATTR_3DOBJ_BACKSCALE, nVal);
}

inline SfxUInt32Item makeSvx3DDepthItem(sal_uInt32 nVal) {
    return SfxUInt32Item(SDRATTR_3DOBJ_DEPTH, nVal);
}

inline SfxUInt32Item makeSvx3DHorizontalSegmentsItem(sal_uInt32 nVal) {
    return SfxUInt32Item(SDRATTR_3DOBJ_HORZ_SEGS, nVal);
}

inline SfxUInt32Item makeSvx3DVerticalSegmentsItem(sal_uInt32 nVal) {
    return SfxUInt32Item(SDRATTR_3DOBJ_VERT_SEGS, nVal);
}

inline SfxUInt32Item makeSvx3DEndAngleItem(sal_uInt32 nVal) {
    return SfxUInt32Item(SDRATTR_3DOBJ_END_ANGLE, nVal);
}

inline SfxBoolItem makeSvx3DDoubleSidedItem(bool bVal) {
    return SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, bVal);
}

// #i28528#
// Added extra Item (Bool) for chart2 to be able to show reduced line geometry
class Svx3DReducedLineGeometryItem : public SfxBoolItem {
public:
    Svx3DReducedLineGeometryItem(bool bVal = false);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const override;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const override;
    SfxPoolItem * Clone(SfxItemPool * = nullptr) const override;
};

class SVX_DLLPUBLIC Svx3DNormalsKindItem : public SfxUInt16Item {
public:
    Svx3DNormalsKindItem(sal_uInt16 nVal = 0);

    // use drawing::NormalsKind
    SVX_DLLPRIVATE virtual  bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    SVX_DLLPRIVATE virtual  bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
};

inline SfxBoolItem makeSvx3DNormalsInvertItem(bool bVal) {
    return SfxBoolItem(SDRATTR_3DOBJ_NORMALS_INVERT, bVal);
}

class SVX_DLLPUBLIC Svx3DTextureProjectionXItem : public SfxUInt16Item {
public:
    Svx3DTextureProjectionXItem(sal_uInt16 nVal = 0);

    // use drawing::TextureProjectionMode
    SVX_DLLPRIVATE virtual  bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    SVX_DLLPRIVATE virtual  bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
};

class SVX_DLLPUBLIC Svx3DTextureProjectionYItem : public SfxUInt16Item {
public:
    Svx3DTextureProjectionYItem(sal_uInt16 nVal = 0);

    // use drawing::TextureProjectionMode
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
};

inline SfxBoolItem makeSvx3DShadow3DItem(bool bVal) {
    return SfxBoolItem(SDRATTR_3DOBJ_SHADOW_3D, bVal);
}

inline SvxColorItem makeSvx3DMaterialEmissionItem(const Color& rCol) {
    return SvxColorItem(rCol, SDRATTR_3DOBJ_MAT_EMISSION);
}

inline SvxColorItem makeSvx3DMaterialSpecularItem(const Color& rCol) {
    return SvxColorItem(rCol, SDRATTR_3DOBJ_MAT_SPECULAR);
}

inline SfxUInt16Item makeSvx3DMaterialSpecularIntensityItem(sal_uInt16 nVal) {
    return SfxUInt16Item(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY, nVal);
}

class SVX_DLLPUBLIC Svx3DTextureKindItem : public SfxUInt16Item {
public:
    Svx3DTextureKindItem(sal_uInt16 nVal = 3);

    // use drawing::TextureKind
    SVX_DLLPRIVATE virtual  bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    SVX_DLLPRIVATE virtual  bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
};

class SVX_DLLPUBLIC Svx3DTextureModeItem : public SfxUInt16Item {
public:
    Svx3DTextureModeItem(sal_uInt16 nVal = 2);

    // use drawing:TextureMode
    SVX_DLLPRIVATE virtual  bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    SVX_DLLPRIVATE virtual  bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
};

inline SfxBoolItem makeSvx3DTextureFilterItem(bool bVal) {
    return SfxBoolItem(SDRATTR_3DOBJ_TEXTURE_FILTER, bVal);
}

// Svx3D _3DSCENE_ Items
class SVX_DLLPUBLIC Svx3DPerspectiveItem : public SfxUInt16Item {
public:
    Svx3DPerspectiveItem(sal_uInt16 nVal = (sal_uInt16)PR_PERSPECTIVE);

    // use drawing::ProjectionMode
    SVX_DLLPRIVATE virtual  bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    SVX_DLLPRIVATE virtual  bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
};

inline SfxUInt32Item makeSvx3DDistanceItem(sal_uInt32 nVal) {
    return SfxUInt32Item(SDRATTR_3DSCENE_DISTANCE, nVal);
}

inline SfxUInt32Item makeSvx3DFocalLengthItem(sal_uInt32 nVal) {
    return SfxUInt32Item(SDRATTR_3DSCENE_FOCAL_LENGTH, nVal);
}

inline SfxBoolItem makeSvx3DTwoSidedLightingItem(bool bVal) {
    return SfxBoolItem(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING, bVal);
}

inline SvxColorItem makeSvx3DLightcolor1Item(const Color& rCol) {
    return SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_1);
}

inline SvxColorItem makeSvx3DLightcolor2Item(const Color& rCol) {
    return SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_2);
}

inline SvxColorItem makeSvx3DLightcolor3Item(const Color& rCol) {
    return SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_3);
}

inline SvxColorItem makeSvx3DLightcolor4Item(const Color& rCol) {
    return SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_4);
}

inline SvxColorItem makeSvx3DLightcolor5Item(const Color& rCol) {
    return SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_5);
}

inline SvxColorItem makeSvx3DLightcolor6Item(const Color& rCol) {
    return SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_6);
}

inline SvxColorItem makeSvx3DLightcolor7Item(const Color& rCol) {
    return SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_7);
}

inline SvxColorItem makeSvx3DLightcolor8Item(const Color& rCol) {
    return SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_8);
}

inline SvxColorItem makeSvx3DAmbientcolorItem(const Color& rCol) {
    return SvxColorItem(rCol, SDRATTR_3DSCENE_AMBIENTCOLOR);
}

inline SfxBoolItem makeSvx3DLightOnOff1Item(bool bVal) {
    return SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_1, bVal);
}

inline SfxBoolItem makeSvx3DLightOnOff2Item(bool bVal) {
    return SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_2, bVal);
}

inline SfxBoolItem makeSvx3DLightOnOff3Item(bool bVal) {
    return SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_3, bVal);
}

inline SfxBoolItem makeSvx3DLightOnOff4Item(bool bVal) {
    return SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_4, bVal);
}

inline SfxBoolItem makeSvx3DLightOnOff5Item(bool bVal) {
    return SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_5, bVal);
}

inline SfxBoolItem makeSvx3DLightOnOff6Item(bool bVal) {
    return SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_6, bVal);
}

inline SfxBoolItem makeSvx3DLightOnOff7Item(bool bVal) {
    return SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_7, bVal);
}

inline SfxBoolItem makeSvx3DLightOnOff8Item(bool bVal) {
    return SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_8, bVal);
}

inline SvxB3DVectorItem makeSvx3DLightDirection1Item(const basegfx::B3DVector& rVec) {
    return SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_1, rVec);
}

inline SvxB3DVectorItem makeSvx3DLightDirection2Item(const basegfx::B3DVector& rVec) {
    return SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_2, rVec);
}

inline SvxB3DVectorItem makeSvx3DLightDirection3Item(const basegfx::B3DVector& rVec) {
    return SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_3, rVec);
}

inline SvxB3DVectorItem makeSvx3DLightDirection4Item(const basegfx::B3DVector& rVec) {
    return SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_4, rVec);
}

inline SvxB3DVectorItem makeSvx3DLightDirection5Item(const basegfx::B3DVector& rVec) {
    return SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_5, rVec);
}

inline SvxB3DVectorItem makeSvx3DLightDirection6Item(const basegfx::B3DVector& rVec) {
    return SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_6, rVec);
}

inline SvxB3DVectorItem makeSvx3DLightDirection7Item(const basegfx::B3DVector& rVec) {
    return SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_7, rVec);
}

inline SvxB3DVectorItem makeSvx3DLightDirection8Item(const basegfx::B3DVector& rVec) {
    return SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_8, rVec);
}

inline SfxUInt16Item makeSvx3DShadowSlantItem(sal_uInt16 nVal) {
    return SfxUInt16Item(SDRATTR_3DSCENE_SHADOW_SLANT, nVal);
}

class SVX_DLLPUBLIC Svx3DShadeModeItem : public SfxUInt16Item {
public:
    Svx3DShadeModeItem(sal_uInt16 nVal = 2);

    // use drawing::ShadeMode
    SVX_DLLPRIVATE virtual  bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    SVX_DLLPRIVATE virtual  bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
};


// #107245# Item to replace bExtrudeSmoothed and bLatheSmoothed
class Svx3DSmoothNormalsItem : public SfxBoolItem {
public:
    Svx3DSmoothNormalsItem(bool bVal = true);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const override;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const override;
    SfxPoolItem * Clone(SfxItemPool * = nullptr) const override;
};

// #107245# Item to replace bExtrudeSmoothFrontBack and bLatheSmoothFrontBack
class Svx3DSmoothLidsItem : public SfxBoolItem {
public:
    Svx3DSmoothLidsItem(bool bVal = false);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const override;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const override;
    SfxPoolItem * Clone(SfxItemPool * = nullptr) const override;
};

// #107245# Item to replace bExtrudeCharacterMode and bLatheCharacterMode
class Svx3DCharacterModeItem : public SfxBoolItem {
public:
    Svx3DCharacterModeItem(bool bVal = false);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const override;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const override;
    SfxPoolItem * Clone(SfxItemPool * = nullptr) const override;
};

// #107245# Item to replace bExtrudeCloseFront and bLatheCloseFront
class SVX_DLLPUBLIC Svx3DCloseFrontItem : public SfxBoolItem {
public:
    Svx3DCloseFrontItem(bool bVal = true);
    SVX_DLLPRIVATE virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const override;
    SVX_DLLPRIVATE virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const override;
    SfxPoolItem * Clone(SfxItemPool * = nullptr) const override;
};

// #107245# Item to replace bExtrudeCloseBack and bLatheCloseBack
class SVX_DLLPUBLIC Svx3DCloseBackItem : public SfxBoolItem {
public:
    Svx3DCloseBackItem(bool bVal = true);
    SVX_DLLPRIVATE virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const override;
    SVX_DLLPRIVATE virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const override;
    SfxPoolItem * Clone(SfxItemPool * = nullptr) const override;
};

#endif // INCLUDED_SVX_SVX3DITEMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
