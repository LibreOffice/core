/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/i18n/TextConversion.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/i18n/TextConversionType.hpp>

namespace
{
class CharacterConversionTest : public test::BootstrapFixture
{
};

bool isValidOUString(const OUString& rString)
{
    OString sDummy;
    return rString.convertToString(&sDummy, RTL_TEXTENCODING_UTF8,
                                   RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                                       | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR);
}

// Test that Korean Hangul/Hanja character conversion only gives conversion
// candidate strings consisting of valid UTF-16 strings.
// Underlying logic is implemented in i18npool and gets used in class HangulHanjaConversion in editeng.
// Test is done in editeng (and not i18npool) tests to have access to a component context
// that supplies the required service com.sun.star.linguistic2.ConversionDictionaryList .
CPPUNIT_TEST_FIXTURE(CharacterConversionTest, testKoreanHangulHanjaCharacterConversion)
{
    css::lang::Locale aKoreanLocale;
    aKoreanLocale.Language = u"ko"_ustr;
    aKoreanLocale.Country = u"KR"_ustr;

    css::uno::Reference<css::i18n::XExtendedTextConversion> xConverter
        = css::i18n::TextConversion::create(m_xContext);
    CPPUNIT_ASSERT_MESSAGE("Creating text converter for Korean must succeed", xConverter);
    css::i18n::TextConversionResult aResult = xConverter->getConversions(
        u"한글맞춤법"_ustr, 0, 5, aKoreanLocale, css::i18n::TextConversionType::TO_HANJA,
        css::i18n::TextConversionOption::CHARACTER_BY_CHARACTER
            | css::i18n::TextConversionOption::IGNORE_POST_POSITIONAL_WORD);

    CPPUNIT_ASSERT_MESSAGE("Converter must returns character conversion candidates",
                           aResult.Candidates.size() > 0);

    for (const OUString& rCandidate : aResult.Candidates)
        CPPUNIT_ASSERT_MESSAGE("Conversion candidate must be a valid UTF-16 string",
                               isValidOUString(rCandidate));
}

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
