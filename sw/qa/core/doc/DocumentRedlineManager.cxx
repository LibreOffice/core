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
    CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, rOldCharFormat.GetCharFormat()->GetName().toString());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
