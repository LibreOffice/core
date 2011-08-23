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


#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#include <toolkit/unohlp.hxx>
#include <math.h>
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#include <eeitem.hxx>

#ifdef _MSC_VER
#pragma hdrstop
#endif

#define ITEMID_FONTLIST	 		0
#define ITEMID_FONT	 	        0
#define ITEMID_POSTURE	 	    0
#define ITEMID_WEIGHT	 	    0
#define ITEMID_FONTHEIGHT	    0
#define ITEMID_UNDERLINE	    0
#define ITEMID_CROSSEDOUT	    0
#define ITEMID_SHADOWED	 	    0
#define ITEMID_AUTOKERN	 	    0
#define ITEMID_WORDLINEMODE     0
#define ITEMID_CONTOUR	 	    0
#define ITEMID_PROPSIZE	 	    0
#define ITEMID_COLOR	 	    0
#define ITEMID_CHARSETCOLOR     0
#define ITEMID_KERNING	 	    0
#define ITEMID_CASEMAP	 	    0
#define ITEMID_ESCAPEMENT	    0
#define ITEMID_LANGUAGE	 	    0
#define ITEMID_NOLINEBREAK	    0
#define ITEMID_NOHYPHENHERE     0
#define ITEMID_BLINK	 	    0
#define ITEMID_EMPHASISMARK	    0
#define ITEMID_TWOLINES	    	0
#define ITEMID_CHARROTATE		0
#define ITEMID_CHARSCALE_W		0
#define ITEMID_CHARRELIEF		0


#define GLOBALOVERFLOW3


#include <bf_svtools/memberid.hrc>
#include "svxids.hrc"
#include "svxitems.hrc"

//#ifndef NOOLDSV //autogen
//#include <vcl/system.hxx>
//#endif


#include "rtl/ustring.hxx"
#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <bf_svtools/itemset.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <bf_svtools/itempool.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_CASEMAP_HPP_
#include <com/sun/star/style/CaseMap.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFONT_HPP_
#include <com/sun/star/awt/XFont.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_FONTEMPHASIS_HPP_
#include <com/sun/star/text/FontEmphasis.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif



#include "flstitem.hxx"
#include "fontitem.hxx"
#include "postitem.hxx"
#include "wghtitem.hxx"
#include "fhgtitem.hxx"
#include "fwdtitem.hxx"
#include "udlnitem.hxx"
#include "crsditem.hxx"
#include "shdditem.hxx"
#include "akrnitem.hxx"
#include "wrlmitem.hxx"
#include "cntritem.hxx"
#include "prszitem.hxx"
#include "cscoitem.hxx"
#include "kernitem.hxx"
#include "cmapitem.hxx"
#include "escpitem.hxx"
#include "langitem.hxx"
#include "nlbkitem.hxx"
#include "nhypitem.hxx"
#include "blnkitem.hxx"
#include "emphitem.hxx"
#include "twolinesitem.hxx"
#include "scripttypeitem.hxx"
#include "charrotateitem.hxx"
#include "charscaleitem.hxx"
#include "charreliefitem.hxx"
#include "itemtype.hxx"
#include "dlgutil.hxx"

// #90477#
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif
namespace binfilter {

#define STORE_UNICODE_MAGIC_MARKER  0xFE331188

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;

// Konvertierung fuer UNO
#define TWIP_TO_MM100(TWIP) 	((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)	((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

BOOL SvxFontItem::bEnableStoreUnicodeNames = FALSE;

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

/*N*/ TYPEINIT1(SvxFontListItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxFontItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxPostureItem, SfxEnumItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxWeightItem, SfxEnumItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxFontHeightItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxUnderlineItem, SfxEnumItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxCrossedOutItem, SfxEnumItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxShadowedItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxAutoKernItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxWordLineModeItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxContourItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxPropSizeItem, SfxUInt16Item);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxColorItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxCharSetColorItem, SvxColorItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxKerningItem, SfxInt16Item);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxCaseMapItem, SfxEnumItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxEscapementItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxLanguageItem, SfxEnumItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxNoLinebreakItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxNoHyphenItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxBlinkItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxEmphasisMarkItem, SfxUInt16Item);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxTwoLinesItem, SfxPoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxCharRotateItem, SfxUInt16Item);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxCharScaleWidthItem, SfxUInt16Item);
/*N*/ TYPEINIT1_AUTOFACTORY(SvxCharReliefItem, SfxEnumItem);


/*N*/ TYPEINIT1(SvxScriptSetItem, SfxSetItem );


// class SvxFontListItem -------------------------------------------------

/*N*/ SvxFontListItem::SvxFontListItem( const FontList* pFontLst,
/*N*/ 								  const USHORT nId ) :
/*N*/ 	SfxPoolItem( nId ),
/*N*/ 	pFontList( pFontLst )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxFontListItem::SvxFontListItem( const SvxFontListItem& rItem ) :
/*N*/ 
/*N*/ 	SfxPoolItem( rItem ),
/*N*/ 	pFontList( rItem.GetFontList() )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxFontListItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxFontListItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*?*/ int SvxFontListItem::operator==( const SfxPoolItem& rAttr ) const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*?*/ }

//------------------------------------------------------------------------


// class SvxFontItem -----------------------------------------------------

/*N*/ SvxFontItem::SvxFontItem( const USHORT nId ) :
/*N*/ 	SfxPoolItem( nId )
/*N*/ {
/*N*/ 	eFamily = FAMILY_SWISS;
/*N*/ 	ePitch = PITCH_VARIABLE;
/*N*/ 	eTextEncoding = RTL_TEXTENCODING_DONTKNOW;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxFontItem::SvxFontItem( const FontFamily eFam, const XubString& aName,
/*N*/ 				  const XubString& aStName, const FontPitch eFontPitch,
/*N*/ 				  const rtl_TextEncoding eFontTextEncoding, const USHORT nId ) :
/*N*/ 
/*N*/ 	SfxPoolItem( nId ),
/*N*/ 
/*N*/ 	aFamilyName(aName),
/*N*/ 	aStyleName(aStName)
/*N*/ {
/*N*/ 	eFamily = eFam;
/*N*/ 	ePitch = eFontPitch;
/*N*/ 	eTextEncoding = eFontTextEncoding;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_Bool SvxFontItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case MID_FONT_FAMILY_NAME	:
/*N*/ 			rVal <<= OUString(aFamilyName.GetBuffer());
/*N*/ 		break;
/*N*/ 		case MID_FONT_STYLE_NAME:
/*N*/ 			rVal <<= OUString(aStyleName.GetBuffer());
/*N*/ 		break;
/*N*/ 		case MID_FONT_FAMILY    : rVal <<= (sal_Int16)(eFamily);	break;
/*N*/ 		case MID_FONT_CHAR_SET  : rVal <<= (sal_Int16)(eTextEncoding);	break;
/*N*/ 		case MID_FONT_PITCH     : rVal <<= (sal_Int16)(ePitch);	break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
// -----------------------------------------------------------------------
/*N*/ sal_Bool SvxFontItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case MID_FONT_FAMILY_NAME	:
/*N*/ 		{
/*N*/ 			OUString aStr;
/*N*/ 			if(!(rVal >>= aStr))
/*N*/ 				return sal_False;
/*N*/ 			aFamilyName = aStr.getStr();
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_FONT_STYLE_NAME:
/*N*/ 		{
/*N*/ 			OUString aStr;
/*N*/ 			if(!(rVal >>= aStr))
/*N*/ 				return sal_False;
/*N*/ 			aStyleName = aStr.getStr();
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_FONT_FAMILY :
/*N*/ 		{
/*N*/ 			sal_Int16 nFamily;
/*N*/ 			if(!(rVal >>= nFamily))
/*N*/ 				return sal_False;
/*N*/ 			eFamily = (FontFamily)nFamily;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_FONT_CHAR_SET  :
/*N*/ 		{
/*N*/ 			sal_Int16 nSet;
/*N*/ 			if(!(rVal >>= nSet))
/*N*/ 				return sal_False;
/*N*/ 			eTextEncoding = (rtl_TextEncoding)nSet;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_FONT_PITCH     :
/*N*/ 		{
/*N*/ 			sal_Int16 nPitch;
/*N*/ 			if(!(rVal >>= nPitch))
/*N*/ 				return sal_False;
/*N*/ 			ePitch =  (FontPitch)nPitch;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxFontItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/ 
/*N*/ 	const SvxFontItem& rItem = (const SvxFontItem&)rAttr;
/*N*/ 
/*N*/ 	int bRet = ( eFamily == rItem.eFamily &&
/*N*/ 				 aFamilyName == rItem.aFamilyName &&
/*N*/ 				 aStyleName == rItem.aStyleName );
/*N*/ 
/*N*/ 	if ( bRet )
/*N*/ 	{
/*N*/ 		if ( ePitch != rItem.ePitch || eTextEncoding != rItem.eTextEncoding )
/*N*/ 		{
/*N*/ 			bRet = sal_False;
/*N*/ 			DBG_WARNING( "FontItem::operator==(): nur Pitch oder rtl_TextEncoding unterschiedlich" );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxFontItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxFontItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxFontItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	BOOL bToBats =
/*N*/ 		GetFamilyName().EqualsAscii( "StarSymbol", 0, sizeof("StarSymbol")-1 ) ||
/*N*/ 		GetFamilyName().EqualsAscii( "OpenSymbol", 0, sizeof("OpenSymbol")-1 );
/*N*/ 
/*N*/ 	// #90477# rStrm << (BYTE) GetFamily()
/*N*/ 	//	  << (BYTE) GetPitch()
/*N*/ 	//	  << (BYTE)(bToBats ? RTL_TEXTENCODING_SYMBOL : GetStoreCharSet( GetCharSet(), (USHORT)rStrm.GetVersion() ) );
/*N*/ 	rStrm << (BYTE) GetFamily() << (BYTE) GetPitch()
/*N*/ 		  << (BYTE)(bToBats ? RTL_TEXTENCODING_SYMBOL : GetSOStoreTextEncoding(GetCharSet(), (sal_uInt16)rStrm.GetVersion()));
/*N*/ 
/*N*/     String aStoreFamilyName( GetFamilyName() );
/*N*/     if( bToBats )
/*N*/ 		aStoreFamilyName = String( "StarBats", sizeof("StarBats")-1, RTL_TEXTENCODING_ASCII_US );
/*N*/     rStrm.WriteByteString(aStoreFamilyName);
/*N*/ 	rStrm.WriteByteString(GetStyleName());
/*N*/ 
/*N*/     // #96441# Kach for EditEngine, only set while creating clipboard stream.
/*N*/     if ( bEnableStoreUnicodeNames )
/*N*/     {
/*N*/         sal_uInt32 nMagic = STORE_UNICODE_MAGIC_MARKER;
/*N*/         rStrm << nMagic;
/*N*/         rStrm.WriteByteString( aStoreFamilyName, RTL_TEXTENCODING_UNICODE );
/*N*/ 	    rStrm.WriteByteString( GetStyleName(), RTL_TEXTENCODING_UNICODE );
/*N*/     }
/*N*/ 
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxFontItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	BYTE eFamily, eFontPitch, eFontTextEncoding;
/*N*/ 	String aName, aStyle;
/*N*/ 	rStrm >> eFamily;
/*N*/ 	rStrm >> eFontPitch;
/*N*/ 	rStrm >> eFontTextEncoding;
/*N*/ 
/*N*/ 	// UNICODE: rStrm >> aName;
/*N*/ 	rStrm.ReadByteString(aName);
/*N*/ 
/*N*/ 	// UNICODE: rStrm >> aStyle;
/*N*/ 	rStrm.ReadByteString(aStyle);
/*N*/ 
/*N*/ 	// Task 91008/90471: set the "correct" textencoding
/*N*/ 	eFontTextEncoding = (BYTE)GetSOLoadTextEncoding( eFontTextEncoding, (USHORT)rStrm.GetVersion() );
/*N*/ 
/*N*/ 	// irgendwann wandelte sich der StarBats vom ANSI- zum SYMBOL-Font
/*N*/ 	if ( RTL_TEXTENCODING_SYMBOL != eFontTextEncoding && aName.EqualsAscii("StarBats") )
/*N*/ 		eFontTextEncoding = RTL_TEXTENCODING_SYMBOL;
/*N*/ 
/*N*/     // Check if we have stored unicode
/*N*/     ULONG nStreamPos = rStrm.Tell();
/*N*/     sal_uInt32 nMagic = STORE_UNICODE_MAGIC_MARKER;
/*N*/     rStrm >> nMagic;
/*N*/     if ( nMagic == STORE_UNICODE_MAGIC_MARKER )
/*N*/     {
/*N*/         rStrm.ReadByteString( aName, RTL_TEXTENCODING_UNICODE );
/*N*/ 	    rStrm.ReadByteString( aStyle, RTL_TEXTENCODING_UNICODE );
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         rStrm.Seek( nStreamPos );
/*N*/     }
/*N*/ 
/*N*/ 
/*N*/ 
/*N*/ 	return new SvxFontItem( (FontFamily)eFamily, aName, aStyle,
/*N*/ 							(FontPitch)eFontPitch, (rtl_TextEncoding)eFontTextEncoding,	Which() );
/*N*/ }

//------------------------------------------------------------------------


// class SvxPostureItem --------------------------------------------------

/*N*/ SvxPostureItem::SvxPostureItem( const FontItalic ePosture, const USHORT nId ) :
/*N*/ 	SfxEnumItem( nId, ePosture )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxPostureItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxPostureItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*?*/ USHORT SvxPostureItem::GetValueCount() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 /*?*/ 	return ITALIC_NORMAL + 1;	// auch ITALIC_NONE geh"ort dazu
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxPostureItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (BYTE)GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxPostureItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	BYTE nPosture;
/*N*/ 	rStrm >> nPosture;
/*N*/ 	return new SvxPostureItem( (const FontItalic)nPosture, Which() );
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------



/*-----------------13.03.98 14:28-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxPostureItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_ITALIC:
/*?*/ 			rVal = Bool2Any(GetBoolValue());
/*N*/ 			break;
/*N*/ 		case MID_POSTURE:
/*N*/ 			rVal <<= (awt::FontSlant)GetValue();	// Werte von awt::FontSlant und FontItalic sind gleich
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------13.03.98 14:28-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxPostureItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_ITALIC:
/*?*/ 			SetBoolValue(Any2Bool(rVal));
/*N*/ 		break;
/*N*/ 		case MID_POSTURE:
/*N*/ 		{
/*?*/ 			awt::FontSlant eSlant;
/*?*/ 			if(!(rVal >>= eSlant))
/*?*/ 			{
/*?*/ 				sal_Int32 nValue;
/*?*/ 				if(!(rVal >>= nValue))
/*?*/ 					return sal_False;
/*?*/ 
/*?*/ 				eSlant = (awt::FontSlant)nValue;
/*N*/ 			}
/*N*/ 			SetValue((USHORT)eSlant);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
// -----------------------------------------------------------------------

/*N*/ int SvxPostureItem::HasBoolValue() const
/*N*/ {
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_Bool SvxPostureItem::GetBoolValue() const
/*N*/ {
/*N*/ 	return ( (FontItalic)GetValue() >= ITALIC_OBLIQUE );
/*N*/ }

// -----------------------------------------------------------------------


// class SvxWeightItem ---------------------------------------------------

/*N*/ SvxWeightItem::SvxWeightItem( const FontWeight eWght, const USHORT nId ) :
/*N*/ 	SfxEnumItem( nId, eWght )
/*N*/ {
/*N*/ }



// -----------------------------------------------------------------------

/*N*/ int SvxWeightItem::HasBoolValue() const
/*N*/ {
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_Bool SvxWeightItem::GetBoolValue() const
/*N*/ {
/*N*/ 	return  (FontWeight)GetValue() >= WEIGHT_BOLD;
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*?*/ USHORT SvxWeightItem::GetValueCount() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return WEIGHT_BLACK;	// WEIGHT_DONTKNOW geh"ort nicht dazu
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxWeightItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxWeightItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxWeightItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (BYTE)GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxWeightItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	BYTE nWeight;
/*N*/ 	rStrm >> nWeight;
/*N*/ 	return new SvxWeightItem( (FontWeight)nWeight, Which() );
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------


/*-----------------13.03.98 14:18-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxWeightItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_BOLD	:
/*?*/ 			rVal = Bool2Any(GetBoolValue());
/*N*/ 		break;
/*N*/ 		case MID_WEIGHT:
/*N*/ 		{
/*N*/ 			rVal <<= (float)( VCLUnoHelper::ConvertFontWeight( (FontWeight)GetValue() ) );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------13.03.98 14:18-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxWeightItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_BOLD	:
/*?*/ 			SetBoolValue(Any2Bool(rVal));
/*N*/ 		break;
/*N*/ 		case MID_WEIGHT:
/*N*/ 		{
/*N*/             double fValue;
/*N*/ 			if(!(rVal >>= fValue))
/*N*/ 			{
/*?*/ 				sal_Int32 nValue;
/*?*/ 				if(!(rVal >>= nValue))
/*?*/ 					return sal_False;
/*?*/ 				fValue = (float)nValue;
/*N*/ 			}
/*N*/ 			SetValue( VCLUnoHelper::ConvertFontWeight((float)fValue) );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

// class SvxFontHeightItem -----------------------------------------------

/*N*/ SvxFontHeightItem::SvxFontHeightItem( const ULONG nSz,
/*N*/ 									  const USHORT nPrp,
/*N*/ 									  const USHORT nId ) :
/*N*/ 	SfxPoolItem( nId )
/*N*/ {
/*N*/ 	SetHeight( nSz,nPrp );	// mit den Prozenten rechnen
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxFontHeightItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxFontHeightItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxFontHeightItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (USHORT)GetHeight();
/*N*/ 
/*N*/ 	if( FONTHEIGHT_UNIT_VERSION <= nItemVersion )
/*N*/ 		rStrm << GetProp() << (USHORT)GetPropUnit();
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// JP 30.06.98: beim Export in alte Versionen geht die relative
/*N*/ 		// Angabe verloren, wenn es keine Prozentuale ist
/*N*/ 		USHORT nProp = GetProp();
/*N*/ 		if( SFX_MAPUNIT_RELATIVE != GetPropUnit() )
/*N*/ 			nProp = 100;
/*N*/ 		rStrm << nProp;
/*N*/ 	}
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxFontHeightItem::Create( SvStream& rStrm,
/*N*/ 												 USHORT nVersion ) const
/*N*/ {
/*N*/ 	USHORT nsize, nprop = 0, nPropUnit = SFX_MAPUNIT_RELATIVE;
/*N*/ 
/*N*/ 	rStrm >> nsize;
/*N*/ 
/*N*/ 	if( FONTHEIGHT_16_VERSION <= nVersion )
/*N*/ 		rStrm >> nprop;
/*N*/ 	else
/*N*/ 	{
/*?*/ 		BYTE nP;
/*?*/ 		rStrm  >> nP;
/*?*/ 		nprop = (USHORT)nP;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( FONTHEIGHT_UNIT_VERSION <= nVersion )
/*N*/ 		rStrm >> nPropUnit;
/*N*/ 
/*N*/ 	SvxFontHeightItem* pItem = new SvxFontHeightItem( nsize, 100, Which() );
/*N*/ 	pItem->SetProp( nprop, (SfxMapUnit)nPropUnit );
/*N*/ 	return pItem;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxFontHeightItem::operator==( const SfxPoolItem& rItem ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
/*N*/ 	return GetHeight() == ((SvxFontHeightItem&)rItem).GetHeight() &&
/*N*/ 			GetProp() == ((SvxFontHeightItem&)rItem).GetProp() &&
/*N*/ 			GetPropUnit() == ((SvxFontHeightItem&)rItem).GetPropUnit();
/*N*/ }

/*-----------------13.03.98 14:53-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxFontHeightItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	//	In StarOne sind im uno::Any immer 1/100mm. Ueber die MemberId wird
/*N*/ 	//	gesteuert, ob der Wert im Item 1/100mm oder Twips sind.
/*N*/ 
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_FONTHEIGHT:
/*N*/ 		{
/*N*/ 			//	Point (also Twips) sind gefragt,
/*N*/ 			//	also umrechnen, wenn CONVERT_TWIPS nicht gesetzt ist
/*N*/ 			if( bConvert )
/*N*/ 			{
/*N*/ 				long nTwips = bConvert ? nHeight : MM100_TO_TWIP(nHeight);
/*N*/ 				rVal <<= (float)( nTwips / 20.0 );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				double fPoints = MM100_TO_TWIP((long)nHeight) / 20.0;
/*N*/                 float fRoundPoints =
/*N*/                     static_cast<float>(::rtl::math::round(fPoints, 1));
/*N*/ 				rVal <<= fRoundPoints;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_FONTHEIGHT_PROP:
/*N*/ 			rVal <<= (sal_Int16)(SFX_MAPUNIT_RELATIVE == ePropUnit ? nProp : 100);
/*N*/ 		break;
/*N*/ 		case MID_FONTHEIGHT_DIFF:
/*N*/ 		{
/*N*/ 			float fRet = (float)(short)nProp;
/*N*/ 			switch( ePropUnit )
/*N*/ 			{
/*N*/ 				case SFX_MAPUNIT_RELATIVE:
/*N*/ 					fRet = 0.;
/*N*/ 				break;
/*N*/ 				case SFX_MAPUNIT_100TH_MM:
/*?*/ 					fRet = MM100_TO_TWIP(fRet);
/*?*/ 					fRet /= 20.;
/*?*/ 				break;
/*?*/ 				case SFX_MAPUNIT_POINT:
/*?*/ 
/*?*/ 				break;
/*?*/ 				case SFX_MAPUNIT_TWIP:
/*?*/ 					fRet /= 20.;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			rVal <<= fRet;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/* -----------------01.07.98 13:43-------------------
 * 	Relative Abweichung aus der Hoehe herausrechnen
 * --------------------------------------------------*/
/*N*/ sal_uInt32 lcl_GetRealHeight_Impl(sal_uInt32 nHeight, sal_uInt16 nProp, SfxMapUnit eProp, sal_Bool bCoreInTwip)
/*N*/ {
/*N*/ 	sal_uInt32 nRet = nHeight;
/*N*/ 	short nDiff = 0;
/*N*/ 	switch( eProp )
/*N*/ 	{
/*N*/ 		case SFX_MAPUNIT_RELATIVE:
/*N*/ 			nRet *= 100;
/*N*/ 			nRet /= nProp;
/*N*/ 		break;
/*N*/ 		case SFX_MAPUNIT_POINT:
/*N*/ 		{
/*?*/ 			short nTemp = (short)nProp;
/*?*/ 			nDiff = nTemp * 20;
/*?*/ 			if(!bCoreInTwip)
/*?*/ 				nDiff = (short)TWIP_TO_MM100((long)(nDiff));
/*?*/ 		}
/*?*/ 		break;
/*?*/ 		case SFX_MAPUNIT_100TH_MM:
/*?*/ 			//dann ist die Core doch wohl auch in 1/100 mm
/*?*/ 			nDiff = (short)nProp;
/*?*/ 		break;
/*?*/ 		case SFX_MAPUNIT_TWIP:
/*?*/ 			// hier doch sicher TWIP
/*?*/ 			nDiff = ((short)nProp);
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	nRet -= nDiff;
/*N*/ 
/*N*/ 	return nRet;
/*N*/ }

/*-----------------13.03.98 14:53-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxFontHeightItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/ 	nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_FONTHEIGHT:
/*N*/ 		{
/*N*/ 			ePropUnit = SFX_MAPUNIT_RELATIVE;
/*N*/ 			nProp = 100;
/*N*/             double fPoint;
/*N*/ 			if(!(rVal >>= fPoint))
/*N*/ 			{
/*N*/ 				sal_Int32 nValue;
/*?*/                 if(!(rVal >>= nValue))
/*?*/ 					return sal_False;
/*?*/ 				fPoint = (float)nValue;
/*N*/ 			}
/*N*/             if(fPoint < 0. || fPoint > 10000.)
/*N*/ 					return sal_False;
/*N*/ 
/*N*/ 			nHeight = (long)( fPoint * 20.0 + 0.5 );		// Twips
/*N*/ 			if (!bConvert)
/*N*/ 				nHeight = TWIP_TO_MM100(nHeight);	// umrechnen, wenn das Item 1/100mm enthaelt
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_FONTHEIGHT_PROP:
/*N*/ 		{
/*N*/ 			sal_Int16 nNew;
/*N*/ 			if(!(rVal >>= nNew))
/*N*/ 				return sal_True;
/*N*/ 
/*N*/ 			nHeight = lcl_GetRealHeight_Impl(nHeight, nProp, ePropUnit, bConvert);
/*N*/ 
/*N*/ 			nHeight *= nNew;
/*N*/ 			nHeight /= 100;
/*N*/ 			nProp = nNew;
/*N*/ 			ePropUnit = SFX_MAPUNIT_RELATIVE;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_FONTHEIGHT_DIFF:
/*N*/ 		{
/*?*/ 			nHeight = lcl_GetRealHeight_Impl(nHeight, nProp, ePropUnit, bConvert);
/*?*/ 			float fValue;
/*?*/ 			if(!(rVal >>= fValue))
/*?*/ 			{
/*?*/ 				sal_Int32 nValue;
/*?*/ 				if(!(rVal >>= nValue))
/*?*/ 					return sal_False;
/*?*/ 				fValue = (float)nValue;
/*?*/ 			}
/*?*/ 			sal_Int16 nCoreDiffValue = (sal_Int16)(fValue * 20.);
/*?*/ 			nHeight += bConvert ? nCoreDiffValue : TWIP_TO_MM100(nCoreDiffValue);
/*?*/ 			nProp = (sal_uInt16)((sal_Int16)fValue);
/*?*/ 			ePropUnit = SFX_MAPUNIT_POINT;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ USHORT SvxFontHeightItem::GetVersion(USHORT nFileVersion) const
/*N*/ {
/*N*/ 	return (nFileVersion <= SOFFICE_FILEFORMAT_40)
/*N*/ 			   ? FONTHEIGHT_16_VERSION
/*N*/ 			   : FONTHEIGHT_UNIT_VERSION;
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


/*N*/ void SvxFontHeightItem::SetHeight( sal_uInt32 nNewHeight, const USHORT nNewProp,
/*N*/ 									SfxMapUnit eUnit )
/*N*/ {
/*N*/ 	DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if( SFX_MAPUNIT_RELATIVE != eUnit )
nHeight = nNewHeight + ::binfilter::ItemToControl( (short)nNewProp, eUnit,//STRIP008 /*N*/ 		nHeight = nNewHeight + ::ItemToControl( (short)nNewProp, eUnit,
/*N*/ 												SFX_FUNIT_TWIP );
/*N*/ 	else
/*N*/ #endif // !SVX_LIGHT
/*N*/ 	if( 100 != nNewProp )
/*N*/ 		nHeight = sal_uInt32(( nNewHeight * nNewProp ) / 100 );
/*N*/ 	else
/*N*/ 		nHeight = nNewHeight;
/*N*/ 
/*N*/ 	nProp = nNewProp;
/*N*/ 	ePropUnit = eUnit;
/*N*/ }

// class SvxUnderlineItem ------------------------------------------------

/*N*/ SvxUnderlineItem::SvxUnderlineItem( const FontUnderline eSt, const USHORT nId )
/*N*/ 	: SfxEnumItem( nId, eSt ), mColor( COL_TRANSPARENT )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxUnderlineItem::HasBoolValue() const
/*N*/ {
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_Bool SvxUnderlineItem::GetBoolValue() const
/*N*/ {
/*N*/ 	return  (FontUnderline)GetValue() != UNDERLINE_NONE;
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxUnderlineItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	SvxUnderlineItem* pNew = new SvxUnderlineItem( *this );
/*N*/ 	pNew->SetColor( GetColor() );
/*N*/ 	return pNew;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ USHORT SvxUnderlineItem::GetValueCount() const
/*N*/ {
/*N*/ 	return UNDERLINE_DOTTED + 1;	// auch UNDERLINE_NONE geh"ort dazu
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxUnderlineItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (BYTE)GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxUnderlineItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	BYTE nState;
/*N*/ 	rStrm >> nState;
/*N*/ 	return new SvxUnderlineItem(  (FontUnderline)nState, Which() );
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------


/*-----------------13.03.98 16:25-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxUnderlineItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 	case MID_UNDERLINED:
/*?*/ 		rVal = Bool2Any(GetBoolValue());
/*?*/ 		break;
/*N*/ 	case MID_UNDERLINE:
/*N*/ 		rVal <<= (sal_Int16)(GetValue());
/*N*/ 		break;
/*N*/ 	case MID_UL_COLOR:
/*N*/ 		rVal <<= (sal_Int32)( mColor.GetColor() );
/*N*/ 		break;
/*N*/ 	case MID_UL_HASCOLOR:
/*N*/ 		rVal = Bool2Any( !mColor.GetTransparency() );
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ 
/*N*/ }
/*-----------------13.03.98 16:28-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxUnderlineItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	sal_Bool bRet = sal_True;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 	case MID_UNDERLINED:
/*?*/ 		SetBoolValue(Any2Bool(rVal));
/*N*/ 	break;
/*N*/ 	case MID_UNDERLINE:
/*N*/ 	{
/*N*/ 		sal_Int32 nValue;
/*N*/ 		if(!(rVal >>= nValue))
/*N*/ 			bRet = sal_False;
/*N*/ 		else
/*N*/ 			SetValue((sal_Int16)nValue);
/*N*/ 	}
/*N*/ 	break;
/*N*/ 	case MID_UL_COLOR:
/*N*/ 	{
/*?*/ 		sal_Int32 nCol;
/*?*/ 		if( !( rVal >>= nCol ) )
/*?*/ 			bRet = sal_False;
/*?*/ 		else
/*?*/ 		{
/*?*/ 			// Keep transparence, because it contains the information
/*?*/ 			// whether the font color or the stored color should be used
/*?*/ 			sal_uInt8 nTrans = mColor.GetTransparency();
/*?*/ 			mColor = Color( nCol );
/*?*/ 			mColor.SetTransparency( nTrans );
/*?*/ 		}
/*N*/ 	}
/*N*/ 	break;
/*N*/ 	case MID_UL_HASCOLOR:
/*N*/ 		mColor.SetTransparency( Any2Bool( rVal ) ? 0 : 0xff );
/*N*/ 	break;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ int SvxUnderlineItem::operator==( const SfxPoolItem& rItem ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
/*N*/ 	return SfxEnumItem::operator==( rItem ) &&
/*N*/ 		   GetColor() == ((SvxUnderlineItem&)rItem).GetColor();
/*N*/ }

// class SvxCrossedOutItem -----------------------------------------------

/*N*/ SvxCrossedOutItem::SvxCrossedOutItem( const FontStrikeout eSt, const USHORT nId )
/*N*/ 	: SfxEnumItem( nId, eSt )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ USHORT SvxCrossedOutItem::GetValueCount() const
/*N*/ {
/*N*/ 	return STRIKEOUT_DOUBLE + 1;	// auch STRIKEOUT_NONE geh"ort dazu
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxCrossedOutItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxCrossedOutItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxCrossedOutItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (BYTE)GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxCrossedOutItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	BYTE eCross;
/*N*/ 	rStrm >> eCross;
/*N*/ 	return new SvxCrossedOutItem(  (FontStrikeout)eCross, Which() );
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------


/*-----------------13.03.98 16:28-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxCrossedOutItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case MID_CROSSED_OUT:
/*?*/ 			rVal = Bool2Any(GetBoolValue());
/*N*/ 		break;
/*N*/ 		case MID_CROSS_OUT:
/*N*/ 			rVal <<= (sal_Int16)(GetValue());
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------13.03.98 16:29-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxCrossedOutItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case MID_CROSSED_OUT:
/*?*/ 			SetBoolValue(Any2Bool(rVal));
/*N*/ 		break;
/*N*/ 		case MID_CROSS_OUT:
/*N*/ 		{
/*N*/ 			sal_Int32 nValue;
/*N*/ 			if(!(rVal >>= nValue))
/*N*/ 				return sal_False;
/*N*/ 			SetValue((sal_Int16)nValue);
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
// class SvxShadowedItem -------------------------------------------------

/*N*/ SvxShadowedItem::SvxShadowedItem( const sal_Bool bShadowed, const USHORT nId ) :
/*N*/ 	SfxBoolItem( nId, bShadowed )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxShadowedItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxShadowedItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxShadowedItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (BYTE) GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxShadowedItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	BYTE nState;
/*N*/ 	rStrm >> nState;
/*N*/ 	return new SvxShadowedItem( nState, Which() );
/*N*/ }

//------------------------------------------------------------------------


// class SvxAutoKernItem -------------------------------------------------

/*N*/ SvxAutoKernItem::SvxAutoKernItem( const sal_Bool bAutoKern, const USHORT nId ) :
/*N*/ 	SfxBoolItem( nId, bAutoKern )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxAutoKernItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxAutoKernItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxAutoKernItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (BYTE) GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxAutoKernItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	BYTE nState;
/*N*/ 	rStrm >> nState;
/*N*/ 	return new SvxAutoKernItem( nState, Which() );
/*N*/ }

//------------------------------------------------------------------------


// class SvxWordLineModeItem ---------------------------------------------

/*N*/ SvxWordLineModeItem::SvxWordLineModeItem( const sal_Bool bWordLineMode,
/*N*/ 										  const USHORT nId ) :
/*N*/ 	SfxBoolItem( nId, bWordLineMode )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxWordLineModeItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxWordLineModeItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxWordLineModeItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_Bool) GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxWordLineModeItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	sal_Bool bValue;
/*N*/ 	rStrm >> bValue;
/*N*/ 	return new SvxWordLineModeItem( bValue, Which() );
/*N*/ }

//------------------------------------------------------------------------


// class SvxContourItem --------------------------------------------------

/*N*/ SvxContourItem::SvxContourItem( const sal_Bool bContoured, const USHORT nId ) :
/*N*/ 	SfxBoolItem( nId, bContoured )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxContourItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxContourItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxContourItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_Bool) GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxContourItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	sal_Bool bValue;
/*N*/ 	rStrm >> bValue;
/*N*/ 	return new SvxContourItem( bValue, Which() );
/*N*/ }

//------------------------------------------------------------------------


// class SvxPropSizeItem -------------------------------------------------

/*N*/ SvxPropSizeItem::SvxPropSizeItem( const USHORT nPercent, const USHORT nId ) :
/*N*/ 	SfxUInt16Item( nId, nPercent )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxPropSizeItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxPropSizeItem( *this );
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxPropSizeItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	USHORT nSize;
/*N*/ 	rStrm >> nSize;
/*N*/ 	return new SvxPropSizeItem( nSize, Which() );
/*N*/ }

//------------------------------------------------------------------------


// class SvxColorItem ----------------------------------------------------

/*N*/ SvxColorItem::SvxColorItem( const USHORT nId ) :
/*N*/ 	SfxPoolItem( nId ),
/*N*/ 	mColor( COL_BLACK )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxColorItem::SvxColorItem( const Color& rCol, const USHORT nId ) :
/*N*/ 	SfxPoolItem( nId ),
/*N*/ 	mColor( rCol )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxColorItem::SvxColorItem( SvStream &rStrm, const USHORT nId ) :
/*N*/ 	SfxPoolItem( nId )
/*N*/ {
/*N*/ 	Color aColor;
/*N*/ 	rStrm >> aColor;
/*N*/ 	mColor = aColor;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxColorItem::SvxColorItem( const SvxColorItem &rCopy ) :
/*N*/ 	SfxPoolItem( rCopy ),
/*N*/ 	mColor( rCopy.mColor )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxColorItem::~SvxColorItem()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ USHORT SvxColorItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SvxColorItem: Gibt es ein neues Fileformat?" );
/*N*/ 	return  SOFFICE_FILEFORMAT_50 >= nFFVer ? VERSION_USEAUTOCOLOR : 0;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxColorItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/ 
/*N*/ 	return	mColor == ( (const SvxColorItem&)rAttr ).mColor;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_Bool SvxColorItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (sal_Int32)(mColor.GetColor());
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ sal_Bool SvxColorItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Int32 nColor;
/*N*/ 	if(!(rVal >>= nColor))
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	mColor.SetColor( nColor );
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxColorItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxColorItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxColorItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	if( VERSION_USEAUTOCOLOR == nItemVersion &&
/*N*/ 		COL_AUTO == mColor.GetColor() )
/*N*/ 		rStrm << Color( COL_BLACK );
/*N*/ 	else
/*N*/ 		rStrm << mColor;
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxColorItem::Create(SvStream& rStrm, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new SvxColorItem( rStrm, Which() );
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ void SvxColorItem::SetValue( const Color& rNewCol )
/*N*/ {
/*N*/ 	mColor = rNewCol;
/*N*/ }

// class SvxCharSetColorItem ---------------------------------------------

/*N*/ SvxCharSetColorItem::SvxCharSetColorItem( const USHORT nId ) :
/*N*/ 	SvxColorItem( nId ),
/*N*/ 
/*N*/ 	eFrom( RTL_TEXTENCODING_DONTKNOW )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxCharSetColorItem::SvxCharSetColorItem( const Color& rCol,
/*N*/ 										  const rtl_TextEncoding eFrom,
/*N*/ 										  const USHORT nId ) :
/*N*/ 	SvxColorItem( rCol, nId ),
/*N*/ 
/*N*/ 	eFrom( eFrom )
/*N*/ {
/*N*/ }


// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxCharSetColorItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxCharSetColorItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxCharSetColorItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	// #90477# rStrm << (BYTE) GetStoreCharSet( GetCharSet(), (USHORT)rStrm.GetVersion() )
/*N*/ 	//	  << GetValue();
/*N*/ 	rStrm << (BYTE)GetSOStoreTextEncoding(GetCharSet(), (sal_uInt16)rStrm.GetVersion())
/*N*/ 		  << GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxCharSetColorItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	BYTE cSet;
/*N*/ 	Color aColor;
/*N*/ 	rStrm >> cSet >> aColor;
/*N*/ 	return new SvxCharSetColorItem( aColor,  (rtl_TextEncoding)cSet, Which() );
/*N*/ }

//------------------------------------------------------------------------


// class SvxKerningItem --------------------------------------------------

/*N*/ SvxKerningItem::SvxKerningItem( const short nKern, const USHORT nId ) :
/*N*/ 	SfxInt16Item( nId, nKern )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxKerningItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxKerningItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxKerningItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (short) GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxKerningItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	short nValue;
/*N*/ 	rStrm >> nValue;
/*N*/ 	return new SvxKerningItem( nValue, Which() );
/*N*/ }

//------------------------------------------------------------------------

/* -----------------------------19.02.01 12:21--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ sal_Bool SvxKerningItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	sal_Int16 nVal = GetValue();
/*N*/ 	if(nMemberId & CONVERT_TWIPS)
/*N*/ 		nVal = (sal_Int16)TWIP_TO_MM100(nVal);
/*N*/ 	rVal <<= nVal;
/*N*/ 	return sal_True;
/*N*/ }
// -----------------------------------------------------------------------
/*N*/ sal_Bool SvxKerningItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
/*N*/ {
/*N*/ 	sal_Int16 nVal;
/*N*/ 	if(!(rVal >>= nVal))
/*N*/ 		return sal_False;
/*N*/ 	if(nMemberId & CONVERT_TWIPS)
/*N*/ 		nVal = (sal_Int16)MM100_TO_TWIP(nVal);
/*N*/ 	SetValue(nVal);
/*N*/ 	return sal_True;
/*N*/ }

// class SvxCaseMapItem --------------------------------------------------

/*N*/ SvxCaseMapItem::SvxCaseMapItem( const SvxCaseMap eMap, const USHORT nId ) :
/*N*/ 	SfxEnumItem( nId, eMap )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*?*/ USHORT SvxCaseMapItem::GetValueCount() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return SVX_CASEMAP_END;	// SVX_CASEMAP_KAPITAELCHEN	+ 1
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxCaseMapItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxCaseMapItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxCaseMapItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (BYTE) GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxCaseMapItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	BYTE cMap;
/*N*/ 	rStrm >> cMap;
/*N*/ 	return new SvxCaseMapItem( (const SvxCaseMap)cMap, Which() );
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------


/*-----------------13.03.98 16:29-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxCaseMapItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	sal_Int16 nRet = style::CaseMap::NONE;
/*N*/ 	switch( GetValue() )
/*N*/ 	{
/*N*/ //		case SVX_CASEMAP_NOT_MAPPED  :		nRet = style::CaseMap::NONE 	; break;
/*N*/ 		case SVX_CASEMAP_VERSALIEN   :      nRet = style::CaseMap::UPPERCASE; break;
/*?*/ 		case SVX_CASEMAP_GEMEINE     :      nRet = style::CaseMap::LOWERCASE; break;
/*N*/ 		case SVX_CASEMAP_TITEL       :      nRet = style::CaseMap::TITLE    ; break;
/*N*/ 		case SVX_CASEMAP_KAPITAELCHEN:      nRet = style::CaseMap::SMALLCAPS; break;
/*N*/ 	}
/*N*/ 	rVal <<= (sal_Int16)(nRet);
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------13.03.98 16:29-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxCaseMapItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_uInt16 nVal;
/*N*/ 	if(!(rVal >>= nVal))
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	switch( nVal )
/*N*/ 	{
/*N*/ 	case style::CaseMap::NONE 	 :  nVal = SVX_CASEMAP_NOT_MAPPED  ; break;
/*N*/ 	case style::CaseMap::UPPERCASE:  nVal = SVX_CASEMAP_VERSALIEN   ; break;
/*?*/ 	case style::CaseMap::LOWERCASE:  nVal = SVX_CASEMAP_GEMEINE     ; break;
/*N*/ 	case style::CaseMap::TITLE    :  nVal = SVX_CASEMAP_TITEL       ; break;
/*N*/ 	case style::CaseMap::SMALLCAPS:  nVal = SVX_CASEMAP_KAPITAELCHEN; break;
/*N*/ 	}
/*N*/ 	SetValue(nVal);
/*N*/ 	return sal_True;
/*N*/ }

// class SvxEscapementItem -----------------------------------------------

/*N*/ SvxEscapementItem::SvxEscapementItem( const USHORT nId ) :
/*N*/ 	SfxEnumItemInterface( nId ),
/*N*/ 
/*N*/ 	nEsc	( 0 ),
/*N*/ 	nProp	( 100 )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ SvxEscapementItem::SvxEscapementItem( const short nEsc,
/*N*/ 									  const BYTE nProp,
/*N*/ 									  const USHORT nId ) :
/*N*/ 	SfxEnumItemInterface( nId ),
/*N*/ 	nEsc	( nEsc ),
/*N*/ 	nProp	( nProp )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxEscapementItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*N*/ 
/*N*/ 	return( nEsc  == ((SvxEscapementItem&)rAttr).nEsc &&
/*N*/ 			nProp == ((SvxEscapementItem&)rAttr).nProp );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxEscapementItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxEscapementItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxEscapementItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	short nEsc = GetEsc();
/*N*/ 	if( SOFFICE_FILEFORMAT_31 == rStrm.GetVersion() )
/*N*/ 	{
/*N*/ 		if( DFLT_ESC_AUTO_SUPER == nEsc )
/*N*/ 			nEsc = DFLT_ESC_SUPER;
/*N*/ 		else if( DFLT_ESC_AUTO_SUB == nEsc )
/*N*/ 			nEsc = DFLT_ESC_SUB;
/*N*/ 	}
/*N*/ 	rStrm << (BYTE) GetProp()
/*N*/ 		  << (short) nEsc;
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxEscapementItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	BYTE nProp;
/*N*/ 	short nEsc;
/*N*/ 	rStrm >> nProp >> nEsc;
/*N*/ 	return new SvxEscapementItem( nEsc, nProp, Which() );
/*N*/ }

// -----------------------------------------------------------------------

/*?*/ USHORT SvxEscapementItem::GetValueCount() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return SVX_ESCAPEMENT_END;	// SVX_ESCAPEMENT_SUBSCRIPT + 1
/*?*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*?*/ USHORT SvxEscapementItem::GetEnumValue() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 //STRIP001 	if ( nEsc < 0 )
/*?*/ }

// -----------------------------------------------------------------------

/*?*/ void SvxEscapementItem::SetEnumValue( USHORT nVal )
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	SetEscapement( (const SvxEscapement)nVal );
/*?*/ }

/*-----------------13.03.98 17:05-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxEscapementItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case MID_ESC:
/*N*/ 			rVal <<= (sal_Int16)(nEsc);
/*N*/ 		break;
/*N*/ 		case MID_ESC_HEIGHT:
/*N*/ 			rVal <<= (sal_Int8)(nProp);
/*N*/ 		break;
/*N*/ 		case MID_AUTO_ESC:
/*?*/ 			rVal = Bool2Any(DFLT_ESC_AUTO_SUB == nEsc || DFLT_ESC_AUTO_SUPER == nEsc);
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------13.03.98 17:05-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxEscapementItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case MID_ESC:
/*N*/ 		{
/*N*/ 			sal_Int16 nVal;
/*N*/ 			if( (rVal >>= nVal) && (Abs(nVal) <= 101))
/*N*/ 				nEsc = nVal;
/*N*/ 			else
/*N*/ 				return sal_False;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_ESC_HEIGHT:
/*N*/ 		{
/*N*/ 			sal_Int8 nVal;
/*N*/ 			if( (rVal >>= nVal) && (nVal <= 100))
/*N*/ 				nProp = nVal;
/*N*/ 			else
/*N*/ 				return sal_False;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_AUTO_ESC:
/*N*/ 		{
/*?*/ 			BOOL bVal = Any2Bool(rVal);
/*?*/ 			if(bVal)
/*?*/ 			{
/*?*/ 				if(nEsc < 0)
/*?*/ 					nEsc = DFLT_ESC_AUTO_SUB;
/*?*/ 				else
/*?*/ 					nEsc = DFLT_ESC_AUTO_SUPER;
/*?*/ 			}
/*?*/ 			else
/*?*/ 				if(DFLT_ESC_AUTO_SUPER == nEsc )
/*?*/ 					--nEsc;
/*?*/ 				else if(DFLT_ESC_AUTO_SUB == nEsc)
/*?*/ 					++nEsc;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

// class SvxLanguageItem -------------------------------------------------

/*N*/ SvxLanguageItem::SvxLanguageItem( const LanguageType eLang, const USHORT nId )
/*N*/ 	: SfxEnumItem( nId , eLang )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*?*/ USHORT SvxLanguageItem::GetValueCount() const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return LANGUAGE_COUNT;	// aus tlintl.hxx
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxLanguageItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxLanguageItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxLanguageItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (USHORT) GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxLanguageItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	USHORT nValue;
/*N*/ 	rStrm >> nValue;
/*N*/ 	return new SvxLanguageItem( (LanguageType)nValue, Which() );
/*N*/ }

//------------------------------------------------------------------------


/*-----------------14.03.98 14:13-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxLanguageItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/         case MID_LANG_INT:  // for basic conversions!
/*?*/ 			rVal <<= (sal_Int16)(GetValue());
/*N*/ 		break;
/*N*/ 		case MID_LANG_LOCALE:
/*N*/ 			lang::Locale aRet( MsLangId::convertLanguageToLocale( GetValue() ));
/*N*/ 			rVal <<= aRet;
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------14.03.98 14:13-------------------

--------------------------------------------------*/
/*N*/ sal_Bool SvxLanguageItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/         case MID_LANG_INT:  // for basic conversions!
/*N*/ 		{
/*?*/ 			sal_Int32 nValue;
/*?*/ 			if(!(rVal >>= nValue))
/*?*/ 				return sal_False;
/*?*/ 
/*?*/ 			SetValue((sal_Int16)nValue);
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_LANG_LOCALE:
/*N*/ 		{
/*N*/ 			lang::Locale aLocale;
/*N*/ 			if(!(rVal >>= aLocale))
/*N*/ 				return sal_False;
/*N*/ 
/*N*/ 			if (aLocale.Language.getLength() || aLocale.Country.getLength())
/*N*/ 				SetValue(MsLangId::convertIsoNamesToLanguage( aLocale.Language, aLocale.Country ));
/*N*/ 			else
/*N*/ 				SetValue(LANGUAGE_NONE);
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

// class SvxNoLinebreakItem ----------------------------------------------
/*N*/ SvxNoLinebreakItem::SvxNoLinebreakItem( const sal_Bool bBreak, const USHORT nId ) :
/*N*/       SfxBoolItem( nId, bBreak )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


//------------------------------------------------------------------------


// class SvxNoHyphenItem -------------------------------------------------

/*N*/ SvxNoHyphenItem::SvxNoHyphenItem( const sal_Bool bHyphen, const USHORT nId ) :
/*N*/ 	SfxBoolItem( nId , bHyphen )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


//------------------------------------------------------------------------


/*
 * Dummy-Item fuer ToolBox-Controls:
 *
 */

// -----------------------------------------------------------------------
// class SvxLineColorItem (== SvxColorItem)
// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


//------------------------------------------------------------------------


// class SvxBlinkItem -------------------------------------------------


/*N*/ SvxBlinkItem::SvxBlinkItem( const sal_Bool bBlink, const USHORT nId ) :
/*N*/ 	SfxBoolItem( nId, bBlink )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxBlinkItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxBlinkItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxBlinkItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (BYTE) GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxBlinkItem::Create(SvStream& rStrm, USHORT) const
/*N*/ {
/*N*/ 	BYTE nState;
/*N*/ 	rStrm >> nState;
/*N*/ 	return new SvxBlinkItem( nState, Which() );
/*N*/ }

// -----------------------------------------------------------------------


// class SvxEmphaisMarkItem ---------------------------------------------------

/*N*/ SvxEmphasisMarkItem::SvxEmphasisMarkItem( const FontEmphasisMark nValue,
/*N*/ 										const USHORT nId )
/*N*/ 	: SfxUInt16Item( nId, nValue )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxEmphasisMarkItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxEmphasisMarkItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxEmphasisMarkItem::Store( SvStream& rStrm,
/*N*/ 									 USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	rStrm << (sal_uInt16)GetValue();
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxEmphasisMarkItem::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 	sal_uInt16 nValue;
/*N*/ 	rStrm >> nValue;
/*N*/ 	return new SvxEmphasisMarkItem( (FontEmphasisMark)nValue, Which() );
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ sal_Bool SvxEmphasisMarkItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 	case MID_EMPHASIS:
/*N*/ 	{
/*N*/ 		sal_Int16 nValue = GetValue();
/*N*/ 		sal_Int16 nRet = 0;
/*N*/ 		switch(nValue & EMPHASISMARK_STYLE)
/*N*/ 		{
/*N*/ 			case EMPHASISMARK_NONE   : nRet = FontEmphasis::NONE;			break;
/*?*/ 			case EMPHASISMARK_DOT	 : nRet = FontEmphasis::DOT_ABOVE;		break;
/*?*/ 			case EMPHASISMARK_CIRCLE : nRet = FontEmphasis::CIRCLE_ABOVE;	break;
/*?*/ 			case EMPHASISMARK_DISC	 : nRet = FontEmphasis::DISK_ABOVE;		break;
/*?*/ 			case EMPHASISMARK_ACCENT : nRet = FontEmphasis::ACCENT_ABOVE;	break;
/*N*/ 		}
/*N*/ 		if(nRet && nValue & EMPHASISMARK_POS_BELOW)
/*N*/ 			nRet += 10;
/*N*/ 		rVal <<= nRet;
/*N*/ 	}
/*N*/ 	break;
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool SvxEmphasisMarkItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	sal_Bool bRet = sal_True;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 	case MID_EMPHASIS:
/*N*/ 	{
/*N*/ 		sal_Int32 nValue = -1;
/*N*/ 		rVal >>= nValue;
/*N*/ 		switch(nValue)
/*N*/ 		{
/*N*/ 			case FontEmphasis::NONE 	   : nValue = EMPHASISMARK_NONE;   break;
/*?*/ 			case FontEmphasis::DOT_ABOVE   : nValue = EMPHASISMARK_DOT|EMPHASISMARK_POS_ABOVE;    break;
/*?*/ 			case FontEmphasis::CIRCLE_ABOVE: nValue = EMPHASISMARK_CIRCLE|EMPHASISMARK_POS_ABOVE; break;
/*?*/ 			case FontEmphasis::DISK_ABOVE  : nValue = EMPHASISMARK_DISC|EMPHASISMARK_POS_ABOVE;   break;
/*?*/ 			case FontEmphasis::ACCENT_ABOVE: nValue = EMPHASISMARK_ACCENT|EMPHASISMARK_POS_ABOVE; break;
/*?*/ 			case FontEmphasis::DOT_BELOW   : nValue = EMPHASISMARK_DOT|EMPHASISMARK_POS_BELOW;    break;
/*?*/ 			case FontEmphasis::CIRCLE_BELOW: nValue = EMPHASISMARK_CIRCLE|EMPHASISMARK_POS_BELOW; break;
/*?*/ 			case FontEmphasis::DISK_BELOW  : nValue = EMPHASISMARK_DISC|EMPHASISMARK_POS_BELOW;   break;
/*?*/ 			case FontEmphasis::ACCENT_BELOW: nValue = EMPHASISMARK_ACCENT|EMPHASISMARK_POS_BELOW; break;
/*?*/ 			default: return sal_False;
/*N*/ 		}
/*N*/ 		SetValue( (sal_Int16)nValue );
/*N*/ 	}
/*N*/ 	break;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ USHORT SvxEmphasisMarkItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SvxEmphasisMarkItem: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }


/*************************************************************************
|*    class SvxTwoLinesItem
*************************************************************************/

/*N*/ SvxTwoLinesItem::SvxTwoLinesItem( sal_Bool bFlag, sal_Unicode nStartBracket,
/*N*/ 									sal_Unicode nEndBracket, sal_uInt16 nW )
/*N*/ 	: SfxPoolItem( nW ),
/*N*/ 	bOn( bFlag ), cStartBracket( nStartBracket ), cEndBracket( nEndBracket )
/*N*/ {
/*N*/ }

/*NBFF*/ SvxTwoLinesItem::SvxTwoLinesItem( const SvxTwoLinesItem& rAttr )
/*NBFF*/ 	: SfxPoolItem( rAttr.Which() ),
/*NBFF*/ 	bOn( rAttr.bOn ), cStartBracket( rAttr.cStartBracket ),
/*NBFF*/ 	cEndBracket( rAttr.cEndBracket )
/*NBFF*/ {
/*NBFF*/ }

/*N*/ SvxTwoLinesItem::~SvxTwoLinesItem()
/*N*/ {
/*N*/ }

/*?*/ int SvxTwoLinesItem::operator==( const SfxPoolItem& rAttr ) const
/*?*/ {
/*NFFF*/	DBG_ASSERT( SfxPoolItem::operator==( rAttr ), "not equal attribute types" );
/*NBFF*/ 	return bOn == ((SvxTwoLinesItem&)rAttr).bOn &&
/*NBFF*/ 		   cStartBracket == ((SvxTwoLinesItem&)rAttr).cStartBracket &&
/*NBFF*/ 		   cEndBracket == ((SvxTwoLinesItem&)rAttr).cEndBracket;
/*?*/ }

/*?*/ SfxPoolItem* SvxTwoLinesItem::Clone( SfxItemPool* ) const
/*?*/ {
/*NBFF*/	return new SvxTwoLinesItem( *this );
/*?*/ }

/*NBFF*/ sal_Bool SvxTwoLinesItem::QueryValue( ::com::sun::star::uno::Any& rVal,
/*NBFF*/ 								BYTE nMemberId ) const
/*NBFF*/ {
/*NBFF*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*NBFF*/     nMemberId &= ~CONVERT_TWIPS;
/*NBFF*/ 	sal_Bool bRet = sal_True;
/*NBFF*/ 	switch( nMemberId )
/*NBFF*/ 	{
/*NBFF*/ 	case MID_TWOLINES:
/*NBFF*/ 		rVal = Bool2Any( bOn );
/*NBFF*/ 		break;
/*NBFF*/ 	case MID_START_BRACKET:
/*NBFF*/ 		{
/*NBFF*/ 			OUString s;
/*NBFF*/ 			if( cStartBracket )
/*NBFF*/ 				s = OUString( cStartBracket );
/*NBFF*/ 			rVal <<= s;
/*NBFF*/ 		}
/*NBFF*/ 		break;
/*NBFF*/ 	case MID_END_BRACKET:
/*NBFF*/ 		{
/*NBFF*/ 			OUString s;
/*NBFF*/ 			if( cEndBracket )
/*NBFF*/ 				s = OUString( cEndBracket );
/*NBFF*/ 			rVal <<= s;
/*NBFF*/ 		}
/*NBFF*/ 		break;
/*NBFF*/ 	default:
/*NBFF*/ 		bRet = sal_False;
/*NBFF*/ 		break;
/*NBFF*/ 	}
/*NBFF*/ 	return bRet;
/*NBFF*/ }

/*NBFF*/ sal_Bool SvxTwoLinesItem::PutValue( const ::com::sun::star::uno::Any& rVal,
/*NBFF*/ 									BYTE nMemberId )
/*NBFF*/ {
/*NBFF*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*NBFF*/     nMemberId &= ~CONVERT_TWIPS;
/*NBFF*/ 	sal_Bool bRet = sal_False;
/*NBFF*/ 	OUString s;
/*NBFF*/ 	switch( nMemberId )
/*NBFF*/ 	{
/*NBFF*/ 	case MID_TWOLINES:
/*NBFF*/ 		bOn = Any2Bool( rVal );
/*NBFF*/ 		bRet = sal_True;
/*NBFF*/ 		break;
/*NBFF*/ 	case MID_START_BRACKET:
/*NBFF*/ 		if( rVal >>= s )
/*NBFF*/ 		{
/*NBFF*/ 			cStartBracket = s.getLength() ? s[ 0 ] : 0;
/*NBFF*/ 			bRet = sal_True;
/*NBFF*/ 		}
/*NBFF*/ 		break;
/*NBFF*/ 	case MID_END_BRACKET:
/*NBFF*/ 		if( rVal >>= s )
/*NBFF*/ 		{
/*NBFF*/ 			cEndBracket = s.getLength() ? s[ 0 ] : 0;
/*NBFF*/ 			bRet = sal_True;
/*NBFF*/ 		}
/*NBFF*/ 		break;
/*NBFF*/ 	}
/*NBFF*/ 	return bRet;
/*NBFF*/ }

/*NBFF*/ SfxItemPresentation SvxTwoLinesItem::GetPresentation( SfxItemPresentation ePres,
/*NBFF*/ 							SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
/*NBFF*/                             String &rText, const ::IntlWrapper* pIntl ) const
/*NBFF*/ {
/*NBFF*/ 	switch( ePres )
/*NBFF*/ 	{
/*NBFF*/ 	case SFX_ITEM_PRESENTATION_NONE:
/*NBFF*/ 		rText.Erase();
/*NBFF*/ 		break;
/*NBFF*/ 	case SFX_ITEM_PRESENTATION_NAMELESS:
/*NBFF*/ 	case SFX_ITEM_PRESENTATION_COMPLETE:
/*NBFF*/ 		{
/*NBFF*/ 			if( !GetValue() )
/*NBFF*/ 				rText = SVX_RESSTR( RID_SVXITEMS_TWOLINES_OFF );
/*NBFF*/ 			else
/*NBFF*/ 			{
/*NBFF*/ 				rText = SVX_RESSTR( RID_SVXITEMS_TWOLINES );
/*NBFF*/ 				if( GetStartBracket() )
/*NBFF*/ 					rText.Insert( GetStartBracket(), 0 );
/*NBFF*/ 				if( GetEndBracket() )
/*NBFF*/ 					rText += GetEndBracket();
/*NBFF*/ 			}
/*NBFF*/ 			return ePres;
/*NBFF*/ 		}
/*NBFF*/ 		break;
/*NBFF*/ 	}
/*NBFF*/ 	return SFX_ITEM_PRESENTATION_NONE;
/*NBFF*/ }


/*NBFF*/ SfxPoolItem* SvxTwoLinesItem::Create( SvStream & rStrm, USHORT nVer) const
/*NBFF*/ {
/*NBFF*/ 	sal_Bool bOn;
/*NBFF*/ 	sal_Unicode cStart, cEnd;
/*NBFF*/ 	rStrm >> bOn >> cStart >> cEnd;
/*NBFF*/ 	return new SvxTwoLinesItem( bOn, cStart, cEnd, Which() );
/*NBFF*/ }

/*NBFF*/ SvStream& SvxTwoLinesItem::Store(SvStream & rStrm, USHORT nIVer) const
/*NBFF*/ {
/*NBFF*/ 	rStrm << GetValue() << GetStartBracket() << GetEndBracket();
/*NBFF*/ 	return rStrm;
/*NBFF*/ }

/*N*/ USHORT SvxTwoLinesItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/ 			SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/ 			"SvxTwoLinesItem: Gibt es ein neues Fileformat?" );
/*N*/ 
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }


/*************************************************************************
|*    class SvxCharRotateItem
*************************************************************************/

/*N*/ SvxCharRotateItem::SvxCharRotateItem( sal_uInt16 nValue,
/*N*/ 					   				sal_Bool bFitIntoLine,
/*N*/ 					   				const sal_uInt16 nW )
/*N*/ 	: SfxUInt16Item( nW, nValue ), bFitToLine( bFitIntoLine )
/*N*/ {
/*N*/ }

/*NBFF*/ SfxPoolItem* SvxCharRotateItem::Clone( SfxItemPool* ) const
/*NBFF*/ {
/*NBFF*/ 	return new SvxCharRotateItem( GetValue(), IsFitToLine(), Which() );
/*NBFF*/ }

/*NBFF*/ SfxPoolItem* SvxCharRotateItem::Create( SvStream& rStrm, USHORT ) const
/*NBFF*/ {
/*NBFF*/ 	sal_uInt16 nVal;
/*NBFF*/ 	sal_Bool b;
/*NBFF*/ 	rStrm >> nVal >> b;
/*NBFF*/ 	return new SvxCharRotateItem( nVal, b, Which() );
/*NBFF*/ }

/*NBFF*/ SvStream& SvxCharRotateItem::Store( SvStream & rStrm, USHORT ) const
/*NBFF*/ {
/*NBFF*/ 	sal_Bool bFlag = IsFitToLine();
/*NBFF*/ 	rStrm << GetValue() << bFlag;
/*NBFF*/ 	return rStrm;
/*NBFF*/ }

/*N*/ USHORT SvxCharRotateItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*NBFF*/ SfxItemPresentation SvxCharRotateItem::GetPresentation(
/*NBFF*/ 		SfxItemPresentation ePres,
/*NBFF*/ 		SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
/*NBFF*/         String &rText, const ::IntlWrapper*  ) const
/*NBFF*/ {
/*NBFF*/ 	switch( ePres )
/*NBFF*/ 	{
/*NBFF*/ 	case SFX_ITEM_PRESENTATION_NONE:
/*NBFF*/ 		rText.Erase();
/*NBFF*/ 		break;
/*NBFF*/ 	case SFX_ITEM_PRESENTATION_NAMELESS:
/*NBFF*/ 	case SFX_ITEM_PRESENTATION_COMPLETE:
/*NBFF*/ 		{
/*NBFF*/ 			if( !GetValue() )
/*NBFF*/ 				rText = SVX_RESSTR( RID_SVXITEMS_CHARROTATE_OFF );
/*NBFF*/ 			else
/*NBFF*/ 			{
/*NBFF*/ 				rText = SVX_RESSTR( RID_SVXITEMS_CHARROTATE );
/*NBFF*/ 				rText.SearchAndReplaceAscii( "$(ARG1)",
/*NBFF*/ 							String::CreateFromInt32( GetValue() / 10 ));
/*NBFF*/ 				if( IsFitToLine() )
/*NBFF*/ 					rText += SVX_RESSTR( RID_SVXITEMS_CHARROTATE_FITLINE );
/*NBFF*/ 			}
/*NBFF*/ 			return ePres;
/*NBFF*/ 		}
/*NBFF*/ 		break;
/*NBFF*/ 	}
/*NBFF*/ 	return SFX_ITEM_PRESENTATION_NONE;
/*NBFF*/ }

/*NBFF*/ sal_Bool SvxCharRotateItem::QueryValue( ::com::sun::star::uno::Any& rVal,
/*NBFF*/ 								BYTE nMemberId ) const
/*NBFF*/ {
/*NBFF*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*NBFF*/     nMemberId &= ~CONVERT_TWIPS;
/*NBFF*/ 	sal_Bool bRet = sal_True;
/*NBFF*/ 	switch( nMemberId )
/*NBFF*/ 	{
/*NBFF*/ 	case MID_ROTATE:
/*NBFF*/ 		rVal <<= (sal_Int16)GetValue();
/*NBFF*/ 		break;
/*NBFF*/ 	case MID_FITTOLINE:
/*NBFF*/ 		rVal = Bool2Any( IsFitToLine() );
/*NBFF*/ 		break;
/*NBFF*/ 	default:
/*NBFF*/ 		bRet = sal_False;
/*NBFF*/ 		break;
/*NBFF*/ 	}
/*NBFF*/ 	return bRet;
/*NBFF*/ }

/*NBFF*/ sal_Bool SvxCharRotateItem::PutValue( const ::com::sun::star::uno::Any& rVal,
/*NBFF*/ 									BYTE nMemberId )
/*NBFF*/ {
/*NBFF*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*NBFF*/     nMemberId &= ~CONVERT_TWIPS;
/*NBFF*/ 	sal_Bool bRet = sal_True;
/*NBFF*/ 	sal_Int16 nVal;
/*NBFF*/ 	switch( nMemberId )
/*NBFF*/ 	{
/*NBFF*/ 	case MID_ROTATE:
/*NBFF*/ 		rVal >>= nVal;
/*NBFF*/ 		if(!nVal || 900 == nVal || 2700 == nVal)
/*NBFF*/ 			SetValue( (USHORT)nVal );
/*NBFF*/ 		else
/*NBFF*/ 			bRet = sal_False;
/*NBFF*/ 		break;
/*NBFF*/ 
/*NBFF*/ 	case MID_FITTOLINE:
/*NBFF*/ 		SetFitToLine( Any2Bool( rVal ) );
/*NBFF*/ 		break;
/*NBFF*/ 	default:
/*NBFF*/ 		bRet = sal_False;
/*NBFF*/ 	}
/*NBFF*/ 	return bRet;
/*NBFF*/ }

/*NBFF*/ int SvxCharRotateItem::operator==( const SfxPoolItem& rItem ) const
/*NBFF*/ {
/*NBFF*/ 	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
/*NBFF*/ 	return SfxUInt16Item::operator==( rItem ) &&
/*NBFF*/ 		   IsFitToLine() == ((const SvxCharRotateItem&)rItem).IsFitToLine();
/*NBFF*/ }


/*************************************************************************
|*    class SvxCharScaleItem
*************************************************************************/

/*N*/ SvxCharScaleWidthItem::SvxCharScaleWidthItem( sal_uInt16 nValue,
/*N*/ 					   						const sal_uInt16 nW )
/*N*/ 	: SfxUInt16Item( nW, nValue )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* SvxCharScaleWidthItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxCharScaleWidthItem( GetValue(), Which() );
/*N*/ }

/*N*/ SfxPoolItem* SvxCharScaleWidthItem::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 	sal_uInt16 nVal;
/*N*/ 	rStrm >> nVal;
/*N*/ 	SvxCharScaleWidthItem* pItem = new SvxCharScaleWidthItem( nVal, Which() );
/*N*/ 
/*N*/     if ( Which() == EE_CHAR_FONTWIDTH )
/*N*/     {
/*N*/         // #87271#: Was a SvxFontWidthItem in 5.2
/*N*/         // USHORT nFixWidth, USHORT nPropWidth.
/*N*/         // nFixWidth has never been used...
/*N*/ 	    rStrm >> nVal;
/*N*/         USHORT nTest;
/*N*/ 	    rStrm >> nTest;
/*N*/         if ( nTest == 0x1234 )
/*N*/ 	        pItem->SetValue( nVal );
/*N*/         else
/*N*/             rStrm.SeekRel( -2*(long)sizeof(sal_uInt16) );
/*N*/     }
/*N*/ 
/*N*/     return pItem;
/*N*/ }

/*N*/ SvStream& SvxCharScaleWidthItem::Store( SvStream& rStream, USHORT nVer ) const
/*N*/ {
/*N*/     SvStream& rRet = SfxUInt16Item::Store( rStream, nVer );
/*N*/     if ( Which() == EE_CHAR_FONTWIDTH )
/*N*/     {
/*N*/         // see comment in Create()....
/*N*/         rRet.SeekRel( -1*(long)sizeof(USHORT) );
/*N*/         rRet << (USHORT)0;
/*N*/         rRet << GetValue();
/*N*/         // Really ugly, but not a problem for reading the doc in 5.2
/*N*/         rRet << (USHORT)0x1234;
/*N*/     }
/*N*/     return rRet;
/*N*/ }


/*N*/ USHORT SvxCharScaleWidthItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }


/*N*/ sal_Bool SvxCharScaleWidthItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
/*N*/     // where we still want this to be a sal_Int16
/*N*/     sal_Int16 nValue;
/*N*/ 	if (rVal >>= nValue)
/*N*/ 	{
/*N*/         SetValue( (UINT16) nValue );
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/     DBG_ERROR( "SvxCharScaleWidthItem::PutValue - Wrong type!" );
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ sal_Bool SvxCharScaleWidthItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
/*N*/     // where we still want this to be a sal_Int16
/*N*/     rVal <<= (sal_Int16)GetValue();
/*N*/     return TRUE;
/*N*/ }

/*************************************************************************
|*    class SvxCharReliefItem
*************************************************************************/

/*N*/ SvxCharReliefItem::SvxCharReliefItem( FontRelief eValue,
/*N*/ 					   				  const sal_uInt16 nId )
/*N*/ 	: SfxEnumItem( nId, eValue )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* SvxCharReliefItem::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SvxCharReliefItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* SvxCharReliefItem::Create(SvStream & rStrm, USHORT) const
/*N*/ {
/*N*/ 	sal_uInt16 nVal;
/*N*/ 	rStrm >> nVal;
/*N*/ 	return new SvxCharReliefItem( (FontRelief)nVal, Which() );
/*N*/ }

/*N*/ SvStream& SvxCharReliefItem::Store(SvStream & rStrm, USHORT nIVer) const
/*N*/ {
/*N*/ 	sal_uInt16 nVal = GetValue();
/*N*/ 	rStrm << nVal;
/*N*/ 	return rStrm;
/*N*/ }

/*N*/ USHORT SvxCharReliefItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/ 	return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*N*/ String SvxCharReliefItem::GetValueTextByPos( USHORT nPos ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( RID_SVXITEMS_RELIEF_ENGRAVED - RID_SVXITEMS_RELIEF_NONE,
/*N*/ 					"enum overflow" );
/*N*/ 	return String( SVX_RES( RID_SVXITEMS_RELIEF_BEGIN + nPos ));
/*N*/ }

/*N*/ USHORT SvxCharReliefItem::GetValueCount() const
/*N*/ {
/*N*/ 	return RID_SVXITEMS_RELIEF_ENGRAVED - RID_SVXITEMS_RELIEF_NONE;
/*N*/ }

/*N*/ SfxItemPresentation SvxCharReliefItem::GetPresentation
/*N*/ (
/*N*/     SfxItemPresentation ePres,
/*N*/     SfxMapUnit          eCoreUnit,
/*N*/     SfxMapUnit          ePresUnit,
/*N*/     XubString&          rText, const ::IntlWrapper *pIntl
/*N*/ )   const
/*N*/ {
/*?*/ 	SfxItemPresentation eRet = ePres;
/*?*/     switch( ePres )
/*?*/     {
/*?*/     case SFX_ITEM_PRESENTATION_NONE:
/*?*/         rText.Erase();
/*?*/ 		break;
/*?*/ 
/*?*/     case SFX_ITEM_PRESENTATION_NAMELESS:
/*?*/     case SFX_ITEM_PRESENTATION_COMPLETE:
/*?*/         rText = GetValueTextByPos( GetValue() );
/*?*/ 		break;
/*?*/ 
/*?*/ 	default:
/*?*/ 		eRet = SFX_ITEM_PRESENTATION_NONE;
/*?*/     }
/*?*/     return eRet;
/*N*/ }

/*N*/ sal_Bool SvxCharReliefItem::PutValue( const ::com::sun::star::uno::Any& rVal,
/*N*/ 										BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	sal_Bool bRet = sal_True;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 	case MID_RELIEF:
/*N*/ 		{
/*N*/             sal_Int16 nVal = -1;
/*N*/ 			rVal >>= nVal;
/*N*/             if(nVal >= 0 && nVal <= RELIEF_ENGRAVED)
/*N*/                 SetValue( (FontRelief)nVal );
/*N*/             else
/*N*/                 bRet = sal_False;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	default:
/*N*/ 		bRet = sal_False;
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ sal_Bool SvxCharReliefItem::QueryValue( ::com::sun::star::uno::Any& rVal,
/*N*/ 										BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	sal_Bool bRet = sal_True;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 	case MID_RELIEF:
/*N*/ 		rVal <<= (sal_Int16)GetValue();
/*N*/ 		break;
/*N*/ 	default:
/*N*/ 		bRet = sal_False;
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*************************************************************************
|*    class SvxScriptTypeItemItem
*************************************************************************/


/*************************************************************************
|*    class SvxScriptSetItem
*************************************************************************/

/*N*/ SfxPoolItem* SvxScriptSetItem::Clone( SfxItemPool *pPool ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 
/*N*/ }

/*N*/ SfxPoolItem* SvxScriptSetItem::Create( SvStream &, USHORT ) const
/*N*/ {
/*N*/ 	return 0;
/*N*/ }

/*N*/ void GetDefaultFonts( SvxFontItem& rLatin, SvxFontItem& rAsian, SvxFontItem& rComplex )
/*N*/ {
/*N*/ 	const USHORT nItemCnt = 3;
/*N*/ 
/*N*/ 	static struct
/*N*/ 	{
/*N*/ 		USHORT nFontType;
/*N*/ 		USHORT nLanguage;
/*N*/ 	}
/*N*/ 	aOutTypeArr[ nItemCnt ] =
/*N*/ 	{
/*N*/ 		{  DEFAULTFONT_LATIN_TEXT, LANGUAGE_ENGLISH_US },
/*N*/ 		{  DEFAULTFONT_CJK_TEXT, LANGUAGE_ENGLISH_US },
/*N*/ 		{  DEFAULTFONT_CTL_TEXT, LANGUAGE_ARABIC_SAUDI_ARABIA }
/*N*/ 	};
/*N*/ 
/*N*/ 	SvxFontItem* aItemArr[ nItemCnt ] = { &rLatin, &rAsian, &rComplex };
/*N*/ 
/*N*/ 	for ( USHORT n = 0; n < nItemCnt; ++n )
/*N*/ 	{
/*N*/ 		Font aFont( OutputDevice::GetDefaultFont( aOutTypeArr[ n ].nFontType,
/*N*/ 												  aOutTypeArr[ n ].nLanguage,
/*N*/ 												  DEFAULTFONT_FLAGS_ONLYONE, 0 ) );
/*N*/ 		SvxFontItem* pItem = aItemArr[ n ];
/*N*/ 		pItem->GetFamily() = aFont.GetFamily();
/*N*/ 		pItem->GetFamilyName() = aFont.GetName();
/*N*/ 		pItem->GetStyleName().Erase();
/*N*/ 		pItem->GetPitch() = aFont.GetPitch();
/*N*/ 		pItem->GetCharSet() = aFont.GetCharSet();
/*N*/ 	}
/*N*/ }


/*N*/ USHORT GetI18NScriptTypeOfLanguage( USHORT nLang )
/*N*/ {
/*N*/     return GetI18NScriptType( SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ) );
/*N*/ }

/*N*/ USHORT GetItemScriptType( short nI18NType )
/*N*/ {
/*N*/     switch ( nI18NType )
/*N*/     {
/*N*/ 	    case i18n::ScriptType::LATIN:   return SCRIPTTYPE_LATIN;
/*?*/ 	    case i18n::ScriptType::ASIAN:   return SCRIPTTYPE_ASIAN;
/*?*/ 	    case i18n::ScriptType::COMPLEX: return SCRIPTTYPE_COMPLEX;
/*N*/     }
/*N*/     return 0;
/*N*/ }

/*N*/ short GetI18NScriptType( USHORT nItemType )
/*N*/ {
/*N*/     switch ( nItemType )
/*N*/     {
/*N*/         case SCRIPTTYPE_LATIN:      return i18n::ScriptType::LATIN;
/*?*/         case SCRIPTTYPE_ASIAN:      return i18n::ScriptType::ASIAN;
/*?*/         case SCRIPTTYPE_COMPLEX:    return i18n::ScriptType::COMPLEX;
/*N*/     }
/*N*/     return 0;
/*N*/ }
}
