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

#ifndef _SFXINTITEM_HXX
#include <bf_svtools/intitem.hxx>
#endif

#ifndef _SFXENUMITEM_HXX
#include <bf_svtools/eitem.hxx>
#endif

#ifdef ITEMID_COLOR
#undef ITEMID_COLOR
#endif
#define ITEMID_COLOR			(SID_ATTR_3D_LIGHTCOLOR)
#ifndef _SVX_COLRITEM_HXX
#include <bf_svx/colritem.hxx>
#endif

#ifndef _SVXE3DITEM_HXX
#include <bf_svx/e3ditem.hxx>
#endif

#ifndef _VIEWPT3D_HXX
#include <bf_svx/viewpt3d.hxx>
#endif

#ifndef _SVDDEF_HXX
#include <bf_svx/svddef.hxx>
#endif
namespace binfilter {

// Svx3D _3DOBJ_ Items
class Svx3DPercentDiagonalItem : public SfxUInt16Item {
public:
    Svx3DPercentDiagonalItem(sal_uInt16 nVal = 10);
};

class Svx3DBackscaleItem : public SfxUInt16Item {
public:
    Svx3DBackscaleItem(sal_uInt16 nVal = 100);
};

class Svx3DDepthItem : public SfxUInt32Item {
public:
    Svx3DDepthItem(sal_uInt32 nVal = 1000);
};

class Svx3DHorizontalSegmentsItem : public SfxUInt32Item {
public:
    Svx3DHorizontalSegmentsItem(sal_uInt32 nVal = 24);
};

class Svx3DVerticalSegmentsItem : public SfxUInt32Item {
public:
    Svx3DVerticalSegmentsItem(sal_uInt32 nVal = 24);
};

class Svx3DEndAngleItem : public SfxUInt32Item {
public:
    Svx3DEndAngleItem(sal_uInt32 nVal = 3600);
};

class Svx3DDoubleSidedItem : public SfxBoolItem {
public:
    Svx3DDoubleSidedItem(BOOL bVal = 0);
};

class Svx3DNormalsKindItem : public SfxUInt16Item {
public:
    Svx3DNormalsKindItem(sal_uInt16 nVal = 0);

    // use drawing::NormalsKind
    virtual	sal_Bool QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class Svx3DNormalsInvertItem : public SfxBoolItem {
public:
    Svx3DNormalsInvertItem(BOOL bVal = 0);
};

class Svx3DTextureProjectionXItem : public SfxUInt16Item {
public:
    Svx3DTextureProjectionXItem(sal_uInt16 nVal = 0);

    // use drawing::TextureProjectionMode
    virtual	sal_Bool QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class Svx3DTextureProjectionYItem : public SfxUInt16Item {
public:
    Svx3DTextureProjectionYItem(sal_uInt16 nVal = 0);

    // use drawing::TextureProjectionMode
    virtual	sal_Bool QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class Svx3DShadow3DItem : public SfxBoolItem {
public:
    Svx3DShadow3DItem(BOOL bVal = 0);
};

class Svx3DMaterialColorItem : public SvxColorItem {
public:
    Svx3DMaterialColorItem(const Color& rCol = Color(0x0000b8ff));
};

class Svx3DMaterialEmissionItem : public SvxColorItem {
public:
    Svx3DMaterialEmissionItem(const Color& rCol = Color(0x00000000));
};

class Svx3DMaterialSpecularItem : public SvxColorItem {
public:
    Svx3DMaterialSpecularItem(const Color& rCol = Color(0x00ffffff));
};

class Svx3DMaterialSpecularIntensityItem : public SfxUInt16Item {
public:
    Svx3DMaterialSpecularIntensityItem(sal_uInt16 nVal = 15);
};

class Svx3DTextureKindItem : public SfxUInt16Item {
public:
    Svx3DTextureKindItem(sal_uInt16 nVal = 3);

    // use drawing::TextureKind
    virtual	sal_Bool QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class Svx3DTextureModeItem : public SfxUInt16Item {
public:
    Svx3DTextureModeItem(sal_uInt16 nVal = 2);

    // use drawing:TextureMode
    virtual	sal_Bool QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class Svx3DTextureFilterItem : public SfxBoolItem {
public:
    Svx3DTextureFilterItem(BOOL bVal = 0);
};

// Svx3D _3DSCENE_ Items
class Svx3DPerspectiveItem : public SfxUInt16Item {
public:
    Svx3DPerspectiveItem(sal_uInt16 nVal = (sal_uInt16)PR_PERSPECTIVE);

    // use drawing::ProjectionMode
    virtual	sal_Bool QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

class Svx3DDistanceItem : public SfxUInt32Item {
public:
    Svx3DDistanceItem(sal_uInt32 nVal = 100);
};

class Svx3DFocalLengthItem : public SfxUInt32Item {
public:
    Svx3DFocalLengthItem(sal_uInt32 nVal = 100);
};

class Svx3DTwoSidedLightingItem : public SfxBoolItem {
public:
    Svx3DTwoSidedLightingItem(BOOL bVal = 0);
};

class Svx3DLightcolor1Item : public SvxColorItem {
public:
    Svx3DLightcolor1Item(const Color& rCol = Color(0xffcccccc));
};

class Svx3DLightcolor2Item : public SvxColorItem {
public:
    Svx3DLightcolor2Item(const Color& rCol = Color(0x00000000));
};

class Svx3DLightcolor3Item : public SvxColorItem {
public:
    Svx3DLightcolor3Item(const Color& rCol = Color(0x00000000));
};

class Svx3DLightcolor4Item : public SvxColorItem {
public:
    Svx3DLightcolor4Item(const Color& rCol = Color(0x00000000));
};

class Svx3DLightcolor5Item : public SvxColorItem {
public:
    Svx3DLightcolor5Item(const Color& rCol = Color(0x00000000));
};

class Svx3DLightcolor6Item : public SvxColorItem {
public:
    Svx3DLightcolor6Item(const Color& rCol = Color(0x00000000));
};

class Svx3DLightcolor7Item : public SvxColorItem {
public:
    Svx3DLightcolor7Item(const Color& rCol = Color(0x00000000));
};

class Svx3DLightcolor8Item : public SvxColorItem {
public:
    Svx3DLightcolor8Item(const Color& rCol = Color(0x00000000));
};

class Svx3DAmbientcolorItem : public SvxColorItem {
public:
    Svx3DAmbientcolorItem(const Color& rCol = Color(0x00666666));
};

class Svx3DLightOnOff1Item : public SfxBoolItem {
public:
    Svx3DLightOnOff1Item(BOOL bVal = 1);
};

class Svx3DLightOnOff2Item : public SfxBoolItem {
public:
    Svx3DLightOnOff2Item(BOOL bVal = 0);
};

class Svx3DLightOnOff3Item : public SfxBoolItem {
public:
    Svx3DLightOnOff3Item(BOOL bVal = 0);
};

class Svx3DLightOnOff4Item : public SfxBoolItem {
public:
    Svx3DLightOnOff4Item(BOOL bVal = 0);
};

class Svx3DLightOnOff5Item : public SfxBoolItem {
public:
    Svx3DLightOnOff5Item(BOOL bVal = 0);
};

class Svx3DLightOnOff6Item : public SfxBoolItem {
public:
    Svx3DLightOnOff6Item(BOOL bVal = 0);
};

class Svx3DLightOnOff7Item : public SfxBoolItem {
public:
    Svx3DLightOnOff7Item(BOOL bVal = 0);
};

class Svx3DLightOnOff8Item : public SfxBoolItem {
public:
    Svx3DLightOnOff8Item(BOOL bVal = 0);
};

class Svx3DLightDirection1Item : public SvxVector3DItem {
public:
    Svx3DLightDirection1Item(const Vector3D& rVec = Vector3D(0.57735026918963, 0.57735026918963, 0.57735026918963));
};

class Svx3DLightDirection2Item : public SvxVector3DItem {
public:
    Svx3DLightDirection2Item(const Vector3D& rVec = Vector3D(0.0,0.0,1.0));
};

class Svx3DLightDirection3Item : public SvxVector3DItem {
public:
    Svx3DLightDirection3Item(const Vector3D& rVec = Vector3D(0.0,0.0,1.0));
};

class Svx3DLightDirection4Item : public SvxVector3DItem {
public:
    Svx3DLightDirection4Item(const Vector3D& rVec = Vector3D(0.0,0.0,1.0));
};

class Svx3DLightDirection5Item : public SvxVector3DItem {
public:
    Svx3DLightDirection5Item(const Vector3D& rVec = Vector3D(0.0,0.0,1.0));
};

class Svx3DLightDirection6Item : public SvxVector3DItem {
public:
    Svx3DLightDirection6Item(const Vector3D& rVec = Vector3D(0.0,0.0,1.0));
};

class Svx3DLightDirection7Item : public SvxVector3DItem {
public:
    Svx3DLightDirection7Item(const Vector3D& rVec = Vector3D(0.0,0.0,1.0));
};

class Svx3DLightDirection8Item : public SvxVector3DItem {
public:
    Svx3DLightDirection8Item(const Vector3D& rVec = Vector3D(0.0,0.0,1.0));
};

class Svx3DShadowSlantItem : public SfxUInt16Item {
public:
    Svx3DShadowSlantItem(sal_uInt16 nVal = 0);
};

class Svx3DShadeModeItem : public SfxUInt16Item {
public:
    Svx3DShadeModeItem(sal_uInt16 nVal = 2);

    // use drawing::ShadeMode 
    virtual	sal_Bool QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
};

//////////////////////////////////////////////////////////////////////////////
// #107245# Item to replace bExtrudeSmoothed and bLatheSmoothed
class Svx3DSmoothNormalsItem : public SfxBoolItem {
public:
    Svx3DSmoothNormalsItem(BOOL bVal = 1);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
};

// #107245# Item to replace bExtrudeSmoothFrontBack and bLatheSmoothFrontBack
class Svx3DSmoothLidsItem : public SfxBoolItem {
public:
    Svx3DSmoothLidsItem(BOOL bVal = 0);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
};

// #107245# Item to replace bExtrudeCharacterMode and bLatheCharacterMode
class Svx3DCharacterModeItem : public SfxBoolItem {
public:
    Svx3DCharacterModeItem(BOOL bVal = 0);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
};

// #107245# Item to replace bExtrudeCloseFront and bLatheCloseFront
class Svx3DCloseFrontItem : public SfxBoolItem {
public:
    Svx3DCloseFrontItem(BOOL bVal = 1);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
};

// #107245# Item to replace bExtrudeCloseBack and bLatheCloseBack
class Svx3DCloseBackItem : public SfxBoolItem {
public:
    Svx3DCloseBackItem(BOOL bVal = 1);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
};

}//end of namespace binfilter
#endif // _SVX3DITEMS_HXX
