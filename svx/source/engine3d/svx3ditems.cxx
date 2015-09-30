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

#include <svx/svx3ditems.hxx>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/TextureKind.hpp>
#include <com/sun/star/drawing/TextureMode.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>




using namespace ::com::sun::star;

// #i28528#
// Added extra Item (Bool) for chart2 to be able to show reduced line geometry

Svx3DReducedLineGeometryItem::Svx3DReducedLineGeometryItem(bool bVal)
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

SfxPoolItem * Svx3DReducedLineGeometryItem::Clone(SfxItemPool *) const
{
    return new Svx3DReducedLineGeometryItem(*this);
}

Svx3DNormalsKindItem::Svx3DNormalsKindItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_NORMALS_KIND, nVal)
{}

Svx3DTextureProjectionXItem::Svx3DTextureProjectionXItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_PROJ_X, nVal)
{}

Svx3DTextureProjectionYItem::Svx3DTextureProjectionYItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_PROJ_Y, nVal)
{}

Svx3DTextureKindItem::Svx3DTextureKindItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_KIND, nVal)
{}

Svx3DTextureModeItem::Svx3DTextureModeItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_MODE, nVal)
{}

Svx3DPerspectiveItem::Svx3DPerspectiveItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DSCENE_PERSPECTIVE, nVal)
{}

Svx3DShadeModeItem::Svx3DShadeModeItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DSCENE_SHADE_MODE, nVal)
{}



Svx3DSmoothNormalsItem::Svx3DSmoothNormalsItem(bool bVal)
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

SfxPoolItem * Svx3DSmoothNormalsItem::Clone(SfxItemPool *) const
{
    return new Svx3DSmoothNormalsItem(*this);
}


Svx3DSmoothLidsItem::Svx3DSmoothLidsItem(bool bVal)
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

SfxPoolItem * Svx3DSmoothLidsItem::Clone(SfxItemPool *) const
{
    return new Svx3DSmoothLidsItem(*this);
}


Svx3DCharacterModeItem::Svx3DCharacterModeItem(bool bVal)
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

SfxPoolItem * Svx3DCharacterModeItem::Clone(SfxItemPool *) const
{
    return new Svx3DCharacterModeItem(*this);
}


Svx3DCloseFrontItem::Svx3DCloseFrontItem(bool bVal)
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

SfxPoolItem * Svx3DCloseFrontItem::Clone(SfxItemPool *) const
{
    return new Svx3DCloseFrontItem(*this);
}


Svx3DCloseBackItem::Svx3DCloseBackItem(bool bVal)
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

SfxPoolItem * Svx3DCloseBackItem::Clone(SfxItemPool *) const
{
    return new Svx3DCloseBackItem(*this);
}

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
