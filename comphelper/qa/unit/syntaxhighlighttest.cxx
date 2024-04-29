/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/syntaxhighlight.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <rtl/ustring.hxx>

#include <vector>

class SyntaxHighlightTest : public CppUnit::TestFixture
{
public:
    void testBasicString();
    void testBasicComment();
    void testBasicCommentNewline();
    void testBasicEmptyComment();
    void testBasicEmptyCommentNewline();
    void testBasic();

    CPPUNIT_TEST_SUITE(SyntaxHighlightTest);
    CPPUNIT_TEST(testBasicString);
    CPPUNIT_TEST(testBasicComment);
    CPPUNIT_TEST(testBasicCommentNewline);
    CPPUNIT_TEST(testBasicEmptyComment);
    CPPUNIT_TEST(testBasicEmptyCommentNewline);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST_SUITE_END();
};

void SyntaxHighlightTest::testBasicString() {
    std::vector<HighlightPortion> ps;
    SyntaxHighlighter(HighlighterLanguage::Basic).getHighlightPortions(u"\"foo\"", ps);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<std::vector<HighlightPortion>::size_type>(1), ps.size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), ps[0].nBegin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), ps[0].nEnd);
    CPPUNIT_ASSERT_EQUAL(TokenType::String, ps[0].tokenType);
}

void SyntaxHighlightTest::testBasicComment() {
    std::vector<HighlightPortion> ps;
    SyntaxHighlighter(HighlighterLanguage::Basic).getHighlightPortions(u"' foo", ps);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<std::vector<HighlightPortion>::size_type>(1), ps.size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), ps[0].nBegin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), ps[0].nEnd);
    CPPUNIT_ASSERT_EQUAL(TokenType::Comment, ps[0].tokenType);
}

void SyntaxHighlightTest::testBasicCommentNewline() {
    std::vector<HighlightPortion> ps;
    SyntaxHighlighter(HighlighterLanguage::Basic).getHighlightPortions(u"' foo\n", ps);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<std::vector<HighlightPortion>::size_type>(2), ps.size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), ps[0].nBegin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), ps[0].nEnd);
    CPPUNIT_ASSERT_EQUAL(TokenType::Comment, ps[0].tokenType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), ps[1].nBegin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), ps[1].nEnd);
    CPPUNIT_ASSERT_EQUAL(TokenType::EOL, ps[1].tokenType);
}

void SyntaxHighlightTest::testBasicEmptyComment() {
    std::vector<HighlightPortion> ps;
    SyntaxHighlighter(HighlighterLanguage::Basic).getHighlightPortions(u"'", ps);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<std::vector<HighlightPortion>::size_type>(1), ps.size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), ps[0].nBegin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ps[0].nEnd);
    CPPUNIT_ASSERT_EQUAL(TokenType::Comment, ps[0].tokenType);
}

void SyntaxHighlightTest::testBasicEmptyCommentNewline() {
    std::vector<HighlightPortion> ps;
    SyntaxHighlighter(HighlighterLanguage::Basic).getHighlightPortions(u"'\n", ps);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<std::vector<HighlightPortion>::size_type>(2), ps.size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), ps[0].nBegin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ps[0].nEnd);
    CPPUNIT_ASSERT_EQUAL(TokenType::Comment, ps[0].tokenType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ps[1].nBegin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), ps[1].nEnd);
    CPPUNIT_ASSERT_EQUAL(TokenType::EOL, ps[1].tokenType);
}

void SyntaxHighlightTest::testBasic()
{
    OUString aBasicString(u"        if Mid(sText,iRun,1 )<> \" \" then Mid( sText ,iRun, 1, Chr( 1 + Asc( Mid(sText,iRun,1 )) ) '"_ustr);

    std::vector<HighlightPortion> aPortions;
    SyntaxHighlighter(HighlighterLanguage::Basic).getHighlightPortions(
        aBasicString, aPortions );

    sal_Int32 prevEnd = 0;
    for (auto const& portion : aPortions)
    {
        CPPUNIT_ASSERT_EQUAL(prevEnd, portion.nBegin);
        CPPUNIT_ASSERT(portion.nBegin < portion.nEnd);
        prevEnd = portion.nEnd;
    }
    CPPUNIT_ASSERT_EQUAL(aBasicString.getLength(), prevEnd);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SyntaxHighlightTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
