/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/unoapi_test.hxx>
#include <comphelper/servicehelper.hxx>

#include <docsh.hxx>
#include <docfunc.hxx>
#include <cellmergeoption.hxx>
#include <tabvwsh.hxx>
#include <impex.hxx>
#include <scitems.hxx>
#include <attrib.hxx>
#include <userlist.hxx>
#include <undomanager.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScCopyPasteTest : public UnoApiTest
{
public:
    ScCopyPasteTest();

    void testCopyPasteXLS();
    void testTdf84411();
    void testTdf124565();
    void testTdf126421();
    void testTdf107394();
    void testTdf53431_fillOnAutofilter();
    void testTdf40993_fillMergedCells();
    void testTdf43958_clickSelectOnMergedCells();
    void testTdf88782_autofillLinearNumbersInMergedCells();
    void tdf137621_autofillMergedBool();
    void tdf137205_autofillDatesInMergedCells();
    void tdf137653_137654_autofillUserlist();
    void tdf113500_autofillMixed();
    void tdf137625_autofillMergedUserlist();
    void tdf137624_autofillMergedMixed();

    CPPUNIT_TEST_SUITE(ScCopyPasteTest);
    CPPUNIT_TEST(testCopyPasteXLS);
    CPPUNIT_TEST(testTdf84411);
    CPPUNIT_TEST(testTdf124565);
    CPPUNIT_TEST(testTdf126421);
    CPPUNIT_TEST(testTdf107394);
    CPPUNIT_TEST(testTdf53431_fillOnAutofilter);
    CPPUNIT_TEST(testTdf40993_fillMergedCells);
    CPPUNIT_TEST(testTdf43958_clickSelectOnMergedCells);
    CPPUNIT_TEST(testTdf88782_autofillLinearNumbersInMergedCells);
    CPPUNIT_TEST(tdf137621_autofillMergedBool);
    CPPUNIT_TEST(tdf137205_autofillDatesInMergedCells);
    CPPUNIT_TEST(tdf137653_137654_autofillUserlist);
    CPPUNIT_TEST(tdf113500_autofillMixed);
    CPPUNIT_TEST(tdf137625_autofillMergedUserlist);
    CPPUNIT_TEST(tdf137624_autofillMergedMixed);
    CPPUNIT_TEST_SUITE_END();

private:
    void addToUserList(const OUString& rStr);
};

// tdf#83366
void ScCopyPasteTest::testCopyPasteXLS()
{
    loadFromFile(u"xls/chartx2.xls");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // 2. Highlight B2:C5
    ScRange aSrcRange;
    ScRefFlags nRes = aSrcRange.Parse(u"B2:C5"_ustr, *pDoc, pDoc->GetAddressConvention());
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));

    ScMarkData aMark(pDoc->GetSheetLimits());
    aMark.SetMarkArea(aSrcRange);

    pViewShell->GetViewData().GetMarkData().SetMarkArea(aSrcRange);

    // 3. Copy
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, false, false);

    // 4. Close the document (Ctrl-W)
    mxComponent->dispose();
    mxComponent.clear();

    // Open a new document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);

    // Get the document controller
    pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // 6. Paste
    pViewShell->GetViewData().GetView()->PasteFromClip(InsertDeleteFlags::ALL, &aClipDoc);
}

namespace {

ScMarkData::MarkedTabsType TabsInRange(const ScRange& r)
{
    ScMarkData::MarkedTabsType aResult;
    for (SCTAB i = r.aStart.Tab(); i <= r.aEnd.Tab(); ++i)
        aResult.insert(i);
    return aResult;
}

void lcl_copy( const OUString& rSrcRange, const OUString& rDstRange, ScDocument& rDoc, ScTabViewShell* pViewShell )
{
    ScDocument aClipDoc(SCDOCMODE_CLIP);

    // 1. Copy
    ScRange aSrcRange;
    ScRefFlags nRes = aSrcRange.Parse(rSrcRange, rDoc, rDoc.GetAddressConvention());
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    pViewShell->GetViewData().GetMarkData().SetMarkArea(aSrcRange);
    pViewShell->GetViewData().GetMarkData().SetSelectedTabs(TabsInRange(aSrcRange));
    pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, false, false);

    // 2. Paste
    ScRange aDstRange;
    nRes = aDstRange.Parse(rDstRange, rDoc, rDoc.GetAddressConvention());
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    pViewShell->GetViewData().GetMarkData().SetMarkArea(aDstRange);
    pViewShell->GetViewData().GetMarkData().SetSelectedTabs(TabsInRange(aDstRange));
    pViewShell->GetViewData().GetView()->PasteFromClip(InsertDeleteFlags::ALL, &aClipDoc);
}

} // anonymous namespace

void ScCopyPasteTest::testTdf84411()
{
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // 2. Setup data and formulas
    for (unsigned int r = 0; r <= 4991; ++r)
        for (unsigned int c = 0; c <= 14; ++c)
            pDoc->SetValue( ScAddress(c,r,0), (r+1)*(c+1) );

    pDoc->SetString(ScAddress(15,10000,0), u"=AVERAGE(A10001:O10001)"_ustr);
    pDoc->SetString(ScAddress(16,10000,0), u"=MIN(A10001:O10001)"_ustr);
    pDoc->SetString(ScAddress(17,10000,0), u"=MAX(A10001:O10001)"_ustr);

    lcl_copy(u"P10001:R10001"_ustr, u"P10002:R12500"_ustr, *pDoc, pViewShell);


    // 3. Disable OpenCL
    bool bOpenCLState = ScCalcConfig::isOpenCLEnabled();
    pModelObj->enableOpenCL(false);
    CPPUNIT_ASSERT(!ScCalcConfig::isOpenCLEnabled() || ScCalcConfig::getForceCalculationType() == ForceCalculationOpenCL);
    pModelObj->enableAutomaticCalculation(true);


    // 4. Copy and Paste
    lcl_copy(u"A1:O2500"_ustr, u"A10001:O12500"_ustr, *pDoc, pViewShell);

    lcl_copy(u"A2501:O5000"_ustr, u"A12501:O15000"_ustr, *pDoc, pViewShell);

    lcl_copy(u"P10001:R10001"_ustr, u"P12501:R15000"_ustr, *pDoc, pViewShell);


    // 5. Close the document (Ctrl-W)
    pModelObj->enableOpenCL(bOpenCLState);
}

void ScCopyPasteTest::testTdf124565()
{
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // Set content and height of first row
    pDoc->SetString(ScAddress(0, 0, 0), u"Test"_ustr);
    pDoc->SetRowHeight(0, 0, 500);
    pDoc->SetManualHeight(0, 0, 0, true);

    // Copy first row
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aCopyRange(0, 0, 0, aClipDoc.MaxCol(), 0, 0);
    pViewShell->GetViewData().GetMarkData().SetMarkArea(aCopyRange);
    pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, false, false);

    // Paste to second row
    SCTAB nTab = 0;
    SCCOL nCol = 0;
    SCROW nRow = 1;

    ScRange aPasteRange(nCol, nRow, nTab, aClipDoc.MaxCol(), nRow, nTab);
    pViewShell->GetViewData().GetMarkData().SetMarkArea(aPasteRange);
    pViewShell->GetViewData().GetView()->PasteFromClip(InsertDeleteFlags::ALL, &aClipDoc);

    // Copy-pasted?
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String was not pasted!", u"Test"_ustr, pDoc->GetString(nCol, nRow, nTab));

    // And height same as in source?
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row#2 height is invalid!", sal_uInt16(500), pDoc->GetRowHeight(nRow, nTab));

    CPPUNIT_ASSERT_MESSAGE("Row#2 must be manual height!", pDoc->IsManualRowHeight(nRow, nTab));
}

void ScCopyPasteTest::testTdf126421()
{
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // 2. Setup data
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 1024; ++c)
            pDoc->SetValue(c, r, 0, (c + 1) * 100 + (r + 1));

    const SCTAB n2ndTab = pDoc->GetMaxTableNumber() + 1;
    pDoc->MakeTable(n2ndTab);
    const auto aTabNames = pDoc->GetAllTableNames();

    lcl_copy(aTabNames[0] + ".A1:AMJ2", aTabNames[n2ndTab] + ".A1:AMJ2", *pDoc, pViewShell);

    // 3. Check all cells in destination table
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 1024; ++c)
            CPPUNIT_ASSERT_EQUAL(double((c + 1) * 100 + (r + 1)), pDoc->GetValue(c, r, n2ndTab));
}

void ScCopyPasteTest::testTdf107394()
{
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    sal_uInt16 nFirstRowHeight = pDoc->GetRowHeight(0, 0);
    sal_uInt16 nSecondRowHeight = pDoc->GetRowHeight(1, 0);
    CPPUNIT_ASSERT_EQUAL(nFirstRowHeight, nSecondRowHeight);

    // Import values to A1:A2.
    ScImportExport aObj(*pDoc, ScAddress(0,0,0));
    aObj.SetImportBroadcast(true);

    SvMemoryStream aStream;
    aStream.WriteOString("<pre>First\nVery long sentence.</pre>");
    aStream.Seek(0);
    CPPUNIT_ASSERT(aObj.ImportStream(aStream, OUString(), SotClipboardFormatId::HTML));

    CPPUNIT_ASSERT_EQUAL(u"First"_ustr, pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(u"Very long sentence."_ustr, pDoc->GetString(ScAddress(0,1,0)));

    nFirstRowHeight = pDoc->GetRowHeight(0, 0);
    nSecondRowHeight = pDoc->GetRowHeight(1, 0);
    CPPUNIT_ASSERT_GREATER(nFirstRowHeight, nSecondRowHeight);

    // Undo, and check the result.
    SfxUndoManager* pUndoMgr = pDoc->GetUndoManager();
    CPPUNIT_ASSERT_MESSAGE("Failed to get the undo manager.", pUndoMgr);
    pUndoMgr->Undo();

    CPPUNIT_ASSERT(pDoc->GetString(ScAddress(0,0,0)).isEmpty());
    CPPUNIT_ASSERT(pDoc->GetString(ScAddress(0,1,0)).isEmpty());

    nFirstRowHeight = pDoc->GetRowHeight(0, 0);
    nSecondRowHeight = pDoc->GetRowHeight(1, 0);
    // Without the accompanying fix in place, this test would have failed:
    // - Expected: 256
    // - Actual  : 477
    // i.e. the increased height of the second row remained after undo.
    CPPUNIT_ASSERT_EQUAL(nFirstRowHeight, nSecondRowHeight);
}

static ScMF lcl_getMergeFlagOfCell(const ScDocument& rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    const SfxPoolItem& rPoolItem = rDoc.GetPattern(nCol, nRow, nTab)->GetItem(ATTR_MERGE_FLAG);
    const ScMergeFlagAttr& rMergeFlag = static_cast<const ScMergeFlagAttr&>(rPoolItem);
    return rMergeFlag.GetValue();
}

static ScAddress lcl_getMergeSizeOfCell(const ScDocument& rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    const SfxPoolItem& rPoolItem = rDoc.GetPattern(nCol, nRow, nTab)->GetItem(ATTR_MERGE);
    const ScMergeAttr& rMerge = static_cast<const ScMergeAttr&>(rPoolItem);
    return ScAddress(rMerge.GetColMerge(), rMerge.GetRowMerge(), nTab);
}

void ScCopyPasteTest::testTdf53431_fillOnAutofilter()
{
    loadFromFile(u"ods/tdf53431_autofilterFilldown.ods");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    //Fill should not clone Autofilter button
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(1, 1, 0, 2, 4, 0));
    pViewShell->FillSimple(FILL_TO_BOTTOM);
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 1, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 2, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(*pDoc, 1, 4, 0) & ScMF::Auto));

    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(1, 1, 0, 4, 4, 0));
    pViewShell->FillSimple(FILL_TO_RIGHT);
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 1, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 2, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(*pDoc, 4, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(*pDoc, 1, 4, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(*pDoc, 4, 4, 0) & ScMF::Auto));

    //Fill should not delete Autofilter buttons
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(0, 0, 0, 2, 4, 0));
    pViewShell->FillSimple(FILL_TO_TOP);
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 1, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 2, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(*pDoc, 1, 0, 0) & ScMF::Auto));

    //Fill should not clone Autofilter button
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(1, 1, 0, 2, 4, 0));
    pViewShell->FillSimple(FILL_TO_BOTTOM);
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 1, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 2, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(*pDoc, 1, 4, 0) & ScMF::Auto));

    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(1, 1, 0, 4, 4, 0));
    pViewShell->FillSimple(FILL_TO_RIGHT);
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 1, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 2, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(*pDoc, 4, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(*pDoc, 1, 4, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(*pDoc, 4, 4, 0) & ScMF::Auto));

    //Fill should not delete Autofilter buttons
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(0, 0, 0, 2, 4, 0));
    pViewShell->FillSimple(FILL_TO_TOP);
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 1, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 2, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(*pDoc, 1, 0, 0) & ScMF::Auto));

    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(0, 0, 0, 4, 4, 0));
    pViewShell->FillSimple(FILL_TO_LEFT);
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 1, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(*pDoc, 2, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(*pDoc, 0, 1, 0) & ScMF::Auto));
}

void ScCopyPasteTest::testTdf40993_fillMergedCells()
{
    loadFromFile(u"ods/tdf40993_fillMergedCells.ods");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // check content of the merged cell H11:I11
    CPPUNIT_ASSERT_EQUAL(u"1.5"_ustr, pDoc->GetString(ScAddress(7, 10, 0)));

    // fill operation on the merged cell should clone ATTR_MERGE and ATTR_MERGE_FLAG
    // (as long as ATTR_MERGE_FLAG has only ScMF::Hor or ScMF::Ver)
    //
    // select merged cell
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(7, 10, 0, 8, 10, 0));
    // copy its content in the next ten rows
    pViewShell->FillAuto(FILL_TO_BOTTOM, 7, 10, 8, 10, 10);
    for (int i = 7; i < 9; i++)
    {
        ScMF nOriginFlag = lcl_getMergeFlagOfCell(*pDoc, i, 10, 0);
        ScAddress aOriginMerge = lcl_getMergeSizeOfCell(*pDoc, i, 10, 0);

        // ATTR_MERGE_FLAG: top left cell is NONE, the other cell shows horizontal overlapping
        CPPUNIT_ASSERT_EQUAL(i == 7 ? ScMF::NONE : ScMF::Hor, nOriginFlag);

        // ATTR_MERGE: top left cell contains the size of the
        // merged area (2:1), the other cell doesn't
        CPPUNIT_ASSERT_EQUAL(i == 7 ? ScAddress(2, 1, 0): ScAddress(0, 0, 0), aOriginMerge);

        for (int j = 11; j < 21; j++)
        {
            // check copying of ATTR_MERGE and ATTR_MERGE_FLAG
            CPPUNIT_ASSERT_EQUAL(lcl_getMergeFlagOfCell(*pDoc, i, j, 0), nOriginFlag);
            CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(*pDoc, i, j, 0), aOriginMerge);
        }
    }

    CPPUNIT_ASSERT_EQUAL(lcl_getMergeFlagOfCell(*pDoc, 7, 21, 0),
                    lcl_getMergeFlagOfCell(*pDoc, 7, 10, 0));
    CPPUNIT_ASSERT(lcl_getMergeSizeOfCell(*pDoc, 7, 21, 0) !=
                    lcl_getMergeSizeOfCell(*pDoc, 7, 10, 0));
    CPPUNIT_ASSERT(lcl_getMergeFlagOfCell(*pDoc, 8, 21, 0) !=
                    lcl_getMergeFlagOfCell(*pDoc, 8, 10, 0));
    CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(*pDoc, 8, 21, 0),
                    lcl_getMergeSizeOfCell(*pDoc, 8, 10, 0));

    // area A6:E9 with various merged cells copied vertically and horizontally
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(0, 5, 0, 4, 8, 0));
    pViewShell->FillAuto(FILL_TO_BOTTOM, 0, 5, 4, 8, 12);
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(0, 5, 0, 4, 8, 0));
    pViewShell->FillAuto(FILL_TO_RIGHT, 0, 5, 4, 8, 10);
    for (int i = 0; i < 5; i++)
    {
        for (int j = 5; j < 9; j++)
        {
            ScMF nOriginFlag = lcl_getMergeFlagOfCell(*pDoc, i, j, 0);
            ScAddress aOriginMerge = lcl_getMergeSizeOfCell(*pDoc, i, j, 0);
            // copies contain the same ATTR_MERGE and ATTR_MERGE_FLAG
            for (int k = 0; k < 12; k += 4)
            {
                CPPUNIT_ASSERT_EQUAL(lcl_getMergeFlagOfCell(*pDoc, i, j + k, 0), nOriginFlag);
                CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(*pDoc, i, j + k, 0), aOriginMerge);
            }
            for (int k = 0; k < 10; k += 5)
            {
                CPPUNIT_ASSERT_EQUAL(lcl_getMergeFlagOfCell(*pDoc, i + k, j, 0), nOriginFlag);
                CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(*pDoc, i + k, j, 0), aOriginMerge);
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(*pDoc, 1, 5, 0), ScAddress(2, 4, 0));
    CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(*pDoc, 0, 5, 0), ScAddress(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(*pDoc, 4, 6, 0), ScAddress(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(*pDoc, 3, 5, 0), ScAddress(2, 1, 0));
}

static void lcl_clickAndCheckCurrentArea(SCCOL nCol, SCROW nRow, SCCOL nCol2, SCROW nRow2)
{
    ScRange aRange;
    ScDocShell::GetViewData()->SetCurX(nCol);
    ScDocShell::GetViewData()->SetCurY(nRow);
    ScDocShell::GetViewData()->GetSimpleArea(aRange);
    CPPUNIT_ASSERT_EQUAL(aRange, ScRange(nCol, nRow, 0, nCol2, nRow2, 0));
}

void ScCopyPasteTest::testTdf43958_clickSelectOnMergedCells()
{
    loadFromFile(u"ods/tdf40993_fillMergedCells.ods");

    // select cell (e.g. by clicking on it) and check what is selected [but not marked]:
    // if it is the top left cell of a merged area, the selection is enlarged to the area
    lcl_clickAndCheckCurrentArea(1, 5, 2, 8);    // B6 -> B6:C9
    lcl_clickAndCheckCurrentArea(0, 5, 0, 6);    // A6 -> A6:A7
    lcl_clickAndCheckCurrentArea(3, 5, 4, 5);    // D6 -> D6:E6
    lcl_clickAndCheckCurrentArea(4, 6, 4, 7);    // D7 -> D6:D7
    lcl_clickAndCheckCurrentArea(7, 10, 8, 10);  // H11 -> H11:I11
    lcl_clickAndCheckCurrentArea(7, 13, 8, 13);  // H14 -> H14:I14

    // otherwise it remains the same
    lcl_clickAndCheckCurrentArea(0, 7, 0, 7);    // A8
    lcl_clickAndCheckCurrentArea(0, 8, 0, 8);    // A9
    lcl_clickAndCheckCurrentArea(2, 6, 2, 6);    // C7
    lcl_clickAndCheckCurrentArea(2, 7, 2, 7);    // C8
    lcl_clickAndCheckCurrentArea(2, 8, 2, 8);    // C9
}

void ScCopyPasteTest::testTdf88782_autofillLinearNumbersInMergedCells()
{
    loadFromFile(u"ods/tdf88782_AutofillLinearNumbersInMergedCells.ods");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // merge the yellow cells
    ScCellMergeOption aMergeOptions(9, 11, 10, 13);     //J12:K14
    aMergeOptions.maTabs.insert(0);
    ScDocShell* pDocSh = ScDocShell::GetViewData()->GetDocShell();
    pDocSh->GetDocFunc().MergeCells(aMergeOptions, false, true, true, false);

    // fillauto numbers, these areas contain mostly merged cells
    pViewShell->FillAuto(FILL_TO_BOTTOM, 1, 8, 3, 14, 7);    // B9:D15 ->  B9:D22
    pViewShell->FillAuto(FILL_TO_BOTTOM, 5, 8, 7, 17, 10);   // F9:H18 ->  F9:H28
    pViewShell->FillAuto(FILL_TO_BOTTOM, 9, 8, 10, 13, 6);   // J9:K14 ->  J9:K20
    pViewShell->FillAuto(FILL_TO_RIGHT, 9, 30, 16, 35, 8);   //J31:Q36 -> J31:Y36
    pViewShell->FillAuto(FILL_TO_LEFT, 9, 30, 16, 35, 8);    //J31:Q36 -> B31:Q36

    // compare the results of fill-down with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    // the test file have instructions / explanations, so that is easy to understand
    for (int nCol = 1; nCol <= 10; nCol++)
    {
        for (int nRow = 8; nRow <= 27; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol + 22, nRow, 0));
            double* pValue1 = pDoc->GetValueCell(ScAddress(nCol, nRow, 0));
            double* pValue2 = pDoc->GetValueCell(ScAddress(nCol + 22, nRow, 0));

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            if (pValue2 != nullptr)
                CPPUNIT_ASSERT_EQUAL(*pValue1, *pValue2);   //cells with number value
            else
                CPPUNIT_ASSERT_EQUAL(pValue1, pValue2);     //empty cells
        }
    }

    // compare the results of fill-right and left with the reference stored in the test file
    for (int nCol = 1; nCol <= 24; nCol++)
    {
        for (int nRow = 30; nRow <= 35; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol, nRow + 16, 0));
            double* pValue1 = pDoc->GetValueCell(ScAddress(nCol, nRow, 0));
            double* pValue2 = pDoc->GetValueCell(ScAddress(nCol, nRow + 16, 0));

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            if (pValue2 != nullptr)
                CPPUNIT_ASSERT_EQUAL(*pValue1, *pValue2);
            else
                CPPUNIT_ASSERT_EQUAL(pValue1, pValue2);
        }
    }
}

void ScCopyPasteTest::tdf137621_autofillMergedBool()
{
    loadFromFile(u"ods/tdf137621_autofillMergedBool.ods");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // fillauto booleans, these areas contain only merged cells
    pViewShell->FillAuto(FILL_TO_RIGHT, 0, 4, 3, 5, 8);   //A5:D6

    // compare the results of fill-right with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    for (int nCol = 4; nCol <= 11; nCol++)
    {
        for (int nRow = 4; nRow <= 5; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol, nRow + 3, 0));
            double* pValue1 = pDoc->GetValueCell(ScAddress(nCol, nRow, 0));
            double* pValue2 = pDoc->GetValueCell(ScAddress(nCol, nRow + 3, 0));

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            if (pValue2 != nullptr)
                CPPUNIT_ASSERT_EQUAL(*pValue1, *pValue2);   //cells with boolean value
            else
                CPPUNIT_ASSERT_EQUAL(pValue1, pValue2);     //empty cells
        }
    }
}

void ScCopyPasteTest::tdf137205_autofillDatesInMergedCells()
{
    loadFromFile(u"ods/tdf137205_AutofillDatesInMergedCells.ods");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // fillauto dates, this areas contain only merged cells
    pViewShell->FillAuto(FILL_TO_RIGHT, 1, 5, 4, 7, 8);   //B6:E8

    // compare the results of fill-right with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    for (int nCol = 5; nCol <= 12; nCol++)
    {
        for (int nRow = 5; nRow <= 7; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol, nRow + 5, 0));
            double* pValue1 = pDoc->GetValueCell(ScAddress(nCol, nRow, 0));
            double* pValue2 = pDoc->GetValueCell(ScAddress(nCol, nRow + 5, 0));

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            if (pValue2 != nullptr)
                CPPUNIT_ASSERT_EQUAL(*pValue1, *pValue2);   //cells with number value
            else
                CPPUNIT_ASSERT_EQUAL(pValue1, pValue2);     //empty cells
        }
    }
}

void ScCopyPasteTest::addToUserList(const OUString& rStr)
{
    ScGlobal::GetUserList().emplace_back(rStr);
}

void ScCopyPasteTest::tdf137653_137654_autofillUserlist()
{
    loadFromFile(u"ods/tdf137653_137654_autofillUserlist.ods");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // delete every userlist to make sure there won't be any string that is in 2 different userlist
    ScGlobal::GetUserList().clear();
    addToUserList({ u"January,February,March,April,May,June,July,August,September,October,November,December"_ustr });
    const ScUserListData* pListData = ScGlobal::GetUserList().GetData(u"January"_ustr);
    sal_uInt16 nIdx1 = 0, nIdx2 = 0;
    bool bHasIdx1, bHasIdx2;
    bool bMatchCase = false;

    // fillauto userlist, these areas contain only merged cells
    pViewShell->FillAuto(FILL_TO_RIGHT, 4, 5, 6, 7, 3);   //E6:G8
    pViewShell->FillAuto(FILL_TO_LEFT, 4, 5, 6, 7, 3);    //E6:G8
    pViewShell->FillAuto(FILL_TO_BOTTOM, 1, 18, 3, 19, 2); //B19:D20
    pViewShell->FillAuto(FILL_TO_TOP, 1, 18, 3, 19, 2);    //B19:D20

    // compare the results of fill-right / -left with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    for (int nCol = 1; nCol <= 9; nCol++)
    {
        for (int nRow = 5; nRow <= 7; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol, nRow + 4, 0));
            bHasIdx1 = pListData->GetSubIndex(pDoc->GetString(nCol, nRow, 0), nIdx1, bMatchCase);
            bHasIdx2 = pListData->GetSubIndex(pDoc->GetString(nCol, nRow + 4, 0), nIdx2, bMatchCase);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT(bHasIdx1);
            CPPUNIT_ASSERT(bHasIdx2);
            CPPUNIT_ASSERT_EQUAL(nIdx1, nIdx2);   // userlist index value of cells
        }
    }

    // compare the results of fill-up / -down
    for (int nCol = 1; nCol <= 3; nCol++)
    {
        for (int nRow = 16; nRow <= 21; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol + 4, nRow, 0));
            bHasIdx1 = pListData->GetSubIndex(pDoc->GetString(nCol, nRow, 0), nIdx1, bMatchCase);
            bHasIdx2 = pListData->GetSubIndex(pDoc->GetString(nCol + 4, nRow, 0), nIdx2, bMatchCase);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT(bHasIdx1);
            CPPUNIT_ASSERT(bHasIdx2);
            CPPUNIT_ASSERT_EQUAL(nIdx1, nIdx2);   // userlist index value of cells
        }
    }
}

void ScCopyPasteTest::tdf113500_autofillMixed()
{
    loadFromFile(u"ods/tdf113500_autofillMixed.ods");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // fillauto userlist, these areas contain only merged cells
    pViewShell->FillAuto(FILL_TO_RIGHT, 4, 5, 6, 7, 3);   //E6:G8
    pViewShell->FillAuto(FILL_TO_LEFT, 4, 5, 6, 7, 3);    //E6:G8
    pViewShell->FillAuto(FILL_TO_BOTTOM, 1, 18, 3, 19, 2); //B19:D20
    pViewShell->FillAuto(FILL_TO_TOP, 1, 18, 3, 19, 2);    //B19:D20

    // compare the results of fill-right / -left with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    // do not check the 3. row: a1,b2,a3. It is another bug to fix
    for (int nCol = 1; nCol <= 9; nCol++)
    {
        for (int nRow = 5; nRow <= 6; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol, nRow + 4, 0));
            OUString aStr1 = pDoc->GetString(nCol, nRow, 0);
            OUString aStr2 = pDoc->GetString(nCol, nRow + 4, 0);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(aStr1, aStr2);
        }
    }

    // compare the results of fill-up / -down
    // do not check the 2. column: 1st,3rd. It is another bug to fix
    for (int nCol = 1; nCol <= 3; nCol+=2)
    {
        for (int nRow = 16; nRow <= 21; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol + 4, nRow, 0));
            OUString aStr1 = pDoc->GetString(nCol, nRow, 0);
            OUString aStr2 = pDoc->GetString(nCol + 4, nRow, 0);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(aStr1, aStr2);
        }
    }
}

void ScCopyPasteTest::tdf137625_autofillMergedUserlist()
{
    loadFromFile(u"ods/tdf137625_autofillMergedUserlist.ods");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // delete every userlist to make sure there won't be any string that is in 2 different userlist
    ScGlobal::GetUserList().clear();
    addToUserList({ u"January,February,March,April,May,June,July,August,September,October,November,December"_ustr });
    const ScUserListData* pListData = ScGlobal::GetUserList().GetData(u"January"_ustr);
    sal_uInt16 nIdx1 = 0, nIdx2 = 0;
    bool bHasIdx1, bHasIdx2;
    bool bMatchCase = false;

    // fillauto userlist, these areas contain only merged cells
    pViewShell->FillAuto(FILL_TO_RIGHT, 7, 5, 12, 7, 6);   //H6:M8
    pViewShell->FillAuto(FILL_TO_LEFT, 7, 5, 12, 7, 6);    //H6:M8
    pViewShell->FillAuto(FILL_TO_BOTTOM, 1, 20, 3, 23, 4); //B21:D24
    pViewShell->FillAuto(FILL_TO_TOP, 1, 20, 3, 23, 4);    //B21:D24

    // compare the results of fill-right / -left with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    for (int nCol = 1; nCol <= 18; nCol++)
    {
        for (int nRow = 5; nRow <= 7; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol, nRow + 4, 0));
            bHasIdx1 = pListData->GetSubIndex(pDoc->GetString(nCol, nRow, 0), nIdx1, bMatchCase);
            bHasIdx2 = pListData->GetSubIndex(pDoc->GetString(nCol, nRow + 4, 0), nIdx2, bMatchCase);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(bHasIdx1, bHasIdx2);
            if (bHasIdx1)
                CPPUNIT_ASSERT_EQUAL(nIdx1, nIdx2);   //cells with userlist value
        }
    }

    // compare the results of fill-up / -down
    for (int nCol = 1; nCol <= 3; nCol++)
    {
        for (int nRow = 16; nRow <= 27; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol + 4, nRow, 0));
            bHasIdx1 = pListData->GetSubIndex(pDoc->GetString(nCol, nRow, 0), nIdx1, bMatchCase);
            bHasIdx2 = pListData->GetSubIndex(pDoc->GetString(nCol + 4, nRow, 0), nIdx2, bMatchCase);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(bHasIdx1, bHasIdx2);
            if (bHasIdx1)
                CPPUNIT_ASSERT_EQUAL(nIdx1, nIdx2);   //cells with userlist value
        }
    }
}

void ScCopyPasteTest::tdf137624_autofillMergedMixed()
{
    loadFromFile(u"ods/tdf137624_autofillMergedMixed.ods");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    // add 1aa,2bb,3cc,4dd,5ee,6ff to userlist, to test that autofill won't confuse it with 1aa,3aa
    // delete every userlist to make sure there won't be any string that is in 2 different userlist
    ScGlobal::GetUserList().clear();
    addToUserList({ u"1aa,2bb,3cc,4dd,5ee,6ff"_ustr });

    // fillauto mixed (string + number), these areas contain only merged cells
    pViewShell->FillAuto(FILL_TO_RIGHT, 7, 5, 12, 7, 6);   //H6:M8
    pViewShell->FillAuto(FILL_TO_LEFT, 7, 5, 12, 7, 6);    //H6:M8
    pViewShell->FillAuto(FILL_TO_BOTTOM, 1, 20, 3, 23, 4); //B21:D24
    pViewShell->FillAuto(FILL_TO_TOP, 1, 20, 3, 23, 4);    //B21:D24

    // compare the results of fill-right / -left with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    for (int nCol = 1; nCol <= 18; nCol++)
    {
        for (int nRow = 5; nRow <= 7; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol, nRow + 4, 0));
            OUString aStr1 = pDoc->GetString(nCol, nRow, 0);
            OUString aStr2 = pDoc->GetString(nCol, nRow + 4, 0);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(aStr1, aStr2);
        }
    }

    // compare the results of fill-up / -down
    for (int nCol = 1; nCol <= 3; nCol++)
    {
        for (int nRow = 16; nRow <= 27; nRow++)
        {
            CellType nType1 = pDoc->GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = pDoc->GetCellType(ScAddress(nCol + 4, nRow, 0));
            OUString aStr1 = pDoc->GetString(nCol, nRow, 0);
            OUString aStr2 = pDoc->GetString(nCol + 4, nRow, 0);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(aStr1, aStr2);
        }
    }
}

ScCopyPasteTest::ScCopyPasteTest()
      : UnoApiTest(u"/sc/qa/unit/data/"_ustr)
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCopyPasteTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
