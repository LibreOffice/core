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

#ifndef INCLUDED_SD_INC_SDATTR_HXX
#define INCLUDED_SD_INC_SDATTR_HXX

#include <com/sun/star/presentation/FadeEffect.hpp>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/sfx.hrc>

#include "sdattr.hrc"
#include "glob.hxx"
#include "fadedef.h"
#include "diadef.h"

// layer attributes
class SdAttrLayerName : public SfxStringItem
{
public:
    SdAttrLayerName() :
        SfxStringItem( ATTR_LAYER_NAME, OUString("neue Ebene") ) {}
    SdAttrLayerName( const OUString& aStr ) :
        SfxStringItem( ATTR_LAYER_NAME, aStr ) {}
};

class SdAttrLayerTitle : public SfxStringItem
{
public:
    SdAttrLayerTitle() : SfxStringItem( ATTR_LAYER_TITLE, OUString()) {}
    SdAttrLayerTitle( const OUString& aStr ) : SfxStringItem( ATTR_LAYER_TITLE, aStr ) {}
};

class SdAttrLayerDesc : public SfxStringItem
{
public:
    SdAttrLayerDesc() : SfxStringItem( ATTR_LAYER_DESC, OUString()) {}
    SdAttrLayerDesc( const OUString& aStr ) : SfxStringItem( ATTR_LAYER_DESC, aStr ) {}
};

class SdAttrLayerVisible : public SfxBoolItem
{
public:
    SdAttrLayerVisible( bool bValue = true ) :
        SfxBoolItem( ATTR_LAYER_VISIBLE, bValue ) {}
};

class SdAttrLayerPrintable : public SfxBoolItem
{
public:
    SdAttrLayerPrintable( bool bValue = true ) :
        SfxBoolItem( ATTR_LAYER_PRINTABLE, bValue ) {}
};

class SdAttrLayerLocked : public SfxBoolItem
{
public:
    SdAttrLayerLocked( bool bValue = false ) :
        SfxBoolItem( ATTR_LAYER_LOCKED, bValue ) {}
};

class SdAttrLayerThisPage : public SfxBoolItem
{
public:
    SdAttrLayerThisPage( bool bValue = false ) :
        SfxBoolItem( ATTR_LAYER_THISPAGE, bValue ) {}
};

class DiaEffectItem : public SfxEnumItem
{
public:
            DiaEffectItem( css::presentation::FadeEffect eFade = css::presentation::FadeEffect_NONE );
            DiaEffectItem( SvStream& rIn );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;
            sal_uInt16          GetValueCount() const override { return FADE_EFFECT_COUNT; }
};

class DiaSpeedItem : public SfxEnumItem
{
public:
            DiaSpeedItem( FadeSpeed = FADE_SPEED_MEDIUM );
            DiaSpeedItem( SvStream& rIn );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;
            sal_uInt16          GetValueCount() const override { return FADE_SPEED_COUNT; }
};

class DiaAutoItem : public SfxEnumItem
{
public:
            DiaAutoItem( PresChange = PRESCHANGE_MANUAL );
            DiaAutoItem( SvStream& rIn );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;
            sal_uInt16          GetValueCount() const override { return PRESCHANGE_COUNT; }
};

class DiaTimeItem : public SfxUInt32Item
{
public:
            DiaTimeItem( sal_uInt32 nValue = 0L );

    virtual SfxPoolItem* Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool         operator==( const SfxPoolItem& ) const override;
};

#endif // INCLUDED_SD_INC_SDATTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
