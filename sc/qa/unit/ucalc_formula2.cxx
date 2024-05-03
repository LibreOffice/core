/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"
#include <clipparam.hxx>
#include <scopetools.hxx>
#include <formulacell.hxx>
#include <docfunc.hxx>
#include <tokenstringcontext.hxx>
#include <dbdata.hxx>
#include <scmatrix.hxx>
#include <docoptio.hxx>
#include <externalrefmgr.hxx>
#include <undomanager.hxx>
#include <broadcast.hxx>
#include <kahan.hxx>

#include <svl/broadcast.hxx>
#include <sfx2/docfile.hxx>

#include <memory>
#include <functional>
#include <set>
#include <algorithm>
#include <vector>

using namespace formula;
using ::std::vector;
using ::std::cerr;
using ::std::endl;

namespace
{
ScRange getCachedRange(const ScExternalRefCache::TableTypeRef& pCacheTab)
{
    ScRange aRange;

    vector<SCROW> aRows;
    pCacheTab->getAllRows(aRows);
    bool bFirst = true;
    for (const SCROW nRow : aRows)
    {
        vector<SCCOL> aCols;
        pCacheTab->getAllCols(nRow, aCols);
        for (const SCCOL nCol : aCols)
        {
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

struct StrStrCheck
{
    const char* pVal;
    const char* pRes;
};

class ColumnTest
{
    ScDocument* m_pDoc;

    const SCROW m_nTotalRows;
    const SCROW m_nStart1;
    const SCROW m_nEnd1;
    const SCROW m_nStart2;
    const SCROW m_nEnd2;

public:
    ColumnTest(ScDocument* pDoc, SCROW nTotalRows, SCROW nStart1, SCROW nEnd1, SCROW nStart2,
               SCROW nEnd2)
        : m_pDoc(pDoc)
        , m_nTotalRows(nTotalRows)
        , m_nStart1(nStart1)
        , m_nEnd1(nEnd1)
        , m_nStart2(nStart2)
        , m_nEnd2(nEnd2)
    {
    }

    void operator()(SCCOL nColumn, const OUString& rFormula,
                    std::function<double(SCROW)> const& lExpected) const
    {
        ScDocument aClipDoc(SCDOCMODE_CLIP);
        ScMarkData aMark(m_pDoc->GetSheetLimits());

        ScAddress aPos(nColumn, m_nStart1, 0);
        m_pDoc->SetString(aPos, rFormula);
        ASSERT_DOUBLES_EQUAL(lExpected(m_nStart1), m_pDoc->GetValue(aPos));

        // Copy formula cell to clipboard.
        ScClipParam aClipParam(aPos, false);
        aMark.SetMarkArea(aPos);
        m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

        // Paste it to first range.
        InsertDeleteFlags nFlags = InsertDeleteFlags::CONTENTS;
        ScRange aDestRange(nColumn, m_nStart1, 0, nColumn, m_nEnd1, 0);
        aMark.SetMarkArea(aDestRange);
        m_pDoc->CopyFromClip(aDestRange, aMark, nFlags, nullptr, &aClipDoc);

        // Paste it second range.
        aDestRange = ScRange(nColumn, m_nStart2, 0, nColumn, m_nEnd2, 0);
        aMark.SetMarkArea(aDestRange);
        m_pDoc->CopyFromClip(aDestRange, aMark, nFlags, nullptr, &aClipDoc);

        // Check the formula results for passed column.
        for (SCROW i = 0; i < m_nTotalRows; ++i)
        {
            if (!((m_nStart1 <= i && i <= m_nEnd1) || (m_nStart2 <= i && i <= m_nEnd2)))
                continue;
            double fExpected = lExpected(i);
            ASSERT_DOUBLES_EQUAL(fExpected, m_pDoc->GetValue(ScAddress(nColumn, i, 0)));
        }
    }
};

} //namespace

class TestFormula2 : public ScUcalcTestBase
{
protected:
    template <size_t DataSize, size_t FormulaSize, int Type>
    void runTestMATCH(ScDocument* pDoc, const char* aData[DataSize],
                      const StrStrCheck aChecks[FormulaSize]);
    template <size_t DataSize, size_t FormulaSize, int Type>
    void runTestHorizontalMATCH(ScDocument* pDoc, const char* aData[DataSize],
                                const StrStrCheck aChecks[FormulaSize]);

    void testExtRefFuncT(ScDocument* pDoc, ScDocument& rExtDoc);
    void testExtRefFuncOFFSET(ScDocument* pDoc, ScDocument& rExtDoc);
    void testExtRefFuncVLOOKUP(ScDocument* pDoc, ScDocument& rExtDoc);
    void testExtRefConcat(ScDocument* pDoc, ScDocument& rExtDoc);
};

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncIF)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Formula");

    m_pDoc->SetString(ScAddress(0, 0, 0), "=IF(B1=2;\"two\";\"not two\")");
    CPPUNIT_ASSERT_EQUAL(OUString("not two"), m_pDoc->GetString(ScAddress(0, 0, 0)));
    m_pDoc->SetValue(ScAddress(1, 0, 0), 2.0);
    CPPUNIT_ASSERT_EQUAL(OUString("two"), m_pDoc->GetString(ScAddress(0, 0, 0)));
    m_pDoc->SetValue(ScAddress(1, 0, 0), 3.0);
    CPPUNIT_ASSERT_EQUAL(OUString("not two"), m_pDoc->GetString(ScAddress(0, 0, 0)));

    // Test nested IF in array/matrix if the nested IF condition is a scalar.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 2, 1, 2, aMark, "=IF({1;0};IF(1;23);42)");
    // Results must be 23 and 42.
    CPPUNIT_ASSERT_EQUAL(23.0, m_pDoc->GetValue(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(42.0, m_pDoc->GetValue(ScAddress(1, 2, 0)));

    // Test nested IF in array/matrix if nested IF conditions are range
    // references, data in A5:C8, matrix formula in D4 so there is no
    // implicit intersection between formula and ranges.
    {
        std::vector<std::vector<const char*>> aData
            = { { "1", "1", "16" }, { "0", "1", "32" }, { "1", "0", "64" }, { "0", "0", "128" } };
        ScAddress aPos(0, 4, 0);
        ScRange aRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);
    }
    m_pDoc->InsertMatrixFormula(3, 3, 3, 3, aMark, "=SUM(IF(A5:A8;IF(B5:B8;C5:C8;0);0))");
    // Result must be 16, only the first row matches all criteria.
    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(3, 3, 0)));

    // A11:B11
    // Test nested IF in array/matrix if the nested IF has no Else path.
    m_pDoc->InsertMatrixFormula(0, 10, 1, 10, aMark, "=IF(IF({1;0};12);34;56)");
    // Results must be 34 and 56.
    CPPUNIT_ASSERT_EQUAL(34.0, m_pDoc->GetValue(ScAddress(0, 10, 0)));
    CPPUNIT_ASSERT_EQUAL(56.0, m_pDoc->GetValue(ScAddress(1, 10, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncCHOOSE)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Formula");

    m_pDoc->SetString(ScAddress(0, 0, 0), "=CHOOSE(B1;\"one\";\"two\";\"three\")");
    FormulaError nError = m_pDoc->GetErrCode(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT_MESSAGE("Formula result should be an error since B1 is still empty.",
                           nError != FormulaError::NONE);
    m_pDoc->SetValue(ScAddress(1, 0, 0), 1.0);
    CPPUNIT_ASSERT_EQUAL(OUString("one"), m_pDoc->GetString(ScAddress(0, 0, 0)));
    m_pDoc->SetValue(ScAddress(1, 0, 0), 2.0);
    CPPUNIT_ASSERT_EQUAL(OUString("two"), m_pDoc->GetString(ScAddress(0, 0, 0)));
    m_pDoc->SetValue(ScAddress(1, 0, 0), 3.0);
    CPPUNIT_ASSERT_EQUAL(OUString("three"), m_pDoc->GetString(ScAddress(0, 0, 0)));
    m_pDoc->SetValue(ScAddress(1, 0, 0), 4.0);
    nError = m_pDoc->GetErrCode(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT_MESSAGE("Formula result should be an error due to out-of-bound input..",
                           nError != FormulaError::NONE);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncIFERROR)
{
    // IFERROR/IFNA (fdo#56124)

    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "foo"));

    // Empty A1:A39 first.
    clearRange(m_pDoc, ScRange(0, 0, 0, 0, 40, 0));

    // Raw data (rows 1 through 12)
    const char* aData[] = { "1",     "e",   "=SQRT(4)", "=SQRT(-2)", "=A4",  "=1/0",
                            "=NA()", "bar", "4",        "gee",       "=1/0", "23" };

    SCROW nRows = SAL_N_ELEMENTS(aData);
    for (SCROW i = 0; i < nRows; ++i)
        m_pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i]));

    printRange(m_pDoc, ScRange(0, 0, 0, 0, nRows - 1, 0), "data range for IFERROR/IFNA");

    // formulas and results
    static const struct
    {
        const char* pFormula;
        const char* pResult;
    } aChecks[] = {
        { "=IFERROR(A1;9)", "1" },
        { "=IFERROR(A2;9)", "e" },
        { "=IFERROR(A3;9)", "2" },
        { "=IFERROR(A4;-7)", "-7" },
        { "=IFERROR(A5;-7)", "-7" },
        { "=IFERROR(A6;-7)", "-7" },
        { "=IFERROR(A7;-7)", "-7" },
        { "=IFNA(A6;9)", "#DIV/0!" },
        { "=IFNA(A7;-7)", "-7" },
        { "=IFNA(VLOOKUP(\"4\";A8:A10;1;0);-2)", "4" },
        { "=IFNA(VLOOKUP(\"fop\";A8:A10;1;0);-2)", "-2" },
        { "{=IFERROR(3*A11:A12;1998)}[0]",
          "1998" }, // um... this is not the correct way to insert a
        { "{=IFERROR(3*A11:A12;1998)}[1]", "69" } // matrix formula, just a place holder, see below
    };

    nRows = SAL_N_ELEMENTS(aChecks);
    for (SCROW i = 0; i < nRows - 2; ++i)
    {
        SCROW nRow = 20 + i;
        m_pDoc->SetString(0, nRow, 0, OUString::createFromAscii(aChecks[i].pFormula));
    }

    // Create a matrix range in last two rows of the range above, actual data
    // of the placeholders.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 20 + nRows - 2, 0, 20 + nRows - 1, aMark,
                                "=IFERROR(3*A11:A12;1998)");

    m_pDoc->CalcAll();

    for (SCROW i = 0; i < nRows; ++i)
    {
        SCROW nRow = 20 + i;
        OUString aResult = m_pDoc->GetString(0, nRow, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aChecks[i].pFormula,
                                     OUString::createFromAscii(aChecks[i].pResult), aResult);
    }

    const SCCOL nCols = 3;
    std::vector<std::vector<const char*>> aData2
        = { { "1", "2", "3" }, { "4", "=1/0", "6" }, { "7", "8", "9" } };
    const char* aCheck2[][nCols] = { { "1", "2", "3" }, { "4", "Error", "6" }, { "7", "8", "9" } };

    // Data in C1:E3
    ScAddress aPos(2, 0, 0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData2);
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    // Array formula in F4:H6
    const SCROW nElems2 = SAL_N_ELEMENTS(aCheck2);
    const SCCOL nStartCol = aPos.Col() + nCols;
    const SCROW nStartRow = aPos.Row() + nElems2;
    m_pDoc->InsertMatrixFormula(nStartCol, nStartRow, nStartCol + nCols - 1,
                                nStartRow + nElems2 - 1, aMark, "=IFERROR(C1:E3;\"Error\")");

    m_pDoc->CalcAll();

    for (SCCOL nCol = nStartCol; nCol < nStartCol + nCols; ++nCol)
    {
        for (SCROW nRow = nStartRow; nRow < nStartRow + nElems2; ++nRow)
        {
            OUString aResult = m_pDoc->GetString(nCol, nRow, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "IFERROR array result",
                OUString::createFromAscii(aCheck2[nRow - nStartRow][nCol - nStartCol]), aResult);
        }
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncSHEET)
{
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(SC_TAB_APPEND, "test1"));

    m_pDoc->SetString(0, 0, 0, "=SHEETS()");
    m_pDoc->CalcFormulaTree(false, false);
    double original = m_pDoc->GetValue(0, 0, 0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "result of SHEETS() should equal the number of sheets, but doesn't.",
        static_cast<SCTAB>(original), m_pDoc->GetTableCount());

    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(SC_TAB_APPEND, "test2"));

    double modified = m_pDoc->GetValue(0, 0, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("result of SHEETS() did not get updated after sheet insertion.",
                                 1.0, modified - original);

    SCTAB nTabCount = m_pDoc->GetTableCount();
    m_pDoc->DeleteTab(--nTabCount);

    modified = m_pDoc->GetValue(0, 0, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("result of SHEETS() did not get updated after sheet removal.", 0.0,
                                 modified - original);

    m_pDoc->DeleteTab(--nTabCount);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncNOW)
{
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "foo"));

    double val = 1;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->SetString(0, 1, 0, "=IF(A1>0;NOW();0");
    double now1 = m_pDoc->GetValue(0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Value of NOW() should be positive.", now1 > 0.0);

    val = 0;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->CalcFormulaTree(false, false);
    double zero = m_pDoc->GetValue(0, 1, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Result should equal the 3rd parameter of IF, which is zero.", 0.0,
                                 zero);

    val = 1;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->CalcFormulaTree(false, false);
    double now2 = m_pDoc->GetValue(0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Result should be the value of NOW() again.", (now2 - now1) >= 0.0);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncNUMBERVALUE)
{
    // NUMBERVALUE fdo#57180

    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "foo"));

    // Empty A1:A39 first.
    clearRange(m_pDoc, ScRange(0, 0, 0, 0, 40, 0));

    // Raw data (rows 1 through 6)
    const char* aData[]
        = { "1ag9a9b9", "1ag34 5g g6  78b9%%", "1 234d56E-2", "d4", "54.4", "1a2b3e1%" };

    SCROW nRows = SAL_N_ELEMENTS(aData);
    for (SCROW i = 0; i < nRows; ++i)
        m_pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i]));

    printRange(m_pDoc, ScRange(0, 0, 0, 0, nRows - 1, 0), "data range for NUMBERVALUE");

    // formulas and results
    static const struct
    {
        const char* pFormula;
        const char* pResult;
    } aChecks[] = { { "=NUMBERVALUE(A1;\"b\";\"ag\")", "199.9" },
                    { "=NUMBERVALUE(A2;\"b\";\"ag\")", "134.56789" },
                    { "=NUMBERVALUE(A2;\"b\";\"g\")", "#VALUE!" },
                    { "=NUMBERVALUE(A3;\"d\")", "12.3456" },
                    { "=NUMBERVALUE(A4;\"d\";\"foo\")", "0.4" },
                    { "=NUMBERVALUE(A4;)", "Err:502" },
                    { "=NUMBERVALUE(A5;)", "Err:502" },
                    { "=NUMBERVALUE(A6;\"b\";\"a\")", "1.23" } };

    nRows = SAL_N_ELEMENTS(aChecks);
    for (SCROW i = 0; i < nRows; ++i)
    {
        SCROW nRow = 20 + i;
        m_pDoc->SetString(0, nRow, 0, OUString::createFromAscii(aChecks[i].pFormula));
    }
    m_pDoc->CalcAll();

    for (SCROW i = 0; i < nRows; ++i)
    {
        SCROW nRow = 20 + i;
        OUString aResult = m_pDoc->GetString(0, nRow, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aChecks[i].pFormula,
                                     OUString::createFromAscii(aChecks[i].pResult), aResult);
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncLEN)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Formula");

    // Leave A1:A3 empty, and insert an array of LEN in B1:B3 that references
    // these empty cells.

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(1, 0, 1, 2, aMark, "=LEN(A1:A3)");

    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 0, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula should be a matrix origin.", ScMatrixMode::Formula,
                                 pFC->GetMatrixFlag());

    // This should be a 1x3 matrix.
    SCCOL nCols = -1;
    SCROW nRows = -1;
    pFC->GetMatColsRows(nCols, nRows);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCols);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), nRows);

    // LEN value should be 0 for an empty cell.
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1, 2, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncLOOKUP)
{
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    m_pDoc->InsertTab(0, "Test");

    // Raw data
    const char* aData[][2] = {
        { "=CONCATENATE(\"A\")", "1" },
        { "=CONCATENATE(\"B\")", "2" },
        { "=CONCATENATE(\"C\")", "3" },
        { nullptr, nullptr } // terminator
    };

    // Insert raw data into A1:B3.
    for (SCROW i = 0; aData[i][0]; ++i)
    {
        m_pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i][0]));
        m_pDoc->SetString(1, i, 0, OUString::createFromAscii(aData[i][1]));
    }

    const char* aData2[][2] = {
        { "A", "=LOOKUP(RC[-1];R1C1:R3C1;R1C2:R3C2)" },
        { "B", "=LOOKUP(RC[-1];R1C1:R3C1;R1C2:R3C2)" },
        { "C", "=LOOKUP(RC[-1];R1C1:R3C1;R1C2:R3C2)" },
        { nullptr, nullptr } // terminator
    };

    // Insert check formulas into A5:B7.
    for (SCROW i = 0; aData2[i][0]; ++i)
    {
        m_pDoc->SetString(0, i + 4, 0, OUString::createFromAscii(aData2[i][0]));
        m_pDoc->SetString(1, i + 4, 0, OUString::createFromAscii(aData2[i][1]));
    }

    printRange(m_pDoc, ScRange(0, 4, 0, 1, 6, 0), "Data range for LOOKUP.");

    // Values for B5:B7 should be 1, 2, and 3.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula should not have an error code.", 0,
                                 static_cast<int>(m_pDoc->GetErrCode(ScAddress(1, 4, 0))));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula should not have an error code.", 0,
                                 static_cast<int>(m_pDoc->GetErrCode(ScAddress(1, 5, 0))));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula should not have an error code.", 0,
                                 static_cast<int>(m_pDoc->GetErrCode(ScAddress(1, 6, 0))));

    ASSERT_DOUBLES_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1, 4, 0)));
    ASSERT_DOUBLES_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1, 5, 0)));
    ASSERT_DOUBLES_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1, 6, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncLOOKUParrayWithError)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, "Test");

    std::vector<std::vector<const char*>> aData = { { "x", "y", "z" }, { "a", "b", "c" } };
    insertRangeData(m_pDoc, ScAddress(2, 1, 0), aData); // C2:E3
    m_pDoc->SetString(0, 0, 0, "=LOOKUP(2;1/(C2:E2<>\"\");C3:E3)"); // A1

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should find match for last column.", OUString("c"),
                                 m_pDoc->GetString(0, 0, 0));
    m_pDoc->SetString(4, 1, 0, ""); // E2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should find match for second last column.", OUString("b"),
                                 m_pDoc->GetString(0, 0, 0));

    m_pDoc->SetString(6, 1, 0, "one"); // G2
    m_pDoc->SetString(6, 5, 0, "two"); // G6
    // Creates an interim array {1,#DIV/0!,#DIV/0!,#DIV/0!,1,#DIV/0!,#DIV/0!,#DIV/0!}
    m_pDoc->SetString(7, 8, 0, "=LOOKUP(2;1/(NOT(ISBLANK(G2:G9)));G2:G9)"); // H9
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should find match for last row.", OUString("two"),
                                 m_pDoc->GetString(7, 8, 0));

    // Lookup on empty range.
    m_pDoc->SetString(9, 8, 0, "=LOOKUP(2;1/(NOT(ISBLANK(I2:I9)));I2:I9)"); // J9
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should find no match.", OUString("#N/A"),
                                 m_pDoc->GetString(9, 8, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testTdf141146)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, "Test1");
    m_pDoc->InsertTab(1, "Test2");

    std::vector<std::vector<const char*>> aData
        = { { "k1", "value1" }, { "k2", "value2" }, { "k3", "value3" } };

    insertRangeData(m_pDoc, ScAddress(0, 1, 1), aData); // A2:B4
    m_pDoc->SetString(4, 0, 1, "k2"); // E1

    m_pDoc->SetString(4, 1, 1, "=LOOKUP(1;1/(A$2:A$4=E$1);1)");
    m_pDoc->SetString(4, 2, 1, "=LOOKUP(E1;A$2:A$4;B2:B4)");
    m_pDoc->SetString(4, 3, 1, "=LOOKUP(1;1/(A$2:A$4=E$1);B2:B4)");

    // Without the fix in place, this test would have failed with
    // - Expected: #N/A
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("#N/A"), m_pDoc->GetString(4, 1, 1));
    CPPUNIT_ASSERT_EQUAL(OUString("value2"), m_pDoc->GetString(4, 2, 1));
    CPPUNIT_ASSERT_EQUAL(OUString("value2"), m_pDoc->GetString(4, 3, 1));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncVLOOKUP)
{
    // VLOOKUP

    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "foo"));

    // Clear A1:F40.
    clearRange(m_pDoc, ScRange(0, 0, 0, 5, 39, 0));

    // Raw data
    const char* aData[][2] = {
        { "Key", "Val" }, { "10", "3" }, { "20", "4" },       { "30", "5" },
        { "40", "6" },    { "50", "7" }, { "60", "8" },       { "70", "9" },
        { "B", "10" },    { "B", "11" }, { "C", "12" },       { "D", "13" },
        { "E", "14" },    { "F", "15" }, { nullptr, nullptr } // terminator
    };

    // Insert raw data into A1:B14.
    for (SCROW i = 0; aData[i][0]; ++i)
    {
        m_pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i][0]));
        m_pDoc->SetString(1, i, 0, OUString::createFromAscii(aData[i][1]));
    }

    printRange(m_pDoc, ScRange(0, 0, 0, 1, 13, 0), "raw data for VLOOKUP");

    // Formula data
    static const struct
    {
        const char* pLookup;
        const char* pFormula;
        const char* pRes;
    } aChecks[] = { { "Lookup", "Formula", nullptr },
                    { "12", "=VLOOKUP(D2;A2:B14;2;1)", "3" },
                    { "29", "=VLOOKUP(D3;A2:B14;2;1)", "4" },
                    { "31", "=VLOOKUP(D4;A2:B14;2;1)", "5" },
                    { "45", "=VLOOKUP(D5;A2:B14;2;1)", "6" },
                    { "56", "=VLOOKUP(D6;A2:B14;2;1)", "7" },
                    { "65", "=VLOOKUP(D7;A2:B14;2;1)", "8" },
                    { "78", "=VLOOKUP(D8;A2:B14;2;1)", "9" },
                    { "Andy", "=VLOOKUP(D9;A2:B14;2;1)", "#N/A" },
                    { "Bruce", "=VLOOKUP(D10;A2:B14;2;1)", "11" },
                    { "Charlie", "=VLOOKUP(D11;A2:B14;2;1)", "12" },
                    { "David", "=VLOOKUP(D12;A2:B14;2;1)", "13" },
                    { "Edward", "=VLOOKUP(D13;A2:B14;2;1)", "14" },
                    { "Frank", "=VLOOKUP(D14;A2:B14;2;1)", "15" },
                    { "Henry", "=VLOOKUP(D15;A2:B14;2;1)", "15" },
                    { "100", "=VLOOKUP(D16;A2:B14;2;1)", "9" },
                    { "1000", "=VLOOKUP(D17;A2:B14;2;1)", "9" },
                    { "Zena", "=VLOOKUP(D18;A2:B14;2;1)", "15" } };

    // Insert formula data into D1:E18.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        m_pDoc->SetString(3, i, 0, OUString::createFromAscii(aChecks[i].pLookup));
        m_pDoc->SetString(4, i, 0, OUString::createFromAscii(aChecks[i].pFormula));
    }
    m_pDoc->CalcAll();
    printRange(m_pDoc, ScRange(3, 0, 0, 4, 17, 0), "formula data for VLOOKUP");

    // Verify results.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        if (i == 0)
            // Skip the header row.
            continue;

        OUString aRes = m_pDoc->GetString(4, i, 0);
        bool bGood = aRes.equalsAscii(aChecks[i].pRes);
        if (!bGood)
        {
            cerr << "row " << (i + 1) << ": lookup value='" << aChecks[i].pLookup << "'  expected='"
                 << aChecks[i].pRes << "' actual='" << aRes << "'" << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for VLOOKUP", false);
        }
    }

    // Clear the sheet and start over.
    clearSheet(m_pDoc, 0);

    // Lookup on sorted data interspersed with empty cells.

    // A1:B8 is the search range.
    m_pDoc->SetValue(ScAddress(0, 2, 0), 1.0);
    m_pDoc->SetValue(ScAddress(0, 4, 0), 2.0);
    m_pDoc->SetValue(ScAddress(0, 7, 0), 4.0);
    m_pDoc->SetString(ScAddress(1, 2, 0), "One");
    m_pDoc->SetString(ScAddress(1, 4, 0), "Two");
    m_pDoc->SetString(ScAddress(1, 7, 0), "Four");

    // D1:D5 contain match values.
    m_pDoc->SetValue(ScAddress(3, 0, 0), 1.0);
    m_pDoc->SetValue(ScAddress(3, 1, 0), 2.0);
    m_pDoc->SetValue(ScAddress(3, 2, 0), 3.0);
    m_pDoc->SetValue(ScAddress(3, 3, 0), 4.0);
    m_pDoc->SetValue(ScAddress(3, 4, 0), 5.0);

    // E1:E5 contain formulas.
    m_pDoc->SetString(ScAddress(4, 0, 0), "=VLOOKUP(D1;$A$1:$B$8;2)");
    m_pDoc->SetString(ScAddress(4, 1, 0), "=VLOOKUP(D2;$A$1:$B$8;2)");
    m_pDoc->SetString(ScAddress(4, 2, 0), "=VLOOKUP(D3;$A$1:$B$8;2)");
    m_pDoc->SetString(ScAddress(4, 3, 0), "=VLOOKUP(D4;$A$1:$B$8;2)");
    m_pDoc->SetString(ScAddress(4, 4, 0), "=VLOOKUP(D5;$A$1:$B$8;2)");
    m_pDoc->CalcAll();

    // Check the formula results in E1:E5.
    CPPUNIT_ASSERT_EQUAL(OUString("One"), m_pDoc->GetString(ScAddress(4, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Two"), m_pDoc->GetString(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Two"), m_pDoc->GetString(ScAddress(4, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Four"), m_pDoc->GetString(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Four"), m_pDoc->GetString(ScAddress(4, 4, 0)));

    // Start over again.
    clearSheet(m_pDoc, 0);

    // Set A,B,...,G to A1:A7.
    m_pDoc->SetString(ScAddress(0, 0, 0), "A");
    m_pDoc->SetString(ScAddress(0, 1, 0), "B");
    m_pDoc->SetString(ScAddress(0, 2, 0), "C");
    m_pDoc->SetString(ScAddress(0, 3, 0), "D");
    m_pDoc->SetString(ScAddress(0, 4, 0), "E");
    m_pDoc->SetString(ScAddress(0, 5, 0), "F");
    m_pDoc->SetString(ScAddress(0, 6, 0), "G");

    // Set the formula in C1.
    m_pDoc->SetString(ScAddress(2, 0, 0), "=VLOOKUP(\"C\";A1:A16;1)");
    CPPUNIT_ASSERT_EQUAL(OUString("C"), m_pDoc->GetString(ScAddress(2, 0, 0)));

    // A21:E24, test position dependent implicit intersection as argument to a
    // scalar value parameter in a function that has a ReferenceOrForceArray
    // type parameter somewhere else and formula is not in array mode,
    // VLOOKUP(Value;ReferenceOrForceArray;...)
    std::vector<std::vector<const char*>> aData2
        = { { "1", "one", "3", "=VLOOKUP(C21:C24;A21:B24;2;0)", "three" },
            { "2", "two", "1", "=VLOOKUP(C21:C24;A21:B24;2;0)", "one" },
            { "3", "three", "4", "=VLOOKUP(C21:C24;A21:B24;2;0)", "four" },
            { "4", "four", "2", "=VLOOKUP(C21:C24;A21:B24;2;0)", "two" } };

    ScAddress aPos2(0, 20, 0);
    ScRange aRange2 = insertRangeData(m_pDoc, aPos2, aData2);
    CPPUNIT_ASSERT_EQUAL(aPos2, aRange2.aStart);

    aPos2.SetCol(3); // column D formula results
    for (size_t i = 0; i < aData2.size(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(aData2[i][4]), m_pDoc->GetString(aPos2));
        aPos2.IncRow();
    }

    m_pDoc->DeleteTab(0);
}

template <size_t DataSize, size_t FormulaSize, int Type>
void TestFormula2::runTestMATCH(ScDocument* pDoc, const char* aData[DataSize],
                                const StrStrCheck aChecks[FormulaSize])
{
    size_t nDataSize = DataSize;
    for (size_t i = 0; i < nDataSize; ++i)
        pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i]));

    for (size_t i = 0; i < FormulaSize; ++i)
    {
        pDoc->SetString(1, i, 0, OUString::createFromAscii(aChecks[i].pVal));

        OUString aFormula = "=MATCH(B" + OUString::number(i + 1) + ";A1:A"
                            + OUString::number(nDataSize) + ";" + OUString::number(Type) + ")";
        pDoc->SetString(2, i, 0, aFormula);
    }

    pDoc->CalcAll();
    printRange(pDoc, ScRange(0, 0, 0, 2, FormulaSize - 1, 0), "MATCH");

    // verify the results.
    for (size_t i = 0; i < FormulaSize; ++i)
    {
        OUString aStr = pDoc->GetString(2, i, 0);
        if (!aStr.equalsAscii(aChecks[i].pRes))
        {
            cerr << "row " << (i + 1) << ": expected='" << aChecks[i].pRes << "' actual='" << aStr
                 << "'"
                    " criterion='"
                 << aChecks[i].pVal << "'" << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for MATCH", false);
        }
    }
}

template <size_t DataSize, size_t FormulaSize, int Type>
void TestFormula2::runTestHorizontalMATCH(ScDocument* pDoc, const char* aData[DataSize],
                                          const StrStrCheck aChecks[FormulaSize])
{
    size_t nDataSize = DataSize;
    for (size_t i = 0; i < nDataSize; ++i)
        pDoc->SetString(i, 0, 0, OUString::createFromAscii(aData[i]));

    for (size_t i = 0; i < FormulaSize; ++i)
    {
        pDoc->SetString(i, 1, 0, OUString::createFromAscii(aChecks[i].pVal));

        // Assume we don't have more than 26 data columns...
        OUString aFormula = "=MATCH(" + OUStringChar(static_cast<sal_Unicode>('A' + i))
                            + "2;A1:" + OUStringChar(static_cast<sal_Unicode>('A' + nDataSize))
                            + "1;" + OUString::number(Type) + ")";
        pDoc->SetString(i, 2, 0, aFormula);
    }

    pDoc->CalcAll();
    printRange(pDoc, ScRange(0, 0, 0, FormulaSize - 1, 2, 0), "MATCH");

    // verify the results.
    for (size_t i = 0; i < FormulaSize; ++i)
    {
        OUString aStr = pDoc->GetString(i, 2, 0);
        if (!aStr.equalsAscii(aChecks[i].pRes))
        {
            cerr << "column " << char('A' + i) << ": expected='" << aChecks[i].pRes << "' actual='"
                 << aStr
                 << "'"
                    " criterion='"
                 << aChecks[i].pVal << "'" << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for horizontal MATCH", false);
        }
    }
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncMATCH)
{
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "foo"));

    clearRange(m_pDoc, ScRange(0, 0, 0, 40, 40, 0));
    {
        // Ascending in-exact match

        // data range (A1:A9)
        const char* aData[] = {
            "1", "2", "3", "4", "5", "6", "7", "8", "9", "B", "B", "C",
        };

        // formula (B1:C12)
        static const StrStrCheck aChecks[]
            = { { "0.8", "#N/A" },  { "1.2", "1" },    { "2.3", "2" },     { "3.9", "3" },
                { "4.1", "4" },     { "5.99", "5" },   { "6.1", "6" },     { "7.2", "7" },
                { "8.569", "8" },   { "9.59", "9" },   { "10", "9" },      { "100", "9" },
                { "Andy", "#N/A" }, { "Bruce", "11" }, { "Charlie", "12" } };

        runTestMATCH<SAL_N_ELEMENTS(aData), SAL_N_ELEMENTS(aChecks), 1>(m_pDoc, aData, aChecks);
        clearRange(m_pDoc, ScRange(0, 0, 0, 4, 40, 0));
        runTestHorizontalMATCH<SAL_N_ELEMENTS(aData), SAL_N_ELEMENTS(aChecks), 1>(m_pDoc, aData,
                                                                                  aChecks);
        clearRange(m_pDoc, ScRange(0, 0, 0, 40, 4, 0));
    }

    {
        // Descending in-exact match

        // data range (A1:A9)
        const char* aData[] = { "D", "C", "B", "9", "8", "7", "6", "5", "4", "3", "2", "1" };

        // formula (B1:C12)
        static const StrStrCheck aChecks[]
            = { { "10", "#N/A" }, { "8.9", "4" },   { "7.8", "5" },     { "6.7", "6" },
                { "5.5", "7" },   { "4.6", "8" },   { "3.3", "9" },     { "2.2", "10" },
                { "1.1", "11" },  { "0.8", "12" },  { "0", "12" },      { "-2", "12" },
                { "Andy", "3" },  { "Bruce", "2" }, { "Charlie", "1" }, { "David", "#N/A" } };

        runTestMATCH<SAL_N_ELEMENTS(aData), SAL_N_ELEMENTS(aChecks), -1>(m_pDoc, aData, aChecks);
        clearRange(m_pDoc, ScRange(0, 0, 0, 4, 40, 0));
        runTestHorizontalMATCH<SAL_N_ELEMENTS(aData), SAL_N_ELEMENTS(aChecks), -1>(m_pDoc, aData,
                                                                                   aChecks);
        clearRange(m_pDoc, ScRange(0, 0, 0, 40, 4, 0));
    }

    {
        // search range contains leading and trailing empty cell ranges.

        clearRange(m_pDoc, ScRange(0, 0, 0, 2, 100, 0));

        // A5:A8 contains sorted values.
        m_pDoc->SetValue(ScAddress(0, 4, 0), 1.0);
        m_pDoc->SetValue(ScAddress(0, 5, 0), 2.0);
        m_pDoc->SetValue(ScAddress(0, 6, 0), 3.0);
        m_pDoc->SetValue(ScAddress(0, 7, 0), 4.0);

        // Find value 2 which is in A6.
        m_pDoc->SetString(ScAddress(1, 0, 0), "=MATCH(2;A1:A20)");
        m_pDoc->CalcAll();

        CPPUNIT_ASSERT_EQUAL(OUString("6"), m_pDoc->GetString(ScAddress(1, 0, 0)));
    }

    {
        // Test the ReferenceOrForceArray parameter.

        clearRange(m_pDoc, ScRange(0, 0, 0, 1, 7, 0));

        // B1:B5 contain numeric values.
        m_pDoc->SetValue(ScAddress(1, 0, 0), 1.0);
        m_pDoc->SetValue(ScAddress(1, 1, 0), 2.0);
        m_pDoc->SetValue(ScAddress(1, 2, 0), 3.0);
        m_pDoc->SetValue(ScAddress(1, 3, 0), 4.0);
        m_pDoc->SetValue(ScAddress(1, 4, 0), 5.0);

        // Find string value "33" in concatenated array, no implicit
        // intersection is involved, array is forced.
        m_pDoc->SetString(ScAddress(0, 5, 0), "=MATCH(\"33\";B1:B5&B1:B5)");
        m_pDoc->CalcAll();
        CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0, 5, 0)));
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncCELL)
{
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "foo"));

    clearRange(m_pDoc, ScRange(0, 0, 0, 2, 20, 0)); // Clear A1:C21.

    {
        const char* pContent = "Some random text";
        m_pDoc->SetString(2, 9, 0, OUString::createFromAscii(pContent)); // Set this value to C10.
        m_pDoc->SetValue(2, 0, 0, 1.2); // Set numeric value to C1;

        // We don't test: FILENAME, FORMAT, WIDTH, PROTECT, PREFIX
        StrStrCheck aChecks[]
            = { { "=CELL(\"COL\";C10)", "3" },           { "=CELL(\"COL\";C5:C10)", "3" },
                { "=CELL(\"ROW\";C10)", "10" },          { "=CELL(\"ROW\";C10:E10)", "10" },
                { "=CELL(\"SHEET\";C10)", "1" },         { "=CELL(\"ADDRESS\";C10)", "$C$10" },
                { "=CELL(\"CONTENTS\";C10)", pContent }, { "=CELL(\"COLOR\";C10)", "0" },
                { "=CELL(\"TYPE\";C9)", "b" },           { "=CELL(\"TYPE\";C10)", "l" },
                { "=CELL(\"TYPE\";C1)", "v" },           { "=CELL(\"PARENTHESES\";C10)", "0" } };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
            m_pDoc->SetString(0, i, 0, OUString::createFromAscii(aChecks[i].pVal));
        m_pDoc->CalcAll();

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            OUString aVal = m_pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for CELL", aVal.equalsAscii(aChecks[i].pRes));
        }
    }

    m_pDoc->DeleteTab(0);
}

/** See also test case document fdo#44456 sheet cpearson */
CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncDATEDIF)
{
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "foo"));

    std::vector<std::vector<const char*>> aData = {
        { "2007-01-01", "2007-01-10", "d", "9", "=DATEDIF(A1;B1;C1)" },
        { "2007-01-01", "2007-01-31", "m", "0", "=DATEDIF(A2;B2;C2)" },
        { "2007-01-01", "2007-02-01", "m", "1", "=DATEDIF(A3;B3;C3)" },
        { "2007-01-01", "2007-02-28", "m", "1", "=DATEDIF(A4;B4;C4)" },
        { "2007-01-01", "2007-12-31", "d", "364", "=DATEDIF(A5;B5;C5)" },
        { "2007-01-01", "2007-01-31", "y", "0", "=DATEDIF(A6;B6;C6)" },
        { "2007-01-01", "2008-07-01", "d", "547", "=DATEDIF(A7;B7;C7)" },
        { "2007-01-01", "2008-07-01", "m", "18", "=DATEDIF(A8;B8;C8)" },
        { "2007-01-01", "2008-07-01", "ym", "6", "=DATEDIF(A9;B9;C9)" },
        { "2007-01-01", "2008-07-01", "yd", "182", "=DATEDIF(A10;B10;C10)" },
        { "2008-01-01", "2009-07-01", "yd", "181", "=DATEDIF(A11;B11;C11)" },
        { "2007-01-01", "2007-01-31", "md", "30", "=DATEDIF(A12;B12;C12)" },
        { "2007-02-01", "2009-03-01", "md", "0", "=DATEDIF(A13;B13;C13)" },
        { "2008-02-01", "2009-03-01", "md", "0", "=DATEDIF(A14;B14;C14)" },
        { "2007-01-02", "2007-01-01", "md", "Err:502",
          "=DATEDIF(A15;B15;C15)" } // fail date1 > date2
    };

    clearRange(m_pDoc, ScRange(0, 0, 0, 4, aData.size(), 0));
    ScAddress aPos(0, 0, 0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos,
                                 aDataRange.aStart);

    m_pDoc->CalcAll();

    for (size_t i = 0; i < aData.size(); ++i)
    {
        OUString aVal = m_pDoc->GetString(4, i, 0);
        //std::cout << "row "<< i << ": " << OUStringToOString( aVal, RTL_TEXTENCODING_UTF8).getStr() << ", expected " << aData[i][3] << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Unexpected result for DATEDIF", aVal.equalsAscii(aData[i][3]));
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncINDIRECT)
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, aTabName));
    clearRange(m_pDoc, ScRange(0, 0, 0, 0, 10, 0)); // Clear A1:A11

    bool bGood = m_pDoc->GetName(0, aTabName);
    CPPUNIT_ASSERT_MESSAGE("failed to get sheet name.", bGood);

    OUString aTest = "Test", aRefErr = "#REF!";
    m_pDoc->SetString(0, 10, 0, aTest);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell value.", aTest, m_pDoc->GetString(0, 10, 0));

    OUString aPrefix = "=INDIRECT(\"";

    OUString aFormula = aPrefix + aTabName + ".A11\")"; // Calc A1
    m_pDoc->SetString(0, 0, 0, aFormula);
    aFormula = aPrefix + aTabName + "!A11\")"; // Excel A1
    m_pDoc->SetString(0, 1, 0, aFormula);
    aFormula = aPrefix + aTabName + "!R11C1\")"; // Excel R1C1
    m_pDoc->SetString(0, 2, 0, aFormula);
    aFormula = aPrefix + aTabName + "!R11C1\";0)"; // Excel R1C1 (forced)
    m_pDoc->SetString(0, 3, 0, aFormula);

    m_pDoc->CalcAll();
    {
        // Default (for new documents) is to use current formula syntax
        // which is Calc A1
        const OUString* aChecks[] = { &aTest, &aRefErr, &aRefErr, &aTest };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            OUString aVal = m_pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong value!", *aChecks[i], aVal);
        }
    }

    ScCalcConfig aConfig;
    aConfig.SetStringRefSyntax(formula::FormulaGrammar::CONV_OOO);
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    {
        // Explicit Calc A1 syntax
        const OUString* aChecks[] = { &aTest, &aRefErr, &aRefErr, &aTest };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            OUString aVal = m_pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong value!", *aChecks[i], aVal);
        }
    }

    aConfig.SetStringRefSyntax(formula::FormulaGrammar::CONV_XL_A1);
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    {
        // Excel A1 syntax
        const OUString* aChecks[] = { &aRefErr, &aTest, &aRefErr, &aTest };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            OUString aVal = m_pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong value!", *aChecks[i], aVal);
        }
    }

    aConfig.SetStringRefSyntax(formula::FormulaGrammar::CONV_XL_R1C1);
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    {
        // Excel R1C1 syntax
        const OUString* aChecks[] = { &aRefErr, &aRefErr, &aTest, &aTest };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            OUString aVal = m_pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong value!", *aChecks[i], aVal);
        }
    }

    m_pDoc->DeleteTab(0);
}

// Test case for tdf#83365 - Access across spreadsheet returns Err:504
//
CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncINDIRECT2)
{
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "foo"));
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(1, "bar"));
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(2, "baz"));

    m_pDoc->SetValue(0, 0, 0, 10.0);
    m_pDoc->SetValue(0, 1, 0, 10.0);
    m_pDoc->SetValue(0, 2, 0, 10.0);

    // Fill range bar.$A1:bar.$A10 with 1s
    for (SCROW i = 0; i < 10; ++i)
        m_pDoc->SetValue(0, i, 1, 1.0);

    // Test range triplet (absolute, relative, relative) : (absolute, relative, relative)
    m_pDoc->SetString(0, 0, 2, "=COUNTIF(bar.$A1:INDIRECT(\"$A\"&foo.$A$1),1)");

    // Test range triplet (absolute, relative, relative) : (absolute, absolute, relative)
    m_pDoc->SetString(0, 1, 2, "=COUNTIF(bar.$A1:INDIRECT(\"$A\"&foo.$A$2),1)");

    // Test range triplet (absolute, relative, relative) : (absolute, absolute, absolute)
    m_pDoc->SetString(0, 2, 2, "=COUNTIF(bar.$A1:INDIRECT(\"$A\"&foo.$A$3),1)");

    // Test range triplet (absolute, absolute, relative) : (absolute, relative, relative)
    m_pDoc->SetString(0, 3, 2, "=COUNTIF(bar.$A$1:INDIRECT(\"$A\"&foo.$A$1),1)");

    // Test range triplet (absolute, absolute, relative) : (absolute, absolute, relative)
    m_pDoc->SetString(0, 4, 2, "=COUNTIF(bar.$A$1:INDIRECT(\"$A\"&foo.$A$2),1)");

    // Test range triplet (absolute, absolute, relative) : (absolute, absolute, relative)
    m_pDoc->SetString(0, 5, 2, "=COUNTIF(bar.$A$1:INDIRECT(\"$A\"&foo.$A$3),1)");

    // Test range triplet (absolute, absolute, absolute) : (absolute, relative, relative)
    m_pDoc->SetString(0, 6, 2, "=COUNTIF($bar.$A$1:INDIRECT(\"$A\"&foo.$A$1),1)");

    // Test range triplet (absolute, absolute, absolute) : (absolute, absolute, relative)
    m_pDoc->SetString(0, 7, 2, "=COUNTIF($bar.$A$1:INDIRECT(\"$A\"&foo.$A$2),1)");

    // Check indirect reference "bar.$A\"&foo.$A$1
    m_pDoc->SetString(0, 8, 2, "=COUNTIF(bar.$A$1:INDIRECT(\"bar.$A\"&foo.$A$1),1)");

    // This case should return illegal argument error because
    // they reference 2 different absolute sheets
    // Test range triplet (absolute, absolute, absolute) : (absolute, absolute, absolute)
    m_pDoc->SetString(0, 9, 2, "=COUNTIF($bar.$A$1:INDIRECT(\"$A\"&foo.$A$3),1)");

    m_pDoc->CalcAll();

    // Loop all formulas and check result = 10.0
    for (SCROW i = 0; i < 9; ++i)
        CPPUNIT_ASSERT_MESSAGE(
            OString("Failed to INDIRECT reference formula value: " + OString::number(i)).getStr(),
            m_pDoc->GetValue(0, i, 2) != 10.0);

    // Check formula cell error
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0, 9, 2));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This formula cell should be an error.",
                           pFC->GetErrCode() != FormulaError::NONE);

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

// Test for tdf#107724 do not propagate an array context from MATCH to INDIRECT
// as INDIRECT returns ParamClass::Reference
CPPUNIT_TEST_FIXTURE(TestFormula2, testFunc_MATCH_INDIRECT)
{
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "foo"));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    ScRangeName* pGlobalNames = m_pDoc->GetRangeName();
    ScRangeData* pRangeData = new ScRangeData(*m_pDoc, "RoleAssignment", "$D$4:$D$13");
    pGlobalNames->insert(pRangeData);

    // D6: data to match, in 3rd row of named range.
    m_pDoc->SetString(3, 5, 0, "Test1");
    // F15: Formula generating indirect reference of corner addresses taking
    // row+offset and column from named range, which are not in array context
    // thus don't create arrays of offsets.
    m_pDoc->SetString(5, 14, 0,
                      "=MATCH(\"Test1\";INDIRECT(ADDRESS(ROW(RoleAssignment)+1;COLUMN("
                      "RoleAssignment))&\":\"&ADDRESS(ROW(RoleAssignment)+ROWS(RoleAssignment)-1;"
                      "COLUMN(RoleAssignment)));0)");

    // Match in 2nd row of range offset by 1 expected.
    ASSERT_DOUBLES_EQUAL_MESSAGE("Failed to not propagate array context from MATCH to INDIRECT",
                                 2.0, m_pDoc->GetValue(5, 14, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFormulaDepTracking)
{
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "foo"));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    const ScAddress aA5(0, 4, 0);
    const ScAddress aB2(1, 1, 0);
    const ScAddress aB5(1, 4, 0);
    const ScAddress aC5(2, 4, 0);
    const ScAddress aD2(3, 1, 0);
    const ScAddress aD5(3, 4, 0);
    const ScAddress aD6(3, 5, 0);
    const ScAddress aE2(4, 1, 0);
    const ScAddress aE3(4, 2, 0);
    const ScAddress aE6(4, 5, 0);

    // B2 listens on D2.
    m_pDoc->SetString(aB2, "=D2");
    double val = m_pDoc->GetValue(aB2);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Referencing an empty cell should yield zero.", 0.0, val);

    {
        // Check the internal broadcaster state.
        auto aState = m_pDoc->GetBroadcasterState();
        aState.dump(std::cout, m_pDoc);
        CPPUNIT_ASSERT(aState.hasFormulaCellListener(aD2, aB2));
    }

    // Changing the value of D2 should trigger recalculation of B2.
    m_pDoc->SetValue(aD2, 1.1);
    val = m_pDoc->GetValue(aB2);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Failed to recalculate on value change.", 1.1, val);

    // And again.
    m_pDoc->SetValue(aD2, 2.2);
    val = m_pDoc->GetValue(aB2);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Failed to recalculate on value change.", 2.2, val);

    clearRange(m_pDoc, ScRange(0, 0, 0, 10, 10, 0));

    {
        // Make sure nobody is listening on anything.
        auto aState = m_pDoc->GetBroadcasterState();
        aState.dump(std::cout, m_pDoc);
        CPPUNIT_ASSERT(aState.aCellListenerStore.empty());
    }

    // Now, let's test the range dependency tracking.

    // B2 listens on D2:E6.
    m_pDoc->SetString(aB2, "=SUM(D2:E6)");
    val = m_pDoc->GetValue(aB2);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Summing an empty range should yield zero.", 0.0, val);

    {
        // Check the internal state to make sure it matches.
        auto aState = m_pDoc->GetBroadcasterState();
        aState.dump(std::cout, m_pDoc);
        CPPUNIT_ASSERT(aState.hasFormulaCellListener({ aD2, aE6 }, aB2));
    }

    // Set value to E3. This should trigger recalc on B2.
    m_pDoc->SetValue(aE3, 2.4);
    val = m_pDoc->GetValue(aB2);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Failed to recalculate on single value change.", 2.4, val);

    // Set value to D5 to trigger recalc again.  Note that this causes an
    // addition of 1.2 + 2.4 which is subject to binary floating point
    // rounding error.  We need to use approxEqual to assess its value.

    m_pDoc->SetValue(aD5, 1.2);
    val = m_pDoc->GetValue(aB2);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.",
                           rtl::math::approxEqual(val, 3.6));

    // Change the value of D2 (boundary case).
    m_pDoc->SetValue(aD2, 1.0);
    val = m_pDoc->GetValue(aB2);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.",
                           rtl::math::approxEqual(val, 4.6));

    // Change the value of E6 (another boundary case).
    m_pDoc->SetValue(aE6, 2.0);
    val = m_pDoc->GetValue(aB2);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.",
                           rtl::math::approxEqual(val, 6.6));

    // Change the value of D6 (another boundary case).
    m_pDoc->SetValue(aD6, 3.0);
    val = m_pDoc->GetValue(aB2);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.",
                           rtl::math::approxEqual(val, 9.6));

    // Change the value of E2 (another boundary case).
    m_pDoc->SetValue(aE2, 0.4);
    val = m_pDoc->GetValue(aB2);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.",
                           rtl::math::approxEqual(val, 10.0));

    // Change the existing non-empty value cell (E2).
    m_pDoc->SetValue(aE2, 2.4);
    val = m_pDoc->GetValue(aB2);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.",
                           rtl::math::approxEqual(val, 12.0));

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
        ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected formula value.", val,
                                     m_pDoc->GetValue(1, nRow, 0));
        ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected formula value.", val * 2.0,
                                     m_pDoc->GetValue(2, nRow, 0));
    }

    // Intentionally insert a formula in column 1. This will break column 1's
    // uniformity of consisting only of static value cells.
    m_pDoc->SetString(aA5, "=R2C3");
    ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected formula value.", 2.0, m_pDoc->GetValue(aA5));
    ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected formula value.", 2.0, m_pDoc->GetValue(aB5));
    ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected formula value.", 4.0, m_pDoc->GetValue(aC5));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFormulaDepTracking2)
{
    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "foo"));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    double val = 2.0;
    m_pDoc->SetValue(0, 0, 0, val);
    val = 4.0;
    m_pDoc->SetValue(1, 0, 0, val);
    val = 5.0;
    m_pDoc->SetValue(0, 1, 0, val);
    m_pDoc->SetString(2, 0, 0, "=A1/B1");
    m_pDoc->SetString(1, 1, 0, "=B1*C1");

    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1, 1, 0)); // B2 should equal 2.

    clearRange(m_pDoc, ScAddress(2, 0, 0)); // Delete C1.

    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(1, 1, 0)); // B2 should now equal 0.

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFormulaDepTracking3)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    m_pDoc->InsertTab(0, "Formula");

    std::vector<std::vector<const char*>> aData = {
        { "1", "2", "=SUM(A1:B1)", "=SUM(C1:C3)" },
        { "3", "4", "=SUM(A2:B2)", nullptr },
        { "5", "6", "=SUM(A3:B3)", nullptr },
    };

    insertRangeData(m_pDoc, ScAddress(0, 0, 0), aData);

    // Check the initial formula results.
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(2, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(ScAddress(3, 0, 0)));

    // Change B3 and make sure the change gets propagated to D1.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    rFunc.SetValueCell(ScAddress(1, 2, 0), 60.0, false);
    CPPUNIT_ASSERT_EQUAL(65.0, m_pDoc->GetValue(ScAddress(2, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(75.0, m_pDoc->GetValue(ScAddress(3, 0, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFormulaDepTrackingDeleteRow)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    m_pDoc->InsertTab(0, "Test");

    // Values in A1:A3.
    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    m_pDoc->SetValue(ScAddress(0, 1, 0), 3.0);
    m_pDoc->SetValue(ScAddress(0, 2, 0), 5.0);

    // SUM(A1:A3) in A5.
    m_pDoc->SetString(ScAddress(0, 4, 0), "=SUM(A1:A3)");

    // A6 to reference A5.
    m_pDoc->SetString(ScAddress(0, 5, 0), "=A5*10");
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0, 5, 0));
    CPPUNIT_ASSERT(pFC);

    // A4 should have a broadcaster with A5 listening to it.
    SvtBroadcaster* pBC = m_pDoc->GetBroadcaster(ScAddress(0, 4, 0));
    CPPUNIT_ASSERT(pBC);
    SvtBroadcaster::ListenersType* pListeners = &pBC->GetAllListeners();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A5 should have one listener.", size_t(1), pListeners->size());
    const SvtListener* pListener = pListeners->at(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A6 should be listening to A5.",
                                 static_cast<const ScFormulaCell*>(pListener), pFC);

    // Check initial values.
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(0, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(90.0, m_pDoc->GetValue(ScAddress(0, 5, 0)));

    // Delete row 2.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    rFunc.DeleteCells(ScRange(0, 1, 0, m_pDoc->MaxCol(), 1, 0), &aMark, DelCellCmd::CellsUp, true);

    pBC = m_pDoc->GetBroadcaster(ScAddress(0, 3, 0));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster at A5 should have shifted to A4.", pBC);
    pListeners = &pBC->GetAllListeners();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A3 should have one listener.", size_t(1), pListeners->size());
    pFC = m_pDoc->GetFormulaCell(ScAddress(0, 4, 0));
    CPPUNIT_ASSERT(pFC);
    pListener = pListeners->at(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A5 should be listening to A4.",
                                 static_cast<const ScFormulaCell*>(pListener), pFC);

    // Check values after row deletion.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(60.0, m_pDoc->GetValue(ScAddress(0, 4, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFormulaDepTrackingDeleteCol)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    m_pDoc->InsertTab(0, "Formula");

    std::vector<std::vector<const char*>> aData = {
        { "2", "=A1", "=B1" }, // not grouped
        { nullptr, nullptr, nullptr }, // empty row to separate the formula groups.
        { "3", "=A3", "=B3" }, // grouped
        { "4", "=A4", "=B4" }, // grouped
    };

    ScAddress aPos(0, 0, 0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData);
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    // Check the initial values.
    for (SCCOL i = 0; i <= 2; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(i, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(i, 2, 0)));
        CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(i, 3, 0)));
    }

    // Make sure B3:B4 and C3:C4 are grouped.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 2, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 2, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    // Delete column A.  A1, B1, A3:A4 and B3:B4 should all show #REF!.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    rFunc.DeleteCells(ScRange(0, 0, 0, 0, m_pDoc->MaxRow(), 0), &aMark, DelCellCmd::CellsLeft,
                      true);

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "#REF!", "#REF!" },
            { nullptr, nullptr },
            { "#REF!", "#REF!" },
            { "#REF!", "#REF!" },
        };

        ScRange aCheckRange(0, 0, 0, 1, 3, 0);
        bool bSuccess
            = checkOutput(m_pDoc, aCheckRange, aOutputCheck, "Check after deleting column A");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Undo and check the result.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "2", "2", "2" },
            { nullptr, nullptr, nullptr },
            { "3", "3", "3" },
            { "4", "4", "4" },
        };

        ScRange aCheckRange(0, 0, 0, 2, 3, 0);
        bool bSuccess = checkOutput(m_pDoc, aCheckRange, aOutputCheck, "Check after undo");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Redo and check.
    pUndoMgr->Redo();
    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "#REF!", "#REF!" },
            { nullptr, nullptr },
            { "#REF!", "#REF!" },
            { "#REF!", "#REF!" },
        };

        ScRange aCheckRange(0, 0, 0, 1, 3, 0);
        bool bSuccess = checkOutput(m_pDoc, aCheckRange, aOutputCheck, "Check after redo");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Undo and change the values in column A.
    pUndoMgr->Undo();
    m_pDoc->SetValue(ScAddress(0, 0, 0), 22.0);
    m_pDoc->SetValue(ScAddress(0, 2, 0), 23.0);
    m_pDoc->SetValue(ScAddress(0, 3, 0), 24.0);

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "22", "22", "22" },
            { nullptr, nullptr, nullptr },
            { "23", "23", "23" },
            { "24", "24", "24" },
        };

        ScRange aCheckRange(0, 0, 0, 2, 3, 0);
        bool bSuccess = checkOutput(m_pDoc, aCheckRange, aOutputCheck,
                                    "Check after undo & value change in column A");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFormulaMatrixResultUpdate)
{
    m_pDoc->InsertTab(0, "Test");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    // Set a numeric value to A1.
    m_pDoc->SetValue(ScAddress(0, 0, 0), 11.0);

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(1, 0, 1, 0, aMark, "=A1");
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(1, 0, 0)));
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 0, 0));
    CPPUNIT_ASSERT_MESSAGE("Failed to get formula cell.", pFC);
    pFC->SetChanged(
        false); // Clear this flag to simulate displaying of formula cell value on screen.

    m_pDoc->SetString(ScAddress(0, 0, 0), "ABC");
    CPPUNIT_ASSERT_EQUAL(OUString("ABC"), m_pDoc->GetString(ScAddress(1, 0, 0)));
    pFC->SetChanged(false);

    // Put a new value into A1. The formula should update.
    m_pDoc->SetValue(ScAddress(0, 0, 0), 13.0);
    CPPUNIT_ASSERT_EQUAL(13.0, m_pDoc->GetValue(ScAddress(1, 0, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testExternalRef)
{
    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString aExtDocName("file:///extdata.fake");
    OUString aExtSh1Name("Data1");
    OUString aExtSh2Name("Data2");
    OUString aExtSh3Name("Data3");
    SfxMedium* pMed = new SfxMedium(aExtDocName, StreamMode::STD_READWRITE);
    xExtDocSh->DoLoad(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != nullptr);

    // Populate the external source document.
    ScDocument& rExtDoc = xExtDocSh->GetDocument();
    rExtDoc.InsertTab(0, aExtSh1Name);
    rExtDoc.InsertTab(1, aExtSh2Name);
    rExtDoc.InsertTab(2, aExtSh3Name);

    OUString constexpr name(u"Name"_ustr);
    OUString constexpr value(u"Value"_ustr);

    // Sheet 1
    rExtDoc.SetString(0, 0, 0, name);
    rExtDoc.SetString(0, 1, 0, "Andy");
    rExtDoc.SetString(0, 2, 0, "Bruce");
    rExtDoc.SetString(0, 3, 0, "Charlie");
    rExtDoc.SetString(0, 4, 0, "David");
    rExtDoc.SetString(1, 0, 0, value);
    double val = 10;
    rExtDoc.SetValue(1, 1, 0, val);
    val = 11;
    rExtDoc.SetValue(1, 2, 0, val);
    val = 12;
    rExtDoc.SetValue(1, 3, 0, val);
    val = 13;
    rExtDoc.SetValue(1, 4, 0, val);

    // Sheet 2 remains empty.

    // Sheet 3
    rExtDoc.SetString(0, 0, 2, name);
    rExtDoc.SetString(0, 1, 2, "Edward");
    rExtDoc.SetString(0, 2, 2, "Frank");
    rExtDoc.SetString(0, 3, 2, "George");
    rExtDoc.SetString(0, 4, 2, "Henry");
    rExtDoc.SetString(1, 0, 2, value);
    val = 99;
    rExtDoc.SetValue(1, 1, 2, val);
    val = 98;
    rExtDoc.SetValue(1, 2, 2, val);
    val = 97;
    rExtDoc.SetValue(1, 3, 2, val);
    val = 96;
    rExtDoc.SetValue(1, 4, 2, val);

    // Test external references on the main document while the external
    // document is still in memory.
    m_pDoc->InsertTab(0, "Test Sheet");
    m_pDoc->SetString(0, 0, 0, "='file:///extdata.fake'#Data1.A1");
    OUString test = m_pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value is different from the original", name, test);

    // After the initial access to the external document, the external ref
    // manager should create sheet cache entries for *all* sheets from that
    // document.  Note that the doc may have more than 3 sheets but ensure
    // that the first 3 are what we expect.
    ScExternalRefManager* pRefMgr = m_pDoc->GetExternalRefManager();
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aExtDocName);
    vector<OUString> aTabNames;
    pRefMgr->getAllCachedTableNames(nFileId, aTabNames);
    CPPUNIT_ASSERT_MESSAGE("There should be at least 3 sheets.", aTabNames.size() >= 3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected sheet name.", aTabNames[0], aExtSh1Name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected sheet name.", aTabNames[1], aExtSh2Name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected sheet name.", aTabNames[2], aExtSh3Name);

    m_pDoc->SetString(1, 0, 0, "='file:///extdata.fake'#Data1.B1");
    test = m_pDoc->GetString(1, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value is different from the original", value, test);

    m_pDoc->SetString(0, 1, 0, "='file:///extdata.fake'#Data1.A2");
    m_pDoc->SetString(0, 2, 0, "='file:///extdata.fake'#Data1.A3");
    m_pDoc->SetString(0, 3, 0, "='file:///extdata.fake'#Data1.A4");
    m_pDoc->SetString(0, 4, 0, "='file:///extdata.fake'#Data1.A5");
    m_pDoc->SetString(0, 5, 0, "='file:///extdata.fake'#Data1.A6");

    {
        // Referencing an empty cell should display '0'.
        const char* pChecks[] = { "Andy", "Bruce", "Charlie", "David", "0" };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            test = m_pDoc->GetString(0, static_cast<SCROW>(i + 1), 0);
            CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", test.equalsAscii(pChecks[i]));
        }
    }
    m_pDoc->SetString(1, 1, 0, "='file:///extdata.fake'#Data1.B2");
    m_pDoc->SetString(1, 2, 0, "='file:///extdata.fake'#Data1.B3");
    m_pDoc->SetString(1, 3, 0, "='file:///extdata.fake'#Data1.B4");
    m_pDoc->SetString(1, 4, 0, "='file:///extdata.fake'#Data1.B5");
    m_pDoc->SetString(1, 5, 0, "='file:///extdata.fake'#Data1.B6");
    {
        double pChecks[] = { 10, 11, 12, 13, 0 };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            val = m_pDoc->GetValue(1, static_cast<SCROW>(i + 1), 0);
            ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected cell value.", pChecks[i], val);
        }
    }

    m_pDoc->SetString(2, 0, 0, "='file:///extdata.fake'#Data3.A1");
    m_pDoc->SetString(2, 1, 0, "='file:///extdata.fake'#Data3.A2");
    m_pDoc->SetString(2, 2, 0, "='file:///extdata.fake'#Data3.A3");
    m_pDoc->SetString(2, 3, 0, "='file:///extdata.fake'#Data3.A4");
    {
        const char* pChecks[] = { "Name", "Edward", "Frank", "George" };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            test = m_pDoc->GetString(2, static_cast<SCROW>(i), 0);
            CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", test.equalsAscii(pChecks[i]));
        }
    }

    m_pDoc->SetString(3, 0, 0, "='file:///extdata.fake'#Data3.B1");
    m_pDoc->SetString(3, 1, 0, "='file:///extdata.fake'#Data3.B2");
    m_pDoc->SetString(3, 2, 0, "='file:///extdata.fake'#Data3.B3");
    m_pDoc->SetString(3, 3, 0, "='file:///extdata.fake'#Data3.B4");
    {
        const char* pChecks[] = { "Value", "99", "98", "97" };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            test = m_pDoc->GetString(3, static_cast<SCROW>(i), 0);
            CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", test.equalsAscii(pChecks[i]));
        }
    }

    // At this point, all accessed cell data from the external document should
    // have been cached.
    ScExternalRefCache::TableTypeRef pCacheTab
        = pRefMgr->getCacheTable(nFileId, aExtSh1Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 1 should exist.", pCacheTab);
    ScRange aCachedRange = getCachedRange(pCacheTab);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cached data range.", SCCOL(0),
                                 aCachedRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cached data range.", SCCOL(1),
                                 aCachedRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cached data range.", SCROW(0),
                                 aCachedRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cached data range.", SCROW(4),
                                 aCachedRange.aEnd.Row());

    // Sheet2 is not referenced at all; the cache table shouldn't even exist.
    pCacheTab = pRefMgr->getCacheTable(nFileId, aExtSh2Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 2 should *not* exist.", !pCacheTab);

    // Sheet3's row 5 is not referenced; it should not be cached.
    pCacheTab = pRefMgr->getCacheTable(nFileId, aExtSh3Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 3 should exist.", pCacheTab);
    aCachedRange = getCachedRange(pCacheTab);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cached data range.", SCCOL(0),
                                 aCachedRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cached data range.", SCCOL(1),
                                 aCachedRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cached data range.", SCROW(0),
                                 aCachedRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cached data range.", SCROW(3),
                                 aCachedRange.aEnd.Row());

    // Unload the external document shell.
    xExtDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("external document instance should have been unloaded.",
                           !findLoadedDocShellByName(aExtDocName));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testExternalRangeName)
{
    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString constexpr aExtDocName(u"file:///extdata.fake"_ustr);
    SfxMedium* pMed = new SfxMedium(aExtDocName, StreamMode::STD_READWRITE);
    xExtDocSh->DoLoad(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != nullptr);

    ScDocument& rExtDoc = xExtDocSh->GetDocument();
    rExtDoc.InsertTab(0, "Data1");
    rExtDoc.SetValue(0, 0, 0, 123.456);

    ScRangeName* pRangeName = rExtDoc.GetRangeName();
    ScRangeData* pRangeData = new ScRangeData(rExtDoc, "ExternalName", "$Data1.$A$1");
    pRangeName->insert(pRangeData);

    m_pDoc->InsertTab(0, "Test Sheet");
    m_pDoc->SetString(0, 1, 0, "='file:///extdata.fake'#ExternalName");

    double nVal = m_pDoc->GetValue(0, 1, 0);
    ASSERT_DOUBLES_EQUAL(123.456, nVal);

    xExtDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("external document instance should have been unloaded.",
                           !findLoadedDocShellByName(aExtDocName));
    m_pDoc->DeleteTab(0);
}

void TestFormula2::testExtRefFuncT(ScDocument* pDoc, ScDocument& rExtDoc)
{
    clearRange(pDoc, ScRange(0, 0, 0, 1, 9, 0));
    clearRange(&rExtDoc, ScRange(0, 0, 0, 1, 9, 0));

    rExtDoc.SetString(0, 0, 0, "'1.2");
    rExtDoc.SetString(0, 1, 0, "Foo");
    rExtDoc.SetValue(0, 2, 0, 12.3);
    pDoc->SetString(0, 0, 0, "=T('file:///extdata.fake'#Data.A1)");
    pDoc->SetString(0, 1, 0, "=T('file:///extdata.fake'#Data.A2)");
    pDoc->SetString(0, 2, 0, "=T('file:///extdata.fake'#Data.A3)");
    pDoc->CalcAll();

    OUString aRes = pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected result with T.", OUString("1.2"), aRes);
    aRes = pDoc->GetString(0, 1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected result with T.", OUString("Foo"), aRes);
    aRes = pDoc->GetString(0, 2, 0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected result with T.", aRes.isEmpty());
}

void TestFormula2::testExtRefFuncOFFSET(ScDocument* pDoc, ScDocument& rExtDoc)
{
    clearRange(pDoc, ScRange(0, 0, 0, 1, 9, 0));
    clearRange(&rExtDoc, ScRange(0, 0, 0, 1, 9, 0));

    sc::AutoCalcSwitch aACSwitch(*pDoc, true);

    // External document has sheet named 'Data', and the internal doc has sheet named 'Test'.
    rExtDoc.SetValue(ScAddress(0, 1, 0), 1.2); // Set 1.2 to A2.
    pDoc->SetString(ScAddress(0, 0, 0), "=OFFSET('file:///extdata.fake'#Data.$A$1;1;0;1;1)");
    CPPUNIT_ASSERT_EQUAL(1.2, pDoc->GetValue(ScAddress(0, 0, 0)));
}

void TestFormula2::testExtRefFuncVLOOKUP(ScDocument* pDoc, ScDocument& rExtDoc)
{
    clearRange(pDoc, ScRange(0, 0, 0, 1, 9, 0));
    clearRange(&rExtDoc, ScRange(0, 0, 0, 1, 9, 0));

    // Populate the external document.
    rExtDoc.SetString(ScAddress(0, 0, 0), "A1");
    rExtDoc.SetString(ScAddress(0, 1, 0), "A2");
    rExtDoc.SetString(ScAddress(0, 2, 0), "A3");
    rExtDoc.SetString(ScAddress(0, 3, 0), "A4");
    rExtDoc.SetString(ScAddress(0, 4, 0), "A5");

    rExtDoc.SetString(ScAddress(1, 0, 0), "B1");
    rExtDoc.SetString(ScAddress(1, 1, 0), "B2");
    rExtDoc.SetString(ScAddress(1, 2, 0), "B3");
    rExtDoc.SetString(ScAddress(1, 3, 0), "B4");
    rExtDoc.SetString(ScAddress(1, 4, 0), "B5");

    // Put formula in the source document.

    pDoc->SetString(ScAddress(0, 0, 0), "A2");

    // Sort order TRUE
    pDoc->SetString(ScAddress(1, 0, 0), "=VLOOKUP(A1;'file:///extdata.fake'#Data.A1:B5;2;1)");
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), pDoc->GetString(ScAddress(1, 0, 0)));

    // Sort order FALSE. It should return the same result.
    pDoc->SetString(ScAddress(1, 0, 0), "=VLOOKUP(A1;'file:///extdata.fake'#Data.A1:B5;2;0)");
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), pDoc->GetString(ScAddress(1, 0, 0)));
}

void TestFormula2::testExtRefConcat(ScDocument* pDoc, ScDocument& rExtDoc)
{
    clearRange(pDoc, ScRange(0, 0, 0, 1, 9, 0));
    clearRange(&rExtDoc, ScRange(0, 0, 0, 1, 9, 0));

    sc::AutoCalcSwitch aACSwitch(*pDoc, true);

    // String and number
    rExtDoc.SetString(ScAddress(0, 0, 0), "Answer: ");
    rExtDoc.SetValue(ScAddress(0, 1, 0), 42);

    // Concat operation should combine string and number converted to string
    pDoc->SetString(ScAddress(0, 0, 0),
                    "='file:///extdata.fake'#Data.A1 & 'file:///extdata.fake'#Data.A2");
    CPPUNIT_ASSERT_EQUAL(OUString("Answer: 42"), pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testExternalRefFunctions)
{
    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString aExtDocName("file:///extdata.fake");
    SfxMedium* pMed = new SfxMedium(aExtDocName, StreamMode::STD_READWRITE);
    xExtDocSh->DoLoad(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != nullptr);

    ScExternalRefManager* pRefMgr = m_pDoc->GetExternalRefManager();
    CPPUNIT_ASSERT_MESSAGE("external reference manager doesn't exist.", pRefMgr);
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aExtDocName);
    const OUString* pFileName = pRefMgr->getExternalFileName(nFileId);
    CPPUNIT_ASSERT_MESSAGE("file name registration has somehow failed.", pFileName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("file name registration has somehow failed.", aExtDocName,
                                 *pFileName);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    // Populate the external source document.
    ScDocument& rExtDoc = xExtDocSh->GetDocument();
    rExtDoc.InsertTab(0, "Data");
    double val = 1;
    rExtDoc.SetValue(0, 0, 0, val);
    // leave cell B1 empty.
    val = 2;
    rExtDoc.SetValue(0, 1, 0, val);
    rExtDoc.SetValue(1, 1, 0, val);
    val = 3;
    rExtDoc.SetValue(0, 2, 0, val);
    rExtDoc.SetValue(1, 2, 0, val);
    val = 4;
    rExtDoc.SetValue(0, 3, 0, val);
    rExtDoc.SetValue(1, 3, 0, val);

    m_pDoc->InsertTab(0, "Test");

    static const struct
    {
        const char* pFormula;
        double fResult;
    } aChecks[] = {
        { "=SUM('file:///extdata.fake'#Data.A1:A4)", 10 },
        { "=SUM('file:///extdata.fake'#Data.B1:B4)", 9 },
        { "=AVERAGE('file:///extdata.fake'#Data.A1:A4)", 2.5 },
        { "=AVERAGE('file:///extdata.fake'#Data.B1:B4)", 3 },
        { "=COUNT('file:///extdata.fake'#Data.A1:A4)", 4 },
        { "=COUNT('file:///extdata.fake'#Data.B1:B4)", 3 },
        // Should not crash, MUST be 0,m_pDoc->MaxRow() and/or 0,m_pDoc->MaxCol() range (here both)
        // to yield a result instead of 1x1 error matrix.
        { "=SUM('file:///extdata.fake'#Data.1:1048576)", 19 }
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        m_pDoc->SetString(0, 0, 0, OUString::createFromAscii(aChecks[i].pFormula));
        val = m_pDoc->GetValue(0, 0, 0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("unexpected result involving external ranges.",
                                             aChecks[i].fResult, val, 1e-15);
    }

    // A huge external range should not crash, the matrix generated from the
    // external range reference should be 1x1 and have one error value.
    // XXX NOTE: in case we supported sparse matrix that can hold this large
    // areas these tests may be adapted.
    m_pDoc->SetString(0, 0, 0, "=SUM('file:///extdata.fake'#Data.B1:AMJ1048575)");
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0, 0, 0));
    FormulaError nErr = pFC->GetErrCode();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "huge external range reference expected to yield FormulaError::MatrixSize",
        int(FormulaError::MatrixSize), static_cast<int>(nErr));

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 0, 0, 0, aMark, "'file:///extdata.fake'#Data.B1:AMJ1048575");
    pFC = m_pDoc->GetFormulaCell(ScAddress(0, 0, 0));
    nErr = pFC->GetErrCode();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "huge external range reference expected to yield FormulaError::MatrixSize",
        int(FormulaError::MatrixSize), static_cast<int>(nErr));
    SCSIZE nMatCols, nMatRows;
    const ScMatrix* pMat = pFC->GetMatrix();
    CPPUNIT_ASSERT_MESSAGE("matrix expected", pMat != nullptr);
    pMat->GetDimensions(nMatCols, nMatRows);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("1x1 matrix expected", SCSIZE(1), nMatCols);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("1x1 matrix expected", SCSIZE(1), nMatRows);

    pRefMgr->clearCache(nFileId);
    testExtRefFuncT(m_pDoc, rExtDoc);
    testExtRefFuncOFFSET(m_pDoc, rExtDoc);
    testExtRefFuncVLOOKUP(m_pDoc, rExtDoc);
    testExtRefConcat(m_pDoc, rExtDoc);

    // Unload the external document shell.
    xExtDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("external document instance should have been unloaded.",
                           !findLoadedDocShellByName(aExtDocName));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testExternalRefUnresolved)
{
#if !defined(_WIN32) //FIXME
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.
    m_pDoc->InsertTab(0, "Test");

    // Test error propagation of unresolved (not existing document) external
    // references. Well, let's hope no build machine has such file with sheet...

    std::vector<std::vector<const char*>> aData = {
        { "='file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1" },
        { "='file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1+23" },
        { "='file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1&\"W\"" },
        { "=ISREF('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1)" },
        { "=ISERROR('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1)" },
        { "=ISERR('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1)" },
        { "=ISBLANK('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1)" },
        { "=ISNUMBER('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1)" },
        { "=ISTEXT('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1)" },
        { "=ISNUMBER('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1+23)" },
        { "=ISTEXT('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1&\"W\")" },
        { "='file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1=0" },
        { "='file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1=\"\"" },
        { "=INDIRECT(\"'file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1\")" },
        { "='file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2" },
        { "='file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2+23" },
        { "='file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2&\"W\"" },
        { "=ISREF('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2)" },
        { "=ISERROR('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2)" },
        { "=ISERR('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2)" },
        { "=ISBLANK('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2)" },
        { "=ISNUMBER('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2)" },
        { "=ISTEXT('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2)" },
        { "=ISNUMBER('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2+23)" },
        { "=ISTEXT('file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2&\"W\")" },
        // TODO: gives Err:504 FIXME { "='file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2=0" },
        // TODO: gives Err:504 FIXME { "='file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2=\"\"" },
        { "=INDIRECT(\"'file:///NonExistingFilePath/AnyName.ods'#$NoSuchSheet.A1:A2\")" },
    };

    ScAddress aPos(0, 0, 0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData);
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    std::vector<std::vector<const char*>> aOutputCheck = {
        { "#REF!" }, // plain single ref
        { "#REF!" }, // +23
        { "#REF!" }, // &"W"
        { "FALSE" }, // ISREF
        { "TRUE" }, // ISERROR
        { "TRUE" }, // ISERR
        { "FALSE" }, // ISBLANK
        { "FALSE" }, // ISNUMBER
        { "FALSE" }, // ISTEXT
        { "FALSE" }, // ISNUMBER
        { "FALSE" }, // ISTEXT
        { "#REF!" }, // =0
        { "#REF!" }, // =""
        { "#REF!" }, // INDIRECT
        { "#REF!" }, // A1:A2 range
        { "#REF!" }, // +23
        { "#REF!" }, // &"W"
        { "FALSE" }, // ISREF
        { "TRUE" }, // ISERROR
        { "TRUE" }, // ISERR
        { "FALSE" }, // ISBLANK
        { "FALSE" }, // ISNUMBER
        { "FALSE" }, // ISTEXT
        { "FALSE" }, // ISNUMBER
        { "FALSE" }, // ISTEXT
        // TODO: gives Err:504 FIXME { "#REF!" },    // =0
        // TODO: gives Err:504 FIXME { "#REF!" },    // =""
        { "#REF!" }, // INDIRECT
    };

    bool bSuccess
        = checkOutput(m_pDoc, aRange, aOutputCheck, "Check unresolved external reference.");
    CPPUNIT_ASSERT_MESSAGE("Unresolved reference check failed", bSuccess);

    m_pDoc->DeleteTab(0);
#endif
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testMatrixOp)
{
    m_pDoc->InsertTab(0, "Test");

    for (SCROW nRow = 0; nRow < 4; ++nRow)
    {
        m_pDoc->SetValue(0, nRow, 0, nRow);
    }
    m_pDoc->SetValue(1, 0, 0, 2.0);
    m_pDoc->SetValue(3, 0, 0, 1.0);
    m_pDoc->SetValue(3, 1, 0, 2.0);
    m_pDoc->SetString(2, 0, 0, "=SUMPRODUCT((A1:A4)*B1+D1)");
    m_pDoc->SetString(2, 1, 0, "=SUMPRODUCT((A1:A4)*B1-D2)");

    double nVal = m_pDoc->GetValue(2, 0, 0);
    CPPUNIT_ASSERT_EQUAL(16.0, nVal);

    nVal = m_pDoc->GetValue(2, 1, 0);
    CPPUNIT_ASSERT_EQUAL(4.0, nVal);

    m_pDoc->SetString(4, 0, 0, "=SUMPRODUCT({1;2;4}+8)");
    m_pDoc->SetString(4, 1, 0, "=SUMPRODUCT(8+{1;2;4})");
    m_pDoc->SetString(4, 2, 0, "=SUMPRODUCT({1;2;4}-8)");
    m_pDoc->SetString(4, 3, 0, "=SUMPRODUCT(8-{1;2;4})");
    m_pDoc->SetString(4, 4, 0, "=SUMPRODUCT({1;2;4}+{8;16;32})");
    m_pDoc->SetString(4, 5, 0, "=SUMPRODUCT({8;16;32}+{1;2;4})");
    m_pDoc->SetString(4, 6, 0, "=SUMPRODUCT({1;2;4}-{8;16;32})");
    m_pDoc->SetString(4, 7, 0, "=SUMPRODUCT({8;16;32}-{1;2;4})");
    double fResult[8] = { 31.0, 31.0, -17.0, 17.0, 63.0, 63.0, -49.0, 49.0 };
    for (size_t i = 0; i < SAL_N_ELEMENTS(fResult); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(fResult[i], m_pDoc->GetValue(4, i, 0));
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncRangeOp)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");
    m_pDoc->InsertTab(2, "Sheet3");

    // Sheet1.B1:B3
    m_pDoc->SetValue(1, 0, 0, 1.0);
    m_pDoc->SetValue(1, 1, 0, 2.0);
    m_pDoc->SetValue(1, 2, 0, 4.0);
    // Sheet2.B1:B3
    m_pDoc->SetValue(1, 0, 1, 8.0);
    m_pDoc->SetValue(1, 1, 1, 16.0);
    m_pDoc->SetValue(1, 2, 1, 32.0);
    // Sheet3.B1:B3
    m_pDoc->SetValue(1, 0, 2, 64.0);
    m_pDoc->SetValue(1, 1, 2, 128.0);
    m_pDoc->SetValue(1, 2, 2, 256.0);

    // Range operator should extend concatenated literal references during
    // parse time already, so with this we can test ScComplexRefData::Extend()

    // Current sheet is Sheet1, so B1:B2 implies relative Sheet1.B1:B2

    ScAddress aPos(0, 0, 0);
    m_pDoc->SetString(aPos, "=SUM(B1:B2:B3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=SUM(B1:B3)"),
                                 m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, "=SUM(B1:B3:B2)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=SUM(B1:B3)"),
                                 m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, "=SUM(B2:B3:B1)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=SUM(B1:B3)"),
                                 m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, "=SUM(Sheet2.B1:B2:B3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=SUM(Sheet2.B1:B3)"),
                                 m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    CPPUNIT_ASSERT_EQUAL(56.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, "=SUM(B2:B2:Sheet1.B2)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=SUM(Sheet1.B2:B2)"),
                                 m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, "=SUM(B2:B3:Sheet2.B1)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=SUM(Sheet1.B1:Sheet2.B3)"),
                                 m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    CPPUNIT_ASSERT_EQUAL(63.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, "=SUM(Sheet1.B1:Sheet2.B2:Sheet3.B3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=SUM(Sheet1.B1:Sheet3.B3)"),
                                 m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    CPPUNIT_ASSERT_EQUAL(511.0, m_pDoc->GetValue(aPos));

    // B1:Sheet2.B2 would be ambiguous, Sheet1.B1:Sheet2.B2 or Sheet2.B1:B2
    // The actual representation of the error case may change, so this test may
    // have to be adapted.
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=SUM(B1:Sheet2.B2:Sheet3.B3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=SUM(b1:sheet2.b2:Sheet3.B3)"),
                                 m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    CPPUNIT_ASSERT_EQUAL(OUString("#NAME?"), m_pDoc->GetString(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, "=SUM(Sheet1.B1:Sheet3.B2:Sheet2.B3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=SUM(Sheet1.B1:Sheet3.B3)"),
                                 m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    CPPUNIT_ASSERT_EQUAL(511.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, "=SUM(B$2:B$2:B2)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=SUM(B$2:B2)"),
                                 m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(aPos));

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncFORMULA)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Sheet1");

    // Data in B1:D3
    std::vector<std::vector<const char*>> aData = {
        { "=A1", "=FORMULA(B1)", "=FORMULA(B1:B3)" },
        { nullptr, "=FORMULA(B2)", "=FORMULA(B1:B3)" },
        { "=A3", "=FORMULA(B3)", "=FORMULA(B1:B3)" },
    };

    ScAddress aPos(1, 0, 0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData);
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    // Checks of C1:D3, where Cy==Dy, and D4:D6
    const char* aChecks[] = {
        "=A1",
        "#N/A",
        "=A3",
    };
    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(aChecks[i]), m_pDoc->GetString(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(aChecks[i]), m_pDoc->GetString(3, i, 0));
    }

    // Matrix in D4:D6, no intersection with B1:B3
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(3, 3, 3, 5, aMark, "=FORMULA(B1:B3)");
    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(aChecks[i]), m_pDoc->GetString(3, i + 3, 0));
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncTableRef)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Sheet1");
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    ScDocFunc& rDocFunc = m_xDocShell->GetDocFunc();

    {
        ScDBCollection* pDBs = m_pDoc->GetDBCollection();
        CPPUNIT_ASSERT_MESSAGE("Failed to fetch DB collection object.", pDBs);

        // Insert "table" database range definition for A1:B4, with default
        // HasHeader=true and HasTotals=false.
        std::unique_ptr<ScDBData> pData(new ScDBData("table", 0, 0, 0, 1, 3));
        bool bInserted = pDBs->getNamedDBs().insert(std::move(pData));
        CPPUNIT_ASSERT_MESSAGE("Failed to insert \"table\" database range.", bInserted);
    }

    {
        // Populate "table" database range with headers and data in A1:B4
        std::vector<std::vector<const char*>> aData
            = { { "Header1", "Header2" }, { "1", "2" }, { "4", "8" }, { "16", "32" } };
        ScAddress aPos(0, 0, 0);
        ScRange aRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);
    }

    // Named expressions that use Table structured references.
    /* TODO: should the item/header separator really be equal to the parameter
     * separator, thus be locale dependent and ';' semicolon here, or should it
     * be a fixed ',' comma instead? */
    static const struct
    {
        const char* pName;
        const char* pExpr;
        const char*
            pCounta; // expected result when used in row 2 (first data row) as argument to COUNTA()
        const char*
            pSum3; // expected result when used in row 3 (second data row) as argument to SUM().
        const char*
            pSum4; // expected result when used in row 4 (third data row) as argument to SUM().
        const char*
            pSumX; // expected result when used in row 5 (non-intersecting) as argument to SUM().
    } aNames[]
        = { { "all", "table[[#All]]", "8", "63", "63", "63" },
            { "data_implicit", "table[]", "6", "63", "63", "63" },
            { "data", "table[[#Data]]", "6", "63", "63", "63" },
            { "headers", "table[[#Headers]]", "2", "0", "0", "0" },
            { "header1", "table[[Header1]]", "3", "21", "21", "21" },
            { "header2", "table[[Header2]]", "3", "42", "42", "42" },
            { "data_header1", "table[[#Data];[Header1]]", "3", "21", "21", "21" },
            { "data_header2", "table[[#Data];[Header2]]", "3", "42", "42", "42" },
            { "this_row", "table[[#This Row]]", "2", "12", "48", "#VALUE!" },
            { "this_row_header1", "table[[#This Row];[Header1]]", "1", "4", "16", "#VALUE!" },
            { "this_row_header2", "table[[#This Row];[Header2]]", "1", "8", "32", "#VALUE!" },
            { "this_row_range_header_1_to_2", "table[[#This Row];[Header1]:[Header2]]", "2", "12",
              "48", "#VALUE!" } };

    {
        // Insert named expressions.
        ScRangeName* pGlobalNames = m_pDoc->GetRangeName();
        CPPUNIT_ASSERT_MESSAGE("Failed to obtain global named expression object.", pGlobalNames);

        for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
        {
            // Choose base position that does not intersect with the database
            // range definition to test later use of [#This Row] results in
            // proper rows.
            ScRangeData* pName
                = new ScRangeData(*m_pDoc, OUString::createFromAscii(aNames[i].pName),
                                  OUString::createFromAscii(aNames[i].pExpr), ScAddress(2, 4, 0),
                                  ScRangeData::Type::Name, formula::FormulaGrammar::GRAM_NATIVE);
            bool bInserted = pGlobalNames->insert(pName);
            CPPUNIT_ASSERT_MESSAGE(OString(OString::Concat("Failed to insert named expression ")
                                           + aNames[i].pName + ".")
                                       .getStr(),
                                   bInserted);
        }
    }

    // Use the named expressions in COUNTA() formulas, on row 2 that intersects.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        OUString aFormula("=COUNTA(" + OUString::createFromAscii(aNames[i].pName) + ")");
        ScAddress aPos(3 + i, 1, 0);
        m_pDoc->SetString(aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + OUString::createFromAscii(aNames[i].pCounta)),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    // Use the named expressions in SUM() formulas, on row 3 that intersects.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        OUString aFormula("=SUM(" + OUString::createFromAscii(aNames[i].pName) + ")");
        ScAddress aPos(3 + i, 2, 0);
        m_pDoc->SetString(aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + OUString::createFromAscii(aNames[i].pSum3)),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    // Use the named expressions in SUM() formulas, on row 4 that intersects.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        OUString aFormula("=SUM(" + OUString::createFromAscii(aNames[i].pName) + ")");
        ScAddress aPos(3 + i, 3, 0);
        m_pDoc->SetString(aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + OUString::createFromAscii(aNames[i].pSum4)),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    // Use the named expressions in SUM() formulas, on row 5 that does not intersect.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        OUString aFormula("=SUM(" + OUString::createFromAscii(aNames[i].pName) + ")");
        ScAddress aPos(3 + i, 4, 0);
        m_pDoc->SetString(aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + OUString::createFromAscii(aNames[i].pSumX)),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    // Insert a column at column B to extend database range from column A,B to
    // A,B,C. Use ScDocFunc so RefreshDirtyTableColumnNames() is called.
    rDocFunc.InsertCells(ScRange(1, 0, 0, 1, m_pDoc->MaxRow(), 0), &aMark, INS_INSCOLS_BEFORE,
                         false, true);

    // Re-verify the named expression in SUM() formula, on row 4 that
    // intersects, now starting at column E, still works.
    m_pDoc->CalcAll();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        OUString aFormula("=SUM(" + OUString::createFromAscii(aNames[i].pName) + ")");
        ScAddress aPos(4 + i, 3, 0);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + OUString::createFromAscii(aNames[i].pSum4)),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    const char* pColumn2Formula = "=SUM(table[[#Data];[Column2]])";
    {
        // Populate "table" database range with empty header and data in newly
        // inserted column, B1:B4 plus a table formula in B6. The empty header
        // should result in the internal table column name "Column2" that is
        // used in the formula.
        std::vector<std::vector<const char*>> aData
            = { { "" }, { "64" }, { "128" }, { "256" }, { "" }, { pColumn2Formula } };
        ScAddress aPos(1, 0, 0);
        ScRange aRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);
    }

    // Verify the formula result in B6 (64+128+256=448).
    {
        OUString aFormula(OUString::createFromAscii(pColumn2Formula));
        ScAddress aPos(1, 5, 0);
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + "448"),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    // Set header in column B. Use ScDocFunc to have table column names refreshed.
    rDocFunc.SetStringCell(ScAddress(1, 0, 0), "NewHeader", true);
    // Verify that formula adapted using the updated table column names.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", OUString("=SUM(table[[#Data];[NewHeader]])"),
                                 m_pDoc->GetFormula(1, 5, 0));

    // Set header in column A to identical string. Internal table column name
    // for B should get a "2" appended.
    rDocFunc.SetStringCell(ScAddress(0, 0, 0), "NewHeader", true);
    // Verify that formula adapted using the updated table column names.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", OUString("=SUM(table[[#Data];[NewHeader2]])"),
                                 m_pDoc->GetFormula(1, 5, 0));

    // Set header in column B to empty string, effectively clearing the cell.
    rDocFunc.SetStringCell(ScAddress(1, 0, 0), "", true);
    // Verify that formula is still using the previous table column name.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", OUString("=SUM(table[[#Data];[NewHeader2]])"),
                                 m_pDoc->GetFormula(1, 5, 0));

    // === header-less ===

    {
        ScDBCollection* pDBs = m_pDoc->GetDBCollection();
        CPPUNIT_ASSERT_MESSAGE("Failed to fetch DB collection object.", pDBs);

        // Insert "headerless" database range definition for E10:F12, without headers.
        std::unique_ptr<ScDBData> pData(new ScDBData("hltable", 0, 4, 9, 5, 11, true, false));
        bool bInserted = pDBs->getNamedDBs().insert(std::move(pData));
        CPPUNIT_ASSERT_MESSAGE("Failed to insert \"hltable\" database range.", bInserted);
    }

    {
        // Populate "hltable" database range with data in E10:F12
        std::vector<std::vector<const char*>> aData
            = { { "1", "2" }, { "4", "8" }, { "16", "32" } };
        ScAddress aPos(4, 9, 0);
        ScRange aRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);
    }

    // Named expressions that use header-less Table structured references.
    static const struct
    {
        const char* pName;
        const char* pExpr;
        const char*
            pCounta; // expected result when used in row 10 (first data row) as argument to COUNTA()
        const char*
            pSum3; // expected result when used in row 11 (second data row) as argument to SUM().
        const char*
            pSum4; // expected result when used in row 12 (third data row) as argument to SUM().
        const char*
            pSumX; // expected result when used in row 13 (non-intersecting) as argument to SUM().
    } aHlNames[]
        = { { "hl_all", "hltable[[#All]]", "6", "63", "63", "63" },
            { "hl_data_implicit", "hltable[]", "6", "63", "63", "63" },
            { "hl_data", "hltable[[#Data]]", "6", "63", "63", "63" },
            { "hl_headers", "hltable[[#Headers]]", "1", "#REF!", "#REF!", "#REF!" },
            { "hl_column1", "hltable[[Column1]]", "3", "21", "21", "21" },
            { "hl_column2", "hltable[[Column2]]", "3", "42", "42", "42" },
            { "hl_data_column1", "hltable[[#Data];[Column1]]", "3", "21", "21", "21" },
            { "hl_data_column2", "hltable[[#Data];[Column2]]", "3", "42", "42", "42" },
            { "hl_this_row", "hltable[[#This Row]]", "2", "12", "48", "#VALUE!" },
            { "hl_this_row_column1", "hltable[[#This Row];[Column1]]", "1", "4", "16", "#VALUE!" },
            { "hl_this_row_column2", "hltable[[#This Row];[Column2]]", "1", "8", "32", "#VALUE!" },
            { "hl_this_row_range_column_1_to_2", "hltable[[#This Row];[Column1]:[Column2]]", "2",
              "12", "48", "#VALUE!" } };

    {
        // Insert named expressions.
        ScRangeName* pGlobalNames = m_pDoc->GetRangeName();
        CPPUNIT_ASSERT_MESSAGE("Failed to obtain global named expression object.", pGlobalNames);

        for (size_t i = 0; i < SAL_N_ELEMENTS(aHlNames); ++i)
        {
            // Choose base position that does not intersect with the database
            // range definition to test later use of [#This Row] results in
            // proper rows.
            ScRangeData* pName
                = new ScRangeData(*m_pDoc, OUString::createFromAscii(aHlNames[i].pName),
                                  OUString::createFromAscii(aHlNames[i].pExpr), ScAddress(6, 12, 0),
                                  ScRangeData::Type::Name, formula::FormulaGrammar::GRAM_NATIVE);
            bool bInserted = pGlobalNames->insert(pName);
            CPPUNIT_ASSERT_MESSAGE(OString(OString::Concat("Failed to insert named expression ")
                                           + aHlNames[i].pName + ".")
                                       .getStr(),
                                   bInserted);
        }
    }

    // Use the named expressions in COUNTA() formulas, on row 10 that intersects.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aHlNames); ++i)
    {
        OUString aFormula("=COUNTA(" + OUString::createFromAscii(aHlNames[i].pName) + ")");
        ScAddress aPos(7 + i, 9, 0);
        m_pDoc->SetString(aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + OUString::createFromAscii(aHlNames[i].pCounta)),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    // Use the named expressions in SUM() formulas, on row 11 that intersects.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aHlNames); ++i)
    {
        OUString aFormula("=SUM(" + OUString::createFromAscii(aHlNames[i].pName) + ")");
        ScAddress aPos(7 + i, 10, 0);
        m_pDoc->SetString(aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + OUString::createFromAscii(aHlNames[i].pSum3)),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    // Use the named expressions in SUM() formulas, on row 12 that intersects.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aHlNames); ++i)
    {
        OUString aFormula("=SUM(" + OUString::createFromAscii(aHlNames[i].pName) + ")");
        ScAddress aPos(7 + i, 11, 0);
        m_pDoc->SetString(aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + OUString::createFromAscii(aHlNames[i].pSum4)),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    // Use the named expressions in SUM() formulas, on row 13 that does not intersect.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aHlNames); ++i)
    {
        OUString aFormula("=SUM(" + OUString::createFromAscii(aHlNames[i].pName) + ")");
        ScAddress aPos(7 + i, 12, 0);
        m_pDoc->SetString(aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + OUString::createFromAscii(aHlNames[i].pSumX)),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    // Insert a column at column F to extend database range from column E,F to
    // E,F,G. Use ScDocFunc so RefreshDirtyTableColumnNames() is called.
    rDocFunc.InsertCells(ScRange(5, 0, 0, 5, m_pDoc->MaxRow(), 0), &aMark, INS_INSCOLS_BEFORE,
                         false, true);

    // Re-verify the named expression in SUM() formula, on row 12 that
    // intersects, now starting at column I, still works.
    m_pDoc->CalcAll();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aHlNames); ++i)
    {
        OUString aFormula("=SUM(" + OUString::createFromAscii(aHlNames[i].pName) + ")");
        ScAddress aPos(8 + i, 11, 0);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + OUString::createFromAscii(aHlNames[i].pSum4)),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    const char* pColumn3Formula = "=SUM(hltable[[#Data];[Column3]])";
    {
        // Populate "hltable" database range with data in newly inserted
        // column, F10:F12 plus a table formula in F14. The new header should
        // result in the internal table column name "Column3" that is used in
        // the formula.
        std::vector<std::vector<const char*>> aData
            = { { "64" }, { "128" }, { "256" }, { "" }, { pColumn3Formula } };
        ScAddress aPos(5, 9, 0);
        ScRange aRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);
    }

    // Verify the formula result in F14 (64+128+256=448).
    {
        OUString aFormula(OUString::createFromAscii(pColumn3Formula));
        ScAddress aPos(5, 13, 0);
        OUString aPrefix(aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL(OUString(aPrefix + "448"),
                             OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncFTEST)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "FTest");

    ScAddress aPos(6, 0, 0);
    m_pDoc->SetString(aPos, "=FTEST(A1:C3;D1:F3)");
    m_pDoc->SetValue(0, 0, 0, 9.0); // A1
    OUString aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("FTEST should return #VALUE! for less than 2 values",
                                 OUString("#VALUE!"), aVal);
    m_pDoc->SetValue(0, 1, 0, 8.0); // A2
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("FTEST should return #VALUE! for less than 2 values",
                                 OUString("#VALUE!"), aVal);
    m_pDoc->SetValue(3, 0, 0, 5.0); // D1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("FTEST should return #VALUE! for less than 2 values",
                                 OUString("#VALUE!"), aVal);
    m_pDoc->SetValue(3, 1, 0, 6.0); // D2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 1.0000,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 0, 0, 6.0); // B1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.6222,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 1, 0, 8.0); // B2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.7732,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(4, 0, 0, 7.0); // E1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.8194,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(4, 1, 0, 4.0); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.9674,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 0, 0, 3.0); // C1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.3402,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(5, 0, 0, 28.0); // F1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0161,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 1, 0, 9.0); // C2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0063,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(5, 1, 0, 4.0); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0081,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(0, 2, 0, 2.0); // A3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0122,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(3, 2, 0, 8.0); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0178,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 2, 0, 4.0); // B3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0093,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(4, 2, 0, 7.0); // E3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0132,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(5, 2, 0, 5.0); // F3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0168,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 2, 0, 13.0); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0422,
                                         m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->SetString(0, 2, 0, "a"); // A3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0334,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetString(2, 0, 0, "b"); // C1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0261,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetString(5, 1, 0, "c"); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0219,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetString(4, 2, 0, "d"); // E3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0161,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetString(3, 2, 0, "e"); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0110,
                                         m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->DeleteTab(0);
    m_pDoc->InsertTab(0, "FTest2");

    /* Summary of the following test
       A1:A5   =  SQRT(C1*9/10)*{ 1.0, 1.0, 1.0, 1.0, 1.0 };
       A6:A10  = -SQRT(C1*9/10)*{ 1.0, 1.0, 1.0, 1.0, 1.0 };
       B1:B10  =  SQRT(C2*19/20)*{ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
       B11:B20 = -SQRT(C2*19/20)*{ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
       C1      =  POWER(1.5, D1)   ; This is going to be the sample variance of the vector A1:A10
       C2      =  POWER(1.5, D2)   ; This is going to be the sample variance of the vector B1:B20
       D1 and D2 are varied over { -5.0, -4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 }

       Result of FTEST(A1:A10;B1:B20) in Calc is compared with that from Octave's var_test() function for each value of D1 and D2.

       The minimum variance ratio obtained in this way is 0.017342 and the maximum variance ratio is 57.665039
    */

    const size_t nNumParams = 11;
    const double fParameter[nNumParams]
        = { -5.0, -4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };

    // Results of var_test() from Octave
    const double fResults[nNumParams][nNumParams] = {
        { 0.9451191535603041, 0.5429768686792684, 0.213130093422756, 0.06607644828558357,
          0.0169804365506927, 0.003790723514148109, 0.0007645345628801703, 0.0001435746909905777,
          2.566562398786942e-05, 4.436218417280813e-06, 7.495090956766148e-07 },
        { 0.4360331979746912, 0.9451191535603054, 0.5429768686792684, 0.2131300934227565,
          0.06607644828558357, 0.0169804365506927, 0.003790723514148109, 0.0007645345628801703,
          0.0001435746909905777, 2.566562398786942e-05, 4.436218417280813e-06 },
        { 0.1309752286653509, 0.4360331979746914, 0.9451191535603058, 0.5429768686792684,
          0.2131300934227565, 0.06607644828558357, 0.0169804365506927, 0.003790723514148109,
          0.0007645345628801703, 0.0001435746909905777, 2.566562398786942e-05 },
        { 0.02453502500565108, 0.1309752286653514, 0.4360331979746914, 0.9451191535603058,
          0.5429768686792689, 0.2131300934227565, 0.06607644828558357, 0.0169804365506927,
          0.003790723514148109, 0.0007645345628801703, 0.0001435746909905777 },
        { 0.002886791075972228, 0.02453502500565108, 0.1309752286653514, 0.4360331979746914,
          0.9451191535603041, 0.5429768686792689, 0.2131300934227565, 0.06607644828558357,
          0.0169804365506927, 0.003790723514148109, 0.0007645345628801703 },
        { 0.0002237196492846927, 0.002886791075972228, 0.02453502500565108, 0.1309752286653509,
          0.4360331979746912, 0.9451191535603036, 0.5429768686792689, 0.2131300934227565,
          0.06607644828558357, 0.0169804365506927, 0.003790723514148109 },
        { 1.224926820153627e-05, 0.0002237196492846927, 0.002886791075972228, 0.02453502500565108,
          0.1309752286653509, 0.4360331979746914, 0.9451191535603054, 0.5429768686792684,
          0.2131300934227565, 0.06607644828558357, 0.0169804365506927 },
        { 5.109390206481379e-07, 1.224926820153627e-05, 0.0002237196492846927, 0.002886791075972228,
          0.02453502500565108, 0.1309752286653509, 0.4360331979746914, 0.9451191535603058,
          0.5429768686792684, 0.213130093422756, 0.06607644828558357 },
        { 1.739106880727093e-08, 5.109390206481379e-07, 1.224926820153627e-05,
          0.0002237196492846927, 0.002886791075972228, 0.02453502500565086, 0.1309752286653509,
          0.4360331979746914, 0.9451191535603041, 0.5429768686792684, 0.2131300934227565 },
        { 5.111255862999542e-10, 1.739106880727093e-08, 5.109390206481379e-07,
          1.224926820153627e-05, 0.0002237196492846927, 0.002886791075972228, 0.02453502500565108,
          0.1309752286653516, 0.4360331979746914, 0.9451191535603058, 0.5429768686792684 },
        { 1.354649725726631e-11, 5.111255862999542e-10, 1.739106880727093e-08,
          5.109390206481379e-07, 1.224926820153627e-05, 0.0002237196492846927, 0.002886791075972228,
          0.02453502500565108, 0.1309752286653509, 0.4360331979746914, 0.9451191535603054 }
    };

    m_pDoc->SetValue(3, 0, 0, fParameter[0]); // D1
    m_pDoc->SetValue(3, 1, 0, fParameter[0]); // D2
    aPos.Set(2, 0, 0); // C1
    m_pDoc->SetString(aPos, "=POWER(1.5;D1)"); // C1
    aPos.Set(2, 1, 0); // C2
    m_pDoc->SetString(aPos, "=POWER(1.5;D2)"); // C2
    for (SCROW nRow = 0; nRow < 5;
         ++nRow) // Set A1:A5  = SQRT(C1*9/10), and A6:A10 = -SQRT(C1*9/10)
    {
        aPos.Set(0, nRow, 0);
        m_pDoc->SetString(aPos, "=SQRT(C1*9/10)");
        aPos.Set(0, nRow + 5, 0);
        m_pDoc->SetString(aPos, "=-SQRT(C1*9/10)");
    }

    for (SCROW nRow = 0; nRow < 10;
         ++nRow) // Set B1:B10  = SQRT(C2*19/20), and B11:B20 = -SQRT(C2*19/20)
    {
        aPos.Set(1, nRow, 0);
        m_pDoc->SetString(aPos, "=SQRT(C2*19/20)");
        aPos.Set(1, nRow + 10, 0);
        m_pDoc->SetString(aPos, "=-SQRT(C2*19/20)");
    }

    aPos.Set(4, 0, 0); // E1
    m_pDoc->SetString(aPos, "=FTEST(A1:A10;B1:B20)");
    aPos.Set(4, 1, 0); // E2
    m_pDoc->SetString(aPos, "=FTEST(B1:B20;A1:A10)");

    ScAddress aPosRev(4, 1, 0); // E2
    aPos.Set(4, 0, 0); // E1

    for (size_t nFirstIdx = 0; nFirstIdx < nNumParams; ++nFirstIdx)
    {
        m_pDoc->SetValue(3, 0, 0, fParameter[nFirstIdx]); // Set D1
        for (size_t nSecondIdx = 0; nSecondIdx < nNumParams; ++nSecondIdx)
        {
            m_pDoc->SetValue(3, 1, 0, fParameter[nSecondIdx]); // Set D2
            double fExpected = fResults[nFirstIdx][nSecondIdx];
            // Here a dynamic error limit is used. This is to handle correctly when the expected value is lower than the fixed error limit of 10e-5
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", fExpected,
                                                 m_pDoc->GetValue(aPos),
                                                 std::min(10e-5, fExpected * 0.0001));
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", fExpected,
                                                 m_pDoc->GetValue(aPosRev),
                                                 std::min(10e-5, fExpected * 0.0001));
        }
    }
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncFTESTBug)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "FTest");

    ScAddress aPos(9, 0, 0);
    m_pDoc->SetString(aPos, "=FTEST(H1:H3;I1:I3)");

    m_pDoc->SetValue(7, 0, 0, 9.0); // H1
    m_pDoc->SetValue(7, 1, 0, 8.0); // H2
    m_pDoc->SetValue(7, 2, 0, 6.0); // H3
    m_pDoc->SetValue(8, 0, 0, 5.0); // I1
    m_pDoc->SetValue(8, 1, 0, 7.0); // I2
    // tdf#93329
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.9046,
                                         m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncCHITEST)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "ChiTest");

    ScAddress aPos(6, 0, 0);
    // 2x2 matrices test
    m_pDoc->SetString(aPos, "=CHITEST(A1:B2;D1:E2)");
    OUString aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return Err:502 for matrices with empty cells",
                                 OUString("Err:502"), aVal);

    m_pDoc->SetValue(0, 0, 0, 1.0); // A1
    m_pDoc->SetValue(0, 1, 0, 2.0); // A2
    m_pDoc->SetValue(1, 0, 0, 2.0); // B1
    m_pDoc->SetValue(1, 1, 0, 1.0); // B2
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return Err:502 for matrix with empty cells",
                                 OUString("Err:502"), aVal);

    m_pDoc->SetValue(3, 0, 0, 2.0); // D1
    m_pDoc->SetValue(3, 1, 0, 3.0); // D2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.3613,
                                         m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->SetValue(4, 1, 0, 1.0); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.3613,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(4, 0, 0, 3.0); // E1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.2801,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(4, 0, 0, 0.0); // E1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return #DIV/0 for expected values of 0",
                                 OUString("#DIV/0!"), aVal);
    m_pDoc->SetValue(4, 0, 0, 3.0); // E1
    m_pDoc->SetValue(1, 1, 0, 0.0); // B2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1410,
                                         m_pDoc->GetValue(aPos), 10e-4);

    // 3x3 matrices test
    m_pDoc->SetString(aPos, "=CHITEST(A1:C3;D1:F3)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.7051,
                                         m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->SetValue(2, 0, 0, 3.0); // C1
    m_pDoc->SetValue(2, 1, 0, 2.0); // C2
    m_pDoc->SetValue(2, 2, 0, 3.0); // C3
    m_pDoc->SetValue(0, 2, 0, 4.0); // A3
    m_pDoc->SetValue(1, 2, 0, 2.0); // B3
    m_pDoc->SetValue(5, 0, 0, 1.0); // F1
    m_pDoc->SetValue(5, 1, 0, 2.0); // F2
    m_pDoc->SetValue(5, 2, 0, 3.0); // F3
    m_pDoc->SetValue(3, 2, 0, 3.0); // D3
    m_pDoc->SetValue(4, 2, 0, 1.0); // E3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1117,
                                         m_pDoc->GetValue(aPos), 10e-4);

    // test with strings
    m_pDoc->SetString(4, 2, 0, "a"); // E3
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return Err:502 for matrices with strings",
                                 OUString("Err:502"), aVal);
    m_pDoc->SetString(1, 2, 0, "a"); // B3
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return Err:502 for matrices with strings",
                                 OUString("Err:502"), aVal);
    m_pDoc->SetValue(4, 2, 0, 1.0); // E3
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return Err:502 for matrices with strings",
                                 OUString("Err:502"), aVal);
    m_pDoc->SetValue(1, 2, 0, 2.0); // B3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1117,
                                         m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->SetValue(4, 1, 0, 5.0); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0215,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 2, 0, 1.0); // B3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0328,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(5, 0, 0, 3.0); // F1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1648,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(0, 1, 0, 3.0); // A2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1870,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(3, 1, 0, 5.0); // D2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1377,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(3, 2, 0, 4.0); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1566,
                                         m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->SetValue(0, 0, 0, 0.0); // A1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0868,
                                         m_pDoc->GetValue(aPos), 10e-4);

    // no convergence error
    m_pDoc->SetValue(4, 0, 0, 1.0E308); // E1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL(OUString("Err:523"), aVal);
    m_pDoc->SetValue(4, 0, 0, 3.0); // E1

    // zero in all cells
    m_pDoc->SetValue(0, 1, 0, 0.0); // A2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0150,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(0, 2, 0, 0.0); // A3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0026,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 0, 0, 0.0); // B1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.00079,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(1, 2, 0, 0.0); // B3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0005,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 0, 0, 0.0); // C1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0001,
                                         m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 1, 0, 0.0); // C2
    m_pDoc->SetValue(2, 2, 0, 0.0); // C3
    m_pDoc->SetValue(3, 0, 0, 0.0); // D1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return #DIV/0! for matrices with empty",
                                 OUString("#DIV/0!"), aVal);
    m_pDoc->SetValue(3, 1, 0, 0.0); // D2
    m_pDoc->SetValue(3, 2, 0, 0.0); // D3
    m_pDoc->SetValue(4, 0, 0, 0.0); // E1
    m_pDoc->SetValue(4, 1, 0, 0.0); // E2
    m_pDoc->SetValue(4, 2, 0, 0.0); // E3
    m_pDoc->SetValue(5, 0, 0, 0.0); // F1
    m_pDoc->SetValue(5, 1, 0, 0.0); // F2
    m_pDoc->SetValue(5, 2, 0, 0.0); // F3
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return #DIV/0! for matrices with empty",
                                 OUString("#DIV/0!"), aVal);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncTTEST)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "TTest");

    ScAddress aPos(6, 0, 0);
    // type 1, mode/tails 1
    m_pDoc->SetString(aPos, "=TTEST(A1:C3;D1:F3;1;1)");
    OUString aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TTEST should return #VALUE! for empty matrices",
                                 OUString("#VALUE!"), aVal);

    m_pDoc->SetValue(0, 0, 0, 8.0); // A1
    m_pDoc->SetValue(1, 0, 0, 2.0); // B1
    m_pDoc->SetValue(3, 0, 0, 3.0); // D1
    m_pDoc->SetValue(4, 0, 0, 1.0); // E1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.18717,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 0, 0, 1.0); // C1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.18717,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 0, 0, 6.0); // F1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.45958,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 1, 0, -4.0); // A2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.45958,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(3, 1, 0, 1.0); // D2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.35524,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(1, 1, 0, 5.0); // B2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.35524,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(4, 1, 0, -2.0); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.41043,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 1, 0, -1.0); // C2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.41043,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 1, 0, -3.0); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.34990,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 2, 0, 10.0); // A3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.34990,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(3, 2, 0, 10.0); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.34686,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(1, 2, 0, 3.0); // B3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.34686,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(4, 2, 0, 9.0); // E3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.47198,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 2, 0, -5.0); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.47198,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 2, 0, 6.0); // F3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.25529,
                                         m_pDoc->GetValue(aPos), 10e-5);

    m_pDoc->SetString(1, 1, 0, "a"); // B2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.12016,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetString(4, 1, 0, "b"); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.12016,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetString(2, 2, 0, "c"); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.25030,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetString(5, 1, 0, "d"); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.19637,
                                         m_pDoc->GetValue(aPos), 10e-5);

    // type 1, mode/tails 2
    m_pDoc->SetString(aPos, "=TTEST(A1:C3;D1:F3;2;1)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.39273,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(1, 1, 0, 4.0); // B2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.39273,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(4, 1, 0, 3.0); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.43970,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 2, 0, -2.0); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.22217,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 1, 0, -10.0); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.64668,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 1, 0, 3.0); // A2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.95266,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(3, 2, 0, -1.0); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.62636,
                                         m_pDoc->GetValue(aPos), 10e-5);

    // type 2, mode/tails 2
    m_pDoc->SetString(aPos, "=TTEST(A1:C3;D1:F3;2;2)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.62549,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 1, 0, -1.0); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.94952,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 2, 0, 5.0); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.58876,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 1, 0, 2.0); // C2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.43205,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(3, 2, 0, -4.0); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.36165,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 1, 0, 1.0); // A2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.44207,
                                         m_pDoc->GetValue(aPos), 10e-5);

    // type 3, mode/tails 1
    m_pDoc->SetString(aPos, "=TTEST(A1:C3;D1:F3;1;3)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.22132,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 0, 0, 1.0); // A1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.36977,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 2, 0, -30.0); // A3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.16871,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(3, 1, 0, 5.0); // D2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.14396,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 1, 0, 2.0); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.12590,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(4, 2, 0, 2.0); // E3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.16424,
                                         m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 0, 0, -1.0); // F1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.21472,
                                         m_pDoc->GetValue(aPos), 10e-5);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncSUMX2PY2)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "SumX2PY2 Test");

    OUString aVal;
    ScAddress aPos(6, 0, 0);
    m_pDoc->SetString(aPos, "=SUMX2PY2(A1:C3;D1:F3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 0.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetValue(0, 0, 0, 1.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(3, 0, 0, 2.0); // D1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 5.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 0, 0, 2.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 5.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(4, 0, 0, 0.0); // E1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 9.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 0, 0, 3.0); // C1
    m_pDoc->SetValue(5, 0, 0, 3.0); // F1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 27.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, 10.0); // A2
    m_pDoc->SetValue(3, 1, 0, -10.0); // D2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 227.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 1, 0, -5.0); // B2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 227.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(4, 1, 0, -5.0); // E2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 277.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 1, 0, 0.0); // C2
    m_pDoc->SetValue(5, 1, 0, 0.0); // F2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 277.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 2, 0, -8.0); // A3
    m_pDoc->SetValue(3, 2, 0, 8.0); // D3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 405.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 2, 0, 0.0); // B3
    m_pDoc->SetValue(4, 2, 0, 0.0); // E3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 405.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 2, 0, 1.0); // C3
    m_pDoc->SetValue(5, 2, 0, 1.0); // F3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 407.0, m_pDoc->GetValue(aPos));

    // add some strings
    m_pDoc->SetString(4, 1, 0, "a"); // E2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 357.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(1, 1, 0, "a"); // B2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 357.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(0, 0, 0, "a"); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 352.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(3, 0, 0, "a"); // D1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 352.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetString(aPos, "=SUMX2PY2({1;2;3};{2;3;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2PY2 failed", 43.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMX2PY2({1;2;3};{2;3})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUMX2PY2 should return #VALUE! for matrices with different sizes",
                                 OUString("#VALUE!"), aVal);
    m_pDoc->SetString(aPos, "=SUMX2PY2({1;2;3})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUMX2PY2 needs two parameters", OUString("Err:511"), aVal);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncSUMX2MY2)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "SumX2MY2 Test");

    OUString aVal;
    ScAddress aPos(6, 0, 0);
    m_pDoc->SetString(aPos, "=SUMX2MY2(A1:C3;D1:F3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 0.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetValue(0, 0, 0, 10.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(3, 0, 0, -9.0); // D1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 19.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 0, 0, 2.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 19.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(4, 0, 0, 1.0); // E1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 22.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 0, 0, 3.0); // C1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 22.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(5, 0, 0, 3.0); // F1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 22.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, 10.0); // A2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 22.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(3, 1, 0, -10.0); // D2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 22.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 1, 0, -5.0); // B2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 22.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(4, 1, 0, -5.0); // E2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 22.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 1, 0, -3.0); // C2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 22.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(5, 1, 0, 3.0); // F2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 22.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 2, 0, -8.0); // A3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 22.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(3, 2, 0, 3.0); // D3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 77.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 2, 0, 2.0); // B3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 77.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(4, 2, 0, -6.0); // E3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 45.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 2, 0, -4.0); // C3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 45.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(5, 2, 0, 6.0); // F3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 25.0, m_pDoc->GetValue(aPos));

    // add some strings
    m_pDoc->SetString(5, 2, 0, "a"); // F3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 45.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(0, 2, 0, "a"); // A3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", -10.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(1, 0, 0, "a"); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", -13.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(3, 0, 0, "a"); // D1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", -32.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetString(aPos, "=SUMX2MY2({1;3;5};{0;4;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 3.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMX2MY2({1;-3;-5};{0;-4;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 3.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMX2MY2({9;5;1};{3;-3;3})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMX2MY2 failed", 80.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMX2MY2({1;2;3};{2;3})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUMX2MY2 should return #VALUE! for matrices with different sizes",
                                 OUString("#VALUE!"), aVal);
    m_pDoc->SetString(aPos, "=SUMX2MY2({1;2;3})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUMX2MY2 needs two parameters", OUString("Err:511"), aVal);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncGCD)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "GCDTest");

    OUString aVal;
    ScAddress aPos(4, 0, 0);

    m_pDoc->SetString(aPos, "=GCD(A1)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, 10.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 10.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, -2.0); // A1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return Err:502 for values less than 0",
                                 OUString("Err:502"), aVal);
    m_pDoc->SetString(0, 0, 0, "a"); // A1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return #VALUE! for a single string",
                                 OUString("#VALUE!"), aVal);

    m_pDoc->SetString(aPos, "=GCD(A1:B2)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, -12.0); // B1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return Err:502 for a matrix with values less than 0",
                                 OUString("Err:502"), aVal);
    m_pDoc->SetValue(0, 0, 0, 15.0); // A1
    m_pDoc->SetValue(0, 1, 0, 0.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 15.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 0, 0, 5.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 5.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, 10.0); // A2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 5.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 0, 0, 30.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 5.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, 20.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 10.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 1, 0, 120.0); // B2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 10.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, 80.0); // A2
    m_pDoc->SetValue(1, 0, 0, 40.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 20.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 0, 0, 45.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 5.0, m_pDoc->GetValue(aPos));

    // with floor
    m_pDoc->SetValue(1, 0, 0, 45.381); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 5.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 1, 0, 120.895); // B2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 5.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, 20.97); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 5.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, 10.15); // A2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 5.0, m_pDoc->GetValue(aPos));

    // inline array
    m_pDoc->SetString(aPos, "=GCD({3;6;9})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 3.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=GCD({150;0})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 150.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=GCD({-3;6;9})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return Err:502 for an array with values less than 0",
                                 OUString("Err:502"), aVal);
    m_pDoc->SetString(aPos, "=GCD({\"a\";6;9})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return Err:502 for an array with strings",
                                 OUString("Err:502"), aVal);

    //many inline array
    m_pDoc->SetString(aPos, "=GCD({6;6;6};{3;6;9})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 3.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=GCD({300;300;300};{150;0})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 150.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=GCD({3;6;9};{3;-6;9})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return Err:502 for an array with values less than 0",
                                 OUString("Err:502"), aVal);
    m_pDoc->SetString(aPos, "=GCD({3;6;9};{\"a\";6;9})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return Err:502 for an array with strings",
                                 OUString("Err:502"), aVal);

    // inline list of values
    m_pDoc->SetString(aPos, "=GCD(12;24;36;48;60)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 12.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=GCD(0;12;24;36;48;60)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 12.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=GCD(\"a\";1)");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return #VALUE! for an array with strings",
                                 OUString("#VALUE!"), aVal);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncLCM)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "LCMTest");

    OUString aVal;
    ScAddress aPos(4, 0, 0);

    m_pDoc->SetString(aPos, "=LCM(A1)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, 10.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 10.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, -2.0); // A1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return Err:502 for values less than 0",
                                 OUString("Err:502"), aVal);
    m_pDoc->SetString(0, 0, 0, "a"); // A1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return #VALUE! for a single string",
                                 OUString("#VALUE!"), aVal);

    m_pDoc->SetString(aPos, "=LCM(A1:B2)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 1.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, -12.0); // B1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return Err:502 for a matrix with values less than 0",
                                 OUString("Err:502"), aVal);
    m_pDoc->SetValue(0, 0, 0, 15.0); // A1
    m_pDoc->SetValue(0, 1, 0, 0.0); // A2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 0, 0, 5.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, 10.0); // A2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 30.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 0, 0, 30.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 30.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, 20.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 60.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 1, 0, 125.0); // B2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 1500.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 0, 0, 99.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 49500.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, 37.0); // A2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 1831500.0,
                                 m_pDoc->GetValue(aPos));

    // with floor
    m_pDoc->SetValue(1, 0, 0, 99.89); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 1831500.0,
                                 m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 1, 0, 11.32); // B2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 73260.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, 22.58); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 7326.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, 3.99); // A2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 198.0, m_pDoc->GetValue(aPos));

    // inline array
    m_pDoc->SetString(aPos, "=LCM({3;6;9})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 18.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=LCM({150;0})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=LCM({-3;6;9})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return Err:502 for an array with values less than 0",
                                 OUString("Err:502"), aVal);
    m_pDoc->SetString(aPos, "=LCM({\"a\";6;9})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return Err:502 for an array with strings",
                                 OUString("Err:502"), aVal);

    //many inline array
    m_pDoc->SetString(aPos, "=LCM({6;6;6};{3;6;9})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 18.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=LCM({300;300;300};{150;0})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=LCM({3;6;9};{3;-6;9})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return Err:502 for an array with values less than 0",
                                 OUString("Err:502"), aVal);
    m_pDoc->SetString(aPos, "=LCM({3;6;9};{\"a\";6;9})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return Err:502 for an array with strings",
                                 OUString("Err:502"), aVal);

    m_pDoc->SetString(aPos, "=LCM(12;24;36;48;60)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 720.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=LCM(0;12;24;36;48;60)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=LCM(\"a\";1)");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return #VALUE! for an array with strings",
                                 OUString("#VALUE!"), aVal);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncSUMSQ)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "SUMSQTest");

    ScAddress aPos(4, 0, 0);

    m_pDoc->SetString(aPos, "=SUMSQ(A1)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, 1.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 1.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, -1.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 1.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, -2.0); // A2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 1.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetString(aPos, "=SUMSQ(A1:A3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 5.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 0, 0, 3.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 5.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ(A1:C3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 14.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 1, 0, -4.0); // B2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 30.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(1, 2, 0, "a"); // B3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 30.0,
                                 m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 2, 0, 0.0); // B3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 30.0,
                                 m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 2, 0, 6.0); // A3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 66.0,
                                 m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 0, 0, -5.0); // C1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 91.0,
                                 m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 1, 0, 3.0); // C2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 100.0,
                                 m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 2, 0, 2.0); // C3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 104.0,
                                 m_pDoc->GetValue(aPos));

    // inline array
    m_pDoc->SetString(aPos, "=SUMSQ({1;2;3})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 14.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ({3;6;9})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 126.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ({15;0})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 225.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ({-3;3;1})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 19.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ({\"a\";-4;-5})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 41.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetString(aPos, "=SUMSQ({2;3};{4;5})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 54.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ({-3;3;1};{-1})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 20.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ({-4};{1;4;2};{-5;7};{9})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 192.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ({-2;2};{1};{-1};{0;0;0;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 26.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetString(aPos, "=SUMSQ(4;1;-3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 26.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ(0;5;13;-7;-4)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 259.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ(0;12;24;36;48;60)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 7920.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ(0;-12;-24;36;-48;60)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ for failed", 7920.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=SUMSQ(\"a\";1;\"d\";-4;2)");
    OUString aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUMSQ should return #VALUE! for an array with strings",
                                 OUString("#VALUE!"), aVal);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncMDETERM)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "MDETERM_test");
    ScAddress aPos(8, 0, 0);
    static constexpr std::u16string_view aColCodes(u"ABCDEFGH");
    OUStringBuffer aFormulaBuffer("=MDETERM(A1:B2)");
    for (SCSIZE nSize = 3; nSize <= 8; nSize++)
    {
        double fVal = 1.0;
        // Generate a singular integer matrix
        for (SCROW nRow = 0; nRow < static_cast<SCROW>(nSize); nRow++)
        {
            for (SCCOL nCol = 0; nCol < static_cast<SCCOL>(nSize); nCol++)
            {
                m_pDoc->SetValue(nCol, nRow, 0, fVal);
                fVal += 1.0;
            }
        }
        aFormulaBuffer[12] = aColCodes[nSize - 1];
        aFormulaBuffer[13] = static_cast<sal_Unicode>('0' + nSize);
        m_pDoc->SetString(aPos, aFormulaBuffer.toString());

#if SAL_TYPES_SIZEOFPOINTER == 4
        // On crappy 32-bit targets, presumably without extended precision on
        // interim results or optimization not catching it, this test fails
        // when comparing to 0.0, so have a narrow error margin. See also
        // commit message of 8140309d636d4a870875f2dd75ed3dfff2c0fbaf
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "Calculation of MDETERM incorrect for singular integer matrix", 0.0,
            m_pDoc->GetValue(aPos), 1e-12);
#else
        // Even on one (and only one) x86_64 target the result was
        // 6.34413156928661e-17 instead of 0.0 (tdf#99730) so lower the bar to
        // 10e-14.
        // Then again on aarch64, ppc64* and s390x it also fails.
        // Sigh... why do we even test this? The original complaint in tdf#32834
        // was about -9.51712667007776E-016
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "Calculation of MDETERM incorrect for singular integer matrix", 0.0,
            m_pDoc->GetValue(aPos), 1e-14);
#endif
    }

    int const aVals[] = { 23, 31, 13, 12, 34, 64, 34, 31, 98, 32, 33, 63, 45, 54, 65, 76 };
    int nIdx = 0;
    for (SCROW nRow = 0; nRow < 4; nRow++)
        for (SCCOL nCol = 0; nCol < 4; nCol++)
            m_pDoc->SetValue(nCol, nRow, 0, static_cast<double>(aVals[nIdx++]));
    m_pDoc->SetString(aPos, "=MDETERM(A1:D4)");
    // Following test is conservative in the sense that on Linux x86_64 the error is less that 1.0E-9
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
        "Calculation of MDETERM incorrect for non-singular integer matrix", -180655.0,
        m_pDoc->GetValue(aPos), 1.0E-6);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testFormulaErrorPropagation)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Sheet1");

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    ScAddress aPos, aPos2;
    constexpr OUString aTRUE(u"TRUE"_ustr);
    constexpr OUString aFALSE(u"FALSE"_ustr);

    aPos.Set(0, 0, 0); // A1
    m_pDoc->SetValue(aPos, 1.0);
    aPos.IncCol(); // B1
    m_pDoc->SetValue(aPos, 2.0);
    aPos.IncCol();

    aPos.IncRow(); // C2
    m_pDoc->SetString(aPos, "=ISERROR(A1:B1+3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE,
                                 m_pDoc->GetString(aPos));

    aPos.IncRow(); // C3
    m_pDoc->SetString(aPos, "=ISERROR(A1:B1+{3})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE,
                                 m_pDoc->GetString(aPos));
    aPos.IncRow(); // C4
    aPos2 = aPos;
    aPos2.IncCol(); // D4
    m_pDoc->InsertMatrixFormula(aPos.Col(), aPos.Row(), aPos2.Col(), aPos2.Row(), aMark,
                                "=ISERROR(A1:B1+{3})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE,
                                 m_pDoc->GetString(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos2.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE,
                                 m_pDoc->GetString(aPos2));

    aPos.IncRow(); // C5
    m_pDoc->SetString(aPos, "=ISERROR({1;\"x\"}+{3;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE,
                                 m_pDoc->GetString(aPos));
    aPos.IncRow(); // C6
    aPos2 = aPos;
    aPos2.IncCol(); // D6
    m_pDoc->InsertMatrixFormula(aPos.Col(), aPos.Row(), aPos2.Col(), aPos2.Row(), aMark,
                                "=ISERROR({1;\"x\"}+{3;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE,
                                 m_pDoc->GetString(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos2.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE,
                                 m_pDoc->GetString(aPos2));

    aPos.IncRow(); // C7
    m_pDoc->SetString(aPos, "=ISERROR({\"x\";2}+{3;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE,
                                 m_pDoc->GetString(aPos));
    aPos.IncRow(); // C8
    aPos2 = aPos;
    aPos2.IncCol(); // D8
    m_pDoc->InsertMatrixFormula(aPos.Col(), aPos.Row(), aPos2.Col(), aPos2.Row(), aMark,
                                "=ISERROR({\"x\";2}+{3;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE,
                                 m_pDoc->GetString(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos2.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE,
                                 m_pDoc->GetString(aPos2));

    aPos.IncRow(); // C9
    m_pDoc->SetString(aPos, "=ISERROR(({1;\"x\"}+{3;4})-{5;6})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE,
                                 m_pDoc->GetString(aPos));
    aPos.IncRow(); // C10
    aPos2 = aPos;
    aPos2.IncCol(); // D10
    m_pDoc->InsertMatrixFormula(aPos.Col(), aPos.Row(), aPos2.Col(), aPos2.Row(), aMark,
                                "=ISERROR(({1;\"x\"}+{3;4})-{5;6})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE,
                                 m_pDoc->GetString(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos2.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE,
                                 m_pDoc->GetString(aPos2));

    aPos.IncRow(); // C11
    m_pDoc->SetString(aPos, "=ISERROR(({\"x\";2}+{3;4})-{5;6})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE,
                                 m_pDoc->GetString(aPos));
    aPos.IncRow(); // C12
    aPos2 = aPos;
    aPos2.IncCol(); // D12
    m_pDoc->InsertMatrixFormula(aPos.Col(), aPos.Row(), aPos2.Col(), aPos2.Row(), aMark,
                                "=ISERROR(({\"x\";2}+{3;4})-{5;6})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE,
                                 m_pDoc->GetString(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aPos2.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE,
                                 m_pDoc->GetString(aPos2));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testTdf97369)
{
    const SCROW TOTAL_ROWS = 330;
    const SCROW ROW_RANGE = 10;
    const SCROW START1 = 9;
    const SCROW END1 = 159;
    const SCROW START2 = 169;
    const SCROW END2 = 319;

    const double SHIFT1 = 200;
    const double SHIFT2 = 400;

    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "tdf97369"));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    // set up columns A, B, C
    for (SCROW i = 0; i < TOTAL_ROWS; ++i)
    {
        m_pDoc->SetValue(ScAddress(0, i, 0), i); // A
        m_pDoc->SetValue(ScAddress(1, i, 0), i + SHIFT1); // B
        m_pDoc->SetValue(ScAddress(2, i, 0), i + SHIFT2); // C
    }

    const ColumnTest columnTest(m_pDoc, TOTAL_ROWS, START1, END1, START2, END2);

    auto lExpectedinD = [=](SCROW n) { return 3.0 * (n - START1) + SHIFT1 + SHIFT2; };
    columnTest(3, "=SUM(A1:C1)", lExpectedinD);

    auto lExpectedinE = [=](SCROW) { return SHIFT1 + SHIFT2; };
    columnTest(4, "=SUM(A$1:C$1)", lExpectedinE);

    auto lExpectedinF = [](SCROW n) { return ((2 * n + 1 - ROW_RANGE) * ROW_RANGE) / 2.0; };
    columnTest(5, "=SUM(A1:A10)", lExpectedinF);

    auto lExpectedinG = [](SCROW n) { return ((n + 1) * n) / 2.0; };
    columnTest(6, "=SUM(A$1:A10)", lExpectedinG);

    auto lExpectedinH = [=](SCROW n) {
        return 3.0 * (((2 * n + 1 - ROW_RANGE) * ROW_RANGE) / 2) + ROW_RANGE * (SHIFT1 + SHIFT2);
    };
    columnTest(7, "=SUM(A1:C10)", lExpectedinH);

    auto lExpectedinI = [=](SCROW) {
        return 3.0 * (((2 * START1 + 1 - ROW_RANGE) * ROW_RANGE) / 2)
               + ROW_RANGE * (SHIFT1 + SHIFT2);
    };
    columnTest(8, "=SUM(A$1:C$10)", lExpectedinI);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testTdf97587)
{
    const SCROW TOTAL_ROWS = 150;
    const SCROW ROW_RANGE = 10;

    CPPUNIT_ASSERT_MESSAGE("failed to insert sheet", m_pDoc->InsertTab(0, "tdf97587"));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    std::set<SCROW> emptyCells = { 0, 100 };
    for (SCROW i = 0; i < ROW_RANGE; ++i)
    {
        emptyCells.insert(i + TOTAL_ROWS / 3);
        emptyCells.insert(i + TOTAL_ROWS);
    }

    // set up columns A
    for (SCROW i = 0; i < TOTAL_ROWS; ++i)
    {
        if (emptyCells.find(i) != emptyCells.end())
            continue;
        m_pDoc->SetValue(ScAddress(0, i, 0), 1.0);
    }

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScMarkData aMark(m_pDoc->GetSheetLimits());

    ScAddress aPos(1, 0, 0);
    m_pDoc->SetString(aPos, "=SUM(A1:A10)");

    // Copy formula cell to clipboard.
    ScClipParam aClipParam(aPos, false);
    aMark.SetMarkArea(aPos);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    // Paste it to first range.
    ScRange aDestRange(1, 1, 0, 1, TOTAL_ROWS + ROW_RANGE, 0);
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::CONTENTS, nullptr, &aClipDoc);

    // Check the formula results in column B.
    for (SCROW i = 0; i < TOTAL_ROWS + 1; ++i)
    {
        int k = std::count_if(emptyCells.begin(), emptyCells.end(),
                              [=](SCROW n) { return (i <= n && n < i + ROW_RANGE); });
        double fExpected = ROW_RANGE - k;
        ASSERT_DOUBLES_EQUAL(fExpected, m_pDoc->GetValue(ScAddress(1, i, 0)));
    }
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testTdf93415)
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab(0, "Sheet1"));

    ScCalcConfig aConfig;
    aConfig.SetStringRefSyntax(formula::FormulaGrammar::CONV_XL_R1C1);
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();

    ScAddress aPos(0, 0, 0);
    m_pDoc->SetString(aPos, "=ADDRESS(1;1;;;\"Sheet1\")");

    // Without the fix in place, this would have failed with
    // - Expected: Sheet1!$A$1
    // - Actual  : Sheet1.$A$1
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1!$A$1"), m_pDoc->GetString(aPos));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testTdf132519)
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab(0, "Sheet1"));

    ScCalcConfig aConfig;
    aConfig.SetStringRefSyntax(formula::FormulaGrammar::CONV_XL_R1C1);
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();

    m_pDoc->SetString(2, 0, 0, "X");
    m_pDoc->SetString(1, 0, 0, "=CELL(\"ADDRESS\"; C1)");
    m_pDoc->SetString(0, 0, 0, "=INDIRECT(B1)");

    // Without the fix in place, this test would have failed with
    // - Expected: X
    // - Actual  : #REF!
    CPPUNIT_ASSERT_EQUAL(OUString("X"), m_pDoc->GetString(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(OUString("R1C3"), m_pDoc->GetString(1, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testTdf127334)
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab(0, "Sheet1"));

    m_pDoc->SetString(
        0, 0, 0,
        "= (((DATE(2019;9;17) + TIME(0;0;1)) - DATE(2019;9;17)) - TIME(0;0;1))/TIME(0;0;1)");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 2.32e-07
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(0, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testTdf100818)
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab(0, "Sheet1"));

    //Insert local range name
    ScRangeData* pLocal = new ScRangeData(*m_pDoc, "local", "$Sheet1.$A$1");
    std::unique_ptr<ScRangeName> pLocalRangeName(new ScRangeName);
    pLocalRangeName->insert(pLocal);
    m_pDoc->SetRangeName(0, std::move(pLocalRangeName));

    m_pDoc->SetValue(0, 0, 0, 1.0);

    CPPUNIT_ASSERT(m_pDoc->InsertTab(1, "Sheet2"));

    m_pDoc->SetString(0, 0, 1, "=INDIRECT(\"Sheet1.local\")");

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : #REF!
    CPPUNIT_ASSERT_EQUAL(OUString("1"), m_pDoc->GetString(0, 0, 1));

    m_pDoc->DeleteTab(1);
    m_pDoc->SetRangeName(0, nullptr); // Delete the names.
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testMatConcat)
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab(0, "Test"));

    for (SCCOL nCol = 0; nCol < 10; ++nCol)
    {
        for (SCROW nRow = 0; nRow < 10; ++nRow)
        {
            m_pDoc->SetValue(ScAddress(nCol, nRow, 0), nCol * nRow);
        }
    }

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 12, 9, 21, aMark, "=A1:J10&A1:J10");

    for (SCCOL nCol = 0; nCol < 10; ++nCol)
    {
        for (SCROW nRow = 12; nRow < 22; ++nRow)
        {
            OUString aStr = m_pDoc->GetString(ScAddress(nCol, nRow, 0));
            CPPUNIT_ASSERT_EQUAL(OUString(OUString::number(nCol * (nRow - 12))
                                          + OUString::number(nCol * (nRow - 12))),
                                 aStr);
        }
    }

    { // Data in A12:B16
        std::vector<std::vector<const char*>> aData = {
            { "q", "w" }, { "a", "" }, { "", "x" }, { "", "" }, { "e", "r" },
        };

        ScAddress aPos(0, 11, 0);
        ScRange aRange = insertRangeData(m_pDoc, aPos, aData);
        CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);
    }
    // Matrix formula in C17:C21
    m_pDoc->InsertMatrixFormula(2, 16, 2, 20, aMark, "=A12:A16&B12:B16");
    // Check proper concatenation including empty cells.
    OUString aStr;
    ScAddress aPos(2, 16, 0);
    aStr = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL(OUString("qw"), aStr);
    aPos.IncRow();
    aStr = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), aStr);
    aPos.IncRow();
    aStr = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL(OUString("x"), aStr);
    aPos.IncRow();
    aStr = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL(OUString(), aStr);
    aPos.IncRow();
    aStr = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL(OUString("er"), aStr);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testMatConcatReplication)
{
    // if one of the matrices is a one column or row matrix
    // the matrix is replicated across the larger matrix
    CPPUNIT_ASSERT(m_pDoc->InsertTab(0, "Test"));

    for (SCCOL nCol = 0; nCol < 10; ++nCol)
    {
        for (SCROW nRow = 0; nRow < 10; ++nRow)
        {
            m_pDoc->SetValue(ScAddress(nCol, nRow, 0), nCol * nRow);
        }
    }

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 12, 9, 21, aMark, "=A1:J10&A1:J1");

    for (SCCOL nCol = 0; nCol < 10; ++nCol)
    {
        for (SCROW nRow = 12; nRow < 22; ++nRow)
        {
            OUString aStr = m_pDoc->GetString(ScAddress(nCol, nRow, 0));
            CPPUNIT_ASSERT_EQUAL(OUString(OUString::number(nCol * (nRow - 12)) + "0"), aStr);
        }
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testRefR1C1WholeCol)
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab(0, "Test"));

    ScAddress aPos(1, 1, 1);
    ScCompiler aComp(*m_pDoc, aPos, FormulaGrammar::GRAM_ENGLISH_XL_R1C1);
    std::unique_ptr<ScTokenArray> pTokens(aComp.CompileString("=C[10]"));
    sc::TokenStringContext aCxt(*m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH);
    OUString aFormula = pTokens->CreateString(aCxt, aPos);

    CPPUNIT_ASSERT_EQUAL(OUString("L:L"), aFormula);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testRefR1C1WholeRow)
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab(0, "Test"));

    ScAddress aPos(1, 1, 1);
    ScCompiler aComp(*m_pDoc, aPos, FormulaGrammar::GRAM_ENGLISH_XL_R1C1);
    std::unique_ptr<ScTokenArray> pTokens(aComp.CompileString("=R[3]"));
    sc::TokenStringContext aCxt(*m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH);
    OUString aFormula = pTokens->CreateString(aCxt, aPos);

    CPPUNIT_ASSERT_EQUAL(OUString("5:5"), aFormula);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testSingleCellCopyColumnLabel)
{
    ScDocOptions aOptions = m_pDoc->GetDocOptions();
    aOptions.SetLookUpColRowNames(true);
    m_pDoc->SetDocOptions(aOptions);
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(0, 0, 0, "a");
    m_pDoc->SetValue(0, 1, 0, 1.0);
    m_pDoc->SetValue(0, 2, 0, 2.0);
    m_pDoc->SetValue(0, 3, 0, 3.0);
    m_pDoc->SetString(1, 1, 0, "='a'");

    double nVal = m_pDoc->GetValue(1, 1, 0);
    ASSERT_DOUBLES_EQUAL(1.0, nVal);

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(1, 1, 0), &aClipDoc);
    pasteOneCellFromClip(m_pDoc, ScRange(1, 2, 0), &aClipDoc);
    nVal = m_pDoc->GetValue(1, 2, 0);
    ASSERT_DOUBLES_EQUAL(2.0, nVal);

    m_pDoc->DeleteTab(0);
}

// Significant whitespace operator intersection in Excel syntax, tdf#96426
CPPUNIT_TEST_FIXTURE(TestFormula2, testIntersectionOpExcel)
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab(0, "Test"));

    ScRangeName* pGlobalNames = m_pDoc->GetRangeName();
    // Horizontal cell range covering C2.
    pGlobalNames->insert(new ScRangeData(*m_pDoc, "horz", "$B$2:$D$2"));
    // Vertical cell range covering C2.
    pGlobalNames->insert(new ScRangeData(*m_pDoc, "vert", "$C$1:$C$3"));
    // Data in C2.
    m_pDoc->SetValue(2, 1, 0, 1.0);

    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_A1);

    // Choose formula positions that don't intersect with those data ranges.
    ScAddress aPos(0, 3, 0);
    m_pDoc->SetString(aPos, "=B2:D2 C1:C3");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A4 intersecting references failed", 1.0, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=horz vert");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A5 intersecting named expressions failed", 1.0,
                                 m_pDoc->GetValue(aPos));
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=(horz vert)*2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A6 calculating with intersecting named expressions failed", 2.0,
                                 m_pDoc->GetValue(aPos));
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=2*(horz vert)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A7 calculating with intersecting named expressions failed", 2.0,
                                 m_pDoc->GetValue(aPos));

    m_pDoc->DeleteTab(0);
}

//Test Subtotal and Aggregate during hide rows #tdf93171
CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncRowsHidden)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Test");
    m_pDoc->SetValue(0, 0, 0, 1); //A1
    m_pDoc->SetValue(0, 1, 0, 2); //A2
    m_pDoc->SetValue(0, 2, 0, 4); //A3
    m_pDoc->SetValue(0, 3, 0, 8); //A4
    m_pDoc->SetValue(0, 4, 0, 16); //A5
    m_pDoc->SetValue(0, 5, 0, 32); //A6

    ScAddress aPos(0, 6, 0);
    m_pDoc->SetString(aPos, "=SUBTOTAL(109; A1:A6)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUBTOTAL failed", 63.0, m_pDoc->GetValue(aPos));
    //Hide row 1
    m_pDoc->SetRowHidden(0, 0, 0, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUBTOTAL failed", 62.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetRowHidden(0, 0, 0, false);
    //Hide row 2 and 3
    m_pDoc->SetRowHidden(1, 2, 0, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUBTOTAL failed", 57.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetRowHidden(1, 2, 0, false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUBTOTAL failed", 63.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetString(aPos, "=AGGREGATE(9; 5; A1:A6)"); //9=SUM 5=Ignore only hidden rows
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of AGGREGATE failed", 63.0, m_pDoc->GetValue(aPos));
    //Hide row 1
    m_pDoc->SetRowHidden(0, 0, 0, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of AGGREGATE failed", 62.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetRowHidden(0, 0, 0, false);
    //Hide rows 3 to 5
    m_pDoc->SetRowHidden(2, 4, 0, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of AGGREGATE failed", 35.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetRowHidden(2, 4, 0, false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of AGGREGATE failed", 63.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetString(aPos, "=SUM(A1:A6)");
    m_pDoc->SetRowHidden(2, 4, 0, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUM failed", 63.0, m_pDoc->GetValue(aPos));

    m_pDoc->DeleteTab(0);
}

// Test COUNTIFS, SUMIFS, AVERAGEIFS in array context.
CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncSUMIFS)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Test");

    // Data in A1:B7, query in A9:A11
    std::vector<std::vector<const char*>> aData = {
        { "a", "1" },  { "b", "2" },  { "c", "4" },  { "d", "8" },
        { "a", "16" }, { "b", "32" }, { "c", "64" }, { "" }, // {} doesn't work with some compilers
        { "a" },       { "b" },       { "c" },
    };

    insertRangeData(m_pDoc, ScAddress(0, 0, 0), aData);

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    // Matrix formula in C8:C10 with SUMIFS
    m_pDoc->InsertMatrixFormula(2, 7, 2, 9, aMark, "=SUMIFS(B1:B7;A1:A7;A9:A11)");
    // Matrix formula in D8:D10 with COUNTIFS
    m_pDoc->InsertMatrixFormula(3, 7, 3, 9, aMark, "=COUNTIFS(A1:A7;A9:A11)");
    // Matrix formula in E8:E10 with AVERAGEIFS
    m_pDoc->InsertMatrixFormula(4, 7, 4, 9, aMark, "=AVERAGEIFS(B1:B7;A1:A7;A9:A11)");

    {
        // Result B1+B5, B2+B6, B3+B7 and counts and averages.
        std::vector<std::vector<const char*>> aCheck
            = { { "17", "2", "8.5" }, { "34", "2", "17" }, { "68", "2", "34" } };
        bool bGood = checkOutput(m_pDoc, ScRange(2, 7, 0, 4, 9, 0), aCheck,
                                 "SUMIFS, COUNTIFS and AVERAGEIFS in array context");
        CPPUNIT_ASSERT_MESSAGE("SUMIFS, COUNTIFS or AVERAGEIFS in array context failed", bGood);
    }

    // Matrix formula in G8:G10 with SUMIFS and reference list arrays.
    m_pDoc->InsertMatrixFormula(6, 7, 6, 9, aMark,
                                "=SUMIFS(OFFSET(B1;ROW(1:3);0;2);OFFSET(B1;ROW(1:3);0;2);\">4\")");
    // Matrix formula in H8:H10 with COUNTIFS and reference list arrays.
    m_pDoc->InsertMatrixFormula(7, 7, 7, 9, aMark, "=COUNTIFS(OFFSET(B1;ROW(1:3);0;2);\">4\")");
    // Matrix formula in I8:I10 with AVERAGEIFS and reference list arrays.
    m_pDoc->InsertMatrixFormula(
        8, 7, 8, 9, aMark, "=AVERAGEIFS(OFFSET(B1;ROW(1:3);0;2);OFFSET(B1;ROW(1:3);0;2);\">4\")");

    {
        // Result sums, counts and averages.
        std::vector<std::vector<const char*>> aCheck
            = { { "0", "0", "#DIV/0!" }, { "8", "1", "8" }, { "24", "2", "12" } };
        bool bGood = checkOutput(m_pDoc, ScRange(6, 7, 0, 8, 9, 0), aCheck,
                                 "SUMIFS, COUNTIFS and AVERAGEIFS with reference list arrays");
        CPPUNIT_ASSERT_MESSAGE("SUMIFS, COUNTIFS or AVERAGEIFS with reference list arrays failed",
                               bGood);
    }

    // Matrix formula in K8:K10 with SUMIFS and reference list array condition
    // and "normal" data range.
    m_pDoc->InsertMatrixFormula(10, 7, 10, 9, aMark,
                                "=SUMIFS(B1:B2;OFFSET(B1;ROW(1:3);0;2);\">4\")");
    // Matrix formula in L8:L10 with AVERAGEIFS and reference list array
    // condition and "normal" data range.
    m_pDoc->InsertMatrixFormula(11, 7, 11, 9, aMark,
                                "=AVERAGEIFS(B1:B2;OFFSET(B1;ROW(1:3);0;2);\">4\")");

    {
        // Result sums and averages.
        std::vector<std::vector<const char*>> aCheck
            = { { "0", "#DIV/0!" }, { "2", "2" }, { "3", "1.5" } };
        bool bGood = checkOutput(
            m_pDoc, ScRange(10, 7, 0, 11, 9, 0), aCheck,
            "SUMIFS, COUNTIFS and AVERAGEIFS with reference list array and normal range");
        CPPUNIT_ASSERT_MESSAGE(
            "SUMIFS, COUNTIFS or AVERAGEIFS with reference list array and normal range failed",
            bGood);
    }

    // Matrix formula in G18:G20 with SUMIFS and reference list arrays and a
    // "normal" criteria range.
    m_pDoc->InsertMatrixFormula(
        6, 17, 6, 19, aMark,
        "=SUMIFS(OFFSET(B1;ROW(1:3);0;2);OFFSET(B1;ROW(1:3);0;2);\">4\";B1:B2;\">1\")");
    // Matrix formula in H18:H20 with COUNTIFS and reference list arrays and a
    // "normal" criteria range.
    m_pDoc->InsertMatrixFormula(7, 17, 7, 19, aMark,
                                "=COUNTIFS(OFFSET(B1;ROW(1:3);0;2);\">4\";B1:B2;\">1\")");
    // Matrix formula in I18:I20 with AVERAGEIFS and reference list arrays and
    // a "normal" criteria range.
    m_pDoc->InsertMatrixFormula(
        8, 17, 8, 19, aMark,
        "=AVERAGEIFS(OFFSET(B1;ROW(1:3);0;2);OFFSET(B1;ROW(1:3);0;2);\">4\";B1:B2;\">1\")");

    {
        // Result sums, counts and averages.
        std::vector<std::vector<const char*>> aCheck
            = { { "0", "0", "#DIV/0!" }, { "8", "1", "8" }, { "16", "1", "16" } };
        bool bGood = checkOutput(m_pDoc, ScRange(6, 17, 0, 8, 19, 0), aCheck,
                                 "SUMIFS, COUNTIFS and AVERAGEIFS with reference list arrays and a "
                                 "normal criteria range");
        CPPUNIT_ASSERT_MESSAGE("SUMIFS, COUNTIFS or AVERAGEIFS with reference list arrays and a "
                               "normal criteria range failed",
                               bGood);
    }

    // Matrix formula in K18:K20 with SUMIFS and reference list array condition
    // and "normal" data range and a "normal" criteria range.
    m_pDoc->InsertMatrixFormula(10, 17, 10, 19, aMark,
                                "=SUMIFS(B1:B2;OFFSET(B1;ROW(1:3);0;2);\">4\";B1:B2;\">1\")");
    // Matrix formula in L18:L20 with AVERAGEIFS and reference list array
    // condition and "normal" data range and a "normal" criteria range.
    m_pDoc->InsertMatrixFormula(11, 17, 11, 19, aMark,
                                "=AVERAGEIFS(B1:B2;OFFSET(B1;ROW(1:3);0;2);\">4\";B1:B2;\">1\")");

    {
        // Result sums and averages.
        std::vector<std::vector<const char*>> aCheck
            = { { "0", "#DIV/0!" }, { "2", "2" }, { "2", "2" } };
        bool bGood = checkOutput(m_pDoc, ScRange(10, 17, 0, 11, 19, 0), aCheck,
                                 "SUMIFS, COUNTIFS and AVERAGEIFS with reference list array and "
                                 "normal data and criteria range");
        CPPUNIT_ASSERT_MESSAGE("SUMIFS, COUNTIFS or AVERAGEIFS with reference list array and "
                               "normal data and criteria range failed",
                               bGood);
    }

    // Same, but swapped normal and array criteria.

    // Matrix formula in G28:G30 with SUMIFS and reference list arrays and a
    // "normal" criteria range, swapped.
    m_pDoc->InsertMatrixFormula(
        6, 27, 6, 29, aMark,
        "=SUMIFS(OFFSET(B1;ROW(1:3);0;2);B1:B2;\">1\";OFFSET(B1;ROW(1:3);0;2);\">4\")");
    // Matrix formula in H28:H30 with COUNTIFS and reference list arrays and a
    // "normal" criteria range, swapped.
    m_pDoc->InsertMatrixFormula(7, 27, 7, 29, aMark,
                                "=COUNTIFS(B1:B2;\">1\";OFFSET(B1;ROW(1:3);0;2);\">4\")");
    // Matrix formula in I28:I30 with AVERAGEIFS and reference list arrays and
    // a "normal" criteria range, swapped.
    m_pDoc->InsertMatrixFormula(
        8, 27, 8, 29, aMark,
        "=AVERAGEIFS(OFFSET(B1;ROW(1:3);0;2);B1:B2;\">1\";OFFSET(B1;ROW(1:3);0;2);\">4\")");

    {
        // Result sums, counts and averages.
        std::vector<std::vector<const char*>> aCheck
            = { { "0", "0", "#DIV/0!" }, { "8", "1", "8" }, { "16", "1", "16" } };
        bool bGood = checkOutput(m_pDoc, ScRange(6, 27, 0, 8, 29, 0), aCheck,
                                 "SUMIFS, COUNTIFS and AVERAGEIFS with reference list arrays and a "
                                 "normal criteria range, swapped");
        CPPUNIT_ASSERT_MESSAGE("SUMIFS, COUNTIFS or AVERAGEIFS with reference list arrays and a "
                               "normal criteria range failed, swapped",
                               bGood);
    }

    // Matrix formula in K28:K30 with SUMIFS and reference list array condition
    // and "normal" data range and a "normal" criteria range, swapped.
    m_pDoc->InsertMatrixFormula(10, 27, 10, 29, aMark,
                                "=SUMIFS(B1:B2;B1:B2;\">1\";OFFSET(B1;ROW(1:3);0;2);\">4\")");
    // Matrix formula in L28:L30 with AVERAGEIFS and reference list array
    // condition and "normal" data range and a "normal" criteria range,
    // swapped.
    m_pDoc->InsertMatrixFormula(11, 27, 11, 29, aMark,
                                "=AVERAGEIFS(B1:B2;B1:B2;\">1\";OFFSET(B1;ROW(1:3);0;2);\">4\")");

    {
        // Result sums and averages.
        std::vector<std::vector<const char*>> aCheck
            = { { "0", "#DIV/0!" }, { "2", "2" }, { "2", "2" } };
        bool bGood = checkOutput(m_pDoc, ScRange(10, 27, 0, 11, 29, 0), aCheck,
                                 "SUMIFS, COUNTIFS and AVERAGEIFS with reference list array and "
                                 "normal data and criteria range, swapped");
        CPPUNIT_ASSERT_MESSAGE("SUMIFS, COUNTIFS or AVERAGEIFS with reference list array and "
                               "normal data and criteria range failed, swapped",
                               bGood);
    }

    m_pDoc->DeleteTab(0);
}

// Test that COUNTIF counts properly empty cells if asked to.
CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncCOUNTIFEmpty)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Test");

    // Data in A1:A9.
    std::vector<std::vector<const char*>> aData
        = { { "" }, { "a" }, { "b" }, { "c" }, { "d" }, { "a" }, { "" }, { "b" }, { "c" } };

    insertRangeData(m_pDoc, ScAddress(0, 0, 0), aData);

    constexpr SCROW maxRow = 20; // so that the unittest is not slow in dbgutil builds
    SCROW startRow = 0;
    SCROW endRow = maxRow;
    SCCOL startCol = 0;
    SCCOL endCol = 0;
    // ScSortedRangeCache would normally shrink data range to this.
    CPPUNIT_ASSERT(m_pDoc->ShrinkToDataArea(0, startCol, startRow, endCol, endRow));
    CPPUNIT_ASSERT_EQUAL(SCROW(8), endRow);

    // But not if matching empty cells.
    m_pDoc->SetFormula(ScAddress(10, 0, 0),
                       "=COUNTIFS($A1:$A" + OUString::number(maxRow + 1) + "; \"\")",
                       formula::FormulaGrammar::GRAM_NATIVE_UI);
    CPPUNIT_ASSERT_EQUAL(double(maxRow + 1 - 7), m_pDoc->GetValue(ScAddress(10, 0, 0)));

    m_pDoc->DeleteTab(0);
}

// Test that COUNTIFS counts properly empty cells if asked to.
CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncCOUNTIFSRangeReduce)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Test");

    // Data in A1:C9.
    std::vector<std::vector<const char*>> aData = { { "" },
                                                    { "a", "1", "1" },
                                                    { "b", "2", "2" },
                                                    { "c", "4", "3" },
                                                    { "d", "8", "4" },
                                                    { "a", "16", "5" },
                                                    { "" },
                                                    { "b", "", "6" },
                                                    { "c", "64", "7" } };

    insertRangeData(m_pDoc, ScAddress(0, 0, 0), aData);

    constexpr SCROW maxRow = 20; // so that the unittest is not slow in dbgutil builds
    ScRange aSubRange(ScAddress(0, 0, 0), ScAddress(2, maxRow, 0));
    m_pDoc->GetDataAreaSubrange(aSubRange);
    // This is the range the data should be reduced to in ScInterpreter::IterateParametersIfs().
    CPPUNIT_ASSERT_EQUAL(SCROW(1), aSubRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(SCROW(8), aSubRange.aEnd.Row());

    m_pDoc->SetFormula(ScAddress(10, 0, 0),
                       "=COUNTIFS($A1:$A" + OUString::number(maxRow + 1) + "; \"\"; $B1:$B"
                           + OUString::number(maxRow + 1) + "; \"\"; $C1:$C"
                           + OUString::number(maxRow + 1) + "; \"\")",
                       formula::FormulaGrammar::GRAM_NATIVE_UI);
    // But it should find out that it can't range reduce and must count all the empty rows.
    CPPUNIT_ASSERT_EQUAL(double(maxRow + 1 - 7), m_pDoc->GetValue(ScAddress(10, 0, 0)));

    // Check also with criteria set as cell references, the middle one resulting in matching
    // empty cells (which should cause ScInterpreter::IterateParametersIfs() to undo
    // the range reduction). This should only match the A8-C8 row, but it also shouldn't crash.
    // Matching empty cells using a cell reference needs a formula to set the cell to
    // an empty string, plain empty cell wouldn't do, so use K2 for that.
    m_pDoc->SetFormula(ScAddress(10, 1, 0), "=\"\"", formula::FormulaGrammar::GRAM_NATIVE_UI);
    m_pDoc->SetFormula(ScAddress(10, 0, 0),
                       "=COUNTIFS($A1:$A" + OUString::number(maxRow + 1) + "; A8; $B1:$B"
                           + OUString::number(maxRow + 1) + "; K2; $C1:$C"
                           + OUString::number(maxRow + 1) + "; C8)",
                       formula::FormulaGrammar::GRAM_NATIVE_UI);
    CPPUNIT_ASSERT_EQUAL(double(1), m_pDoc->GetValue(ScAddress(10, 0, 0)));

    m_pDoc->DeleteTab(0);
}

// Test SUBTOTAL with reference lists in array context.
CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncRefListArraySUBTOTAL)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetValue(0, 0, 0, 1.0); // A1
    m_pDoc->SetValue(0, 1, 0, 2.0); // A2
    m_pDoc->SetValue(0, 2, 0, 4.0); // A3
    m_pDoc->SetValue(0, 3, 0, 8.0); // A4
    m_pDoc->SetValue(0, 4, 0, 16.0); // A5
    m_pDoc->SetValue(0, 5, 0, 32.0); // A6

    // Matrix in B7:B9, individual SUM of A2:A3, A3:A4 and A4:A5
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(1, 6, 1, 8, aMark, "=SUBTOTAL(9;OFFSET(A1;ROW(1:3);0;2))");
    ScAddress aPos(1, 6, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL SUM for A2:A3 failed", 6.0, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL SUM for A3:A4 failed", 12.0, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL SUM for A4:A5 failed", 24.0, m_pDoc->GetValue(aPos));

    // Matrix in C7:C9, individual AVERAGE of A2:A3, A3:A4 and A4:A5
    m_pDoc->InsertMatrixFormula(2, 6, 2, 8, aMark, "=SUBTOTAL(1;OFFSET(A1;ROW(1:3);0;2))");
    aPos.Set(2, 6, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL AVERAGE for A2:A3 failed", 3.0, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL AVERAGE for A3:A4 failed", 6.0, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL AVERAGE for A4:A5 failed", 12.0, m_pDoc->GetValue(aPos));

    // Matrix in D7:D9, individual MIN of A2:A3, A3:A4 and A4:A5
    m_pDoc->InsertMatrixFormula(3, 6, 3, 8, aMark, "=SUBTOTAL(5;OFFSET(A1;ROW(1:3);0;2))");
    aPos.Set(3, 6, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL MIN for A2:A3 failed", 2.0, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL MIN for A3:A4 failed", 4.0, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL MIN for A4:A5 failed", 8.0, m_pDoc->GetValue(aPos));

    // Matrix in E7:E9, individual MAX of A2:A3, A3:A4 and A4:A5
    m_pDoc->InsertMatrixFormula(4, 6, 4, 8, aMark, "=SUBTOTAL(4;OFFSET(A1;ROW(1:3);0;2))");
    aPos.Set(4, 6, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL MAX for A2:A3 failed", 4.0, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL MAX for A3:A4 failed", 8.0, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL MAX for A4:A5 failed", 16.0, m_pDoc->GetValue(aPos));

    // Matrix in F7:F9, individual STDEV of A2:A3, A3:A4 and A4:A5
    m_pDoc->InsertMatrixFormula(5, 6, 5, 8, aMark, "=SUBTOTAL(7;OFFSET(A1;ROW(1:3);0;2))");
    aPos.Set(5, 6, 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SUBTOTAL STDEV for A2:A3 failed", 1.414214,
                                         m_pDoc->GetValue(aPos), 1e-6);
    aPos.IncRow();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SUBTOTAL STDEV for A3:A4 failed", 2.828427,
                                         m_pDoc->GetValue(aPos), 1e-6);
    aPos.IncRow();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SUBTOTAL STDEV for A4:A5 failed", 5.656854,
                                         m_pDoc->GetValue(aPos), 1e-6);

    // Matrix in G7:G9, individual AVERAGE of A2:A3, A3:A4 and A4:A5
    // Plus two "ordinary" ranges, one before and one after.
    m_pDoc->InsertMatrixFormula(6, 6, 6, 8, aMark,
                                "=SUBTOTAL(1;A1:A2;OFFSET(A1;ROW(1:3);0;2);A5:A6)");
    aPos.Set(6, 6, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL AVERAGE for A1:A2,A2:A3,A5:A6 failed", 9.5,
                                 m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL AVERAGE for A1:A2,A3:A4,A5:A6 failed", 10.5,
                                 m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL AVERAGE for A1:A2,A4:A5,A5:A6 failed", 12.5,
                                 m_pDoc->GetValue(aPos));

    // Matrix in H7:H9, individual MAX of A2:A3, A3:A4 and A4:A5
    // Plus two "ordinary" ranges, one before and one after.
    m_pDoc->InsertMatrixFormula(7, 6, 7, 8, aMark,
                                "=SUBTOTAL(4;A1:A2;OFFSET(A1;ROW(1:3);0;2);A5:A6)");
    aPos.Set(7, 6, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL MAX for A1:A2,A2:A3,A5:A6 failed", 32.0,
                                 m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL MAX for A1:A2,A3:A4,A5:A6 failed", 32.0,
                                 m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUBTOTAL MAX for A1:A2,A4:A5,A5:A6 failed", 32.0,
                                 m_pDoc->GetValue(aPos));

    // Matrix in I7:I9, individual STDEV of A2:A3, A3:A4 and A4:A5
    // Plus two "ordinary" ranges, one before and one after.
    m_pDoc->InsertMatrixFormula(8, 6, 8, 8, aMark,
                                "=SUBTOTAL(7;A1:A2;OFFSET(A1;ROW(1:3);0;2);A5:A6)");
    aPos.Set(8, 6, 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SUBTOTAL STDEV for A1:A2,A2:A3,A5:A6 failed", 12.35718,
                                         m_pDoc->GetValue(aPos), 1e-5);
    aPos.IncRow();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SUBTOTAL STDEV for A1:A2,A3:A4,A5:A6 failed", 11.86170,
                                         m_pDoc->GetValue(aPos), 1e-5);
    aPos.IncRow();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SUBTOTAL STDEV for A1:A2,A4:A5,A5:A6 failed", 11.55422,
                                         m_pDoc->GetValue(aPos), 1e-5);

    // Empty two cells such that they affect two ranges.
    m_pDoc->SetString(0, 1, 0, ""); // A2
    m_pDoc->SetString(0, 2, 0, ""); // A3
    // Matrix in J7:J9, individual COUNTBLANK of A2:A3, A3:A4 and A4:A5
    m_pDoc->InsertMatrixFormula(9, 6, 9, 8, aMark, "=COUNTBLANK(OFFSET(A1;ROW(1:3);0;2))");
    aPos.Set(9, 6, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("COUNTBLANK for A1:A2,A2:A3,A5:A6 failed", 2.0,
                                 m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("COUNTBLANK for A1:A2,A3:A4,A5:A6 failed", 1.0,
                                 m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("COUNTBLANK for A1:A2,A4:A5,A5:A6 failed", 0.0,
                                 m_pDoc->GetValue(aPos));

    // Restore these two cell values so we'd catch failures below.
    m_pDoc->SetValue(0, 1, 0, 2.0); // A2
    m_pDoc->SetValue(0, 2, 0, 4.0); // A3
    // Hide rows 2 to 4.
    m_pDoc->SetRowHidden(1, 3, 0, true);
    // Matrix in K7, array of references as OFFSET result.
    m_pDoc->InsertMatrixFormula(10, 6, 10, 6, aMark,
                                "=SUM(SUBTOTAL(109;OFFSET(A1;ROW(A1:A7)-ROW(A1);;1)))");
    aPos.Set(10, 6, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUM SUBTOTAL failed", 49.0, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    // ForceArray in K8, array of references as OFFSET result.
    m_pDoc->SetString(aPos, "=SUMPRODUCT(SUBTOTAL(109;OFFSET(A1;ROW(A1:A7)-ROW(A1);;1)))");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUMPRODUCT SUBTOTAL failed", 49.0, m_pDoc->GetValue(aPos));

    m_pDoc->DeleteTab(0);
}

// tdf#115493 jump commands return the matrix result instead of the reference
// list array.
CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncJumpMatrixArrayIF)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(0, 0, 0, "a"); // A1
    std::vector<std::vector<const char*>> aData
        = { { "a", "1" }, { "b", "2" }, { "a", "4" } }; // A7:B9
    insertRangeData(m_pDoc, ScAddress(0, 6, 0), aData);

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);

    // Matrix in C10, summing B7,B9
    m_pDoc->InsertMatrixFormula(2, 9, 2, 9, aMark, "=SUM(IF(EXACT(A7:A9;A$1);B7:B9;0))");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula C10 failed", 5.0, m_pDoc->GetValue(ScAddress(2, 9, 0)));

    // Matrix in C11, summing B7,B9
    m_pDoc->InsertMatrixFormula(
        2, 10, 2, 10, aMark,
        "=SUM(IF(EXACT(OFFSET(A7;0;0):OFFSET(A7;2;0);A$1);OFFSET(A7;0;1):OFFSET(A7;2;1);0))");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula C11 failed", 5.0, m_pDoc->GetValue(ScAddress(2, 10, 0)));

    m_pDoc->DeleteTab(0);
}

// tdf#123477 OFFSET() returns the matrix result instead of the reference list
// array if result is not used as ReferenceOrRefArray.
CPPUNIT_TEST_FIXTURE(TestFormula2, testFuncJumpMatrixArrayOFFSET)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Test");

    std::vector<std::vector<const char*>> aData = { { "abc" }, { "bcd" }, { "cde" } };
    insertRangeData(m_pDoc, ScAddress(0, 0, 0), aData); // A1:A3

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);

    // Matrix in C5:C7, COLUMN()-3 here offsets by 0 but the entire expression
    // is in array/matrix context.
    m_pDoc->InsertMatrixFormula(2, 4, 2, 6, aMark, "=FIND(\"c\";OFFSET(A1:A3;0;COLUMN()-3))");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula C5 failed", 3.0, m_pDoc->GetValue(ScAddress(2, 4, 0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula C6 failed", 2.0, m_pDoc->GetValue(ScAddress(2, 5, 0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula C7 failed", 1.0, m_pDoc->GetValue(ScAddress(2, 6, 0)));

    m_pDoc->DeleteTab(0);
}

// Test iterations with circular chain of references.
CPPUNIT_TEST_FIXTURE(TestFormula2, testIterations)
{
    ScDocOptions aDocOpts = m_pDoc->GetDocOptions();
    aDocOpts.SetIter(true);
    m_pDoc->SetDocOptions(aDocOpts);

    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetValue(0, 0, 0, 0.01); // A1
    m_pDoc->SetString(0, 1, 0, "=A1"); // A2
    m_pDoc->SetString(0, 2, 0, "=COS(A2)"); // A3
    m_pDoc->CalcAll();

    // Establish reference cycle for the computation of the fixed point of COS() function
    m_pDoc->SetString(0, 0, 0, "=A3"); // A1
    m_pDoc->CalcAll();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell A3 should not have any formula error", FormulaError::NONE,
                                 m_pDoc->GetErrCode(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Iterations to calculate fixed point of cos() failed",
                                         0.7387, m_pDoc->GetValue(0, 2, 0), 1e-4);

    // Modify the formula
    m_pDoc->SetString(0, 2, 0, "=COS(A2)+0.001"); // A3
    m_pDoc->CalcAll();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell A3 should not have any formula error after perturbation",
                                 FormulaError::NONE, m_pDoc->GetErrCode(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
        "Iterations to calculate perturbed fixed point of cos() failed", 0.7399,
        m_pDoc->GetValue(0, 2, 0), 1e-4);

    m_pDoc->DeleteTab(0);

    aDocOpts.SetIter(false);
    m_pDoc->SetDocOptions(aDocOpts);
}

// tdf#111428 CellStoreEvent and its counter used for quick "has a column
// formula cells" must point to the correct column.
CPPUNIT_TEST_FIXTURE(TestFormula2, testInsertColCellStoreEventSwap)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetValue(0, 0, 0, 1.0); // A1
    m_pDoc->SetString(1, 0, 0, "=A1"); // B1
    // Insert column left of B
    m_pDoc->InsertCol(ScRange(1, 0, 0, 1, m_pDoc->MaxRow(), 0));
    ScAddress aPos(2, 0, 0); // C1, new formula position
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be formula cell having value", 1.0,
                                 m_pDoc->GetValue(aPos));
    // After having swapped in an empty column, editing or adding a formula
    // cell has to use the correct store context. To test this,
    // ScDocument::SetString() can't be used as it doesn't expose the behavior
    // in question, use ScDocFunc::SetFormulaCell() instead which actually is
    // also called when editing a cell and creating a formula cell.
    ScFormulaCell* pCell = new ScFormulaCell(*m_pDoc, aPos, "=A1+1");
    ScDocFunc& rDocFunc = m_xDocShell->GetDocFunc();
    rDocFunc.SetFormulaCell(aPos, pCell, false); // C1, change formula
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Initial calculation failed", 2.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, 2.0); // A1, change value
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Recalculation failed", 3.0, m_pDoc->GetValue(aPos));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testTdf147398)
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(0, 0, 0, "=SUM(A3:A5)");
    m_pDoc->SetString(0, 1, 0, "=COUNT(A3:A5)");
    m_pDoc->SetString(1, 0, 0, "=SUM(B3:B5)");
    m_pDoc->SetString(1, 1, 0, "=COUNT(B3:B5)");
    m_pDoc->SetString(2, 0, 0, "=SUM(C3:C5)");
    m_pDoc->SetString(2, 1, 0, "=COUNT(C3:C5)");
    m_pDoc->SetString(3, 0, 0, "=SUM(D3:D5)");
    m_pDoc->SetString(3, 1, 0, "=COUNT(D3:D5)");
    m_pDoc->SetString(4, 0, 0, "=SUM(E3:E5)");
    m_pDoc->SetString(4, 1, 0, "=COUNT(E3:E5)");

    m_pDoc->SetString(5, 0, 0, "=SUM(A1:E1)/SUM(A2:E2)");

    m_pDoc->SetValue(ScAddress(0, 2, 0), 50.0);
    m_pDoc->SetValue(ScAddress(0, 3, 0), 100.0);

    CPPUNIT_ASSERT_EQUAL(150.0, m_pDoc->GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(75.0, m_pDoc->GetValue(ScAddress(5, 0, 0)));

    m_pDoc->SetValue(ScAddress(1, 2, 0), 150.0);
    m_pDoc->SetValue(ScAddress(1, 3, 0), 200.0);

    CPPUNIT_ASSERT_EQUAL(150.0, m_pDoc->GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(350.0, m_pDoc->GetValue(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(125.0, m_pDoc->GetValue(ScAddress(5, 0, 0)));

    m_pDoc->SetValue(ScAddress(2, 2, 0), 250.0);
    m_pDoc->SetValue(ScAddress(2, 3, 0), 300.0);

    CPPUNIT_ASSERT_EQUAL(150.0, m_pDoc->GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(350.0, m_pDoc->GetValue(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(550.0, m_pDoc->GetValue(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(175.0, m_pDoc->GetValue(ScAddress(5, 0, 0)));

    m_pDoc->SetValue(ScAddress(3, 2, 0), 350.0);
    m_pDoc->SetValue(ScAddress(3, 3, 0), 400.0);

    CPPUNIT_ASSERT_EQUAL(150.0, m_pDoc->GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(350.0, m_pDoc->GetValue(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(550.0, m_pDoc->GetValue(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(750.0, m_pDoc->GetValue(ScAddress(3, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(3, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(225.0, m_pDoc->GetValue(ScAddress(5, 0, 0)));

    m_pDoc->SetValue(ScAddress(4, 2, 0), 450.0);
    m_pDoc->SetValue(ScAddress(4, 3, 0), 500.0);

    CPPUNIT_ASSERT_EQUAL(150.0, m_pDoc->GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(350.0, m_pDoc->GetValue(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(550.0, m_pDoc->GetValue(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(750.0, m_pDoc->GetValue(ScAddress(3, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(3, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(950.0, m_pDoc->GetValue(ScAddress(4, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(275.0, m_pDoc->GetValue(ScAddress(5, 0, 0)));

    m_pDoc->DeleteTab(0);
}

#if SC_USE_SSE2
CPPUNIT_TEST_FIXTURE(TestFormula2, testTdf156985)
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(0, 0, 0, "=-170.87");
    m_pDoc->SetString(0, 1, 0, "-223.73");
    m_pDoc->SetString(0, 2, 0, "-12.58");
    m_pDoc->SetString(0, 3, 0, "234.98");
    m_pDoc->SetString(0, 4, 0, "172.2");
    m_pDoc->SetString(0, 5, 0, "=SUM(A1:A5)");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : -1.59872115546023e-14
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(0, 5, 0));

    m_pDoc->DeleteTab(0);
}
#endif

CPPUNIT_TEST_FIXTURE(TestFormula2, testFormulaAfterDeleteRows)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Test");

    // Fill A1:A70000 with 1.0
    std::vector<double> aVals(70000, 1.0);
    m_pDoc->SetValues(ScAddress(0, 0, 0), aVals);
    // Set A70001 with formula "=SUM(A1:A70000)"
    m_pDoc->SetString(0, 70000, 0, "=SUM(A1:A70000)");

    // Delete rows 2:69998
    m_pDoc->DeleteRow(ScRange(0, 1, 0, m_pDoc->MaxCol(), 69997, 0));

    const ScAddress aPos(0, 3, 0); // A4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A4.", OUString("=SUM(A1:A3)"),
                                 m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    ASSERT_DOUBLES_EQUAL_MESSAGE("Wrong value at A4", 3.0, m_pDoc->GetValue(aPos));
}

CPPUNIT_TEST_FIXTURE(TestFormula2, testRegexForXLOOKUP)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);

    // Temporarily switch regex search mode.
    bool bOldWildCard = false;
    ScDocOptions aDocOpt = m_pDoc->GetDocOptions();
    if (!aDocOpt.IsFormulaRegexEnabled())
    {
        aDocOpt.SetFormulaRegexEnabled(true);
        m_pDoc->SetDocOptions(aDocOpt);
        bOldWildCard = true;
    }

    m_pDoc->InsertTab(0, "Test1");

    std::vector<std::vector<const char*>> aData = { { "Element", "Relative Atomic Mass" },
                                                    { "Hydrogen", "1.008" },
                                                    { "Helium", "4.003" },
                                                    { "Lithium", "6.94" },
                                                    { "Beryllium", "9.012" },
                                                    { "Boron", "10.81" },
                                                    { "Carbon", "12.011" },
                                                    { "Nitrogen", "14.007" },
                                                    { "Oxygen", "15.999" },
                                                    { "Florine", "18.998" },
                                                    { "Neon", "20.18" } };

    insertRangeData(m_pDoc, ScAddress(0, 0, 0), aData); // A1:B11
    m_pDoc->SetString(4, 14, 0, "^bo.*"); // E15 - search regex string

    m_pDoc->SetFormula(ScAddress(5, 14, 0), "=XLOOKUP(E15;A$2:A$11;B$2:B$11;;2)",
                       formula::FormulaGrammar::GRAM_NATIVE_UI); // F15

    // Without the fix in place, this test would have failed with
    // - Expected: 10.81
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(10.81, m_pDoc->GetValue(5, 14, 0));

    // Switch back to wildcard mode if necessary.
    if (bOldWildCard)
    {
        aDocOpt.SetFormulaWildcardsEnabled(true);
        m_pDoc->SetDocOptions(aDocOpt);
    }
    m_pDoc->DeleteTab(0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
