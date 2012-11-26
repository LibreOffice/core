/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

SfxPoolItem* Svx3DReducedLineGeometryItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DReducedLineGeometryItem(*this);
}

//////////////////////////////////////////////////////////////////////////////

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

SfxPoolItem* Svx3DSmoothNormalsItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DSmoothNormalsItem(*this);
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

SfxPoolItem* Svx3DSmoothLidsItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DSmoothLidsItem(*this);
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

SfxPoolItem* Svx3DCharacterModeItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DCharacterModeItem(*this);
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

SfxPoolItem* Svx3DCloseFrontItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DCloseFrontItem(*this);
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

SfxPoolItem* Svx3DCloseBackItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DCloseBackItem(*this);
}

//////////////////////////////////////////////////////////////////////////////

Svx3DNormalsKindItem::Svx3DNormalsKindItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_NORMALS_KIND, nVal)
{}

// Svx3DNormalsKindItem: use drawing::NormalsKind
sal_Bool Svx3DNormalsKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::NormalsKind)GetValue();
    return sal_True;
}

sal_Bool Svx3DNormalsKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::NormalsKind eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

SfxPoolItem* Svx3DNormalsKindItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DNormalsKindItem(*this);
}

//////////////////////////////////////////////////////////////////////////////

Svx3DTextureProjectionXItem::Svx3DTextureProjectionXItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_PROJ_X, nVal)
{}

// Svx3DTextureProjectionXItem: use drawing::TextureProjectionMode
sal_Bool Svx3DTextureProjectionXItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextureProjectionMode)GetValue();
    return sal_True;
}

sal_Bool Svx3DTextureProjectionXItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureProjectionMode eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

SfxPoolItem* Svx3DTextureProjectionXItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DTextureProjectionXItem(*this);
}

//////////////////////////////////////////////////////////////////////////////

Svx3DTextureProjectionYItem::Svx3DTextureProjectionYItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_PROJ_Y, nVal)
{}

// Svx3DTextureProjectionYItem: use drawing::TextureProjectionMode
sal_Bool Svx3DTextureProjectionYItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextureProjectionMode)GetValue();
    return sal_True;
}

sal_Bool Svx3DTextureProjectionYItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureProjectionMode eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

SfxPoolItem* Svx3DTextureProjectionYItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DTextureProjectionYItem(*this);
}

//////////////////////////////////////////////////////////////////////////////

Svx3DTextureKindItem::Svx3DTextureKindItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_KIND, nVal)
{}

// Svx3DTextureKindItem: use drawing::TextureKind
sal_Bool Svx3DTextureKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextureKind)GetValue();
    return sal_True;
}

sal_Bool Svx3DTextureKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureKind eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

SfxPoolItem* Svx3DTextureKindItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DTextureKindItem(*this);
}

//////////////////////////////////////////////////////////////////////////////

Svx3DTextureModeItem::Svx3DTextureModeItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_MODE, nVal)
{}

// Svx3DTextureModeItem: use drawing:TextureMode
sal_Bool Svx3DTextureModeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextureMode)GetValue();
    return sal_True;
}

sal_Bool Svx3DTextureModeItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextureMode eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

SfxPoolItem* Svx3DTextureModeItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DTextureModeItem(*this);
}

//////////////////////////////////////////////////////////////////////////////

Svx3DPerspectiveItem::Svx3DPerspectiveItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DSCENE_PERSPECTIVE, nVal)
{}

// Svx3DPerspectiveItem: use drawing::ProjectionMode
sal_Bool Svx3DPerspectiveItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::ProjectionMode)GetValue();
    return sal_True;
}

sal_Bool Svx3DPerspectiveItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::ProjectionMode eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

SfxPoolItem* Svx3DPerspectiveItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DPerspectiveItem(*this);
}

//////////////////////////////////////////////////////////////////////////////

Svx3DShadeModeItem::Svx3DShadeModeItem(sal_uInt16 nVal)
:   SfxUInt16Item(SDRATTR_3DSCENE_SHADE_MODE, nVal)
{}

// Svx3DShadeModeItem: use drawing::ShadeMode
sal_Bool Svx3DShadeModeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::ShadeMode)GetValue();
    return sal_True;
}

sal_Bool Svx3DShadeModeItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::ShadeMode eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

SfxPoolItem* Svx3DShadeModeItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new Svx3DShadeModeItem(*this);
}

//////////////////////////////////////////////////////////////////////////////
// EOF
