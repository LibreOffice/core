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
#include <validat.hxx>
#include <scitems.hxx>
#include <patattr.hxx>
#include <docpool.hxx>

#include <formula/vectortoken.hxx>
#include <svl/broadcast.hxx>

#include <boost/scoped_ptr.hpp>

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
            ScAddress(0,0,0), RT_NAME, formula::FormulaGrammar::GRAM_NATIVE);

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

    boost::scoped_ptr<ScTokenArray> pArray;

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
            if (!checkFormula(*m_pDoc, ScAddress(0,0,0), aChecks[i]))
                CPPUNIT_FAIL("Wrong formula");
        }
    }

    ScAddress aPos;
    ScAddress::ExternalInfo aExtInfo;
    sal_uInt16 nRes = aPos.Parse("'90''s Music'.D10", m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & SCA_VALID) != 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(3), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

    nRes = aPos.Parse("'90''s and 70''s'.C100", m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & SCA_VALID) != 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(2), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(99), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

    nRes = aPos.Parse("'All Others'.B3", m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & SCA_VALID) != 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(3), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

    nRes = aPos.Parse("NoQuote.E13", m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & SCA_VALID) != 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(4), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(4), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

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
        const char* pFormula; ScFormulaVectorState eState;
    } aVectorTests[] = {
        { "=SUM(1;2;3;4;5)", FormulaVectorEnabled },
        { "=NOW()", FormulaVectorDisabled },
        { "=AVERAGE(X1:Y200)", FormulaVectorCheckReference },
        { "=MAX(X1:Y200;10;20)", FormulaVectorCheckReference },
        { "=MIN(10;11;22)", FormulaVectorEnabled },
        { "=H4", FormulaVectorCheckReference },
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aVectorTests); ++i)
    {
        m_pDoc->SetString(aPos1, OUString::createFromAscii(aVectorTests[i].pFormula));
        ScFormulaVectorState eState = m_pDoc->GetFormulaVectorState(aPos1);

        if (eState != aVectorTests[i].eState)
        {
            std::ostringstream os;
            os << "Unexpected vectorization state: expr:" << aVectorTests[i].pFormula;
            CPPUNIT_ASSERT_MESSAGE(os.str().c_str(), false);
        }
        aPos1.IncRow();
    }

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
    CPPUNIT_ASSERT_MESSAGE("Wrong start position of extended range.", aTest.aStart == ScAddress(2,2,0));
    CPPUNIT_ASSERT_MESSAGE("Wrong end position of extended range.", aTest.aEnd == ScAddress(6,5,0));

    ScComplexRefData aDoubleRef2;
    aDoubleRef2.InitRangeRel(ScRange(1,2,0,8,6,0), ScAddress(5,5,0));
    aDoubleRef.Extend(aDoubleRef2, ScAddress(5,5,0));
    aTest = aDoubleRef.toAbs(ScAddress(5,5,0));

    CPPUNIT_ASSERT_MESSAGE("Wrong start position of extended range.", aTest.aStart == ScAddress(1,2,0));
    CPPUNIT_ASSERT_MESSAGE("Wrong end position of extended range.", aTest.aEnd == ScAddress(8,6,0));
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
        boost::scoped_ptr<ScTokenArray> pArray;
        {
            pArray.reset(compileFormula(m_pDoc, OUString::createFromAscii(aTests[i].pInput), NULL, aTests[i].eInputGram));
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
        boost::scoped_ptr<ScTokenArray> pCode(compileFormula(m_pDoc, aInput));
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
        CPPUNIT_ASSERT_MESSAGE("Wrong RPN token count.", nLen == SAL_N_ELEMENTS(aCheckRPN));

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
        CPPUNIT_ASSERT_MESSAGE("Wrong RPN token count.", nLen == SAL_N_ELEMENTS(aCheckRPN2));
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
    if (!checkFormula(*m_pDoc, aPos, "A1"))
        CPPUNIT_FAIL("Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    if (!checkFormula(*m_pDoc, aPos, "$A$1"))
        CPPUNIT_FAIL("Wrong formula in C4.");

    // Delete row 2 to push formula cells up (to C2:C3).
    m_pDoc->DeleteRow(ScRange(0,1,0,MAXCOL,1,0));

    aPos = ScAddress(2,1,0);
    if (!checkFormula(*m_pDoc, aPos, "A1"))
        CPPUNIT_FAIL("Wrong formula in C2.");

    aPos = ScAddress(2,2,0);
    if (!checkFormula(*m_pDoc, aPos, "$A$1"))
        CPPUNIT_FAIL("Wrong formula in C3.");

    // Insert one row at row 2 to move them back.
    m_pDoc->InsertRow(ScRange(0,1,0,MAXCOL,1,0));

    aPos = ScAddress(2,2,0);
    if (!checkFormula(*m_pDoc, aPos, "A1"))
        CPPUNIT_FAIL("Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    if (!checkFormula(*m_pDoc, aPos, "$A$1"))
        CPPUNIT_FAIL("Wrong formula in C4.");

    // Insert 2 rows at row 1 to shift all of A1 and C3:C4 down.
    m_pDoc->InsertRow(ScRange(0,0,0,MAXCOL,1,0));

    aPos = ScAddress(2,4,0);
    if (!checkFormula(*m_pDoc, aPos, "A3"))
        CPPUNIT_FAIL("Wrong formula in C5.");

    aPos = ScAddress(2,5,0);
    if (!checkFormula(*m_pDoc, aPos, "$A$3"))
        CPPUNIT_FAIL("Wrong formula in C6.");

    // Delete 2 rows at row 1 to shift them back.
    m_pDoc->DeleteRow(ScRange(0,0,0,MAXCOL,1,0));

    aPos = ScAddress(2,2,0);
    if (!checkFormula(*m_pDoc, aPos, "A1"))
        CPPUNIT_FAIL("Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    if (!checkFormula(*m_pDoc, aPos, "$A$1"))
        CPPUNIT_FAIL("Wrong formula in C4.");

    // Insert 3 columns at column B. to shift C3:C4 to F3:F4.
    m_pDoc->InsertCol(ScRange(1,0,0,3,MAXROW,0));

    aPos = ScAddress(5,2,0);
    if (!checkFormula(*m_pDoc, aPos, "A1"))
        CPPUNIT_FAIL("Wrong formula in F3.");

    aPos = ScAddress(5,3,0);
    if (!checkFormula(*m_pDoc, aPos, "$A$1"))
        CPPUNIT_FAIL("Wrong formula in F4.");

    // Delete columns B:D to shift them back.
    m_pDoc->DeleteCol(ScRange(1,0,0,3,MAXROW,0));

    aPos = ScAddress(2,2,0);
    if (!checkFormula(*m_pDoc, aPos, "A1"))
        CPPUNIT_FAIL("Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    if (!checkFormula(*m_pDoc, aPos, "$A$1"))
        CPPUNIT_FAIL("Wrong formula in C4.");

    // Insert cells over A1:A3 to only shift A1 down to A4.
    m_pDoc->InsertRow(ScRange(0,0,0,0,2,0));

    aPos = ScAddress(2,2,0);
    if (!checkFormula(*m_pDoc, aPos, "A4"))
        CPPUNIT_FAIL("Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    if (!checkFormula(*m_pDoc, aPos, "$A$4"))
        CPPUNIT_FAIL("Wrong formula in C4.");

    // .. and back.
    m_pDoc->DeleteRow(ScRange(0,0,0,0,2,0));

    aPos = ScAddress(2,2,0);
    if (!checkFormula(*m_pDoc, aPos, "A1"))
        CPPUNIT_FAIL("Wrong formula in C3.");

    aPos = ScAddress(2,3,0);
    if (!checkFormula(*m_pDoc, aPos, "$A$1"))
        CPPUNIT_FAIL("Wrong formula in C4.");

    // Delete row 1 which will delete the value cell (A1).
    m_pDoc->DeleteRow(ScRange(0,0,0,MAXCOL,0,0));

    aPos = ScAddress(2,1,0);
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(ScErrorCodes::errNoRef, pFC->GetErrCode());
    aPos = ScAddress(2,2,0);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(ScErrorCodes::errNoRef, pFC->GetErrCode());

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
    if (!checkFormula(*m_pDoc, aPos, "SUM(B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    aPos = ScAddress(0,6,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM($B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in A7.");

    // Insert a row at row 1.
    m_pDoc->InsertRow(ScRange(0,0,0,MAXCOL,0,0));

    aPos = ScAddress(0,6,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM(B3:C4)"))
        CPPUNIT_FAIL("Wrong formula in A7.");

    aPos = ScAddress(0,7,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM($B$3:$C$4)"))
        CPPUNIT_FAIL("Wrong formula in A8.");

    // ... and back.
    m_pDoc->DeleteRow(ScRange(0,0,0,MAXCOL,0,0));

    aPos = ScAddress(0,5,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM(B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    aPos = ScAddress(0,6,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM($B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in A7.");

    // Insert columns B:C to shift only the value range.
    m_pDoc->InsertCol(ScRange(1,0,0,2,MAXROW,0));

    aPos = ScAddress(0,5,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM(D2:E3)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    aPos = ScAddress(0,6,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM($D$2:$E$3)"))
        CPPUNIT_FAIL("Wrong formula in A7.");

    // ... and back.
    m_pDoc->DeleteCol(ScRange(1,0,0,2,MAXROW,0));

    aPos = ScAddress(0,5,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM(B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    aPos = ScAddress(0,6,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM($B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in A7.");

    // Insert rows 5:6 to shift the formula cells only.
    m_pDoc->InsertRow(ScRange(0,4,0,MAXCOL,5,0));

    aPos = ScAddress(0,7,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM(B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in A8.");

    aPos = ScAddress(0,8,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM($B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in A9.");

    // ... and back.
    m_pDoc->DeleteRow(ScRange(0,4,0,MAXCOL,5,0));

    aPos = ScAddress(0,5,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM(B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    aPos = ScAddress(0,6,0);
    if (!checkFormula(*m_pDoc, aPos, "SUM($B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in A7.");

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
    CPPUNIT_ASSERT_EQUAL(ScErrorCodes::errNoRef, pFC->GetErrCode());

    pFC = m_pDoc->GetFormulaCell(ScAddress(0,4,0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(ScErrorCodes::errNoRef, pFC->GetErrCode());

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

    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "SUM(B2:C5)"))
        CPPUNIT_FAIL("Wrong formula in A7.");

    if (!checkFormula(*m_pDoc, ScAddress(0,7,0), "SUM($B$2:$C$5)"))
        CPPUNIT_FAIL("Wrong formula in A8.");

    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,7,0)));

    // Delete row 3. This should shrink the range references by one row.
    m_pDoc->DeleteRow(ScRange(0,2,0,MAXCOL,2,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "SUM(B2:C4)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "SUM($B$2:$C$4)"))
        CPPUNIT_FAIL("Wrong formula in A7.");

    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,6,0)));

    // Delete row 4 - bottom of range
    m_pDoc->DeleteRow(ScRange(0,3,0,MAXCOL,3,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,4,0), "SUM(B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in A5.");

    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "SUM($B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Delete row 2 - top of range
    m_pDoc->DeleteRow(ScRange(0,1,0,MAXCOL,1,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,3,0), "SUM(B2:C2)"))
        CPPUNIT_FAIL("Wrong formula in A4.");

    if (!checkFormula(*m_pDoc, ScAddress(0,4,0), "SUM($B$2:$C$2)"))
        CPPUNIT_FAIL("Wrong formula in A5.");

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

    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "SUM(C2:F3)"))
        CPPUNIT_FAIL("Wrong formula in A2.");

    if (!checkFormula(*m_pDoc, ScAddress(0,2,0), "SUM($C$2:$F$3)"))
        CPPUNIT_FAIL("Wrong formula in A3.");

    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Delete column D.
    m_pDoc->DeleteCol(ScRange(3,0,0,3,MAXROW,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "SUM(C2:E3)"))
        CPPUNIT_FAIL("Wrong formula in A2.");

    if (!checkFormula(*m_pDoc, ScAddress(0,2,0), "SUM($C$2:$E$3)"))
        CPPUNIT_FAIL("Wrong formula in A3.");

    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Delete column E - the right edge of reference range.
    m_pDoc->DeleteCol(ScRange(4,0,0,4,MAXROW,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "SUM(C2:D3)"))
        CPPUNIT_FAIL("Wrong formula in A2.");

    if (!checkFormula(*m_pDoc, ScAddress(0,2,0), "SUM($C$2:$D$3)"))
        CPPUNIT_FAIL("Wrong formula in A3.");

    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Delete column C - the left edge of reference range.
    m_pDoc->DeleteCol(ScRange(2,0,0,2,MAXROW,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "SUM(C2:C3)"))
        CPPUNIT_FAIL("Wrong formula in A2.");

    if (!checkFormula(*m_pDoc, ScAddress(0,2,0), "SUM($C$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in A3.");

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

    if (!checkFormula(*m_pDoc, ScAddress(0,4,0), "SUM(C2:D3)"))
        CPPUNIT_FAIL("Wrong formula in A5.");

    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "SUM($C$2:$D$3)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    // Insert a column at column C. This should simply shift the reference without expansion.
    m_pDoc->InsertCol(ScRange(2,0,0,2,MAXROW,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,4,0), "SUM(D2:E3)"))
        CPPUNIT_FAIL("Wrong formula in A5.");

    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "SUM($D$2:$E$3)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    // Shift it back.
    m_pDoc->DeleteCol(ScRange(2,0,0,2,MAXROW,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,4,0), "SUM(C2:D3)"))
        CPPUNIT_FAIL("Wrong formula in A5.");

    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "SUM($C$2:$D$3)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    // Insert at column D. This should expand the reference by one column length.
    m_pDoc->InsertCol(ScRange(3,0,0,3,MAXROW,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,4,0), "SUM(C2:E3)"))
        CPPUNIT_FAIL("Wrong formula in A5.");

    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "SUM($C$2:$E$3)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    // Insert at column F. No expansion should occur since the edge expansion is turned off.
    m_pDoc->InsertCol(ScRange(5,0,0,5,MAXROW,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,4,0), "SUM(C2:E3)"))
        CPPUNIT_FAIL("Wrong formula in A5.");

    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "SUM($C$2:$E$3)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    // Insert at row 2. No expansion should occur with edge expansion turned off.
    m_pDoc->InsertRow(ScRange(0,1,0,MAXCOL,1,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "SUM(C3:E4)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "SUM($C$3:$E$4)"))
        CPPUNIT_FAIL("Wrong formula in A7.");

    // Insert at row 4 to expand the reference range.
    m_pDoc->InsertRow(ScRange(0,3,0,MAXCOL,3,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "SUM(C3:E5)"))
        CPPUNIT_FAIL("Wrong formula in A7.");

    if (!checkFormula(*m_pDoc, ScAddress(0,7,0), "SUM($C$3:$E$5)"))
        CPPUNIT_FAIL("Wrong formula in A8.");

    // Insert at row 6. No expansion with edge expansion turned off.
    m_pDoc->InsertRow(ScRange(0,5,0,MAXCOL,5,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,7,0), "SUM(C3:E5)"))
        CPPUNIT_FAIL("Wrong formula in A8.");

    if (!checkFormula(*m_pDoc, ScAddress(0,8,0), "SUM($C$3:$E$5)"))
        CPPUNIT_FAIL("Wrong formula in A9.");

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

    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "SUM(C6:D7)"))
        CPPUNIT_FAIL("Wrong formula in A2.");

    if (!checkFormula(*m_pDoc, ScAddress(0,2,0), "SUM($C$6:$D$7)"))
        CPPUNIT_FAIL("Wrong formula in A3.");

    // Insert at column E. This should expand the reference range by one column.
    m_pDoc->InsertCol(ScRange(4,0,0,4,MAXROW,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "SUM(C6:E7)"))
        CPPUNIT_FAIL("Wrong formula in A2.");

    if (!checkFormula(*m_pDoc, ScAddress(0,2,0), "SUM($C$6:$E$7)"))
        CPPUNIT_FAIL("Wrong formula in A3.");

    // Insert at column C to edge-expand the reference range.
    m_pDoc->InsertCol(ScRange(2,0,0,2,MAXROW,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "SUM(C6:F7)"))
        CPPUNIT_FAIL("Wrong formula in A2.");

    if (!checkFormula(*m_pDoc, ScAddress(0,2,0), "SUM($C$6:$F$7)"))
        CPPUNIT_FAIL("Wrong formula in A3.");

    // Insert at row 8 to edge-expand.
    m_pDoc->InsertRow(ScRange(0,7,0,MAXCOL,7,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "SUM(C6:F8)"))
        CPPUNIT_FAIL("Wrong formula in A2.");

    if (!checkFormula(*m_pDoc, ScAddress(0,2,0), "SUM($C$6:$F$8)"))
        CPPUNIT_FAIL("Wrong formula in A3.");

    // Insert at row 6 to edge-expand.
    m_pDoc->InsertRow(ScRange(0,5,0,MAXCOL,5,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "SUM(C6:F9)"))
        CPPUNIT_FAIL("Wrong formula in A2.");

    if (!checkFormula(*m_pDoc, ScAddress(0,2,0), "SUM($C$6:$F$9)"))
        CPPUNIT_FAIL("Wrong formula in A3.");

    m_pDoc->DeleteTab(0);
}

void Test::testErrorOnExternalReferences()
{
    // Test tdf#89330
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->SetString(ScAddress(0,0,0), "='file:///Path/To/FileA.ods'#$Sheet1.A1A");

    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(ScErrorCodes::errNoName, pFC->GetErrCode());

    if (!checkFormula(*m_pDoc, ScAddress(0,0,0), "'file:///Path/To/FileA.ods'#$Sheet1.A1A"))
        CPPUNIT_FAIL("Formula changed");

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

    if (!checkFormula(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

    // Swap the sheets.
    m_pDoc->MoveTab(0, 1);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);
    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aName);

    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "SUM(Sheet1.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "SUM($Sheet1.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

    // Swap back.
    m_pDoc->MoveTab(0, 1);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aName);
    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);

    if (!checkFormula(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

    // Insert a new sheet between the two.
    m_pDoc->InsertTab(1, "Temp");

    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Temp"), aName);
    m_pDoc->GetName(2, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);

    if (!checkFormula(*m_pDoc, ScAddress(1,1,2), "SUM(Sheet1.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,2), "SUM($Sheet1.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

    // Move the last sheet (Sheet2) to the first position.
    m_pDoc->MoveTab(2, 0);

    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "SUM(Sheet1.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "SUM($Sheet1.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

    // Move back.
    m_pDoc->MoveTab(0, 2);

    if (!checkFormula(*m_pDoc, ScAddress(1,1,2), "SUM(Sheet1.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,2), "SUM($Sheet1.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

    // Move the "Temp" sheet to the last position.
    m_pDoc->MoveTab(1, 2);

    if (!checkFormula(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

    // Move back.
    m_pDoc->MoveTab(2, 1);

    // Delete the temporary sheet.
    m_pDoc->DeleteTab(1);

    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);

    if (!checkFormula(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

    // Insert a new sheet before the first one.
    m_pDoc->InsertTab(0, "Temp");

    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aName);
    m_pDoc->GetName(2, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);

    if (!checkFormula(*m_pDoc, ScAddress(1,1,2), "SUM(Sheet1.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,2), "SUM($Sheet1.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

    // Delete the temporary sheet.
    m_pDoc->DeleteTab(0);

    if (!checkFormula(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

    // Append a bunch of sheets.
    m_pDoc->InsertTab(2, "Temp1");
    m_pDoc->InsertTab(3, "Temp2");
    m_pDoc->InsertTab(4, "Temp3");

    // Move these tabs around. This shouldn't affects the first 2 sheets.
    m_pDoc->MoveTab(2, 4);
    m_pDoc->MoveTab(3, 2);

    if (!checkFormula(*m_pDoc, ScAddress(1,1,1), "SUM(Sheet1.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,1), "SUM($Sheet1.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

    // Delete the temp sheets.
    m_pDoc->DeleteTab(4);
    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);

    // Delete Sheet1.
    m_pDoc->DeleteTab(0);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aName);

    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "SUM(#REF!.B2:C3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B2.");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "SUM($#REF!.$B$2:$C$3)"))
        CPPUNIT_FAIL("Wrong formula in Sheet2.B3.");

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
    rFunc.InsertCells(ScRange(0,0,0,MAXCOL,1,0), &aMark, INS_INSROWS, false, true, false);

    // The raw data should have shifted to B4:B6.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,4,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,5,0)));

    if (!checkFormula(*m_pDoc, ScAddress(1,6,0), "SUM(B4:B6)"))
        CPPUNIT_FAIL("Wrong formula!");

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
    rFunc.InsertCells(ScRange(0,1,0,MAXCOL,3,0), &aMark, INS_INSROWS, false, true, false);
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0,5,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_MESSAGE("This formula cell should not be an error.", pFC->GetErrCode() == 0);
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "MAX(A7:A9)"))
        CPPUNIT_FAIL("Wrong formula!");

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaRefUpdateInsertColumns()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    setExpandRefs(false);

    m_pDoc->InsertTab(0, "Formula");

    // Set values in B1:B3.
    m_pDoc->SetValue(ScAddress(1,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,1,0), 2.0);
    m_pDoc->SetValue(ScAddress(1,2,0), 3.0);

    // Reference them in B4.
    m_pDoc->SetString(ScAddress(1,3,0), "=SUM(B1:B3)");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,3,0)));

    // Inert columns over A:B.
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    rFunc.InsertCells(ScRange(0,0,0,1,MAXROW,0), &aMark, INS_INSCOLS, false, true, false);

    // Now, the original column B has moved to column D.
    if (!checkFormula(*m_pDoc, ScAddress(3,3,0), "SUM(D1:D3)"))
        CPPUNIT_FAIL("Wrong formula in D4 after column insertion.");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(3,3,0)));

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

    // Move B4:B6 to D4 (two columsn to the right).
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(1,3,0,1,5,0), ScAddress(3,3,0), true, false, false, false);
    CPPUNIT_ASSERT_MESSAGE("Failed to move B4:B6.", bMoved);

    // The results of the formula cells that reference the moved range should remain the same.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,8,0));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,9,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0,10,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(0,11,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,8,0), "SUM(D4:D6)"))
        CPPUNIT_FAIL("Wrong formula.");

    if (!checkFormula(*m_pDoc, ScAddress(0,9,0), "SUM($D$4:$D$6)"))
        CPPUNIT_FAIL("Wrong formula.");

    if (!checkFormula(*m_pDoc, ScAddress(0,10,0), "D5"))
        CPPUNIT_FAIL("Wrong formula.");

    if (!checkFormula(*m_pDoc, ScAddress(0,11,0), "$D$6"))
        CPPUNIT_FAIL("Wrong formula.");

    // Move A9:A12 to B10:B13.
    bMoved = rFunc.MoveBlock(ScRange(0,8,0,0,11,0), ScAddress(1,9,0), true, false, false, false);
    CPPUNIT_ASSERT_MESSAGE("Failed to move A9:A12 to B10:B13", bMoved);

    // The results of these formula cells should still stay the same.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(1,9,0));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(1,10,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1,11,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,12,0));

    // Displayed formulas should stay the same since the referenced range hasn't moved.
    if (!checkFormula(*m_pDoc, ScAddress(1,9,0), "SUM(D4:D6)"))
        CPPUNIT_FAIL("Wrong formula.");

    if (!checkFormula(*m_pDoc, ScAddress(1,10,0), "SUM($D$4:$D$6)"))
        CPPUNIT_FAIL("Wrong formula.");

    if (!checkFormula(*m_pDoc, ScAddress(1,11,0), "D5"))
        CPPUNIT_FAIL("Wrong formula.");

    if (!checkFormula(*m_pDoc, ScAddress(1,12,0), "$D$6"))
        CPPUNIT_FAIL("Wrong formula.");

    // The value cells are in D4:D6. Move D4:D5 to the right but leave D6
    // where it is.
    bMoved = rFunc.MoveBlock(ScRange(3,3,0,3,4,0), ScAddress(4,3,0), true, false, false, false);
    CPPUNIT_ASSERT_MESSAGE("Failed to move D4:D5 to E4:E5", bMoved);

    // Only the values of B10 and B11 should be updated.
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,9,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,10,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1,11,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,12,0));

    if (!checkFormula(*m_pDoc, ScAddress(1,9,0), "SUM(D4:D6)"))
        CPPUNIT_FAIL("Wrong formula.");

    if (!checkFormula(*m_pDoc, ScAddress(1,10,0), "SUM($D$4:$D$6)"))
        CPPUNIT_FAIL("Wrong formula.");

    if (!checkFormula(*m_pDoc, ScAddress(1,11,0), "E5"))
        CPPUNIT_FAIL("Wrong formula.");

    if (!checkFormula(*m_pDoc, ScAddress(1,12,0), "$D$6"))
        CPPUNIT_FAIL("Wrong formula.");

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
    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "A1"))
        CPPUNIT_FAIL("Wrong formula.");

    m_pDoc->SetString(ScAddress(0,6,0), "=A1+A2+A3");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "A1+A2+A3"))
        CPPUNIT_FAIL("Wrong formula.");

    m_pDoc->SetString(ScAddress(0,7,0), "=A1+A3+A4");
    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(0,7,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,7,0), "A1+A3+A4"))
        CPPUNIT_FAIL("Wrong formula.");

    // Set formulas with range references in A10:A12.
    m_pDoc->SetString(ScAddress(0,9,0), "=SUM(A1:A2)");
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,9,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,9,0), "SUM(A1:A2)"))
        CPPUNIT_FAIL("Wrong formula.");

    m_pDoc->SetString(ScAddress(0,10,0), "=SUM(A1:A3)");
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,10,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,10,0), "SUM(A1:A3)"))
        CPPUNIT_FAIL("Wrong formula.");

    m_pDoc->SetString(ScAddress(0,11,0), "=SUM(A1:A4)");
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,11,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,11,0), "SUM(A1:A4)"))
        CPPUNIT_FAIL("Wrong formula.");

    // Move A1:A3 to C1:C3. Note that A4 remains.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(0,0,0,0,2,0), ScAddress(2,0,0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "C1"))
        CPPUNIT_FAIL("Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "C1+C2+C3"))
        CPPUNIT_FAIL("Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(0,7,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,7,0), "C1+C3+A4"))
        CPPUNIT_FAIL("Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,9,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,9,0), "SUM(C1:C2)"))
        CPPUNIT_FAIL("Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,10,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,10,0), "SUM(C1:C3)"))
        CPPUNIT_FAIL("Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(0,11,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,11,0), "SUM(A1:A4)"))
        CPPUNIT_FAIL("Wrong formula.");

    // Undo the move.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "A1"))
        CPPUNIT_FAIL("Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "A1+A2+A3"))
        CPPUNIT_FAIL("Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(0,7,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,7,0), "A1+A3+A4"))
        CPPUNIT_FAIL("Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,9,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,9,0), "SUM(A1:A2)"))
        CPPUNIT_FAIL("Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,10,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,10,0), "SUM(A1:A3)"))
        CPPUNIT_FAIL("Wrong formula.");

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,11,0)));
    if (!checkFormula(*m_pDoc, ScAddress(0,11,0), "SUM(A1:A4)"))
        CPPUNIT_FAIL("Wrong formula.");

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

    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "A1"))
        CPPUNIT_FAIL("Wrong formula");

    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "A2"))
        CPPUNIT_FAIL("Wrong formula");

    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(1,1,0)));

    // Move A1:A2 on Sheet1 to B3:B4 on Sheet2.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(0,0,0,0,1,0), ScAddress(1,2,1), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "Sheet2.B3"))
        CPPUNIT_FAIL("Wrong formula");

    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "Sheet2.B4"))
        CPPUNIT_FAIL("Wrong formula");

    // Undo and check again.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    pUndoMgr->Undo();

    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "A1"))
        CPPUNIT_FAIL("Wrong formula");

    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "A2"))
        CPPUNIT_FAIL("Wrong formula");

    // Redo and check.
    pUndoMgr->Redo();

    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "Sheet2.B3"))
        CPPUNIT_FAIL("Wrong formula");

    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "Sheet2.B4"))
        CPPUNIT_FAIL("Wrong formula");

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
    rFunc.DeleteContents(aMark, IDF_CONTENTS, true, true);

    CPPUNIT_ASSERT_MESSAGE("B2 should be empty.", m_pDoc->GetCellType(ScAddress(1,1,0)) == CELLTYPE_NONE);
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(2,1,0)));

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    // Undo and check the result of C2.
    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0))); // B2
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,1,0))); // C2

    // Redo and check.
    pUndoMgr->Redo();
    CPPUNIT_ASSERT_MESSAGE("B2 should be empty.", m_pDoc->GetCellType(ScAddress(1,1,0)) == CELLTYPE_NONE);
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
    bool bDeleted = rFunc.DeleteCells(ScRange(3,0,0,4,MAXROW,0), &aMark, DEL_CELLSLEFT, true, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncCol(-2);
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(C1:E1)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Undo and check.
    SfxUndoManager* pUndo = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndo);

    pUndo->Undo();
    aPos.IncCol(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(C1:G1)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Delete columns C:D (left end of the reference).
    bDeleted = rFunc.DeleteCells(ScRange(2,0,0,3,MAXROW,0), &aMark, DEL_CELLSLEFT, true, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncCol(-2);
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(C1:E1)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    aPos.IncCol(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(C1:G1)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Delete columns B:E (overlaps on the left).
    bDeleted = rFunc.DeleteCells(ScRange(1,0,0,4,MAXROW,0), &aMark, DEL_CELLSLEFT, true, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncCol(-4);
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(B1:C1)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    aPos.IncCol(4);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(C1:G1)"))
        CPPUNIT_FAIL("Wrong formula!");

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
    bDeleted = rFunc.DeleteCells(ScRange(5,0,0,7,MAXROW,0), &aMark, DEL_CELLSLEFT, true, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(C1:E1)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Undo and check.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(C1:H1)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Delete columns G:I (overlaps on the right).
    bDeleted = rFunc.DeleteCells(ScRange(6,0,0,8,MAXROW,0), &aMark, DEL_CELLSLEFT, true, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(C1:F1)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(C1:H1)"))
        CPPUNIT_FAIL("Wrong formula!");

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
    bool bDeleted = rFunc.DeleteCells(ScRange(0,3,0,MAXCOL,4,0), &aMark, DEL_CELLSUP, true, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncRow(-2);
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(A3:A5)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Undo and check.
    SfxUndoManager* pUndo = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndo);

    pUndo->Undo();
    aPos.IncRow(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(A3:A7)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Delete rows 3:4 (top end of the reference).
    bDeleted = rFunc.DeleteCells(ScRange(0,2,0,MAXCOL,3,0), &aMark, DEL_CELLSUP, true, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncRow(-2);
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(A3:A5)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    aPos.IncRow(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(A3:A7)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Delete rows 2:5 (overlaps on the top).
    bDeleted = rFunc.DeleteCells(ScRange(0,1,0,MAXCOL,4,0), &aMark, DEL_CELLSUP, true, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncRow(-4);
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(A2:A3)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    aPos.IncRow(4);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(A3:A7)"))
        CPPUNIT_FAIL("Wrong formula!");

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
    bDeleted = rFunc.DeleteCells(ScRange(0,5,0,MAXCOL,7,0), &aMark, DEL_CELLSUP, true, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(A3:A5)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Undo and check.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(A3:A8)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Delete rows 7:9 (overlaps on the bottom).
    bDeleted = rFunc.DeleteCells(ScRange(0,6,0,MAXCOL,8,0), &aMark, DEL_CELLSUP, true, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(A3:A6)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Undo and check again.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    if (!checkFormula(*m_pDoc, aPos, "SUM(A3:A8)"))
        CPPUNIT_FAIL("Wrong formula!");

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
        m_pDoc, "ToLeft", "RC[-1]", ScAddress(2,1,0), RT_NAME, formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1);

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
        m_pDoc, "MyRange", "$B$10:$B$12", ScAddress(0,0,0), RT_NAME, formula::FormulaGrammar::GRAM_NATIVE);
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
        m_pDoc, "MyRange", "$B$1:$C$6", ScAddress(0,0,0), RT_NAME, formula::FormulaGrammar::GRAM_NATIVE);
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
    rFunc.InsertCells(ScRange(0,3,0,MAXCOL,3,0), &aMark, INS_INSROWS, false, true, false);
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
    rFunc.InsertCells(ScRange(0,2,0,MAXCOL,4,0), &aMark, INS_INSROWS, false, true, false);

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
    rFunc.InsertCells(ScRange(0,2,0,MAXCOL,2,0), &aMark, INS_INSROWS, false, true, false);

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
    rFunc.DeleteCells(ScRange(0,2,0,MAXCOL,2,0), &aMark, DEL_CELLSUP, true, true);

    // The reference in the name should get updated to B2:B3.
    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$B$2:$B$3"), aExpr);

    // Delete row 3 again.
    rFunc.DeleteCells(ScRange(0,2,0,MAXCOL,2,0), &aMark, DEL_CELLSUP, true, true);
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
    rFunc.DeleteCells(ScRange(0,1,0,MAXCOL,2,0), &aMark, DEL_CELLSUP, true, true);

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$B$2:$B$2"), aExpr);

    // Undo and check.
    pUndoMgr->Undo();

    pName = m_pDoc->GetRangeName()->findByUpperName("MYRANGE");
    CPPUNIT_ASSERT(pName);
    pCode = pName->GetCode();

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("$B$2:$B$4"), aExpr);

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
    bool bInserted = rFunc.InsertCells(ScRange(1,0,0,1,MAXROW,0), &aMark, INS_INSCOLS, true, true, false);
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

    m_pDoc->DeleteTab(0);
}

void Test::testFuncCOUNTBLANK()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, "Formula");

    const char* aData[][4] = {
        { "1", 0, "=B1", "=\"\"" },
        { "2", 0, "=B2", "=\"\"" },
        { "A", 0, "=B3", "=\"\"" },
        { "B", 0, "=B4", "=D3" },
        {   0, 0, "=B5", "=D4" },
        { "=COUNTBLANK(A1:A5)", "=COUNTBLANK(B1:B5)", "=COUNTBLANK(C1:C5)", "=COUNTBLANK(D1:D5)" }
    };

    ScAddress aPos(0,0,0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT(aRange.aStart == aPos);

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(1,5,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(2,5,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(3,5,0)));

    // Test single cell reference cases.

    clearSheet(m_pDoc, 0);

    const char* aData2[][2] = {
        { "1",     "=COUNTBLANK(A1)" },
        { "A",     "=COUNTBLANK(A2)" },
        {   0,     "=COUNTBLANK(A3)" },
        { "=\"\"", "=COUNTBLANK(A4)" },
        { "=A4"  , "=COUNTBLANK(A5)" },
    };

    aRange = insertRangeData(m_pDoc, aPos, aData2, SAL_N_ELEMENTS(aData2));
    CPPUNIT_ASSERT(aRange.aStart == aPos);

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
    rFunc.InsertCells(ScRange(0,3,0,MAXCOL,3,0), &aMark, INS_INSROWS, false, true, false);
    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "ROW(A6)"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "ROW(B6)"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "ROW(B7)"))
        CPPUNIT_FAIL("Wrong formula!");

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
    CPPUNIT_ASSERT_MESSAGE("Cell should have a division by zero error.",
                           nErr == errDivisionByZero);
    nErr = m_pDoc->GetErrCode(ScAddress(0,3,0));
    CPPUNIT_ASSERT_MESSAGE("SUM should have also inherited a div-by-zero error.",
                           nErr == errDivisionByZero);

    // Set #NA! to A2. A4 should now inherit this error.
    m_pDoc->SetString(ScAddress(0,1,0), "=NA()");
    nErr = m_pDoc->GetErrCode(ScAddress(0,1,0));
    CPPUNIT_ASSERT_MESSAGE("A2 should be an error.", nErr);
    CPPUNIT_ASSERT_MESSAGE("A4 should have inherited the same error as A2.",
                           nErr == m_pDoc->GetErrCode(ScAddress(0,3,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncPRODUCT()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto recalc.

    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    double val = 1;
    double result;
    m_pDoc->SetValue(0, 0, 0, val);
    val = 2;
    m_pDoc->SetValue(0, 1, 0, val);
    val = 3;
    m_pDoc->SetValue(0, 2, 0, val);
    m_pDoc->SetString(0, 3, 0, OUString("=PRODUCT(A1:A3)"));
    m_pDoc->GetValue(0, 3, 0, result);
    CPPUNIT_ASSERT_MESSAGE("Calculation of PRODUCT failed", result == 6.0);

    m_pDoc->SetString(0, 4, 0, OUString("=PRODUCT({2;3;4})"));
    m_pDoc->GetValue(0, 4, 0, result);
    CPPUNIT_ASSERT_MESSAGE("Calculation of PRODUCT with inline array failed", result == 24.0);

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
    CPPUNIT_ASSERT_MESSAGE("This formula should be an array.", pFC->GetMatrixFlag() == MM_FORMULA);

    SCCOL nCols;
    SCROW nRows;
    pFC->GetMatColsRows(nCols, nRows);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCols);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), nRows);

    CPPUNIT_ASSERT_MESSAGE("Formula in C1 is invalid.", m_pDoc->GetErrCode(ScAddress(2,0,0)) == 0);
    CPPUNIT_ASSERT_MESSAGE("Formula in C2 is invalid.", m_pDoc->GetErrCode(ScAddress(2,1,0)) == 0);

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
    m_pDoc->SetString(0, 2, 0, OUString("Text"));
    val = 1;
    m_pDoc->SetValue(0, 3, 0, val);
    val = -1;
    m_pDoc->SetValue(0, 4, 0, val);
    val = 12.3;
    m_pDoc->SetValue(0, 5, 0, val);
    m_pDoc->SetString(0, 6, 0, OUString("'12.3"));

    // Cell references
    m_pDoc->SetString(1, 0, 0, OUString("=N(A1)"));
    m_pDoc->SetString(1, 1, 0, OUString("=N(A2)"));
    m_pDoc->SetString(1, 2, 0, OUString("=N(A3)"));
    m_pDoc->SetString(1, 3, 0, OUString("=N(A4)"));
    m_pDoc->SetString(1, 4, 0, OUString("=N(A5)"));
    m_pDoc->SetString(1, 5, 0, OUString("=N(A6)"));
    m_pDoc->SetString(1, 6, 0, OUString("=N(A9)"));

    // In-line values
    m_pDoc->SetString(1, 7, 0, OUString("=N(0)"));
    m_pDoc->SetString(1, 8, 0, OUString("=N(1)"));
    m_pDoc->SetString(1, 9, 0, OUString("=N(-1)"));
    m_pDoc->SetString(1, 10, 0, OUString("=N(123)"));
    m_pDoc->SetString(1, 11, 0, OUString("=N(\"\")"));
    m_pDoc->SetString(1, 12, 0, OUString("=N(\"12\")"));
    m_pDoc->SetString(1, 13, 0, OUString("=N(\"foo\")"));

    // Range references
    m_pDoc->SetString(2, 2, 0, OUString("=N(A1:A8)"));
    m_pDoc->SetString(2, 3, 0, OUString("=N(A1:A8)"));
    m_pDoc->SetString(2, 4, 0, OUString("=N(A1:A8)"));
    m_pDoc->SetString(2, 5, 0, OUString("=N(A1:A8)"));

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

    m_pDoc->SetString(0, 0, 0, OUString("=\"\""));
    m_pDoc->SetString(0, 1, 0, OUString("=COUNTIF(A1;1)"));

    double result = m_pDoc->GetValue(0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("We shouldn't count empty string as valid number.", result == 0.0);

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
    m_pDoc->InsertMatrixFormula(0, 20 + nRows-2, 0, 20 + nRows-1, aMark, "=IFERROR(3*A11:A12;1998)", NULL);

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

void Test::testFuncSHEET()
{
    OUString aTabName1("test1");
    OUString aTabName2("test2");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (SC_TAB_APPEND, aTabName1));

    m_pDoc->SetString(0, 0, 0, OUString("=SHEETS()"));
    m_pDoc->CalcFormulaTree(false, false);
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

void Test::testFuncNOW()
{
    OUString aTabName("foo");
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
    m_pDoc->CalcFormulaTree(false, false);
    double zero;
    m_pDoc->GetValue(0, 1, 0, zero);
    CPPUNIT_ASSERT_MESSAGE("Result should equal the 3rd parameter of IF, which is zero.", zero == 0.0);

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
    m_pDoc->InsertMatrixFormula(1, 0, 1, 2, aMark, "=LEN(A1:A3)", NULL);

    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_MESSAGE("This formulashould be a matrix origin.",
                           pFC->GetMatrixFlag() == MM_FORMULA);

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
        { 0, 0 } // terminator
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
        { 0, 0 } // terminator
    };

    // Insert check formulas into A5:B7.
    for (SCROW i = 0; aData2[i][0]; ++i)
    {
        m_pDoc->SetString(0, i+4, 0, OUString::createFromAscii(aData2[i][0]));
        m_pDoc->SetString(1, i+4, 0, OUString::createFromAscii(aData2[i][1]));
    }

    printRange(m_pDoc, ScRange(0,4,0,1,6,0), "Data range for LOOKUP.");

    // Values for B5:B7 should be 1, 2, and 3.
    CPPUNIT_ASSERT_MESSAGE("This formula should not have an error code.", m_pDoc->GetErrCode(ScAddress(1,4,0)) == 0);
    CPPUNIT_ASSERT_MESSAGE("This formula should not have an error code.", m_pDoc->GetErrCode(ScAddress(1,5,0)) == 0);
    CPPUNIT_ASSERT_MESSAGE("This formula should not have an error code.", m_pDoc->GetErrCode(ScAddress(1,6,0)) == 0);

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,4,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,5,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,6,0)));

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
        { 0, 0 } // terminator
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

    m_pDoc->DeleteTab(0);
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
        pDoc->SetString(0, i, 0, OUString::createFromAscii(aData[i]));

    for (size_t i = 0; i < _FormulaSize; ++i)
    {
        pDoc->SetString(1, i, 0, OUString::createFromAscii(aChecks[i].pVal));

        OUStringBuffer aBuf;
        aBuf.appendAscii("=MATCH(B");
        aBuf.append(static_cast<sal_Int32>(i+1));
        aBuf.appendAscii(";A1:A");
        aBuf.append(static_cast<sal_Int32>(nDataSize));
        aBuf.appendAscii(";");
        aBuf.append(static_cast<sal_Int32>(_Type));
        aBuf.appendAscii(")");
        OUString aFormula = aBuf.makeStringAndClear();
        pDoc->SetString(2, i, 0, aFormula);
    }

    pDoc->CalcAll();
    Test::printRange(pDoc, ScRange(0, 0, 0, 2, _FormulaSize-1, 0), "MATCH");

    // verify the results.
    for (size_t i = 0; i < _FormulaSize; ++i)
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

template<size_t _DataSize, size_t _FormulaSize, int _Type>
void runTestHorizontalMATCH(ScDocument* pDoc, const char* aData[_DataSize], StrStrCheck aChecks[_FormulaSize])
{
    size_t nDataSize = _DataSize;
    for (size_t i = 0; i < nDataSize; ++i)
        pDoc->SetString(i, 0, 0, OUString::createFromAscii(aData[i]));

    for (size_t i = 0; i < _FormulaSize; ++i)
    {
        pDoc->SetString(i, 1, 0, OUString::createFromAscii(aChecks[i].pVal));

        // Assume we don't have more than 26 data columns..
        OUStringBuffer aBuf;
        aBuf.appendAscii("=MATCH(");
        aBuf.append(static_cast<sal_Unicode>('A'+i));
        aBuf.appendAscii("2;A1:");
        aBuf.append(static_cast<sal_Unicode>('A'+nDataSize));
        aBuf.appendAscii("1;");
        aBuf.append(static_cast<sal_Int32>(_Type));
        aBuf.appendAscii(")");
        OUString aFormula = aBuf.makeStringAndClear();
        pDoc->SetString(i, 2, 0, aFormula);
    }

    pDoc->CalcAll();
    Test::printRange(pDoc, ScRange(0, 0, 0, _FormulaSize-1, 2, 0), "MATCH");

    // verify the results.
    for (size_t i = 0; i < _FormulaSize; ++i)
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
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

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
    CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", m_pDoc->GetString(0,10,0) == aTest);

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
        // Default is to use the current formula syntax, which is Calc A1.
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
    aConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_OOO;
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
            CPPUNIT_ASSERT_MESSAGE("Wrong value!", aVal == *aChecks[i]);
        }
    }

    aConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_XL_A1;
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
            CPPUNIT_ASSERT_MESSAGE("Wrong value!", aVal == *aChecks[i]);
        }
    }

    aConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_XL_R1C1;
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
            CPPUNIT_ASSERT_MESSAGE("Wrong value!", aVal == *aChecks[i]);
        }
    }

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
        { "3", "4", "=SUM(A2:B2)", 0 },
        { "5", "6", "=SUM(A3:B3)", 0 },
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
    CPPUNIT_ASSERT_MESSAGE("A5 should have one listener.", pListeners->size() == 1);
    SvtListener* pListener = pListeners->at(0);
    CPPUNIT_ASSERT_MESSAGE("A6 should be listening to A5.", pListener == pFC);

    // Check initial values.
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(90.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Delete row 2.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    rFunc.DeleteCells(ScRange(0,1,0,MAXCOL,1,0), &aMark, DEL_CELLSUP, true, true);

    pBC = m_pDoc->GetBroadcaster(ScAddress(0,3,0));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster at A5 should have shifted to A4.", pBC);
    pListeners = &pBC->GetAllListeners();
    CPPUNIT_ASSERT_MESSAGE("A3 should have one listener.", pListeners->size() == 1);
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
        { 0, 0, 0 },           // empty row to separate the formula groups.
        { "3", "=A3", "=B3" }, // grouped
        { "4", "=A4", "=B4" }, // grouped
    };

    ScAddress aPos(0,0,0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT(aRange.aStart == aPos);

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
    rFunc.DeleteCells(ScRange(0,0,0,0,MAXROW,0), &aMark, DEL_CELLSLEFT, true, true);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "#REF!", "#REF!" },
            { 0,  0 },
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
            { 0,  0, 0 },
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
            { 0, 0 },
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
            { 0, 0, 0 },
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
    m_pDoc->InsertMatrixFormula(1, 0, 1, 0, aMark, "=A1", NULL);
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
                           findLoadedDocShellByName(aExtDocName) != NULL);

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
    m_pDoc->InsertTab(0, OUString("Test Sheet"));
    m_pDoc->SetString(0, 0, 0, OUString("='file:///extdata.fake'#Data1.A1"));
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

    m_pDoc->SetString(1, 0, 0, OUString("='file:///extdata.fake'#Data1.B1"));
    test = m_pDoc->GetString(1, 0, 0);
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
            test = m_pDoc->GetString(0, static_cast<SCROW>(i+1), 0);
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
            test = m_pDoc->GetString(2, static_cast<SCROW>(i), 0);
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
            test = m_pDoc->GetString(3, static_cast<SCROW>(i), 0);
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

void testExtRefFuncT(ScDocument* pDoc, ScDocument& rExtDoc)
{
    Test::clearRange(pDoc, ScRange(0, 0, 0, 1, 9, 0));
    Test::clearRange(&rExtDoc, ScRange(0, 0, 0, 1, 9, 0));

    rExtDoc.SetString(0, 0, 0, OUString("'1.2"));
    rExtDoc.SetString(0, 1, 0, OUString("Foo"));
    rExtDoc.SetValue(0, 2, 0, 12.3);
    pDoc->SetString(0, 0, 0, OUString("=T('file:///extdata.fake'#Data.A1)"));
    pDoc->SetString(0, 1, 0, OUString("=T('file:///extdata.fake'#Data.A2)"));
    pDoc->SetString(0, 2, 0, OUString("=T('file:///extdata.fake'#Data.A3)"));
    pDoc->CalcAll();

    OUString aRes = pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE( "Unexpected result with T.", aRes == "1.2" );
    aRes = pDoc->GetString(0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE( "Unexpected result with T.", aRes == "Foo" );
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
                           findLoadedDocShellByName(aExtDocName) != NULL);

    ScExternalRefManager* pRefMgr = m_pDoc->GetExternalRefManager();
    CPPUNIT_ASSERT_MESSAGE("external reference manager doesn't exist.", pRefMgr);
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aExtDocName);
    const OUString* pFileName = pRefMgr->getExternalFileName(nFileId);
    CPPUNIT_ASSERT_MESSAGE("file name registration has somehow failed.",
                           pFileName && pFileName->equals(aExtDocName));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    // Populate the external source document.
    ScDocument& rExtDoc = xExtDocSh->GetDocument();
    rExtDoc.InsertTab(0, OUString("Data"));
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

    m_pDoc->InsertTab(0, OUString("Test"));

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
        m_pDoc->SetString(0, 0, 0, OUString::createFromAscii(aChecks[i].pFormula));
        m_pDoc->GetValue(0, 0, 0, val);
        CPPUNIT_ASSERT_MESSAGE("unexpected result involving external ranges.", val == aChecks[i].fResult);
    }

    pRefMgr->clearCache(nFileId);
    testExtRefFuncT(m_pDoc, rExtDoc);
    testExtRefFuncOFFSET(m_pDoc, rExtDoc);
    testExtRefFuncVLOOKUP(m_pDoc, rExtDoc);

    // Unload the external document shell.
    xExtDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("external document instance should have been unloaded.",
                           findLoadedDocShellByName(aExtDocName) == NULL);

    m_pDoc->DeleteTab(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
