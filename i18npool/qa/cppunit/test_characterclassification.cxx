/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Caolán McNamara <caolanm@redhat.com>
 *
 * Contributor(s):
 *   Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
    aLocale.Language = ::rtl::OUString("en");
    aLocale.Country = ::rtl::OUString("US");

    {
        //basic example
        ::rtl::OUString sTest("Some text");
        ::rtl::OUString sTitleCase = m_xCC->toTitle(sTest, 0, sTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be title", sTitleCase == "Some Text");
        ::rtl::OUString sUpperCase = m_xCC->toUpper(sTest, 0, sTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be upper", sUpperCase == "SOME TEXT");
        ::rtl::OUString sLowerCase = m_xCC->toLower(sTest, 0, sTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be lower ", sLowerCase == "some text");
    }

    {
        //tricky one
        const sal_Unicode LATINSMALLLETTERDZ[] = { 0x01F3 };
        ::rtl::OUString aTest(LATINSMALLLETTERDZ, SAL_N_ELEMENTS(LATINSMALLLETTERDZ));
        ::rtl::OUString sTitleCase = m_xCC->toTitle(aTest, 0, aTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be title", sTitleCase.getLength() == 1 && sTitleCase[0] == 0x01F2);
        ::rtl::OUString sUpperCase = m_xCC->toUpper(aTest, 0, aTest.getLength(), aLocale);
        CPPUNIT_ASSERT_MESSAGE("Should be upper", sUpperCase.getLength() == 1 && sUpperCase[0] == 0x01F1);
        ::rtl::OUString sLowerCase = m_xCC->toLower(aTest, 0, aTest.getLength(), aLocale);
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
