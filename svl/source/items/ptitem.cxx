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
#include "precompiled_svl.hxx"

#include <svl/ptitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <tools/stream.hxx>

#include <svl/poolitem.hxx>
#include <svl/memberid.hrc>

using namespace ::com::sun::star;
// STATIC DATA -----------------------------------------------------------

IMPL_POOLITEM_FACTORY(SfxPointItem)
DBG_NAME(SfxPointItem)

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

// -----------------------------------------------------------------------

SfxPointItem::SfxPointItem()
{
    DBG_CTOR(SfxPointItem, 0);
}

// -----------------------------------------------------------------------

SfxPointItem::SfxPointItem( sal_uInt16 nW, const Point& rVal ) :
    SfxPoolItem( nW ),
    aVal( rVal )
{
    DBG_CTOR(SfxPointItem, 0);
}

// -----------------------------------------------------------------------

SfxPointItem::SfxPointItem( sal_uInt16 nW, SvStream &rStream ) :
    SfxPoolItem( nW )
{
    DBG_CTOR(SfxPointItem, 0);
    rStream >> aVal;
}

// -----------------------------------------------------------------------

SfxPointItem::SfxPointItem( const SfxPointItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
    DBG_CTOR(SfxPointItem, 0);
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxPointItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    XubString&              rText,
    const IntlWrapper *
)   const
{
    DBG_CHKTHIS(SfxPointItem, 0);
    rText = UniString::CreateFromInt32(aVal.X());
    rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    rText += UniString::CreateFromInt32(aVal.Y());
    rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SfxPointItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS(SfxPointItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ((SfxPointItem&)rItem).aVal == aVal;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxPointItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxPointItem, 0);
    return new SfxPointItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxPointItem::Create(SvStream &rStream, sal_uInt16 ) const
{
    DBG_CHKTHIS(SfxPointItem, 0);
    Point aStr;
    rStream >> aStr;
    return new SfxPointItem(Which(), aStr);
}

// -----------------------------------------------------------------------

SvStream& SfxPointItem::Store(SvStream &rStream, sal_uInt16 ) const
{
    DBG_CHKTHIS(SfxPointItem, 0);
    rStream << aVal;
    return rStream;
}

// -----------------------------------------------------------------------

sal_Bool SfxPointItem::QueryValue( uno::Any& rVal,
                               sal_uInt8 nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    awt::Point aTmp(aVal.X(), aVal.Y());
    if( bConvert )
    {
        aTmp.X = TWIP_TO_MM100(aTmp.X);
        aTmp.Y = TWIP_TO_MM100(aTmp.Y);
    }
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0: rVal <<= aTmp; break;
        case MID_X: rVal <<= aTmp.X; break;
        case MID_Y: rVal <<= aTmp.Y; break;
        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SfxPointItem::PutValue( const uno::Any& rVal,
                             sal_uInt8 nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_False;
    awt::Point aValue;
    sal_Int32 nVal = 0;
    if ( !nMemberId )
    {
        bRet = ( rVal >>= aValue );
        if( bConvert )
        {
            aValue.X = MM100_TO_TWIP(aValue.X);
            aValue.Y = MM100_TO_TWIP(aValue.Y);
        }
    }
    else
    {
        bRet = ( rVal >>= nVal );
        if( bConvert )
            nVal = MM100_TO_TWIP( nVal );
    }

    if ( bRet )
    {
        switch ( nMemberId )
        {
            case 0: aVal.setX( aValue.X ); aVal.setY( aValue.Y ); break;
            case MID_X: aVal.setX( nVal ); break;
            case MID_Y: aVal.setY( nVal ); break;
            default: DBG_ERROR("Wrong MemberId!"); return sal_False;
        }
    }

    return bRet;
}



