/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <unotest/bootstrapfixturebase.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <string.h>

using namespace ::com::sun::star;

class TestCharacterClassification : public test::BootstrapFixtureBase
{
public:
    virtual void setUp();
    virtual void tearDown();

    void testTitleCase();

    CPPUNIT_TEST_SUITE(TestCharacterClassification);
    CPPUNIT_TEST(testTitleCase);
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference<i18n::XCharacterClassification> m_xCC;
};

//A test to ensure that our Title Case functionality is working
//http://lists.freedesktop.org/archives/libreoffice/2012-June/032767.html
//https://issues.apache.org/ooo/show_bug.cgi?id=30863
void TestCharacterClassification::testTitleCase()
{
    lang::Locale aLocale;
    aLocale.Language = OUString("en");
    aLocale.Country = OUString("US");

    {
        //basic example
        OUString sTest("Some text");
        OUString sTitleCase = m_xCC->toTitle(sTest, 0, sTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be title", sTitleCase == "Some Text");
        OUString sUpperCase = m_xCC->toUpper(sTest, 0, sTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be upper", sUpperCase == "SOME TEXT");
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be lower ", sLowerCase == "some text");
    }

    {
        //tricky one
        const sal_Unicode LATINSMALLLETTERDZ[] = { 0x01F3 };
        OUString aTest(LATINSMALLLETTERDZ, SAL_N_ELEMENTS(LATINSMALLLETTERDZ));
        OUString sTitleCase = m_xCC->toTitle(aTest, 0, aTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be title", sTitleCase.getLength() == 1 && sTitleCase[0] == 0x01F2);
        OUString sUpperCase = m_xCC->toUpper(aTest, 0, aTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be upper", sUpperCase.getLength() == 1 && sUpperCase[0] == 0x01F1);
        OUString sLowerCase = m_xCC->toLower(aTest, 0, aTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be lower ", sLowerCase.getLength() == 1 && sLowerCase[0] == 0x01F3);
    }
}

void TestCharacterClassification::setUp()
{
    BootstrapFixtureBase::setUp();
    m_xCC = uno::Reference< i18n::XCharacterClassification >(m_xSFactory->createInstance(
        "com.sun.star.i18n.CharacterClassification"), uno::UNO_QUERY_THROW);
}

void TestCharacterClassification::tearDown()
{
    BootstrapFixtureBase::tearDown();
    m_xCC.clear();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestCharacterClassification);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
