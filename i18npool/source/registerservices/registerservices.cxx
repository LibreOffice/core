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

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <comphelper/componentcontext.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <localedata.hxx>
#include <numberformatcode.hxx>
#include <nativenumbersupplier.hxx>
#include <defaultnumberingprovider.hxx>
#include <servicename.hxx>

#define TRANSLITERATION_ALL
#include <transliterationImpl.hxx>
#include <transliteration_body.hxx>
#include <transliteration_caseignore.hxx>
#include <transliteration_Ignore.hxx>
#include <transliteration_OneToOne.hxx>
#include <textToPronounce_zh.hxx>
#include <numtotext_cjk.hxx>
#include <numtochar.hxx>
#include <texttonum.hxx>
#include <chartonum.hxx>

#include <calendarImpl.hxx>
#include <calendar_gregorian.hxx>
#include <calendar_hijri.hxx>
#include <calendar_jewish.hxx>

#define BREAKITERATOR_ALL
#include <breakiteratorImpl.hxx>
#include <breakiterator_cjk.hxx>
#include <breakiterator_th.hxx>
#include <breakiterator_unicode.hxx>

#include <indexentrysupplier.hxx>
#include <indexentrysupplier_asian.hxx>
#include <indexentrysupplier_ja_phonetic.hxx>
#include <indexentrysupplier_common.hxx>
#include <indexentrysupplier_default.hxx>

#define CCLASS_ALL
#include <characterclassificationImpl.hxx>
#include <unoscripttypedetector.hxx>
#include <cclass_unicode.hxx>

#define COLLATOR_ALL
#include <collatorImpl.hxx>
#include <chaptercollator.hxx>
#include <collator_unicode.hxx>

#include <inputsequencechecker.hxx>
#include <inputsequencechecker_th.hxx>
#include <inputsequencechecker_hi.hxx>

#include <textconversionImpl.hxx>
#include <textconversion.hxx>

#include <ordinalsuffix.hxx>

#define IMPL_CREATEINSTANCE( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   \
        SAL_CALL ImplName##_CreateInstance(                                 \
            SAL_UNUSED_PARAMETER const ::com::sun::star::uno::Reference<    \
                    ::com::sun::star::lang::XMultiServiceFactory >& )       \
{                                                                           \
    return ::com::sun::star::uno::Reference <                               \
            ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* )   \
                                    new ImplName );                         \
}

#define IMPL_CREATEINSTANCE_MSF( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   \
        SAL_CALL ImplName##_CreateInstance(                                 \
            const ::com::sun::star::uno::Reference<                         \
                    ::com::sun::star::lang::XMultiServiceFactory >& rxMSF ) \
{                                                                           \
    return ::com::sun::star::uno::Reference <                               \
            ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* )   \
                                    new ImplName( rxMSF ) );                \
}

#define IMPL_CREATEINSTANCE_CTX( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   \
        SAL_CALL ImplName##_CreateInstance(                                 \
            const ::com::sun::star::uno::Reference<                         \
                    ::com::sun::star::lang::XMultiServiceFactory >& rxMSF ) \
{                                                                           \
    return ::com::sun::star::uno::Reference <                               \
            ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* )   \
                                    new ImplName( comphelper::ComponentContext(rxMSF).getUNOContext() ) );                \
}

typedef ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > (SAL_CALL *FN_CreateInstance)(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& );

#define IMPL_TRANSLITERATION_ITEM( implName ) \
    {       TRLT_SERVICELNAME_L10N, \
        TRLT_IMPLNAME_PREFIX  #implName, \
        &implName##_CreateInstance }

// -------------------------------------------------------------------------------------

using namespace ::com::sun::star::i18n;

IMPL_CREATEINSTANCE_CTX( NumberFormatCodeMapper )
IMPL_CREATEINSTANCE( NativeNumberSupplier )
IMPL_CREATEINSTANCE( LocaleData )
IMPL_CREATEINSTANCE_MSF( DefaultNumberingProvider )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_asian )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_syllable )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_consonant )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_syllable )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_consonant )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_Unicode )
IMPL_CREATEINSTANCE_MSF( CalendarImpl )
IMPL_CREATEINSTANCE( Calendar_gregorian )
IMPL_CREATEINSTANCE( Calendar_hanja )
IMPL_CREATEINSTANCE( Calendar_gengou )
IMPL_CREATEINSTANCE( Calendar_ROC )
IMPL_CREATEINSTANCE( Calendar_hijri )
IMPL_CREATEINSTANCE( Calendar_jewish )
IMPL_CREATEINSTANCE( Calendar_buddhist )
IMPL_CREATEINSTANCE_MSF( BreakIteratorImpl )
IMPL_CREATEINSTANCE( BreakIterator_Unicode )
IMPL_CREATEINSTANCE( BreakIterator_ja )
IMPL_CREATEINSTANCE( BreakIterator_zh )
IMPL_CREATEINSTANCE( BreakIterator_zh_TW )
IMPL_CREATEINSTANCE( BreakIterator_ko )
IMPL_CREATEINSTANCE( BreakIterator_th )
IMPL_CREATEINSTANCE_MSF( ChapterCollator )
IMPL_CREATEINSTANCE_MSF( CollatorImpl )
IMPL_CREATEINSTANCE( Collator_Unicode )

IMPL_CREATEINSTANCE_MSF( CharacterClassificationImpl )
IMPL_CREATEINSTANCE_MSF( cclass_Unicode )
IMPL_CREATEINSTANCE_CTX( TransliterationImpl )
IMPL_CREATEINSTANCE( UnoScriptTypeDetector )

IMPL_CREATEINSTANCE_MSF( InputSequenceCheckerImpl )
IMPL_CREATEINSTANCE( InputSequenceChecker_th )
IMPL_CREATEINSTANCE( InputSequenceChecker_hi )

IMPL_CREATEINSTANCE_MSF( TextConversionImpl )
IMPL_CREATEINSTANCE_MSF( TextConversion_ko )
IMPL_CREATEINSTANCE_MSF( TextConversion_zh )

IMPL_CREATEINSTANCE( Transliteration_u2l )
IMPL_CREATEINSTANCE( Transliteration_l2u )
IMPL_CREATEINSTANCE( Transliteration_sentencecase )
IMPL_CREATEINSTANCE( Transliteration_titlecase )
IMPL_CREATEINSTANCE( Transliteration_togglecase )
IMPL_CREATEINSTANCE( Transliteration_caseignore )
IMPL_CREATEINSTANCE( hiraganaToKatakana )
IMPL_CREATEINSTANCE( katakanaToHiragana )
IMPL_CREATEINSTANCE( ignoreKana )
IMPL_CREATEINSTANCE( fullwidthToHalfwidth )
IMPL_CREATEINSTANCE( halfwidthToFullwidth )
IMPL_CREATEINSTANCE( ignoreWidth )

IMPL_CREATEINSTANCE( smallToLarge_ja_JP)
IMPL_CREATEINSTANCE( largeToSmall_ja_JP)
IMPL_CREATEINSTANCE( ignoreTraditionalKanji_ja_JP)
IMPL_CREATEINSTANCE( ignoreTraditionalKana_ja_JP)
IMPL_CREATEINSTANCE( ignoreMinusSign_ja_JP)
IMPL_CREATEINSTANCE( ignoreIterationMark_ja_JP)
IMPL_CREATEINSTANCE( ignoreSeparator_ja_JP)
IMPL_CREATEINSTANCE( ignoreZiZu_ja_JP)
IMPL_CREATEINSTANCE( ignoreBaFa_ja_JP)
IMPL_CREATEINSTANCE( ignoreTiJi_ja_JP)
IMPL_CREATEINSTANCE( ignoreHyuByu_ja_JP)
IMPL_CREATEINSTANCE( ignoreSeZe_ja_JP)
IMPL_CREATEINSTANCE( ignoreIandEfollowedByYa_ja_JP)
IMPL_CREATEINSTANCE( ignoreKiKuFollowedBySa_ja_JP)
IMPL_CREATEINSTANCE( ignoreSize_ja_JP)
IMPL_CREATEINSTANCE( ignoreProlongedSoundMark_ja_JP)
IMPL_CREATEINSTANCE( ignoreMiddleDot_ja_JP)
IMPL_CREATEINSTANCE( ignoreSpace_ja_JP)

IMPL_CREATEINSTANCE( TextToChuyin_zh_TW )
IMPL_CREATEINSTANCE( TextToPinyin_zh_CN )

IMPL_CREATEINSTANCE( NumToCharLower_zh_CN )
IMPL_CREATEINSTANCE( NumToCharUpper_zh_CN )
IMPL_CREATEINSTANCE( NumToCharLower_zh_TW )
IMPL_CREATEINSTANCE( NumToCharUpper_zh_TW )
IMPL_CREATEINSTANCE( NumToCharHalfwidth )
IMPL_CREATEINSTANCE( NumToCharFullwidth )
IMPL_CREATEINSTANCE( NumToCharKanjiShort_ja_JP )
IMPL_CREATEINSTANCE( NumToCharKanjiTraditional_ja_JP )
IMPL_CREATEINSTANCE( NumToCharHangul_ko )
IMPL_CREATEINSTANCE( NumToCharLower_ko )
IMPL_CREATEINSTANCE( NumToCharUpper_ko )
IMPL_CREATEINSTANCE( NumToCharIndic_ar )
IMPL_CREATEINSTANCE( NumToCharEastIndic_ar )
IMPL_CREATEINSTANCE( NumToCharIndic_hi )
IMPL_CREATEINSTANCE( NumToChar_th )

IMPL_CREATEINSTANCE( CharToNumLower_zh_CN )
IMPL_CREATEINSTANCE( CharToNumUpper_zh_CN )
IMPL_CREATEINSTANCE( CharToNumLower_zh_TW )
IMPL_CREATEINSTANCE( CharToNumUpper_zh_TW )
IMPL_CREATEINSTANCE( CharToNumFullwidth )
IMPL_CREATEINSTANCE( CharToNumKanjiShort_ja_JP )
IMPL_CREATEINSTANCE( CharToNumKanjiTraditional_ja_JP )
IMPL_CREATEINSTANCE( CharToNumHangul_ko )
IMPL_CREATEINSTANCE( CharToNumLower_ko )
IMPL_CREATEINSTANCE( CharToNumUpper_ko )
IMPL_CREATEINSTANCE( CharToNumIndic_ar )
IMPL_CREATEINSTANCE( CharToNumEastIndic_ar )
IMPL_CREATEINSTANCE( CharToNumIndic_hi )
IMPL_CREATEINSTANCE( CharToNum_th )

IMPL_CREATEINSTANCE( NumToTextLower_zh_CN )
IMPL_CREATEINSTANCE( NumToTextUpper_zh_CN )
IMPL_CREATEINSTANCE( NumToTextLower_zh_TW )
IMPL_CREATEINSTANCE( NumToTextUpper_zh_TW )
IMPL_CREATEINSTANCE( NumToTextFullwidth_zh_CN )
IMPL_CREATEINSTANCE( NumToTextFullwidth_zh_TW )
IMPL_CREATEINSTANCE( NumToTextFullwidth_ja_JP )
IMPL_CREATEINSTANCE( NumToTextFullwidth_ko )
IMPL_CREATEINSTANCE( NumToTextKanjiLongModern_ja_JP )
IMPL_CREATEINSTANCE( NumToTextKanjiLongTraditional_ja_JP )
IMPL_CREATEINSTANCE( NumToTextKanjiShortModern_ja_JP )
IMPL_CREATEINSTANCE( NumToTextKanjiShortTraditional_ja_JP )
IMPL_CREATEINSTANCE( NumToTextFormalHangul_ko )
IMPL_CREATEINSTANCE( NumToTextFormalLower_ko )
IMPL_CREATEINSTANCE( NumToTextFormalUpper_ko )
IMPL_CREATEINSTANCE( NumToTextInformalHangul_ko )
IMPL_CREATEINSTANCE( NumToTextInformalUpper_ko )
IMPL_CREATEINSTANCE( NumToTextInformalLower_ko )

IMPL_CREATEINSTANCE( TextToNumLower_zh_CN )
IMPL_CREATEINSTANCE( TextToNumUpper_zh_CN )
IMPL_CREATEINSTANCE( TextToNumLower_zh_TW )
IMPL_CREATEINSTANCE( TextToNumUpper_zh_TW )
IMPL_CREATEINSTANCE( TextToNumKanjiLongModern_ja_JP )
IMPL_CREATEINSTANCE( TextToNumKanjiLongTraditional_ja_JP )
IMPL_CREATEINSTANCE( TextToNumFormalHangul_ko )
IMPL_CREATEINSTANCE( TextToNumFormalLower_ko )
IMPL_CREATEINSTANCE( TextToNumFormalUpper_ko )
IMPL_CREATEINSTANCE( TextToNumInformalHangul_ko )
IMPL_CREATEINSTANCE( TextToNumInformalUpper_ko )
IMPL_CREATEINSTANCE( TextToNumInformalLower_ko )

IMPL_CREATEINSTANCE( NumToTextDate_zh )
IMPL_CREATEINSTANCE( NumToTextAIUFullWidth_ja_JP )
IMPL_CREATEINSTANCE( NumToTextAIUHalfWidth_ja_JP )
IMPL_CREATEINSTANCE( NumToTextIROHAFullWidth_ja_JP )
IMPL_CREATEINSTANCE( NumToTextIROHAHalfWidth_ja_JP )
IMPL_CREATEINSTANCE( NumToTextCircledNumber )
IMPL_CREATEINSTANCE( NumToTextHangulJamo_ko )
IMPL_CREATEINSTANCE( NumToTextHangulSyllable_ko )
IMPL_CREATEINSTANCE( NumToTextHangulCircledJamo_ko )
IMPL_CREATEINSTANCE( NumToTextHangulCircledSyllable_ko )
IMPL_CREATEINSTANCE( NumToTextTianGan_zh )
IMPL_CREATEINSTANCE( NumToTextDiZi_zh )

IMPL_CREATEINSTANCE( fullwidthKatakanaToHalfwidthKatakana )
IMPL_CREATEINSTANCE( halfwidthKatakanaToFullwidthKatakana )
IMPL_CREATEINSTANCE( fullwidthToHalfwidthLikeASC )
IMPL_CREATEINSTANCE( halfwidthToFullwidthLikeJIS )

IMPL_CREATEINSTANCE_MSF( OrdinalSuffix )

static const struct InstancesArray {
        const sal_Char* pServiceNm;
        const sal_Char* pImplementationNm;
        FN_CreateInstance pFn;
} aInstances[] = {
    {   "com.sun.star.i18n.LocaleData",
        "com.sun.star.i18n.LocaleData",
        & LocaleData_CreateInstance },
    {   "com.sun.star.i18n.NumberFormatMapper",
        "com.sun.star.i18n.NumberFormatCodeMapper",
        & NumberFormatCodeMapper_CreateInstance },
    {   "com.sun.star.i18n.NativeNumberSupplier",
        "com.sun.star.i18n.NativeNumberSupplier",
        & NativeNumberSupplier_CreateInstance },
    {   "com.sun.star.text.DefaultNumberingProvider",
        "com.sun.star.text.DefaultNumberingProvider",
        &DefaultNumberingProvider_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier",
        "com.sun.star.i18n.IndexEntrySupplier",
        &IndexEntrySupplier_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_asian",
        "com.sun.star.i18n.IndexEntrySupplier_asian",
        &IndexEntrySupplier_asian_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric first)",
        "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric first)",
        &IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_syllable_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric last)",
        "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric last)",
        &IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_syllable_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric first) (grouped by syllable)",
        "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric first) (grouped by syllable)",
        &IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_syllable_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric first) (grouped by consonant)",
        "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric first) (grouped by consonant)",
        &IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_consonant_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric last) (grouped by syllable)",
        "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric last) (grouped by syllable)",
        &IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_syllable_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric last) (grouped by consonant)",
        "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic (alphanumeric last) (grouped by consonant)",
        &IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_consonant_CreateInstance },
    {       "com.sun.star.i18n.IndexEntrySupplier_Unicode",
        "com.sun.star.i18n.IndexEntrySupplier_Unicode",
        &IndexEntrySupplier_Unicode_CreateInstance },
    {   "com.sun.star.i18n.LocaleCalendar",
        "com.sun.star.i18n.CalendarImpl",
        &CalendarImpl_CreateInstance },
    {   "com.sun.star.i18n.Calendar_gregorian",
        "com.sun.star.i18n.Calendar_gregorian",
        &Calendar_gregorian_CreateInstance },
    {   "com.sun.star.i18n.Calendar_gengou",
        "com.sun.star.i18n.Calendar_gengou",
        &Calendar_gengou_CreateInstance },
    {   "com.sun.star.i18n.Calendar_ROC",
        "com.sun.star.i18n.Calendar_ROC",
        &Calendar_ROC_CreateInstance },
    {   "com.sun.star.i18n.Calendar_hanja_yoil",
        "com.sun.star.i18n.Calendar_hanja_yoil",
        &Calendar_hanja_CreateInstance },
    {   "com.sun.star.i18n.Calendar_hanja",
        "com.sun.star.i18n.Calendar_hanja",
        &Calendar_hanja_CreateInstance },
    {   "com.sun.star.i18n.Calendar_hijri",
        "com.sun.star.i18n.Calendar_hijri",
        &Calendar_hijri_CreateInstance },
    {   "com.sun.star.i18n.Calendar_jewish",
        "com.sun.star.i18n.Calendar_jewish",
        &Calendar_jewish_CreateInstance },
    {   "com.sun.star.i18n.Calendar_buddhist",
        "com.sun.star.i18n.Calendar_buddhist",
        &Calendar_buddhist_CreateInstance },
    {   "com.sun.star.i18n.BreakIterator",
        "com.sun.star.i18n.BreakIterator",
        &BreakIteratorImpl_CreateInstance },
    {   "com.sun.star.i18n.BreakIterator_Unicode",
        "com.sun.star.i18n.BreakIterator_Unicode",
        &BreakIterator_Unicode_CreateInstance },
    {   "com.sun.star.i18n.BreakIterator_ja",
        "com.sun.star.i18n.BreakIterator_ja",
        &BreakIterator_ja_CreateInstance },
    {   "com.sun.star.i18n.BreakIterator_zh",
        "com.sun.star.i18n.BreakIterator_zh",
        &BreakIterator_zh_CreateInstance },
    {   "com.sun.star.i18n.BreakIterator_zh_TW",
        "com.sun.star.i18n.BreakIterator_zh_TW",
        &BreakIterator_zh_TW_CreateInstance },
    {   "com.sun.star.i18n.BreakIterator_ko",
        "com.sun.star.i18n.BreakIterator_ko",
        &BreakIterator_ko_CreateInstance },
    {   "com.sun.star.i18n.BreakIterator_th",
        "com.sun.star.i18n.BreakIterator_th",
        &BreakIterator_th_CreateInstance },
    {   "com.sun.star.i18n.Collator",
        "com.sun.star.i18n.Collator",
        &CollatorImpl_CreateInstance },
    {   "com.sun.star.i18n.ChapterCollator",
        "com.sun.star.i18n.ChapterCollator",
        &ChapterCollator_CreateInstance },
    {   "com.sun.star.i18n.Collator_Unicode",
        "com.sun.star.i18n.Collator_Unicode",
        &Collator_Unicode_CreateInstance },
    {   "com.sun.star.i18n.ScriptTypeDetector",
        "com.sun.star.i18n.ScriptTypeDetector",
        &UnoScriptTypeDetector_CreateInstance },
    {   "com.sun.star.i18n.CharacterClassification",
        "com.sun.star.i18n.CharacterClassification",
        &CharacterClassificationImpl_CreateInstance },
    {   "com.sun.star.i18n.CharacterClassification_Unicode",
        "com.sun.star.i18n.CharacterClassification_Unicode",
        &cclass_Unicode_CreateInstance },
    {   "com.sun.star.i18n.InputSequenceChecker",
        "com.sun.star.i18n.InputSequenceChecker",
        &InputSequenceCheckerImpl_CreateInstance },
    {   "com.sun.star.i18n.InputSequenceChecker_th",
        "com.sun.star.i18n.InputSequenceChecker_th",
        &InputSequenceChecker_th_CreateInstance },
    {   "com.sun.star.i18n.InputSequenceChecker_hi",
        "com.sun.star.i18n.InputSequenceChecker_hi",
        &InputSequenceChecker_hi_CreateInstance },
    {   "com.sun.star.i18n.TextConversion",
        "com.sun.star.i18n.TextConversion",
        &TextConversionImpl_CreateInstance },
    {   "com.sun.star.i18n.TextConversion_ko",
        "com.sun.star.i18n.TextConversion_ko",
        &TextConversion_ko_CreateInstance },
    {   "com.sun.star.i18n.TextConversion_zh",
        "com.sun.star.i18n.TextConversion_zh",
        &TextConversion_zh_CreateInstance },
    {   TRLT_SERVICELNAME,
        TRLT_IMPLNAME ,
        &TransliterationImpl_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "UPPERCASE_LOWERCASE",
        &Transliteration_u2l_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "LOWERCASE_UPPERCASE",
        &Transliteration_l2u_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "SENTENCE_CASE",
        &Transliteration_sentencecase_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "TITLE_CASE",
        &Transliteration_titlecase_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "TOGGLE_CASE",
        &Transliteration_togglecase_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "IGNORE_CASE",
        &Transliteration_caseignore_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "HIRAGANA_KATAKANA",
        &hiraganaToKatakana_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "KATAKANA_HIRAGANA",
        &katakanaToHiragana_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "IGNORE_KANA",
        &ignoreKana_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "FULLWIDTH_HALFWIDTH",
        &fullwidthToHalfwidth_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "HALFWIDTH_FULLWIDTH",
        &halfwidthToFullwidth_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "IGNORE_WIDTH",
        &ignoreWidth_CreateInstance },
    IMPL_TRANSLITERATION_ITEM (smallToLarge_ja_JP),
    IMPL_TRANSLITERATION_ITEM (largeToSmall_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreTraditionalKanji_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreTraditionalKana_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreKana),
    IMPL_TRANSLITERATION_ITEM (ignoreMinusSign_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreIterationMark_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreSeparator_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreZiZu_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreBaFa_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreTiJi_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreHyuByu_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreSeZe_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreIandEfollowedByYa_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreKiKuFollowedBySa_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreSize_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreProlongedSoundMark_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreMiddleDot_ja_JP),
    IMPL_TRANSLITERATION_ITEM (ignoreSpace_ja_JP),
    IMPL_TRANSLITERATION_ITEM (TextToPinyin_zh_CN),
    IMPL_TRANSLITERATION_ITEM (TextToChuyin_zh_TW),

    IMPL_TRANSLITERATION_ITEM (NumToCharUpper_zh_CN),
    IMPL_TRANSLITERATION_ITEM (NumToCharLower_zh_CN),
    IMPL_TRANSLITERATION_ITEM (NumToCharUpper_zh_TW),
    IMPL_TRANSLITERATION_ITEM (NumToCharLower_zh_TW),
    IMPL_TRANSLITERATION_ITEM (NumToCharHalfwidth),
    IMPL_TRANSLITERATION_ITEM (NumToCharFullwidth),
    IMPL_TRANSLITERATION_ITEM (NumToCharKanjiShort_ja_JP),
    IMPL_TRANSLITERATION_ITEM (NumToCharKanjiTraditional_ja_JP),
    IMPL_TRANSLITERATION_ITEM (NumToCharLower_ko),
    IMPL_TRANSLITERATION_ITEM (NumToCharUpper_ko),
    IMPL_TRANSLITERATION_ITEM (NumToCharHangul_ko),
    IMPL_TRANSLITERATION_ITEM (NumToCharIndic_ar),
    IMPL_TRANSLITERATION_ITEM (NumToCharEastIndic_ar),
    IMPL_TRANSLITERATION_ITEM (NumToCharIndic_hi),
    IMPL_TRANSLITERATION_ITEM (NumToChar_th),

    IMPL_TRANSLITERATION_ITEM (CharToNumUpper_zh_CN),
    IMPL_TRANSLITERATION_ITEM (CharToNumLower_zh_CN),
    IMPL_TRANSLITERATION_ITEM (CharToNumUpper_zh_TW),
    IMPL_TRANSLITERATION_ITEM (CharToNumLower_zh_TW),
    IMPL_TRANSLITERATION_ITEM (CharToNumFullwidth),
    IMPL_TRANSLITERATION_ITEM (CharToNumKanjiShort_ja_JP),
    IMPL_TRANSLITERATION_ITEM (CharToNumKanjiTraditional_ja_JP),
    IMPL_TRANSLITERATION_ITEM (CharToNumLower_ko),
    IMPL_TRANSLITERATION_ITEM (CharToNumUpper_ko),
    IMPL_TRANSLITERATION_ITEM (CharToNumHangul_ko),
    IMPL_TRANSLITERATION_ITEM (CharToNumIndic_ar),
    IMPL_TRANSLITERATION_ITEM (CharToNumEastIndic_ar),
    IMPL_TRANSLITERATION_ITEM (CharToNumIndic_hi),
    IMPL_TRANSLITERATION_ITEM (CharToNum_th),

    IMPL_TRANSLITERATION_ITEM (NumToTextUpper_zh_CN),
    IMPL_TRANSLITERATION_ITEM (NumToTextLower_zh_CN),
    IMPL_TRANSLITERATION_ITEM (NumToTextUpper_zh_TW),
    IMPL_TRANSLITERATION_ITEM (NumToTextLower_zh_TW),
    IMPL_TRANSLITERATION_ITEM (NumToTextFullwidth_zh_CN),
    IMPL_TRANSLITERATION_ITEM (NumToTextFullwidth_zh_TW),
    IMPL_TRANSLITERATION_ITEM (NumToTextFullwidth_ja_JP),
    IMPL_TRANSLITERATION_ITEM (NumToTextFullwidth_ko),
    IMPL_TRANSLITERATION_ITEM (NumToTextKanjiLongModern_ja_JP),
    IMPL_TRANSLITERATION_ITEM (NumToTextKanjiLongTraditional_ja_JP),
    IMPL_TRANSLITERATION_ITEM (NumToTextKanjiShortModern_ja_JP),
    IMPL_TRANSLITERATION_ITEM (NumToTextKanjiShortTraditional_ja_JP),
    IMPL_TRANSLITERATION_ITEM (NumToTextInformalHangul_ko),
    IMPL_TRANSLITERATION_ITEM (NumToTextInformalLower_ko),
    IMPL_TRANSLITERATION_ITEM (NumToTextInformalUpper_ko),
    IMPL_TRANSLITERATION_ITEM (NumToTextFormalHangul_ko),
    IMPL_TRANSLITERATION_ITEM (NumToTextFormalLower_ko),
    IMPL_TRANSLITERATION_ITEM (NumToTextFormalUpper_ko),

    IMPL_TRANSLITERATION_ITEM (TextToNumUpper_zh_CN),
    IMPL_TRANSLITERATION_ITEM (TextToNumLower_zh_CN),
    IMPL_TRANSLITERATION_ITEM (TextToNumUpper_zh_TW),
    IMPL_TRANSLITERATION_ITEM (TextToNumLower_zh_TW),
    IMPL_TRANSLITERATION_ITEM (TextToNumKanjiLongModern_ja_JP),
    IMPL_TRANSLITERATION_ITEM (TextToNumKanjiLongTraditional_ja_JP),
    IMPL_TRANSLITERATION_ITEM (TextToNumInformalHangul_ko),
    IMPL_TRANSLITERATION_ITEM (TextToNumInformalLower_ko),
    IMPL_TRANSLITERATION_ITEM (TextToNumInformalUpper_ko),
    IMPL_TRANSLITERATION_ITEM (TextToNumFormalHangul_ko),
    IMPL_TRANSLITERATION_ITEM (TextToNumFormalLower_ko),
    IMPL_TRANSLITERATION_ITEM (TextToNumFormalUpper_ko),

    IMPL_TRANSLITERATION_ITEM (NumToTextDate_zh),
    IMPL_TRANSLITERATION_ITEM (NumToTextAIUFullWidth_ja_JP),
    IMPL_TRANSLITERATION_ITEM (NumToTextAIUHalfWidth_ja_JP),
    IMPL_TRANSLITERATION_ITEM (NumToTextIROHAFullWidth_ja_JP),
    IMPL_TRANSLITERATION_ITEM (NumToTextIROHAHalfWidth_ja_JP),
    IMPL_TRANSLITERATION_ITEM (NumToTextCircledNumber),
    IMPL_TRANSLITERATION_ITEM (NumToTextHangulJamo_ko),
    IMPL_TRANSLITERATION_ITEM (NumToTextHangulSyllable_ko),
    IMPL_TRANSLITERATION_ITEM (NumToTextHangulCircledJamo_ko),
    IMPL_TRANSLITERATION_ITEM (NumToTextHangulCircledSyllable_ko),
    IMPL_TRANSLITERATION_ITEM (NumToTextTianGan_zh),
    IMPL_TRANSLITERATION_ITEM (NumToTextDiZi_zh),

    {   "com.sun.star.i18n.OrdinalSuffix",
        "com.sun.star.i18n.OrdinalSuffix",
        & OrdinalSuffix_CreateInstance },

    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "FULLWIDTHKATAKANA_HALFWIDTHKATAKANA",
        &fullwidthKatakanaToHalfwidthKatakana_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "HALFWIDTHKATAKANA_FULLWIDTHKATAKANA",
        &halfwidthKatakanaToFullwidthKatakana_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "FULLWIDTH_HALFWIDTH_LIKE_ASC",
        &fullwidthToHalfwidthLikeASC_CreateInstance },
    {   TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "HALFWIDTH_FULLWIDTH_LIKE_JIS",
        &halfwidthToFullwidthLikeJIS_CreateInstance },
// add here new services !!
    { 0, 0, 0 }
};


extern "C"
{

SAL_DLLPUBLIC_EXPORT void* SAL_CALL i18npool_component_getFactory( const sal_Char* sImplementationName, void* _pServiceManager, SAL_UNUSED_PARAMETER void* /*_pRegistryKey*/ )
{
    void* pRet = NULL;

    ::com::sun::star::lang::XMultiServiceFactory* pServiceManager =
        reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >
            ( _pServiceManager );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XSingleServiceFactory > xFactory;

    for( const InstancesArray* pArr = aInstances; pArr->pServiceNm; ++pArr )
    {
        if( 0 == rtl_str_compare( sImplementationName, pArr->pImplementationNm ) )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                ::rtl::OUString::createFromAscii( pArr->pServiceNm );
            xFactory = ::cppu::createSingleFactory(
                    pServiceManager,
                    ::rtl::OUString::createFromAscii( pArr->pImplementationNm ),
                    *pArr->pFn, aServiceNames );
            break;
        }
    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

}   // "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
