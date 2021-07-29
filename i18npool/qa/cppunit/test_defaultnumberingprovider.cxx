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
            case css::style::NumberingType::NUMBER_DIGITAL_KO:
                // FIXME: duplicate of NUMBER_HANGUL_KO
            case css::style::NumberingType::NUMBER_DIGITAL2_KO:
                // FIXME: duplicate of NUMBER_LOWER_ZH
                break;
            default:
                if (aID.isEmpty() || !aMap.insert(std::pair(aID, i)).second)
                {
                    aFail.emplace_back(
                        "Numbering: " + OString::number(i) + " \"" + aID.toUtf8() + "\""
                        + (aID.isEmpty() ? ""
                                         : OString(" duplicate of " + OString::number(aMap[aID])))
                        + "\n");
                }
        }
    }

    if (!aFail.empty())
    {
        OString aMsg("Not unique numbering identifiers:\n");
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
        comphelper::makePropertyValue("NumberingType",
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO)),
        comphelper::makePropertyValue("Value", static_cast<sal_Int32>(1)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for ARABIC_ZERO was missing.
    CPPUNIT_ASSERT_EQUAL(OUString("01"), aActual);

    // 10 -> "10"
    aProperties = {
        comphelper::makePropertyValue("NumberingType",
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO)),
        comphelper::makePropertyValue("Value", static_cast<sal_Int32>(10)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(OUString("10"), aActual);
}

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testArabicZero3)
{
    // 10 -> "010"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProperties = {
        comphelper::makePropertyValue("NumberingType",
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO3)),
        comphelper::makePropertyValue("Value", static_cast<sal_Int32>(10)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for ARABIC_ZERO3 was missing.
    CPPUNIT_ASSERT_EQUAL(OUString("010"), aActual);

    // 100 -> "100"
    aProperties = {
        comphelper::makePropertyValue("NumberingType",
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO3)),
        comphelper::makePropertyValue("Value", static_cast<sal_Int32>(100)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(OUString("100"), aActual);
}

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testArabicZero4)
{
    // 100 -> "0100"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProperties = {
        comphelper::makePropertyValue("NumberingType",
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO4)),
        comphelper::makePropertyValue("Value", static_cast<sal_Int32>(100)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for ARABIC_ZERO4 was missing.
    CPPUNIT_ASSERT_EQUAL(OUString("0100"), aActual);

    // 1000 -> "1000"
    aProperties = {
        comphelper::makePropertyValue("NumberingType",
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO4)),
        comphelper::makePropertyValue("Value", static_cast<sal_Int32>(1000)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aActual);
}

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testArabicZero5)
{
    // 1000 -> "01000"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(mxComponentContext), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProperties = {
        comphelper::makePropertyValue("NumberingType",
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO5)),
        comphelper::makePropertyValue("Value", static_cast<sal_Int32>(1000)),
    };
    lang::Locale aLocale;
    OUString aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    // Without the accompanying fix in place, this test would have failed with a
    // lang.IllegalArgumentException, support for ARABIC_ZERO5 was missing.
    CPPUNIT_ASSERT_EQUAL(OUString("01000"), aActual);

    // 10000 -> "10000"
    aProperties = {
        comphelper::makePropertyValue("NumberingType",
                                      static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO5)),
        comphelper::makePropertyValue("Value", static_cast<sal_Int32>(10000)),
    };
    aActual = xFormatter->makeNumberingString(aProperties, aLocale);
    CPPUNIT_ASSERT_EQUAL(OUString("10000"), aActual);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
