/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <i18nutil/scriptchangescanner.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>

using namespace i18nutil;
namespace css = ::com::sun::star;

namespace
{
class ScriptChangeScannerTest : public CppUnit::TestFixture
{
public:
    void testEmpty();
    void testTrivial();
    void testTrivialAppLang();
    void testWeakAtStart();
    void testStrongChange();
    void testMongolianAfterNNBSP();
    void testNonspacingMark();
    void testSmartQuoteCompatibilityCJ();
    void testSmartQuoteCompatibilityComplexAndCJ();
    void testSmartQuoteCJAtStart();

    CPPUNIT_TEST_SUITE(ScriptChangeScannerTest);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testTrivial);
    CPPUNIT_TEST(testTrivialAppLang);
    CPPUNIT_TEST(testWeakAtStart);
    CPPUNIT_TEST(testStrongChange);
    CPPUNIT_TEST(testMongolianAfterNNBSP);
    CPPUNIT_TEST(testNonspacingMark);
    CPPUNIT_TEST(testSmartQuoteCompatibilityCJ);
    CPPUNIT_TEST(testSmartQuoteCompatibilityComplexAndCJ);
    CPPUNIT_TEST(testSmartQuoteCJAtStart);
    CPPUNIT_TEST_SUITE_END();
};

void ScriptChangeScannerTest::testEmpty()
{
    auto aText = u""_ustr;
    auto pScanner = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN);
    CPPUNIT_ASSERT(pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nEndIndex);
}

void ScriptChangeScannerTest::testTrivial()
{
    auto aText = u"Trivial case with a single span of a script"_ustr;
    auto pScanner = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(43), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();
    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testTrivialAppLang()
{
    auto aText = u"Trivial case with a single span of a script"_ustr;
    auto pScanner = MakeScriptChangeScanner(aText, css::i18n::ScriptType::ASIAN);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(43), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();
    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testWeakAtStart()
{
    auto aText = u"“x”"_ustr;
    auto pScanner = MakeScriptChangeScanner(aText, css::i18n::ScriptType::COMPLEX);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testStrongChange()
{
    auto aText = u"wide 廣 vast"_ustr;
    auto pScanner = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testMongolianAfterNNBSP()
{
    // NNBSP before Mongolian text should be part of the Mongolian run
    auto aText = u"Before\u202f\u1822\u1822After"_ustr;
    auto pScanner = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testNonspacingMark()
{
    // A preceding weak character should be included in the run
    // of a following non-spacing mark
    auto aText = u"Before \u0944\u0911\u0911 After"_ustr;
    auto pScanner = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testSmartQuoteCompatibilityCJ()
{
    // tdf#66791: For compatibility with other programs, weak-script quotes in paragraphs
    // containing CJ characters should be treated as Asian script

    auto aText = u"Before \u201c水\u201d After"_ustr;
    auto pScanner = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testSmartQuoteCompatibilityComplexAndCJ()
{
    // tdf#66791: However, if a paragraph contains complex text, weak-script
    // quotes are assigned in the usual greedy way.

    auto aText = u"Before \u201c水\u201d After \u05d0"_ustr;
    auto pScanner = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(17), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(17), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testSmartQuoteCJAtStart()
{
    auto aText = u"“廣”"_ustr;
    auto pScanner = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScriptChangeScannerTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
