/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawitem.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 17:09:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include "svxids.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST

#include "xoutx.hxx"
#include "drawitem.hxx"
#include "xtable.hxx"

using namespace ::com::sun::star;

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY( SvxColorTableItem, SfxPoolItem );
TYPEINIT1_AUTOFACTORY( SvxGradientListItem, SfxPoolItem );
TYPEINIT1_AUTOFACTORY( SvxHatchListItem, SfxPoolItem );
TYPEINIT1_AUTOFACTORY( SvxBitmapListItem, SfxPoolItem );
TYPEINIT1_AUTOFACTORY( SvxDashListItem, SfxPoolItem );
TYPEINIT1_AUTOFACTORY( SvxLineEndListItem, SfxPoolItem );

//==================================================================
//
//  SvxColorTableItem
//
//==================================================================

SvxColorTableItem::SvxColorTableItem()
{
}

// -----------------------------------------------------------------------

SvxColorTableItem::SvxColorTableItem( XColorTable* pTable, sal_uInt16 nW ) :
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

sal_Bool SvxColorTableItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = sal_Int64( (ULONG)pColorTable );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxColorTableItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pColorTable = (XColorTable *)(ULONG)aValue;
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

sal_Bool SvxGradientListItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = sal_Int64( (ULONG)pGradientList );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxGradientListItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pGradientList = (XGradientList *)(ULONG)aValue;
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

sal_Bool SvxHatchListItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = sal_Int64( (ULONG)pHatchList );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxHatchListItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pHatchList = (XHatchList *)(ULONG)aValue;
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

sal_Bool SvxBitmapListItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = sal_Int64( (ULONG)pBitmapList );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxBitmapListItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    // Please ask CD if you want to change this.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pBitmapList = (XBitmapList *)(ULONG)aValue;
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

sal_Bool SvxDashListItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    sal_Int64 aValue = sal_Int64( (ULONG)pDashList );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxDashListItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pDashList = (XDashList *)(ULONG)aValue;
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

sal_Bool SvxLineEndListItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ ) const
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    sal_Int64 aValue = sal_Int64( (ULONG)pLineEndList );
    rVal = uno::makeAny( aValue );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxLineEndListItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ )
{
    // This is only a quick helper to have UI support for these list items. Don't use
    // this method to query for a valid UNO representation.
    sal_Int64 aValue = 0;
    if ( rVal >>= aValue )
    {
        pLineEndList = (XLineEndList *)(ULONG)aValue;
        return sal_True;
    }

    return sal_False;
}
