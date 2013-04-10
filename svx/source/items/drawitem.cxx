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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <svx/svxids.hrc>


#include "svx/drawitem.hxx"
#include <svx/xtable.hxx>

using namespace ::com::sun::star;

// -----------------------------------------------------------------------

TYPEINIT1_FACTORY( SvxColorTableItem, SfxPoolItem , new  SvxColorTableItem);
TYPEINIT1_FACTORY( SvxGradientListItem, SfxPoolItem , new  SvxGradientListItem);
TYPEINIT1_FACTORY( SvxHatchListItem, SfxPoolItem , new  SvxHatchListItem);
TYPEINIT1_FACTORY( SvxBitmapListItem, SfxPoolItem , new  SvxBitmapListItem);
TYPEINIT1_FACTORY( SvxDashListItem, SfxPoolItem , new  SvxDashListItem);
TYPEINIT1_FACTORY( SvxLineEndListItem, SfxPoolItem , new  SvxLineEndListItem);

//==================================================================
//
//  SvxColorTableItem
//
//==================================================================

SvxColorTableItem::SvxColorTableItem()
{
}

// -----------------------------------------------------------------------

SvxColorTableItem::SvxColorTableItem( XColorList* pTable, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pColorTable( pTable )
{
}

// -----------------------------------------------------------------------

SvxColorTableItem::SvxColorTableItem( const SvxColorTableItem& rItem ) :
    SfxPoolItem( rItem ),
    pColorTable( rItem.pColorTable )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxColorTableItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

int SvxColorTableItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ( ( SvxColorTableItem& ) rItem).pColorTable == pColorTable;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxColorTableItem::Clone( SfxItemPool * ) const
{
    return new SvxColorTableItem( *this );
}

// -----------------------------------------------------------------------

sal_Bool SvxColorTableItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = sal_Int64( (sal_uLong)pColorTable );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxColorTableItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pColorTable = (XColorList*)(sal_uLong)aValue;
        return sal_True;
    }

    return sal_False;
}

//==================================================================
//
//  SvxGradientListItem
//
//==================================================================

SvxGradientListItem::SvxGradientListItem()
{
}

// -----------------------------------------------------------------------

SvxGradientListItem::SvxGradientListItem( XGradientList* pList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pGradientList( pList )
{
}

// -----------------------------------------------------------------------

SvxGradientListItem::SvxGradientListItem( const SvxGradientListItem& rItem ) :
    SfxPoolItem( rItem ),
    pGradientList( rItem.pGradientList )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxGradientListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

int SvxGradientListItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ( ( SvxGradientListItem& ) rItem).pGradientList == pGradientList;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxGradientListItem::Clone( SfxItemPool * ) const
{
    return new SvxGradientListItem( *this );
}

// -----------------------------------------------------------------------

sal_Bool SvxGradientListItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = sal_Int64( (sal_uLong)pGradientList );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxGradientListItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pGradientList = (XGradientList *)(sal_uLong)aValue;
        return sal_True;
    }

    return sal_False;
}

//==================================================================
//
//  SvxHatchListItem
//
//==================================================================

SvxHatchListItem::SvxHatchListItem()
{
}

// -----------------------------------------------------------------------

SvxHatchListItem::SvxHatchListItem( XHatchList* pList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pHatchList( pList )
{
}

// -----------------------------------------------------------------------

SvxHatchListItem::SvxHatchListItem( const SvxHatchListItem& rItem ) :
    SfxPoolItem( rItem ),
    pHatchList( rItem.pHatchList )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxHatchListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

int SvxHatchListItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ( ( SvxHatchListItem& ) rItem).pHatchList == pHatchList;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxHatchListItem::Clone( SfxItemPool * ) const
{
    return new SvxHatchListItem( *this );
}

// -----------------------------------------------------------------------

sal_Bool SvxHatchListItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = sal_Int64( (sal_uLong)pHatchList );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxHatchListItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pHatchList = (XHatchList *)(sal_uLong)aValue;
        return sal_True;
    }

    return sal_False;
}

//==================================================================
//
//  SvxBitmapListItem
//
//==================================================================

SvxBitmapListItem::SvxBitmapListItem()
{
}

// -----------------------------------------------------------------------

SvxBitmapListItem::SvxBitmapListItem( XBitmapList* pList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pBitmapList( pList )
{
}

// -----------------------------------------------------------------------

SvxBitmapListItem::SvxBitmapListItem( const SvxBitmapListItem& rItem ) :
    SfxPoolItem( rItem ),
    pBitmapList( rItem.pBitmapList )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxBitmapListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

int SvxBitmapListItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ( ( SvxBitmapListItem& ) rItem).pBitmapList == pBitmapList;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBitmapListItem::Clone( SfxItemPool * ) const
{
    return new SvxBitmapListItem( *this );
}

// -----------------------------------------------------------------------

sal_Bool SvxBitmapListItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = sal_Int64( (sal_uLong)pBitmapList );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxBitmapListItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pBitmapList = (XBitmapList *)(sal_uLong)aValue;
        return sal_True;
    }

    return sal_False;
}


//==================================================================
//
//  SvxDashListItem
//
//==================================================================

SvxDashListItem::SvxDashListItem() :
    pDashList( 0 )
{
}

// -----------------------------------------------------------------------

SvxDashListItem::SvxDashListItem( XDashList* pList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pDashList( pList )
{
}

// -----------------------------------------------------------------------

SvxDashListItem::SvxDashListItem( const SvxDashListItem& rItem ) :
    SfxPoolItem( rItem ),
    pDashList( rItem.pDashList )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxDashListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

int SvxDashListItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ( ( SvxDashListItem& ) rItem).pDashList == pDashList;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxDashListItem::Clone( SfxItemPool * ) const
{
    return new SvxDashListItem( *this );
}

// -----------------------------------------------------------------------

sal_Bool SvxDashListItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    sal_Int64 aValue = sal_Int64( (sal_uLong)pDashList );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxDashListItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pDashList = (XDashList *)(sal_uLong)aValue;
        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void SvxDashListItem::SetDashList( XDashList* pList )
{
    pDashList = pList;
}

//==================================================================
//
//  SvxLineEndListItem
//
//==================================================================

SvxLineEndListItem::SvxLineEndListItem()
{
}

// -----------------------------------------------------------------------

SvxLineEndListItem::SvxLineEndListItem( XLineEndList* pList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pLineEndList( pList )
{
}

// -----------------------------------------------------------------------

SvxLineEndListItem::SvxLineEndListItem( const SvxLineEndListItem& rItem ) :
    SfxPoolItem( rItem ),
    pLineEndList( rItem.pLineEndList )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLineEndListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

int SvxLineEndListItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ( ( SvxLineEndListItem& ) rItem).pLineEndList == pLineEndList;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLineEndListItem::Clone( SfxItemPool * ) const
{
    return new SvxLineEndListItem( *this );
}

// -----------------------------------------------------------------------

sal_Bool SvxLineEndListItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    sal_Int64 aValue = sal_Int64( (sal_uLong)pLineEndList );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxLineEndListItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pLineEndList = (XLineEndList *)(sal_uLong)aValue;
        return sal_True;
    }

    return sal_False;
}
