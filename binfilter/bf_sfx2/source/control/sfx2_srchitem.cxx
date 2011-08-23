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

#include "bf_basic/sbxvar.hxx"
#include <bf_svtools/searchopt.hxx>
#include <sal/macros.h>

#include <bf_svtools/memberid.hrc>

#ifdef _MSC_VER
#pragma hdrstop
#endif

#define _SVX_SRCHITEM_CXX

#include "sfxsids.hrc"
#define ITEMID_SEARCH	SID_SEARCH_ITEM
#include "srchitem.hxx"

namespace binfilter {

//using namespace uno;
using namespace rtl;
using namespace utl;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

#define CFG_ROOT_NODE	"Office.Common/SearchOptions"

// STATIC DATA -----------------------------------------------------------

/*N*/ TYPEINIT1_AUTOFACTORY(SvxSearchItem, SfxPoolItem);

// -----------------------------------------------------------------------

/*N*/ static Sequence< OUString > lcl_GetNotifyNames()
/*N*/ {
/*N*/ 	// names of transliteration relevant properties
/*N*/ 	static const char* aTranslitNames[] =
/*N*/ 	{
/*N*/ 		"IsMatchCase",							//  0
/*N*/ 		"Japanese/IsMatchFullHalfWidthForms",	//  1
/*N*/ 		"Japanese/IsMatchHiraganaKatakana",		//  2
/*N*/ 		"Japanese/IsMatchContractions",			//  3
/*N*/ 		"Japanese/IsMatchMinusDashCho-on",		//  4
/*N*/ 		"Japanese/IsMatchRepeatCharMarks",		//  5
/*N*/ 		"Japanese/IsMatchVariantFormKanji",		//  6
/*N*/ 		"Japanese/IsMatchOldKanaForms",			//  7
/*N*/ 		"Japanese/IsMatch_DiZi_DuZu",			//  8
/*N*/ 		"Japanese/IsMatch_BaVa_HaFa",			//  9
/*N*/ 		"Japanese/IsMatch_TsiThiChi_DhiZi",		// 10
/*N*/ 		"Japanese/IsMatch_HyuIyu_ByuVyu",		// 11
/*N*/ 		"Japanese/IsMatch_SeShe_ZeJe",			// 12
/*N*/ 		"Japanese/IsMatch_IaIya",				// 13
/*N*/ 		"Japanese/IsMatch_KiKu",				// 14
/*N*/ 		"Japanese/IsIgnorePunctuation",			// 15
/*N*/ 		"Japanese/IsIgnoreWhitespace",			// 16
/*N*/ 		"Japanese/IsIgnoreProlongedSoundMark",	// 17
/*N*/ 		"Japanese/IsIgnoreMiddleDot"			// 18
/*N*/ 	};
/*N*/ 
/*N*/   const int nCount = SAL_N_ELEMENTS( aTranslitNames );
/*N*/ 	Sequence< OUString > aNames( nCount );
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for (INT32 i = 0;  i < nCount;  ++i)
/*N*/ 		pNames[i] = OUString::createFromAscii( aTranslitNames[i] );
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ SvxSearchItem::SvxSearchItem( const sal_uInt16 nId ) :
/*N*/ 
/*N*/ 	SfxPoolItem( nId ),
/*N*/ 	ConfigItem( OUString::createFromAscii( CFG_ROOT_NODE ) ),
/*N*/ 
/*N*/ 	aSearchOpt		(	SearchAlgorithms_ABSOLUTE,
/*N*/ 						SearchFlags::LEV_RELAXED,
/*N*/   						OUString(),
/*N*/   						OUString(),
/*N*/   						Locale(),
/*N*/   						2, 2, 2,
/*N*/   						TransliterationModules_IGNORE_CASE ),
/*N*/ 	nCommand		( 0 ),
/*N*/ 	bBackward		( sal_False ),
/*N*/ 	bPattern		( sal_False ),
/*N*/ 	bContent		( sal_False ),
/*N*/ 	eFamily			( SFX_STYLE_FAMILY_PARA ),
/*N*/ 	bRowDirection	( sal_True ),
/*N*/ 	bAllTables		( sal_False ),
/*N*/ 	nCellType		( SVX_SEARCHIN_FORMULA ),
/*N*/ 	nAppFlag		( SVX_SEARCHAPP_WRITER ),
/*N*/ 	bAsianOptions	( FALSE )
/*N*/ {
/*N*/ 	EnableNotification( lcl_GetNotifyNames() );
/*N*/ 
/*N*/ 	SvtSearchOptions aOpt;
/*N*/ 
/*N*/ 	bBackward 		= aOpt.IsBackwards();
/*N*/ 	bAsianOptions	= aOpt.IsUseAsianOptions();
/*N*/ 
/*N*/ 	if (aOpt.IsUseRegularExpression())
/*N*/ 		aSearchOpt.algorithmType = SearchAlgorithms_REGEXP;
/*N*/ 	if (aOpt.IsSimilaritySearch())
/*N*/ 		aSearchOpt.algorithmType = SearchAlgorithms_APPROXIMATE;
/*N*/ 	if (aOpt.IsWholeWordsOnly())
/*N*/ 		aSearchOpt.searchFlag |= SearchFlags::NORM_WORD_ONLY;
/*N*/ 
/*N*/ 	INT32 &rFlags = aSearchOpt.transliterateFlags;
/*N*/ 
/*N*/ 	if (!aOpt.IsMatchCase())
/*N*/ 		rFlags |= TransliterationModules_IGNORE_CASE;
/*N*/     if ( aOpt.IsMatchFullHalfWidthForms())
/*N*/ 		rFlags |= TransliterationModules_IGNORE_WIDTH;
/*N*/     if ( aOpt.IsMatchHiraganaKatakana())
/*N*/ 		rFlags |= TransliterationModules_IGNORE_KANA;
/*N*/     if ( aOpt.IsMatchContractions())
/*N*/ 		rFlags |= TransliterationModules_ignoreSize_ja_JP;
/*N*/     if ( aOpt.IsMatchMinusDashChoon())
/*N*/ 		rFlags |= TransliterationModules_ignoreMinusSign_ja_JP;
/*N*/     if ( aOpt.IsMatchRepeatCharMarks())
/*N*/ 		rFlags |= TransliterationModules_ignoreIterationMark_ja_JP;
/*N*/     if ( aOpt.IsMatchVariantFormKanji())
/*N*/ 		rFlags |= TransliterationModules_ignoreTraditionalKanji_ja_JP;
/*N*/     if ( aOpt.IsMatchOldKanaForms())
/*N*/ 		rFlags |= TransliterationModules_ignoreTraditionalKana_ja_JP;
/*N*/     if ( aOpt.IsMatchDiziDuzu())
/*N*/ 		rFlags |= TransliterationModules_ignoreZiZu_ja_JP;
/*N*/     if ( aOpt.IsMatchBavaHafa())
/*N*/ 		rFlags |= TransliterationModules_ignoreBaFa_ja_JP;
/*N*/     if ( aOpt.IsMatchTsithichiDhizi())
/*N*/ 		rFlags |= TransliterationModules_ignoreTiJi_ja_JP;
/*N*/     if ( aOpt.IsMatchHyuiyuByuvyu())
/*N*/ 		rFlags |= TransliterationModules_ignoreHyuByu_ja_JP;
/*N*/     if ( aOpt.IsMatchSesheZeje())
/*N*/ 		rFlags |= TransliterationModules_ignoreSeZe_ja_JP;
/*N*/     if ( aOpt.IsMatchIaiya())
/*N*/ 		rFlags |= TransliterationModules_ignoreIandEfollowedByYa_ja_JP;
/*N*/     if ( aOpt.IsMatchKiku())
/*N*/ 		rFlags |= TransliterationModules_ignoreKiKuFollowedBySa_ja_JP;
/*N*/ 	if ( aOpt.IsIgnorePunctuation())
/*N*/ 		rFlags |= TransliterationModules_ignoreSeparator_ja_JP;
/*N*/ 	if ( aOpt.IsIgnoreWhitespace())
/*N*/ 		rFlags |= TransliterationModules_ignoreSpace_ja_JP;
/*N*/ 	if ( aOpt.IsIgnoreProlongedSoundMark())
/*N*/ 		rFlags |= TransliterationModules_ignoreProlongedSoundMark_ja_JP;
/*N*/ 	if ( aOpt.IsIgnoreMiddleDot())
/*N*/ 		rFlags |= TransliterationModules_ignoreMiddleDot_ja_JP;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxSearchItem::SvxSearchItem( const SvxSearchItem& rItem ) :
/*N*/ 
/*N*/ 	SfxPoolItem	( rItem ),
/*N*/ 	ConfigItem( OUString::createFromAscii( CFG_ROOT_NODE ) ),
/*N*/ 
/*N*/ 	aSearchOpt		( rItem.aSearchOpt ),
/*N*/ 	nCommand		( rItem.nCommand ),
/*N*/ 	bBackward		( rItem.bBackward ),
/*N*/ 	bPattern		( rItem.bPattern ),
/*N*/ 	bContent		( rItem.bContent ),
/*N*/ 	eFamily			( rItem.eFamily ),
/*N*/ 	bRowDirection	( rItem.bRowDirection ),
/*N*/ 	bAllTables		( rItem.bAllTables ),
/*N*/ 	nCellType		( rItem.nCellType ),
/*N*/ 	nAppFlag		( rItem.nAppFlag ),
/*N*/ 	bAsianOptions	( rItem.bAsianOptions )
/*N*/ {
/*N*/ 	EnableNotification( lcl_GetNotifyNames() );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxSearchItem::~SvxSearchItem()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ SfxPoolItem* SvxSearchItem::Clone( SfxItemPool *pPool) const
/*N*/ {
/*N*/ 	return new SvxSearchItem(*this);
/*N*/ }

// -----------------------------------------------------------------------

//! used below
/*N*/ static BOOL operator == ( const SearchOptions& rItem1, const SearchOptions& rItem2 )
/*N*/ {
/*N*/ 	return rItem1.algorithmType 		== rItem2.algorithmType	&&
/*N*/ 		   rItem1.searchFlag 			== rItem2.searchFlag	&&
/*N*/ 		   rItem1.searchString 			== rItem2.searchString	&&
/*N*/ 		   rItem1.replaceString 		== rItem2.replaceString	&&
/*N*/ 		   //rItem1.Locale 				== rItem2.Locale		&&
/*N*/ 		   rItem1.changedChars 			== rItem2.changedChars	&&
/*N*/ 		   rItem1.deletedChars 			== rItem2.deletedChars	&&
/*N*/ 		   rItem1.insertedChars 		== rItem2.insertedChars	&&
/*N*/ 		   rItem1.transliterateFlags	== rItem2.transliterateFlags;
/*N*/ }


/*N*/ int SvxSearchItem::operator==( const SfxPoolItem& rItem ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal which or type" );
/*N*/ 	const SvxSearchItem &rSItem = (SvxSearchItem &) rItem;
/*N*/ 	return ( nCommand 		== rSItem.nCommand )		&&
/*N*/ 		   ( bBackward 		== rSItem.bBackward )		&&
/*N*/ 		   ( bPattern 		== rSItem.bPattern )		&&
/*N*/ 		   ( bContent 		== rSItem.bContent )		&&
/*N*/ 		   ( eFamily 		== rSItem.eFamily )			&&
/*N*/ 		   ( bRowDirection 	== rSItem.bRowDirection )	&&
/*N*/ 		   ( bAllTables 	== rSItem.bAllTables )		&&
/*N*/ 		   ( nCellType 		== rSItem.nCellType )		&&
/*N*/ 		   ( nAppFlag 		== rSItem.nAppFlag )		&&
/*N*/ 		   ( bAsianOptions	== rSItem.bAsianOptions )	&&
/*N*/ 		   ( aSearchOpt     == rSItem.aSearchOpt );
/*N*/ }


//------------------------------------------------------------------------

/*N*/ SfxItemPresentation SvxSearchItem::GetPresentation
/*N*/ (
/*N*/ 	SfxItemPresentation ePres,
/*N*/ 	SfxMapUnit			eCoreUnit,
/*N*/ 	SfxMapUnit			ePresUnit,
/*N*/ 	XubString& 			rText,
/*N*/     const ::IntlWrapper *
/*N*/ )	const
/*N*/ {
/*N*/ 	return SFX_ITEM_PRESENTATION_NONE;
/*N*/ }

/*N*/ void SvxSearchItem::Notify( const Sequence< OUString > &rPropertyNames )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ bool SvxSearchItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return true;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ bool SvxSearchItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return true;
/*N*/ }

void SvxSearchItem::Commit()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
