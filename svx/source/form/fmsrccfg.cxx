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

#include <svx/fmsrccfg.hxx>

#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nutil/transliteration.hxx>

using namespace ::com::sun::star::uno;

namespace svxform
{
    // search parameters

    FmSearchParams::FmSearchParams()
        :nTransliterationFlags( TransliterationFlags::NONE )
        ,nSearchForType     ( 0 )
        ,nPosition          ( MATCHING_ANYWHERE )
        ,nLevOther          ( 2 )
        ,nLevShorter        ( 2 )
        ,nLevLonger         ( 2 )
        ,bLevRelaxed        ( true )
        ,bAllFields         ( false )
        ,bUseFormatter      ( true )
        ,bBackwards         ( false )
        ,bWildcard          ( false )
        ,bRegular           ( false )
        ,bApproxSearch      ( false )
        ,bSoundsLikeCJK     ( false )
    {
        nTransliterationFlags =
                TransliterationFlags::ignoreSpace_ja_JP
            |   TransliterationFlags::ignoreMiddleDot_ja_JP
            |   TransliterationFlags::ignoreProlongedSoundMark_ja_JP
            |   TransliterationFlags::ignoreSeparator_ja_JP
            |   TransliterationFlags::IGNORE_CASE;
    }

    bool FmSearchParams::isIgnoreWidthCJK( ) const
    {
        return bool(nTransliterationFlags & TransliterationFlags::IGNORE_WIDTH);
    }

    bool FmSearchParams::isCaseSensitive( ) const
    {
        return !(nTransliterationFlags & TransliterationFlags::IGNORE_CASE);
    }

    void FmSearchParams::setCaseSensitive( bool _bCase )
    {
        if ( _bCase )
            nTransliterationFlags &= ~TransliterationFlags::IGNORE_CASE;
        else
            nTransliterationFlags |= TransliterationFlags::IGNORE_CASE;
    }

    // maps from ascii values to int values

    namespace {

    struct Ascii2Int16
    {
        const char* pAscii;
        sal_Int16   nValue;
    };

    }

    static const Ascii2Int16* lcl_getSearchForTypeValueMap()
    {
        static const Ascii2Int16 s_aSearchForTypeMap[] =
        {
            { "text",       0 },
            { "null",       1 },
            { "non-null",   2 },
            { nullptr,         -1 }
        };
        return s_aSearchForTypeMap;
    }

    static const Ascii2Int16* lcl_getSearchPositionValueMap()
    {
        static const Ascii2Int16 s_aSearchPositionMap[] =
        {
            { "anywhere-in-field",      MATCHING_ANYWHERE },
            { "beginning-of-field",     MATCHING_BEGINNING },
            { "end-of-field",           MATCHING_END },
            { "complete-field",         MATCHING_WHOLETEXT },
            { nullptr,                     -1 }
        };
        return s_aSearchPositionMap;
    }

    static sal_Int16 lcl_implMapAsciiValue( const OUString& _rAsciiValue, const Ascii2Int16* _pMap )
    {
        // search the map for the given ascii value
        const Ascii2Int16* pSearch = _pMap;
        while ( pSearch && pSearch->pAscii )
        {
            if ( _rAsciiValue.equalsAscii( pSearch->pAscii ) )
                // found
                return pSearch->nValue;
            ++pSearch;
        }

        SAL_WARN(
            "svx.form", "could not convert the ascii value " << _rAsciiValue);
        return -1;
    }

    static const char* lcl_implMapIntValue( const sal_Int16 _nValue, const Ascii2Int16* _pMap )
    {
        // search the map for the given integer value
        const Ascii2Int16* pSearch = _pMap;
        while ( pSearch && pSearch->pAscii )
        {
            if ( _nValue == pSearch->nValue )
                // found
                return pSearch->pAscii;
            ++pSearch;
        }

        SAL_WARN( "svx", "lcl_implMapIntValue: could not convert the integer value "
                    << _nValue <<  " !");
        static const char* const s_pDummy = "";
            // just as a fallback...
        return s_pDummy;
    }

    // class FmSearchConfigItem - a config item that stores search parameters

#define TA( c )     &c, cppu::UnoType<decltype(c)>::get()

    FmSearchConfigItem::FmSearchConfigItem()
        :OConfigurationValueContainer( ::comphelper::getProcessComponentContext(), m_aMutex, u"/org.openoffice.Office.DataAccess/FormSearchOptions"_ustr, 2 )
    {
        // register our members so the data exchange with the node values is done automatically

        registerExchangeLocation( u"SearchHistory"_ustr,                      TA( aHistory ) );
        registerExchangeLocation( u"LevenshteinOther"_ustr,                   TA( nLevOther ) );
        registerExchangeLocation( u"LevenshteinShorter"_ustr,                 TA( nLevShorter ) );
        registerExchangeLocation( u"LevenshteinLonger"_ustr,                  TA( nLevLonger ) );
        registerExchangeLocation( u"IsLevenshteinRelaxed"_ustr,               TA( bLevRelaxed ) );
        registerExchangeLocation( u"IsSearchAllFields"_ustr,                  TA( bAllFields ) );
        registerExchangeLocation( u"IsUseFormatter"_ustr,                     TA( bUseFormatter ) );
        registerExchangeLocation( u"IsBackwards"_ustr,                        TA( bBackwards ) );
        registerExchangeLocation( u"IsWildcardSearch"_ustr,                   TA( bWildcard ) );
        registerExchangeLocation( u"IsUseRegularExpression"_ustr,             TA( bRegular ) );
        registerExchangeLocation( u"IsSimilaritySearch"_ustr,                 TA( bApproxSearch ) );
        registerExchangeLocation( u"IsUseAsianOptions"_ustr,                  TA( bSoundsLikeCJK ) );

        // the properties which need to be translated
        registerExchangeLocation( u"SearchType"_ustr,                         TA( m_sSearchForType ) );
        registerExchangeLocation( u"SearchPosition"_ustr,                     TA( m_sSearchPosition ) );

        registerExchangeLocation( u"IsMatchCase"_ustr,                        TA( m_bIsMatchCase ) );
        registerExchangeLocation( u"Japanese/IsMatchFullHalfWidthForms"_ustr, TA( m_bIsMatchFullHalfWidthForms ) );
        registerExchangeLocation( u"Japanese/IsMatchHiraganaKatakana"_ustr,   TA( m_bIsMatchHiraganaKatakana ) );
        registerExchangeLocation( u"Japanese/IsMatchContractions"_ustr,       TA( m_bIsMatchContractions ) );
        registerExchangeLocation( u"Japanese/IsMatchMinusDashCho-on"_ustr,    TA( m_bIsMatchMinusDashCho_on ) );
        registerExchangeLocation( u"Japanese/IsMatchRepeatCharMarks"_ustr,    TA( m_bIsMatchRepeatCharMarks ) );
        registerExchangeLocation( u"Japanese/IsMatchVariantFormKanji"_ustr,   TA( m_bIsMatchVariantFormKanji ) );
        registerExchangeLocation( u"Japanese/IsMatchOldKanaForms"_ustr,       TA( m_bIsMatchOldKanaForms ) );
        registerExchangeLocation( u"Japanese/IsMatch_DiZi_DuZu"_ustr,         TA( m_bIsMatch_DiZi_DuZu ) );
        registerExchangeLocation( u"Japanese/IsMatch_BaVa_HaFa"_ustr,         TA( m_bIsMatch_BaVa_HaFa ) );
        registerExchangeLocation( u"Japanese/IsMatch_TsiThiChi_DhiZi"_ustr,   TA( m_bIsMatch_TsiThiChi_DhiZi ) );
        registerExchangeLocation( u"Japanese/IsMatch_HyuIyu_ByuVyu"_ustr,     TA( m_bIsMatch_HyuIyu_ByuVyu ) );
        registerExchangeLocation( u"Japanese/IsMatch_SeShe_ZeJe"_ustr,        TA( m_bIsMatch_SeShe_ZeJe ) );
        registerExchangeLocation( u"Japanese/IsMatch_IaIya"_ustr,             TA( m_bIsMatch_IaIya ) );
        registerExchangeLocation( u"Japanese/IsMatch_KiKu"_ustr,              TA( m_bIsMatch_KiKu ) );
        registerExchangeLocation( u"Japanese/IsIgnorePunctuation"_ustr,       TA( m_bIsIgnorePunctuation ) );
        registerExchangeLocation( u"Japanese/IsIgnoreWhitespace"_ustr,        TA( m_bIsIgnoreWhitespace ) );
        registerExchangeLocation( u"Japanese/IsIgnoreProlongedSoundMark"_ustr,TA( m_bIsIgnoreProlongedSoundMark ) );
        registerExchangeLocation( u"Japanese/IsIgnoreMiddleDot"_ustr,         TA( m_bIsIgnoreMiddleDot ) );

        read( );
    }

    FmSearchConfigItem::~FmSearchConfigItem()
    {
        commit( );
    }

    void FmSearchConfigItem::implTranslateFromConfig( )
    {
        // the search-for string
        nSearchForType = lcl_implMapAsciiValue( m_sSearchForType, lcl_getSearchForTypeValueMap() );

        // the search position
        nPosition = lcl_implMapAsciiValue( m_sSearchPosition, lcl_getSearchPositionValueMap() );

        // the transliteration flags
        nTransliterationFlags = TransliterationFlags::NONE;

        if ( !m_bIsMatchCase                )   nTransliterationFlags |= TransliterationFlags::IGNORE_CASE;
        if ( m_bIsMatchFullHalfWidthForms   )   nTransliterationFlags |= TransliterationFlags::IGNORE_WIDTH;
        if ( m_bIsMatchHiraganaKatakana     )   nTransliterationFlags |= TransliterationFlags::IGNORE_KANA;
        if ( m_bIsMatchContractions         )   nTransliterationFlags |= TransliterationFlags::ignoreSize_ja_JP;
        if ( m_bIsMatchMinusDashCho_on      )   nTransliterationFlags |= TransliterationFlags::ignoreMinusSign_ja_JP;
        if ( m_bIsMatchRepeatCharMarks      )   nTransliterationFlags |= TransliterationFlags::ignoreIterationMark_ja_JP;
        if ( m_bIsMatchVariantFormKanji     )   nTransliterationFlags |= TransliterationFlags::ignoreTraditionalKanji_ja_JP;
        if ( m_bIsMatchOldKanaForms         )   nTransliterationFlags |= TransliterationFlags::ignoreTraditionalKana_ja_JP;
        if ( m_bIsMatch_DiZi_DuZu           )   nTransliterationFlags |= TransliterationFlags::ignoreZiZu_ja_JP;
        if ( m_bIsMatch_BaVa_HaFa           )   nTransliterationFlags |= TransliterationFlags::ignoreBaFa_ja_JP;
        if ( m_bIsMatch_TsiThiChi_DhiZi     )   nTransliterationFlags |= TransliterationFlags::ignoreTiJi_ja_JP;
        if ( m_bIsMatch_HyuIyu_ByuVyu       )   nTransliterationFlags |= TransliterationFlags::ignoreHyuByu_ja_JP;
        if ( m_bIsMatch_SeShe_ZeJe          )   nTransliterationFlags |= TransliterationFlags::ignoreSeZe_ja_JP;
        if ( m_bIsMatch_IaIya               )   nTransliterationFlags |= TransliterationFlags::ignoreIandEfollowedByYa_ja_JP;
        if ( m_bIsMatch_KiKu                )   nTransliterationFlags |= TransliterationFlags::ignoreKiKuFollowedBySa_ja_JP;

        if ( m_bIsIgnorePunctuation         )   nTransliterationFlags |= TransliterationFlags::ignoreSeparator_ja_JP;
        if ( m_bIsIgnoreWhitespace          )   nTransliterationFlags |= TransliterationFlags::ignoreSpace_ja_JP;
        if ( m_bIsIgnoreProlongedSoundMark  )   nTransliterationFlags |= TransliterationFlags::ignoreProlongedSoundMark_ja_JP;
        if ( m_bIsIgnoreMiddleDot           )   nTransliterationFlags |= TransliterationFlags::ignoreMiddleDot_ja_JP;
    }

    void FmSearchConfigItem::implTranslateToConfig( )
    {
        // the search-for string
        m_sSearchForType = OUString::createFromAscii( lcl_implMapIntValue( nSearchForType, lcl_getSearchForTypeValueMap() ) );

        // the search position
        m_sSearchPosition = OUString::createFromAscii( lcl_implMapIntValue( nPosition, lcl_getSearchPositionValueMap() ) );

        // the transliteration flags

        m_bIsMatchCase                  = !( nTransliterationFlags & TransliterationFlags::IGNORE_CASE );
        m_bIsMatchFullHalfWidthForms    = bool( nTransliterationFlags & TransliterationFlags::IGNORE_WIDTH );
        m_bIsMatchHiraganaKatakana      = bool( nTransliterationFlags & TransliterationFlags::IGNORE_KANA );
        m_bIsMatchContractions          = bool( nTransliterationFlags & TransliterationFlags::ignoreSize_ja_JP );
        m_bIsMatchMinusDashCho_on       = bool( nTransliterationFlags & TransliterationFlags::ignoreMinusSign_ja_JP );
        m_bIsMatchRepeatCharMarks       = bool( nTransliterationFlags & TransliterationFlags::ignoreIterationMark_ja_JP );
        m_bIsMatchVariantFormKanji      = bool( nTransliterationFlags & TransliterationFlags::ignoreTraditionalKanji_ja_JP );
        m_bIsMatchOldKanaForms          = bool( nTransliterationFlags & TransliterationFlags::ignoreTraditionalKana_ja_JP );
        m_bIsMatch_DiZi_DuZu            = bool( nTransliterationFlags & TransliterationFlags::ignoreZiZu_ja_JP );
        m_bIsMatch_BaVa_HaFa            = bool( nTransliterationFlags & TransliterationFlags::ignoreBaFa_ja_JP );
        m_bIsMatch_TsiThiChi_DhiZi      = bool( nTransliterationFlags & TransliterationFlags::ignoreTiJi_ja_JP );
        m_bIsMatch_HyuIyu_ByuVyu        = bool( nTransliterationFlags & TransliterationFlags::ignoreHyuByu_ja_JP );
        m_bIsMatch_SeShe_ZeJe           = bool( nTransliterationFlags & TransliterationFlags::ignoreSeZe_ja_JP );
        m_bIsMatch_IaIya                = bool( nTransliterationFlags & TransliterationFlags::ignoreIandEfollowedByYa_ja_JP );
        m_bIsMatch_KiKu                 = bool( nTransliterationFlags & TransliterationFlags::ignoreKiKuFollowedBySa_ja_JP );

        m_bIsIgnorePunctuation          = bool( nTransliterationFlags & TransliterationFlags::ignoreSeparator_ja_JP );
        m_bIsIgnoreWhitespace           = bool( nTransliterationFlags & TransliterationFlags::ignoreSpace_ja_JP );
        m_bIsIgnoreProlongedSoundMark   = bool( nTransliterationFlags & TransliterationFlags::ignoreProlongedSoundMark_ja_JP );
        m_bIsIgnoreMiddleDot            = bool( nTransliterationFlags & TransliterationFlags::ignoreMiddleDot_ja_JP );
    }

    const FmSearchParams& FmSearchConfigItem::getParams() const
    {
        // ensure that the properties which are not stored directly are up-to-date
        const_cast< FmSearchConfigItem* >( this )->implTranslateFromConfig( );

        // and return our FmSearchParams part
        return *this;
    }

    void FmSearchConfigItem::setParams( const FmSearchParams& _rParams )
    {
        // copy the FmSearchParams part
        *static_cast< FmSearchParams* >( this ) = _rParams;

        // translate the settings not represented by a direct config value
        implTranslateToConfig();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
