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
#include <AccessibilityIssue.hxx>
#include <OnlineAccessibilityCheck.hxx>
#include <wrtsh.hxx>
#include <vcl/scheduler.hxx>
#include <comphelper/propertysequence.hxx>

#include <comphelper/scopeguard.hxx>
#include <officecfg/Office/Common.hxx>

class AccessibilityCheckTest : public SwModelTestBase
{
public:
    AccessibilityCheckTest()
        : SwModelTestBase(u"/sw/qa/core/accessibilitycheck/data/"_ustr)
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
    CPPUNIT_ASSERT_EQUAL(size_t(6), aIssues.size());

    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::NO_ALT_GRAPHIC, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT, aIssues[1]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT, aIssues[2]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT, aIssues[3]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT, aIssues[4]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::NO_ALT_SHAPE, aIssues[5]->m_eIssueID);
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

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckLinkedImage)
{
    createSwDoc("LinkedImageTest.fodt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(4), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::LINKED_GRAPHIC, aIssues[1]->m_eIssueID);
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

//tdf#156550 - Accessibility sidebar complains about TOC hyperlinks
CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckTOCHyperlink)
{
    createSwDoc("TOCHyperlinkTest.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    // Without the fix in place, this test would have failed with
    // - Expected: 0 (No Detected HYPERLINK_IS_TEXT Issues)
    // - Actual  : 4 (Number of TOC Hyperlinks)
    CPPUNIT_ASSERT_EQUAL(size_t(0), aIssues.size());
}

namespace
{
std::vector<std::shared_ptr<sfx::AccessibilityIssue>>
scanAccessibilityIssuesOnNodes(SwDoc* pDocument)
{
    std::vector<std::shared_ptr<sfx::AccessibilityIssue>> aIssues;
    auto const& pNodes = pDocument->GetNodes();
    for (SwNodeOffset n(0); n < pNodes.Count(); ++n)
    {
        SwNode* pNode = pNodes[n];
        auto& pCollection = pNode->getAccessibilityCheckStatus().pCollection;
        if (pCollection)
        {
            for (auto& pIssue : pCollection->getIssues())
            {
                aIssues.push_back(pIssue);
            }
        }
    }
    return aIssues;
}

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheckTabsinTOC)
{
    createSwDoc("Tabs-in-TOC.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollection().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(0), aIssues.size());
}

void checkIssuePosition(std::shared_ptr<sfx::AccessibilityIssue> const& pIssue, int nLine,
                        sal_Int32 nStart, sal_Int32 nEnd, SwNodeOffset nIndex)
{
    auto* pSwIssue = static_cast<sw::AccessibilityIssue*>(pIssue.get());

    OString sFailMessage = OString::Concat("Start doesn't match at line: ")
                           + OString::Concat(OString::number(nLine));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailMessage.getStr(), nStart, pSwIssue->getStart());

    sFailMessage
        = OString::Concat("End doesn't match at line: ") + OString::Concat(OString::number(nLine));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailMessage.getStr(), nEnd, pSwIssue->getEnd());

    sFailMessage = OString::Concat("Offset doesn't match at line: ")
                   + OString::Concat(OString::number(nLine));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailMessage.getStr(), nIndex, pSwIssue->getNode()->GetIndex());
}

} // end anonymous ns

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testOnlineNodeSplitAppend)
{
    // Checks the a11y checker is setting the a11y issues to the nodes
    // correctly when splitting and appending nodes (through undo), which
    // happen on editing all the time.
    // When a node is split, it can happen that both nodes get a11y issues
    // if the node splits the area of direct formatting.

    createSwDoc("OnlineCheck.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Enable online a11y checker
    {
        auto pBatch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::set(true, pBatch);
        pBatch->commit();
    }
    comphelper::ScopeGuard g([] {
        auto pBatch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::set(false, pBatch);
        pBatch->commit();
    });

    Scheduler::ProcessEventsToIdle();

    // Check we have 2 a11y issue
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                         pDoc->getOnlineAccessibilityCheck()->getNumberOfAccessibilityIssues());
    auto aIssues = scanAccessibilityIssuesOnNodes(pDoc);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[1]->m_eIssueID);
    checkIssuePosition(aIssues[0], __LINE__, 0, 32, SwNodeOffset(9));
    checkIssuePosition(aIssues[1], __LINE__, 33, 136, SwNodeOffset(9));

    // Position the cursor and hit "enter" (trigger split-node action)
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 33, /*bBasicCall=*/false);
    pWrtShell->SplitNode();

    // Check the result
    CPPUNIT_ASSERT_EQUAL(u"He heard quiet steps behind him. "_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"That didn't bode well. Who could be following him this late at "
                         "night and in this deadbeat part of town?"_ustr,
                         getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                         pDoc->getOnlineAccessibilityCheck()->getNumberOfAccessibilityIssues());

    aIssues = scanAccessibilityIssuesOnNodes(pDoc);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[1]->m_eIssueID);
    checkIssuePosition(aIssues[0], __LINE__, 0, 32, SwNodeOffset(9));
    checkIssuePosition(aIssues[1], __LINE__, 0, 103, SwNodeOffset(10));

    // Position cursor and split again
    pWrtShell->Down(/*bSelect*/ false, /*nCount*/ 0);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 23, /*bBasicCall=*/false);
    pWrtShell->SplitNode();

    // Check the result
    CPPUNIT_ASSERT_EQUAL(u"He heard quiet steps behind him. "_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"That didn't bode well. "_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(
        u"Who could be following him this late at night and in this deadbeat part of town?"_ustr,
        getParagraph(3)->getString());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         pDoc->getOnlineAccessibilityCheck()->getNumberOfAccessibilityIssues());
    aIssues = scanAccessibilityIssuesOnNodes(pDoc);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[1]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[2]->m_eIssueID);
    checkIssuePosition(aIssues[0], __LINE__, 0, 32, SwNodeOffset(9));
    checkIssuePosition(aIssues[1], __LINE__, 0, 23, SwNodeOffset(10));
    checkIssuePosition(aIssues[2], __LINE__, 0, 80, SwNodeOffset(11));

    // Undo second change
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"He heard quiet steps behind him. "_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"That didn't bode well. Who could be following him this late at "
                         "night and in this deadbeat part of town?"_ustr,
                         getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                         pDoc->getOnlineAccessibilityCheck()->getNumberOfAccessibilityIssues());
    aIssues = scanAccessibilityIssuesOnNodes(pDoc);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[1]->m_eIssueID);
    checkIssuePosition(aIssues[0], __LINE__, 0, 32, SwNodeOffset(9));
    checkIssuePosition(aIssues[1], __LINE__, 0, 103, SwNodeOffset(10));

    // Undo first change
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                         pDoc->getOnlineAccessibilityCheck()->getNumberOfAccessibilityIssues());
    CPPUNIT_ASSERT_EQUAL(
        u"He heard quiet steps behind him. That didn't bode well. Who could be following "
        "him this late at night and in this deadbeat part of town?"_ustr,
        getParagraph(1)->getString());
    aIssues = scanAccessibilityIssuesOnNodes(pDoc);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(sfx::AccessibilityIssueID::TEXT_FORMATTING, aIssues[1]->m_eIssueID);
    checkIssuePosition(aIssues[0], __LINE__, 0, 32, SwNodeOffset(9));
    checkIssuePosition(aIssues[1], __LINE__, 33, 136, SwNodeOffset(9));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
