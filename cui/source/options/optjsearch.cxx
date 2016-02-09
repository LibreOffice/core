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

#include <unotools/searchopt.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <cuires.hrc>
#include <dialmgr.hxx>
#include <optjsearch.hxx>

using namespace com::sun::star::i18n;


SvxJSearchOptionsPage::SvxJSearchOptionsPage( vcl::Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage( pParent, "OptJSearchPage", "cui/ui/optjsearchpage.ui", &rSet )
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

    bSaveOptions = true;
    nTransliterationFlags = 0x00000000;
}

SvxJSearchOptionsPage::~SvxJSearchOptionsPage()
{
    disposeOnce();
}

void SvxJSearchOptionsPage::dispose()
{
    m_pMatchCase.clear();
    m_pMatchFullHalfWidth.clear();
    m_pMatchHiraganaKatakana.clear();
    m_pMatchContractions.clear();
    m_pMatchMinusDashChoon.clear();
    m_pMatchRepeatCharMarks.clear();
    m_pMatchVariantFormKanji.clear();
    m_pMatchOldKanaForms.clear();
    m_pMatchDiziDuzu.clear();
    m_pMatchBavaHafa.clear();
    m_pMatchTsithichiDhizi.clear();
    m_pMatchHyuiyuByuvyu.clear();
    m_pMatchSesheZeje.clear();
    m_pMatchIaiya.clear();
    m_pMatchKiku.clear();
    m_pMatchProlongedSoundMark.clear();
    m_pIgnorePunctuation.clear();
    m_pIgnoreWhitespace.clear();
    m_pIgnoreMiddleDot.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxJSearchOptionsPage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxJSearchOptionsPage>::Create( pParent, *rSet );
}


void SvxJSearchOptionsPage::SetTransliterationFlags( sal_Int32 nSettings )
{
    bool  bVal = 0 != (nSettings & TransliterationModules_IGNORE_CASE);
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


void SvxJSearchOptionsPage::Reset( const SfxItemSet* )
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


bool SvxJSearchOptionsPage::FillItemSet( SfxItemSet* )
{
    sal_Int32 nOldVal = nTransliterationFlags;
    nTransliterationFlags = GetTransliterationFlags_Impl();
    bool bModified = nOldVal != nTransliterationFlags;

    if (!IsSaveOptions())
        return bModified;

    bModified = false;
    SvtSearchOptions aOpt;
    bool bNewVal, bChanged;

    bNewVal  = m_pMatchCase->IsChecked();  //! treat as equal uppercase/lowercase
    bChanged = m_pMatchCase->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchCase(!bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchFullHalfWidth->IsChecked();
    bChanged = m_pMatchFullHalfWidth->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchFullHalfWidthForms( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchHiraganaKatakana->IsChecked();
    bChanged = m_pMatchHiraganaKatakana->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchHiraganaKatakana( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchContractions->IsChecked();
    bChanged = m_pMatchContractions->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchContractions( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchMinusDashChoon->IsChecked();
    bChanged = m_pMatchMinusDashChoon->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchMinusDashChoon( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchRepeatCharMarks->IsChecked();
    bChanged = m_pMatchRepeatCharMarks->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchRepeatCharMarks( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchVariantFormKanji->IsChecked();
    bChanged = m_pMatchVariantFormKanji->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchVariantFormKanji( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchOldKanaForms->IsChecked();
    bChanged = m_pMatchOldKanaForms->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchOldKanaForms( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchDiziDuzu->IsChecked();
    bChanged = m_pMatchDiziDuzu->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchDiziDuzu( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchBavaHafa->IsChecked();
    bChanged = m_pMatchBavaHafa->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchBavaHafa( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchTsithichiDhizi->IsChecked();
    bChanged = m_pMatchTsithichiDhizi->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchTsithichiDhizi( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchHyuiyuByuvyu->IsChecked();
    bChanged = m_pMatchHyuiyuByuvyu->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchHyuiyuByuvyu( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchSesheZeje->IsChecked();
    bChanged = m_pMatchSesheZeje->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchSesheZeje( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchIaiya->IsChecked();
    bChanged = m_pMatchIaiya->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchIaiya( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchKiku->IsChecked();
    bChanged = m_pMatchKiku->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetMatchKiku( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pIgnorePunctuation->IsChecked();
    bChanged = m_pIgnorePunctuation->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetIgnorePunctuation( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pIgnoreWhitespace->IsChecked();
    bChanged = m_pIgnoreWhitespace->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetIgnoreWhitespace( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pMatchProlongedSoundMark->IsChecked();
    bChanged = m_pMatchProlongedSoundMark->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetIgnoreProlongedSoundMark( bNewVal );
        bModified = true;
    }
    bNewVal  = m_pIgnoreMiddleDot->IsChecked();
    bChanged = m_pIgnoreMiddleDot->IsValueChangedFromSaved();
    if (bChanged)
    {
        aOpt.SetIgnoreMiddleDot( bNewVal );
        bModified = true;
    }

    if (bModified)
        aOpt.Commit();

    return bModified;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
