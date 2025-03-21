/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <docsh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <swmodule.hxx>
#include <redline.hxx>

namespace
{
/// Covers sw/source/core/edit/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/edit/data/"_ustr)
    {
    }
};
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineHidden)
{
    // Given a document with ShowRedlineChanges=false:
    createSwDoc("redline-hidden.fodt");

    // When formatting a paragraph by setting the para adjust to center, then make sure setting the
    // new item set on the paragraph doesn't crash:
    SwView* pView = getSwDocShell()->GetView();
    SfxItemSet aSet(pView->GetPool(), svl::Items<RES_PARATR_ADJUST, RES_PARATR_ADJUST>);
    SvxAdjustItem aItem(SvxAdjust::Center, RES_PARATR_ADJUST);
    aSet.Put(aItem);
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SetAttrSet(aSet, SetAttrMode::DEFAULT, nullptr, true);
}

CPPUNIT_TEST_FIXTURE(Test, testAutocorrect)
{
    // Given an empty document:
    createSwDoc();

    // When typing a string, which contains a "-", then make sure no memory corruption happens when
    // it gets auto-corrected to "–":
    // Without the accompanying fix in place, this test would have failed with a
    // heap-use-after-free:
    emulateTyping(u"But not now - with ");
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteSelNormalize)
{
    // Given a read-only document with a fillable form, the placeholder text is selected:
    createSwDoc("delete-sel-normalize.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->GotoFormControl(/*bNext=*/true);

    // When you press 'delete' to type some content instead:
    pWrtShell->DelRight();

    // Then make sure the position after the delete is correct:
    SwPosition& rCursor = *pWrtShell->GetCursor()->GetPoint();
    // The full text is "key <field start><field separator><field end>", so this marks the position
    // after the field separator but before the field end.
    sal_Int32 nExpectedCharPos = strlen("key **");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6
    // - Actual  : 5
    // so we started to type between the field start and the field separator, nothing was visible on
    // the screen.
    CPPUNIT_ASSERT_EQUAL(nExpectedCharPos, rCursor.nContent.GetIndex());
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineReinstateSingleInsert)
{
    // Given a document with a single insertion:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert("aaa");
    SwModule* pModule = SwModule::get();
    pModule->SetRedlineAuthor("Alice");
    RedlineFlags nMode = pWrtShell->GetRedlineFlags();
    pWrtShell->SetRedlineFlags(nMode | RedlineFlags::On);
    pWrtShell->Insert("bbb");
    pWrtShell->SetRedlineFlags(nMode);
    pWrtShell->Insert("ccc");

    // When a 2nd user reinstates that change:
    pModule->SetRedlineAuthor("Bob");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:ReinstateTrackedChange", {});

    // Then make sure this results in a delete on top of an insert:
    SwDoc* pDoc = pWrtShell->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
    const SwRangeRedline* pRedline = rRedlines[0];
    const SwRedlineData& rRedlineData = pRedline->GetRedlineData(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1 (Delete)
    // - Actual  : 0 (Insert)
    // i.e. reinstate didn't happen.
    CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlineData.GetType());
    CPPUNIT_ASSERT(rRedlineData.Next());
    const SwRedlineData& rInnerRedlineData = *rRedlineData.Next();
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rInnerRedlineData.GetType());
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineReinstateInsertsInSelection)
{
    // Given a document with two insertions:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert("aaa");
    SwModule* pModule = SwModule::get();
    pModule->SetRedlineAuthor("Alice");
    RedlineFlags nMode = pWrtShell->GetRedlineFlags();
    pWrtShell->SetRedlineFlags(nMode | RedlineFlags::On);
    pWrtShell->Insert("bbb");
    pWrtShell->SetRedlineFlags(nMode);
    pWrtShell->Insert("ccc");
    pWrtShell->SetRedlineFlags(nMode | RedlineFlags::On);
    pWrtShell->Insert("ddd");
    pWrtShell->SetRedlineFlags(nMode);
    pWrtShell->Insert("eee");

    // When a 2nd user reinstates those changes with a selection:
    pModule->SetRedlineAuthor("Bob");
    // Create a selection that excludes the initial "a" and the last "e":
    pWrtShell->SttPara(/*bSelect=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:ReinstateTrackedChange", {});

    // Then make sure this results in deletes on top of inserts:
    SwDoc* pDoc = pWrtShell->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0
    // i.e. a reject was performed instead of a reinstate.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rRedlines.size());
    const SwRangeRedline* pRedline1 = rRedlines[0];
    const SwRedlineData& rRedlineData1 = pRedline1->GetRedlineData(0);
    CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlineData1.GetType());
    CPPUNIT_ASSERT(rRedlineData1.Next());
    const SwRedlineData& rInnerRedlineData1 = *rRedlineData1.Next();
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rInnerRedlineData1.GetType());
    const SwRangeRedline* pRedline2 = rRedlines[1];
    const SwRedlineData& rRedlineData2 = pRedline2->GetRedlineData(0);
    CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlineData2.GetType());
    CPPUNIT_ASSERT(rRedlineData2.Next());
    const SwRedlineData& rInnerRedlineData2 = *rRedlineData2.Next();
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rInnerRedlineData2.GetType());
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineReinstateSinglePlainDelete)
{
    // Given a document with a single deletion:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert("abcd");
    SwModule* pModule = SwModule::get();
    pModule->SetRedlineAuthor("Alice");
    RedlineFlags nMode = pWrtShell->GetRedlineFlags();
    pWrtShell->SetRedlineFlags(nMode | RedlineFlags::On);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pWrtShell->DelRight();
    pWrtShell->SetRedlineFlags(nMode);

    // When a 2nd user reinstates that change:
    pModule->SetRedlineAuthor("Bob");
    pWrtShell->EndPara(/*bSelect=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:ReinstateTrackedChange", {});

    // Then make sure this results in an insert after a delete:
    SwDoc* pDoc = pWrtShell->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. reinstate didn't do anything.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rRedlines.size());
    const SwRangeRedline* pRedline1 = rRedlines[0];
    const SwRedlineData& rRedlineData1 = pRedline1->GetRedlineData(0);
    CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlineData1.GetType());
    const SwRangeRedline* pRedline2 = rRedlines[1];
    const SwRedlineData& rRedlineData2 = pRedline2->GetRedlineData(0);
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlineData2.GetType());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
