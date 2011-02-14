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
#define _SVX3DITEMS_HXX

#include <svl/intitem.hxx>
#include <svl/eitem.hxx>


#include <editeng/colritem.hxx>
#include <svx/e3ditem.hxx>
#include <svx/viewpt3d.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

// Svx3D _3DOBJ_ Items
class SVX_DLLPUBLIC Svx3DPercentDiagonalItem : public SfxUInt16Item {
public:
    Svx3DPercentDiagonalItem(sal_uInt16 nVal = 10);
};

class Svx3DBackscaleItem : public SfxUInt16Item {
public:
    SVX_DLLPUBLIC Svx3DBackscaleItem(sal_uInt16 nVal = 100);
};

class Svx3DDepthItem : public SfxUInt32Item {
public:
    SVX_DLLPUBLIC Svx3DDepthItem(sal_uInt32 nVal = 1000);
};

class SVX_DLLPUBLIC Svx3DHorizontalSegmentsItem : public SfxUInt32Item {
public:
    Svx3DHorizontalSegmentsItem(sal_uInt32 nVal = 24);
};

class Svx3DVerticalSegmentsItem : public SfxUInt32Item {
public:
    SVX_DLLPUBLIC Svx3DVerticalSegmentsItem(sal_uInt32 nVal = 24);
};

class Svx3DEndAngleItem : public SfxUInt32Item {
public:
    SVX_DLLPUBLIC Svx3DEndAngleItem(sal_uInt32 nVal = 3600);
};

class SVX_DLLPUBLIC Svx3DDoubleSidedItem : public SfxBoolItem {
public:
    Svx3DDoubleSidedItem(sal_Bool bVal = 0);
};

// #i28528#
// Added extra Item (Bool) for chart2 to be able to show reduced line geometry
class Svx3DReducedLineGeometryItem : public SfxBoolItem {
public:
    Svx3DReducedLineGeometryItem(sal_Bool bVal = 0);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
};

class SVX_DLLPUBLIC Svx3DNormalsKindItem : public SfxUInt16Item {
public:
    Svx3DNormalsKindItem(sal_uInt16 nVal = 0);

    // use drawing::NormalsKind
    SVX_DLLPRIVATE virtual  sal_Bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    SVX_DLLPRIVATE virtual  sal_Bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class Svx3DNormalsInvertItem : public SfxBoolItem {
public:
    SVX_DLLPUBLIC Svx3DNormalsInvertItem(sal_Bool bVal = 0);
};

class SVX_DLLPUBLIC Svx3DTextureProjectionXItem : public SfxUInt16Item {
public:
    Svx3DTextureProjectionXItem(sal_uInt16 nVal = 0);

    // use drawing::TextureProjectionMode
    SVX_DLLPRIVATE virtual  sal_Bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    SVX_DLLPRIVATE virtual  sal_Bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class SVX_DLLPUBLIC Svx3DTextureProjectionYItem : public SfxUInt16Item {
public:
    Svx3DTextureProjectionYItem(sal_uInt16 nVal = 0);

    // use drawing::TextureProjectionMode
    virtual sal_Bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class Svx3DShadow3DItem : public SfxBoolItem {
public:
    SVX_DLLPUBLIC Svx3DShadow3DItem(sal_Bool bVal = 0);
};

class Svx3DMaterialColorItem : public SvxColorItem {
public:
    Svx3DMaterialColorItem(const Color& rCol = Color(0x0000b8ff));
};

class Svx3DMaterialEmissionItem : public SvxColorItem {
public:
    SVX_DLLPUBLIC Svx3DMaterialEmissionItem(const Color& rCol = Color(0x00000000));
};

class Svx3DMaterialSpecularItem : public SvxColorItem {
public:
    SVX_DLLPUBLIC Svx3DMaterialSpecularItem(const Color& rCol = Color(0x00ffffff));
};

class Svx3DMaterialSpecularIntensityItem : public SfxUInt16Item {
public:
    SVX_DLLPUBLIC Svx3DMaterialSpecularIntensityItem(sal_uInt16 nVal = 15);
};

class SVX_DLLPUBLIC Svx3DTextureKindItem : public SfxUInt16Item {
public:
    Svx3DTextureKindItem(sal_uInt16 nVal = 3);

    // use drawing::TextureKind
    SVX_DLLPRIVATE virtual  sal_Bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    SVX_DLLPRIVATE virtual  sal_Bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class SVX_DLLPUBLIC Svx3DTextureModeItem : public SfxUInt16Item {
public:
    Svx3DTextureModeItem(sal_uInt16 nVal = 2);

    // use drawing:TextureMode
    SVX_DLLPRIVATE virtual  sal_Bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    SVX_DLLPRIVATE virtual  sal_Bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class Svx3DTextureFilterItem : public SfxBoolItem {
public:
    SVX_DLLPUBLIC Svx3DTextureFilterItem(sal_Bool bVal = 0);
};

// Svx3D _3DSCENE_ Items
class SVX_DLLPUBLIC Svx3DPerspectiveItem : public SfxUInt16Item {
public:
    Svx3DPerspectiveItem(sal_uInt16 nVal = (sal_uInt16)PR_PERSPECTIVE);

    // use drawing::ProjectionMode
    SVX_DLLPRIVATE virtual  sal_Bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    SVX_DLLPRIVATE virtual  sal_Bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class Svx3DDistanceItem : public SfxUInt32Item {
public:
    SVX_DLLPUBLIC Svx3DDistanceItem(sal_uInt32 nVal = 100);
};

class Svx3DFocalLengthItem : public SfxUInt32Item {
public:
    SVX_DLLPUBLIC Svx3DFocalLengthItem(sal_uInt32 nVal = 100);
};

class Svx3DTwoSidedLightingItem : public SfxBoolItem {
public:
    SVX_DLLPUBLIC Svx3DTwoSidedLightingItem(sal_Bool bVal = 0);
};

class SVX_DLLPUBLIC Svx3DLightcolor1Item : public SvxColorItem {
public:
    Svx3DLightcolor1Item(const Color& rCol = Color(0xffcccccc));
};

class SVX_DLLPUBLIC Svx3DLightcolor2Item : public SvxColorItem {
public:
    Svx3DLightcolor2Item(const Color& rCol = Color(0x00000000));
};

class SVX_DLLPUBLIC Svx3DLightcolor3Item : public SvxColorItem {
public:
    Svx3DLightcolor3Item(const Color& rCol = Color(0x00000000));
};

class SVX_DLLPUBLIC Svx3DLightcolor4Item : public SvxColorItem {
public:
    Svx3DLightcolor4Item(const Color& rCol = Color(0x00000000));
};

class SVX_DLLPUBLIC Svx3DLightcolor5Item : public SvxColorItem {
public:
    Svx3DLightcolor5Item(const Color& rCol = Color(0x00000000));
};

class SVX_DLLPUBLIC Svx3DLightcolor6Item : public SvxColorItem {
public:
    Svx3DLightcolor6Item(const Color& rCol = Color(0x00000000));
};

class SVX_DLLPUBLIC Svx3DLightcolor7Item : public SvxColorItem {
public:
    Svx3DLightcolor7Item(const Color& rCol = Color(0x00000000));
};

class SVX_DLLPUBLIC Svx3DLightcolor8Item : public SvxColorItem {
public:
    Svx3DLightcolor8Item(const Color& rCol = Color(0x00000000));
};

class SVX_DLLPUBLIC Svx3DAmbientcolorItem : public SvxColorItem {
public:
    Svx3DAmbientcolorItem(const Color& rCol = Color(0x00666666));
};

class SVX_DLLPUBLIC Svx3DLightOnOff1Item : public SfxBoolItem {
public:
    Svx3DLightOnOff1Item(sal_Bool bVal = 1);
};

class SVX_DLLPUBLIC Svx3DLightOnOff2Item : public SfxBoolItem {
public:
    Svx3DLightOnOff2Item(sal_Bool bVal = 0);
};

class SVX_DLLPUBLIC Svx3DLightOnOff3Item : public SfxBoolItem {
public:
    Svx3DLightOnOff3Item(sal_Bool bVal = 0);
};

class SVX_DLLPUBLIC Svx3DLightOnOff4Item : public SfxBoolItem {
public:
    Svx3DLightOnOff4Item(sal_Bool bVal = 0);
};

class SVX_DLLPUBLIC Svx3DLightOnOff5Item : public SfxBoolItem {
public:
    Svx3DLightOnOff5Item(sal_Bool bVal = 0);
};

class SVX_DLLPUBLIC Svx3DLightOnOff6Item : public SfxBoolItem {
public:
    Svx3DLightOnOff6Item(sal_Bool bVal = 0);
};

class SVX_DLLPUBLIC Svx3DLightOnOff7Item : public SfxBoolItem {
public:
    Svx3DLightOnOff7Item(sal_Bool bVal = 0);
};

class SVX_DLLPUBLIC Svx3DLightOnOff8Item : public SfxBoolItem {
public:
    Svx3DLightOnOff8Item(sal_Bool bVal = 0);
};

class SVX_DLLPUBLIC Svx3DLightDirection1Item : public SvxB3DVectorItem {
public:
    Svx3DLightDirection1Item(const basegfx::B3DVector& rVec = basegfx::B3DVector(0.57735026918963, 0.57735026918963, 0.57735026918963));
};

class SVX_DLLPUBLIC Svx3DLightDirection2Item : public SvxB3DVectorItem {
public:
    Svx3DLightDirection2Item(const basegfx::B3DVector& rVec = basegfx::B3DVector(0.0,0.0,1.0));
};

class SVX_DLLPUBLIC Svx3DLightDirection3Item : public SvxB3DVectorItem {
public:
    Svx3DLightDirection3Item(const basegfx::B3DVector& rVec = basegfx::B3DVector(0.0,0.0,1.0));
};

class SVX_DLLPUBLIC Svx3DLightDirection4Item : public SvxB3DVectorItem {
public:
    Svx3DLightDirection4Item(const basegfx::B3DVector& rVec = basegfx::B3DVector(0.0,0.0,1.0));
};

class SVX_DLLPUBLIC Svx3DLightDirection5Item : public SvxB3DVectorItem {
public:
    Svx3DLightDirection5Item(const basegfx::B3DVector& rVec = basegfx::B3DVector(0.0,0.0,1.0));
};

class SVX_DLLPUBLIC Svx3DLightDirection6Item : public SvxB3DVectorItem {
public:
    Svx3DLightDirection6Item(const basegfx::B3DVector& rVec = basegfx::B3DVector(0.0,0.0,1.0));
};

class SVX_DLLPUBLIC Svx3DLightDirection7Item : public SvxB3DVectorItem {
public:
    Svx3DLightDirection7Item(const basegfx::B3DVector& rVec = basegfx::B3DVector(0.0,0.0,1.0));
};

class SVX_DLLPUBLIC Svx3DLightDirection8Item : public SvxB3DVectorItem {
public:
    Svx3DLightDirection8Item(const basegfx::B3DVector& rVec = basegfx::B3DVector(0.0,0.0,1.0));
};

class Svx3DShadowSlantItem : public SfxUInt16Item {
public:
    SVX_DLLPUBLIC Svx3DShadowSlantItem(sal_uInt16 nVal = 0);
};

class SVX_DLLPUBLIC Svx3DShadeModeItem : public SfxUInt16Item {
public:
    Svx3DShadeModeItem(sal_uInt16 nVal = 2);

    // use drawing::ShadeMode
    SVX_DLLPRIVATE virtual  sal_Bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    SVX_DLLPRIVATE virtual  sal_Bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

//////////////////////////////////////////////////////////////////////////////
// #107245# Item to replace bExtrudeSmoothed and bLatheSmoothed
class Svx3DSmoothNormalsItem : public SfxBoolItem {
public:
    Svx3DSmoothNormalsItem(sal_Bool bVal = 1);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
};

// #107245# Item to replace bExtrudeSmoothFrontBack and bLatheSmoothFrontBack
class Svx3DSmoothLidsItem : public SfxBoolItem {
public:
    Svx3DSmoothLidsItem(sal_Bool bVal = 0);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
};

// #107245# Item to replace bExtrudeCharacterMode and bLatheCharacterMode
class Svx3DCharacterModeItem : public SfxBoolItem {
public:
    Svx3DCharacterModeItem(sal_Bool bVal = 0);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
};

// #107245# Item to replace bExtrudeCloseFront and bLatheCloseFront
class SVX_DLLPUBLIC Svx3DCloseFrontItem : public SfxBoolItem {
public:
    Svx3DCloseFrontItem(sal_Bool bVal = 1);
    SVX_DLLPRIVATE virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    SVX_DLLPRIVATE virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
};

// #107245# Item to replace bExtrudeCloseBack and bLatheCloseBack
class SVX_DLLPUBLIC Svx3DCloseBackItem : public SfxBoolItem {
public:
    Svx3DCloseBackItem(sal_Bool bVal = 1);
    SVX_DLLPRIVATE virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    SVX_DLLPRIVATE virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
};

#endif // _SVX3DITEMS_HXX
