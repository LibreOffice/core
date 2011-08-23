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

#ifndef _SVX3DITEMS_HXX
#include "svx3ditems.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_NORMALSKIND_HPP_
#include <com/sun/star/drawing/NormalsKind.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREPROJECTIONMODE_HPP_
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREKIND_HPP_
#include <com/sun/star/drawing/TextureKind.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREMODE_HPP_
#include <com/sun/star/drawing/TextureMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif

#ifndef _XDEF_HXX //autogen
#include "xdef.hxx"
#endif

namespace binfilter {


//////////////////////////////////////////////////////////////////////////////

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
/*N*/ Svx3DPercentDiagonalItem::Svx3DPercentDiagonalItem(sal_uInt16 nVal) 
/*N*/ :	SfxUInt16Item(SDRATTR_3DOBJ_PERCENT_DIAGONAL, nVal) 
/*N*/ {}

/*N*/ Svx3DBackscaleItem::Svx3DBackscaleItem(sal_uInt16 nVal) 
/*N*/ :	SfxUInt16Item(SDRATTR_3DOBJ_BACKSCALE, nVal) 
/*N*/ {}

/*N*/ Svx3DDepthItem::Svx3DDepthItem(sal_uInt32 nVal) 
/*N*/ :	SfxUInt32Item(SDRATTR_3DOBJ_DEPTH, nVal) 
/*N*/ {}

/*N*/ Svx3DHorizontalSegmentsItem::Svx3DHorizontalSegmentsItem(sal_uInt32 nVal) 
/*N*/ :	SfxUInt32Item(SDRATTR_3DOBJ_HORZ_SEGS, nVal) 
/*N*/ {}

/*N*/ Svx3DVerticalSegmentsItem::Svx3DVerticalSegmentsItem(sal_uInt32 nVal) 
/*N*/ :	SfxUInt32Item(SDRATTR_3DOBJ_VERT_SEGS, nVal) 
/*N*/ {}

/*N*/ Svx3DEndAngleItem::Svx3DEndAngleItem(sal_uInt32 nVal) 
/*N*/ :	SfxUInt32Item(SDRATTR_3DOBJ_END_ANGLE, nVal) 
/*N*/ {}

/*N*/ Svx3DDoubleSidedItem::Svx3DDoubleSidedItem(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, bVal) 
/*N*/ {}

/*N*/ Svx3DNormalsKindItem::Svx3DNormalsKindItem(sal_uInt16 nVal) 
/*N*/ :	SfxUInt16Item(SDRATTR_3DOBJ_NORMALS_KIND, nVal) 
/*N*/ {}

/*N*/ Svx3DNormalsInvertItem::Svx3DNormalsInvertItem(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DOBJ_NORMALS_INVERT, bVal) 
/*N*/ {}

/*N*/ Svx3DTextureProjectionXItem::Svx3DTextureProjectionXItem(sal_uInt16 nVal) 
/*N*/ :	SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_PROJ_X, nVal) 
/*N*/ {}

/*N*/ Svx3DTextureProjectionYItem::Svx3DTextureProjectionYItem(sal_uInt16 nVal) 
/*N*/ :	SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_PROJ_Y, nVal) 
/*N*/ {}

/*N*/ Svx3DShadow3DItem::Svx3DShadow3DItem(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DOBJ_SHADOW_3D, bVal) 
/*N*/ {}

/*N*/ Svx3DMaterialColorItem::Svx3DMaterialColorItem(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DOBJ_MAT_COLOR) 
/*N*/ {}

/*N*/ Svx3DMaterialEmissionItem::Svx3DMaterialEmissionItem(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DOBJ_MAT_EMISSION) 
/*N*/ {}

/*N*/ Svx3DMaterialSpecularItem::Svx3DMaterialSpecularItem(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DOBJ_MAT_SPECULAR) 
/*N*/ {}

/*N*/ Svx3DMaterialSpecularIntensityItem::Svx3DMaterialSpecularIntensityItem(sal_uInt16 nVal) 
/*N*/ :	SfxUInt16Item(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY, nVal) 
/*N*/ {}

/*N*/ Svx3DTextureKindItem::Svx3DTextureKindItem(sal_uInt16 nVal) 
/*N*/ :	SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_KIND, nVal) 
/*N*/ {}

/*N*/ Svx3DTextureModeItem::Svx3DTextureModeItem(sal_uInt16 nVal) 
/*N*/ :	SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_MODE, nVal) 
/*N*/ {}

/*N*/ Svx3DTextureFilterItem::Svx3DTextureFilterItem(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DOBJ_TEXTURE_FILTER, bVal) 
/*N*/ {}

/*N*/ Svx3DPerspectiveItem::Svx3DPerspectiveItem(sal_uInt16 nVal) 
/*N*/ :	SfxUInt16Item(SDRATTR_3DSCENE_PERSPECTIVE, nVal) 
/*N*/ {}

/*N*/ Svx3DDistanceItem::Svx3DDistanceItem(sal_uInt32 nVal) 
/*N*/ :	SfxUInt32Item(SDRATTR_3DSCENE_DISTANCE, nVal) 
/*N*/ {}

/*N*/ Svx3DFocalLengthItem::Svx3DFocalLengthItem(sal_uInt32 nVal) 
/*N*/ :	SfxUInt32Item(SDRATTR_3DSCENE_FOCAL_LENGTH, nVal) 
/*N*/ {}

/*N*/ Svx3DTwoSidedLightingItem::Svx3DTwoSidedLightingItem(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING, bVal) 
/*N*/ {}

/*N*/ Svx3DLightcolor1Item::Svx3DLightcolor1Item(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_1) 
/*N*/ {}

/*N*/ Svx3DLightcolor2Item::Svx3DLightcolor2Item(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_2) 
/*N*/ {}

/*N*/ Svx3DLightcolor3Item::Svx3DLightcolor3Item(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_3) 
/*N*/ {}

/*N*/ Svx3DLightcolor4Item::Svx3DLightcolor4Item(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_4) 
/*N*/ {}

/*N*/ Svx3DLightcolor5Item::Svx3DLightcolor5Item(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_5) 
/*N*/ {}

/*N*/ Svx3DLightcolor6Item::Svx3DLightcolor6Item(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_6) 
/*N*/ {}

/*N*/ Svx3DLightcolor7Item::Svx3DLightcolor7Item(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_7) 
/*N*/ {}

/*N*/ Svx3DLightcolor8Item::Svx3DLightcolor8Item(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DSCENE_LIGHTCOLOR_8) 
/*N*/ {}

/*N*/ Svx3DAmbientcolorItem::Svx3DAmbientcolorItem(const Color& rCol) 
/*N*/ :	SvxColorItem(rCol, SDRATTR_3DSCENE_AMBIENTCOLOR) 
/*N*/ {}

/*N*/ Svx3DLightOnOff1Item::Svx3DLightOnOff1Item(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_1, bVal) 
/*N*/ {}

/*N*/ Svx3DLightOnOff2Item::Svx3DLightOnOff2Item(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_2, bVal) 
/*N*/ {}

/*N*/ Svx3DLightOnOff3Item::Svx3DLightOnOff3Item(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_3, bVal) 
/*N*/ {}

/*N*/ Svx3DLightOnOff4Item::Svx3DLightOnOff4Item(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_4, bVal) 
/*N*/ {}

/*N*/ Svx3DLightOnOff5Item::Svx3DLightOnOff5Item(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_5, bVal) 
/*N*/ {}

/*N*/ Svx3DLightOnOff6Item::Svx3DLightOnOff6Item(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_6, bVal) 
/*N*/ {}

/*N*/ Svx3DLightOnOff7Item::Svx3DLightOnOff7Item(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_7, bVal) 
/*N*/ {}

/*N*/ Svx3DLightOnOff8Item::Svx3DLightOnOff8Item(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_8, bVal) 
/*N*/ {}
/*N*/ 
/*N*/ Svx3DLightDirection1Item::Svx3DLightDirection1Item(const Vector3D& rVec) 
/*N*/ :	SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_1, rVec) 
/*N*/ {}

/*N*/ Svx3DLightDirection2Item::Svx3DLightDirection2Item(const Vector3D& rVec) 
/*N*/ :	SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_2, rVec) 
/*N*/ {}

/*N*/ Svx3DLightDirection3Item::Svx3DLightDirection3Item(const Vector3D& rVec) 
/*N*/ :	SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_3, rVec) 
/*N*/ {}

/*N*/ Svx3DLightDirection4Item::Svx3DLightDirection4Item(const Vector3D& rVec) 
/*N*/ :	SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_4, rVec) 
/*N*/ {}

/*N*/ Svx3DLightDirection5Item::Svx3DLightDirection5Item(const Vector3D& rVec) 
/*N*/ :	SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_5, rVec) 
/*N*/ {}

/*N*/ Svx3DLightDirection6Item::Svx3DLightDirection6Item(const Vector3D& rVec) 
/*N*/ :	SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_6, rVec) 
/*N*/ {}

/*N*/ Svx3DLightDirection7Item::Svx3DLightDirection7Item(const Vector3D& rVec) 
/*N*/ :	SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_7, rVec) 
/*N*/ {}

/*N*/ Svx3DLightDirection8Item::Svx3DLightDirection8Item(const Vector3D& rVec) 
/*N*/ :	SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_8, rVec) 
/*N*/ {}

/*N*/ Svx3DShadowSlantItem::Svx3DShadowSlantItem(sal_uInt16 nVal) 
/*N*/ :	SfxUInt16Item(SDRATTR_3DSCENE_SHADOW_SLANT, nVal) 
/*N*/ {}

/*N*/ Svx3DShadeModeItem::Svx3DShadeModeItem(sal_uInt16 nVal) 
/*N*/ :	SfxUInt16Item(SDRATTR_3DSCENE_SHADE_MODE, nVal) 
/*N*/ {}

//////////////////////////////////////////////////////////////////////////////
// #107245#

/*N*/ Svx3DSmoothNormalsItem::Svx3DSmoothNormalsItem(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DOBJ_SMOOTH_NORMALS, bVal) 
/*N*/ {}

/*N*/ sal_uInt16 Svx3DSmoothNormalsItem::GetVersion(sal_uInt16 nFileFormatVersion) const
/*N*/ {
/*N*/ 	return 1;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DSmoothNormalsItem::Create(SvStream& rIn, sal_uInt16 nItemVersion) const
/*N*/ {
/*N*/ 	SfxBoolItem* pRetval = new Svx3DSmoothNormalsItem();
/*N*/ 
/*N*/ 	if(nItemVersion > 0)
/*N*/ 	{
/*?*/ 		SfxBoolItem aBoolItem(Which(), rIn);
/*?*/ 		pRetval->SetValue(aBoolItem.GetValue());
/*N*/ 	}
/*N*/ 
/*N*/ 	return pRetval;
/*N*/ }

//////////////////////////////////////////////////////////////////////////////
// #107245#

/*N*/ Svx3DSmoothLidsItem::Svx3DSmoothLidsItem(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DOBJ_SMOOTH_LIDS, bVal) 
/*N*/ {}

/*N*/ sal_uInt16 Svx3DSmoothLidsItem::GetVersion(sal_uInt16 nFileFormatVersion) const
/*N*/ {
/*N*/ 	return 1;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DSmoothLidsItem::Create(SvStream& rIn, sal_uInt16 nItemVersion) const
/*N*/ {
/*N*/ 	SfxBoolItem* pRetval = new Svx3DSmoothLidsItem();
/*N*/ 
/*N*/ 	if(nItemVersion > 0)
/*N*/ 	{
/*?*/ 		SfxBoolItem aBoolItem(Which(), rIn);
/*?*/ 		pRetval->SetValue(aBoolItem.GetValue());
/*N*/ 	}
/*N*/ 
/*N*/ 	return pRetval;
/*N*/ }

//////////////////////////////////////////////////////////////////////////////
// #107245#

/*N*/ Svx3DCharacterModeItem::Svx3DCharacterModeItem(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DOBJ_CHARACTER_MODE, bVal) 
/*N*/ {}

/*N*/ sal_uInt16 Svx3DCharacterModeItem::GetVersion(sal_uInt16 nFileFormatVersion) const
/*N*/ {
/*N*/ 	return 1;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DCharacterModeItem::Create(SvStream& rIn, sal_uInt16 nItemVersion) const
/*N*/ {
/*N*/ 	SfxBoolItem* pRetval = new Svx3DCharacterModeItem();
/*N*/ 
/*N*/ 	if(nItemVersion > 0)
/*N*/ 	{
/*?*/ 		SfxBoolItem aBoolItem(Which(), rIn);
/*?*/ 		pRetval->SetValue(aBoolItem.GetValue());
/*N*/ 	}
/*N*/ 
/*N*/ 	return pRetval;
/*N*/ }

//////////////////////////////////////////////////////////////////////////////
// #107245#

/*N*/ Svx3DCloseFrontItem::Svx3DCloseFrontItem(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DOBJ_CLOSE_FRONT, bVal) 
/*N*/ {}

/*N*/ sal_uInt16 Svx3DCloseFrontItem::GetVersion(sal_uInt16 nFileFormatVersion) const
/*N*/ {
/*N*/ 	return 1;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DCloseFrontItem::Create(SvStream& rIn, sal_uInt16 nItemVersion) const
/*N*/ {
/*N*/ 	SfxBoolItem* pRetval = new Svx3DCloseFrontItem();
/*N*/ 
/*N*/ 	if(nItemVersion > 0)
/*N*/ 	{
/*?*/ 		SfxBoolItem aBoolItem(Which(), rIn);
/*?*/ 		pRetval->SetValue(aBoolItem.GetValue());
/*N*/ 	}
/*N*/ 
/*N*/ 	return pRetval;
/*N*/ }

//////////////////////////////////////////////////////////////////////////////
// #107245#

/*N*/ Svx3DCloseBackItem::Svx3DCloseBackItem(BOOL bVal) 
/*N*/ :	SfxBoolItem(SDRATTR_3DOBJ_CLOSE_BACK, bVal) 
/*N*/ {}

/*N*/ sal_uInt16 Svx3DCloseBackItem::GetVersion(sal_uInt16 nFileFormatVersion) const
/*N*/ {
/*N*/ 	return 1;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DCloseBackItem::Create(SvStream& rIn, sal_uInt16 nItemVersion) const
/*N*/ {
/*N*/ 	SfxBoolItem* pRetval = new Svx3DCloseBackItem();
/*N*/ 
/*N*/ 	if(nItemVersion > 0)
/*N*/ 	{
/*?*/ 		SfxBoolItem aBoolItem(Which(), rIn);
/*?*/ 		pRetval->SetValue(aBoolItem.GetValue());
/*N*/ 	}
/*N*/ 
/*N*/ 	return pRetval;
/*N*/ }

//////////////////////////////////////////////////////////////////////////////

// Svx3DNormalsKindItem: use drawing::NormalsKind
/*N*/ sal_Bool Svx3DNormalsKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
/*N*/ {
/*N*/ 	rVal <<= (drawing::NormalsKind)GetValue();
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool Svx3DNormalsKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
/*N*/ {
/*N*/ 	drawing::NormalsKind eVar;
/*N*/ 	if(!(rVal >>= eVar))
/*N*/ 		return sal_False;
/*N*/ 	SetValue((sal_Int16)eVar);
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DNormalsKindItem::Clone(SfxItemPool* pPool) const 
/*N*/ { 
/*N*/ 	return new Svx3DNormalsKindItem(*this);
/*N*/ }

// Svx3DTextureProjectionXItem: use drawing::TextureProjectionMode
/*N*/ sal_Bool Svx3DTextureProjectionXItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
/*N*/ {
/*N*/ 	rVal <<= (drawing::TextureProjectionMode)GetValue();
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool Svx3DTextureProjectionXItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
/*N*/ {
/*N*/ 	drawing::TextureProjectionMode eVar;
/*N*/ 	if(!(rVal >>= eVar))
/*N*/ 		return sal_False;
/*N*/ 	SetValue((sal_Int16)eVar);
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DTextureProjectionXItem::Clone(SfxItemPool* pPool) const 
/*N*/ { 
/*N*/ 	return new Svx3DTextureProjectionXItem(*this);
/*N*/ }

// Svx3DTextureProjectionYItem: use drawing::TextureProjectionMode
/*N*/ sal_Bool Svx3DTextureProjectionYItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
/*N*/ {
/*N*/ 	rVal <<= (drawing::TextureProjectionMode)GetValue();
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool Svx3DTextureProjectionYItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
/*N*/ {
/*N*/ 	drawing::TextureProjectionMode eVar;
/*N*/ 	if(!(rVal >>= eVar))
/*N*/ 		return sal_False;
/*N*/ 	SetValue((sal_Int16)eVar);
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DTextureProjectionYItem::Clone(SfxItemPool* pPool) const 
/*N*/ { 
/*N*/ 	return new Svx3DTextureProjectionYItem(*this);
/*N*/ }

// Svx3DTextureKindItem: use drawing::TextureKind
/*N*/ sal_Bool Svx3DTextureKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
/*N*/ {
/*N*/ 	rVal <<= (drawing::TextureKind)GetValue();
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool Svx3DTextureKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
/*N*/ {
/*N*/ 	drawing::TextureKind eVar;
/*N*/ 	if(!(rVal >>= eVar))
/*N*/ 		return sal_False;
/*N*/ 	SetValue((sal_Int16)eVar);
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DTextureKindItem::Clone(SfxItemPool* pPool) const 
/*N*/ { 
/*N*/ 	return new Svx3DTextureKindItem(*this);
/*N*/ }

// Svx3DTextureModeItem: use drawing:TextureMode
/*N*/ sal_Bool Svx3DTextureModeItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
/*N*/ {
/*N*/ 	rVal <<= (drawing::TextureMode)GetValue();
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool Svx3DTextureModeItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
/*N*/ {
/*N*/ 	drawing::TextureMode eVar;
/*N*/ 	if(!(rVal >>= eVar))
/*N*/ 		return sal_False;
/*N*/ 	SetValue((sal_Int16)eVar);
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DTextureModeItem::Clone(SfxItemPool* pPool) const 
/*N*/ { 
/*N*/ 	return new Svx3DTextureModeItem(*this);
/*N*/ }

// Svx3DPerspectiveItem: use drawing::ProjectionMode
/*N*/ sal_Bool Svx3DPerspectiveItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
/*N*/ {
/*N*/ 	rVal <<= (drawing::ProjectionMode)GetValue();
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool Svx3DPerspectiveItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
/*N*/ {
/*N*/ 	drawing::ProjectionMode eVar;
/*N*/ 	if(!(rVal >>= eVar))
/*N*/ 		return sal_False;
/*N*/ 	SetValue((sal_Int16)eVar);
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DPerspectiveItem::Clone(SfxItemPool* pPool) const 
/*N*/ { 
/*N*/ 	return new Svx3DPerspectiveItem(*this);
/*N*/ }

// Svx3DShadeModeItem: use drawing::ShadeMode
/*N*/ sal_Bool Svx3DShadeModeItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
/*N*/ {
/*N*/ 	rVal <<= (drawing::ShadeMode)GetValue();
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool Svx3DShadeModeItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
/*N*/ {
/*N*/ 	drawing::ShadeMode eVar;
/*N*/ 	if(!(rVal >>= eVar))
/*N*/ 		return sal_False;
/*N*/ 	SetValue((sal_Int16)eVar);
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ SfxPoolItem* Svx3DShadeModeItem::Clone(SfxItemPool* pPool) const 
/*N*/ { 
/*N*/ 	return new Svx3DShadeModeItem(*this);
/*N*/ }

// EOF
}
