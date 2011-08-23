/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// include ---------------------------------------------------------------

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "svxitems.hrc"

#define ITEMID_HORJUSTIFY		0
#define ITEMID_VERJUSTIFY		0
#define ITEMID_ORIENTATION		0
#define ITEMID_LINEBREAK		0
#define ITEMID_MARGIN			0

#include <tools/stream.hxx>

#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/CellOrientation.hpp>

#include "algitem.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

// Konvertierung fuer UNO
#define TWIP_TO_MM100(TWIP) 	((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)	((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

// STATIC DATA -----------------------------------------------------------

/*N*/ TYPEINIT1_AUTOFACTORY( SvxHorJustifyItem, SfxEnumItem );
/*N*/ TYPEINIT1_AUTOFACTORY( SvxVerJustifyItem, SfxEnumItem );
/*N*/ TYPEINIT1_AUTOFACTORY( SvxOrientationItem, SfxEnumItem );
/*N*/ TYPEINIT1_AUTOFACTORY( SvxMarginItem, SfxPoolItem );

// class SvxHorJustifyItem -----------------------------------------------

/*N*/ SvxHorJustifyItem::SvxHorJustifyItem( const SvxCellHorJustify eJustify,
/*N*/ 									  const USHORT nId ) :
/*N*/ 	SfxEnumItem( nId, eJustify )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------


/*N*/ //------------------------------------------------------------------------

/*N*/ bool SvxHorJustifyItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch ( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_HORJUST_HORJUST:
/*N*/ 			{
/*N*/ 				table::CellHoriJustify eUno = table::CellHoriJustify_STANDARD;
/*N*/ 				switch ( (SvxCellHorJustify)GetValue() )
/*N*/ 				{
/*N*/ 					case SVX_HOR_JUSTIFY_STANDARD: eUno = table::CellHoriJustify_STANDARD; break;
/*N*/ 					case SVX_HOR_JUSTIFY_LEFT:	   eUno = table::CellHoriJustify_LEFT;	   break;
/*N*/ 					case SVX_HOR_JUSTIFY_CENTER:   eUno = table::CellHoriJustify_CENTER;   break;
/*N*/ 					case SVX_HOR_JUSTIFY_RIGHT:	   eUno = table::CellHoriJustify_RIGHT;	   break;
/*N*/ 					case SVX_HOR_JUSTIFY_BLOCK:	   eUno = table::CellHoriJustify_BLOCK;	   break;
/*?*/ 					case SVX_HOR_JUSTIFY_REPEAT:   eUno = table::CellHoriJustify_REPEAT;   break;
/*N*/ 				}
/*N*/ 				rVal <<= eUno;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		case MID_HORJUST_ADJUST:
/*N*/ 			{
/*N*/ 				//	ParagraphAdjust values, as in SvxAdjustItem
/*N*/ 				//	(same value for ParaAdjust and ParaLastLineAdjust)
/*N*/ 
/*?*/ 				sal_Int16 nAdjust = style::ParagraphAdjust_LEFT;
/*?*/ 				switch ( (SvxCellHorJustify)GetValue() )
/*?*/ 				{
/*?*/ 					// ParagraphAdjust_LEFT is used for STANDARD and REPEAT
/*?*/ 					case SVX_HOR_JUSTIFY_STANDARD:
/*?*/ 					case SVX_HOR_JUSTIFY_REPEAT:
/*?*/ 					case SVX_HOR_JUSTIFY_LEFT:	 nAdjust = style::ParagraphAdjust_LEFT;	  break;
/*?*/ 					case SVX_HOR_JUSTIFY_CENTER: nAdjust = style::ParagraphAdjust_CENTER; break;
/*?*/ 					case SVX_HOR_JUSTIFY_RIGHT:	 nAdjust = style::ParagraphAdjust_RIGHT;  break;
/*?*/ 					case SVX_HOR_JUSTIFY_BLOCK:	 nAdjust = style::ParagraphAdjust_BLOCK;  break;
/*?*/ 				}
/*?*/ 				rVal <<= nAdjust;		// as sal_Int16
/*N*/ 			}
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ bool SvxHorJustifyItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch ( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_HORJUST_HORJUST:
/*N*/ 			{
/*N*/ 				table::CellHoriJustify eUno;
/*N*/ 				if(!(rVal >>= eUno))
/*N*/ 				{
/*N*/ 					sal_Int32 nValue;
/*?*/ 					if(!(rVal >>= nValue))
/*?*/ 						return sal_False;
/*?*/ 					eUno = (table::CellHoriJustify)nValue;
/*N*/ 				}
/*N*/ 				SvxCellHorJustify eSvx = SVX_HOR_JUSTIFY_STANDARD;
/*N*/ 				switch (eUno)
/*N*/ 				{
/*N*/ 					case table::CellHoriJustify_STANDARD: eSvx = SVX_HOR_JUSTIFY_STANDARD; break;
/*N*/ 					case table::CellHoriJustify_LEFT:	  eSvx = SVX_HOR_JUSTIFY_LEFT;	   break;
/*N*/ 					case table::CellHoriJustify_CENTER:   eSvx = SVX_HOR_JUSTIFY_CENTER;   break;
/*N*/ 					case table::CellHoriJustify_RIGHT:	  eSvx = SVX_HOR_JUSTIFY_RIGHT;	   break;
/*N*/ 					case table::CellHoriJustify_BLOCK:	  eSvx = SVX_HOR_JUSTIFY_BLOCK;	   break;
/*?*/ 					case table::CellHoriJustify_REPEAT:   eSvx = SVX_HOR_JUSTIFY_REPEAT;   break;
/*N*/ 				}
/*N*/ 				SetValue( eSvx );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		case MID_HORJUST_ADJUST:
/*N*/ 			{
/*N*/ 				//	property contains ParagraphAdjust values as sal_Int16
/*N*/ 				sal_Int16 nVal;
/*?*/ 				if(!(rVal >>= nVal))
/*?*/ 					return sal_False;
/*?*/ 
/*?*/ 				SvxCellHorJustify eSvx = SVX_HOR_JUSTIFY_STANDARD;
/*?*/ 				switch (nVal)
/*?*/ 				{
/*?*/ 					//	STRETCH is treated as BLOCK
/*?*/ 					case style::ParagraphAdjust_LEFT:    eSvx = SVX_HOR_JUSTIFY_LEFT;   break;
/*?*/ 					case style::ParagraphAdjust_RIGHT:   eSvx = SVX_HOR_JUSTIFY_RIGHT;  break;
/*?*/ 					case style::ParagraphAdjust_STRETCH:
/*?*/ 					case style::ParagraphAdjust_BLOCK:   eSvx = SVX_HOR_JUSTIFY_BLOCK;  break;
/*?*/ 					case style::ParagraphAdjust_CENTER:  eSvx = SVX_HOR_JUSTIFY_CENTER; break;
/*?*/ 				}
/*?*/ 				SetValue( eSvx );
/*N*/ 			}
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxHorJustifyItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxHorJustifyItem( *this );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxHorJustifyItem::Create( SvStream& rStream, USHORT ) const
/*N*/ {
/*N*/ 	USHORT nVal;
/*N*/ 	rStream >> nVal;
/*N*/ 	return new SvxHorJustifyItem( (SvxCellHorJustify)nVal, Which() );
/*N*/ }
//------------------------------------------------------------------------

/*?*/ USHORT SvxHorJustifyItem::GetValueCount() const
/*?*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 return SVX_HOR_JUSTIFY_REPEAT + 1;	// letzter Enum-Wert + 1
/*?*/ }

// class SvxVerJustifyItem -----------------------------------------------

/*N*/ SvxVerJustifyItem::SvxVerJustifyItem( const SvxCellVerJustify eJustify,
/*N*/ 									  const USHORT nId ) :
/*N*/ 	SfxEnumItem( nId, eJustify )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ bool SvxVerJustifyItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	table::CellVertJustify eUno = table::CellVertJustify_STANDARD;
/*N*/ 	switch ( (SvxCellVerJustify)GetValue() )
/*N*/ 	{
/*N*/ 		case SVX_VER_JUSTIFY_STANDARD: eUno = table::CellVertJustify_STANDARD; break;
/*N*/ 		case SVX_VER_JUSTIFY_TOP:	   eUno = table::CellVertJustify_TOP;	    break;
/*N*/ 		case SVX_VER_JUSTIFY_CENTER:   eUno = table::CellVertJustify_CENTER;	break;
/*N*/ 		case SVX_VER_JUSTIFY_BOTTOM:   eUno = table::CellVertJustify_BOTTOM;	break;
/*N*/ 	}
/*N*/ 	rVal <<= eUno;
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ bool SvxVerJustifyItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	table::CellVertJustify eUno;
/*N*/ 	if(!(rVal >>= eUno))
/*N*/ 	{
/*?*/ 		sal_Int32 nValue;
/*?*/ 		if(!(rVal >>= nValue))
/*?*/ 			return sal_False;
/*?*/ 		eUno = (table::CellVertJustify)nValue;
/*N*/ 	}
/*N*/ 
/*N*/ 	SvxCellVerJustify eSvx = SVX_VER_JUSTIFY_STANDARD;
/*N*/ 	switch (eUno)
/*N*/ 	{
/*N*/ 		case table::CellVertJustify_STANDARD: eSvx = SVX_VER_JUSTIFY_STANDARD; break;
/*N*/ 		case table::CellVertJustify_TOP:	   eSvx = SVX_VER_JUSTIFY_TOP;	  	break;
/*N*/ 		case table::CellVertJustify_CENTER:   eSvx = SVX_VER_JUSTIFY_CENTER;	break;
/*N*/ 		case table::CellVertJustify_BOTTOM:   eSvx = SVX_VER_JUSTIFY_BOTTOM;	break;
/*N*/ 	}
/*N*/ 	SetValue( eSvx );
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxVerJustifyItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxVerJustifyItem( *this );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxVerJustifyItem::Create( SvStream& rStream, USHORT ) const
/*N*/ {
/*N*/ 	USHORT nVal;
/*N*/ 	rStream >> nVal;
/*N*/ 	return new SvxVerJustifyItem( (SvxCellVerJustify)nVal, Which() );
/*N*/ }

//------------------------------------------------------------------------

/*?*/ USHORT SvxVerJustifyItem::GetValueCount() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return SVX_VER_JUSTIFY_BOTTOM + 1;	// letzter Enum-Wert + 1
/*?*/ }

// class SvxOrientationItem ----------------------------------------------

/*N*/ SvxOrientationItem::SvxOrientationItem( const SvxCellOrientation eOrientation,
/*N*/ 										const USHORT nId):
/*N*/ 	SfxEnumItem( nId, eOrientation )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ bool SvxOrientationItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	table::CellOrientation eUno = table::CellOrientation_STANDARD;
/*N*/ 	switch ( (SvxCellOrientation)GetValue() )
/*N*/ 	{
/*N*/ 	case SVX_ORIENTATION_STANDARD:	eUno = table::CellOrientation_STANDARD;  break;
/*N*/ 	case SVX_ORIENTATION_TOPBOTTOM:	eUno = table::CellOrientation_TOPBOTTOM; break;
/*N*/ 	case SVX_ORIENTATION_BOTTOMTOP:	eUno = table::CellOrientation_BOTTOMTOP; break;
/*N*/ 	case SVX_ORIENTATION_STACKED:	eUno = table::CellOrientation_STACKED;	  break;
/*N*/ 	}
/*N*/ 	rVal <<= eUno;
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ bool SvxOrientationItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	table::CellOrientation eOrient;
/*N*/ 	if(!(rVal >>= eOrient))
/*N*/ 	{
/*?*/ 		sal_Int32 nValue;
/*?*/ 		if(!(rVal >>= nValue))
/*?*/ 			return sal_False;
/*?*/ 		eOrient = (table::CellOrientation)nValue;
/*N*/ 	}
/*N*/ 	SvxCellOrientation eSvx = SVX_ORIENTATION_STANDARD;
/*N*/ 	switch (eOrient)
/*N*/ 	{
/*N*/ 		case table::CellOrientation_STANDARD:	eSvx = SVX_ORIENTATION_STANDARD;  break;
/*?*/ 		case table::CellOrientation_TOPBOTTOM:	eSvx = SVX_ORIENTATION_TOPBOTTOM; break;
/*?*/ 		case table::CellOrientation_BOTTOMTOP:	eSvx = SVX_ORIENTATION_BOTTOMTOP; break;
/*N*/ 		case table::CellOrientation_STACKED:	eSvx = SVX_ORIENTATION_STACKED;	  break;
/*N*/ 	}
/*N*/ 	SetValue( eSvx );
/*N*/ 	return sal_True;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxOrientationItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxOrientationItem( *this );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxOrientationItem::Create( SvStream& rStream, USHORT ) const
/*N*/ {
/*N*/ 	USHORT nVal;
/*N*/ 	rStream >> nVal;
/*N*/ 	return new SvxOrientationItem( (SvxCellOrientation)nVal, Which() );
/*N*/ }

//------------------------------------------------------------------------

/*?*/ USHORT SvxOrientationItem::GetValueCount() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return SVX_ORIENTATION_STACKED + 1;	// letzter Enum-Wert + 1
/*?*/ }

// class SvxMarginItem ---------------------------------------------------

/*N*/ SvxMarginItem::SvxMarginItem( const USHORT nId ) :
/*N*/ 
/*N*/ 	SfxPoolItem( nId ),
/*N*/ 
/*N*/ 	nLeftMargin	 ( 20 ),
/*N*/ 	nTopMargin	 ( 20 ),
/*N*/ 	nRightMargin ( 20 ),
/*N*/ 	nBottomMargin( 20 )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SvxMarginItem::SvxMarginItem( sal_Int16 nLeft,
/*N*/ 							  sal_Int16 nTop,
/*N*/ 							  sal_Int16 nRight,
/*N*/ 							  sal_Int16 nBottom,
/*N*/ 							  const USHORT nId ) :
/*N*/ 	SfxPoolItem( nId ),
/*N*/ 
/*N*/ 	nLeftMargin	 ( nLeft ),
/*N*/ 	nTopMargin	 ( nTop ),
/*N*/ 	nRightMargin ( nRight ),
/*N*/ 	nBottomMargin( nBottom )
/*N*/ {
/*N*/ }


//------------------------------------------------------------------------

/*N*/ SvxMarginItem::SvxMarginItem( const SvxMarginItem& rItem ) :
/*N*/ 
/*N*/ 	SfxPoolItem( rItem.Which() )
/*N*/ {
/*N*/ 	nLeftMargin = rItem.nLeftMargin;
/*N*/ 	nTopMargin = rItem.nTopMargin;
/*N*/ 	nRightMargin = rItem.nRightMargin;
/*N*/ 	nBottomMargin = rItem.nBottomMargin;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ int SvxMarginItem::operator==( const SfxPoolItem& rItem ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
/*N*/ 
/*N*/ 	return ( ( nLeftMargin == ( (SvxMarginItem&)rItem ).nLeftMargin )	&&
/*N*/ 			 ( nTopMargin == ( (SvxMarginItem&)rItem ).nTopMargin )		&&
/*N*/ 			 ( nRightMargin == ( (SvxMarginItem&)rItem ).nRightMargin )	&&
/*N*/ 			 ( nBottomMargin == ( (SvxMarginItem&)rItem ).nBottomMargin ) );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxMarginItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxMarginItem(*this);
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxMarginItem::Create( SvStream& rStream, USHORT ) const
/*N*/ {
/*N*/ 	sal_Int16   nLeft;
/*N*/ 	sal_Int16   nTop;
/*N*/ 	sal_Int16   nRight;
/*N*/ 	sal_Int16   nBottom;
/*N*/ 	rStream >> nLeft;
/*N*/ 	rStream >> nTop;
/*N*/ 	rStream >> nRight;
/*N*/ 	rStream >> nBottom;
/*N*/ 	return new SvxMarginItem( nLeft, nTop, nRight, nBottom, Which() );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SvStream& SvxMarginItem::Store( SvStream &rStream, USHORT nItemVersion) const
/*N*/ {
/*N*/ 	rStream << nLeftMargin;
/*N*/ 	rStream << nTopMargin;
/*N*/ 	rStream << nRightMargin;
/*N*/ 	rStream << nBottomMargin;
/*N*/ 	return rStream;
/*N*/ }


//------------------------------------------------------------------------

/*N*/ bool SvxMarginItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/     switch ( nMemberId )
/*N*/ 	{
/*N*/ 		//	jetzt alles signed
/*N*/ 		case MID_MARGIN_L_MARGIN:
/*N*/ 			rVal <<= (sal_Int32)( bConvert ? TWIP_TO_MM100(nLeftMargin) : nLeftMargin );
/*N*/ 			break;
/*N*/ 		case MID_MARGIN_R_MARGIN:
/*N*/ 			rVal <<= (sal_Int32)( bConvert ? TWIP_TO_MM100(nRightMargin) : nRightMargin );
/*N*/ 			break;
/*N*/ 		case MID_MARGIN_UP_MARGIN:
/*N*/ 			rVal <<= (sal_Int32)( bConvert ? TWIP_TO_MM100(nTopMargin) : nTopMargin );
/*N*/ 			break;
/*N*/ 		case MID_MARGIN_LO_MARGIN:
/*N*/ 			rVal <<= (sal_Int32)( bConvert ? TWIP_TO_MM100(nBottomMargin) : nBottomMargin );
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR("unknown MemberId");
/*N*/ 			return sal_False;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ bool SvxMarginItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Bool bConvert = ( ( nMemberId & CONVERT_TWIPS ) != 0 );
/*N*/ 	long nMaxVal = bConvert ? TWIP_TO_MM100(SHRT_MAX) : SHRT_MAX;	// Members sind sal_Int16
/*N*/ 	sal_Int32 nVal;
/*N*/ 	if(!(rVal >>= nVal) || (nVal > nMaxVal))
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	switch ( nMemberId & ~CONVERT_TWIPS )
/*N*/ 	{
/*N*/ 		case MID_MARGIN_L_MARGIN:
/*N*/ 			nLeftMargin = (sal_Int16)( bConvert ? MM100_TO_TWIP(nVal) : nVal );
/*N*/ 			break;
/*N*/ 		case MID_MARGIN_R_MARGIN:
/*N*/ 			nRightMargin = (sal_Int16)( bConvert ? MM100_TO_TWIP(nVal) : nVal );
/*N*/ 			break;
/*N*/ 		case MID_MARGIN_UP_MARGIN:
/*N*/ 			nTopMargin = (sal_Int16)( bConvert ? MM100_TO_TWIP(nVal) : nVal );
/*N*/ 			break;
/*N*/ 		case MID_MARGIN_LO_MARGIN:
/*N*/ 			nBottomMargin = (sal_Int16)( bConvert ? MM100_TO_TWIP(nVal) : nVal );
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR("unknown MemberId");
/*N*/ 			return sal_False;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
