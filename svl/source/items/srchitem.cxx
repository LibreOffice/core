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

#include <comphelper/propertyvalue.hxx>
#include <unotools/searchopt.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <svl/memberid.h>
#include <i18nlangtag/languagetag.hxx>

#include <unordered_set>

using namespace utl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;

constexpr OUString CFG_ROOT_NODE = u"Office.Common/SearchOptions"_ustr;

#define SRCH_PARAMS         13
constexpr OUString SRCH_PARA_OPTIONS = u"Options"_ustr;
constexpr OUString SRCH_PARA_FAMILY = u"Family"_ustr;
constexpr OUString SRCH_PARA_COMMAND = u"Command"_ustr;
constexpr OUString SRCH_PARA_CELLTYPE = u"CellType"_ustr;
constexpr OUString SRCH_PARA_APPFLAG = u"AppFlag"_ustr;
constexpr OUString SRCH_PARA_ROWDIR = u"RowDirection"_ustr;
constexpr OUString SRCH_PARA_ALLTABLES = u"AllTables"_ustr;
constexpr OUString SRCH_PARA_SEARCHFILTERED = u"SearchFiltered"_ustr;
constexpr OUString SRCH_PARA_SEARCHFORMATTED = u"SearchFormatted"_ustr;
constexpr OUString SRCH_PARA_BACKWARD = u"Backward"_ustr;
constexpr OUString SRCH_PARA_PATTERN = u"Pattern"_ustr;
constexpr OUString SRCH_PARA_CONTENT = u"Content"_ustr;
constexpr OUString SRCH_PARA_ASIANOPT = u"AsianOptions"_ustr;

SfxPoolItem* SvxSearchItem::CreateDefault() { return new  SvxSearchItem(0);}


static Sequence< OUString > lcl_GetNotifyNames()
{
    // names of transliteration relevant properties
    return
    {
        u"IsMatchCase"_ustr,                          //  0
        u"Japanese/IsMatchFullHalfWidthForms"_ustr,   //  1
        u"Japanese/IsMatchHiraganaKatakana"_ustr,     //  2
        u"Japanese/IsMatchContractions"_ustr,         //  3
        u"Japanese/IsMatchMinusDashCho-on"_ustr,      //  4
        u"Japanese/IsMatchRepeatCharMarks"_ustr,      //  5
        u"Japanese/IsMatchVariantFormKanji"_ustr,     //  6
        u"Japanese/IsMatchOldKanaForms"_ustr,         //  7
        u"Japanese/IsMatch_DiZi_DuZu"_ustr,           //  8
        u"Japanese/IsMatch_BaVa_HaFa"_ustr,           //  9
        u"Japanese/IsMatch_TsiThiChi_DhiZi"_ustr,     // 10
        u"Japanese/IsMatch_HyuIyu_ByuVyu"_ustr,       // 11
        u"Japanese/IsMatch_SeShe_ZeJe"_ustr,          // 12
        u"Japanese/IsMatch_IaIya"_ustr,               // 13
        u"Japanese/IsMatch_KiKu"_ustr,                // 14
        u"Japanese/IsIgnorePunctuation"_ustr,         // 15
        u"Japanese/IsIgnoreWhitespace"_ustr,          // 16
        u"Japanese/IsIgnoreProlongedSoundMark"_ustr,  // 17
        u"Japanese/IsIgnoreMiddleDot"_ustr,           // 18
        u"IsIgnoreDiacritics_CTL"_ustr,               // 19
        u"IsIgnoreKashida_CTL"_ustr                   // 20
    };
}


SvxSearchItem::SvxSearchItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId ),
    ConfigItem( CFG_ROOT_NODE ),

    m_aSearchOpt      ( SearchFlags::LEV_RELAXED,
                        OUString(),
                        OUString(),
                        lang::Locale(),
                        2, 2, 2,
                        TransliterationFlags::IGNORE_CASE,
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
    }
    if (aOpt.IsUseRegularExpression())
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::REGEXP;
    }
    if (aOpt.IsSimilaritySearch())
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::APPROXIMATE;
    }
    if (aOpt.IsWholeWordsOnly())
        m_aSearchOpt.searchFlag |= SearchFlags::NORM_WORD_ONLY;

    TransliterationFlags& rFlags = m_aSearchOpt.transliterateFlags;

    if (!aOpt.IsMatchCase())
        rFlags |= TransliterationFlags::IGNORE_CASE;
    if ( aOpt.IsMatchFullHalfWidthForms())
        rFlags |= TransliterationFlags::IGNORE_WIDTH;
    if ( aOpt.IsIgnoreDiacritics_CTL())
        rFlags |= TransliterationFlags::IGNORE_DIACRITICS_CTL ;
    if ( aOpt.IsIgnoreKashida_CTL())
        rFlags |= TransliterationFlags::IGNORE_KASHIDA_CTL ;
    if ( !m_bAsianOptions )
        return;

    if ( aOpt.IsMatchHiraganaKatakana())
        rFlags |= TransliterationFlags::IGNORE_KANA;
    if ( aOpt.IsMatchContractions())
        rFlags |= TransliterationFlags::ignoreSize_ja_JP;
    if ( aOpt.IsMatchMinusDashChoon())
        rFlags |= TransliterationFlags::ignoreMinusSign_ja_JP;
    if ( aOpt.IsMatchRepeatCharMarks())
        rFlags |= TransliterationFlags::ignoreIterationMark_ja_JP;
    if ( aOpt.IsMatchVariantFormKanji())
        rFlags |= TransliterationFlags::ignoreTraditionalKanji_ja_JP;
    if ( aOpt.IsMatchOldKanaForms())
        rFlags |= TransliterationFlags::ignoreTraditionalKana_ja_JP;
    if ( aOpt.IsMatchDiziDuzu())
        rFlags |= TransliterationFlags::ignoreZiZu_ja_JP;
    if ( aOpt.IsMatchBavaHafa())
        rFlags |= TransliterationFlags::ignoreBaFa_ja_JP;
    if ( aOpt.IsMatchTsithichiDhizi())
        rFlags |= TransliterationFlags::ignoreTiJi_ja_JP;
    if ( aOpt.IsMatchHyuiyuByuvyu())
        rFlags |= TransliterationFlags::ignoreHyuByu_ja_JP;
    if ( aOpt.IsMatchSesheZeje())
        rFlags |= TransliterationFlags::ignoreSeZe_ja_JP;
    if ( aOpt.IsMatchIaiya())
        rFlags |= TransliterationFlags::ignoreIandEfollowedByYa_ja_JP;
    if ( aOpt.IsMatchKiku())
        rFlags |= TransliterationFlags::ignoreKiKuFollowedBySa_ja_JP;
    if ( aOpt.IsIgnorePunctuation())
        rFlags |= TransliterationFlags::ignoreSeparator_ja_JP;
    if ( aOpt.IsIgnoreWhitespace())
        rFlags |= TransliterationFlags::ignoreSpace_ja_JP;
    if ( aOpt.IsIgnoreProlongedSoundMark())
        rFlags |= TransliterationFlags::ignoreProlongedSoundMark_ja_JP;
    if ( aOpt.IsIgnoreMiddleDot())
        rFlags |= TransliterationFlags::ignoreMiddleDot_ja_JP;
}


SvxSearchItem::SvxSearchItem( const SvxSearchItem& rItem ) :

    SfxPoolItem ( rItem ),
    ConfigItem( CFG_ROOT_NODE ),

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

SvxSearchItem* SvxSearchItem::Clone( SfxItemPool *) const
{
    return new SvxSearchItem(*this);
}

//! used below
static bool equalsWithoutLocaleOrReplace(const i18nutil::SearchOptions2& rItem1,
                                         const i18nutil::SearchOptions2& rItem2)
{
    return rItem1.searchFlag            == rItem2.searchFlag    &&
           rItem1.searchString          == rItem2.searchString  &&
           //rItem1.replaceString       == rItem2.replaceString &&
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
    assert(SfxPoolItem::operator==(rItem));
    const SvxSearchItem &rSItem = static_cast<const SvxSearchItem &>(rItem);
    return equalsIgnoring(rSItem, /*bIgnoreReplace=*/false, /*bIgnoreCommand=*/false);
}

bool SvxSearchItem::equalsIgnoring(const SvxSearchItem& rSItem, bool bIgnoreReplace,
                                   bool bIgnoreCommand) const
{
    if (!bIgnoreReplace && m_aSearchOpt.replaceString != rSItem.m_aSearchOpt.replaceString)
        return false;
    if (!bIgnoreCommand && m_nCommand != rSItem.m_nCommand)
        return false;

    return ( m_bBackward      == rSItem.m_bBackward )       &&
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
           ( equalsWithoutLocaleOrReplace(m_aSearchOpt, rSItem.m_aSearchOpt )) &&
           ( m_bNotes         == rSItem.m_bNotes );
}


bool SvxSearchItem::GetPresentation
(
    SfxItemPresentation ,
    MapUnit             ,
    MapUnit             ,
    OUString&           ,
    const IntlWrapper&
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
        m_aSearchOpt.transliterateFlags |=  TransliterationFlags::IGNORE_WIDTH;
    else
        m_aSearchOpt.transliterateFlags &= ~TransliterationFlags::IGNORE_WIDTH;
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
        m_aSearchOpt.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    else
        m_aSearchOpt.transliterateFlags &= ~TransliterationFlags::IGNORE_CASE;
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
    }
    else if ( SearchAlgorithms2::REGEXP == m_aSearchOpt.AlgorithmType2 )
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::ABSOLUTE;
    }
}


void SvxSearchItem::SetWildcard( bool bVal )
{
    if ( bVal )
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::WILDCARD;
    }
    else if ( SearchAlgorithms2::WILDCARD == m_aSearchOpt.AlgorithmType2 )
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::ABSOLUTE;
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
    }
    else if ( SearchAlgorithms2::APPROXIMATE == m_aSearchOpt.AlgorithmType2 )
    {
        m_aSearchOpt.AlgorithmType2 = SearchAlgorithms2::ABSOLUTE;
    }
}


void SvxSearchItem::SetTransliterationFlags( TransliterationFlags nFlags )
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
            Sequence<PropertyValue> aSeq{
                comphelper::makePropertyValue(SRCH_PARA_OPTIONS,
                                              m_aSearchOpt.toUnoSearchOptions2()),
                comphelper::makePropertyValue(SRCH_PARA_FAMILY, sal_Int16(m_eFamily)),
                comphelper::makePropertyValue(SRCH_PARA_COMMAND,
                                              static_cast<sal_uInt16>(m_nCommand)),
                comphelper::makePropertyValue(SRCH_PARA_CELLTYPE,
                                              static_cast<sal_uInt16>(m_nCellType)),
                comphelper::makePropertyValue(SRCH_PARA_APPFLAG,
                                              static_cast<sal_uInt16>(m_nAppFlag)),
                comphelper::makePropertyValue(SRCH_PARA_ROWDIR, m_bRowDirection),
                comphelper::makePropertyValue(SRCH_PARA_ALLTABLES, m_bAllTables),
                comphelper::makePropertyValue(SRCH_PARA_SEARCHFILTERED, m_bSearchFiltered),
                comphelper::makePropertyValue(SRCH_PARA_SEARCHFORMATTED, m_bSearchFormatted),
                comphelper::makePropertyValue(SRCH_PARA_BACKWARD, m_bBackward),
                comphelper::makePropertyValue(SRCH_PARA_PATTERN, m_bPattern),
                comphelper::makePropertyValue(SRCH_PARA_CONTENT, m_bContent),
                comphelper::makePropertyValue(SRCH_PARA_ASIANOPT, m_bAsianOptions)
            };
            assert(aSeq.getLength() == SRCH_PARAMS);
            rVal <<= aSeq;
        }
        break;
        case MID_SEARCH_COMMAND:
            rVal <<= static_cast<sal_Int16>(m_nCommand); break;
        case MID_SEARCH_STYLEFAMILY:
            rVal <<= static_cast<sal_Int16>(m_eFamily); break;
        case MID_SEARCH_CELLTYPE:
            rVal <<= static_cast<sal_Int32>(m_nCellType); break;
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
            rVal <<= static_cast<sal_Int16>(i18nutil::downgradeSearchAlgorithms2(m_aSearchOpt.AlgorithmType2)); break;
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
            rVal <<= static_cast<sal_Int32>(m_aSearchOpt.transliterateFlags); break;
        case MID_SEARCH_LOCALE:
        {
            LanguageType nLocale;
            if (!m_aSearchOpt.Locale.Language.isEmpty() || !m_aSearchOpt.Locale.Country.isEmpty() )
                nLocale = LanguageTag::convertToLanguageType( m_aSearchOpt.Locale );
            else
                nLocale = LANGUAGE_NONE;
            rVal <<= static_cast<sal_Int16>(static_cast<sal_uInt16>(nLocale));
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
    auto ExtractNumericAny = [](const css::uno::Any& a, auto& target)
    {
        sal_Int32 nInt;
        if (!(a >>= nInt))
            return false;
        target = static_cast<std::remove_reference_t<decltype(target)>>(nInt);
        return true;
    };
    switch ( nMemberId )
    {
        case 0 :
        {
            Sequence< PropertyValue > aSeq;
            if (!(rVal >>= aSeq) || aSeq.getLength() != SRCH_PARAMS)
                break;
            std::unordered_set<OUString> aConvertedParams;
            for (const auto& rProp : aSeq)
            {
                if (rProp.Name == SRCH_PARA_OPTIONS)
                {
                    if (css::util::SearchOptions2 nTmpSearchOpt2; rProp.Value >>= nTmpSearchOpt2)
                    {
                        m_aSearchOpt = nTmpSearchOpt2;
                        aConvertedParams.insert(rProp.Name);
                    }
                }
                else if (rProp.Name == SRCH_PARA_FAMILY)
                {
                    if (SvxSearchItem::PutValue(rProp.Value, MID_SEARCH_STYLEFAMILY))
                        aConvertedParams.insert(rProp.Name);
                }
                else if (rProp.Name == SRCH_PARA_COMMAND)
                {
                    if (SvxSearchItem::PutValue(rProp.Value, MID_SEARCH_COMMAND))
                        aConvertedParams.insert(rProp.Name);
                }
                else if (rProp.Name == SRCH_PARA_CELLTYPE)
                {
                    if (SvxSearchItem::PutValue(rProp.Value, MID_SEARCH_CELLTYPE))
                        aConvertedParams.insert(rProp.Name);
                }
                else if (rProp.Name == SRCH_PARA_APPFLAG)
                {
                    if (ExtractNumericAny(rProp.Value, m_nAppFlag))
                        aConvertedParams.insert(rProp.Name);
                }
                else if (rProp.Name == SRCH_PARA_ROWDIR)
                {
                    if (SvxSearchItem::PutValue(rProp.Value, MID_SEARCH_ROWDIRECTION))
                        aConvertedParams.insert(rProp.Name);
                }
                else if (rProp.Name == SRCH_PARA_ALLTABLES)
                {
                    if (SvxSearchItem::PutValue(rProp.Value, MID_SEARCH_ALLTABLES))
                        aConvertedParams.insert(rProp.Name);
                }
                else if (rProp.Name == SRCH_PARA_SEARCHFILTERED)
                {
                    if (SvxSearchItem::PutValue(rProp.Value, MID_SEARCH_SEARCHFILTERED))
                        aConvertedParams.insert(rProp.Name);
                }
                else if (rProp.Name == SRCH_PARA_SEARCHFORMATTED)
                {
                    if (SvxSearchItem::PutValue(rProp.Value, MID_SEARCH_SEARCHFORMATTED))
                        aConvertedParams.insert(rProp.Name);
                }
                else if (rProp.Name == SRCH_PARA_BACKWARD)
                {
                    if (SvxSearchItem::PutValue(rProp.Value, MID_SEARCH_BACKWARD))
                        aConvertedParams.insert(rProp.Name);
                }
                else if (rProp.Name == SRCH_PARA_PATTERN)
                {
                    if (SvxSearchItem::PutValue(rProp.Value, MID_SEARCH_PATTERN))
                        aConvertedParams.insert(rProp.Name);
                }
                else if (rProp.Name == SRCH_PARA_CONTENT)
                {
                    if (SvxSearchItem::PutValue(rProp.Value, MID_SEARCH_CONTENT))
                        aConvertedParams.insert(rProp.Name);
                }
                else if (rProp.Name == SRCH_PARA_ASIANOPT)
                {
                    if (SvxSearchItem::PutValue(rProp.Value, MID_SEARCH_ASIANOPTIONS))
                        aConvertedParams.insert(rProp.Name);
                }
            }
            return aConvertedParams.size() == SRCH_PARAMS;
        }
        case MID_SEARCH_COMMAND:
            return ExtractNumericAny(rVal, m_nCommand);
        case MID_SEARCH_STYLEFAMILY:
            return ExtractNumericAny(rVal, m_eFamily);
        case MID_SEARCH_CELLTYPE:
            return ExtractNumericAny(rVal, m_nCellType);
        case MID_SEARCH_ROWDIRECTION:
            return (rVal >>= m_bRowDirection);
        case MID_SEARCH_ALLTABLES:
            return (rVal >>= m_bAllTables);
        case MID_SEARCH_SEARCHFILTERED:
            return (rVal >>= m_bSearchFiltered);
        case MID_SEARCH_SEARCHFORMATTED:
            return (rVal >>= m_bSearchFormatted);
        case MID_SEARCH_BACKWARD:
            return (rVal >>= m_bBackward);
        case MID_SEARCH_PATTERN:
            return (rVal >>= m_bPattern);
        case MID_SEARCH_CONTENT:
            return (rVal >>= m_bContent);
        case MID_SEARCH_ASIANOPTIONS:
            return (rVal >>= m_bAsianOptions);
        case MID_SEARCH_ALGORITHMTYPE:
            if (SearchAlgorithms eVal; ExtractNumericAny(rVal, eVal))
            {
                m_aSearchOpt.AlgorithmType2 = i18nutil::upgradeSearchAlgorithms(eVal);
                return true;
            }
            break;
        case MID_SEARCH_ALGORITHMTYPE2:
            return (rVal >>= m_aSearchOpt.AlgorithmType2);
        case MID_SEARCH_FLAGS:
            return (rVal >>= m_aSearchOpt.searchFlag);
        case MID_SEARCH_SEARCHSTRING:
            return (rVal >>= m_aSearchOpt.searchString);
        case MID_SEARCH_REPLACESTRING:
            return (rVal >>= m_aSearchOpt.replaceString);
        case MID_SEARCH_CHANGEDCHARS:
            return (rVal >>= m_aSearchOpt.changedChars);
        case MID_SEARCH_DELETEDCHARS:
            return (rVal >>= m_aSearchOpt.deletedChars);
        case MID_SEARCH_INSERTEDCHARS:
            return (rVal >>= m_aSearchOpt.insertedChars);
        case MID_SEARCH_TRANSLITERATEFLAGS:
            return ExtractNumericAny(rVal, m_aSearchOpt.transliterateFlags);
        case MID_SEARCH_LOCALE:
            if (LanguageType aVal; ExtractNumericAny(rVal, aVal))
            {
                m_aSearchOpt.Locale = (aVal == LANGUAGE_NONE) ? css::lang::Locale()
                                                              : LanguageTag::convertToLocale(aVal);
                return true;
            }
            break;
        case MID_SEARCH_STARTPOINTX:
            return (rVal >>= m_nStartPointX);
        case MID_SEARCH_STARTPOINTY:
            return (rVal >>= m_nStartPointY);
        default:
            OSL_FAIL( "Unknown MemberId" );
    }

    return false;
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
