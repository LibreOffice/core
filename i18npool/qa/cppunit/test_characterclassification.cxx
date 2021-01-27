/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <unotest/bootstrapfixturebase.hxx>

using namespace ::com::sun::star;

class TestCharacterClassification : public test::BootstrapFixtureBase
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void testTitleCase();
    void testStringType();

    CPPUNIT_TEST_SUITE(TestCharacterClassification);
    CPPUNIT_TEST(testTitleCase);
    CPPUNIT_TEST(testStringType);
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference<i18n::XCharacterClassification> m_xCC;
};

//A test to ensure that our Title Case functionality is working
//http://lists.freedesktop.org/archives/libreoffice/2012-June/032767.html
//https://bz.apache.org/ooo/show_bug.cgi?id=30863
void TestCharacterClassification::testTitleCase()
{
    lang::Locale aLocale;
    aLocale.Language = "en";
    aLocale.Country = "US";

    {
        //basic example
        OUString sTest("Some text");
        OUString sTitleCase = m_xCC->toTitle(sTest, 0, sTest.getLength(), aLocale);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", OUString("Some Text"), sTitleCase);
        OUString sUpperCase = m_xCC->toUpper(sTest, 0, sTest.getLength(), aLocale);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", OUString("SOME TEXT"), sUpperCase);
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), aLocale);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower ", OUString("some text"), sLowerCase);
    }

    {
        //tricky one
        static constexpr OUStringLiteral aTest = u"\u01F3"; // LATIN SMALL LETTER DZ
        OUString sTitleCase = m_xCC->toTitle(aTest, 0, aTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be title", sTitleCase.getLength() == 1 && sTitleCase[0] == 0x01F2);
        OUString sUpperCase = m_xCC->toUpper(aTest, 0, aTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be upper", sUpperCase.getLength() == 1 && sUpperCase[0] == 0x01F1);
        OUString sLowerCase = m_xCC->toLower(aTest, 0, aTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be lower ", sLowerCase.getLength() == 1 && sLowerCase[0] == 0x01F3);
    }
}

//https://bugs.libreoffice.org/show_bug.cgi?id=69641
void TestCharacterClassification::testStringType()
{
    lang::Locale aLocale;
    aLocale.Language = "en";
    aLocale.Country = "US";

    {
        //simple case
        OUString sTest("Some text");
        sal_Int32 nResult = m_xCC->getStringType(sTest, 0, sTest.getLength(), aLocale);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(230), nResult);
    }

    {
        //tricky case
        static constexpr OUStringLiteral sTest = u"\U0001D703"; // MATHEMATICAL ITALIC SMALL THETA
        sal_Int32 nResult = m_xCC->getStringType(sTest, 0, sTest.getLength(), aLocale);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(228), nResult);
    }

}

void TestCharacterClassification::setUp()
{
    BootstrapFixtureBase::setUp();
    m_xCC.set(m_xSFactory->createInstance("com.sun.star.i18n.CharacterClassification"), uno::UNO_QUERY_THROW);
}

void TestCharacterClassification::tearDown()
{
    BootstrapFixtureBase::tearDown();
    m_xCC.clear();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestCharacterClassification);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
