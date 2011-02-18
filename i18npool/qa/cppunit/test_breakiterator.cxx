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
#include "precompiled_i18npool.hxx"

#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>

#include <string.h>

using namespace ::com::sun::star;

class TestBreakIterator : public CppUnit::TestFixture
{
public:
    TestBreakIterator();
    ~TestBreakIterator();

    virtual void setUp();
    virtual void tearDown();

    void testLineBreaking();
    void testGraphemeIteration();

    CPPUNIT_TEST_SUITE(TestBreakIterator);
    CPPUNIT_TEST(testLineBreaking);
    CPPUNIT_TEST(testGraphemeIteration);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XMultiComponentFactory> m_xFactory;
    uno::Reference<lang::XMultiServiceFactory> m_xMSF;
    uno::Reference<i18n::XBreakIterator> m_xBreak;
};

//See https://bugs.freedesktop.org/show_bug.cgi?id=31271 for motivation
void TestBreakIterator::testLineBreaking()
{
    ::rtl::OUString aTest1(RTL_CONSTASCII_USTRINGPARAM("(some text here)"));

    i18n::LineBreakHyphenationOptions aHyphOptions;
    i18n::LineBreakUserOptions aUserOptions;
    lang::Locale aLocale;

    aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("en"));
    aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("US"));

    {
        //Here we want the line break to leave text here) on the next line
        i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest1, strlen("(some tex"), aLocale, 0, aHyphOptions, aUserOptions);
        CPPUNIT_ASSERT_MESSAGE("Expected a break at the the start of the word", aResult.breakIndex == 6);
    }

    {
        //Here we want the line break to leave "here)" on the next line
        i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest1, strlen("(some text here"), aLocale, 0, aHyphOptions, aUserOptions);
        CPPUNIT_ASSERT_MESSAGE("Expected a break at the the start of the word", aResult.breakIndex == 11);
    }
}

//See http://qa.openoffice.org/issues/show_bug.cgi?id=111152 for motivation
void TestBreakIterator::testGraphemeIteration()
{
    lang::Locale aLocale;
    aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bn"));
    aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IN"));

    {
        sal_Unicode BA_HALANT_LA[] = { 0x09AC, 0x09CD, 0x09AF };
        ::rtl::OUString aTest1(BA_HALANT_LA, SAL_N_ELEMENTS(BA_HALANT_LA));

        sal_Int32 nDone=0;
        sal_Int32 nPos;
        nPos = m_xBreak->nextCharacters(aTest1, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == SAL_N_ELEMENTS(BA_HALANT_LA));
        nPos = m_xBreak->previousCharacters(aTest1, SAL_N_ELEMENTS(BA_HALANT_LA), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == 0);
    }

    {
        sal_Unicode HA_HALANT_NA_VOWELSIGNI[] = { 0x09B9, 0x09CD, 0x09A3, 0x09BF };
        ::rtl::OUString aTest1(HA_HALANT_NA_VOWELSIGNI, SAL_N_ELEMENTS(HA_HALANT_NA_VOWELSIGNI));

        sal_Int32 nDone=0;
        sal_Int32 nPos;
        nPos = m_xBreak->nextCharacters(aTest1, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == SAL_N_ELEMENTS(HA_HALANT_NA_VOWELSIGNI));
        nPos = m_xBreak->previousCharacters(aTest1, SAL_N_ELEMENTS(HA_HALANT_NA_VOWELSIGNI), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == 0);
    }

    {
        sal_Unicode TA_HALANT_MA_HALANT_YA  [] = { 0x09A4, 0x09CD, 0x09AE, 0x09CD, 0x09AF };
        ::rtl::OUString aTest1(TA_HALANT_MA_HALANT_YA, SAL_N_ELEMENTS(TA_HALANT_MA_HALANT_YA));

        sal_Int32 nDone=0;
        sal_Int32 nPos;
        nPos = m_xBreak->nextCharacters(aTest1, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == SAL_N_ELEMENTS(TA_HALANT_MA_HALANT_YA));
        nPos = m_xBreak->previousCharacters(aTest1, SAL_N_ELEMENTS(TA_HALANT_MA_HALANT_YA), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == 0);
    }
}

TestBreakIterator::TestBreakIterator()
{
    m_xContext = cppu::defaultBootstrap_InitialComponentContext();
    m_xFactory = m_xContext->getServiceManager();
    m_xMSF = uno::Reference<lang::XMultiServiceFactory>(m_xFactory, uno::UNO_QUERY_THROW);
    m_xBreak = uno::Reference< i18n::XBreakIterator >(m_xMSF->createInstance(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.BreakIterator"))),
        uno::UNO_QUERY_THROW);
}

void TestBreakIterator::setUp()
{
}

TestBreakIterator::~TestBreakIterator()
{
}

void TestBreakIterator::tearDown()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestBreakIterator);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
