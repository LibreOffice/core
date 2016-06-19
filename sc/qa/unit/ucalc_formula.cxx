/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ucalc.hxx"
#include "markdata.hxx"
#include "calcconfig.hxx"
#include "clipparam.hxx"
#include "interpre.hxx"
#include "compiler.hxx"
#include "tokenarray.hxx"
#include "refdata.hxx"
#include "scopetools.hxx"
#include "formulacell.hxx"
#include "formulagroup.hxx"
#include "scmod.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "paramisc.hxx"
#include "tokenstringcontext.hxx"
#include "dbdata.hxx"
#include "scmatrix.hxx"
#include <validat.hxx>
#include <scitems.hxx>
#include <patattr.hxx>
#include <docpool.hxx>

#include <formula/vectortoken.hxx>
#include <officecfg/Office/Common.hxx>
#include <svl/broadcast.hxx>

#include <memory>
#include <functional>
#include <set>
#include <algorithm>

using namespace formula;

namespace {

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

}

void Test::testFormulaCreateStringFromTokens()
{
    // Insert sheets.
    OUString aTabName1("Test");
    OUString aTabName2("Kevin's Data");
    OUString aTabName3("Past Data");
    OUString aTabName4("2013");
    m_pDoc->InsertTab(0, aTabName1);
    m_pDoc->InsertTab(1, aTabName2);
    m_pDoc->InsertTab(2, aTabName3);
    m_pDoc->InsertTab(3, aTabName4);

    // Insert named ranges.
    struct {
        bool bGlobal;
        const char* pName;
        const char* pExpr;
    } aNames[] = {
        { true, "x", "Test.H1" },
        { true, "y", "Test.H2" },
        { true, "z", "Test.H3" },

        { false, "sheetx", "Test.J1" }
    };

    ScRangeName* pGlobalNames = m_pDoc->GetRangeName();
    ScRangeName* pSheetNames = m_pDoc->GetRangeName(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to obtain global named expression object.", pGlobalNames);
    CPPUNIT_ASSERT_MESSAGE("Failed to obtain sheet-local named expression object.", pSheetNames);

    for (size_t i = 0, n = SAL_N_ELEMENTS(aNames); i < n; ++i)
    {
        ScRangeData* pName = new ScRangeData(
            m_pDoc, OUString::createFromAscii(aNames[i].pName), OUString::createFromAscii(aNames[i].pExpr),
            ScAddress(0,0,0), ScRangeData::Type::Name, formula::FormulaGrammar::GRAM_NATIVE);

        if (aNames[i].bGlobal)
        {
            bool bInserted = pGlobalNames->insert(pName);
            CPPUNIT_ASSERT_MESSAGE("Failed to insert a new name.", bInserted);
        }
        else
        {
            bool bInserted = pSheetNames->insert(pName);
            CPPUNIT_ASSERT_MESSAGE("Failed to insert a new name.", bInserted);
        }
    }

    // Insert DB ranges.
    struct {
        const char* pName;
        SCTAB nTab;
        SCCOL nCol1;
        SCROW nRow1;
        SCCOL nCol2;
        SCROW nRow2;
    } aDBs[] = {
        { "Table1", 0, 0, 0, 10, 10 },
        { "Table2", 1, 0, 0, 10, 10 },
        { "Table3", 2, 0, 0, 10, 10 }
    };

    ScDBCollection* pDBs = m_pDoc->GetDBCollection();
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch DB collection object.", pDBs);

    for (size_t i = 0, n = SAL_N_ELEMENTS(aDBs); i < n; ++i)
    {
        ScDBData* pData = new ScDBData(
            OUString::createFromAscii(
                aDBs[i].pName), aDBs[i].nTab, aDBs[i].nCol1, aDBs[i].nRow1, aDBs[i].nCol2,aDBs[i].nRow2);
        bool bInserted = pDBs->getNamedDBs().insert(pData);
        CPPUNIT_ASSERT_MESSAGE(
            OString(
                "Failed to insert \"" + OString(aDBs[i].pName) + "\"").getStr(),
            bInserted);
    }

    const char* aTests[] = {
        "1+2",
        "SUM(A1:A10;B1:B10;C5;D6)",
        "IF(Test.B10<>10;\"Good\";\"Bad\")",
        "AVERAGE('2013'.B10:C20)",
        "'Kevin''s Data'.B10",
        "'Past Data'.B1+'2013'.B2*(1+'Kevin''s Data'.C10)",
        "x+y*z", // named ranges
        "SUM(sheetx;x;y;z)", // sheet local and global named ranges mixed
        "MAX(Table1)+MIN(Table2)*SUM(Table3)", // database ranges
        "{1;TRUE;3|FALSE;5;\"Text\"|;;}", // inline matrix
        "SUM('file:///path/to/fake.file'#$Sheet.A1:B10)",
    };
    (void) aTests;

    std::unique_ptr<ScTokenArray> pArray;

    sc::TokenStringContext aCxt(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH);

    // Artificially add external refererence data after the context object is
    // initialized.
    aCxt.maExternalFileNames.push_back("file:///path/to/fake.file");
    std::vector<OUString> aExtTabNames;
    aExtTabNames.push_back("Sheet");
    aCxt.maExternalCachedTabNames.insert(
        sc::TokenStringContext::IndexNamesMapType::value_type(0, aExtTabNames));

    ScAddress aPos(0,0,0);

    for (size_t i = 0, n = SAL_N_ELEMENTS(aTests); i < n; ++i)
    {
#if 0
        OUString aFormula = OUString::createFromAscii(aTests[i]);
#endif
        ScCompiler aComp(m_pDoc, aPos);
        aComp.SetGrammar(FormulaGrammar::GRAM_ENGLISH);
#if 0 // TODO: This call to CompileString() causes the cppunittester to somehow fail on Windows.
        pArray.reset(aComp.CompileString(aFormula));
        CPPUNIT_ASSERT_MESSAGE("Failed to compile formula string.", pArray.get());

        OUString aCheck = pArray->CreateString(aCxt, aPos);
        CPPUNIT_ASSERT_EQUAL(aFormula, aCheck);
#endif
    }

    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

namespace {

bool isEmpty( const formula::VectorRefArray& rArray, size_t nPos )
{
    if (rArray.mpStringArray)
    {
        if (rArray.mpStringArray[nPos])
            return false;
    }

    if (rArray.mpNumericArray)
        return rtl::math::isNan(rArray.mpNumericArray[nPos]);
    else
        return true;
}

bool equals( const formula::VectorRefArray& rArray, size_t nPos, double fVal )
{
    if (rArray.mpStringArray && rArray.mpStringArray[nPos])
        // This is a string cell.
        return false;

    if (rArray.mpNumericArray && rArray.mpNumericArray[nPos] == fVal)
        return true;

    return false;
}

bool equals( const formula::VectorRefArray& rArray, size_t nPos, const OUString& rVal )
{
    if (!rArray.mpStringArray)
        return false;

    bool bEquals = OUString(rArray.mpStringArray[nPos]).equalsIgnoreAsciiCase(rVal);
    if (!bEquals)
    {
        cerr << "Expected: " << rVal.toAsciiUpperCase() << " (upcased)" << endl;
        cerr << "Actual: " << OUString(rArray.mpStringArray[nPos]) << " (upcased)" << endl;
    }
    return bEquals;
}

}

void Test::testFormulaParseReference()
{
    OUString aTab1("90's Music"), aTab2("90's and 70's"), aTab3("All Others"), aTab4("NoQuote");
    m_pDoc->InsertTab(0, "Dummy"); // just to shift the sheet indices...
    m_pDoc->InsertTab(1, aTab1); // name with a single quote.
    m_pDoc->InsertTab(2, aTab2); // name with 2 single quotes.
    m_pDoc->InsertTab(3, aTab3); // name without single quotes.
    m_pDoc->InsertTab(4, aTab4); // name that doesn't require to be quoted.

    OUString aTabName;
    m_pDoc->GetName(1, aTabName);
    CPPUNIT_ASSERT_EQUAL(aTab1, aTabName);
    m_pDoc->GetName(2, aTabName);
    CPPUNIT_ASSERT_EQUAL(aTab2, aTabName);
    m_pDoc->GetName(3, aTabName);
    CPPUNIT_ASSERT_EQUAL(aTab3, aTabName);
    m_pDoc->GetName(4, aTabName);
    CPPUNIT_ASSERT_EQUAL(aTab4, aTabName);

    // Make sure the formula input and output match.
    {
        const char* aChecks[] = {
            "'90''s Music'.B12",
            "'90''s and 70''s'.$AB$100",
            "'All Others'.Z$100",
            "NoQuote.$C111"
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            // Use the 'Dummy' sheet for this.
            OUString aInput("=");
            aInput += OUString::createFromAscii(aChecks[i]);
            m_pDoc->SetString(ScAddress(0,0,0), aInput);
            ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,0,0), aChecks[i], "Wrong formula");
        }
    }

    ScAddress aPos;
    ScAddress::ExternalInfo aExtInfo;
    ScRefFlags nRes = aPos.Parse("'90''s Music'.D10", m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(3), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

    nRes = aPos.Parse("'90''s and 70''s'.C100", m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(2), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(99), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

    nRes = aPos.Parse("'All Others'.B3", m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(3), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

    nRes = aPos.Parse("NoQuote.E13", m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(4), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(4), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

    ScRange aRange;

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(":B", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse("B:", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(":B2", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse("B2:", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(":2", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse("2:", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(":2B", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse("2B:", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse("abc_foo:abc_bar", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse("B:B", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aStart.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), aRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aEnd.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(MAXROW), aRange.aEnd.Row());
    CPPUNIT_ASSERT((nRes & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID)) ==
                           (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID));
    CPPUNIT_ASSERT((nRes & (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS)) ==
                            ScRefFlags::ZERO);
    CPPUNIT_ASSERT((nRes & (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS)) ==
                           (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse("2:2", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aStart.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), aRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), aRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aEnd.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(MAXCOL), aRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), aRange.aEnd.Row());
    CPPUNIT_ASSERT((nRes & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID)) ==
                           (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID));
    CPPUNIT_ASSERT((nRes & (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS)) ==
                            ScRefFlags::ZERO);
    CPPUNIT_ASSERT((nRes & (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS)) ==
                           (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS));

    nRes = aRange.Parse("NoQuote.B:C", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(4), aRange.aStart.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), aRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(4), aRange.aEnd.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(2), aRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(MAXROW), aRange.aEnd.Row());
    CPPUNIT_ASSERT((nRes & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID)) ==
                           (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID));
    CPPUNIT_ASSERT((nRes & (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS)) ==
                            ScRefFlags::ZERO);
    CPPUNIT_ASSERT((nRes & (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS)) ==
                           (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS));

    // Both rows at sheet bounds and relative => convert to absolute => entire column reference.
    aRange.aStart.SetTab(0);
    nRes = aRange.Parse("B1:B1048576", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aStart.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), aRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aEnd.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(MAXROW), aRange.aEnd.Row());
    CPPUNIT_ASSERT((nRes & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID)) ==
                           (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID));
    CPPUNIT_ASSERT((nRes & (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS)) ==
                           ScRefFlags::ZERO);
    CPPUNIT_ASSERT((nRes & (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS)) ==
                           (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS));

    // Both columns at sheet bounds and relative => convert to absolute => entire row reference.
    aRange.aStart.SetTab(0);
    nRes = aRange.Parse("A2:AMJ2", m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aStart.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), aRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), aRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aEnd.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(MAXCOL), aRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), aRange.aEnd.Row());
    CPPUNIT_ASSERT((nRes & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID)) ==
                           (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID));
    CPPUNIT_ASSERT((nRes & (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS)) ==
                            ScRefFlags::ZERO);
    CPPUNIT_ASSERT((nRes & (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS)) ==
                           (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS));

    // Check for reference input conversion to and display string of entire column/row.
    {
        const char* aChecks[][2] = {
            { "=B:B",           "B:B" },
            { "=B1:B1048576",   "B:B" },
            { "=B1:B$1048576",  "B1:B$1048576" },
            { "=B$1:B1048576",  "B$1:B1048576" },
            { "=B$1:B$1048576", "B:B" },
            { "=2:2",           "2:2" },
            { "=A2:AMJ2",       "2:2" },
            { "=A2:$AMJ2",      "A2:$AMJ2" },
            { "=$A2:AMJ2",      "$A2:AMJ2" },
            { "=$A2:$AMJ2",     "2:2" }
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            // Use the 'Dummy' sheet for this.
            m_pDoc->SetString(ScAddress(0,0,0), OUString::createFromAscii(aChecks[i][0]));
            ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,0,0), aChecks[i][1], "Wrong formula");
        }
    }

    m_pDoc->DeleteTab(4);
    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testFetchVectorRefArray()
{
    m_pDoc->InsertTab(0, "Test");

    // All numeric cells in Column A.
    m_pDoc->SetValue(ScAddress(0,0,0), 1);
    m_pDoc->SetValue(ScAddress(0,1,0), 2);
    m_pDoc->SetValue(ScAddress(0,2,0), 3);
    m_pDoc->SetValue(ScAddress(0,3,0), 4);

    formula::VectorRefArray aArray = m_pDoc->FetchVectorRefArray(ScAddress(0,0,0), 4);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array is expected to be numeric cells only.", !aArray.mpStringArray);
    CPPUNIT_ASSERT_EQUAL(1.0, aArray.mpNumericArray[0]);
    CPPUNIT_ASSERT_EQUAL(2.0, aArray.mpNumericArray[1]);
    CPPUNIT_ASSERT_EQUAL(3.0, aArray.mpNumericArray[2]);
    CPPUNIT_ASSERT_EQUAL(4.0, aArray.mpNumericArray[3]);

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(0,0,0), 5);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array is expected to be numeric cells only.", !aArray.mpStringArray);
    CPPUNIT_ASSERT_EQUAL(1.0, aArray.mpNumericArray[0]);
    CPPUNIT_ASSERT_EQUAL(2.0, aArray.mpNumericArray[1]);
    CPPUNIT_ASSERT_EQUAL(3.0, aArray.mpNumericArray[2]);
    CPPUNIT_ASSERT_EQUAL(4.0, aArray.mpNumericArray[3]);
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 4));

    // All string cells in Column B.  Note that the fetched string arrays are
    // only to be compared case-insensitively.  Right now, we use upper cased
    // strings to achieve case-insensitive-ness, but that may change. So,
    // don't count on that.
    m_pDoc->SetString(ScAddress(1,0,0), "Andy");
    m_pDoc->SetString(ScAddress(1,1,0), "Bruce");
    m_pDoc->SetString(ScAddress(1,2,0), "Charlie");
    m_pDoc->SetString(ScAddress(1,3,0), "David");
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(1,0,0), 5);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array is expected to be string cells only.", !aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 0, "Andy"));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 1, "Bruce"));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 2, "Charlie"));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 3, "David"));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 4));

    // Mixture of numeric, string, and empty cells in Column C.
    m_pDoc->SetString(ScAddress(2,0,0), "Header");
    m_pDoc->SetValue(ScAddress(2,1,0), 11);
    m_pDoc->SetValue(ScAddress(2,2,0), 12);
    m_pDoc->SetValue(ScAddress(2,3,0), 13);
    m_pDoc->SetString(ScAddress(2,5,0), "=SUM(C2:C4)");
    m_pDoc->CalcAll();

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(2,0,0), 7);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should have both numeric and string arrays.", aArray.mpNumericArray && aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 0, "Header"));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 1, 11));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 2, 12));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 3, 13));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 4));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 5, 36));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 6));

    // Mixed type again in Column D, but it starts with a numeric cell.
    m_pDoc->SetValue(ScAddress(3,0,0), 10);
    m_pDoc->SetString(ScAddress(3,1,0), "Below 10");
    // Leave 2 empty cells.
    m_pDoc->SetValue(ScAddress(3,4,0), 11);
    m_pDoc->SetString(ScAddress(3,5,0), "=12");
    m_pDoc->SetString(ScAddress(3,6,0), "=13");
    m_pDoc->SetString(ScAddress(3,7,0), "=CONCATENATE(\"A\";\"B\";\"C\")");
    m_pDoc->CalcAll();

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(3,0,0), 8);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should have both numeric and string arrays.", aArray.mpNumericArray && aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 0, 10));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 1, "Below 10"));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 2));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 3));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 4, 11));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 5, 12));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 6, 13));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 7, "ABC"));

    // Column E consists of formula cells whose results are all numeric.
    for (SCROW i = 0; i <= 6; ++i)
        m_pDoc->SetString(ScAddress(4,i,0), "=ROW()");
    m_pDoc->CalcAll();

    // Leave row 7 empty.
    m_pDoc->SetString(ScAddress(4,8,0), "Andy");
    m_pDoc->SetValue(ScAddress(4,9,0), 123);

    // This array fits within a single formula block.
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(4,0,0), 5);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should be purely numeric.", aArray.mpNumericArray && !aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 0, 1));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 1, 2));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 2, 3));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 3, 4));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 4, 5));

    // This array spans over multiple blocks.
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(4,0,0), 11);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should have both numeric and string arrays.", aArray.mpNumericArray && aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 0, 1));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 1, 2));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 2, 3));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 3, 4));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 4, 5));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 5, 6));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 6, 7));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 7));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 8, "Andy"));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 9, 123));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 10));

    // Hit the cache but at a different start row.
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(4,2,0), 3);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should at least have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 0, 3));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 1, 4));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 2, 5));

    // Column F begins with empty rows at the top.
    m_pDoc->SetValue(ScAddress(5,2,0), 1.1);
    m_pDoc->SetValue(ScAddress(5,3,0), 1.2);
    m_pDoc->SetString(ScAddress(5,4,0), "=2*8");
    m_pDoc->CalcAll();

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(5,2,0), 4);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should at least have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 0, 1.1));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 1, 1.2));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 2, 16));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 3));

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(5,0,0), 3);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should at least have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 0));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 1));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 2, 1.1));

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(5,0,0), 10);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should at least have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 0));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 1));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 2, 1.1));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 3, 1.2));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 4, 16));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 5));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 6));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 7));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 8));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 9));

    // Get the array for F3:F4. This array should only consist of numeric array.
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(5,2,0), 3);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);

    // Column G consists only of strings.
    m_pDoc->SetString(ScAddress(6,0,0), "Title");
    m_pDoc->SetString(ScAddress(6,1,0), "foo");
    m_pDoc->SetString(ScAddress(6,2,0), "bar");
    m_pDoc->SetString(ScAddress(6,3,0), "foo");
    m_pDoc->SetString(ScAddress(6,4,0), "baz");
    m_pDoc->SetString(ScAddress(6,5,0), "quack");
    m_pDoc->SetString(ScAddress(6,6,0), "beep");
    m_pDoc->SetString(ScAddress(6,7,0), "kerker");

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(6,1,0), 4); // G2:G5
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a numeric array.", !aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should have a string array.", aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 0, "foo"));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 1, "bar"));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 2, "foo"));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 3, "baz"));

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(6,2,0), 4); // G3:G6
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a numeric array.", !aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should have a string array.", aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 0, "bar"));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 1, "foo"));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 2, "baz"));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 3, "quack"));

    // Column H starts with formula cells.
    for (SCROW i = 0; i < 10; ++i)
        m_pDoc->SetString(ScAddress(7,i,0), "=ROW()");

    m_pDoc->CalcAll();
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(7,3,0), 3); // H4:H6
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 0, 4.0));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 1, 5.0));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 2, 6.0));

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(7,4,0), 10); // H5:H15
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 0, 5.0));

    // Clear everything and start over.
    clearRange(m_pDoc, ScRange(0,0,0,MAXCOL,MAXROW,0));
    m_pDoc->ClearFormulaContext();

    // Totally empty range in a totally empty column (Column A).
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(0,0,0), 3); // A1:A3
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[0]));
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[1]));
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[2]));

    // Totally empty range in a non-empty column (Column B).
    m_pDoc->SetString(ScAddress(1,10,0), "Some text"); // B11
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(1,0,0), 3); // B1:B3
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[0]));
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[1]));
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[2]));

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(1,12,0), 3); // B13:B15
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[0]));
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[1]));
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[2]));

    // These values come from a cache because of the call above.
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(1,1,0), 3); // B2:B4
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[0]));
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[1]));
    CPPUNIT_ASSERT(rtl::math::isNan(aArray.mpNumericArray[2]));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaHashAndTag()
{
    m_pDoc->InsertTab(0, "Test");

    ScAddress aPos1(0,0,0), aPos2(1,0,0);

    // Test formula hashing.

    struct {
        const char* pFormula1; const char* pFormula2; bool bEqual;
    } aHashTests[] = {
        { "=1", "=2", false }, // different constants
        { "=SUM(1;2;3;4;5)", "=AVERAGE(1;2;3;4;5)", false }, // different functions
        { "=C2*3", "=D2*3", true },  // relative references
        { "=C2*3", "=D2*4", false }, // different constants
        { "=C2*4", "=D2*4", true },  // relative references
        { "=3*4*5", "=3*4*\"foo\"", false }, // numeric vs string constants
        { "=$C3/2", "=$C3/2", true }, // absolute column references
        { "=C$3/2", "=D$3/2", true }, // absolute row references
        { "=$E$30/2", "=$E$30/2", true }, // absolute references
        { "=X20", "=$X$20", false }, // absolute vs relative
        { "=X20", "=X$20", false }, // absolute vs relative
        { "=X20", "=$X20", false }, // absolute vs relative
        { "=X$20", "=$X20", false }, // column absolute vs row absolute
        // similar enough for merging ...
        { "=A1", "=B1", true },
        { "=$A$1", "=$B$1", true },
        { "=A1", "=C2", true },
        { "=SUM(A1)", "=SUM(B1)", true },
        { "=A1+3", "=B1+3", true },
        { "=A1+7", "=B1+42", false },
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aHashTests); ++i)
    {
        m_pDoc->SetString(aPos1, OUString::createFromAscii(aHashTests[i].pFormula1));
        m_pDoc->SetString(aPos2, OUString::createFromAscii(aHashTests[i].pFormula2));
        size_t nHashVal1 = m_pDoc->GetFormulaHash(aPos1);
        size_t nHashVal2 = m_pDoc->GetFormulaHash(aPos2);

        std::ostringstream os;
        os << "(expr1:" << aHashTests[i].pFormula1 << "; expr2:" << aHashTests[i].pFormula2 << ")";
        if (aHashTests[i].bEqual)
        {
            os << " Error: these hashes should be equal." << endl;
            CPPUNIT_ASSERT_MESSAGE(os.str().c_str(), nHashVal1 == nHashVal2);
        }
        else
        {
            os << " Error: these hashes should differ." << endl;
            CPPUNIT_ASSERT_MESSAGE(os.str().c_str(), nHashVal1 != nHashVal2);
        }

        aPos1.IncRow();
        aPos2.IncRow();
    }

    // Go back to row 1.
    aPos1.SetRow(0);
    aPos2.SetRow(0);

    // Test formula vectorization state.

    struct {
        const char* pFormula;
        ScFormulaVectorState eState;
        ScFormulaVectorState eSwInterpreterState; // these can change when more is whitelisted
    } aVectorTests[] = {
        { "=SUM(1;2;3;4;5)", FormulaVectorEnabled, FormulaVectorEnabled },
        { "=NOW()", FormulaVectorDisabled, FormulaVectorDisabled },
        { "=AVERAGE(X1:Y200)", FormulaVectorCheckReference, FormulaVectorDisabled },
        { "=MAX(X1:Y200;10;20)", FormulaVectorCheckReference, FormulaVectorDisabled },
        { "=MIN(10;11;22)", FormulaVectorEnabled, FormulaVectorDisabled },
        { "=H4", FormulaVectorCheckReference, FormulaVectorCheckReference },
    };

    bool bSwInterpreter = officecfg::Office::Common::Misc::UseSwInterpreter::get();
    bool bOpenCL = officecfg::Office::Common::Misc::UseOpenCL::get();

    for (bool bForceSwInterpreter : { false, true })
    {
        std::shared_ptr< comphelper::ConfigurationChanges > xBatch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::UseSwInterpreter::set(bForceSwInterpreter, xBatch);
        if (bForceSwInterpreter)
            officecfg::Office::Common::Misc::UseOpenCL::set(false, xBatch);
        xBatch->commit();

        for (size_t i = 0; i < SAL_N_ELEMENTS(aVectorTests); ++i)
        {
            m_pDoc->SetString(aPos1, OUString::createFromAscii(aVectorTests[i].pFormula));
            ScFormulaVectorState eState = m_pDoc->GetFormulaVectorState(aPos1);
            ScFormulaVectorState eReferenceState = bForceSwInterpreter? aVectorTests[i].eSwInterpreterState: aVectorTests[i].eState;

            if (eState != eReferenceState)
            {
                std::ostringstream os;
                os << "Unexpected vectorization state: expr: '" << aVectorTests[i].pFormula << "', using software interpreter: " << bForceSwInterpreter;
                CPPUNIT_ASSERT_MESSAGE(os.str().c_str(), false);
            }
            aPos1.IncRow();
        }
    }

    std::shared_ptr< comphelper::ConfigurationChanges > xBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::UseSwInterpreter::set(bSwInterpreter, xBatch);
    officecfg::Office::Common::Misc::UseOpenCL::set(bOpenCL, xBatch);
    xBatch->commit();

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaTokenEquality()
{
    struct FormulaTokenEqualityTest
    {
        const char* mpFormula1;
        const char* mpFormula2;
        bool mbEqual;
    };

    FormulaTokenEqualityTest aTests[] = {
        { "R1C2", "R1C2", true },
        { "R1C2", "R1C3", false },
        { "R1C2", "R2C2", false },
        { "RC2",  "RC[1]", false },
        { "R1C2:R10C2", "R1C2:R10C2", true },
        { "R1C2:R10C2", "R1C2:R11C2", false },
        { "1", "2", false },
        { "RC[1]+1.2", "RC[1]+1.2", true },
        { "RC[1]*0.2", "RC[1]*0.5", false },
        { "\"Test1\"", "\"Test2\"", false },
        { "\"Test\"", "\"Test\"", true },
        { "CONCATENATE(\"Test1\")", "CONCATENATE(\"Test1\")", true },
        { "CONCATENATE(\"Test1\")", "CONCATENATE(\"Test2\")", false },
    };

    formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1;
    for (size_t i = 0; i < SAL_N_ELEMENTS(aTests); ++i)
    {
        ScFormulaCell aCell1(m_pDoc, ScAddress(), OUString::createFromAscii(aTests[i].mpFormula1), eGram);
        ScFormulaCell aCell2(m_pDoc, ScAddress(), OUString::createFromAscii(aTests[i].mpFormula2), eGram);

        ScFormulaCell::CompareState eComp = aCell1.CompareByTokenArray(aCell2);
        if (aTests[i].mbEqual)
        {
            if (eComp == ScFormulaCell::NotEqual)
            {
                std::ostringstream os;
                os << "These two formulas should be evaluated equal: '"
                    << aTests[i].mpFormula1 << "' vs '" << aTests[i].mpFormula2 << "'" << endl;
                CPPUNIT_FAIL(os.str().c_str());
            }
        }
        else
        {
            if (eComp != ScFormulaCell::NotEqual)
            {
                std::ostringstream os;
                os << "These two formulas should be evaluated non-equal: '"
                    << aTests[i].mpFormula1 << "' vs '" << aTests[i].mpFormula2 << "'" << endl;
                CPPUNIT_FAIL(os.str().c_str());
            }
        }
    }
}

void Test::testFormulaRefData()
{
    ScAddress aAddr(4,5,3), aPos(2,2,2);
    ScSingleRefData aRef;
    aRef.InitAddress(aAddr);
    CPPUNIT_ASSERT_MESSAGE("Wrong ref data state.", !aRef.IsRowRel() && !aRef.IsColRel() && !aRef.IsTabRel());
    ASSERT_EQUAL_TYPE(SCCOL, 4, aRef.Col());
    ASSERT_EQUAL_TYPE(SCROW, 5, aRef.Row());
    ASSERT_EQUAL_TYPE(SCTAB, 3, aRef.Tab());

    aRef.SetRowRel(true);
    aRef.SetColRel(true);
    aRef.SetTabRel(true);
    aRef.SetAddress(aAddr, aPos);
    ASSERT_EQUAL_TYPE(SCCOL, 2, aRef.Col());
    ASSERT_EQUAL_TYPE(SCROW, 3, aRef.Row());
    ASSERT_EQUAL_TYPE(SCTAB, 1, aRef.Tab());

    // Test extension of range reference.

    ScComplexRefData aDoubleRef;
    aDoubleRef.InitRange(ScRange(2,2,0,4,4,0));

    aRef.InitAddress(ScAddress(6,5,0));

    aDoubleRef.Extend(aRef, ScAddress());
    ScRange aTest = aDoubleRef.toAbs(ScAddress());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start position of extended range.", ScAddress(2,2,0), aTest.aStart);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end position of extended range.", ScAddress(6,5,0), aTest.aEnd);

    ScComplexRefData aDoubleRef2;
    aDoubleRef2.InitRangeRel(ScRange(1,2,0,8,6,0), ScAddress(5,5,0));
    aDoubleRef.Extend(aDoubleRef2, ScAddress(5,5,0));
    aTest = aDoubleRef.toAbs(ScAddress(5,5,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start position of extended range.", ScAddress(1,2,0), aTest.aStart);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end position of extended range.", ScAddress(8,6,0), aTest.aEnd);
}

void Test::testFormulaCompiler()
{
    struct {
        const char* pInput; FormulaGrammar::Grammar eInputGram;
        const char* pOutput; FormulaGrammar::Grammar eOutputGram;
    } aTests[] = {
        { "=B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE, "[.B1]-[.$C2]+[.D$3]-[.$E$4]", FormulaGrammar::GRAM_ODFF },
        { "=B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE, "B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE },
        { "=B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE, "B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE_XL_A1 },
        { "=B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE, "RC[1]-R[1]C3+R3C[3]-R4C5", FormulaGrammar::GRAM_NATIVE_XL_R1C1 },
    };

    for (size_t i = 0, n = SAL_N_ELEMENTS(aTests); i < n; ++i)
    {
        std::unique_ptr<ScTokenArray> pArray;
        {
            pArray.reset(compileFormula(m_pDoc, OUString::createFromAscii(aTests[i].pInput), nullptr, aTests[i].eInputGram));
            CPPUNIT_ASSERT_MESSAGE("Token array shouldn't be NULL!", pArray.get());
        }

        OUString aFormula = toString(*m_pDoc, ScAddress(), *pArray, aTests[i].eOutputGram);
        CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(aTests[i].pOutput), aFormula);
    }
}

void Test::testFormulaCompilerJumpReordering()
{
    struct TokenCheck
    {
        OpCode meOp;
        StackVar meType;
    };

    // Set separators first.
    ScFormulaOptions aOptions;
    aOptions.SetFormulaSepArg(";");
    aOptions.SetFormulaSepArrayCol(";");
    aOptions.SetFormulaSepArrayRow("|");
    getDocShell().SetFormulaOptions(aOptions);

    {
        OUString aInput("=IF(B1;12;\"text\")");

        // Compile formula string first.
        std::unique_ptr<ScTokenArray> pCode(compileFormula(m_pDoc, aInput));
        CPPUNIT_ASSERT(pCode.get());

        // Then generate RPN tokens.
        ScCompiler aCompRPN(m_pDoc, ScAddress(), *pCode);
        aCompRPN.SetGrammar(FormulaGrammar::GRAM_NATIVE);
        aCompRPN.CompileTokenArray();

        // RPN tokens should be ordered: B1, ocIf, C1, ocSep, D1, ocClose.
        TokenCheck aCheckRPN[] =
        {
            { ocPush,  svSingleRef },
            { ocIf,    static_cast<formula::StackVar>(0) },
            { ocPush,  svDouble    },
            { ocSep,   static_cast<formula::StackVar>(0) },
            { ocPush,  svString    },
            { ocClose, static_cast<formula::StackVar>(0) },
        };

        sal_uInt16 nLen = pCode->GetCodeLen();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong RPN token count.", static_cast<sal_uInt16>(SAL_N_ELEMENTS(aCheckRPN)), nLen);

        FormulaToken** ppTokens = pCode->GetCode();
        for (sal_uInt16 i = 0; i < nLen; ++i)
        {
            const FormulaToken* p = ppTokens[i];
            CPPUNIT_ASSERT_EQUAL(aCheckRPN[i].meOp, p->GetOpCode());
            if (aCheckRPN[i].meOp == ocPush)
                CPPUNIT_ASSERT_EQUAL(static_cast<int>(aCheckRPN[i].meType), static_cast<int>(p->GetType()));
        }

        // Generate RPN tokens again, but this time no jump command reordering.
        pCode->DelRPN();
        ScCompiler aCompRPN2(m_pDoc, ScAddress(), *pCode);
        aCompRPN2.SetGrammar(FormulaGrammar::GRAM_NATIVE);
        aCompRPN2.EnableJumpCommandReorder(false);
        aCompRPN2.CompileTokenArray();

        TokenCheck aCheckRPN2[] =
        {
            { ocPush,  svSingleRef },
            { ocPush,  svDouble    },
            { ocPush,  svString    },
            { ocIf,    static_cast<formula::StackVar>(0) },
        };

        nLen = pCode->GetCodeLen();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong RPN token count.", static_cast<sal_uInt16>(SAL_N_ELEMENTS(aCheckRPN2)), nLen);
        ppTokens = pCode->GetCode();
        for (sal_uInt16 i = 0; i < nLen; ++i)
        {
            const FormulaToken* p = ppTokens[i];
            CPPUNIT_ASSERT_EQUAL(aCheckRPN2[i].meOp, p->GetOpCode());
            if (aCheckRPN[i].meOp == ocPush)
                CPPUNIT_ASSERT_EQUAL(static_cast<int>(aCheckRPN2[i].meType), static_cast<int>(p->GetType()));
        }
    }
}

void Test::testFormulaRefUpdate()
{
    m_pDoc->InsertTab(0, "Formula");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->SetValue(ScAddress(0,0,0), 2.0); // A1
    m_pDoc->SetString(ScAddress(2,2,0), "=A1");   // C3
    m_pDoc->SetString(ScAddress(2,3,0), "=$A$1"); // C4

    ScAddress aPos(2,2,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "A1", "Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "$A$1", "Wrong formula in C4.");

    // Delete row 2 to push formula cells up (to C2:C3).
    m_pDoc->DeleteRow(ScRange(0,1,0,MAXCOL,1,0));

    aPos = ScAddress(2,1,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "A1", "Wrong formula in C2.");

    aPos = ScAddress(2,2,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "$A$1", "Wrong formula in C3.");

    // Insert one row at row 2 to move them back.
    m_pDoc->InsertRow(ScRange(0,1,0,MAXCOL,1,0));

    aPos = ScAddress(2,2,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "A1", "Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "$A$1", "Wrong formula in C4.");

    // Insert 2 rows at row 1 to shift all of A1 and C3:C4 down.
    m_pDoc->InsertRow(ScRange(0,0,0,MAXCOL,1,0));

    aPos = ScAddress(2,4,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "A3", "Wrong formula in C5.");

    aPos = ScAddress(2,5,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "$A$3", "Wrong formula in C6.");

    // Delete 2 rows at row 1 to shift them back.
    m_pDoc->DeleteRow(ScRange(0,0,0,MAXCOL,1,0));

    aPos = ScAddress(2,2,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "A1", "Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "$A$1", "Wrong formula in C4.");

    // Insert 3 columns at column B. to shift C3:C4 to F3:F4.
    m_pDoc->InsertCol(ScRange(1,0,0,3,MAXROW,0));

    aPos = ScAddress(5,2,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "A1", "Wrong formula in F3.");

    aPos = ScAddress(5,3,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "$A$1", "Wrong formula in F4.");

    // Delete columns B:D to shift them back.
    m_pDoc->DeleteCol(ScRange(1,0,0,3,MAXROW,0));

    aPos = ScAddress(2,2,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "A1", "Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "$A$1", "Wrong formula in C4.");

    // Insert cells over A1:A3 to only shift A1 down to A4.
    m_pDoc->InsertRow(ScRange(0,0,0,0,2,0));

    aPos = ScAddress(2,2,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "A4", "Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "$A$4", "Wrong formula in C4.");

    // .. and back.
    m_pDoc->DeleteRow(ScRange(0,0,0,0,2,0));

    aPos = ScAddress(2,2,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "A1", "Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "$A$1", "Wrong formula in C4.");

    // Delete row 1 which will delete the value cell (A1).
    m_pDoc->DeleteRow(ScRange(0,0,0,MAXCOL,0,0));

    aPos = ScAddress(2,1,0);
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(formula::errNoRef, pFC->GetErrCode());
    aPos = ScAddress(2,2,0);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(formula::errNoRef, pFC->GetErrCode());

    // Clear all and start over.
    clearRange(m_pDoc, ScRange(0,0,0,10,10,0));

    // Test range updates

    // Fill B2:C3 with values.
    m_pDoc->SetValue(ScAddress(1,1,0), 1);
    m_pDoc->SetValue(ScAddress(1,2,0), 2);
    m_pDoc->SetValue(ScAddress(2,1,0), 3);
    m_pDoc->SetValue(ScAddress(2,2,0), 4);

    m_pDoc->SetString(ScAddress(0,5,0), "=SUM(B2:C3)");
    m_pDoc->SetString(ScAddress(0,6,0), "=SUM($B$2:$C$3)");

    aPos = ScAddress(0,5,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(B2:C3)", "Wrong formula in A6.");

    aPos = ScAddress(0,6,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM($B$2:$C$3)", "Wrong formula in A7.");

    // Insert a row at row 1.
    m_pDoc->InsertRow(ScRange(0,0,0,MAXCOL,0,0));

    aPos = ScAddress(0,6,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(B3:C4)", "Wrong formula in A7.");

    aPos = ScAddress(0,7,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM($B$3:$C$4)", "Wrong formula in A8.");

    // ... and back.
    m_pDoc->DeleteRow(ScRange(0,0,0,MAXCOL,0,0));

    aPos = ScAddress(0,5,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(B2:C3)", "Wrong formula in A6.");

    aPos = ScAddress(0,6,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM($B$2:$C$3)", "Wrong formula in A7.");

    // Insert columns B:C to shift only the value range.
    m_pDoc->InsertCol(ScRange(1,0,0,2,MAXROW,0));

    aPos = ScAddress(0,5,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(D2:E3)", "Wrong formula in A6.");

    aPos = ScAddress(0,6,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM($D$2:$E$3)", "Wrong formula in A7.");

    // ... and back.
    m_pDoc->DeleteCol(ScRange(1,0,0,2,MAXROW,0));

    aPos = ScAddress(0,5,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(B2:C3)", "Wrong formula in A6.");

    aPos = ScAddress(0,6,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM($B$2:$C$3)", "Wrong formula in A7.");

    // Insert rows 5:6 to shift the formula cells only.
    m_pDoc->InsertRow(ScRange(0,4,0,MAXCOL,5,0));

    aPos = ScAddress(0,7,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(B2:C3)", "Wrong formula in A8.");

    aPos = ScAddress(0,8,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM($B$2:$C$3)", "Wrong formula in A9.");

    // ... and back.
    m_pDoc->DeleteRow(ScRange(0,4,0,MAXCOL,5,0));

    aPos = ScAddress(0,5,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(B2:C3)", "Wrong formula in A6.");

    aPos = ScAddress(0,6,0);
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM($B$2:$C$3)", "Wrong formula in A7.");

    // Check the values of the formula cells in A6:A7.
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,6,0)));

    // Insert cells over B1:B2 to partially shift value range.
    m_pDoc->InsertRow(ScRange(1,0,0,1,1,0));

    // Check the values of the formula cells in A6:A7 again.
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(ScAddress(0,6,0)));

    // ... and shift them back.
    m_pDoc->DeleteRow(ScRange(1,0,0,1,1,0));

    // The formula cell results should be back too.
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,6,0)));

    // Delete rows 2:3 to completely remove the referenced range.
    m_pDoc->DeleteRow(ScRange(0,1,0,MAXCOL,2,0));

    // Both A4 and A5 should show #REF! errors.
    pFC = m_pDoc->GetFormulaCell(ScAddress(0,3,0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(formula::errNoRef, pFC->GetErrCode());

    pFC = m_pDoc->GetFormulaCell(ScAddress(0,4,0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(formula::errNoRef, pFC->GetErrCode());

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateRange()
{
    m_pDoc->InsertTab(0, "Formula");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    setExpandRefs(false);

    // Set values to B2:C5.
    m_pDoc->SetValue(ScAddress(1,1,0), 1);
    m_pDoc->SetValue(ScAddress(1,2,0), 2);
    m_pDoc->SetValue(ScAddress(1,3,0), 3);
    m_pDoc->SetValue(ScAddress(1,4,0), 4);
    m_pDoc->SetValue(ScAddress(2,1,0), 5);
    m_pDoc->SetValue(ScAddress(2,2,0), 6);
    m_pDoc->SetValue(ScAddress(2,3,0), 7);
    m_pDoc->SetValue(ScAddress(2,4,0), 8);

    // Set formula cells to A7 and A8.
    m_pDoc->SetString(ScAddress(0,6,0), "=SUM(B2:C5)");
    m_pDoc->SetString(ScAddress(0,7,0), "=SUM($B$2:$C$5)");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,6,0), "SUM(B2:C5)", "Wrong formula in A7.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,7,0), "SUM($B$2:$C$5)", "Wrong formula in A8.");

    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,7,0)));

    // Delete row 3. This should shrink the range references by one row.
    m_pDoc->DeleteRow(ScRange(0,2,0,MAXCOL,2,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "SUM(B2:C4)", "Wrong formula in A6.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,6,0), "SUM($B$2:$C$4)", "Wrong formula in A7.");

    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,6,0)));

    // Delete row 4 - bottom of range
    m_pDoc->DeleteRow(ScRange(0,3,0,MAXCOL,3,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,4,0), "SUM(B2:C3)", "Wrong formula in A5.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "SUM($B$2:$C$3)", "Wrong formula in A6.");

    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Delete row 2 - top of range
    m_pDoc->DeleteRow(ScRange(0,1,0,MAXCOL,1,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,3,0), "SUM(B2:C2)", "Wrong formula in A4.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,4,0), "SUM($B$2:$C$2)", "Wrong formula in A5.");

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,4,0)));

    // Clear the range and start over.
    clearRange(m_pDoc, ScRange(0,0,0,20,20,0));

    // Fill C2:F3 with values.
    m_pDoc->SetValue(ScAddress(2,1,0), 1);
    m_pDoc->SetValue(ScAddress(3,1,0), 2);
    m_pDoc->SetValue(ScAddress(4,1,0), 3);
    m_pDoc->SetValue(ScAddress(5,1,0), 4);
    m_pDoc->SetValue(ScAddress(2,2,0), 5);
    m_pDoc->SetValue(ScAddress(3,2,0), 6);
    m_pDoc->SetValue(ScAddress(4,2,0), 7);
    m_pDoc->SetValue(ScAddress(5,2,0), 8);

    // Set formulas to A2 and A3.
    m_pDoc->SetString(ScAddress(0,1,0), "=SUM(C2:F3)");
    m_pDoc->SetString(ScAddress(0,2,0), "=SUM($C$2:$F$3)");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "SUM(C2:F3)", "Wrong formula in A2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,2,0), "SUM($C$2:$F$3)", "Wrong formula in A3.");

    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Delete column D.
    m_pDoc->DeleteCol(ScRange(3,0,0,3,MAXROW,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "SUM(C2:E3)", "Wrong formula in A2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,2,0), "SUM($C$2:$E$3)", "Wrong formula in A3.");

    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Delete column E - the right edge of reference range.
    m_pDoc->DeleteCol(ScRange(4,0,0,4,MAXROW,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "SUM(C2:D3)", "Wrong formula in A2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,2,0), "SUM($C$2:$D$3)", "Wrong formula in A3.");

    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Delete column C - the left edge of reference range.
    m_pDoc->DeleteCol(ScRange(2,0,0,2,MAXROW,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "SUM(C2:C3)", "Wrong formula in A2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,2,0), "SUM($C$2:$C$3)", "Wrong formula in A3.");

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Clear the range and start over.
    clearRange(m_pDoc, ScRange(0,0,0,20,20,0));

    // Disable expansion of range reference on insertion in adjacent areas.
    setExpandRefs(false);

    // Fill C2:D3 with values.
    m_pDoc->SetValue(ScAddress(2,1,0), 1);
    m_pDoc->SetValue(ScAddress(3,1,0), 2);
    m_pDoc->SetValue(ScAddress(2,2,0), 3);
    m_pDoc->SetValue(ScAddress(3,2,0), 4);

    // Set formulas at A5 and A6.
    m_pDoc->SetString(ScAddress(0,4,0), "=SUM(C2:D3)");
    m_pDoc->SetString(ScAddress(0,5,0), "=SUM($C$2:$D$3)");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,4,0), "SUM(C2:D3)", "Wrong formula in A5.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "SUM($C$2:$D$3)", "Wrong formula in A6.");

    // Insert a column at column C. This should simply shift the reference without expansion.
    m_pDoc->InsertCol(ScRange(2,0,0,2,MAXROW,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,4,0), "SUM(D2:E3)", "Wrong formula in A5.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "SUM($D$2:$E$3)", "Wrong formula in A6.");

    // Shift it back.
    m_pDoc->DeleteCol(ScRange(2,0,0,2,MAXROW,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,4,0), "SUM(C2:D3)", "Wrong formula in A5.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "SUM($C$2:$D$3)", "Wrong formula in A6.");

    // Insert at column D. This should expand the reference by one column length.
    m_pDoc->InsertCol(ScRange(3,0,0,3,MAXROW,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,4,0), "SUM(C2:E3)", "Wrong formula in A5.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "SUM($C$2:$E$3)", "Wrong formula in A6.");

    // Insert at column F. No expansion should occur since the edge expansion is turned off.
    m_pDoc->InsertCol(ScRange(5,0,0,5,MAXROW,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,4,0), "SUM(C2:E3)", "Wrong formula in A5.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "SUM($C$2:$E$3)", "Wrong formula in A6.");

    // Insert at row 2. No expansion should occur with edge expansion turned off.
    m_pDoc->InsertRow(ScRange(0,1,0,MAXCOL,1,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "SUM(C3:E4)", "Wrong formula in A6.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,6,0), "SUM($C$3:$E$4)", "Wrong formula in A7.");

    // Insert at row 4 to expand the reference range.
    m_pDoc->InsertRow(ScRange(0,3,0,MAXCOL,3,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,6,0), "SUM(C3:E5)", "Wrong formula in A7.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,7,0), "SUM($C$3:$E$5)", "Wrong formula in A8.");

    // Insert at row 6. No expansion with edge expansion turned off.
    m_pDoc->InsertRow(ScRange(0,5,0,MAXCOL,5,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,7,0), "SUM(C3:E5)", "Wrong formula in A8.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,8,0), "SUM($C$3:$E$5)", "Wrong formula in A9.");

    // Clear the range and start over.
    clearRange(m_pDoc, ScRange(0,0,0,20,20,0));

    // Turn edge expansion on.
    setExpandRefs(true);

    // Fill C6:D7 with values.
    m_pDoc->SetValue(ScAddress(2,5,0), 1);
    m_pDoc->SetValue(ScAddress(2,6,0), 2);
    m_pDoc->SetValue(ScAddress(3,5,0), 3);
    m_pDoc->SetValue(ScAddress(3,6,0), 4);

    // Set formulas at A2 and A3.
    m_pDoc->SetString(ScAddress(0,1,0), "=SUM(C6:D7)");
    m_pDoc->SetString(ScAddress(0,2,0), "=SUM($C$6:$D$7)");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "SUM(C6:D7)", "Wrong formula in A2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,2,0), "SUM($C$6:$D$7)", "Wrong formula in A3.");

    // Insert at column E. This should expand the reference range by one column.
    m_pDoc->InsertCol(ScRange(4,0,0,4,MAXROW,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "SUM(C6:E7)", "Wrong formula in A2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,2,0), "SUM($C$6:$E$7)", "Wrong formula in A3.");

    // Insert at column C to edge-expand the reference range.
    m_pDoc->InsertCol(ScRange(2,0,0,2,MAXROW,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "SUM(C6:F7)", "Wrong formula in A2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,2,0), "SUM($C$6:$F$7)", "Wrong formula in A3.");

    // Insert at row 8 to edge-expand.
    m_pDoc->InsertRow(ScRange(0,7,0,MAXCOL,7,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "SUM(C6:F8)", "Wrong formula in A2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,2,0), "SUM($C$6:$F$8)", "Wrong formula in A3.");

    // Insert at row 6 to edge-expand.
    m_pDoc->InsertRow(ScRange(0,5,0,MAXCOL,5,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "SUM(C6:F9)", "Wrong formula in A2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,2,0), "SUM($C$6:$F$9)", "Wrong formula in A3.");

    m_pDoc->InsertTab(1, "StickyRange");

    // A3:A18 all possible combinations of relative and absolute addressing,
    // leaving one row above and below unreferenced.
    ScAddress aPos(0,2,1);
    m_pDoc->SetString( aPos, "=B2:B1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=B2:B$1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=B2:$B1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=B2:$B$1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=B$2:B1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=B$2:B$1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=B$2:$B1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=B$2:$B$1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=$B2:B1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=$B2:B$1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=$B2:$B1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=$B2:$B$1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=$B$2:B1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=$B$2:B$1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=$B$2:$B1048575");
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=$B$2:$B$1048575");
    aPos.IncRow();
    // A19 reference to two cells on one row.
    m_pDoc->SetString( aPos, "=B1048575:C1048575");
    aPos.IncRow();

    // Insert 2 rows in the middle to shift bottom reference down and make it
    // sticky.
    m_pDoc->InsertRow( ScRange( 0, aPos.Row(), 1, MAXCOL, aPos.Row()+1, 1));

    // A3:A18 must not result in #REF! anywhere.
    bool bCheck = true;
    aPos.Set(0,2,1);
    bCheck &= checkFormula(*m_pDoc, aPos, "B2:B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B2:B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B2:$B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B2:$B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B$2:B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B$2:B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B$2:$B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B$2:$B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B2:B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B2:B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B2:$B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B2:$B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B$2:B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B$2:B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B$2:$B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B$2:$B$1048576");
    aPos.IncRow();
    if (!bCheck)
        CPPUNIT_FAIL("Wrong reference in A3:A18 after insertion.");

    // A19 reference to one row shifted out should be #REF!
    bCheck &= checkFormula(*m_pDoc, aPos, "B#REF!:C#REF!");
    if (!bCheck)
        CPPUNIT_FAIL("Wrong reference in A19 after insertion.");
    // A19 enter reference to last row.
    m_pDoc->SetString( aPos, "=B1048576:C1048576");
    aPos.IncRow();

    // Delete row 1 to shift top reference up, bottom reference stays sticky.
    m_pDoc->DeleteRow(ScRange(0,0,1,MAXCOL,0,1));

    // Check sticky bottom references and display of entire column references,
    // now in A2:A17.
    bCheck = true;
    aPos.Set(0,1,1);
    bCheck &= checkFormula(*m_pDoc, aPos, "B:B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B1:B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B:$B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B1:$B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B$1:B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B:B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B$1:$B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B:$B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B:B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B1:B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B:$B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B1:$B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B$1:B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B:B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B$1:$B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B:$B");
    aPos.IncRow();
    if (!bCheck)
        CPPUNIT_FAIL("Wrong reference in A2:A17 after deletion.");

    // A18 reference to one last row should be shifted up.
    bCheck &= checkFormula(*m_pDoc, aPos, "B1048575:C1048575");
    if (!bCheck)
        CPPUNIT_FAIL("Wrong reference in A18 after deletion.");
    aPos.IncRow();

    // Insert 4 rows in the middle.
    m_pDoc->InsertRow( ScRange( 0, aPos.Row(), 1, MAXCOL, aPos.Row()+3, 1));
    // Delete 2 rows in the middle.
    m_pDoc->DeleteRow( ScRange( 0, aPos.Row(), 1, MAXCOL, aPos.Row()+1, 1));

    // References in A2:A17 must still be the same.
    bCheck = true;
    aPos.Set(0,1,1);
    bCheck &= checkFormula(*m_pDoc, aPos, "B:B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B1:B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B:$B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B1:$B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B$1:B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B:B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B$1:$B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "B:$B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B:B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B1:B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B:$B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B1:$B$1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B$1:B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B:B");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B$1:$B1048576");
    aPos.IncRow();
    bCheck &= checkFormula(*m_pDoc, aPos, "$B:$B");
    aPos.IncRow();
    if (!bCheck)
        CPPUNIT_FAIL("Wrong reference in A2:A17 after deletion.");

    m_pDoc->DeleteTab(1);

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateSheets()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    OUString aName;
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aName);
    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    // Set values to B2:C3 on sheet Sheet1.
    m_pDoc->SetValue(ScAddress(1,1,0), 1);
    m_pDoc->SetValue(ScAddress(1,2,0), 2);
    m_pDoc->SetValue(ScAddress(2,1,0), 3);
    m_pDoc->SetValue(ScAddress(2,2,0), 4);

    // Set formulas to B2 and B3 on sheet Sheet2.
    m_pDoc->SetString(ScAddress(1,1,1), "=SUM(Sheet1.B2:C3)");
    m_pDoc->SetString(ScAddress(1,2,1), "=SUM($Sheet1.$B$2:$C$3)");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    // Swap the sheets.
    m_pDoc->MoveTab(0, 1);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);
    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aName);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,0), "SUM(Sheet1.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,0), "SUM($Sheet1.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    // Swap back.
    m_pDoc->MoveTab(0, 1);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aName);
    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    // Insert a new sheet between the two.
    m_pDoc->InsertTab(1, "Temp");

    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Temp"), aName);
    m_pDoc->GetName(2, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,2), "SUM(Sheet1.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,2), "SUM($Sheet1.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    // Move the last sheet (Sheet2) to the first position.
    m_pDoc->MoveTab(2, 0);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,0), "SUM(Sheet1.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,0), "SUM($Sheet1.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    // Move back.
    m_pDoc->MoveTab(0, 2);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,2), "SUM(Sheet1.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,2), "SUM($Sheet1.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    // Move the "Temp" sheet to the last position.
    m_pDoc->MoveTab(1, 2);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    // Move back.
    m_pDoc->MoveTab(2, 1);

    // Delete the temporary sheet.
    m_pDoc->DeleteTab(1);

    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    // Insert a new sheet before the first one.
    m_pDoc->InsertTab(0, "Temp");

    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aName);
    m_pDoc->GetName(2, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,2), "SUM(Sheet1.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,2), "SUM($Sheet1.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    // Delete the temporary sheet.
    m_pDoc->DeleteTab(0);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    // Append a bunch of sheets.
    m_pDoc->InsertTab(2, "Temp1");
    m_pDoc->InsertTab(3, "Temp2");
    m_pDoc->InsertTab(4, "Temp3");

    // Move these tabs around. This shouldn't affects the first 2 sheets.
    m_pDoc->MoveTab(2, 4);
    m_pDoc->MoveTab(3, 2);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    // Delete the temp sheets.
    m_pDoc->DeleteTab(4);
    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);

    // Delete Sheet1.
    m_pDoc->DeleteTab(0);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,0), "SUM(#REF!.B2:C3)", "Wrong formula in Sheet2.B2.");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,0), "SUM($#REF!.$B$2:$C$3)", "Wrong formula in Sheet2.B3.");

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateInsertRows()
{
    setExpandRefs(false);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Formula");

    // Insert raw values in B2:B4.
    m_pDoc->SetValue(ScAddress(1,1,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,2,0), 2.0);
    m_pDoc->SetValue(ScAddress(1,3,0), 3.0);

    // Insert a formula in B5 to sum up B2:B4.
    m_pDoc->SetString(ScAddress(1,4,0), "=SUM(B2:B4)");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    // Insert rows over rows 1:2.
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    rFunc.InsertCells(ScRange(0,0,0,MAXCOL,1,0), &aMark, INS_INSROWS_BEFORE, false, true);

    // The raw data should have shifted to B4:B6.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,4,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,5,0)));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,6,0), "SUM(B4:B6)", "Wrong formula!");

    // Clear and start over.
    clearSheet(m_pDoc, 0);

    // Set raw values in A4:A6.
    m_pDoc->SetValue(ScAddress(0,3,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,4,0), 2.0);
    m_pDoc->SetValue(ScAddress(0,5,0), 3.0);

    // Set formula in A3 to reference A4:A6.
    m_pDoc->SetString(ScAddress(0,2,0), "=MAX(A4:A6)");

    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Insert 3 rows over 2:4.  This should push A3:A6 to A6:A9.
    rFunc.InsertCells(ScRange(0,1,0,MAXCOL,3,0), &aMark, INS_INSROWS_BEFORE, false, true);
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0,5,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula cell should not be an error.", static_cast<sal_uInt16>(0), pFC->GetErrCode());
    ASSERT_DOUBLES_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "MAX(A7:A9)", "Wrong formula!");

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateSheetsDelete()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");
    m_pDoc->InsertTab(2, "Sheet3");
    m_pDoc->InsertTab(3, "Sheet4");

    m_pDoc->SetString(ScAddress(4,1,0), "=SUM(Sheet2.A4:Sheet4.A4)");
    m_pDoc->SetString(ScAddress(4,2,0), "=SUM($Sheet2.A4:$Sheet4.A4)");
    m_pDoc->DeleteTab(1);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(4,1,0), "SUM(Sheet3.A4:Sheet4.A4)", "Wrong Formula");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(4,2,0), "SUM($Sheet3.A4:$Sheet4.A4)", "Wrong Formula");

    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetString(ScAddress(5,1,3), "=SUM(Sheet1.A5:Sheet3.A5)");
    m_pDoc->SetString(ScAddress(5,2,3), "=SUM($Sheet1.A5:$Sheet3.A5)");
    m_pDoc->DeleteTab(2);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(5,1,2), "SUM(Sheet1.A5:Sheet2.A5)", "Wrong Formula");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(5,2,2), "SUM($Sheet1.A5:$Sheet2.A5)", "Wrong Formula");

    m_pDoc->InsertTab(2, "Sheet3");

    m_pDoc->SetString(ScAddress(6,1,3), "=SUM(Sheet1.A6:Sheet3.A6)");
    m_pDoc->SetString(ScAddress(6,2,3), "=SUM($Sheet1.A6:$Sheet3.A6)");
    m_pDoc->DeleteTabs(0,3);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(6,1,0), "SUM(#REF!.A6:#REF!.A6)", "Wrong Formula");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(6,2,0), "SUM($#REF!.A6:$#REF!.A6)", "Wrong Formula");

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");
    m_pDoc->InsertTab(2, "Sheet3");

    m_pDoc->SetString(ScAddress(1,1,1), "=SUM(Sheet1.A2:Sheet3.A2");
    m_pDoc->SetString(ScAddress(2,1,1), "=SUM(Sheet1.A1:Sheet2.A1");
    m_pDoc->SetString(ScAddress(3,1,1), "=SUM(Sheet2.A3:Sheet4.A3");

    m_pDoc->SetString(ScAddress(1,2,1), "=SUM($Sheet1.A2:$Sheet3.A2");
    m_pDoc->SetString(ScAddress(2,2,1), "=SUM($Sheet1.A1:$Sheet2.A1");
    m_pDoc->SetString(ScAddress(3,2,1), "=SUM($Sheet2.A3:$Sheet4.A3");

    m_pDoc->DeleteTab(2);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.A2:Sheet2.A2)", "Wrong Formula");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(2,1,1), "SUM(Sheet1.A1:Sheet2.A1)", "Wrong Formula");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(3,1,1), "SUM(Sheet2.A3:Sheet4.A3)", "Wrong Formula");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.A2:$Sheet2.A2)", "Wrong Formula");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(2,2,1), "SUM($Sheet1.A1:$Sheet2.A1)", "Wrong Formula");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(3,2,1), "SUM($Sheet2.A3:$Sheet4.A3)", "Wrong Formula");

    m_pDoc->DeleteTab(0);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,0), "SUM(Sheet2.A2:Sheet2.A2)", "Wrong Formula");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(2,1,0), "SUM(Sheet2.A1:Sheet2.A1)", "Wrong Formula");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(3,1,0), "SUM(Sheet2.A3:Sheet4.A3)", "Wrong Formula");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,0), "SUM($Sheet2.A2:$Sheet2.A2)", "Wrong Formula");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(2,2,0), "SUM($Sheet2.A1:$Sheet2.A1)", "Wrong Formula");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(3,2,0), "SUM($Sheet2.A3:$Sheet4.A3)", "Wrong Formula");

    m_pDoc->DeleteTab(0);
    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateInsertColumns()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    setExpandRefs(false);

    m_pDoc->InsertTab(0, "Formula");

    // Set named range for B2 with absolute column and relative same row.
    const ScAddress aNamePos(0,1,0);
    bool bInserted = m_pDoc->InsertNewRangeName("RowRelativeRange", aNamePos, "$Formula.$B2");
    CPPUNIT_ASSERT(bInserted);

    // Set named range for entire absolute column B.
    bInserted = m_pDoc->InsertNewRangeName("EntireColumn", aNamePos, "$B:$B");
    CPPUNIT_ASSERT(bInserted);

    // Set named range for entire absolute row 2.
    bInserted = m_pDoc->InsertNewRangeName("EntireRow", aNamePos, "$2:$2");
    CPPUNIT_ASSERT(bInserted);

    // Set values in B1:B3.
    m_pDoc->SetValue(ScAddress(1,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,1,0), 2.0);
    m_pDoc->SetValue(ScAddress(1,2,0), 3.0);

    // Reference them in B4.
    m_pDoc->SetString(ScAddress(1,3,0), "=SUM(B1:B3)");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,3,0)));

    // Use named range in C2 to reference B2.
    m_pDoc->SetString(ScAddress(2,1,0), "=RowRelativeRange");
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,1,0)));

    // Use named range in C3 to reference column B, values in B1,B2,B3,B4
    m_pDoc->SetString(ScAddress(2,2,0), "=SUM(EntireColumn)");
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(2,2,0)));

    // Use named range in C4 to reference row 2, values in B2 and C2.
    m_pDoc->SetString(ScAddress(2,3,0), "=SUM(EntireRow)");
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(2,3,0)));

    // Inert columns over A:B.
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    rFunc.InsertCells(ScRange(0,0,0,1,MAXROW,0), &aMark, INS_INSCOLS_BEFORE, false, true);

    // Now, the original column B has moved to column D.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(3,3,0), "SUM(D1:D3)", "Wrong formula in D4 after column insertion.");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(3,3,0)));

    // Check that the named reference points to the moved cell, now D2.
    ScRangeData* pName = m_pDoc->GetRangeName()->findByUpperName("ROWRELATIVERANGE");
    CPPUNIT_ASSERT(pName);
    OUString aSymbol;
    pName->GetSymbol(aSymbol, aNamePos, formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL(OUString("$Formula.$D2"), aSymbol);

    // Check that the formula using the name, now in E2, still has the same result.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(4,1,0), "RowRelativeRange", "Wrong formula in E2 after column insertion.");

    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(4,1,0)));

    // Check that the named column reference points to the moved column, now D.
    pName = m_pDoc->GetRangeName()->findByUpperName("ENTIRECOLUMN");
    CPPUNIT_ASSERT(pName);
    pName->GetSymbol(aSymbol, aNamePos, formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL(OUString("$D:$D"), aSymbol);

    // Check that the formula using the name, now in E3, still has the same result.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(4,2,0), "SUM(EntireColumn)", "Wrong formula in E3 after column insertion.");

    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(4,2,0)));

    // Check that the named row reference still points to the same entire row
    // and does not have a #REF! error due to inserted columns.
    pName = m_pDoc->GetRangeName()->findByUpperName("ENTIREROW");
    CPPUNIT_ASSERT(pName);
    pName->GetSymbol(aSymbol, aNamePos, formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL(OUString("$2:$2"), aSymbol);

    // Check that the formula using the name, now in E4, still has the same result.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(4,3,0), "SUM(EntireRow)", "Wrong formula in E4 after column insertion.");

    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(4,3,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateMove()
{
    m_pDoc->InsertTab(0, "Sheet1");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    // Set value to B4:B6.
    m_pDoc->SetValue(ScAddress(1,3,0), 1);
    m_pDoc->SetValue(ScAddress(1,4,0), 2);
    m_pDoc->SetValue(ScAddress(1,5,0), 3);

    // Set formulas to A9:A12 that references B4:B6.
    m_pDoc->SetString(ScAddress(0,8,0), "=SUM(B4:B6)");
    m_pDoc->SetString(ScAddress(0,9,0), "=SUM($B$4:$B$6)");
    m_pDoc->SetString(ScAddress(0,10,0), "=B5");
    m_pDoc->SetString(ScAddress(0,11,0), "=$B$6");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,8,0));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,9,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0,10,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(0,11,0));

    // Move B4:B6 to D4 (two columns to the right).
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(1,3,0,1,5,0), ScAddress(3,3,0), true, false, false, false);
    CPPUNIT_ASSERT_MESSAGE("Failed to move B4:B6.", bMoved);

    // The results of the formula cells that reference the moved range should remain the same.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,8,0));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,9,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0,10,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(0,11,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,8,0), "SUM(D4:D6)", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,9,0), "SUM($D$4:$D$6)", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,10,0), "D5", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,11,0), "$D$6", "Wrong formula.");

    // Move A9:A12 to B10:B13.
    bMoved = rFunc.MoveBlock(ScRange(0,8,0,0,11,0), ScAddress(1,9,0), true, false, false, false);
    CPPUNIT_ASSERT_MESSAGE("Failed to move A9:A12 to B10:B13", bMoved);

    // The results of these formula cells should still stay the same.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(1,9,0));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(1,10,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1,11,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,12,0));

    // Displayed formulas should stay the same since the referenced range hasn't moved.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,9,0), "SUM(D4:D6)", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,10,0), "SUM($D$4:$D$6)", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,11,0), "D5", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,12,0), "$D$6", "Wrong formula.");

    // The value cells are in D4:D6. Move D4:D5 to the right but leave D6
    // where it is.
    bMoved = rFunc.MoveBlock(ScRange(3,3,0,3,4,0), ScAddress(4,3,0), true, false, false, false);
    CPPUNIT_ASSERT_MESSAGE("Failed to move D4:D5 to E4:E5", bMoved);

    // Only the values of B10 and B11 should be updated.
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,9,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,10,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1,11,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,12,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,9,0), "SUM(D4:D6)", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,10,0), "SUM($D$4:$D$6)", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,11,0), "E5", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,12,0), "$D$6", "Wrong formula.");

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateMoveUndo()
{
    m_pDoc->InsertTab(0, "Test");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    // Set values in A1:A4.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 2.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 3.0);
    m_pDoc->SetValue(ScAddress(0,3,0), 4.0);

    // Set formulas with single cell references in A6:A8.
    m_pDoc->SetString(ScAddress(0,5,0), "=A1");
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "A1", "Wrong formula.");

    m_pDoc->SetString(ScAddress(0,6,0), "=A1+A2+A3");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,6,0), "A1+A2+A3", "Wrong formula.");

    m_pDoc->SetString(ScAddress(0,7,0), "=A1+A3+A4");
    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(0,7,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,7,0), "A1+A3+A4", "Wrong formula.");

    // Set formulas with range references in A10:A12.
    m_pDoc->SetString(ScAddress(0,9,0), "=SUM(A1:A2)");
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,9,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,9,0), "SUM(A1:A2)", "Wrong formula.");

    m_pDoc->SetString(ScAddress(0,10,0), "=SUM(A1:A3)");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,10,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,10,0), "SUM(A1:A3)", "Wrong formula.");

    m_pDoc->SetString(ScAddress(0,11,0), "=SUM(A1:A4)");
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,11,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,11,0), "SUM(A1:A4)", "Wrong formula.");

    // Move A1:A3 to C1:C3. Note that A4 remains.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(0,0,0,0,2,0), ScAddress(2,0,0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "C1", "Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,6,0), "C1+C2+C3", "Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(0,7,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,7,0), "C1+C3+A4", "Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,9,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,9,0), "SUM(C1:C2)", "Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,10,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,10,0), "SUM(C1:C3)", "Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(0,11,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,11,0), "SUM(A1:A4)", "Wrong formula.");

    // Undo the move.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,5,0), "A1", "Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,6,0), "A1+A2+A3", "Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(0,7,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,7,0), "A1+A3+A4", "Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,9,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,9,0), "SUM(A1:A2)", "Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,10,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,10,0), "SUM(A1:A3)", "Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,11,0)));
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,11,0), "SUM(A1:A4)","Wrong formula." );

    // Make sure the broadcasters are still valid by changing the value of A1.
    m_pDoc->SetValue(ScAddress(0,0,0), 20);

    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL(25.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    CPPUNIT_ASSERT_EQUAL(27.0, m_pDoc->GetValue(ScAddress(0,7,0)));

    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(ScAddress(0,9,0)));
    CPPUNIT_ASSERT_EQUAL(25.0, m_pDoc->GetValue(ScAddress(0,10,0)));
    CPPUNIT_ASSERT_EQUAL(29.0, m_pDoc->GetValue(ScAddress(0,11,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateMoveToSheet()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    // Set values to A1:A2 on Sheet1, and B1:B2 to reference them.
    m_pDoc->SetValue(ScAddress(0,0,0), 11);
    m_pDoc->SetValue(ScAddress(0,1,0), 12);
    m_pDoc->SetString(ScAddress(1,0,0), "=A1");
    m_pDoc->SetString(ScAddress(1,1,0), "=A2");

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,0,0), "A1", "Wrong formula");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,0), "A2", "Wrong formula");

    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(1,1,0)));

    // Move A1:A2 on Sheet1 to B3:B4 on Sheet2.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(0,0,0,0,1,0), ScAddress(1,2,1), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,0,0), "Sheet2.B3", "Wrong formula");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,0), "Sheet2.B4", "Wrong formula");

    // Undo and check again.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    pUndoMgr->Undo();

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,0,0), "A1", "Wrong formula");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,0), "A2", "Wrong formula");

    // Redo and check.
    pUndoMgr->Redo();

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,0,0), "Sheet2.B3", "Wrong formula");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,0), "Sheet2.B4", "Wrong formula");

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateDeleteContent()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Test");

    // Set value in B2.
    m_pDoc->SetValue(ScAddress(1,1,0), 2.0);
    // Set formula in C2 to reference B2.
    m_pDoc->SetString(ScAddress(2,1,0), "=B2");

    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,1,0)));

    // Delete B2.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    ScMarkData aMark;
    aMark.SetMarkArea(ScAddress(1,1,0));
    rFunc.DeleteContents(aMark, InsertDeleteFlags::CONTENTS, true, true);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("B2 should be empty.", CELLTYPE_NONE, m_pDoc->GetCellType(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(2,1,0)));

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    // Undo and check the result of C2.
    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0))); // B2
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,1,0))); // C2

    // Redo and check.
    pUndoMgr->Redo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("B2 should be empty.", CELLTYPE_NONE, m_pDoc->GetCellType(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(2,1,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateDeleteAndShiftLeft()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Test");

    // Insert 1,2,3,4,5 in C1:G1.
    for (SCCOL i = 0; i <= 4; ++i)
        m_pDoc->SetValue(ScAddress(i+2,0,0), i+1);

    // Insert formula in H1.
    ScAddress aPos(7,0,0);
    m_pDoc->SetString(aPos, "=SUM(C1:G1)");

    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));

    // Delete columns D:E (middle of the reference).
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    bool bDeleted = rFunc.DeleteCells(ScRange(3,0,0,4,MAXROW,0), &aMark, DEL_CELLSLEFT, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncCol(-2);
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(C1:E1)", "Wrong formula!");

    // Undo and check.
    SfxUndoManager* pUndo = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndo);

    pUndo->Undo();
    aPos.IncCol(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(C1:G1)", "Wrong formula!");

    // Delete columns C:D (left end of the reference).
    bDeleted = rFunc.DeleteCells(ScRange(2,0,0,3,MAXROW,0), &aMark, DEL_CELLSLEFT, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncCol(-2);
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(C1:E1)", "Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    aPos.IncCol(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(C1:G1)", "Wrong formula!");

    // Delete columns B:E (overlaps on the left).
    bDeleted = rFunc.DeleteCells(ScRange(1,0,0,4,MAXROW,0), &aMark, DEL_CELLSLEFT, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncCol(-4);
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(B1:C1)", "Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    aPos.IncCol(4);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(C1:G1)", "Wrong formula!");

    // Start over with a new scenario.
    clearSheet(m_pDoc, 0);

    // Insert 1,2,3,4,5,6 into C1:H1.
    for (SCCOL i = 0; i <= 5; ++i)
        m_pDoc->SetValue(ScAddress(i+2,0,0), i+1);

    // Set formula in B1.
    aPos = ScAddress(1,0,0);
    m_pDoc->SetString(aPos, "=SUM(C1:H1)");
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));

    // Delete columns F:H (right end of the reference).
    bDeleted = rFunc.DeleteCells(ScRange(5,0,0,7,MAXROW,0), &aMark, DEL_CELLSLEFT, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(C1:E1)", "Wrong formula!");

    // Undo and check.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(C1:H1)", "Wrong formula!");

    // Delete columns G:I (overlaps on the right).
    bDeleted = rFunc.DeleteCells(ScRange(6,0,0,8,MAXROW,0), &aMark, DEL_CELLSLEFT, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(C1:F1)", "Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(C1:H1)", "Wrong formula!");

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateDeleteAndShiftUp()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Test");

    // Insert 1,2,3,4,5 in A3:A7.
    for (SCROW i = 0; i <= 4; ++i)
        m_pDoc->SetValue(ScAddress(0,i+2,0), i+1);

    // Insert formula in A8.
    ScAddress aPos(0,7,0);
    m_pDoc->SetString(aPos, "=SUM(A3:A7)");

    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));

    // Delete rows 4:5 (middle of the reference).
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    bool bDeleted = rFunc.DeleteCells(ScRange(0,3,0,MAXCOL,4,0), &aMark, DEL_CELLSUP, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncRow(-2);
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(A3:A5)", "Wrong formula!");

    // Undo and check.
    SfxUndoManager* pUndo = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndo);

    pUndo->Undo();
    aPos.IncRow(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(A3:A7)", "Wrong formula!");

    // Delete rows 3:4 (top end of the reference).
    bDeleted = rFunc.DeleteCells(ScRange(0,2,0,MAXCOL,3,0), &aMark, DEL_CELLSUP, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncRow(-2);
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(A3:A5)", "Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    aPos.IncRow(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(A3:A7)", "Wrong formula!");

    // Delete rows 2:5 (overlaps on the top).
    bDeleted = rFunc.DeleteCells(ScRange(0,1,0,MAXCOL,4,0), &aMark, DEL_CELLSUP, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncRow(-4);
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(A2:A3)", "Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    aPos.IncRow(4);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(A3:A7)", "Wrong formula!");

    // Start over with a new scenario.
    clearSheet(m_pDoc, 0);

    // Insert 1,2,3,4,5,6 into A3:A8.
    for (SCROW i = 0; i <= 5; ++i)
        m_pDoc->SetValue(ScAddress(0,i+2,0), i+1);

    // Set formula in B1.
    aPos = ScAddress(0,1,0);
    m_pDoc->SetString(aPos, "=SUM(A3:A8)");
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));

    // Delete rows 6:8 (bottom end of the reference).
    bDeleted = rFunc.DeleteCells(ScRange(0,5,0,MAXCOL,7,0), &aMark, DEL_CELLSUP, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(A3:A5)", "Wrong formula!");

    // Undo and check.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(A3:A8)", "Wrong formula!");

    // Delete rows 7:9 (overlaps on the bottom).
    bDeleted = rFunc.DeleteCells(ScRange(0,6,0,MAXCOL,8,0), &aMark, DEL_CELLSUP, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(A3:A6)", "Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(A3:A8)", "Wrong formula!");

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateName()
{
    m_pDoc->InsertTab(0, "Formula");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    // Fill C2:C5 with values.
    m_pDoc->SetValue(ScAddress(2,1,0), 1);
    m_pDoc->SetValue(ScAddress(2,2,0), 2);
    m_pDoc->SetValue(ScAddress(2,3,0), 3);
    m_pDoc->SetValue(ScAddress(2,4,0), 4);

    // Add a named expression that references the immediate left cell.
    ScRangeName* pGlobalNames = m_pDoc->GetRangeName();
    CPPUNIT_ASSERT_MESSAGE("Failed to obtain global named expression object.", pGlobalNames);
    ScRangeData* pName = new ScRangeData(
        m_pDoc, "ToLeft", "RC[-1]", ScAddress(2,1,0),
        ScRangeData::Type::Name, formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1);

    bool bInserted = pGlobalNames->insert(pName);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert a new name.", bInserted);

    // Insert formulas in D2:D5 using the named expression.
    m_pDoc->SetString(ScAddress(3,1,0), "=ToLeft");
    m_pDoc->SetString(ScAddress(3,2,0), "=ToLeft");
    m_pDoc->SetString(ScAddress(3,3,0), "=ToLeft");
    m_pDoc->SetString(ScAddress(3,4,0), "=ToLeft");

    // Make sure the results are correct.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(3,1,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(3,2,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(3,3,0));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(3,4,0));

    // Push cells in column C down by one cell.
    m_pDoc->InsertRow(ScRange(2,0,0,2,0,0));

    // Make sure the results change accordingly.
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(3,1,0));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(3,2,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(3,3,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(3,4,0));

    // Move cells back.
    m_pDoc->DeleteRow(ScRange(2,0,0,2,0,0));

    // Make sure the results are back as well.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(3,1,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(3,2,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(3,3,0));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(3,4,0));

    // Fill B10:B12 with values.
    m_pDoc->SetValue(ScAddress(1,9,0), 10);
    m_pDoc->SetValue(ScAddress(1,10,0), 11);
    m_pDoc->SetValue(ScAddress(1,11,0), 12);

    // Insert a new named expression that references these values as absolute range.
    pName = new ScRangeData(
        m_pDoc, "MyRange", "$B$10:$B$12", ScAddress(0,0,0), ScRangeData::Type::Name, formula::FormulaGrammar::GRAM_NATIVE);
    bInserted = pGlobalNames->insert(pName);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert a new name.", bInserted);

    // Set formula at C8 that references this named expression.
    m_pDoc->SetString(ScAddress(2,7,0), "=SUM(MyRange)");
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(ScAddress(2,7,0)));

    // Shift B10:B12 to right by 2 columns.
    m_pDoc->InsertCol(ScRange(1,9,0,2,11,0));

    // This should shift the absolute range B10:B12 that MyRange references.
    pName = pGlobalNames->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT_MESSAGE("Failed to find named expression 'MyRange' in the global scope.", pName);
    OUString aExpr;
    pName->GetSymbol(aExpr);
    CPPUNIT_ASSERT_EQUAL(OUString("$D$10:$D$12"), aExpr);

    // This move shouldn't affect the value of C8.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(2,7,0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(ScAddress(2,7,0)));

    // Update the value of D10 and make sure C8 gets updated.
    m_pDoc->SetValue(ScAddress(3,9,0), 20);
    CPPUNIT_ASSERT_EQUAL(43.0, m_pDoc->GetValue(ScAddress(2,7,0)));

    // Insert a new sheet before the current.
    m_pDoc->InsertTab(0, "New");
    OUString aName;
    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Formula"), aName);

    pName = pGlobalNames->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT_MESSAGE("Failed to find named expression 'MyRange' in the global scope.", pName);

    m_pDoc->SetValue(ScAddress(3,9,1), 10);
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(ScAddress(2,7,1)));

    // Delete the inserted sheet, which will shift the 'Formula' sheet to the left.
    m_pDoc->DeleteTab(0);

    aName.clear();
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Formula"), aName);

    pName = pGlobalNames->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT_MESSAGE("Failed to find named expression 'MyRange' in the global scope.", pName);

    m_pDoc->SetValue(ScAddress(3,9,0), 11);
    CPPUNIT_ASSERT_EQUAL(34.0, m_pDoc->GetValue(ScAddress(2,7,0)));

    // Clear all and start over.
    clearRange(m_pDoc, ScRange(0,0,0,100,100,0));
    pGlobalNames->clear();

    pName = new ScRangeData(
        m_pDoc, "MyRange", "$B$1:$C$6", ScAddress(0,0,0), ScRangeData::Type::Name, formula::FormulaGrammar::GRAM_NATIVE);
    bInserted = pGlobalNames->insert(pName);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert a new name.", bInserted);
    pName->GetSymbol(aExpr);
    CPPUNIT_ASSERT_EQUAL(OUString("$B$1:$C$6"), aExpr);

    // Insert range of cells to shift right. The range partially overlaps the named range.
    m_pDoc->InsertCol(ScRange(2,4,0,3,8,0));

    // This should not alter the range.
    pName->GetSymbol(aExpr);
    CPPUNIT_ASSERT_EQUAL(OUString("$B$1:$C$6"), aExpr);

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateNameMove()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Test");

    // Set values to B2:B4.
    m_pDoc->SetValue(ScAddress(1,1,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,2,0), 2.0);
    m_pDoc->SetValue(ScAddress(1,3,0), 3.0);

    // Set named range for B2:B4.
    bool bInserted = m_pDoc->InsertNewRangeName("MyRange", ScAddress(0,0,0), "$Test.$B$2:$B$4");
    CPPUNIT_ASSERT(bInserted);

    // Set formula in A10.
    m_pDoc->SetString(ScAddress(0,9,0), "=SUM(MyRange)");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,9,0)));

    ScRangeData* pData = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pData);
    OUString aSymbol;
    pData->GetSymbol(aSymbol, m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2:$B$4"), aSymbol);

    // Move B2:B4 to D3.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(1,1,0,1,3,0), ScAddress(3,2,0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    // The named range should have moved as well.
    pData->GetSymbol(aSymbol, m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$D$3:$D$5"), aSymbol);

    // The value of A10 should remain unchanged.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,9,0)));

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    // Undo and check.
    pUndoMgr->Undo();

    pData = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pData);
    pData->GetSymbol(aSymbol, m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2:$B$4"), aSymbol);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,9,0)));

    // Redo and check.
    pUndoMgr->Redo();

    pData = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pData);
    pData->GetSymbol(aSymbol, m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$D$3:$D$5"), aSymbol);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,9,0)));

    // Undo again to bring it back to the initial condition, and clear the undo buffer.
    pUndoMgr->Undo();
    pUndoMgr->Clear();

    // Add an identical formula to A11 and make a formula group over A10:A11.
    m_pDoc->SetString(ScAddress(0,10,0), "=SUM(MyRange)");
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0,9,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    // Move B2:B4 to D3 again.
    bMoved = rFunc.MoveBlock(ScRange(1,1,0,1,3,0), ScAddress(3,2,0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    // Values of A10 and A11 should remain the same.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,9,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,10,0)));

    // Clear and start over.
    clearSheet(m_pDoc, 0);
    m_pDoc->GetRangeName()->clear();

    // Set value to B2.
    m_pDoc->SetValue(ScAddress(1,1,0), 2.0);

    // Define B2 as 'MyCell'.
    bInserted = m_pDoc->InsertNewRangeName("MyCell", ScAddress(0,0,0), "$Test.$B$2");
    CPPUNIT_ASSERT(bInserted);

    // Set formula to B3 that references B2 via MyCell.
    m_pDoc->SetString(ScAddress(1,2,0), "=MyCell*2");
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    // Move B2 to D2.
    bMoved = rFunc.MoveBlock(ScRange(1,1,0,1,1,0), ScAddress(3,1,0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    // Value in B3 should remain unchanged.
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateNameExpandRef()
{
    setExpandRefs(true);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Test");

    bool bInserted = m_pDoc->InsertNewRangeName("MyRange", ScAddress(0,0,0), "$A$1:$A$3");
    CPPUNIT_ASSERT(bInserted);

    // Set values to A1:A3.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 2.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 3.0);

    m_pDoc->SetString(ScAddress(0,5,0), "=SUM(MyRange)");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Insert a new row at row 4, which should expand the named range to A1:A4.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    rFunc.InsertCells(ScRange(0,3,0,MAXCOL,3,0), &aMark, INS_INSROWS_BEFORE, false, true);
    ScRangeData* pName = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pName);
    OUString aSymbol;
    pName->GetSymbol(aSymbol, m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(OUString("$A$1:$A$4"), aSymbol);

    // Make sure the listening area has been expanded as well.  Note the
    // formula cell has been pushed downward by one cell.
    m_pDoc->SetValue(ScAddress(0,3,0), 4.0);
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,6,0)));

    // Clear the document and start over.
    m_pDoc->GetRangeName()->clear();
    clearSheet(m_pDoc, 0);

    // Set values to B4:B6.
    m_pDoc->SetValue(ScAddress(1,3,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,4,0), 2.0);
    m_pDoc->SetValue(ScAddress(1,5,0), 3.0);

    bInserted = m_pDoc->InsertNewRangeName("MyRange", ScAddress(0,0,0), "$B$4:$B$6");
    CPPUNIT_ASSERT(bInserted);

    // Set formula to A1.
    m_pDoc->SetString(ScAddress(0,0,0), "=SUM(MyRange)");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,0,0));

    // Insert rows over 3:5 which should expand the range by 3 rows.
    rFunc.InsertCells(ScRange(0,2,0,MAXCOL,4,0), &aMark, INS_INSROWS_BEFORE, false, true);

    pName = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pName);

    pName->GetSymbol(aSymbol, m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(OUString("$B$4:$B$9"), aSymbol);

    // Clear the document and start over.
    m_pDoc->GetRangeName()->clear();
    clearSheet(m_pDoc, 0);

    // Set values to A1:A3.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 2.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 3.0);

    // Name A1:A3 'MyData'.
    bInserted = m_pDoc->InsertNewRangeName("MyData", ScAddress(0,0,0), "$A$1:$A$3");
    CPPUNIT_ASSERT(bInserted);

    // Set formulas to C1:C2 and E1.
    m_pDoc->SetString(ScAddress(2,0,0), "=SUM(MyData)");
    m_pDoc->SetString(ScAddress(2,1,0), "=SUM(MyData)");
    m_pDoc->SetString(ScAddress(4,0,0), "=SUM(MyData)");

    // C1:C2 should be shared.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(2,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    // E1 should not be shared.
    pFC = m_pDoc->GetFormulaCell(ScAddress(4,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT(!pFC->IsShared());

    // Check the results.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(2,1,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(4,0,0)));

    // Insert a new row at row 3.  This should expand MyData to A1:A4.
    rFunc.InsertCells(ScRange(0,2,0,MAXCOL,2,0), &aMark, INS_INSROWS_BEFORE, false, true);

    // Set new value to A3.
    m_pDoc->SetValue(ScAddress(0,2,0), 4.0);

    // Check the results again.
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(2,1,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(4,0,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateNameDeleteRow()
{
    m_pDoc->InsertTab(0, "Test");

    // Insert a new name 'MyRange' to reference B2:B4.
    bool bInserted = m_pDoc->InsertNewRangeName("MyRange", ScAddress(0,0,0), "$B$2:$B$4");
    CPPUNIT_ASSERT(bInserted);

    const ScRangeData* pName = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pName);

    sc::TokenStringContext aCxt(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH);
    const ScTokenArray* pCode = pName->GetCode();
    OUString aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$B$2:$B$4"), aExpr);

    ScDocFunc& rFunc = getDocShell().GetDocFunc();

    // Delete row 3.
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    rFunc.DeleteCells(ScRange(0,2,0,MAXCOL,2,0), &aMark, DEL_CELLSUP, true);

    // The reference in the name should get updated to B2:B3.
    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$B$2:$B$3"), aExpr);

    // Delete row 3 again.
    rFunc.DeleteCells(ScRange(0,2,0,MAXCOL,2,0), &aMark, DEL_CELLSUP, true);
    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$B$2:$B$2"), aExpr);

    // Undo and check.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    pUndoMgr->Undo();

    pName = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pName);
    pCode = pName->GetCode();

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$B$2:$B$3"), aExpr);

    // Undo again and check.
    pUndoMgr->Undo();

    pName = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pName);
    pCode = pName->GetCode();

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$B$2:$B$4"), aExpr);

    // Delete row 2-3.
    rFunc.DeleteCells(ScRange(0,1,0,MAXCOL,2,0), &aMark, DEL_CELLSUP, true);

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$B$2:$B$2"), aExpr);

    // Undo and check.
    pUndoMgr->Undo();

    pName = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pName);
    pCode = pName->GetCode();

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$B$2:$B$4"), aExpr);

    m_pDoc->InsertTab(1, "test2");

    ScMarkData aMark2;
    aMark2.SelectOneTable(1);
    rFunc.DeleteCells(ScRange(0,2,1,MAXCOL,2,1), &aMark2, DEL_CELLSUP, true);

    pName = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pName);
    pCode = pName->GetCode();

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$B$2:$B$4"), aExpr);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateNameCopySheet()
{
    m_pDoc->InsertTab(0, "Test");
    m_pDoc->InsertTab(1, "Test2");

    bool bInserted = m_pDoc->InsertNewRangeName("RED", ScAddress(0,0,0), "$Test.$B$2");
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName("BLUE", ScAddress(0,0,0), "$Test.$B$3");
    CPPUNIT_ASSERT(bInserted);
    m_pDoc->SetValue(1, 1, 0, 1);
    m_pDoc->SetValue(1, 2, 0, 2);

    // insert formula into Test2 that is =RED+BLUE
    m_pDoc->SetString(ScAddress(2,2,1), "=RED+BLUE");

    double nVal = m_pDoc->GetValue(2, 2, 1);
    CPPUNIT_ASSERT_EQUAL(3.0, nVal);
    m_pDoc->CopyTab(1, 0);

    nVal = m_pDoc->GetValue(2, 2, 2);
    CPPUNIT_ASSERT_EQUAL(3.0, nVal);

    nVal = m_pDoc->GetValue(2, 2, 0);
    CPPUNIT_ASSERT_EQUAL(3.0, nVal);

    m_pDoc->SetValue(1, 1, 1, 3);

    nVal = m_pDoc->GetValue(2, 2, 2);
    CPPUNIT_ASSERT_EQUAL(5.0, nVal);

    nVal = m_pDoc->GetValue(2, 2, 0);
    CPPUNIT_ASSERT_EQUAL(5.0, nVal);

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);

    m_pDoc->InsertTab(0, "Test1");
    // Global name referencing sheet Test1.
    bInserted = m_pDoc->InsertNewRangeName("sheetnumber", ScAddress(0,0,0), "$Test1.$A$1");
    CPPUNIT_ASSERT(bInserted);
    m_pDoc->SetString(ScAddress(0,0,0), "=SHEET()");
    m_pDoc->SetString(ScAddress(1,0,0), "=sheetnumber");
    nVal = m_pDoc->GetValue(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet number should be 1", 1.0, nVal);

    // Copy sheet after.
    m_pDoc->CopyTab(0, 1);
    nVal = m_pDoc->GetValue(1,0,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("New sheet number should be 2", 2.0, nVal);
    nVal = m_pDoc->GetValue(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Org sheet number should be 1", 1.0, nVal);
    const ScRangeData* pName = m_pDoc->GetRangeName(1)->findByUpperName("SHEETNUMBER");
    CPPUNIT_ASSERT_MESSAGE("New sheet-local name should exist", pName);

    // Copy sheet before, shifting following now two sheets.
    m_pDoc->CopyTab(0, 0);
    nVal = m_pDoc->GetValue(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("New sheet number should be 1", 1.0, nVal);
    pName = m_pDoc->GetRangeName(0)->findByUpperName("SHEETNUMBER");
    CPPUNIT_ASSERT_MESSAGE("New sheet-local name should exist", pName);
    nVal = m_pDoc->GetValue(1,0,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Org sheet number should be 2", 2.0, nVal);
    pName = m_pDoc->GetRangeName(1)->findByUpperName("SHEETNUMBER");
    CPPUNIT_ASSERT_MESSAGE("Org sheet-local name should not exist", !pName);
    nVal = m_pDoc->GetValue(1,0,2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Old sheet number should be 3", 3.0, nVal);
    pName = m_pDoc->GetRangeName(2)->findByUpperName("SHEETNUMBER");
    CPPUNIT_ASSERT_MESSAGE("Old sheet-local name should exist", pName);

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);

    m_pDoc->InsertTab(0, "Test2");
    // Local name referencing sheet Test2.
    bInserted = m_pDoc->GetRangeName(0)->insert( new ScRangeData( m_pDoc, "localname", "$Test2.$A$1"));
    CPPUNIT_ASSERT(bInserted);
    m_pDoc->SetString(ScAddress(0,0,0), "=SHEET()");
    m_pDoc->SetString(ScAddress(1,0,0), "=localname");
    nVal = m_pDoc->GetValue(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Localname sheet number should be 1", 1.0, nVal);

    // Insert sheet before and shift sheet with local name.
    m_pDoc->InsertTab(0, "Test1");
    pName = m_pDoc->GetRangeName(1)->findByUpperName("LOCALNAME");
    CPPUNIT_ASSERT_MESSAGE("Org sheet-local name should exist", pName);
    nVal = m_pDoc->GetValue(1,0,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Localname sheet number should be 2", 2.0, nVal);

    // Copy sheet before, shifting following now two sheets.
    m_pDoc->CopyTab(1, 0);
    pName = m_pDoc->GetRangeName(0)->findByUpperName("LOCALNAME");
    CPPUNIT_ASSERT_MESSAGE("New sheet-local name should exist", pName);
    nVal = m_pDoc->GetValue(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("New sheet number should be 1", 1.0, nVal);
    pName = m_pDoc->GetRangeName(1)->findByUpperName("LOCALNAME");
    CPPUNIT_ASSERT_MESSAGE("Old sheet-local name should not exist", !pName);
    pName = m_pDoc->GetRangeName(2)->findByUpperName("LOCALNAME");
    CPPUNIT_ASSERT_MESSAGE("Org sheet-local name should exist", pName);
    nVal = m_pDoc->GetValue(1,0,2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("New sheet number should be 3", 3.0, nVal);

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
    m_pDoc->SetRangeName(nullptr);

    // Test nested names during copying sheet.

    m_pDoc->InsertTab(0, "Test2");
    ScAddress aPos(0,0,0);
    bInserted = m_pDoc->InsertNewRangeName( "global", aPos, "$Test2.$A$1");
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( aPos.Tab(), "local", aPos, "$Test2.$A$2");
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( "global_global", aPos, "global*100");
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( "global_local", aPos, "local*1000");
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( "global_unused", aPos, "$Test2.$A$1");
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( "global_unused_noref", aPos, "42");
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( aPos.Tab(), "local_global", aPos, "global*10000");
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( aPos.Tab(), "local_local", aPos, "local*100000");
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( aPos.Tab(), "local_unused", aPos, "$Test2.$A$2");
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( aPos.Tab(), "local_unused_noref", aPos, "23");
    CPPUNIT_ASSERT(bInserted);

    m_pDoc->SetString(aPos, "=SHEET()");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A1*10+SHEET()");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=global_global");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=global_local");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=local_global");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=local_local");

    testFormulaRefUpdateNameCopySheetCheckTab( 0, false);

    // Copy sheet after.
    m_pDoc->CopyTab(0, 1);
    testFormulaRefUpdateNameCopySheetCheckTab( 0, false);
    testFormulaRefUpdateNameCopySheetCheckTab( 1, true);

    // Copy sheet before, shifting following now two sheets.
    m_pDoc->CopyTab(1, 0);
    testFormulaRefUpdateNameCopySheetCheckTab( 0, true);
    testFormulaRefUpdateNameCopySheetCheckTab( 1, false);
    testFormulaRefUpdateNameCopySheetCheckTab( 2, true);

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateNameCopySheetCheckTab( SCTAB nTab, bool bCheckNames )
{
    if (bCheckNames)
    {
        const ScRangeData* pName;
        pName = m_pDoc->GetRangeName(nTab)->findByUpperName("GLOBAL");
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name GLOBAL should exist", pName);
        pName = m_pDoc->GetRangeName(nTab)->findByUpperName("LOCAL");
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name LOCAL should exist", pName);
        pName = m_pDoc->GetRangeName(nTab)->findByUpperName("GLOBAL_GLOBAL");
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name GLOBAL_GLOBAL should exist", pName);
        pName = m_pDoc->GetRangeName(nTab)->findByUpperName("GLOBAL_LOCAL");
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name GLOBAL_LOCAL should exist", pName);
        pName = m_pDoc->GetRangeName(nTab)->findByUpperName("GLOBAL_UNUSED");
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name GLOBAL_UNUSED should exist", pName);
        pName = m_pDoc->GetRangeName(nTab)->findByUpperName("GLOBAL_UNUSED_NOREF");
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name GLOBAL_UNUSED_NOREF should not exist", !pName);
        pName = m_pDoc->GetRangeName(nTab)->findByUpperName("LOCAL_GLOBAL");
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name LOCAL_GLOBAL should exist", pName);
        pName = m_pDoc->GetRangeName(nTab)->findByUpperName("LOCAL_LOCAL");
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name LOCAL_LOCAL should exist", pName);
        pName = m_pDoc->GetRangeName(nTab)->findByUpperName("LOCAL_UNUSED");
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name LOCAL_UNUSED should exist", pName);
        pName = m_pDoc->GetRangeName(nTab)->findByUpperName("LOCAL_UNUSED_NOREF");
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name LOCAL_UNUSED_NOREF should exist", pName);
    }

    ScAddress aPos(0,0,0);
    aPos.SetRow(0);
    aPos.SetTab(nTab);
    int nSheet = nTab + 1;
    CPPUNIT_ASSERT_EQUAL( 1.0 * nSheet, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL( 11.0 * nSheet, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL( 100.0 * nSheet, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL( 11000.0 * nSheet, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL( 10000.0 * nSheet, m_pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL( 1100000.0 * nSheet, m_pDoc->GetValue(aPos));
}

void Test::testFormulaRefUpdateNameDelete()
{
    m_pDoc->InsertTab(0, "Test");

    // Insert a new name 'MyRange' to reference B1
    bool bInserted = m_pDoc->InsertNewRangeName("MyRange", ScAddress(0,0,0), "$Test.$B$1");
    CPPUNIT_ASSERT(bInserted);

    const ScRangeData* pName = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pName);

    m_pDoc->DeleteCol(1, 0, 3, 0, 0, 1);
    const ScTokenArray* pCode = pName->GetCode();
    sc::TokenStringContext aCxt(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH);
    OUString aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$1"), aExpr);

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateValidity()
{
    struct {

        bool checkList( std::vector<ScTypedStrData>& rList )
        {
            double aExpected[] = { 1.0, 2.0, 3.0 }; // must be sorted.
            size_t nCheckSize = SAL_N_ELEMENTS(aExpected);

            if (rList.size() != nCheckSize)
            {
                cerr << "List size is not what is expected." << endl;
                return false;
            }

            std::sort(rList.begin(), rList.end(), ScTypedStrData::LessCaseSensitive());

            for (size_t i = 0; i < nCheckSize; ++i)
            {
                if (aExpected[i] != rList[i].GetValue())
                {
                    cerr << "Incorrect value at position " << i
                        << ": expected=" << aExpected[i] << ", actual=" << rList[i].GetValue() << endl;
                    return false;
                }
            }

            return true;
        }

    } aCheck;

    setExpandRefs(false);
    setCalcAsShown(m_pDoc, true);

    m_pDoc->InsertTab(0, "Formula");

    // Set values in C2:C4.
    m_pDoc->SetValue(ScAddress(2,1,0), 1.0);
    m_pDoc->SetValue(ScAddress(2,2,0), 2.0);
    m_pDoc->SetValue(ScAddress(2,3,0), 3.0);

    // Set validity in A2.
    ScValidationData aData(
        SC_VALID_LIST, SC_COND_EQUAL, "C2:C4", "", m_pDoc, ScAddress(0,1,0), "", "",
        m_pDoc->GetGrammar(), m_pDoc->GetGrammar());

    sal_uLong nIndex = m_pDoc->AddValidationEntry(aData);
    SfxUInt32Item aItem(ATTR_VALIDDATA, nIndex);

    ScPatternAttr aNewAttrs(
        new SfxItemSet(*m_pDoc->GetPool(), ATTR_PATTERN_START, ATTR_PATTERN_END));
    aNewAttrs.GetItemSet().Put(aItem);

    m_pDoc->ApplyPattern(0, 1, 0, aNewAttrs);

    const ScValidationData* pData = m_pDoc->GetValidationEntry(nIndex);
    CPPUNIT_ASSERT(pData);

    // Make sure the list is correct.
    std::vector<ScTypedStrData> aList;
    pData->FillSelectionList(aList, ScAddress(0,1,0));
    bool bGood = aCheck.checkList(aList);
    CPPUNIT_ASSERT_MESSAGE("Initial list is incorrect.", bGood);

    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    ScMarkData aMark;
    aMark.SelectOneTable(0);

    // Insert a new column at Column B, to move the list from C2:C4 to D2:D4.
    bool bInserted = rFunc.InsertCells(ScRange(1,0,0,1,MAXROW,0), &aMark, INS_INSCOLS_BEFORE, true, true);
    CPPUNIT_ASSERT_MESSAGE("Column insertion failed.", bInserted);
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(3,1,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(3,2,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(3,3,0)));

    // Check the list values again.
    aList.clear();
    pData->FillSelectionList(aList, ScAddress(0,1,0));
    bGood = aCheck.checkList(aList);
    CPPUNIT_ASSERT_MESSAGE("List content is incorrect after column insertion.", bGood);

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    // Undo and check the list content again.  The list moves back to C2:C4 after the undo.
    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(2,1,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,2,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(2,3,0)));

    aList.clear();
    pData->FillSelectionList(aList, ScAddress(0,1,0));
    bGood = aCheck.checkList(aList);
    CPPUNIT_ASSERT_MESSAGE("List content is incorrect after undo of column insertion.", bGood);

    // Move C2:C4 to E5:E7.
    bool bMoved = rFunc.MoveBlock(ScRange(2,1,0,2,3,0), ScAddress(4,4,0), false, true, false, true);
    CPPUNIT_ASSERT(bMoved);
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(4,4,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(4,5,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(4,6,0)));

    // Check the list again after the move.
    aList.clear();
    pData->FillSelectionList(aList, ScAddress(0,1,0));
    bGood = aCheck.checkList(aList);
    CPPUNIT_ASSERT_MESSAGE("List content is incorrect after moving C2:C4 to E5:E7.", bGood);

    // Undo the move and check.  The list should be back to C2:C4.
    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(2,1,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,2,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(2,3,0)));

    aList.clear();
    pData->FillSelectionList(aList, ScAddress(0,1,0));
    bGood = aCheck.checkList(aList);
    CPPUNIT_ASSERT_MESSAGE("List content is incorrect after undo of the move.", bGood);

    m_pDoc->DeleteTab(0);
}

void Test::testMultipleOperations()
{
    m_pDoc->InsertTab(0, "MultiOp");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    // Insert the reference formula at top row.
    m_pDoc->SetValue(ScAddress(0,0,0), 1);
    m_pDoc->SetString(ScAddress(1,0,0), "=A1*10");
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(1,0,0)));

    // Insert variable inputs in A3:A5.
    m_pDoc->SetValue(ScAddress(0,2,0), 2);
    m_pDoc->SetValue(ScAddress(0,3,0), 3);
    m_pDoc->SetValue(ScAddress(0,4,0), 4);

    // Set multiple operations range.
    ScTabOpParam aParam;
    aParam.aRefFormulaCell = ScRefAddress(1,0,0,false,false,false);
    aParam.aRefFormulaEnd = aParam.aRefFormulaCell;
    aParam.aRefColCell = ScRefAddress(0,0,0,false,false,false);
    ScMarkData aMark;
    aMark.SetMarkArea(ScRange(0,2,0,1,4,0)); // Select A3:B5.
    m_pDoc->InsertTableOp(aParam, 0, 2, 1, 4, aMark);
    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(1,2,0));
    CPPUNIT_ASSERT_EQUAL(30.0, m_pDoc->GetValue(1,3,0));
    CPPUNIT_ASSERT_EQUAL(40.0, m_pDoc->GetValue(1,4,0));

    // Clear A3:B5.
    clearRange(m_pDoc, ScRange(0,2,0,1,4,0));

    // This time, use indirect reference formula cell.
    m_pDoc->SetString(ScAddress(2,0,0), "=B1"); // C1 simply references B1.
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(2,0,0)));

    // Insert variable inputs in A3:A5.
    m_pDoc->SetValue(ScAddress(0,2,0), 3);
    m_pDoc->SetValue(ScAddress(0,3,0), 4);
    m_pDoc->SetValue(ScAddress(0,4,0), 5);

    // Set multiple operations range again, but this time, we'll use C1 as the reference formula.
    aParam.aRefFormulaCell.Set(2,0,0,false,false,false);
    aParam.aRefFormulaEnd = aParam.aRefFormulaCell;
    m_pDoc->InsertTableOp(aParam, 0, 2, 1, 4, aMark);
    CPPUNIT_ASSERT_EQUAL(30.0, m_pDoc->GetValue(1,2,0));
    CPPUNIT_ASSERT_EQUAL(40.0, m_pDoc->GetValue(1,3,0));
    CPPUNIT_ASSERT_EQUAL(50.0, m_pDoc->GetValue(1,4,0));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncCOLUMN()
{
    m_pDoc->InsertTab(0, "Formula");
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->SetString(ScAddress(5,10,0), "=COLUMN()");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(5,10,0)));

    m_pDoc->SetString(ScAddress(0,1,0), "=F11");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Move the formula cell with COLUMN() function to change its value.
    m_pDoc->InsertCol(ScRange(5,0,0,5,MAXROW,0));
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(ScAddress(6,10,0)));

    // The cell that references the moved cell should update its value as well.
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Move the column in the other direction.
    m_pDoc->DeleteCol(ScRange(5,0,0,5,MAXROW,0));

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(5,10,0)));

    // The cell that references the moved cell should update its value as well.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncCOUNT()
{
    m_pDoc->InsertTab(0, "Formula");
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->SetValue(ScAddress(0,0,0), 2); // A1
    m_pDoc->SetValue(ScAddress(0,1,0), 4); // A2
    m_pDoc->SetValue(ScAddress(0,2,0), 6); // A3

    ScAddress aPos(1,0,0);
    m_pDoc->SetString(aPos, "=COUNT(A1:A3)");
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, "=COUNT(A1:A3;2)");
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, "=COUNT(A1:A3;2;4)");
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, "=COUNT(A1:A3;2;4;6)");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(aPos));

    // Matrix in C1.
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(2, 0, 2, 0, aMark, "=COUNT(SEARCH(\"a\";{\"a\";\"b\";\"a\"}))");
    // Check that the #VALUE! error of "a" not found in "b" is not counted.
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,0,0)));

    // Matrix in C3.
    m_pDoc->InsertMatrixFormula(2, 2, 2, 2, aMark, "=COUNTA(SEARCH(\"a\";{\"a\";\"b\";\"a\"}))");
    // Check that the #VALUE! error of "a" not found in "b" is counted.
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(2,2,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncCOUNTBLANK()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Formula");

    const char* aData[][4] = {
        { "1", nullptr, "=B1", "=\"\"" },
        { "2", nullptr, "=B2", "=\"\"" },
        { "A", nullptr, "=B3", "=\"\"" },
        { "B", nullptr, "=B4", "=D3" },
        {   nullptr, nullptr, "=B5", "=D4" },
        { "=COUNTBLANK(A1:A5)", "=COUNTBLANK(B1:B5)", "=COUNTBLANK(C1:C5)", "=COUNTBLANK(D1:D5)" }
    };

    ScAddress aPos(0,0,0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(1,5,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(2,5,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(3,5,0)));

    // Test single cell reference cases.

    clearSheet(m_pDoc, 0);

    const char* aData2[][2] = {
        { "1",     "=COUNTBLANK(A1)" },
        { "A",     "=COUNTBLANK(A2)" },
        {   nullptr,     "=COUNTBLANK(A3)" },
        { "=\"\"", "=COUNTBLANK(A4)" },
        { "=A4"  , "=COUNTBLANK(A5)" },
    };

    aRange = insertRangeData(m_pDoc, aPos, aData2, SAL_N_ELEMENTS(aData2));
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncROW()
{
    m_pDoc->InsertTab(0, "Formula");
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->SetString(ScAddress(5,10,0), "=ROW()");
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(5,10,0)));

    m_pDoc->SetString(ScAddress(0,1,0), "=F11");
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Insert 2 new rows at row 4.
    m_pDoc->InsertRow(ScRange(0,3,0,MAXCOL,4,0));
    CPPUNIT_ASSERT_EQUAL(13.0, m_pDoc->GetValue(ScAddress(5,12,0)));

    // The cell that references the moved cell should update its value as well.
    CPPUNIT_ASSERT_EQUAL(13.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Delete 2 rows to move it back.
    m_pDoc->DeleteRow(ScRange(0,3,0,MAXCOL,4,0));

    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(5,10,0)));

    // The cell that references the moved cell should update its value as well.
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Clear sheet and start over.
    clearSheet(m_pDoc, 0);

    m_pDoc->SetString(ScAddress(0,1,0), "=ROW(A5)");
    m_pDoc->SetString(ScAddress(1,1,0), "=ROW(B5)");
    m_pDoc->SetString(ScAddress(1,2,0), "=ROW(B6)");
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    // B2:B3 should be shared.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,1,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    // Insert a new row at row 4.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    rFunc.InsertCells(ScRange(0,3,0,MAXCOL,3,0), &aMark, INS_INSROWS_BEFORE, false, true);
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "ROW(A6)", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,1,0), "ROW(B6)", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,2,0), "ROW(B7)", "Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncSUM()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    // Single argument case.
    m_pDoc->SetValue(ScAddress(0,0,0), 1);
    m_pDoc->SetValue(ScAddress(0,1,0), 1);
    m_pDoc->SetString(ScAddress(0,2,0), "=SUM(A1:A2)");
    m_pDoc->CalcAll();
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Multiple argument case.
    m_pDoc->SetValue(ScAddress(0,0,0), 1);
    m_pDoc->SetValue(ScAddress(0,1,0), 22);
    m_pDoc->SetValue(ScAddress(0,2,0), 4);
    m_pDoc->SetValue(ScAddress(0,3,0), 5);
    m_pDoc->SetValue(ScAddress(0,4,0), 6);

    m_pDoc->SetValue(ScAddress(1,0,0), 3);
    m_pDoc->SetValue(ScAddress(1,1,0), 4);
    m_pDoc->SetValue(ScAddress(1,2,0), 5);
    m_pDoc->SetValue(ScAddress(1,3,0), 6);
    m_pDoc->SetValue(ScAddress(1,4,0), 7);

    m_pDoc->SetString(ScAddress(3,0,0), "=SUM(A1:A2;B1:B2)");
    m_pDoc->SetString(ScAddress(3,1,0), "=SUM(A2:A3;B2:B3)");
    m_pDoc->SetString(ScAddress(3,2,0), "=SUM(A3:A4;B3:B4)");
    CPPUNIT_ASSERT_EQUAL(30.0, m_pDoc->GetValue(ScAddress(3,0,0)));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(ScAddress(3,1,0)));
    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(ScAddress(3,2,0)));

    // Clear and start over.
    clearRange(m_pDoc, ScRange(0,0,0,3,MAXROW,0));

    // SUM needs to take the first error in case the range contains an error.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 10.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 100.0);
    m_pDoc->SetString(ScAddress(0,3,0), "=SUM(A1:A3)");
    CPPUNIT_ASSERT_EQUAL(111.0, m_pDoc->GetValue(ScAddress(0,3,0)));

    // Set #DIV/0! error to A3. A4 should also inherit this error.
    m_pDoc->SetString(ScAddress(0,2,0), "=1/0");
    sal_uInt16 nErr = m_pDoc->GetErrCode(ScAddress(0,2,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should have a division by zero error.",
                           errDivisionByZero, nErr);
    nErr = m_pDoc->GetErrCode(ScAddress(0,3,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUM should have also inherited a div-by-zero error.",
                           errDivisionByZero, nErr);

    // Set #NA! to A2. A4 should now inherit this error.
    m_pDoc->SetString(ScAddress(0,1,0), "=NA()");
    nErr = m_pDoc->GetErrCode(ScAddress(0,1,0));
    CPPUNIT_ASSERT_MESSAGE("A2 should be an error.", nErr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A4 should have inherited the same error as A2.",
                           nErr, m_pDoc->GetErrCode(ScAddress(0,3,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncPRODUCT()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto recalc.

    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab(0, aTabName));

    ScAddress aPos(3, 0, 0);
    m_pDoc->SetValue(0, 0, 0, 3.0); // A1
    m_pDoc->SetString(aPos, "=PRODUCT(A1)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", 3.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, -3.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", -3.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=PRODUCT(B1)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 0, 0, 10.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", 10.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetString(aPos, "=PRODUCT(A1:C3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", -30.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 1, 0, -1.0); // B2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", 30.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 0, 0, 4.0); // C1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", 120.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, -2.0); // A2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", -240.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 1, 0, 8.0); // C2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", -1920.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 2, 0, 0.2); // A3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of PRODUCT failed", -384.0, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 2, 0, -0.25); // B3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of PRODUCT failed", 96.0, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 2, 0, -0.125); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of PRODUCT failed", -12.0, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 2, 0, 0.0); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of PRODUCT failed", 0.0, m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->SetString(aPos, "=PRODUCT({2;3;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", 24.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=PRODUCT({2;-2;2})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", -8.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=PRODUCT({8;0.125;-1})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", -1.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetString(aPos, "=PRODUCT({2;3};{4;5})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", 120.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=PRODUCT({10;-8};{3;-1};{15;30};{7})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", 756000.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=PRODUCT({10;-0.1;8};{0.125;4;0.25;2};{0.5};{1};{-1})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", 1.0, m_pDoc->GetValue(aPos));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncSUMPRODUCT()
{
    m_pDoc->InsertTab(0, "Test");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto recalc.

    ScAddress aPos(0,0,0);
    m_pDoc->SetString(aPos, "=SUMPRODUCT(B1:B3;C1:C3)");
    CPPUNIT_ASSERT_EQUAL(0.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(2,0,0),  1.0); // C1
    CPPUNIT_ASSERT_EQUAL(0.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(1,0,0),  1.0); // B1
    CPPUNIT_ASSERT_EQUAL(1.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(1,1,0),  2.0); // B2
    CPPUNIT_ASSERT_EQUAL(1.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(2,1,0),  3.0); // C2
    CPPUNIT_ASSERT_EQUAL(7.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(2,2,0), -2.0); // C3
    CPPUNIT_ASSERT_EQUAL(7.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(1,2,0),  5.0); // B3
    CPPUNIT_ASSERT_EQUAL(-3.0, m_pDoc->GetValue(aPos));

    // Force an error in C2 and test ForcedArray matrix error propagation.
    m_pDoc->SetString( 2, 1, 0, "=1/0");
    sal_uInt16 nError = m_pDoc->GetErrCode(aPos);
    CPPUNIT_ASSERT_MESSAGE("Formula result should be a propagated error", nError);

    // Test ForceArray propagation of SUMPRODUCT parameters to ABS and + operator.
    // => ABS({-3,4})*({-3,4}+{-3,4}) => {3,4}*{-6,8} => {-18,32} => 14
    m_pDoc->SetValue(ScAddress(4,0,0), -3.0); // E1
    m_pDoc->SetValue(ScAddress(4,1,0),  4.0); // E2
    // Non-intersecting formula in F3.
    m_pDoc->SetString(ScAddress(5,2,0), "=SUMPRODUCT(ABS(E1:E2);E1:E2+E1:E2)");
    CPPUNIT_ASSERT_EQUAL(14.0, m_pDoc->GetValue(ScAddress(5,2,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncSUMXMY2()
{
    m_pDoc->InsertTab(0, "Test SumXMY2");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto recalc.

    ScAddress aPos(0,0,0);
    m_pDoc->SetString(aPos, "=SUMXMY2(B1:B3;C1:C3)");
    CPPUNIT_ASSERT_EQUAL(0.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(1,0,0),  1.0); // B1
    CPPUNIT_ASSERT_EQUAL(1.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(1,1,0),  2.0); // B2
    CPPUNIT_ASSERT_EQUAL(5.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(1,2,0),  3.0); // B3
    CPPUNIT_ASSERT_EQUAL(14.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(2,0,0), -1.0); // C1
    CPPUNIT_ASSERT_EQUAL(17.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(2,1,0),  3.0); // C2
    CPPUNIT_ASSERT_EQUAL(14.0,  m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(ScAddress(2,2,0),  1.0); // C3
    CPPUNIT_ASSERT_EQUAL(9.0,  m_pDoc->GetValue(aPos));

    double result = 0.0;
    m_pDoc->SetString(0, 4, 0, "=SUMXMY2({2;3;4};{4;3;2})");
    m_pDoc->GetValue(0, 4, 0, result);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of SUMXMY2 with inline arrays failed", 8.0, result);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncMIN()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto recalc.
    m_pDoc->InsertTab(0, "Formula");

    // A1:A2
    m_pDoc->SetString(ScAddress(0,0,0), "a");
    m_pDoc->SetString(ScAddress(0,1,0), "b");

    // B1:B2
    m_pDoc->SetValue(ScAddress(1,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,1,0), 2.0);

    // Matrix in C1:C2.
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(2, 0, 2, 1, aMark, "=MIN(IF(A1:A2=\"c\";B1:B2))");

    // Formula cell in C1:C2 should be a 1x2 matrix array.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(2,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula should be an array.", MM_FORMULA, static_cast<ScMatrixMode>(pFC->GetMatrixFlag()));

    SCCOL nCols;
    SCROW nRows;
    pFC->GetMatColsRows(nCols, nRows);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCols);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), nRows);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula in C1 is invalid.", static_cast<sal_uInt16>(0), m_pDoc->GetErrCode(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula in C2 is invalid.", static_cast<sal_uInt16>(0), m_pDoc->GetErrCode(ScAddress(2,1,0)));

    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(2,1,0)));

    // Inline array input (A4).
    m_pDoc->SetString(ScAddress(0,3,0), "=MIN({-2;4;3})");
    CPPUNIT_ASSERT_EQUAL(-2.0, m_pDoc->GetValue(ScAddress(0,3,0)));

    // Add more values to B3:B4.
    m_pDoc->SetValue(ScAddress(1,2,0),  20.0);
    m_pDoc->SetValue(ScAddress(1,3,0), -20.0);

    // Get the MIN of B1:B4.
    m_pDoc->SetString(ScAddress(2,4,0), "=MIN(B1:B4)");
    CPPUNIT_ASSERT_EQUAL(-20.0, m_pDoc->GetValue(ScAddress(2,4,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncN()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    double result;

    // Clear the area first.
    clearRange(m_pDoc, ScRange(0, 0, 0, 1, 20, 0));

    // Put values to reference.
    double val = 0;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->SetString(0, 2, 0, "Text");
    val = 1;
    m_pDoc->SetValue(0, 3, 0, val);
    val = -1;
    m_pDoc->SetValue(0, 4, 0, val);
    val = 12.3;
    m_pDoc->SetValue(0, 5, 0, val);
    m_pDoc->SetString(0, 6, 0, "'12.3");

    // Cell references
    m_pDoc->SetString(1, 0, 0, "=N(A1)");
    m_pDoc->SetString(1, 1, 0, "=N(A2)");
    m_pDoc->SetString(1, 2, 0, "=N(A3)");
    m_pDoc->SetString(1, 3, 0, "=N(A4)");
    m_pDoc->SetString(1, 4, 0, "=N(A5)");
    m_pDoc->SetString(1, 5, 0, "=N(A6)");
    m_pDoc->SetString(1, 6, 0, "=N(A9)");

    // In-line values
    m_pDoc->SetString(1, 7, 0, "=N(0)");
    m_pDoc->SetString(1, 8, 0, "=N(1)");
    m_pDoc->SetString(1, 9, 0, "=N(-1)");
    m_pDoc->SetString(1, 10, 0, "=N(123)");
    m_pDoc->SetString(1, 11, 0, "=N(\"\")");
    m_pDoc->SetString(1, 12, 0, "=N(\"12\")");
    m_pDoc->SetString(1, 13, 0, "=N(\"foo\")");

    // Range references
    m_pDoc->SetString(2, 2, 0, "=N(A1:A8)");
    m_pDoc->SetString(2, 3, 0, "=N(A1:A8)");
    m_pDoc->SetString(2, 4, 0, "=N(A1:A8)");
    m_pDoc->SetString(2, 5, 0, "=N(A1:A8)");

    // Calculate and check the results.
    m_pDoc->CalcAll();
    double checks1[] = {
        0, 0,  0,    1, -1, 12.3, 0, // cell reference
        0, 1, -1, 123,  0,    0, 0   // in-line values
    };
    for (size_t i = 0; i < SAL_N_ELEMENTS(checks1); ++i)
    {
        m_pDoc->GetValue(1, i, 0, result);
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
        m_pDoc->GetValue(1, i+2, 0, result);
        bool bGood = result == checks2[i];
        if (!bGood)
        {
            cerr << "row " << (i+2+1) << ": expected=" << checks2[i] << " actual=" << result << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for N", false);
        }
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFuncCOUNTIF()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    // COUNTIF (test case adopted from OOo i#36381)

    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    // Empty A1:A39 first.
    clearRange(m_pDoc, ScRange(0, 0, 0, 0, 40, 0));

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
        m_pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i]));

    printRange(m_pDoc, ScRange(0, 0, 0, 0, 8, 0), "data range for COUNTIF");

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
        m_pDoc->SetString(0, nRow, 0, OUString::createFromAscii(aChecks[i].pFormula));
    }

    for (SCROW i = 0; i < nRows; ++i)
    {
        double result;
        SCROW nRow = 20 + i;
        m_pDoc->GetValue(0, nRow, 0, result);
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
    clearRange(m_pDoc, ScRange(0, 0, 0, 0, 1, 0));

    m_pDoc->SetString(0, 0, 0, "=\"\"");
    m_pDoc->SetString(0, 1, 0, "=COUNTIF(A1;1)");

    double result = m_pDoc->GetValue(0, 1, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("We shouldn't count empty string as valid number.", 0.0, result);

    // Another test case adopted from fdo#77039.
    clearSheet(m_pDoc, 0);

    // Set formula cells with blank results in A1:A4.
    for (SCROW i = 0; i <=3; ++i)
        m_pDoc->SetString(ScAddress(0,i,0), "=\"\"");

    // Insert formula into A5 to count all cells with empty strings.
    m_pDoc->SetString(ScAddress(0,4,0), "=COUNTIF(A1:A4;\"\"");

    // We should correctly count with empty string key.
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(0,4,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncIF()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Formula");

    m_pDoc->SetString(ScAddress(0,0,0), "=IF(B1=2;\"two\";\"not two\")");
    CPPUNIT_ASSERT_EQUAL(OUString("not two"), m_pDoc->GetString(ScAddress(0,0,0)));
    m_pDoc->SetValue(ScAddress(1,0,0), 2.0);
    CPPUNIT_ASSERT_EQUAL(OUString("two"), m_pDoc->GetString(ScAddress(0,0,0)));
    m_pDoc->SetValue(ScAddress(1,0,0), 3.0);
    CPPUNIT_ASSERT_EQUAL(OUString("not two"), m_pDoc->GetString(ScAddress(0,0,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncCHOOSE()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Formula");

    m_pDoc->SetString(ScAddress(0,0,0), "=CHOOSE(B1;\"one\";\"two\";\"three\")");
    sal_uInt16 nError = m_pDoc->GetErrCode(ScAddress(0,0,0));
    CPPUNIT_ASSERT_MESSAGE("Formula result should be an error since B1 is still empty.", nError);
    m_pDoc->SetValue(ScAddress(1,0,0), 1.0);
    CPPUNIT_ASSERT_EQUAL(OUString("one"), m_pDoc->GetString(ScAddress(0,0,0)));
    m_pDoc->SetValue(ScAddress(1,0,0), 2.0);
    CPPUNIT_ASSERT_EQUAL(OUString("two"), m_pDoc->GetString(ScAddress(0,0,0)));
    m_pDoc->SetValue(ScAddress(1,0,0), 3.0);
    CPPUNIT_ASSERT_EQUAL(OUString("three"), m_pDoc->GetString(ScAddress(0,0,0)));
    m_pDoc->SetValue(ScAddress(1,0,0), 4.0);
    nError = m_pDoc->GetErrCode(ScAddress(0,0,0));
    CPPUNIT_ASSERT_MESSAGE("Formula result should be an error due to out-of-bound input..", nError);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncIFERROR()
{
    // IFERROR/IFNA (fdo#56124)

    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    // Empty A1:A39 first.
    clearRange(m_pDoc, ScRange(0, 0, 0, 0, 40, 0));

    // Raw data (rows 1 through 12)
    const char* aData[] = {
        "1",
        "e",
        "=SQRT(4)",
        "=SQRT(-2)",
        "=A4",
        "=1/0",
        "=NA()",
        "bar",
        "4",
        "gee",
        "=1/0",
        "23"
    };

    SCROW nRows = SAL_N_ELEMENTS(aData);
    for (SCROW i = 0; i < nRows; ++i)
        m_pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i]));

    printRange(m_pDoc, ScRange(0, 0, 0, 0, nRows-1, 0), "data range for IFERROR/IFNA");

    // formulas and results
    struct {
        const char* pFormula; const char* pResult;
    } aChecks[] = {
        { "=IFERROR(A1;9)",                         "1" },
        { "=IFERROR(A2;9)",                         "e" },
        { "=IFERROR(A3;9)",                         "2" },
        { "=IFERROR(A4;-7)",                       "-7" },
        { "=IFERROR(A5;-7)",                       "-7" },
        { "=IFERROR(A6;-7)",                       "-7" },
        { "=IFERROR(A7;-7)",                       "-7" },
        { "=IFNA(A6;9)",                      "#DIV/0!" },
        { "=IFNA(A7;-7)",                          "-7" },
        { "=IFNA(VLOOKUP(\"4\";A8:A10;1;0);-2)",    "4" },
        { "=IFNA(VLOOKUP(\"fop\";A8:A10;1;0);-2)", "-2" },
        { "{=IFERROR(3*A11:A12;1998)}[0]",       "1998" },  // um.. this is not the correct way to insert a
        { "{=IFERROR(3*A11:A12;1998)}[1]",         "69" }   // matrix formula, just a place holder, see below
    };

    nRows = SAL_N_ELEMENTS(aChecks);
    for (SCROW i = 0; i < nRows-2; ++i)
    {
        SCROW nRow = 20 + i;
        m_pDoc->SetString(0, nRow, 0, OUString::createFromAscii(aChecks[i].pFormula));
    }

    // Create a matrix range in last two rows of the range above, actual data
    // of the placeholders.
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 20 + nRows-2, 0, 20 + nRows-1, aMark, "=IFERROR(3*A11:A12;1998)");

    m_pDoc->CalcAll();

    for (SCROW i = 0; i < nRows; ++i)
    {
        SCROW nRow = 20 + i;
        OUString aResult = m_pDoc->GetString(0, nRow, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            aChecks[i].pFormula, OUString::createFromAscii( aChecks[i].pResult), aResult);
    }

    const SCCOL nCols = 3;
    const char* aData2[][nCols] = {
        { "1", "2",    "3" },
        { "4", "=1/0", "6" },
        { "7", "8",    "9" }
    };
    const char* aCheck2[][nCols] = {
        { "1", "2",    "3" },
        { "4", "Error","6" },
        { "7", "8",    "9" }
    };

    // Data in C1:E3
    ScAddress aPos(2,0,0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData2, SAL_N_ELEMENTS(aData2));
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    // Array formula in F4:H6
    const SCROW nElems2 = SAL_N_ELEMENTS(aCheck2);
    const SCCOL nStartCol = aPos.Col() + nCols;
    const SCROW nStartRow = aPos.Row() + nElems2;
    m_pDoc->InsertMatrixFormula( nStartCol, nStartRow, nStartCol+nCols-1, nStartRow+nElems2-1, aMark,
            "=IFERROR(C1:E3;\"Error\")");

    m_pDoc->CalcAll();

    for (SCCOL nCol = nStartCol; nCol < nStartCol + nCols; ++nCol)
    {
        for (SCROW nRow = nStartRow; nRow < nStartRow + nElems2; ++nRow)
        {
            OUString aResult = m_pDoc->GetString( nCol, nRow, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "IFERROR array result",
                    OUString::createFromAscii( aCheck2[nRow-nStartRow][nCol-nStartCol]), aResult);
        }
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFuncSHEET()
{
    OUString aTabName1("test1");
    OUString aTabName2("test2");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (SC_TAB_APPEND, aTabName1));

    m_pDoc->SetString(0, 0, 0, "=SHEETS()");
    m_pDoc->CalcFormulaTree(false, false);
    double original;
    m_pDoc->GetValue(0, 0, 0, original);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("result of SHEETS() should equal the number of sheets, but doesn't.",
                           static_cast<SCTAB>(original), m_pDoc->GetTableCount());

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (SC_TAB_APPEND, aTabName2));

    double modified;
    m_pDoc->GetValue(0, 0, 0, modified);
    ASSERT_DOUBLES_EQUAL_MESSAGE("result of SHEETS() did not get updated after sheet insertion.",
                           1.0, modified - original);

    SCTAB nTabCount = m_pDoc->GetTableCount();
    m_pDoc->DeleteTab(--nTabCount);

    m_pDoc->GetValue(0, 0, 0, modified);
    ASSERT_DOUBLES_EQUAL_MESSAGE("result of SHEETS() did not get updated after sheet removal.",
                           0.0, modified - original);

    m_pDoc->DeleteTab(--nTabCount);
}

void Test::testFuncNOW()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    double val = 1;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->SetString(0, 1, 0, "=IF(A1>0;NOW();0");
    double now1;
    m_pDoc->GetValue(0, 1, 0, now1);
    CPPUNIT_ASSERT_MESSAGE("Value of NOW() should be positive.", now1 > 0.0);

    val = 0;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->CalcFormulaTree(false, false);
    double zero;
    m_pDoc->GetValue(0, 1, 0, zero);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Result should equal the 3rd parameter of IF, which is zero.", 0.0, zero);

    val = 1;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->CalcFormulaTree(false, false);
    double now2;
    m_pDoc->GetValue(0, 1, 0, now2);
    CPPUNIT_ASSERT_MESSAGE("Result should be the value of NOW() again.", (now2 - now1) >= 0.0);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncNUMBERVALUE()
{
    // NUMBERVALUE fdo#57180

    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    // Empty A1:A39 first.
    clearRange(m_pDoc, ScRange(0, 0, 0, 0, 40, 0));

    // Raw data (rows 1 through 6)
    const char* aData[] = {
        "1ag9a9b9",
        "1ag34 5g g6  78b9%%",
        "1 234d56E-2",
        "d4",
        "54.4",
        "1a2b3e1%"
    };

    SCROW nRows = SAL_N_ELEMENTS(aData);
    for (SCROW i = 0; i < nRows; ++i)
        m_pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i]));

    printRange(m_pDoc, ScRange(0, 0, 0, 0, nRows - 1, 0), "data range for NUMBERVALUE");

    // formulas and results
    struct {
        const char* pFormula; const char* pResult;
    } aChecks[] = {
        { "=NUMBERVALUE(A1;\"b\";\"ag\")",  "199.9" },
        { "=NUMBERVALUE(A2;\"b\";\"ag\")",  "134.56789" },
        { "=NUMBERVALUE(A2;\"b\";\"g\")",   "#VALUE!" },
        { "=NUMBERVALUE(A3;\"d\")",         "12.3456" },
        { "=NUMBERVALUE(A4;\"d\";\"foo\")", "0.4" },
        { "=NUMBERVALUE(A4;)",              "Err:502" },
        { "=NUMBERVALUE(A5;)",              "Err:502" },
        { "=NUMBERVALUE(A6;\"b\";\"a\")",   "1.23" }
    };

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
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            aChecks[i].pFormula, OUString::createFromAscii( aChecks[i].pResult), aResult);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFuncLEN()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Formula");

    // Leave A1:A3 empty, and insert an array of LEN in B1:B3 that references
    // these empty cells.

    ScMarkData aMark;
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(1, 0, 1, 2, aMark, "=LEN(A1:A3)");

    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula should be a matrix origin.",
                           MM_FORMULA, static_cast<ScMatrixMode>(pFC->GetMatrixFlag()));

    // This should be a 1x3 matrix.
    SCCOL nCols = -1;
    SCROW nRows = -1;
    pFC->GetMatColsRows(nCols, nRows);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCols);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), nRows);

    // LEN value should be 0 for an empty cell.
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncLOOKUP()
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
        m_pDoc->SetString(0, i+4, 0, OUString::createFromAscii(aData2[i][0]));
        m_pDoc->SetString(1, i+4, 0, OUString::createFromAscii(aData2[i][1]));
    }

    printRange(m_pDoc, ScRange(0,4,0,1,6,0), "Data range for LOOKUP.");

    // Values for B5:B7 should be 1, 2, and 3.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula should not have an error code.", static_cast<sal_uInt16>(0), m_pDoc->GetErrCode(ScAddress(1,4,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula should not have an error code.", static_cast<sal_uInt16>(0), m_pDoc->GetErrCode(ScAddress(1,5,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula should not have an error code.", static_cast<sal_uInt16>(0), m_pDoc->GetErrCode(ScAddress(1,6,0)));

    ASSERT_DOUBLES_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,4,0)));
    ASSERT_DOUBLES_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,5,0)));
    ASSERT_DOUBLES_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,6,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncVLOOKUP()
{
    // VLOOKUP

    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    // Clear A1:F40.
    clearRange(m_pDoc, ScRange(0, 0, 0, 5, 39, 0));

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
        { nullptr, nullptr } // terminator
    };

    // Insert raw data into A1:B14.
    for (SCROW i = 0; aData[i][0]; ++i)
    {
        m_pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i][0]));
        m_pDoc->SetString(1, i, 0, OUString::createFromAscii(aData[i][1]));
    }

    printRange(m_pDoc, ScRange(0, 0, 0, 1, 13, 0), "raw data for VLOOKUP");

    // Formula data
    struct {
        const char* pLookup; const char* pFormula; const char* pRes;
    } aChecks[] = {
        { "Lookup",  "Formula", nullptr },
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
            cerr << "row " << (i+1) << ": lookup value='" << aChecks[i].pLookup
                << "'  expected='" << aChecks[i].pRes << "' actual='" << aRes << "'" << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for VLOOKUP", false);
        }
    }

    // Clear the sheet and start over.
    clearSheet(m_pDoc, 0);

    // Lookup on sorted data intersparsed with empty cells.

    // A1:B8 is the search range.
    m_pDoc->SetValue(ScAddress(0,2,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,4,0), 2.0);
    m_pDoc->SetValue(ScAddress(0,7,0), 4.0);
    m_pDoc->SetString(ScAddress(1,2,0), "One");
    m_pDoc->SetString(ScAddress(1,4,0), "Two");
    m_pDoc->SetString(ScAddress(1,7,0), "Four");

    // D1:D5 contain match values.
    m_pDoc->SetValue(ScAddress(3,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(3,1,0), 2.0);
    m_pDoc->SetValue(ScAddress(3,2,0), 3.0);
    m_pDoc->SetValue(ScAddress(3,3,0), 4.0);
    m_pDoc->SetValue(ScAddress(3,4,0), 5.0);

    // E1:E5 contain formulas.
    m_pDoc->SetString(ScAddress(4,0,0), "=VLOOKUP(D1;$A$1:$B$8;2)");
    m_pDoc->SetString(ScAddress(4,1,0), "=VLOOKUP(D2;$A$1:$B$8;2)");
    m_pDoc->SetString(ScAddress(4,2,0), "=VLOOKUP(D3;$A$1:$B$8;2)");
    m_pDoc->SetString(ScAddress(4,3,0), "=VLOOKUP(D4;$A$1:$B$8;2)");
    m_pDoc->SetString(ScAddress(4,4,0), "=VLOOKUP(D5;$A$1:$B$8;2)");
    m_pDoc->CalcAll();

    // Check the formula results in E1:E5.
    CPPUNIT_ASSERT_EQUAL(OUString("One"), m_pDoc->GetString(ScAddress(4,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Two"), m_pDoc->GetString(ScAddress(4,1,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Two"), m_pDoc->GetString(ScAddress(4,2,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Four"), m_pDoc->GetString(ScAddress(4,3,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Four"), m_pDoc->GetString(ScAddress(4,4,0)));

    // Start over again.
    clearSheet(m_pDoc, 0);

    // Set A,B,....,G to A1:A7.
    m_pDoc->SetString(ScAddress(0,0,0), "A");
    m_pDoc->SetString(ScAddress(0,1,0), "B");
    m_pDoc->SetString(ScAddress(0,2,0), "C");
    m_pDoc->SetString(ScAddress(0,3,0), "D");
    m_pDoc->SetString(ScAddress(0,4,0), "E");
    m_pDoc->SetString(ScAddress(0,5,0), "F");
    m_pDoc->SetString(ScAddress(0,6,0), "G");

    // Set the formula in C1.
    m_pDoc->SetString(ScAddress(2,0,0), "=VLOOKUP(\"C\";A1:A16;1)");
    CPPUNIT_ASSERT_EQUAL(OUString("C"), m_pDoc->GetString(ScAddress(2,0,0)));


    // A21:E24, test position dependent implicit intersection as argument to a
    // scalar value parameter in a function that has a ReferenceOrForceArray
    // type parameter somewhere else and formula is not in array mode,
    // VLOOKUP(Value;ReferenceOrForceArray;...)
    const char* aData2[][5] = {
        { "1", "one",   "3", "=VLOOKUP(C21:C24;A21:B24;2;0)", "three" },
        { "2", "two",   "1", "=VLOOKUP(C21:C24;A21:B24;2;0)", "one"   },
        { "3", "three", "4", "=VLOOKUP(C21:C24;A21:B24;2;0)", "four"  },
        { "4", "four",  "2", "=VLOOKUP(C21:C24;A21:B24;2;0)", "two"   }
    };

    ScAddress aPos2(0,20,0);
    ScRange aRange2 = insertRangeData(m_pDoc, aPos2, aData2, SAL_N_ELEMENTS(aData2));
    CPPUNIT_ASSERT_EQUAL(aPos2, aRange2.aStart);

    aPos2.SetCol(3);    // column D formula results
    for (size_t i=0; i < SAL_N_ELEMENTS(aData2); ++i)
    {
        CPPUNIT_ASSERT_EQUAL( OUString::createFromAscii( aData2[i][4]), m_pDoc->GetString(aPos2));
        aPos2.IncRow();
    }

    m_pDoc->DeleteTab(0);
}

struct StrStrCheck {
    const char* pVal;
    const char* pRes;
};

template<size_t DataSize, size_t FormulaSize, int Type>
void runTestMATCH(ScDocument* pDoc, const char* aData[DataSize], StrStrCheck aChecks[FormulaSize])
{
    size_t nDataSize = DataSize;
    for (size_t i = 0; i < nDataSize; ++i)
        pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i]));

    for (size_t i = 0; i < FormulaSize; ++i)
    {
        pDoc->SetString(1, i, 0, OUString::createFromAscii(aChecks[i].pVal));

        OUStringBuffer aBuf;
        aBuf.append("=MATCH(B");
        aBuf.append(static_cast<sal_Int32>(i+1));
        aBuf.append(";A1:A");
        aBuf.append(static_cast<sal_Int32>(nDataSize));
        aBuf.append(";");
        aBuf.append(static_cast<sal_Int32>(Type));
        aBuf.append(")");
        OUString aFormula = aBuf.makeStringAndClear();
        pDoc->SetString(2, i, 0, aFormula);
    }

    pDoc->CalcAll();
    Test::printRange(pDoc, ScRange(0, 0, 0, 2, FormulaSize-1, 0), "MATCH");

    // verify the results.
    for (size_t i = 0; i < FormulaSize; ++i)
    {
        OUString aStr = pDoc->GetString(2, i, 0);
        if (!aStr.equalsAscii(aChecks[i].pRes))
        {
            cerr << "row " << (i+1) << ": expected='" << aChecks[i].pRes << "' actual='" << aStr << "'"
                " criterion='" << aChecks[i].pVal << "'" << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for MATCH", false);
        }
    }
}

template<size_t DataSize, size_t FormulaSize, int Type>
void runTestHorizontalMATCH(ScDocument* pDoc, const char* aData[DataSize], StrStrCheck aChecks[FormulaSize])
{
    size_t nDataSize = DataSize;
    for (size_t i = 0; i < nDataSize; ++i)
        pDoc->SetString(i, 0, 0, OUString::createFromAscii(aData[i]));

    for (size_t i = 0; i < FormulaSize; ++i)
    {
        pDoc->SetString(i, 1, 0, OUString::createFromAscii(aChecks[i].pVal));

        // Assume we don't have more than 26 data columns..
        OUStringBuffer aBuf;
        aBuf.append("=MATCH(");
        aBuf.append(static_cast<sal_Unicode>('A'+i));
        aBuf.append("2;A1:");
        aBuf.append(static_cast<sal_Unicode>('A'+nDataSize));
        aBuf.append("1;");
        aBuf.append(static_cast<sal_Int32>(Type));
        aBuf.append(")");
        OUString aFormula = aBuf.makeStringAndClear();
        pDoc->SetString(i, 2, 0, aFormula);
    }

    pDoc->CalcAll();
    Test::printRange(pDoc, ScRange(0, 0, 0, FormulaSize-1, 2, 0), "MATCH");

    // verify the results.
    for (size_t i = 0; i < FormulaSize; ++i)
    {
        OUString aStr = pDoc->GetString(i, 2, 0);
        if (!aStr.equalsAscii(aChecks[i].pRes))
        {
            cerr << "column " << char('A'+i) << ": expected='" << aChecks[i].pRes << "' actual='" << aStr << "'"
                " criterion='" << aChecks[i].pVal << "'" << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for horizontal MATCH", false);
        }
    }
}

void Test::testFuncMATCH()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    clearRange(m_pDoc, ScRange(0, 0, 0, 40, 40, 0));
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

        runTestMATCH<SAL_N_ELEMENTS(aData),SAL_N_ELEMENTS(aChecks),1>(m_pDoc, aData, aChecks);
        clearRange(m_pDoc, ScRange(0, 0, 0, 4, 40, 0));
        runTestHorizontalMATCH<SAL_N_ELEMENTS(aData),SAL_N_ELEMENTS(aChecks),1>(m_pDoc, aData, aChecks);
        clearRange(m_pDoc, ScRange(0, 0, 0, 40, 4, 0));
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

        runTestMATCH<SAL_N_ELEMENTS(aData),SAL_N_ELEMENTS(aChecks),-1>(m_pDoc, aData, aChecks);
        clearRange(m_pDoc, ScRange(0, 0, 0, 4, 40, 0));
        runTestHorizontalMATCH<SAL_N_ELEMENTS(aData),SAL_N_ELEMENTS(aChecks),-1>(m_pDoc, aData, aChecks);
        clearRange(m_pDoc, ScRange(0, 0, 0, 40, 4, 0));
    }

    {
        // search range contains leading and trailing empty cell ranges.

        clearRange(m_pDoc, ScRange(0,0,0,2,100,0));

        // A5:A8 contains sorted values.
        m_pDoc->SetValue(ScAddress(0,4,0), 1.0);
        m_pDoc->SetValue(ScAddress(0,5,0), 2.0);
        m_pDoc->SetValue(ScAddress(0,6,0), 3.0);
        m_pDoc->SetValue(ScAddress(0,7,0), 4.0);

        // Find value 2 which is in A6.
        m_pDoc->SetString(ScAddress(1,0,0), "=MATCH(2;A1:A20)");
        m_pDoc->CalcAll();

        CPPUNIT_ASSERT_EQUAL(OUString("6"), m_pDoc->GetString(ScAddress(1,0,0)));
    }

    {
        // Test the ReferenceOrForceArray parameter.

        clearRange(m_pDoc, ScRange(0,0,0,1,7,0));

        // B1:B5 contain numeric values.
        m_pDoc->SetValue(ScAddress(1,0,0), 1.0);
        m_pDoc->SetValue(ScAddress(1,1,0), 2.0);
        m_pDoc->SetValue(ScAddress(1,2,0), 3.0);
        m_pDoc->SetValue(ScAddress(1,3,0), 4.0);
        m_pDoc->SetValue(ScAddress(1,4,0), 5.0);

        // Find string value "33" in concatenated array, no implicit
        // intersection is involved, array is forced.
        m_pDoc->SetString(ScAddress(0,5,0), "=MATCH(\"33\";B1:B5&B1:B5)");
        m_pDoc->CalcAll();
        CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFuncCELL()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    clearRange(m_pDoc, ScRange(0, 0, 0, 2, 20, 0)); // Clear A1:C21.

    {
        const char* pContent = "Some random text";
        m_pDoc->SetString(2, 9, 0, OUString::createFromAscii(pContent)); // Set this value to C10.
        double val = 1.2;
        m_pDoc->SetValue(2, 0, 0, val); // Set numeric value to C1;

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
void Test::testFuncDATEDIF()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

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

    clearRange( m_pDoc, ScRange(0, 0, 0, 4, SAL_N_ELEMENTS(aData), 0));
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aDataRange.aStart);

    m_pDoc->CalcAll();

    for (size_t i = 0; i < SAL_N_ELEMENTS(aData); ++i)
    {
        OUString aVal = m_pDoc->GetString( 4, i, 0);
        //std::cout << "row "<< i << ": " << OUStringToOString( aVal, RTL_TEXTENCODING_UTF8).getStr() << ", expected " << aData[i][3] << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Unexpected result for DATEDIF", aVal.equalsAscii( aData[i][3]));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFuncINDIRECT()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));
    clearRange(m_pDoc, ScRange(0, 0, 0, 0, 10, 0)); // Clear A1:A11

    bool bGood = m_pDoc->GetName(0, aTabName);
    CPPUNIT_ASSERT_MESSAGE("failed to get sheet name.", bGood);

    OUString aTest = "Test", aRefErr = "#REF!";
    m_pDoc->SetString(0, 10, 0, aTest);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell value.", aTest, m_pDoc->GetString(0,10,0));

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
        const OUString* aChecks[] = {
            &aTest, &aRefErr, &aRefErr, &aTest
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            OUString aVal = m_pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong value!", *aChecks[i], aVal);
        }
    }

    ScCalcConfig aConfig;
    aConfig.SetStringRefSyntax( formula::FormulaGrammar::CONV_OOO );
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    {
        // Explicit Calc A1 syntax
        const OUString* aChecks[] = {
            &aTest, &aRefErr, &aRefErr, &aTest
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            OUString aVal = m_pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong value!", *aChecks[i], aVal);
        }
    }

    aConfig.SetStringRefSyntax( formula::FormulaGrammar::CONV_XL_A1 );
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    {
        // Excel A1 syntax
        const OUString* aChecks[] = {
            &aRefErr, &aTest, &aRefErr, &aTest
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            OUString aVal = m_pDoc->GetString(0, i, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong value!", *aChecks[i], aVal);
        }
    }

    aConfig.SetStringRefSyntax( formula::FormulaGrammar::CONV_XL_R1C1 );
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    {
        // Excel R1C1 syntax
        const OUString* aChecks[] = {
            &aRefErr, &aRefErr, &aTest, &aTest
        };

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
void Test::testFuncINDIRECT2()
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, "foo"));
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (1, "bar"));
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (2, "baz"));

    ScAddress aStart;
    ScAddress aEnd;
    ScAddress aRef;

    m_pDoc->SetValue(0,0,0, 10.0);
    m_pDoc->SetValue(0,1,0, 10.0);
    m_pDoc->SetValue(0,2,0, 10.0);

    // Fill range bar.$A1:bar.$A10 with 1s
    for (SCROW i = 0; i < 10; ++i)
        m_pDoc->SetValue(0,i,1, 1.0);

    // Test range triplet (absolute, relative, relative) : (absolute, relative, relative)
    m_pDoc->SetString(0,0,2, "=COUNTIF(bar.$A1:INDIRECT(\"$A\"&foo.$A$1),1)");

    // Test range triplet (absolute, relative, relative) : (absolute, absolute, relative)
    m_pDoc->SetString(0,1,2, "=COUNTIF(bar.$A1:INDIRECT(\"$A\"&foo.$A$2),1)");

    // Test range triplet (absolute, relative, relative) : (absolute, absolute, absolute)
    m_pDoc->SetString(0,2,2, "=COUNTIF(bar.$A1:INDIRECT(\"$A\"&foo.$A$3),1)");

    // Test range triplet (absolute, absolute, relative) : (absolute, relative, relative)
    m_pDoc->SetString(0,3,2, "=COUNTIF(bar.$A$1:INDIRECT(\"$A\"&foo.$A$1),1)");

    // Test range triplet (absolute, absolute, relative) : (absolute, absolute, relative)
    m_pDoc->SetString(0,4,2, "=COUNTIF(bar.$A$1:INDIRECT(\"$A\"&foo.$A$2),1)");

    // Test range triplet (absolute, absolute, relative) : (absolute, absolute, relative)
    m_pDoc->SetString(0,5,2, "=COUNTIF(bar.$A$1:INDIRECT(\"$A\"&foo.$A$3),1)");

    // Test range triplet (absolute, absolute, absolute) : (absolute, relative, relative)
    m_pDoc->SetString(0,6,2, "=COUNTIF($bar.$A$1:INDIRECT(\"$A\"&foo.$A$1),1)");

    // Test range triplet (absolute, absolute, absolute) : (absolute, absolute, relative)
    m_pDoc->SetString(0,7,2, "=COUNTIF($bar.$A$1:INDIRECT(\"$A\"&foo.$A$2),1)");

    // Check indirect reference "bar.$A\"&foo.$A$1
    m_pDoc->SetString(0,8,2, "=COUNTIF(bar.$A$1:INDIRECT(\"bar.$A\"&foo.$A$1),1)");

    // This case should return illegal argument error because
    // they reference 2 different absolute sheets
    // Test range triplet (absolute, absolute, absolute) : (absolute, absolute, absolute)
    m_pDoc->SetString(0,9,2, "=COUNTIF($bar.$A$1:INDIRECT(\"$A\"&foo.$A$3),1)");

    m_pDoc->CalcAll();

    // Loop all formulas and check result = 10.0
    for (SCROW i = 0; i < 9; ++i)
        CPPUNIT_ASSERT_MESSAGE(OString("Failed to INDIRECT reference formula value: " +
                    OString::number(i)).getStr(), m_pDoc->GetValue(0,i,2) != 10.0);

    // Check formula cell error
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0,9,2));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This formula cell should be an error.", pFC->GetErrCode() != 0);

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testFormulaDepTracking()
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "foo"));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    // B2 listens on D2.
    m_pDoc->SetString(1, 1, 0, "=D2");
    double val = -999.0; // dummy initial value
    m_pDoc->GetValue(1, 1, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Referencing an empty cell should yield zero.", 0.0, val);

    // Changing the value of D2 should trigger recalculation of B2.
    m_pDoc->SetValue(3, 1, 0, 1.1);
    m_pDoc->GetValue(1, 1, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Failed to recalculate on value change.", 1.1, val);

    // And again.
    m_pDoc->SetValue(3, 1, 0, 2.2);
    m_pDoc->GetValue(1, 1, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Failed to recalculate on value change.", 2.2, val);

    clearRange(m_pDoc, ScRange(0, 0, 0, 10, 10, 0));

    // Now, let's test the range dependency tracking.

    // B2 listens on D2:E6.
    m_pDoc->SetString(1, 1, 0, "=SUM(D2:E6)");
    m_pDoc->GetValue(1, 1, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Summing an empty range should yield zero.", 0.0, val);

    // Set value to E3. This should trigger recalc on B2.
    m_pDoc->SetValue(4, 2, 0, 2.4);
    m_pDoc->GetValue(1, 1, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Failed to recalculate on single value change.", 2.4, val);

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
        ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected formula value.", val, m_pDoc->GetValue(1, nRow, 0));
        ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected formula value.", val*2.0, m_pDoc->GetValue(2, nRow, 0));
    }

    // Intentionally insert a formula in column 1. This will break column 1's
    // uniformity of consisting only of static value cells.
    m_pDoc->SetString(0, 4, 0, "=R2C3");
    ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected formula value.", 2.0, m_pDoc->GetValue(0, 4, 0));
    ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected formula value.", 2.0, m_pDoc->GetValue(1, 4, 0));
    ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected formula value.", 4.0, m_pDoc->GetValue(2, 4, 0));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaDepTracking2()
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "foo"));

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

void Test::testFormulaDepTracking3()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    m_pDoc->InsertTab(0, "Formula");

    const char* pData[][4] = {
        { "1", "2", "=SUM(A1:B1)", "=SUM(C1:C3)" },
        { "3", "4", "=SUM(A2:B2)", nullptr },
        { "5", "6", "=SUM(A3:B3)", nullptr },
    };

    insertRangeData(m_pDoc, ScAddress(0,0,0), pData, SAL_N_ELEMENTS(pData));

    // Check the initial formula results.
    CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL( 7.0, m_pDoc->GetValue(ScAddress(2,1,0)));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(2,2,0)));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(ScAddress(3,0,0)));

    // Change B3 and make sure the change gets propagated to D1.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    rFunc.SetValueCell(ScAddress(1,2,0), 60.0, false);
    CPPUNIT_ASSERT_EQUAL(65.0, m_pDoc->GetValue(ScAddress(2,2,0)));
    CPPUNIT_ASSERT_EQUAL(75.0, m_pDoc->GetValue(ScAddress(3,0,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaDepTrackingDeleteRow()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    m_pDoc->InsertTab(0, "Test");

    // Values in A1:A3.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 3.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 5.0);

    // SUM(A1:A3) in A5.
    m_pDoc->SetString(ScAddress(0,4,0), "=SUM(A1:A3)");

    // A6 to reference A5.
    m_pDoc->SetString(ScAddress(0,5,0), "=A5*10");
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0,5,0));
    CPPUNIT_ASSERT(pFC);

    // A4 should have a broadcaster with A5 listening to it.
    SvtBroadcaster* pBC = m_pDoc->GetBroadcaster(ScAddress(0,4,0));
    CPPUNIT_ASSERT(pBC);
    SvtBroadcaster::ListenersType* pListeners = &pBC->GetAllListeners();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A5 should have one listener.", size_t(1), pListeners->size());
    SvtListener* pListener = pListeners->at(0);
    CPPUNIT_ASSERT_MESSAGE("A6 should be listening to A5.", pListener == pFC);

    // Check initial values.
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(90.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Delete row 2.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    rFunc.DeleteCells(ScRange(0,1,0,MAXCOL,1,0), &aMark, DEL_CELLSUP, true);

    pBC = m_pDoc->GetBroadcaster(ScAddress(0,3,0));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster at A5 should have shifted to A4.", pBC);
    pListeners = &pBC->GetAllListeners();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A3 should have one listener.", size_t(1), pListeners->size());
    pFC = m_pDoc->GetFormulaCell(ScAddress(0,4,0));
    CPPUNIT_ASSERT(pFC);
    pListener = pListeners->at(0);
    CPPUNIT_ASSERT_MESSAGE("A5 should be listening to A4.", pFC == pListener);

    // Check values after row deletion.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(60.0, m_pDoc->GetValue(ScAddress(0,4,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaDepTrackingDeleteCol()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    m_pDoc->InsertTab(0, "Formula");

    const char* aData[][3] = {
        { "2", "=A1", "=B1" }, // not grouped
        { nullptr, nullptr, nullptr },           // empty row to separate the formula groups.
        { "3", "=A3", "=B3" }, // grouped
        { "4", "=A4", "=B4" }, // grouped
    };

    ScAddress aPos(0,0,0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    // Check the initial values.
    for (SCCOL i = 0; i <= 2; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(i,0,0)));
        CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(i,2,0)));
        CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(i,3,0)));
    }

    // Make sure B3:B4 and C3:C4 are grouped.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,2,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2,2,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    // Delete column A.  A1, B1, A3:A4 and B3:B4 should all show #REF!.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    rFunc.DeleteCells(ScRange(0,0,0,0,MAXROW,0), &aMark, DEL_CELLSLEFT, true);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "#REF!", "#REF!" },
            { nullptr,  nullptr },
            { "#REF!", "#REF!" },
            { "#REF!", "#REF!" },
        };

        ScRange aCheckRange(0,0,0,1,3,0);
        bool bSuccess = checkOutput<2>(m_pDoc, aCheckRange, aOutputCheck, "Check after deleting column A");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Undo and check the result.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { "2", "2", "2" },
            { nullptr,  nullptr, nullptr },
            { "3", "3", "3" },
            { "4", "4", "4" },
        };

        ScRange aCheckRange(0,0,0,2,3,0);
        bool bSuccess = checkOutput<3>(m_pDoc, aCheckRange, aOutputCheck, "Check after undo");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Redo and check.
    pUndoMgr->Redo();
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "#REF!", "#REF!" },
            { nullptr, nullptr },
            { "#REF!", "#REF!" },
            { "#REF!", "#REF!" },
        };

        ScRange aCheckRange(0,0,0,1,3,0);
        bool bSuccess = checkOutput<2>(m_pDoc, aCheckRange, aOutputCheck, "Check after redo");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Undo and change the values in column A.
    pUndoMgr->Undo();
    m_pDoc->SetValue(ScAddress(0,0,0), 22.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 23.0);
    m_pDoc->SetValue(ScAddress(0,3,0), 24.0);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { "22", "22", "22" },
            { nullptr, nullptr, nullptr },
            { "23", "23", "23" },
            { "24", "24", "24" },
        };

        ScRange aCheckRange(0,0,0,2,3,0);
        bool bSuccess = checkOutput<3>(m_pDoc, aCheckRange, aOutputCheck, "Check after undo & value change in column A");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaMatrixResultUpdate()
{
    m_pDoc->InsertTab(0, "Test");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    // Set a numeric value to A1.
    m_pDoc->SetValue(ScAddress(0,0,0), 11.0);

    ScMarkData aMark;
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(1, 0, 1, 0, aMark, "=A1");
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("Failed to get formula cell.", pFC);
    pFC->SetChanged(false); // Clear this flag to simulate displaying of formula cell value on screen.

    m_pDoc->SetString(ScAddress(0,0,0), "ABC");
    CPPUNIT_ASSERT_EQUAL(OUString("ABC"), m_pDoc->GetString(ScAddress(1,0,0)));
    pFC->SetChanged(false);

    // Put a new value into A1. The formula should update.
    m_pDoc->SetValue(ScAddress(0,0,0), 13.0);
    CPPUNIT_ASSERT_EQUAL(13.0, m_pDoc->GetValue(ScAddress(1,0,0)));

    m_pDoc->DeleteTab(0);
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
                           findLoadedDocShellByName(aExtDocName) != nullptr);

    // Populate the external source document.
    ScDocument& rExtDoc = xExtDocSh->GetDocument();
    rExtDoc.InsertTab(0, aExtSh1Name);
    rExtDoc.InsertTab(1, aExtSh2Name);
    rExtDoc.InsertTab(2, aExtSh3Name);

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
    rExtDoc.SetString(0, 0, 0, name);
    rExtDoc.SetString(0, 1, 0, andy);
    rExtDoc.SetString(0, 2, 0, bruce);
    rExtDoc.SetString(0, 3, 0, charlie);
    rExtDoc.SetString(0, 4, 0, david);
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
    rExtDoc.SetString(0, 1, 2, edward);
    rExtDoc.SetString(0, 2, 2, frank);
    rExtDoc.SetString(0, 3, 2, george);
    rExtDoc.SetString(0, 4, 2, henry);
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

    m_pDoc->SetString(1, 0, 0, "='file:///extdata.fake'#Data1.B1");
    test = m_pDoc->GetString(1, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Value is different from the original", test.equals(value));

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
            test = m_pDoc->GetString(0, static_cast<SCROW>(i+1), 0);
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
            m_pDoc->GetValue(1, static_cast<SCROW>(i+1), 0, val);
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
    ScExternalRefCache::TableTypeRef pCacheTab = pRefMgr->getCacheTable(
        nFileId, aExtSh1Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 1 should exist.", pCacheTab.get() != nullptr);
    ScRange aCachedRange = getCachedRange(pCacheTab);
    CPPUNIT_ASSERT_MESSAGE("Unexpected cached data range.",
                           aCachedRange.aStart.Col() == 0 && aCachedRange.aEnd.Col() == 1 &&
                           aCachedRange.aStart.Row() == 0 && aCachedRange.aEnd.Row() == 4);

    // Sheet2 is not referenced at all; the cache table shouldn't even exist.
    pCacheTab = pRefMgr->getCacheTable(nFileId, aExtSh2Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 2 should *not* exist.", pCacheTab.get() == nullptr);

    // Sheet3's row 5 is not referenced; it should not be cached.
    pCacheTab = pRefMgr->getCacheTable(nFileId, aExtSh3Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 3 should exist.", pCacheTab.get() != nullptr);
    aCachedRange = getCachedRange(pCacheTab);
    CPPUNIT_ASSERT_MESSAGE("Unexpected cached data range.",
                           aCachedRange.aStart.Col() == 0 && aCachedRange.aEnd.Col() == 1 &&
                           aCachedRange.aStart.Row() == 0 && aCachedRange.aEnd.Row() == 3);

    // Unload the external document shell.
    xExtDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("external document instance should have been unloaded.",
                           findLoadedDocShellByName(aExtDocName) == nullptr);

    m_pDoc->DeleteTab(0);
}

void Test::testExternalRangeName()
{
    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString aExtDocName("file:///extdata.fake");
    OUString aExtSh1Name("Data1");
    SfxMedium* pMed = new SfxMedium(aExtDocName, STREAM_STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != nullptr);

    ScDocument& rExtDoc = xExtDocSh->GetDocument();
    rExtDoc.InsertTab(0, aExtSh1Name);
    rExtDoc.SetValue(0, 0, 0, 123.456);

    ScRangeName* pRangeName = rExtDoc.GetRangeName();
    ScRangeData* pRangeData = new ScRangeData(&rExtDoc, "ExternalName",
            "$Data1.$A$1");
    pRangeName->insert(pRangeData);

    m_pDoc->InsertTab(0, "Test Sheet");
    m_pDoc->SetString(0, 1, 0, "='file:///extdata.fake'#ExternalName");

    double nVal = m_pDoc->GetValue(0, 1, 0);
    ASSERT_DOUBLES_EQUAL(123.456, nVal);

    xExtDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("external document instance should have been unloaded.",
                           findLoadedDocShellByName(aExtDocName) == nullptr);
    m_pDoc->DeleteTab(0);
}

void testExtRefFuncT(ScDocument* pDoc, ScDocument& rExtDoc)
{
    Test::clearRange(pDoc, ScRange(0, 0, 0, 1, 9, 0));
    Test::clearRange(&rExtDoc, ScRange(0, 0, 0, 1, 9, 0));

    rExtDoc.SetString(0, 0, 0, "'1.2");
    rExtDoc.SetString(0, 1, 0, "Foo");
    rExtDoc.SetValue(0, 2, 0, 12.3);
    pDoc->SetString(0, 0, 0, "=T('file:///extdata.fake'#Data.A1)");
    pDoc->SetString(0, 1, 0, "=T('file:///extdata.fake'#Data.A2)");
    pDoc->SetString(0, 2, 0, "=T('file:///extdata.fake'#Data.A3)");
    pDoc->CalcAll();

    OUString aRes = pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected result with T.", OUString("1.2"), aRes);
    aRes = pDoc->GetString(0, 1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected result with T.", OUString("Foo"), aRes);
    aRes = pDoc->GetString(0, 2, 0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected result with T.", aRes.isEmpty());
}

void testExtRefFuncOFFSET(ScDocument* pDoc, ScDocument& rExtDoc)
{
    Test::clearRange(pDoc, ScRange(0, 0, 0, 1, 9, 0));
    Test::clearRange(&rExtDoc, ScRange(0, 0, 0, 1, 9, 0));

    sc::AutoCalcSwitch aACSwitch(*pDoc, true);

    // External document has sheet named 'Data', and the internal doc has sheet named 'Test'.
    rExtDoc.SetValue(ScAddress(0,1,0), 1.2); // Set 1.2 to A2.
    pDoc->SetString(ScAddress(0,0,0), "=OFFSET('file:///extdata.fake'#Data.$A$1;1;0;1;1)");
    CPPUNIT_ASSERT_EQUAL(1.2, pDoc->GetValue(ScAddress(0,0,0)));
}

void testExtRefFuncVLOOKUP(ScDocument* pDoc, ScDocument& rExtDoc)
{
    Test::clearRange(pDoc, ScRange(0, 0, 0, 1, 9, 0));
    Test::clearRange(&rExtDoc, ScRange(0, 0, 0, 1, 9, 0));

    // Populate the external document.
    rExtDoc.SetString(ScAddress(0,0,0), "A1");
    rExtDoc.SetString(ScAddress(0,1,0), "A2");
    rExtDoc.SetString(ScAddress(0,2,0), "A3");
    rExtDoc.SetString(ScAddress(0,3,0), "A4");
    rExtDoc.SetString(ScAddress(0,4,0), "A5");

    rExtDoc.SetString(ScAddress(1,0,0), "B1");
    rExtDoc.SetString(ScAddress(1,1,0), "B2");
    rExtDoc.SetString(ScAddress(1,2,0), "B3");
    rExtDoc.SetString(ScAddress(1,3,0), "B4");
    rExtDoc.SetString(ScAddress(1,4,0), "B5");

    // Put formula in the source document.

    pDoc->SetString(ScAddress(0,0,0), "A2");

    // Sort order TRUE
    pDoc->SetString(ScAddress(1,0,0), "=VLOOKUP(A1;'file:///extdata.fake'#Data.A1:B5;2;1)");
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), pDoc->GetString(ScAddress(1,0,0)));

    // Sort order FALSE. It should return the same result.
    pDoc->SetString(ScAddress(1,0,0), "=VLOOKUP(A1;'file:///extdata.fake'#Data.A1:B5;2;0)");
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), pDoc->GetString(ScAddress(1,0,0)));
}

void Test::testExternalRefFunctions()
{
    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString aExtDocName("file:///extdata.fake");
    SfxMedium* pMed = new SfxMedium(aExtDocName, STREAM_STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != nullptr);

    ScExternalRefManager* pRefMgr = m_pDoc->GetExternalRefManager();
    CPPUNIT_ASSERT_MESSAGE("external reference manager doesn't exist.", pRefMgr);
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aExtDocName);
    const OUString* pFileName = pRefMgr->getExternalFileName(nFileId);
    CPPUNIT_ASSERT_MESSAGE("file name registration has somehow failed.",
                           pFileName && pFileName->equals(aExtDocName));

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

    struct {
        const char* pFormula; double fResult;
    } aChecks[] = {
        { "=SUM('file:///extdata.fake'#Data.A1:A4)",     10 },
        { "=SUM('file:///extdata.fake'#Data.B1:B4)",     9 },
        { "=AVERAGE('file:///extdata.fake'#Data.A1:A4)", 2.5 },
        { "=AVERAGE('file:///extdata.fake'#Data.B1:B4)", 3 },
        { "=COUNT('file:///extdata.fake'#Data.A1:A4)",   4 },
        { "=COUNT('file:///extdata.fake'#Data.B1:B4)",   3 },
        // Should not crash, MUST be 0,MAXROW and/or 0,MAXCOL range (here both)
        // to yield a result instead of 1x1 error matrix.
        { "=SUM('file:///extdata.fake'#Data.1:1048576)", 19 }
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        m_pDoc->SetString(0, 0, 0, OUString::createFromAscii(aChecks[i].pFormula));
        m_pDoc->GetValue(0, 0, 0, val);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("unexpected result involving external ranges.", aChecks[i].fResult, val, 1e-15);
    }

    // A huge external range should not crash, the matrix generated from the
    // external range reference should be 1x1 and have one error value.
    // XXX NOTE: in case we supported sparse matrix that can hold this large
    // areas these tests may be adapted.
    m_pDoc->SetString(0, 0, 0, "=SUM('file:///extdata.fake'#Data.B1:AMJ1048575)");
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell( ScAddress(0,0,0));
    sal_uInt16 nErr = pFC->GetErrCode();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("huge external range reference expected to yield errMatrixSize", errMatrixSize, nErr);

    ScMarkData aMark;
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0,0,0,0, aMark, "'file:///extdata.fake'#Data.B1:AMJ1048575");
    pFC = m_pDoc->GetFormulaCell( ScAddress(0,0,0));
    nErr = pFC->GetErrCode();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("huge external range reference expected to yield errMatrixSize", errMatrixSize, nErr);
    SCSIZE nMatCols, nMatRows;
    const ScMatrix* pMat = pFC->GetMatrix();
    CPPUNIT_ASSERT_MESSAGE("matrix expected", pMat != nullptr);
    pMat->GetDimensions( nMatCols, nMatRows);
    CPPUNIT_ASSERT_MESSAGE("1x1 matrix expected", nMatCols == 1 && nMatRows == 1);

    pRefMgr->clearCache(nFileId);
    testExtRefFuncT(m_pDoc, rExtDoc);
    testExtRefFuncOFFSET(m_pDoc, rExtDoc);
    testExtRefFuncVLOOKUP(m_pDoc, rExtDoc);

    // Unload the external document shell.
    xExtDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("external document instance should have been unloaded.",
                           findLoadedDocShellByName(aExtDocName) == nullptr);

    m_pDoc->DeleteTab(0);
}

void Test::testExternalRefUnresolved()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.
    m_pDoc->InsertTab(0, "Test");

    // Test error propagation of unresolved (not existing document) external
    // references. Well, let's hope no build machine has such file with sheet..

    const char* aData[][1] = {
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

    ScAddress aPos(0,0,0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    const char* aOutputCheck[][1] = {
        { "#REF!" },    // plain single ref
        { "#REF!" },    // +23
        { "#REF!" },    // &"W"
        { "FALSE" },    // ISREF
        { "TRUE"  },    // ISERROR
        { "TRUE"  },    // ISERR
        { "FALSE" },    // ISBLANK
        { "FALSE" },    // ISNUMBER
        { "FALSE" },    // ISTEXT
        { "FALSE" },    // ISNUMBER
        { "FALSE" },    // ISTEXT
        { "#REF!" },    // =0
        { "#REF!" },    // =""
        { "#REF!" },    // INDIRECT
        { "#REF!" },    // A1:A2 range
        { "#REF!" },    // +23
        { "#REF!" },    // &"W"
        { "FALSE" },    // ISREF
        { "TRUE"  },    // ISERROR
        { "TRUE"  },    // ISERR
        { "FALSE" },    // ISBLANK
        { "FALSE" },    // ISNUMBER
        { "FALSE" },    // ISTEXT
        { "FALSE" },    // ISNUMBER
        { "FALSE" },    // ISTEXT
        // TODO: gives Err:504 FIXME { "#REF!" },    // =0
        // TODO: gives Err:504 FIXME { "#REF!" },    // =""
        { "#REF!" },    // INDIRECT
    };

    bool bSuccess = checkOutput<1>(m_pDoc, aRange, aOutputCheck, "Check unresolved external reference.");
    CPPUNIT_ASSERT_MESSAGE("Unresolved reference check failed", bSuccess);

    m_pDoc->DeleteTab(0);
}

void Test::testMatrixOp()
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
        CPPUNIT_ASSERT_EQUAL( fResult[i], m_pDoc->GetValue(4, i, 0));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFuncRangeOp()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");
    m_pDoc->InsertTab(2, "Sheet3");

    // Sheet1.B1:B3
    m_pDoc->SetValue(1,0,0, 1.0);
    m_pDoc->SetValue(1,1,0, 2.0);
    m_pDoc->SetValue(1,2,0, 4.0);
    // Sheet2.B1:B3
    m_pDoc->SetValue(1,0,1, 8.0);
    m_pDoc->SetValue(1,1,1, 16.0);
    m_pDoc->SetValue(1,2,1, 32.0);
    // Sheet3.B1:B3
    m_pDoc->SetValue(1,0,2, 64.0);
    m_pDoc->SetValue(1,1,2, 128.0);
    m_pDoc->SetValue(1,2,2, 256.0);

    // Range operator should extend concatenated literal references during
    // parse time already, so with this we can test ScComplexRefData::Extend()

    // Current sheet is Sheet1, so B1:B2 implies relative Sheet1.B1:B2

    ScAddress aPos(0,0,0);
    m_pDoc->SetString( aPos, "=SUM(B1:B2:B3)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(B1:B3)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL( 7.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString( aPos, "=SUM(B1:B3:B2)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(B1:B3)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL( 7.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString( aPos, "=SUM(B2:B3:B1)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(B1:B3)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL( 7.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString( aPos, "=SUM(Sheet2.B1:B2:B3)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(Sheet2.B1:B3)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL( 56.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString( aPos, "=SUM(B2:B2:Sheet1.B2)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(Sheet1.B2:B2)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString( aPos, "=SUM(B2:B3:Sheet2.B1)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(Sheet1.B1:Sheet2.B3)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL( 63.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString( aPos, "=SUM(Sheet1.B1:Sheet2.B2:Sheet3.B3)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(Sheet1.B1:Sheet3.B3)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL( 511.0, m_pDoc->GetValue(aPos));

    // B1:Sheet2.B2 would be ambiguous, Sheet1.B1:Sheet2.B2 or Sheet2.B1:B2
    // The actual representation of the error case may change, so this test may
    // have to be adapted.
    aPos.IncRow();
    m_pDoc->SetString( aPos, "=SUM(B1:Sheet2.B2:Sheet3.B3)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(#REF!.B2:#REF!.B3)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL( OUString("#REF!"), m_pDoc->GetString(aPos));

    aPos.IncRow();
    m_pDoc->SetString( aPos, "=SUM(Sheet1.B1:Sheet3.B2:Sheet2.B3)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(Sheet1.B1:Sheet3.B3)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL( 511.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString( aPos, "=SUM(B$2:B$2:B2)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, aPos, "SUM(B$2:B2)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(aPos));

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testFuncFORMULA()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Sheet1");

    // Data in B1:D3
    const char* aData[][3] = {
        { "=A1", "=FORMULA(B1)", "=FORMULA(B1:B3)" },
        {     nullptr, "=FORMULA(B2)", "=FORMULA(B1:B3)" },
        { "=A3", "=FORMULA(B3)", "=FORMULA(B1:B3)" },
    };

    ScAddress aPos(1,0,0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    // Checks of C1:D3, where Cy==Dy, and D4:D6
    const char* aChecks[] = {
        "=A1",
        "#N/A",
        "=A3",
    };
    for (size_t i=0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        CPPUNIT_ASSERT_EQUAL( OUString::createFromAscii( aChecks[i]), m_pDoc->GetString(2,i,0));
        CPPUNIT_ASSERT_EQUAL( OUString::createFromAscii( aChecks[i]), m_pDoc->GetString(3,i,0));
    }

    // Matrix in D4:D6, no intersection with B1:B3
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(3, 3, 3, 5, aMark, "=FORMULA(B1:B3)");
    for (size_t i=0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        CPPUNIT_ASSERT_EQUAL( OUString::createFromAscii( aChecks[i]), m_pDoc->GetString(3,i+3,0));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFuncTableRef()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Sheet1");
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    ScDocFunc& rDocFunc = getDocShell().GetDocFunc();

    {
        ScDBCollection* pDBs = m_pDoc->GetDBCollection();
        CPPUNIT_ASSERT_MESSAGE("Failed to fetch DB collection object.", pDBs);

        // Insert "table" database range definition for A1:B4, with default
        // HasHeader=true and HasTotals=false.
        ScDBData* pData = new ScDBData( "table", 0,0,0, 1,3);
        bool bInserted = pDBs->getNamedDBs().insert(pData);
        CPPUNIT_ASSERT_MESSAGE( "Failed to insert \"table\" database range.", bInserted);
    }

    {
        // Populate "table" database range with headers and data in A1:B4
        const char* aData[][2] = {
            { "Header1", "Header2" },
            { "1", "2" },
            { "4", "8" },
            { "16", "32" }
        };
        ScAddress aPos(0,0,0);
        ScRange aRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
        CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);
    }

    // Named expressions that use Table structured references.
    /* TODO: should the item/header separator really be equal to the parameter
     * separator, thus be locale dependent and ';' semicolon here, or should it
     * be a fixed ',' comma instead? */
    struct {
        const char* pName;
        const char* pExpr;
        const char* pCounta; // expected result when used in row 2 (first data row) as argument to COUNTA()
        const char* pSum3;   // expected result when used in row 3 (second data row) as argument to SUM().
        const char* pSum4;   // expected result when used in row 4 (third data row) as argument to SUM().
        const char* pSumX;   // expected result when used in row 5 (non-intersecting) as argument to SUM().
    } aNames[] = {
        { "all",                            "table[[#All]]",                            "8", "63", "63", "63" },
        { "data_implicit",                  "table[]",                                  "6", "63", "63", "63" },
        { "data",                           "table[[#Data]]",                           "6", "63", "63", "63" },
        { "headers",                        "table[[#Headers]]",                        "2",  "0",  "0",  "0" },
        { "header1",                        "table[[Header1]]",                         "3", "21", "21", "21" },
        { "header2",                        "table[[Header2]]",                         "3", "42", "42", "42" },
        { "data_header1",                   "table[[#Data];[Header1]]",                 "3", "21", "21", "21" },
        { "data_header2",                   "table[[#Data];[Header2]]",                 "3", "42", "42", "42" },
        { "this_row",                       "table[[#This Row]]",                       "2", "12", "48", "#VALUE!" },
        { "this_row_header1",               "table[[#This Row];[Header1]]",             "1",  "4", "16", "#VALUE!" },
        { "this_row_header2",               "table[[#This Row];[Header2]]",             "1",  "8", "32", "#VALUE!" },
        { "this_row_range_header_1_to_2",   "table[[#This Row];[Header1]:[Header2]]",   "2", "12", "48", "#VALUE!" }
    };

    {
        // Insert named expressions.
        ScRangeName* pGlobalNames = m_pDoc->GetRangeName();
        CPPUNIT_ASSERT_MESSAGE("Failed to obtain global named expression object.", pGlobalNames);

        for (size_t i = 0, n = SAL_N_ELEMENTS(aNames); i < n; ++i)
        {
            // Choose base position that does not intersect with the database
            // range definition to test later use of [#This Row] results in
            // proper rows.
            ScRangeData* pName = new ScRangeData(
                    m_pDoc, OUString::createFromAscii(aNames[i].pName), OUString::createFromAscii(aNames[i].pExpr),
                    ScAddress(2,4,0), ScRangeData::Type::Name, formula::FormulaGrammar::GRAM_NATIVE);
            bool bInserted = pGlobalNames->insert(pName);
            CPPUNIT_ASSERT_MESSAGE(
                    OString("Failed to insert named expression "+ OString(aNames[i].pName) +".").getStr(), bInserted);
        }
    }

    // Use the named expressions in COUNTA() formulas, on row 2 that intersects.
    for (size_t i = 0, n = SAL_N_ELEMENTS(aNames); i < n; ++i)
    {
        OUString aFormula( "=COUNTA(" + OUString::createFromAscii( aNames[i].pName) + ")");
        ScAddress aPos(3+i,1,0);
        m_pDoc->SetString( aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( aPrefix + OUString::createFromAscii( aNames[i].pCounta),
                aPrefix + m_pDoc->GetString( aPos));
    }

    // Use the named expressions in SUM() formulas, on row 3 that intersects.
    for (size_t i = 0, n = SAL_N_ELEMENTS(aNames); i < n; ++i)
    {
        OUString aFormula( "=SUM(" + OUString::createFromAscii( aNames[i].pName) + ")");
        ScAddress aPos(3+i,2,0);
        m_pDoc->SetString( aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( aPrefix + OUString::createFromAscii( aNames[i].pSum3),
                aPrefix + m_pDoc->GetString( aPos));
    }

    // Use the named expressions in SUM() formulas, on row 4 that intersects.
    for (size_t i = 0, n = SAL_N_ELEMENTS(aNames); i < n; ++i)
    {
        OUString aFormula( "=SUM(" + OUString::createFromAscii( aNames[i].pName) + ")");
        ScAddress aPos(3+i,3,0);
        m_pDoc->SetString( aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( aPrefix + OUString::createFromAscii( aNames[i].pSum4),
                aPrefix + m_pDoc->GetString( aPos));
    }

    // Use the named expressions in SUM() formulas, on row 5 that does not intersect.
    for (size_t i = 0, n = SAL_N_ELEMENTS(aNames); i < n; ++i)
    {
        OUString aFormula( "=SUM(" + OUString::createFromAscii( aNames[i].pName) + ")");
        ScAddress aPos(3+i,4,0);
        m_pDoc->SetString( aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( aPrefix + OUString::createFromAscii( aNames[i].pSumX),
                aPrefix + m_pDoc->GetString( aPos));
    }

    // Insert a column at column B to extend database range from column A,B to
    // A,B,C. Use ScDocFunc so RefreshDirtyTableColumnNames() is called.
    rDocFunc.InsertCells(ScRange(1,0,0,1,MAXROW,0), &aMark, INS_INSCOLS_BEFORE, false, true);

    // Re-verify the named expression in SUM() formula, on row 4 that
    // intersects, now starting at column E, still works.
    m_pDoc->CalcAll();
    for (size_t i = 0, n = SAL_N_ELEMENTS(aNames); i < n; ++i)
    {
        OUString aFormula( "=SUM(" + OUString::createFromAscii( aNames[i].pName) + ")");
        ScAddress aPos(4+i,3,0);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( aPrefix + OUString::createFromAscii( aNames[i].pSum4),
                aPrefix + m_pDoc->GetString( aPos));
    }

    const char* pColumn2Formula = "=SUM(table[[#Data];[Column2]])";
    {
        // Populate "table" database range with empty header and data in newly
        // inserted column, B1:B4 plus a table formula in B6. The empty header
        // should result in the internal table column name "Column2" that is
        // used in the formula.
        const char* aData[][1] = {
            { "" },
            { "64" },
            { "128" },
            { "256" },
            { "" },
            { pColumn2Formula }
        };
        ScAddress aPos(1,0,0);
        ScRange aRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
        CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);
    }

    // Verify the formula result in B6 (64+128+256=448).
    {
        OUString aFormula( OUString::createFromAscii( pColumn2Formula));
        ScAddress aPos(1,5,0);
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( OUString(aPrefix + "448"), OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    // Set header in column B. Use ScDocFunc to have table column names refreshed.
    rDocFunc.SetStringCell(ScAddress(1,0,0), "NewHeader",true);
    // Verify that formula adapted using the updated table column names.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,5,0), "SUM(table[[#Data];[NewHeader]])", "Wrong formula");

    // Set header in column A to identical string. Internal table column name
    // for B should get a "2" appended.
    rDocFunc.SetStringCell(ScAddress(0,0,0), "NewHeader",true);
    // Verify that formula adapted using the updated table column names.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,5,0), "SUM(table[[#Data];[NewHeader2]])", "Wrong formula");

    // Set header in column B to empty string, effectively clearing the cell.
    rDocFunc.SetStringCell(ScAddress(1,0,0), "",true);
    // Verify that formula is still using the previous table column name.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,5,0), "SUM(table[[#Data];[NewHeader2]])", "Wrong formula");

    // === header-less ===

    {
        ScDBCollection* pDBs = m_pDoc->GetDBCollection();
        CPPUNIT_ASSERT_MESSAGE("Failed to fetch DB collection object.", pDBs);

        // Insert "headerless" database range definition for E10:F12, without headers.
        ScDBData* pData = new ScDBData( "hltable", 0, 4,9, 5,11, true, false);
        bool bInserted = pDBs->getNamedDBs().insert(pData);
        CPPUNIT_ASSERT_MESSAGE( "Failed to insert \"hltable\" database range.", bInserted);
    }

    {
        // Populate "hltable" database range with data in E10:F12
        const char* aData[][2] = {
            { "1", "2" },
            { "4", "8" },
            { "16", "32" }
        };
        ScAddress aPos(4,9,0);
        ScRange aRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
        CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);
    }

    // Named expressions that use header-less Table structured references.
    struct {
        const char* pName;
        const char* pExpr;
        const char* pCounta; // expected result when used in row 10 (first data row) as argument to COUNTA()
        const char* pSum3;   // expected result when used in row 11 (second data row) as argument to SUM().
        const char* pSum4;   // expected result when used in row 12 (third data row) as argument to SUM().
        const char* pSumX;   // expected result when used in row 13 (non-intersecting) as argument to SUM().
    } aHlNames[] = {
        { "hl_all",                          "hltable[[#All]]",                          "6", "63", "63", "63" },
        { "hl_data_implicit",                "hltable[]",                                "6", "63", "63", "63" },
        { "hl_data",                         "hltable[[#Data]]",                         "6", "63", "63", "63" },
        { "hl_headers",                      "hltable[[#Headers]]",                      "1", "#REF!", "#REF!", "#REF!" },
        { "hl_column1",                      "hltable[[Column1]]",                       "3", "21", "21", "21" },
        { "hl_column2",                      "hltable[[Column2]]",                       "3", "42", "42", "42" },
        { "hl_data_column1",                 "hltable[[#Data];[Column1]]",               "3", "21", "21", "21" },
        { "hl_data_column2",                 "hltable[[#Data];[Column2]]",               "3", "42", "42", "42" },
        { "hl_this_row",                     "hltable[[#This Row]]",                     "2", "12", "48", "#VALUE!" },
        { "hl_this_row_column1",             "hltable[[#This Row];[Column1]]",           "1",  "4", "16", "#VALUE!" },
        { "hl_this_row_column2",             "hltable[[#This Row];[Column2]]",           "1",  "8", "32", "#VALUE!" },
        { "hl_this_row_range_column_1_to_2", "hltable[[#This Row];[Column1]:[Column2]]", "2", "12", "48", "#VALUE!" }
    };

    {
        // Insert named expressions.
        ScRangeName* pGlobalNames = m_pDoc->GetRangeName();
        CPPUNIT_ASSERT_MESSAGE("Failed to obtain global named expression object.", pGlobalNames);

        for (size_t i = 0, n = SAL_N_ELEMENTS(aHlNames); i < n; ++i)
        {
            // Choose base position that does not intersect with the database
            // range definition to test later use of [#This Row] results in
            // proper rows.
            ScRangeData* pName = new ScRangeData(
                    m_pDoc, OUString::createFromAscii(aHlNames[i].pName), OUString::createFromAscii(aHlNames[i].pExpr),
                    ScAddress(6,12,0), ScRangeData::Type::Name, formula::FormulaGrammar::GRAM_NATIVE);
            bool bInserted = pGlobalNames->insert(pName);
            CPPUNIT_ASSERT_MESSAGE(
                    OString("Failed to insert named expression "+ OString(aHlNames[i].pName) +".").getStr(), bInserted);
        }
    }

    // Use the named expressions in COUNTA() formulas, on row 10 that intersects.
    for (size_t i = 0, n = SAL_N_ELEMENTS(aHlNames); i < n; ++i)
    {
        OUString aFormula( "=COUNTA(" + OUString::createFromAscii( aHlNames[i].pName) + ")");
        ScAddress aPos(7+i,9,0);
        m_pDoc->SetString( aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( aPrefix + OUString::createFromAscii( aHlNames[i].pCounta),
                aPrefix + m_pDoc->GetString( aPos));
    }

    // Use the named expressions in SUM() formulas, on row 11 that intersects.
    for (size_t i = 0, n = SAL_N_ELEMENTS(aHlNames); i < n; ++i)
    {
        OUString aFormula( "=SUM(" + OUString::createFromAscii( aHlNames[i].pName) + ")");
        ScAddress aPos(7+i,10,0);
        m_pDoc->SetString( aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( aPrefix + OUString::createFromAscii( aHlNames[i].pSum3),
                aPrefix + m_pDoc->GetString( aPos));
    }

    // Use the named expressions in SUM() formulas, on row 12 that intersects.
    for (size_t i = 0, n = SAL_N_ELEMENTS(aHlNames); i < n; ++i)
    {
        OUString aFormula( "=SUM(" + OUString::createFromAscii( aHlNames[i].pName) + ")");
        ScAddress aPos(7+i,11,0);
        m_pDoc->SetString( aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( aPrefix + OUString::createFromAscii( aHlNames[i].pSum4),
                aPrefix + m_pDoc->GetString( aPos));
    }

    // Use the named expressions in SUM() formulas, on row 13 that does not intersect.
    for (size_t i = 0, n = SAL_N_ELEMENTS(aHlNames); i < n; ++i)
    {
        OUString aFormula( "=SUM(" + OUString::createFromAscii( aHlNames[i].pName) + ")");
        ScAddress aPos(7+i,12,0);
        m_pDoc->SetString( aPos, aFormula);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( aPrefix + OUString::createFromAscii( aHlNames[i].pSumX),
                aPrefix + m_pDoc->GetString( aPos));
    }

    // Insert a column at column F to extend database range from column E,F to
    // E,F,G. Use ScDocFunc so RefreshDirtyTableColumnNames() is called.
    rDocFunc.InsertCells(ScRange(5,0,0,5,MAXROW,0), &aMark, INS_INSCOLS_BEFORE, false, true);

    // Re-verify the named expression in SUM() formula, on row 12 that
    // intersects, now starting at column I, still works.
    m_pDoc->CalcAll();
    for (size_t i = 0, n = SAL_N_ELEMENTS(aHlNames); i < n; ++i)
    {
        OUString aFormula( "=SUM(" + OUString::createFromAscii( aHlNames[i].pName) + ")");
        ScAddress aPos(8+i,11,0);
        // For easier "debugability" have position and formula in assertion.
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( aPrefix + OUString::createFromAscii( aHlNames[i].pSum4),
                aPrefix + m_pDoc->GetString( aPos));
    }

    const char* pColumn3Formula = "=SUM(hltable[[#Data];[Column3]])";
    {
        // Populate "hltable" database range with data in newly inserted
        // column, F10:F12 plus a table formula in F14. The new header should
        // result in the internal table column name "Column3" that is used in
        // the formula.
        const char* aData[][1] = {
            { "64" },
            { "128" },
            { "256" },
            { "" },
            { pColumn3Formula }
        };
        ScAddress aPos(5,9,0);
        ScRange aRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
        CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);
    }

    // Verify the formula result in F14 (64+128+256=448).
    {
        OUString aFormula( OUString::createFromAscii( pColumn3Formula));
        ScAddress aPos(5,13,0);
        OUString aPrefix( aPos.Format(ScRefFlags::VALID) + " " + aFormula + " : ");
        CPPUNIT_ASSERT_EQUAL( OUString(aPrefix + "448"), OUString(aPrefix + m_pDoc->GetString(aPos)));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFuncFTEST()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "FTest");

    ScAddress aPos(6,0,0);
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
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 1.0000, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 0, 0, 6.0); // B1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.6222, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 1, 0, 8.0); // B2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.7732, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(4, 0, 0, 7.0); // E1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.8194, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(4, 1, 0, 4.0); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.9674, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 0, 0, 3.0); // C1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.3402, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(5, 0, 0, 28.0); // F1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0161, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 1, 0, 9.0); // C2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0063, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(5, 1, 0, 4.0); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0081, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(0, 2, 0, 2.0); // A3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0122, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(3, 2, 0, 8.0); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0178, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 2, 0, 4.0); // B3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0093, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(4, 2, 0, 7.0); // E3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0132, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(5, 2, 0, 5.0); // F3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0168, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 2, 0, 13.0); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0422, m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->SetString(0, 2, 0, "a"); // A3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0334, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetString(2, 0, 0, "b"); // C1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0261, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetString(5, 1, 0, "c"); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0219, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetString(4, 2, 0, "d"); // E3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0161, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetString(3, 2, 0, "e"); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.0110, m_pDoc->GetValue(aPos), 10e-4);

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
    const double fParameter[nNumParams] = { -5.0, -4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };

    // Results of var_test() from Octave
    const double fResults[nNumParams][nNumParams] = {
        { 0.9451191535603041,0.5429768686792684,0.213130093422756,0.06607644828558357,0.0169804365506927,0.003790723514148109,
          0.0007645345628801703,0.0001435746909905777,2.566562398786942e-05,4.436218417280813e-06,7.495090956766148e-07 },
        { 0.4360331979746912,0.9451191535603054,0.5429768686792684,0.2131300934227565,0.06607644828558357,0.0169804365506927,
          0.003790723514148109,0.0007645345628801703,0.0001435746909905777,2.566562398786942e-05,4.436218417280813e-06 },
        { 0.1309752286653509,0.4360331979746914,0.9451191535603058,0.5429768686792684,0.2131300934227565,0.06607644828558357,
          0.0169804365506927,0.003790723514148109,0.0007645345628801703,0.0001435746909905777,2.566562398786942e-05 },
        { 0.02453502500565108,0.1309752286653514,0.4360331979746914,0.9451191535603058,0.5429768686792689,0.2131300934227565,
          0.06607644828558357,0.0169804365506927,0.003790723514148109,0.0007645345628801703,0.0001435746909905777 },
        { 0.002886791075972228,0.02453502500565108,0.1309752286653514,0.4360331979746914,0.9451191535603041,0.5429768686792689,
          0.2131300934227565,0.06607644828558357,0.0169804365506927,0.003790723514148109,0.0007645345628801703 },
        { 0.0002237196492846927,0.002886791075972228,0.02453502500565108,0.1309752286653509,0.4360331979746912,0.9451191535603036,
          0.5429768686792689,0.2131300934227565,0.06607644828558357,0.0169804365506927,0.003790723514148109 },
        { 1.224926820153627e-05,0.0002237196492846927,0.002886791075972228,0.02453502500565108,0.1309752286653509,0.4360331979746914,
          0.9451191535603054,0.5429768686792684,0.2131300934227565,0.06607644828558357,0.0169804365506927 },
        { 5.109390206481379e-07,1.224926820153627e-05,0.0002237196492846927,0.002886791075972228,0.02453502500565108,
          0.1309752286653509,0.4360331979746914,0.9451191535603058,0.5429768686792684,0.213130093422756,0.06607644828558357 },
        { 1.739106880727093e-08,5.109390206481379e-07,1.224926820153627e-05,0.0002237196492846927,0.002886791075972228,
          0.02453502500565086,0.1309752286653509,0.4360331979746914,0.9451191535603041,0.5429768686792684,0.2131300934227565 },
        { 5.111255862999542e-10,1.739106880727093e-08,5.109390206481379e-07,1.224926820153627e-05,0.0002237196492846927,
          0.002886791075972228,0.02453502500565108,0.1309752286653516,0.4360331979746914,0.9451191535603058,0.5429768686792684 },
        { 1.354649725726631e-11,5.111255862999542e-10,1.739106880727093e-08,5.109390206481379e-07,1.224926820153627e-05,
          0.0002237196492846927,0.002886791075972228,0.02453502500565108,0.1309752286653509,0.4360331979746914,0.9451191535603054 }
    };

    m_pDoc->SetValue(3, 0, 0, fParameter[0]); // D1
    m_pDoc->SetValue(3, 1, 0, fParameter[0]); // D2
    aPos.Set(2,0,0); // C1
    m_pDoc->SetString(aPos, "=POWER(1.5;D1)" ); // C1
    aPos.Set(2, 1, 0);     // C2
    m_pDoc->SetString(aPos, "=POWER(1.5;D2)" ); // C2
    for ( SCROW nRow = 0; nRow < 5; ++nRow )    // Set A1:A5  = SQRT(C1*9/10), and A6:A10 = -SQRT(C1*9/10)
    {
        aPos.Set(0, nRow, 0);
        m_pDoc->SetString(aPos, "=SQRT(C1*9/10)");
        aPos.Set(0, nRow + 5, 0);
        m_pDoc->SetString(aPos, "=-SQRT(C1*9/10)");
    }

    for ( SCROW nRow = 0; nRow < 10; ++nRow )    // Set B1:B10  = SQRT(C2*19/20), and B11:B20 = -SQRT(C2*19/20)
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
    aPos.Set(4, 0, 0);  // E1

    for ( size_t nFirstIdx = 0; nFirstIdx < nNumParams; ++nFirstIdx )
    {
        m_pDoc->SetValue(3, 0, 0, fParameter[nFirstIdx]); // Set D1
        for ( size_t nSecondIdx = 0; nSecondIdx < nNumParams; ++nSecondIdx )
        {
            m_pDoc->SetValue(3, 1, 0, fParameter[nSecondIdx]); // Set D2
            double fExpected = fResults[nFirstIdx][nSecondIdx];
            // Here a dynamic error limit is used. This is to handle correctly when the expected value is lower than the fixed error limit of 10e-5
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", fExpected, m_pDoc->GetValue(aPos),    std::min(10e-5, fExpected*0.0001) );
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", fExpected, m_pDoc->GetValue(aPosRev), std::min(10e-5, fExpected*0.0001) );
        }
    }
    m_pDoc->DeleteTab(0);
}

void Test::testFuncFTESTBug()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "FTest");

    ScAddress aPos(9,0,0);
    m_pDoc->SetString(aPos, "=FTEST(H1:H3;I1:I3)");

    m_pDoc->SetValue(7, 0, 0, 9.0); // H1
    m_pDoc->SetValue(7, 1, 0, 8.0); // H2
    m_pDoc->SetValue(7, 2, 0, 6.0); // H3
    m_pDoc->SetValue(8, 0, 0, 5.0); // I1
    m_pDoc->SetValue(8, 1, 0, 7.0); // I2
    // tdf#93329
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of FTEST failed", 0.9046, m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncCHITEST()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "ChiTest");

    ScAddress aPos(6,0,0);
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return Err:502 for matrices with different size",
            OUString("Err:502"), aVal);

    m_pDoc->SetValue(3, 0, 0, 2.0); // D1
    m_pDoc->SetValue(3, 1, 0, 3.0); // D2
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return Err:502 for matrices with different size",
            OUString("Err:502"), aVal);
    m_pDoc->SetValue(4, 0, 0, 2.0); // E1
    m_pDoc->SetValue(4, 1, 0, 1.0); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.3613, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(4, 0, 0, 3.0); // E1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.2801, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 1, 0, 0.0); // B2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1410, m_pDoc->GetValue(aPos), 10e-4);

    // 3x3 matrices test
    m_pDoc->SetString(aPos, "=CHITEST(A1:C3;D1:F3)");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return Err:502 for matrices with empty",
            OUString("Err:502"), aVal);

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
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1117, m_pDoc->GetValue(aPos), 10e-4);

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
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1117, m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->SetValue(4, 1, 0, 5.0); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0215, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 2, 0, 1.0); // B3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0328, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(5, 0, 0, 3.0); // F1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1648, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(0, 1, 0, 3.0); // A2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1870, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(3, 1, 0, 5.0); // D2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1377, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(3, 2, 0, 4.0); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.1566, m_pDoc->GetValue(aPos), 10e-4);

    m_pDoc->SetValue(0, 0, 0, 0.0); // A1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0868, m_pDoc->GetValue(aPos), 10e-4);

    // no convergence error
    m_pDoc->SetValue(4, 0, 0, 0.0); // E1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL(OUString("Err:523"), aVal);
    m_pDoc->SetValue(4, 0, 0, 3.0); // E1

    // zero in all cells
    m_pDoc->SetValue(0, 1, 0, 0.0); // A2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0150, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(0, 2, 0, 0.0); // A3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0026, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(1, 0, 0, 0.0); // B1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.00079, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(1, 2, 0, 0.0); // B3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0005, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 0, 0, 0.0); // C1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of CHITEST failed", 0.0001, m_pDoc->GetValue(aPos), 10e-4);
    m_pDoc->SetValue(2, 1, 0, 0.0); // C2
    m_pDoc->SetValue(2, 2, 0, 0.0); // C3
    m_pDoc->SetValue(3, 0, 0, 0.0); // D1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return #VALUE! for matrices with empty",
            OUString("#VALUE!"), aVal);
    m_pDoc->SetValue(3, 1, 0, 0.0); // D2
    m_pDoc->SetValue(3, 2, 0, 0.0); // D3
    m_pDoc->SetValue(4, 0, 0, 0.0); // E1
    m_pDoc->SetValue(4, 1, 0, 0.0); // E2
    m_pDoc->SetValue(4, 2, 0, 0.0); // E3
    m_pDoc->SetValue(5, 0, 0, 0.0); // F1
    m_pDoc->SetValue(5, 1, 0, 0.0); // F2
    m_pDoc->SetValue(5, 2, 0, 0.0); // F3
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CHITEST should return #VALUE! for matrices with empty",
            OUString("#VALUE!"), aVal);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncTTEST()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "TTest");

    ScAddress aPos(6,0,0);
    // type 1, mode/tails 1
    m_pDoc->SetString(aPos, "=TTEST(A1:C3;D1:F3;1;1)");
    OUString aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TTEST should return #VALUE! for empty matrices",
            OUString("#VALUE!"), aVal);

    m_pDoc->SetValue(0, 0, 0, 8.0); // A1
    m_pDoc->SetValue(1, 0, 0, 2.0); // B1
    m_pDoc->SetValue(3, 0, 0, 3.0); // D1
    m_pDoc->SetValue(4, 0, 0, 1.0); // E1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.18717, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 0, 0, 1.0); // C1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.18717, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 0, 0, 6.0); // F1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.45958, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 1, 0, -4.0); // A2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.45958, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(3, 1, 0, 1.0); // D2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.35524, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(1, 1, 0, 5.0); // B2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.35524, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(4, 1, 0, -2.0); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.41043, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 1, 0, -1.0); // C2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.41043, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 1, 0, -3.0); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.34990, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 2, 0, 10.0); // A3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.34990, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(3, 2, 0, 10.0); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.34686, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(1, 2, 0, 3.0); // B3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.34686, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(4, 2, 0, 9.0); // E3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.47198, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 2, 0, -5.0); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.47198, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 2, 0, 6.0); // F3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.25529, m_pDoc->GetValue(aPos), 10e-5);

    m_pDoc->SetString(1, 1, 0, "a"); // B2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.12016, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetString(4, 1, 0, "b"); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.12016, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetString(2, 2, 0, "c"); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.25030, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetString(5, 1, 0, "d"); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.19637, m_pDoc->GetValue(aPos), 10e-5);

    // type 1, mode/tails 2
    m_pDoc->SetString(aPos, "=TTEST(A1:C3;D1:F3;2;1)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.39273, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(1, 1, 0, 4.0); // B2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.39273, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(4, 1, 0, 3.0); // E2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.43970, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 2, 0, -2.0); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.22217, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 1, 0, -10.0); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.64668, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 1, 0, 3.0); // A2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.95266, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(3, 2, 0, -1.0); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.62636, m_pDoc->GetValue(aPos), 10e-5);

    // type 2, mode/tails 2
    m_pDoc->SetString(aPos, "=TTEST(A1:C3;D1:F3;2;2)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.62549, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 1, 0, -1.0); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.94952, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 2, 0, 5.0); // C3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.58876, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(2, 1, 0, 2.0); // C2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.43205, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(3, 2, 0, -4.0); // D3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.36165, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 1, 0, 1.0); // A2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.44207, m_pDoc->GetValue(aPos), 10e-5);

    // type 3, mode/tails 1
    m_pDoc->SetString(aPos, "=TTEST(A1:C3;D1:F3;1;3)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.22132, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 0, 0, 1.0); // A1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.36977, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(0, 2, 0, -30.0); // A3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.16871, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(3, 1, 0, 5.0); // D2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.14396, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 1, 0, 2.0); // F2
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.12590, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(4, 2, 0, 2.0); // E3
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.16424, m_pDoc->GetValue(aPos), 10e-5);
    m_pDoc->SetValue(5, 0, 0, -1.0); // F1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of TTEST failed", 0.21472, m_pDoc->GetValue(aPos), 10e-5);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncSUMX2PY2()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "SumX2PY2 Test");

    OUString aVal;
    ScAddress aPos(6,0,0);
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUMX2PY2 needs two parameters",
            OUString("Err:511"), aVal);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncSUMX2MY2()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "SumX2MY2 Test");

    OUString aVal;
    ScAddress aPos(6,0,0);
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUMX2MY2 needs two parameters",
            OUString("Err:511"), aVal);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncGCD()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "GCDTest");

    OUString aVal;
    ScAddress aPos(4,0,0);

    m_pDoc->SetString(aPos, "=GCD(A1)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, 10.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 10.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, -2.0); // A1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return Err:502 for values less then 0",
            OUString("Err:502"), aVal);
    m_pDoc->SetString(0, 0, 0, "a"); // A1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return #VALUE! for a single string",
            OUString("#VALUE!"), aVal);

    m_pDoc->SetString(aPos, "=GCD(A1:B2)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, -12.0); // B1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return Err:502 for a matrix with values less then 0",
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return Err:502 for a array with values less then 0",
            OUString("Err:502"), aVal);
    m_pDoc->SetString(aPos, "=GCD({\"a\";6;9})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return Err:502 for a array with strings",
            OUString("Err:502"), aVal);

    // inline list of values
    m_pDoc->SetString(aPos, "=GCD(12;24;36;48;60)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 12.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=GCD(0;12;24;36;48;60)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 12.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=GCD(\"a\";1)");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GCD should return #VALUE! for a array with strings",
            OUString("#VALUE!"), aVal);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncLCM()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "LCMTest");

    OUString aVal;
    ScAddress aPos(4,0,0);

    m_pDoc->SetString(aPos, "=LCM(A1)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, 10.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 10.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, -2.0); // A1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return Err:502 for values less then 0",
            OUString("Err:502"), aVal);
    m_pDoc->SetString(0, 0, 0, "a"); // A1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return #VALUE! for a single string",
            OUString("#VALUE!"), aVal);

    m_pDoc->SetString(aPos, "=LCM(A1:B2)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of GCD for failed", 1.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 1, 0, -12.0); // B1
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return Err:502 for a matrix with values less then 0",
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 1831500.0, m_pDoc->GetValue(aPos));

    // with floor
    m_pDoc->SetValue(1, 0, 0, 99.89); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 1831500.0, m_pDoc->GetValue(aPos));
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return Err:502 for a array with values less then 0",
            OUString("Err:502"), aVal);
    m_pDoc->SetString(aPos, "=LCM({\"a\";6;9})");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return Err:502 for a array with strings",
            OUString("Err:502"), aVal);

    m_pDoc->SetString(aPos, "=LCM(12;24;36;48;60)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 720.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=LCM(0;12;24;36;48;60)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of LCM for failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, "=LCM(\"a\";1)");
    aVal = m_pDoc->GetString(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LCM should return #VALUE! for a array with strings",
            OUString("#VALUE!"), aVal);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncSUMSQ()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "SUMSQTest");

    ScAddress aPos(4,0,0);

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 30.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 2, 0, 0.0); // B3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 30.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 2, 0, 6.0); // A3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 66.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 0, 0, -5.0); // C1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 91.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 1, 0, 3.0); // C2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 100.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(2, 2, 0, 2.0); // C3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of SUMSQ with a string for failed", 104.0, m_pDoc->GetValue(aPos));

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUMSQ should return #VALUE! for a array with strings",
            OUString("#VALUE!"), aVal);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncMDETERM()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "MDETERM_test");
    ScAddress aPos(8,0,0);
    OUString aColCodes("ABCDEFGH");
    OUString aFormulaTemplate("=MDETERM(A1:B2)");
    OUStringBuffer aFormulaBuffer(aFormulaTemplate);
    for( SCSIZE nSize = 3; nSize <= 8; nSize++ )
    {
        double fVal = 1.0;
        // Generate a singular integer matrix
        for( SCROW nRow = 0; nRow < static_cast<SCROW>(nSize); nRow++ )
        {
            for( SCCOL nCol = 0; nCol < static_cast<SCCOL>(nSize); nCol++ )
            {
                m_pDoc->SetValue(nCol, nRow, 0, fVal);
                fVal += 1.0;
            }
        }
        aFormulaBuffer[12] = aColCodes[nSize-1];
        aFormulaBuffer[13] = static_cast<sal_Unicode>( '0' + nSize );
        m_pDoc->SetString(aPos, aFormulaBuffer.toString());

#if SAL_TYPES_SIZEOFPOINTER == 4
        // On crappy 32-bit targets, presumably without extended precision on
        // interim results or optimization not catching it, this test fails
        // when comparing to 0.0, so have a narrow error margin. See also
        // commit message of 8140309d636d4a870875f2dd75ed3dfff2c0fbaf
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of MDETERM incorrect for singular integer matrix",
                0.0, m_pDoc->GetValue(aPos), 1e-12);
#else
        // Even on one (and only one) x86_64 target the result was
        // 6.34413156928661e-17 instead of 0.0 (tdf#99730) so lower the bar to
        // 10e-14.
        // Then again on aarch64, ppc64* and s390x it also fails.
        // Sigh.. why do we even test this? The original complaint in tdf#32834
        // was about -9.51712667007776E-016
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of MDETERM incorrect for singular integer matrix",
                0.0, m_pDoc->GetValue(aPos), 1e-14);
#endif
    }

    int aVals[] = {23, 31, 13, 12, 34, 64, 34, 31, 98, 32, 33, 63, 45, 54, 65, 76};
    int nIdx = 0;
    for( SCROW nRow = 0; nRow < 4; nRow++ )
        for( SCCOL nCol = 0; nCol < 4; nCol++ )
            m_pDoc->SetValue(nCol, nRow, 0, static_cast<double>(aVals[nIdx++]));
    m_pDoc->SetString(aPos, "=MDETERM(A1:D4)");
    // Following test is conservative in the sense that on Linux x86_64 the error is less that 1.0E-9
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of MDETERM incorrect for non-singular integer matrix",
                                         -180655.0, m_pDoc->GetValue(aPos), 1.0E-6);
    m_pDoc->DeleteTab(0);
}

void Test::testFormulaErrorPropagation()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Sheet1");

    ScMarkData aMark;
    aMark.SelectOneTable(0);
    ScAddress aPos, aPos2;
    const OUString aTRUE("TRUE");
    const OUString aFALSE("FALSE");

    aPos.Set(0,0,0);// A1
    m_pDoc->SetValue( aPos, 1.0);
    aPos.IncCol();  // B1
    m_pDoc->SetValue( aPos, 2.0);
    aPos.IncCol();

    aPos.IncRow();  // C2
    m_pDoc->SetString( aPos, "=ISERROR(A1:B1+3)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE, m_pDoc->GetString(aPos));

    aPos.IncRow();  // C3
    m_pDoc->SetString( aPos, "=ISERROR(A1:B1+{3})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE, m_pDoc->GetString(aPos));
    aPos.IncRow();  // C4
    aPos2 = aPos;
    aPos2.IncCol(); // D4
    m_pDoc->InsertMatrixFormula(aPos.Col(), aPos.Row(), aPos2.Col(), aPos2.Row(), aMark, "=ISERROR(A1:B1+{3})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE, m_pDoc->GetString(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos2.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE, m_pDoc->GetString(aPos2));

    aPos.IncRow();  // C5
    m_pDoc->SetString( aPos, "=ISERROR({1;\"x\"}+{3;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE, m_pDoc->GetString(aPos));
    aPos.IncRow();  // C6
    aPos2 = aPos;
    aPos2.IncCol(); // D6
    m_pDoc->InsertMatrixFormula(aPos.Col(), aPos.Row(), aPos2.Col(), aPos2.Row(), aMark, "=ISERROR({1;\"x\"}+{3;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE, m_pDoc->GetString(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos2.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE, m_pDoc->GetString(aPos2));

    aPos.IncRow();  // C7
    m_pDoc->SetString( aPos, "=ISERROR({\"x\";2}+{3;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE, m_pDoc->GetString(aPos));
    aPos.IncRow();  // C8
    aPos2 = aPos;
    aPos2.IncCol(); // D8
    m_pDoc->InsertMatrixFormula(aPos.Col(), aPos.Row(), aPos2.Col(), aPos2.Row(), aMark, "=ISERROR({\"x\";2}+{3;4})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE, m_pDoc->GetString(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos2.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE, m_pDoc->GetString(aPos2));

    aPos.IncRow();  // C9
    m_pDoc->SetString( aPos, "=ISERROR(({1;\"x\"}+{3;4})-{5;6})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE, m_pDoc->GetString(aPos));
    aPos.IncRow();  // C10
    aPos2 = aPos;
    aPos2.IncCol(); // D10
    m_pDoc->InsertMatrixFormula(aPos.Col(), aPos.Row(), aPos2.Col(), aPos2.Row(), aMark, "=ISERROR(({1;\"x\"}+{3;4})-{5;6})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE, m_pDoc->GetString(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos2.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE, m_pDoc->GetString(aPos2));

    aPos.IncRow();  // C11
    m_pDoc->SetString( aPos, "=ISERROR(({\"x\";2}+{3;4})-{5;6})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE, m_pDoc->GetString(aPos));
    aPos.IncRow();  // C12
    aPos2 = aPos;
    aPos2.IncCol(); // D12
    m_pDoc->InsertMatrixFormula(aPos.Col(), aPos.Row(), aPos2.Col(), aPos2.Row(), aMark, "=ISERROR(({\"x\";2}+{3;4})-{5;6})");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos.Format(ScRefFlags::VALID).toUtf8().getStr(), aTRUE, m_pDoc->GetString(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aPos2.Format(ScRefFlags::VALID).toUtf8().getStr(), aFALSE, m_pDoc->GetString(aPos2));

    m_pDoc->DeleteTab(0);
}

namespace {

class ColumnTest
{
    ScDocument * m_pDoc;

    const SCROW m_nTotalRows;
    const SCROW m_nStart1;
    const SCROW m_nEnd1;
    const SCROW m_nStart2;
    const SCROW m_nEnd2;

public:
    ColumnTest( ScDocument * pDoc, SCROW nTotalRows,
                SCROW nStart1, SCROW nEnd1, SCROW nStart2, SCROW nEnd2 )
        : m_pDoc(pDoc), m_nTotalRows(nTotalRows)
        , m_nStart1(nStart1), m_nEnd1(nEnd1)
        , m_nStart2(nStart2), m_nEnd2(nEnd2)
    {}

    void operator() ( SCCOL nColumn, const OUString& rFormula,
                      std::function<double(SCROW )> lExpected ) const
    {
        ScDocument aClipDoc(SCDOCMODE_CLIP);
        ScMarkData aMark;

        ScAddress aPos(nColumn, m_nStart1, 0);
        m_pDoc->SetString(aPos, rFormula);
        ASSERT_DOUBLES_EQUAL( lExpected(m_nStart1), m_pDoc->GetValue(aPos) );

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
        for( SCROW i = 0; i < m_nTotalRows; ++i )
        {
            if( !((m_nStart1 <= i && i <= m_nEnd1) || (m_nStart2 <= i && i <= m_nEnd2)) )
                continue;
            double fExpected = lExpected(i);
            ASSERT_DOUBLES_EQUAL(fExpected, m_pDoc->GetValue(ScAddress(nColumn,i,0)));
        }
    }
};

}

void Test::testTdf97369()
{
    const SCROW TOTAL_ROWS = 330;
    const SCROW ROW_RANGE = 10;
    const SCROW START1 = 9;
    const SCROW END1 = 159;
    const SCROW START2 = 169;
    const SCROW END2 = 319;

    const double SHIFT1 = 200;
    const double SHIFT2 = 400;

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, "tdf97369"));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    // set up columns A, B, C
    for( SCROW i = 0; i < TOTAL_ROWS; ++i )
    {
        m_pDoc->SetValue(ScAddress(0, i, 0), i);          // A
        m_pDoc->SetValue(ScAddress(1, i, 0), i + SHIFT1); // B
        m_pDoc->SetValue(ScAddress(2, i, 0), i + SHIFT2); // C
    }

    const ColumnTest columnTest( m_pDoc, TOTAL_ROWS, START1, END1, START2, END2 );

    auto lExpectedinD = [=] (SCROW n) {
        return 3.0 * (n-START1) + SHIFT1 + SHIFT2;
    };
    columnTest(3, "=SUM(A1:C1)", lExpectedinD);

    auto lExpectedinE = [=] (SCROW ) {
        return SHIFT1 + SHIFT2;
    };
    columnTest(4, "=SUM(A$1:C$1)", lExpectedinE);

    auto lExpectedinF = [=] (SCROW n) {
        return ((2*n + 1 - ROW_RANGE) * ROW_RANGE) / 2.0;
    };
    columnTest(5, "=SUM(A1:A10)", lExpectedinF);

    auto lExpectedinG = [=] (SCROW n) {
        return ((n + 1) * n) / 2.0;
    };
    columnTest(6, "=SUM(A$1:A10)", lExpectedinG);

    auto lExpectedinH = [=] (SCROW n) {
        return 3.0 * (((2*n + 1 - ROW_RANGE) * ROW_RANGE) / 2) + ROW_RANGE * (SHIFT1 + SHIFT2);
    };
    columnTest(7, "=SUM(A1:C10)", lExpectedinH);

    auto lExpectedinI = [=] (SCROW ) {
        return 3.0 * (((2*START1 + 1 - ROW_RANGE) * ROW_RANGE) / 2) + ROW_RANGE * (SHIFT1 + SHIFT2);
    };
    columnTest(8, "=SUM(A$1:C$10)", lExpectedinI);

    m_pDoc->DeleteTab(0);
}

void Test::testTdf97587()
{
    const SCROW TOTAL_ROWS = 150;
    const SCROW ROW_RANGE = 10;

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, "tdf97587"));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    std::set<SCROW> emptyCells = {0, 100};
    for( SCROW i = 0; i < ROW_RANGE; ++i )
    {
        emptyCells.insert(i + TOTAL_ROWS / 3);
        emptyCells.insert(i + TOTAL_ROWS);
    }

    // set up columns A
    for( SCROW i = 0; i < TOTAL_ROWS; ++i )
    {
        if( emptyCells.find(i) != emptyCells.end() )
            continue;
        m_pDoc->SetValue(ScAddress(0, i, 0), 1.0);
    }

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScMarkData aMark;

    ScAddress aPos(1, 0, 0);
    m_pDoc->SetString(aPos, "=SUM(A1:A10)");

    // Copy formula cell to clipboard.
    ScClipParam aClipParam(aPos, false);
    aMark.SetMarkArea(aPos);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    // Paste it to first range.
    InsertDeleteFlags nFlags = InsertDeleteFlags::CONTENTS;
    ScRange aDestRange(1, 1, 0, 1, TOTAL_ROWS + ROW_RANGE, 0);
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, nFlags, nullptr, &aClipDoc);

    // Check the formula results in column B.
    for( SCROW i = 0; i < TOTAL_ROWS + 1; ++i )
    {
        int k = std::count_if( emptyCells.begin(), emptyCells.end(),
                [=](SCROW n) { return (i <= n && n < i + ROW_RANGE); } );
        double fExpected = ROW_RANGE - k;
        ASSERT_DOUBLES_EQUAL(fExpected, m_pDoc->GetValue(ScAddress(1,i,0)));
    }
    m_pDoc->DeleteTab(0);
}

void Test::testMatConcat()
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab (0, "Test"));

    for (SCCOL nCol = 0; nCol < 10; ++nCol)
    {
        for (SCROW nRow = 0; nRow < 10; ++nRow)
        {
            m_pDoc->SetValue(ScAddress(nCol, nRow, 0), nCol*nRow);
        }
    }

    ScMarkData aMark;
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 12, 9, 21, aMark, "=A1:J10&A1:J10");

    for (SCCOL nCol = 0; nCol < 10; ++nCol)
    {
        for (SCROW nRow = 12; nRow < 22; ++nRow)
        {
            OUString aStr = m_pDoc->GetString(ScAddress(nCol, nRow, 0));
            CPPUNIT_ASSERT_EQUAL(OUString(OUString::number(nCol * (nRow - 12)) + OUString::number(nCol * (nRow - 12))), aStr);
        }
    }
    m_pDoc->DeleteTab(0);
}

void Test::testMatConcatReplication()
{
    // if one of the matrices is an one column or row matrix
    // the matrix is replicated across the larger matrix
    CPPUNIT_ASSERT(m_pDoc->InsertTab (0, "Test"));

    for (SCCOL nCol = 0; nCol < 10; ++nCol)
    {
        for (SCROW nRow = 0; nRow < 10; ++nRow)
        {
            m_pDoc->SetValue(ScAddress(nCol, nRow, 0), nCol*nRow);
        }
    }

    ScMarkData aMark;
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

void Test::testRefR1C1WholeCol()
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab (0, "Test"));

    ScAddress aPos(1, 1, 1);
    ScCompiler aComp(m_pDoc, aPos);
    aComp.SetGrammar(FormulaGrammar::GRAM_ENGLISH_XL_R1C1);
    std::unique_ptr<ScTokenArray> pTokens(aComp.CompileString("=C[10]"));
    sc::TokenStringContext aCxt(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH);
    OUString aFormula = pTokens->CreateString(aCxt, aPos);

    CPPUNIT_ASSERT_EQUAL(OUString("L:L"), aFormula);

    m_pDoc->DeleteTab(0);
}

void Test::testRefR1C1WholeRow()
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab (0, "Test"));

    ScAddress aPos(1, 1, 1);
    ScCompiler aComp(m_pDoc, aPos);
    aComp.SetGrammar(FormulaGrammar::GRAM_ENGLISH_XL_R1C1);
    std::unique_ptr<ScTokenArray> pTokens(aComp.CompileString("=R[3]"));
    sc::TokenStringContext aCxt(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH);
    OUString aFormula = pTokens->CreateString(aCxt, aPos);

    CPPUNIT_ASSERT_EQUAL(OUString("5:5"), aFormula);

    m_pDoc->DeleteTab(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
