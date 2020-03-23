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

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace ::com::sun::star;

/// i18npool defaultnumberingprovider tests.
class I18npoolDefaultnumberingproviderTest : public test::BootstrapFixture
{
};

CPPUNIT_TEST_FIXTURE(I18npoolDefaultnumberingproviderTest, testArabicZero)
{
    uno::Reference<uno::XComponentContext> xComponentContext
        = comphelper::getComponentContext(getMultiServiceFactory());

    // 1 -> "01"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(xComponentContext), uno::UNO_QUERY);
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
    uno::Reference<uno::XComponentContext> xComponentContext
        = comphelper::getComponentContext(getMultiServiceFactory());

    // 10 -> "010"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(xComponentContext), uno::UNO_QUERY);
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
    uno::Reference<uno::XComponentContext> xComponentContext
        = comphelper::getComponentContext(getMultiServiceFactory());

    // 100 -> "0100"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(xComponentContext), uno::UNO_QUERY);
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
    uno::Reference<uno::XComponentContext> xComponentContext
        = comphelper::getComponentContext(getMultiServiceFactory());

    // 1000 -> "01000"
    uno::Reference<text::XNumberingFormatter> xFormatter(
        text::DefaultNumberingProvider::create(xComponentContext), uno::UNO_QUERY);
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
