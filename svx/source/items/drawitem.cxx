/*************************************************************************
 *
 *  $RCSfile: drawitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#pragma hdrstop

#include "svxids.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST

#include "xoutx.hxx"
#include "drawitem.hxx"

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
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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


//==================================================================
//
//  SvxDashListItem
//
//==================================================================

SvxDashListItem::SvxDashListItem()
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
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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


