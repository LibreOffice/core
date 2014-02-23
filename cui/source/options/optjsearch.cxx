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

#include <tools/shl.hxx>
#include <unotools/searchopt.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <cuires.hrc>
#include <dialmgr.hxx>
#include <optjsearch.hxx>

using namespace com::sun::star::i18n;



SvxJSearchOptionsPage::SvxJSearchOptionsPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage( pParent, "OptJSearchPage", "cui/ui/optjsearchpage.ui", rSet )
{
    get( m_pMatchCase, "matchcase");
    get( m_pMatchFullHalfWidth, "matchfullhalfwidth");
    get( m_pMatchHiraganaKatakana, "matchhiraganakatakana");
    get( m_pMatchContractions, "matchcontractions");
    get( m_pMatchMinusDashChoon, "matchminusdashchoon");
    get( m_pMatchRepeatCharMarks, "matchrepeatcharmarks");
    get( m_pMatchVariantFormKanji, "matchvariantformkanji");
    get( m_pMatchOldKanaForms, "matcholdkanaforms");
    get( m_pMatchDiziDuzu, "matchdiziduzu");
    get( m_pMatchBavaHafa, "matchbavahafa");
    get( m_pMatchTsithichiDhizi, "matchtsithichidhizi");
    get( m_pMatchHyuiyuByuvyu, "matchhyuiyubyuvyu");
    get( m_pMatchSesheZeje, "matchseshezeje");
    get( m_pMatchIaiya, "matchiaiya");
    get( m_pMatchKiku, "matchkiku");
    get( m_pMatchProlongedSoundMark, "matchprolongedsoundmark");

    get( m_pIgnorePunctuation, "ignorepunctuation");
    get( m_pIgnoreWhitespace, "ignorewhitespace");
    get( m_pIgnoreMiddleDot, "ignoremiddledot");

    bSaveOptions = sal_True;
    nTransliterationFlags = 0x00000000;
}


SvxJSearchOptionsPage::~SvxJSearchOptionsPage()
{
}


SfxTabPage*  SvxJSearchOptionsPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxJSearchOptionsPage( pParent, rSet );
}


void SvxJSearchOptionsPage::SetTransliterationFlags( sal_Int32 nSettings )
{
    sal_Bool  bVal = 0 != (nSettings & TransliterationModules_IGNORE_CASE);
    m_pMatchCase              ->Check( bVal );    //! treat as equal uppercase/lowercase
    bVal = 0 != (nSettings & TransliterationModules_IGNORE_WIDTH);
    m_pMatchFullHalfWidth     ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_IGNORE_KANA);
    m_pMatchHiraganaKatakana  ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreSize_ja_JP);
    m_pMatchContractions      ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreMinusSign_ja_JP);
    m_pMatchMinusDashChoon    ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreIterationMark_ja_JP);
    m_pMatchRepeatCharMarks   ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreTraditionalKanji_ja_JP);
    m_pMatchVariantFormKanji  ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreTraditionalKana_ja_JP);
    m_pMatchOldKanaForms      ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreZiZu_ja_JP);
    m_pMatchDiziDuzu          ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreBaFa_ja_JP);
    m_pMatchBavaHafa          ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreTiJi_ja_JP);
    m_pMatchTsithichiDhizi    ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreHyuByu_ja_JP);
    m_pMatchHyuiyuByuvyu      ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreSeZe_ja_JP);
    m_pMatchSesheZeje         ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreIandEfollowedByYa_ja_JP);
    m_pMatchIaiya             ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreKiKuFollowedBySa_ja_JP);
    m_pMatchKiku              ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreSeparator_ja_JP);
    m_pIgnorePunctuation      ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreSpace_ja_JP);
    m_pIgnoreWhitespace       ->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreProlongedSoundMark_ja_JP);
    m_pMatchProlongedSoundMark->Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreMiddleDot_ja_JP);
    m_pIgnoreMiddleDot        ->Check( bVal );

    nTransliterationFlags = nSettings;
}


sal_Int32 SvxJSearchOptionsPage::GetTransliterationFlags_Impl()
{
    sal_Int32 nTmp = 0;
    if (m_pMatchCase->IsChecked()) //! treat as equal uppercase/lowercase
        nTmp |= TransliterationModules_IGNORE_CASE;
    if (m_pMatchFullHalfWidth->IsChecked())
        nTmp |= TransliterationModules_IGNORE_WIDTH;
    if (m_pMatchHiraganaKatakana->IsChecked())
        nTmp |= TransliterationModules_IGNORE_KANA;
    if (m_pMatchContractions->IsChecked())
        nTmp |= TransliterationModules_ignoreSize_ja_JP;
    if (m_pMatchMinusDashChoon->IsChecked())
        nTmp |= TransliterationModules_ignoreMinusSign_ja_JP;
    if (m_pMatchRepeatCharMarks->IsChecked())
        nTmp |= TransliterationModules_ignoreIterationMark_ja_JP;
    if (m_pMatchVariantFormKanji->IsChecked())
        nTmp |= TransliterationModules_ignoreTraditionalKanji_ja_JP;
    if (m_pMatchOldKanaForms->IsChecked())
        nTmp |= TransliterationModules_ignoreTraditionalKana_ja_JP;
    if (m_pMatchDiziDuzu->IsChecked())
        nTmp |= TransliterationModules_ignoreZiZu_ja_JP;
    if (m_pMatchBavaHafa->IsChecked())
        nTmp |= TransliterationModules_ignoreBaFa_ja_JP;
    if (m_pMatchTsithichiDhizi->IsChecked())
        nTmp |= TransliterationModules_ignoreTiJi_ja_JP;
    if (m_pMatchHyuiyuByuvyu->IsChecked())
        nTmp |= TransliterationModules_ignoreHyuByu_ja_JP;
    if (m_pMatchSesheZeje->IsChecked())
        nTmp |= TransliterationModules_ignoreSeZe_ja_JP;
    if (m_pMatchIaiya->IsChecked())
        nTmp |= TransliterationModules_ignoreIandEfollowedByYa_ja_JP;
    if (m_pMatchKiku->IsChecked())
        nTmp |= TransliterationModules_ignoreKiKuFollowedBySa_ja_JP;
    if (m_pIgnorePunctuation->IsChecked())
        nTmp |= TransliterationModules_ignoreSeparator_ja_JP;
    if (m_pIgnoreWhitespace->IsChecked())
        nTmp |= TransliterationModules_ignoreSpace_ja_JP;
    if (m_pMatchProlongedSoundMark->IsChecked())
        nTmp |= TransliterationModules_ignoreProlongedSoundMark_ja_JP;
    if (m_pIgnoreMiddleDot->IsChecked())
        nTmp |= TransliterationModules_ignoreMiddleDot_ja_JP;

    nTransliterationFlags = nTmp;
    return nTransliterationFlags;
}


void SvxJSearchOptionsPage::Reset( const SfxItemSet& )
{
    SvtSearchOptions aOpt;

    // read settings from configuration
    m_pMatchCase               ->Check(!aOpt.IsMatchCase() );  //! treat as equal uppercase/lowercase
    m_pMatchFullHalfWidth      ->Check( aOpt.IsMatchFullHalfWidthForms() );
    m_pMatchHiraganaKatakana   ->Check( aOpt.IsMatchHiraganaKatakana() );
    m_pMatchContractions       ->Check( aOpt.IsMatchContractions() );
    m_pMatchMinusDashChoon     ->Check( aOpt.IsMatchMinusDashChoon() );
    m_pMatchRepeatCharMarks    ->Check( aOpt.IsMatchRepeatCharMarks() );
    m_pMatchVariantFormKanji   ->Check( aOpt.IsMatchVariantFormKanji() );
    m_pMatchOldKanaForms       ->Check( aOpt.IsMatchOldKanaForms() );
    m_pMatchDiziDuzu           ->Check( aOpt.IsMatchDiziDuzu() );
    m_pMatchBavaHafa           ->Check( aOpt.IsMatchBavaHafa() );
    m_pMatchTsithichiDhizi     ->Check( aOpt.IsMatchTsithichiDhizi() );
    m_pMatchHyuiyuByuvyu       ->Check( aOpt.IsMatchHyuiyuByuvyu() );
    m_pMatchSesheZeje          ->Check( aOpt.IsMatchSesheZeje() );
    m_pMatchIaiya              ->Check( aOpt.IsMatchIaiya() );
    m_pMatchKiku               ->Check( aOpt.IsMatchKiku() );
    m_pIgnorePunctuation       ->Check( aOpt.IsIgnorePunctuation() );
    m_pIgnoreWhitespace        ->Check( aOpt.IsIgnoreWhitespace() );
    m_pMatchProlongedSoundMark ->Check( aOpt.IsIgnoreProlongedSoundMark() );
    m_pIgnoreMiddleDot         ->Check( aOpt.IsIgnoreMiddleDot() );

    nTransliterationFlags = GetTransliterationFlags_Impl();
    DBG_ASSERT( nTransliterationFlags == aOpt.GetTransliterationFlags(),
            "Transliteration settings different" );

    m_pMatchCase               ->SaveValue();
    m_pMatchFullHalfWidth      ->SaveValue();
    m_pMatchHiraganaKatakana   ->SaveValue();
    m_pMatchContractions       ->SaveValue();
    m_pMatchMinusDashChoon     ->SaveValue();
    m_pMatchRepeatCharMarks    ->SaveValue();
    m_pMatchVariantFormKanji   ->SaveValue();
    m_pMatchOldKanaForms       ->SaveValue();
    m_pMatchDiziDuzu           ->SaveValue();
    m_pMatchBavaHafa           ->SaveValue();
    m_pMatchTsithichiDhizi     ->SaveValue();
    m_pMatchHyuiyuByuvyu       ->SaveValue();
    m_pMatchSesheZeje          ->SaveValue();
    m_pMatchIaiya              ->SaveValue();
    m_pMatchKiku               ->SaveValue();
    m_pIgnorePunctuation       ->SaveValue();
    m_pIgnoreWhitespace        ->SaveValue();
    m_pMatchProlongedSoundMark ->SaveValue();
    m_pIgnoreMiddleDot         ->SaveValue();
}


sal_Bool SvxJSearchOptionsPage::FillItemSet( SfxItemSet& )
{
    sal_Int32 nOldVal = nTransliterationFlags;
    nTransliterationFlags = GetTransliterationFlags_Impl();
    sal_Bool bModified = nOldVal != nTransliterationFlags;

    if (!IsSaveOptions())
        return bModified;

    bModified = sal_False;
    SvtSearchOptions aOpt;
    sal_Bool bNewVal, bChanged;

    bNewVal  = m_pMatchCase->IsChecked();  //! treat as equal uppercase/lowercase
    bChanged = bNewVal != m_pMatchCase->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchCase(!bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchFullHalfWidth->IsChecked();
    bChanged = bNewVal != m_pMatchFullHalfWidth->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchFullHalfWidthForms( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchHiraganaKatakana->IsChecked();
    bChanged = bNewVal != m_pMatchHiraganaKatakana->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchHiraganaKatakana( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchContractions->IsChecked();
    bChanged = bNewVal != m_pMatchContractions->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchContractions( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchMinusDashChoon->IsChecked();
    bChanged = bNewVal != m_pMatchMinusDashChoon->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchMinusDashChoon( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchRepeatCharMarks->IsChecked();
    bChanged = bNewVal != m_pMatchRepeatCharMarks->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchRepeatCharMarks( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchVariantFormKanji->IsChecked();
    bChanged = bNewVal != m_pMatchVariantFormKanji->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchVariantFormKanji( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchOldKanaForms->IsChecked();
    bChanged = bNewVal != m_pMatchOldKanaForms->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchOldKanaForms( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchDiziDuzu->IsChecked();
    bChanged = bNewVal != m_pMatchDiziDuzu->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchDiziDuzu( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchBavaHafa->IsChecked();
    bChanged = bNewVal != m_pMatchBavaHafa->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchBavaHafa( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchTsithichiDhizi->IsChecked();
    bChanged = bNewVal != m_pMatchTsithichiDhizi->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchTsithichiDhizi( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchHyuiyuByuvyu->IsChecked();
    bChanged = bNewVal != m_pMatchHyuiyuByuvyu->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchHyuiyuByuvyu( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchSesheZeje->IsChecked();
    bChanged = bNewVal != m_pMatchSesheZeje->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchSesheZeje( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchIaiya->IsChecked();
    bChanged = bNewVal != m_pMatchIaiya->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchIaiya( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchKiku->IsChecked();
    bChanged = bNewVal != m_pMatchKiku->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchKiku( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pIgnorePunctuation->IsChecked();
    bChanged = bNewVal != m_pIgnorePunctuation->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetIgnorePunctuation( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pIgnoreWhitespace->IsChecked();
    bChanged = bNewVal != m_pIgnoreWhitespace->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetIgnoreWhitespace( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pMatchProlongedSoundMark->IsChecked();
    bChanged = bNewVal != m_pMatchProlongedSoundMark->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetIgnoreProlongedSoundMark( bNewVal );
        bModified = sal_True;
    }
    bNewVal  = m_pIgnoreMiddleDot->IsChecked();
    bChanged = bNewVal != m_pIgnoreMiddleDot->GetSavedValue();
    if (bChanged)
    {
        aOpt.SetIgnoreMiddleDot( bNewVal );
        bModified = sal_True;
    }

    return bModified;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
