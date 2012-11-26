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



// This snippet of code is included by rngitem.cxx but not compiled directly.
// Ugly hack, probably due to lack of templates in the 20th century.

static inline NUMTYPE Count_Impl(const NUMTYPE * pRanges)
{
    NUMTYPE nCount = 0;
    for (; *pRanges; pRanges += 2) nCount += 2;
    return nCount;
}

// -----------------------------------------------------------------------

NUMTYPE Count_Impl( const NUMTYPE *pRanges );

// -----------------------------------------------------------------------

SfxXRangeItem::SfxXRangeItem()
{
    nFrom = 0;
    nTo = 0;
}

// -----------------------------------------------------------------------

SfxXRangeItem::SfxXRangeItem( sal_uInt16 which, NUMTYPE from, NUMTYPE to ):
    SfxPoolItem( which ),
    nFrom( from ),
    nTo( to )
{
}


// -----------------------------------------------------------------------

SfxXRangeItem::SfxXRangeItem( sal_uInt16 nW, SvStream &rStream ) :
    SfxPoolItem( nW )
{
    rStream >> nFrom;
    rStream >> nTo;
}

// -----------------------------------------------------------------------

SfxXRangeItem::SfxXRangeItem( const SfxXRangeItem& rItem ) :
    SfxPoolItem( rItem )
{
    nFrom = rItem.nFrom;
    nTo = rItem.nTo;
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxXRangeItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    XubString&              rText,
    const IntlWrapper *
)   const
{
    rText = UniString::CreateFromInt64(nFrom);
    rText += ':';
    rText += UniString::CreateFromInt64(nTo);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SfxXRangeItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    SfxXRangeItem* pT = (SfxXRangeItem*)&rItem;
    if( nFrom==pT->nFrom && nTo==pT->nTo )
        return 1;
    return 0;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxXRangeItem::Clone(SfxItemPool *) const
{
    return new SfxXRangeItem( Which(), nFrom, nTo );
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxXRangeItem::Create(SvStream &rStream, sal_uInt16) const
{
    NUMTYPE     nVon, nBis;
    rStream >> nVon;
    rStream >> nBis;
    return new SfxXRangeItem( Which(), nVon, nBis );
}

// -----------------------------------------------------------------------

SvStream& SfxXRangeItem::Store(SvStream &rStream, sal_uInt16) const
{
    rStream << nFrom;
    rStream << nTo;
    return rStream;
}

//=========================================================================

SfxXRangesItem::SfxXRangesItem()
:   _pRanges(0)
{
}

//-------------------------------------------------------------------------

SfxXRangesItem::SfxXRangesItem( sal_uInt16 nWID, const NUMTYPE *pRanges )
:   SfxPoolItem( nWID )
{
    NUMTYPE nCount = Count_Impl(pRanges) + 1;
    _pRanges = new NUMTYPE[nCount];
    memcpy( _pRanges, pRanges, sizeof(NUMTYPE) * nCount );
}

//-------------------------------------------------------------------------

SfxXRangesItem::SfxXRangesItem( sal_uInt16 nWID, SvStream &rStream )
:   SfxPoolItem( nWID )
{
    NUMTYPE nCount;
    rStream >> nCount;
    _pRanges = new NUMTYPE[nCount + 1];
    for ( NUMTYPE n = 0; n < nCount; ++n )
        rStream >> _pRanges[n];
    _pRanges[nCount] = 0;
}

//-------------------------------------------------------------------------

SfxXRangesItem::SfxXRangesItem( const SfxXRangesItem& rItem )
:   SfxPoolItem( rItem )
{
    NUMTYPE nCount = Count_Impl(rItem._pRanges) + 1;
    _pRanges = new NUMTYPE[nCount];
    memcpy( _pRanges, rItem._pRanges, sizeof(NUMTYPE) * nCount );
}

//-------------------------------------------------------------------------

SfxXRangesItem::~SfxXRangesItem()
{
    delete _pRanges;
}

//-------------------------------------------------------------------------

int SfxXRangesItem::operator==( const SfxPoolItem &rItem ) const
{
    const SfxXRangesItem &rOther = (const SfxXRangesItem&) rItem;
    if ( !_pRanges && !rOther._pRanges )
        return sal_True;
    if ( _pRanges || rOther._pRanges )
        return sal_False;

    NUMTYPE n;
    for ( n = 0; _pRanges[n] && rOther._pRanges[n]; ++n )
        if ( *_pRanges != rOther._pRanges[n] )
            return 0;

    return !_pRanges[n] && !rOther._pRanges[n];
}

//-------------------------------------------------------------------------

SfxItemPresentation SfxXRangesItem::GetPresentation( SfxItemPresentation /*ePres*/,
                                    SfxMapUnit /*eCoreMetric*/,
                                    SfxMapUnit /*ePresMetric*/,
                                    XubString &/*rText*/,
                                    const IntlWrapper * ) const
{
    HACK(n. i.)
    return SFX_ITEM_PRESENTATION_NONE;
}

//-------------------------------------------------------------------------

SfxPoolItem* SfxXRangesItem::Clone( SfxItemPool * ) const
{
    return new SfxXRangesItem( *this );
}

//-------------------------------------------------------------------------

SfxPoolItem* SfxXRangesItem::Create( SvStream &rStream, sal_uInt16 ) const
{
    return new SfxXRangesItem( Which(), rStream );
}

//-------------------------------------------------------------------------

SvStream& SfxXRangesItem::Store( SvStream &rStream, sal_uInt16 ) const
{
    NUMTYPE nCount = Count_Impl( _pRanges );
    rStream >> nCount;
    for ( NUMTYPE n = 0; _pRanges[n]; ++n )
        rStream >> _pRanges[n];
    return rStream;
}


#undef NUMTYPE
#undef SfxXRangeItem
#undef SfxXRangesItem
