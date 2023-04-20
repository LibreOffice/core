/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <svx/svdview.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <anchoredobject.hxx>
#include <flyfrms.hxx>
#include <formatflysplit.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <sortedobjs.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>
#include <fmtfsize.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <itabenum.hxx>
#include <frmmgr.hxx>
#include <frameformats.hxx>
#include <cellfrm.hxx>
#include <ndtxt.hxx>
#include <dflyobj.hxx>

namespace
{
/// Covers sw/source/core/layout/flycnt.cxx fixes, i.e. mostly SwFlyAtContentFrame.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/layout/data/")
    {
    }

    /// Creates a document with a multi-page floating table: 1 columns and 2 rows.
    void Create1x2SplitFly();
};

void Test::Create1x2SplitFly()
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwPageDesc aStandard(pDoc->GetPageDesc(0));
    SwFormatFrameSize aPageSize(aStandard.GetMaster().GetFrameSize());
    // 5cm for the page height, 2cm are the top and bottom margins, so 1cm remains for the body
    // frame:
    aPageSize.SetHeight(2834);
    aStandard.GetMaster().SetFormatAttr(aPageSize);
    pDoc->ChgPageDesc(0, aStandard);
    // Insert a table:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    pWrtShell->GoPrevCell();
    pWrtShell->Insert("A1");
    pWrtShell->GoNextCell();
    pWrtShell->Insert("A2");
    // Select cell:
    pWrtShell->SelAll();
    // Select table:
    pWrtShell->SelAll();
    // Wrap the table in a text frame:
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    pWrtShell->StartAllAction();
    aMgr.InsertFlyFrame(RndStdIds::FLY_AT_PARA, aMgr.GetPos(), aMgr.GetSize());
    pWrtShell->EndAllAction();
    // Allow the text frame to split:
    pWrtShell->StartAllAction();
    SwFrameFormats& rFlys = *pDoc->GetSpzFrameFormats();
    SwFrameFormat* pFly = rFlys[0];
    SwAttrSet aSet(pFly->GetAttrSet());
    aSet.Put(SwFormatFlySplit(true));
    pDoc->SetAttr(aSet, *pFly);
    pWrtShell->EndAllAction();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    // We have 2 pages:
    CPPUNIT_ASSERT(pPage1->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyWithTable)
{
    // Given a document with a multi-page floating table:
    createSwDoc("floattable.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the first row goes to page 1 and the second row goes to page 2, while the
    // table is floating:
    SwDoc* pDoc = getSwDoc();
    // Without the accompanying fix in place, this test would have failed with a stack overflow
    // because the follow frame of the anchor was moved into the follow frame of the fly, so the fly
    // was anchored in itself.
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    // Page 1 has a master fly, which contains a master table:
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    CPPUNIT_ASSERT(!pPage1Fly->GetPrecede());
    CPPUNIT_ASSERT(pPage1Fly->GetFollow());
    auto pPage1Table = dynamic_cast<SwTabFrame*>(pPage1Fly->GetLower());
    CPPUNIT_ASSERT(pPage1Table);
    CPPUNIT_ASSERT(!pPage1Table->GetPrecede());
    CPPUNIT_ASSERT(pPage1Table->GetFollow());
    // Page 2 has a follow fly, which contains a follow table:
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    CPPUNIT_ASSERT(pPage2Fly);
    CPPUNIT_ASSERT(pPage2Fly->GetPrecede());
    CPPUNIT_ASSERT(!pPage2Fly->GetFollow());
    auto pPage2Table = dynamic_cast<SwTabFrame*>(pPage2Fly->GetLower());
    CPPUNIT_ASSERT(pPage2Table);
    CPPUNIT_ASSERT(pPage2Table->GetPrecede());
    CPPUNIT_ASSERT(!pPage2Table->GetFollow());
    // Page 1 anchor has no text:
    auto pPage1Anchor = dynamic_cast<SwTextFrame*>(pPage1->FindLastBodyContent());
    CPPUNIT_ASSERT(pPage1Anchor);
    // This failed, page 1 anchor had unexpected, leftover text.
    CPPUNIT_ASSERT(!pPage1Anchor->HasPara());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyVertOffset)
{
    // Given a document with a floattable, split on 2 pages and a positive vertical offset:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-vertoffset.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the vert offset has an effect on the master fly, but not on follow flys:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    auto pPage1Anchor = dynamic_cast<SwTextFrame*>(pPage1->FindLastBodyContent());
    CPPUNIT_ASSERT(pPage1Anchor);
    SwTwips nPage1AnchorTop = pPage1Anchor->getFrameArea().Top();
    SwTwips nPage1FlyTop = pPage1Fly->getFrameArea().Top();
    // First page, the vert offset should be there. This comes from word/document.xml:
    // <w:tblpPr ... w:tblpY="1135"/>
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(1135), nPage1FlyTop - nPage1AnchorTop);

    // Also verify that the 2nd page has no such offset:
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    CPPUNIT_ASSERT(pPage2Fly);
    auto pPage2Anchor = dynamic_cast<SwTextFrame*>(pPage2->FindLastBodyContent());
    CPPUNIT_ASSERT(pPage2Anchor);
    SwTwips nPage2AnchorTop = pPage2Anchor->getFrameArea().Top();
    SwTwips nPage2FlyTop = pPage2Fly->getFrameArea().Top();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1135
    // i.e. the fly frame on the 2nd page was also shifted down in Writer, but not in Word.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(0), nPage2FlyTop - nPage2AnchorTop);
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFly3Pages)
{
    // Given a document with a floattable, split on 3 pages:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-3pages.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that row 1, 2 & 3 go to page 1, 2 & 3, while the table is floating:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    auto pPage1Anchor = dynamic_cast<SwTextFrame*>(pPage1->FindLastBodyContent());
    CPPUNIT_ASSERT(pPage1Anchor);
    SwTwips nPage1AnchorTop = pPage1Anchor->getFrameArea().Top();
    SwTwips nPage1FlyTop = pPage1Fly->getFrameArea().Top();
    // The vert offset should be there on the first page:
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(1135), nPage1FlyTop - nPage1AnchorTop);
    // Second page:
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. both the 2nd and 3rd fly was anchored on page 2.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    CPPUNIT_ASSERT(pPage2Fly);
    auto pPage2Anchor = dynamic_cast<SwTextFrame*>(pPage2->FindLastBodyContent());
    CPPUNIT_ASSERT(pPage2Anchor);
    SwTwips nPage2AnchorTop = pPage2Anchor->getFrameArea().Top();
    SwTwips nPage2FlyTop = pPage2Fly->getFrameArea().Top();
    // No vert offset on the second page:
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(0), nPage2FlyTop - nPage2AnchorTop);
    // 3rd page:
    auto pPage3 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage3);
    const SwSortedObjs& rPage3Objs = *pPage3->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage3Objs.size());
    auto pPage3Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage3Objs[0]);
    CPPUNIT_ASSERT(pPage3Fly);
    auto pPage3Anchor = dynamic_cast<SwTextFrame*>(pPage3->FindLastBodyContent());
    CPPUNIT_ASSERT(pPage3Anchor);
    SwTwips nPage3AnchorTop = pPage3Anchor->getFrameArea().Top();
    SwTwips nPage3FlyTop = pPage3Fly->getFrameArea().Top();
    // No vert offset on the 3rd page:
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(0), nPage3FlyTop - nPage3AnchorTop);
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyRow)
{
    // Given a document with a floattable, single row split on 2 pages:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-rowsplit.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the single row is split to 2 pages, and the fly frames have the correct
    // coordinates:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    auto pPage1Anchor = dynamic_cast<SwTextFrame*>(pPage1->FindLastBodyContent());
    CPPUNIT_ASSERT(pPage1Anchor);
    // ~No offset between the fly and its anchor:
    SwTwips nPage1AnchorTop = pPage1Anchor->getFrameArea().Top();
    SwTwips nPage1FlyTop = pPage1Fly->getFrameArea().Top();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(1), nPage1FlyTop - nPage1AnchorTop);
    // Second page:
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    CPPUNIT_ASSERT(pPage2Fly);
    auto pPage2Anchor = dynamic_cast<SwTextFrame*>(pPage2->FindLastBodyContent());
    CPPUNIT_ASSERT(pPage2Anchor);
    // No offset between the fly and its anchor:
    SwTwips nPage2AnchorTop = pPage2Anchor->getFrameArea().Top();
    SwTwips nPage2FlyTop = pPage2Fly->getFrameArea().Top();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : -1440
    // i.e. the 2nd page's fly had a wrong position.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(0), nPage2FlyTop - nPage2AnchorTop);
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyEnable)
{
    // Given a document with a table in a textframe:
    Create1x2SplitFly();

    // Then make sure that the layout is updated and we have 2 pages:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    // Without the accompanying fix in place, this test would have failed, there was no 2nd page.
    CPPUNIT_ASSERT(pPage2);
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyFooter)
{
    // Given a document with a floattable, table split on 2 pages with headers/footers:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-footer.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the table is split to 2 pages:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    SwTwips nPage1Top = pPage1->getFrameArea().Top();
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    SwTwips nPage1FlyTop = pPage1Fly->getFrameArea().Top();
    // <w:tblpPr w:tblpY="3286"> from the bugdoc.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(3286), nPage1FlyTop - nPage1Top);
    // Second page:
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    // Without the accompanying fix in place, this test would have failed, there was no 2nd page.
    CPPUNIT_ASSERT(pPage2);
    SwTwips nPage2Top = pPage2->getFrameArea().Top();
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    CPPUNIT_ASSERT(pPage2Fly);
    SwTwips nPage2FlyTop = pPage2Fly->getFrameArea().Top();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1440
    // - Actual  : 0
    // i.e. <w:pgMar w:top="1440"> from the bugdoc was lost, the follow fly had no vertical offset.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(1440), nPage2FlyTop - nPage2Top);
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyFooter2Rows)
{
    // Given a document with a 2nd page that contains the second half of a split row + a last row:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-footer-2rows.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the table is split to 2 pages:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    // Without the accompanying fix in place, this test would have failed. The 2nd page only had the
    // 2nd half of the split row and the last row went to a 3rd page.
    CPPUNIT_ASSERT(!pPage2->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFly2Cols)
{
    // Given a document with a 2nd page that contains the second half of a split row and 2 columns:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-2cols.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the table is split to 2 pages:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    // Without the accompanying fix in place, this test would have failed. The 2nd page only had the
    // 2nd half of the split row and the very last row went to a 3rd page.
    CPPUNIT_ASSERT(!pPage2->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyWidow)
{
    // Given a document with a 2nd page that contains 2 lines, due to widow control:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-widow.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that widow control works:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    SwFrame* pTab1 = pPage1Fly->GetLower();
    SwFrame* pRow1 = pTab1->GetLower();
    SwFrame* pCell1 = pRow1->GetLower();
    auto pText1 = dynamic_cast<SwTextFrame*>(pCell1->GetLower());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6
    // - Actual  : 7
    // i.e. widow control was disabled, layout didn't match Word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(6), pText1->GetThisLines());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    CPPUNIT_ASSERT(pPage2Fly);
    SwFrame* pTab2 = pPage2Fly->GetLower();
    SwFrame* pRow2 = pTab2->GetLower();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1014
    // - Actual  : 553
    // i.e. <w:trHeight w:val="1014"> from the file was ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(1014), pRow2->getFrameArea().Height());
    SwFrame* pCell2 = pRow2->GetLower();
    auto pText2 = dynamic_cast<SwTextFrame*>(pCell2->GetLower());
    // And then similarly this was 1, not 2.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(2), pText2->GetThisLines());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyCompat14)
{
    // Given a Word 2010 document with 2 pages, one table row each:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-compat14.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the first row is entirely on page 1:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    SwFrame* pTab1 = pPage1Fly->GetLower();
    SwFrame* pRow1 = pTab1->GetLower();
    auto pCell1 = dynamic_cast<SwCellFrame*>(pRow1->GetLower());
    // Without the accompanying fix in place, this test would have failed, the first row was split,
    // but not in Word.
    CPPUNIT_ASSERT(!pCell1->GetFollowCell());
    // Also make sure that the second row is entirely on page 2:
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    CPPUNIT_ASSERT(pPage2Fly);
    SwFrame* pTab2 = pPage2Fly->GetLower();
    SwFrame* pRow2 = pTab2->GetLower();
    auto pCell2 = dynamic_cast<SwCellFrame*>(pRow2->GetLower());
    // Without the accompanying fix in place, this test would have failed, the second row was split,
    // but not in Word.
    CPPUNIT_ASSERT(!pCell2->GetPreviousCell());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyCompat14Nosplit)
{
    // Given a Word 2010 document with 2 pages, 2 rows on page 1, 1 row on page 2:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-compat14-nosplit.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the last row is on a separate page:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    SwFrame* pTab1 = pPage1Fly->GetLower();
    SwFrame* pRow1 = pTab1->GetLower();
    CPPUNIT_ASSERT(pRow1->GetNext());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    // Without the accompanying fix in place, this test would have failed, all rows were on page 1.
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    CPPUNIT_ASSERT(pPage2Fly);
    SwFrame* pTab2 = pPage2Fly->GetLower();
    SwFrame* pRow2 = pTab2->GetLower();
    CPPUNIT_ASSERT(!pRow2->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyCompat14Body)
{
    // Given a Word 2010 document with 2 pages, 1 row on page 1, 1 row on page 2:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-compat14-body.docx");

    // When laying out that document:
    // (This is legacy mode, but still Word doesn't split row 2 because 1) row 2 has a minimal
    // height, so even the first part of row 2 would not fit the body frame and 2) Word allows using
    // the bottom margin area in legacy mode, but only in case the fly height <= body height.)
    calcLayout();

    // Then make sure that the second row is on a page 2:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    SwFrame* pTab1 = pPage1Fly->GetLower();
    SwFrame* pRow1 = pTab1->GetLower();
    // Without the accompanying fix in place, this test would have failed, part of row 2 was on page
    // 1.
    CPPUNIT_ASSERT(!pRow1->GetNext());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    CPPUNIT_ASSERT(pPage2Fly);
    SwFrame* pTab2 = pPage2Fly->GetLower();
    SwFrame* pRow2 = pTab2->GetLower();
    CPPUNIT_ASSERT(!pRow2->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyFollowHorizontalPosition)
{
    // Given a document with 2 pages, master fly on page 1, follow fly on page 2:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-hori-pos.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the follow fly doesn't have a different horizontal position:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    tools::Long nPage1FlyLeft = pPage1Fly->getFrameArea().Left();
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    CPPUNIT_ASSERT(pPage2Fly);
    tools::Long nPage2FlyLeft = pPage2Fly->getFrameArea().Left();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5528
    // - Actual  : 284
    // i.e. the follow fly was pushed towards the left, instead of having the same position as the
    // master fly.
    CPPUNIT_ASSERT_EQUAL(nPage1FlyLeft, nPage2FlyLeft);
}

CPPUNIT_TEST_FIXTURE(Test, testCursorTraversal)
{
    // Given a document with a multi-page floating table:
    Create1x2SplitFly();

    // When going from A1 to A2:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GotoTable("Table1");
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(OUString("A1"), pTextNode->GetText());
    pWrtShell->Down(/*bSelect=*/false);

    // Then make sure we get to A2 and don't stay in A1:
    pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: A2
    // - Actual  : A1
    // i.e. the cursor didn't get from A1 to A2.
    CPPUNIT_ASSERT_EQUAL(OUString("A2"), pTextNode->GetText());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyRowDelete)
{
    // Given a document with a multi-page floating table:
    Create1x2SplitFly();

    // When deleting the row of A2:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GotoTable("Table1");
    pWrtShell->Down(/*bSelect=*/false);
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // We delete the right row:
    CPPUNIT_ASSERT_EQUAL(OUString("A2"), pTextNode->GetText());
    pWrtShell->DeleteRow();

    // Then make sure we only have 1 page:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(!pPage1->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFly1stRowDelete)
{
    // Given a document with a multi-page floating table:
    Create1x2SplitFly();

    // When deleting the row of A1:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GotoTable("Table1");
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // We delete the right row:
    CPPUNIT_ASSERT_EQUAL(OUString("A1"), pTextNode->GetText());
    pWrtShell->DeleteRow();

    // Then make sure we only have 1 page:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    // Without the accompanying fix in place, this test would have failed, the follow fly was still
    // on page 2.
    CPPUNIT_ASSERT(!pPage1->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFly3rdRowDelete)
{
    // Given a document with a floattable, split on 3 pages:
    SwModelTestBase::FlySplitGuard aGuard;
    createSwDoc("floattable-3pages.docx");

    // When deleting the row of A3:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GotoTable("Table1");
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // We delete the right row:
    CPPUNIT_ASSERT_EQUAL(OUString("A3"), pTextNode->GetText());
    pWrtShell->DeleteRow();

    // Then make sure we only have 2 pages:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    // Without the accompanying fix in place, this test would have failed, page 3 was not deleted.
    CPPUNIT_ASSERT(!pPage2->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFly2ndRowSelect)
{
    // Given a document with a multi-page floating table:
    createSwDoc("floattable.docx");

    // When selecting the second row:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    const SwRect& aFollowArea = pPage2Fly->getFrameArea();
    Point aTopCenter((aFollowArea.Left() + aFollowArea.Right()) / 2, aFollowArea.Top());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SelectObj(aTopCenter);

    // Then make sure the first row is selected:
    const SdrMarkList& rMarkList = pWrtShell->GetDrawView()->GetMarkedObjectList();
    SdrObject* pSelectedObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
    auto pSelectedVirtObj = dynamic_cast<SwVirtFlyDrawObj*>(pSelectedObj);
    auto pSelected = static_cast<SwFlyAtContentFrame*>(pSelectedVirtObj->GetFlyFrame());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5
    // - Actual  : 17
    // i.e. a follow fly was possible to select (instead of its master)
    CPPUNIT_ASSERT_EQUAL(pPage2Fly->GetPrecede()->GetFrameId(), pSelected->GetFrameId());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
