/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/sorthelper.hxx"
#include "helper/debughelper.hxx"
#include "helper/qahelper.hxx"

#include <postit.hxx>
#include <sortparam.hxx>
#include <dbdata.hxx>
#include <formulacell.hxx>
#include <scopetools.hxx>
#include <globalnames.hxx>
#include <dbdocfun.hxx>
#include <docfunc.hxx>
#include <scitems.hxx>
#include <editutil.hxx>
#include <drwlayer.hxx>
#include <queryiter.hxx>
#include <undomanager.hxx>

#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdpage.hxx>
#include <rtl/math.hxx>

using ::std::cerr;
using ::std::endl;

class TestSort : public ScUcalcTestBase
{
protected:
     void testSortRefUpdate4_Impl();
};

CPPUNIT_TEST_FIXTURE(TestSort, testSort)
{
    m_pDoc->InsertTab(0, u"test1"_ustr);

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    ScAddress aPos(0,0,0);
    {
        const std::vector<std::vector<const char*>> aData = {
            { "2", "4" },
            { "4", "1" },
            { "1", "2" },
            { "1", "23" },
        };

        clearRange(m_pDoc, ScRange(0, 0, 0, 1, aData.size(), 0));
        ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aDataRange.aStart);
    }

    // Insert note in cell B2.
    ScAddress rAddr(1, 1, 0);
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, u"Hello"_ustr);
    pNote->SetAuthor(u"Jim Bob"_ustr);

    ScSortParam aSortData;
    aSortData.nCol1 = 1;
    aSortData.nCol2 = 1;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 2;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 1;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;

    m_pDoc->Sort(0, aSortData, false, true, nullptr, nullptr);

    double nVal = m_pDoc->GetValue(1,0,0);
    ASSERT_DOUBLES_EQUAL(1.0, nVal);

    // check that note is also moved after sorting
    pNote = m_pDoc->GetNote(1, 0, 0);
    CPPUNIT_ASSERT(pNote);

    ScRange aDataRange;
    clearRange(m_pDoc, ScRange(0, 0, 0, 1, 9, 0)); // Clear A1:B10.
    {
        // 0 = empty cell
        const std::vector<std::vector<const char*>> aData = {
            { "Title" },
            { nullptr },
            { nullptr },
            { "12" },
            { "b" },
            { "1" },
            { "9" },
            { "123" }
        };

        aDataRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aDataRange.aStart);
    }

    aSortData.nCol1 = aDataRange.aStart.Col();
    aSortData.nCol2 = aDataRange.aEnd.Col();
    aSortData.nRow1 = aDataRange.aStart.Row();
    aSortData.nRow2 = aDataRange.aEnd.Row();
    aSortData.bHasHeader = true;
    aSortData.maKeyState[0].nField = 0;
    m_pDoc->Sort(0, aSortData, false, true, nullptr, nullptr);

    // Title should stay at the top, numbers should be sorted numerically,
    // numbers always come before strings, and empty cells always occur at the
    // end.
    CPPUNIT_ASSERT_EQUAL(u"Title"_ustr, m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(u"9"_ustr, m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(u"12"_ustr, m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(u"123"_ustr, m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(u"b"_ustr, m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(aPos));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortHorizontal)
{
    SortRefUpdateSetter aUpdateSet;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sort"_ustr);

    // Test case from fdo#78079.

    // 0 = empty cell
    const std::vector<std::vector<const char*>> aData = {
        { "table", "has UNIQUE", "Publish to EC2", "flag" },
        { "w2gi.mobilehit", "Yes", "No", "=CONCATENATE(B2;\"-\";C2)" },
        { "w2gi.visitors", "No", "No", "=CONCATENATE(B3;\"-\";C3)" },
        { "w2gi.pagedimension", "Yes", "Yes", "=CONCATENATE(B4;\"-\";C4)" },
    };

    // Insert raw data into A1:D4.
    ScRange aDataRange = insertRangeData(m_pDoc, ScAddress(0,0,0), aData);
    CPPUNIT_ASSERT_EQUAL(u"A1:D4"_ustr, aDataRange.Format(*m_pDoc, ScRefFlags::VALID));

    // Check the formula values.
    CPPUNIT_ASSERT_EQUAL(u"Yes-No"_ustr, m_pDoc->GetString(ScAddress(3,1,0)));
    CPPUNIT_ASSERT_EQUAL(u"No-No"_ustr, m_pDoc->GetString(ScAddress(3,2,0)));
    CPPUNIT_ASSERT_EQUAL(u"Yes-Yes"_ustr, m_pDoc->GetString(ScAddress(3,3,0)));

    // Define A1:D4 as sheet-local anonymous database range.
    m_pDoc->SetAnonymousDBData(
        0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 3, 3)));

    // Sort A1:D4 horizontally, ascending by row 1.
    ScDBDocFunc aFunc(*m_xDocShell);

    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 3;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 3;
    aSortData.bHasHeader = true;
    aSortData.bByRow = false; // Sort by column (in horizontal direction).
    aSortData.aDataAreaExtras.mbCellFormats = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "table", "flag", "has UNIQUE", "Publish to EC2" },
            { "w2gi.mobilehit",     "Yes-No",  "Yes", "No" },
            { "w2gi.visitors",      "No-No",   "No",  "No" },
            { "w2gi.pagedimension", "Yes-Yes", "Yes", "Yes" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Sorted by column with formula");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=CONCATENATE(C2;\"-\";D2)"_ustr, m_pDoc->GetFormula(1,1,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=CONCATENATE(C3;\"-\";D3)"_ustr, m_pDoc->GetFormula(1,2,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=CONCATENATE(C4;\"-\";D4)"_ustr, m_pDoc->GetFormula(1,3,0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortHorizontalWholeColumn)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sort"_ustr);

    // 0 = empty cell
    const std::vector<std::vector<const char*>> aData = {
        { "4", "2", "47", "a", "9" }
    };

    // Insert row data to C1:G1.
    ScRange aSortRange = insertRangeData(m_pDoc, ScAddress(2,0,0), aData);
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(3,0,0)));
    CPPUNIT_ASSERT_EQUAL(47.0, m_pDoc->GetValue(ScAddress(4,0,0)));
    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, m_pDoc->GetString(ScAddress(5,0,0)));
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(6,0,0)));

    // Extend the sort range to whole column.
    aSortRange.aEnd.SetRow(m_pDoc->MaxRow());

    SCCOL nCol1 = aSortRange.aStart.Col();
    SCCOL nCol2 = aSortRange.aEnd.Col();
    SCROW nRow1 = aSortRange.aStart.Row();
    SCROW nRow2 = aSortRange.aEnd.Row();

    // Define C:G as sheet-local anonymous database range.
    m_pDoc->SetAnonymousDBData(
        0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, nCol1, nRow1, nCol2, nRow2, false, false)));

    // Sort C:G horizontally ascending by row 1.
    ScDBDocFunc aFunc(*m_xDocShell);

    ScSortParam aSortData;
    aSortData.nCol1 = nCol1;
    aSortData.nCol2 = nCol2;
    aSortData.nRow1 = nRow1;
    aSortData.nRow2 = nRow2;
    aSortData.bHasHeader = false;
    aSortData.bByRow = false; // Sort by column (in horizontal direction).
    aSortData.aDataAreaExtras.mbCellFormats = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // Check the sort result.
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(3,0,0)));
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(4,0,0)));
    CPPUNIT_ASSERT_EQUAL(47.0, m_pDoc->GetValue(ScAddress(5,0,0)));
    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, m_pDoc->GetString(ScAddress(6,0,0)));

    // Undo and check.

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(3,0,0)));
    CPPUNIT_ASSERT_EQUAL(47.0, m_pDoc->GetValue(ScAddress(4,0,0)));
    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, m_pDoc->GetString(ScAddress(5,0,0)));
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(6,0,0)));

    // Redo and check.
    pUndoMgr->Redo();
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(3,0,0)));
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(4,0,0)));
    CPPUNIT_ASSERT_EQUAL(47.0, m_pDoc->GetValue(ScAddress(5,0,0)));
    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, m_pDoc->GetString(ScAddress(6,0,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortSingleRow)
{
    // This test case is from fdo#80462.

    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Sort range consists of only one row.
    m_pDoc->SetString(ScAddress(0,0,0), u"X"_ustr);
    m_pDoc->SetString(ScAddress(1,0,0), u"Y"_ustr);

    // Define A1:B1 as sheet-local anonymous database range.
    m_pDoc->SetAnonymousDBData(
        0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 1, 0)));

    // Sort A1:B1 horizontally, ascending by row 1.
    ScDBDocFunc aFunc(*m_xDocShell);

    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 1;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 0;
    aSortData.bHasHeader = true;
    aSortData.bByRow = true;
    aSortData.aDataAreaExtras.mbCellFormats = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;

    // Do the sorting.  This should not crash.
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // Another test case - single row horizontal sort with header column.
    clearSheet(m_pDoc, 0);

    // A1:G1
    m_pDoc->SetString(ScAddress(0,0,0), u"Header"_ustr);
    m_pDoc->SetValue(ScAddress(1,0,0),  1.0);
    m_pDoc->SetValue(ScAddress(2,0,0), 10.0);
    m_pDoc->SetValue(ScAddress(3,0,0),  3.0);
    m_pDoc->SetValue(ScAddress(4,0,0),  9.0);
    m_pDoc->SetValue(ScAddress(5,0,0), 12.0);
    m_pDoc->SetValue(ScAddress(6,0,0),  2.0);

    // Define A1:G1 as sheet-local anonymous database range.
    m_pDoc->SetAnonymousDBData(
        0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 6, 0, false, true)));

    // Update the sort data.
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 6;
    aSortData.bByRow = false;
    bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // Check the result.
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(3,0,0)));
    CPPUNIT_ASSERT_EQUAL( 9.0, m_pDoc->GetValue(ScAddress(4,0,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(5,0,0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(6,0,0)));

    // Undo and check.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(3,0,0)));
    CPPUNIT_ASSERT_EQUAL( 9.0, m_pDoc->GetValue(ScAddress(4,0,0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(5,0,0)));
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(6,0,0)));

    // Redo and check.
    pUndoMgr->Redo();
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(3,0,0)));
    CPPUNIT_ASSERT_EQUAL( 9.0, m_pDoc->GetValue(ScAddress(4,0,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(5,0,0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(6,0,0)));

    m_pDoc->DeleteTab(0);
}

// regression test of fdo#53814, sorting doesn't work as expected
// if cells in the sort are referenced by formulas
CPPUNIT_TEST_FIXTURE(TestSort, testSortWithFormulaRefs)
{
    SortRefUpdateSetter aUpdateSet;

    m_pDoc->InsertTab(0, u"List1"_ustr);
    m_pDoc->InsertTab(1, u"List2"_ustr);

    const char* aFormulaData[6] = {
        "=IF($List1.A2<>\"\";$List1.A2;\"\")",
        "=IF($List1.A3<>\"\";$List1.A3;\"\")",
        "=IF($List1.A4<>\"\";$List1.A4;\"\")",
        "=IF($List1.A5<>\"\";$List1.A5;\"\")",
        "=IF($List1.A6<>\"\";$List1.A6;\"\")",
        "=IF($List1.A7<>\"\";$List1.A7;\"\")",
    };

    const char* const aTextData[4] = {
        "bob",
        "tim",
        "brian",
        "larry",
    };

    const char* aResults[6] = {
        "bob",
        "brian",
        "larry",
        "tim",
        "",
        "",
    };

    // Insert data to sort in A2:A5 on the 1st sheet.
    for (SCROW i = 1; i <= 4; ++i)
        m_pDoc->SetString( 0, i, 0, OUString::createFromAscii(aTextData[i-1]) );

    // Insert formulas in A1:A6 on the 2nd sheet.
    for (size_t i = 0; i < std::size(aFormulaData); ++i)
        m_pDoc->SetString( 0, i, 1, OUString::createFromAscii(aFormulaData[i]) );

    // Sort data in A2:A8 on the 1st sheet. No column header.
    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 0;
    aSortData.nRow1 = 1;
    aSortData.nRow2 = 7;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;

    m_pDoc->Sort(0, aSortData, false, true, nullptr, nullptr);

    for (size_t i = 0; i < std::size(aResults); ++i)
    {
        OUString sResult = m_pDoc->GetString(0, i + 1, 0);
        CPPUNIT_ASSERT_EQUAL( OUString::createFromAscii( aResults[i] ), sResult );
    }
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortWithStrings)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    ScFieldEditEngine& rEE = m_pDoc->GetEditEngine();
    rEE.SetTextCurrentDefaults(u"Val1"_ustr);
    m_pDoc->SetString(ScAddress(1,1,0), u"Header"_ustr);
    m_pDoc->SetString(ScAddress(1,2,0), u"Val2"_ustr);
    m_pDoc->SetEditText(ScAddress(1,3,0), rEE.CreateTextObject());

    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(u"Val2"_ustr, m_pDoc->GetString(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(u"Val1"_ustr, m_pDoc->GetString(ScAddress(1,3,0)));

    ScSortParam aParam;
    aParam.nCol1 = 1;
    aParam.nCol2 = 1;
    aParam.nRow1 = 1;
    aParam.nRow2 = 3;
    aParam.bHasHeader = true;
    aParam.maKeyState[0].bDoSort = true;
    aParam.maKeyState[0].bAscending = true;
    aParam.maKeyState[0].nField = 1;
    aParam.maKeyState[0].aColorSortMode = ScColorSortMode::None;

    m_pDoc->Sort(0, aParam, false, true, nullptr, nullptr);

    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(u"Val1"_ustr, m_pDoc->GetString(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(u"Val2"_ustr, m_pDoc->GetString(ScAddress(1,3,0)));

    aParam.maKeyState[0].bAscending = false;

    m_pDoc->Sort(0, aParam, false, true, nullptr, nullptr);

    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(u"Val2"_ustr, m_pDoc->GetString(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(u"Val1"_ustr, m_pDoc->GetString(ScAddress(1,3,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortInFormulaGroup)
{
    SortRefUpdateSetter aUpdateSet;

    static const struct {
        SCCOL nCol;
        SCROW nRow;
        const char *pData;
    } aEntries[] = {
        { 0, 0, "3" },   { 1, 0, "=A1" },
        { 0, 1, "1" },   { 1, 1, "=A2" },
        { 0, 2, "20" },  { 1, 2, "=A3" },
        { 0, 3, "10" },  { 1, 3, "=A4+1" }, // swap across groups
        { 0, 4, "2"  },  { 1, 4, "=A5+1" },
        { 0, 5, "101" }, { 1, 5, "=A6" }, // swap inside contiguous group
        { 0, 6, "100" }, { 1, 6, "=A7" },
        { 0, 7, "102" }, { 1, 7, "=A8" },
        { 0, 8, "104" }, { 1, 8, "=A9" },
        { 0, 9, "103" }, { 1, 9, "=A10" },
    };

    m_pDoc->InsertTab(0, u"sorttest"_ustr);

    for ( auto const & i: aEntries )
        m_pDoc->SetString( i.nCol, i.nRow, 0,
                           OUString::createFromAscii( i.pData) );

    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 1;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 9;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;

    m_pDoc->Sort(0, aSortData, false, true, nullptr, nullptr);

    static const double aResults[] = {
        1.0,   1.0,
        2.0,   3.0,
        3.0,   3.0,
        10.0,  11.0,
        20.0,  20.0,
        100.0, 100.0,
        101.0, 101.0,
        102.0, 102.0,
        103.0, 103.0,
        104.0, 104.0
    };

    for ( SCROW i = 0; i < SCROW(std::size( aEntries )); ++i )
    {
        double val = m_pDoc->GetValue( aEntries[i].nCol, aEntries[i].nRow, 0 );
        CPPUNIT_ASSERT_MESSAGE("Mis-matching value after sort.",
                               rtl::math::approxEqual(val, aResults[i]));
    }

    m_pDoc->DeleteTab( 0 );
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortWithCellFormats)
{
    struct
    {
        bool isBold( const ScPatternAttr* pPat ) const
        {
            if (!pPat)
            {
                cerr << "Pattern is NULL!" << endl;
                return false;
            }

            const SfxPoolItem* pItem = nullptr;
            if (!pPat->GetItemSet().HasItem(ATTR_FONT_WEIGHT, &pItem))
            {
                cerr << "Pattern does not have a font weight item, but it should." << endl;
                return false;
            }

            CPPUNIT_ASSERT(pItem);

            if (static_cast<const SvxWeightItem*>(pItem)->GetEnumValue() != WEIGHT_BOLD)
            {
                cerr << "Font weight should be bold." << endl;
                return false;
            }

            return true;
        }

        bool isItalic( const ScPatternAttr* pPat ) const
        {
            if (!pPat)
            {
                cerr << "Pattern is NULL!" << endl;
                return false;
            }

            const SfxPoolItem* pItem = nullptr;
            if (!pPat->GetItemSet().HasItem(ATTR_FONT_POSTURE, &pItem))
            {
                cerr << "Pattern does not have a font posture item, but it should." << endl;
                return false;
            }

            CPPUNIT_ASSERT(pItem);

            if (static_cast<const SvxPostureItem*>(pItem)->GetEnumValue() != ITALIC_NORMAL)
            {
                cerr << "Italic should be applied.." << endl;
                return false;
            }

            return true;
        }

        bool isNormal( const ScPatternAttr* pPat ) const
        {
            if (!pPat)
            {
                cerr << "Pattern is NULL!" << endl;
                return false;
            }

            const SfxPoolItem* pItem = nullptr;
            if (pPat->GetItemSet().HasItem(ATTR_FONT_WEIGHT, &pItem))
            {
                // Check if the font weight is applied.
                if (static_cast<const SvxWeightItem*>(pItem)->GetEnumValue() == WEIGHT_BOLD)
                {
                    cerr << "This cell is bold, but shouldn't." << endl;
                    return false;
                }
            }

            if (pPat->GetItemSet().HasItem(ATTR_FONT_POSTURE, &pItem))
            {
                // Check if the italics is applied.
                if (static_cast<const SvxPostureItem*>(pItem)->GetEnumValue() == ITALIC_NORMAL)
                {
                    cerr << "This cell is italic, but shouldn't." << endl;
                    return false;
                }
            }

            return true;
        }

    } aCheck;

    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Insert some values into A1:A4.
    m_pDoc->SetString(ScAddress(0,0,0), u"Header"_ustr);
    m_pDoc->SetString(ScAddress(0,1,0), u"Normal"_ustr);
    m_pDoc->SetString(ScAddress(0,2,0), u"Bold"_ustr);
    m_pDoc->SetString(ScAddress(0,3,0), u"Italic"_ustr);

    // Set A3 bold and A4 italic.
    const ScPatternAttr* pPat = m_pDoc->GetPattern(ScAddress(0,2,0));
    CPPUNIT_ASSERT(pPat);
    {
        ScPatternAttr aNewPat(*pPat);
        SfxItemSet& rSet = aNewPat.GetItemSet();
        rSet.Put(SvxWeightItem(WEIGHT_BOLD, ATTR_FONT_WEIGHT));
        m_pDoc->ApplyPattern(0, 2, 0, aNewPat);

        // Make sure it's really in.
        bool bGood = aCheck.isBold(m_pDoc->GetPattern(ScAddress(0,2,0)));
        CPPUNIT_ASSERT_MESSAGE("A3 is not bold but it should.", bGood);
    }

    pPat = m_pDoc->GetPattern(ScAddress(0,3,0));
    CPPUNIT_ASSERT(pPat);
    {
        ScPatternAttr aNewPat(*pPat);
        SfxItemSet& rSet = aNewPat.GetItemSet();
        rSet.Put(SvxPostureItem(ITALIC_NORMAL, ATTR_FONT_POSTURE));
        m_pDoc->ApplyPattern(0, 3, 0, aNewPat);

        bool bGood = aCheck.isItalic(m_pDoc->GetPattern(ScAddress(0,3,0)));
        CPPUNIT_ASSERT_MESSAGE("A4 is not italic but it should.", bGood);
    }

    // Define A1:A4 as sheet-local anonymous database range, else sort wouldn't run.
    m_pDoc->SetAnonymousDBData(
        0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 0, 3)));

    // Sort A1:A4 ascending with cell formats.
    ScDBDocFunc aFunc(*m_xDocShell);

    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 0;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 3;
    aSortData.bHasHeader = true;
    aSortData.aDataAreaExtras.mbCellFormats = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
    bool bSorted = aFunc.Sort(0, aSortData, true, false, true);
    CPPUNIT_ASSERT(bSorted);

    // Check the sort result.
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(u"Bold"_ustr,   m_pDoc->GetString(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(u"Italic"_ustr, m_pDoc->GetString(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(u"Normal"_ustr, m_pDoc->GetString(ScAddress(0,3,0)));

    // A2 should be bold now.
    bool bBold = aCheck.isBold(m_pDoc->GetPattern(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_MESSAGE("A2 should be bold after the sort.", bBold);

    // and A3 should be italic.
    bool bItalic = aCheck.isItalic(m_pDoc->GetPattern(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_MESSAGE("A3 should be italic.", bItalic);

    // A4 should have neither bold nor italic.
    bool bNormal = aCheck.isNormal(m_pDoc->GetPattern(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_MESSAGE("A4 should be neither bold nor italic.", bNormal);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortRefUpdate)
{
    SortTypeSetter aSortTypeSet(true);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    m_pDoc->InsertTab(0, u"Sort"_ustr);

    // Set values to sort in column A.
    m_pDoc->SetString(ScAddress(0,0,0), u"Header"_ustr);

    double aValues[] = { 4.0, 36.0, 14.0, 29.0, 98.0, 78.0, 0.0, 99.0, 1.0 };
    size_t nCount = std::size(aValues);
    for (size_t i = 0; i < nCount; ++i)
        m_pDoc->SetValue(ScAddress(0,i+1,0), aValues[i]);

    // Set formulas to reference these values in column C.
    m_pDoc->SetString(ScAddress(2,0,0), u"Formula"_ustr);
    for (size_t i = 0; i < nCount; ++i)
        m_pDoc->SetString(ScAddress(2,1+i,0), u"=RC[-2]"_ustr);

    // Check the values in column C.
    for (size_t i = 0; i < nCount; ++i)
    {
        double fCheck = aValues[i];
        CPPUNIT_ASSERT_EQUAL(fCheck, m_pDoc->GetValue(ScAddress(2,i+1,0)));
    }

    ScDBDocFunc aFunc(*m_xDocShell);

    // Define A1:A10 as sheet-local anonymous database range, else sort wouldn't run.
    m_pDoc->SetAnonymousDBData(
        0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 0, 9)));

    // Sort A1:A10 (with a header row).
    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 0;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 9;
    aSortData.bHasHeader = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    double aSorted[] = { 0.0, 1.0, 4.0, 14.0, 29.0, 36.0, 78.0, 98.0, 99.0 };

    // Check the sort result.
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    for (size_t i = 0; i < nCount; ++i)
    {
        double fCheck = aSorted[i];
        CPPUNIT_ASSERT_EQUAL(fCheck, m_pDoc->GetValue(ScAddress(0,i+1,0)));
    }

    // Sorting should not alter the values in column C.
    m_pDoc->CalcAll(); // just in case...
    for (size_t i = 0; i < nCount; ++i)
    {
        double fCheck = aValues[i];
        CPPUNIT_ASSERT_EQUAL(fCheck, m_pDoc->GetValue(ScAddress(2,i+1,0)));
    }

    // C2 should now point to A4.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C2!", u"=R[2]C[-2]"_ustr, m_pDoc->GetFormula(2,1,0));

    // Undo the sort.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    pUndoMgr->Undo();

    // Check the undo result.
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    for (size_t i = 0; i < nCount; ++i)
    {
        double fCheck = aValues[i];
        CPPUNIT_ASSERT_EQUAL(fCheck, m_pDoc->GetValue(ScAddress(0,i+1,0)));
    }

    // Values in column C should still be unaltered.
    m_pDoc->CalcAll(); // just in case...
    for (size_t i = 0; i < nCount; ++i)
    {
        double fCheck = aValues[i];
        CPPUNIT_ASSERT_EQUAL(fCheck, m_pDoc->GetValue(ScAddress(2,i+1,0)));
    }

    // C2 should now point to A2.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C2!", u"=RC[-2]"_ustr, m_pDoc->GetFormula(2,1,0));

    // Redo.
    pUndoMgr->Redo();

    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    for (size_t i = 0; i < nCount; ++i)
    {
        double fCheck = aSorted[i];
        CPPUNIT_ASSERT_EQUAL(fCheck, m_pDoc->GetValue(ScAddress(0,i+1,0)));
    }

    // Sorting should not alter the values in column C.
    m_pDoc->CalcAll(); // just in case...
    for (size_t i = 0; i < nCount; ++i)
    {
        double fCheck = aValues[i];
        CPPUNIT_ASSERT_EQUAL(fCheck, m_pDoc->GetValue(ScAddress(2,i+1,0)));
    }

    // C2 should now point to A4.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C2!", u"=R[2]C[-2]"_ustr, m_pDoc->GetFormula(2,1,0));

    // Undo again.
    pUndoMgr->Undo();

    // Formulas in column C should all be "RC[-2]" again.
    for (size_t i = 0; i < nCount; ++i)
        m_pDoc->SetString(ScAddress(2,1+i,0), u"=RC[-2]"_ustr);

    // Turn off reference update on sort.
    SortTypeSetter::changeTo(false);

    bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // Check the sort result again.
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    for (size_t i = 0; i < nCount; ++i)
    {
        double fCheck = aSorted[i];
        CPPUNIT_ASSERT_EQUAL(fCheck, m_pDoc->GetValue(ScAddress(0,i+1,0)));
    }

    // Formulas in column C should all remain "RC[-2]".
    for (size_t i = 0; i < nCount; ++i)
        m_pDoc->SetString(ScAddress(2,1+i,0), u"=RC[-2]"_ustr);

    // The values in column C should now be the same as sorted values in column A.
    m_pDoc->CalcAll(); // just in case...
    for (size_t i = 0; i < nCount; ++i)
    {
        double fCheck = aSorted[i];
        CPPUNIT_ASSERT_EQUAL(fCheck, m_pDoc->GetValue(ScAddress(2,i+1,0))); // column C
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortRefUpdate2)
{
    SortRefUpdateSetter aUpdateSet;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    m_pDoc->InsertTab(0, u"Sort"_ustr);

    // Set up the sheet.
    const char* aData[][2] = {
        { "F1", "F2" },
        { "9", "=RC[-1]" },
        { "2", "=RC[-1]" },
        { "6", "=RC[-1]" },
        { "4", "=RC[-1]" },
        { nullptr, nullptr } // terminator
    };

    for (SCROW i = 0; aData[i][0]; ++i)
    {
        m_pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i][0]));
        m_pDoc->SetString(1, i, 0, OUString::createFromAscii(aData[i][1]));
    }

    // Check the values in B2:B5.
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    ScDBDocFunc aFunc(*m_xDocShell);

    // Define A1:B5 as sheet-local anonymous database range, else sort wouldn't run.
    m_pDoc->SetAnonymousDBData(
        0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 1, 4)));

    // Sort A1:B5 by column A (with a row header).
    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 1;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 4;
    aSortData.bHasHeader = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // Check the sort result in column A.
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(0,4,0)));

    // and column B.
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    // Formulas in column B should still point to their respective left neighbor cell.
    for (SCROW i = 1; i <= 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=RC[-1]"_ustr, m_pDoc->GetFormula(1,i,0));
    }

    // Undo and check the result in column B.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    pUndoMgr->Undo();

    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    // and redo.
    pUndoMgr->Redo();

    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortRefUpdate3)
{
    SortRefUpdateSetter aUpdateSet;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, u"Sort"_ustr);

    const char* pData[] = {
        "Header",
        "1",
        "=A2+10",
        "2",
        "=A4+10",
        "=A2+A4",
        nullptr // terminator
    };

    for (SCROW i = 0; pData[i]; ++i)
        m_pDoc->SetString(ScAddress(0,i,0), OUString::createFromAscii(pData[i]));

    // Check the initial values.
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    ScDBDocFunc aFunc(*m_xDocShell);

    // Sort A1:A6.
    m_pDoc->SetAnonymousDBData(
        0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 0, 5)));

    // Sort A1:A6 by column A (with a row header).
    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 0;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 5;
    aSortData.bHasHeader = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // Check the sorted values.
    m_pDoc->CalcAll();
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Make sure the formula cells have been adjusted correctly.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A4.", u"=A2+A3"_ustr, m_pDoc->GetFormula(0,3,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A5.", u"=A2+10"_ustr, m_pDoc->GetFormula(0,4,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=A3+10"_ustr, m_pDoc->GetFormula(0,5,0));

    // Undo and check the result.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    pUndoMgr->Undo();
    m_pDoc->CalcAll();
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Redo and check the result.
    pUndoMgr->Redo();
    m_pDoc->CalcAll();
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    m_pDoc->DeleteTab(0);
}

// Derived from fdo#79441 https://bugs.freedesktop.org/attachment.cgi?id=100144
// testRefInterne.ods
CPPUNIT_TEST_FIXTURE(TestSort, testSortRefUpdate4)
{
    // This test has to work in both update reference modes.
    {
        SortRefNoUpdateSetter aUpdateSet;
        testSortRefUpdate4_Impl();
    }
    {
        SortRefUpdateSetter aUpdateSet;
        testSortRefUpdate4_Impl();
    }
}

void TestSort::testSortRefUpdate4_Impl()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, u"Sort"_ustr);
    m_pDoc->InsertTab(1, u"Lesson1"_ustr);
    m_pDoc->InsertTab(2, u"Lesson2"_ustr);

    {
        const std::vector<std::vector<const char*>> aData = {
            { "Name", "Note" },
            { "Student1", "1" },
            { "Student2", "2" },
            { "Student3", "3" },
            { "Student4", "4" },
            { "Student5", "5" },
        };

        SCTAB nTab = 1;
        ScAddress aPos(0,0,nTab);
        clearRange(m_pDoc, ScRange(0, 0, nTab, 1, aData.size(), nTab));
        ScRange aLesson1Range = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aLesson1Range.aStart);
    }

    {
        const std::vector<std::vector<const char*>> aData = {
            { "Name", "Note" },
            { "=Lesson1.A2", "3" },
            { "=Lesson1.A3", "4" },
            { "=Lesson1.A4", "9" },
            { "=Lesson1.A5", "6" },
            { "=Lesson1.A6", "3" },
        };

        SCTAB nTab = 2;
        ScAddress aPos(0,0,nTab);
        clearRange(m_pDoc, ScRange(0, 0, nTab, 1, aData.size(), nTab));
        ScRange aLesson2Range = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aLesson2Range.aStart);
    }

    ScRange aSortRange;
    {
        const std::vector<std::vector<const char*>> aData = {
            { "Name", "Lesson1", "Lesson2", "Average" },
            { "=Lesson1.A2", "=Lesson1.B2", "=Lesson2.B2", "=AVERAGE(B2:C2)" },
            { "=Lesson1.A3", "=Lesson1.B3", "=Lesson2.B3", "=AVERAGE(B3:C3)" },
            { "=Lesson1.A4", "=Lesson1.B4", "=Lesson2.B4", "=AVERAGE(B4:C4)" },
            { "=Lesson1.A5", "=Lesson1.B5", "=Lesson2.B5", "=AVERAGE(B5:C5)" },
            { "=Lesson1.A6", "=Lesson1.B6", "=Lesson2.B6", "=AVERAGE(B6:C6)" },
        };

        SCTAB nTab = 0;
        ScAddress aPos(0,0,nTab);
        clearRange(m_pDoc, ScRange(0, 0, nTab, 1, aData.size(), nTab));
        aSortRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aSortRange.aStart);
    }

    ScDBDocFunc aFunc(*m_xDocShell);

    // Sort A1:D6 by column D (Average, with a row header).
    {
        ScSortParam aSortData;
        aSortData.nCol1 = aSortRange.aStart.Col();
        aSortData.nCol2 = aSortRange.aEnd.Col();
        aSortData.nRow1 = aSortRange.aStart.Row();
        aSortData.nRow2 = aSortRange.aEnd.Row();
        aSortData.bHasHeader = true;
        aSortData.maKeyState[0].bDoSort = true;         // sort on
        aSortData.maKeyState[0].nField = 3;             // Average
        aSortData.maKeyState[0].bAscending = false;     // descending
        aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;

        m_pDoc->SetAnonymousDBData( 0, std::unique_ptr<ScDBData>(new ScDBData( STR_DB_LOCAL_NONAME, aSortRange.aStart.Tab(),
                    aSortData.nCol1, aSortData.nRow1, aSortData.nCol2, aSortData.nRow2)));

        bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
        CPPUNIT_ASSERT(bSorted);

        // Check the sorted values.
        m_pDoc->CalcAll();
        CPPUNIT_ASSERT_EQUAL(u"Name"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
        CPPUNIT_ASSERT_EQUAL(u"Student3"_ustr, m_pDoc->GetString(ScAddress(0,1,0)));
        CPPUNIT_ASSERT_EQUAL(u"Student4"_ustr, m_pDoc->GetString(ScAddress(0,2,0)));
        CPPUNIT_ASSERT_EQUAL(u"Student5"_ustr, m_pDoc->GetString(ScAddress(0,3,0)));
        CPPUNIT_ASSERT_EQUAL(u"Student2"_ustr, m_pDoc->GetString(ScAddress(0,4,0)));
        CPPUNIT_ASSERT_EQUAL(u"Student1"_ustr, m_pDoc->GetString(ScAddress(0,5,0)));
        CPPUNIT_ASSERT_EQUAL( 6.0, m_pDoc->GetValue(ScAddress(3,1,0)));
        CPPUNIT_ASSERT_EQUAL( 5.0, m_pDoc->GetValue(ScAddress(3,2,0)));
        CPPUNIT_ASSERT_EQUAL( 4.0, m_pDoc->GetValue(ScAddress(3,3,0)));
        CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(3,4,0)));
        CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(3,5,0)));

        // Make sure the formula cells have been adjusted correctly.
        const char* aCheck[][4] = {
            // Name          Lesson1       Lesson2       Average
            { "=Lesson1.A4", "=Lesson1.B4", "=Lesson2.B4", "=AVERAGE(B2:C2)" },
            { "=Lesson1.A5", "=Lesson1.B5", "=Lesson2.B5", "=AVERAGE(B3:C3)" },
            { "=Lesson1.A6", "=Lesson1.B6", "=Lesson2.B6", "=AVERAGE(B4:C4)" },
            { "=Lesson1.A3", "=Lesson1.B3", "=Lesson2.B3", "=AVERAGE(B5:C5)" },
            { "=Lesson1.A2", "=Lesson1.B2", "=Lesson2.B2", "=AVERAGE(B6:C6)" },
        };
        for (SCROW nRow=0; nRow < static_cast<SCROW>(SAL_N_ELEMENTS(aCheck)); ++nRow)
        {
            for (SCCOL nCol=0; nCol < 4; ++nCol)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("Wrong formula in " + OStringChar(char('A'+nCol)) + OString::number(nRow+2) + ".").getStr(), OUString::createFromAscii(aCheck[nRow][nCol]), m_pDoc->GetFormula(nCol,nRow+1,0));
            }
        }

        // Undo and check the result.
        SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
        pUndoMgr->Undo();
        m_pDoc->CalcAll();
        CPPUNIT_ASSERT_EQUAL(u"Name"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
        CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(3,1,0)));
        CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(3,2,0)));
        CPPUNIT_ASSERT_EQUAL( 6.0, m_pDoc->GetValue(ScAddress(3,3,0)));
        CPPUNIT_ASSERT_EQUAL( 5.0, m_pDoc->GetValue(ScAddress(3,4,0)));
        CPPUNIT_ASSERT_EQUAL( 4.0, m_pDoc->GetValue(ScAddress(3,5,0)));

        // Redo and check the result.
        pUndoMgr->Redo();
        m_pDoc->CalcAll();
        CPPUNIT_ASSERT_EQUAL(u"Name"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
        CPPUNIT_ASSERT_EQUAL( 6.0, m_pDoc->GetValue(ScAddress(3,1,0)));
        CPPUNIT_ASSERT_EQUAL( 5.0, m_pDoc->GetValue(ScAddress(3,2,0)));
        CPPUNIT_ASSERT_EQUAL( 4.0, m_pDoc->GetValue(ScAddress(3,3,0)));
        CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(3,4,0)));
        CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(3,5,0)));
    }

    // Sort A2:AMJ6 by column A (Name, without header).
    {
        ScSortParam aSortData;
        aSortData.nCol1 = 0;
        aSortData.nCol2 = m_pDoc->MaxCol();
        aSortData.nRow1 = aSortRange.aStart.Row()+1;
        aSortData.nRow2 = aSortRange.aEnd.Row();
        aSortData.bHasHeader = false;
        aSortData.maKeyState[0].bDoSort = true;         // sort on
        aSortData.maKeyState[0].nField = 0;             // Name
        aSortData.maKeyState[0].bAscending = false;     // descending
        aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;

        m_pDoc->SetAnonymousDBData( 0, std::unique_ptr<ScDBData>(new ScDBData( STR_DB_LOCAL_NONAME, aSortRange.aStart.Tab(),
                    aSortData.nCol1, aSortData.nRow1, aSortData.nCol2, aSortData.nRow2)));

        bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
        CPPUNIT_ASSERT(bSorted);

        // Check the sorted values.
        m_pDoc->CalcAll();
        CPPUNIT_ASSERT_EQUAL(u"Name"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
        CPPUNIT_ASSERT_EQUAL(u"Student5"_ustr, m_pDoc->GetString(ScAddress(0,1,0)));
        CPPUNIT_ASSERT_EQUAL(u"Student4"_ustr, m_pDoc->GetString(ScAddress(0,2,0)));
        CPPUNIT_ASSERT_EQUAL(u"Student3"_ustr, m_pDoc->GetString(ScAddress(0,3,0)));
        CPPUNIT_ASSERT_EQUAL(u"Student2"_ustr, m_pDoc->GetString(ScAddress(0,4,0)));
        CPPUNIT_ASSERT_EQUAL(u"Student1"_ustr, m_pDoc->GetString(ScAddress(0,5,0)));
        CPPUNIT_ASSERT_EQUAL( 4.0, m_pDoc->GetValue(ScAddress(3,1,0)));
        CPPUNIT_ASSERT_EQUAL( 5.0, m_pDoc->GetValue(ScAddress(3,2,0)));
        CPPUNIT_ASSERT_EQUAL( 6.0, m_pDoc->GetValue(ScAddress(3,3,0)));
        CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(3,4,0)));
        CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(3,5,0)));

        // Make sure the formula cells have been adjusted correctly.
        const char* aCheck[][4] = {
            // Name          Lesson1       Lesson2       Average
            { "=Lesson1.A6", "=Lesson1.B6", "=Lesson2.B6", "=AVERAGE(B2:C2)" },
            { "=Lesson1.A5", "=Lesson1.B5", "=Lesson2.B5", "=AVERAGE(B3:C3)" },
            { "=Lesson1.A4", "=Lesson1.B4", "=Lesson2.B4", "=AVERAGE(B4:C4)" },
            { "=Lesson1.A3", "=Lesson1.B3", "=Lesson2.B3", "=AVERAGE(B5:C5)" },
            { "=Lesson1.A2", "=Lesson1.B2", "=Lesson2.B2", "=AVERAGE(B6:C6)" },
        };
        for (SCROW nRow=0; nRow < static_cast<SCROW>(SAL_N_ELEMENTS(aCheck)); ++nRow)
        {
            for (SCCOL nCol=0; nCol < 4; ++nCol)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("Wrong formula in " + OStringChar(char('A'+nCol)) + OString::number(nRow+2) + ".").getStr(), OUString::createFromAscii(aCheck[nRow][nCol]), m_pDoc->GetFormula(nCol,nRow+1,0));
            }
        }
    }

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

// Make sure the refupdate works also with volatile cells, see fdo#83067
/* FIXME: this test is not roll-over-midnight safe and will fail then! We may
 * want to have something different, but due to the nature of volatile
 * functions it's not that easy to come up with something reproducible staying
 * stable over sorts... ;-)  Check for time and don't run test a few seconds
 * before midnight, ermm... */
CPPUNIT_TEST_FIXTURE(TestSort, testSortRefUpdate5)
{
    SortRefUpdateSetter aUpdateSet;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, u"Sort"_ustr);

    double aValCheck[][3] = {
        // Result, Unsorted order, Sorted result.
        { 0, 4, 0 },
        { 0, 1, 0 },
        { 0, 3, 0 },
        { 0, 2, 0 },
    };
    ScRange aSortRange;
    {
        const std::vector<std::vector<const char*>> aData = {
            { "Date", "Volatile", "Order" },
            { "1999-05-05", "=TODAY()-$A2", "4" },
            { "1994-10-18", "=TODAY()-$A3", "1" },
            { "1996-06-30", "=TODAY()-$A4", "3" },
            { "1995-11-21", "=TODAY()-$A5", "2" },
        };

        SCTAB nTab = 0;
        ScAddress aPos(0,0,nTab);
        clearRange(m_pDoc, ScRange(0, 0, nTab, 2, aData.size(), nTab));
        aSortRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aSortRange.aStart);

        // Actual results and expected sorted results.
        for (SCROW nRow=0; nRow < static_cast<SCROW>(SAL_N_ELEMENTS(aValCheck)); ++nRow)
        {
            double fVal = m_pDoc->GetValue(ScAddress(1,nRow+1,0));
            aValCheck[nRow][0] = fVal;
            aValCheck[static_cast<size_t>(aValCheck[nRow][1])-1][2] = fVal;
        }
    }

    ScDBDocFunc aFunc(*m_xDocShell);

    // Sort A1:B5.
    m_pDoc->SetAnonymousDBData( 0, std::unique_ptr<ScDBData>(new ScDBData( STR_DB_LOCAL_NONAME, aSortRange.aStart.Tab(),
                aSortRange.aStart.Col(), aSortRange.aStart.Row(), aSortRange.aEnd.Col(), aSortRange.aEnd.Row())));

    // Sort by column A.
    ScSortParam aSortData;
    aSortData.nCol1 = aSortRange.aStart.Col();
    aSortData.nCol2 = aSortRange.aEnd.Col();
    aSortData.nRow1 = aSortRange.aStart.Row();
    aSortData.nRow2 = aSortRange.aEnd.Row();
    aSortData.bHasHeader = true;
    aSortData.maKeyState[0].bDoSort = true;         // sort on
    aSortData.maKeyState[0].nField = 0;             // Date
    aSortData.maKeyState[0].bAscending = true;      // ascending
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // Check the sorted values.
    m_pDoc->CalcAll();
    for (SCROW nRow=0; nRow < static_cast<SCROW>(SAL_N_ELEMENTS(aValCheck)); ++nRow)
    {
        size_t i = static_cast<size_t>(m_pDoc->GetValue(ScAddress(2,nRow+1,0)));    // order 1..4
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(nRow+1), i);
        CPPUNIT_ASSERT_EQUAL( aValCheck[i-1][2], m_pDoc->GetValue(ScAddress(1,nRow+1,0)));
    }

    // Make sure the formula cells have been adjusted correctly.
    const char* aFormulaCheck[] = {
        // Volatile
        "=TODAY()-$A2",
        "=TODAY()-$A3",
        "=TODAY()-$A4",
        "=TODAY()-$A5",
    };
    for (SCROW nRow=0; nRow < static_cast<SCROW>(SAL_N_ELEMENTS(aFormulaCheck)); ++nRow)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("Wrong formula in B" + OString::number(nRow+2) + ".").getStr(), OUString::createFromAscii(aFormulaCheck[nRow]), m_pDoc->GetFormula(1,nRow+1,0));
    }

    // Undo and check the result.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    pUndoMgr->Undo();
    m_pDoc->CalcAll();
    for (SCROW nRow=0; nRow < static_cast<SCROW>(SAL_N_ELEMENTS(aValCheck)); ++nRow)
    {
        CPPUNIT_ASSERT_EQUAL( aValCheck[nRow][0], m_pDoc->GetValue(ScAddress(1,nRow+1,0)));
        CPPUNIT_ASSERT_EQUAL( aValCheck[nRow][1], m_pDoc->GetValue(ScAddress(2,nRow+1,0)));
    }

    // Redo and check the result.
    pUndoMgr->Redo();
    m_pDoc->CalcAll();
    for (SCROW nRow=0; nRow < static_cast<SCROW>(SAL_N_ELEMENTS(aValCheck)); ++nRow)
    {
        size_t i = static_cast<size_t>(m_pDoc->GetValue(ScAddress(2,nRow+1,0)));    // order 1..4
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(nRow+1), i);
        CPPUNIT_ASSERT_EQUAL( aValCheck[i-1][2], m_pDoc->GetValue(ScAddress(1,nRow+1,0)));
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortRefUpdate6)
{
    SortRefNoUpdateSetter aUpdateSet;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, u"Sort"_ustr);

    const std::vector<std::vector<const char*>> aData = {
        { "Order", "Value", "1" },
        { "9", "1", "=C1+B2" },
        { "1", "2", "=C2+B3" },
        { "8", "3", "=C3+B4" },
    };

    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData);
    CPPUNIT_ASSERT_EQUAL(aPos, aDataRange.aStart);

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "Order", "Value", "1" },
            { "9", "1", "2" },
            { "1", "2", "4" },
            { "8", "3", "7" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Initial value");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    ScDBDocFunc aFunc(*m_xDocShell);

    // Sort A1:C4.
    m_pDoc->SetAnonymousDBData(
        0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 2, 3)));

    // Sort A1:A6 by column A (with a row header).
    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 2;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 3;
    aSortData.bHasHeader = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "Order", "Value", "1" },
            { "1", "2", "3" },
            { "8", "3", "6" },
            { "9", "1", "7" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Sorted without reference update");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Make sure that the formulas in C2:C4 are not adjusted.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=C1+B2"_ustr, m_pDoc->GetFormula(2,1,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=C2+B3"_ustr, m_pDoc->GetFormula(2,2,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=C3+B4"_ustr, m_pDoc->GetFormula(2,3,0));

    // Undo and check.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    pUndoMgr->Undo();

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "Order", "Value", "1" },
            { "9", "1", "2" },
            { "1", "2", "4" },
            { "8", "3", "7" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "After undo");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Redo and check.
    pUndoMgr->Redo();
    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "Order", "Value", "1" },
            { "1", "2", "3" },
            { "8", "3", "6" },
            { "9", "1", "7" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "After redo");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Change the value of C1 and make sure the formula broadcasting chain still works.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    rFunc.SetValueCell(ScAddress(2,0,0), 11.0, false);
    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "Order", "Value", "11" },
            { "1", "2", "13" },
            { "8", "3", "16" },
            { "9", "1", "17" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Change the header value");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Undo and check.
    pUndoMgr->Undo();
    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "Order", "Value", "1" },
            { "1", "2", "3" },
            { "8", "3", "6" },
            { "9", "1", "7" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "After undo of header value change");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    m_pDoc->DeleteTab(0);
}

// fdo#86762 check that broadcasters are sorted correctly and empty cell is
// broadcasted.
CPPUNIT_TEST_FIXTURE(TestSort, testSortBroadcaster)
{
    SortRefNoUpdateSetter aUpdateSet;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, u"Sort"_ustr);

    {
        const std::vector<std::vector<const char*>> aData = {
            { "1",   nullptr, nullptr, "=B1", "=$B$1", "=SUM(A1:B1)", "=SUM($A$1:$B$1)" },
            { "2", "8", nullptr, "=B2", "=$B$2", "=SUM(A2:B2)", "=SUM($A$2:$B$2)" },
        };

        ScAddress aPos(0,0,0);
        ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL(aPos, aDataRange.aStart);

        {
            // Expected output table content.  0 = empty cell
            std::vector<std::vector<const char*>> aOutputCheck = {
                { "1",   nullptr, nullptr, "0", "0",  "1",  "1" },
                { "2", "8", nullptr, "8", "8", "10", "10" },
            };

            bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Initial value");
            CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
        }

        // Sort A1:B2.
        m_pDoc->SetAnonymousDBData(
                0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 1, 1)));

        ScDBDocFunc aFunc(*m_xDocShell);

        // Sort A1:B2 by column A descending.
        ScSortParam aSortData;
        aSortData.nCol1 = 0;
        aSortData.nCol2 = 1;
        aSortData.nRow1 = 0;
        aSortData.nRow2 = 1;
        aSortData.bHasHeader = false;
        aSortData.bByRow = true;
        aSortData.maKeyState[0].bDoSort = true;
        aSortData.maKeyState[0].nField = 0;
        aSortData.maKeyState[0].bAscending = false;
        aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
        bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
        CPPUNIT_ASSERT(bSorted);

        {
            // Expected output table content.  0 = empty cell
            std::vector<std::vector<const char*>> aOutputCheck = {
                { "2", "8", nullptr, "8", "8", "10", "10" },
                { "1",   nullptr, nullptr, "0", "0",  "1",  "1" },
            };

            bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Sorted without reference update");
            CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
        }

        // Make sure that the formulas in D1:G2 are not adjusted.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=B1"_ustr, m_pDoc->GetFormula(3,0,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=B2"_ustr, m_pDoc->GetFormula(3,1,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=$B$1"_ustr, m_pDoc->GetFormula(4,0,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=$B$2"_ustr, m_pDoc->GetFormula(4,1,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A1:B1)"_ustr, m_pDoc->GetFormula(5,0,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A2:B2)"_ustr, m_pDoc->GetFormula(5,1,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM($A$1:$B$1)"_ustr, m_pDoc->GetFormula(6,0,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM($A$2:$B$2)"_ustr, m_pDoc->GetFormula(6,1,0));

        // Enter new value and check that it is broadcasted. First in empty cell.
        m_pDoc->SetString(1,1,0, u"16"_ustr);
        double nVal = m_pDoc->GetValue(3,1,0);
        ASSERT_DOUBLES_EQUAL( 16.0, nVal);
        nVal = m_pDoc->GetValue(4,1,0);
        ASSERT_DOUBLES_EQUAL( 16.0, nVal);
        nVal = m_pDoc->GetValue(5,1,0);
        ASSERT_DOUBLES_EQUAL( 17.0, nVal);
        nVal = m_pDoc->GetValue(6,1,0);
        ASSERT_DOUBLES_EQUAL( 17.0, nVal);

        // Enter new value and check that it is broadcasted. Now overwriting data.
        m_pDoc->SetString(1,0,0, u"32"_ustr);
        nVal = m_pDoc->GetValue(3,0,0);
        ASSERT_DOUBLES_EQUAL( 32.0, nVal);
        nVal = m_pDoc->GetValue(4,0,0);
        ASSERT_DOUBLES_EQUAL( 32.0, nVal);
        nVal = m_pDoc->GetValue(5,0,0);
        ASSERT_DOUBLES_EQUAL( 34.0, nVal);
        nVal = m_pDoc->GetValue(6,0,0);
        ASSERT_DOUBLES_EQUAL( 34.0, nVal);
    }

    // The same for sort by column. Start data at A5.

    {
        const std::vector<std::vector<const char*>> aData = {
            { "1", "2" },
            {   nullptr, "8" },
            { nullptr, nullptr },
            { "=A6",             "=B6" },
            { "=$A$6",           "=$B$6" },
            { "=SUM(A5:A6)",     "=SUM(B5:B6)" },
            { "=SUM($A$5:$A$6)", "=SUM($B$5:$B$6)" },
        };

        ScAddress aPos(0,4,0);
        ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL(aPos, aDataRange.aStart);

        {
            // Expected output table content.  0 = empty cell
            std::vector<std::vector<const char*>> aOutputCheck = {
                { "1", "2" },
                {   nullptr, "8" },
                { nullptr, nullptr },
                { "0",  "8" },
                { "0",  "8" },
                { "1", "10" },
                { "1", "10" },
            };

            bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Initial value");
            CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
        }

        // Sort A5:B6.
        m_pDoc->SetAnonymousDBData(
                0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 4, 1, 5)));

        ScDBDocFunc aFunc(*m_xDocShell);

        // Sort A5:B6 by row 5 descending.
        ScSortParam aSortData;
        aSortData.nCol1 = 0;
        aSortData.nCol2 = 1;
        aSortData.nRow1 = 4;
        aSortData.nRow2 = 5;
        aSortData.bHasHeader = false;
        aSortData.bByRow = false;
        aSortData.maKeyState[0].bDoSort = true;
        aSortData.maKeyState[0].nField = 0;
        aSortData.maKeyState[0].bAscending = false;
        aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
        bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
        CPPUNIT_ASSERT(bSorted);

        {
            // Expected output table content.  0 = empty cell
            std::vector<std::vector<const char*>> aOutputCheck = {
                { "2", "1" },
                { "8",   nullptr },
                { nullptr, nullptr },
                { "8",  "0" },
                { "8",  "0" },
                { "10", "1" },
                { "10", "1" },
            };

            bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Sorted without reference update");
            CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
        }

        // Make sure that the formulas in A8:B11 are not adjusted.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=A6"_ustr, m_pDoc->GetFormula(0,7,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=B6"_ustr, m_pDoc->GetFormula(1,7,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=$A$6"_ustr, m_pDoc->GetFormula(0,8,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=$B$6"_ustr, m_pDoc->GetFormula(1,8,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A5:A6)"_ustr, m_pDoc->GetFormula(0,9,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(B5:B6)"_ustr, m_pDoc->GetFormula(1,9,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM($A$5:$A$6)"_ustr, m_pDoc->GetFormula(0,10,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM($B$5:$B$6)"_ustr, m_pDoc->GetFormula(1,10,0));

        // Enter new value and check that it is broadcasted. First in empty cell.
        m_pDoc->SetString(1,5,0, u"16"_ustr);
        double nVal = m_pDoc->GetValue(1,7,0);
        ASSERT_DOUBLES_EQUAL(16.0, nVal);
        nVal = m_pDoc->GetValue(1,8,0);
        ASSERT_DOUBLES_EQUAL(16.0, nVal);
        nVal = m_pDoc->GetValue(1,9,0);
        ASSERT_DOUBLES_EQUAL(17.0, nVal);
        nVal = m_pDoc->GetValue(1,10,0);
        ASSERT_DOUBLES_EQUAL(17.0, nVal);

        // Enter new value and check that it is broadcasted. Now overwriting data.
        m_pDoc->SetString(0,5,0, u"32"_ustr);
        nVal = m_pDoc->GetValue(0,7,0);
        ASSERT_DOUBLES_EQUAL(32.0, nVal);
        nVal = m_pDoc->GetValue(0,8,0);
        ASSERT_DOUBLES_EQUAL(32.0, nVal);
        nVal = m_pDoc->GetValue(0,9,0);
        ASSERT_DOUBLES_EQUAL(34.0, nVal);
        nVal = m_pDoc->GetValue(0,10,0);
        ASSERT_DOUBLES_EQUAL(34.0, nVal);
    }

    m_pDoc->DeleteTab(0);
}

// tdf#99417 check that formulas are tracked that *only* indirectly depend on
// sorted data and no other broadcasting than BroadcastBroadcasters is
// involved (for which this test can not be included in testSortBroadcaster()).
CPPUNIT_TEST_FIXTURE(TestSort, testSortBroadcastBroadcaster)
{
    SortRefNoUpdateSetter aUpdateSet;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, u"Sort"_ustr);

    {
        const std::vector<std::vector<const char*>> aData = {
            { "1", "=A1", "=B1" },
            { "2", "=A2", "=B2" },
        };

        ScAddress aPos(0,0,0);
        ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL(aPos, aDataRange.aStart);

        {
            // Expected output table content.  0 = empty cell
            std::vector<std::vector<const char*>> aOutputCheck = {
                { "1", "1", "1" },
                { "2", "2", "2" },
            };

            bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Initial value");
            CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
        }

        // Sort A1:A2.
        m_pDoc->SetAnonymousDBData(
                0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 0, 1)));

        ScDBDocFunc aFunc(*m_xDocShell);

        // Sort A1:A2 by column A descending.
        ScSortParam aSortData;
        aSortData.nCol1 = 0;
        aSortData.nCol2 = 0;
        aSortData.nRow1 = 0;
        aSortData.nRow2 = 1;
        aSortData.bHasHeader = false;
        aSortData.bByRow = true;
        aSortData.maKeyState[0].bDoSort = true;
        aSortData.maKeyState[0].nField = 0;
        aSortData.maKeyState[0].bAscending = false;
        aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
        bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
        CPPUNIT_ASSERT(bSorted);

        {
            // Expected output table content.  0 = empty cell
            std::vector<std::vector<const char*>> aOutputCheck = {
                { "2", "2", "2" },
                { "1", "1", "1" },
            };

            bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Sorted without reference update");
            CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
        }

        // Make sure that the formulas in B1:C2 are not adjusted.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=A1"_ustr, m_pDoc->GetFormula(1,0,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=A2"_ustr, m_pDoc->GetFormula(1,1,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=B1"_ustr, m_pDoc->GetFormula(2,0,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=B2"_ustr, m_pDoc->GetFormula(2,1,0));
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortOutOfPlaceResult)
{
    m_pDoc->InsertTab(0, u"Sort"_ustr);
    m_pDoc->InsertTab(1, u"Result"_ustr);

    const char* pData[] = {
        "Header",
        "1",
        "23",
        "2",
        "9",
        "-2",
        nullptr // terminator
    };

    // source data in A1:A6.
    for (SCROW i = 0; pData[i]; ++i)
        m_pDoc->SetString(ScAddress(0,i,0), OUString::createFromAscii(pData[i]));

    // Check the initial values.
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(23.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL( 9.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(-2.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    ScDBDocFunc aFunc(*m_xDocShell);

    // Sort A1:A6, and set the result to C2:C7
    m_pDoc->SetAnonymousDBData(
        0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 0, 5)));

    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 0;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 5;
    aSortData.bHasHeader = true;
    aSortData.bInplace = false;
    aSortData.nDestTab = 1;
    aSortData.nDestCol = 2;
    aSortData.nDestRow = 1;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // Source data still intact.
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(23.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL( 9.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(-2.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Sort result in C2:C7 on sheet "Result".
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, m_pDoc->GetString(ScAddress(2,1,1)));
    CPPUNIT_ASSERT_EQUAL(-2.0, m_pDoc->GetValue(ScAddress(2,2,1)));
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(2,3,1)));
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(2,4,1)));
    CPPUNIT_ASSERT_EQUAL( 9.0, m_pDoc->GetValue(ScAddress(2,5,1)));
    CPPUNIT_ASSERT_EQUAL(23.0, m_pDoc->GetValue(ScAddress(2,6,1)));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortPartialFormulaGroup)
{
    SortRefUpdateSetter aUpdateSet;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    m_pDoc->InsertTab(0, u"Sort"_ustr);

    // Set up the sheet.
    const char* aData[][2] = {
        { "F1", "F2" },
        { "43", "=RC[-1]" },
        { "50", "=RC[-1]" },
        {  "8", "=RC[-1]" },
        { "47", "=RC[-1]" },
        { "28", "=RC[-1]" },
        { nullptr, nullptr } // terminator
    };

    // A1:B6.
    for (SCROW i = 0; aData[i][0]; ++i)
    {
        m_pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i][0]));
        m_pDoc->SetString(1, i, 0, OUString::createFromAscii(aData[i][1]));
    }

    // Check the initial condition.
    for (SCROW i = 1; i <= 5; ++i)
        // A2:A6 should equal B2:B6.
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetValue(ScAddress(0,i,0)), m_pDoc->GetValue(ScAddress(1,i,0)));

    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,1,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_MESSAGE("This formula cell should be the first in a group.", pFC->IsSharedTop());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect formula group length.", static_cast<SCROW>(5), pFC->GetSharedLength());

    ScDBDocFunc aFunc(*m_xDocShell);

    // Sort only B2:B4.  This caused crash at one point (c.f. fdo#81617).

    m_pDoc->SetAnonymousDBData(0, std::unique_ptr<ScDBData>(new ScDBData(STR_DB_LOCAL_NONAME, 0, 1, 1, 1, 3)));

    ScSortParam aSortData;
    aSortData.nCol1 = 1;
    aSortData.nCol2 = 1;
    aSortData.nRow1 = 1;
    aSortData.nRow2 = 3;
    aSortData.bHasHeader = false;
    aSortData.bInplace = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    m_pDoc->CalcAll(); // just in case...

    // Check the cell values after the partial sort.

    // Column A
    CPPUNIT_ASSERT_EQUAL(43.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(50.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL( 8.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(47.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Column B
    CPPUNIT_ASSERT_EQUAL( 8.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(43.0, m_pDoc->GetValue(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(50.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(47.0, m_pDoc->GetValue(ScAddress(1,4,0)));
    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(1,5,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSort, testSortImages)
{
    m_pDoc->InsertTab(0, u"testSortImages"_ustr);

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());
    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    ScAddress aPos(0,0,0);
    {
        const std::vector<std::vector<const char*>> aData = {
            { "2" },
            { "1" },
        };

        clearRange(m_pDoc, ScRange(0, 0, 0, 1, aData.size(), 0));
        ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aDataRange.aStart);
    }

    // Insert graphic in cell B2.
    const tools::Rectangle aOrigRect(1000, 1000, 1200, 1200);
    rtl::Reference<SdrCircObj> pObj = new SdrCircObj(*pDrawLayer, SdrCircKind::Full, aOrigRect);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    pPage->InsertObject(pObj.get());
    // Anchor to cell
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0, false);
    // Move to cell B2
    ScAddress aCellPos(1, 1, 0);
    pDrawLayer->MoveObject(pObj.get(), aCellPos);

    std::map<SCROW, std::vector<SdrObject*>> pRowObjects
        = pDrawLayer->GetObjectsAnchoredToRange(aCellPos.Tab(), aCellPos.Col(), aCellPos.Row(), aCellPos.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pRowObjects[aCellPos.Row()].size());

    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 1;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 1;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;
    aSortData.maKeyState[0].aColorSortMode = ScColorSortMode::None;

    m_pDoc->Sort(0, aSortData, false, true, nullptr, nullptr);

    double nVal = m_pDoc->GetValue(0,0,0);
    ASSERT_DOUBLES_EQUAL(1.0, nVal);

    // check that note is also moved after sorting
    aCellPos = ScAddress(1, 0, 0);
    pRowObjects
        = pDrawLayer->GetObjectsAnchoredToRange(aCellPos.Tab(), aCellPos.Col(), aCellPos.Row(), aCellPos.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pRowObjects[aCellPos.Row()].size());

    m_pDoc->DeleteTab(0);
}

namespace
{

class TestQueryIterator
    : public ScQueryCellIteratorBase< ScQueryCellIteratorAccess::Direct, ScQueryCellIteratorType::Generic >
{
    typedef ScQueryCellIteratorBase< ScQueryCellIteratorAccess::Direct, ScQueryCellIteratorType::Generic > Base;
public:
    TestQueryIterator( ScDocument& rDocument, ScInterpreterContext& rContext, SCTAB nTable,
        const ScQueryParam& aParam, bool bMod, bool bReverse = false )
    : Base( rDocument, rContext, nTable, aParam, bMod, bReverse )
    {
    }
    using Base::BinarySearch; // make public
    SCROW GetRow() const { return nRow; }
};

ScQueryParam makeSearchParam( const ScRange& range, SCCOL col, ScQueryOp op, double value )
{
    ScQueryParam param;
    param.nCol1 = param.nCol2 = col;
    param.nRow1 = range.aStart.Row();
    param.nRow2 = range.aEnd.Row();
    param.nTab = 0;
    ScQueryEntry& entry = param.GetEntry(0);
    ScQueryEntry::Item& item = entry.GetQueryItem();
    entry.bDoQuery = true;
    entry.eOp = op;
    item.mfVal = value;
    item.meType = ScQueryEntry::ByValue;
    return param;
}

} // namespace

CPPUNIT_TEST_FIXTURE(TestSort, testQueryBinarySearch)
{
    m_pDoc->InsertTab(0, u"testQueryBinarySearch"_ustr);

    const ScAddress formulaAddress( 10, 0, 0 );
    ScRange range;
    SCCOL ascendingCol;
    SCCOL descendingCol;
    OUString ascendingRangeName;
    OUString descendingRangeName;
    {
        const std::vector<std::vector<const char*>> data = {
            { "1", "9" },  // 0
            { "2", "9" },  // 1
            { "4", "5" },  // 2
            { "5", "5" },  // 3
            { "5", "5" },  // 4
            { "5", "5" },  // 5
            { "5", "5" },  // 6
            { "5", "4" },  // 7
            { "5", "4" },  // 8
            { "9", "2" },  // 9
            { "9", "1" },  // 10
        };
        ascendingCol = 0;
        descendingCol = 1;
        ascendingRangeName = u"$A$1:$A$" + OUString::number(data.size());
        descendingRangeName = u"$B$1:$B$" + OUString::number(data.size());

        ScAddress pos(0,0,0);
        range = insertRangeData(m_pDoc, pos, data);
        CPPUNIT_ASSERT_EQUAL( ScRange( 0, 0, 0, data[ 0 ].size() - 1, data.size() - 1, 0 ), range );
    }

    {
        // This should return the last 5.
        m_pDoc->SetFormula( formulaAddress, "=MATCH(5;" + ascendingRangeName + ";1)",
            formula::FormulaGrammar::GRAM_NATIVE_UI);
        CPPUNIT_ASSERT_EQUAL( 9.0, m_pDoc->GetValue( formulaAddress ));

        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_LESS_EQUAL, 5 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_LESS, 5 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(2), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_EQUAL, 5 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), it.GetRow());
    }

    {
        // Descending, this should return the last 5.
        m_pDoc->SetFormula( formulaAddress, "=MATCH(5;" + descendingRangeName + ";-1)",
            formula::FormulaGrammar::GRAM_NATIVE_UI);
        CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue( formulaAddress ));

        ScQueryParam param = makeSearchParam( range, descendingCol, SC_GREATER_EQUAL, 5 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( descendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(6), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, descendingCol, SC_GREATER, 5 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( descendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(1), it.GetRow());
    }

    {
        // There's no 6, so this should return the last 5.
        m_pDoc->SetFormula( formulaAddress, "=MATCH(6;" + ascendingRangeName + ";1)",
            formula::FormulaGrammar::GRAM_NATIVE_UI);
        CPPUNIT_ASSERT_EQUAL( 9.0, m_pDoc->GetValue( formulaAddress ));

        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_LESS_EQUAL, 6 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_LESS, 6 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_EQUAL, 6 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), it.GetRow());
    }

    {
        // Descending, there's no 6, so this should return the last 9.
        m_pDoc->SetFormula( formulaAddress, "=MATCH(6;" + descendingRangeName + ";-1)",
            formula::FormulaGrammar::GRAM_NATIVE_UI);
        CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue( formulaAddress ));

        ScQueryParam param = makeSearchParam( range, descendingCol, SC_GREATER_EQUAL, 6 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( descendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(1), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, descendingCol, SC_GREATER, 6 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( descendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(1), it.GetRow());
    }

    {
        // All values are larger than 0, so there should be no match.
        m_pDoc->SetFormula( formulaAddress, "=MATCH(0;" + ascendingRangeName + ";1)",
            formula::FormulaGrammar::GRAM_NATIVE_UI);
        CPPUNIT_ASSERT_EQUAL( FormulaError::NotAvailable, m_pDoc->GetErrCode( formulaAddress ));

        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_LESS_EQUAL, 0 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(!it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(0), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_LESS, 0 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(!it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(0), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_EQUAL, 0 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(!it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(0), it.GetRow());
    }

    {
        // Descending, all values are larger than 0, so this should return the last item.
        m_pDoc->SetFormula( formulaAddress, "=MATCH(0;" + descendingRangeName + ";-1)",
            formula::FormulaGrammar::GRAM_NATIVE_UI);
        CPPUNIT_ASSERT_EQUAL( 11.0, m_pDoc->GetValue( formulaAddress ));

        ScQueryParam param = makeSearchParam( range, descendingCol, SC_GREATER_EQUAL, 0 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( descendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(10), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, descendingCol, SC_GREATER, 0 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( descendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(10), it.GetRow());
    }

    {
        // All values are smaller than 10, so this should return the last item.
        m_pDoc->SetFormula( formulaAddress, "=MATCH(10;" + ascendingRangeName + ";1)",
            formula::FormulaGrammar::GRAM_NATIVE_UI);
        CPPUNIT_ASSERT_EQUAL( 11.0, m_pDoc->GetValue( formulaAddress ));

        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_LESS_EQUAL, 10 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(10), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_LESS, 10 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(10), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_EQUAL, 10 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(10), it.GetRow());
    }

    {
        // Descending, all values are smaller than 10, so there should be no match.
        m_pDoc->SetFormula( formulaAddress, "=MATCH(10;" + descendingRangeName + ";-1)",
            formula::FormulaGrammar::GRAM_NATIVE_UI);
        CPPUNIT_ASSERT_EQUAL( FormulaError::NotAvailable, m_pDoc->GetErrCode( formulaAddress ));

        ScQueryParam param = makeSearchParam( range, descendingCol, SC_GREATER_EQUAL, 10 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(!it.BinarySearch( descendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(0), it.GetRow());
    }
    {
        ScQueryParam param = makeSearchParam( range, descendingCol, SC_GREATER, 10 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(!it.BinarySearch( descendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(0), it.GetRow());
    }

    {
        // Search as ascending but use descending range, will return no match.
        ScQueryParam param = makeSearchParam( range, descendingCol, SC_LESS_EQUAL, 1 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(!it.BinarySearch( descendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(0), it.GetRow());
    }

    {
        // Search as descending but use ascending range, will return no match.
        ScQueryParam param = makeSearchParam( range, ascendingCol, SC_GREATER_EQUAL, 9 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(!it.BinarySearch( ascendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(0), it.GetRow());
    }

    {
        // SC_EQUAL with descending is considered an error, will return no match.
        ScQueryParam param = makeSearchParam( range, descendingCol, SC_EQUAL, 9 );
        TestQueryIterator it( *m_pDoc, m_pDoc->GetNonThreadedContext(), 0, param, false );
        CPPUNIT_ASSERT(!it.BinarySearch( descendingCol ));
        CPPUNIT_ASSERT_EQUAL(SCROW(0), it.GetRow());
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
