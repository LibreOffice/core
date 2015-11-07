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

#include "sdattr.hxx"

using namespace ::com::sun::star;

/*************************************************************************
|*
|*  DiaEffectItem
|*
*************************************************************************/

DiaEffectItem::DiaEffectItem( presentation::FadeEffect eFE ) :
    SfxEnumItem( ATTR_DIA_EFFECT, (sal_uInt16)eFE )
{
}

DiaEffectItem::DiaEffectItem( SvStream& rIn ) :
    SfxEnumItem( ATTR_DIA_EFFECT, rIn )
{
}

SfxPoolItem* DiaEffectItem::Clone( SfxItemPool* ) const
{
    return new DiaEffectItem( *this );
}

SfxPoolItem* DiaEffectItem::Create( SvStream& rIn, sal_uInt16 ) const
{
    return new DiaEffectItem( rIn );
}

/*************************************************************************
|*
|*  DiaSpeedItem
|*
*************************************************************************/

DiaSpeedItem::DiaSpeedItem( FadeSpeed eFS ) :
    SfxEnumItem( ATTR_DIA_SPEED, (sal_uInt16)eFS )
{
}

DiaSpeedItem::DiaSpeedItem( SvStream& rIn ) :
    SfxEnumItem( ATTR_DIA_SPEED, rIn )
{
}

SfxPoolItem* DiaSpeedItem::Clone( SfxItemPool* ) const
{
    return new DiaSpeedItem( *this );
}

SfxPoolItem* DiaSpeedItem::Create( SvStream& rIn, sal_uInt16 ) const
{
    return new DiaSpeedItem( rIn );
}

/*************************************************************************
|*
|*  DiaAutoItem
|*
*************************************************************************/

DiaAutoItem::DiaAutoItem( PresChange eChange ) :
    SfxEnumItem( ATTR_DIA_AUTO, (sal_uInt16)eChange )
{
}

DiaAutoItem::DiaAutoItem( SvStream& rIn ) :
    SfxEnumItem( ATTR_DIA_AUTO, rIn )
{
}

SfxPoolItem* DiaAutoItem::Clone( SfxItemPool* ) const
{
    return new DiaAutoItem( *this );
}

SfxPoolItem* DiaAutoItem::Create( SvStream& rIn, sal_uInt16 ) const
{
    return new DiaAutoItem( rIn );
}

/*************************************************************************
|*
|*  DiaTimeItem
|*
*************************************************************************/

DiaTimeItem::DiaTimeItem( sal_uInt32 nValue ) :
        SfxUInt32Item( ATTR_DIA_TIME, nValue )
{
}

SfxPoolItem* DiaTimeItem::Clone( SfxItemPool* ) const
{
    return new DiaTimeItem( *this );
}

bool DiaTimeItem::operator==( const SfxPoolItem& rItem ) const
{
    return static_cast<const DiaTimeItem&>( rItem ).GetValue() == GetValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
