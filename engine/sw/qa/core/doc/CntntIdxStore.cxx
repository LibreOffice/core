/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swtiledrenderingtest.hxx>

#include <comphelper/dispatchcommand.hxx>
#include <sfx2/kit/helper.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <docsh.hxx>
#include <swtestviewcallback.hxx>
#include <unotxdoc.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

using namespace css;

namespace
{
/// Covers sw/source/core/doc/CntntIdxStore.cxx fixes.
CPPUNIT_TEST_FIXTURE(SwTiledRenderingTest, testSplitNodeMultiViewOwnCursorMoves)
{
    // Given 2 views on a Writer document with a bulleted "foo" paragraph, view 2's
    // cursor at the start and view 1's cursor at the end:
    SwXTextDocument* pXTextDocument = createDoc();
    CPPUNIT_ASSERT(pXTextDocument);
    SwTestViewCallback aView1;
    int nView1 = KitHelper::getCurrentView();
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    comphelper::dispatchCommand(u".uno:DefaultBullet"_ustr, {});
    pWrtShell1->Insert(u"foo"_ustr);
    KitHelper::createView();
    SwTestViewCallback aView2;
    int nView2 = KitHelper::getCurrentView();
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
    KitHelper::setView(nView2);
    pWrtShell2->SttEndDoc(/*bStt=*/true);
    KitHelper::setView(nView1);
    pWrtShell1->SttEndDoc(/*bStt=*/false);
    const SwNodeOffset nOldNode = pWrtShell1->GetCursor()->GetPoint()->GetNodeIndex();
    CPPUNIT_ASSERT_EQUAL(nOldNode, pWrtShell2->GetCursor()->GetPoint()->GetNodeIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pWrtShell1->GetCursor()->GetPoint()->GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pWrtShell2->GetCursor()->GetPoint()->GetContentIndex());

    // Simulate the online-side state where SwView::GotFocus did not fire on
    // KitHelper::setView, so IDocumentLayoutAccess::GetCurrentViewShell() lags
    // behind on view 2 while view 1 is really about to run the split.
    pXTextDocument->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().SetCurrentViewShell(
        pWrtShell2);

    // When view 1 presses Enter at the end of "foo":
    pWrtShell1->SplitNode();

    // Then view 1's own cursor moves on to the new, second bullet; view 2 stays:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 10
    // - Actual  : 9
    // i.e. even the current view's cursor didn't move to the new bullet.
    CPPUNIT_ASSERT_EQUAL(nOldNode + SwNodeOffset(1),
                         pWrtShell1->GetCursor()->GetPoint()->GetNodeIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pWrtShell1->GetCursor()->GetPoint()->GetContentIndex());
    CPPUNIT_ASSERT_EQUAL(nOldNode, pWrtShell2->GetCursor()->GetPoint()->GetNodeIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pWrtShell2->GetCursor()->GetPoint()->GetContentIndex());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
