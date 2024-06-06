/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <vcl/scheduler.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <anchoredobject.hxx>
#include <docsh.hxx>
#include <flyfrm.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <rowfrm.hxx>
#include <sortedobjs.hxx>
#include <tabfrm.hxx>
#include <wrtsh.hxx>
#include <sectfrm.hxx>

namespace
{
/// Covers sw/source/core/layout/layact.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/layout/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyNextRowInvalidatePos)
{
    // Given a multi-page floating table, row1 is split, i.e. is both on page 1 and page 2:
    createSwDoc("floattable-next-row-invalidate-pos.docx");
    // Make sure the follow anchor's IsCompletePaint() reaches its false state, as it happens in the
    // interactive case.
    Scheduler::ProcessEventsToIdle();
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    auto pFly2 = rPage2Objs[0]->DynCastFlyFrame();
    auto pTable2 = pFly2->GetLower()->DynCastTabFrame();
    auto pRow2 = pTable2->GetLastLower()->DynCastRowFrame();
    SwTwips nOldRow2Top = pRow2->getFrameArea().Top();

    // When adding a new paragraph at the end of B1:
    // Go to the table: A1 cell.
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GotoTable(u"Table1"_ustr);
    // Go to the column: B1 cell.
    pWrtShell->GoNextCell();
    // Go to the end of the B1 cell, on page 2.
    pWrtShell->EndOfSection();
    // Add a new paragraph at the cell end.
    pWrtShell->SplitNode();

    // Then make sure row 2 is shifted down:
    SwTwips nNewRow2Top = pRow2->getFrameArea().Top();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 7121
    // - Actual  : 7121
    // i.e. row 2 has to be shifted down to 7390, but this didn't happen.
    CPPUNIT_ASSERT_GREATER(nOldRow2Top, nNewRow2Top);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf157096)
{
    createSwDoc("tdf157096.docx");

    CPPUNIT_ASSERT_EQUAL(2, getPages());
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyInSection)
{
    // Given a document with multiple sections, the 2nd section on page 1 has a one-page floating
    // table:
    createSwDoc("floattable-in-section.docx");

    // When laying out that document:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();

    // Then make sure the table is on page 1, not on page 2:
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    // Without the fix in place, it would have failed, the table was on page 2, not on page 1.
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(!pPage2->GetSortedObjs());
}

CPPUNIT_TEST_FIXTURE(Test, testBadSplitSection)
{
    // Given a document with a section, containing 5 paragraphs:
    createSwDoc("bad-split-section.odt");

    // When laying out that document:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();

    // Then make sure the entire section is on page 1:
    auto pPage = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage);
    auto pBody = pPage->FindBodyCont();
    CPPUNIT_ASSERT(pBody);
    auto pSection = dynamic_cast<SwSectionFrame*>(pBody->GetLastLower());
    CPPUNIT_ASSERT(pSection);
    // Without the fix in place, it would have failed, the section was split between page 1 and page
    // 2.
    CPPUNIT_ASSERT(!pSection->GetFollow());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
