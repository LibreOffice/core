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
    void testOnlyWeak();
    void testStrongChange();
    void testMongolianAfterNNBSP();
    void testNonspacingMark();
    void testSmartQuoteCompatibilityCJ();
    void testSmartQuoteCompatibilityComplexAndCJ();
    void testSmartQuoteCJAtStart();
    void testRtlRunTrivial();
    void testRtlRunEmbeddedComplex();
    void testRtlRunEmbeddedLtrStrong();
    void testRtlRunEmbeddedLtrWeakComplex();
    void testRtlRunOverrideCJKAsian();
    void testTdf164493InfiniteLoop();
    void testHintsDefault();
    void testHintsParaOnly();
    void testHintsRunAtStart();
    void testHintsRunAfterSpace();
    void testHintsRunsAdjacent();
    void testHintsRunsSpaced();
    void testHintIgnore();
    void testHintLatin();
    void testHintAsian();
    void testHintComplex();
    void testHintMultipleRuns();

    CPPUNIT_TEST_SUITE(ScriptChangeScannerTest);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testTrivial);
    CPPUNIT_TEST(testTrivialAppLang);
    CPPUNIT_TEST(testWeakAtStart);
    CPPUNIT_TEST(testOnlyWeak);
    CPPUNIT_TEST(testStrongChange);
    CPPUNIT_TEST(testMongolianAfterNNBSP);
    CPPUNIT_TEST(testNonspacingMark);
    CPPUNIT_TEST(testSmartQuoteCompatibilityCJ);
    CPPUNIT_TEST(testSmartQuoteCompatibilityComplexAndCJ);
    CPPUNIT_TEST(testSmartQuoteCJAtStart);
    CPPUNIT_TEST(testRtlRunTrivial);
    CPPUNIT_TEST(testRtlRunEmbeddedComplex);
    CPPUNIT_TEST(testRtlRunEmbeddedLtrStrong);
    CPPUNIT_TEST(testRtlRunEmbeddedLtrWeakComplex);
    CPPUNIT_TEST(testRtlRunOverrideCJKAsian);
    CPPUNIT_TEST(testTdf164493InfiniteLoop);
    CPPUNIT_TEST(testHintsDefault);
    CPPUNIT_TEST(testHintsParaOnly);
    CPPUNIT_TEST(testHintsRunAtStart);
    CPPUNIT_TEST(testHintsRunAfterSpace);
    CPPUNIT_TEST(testHintsRunsAdjacent);
    CPPUNIT_TEST(testHintsRunsSpaced);
    CPPUNIT_TEST(testHintIgnore);
    CPPUNIT_TEST(testHintLatin);
    CPPUNIT_TEST(testHintAsian);
    CPPUNIT_TEST(testHintComplex);
    CPPUNIT_TEST(testHintMultipleRuns);
    CPPUNIT_TEST_SUITE_END();
};

void ScriptChangeScannerTest::testEmpty()
{
    auto aText = u""_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);
    CPPUNIT_ASSERT(pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nEndIndex);
}

void ScriptChangeScannerTest::testTrivial()
{
    auto aText = u"Trivial case with a single span of a script"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

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
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::ASIAN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(43), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();
    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testWeakAtStart()
{
    // The first-seen script type is used for weak characters at the start
    auto aText = u"“x”"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::COMPLEX, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testOnlyWeak()
{
    // The application language is used for text containing only weak characters
    auto aText = u"“”"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::COMPLEX, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testStrongChange()
{
    auto aText = u"wide 廣 vast"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

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
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

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
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

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
    // tdf#166104: Weak-script quotes should take the previously-seen script type

    auto aText = u"Before \u201c水\u201d After"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testSmartQuoteCompatibilityComplexAndCJ()
{
    // tdf#66791: However, if a paragraph contains complex text, weak-script
    // quotes are assigned in the usual greedy way.

    auto aText = u"Before \u201c水\u201d After \u05d0"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

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
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testRtlRunTrivial()
{
    auto aText = u"Before אאאאאא after"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(1), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(13), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(13), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(19), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(pDirScanner->AtEnd());

    pDirScanner->Reset();

    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(19), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testRtlRunEmbeddedComplex()
{
    auto aText = u"Before אא(א\"א)אא after"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(1), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(22), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(pDirScanner->AtEnd());

    pDirScanner->Reset();

    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(17), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(17), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(22), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testRtlRunEmbeddedLtrStrong()
{
    auto aText = u"אאא Inside אאא"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 1);

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(1), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(2), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(1), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(pDirScanner->AtEnd());

    pDirScanner->Reset();

    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::COMPLEX, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testRtlRunEmbeddedLtrWeakComplex()
{
    auto aText = u"אאא 123 אאא"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 1);

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(1), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(2), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(1), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(pDirScanner->AtEnd());

    pDirScanner->Reset();

    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testRtlRunOverrideCJKAsian()
{
    // tdf#163660: Asian-script characters following an RTL override should
    // still be treated as Asian script, rather than Complex script
    auto aText = u"一二\u202e三四五"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(!pDirScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(1), pDirScanner->Peek().m_nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pDirScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pDirScanner->Peek().m_nEndIndex);
    CPPUNIT_ASSERT(!pDirScanner->Peek().m_bHasEmbeddedStrongLTR);

    pDirScanner->Advance();

    CPPUNIT_ASSERT(pDirScanner->AtEnd());

    pDirScanner->Reset();

    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testTdf164493InfiniteLoop()
{
    // tdf#164493: Tests a case causing an infinite loop due to interaction
    // between right-to-left override and a CJK combining mark.

    // U+202E: RIGHT-TO-LEFT OVERRIDE
    // U+302E: HANGUL SINGLE DOT TONE MARK
    auto aText = u"\u202e\u302e"_ustr;
    ScriptHintProvider stHints;
    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testHintsDefault()
{
    ScriptHintProvider stHints;

    stHints.Start();
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Automatic, stHints.Peek());

    stHints.AdvanceTo(1);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Automatic, stHints.Peek());

    stHints.AdvanceTo(2);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Automatic, stHints.Peek());
}

void ScriptChangeScannerTest::testHintsParaOnly()
{
    ScriptHintProvider stHints;
    stHints.SetParagraphLevelHint(ScriptHintType::Ignore);

    stHints.Start();
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(1);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(2);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());
}

void ScriptChangeScannerTest::testHintsRunAtStart()
{
    ScriptHintProvider stHints;
    stHints.SetParagraphLevelHint(ScriptHintType::Ignore);
    stHints.AddHint(ScriptHintType::Latin, 0, 3);

    stHints.Start();
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(1);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(2);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(3);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(4);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());
}

void ScriptChangeScannerTest::testHintsRunAfterSpace()
{
    ScriptHintProvider stHints;
    stHints.SetParagraphLevelHint(ScriptHintType::Ignore);
    stHints.AddHint(ScriptHintType::Latin, 2, 5);

    stHints.Start();
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(1);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(2);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(3);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(4);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(5);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(6);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());
}

void ScriptChangeScannerTest::testHintsRunsAdjacent()
{
    ScriptHintProvider stHints;
    stHints.SetParagraphLevelHint(ScriptHintType::Ignore);
    stHints.AddHint(ScriptHintType::Latin, 2, 5);
    stHints.AddHint(ScriptHintType::Complex, 5, 7);

    stHints.Start();
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(1);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(2);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(3);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(4);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(5);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Complex, stHints.Peek());

    stHints.AdvanceTo(6);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Complex, stHints.Peek());

    stHints.AdvanceTo(7);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(8);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());
}

void ScriptChangeScannerTest::testHintsRunsSpaced()
{
    ScriptHintProvider stHints;
    stHints.SetParagraphLevelHint(ScriptHintType::Ignore);
    stHints.AddHint(ScriptHintType::Latin, 2, 5);
    stHints.AddHint(ScriptHintType::Complex, 7, 9);

    stHints.Start();
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(1);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(2);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(3);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(4);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Latin, stHints.Peek());

    stHints.AdvanceTo(5);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(6);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(7);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Complex, stHints.Peek());

    stHints.AdvanceTo(8);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Complex, stHints.Peek());

    stHints.AdvanceTo(9);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());

    stHints.AdvanceTo(10);
    CPPUNIT_ASSERT_EQUAL(ScriptHintType::Ignore, stHints.Peek());
}

void ScriptChangeScannerTest::testHintIgnore()
{
    auto aText = u"Before \u201c水\u201d After"_ustr;
    ScriptHintProvider stHints;
    stHints.SetParagraphLevelHint(ScriptHintType::Ignore);

    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testHintLatin()
{
    auto aText = u"水 \u201c水\u201d After"_ustr;
    ScriptHintProvider stHints;
    stHints.SetParagraphLevelHint(ScriptHintType::Latin);

    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testHintAsian()
{
    auto aText = u"Before \u201c水\u201d After"_ustr;
    ScriptHintProvider stHints;
    stHints.SetParagraphLevelHint(ScriptHintType::Asian);

    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
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

void ScriptChangeScannerTest::testHintComplex()
{
    auto aText = u"Before \u201c水\u201d After"_ustr;
    ScriptHintProvider stHints;
    stHints.SetParagraphLevelHint(ScriptHintType::Complex);

    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

void ScriptChangeScannerTest::testHintMultipleRuns()
{
    auto aText = u"““““““““““““““““"_ustr;
    ScriptHintProvider stHints;
    stHints.AddHint(ScriptHintType::Latin, 3, 5);
    stHints.AddHint(ScriptHintType::Complex, 7, 9);
    stHints.AddHint(ScriptHintType::Asian, 11, 13);

    auto pDirScanner = MakeDirectionChangeScanner(aText, 0);
    auto pScanner
        = MakeScriptChangeScanner(aText, css::i18n::ScriptType::LATIN, *pDirScanner, stHints);

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::LATIN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::COMPLEX, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(!pScanner->AtEnd());
    CPPUNIT_ASSERT_EQUAL(css::i18n::ScriptType::ASIAN, pScanner->Peek().m_nScriptType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), pScanner->Peek().m_nStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), pScanner->Peek().m_nEndIndex);

    pScanner->Advance();

    CPPUNIT_ASSERT(pScanner->AtEnd());
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScriptChangeScannerTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
