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

std::unique_ptr<SfxPoolItem> Svx3DReducedLineGeometryItem::CloneInternal(SfxItemPool *) const
{
    return std::unique_ptr<SfxPoolItem>(new Svx3DReducedLineGeometryItem(*this));
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

Svx3DPerspectiveItem::Svx3DPerspectiveItem(ProjectionType nVal)
:   SfxUInt16Item(SDRATTR_3DSCENE_PERSPECTIVE, static_cast<sal_uInt16>(nVal))
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

std::unique_ptr<SfxPoolItem> Svx3DSmoothNormalsItem::CloneInternal(SfxItemPool *) const
{
    return std::unique_ptr<SfxPoolItem>(new Svx3DSmoothNormalsItem(*this));
}


Svx3DSmoothLidsItem::Svx3DSmoothLidsItem(bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_SMOOTH_LIDS, bVal)
{}

sal_uInt16 Svx3DSmoothLidsItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

std::unique_ptr<SfxPoolItem> Svx3DSmoothLidsItem::CloneInternal(SfxItemPool *) const
{
    return std::unique_ptr<SfxPoolItem>(new Svx3DSmoothLidsItem(*this));
}


Svx3DCharacterModeItem::Svx3DCharacterModeItem(bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_CHARACTER_MODE, bVal)
{}

sal_uInt16 Svx3DCharacterModeItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

std::unique_ptr<SfxPoolItem> Svx3DCharacterModeItem::CloneInternal(SfxItemPool *) const
{
    return std::unique_ptr<SfxPoolItem>(new Svx3DCharacterModeItem(*this));
}


Svx3DCloseFrontItem::Svx3DCloseFrontItem(bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_CLOSE_FRONT, bVal)
{}

sal_uInt16 Svx3DCloseFrontItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

std::unique_ptr<SfxPoolItem> Svx3DCloseFrontItem::CloneInternal(SfxItemPool *) const
{
    return std::unique_ptr<SfxPoolItem>(new Svx3DCloseFrontItem(*this));
}


Svx3DCloseBackItem::Svx3DCloseBackItem(bool bVal)
:   SfxBoolItem(SDRATTR_3DOBJ_CLOSE_BACK, bVal)
{}

sal_uInt16 Svx3DCloseBackItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

std::unique_ptr<SfxPoolItem> Svx3DCloseBackItem::CloneInternal(SfxItemPool *) const
{
    return std::unique_ptr<SfxPoolItem>(new Svx3DCloseBackItem(*this));
}

// Svx3DNormalsKindItem: use drawing::NormalsKind
bool Svx3DNormalsKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::NormalsKind>(GetValue());
    return true;
}

bool Svx3DNormalsKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::NormalsKind eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue(static_cast<sal_Int16>(eVar));
    return true;
}

std::unique_ptr<SfxPoolItem> Svx3DNormalsKindItem::CloneInternal(SfxItemPool* /*pPool*/) const
{
    return o3tl::make_unique<Svx3DNormalsKindItem>(*this);
}

// Svx3DTextureProjectionXItem: use drawing::TextureProjectionMode
bool Svx3DTextureProjectionXItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextureProjectionMode>(GetValue());
    return true;
}

bool Svx3DTextureProjectionXItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureProjectionMode eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue(static_cast<sal_Int16>(eVar));
    return true;
}

std::unique_ptr<SfxPoolItem> Svx3DTextureProjectionXItem::CloneInternal(SfxItemPool* /*pPool*/) const
{
    return o3tl::make_unique<Svx3DTextureProjectionXItem>(*this);
}

// Svx3DTextureProjectionYItem: use drawing::TextureProjectionMode
bool Svx3DTextureProjectionYItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextureProjectionMode>(GetValue());
    return true;
}

bool Svx3DTextureProjectionYItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureProjectionMode eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue(static_cast<sal_Int16>(eVar));
    return true;
}

std::unique_ptr<SfxPoolItem> Svx3DTextureProjectionYItem::CloneInternal(SfxItemPool* /*pPool*/) const
{
    return o3tl::make_unique<Svx3DTextureProjectionYItem>(*this);
}

// Svx3DTextureKindItem: use drawing::TextureKind
bool Svx3DTextureKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextureKind>(GetValue());
    return true;
}

bool Svx3DTextureKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureKind eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue(static_cast<sal_Int16>(eVar));
    return true;
}

std::unique_ptr<SfxPoolItem> Svx3DTextureKindItem::CloneInternal(SfxItemPool* /*pPool*/) const
{
    return o3tl::make_unique<Svx3DTextureKindItem>(*this);
}

// Svx3DTextureModeItem: use drawing:TextureMode
bool Svx3DTextureModeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextureMode>(GetValue());
    return true;
}

bool Svx3DTextureModeItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureMode eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue(static_cast<sal_Int16>(eVar));
    return true;
}

std::unique_ptr<SfxPoolItem> Svx3DTextureModeItem::CloneInternal(SfxItemPool* /*pPool*/) const
{
    return o3tl::make_unique<Svx3DTextureModeItem>(*this);
}

// Svx3DPerspectiveItem: use drawing::ProjectionMode
bool Svx3DPerspectiveItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::ProjectionMode>(GetValue());
    return true;
}

bool Svx3DPerspectiveItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::ProjectionMode eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue(static_cast<sal_Int16>(eVar));
    return true;
}

std::unique_ptr<SfxPoolItem> Svx3DPerspectiveItem::CloneInternal(SfxItemPool* /*pPool*/) const
{
    return o3tl::make_unique<Svx3DPerspectiveItem>(*this);
}

// Svx3DShadeModeItem: use drawing::ShadeMode
bool Svx3DShadeModeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::ShadeMode>(GetValue());
    return true;
}

bool Svx3DShadeModeItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::ShadeMode eVar;
    if(!(rVal >>= eVar))
        return false;
    SetValue(static_cast<sal_Int16>(eVar));
    return true;
}

std::unique_ptr<SfxPoolItem> Svx3DShadeModeItem::CloneInternal(SfxItemPool* /*pPool*/) const
{
    return o3tl::make_unique<Svx3DShadeModeItem>(*this);
}

// EOF

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
