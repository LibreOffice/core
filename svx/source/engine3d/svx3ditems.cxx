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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/svx3ditems.hxx>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/TextureKind.hpp>
#include <com/sun/star/drawing/TextureMode.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>


//////////////////////////////////////////////////////////////////////////////

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
Svx3DPercentDiagonalItem::Svx3DPercentDiagonalItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_PERCENT_DIAGONAL, nVal)
{}

Svx3DBackscaleItem::Svx3DBackscaleItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_BACKSCALE, nVal)
{}

Svx3DDepthItem::Svx3DDepthItem(sal_uInt32 nVal)
:   SfxUInt32Item(SDRATTR_3DOBJ_DEPTH, nVal)
{}

Svx3DHorizontalSegmentsItem::Svx3DHorizontalSegmentsItem(sal_uInt32 nVal)
:   SfxUInt32Item(SDRATTR_3DOBJ_HORZ_SEGS, nVal)
{}

Svx3DVerticalSegmentsItem::Svx3DVerticalSegmentsItem(sal_uInt32 nVal)
:   SfxUInt32Item(SDRATTR_3DOBJ_VERT_SEGS, nVal)
{}

Svx3DEndAngleItem::Svx3DEndAngleItem(sal_uInt32 nVal)
:   SfxUInt32Item(SDRATTR_3DOBJ_END_ANGLE, nVal)
{}

Svx3DDoubleSidedItem::Svx3DDoubleSidedItem(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, bVal)
{}

//////////////////////////////////////////////////////////////////////////////
// #i28528#
// Added extra Item (Bool) for chart2 to be able to show reduced line geometry

Svx3DReducedLineGeometryItem::Svx3DReducedLineGeometryItem(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY, bVal)
{}

sal_uInt16 Svx3DReducedLineGeometryItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

SfxPoolItem* Svx3DReducedLineGeometryItem::Create(SvStream& rIn, sal_uInt16 nItemVersion) const
{
    SfxBoolItem* pRetval = new Svx3DReducedLineGeometryItem();

    if(nItemVersion > 0)
    {
        SfxBoolItem aBoolItem(Which(), rIn);
        pRetval->SetValue(aBoolItem.GetValue());
    }

    return pRetval;
}

//////////////////////////////////////////////////////////////////////////////

Svx3DNormalsKindItem::Svx3DNormalsKindItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_NORMALS_KIND, nVal)
{}

Svx3DNormalsInvertItem::Svx3DNormalsInvertItem(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_NORMALS_INVERT, bVal)
{}

Svx3DTextureProjectionXItem::Svx3DTextureProjectionXItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_PROJ_X, nVal)
{}

Svx3DTextureProjectionYItem::Svx3DTextureProjectionYItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_PROJ_Y, nVal)
{}

Svx3DShadow3DItem::Svx3DShadow3DItem(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_SHADOW_3D, bVal)
{}

Svx3DMaterialColorItem::Svx3DMaterialColorItem(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DOBJ_MAT_COLOR)
{}

Svx3DMaterialEmissionItem::Svx3DMaterialEmissionItem(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DOBJ_MAT_EMISSION)
{}

Svx3DMaterialSpecularItem::Svx3DMaterialSpecularItem(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DOBJ_MAT_SPECULAR)
{}

Svx3DMaterialSpecularIntensityItem::Svx3DMaterialSpecularIntensityItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY, nVal)
{}

Svx3DTextureKindItem::Svx3DTextureKindItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_KIND, nVal)
{}

Svx3DTextureModeItem::Svx3DTextureModeItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_MODE, nVal)
{}

Svx3DTextureFilterItem::Svx3DTextureFilterItem(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_TEXTURE_FILTER, bVal)
{}

Svx3DPerspectiveItem::Svx3DPerspectiveItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DSCENE_PERSPECTIVE, nVal)
{}

Svx3DDistanceItem::Svx3DDistanceItem(sal_uInt32 nVal)
:   SfxUInt32Item(SDRATTR_3DSCENE_DISTANCE, nVal)
{}

Svx3DFocalLengthItem::Svx3DFocalLengthItem(sal_uInt32 nVal)
:   SfxUInt32Item(SDRATTR_3DSCENE_FOCAL_LENGTH, nVal)
{}

Svx3DTwoSidedLightingItem::Svx3DTwoSidedLightingItem(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING, bVal)
{}

Svx3DLightcolor1Item::Svx3DLightcolor1Item(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_1)
{}

Svx3DLightcolor2Item::Svx3DLightcolor2Item(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_2)
{}

Svx3DLightcolor3Item::Svx3DLightcolor3Item(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_3)
{}

Svx3DLightcolor4Item::Svx3DLightcolor4Item(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_4)
{}

Svx3DLightcolor5Item::Svx3DLightcolor5Item(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_5)
{}

Svx3DLightcolor6Item::Svx3DLightcolor6Item(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_6)
{}

Svx3DLightcolor7Item::Svx3DLightcolor7Item(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_7)
{}

Svx3DLightcolor8Item::Svx3DLightcolor8Item(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_8)
{}

Svx3DAmbientcolorItem::Svx3DAmbientcolorItem(const Color& rCol)
:   SvxColorItem(rCol, SDRATTR_3DSCENE_AMBIENTCOLOR)
{}

Svx3DLightOnOff1Item::Svx3DLightOnOff1Item(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_1, bVal)
{}

Svx3DLightOnOff2Item::Svx3DLightOnOff2Item(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_2, bVal)
{}

Svx3DLightOnOff3Item::Svx3DLightOnOff3Item(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_3, bVal)
{}

Svx3DLightOnOff4Item::Svx3DLightOnOff4Item(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_4, bVal)
{}

Svx3DLightOnOff5Item::Svx3DLightOnOff5Item(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_5, bVal)
{}

Svx3DLightOnOff6Item::Svx3DLightOnOff6Item(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_6, bVal)
{}

Svx3DLightOnOff7Item::Svx3DLightOnOff7Item(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_7, bVal)
{}

Svx3DLightOnOff8Item::Svx3DLightOnOff8Item(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_8, bVal)
{}

Svx3DLightDirection1Item::Svx3DLightDirection1Item(const basegfx::B3DVector& rVec)
:   SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_1, rVec)
{}

Svx3DLightDirection2Item::Svx3DLightDirection2Item(const basegfx::B3DVector& rVec)
:   SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_2, rVec)
{}

Svx3DLightDirection3Item::Svx3DLightDirection3Item(const basegfx::B3DVector& rVec)
:   SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_3, rVec)
{}

Svx3DLightDirection4Item::Svx3DLightDirection4Item(const basegfx::B3DVector& rVec)
:   SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_4, rVec)
{}

Svx3DLightDirection5Item::Svx3DLightDirection5Item(const basegfx::B3DVector& rVec)
:   SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_5, rVec)
{}

Svx3DLightDirection6Item::Svx3DLightDirection6Item(const basegfx::B3DVector& rVec)
:   SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_6, rVec)
{}

Svx3DLightDirection7Item::Svx3DLightDirection7Item(const basegfx::B3DVector& rVec)
:   SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_7, rVec)
{}

Svx3DLightDirection8Item::Svx3DLightDirection8Item(const basegfx::B3DVector& rVec)
:   SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_8, rVec)
{}

Svx3DShadowSlantItem::Svx3DShadowSlantItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DSCENE_SHADOW_SLANT, nVal)
{}

Svx3DShadeModeItem::Svx3DShadeModeItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DSCENE_SHADE_MODE, nVal)
{}

//////////////////////////////////////////////////////////////////////////////
// #107245#

Svx3DSmoothNormalsItem::Svx3DSmoothNormalsItem(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_SMOOTH_NORMALS, bVal)
{}

sal_uInt16 Svx3DSmoothNormalsItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

SfxPoolItem* Svx3DSmoothNormalsItem::Create(SvStream& rIn, sal_uInt16 nItemVersion) const
{
    SfxBoolItem* pRetval = new Svx3DSmoothNormalsItem();

    if(nItemVersion > 0)
    {
        SfxBoolItem aBoolItem(Which(), rIn);
        pRetval->SetValue(aBoolItem.GetValue());
    }

    return pRetval;
}

//////////////////////////////////////////////////////////////////////////////
// #107245#

Svx3DSmoothLidsItem::Svx3DSmoothLidsItem(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_SMOOTH_LIDS, bVal)
{}

sal_uInt16 Svx3DSmoothLidsItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

SfxPoolItem* Svx3DSmoothLidsItem::Create(SvStream& rIn, sal_uInt16 nItemVersion) const
{
    SfxBoolItem* pRetval = new Svx3DSmoothLidsItem();

    if(nItemVersion > 0)
    {
        SfxBoolItem aBoolItem(Which(), rIn);
        pRetval->SetValue(aBoolItem.GetValue());
    }

    return pRetval;
}

//////////////////////////////////////////////////////////////////////////////
// #107245#

Svx3DCharacterModeItem::Svx3DCharacterModeItem(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_CHARACTER_MODE, bVal)
{}

sal_uInt16 Svx3DCharacterModeItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

SfxPoolItem* Svx3DCharacterModeItem::Create(SvStream& rIn, sal_uInt16 nItemVersion) const
{
    SfxBoolItem* pRetval = new Svx3DCharacterModeItem();

    if(nItemVersion > 0)
    {
        SfxBoolItem aBoolItem(Which(), rIn);
        pRetval->SetValue(aBoolItem.GetValue());
    }

    return pRetval;
}

//////////////////////////////////////////////////////////////////////////////
// #107245#

Svx3DCloseFrontItem::Svx3DCloseFrontItem(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_CLOSE_FRONT, bVal)
{}

sal_uInt16 Svx3DCloseFrontItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

SfxPoolItem* Svx3DCloseFrontItem::Create(SvStream& rIn, sal_uInt16 nItemVersion) const
{
    SfxBoolItem* pRetval = new Svx3DCloseFrontItem();

    if(nItemVersion > 0)
    {
        SfxBoolItem aBoolItem(Which(), rIn);
        pRetval->SetValue(aBoolItem.GetValue());
    }

    return pRetval;
}

//////////////////////////////////////////////////////////////////////////////
// #107245#

Svx3DCloseBackItem::Svx3DCloseBackItem(sal_Bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_CLOSE_BACK, bVal)
{}

sal_uInt16 Svx3DCloseBackItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

SfxPoolItem* Svx3DCloseBackItem::Create(SvStream& rIn, sal_uInt16 nItemVersion) const
{
    SfxBoolItem* pRetval = new Svx3DCloseBackItem();

    if(nItemVersion > 0)
    {
        SfxBoolItem aBoolItem(Which(), rIn);
        pRetval->SetValue(aBoolItem.GetValue());
    }

    return pRetval;
}

//////////////////////////////////////////////////////////////////////////////

// Svx3DNormalsKindItem: use drawing::NormalsKind
bool Svx3DNormalsKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::NormalsKind)GetValue();
    return true;
}

bool Svx3DNormalsKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::NormalsKind eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue((sal_Int16)eVar);
    return true;
}

SfxPoolItem* Svx3DNormalsKindItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DNormalsKindItem(*this);
}

// Svx3DTextureProjectionXItem: use drawing::TextureProjectionMode
bool Svx3DTextureProjectionXItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextureProjectionMode)GetValue();
    return true;
}

bool Svx3DTextureProjectionXItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureProjectionMode eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue((sal_Int16)eVar);
    return true;
}

SfxPoolItem* Svx3DTextureProjectionXItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DTextureProjectionXItem(*this);
}

// Svx3DTextureProjectionYItem: use drawing::TextureProjectionMode
bool Svx3DTextureProjectionYItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextureProjectionMode)GetValue();
    return true;
}

bool Svx3DTextureProjectionYItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureProjectionMode eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue((sal_Int16)eVar);
    return true;
}

SfxPoolItem* Svx3DTextureProjectionYItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DTextureProjectionYItem(*this);
}

// Svx3DTextureKindItem: use drawing::TextureKind
bool Svx3DTextureKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextureKind)GetValue();
    return true;
}

bool Svx3DTextureKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureKind eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue((sal_Int16)eVar);
    return true;
}

SfxPoolItem* Svx3DTextureKindItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DTextureKindItem(*this);
}

// Svx3DTextureModeItem: use drawing:TextureMode
bool Svx3DTextureModeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextureMode)GetValue();
    return true;
}

bool Svx3DTextureModeItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureMode eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue((sal_Int16)eVar);
    return true;
}

SfxPoolItem* Svx3DTextureModeItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DTextureModeItem(*this);
}

// Svx3DPerspectiveItem: use drawing::ProjectionMode
bool Svx3DPerspectiveItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::ProjectionMode)GetValue();
    return true;
}

bool Svx3DPerspectiveItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::ProjectionMode eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue((sal_Int16)eVar);
    return true;
}

SfxPoolItem* Svx3DPerspectiveItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DPerspectiveItem(*this);
}

// Svx3DShadeModeItem: use drawing::ShadeMode
bool Svx3DShadeModeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::ShadeMode)GetValue();
    return true;
}

bool Svx3DShadeModeItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::ShadeMode eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue((sal_Int16)eVar);
    return true;
}

SfxPoolItem* Svx3DShadeModeItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DShadeModeItem(*this);
}

// EOF

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
