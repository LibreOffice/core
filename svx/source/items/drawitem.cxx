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

SvxColorTableItem::SvxColorTableItem( XColorListSharedPtr aTable, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    maColorTable( aTable )
{
}

// -----------------------------------------------------------------------

SvxColorTableItem::SvxColorTableItem( const SvxColorTableItem& rItem ) :
    SfxPoolItem( rItem ),
    maColorTable( rItem.maColorTable )
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
    return static_cast< const SvxColorTableItem& >(rItem).maColorTable == maColorTable;
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
    sal_Int64 aValue = sal_Int64((sal_uLong)&maColorTable);
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
        if(aValue)
            maColorTable = *((XColorListSharedPtr*)(sal_uLong)aValue);
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

SvxGradientListItem::SvxGradientListItem( XGradientListSharedPtr aList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    maGradientList( aList )
{
}

// -----------------------------------------------------------------------

SvxGradientListItem::SvxGradientListItem( const SvxGradientListItem& rItem ) :
    SfxPoolItem( rItem ),
    maGradientList( rItem.maGradientList )
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
    return static_cast< const SvxGradientListItem& >(rItem).maGradientList == maGradientList;
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
    sal_Int64 aValue = sal_Int64((sal_uLong)&maGradientList);
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
        if(aValue)
            maGradientList = *((XGradientListSharedPtr*)(sal_uLong)aValue);
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

SvxHatchListItem::SvxHatchListItem( XHatchListSharedPtr aList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    maHatchList( aList )
{
}

// -----------------------------------------------------------------------

SvxHatchListItem::SvxHatchListItem( const SvxHatchListItem& rItem ) :
    SfxPoolItem( rItem ),
    maHatchList( rItem.maHatchList )
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
    return static_cast< const SvxHatchListItem& >(rItem).maHatchList == maHatchList;
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
    sal_Int64 aValue = sal_Int64((sal_uLong)&maHatchList );
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
        if(aValue)
            maHatchList = *((XHatchListSharedPtr*)(sal_uLong)aValue);
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

SvxBitmapListItem::SvxBitmapListItem( XBitmapListSharedPtr aList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    maBitmapList( aList )
{
}

// -----------------------------------------------------------------------

SvxBitmapListItem::SvxBitmapListItem( const SvxBitmapListItem& rItem ) :
    SfxPoolItem( rItem ),
    maBitmapList( rItem.maBitmapList )
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
    return static_cast< const SvxBitmapListItem& >(rItem).maBitmapList == maBitmapList;
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
    sal_Int64 aValue = sal_Int64((sal_uLong)&maBitmapList );
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
        if(aValue)
            maBitmapList = *((XBitmapListSharedPtr*)(sal_uLong)aValue);
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
    maDashList()
{
}

// -----------------------------------------------------------------------

SvxDashListItem::SvxDashListItem( XDashListSharedPtr aList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    maDashList( aList )
{
}

// -----------------------------------------------------------------------

SvxDashListItem::SvxDashListItem( const SvxDashListItem& rItem ) :
    SfxPoolItem( rItem ),
    maDashList( rItem.maDashList )
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
    return static_cast< const SvxDashListItem& >(rItem).maDashList == maDashList;
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
    sal_Int64 aValue = sal_Int64((sal_uLong)&maDashList );
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
        if(aValue)
            maDashList = *((XDashListSharedPtr*)(sal_uLong)aValue);
        return sal_True;
    }

    return sal_False;
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

SvxLineEndListItem::SvxLineEndListItem( XLineEndListSharedPtr aList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    maLineEndList( aList )
{
}

// -----------------------------------------------------------------------

SvxLineEndListItem::SvxLineEndListItem( const SvxLineEndListItem& rItem ) :
    SfxPoolItem( rItem ),
    maLineEndList( rItem.maLineEndList )
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
    return static_cast< const SvxLineEndListItem& >(rItem).maLineEndList == maLineEndList;
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
    sal_Int64 aValue = sal_Int64( (sal_uLong)&maLineEndList );
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
        if(aValue)
            maLineEndList = *((XLineEndListSharedPtr*)(sal_uLong)aValue);
        return sal_True;
    }

    return sal_False;
}

// eof
