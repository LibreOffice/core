/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optjsearch.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:31:53 $
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
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif


#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SVT_SEARCHOPT_HXX_
#include <svtools/searchopt.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HPP_
#include <com/sun/star/i18n/TransliterationModules.hpp>
#endif

#ifndef _SVX_DIALOGS_HRC
#include <dialogs.hrc>
#endif
#ifndef _SVX_DIALMGR_HXX
#include <dialmgr.hxx>
#endif

#ifndef _SVX_OPTJSEARCH_HXX_
#include <optjsearch.hxx>
#endif
#include "optjsearch.hrc"

using namespace com::sun::star::i18n;

//////////////////////////////////////////////////////////////////////

SvxJSearchOptionsPage::SvxJSearchOptionsPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_JSEARCH_OPTIONS ), rSet ),

    aTreatAsEqual               ( this, ResId( FL_TREAT_AS_EQUAL ) ),
    aMatchCase                  ( this, ResId( CB_MATCH_CASE ) ),
    aMatchFullHalfWidth         ( this, ResId( CB_MATCH_FULL_HALF_WIDTH ) ),
    aMatchHiraganaKatakana      ( this, ResId( CB_MATCH_HIRAGANA_KATAKANA ) ),
    aMatchContractions          ( this, ResId( CB_MATCH_CONTRACTIONS ) ),
    aMatchMinusDashChoon        ( this, ResId( CB_MATCH_MINUS_DASH_CHOON ) ),
    aMatchRepeatCharMarks       ( this, ResId( CB_MATCH_REPEAT_CHAR_MARKS ) ),
    aMatchVariantFormKanji      ( this, ResId( CB_MATCH_VARIANT_FORM_KANJI ) ),
    aMatchOldKanaForms          ( this, ResId( CB_MATCH_OLD_KANA_FORMS ) ),
    aMatchDiziDuzu              ( this, ResId( CB_MATCH_DIZI_DUZU ) ),
    aMatchBavaHafa              ( this, ResId( CB_MATCH_BAVA_HAFA ) ),
    aMatchTsithichiDhizi        ( this, ResId( CB_MATCH_TSITHICHI_DHIZI ) ),
    aMatchHyuiyuByuvyu          ( this, ResId( CB_MATCH_HYUFYU_BYUVYU ) ),
    aMatchSesheZeje             ( this, ResId( CB_MATCH_SESHE_ZEJE ) ),
    aMatchIaiya                 ( this, ResId( CB_MATCH_IAIYA ) ),
    aMatchKiku                  ( this, ResId( CB_MATCH_KIKU ) ),
    aMatchProlongedSoundMark    ( this, ResId( CB_MATCH_PROLONGED_SOUNDMARK ) ),
    aIgnore                     ( this, ResId( FL_IGNORE ) ),
    aIgnorePunctuation          ( this, ResId( CB_IGNORE_PUNCTUATION ) ),
    aIgnoreWhitespace           ( this, ResId( CB_IGNORE_WHITESPACES ) ),
    aIgnoreMiddleDot            ( this, ResId( CB_IGNORE_MIDDLE_DOT ) )
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

