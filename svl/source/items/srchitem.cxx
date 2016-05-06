/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <sal/log.hxx>
#include <svl/srchitem.hxx>
#include <sal/macros.h>
#include <osl/diagnose.h>

#include <unotools/searchopt.hxx>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XSearchable.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/util/XPropertyReplace.hpp>
#include <com/sun/star/util/XReplaceDescriptor.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <svl/memberid.hrc>
#include <i18nlangtag/languagetag.hxx>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>

using namespace utl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::i18n;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;

#define CFG_ROOT_NODE       "Office.Common/SearchOptions"

#define SRCH_PARAMS         13
#define SRCH_PARA_OPTIONS   "Options"
#define SRCH_PARA_FAMILY    "Family"
#define SRCH_PARA_COMMAND   "Command"
#define SRCH_PARA_CELLTYPE  "CellType"
#define SRCH_PARA_APPFLAG   "AppFlag"
#define SRCH_PARA_ROWDIR    "RowDirection"
#define SRCH_PARA_ALLTABLES "AllTables"
#define SRCH_PARA_SEARCHFILTERED "SearchFiltered"
#define SRCH_PARA_SEARCHFORMATTED  "SearchFormatted"
#define SRCH_PARA_BACKWARD  "Backward"
#define SRCH_PARA_PATTERN   "Pattern"
#define SRCH_PARA_CONTENT   "Content"
#define SRCH_PARA_ASIANOPT  "AsianOptions"

SfxPoolItem* SvxSearchItem::CreateDefault() { return new  SvxSearchItem(0);}


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
        "Japanese/IsIgnoreMiddleDot",           // 18
        "IsIgnoreDiacritics_CTL",               // 19
        "IsIgnoreKashida_CTL"                   // 20
    };

    const int nCount = SAL_N_ELEMENTS( aTranslitNames );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for (sal_Int32 i = 0;  i < nCount;  ++i)
        pNames[i] = OUString::createFromAscii( aTranslitNames[i] );

    return aNames;
}


SvxSearchItem::SvxSearchItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId ),
    ConfigItem( OUString(CFG_ROOT_NODE ) ),

    m_aSearchOpt      (   SearchAlgorithms_ABSOLUTE,
                        SearchFlags::LEV_RELAXED,
                        OUString(),
                        OUString(),
                        lang::Locale(),
                          2, 2, 2,
                          TransliterationModules_IGNORE_CASE,
                          SearchAlgorithms2::ABSOLUTE, '\\' ),
    m_eFamily         ( SfxStyleFamily::Para ),
    m_nCommand        ( SvxSearchCmd::FIND ),
    m_nCellType       ( SvxSearchCellType::FORMULA ),
    m_nAppFlag        ( SvxSearchApp::WRITER ),
    m_bRowDirection   ( true ),
    m_bAllTables      ( false ),
    m_bSearchFiltered ( false ),
    m_bSearchFormatted( false ),
    m_bNotes          ( false),
    m_bBackward       ( false ),
    m_bPattern        ( false ),
    m_bContent        ( false ),
    m_bAsianOptions   ( false ),
    m_nStartPointX(0),
    m_nStartPointY(0)
{
    EnableNotification( lcl_GetNotifyNames() );

    SvtSearchOptions aOpt;

    m_bBackward       = aOpt.IsBackwards();
    m_bAsianOptions   = aOpt.IsUseAsianOptions();
    m_bNotes = aOpt.IsNotes();

    if (aOpt.IsUseWildcard())
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::WILDCARD;
        m_aSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE; // something valid
    }
    if (aOpt.IsUseRegularExpression())
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::REGEXP;
        m_aSearchOpt.algorithmType = SearchAlgorithms_REGEXP;
    }
    if (aOpt.IsSimilaritySearch())
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::APPROXIMATE;
        m_aSearchOpt.algorithmType = SearchAlgorithms_APPROXIMATE;
    }
    if (aOpt.IsWholeWordsOnly())
        m_aSearchOpt.searchFlag |= SearchFlags::NORM_WORD_ONLY;

    sal_Int32 &rFlags = m_aSearchOpt.transliterateFlags;

    if (!aOpt.IsMatchCase())
        rFlags |= TransliterationModules_IGNORE_CASE;
    if ( aOpt.IsMatchFullHalfWidthForms())
        rFlags |= TransliterationModules_IGNORE_WIDTH;
    if ( aOpt.IsIgnoreDiacritics_CTL())
        rFlags |= TransliterationModulesExtra::IGNORE_DIACRITICS_CTL ;
    if ( aOpt.IsIgnoreKashida_CTL())
        rFlags |= TransliterationModulesExtra::IGNORE_KASHIDA_CTL ;
    if ( m_bAsianOptions )
    {
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

}


SvxSearchItem::SvxSearchItem( const SvxSearchItem& rItem ) :

    SfxPoolItem ( rItem ),
    ConfigItem( OUString(CFG_ROOT_NODE ) ),

    m_aSearchOpt      ( rItem.m_aSearchOpt ),
    m_eFamily         ( rItem.m_eFamily ),
    m_nCommand        ( rItem.m_nCommand ),
    m_nCellType       ( rItem.m_nCellType ),
    m_nAppFlag        ( rItem.m_nAppFlag ),
    m_bRowDirection   ( rItem.m_bRowDirection ),
    m_bAllTables      ( rItem.m_bAllTables ),
    m_bSearchFiltered   ( rItem.m_bSearchFiltered ),
    m_bSearchFormatted  ( rItem.m_bSearchFormatted ),
    m_bNotes          ( rItem.m_bNotes),
    m_bBackward       ( rItem.m_bBackward ),
    m_bPattern        ( rItem.m_bPattern ),
    m_bContent        ( rItem.m_bContent ),
    m_bAsianOptions   ( rItem.m_bAsianOptions ),
    m_nStartPointX(rItem.m_nStartPointX),
    m_nStartPointY(rItem.m_nStartPointY)
{
    EnableNotification( lcl_GetNotifyNames() );
}


SvxSearchItem::~SvxSearchItem()
{
}


SfxPoolItem* SvxSearchItem::Clone( SfxItemPool *) const
{
    return new SvxSearchItem(*this);
}


//! used below
static bool equalsWithoutLocale( const SearchOptions2& rItem1, const SearchOptions2& rItem2 )
{
    return rItem1.algorithmType         == rItem2.algorithmType &&
           rItem1.searchFlag            == rItem2.searchFlag    &&
           rItem1.searchString          == rItem2.searchString  &&
           rItem1.replaceString         == rItem2.replaceString &&
           //rItem1.Locale              == rItem2.Locale        &&
           rItem1.changedChars          == rItem2.changedChars  &&
           rItem1.deletedChars          == rItem2.deletedChars  &&
           rItem1.insertedChars         == rItem2.insertedChars &&
           rItem1.transliterateFlags    == rItem2.transliterateFlags &&
           rItem1.AlgorithmType2        == rItem2.AlgorithmType2 &&
           rItem1.WildcardEscapeCharacter == rItem2.WildcardEscapeCharacter;
}


bool SvxSearchItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal which or type" );
    const SvxSearchItem &rSItem = static_cast<const SvxSearchItem &>(rItem);
    return ( m_nCommand       == rSItem.m_nCommand )        &&
           ( m_bBackward      == rSItem.m_bBackward )       &&
           ( m_bPattern       == rSItem.m_bPattern )        &&
           ( m_bContent       == rSItem.m_bContent )        &&
           ( m_eFamily        == rSItem.m_eFamily )         &&
           ( m_bRowDirection  == rSItem.m_bRowDirection )   &&
           ( m_bAllTables     == rSItem.m_bAllTables )      &&
           ( m_bSearchFiltered  == rSItem.m_bSearchFiltered )   &&
           ( m_bSearchFormatted == rSItem.m_bSearchFormatted )  &&
           ( m_nCellType      == rSItem.m_nCellType )       &&
           ( m_nAppFlag       == rSItem.m_nAppFlag )        &&
           ( m_bAsianOptions  == rSItem.m_bAsianOptions )   &&
           ( equalsWithoutLocale(m_aSearchOpt,rSItem.m_aSearchOpt )) &&
           ( m_bNotes         == rSItem.m_bNotes );
}


bool SvxSearchItem::GetPresentation
(
    SfxItemPresentation ,
    SfxMapUnit          ,
    SfxMapUnit          ,
    OUString&           ,
    const IntlWrapper *
)   const
{
    return false;
}

void SvxSearchItem::Notify( const Sequence< OUString > & )
{
    // applies transliteration changes in the configuration database
    // to the current SvxSearchItem
    SetTransliterationFlags( SvtSearchOptions().GetTransliterationFlags() );
}

void SvxSearchItem::ImplCommit()
{
}

void SvxSearchItem::SetMatchFullHalfWidthForms( bool bVal )
{
    if (bVal)
        m_aSearchOpt.transliterateFlags |=  TransliterationModules_IGNORE_WIDTH;
    else
        m_aSearchOpt.transliterateFlags &= ~TransliterationModules_IGNORE_WIDTH;
}


void SvxSearchItem::SetWordOnly( bool bVal )
{
    if (bVal)
        m_aSearchOpt.searchFlag |=  SearchFlags::NORM_WORD_ONLY;
    else
        m_aSearchOpt.searchFlag &= ~SearchFlags::NORM_WORD_ONLY;
}


void SvxSearchItem::SetExact( bool bVal )
{
    if (!bVal)
        m_aSearchOpt.transliterateFlags |=  TransliterationModules_IGNORE_CASE;
    else
        m_aSearchOpt.transliterateFlags &= ~TransliterationModules_IGNORE_CASE;
}


void SvxSearchItem::SetSelection( bool bVal )
{
    if (bVal)
    {
        m_aSearchOpt.searchFlag |=  (SearchFlags::REG_NOT_BEGINOFLINE |
                                   SearchFlags::REG_NOT_ENDOFLINE);
    }
    else
    {
        m_aSearchOpt.searchFlag &= ~(SearchFlags::REG_NOT_BEGINOFLINE |
                                   SearchFlags::REG_NOT_ENDOFLINE);
    }
}


void SvxSearchItem::SetRegExp( bool bVal )
{
    if ( bVal )
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::REGEXP;
        m_aSearchOpt.algorithmType = SearchAlgorithms_REGEXP;
    }
    else if ( SearchAlgorithms2::REGEXP == m_aSearchOpt.AlgorithmType2 )
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::ABSOLUTE;
        m_aSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
    }
}


void SvxSearchItem::SetWildcard( bool bVal )
{
    if ( bVal )
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::WILDCARD;
        m_aSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE; // something valid
    }
    else if ( SearchAlgorithms2::REGEXP == m_aSearchOpt.AlgorithmType2 )
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::ABSOLUTE;
        m_aSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
    }
}


void SvxSearchItem::SetLEVRelaxed( bool bVal )
{
    if (bVal)
        m_aSearchOpt.searchFlag |=  SearchFlags::LEV_RELAXED;
    else
        m_aSearchOpt.searchFlag &= ~SearchFlags::LEV_RELAXED;
}


void SvxSearchItem::SetLevenshtein( bool bVal )
{
    if ( bVal )
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::APPROXIMATE;
        m_aSearchOpt.algorithmType = SearchAlgorithms_APPROXIMATE;
    }
    else if ( SearchAlgorithms2::APPROXIMATE == m_aSearchOpt.AlgorithmType2 )
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::ABSOLUTE;
        m_aSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
    }
}


void SvxSearchItem::SetTransliterationFlags( sal_Int32 nFlags )
{
    m_aSearchOpt.transliterateFlags = nFlags;
}

bool SvxSearchItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0 :
        {
            Sequence< PropertyValue > aSeq( SRCH_PARAMS );
            aSeq[0].Name = SRCH_PARA_OPTIONS;
            aSeq[0].Value <<= m_aSearchOpt;
            aSeq[1].Name = SRCH_PARA_FAMILY;
            aSeq[1].Value <<= sal_Int16( m_eFamily );
            aSeq[2].Name = SRCH_PARA_COMMAND;
            aSeq[2].Value <<= static_cast<sal_uInt16>(m_nCommand);
            aSeq[3].Name = SRCH_PARA_CELLTYPE;
            aSeq[3].Value <<= static_cast<sal_uInt16>(m_nCellType);
            aSeq[4].Name = SRCH_PARA_APPFLAG;
            aSeq[4].Value <<= static_cast<sal_uInt16>(m_nAppFlag);
            aSeq[5].Name = SRCH_PARA_ROWDIR;
            aSeq[5].Value <<= m_bRowDirection;
            aSeq[6].Name = SRCH_PARA_ALLTABLES;
            aSeq[6].Value <<= m_bAllTables;
            aSeq[7].Name = SRCH_PARA_SEARCHFILTERED;
            aSeq[7].Value <<= m_bSearchFiltered;
            aSeq[8].Name = SRCH_PARA_SEARCHFORMATTED;
            aSeq[8].Value <<= m_bSearchFormatted;
            aSeq[9].Name = SRCH_PARA_BACKWARD;
            aSeq[9].Value <<= m_bBackward;
            aSeq[10].Name = SRCH_PARA_PATTERN;
            aSeq[10].Value <<= m_bPattern;
            aSeq[11].Name = SRCH_PARA_CONTENT;
            aSeq[11].Value <<= m_bContent;
            aSeq[12].Name = SRCH_PARA_ASIANOPT;
            aSeq[12].Value <<= m_bAsianOptions;
            rVal <<= aSeq;
        }
        break;
        case MID_SEARCH_COMMAND:
            rVal <<= (sal_Int16) m_nCommand; break;
        case MID_SEARCH_STYLEFAMILY:
            rVal <<= (sal_Int16) m_eFamily; break;
        case MID_SEARCH_CELLTYPE:
            rVal <<= (sal_Int32) m_nCellType; break;
        case MID_SEARCH_ROWDIRECTION:
            rVal <<= m_bRowDirection; break;
        case MID_SEARCH_ALLTABLES:
            rVal <<= m_bAllTables; break;
        case MID_SEARCH_SEARCHFILTERED:
            rVal <<= m_bSearchFiltered; break;
        case MID_SEARCH_SEARCHFORMATTED:
            rVal <<= m_bSearchFormatted; break;
        case MID_SEARCH_BACKWARD:
            rVal <<= m_bBackward; break;
        case MID_SEARCH_PATTERN:
            rVal <<= m_bPattern; break;
        case MID_SEARCH_CONTENT:
            rVal <<= m_bContent; break;
        case MID_SEARCH_ASIANOPTIONS:
            rVal <<= m_bAsianOptions; break;
        case MID_SEARCH_ALGORITHMTYPE:
            rVal <<= (sal_Int16) m_aSearchOpt.algorithmType; break;
        case MID_SEARCH_ALGORITHMTYPE2:
            rVal <<= m_aSearchOpt.AlgorithmType2; break;
        case MID_SEARCH_FLAGS:
            rVal <<= m_aSearchOpt.searchFlag; break;
        case MID_SEARCH_SEARCHSTRING:
            rVal <<= m_aSearchOpt.searchString; break;
        case MID_SEARCH_REPLACESTRING:
            rVal <<= m_aSearchOpt.replaceString; break;
        case MID_SEARCH_CHANGEDCHARS:
            rVal <<= m_aSearchOpt.changedChars; break;
        case MID_SEARCH_DELETEDCHARS:
            rVal <<= m_aSearchOpt.deletedChars; break;
        case MID_SEARCH_INSERTEDCHARS:
            rVal <<= m_aSearchOpt.insertedChars; break;
        case MID_SEARCH_TRANSLITERATEFLAGS:
            rVal <<= m_aSearchOpt.transliterateFlags; break;
        case MID_SEARCH_LOCALE:
        {
            sal_Int16 nLocale;
            if (!m_aSearchOpt.Locale.Language.isEmpty() || !m_aSearchOpt.Locale.Country.isEmpty() )
                nLocale = LanguageTag::convertToLanguageType( m_aSearchOpt.Locale );
            else
                nLocale = LANGUAGE_NONE;
            rVal <<= nLocale;
            break;
        }

        default:
            SAL_WARN( "svl.items", "SvxSearchItem::QueryValue(): Unknown MemberId" );
            return false;
    }

    return true;
}


bool SvxSearchItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = false;
    sal_Int32 nInt = 0;
    switch ( nMemberId )
    {
        case 0 :
        {
            Sequence< PropertyValue > aSeq;
            if ( ( rVal >>= aSeq ) && ( aSeq.getLength() == SRCH_PARAMS ) )
            {
                sal_Int16 nConvertedCount( 0 );
                for ( sal_Int32 i = 0; i < aSeq.getLength(); ++i )
                {
                    if ( aSeq[i].Name == SRCH_PARA_OPTIONS )
                    {
                        if ( aSeq[i].Value >>= m_aSearchOpt )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_FAMILY )
                    {
                        sal_uInt16 nTemp( 0 );
                        if ( aSeq[i].Value >>= nTemp )
                        {
                            m_eFamily = SfxStyleFamily( nTemp );
                            ++nConvertedCount;
                        }
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_COMMAND )
                    {
                        sal_uInt16 nTmp;
                        if ( aSeq[i].Value >>= nTmp )
                        {
                            m_nCommand = static_cast<SvxSearchCmd>(nTmp);
                            ++nConvertedCount;
                        }
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_CELLTYPE )
                    {
                        sal_uInt16 nTmp;
                        if ( aSeq[i].Value >>= nTmp )
                        {
                            m_nCellType = static_cast<SvxSearchCellType>(nTmp);
                            ++nConvertedCount;
                        }
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_APPFLAG )
                    {
                        sal_uInt16 nTmp;
                        if ( aSeq[i].Value >>= nTmp )
                        {
                            m_nAppFlag = static_cast<SvxSearchApp>(nTmp);
                            ++nConvertedCount;
                        }
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_ROWDIR )
                    {
                        if ( aSeq[i].Value >>= m_bRowDirection )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_ALLTABLES )
                    {
                        if ( aSeq[i].Value >>= m_bAllTables )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_SEARCHFILTERED )
                    {
                        if ( aSeq[i].Value >>= m_bSearchFiltered )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_SEARCHFORMATTED )
                    {
                        if ( aSeq[i].Value >>= m_bSearchFormatted )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_BACKWARD )
                    {
                        if ( aSeq[i].Value >>= m_bBackward )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_PATTERN )
                    {
                        if ( aSeq[i].Value >>= m_bPattern )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_CONTENT )
                    {
                        if ( aSeq[i].Value >>= m_bContent )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name == SRCH_PARA_ASIANOPT )
                    {
                        if ( aSeq[i].Value >>= m_bAsianOptions )
                            ++nConvertedCount;
                    }
                }

                bRet = ( nConvertedCount == SRCH_PARAMS );
            }
            break;
        }
        case MID_SEARCH_COMMAND:
            bRet = (rVal >>= nInt); m_nCommand = static_cast<SvxSearchCmd>(nInt); break;
        case MID_SEARCH_STYLEFAMILY:
            bRet = (rVal >>= nInt); m_eFamily =  (SfxStyleFamily) (sal_Int16) nInt; break;
        case MID_SEARCH_CELLTYPE:
            bRet = (rVal >>= nInt); m_nCellType = static_cast<SvxSearchCellType>(nInt); break;
        case MID_SEARCH_ROWDIRECTION:
            bRet = (rVal >>= m_bRowDirection); break;
        case MID_SEARCH_ALLTABLES:
            bRet = (rVal >>= m_bAllTables); break;
        case MID_SEARCH_SEARCHFILTERED:
            bRet = (rVal >>= m_bSearchFiltered); break;
        case MID_SEARCH_SEARCHFORMATTED:
            bRet = (rVal >>= m_bSearchFormatted); break;
        case MID_SEARCH_BACKWARD:
            bRet = (rVal >>= m_bBackward); break;
        case MID_SEARCH_PATTERN:
            bRet = (rVal >>= m_bPattern); break;
        case MID_SEARCH_CONTENT:
            bRet = (rVal >>= m_bContent); break;
        case MID_SEARCH_ASIANOPTIONS:
            bRet = (rVal >>= m_bAsianOptions); break;
        case MID_SEARCH_ALGORITHMTYPE:
            bRet = (rVal >>= nInt); m_aSearchOpt.algorithmType = (SearchAlgorithms)(sal_Int16)nInt; break;
        case MID_SEARCH_ALGORITHMTYPE2:
            bRet = (rVal >>= nInt); m_aSearchOpt.AlgorithmType2 = (sal_Int16)nInt; break;
        case MID_SEARCH_FLAGS:
            bRet = (rVal >>= m_aSearchOpt.searchFlag); break;
        case MID_SEARCH_SEARCHSTRING:
            bRet = (rVal >>= m_aSearchOpt.searchString); break;
        case MID_SEARCH_REPLACESTRING:
            bRet = (rVal >>= m_aSearchOpt.replaceString); break;
        case MID_SEARCH_CHANGEDCHARS:
            bRet = (rVal >>= m_aSearchOpt.changedChars); break;
        case MID_SEARCH_DELETEDCHARS:
            bRet = (rVal >>= m_aSearchOpt.deletedChars); break;
        case MID_SEARCH_INSERTEDCHARS:
            bRet = (rVal >>= m_aSearchOpt.insertedChars); break;
        case MID_SEARCH_TRANSLITERATEFLAGS:
            bRet = (rVal >>= m_aSearchOpt.transliterateFlags); break;
        case MID_SEARCH_LOCALE:
        {
            bRet = (rVal >>= nInt);
            if ( bRet )
            {
                if ( nInt == LANGUAGE_NONE )
                {
                    m_aSearchOpt.Locale = css::lang::Locale();
                }
                else
                {
                    m_aSearchOpt.Locale = LanguageTag::convertToLocale( nInt);
                }
            }
            break;
        }
        case MID_SEARCH_STARTPOINTX:
        {
            bRet = (rVal >>= m_nStartPointX);
            break;
        }
        case MID_SEARCH_STARTPOINTY:
        {
            bRet = (rVal >>= m_nStartPointY);
            break;
        }
        default:
            OSL_FAIL( "Unknown MemberId" );
    }

    return bRet;
}

sal_Int32 SvxSearchItem::GetStartPointX() const
{
    return m_nStartPointX;
}

sal_Int32 SvxSearchItem::GetStartPointY() const
{
    return m_nStartPointY;
}

bool SvxSearchItem::HasStartPoint() const
{
    return m_nStartPointX > 0 || m_nStartPointY > 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
