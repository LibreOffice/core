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

#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <bf_svtools/syslocale.hxx>

#include <comphelper/types.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

#define ITEMID_TABSTOP		0
#define ITEMID_LINESPACING  0
#define ITEMID_ADJUST       0
#define ITEMID_ORPHANS      0
#define ITEMID_WIDOWS       0
#define ITEMID_PAGEMODEL    0
#define ITEMID_FMTSPLIT     0
#define ITEMID_HYPHENZONE   0
#define ITEMID_SCRIPTSPACE  0
#define ITEMID_HANGINGPUNCTUATION 0
#define ITEMID_FORBIDDENRULE 0
#define ITEMID_PARAVERTALIGN 0
#define ITEMID_PARAGRID 0

#include <tools/rtti.hxx>
#include "bf_basic/sbx.hxx"
#define GLOBALOVERFLOW3

#define _SVX_PARAITEM_CXX

#include <bf_svtools/itempool.hxx>

#include <bf_svtools/memberid.hrc>
#include "svxitems.hrc"

#include "lspcitem.hxx"
#include "adjitem.hxx"
#include "orphitem.hxx"
#include "widwitem.hxx"
#include "tstpitem.hxx"
#include "pmdlitem.hxx"
#include "spltitem.hxx"
#include "hyznitem.hxx"
#include "scriptspaceitem.hxx"
#include "hngpnctitem.hxx"
#include "forbiddenruleitem.hxx"
#include "paravertalignitem.hxx"
#include "pgrditem.hxx"


#include "rtl/ustring.hxx"

#include <unomid.hxx>

#include "itemtype.hxx"
#include "dialmgr.hxx"
#include "paperinf.hxx"

#include <vcl/svapp.hxx>

#include <algorithm>
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

// Konvertierung fuer UNO
#define TWIP_TO_MM100(TWIP) 	((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)	((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))


// STATIC DATA -----------------------------------------------------------


// -----------------------------------------------------------------------


/*N*/ TYPEINIT1_AUTOFACTORY(SvxLineSpacingItem, SfxPoolItem );
/*N*/ TYPEINIT1_AUTOFACTORY(SvxAdjustItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxWidowsItem, SfxByteItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxOrphansItem, SfxByteItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxHyphenZoneItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxTabStopItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxFmtSplitItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxPageModelItem, SfxStringItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxScriptSpaceItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxHangingPunctuationItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxForbiddenRuleItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxParaVertAlignItem, SfxUInt16Item);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxParaGridItem, SfxBoolItem);

/*N*/ SV_IMPL_VARARR_SORT( SvxTabStopArr, SvxTabStop )

// -----------------------------------------------------------------------

/*N*/ SvxLineSpacingItem::SvxLineSpacingItem( sal_uInt16 nHeight, const sal_uInt16 nId )
/*N*/ 	: SfxEnumItemInterface( nId )
/*N*/ {
/*N*/ 	nPropLineSpace = 100;
/*N*/ 	nInterLineSpace = 0;
/*N*/ 	nLineHeight = nHeight;
/*N*/ 	eLineSpace = SVX_LINE_SPACE_AUTO;
/*N*/ 	eInterLineSpace = SVX_INTER_LINE_SPACE_OFF;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxLineSpacingItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/ 
/*N*/ 	const SvxLineSpacingItem& rLineSpace = (const SvxLineSpacingItem&)rAttr;
/*N*/ 	return (
/*N*/ 		// Gleiche Linespacing Rule?
/*N*/ 		(eLineSpace == rLineSpace.eLineSpace)
/*N*/ 		// Bei maximalem und minimalem Linespacing muss das Mass
/*N*/ 		// uebereinstimmen.
/*N*/ 		&& (eLineSpace == SVX_LINE_SPACE_AUTO ||
/*N*/ 			nLineHeight == rLineSpace.nLineHeight)
/*N*/ 		// Gleiche Interlinespacing Rule?
/*N*/ 		&& ( eInterLineSpace == rLineSpace.eInterLineSpace )
/*N*/ 		// Entweder proportional oder draufaddieren eingestellt.
/*N*/ 		&& (( eInterLineSpace == SVX_INTER_LINE_SPACE_OFF)
/*N*/ 			|| (eInterLineSpace == SVX_INTER_LINE_SPACE_PROP
/*N*/ 				&& nPropLineSpace == rLineSpace.nPropLineSpace)
/*N*/ 			|| (eInterLineSpace == SVX_INTER_LINE_SPACE_FIX
/*N*/ 				&& (nInterLineSpace == rLineSpace.nInterLineSpace)))) ?
/*N*/ 				1 : 0;
/*N*/ }

/*-----------------18.03.98 16:32-------------------
    os: wer weiss noch, wieso das LineSpacingItem so
    kompliziert ist? Fuer UNO koennen wir das nicht
    gebrauchen. Da gibt es nur zwei Werte:
        - ein sal_uInt16 fuer den Modus
        - ein sal_uInt32 fuer alle Werte (Abstand, Hoehe, rel. Angaben)

--------------------------------------------------*/
/*N*/ bool SvxLineSpacingItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/ 
/*N*/ 	style::LineSpacing aLSp;
/*N*/ 	switch( eLineSpace )
/*N*/ 	{
/*N*/ 		case SVX_LINE_SPACE_AUTO:
/*N*/ 			if(eInterLineSpace == SVX_INTER_LINE_SPACE_FIX)
/*N*/ 			{
/*?*/ 				aLSp.Mode = style::LineSpacingMode::LEADING;
/*?*/ 				aLSp.Height = nInterLineSpace;
/*N*/ 			}
/*N*/ 			else if(eInterLineSpace == SVX_INTER_LINE_SPACE_OFF)
/*N*/ 			{
/*N*/ 				aLSp.Mode = style::LineSpacingMode::PROP;
/*N*/ 				aLSp.Height = 100;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				aLSp.Mode = style::LineSpacingMode::PROP;
/*N*/ 				aLSp.Height = nPropLineSpace;
/*N*/ 			}
/*N*/ 		break;
/*N*/ 		case SVX_LINE_SPACE_FIX :
/*N*/ 		case SVX_LINE_SPACE_MIN :
/*N*/ 			aLSp.Mode = eLineSpace == SVX_LINE_SPACE_FIX ? style::LineSpacingMode::FIX : style::LineSpacingMode::MINIMUM;
/*N*/             aLSp.Height = ( bConvert ? (short)TWIP_TO_MM100(nLineHeight) : nLineHeight );
/*N*/ 		break;
/*N*/ 	}
/*N*/ 
/*N*/     switch ( nMemberId )
/*N*/     {
/*?*/         case 0 :                rVal <<= aLSp; break;
/*?*/         case MID_LINESPACE :    rVal <<= aLSp.Mode; break;
/*?*/         case MID_HEIGHT :       rVal <<= aLSp.Height; break;
/*?*/         default: DBG_ERROR("Wrong MemberId!"); break;
/*N*/     }
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------18.03.98 16:32-------------------

--------------------------------------------------*/
/*N*/ bool SvxLineSpacingItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/ 
/*N*/     // fill with current data
/*N*/ 	style::LineSpacing aLSp;
/*N*/     uno::Any aAny;
/*N*/     sal_Bool bRet = QueryValue( aAny, bConvert ? CONVERT_TWIPS : 0 ) && ( aAny >>= aLSp );
/*N*/ 
/*N*/     // get new data
/*N*/     switch ( nMemberId )
/*N*/     {
/*N*/         case 0 :                bRet = (rVal >>= aLSp); break;
/*?*/         case MID_LINESPACE :    bRet = (rVal >>= aLSp.Mode); break;
/*?*/         case MID_HEIGHT :       bRet = (rVal >>= aLSp.Height); break;
/*N*/         default: DBG_ERROR("Wrong MemberId!"); break;
/*N*/     }
/*N*/ 
/*N*/     if( bRet )
/*N*/     {
/*N*/         nLineHeight = aLSp.Height;
/*N*/         switch( aLSp.Mode )
/*N*/         {
/*N*/             case style::LineSpacingMode::LEADING:
/*N*/             {
/*?*/                 eInterLineSpace = SVX_INTER_LINE_SPACE_FIX;
/*?*/                 eLineSpace = SVX_LINE_SPACE_AUTO;
/*?*/                 nInterLineSpace = aLSp.Height;
/*N*/             }
/*N*/             break;
/*N*/             case style::LineSpacingMode::PROP:
/*N*/             {
/*N*/                 eLineSpace = SVX_LINE_SPACE_AUTO;
/*N*/                 nPropLineSpace = (sal_Int8)std::min(aLSp.Height, (short)0xFF);
/*N*/                 if(100 == aLSp.Height)
/*N*/                     eInterLineSpace = SVX_INTER_LINE_SPACE_OFF;
/*N*/                 else
/*N*/                     eInterLineSpace = SVX_INTER_LINE_SPACE_PROP;
/*N*/             }
/*N*/             break;
/*N*/             case style::LineSpacingMode::FIX:
/*N*/             case style::LineSpacingMode::MINIMUM:
/*N*/             {
/*N*/                 eInterLineSpace =  SVX_INTER_LINE_SPACE_OFF;
/*N*/                 eLineSpace = aLSp.Mode == style::LineSpacingMode::FIX ? SVX_LINE_SPACE_FIX : SVX_LINE_SPACE_MIN;
/*N*/                 nLineHeight = aLSp.Height;
/*N*/                 if(bConvert)
/*N*/                     nLineHeight = (USHORT)MM100_TO_TWIP(nLineHeight);
/*N*/             }
/*N*/             break;
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     return bRet;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxLineSpacingItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxLineSpacingItem( *this );
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxLineSpacingItem::Create(SvStream& rStrm, sal_uInt16) const
/*N*/ {
/*N*/ 	sal_Int8	nPropSpace;
/*N*/ 	short	nInterSpace;
/*N*/ 	sal_uInt16	nHeight;
/*N*/ 	sal_Int8	nRule, nInterRule;
/*N*/ 
/*N*/ 	rStrm >> nPropSpace
/*N*/ 		  >> nInterSpace
/*N*/ 		  >> nHeight
/*N*/ 		  >> nRule
/*N*/ 		  >> nInterRule;
/*N*/ 
/*N*/ 	SvxLineSpacingItem* pAttr = new SvxLineSpacingItem( nHeight, Which() );
/*N*/ 	pAttr->SetInterLineSpace( nInterSpace );
/*N*/ 	pAttr->SetPropLineSpace( nPropSpace );
/*N*/ 	pAttr->GetLineSpaceRule() = (SvxLineSpace)nRule;
/*N*/ 	pAttr->GetInterLineSpaceRule() = (SvxInterLineSpace)nInterRule;
/*N*/ 	return pAttr;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxLineSpacingItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_Int8)  GetPropLineSpace()
/*N*/ 		  << (short)  GetInterLineSpace()
/*N*/ 		  << (sal_uInt16) GetLineHeight()
/*N*/ 		  << (sal_Int8)   GetLineSpaceRule()
/*N*/ 		  << (sal_Int8)   GetInterLineSpaceRule();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*?*/ sal_uInt16 SvxLineSpacingItem::GetValueCount() const
/*?*/ {
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); return SVX_LINESPACE_END;//STRIP001 /*?*/ 	return SVX_LINESPACE_END;	// SVX_LINESPACE_TWO_LINES + 1
/*?*/ }

// -----------------------------------------------------------------------

/*?*/ XubString SvxLineSpacingItem::GetValueTextByPos( sal_uInt16 nPos ) const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); XubString aText;//STRIP001 //STRIP001 	//! Strings demnaechst aus Resource laden
/*?*/ 	return aText;
/*?*/ }

// -----------------------------------------------------------------------

/*?*/ sal_uInt16 SvxLineSpacingItem::GetEnumValue() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 //STRIP001 	sal_uInt16 nVal;
/*?*/ }

// -----------------------------------------------------------------------

/*?*/ void SvxLineSpacingItem::SetEnumValue( sal_uInt16 nVal )
/*?*/ {
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	switch ( nVal )
/*?*/ }

// class SvxAdjustItem ---------------------------------------------------

/*N*/ SvxAdjustItem::SvxAdjustItem(const SvxAdjust eAdjst, const sal_uInt16 nId )
/*N*/ 	: SfxEnumItemInterface( nId ),
/*N*/ 	bOneBlock( sal_False ), bLastCenter( sal_False ), bLastBlock( sal_False )
/*N*/ {
/*N*/ 	SetAdjust( eAdjst );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxAdjustItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/ 
/*N*/ 	return( ( GetAdjust() == ((SvxAdjustItem&)rAttr).GetAdjust() &&
/*N*/ 		bOneBlock == ((SvxAdjustItem&)rAttr).bOneBlock &&
/*N*/ 		bLastCenter == ((SvxAdjustItem&)rAttr).bLastCenter &&
/*N*/ 		bLastBlock == ((SvxAdjustItem&)rAttr).bLastBlock )
/*N*/ 		? 1 : 0 );
/*N*/ }

/*-----------------18.03.98 16:15-------------------

--------------------------------------------------*/
/*N*/ bool SvxAdjustItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_PARA_ADJUST	  :	rVal <<= (sal_Int16)GetAdjust(); break;
/*N*/ 		case MID_LAST_LINE_ADJUST : rVal <<= (sal_Int16)GetLastBlock(); break;
/*N*/ 		case MID_EXPAND_SINGLE    :
/*N*/ 		{
/*N*/ 			sal_Bool bValue = bOneBlock;
/*N*/ 			rVal.setValue( &bValue, ::getCppuBooleanType() );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------18.03.98 16:15-------------------

--------------------------------------------------*/

/*N*/ bool SvxAdjustItem::PutValue( const uno::Any& rVal, BYTE nMemberId  )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_PARA_ADJUST			  :
/*N*/ 		case MID_LAST_LINE_ADJUST :
/*N*/ 		{
/*N*/ 			sal_Int32 eVal = - 1;
/*N*/ 			try
/*N*/ 			{
/*N*/ 				eVal = ::comphelper::getEnumAsINT32(rVal);
/*N*/ 			}
/*N*/ 			catch(...) {}
/*N*/ 			if(eVal >= 0 && eVal <= 4)
/*N*/ 			{
/*N*/ 				if(MID_LAST_LINE_ADJUST == nMemberId &&
/*N*/ 					eVal != SVX_ADJUST_LEFT &&
/*N*/ 					eVal != SVX_ADJUST_BLOCK &&
/*N*/ 					eVal != SVX_ADJUST_CENTER)
/*N*/ 						return FALSE;
/*N*/ 				if(eVal < (sal_uInt16)SVX_ADJUST_END)
/*N*/ 					nMemberId == MID_PARA_ADJUST ?
/*N*/ 						SetAdjust((SvxAdjust)eVal) :
/*N*/ 							SetLastBlock((SvxAdjust)eVal);
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_EXPAND_SINGLE :
/*N*/ 			bOneBlock = Any2Bool(rVal);
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxAdjustItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxAdjustItem( *this );
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------

/*?*/ sal_uInt16 SvxAdjustItem::GetValueCount() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return SVX_ADJUST_END;	// SVX_ADJUST_BLOCKLINE + 1
/*?*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*?*/ sal_uInt16 SvxAdjustItem::GetEnumValue() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return GetAdjust();
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxAdjustItem::SetEnumValue( sal_uInt16 nVal )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_uInt16 SvxAdjustItem::GetVersion( sal_uInt16 nFileVersion ) const
/*N*/ {
/*N*/ 	return (nFileVersion == SOFFICE_FILEFORMAT_31)
/*N*/ 			   ? 0 : ADJUST_LASTBLOCK_VERSION;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxAdjustItem::Create(SvStream& rStrm, sal_uInt16 nVersion) const
/*N*/ {
/*N*/ 	char eAdjustment;
/*N*/ 	rStrm >> eAdjustment;
/*N*/ 	SvxAdjustItem *pRet = new SvxAdjustItem( (SvxAdjust)eAdjustment, Which() );
/*N*/ 	if( nVersion >= ADJUST_LASTBLOCK_VERSION )
/*N*/ 	{
/*N*/ 		sal_Int8 nFlags;
/*N*/ 		rStrm >> nFlags;
/*N*/ 		pRet->bOneBlock = 0 != (nFlags & 0x0001);
/*N*/ 		pRet->bLastCenter = 0 != (nFlags & 0x0002);
/*N*/ 		pRet->bLastBlock = 0 != (nFlags & 0x0004);
/*N*/ 	}
/*N*/ 	return pRet;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxAdjustItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (char)GetAdjust();
/*N*/ 	if ( nItemVersion >= ADJUST_LASTBLOCK_VERSION )
/*N*/ 	{
/*N*/ 		sal_Int8 nFlags = 0;
/*N*/ 		if ( bOneBlock )
/*N*/ 			nFlags |= 0x0001;
/*N*/ 		if ( bLastCenter )
/*N*/ 			nFlags |= 0x0002;
/*N*/ 		if ( bLastBlock )
/*N*/ 			nFlags |= 0x0004;
/*N*/ 		rStrm << (sal_Int8) nFlags;
/*N*/ 	}
/*N*/ 	return rStrm;
/*N*/ }

// class SvxWidowsItem ---------------------------------------------------

/*N*/ SvxWidowsItem::SvxWidowsItem(const BYTE nL, const USHORT nId ) :
/*N*/ 	SfxByteItem( nId, nL )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxWidowsItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxWidowsItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxWidowsItem::Create(SvStream& rStrm, sal_uInt16) const
/*N*/ {
/*N*/ 	sal_Int8 nLines;
/*N*/ 	rStrm >> nLines;
/*N*/ 	return new SvxWidowsItem( nLines, Which() );
/*N*/ }

// -----------------------------------------------------------------------


//------------------------------------------------------------------------


// class SvxOrphansItem --------------------------------------------------

/*N*/ SvxOrphansItem::SvxOrphansItem(const BYTE nL, const USHORT nId ) :
/*N*/ 	SfxByteItem( nId, nL )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxOrphansItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxOrphansItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxOrphansItem::Create(SvStream& rStrm, sal_uInt16) const
/*N*/ {
/*N*/ 	sal_Int8 nLines;
/*N*/ 	rStrm >> nLines;
/*N*/ 	return new SvxOrphansItem( nLines, Which() );
/*N*/ }

// -----------------------------------------------------------------------


//------------------------------------------------------------------------


// class SvxHyphenZoneItem -----------------------------------------------

/*N*/ SvxHyphenZoneItem::SvxHyphenZoneItem( const sal_Bool bHyph, const sal_uInt16 nId ) :
/*N*/ 	SfxPoolItem( nId )
/*N*/ {
/*N*/ 	bHyphen = bHyph;
/*N*/ 	bPageEnd = sal_True;
/*N*/ 	nMinLead = nMinTrail = 0;
/*N*/ 	nMaxHyphens = 255;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ bool	SvxHyphenZoneItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case  MID_IS_HYPHEN:
/*N*/ 			rVal = Bool2Any(bHyphen);
/*N*/ 		break;
/*N*/ 		case MID_HYPHEN_MIN_LEAD:
/*N*/ 			rVal <<= (sal_Int16)nMinLead;
/*N*/ 		break;
/*N*/ 		case MID_HYPHEN_MIN_TRAIL:
/*N*/ 			rVal <<= (sal_Int16)nMinTrail;
/*N*/ 		break;
/*N*/ 		case MID_HYPHEN_MAX_HYPHENS:
/*N*/ 			rVal <<= (sal_Int16)nMaxHyphens;
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
// -----------------------------------------------------------------------
/*N*/ bool SvxHyphenZoneItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	sal_Int16 nNewVal = 0;
/*N*/ 
/*N*/ 	if( nMemberId != MID_IS_HYPHEN )
/*N*/ 		if(!(rVal >>= nNewVal))
/*N*/ 			return sal_False;
/*N*/ 
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case  MID_IS_HYPHEN:
/*N*/ 			bHyphen = Any2Bool(rVal);
/*N*/ 		break;
/*N*/ 		case MID_HYPHEN_MIN_LEAD:
/*N*/ 			nMinLead = (BYTE)nNewVal;
/*N*/ 		break;
/*N*/ 		case MID_HYPHEN_MIN_TRAIL:
/*N*/ 			nMinTrail = (BYTE)nNewVal;
/*N*/ 		break;
/*N*/ 		case MID_HYPHEN_MAX_HYPHENS:
/*N*/ 			nMaxHyphens = (BYTE)nNewVal;
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxHyphenZoneItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/ 
/*N*/ 	return ( (((SvxHyphenZoneItem&)rAttr).bHyphen == bHyphen)
/*N*/ 			&& (((SvxHyphenZoneItem&)rAttr).bPageEnd == bPageEnd)
/*N*/ 			&& (((SvxHyphenZoneItem&)rAttr).nMinLead == nMinLead)
/*N*/ 			&& (((SvxHyphenZoneItem&)rAttr).nMinTrail == nMinTrail)
/*N*/ 			&& (((SvxHyphenZoneItem&)rAttr).nMaxHyphens == nMaxHyphens) );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxHyphenZoneItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxHyphenZoneItem( *this );
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxHyphenZoneItem::Create(SvStream& rStrm, sal_uInt16) const
/*N*/ {
/*N*/ 	sal_Int8 bHyphen, bHyphenPageEnd;
/*N*/ 	sal_Int8 nMinLead, nMinTrail, nMaxHyphens;
/*N*/ 	rStrm >> bHyphen >> bHyphenPageEnd >> nMinLead >> nMinTrail >> nMaxHyphens;
/*N*/ 	SvxHyphenZoneItem* pAttr = new SvxHyphenZoneItem( sal_False, Which() );
/*N*/ 	pAttr->SetHyphen( sal_Bool( bHyphen != 0 ) );
/*N*/ 	pAttr->SetPageEnd( sal_Bool( bHyphenPageEnd != 0 ) );
/*N*/ 	pAttr->GetMinLead() = nMinLead;
/*N*/ 	pAttr->GetMinTrail() = nMinTrail;
/*N*/ 	pAttr->GetMaxHyphens() = nMaxHyphens;
/*N*/ 	return pAttr;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxHyphenZoneItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_Int8) IsHyphen()
/*N*/ 		  << (sal_Int8) IsPageEnd()
/*N*/ 		  << (sal_Int8) GetMinLead()
/*N*/ 		  << (sal_Int8) GetMinTrail()
/*N*/ 		  << (sal_Int8) GetMaxHyphens();
/*N*/ 	return rStrm;
/*N*/ }

// class SvxTabStop ------------------------------------------------------

/*N*/ SvxTabStop::SvxTabStop()
/*N*/ {
/*N*/ 	nTabPos = 0;
/*N*/ 	eAdjustment = SVX_TAB_ADJUST_LEFT;
/*N*/ 	cDecimal = SvtSysLocale().GetLocaleData().getNumDecimalSep().GetChar(0);
/*N*/ 	cFill = cDfltFillChar;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxTabStop::SvxTabStop( const long nPos, const SvxTabAdjust eAdjst,
/*N*/ 						const sal_Unicode cDec, const sal_Unicode cFil )
/*N*/ {
/*N*/ 	nTabPos = nPos;
/*N*/ 	eAdjustment = eAdjst;
/*N*/ 	cDecimal = ( cDfltDecimalChar == cDec ) ? SvtSysLocale().GetLocaleData().getNumDecimalSep().GetChar(0) : cDec;
/*N*/ 	cFill = cFil;
/*N*/ }

// -----------------------------------------------------------------------


// class SvxTabStopItem --------------------------------------------------

/*N*/ SvxTabStopItem::SvxTabStopItem( sal_uInt16 nWhich ) :
/*N*/ 	SfxPoolItem( nWhich ),
/*N*/ 	SvxTabStopArr( sal_Int8(SVX_TAB_DEFCOUNT) )
/*N*/ {
/*N*/ 	const sal_uInt16 nTabs = SVX_TAB_DEFCOUNT, nDist = SVX_TAB_DEFDIST;
/*N*/ 	const SvxTabAdjust eAdjst= SVX_TAB_ADJUST_DEFAULT;
/*N*/ 
/*N*/ 	for (sal_uInt16 i = 0; i < nTabs; ++i)
/*N*/ 	{
/*N*/ 		SvxTabStop * pTab = new SvxTabStop( (i + 1) * nDist, eAdjst );
/*N*/ 		SvxTabStopArr::Insert( pTab, 1 );
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxTabStopItem::SvxTabStopItem( const sal_uInt16 nTabs,
/*N*/ 								const sal_uInt16 nDist,
/*N*/ 								const SvxTabAdjust eAdjst,
/*N*/ 								sal_uInt16 nWhich ) :
/*N*/ 	SfxPoolItem( nWhich ),
/*N*/ 	SvxTabStopArr( sal_Int8(nTabs) )
/*N*/ {
/*N*/ 	for ( sal_uInt16 i = 0; i < nTabs; ++i )
/*N*/ 	{
/*N*/ 		SvxTabStop * pTab = new SvxTabStop( (i + 1) * nDist, eAdjst );
/*N*/ 		SvxTabStopArr::Insert( pTab, 1 );
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxTabStopItem::SvxTabStopItem( const SvxTabStopItem& rTSI ) :
/*N*/ 	SfxPoolItem( rTSI.Which() ),
/*N*/ 	SvxTabStopArr( (sal_Int8)rTSI.Count() )
/*N*/ {
/*N*/ 	SvxTabStopArr::Insert( &rTSI );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_uInt16 SvxTabStopItem::GetPos( const SvxTabStop& rTab ) const
/*N*/ {
/*N*/ 	sal_uInt16 nFound;
/*N*/ 	return Seek_Entry( rTab, &nFound ) ? nFound : SVX_TAB_NOTFOUND;
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------



/*
 enum ::com::sun::star::style::TabAlign
{
    TABALIGN_LEFT,
    TABALIGN_CENTER,
    TABALIGN_RIGHT,
    TABALIGN_DECIMAL
};

struct	 ::com::sun::star::style::TabStop
{
    long			Position;
 ::com::sun::star::style::TabAlign 	 ::com::sun::star::drawing::Alignment;
    unsigned short 	DecimalChar;
    unsigned short 	FillChar;
};
typedef sequence ::com::sun::star::style::TabStop> TabSTopSequence;

 */
/*-----------------19.03.98 08:50-------------------

--------------------------------------------------*/

/*N*/ bool SvxTabStopItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/     switch ( nMemberId )
/*N*/     {
/*N*/         case MID_TABSTOPS:
/*N*/         {
/*N*/             sal_uInt16 nCount = Count();
/*N*/             uno::Sequence< style::TabStop> aSeq(nCount);
/*N*/             style::TabStop* pArr = aSeq.getArray();
/*N*/             for(sal_uInt16 i = 0; i < nCount; i++)
/*N*/             {
/*N*/                 const SvxTabStop& rTab = *(GetStart() + i);
/*N*/                 pArr[i].Position        = bConvert ? TWIP_TO_MM100(rTab.GetTabPos()) : rTab.GetTabPos();
/*N*/                 switch(rTab.GetAdjustment())
/*N*/                 {
/*N*/                 case  SVX_TAB_ADJUST_LEFT   : pArr[i].Alignment = style::TabAlign_LEFT; break;
/*N*/                 case  SVX_TAB_ADJUST_RIGHT  : pArr[i].Alignment = style::TabAlign_RIGHT; break;
/*N*/                 case  SVX_TAB_ADJUST_DECIMAL: pArr[i].Alignment = style::TabAlign_DECIMAL; break;
/*N*/                 case  SVX_TAB_ADJUST_CENTER : pArr[i].Alignment = style::TabAlign_CENTER; break;
/*N*/                     default: //SVX_TAB_ADJUST_DEFAULT
/*N*/                         pArr[i].Alignment = style::TabAlign_DEFAULT;
/*N*/ 
/*N*/                 }
/*N*/                 pArr[i].DecimalChar		= rTab.GetDecimal();
/*N*/                 pArr[i].FillChar		= rTab.GetFill();
/*N*/             }
/*N*/ 	        rVal <<= aSeq;
/*N*/             break;
/*N*/         }
/*N*/         case MID_STD_TAB:
/*N*/         {
/*N*/             const SvxTabStop &rTab = *(GetStart());
/*N*/             rVal <<= bConvert ? TWIP_TO_MM100(rTab.GetTabPos()) : rTab.GetTabPos();
/*N*/             break;
/*N*/         }
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------19.03.98 08:50-------------------

--------------------------------------------------*/

/*N*/ bool SvxTabStopItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/     switch ( nMemberId )
/*N*/     {
/*N*/         case MID_TABSTOPS:
/*N*/         {
/*N*/             uno::Sequence< style::TabStop> aSeq;
/*N*/             if(!(rVal >>= aSeq))
/*N*/             {
/*?*/                 uno::Sequence < uno::Sequence < uno::Any >  > aAnySeq;
/*?*/                 if (!(rVal >>= aAnySeq))
/*?*/                     return sal_False;
/*?*/                 sal_Int32 nLength = aAnySeq.getLength();
/*?*/                 aSeq.realloc( nLength );
/*?*/                 for ( sal_Int32 n=0; n<nLength; n++ )
/*?*/                 {
/*?*/                     uno::Sequence < uno::Any >& rAnySeq = aAnySeq[n];
/*?*/                     if ( rAnySeq.getLength() == 4 )
/*?*/                     {
/*?*/                         if (!(rAnySeq[0] >>= aSeq[n].Position)) return sal_False;
/*?*/                         if (!(rAnySeq[1] >>= aSeq[n].Alignment))
/*?*/                         {
/*?*/                             sal_Int32 nVal;
/*?*/                             if (rAnySeq[1] >>= nVal)
/*?*/                                 aSeq[n].Alignment = (::com::sun::star::style::TabAlign) nVal;
/*?*/                             else
/*?*/                                 return sal_False;
/*?*/                         }
/*?*/                         if (!(rAnySeq[2] >>= aSeq[n].DecimalChar))
/*?*/                         {
/*?*/                             ::rtl::OUString aVal;
/*?*/                             if ( (rAnySeq[2] >>= aVal) && aVal.getLength() == 1 )
/*?*/                                 aSeq[n].DecimalChar = aVal.toChar();
/*?*/                             else
/*?*/                                 return sal_False;
/*?*/                         }
/*?*/                         if (!(rAnySeq[3] >>= aSeq[n].FillChar))
/*?*/                         {
/*?*/                             ::rtl::OUString aVal;
/*?*/                             if ( (rAnySeq[3] >>= aVal) && aVal.getLength() == 1 )
/*?*/                                 aSeq[n].FillChar = aVal.toChar();
/*?*/                             else
/*?*/                                 return sal_False;
/*?*/                         }
/*?*/                     }
/*?*/                     else
/*?*/                         return sal_False;
/*?*/                 }
/*N*/             }
/*N*/ 
/*N*/             SvxTabStopArr::Remove( 0, Count() );
/*N*/             const style::TabStop* pArr = aSeq.getConstArray();
/*N*/             const sal_uInt16 nCount = (sal_uInt16)aSeq.getLength();
/*N*/             for(sal_uInt16 i = 0; i < nCount ; i++)
/*N*/             {
/*N*/                 SvxTabAdjust eAdjust = SVX_TAB_ADJUST_DEFAULT;
/*N*/                 switch(pArr[i].Alignment)
/*N*/                 {
/*N*/                 case style::TabAlign_LEFT   : eAdjust = SVX_TAB_ADJUST_LEFT; break;
/*N*/                 case style::TabAlign_CENTER : eAdjust = SVX_TAB_ADJUST_CENTER; break;
/*N*/                 case style::TabAlign_RIGHT  : eAdjust = SVX_TAB_ADJUST_RIGHT; break;
/*N*/                 case style::TabAlign_DECIMAL: eAdjust = SVX_TAB_ADJUST_DECIMAL; break;
/*N*/                 }
/*N*/                 sal_Unicode cFill = pArr[i].FillChar;
/*N*/                 sal_Unicode cDecimal = pArr[i].DecimalChar;
/*N*/                 SvxTabStop aTab( bConvert ? MM100_TO_TWIP(pArr[i].Position) : pArr[i].Position,
/*N*/                                     eAdjust,
/*N*/                                     cDecimal,
/*N*/                                     cFill );
/*N*/                 Insert(aTab);
/*N*/             }
/*N*/             break;
/*N*/         }
/*N*/         case MID_STD_TAB:
/*N*/         {
/*N*/             sal_Int32 nNewPos;
/*N*/             if (!(rVal >>= nNewPos) )
/*N*/                 return sal_False;
/*N*/             const SvxTabStop& rTab = *(GetStart());
/*N*/             SvxTabStop aNewTab ( bConvert ? MM100_TO_TWIP ( nNewPos ) : nNewPos,
/*N*/                                  rTab.GetAdjustment(), rTab.GetDecimal(), rTab.GetFill() );
/*N*/             Remove ( 0 );
/*N*/ 	        Insert( aNewTab );
/*N*/             break;
/*N*/         }
/*N*/     }
/*N*/ 	return sal_True;
/*N*/ }
// -----------------------------------------------------------------------

/*N*/ int SvxTabStopItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/ 
/*N*/ 	const SvxTabStopItem& rTSI = (SvxTabStopItem&)rAttr;
/*N*/ 
/*N*/ 	if ( Count() != rTSI.Count() )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	for ( sal_uInt16 i = 0; i < Count(); ++i )
/*N*/ 		if( !(*this)[i].IsEqual( rTSI[i] ) )
/*N*/ 			return 0;
/*N*/ 	return 1;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxTabStopItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxTabStopItem( *this );
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxTabStopItem::Create( SvStream& rStrm, sal_uInt16 ) const
/*N*/ {
/*N*/ 	sal_Int8 nTabs;
/*N*/ 	rStrm >> nTabs;
/*N*/ 	SvxTabStopItem* pAttr =
/*N*/ 		new SvxTabStopItem( 0, 0, SVX_TAB_ADJUST_DEFAULT, Which() );
/*N*/ 
/*N*/ 	for ( sal_Int8 i = 0; i < nTabs; i++ )
/*N*/ 	{
/*N*/ 		long nPos;
/*N*/ 		sal_Int8 eAdjust;
/*N*/ 		unsigned char cDecimal, cFill;
/*N*/ 		rStrm >> nPos >> eAdjust >> cDecimal >> cFill;
/*N*/ 		if( !i || SVX_TAB_ADJUST_DEFAULT != eAdjust )
/*N*/ 			pAttr->Insert( SvxTabStop
/*N*/ 				( nPos, (SvxTabAdjust)eAdjust, sal_Unicode(cDecimal), sal_Unicode(cFill) ) );
/*N*/ 	}
/*N*/ 	return pAttr;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxTabStopItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	//MA 05. Sep. 96: Default-Tabs werden nur noch fuer das default-Attr
/*N*/ 	//expandiert. Fuer vollstaendige Rueckwaertskompatibilitaet (<=304)
/*N*/ 	//muessten alle Tabs expandiert werden, dass blaeht aber das File u.U.
/*N*/ 	//enorm auf.
/*N*/ 	//Alles nur SWG!
/*N*/ 
/*N*/ 	const SfxItemPool *pPool = SfxItemPool::GetStoringPool();
/*N*/ 	const FASTBOOL bStoreDefTabs = pPool
/*N*/ 		&& pPool->GetName().EqualsAscii("SWG")
/*N*/ 		&& binfilter::IsDefaultItem( this );
/*N*/ 
/*N*/ 	const short nTabs = Count();
/*N*/ 	sal_uInt16 	nCount = 0, nDefDist;
/*N*/ 	long nNew;
/*N*/ 
/*N*/ 	if( bStoreDefTabs )
/*N*/ 	{
/*N*/ 		const SvxTabStopItem& rDefTab = (const SvxTabStopItem &)
/*N*/ 			pPool->GetDefaultItem( pPool->GetWhich(	SID_ATTR_TABSTOP, sal_False ) );
/*N*/ 		nDefDist = sal_uInt16( rDefTab.GetStart()->GetTabPos() );
/*N*/ 		const long nPos = nTabs > 0 ? (*this)[nTabs-1].GetTabPos() : 0;
/*N*/ 		nCount 	= (sal_uInt16)(nPos / nDefDist);
/*N*/ 		nNew	= (nCount + 1) * nDefDist;
/*N*/ 
/*N*/ 		if( nNew <= nPos + 50 )
/*N*/ 			nNew += nDefDist;
/*N*/ 
/*N*/ 		nCount = nNew < lA3Width ? ( lA3Width - nNew ) / nDefDist + 1 : 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	rStrm << (sal_Int8) ( nTabs + nCount );
/*N*/ 	for ( short i = 0; i < nTabs; i++ )
/*N*/ 	{
/*N*/ 		const SvxTabStop& rTab = (*this)[ i ];
/*N*/ 		rStrm << (long) rTab.GetTabPos()
/*N*/ 			  << (sal_Int8) rTab.GetAdjustment()
/*N*/ 			  << (unsigned char) rTab.GetDecimal()
/*N*/ 			  << (unsigned char) rTab.GetFill();
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bStoreDefTabs )
/*N*/ 		for( ; nCount; --nCount )
/*N*/ 		{
/*N*/ 			SvxTabStop aSwTabStop(nNew, SVX_TAB_ADJUST_DEFAULT);
/*N*/ 			rStrm << (long) aSwTabStop.GetTabPos()
/*N*/ 				  << (sal_Int8) aSwTabStop.GetAdjustment()
/*N*/ 				  << (unsigned char) aSwTabStop.GetDecimal()
/*N*/ 				  << (unsigned char) aSwTabStop.GetFill();
/*N*/ 			nNew += nDefDist;
/*N*/ 		}
/*N*/ 
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ sal_Bool SvxTabStopItem::Insert( const SvxTabStop& rTab )
/*N*/ {
/*N*/ 	sal_uInt16 nTabPos = GetPos(rTab);
/*N*/ 	if(SVX_TAB_NOTFOUND != nTabPos )
/*?*/ 		Remove(nTabPos);
/*N*/ 	return SvxTabStopArr::Insert( rTab );
/*N*/ }
// -----------------------------------------------------------------------



// class SvxFmtSplitItem -------------------------------------------------
/*N*/ SvxFmtSplitItem::~SvxFmtSplitItem()
/*N*/ {
/*N*/ }
// -----------------------------------------------------------------------
/*N*/ SfxPoolItem* SvxFmtSplitItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxFmtSplitItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxFmtSplitItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_Int8)GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ 
/*N*/ SfxPoolItem* SvxFmtSplitItem::Create( SvStream& rStrm, sal_uInt16 ) const
/*N*/ {
/*N*/ 	sal_Int8 bIsSplit;
/*N*/ 	rStrm >> bIsSplit;
/*N*/ 	return new SvxFmtSplitItem( sal_Bool( bIsSplit != 0 ), Which() );
/*N*/ }

//------------------------------------------------------------------------


// --------------------------------------------------------------------


//------------------------------------------------------------------------




//------------------------------------------------------------------------

/*N*/ SvxScriptSpaceItem::SvxScriptSpaceItem( sal_Bool bOn, const sal_uInt16 nId )
/*N*/ 	: SfxBoolItem( nId, bOn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* SvxScriptSpaceItem::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SvxScriptSpaceItem( GetValue(), Which() );
/*N*/ }

/*N*/ SfxPoolItem* SvxScriptSpaceItem::Create(SvStream & rStrm, USHORT) const
/*N*/ {
/*N*/ 	sal_Bool bFlag;
/*N*/ 	rStrm >> bFlag;
/*N*/ 	return new SvxScriptSpaceItem( bFlag, Which() );
/*N*/ }

/*N*/ USHORT	SvxScriptSpaceItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SvxTwoLinesItem: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }


//------------------------------------------------------------------------

/*N*/ SvxHangingPunctuationItem::SvxHangingPunctuationItem(
/*N*/ 									sal_Bool bOn, const sal_uInt16 nId )
/*N*/ 	: SfxBoolItem( nId, bOn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* SvxHangingPunctuationItem::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SvxHangingPunctuationItem( GetValue(), Which() );
/*N*/ }

/*N*/ SfxPoolItem* SvxHangingPunctuationItem::Create(SvStream & rStrm, USHORT) const
/*N*/ {
/*N*/ 	sal_Bool nValue;
/*N*/ 	rStrm >> nValue;
/*N*/ 	return new SvxHangingPunctuationItem( nValue, Which() );
/*N*/ }

/*N*/ USHORT SvxHangingPunctuationItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SvxHangingPunctuationItem: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SvxForbiddenRuleItem::SvxForbiddenRuleItem(
/*N*/ 									sal_Bool bOn, const sal_uInt16 nId )
/*N*/ 	: SfxBoolItem( nId, bOn )
/*N*/ {
/*N*/ }
/* -----------------------------29.11.00 11:23--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SfxPoolItem* SvxForbiddenRuleItem::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SvxForbiddenRuleItem( GetValue(), Which() );
/*N*/ }
/* -----------------------------29.11.00 11:23--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SfxPoolItem* SvxForbiddenRuleItem::Create(SvStream & rStrm, USHORT) const
/*N*/ {
/*N*/ 	sal_Bool nValue;
/*N*/ 	rStrm >> nValue;
/*N*/ 	return new SvxForbiddenRuleItem( nValue, Which() );
/*N*/ }
/* -----------------------------29.11.00 11:23--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ USHORT SvxForbiddenRuleItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SvxForbiddenRuleItem: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }
/* -----------------------------29.11.00 11:23--------------------------------

 ---------------------------------------------------------------------------*/

/*************************************************************************
|*    class SvxParaVertAlignItem
*************************************************************************/

/*N*/ SvxParaVertAlignItem::SvxParaVertAlignItem( sal_uInt16 nValue,
/*N*/ 	const sal_uInt16 nW )
/*N*/ 	: SfxUInt16Item( nW, nValue )
/*N*/ {
/*N*/ }




/*N*/ USHORT SvxParaVertAlignItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }






/*N*/ SvxParaGridItem::SvxParaGridItem( sal_Bool bOn, const sal_uInt16 nId )
/*N*/ 	: SfxBoolItem( nId, bOn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* SvxParaGridItem::Clone( SfxItemPool *pPool ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 
/*N*/ }

/*N*/ SfxPoolItem* SvxParaGridItem::Create(SvStream & rStrm, USHORT) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 
/*N*/ }

/*N*/ USHORT  SvxParaGridItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/             "SvxParaGridItem: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*N*/ SfxItemPresentation SvxParaGridItem::GetPresentation(
/*N*/ 		SfxItemPresentation ePres,
/*N*/ 		SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
/*N*/         String &rText, const ::IntlWrapper* pIntl ) const
/*N*/ {
/*?*/ 	switch( ePres )
/*?*/ 	{
/*?*/ 	case SFX_ITEM_PRESENTATION_NONE:
/*?*/ 		rText.Erase();
/*?*/ 		break;
/*?*/ 	case SFX_ITEM_PRESENTATION_NAMELESS:
/*?*/ 	case SFX_ITEM_PRESENTATION_COMPLETE:
/*?*/ 		{
/*?*/             rText = GetValue() ?
/*?*/                     SVX_RESSTR( RID_SVXITEMS_PARASNAPTOGRID_ON ) :
/*?*/                     SVX_RESSTR( RID_SVXITEMS_PARASNAPTOGRID_OFF );
/*?*/ 
/*?*/ 			return ePres;
/*?*/ 		}
/*?*/ 		break;
/*?*/ 	}
/*?*/ 	return SFX_ITEM_PRESENTATION_NONE;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
