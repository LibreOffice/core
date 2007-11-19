/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: srchitem.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 17:27:19 $
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
#include "precompiled_sfx2.hxx"

// include ---------------------------------------------------------------

#ifndef __SBX_SBXVARIABLE_HXX
#include <basic/sbxvar.hxx>
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
#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif

#ifndef GCC
#endif

#define _SVX_SRCHITEM_CXX

#include <sfx2/sfxsids.hrc>
#define ITEMID_SEARCH   SID_SEARCH_ITEM
#include <sfx2/srchitem.hxx>

#include <sfx2/sfxuno.hxx>

//using namespace uno;
using namespace rtl;
using namespace utl;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace com::sun::star::uno;

#define CFG_ROOT_NODE   "Office.Common/SearchOptions"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1_FACTORY(SvxSearchItem, SfxPoolItem, new SvxSearchItem(0));

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
                        SearchFlags::LEV_RELAXED,
                          OUString(),
                          OUString(),
                          Locale(),
                          2, 2, 2,
                          TransliterationModules_IGNORE_CASE ),
    eFamily         ( SFX_STYLE_FAMILY_PARA ),
    nCommand        ( 0 ),
    nCellType       ( SVX_SEARCHIN_FORMULA ),
    nAppFlag        ( SVX_SEARCHAPP_WRITER ),
    bRowDirection   ( sal_True ),
    bAllTables      ( sal_False ),
    bBackward       ( sal_False ),
    bPattern        ( sal_False ),
    bContent        ( sal_False ),
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
        rFlags |= TransliterationModules_IGNORE_CASE;
    if ( aOpt.IsMatchFullHalfWidthForms())
        rFlags |= TransliterationModules_IGNORE_WIDTH;
    if ( aOpt.IsMatchHiraganaKatakana())
        rFlags |= TransliterationModules_IGNORE_KANA;
    if ( aOpt.IsMatchContractions())
        rFlags |= TransliterationModules_ignoreSize_ja_JP;
    if ( aOpt.IsMatchMinusDashChoon())
        rFlags |= TransliterationModules_ignoreMinusSign_ja_JP;
    if ( aOpt.IsMatchRepeatCharMarks())
        rFlags |= TransliterationModules_ignoreIterationMark_ja_JP;
    if ( aOpt.IsMatchVariantFormKanji())
        rFlags |= TransliterationModules_ignoreTraditionalKanji_ja_JP;
    if ( aOpt.IsMatchOldKanaForms())
        rFlags |= TransliterationModules_ignoreTraditionalKana_ja_JP;
    if ( aOpt.IsMatchDiziDuzu())
        rFlags |= TransliterationModules_ignoreZiZu_ja_JP;
    if ( aOpt.IsMatchBavaHafa())
        rFlags |= TransliterationModules_ignoreBaFa_ja_JP;
    if ( aOpt.IsMatchTsithichiDhizi())
        rFlags |= TransliterationModules_ignoreTiJi_ja_JP;
    if ( aOpt.IsMatchHyuiyuByuvyu())
        rFlags |= TransliterationModules_ignoreHyuByu_ja_JP;
    if ( aOpt.IsMatchSesheZeje())
        rFlags |= TransliterationModules_ignoreSeZe_ja_JP;
    if ( aOpt.IsMatchIaiya())
        rFlags |= TransliterationModules_ignoreIandEfollowedByYa_ja_JP;
    if ( aOpt.IsMatchKiku())
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
    eFamily         ( rItem.eFamily ),
    nCommand        ( rItem.nCommand ),
    nCellType       ( rItem.nCellType ),
    nAppFlag        ( rItem.nAppFlag ),
    bRowDirection   ( rItem.bRowDirection ),
    bAllTables      ( rItem.bAllTables ),
    bBackward       ( rItem.bBackward ),
    bPattern        ( rItem.bPattern ),
    bContent        ( rItem.bContent ),
    bAsianOptions   ( rItem.bAsianOptions )
{
    EnableNotification( lcl_GetNotifyNames() );
}

// -----------------------------------------------------------------------

SvxSearchItem::~SvxSearchItem()
{
}

// -----------------------------------------------------------------------
SfxPoolItem* SvxSearchItem::Clone( SfxItemPool *) const
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
    SfxItemPresentation ,
    SfxMapUnit          ,
    SfxMapUnit          ,
    XubString&          ,
    const IntlWrapper *
)   const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

void SvxSearchItem::GetFromDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& rDescr )
{
    SetSearchString( rDescr->getSearchString() );
    ::com::sun::star::uno::Any aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchWords") );
    sal_Bool bTemp = false;
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
    sal_Int16 nTemp = 0;
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


void SvxSearchItem::Notify( const Sequence< OUString > & )
{
    // applies transliteration changes in the configuration database
    // to the current SvxSearchItem
    SetTransliterationFlags( SvtSearchOptions().GetTransliterationFlags() );
}


void SvxSearchItem::SetMatchFullHalfWidthForms( sal_Bool bVal )
{
    if (bVal)
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
        aSearchOpt.transliterateFlags |=  TransliterationModules_IGNORE_CASE;
    else
        aSearchOpt.transliterateFlags &= ~TransliterationModules_IGNORE_CASE;
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
}

sal_Bool SvxSearchItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        {
            com::sun::star::util::SearchOptions aSearchOptions;
            rVal <<= aSearchOpt;
            break;
        }
        case MID_SEARCH_COMMAND:
            rVal <<= (sal_Int16) nCommand; break;
        case MID_SEARCH_STYLEFAMILY:
            rVal <<= (sal_Int16) eFamily; break;
        case MID_SEARCH_CELLTYPE:
            rVal <<= (sal_Int32) nCellType; break;
        case MID_SEARCH_ROWDIRECTION:
            rVal <<= (sal_Bool) bRowDirection; break;
        case MID_SEARCH_ALLTABLES:
            rVal <<= (sal_Bool) bAllTables; break;
        case MID_SEARCH_BACKWARD:
            rVal <<= (sal_Bool) bBackward; break;
        case MID_SEARCH_PATTERN:
            rVal <<= (sal_Bool) bPattern; break;
        case MID_SEARCH_CONTENT:
            rVal <<= (sal_Bool) bContent; break;
        case MID_SEARCH_ASIANOPTIONS:
            rVal <<= (sal_Bool) bAsianOptions; break;
        case MID_SEARCH_ALGORITHMTYPE:
            rVal <<= (sal_Int16) aSearchOpt.algorithmType; break;
        case MID_SEARCH_FLAGS:
            rVal <<= aSearchOpt.searchFlag; break;
        case MID_SEARCH_SEARCHSTRING:
            rVal <<= aSearchOpt.searchString; break;
        case MID_SEARCH_REPLACESTRING:
            rVal <<= aSearchOpt.replaceString; break;
        case MID_SEARCH_CHANGEDCHARS:
            rVal <<= aSearchOpt.changedChars; break;
        case MID_SEARCH_DELETEDCHARS:
            rVal <<= aSearchOpt.deletedChars; break;
        case MID_SEARCH_INSERTEDCHARS:
            rVal <<= aSearchOpt.insertedChars; break;
        case MID_SEARCH_TRANSLITERATEFLAGS:
            rVal <<= aSearchOpt.transliterateFlags; break;
        case MID_SEARCH_LOCALE:
        {
            sal_Int16 nLocale;
            if (aSearchOpt.Locale.Language.getLength() || aSearchOpt.Locale.Country.getLength() )
                nLocale = MsLangId::convertLocaleToLanguage( aSearchOpt.Locale );
            else
                nLocale = LANGUAGE_NONE;
            rVal <<= nLocale;
            break;
        }
        default:
            DBG_ERROR( "Unknown MemberId" );
            return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxSearchItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_False;
    sal_Int32 nInt = 0;
    switch ( nMemberId )
    {
        case 0:
        {
            com::sun::star::util::SearchOptions aSearchOptions;
            bRet = rVal >>= aSearchOptions;
            aSearchOpt = aSearchOptions;
            break;
        }
        case MID_SEARCH_COMMAND:
            bRet = (rVal >>= nInt); nCommand = (sal_uInt16) nInt; break;
        case MID_SEARCH_STYLEFAMILY:
            bRet = (rVal >>= nInt); eFamily =  (SfxStyleFamily) (sal_Int16) nInt; break;
        case MID_SEARCH_CELLTYPE:
            bRet = (rVal >>= nInt); nCellType = (sal_uInt16) nInt; break;
        case MID_SEARCH_ROWDIRECTION:
            bRet = (rVal >>= bRowDirection); break;
        case MID_SEARCH_ALLTABLES:
            bRet = (rVal >>= bAllTables); break;
        case MID_SEARCH_BACKWARD:
            bRet = (rVal >>= bBackward); break;
        case MID_SEARCH_PATTERN:
            bRet = (rVal >>= bPattern); break;
        case MID_SEARCH_CONTENT:
            bRet = (rVal >>= bContent); break;
        case MID_SEARCH_ASIANOPTIONS:
            bRet = (rVal >>= bAsianOptions); break;
        case MID_SEARCH_ALGORITHMTYPE:
            bRet = (rVal >>= nInt); aSearchOpt.algorithmType = (::com::sun::star::util::SearchAlgorithms) (sal_Int16) nInt; break;
        case MID_SEARCH_FLAGS:
            bRet = (rVal >>= aSearchOpt.searchFlag); break;
        case MID_SEARCH_SEARCHSTRING:
            bRet = (rVal >>= aSearchOpt.searchString); break;
        case MID_SEARCH_REPLACESTRING:
            bRet = (rVal >>= aSearchOpt.replaceString); break;
        case MID_SEARCH_CHANGEDCHARS:
            bRet = (rVal >>= aSearchOpt.changedChars); break;
        case MID_SEARCH_DELETEDCHARS:
            bRet = (rVal >>= aSearchOpt.deletedChars); break;
        case MID_SEARCH_INSERTEDCHARS:
            bRet = (rVal >>= aSearchOpt.insertedChars); break;
        case MID_SEARCH_TRANSLITERATEFLAGS:
            bRet = (rVal >>= aSearchOpt.transliterateFlags); break;
        case MID_SEARCH_LOCALE:
        {
            bRet = (rVal >>= nInt);
            if ( bRet )
            {
                if ( nInt == LANGUAGE_NONE )
                {
                    aSearchOpt.Locale = ::com::sun::star::lang::Locale();
                }
                else
                {
                    MsLangId::convertLanguageToLocale( (sal_Int16) nInt, aSearchOpt.Locale );
                }
            }
            break;
        }
        default:
            DBG_ERROR( "Unknown MemberId" );
    }

    return bRet;
}


