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

#include <tools/shl.hxx>
#include <unotools/searchopt.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <cuires.hrc>
#include <dialmgr.hxx>
#include <optjsearch.hxx>
#include "optjsearch.hrc"

using namespace com::sun::star::i18n;

//////////////////////////////////////////////////////////////////////

SvxJSearchOptionsPage::SvxJSearchOptionsPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_JSEARCH_OPTIONS ), rSet ),

    aTreatAsEqual               ( this, CUI_RES( FL_TREAT_AS_EQUAL ) ),
    aMatchCase                  ( this, CUI_RES( CB_MATCH_CASE ) ),
    aMatchFullHalfWidth         ( this, CUI_RES( CB_MATCH_FULL_HALF_WIDTH ) ),
    aMatchHiraganaKatakana      ( this, CUI_RES( CB_MATCH_HIRAGANA_KATAKANA ) ),
    aMatchContractions          ( this, CUI_RES( CB_MATCH_CONTRACTIONS ) ),
    aMatchMinusDashChoon        ( this, CUI_RES( CB_MATCH_MINUS_DASH_CHOON ) ),
    aMatchRepeatCharMarks       ( this, CUI_RES( CB_MATCH_REPEAT_CHAR_MARKS ) ),
    aMatchVariantFormKanji      ( this, CUI_RES( CB_MATCH_VARIANT_FORM_KANJI ) ),
    aMatchOldKanaForms          ( this, CUI_RES( CB_MATCH_OLD_KANA_FORMS ) ),
    aMatchDiziDuzu              ( this, CUI_RES( CB_MATCH_DIZI_DUZU ) ),
    aMatchBavaHafa              ( this, CUI_RES( CB_MATCH_BAVA_HAFA ) ),
    aMatchTsithichiDhizi        ( this, CUI_RES( CB_MATCH_TSITHICHI_DHIZI ) ),
    aMatchHyuiyuByuvyu          ( this, CUI_RES( CB_MATCH_HYUFYU_BYUVYU ) ),
    aMatchSesheZeje             ( this, CUI_RES( CB_MATCH_SESHE_ZEJE ) ),
    aMatchIaiya                 ( this, CUI_RES( CB_MATCH_IAIYA ) ),
    aMatchKiku                  ( this, CUI_RES( CB_MATCH_KIKU ) ),
    aMatchProlongedSoundMark    ( this, CUI_RES( CB_MATCH_PROLONGED_SOUNDMARK ) ),
    aIgnore                     ( this, CUI_RES( FL_IGNORE ) ),
    aIgnorePunctuation          ( this, CUI_RES( CB_IGNORE_PUNCTUATION ) ),
    aIgnoreWhitespace           ( this, CUI_RES( CB_IGNORE_WHITESPACES ) ),
    aIgnoreMiddleDot            ( this, CUI_RES( CB_IGNORE_MIDDLE_DOT ) )
{
    FreeResource();
    bSaveOptions = TRUE;
    nTransliterationFlags = 0x00000000;
}


SvxJSearchOptionsPage::~SvxJSearchOptionsPage()
{
}


SfxTabPage*  SvxJSearchOptionsPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxJSearchOptionsPage( pParent, rSet );
}


void SvxJSearchOptionsPage::SetTransliterationFlags( INT32 nSettings )
{
    BOOL  bVal = 0 != (nSettings & TransliterationModules_IGNORE_CASE);
    aMatchCase               .Check( bVal );    //! treat as equal uppercase/lowercase
    bVal = 0 != (nSettings & TransliterationModules_IGNORE_WIDTH);
    aMatchFullHalfWidth      .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_IGNORE_KANA);
    aMatchHiraganaKatakana   .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreSize_ja_JP);
    aMatchContractions       .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreMinusSign_ja_JP);
    aMatchMinusDashChoon     .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreIterationMark_ja_JP);
    aMatchRepeatCharMarks    .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreTraditionalKanji_ja_JP);
    aMatchVariantFormKanji   .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreTraditionalKana_ja_JP);
    aMatchOldKanaForms       .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreZiZu_ja_JP);
    aMatchDiziDuzu           .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreBaFa_ja_JP);
    aMatchBavaHafa           .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreTiJi_ja_JP);
    aMatchTsithichiDhizi     .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreHyuByu_ja_JP);
    aMatchHyuiyuByuvyu       .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreSeZe_ja_JP);
    aMatchSesheZeje          .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreIandEfollowedByYa_ja_JP);
    aMatchIaiya              .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreKiKuFollowedBySa_ja_JP);
    aMatchKiku               .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreSeparator_ja_JP);
    aIgnorePunctuation       .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreSpace_ja_JP);
    aIgnoreWhitespace        .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreProlongedSoundMark_ja_JP);
    aMatchProlongedSoundMark.Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_ignoreMiddleDot_ja_JP);
    aIgnoreMiddleDot         .Check( bVal );

    nTransliterationFlags = nSettings;
}


INT32 SvxJSearchOptionsPage::GetTransliterationFlags_Impl()
{
    INT32 nTmp = 0;
    if (aMatchCase.IsChecked()) //! treat as equal uppercase/lowercase
        nTmp |= TransliterationModules_IGNORE_CASE;
    if (aMatchFullHalfWidth.IsChecked())
        nTmp |= TransliterationModules_IGNORE_WIDTH;
    if (aMatchHiraganaKatakana.IsChecked())
        nTmp |= TransliterationModules_IGNORE_KANA;
    if (aMatchContractions.IsChecked())
        nTmp |= TransliterationModules_ignoreSize_ja_JP;
    if (aMatchMinusDashChoon.IsChecked())
        nTmp |= TransliterationModules_ignoreMinusSign_ja_JP;
    if (aMatchRepeatCharMarks.IsChecked())
        nTmp |= TransliterationModules_ignoreIterationMark_ja_JP;
    if (aMatchVariantFormKanji.IsChecked())
        nTmp |= TransliterationModules_ignoreTraditionalKanji_ja_JP;
    if (aMatchOldKanaForms.IsChecked())
        nTmp |= TransliterationModules_ignoreTraditionalKana_ja_JP;
    if (aMatchDiziDuzu.IsChecked())
        nTmp |= TransliterationModules_ignoreZiZu_ja_JP;
    if (aMatchBavaHafa.IsChecked())
        nTmp |= TransliterationModules_ignoreBaFa_ja_JP;
    if (aMatchTsithichiDhizi.IsChecked())
        nTmp |= TransliterationModules_ignoreTiJi_ja_JP;
    if (aMatchHyuiyuByuvyu.IsChecked())
        nTmp |= TransliterationModules_ignoreHyuByu_ja_JP;
    if (aMatchSesheZeje.IsChecked())
        nTmp |= TransliterationModules_ignoreSeZe_ja_JP;
    if (aMatchIaiya.IsChecked())
        nTmp |= TransliterationModules_ignoreIandEfollowedByYa_ja_JP;
    if (aMatchKiku.IsChecked())
        nTmp |= TransliterationModules_ignoreKiKuFollowedBySa_ja_JP;
    if (aIgnorePunctuation.IsChecked())
        nTmp |= TransliterationModules_ignoreSeparator_ja_JP;
    if (aIgnoreWhitespace.IsChecked())
        nTmp |= TransliterationModules_ignoreSpace_ja_JP;
    if (aMatchProlongedSoundMark.IsChecked())
        nTmp |= TransliterationModules_ignoreProlongedSoundMark_ja_JP;
    if (aIgnoreMiddleDot.IsChecked())
        nTmp |= TransliterationModules_ignoreMiddleDot_ja_JP;

    nTransliterationFlags = nTmp;
    return nTransliterationFlags;
}


void SvxJSearchOptionsPage::Reset( const SfxItemSet& )
{
    SvtSearchOptions aOpt;

    // read settings from configuration
    aMatchCase               .Check(!aOpt.IsMatchCase() );  //! treat as equal uppercase/lowercase
    aMatchFullHalfWidth      .Check( aOpt.IsMatchFullHalfWidthForms() );
    aMatchHiraganaKatakana   .Check( aOpt.IsMatchHiraganaKatakana() );
    aMatchContractions       .Check( aOpt.IsMatchContractions() );
    aMatchMinusDashChoon     .Check( aOpt.IsMatchMinusDashChoon() );
    aMatchRepeatCharMarks    .Check( aOpt.IsMatchRepeatCharMarks() );
    aMatchVariantFormKanji   .Check( aOpt.IsMatchVariantFormKanji() );
    aMatchOldKanaForms       .Check( aOpt.IsMatchOldKanaForms() );
    aMatchDiziDuzu           .Check( aOpt.IsMatchDiziDuzu() );
    aMatchBavaHafa           .Check( aOpt.IsMatchBavaHafa() );
    aMatchTsithichiDhizi     .Check( aOpt.IsMatchTsithichiDhizi() );
    aMatchHyuiyuByuvyu       .Check( aOpt.IsMatchHyuiyuByuvyu() );
    aMatchSesheZeje          .Check( aOpt.IsMatchSesheZeje() );
    aMatchIaiya              .Check( aOpt.IsMatchIaiya() );
    aMatchKiku               .Check( aOpt.IsMatchKiku() );
    aIgnorePunctuation       .Check( aOpt.IsIgnorePunctuation() );
    aIgnoreWhitespace        .Check( aOpt.IsIgnoreWhitespace() );
    aMatchProlongedSoundMark .Check( aOpt.IsIgnoreProlongedSoundMark() );
    aIgnoreMiddleDot         .Check( aOpt.IsIgnoreMiddleDot() );

    nTransliterationFlags = GetTransliterationFlags_Impl();
    DBG_ASSERT( nTransliterationFlags == aOpt.GetTransliterationFlags(),
            "Transliteration settings different" );

    aMatchCase               .SaveValue();
    aMatchFullHalfWidth      .SaveValue();
    aMatchHiraganaKatakana   .SaveValue();
    aMatchContractions       .SaveValue();
    aMatchMinusDashChoon     .SaveValue();
    aMatchRepeatCharMarks    .SaveValue();
    aMatchVariantFormKanji   .SaveValue();
    aMatchOldKanaForms       .SaveValue();
    aMatchDiziDuzu           .SaveValue();
    aMatchBavaHafa           .SaveValue();
    aMatchTsithichiDhizi     .SaveValue();
    aMatchHyuiyuByuvyu       .SaveValue();
    aMatchSesheZeje          .SaveValue();
    aMatchIaiya              .SaveValue();
    aMatchKiku               .SaveValue();
    aIgnorePunctuation       .SaveValue();
    aIgnoreWhitespace        .SaveValue();
    aMatchProlongedSoundMark .SaveValue();
    aIgnoreMiddleDot         .SaveValue();
}


BOOL SvxJSearchOptionsPage::FillItemSet( SfxItemSet& )
{
    BOOL bModified = FALSE;

    INT32 nOldVal = nTransliterationFlags;
    nTransliterationFlags = GetTransliterationFlags_Impl();
    bModified = nOldVal != nTransliterationFlags;

    if (!IsSaveOptions())
        return bModified;

    bModified = FALSE;
    SvtSearchOptions aOpt;
    BOOL bNewVal, bChanged;

    bNewVal  = aMatchCase.IsChecked();  //! treat as equal uppercase/lowercase
    bChanged = bNewVal != aMatchCase.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchCase(!bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchFullHalfWidth.IsChecked();
    bChanged = bNewVal != aMatchFullHalfWidth.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchFullHalfWidthForms( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchHiraganaKatakana.IsChecked();
    bChanged = bNewVal != aMatchHiraganaKatakana.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchHiraganaKatakana( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchContractions.IsChecked();
    bChanged = bNewVal != aMatchContractions.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchContractions( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchMinusDashChoon.IsChecked();
    bChanged = bNewVal != aMatchMinusDashChoon.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchMinusDashChoon( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchRepeatCharMarks.IsChecked();
    bChanged = bNewVal != aMatchRepeatCharMarks.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchRepeatCharMarks( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchVariantFormKanji.IsChecked();
    bChanged = bNewVal != aMatchVariantFormKanji.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchVariantFormKanji( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchOldKanaForms.IsChecked();
    bChanged = bNewVal != aMatchOldKanaForms.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchOldKanaForms( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchDiziDuzu.IsChecked();
    bChanged = bNewVal != aMatchDiziDuzu.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchDiziDuzu( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchBavaHafa.IsChecked();
    bChanged = bNewVal != aMatchBavaHafa.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchBavaHafa( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchTsithichiDhizi.IsChecked();
    bChanged = bNewVal != aMatchTsithichiDhizi.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchTsithichiDhizi( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchHyuiyuByuvyu.IsChecked();
    bChanged = bNewVal != aMatchHyuiyuByuvyu.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchHyuiyuByuvyu( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchSesheZeje.IsChecked();
    bChanged = bNewVal != aMatchSesheZeje.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchSesheZeje( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchIaiya.IsChecked();
    bChanged = bNewVal != aMatchIaiya.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchIaiya( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchKiku.IsChecked();
    bChanged = bNewVal != aMatchKiku.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetMatchKiku( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aIgnorePunctuation.IsChecked();
    bChanged = bNewVal != aIgnorePunctuation.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetIgnorePunctuation( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aIgnoreWhitespace.IsChecked();
    bChanged = bNewVal != aIgnoreWhitespace.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetIgnoreWhitespace( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aMatchProlongedSoundMark.IsChecked();
    bChanged = bNewVal != aMatchProlongedSoundMark.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetIgnoreProlongedSoundMark( bNewVal );
        bModified = TRUE;
    }
    bNewVal  = aIgnoreMiddleDot.IsChecked();
    bChanged = bNewVal != aIgnoreMiddleDot.GetSavedValue();
    if (bChanged)
    {
        aOpt.SetIgnoreMiddleDot( bNewVal );
        bModified = TRUE;
    }

    return bModified;
}


//////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
