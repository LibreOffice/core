/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>

#include <comphelper/propertyvalue.hxx>

#include <unordered_map>

using namespace ::com::sun::star;

/// i18npool defaultnumberingprovider tests.
class I18npoolDefaultnumberingproviderTest : public test::BootstrapFixture
{
};

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testNumberingIdentifiers)
{
    // All numbering identifiers must be unique.
    std::unordered_map<OUString, sal_Int16> aMap;
    std::vector<OString> aFail;

    uno::Reference<text::XNumberingTypeInfo> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);

    // Do not use getSupportedNumberingTypes() because it depends on
    // configuration whether CTL and CJK numberings are included or not.
    // Also do not test for known values of
    // offapi/com/sun/star/style/NumberingType.idl and miss newly added values.
    // Instead, enumerate until an empty ID is returned but also check there
    // are at least the known NumberingType values covered, just in case the
    // table wasn't maintained. So this may have to be adapted from time to
    // time.
    constexpr sal_Int16 kLastKnown = css::style::NumberingType::NUMBER_LEGAL_KO;
    for (sal_Int16 i = 0; i < SAL_MAX_INT16; ++i)
    {
        OUString aID(xFormatter->getNumberingIdentifier(i));
        if (aID.isEmpty() && i > kLastKnown)
            break; // for

        switch (i)
        {
            case css::style::NumberingType::TRANSLITERATION:
                // TODO: why does this have no identifier?
            case css::style::NumberingType::NUMBER_UPPER_KO:
                // FIXME: duplicate of NUMBER_UPPER_ZH_TW
            case css::style::NumberingType::NUMBER_INDIC_DEVANAGARI:
                // FIXME: duplicate of NUMBER_EAST_ARABIC_INDIC
                break;
            default:
                if (aID.isEmpty() || !aMap.insert(std::pair(aID, i)).second)
                {
                    aFail.emplace_back(
                        "Numbering: " + OString::number(i) + " \"" + aID.toUtf8() + "\""
                        + (aID.isEmpty() ? ""_ostr
                                         : OString(" duplicate of " + OString::number(aMap[aID])))
                        + "\n");
                }
        }
    }

    if (!aFail.empty())
    {
        OString aMsg("Not unique numbering identifiers:\n"_ostr);
        for (auto const& r : aFail)
            aMsg += r;
        CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(), false);
    }
}

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testArabicZero)
{
    // 1 -> "01"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProperties = {
        comphelper::makePropertyValue(u"NumberingType"_ustr,
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(1)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for ARABIC_ZERO was missing.
    CPPUNIT_ASSERT_EQUAL(u"01"_ustr, aActual);

    // 10 -> "10"
    aProperties = {
        comphelper::makePropertyValue(u"NumberingType"_ustr,
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(10)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"10"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testArabicZero3)
{
    // 10 -> "010"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProperties = {
        comphelper::makePropertyValue(u"NumberingType"_ustr,
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO3)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(10)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for ARABIC_ZERO3 was missing.
    CPPUNIT_ASSERT_EQUAL(u"010"_ustr, aActual);

    // 100 -> "100"
    aProperties = {
        comphelper::makePropertyValue(u"NumberingType"_ustr,
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO3)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(100)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"100"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testArabicZero4)
{
    // 100 -> "0100"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProperties = {
        comphelper::makePropertyValue(u"NumberingType"_ustr,
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO4)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(100)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for ARABIC_ZERO4 was missing.
    CPPUNIT_ASSERT_EQUAL(u"0100"_ustr, aActual);

    // 1000 -> "1000"
    aProperties = {
        comphelper::makePropertyValue(u"NumberingType"_ustr,
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO4)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(1000)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"1000"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testArabicZero5)
{
    // 1000 -> "01000"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProperties = {
        comphelper::makePropertyValue(u"NumberingType"_ustr,
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO5)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(1000)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for ARABIC_ZERO5 was missing.
    CPPUNIT_ASSERT_EQUAL(u"01000"_ustr, aActual);

    // 10000 -> "10000"
    aProperties = {
        comphelper::makePropertyValue(u"NumberingType"_ustr,
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO5)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(10000)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"10000"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testKoreanCounting)
{
    // 1 -> "일"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_HANGUL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(1)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for NUMBER_HANGUL_KO was missing.
    CPPUNIT_ASSERT_EQUAL(u"\uc77c"_ustr, aActual);

    // 10 -> "십"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_HANGUL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(10)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc2ed"_ustr, aActual);

    // 100 -> "백"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_HANGUL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(100)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\ubc31"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testKoreanLegal)
{
    // 1 -> "하나"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(1)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for NUMBER_LEGAL_KO was missing.
    CPPUNIT_ASSERT_EQUAL(u"\ud558\ub098"_ustr, aActual);

    // 2 -> "둘"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(2)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\ub458"_ustr, aActual);

    // 3 -> "셋"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(3)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc14b"_ustr, aActual);

    // 4 -> "넷"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(4)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\ub137"_ustr, aActual);

    // 5 -> "다섯"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(5)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\ub2e4\uc12f"_ustr, aActual);
    // 6 -> "여섯
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(6)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc5ec\uc12f"_ustr, aActual);
    // 7 -> "일곱"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(7)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc77c\uacf1"_ustr, aActual);

    // 8 -> "여덟"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(8)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc5ec\ub35f"_ustr, aActual);

    // 9 -> "아홉"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(9)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc544\ud649"_ustr, aActual);

    // 10 -> "열"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(10)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc5f4"_ustr, aActual);

    // 21 -> "스물하나"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(21)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc2a4\ubb3c\ud558\ub098"_ustr, aActual);

    // 32 -> "서른둘"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(32)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc11c\ub978\ub458"_ustr, aActual);

    // 43 -> "마흔셋"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(43)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\ub9c8\ud754\uc14b"_ustr, aActual);

    // 54 -> "쉰넷"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(54)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc270\ub137"_ustr, aActual);

    // 65 -> "예순다섯"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(65)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc608\uc21c\ub2e4\uc12f"_ustr, aActual);

    // 76 -> "일흔여섯"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(76)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc77c\ud754\uc5ec\uc12f"_ustr, aActual);

    // 87 -> "여든일곱"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(87)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc5ec\ub4e0\uc77c\uacf1"_ustr, aActual);

    // 98 -> "아흔여덟"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(98)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc544\ud754\uc5ec\ub35f"_ustr, aActual);

    // 99 -> "아흔아홉"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr, static_cast<sal_uInt16>(style::NumberingType::NUMBER_LEGAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(99)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc544\ud754\uc544\ud649"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testKoreanDigital)
{
    // 1 -> "일"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr,
            static_cast<sal_uInt16>(style::NumberingType::NUMBER_DIGITAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(1)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for NUMBER_DIGITAL_KO was missing.
    CPPUNIT_ASSERT_EQUAL(u"\uc77c"_ustr, aActual);

    // 10 -> "일영"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr,
            static_cast<sal_uInt16>(style::NumberingType::NUMBER_DIGITAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(10)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc77c\uc601"_ustr, aActual);

    // 100 -> "일영영"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr,
            static_cast<sal_uInt16>(style::NumberingType::NUMBER_DIGITAL_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(100)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\uc77c\uc601\uc601"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testKoreanDigital2)
{
    // 1 -> "一"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr,
            static_cast<sal_uInt16>(style::NumberingType::NUMBER_DIGITAL2_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(1)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for NUMBER_DIGITAL2_KO was missing.
    CPPUNIT_ASSERT_EQUAL(u"\u4e00"_ustr, aActual);

    // 10 -> "一零"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr,
            static_cast<sal_uInt16>(style::NumberingType::NUMBER_DIGITAL2_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(10)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\u4e00\u96f6"_ustr, aActual);

    // 100 -> "一零零"
    aProperties = {
        comphelper::makePropertyValue(
            u"NumberingType"_ustr,
            static_cast<sal_uInt16>(style::NumberingType::NUMBER_DIGITAL2_KO)),
        comphelper::makePropertyValue(u"Value"_ustr, static_cast<sal_Int32>(100)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(u"\u4e00\u96f6\u96f6"_ustr, aActual);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
