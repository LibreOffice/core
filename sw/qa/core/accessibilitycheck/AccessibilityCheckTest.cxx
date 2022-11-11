/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <AccessibilityCheck.hxx>

class AccessibilityCheckTest : public SwModelTestBase
{
public:
    AccessibilityCheckTest()
        : SwModelTestBase("/sw/qa/core/accessibilitycheck/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckDocumentIssues)
{
    createSwDoc("DocumentTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::DOCUMENT_LANGUAGE, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::DOCUMENT_TITLE, aIssues[1]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testTableSplitMergeAndAltText)
{
    createSwDoc("AccessibilityTests1.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(7), aIssues.size());

    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::NO_ALT_GRAPHIC, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::NO_ALT_OLE, aIssues[1]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT, aIssues[2]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT, aIssues[3]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT, aIssues[4]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT, aIssues[5]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::NO_ALT_SHAPE, aIssues[6]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckParagraphIssues)
{
    // Tests whether formatting issues are detected when the whole paragraph has them instead of
    // some text inside the paragraph
    createSwDoc("ParagraphTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[0]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckBackgroundImage)
{
    createSwDoc("BackgroundImageTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::DOCUMENT_BACKGROUND, aIssues[0]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckNewlineSpace)
{
    createSwDoc("NewlineTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[1]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckSpacebarSpace)
{
    createSwDoc("SpaceTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[0]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testHyperlinks)
{
    createSwDoc("HyperlinkTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::HYPERLINK_SHORT, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::HYPERLINK_IS_TEXT, aIssues[1]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckHighlightedText)
{
    createSwDoc("HighlightTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[0]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testNumberingCheck)
{
    createSwDoc("AccessibilityTests_NumberingCheck.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(5), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::MANUAL_NUMBERING, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::MANUAL_NUMBERING, aIssues[1]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::MANUAL_NUMBERING, aIssues[2]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::MANUAL_NUMBERING, aIssues[3]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::MANUAL_NUMBERING, aIssues[4]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckFakeFootnote)
{
    createSwDoc("FakeFootnoteTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::FAKE_FOOTNOTE, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::FAKE_FOOTNOTE, aIssues[1]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckFakeCaption)
{
    createSwDoc("FakeCaptionTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::FAKE_CAPTION, aIssues[0]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckTableFormatting)
{
    createSwDoc("TableFormattingTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TABLE_FORMATTING, aIssues[0]->m_eIssueID);
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckTabsFormatting)
{
    createSwDoc("TabsTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(4), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[1]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[2]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[3]->m_eIssueID);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
