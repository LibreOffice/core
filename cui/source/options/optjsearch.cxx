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

#include <tools/debug.hxx>
#include <unotools/searchopt.hxx>
#include <i18nutil/transliteration.hxx>
#include "optjsearch.hxx"

using namespace com::sun::star::i18n;


SvxJSearchOptionsPage::SvxJSearchOptionsPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/optjsearchpage.ui", "OptJSearchPage", &rSet)
    , m_xMatchCase(m_xBuilder->weld_check_button("matchcase"))
    , m_xMatchFullHalfWidth(m_xBuilder->weld_check_button("matchfullhalfwidth"))
    , m_xMatchHiraganaKatakana(m_xBuilder->weld_check_button("matchhiraganakatakana"))
    , m_xMatchContractions(m_xBuilder->weld_check_button("matchcontractions"))
    , m_xMatchMinusDashChoon(m_xBuilder->weld_check_button("matchminusdashchoon"))
    , m_xMatchRepeatCharMarks(m_xBuilder->weld_check_button("matchrepeatcharmarks"))
    , m_xMatchVariantFormKanji(m_xBuilder->weld_check_button("matchvariantformkanji"))
    , m_xMatchOldKanaForms(m_xBuilder->weld_check_button("matcholdkanaforms"))
    , m_xMatchDiziDuzu(m_xBuilder->weld_check_button("matchdiziduzu"))
    , m_xMatchBavaHafa(m_xBuilder->weld_check_button("matchbavahafa"))
    , m_xMatchTsithichiDhizi(m_xBuilder->weld_check_button("matchtsithichidhizi"))
    , m_xMatchHyuiyuByuvyu(m_xBuilder->weld_check_button("matchhyuiyubyuvyu"))
    , m_xMatchSesheZeje(m_xBuilder->weld_check_button("matchseshezeje"))
    , m_xMatchIaiya(m_xBuilder->weld_check_button("matchiaiya"))
    , m_xMatchKiku(m_xBuilder->weld_check_button("matchkiku"))
    , m_xMatchProlongedSoundMark(m_xBuilder->weld_check_button("matchprolongedsoundmark"))
    , m_xIgnorePunctuation(m_xBuilder->weld_check_button("ignorepunctuation"))
    , m_xIgnoreWhitespace(m_xBuilder->weld_check_button("ignorewhitespace"))
    , m_xIgnoreMiddleDot(m_xBuilder->weld_check_button("ignoremiddledot"))
{
    bSaveOptions = true;
    nTransliterationFlags = TransliterationFlags::NONE;
}

SvxJSearchOptionsPage::~SvxJSearchOptionsPage()
{
    disposeOnce();
}

VclPtr<SfxTabPage> SvxJSearchOptionsPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxJSearchOptionsPage>::Create(pParent, *rSet);
}

void SvxJSearchOptionsPage::SetTransliterationFlags( TransliterationFlags nSettings )
{
    bool  bVal(nSettings & TransliterationFlags::IGNORE_CASE);
    m_xMatchCase              ->set_active( bVal );    //! treat as equal uppercase/lowercase
    bVal = bool(nSettings & TransliterationFlags::IGNORE_WIDTH);
    m_xMatchFullHalfWidth     ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::IGNORE_KANA);
    m_xMatchHiraganaKatakana  ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreSize_ja_JP);
    m_xMatchContractions      ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreMinusSign_ja_JP);
    m_xMatchMinusDashChoon    ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreIterationMark_ja_JP);
    m_xMatchRepeatCharMarks   ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreTraditionalKanji_ja_JP);
    m_xMatchVariantFormKanji  ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreTraditionalKana_ja_JP);
    m_xMatchOldKanaForms      ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreZiZu_ja_JP);
    m_xMatchDiziDuzu          ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreBaFa_ja_JP);
    m_xMatchBavaHafa          ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreTiJi_ja_JP);
    m_xMatchTsithichiDhizi    ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreHyuByu_ja_JP);
    m_xMatchHyuiyuByuvyu      ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreSeZe_ja_JP);
    m_xMatchSesheZeje         ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreIandEfollowedByYa_ja_JP);
    m_xMatchIaiya             ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreKiKuFollowedBySa_ja_JP);
    m_xMatchKiku              ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreSeparator_ja_JP);
    m_xIgnorePunctuation      ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreSpace_ja_JP);
    m_xIgnoreWhitespace       ->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreProlongedSoundMark_ja_JP);
    m_xMatchProlongedSoundMark->set_active( bVal );
    bVal = bool(nSettings & TransliterationFlags::ignoreMiddleDot_ja_JP);
    m_xIgnoreMiddleDot        ->set_active( bVal );

    nTransliterationFlags = nSettings;
}

TransliterationFlags SvxJSearchOptionsPage::GetTransliterationFlags_Impl()
{
    TransliterationFlags nTmp = TransliterationFlags::NONE;
    if (m_xMatchCase->get_active()) //! treat as equal uppercase/lowercase
        nTmp |= TransliterationFlags::IGNORE_CASE;
    if (m_xMatchFullHalfWidth->get_active())
        nTmp |= TransliterationFlags::IGNORE_WIDTH;
    if (m_xMatchHiraganaKatakana->get_active())
        nTmp |= TransliterationFlags::IGNORE_KANA;
    if (m_xMatchContractions->get_active())
        nTmp |= TransliterationFlags::ignoreSize_ja_JP;
    if (m_xMatchMinusDashChoon->get_active())
        nTmp |= TransliterationFlags::ignoreMinusSign_ja_JP;
    if (m_xMatchRepeatCharMarks->get_active())
        nTmp |= TransliterationFlags::ignoreIterationMark_ja_JP;
    if (m_xMatchVariantFormKanji->get_active())
        nTmp |= TransliterationFlags::ignoreTraditionalKanji_ja_JP;
    if (m_xMatchOldKanaForms->get_active())
        nTmp |= TransliterationFlags::ignoreTraditionalKana_ja_JP;
    if (m_xMatchDiziDuzu->get_active())
        nTmp |= TransliterationFlags::ignoreZiZu_ja_JP;
    if (m_xMatchBavaHafa->get_active())
        nTmp |= TransliterationFlags::ignoreBaFa_ja_JP;
    if (m_xMatchTsithichiDhizi->get_active())
        nTmp |= TransliterationFlags::ignoreTiJi_ja_JP;
    if (m_xMatchHyuiyuByuvyu->get_active())
        nTmp |= TransliterationFlags::ignoreHyuByu_ja_JP;
    if (m_xMatchSesheZeje->get_active())
        nTmp |= TransliterationFlags::ignoreSeZe_ja_JP;
    if (m_xMatchIaiya->get_active())
        nTmp |= TransliterationFlags::ignoreIandEfollowedByYa_ja_JP;
    if (m_xMatchKiku->get_active())
        nTmp |= TransliterationFlags::ignoreKiKuFollowedBySa_ja_JP;
    if (m_xIgnorePunctuation->get_active())
        nTmp |= TransliterationFlags::ignoreSeparator_ja_JP;
    if (m_xIgnoreWhitespace->get_active())
        nTmp |= TransliterationFlags::ignoreSpace_ja_JP;
    if (m_xMatchProlongedSoundMark->get_active())
        nTmp |= TransliterationFlags::ignoreProlongedSoundMark_ja_JP;
    if (m_xIgnoreMiddleDot->get_active())
        nTmp |= TransliterationFlags::ignoreMiddleDot_ja_JP;

    nTransliterationFlags = nTmp;
    return nTransliterationFlags;
}


void SvxJSearchOptionsPage::Reset( const SfxItemSet* )
{
    SvtSearchOptions aOpt;

    // read settings from configuration
    m_xMatchCase               ->set_active(!aOpt.IsMatchCase() );  //! treat as equal uppercase/lowercase
    m_xMatchFullHalfWidth      ->set_active( aOpt.IsMatchFullHalfWidthForms() );
    m_xMatchHiraganaKatakana   ->set_active( aOpt.IsMatchHiraganaKatakana() );
    m_xMatchContractions       ->set_active( aOpt.IsMatchContractions() );
    m_xMatchMinusDashChoon     ->set_active( aOpt.IsMatchMinusDashChoon() );
    m_xMatchRepeatCharMarks    ->set_active( aOpt.IsMatchRepeatCharMarks() );
    m_xMatchVariantFormKanji   ->set_active( aOpt.IsMatchVariantFormKanji() );
    m_xMatchOldKanaForms       ->set_active( aOpt.IsMatchOldKanaForms() );
    m_xMatchDiziDuzu           ->set_active( aOpt.IsMatchDiziDuzu() );
    m_xMatchBavaHafa           ->set_active( aOpt.IsMatchBavaHafa() );
    m_xMatchTsithichiDhizi     ->set_active( aOpt.IsMatchTsithichiDhizi() );
    m_xMatchHyuiyuByuvyu       ->set_active( aOpt.IsMatchHyuiyuByuvyu() );
    m_xMatchSesheZeje          ->set_active( aOpt.IsMatchSesheZeje() );
    m_xMatchIaiya              ->set_active( aOpt.IsMatchIaiya() );
    m_xMatchKiku               ->set_active( aOpt.IsMatchKiku() );
    m_xIgnorePunctuation       ->set_active( aOpt.IsIgnorePunctuation() );
    m_xIgnoreWhitespace        ->set_active( aOpt.IsIgnoreWhitespace() );
    m_xMatchProlongedSoundMark ->set_active( aOpt.IsIgnoreProlongedSoundMark() );
    m_xIgnoreMiddleDot         ->set_active( aOpt.IsIgnoreMiddleDot() );

    nTransliterationFlags = GetTransliterationFlags_Impl();
    DBG_ASSERT( nTransliterationFlags == aOpt.GetTransliterationFlags(),
            "Transliteration settings different" );

    m_xMatchCase               ->save_state();
    m_xMatchFullHalfWidth      ->save_state();
    m_xMatchHiraganaKatakana   ->save_state();
    m_xMatchContractions       ->save_state();
    m_xMatchMinusDashChoon     ->save_state();
    m_xMatchRepeatCharMarks    ->save_state();
    m_xMatchVariantFormKanji   ->save_state();
    m_xMatchOldKanaForms       ->save_state();
    m_xMatchDiziDuzu           ->save_state();
    m_xMatchBavaHafa           ->save_state();
    m_xMatchTsithichiDhizi     ->save_state();
    m_xMatchHyuiyuByuvyu       ->save_state();
    m_xMatchSesheZeje          ->save_state();
    m_xMatchIaiya              ->save_state();
    m_xMatchKiku               ->save_state();
    m_xIgnorePunctuation       ->save_state();
    m_xIgnoreWhitespace        ->save_state();
    m_xMatchProlongedSoundMark ->save_state();
    m_xIgnoreMiddleDot         ->save_state();
}


bool SvxJSearchOptionsPage::FillItemSet( SfxItemSet* )
{
    TransliterationFlags nOldVal = nTransliterationFlags;
    nTransliterationFlags = GetTransliterationFlags_Impl();
    bool bModified = nOldVal != nTransliterationFlags;

    if (!bSaveOptions)
        return bModified;

    bModified = false;
    SvtSearchOptions aOpt;
    bool bNewVal, bChanged;

    bNewVal  = m_xMatchCase->get_active();  //! treat as equal uppercase/lowercase
    bChanged = m_xMatchCase->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchCase(!bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchFullHalfWidth->get_active();
    bChanged = m_xMatchFullHalfWidth->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchFullHalfWidthForms( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchHiraganaKatakana->get_active();
    bChanged = m_xMatchHiraganaKatakana->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchHiraganaKatakana( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchContractions->get_active();
    bChanged = m_xMatchContractions->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchContractions( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchMinusDashChoon->get_active();
    bChanged = m_xMatchMinusDashChoon->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchMinusDashChoon( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchRepeatCharMarks->get_active();
    bChanged = m_xMatchRepeatCharMarks->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchRepeatCharMarks( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchVariantFormKanji->get_active();
    bChanged = m_xMatchVariantFormKanji->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchVariantFormKanji( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchOldKanaForms->get_active();
    bChanged = m_xMatchOldKanaForms->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchOldKanaForms( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchDiziDuzu->get_active();
    bChanged = m_xMatchDiziDuzu->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchDiziDuzu( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchBavaHafa->get_active();
    bChanged = m_xMatchBavaHafa->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchBavaHafa( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchTsithichiDhizi->get_active();
    bChanged = m_xMatchTsithichiDhizi->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchTsithichiDhizi( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchHyuiyuByuvyu->get_active();
    bChanged = m_xMatchHyuiyuByuvyu->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchHyuiyuByuvyu( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchSesheZeje->get_active();
    bChanged = m_xMatchSesheZeje->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchSesheZeje( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchIaiya->get_active();
    bChanged = m_xMatchIaiya->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchIaiya( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchKiku->get_active();
    bChanged = m_xMatchKiku->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetMatchKiku( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xIgnorePunctuation->get_active();
    bChanged = m_xIgnorePunctuation->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetIgnorePunctuation( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xIgnoreWhitespace->get_active();
    bChanged = m_xIgnoreWhitespace->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetIgnoreWhitespace( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xMatchProlongedSoundMark->get_active();
    bChanged = m_xMatchProlongedSoundMark->get_state_changed_from_saved();
    if (bChanged)
    {
        aOpt.SetIgnoreProlongedSoundMark( bNewVal );
        bModified = true;
    }
    bNewVal  = m_xIgnoreMiddleDot->get_active();
    bChanged = m_xIgnoreMiddleDot->get_state_changed_from_saved();
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
