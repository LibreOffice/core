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
inline SfxStringItem makeSdAttrLayerName( const OUString& aStr )
{
    return SfxStringItem( ATTR_LAYER_NAME, aStr );
}

inline SfxStringItem makeSdAttrLayerTitle( const OUString& aStr = OUString() )
{
    return SfxStringItem( ATTR_LAYER_TITLE, aStr );
}

inline SfxStringItem makeSdAttrLayerDesc( const OUString& aStr = OUString() )
{
    return SfxStringItem( ATTR_LAYER_DESC, aStr );
}

inline SfxBoolItem makeSdAttrLayerVisible( bool bValue = true )
{
    return SfxBoolItem( ATTR_LAYER_VISIBLE, bValue );
}

inline SfxBoolItem makeSdAttrLayerPrintable( bool bValue = true )
{
    return SfxBoolItem( ATTR_LAYER_PRINTABLE, bValue );
}

inline SfxBoolItem makeSdAttrLayerLocked( bool bValue = false )
{
    return SfxBoolItem( ATTR_LAYER_LOCKED, bValue );
}

inline SfxBoolItem makeSdAttrLayerThisPage()
{
    return SfxBoolItem( ATTR_LAYER_THISPAGE, false );
}

class DiaEffectItem : public SfxEnumItem<sal_uInt16>
{
public:
            DiaEffectItem( SvStream& rIn );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;
            sal_uInt16      GetValueCount() const override { return FADE_EFFECT_COUNT; }
};

class DiaSpeedItem : public SfxEnumItem<sal_uInt16>
{
public:
            DiaSpeedItem( SvStream& rIn );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;
            sal_uInt16      GetValueCount() const override { return FADE_SPEED_COUNT; }
};

class DiaAutoItem : public SfxEnumItem<PresChange>
{
public:
            DiaAutoItem( SvStream& rIn );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;
            sal_uInt16      GetValueCount() const override { return PRESCHANGE_COUNT; }
};

#endif // INCLUDED_SD_INC_SDATTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
