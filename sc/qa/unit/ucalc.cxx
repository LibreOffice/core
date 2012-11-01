/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):  Michael Meeks <michael.meeks@novell.com>
 *                  Kohei Yoshida <kyoshida@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include "scdll.hxx"
#include "document.hxx"
#include "stringutil.hxx"
#include "scmatrix.hxx"
#include "drwlayer.hxx"
#include "scitems.hxx"
#include "reffind.hxx"
#include "markdata.hxx"
#include "clipparam.hxx"
#include "refundo.hxx"
#include "undoblk.hxx"
#include "undotab.hxx"
#include "queryentry.hxx"
#include "postit.hxx"
#include "attrib.hxx"
#include "dbdata.hxx"
#include "reftokenhelper.hxx"
#include "userdat.hxx"

#include "docsh.hxx"
#include "docfunc.hxx"
#include "dbdocfun.hxx"
#include "funcdesc.hxx"
#include "externalrefmgr.hxx"

#include "dpshttab.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpdimsave.hxx"
#include "dpcache.hxx"
#include "dpfilteredcache.hxx"
#include "calcconfig.hxx"
#include "interpre.hxx"

#include "formula/IFunctionDescription.hxx"

#include <basegfx/polygon/b2dpolygon.hxx>
#include <editeng/boxitem.hxx>

#include <svx/svdograf.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>

#include <sfx2/docfile.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>

#include <iostream>
#include <sstream>
#include <vector>

#define CALC_DEBUG_OUTPUT 0

#include "helper/debughelper.hxx"

const int indeterminate = 2;

using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::std::cout;
using ::std::cerr;
using ::std::endl;
using ::std::vector;

namespace {

class Test : public test::BootstrapFixture {
public:
    Test();

    virtual void setUp();
    virtual void tearDown();

    void testCollator();
    void testRangeList();
    void testInput();
    void testCellFunctions();

    /**
     * Make sure the SHEETS function gets properly updated during sheet
     * insertion and removal.
     */
    void testSheetsFunc();
    void testVolatileFunc();
    void testFormulaDepTracking();
    void testFuncParam();
    void testNamedRange();
    void testCSV();
    void testMatrix();

    /**
     * Basic test for pivot tables.
     */
    void testPivotTable();

    /**
     * Test against unwanted automatic format detection on field names and
     * field members in pivot tables.
     */
    void testPivotTableLabels();

    /**
     * Make sure that we set cells displaying date values numeric cells,
     * rather than text cells.  Grouping by date or number functionality
     * depends on this.
     */
    void testPivotTableDateLabels();

    /**
     * Test for pivot table's filtering functionality by page fields.
     */
    void testPivotTableFilters();

    /**
     * Test for pivot table's named source range.
     */
    void testPivotTableNamedSource();

    /**
     * Test for pivot table cache.  Each dimension in the pivot cache stores
     * only unique values that are sorted in ascending order.
     */
    void testPivotTableCache();

    /**
     * Test for pivot table containing data fields that reference the same
     * source field but different functions.
     */
    void testPivotTableDuplicateDataFields();

    void testPivotTableNormalGrouping();
    void testPivotTableNumberGrouping();
    void testPivotTableDateGrouping();
    void testPivotTableEmptyRows();
    void testPivotTableTextNumber();

    /**
     * Test for checking that pivot table treats strings in a case insensitive
     * manner.
     */
    void testPivotTableCaseInsensitiveStrings();

    void testSheetCopy();
    void testSheetMove();
    void testExternalRef();
    void testExternalRefFunctions();
    void testDataArea();
    void testAutofilter();
    void testCopyPaste();
    void testMergedCells();
    void testUpdateReference();

    /**
     * Make sure the sheet streams are invalidated properly.
     */
    void testStreamValid();

    /**
     * Test built-in cell functions to make sure their categories and order
     * are correct.
     */
    void testFunctionLists();

    void testGraphicsInGroup();
    void testGraphicsOnSheetMove();

    void testPostIts();

    /**
     * Test toggling relative/absolute flag of cell and cell range references.
     * This corresponds with hitting Shift-F4 while the cursor is on a formula
     * cell.
     */
    void testToggleRefFlag();

    /**
     * Test to make sure correct precedent / dependent cells are obtained when
     * preparing to jump to them.
     */
    void testJumpToPrecedentsDependents();

    void testSetBackgroundColor();
    void testRenameTable();

    void testAutoFill();
    void testCopyPasteFormulas();
    void testCopyPasteFormulasExternalDoc();

    void testFindAreaPosRowDown();
    void testFindAreaPosColRight();
    void testSort();
    void testSortWithFormulaRefs();
    void testDeleteRow();
    void testDeleteCol();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testCollator);
    CPPUNIT_TEST(testRangeList);
    CPPUNIT_TEST(testInput);
    CPPUNIT_TEST(testCellFunctions);
    CPPUNIT_TEST(testSheetsFunc);
    CPPUNIT_TEST(testVolatileFunc);
    CPPUNIT_TEST(testFormulaDepTracking);
    CPPUNIT_TEST(testFuncParam);
    CPPUNIT_TEST(testNamedRange);
    CPPUNIT_TEST(testCSV);
    CPPUNIT_TEST(testMatrix);
    CPPUNIT_TEST(testPivotTable);
    CPPUNIT_TEST(testPivotTableLabels);
    CPPUNIT_TEST(testPivotTableDateLabels);
    CPPUNIT_TEST(testPivotTableFilters);
    CPPUNIT_TEST(testPivotTableNamedSource);
    CPPUNIT_TEST(testPivotTableCache);
    CPPUNIT_TEST(testPivotTableDuplicateDataFields);
    CPPUNIT_TEST(testPivotTableNormalGrouping);
    CPPUNIT_TEST(testPivotTableNumberGrouping);
    CPPUNIT_TEST(testPivotTableDateGrouping);
    CPPUNIT_TEST(testPivotTableEmptyRows);
    CPPUNIT_TEST(testPivotTableTextNumber);
    CPPUNIT_TEST(testPivotTableCaseInsensitiveStrings);
    CPPUNIT_TEST(testSheetCopy);
    CPPUNIT_TEST(testSheetMove);
    CPPUNIT_TEST(testExternalRef);
    CPPUNIT_TEST(testExternalRefFunctions);
    CPPUNIT_TEST(testDataArea);
    CPPUNIT_TEST(testGraphicsInGroup);
    CPPUNIT_TEST(testGraphicsOnSheetMove);
    CPPUNIT_TEST(testPostIts);
    CPPUNIT_TEST(testStreamValid);
    CPPUNIT_TEST(testFunctionLists);
    CPPUNIT_TEST(testToggleRefFlag);
    CPPUNIT_TEST(testAutofilter);
    CPPUNIT_TEST(testCopyPaste);
    CPPUNIT_TEST(testMergedCells);
    CPPUNIT_TEST(testUpdateReference);
    CPPUNIT_TEST(testJumpToPrecedentsDependents);
    CPPUNIT_TEST(testSetBackgroundColor);
    CPPUNIT_TEST(testRenameTable);
    CPPUNIT_TEST(testAutoFill);
    CPPUNIT_TEST(testCopyPasteFormulas);
    CPPUNIT_TEST(testCopyPasteFormulasExternalDoc);
    CPPUNIT_TEST(testFindAreaPosRowDown);
    CPPUNIT_TEST(testFindAreaPosColRight);
    CPPUNIT_TEST(testSort);
    CPPUNIT_TEST(testSortWithFormulaRefs);
    CPPUNIT_TEST(testDeleteRow);
    CPPUNIT_TEST(testDeleteCol);
    CPPUNIT_TEST_SUITE_END();

private:
    ScDocument *m_pDoc;
    ScDocShellRef m_xDocShRef;
};

void clearRange(ScDocument* pDoc, const ScRange& rRange)
{
    ScMarkData aMarkData;
    aMarkData.SetMarkArea(rRange);
    pDoc->DeleteArea(
        rRange.aStart.Col(), rRange.aStart.Row(),
        rRange.aEnd.Col(), rRange.aEnd.Row(), aMarkData, IDF_CONTENTS);
}

void printRange(ScDocument* pDoc, const ScRange& rRange, const char* pCaption)
{
    SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();
    SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
    SheetPrinter printer(nRow2 - nRow1 + 1, nCol2 - nCol1 + 1);
    for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
    {
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        {
            rtl::OUString aVal;
            pDoc->GetString(nCol, nRow, rRange.aStart.Tab(), aVal);
            printer.set(nRow-nRow1, nCol-nCol1, aVal);
        }
    }
    printer.print(pCaption);
}

template<size_t _Size>
ScRange insertRangeData(ScDocument* pDoc, const ScAddress& rPos, const char* aData[][_Size], size_t nRowCount)
{
    for (size_t i = 0; i < _Size; ++i)
    {
        for (size_t j = 0; j < nRowCount; ++j)
        {
            SCCOL nCol = i + rPos.Col();
            SCROW nRow = j + rPos.Row();
            pDoc->SetString(nCol, nRow, rPos.Tab(), OUString(aData[j][i], strlen(aData[j][i]), RTL_TEXTENCODING_UTF8));
        }
    }

    ScRange aRange(rPos);
    aRange.aEnd.SetCol(rPos.Col()+_Size-1);
    aRange.aEnd.SetRow(rPos.Row()+nRowCount-1);
    printRange(pDoc, aRange, "Range data content");
    return aRange;
}

/**
 * Temporarily switch on/off auto calculation mode.
 */
class AutoCalcSwitch
{
    ScDocument* mpDoc;
    bool mbOldValue;
public:
    AutoCalcSwitch(ScDocument* pDoc, bool bAutoCalc) : mpDoc(pDoc), mbOldValue(pDoc->GetAutoCalc())
    {
        mpDoc->SetAutoCalc(bAutoCalc);
    }

    ~AutoCalcSwitch()
    {
        mpDoc->SetAutoCalc(mbOldValue);
    }
};

/**
 * Temporarily set formula grammar.
 */
class FormulaGrammarSwitch
{
    ScDocument* mpDoc;
    formula::FormulaGrammar::Grammar meOldGrammar;
public:
    FormulaGrammarSwitch(ScDocument* pDoc, formula::FormulaGrammar::Grammar eGrammar) :
        mpDoc(pDoc), meOldGrammar(pDoc->GetGrammar())
    {
        mpDoc->SetGrammar(eGrammar);
    }

    ~FormulaGrammarSwitch()
    {
        mpDoc->SetGrammar(meOldGrammar);
    }
};

Test::Test()
    : m_pDoc(0)
{
}

void Test::setUp()
{
    BootstrapFixture::setUp();

    ScDLL::Init();
    m_xDocShRef = new ScDocShell(
        SFXMODEL_STANDARD |
        SFXMODEL_DISABLE_EMBEDDED_SCRIPTS |
        SFXMODEL_DISABLE_DOCUMENT_RECOVERY);

    m_pDoc = m_xDocShRef->GetDocument();
}

void Test::tearDown()
{
    m_xDocShRef.Clear();
    BootstrapFixture::tearDown();
}

void Test::testCollator()
{
    OUString s1("A");
    OUString s2("B");
    CollatorWrapper* p = ScGlobal::GetCollator();
    sal_Int32 nRes = p->compareString(s1, s2);
    CPPUNIT_ASSERT_MESSAGE("these strings are supposed to be different!", nRes != 0);
}

void Test::testRangeList()
{
    m_pDoc->InsertTab(0, "foo");

    ScRangeList aRL;
    aRL.Append(ScRange(1,1,0,3,10,0));
    CPPUNIT_ASSERT_MESSAGE("List should have one range.", aRL.size() == 1);
    const ScRange* p = aRL[0];
    CPPUNIT_ASSERT_MESSAGE("Failed to get the range object.", p);
    CPPUNIT_ASSERT_MESSAGE("Wrong range.", p->aStart == ScAddress(1,1,0) && p->aEnd == ScAddress(3,10,0));

    // TODO: Add more tests here.

    m_pDoc->DeleteTab(0);
}

void Test::testInput()
{
    rtl::OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    OUString numstr("'10.5");
    OUString str("'apple'");
    OUString test;

    m_pDoc->SetString(0, 0, 0, numstr);
    m_pDoc->GetString(0, 0, 0, test);
    bool bTest = test == "10.5";
    CPPUNIT_ASSERT_MESSAGE("String number should have the first apostrophe stripped.", bTest);
    m_pDoc->SetString(0, 0, 0, str);
    m_pDoc->GetString(0, 0, 0, test);
    bTest = test == "'apple'";
    CPPUNIT_ASSERT_MESSAGE("Text content should have retained the first apostrophe.", bTest);

    // Customized string handling policy.
    ScSetStringParam aParam;
    aParam.mbDetectNumberFormat = false;
    aParam.meSetTextNumFormat = ScSetStringParam::Always;
    aParam.mbHandleApostrophe = false;
    m_pDoc->SetString(0, 0, 0, "000123", &aParam);
    m_pDoc->GetString(0, 0, 0, test);
    CPPUNIT_ASSERT_MESSAGE("Text content should have been treated as string, not number.", test == "000123");

    m_pDoc->DeleteTab(0);
}

void testFuncSUM(ScDocument* pDoc)
{
    double val = 1;
    double result;
    pDoc->SetValue (0, 0, 0, val);
    pDoc->SetValue (0, 1, 0, val);
    pDoc->SetString (0, 2, 0, rtl::OUString("=SUM(A1:A2)"));
    pDoc->CalcAll();
    pDoc->GetValue (0, 2, 0, result);
    CPPUNIT_ASSERT_MESSAGE ("calculation failed", result == 2.0);
}

void testFuncPRODUCT(ScDocument* pDoc)
{
    double val = 1;
    double result;
    pDoc->SetValue(0, 0, 0, val);
    val = 2;
    pDoc->SetValue(0, 1, 0, val);
    val = 3;
    pDoc->SetValue(0, 2, 0, val);
    pDoc->SetString(0, 3, 0, OUString("=PRODUCT(A1:A3)"));
    pDoc->CalcAll();
    pDoc->GetValue(0, 3, 0, result);
    CPPUNIT_ASSERT_MESSAGE("Calculation of PRODUCT failed", result == 6.0);

    pDoc->SetString(0, 4, 0, OUString("=PRODUCT({1;2;3})"));
    pDoc->CalcAll();
    pDoc->GetValue(0, 4, 0, result);
    CPPUNIT_ASSERT_MESSAGE("Calculation of PRODUCT with inline array failed", result == 6.0);
}

void testFuncN(ScDocument* pDoc)
{
    double result;

    // Clear the area first.
    clearRange(pDoc, ScRange(0, 0, 0, 1, 20, 0));

    // Put values to reference.
    double val = 0;
    pDoc->SetValue(0, 0, 0, val);
    pDoc->SetString(0, 2, 0, OUString("Text"));
    val = 1;
    pDoc->SetValue(0, 3, 0, val);
    val = -1;
    pDoc->SetValue(0, 4, 0, val);
    val = 12.3;
    pDoc->SetValue(0, 5, 0, val);
    pDoc->SetString(0, 6, 0, OUString("'12.3"));

    // Cell references
    pDoc->SetString(1, 0, 0, OUString("=N(A1)"));
    pDoc->SetString(1, 1, 0, OUString("=N(A2)"));
    pDoc->SetString(1, 2, 0, OUString("=N(A3)"));
    pDoc->SetString(1, 3, 0, OUString("=N(A4)"));
    pDoc->SetString(1, 4, 0, OUString("=N(A5)"));
    pDoc->SetString(1, 5, 0, OUString("=N(A6)"));
    pDoc->SetString(1, 6, 0, OUString("=N(A9)"));

    // In-line values
    pDoc->SetString(1, 7, 0, OUString("=N(0)"));
    pDoc->SetString(1, 8, 0, OUString("=N(1)"));
    pDoc->SetString(1, 9, 0, OUString("=N(-1)"));
    pDoc->SetString(1, 10, 0, OUString("=N(123)"));
    pDoc->SetString(1, 11, 0, OUString("=N(\"\")"));
    pDoc->SetString(1, 12, 0, OUString("=N(\"12\")"));
    pDoc->SetString(1, 13, 0, OUString("=N(\"foo\")"));

    // Range references
    pDoc->SetString(2, 2, 0, OUString("=N(A1:A8)"));
    pDoc->SetString(2, 3, 0, OUString("=N(A1:A8)"));
    pDoc->SetString(2, 4, 0, OUString("=N(A1:A8)"));
    pDoc->SetString(2, 5, 0, OUString("=N(A1:A8)"));

    // Calculate and check the results.
    pDoc->CalcAll();
    double checks1[] = {
        0, 0,  0,    1, -1, 12.3, 0, // cell reference
        0, 1, -1, 123,  0,    0, 0   // in-line values
    };
    for (size_t i = 0; i < SAL_N_ELEMENTS(checks1); ++i)
    {
        pDoc->GetValue(1, i, 0, result);
        bool bGood = result == checks1[i];
        if (!bGood)
        {
            cerr << "row " << (i+1) << ": expected=" << checks1[i] << " actual=" << result << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for N", false);
        }
    }
    double checks2[] = {
        0, 1, -1, 12.3               // range references
    };
    for (size_t i = 0; i < SAL_N_ELEMENTS(checks2); ++i)
    {
        pDoc->GetValue(1, i+2, 0, result);
        bool bGood = result == checks2[i];
        if (!bGood)
        {
            cerr << "row " << (i+2+1) << ": expected=" << checks2[i] << " actual=" << result << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for N", false);
        }
    }
}

void testFuncCOUNTIF(ScDocument* pDoc)
{
    // COUNTIF (test case adopted from OOo i#36381)

    // Empty A1:A39 first.
    clearRange(pDoc, ScRange(0, 0, 0, 0, 40, 0));

    // Raw data (rows 1 through 9)
    const char* aData[] = {
        "1999",
        "2000",
        "0",
        "0",
        "0",
        "2002",
        "2001",
        "X",
        "2002"
    };

    SCROW nRows = SAL_N_ELEMENTS(aData);
    for (SCROW i = 0; i < nRows; ++i)
        pDoc->SetString(0, i, 0, rtl::OUString::createFromAscii(aData[i]));

    printRange(pDoc, ScRange(0, 0, 0, 0, 8, 0), "data range for COUNTIF");

    // formulas and results
    struct {
        const char* pFormula; double fResult;
    } aChecks[] = {
        { "=COUNTIF(A1:A12;1999)",       1 },
        { "=COUNTIF(A1:A12;2002)",       2 },
        { "=COUNTIF(A1:A12;1998)",       0 },
        { "=COUNTIF(A1:A12;\">=1999\")", 5 },
        { "=COUNTIF(A1:A12;\">1999\")",  4 },
        { "=COUNTIF(A1:A12;\"<2001\")",  5 },
        { "=COUNTIF(A1:A12;\">0\")",     5 },
        { "=COUNTIF(A1:A12;\">=0\")",    8 },
        { "=COUNTIF(A1:A12;0)",          3 },
        { "=COUNTIF(A1:A12;\"X\")",      1 },
        { "=COUNTIF(A1:A12;)",           3 }
    };

    nRows = SAL_N_ELEMENTS(aChecks);
    for (SCROW i = 0; i < nRows; ++i)
    {
        SCROW nRow = 20 + i;
        pDoc->SetString(0, nRow, 0, rtl::OUString::createFromAscii(aChecks[i].pFormula));
    }
    pDoc->CalcAll();

    for (SCROW i = 0; i < nRows; ++i)
    {
        double result;
        SCROW nRow = 20 + i;
        pDoc->GetValue(0, nRow, 0, result);
        bool bGood = result == aChecks[i].fResult;
        if (!bGood)
        {
            cerr << "row " << (nRow+1) << ": formula" << aChecks[i].pFormula
                << "  expected=" << aChecks[i].fResult << "  actual=" << result << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for COUNTIF", false);
        }
    }

    // Don't count empty strings when searching for a number.

    // Clear A1:A2.
    clearRange(pDoc, ScRange(0, 0, 0, 0, 1, 0));

    pDoc->SetString(0, 0, 0, rtl::OUString("=\"\""));
    pDoc->SetString(0, 1, 0, rtl::OUString("=COUNTIF(A1;1)"));
    pDoc->CalcAll();

    double result = pDoc->GetValue(0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("We shouldn't count empty string as valid number.", result == 0.0);
}

void testFuncVLOOKUP(ScDocument* pDoc)
{
    // VLOOKUP

    // Clear A1:F40.
    clearRange(pDoc, ScRange(0, 0, 0, 5, 39, 0));

    // Raw data
    const char* aData[][2] = {
        { "Key", "Val" },
        {  "10",   "3" },
        {  "20",   "4" },
        {  "30",   "5" },
        {  "40",   "6" },
        {  "50",   "7" },
        {  "60",   "8" },
        {  "70",   "9" },
        {   "B",  "10" },
        {   "B",  "11" },
        {   "C",  "12" },
        {   "D",  "13" },
        {   "E",  "14" },
        {   "F",  "15" },
        { 0, 0 } // terminator
    };

    // Insert raw data into A1:B14.
    for (SCROW i = 0; aData[i][0]; ++i)
    {
        pDoc->SetString(0, i, 0, rtl::OUString::createFromAscii(aData[i][0]));
        pDoc->SetString(1, i, 0, rtl::OUString::createFromAscii(aData[i][1]));
    }

    printRange(pDoc, ScRange(0, 0, 0, 1, 13, 0), "raw data for VLOOKUP");

    // Formula data
    struct {
        const char* pLookup; const char* pFormula; const char* pRes;
    } aChecks[] = {
        { "Lookup",  "Formula", 0 },
        { "12",      "=VLOOKUP(D2;A2:B14;2;1)",     "3" },
        { "29",      "=VLOOKUP(D3;A2:B14;2;1)",     "4" },
        { "31",      "=VLOOKUP(D4;A2:B14;2;1)",     "5" },
        { "45",      "=VLOOKUP(D5;A2:B14;2;1)",     "6" },
        { "56",      "=VLOOKUP(D6;A2:B14;2;1)",     "7" },
        { "65",      "=VLOOKUP(D7;A2:B14;2;1)",     "8" },
        { "78",      "=VLOOKUP(D8;A2:B14;2;1)",     "9" },
        { "Andy",    "=VLOOKUP(D9;A2:B14;2;1)",  "#N/A" },
        { "Bruce",   "=VLOOKUP(D10;A2:B14;2;1)",   "11" },
        { "Charlie", "=VLOOKUP(D11;A2:B14;2;1)",   "12" },
        { "David",   "=VLOOKUP(D12;A2:B14;2;1)",   "13" },
        { "Edward",  "=VLOOKUP(D13;A2:B14;2;1)",   "14" },
        { "Frank",   "=VLOOKUP(D14;A2:B14;2;1)",   "15" },
        { "Henry",   "=VLOOKUP(D15;A2:B14;2;1)",   "15" },
        { "100",     "=VLOOKUP(D16;A2:B14;2;1)",    "9" },
        { "1000",    "=VLOOKUP(D17;A2:B14;2;1)",    "9" },
        { "Zena",    "=VLOOKUP(D18;A2:B14;2;1)",   "15" }
    };

    // Insert formula data into D1:E18.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        pDoc->SetString(3, i, 0, rtl::OUString::createFromAscii(aChecks[i].pLookup));
        pDoc->SetString(4, i, 0, rtl::OUString::createFromAscii(aChecks[i].pFormula));
    }
    pDoc->CalcAll();
    printRange(pDoc, ScRange(3, 0, 0, 4, 17, 0), "formula data for VLOOKUP");

    // Verify results.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        if (i == 0)
            // Skip the header row.
            continue;

        rtl::OUString aRes;
        pDoc->GetString(4, i, 0, aRes);
        bool bGood = aRes.equalsAscii(aChecks[i].pRes);
        if (!bGood)
        {
            cerr << "row " << (i+1) << ": lookup value='" << aChecks[i].pLookup
                << "'  expected='" << aChecks[i].pRes << "' actual='" << aRes << "'" << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for VLOOKUP", false);
        }
    }
}

struct NumStrCheck {
    double fVal;
    const char* pRes;
};

struct StrStrCheck {
    const char* pVal;
    const char* pRes;
};

template<size_t _DataSize, size_t _FormulaSize, int _Type>
void runTestMATCH(ScDocument* pDoc, const char* aData[_DataSize], StrStrCheck aChecks[_FormulaSize])
{
    size_t nDataSize = _DataSize;
    for (size_t i = 0; i < nDataSize; ++i)
        pDoc->SetString(0, i, 0, rtl::OUString::createFromAscii(aData[i]));

    for (size_t i = 0; i < _FormulaSize; ++i)
    {
        pDoc->SetString(1, i, 0, rtl::OUString::createFromAscii(aChecks[i].pVal));

        rtl::OUStringBuffer aBuf;
        aBuf.appendAscii("=MATCH(B");
        aBuf.append(static_cast<sal_Int32>(i+1));
        aBuf.appendAscii(";A1:A");
        aBuf.append(static_cast<sal_Int32>(nDataSize));
        aBuf.appendAscii(";");
        aBuf.append(static_cast<sal_Int32>(_Type));
        aBuf.appendAscii(")");
        rtl::OUString aFormula = aBuf.makeStringAndClear();
        pDoc->SetString(2, i, 0, aFormula);
    }

    pDoc->CalcAll();
    printRange(pDoc, ScRange(0, 0, 0, 2, _FormulaSize-1, 0), "MATCH");

    // verify the results.
    for (size_t i = 0; i < _FormulaSize; ++i)
    {
        rtl::OUString aStr;
        pDoc->GetString(2, i, 0, aStr);
        if (!aStr.equalsAscii(aChecks[i].pRes))
        {
            cerr << "row " << (i+1) << ": expected='" << aChecks[i].pRes << "' actual='" << aStr << "'"
                << " criterion='" << aChecks[i].pVal << "'" << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for MATCH", false);
        }
    }
}

void testFuncMATCH(ScDocument* pDoc)
{
    clearRange(pDoc, ScRange(0, 0, 0, 4, 40, 0));
    {
        // Ascending in-exact match

        // data range (A1:A9)
        const char* aData[] = {
            "1",
            "2",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8",
            "9",
            "B",
            "B",
            "C",
        };

        // formula (B1:C12)
        StrStrCheck aChecks[] = {
            { "0.8",   "#N/A" },
            { "1.2",      "1" },
            { "2.3",      "2" },
            { "3.9",      "3" },
            { "4.1",      "4" },
            { "5.99",     "5" },
            { "6.1",      "6" },
            { "7.2",      "7" },
            { "8.569",    "8" },
            { "9.59",     "9" },
            { "10",       "9" },
            { "100",      "9" },
            { "Andy",  "#N/A" },
            { "Bruce",   "11" },
            { "Charlie", "12" }
        };

        runTestMATCH<SAL_N_ELEMENTS(aData),SAL_N_ELEMENTS(aChecks),1>(pDoc, aData, aChecks);
    }

    {
        // Descending in-exact match

        // data range (A1:A9)
        const char* aData[] = {
            "D",
            "C",
            "B",
            "9",
            "8",
            "7",
            "6",
            "5",
            "4",
            "3",
            "2",
            "1"
        };

        // formula (B1:C12)
        StrStrCheck aChecks[] = {
            { "10",      "#N/A" },
            { "8.9",     "4" },
            { "7.8",     "5" },
            { "6.7",     "6" },
            { "5.5",     "7" },
            { "4.6",     "8" },
            { "3.3",     "9" },
            { "2.2",     "10" },
            { "1.1",     "11" },
            { "0.8",     "12" },
            { "0",       "12" },
            { "-2",      "12" },
            { "Andy",    "3" },
            { "Bruce",   "2" },
            { "Charlie", "1" },
            { "David", "#N/A" }
        };

        runTestMATCH<SAL_N_ELEMENTS(aData),SAL_N_ELEMENTS(aChecks),-1>(pDoc, aData, aChecks);
    }
}

void testFuncCELL(ScDocument* pDoc)
{
    clearRange(pDoc, ScRange(0, 0, 0, 2, 20, 0)); // Clear A1:C21.

    {
        const char* pContent = "Some random text";
        pDoc->SetString(2, 9, 0, rtl::OUString::createFromAscii(pContent)); // Set this value to C10.
        double val = 1.2;
        pDoc->SetValue(2, 0, 0, val); // Set numeric value to C1;

        // We don't test: FILENAME, FORMAT, WIDTH, PROTECT, PREFIX
        StrStrCheck aChecks[] = {
            { "=CELL(\"COL\";C10)",           "3" },
            { "=CELL(\"ROW\";C10)",          "10" },
            { "=CELL(\"SHEET\";C10)",         "1" },
            { "=CELL(\"ADDRESS\";C10)",   "$C$10" },
            { "=CELL(\"CONTENTS\";C10)", pContent },
            { "=CELL(\"COLOR\";C10)",         "0" },
            { "=CELL(\"TYPE\";C9)",           "b" },
            { "=CELL(\"TYPE\";C10)",          "l" },
            { "=CELL(\"TYPE\";C1)",           "v" },
            { "=CELL(\"PARENTHESES\";C10)",   "0" }
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
            pDoc->SetString(0, i, 0, rtl::OUString::createFromAscii(aChecks[i].pVal));
        pDoc->CalcAll();

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            rtl::OUString aVal = pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for CELL", aVal.equalsAscii(aChecks[i].pRes));
        }
    }
}

/** See also test case document fdo#44456 sheet cpearson */
void testFuncDATEDIF( ScDocument* pDoc )
{
    const char* aData[][5] = {
        { "2007-01-01", "2007-01-10",  "d",   "9", "=DATEDIF(A1;B1;C1)" } ,
        { "2007-01-01", "2007-01-31",  "m",   "0", "=DATEDIF(A2;B2;C2)" } ,
        { "2007-01-01", "2007-02-01",  "m",   "1", "=DATEDIF(A3;B3;C3)" } ,
        { "2007-01-01", "2007-02-28",  "m",   "1", "=DATEDIF(A4;B4;C4)" } ,
        { "2007-01-01", "2007-12-31",  "d", "364", "=DATEDIF(A5;B5;C5)" } ,
        { "2007-01-01", "2007-01-31",  "y",   "0", "=DATEDIF(A6;B6;C6)" } ,
        { "2007-01-01", "2008-07-01",  "d", "547", "=DATEDIF(A7;B7;C7)" } ,
        { "2007-01-01", "2008-07-01",  "m",  "18", "=DATEDIF(A8;B8;C8)" } ,
        { "2007-01-01", "2008-07-01", "ym",   "6", "=DATEDIF(A9;B9;C9)" } ,
        { "2007-01-01", "2008-07-01", "yd", "182", "=DATEDIF(A10;B10;C10)" } ,
        { "2008-01-01", "2009-07-01", "yd", "181", "=DATEDIF(A11;B11;C11)" } ,
        { "2007-01-01", "2007-01-31", "md",  "30", "=DATEDIF(A12;B12;C12)" } ,
        { "2007-02-01", "2009-03-01", "md",   "0", "=DATEDIF(A13;B13;C13)" } ,
        { "2008-02-01", "2009-03-01", "md",   "0", "=DATEDIF(A14;B14;C14)" } ,
        { "2007-01-02", "2007-01-01", "md", "Err:502", "=DATEDIF(A15;B15;C15)" }    // fail date1 > date2
    };

    clearRange( pDoc, ScRange(0, 0, 0, 4, SAL_N_ELEMENTS(aData), 0));
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    pDoc->CalcAll();

    for (size_t i = 0; i < SAL_N_ELEMENTS(aData); ++i)
    {
        rtl::OUString aVal = pDoc->GetString( 4, i, 0);
        //std::cout << "row "<< i << ": " << rtl::OUStringToOString( aVal, RTL_TEXTENCODING_UTF8).getStr() << ", expected " << aData[i][3] << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Unexpected result for DATEDIF", aVal.equalsAscii( aData[i][3]));
    }
}

void testFuncINDIRECT(ScDocument* pDoc)
{
    clearRange(pDoc, ScRange(0, 0, 0, 0, 10, 0)); // Clear A1:A11
    rtl::OUString aTabName;
    bool bGood = pDoc->GetName(0, aTabName);
    CPPUNIT_ASSERT_MESSAGE("failed to get sheet name.", bGood);

    rtl::OUString aTest = "Test", aRefErr = "#REF!";
    pDoc->SetString(0, 10, 0, aTest);
    CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", pDoc->GetString(0,10,0) == aTest);

    rtl::OUString aPrefix = "=INDIRECT(\"";

    rtl::OUString aFormula = aPrefix + aTabName + ".A11\")"; // Calc A1
    pDoc->SetString(0, 0, 0, aFormula);
    aFormula = aPrefix + aTabName + "!A11\")"; // Excel A1
    pDoc->SetString(0, 1, 0, aFormula);
    aFormula = aPrefix + aTabName + "!R11C1\")"; // Excel R1C1
    pDoc->SetString(0, 2, 0, aFormula);
    aFormula = aPrefix + aTabName + "!R11C1\";0)"; // Excel R1C1 (forced)
    pDoc->SetString(0, 3, 0, aFormula);

    pDoc->CalcAll();
    {
        // Default is to use the current formula syntax, which is Calc A1.
        const rtl::OUString* aChecks[] = {
            &aTest, &aRefErr, &aRefErr, &aTest
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            rtl::OUString aVal = pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_MESSAGE("Wrong value!", aVal == *aChecks[i]);
        }
    }

    ScCalcConfig aConfig;
    aConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_OOO;
    ScInterpreter::SetGlobalConfig(aConfig);
    pDoc->CalcAll();
    {
        // Explicit Calc A1 syntax
        const rtl::OUString* aChecks[] = {
            &aTest, &aRefErr, &aRefErr, &aTest
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            rtl::OUString aVal = pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_MESSAGE("Wrong value!", aVal == *aChecks[i]);
        }
    }

    aConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_XL_A1;
    ScInterpreter::SetGlobalConfig(aConfig);
    pDoc->CalcAll();
    {
        // Excel A1 syntax
        const rtl::OUString* aChecks[] = {
            &aRefErr, &aTest, &aRefErr, &aTest
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            rtl::OUString aVal = pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_MESSAGE("Wrong value!", aVal == *aChecks[i]);
        }
    }

    aConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_XL_R1C1;
    ScInterpreter::SetGlobalConfig(aConfig);
    pDoc->CalcAll();
    {
        // Excel R1C1 syntax
        const rtl::OUString* aChecks[] = {
            &aRefErr, &aRefErr, &aTest, &aTest
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            rtl::OUString aVal = pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_MESSAGE("Wrong value!", aVal == *aChecks[i]);
        }
    }
}

void Test::testCellFunctions()
{
    rtl::OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    testFuncSUM(m_pDoc);
    testFuncPRODUCT(m_pDoc);
    testFuncN(m_pDoc);
    testFuncCOUNTIF(m_pDoc);
    testFuncVLOOKUP(m_pDoc);
    testFuncMATCH(m_pDoc);
    testFuncCELL(m_pDoc);
    testFuncDATEDIF(m_pDoc);
    testFuncINDIRECT(m_pDoc);

    m_pDoc->DeleteTab(0);
}

void Test::testSheetsFunc()
{
    rtl::OUString aTabName1("test1");
    rtl::OUString aTabName2("test2");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (SC_TAB_APPEND, aTabName1));

    m_pDoc->SetString(0, 0, 0, OUString("=SHEETS()"));
    m_pDoc->CalcFormulaTree(false, true);
    double original;
    m_pDoc->GetValue(0, 0, 0, original);

    CPPUNIT_ASSERT_MESSAGE("result of SHEETS() should equal the number of sheets, but doesn't.",
                           static_cast<SCTAB>(original) == m_pDoc->GetTableCount());

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (SC_TAB_APPEND, aTabName2));

    double modified;
    m_pDoc->GetValue(0, 0, 0, modified);
    CPPUNIT_ASSERT_MESSAGE("result of SHEETS() did not get updated after sheet insertion.",
                           modified - original == 1.0);

    SCTAB nTabCount = m_pDoc->GetTableCount();
    m_pDoc->DeleteTab(--nTabCount);

    m_pDoc->GetValue(0, 0, 0, modified);
    CPPUNIT_ASSERT_MESSAGE("result of SHEETS() did not get updated after sheet removal.",
                           modified - original == 0.0);

    m_pDoc->DeleteTab(--nTabCount);
}

void Test::testVolatileFunc()
{
    rtl::OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    double val = 1;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->SetString(0, 1, 0, OUString("=IF(A1>0;NOW();0"));
    double now1;
    m_pDoc->GetValue(0, 1, 0, now1);
    CPPUNIT_ASSERT_MESSAGE("Value of NOW() should be positive.", now1 > 0.0);

    val = 0;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->CalcFormulaTree(false, true);
    double zero;
    m_pDoc->GetValue(0, 1, 0, zero);
    CPPUNIT_ASSERT_MESSAGE("Result should equal the 3rd parameter of IF, which is zero.", zero == 0.0);

    val = 1;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->CalcFormulaTree(false, true);
    double now2;
    m_pDoc->GetValue(0, 1, 0, now2);
    CPPUNIT_ASSERT_MESSAGE("Result should be the value of NOW() again.", (now2 - now1) >= 0.0);

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaDepTracking()
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "foo"));

    AutoCalcSwitch aACSwitch(m_pDoc, true); // turn on auto calculation.

    // B2 listens on D2.
    m_pDoc->SetString(1, 1, 0, "=D2");
    double val = -999.0; // dummy initial value
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Referencing an empty cell should yield zero.", val == 0.0);

    // Changing the value of D2 should trigger recalculation of B2.
    m_pDoc->SetValue(3, 1, 0, 1.1);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on value change.", val == 1.1);

    // And again.
    m_pDoc->SetValue(3, 1, 0, 2.2);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on value change.", val == 2.2);

    clearRange(m_pDoc, ScRange(0, 0, 0, 10, 10, 0));

    // Now, let's test the range dependency tracking.

    // B2 listens on D2:E6.
    m_pDoc->SetString(1, 1, 0, "=SUM(D2:E6)");
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Summing an empty range should yield zero.", val == 0.0);

    // Set value to E3. This should trigger recalc on B2.
    m_pDoc->SetValue(4, 2, 0, 2.4);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", val == 2.4);

    // Set value to D5 to trigger recalc again.  Note that this causes an
    // addition of 1.2 + 2.4 which is subject to binary floating point
    // rounding error.  We need to use approxEqual to assess its value.

    m_pDoc->SetValue(3, 4, 0, 1.2);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 3.6));

    // Change the value of D2 (boundary case).
    m_pDoc->SetValue(3, 1, 0, 1.0);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 4.6));

    // Change the value of E6 (another boundary case).
    m_pDoc->SetValue(4, 5, 0, 2.0);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 6.6));

    // Change the value of D6 (another boundary case).
    m_pDoc->SetValue(3, 5, 0, 3.0);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 9.6));

    // Change the value of E2 (another boundary case).
    m_pDoc->SetValue(4, 1, 0, 0.4);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 10.0));

    // Change the existing non-empty value cell (E2).
    m_pDoc->SetValue(4, 1, 0, 2.4);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 12.0));

    clearRange(m_pDoc, ScRange(0, 0, 0, 10, 10, 0));

    // Now, column-based dependency tracking.  We now switch to the R1C1
    // syntax which is easier to use for repeated relative references.

    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    val = 0.0;
    for (SCROW nRow = 1; nRow <= 9; ++nRow)
    {
        // Static value in column 1.
        m_pDoc->SetValue(0, nRow, 0, ++val);

        // Formula in column 2 that references cell to the left.
        m_pDoc->SetString(1, nRow, 0, "=RC[-1]");

        // Formula in column 3 that references cell to the left.
        m_pDoc->SetString(2, nRow, 0, "=RC[-1]*2");
    }

    // Check formula values.
    val = 0.0;
    for (SCROW nRow = 1; nRow <= 9; ++nRow)
    {
        ++val;
        CPPUNIT_ASSERT_MESSAGE("Unexpected formula value.", m_pDoc->GetValue(1, nRow, 0) == val);
        CPPUNIT_ASSERT_MESSAGE("Unexpected formula value.", m_pDoc->GetValue(2, nRow, 0) == val*2.0);
    }

    // Intentionally insert a formula in column 1. This will break column 1's
    // uniformity of consisting only of static value cells.
    m_pDoc->SetString(0, 4, 0, "=R2C3");
    CPPUNIT_ASSERT_MESSAGE("Unexpected formula value.", m_pDoc->GetValue(0, 4, 0) == 2.0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected formula value.", m_pDoc->GetValue(1, 4, 0) == 2.0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected formula value.", m_pDoc->GetValue(2, 4, 0) == 4.0);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncParam()
{
    rtl::OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    // First, the normal case, with no missing parameters.
    m_pDoc->SetString(0, 0, 0, OUString("=AVERAGE(1;2;3)"));
    m_pDoc->CalcFormulaTree(false, true);
    double val;
    m_pDoc->GetValue(0, 0, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 2);

    // Now function with missing parameters.  Missing values should be treated
    // as zeros.
    m_pDoc->SetString(0, 0, 0, OUString("=AVERAGE(1;;;)"));
    m_pDoc->CalcFormulaTree(false, true);
    m_pDoc->GetValue(0, 0, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 0.25);

    // Conversion of string to numeric argument.
    m_pDoc->SetString(0, 0, 0, OUString("=\"\"+3"));    // empty string
    m_pDoc->SetString(0, 1, 0, OUString("=\" \"+3"));   // only blank
    m_pDoc->SetString(0, 2, 0, OUString("=\" 4 \"+3")); // number in blanks
    m_pDoc->SetString(0, 3, 0, OUString("=\" x \"+3")); // non-numeric => #VALUE! error

    rtl::OUString aVal;
    ScCalcConfig aConfig;

    // With "Empty string as zero" option.
    aConfig.mbEmptyStringAsZero = true;
    ScInterpreter::SetGlobalConfig(aConfig);
    m_pDoc->CalcAll();
    m_pDoc->GetValue(0, 0, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 3);
    m_pDoc->GetValue(0, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 3);
    m_pDoc->GetValue(0, 2, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 7);
    aVal = m_pDoc->GetString( 0, 3, 0);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", aVal == "#VALUE!");

    // Without "Empty string as zero" option.
    aConfig.mbEmptyStringAsZero = false;
    ScInterpreter::SetGlobalConfig(aConfig);
    m_pDoc->CalcAll();
    aVal = m_pDoc->GetString( 0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", aVal == "#VALUE!");
    aVal = m_pDoc->GetString( 0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", aVal == "#VALUE!");
    m_pDoc->GetValue(0, 2, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 7);
    aVal = m_pDoc->GetString( 0, 3, 0);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", aVal == "#VALUE!");

    m_pDoc->DeleteTab(0);
}

void Test::testNamedRange()
{
    struct {
        const char* pName; const char* pExpr; sal_uInt16 nIndex;
    } aNames[] = {
        { "Divisor",  "$Sheet1.$A$1:$A$1048576", 1 },
        { "MyRange1", "$Sheet1.$A$1:$A$100",     2 },
        { "MyRange2", "$Sheet1.$B$1:$B$100",     3 },
        { "MyRange3", "$Sheet1.$C$1:$C$100",     4 }
    };

    rtl::OUString aTabName("Sheet1");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    m_pDoc->SetValue (0, 0, 0, 101);

    ScAddress aA1(0, 0, 0);
    ScRangeName* pNewRanges = new ScRangeName();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        ScRangeData* pNew = new ScRangeData(
            m_pDoc,
            rtl::OUString::createFromAscii(aNames[i].pName),
            rtl::OUString::createFromAscii(aNames[i].pExpr),
            aA1, 0, formula::FormulaGrammar::GRAM_ENGLISH);
        pNew->SetIndex(aNames[i].nIndex);
        bool bSuccess = pNewRanges->insert(pNew);
        CPPUNIT_ASSERT_MESSAGE ("insertion failed", bSuccess);
    }

    // Make sure the index lookup does the right thing.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        const ScRangeData* p = pNewRanges->findByIndex(aNames[i].nIndex);
        CPPUNIT_ASSERT_MESSAGE("lookup of range name by index failed.", p);
        rtl::OUString aName = p->GetName();
        CPPUNIT_ASSERT_MESSAGE("wrong range name is retrieved.", aName.equalsAscii(aNames[i].pName));
    }

    // Test usage in formula expression.
    m_pDoc->SetRangeName(pNewRanges);
    m_pDoc->SetString (1, 0, 0, rtl::OUString("=A1/Divisor"));
    m_pDoc->CalcAll();

    double result;
    m_pDoc->GetValue (1, 0, 0, result);
    CPPUNIT_ASSERT_MESSAGE ("calculation failed", result == 1.0);

    // Test copy-ability of range names.
    ScRangeName* pCopiedRanges = new ScRangeName(*pNewRanges);
    m_pDoc->SetRangeName(pCopiedRanges);
    // Make sure the index lookup still works.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        const ScRangeData* p = pCopiedRanges->findByIndex(aNames[i].nIndex);
        CPPUNIT_ASSERT_MESSAGE("lookup of range name by index failed with the copied instance.", p);
        rtl::OUString aName = p->GetName();
        CPPUNIT_ASSERT_MESSAGE("wrong range name is retrieved with the copied instance.", aName.equalsAscii(aNames[i].pName));
    }

    m_pDoc->SetRangeName(NULL); // Delete the names.
    m_pDoc->DeleteTab(0);
}

void Test::testCSV()
{
    const int English = 0, European = 1;
    struct {
        const char *pStr; int eSep; bool bResult; double nValue;
    } aTests[] = {
        { "foo",       English,  false, 0.0 },
        { "1.0",       English,  true,  1.0 },
        { "1,0",       English,  false, 0.0 },
        { "1.0",       European, false, 0.0 },
        { "1.000",     European, true,  1000.0 },
        { "1,000",     European, true,  1.0 },
        { "1.000",     English,  true,  1.0 },
        { "1,000",     English,  true,  1000.0 },
        { " 1.0",      English,  true,  1.0 },
        { " 1.0  ",    English,  true,  1.0 },
        { "1.0 ",      European, false, 0.0 },
        { "1.000",     European, true,  1000.0 },
        { "1137.999",  English,  true,  1137.999 },
        { "1.000.00",  European, false, 0.0 }
    };
    for (sal_uInt32 i = 0; i < SAL_N_ELEMENTS(aTests); i++) {
        rtl::OUString aStr(aTests[i].pStr, strlen (aTests[i].pStr), RTL_TEXTENCODING_UTF8);
        double nValue = 0.0;
        bool bResult = ScStringUtil::parseSimpleNumber
                (aStr, aTests[i].eSep == English ? '.' : ',',
                 aTests[i].eSep == English ? ',' : '.',
                 nValue);
        CPPUNIT_ASSERT_MESSAGE ("CSV numeric detection failure", bResult == aTests[i].bResult);
        CPPUNIT_ASSERT_MESSAGE ("CSV numeric value failure", nValue == aTests[i].nValue);
    }
}

template<typename Evaluator>
void checkMatrixElements(const ScMatrix& rMat)
{
    SCSIZE nC, nR;
    rMat.GetDimensions(nC, nR);
    Evaluator aEval;
    for (SCSIZE i = 0; i < nC; ++i)
    {
        for (SCSIZE j = 0; j < nR; ++j)
        {
            aEval(i, j, rMat.Get(i, j));
        }
    }
}

struct AllZeroMatrix
{
    void operator() (SCSIZE /*nCol*/, SCSIZE /*nRow*/, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_MESSAGE("element is not of numeric type", rVal.nType == SC_MATVAL_VALUE);
        CPPUNIT_ASSERT_MESSAGE("element value must be zero", rVal.fVal == 0.0);
    }
};

struct PartiallyFilledZeroMatrix
{
    void operator() (SCSIZE nCol, SCSIZE nRow, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_MESSAGE("element is not of numeric type", rVal.nType == SC_MATVAL_VALUE);
        if (1 <= nCol && nCol <= 2 && 2 <= nRow && nRow <= 8)
        {
            CPPUNIT_ASSERT_MESSAGE("element value must be 3.0", rVal.fVal == 3.0);
        }
        else
        {
            CPPUNIT_ASSERT_MESSAGE("element value must be zero", rVal.fVal == 0.0);
        }
    }
};

struct AllEmptyMatrix
{
    void operator() (SCSIZE /*nCol*/, SCSIZE /*nRow*/, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_MESSAGE("element is not of empty type", rVal.nType == SC_MATVAL_EMPTY);
        CPPUNIT_ASSERT_MESSAGE("value of \"empty\" element is expected to be zero", rVal.fVal == 0.0);
    }
};

struct PartiallyFilledEmptyMatrix
{
    void operator() (SCSIZE nCol, SCSIZE nRow, const ScMatrixValue& rVal) const
    {
        if (nCol == 1 && nRow == 1)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of boolean type", rVal.nType == SC_MATVAL_BOOLEAN);
            CPPUNIT_ASSERT_MESSAGE("element value is not what is expected", rVal.fVal == 1.0);
        }
        else if (nCol == 4 && nRow == 5)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of value type", rVal.nType == SC_MATVAL_VALUE);
            CPPUNIT_ASSERT_MESSAGE("element value is not what is expected", rVal.fVal == -12.5);
        }
        else if (nCol == 8 && nRow == 2)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of value type", rVal.nType == SC_MATVAL_STRING);
            CPPUNIT_ASSERT_MESSAGE("element value is not what is expected", rVal.aStr == "Test");
        }
        else if (nCol == 8 && nRow == 11)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of empty path type", rVal.nType == SC_MATVAL_EMPTYPATH);
            CPPUNIT_ASSERT_MESSAGE("value of \"empty\" element is expected to be zero", rVal.fVal == 0.0);
        }
        else
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of empty type", rVal.nType == SC_MATVAL_EMPTY);
            CPPUNIT_ASSERT_MESSAGE("value of \"empty\" element is expected to be zero", rVal.fVal == 0.0);
        }
    }
};

void Test::testMatrix()
{
    ScMatrixRef pMat;

    // First, test the zero matrix type.
    pMat = new ScMatrix(0, 0, 0.0);
    SCSIZE nC, nR;
    pMat->GetDimensions(nC, nR);
    CPPUNIT_ASSERT_MESSAGE("matrix is not empty", nC == 0 && nR == 0);
    pMat->Resize(4, 10, 0.0);
    pMat->GetDimensions(nC, nR);
    CPPUNIT_ASSERT_MESSAGE("matrix size is not as expected", nC == 4 && nR == 10);
    CPPUNIT_ASSERT_MESSAGE("both 'and' and 'or' should evaluate to false",
                           !pMat->And() && !pMat->Or());

    // Resizing into a larger matrix should fill the void space with zeros.
    checkMatrixElements<AllZeroMatrix>(*pMat);

    pMat->FillDouble(3.0, 1, 2, 2, 8);
    checkMatrixElements<PartiallyFilledZeroMatrix>(*pMat);
    CPPUNIT_ASSERT_MESSAGE("matrix is expected to be numeric", pMat->IsNumeric());
    CPPUNIT_ASSERT_MESSAGE("partially non-zero matrix should evaluate false on 'and' and true on 'or",
                           !pMat->And() && pMat->Or());
    pMat->FillDouble(5.0, 0, 0, nC-1, nR-1);
    CPPUNIT_ASSERT_MESSAGE("fully non-zero matrix should evaluate true both on 'and' and 'or",
                           pMat->And() && pMat->Or());

    // Test the AND and OR evaluations.
    pMat = new ScMatrix(2, 2, 0.0);

    // Only some of the elements are non-zero.
    pMat->PutBoolean(true, 0, 0);
    pMat->PutDouble(1.0, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("incorrect OR result", pMat->Or());
    CPPUNIT_ASSERT_MESSAGE("incorrect AND result", !pMat->And());

    // All of the elements are non-zero.
    pMat->PutBoolean(true, 0, 1);
    pMat->PutDouble(2.3, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("incorrect OR result", pMat->Or());
    CPPUNIT_ASSERT_MESSAGE("incorrect AND result", pMat->And());

    // Now test the emtpy matrix type.
    pMat = new ScMatrix(10, 20);
    pMat->GetDimensions(nC, nR);
    CPPUNIT_ASSERT_MESSAGE("matrix size is not as expected", nC == 10 && nR == 20);
    checkMatrixElements<AllEmptyMatrix>(*pMat);

    pMat->PutBoolean(true, 1, 1);
    pMat->PutDouble(-12.5, 4, 5);
    rtl::OUString aStr("Test");
    pMat->PutString(aStr, 8, 2);
    pMat->PutEmptyPath(8, 11);
    checkMatrixElements<PartiallyFilledEmptyMatrix>(*pMat);
}

namespace {

struct DPFieldDef
{
    const char* pName;
    sheet::DataPilotFieldOrientation eOrient;

    /**
     * Function for data field.  It's used only for data field.  When 0, the
     * default function (SUM) is used.
     */
    int eFunc;
};

template<size_t _Size>
ScRange insertDPSourceData(ScDocument* pDoc, DPFieldDef aFields[], size_t nFieldCount, const char* aData[][_Size], size_t nDataCount)
{
    // Insert field names in row 0.
    for (size_t i = 0; i < nFieldCount; ++i)
        pDoc->SetString(static_cast<SCCOL>(i), 0, 0, OUString(aFields[i].pName, strlen(aFields[i].pName), RTL_TEXTENCODING_UTF8));

    // Insert data into row 1 and downward.
    for (size_t i = 0; i < nDataCount; ++i)
    {
        SCROW nRow = static_cast<SCROW>(i) + 1;
        for (size_t j = 0; j < nFieldCount; ++j)
        {
            SCCOL nCol = static_cast<SCCOL>(j);
            pDoc->SetString(
                nCol, nRow, 0, OUString(aData[i][j], strlen(aData[i][j]), RTL_TEXTENCODING_UTF8));
        }
    }

    SCROW nRow1 = 0, nRow2 = 0;
    SCCOL nCol1 = 0, nCol2 = 0;
    pDoc->GetDataArea(0, nCol1, nRow1, nCol2, nRow2, true, false);
    CPPUNIT_ASSERT_MESSAGE("Data is expected to start from (col=0,row=0).", nCol1 == 0 && nRow1 == 0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected data range.",
                           nCol2 == static_cast<SCCOL>(nFieldCount - 1) && nRow2 == static_cast<SCROW>(nDataCount));

    ScRange aSrcRange(nCol1, nRow1, 0, nCol2, nRow2, 0);
    printRange(pDoc, aSrcRange, "Data sheet content");
    return aSrcRange;
}

template<size_t _Size>
bool checkDPTableOutput(ScDocument* pDoc, const ScRange& aOutRange, const char* aOutputCheck[][_Size], const char* pCaption)
{
    bool bResult = true;
    const ScAddress& s = aOutRange.aStart;
    const ScAddress& e = aOutRange.aEnd;
    SheetPrinter printer(e.Row() - s.Row() + 1, e.Col() - s.Col() + 1);
    SCROW nOutRowSize = e.Row() - s.Row() + 1;
    SCCOL nOutColSize = e.Col() - s.Col() + 1;
    for (SCROW nRow = 0; nRow < nOutRowSize; ++nRow)
    {
        for (SCCOL nCol = 0; nCol < nOutColSize; ++nCol)
        {
            OUString aVal;
            pDoc->GetString(nCol + s.Col(), nRow + s.Row(), s.Tab(), aVal);
            printer.set(nRow, nCol, aVal);
            const char* p = aOutputCheck[nRow][nCol];
            if (p)
            {
                OUString aCheckVal = OUString::createFromAscii(p);
                bool bEqual = aCheckVal.equals(aVal);
                if (!bEqual)
                {
                    cout << "Expected: " << aCheckVal << "  Actual: " << aVal << endl;
                    bResult = false;
                }
            }
            else if (!aVal.isEmpty())
            {
                cout << "Empty cell expected" << endl;
                bResult = false;
            }
        }
    }
    printer.print(pCaption);
    return bResult;
}

ScDPObject* createDPFromSourceDesc(
    ScDocument* pDoc, const ScSheetSourceDesc& rDesc, DPFieldDef aFields[], size_t nFieldCount,
    bool bFilterButton)
{
    ScDPObject* pDPObj = new ScDPObject(pDoc);
    pDPObj->SetSheetDesc(rDesc);
    pDPObj->SetOutRange(ScAddress(0, 0, 1));

    ScDPSaveData aSaveData;
    // Set data pilot table output options.
    aSaveData.SetIgnoreEmptyRows(false);
    aSaveData.SetRepeatIfEmpty(false);
    aSaveData.SetColumnGrand(true);
    aSaveData.SetRowGrand(true);
    aSaveData.SetFilterButton(bFilterButton);
    aSaveData.SetDrillDown(true);

    // Check the sanity of the source range.
    const ScRange& rSrcRange = rDesc.GetSourceRange();
    SCROW nRow1 = rSrcRange.aStart.Row();
    SCROW nRow2 = rSrcRange.aEnd.Row();
    CPPUNIT_ASSERT_MESSAGE("source range contains no data!", nRow2 - nRow1 > 1);

    // Set the dimension information.
    for (size_t i = 0; i < nFieldCount; ++i)
    {
        OUString aDimName = rtl::OUString::createFromAscii(aFields[i].pName);
        ScDPSaveDimension* pDim = aSaveData.GetNewDimensionByName(aDimName);
        pDim->SetOrientation(static_cast<sal_uInt16>(aFields[i].eOrient));
        pDim->SetUsedHierarchy(0);

        if (aFields[i].eOrient == sheet::DataPilotFieldOrientation_DATA)
        {
            sheet::GeneralFunction eFunc = sheet::GeneralFunction_SUM;
            if (aFields[i].eFunc)
                eFunc = static_cast<sheet::GeneralFunction>(aFields[i].eFunc);

            pDim->SetFunction(eFunc);
            pDim->SetReferenceValue(NULL);
        }
        else
        {
            sheet::DataPilotFieldSortInfo aSortInfo;
            aSortInfo.IsAscending = true;
            aSortInfo.Mode = 2;
            pDim->SetSortInfo(&aSortInfo);

            sheet::DataPilotFieldLayoutInfo aLayInfo;
            aLayInfo.LayoutMode = 0;
            aLayInfo.AddEmptyLines = false;
            pDim->SetLayoutInfo(&aLayInfo);
            sheet::DataPilotFieldAutoShowInfo aShowInfo;
            aShowInfo.IsEnabled = false;
            aShowInfo.ShowItemsMode = 0;
            aShowInfo.ItemCount = 0;
            pDim->SetAutoShowInfo(&aShowInfo);
        }
    }

    // Don't forget the data layout dimension.
    ScDPSaveDimension* pDim = aSaveData.GetDataLayoutDimension();
    pDim->SetOrientation(sheet::DataPilotFieldOrientation_ROW);
    pDim->SetShowEmpty(true);

    pDPObj->SetSaveData(aSaveData);
    pDPObj->SetAlive(true);
    pDPObj->InvalidateData();

    return pDPObj;
}

ScDPObject* createDPFromRange(
    ScDocument* pDoc, const ScRange& rRange, DPFieldDef aFields[], size_t nFieldCount,
    bool bFilterButton)
{
    ScSheetSourceDesc aSheetDesc(pDoc);
    aSheetDesc.SetSourceRange(rRange);
    return createDPFromSourceDesc(pDoc, aSheetDesc, aFields, nFieldCount, bFilterButton);
}

ScRange refresh(ScDPObject* pDPObj)
{
    bool bOverFlow = false;
    ScRange aOutRange = pDPObj->GetNewOutputRange(bOverFlow);
    CPPUNIT_ASSERT_MESSAGE("Table overflow!?", !bOverFlow);

    pDPObj->Output(aOutRange.aStart);
    aOutRange = pDPObj->GetOutRange();
    return aOutRange;
}

ScRange refreshGroups(ScDPCollection* pDPs, ScDPObject* pDPObj)
{
    // We need to first create group data in the cache, then the group data in
    // the object.
    std::set<ScDPObject*> aRefs;
    bool bSuccess = pDPs->ReloadGroupsInCache(pDPObj, aRefs);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload group data in cache.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("There should be only one table linked to this cache.", aRefs.size() == 1);
    pDPObj->ReloadGroupTableData();

    return refresh(pDPObj);
}

}

void Test::testPivotTable()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Group", sheet::DataPilotFieldOrientation_COLUMN, 0 },
        { "Score", sheet::DataPilotFieldOrientation_DATA, 0 }
    };

    // Raw data
    const char* aData[][3] = {
        { "Andy",    "A", "30" },
        { "Bruce",   "A", "20" },
        { "Charlie", "B", "45" },
        { "David",   "B", "12" },
        { "Edward",  "C",  "8" },
        { "Frank",   "C", "15" },
    };

    size_t nFieldCount = SAL_N_ELEMENTS(aFields);
    size_t nDataCount = SAL_N_ELEMENTS(aData);

    ScRange aSrcRange = insertDPSourceData(m_pDoc, aFields, nFieldCount, aData, nDataCount);
    SCROW nRow1 = aSrcRange.aStart.Row(), nRow2 = aSrcRange.aEnd.Row();
    SCCOL nCol1 = aSrcRange.aStart.Col(), nCol2 = aSrcRange.aEnd.Col();

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), aFields, nFieldCount, false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new datapilot object into document", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    bool bOverFlow = false;
    ScRange aOutRange = pDPObj->GetNewOutputRange(bOverFlow);
    CPPUNIT_ASSERT_MESSAGE("Table overflow!?", !bOverFlow);

    pDPObj->Output(aOutRange.aStart);
    aOutRange = pDPObj->GetOutRange();
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - Score", "Group", 0, 0, 0 },
            { "Name", "A", "B", "C", "Total Result" },
            { "Andy", "30", 0, 0, "30" },
            { "Bruce", "20", 0, 0, "20" },
            { "Charlie", 0, "45", 0, "45" },
            { "David", 0, "12", 0, "12" },
            { "Edward", 0, 0, "8", "8" },
            { "Frank", 0, 0, "15", "15" },
            { "Total Result", "50", "57", "23", "130" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }
    CPPUNIT_ASSERT_MESSAGE("There should be only one data cache.", pDPs->GetSheetCaches().size() == 1);

    // Update the cell values.
    double aData2[] = { 100, 200, 300, 400, 500, 600 };
    for (size_t i = 0; i < SAL_N_ELEMENTS(aData2); ++i)
    {
        SCROW nRow = i + 1;
        m_pDoc->SetValue(2, nRow, 0, aData2[i]);
    }

    printRange(m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), "Data sheet content (modified)");

    // Now, create a copy of the datapilot object for the updated table, but
    // don't reload the cache which should force the copy to use the old data
    // from the cache.
    ScDPObject* pDPObj2 = new ScDPObject(*pDPObj);
    pDPs->InsertNewTable(pDPObj2);

    aOutRange = pDPObj2->GetOutRange();
    pDPObj2->ClearTableData();
    pDPObj2->Output(aOutRange.aStart);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - Score", "Group", 0, 0, 0 },
            { "Name", "A", "B", "C", "Total Result" },
            { "Andy", "30", 0, 0, "30" },
            { "Bruce", "20", 0, 0, "20" },
            { "Charlie", 0, "45", 0, "45" },
            { "David", 0, "12", 0, "12" },
            { "Edward", 0, 0, "8", "8" },
            { "Frank", 0, 0, "15", "15" },
            { "Total Result", "50", "57", "23", "130" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (from old cache)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    CPPUNIT_ASSERT_MESSAGE("There should be only one data cache.", pDPs->GetSheetCaches().size() == 1);

    // Free the first datapilot object after the 2nd one gets reloaded, to
    // prevent the data cache from being deleted before the reload.
    pDPs->FreeTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("There should be only one data cache.", pDPs->GetSheetCaches().size() == 1);

    // This time clear the cache to refresh the data from the source range.
    CPPUNIT_ASSERT_MESSAGE("This datapilot should be based on sheet data.", pDPObj2->IsSheetData());
    std::set<ScDPObject*> aRefs;
    sal_uLong nErrId = pDPs->ReloadCache(pDPObj2, aRefs);
    CPPUNIT_ASSERT_MESSAGE("Cache reload failed.", nErrId == 0);
    CPPUNIT_ASSERT_MESSAGE("Reloading a cache shouldn't remove any cache.",
                           pDPs->GetSheetCaches().size() == 1);

    pDPObj2->ClearTableData();
    pDPObj2->Output(aOutRange.aStart);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - Score", "Group", 0, 0, 0 },
            { "Name", "A", "B", "C", "Total Result" },
            { "Andy", "100", 0, 0, "100" },
            { "Bruce", "200", 0, 0, "200" },
            { "Charlie", 0, "300", 0, "300" },
            { "David", 0, "400", 0, "400" },
            { "Edward", 0, 0, "500", "500" },
            { "Frank", 0, 0, "600", "600" },
            { "Total Result", "300", "700", "1100", "2100" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (refreshed)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    CPPUNIT_ASSERT_MESSAGE("Cache should be here.", pDPs->GetSheetCaches().hasCache(aSrcRange));

    // Swap the two sheets.
    m_pDoc->MoveTab(1, 0);
    CPPUNIT_ASSERT_MESSAGE("Swapping the sheets shouldn't remove the cache.",
                           pDPs->GetSheetCaches().size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Cache should have moved.", !pDPs->GetSheetCaches().hasCache(aSrcRange));
    aSrcRange.aStart.SetTab(1);
    aSrcRange.aEnd.SetTab(1);
    CPPUNIT_ASSERT_MESSAGE("Cache should be here.", pDPs->GetSheetCaches().hasCache(aSrcRange));

    pDPs->FreeTable(pDPObj2);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be any data pilot table stored with the document.",
                           pDPs->GetCount() == 0);

    CPPUNIT_ASSERT_MESSAGE("There shouldn't be any more data cache.",
                           pDPs->GetSheetCaches().size() == 0);

    // Insert a brand new pivot table object once again, but this time, don't
    // create the output to avoid creating a data cache.
    m_pDoc->DeleteTab(1);
    m_pDoc->InsertTab(1, OUString("Table"));

    pDPObj = createDPFromRange(
        m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), aFields, nFieldCount, false);
    bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new datapilot object into document", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());
    CPPUNIT_ASSERT_MESSAGE("Data cache shouldn't exist yet before creating the table output.",
                           pDPs->GetSheetCaches().size() == 0);

    // Now, "refresh" the table.  This should still return a reference to self
    // even with the absence of data cache.
    aRefs.clear();
    pDPs->ReloadCache(pDPObj, aRefs);
    CPPUNIT_ASSERT_MESSAGE("It should return the same object as a reference.",
                           aRefs.size() == 1 && *aRefs.begin() == pDPObj);

    pDPs->FreeTable(pDPObj);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableLabels()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Software", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Version",  sheet::DataPilotFieldOrientation_COLUMN, 0 },
        { "1.2.3",    sheet::DataPilotFieldOrientation_DATA, 0 }
    };

    // Raw data
    const char* aData[][3] = {
        { "LibreOffice", "3.3.0", "30" },
        { "LibreOffice", "3.3.1", "20" },
        { "LibreOffice", "3.4.0", "45" },
    };

    size_t nFieldCount = SAL_N_ELEMENTS(aFields);
    size_t nDataCount = SAL_N_ELEMENTS(aData);

    ScRange aSrcRange = insertDPSourceData(m_pDoc, aFields, nFieldCount, aData, nDataCount);
    SCROW nRow1 = aSrcRange.aStart.Row(), nRow2 = aSrcRange.aEnd.Row();
    SCCOL nCol1 = aSrcRange.aStart.Col(), nCol2 = aSrcRange.aEnd.Col();

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), aFields, nFieldCount, false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new datapilot object into document", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - 1.2.3", "Version", 0, 0, 0 },
            { "Software", "3.3.0", "3.3.1", "3.4.0", "Total Result" },
            { "LibreOffice", "30", "20", "45", "95" },
            { "Total Result", "30", "20", "45", "95" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableDateLabels()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Date",  sheet::DataPilotFieldOrientation_COLUMN, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, 0 }
    };

    // Raw data
    const char* aData[][3] = {
        { "Zena",   "2011-1-1", "30" },
        { "Yodel",  "2011-1-2", "20" },
        { "Xavior", "2011-1-3", "45" }
    };

    size_t nFieldCount = SAL_N_ELEMENTS(aFields);
    size_t nDataCount = SAL_N_ELEMENTS(aData);

    ScRange aSrcRange = insertDPSourceData(m_pDoc, aFields, nFieldCount, aData, nDataCount);
    SCROW nRow1 = aSrcRange.aStart.Row(), nRow2 = aSrcRange.aEnd.Row();
    SCCOL nCol1 = aSrcRange.aStart.Col(), nCol2 = aSrcRange.aEnd.Col();

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), aFields, nFieldCount, false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new datapilot object into document", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - Value", "Date", 0, 0, 0 },
            { "Name", "2011-01-01", "2011-01-02", "2011-01-03", "Total Result" },
            { "Xavior",  0, 0, "45", "45" },
            { "Yodel",  0, "20", 0, "20" },
            { "Zena",  "30", 0, 0, "30" },
            { "Total Result", "30", "20", "45", "95" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    {
        const char* aChecks[] = {
            "2011-01-01", "2011-01-02", "2011-01-03"
        };

        // Make sure those cells that contain dates are numeric.
        SCROW nRow = aOutRange.aStart.Row() + 1;
        nCol1 = aOutRange.aStart.Col() + 1;
        nCol2 = nCol1 + 2;
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        {
            OUString aVal = m_pDoc->GetString(nCol, nRow, 1);
            CPPUNIT_ASSERT_MESSAGE("Cell value is not as expected.", aVal.equalsAscii(aChecks[nCol-nCol1]));
            CPPUNIT_ASSERT_MESSAGE("This cell contains a date value and is supposed to be numeric.",
                                   m_pDoc->HasValueData(nCol, nRow, 1));
        }
    }

    pDPs->FreeTable(pDPObj);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableFilters()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",   sheet::DataPilotFieldOrientation_HIDDEN, 0 },
        { "Group1", sheet::DataPilotFieldOrientation_HIDDEN, 0 },
        { "Group2", sheet::DataPilotFieldOrientation_PAGE, 0 },
        { "Val1",   sheet::DataPilotFieldOrientation_DATA, 0 },
        { "Val2",   sheet::DataPilotFieldOrientation_DATA, 0 }
    };

    // Raw data
    const char* aData[][5] = {
        { "A", "1", "A", "1", "10" },
        { "B", "1", "A", "1", "10" },
        { "C", "1", "B", "1", "10" },
        { "D", "1", "B", "1", "10" },
        { "E", "2", "A", "1", "10" },
        { "F", "2", "A", "1", "10" },
        { "G", "2", "B", "1", "10" },
        { "H", "2", "B", "1", "10" }
    };

    size_t nFieldCount = SAL_N_ELEMENTS(aFields);
    size_t nDataCount = SAL_N_ELEMENTS(aData);

    ScRange aSrcRange = insertDPSourceData(m_pDoc, aFields, nFieldCount, aData, nDataCount);
    SCROW nRow1 = aSrcRange.aStart.Row(), nRow2 = aSrcRange.aEnd.Row();
    SCCOL nCol1 = aSrcRange.aStart.Col(), nCol2 = aSrcRange.aEnd.Col();

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), aFields, nFieldCount, true);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new datapilot object into document", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Filter", 0 },
            { "Group2", "- all -" },
            { 0, 0 },
            { "Data", 0 },
            { "Sum - Val1", "8" },
            { "Sum - Val2", "80" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (unfiltered)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    AutoCalcSwitch aACSwitch(m_pDoc, true); // turn on auto calculation.

    ScAddress aFormulaAddr = aOutRange.aEnd;
    aFormulaAddr.IncRow(2);
    m_pDoc->SetString(aFormulaAddr.Col(), aFormulaAddr.Row(), aFormulaAddr.Tab(),
                      rtl::OUString("=B6"));
    double fTest = m_pDoc->GetValue(aFormulaAddr);
    CPPUNIT_ASSERT_MESSAGE("Incorrect formula value that references a cell in the pivot table output.", fTest == 80.0);

    // Set current page of 'Group2' to 'A'.
    ScDPSaveData aSaveData(*pDPObj->GetSaveData());
    ScDPSaveDimension* pPageDim = aSaveData.GetDimensionByName(
        OUString("Group2"));
    CPPUNIT_ASSERT_MESSAGE("Dimension not found", pPageDim);
    OUString aPage("A");
    pPageDim->SetCurrentPage(&aPage);
    pDPObj->SetSaveData(aSaveData);
    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Filter", 0 },
            { "Group2", "A" },
            { 0, 0 },
            { "Data", 0 },
            { "Sum - Val1", "4" },
            { "Sum - Val2", "40" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (filtered by page)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    fTest = m_pDoc->GetValue(aFormulaAddr);
    CPPUNIT_ASSERT_MESSAGE("Incorrect formula value that references a cell in the pivot table output.", fTest == 40.0);

    // Set query filter.
    ScSheetSourceDesc aDesc(*pDPObj->GetSheetDesc());
    ScQueryParam aQueryParam(aDesc.GetQueryParam());
    CPPUNIT_ASSERT_MESSAGE("There should be at least one query entry.", aQueryParam.GetEntryCount() > 0);
    ScQueryEntry& rEntry = aQueryParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 1;  // Group1
    rEntry.GetQueryItem().mfVal = 1;
    aDesc.SetQueryParam(aQueryParam);
    pDPObj->SetSheetDesc(aDesc);
    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Filter", 0 },
            { "Group2", "A" },
            { 0, 0 },
            { "Data", 0 },
            { "Sum - Val1", "2" },
            { "Sum - Val2", "20" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (filtered by query)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    fTest = m_pDoc->GetValue(aFormulaAddr);
    CPPUNIT_ASSERT_MESSAGE("Incorrect formula value that references a cell in the pivot table output.", fTest == 20.0);

    // Set the current page of 'Group2' back to '- all -'. The query filter
    // should still be in effect.
    pPageDim->SetCurrentPage(NULL); // Remove the page.
    pDPObj->SetSaveData(aSaveData);
    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Filter", 0 },
            { "Group2", "- all -" },
            { 0, 0 },
            { "Data", 0 },
            { "Sum - Val1", "4" },
            { "Sum - Val2", "40" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (filtered by page)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }


    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be any data pilot table stored with the document.",
                           pDPs->GetCount() == 0);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableNamedSource()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Group", sheet::DataPilotFieldOrientation_COLUMN, 0 },
        { "Score", sheet::DataPilotFieldOrientation_DATA, 0 }
    };

    // Raw data
    const char* aData[][3] = {
        { "Andy",    "A", "30" },
        { "Bruce",   "A", "20" },
        { "Charlie", "B", "45" },
        { "David",   "B", "12" },
        { "Edward",  "C",  "8" },
        { "Frank",   "C", "15" },
    };

    size_t nFieldCount = SAL_N_ELEMENTS(aFields);
    size_t nDataCount = SAL_N_ELEMENTS(aData);

    // Insert the raw data.
    ScRange aSrcRange = insertDPSourceData(m_pDoc, aFields, nFieldCount, aData, nDataCount);
    rtl::OUString aRangeStr;
    aSrcRange.Format(aRangeStr, SCR_ABS_3D, m_pDoc);

    // Name this range.
    rtl::OUString aRangeName("MyData");
    ScRangeName* pNames = m_pDoc->GetRangeName();
    CPPUNIT_ASSERT_MESSAGE("Failed to get global range name container.", pNames);
    ScRangeData* pName = new ScRangeData(
        m_pDoc, aRangeName, aRangeStr);
    bool bSuccess = pNames->insert(pName);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert a new name.", bSuccess);

    ScSheetSourceDesc aSheetDesc(m_pDoc);
    aSheetDesc.SetRangeName(aRangeName);
    ScDPObject* pDPObj = createDPFromSourceDesc(m_pDoc, aSheetDesc, aFields, nFieldCount, false);
    CPPUNIT_ASSERT_MESSAGE("Failed to create a new pivot table object.", pDPObj);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - Score", "Group", 0, 0, 0 },
            { "Name", "A", "B", "C", "Total Result" },
            { "Andy", "30", 0, 0, "30" },
            { "Bruce", "20", 0, 0, "20" },
            { "Charlie", 0, "45", 0, "45" },
            { "David", 0, "12", 0, "12" },
            { "Edward", 0, 0, "8", "8" },
            { "Frank", 0, 0, "15", "15" },
            { "Total Result", "50", "57", "23", "130" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    CPPUNIT_ASSERT_MESSAGE("There should be one named range data cache.",
                           pDPs->GetNameCaches().size() == 1 && pDPs->GetSheetCaches().size() == 0);

    // Move the table with pivot table to the left of the source data sheet.
    m_pDoc->MoveTab(1, 0);
    rtl::OUString aTabName;
    m_pDoc->GetName(0, aTabName);
    CPPUNIT_ASSERT_MESSAGE( "Wrong sheet name.", aTabName == "Table" );
    CPPUNIT_ASSERT_MESSAGE("Pivot table output is on the wrong sheet!",
                           pDPObj->GetOutRange().aStart.Tab() == 0);

    CPPUNIT_ASSERT_MESSAGE("Moving the pivot table to another sheet shouldn't have changed the cache state.",
                           pDPs->GetNameCaches().size() == 1 && pDPs->GetSheetCaches().size() == 0);

    const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
    CPPUNIT_ASSERT_MESSAGE("Sheet source description doesn't exist.", pDesc);
    CPPUNIT_ASSERT_MESSAGE("Named source range has been altered unexpectedly!",
                           pDesc->GetRangeName().equals(aRangeName));

    CPPUNIT_ASSERT_MESSAGE("Cache should exist.", pDPs->GetNameCaches().hasCache(aRangeName));

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("There should be no more tables.", pDPs->GetCount() == 0);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetNameCaches().size() == 0);

    pNames->clear();
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableCache()
{
    m_pDoc->InsertTab(0, OUString("Data"));

    // Raw data
    const char* aData[][3] = {
        { "F1", "F2", "F3" },
        { "Z",  "A", "30" },
        { "R",  "A", "20" },
        { "A",  "B", "45" },
        { "F",  "B", "12" },
        { "Y",  "C",  "8" },
        { "12", "C", "15" },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPCache aCache(m_pDoc);
    aCache.InitFromDoc(m_pDoc, aDataRange);
    long nDimCount = aCache.GetColumnCount();
    CPPUNIT_ASSERT_MESSAGE("wrong dimension count.", nDimCount == 3);
    rtl::OUString aDimName = aCache.GetDimensionName(0);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension name", aDimName.equalsAscii("F1"));
    aDimName = aCache.GetDimensionName(1);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension name", aDimName.equalsAscii("F2"));
    aDimName = aCache.GetDimensionName(2);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension name", aDimName.equalsAscii("F3"));

    // In each dimension, member ID values also represent their sort order (in
    // source dimensions only, not in group dimensions). Value items are
    // sorted before string ones. Also, no duplicate dimension members should
    // exist.

    // Dimension 0 - a mix of strings and values.
    long nMemCount = aCache.GetDimMemberCount(0);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension member count", nMemCount == 6);
    const ScDPItemData* pItem = aCache.GetItemDataById(0, 0);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 12);
    pItem = aCache.GetItemDataById(0, 1);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("A"));
    pItem = aCache.GetItemDataById(0, 2);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("F"));
    pItem = aCache.GetItemDataById(0, 3);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("R"));
    pItem = aCache.GetItemDataById(0, 4);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("Y"));
    pItem = aCache.GetItemDataById(0, 5);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("Z"));
    pItem = aCache.GetItemDataById(0, 6);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", !pItem);

    // Dimension 1 - duplicate values in source.
    nMemCount = aCache.GetDimMemberCount(1);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension member count", nMemCount == 3);
    pItem = aCache.GetItemDataById(1, 0);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("A"));
    pItem = aCache.GetItemDataById(1, 1);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("B"));
    pItem = aCache.GetItemDataById(1, 2);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("C"));
    pItem = aCache.GetItemDataById(1, 3);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", !pItem);

    // Dimension 2 - values only.
    nMemCount = aCache.GetDimMemberCount(2);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension member count", nMemCount == 6);
    pItem = aCache.GetItemDataById(2, 0);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 8);
    pItem = aCache.GetItemDataById(2, 1);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 12);
    pItem = aCache.GetItemDataById(2, 2);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 15);
    pItem = aCache.GetItemDataById(2, 3);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 20);
    pItem = aCache.GetItemDataById(2, 4);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 30);
    pItem = aCache.GetItemDataById(2, 5);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 45);
    pItem = aCache.GetItemDataById(2, 6);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", !pItem);

    {
        // Check the integrity of the source data.
        ScDPItemData aTest;
        long nDim;

        {
            // Dimension 0: Z, R, A, F, Y, 12
            nDim = 0;
            const char* aChecks[] = { "Z", "R", "A", "F", "Y" };
            for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
            {
                pItem = aCache.GetItemDataById(nDim, aCache.GetItemDataId(nDim, i, false));
                aTest.SetString(rtl::OUString::createFromAscii(aChecks[i]));
                CPPUNIT_ASSERT_MESSAGE("wrong data value", pItem && *pItem == aTest);
            }

            pItem = aCache.GetItemDataById(nDim, aCache.GetItemDataId(nDim, 5, false));
            aTest.SetValue(12);
            CPPUNIT_ASSERT_MESSAGE("wrong data value", pItem && *pItem == aTest);
        }

        {
            // Dimension 1: A, A, B, B, C, C
            nDim = 1;
            const char* aChecks[] = { "A", "A", "B", "B", "C", "C" };
            for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
            {
                pItem = aCache.GetItemDataById(nDim, aCache.GetItemDataId(nDim, i, false));
                aTest.SetString(rtl::OUString::createFromAscii(aChecks[i]));
                CPPUNIT_ASSERT_MESSAGE("wrong data value", pItem && *pItem == aTest);
            }
        }

        {
            // Dimension 2: 30, 20, 45, 12, 8, 15
            nDim = 2;
            double aChecks[] = { 30, 20, 45, 12, 8, 15 };
            for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
            {
                pItem = aCache.GetItemDataById(nDim, aCache.GetItemDataId(nDim, i, false));
                aTest.SetValue(aChecks[i]);
                CPPUNIT_ASSERT_MESSAGE("wrong data value", pItem && *pItem == aTest);
            }
        }
    }

    // Now, on to testing the filtered cache.

    {
        // Non-filtered cache - everything should be visible.
        ScDPFilteredCache aFilteredCache(aCache);
        aFilteredCache.fillTable();

        sal_Int32 nRows = aFilteredCache.getRowSize();
        CPPUNIT_ASSERT_MESSAGE("Wrong dimension.", nRows == 6 && aFilteredCache.getColSize() == 3);

        for (sal_Int32 i = 0; i < nRows; ++i)
        {
            if (!aFilteredCache.isRowActive(i))
            {
                std::ostringstream os;
                os << "Row " << i << " should be visible but it isn't.";
                CPPUNIT_ASSERT_MESSAGE(os.str().c_str(), false);
            }
        }
    }

    // TODO : Add test for filtered caches.

    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableDuplicateDataFields()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "A",       "45" },
        { "A",        "5" },
        { "A",       "41" },
        { "A",       "49" },
        { "A",        "4" },
        { "B",       "33" },
        { "B",       "84" },
        { "B",       "74" },
        { "B",        "8" },
        { "B",       "68" }
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_COUNT }
    };

    ScAddress aPos(2,2,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount(), static_cast<size_t>(1));
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { "Name", "Data", 0 },
            { "A", "Sum - Value", "144" },
            { 0, "Count - Value", "5" },
            { "B", "Sum - Value", "267" },
            { 0, "Count - Value", "5" },
            { "Total Sum - Value", 0, "411" },
            { "Total Count - Value", 0, "10" },
        };

        bSuccess = checkDPTableOutput<3>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Move the data layout dimension from row to column.
    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("No save data!?", pSaveData);
    ScDPSaveDimension* pDataLayout = pSaveData->GetDataLayoutDimension();
    CPPUNIT_ASSERT_MESSAGE("No data layout dimension.", pDataLayout);
    pDataLayout->SetOrientation(sheet::DataPilotFieldOrientation_COLUMN);
    pDPObj->SetSaveData(*pSaveData);

    // Refresh the table output.
    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { 0, "Data", 0 },
            { "Name", "Sum - Value", "Count - Value" },
            { "A", "144", "5" },
            { "B", "267", "5" },
            { "Total Result", "411", "10" }
        };

        bSuccess = checkDPTableOutput<3>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    ScPivotParam aParam;
    pDPObj->FillLabelData(aParam);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be exactly 4 labels (2 original, 1 data layout, and 1 duplicate dimensions).",
                           aParam.maLabelArray.size(), static_cast<size_t>(4));

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableNormalGrouping()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "A", "1" },
        { "B", "2" },
        { "C", "3" },
        { "D", "4" },
        { "E", "5" },
        { "F", "6" },
        { "G", "7" }
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount(), static_cast<size_t>(1));
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "1" },
            { "B", "2" },
            { "C", "3" },
            { "D", "4" },
            { "E", "5" },
            { "F", "6" },
            { "G", "7" },
            { "Total Result", "28" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Initial output without grouping");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("No save data !?", pSaveData);
    ScDPDimensionSaveData* pDimData = pSaveData->GetDimensionData();
    CPPUNIT_ASSERT_MESSAGE("Failed to create dimension data.", pDimData);

    rtl::OUString aGroupPrefix("Group");
    rtl::OUString aBaseDimName("Name");
    rtl::OUString aGroupDimName =
        pDimData->CreateGroupDimName(aBaseDimName, *pDPObj, false, NULL);

    {
        // Group A, B and C together.
        ScDPSaveGroupDimension aGroupDim(aBaseDimName, aGroupDimName);
        rtl::OUString aGroupName = aGroupDim.CreateGroupName(aGroupPrefix);
        CPPUNIT_ASSERT_MESSAGE("Unexpected group name", aGroupName.equalsAscii("Group1"));

        ScDPSaveGroupItem aGroup(aGroupName);
        aGroup.AddElement(rtl::OUString("A"));
        aGroup.AddElement(rtl::OUString("B"));
        aGroup.AddElement(rtl::OUString("C"));
        aGroupDim.AddGroupItem(aGroup);
        pDimData->AddGroupDimension(aGroupDim);

        ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(aGroupDimName);
        pDim->SetOrientation(sheet::DataPilotFieldOrientation_ROW);
        pSaveData->SetPosition(pDim, 0); // Set it before the base dimension.
    }

    pDPObj->SetSaveData(*pSaveData);
    aOutRange = refreshGroups(pDPs, pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { "Name2", "Name", 0 },
            { "D", "D", "4" },
            { "E", "E", "5" },
            { "F", "F", "6" },
            { "G", "G", "7" },
            { "Group1", "A", "1" },
            { 0,        "B", "2" },
            { 0,        "C", "3" },
            { "Total Result", 0, "28" }
        };

        bSuccess = checkDPTableOutput<3>(m_pDoc, aOutRange, aOutputCheck, "A, B, C grouped by Group1.");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pSaveData = pDPObj->GetSaveData();
    pDimData = pSaveData->GetDimensionData();

    {
        // Group D, E, F together.
        ScDPSaveGroupDimension* pGroupDim = pDimData->GetGroupDimAccForBase(aBaseDimName);
        CPPUNIT_ASSERT_MESSAGE("There should be an existing group dimension.", pGroupDim);
        rtl::OUString aGroupName = pGroupDim->CreateGroupName(aGroupPrefix);
        CPPUNIT_ASSERT_MESSAGE("Unexpected group name", aGroupName.equalsAscii("Group2"));

        ScDPSaveGroupItem aGroup(aGroupName);
        aGroup.AddElement(rtl::OUString("D"));
        aGroup.AddElement(rtl::OUString("E"));
        aGroup.AddElement(rtl::OUString("F"));
        pGroupDim->AddGroupItem(aGroup);
    }

    pDPObj->SetSaveData(*pSaveData);
    aOutRange = refreshGroups(pDPs, pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { "Name2", "Name", 0 },
            { "G", "G", "7" },
            { "Group1", "A", "1" },
            { 0,        "B", "2" },
            { 0,        "C", "3" },
            { "Group2", "D", "4" },
            { 0,        "E", "5" },
            { 0,        "F", "6" },
            { "Total Result", 0, "28" }
        };

        bSuccess = checkDPTableOutput<3>(m_pDoc, aOutRange, aOutputCheck, "D, E, F grouped by Group2.");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableNumberGrouping()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Order", "Score" },
        { "43", "171" },
        { "18", "20"  },
        { "69", "159" },
        { "95", "19"  },
        { "96", "163" },
        { "46", "70"  },
        { "22", "36"  },
        { "81", "49"  },
        { "54", "61"  },
        { "39", "62"  },
        { "86", "17"  },
        { "34", "0"   },
        { "30", "25"  },
        { "24", "103" },
        { "16", "59"  },
        { "24", "119" },
        { "15", "86"  },
        { "69", "170" }
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Order", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Score", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount(), static_cast<size_t>(1));
    pDPObj->SetName(pDPs->CreateNewName());

    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("No save data !?", pSaveData);
    ScDPDimensionSaveData* pDimData = pSaveData->GetDimensionData();
    CPPUNIT_ASSERT_MESSAGE("No dimension data !?", pDimData);

    {
        ScDPNumGroupInfo aInfo;
        aInfo.mbEnable = true;
        aInfo.mbAutoStart = false;
        aInfo.mbAutoEnd = false;
        aInfo.mbDateValues = false;
        aInfo.mbIntegerOnly = true;
        aInfo.mfStart = 30;
        aInfo.mfEnd = 60;
        aInfo.mfStep = 10;
        ScDPSaveNumGroupDimension aGroup(rtl::OUString("Order"), aInfo);
        pDimData->AddNumGroupDimension(aGroup);
    }

    pDPObj->SetSaveData(*pSaveData);
    ScRange aOutRange = refreshGroups(pDPs, pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Order", 0 },
            { "<30",   "423" },
            { "30-39", "87"  },
            { "40-49", "241" },
            { "50-60", "61"  },
            { ">60",   "577" },
            { "Total Result", "1389" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Order grouped by numbers");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableDateGrouping()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Date", "Value" },
        { "2011-01-01", "1" },
        { "2011-03-02", "2" },
        { "2012-01-04", "3" },
        { "2012-02-23", "4" },
        { "2012-02-24", "5" },
        { "2012-03-15", "6" },
        { "2011-09-03", "7" },
        { "2012-12-25", "8" }
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Date", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("No save data !?", pSaveData);
    ScDPDimensionSaveData* pDimData = pSaveData->GetDimensionData();
    CPPUNIT_ASSERT_MESSAGE("No dimension data !?", pDimData);

    rtl::OUString aBaseDimName("Date");

    ScDPNumGroupInfo aInfo;
    aInfo.mbEnable = true;
    aInfo.mbAutoStart = true;
    aInfo.mbAutoEnd = true;
    {
        // Turn the Date dimension into months.  The first of the date
        // dimensions is always a number-group dimension which replaces the
        // original dimension.
        ScDPSaveNumGroupDimension aGroup(aBaseDimName, aInfo, sheet::DataPilotFieldGroupBy::MONTHS);
        pDimData->AddNumGroupDimension(aGroup);
    }

    {
        // Add quarter dimension.  This will be an additional dimension.
        rtl::OUString aGroupDimName =
            pDimData->CreateDateGroupDimName(
                sheet::DataPilotFieldGroupBy::QUARTERS, *pDPObj, true, NULL);
        ScDPSaveGroupDimension aGroupDim(aBaseDimName, aGroupDimName);
        aGroupDim.SetDateInfo(aInfo, sheet::DataPilotFieldGroupBy::QUARTERS);
        pDimData->AddGroupDimension(aGroupDim);

        // Set orientation.
        ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(aGroupDimName);
        pDim->SetOrientation(sheet::DataPilotFieldOrientation_ROW);
        pSaveData->SetPosition(pDim, 0); // set it to the left end.
    }

    {
        // Add year dimension.  This is a new dimension also.
        rtl::OUString aGroupDimName =
            pDimData->CreateDateGroupDimName(
                sheet::DataPilotFieldGroupBy::YEARS, *pDPObj, true, NULL);
        ScDPSaveGroupDimension aGroupDim(aBaseDimName, aGroupDimName);
        aGroupDim.SetDateInfo(aInfo, sheet::DataPilotFieldGroupBy::YEARS);
        pDimData->AddGroupDimension(aGroupDim);

        // Set orientation.
        ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(aGroupDimName);
        pDim->SetOrientation(sheet::DataPilotFieldOrientation_ROW);
        pSaveData->SetPosition(pDim, 0); // set it to the left end.
    }

    pDPObj->SetSaveData(*pSaveData);
    ScRange aOutRange = refreshGroups(pDPs, pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][4] = {
            { "Years", "Quarters", "Date", 0 },
            { "2011", "Q1", "Jan", "1" },
            { 0, 0,         "Mar", "2" },
            { 0,      "Q3", "Sep", "7" },
            { "2012", "Q1", "Jan", "3" },
            { 0, 0,         "Feb", "9" },
            { 0, 0,         "Mar", "6" },
            { 0,      "Q4", "Dec", "8" },
            { "Total Result", 0, 0, "36" },
        };

        bSuccess = checkDPTableOutput<4>(m_pDoc, aOutRange, aOutputCheck, "Years, quarters and months date groups.");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    {
        // Let's hide year 2012.
        pSaveData = pDPObj->GetSaveData();
        ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(rtl::OUString("Years"));
        CPPUNIT_ASSERT_MESSAGE("Years dimension should exist.", pDim);
        ScDPSaveMember* pMem = pDim->GetMemberByName(rtl::OUString("2012"));
        CPPUNIT_ASSERT_MESSAGE("Member should exist.", pMem);
        pMem->SetIsVisible(false);
    }
    pDPObj->SetSaveData(*pSaveData);
    pDPObj->ReloadGroupTableData();
    pDPObj->InvalidateData();

    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][4] = {
            { "Years", "Quarters", "Date", 0 },
            { "2011", "Q1", "Jan", "1" },
            { 0, 0,         "Mar", "2" },
            { 0,      "Q3", "Sep", "7" },
            { "Total Result", 0, 0, "10" },
        };

        bSuccess = checkDPTableOutput<4>(m_pDoc, aOutRange, aOutputCheck, "Year 2012 data now hidden");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableEmptyRows()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "A", "1" },
        { "B", "2" },
        { "C", "3" },
        { "D", "4" },
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    // Extend the range downward to include some trailing empty rows.
    aDataRange.aEnd.IncRow(2);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "1" },
            { "B", "2" },
            { "C", "3" },
            { "D", "4" },
            { "(empty)", 0 },
            { "Total Result", "10" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Include empty rows");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // This time, ignore empty rows.
    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("Save data doesn't exist.", pSaveData);
    pSaveData->SetIgnoreEmptyRows(true);
    pDPObj->ClearTableData();
    aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "1" },
            { "B", "2" },
            { "C", "3" },
            { "D", "4" },
            { "Total Result", "10" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Ignore empty rows");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Modify the source to remove member 'A', then refresh the table.
    m_pDoc->SetString(1, 2, 0, "B");

    std::set<ScDPObject*> aRefs;
    sal_uLong nErr = pDPs->ReloadCache(pDPObj, aRefs);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload cache.", !nErr);
    CPPUNIT_ASSERT_MESSAGE("There should only be one pivot table linked to this cache.",
                           aRefs.size() == 1 && *aRefs.begin() == pDPObj);

    pDPObj->ClearTableData();
    aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "B", "3" },
            { "C", "3" },
            { "D", "4" },
            { "Total Result", "10" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Ignore empty rows");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableTextNumber()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "0001", "1" },
        { "0002", "2" },
        { "0003", "3" },
        { "0004", "4" },
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    // Insert raw data such that the first column values are entered as text.
    size_t nRowCount = SAL_N_ELEMENTS(aData);
    for (size_t nRow = 0; nRow < nRowCount; ++nRow)
    {
        ScSetStringParam aParam;
        aParam.mbDetectNumberFormat = false;
        aParam.meSetTextNumFormat = ScSetStringParam::Always;
        m_pDoc->SetString(0, nRow, 0, OUString::createFromAscii(aData[nRow][0]), &aParam);
        aParam.meSetTextNumFormat = ScSetStringParam::Never;
        m_pDoc->SetString(1, nRow, 0, OUString::createFromAscii(aData[nRow][1]), &aParam);

        if (nRow == 0)
            // Don't check the header row.
            continue;

        // Check the data rows.
        CPPUNIT_ASSERT_MESSAGE("This cell is supposed to be text.", m_pDoc->HasStringData(0, nRow, 0));
        CPPUNIT_ASSERT_MESSAGE("This cell is supposed to be numeric.", m_pDoc->HasValueData(1, nRow, 0));
    }

    ScRange aDataRange(0, 0, 0, 1, 4, 0);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "0001", "1" },
            { "0002", "2" },
            { "0003", "3" },
            { "0004", "4" },
            { "Total Result", "10" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Text number field members");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                                 pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableCaseInsensitiveStrings()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "A", "1" },
        { "a", "2" },
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "3" },
            { "Total Result", "3" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Case insensitive strings");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testSheetCopy()
{
    OUString aTabName("TestTab");
    m_pDoc->InsertTab(0, aTabName);
    CPPUNIT_ASSERT_MESSAGE("document should have one sheet to begin with.", m_pDoc->GetTableCount() == 1);
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    // Copy and test the result.
    m_pDoc->CopyTab(0, 1);
    CPPUNIT_ASSERT_MESSAGE("document now should have two sheets.", m_pDoc->GetTableCount() == 2);
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("copied sheet should also have all rows visible as the original.", !bHidden && nRow1 == 0 && nRow2 == MAXROW);
    m_pDoc->DeleteTab(1);

    m_pDoc->SetRowHidden(5, 10, 0, true);
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);

    // Copy the sheet once again.
    m_pDoc->CopyTab(0, 1);
    CPPUNIT_ASSERT_MESSAGE("document now should have two sheets.", m_pDoc->GetTableCount() == 2);
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testSheetMove()
{
    OUString aTabName("TestTab1");
    m_pDoc->InsertTab(0, aTabName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document should have one sheet to begin with.", m_pDoc->GetTableCount(), static_cast<SCTAB>(1));
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    //test if inserting before another sheet works
    m_pDoc->InsertTab(0, OUString("TestTab2"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document should have two sheets", m_pDoc->GetTableCount(), static_cast<SCTAB>(2));
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    // Move and test the result.
    m_pDoc->MoveTab(0, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.", m_pDoc->GetTableCount(), static_cast<SCTAB>(2));
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("copied sheet should also have all rows visible as the original.", !bHidden && nRow1 == 0 && nRow2 == MAXROW);
    rtl::OUString aName;
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_MESSAGE( "sheets should have changed places", aName == "TestTab1" );

    m_pDoc->SetRowHidden(5, 10, 0, true);
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);

    // Move the sheet once again.
    m_pDoc->MoveTab(1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.", m_pDoc->GetTableCount(), static_cast<SCTAB>(2));
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_MESSAGE( "sheets should have changed places", aName == "TestTab2" );
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

ScDocShell* findLoadedDocShellByName(const OUString& rName)
{
    TypeId aType(TYPE(ScDocShell));
    ScDocShell* pShell = static_cast<ScDocShell*>(SfxObjectShell::GetFirst(&aType, false));
    while (pShell)
    {
        SfxMedium* pMedium = pShell->GetMedium();
        if (pMedium)
        {
            OUString aName = pMedium->GetName();
            if (aName.equals(rName))
                return pShell;
        }
        pShell = static_cast<ScDocShell*>(SfxObjectShell::GetNext(*pShell, &aType, false));
    }
    return NULL;
}

ScRange getCachedRange(const ScExternalRefCache::TableTypeRef& pCacheTab)
{
    ScRange aRange;

    vector<SCROW> aRows;
    pCacheTab->getAllRows(aRows);
    vector<SCROW>::const_iterator itrRow = aRows.begin(), itrRowEnd = aRows.end();
    bool bFirst = true;
    for (; itrRow != itrRowEnd; ++itrRow)
    {
        SCROW nRow = *itrRow;
        vector<SCCOL> aCols;
        pCacheTab->getAllCols(nRow, aCols);
        vector<SCCOL>::const_iterator itrCol = aCols.begin(), itrColEnd = aCols.end();
        for (; itrCol != itrColEnd; ++itrCol)
        {
            SCCOL nCol = *itrCol;
            if (bFirst)
            {
                aRange.aStart = ScAddress(nCol, nRow, 0);
                aRange.aEnd = aRange.aStart;
                bFirst = false;
            }
            else
            {
                if (nCol < aRange.aStart.Col())
                    aRange.aStart.SetCol(nCol);
                else if (aRange.aEnd.Col() < nCol)
                    aRange.aEnd.SetCol(nCol);

                if (nRow < aRange.aStart.Row())
                    aRange.aStart.SetRow(nRow);
                else if (aRange.aEnd.Row() < nRow)
                    aRange.aEnd.SetRow(nRow);
            }
        }
    }
    return aRange;
}

void Test::testExternalRef()
{
    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString aExtDocName("file:///extdata.fake");
    OUString aExtSh1Name("Data1");
    OUString aExtSh2Name("Data2");
    OUString aExtSh3Name("Data3");
    SfxMedium* pMed = new SfxMedium(aExtDocName, STREAM_STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != NULL);

    // Populate the external source document.
    ScDocument* pExtDoc = xExtDocSh->GetDocument();
    pExtDoc->InsertTab(0, aExtSh1Name);
    pExtDoc->InsertTab(1, aExtSh2Name);
    pExtDoc->InsertTab(2, aExtSh3Name);

    OUString name("Name");
    OUString value("Value");
    OUString andy("Andy");
    OUString bruce("Bruce");
    OUString charlie("Charlie");
    OUString david("David");
    OUString edward("Edward");
    OUString frank("Frank");
    OUString george("George");
    OUString henry("Henry");

    // Sheet 1
    pExtDoc->SetString(0, 0, 0, name);
    pExtDoc->SetString(0, 1, 0, andy);
    pExtDoc->SetString(0, 2, 0, bruce);
    pExtDoc->SetString(0, 3, 0, charlie);
    pExtDoc->SetString(0, 4, 0, david);
    pExtDoc->SetString(1, 0, 0, value);
    double val = 10;
    pExtDoc->SetValue(1, 1, 0, val);
    val = 11;
    pExtDoc->SetValue(1, 2, 0, val);
    val = 12;
    pExtDoc->SetValue(1, 3, 0, val);
    val = 13;
    pExtDoc->SetValue(1, 4, 0, val);

    // Sheet 2 remains empty.

    // Sheet 3
    pExtDoc->SetString(0, 0, 2, name);
    pExtDoc->SetString(0, 1, 2, edward);
    pExtDoc->SetString(0, 2, 2, frank);
    pExtDoc->SetString(0, 3, 2, george);
    pExtDoc->SetString(0, 4, 2, henry);
    pExtDoc->SetString(1, 0, 2, value);
    val = 99;
    pExtDoc->SetValue(1, 1, 2, val);
    val = 98;
    pExtDoc->SetValue(1, 2, 2, val);
    val = 97;
    pExtDoc->SetValue(1, 3, 2, val);
    val = 96;
    pExtDoc->SetValue(1, 4, 2, val);

    // Test external refernces on the main document while the external
    // document is still in memory.
    OUString test;
    m_pDoc->InsertTab(0, OUString("Test Sheet"));
    m_pDoc->SetString(0, 0, 0, OUString("='file:///extdata.fake'#Data1.A1"));
    m_pDoc->GetString(0, 0, 0, test);
    CPPUNIT_ASSERT_MESSAGE("Value is different from the original", test.equals(name));

    // After the initial access to the external document, the external ref
    // manager should create sheet cache entries for *all* sheets from that
    // document.  Note that the doc may have more than 3 sheets but ensure
    // that the first 3 are what we expect.
    ScExternalRefManager* pRefMgr = m_pDoc->GetExternalRefManager();
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aExtDocName);
    vector<OUString> aTabNames;
    pRefMgr->getAllCachedTableNames(nFileId, aTabNames);
    CPPUNIT_ASSERT_MESSAGE("There should be at least 3 sheets.", aTabNames.size() >= 3);
    CPPUNIT_ASSERT_MESSAGE("Unexpected sheet name.", aTabNames[0].equals(aExtSh1Name));
    CPPUNIT_ASSERT_MESSAGE("Unexpected sheet name.", aTabNames[1].equals(aExtSh2Name));
    CPPUNIT_ASSERT_MESSAGE("Unexpected sheet name.", aTabNames[2].equals(aExtSh3Name));

    m_pDoc->SetString(1, 0, 0, OUString("='file:///extdata.fake'#Data1.B1"));
    m_pDoc->GetString(1, 0, 0, test);
    CPPUNIT_ASSERT_MESSAGE("Value is different from the original", test.equals(value));

    m_pDoc->SetString(0, 1, 0, OUString("='file:///extdata.fake'#Data1.A2"));
    m_pDoc->SetString(0, 2, 0, OUString("='file:///extdata.fake'#Data1.A3"));
    m_pDoc->SetString(0, 3, 0, OUString("='file:///extdata.fake'#Data1.A4"));
    m_pDoc->SetString(0, 4, 0, OUString("='file:///extdata.fake'#Data1.A5"));
    m_pDoc->SetString(0, 5, 0, OUString("='file:///extdata.fake'#Data1.A6"));

    {
        // Referencing an empty cell should display '0'.
        const char* pChecks[] = { "Andy", "Bruce", "Charlie", "David", "0" };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            m_pDoc->GetString(0, static_cast<SCROW>(i+1), 0, test);
            CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", test.equalsAscii(pChecks[i]));
        }
    }
    m_pDoc->SetString(1, 1, 0, OUString("='file:///extdata.fake'#Data1.B2"));
    m_pDoc->SetString(1, 2, 0, OUString("='file:///extdata.fake'#Data1.B3"));
    m_pDoc->SetString(1, 3, 0, OUString("='file:///extdata.fake'#Data1.B4"));
    m_pDoc->SetString(1, 4, 0, OUString("='file:///extdata.fake'#Data1.B5"));
    m_pDoc->SetString(1, 5, 0, OUString("='file:///extdata.fake'#Data1.B6"));
    {
        double pChecks[] = { 10, 11, 12, 13, 0 };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            m_pDoc->GetValue(1, static_cast<SCROW>(i+1), 0, val);
            CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", val == pChecks[i]);
        }
    }

    m_pDoc->SetString(2, 0, 0, OUString("='file:///extdata.fake'#Data3.A1"));
    m_pDoc->SetString(2, 1, 0, OUString("='file:///extdata.fake'#Data3.A2"));
    m_pDoc->SetString(2, 2, 0, OUString("='file:///extdata.fake'#Data3.A3"));
    m_pDoc->SetString(2, 3, 0, OUString("='file:///extdata.fake'#Data3.A4"));
    {
        const char* pChecks[] = { "Name", "Edward", "Frank", "George" };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            m_pDoc->GetString(2, static_cast<SCROW>(i), 0, test);
            CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", test.equalsAscii(pChecks[i]));
        }
    }

    m_pDoc->SetString(3, 0, 0, OUString("='file:///extdata.fake'#Data3.B1"));
    m_pDoc->SetString(3, 1, 0, OUString("='file:///extdata.fake'#Data3.B2"));
    m_pDoc->SetString(3, 2, 0, OUString("='file:///extdata.fake'#Data3.B3"));
    m_pDoc->SetString(3, 3, 0, OUString("='file:///extdata.fake'#Data3.B4"));
    {
        const char* pChecks[] = { "Value", "99", "98", "97" };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            m_pDoc->GetString(3, static_cast<SCROW>(i), 0, test);
            CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", test.equalsAscii(pChecks[i]));
        }
    }

    // At this point, all accessed cell data from the external document should
    // have been cached.
    ScExternalRefCache::TableTypeRef pCacheTab = pRefMgr->getCacheTable(
        nFileId, aExtSh1Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 1 should exist.", pCacheTab.get() != NULL);
    ScRange aCachedRange = getCachedRange(pCacheTab);
    CPPUNIT_ASSERT_MESSAGE("Unexpected cached data range.",
                           aCachedRange.aStart.Col() == 0 && aCachedRange.aEnd.Col() == 1 &&
                           aCachedRange.aStart.Row() == 0 && aCachedRange.aEnd.Row() == 4);

    // Sheet2 is not referenced at all; the cache table shouldn't even exist.
    pCacheTab = pRefMgr->getCacheTable(nFileId, aExtSh2Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 2 should *not* exist.", pCacheTab.get() == NULL);

    // Sheet3's row 5 is not referenced; it should not be cached.
    pCacheTab = pRefMgr->getCacheTable(nFileId, aExtSh3Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 3 should exist.", pCacheTab.get() != NULL);
    aCachedRange = getCachedRange(pCacheTab);
    CPPUNIT_ASSERT_MESSAGE("Unexpected cached data range.",
                           aCachedRange.aStart.Col() == 0 && aCachedRange.aEnd.Col() == 1 &&
                           aCachedRange.aStart.Row() == 0 && aCachedRange.aEnd.Row() == 3);

    // Unload the external document shell.
    xExtDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("external document instance should have been unloaded.",
                           findLoadedDocShellByName(aExtDocName) == NULL);

    m_pDoc->DeleteTab(0);
}

void testExtRefFuncT(ScDocument* pDoc, ScDocument* pExtDoc)
{
    clearRange(pDoc, ScRange(0, 0, 0, 1, 9, 0));
    clearRange(pExtDoc, ScRange(0, 0, 0, 1, 9, 0));

    pExtDoc->SetString(0, 0, 0, rtl::OUString("'1.2"));
    pExtDoc->SetString(0, 1, 0, rtl::OUString("Foo"));
    pExtDoc->SetValue(0, 2, 0, 12.3);
    pDoc->SetString(0, 0, 0, rtl::OUString("=T('file:///extdata.fake'#Data.A1)"));
    pDoc->SetString(0, 1, 0, rtl::OUString("=T('file:///extdata.fake'#Data.A2)"));
    pDoc->SetString(0, 2, 0, rtl::OUString("=T('file:///extdata.fake'#Data.A3)"));
    pDoc->CalcAll();

    rtl::OUString aRes = pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE( "Unexpected result with T.", aRes == "1.2" );
    aRes = pDoc->GetString(0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE( "Unexpected result with T.", aRes == "Foo" );
    aRes = pDoc->GetString(0, 2, 0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected result with T.", aRes.isEmpty());
}

void Test::testExternalRefFunctions()
{
    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString aExtDocName("file:///extdata.fake");
    SfxMedium* pMed = new SfxMedium(aExtDocName, STREAM_STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != NULL);

    ScExternalRefManager* pRefMgr = m_pDoc->GetExternalRefManager();
    CPPUNIT_ASSERT_MESSAGE("external reference manager doesn't exist.", pRefMgr);
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aExtDocName);
    const OUString* pFileName = pRefMgr->getExternalFileName(nFileId);
    CPPUNIT_ASSERT_MESSAGE("file name registration has somehow failed.",
                           pFileName && pFileName->equals(aExtDocName));

    // Populate the external source document.
    ScDocument* pExtDoc = xExtDocSh->GetDocument();
    pExtDoc->InsertTab(0, rtl::OUString("Data"));
    double val = 1;
    pExtDoc->SetValue(0, 0, 0, val);
    // leave cell B1 empty.
    val = 2;
    pExtDoc->SetValue(0, 1, 0, val);
    pExtDoc->SetValue(1, 1, 0, val);
    val = 3;
    pExtDoc->SetValue(0, 2, 0, val);
    pExtDoc->SetValue(1, 2, 0, val);
    val = 4;
    pExtDoc->SetValue(0, 3, 0, val);
    pExtDoc->SetValue(1, 3, 0, val);

    m_pDoc->InsertTab(0, rtl::OUString("Test"));

    struct {
        const char* pFormula; double fResult;
    } aChecks[] = {
        { "=SUM('file:///extdata.fake'#Data.A1:A4)",     10 },
        { "=SUM('file:///extdata.fake'#Data.B1:B4)",     9 },
        { "=AVERAGE('file:///extdata.fake'#Data.A1:A4)", 2.5 },
        { "=AVERAGE('file:///extdata.fake'#Data.B1:B4)", 3 },
        { "=COUNT('file:///extdata.fake'#Data.A1:A4)",   4 },
        { "=COUNT('file:///extdata.fake'#Data.B1:B4)",   3 }
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        m_pDoc->SetString(0, 0, 0, rtl::OUString::createFromAscii(aChecks[i].pFormula));
        m_pDoc->CalcAll();
        m_pDoc->GetValue(0, 0, 0, val);
        CPPUNIT_ASSERT_MESSAGE("unexpected result involving external ranges.", val == aChecks[i].fResult);
    }

    pRefMgr->clearCache(nFileId);
    testExtRefFuncT(m_pDoc, pExtDoc);

    // Unload the external document shell.
    xExtDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("external document instance should have been unloaded.",
                           findLoadedDocShellByName(aExtDocName) == NULL);

    m_pDoc->DeleteTab(0);
}

void Test::testDataArea()
{
    m_pDoc->InsertTab(0, OUString("Data"));

    // Totally empty sheet should be rightfully considered empty in all accounts.
    CPPUNIT_ASSERT_MESSAGE("Sheet is expected to be empty.", m_pDoc->IsPrintEmpty(0, 0, 0, 100, 100));
    CPPUNIT_ASSERT_MESSAGE("Sheet is expected to be empty.", m_pDoc->IsBlockEmpty(0, 0, 0, 100, 100));

    // Now, set borders in some cells....
    ::editeng::SvxBorderLine aLine(NULL, 50, table::BorderLineStyle::SOLID);
    SvxBoxItem aBorderItem(ATTR_BORDER);
    aBorderItem.SetLine(&aLine, BOX_LINE_LEFT);
    aBorderItem.SetLine(&aLine, BOX_LINE_RIGHT);
    for (SCROW i = 0; i < 100; ++i)
        // Set borders from row 1 to 100.
        m_pDoc->ApplyAttr(0, i, 0, aBorderItem);

    // Now the sheet is considered non-empty for printing purposes, but still
    // be empty in all the other cases.
    CPPUNIT_ASSERT_MESSAGE("Empty sheet with borders should be printable.",
                           !m_pDoc->IsPrintEmpty(0, 0, 0, 100, 100));
    CPPUNIT_ASSERT_MESSAGE("But it should still be considered empty in all the other cases.",
                           m_pDoc->IsBlockEmpty(0, 0, 0, 100, 100));

    // Adding a real cell content should turn the block non-empty.
    m_pDoc->SetString(0, 0, 0, OUString("Some text"));
    CPPUNIT_ASSERT_MESSAGE("Now the block should not be empty with a real cell content.",
                           !m_pDoc->IsBlockEmpty(0, 0, 0, 100, 100));

    // TODO: Add more tests for normal data area calculation.

    m_pDoc->DeleteTab(0);
}

void Test::testStreamValid()
{
    m_pDoc->InsertTab(0, OUString("Sheet1"));
    m_pDoc->InsertTab(1, OUString("Sheet2"));
    m_pDoc->InsertTab(2, OUString("Sheet3"));
    m_pDoc->InsertTab(3, OUString("Sheet4"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("We should have 4 sheet instances.", m_pDoc->GetTableCount(), static_cast<SCTAB>(4));

    OUString a1("A1");
    OUString a2("A2");
    OUString test;

    // Put values into Sheet1.
    m_pDoc->SetString(0, 0, 0, a1);
    m_pDoc->SetString(0, 1, 0, a2);
    m_pDoc->GetString(0, 0, 0, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet1.A1", test.equals(a1));
    m_pDoc->GetString(0, 1, 0, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet1.A2", test.equals(a2));

    // Put formulas into Sheet2 to Sheet4 to reference values from Sheet1.
    m_pDoc->SetString(0, 0, 1, OUString("=Sheet1.A1"));
    m_pDoc->SetString(0, 1, 1, OUString("=Sheet1.A2"));
    m_pDoc->SetString(0, 0, 2, OUString("=Sheet1.A1"));
    m_pDoc->SetString(0, 0, 3, OUString("=Sheet1.A2"));

    m_pDoc->GetString(0, 0, 1, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet2.A1", test.equals(a1));
    m_pDoc->GetString(0, 1, 1, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet2.A2", test.equals(a2));
    m_pDoc->GetString(0, 0, 2, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet3.A1", test.equals(a1));
    m_pDoc->GetString(0, 0, 3, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet3.A1", test.equals(a2));

    // Set all sheet streams valid after all the initial cell values are in
    // place. In reality we need to have real XML streams stored in order to
    // claim they are valid, but we are just testing the flag values here.
    m_pDoc->SetStreamValid(0, true);
    m_pDoc->SetStreamValid(1, true);
    m_pDoc->SetStreamValid(2, true);
    m_pDoc->SetStreamValid(3, true);
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(0));
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(1));
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(2));
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(3));

    // Now, insert a new row at row 2 position on Sheet1.  This will move cell
    // A2 downward but cell A1 remains unmoved.
    m_pDoc->InsertRow(0, 0, MAXCOL, 0, 1, 2);
    m_pDoc->GetString(0, 0, 0, test);
    CPPUNIT_ASSERT_MESSAGE("Cell A1 should not have moved.", test.equals(a1));
    m_pDoc->GetString(0, 3, 0, test);
    CPPUNIT_ASSERT_MESSAGE("the old cell A2 should now be at A4.", test.equals(a2));
    const ScBaseCell* pCell = m_pDoc->GetCell(ScAddress(0, 1, 0));
    CPPUNIT_ASSERT_MESSAGE("Cell A2 should be empty.", pCell == NULL);
    pCell = m_pDoc->GetCell(ScAddress(0, 2, 0));
    CPPUNIT_ASSERT_MESSAGE("Cell A3 should be empty.", pCell == NULL);

    // After the move, Sheet1, Sheet2, and Sheet4 should have their stream
    // invalidated, whereas Sheet3's stream should still be valid.
    CPPUNIT_ASSERT_MESSAGE("Stream should have been invalidated.", !m_pDoc->IsStreamValid(0));
    CPPUNIT_ASSERT_MESSAGE("Stream should have been invalidated.", !m_pDoc->IsStreamValid(1));
    CPPUNIT_ASSERT_MESSAGE("Stream should have been invalidated.", !m_pDoc->IsStreamValid(3));
    CPPUNIT_ASSERT_MESSAGE("Stream should still be valid.", m_pDoc->IsStreamValid(2));

    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testFunctionLists()
{
    const char* aDataBase[] = {
        "DAVERAGE",
        "DCOUNT",
        "DCOUNTA",
        "DGET",
        "DMAX",
        "DMIN",
        "DPRODUCT",
        "DSTDEV",
        "DSTDEVP",
        "DSUM",
        "DVAR",
        "DVARP",
        0
    };

    const char* aDateTime[] = {
        "DATE",
        "DATEDIF",
        "DATEVALUE",
        "DAY",
        "DAYS",
        "DAYS360",
        "EASTERSUNDAY",
        "HOUR",
        "MINUTE",
        "MONTH",
        "NOW",
        "SECOND",
        "TIME",
        "TIMEVALUE",
        "TODAY",
        "WEEKDAY",
        "WEEKNUM",
        "YEAR",
        0
    };

    const char* aFinancial[] = {
        "CUMIPMT",
        "CUMPRINC",
        "DB",
        "DDB",
        "DURATION",
        "EFFECTIVE",
        "FV",
        "IPMT",
        "IRR",
        "ISPMT",
        "MIRR",
        "NOMINAL",
        "NPER",
        "NPV",
        "PMT",
        "PPMT",
        "PV",
        "RATE",
        "RRI",
        "SLN",
        "SYD",
        "VDB",
        0
    };

    const char* aInformation[] = {
        "CELL",
        "CURRENT",
        "FORMULA",
        "INFO",
        "ISBLANK",
        "ISERR",
        "ISERROR",
        "ISFORMULA",
        "ISLOGICAL",
        "ISNA",
        "ISNONTEXT",
        "ISNUMBER",
        "ISREF",
        "ISTEXT",
        "N",
        "NA",
        "TYPE",
        0
    };

    const char* aLogical[] = {
        "AND",
        "FALSE",
        "IF",
        "NOT",
        "OR",
        "TRUE",
        "XOR",
        0
    };

    const char* aMathematical[] = {
        "ABS",
        "ACOS",
        "ACOSH",
        "ACOT",
        "ACOTH",
        "ASIN",
        "ASINH",
        "ATAN",
        "ATAN2",
        "ATANH",
        "BITAND",
        "BITLSHIFT",
        "BITOR",
        "BITRSHIFT",
        "BITXOR",
        "CEILING",
        "COMBIN",
        "COMBINA",
        "CONVERT",
        "COS",
        "COSH",
        "COT",
        "COTH",
        "COUNTBLANK",
        "COUNTIF",
        "CSC",
        "CSCH",
        "DEGREES",
        "EUROCONVERT",
        "EVEN",
        "EXP",
        "FACT",
        "FLOOR",
        "GCD",
        "INT",
        "ISEVEN",
        "ISODD",
        "LCM",
        "LN",
        "LOG",
        "LOG10",
        "MOD",
        "ODD",
        "PI",
        "POWER",
        "PRODUCT",
        "RADIANS",
        "RAND",
        "ROUND",
        "ROUNDDOWN",
        "ROUNDUP",
        "SEC",
        "SECH",
        "SIGN",
        "SIN",
        "SINH",
        "SQRT",
        "SUBTOTAL",
        "SUM",
        "SUMIF",
        "SUMSQ",
        "TAN",
        "TANH",
        "TRUNC",
        0
    };

    const char* aArray[] = {
        "FREQUENCY",
        "GROWTH",
        "LINEST",
        "LOGEST",
        "MDETERM",
        "MINVERSE",
        "MMULT",
        "MUNIT",
        "SUMPRODUCT",
        "SUMX2MY2",
        "SUMX2PY2",
        "SUMXMY2",
        "TRANSPOSE",
        "TREND",
        0
    };

    const char* aStatistical[] = {
        "AVEDEV",
        "AVERAGE",
        "AVERAGEA",
        "B",
        "BETADIST",
        "BETAINV",
        "BINOMDIST",
        "CHIDIST",
        "CHIINV",
        "CHISQDIST",
        "CHISQINV",
        "CHITEST",
        "CONFIDENCE",
        "CORREL",
        "COUNT",
        "COUNTA",
        "COVAR",
        "CRITBINOM",
        "DEVSQ",
        "EXPONDIST",
        "FDIST",
        "FINV",
        "FISHER",
        "FISHERINV",
        "FORECAST",
        "FTEST",
        "GAMMA",
        "GAMMADIST",
        "GAMMAINV",
        "GAMMALN",
        "GAUSS",
        "GEOMEAN",
        "HARMEAN",
        "HYPGEOMDIST",
        "INTERCEPT",
        "KURT",
        "LARGE",
        "LOGINV",
        "LOGNORMDIST",
        "MAX",
        "MAXA",
        "MEDIAN",
        "MIN",
        "MINA",
        "MODE",
        "NEGBINOMDIST",
        "NORMDIST",
        "NORMINV",
        "NORMSDIST",
        "NORMSINV",
        "PEARSON",
        "PERCENTILE",
        "PERCENTRANK",
        "PERMUT",
        "PERMUTATIONA",
        "PHI",
        "POISSON",
        "PROB",
        "QUARTILE",
        "RANK",
        "RSQ",
        "SKEW",
        "SLOPE",
        "SMALL",
        "STANDARDIZE",
        "STDEV",
        "STDEVA",
        "STDEVP",
        "STDEVPA",
        "STEYX",
        "TDIST",
        "TINV",
        "TRIMMEAN",
        "TTEST",
        "VAR",
        "VARA",
        "VARP",
        "VARPA",
        "WEIBULL",
        "ZTEST",
        0
    };

    const char* aSpreadsheet[] = {
        "ADDRESS",
        "AREAS",
        "CHOOSE",
        "COLUMN",
        "COLUMNS",
        "DDE",
        "ERRORTYPE",
        "GETPIVOTDATA",
        "HLOOKUP",
        "HYPERLINK",
        "INDEX",
        "INDIRECT",
        "LOOKUP",
        "MATCH",
        "OFFSET",
        "ROW",
        "ROWS",
        "SHEET",
        "SHEETS",
        "STYLE",
        "VLOOKUP",
        0
    };

    const char* aText[] = {
        "ARABIC",
        "ASC",
        "BAHTTEXT",
        "BASE",
        "CHAR",
        "CLEAN",
        "CODE",
        "CONCATENATE",
        "DECIMAL",
        "DOLLAR",
        "EXACT",
        "FIND",
        "FIXED",
        "JIS",
        "LEFT",
        "LEN",
        "LOWER",
        "MID",
        "PROPER",
        "REPLACE",
        "REPT",
        "RIGHT",
        "ROMAN",
        "SEARCH",
        "SUBSTITUTE",
        "T",
        "TEXT",
        "TRIM",
        "UNICHAR",
        "UNICODE",
        "UPPER",
        "VALUE",
        0
    };

    struct {
        const char* Category; const char** Functions;
    } aTests[] = {
        { "Database",     aDataBase },
        { "Date&Time",    aDateTime },
        { "Financial",    aFinancial },
        { "Information",  aInformation },
        { "Logical",      aLogical },
        { "Mathematical", aMathematical },
        { "Array",        aArray },
        { "Statistical",  aStatistical },
        { "Spreadsheet",  aSpreadsheet },
        { "Text",         aText },
        { "Add-in",       0 },
        { 0, 0 }
    };

    ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
    sal_uInt32 n = pFuncMgr->getCount();
    for (sal_uInt32 i = 0; i < n; ++i)
    {
        const formula::IFunctionCategory* pCat = pFuncMgr->getCategory(i);
        CPPUNIT_ASSERT_MESSAGE("Unexpected category name", pCat->getName().equalsAscii(aTests[i].Category));
        sal_uInt32 nFuncCount = pCat->getCount();
        for (sal_uInt32 j = 0; j < nFuncCount; ++j)
        {
            const formula::IFunctionDescription* pFunc = pCat->getFunction(j);
            CPPUNIT_ASSERT_MESSAGE("Unexpected function name", pFunc->getFunctionName().equalsAscii(aTests[i].Functions[j]));
        }
    }
}

void Test::testGraphicsInGroup()
{
    OUString aTabName("TestTab");
    m_pDoc->InsertTab(0, aTabName);
    CPPUNIT_ASSERT_MESSAGE("document should have one sheet to begin with.", m_pDoc->GetTableCount() == 1);
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    m_pDoc->InitDrawLayer();
    ScDrawLayer *pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("must have a draw layer", pDrawLayer != NULL);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("must have a draw page", pPage != NULL);

    {
        //Add a square
        Rectangle aOrigRect(2,2,100,100);
        SdrRectObj *pObj = new SdrRectObj(aOrigRect);
        pPage->InsertObject(pObj);
        const Rectangle &rNewRect = pObj->GetLogicRect();
        CPPUNIT_ASSERT_MESSAGE("must have equal position and size", aOrigRect == rNewRect);

        ScDrawLayer::SetPageAnchored(*pObj);

        //Use a range of rows guaranteed to include all of the square
        m_pDoc->ShowRows(0, 100, 0, false);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Should not change when page anchored", aOrigRect == rNewRect);
        m_pDoc->ShowRows(0, 100, 0, true);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Should not change when page anchored", aOrigRect == rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);
        CPPUNIT_ASSERT_MESSAGE("That shouldn't change size or positioning", aOrigRect == rNewRect);

        m_pDoc->ShowRows(0, 100, 0, false);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Left and Right should be unchanged",
            aOrigRect.nLeft == rNewRect.nLeft && aOrigRect.nRight == rNewRect.nRight);
        CPPUNIT_ASSERT_MESSAGE("Height should be minimum allowed height",
            (rNewRect.nBottom - rNewRect.nTop) <= 1);
        m_pDoc->ShowRows(0, 100, 0, true);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Should not change when page anchored", aOrigRect == rNewRect);
    }

    {
        // Add a circle.
        Rectangle aOrigRect = Rectangle(10,10,210,210); // 200 x 200
        SdrCircObj* pObj = new SdrCircObj(OBJ_CIRC, aOrigRect);
        pPage->InsertObject(pObj);
        const Rectangle& rNewRect = pObj->GetLogicRect();
        CPPUNIT_ASSERT_MESSAGE("Position and size of the circle shouldn't change when inserted into the page.",
                               aOrigRect == rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);
        CPPUNIT_ASSERT_MESSAGE("Size changed when cell anchored. Not good.",
                               aOrigRect == rNewRect);

        // Insert 2 rows at the top.  This should push the circle object down.
        m_pDoc->InsertRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->SetDrawPageSize(0);

        // Make sure the size of the circle is still identical.
        CPPUNIT_ASSERT_MESSAGE("Size of the circle has changed, but shouldn't!",
                               aOrigRect.GetSize() == rNewRect.GetSize());

        // Delete 2 rows at the top.  This should bring the circle object to its original position.
        m_pDoc->DeleteRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Failed to move back to its original position.", aOrigRect == rNewRect);
    }

    {
        // Add a line.
        basegfx::B2DPolygon aTempPoly;
        Point aStartPos(10,300), aEndPos(110,200); // bottom-left to top-right.
        Rectangle aOrigRect(10,200,110,300); // 100 x 100
        aTempPoly.append(basegfx::B2DPoint(aStartPos.X(), aStartPos.Y()));
        aTempPoly.append(basegfx::B2DPoint(aEndPos.X(), aEndPos.Y()));
        SdrPathObj* pObj = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aTempPoly));
        pObj->NbcSetLogicRect(aOrigRect);
        pPage->InsertObject(pObj);
        const Rectangle& rNewRect = pObj->GetLogicRect();
        CPPUNIT_ASSERT_MESSAGE("Size differ.", aOrigRect == rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);
        CPPUNIT_ASSERT_MESSAGE("Size changed when cell-anchored. Not good.",
                               aOrigRect == rNewRect);

        // Insert 2 rows at the top and delete them immediately.
        m_pDoc->InsertRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->DeleteRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Size of a line object changed after row insertion and removal.",
                               aOrigRect == rNewRect);

        sal_Int32 n = pObj->GetPointCount();
        CPPUNIT_ASSERT_MESSAGE("There should be exactly 2 points in a line object.", n == 2);
        CPPUNIT_ASSERT_MESSAGE("Line shape has changed.",
                               aStartPos == pObj->GetPoint(0) && aEndPos == pObj->GetPoint(1));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testGraphicsOnSheetMove()
{
    m_pDoc->InsertTab(0, rtl::OUString("Tab1"));
    m_pDoc->InsertTab(1, rtl::OUString("Tab2"));
    CPPUNIT_ASSERT_MESSAGE("There should be only 2 sheets to begin with", m_pDoc->GetTableCount() == 2);

    m_pDoc->InitDrawLayer();
    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No drawing layer.", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No page instance for the 1st sheet.", pPage);

    // Insert an object.
    Rectangle aObjRect(2,2,100,100);
    SdrObject* pObj = new SdrRectObj(aObjRect);
    pPage->InsertObject(pObj);
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be one object on the 1st sheet.", pPage->GetObjCount(), static_cast<sal_uIntPtr>(1));

    const ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Object meta-data doesn't exist.", pData);
    CPPUNIT_ASSERT_MESSAGE("Wrong sheet ID in cell anchor data!", pData->maStart.Tab() == 0 && pData->maEnd.Tab() == 0);

    pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("No page instance for the 2nd sheet.", pPage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd sheet shouldn't have any object.", pPage->GetObjCount(), static_cast<sal_uIntPtr>(0));

    // Insert a new sheet at left-end, and make sure the object has moved to
    // the 2nd page.
    m_pDoc->InsertTab(0, rtl::OUString("NewTab"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 3 sheets.", m_pDoc->GetTableCount(), static_cast<SCTAB>(3));
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("1st sheet should have no object.", pPage && pPage->GetObjCount() == 0);
    pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("2nd sheet should have one object.", pPage && pPage->GetObjCount() == 1);
    pPage = pDrawLayer->GetPage(2);
    CPPUNIT_ASSERT_MESSAGE("3rd sheet should have no object.", pPage && pPage->GetObjCount() == 0);

    CPPUNIT_ASSERT_MESSAGE("Wrong sheet ID in cell anchor data!", pData->maStart.Tab() == 1 && pData->maEnd.Tab() == 1);

    // Now, delete the sheet that just got inserted. The object should be back
    // on the 1st sheet.
    m_pDoc->DeleteTab(0);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("1st sheet should have one object.", pPage && pPage->GetObjCount() == 1);
    CPPUNIT_ASSERT_MESSAGE("Size and position of the object shouldn't change.",
                           pObj->GetLogicRect() == aObjRect);

    CPPUNIT_ASSERT_MESSAGE("Wrong sheet ID in cell anchor data!", pData->maStart.Tab() == 0 && pData->maEnd.Tab() == 0);

    // Move the 1st sheet to the last position.
    m_pDoc->MoveTab(0, 1);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("1st sheet should have no object.", pPage && pPage->GetObjCount() == 0);
    pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("2nd sheet should have one object.", pPage && pPage->GetObjCount() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong sheet ID in cell anchor data!", pData->maStart.Tab() == 1 && pData->maEnd.Tab() == 1);

    // Copy the 2nd sheet, which has one drawing object to the last position.
    m_pDoc->CopyTab(1, 2);
    pPage = pDrawLayer->GetPage(2);
    CPPUNIT_ASSERT_MESSAGE("Copied sheet should have one object.", pPage && pPage->GetObjCount() == 1);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object meta-data.", pData);
    CPPUNIT_ASSERT_MESSAGE("Wrong sheet ID in cell anchor data!", pData->maStart.Tab() == 2 && pData->maEnd.Tab() == 2);

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPostIts()
{
    rtl::OUString aHello("Hello world");
    rtl::OUString aJimBob("Jim Bob");
    rtl::OUString aTabName("PostIts");
    rtl::OUString aTabName2("Table2");
    m_pDoc->InsertTab(0, aTabName);

    ScAddress rAddr(2, 2, 0);
    ScPostIt *pNote = m_pDoc->GetNotes(rAddr.Tab())->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, aHello);
    pNote->SetAuthor(aJimBob);

    ScPostIt *pGetNote = m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr);
    CPPUNIT_ASSERT_MESSAGE("note should be itself", pGetNote == pNote );

    bool bInsertRow = m_pDoc->InsertRow( 0, 0, 100, 0, 1, 1 );
    CPPUNIT_ASSERT_MESSAGE("failed to insert row", bInsertRow );

    CPPUNIT_ASSERT_MESSAGE("note hasn't moved", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == NULL);
    rAddr.IncRow();
    CPPUNIT_ASSERT_MESSAGE("note not there", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    bool bInsertCol = m_pDoc->InsertCol( 0, 0, 100, 0, 1, 1 );
    CPPUNIT_ASSERT_MESSAGE("failed to insert column", bInsertCol );

    CPPUNIT_ASSERT_MESSAGE("note hasn't moved", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == NULL);
    rAddr.IncCol();
    CPPUNIT_ASSERT_MESSAGE("note not there", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    m_pDoc->InsertTab(0, aTabName2);
    CPPUNIT_ASSERT_MESSAGE("note hasn't moved", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == NULL);
    rAddr.IncTab();
    CPPUNIT_ASSERT_MESSAGE("note not there", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    m_pDoc->DeleteTab(0);
    rAddr.IncTab(-1);
    CPPUNIT_ASSERT_MESSAGE("note not there", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    m_pDoc->DeleteTab(0);
}

void Test::testToggleRefFlag()
{
    // In this test, there is no need to insert formula string into a cell in
    // the document, as ScRefFinder does not depend on the content of the
    // document except for the sheet names.

    OUString aTabName("Test");
    m_pDoc->InsertTab(0, aTabName);

    {
        // Calc A1: basic 2D reference

        OUString aFormula("=B100");
        ScAddress aPos(1, 5, 0);
        ScRefFinder aFinder(aFormula, aPos, m_pDoc, formula::FormulaGrammar::CONV_OOO);

        // Original
        CPPUNIT_ASSERT_MESSAGE("Does not equal the original text.", aFormula.equals(aFinder.GetText()));

        // column relative / row relative -> column absolute / row absolute
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=$B$100" );

        // column absolute / row absolute -> column relative / row absolute
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=B$100" );

        // column relative / row absolute -> column absolute / row relative
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=$B100" );

        // column absolute / row relative -> column relative / row relative
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=B100" );
    }

    {
        // Excel R1C1: basic 2D reference

        OUString aFormula("=R2C1");
        ScAddress aPos(3, 5, 0);
        ScRefFinder aFinder(aFormula, aPos, m_pDoc, formula::FormulaGrammar::CONV_XL_R1C1);

        // Original
        CPPUNIT_ASSERT_MESSAGE("Does not equal the original text.", aFormula.equals(aFinder.GetText()));

        // column absolute / row absolute -> column relative / row absolute
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=R2C[-3]" );

        // column relative / row absolute - > column absolute / row relative
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=R[-4]C1" );

        // column absolute / row relative -> column relative / row relative
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=R[-4]C[-3]" );

        // column relative / row relative -> column absolute / row absolute
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=R2C1" );
    }

    // TODO: Add more test cases esp. for 3D references, Excel A1 syntax, and
    // partial selection within formula string.

    m_pDoc->DeleteTab(0);
}

void Test::testAutofilter()
{
    OUString aTabName("Test");
    OUString aDBName("NONAME");

    m_pDoc->InsertTab( 0, aTabName );

    // cell contents (0 = empty cell)
    const char* aData[][3] = {
        { "C1", "C2", "C3" },
        {  "0",  "1",  "A" },
        {  "1",  "2",    0 },
        {  "1",  "2",  "B" },
        {  "0",  "2",  "B" }
    };

    SCCOL nCols = SAL_N_ELEMENTS(aData[0]);
    SCROW nRows = SAL_N_ELEMENTS(aData);

    // Populate cells.
    for (SCROW i = 0; i < nRows; ++i)
        for (SCCOL j = 0; j < nCols; ++j)
            if (aData[i][j])
                m_pDoc->SetString(j, i, 0, rtl::OUString::createFromAscii(aData[i][j]));

    ScDBData* pDBData = new ScDBData(aDBName, 0, 0, 0, nCols-1, nRows-1);
    m_pDoc->SetAnonymousDBData(0,pDBData);

    pDBData->SetAutoFilter(true);
    ScRange aRange;
    pDBData->GetArea(aRange);
    m_pDoc->ApplyFlagsTab( aRange.aStart.Col(), aRange.aStart.Row(),
                           aRange.aEnd.Col(), aRange.aStart.Row(),
                           aRange.aStart.Tab(), SC_MF_AUTO);

    //create the query param
    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);
    ScQueryEntry& rEntry = aParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 0;
    rEntry.eOp = SC_EQUAL;
    rEntry.GetQueryItem().mfVal = 0;
    // add queryParam to database range.
    pDBData->SetQueryParam(aParam);

    // perform the query.
    m_pDoc->Query(0, aParam, true);

    //control output
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(2, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 2 & 3 should be hidden", bHidden && nRow1 == 2 && nRow2 == 3);

    // Remove filtering.
    rEntry.Clear();
    m_pDoc->Query(0, aParam, true);
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("All rows should be shown.", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    // Filter for non-empty cells by column C.
    rEntry.bDoQuery = true;
    rEntry.nField = 2;
    rEntry.SetQueryByNonEmpty();
    m_pDoc->Query(0, aParam, true);

    // only row 3 should be hidden.  The rest should be visible.
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 1 & 2 should be visible.", !bHidden && nRow1 == 0 && nRow2 == 1);
    bHidden = m_pDoc->RowHidden(2, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 3 should be hidden.", bHidden && nRow1 == 2 && nRow2 == 2);
    bHidden = m_pDoc->RowHidden(3, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 4 and down should be visible.", !bHidden && nRow1 == 3 && nRow2 == MAXROW);

    // Now, filter for empty cells by column C.
    rEntry.SetQueryByEmpty();
    m_pDoc->Query(0, aParam, true);

    // Now, only row 1 and 3, and 6 and down should be visible.
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 1 should be visible.", !bHidden && nRow1 == 0 && nRow2 == 0);
    bHidden = m_pDoc->RowHidden(1, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 2 should be hidden.", bHidden && nRow1 == 1 && nRow2 == 1);
    bHidden = m_pDoc->RowHidden(2, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 3 should be visible.", !bHidden && nRow1 == 2 && nRow2 == 2);
    bHidden = m_pDoc->RowHidden(3, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 4 & 5 should be hidden.", bHidden && nRow1 == 3 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 6 and down should be all visible.", !bHidden && nRow1 == 5 && nRow2 == MAXROW);

    m_pDoc->DeleteTab(0);
}

void Test::testCopyPaste()
{
    m_pDoc->InsertTab(0, OUString("Sheet1"));
    m_pDoc->InsertTab(1, OUString("Sheet2"));
    //test copy&paste + ScUndoPaste
    //copy local and global range names in formulas
    //string cells and value cells
    m_pDoc->SetValue(0, 0, 0, 1);
    m_pDoc->SetValue(3, 0, 0, 0);
    m_pDoc->SetValue(3, 1, 0, 1);
    m_pDoc->SetValue(3, 2, 0, 2);
    m_pDoc->SetValue(3, 3, 0, 3);
    m_pDoc->SetString(2, 0, 0, OUString("test"));
    ScAddress aAdr (0, 0, 0);

    //create some range names, local and global
    ScRangeData* pLocal1 = new ScRangeData(m_pDoc, rtl::OUString("local1"), aAdr);
    ScRangeData* pLocal2 = new ScRangeData(m_pDoc, OUString("local2"), aAdr);
    ScRangeData* pGlobal = new ScRangeData(m_pDoc, OUString("global"), aAdr);
    ScRangeName* pGlobalRangeName = new ScRangeName();
    pGlobalRangeName->insert(pGlobal);
    ScRangeName* pLocalRangeName1 = new ScRangeName();
    pLocalRangeName1->insert(pLocal1);
    pLocalRangeName1->insert(pLocal2);
    m_pDoc->SetRangeName(pGlobalRangeName);
    m_pDoc->SetRangeName(0, pLocalRangeName1);

    //add formula
    rtl::OUString aFormulaString("=local1+global+SUM($C$1:$D$4)");
    m_pDoc->SetString(1, 0, 0, aFormulaString);

    double aValue = 0;
    m_pDoc->GetValue(1, 0, 0, aValue);
    std::cout << "Value: " << aValue << std::endl;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("formula should return 8", aValue, 8, 0.00000001);

    //copy Sheet1.A1:C1 to Sheet2.A2:C2
    ScRange aRange(0,0,0,2,0,0);
    ScClipParam aClipParam(aRange, false);
    ScMarkData aMark;
    aMark.SetMarkArea(aRange);
    ScDocument* pClipDoc = new ScDocument(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, pClipDoc, &aMark);

    sal_uInt16 nFlags = IDF_ALL;
    aRange = ScRange(0,1,1,2,1,1);//target: Sheet2.A2:C2
    ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
    pUndoDoc->InitUndo(m_pDoc, 1, 1, true, true);
    ScMarkData aMarkData2;
    aMarkData2.SetMarkArea(aRange);
    ScRefUndoData* pRefUndoData= new ScRefUndoData(m_pDoc);
    SfxUndoAction* pUndo = new ScUndoPaste(
        &m_xDocShRef, ScRange(0, 1, 1, 2, 1, 1), aMarkData2, pUndoDoc, NULL, IDF_ALL, pRefUndoData, false);
    m_pDoc->CopyFromClip(aRange, aMarkData2, nFlags, NULL, pClipDoc);

    //check values after copying
    rtl::OUString aString;
    m_pDoc->GetValue(1,1,1, aValue);
    m_pDoc->GetFormula(1,1,1, aString);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("copied formula should return 2", aValue, 2, 0.00000001);
    CPPUNIT_ASSERT_MESSAGE("formula string was not copied correctly", aString == aFormulaString);
    m_pDoc->GetValue(0,1,1, aValue);
    CPPUNIT_ASSERT_MESSAGE("copied value should be 1", aValue == 1);

    //chack local range name after copying
    pLocal1 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL1"));
    CPPUNIT_ASSERT_MESSAGE("local range name 1 should be copied", pLocal1);
    ScRange aRangeLocal1;
    pLocal1->IsValidReference(aRangeLocal1);
    CPPUNIT_ASSERT_MESSAGE("local range 1 should still point to Sheet1.A1",aRangeLocal1 == ScRange(0,0,0,0,0,0));
    pLocal2 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL2"));
    CPPUNIT_ASSERT_MESSAGE("local2 should not be copied", pLocal2 == NULL);


    //check undo and redo
    pUndo->Undo();
    m_pDoc->GetValue(1,1,1, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after undo formula should return nothing", aValue, 0, 0.00000001);
    m_pDoc->GetString(2,1,1, aString);
    CPPUNIT_ASSERT_MESSAGE("after undo string should be removed", aString.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("")));

    pUndo->Redo();
    m_pDoc->GetValue(1,1,1, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("formula should return 2 after redo", aValue, 2, 0.00000001);
    m_pDoc->GetString(2,1,1, aString);
    CPPUNIT_ASSERT_MESSAGE("Cell Sheet2.C2 should contain: test", aString.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("test")));
    m_pDoc->GetFormula(1,1,1, aString);
    CPPUNIT_ASSERT_MESSAGE("Formula should be correct again", aString == aFormulaString);

    //clear all variables
    delete pClipDoc;
    delete pUndoDoc;
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testMergedCells()
{
    //test merge and unmerge
    //TODO: an undo/redo test for this would be a good idea
    m_pDoc->InsertTab(0, rtl::OUString("Sheet1"));
    m_pDoc->DoMerge(0, 1, 1, 3, 3, false);
    SCCOL nEndCol = 1;
    SCROW nEndRow = 1;
    m_pDoc->ExtendMerge( 1, 1, nEndCol, nEndRow, 0, false);
    CPPUNIT_ASSERT_MESSAGE("did not merge cells", nEndCol == 3 && nEndRow == 3);
    ScRange aRange(0,2,0,MAXCOL,2,0);
    ScMarkData aMark;
    aMark.SetMarkArea(aRange);
    m_pDoc->SetInTest();
    m_xDocShRef->GetDocFunc().InsertCells(aRange, &aMark, INS_INSROWS, true, true);
    m_pDoc->ExtendMerge(1, 1, nEndCol, nEndRow, 0, false);
    cout << nEndRow << nEndCol;
    CPPUNIT_ASSERT_MESSAGE("did not increase merge area", nEndCol == 3 && nEndRow == 4);
    m_pDoc->DeleteTab(0);
}


void Test::testRenameTable()
{
    //test set rename table
    //TODO: set name1 and name2 and do an undo to check if name 1 is set now
    //TODO: also check if new name for table is same as another table

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    //test case 1 , rename table2 to sheet 1, it should return error
    rtl::OUString nameToSet = "Sheet1";
    ScDocFunc& rDocFunc = m_xDocShRef->GetDocFunc();
    CPPUNIT_ASSERT_MESSAGE("name same as another table is being set", !rDocFunc.RenameTable(1,nameToSet,false,true) );

    //test case 2 , simple rename to check name
    nameToSet = "test1";
    m_xDocShRef->GetDocFunc().RenameTable(0,nameToSet,false,true);
    rtl::OUString nameJustSet;
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_MESSAGE("table not renamed", nameToSet != nameJustSet);

    //test case 3 , rename again
    rtl::OUString anOldName;
    m_pDoc->GetName(0,anOldName);

    nameToSet = "test2";
    rDocFunc.RenameTable(0,nameToSet,false,true);
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_MESSAGE("table not renamed", nameToSet != nameJustSet);

    //test case 4 , check if  undo works
    SfxUndoAction* pUndo = new ScUndoRenameTab(m_xDocShRef,0,anOldName,nameToSet);
    pUndo->Undo();
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_MESSAGE("the correct name is not set after undo", nameJustSet == anOldName);

    pUndo->Redo();
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_MESSAGE("the correct color is not set after redo", nameJustSet == nameToSet);

    m_pDoc->DeleteTab(0);
    m_pDoc->DeleteTab(1);
}



void Test::testSetBackgroundColor()
{
    //test set background color
    //TODO: set color1 and set color2 and do an undo to check if color1 is set now.

    m_pDoc->InsertTab(0, rtl::OUString("Sheet1"));
    Color aColor;

     //test yellow
    aColor=Color(COL_YELLOW);
    m_xDocShRef->GetDocFunc().SetTabBgColor(0,aColor,false, true);
    CPPUNIT_ASSERT_MESSAGE("the correct color is not set", m_pDoc->GetTabBgColor(0)!= aColor);


    Color aOldTabBgColor=m_pDoc->GetTabBgColor(0);
    aColor.SetColor(COL_BLUE);//set BLUE
    m_xDocShRef->GetDocFunc().SetTabBgColor(0,aColor,false, true);
    CPPUNIT_ASSERT_MESSAGE("the correct color is not set the second time", m_pDoc->GetTabBgColor(0)!= aColor);

    //now check for undo
    SfxUndoAction* pUndo = new ScUndoTabColor(m_xDocShRef,0, aOldTabBgColor, aColor);
    pUndo->Undo();
    CPPUNIT_ASSERT_MESSAGE("the correct color is not set after undo", m_pDoc->GetTabBgColor(0)== aOldTabBgColor);
    pUndo->Redo();
    CPPUNIT_ASSERT_MESSAGE("the correct color is not set after undo", m_pDoc->GetTabBgColor(0)== aColor);
    m_pDoc->DeleteTab(0);
}



void Test::testUpdateReference()
{
    //test that formulas are correctly updated during sheet delete
    //TODO: add tests for relative references, updating of named ranges, ...
    rtl::OUString aSheet1("Sheet1");
    rtl::OUString aSheet2("Sheet2");
    rtl::OUString aSheet3("Sheet3");
    rtl::OUString aSheet4("Sheet4");
    m_pDoc->InsertTab(0, aSheet1);
    m_pDoc->InsertTab(1, aSheet2);
    m_pDoc->InsertTab(2, aSheet3);
    m_pDoc->InsertTab(3, aSheet4);

    m_pDoc->SetValue(0,0,2, 1);
    m_pDoc->SetValue(1,0,2, 2);
    m_pDoc->SetValue(1,1,3, 4);
    m_pDoc->SetString(2,0,2, rtl::OUString("=A1+B1"));
    m_pDoc->SetString(2,1,2, rtl::OUString("=Sheet4.B2+A1"));

    double aValue;
    m_pDoc->GetValue(2,0,2, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("formula does not return correct result", aValue, 3, 0.00000001);
    m_pDoc->GetValue(2,1,2, aValue);
    CPPUNIT_ASSERT_MESSAGE("formula does not return correct result", aValue == 5);

    //test deleting both sheets: one is not directly before the sheet, the other one is
    m_pDoc->DeleteTab(0);
    m_pDoc->GetValue(2,0,1, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting first sheet formula does not return correct result", aValue, 3, 0.00000001);
    m_pDoc->GetValue(2,1,1, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting first sheet formula does not return correct result", aValue, 5, 0.00000001);

    m_pDoc->DeleteTab(0);
    m_pDoc->GetValue(2,0,0, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting second sheet formula does not return correct result", aValue, 3, 0.00000001);
    m_pDoc->GetValue(2,1,0, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting second sheet formula does not return correct result", aValue, 5, 0.00000001);

    //test adding two sheets
    m_pDoc->InsertTab(0, aSheet2);
    m_pDoc->GetValue(2,0,1, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting first sheet formula does not return correct result", aValue, 3, 0.00000001);
    m_pDoc->GetValue(2,1,1, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting first sheet formula does not return correct result", aValue, 5, 0.00000001);

    m_pDoc->InsertTab(0, aSheet1);
    m_pDoc->GetValue(2,0,2, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting second sheet formula does not return correct result", aValue, 3, 0.00000001);
    m_pDoc->GetValue(2,1,2, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting second sheet formula does not return correct result", aValue, 5, 0.00000001);

    //test new DeleteTabs/InsertTabs methods
    m_pDoc->DeleteTabs(0, 2);
    m_pDoc->GetValue(2, 0, 0, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting sheets formula does not return correct result", aValue, 3, 0.00000001);
    m_pDoc->GetValue(2, 1, 0, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting sheets formula does not return correct result", aValue, 5, 0.00000001);

    std::vector<rtl::OUString> aSheets;
    aSheets.push_back(aSheet1);
    aSheets.push_back(aSheet2);
    m_pDoc->InsertTabs(0, aSheets, false, true);
    m_pDoc->GetValue(2, 0, 2, aValue);
    rtl::OUString aFormula;
    m_pDoc->GetFormula(2,0,2, aFormula);
    std::cout << "formel: " << rtl::OUStringToOString(aFormula, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    std::cout << std::endl << aValue << std::endl;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting sheets formula does not return correct result", aValue, 3, 0.00000001);
    m_pDoc->GetValue(2, 1, 2, aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting sheets formula does not return correct result", aValue, 5, 0.00000001);

    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

namespace {

bool hasRange(const std::vector<ScTokenRef>& rRefTokens, const ScRange& rRange)
{
    std::vector<ScTokenRef>::const_iterator it = rRefTokens.begin(), itEnd = rRefTokens.end();
    for (; it != itEnd; ++it)
    {
        const ScTokenRef& p = *it;
        if (!ScRefTokenHelper::isRef(p) || ScRefTokenHelper::isExternalRef(p))
            continue;

        switch (p->GetType())
        {
            case formula::svSingleRef:
            {
                ScSingleRefData aData = p->GetSingleRef();
                if (rRange.aStart != rRange.aEnd)
                    break;

                ScAddress aThis(aData.nCol, aData.nRow, aData.nTab);
                if (aThis == rRange.aStart)
                    return true;
            }
            break;
            case formula::svDoubleRef:
            {
                ScComplexRefData aData = p->GetDoubleRef();
                ScRange aThis(aData.Ref1.nCol, aData.Ref1.nRow, aData.Ref1.nTab, aData.Ref2.nCol, aData.Ref2.nRow, aData.Ref2.nTab);
                if (aThis == rRange)
                    return true;
            }
            break;
            default:
                ;
        }
    }
    return false;
}

}

void Test::testJumpToPrecedentsDependents()
{
    // Precedent is another cell that the cell references, while dependent is
    // another cell that references it.
    m_pDoc->InsertTab(0, rtl::OUString("Test"));

    m_pDoc->SetString(2, 0, 0, rtl::OUString("=A1+A2+B3")); // C1
    m_pDoc->SetString(2, 1, 0, rtl::OUString("=A1"));       // C2
    m_pDoc->CalcAll();

    std::vector<ScTokenRef> aRefTokens;
    ScDocFunc& rDocFunc = m_xDocShRef->GetDocFunc();

    {
        // C1's precedent should be A1:A2,B3.
        ScRangeList aRange(ScRange(2, 0, 0));
        rDocFunc.DetectiveCollectAllPreds(aRange, aRefTokens);
        CPPUNIT_ASSERT_MESSAGE("A1:A2 should be a precedent of C1.",
                               hasRange(aRefTokens, ScRange(0, 0, 0, 0, 1, 0)));
        CPPUNIT_ASSERT_MESSAGE("B3 should be a precedent of C1.",
                               hasRange(aRefTokens, ScRange(1, 2, 0)));
    }

    {
        // C2's precedent should be A1 only.
        ScRangeList aRange(ScRange(2, 1, 0));
        rDocFunc.DetectiveCollectAllPreds(aRange, aRefTokens);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("there should only be one reference token.",
                               aRefTokens.size(), static_cast<size_t>(1));
        CPPUNIT_ASSERT_MESSAGE("A1 should be a precedent of C1.",
                               hasRange(aRefTokens, ScRange(0, 0, 0)));
    }

    {
        // A1's dependent should be C1:C2.
        ScRangeList aRange(ScRange(0, 0, 0));
        rDocFunc.DetectiveCollectAllSuccs(aRange, aRefTokens);
        CPPUNIT_ASSERT_MESSAGE("C1:C2 should be the only dependent of A1.",
                               aRefTokens.size() == 1 && hasRange(aRefTokens, ScRange(2, 0, 0, 2, 1, 0)));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testAutoFill()
{
    m_pDoc->InsertTab(0, "test");

    m_pDoc->SetValue(0,0,0,1);

    ScMarkData aMarkData;
    aMarkData.SelectTable(0, true);

    m_pDoc->Fill( 0, 0, 0, 0, NULL, aMarkData, 5);
    for (SCROW i = 0; i< 6; ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<double>(i+1.0), m_pDoc->GetValue(0, i, 0), 0.00000001);

    // check that hidden rows are not affected by autofill
    // set values for hidden rows
    m_pDoc->SetValue(0,1,0,10);
    m_pDoc->SetValue(0,2,0,10);

    m_pDoc->SetRowHidden(1, 2, 0, true);
    m_pDoc->Fill( 0, 0, 0, 0, NULL, aMarkData, 8);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, m_pDoc->GetValue(0,1,0), 1e-08);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, m_pDoc->GetValue(0,2,0), 1e-08);
    for (SCROW i = 3; i< 8; ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<double>(i-1.0), m_pDoc->GetValue(0, i, 0), 0.00000001);

    m_pDoc->Fill( 0, 0, 0, 8, NULL, aMarkData, 5, FILL_TO_RIGHT );
    for (SCCOL i = 0; i < 5; ++i)
    {
        for(SCROW j = 0; j < 8; ++j)
        {
            if (j > 2)
            {
                CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<double>(j-1+i), m_pDoc->GetValue(i, j, 0), 1e-8);
            }
            else if (j == 0)
            {
                CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<double>(i+1), m_pDoc->GetValue(i, 0, 0), 1e-8);
            }
            else if (j == 1 || j== 2)
            {
                if(i == 0)
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, m_pDoc->GetValue(0,j,0), 1e-8);
                else
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, m_pDoc->GetValue(i,j,0), 1e-8);
            }
        }
    }

    // test auto fill user data lists
    m_pDoc->SetString( 0, 100, 0, "January" );
    m_pDoc->Fill( 0, 100, 0, 100, NULL, aMarkData, 2, FILL_TO_BOTTOM, FILL_AUTO );
    rtl::OUString aTestValue = m_pDoc->GetString( 0, 101, 0 );
    CPPUNIT_ASSERT_EQUAL( aTestValue, rtl::OUString("February") );
    aTestValue = m_pDoc->GetString( 0, 102, 0 );
    CPPUNIT_ASSERT_EQUAL( aTestValue, rtl::OUString("March") );

    // test that two same user data list entries will not result in incremental fill
    m_pDoc->SetString( 0, 101, 0, "January" );
    m_pDoc->Fill( 0, 100, 0, 101, NULL, aMarkData, 2, FILL_TO_BOTTOM, FILL_AUTO );
    for ( SCROW i = 102; i <= 103; ++i )
    {
        aTestValue = m_pDoc->GetString( 0, i, 0 );
        CPPUNIT_ASSERT_EQUAL( aTestValue, rtl::OUString("January") );
    }
    m_pDoc->DeleteTab(0);
}

void Test::testCopyPasteFormulas()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetString(0,0,0, "=COLUMN($A$1)");
    m_pDoc->SetString(0,1,0, "=$A$1+B2" );
    m_pDoc->SetString(0,2,0, "=$Sheet2.A1");
    m_pDoc->SetString(0,3,0, "=$Sheet2.$A$1");
    m_pDoc->SetString(0,4,0, "=$Sheet2.A$1");

    // to prevent ScEditableTester in ScDocFunc::MoveBlock
    m_pDoc->SetInTest();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(0,0,0), 1.0, 1e-08);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(0,1,0), 1.0, 1e-08);
    ScDocFunc& rDocFunc = m_xDocShRef->GetDocFunc();
    bool bMoveDone = rDocFunc.MoveBlock(ScRange(0,0,0,0,4,0), ScAddress( 10, 10, 0), false, false, false, true);

    // check that moving was succesful, mainly for editable tester
    CPPUNIT_ASSERT(bMoveDone);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(10,10,0), 1.0, 1e-8);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(10,11,0), 1.0, 1e-8);
    rtl::OUString aFormula;
    m_pDoc->GetFormula(10,10,0, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, rtl::OUString("=COLUMN($A$1)"));
    m_pDoc->GetFormula(10,11,0, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, rtl::OUString("=$A$1+L12"));
    m_pDoc->GetFormula(10,12,0, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, rtl::OUString("=$Sheet2.K11"));
    m_pDoc->GetFormula(10,13,0, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, rtl::OUString("=$Sheet2.$A$1"));
    m_pDoc->GetFormula(10,14,0, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, rtl::OUString("=$Sheet2.K$1"));
}

void Test::testCopyPasteFormulasExternalDoc()
{
    rtl::OUString aDocName("file:///source.fake");
    SfxMedium* pMedium = new SfxMedium(aDocName, STREAM_STD_READWRITE);
    m_xDocShRef->DoInitNew(pMedium);
    m_pDoc = m_xDocShRef->GetDocument();

    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString aExtDocName("file:///extdata.fake");
    OUString aExtSh1Name("ExtSheet1");
    OUString aExtSh2Name("ExtSheet2");
    SfxMedium* pMed = new SfxMedium(aExtDocName, STREAM_STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != NULL);

    ScDocument* pExtDoc = xExtDocSh->GetDocument();
    pExtDoc->InsertTab(0, aExtSh1Name);
    pExtDoc->InsertTab(1, aExtSh2Name);

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetString(0,0,0, "=COLUMN($A$1)");
    m_pDoc->SetString(0,1,0, "=$A$1+B2" );
    m_pDoc->SetString(0,2,0, "=$Sheet2.A1");
    m_pDoc->SetString(0,3,0, "=$Sheet2.$A$1");
    m_pDoc->SetString(0,4,0, "=$Sheet2.A$1");
    m_pDoc->SetString(0,5,0, "=$Sheet1.$A$1");

    ScRange aRange(0,0,0,0,5,0);
    ScClipParam aClipParam(aRange, false);
    ScMarkData aMark;
    aMark.SetMarkArea(aRange);
    ScDocument* pClipDoc = new ScDocument(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, pClipDoc, &aMark);

    sal_uInt16 nFlags = IDF_ALL;
    aRange = ScRange(1,1,1,1,6,1);
    ScMarkData aMarkData2;
    aMarkData2.SetMarkArea(aRange);
    pExtDoc->CopyFromClip(aRange, aMarkData2, nFlags, NULL, pClipDoc);

    rtl::OUString aFormula;
    pExtDoc->GetFormula(1,1,1, aFormula);
    //adjust absolute refs pointing to the copy area
    CPPUNIT_ASSERT_EQUAL(aFormula, rtl::OUString("=COLUMN($B$2)"));
    pExtDoc->GetFormula(1,2,1, aFormula);
    //adjust absolute refs and keep relative refs
    CPPUNIT_ASSERT_EQUAL(aFormula, rtl::OUString("=$B$2+C3"));
    pExtDoc->GetFormula(1,3,1, aFormula);
    // make absolute sheet refs external refs
    CPPUNIT_ASSERT_EQUAL(aFormula, rtl::OUString("='file:///source.fake'#$Sheet2.B2"));
    pExtDoc->GetFormula(1,4,1, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, rtl::OUString("='file:///source.fake'#$Sheet2.$A$1"));
    pExtDoc->GetFormula(1,5,1, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, rtl::OUString("='file:///source.fake'#$Sheet2.B$1"));
    pExtDoc->GetFormula(1,6,1, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, rtl::OUString("=$ExtSheet2.$B$2"));
}

void Test::testFindAreaPosRowDown()
{
    const char* aData[][2] = {
        { "", "1" },
        { "1", "" },
        { "1", "1" },
        { "", "1" },
        { "1", "1" },
        { "1", "" },
        { "1", "1" }, };

    ScDocument* pDoc = m_xDocShRef->GetDocument();
    rtl::OUString aTabName1("test1");
    pDoc->InsertTab(0, aTabName1);
    clearRange( pDoc, ScRange(0, 0, 0, 1, SAL_N_ELEMENTS(aData), 0));
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    pDoc->SetRowHidden(4,4,0,true);
    bool bHidden = pDoc->RowHidden(4,0);
    CPPUNIT_ASSERT(bHidden);

    SCCOL nCol = 0;
    SCROW nRow = 0;
    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), nCol);

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), nCol);

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(5), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), nCol);

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), nCol);

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(MAXROW), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), nCol);

    nCol = 1;
    nRow = 2;

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCol);

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCol);

    pDoc->DeleteTab(0);
}

void Test::testFindAreaPosColRight()
{
    const char* aData[][7] = {
        { "", "1", "1", "", "1", "1", "1" },
        { "", "", "1", "1", "1", "", "1" }, };

    ScDocument* pDoc = m_xDocShRef->GetDocument();
    rtl::OUString aTabName1("test1");
    pDoc->InsertTab(0, aTabName1);
    clearRange( pDoc, ScRange(0, 0, 0, 7, SAL_N_ELEMENTS(aData), 0));
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    pDoc->SetColHidden(4,4,0,true);
    bool bHidden = pDoc->ColHidden(4,0);
    CPPUNIT_ASSERT(bHidden);

    SCCOL nCol = 0;
    SCROW nRow = 0;
    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCol);

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(2), nCol);

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(5), nCol);

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(6), nCol);

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(MAXCOL), nCol);

    nCol = 2;
    nRow = 1;

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(3), nCol);

    pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(6), nCol);

    pDoc->DeleteTab(0);
}

// regression test fo fdo#53814, sorting doens't work as expected
// if cells in the sort are referenced by formulas
void Test::testSortWithFormulaRefs()
{
    ScDocument* pDoc = m_xDocShRef->GetDocument();
    rtl::OUString aTabName1("List1");
    rtl::OUString aTabName2("List2");
    pDoc->InsertTab(0, aTabName1);
    pDoc->InsertTab(1, aTabName2);

    const char* aFormulaData[6] = {
        "=IF($List1.A2<>\"\",$List1.A2,\"\")",
        "=IF($List1.A3<>\"\",$List1.A3,\"\")",
        "=IF($List1.A4<>\"\",$List1.A4,\"\")",
        "=IF($List1.A5<>\"\",$List1.A5,\"\")",
        "=IF($List1.A6<>\"\",$List1.A6,\"\")",
        "=IF($List1.A7<>\"\",$List1.A7,\"\")",
    };

    const char* aTextData[4] = {
        "bob",
        "tim",
        "brian",
        "larry",
    };

    const char* aResults[ 6 ] = {
        "bob",
        "brian",
        "larry",
        "tim",
        "",
        "",
    };
    // insert data to sort
    SCROW nStart = 1, nEnd = 4;
    for ( SCROW i = nStart; i <= nEnd; ++i )
        pDoc->SetString( 0, i, 0, rtl::OUString::createFromAscii(aTextData[i-1]) );
    // insert forumulas
    nStart = 0;
    nEnd = SAL_N_ELEMENTS(aFormulaData);
    for ( SCROW i = nStart; i < nEnd; ++i )
        pDoc->SetString( 0, i, 1, rtl::OUString::createFromAscii(aFormulaData[i]) );

    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 0;
    aSortData.nRow1 = 1;
    aSortData.nRow2 = 7;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;

    pDoc->Sort(0, aSortData, false, NULL);

    nEnd = SAL_N_ELEMENTS( aResults );
    for ( SCROW i = nStart; i < nEnd; ++i )
    {
        rtl::OUString sResult;
        pDoc->GetString( 0, i + 1, 0, sResult );
        CPPUNIT_ASSERT_EQUAL( rtl::OUString::createFromAscii( aResults[ i ] ), sResult );
    }
    pDoc->DeleteTab(0);
    pDoc->DeleteTab(1);
}

void Test::testSort()
{
    ScDocument* pDoc = m_xDocShRef->GetDocument();
    rtl::OUString aTabName1("test1");
    pDoc->InsertTab(0, aTabName1);

    const char* aData[][2] = {
        { "2", "4" },
        { "4", "1" },
        { "1", "2" }
    };

    clearRange( pDoc, ScRange(0, 0, 0, 1, SAL_N_ELEMENTS(aData), 0));
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    rtl::OUString aHello("Hello");
    rtl::OUString aJimBob("Jim Bob");
    ScAddress rAddr(1, 1, 0);
    ScPostIt* pNote = m_pDoc->GetNotes(rAddr.Tab())->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, aHello);
    pNote->SetAuthor(aJimBob);

    ScSortParam aSortData;
    aSortData.nCol1 = 1;
    aSortData.nCol2 = 1;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 2;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 1;

    pDoc->Sort(0, aSortData, false, NULL);
    double nVal = pDoc->GetValue(1,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 1.0, 1e-8);

    // check that note is also moved
    pNote = m_pDoc->GetNotes(0)->findByAddress( 1, 0 );
    CPPUNIT_ASSERT(pNote);

    pDoc->DeleteTab(0);
}

void Test::testDeleteRow()
{
    ScDocument* pDoc = m_xDocShRef->GetDocument();
    rtl::OUString aSheet1("Sheet1");
    pDoc->InsertTab(0, aSheet1);

    rtl::OUString aHello("Hello");
    rtl::OUString aJimBob("Jim Bob");
    ScAddress rAddr(1, 1, 0);
    ScPostIt* pNote = m_pDoc->GetNotes(rAddr.Tab())->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, aHello);
    pNote->SetAuthor(aJimBob);

    pDoc->DeleteRow(0, 0, MAXCOL, 0, 1, 1);

    CPPUNIT_ASSERT(m_pDoc->GetNotes(0)->empty());
    pDoc->DeleteTab(0);
}

void Test::testDeleteCol()
{
    ScDocument* pDoc = m_xDocShRef->GetDocument();
    rtl::OUString aSheet1("Sheet1");
    pDoc->InsertTab(0, aSheet1);

    rtl::OUString aHello("Hello");
    rtl::OUString aJimBob("Jim Bob");
    ScAddress rAddr(1, 1, 0);
    ScPostIt* pNote = m_pDoc->GetNotes(rAddr.Tab())->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, aHello);
    pNote->SetAuthor(aJimBob);

    pDoc->DeleteCol(0, 0, MAXROW, 0, 1, 1);

    CPPUNIT_ASSERT(m_pDoc->GetNotes(0)->empty());
    pDoc->DeleteTab(0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
