/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <font/FallbackLanguage.hxx>
#include <i18nlangtag/languagetag.hxx>

class VclFallbackLanguageTest : public test::BootstrapFixture
{
public:
    VclFallbackLanguageTest()
        : BootstrapFixture(true, false)
    {
    }
};

namespace
{
const sal_uInt32 HAN = 0x4e2d; // U+4E2D CJK UNIFIED IDEOGRAPH-4E2D
const sal_uInt32 HANGUL = 0xd55c; // U+D55C HANGUL SYLLABLE HAN
const sal_uInt32 HIRAGANA = 0x3042; // U+3042 HIRAGANA LETTER A
const sal_uInt32 KATAKANA = 0x30a2; // U+30A2 KATAKANA LETTER A
const sal_uInt32 BOPOMOFO = 0x3105; // U+3105 BOPOMOFO LETTER B
const sal_uInt32 LATIN = 0x0041; // U+0041 LATIN CAPITAL LETTER A
}

CPPUNIT_TEST_FIXTURE(VclFallbackLanguageTest, testImpossibleCodePointForLang)
{
    const LanguageTag aEnglish(u"en-US"_ustr);
    const LanguageTag aHungarian(u"hu-HU"_ustr);
    const LanguageTag aNone(LANGUAGE_NONE);
    const LanguageTag aChinese(u"zh-CN"_ustr);
    const LanguageTag aTaiwanese(u"zh-TW"_ustr);
    const LanguageTag aJapanese(u"ja-JP"_ustr);
    const LanguageTag aKorean(u"ko-KR"_ustr);

    // Han is written in Chinese, Japanese and Korean, nothing else
    CPPUNIT_ASSERT(vcl::font::isImpossibleCodePointForLang(aEnglish, HAN));
    CPPUNIT_ASSERT(vcl::font::isImpossibleCodePointForLang(aHungarian, HAN));
    CPPUNIT_ASSERT(vcl::font::isImpossibleCodePointForLang(aNone, HAN));
    CPPUNIT_ASSERT(!vcl::font::isImpossibleCodePointForLang(aChinese, HAN));
    CPPUNIT_ASSERT(!vcl::font::isImpossibleCodePointForLang(aTaiwanese, HAN));
    CPPUNIT_ASSERT(!vcl::font::isImpossibleCodePointForLang(aJapanese, HAN));
    CPPUNIT_ASSERT(!vcl::font::isImpossibleCodePointForLang(aKorean, HAN));

    // Hangul is Korean only
    CPPUNIT_ASSERT(vcl::font::isImpossibleCodePointForLang(aEnglish, HANGUL));
    CPPUNIT_ASSERT(vcl::font::isImpossibleCodePointForLang(aChinese, HANGUL));
    CPPUNIT_ASSERT(!vcl::font::isImpossibleCodePointForLang(aKorean, HANGUL));

    // Kana is Japanese only
    CPPUNIT_ASSERT(vcl::font::isImpossibleCodePointForLang(aEnglish, HIRAGANA));
    CPPUNIT_ASSERT(vcl::font::isImpossibleCodePointForLang(aChinese, KATAKANA));
    CPPUNIT_ASSERT(!vcl::font::isImpossibleCodePointForLang(aJapanese, HIRAGANA));
    CPPUNIT_ASSERT(!vcl::font::isImpossibleCodePointForLang(aJapanese, KATAKANA));

    // Bopomofo is Chinese only
    CPPUNIT_ASSERT(vcl::font::isImpossibleCodePointForLang(aEnglish, BOPOMOFO));
    CPPUNIT_ASSERT(!vcl::font::isImpossibleCodePointForLang(aTaiwanese, BOPOMOFO));

    // Latin is written everywhere, a language tag never contradicts it
    CPPUNIT_ASSERT(!vcl::font::isImpossibleCodePointForLang(aEnglish, LATIN));
    CPPUNIT_ASSERT(!vcl::font::isImpossibleCodePointForLang(aChinese, LATIN));

    // an explicit script subtag is believed as it is
    CPPUNIT_ASSERT(!vcl::font::isImpossibleCodePointForLang(LanguageTag(u"en-Hani"_ustr), HAN));
}

CPPUNIT_TEST_FIXTURE(VclFallbackLanguageTest, testExemplarLangTagForCodePoint)
{
    // the CJK scripts give tags with a region, which both fontconfig and the
    // default font lists need to find a font by language. Han goes to the
    // default Asian language, which is Simplified Chinese unless configured.
    CPPUNIT_ASSERT_EQUAL(u"zh-CN"_ustr, vcl::font::getExemplarLangTagForCodePoint(HAN));
    CPPUNIT_ASSERT_EQUAL(u"ko-KR"_ustr, vcl::font::getExemplarLangTagForCodePoint(HANGUL));
    CPPUNIT_ASSERT_EQUAL(u"ja-JP"_ustr, vcl::font::getExemplarLangTagForCodePoint(HIRAGANA));
    CPPUNIT_ASSERT_EQUAL(u"ja-JP"_ustr, vcl::font::getExemplarLangTagForCodePoint(KATAKANA));
    CPPUNIT_ASSERT_EQUAL(u"zh-TW"_ustr, vcl::font::getExemplarLangTagForCodePoint(BOPOMOFO));

    // other scripts keep the language-script form
    CPPUNIT_ASSERT_EQUAL(u"en-Latn"_ustr, vcl::font::getExemplarLangTagForCodePoint(LATIN));
    CPPUNIT_ASSERT_EQUAL(u"und-zsye"_ustr, vcl::font::getExemplarLangTagForCodePoint(0x1f33f));
}

CPPUNIT_TEST_FIXTURE(VclFallbackLanguageTest, testLanguageForMissingChars)
{
    const LanguageTag aEnglish(u"en-US"_ustr);
    const LanguageTag aChinese(u"zh-CN"_ustr);
    const LanguageTag aKorean(u"ko-KR"_ustr);

    // English tagged Han text is looked up as Chinese
    CPPUNIT_ASSERT_EQUAL(
        u"zh-CN"_ustr,
        vcl::font::getLanguageForMissingChars(aEnglish, u"\u4e2d\u6587"_ustr).getBcp47());
    // English tagged Hangul text is looked up as Korean
    CPPUNIT_ASSERT_EQUAL(
        u"ko-KR"_ustr, vcl::font::getLanguageForMissingChars(aEnglish, u"\ud55c"_ustr).getBcp47());
    // a CJK language tag stays as it is for Han, whichever of the three it is
    CPPUNIT_ASSERT_EQUAL(
        u"zh-CN"_ustr, vcl::font::getLanguageForMissingChars(aChinese, u"\u4e2d"_ustr).getBcp47());
    CPPUNIT_ASSERT_EQUAL(u"ko-KR"_ustr,
                         vcl::font::getLanguageForMissingChars(aKorean, u"\u4e2d"_ustr).getBcp47());
    // Latin chars never change the tag
    CPPUNIT_ASSERT_EQUAL(u"en-US"_ustr,
                         vcl::font::getLanguageForMissingChars(aEnglish, u"abc"_ustr).getBcp47());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
