/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "zoomitem.hxx"
namespace binfilter {

// -----------------------------------------------------------------------

/*N*/ TYPEINIT1_AUTOFACTORY(SvxZoomItem,SfxUInt16Item);

// -----------------------------------------------------------------------

/*N*/ SvxZoomItem::SvxZoomItem
/*N*/ (
/*N*/ 	SvxZoomType eZoomType,
/*N*/ 	sal_uInt16		nVal,
/*N*/ 	sal_uInt16		nWhich
/*N*/ )
/*N*/ :	SfxUInt16Item( nWhich, nVal ),
/*N*/ 	nValueSet( SVX_ZOOM_ENABLE_ALL ),
/*N*/ 	eType( eZoomType )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxZoomItem::SvxZoomItem( const SvxZoomItem& rOrig )
/*N*/ :	SfxUInt16Item( rOrig.Which(), rOrig.GetValue() ),
/*N*/ 	nValueSet( rOrig.GetValueSet() ),
/*N*/ 	eType( rOrig.GetType() )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxZoomItem::~SvxZoomItem()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxZoomItem::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SvxZoomItem( *this );
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ int SvxZoomItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/ 
/*N*/ 	SvxZoomItem& rItem = (SvxZoomItem&)rAttr;
/*N*/ 
/*N*/ 	return ( GetValue() == rItem.GetValue() 	&&
/*N*/ 			 nValueSet 	== rItem.GetValueSet() 	&&
/*N*/ 			 eType 		== rItem.GetType() 			);
/*N*/ }


}
