/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "svx/fmsrccfg.hxx"
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;

namespace svxform
{
    

    FmSearchParams::FmSearchParams()
        :nTransliterationFlags( 0 )
        ,nSearchForType     ( 0 )
        ,nPosition          ( MATCHING_ANYWHERE )
        ,nLevOther          ( 2 )
        ,nLevShorter        ( 2 )
        ,nLevLonger         ( 2 )
        ,bLevRelaxed        ( sal_True )
        ,bAllFields         ( sal_False )
        ,bUseFormatter      ( sal_True )
        ,bBackwards         ( sal_False )
        ,bWildcard          ( sal_False )
        ,bRegular           ( sal_False )
        ,bApproxSearch      ( sal_False )
        ,bSoundsLikeCJK     ( sal_False )
    {
        nTransliterationFlags =
                TransliterationModules_ignoreSpace_ja_JP
            |   TransliterationModules_ignoreMiddleDot_ja_JP
            |   TransliterationModules_ignoreProlongedSoundMark_ja_JP
            |   TransliterationModules_ignoreSeparator_ja_JP
            |   TransliterationModules_IGNORE_CASE;
    }

    sal_Bool FmSearchParams::isIgnoreWidthCJK( ) const
    {
        return 0 != (nTransliterationFlags & TransliterationModules_IGNORE_WIDTH);
    }

    sal_Bool FmSearchParams::isCaseSensitive( ) const
    {
        return 0 == (nTransliterationFlags & TransliterationModules_IGNORE_CASE);
    }

    void FmSearchParams::setCaseSensitive( sal_Bool _bCase )
    {
        if ( _bCase )
            nTransliterationFlags &= ~TransliterationModules_IGNORE_CASE;
        else
            nTransliterationFlags |= TransliterationModules_IGNORE_CASE;
    }

    

    struct Ascii2Int16
    {
        const sal_Char* pAscii;
        sal_Int16       nValue;
    };

    static const Ascii2Int16* lcl_getSearchForTypeValueMap()
    {
        static const Ascii2Int16 s_aSearchForTypeMap[] =
        {
            { "text",       0 },
            { "null",       1 },
            { "non-null",   2 },
            { NULL,         -1 }
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
            { NULL,                     -1 }
        };
        return s_aSearchPositionMap;
    }

    static sal_Int16 lcl_implMapAsciiValue( const OUString& _rAsciiValue, const Ascii2Int16* _pMap )
    {
        
        const Ascii2Int16* pSearch = _pMap;
        while ( pSearch && pSearch->pAscii )
        {
            if ( _rAsciiValue.equalsAscii( pSearch->pAscii ) )
                
                return pSearch->nValue;
            ++pSearch;
        }

        OSL_FAIL("lcl_implMapAsciiValue: could not convert the ascii value " + _rAsciiValue + " !");
        return -1;
    }

    static const sal_Char* lcl_implMapIntValue( const sal_Int16 _nValue, const Ascii2Int16* _pMap )
    {
        
        const Ascii2Int16* pSearch = _pMap;
        while ( pSearch && pSearch->pAscii )
        {
            if ( _nValue == pSearch->nValue )
                
                return pSearch->pAscii;
            ++pSearch;
        }

        OSL_FAIL(
            (   OString( "lcl_implMapIntValue: could not convert the integer value " )
            +=  OString::number( _nValue )
            +=  OString( " !" )
            ).getStr()
        );
        static const sal_Char* s_pDummy = "";
            
        return s_pDummy;
    }

    

#define TA( c )     &c, getCppuType( &c )

    FmSearchConfigItem::FmSearchConfigItem()
        :OConfigurationValueContainer( ::comphelper::getProcessComponentContext(), m_aMutex, "/org.openoffice.Office.DataAccess/FormSearchOptions", CVC_UPDATE_ACCESS | CVC_LAZY_UPDATE, 2 )
    {
        

        registerExchangeLocation( "SearchHistory",                      TA( aHistory ) );
        registerExchangeLocation( "LevenshteinOther",                   TA( nLevOther ) );
        registerExchangeLocation( "LevenshteinShorter",                 TA( nLevShorter ) );
        registerExchangeLocation( "LevenshteinLonger",                  TA( nLevLonger ) );
        registerExchangeLocation( "IsLevenshteinRelaxed",               TA( bLevRelaxed ) );
        registerExchangeLocation( "IsSearchAllFields",                  TA( bAllFields ) );
        registerExchangeLocation( "IsUseFormatter",                     TA( bUseFormatter ) );
        registerExchangeLocation( "IsBackwards",                        TA( bBackwards ) );
        registerExchangeLocation( "IsWildcardSearch",                   TA( bWildcard ) );
        registerExchangeLocation( "IsUseRegularExpression",             TA( bRegular ) );
        registerExchangeLocation( "IsSimilaritySearch",                 TA( bApproxSearch ) );
        registerExchangeLocation( "IsUseAsianOptions",                  TA( bSoundsLikeCJK ) );

        
        registerExchangeLocation( "SearchType",                         TA( m_sSearchForType ) );
        registerExchangeLocation( "SearchPosition",                     TA( m_sSearchPosition ) );

        registerExchangeLocation( "IsMatchCase",                        TA( m_bIsMatchCase ) );
        registerExchangeLocation( "Japanese/IsMatchFullHalfWidthForms", TA( m_bIsMatchFullHalfWidthForms ) );
        registerExchangeLocation( "Japanese/IsMatchHiraganaKatakana",   TA( m_bIsMatchHiraganaKatakana ) );
        registerExchangeLocation( "Japanese/IsMatchContractions",       TA( m_bIsMatchContractions ) );
        registerExchangeLocation( "Japanese/IsMatchMinusDashCho-on",    TA( m_bIsMatchMinusDashCho_on ) );
        registerExchangeLocation( "Japanese/IsMatchRepeatCharMarks",    TA( m_bIsMatchRepeatCharMarks ) );
        registerExchangeLocation( "Japanese/IsMatchVariantFormKanji",   TA( m_bIsMatchVariantFormKanji ) );
        registerExchangeLocation( "Japanese/IsMatchOldKanaForms",       TA( m_bIsMatchOldKanaForms ) );
        registerExchangeLocation( "Japanese/IsMatch_DiZi_DuZu",         TA( m_bIsMatch_DiZi_DuZu ) );
        registerExchangeLocation( "Japanese/IsMatch_BaVa_HaFa",         TA( m_bIsMatch_BaVa_HaFa ) );
        registerExchangeLocation( "Japanese/IsMatch_TsiThiChi_DhiZi",   TA( m_bIsMatch_TsiThiChi_DhiZi ) );
        registerExchangeLocation( "Japanese/IsMatch_HyuIyu_ByuVyu",     TA( m_bIsMatch_HyuIyu_ByuVyu ) );
        registerExchangeLocation( "Japanese/IsMatch_SeShe_ZeJe",        TA( m_bIsMatch_SeShe_ZeJe ) );
        registerExchangeLocation( "Japanese/IsMatch_IaIya",             TA( m_bIsMatch_IaIya ) );
        registerExchangeLocation( "Japanese/IsMatch_KiKu",              TA( m_bIsMatch_KiKu ) );
        registerExchangeLocation( "Japanese/IsIgnorePunctuation",       TA( m_bIsIgnorePunctuation ) );
        registerExchangeLocation( "Japanese/IsIgnoreWhitespace",        TA( m_bIsIgnoreWhitespace ) );
        registerExchangeLocation( "Japanese/IsIgnoreProlongedSoundMark",TA( m_bIsIgnoreProlongedSoundMark ) );
        registerExchangeLocation( "Japanese/IsIgnoreMiddleDot",         TA( m_bIsIgnoreMiddleDot ) );

        read( );
    }

    FmSearchConfigItem::~FmSearchConfigItem()
    {
        commit( );
    }

    void FmSearchConfigItem::implTranslateFromConfig( )
    {
        
        nSearchForType = lcl_implMapAsciiValue( m_sSearchForType, lcl_getSearchForTypeValueMap() );

        
        nPosition = lcl_implMapAsciiValue( m_sSearchPosition, lcl_getSearchPositionValueMap() );

        
        nTransliterationFlags = 0;

        if ( !m_bIsMatchCase                )   nTransliterationFlags |= TransliterationModules_IGNORE_CASE;
        if ( m_bIsMatchFullHalfWidthForms   )   nTransliterationFlags |= TransliterationModules_IGNORE_WIDTH;
        if ( m_bIsMatchHiraganaKatakana     )   nTransliterationFlags |= TransliterationModules_IGNORE_KANA;
        if ( m_bIsMatchContractions         )   nTransliterationFlags |= TransliterationModules_ignoreSize_ja_JP;
        if ( m_bIsMatchMinusDashCho_on      )   nTransliterationFlags |= TransliterationModules_ignoreMinusSign_ja_JP;
        if ( m_bIsMatchRepeatCharMarks      )   nTransliterationFlags |= TransliterationModules_ignoreIterationMark_ja_JP;
        if ( m_bIsMatchVariantFormKanji     )   nTransliterationFlags |= TransliterationModules_ignoreTraditionalKanji_ja_JP;
        if ( m_bIsMatchOldKanaForms         )   nTransliterationFlags |= TransliterationModules_ignoreTraditionalKana_ja_JP;
        if ( m_bIsMatch_DiZi_DuZu           )   nTransliterationFlags |= TransliterationModules_ignoreZiZu_ja_JP;
        if ( m_bIsMatch_BaVa_HaFa           )   nTransliterationFlags |= TransliterationModules_ignoreBaFa_ja_JP;
        if ( m_bIsMatch_TsiThiChi_DhiZi     )   nTransliterationFlags |= TransliterationModules_ignoreTiJi_ja_JP;
        if ( m_bIsMatch_HyuIyu_ByuVyu       )   nTransliterationFlags |= TransliterationModules_ignoreHyuByu_ja_JP;
        if ( m_bIsMatch_SeShe_ZeJe          )   nTransliterationFlags |= TransliterationModules_ignoreSeZe_ja_JP;
        if ( m_bIsMatch_IaIya               )   nTransliterationFlags |= TransliterationModules_ignoreIandEfollowedByYa_ja_JP;
        if ( m_bIsMatch_KiKu                )   nTransliterationFlags |= TransliterationModules_ignoreKiKuFollowedBySa_ja_JP;

        if ( m_bIsIgnorePunctuation         )   nTransliterationFlags |= TransliterationModules_ignoreSeparator_ja_JP;
        if ( m_bIsIgnoreWhitespace          )   nTransliterationFlags |= TransliterationModules_ignoreSpace_ja_JP;
        if ( m_bIsIgnoreProlongedSoundMark  )   nTransliterationFlags |= TransliterationModules_ignoreProlongedSoundMark_ja_JP;
        if ( m_bIsIgnoreMiddleDot           )   nTransliterationFlags |= TransliterationModules_ignoreMiddleDot_ja_JP;
    }

    void FmSearchConfigItem::implTranslateToConfig( )
    {
        
        m_sSearchForType = OUString::createFromAscii( lcl_implMapIntValue( nSearchForType, lcl_getSearchForTypeValueMap() ) );

        
        m_sSearchPosition = OUString::createFromAscii( lcl_implMapIntValue( nPosition, lcl_getSearchPositionValueMap() ) );

        

        m_bIsMatchCase                  = ( 0 == ( nTransliterationFlags & TransliterationModules_IGNORE_CASE ) );
        m_bIsMatchFullHalfWidthForms    = ( 0 != ( nTransliterationFlags & TransliterationModules_IGNORE_WIDTH ) );
        m_bIsMatchHiraganaKatakana      = ( 0 != ( nTransliterationFlags & TransliterationModules_IGNORE_KANA ) );
        m_bIsMatchContractions          = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreSize_ja_JP ) );
        m_bIsMatchMinusDashCho_on       = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreMinusSign_ja_JP ) );
        m_bIsMatchRepeatCharMarks       = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreIterationMark_ja_JP ) );
        m_bIsMatchVariantFormKanji      = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreTraditionalKanji_ja_JP ) );
        m_bIsMatchOldKanaForms          = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreTraditionalKana_ja_JP ) );
        m_bIsMatch_DiZi_DuZu            = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreZiZu_ja_JP ) );
        m_bIsMatch_BaVa_HaFa            = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreBaFa_ja_JP ) );
        m_bIsMatch_TsiThiChi_DhiZi      = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreTiJi_ja_JP ) );
        m_bIsMatch_HyuIyu_ByuVyu        = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreHyuByu_ja_JP ) );
        m_bIsMatch_SeShe_ZeJe           = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreSeZe_ja_JP ) );
        m_bIsMatch_IaIya                = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreIandEfollowedByYa_ja_JP ) );
        m_bIsMatch_KiKu                 = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreKiKuFollowedBySa_ja_JP ) );

        m_bIsIgnorePunctuation          = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreSeparator_ja_JP ) );
        m_bIsIgnoreWhitespace           = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreSpace_ja_JP ) );
        m_bIsIgnoreProlongedSoundMark   = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreProlongedSoundMark_ja_JP ) );
        m_bIsIgnoreMiddleDot            = ( 0 != ( nTransliterationFlags & TransliterationModules_ignoreMiddleDot_ja_JP ) );
    }

    const FmSearchParams& FmSearchConfigItem::getParams() const
    {
        
        const_cast< FmSearchConfigItem* >( this )->implTranslateFromConfig( );

        
        return *this;
    }

    void FmSearchConfigItem::setParams( const FmSearchParams& _rParams )
    {
        
        *static_cast< FmSearchParams* >( this ) = _rParams;

        
        implTranslateToConfig();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
