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

namespace
{
/// Covers sw/source/core/layout/layact.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/layout/data/")
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
    pWrtShell->GotoTable("Table1");
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
