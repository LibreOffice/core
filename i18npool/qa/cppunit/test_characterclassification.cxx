/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <o3tl/cppunittraitshelper.hxx>
#include <unotest/bootstrapfixturebase.hxx>

using namespace ::com::sun::star;

class TestCharacterClassification : public test::BootstrapFixtureBase
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

protected:
    uno::Reference<i18n::XCharacterClassification> m_xCC;
};

//A test to ensure that our Title Case functionality is working
//http://lists.freedesktop.org/archives/libreoffice/2012-June/032767.html
//https://bz.apache.org/ooo/show_bug.cgi?id=30863
CPPUNIT_TEST_FIXTURE(TestCharacterClassification, testTitleCase)
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
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", sal_Int32(1), sTitleCase.getLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", u'\u01F2', sTitleCase[0]);
        OUString sUpperCase = m_xCC->toUpper(aTest, 0, aTest.getLength(), aLocale);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", sal_Int32(1), sUpperCase.getLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", u'\u01F1', sUpperCase[0]);
        OUString sLowerCase = m_xCC->toLower(aTest, 0, aTest.getLength(), aLocale);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower ", sal_Int32(1), sLowerCase.getLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower ", u'\u01F3', sLowerCase[0]);
    }
}

//https://bugs.libreoffice.org/show_bug.cgi?id=69641
CPPUNIT_TEST_FIXTURE(TestCharacterClassification, testStringType)
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

CPPUNIT_TEST_FIXTURE(TestCharacterClassification, testSigma)
{
    {
        // From upper case
        OUString sTest(u"ὈΔΥΣΣΕΎΣ");
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", OUString(u"ὀδυσσεύς"), sLowerCase);
        OUString sUpperCase = m_xCC->toUpper(sLowerCase, 0, sLowerCase.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", sTest, sUpperCase);
    }

    {
        // From lower case
        OUString sTest(u"ὀδυσσεύς");
        OUString sTitleCase = m_xCC->toTitle(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", OUString(u"Ὀδυσσεύς"), sTitleCase);
        OUString sUpperCase = m_xCC->toUpper(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", OUString(u"ὈΔΥΣΣΕΎΣ"), sUpperCase);
        OUString sLowerCase = m_xCC->toLower(sUpperCase, 0, sUpperCase.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", sTest, sLowerCase);
    }

    {
        // From title case
        OUString sTest(u"Ὀδυσσεύς");
        OUString sTitleCase = m_xCC->toTitle(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", OUString(u"Ὀδυσσεύς"), sTitleCase);
        OUString sUpperCase = m_xCC->toUpper(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", OUString(u"ὈΔΥΣΣΕΎΣ"), sUpperCase);
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", OUString(u"ὀδυσσεύς"), sLowerCase);
    }
}

CPPUNIT_TEST_FIXTURE(TestCharacterClassification, testTdf96343)
{
    {
        // From upper case
        OUString sTest(u"ꙊꙌꙖ");
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", OUString(u"ꙋꙍꙗ"), sLowerCase);
        OUString sUpperCase = m_xCC->toUpper(sLowerCase, 0, sLowerCase.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", sTest, sUpperCase);
    }

    {
        // From lower case
        OUString sTest(u"ꙋꙍꙗ");
        OUString sTitleCase = m_xCC->toTitle(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", OUString(u"Ꙋꙍꙗ"), sTitleCase);
        OUString sUpperCase = m_xCC->toUpper(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", OUString(u"ꙊꙌꙖ"), sUpperCase);
        OUString sLowerCase = m_xCC->toLower(sUpperCase, 0, sUpperCase.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", sTest, sLowerCase);
    }

    {
        // From title case
        OUString sTest(u"Ꙋꙍꙗ");
        OUString sTitleCase = m_xCC->toTitle(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", OUString(u"Ꙋꙍꙗ"), sTitleCase);
        OUString sUpperCase = m_xCC->toUpper(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", OUString(u"ꙊꙌꙖ"), sUpperCase);
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", OUString(u"ꙋꙍꙗ"), sLowerCase);
    }
}

CPPUNIT_TEST_FIXTURE(TestCharacterClassification, testTdf134766)
{
    {
        // From upper case
        OUString sTest(u"QꞋORBꞋAL");
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", OUString(u"qꞌorbꞌal"), sLowerCase);
        OUString sUpperCase = m_xCC->toUpper(sLowerCase, 0, sLowerCase.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", sTest, sUpperCase);
    }

    {
        // From lower case
        OUString sTest(u"qꞌorbꞌal");
        OUString sTitleCase = m_xCC->toTitle(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", OUString(u"Qꞌorbꞌal"), sTitleCase);
        OUString sUpperCase = m_xCC->toUpper(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", OUString(u"QꞋORBꞋAL"), sUpperCase);
        OUString sLowerCase = m_xCC->toLower(sUpperCase, 0, sUpperCase.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", sTest, sLowerCase);
    }

    {
        // From title case
        OUString sTest(u"Qꞌorbꞌal");
        OUString sTitleCase = m_xCC->toTitle(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", OUString(u"Qꞌorbꞌal"), sTitleCase);
        OUString sUpperCase = m_xCC->toUpper(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", OUString(u"QꞋORBꞋAL"), sUpperCase);
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", OUString(u"qꞌorbꞌal"), sLowerCase);
    }
}

CPPUNIT_TEST_FIXTURE(TestCharacterClassification, testTdf97152)
{
    {
        // From upper case
        OUString sTest(u"ͲͰϽϾϿͿϏϹ");
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", OUString(u"ͳͱͻͼͽϳϗϲ"), sLowerCase);
        OUString sUpperCase = m_xCC->toUpper(sLowerCase, 0, sLowerCase.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", sTest, sUpperCase);
    }

    {
        // From lower case
        OUString sTest(u"ͳͱͻͼͽϳϗϲ");
        OUString sTitleCase = m_xCC->toTitle(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", OUString(u"Ͳͱͻͼͽϳϗϲ"), sTitleCase);
        OUString sUpperCase = m_xCC->toUpper(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", OUString(u"ͲͰϽϾϿͿϏϹ"), sUpperCase);
        OUString sLowerCase = m_xCC->toLower(sUpperCase, 0, sUpperCase.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", sTest, sLowerCase);
    }

    {
        // From title case
        OUString sTest(u"Ͳͱͻͼͽϳϗϲ");
        OUString sTitleCase = m_xCC->toTitle(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", OUString(u"Ͳͱͻͼͽϳϗϲ"), sTitleCase);
        OUString sUpperCase = m_xCC->toUpper(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", OUString(u"ͲͰϽϾϿͿϏϹ"), sUpperCase);
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", OUString(u"ͳͱͻͼͽϳϗϲ"), sLowerCase);
    }
}

CPPUNIT_TEST_FIXTURE(TestCharacterClassification, testSurrogatePairs)
{
    {
        // No case mapping
        OUString sTest(u"\U0001F600");
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", OUString(u"\U0001F600"), sLowerCase);
        OUString sUpperCase = m_xCC->toUpper(sLowerCase, 0, sLowerCase.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", sTest, sUpperCase);
    }

    {
        // Case mapping
        OUString sTest(u"\U00010400");
        OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", OUString(u"\U00010428"), sLowerCase);
        OUString sUpperCase = m_xCC->toUpper(sLowerCase, 0, sLowerCase.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", sTest, sUpperCase);
    }
}

CPPUNIT_TEST_FIXTURE(TestCharacterClassification, testAdlam)
{
    OUString sUpper(u"𞤀𞤁𞤂𞤃𞤄𞤅𞤆𞤇𞤈𞤉𞤊𞤋𞤌𞤍𞤎𞤏𞤐𞤑𞤒𞤓𞤔𞤕𞤖𞤗𞤘𞤙𞤚𞤛𞤜𞤝𞤞𞤟𞤠𞤡");
    OUString sLower(u"𞤢𞤣𞤤𞤥𞤦𞤧𞤨𞤩𞤪𞤫𞤬𞤭𞤮𞤯𞤰𞤱𞤲𞤳𞤴𞤵𞤶𞤷𞤸𞤹𞤺𞤻𞤼𞤽𞤾𞤿𞥀𞥁𞥂𞥃");
    OUString sTitle = sLower; // Adlam doesn’t have title case?
    {
        // From upper case
        OUString sLowerRes = m_xCC->toLower(sUpper, 0, sUpper.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", sLower, sLowerRes);
        OUString sUpperRes = m_xCC->toUpper(sLowerRes, 0, sLower.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", sUpper, sUpperRes);
    }

    {
        // From lower case
        OUString sTitleRes = m_xCC->toTitle(sLower, 0, sLower.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", sTitle, sTitleRes);
        OUString sUpperRes = m_xCC->toUpper(sLower, 0, sLower.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", sUpper, sUpperRes);
        OUString sLowerRes = m_xCC->toLower(sUpperRes, 0, sUpperRes.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", sLower, sLowerRes);
    }

    {
        // From title case
        OUString sTitleRes = m_xCC->toTitle(sTitle, 0, sTitle.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be title", sTitle, sTitleRes);
        OUString sUpperRes = m_xCC->toUpper(sTitle, 0, sTitle.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be upper", sUpper, sUpperRes);
        OUString sLowerRes = m_xCC->toLower(sTitle, 0, sTitle.getLength(), {});
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be lower", sLower, sLowerRes);
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
