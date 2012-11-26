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
// Added extra Item (Bool) for chart2 to be able to show reduced line geometry
class Svx3DReducedLineGeometryItem : public SfxBoolItem {
public:
    Svx3DReducedLineGeometryItem(sal_Bool bVal = 0);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
    virtual SfxPoolItem* Clone(SfxItemPool * = 0) const;
};

class SVX_DLLPUBLIC Svx3DNormalsKindItem : public SfxUInt16Item {
public:
    Svx3DNormalsKindItem(sal_uInt16 nVal = 0);

    // use drawing::NormalsKind
    SVX_DLLPRIVATE virtual  sal_Bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    SVX_DLLPRIVATE virtual  sal_Bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
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

// Svx3D _3DSCENE_ Items
class SVX_DLLPUBLIC Svx3DPerspectiveItem : public SfxUInt16Item {
public:
    Svx3DPerspectiveItem(sal_uInt16 nVal = (sal_uInt16)PR_PERSPECTIVE);

    // use drawing::ProjectionMode
    SVX_DLLPRIVATE virtual  sal_Bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    SVX_DLLPRIVATE virtual  sal_Bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool* pPool = NULL) const;
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
    virtual SfxPoolItem* Clone(SfxItemPool * = 0) const;
};

// #107245# Item to replace bExtrudeSmoothFrontBack and bLatheSmoothFrontBack
class Svx3DSmoothLidsItem : public SfxBoolItem {
public:
    Svx3DSmoothLidsItem(sal_Bool bVal = 0);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
    virtual SfxPoolItem* Clone(SfxItemPool * = 0) const;
};

// #107245# Item to replace bExtrudeCharacterMode and bLatheCharacterMode
class Svx3DCharacterModeItem : public SfxBoolItem {
public:
    Svx3DCharacterModeItem(sal_Bool bVal = 0);
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
    virtual SfxPoolItem* Clone(SfxItemPool * = 0) const;
};

// #107245# Item to replace bExtrudeCloseFront and bLatheCloseFront
class SVX_DLLPUBLIC Svx3DCloseFrontItem : public SfxBoolItem {
public:
    Svx3DCloseFrontItem(sal_Bool bVal = 1);
    SVX_DLLPRIVATE virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    SVX_DLLPRIVATE virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool * = 0) const;
};

// #107245# Item to replace bExtrudeCloseBack and bLatheCloseBack
class SVX_DLLPUBLIC Svx3DCloseBackItem : public SfxBoolItem {
public:
    Svx3DCloseBackItem(sal_Bool bVal = 1);
    SVX_DLLPRIVATE virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nItemVersion) const;
    SVX_DLLPRIVATE virtual sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion) const;
    SVX_DLLPRIVATE virtual SfxPoolItem* Clone(SfxItemPool * = 0) const;
};

#endif // _SVX3DITEMS_HXX
