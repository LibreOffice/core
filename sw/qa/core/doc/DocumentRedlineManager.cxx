/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <editeng/wghtitem.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/propertyvalue.hxx>

#include <IDocumentRedlineAccess.hxx>
#include <docsh.hxx>
#include <redline.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <strings.hrc>
#include <fchrfmt.hxx>
#include <ndtxt.hxx>

namespace
{
/// Covers sw/source/core/doc/DocumentRedlineManager.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/doc/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testRedlineIns)
{
    // Given a document with an insert redline:
    createSwDoc("ins.docx");

    // When selecting BBB (a subset of the insert) and marking that as bold:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 3, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 3, /*bBasicCall=*/false);
    SwView& rView = pWrtShell->GetView();
    {
        SvxWeightItem aWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
        SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END> aSet(rView.GetPool());
        aSet.Put(aWeightItem);
        pWrtShell->SetAttrSet(aSet);
    }

    // Then make sure BBB is covered by an insert-then-format redline:
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    {
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 1
        // - Actual  : 3
        // i.e. no redline was created for the format change.
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());
        CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[0]->GetType());
        const SwRedlineData& rRedlineData1 = rRedlines[1]->GetRedlineData(0);
        CPPUNIT_ASSERT_EQUAL(RedlineType::Format, rRedlineData1.GetType());
        CPPUNIT_ASSERT(rRedlineData1.Next());
        const SwRedlineData& rInnerRedlineData = *rRedlineData1.Next();
        CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rInnerRedlineData.GetType());
        CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[2]->GetType());
    }

    // And when undoing:
    pWrtShell->Undo();

    // Then make sure we again have a single insert that covers all text:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the insert redline of BBB was lost on undo.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[0]->GetType());
    CPPUNIT_ASSERT_EQUAL(u"AAABBBCCC"_ustr, rRedlines[0]->GetText());

    // And when redoing:
    pWrtShell->Redo();

    // Then make sure we get <ins>AAA<format>BBB</format>CCC</ins>:
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());
        CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[0]->GetType());
        const SwRedlineData& rRedlineData1 = rRedlines[1]->GetRedlineData(0);
        CPPUNIT_ASSERT_EQUAL(RedlineType::Format, rRedlineData1.GetType());
        // Without the accompanying fix in place, this test would have failed, the "format" redline
        // has no underlying "insert" redline.
        CPPUNIT_ASSERT(rRedlineData1.Next());
        const SwRedlineData& rInnerRedlineData = *rRedlineData1.Next();
        CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rInnerRedlineData.GetType());
        CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[2]->GetType());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testInsThenFormatSelf)
{
    // Given a document with <ins>A<format>B</format>C</ins> redlines, created by Alice:
    createSwDoc("ins-then-format-self.docx");
    SwModule* pModule = SwModule::get();
    pModule->SetRedlineAuthor("Alice");
    comphelper::ScopeGuard g(
        [pModule] { pModule->SetRedlineAuthor(SwResId(STR_REDLINE_UNKNOWN_AUTHOR)); });
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());
    pWrtShell->DelLeft();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rRedlines.size());

    // When deleting B:
    pWrtShell->DelLeft();

    // Then make sure that B is removed from the document (since this is a self-insert):
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. a delete was created instead of removing the insert.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
}

CPPUNIT_TEST_FIXTURE(Test, testFormatRedlineRecordOldCharStyle)
{
    // Given a document with one char style applied + redline record on:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"x"_ustr);
    pWrtShell->SelAll();
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue(u"Style"_ustr, uno::Any(u"Emphasis"_ustr)),
        comphelper::makePropertyValue(u"FamilyName"_ustr, uno::Any(u"CharacterStyles"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);
    pDocShell->SetChangeRecording(true);

    // When changing to a second char style:
    aPropertyValues = {
        comphelper::makePropertyValue(u"Style"_ustr, uno::Any(u"Strong Emphasis"_ustr)),
        comphelper::makePropertyValue(u"FamilyName"_ustr, uno::Any(u"CharacterStyles"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    // Then make sure the redline refers to the old char style and just to that:
    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
    const SwRangeRedline& rRedline = *rRedlines[0];
    CPPUNIT_ASSERT_EQUAL(RedlineType::Format, rRedline.GetType());
    auto pExtraData = dynamic_cast<const SwRedlineExtraData_FormatColl*>(rRedline.GetExtraData());
    CPPUNIT_ASSERT(pExtraData);
    std::shared_ptr<SfxItemSet> pRedlineSet = pExtraData->GetItemSet();
    CPPUNIT_ASSERT(pRedlineSet);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 5
    // i.e. more than just the char style change was recorded, which was unexpected.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), pRedlineSet->Count());
    const SwFormatCharFormat& rOldCharFormat = pRedlineSet->Get(RES_TXTATR_CHARFMT);
    CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, rOldCharFormat.GetCharFormat()->GetName());
}

CPPUNIT_TEST_FIXTURE(Test, testDelThenFormatDirect)
{
    // Given a document with a delete redline, part of it has a format redline on top:
    createSwDoc("del-then-format.docx");

    // When "directly" accepting the delete-then-format redline:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->AcceptRedline(1, /*bDirect=*/true);

    // Then make sure that the format gets accepted, and the delete redline is kept:
    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    {
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 1
        // - Actual  : 0
        // i.e. when ignoring the "direct" parameter, the delete redline was accepted instead of the
        // format one.
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
        const SwRedlineData& rRedlineData = rRedlines[0]->GetRedlineData(0);
        CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlineData.GetType());
        CPPUNIT_ASSERT(!rRedlineData.Next());

        // After first char inside the redline: bold.
        pWrtShell->SttEndDoc(/*bStt=*/true);
        pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
        SfxItemSetFixed<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT> aSet(pDoc->GetAttrPool());
        pWrtShell->GetCurAttr(aSet);
        const SvxWeightItem& rWeightItem = aSet.Get(RES_CHRATR_WEIGHT);
        CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeightItem.GetValue());
    }

    // And given an undo, so we can redo:
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());

    // When doing the same again via redo:
    pWrtShell->Redo();

    // Then make sure we again have a single large delete:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. instead of a large delete redline, we have 2 small ones for AAA and CCC only.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());

    // And given a reset state:
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());

    // When "directly" rejecting the delete-then-format redline:
    pWrtShell->RejectRedline(1, /*bDirect=*/true);

    // Then make sure that the format gets rejected and the delete redline is kept:
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
        const SwRedlineData& rRedlineData = rRedlines[0]->GetRedlineData(0);
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 1 (delete)
        // - Actual  : 2 (format)
        // i.e. the delete redline was rejected and format remained, not the other way around.
        CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlineData.GetType());
        CPPUNIT_ASSERT(!rRedlineData.Next());

        // After first char inside the redline: not bold anymore.
        pWrtShell->SttEndDoc(/*bStt=*/true);
        pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
        SfxItemSetFixed<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT> aSet(pDoc->GetAttrPool());
        pWrtShell->GetCurAttr(aSet);
        const SvxWeightItem& rWeightItem = aSet.Get(RES_CHRATR_WEIGHT);
        CPPUNIT_ASSERT_EQUAL(WEIGHT_NORMAL, rWeightItem.GetValue());
    }

    // And given an undo:
    pWrtShell->Undo();

    // When redoing:
    pWrtShell->Redo();

    // Then make sure that we only get a single big delete redline:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 3
    // i.e. we got <del>AAA</del><format>BBB</format><del>CCC</del> instead of one big delete
    // redline.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
}

CPPUNIT_TEST_FIXTURE(Test, testInsThenFormatDirect)
{
    // Given a document with an insert redline, part of it has a format redline on top:
    createSwDoc("ins-then-format.docx");

    // When "directly" accepting the insert-then-format redline:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->AcceptRedline(1, /*bDirect=*/true);

    // Then make sure that the format gets accepted, and the insert redline is kept:
    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    {
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 1
        // - Actual  : 3
        // i.e. the accept didn't do anything in direct mode.
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
        const SwRedlineData& rRedlineData = rRedlines[0]->GetRedlineData(0);
        CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlineData.GetType());
        CPPUNIT_ASSERT(!rRedlineData.Next());

        // After first char inside the redline: bold.
        pWrtShell->SttEndDoc(/*bStt=*/true);
        pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
        SfxItemSetFixed<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT> aSet(pDoc->GetAttrPool());
        pWrtShell->GetCurAttr(aSet);
        const SvxWeightItem& rWeightItem = aSet.Get(RES_CHRATR_WEIGHT);
        CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeightItem.GetValue());
    }

    // And given an undo, so we can redo:
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());

    // When doing the same again via redo:
    pWrtShell->Redo();

    // Then make sure we again have a single large insert:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 3
    // i.e. redo was broken, the middle part was a format redline instead of an insert redline.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());

    // And given a reset state:
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());

    // When "directly" rejecting the insert-then-format redline:
    pWrtShell->RejectRedline(1, /*bDirect=*/true);

    // Then make sure that the format gets rejected and the insert redline is kept:
    {
        SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: AAABBBCCC
        // - Actual  : AAACCC
        // i.e. the insert was rejected, not the format.
        CPPUNIT_ASSERT_EQUAL(u"AAABBBCCC"_ustr, pTextNode->GetText());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
        const SwRedlineData& rRedlineData = rRedlines[0]->GetRedlineData(0);
        CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlineData.GetType());
        CPPUNIT_ASSERT(!rRedlineData.Next());

        // After first char inside the redline: not bold anymore.
        pWrtShell->SttEndDoc(/*bStt=*/true);
        pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
        SfxItemSetFixed<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT> aSet(pDoc->GetAttrPool());
        pWrtShell->GetCurAttr(aSet);
        const SvxWeightItem& rWeightItem = aSet.Get(RES_CHRATR_WEIGHT);
        CPPUNIT_ASSERT_EQUAL(WEIGHT_NORMAL, rWeightItem.GetValue());
    }

    // And given an undo:
    pWrtShell->Undo();

    // When redoing:
    pWrtShell->Redo();

    // Then make sure that we only get a single big insert redline:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. we got <ins>AAA</ins>BBB<ins>CCC</ins> instead of one big insert.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
}

CPPUNIT_TEST_FIXTURE(Test, testDelThenFormatOwn)
{
    // Given a document with an overlapping delete and format redline:
    // When importing that document:
    createSwDoc("del-then-format-own.docx");

    // Then make sure that the overlap part is not lost, instead it's represented with an
    // SwRangeRedline with 2 SwRedlineData members:
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 2
    // i.e. the document had a format and a delete redline, but part of the format was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());
    {
        const SwRedlineData& rRedlineData = rRedlines[0]->GetRedlineData(0);
        CPPUNIT_ASSERT_EQUAL(RedlineType::Format, rRedlineData.GetType());
        CPPUNIT_ASSERT(!rRedlineData.Next());
    }
    {
        // Overlap: both format and delete is tracked, so reject removes all boldness from the
        // document.
        const SwRedlineData& rRedlineData = rRedlines[1]->GetRedlineData(0);
        CPPUNIT_ASSERT_EQUAL(RedlineType::Format, rRedlineData.GetType());
        CPPUNIT_ASSERT(rRedlineData.Next());
        const SwRedlineData& rRedlineData2 = rRedlines[1]->GetRedlineData(1);
        CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlineData2.GetType());
        CPPUNIT_ASSERT(!rRedlineData2.Next());
    }
    {
        const SwRedlineData& rRedlineData = rRedlines[2]->GetRedlineData(0);
        CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlineData.GetType());
        CPPUNIT_ASSERT(!rRedlineData.Next());
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
