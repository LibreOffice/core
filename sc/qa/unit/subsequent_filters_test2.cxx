/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <memory>

#include <osl/thread.h>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <svx/svdograf.hxx>

#include <svx/svdpage.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/borderline.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/colritem.hxx>
#include <cellvalue.hxx>
#include <docpool.hxx>
#include <dbdata.hxx>
#include <validat.hxx>
#include <formulacell.hxx>
#include <docfunc.hxx>
#include <markdata.hxx>
#include <olinetab.hxx>
#include <scitems.hxx>
#include <docsh.hxx>
#include <attrib.hxx>
#include <columnspanset.hxx>
#include <tokenstringcontext.hxx>
#include <externalrefmgr.hxx>
#include <filterentries.hxx>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <tools/UnitConversion.hxx>
#include <vcl/scheduler.hxx>
#include "helper/qahelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScFiltersTest2 : public ScModelTestBase
{
public:
    ScFiltersTest2();
};

ScFiltersTest2::ScFiltersTest2()
    : ScModelTestBase("sc/qa/unit/data")
{
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testMiscRowHeights)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    static const TestParam::RowData DfltRowData[] = {
        // check rows at the beginning and end of document
        // and make sure they are reported as the default row
        // height ( indicated by -1 )
        { 2, 4, 0, -1, 0, false },
        { 1048573, 1048575, 0, -1, 0, false },
    };

    static const TestParam::RowData MultiLineOptData[] = {
        // Row 0 is 12.63 mm and optimal flag is set => 12.36 mm
        { 0, 0, 0, 1236, CHECK_OPTIMAL, true },
        // Row 1 is 11.99 mm and optimal flag is NOT set
        { 1, 1, 0, 1199, CHECK_OPTIMAL, false },
    };

    TestParam aTestValues[] = {
        /* Checks that a document saved to ods with default rows does indeed
           have default row heights ( there was a problem where the optimal
           height was being calculated after import if no hard height )
        */
        { u"ods/alldefaultheights.ods", OUString(), SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        /* Checks the imported height of some multiline input, additionally checks
           that the optimal height flag is set ( or not )
        */
        { u"ods/multilineoptimal.ods", OUString(), SAL_N_ELEMENTS(MultiLineOptData),
          MultiLineOptData },
    };
    miscRowHeightsTest(aTestValues, SAL_N_ELEMENTS(aTestValues));
}

// regression test at least fdo#59193
// what we want to test here is that when cell contents are deleted
// and the optimal flag is set for that row that the row is actually resized

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testOptimalHeightReset)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    createScDoc("ods/multilineoptimal.ods");
    SCTAB nTab = 0;
    SCROW nRow = 0;
    ScDocument* pDoc = getScDoc();
    // open document in read/write mode ( otherwise optimal height stuff won't
    // be triggered ) *and* you can't delete cell contents.
    int nHeight = convertTwipToMm100(pDoc->GetRowHeight(nRow, nTab, false));
    CPPUNIT_ASSERT_EQUAL(1236, nHeight);

    ScDocShell* pDocSh = getScDocShell();
    ScDocFunc& rFunc = pDocSh->GetDocFunc();

    // delete content of A1
    ScRange aDelRange(0, 0, 0, 0, 0, 0);
    ScMarkData aMark(pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDelRange);
    bool bRet = rFunc.DeleteContents(aMark, InsertDeleteFlags::ALL, false, true);
    CPPUNIT_ASSERT_MESSAGE("DeleteContents failed", bRet);

    // get the new height of A1
    nHeight = convertTwipToMm100(pDoc->GetRowHeight(nRow, nTab, false));

    // set optimal height for empty row 2
    std::vector<sc::ColRowSpan> aRowArr(1, sc::ColRowSpan(2, 2));
    rFunc.SetWidthOrHeight(false, aRowArr, nTab, SC_SIZE_OPTIMAL, 0, true, true);

    // retrieve optimal height
    int nOptimalHeight = convertTwipToMm100(pDoc->GetRowHeight(aRowArr[0].mnStart, nTab, false));

    // check if the new height of A1 ( after delete ) is now the optimal height of an empty cell
    CPPUNIT_ASSERT_EQUAL(nOptimalHeight, nHeight);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf123026_optimalRowHeight)
{
    createScDoc("xlsx/tdf123026_optimalRowHeight.xlsx");
    SCTAB nTab = 0;
    SCROW nRow = 4;
    int nHeight = convertTwipToMm100(getScDoc()->GetRowHeight(nRow, nTab, false));

    // Without the fix, this was 529 (300 twip). It should be 3210.
    CPPUNIT_ASSERT_GREATER(2000, nHeight);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf159581_optimalRowHeight)
{
    createScDoc("xlsx/tdf159581_optimalRowHeight.xlsx");
    SCTAB nTab = 1;
    SCROW nRow = 0; // row 1
    int nHeight = convertTwipToMm100(getScDoc()->GetRowHeight(nRow, nTab, false));

    // Without the fix, this was 2027. It should be 450.
    CPPUNIT_ASSERT_LESS(500, nHeight);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testCustomNumFormatHybridCellODS)
{
    createScDoc("ods/custom-numfmt-hybrid-cell.ods");
    ScDocument* pDoc = getScDoc();
    pDoc->SetAutoCalc(true);

    // All of B14, B16 and B18 should be displaying empty strings by virtue
    // of the custom number format being set on those cells.

    for (SCROW nRow : { 13, 15, 17 })
    {
        ScAddress aPos(1, nRow, 0);
        OUString aStr = pDoc->GetString(aPos);
        CPPUNIT_ASSERT(aStr.isEmpty());
    }

    // Now, set value of 1 to B15.  This should trigger re-calc on B18 and B18
    // should now show a value of 1.
    pDoc->SetValue(ScAddress(1, 15, 0), 1.0);

    OUString aStr = pDoc->GetString(ScAddress(1, 17, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("1"), aStr);

    // Make sure the cell doesn't have an error value.
    ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(1, 17, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NONE, pFC->GetErrCode());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf121040)
{
    createScDoc("ods/tdf121040.ods");

    const SCTAB nTab = 0;
    ScDocument* pDoc = getScDoc();

    // The first 9 rows should have the same height
    const sal_uInt16 nHeight = pDoc->GetRowHeight(0, nTab, false);
    for (SCTAB nRow = 1; nRow < 9; nRow++)
    {
        CPPUNIT_ASSERT_EQUAL(nHeight, pDoc->GetRowHeight(nRow, nTab, false));
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf118086)
{
    createScDoc("ods/tdf118086.ods");

    ScDocument* pDoc = getScDoc();

    // Depending on DPI, this might be 477 or 480
    CPPUNIT_ASSERT_DOUBLES_EQUAL(477, pDoc->GetRowHeight(2, static_cast<SCTAB>(0), false), 5);

    // Without the fix in place, this test would have failed with
    // - Expected: 256
    // - Actual  : 477
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(256), pDoc->GetRowHeight(2, static_cast<SCTAB>(1), false));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(256), pDoc->GetRowHeight(2, static_cast<SCTAB>(2), false));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf118624)
{
    createScDoc("ods/tdf118624.ods");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_MESSAGE("RAND() in array/matrix mode shouldn't return the same value",
                           pDoc->GetString(ScAddress(0, 0, 0))
                               != pDoc->GetString(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf153767)
{
    createScDoc("xlsx/tdf153767.xlsx");

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: TRUE
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(OUString("TRUE"), pDoc->GetString(ScAddress(7, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("FALSE"), pDoc->GetString(ScAddress(7, 2, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf124454)
{
    createScDoc("ods/tdf124454.ods");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(ScAddress(2, 0, 0)));
    // Without the fix in place, double negation with text in array
    // would have returned -1
    CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(ScAddress(3, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testPrintRangeODS)
{
    createScDoc("ods/print-range.ods");
    ScDocument* pDoc = getScDoc();
    std::optional<ScRange> pRange = pDoc->GetRepeatRowRange(0);
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 0, 1, 0), *pRange);

    pRange = pDoc->GetRepeatRowRange(1);
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 2, 0, 0, 4, 0), *pRange);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testOutlineODS)
{
    createScDoc("ods/outline.ods");
    ScDocument* pDoc = getScDoc();

    const ScOutlineTable* pTable = pDoc->GetOutlineTable(0);
    CPPUNIT_ASSERT(pTable);

    const ScOutlineArray& rArr = pTable->GetRowArray();
    size_t nDepth = rArr.GetDepth();
    CPPUNIT_ASSERT_EQUAL(size_t(4), nDepth);

    for (size_t i = 0; i < nDepth; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(size_t(1), rArr.GetCount(i));
    }

    struct OutlineData
    {
        SCCOLROW nStart;
        SCCOLROW nEnd;
        bool bHidden;
        bool bVisible;

        size_t nDepth;
        size_t nIndex;
    };

    static const OutlineData aRow[] = { { 1, 29, false, true, 0, 0 },
                                        { 2, 26, false, true, 1, 0 },
                                        { 4, 23, false, true, 2, 0 },
                                        { 6, 20, true, true, 3, 0 } };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aRow); ++i)
    {
        const ScOutlineEntry* pEntry = rArr.GetEntry(aRow[i].nDepth, aRow[i].nIndex);
        SCCOLROW nStart = pEntry->GetStart();
        CPPUNIT_ASSERT_EQUAL(aRow[i].nStart, nStart);

        SCCOLROW nEnd = pEntry->GetEnd();
        CPPUNIT_ASSERT_EQUAL(aRow[i].nEnd, nEnd);

        bool bHidden = pEntry->IsHidden();
        CPPUNIT_ASSERT_EQUAL(aRow[i].bHidden, bHidden);

        bool bVisible = pEntry->IsVisible();
        CPPUNIT_ASSERT_EQUAL(aRow[i].bVisible, bVisible);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testColumnStyleXLSX)
{
    createScDoc("xlsx/column-style.xlsx");
    ScDocument* pDoc = getScDoc();

    const ScPatternAttr* pPattern = pDoc->GetPattern(0, 0, 0);
    CPPUNIT_ASSERT(pPattern);

    const ScProtectionAttr& rAttr = pPattern->GetItem(ATTR_PROTECTION);
    CPPUNIT_ASSERT(rAttr.GetProtection());

    pPattern = pDoc->GetPattern(0, 1, 0);
    CPPUNIT_ASSERT(pPattern);

    const ScProtectionAttr& rAttrNew = pPattern->GetItem(ATTR_PROTECTION);
    CPPUNIT_ASSERT(!rAttrNew.GetProtection());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testColumnStyleAutoFilterXLSX)
{
    createScDoc("xlsx/column-style-autofilter.xlsx");
    ScDocument* pDoc = getScDoc();

    const ScPatternAttr* pPattern = pDoc->GetPattern(0, 10, 18);
    CPPUNIT_ASSERT(pPattern);

    const ScMergeFlagAttr& rAttr = pPattern->GetItem(ATTR_MERGE_FLAG);
    CPPUNIT_ASSERT(!rAttr.HasAutoFilter());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testSharedFormulaHorizontalXLS)
{
    createScDoc("xls/shared-formula/horizontal.xls");
    ScDocument* pDoc = getScDoc();

    // Make sure K2:S2 on the 2nd sheet are all formula cells.
    ScAddress aPos(0, 1, 1);
    for (SCCOL nCol = 10; nCol <= 18; ++nCol)
    {
        aPos.SetCol(nCol);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell is expected here.", CELLTYPE_FORMULA,
                                     pDoc->GetCellType(aPos));
    }

    // Likewise, B3:J9 all should be formula cells.
    for (SCCOL nCol = 1; nCol <= 9; ++nCol)
    {
        aPos.SetCol(nCol);
        for (SCROW nRow = 2; nRow <= 8; ++nRow)
        {
            aPos.SetRow(nRow);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell is expected here.", CELLTYPE_FORMULA,
                                         pDoc->GetCellType(aPos));
        }
    }

    // B2:I2 too.
    aPos.SetRow(1);
    for (SCCOL nCol = 1; nCol <= 8; ++nCol)
    {
        aPos.SetCol(nCol);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell is expected here.", CELLTYPE_FORMULA,
                                     pDoc->GetCellType(aPos));
    }

    // J2 has a string of "MW".
    aPos.SetCol(9);
    CPPUNIT_ASSERT_EQUAL(OUString("MW"), pDoc->GetString(aPos));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testSharedFormulaWrappedRefsXLS)
{
    createScDoc("xls/shared-formula/wrapped-refs.xls");
    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();

    // Check the values of H7:H10.
    CPPUNIT_ASSERT_EQUAL(7.0, pDoc->GetValue(ScAddress(7, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(8.0, pDoc->GetValue(ScAddress(7, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(9.0, pDoc->GetValue(ScAddress(7, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(7, 9, 0)));

    // EM7:EM10 should reference H7:H10.
    CPPUNIT_ASSERT_EQUAL(7.0, pDoc->GetValue(ScAddress(142, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(8.0, pDoc->GetValue(ScAddress(142, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(9.0, pDoc->GetValue(ScAddress(142, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(142, 9, 0)));

    // Make sure EM7:EM10 are grouped.
    const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(142, 6, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testSharedFormulaBIFF5)
{
    createScDoc("xls/shared-formula/biff5.xls");
    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();

    // E6:E376 should be all formulas, and they should belong to the same group.
    const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(4, 5, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(5), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(371), pFC->GetSharedLength());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testSharedFormulaXLSB)
{
    createScDoc("xlsb/shared_formula.xlsb");
    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();

    // A1:A30 should be all formulas, and they should belong to the same group.
    const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(30), pFC->GetSharedLength());

    for (SCROW nRow = 0; nRow < 30; ++nRow)
    {
        ASSERT_DOUBLES_EQUAL(3.0, pDoc->GetValue(0, nRow, 0));
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testSharedFormulaXLS)
{
    {
        // fdo#80091
        createScDoc("xls/shared-formula/relative-refs1.xls");
        ScDocument* pDoc = getScDoc();
        pDoc->CalcAll();

        // A1:A30 should be all formulas, and they should belong to the same group.
        const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(0, 1, 0));
        CPPUNIT_ASSERT(pFC);
        CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
        CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(29), pFC->GetSharedLength());

        for (SCROW nRow = 0; nRow < 30; ++nRow)
        {
            ASSERT_DOUBLES_EQUAL(double(nRow + 1), pDoc->GetValue(0, nRow, 0));
        }
    }

    {
        // fdo#84556 and some related tests
        createScDoc("xls/shared-formula/relative-refs2.xls");
        ScDocument* pDoc = getScDoc();
        pDoc->CalcAll();

        {
            const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(2, 1, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            pFC = pDoc->GetFormulaCell(ScAddress(2, 10, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            OUString aFormula = pDoc->GetFormula(2, 1, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B9:D9)"), aFormula);

            aFormula = pDoc->GetFormula(2, 10, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B18:D18)"), aFormula);
        }

        {
            const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(4, 8, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            pFC = pDoc->GetFormulaCell(ScAddress(4, 17, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            OUString aFormula = pDoc->GetFormula(4, 8, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(G9:EY9)"), aFormula);

            aFormula = pDoc->GetFormula(4, 17, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(G18:EY18)"), aFormula);
        }

        {
            const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(6, 15, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(15), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            pFC = pDoc->GetFormulaCell(ScAddress(6, 24, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(15), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            OUString aFormula = pDoc->GetFormula(6, 15, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A16:A40000)"), aFormula);

            aFormula = pDoc->GetFormula(6, 24, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A25:A40009)"), aFormula);
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testSharedFormulaColumnLabelsODS)
{
    createScDoc("ods/shared-formula/column-labels.ods");

    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();

    CPPUNIT_ASSERT_EQUAL(5.0, pDoc->GetValue(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(15.0, pDoc->GetValue(ScAddress(2, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, pDoc->GetValue(ScAddress(2, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(28.0, pDoc->GetValue(ScAddress(2, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(48.0, pDoc->GetValue(ScAddress(2, 5, 0)));

    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(3, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(50.0, pDoc->GetValue(ScAddress(3, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(144.0, pDoc->GetValue(ScAddress(3, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(147.0, pDoc->GetValue(ScAddress(3, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(320.0, pDoc->GetValue(ScAddress(3, 5, 0)));

    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(4, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(4, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(5.0, pDoc->GetValue(ScAddress(4, 5, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testSharedFormulaColumnRowLabelsODS)
{
    createScDoc("ods/shared-formula/column-row-labels.ods");

    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();

    // Expected output in each of the three ranges.
    //
    // +---+---+---+
    // | 1 | 4 | 7 |
    // +---+---+---+
    // | 2 | 5 | 8 |
    // +---+---+---+
    // | 3 | 6 | 9 |
    // +---+---+---+

    auto aCheckFunc = [&](SCCOL nStartCol, SCROW nStartRow) {
        double fExpected = 1.0;
        for (SCCOL nCol = 0; nCol <= 2; ++nCol)
        {
            for (SCROW nRow = 0; nRow <= 2; ++nRow)
            {
                ScAddress aPos(nStartCol + nCol, nStartRow + nRow, 0);
                CPPUNIT_ASSERT_EQUAL(fExpected, pDoc->GetValue(aPos));
                fExpected += 1.0;
            }
        }
    };

    aCheckFunc(5, 1); // F2:H4
    aCheckFunc(9, 1); // J2:L4
    aCheckFunc(1, 6); // B7:D9
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testExternalRefCacheXLSX)
{
    createScDoc("xlsx/external-refs.xlsx");
    ScDocument* pDoc = getScDoc();

    // These string values are cached external cell values.
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Andy"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Bruce"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Charlie"), pDoc->GetString(ScAddress(0, 3, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testExternalRefCacheODS)
{
    createScDoc("ods/external-ref-cache.ods");

    ScDocument* pDoc = getScDoc();

    // Cells B2:B4 have VLOOKUP with external references which should all show "text".
    CPPUNIT_ASSERT_EQUAL(OUString("text"), pDoc->GetString(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("text"), pDoc->GetString(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("text"), pDoc->GetString(ScAddress(1, 3, 0)));

    // Both cells A6 and A7 should be registered with scExternalRefManager properly
    CPPUNIT_ASSERT_EQUAL(
        true, pDoc->GetExternalRefManager()->hasCellExternalReference(ScAddress(0, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(
        true, pDoc->GetExternalRefManager()->hasCellExternalReference(ScAddress(0, 6, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testHybridSharedStringODS)
{
    createScDoc("ods/hybrid-shared-string.ods");

    ScDocument* pDoc = getScDoc();

    // A2 contains formula with MATCH function.  The result must be 2, not #N/A!
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testCopyMergedNumberFormats)
{
    createScDoc("ods/copy-merged-number-formats.ods");
    ScDocument* pDoc = getScDoc();

    // Cells B1, C1 and D1 are formatted as dates.
    OUString aStrB1 = pDoc->GetString(ScAddress(1, 0, 0));
    OUString aStrC1 = pDoc->GetString(ScAddress(2, 0, 0));
    OUString aStrD1 = pDoc->GetString(ScAddress(3, 0, 0));

    ScDocument aCopyDoc;
    aCopyDoc.InsertTab(0, "CopyHere");
    pDoc->CopyStaticToDocument(ScRange(1, 0, 0, 3, 0, 0), 0, aCopyDoc);

    // Make sure the date formats are copied to the new document.
    CPPUNIT_ASSERT_EQUAL(aStrB1, aCopyDoc.GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(aStrC1, aCopyDoc.GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(aStrD1, aCopyDoc.GetString(ScAddress(3, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testVBAUserFunctionXLSM)
{
    createScDoc("xlsm/vba-user-function.xlsm");
    ScDocument* pDoc = getScDoc();

    // A1 contains formula with user-defined function, and the function is defined in VBA.
    ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pFC);

    sc::CompileFormulaContext aCxt(*pDoc);
    OUString aFormula = pFC->GetFormula(aCxt);

    CPPUNIT_ASSERT_EQUAL(OUString("=MYFUNC()"), aFormula);

    // Check the formula state after the load.
    FormulaError nErrCode = pFC->GetErrCode();
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(nErrCode));

    // Check the result.
    CPPUNIT_ASSERT_EQUAL(42.0, pDoc->GetValue(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testEmbeddedImageXLS)
{
    // The document has one embedded image on the first sheet.  Make sure it's
    // imported properly.

    createScDoc("xls/file-with-png-image.xls");
    ScDocument* pDoc = getScDoc();

    ScDrawLayer* pDL = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDL);
    const SdrPage* pPage = pDL->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    const SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);
    const SdrGrafObj* pImageObj = dynamic_cast<const SdrGrafObj*>(pObj);
    CPPUNIT_ASSERT(pImageObj);
    const Graphic& rGrf = pImageObj->GetGraphic();
    BitmapEx aBMP = rGrf.GetBitmapEx();
    CPPUNIT_ASSERT_MESSAGE(
        "Bitmap content should not be empty if the image has been properly imported.",
        !aBMP.IsEmpty());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testErrorOnExternalReferences)
{
    createScDoc();

    ScDocument* pDoc = getScDoc();

    // Test tdf#89330
    pDoc->SetString(ScAddress(0, 0, 0), "='file:///Path/To/FileA.ods'#$Sheet1.A1A");

    ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(int(FormulaError::NoName), static_cast<int>(pFC->GetErrCode()));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula changed",
                                 OUString("='file:///Path/To/FileA.ods'#$Sheet1.A1A"),
                                 pDoc->GetFormula(0, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf145054)
{
    createScDoc("xlsx/tdf145054.xlsx");

    ScDocument* pDoc = getScDoc();

    // Copy sheet
    pDoc->CopyTab(0, 1);
    CPPUNIT_ASSERT_EQUAL(SCTAB(2), pDoc->GetTableCount());

    // Make sure named DB was copied
    ScDBData* pDBData
        = pDoc->GetDBCollection()->getNamedDBs().findByName("__Anonymous_Sheet_DB__1");
    CPPUNIT_ASSERT(pDBData);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf84762)
{
    createScDoc();

    ScDocument* pDoc = getScDoc();

    pDoc->SetString(ScAddress(0, 0, 0), "=RAND()");
    pDoc->SetString(ScAddress(0, 1, 0), "=RAND()");
    pDoc->SetString(ScAddress(1, 0, 0), "=RAND()*A1");
    pDoc->SetString(ScAddress(1, 1, 0), "=RAND()*B1");

    double nValA1, nValB1, nValA2, nValB2;

    ScDocShell* pDocSh = getScDocShell();

    // Without the fix in place, some cells wouldn't have been updated
    // after using F9 a few times
    for (sal_Int16 i = 0; i < 10; ++i)
    {
        nValA1 = pDoc->GetValue(0, 0, 0);
        nValB1 = pDoc->GetValue(0, 1, 0);
        nValA2 = pDoc->GetValue(1, 0, 0);
        nValB2 = pDoc->GetValue(1, 1, 0);

        pDocSh->DoRecalc(false);

        CPPUNIT_ASSERT(nValA1 != pDoc->GetValue(0, 0, 0));
        CPPUNIT_ASSERT(nValA2 != pDoc->GetValue(0, 1, 0));
        CPPUNIT_ASSERT(nValB1 != pDoc->GetValue(1, 0, 0));
        CPPUNIT_ASSERT(nValB2 != pDoc->GetValue(1, 1, 0));
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf44076)
{
    createScDoc();

    ScDocument* pDoc = getScDoc();

    pDoc->SetString(ScAddress(0, 0, 0), "=(-8)^(1/3)");

    CPPUNIT_ASSERT_EQUAL(-2.0, pDoc->GetValue(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testEditEngStrikeThroughXLSX)
{
    createScDoc("xlsx/strike-through.xlsx");

    ScDocument* pDoc = getScDoc();

    const EditTextObject* pObj = pDoc->GetEditText(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pObj);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pObj->GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(OUString("this is strike through  this not"), pObj->GetText(0));

    std::vector<EECharAttrib> aAttribs;
    pObj->GetCharAttribs(0, aAttribs);
    for (const auto& rAttrib : aAttribs)
    {
        if (rAttrib.pAttr->Which() == EE_CHAR_STRIKEOUT)
        {
            const SvxCrossedOutItem& rItem = static_cast<const SvxCrossedOutItem&>(*rAttrib.pAttr);
            if (rAttrib.nStart == 0)
            {
                CPPUNIT_ASSERT(rItem.GetStrikeout() != STRIKEOUT_NONE);
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL(STRIKEOUT_NONE, rItem.GetStrikeout());
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testRefStringXLSX)
{
    createScDoc("xlsx/ref_string.xlsx");

    ScDocument* pDoc = getScDoc();

    double nVal = pDoc->GetValue(2, 2, 0);
    ASSERT_DOUBLES_EQUAL(3.0, nVal);

    const ScCalcConfig& rCalcConfig = pDoc->GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL(formula::FormulaGrammar::CONV_XL_A1, rCalcConfig.meStringRefAddressSyntax);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf130132)
{
    createScDoc("ods/tdf130132.ods");

    ScDocument* pDoc = getScDoc();
    const ScPatternAttr* pAttr = pDoc->GetPattern(434, 0, 0);

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_BACKGROUND);
        const SvxBrushItem& rBackground = static_cast<const SvxBrushItem&>(rItem);
        const Color& rColor = rBackground.GetColor();
        // background colour is yellow
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, rColor);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf133327)
{
    createScDoc("ods/tdf133327.ods");

    ScDocument* pDoc = getScDoc();

    const ScPatternAttr* pAttr = pDoc->GetPattern(250, 1, 0);

    const SfxPoolItem& rItem = pAttr->GetItem(ATTR_BACKGROUND);
    const SvxBrushItem& rBackground = static_cast<const SvxBrushItem&>(rItem);
    const Color& rColor = rBackground.GetColor();

    // Without the fix in place, this test would have failed with
    // - Expected: Color: R:255 G:255 B: 0
    // - Actual  : Color: R:255 G:255 B: 255
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, rColor);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testColumnStyle2XLSX)
{
    createScDoc("xlsx/column_style.xlsx");

    ScDocument* pDoc = getScDoc();
    const ScPatternAttr* pAttr = pDoc->GetPattern(1, 1, 0);

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_BACKGROUND);
        const SvxBrushItem& rBackground = static_cast<const SvxBrushItem&>(rItem);
        const Color& rColor = rBackground.GetColor();
        CPPUNIT_ASSERT_EQUAL(Color(255, 51, 51), rColor);
    }

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_HOR_JUSTIFY);
        const SvxHorJustifyItem& rJustify = static_cast<const SvxHorJustifyItem&>(rItem);
        CPPUNIT_ASSERT_EQUAL(SvxCellHorJustify::Center, rJustify.GetValue());
    }

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_FONT_HEIGHT);
        const SvxFontHeightItem& rFontHeight = static_cast<const SvxFontHeightItem&>(rItem);
        sal_uInt16 nHeight = rFontHeight.GetHeight();
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(240), nHeight);
    }

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_FONT);
        const SvxFontItem& rFont = static_cast<const SvxFontItem&>(rItem);
        OUString aName = rFont.GetFamilyName();
        CPPUNIT_ASSERT_EQUAL(OUString("Linux Biolinum G"), aName);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf110440XLSX)
{
    createScDoc("xlsx/tdf110440.xlsx");

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 uno::UNO_QUERY_THROW);
    xIA.set(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xIA->getByIndex(0), uno::UNO_QUERY_THROW);
    bool bVisible = true;
    xShape->getPropertyValue("Visible") >>= bVisible;
    // This failed: group shape's hidden property was lost on import.
    CPPUNIT_ASSERT(!bVisible);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testBnc762542)
{
    createScDoc("xlsx/bnc762542.xlsx");

    ScDocument* pDoc = getScDoc();
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    const size_t nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 10 shapes.", static_cast<size_t>(10), nCount);

    // previously, some of the shapes were (incorrectly) rotated by 90 degrees
    for (size_t i : { 1, 2, 4, 5, 7, 9 })
    {
        SdrObject* pObj = pPage->GetObj(i);
        CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);

        tools::Rectangle aRect(pObj->GetCurrentBoundRect());
        CPPUNIT_ASSERT_MESSAGE("Drawing object shouldn't be rotated.",
                               aRect.GetWidth() > aRect.GetHeight());
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testHiddenSheetsXLSX)
{
    createScDoc("xlsx/hidden_sheets.xlsx");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("1st sheet should be hidden", false, pDoc->IsVisible(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd sheet should be visible", true, pDoc->IsVisible(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("3rd sheet should be hidden", false, pDoc->IsVisible(2));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testAutofilterXLSX)
{
    createScDoc("xlsx/autofilter.xlsx");

    ScDocument* pDoc = getScDoc();
    const ScDBData* pData = pDoc->GetDBCollection()->GetDBNearCursor(0, 0, 0);
    CPPUNIT_ASSERT(pData);
    ScRange aRange;
    pData->GetArea(aRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 4, 0), aRange);
}

namespace
{
void checkValidationFormula(const ScAddress& rPos, const ScDocument& rDoc,
                            const OUString& rExpectedFormula)
{
    const SfxUInt32Item* pItem = rDoc.GetAttr(rPos, ATTR_VALIDDATA);
    CPPUNIT_ASSERT(pItem);
    sal_uInt32 nKey = pItem->GetValue();
    const ScValidationData* pData = rDoc.GetValidationEntry(nKey);
    CPPUNIT_ASSERT(pData);

    OUString aFormula = pData->GetExpression(rPos, 0);
    CPPUNIT_ASSERT_EQUAL(rExpectedFormula, aFormula);
}
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testRelFormulaValidationXLS)
{
    createScDoc("xls/validation.xls");

    ScDocument* pDoc = getScDoc();

    checkValidationFormula(ScAddress(3, 4, 0), *pDoc, "C5");
    checkValidationFormula(ScAddress(5, 8, 0), *pDoc, "D7");
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf136364)
{
    createScDoc("xlsx/tdf136364.xlsx");

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, it would have failed with
    // - Expected: =SUM((B2:B3~C4:C5~D6:D7))
    // - Actual  : =SUM((B2:B3~C4:C5,D6:D7))
    OUString aFormula = pDoc->GetFormula(4, 0, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM((B2:B3~C4:C5~D6:D7))"), aFormula);
    CPPUNIT_ASSERT_EQUAL(27.0, pDoc->GetValue(ScAddress(4, 0, 0)));

    // - Expected: =SUM((B2~C4~D6))
    // - Actual  : =SUM((B2~C4,D6))
    aFormula = pDoc->GetFormula(4, 1, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM((B2~C4~D6))"), aFormula);
    CPPUNIT_ASSERT_EQUAL(12.0, pDoc->GetValue(ScAddress(4, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf103734)
{
    createScDoc("ods/tdf103734.ods");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, MAX() would have returned -1.8E+308
    CPPUNIT_ASSERT_EQUAL(OUString("#N/A"), pDoc->GetString(ScAddress(2, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf126116)
{
    createScDoc("fods/tdf126116.fods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("02/02/21"), pDoc->GetString(ScAddress(0, 0, 0)));

    pDoc->SetString(ScAddress(0, 0, 0), "03/03");

    sal_uInt32 nNumberFormat = pDoc->GetNumberFormat(0, 0, 0);
    const SvNumberformat* pNumberFormat = pDoc->GetFormatTable()->GetEntry(nNumberFormat);
    const OUString& rFormatStr = pNumberFormat->GetFormatstring();

    // Without the fix in place, this test would have failed with
    // - Expected: MM/DD/YY
    // - Actual  : MM/DD/YYYY
    CPPUNIT_ASSERT_EQUAL(OUString("MM/DD/YY"), rFormatStr);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf144209)
{
    createScDoc("ods/tdf144209.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("AA 0"), pDoc->GetString(ScAddress(0, 0, 0)));

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    // Without the fix in place, this test would have failed with
    // - Expected: AA 33263342642.5385
    // - Actual  : AA 0
    CPPUNIT_ASSERT_EQUAL(OUString("AA 33263342642.5385"), pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf98844)
{
    createScDoc("ods/tdf98844.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(47.6227, pDoc->GetValue(ScAddress(0, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(48.0, pDoc->GetValue(ScAddress(0, 8, 0)));

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    // Without the fix in place, SUM() wouldn't have been updated when
    // Precision as shown is enabled
    CPPUNIT_ASSERT_EQUAL(48.0, pDoc->GetValue(ScAddress(0, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(48.0, pDoc->GetValue(ScAddress(0, 8, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf100458)
{
    createScDoc("ods/tdf100458_lost_zero_value.ods");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc->HasValueData(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT(!pDoc->HasStringData(0, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf118561)
{
    createScDoc("ods/tdf118561.ods");
    ScDocument* pDoc = getScDoc();

    //Without the fix in place, it would have failed with
    //- Expected: apple
    //- Actual  : Err:502
    CPPUNIT_ASSERT_EQUAL(OUString("apple"), pDoc->GetString(ScAddress(1, 1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("apple"), pDoc->GetString(ScAddress(2, 1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("TRUE"), pDoc->GetString(ScAddress(3, 1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("fruits"), pDoc->GetString(ScAddress(4, 1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("apple"), pDoc->GetString(ScAddress(5, 1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("hat"), pDoc->GetString(ScAddress(6, 1, 1)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf125099)
{
    createScDoc("ods/tdf125099.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("03:53:46"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("03:23:59"), pDoc->GetString(ScAddress(0, 1, 0)));

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_EQUAL(OUString("03:53:46"), pDoc->GetString(ScAddress(0, 0, 0)));

    // Without the fix in place, this would have failed with
    // - Expected: 03:24:00
    // - Actual  : 03:23:59
    CPPUNIT_ASSERT_EQUAL(OUString("03:24:00"), pDoc->GetString(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf134455)
{
    createScDoc("xlsx/tdf134455.xlsx");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("00:05"), pDoc->GetString(ScAddress(3, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("00:10"), pDoc->GetString(ScAddress(3, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("00:59"), pDoc->GetString(ScAddress(3, 6, 0)));

    // Without the fix in place, TIMEVALUE would have returned Err:502 for values
    // greater than 59
    CPPUNIT_ASSERT_EQUAL(OUString("01:05"), pDoc->GetString(ScAddress(3, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("04:00"), pDoc->GetString(ScAddress(3, 8, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf119533)
{
    createScDoc("ods/tdf119533.ods");
    ScDocument* pDoc = getScDoc();

    // Without fix in place, this test would have failed with
    // - Expected: 0.5
    // - Actual  : 0.483333333333333
    CPPUNIT_ASSERT_EQUAL(OUString("0.5"), pDoc->GetString(ScAddress(4, 0, 0)));

    // Without fix in place, this test would have failed with
    // - Expected: 9.5
    // - Actual  : 9.51666666666667
    CPPUNIT_ASSERT_EQUAL(OUString("9.5"), pDoc->GetString(ScAddress(5, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf127982)
{
    createScDoc("ods/tdf127982.ods");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, these cells would be empty
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pDoc->GetString(ScAddress(3, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R6"), pDoc->GetString(ScAddress(3, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R7"), pDoc->GetString(ScAddress(3, 7, 0)));

    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pDoc->GetString(ScAddress(4, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R6"), pDoc->GetString(ScAddress(4, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R7"), pDoc->GetString(ScAddress(4, 7, 0)));

    // Without the fix in place, these cells would be empty
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pDoc->GetString(ScAddress(4, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R6"), pDoc->GetString(ScAddress(4, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R7"), pDoc->GetString(ScAddress(4, 7, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf109409)
{
    createScDoc("ods/tdf109409.ods");
    ScDocument* pDoc = getScDoc();

    // TEXTJOIN
    CPPUNIT_ASSERT_EQUAL(OUString("A1;B1;A2;B2;A3;B3"), pDoc->GetString(ScAddress(3, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("A1;B1;A2;B2;A3;B3"), pDoc->GetString(ScAddress(3, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("A1;A2;A3;B1;B2;B3"), pDoc->GetString(ScAddress(3, 4, 0)));

    // Without the fix in place, it would have failed with
    //- Expected: A1;B1;A2;B2;A3;B3
    //- Actual  : A1;A2;A3;B1;B2;B3
    CPPUNIT_ASSERT_EQUAL(OUString("A1;B1;A2;B2;A3;B3"), pDoc->GetString(ScAddress(3, 5, 0)));

    // CONCAT
    CPPUNIT_ASSERT_EQUAL(OUString("A1B1A2B2A3B3"), pDoc->GetString(ScAddress(6, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("A1B1A2B2A3B3"), pDoc->GetString(ScAddress(6, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("A1A2A3B1B2B3"), pDoc->GetString(ScAddress(6, 4, 0)));

    // Without the fix in place, it would have failed with
    //- Expected: A1B1A2B2A3B3
    //- Actual  : A1A2A3B1B2B3
    CPPUNIT_ASSERT_EQUAL(OUString("A1B1A2B2A3B3"), pDoc->GetString(ScAddress(6, 5, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf132105)
{
    createScDoc("ods/tdf132105.ods");
    ScDocument* pDoc = getScDoc();

    // MATCH
    CPPUNIT_ASSERT_EQUAL(OUString("5"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("5"), pDoc->GetString(ScAddress(1, 1, 0)));

    // COUNT
    CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("20"), pDoc->GetString(ScAddress(1, 2, 0)));

    // COUNTA
    CPPUNIT_ASSERT_EQUAL(OUString("20"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("20"), pDoc->GetString(ScAddress(1, 3, 0)));

    // COUNTBLANK
    // Without the fix in place, it would have failed with
    // - Expected: 0
    //- Actual  : Err:504
    CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(0, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(1, 4, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf131424)
{
    createScDoc("xlsx/tdf131424.xlsx");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, table reference would have failed
    CPPUNIT_ASSERT_EQUAL(35.0, pDoc->GetValue(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(58.0, pDoc->GetValue(ScAddress(2, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(81.0, pDoc->GetValue(ScAddress(2, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(104.0, pDoc->GetValue(ScAddress(2, 4, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf100709XLSX)
{
    createScDoc("xlsx/tdf100709.xlsx");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B52 should not be formatted with a $", OUString("218"),
                                 pDoc->GetString(1, 51, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell A75 should not be formatted as a date", OUString("218"),
                                 pDoc->GetString(0, 74, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf97598XLSX)
{
    createScDoc("xlsx/tdf97598_scenarios.xlsx");

    ScDocument* pDoc = getScDoc();
    OUString aStr = pDoc->GetString(0, 0, 0); // A1
    CPPUNIT_ASSERT_EQUAL(OUString("Cell A1"), aStr);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf83672XLSX)
{
    createScDoc("xlsx/tdf83672.xlsx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    sal_Int32 nRotate = 0;
    xShapeProperties->getPropertyValue("RotateAngle") >>= nRotate;
    CPPUNIT_ASSERT(nRotate != 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testUnicodeFileNameGnumeric)
{
    // Mapping the LO-internal URL
    // <file:///.../sc/qa/unit/data/gnumeric/t%C3%A4%C3%9Ft.gnumeric> to the
    // repo's file sc/qa/unit/data/gnumeric/t\303\244\303\237t.gnumeric only
    // works when the system encoding is UTF-8:
    if (osl_getThreadTextEncoding() != RTL_TEXTENCODING_UTF8)
    {
        return;
    }
    loadFromFile(u"gnumeric/t\u00E4\u00DFt.gnumeric");
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testMergedCellsXLSXML)
{
    createScDoc("xml/merged-cells.xml");
    ScDocument* pDoc = getScDoc();

    // B1:C1 is merged.
    ScRange aMergedRange(1, 0, 0); // B1
    pDoc->ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 0, 0, 2, 0, 0), aMergedRange);

    // D1:F1 is merged.
    aMergedRange = ScRange(3, 0, 0); // D1
    pDoc->ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(3, 0, 0, 5, 0, 0), aMergedRange);

    // A2:A3 is merged.
    aMergedRange = ScRange(0, 1, 0); // A2
    pDoc->ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 1, 0, 0, 2, 0), aMergedRange);

    // A4:A6 is merged.
    aMergedRange = ScRange(0, 3, 0); // A4
    pDoc->ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 3, 0, 0, 5, 0), aMergedRange);

    // C3:F6 is merged.
    aMergedRange = ScRange(2, 2, 0); // C3
    pDoc->ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(2, 2, 0, 5, 5, 0), aMergedRange);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testBackgroundColorStandardXLSXML)
{
    createScDoc("xml/background-color-standard.xml");
    ScDocument* pDoc = getScDoc();

    struct Check
    {
        OUString aCellValue;
        Color aFontColor;
        Color aBgColor;
    };

    const std::vector<Check> aChecks = {
        { OUString("Background Color"), COL_BLACK, COL_TRANSPARENT },
        { OUString("Dark Red"), COL_WHITE, Color(192, 0, 0) },
        { OUString("Red"), COL_WHITE, COL_LIGHTRED },
        { OUString("Orange"), COL_WHITE, Color(255, 192, 0) },
        { OUString("Yellow"), COL_WHITE, COL_YELLOW },
        { OUString("Light Green"), COL_WHITE, Color(146, 208, 80) },
        { OUString("Green"), COL_WHITE, Color(0, 176, 80) },
        { OUString("Light Blue"), COL_WHITE, Color(0, 176, 240) },
        { OUString("Blue"), COL_WHITE, Color(0, 112, 192) },
        { OUString("Dark Blue"), COL_WHITE, Color(0, 32, 96) },
        { OUString("Purple"), COL_WHITE, Color(112, 48, 160) },
    };

    for (size_t nRow = 0; nRow < aChecks.size(); ++nRow)
    {
        ScAddress aPos(0, nRow, 0);
        OUString aStr = pDoc->GetString(aPos);
        CPPUNIT_ASSERT_EQUAL(aChecks[nRow].aCellValue, aStr);

        const ScPatternAttr* pPat = pDoc->GetPattern(aPos);
        CPPUNIT_ASSERT(pPat);

        const SvxColorItem& rColor = pPat->GetItem(ATTR_FONT_COLOR);
        CPPUNIT_ASSERT_EQUAL(aChecks[nRow].aFontColor, rColor.GetValue());

        const SvxBrushItem& rBgColor = pPat->GetItem(ATTR_BACKGROUND);
        CPPUNIT_ASSERT_EQUAL(aChecks[nRow].aBgColor, rBgColor.GetColor());
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf131536)
{
    createScDoc("xlsx/tdf131536.xlsx");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(3, 9, 0));
    CPPUNIT_ASSERT_EQUAL(
        OUString(
            "=IF(D$4=\"-\",\"-\",MID(TEXT(INDEX($Comparison.$I:$J,$Comparison.$A5,$Comparison.D$2),"
            "\"\")"
            ",2,4)"
            "=RIGHT(TEXT(INDEX($Comparison.$L:$Z,$Comparison.$A5,$Comparison.D$4),\"\"),4))"),
        pDoc->GetFormula(3, 9, 0));

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(4, 9, 0));
    CPPUNIT_ASSERT_EQUAL(
        OUString(
            "=IF(D$4=\"-\",\"-\",MID(TEXT(INDEX($Comparison.$I:$J,$Comparison.$A5,$Comparison.D$2),"
            "\"0\"),2,4)"
            "=RIGHT(TEXT(INDEX($Comparison.$L:$Z,$Comparison.$A5,$Comparison.D$4),\"0\"),4))"),
        pDoc->GetFormula(4, 9, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf130583)
{
    createScDoc("ods/tdf130583.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("b"), pDoc->GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("c"), pDoc->GetString(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("a"), pDoc->GetString(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("d"), pDoc->GetString(ScAddress(1, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#N/A"), pDoc->GetString(ScAddress(1, 4, 0)));

    // Without the fix in place, SWITCH would have returned #VALUE!
    CPPUNIT_ASSERT_EQUAL(OUString("b"), pDoc->GetString(ScAddress(4, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("c"), pDoc->GetString(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("a"), pDoc->GetString(ScAddress(4, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("d"), pDoc->GetString(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#N/A"), pDoc->GetString(ScAddress(4, 4, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf85617)
{
    createScDoc("xlsx/tdf85617.xlsx");
    ScDocument* pDoc = getScDoc();

    ScAddress aPos(2, 2, 0);
    //Without the fix in place, it would be Err:509
    CPPUNIT_ASSERT_EQUAL(4.5, pDoc->GetValue(aPos));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf134234)
{
    createScDoc("ods/tdf134234.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(1, 0, 1)));

    //Without the fix in place, SUMPRODUCT would have returned 0
    CPPUNIT_ASSERT_EQUAL(36.54, pDoc->GetValue(ScAddress(2, 0, 1)));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(833),
                         static_cast<sal_uInt32>(pDoc->GetValue(ScAddress(3, 0, 1))));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testTdf42481)
{
    createScDoc("ods/tdf42481.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), pDoc->GetString(ScAddress(3, 9, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected: #VALUE!
    // - Actual  : 14
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), pDoc->GetString(ScAddress(3, 10, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("14"), pDoc->GetString(ScAddress(3, 11, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testNamedExpressionsXLSXML)
{
    {
        // global named expressions

        createScDoc("xml/named-exp-global.xml");
        ScDocument* pDoc = getScDoc();

        // A7
        ScAddress aPos(0, 6, 0);
        CPPUNIT_ASSERT_EQUAL(15.0, pDoc->GetValue(aPos));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(MyRange)"),
                             pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

        // B7
        aPos.IncCol();
        CPPUNIT_ASSERT_EQUAL(55.0, pDoc->GetValue(aPos));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(MyRange2)"),
                             pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

        const ScRangeData* pRD = pDoc->GetRangeName()->findByUpperName("MYRANGE");
        CPPUNIT_ASSERT(pRD);
        pRD = pDoc->GetRangeName()->findByUpperName("MYRANGE2");
        CPPUNIT_ASSERT(pRD);
    }

    {
        // sheet-local named expressions

        createScDoc("xml/named-exp-local.xml");
        ScDocument* pDoc = getScDoc();

        // A7 on Sheet1
        ScAddress aPos(0, 6, 0);
        CPPUNIT_ASSERT_EQUAL(27.0, pDoc->GetValue(aPos));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(MyRange)"),
                             pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

        // A7 on Sheet2
        aPos.IncTab();
        CPPUNIT_ASSERT_EQUAL(74.0, pDoc->GetValue(aPos));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(MyRange)"),
                             pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

        const ScRangeName* pRN = pDoc->GetRangeName(0);
        CPPUNIT_ASSERT(pRN);
        const ScRangeData* pRD = pRN->findByUpperName("MYRANGE");
        CPPUNIT_ASSERT(pRD);
        pRN = pDoc->GetRangeName(1);
        CPPUNIT_ASSERT(pRN);
        pRD = pRN->findByUpperName("MYRANGE");
        CPPUNIT_ASSERT(pRD);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testEmptyRowsXLSXML)
{
    createScDoc("xml/empty-rows.xml");
    ScDocument* pDoc = getScDoc();

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "Top row, followed by 2 empty rows.", nullptr },
            { nullptr, nullptr },
            { nullptr, nullptr },
            { nullptr, "1" },
            { nullptr, "2" },
            { nullptr, "3" },
            { nullptr, "4" },
            { nullptr, "5" },
            { nullptr, "15" },
        };

        ScRange aDataRange;
        aDataRange.Parse("A1:B9", *pDoc);
        bool bSuccess = checkOutput(pDoc, aDataRange, aOutputCheck, "Expected output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    ScAddress aPos;
    aPos.Parse("B9", *pDoc);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B4:B8)"),
                         pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testBorderDirectionsXLSXML)
{
    createScDoc("xml/border-directions.xml");
    ScDocument* pDoc = getScDoc();

    struct Check
    {
        ScAddress aPos;
        bool bTop;
        bool bBottom;
        bool bLeft;
        bool bRight;
        bool bTLtoBR;
        bool bTRtoBL;
    };

    std::vector<Check> aChecks = {
        { { 1, 1, 0 }, true, false, false, false, false, false }, // B2 - top
        { { 1, 3, 0 }, false, false, true, false, false, false }, // B4 - left
        { { 1, 5, 0 }, false, false, false, true, false, false }, // B6 - right
        { { 1, 7, 0 }, false, true, false, false, false, false }, // B8 - bottom
        { { 1, 9, 0 }, false, false, false, false, true, false }, // B10 - tl to br
        { { 1, 11, 0 }, false, false, false, false, false, true }, // B12 - tr to bl
        { { 1, 13, 0 }, false, false, false, false, true, true }, // B14 - cross-diagonal
    };

    auto funcCheckBorder = [](bool bHasBorder, const editeng::SvxBorderLine* pLine) -> bool {
        if (bHasBorder)
        {
            if (!pLine)
            {
                std::cout << "Border was expected, but not found!" << std::endl;
                return false;
            }

            if (SvxBorderLineStyle::SOLID != pLine->GetBorderLineStyle())
            {
                std::cout << "Border type was expected to be of SOLID, but is not." << std::endl;
                return false;
            }

            if (COL_BLACK != pLine->GetColor())
            {
                std::cout << "Border color was expected to be black, but is not." << std::endl;
                return false;
            }
        }
        else
        {
            if (pLine)
            {
                std::cout << "Border was not expected, but is found!" << std::endl;
                return false;
            }
        }

        return true;
    };

    for (const Check& c : aChecks)
    {
        const ScPatternAttr* pPat = pDoc->GetPattern(c.aPos);
        CPPUNIT_ASSERT(pPat);

        const SvxBoxItem& rBox = pPat->GetItem(ATTR_BORDER);

        const editeng::SvxBorderLine* pLine = rBox.GetTop();
        CPPUNIT_ASSERT(funcCheckBorder(c.bTop, pLine));

        pLine = rBox.GetBottom();
        CPPUNIT_ASSERT(funcCheckBorder(c.bBottom, pLine));

        pLine = rBox.GetLeft();
        CPPUNIT_ASSERT(funcCheckBorder(c.bLeft, pLine));

        pLine = rBox.GetRight();
        CPPUNIT_ASSERT(funcCheckBorder(c.bRight, pLine));

        pLine = pPat->GetItem(ATTR_BORDER_TLBR).GetLine();
        CPPUNIT_ASSERT(funcCheckBorder(c.bTLtoBR, pLine));

        pLine = pPat->GetItem(ATTR_BORDER_BLTR).GetLine();
        CPPUNIT_ASSERT(funcCheckBorder(c.bTRtoBL, pLine));
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testNamedTableRef)
{
    createScDoc("xlsx/tablerefsnamed.xlsx");
    ScDocument* pDoc = getScDoc();
    for (sal_Int32 nRow = 1; nRow < 7; ++nRow)
    {
        ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(5, nRow, 0));
        CPPUNIT_ASSERT(pFC);
        // Without the fix there will be #REF in F2:F7.
        CPPUNIT_ASSERT_EQUAL(FormulaError::NONE, pFC->GetErrCode());
        // Without the fix value will be 0 (FALSE).
        CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(6, nRow, 0)));
    }
}

namespace
{
void testCells(ScDocument* pDoc)
{
    {
        const EditTextObject* pObj = pDoc->GetEditText(ScAddress(0, 0, 0));
        CPPUNIT_ASSERT(pObj);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pObj->GetParagraphCount());
        CPPUNIT_ASSERT_EQUAL(size_t(1), pObj->GetSharedStrings().size());
    }

    {
        const EditTextObject* pObj = pDoc->GetEditText(ScAddress(0, 1, 0));
        CPPUNIT_ASSERT(pObj);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pObj->GetParagraphCount());
        CPPUNIT_ASSERT_EQUAL(size_t(3), pObj->GetSharedStrings().size());
    }
}
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testSingleLine)
{
    createScDoc("xls/cell-multi-line.xls");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);
    testCells(pDoc);

    createScDoc("xlsx/cell-multi-line.xlsx");
    pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);
    testCells(pDoc);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest2, testBackColorFilter)
{
    Color aBackColor1(0xc99c00);
    Color aBackColor2(0x0369a3);

    createScDoc();
    ScDocument* pDoc = getScDoc();

    ScPatternAttr aPattern1(pDoc->getCellAttributeHelper());
    aPattern1.GetItemSet().Put(SvxBrushItem(aBackColor1, ATTR_BACKGROUND));

    ScPatternAttr aPattern2(pDoc->getCellAttributeHelper());
    aPattern2.GetItemSet().Put(SvxBrushItem(aBackColor2, ATTR_BACKGROUND));

    // Apply the pattern to cell A1:A2
    pDoc->ApplyPatternAreaTab(0, 0, 0, 1, 0, aPattern1);

    // Apply the pattern to cell A3:A5
    pDoc->ApplyPatternAreaTab(0, 2, 0, 4, 0, aPattern2);

    {
        ScRefCellValue aCell;
        aCell.assign(*pDoc, ScAddress(0, 0, 0));
        CPPUNIT_ASSERT_MESSAGE("Cell A1 should be empty.", aCell.isEmpty());
        aCell.assign(*pDoc, ScAddress(0, 2, 0));
        CPPUNIT_ASSERT_MESSAGE("Cell A3 should be empty.", aCell.isEmpty());
    }

    {
        ScFilterEntries aFilterEntries;
        pDoc->GetFilterEntriesArea(0, 0, 4, 0, true, aFilterEntries);
        CPPUNIT_ASSERT_EQUAL(size_t(2), aFilterEntries.getBackgroundColors().size());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
