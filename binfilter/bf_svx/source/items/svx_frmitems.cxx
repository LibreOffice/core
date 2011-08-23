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

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/script/XTypeConverter.hpp>

#include <limits.h>
#include <comphelper/processfactory.hxx>

#define ITEMID_PAPERBIN	0
#define ITEMID_SIZE     0
#define ITEMID_LRSPACE  0
#define ITEMID_ULSPACE  0
#define ITEMID_PRINT    0
#define ITEMID_OPAQUE   0
#define ITEMID_PROTECT  0
#define ITEMID_SHADOW   0
#define ITEMID_BOX      0
#define ITEMID_BOXINFO  0
#define ITEMID_FMTBREAK 0
#define ITEMID_FMTKEEP  0
#define ITEMID_LINE     0
#define ITEMID_BRUSH    0
#define ITEMID_FRAMEDIR 0

#include <bf_goodies/graphicobject.hxx>
#include <tools/urlobj.hxx>
#ifndef SVX_LIGHT
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/objsh.hxx>
#endif // !SVX_LIGHT
#include "bf_basic/sbx.hxx"
#define GLOBALOVERFLOW3

#define _SVX_FRMITEMS_CXX

#include <bf_svtools/memberid.hrc>

#include <bf_svtools/wallitem.hxx>
#include <bf_svtools/cntwall.hxx>

#include "rtl/ustring.hxx"
#include <rtl/ustrbuf.hxx>

#include <impgrf.hxx>
#include "svxids.hrc"
#include "svxitems.hrc"
#include "dialogs.hrc"

#include "pbinitem.hxx"
#include "sizeitem.hxx"
#include "lrspitem.hxx"
#include "ulspitem.hxx"
#include "prntitem.hxx"
#include "opaqitem.hxx"
#include "protitem.hxx"
#include "shaditem.hxx"
#include "boxitem.hxx"
#include "brkitem.hxx"
#include "keepitem.hxx"
#include "bolnitem.hxx"
#include "brshitem.hxx"
#include "frmdiritem.hxx"

#include "itemtype.hxx"
#include "dialmgr.hxx"
#include "svxerr.hxx"
#include "unoprnms.hxx"

#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/table/ShadowLocation.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/util/SortField.hpp>
#include <com/sun/star/util/SortFieldType.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Selection.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <comphelper/types.hxx>

#include <unomid.hxx>
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002

#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {
using namespace ::rtl;
using namespace ::com::sun::star;


// Konvertierung fuer UNO
#define TWIP_TO_MM100(TWIP) 	((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)	((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

// STATIC DATA -----------------------------------------------------------


/*?*/ inline void SetValueProp( XubString& rStr, const sal_uInt16 nValue,
/*?*/ 						  const sal_uInt16 nProp )
/*?*/ {
/*?*/ 	if( 100 == nProp )
/*?*/ 		rStr += String::CreateFromInt32( nValue );
/*?*/ 	else
/*?*/ 		( rStr += String::CreateFromInt32( nProp )) += sal_Unicode('%');
/*?*/ }

/*?*/ inline void SetValueProp( XubString& rStr, const short nValue,
/*?*/ 						  const sal_uInt16 nProp )
/*?*/ {
/*?*/ 	if( 100 == nProp )
/*?*/ 		rStr += String::CreateFromInt32( nValue );
/*?*/ 	else
/*?*/ 		( rStr += String::CreateFromInt32( nProp )) += sal_Unicode('%');
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ TYPEINIT1_AUTOFACTORY(SvxPaperBinItem, SfxByteItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxSizeItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxLRSpaceItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxULSpaceItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxPrintItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxOpaqueItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxProtectItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxBrushItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxShadowItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxBoxItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxBoxInfoItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxFmtBreakItem, SfxEnumItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxFmtKeepItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxLineItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxFrameDirectionItem, SfxUInt16Item);


// class SvxPaperBinItem ------------------------------------------------

/*N*/ SfxPoolItem* SvxPaperBinItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxPaperBinItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxPaperBinItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxPaperBinItem::Create( SvStream& rStrm, sal_uInt16 ) const
/*N*/ {
/*N*/ 	sal_Int8 nBin;
/*N*/ 	rStrm >> nBin;
/*N*/ 	return new SvxPaperBinItem( Which(), nBin );
/*N*/ }

// -----------------------------------------------------------------------


// class SvxSizeItem -----------------------------------------------------

/*N*/ SvxSizeItem::SvxSizeItem( const sal_uInt16 nId, const Size& rSize ) :
/*N*/
/*N*/ 	SfxPoolItem( nId ),
/*N*/
/*N*/ 	aSize( rSize )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ bool SvxSizeItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/
/*N*/ 	awt::Size aTmp(aSize.Width(), aSize.Height());
/*N*/     if( bConvert )
/*N*/ 	{
/*N*/ 		aTmp.Height = TWIP_TO_MM100(aTmp.Height);
/*N*/ 		aTmp.Width = TWIP_TO_MM100(aTmp.Width);
/*N*/ 	}
/*N*/
/*N*/     switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_SIZE_SIZE:  rVal <<= aTmp;	break;
/*N*/ 		case MID_SIZE_WIDTH: rVal <<= aTmp.Width; break;
/*N*/ 		case MID_SIZE_HEIGHT: rVal <<= aTmp.Height;  break;
/*N*/         default: DBG_ERROR("Wrong MemberId!"); return sal_False;
/*N*/ 	}
/*N*/
/*N*/ 	return sal_True;
/*N*/ }
// -----------------------------------------------------------------------
/*N*/ bool SvxSizeItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/
/*N*/     switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_SIZE_SIZE:
/*N*/ 		{
/*?*/ 			awt::Size aTmp;
/*?*/ 			if( rVal >>= aTmp )
/*?*/ 			{
/*?*/ 				if(bConvert)
/*?*/ 				{
/*?*/ 					aTmp.Height = MM100_TO_TWIP(aTmp.Height);
/*?*/ 					aTmp.Width = MM100_TO_TWIP(aTmp.Width);
/*?*/ 				}
/*?*/ 				aSize = Size( aTmp.Width, aTmp.Height );
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				return sal_False;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_SIZE_WIDTH:
/*N*/ 		{
/*N*/ 			sal_Int32 nVal;
/*N*/ 			if(!(rVal >>= nVal ))
/*N*/ 				return sal_False;
/*N*/
/*N*/ 			aSize.Width() = bConvert ? MM100_TO_TWIP(nVal) : nVal;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_SIZE_HEIGHT:
/*N*/ 		{
/*N*/ 			sal_Int32 nVal;
/*N*/ 			if(!(rVal >>= nVal))
/*N*/ 				return sal_True;
/*N*/
/*N*/ 			aSize.Height() = bConvert ? MM100_TO_TWIP(nVal) : nVal;
/*N*/ 		}
/*N*/ 		break;
/*N*/         default: DBG_ERROR("Wrong MemberId!");
/*N*/ 			return sal_False;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxSizeItem::SvxSizeItem( const sal_uInt16 nId ) :
/*N*/
/*N*/ 	SfxPoolItem( nId )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxSizeItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/
/*N*/ 	return ( aSize == ( (SvxSizeItem&)rAttr ).GetSize() );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxSizeItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxSizeItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxSizeItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << aSize.Width();
/*N*/ 	rStrm << aSize.Height();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------


/*N*/ SfxPoolItem* SvxSizeItem::Create( SvStream& rStrm, sal_uInt16 ) const
/*N*/ {
/*N*/ 	long nWidth, nHeight;
/*N*/ 	rStrm >> nWidth >> nHeight;
/*N*/
/*N*/ 	SvxSizeItem* pAttr = new SvxSizeItem( Which() );
/*N*/ 	pAttr->SetSize(Size(nWidth, nHeight));
/*N*/
/*N*/ 	return pAttr;
/*N*/ }

// class SvxLRSpaceItem --------------------------------------------------

/*N*/ SvxLRSpaceItem::SvxLRSpaceItem( const sal_uInt16 nId ) :
/*N*/
/*N*/ 	SfxPoolItem( nId ),
/*N*/
/*N*/ 	nFirstLineOfst	( 0 ), nPropFirstLineOfst( 100 ),
/*N*/ 	nTxtLeft		( 0 ),
/*N*/ 	nLeftMargin		( 0 ), nPropLeftMargin( 100 ),
/*N*/ 	nRightMargin	( 0 ), nPropRightMargin( 100 ),
/*N*/ 	bBulletFI		( 0 ),
/*N*/ 	bAutoFirst      ( 0 )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxLRSpaceItem::SvxLRSpaceItem( const long nLeft, const long nRight,
/*N*/ 								const long nTLeft, const short nOfset,
/*N*/ 								const sal_uInt16 nId ) :
/*N*/
/*N*/ 	SfxPoolItem( nId ),
/*N*/
/*N*/ 	nFirstLineOfst	( nOfset ), nPropFirstLineOfst( 100 ),
/*N*/ 	nTxtLeft		( nTLeft ),
/*N*/ 	nLeftMargin		( nLeft ), nPropLeftMargin( 100 ),
/*N*/ 	nRightMargin	( nRight ), nPropRightMargin( 100 ),
/*N*/ 	bBulletFI		( 0 ),
/*N*/ 	bAutoFirst      ( 0 )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ bool SvxLRSpaceItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	sal_Bool bRet = sal_True;
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		//	jetzt alles signed
/*N*/ 		case MID_L_MARGIN:
/*N*/ 			rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nLeftMargin) : nLeftMargin);
/*N*/ 			break;
/*N*/
/*N*/ 		case MID_TXT_LMARGIN :
/*N*/ 			rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nTxtLeft) : nTxtLeft);
/*N*/ 		break;
/*N*/ 		case MID_R_MARGIN:
/*N*/ 			rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nRightMargin) : nRightMargin);
/*N*/ 			break;
/*N*/ 		case MID_L_REL_MARGIN:
/*N*/ 			rVal <<= (sal_Int16)nPropLeftMargin;
/*N*/ 		break;
/*N*/ 		case MID_R_REL_MARGIN:
/*N*/ 			rVal <<= (sal_Int16)nPropRightMargin;
/*N*/ 		break;
/*N*/
/*N*/ 		case MID_FIRST_LINE_INDENT:
/*N*/ 			rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nFirstLineOfst) : nFirstLineOfst);
/*N*/ 			break;
/*N*/
/*N*/ 		case MID_FIRST_LINE_REL_INDENT:
/*N*/ 			rVal <<= (sal_Int16)(nPropFirstLineOfst);
/*N*/ 			break;
/*N*/
/*N*/ 		case MID_FIRST_AUTO:
/*N*/ 			rVal = Bool2Any(IsAutoFirst());
/*N*/ 			break;
/*N*/
/*N*/ 		default:
/*N*/ 			bRet = sal_False;
/*N*/ 			DBG_ERROR("unknown MemberId");
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ bool SvxLRSpaceItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	sal_Int32 nMaxVal = bConvert ? TWIP_TO_MM100(USHRT_MAX) : USHRT_MAX;
/*N*/ 	sal_Int32 nVal;
/*N*/ 	if( nMemberId != MID_FIRST_AUTO &&
/*N*/ 			nMemberId != MID_L_REL_MARGIN && nMemberId != MID_R_REL_MARGIN)
/*N*/ 		if(!(rVal >>= nVal))
/*N*/ 			return sal_False;
/*N*/
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_L_MARGIN:
/*N*/ 			SetLeft((sal_Int32)bConvert ? MM100_TO_TWIP(nVal) : nVal);
/*N*/ 			break;
/*N*/
/*N*/ 		case MID_TXT_LMARGIN :
/*N*/ 			SetTxtLeft((sal_Int32)bConvert ? MM100_TO_TWIP(nVal) : nVal);
/*N*/ 		break;
/*N*/
/*N*/ 		case MID_R_MARGIN:
/*N*/ 			SetRight((sal_Int32)	bConvert ? MM100_TO_TWIP(nVal) : nVal);
/*N*/ 			break;
/*N*/ 		case MID_L_REL_MARGIN:
/*N*/ 		case MID_R_REL_MARGIN:
/*N*/ 		{
/*N*/ 			sal_Int32 nRel;
/*N*/             if((rVal >>= nRel) && nRel >= 0 && nRel < USHRT_MAX)
/*N*/ 			{
/*N*/ 				if(MID_L_REL_MARGIN== nMemberId)
/*N*/                     nPropLeftMargin = (USHORT)nRel;
/*N*/ 				else
/*N*/                     nPropRightMargin = (USHORT)nRel;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				return FALSE;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_FIRST_LINE_INDENT	   :
/*N*/             SetTxtFirstLineOfst((short)(bConvert ?  MM100_TO_TWIP(nVal) : nVal));
/*N*/ 			break;
/*N*/
/*N*/ 		case MID_FIRST_LINE_REL_INDENT:
/*?*/ 			SetPropTxtFirstLineOfst	( nVal );
/*N*/ 			break;
/*N*/
/*N*/ 		case MID_FIRST_AUTO:
/*N*/ 			SetAutoFirst( Any2Bool(rVal) );
/*N*/ 			break;
/*N*/
/*N*/ 		default:
/*N*/ 			DBG_ERROR("unknown MemberId");
/*N*/ 			return sal_False;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

// nLeftMargin und nTxtLeft anpassen.

/*N*/ void SvxLRSpaceItem::AdjustLeft()
/*N*/ {
/*N*/ 	if ( 0 > nFirstLineOfst )
/*N*/ 		nLeftMargin = nTxtLeft + nFirstLineOfst;
/*N*/ 	else
/*N*/ 		nLeftMargin = nTxtLeft;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxLRSpaceItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/
/*N*/ 	return (
/*N*/ 		nLeftMargin == ((SvxLRSpaceItem&)rAttr).GetLeft()  &&
/*N*/ 		nRightMargin == ((SvxLRSpaceItem&)rAttr).GetRight() &&
/*N*/ 		nFirstLineOfst == ((SvxLRSpaceItem&)rAttr).GetTxtFirstLineOfst() &&
/*N*/ 		nPropLeftMargin == ((SvxLRSpaceItem&)rAttr).GetPropLeft()  &&
/*N*/ 		nPropRightMargin == ((SvxLRSpaceItem&)rAttr).GetPropRight() &&
/*N*/ 		nPropFirstLineOfst == ((SvxLRSpaceItem&)rAttr).GetPropTxtFirstLineOfst() &&
/*N*/ 		bBulletFI == ((SvxLRSpaceItem&)rAttr).IsBulletFI() &&
/*N*/ 		bAutoFirst == ((SvxLRSpaceItem&)rAttr).IsAutoFirst() );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxLRSpaceItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxLRSpaceItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

// MT: BulletFI: Vor 501 wurde im Outliner das Bullet nicht auf der Position des
// FI positioniert, deshalb muss in aelteren Dokumenten der FI auf 0 stehen.

#define BULLETLR_MARKER	0x599401FE

/*N*/ SvStream& SvxLRSpaceItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	short nSaveFI = nFirstLineOfst;
/*N*/ 	if ( IsBulletFI() )
/*N*/ 		((SvxLRSpaceItem*)this)->SetTxtFirstLineOfst( 0 );	// nLeftMargin wird mitmanipuliert, siehe Create()
/*N*/
/*N*/ 	sal_uInt16 nMargin = 0;
/*N*/ 	if( nLeftMargin > 0 )
/*N*/ 		nMargin = sal_uInt16( nLeftMargin );
/*N*/ 	rStrm << nMargin;
/*N*/ 	rStrm << nPropLeftMargin;
/*N*/ 	if( nRightMargin > 0 )
/*N*/ 		nMargin = sal_uInt16( nRightMargin );
/*N*/ 	else
/*N*/ 		nMargin = 0;
/*N*/ 	rStrm << nMargin;
/*N*/ 	rStrm << nPropRightMargin;
/*N*/ 	rStrm << nFirstLineOfst;
/*N*/ 	rStrm << nPropFirstLineOfst;
/*N*/ 	if( nTxtLeft > 0 )
/*N*/ 		nMargin = sal_uInt16( nTxtLeft );
/*N*/ 	else
/*N*/ 		nMargin = 0;
/*N*/ 	rStrm << nMargin;
/*N*/ 	if( nItemVersion >= LRSPACE_AUTOFIRST_VERSION )
/*N*/ 	{
/*N*/ 		sal_Int8 nAutoFirst = bAutoFirst ? 1 : 0;
/*N*/ 		if( nItemVersion >= LRSPACE_NEGATIVE_VERSION &&
/*N*/ 			( nLeftMargin < 0 || nRightMargin < 0 || nTxtLeft < 0 ) )
/*N*/ 			nAutoFirst |= 0x80;
/*N*/ 		rStrm << nAutoFirst;
/*N*/
/*N*/ 		if ( IsBulletFI() )
/*N*/ 		{
/*N*/ 			// Ab 6.0 keine Magicnumber schreiben...
/*N*/ 			DBG_ASSERT( rStrm.GetVersion() <= SOFFICE_FILEFORMAT_50, "MT: Fileformat SvxLRSpaceItem aendern!" );
/*N*/ 			rStrm << (sal_uInt32) BULLETLR_MARKER;
/*N*/ 			rStrm << nSaveFI;
/*N*/ 		}
/*N*/ 		if( 0x80 & nAutoFirst )
/*N*/ 		{
/*N*/ 			rStrm << nLeftMargin;
/*N*/ 			rStrm << nRightMargin;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if ( IsBulletFI() )
/*N*/ 		((SvxLRSpaceItem*)this)->SetTxtFirstLineOfst( nSaveFI );
/*N*/
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxLRSpaceItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
/*N*/ {
/*N*/ 	sal_uInt16 left, prpleft, right, prpright, prpfirstline, txtleft;
/*N*/ 	short firstline;
/*N*/ 	sal_Int8 autofirst = 0;
/*N*/
/*N*/ 	if ( nVersion >= LRSPACE_AUTOFIRST_VERSION )
/*N*/ 	{
/*N*/ 		rStrm >> left >> prpleft >> right >> prpright >> firstline >>
/*N*/ 				 prpfirstline >> txtleft >> autofirst;
/*N*/
/*N*/ 		sal_uInt32 nPos = rStrm.Tell();
/*N*/ 		sal_uInt32 nMarker;
/*N*/ 		rStrm >> nMarker;
/*N*/ 		if ( nMarker == BULLETLR_MARKER )
/*N*/ 		{
/*N*/ 			rStrm >> firstline;
/*N*/ 			if ( firstline < 0 )
/*N*/ 				left += firstline;	// s.u.: txtleft = ...
/*N*/ 		}
/*N*/ 		else
/*N*/ 			rStrm.Seek( nPos );
/*N*/ 	}
/*N*/ 	else if ( nVersion == LRSPACE_TXTLEFT_VERSION )
/*N*/ 	{
/*N*/ 		rStrm >> left >> prpleft >> right >> prpright >> firstline >>
/*N*/ 				 prpfirstline >> txtleft;
/*N*/ 	}
/*N*/ 	else if ( nVersion == LRSPACE_16_VERSION )
/*N*/ 	{
/*N*/ 		rStrm >> left >> prpleft >> right >> prpright >> firstline >>
/*N*/ 				 prpfirstline;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		sal_Int8 nL, nR, nFL;
/*?*/ 		rStrm >> left >> nL >> right >> nR >> firstline >> nFL;
/*?*/ 		prpleft = (sal_uInt16)nL;
/*?*/ 		prpright = (sal_uInt16)nR;
/*?*/ 		prpfirstline = (sal_uInt16)nFL;
/*N*/ 	}
/*N*/
/*N*/ 	txtleft = firstline >= 0 ? left : left - firstline;
/*N*/ 	SvxLRSpaceItem* pAttr = new SvxLRSpaceItem( Which() );
/*N*/
/*N*/ 	pAttr->nLeftMargin = left;
/*N*/ 	pAttr->nPropLeftMargin = prpleft;
/*N*/ 	pAttr->nRightMargin = right;
/*N*/ 	pAttr->nPropRightMargin = prpright;
/*N*/ 	pAttr->nFirstLineOfst = firstline;
/*N*/ 	pAttr->nPropFirstLineOfst = prpfirstline;
/*N*/ 	pAttr->nTxtLeft = txtleft;
/*N*/ 	pAttr->bAutoFirst = autofirst & 0x01;
/*N*/ 	if( nVersion >= LRSPACE_NEGATIVE_VERSION && ( autofirst & 0x80 ) )
/*N*/ 	{
/*N*/ 		sal_Int32 nMargin;
/*N*/ 		rStrm >> nMargin;
/*N*/ 		pAttr->nLeftMargin = nMargin;
/*N*/ 		pAttr->nTxtLeft = firstline >= 0 ? nMargin : nMargin - firstline;
/*N*/ 		rStrm >> nMargin;
/*N*/ 		pAttr->nRightMargin = nMargin;
/*N*/ 	}
/*N*/ 	return pAttr;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_uInt16 SvxLRSpaceItem::GetVersion( sal_uInt16 nFileVersion ) const
/*N*/ {
/*N*/ 	return (nFileVersion == SOFFICE_FILEFORMAT_31)
/*N*/ 			   ? LRSPACE_TXTLEFT_VERSION
/*N*/ 			   : LRSPACE_NEGATIVE_VERSION;
/*N*/ }

// -----------------------------------------------------------------------


// class SvxULSpaceItem --------------------------------------------------

/*N*/ SvxULSpaceItem::SvxULSpaceItem( const sal_uInt16 nId ) :
/*N*/
/*N*/ 	SfxPoolItem( nId ),
/*N*/
/*N*/ 	nUpper( 0 ), nPropUpper( 100 ),
/*N*/ 	nLower( 0 ), nPropLower( 100 )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxULSpaceItem::SvxULSpaceItem( const sal_uInt16 nUp, const sal_uInt16 nLow,
/*N*/ 								const sal_uInt16 nId ) :
/*N*/
/*N*/ 	SfxPoolItem( nId ),
/*N*/
/*N*/ 	nUpper( nUp  ), nPropUpper( 100 ),
/*N*/ 	nLower( nLow ), nPropLower( 100 )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ bool    SvxULSpaceItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		//	jetzt alles signed
/*N*/ 		case  MID_UP_MARGIN:	rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nUpper) : nUpper); break;
/*N*/ 		case  MID_LO_MARGIN:	rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nLower) : nLower); break;
/*N*/ 		case MID_UP_REL_MARGIN: rVal <<= (sal_Int16) nPropUpper; break;
/*N*/ 		case MID_LO_REL_MARGIN: rVal <<= (sal_Int16) nPropLower; break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ bool SvxULSpaceItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	sal_Int32 nVal;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_UP_MARGIN :
/*N*/ 			if(!(rVal >>= nVal) || nVal < 0)
/*N*/ 				return sal_False;
/*N*/ 			SetUpper((sal_uInt16)bConvert ? MM100_TO_TWIP(nVal) : nVal);
/*N*/ 			break;
/*N*/ 		case MID_LO_MARGIN :
/*N*/ 			if(!(rVal >>= nVal) || nVal < 0)
/*N*/ 				return sal_False;
/*N*/ 			SetLower((sal_uInt16)bConvert ? MM100_TO_TWIP(nVal) : nVal);
/*N*/ 			break;
/*N*/ 		case MID_UP_REL_MARGIN:
/*N*/ 		case MID_LO_REL_MARGIN:
/*N*/ 		{
/*N*/ 			sal_Int32 nRel;
/*?*/ 			if((rVal >>= nRel) && nRel > 1 )
/*?*/ 			{
/*?*/ 				if(MID_UP_REL_MARGIN == nMemberId)
/*?*/ 					nPropUpper = nRel;
/*?*/ 				else
/*?*/ 					nPropLower = nRel;
/*?*/ 			}
/*?*/ 			else
/*?*/ 				return FALSE;
/*?*/ 		}
/*?*/ 		break;
/*N*/
/*N*/
/*N*/ 		default:
/*N*/ 			DBG_ERROR("unknown MemberId");
/*N*/ 			return sal_False;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxULSpaceItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/
/*N*/ 	return ( nUpper == ( (SvxULSpaceItem&)rAttr ).nUpper &&
/*N*/ 			 nLower == ( (SvxULSpaceItem&)rAttr ).nLower &&
/*N*/ 			 nPropUpper == ( (SvxULSpaceItem&)rAttr ).nPropUpper &&
/*N*/ 			 nPropLower == ( (SvxULSpaceItem&)rAttr ).nPropLower );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxULSpaceItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxULSpaceItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxULSpaceItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << GetUpper()
/*N*/ 		  << GetPropUpper()
/*N*/ 		  << GetLower()
/*N*/ 		  << GetPropLower();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxULSpaceItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
/*N*/ {
/*N*/ 	sal_uInt16 upper, lower, nPL = 0, nPU = 0;
/*N*/
/*N*/ 	if ( nVersion == ULSPACE_16_VERSION )
/*N*/ 		rStrm >> upper >> nPU >> lower >> nPL;
/*N*/ 	else
/*N*/ 	{
/*?*/ 		sal_Int8 nU, nL;
/*?*/ 		rStrm >> upper >> nU >> lower >> nL;
/*?*/ 		nPL = (sal_uInt16)nL;
/*?*/ 		nPU = (sal_uInt16)nU;
/*N*/ 	}
/*N*/
/*N*/ 	SvxULSpaceItem* pAttr = new SvxULSpaceItem( Which() );
/*N*/ 	pAttr->SetUpperValue( upper );
/*N*/ 	pAttr->SetLowerValue( lower );
/*N*/ 	pAttr->SetPropUpper( nPU );
/*N*/ 	pAttr->SetPropLower( nPL );
/*N*/ 	return pAttr;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_uInt16 SvxULSpaceItem::GetVersion( sal_uInt16 nFileVersion ) const
/*N*/ {
/*N*/ 	return ULSPACE_16_VERSION;
/*N*/ }

// -----------------------------------------------------------------------

// class SvxPrintItem ----------------------------------------------------

/*N*/ SfxPoolItem* SvxPrintItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxPrintItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxPrintItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_Int8)GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxPrintItem::Create( SvStream& rStrm, sal_uInt16 ) const
/*N*/ {
/*N*/ 	sal_Int8 bIsPrint;
/*N*/ 	rStrm >> bIsPrint;
/*N*/ 	return new SvxPrintItem( Which(), sal_Bool( bIsPrint != 0 ) );
/*N*/ }

//------------------------------------------------------------------------


// class SvxOpaqueItem ---------------------------------------------------

/*N*/ SfxPoolItem* SvxOpaqueItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxOpaqueItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxOpaqueItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_Int8)GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxOpaqueItem::Create( SvStream& rStrm, sal_uInt16 ) const
/*N*/ {
/*N*/ 	sal_Int8 bIsOpaque;
/*N*/ 	rStrm >> bIsOpaque;
/*N*/ 	return new SvxOpaqueItem( Which(), sal_Bool( bIsOpaque != 0 ) );
/*N*/ }

//------------------------------------------------------------------------


// class SvxProtectItem --------------------------------------------------

/*N*/ int SvxProtectItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/
/*N*/ 	return ( bCntnt == ( (SvxProtectItem&)rAttr ).bCntnt &&
/*N*/ 			 bSize  == ( (SvxProtectItem&)rAttr ).bSize  &&
/*N*/ 			 bPos   == ( (SvxProtectItem&)rAttr ).bPos );
/*N*/ }

/*-----------------16.03.98 12:42-------------------
--------------------------------------------------*/
/*N*/ bool SvxProtectItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	sal_Bool bValue;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case MID_PROTECT_CONTENT :	bValue = bCntnt; break;
/*N*/ 		case MID_PROTECT_SIZE    :  bValue = bSize; break;
/*N*/ 		case MID_PROTECT_POSITION:  bValue = bPos; break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR("falsche MemberId");
/*N*/ 			return sal_False;
/*N*/ 	}
/*N*/
/*N*/ 	rVal = Bool2Any( bValue );
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------16.03.98 12:42-------------------

--------------------------------------------------*/
/*N*/ bool	SvxProtectItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	sal_Bool bVal( Any2Bool(rVal) );
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case MID_PROTECT_CONTENT :	bCntnt = bVal;	break;
/*N*/ 		case MID_PROTECT_SIZE    :  bSize  = bVal;	break;
/*N*/ 		case MID_PROTECT_POSITION:  bPos   = bVal;	break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR("falsche MemberId");
/*N*/ 			return sal_False;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxProtectItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxProtectItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxProtectItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	sal_Int8 cProt = 0;
/*N*/ 	if( IsPosProtected() )   cProt |= 0x01;
/*N*/ 	if( IsSizeProtected() )  cProt |= 0x02;
/*N*/ 	if( IsCntntProtected() ) cProt |= 0x04;
/*N*/ 	rStrm << (sal_Int8) cProt;
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxProtectItem::Create( SvStream& rStrm, sal_uInt16 ) const
/*N*/ {
/*N*/ 	sal_Int8 cFlags;
/*N*/ 	rStrm >> cFlags;
/*N*/ 	SvxProtectItem* pAttr = new SvxProtectItem( Which() );
/*N*/ 	pAttr->SetPosProtect( sal_Bool( ( cFlags & 0x01 ) != 0 ) );
/*N*/ 	pAttr->SetSizeProtect( sal_Bool(  ( cFlags & 0x02 ) != 0 ) );
/*N*/ 	pAttr->SetCntntProtect( sal_Bool(  ( cFlags & 0x04 ) != 0 ) );
/*N*/ 	return pAttr;
/*N*/ }

// class SvxShadowItem ---------------------------------------------------

/*N*/ SvxShadowItem::SvxShadowItem( const USHORT nId,
/*N*/ 				 const Color *pColor, const USHORT nW,
/*N*/ 				 const SvxShadowLocation eLoc ) :
/*N*/ 	SfxEnumItemInterface( nId ),
/*N*/ 	aShadowColor(COL_GRAY),
/*N*/ 	nWidth		( nW ),
/*N*/ 	eLocation	( eLoc )
/*N*/ {
/*N*/ 	if ( pColor )
/*N*/ 		aShadowColor = *pColor;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ bool SvxShadowItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/
/*N*/ 	table::ShadowFormat aShadow;
/*N*/ 	table::ShadowLocation eSet = table::ShadowLocation_NONE;
/*N*/ 	switch( eLocation )
/*N*/ 	{
/*N*/ 		case SVX_SHADOW_TOPLEFT    : eSet = table::ShadowLocation_TOP_LEFT    ; break;
/*N*/ 		case SVX_SHADOW_TOPRIGHT   : eSet = table::ShadowLocation_TOP_RIGHT   ; break;
/*N*/ 		case SVX_SHADOW_BOTTOMLEFT : eSet = table::ShadowLocation_BOTTOM_LEFT ; break;
/*N*/ 		case SVX_SHADOW_BOTTOMRIGHT: eSet = table::ShadowLocation_BOTTOM_RIGHT; break;
/*N*/ 	}
/*N*/ 	aShadow.Location = eSet;
/*N*/     aShadow.ShadowWidth =   bConvert ? TWIP_TO_MM100(nWidth) : nWidth;
/*N*/ 	aShadow.IsTransparent = aShadowColor.GetTransparency() > 0;
/*N*/ 	aShadow.Color = aShadowColor.GetRGBColor();
/*N*/
/*N*/     switch ( nMemberId )
/*N*/     {
/*?*/         case MID_LOCATION: rVal <<= aShadow.Location; break;
/*?*/         case MID_WIDTH: rVal <<= aShadow.ShadowWidth; break;
/*?*/         case MID_TRANSPARENT: rVal <<= aShadow.IsTransparent; break;
/*?*/         case MID_BG_COLOR: rVal <<= aShadow.Color; break;
/*N*/         case 0: rVal <<= aShadow; break;
/*N*/         default: DBG_ERROR("Wrong MemberId!"); return sal_False;
/*N*/     }
/*N*/
/*N*/ 	return sal_True;
/*N*/ }
// -----------------------------------------------------------------------
/*N*/ bool SvxShadowItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/
/*N*/     table::ShadowFormat aShadow;
/*N*/     uno::Any aAny;
/*N*/     bool bRet = QueryValue( aAny, bConvert ? CONVERT_TWIPS : 0 ) && ( aAny >>= aShadow );
/*N*/     switch ( nMemberId )
/*N*/     {
/*N*/         case MID_LOCATION:
/*N*/ 		{
/*?*/ 			bRet = (rVal >>= aShadow.Location);
/*?*/ 			if ( !bRet )
/*?*/ 			{
/*?*/ 				sal_Int16 nVal;
/*?*/ 				bRet = (rVal >>= nVal);
/*?*/ 				aShadow.Location = (table::ShadowLocation) nVal;
/*?*/ 			}
/*?*/
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/         case MID_WIDTH: rVal >>= aShadow.ShadowWidth; break;
/*?*/         case MID_TRANSPARENT: rVal >>= aShadow.IsTransparent; break;
/*?*/         case MID_BG_COLOR: rVal >>= aShadow.Color; break;
/*N*/         case 0: rVal >>= aShadow; break;
/*N*/         default: DBG_ERROR("Wrong MemberId!"); return sal_False;
/*N*/     }
/*N*/
/*N*/     if ( bRet )
/*N*/     {
/*N*/ 		SvxShadowLocation eSet = SVX_SHADOW_NONE;
/*N*/ 		switch( aShadow.Location )
/*N*/ 		{
/*N*/ 			case table::ShadowLocation_TOP_LEFT    : eLocation = SVX_SHADOW_TOPLEFT; break;
/*N*/ 			case table::ShadowLocation_TOP_RIGHT   : eLocation = SVX_SHADOW_TOPRIGHT; break;
/*N*/ 			case table::ShadowLocation_BOTTOM_LEFT : eLocation = SVX_SHADOW_BOTTOMLEFT ; break;
/*N*/ 			case table::ShadowLocation_BOTTOM_RIGHT: eLocation = SVX_SHADOW_BOTTOMRIGHT; break;
/*N*/ 		}
/*N*/
/*N*/         nWidth = bConvert ? MM100_TO_TWIP(aShadow.ShadowWidth) : aShadow.ShadowWidth;
/*N*/ 		Color aSet(aShadow.Color);
/*N*/ 		aSet.SetTransparency(aShadow.IsTransparent ? 0xff : 0);
/*N*/ 		aShadowColor = aSet;
/*N*/ 	}
/*N*/
/*N*/     return bRet;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxShadowItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/
/*N*/ 	return ( ( aShadowColor == ( (SvxShadowItem&)rAttr ).aShadowColor ) &&
/*N*/ 			 ( nWidth    == ( (SvxShadowItem&)rAttr ).GetWidth() ) &&
/*N*/ 			 ( eLocation == ( (SvxShadowItem&)rAttr ).GetLocation() ) );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxShadowItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxShadowItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_uInt16 SvxShadowItem::CalcShadowSpace( sal_uInt16 nShadow ) const
/*N*/ {
/*N*/ 	sal_uInt16 nSpace = 0;
/*N*/
/*N*/ 	switch ( nShadow )
/*N*/ 	{
/*N*/ 		case SHADOW_TOP:
/*N*/ 			if ( eLocation == SVX_SHADOW_TOPLEFT ||
/*N*/ 				 eLocation == SVX_SHADOW_TOPRIGHT  )
/*N*/ 				nSpace = nWidth;
/*N*/ 			break;
/*N*/
/*N*/ 		case SHADOW_BOTTOM:
/*N*/ 			if ( eLocation == SVX_SHADOW_BOTTOMLEFT ||
/*N*/ 				 eLocation == SVX_SHADOW_BOTTOMRIGHT  )
/*N*/ 				nSpace = nWidth;
/*N*/ 			break;
/*N*/
/*N*/ 		case SHADOW_LEFT:
/*N*/ 			if ( eLocation == SVX_SHADOW_TOPLEFT ||
/*N*/ 				 eLocation == SVX_SHADOW_BOTTOMLEFT )
/*N*/ 				nSpace = nWidth;
/*N*/ 			break;
/*N*/
/*N*/ 		case SHADOW_RIGHT:
/*N*/ 			if ( eLocation == SVX_SHADOW_TOPRIGHT ||
/*N*/ 				 eLocation == SVX_SHADOW_BOTTOMRIGHT )
/*N*/ 				nSpace = nWidth;
/*N*/ 			break;
/*N*/
/*N*/ 		default:
/*N*/ 			DBG_ERROR( "wrong shadow" );
/*N*/ 	}
/*N*/ 	return nSpace;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxShadowItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_Int8) GetLocation()
/*N*/ 		  << (sal_uInt16) GetWidth()
/*N*/ 		  << (sal_Bool)(aShadowColor.GetTransparency() > 0)
/*N*/ 		  << GetColor()
/*N*/ 		  << GetColor()
/*N*/ 		  << (sal_Int8)(aShadowColor.GetTransparency() > 0 ? 0 : 1); //BRUSH_NULL : BRUSH_SOLID
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxShadowItem::Create( SvStream& rStrm, sal_uInt16 ) const
/*N*/ {
/*N*/ 	sal_Int8 cLoc;
/*N*/ 	sal_uInt16 nWidth;
/*N*/ 	sal_Bool bTrans;
/*N*/ 	Color aColor;
/*N*/ 	Color aFillColor;
/*N*/ 	sal_Int8 nStyle;
/*N*/ 	rStrm >> cLoc >> nWidth
/*N*/ 		  >> bTrans >> aColor >> aFillColor >> nStyle;
/*N*/ 	aColor.SetTransparency(bTrans ? 0xff : 0);
/*N*/ 	return new SvxShadowItem( Which(), &aColor, nWidth, (SvxShadowLocation)cLoc );
/*N*/ }

// -----------------------------------------------------------------------

/*?*/ sal_uInt16 SvxShadowItem::GetValueCount() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return SVX_SHADOW_END;	// SVX_SHADOW_BOTTOMRIGHT + 1
/*?*/ }

// -----------------------------------------------------------------------

/*?*/ sal_uInt16 SvxShadowItem::GetEnumValue() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return GetLocation();
/*?*/ }

// -----------------------------------------------------------------------

/*?*/ void SvxShadowItem::SetEnumValue( sal_uInt16 nVal )
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	SetLocation( (const SvxShadowLocation)nVal );
/*?*/ }

// class SvxBorderLine  --------------------------------------------------

/*N*/ SvxBorderLine::SvxBorderLine( const Color *pCol, sal_uInt16 nOut, sal_uInt16 nIn,
/*N*/ 							  sal_uInt16 nDist ) :
/*N*/
/*N*/ 	nOutWidth( nOut ),
/*N*/ 	nInWidth ( nIn ),
/*N*/ 	nDistance( nDist )
/*N*/
/*N*/ {
/*N*/ 	if ( pCol )
/*N*/ 		aColor = *pCol;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_Bool SvxBorderLine::operator==( const SvxBorderLine& rCmp ) const
/*N*/ {
/*N*/ 	return ( ( aColor    == rCmp.GetColor() ) 	 &&
/*N*/ 			 ( nInWidth  == rCmp.GetInWidth() )  &&
/*N*/ 			 ( nOutWidth == rCmp.GetOutWidth() ) &&
/*N*/ 			 ( nDistance == rCmp.GetDistance() ) );
/*N*/ }

// -----------------------------------------------------------------------

// class SvxBoxItem ------------------------------------------------------

/*N*/ SvxBoxItem::SvxBoxItem( const SvxBoxItem& rCpy ) :
/*N*/
/*N*/ 	SfxPoolItem	( rCpy ),
/*N*/ 	nTopDist	( rCpy.nTopDist ),
/*N*/ 	nBottomDist	( rCpy.nBottomDist ),
/*N*/ 	nLeftDist	( rCpy.nLeftDist ),
/*N*/ 	nRightDist	( rCpy.nRightDist )
/*N*/
/*N*/ {
/*N*/ 	pTop 	= rCpy.GetTop() 	? new SvxBorderLine( *rCpy.GetTop() ) 	 : 0;
/*N*/ 	pBottom = rCpy.GetBottom()	? new SvxBorderLine( *rCpy.GetBottom() ) : 0;
/*N*/ 	pLeft 	= rCpy.GetLeft() 	? new SvxBorderLine( *rCpy.GetLeft() ) 	 : 0;
/*N*/ 	pRight 	= rCpy.GetRight() 	? new SvxBorderLine( *rCpy.GetRight() )  : 0;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBoxItem::SvxBoxItem( const sal_uInt16 nId ) :
/*N*/ 	SfxPoolItem( nId ),
/*N*/
/*N*/ 	pTop		( 0 ),
/*N*/ 	pBottom		( 0 ),
/*N*/ 	pLeft		( 0 ),
/*N*/ 	pRight		( 0 ),
/*N*/ 	nTopDist	( 0 ),
/*N*/ 	nBottomDist	( 0 ),
/*N*/ 	nLeftDist	( 0 ),
/*N*/ 	nRightDist	( 0 )
/*N*/
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBoxItem::~SvxBoxItem()
/*N*/ {
/*N*/ 	delete pTop;
/*N*/ 	delete pBottom;
/*N*/ 	delete pLeft;
/*N*/ 	delete pRight;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBoxItem& SvxBoxItem::operator=( const SvxBoxItem& rBox )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return *this;//STRIP001
/*N*/ }

// -----------------------------------------------------------------------

/*?*/ inline sal_Bool CmpBrdLn( const SvxBorderLine* pBrd1, const SvxBorderLine* pBrd2 )
/*?*/ {
/*?*/ 	sal_Bool bRet;
/*?*/ 	if( 0 != pBrd1 ?  0 == pBrd2 : 0 != pBrd2 )
/*?*/ 		bRet = sal_False;
/*?*/ 	else
/*?*/ 		if( !pBrd1 )
/*?*/ 			bRet = sal_True;
/*?*/ 		else
/*?*/ 			bRet = (*pBrd1 == *pBrd2);
/*?*/ 	return bRet;
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxBoxItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/
/*N*/ 	return (
/*N*/ 		( nTopDist == ( (SvxBoxItem&)rAttr ).nTopDist )	&&
/*N*/ 		( nBottomDist == ( (SvxBoxItem&)rAttr ).nBottomDist )	&&
/*N*/ 		( nLeftDist == ( (SvxBoxItem&)rAttr ).nLeftDist )	&&
/*N*/ 		( nRightDist == ( (SvxBoxItem&)rAttr ).nRightDist )	&&
/*N*/ 		CmpBrdLn( pTop, ( (SvxBoxItem&)rAttr ).GetTop() )			&&
/*N*/ 		CmpBrdLn( pBottom, ( (SvxBoxItem&)rAttr ).GetBottom() )		&&
/*N*/ 		CmpBrdLn( pLeft, ( (SvxBoxItem&)rAttr ).GetLeft() )			&&
/*N*/ 		CmpBrdLn( pRight, ( (SvxBoxItem&)rAttr ).GetRight() ) );
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ table::BorderLine lcl_SvxLineToLine(const SvxBorderLine* pLine, sal_Bool bConvert)
/*N*/ {
/*N*/ 	table::BorderLine aLine;
/*N*/ 	if(pLine)
/*N*/ 	{
/*N*/ 		aLine.Color			 = pLine->GetColor().GetColor() ;
/*N*/ 		aLine.InnerLineWidth = bConvert ? TWIP_TO_MM100(pLine->GetInWidth() ): pLine->GetInWidth()  ;
/*N*/ 		aLine.OuterLineWidth = bConvert ? TWIP_TO_MM100(pLine->GetOutWidth()): pLine->GetOutWidth() ;
/*N*/ 		aLine.LineDistance	 = bConvert ? TWIP_TO_MM100(pLine->GetDistance()): pLine->GetDistance() ;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		aLine.Color			 = aLine.InnerLineWidth = aLine.OuterLineWidth = aLine.LineDistance	 = 0;
/*N*/ 	return aLine;
/*N*/ }
// -----------------------------------------------------------------------
/*N*/ bool SvxBoxItem::QueryValue( uno::Any& rVal, BYTE nMemberId  ) const
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	table::BorderLine aRetLine;
/*N*/ 	sal_uInt16 nDist;
/*N*/ 	sal_Bool bDistMember = sal_False;
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/     sal_Bool bSerialize = sal_False;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/         case MID_LEFT_BORDER:
/*N*/             bSerialize = sal_True;      // intentionally no break!
/*N*/ 		case LEFT_BORDER:
/*N*/ 			aRetLine = lcl_SvxLineToLine(GetLeft(), bConvert);
/*N*/ 			break;
/*N*/         case MID_RIGHT_BORDER:
/*N*/             bSerialize = sal_True;      // intentionally no break!
/*N*/ 		case RIGHT_BORDER:
/*N*/ 			aRetLine = lcl_SvxLineToLine(GetRight(), bConvert);
/*N*/ 			break;
/*N*/         case MID_BOTTOM_BORDER:
/*N*/             bSerialize = sal_True;      // intentionally no break!
/*N*/ 		case BOTTOM_BORDER:
/*N*/ 			aRetLine = lcl_SvxLineToLine(GetBottom(), bConvert);
/*N*/ 			break;
/*N*/         case MID_TOP_BORDER:
/*N*/             bSerialize = sal_True;      // intentionally no break!
/*N*/ 		case TOP_BORDER:
/*N*/ 			aRetLine = lcl_SvxLineToLine(GetTop(), bConvert);
/*N*/ 			break;
/*N*/ 		case BORDER_DISTANCE:
/*?*/ 			nDist = GetDistance();
/*?*/ 			bDistMember = sal_True;
/*?*/ 			break;
/*N*/ 		case TOP_BORDER_DISTANCE:
/*N*/ 			nDist = nTopDist;
/*N*/ 			bDistMember = sal_True;
/*N*/ 			break;
/*N*/ 		case BOTTOM_BORDER_DISTANCE:
/*N*/ 			nDist = nBottomDist;
/*N*/ 			bDistMember = sal_True;
/*N*/ 			break;
/*N*/ 		case LEFT_BORDER_DISTANCE:
/*N*/ 			nDist = nLeftDist;
/*N*/ 			bDistMember = sal_True;
/*N*/ 			break;
/*N*/ 		case RIGHT_BORDER_DISTANCE:
/*N*/ 			nDist = nRightDist;
/*N*/ 			bDistMember = sal_True;
/*N*/ 			break;
/*N*/ 	}
/*N*/
/*N*/ 	if( bDistMember )
/*N*/ 		rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nDist) : nDist);
/*N*/ 	else
/*N*/     {
/*
        if ( bSerialize )
        {
            ::com::sun::star::uno::Sequence < ::com::sun::star::uno::Any > aSeq(4);
            aSeq[0] <<= aRetLine.Color;
            aSeq[1] <<= aRetLine.InnerLineWidth;
            aSeq[2] <<= aRetLine.OuterLineWidth;
            aSeq[3] <<= aRetLine.LineDistance;
            rVal <<= aSeq;
        }
        else
*/
/*N*/             rVal <<= aRetLine;
/*N*/     }
/*N*/
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ sal_Bool lcl_LineToSvxLine(const ::com::sun::star::table::BorderLine& rLine, SvxBorderLine& rSvxLine, sal_Bool bConvert)
/*N*/ {
/*N*/ 	rSvxLine.SetColor(   Color(rLine.Color));
/*N*/ 	rSvxLine.SetInWidth( bConvert ? MM100_TO_TWIP(rLine.InnerLineWidth) : rLine.InnerLineWidth  );
/*N*/ 	rSvxLine.SetOutWidth(bConvert ? MM100_TO_TWIP(rLine.OuterLineWidth) : rLine.OuterLineWidth  );
/*N*/ 	rSvxLine.SetDistance(bConvert ? MM100_TO_TWIP(rLine.LineDistance	)  : rLine.LineDistance	 );
/*N*/ 	sal_Bool bRet = rLine.InnerLineWidth > 0 || rLine.OuterLineWidth > 0;
/*N*/ 	return bRet;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ bool SvxBoxItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	sal_uInt16 nLine = BOX_LINE_TOP;
/*N*/ 	sal_Bool bDistMember = sal_False;
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case LEFT_BORDER_DISTANCE:
/*N*/ 			bDistMember = sal_True;
/*N*/ 		case LEFT_BORDER:
/*N*/         case MID_LEFT_BORDER:
/*N*/ 			nLine = BOX_LINE_LEFT;
/*N*/ 			break;
/*N*/ 		case RIGHT_BORDER_DISTANCE:
/*N*/ 			bDistMember = sal_True;
/*N*/ 		case RIGHT_BORDER:
/*N*/         case MID_RIGHT_BORDER:
/*N*/ 			nLine = BOX_LINE_RIGHT;
/*N*/ 			break;
/*N*/ 		case BOTTOM_BORDER_DISTANCE:
/*N*/ 			bDistMember = sal_True;
/*N*/ 		case BOTTOM_BORDER:
/*N*/         case MID_BOTTOM_BORDER:
/*N*/ 			nLine = BOX_LINE_BOTTOM;
/*N*/ 			break;
/*N*/ 		case TOP_BORDER_DISTANCE:
/*N*/ 			bDistMember = sal_True;
/*N*/ 		case TOP_BORDER:
/*N*/         case MID_TOP_BORDER:
/*N*/ 			nLine = BOX_LINE_TOP;
/*N*/ 			break;
/*N*/ 	}
/*N*/
/*N*/ 	if( bDistMember || nMemberId == BORDER_DISTANCE )
/*N*/ 	{
/*N*/ 		sal_Int32 nDist;
/*N*/ 		if(!(rVal >>= nDist))
/*N*/ 			return sal_False;
/*N*/
/*N*/ 		if(nDist >= 0)
/*N*/ 		{
/*N*/ 			if( bConvert )
/*N*/ 				nDist = MM100_TO_TWIP(nDist);
/*N*/ 			if( nMemberId == BORDER_DISTANCE )
/*?*/ 				SetDistance( nDist );
/*N*/ 			else
/*N*/ 				SetDistance( nDist, nLine );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SvxBorderLine aLine;
/*N*/         if( !rVal.hasValue() )
/*N*/ 			return sal_False;
/*N*/
/*N*/         table::BorderLine aBorderLine;
/*N*/         if( rVal >>= aBorderLine )
/*N*/         {
/*N*/             // usual struct
/*N*/         }
/*N*/         else if (rVal.getValueTypeClass() == uno::TypeClass_SEQUENCE )
/*N*/         {
/*N*/             // serialization for basic macro recording
/*?*/             uno::Reference < script::XTypeConverter > xConverter
/*?*/                     ( ::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.script.Converter")),
/*?*/                     uno::UNO_QUERY );
/*?*/             uno::Sequence < uno::Any > aSeq;
/*?*/             uno::Any aNew;
/*?*/             try { aNew = xConverter->convertTo( rVal, ::getCppuType((const uno::Sequence < uno::Any >*)0) ); }
/*?*/             catch (uno::Exception&) {}
/*?*/
/*?*/             aNew >>= aSeq;
/*?*/             if ( aSeq.getLength() == 4 )
/*?*/             {
/*?*/                 sal_Int32 nVal;
/*?*/                 if ( aSeq[0] >>= nVal )
/*?*/                     aBorderLine.Color = nVal;
/*?*/                 if ( aSeq[1] >>= nVal )
/*?*/                     aBorderLine.InnerLineWidth = (sal_Int16) nVal;
/*?*/                 if ( aSeq[2] >>= nVal )
/*?*/                     aBorderLine.OuterLineWidth = (sal_Int16) nVal;
/*?*/                 if ( aSeq[3] >>= nVal )
/*?*/                     aBorderLine.LineDistance = (sal_Int16) nVal;
/*?*/             }
/*?*/             else
/*N*/                 return sal_False;
/*N*/         }
/*N*/         else
/*N*/             return sal_False;
/*N*/
/*N*/         sal_Bool bSet = lcl_LineToSvxLine(aBorderLine, aLine, bConvert);
/*N*/ 		SetLine(bSet ? &aLine : 0, nLine);
/*N*/ 	}
/*N*/
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxBoxItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxBoxItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxBoxItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_uInt16) GetDistance();
/*N*/ 	const SvxBorderLine* pLine[ 4 ]; 	// top, left, right, bottom
/*N*/ 	pLine[ 0 ] = GetTop();
/*N*/ 	pLine[ 1 ] = GetLeft();
/*N*/ 	pLine[ 2 ] = GetRight();
/*N*/ 	pLine[ 3 ] = GetBottom();
/*N*/
/*N*/ 	for( int i = 0; i < 4; i++ )
/*N*/ 	{
/*N*/ 		const SvxBorderLine* l = pLine[ i ];
/*N*/ 		if( l )
/*N*/ 		{
/*N*/ 			rStrm << (sal_Int8) i
/*N*/ 				  << l->GetColor()
/*N*/ 				  << (sal_uInt16) l->GetOutWidth()
/*N*/ 				  << (sal_uInt16) l->GetInWidth()
/*N*/ 				  << (sal_uInt16) l->GetDistance();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	sal_Int8 cLine = 4;
/*N*/ 	if( nItemVersion >= BOX_4DISTS_VERSION &&
/*N*/ 		!(nTopDist == nLeftDist &&
/*N*/ 		  nTopDist == nRightDist &&
/*N*/ 		  nTopDist == nBottomDist) )
/*N*/ 	{
/*?*/ 		cLine |= 0x10;
/*N*/ 	}
/*N*/
/*N*/ 	rStrm << cLine;
/*N*/
/*N*/ 	if( nItemVersion >= BOX_4DISTS_VERSION && (cLine & 0x10) != 0 )
/*N*/ 	{
/*N*/ 		rStrm << (sal_uInt16)nTopDist
/*N*/ 			  << (sal_uInt16)nLeftDist
/*N*/ 			  << (sal_uInt16)nRightDist
/*?*/ 			  << (sal_uInt16)nBottomDist;
/*N*/ 	}
/*N*/
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_uInt16 SvxBoxItem::GetVersion( sal_uInt16 nFFVer ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SvxBoxItem: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 		   SOFFICE_FILEFORMAT_40==nFFVer ? 0 : BOX_4DISTS_VERSION;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxBoxItem::Create( SvStream& rStrm, sal_uInt16 nIVersion ) const
/*N*/ {
/*N*/ 	sal_uInt16 nDistance;
/*N*/ 	rStrm >> nDistance;
/*N*/ 	SvxBoxItem* pAttr = new SvxBoxItem( Which() );
/*N*/
/*N*/ 	sal_uInt16 aLineMap[4] = { BOX_LINE_TOP, BOX_LINE_LEFT,
/*N*/ 						   BOX_LINE_RIGHT, BOX_LINE_BOTTOM };
/*N*/
/*N*/ 	sal_Int8 cLine;
/*N*/ 	while( sal_True )
/*N*/ 	{
/*N*/ 		rStrm >> cLine;
/*N*/
/*N*/ 		if( cLine > 3 )
/*N*/ 			break;
/*N*/ 		sal_uInt16 nOutline, nInline, nDistance;
/*N*/ 		Color aColor;
/*N*/ 		rStrm >> aColor >> nOutline >> nInline >> nDistance;
/*N*/ 		SvxBorderLine aBorder( &aColor, nOutline, nInline, nDistance );
/*N*/
/*N*/ 		pAttr->SetLine( &aBorder, aLineMap[cLine] );
/*N*/ 	}
/*N*/
/*N*/ 	if( nIVersion >= BOX_4DISTS_VERSION && (cLine&0x10) != 0 )
/*N*/ 	{
/*N*/ 		for( sal_uInt16 i=0; i < 4; i++ )
/*N*/ 		{
/*?*/ 			sal_uInt16 nDist;
/*?*/ 			rStrm >> nDist;
/*?*/ 			pAttr->SetDistance( nDist, aLineMap[i] );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pAttr->SetDistance( nDistance );
/*N*/ 	}
/*N*/
/*N*/ 	return pAttr;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxBoxItem::SetLine( const SvxBorderLine* pNew, sal_uInt16 nLine )
/*N*/ {
/*N*/ 	SvxBorderLine* pTmp = pNew ? new SvxBorderLine( *pNew ) : 0;
/*N*/
/*N*/ 	switch ( nLine )
/*N*/ 	{
/*N*/ 		case BOX_LINE_TOP:
/*N*/ 			delete pTop;
/*N*/ 			pTop = pTmp;
/*N*/ 			break;
/*N*/ 		case BOX_LINE_BOTTOM:
/*N*/ 			delete pBottom;
/*N*/ 			pBottom = pTmp;
/*N*/ 			break;
/*N*/ 		case BOX_LINE_LEFT:
/*N*/ 			delete pLeft;
/*N*/ 			pLeft = pTmp;
/*N*/ 			break;
/*N*/ 		case BOX_LINE_RIGHT:
/*N*/ 			delete pRight;
/*N*/ 			pRight = pTmp;
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			if ( pTmp )
/*N*/ 				delete pTmp;
/*N*/ 			DBG_ERROR( "wrong line" );
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_uInt16 SvxBoxItem::GetDistance() const
/*N*/ {
/*N*/ 	// The smallest distance that is not 0 will be returned.
/*N*/ 	sal_uInt16 nDist = nTopDist;
/*N*/ 	if( nBottomDist && (!nDist || nBottomDist < nDist) )
/*N*/ 		nDist = nBottomDist;
/*N*/ 	if( nLeftDist && (!nDist || nLeftDist < nDist) )
/*N*/ 		nDist = nLeftDist;
/*N*/ 	if( nRightDist && (!nDist || nRightDist < nDist) )
/*N*/ 		nDist = nRightDist;
/*N*/
/*N*/ 	return nDist;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_uInt16 SvxBoxItem::GetDistance( sal_uInt16 nLine ) const
/*N*/ {
/*N*/ 	sal_uInt16 nDist = 0;
/*N*/ 	switch ( nLine )
/*N*/ 	{
/*N*/ 		case BOX_LINE_TOP:
/*N*/ 			nDist = nTopDist;
/*N*/ 			break;
/*N*/ 		case BOX_LINE_BOTTOM:
/*N*/ 			nDist = nBottomDist;
/*N*/ 			break;
/*N*/ 		case BOX_LINE_LEFT:
/*N*/ 			nDist = nLeftDist;
/*N*/ 			break;
/*N*/ 		case BOX_LINE_RIGHT:
/*N*/ 			nDist = nRightDist;
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR( "wrong line" );
/*N*/ 	}
/*N*/
/*N*/ 	return nDist;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxBoxItem::SetDistance( sal_uInt16 nNew, sal_uInt16 nLine )
/*N*/ {
/*N*/ 	switch ( nLine )
/*N*/ 	{
/*N*/ 		case BOX_LINE_TOP:
/*N*/ 			nTopDist = nNew;
/*N*/ 			break;
/*N*/ 		case BOX_LINE_BOTTOM:
/*N*/ 			nBottomDist = nNew;
/*N*/ 			break;
/*N*/ 		case BOX_LINE_LEFT:
/*N*/ 			nLeftDist = nNew;
/*N*/ 			break;
/*N*/ 		case BOX_LINE_RIGHT:
/*N*/ 			nRightDist = nNew;
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR( "wrong line" );
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_uInt16 SvxBoxItem::CalcLineSpace( sal_uInt16 nLine, sal_Bool bIgnoreLine ) const
/*N*/ {
/*N*/ 	SvxBorderLine* pTmp = 0;
/*N*/ 	sal_uInt16 nDist = 0;
/*N*/ 	switch ( nLine )
/*N*/ 	{
/*N*/ 	case BOX_LINE_TOP:
/*N*/ 		pTmp = pTop;
/*N*/ 		nDist = nTopDist;
/*N*/ 		break;
/*N*/ 	case BOX_LINE_BOTTOM:
/*N*/ 		pTmp = pBottom;
/*N*/ 		nDist = nBottomDist;
/*N*/ 		break;
/*N*/ 	case BOX_LINE_LEFT:
/*N*/ 		pTmp = pLeft;
/*N*/ 		nDist = nLeftDist;
/*N*/ 		break;
/*N*/ 	case BOX_LINE_RIGHT:
/*N*/ 		pTmp = pRight;
/*N*/ 		nDist = nRightDist;
/*N*/ 		break;
/*N*/ 	default:
/*N*/ 		DBG_ERROR( "wrong line" );
/*N*/ 	}
/*N*/
/*N*/ 	if( pTmp )
/*N*/ 	{
/*N*/ 		nDist += pTmp->GetOutWidth();
/*N*/ 		nDist += pTmp->GetInWidth();
/*N*/ 		nDist += pTmp->GetDistance();
/*N*/ 	}
/*N*/ 	else if( !bIgnoreLine )
/*N*/ 		nDist = 0;
/*N*/ 	return nDist;
/*N*/ }

// class SvxBoxInfoItem --------------------------------------------------

/*N*/ SvxBoxInfoItem::SvxBoxInfoItem( const sal_uInt16 nId ) :
/*N*/
/*N*/ 	SfxPoolItem( nId ),
/*N*/
/*N*/ 	pHori	( 0 ),
/*N*/ 	pVert	( 0 ),
/*N*/ 	nDefDist( 0 )
/*N*/
/*N*/ {
/*N*/ 	bTable = bDist = bMinDist = sal_False;
/*N*/ 	ResetFlags();
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBoxInfoItem::SvxBoxInfoItem( const SvxBoxInfoItem& rCpy ) :
/*N*/
/*N*/ 	SfxPoolItem( rCpy )
/*N*/
/*N*/ {
/*N*/ 	pHori 		= rCpy.GetHori() ? new SvxBorderLine( *rCpy.GetHori() ) : 0;
/*N*/ 	pVert 		= rCpy.GetVert() ? new SvxBorderLine( *rCpy.GetVert() ) : 0;
/*N*/ 	bTable   	= rCpy.IsTable();
/*N*/ 	bDist    	= rCpy.IsDist();
/*N*/ 	bMinDist 	= rCpy.IsMinDist();
/*N*/ 	nValidFlags = rCpy.nValidFlags;
/*N*/ 	nDefDist 	= rCpy.GetDefDist();
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBoxInfoItem::~SvxBoxInfoItem()
/*N*/ {
/*N*/ 	delete pHori;
/*N*/ 	delete pVert;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxBoxInfoItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	SvxBoxInfoItem& rBoxInfo = (SvxBoxInfoItem&)rAttr;
/*N*/
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/
/*N*/ 	return (   bTable					 == rBoxInfo.IsTable()
/*N*/ 			&& bDist			 		 == rBoxInfo.IsDist()
/*N*/ 			&& bMinDist 		 		 == rBoxInfo.IsMinDist()
/*N*/ 			&& nValidFlags				 == rBoxInfo.nValidFlags
/*N*/ 			&& nDefDist 		 		 == rBoxInfo.GetDefDist()
/*N*/ 			&& CmpBrdLn( pHori, rBoxInfo.GetHori() )
/*N*/ 			&& CmpBrdLn( pVert, rBoxInfo.GetVert() )
/*N*/ 		   );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxBoxInfoItem::SetLine( const SvxBorderLine* pNew, sal_uInt16 nLine )
/*N*/ {
/*N*/ 	SvxBorderLine* pTmp = pNew ? new SvxBorderLine( *pNew ) : 0;
/*N*/
/*N*/ 	if ( BOXINFO_LINE_HORI == nLine )
/*N*/ 	{
/*N*/ 		delete pHori;
/*N*/ 		pHori = pTmp;
/*N*/ 	}
/*N*/ 	else if ( BOXINFO_LINE_VERT == nLine )
/*N*/ 	{
/*N*/ 		delete pVert;
/*N*/ 		pVert = pTmp;
/*N*/ 	}
/*N*/ 	else
/*N*/  	{
/*N*/ 	    	if ( pTmp )
/*N*/		    delete pTmp;
/*N*/ 		DBG_ERROR( "wrong line" );
/*N*/	}
/*N*/ }


// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxBoxInfoItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxBoxInfoItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxBoxInfoItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	sal_Int8 cFlags = 0;
/*N*/
/*N*/ 	if ( IsTable() )
/*N*/ 		cFlags |= 0x01;
/*N*/ 	if ( IsDist() )
/*N*/ 		cFlags |= 0x02;
/*N*/ 	if ( IsMinDist() )
/*N*/ 		cFlags |= 0x04;
/*N*/ 	rStrm << (sal_Int8)   cFlags
/*N*/ 		  << (sal_uInt16) GetDefDist();
/*N*/ 	const SvxBorderLine* pLine[ 2 ];
/*N*/ 	pLine[ 0 ] = GetHori();
/*N*/ 	pLine[ 1 ] = GetVert();
/*N*/
/*N*/ 	for( int i = 0; i < 2; i++ )
/*N*/ 	{
/*N*/ 		const SvxBorderLine* l = pLine[ i ];
/*N*/ 		if( l )
/*N*/ 		{
/*N*/ 			rStrm << (char) i
/*N*/ 				  << l->GetColor()
/*N*/ 				  << (short) l->GetOutWidth()
/*N*/ 				  << (short) l->GetInWidth()
/*?*/ 				  << (short) l->GetDistance();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	rStrm << (char) 2;
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxBoxInfoItem::Create( SvStream& rStrm, sal_uInt16 ) const
/*N*/ {
/*N*/ 	sal_Int8 cFlags;
/*N*/ 	sal_uInt16 nDefDist;
/*N*/ 	rStrm >> cFlags >> nDefDist;
/*N*/
/*N*/ 	SvxBoxInfoItem* pAttr = new SvxBoxInfoItem( Which() );
/*N*/
/*N*/ 	pAttr->SetTable  ( ( cFlags & 0x01 ) != 0 );
/*N*/ 	pAttr->SetDist   ( ( cFlags & 0x02 ) != 0 );
/*N*/ 	pAttr->SetMinDist( ( cFlags & 0x04 ) != 0 );
/*N*/ 	pAttr->SetDefDist( nDefDist );
/*N*/
/*N*/ 	while( sal_True )
/*N*/ 	{
/*N*/ 		sal_Int8 cLine;
/*N*/ 		rStrm >> cLine;
/*N*/
/*N*/ 		if( cLine > 1 )
/*N*/ 			break;
/*?*/ 		short nOutline, nInline, nDistance;
/*?*/ 		Color aColor;
/*?*/ 		rStrm >> aColor >> nOutline >> nInline >> nDistance;
/*?*/ 		SvxBorderLine aBorder( &aColor, nOutline, nInline, nDistance );
/*?*/
/*?*/ 		switch( cLine )
/*?*/ 		{
/*?*/ 			case 0: pAttr->SetLine( &aBorder, BOXINFO_LINE_HORI ); break;
/*?*/ 			case 1: pAttr->SetLine( &aBorder, BOXINFO_LINE_VERT ); break;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return pAttr;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxBoxInfoItem::ResetFlags()
/*N*/ {
/*N*/ 	nValidFlags = 0x7F;	// alles g"ultig au/ser Disable
/*N*/ }


// -----------------------------------------------------------------------


// class SvxFmtBreakItem -------------------------------------------------

/*N*/ int SvxFmtBreakItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==( rAttr ), "unequal types" );
/*N*/
/*N*/ 	return GetValue() == ( (SvxFmtBreakItem&)rAttr ).GetValue();
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------
/*N*/ bool SvxFmtBreakItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	style::BreakType eBreak = style::BreakType_NONE;
/*N*/ 	switch ( (SvxBreak)GetValue() )
/*N*/ 	{
/*N*/ 	case SVX_BREAK_COLUMN_BEFORE:	eBreak = style::BreakType_COLUMN_BEFORE; break;
/*N*/ 	case SVX_BREAK_COLUMN_AFTER:	eBreak = style::BreakType_COLUMN_AFTER ; break;
/*N*/ 	case SVX_BREAK_COLUMN_BOTH:		eBreak = style::BreakType_COLUMN_BOTH  ; break;
/*N*/ 	case SVX_BREAK_PAGE_BEFORE:		eBreak = style::BreakType_PAGE_BEFORE  ; break;
/*N*/ 	case SVX_BREAK_PAGE_AFTER:		eBreak = style::BreakType_PAGE_AFTER   ; break;
/*N*/ 	case SVX_BREAK_PAGE_BOTH:		eBreak = style::BreakType_PAGE_BOTH    ; break;
/*N*/ 	}
/*N*/ 	rVal <<= eBreak;
/*N*/ 	return sal_True;
/*N*/ }
// -----------------------------------------------------------------------
/*N*/ bool SvxFmtBreakItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	style::BreakType nBreak;
/*N*/
/*N*/ 	if(!(rVal >>= nBreak))
/*N*/ 	{
/*N*/ 		sal_Int32 nValue;
/*?*/ 		if(!(rVal >>= nValue))
/*?*/ 			return sal_False;
/*?*/
/*?*/ 		nBreak = (style::BreakType) nValue;
/*N*/ 	}
/*N*/
/*N*/ 	SvxBreak eBreak = SVX_BREAK_NONE;
/*N*/ 	switch( nBreak )
/*N*/ 	{
/*N*/ 		case style::BreakType_COLUMN_BEFORE:	eBreak = SVX_BREAK_COLUMN_BEFORE; break;
/*?*/ 		case style::BreakType_COLUMN_AFTER:	eBreak = SVX_BREAK_COLUMN_AFTER;  break;
/*?*/ 		case style::BreakType_COLUMN_BOTH:		eBreak = SVX_BREAK_COLUMN_BOTH;   break;
/*N*/ 		case style::BreakType_PAGE_BEFORE:		eBreak = SVX_BREAK_PAGE_BEFORE;   break;
/*?*/ 		case style::BreakType_PAGE_AFTER:		eBreak = SVX_BREAK_PAGE_AFTER;    break;
/*?*/ 		case style::BreakType_PAGE_BOTH:		eBreak = SVX_BREAK_PAGE_BOTH;     break;
/*N*/ 	}
/*N*/ 	SetValue((sal_uInt16) eBreak);
/*N*/
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxFmtBreakItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxFmtBreakItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxFmtBreakItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_Int8)GetValue();
/*N*/ 	if( FMTBREAK_NOAUTO > nItemVersion )
/*N*/ 		rStrm << (sal_Int8)0x01;
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_uInt16 SvxFmtBreakItem::GetVersion( sal_uInt16 nFFVer ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SvxFmtBreakItem: Gibt es ein neues Fileformat?" );
/*N*/ 	return SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 		   SOFFICE_FILEFORMAT_40==nFFVer ? 0 : FMTBREAK_NOAUTO;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxFmtBreakItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
/*N*/ {
/*N*/ 	sal_Int8 eBreak, bDummy;
/*N*/ 	rStrm >> eBreak;
/*N*/ 	if( FMTBREAK_NOAUTO > nVersion )
/*N*/ 		rStrm >> bDummy;
/*N*/ 	return new SvxFmtBreakItem( (const SvxBreak)eBreak, Which() );
/*N*/ }

// -----------------------------------------------------------------------

/*?*/ sal_uInt16 SvxFmtBreakItem::GetValueCount() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return SVX_BREAK_END;	// SVX_BREAK_PAGE_BOTH + 1
/*?*/ }

// class SvxFmtKeepItem -------------------------------------------------

/*N*/ SfxPoolItem* SvxFmtKeepItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxFmtKeepItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxFmtKeepItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_Int8)GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxFmtKeepItem::Create( SvStream& rStrm, sal_uInt16 ) const
/*N*/ {
/*N*/ 	sal_Int8 bIsKeep;
/*N*/ 	rStrm >> bIsKeep;
/*N*/ 	return new SvxFmtKeepItem( sal_Bool( bIsKeep != 0 ), Which() );
/*N*/ }

//------------------------------------------------------------------------


// class SvxLineItem ------------------------------------------------------

/*?*/ SvxLineItem::SvxLineItem( const sal_uInt16 nId ) :
/*?*/
/*?*/ 	SfxPoolItem	( nId ),
/*?*/
/*?*/ 	pLine( NULL )
/*?*/ {
/*?*/ }

// -----------------------------------------------------------------------

/*?*/ int SvxLineItem::operator==( const SfxPoolItem& rAttr ) const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*?*/
/*?*/ }

// -----------------------------------------------------------------------

/*?*/ SfxPoolItem* SvxLineItem::Clone( SfxItemPool* ) const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 	return new SvxLineItem( *this );
/*?*/ }


// -----------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

// class SvxBrushItem ----------------------------------------------------

#define LOAD_GRAPHIC	((sal_uInt16)0x0001)
#define LOAD_LINK		((sal_uInt16)0x0002)
#define LOAD_FILTER		((sal_uInt16)0x0004)

// class SvxBrushItem_Impl -----------------------------------------------

class SvxBrushItem_Impl
{
public:
    BfGraphicObject*	pGraphicObject;
    sal_Int8        nGraphicTransparency; //contains a percentage value which is
                                          //copied to the GraphicObject when necessary
#ifndef SVX_LIGHT
    SfxMediumRef	xMedium;
#endif
    Link			aDoneLink;

    SvxBrushItem_Impl( BfGraphicObject* p ) : pGraphicObject( p ), nGraphicTransparency(0) {}
};

// class SvxBrushItemLink_Impl -------------------------------------------

class SvxBrushItemLink_Impl : public SfxBrushItemLink
{
    virtual Graphic						GetGraphic( const String& rLink, const String& rFilter );
    virtual CreateSvxBrushTabPage		GetBackgroundTabpageCreateFunc();
    virtual GetSvxBrushTabPageRanges	GetBackgroundTabpageRanges();
};

// -----------------------------------------------------------------------

/*?*/ Graphic SvxBrushItemLink_Impl::GetGraphic( const String& rLink, const String& rFilter)
/*?*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); Graphic aResult; return aResult;//STRIP001 Graphic aResult;
/*?*/ }

// -----------------------------------------------------------------------

/*?*/ CreateSvxBrushTabPage SvxBrushItemLink_Impl::GetBackgroundTabpageCreateFunc()
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 #ifndef SVX_LIGHT
/*?*/ }

// -----------------------------------------------------------------------

/*?*/ GetSvxBrushTabPageRanges SvxBrushItemLink_Impl::GetBackgroundTabpageRanges()
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 #ifndef SVX_LIGHT
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxBrushItem::InitSfxLink()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBrushItem::SvxBrushItem( sal_uInt16 nWhich ) :
/*N*/
/*N*/ 	SfxPoolItem( nWhich ),
/*N*/
/*N*/ 	aColor		( COL_TRANSPARENT ),
/*N*/ 	pImpl		( new SvxBrushItem_Impl( 0 ) ),
/*N*/ 	pStrLink	( NULL ),
/*N*/ 	pStrFilter	( NULL ),
/*N*/ 	eGraphicPos	( GPOS_NONE ),
/*N*/ 	bLoadAgain	( sal_True )
/*N*/
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBrushItem::SvxBrushItem( const Color& rColor, sal_uInt16 nWhich) :
/*N*/
/*N*/ 	SfxPoolItem( nWhich ),
/*N*/
/*N*/ 	aColor		( rColor ),
/*N*/ 	pImpl		( new SvxBrushItem_Impl( 0 ) ),
/*N*/ 	pStrLink	( NULL ),
/*N*/ 	pStrFilter	( NULL ),
/*N*/ 	eGraphicPos	( GPOS_NONE ),
/*N*/ 	bLoadAgain	( sal_True )
/*N*/
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBrushItem::SvxBrushItem( const Graphic& rGraphic, SvxGraphicPosition ePos,
/*N*/ 							sal_uInt16 nWhich ) :
/*N*/
/*N*/ 	SfxPoolItem( nWhich ),
/*N*/
/*N*/ 	aColor		( COL_TRANSPARENT ),
/*N*/ 	pImpl		( new SvxBrushItem_Impl( new BfGraphicObject( rGraphic ) ) ),
/*N*/ 	pStrLink	( NULL ),
/*N*/ 	pStrFilter	( NULL ),
/*N*/ 	eGraphicPos	( ( GPOS_NONE != ePos ) ? ePos : GPOS_MM ),
/*N*/ 	bLoadAgain	( sal_True )
/*N*/
/*N*/ {
/*N*/ 	DBG_ASSERT( GPOS_NONE != ePos, "SvxBrushItem-Ctor with GPOS_NONE == ePos" );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBrushItem::SvxBrushItem( const BfGraphicObject& rGraphicObj,
/*N*/ 							SvxGraphicPosition ePos, sal_uInt16 nWhich ) :
/*N*/
/*N*/ 	SfxPoolItem( nWhich ),
/*N*/
/*N*/ 	aColor		( COL_TRANSPARENT ),
/*N*/ 	pImpl		( new SvxBrushItem_Impl( new BfGraphicObject( rGraphicObj ) ) ),
/*N*/ 	pStrLink	( NULL ),
/*N*/ 	pStrFilter	( NULL ),
/*N*/ 	eGraphicPos	( ( GPOS_NONE != ePos ) ? ePos : GPOS_MM ),
/*N*/ 	bLoadAgain	( sal_True )
/*N*/
/*N*/ {
/*N*/ 	DBG_ASSERT( GPOS_NONE != ePos, "SvxBrushItem-Ctor with GPOS_NONE == ePos" );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBrushItem::SvxBrushItem(
/*N*/ 	const String& rLink, const String& rFilter,
/*N*/ 	SvxGraphicPosition ePos, sal_uInt16 nWhich ) :
/*N*/
/*N*/ 	SfxPoolItem( nWhich ),
/*N*/
/*N*/ 	aColor		( COL_TRANSPARENT ),
/*N*/ 	pImpl		( new SvxBrushItem_Impl( NULL ) ),
/*N*/ 	pStrLink	( new String( rLink ) ),
/*N*/ 	pStrFilter	( new String( rFilter ) ),
/*N*/ 	eGraphicPos	( ( GPOS_NONE != ePos ) ? ePos : GPOS_MM ),
/*N*/ 	bLoadAgain	( sal_True )
/*N*/
/*N*/ {
/*N*/ 	DBG_ASSERT( GPOS_NONE != ePos, "SvxBrushItem-Ctor with GPOS_NONE == ePos" );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBrushItem::SvxBrushItem( SvStream& rStream, sal_uInt16 nVersion,
/*N*/ 							sal_uInt16 nWhich ) :
/*N*/
/*N*/ 	SfxPoolItem( nWhich ),
/*N*/
/*N*/ 	aColor		( COL_TRANSPARENT ),
/*N*/ 	pImpl		( new SvxBrushItem_Impl( NULL ) ),
/*N*/ 	pStrLink	( NULL ),
/*N*/ 	pStrFilter	( NULL ),
/*N*/ 	eGraphicPos	( GPOS_NONE )
/*N*/
/*N*/ {
/*N*/ 	sal_Bool bTrans;
/*N*/ 	Color aTempColor;
/*N*/ 	Color aTempFillColor;
/*N*/ 	sal_Int8 nStyle;
/*N*/
/*N*/ 	rStream >> bTrans;
/*N*/ 	rStream >> aTempColor;
/*N*/ 	rStream >> aTempFillColor;
/*N*/ 	rStream >> nStyle;
/*N*/
/*N*/ 	switch ( nStyle )
/*N*/ 	{
/*N*/ 		case 8://BRUSH_25:
/*N*/ 		{
/*N*/ 			sal_uInt32	nRed	= aTempColor.GetRed();
/*N*/ 			sal_uInt32	nGreen	= aTempColor.GetGreen();
/*N*/ 			sal_uInt32	nBlue	= aTempColor.GetBlue();
/*N*/ 			nRed   += (sal_uInt32)(aTempFillColor.GetRed())*2;
/*N*/ 			nGreen += (sal_uInt32)(aTempFillColor.GetGreen())*2;
/*N*/ 			nBlue  += (sal_uInt32)(aTempFillColor.GetBlue())*2;
/*N*/ 			aColor = Color( (sal_Int8)(nRed/3), (sal_Int8)(nGreen/3), (sal_Int8)(nBlue/3) );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 		case 9://BRUSH_50:
/*N*/ 		{
/*N*/ 			sal_uInt32	nRed	= aTempColor.GetRed();
/*N*/ 			sal_uInt32	nGreen	= aTempColor.GetGreen();
/*N*/ 			sal_uInt32	nBlue	= aTempColor.GetBlue();
/*N*/ 			nRed   += (sal_uInt32)(aTempFillColor.GetRed());
/*N*/ 			nGreen += (sal_uInt32)(aTempFillColor.GetGreen());
/*N*/ 			nBlue  += (sal_uInt32)(aTempFillColor.GetBlue());
/*N*/ 			aColor = Color( (sal_Int8)(nRed/2), (sal_Int8)(nGreen/2), (sal_Int8)(nBlue/2) );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 		case 10://BRUSH_75:
/*N*/ 		{
/*N*/ 			sal_uInt32	nRed	= aTempColor.GetRed()*2;
/*N*/ 			sal_uInt32	nGreen	= aTempColor.GetGreen()*2;
/*N*/ 			sal_uInt32	nBlue	= aTempColor.GetBlue()*2;
/*N*/ 			nRed   += (sal_uInt32)(aTempFillColor.GetRed());
/*N*/ 			nGreen += (sal_uInt32)(aTempFillColor.GetGreen());
/*N*/ 			nBlue  += (sal_uInt32)(aTempFillColor.GetBlue());
/*N*/ 			aColor = Color( (sal_Int8)(nRed/3), (sal_Int8)(nGreen/3), (sal_Int8)(nBlue/3) );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 		case 0://BRUSH_NULL:
/*N*/ 			aColor = Color( COL_TRANSPARENT );
/*N*/ 		break;
/*N*/
/*N*/ 		default:
/*N*/ 			aColor = aTempColor;
/*N*/ 	}
/*N*/
/*N*/ 	if ( nVersion >= BRUSH_GRAPHIC_VERSION )
/*N*/ 	{
/*N*/ 		sal_uInt16 nDoLoad = 0;
/*N*/ 		sal_Int8 nPos;
/*N*/
/*N*/ 		rStream >> nDoLoad;
/*N*/
/*N*/ 		if ( nDoLoad & LOAD_GRAPHIC )
/*N*/ 		{
/*N*/ 			Graphic aGraphic;
/*N*/
/*N*/ 			rStream >> aGraphic;
/*N*/ 			pImpl->pGraphicObject = new BfGraphicObject( aGraphic );
/*N*/ 
/*N*/ 			if( SVSTREAM_FILEFORMAT_ERROR == rStream.GetError() )
/*N*/ 			{
/*?*/ 				rStream.ResetError();
/*?*/ 				rStream.SetError( ERRCODE_SVX_GRAPHIC_WRONG_FILEFORMAT|
/*?*/ 								  ERRCODE_WARNING_MASK  );
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if ( nDoLoad & LOAD_LINK )
/*N*/ 		{
/*N*/ 			String aRel;
/*N*/ 			// UNICODE: rStream >> aRel;
/*N*/ 			rStream.ReadByteString(aRel);
/*N*/
/*N*/ 			String aAbs = ::binfilter::StaticBaseUrl::RelToAbs( aRel );
/*N*/ 			pStrLink = new String( aAbs );
/*N*/ 		}
/*N*/
/*N*/ 		if ( nDoLoad & LOAD_FILTER )
/*N*/ 		{
/*N*/ 			pStrFilter = new String;
/*N*/ 			// UNICODE: rStream >> *pStrFilter;
/*N*/ 			rStream.ReadByteString(*pStrFilter);
/*N*/ 		}
/*N*/
/*N*/ 		rStream >> nPos;
/*N*/
/*N*/ 		eGraphicPos = (SvxGraphicPosition)nPos;
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBrushItem::SvxBrushItem( const SvxBrushItem& rItem ) :
/*N*/
/*N*/ 	SfxPoolItem( rItem.Which() ),
/*N*/
/*N*/ 	pImpl		( new SvxBrushItem_Impl( NULL ) ),
/*N*/ 	pStrLink	( NULL ),
/*N*/ 	pStrFilter	( NULL ),
/*N*/ 	eGraphicPos	( GPOS_NONE ),
/*N*/ 	bLoadAgain	( sal_True )
/*N*/
/*N*/ {
/*N*/ 	*this = rItem;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBrushItem::~SvxBrushItem()
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if( pImpl->xMedium.Is() )
/*?*/ 		pImpl->xMedium->SetDoneLink( Link() );
/*N*/ #endif
/*N*/ 	delete pImpl->pGraphicObject;
/*N*/ 	delete pImpl;
/*N*/ 	delete pStrLink;
/*N*/ 	delete pStrFilter;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_uInt16 SvxBrushItem::GetVersion( sal_uInt16 nFileVersion ) const
/*N*/ {
/*N*/ 	return BRUSH_GRAPHIC_VERSION;
/*N*/ }

// -----------------------------------------------------------------------
/*?*/ inline sal_Int8 lcl_PercentToTransparency(long nPercent)
/*?*/ {
/*?*/     //0xff must not be returned!
/*?*/     return sal_Int8(nPercent ? (50 + 0xfe * nPercent) / 100 : 0);
/*?*/ }
/*?*/ inline sal_Int8 lcl_TransparencyToPercent(sal_Int32 nTrans)
/*?*/ {
/*?*/     return (nTrans * 100 + 127) / 254;
/*?*/ }

/*N*/ bool SvxBrushItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId)
/*N*/ 	{
/*N*/ 		case MID_BACK_COLOR:
/*N*/ 			rVal <<= (sal_Int32)( aColor.GetColor() );
/*N*/ 		break;
/*N*/         case MID_BACK_COLOR_R_G_B:
/*N*/             rVal <<= (sal_Int32)( aColor.GetRGBColor() );
/*N*/         break;
/*N*/         case MID_BACK_COLOR_TRANSPARENCY:
/*N*/             rVal <<= lcl_TransparencyToPercent(aColor.GetTransparency());
/*N*/         break;
/*N*/         case MID_GRAPHIC_POSITION:
/*N*/             rVal <<= (style::GraphicLocation)(sal_Int16)eGraphicPos;
/*N*/ 		break;
/*N*/
/*N*/ 		case MID_GRAPHIC:
/*N*/ 			DBG_ERRORFILE( "not implemented" );
/*N*/ 		break;
/*N*/
/*N*/ 		case MID_GRAPHIC_TRANSPARENT:
/*N*/             rVal = Bool2Any( aColor.GetTransparency() == 0xff );
/*N*/ 		break;
/*N*/
/*N*/ 		case MID_GRAPHIC_URL:
/*N*/ 		{
/*N*/ 			OUString sLink;
/*N*/ 			if ( pStrLink )
/*N*/ 				sLink = *pStrLink;
/*N*/ 			else if( pImpl->pGraphicObject )
/*N*/ 			{
/*N*/ 				OUString sPrefix(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
/*N*/ 				String sId( pImpl->pGraphicObject->GetUniqueID(),
/*N*/ 							RTL_TEXTENCODING_ASCII_US );
/*N*/ 				sLink = sPrefix;
/*N*/ 			   	sLink += OUString(sId);
/*N*/ 			}
/*N*/ 			rVal <<= sLink;
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 		case MID_GRAPHIC_FILTER:
/*N*/ 		{
/*N*/ 			OUString sFilter;
/*N*/ 			if ( pStrFilter )
/*N*/ 				sFilter = *pStrFilter;
/*N*/ 			rVal <<= sFilter;
/*N*/ 		}
/*N*/ 		break;
/*N*/         case MID_GRAPHIC_TRANSPARENCY :
/*N*/             rVal <<= pImpl->nGraphicTransparency;
/*N*/         break;
/*N*/ 	}
/*N*/
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ bool SvxBrushItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId)
/*N*/ 	{
/*N*/ 		case MID_BACK_COLOR:
/*N*/         case MID_BACK_COLOR_R_G_B:
/*N*/         {
/*N*/ 			sal_Int32 nCol;
/*N*/ 			if ( !( rVal >>= nCol ) )
/*N*/ 				return sal_False;
/*N*/             if(MID_BACK_COLOR_R_G_B == nMemberId)
/*N*/             {
/*N*/                 nCol = COLORDATA_RGB( nCol );
/*N*/                 nCol += aColor.GetColor() & 0xff000000;
/*N*/             }
/*N*/             aColor = Color( nCol );
/*N*/ 		}
/*N*/ 		break;
/*N*/         case MID_BACK_COLOR_TRANSPARENCY:
/*N*/         {
/*N*/             sal_Int32 nTrans;
/*N*/             if ( !( rVal >>= nTrans ) || nTrans < 0 || nTrans > 100 )
/*N*/ 				return sal_False;
/*N*/             aColor.SetTransparency(lcl_PercentToTransparency(nTrans));
/*N*/         }
/*N*/         break;
/*N*/
/*N*/ 		case MID_GRAPHIC_POSITION:
/*N*/ 		{
/*N*/ 			style::GraphicLocation eLocation;
/*N*/ 			if ( !( rVal>>=eLocation ) )
/*N*/ 			{
/*N*/ 				sal_Int32 nValue;
/*?*/ 				if ( !( rVal >>= nValue ) )
/*?*/ 					return sal_False;
/*?*/ 				eLocation = (style::GraphicLocation)nValue;
/*N*/ 			}
/*N*/ 			SetGraphicPos( (SvxGraphicPosition)(sal_uInt16)eLocation );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 		case MID_GRAPHIC:
/*N*/ 			DBG_ERRORFILE( "not implemented" );
/*N*/ 		break;
/*N*/
/*N*/ 		case MID_GRAPHIC_TRANSPARENT:
/*N*/ 			aColor.SetTransparency( Any2Bool( rVal ) ? 0xff : 0 );
/*N*/ 		break;
/*N*/
/*N*/ 		case MID_GRAPHIC_URL:
/*N*/ 		{
/*N*/ 			if ( rVal.getValueType() == ::getCppuType( (OUString*)0 ) )
/*N*/ 			{
/*N*/ 				OUString sLink;
/*N*/ 				rVal >>= sLink;
/*N*/ 				if( 0 == sLink.compareToAscii( UNO_NAME_GRAPHOBJ_URLPKGPREFIX,
/*N*/ 								  sizeof(UNO_NAME_GRAPHOBJ_URLPKGPREFIX)-1 ) )
/*N*/ 				{
/*N*/ 					DBG_ERROR( "package urls aren't implemented" );
/*N*/ 				}
/*N*/ 				else if( 0 == sLink.compareToAscii( UNO_NAME_GRAPHOBJ_URLPREFIX,
/*N*/ 								   sizeof(UNO_NAME_GRAPHOBJ_URLPREFIX)-1 ) )
/*N*/ 				{
/*N*/ 					DELETEZ( pStrLink );
/*N*/ 					String sTmp( sLink );
/*N*/ 					ByteString sId( sTmp.Copy(
/*N*/ 										sizeof(UNO_NAME_GRAPHOBJ_URLPREFIX)-1),
/*N*/ 									RTL_TEXTENCODING_ASCII_US );
/*N*/ 					BfGraphicObject *pOldGrfObj = pImpl->pGraphicObject;
/*N*/ 					pImpl->pGraphicObject = new BfGraphicObject( sId );
/*N*/                     ApplyGraphicTransparency_Impl();
/*N*/                     delete pOldGrfObj;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					SetGraphicLink(sLink);
/*N*/ 				}
/*N*/ 				if ( sLink.getLength() && eGraphicPos == GPOS_NONE )
/*N*/ 					eGraphicPos = GPOS_MM;
/*N*/ 				else if( !sLink.getLength() )
/*N*/ 					eGraphicPos = GPOS_NONE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 		case MID_GRAPHIC_FILTER:
/*N*/ 		{
/*N*/ 			if( rVal.getValueType() == ::getCppuType( (OUString*)0 ) )
/*N*/ 			{
/*N*/ 				OUString sLink;
/*N*/ 				rVal >>= sLink;
/*N*/ 				SetGraphicFilter( sLink );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/         case MID_GRAPHIC_TRANSPARENCY :
/*N*/         {
/*N*/             sal_Int32 nTmp;
/*N*/             rVal >>= nTmp;
/*N*/             if(nTmp >= 0 && nTmp <= 100)
/*N*/             {
/*N*/                 pImpl->nGraphicTransparency = sal_Int8(nTmp);
/*N*/                 if(pImpl->pGraphicObject)
/*N*/                     ApplyGraphicTransparency_Impl();
/*N*/             }
/*N*/         }
/*N*/         break;
/*N*/     }
/*N*/
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBrushItem& SvxBrushItem::operator=( const SvxBrushItem& rItem )
/*N*/ {
/*N*/ 	aColor = rItem.aColor;
/*N*/ 	eGraphicPos = rItem.eGraphicPos;
/*N*/
/*N*/ 	DELETEZ( pImpl->pGraphicObject );
/*N*/ 	DELETEZ( pStrLink );
/*N*/ 	DELETEZ( pStrFilter );
/*N*/
/*N*/ 	if ( GPOS_NONE != eGraphicPos )
/*N*/ 	{
/*N*/ 		if ( rItem.pStrLink )
/*N*/ 			pStrLink = new String( *rItem.pStrLink );
/*N*/ 		if ( rItem.pStrFilter )
/*N*/ 			pStrFilter = new String( *rItem.pStrFilter );
/*N*/ 		if ( rItem.pImpl->pGraphicObject )
/*N*/         {
/*N*/ 			pImpl->pGraphicObject = new BfGraphicObject( *rItem.pImpl->pGraphicObject );
/*N*/         }
/*N*/ 	}
/*N*/     pImpl->nGraphicTransparency = rItem.pImpl->nGraphicTransparency;
/*N*/ 	return *this;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxBrushItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/
/*N*/ 	SvxBrushItem& rCmp = (SvxBrushItem&)rAttr;
/*N*/     sal_Bool bEqual = ( aColor == rCmp.aColor && eGraphicPos == rCmp.eGraphicPos &&
/*N*/         pImpl->nGraphicTransparency == rCmp.pImpl->nGraphicTransparency);
/*N*/
/*N*/ 	if ( bEqual )
/*N*/ 	{
/*N*/ 		if ( GPOS_NONE != eGraphicPos )
/*N*/ 		{
/*N*/ 			if ( !rCmp.pStrLink )
/*N*/ 				bEqual = !pStrLink;
/*N*/ 			else
/*N*/ 				bEqual = pStrLink && ( *pStrLink == *rCmp.pStrLink );
/*N*/
/*N*/ 			if ( bEqual )
/*N*/ 			{
/*N*/ 				if ( !rCmp.pStrFilter )
/*N*/ 					bEqual = !pStrFilter;
/*N*/ 				else
/*N*/ 					bEqual = pStrFilter && ( *pStrFilter == *rCmp.pStrFilter );
/*N*/ 			}
/*N*/
/*N*/ 			if ( bEqual && !rCmp.pStrLink )
/*N*/ 			{
/*N*/ 				if ( !rCmp.pImpl->pGraphicObject )
/*?*/ 					bEqual = !pImpl->pGraphicObject;
/*N*/ 				else
/*N*/ 					bEqual = pImpl->pGraphicObject &&
/*N*/                              ( *pImpl->pGraphicObject == *rCmp.pImpl->pGraphicObject );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return bEqual;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxBrushItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxBrushItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxBrushItem::Create( SvStream& rStream, sal_uInt16 nVersion ) const
/*N*/ {
/*N*/ 	return new SvxBrushItem( rStream, nVersion, Which() );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxBrushItem::Store( SvStream& rStream , sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStream << (sal_Bool)sal_False;
/*N*/ 	rStream << aColor;
/*N*/ 	rStream << aColor;
/*N*/ 	rStream << (sal_Int8)(aColor.GetTransparency() > 0 ? 0 : 1); //BRUSH_NULL : BRUSH_SOLID
/*N*/
/*N*/ 	sal_uInt16 nDoLoad = 0;
/*N*/
/*N*/ 	if ( pImpl->pGraphicObject && !pStrLink )
/*N*/ 		nDoLoad |= LOAD_GRAPHIC;
/*N*/ 	if ( pStrLink )
/*N*/ 		nDoLoad |= LOAD_LINK;
/*N*/ 	if ( pStrFilter )
/*N*/ 		nDoLoad |= LOAD_FILTER;
/*N*/ 	rStream << nDoLoad;
/*N*/
/*N*/ 	if ( pImpl->pGraphicObject && !pStrLink )
/*N*/ 		rStream << pImpl->pGraphicObject->GetGraphic();
/*N*/ 	if ( pStrLink )
/*N*/ 	{
/*N*/ 		String aRel = ::binfilter::StaticBaseUrl::AbsToRel( *pStrLink );
/*N*/ 		// UNICODE: rStream << aRel;
/*N*/ 		rStream.WriteByteString(aRel);
/*N*/ 	}
/*N*/ 	if ( pStrFilter )
/*N*/ 	{
/*N*/ 		// UNICODE: rStream << *pStrFilter;
/*?*/ 		rStream.WriteByteString(*pStrFilter);
/*N*/ 	}
/*N*/ 	rStream << (sal_Int8)eGraphicPos;
/*N*/ 	return rStream;
/*N*/ }

// -----------------------------------------------------------------------
// const wegcasten, da const als logisches const zu verstehen ist
// wenn GetGraphic() gerufen wird, soll sich das Item darum kuemmern,
// eine gelinkte Grafik zu holen.

GraphicFilter* GetGrfFilter();


// -----------------------------------------------------------------------

/*N*/ const BfGraphicObject* SvxBrushItem::GetGraphicObject( SfxObjectShell* pSh ) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if ( bLoadAgain && pStrLink && !pImpl->pGraphicObject && !pImpl->xMedium.Is() )
/*N*/ 	// wenn Grafik schon geladen, als Cache benutzen
/*N*/ 	{
/*?*/ 		if( pSh && pSh->IsAbortingImport() )
/*?*/ 		{
/*?*/ 			SvxBrushItem* pThis = (SvxBrushItem*)this;
/*?*/ 			pThis->bLoadAgain = sal_False;
/*?*/ 			return 0;
/*?*/ 		}
/*?*/ 		//JP 29.6.2001: only with "valid" names - empty names now allowed
/*?*/ 		if( pStrLink->Len() )
/*?*/ 		{
/*?*/ 			pImpl->xMedium = new SfxMedium(
/*?*/ 				*pStrLink, STREAM_STD_READ, sal_False );
/*?*/
/*?*/ 			pImpl->xMedium->SetTransferPriority( SFX_TFPRIO_VISIBLE_LOWRES_GRAPHIC );
/*?*/ 			if( pImpl->xMedium->IsRemote() )
/*?*/ 			{
/*?*/ 				if( pSh )
/*?*/ 					pSh->RegisterTransfer( *pImpl->xMedium );
/*?*/ 				else
/*?*/ 					DBG_WARNING( "SvxBrushItem::GetGraphic ohne DocShell" );
/*?*/ 			}
/*?*/
/*?*/ 			SfxMediumRef xRef( pImpl->xMedium );
/*?*/ 			// Ref halten wg. synchr. DoneCallback
/*?*/ 			if( pImpl->aDoneLink.IsSet() )
/*?*/ 			{
/*?*/ 				// Auf besonderen Wunsch des Writers wird der synchrone und der
/*?*/ 				// asynchrone Fall was die Benachrichtigung angeht unterschiedlich
/*?*/ 				// behandelt. Der Callback erfolgt nur bei asynchronem Eintreffen
/*?*/ 				// der Daten
/*?*/				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 				pImpl->xMedium->DownLoad( );
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/ 	return pImpl->pGraphicObject;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ const Graphic* SvxBrushItem::GetGraphic( SfxObjectShell* pSh ) const
/*N*/ {
/*N*/ 	const BfGraphicObject* pGrafObj = GetGraphicObject( pSh );
/*N*/ 	return( pGrafObj ? &( pGrafObj->GetGraphic() ) : NULL );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxBrushItem::SetGraphicPos( SvxGraphicPosition eNew )
/*N*/ {
/*N*/ 	eGraphicPos = eNew;
/*N*/
/*N*/ 	if ( GPOS_NONE == eGraphicPos )
/*N*/ 	{
/*N*/ 		DELETEZ( pImpl->pGraphicObject );
/*N*/ 		DELETEZ( pStrLink );
/*N*/ 		DELETEZ( pStrFilter );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( !pImpl->pGraphicObject && !pStrLink )
/*N*/ 		{
/*N*/ 			pImpl->pGraphicObject = new BfGraphicObject; // dummy anlegen
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxBrushItem::SetGraphic( const Graphic& rNew )
/*N*/ {
/*N*/ 	if ( !pStrLink )
/*N*/ 	{
/*N*/ 		if ( pImpl->pGraphicObject )
/*N*/ 			pImpl->pGraphicObject->SetGraphic( rNew );
/*N*/ 		else
/*N*/ 			pImpl->pGraphicObject = new BfGraphicObject( rNew );
/*N*/ 
/*N*/         ApplyGraphicTransparency_Impl();
/*N*/
/*N*/         if ( GPOS_NONE == eGraphicPos )
/*N*/ 			eGraphicPos = GPOS_MM; // None waere Brush, also Default: Mitte
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR( "SetGraphic() on linked graphic! :-/" );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxBrushItem::SetGraphicLink( const String& rNew )
/*N*/ {
/*N*/ 	if ( !rNew.Len() )
/*N*/ 		DELETEZ( pStrLink );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( pStrLink )
/*N*/ 			*pStrLink = rNew;
/*N*/ 		else
/*N*/ 			pStrLink = new String( rNew );
/*N*/
/*N*/ 		DELETEZ( pImpl->pGraphicObject );
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxBrushItem::SetGraphicFilter( const String& rNew )
/*N*/ {
/*N*/ 	if ( !rNew.Len() )
/*N*/ 		DELETEZ( pStrFilter );
/*N*/ 	else
/*N*/ 	{
/*?*/ 		if ( pStrFilter )
/*?*/ 			*pStrFilter = rNew;
/*?*/ 		else
/*?*/ 			pStrFilter = new String( rNew );
/*N*/ 	}
/*N*/ }

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif
/* -----------------------------16.08.2002 09:18------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void  SvxBrushItem::ApplyGraphicTransparency_Impl()
/*N*/ {
/*N*/     DBG_ASSERT(pImpl->pGraphicObject, "no GraphicObject available" );
/*N*/     if(pImpl->pGraphicObject)
/*N*/     {
/*N*/         BfGraphicAttr aAttr(pImpl->pGraphicObject->GetAttr());
/*N*/         aAttr.SetTransparency(lcl_PercentToTransparency(
/*N*/                             pImpl->nGraphicTransparency));
/*N*/         pImpl->pGraphicObject->SetAttr(aAttr);
/*N*/     }
/*N*/ }
// class SvxFrameDirectionItem ----------------------------------------------

/*N*/ SvxFrameDirectionItem::SvxFrameDirectionItem( SvxFrameDirection nValue ,
/*N*/ 											USHORT nWhich )
/*N*/ 	: SfxUInt16Item( nWhich, nValue )
/*N*/ {
/*N*/ }

/*N*/ SvxFrameDirectionItem::~SvxFrameDirectionItem()
/*N*/ {
/*N*/ }

/*N*/ int SvxFrameDirectionItem::operator==( const SfxPoolItem& rCmp ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rCmp), "unequal types" );
/*N*/
/*N*/ 	return GetValue() == ((SvxFrameDirectionItem&)rCmp).GetValue();
/*N*/ }

/*N*/ SfxPoolItem* SvxFrameDirectionItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxFrameDirectionItem( *this );
/*N*/ }

SfxPoolItem* SvxFrameDirectionItem::Create( SvStream & rStrm, USHORT /*nVer*/ ) const
{
    sal_uInt16 nValue;
    rStrm >> nValue;
    return new SvxFrameDirectionItem( (SvxFrameDirection)nValue, Which() );
}

/*N*/ SvStream& SvxFrameDirectionItem::Store( SvStream & rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/ 	sal_uInt16 nValue = GetValue();
/*N*/ 	rStrm << nValue;
/*N*/ 	return rStrm;
/*N*/ }

/*N*/ USHORT SvxFrameDirectionItem::GetVersion( USHORT nFVer ) const
/*N*/ {
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFVer ? USHRT_MAX : 0;
/*N*/ }


/*N*/ bool SvxFrameDirectionItem::PutValue( const ::com::sun::star::uno::Any& rVal,
/*N*/ 		 									BYTE )
/*N*/ {
/*N*/     sal_Int16 nVal;
/*N*/     sal_Bool bRet = ( rVal >>= nVal );
/*N*/     if( bRet )
/*N*/     {
/*N*/         // translate WritingDirection2 constants into SvxFrameDirection
/*N*/         switch( nVal )
/*N*/         {
/*N*/             case text::WritingMode2::LR_TB:
/*N*/                 SetValue( FRMDIR_HORI_LEFT_TOP );
/*N*/                 break;
/*N*/             case text::WritingMode2::RL_TB:
/*?*/                 SetValue( FRMDIR_HORI_RIGHT_TOP );
/*?*/                 break;
/*?*/             case text::WritingMode2::TB_RL:
/*?*/                 SetValue( FRMDIR_VERT_TOP_RIGHT );
/*?*/                 break;
/*?*/             case text::WritingMode2::TB_LR:
/*?*/                 SetValue( FRMDIR_VERT_TOP_LEFT );
/*?*/                 break;
/*?*/             case text::WritingMode2::PAGE:
/*?*/                 SetValue( FRMDIR_ENVIRONMENT );
/*?*/                 break;
/*?*/             default:
/*?*/                 bRet = sal_False;
/*N*/                 break;
/*N*/         }
/*N*/     }
/*N*/
/*N*/ 	return bRet;
/*N*/ }

/*N*/ bool SvxFrameDirectionItem::QueryValue( ::com::sun::star::uno::Any& rVal,
/*N*/ 											BYTE ) const
/*N*/ {
/*N*/     // translate SvxFrameDirection into WritingDirection2
/*N*/     sal_Int16 nVal;
/*N*/     sal_Bool bRet = sal_True;
/*N*/     switch( GetValue() )
/*N*/     {
/*N*/         case FRMDIR_HORI_LEFT_TOP:
/*N*/             nVal = text::WritingMode2::LR_TB;
/*N*/             break;
/*?*/         case FRMDIR_HORI_RIGHT_TOP:
/*?*/             nVal = text::WritingMode2::RL_TB;
/*?*/             break;
/*?*/         case FRMDIR_VERT_TOP_RIGHT:
/*?*/             nVal = text::WritingMode2::TB_RL;
/*?*/             break;
/*?*/         case FRMDIR_VERT_TOP_LEFT:
/*?*/             nVal = text::WritingMode2::TB_LR;
/*?*/             break;
/*?*/         case FRMDIR_ENVIRONMENT:
/*?*/             nVal = text::WritingMode2::PAGE;
/*?*/             break;
/*?*/         default:
/*?*/             DBG_ERROR("Unknown SvxFrameDirection value!");
/*?*/             bRet = sal_False;
/*?*/             break;
/*N*/     }
/*N*/
/*N*/     // return value + error state
/*N*/     if( bRet )
/*N*/     {
/*N*/         rVal <<= nVal;
/*N*/     }
/*N*/ 	return bRet;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
