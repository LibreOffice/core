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
#include <comphelper/propertyvalue.hxx>

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
#include <IDocumentSettingAccess.hxx>
#include <formatwraptextatflystart.hxx>

namespace
{
/// Covers sw/source/core/layout/flycnt.cxx fixes, i.e. mostly SwFlyAtContentFrame.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/layout/data/"_ustr)
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
    // 10 cm for the page height, 2cm are the top and bottom margins, so 6cm remains for the body
    // frame:
    aPageSize.SetHeight(5669);
    aStandard.GetMaster().SetFormatAttr(aPageSize);
    pDoc->ChgPageDesc(0, aStandard);
    // Insert a table:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    pWrtShell->GoPrevCell();
    pWrtShell->Insert(u"A1"_ustr);
    SwFormatFrameSize aRowSize(SwFrameSize::Minimum);
    // 4 cm, so 2 rows don't fit 1 page.
    aRowSize.SetHeight(2267);
    pWrtShell->SetRowHeight(aRowSize);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"A2"_ustr);
    pWrtShell->SetRowHeight(aRowSize);
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
    auto& rFlys = *pDoc->GetSpzFrameFormats();
    auto pFly = rFlys[0];
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

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyDisable)
{
    // Given a document with a floating table, table split on 2 pages:
    Create1x2SplitFly();
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetNext());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    CPPUNIT_ASSERT(pPage1Fly->GetFollow());

    // When turning the "split fly" checkbox off:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->StartAllAction();
    auto& rFlys = *pDoc->GetSpzFrameFormats();
    auto pFly = rFlys[0];
    SwAttrSet aSet(pFly->GetAttrSet());
    // Note how the UI puts a SwFormatFrameSize into this item set with a slightly different size
    // (e.g. 3823 twips width -> 3821). This means that by accident the UI works even without the
    // explicit RES_FLY_SPLIT handling in SwFlyFrame::UpdateAttr_(), but this test will fail when
    // that is missing.
    aSet.Put(SwFormatFlySplit(false));
    pDoc->SetAttr(aSet, *pFly);
    pWrtShell->EndAllAction();

    // Then make sure the extra page and follow fly frame is joined:
    CPPUNIT_ASSERT(!pPage1->GetNext());
    // Without the accompanying fix in place, this test would have failed, the follow fly frame was
    // moved to page 1, but it wasn't deleted.
    CPPUNIT_ASSERT(!pPage1Fly->GetFollow());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyFooter)
{
    // Given a document with a floattable, table split on 2 pages with headers/footers:
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
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6), pText1->GetThisLines());
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
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), pText2->GetThisLines());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyCompat14)
{
    // Given a Word 2010 document with 2 pages, one table row each:
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
    pWrtShell->GotoTable(u"Table1"_ustr);
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(u"A1"_ustr, pTextNode->GetText());
    pWrtShell->Down(/*bSelect=*/false);

    // Then make sure we get to A2 and don't stay in A1:
    pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: A2
    // - Actual  : A1
    // i.e. the cursor didn't get from A1 to A2.
    CPPUNIT_ASSERT_EQUAL(u"A2"_ustr, pTextNode->GetText());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyRowDelete)
{
    // Given a document with a multi-page floating table:
    Create1x2SplitFly();

    // When deleting the row of A2:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GotoTable(u"Table1"_ustr);
    pWrtShell->Down(/*bSelect=*/false);
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // We delete the right row:
    CPPUNIT_ASSERT_EQUAL(u"A2"_ustr, pTextNode->GetText());
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
    pWrtShell->GotoTable(u"Table1"_ustr);
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // We delete the right row:
    CPPUNIT_ASSERT_EQUAL(u"A1"_ustr, pTextNode->GetText());
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
    createSwDoc("floattable-3pages.docx");

    // When deleting the row of A3:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GotoTable(u"Table1"_ustr);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // We delete the right row:
    CPPUNIT_ASSERT_EQUAL(u"A3"_ustr, pTextNode->GetText());
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

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyInSection)
{
    // This crashed, the layout assumed that the floating table is directly under the body frame.
    createSwDoc("floattable-in-section.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyThenTable)
{
    // Given a document with a 2 page floating table, followed by an other table:
    // Intentionally load the document as hidden to avoid layout during load (see TestTdf150616):
    uno::Sequence<beans::PropertyValue> aFilterOptions = {
        comphelper::makePropertyValue(u"Hidden"_ustr, true),
    };
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(u"/sw/qa/core/layout/data/")
                                      + "floattable-then-table.docx",
                                  u"com.sun.star.text.TextDocument"_ustr, aFilterOptions);

    // When layout is calculated during PDF export:
    // Then make sure that finishes without errors:
    // This crashed, due to a stack overflow in layout code.
    save(u"writer_pdf_Export"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyInTextSection)
{
    // The document contains a DOCX cont sect break, which is mapped to a TextSection.
    // This crashed, the anchor was split directly, so the follow anchor was moved outside the
    // section frame, which is broken.
    createSwDoc("floattable-in-text-section.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyTableRowKeep)
{
    // Given a document with a floating table, 2.5 rows on the first page:
    createSwDoc("floattable-table-row-keep.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the expected amount of rows is on the first page:
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
    CPPUNIT_ASSERT(pRow1);
    SwFrame* pRow2 = pRow1->GetNext();
    // Without the accompanying fix in place, this test would have failed, the table on the first
    // page only had 1 row, due to TableRowKeep kicking in for floating tables, which is incorrect.
    CPPUNIT_ASSERT(pRow2);
    SwFrame* pRow3 = pRow2->GetNext();
    CPPUNIT_ASSERT(pRow3);
    auto pCell3 = dynamic_cast<SwCellFrame*>(pRow3->GetLower());
    CPPUNIT_ASSERT(pCell3->GetFollowCell());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyDeletedAnchor)
{
    // Given a document with a floating table that spans over 3 pages:
    createSwDoc("floattable-deleted-anchor.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that there are 3 anchors for the 3 pages:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    SwFrame* pBody1 = pPage1->GetLower();
    CPPUNIT_ASSERT(pBody1);
    auto pAnchor1 = dynamic_cast<SwTextFrame*>(pBody1->GetLower()->GetNext());
    CPPUNIT_ASSERT(pAnchor1);
    SwTextFrame* pAnchor2 = pAnchor1->GetFollow();
    CPPUNIT_ASSERT(pAnchor2);
    SwTextFrame* pAnchor3 = pAnchor2->GetFollow();
    // Without the accompanying fix in place, this test would have failed, the fly frame on the 3rd
    // page was anchored to a text frame on the 2nd page, leading to a negative frame height.
    CPPUNIT_ASSERT(pAnchor3);
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyMultiCol)
{
    // Given a document with a floating table that is in a multi-col section:
    createSwDoc("floattable-multi-col.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the fly frame is not split, matching Word:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    // Without the accompanying fix in place, this test would have failed, we tried to split and
    // then hit an assertion failure.
    CPPUNIT_ASSERT(!pPage1Fly->GetFollow());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyTabJoin)
{
    // Given a document with 3 pages and 2 tables: table on first and second page, 3rd page has no
    // table:
    createSwDoc("floattable-tab-join.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that all pages have the expected amount of fly frames:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the 2nd page had 2 fly frames, hosting a split table, instead of joining that table and
    // having 1 fly frame.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage3 = dynamic_cast<SwPageFrame*>(pPage2->GetNext());
    CPPUNIT_ASSERT(pPage3);
    CPPUNIT_ASSERT(!pPage3->GetSortedObjs());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyTabJoinLegacy)
{
    // Given a document with 3 pages and 2 tables: table on first and second page, 3rd page has no
    // table (Word 2010 mode):
    createSwDoc("floattable-tab-join-legacy.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that all pages have the expected amount of fly frames:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the 2nd page had 2 fly frames, hosting a split table, instead of joining that table and
    // having 1 fly frame (even after the non-legacy case was fixed already).
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage3 = dynamic_cast<SwPageFrame*>(pPage2->GetNext());
    CPPUNIT_ASSERT(pPage3);
    CPPUNIT_ASSERT(!pPage3->GetSortedObjs());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyObjectFormatter)
{
    // Given a document with 3 pages and 2 tables: table on first and second page, 3rd page has no
    // table:
    createSwDoc("floattable-object-formatter.docx");

    // When calculating the layout:
    calcLayout();

    // Then make sure we don't crash and also that all pages have the expected amount of fly frames:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage3 = dynamic_cast<SwPageFrame*>(pPage2->GetNext());
    CPPUNIT_ASSERT(pPage3);
    CPPUNIT_ASSERT(!pPage3->GetSortedObjs());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyNextLeafInSection)
{
    // Given a document with 4 pages: page 1 had a floating table, page 2 & 3 had a second floating
    // table and finally page 4 is empty:
    createSwDoc("floattable-next-leaf-in-section.docx");

    // When calculating the layout:
    // Then this never returned, the loop in SwFrame::GetNextFlyLeaf() never finished.
    calcLayout();
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyAnchorKeepWithNext)
{
    // Given a document with 2 pages, a split floating table on both pages:
    createSwDoc("floattable-anchor-keep-with-next.docx");

    // When calculating the layout:
    calcLayout();

    // Then make sure the pages have the expected amount of anchored objects:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    // Without the accompanying fix in place, this test would have failed, page 1 had no floating
    // table, it was entirely on page 2.
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyNoFooterOverlap)
{
    // Given a document with 2 pages, a floating table on both pages:
    createSwDoc("floattable-no-footer-overlap.doc");

    // When calculating the layout:
    calcLayout();

    // Then make sure the second page has a floating table:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. part of the second table was on page 1.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    // Without the accompanying fix in place, this test would have failed, there was no page 2, both
    // floating tables were on page 1.
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyGrowFromBottom)
{
    // Given a document with a floating table that grows from the bottom:
    createSwDoc("floattable-from-bottom.docx");

    // When calculating the layout:
    calcLayout();

    // Then make sure that such a floating table is not split, matching Word:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    const auto pFly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    // Without the accompanying fix in place, this test would have failed, we tried to split the fly
    // frame on page 1 even when it would fit, and this lead to a crash on export later.
    CPPUNIT_ASSERT(!pFly->GetFollow());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyIntoTable)
{
    // Given a document with a floating table, then an inline table on the next page:
    createSwDoc("floattable-then-table.doc");

    // When inserting a page break:
    // Then make sure the layout doesn't crash:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();
    // Without the accompanying fix in place, this test would have crashed, we tried to insert the
    // second part of a floating table into a table on the next page, not before that table.
    calcLayout();
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyFromAsCharAnchor)
{
    // Given a document with a footnote that has a table (imported in an as-char anchored frame in
    // Writer):
    createSwDoc("table-in-footnote.docx");

    // When changing the anchor type of that frame to to-para:
    // Then make sure we don't crash:
    selectShape(1);
    // Without the accompanying fix in place, this test would have crashed, we tried to split a
    // frame+table inside a footnote.
    dispatchCommand(mxComponent, u".uno:SetAnchorToPara"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyNested)
{
    // Given a document with a nested, multi-page floating table:
    // When calculating the layout:
    createSwDoc("floattable-nested.odt");
    calcLayout();

    // Then make sure we don't crash:
    // Without the accompanying fix in place, this test would have crashed.
    // Check that we have exactly 4 fly frames, all of them on the expected pages: master outer,
    // follow outer, master inner and follow inner.
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPage1Objs.size());
    auto pPage1Fly1 = rPage1Objs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
    CPPUNIT_ASSERT(pPage1Fly1);
    CPPUNIT_ASSERT(pPage1Fly1->GetAnchorFrameContainingAnchPos()->IsInFly());
    CPPUNIT_ASSERT(pPage1Fly1->GetFollow());
    auto pPage1Fly2 = rPage1Objs[1]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
    CPPUNIT_ASSERT(pPage1Fly2);
    CPPUNIT_ASSERT(!pPage1Fly2->GetAnchorFrameContainingAnchPos()->IsInFly());
    CPPUNIT_ASSERT(pPage1Fly2->GetFollow());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPage2Objs.size());
    auto pPage2Fly1 = rPage2Objs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
    CPPUNIT_ASSERT(pPage2Fly1);
    CPPUNIT_ASSERT(pPage2Fly1->GetAnchorFrameContainingAnchPos()->IsInFly());
    CPPUNIT_ASSERT(pPage2Fly1->GetPrecede());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 6204
    // - Actual  : 1725
    // i.e. the inner follow fly had a bad position, it was outside the page rectangle, it was not
    // rendered and this way the inner anchor had no fly portion, either.
    CPPUNIT_ASSERT_GREATER(pPage2->getFrameArea().Top(), pPage2Fly1->getFrameArea().Top());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than: 12523
    // - Actual  : 15312
    // i.e. the inner follow fly was not "moved back" to its place to have the wanted 4400 position,
    // which makes the "Inner A2" text visible.
    CPPUNIT_ASSERT_LESS(pPage2->getFrameArea().Right(), pPage2Fly1->getFrameArea().Right());

    auto pPage2Fly2 = rPage2Objs[1]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
    CPPUNIT_ASSERT(pPage2Fly2);
    CPPUNIT_ASSERT(!pPage2Fly2->GetAnchorFrameContainingAnchPos()->IsInFly());
    CPPUNIT_ASSERT(pPage2Fly2->GetPrecede());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyNestedOverlap)
{
    // Given a document with a nested, multi-page floating table, enabling the "don't overlap" logic:
    // When calculating the layout:
    createSwDoc("floattable-nested-overlap.odt");
    calcLayout();

    // Then make sure we get 2 pages (2 flys on each page):
    // Without the accompanying fix in place, this test would have failed with a layout loop.
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPage1Objs.size());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPage2Objs.size());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyMoveMaster)
{
    // Given a document with a multi-page floating table on pages 1 -> 2 -> 3:
    createSwDoc("floattable-move-master.docx");

    // When adding an empty para at the start of the anchor text of the table:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/4);
    pWrtShell->SplitNode();

    // Then make sure page the fly chain is pages 1 -> 2 -> 3, still:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    // Without the accompanying fix in place, this test would have failed, the fly chain was pages 1
    // -> 4 -> 2.
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = rPage1Objs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
    CPPUNIT_ASSERT(pPage1Fly);
    CPPUNIT_ASSERT(!pPage1Fly->GetPrecede());
    CPPUNIT_ASSERT(pPage1Fly->HasFollow());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = rPage2Objs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
    CPPUNIT_ASSERT(pPage2Fly);
    CPPUNIT_ASSERT(pPage2Fly->GetPrecede());
    CPPUNIT_ASSERT(pPage2Fly->HasFollow());
    auto pPage3 = pPage2->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage3);
    CPPUNIT_ASSERT(pPage3->GetSortedObjs());
    SwSortedObjs& rPage3Objs = *pPage3->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage3Objs.size());
    auto pPage3Fly = rPage3Objs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
    CPPUNIT_ASSERT(pPage3Fly);
    CPPUNIT_ASSERT(pPage3Fly->GetPrecede());
    CPPUNIT_ASSERT(!pPage3Fly->GetFollow());
    auto pPage4 = pPage3->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage4);
    CPPUNIT_ASSERT(!pPage4->GetSortedObjs());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyDelEmpty)
{
    // Given a document with multiple floating tables and 7 pages:
    // When loading that document:
    // Without the accompanying fix in place, this test would have crashed due to a
    // heap-use-after-free problem (visible with e.g. MALLOC_PERTURB_=153).
    createSwDoc("floattable-del-empty.docx");

    // Then make sure that the page count matches Word:
    CPPUNIT_ASSERT_EQUAL(7, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyInTableInSection)
{
    // Given a document where page 2 and page 3 has a floating table inside an inline table, inside
    // a section:
    // Without the accompanying fix in place, this test would have crashed, we created a follow
    // anchor which was marked as "in table", but had no table parent.
    createSwDoc("floattable-in-inltbl-in-sect.docx");

    // Then make sure that the floating table is on page 2 and page 3:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(!pPage1->GetSortedObjs());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage3 = pPage2->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage3);
    CPPUNIT_ASSERT(pPage3->GetSortedObjs());
    SwSortedObjs& rPage3Objs = *pPage3->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage3Objs.size());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyWrapOnAllPages)
{
    // Given a document where we want to wrap on all pages, around a split floating table:
    createSwDoc("floattable-wrap-on-all-pages.docx");
    SwDoc* pDoc = getSwDoc();
    pDoc->getIDocumentSettingAccess().set(DocumentSettingId::ALLOW_TEXT_AFTER_FLOATING_TABLE_BREAK,
                                          true);

    // When formatting that document:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Reformat();

    // Then make sure that the anchor text is also split between page 1 and page 2:
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    auto pPage1Anchor = pPage1->FindLastBodyContent()->DynCastTextFrame();
    CPPUNIT_ASSERT(pPage1Anchor);
    OUString aAnchor1Text(pPage1Anchor->GetText().subView(
        static_cast<sal_Int32>(pPage1Anchor->GetOffset()),
        static_cast<sal_Int32>(pPage1Anchor->GetFollow()->GetOffset()
                               - pPage1Anchor->GetOffset())));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: He heard quiet steps behind him. That
    // - Actual  :
    // i.e. the first page had no anchor text, only the second.
    CPPUNIT_ASSERT_EQUAL(u"He heard quiet steps behind him. That "_ustr, aAnchor1Text);
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    auto pPage2Anchor = pPage2->FindLastBodyContent()->DynCastTextFrame();
    CPPUNIT_ASSERT(pPage2Anchor);
    OUString aAnchor2Text(
        pPage2Anchor->GetText().subView(static_cast<sal_Int32>(pPage2Anchor->GetOffset())));
    CPPUNIT_ASSERT(!pPage2Anchor->GetFollow());
    CPPUNIT_ASSERT_EQUAL(u"didn't bode well."_ustr, aAnchor2Text);
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyPerFrameWrapOnAllPages)
{
    // Given a document where we want to wrap on all pages, around a split floating table:
    createSwDoc("floattable-wrap-on-all-pages.docx");
    SwDoc* pDoc = getSwDoc();
    sw::SpzFrameFormats& rFlys = *pDoc->GetSpzFrameFormats();
    sw::SpzFrameFormat* pFly = rFlys[0];
    SfxItemSet aSet(pFly->GetAttrSet());
    SwFormatWrapTextAtFlyStart aItem(true);
    aSet.Put(aItem);
    pDoc->SetFlyFrameAttr(*pFly, aSet);

    // When formatting that document:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Reformat();

    // Then make sure that the anchor text is also split between page 1 and page 2:
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    auto pPage1Anchor = pPage1->FindLastBodyContent()->DynCastTextFrame();
    CPPUNIT_ASSERT(pPage1Anchor);
    OUString aAnchor1Text(pPage1Anchor->GetText().subView(
        static_cast<sal_Int32>(pPage1Anchor->GetOffset()),
        static_cast<sal_Int32>(pPage1Anchor->GetFollow()->GetOffset()
                               - pPage1Anchor->GetOffset())));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: He heard quiet steps behind him. That
    // - Actual  :
    // i.e. the first page had no anchor text, only the second.
    CPPUNIT_ASSERT_EQUAL(u"He heard quiet steps behind him. That "_ustr, aAnchor1Text);
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    auto pPage2Anchor = pPage2->FindLastBodyContent()->DynCastTextFrame();
    CPPUNIT_ASSERT(pPage2Anchor);
    OUString aAnchor2Text(
        pPage2Anchor->GetText().subView(static_cast<sal_Int32>(pPage2Anchor->GetOffset())));
    CPPUNIT_ASSERT(!pPage2Anchor->GetFollow());
    CPPUNIT_ASSERT_EQUAL(u"didn't bode well."_ustr, aAnchor2Text);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
