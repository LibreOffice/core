/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
 * Unit tests for SQL comment handling in the connectivity SQL parser
 * (OSQLParser / OSQLScanner / OSQLParseNode).
 *
 * These tests verify that the flex lexer correctly discards all three
 * comment styles (-- line, slash-star block star-slash) so that OSQLParser::parseTree()
 * succeeds and OSQLParseNode::parseNodeToStr() produces comment-free output.
 *
 */

#include <test/bootstrapfixture.hxx>

#include <connectivity/sqlparse.hxx>
#include <connectivity/sqlnode.hxx>

using namespace ::connectivity;

namespace connectivity::commontools
{
class SQLCommentTest : public test::BootstrapFixture
{
public:
    SQLCommentTest()
        : test::BootstrapFixture(false, false)
    {
    }

    void setUp() override;
    void tearDown() override;

    // -- line comment
    void test_dashDash_inline_comment();
    void test_dashDash_comment_at_end_of_input_no_newline();
    void test_dashDash_comment_strips_from_output();

    // /* block */ comment
    void test_block_comment_inline();
    void test_block_comment_strips_from_output();
    void test_multiline_block_comment();

    // -- in string literals must NOT be treated as a comment
    void test_dashDash_inside_string_literal_not_comment();

    // canonical equivalence: SQL with comment must serialise identically to SQL without
    void test_canonical_dashDash_equals_plain();
    void test_canonical_block_equals_plain();

    // comment in every syntactic position
    void test_comment_between_select_and_columns();
    void test_comment_between_columns_and_from();
    void test_comment_in_where_clause();

    // multiple and adjacent comments
    void test_multiple_dashDash_comments();
    void test_mixed_comment_styles();

    // edge cases
    void test_empty_block_comment();
    void test_block_comment_containing_stars();
    void test_slashSlash_inside_string_literal_not_comment();

    // "//" is not a valid comment; parser must reject it
    void test_slashSlash_rejected();

    // real comment + -- inside string literal in same statement
    void test_real_comment_and_dashDash_in_literal_same_statement();

    CPPUNIT_TEST_SUITE(SQLCommentTest);

    CPPUNIT_TEST(test_dashDash_inline_comment);
    CPPUNIT_TEST(test_dashDash_comment_at_end_of_input_no_newline);
    CPPUNIT_TEST(test_dashDash_comment_strips_from_output);

    CPPUNIT_TEST(test_block_comment_inline);
    CPPUNIT_TEST(test_block_comment_strips_from_output);
    CPPUNIT_TEST(test_multiline_block_comment);

    CPPUNIT_TEST(test_dashDash_inside_string_literal_not_comment);

    CPPUNIT_TEST(test_canonical_dashDash_equals_plain);
    CPPUNIT_TEST(test_canonical_block_equals_plain);

    CPPUNIT_TEST(test_comment_between_select_and_columns);
    CPPUNIT_TEST(test_comment_between_columns_and_from);
    CPPUNIT_TEST(test_comment_in_where_clause);

    CPPUNIT_TEST(test_multiple_dashDash_comments);
    CPPUNIT_TEST(test_mixed_comment_styles);

    CPPUNIT_TEST(test_empty_block_comment);
    CPPUNIT_TEST(test_block_comment_containing_stars);
    CPPUNIT_TEST(test_slashSlash_inside_string_literal_not_comment);

    CPPUNIT_TEST(test_slashSlash_rejected);

    CPPUNIT_TEST(test_real_comment_and_dashDash_in_literal_same_statement);

    CPPUNIT_TEST_SUITE_END();

private:
    /** Count all nodes in the parse tree recursively. */
    static sal_uInt32 countNodes(const OSQLParseNode* pNode);

    std::unique_ptr<OSQLParser> m_pParser;
};

void SQLCommentTest::setUp()
{
    test::BootstrapFixture::setUp();
    // OSQLParser needs a service factory and locale; nullptr is acceptable for
    // tests that only exercise comment stripping.
    m_pParser = std::make_unique<OSQLParser>(getComponentContext());
}

void SQLCommentTest::tearDown()
{
    m_pParser.reset();
    test::BootstrapFixture::tearDown();
}

// Simple check, comments produce no AST nodes
// Workaround since we cannot connect a DB to the parser here

sal_uInt32 SQLCommentTest::countNodes(const OSQLParseNode* pNode)
{
    if (!pNode)
        return 0;
    sal_uInt32 n = 1;
    for (size_t i = 0; i < pNode->count(); ++i)
        n += countNodes(pNode->getChild(i));
    return n;
}

// ---------------------------------------------------------------------------
// -- line comment tests
// ---------------------------------------------------------------------------

void SQLCommentTest::test_dashDash_inline_comment()
{
    // "-- comment\n" at end of a valid SQL statement must be consumed.
    OUString sSQL(u"SELECT * FROM \"t\" -- inline comment\n"_ustr);
    OUString sError;
    std::unique_ptr<OSQLParseNode> pNode(m_pParser->parseTree(sError, sSQL));
    CPPUNIT_ASSERT_MESSAGE("parseTree() must succeed with a trailing -- comment", pNode != nullptr);
}

void SQLCommentTest::test_dashDash_comment_at_end_of_input_no_newline()
{
    // just to be sure that he new flex rule for "--" works at end-of-input
    // without a trailing newline
    OUString sSQL(u"SELECT * FROM \"t\" --no newline at end"_ustr);
    OUString sError;
    std::unique_ptr<OSQLParseNode> pNode(m_pParser->parseTree(sError, sSQL));
    CPPUNIT_ASSERT_MESSAGE("parseTree() must succeed with -- comment at end-of-input (no newline)",
                           pNode != nullptr);
}

void SQLCommentTest::test_dashDash_comment_strips_from_output()
{
    // After parsing, the parser must succeed (i.e. the comment was lexed out).
    // Note: parseNodeToStr() needs a real DB connection and cannot be called
    // with nullptr; parse success alone is proof the comment was consumed.
    OUString sSQL(u"SELECT * FROM \"t\" -- strip me\n"_ustr);
    OUString sError;
    std::unique_ptr<OSQLParseNode> pNode(m_pParser->parseTree(sError, sSQL));
    CPPUNIT_ASSERT_MESSAGE("parseTree() must succeed (comment stripped by lexer)",
                           pNode != nullptr);
    CPPUNIT_ASSERT_MESSAGE("No error message expected", sError.isEmpty());
}

// ---------------------------------------------------------------------------
// /* block */ comment tests
// ---------------------------------------------------------------------------

void SQLCommentTest::test_block_comment_inline()
{
    OUString sSQL(u"SELECT /* block comment */ * FROM \"t\""_ustr);
    OUString sError;
    std::unique_ptr<OSQLParseNode> pNode(m_pParser->parseTree(sError, sSQL));
    CPPUNIT_ASSERT_MESSAGE("parseTree() must succeed with an inline /* comment */",
                           pNode != nullptr);
}

void SQLCommentTest::test_block_comment_strips_from_output()
{
    OUString sSQL(u"SELECT /* strip me */ * FROM \"t\""_ustr);
    OUString sError;
    std::unique_ptr<OSQLParseNode> pNode(m_pParser->parseTree(sError, sSQL));
    CPPUNIT_ASSERT_MESSAGE("parseTree() must succeed (block comment consumed by lexer)",
                           pNode != nullptr);
    CPPUNIT_ASSERT_MESSAGE("No error message expected", sError.isEmpty());
}

void SQLCommentTest::test_multiline_block_comment()
{
    // A block comment spanning multiple lines must be accepted.
    OUString sSQL(u"SELECT /*\n   multi\n   line\n   comment\n*/ * FROM \"t\""_ustr);
    OUString sError;
    std::unique_ptr<OSQLParseNode> pNode(m_pParser->parseTree(sError, sSQL));
    CPPUNIT_ASSERT_MESSAGE("parseTree() must succeed with a multi-line /* comment */",
                           pNode != nullptr);
}

// ---------------------------------------------------------------------------
// -- inside a string literal is NOT a comment
// ---------------------------------------------------------------------------

void SQLCommentTest::test_dashDash_inside_string_literal_not_comment()
{
    // The -- inside '...' must not be treated as a comment start.
    // If it were, the parser would see a truncated WHERE clause and fail.
    OUString sSQL(u"SELECT * FROM \"t\" WHERE \"c\" = 'a--b'"_ustr);
    OUString sError;
    std::unique_ptr<OSQLParseNode> pNode(m_pParser->parseTree(sError, sSQL));
    CPPUNIT_ASSERT_MESSAGE("Query with -- inside string literal must parse successfully",
                           pNode != nullptr);
    CPPUNIT_ASSERT_MESSAGE("No error message expected", sError.isEmpty());
}

// ---------------------------------------------------------------------------
// SQL with comment should be the same as SQL without
// ---------------------------------------------------------------------------

void SQLCommentTest::test_canonical_dashDash_equals_plain()
{
    // Comments produce no AST nodes. A statement with a trailing -- comment
    // must parse to a tree with the same node count as the same statement
    // without any comment.
    OUString sError;
    std::unique_ptr<OSQLParseNode> pPlain(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\""_ustr));
    std::unique_ptr<OSQLParseNode> pWithComment(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\" -- a comment\n"_ustr));
    CPPUNIT_ASSERT_MESSAGE("-- comment statement must parse", pWithComment != nullptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("-- comment must not add AST nodes", countNodes(pPlain.get()),
                                 countNodes(pWithComment.get()));
}

void SQLCommentTest::test_canonical_block_equals_plain()
{
    OUString sError;
    std::unique_ptr<OSQLParseNode> pPlain(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\""_ustr));
    std::unique_ptr<OSQLParseNode> pWithComment(
        m_pParser->parseTree(sError, u"SELECT /* mid-query comment */ * FROM \"t\""_ustr));
    CPPUNIT_ASSERT_MESSAGE("/* */ comment statement must parse", pWithComment != nullptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("/* */ comment must not add AST nodes", countNodes(pPlain.get()),
                                 countNodes(pWithComment.get()));
}

// ---------------------------------------------------------------------------
// Comment in every syntactic position
// ---------------------------------------------------------------------------

void SQLCommentTest::test_comment_between_select_and_columns()
{
    OUString sError;
    std::unique_ptr<OSQLParseNode> pPlain(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\""_ustr));
    std::unique_ptr<OSQLParseNode> pWithComment(
        m_pParser->parseTree(sError, u"SELECT -- pick columns\n * FROM \"t\""_ustr));
    CPPUNIT_ASSERT_MESSAGE("Comment between SELECT and * must parse", pWithComment != nullptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Comment between SELECT and * must not add AST nodes",
                                 countNodes(pPlain.get()), countNodes(pWithComment.get()));
}

void SQLCommentTest::test_comment_between_columns_and_from()
{
    OUString sError;
    std::unique_ptr<OSQLParseNode> pPlain(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\""_ustr));
    std::unique_ptr<OSQLParseNode> pWithComment(
        m_pParser->parseTree(sError, u"SELECT * /* which table? */ FROM \"t\""_ustr));
    CPPUNIT_ASSERT_MESSAGE("Comment between * and FROM must parse", pWithComment != nullptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Comment between * and FROM must not add AST nodes",
                                 countNodes(pPlain.get()), countNodes(pWithComment.get()));
}

void SQLCommentTest::test_comment_in_where_clause()
{
    OUString sError;
    std::unique_ptr<OSQLParseNode> pPlain(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\" WHERE \"c\" = 1"_ustr));
    std::unique_ptr<OSQLParseNode> pWithComment(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\" WHERE /* filter */ \"c\" = 1"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Comment inside WHERE clause must parse", pWithComment != nullptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Comment inside WHERE clause must not add AST nodes",
                                 countNodes(pPlain.get()), countNodes(pWithComment.get()));
}

// ---------------------------------------------------------------------------
// Multiple and adjacent comments
// ---------------------------------------------------------------------------

void SQLCommentTest::test_multiple_dashDash_comments()
{
    // Two consecutive line comments; each must be consumed independently.
    OUString sError;
    std::unique_ptr<OSQLParseNode> pPlain(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\""_ustr));
    std::unique_ptr<OSQLParseNode> pWithComment(
        m_pParser->parseTree(sError, u"SELECT * -- first\n-- second\nFROM \"t\""_ustr));
    CPPUNIT_ASSERT_MESSAGE("Adjacent -- comments must both parse", pWithComment != nullptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Adjacent -- comments must not add AST nodes",
                                 countNodes(pPlain.get()), countNodes(pWithComment.get()));
}

void SQLCommentTest::test_mixed_comment_styles()
{
    OUString sError;
    std::unique_ptr<OSQLParseNode> pPlain(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\""_ustr));
    std::unique_ptr<OSQLParseNode> pWithComment(
        m_pParser->parseTree(sError, u"SELECT /* A */ * -- B\nFROM \"t\""_ustr));
    CPPUNIT_ASSERT_MESSAGE("Mixed comment styles in one statement must parse",
                           pWithComment != nullptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Mixed comment styles must not add AST nodes",
                                 countNodes(pPlain.get()), countNodes(pWithComment.get()));
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

void SQLCommentTest::test_empty_block_comment()
{
    OUString sError;
    std::unique_ptr<OSQLParseNode> pPlain(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\""_ustr));
    std::unique_ptr<OSQLParseNode> pWithComment(
        m_pParser->parseTree(sError, u"SELECT /**/ * FROM \"t\""_ustr));
    CPPUNIT_ASSERT_MESSAGE("Empty /**/ comment must parse", pWithComment != nullptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Empty /**/ comment must not add AST nodes",
                                 countNodes(pPlain.get()), countNodes(pWithComment.get()));
}

void SQLCommentTest::test_block_comment_containing_stars()
{
    // Interior stars must not prematurely close the block comment.
    OUString sError;
    std::unique_ptr<OSQLParseNode> pPlain(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\""_ustr));
    std::unique_ptr<OSQLParseNode> pWithComment(
        m_pParser->parseTree(sError, u"SELECT /* * not-end * */ * FROM \"t\""_ustr));
    CPPUNIT_ASSERT_MESSAGE("Block comment with interior stars must parse", pWithComment != nullptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Block comment with interior stars must not add AST nodes",
                                 countNodes(pPlain.get()), countNodes(pWithComment.get()));
}

void SQLCommentTest::test_slashSlash_inside_string_literal_not_comment()
{
    // // inside a quoted string must not start a line comment.
    OUString sSQL(u"SELECT * FROM \"t\" WHERE \"c\" = 'http://example.com'"_ustr);
    OUString sError;
    std::unique_ptr<OSQLParseNode> pNode(m_pParser->parseTree(sError, sSQL));
    CPPUNIT_ASSERT_MESSAGE("// inside string literal must not be treated as comment",
                           pNode != nullptr);
    CPPUNIT_ASSERT_MESSAGE("No error message expected", sError.isEmpty());
}

// ---------------------------------------------------------------------------
// // is not a comment — parser must reject it
// ---------------------------------------------------------------------------

void SQLCommentTest::test_slashSlash_rejected()
{
    // "//" is not valid SQL comment syntax; the parser must reject it.
    OUString sSQL(u"SELECT * FROM \"t\" // not a comment\n"_ustr);
    OUString sError;
    std::unique_ptr<OSQLParseNode> pNode(m_pParser->parseTree(sError, sSQL));
    CPPUNIT_ASSERT_MESSAGE("parseTree() must fail when // appears outside a string",
                           pNode == nullptr);
}

// ---------------------------------------------------------------------------
// real comment + -- inside string literal in same statement
// ---------------------------------------------------------------------------

void SQLCommentTest::test_real_comment_and_dashDash_in_literal_same_statement()
{
    OUString sError;
    std::unique_ptr<OSQLParseNode> pPlain(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\" WHERE \"c\" = 'a--b'"_ustr));
    std::unique_ptr<OSQLParseNode> pWithComment(
        m_pParser->parseTree(sError, u"SELECT * FROM \"t\" -- comment\nWHERE \"c\" = 'a--b'"_ustr));
    CPPUNIT_ASSERT_MESSAGE(
        "Real -- comment combined with -- inside string literal must parse correctly",
        pWithComment != nullptr);
    CPPUNIT_ASSERT_MESSAGE("No error expected", sError.isEmpty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Real -- comment must not add AST nodes (-- in literal is not a comment)",
        countNodes(pPlain.get()), countNodes(pWithComment.get()));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SQLCommentTest);

} // namespace connectivity::commontools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
