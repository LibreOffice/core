/*************************************************************************
 *
 *  $RCSfile: srchitem.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tl $ $Date: 2001-02-23 12:56:51 $
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

#ifndef __SBX_SBXVARIABLE_HXX
#include <svtools/sbxvar.hxx>
#endif
#ifndef _SVT_SEARCHOPT_HXX_
#include <svtools/searchopt.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XREPLACEABLE_HPP_
#include <com/sun/star/util/XReplaceable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSEARCHABLE_HPP_
#include <com/sun/star/util/XSearchable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSEARCHDESCRIPTOR_HPP_
#include <com/sun/star/util/XSearchDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XPROPERTYREPLACE_HPP_
#include <com/sun/star/util/XPropertyReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREPLACEDESCRIPTOR_HPP_
#include <com/sun/star/util/XReplaceDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#include <svtools/memberid.hrc>
//ASDBG #include <usr/uno.hxx>

#pragma hdrstop

#define _SVX_SRCHITEM_CXX

#include "sfxsids.hrc"
#define ITEMID_SEARCH   SID_SEARCH_ITEM
#include "srchitem.hxx"

#include <sfxuno.hxx>

//using namespace uno;
using namespace rtl;
using namespace utl;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace com::sun::star::uno;

#define CFG_ROOT_NODE   "Office.Common/SearchOptions"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxSearchItem, SfxPoolItem);

// -----------------------------------------------------------------------

static Sequence< OUString > lcl_GetNotifyNames()
{
    // names of transliteration relevant properties
    static const char* aTranslitNames[] =
    {
        "IsMatchCase",                          //  0
        "Japanese/IsMatchFullHalfWidthForms",   //  1
        "Japanese/IsMatchHiraganaKatakana",     //  2
        "Japanese/IsMatchContractions",         //  3
        "Japanese/IsMatchMinusDashCho-on",      //  4
        "Japanese/IsMatchRepeatCharMarks",      //  5
        "Japanese/IsMatchVariantFormKanji",     //  6
        "Japanese/IsMatchOldKanaForms",         //  7
        "Japanese/IsMatch_DiZi_DuZu",           //  8
        "Japanese/IsMatch_BaVa_HaFa",           //  9
        "Japanese/IsMatch_TsiThiChi_DhiZi",     // 10
        "Japanese/IsMatch_HyuIyu_ByuVyu",       // 11
        "Japanese/IsMatch_SeShe_ZeJe",          // 12
        "Japanese/IsMatch_IaIya",               // 13
        "Japanese/IsMatch_KiKu",                // 14
        "Japanese/IsIgnorePunctuation",         // 15
        "Japanese/IsIgnoreWhitespace",          // 16
        "Japanese/IsIgnoreProlongedSoundMark",  // 17
        "Japanese/IsIgnoreMiddleDot"            // 18
    };

    const int nCount = sizeof( aTranslitNames ) / sizeof( aTranslitNames[0] );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for (INT32 i = 0;  i < nCount;  ++i)
        pNames[i] = OUString::createFromAscii( aTranslitNames[i] );

    return aNames;
}

// -----------------------------------------------------------------------
SvxSearchItem::SvxSearchItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId ),
    ConfigItem( OUString::createFromAscii( CFG_ROOT_NODE ) ),

    aSearchOpt      (   SearchAlgorithms_ABSOLUTE,
                        SearchFlags::ALL_IGNORE_CASE | SearchFlags::LEV_RELAXED,
                          OUString(),
                          OUString(),
                          Locale(),
                          2, 2, 2,
                          TransliterationModules_IGNORE_CASE ),
    nCommand        ( 0 ),
    bBackward       ( sal_False ),
    bPattern        ( sal_False ),
    bContent        ( sal_False ),
    eFamily         ( SFX_STYLE_FAMILY_PARA ),
    bRowDirection   ( sal_True ),
    bAllTables      ( sal_False ),
    nCellType       ( SVX_SEARCHIN_FORMULA ),
    nAppFlag        ( SVX_SEARCHAPP_WRITER ),
    bAsianOptions   ( FALSE )
{
    EnableNotification( lcl_GetNotifyNames() );

    SvtSearchOptions aOpt;

    bBackward       = aOpt.IsBackwards();
    bAsianOptions   = aOpt.IsUseAsianOptions();

    if (aOpt.IsUseRegularExpression())
        aSearchOpt.algorithmType = SearchAlgorithms_REGEXP;
    if (aOpt.IsSimilaritySearch())
        aSearchOpt.algorithmType = SearchAlgorithms_APPROXIMATE;
    if (aOpt.IsWholeWordsOnly())
        aSearchOpt.searchFlag |= SearchFlags::NORM_WORD_ONLY;

    INT32 &rFlags = aSearchOpt.transliterateFlags;

    if (!aOpt.IsMatchCase())
    {
        aSearchOpt.searchFlag |= SearchFlags::ALL_IGNORE_CASE;
        rFlags |= TransliterationModules_IGNORE_CASE;
    }

    if (!aOpt.IsMatchFullHalfWidthForms())
        rFlags |= TransliterationModules_IGNORE_WIDTH;
    if (!aOpt.IsMatchHiraganaKatakana())
        rFlags |= TransliterationModules_IGNORE_KANA;
    if (!aOpt.IsMatchContractions())
        rFlags |= TransliterationModules_ignoreSize_ja_JP;
    if (!aOpt.IsMatchMinusDashChoon())
        rFlags |= TransliterationModules_ignoreMinusSign_ja_JP;
    if (!aOpt.IsMatchRepeatCharMarks())
        rFlags |= TransliterationModules_ignoreIterationMark_ja_JP;
    if (!aOpt.IsMatchVariantFormKanji())
        rFlags |= TransliterationModules_ignoreTraditionalKanji_ja_JP;
    if (!aOpt.IsMatchOldKanaForms())
        rFlags |= TransliterationModules_ignoreTraditionalKana_ja_JP;
    if (!aOpt.IsMatchDiziDuzu())
        rFlags |= TransliterationModules_ignoreZiZu_ja_JP;
    if (!aOpt.IsMatchBavaHafa())
        rFlags |= TransliterationModules_ignoreBaFa_ja_JP;
    if (!aOpt.IsMatchTsithichiDhizi())
        rFlags |= TransliterationModules_ignoreTiJi_ja_JP;
    if (!aOpt.IsMatchHyuiyuByuvyu())
        rFlags |= TransliterationModules_ignoreHyuByu_ja_JP;
    if (!aOpt.IsMatchSesheZeje())
        rFlags |= TransliterationModules_ignoreSeZe_ja_JP;
    if (!aOpt.IsMatchIaiya())
        rFlags |= TransliterationModules_ignoreIandEfollowedByYa_ja_JP;
    if (!aOpt.IsMatchKiku())
        rFlags |= TransliterationModules_ignoreKiKuFollowedBySa_ja_JP;
    if ( aOpt.IsIgnorePunctuation())
        rFlags |= TransliterationModules_ignoreSeparator_ja_JP;
    if ( aOpt.IsIgnoreWhitespace())
        rFlags |= TransliterationModules_ignoreSpace_ja_JP;
    if ( aOpt.IsIgnoreProlongedSoundMark())
        rFlags |= TransliterationModules_ignoreProlongedSoundMark_ja_JP;
    if ( aOpt.IsIgnoreMiddleDot())
        rFlags |= TransliterationModules_ignoreMiddleDot_ja_JP;
}

// -----------------------------------------------------------------------

SvxSearchItem::SvxSearchItem( const SvxSearchItem& rItem ) :

    SfxPoolItem ( rItem ),
    ConfigItem( OUString::createFromAscii( CFG_ROOT_NODE ) ),

    aSearchOpt      ( rItem.aSearchOpt ),
    nCommand        ( rItem.nCommand ),
    bBackward       ( rItem.bBackward ),
    bPattern        ( rItem.bPattern ),
    bContent        ( rItem.bContent ),
    eFamily         ( rItem.eFamily ),
    bRowDirection   ( rItem.bRowDirection ),
    bAllTables      ( rItem.bAllTables ),
    nCellType       ( rItem.nCellType ),
    nAppFlag        ( rItem.nAppFlag ),
    bAsianOptions   ( rItem.bAsianOptions )
{
    EnableNotification( lcl_GetNotifyNames() );
}

// -----------------------------------------------------------------------

SvxSearchItem::~SvxSearchItem()
{
}

// -----------------------------------------------------------------------
SfxPoolItem* SvxSearchItem::Clone( SfxItemPool *pPool) const
{
    return new SvxSearchItem(*this);
}

// -----------------------------------------------------------------------

//! used below
static BOOL operator == ( const SearchOptions& rItem1, const SearchOptions& rItem2 )
{
    return rItem1.algorithmType         == rItem2.algorithmType &&
           rItem1.searchFlag            == rItem2.searchFlag    &&
           rItem1.searchString          == rItem2.searchString  &&
           rItem1.replaceString         == rItem2.replaceString &&
           //rItem1.Locale              == rItem2.Locale        &&
           rItem1.changedChars          == rItem2.changedChars  &&
           rItem1.deletedChars          == rItem2.deletedChars  &&
           rItem1.insertedChars         == rItem2.insertedChars &&
           rItem1.transliterateFlags    == rItem2.transliterateFlags;
}


int SvxSearchItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal which or type" );
    const SvxSearchItem &rSItem = (SvxSearchItem &) rItem;
    return ( nCommand       == rSItem.nCommand )        &&
           ( bBackward      == rSItem.bBackward )       &&
           ( bPattern       == rSItem.bPattern )        &&
           ( bContent       == rSItem.bContent )        &&
           ( eFamily        == rSItem.eFamily )         &&
           ( bRowDirection  == rSItem.bRowDirection )   &&
           ( bAllTables     == rSItem.bAllTables )      &&
           ( nCellType      == rSItem.nCellType )       &&
           ( nAppFlag       == rSItem.nAppFlag )        &&
           ( bAsianOptions  == rSItem.bAsianOptions )   &&
           ( aSearchOpt     == rSItem.aSearchOpt );
}


//------------------------------------------------------------------------

SfxItemPresentation SvxSearchItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText,
    const International *
)   const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

void SvxSearchItem::GetFromDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& rDescr )
{
    SetSearchString( rDescr->getSearchString() );
    ::com::sun::star::uno::Any aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchWords") );
    sal_Bool bTemp ;
    aAny >>= bTemp ;
    SetWordOnly( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchCaseSensitive") );
    aAny >>= bTemp ;
    SetExact( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchBackwards") );
    aAny >>= bTemp ;
    SetBackward( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchInSelection") );
    aAny >>= bTemp ;
    SetSelection( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchRegularExpression") );
    aAny >>= bTemp ;
    SetRegExp( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarity") );
    aAny >>= bTemp ;
    SetLevenshtein( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityRelax") );
    aAny >>= bTemp ;
    SetLEVRelaxed( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityExchange") );
    sal_Int16 nTemp ;
    aAny >>= nTemp ;
    SetLEVOther( nTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityRemove") );
    aAny >>= nTemp ;
    SetLEVShorter( nTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityAdd") );
    aAny >>= nTemp ;
    SetLEVLonger( nTemp );
}

void SvxSearchItem::SetToDescriptor( ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > & rDescr )
{
    rDescr->setSearchString( GetSearchString() );
    ::com::sun::star::uno::Any aAny;
    aAny <<= GetWordOnly() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchWords"), aAny );
    aAny <<= GetExact() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchCaseSensitive"), aAny );
    aAny <<= GetBackward() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchBackwards"), aAny );
    aAny <<= GetSelection() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchInSelection"), aAny );
    aAny <<= GetRegExp() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchRegularExpression"), aAny );
    aAny <<= IsLevenshtein() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarity"), aAny );
    aAny <<= IsLEVRelaxed() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityRelax"), aAny );
    aAny <<= GetLEVOther() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityExchange"), aAny );
    aAny <<= GetLEVShorter() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityRemove"), aAny );
    aAny <<= GetLEVLonger() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityAdd"), aAny );
}


void SvxSearchItem::Notify( const Sequence< OUString > &rPropertyNames )
{
    // applies transliteration changes in the configuration database
    // to the current SvxSearchItem
    SetTransliterationFlags( SvtSearchOptions().GetTransliterationFlags() );
}


void SvxSearchItem::SetMatchFullHalfWidthForms( sal_Bool bVal )
{
    if (!bVal)
        aSearchOpt.transliterateFlags |=  TransliterationModules_IGNORE_WIDTH;
    else
        aSearchOpt.transliterateFlags &= ~TransliterationModules_IGNORE_WIDTH;
}


void SvxSearchItem::SetWordOnly( sal_Bool bVal )
{
    if (bVal)
        aSearchOpt.searchFlag |=  SearchFlags::NORM_WORD_ONLY;
    else
        aSearchOpt.searchFlag &= ~SearchFlags::NORM_WORD_ONLY;
}


void SvxSearchItem::SetExact( sal_Bool bVal )
{
    if (!bVal)
    {
        aSearchOpt.searchFlag         |=  SearchFlags::ALL_IGNORE_CASE;
        aSearchOpt.transliterateFlags |=  TransliterationModules_IGNORE_CASE;
    }
    else
    {
        aSearchOpt.searchFlag         &= ~SearchFlags::ALL_IGNORE_CASE;
        aSearchOpt.transliterateFlags &= ~TransliterationModules_IGNORE_CASE;
    }
}


void SvxSearchItem::SetSelection( sal_Bool bVal )
{
    if (bVal)
    {
        aSearchOpt.searchFlag |=  (SearchFlags::REG_NOT_BEGINOFLINE |
                                   SearchFlags::REG_NOT_ENDOFLINE);
    }
    else
    {
        aSearchOpt.searchFlag &= ~(SearchFlags::REG_NOT_BEGINOFLINE |
                                   SearchFlags::REG_NOT_ENDOFLINE);
    }
}


void SvxSearchItem::SetRegExp( sal_Bool bVal )
{
    if (bVal)
        aSearchOpt.algorithmType = SearchAlgorithms_REGEXP;
    else
        aSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
}


void SvxSearchItem::SetLEVRelaxed( sal_Bool bVal )
{
    if (bVal)
        aSearchOpt.searchFlag |=  SearchFlags::LEV_RELAXED;
    else
        aSearchOpt.searchFlag &= ~SearchFlags::LEV_RELAXED;
}


void SvxSearchItem::SetLevenshtein( sal_Bool bVal )
{
    if (bVal)
        aSearchOpt.algorithmType = SearchAlgorithms_APPROXIMATE;
    else
        aSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
}


void SvxSearchItem::SetTransliterationFlags( sal_Int32 nFlags )
{
    aSearchOpt.transliterateFlags = nFlags;
    if (nFlags & TransliterationModules_IGNORE_CASE)
        aSearchOpt.searchFlag |=  SearchFlags::ALL_IGNORE_CASE;
    else
        aSearchOpt.searchFlag &= ~SearchFlags::ALL_IGNORE_CASE;
}


