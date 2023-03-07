/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/configuration.hxx>
#include <comphelper/scopeguard.hxx>
#include <officecfg/Office/Writer.hxx>

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
};

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyWithTable)
{
    // Given a document with a multi-page floating table:
    std::shared_ptr<comphelper::ConfigurationChanges> pChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(true,
                                                                                        pChanges);
    pChanges->commit();
    comphelper::ScopeGuard g([pChanges] {
        officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(
            false, pChanges);
        pChanges->commit();
    });
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
    std::shared_ptr<comphelper::ConfigurationChanges> pChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(true,
                                                                                        pChanges);
    pChanges->commit();
    comphelper::ScopeGuard g([pChanges] {
        officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(
            false, pChanges);
        pChanges->commit();
    });
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
    std::shared_ptr<comphelper::ConfigurationChanges> pChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(true,
                                                                                        pChanges);
    pChanges->commit();
    comphelper::ScopeGuard g([pChanges] {
        officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(
            false, pChanges);
        pChanges->commit();
    });
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
    std::shared_ptr<comphelper::ConfigurationChanges> pChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(true,
                                                                                        pChanges);
    pChanges->commit();
    comphelper::ScopeGuard g([pChanges] {
        officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(
            false, pChanges);
        pChanges->commit();
    });
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
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = getSwDoc();
    SwPageDesc aStandard(pDoc->GetPageDesc(0));
    SwFormatFrameSize aPageSize(aStandard.GetMaster().GetFrameSize());
    // 5cm for the page height, 2cm are the top and bottom margins, so 1cm remains for the body
    // frame:
    aPageSize.SetHeight(2834);
    aStandard.GetMaster().SetFormatAttr(aPageSize);
    pDoc->ChgPageDesc(0, aStandard);
    // Insert a table:
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    pWrtShell->SelAll();
    // Wrap it in a text frame:
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    pWrtShell->StartAllAction();
    aMgr.InsertFlyFrame(RndStdIds::FLY_AT_PARA, aMgr.GetPos(), aMgr.GetSize());
    pWrtShell->EndAllAction();

    // When allowing the text frame to split:
    pWrtShell->StartAllAction();
    SwFrameFormats& rFlys = *pDoc->GetSpzFrameFormats();
    SwFrameFormat* pFly = rFlys[0];
    SwAttrSet aSet(pFly->GetAttrSet());
    aSet.Put(SwFormatFlySplit(true));
    pDoc->SetAttr(aSet, *pFly);
    pWrtShell->EndAllAction();

    // Then make sure that the layout is updated and we have 2 pages:
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
    std::shared_ptr<comphelper::ConfigurationChanges> pChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(true,
                                                                                        pChanges);
    pChanges->commit();
    comphelper::ScopeGuard g([pChanges] {
        officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(
            false, pChanges);
        pChanges->commit();
    });
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
    std::shared_ptr<comphelper::ConfigurationChanges> pChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(true,
                                                                                        pChanges);
    pChanges->commit();
    comphelper::ScopeGuard g([pChanges] {
        officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(
            false, pChanges);
        pChanges->commit();
    });
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
    std::shared_ptr<comphelper::ConfigurationChanges> pChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(true,
                                                                                        pChanges);
    pChanges->commit();
    comphelper::ScopeGuard g([pChanges] {
        officecfg::Office::Writer::Filter::Import::DOCX::ImportFloatingTableAsSplitFly::set(
            false, pChanges);
        pChanges->commit();
    });
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
