/*************************************************************************
 *
 *  $RCSfile: registerservices.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 16:00:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#include <cppuhelper/factory.hxx>

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

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
#include <breakiterator_hi.hxx>
#include <breakiterator_unicode.hxx>

#define INDEXENTRYSUPPLIER_ALL
#include <indexentrysupplier.hxx>
#include <indexentrysupplier_asian.hxx>
#include <indexentrysupplier_western.hxx>
#include <indexentrysupplier_complex.hxx>
#include <indexentrysupplier_default.hxx>

#define CCLASS_ALL
#include <characterclassificationImpl.hxx>
#include <scripttypedetector.hxx>
#include <cclass_unicode.hxx>

#define COLLATOR_ALL
#include <collatorImpl.hxx>
#include <chaptercollator.hxx>
#include <collator_unicode.hxx>
#include <collator_icu.hxx>
#include <collator_simple.hxx>
#include <collator_cjk.hxx>

#include <inputsequencechecker.hxx>
#include <inputsequencechecker_th.hxx>
#include <inputsequencechecker_hi.hxx>

#include <textsearch.hxx>


#define IMPL_CREATEINSTANCE( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   \
        SAL_CALL ImplName##_CreateInstance(                                 \
            const ::com::sun::star::uno::Reference<                         \
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

IMPL_CREATEINSTANCE_MSF( NumberFormatCodeMapper )
IMPL_CREATEINSTANCE( NativeNumberSupplier )
IMPL_CREATEINSTANCE( LocaleData )
IMPL_CREATEINSTANCE_MSF( DefaultNumberingProvider )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_zh_pinyin )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_zh_radical )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_zh_stroke )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_zh_zhuyin )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_zh_TW_radical )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_zh_TW_stroke )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_ko_dict )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_syllable )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_consonant )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_syllable )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_consonant )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_alphanumeric )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_nl_alphanumeric )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_da_alphanumeric )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_ar_alphanumeric )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_he_alphanumeric )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_hi_alphanumeric )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier_th_alphanumeric )
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
IMPL_CREATEINSTANCE( BreakIterator_ca )
IMPL_CREATEINSTANCE( BreakIterator_ja )
IMPL_CREATEINSTANCE( BreakIterator_zh )
IMPL_CREATEINSTANCE( BreakIterator_ko )
IMPL_CREATEINSTANCE( BreakIterator_th )
IMPL_CREATEINSTANCE( BreakIterator_hi )
IMPL_CREATEINSTANCE_MSF( ChapterCollator )
IMPL_CREATEINSTANCE_MSF( CollatorImpl )
IMPL_CREATEINSTANCE( Collator_ICU )
IMPL_CREATEINSTANCE( Collator_Simple )
IMPL_CREATEINSTANCE( Collator_Unicode )
IMPL_CREATEINSTANCE( Collator_zh_pinyin )
IMPL_CREATEINSTANCE( Collator_zh_radical )
IMPL_CREATEINSTANCE( Collator_zh_stroke )
IMPL_CREATEINSTANCE( Collator_zh_zhuyin )
IMPL_CREATEINSTANCE( Collator_zh_TW_radical )
IMPL_CREATEINSTANCE( Collator_zh_TW_stroke )
IMPL_CREATEINSTANCE( Collator_ko_dict )
IMPL_CREATEINSTANCE( Collator_ko_charset )
IMPL_CREATEINSTANCE( Collator_zh_charset )
IMPL_CREATEINSTANCE( Collator_ja_charset )
IMPL_CREATEINSTANCE( Collator_ja_phonetic_alphanumeric_first )
IMPL_CREATEINSTANCE( Collator_ja_phonetic_alphanumeric_last )
IMPL_CREATEINSTANCE( Collator_zh_TW_charset )
IMPL_CREATEINSTANCE( Collator_de_phonebook )

IMPL_CREATEINSTANCE_MSF( CharacterClassificationImpl )
IMPL_CREATEINSTANCE_MSF( cclass_Unicode )
IMPL_CREATEINSTANCE_MSF( TransliterationImpl )
IMPL_CREATEINSTANCE( ScriptTypeDetector )

IMPL_CREATEINSTANCE_MSF( InputSequenceCheckerImpl )
IMPL_CREATEINSTANCE( InputSequenceChecker_th )
IMPL_CREATEINSTANCE( InputSequenceChecker_hi )

IMPL_CREATEINSTANCE( Transliteration_u2l )
IMPL_CREATEINSTANCE( Transliteration_l2u )
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

IMPL_CREATEINSTANCE_MSF( TextSearch )

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
    {   "com.sun.star.i18n.IndexEntrySupplier_zh_pinyin",
        "com.sun.star.i18n.IndexEntrySupplier_zh_pinyin",
        &IndexEntrySupplier_zh_pinyin_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_zh_radical",
        "com.sun.star.i18n.IndexEntrySupplier_zh_radical",
        &IndexEntrySupplier_zh_radical_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_zh_stroke",
        "com.sun.star.i18n.IndexEntrySupplier_zh_stroke",
        &IndexEntrySupplier_zh_stroke_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_zh_zhuyin",
        "com.sun.star.i18n.IndexEntrySupplier_zh_zhuyin",
        &IndexEntrySupplier_zh_zhuyin_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_zh_TW_radical",
        "com.sun.star.i18n.IndexEntrySupplier_zh_TW_radical",
        &IndexEntrySupplier_zh_TW_radical_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_zh_TW_stroke",
        "com.sun.star.i18n.IndexEntrySupplier_zh_TW_stroke",
        &IndexEntrySupplier_zh_TW_stroke_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_ko_dict",
        "com.sun.star.i18n.IndexEntrySupplier_ko_dict",
        &IndexEntrySupplier_ko_dict_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_ko_charset",
        "com.sun.star.i18n.IndexEntrySupplier_ko_charset",
        &IndexEntrySupplier_ko_dict_CreateInstance },   // share same table with ko_dict
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
    {   "com.sun.star.i18n.IndexEntrySupplier_da_alphanumeric",
        "com.sun.star.i18n.IndexEntrySupplier_da_alphanumeric",
        &IndexEntrySupplier_da_alphanumeric_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_nl_alphanumeric",
        "com.sun.star.i18n.IndexEntrySupplier_nl_alphanumeric",
        &IndexEntrySupplier_nl_alphanumeric_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_alphanumeric",
        "com.sun.star.i18n.IndexEntrySupplier_alphanumeric",
        &IndexEntrySupplier_alphanumeric_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_ar_alphanumeric",
        "com.sun.star.i18n.IndexEntrySupplier_ar_alphanumeric",
        &IndexEntrySupplier_ar_alphanumeric_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_he_alphanumeric",
        "com.sun.star.i18n.IndexEntrySupplier_he_alphanumeric",
        &IndexEntrySupplier_he_alphanumeric_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_hi_alphanumeric",
        "com.sun.star.i18n.IndexEntrySupplier_hi_alphanumeric",
        &IndexEntrySupplier_hi_alphanumeric_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier_th_alphanumeric",
        "com.sun.star.i18n.IndexEntrySupplier_th_alphanumeric",
        &IndexEntrySupplier_th_alphanumeric_CreateInstance },
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
     {  "com.sun.star.i18n.BreakIterator_ca",
        "com.sun.star.i18n.BreakIterator_ca",
        &BreakIterator_ca_CreateInstance },
    {   "com.sun.star.i18n.BreakIterator_ja",
        "com.sun.star.i18n.BreakIterator_ja",
        &BreakIterator_ja_CreateInstance },
    {   "com.sun.star.i18n.BreakIterator_zh",
        "com.sun.star.i18n.BreakIterator_zh",
        &BreakIterator_zh_CreateInstance },
    {   "com.sun.star.i18n.BreakIterator_ko",
        "com.sun.star.i18n.BreakIterator_ko",
        &BreakIterator_ko_CreateInstance },
    {   "com.sun.star.i18n.BreakIterator_th",
        "com.sun.star.i18n.BreakIterator_th",
        &BreakIterator_th_CreateInstance },
     {  "com.sun.star.i18n.BreakIterator_hi",
        "com.sun.star.i18n.BreakIterator_hi",
        &BreakIterator_hi_CreateInstance },
    {       "com.sun.star.i18n.Collator",
                "com.sun.star.i18n.Collator",
                &CollatorImpl_CreateInstance },
        {       "com.sun.star.i18n.ChapterCollator",
                "com.sun.star.i18n.ChapterCollator",
                &ChapterCollator_CreateInstance },
        {       "com.sun.star.i18n.Collator_Unicode",
                "com.sun.star.i18n.Collator_Unicode",
                &Collator_Unicode_CreateInstance },
        {       "com.sun.star.i18n.Collator_ICU",
                "com.sun.star.i18n.Collator_ICU",
                &Collator_ICU_CreateInstance },
        {       "com.sun.star.i18n.Collator_Simple",
                "com.sun.star.i18n.Collator_Simple",
                &Collator_Simple_CreateInstance },
        {       "com.sun.star.i18n.Collator_zh_pinyin",
                "com.sun.star.i18n.Collator_zh_pinyin",
                &Collator_zh_pinyin_CreateInstance },
        {       "com.sun.star.i18n.Collator_zh_radical",
                "com.sun.star.i18n.Collator_zh_radical",
                &Collator_zh_radical_CreateInstance },
        {       "com.sun.star.i18n.Collator_zh_stroke",
                "com.sun.star.i18n.Collator_zh_stroke",
                &Collator_zh_stroke_CreateInstance },
        {       "com.sun.star.i18n.Collator_zh_zhuyin",
                "com.sun.star.i18n.Collator_zh_zhuyin",
                &Collator_zh_zhuyin_CreateInstance },
        {       "com.sun.star.i18n.Collator_zh_TW_radical",
                "com.sun.star.i18n.Collator_zh_TW_radical",
                &Collator_zh_TW_radical_CreateInstance },
        {       "com.sun.star.i18n.Collator_zh_TW_stroke",
                "com.sun.star.i18n.Collator_zh_TW_stroke",
                &Collator_zh_TW_stroke_CreateInstance },
        {       "com.sun.star.i18n.Collator_ko_dict",
                "com.sun.star.i18n.Collator_ko_dict",
                &Collator_ko_dict_CreateInstance },
        {       "com.sun.star.i18n.Collator_ko_charset",
                "com.sun.star.i18n.Collator_ko_charset",
                &Collator_ko_charset_CreateInstance },
        {       "com.sun.star.i18n.Collator_ja_phonetic (alphanumeric first)",
                "com.sun.star.i18n.Collator_ja_phonetic (alphanumeric first)",
                &Collator_ja_phonetic_alphanumeric_first_CreateInstance },
        {       "com.sun.star.i18n.Collator_ja_phonetic (alphanumeric last)",
                "com.sun.star.i18n.Collator_ja_phonetic (alphanumeric last)",
                &Collator_ja_phonetic_alphanumeric_last_CreateInstance },
        {       "com.sun.star.i18n.Collator_ja_charset",
                "com.sun.star.i18n.Collator_ja_charset",
                &Collator_ja_charset_CreateInstance },
        {       "com.sun.star.i18n.Collator_zh_charset",
                "com.sun.star.i18n.Collator_zh_charset",
                &Collator_zh_charset_CreateInstance },
        {       "com.sun.star.i18n.Collator_zh_TW_charset",
                "com.sun.star.i18n.Collator_zh_TW_charset",
                &Collator_zh_TW_charset_CreateInstance },
        {       "com.sun.star.i18n.Collator_de_phonebook",
                "com.sun.star.i18n.Collator_de_phonebook",
                &Collator_de_phonebook_CreateInstance },
    {   "com.sun.star.i18n.ScriptTypeDetector",
        "com.sun.star.i18n.ScriptTypeDetector",
        &ScriptTypeDetector_CreateInstance },
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
  { "com.sun.star.i18n.InputSequenceChecker_hi",
        "com.sun.star.i18n.InputSequenceChecker_hi",
        &InputSequenceChecker_hi_CreateInstance },
  { TRLT_SERVICELNAME,
        TRLT_IMPLNAME ,
        &TransliterationImpl_CreateInstance },
    {       TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "UPPERCASE_LOWERCASE",
        &Transliteration_u2l_CreateInstance },
    {       TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "LOWERCASE_UPPERCASE",
        &Transliteration_l2u_CreateInstance },
    {       TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "IGNORE_CASE",
        &Transliteration_caseignore_CreateInstance },
    {       TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "HIRAGANA_KATAKANA",
        &hiraganaToKatakana_CreateInstance },
    {       TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "KATAKANA_HIRAGANA",
        &katakanaToHiragana_CreateInstance },
    {       TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "IGNORE_KANA",
        &ignoreKana_CreateInstance },
    {       TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "FULLWIDTH_HALFWIDTH",
        &fullwidthToHalfwidth_CreateInstance },
    {       TRLT_SERVICELNAME_L10N,
        TRLT_IMPLNAME_PREFIX  "HALFWIDTH_FULLWIDTH",
        &halfwidthToFullwidth_CreateInstance },
    {       TRLT_SERVICELNAME_L10N,
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

    {   "com.sun.star.util.TextSearch",
        "com.sun.star.util.TextSearch",
        &TextSearch_CreateInstance },

// add here new services !!
    { 0, 0, 0 }
};


extern "C"
{

void SAL_CALL component_getImplementationEnvironment( const sal_Char** ppEnvTypeName, uno_Environment** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void* _pServiceManager, void* _pRegistryKey )
{
    if (_pRegistryKey)
    {
        ::com::sun::star::registry::XRegistryKey * pRegistryKey =
            reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >(
                                _pRegistryKey );
        ::com::sun::star::uno::Reference<
                        ::com::sun::star::registry::XRegistryKey > xNewKey;

        for( const InstancesArray* pArr = aInstances; pArr->pServiceNm; ++pArr )
        {
            xNewKey = pRegistryKey->createKey(
                    ::rtl::OUString::createFromAscii( pArr->pImplementationNm )  );
            xNewKey = xNewKey->createKey(
                    ::rtl::OUString::createFromAscii( "/UNO/SERVICES" ) );
            xNewKey->createKey(
                    ::rtl::OUString::createFromAscii( pArr->pServiceNm ) );
        }
    }
    return sal_True;
}

void* SAL_CALL component_getFactory( const sal_Char* sImplementationName, void* _pServiceManager, void* _pRegistryKey )
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
                    pServiceManager, aServiceNames.getArray()[0],
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
