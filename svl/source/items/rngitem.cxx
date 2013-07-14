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

#include <tools/stream.hxx>
#include <svl/rngitem.hxx>

static inline sal_uInt16 Count_Impl(const sal_uInt16 * pRanges)
{
    sal_uInt16 nCount = 0;
    for (; *pRanges; pRanges += 2) nCount += 2;
    return nCount;
}

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxRangeItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SfxUShortRangesItem, SfxPoolItem);

sal_uInt16 Count_Impl( const sal_uInt16 *pRanges );

// -----------------------------------------------------------------------

SfxRangeItem::SfxRangeItem()
{
    nFrom = 0;
    nTo = 0;
}

// -----------------------------------------------------------------------

SfxRangeItem::SfxRangeItem( sal_uInt16 which, sal_uInt16 from, sal_uInt16 to ):
    SfxPoolItem( which ),
    nFrom( from ),
    nTo( to )
{
}

// -----------------------------------------------------------------------

SfxRangeItem::SfxRangeItem( const SfxRangeItem& rItem ) :
    SfxPoolItem( rItem )
{
    nFrom = rItem.nFrom;
    nTo = rItem.nTo;
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxRangeItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper *
)   const
{
    rText = OUString::number(nFrom) + ":" + OUString::number(nTo);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SfxRangeItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    SfxRangeItem* pT = (SfxRangeItem*)&rItem;
    if( nFrom==pT->nFrom && nTo==pT->nTo )
        return 1;
    return 0;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxRangeItem::Clone(SfxItemPool *) const
{
    return new SfxRangeItem( Which(), nFrom, nTo );
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxRangeItem::Create(SvStream &rStream, sal_uInt16) const
{
    sal_uInt16 nVon(0), nBis(0);
    rStream >> nVon;
    rStream >> nBis;
    return new SfxRangeItem( Which(), nVon, nBis );
}

// -----------------------------------------------------------------------

SvStream& SfxRangeItem::Store(SvStream &rStream, sal_uInt16) const
{
    rStream << nFrom;
    rStream << nTo;
    return rStream;
}


SfxUShortRangesItem::SfxUShortRangesItem()
:   _pRanges(0)
{
}


SfxUShortRangesItem::SfxUShortRangesItem( sal_uInt16 nWID, SvStream &rStream )
:   SfxPoolItem( nWID )
{
    sal_uInt16 nCount(0);
    rStream >> nCount;
    _pRanges = new sal_uInt16[nCount + 1];
    for ( sal_uInt16 n = 0; n < nCount; ++n )
        rStream >> _pRanges[n];
    _pRanges[nCount] = 0;
}


SfxUShortRangesItem::SfxUShortRangesItem( const SfxUShortRangesItem& rItem )
:   SfxPoolItem( rItem )
{
    sal_uInt16 nCount = Count_Impl(rItem._pRanges) + 1;
    _pRanges = new sal_uInt16[nCount];
    memcpy( _pRanges, rItem._pRanges, sizeof(sal_uInt16) * nCount );
}


SfxUShortRangesItem::~SfxUShortRangesItem()
{
    delete _pRanges;
}


int SfxUShortRangesItem::operator==( const SfxPoolItem &rItem ) const
{
    const SfxUShortRangesItem &rOther = (const SfxUShortRangesItem&) rItem;
    if ( !_pRanges && !rOther._pRanges )
        return sal_True;
    if ( _pRanges || rOther._pRanges )
        return sal_False;

    sal_uInt16 n;
    for ( n = 0; _pRanges[n] && rOther._pRanges[n]; ++n )
        if ( *_pRanges != rOther._pRanges[n] )
            return 0;

    return !_pRanges[n] && !rOther._pRanges[n];
}


SfxItemPresentation SfxUShortRangesItem::GetPresentation( SfxItemPresentation /*ePres*/,
                                    SfxMapUnit /*eCoreMetric*/,
                                    SfxMapUnit /*ePresMetric*/,
                                    OUString & /*rText*/,
                                    const IntlWrapper * ) const
{
    // not implemented
    return SFX_ITEM_PRESENTATION_NONE;
}


SfxPoolItem* SfxUShortRangesItem::Clone( SfxItemPool * ) const
{
    return new SfxUShortRangesItem( *this );
}


SfxPoolItem* SfxUShortRangesItem::Create( SvStream &rStream, sal_uInt16 ) const
{
    return new SfxUShortRangesItem( Which(), rStream );
}


SvStream& SfxUShortRangesItem::Store( SvStream &rStream, sal_uInt16 ) const
{
    sal_uInt16 nCount = Count_Impl( _pRanges );
    rStream >> nCount;
    for ( sal_uInt16 n = 0; _pRanges[n]; ++n )
        rStream >> _pRanges[n];
    return rStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
