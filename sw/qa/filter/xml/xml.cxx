/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <editeng/fhgtitem.hxx>

#include <frameformats.hxx>
#include <frmatr.hxx>
#include <swtable.hxx>
#include <docsh.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <redline.hxx>
#include <wrtsh.hxx>
#include <fchrfmt.hxx>

namespace
{
/**
 * Covers sw/source/filter/xml/ fixes.
 *
 * Note that these tests are meant to be simple: either load a file and assert some result or build
 * a document model with code, export and assert that result.
 *
 * Keep using the various sw_<format>import/export suites for multiple filter calls inside a single
 * test.
 */
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/filter/xml/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testCoveredCellBackground)
{
    // Given a document with a table with vertically merged cells, with a solid background:
    createSwDoc("covered-cell-background.odt");

    // When checking the background of the last row's first covered table cell:
    SwDoc* pDoc = getSwDoc();
    const SwTableFormat* pTableFormat = (*pDoc->GetTableFrameFormats())[0];
    SwTable* pTable = SwTable::FindTable(pTableFormat);
    SwTableBox* pBox = const_cast<SwTableBox*>(pTable->GetTableBox(u"A5"_ustr));
    SwFrameFormat* pCellFormat = pBox->GetFrameFormat();
    const SvxBrushItem& rBackground = pCellFormat->GetAttrSet().GetBackground();

    // Then make sure the covered cell has a solid background, just like other cells in the first
    // column:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: rgba[e8f2a1ff]
    // - Actual  : rgba[ffffff00]
    // i.e. part of the merged cell had a bad white background.
    CPPUNIT_ASSERT_EQUAL(Color(0xe8f2a1), rBackground.GetColor());
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineRecordFlatExport)
{
    // Given a document with track changes enabled:
    createSwDoc();
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    // When saving that to FODT:
    save(TestFilter::FODT);

    // Then make sure this is written in the export result:
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '/office:document/office:body/office:text/text:tracked-changes' number of nodes is incorrect
    // i.e. the entire XML element was missing.
    OUString aValue = getXPath(
        pDoc, "/office:document/office:body/office:text/text:tracked-changes", "track-changes");
    CPPUNIT_ASSERT_EQUAL(u"true"_ustr, aValue);
}

CPPUNIT_TEST_FIXTURE(Test, testInsertThenFormatOdtImport)
{
    // Given a document with <ins>A<format>B</format>C</ins> style redlines:
    // When importing that document:
    createSwDoc("insert-then-format.odt");

    // Then make sure that both the insert and the format on top of it is in the model:
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[0]->GetType());
    const SwRedlineData& rRedlineData1 = rRedlines[1]->GetRedlineData(0);
    CPPUNIT_ASSERT_EQUAL(RedlineType::Format, rRedlineData1.GetType());
    // Without the accompanying fix in place, this test would have failed, i.e. the insert under the
    // format redline was lost.
    CPPUNIT_ASSERT(rRedlineData1.Next());
    const SwRedlineData& rInnerRedlineData = *rRedlineData1.Next();
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rInnerRedlineData.GetType());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[2]->GetType());
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteThenFormatOdtImport)
{
    // Given a document with <del>A<format>B</format>C</del> style redlines:
    // When importing that document:
    createSwDoc("delete-then-format.odt");

    // Then make sure that both the delete and the format on top of it is in the model:
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 2
    // i.e. there was an empty format redline at doc start and a delete redline for "AC".
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlines[0]->GetType());
    const SwRedlineData& rRedlineData1 = rRedlines[1]->GetRedlineData(0);
    CPPUNIT_ASSERT_EQUAL(RedlineType::Format, rRedlineData1.GetType());
    CPPUNIT_ASSERT(rRedlineData1.Next());
    const SwRedlineData& rInnerRedlineData = *rRedlineData1.Next();
    CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rInnerRedlineData.GetType());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlines[2]->GetType());
}

CPPUNIT_TEST_FIXTURE(Test, testFormatCharStyleChangeOdtImport)
{
    // Given a document with a format redline, containing a char style change (strong -> quote):
    // When importing that document:
    createSwDoc("format-char-style-change.odt");

    // Then make sure the model has the new style name, the redline has the old style name:
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    SfxItemSetFixed<RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT> aSet(pDoc->GetAttrPool());
    pWrtShell->GetCurAttr(aSet);
    const SwFormatCharFormat& rNewCharFormat = aSet.Get(RES_TXTATR_CHARFMT);
    CPPUNIT_ASSERT_EQUAL(u"Quote Char"_ustr, rNewCharFormat.GetCharFormat()->GetName().toString());
    const IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    const SwRedlineTable& rRedlineTable = rIDRA.GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlineTable.size());
    const SwRangeRedline* pRedline = rRedlineTable[0];
    auto pExtraData = dynamic_cast<const SwRedlineExtraData_FormatColl*>(pRedline->GetExtraData());
    // Without the accompanying fix in place, this test would have failed, the format redline didn't
    // contain the old style name.
    CPPUNIT_ASSERT(pExtraData);
    std::shared_ptr<SfxItemSet> pRedlineSet = pExtraData->GetItemSet();
    CPPUNIT_ASSERT(pRedlineSet);
    const SwFormatCharFormat& rOldCharFormat = pRedlineSet->Get(RES_TXTATR_CHARFMT);
    CPPUNIT_ASSERT_EQUAL(u"Strong Emphasis"_ustr,
                         rOldCharFormat.GetCharFormat()->GetName().toString());
}

CPPUNIT_TEST_FIXTURE(Test, testFormatCharstyleDirectOdtImport)
{
    // Given a document with a format redline, containing a char style change (strong -> quote) and
    // a font size change (24 -> 36pt):
    // When importing that document:
    createSwDoc("format-charstyle-direct.odt");

    // Then make sure the model has the new style name & new font size, the redline has the old
    // style name & old font size:
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    SfxItemSetFixed<RES_CHRATR_FONTSIZE, RES_TXTATR_CHARFMT> aSet(pDoc->GetAttrPool());
    pWrtShell->GetCurAttr(aSet);
    const SwFormatCharFormat& rNewCharFormat = aSet.Get(RES_TXTATR_CHARFMT);
    CPPUNIT_ASSERT_EQUAL(u"Quote Char"_ustr, rNewCharFormat.GetCharFormat()->GetName().toString());
    const SvxFontHeightItem& rNewFontSize = aSet.Get(RES_CHRATR_FONTSIZE);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(36 * 20), rNewFontSize.GetHeight());
    const IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    const SwRedlineTable& rRedlineTable = rIDRA.GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlineTable.size());
    const SwRangeRedline* pRedline = rRedlineTable[0];
    auto pExtraData = dynamic_cast<const SwRedlineExtraData_FormatColl*>(pRedline->GetExtraData());
    CPPUNIT_ASSERT(pExtraData);
    std::shared_ptr<SfxItemSet> pRedlineSet = pExtraData->GetItemSet();
    CPPUNIT_ASSERT(pRedlineSet);
    const SwFormatCharFormat& rOldCharFormat = pRedlineSet->Get(RES_TXTATR_CHARFMT);
    CPPUNIT_ASSERT_EQUAL(u"Strong Emphasis"_ustr,
                         rOldCharFormat.GetCharFormat()->GetName().toString());
    const SvxFontHeightItem& rOldFontSize = pRedlineSet->Get(RES_CHRATR_FONTSIZE);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 480
    // - Actual  : 240
    // i.e. the redline didn't contain a correct old font size (direct format).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(24 * 20), rOldFontSize.GetHeight());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
