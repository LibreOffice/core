/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/debughelper.hxx"
#include "helper/qahelper.hxx"
#include <scopetools.hxx>
#include <formulacell.hxx>
#include <docfunc.hxx>
#include <inputopt.hxx>
#include <tokenstringcontext.hxx>
#include <refupdatecontext.hxx>
#include <dbdata.hxx>
#include <validat.hxx>
#include <scitems.hxx>
#include <docpool.hxx>
#include <scmod.hxx>
#include <undomanager.hxx>

#include <formula/vectortoken.hxx>
#include <svl/intitem.hxx>

#include <memory>
#include <algorithm>
#include <vector>

using namespace formula;
using ::std::cerr;
using ::std::endl;

namespace {
void setExpandRefs(bool bExpand)
{
    ScModule* pMod = SC_MOD();
    ScInputOptions aOpt = pMod->GetInputOptions();
    aOpt.SetExpandRefs(bExpand);
    pMod->SetInputOptions(aOpt);
}

void testFormulaRefUpdateNameCopySheetCheckTab( const ScDocument* pDoc, SCTAB nTab, bool bCheckNames )
{
    if (bCheckNames)
    {
        const ScRangeData* pName;
        pName = pDoc->GetRangeName(nTab)->findByUpperName(u"GLOBAL"_ustr);
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name GLOBAL should exist", pName);
        pName = pDoc->GetRangeName(nTab)->findByUpperName(u"LOCAL"_ustr);
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name LOCAL should exist", pName);
        pName = pDoc->GetRangeName(nTab)->findByUpperName(u"GLOBAL_GLOBAL"_ustr);
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name GLOBAL_GLOBAL should exist", pName);
        pName = pDoc->GetRangeName(nTab)->findByUpperName(u"GLOBAL_LOCAL"_ustr);
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name GLOBAL_LOCAL should exist", pName);
        pName = pDoc->GetRangeName(nTab)->findByUpperName(u"GLOBAL_UNUSED"_ustr);
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name GLOBAL_UNUSED should exist", pName);
        pName = pDoc->GetRangeName(nTab)->findByUpperName(u"GLOBAL_UNUSED_NOREF"_ustr);
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name GLOBAL_UNUSED_NOREF should not exist", !pName);
        pName = pDoc->GetRangeName(nTab)->findByUpperName(u"LOCAL_GLOBAL"_ustr);
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name LOCAL_GLOBAL should exist", pName);
        pName = pDoc->GetRangeName(nTab)->findByUpperName(u"LOCAL_LOCAL"_ustr);
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name LOCAL_LOCAL should exist", pName);
        pName = pDoc->GetRangeName(nTab)->findByUpperName(u"LOCAL_UNUSED"_ustr);
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name LOCAL_UNUSED should exist", pName);
        pName = pDoc->GetRangeName(nTab)->findByUpperName(u"LOCAL_UNUSED_NOREF"_ustr);
        CPPUNIT_ASSERT_MESSAGE("Sheet-local name LOCAL_UNUSED_NOREF should exist", pName);
    }

    ScAddress aPos(0,0,0);
    aPos.SetRow(0);
    aPos.SetTab(nTab);
    int nSheet = nTab + 1;
    CPPUNIT_ASSERT_EQUAL( 1.0 * nSheet, pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL( 11.0 * nSheet, pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL( 100.0 * nSheet, pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL( 11000.0 * nSheet, pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL( 10000.0 * nSheet, pDoc->GetValue(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL( 1100000.0 * nSheet, pDoc->GetValue(aPos));
}


}

class TestFormula : public ScUcalcTestBase
{
};

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaCreateStringFromTokens)
{
    // Insert sheets.
    m_pDoc->InsertTab(0, u"Test"_ustr);
    m_pDoc->InsertTab(1, u"Kevin's Data"_ustr);
    m_pDoc->InsertTab(2, u"Past Data"_ustr);
    m_pDoc->InsertTab(3, u"2013"_ustr);

    // Insert named ranges.
    static const struct {
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

    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        ScRangeData* pName = new ScRangeData(
            *m_pDoc, OUString::createFromAscii(aNames[i].pName), OUString::createFromAscii(aNames[i].pExpr),
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
    static const struct {
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

    for (size_t i = 0; i < SAL_N_ELEMENTS(aDBs); ++i)
    {
        std::unique_ptr<ScDBData> pData( new ScDBData(
            OUString::createFromAscii(
                aDBs[i].pName), aDBs[i].nTab, aDBs[i].nCol1, aDBs[i].nRow1, aDBs[i].nCol2,aDBs[i].nRow2) );
        bool bInserted = pDBs->getNamedDBs().insert(std::move(pData));
        CPPUNIT_ASSERT_MESSAGE(
            OString(
                OString::Concat("Failed to insert \"") + aDBs[i].pName + "\"").getStr(),
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

    sc::TokenStringContext aCxt(*m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH);

    // Artificially add external reference data after the context object is
    // initialized.
    aCxt.maExternalFileNames.emplace_back("file:///path/to/fake.file");
    std::vector<OUString> aExtTabNames;
    aExtTabNames.emplace_back("Sheet");
    aCxt.maExternalCachedTabNames.emplace(0, aExtTabNames);

    ScAddress aPos(0,0,0);

    for (size_t i = 0; i < SAL_N_ELEMENTS(aTests); ++i)
    {
#if 0
        OUString aFormula = OUString::createFromAscii(aTests[i]);
#endif
        ScCompiler aComp(*m_pDoc, aPos, FormulaGrammar::GRAM_ENGLISH);
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
        return std::isnan(rArray.mpNumericArray[nPos]);
    else
        return true;
}

bool equals( const formula::VectorRefArray& rArray, size_t nPos, double fVal )
{
    if (rArray.mpStringArray && rArray.mpStringArray[nPos])
        // This is a string cell.
        return false;

    return rArray.mpNumericArray && rArray.mpNumericArray[nPos] == fVal;
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

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaParseReference)
{
    OUString aTab1(u"90's Music"_ustr), aTab2(u"90's and 70's"_ustr), aTab3(u"All Others"_ustr), aTab4(u"NoQuote"_ustr);
    m_pDoc->InsertTab(0, u"Dummy"_ustr); // just to shift the sheet indices...
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
            "='90''s Music'.B12",
            "='90''s and 70''s'.$AB$100",
            "='All Others'.Z$100",
            "=NoQuote.$C111"
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            // Use the 'Dummy' sheet for this.
            m_pDoc->SetString(ScAddress(0,0,0), OUString::createFromAscii(aChecks[i]));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", OUString::createFromAscii(aChecks[i]), m_pDoc->GetFormula(0,0,0));
        }
    }

    ScAddress aPos;
    ScAddress::ExternalInfo aExtInfo;
    ScRefFlags nRes = aPos.Parse(u"'90''s Music'.D10"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(3), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

    nRes = aPos.Parse(u"'90''s and 70''s'.C100"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(2), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(99), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

    nRes = aPos.Parse(u"'All Others'.B3"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(3), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

    nRes = aPos.Parse(u"NoQuote.E13"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO, &aExtInfo);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(4), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(4), aPos.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), aPos.Row());
    CPPUNIT_ASSERT_MESSAGE("This is not an external address.", !aExtInfo.mbExternal);

    ScRange aRange;

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u":B"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u"B:"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u":B2"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u"B2:"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u":2"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u"2:"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u":2B"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u"2B:"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u"abc_foo:abc_bar"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u"B1:B2~C1"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u"B:B"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aStart.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), aRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aEnd.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->MaxRow(), aRange.aEnd.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                                                 ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                                                         ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID)));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::ZERO),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS)));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS)));

    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(u"2:2"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aStart.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), aRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), aRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aEnd.Tab());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->MaxCol(), aRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), aRange.aEnd.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                                                 ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                                                         ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID)));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::ZERO),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS)));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS)));

    nRes = aRange.Parse(u"NoQuote.B:C"_ustr, *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(4), aRange.aStart.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), aRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(4), aRange.aEnd.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(2), aRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->MaxRow(), aRange.aEnd.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                                                 ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                                                         ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID)));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::ZERO),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS)));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS)));

    // Both rows at sheet bounds and relative => convert to absolute => entire column reference.
    aRange.aStart.SetTab(0);
    nRes = aRange.Parse(m_pDoc->MaxRow() == MAXROW ? u"B1:B1048576"_ustr
                                                   : u"B1:B16777216"_ustr,
                        *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aStart.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), aRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aEnd.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), aRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->MaxRow(), aRange.aEnd.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                                                 ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                                                         ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID)));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::ZERO),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS)));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS)));

    // Both columns at sheet bounds and relative => convert to absolute => entire row reference.
    aRange.aStart.SetTab(0);
    nRes = aRange.Parse("A2:" + m_pDoc->MaxColAsString() + "2", *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aStart.Tab());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), aRange.aStart.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), aRange.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(0), aRange.aEnd.Tab());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->MaxCol(), aRange.aEnd.Col());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), aRange.aEnd.Row());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                                                 ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                                                         ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID)));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::ZERO),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS)));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS),
                         static_cast<sal_uInt16>(nRes & (ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS)));

    // Check for reference input conversion to and display string of entire column/row.
    {
        const char* row1048576Checks[][2] = {
            { "=B:B",           "=B:B" },
            { "=B1:B1048576",   "=B:B" },
            { "=B1:B$1048576",  "=B1:B$1048576" },
            { "=B$1:B1048576",  "=B$1:B1048576" },
            { "=B$1:B$1048576", "=B:B" }
        };
        const char* row16777216Checks[][2] = {
            { "=B:B",           "=B:B" },
            { "=B1:B16777216",   "=B:B" },
            { "=B1:B$16777216",  "=B1:B$16777216" },
            { "=B$1:B16777216",  "=B$1:B16777216" },
            { "=B$1:B$16777216", "=B:B" }
        };
        const char* col1024Checks[][2] = {
            { "=2:2",           "=2:2" },
            { "=A2:AMJ2",       "=2:2" },
            { "=A2:$AMJ2",      "=A2:$AMJ2" },
            { "=$A2:AMJ2",      "=$A2:AMJ2" },
            { "=$A2:$AMJ2",     "=2:2" }
        };
        const char* col16384Checks[][2] = {
            { "=2:2",           "=2:2" },
            { "=A2:XFD2",       "=2:2" },
            { "=A2:$XFD2",      "=A2:$XFD2" },
            { "=$A2:XFD2",      "=$A2:XFD2" },
            { "=$A2:$XFD2",     "=2:2" }
        };

        if (m_pDoc->MaxRow() == 1048575)
        {
            for (const auto& check : row1048576Checks)
            {
                // Use the 'Dummy' sheet for this.
                m_pDoc->SetString(ScAddress(0,0,0), OUString::createFromAscii(check[0]));
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", OUString::createFromAscii(check[1]), m_pDoc->GetFormula(0,0,0));
            }
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL(SCROW(16777215), m_pDoc->MaxRow());
            for (const auto& check : row16777216Checks)
            {
                m_pDoc->SetString(ScAddress(0,0,0), OUString::createFromAscii(check[0]));
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", OUString::createFromAscii(check[1]), m_pDoc->GetFormula(0,0,0));
            }
        }
        if (m_pDoc->MaxCol() == 1023)
        {
            for (const auto& check : col1024Checks)
            {
                m_pDoc->SetString(ScAddress(0,0,0), OUString::createFromAscii(check[0]));
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", OUString::createFromAscii(check[1]), m_pDoc->GetFormula(0,0,0));
            }
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL(SCCOL(16383), m_pDoc->MaxCol());
            for (const auto& check : col16384Checks)
            {
                m_pDoc->SetString(ScAddress(0,0,0), OUString::createFromAscii(check[0]));
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", OUString::createFromAscii(check[1]), m_pDoc->GetFormula(0,0,0));
            }
        }
    }

    m_pDoc->DeleteTab(4);
    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFetchVectorRefArray)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

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
    m_pDoc->SetString(ScAddress(1,0,0), u"Andy"_ustr);
    m_pDoc->SetString(ScAddress(1,1,0), u"Bruce"_ustr);
    m_pDoc->SetString(ScAddress(1,2,0), u"Charlie"_ustr);
    m_pDoc->SetString(ScAddress(1,3,0), u"David"_ustr);
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(1,0,0), 5);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array is expected to be string cells only.", !aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 0, u"Andy"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 1, u"Bruce"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 2, u"Charlie"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 3, u"David"_ustr));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 4));

    // Mixture of numeric, string, and empty cells in Column C.
    m_pDoc->SetString(ScAddress(2,0,0), u"Header"_ustr);
    m_pDoc->SetValue(ScAddress(2,1,0), 11);
    m_pDoc->SetValue(ScAddress(2,2,0), 12);
    m_pDoc->SetValue(ScAddress(2,3,0), 13);
    m_pDoc->SetString(ScAddress(2,5,0), u"=SUM(C2:C4)"_ustr);
    m_pDoc->CalcAll();

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(2,0,0), 7);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should have both numeric and string arrays.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should have both numeric and string arrays.", aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 0, u"Header"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 1, 11));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 2, 12));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 3, 13));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 4));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 5, 36));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 6));

    // Mixed type again in Column D, but it starts with a numeric cell.
    m_pDoc->SetValue(ScAddress(3,0,0), 10);
    m_pDoc->SetString(ScAddress(3,1,0), u"Below 10"_ustr);
    // Leave 2 empty cells.
    m_pDoc->SetValue(ScAddress(3,4,0), 11);
    m_pDoc->SetString(ScAddress(3,5,0), u"=12"_ustr);
    m_pDoc->SetString(ScAddress(3,6,0), u"=13"_ustr);
    m_pDoc->SetString(ScAddress(3,7,0), u"=CONCATENATE(\"A\";\"B\";\"C\")"_ustr);
    m_pDoc->CalcAll();

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(3,0,0), 8);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should have both numeric and string arrays.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should have both numeric and string arrays.", aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 0, 10));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 1, u"Below 10"_ustr));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 2));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 3));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 4, 11));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 5, 12));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 6, 13));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 7, u"ABC"_ustr));

    // Column E consists of formula cells whose results are all numeric.
    for (SCROW i = 0; i <= 6; ++i)
        m_pDoc->SetString(ScAddress(4,i,0), u"=ROW()"_ustr);
    m_pDoc->CalcAll();

    // Leave row 7 empty.
    m_pDoc->SetString(ScAddress(4,8,0), u"Andy"_ustr);
    m_pDoc->SetValue(ScAddress(4,9,0), 123);

    // This array fits within a single formula block.
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(4,0,0), 5);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should be purely numeric.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should be purely numeric.", !aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 0, 1));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 1, 2));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 2, 3));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 3, 4));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 4, 5));

    // This array spans over multiple blocks.
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(4,0,0), 11);
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should have both numeric and string arrays.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should have both numeric and string arrays.", aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 0, 1));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 1, 2));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 2, 3));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 3, 4));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 4, 5));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 5, 6));
    CPPUNIT_ASSERT_MESSAGE("Unexpected numeric cell.", equals(aArray, 6, 7));
    CPPUNIT_ASSERT_MESSAGE("This should be empty.", isEmpty(aArray, 7));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 8, u"Andy"_ustr));
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
    m_pDoc->SetString(ScAddress(5,4,0), u"=2*8"_ustr);
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
    m_pDoc->SetString(ScAddress(6,0,0), u"Title"_ustr);
    m_pDoc->SetString(ScAddress(6,1,0), u"foo"_ustr);
    m_pDoc->SetString(ScAddress(6,2,0), u"bar"_ustr);
    m_pDoc->SetString(ScAddress(6,3,0), u"foo"_ustr);
    m_pDoc->SetString(ScAddress(6,4,0), u"baz"_ustr);
    m_pDoc->SetString(ScAddress(6,5,0), u"quack"_ustr);
    m_pDoc->SetString(ScAddress(6,6,0), u"beep"_ustr);
    m_pDoc->SetString(ScAddress(6,7,0), u"kerker"_ustr);

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(6,1,0), 4); // G2:G5
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a numeric array.", !aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should have a string array.", aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 0, u"foo"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 1, u"bar"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 2, u"foo"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 3, u"baz"_ustr));

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(6,2,0), 4); // G3:G6
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch vector ref array.", aArray.isValid());
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a numeric array.", !aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should have a string array.", aArray.mpStringArray);
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 0, u"bar"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 1, u"foo"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 2, u"baz"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Unexpected string cell.", equals(aArray, 3, u"quack"_ustr));

    // Column H starts with formula cells.
    for (SCROW i = 0; i < 10; ++i)
        m_pDoc->SetString(ScAddress(7,i,0), u"=ROW()"_ustr);

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
    clearRange(m_pDoc, ScRange(0,0,0,m_pDoc->MaxCol(),m_pDoc->MaxRow(),0));
    m_pDoc->PrepareFormulaCalc();

    // Totally empty range in a totally empty column (Column A).
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(0,0,0), 3); // A1:A3
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[0]));
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[1]));
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[2]));

    // Totally empty range in a non-empty column (Column B).
    m_pDoc->SetString(ScAddress(1,10,0), u"Some text"_ustr); // B11
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(1,0,0), 3); // B1:B3
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[0]));
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[1]));
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[2]));

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(1,12,0), 3); // B13:B15
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[0]));
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[1]));
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[2]));

    // These values come from a cache because of the call above.
    aArray = m_pDoc->FetchVectorRefArray(ScAddress(1,1,0), 3); // B2:B4
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[0]));
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[1]));
    CPPUNIT_ASSERT(std::isnan(aArray.mpNumericArray[2]));

    // The column begins with a string header at row 1 (Column C).
    m_pDoc->SetString(ScAddress(2,0,0), u"MyHeader"_ustr);
    for (SCROW i = 1; i <= 9; ++i) // rows 2-10 are numeric.
        m_pDoc->SetValue(ScAddress(2,i,0), i);

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(2,1,0), 9); // C2:C10
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    for (size_t i = 0; i < 9; ++i)
        CPPUNIT_ASSERT_EQUAL(double(i+1), aArray.mpNumericArray[i]);

    // The column begins with a number, followed by a string then followed by
    // a block of numbers (Column D).
    m_pDoc->SetValue(ScAddress(3,0,0), 0.0);
    m_pDoc->SetString(ScAddress(3,1,0), u"Some string"_ustr);
    for (SCROW i = 2; i <= 9; ++i) // rows 3-10 are numeric.
        m_pDoc->SetValue(ScAddress(3,i,0), i);

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(3,2,0), 8); // D3:D10
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    for (size_t i = 0; i < 8; ++i)
        CPPUNIT_ASSERT_EQUAL(double(i+2), aArray.mpNumericArray[i]);

    // The column begins with a formula, followed by a string then followed by
    // a block of numbers (Column E).
    m_pDoc->SetString(ScAddress(4,0,0), u"=1*2"_ustr);
    m_pDoc->SetString(ScAddress(4,1,0), u"Some string"_ustr);
    for (SCROW i = 2; i <= 9; ++i) // rows 3-10 are numeric.
        m_pDoc->SetValue(ScAddress(4,i,0), i*2);

    aArray = m_pDoc->FetchVectorRefArray(ScAddress(4,2,0), 8); // E3:E10
    CPPUNIT_ASSERT_MESSAGE("Array should have a numeric array.", aArray.mpNumericArray);
    CPPUNIT_ASSERT_MESSAGE("Array should NOT have a string array.", !aArray.mpStringArray);
    for (size_t i = 0; i < 8; ++i)
        CPPUNIT_ASSERT_EQUAL(double((i+2)*2), aArray.mpNumericArray[i]);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testGroupConverter3D)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);
    m_pDoc->InsertTab(1, u"Test2"_ustr);

    m_pDoc->SetValue(1, 0, 0, 1.0);
    m_pDoc->SetValue(1, 0, 1, 2.0);

    for (SCROW nRow = 0; nRow < 200; ++nRow)
    {
        OUString aFormula = "=SUM(Test.B" + OUString::number(nRow+1) + ":Test2.B" + OUString::number(nRow+1) + ")";
        m_pDoc->SetString(0, nRow, 0, aFormula);
    }

    double nVal = m_pDoc->GetValue(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL(3.0, nVal);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaTokenEquality)
{
    struct FormulaTokenEqualityTest
    {
        const char* mpFormula1;
        const char* mpFormula2;
        bool mbEqual;
    };

    static const FormulaTokenEqualityTest aTests[] = {
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
        ScFormulaCell aCell1(*m_pDoc, ScAddress(), OUString::createFromAscii(aTests[i].mpFormula1), eGram);
        ScFormulaCell aCell2(*m_pDoc, ScAddress(), OUString::createFromAscii(aTests[i].mpFormula2), eGram);

        ScFormulaCell::CompareState eComp = aCell1.CompareByTokenArray(aCell2);
        if (aTests[i].mbEqual)
        {
            if (eComp == ScFormulaCell::NotEqual)
            {
                std::ostringstream os;
                os << "These two formulas should be evaluated equal: '"
                    << aTests[i].mpFormula1 << "' vs '" << aTests[i].mpFormula2 << "'" << endl;
                CPPUNIT_FAIL(os.str());
            }
        }
        else
        {
            if (eComp != ScFormulaCell::NotEqual)
            {
                std::ostringstream os;
                os << "These two formulas should be evaluated non-equal: '"
                    << aTests[i].mpFormula1 << "' vs '" << aTests[i].mpFormula2 << "'" << endl;
                CPPUNIT_FAIL(os.str());
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefData)
{
    std::unique_ptr<ScDocument> pDoc = std::make_unique<ScDocument>();

    ScAddress aAddr(4,5,3), aPos(2,2,2);
    ScSingleRefData aRef;
    aRef.InitAddress(aAddr);
    CPPUNIT_ASSERT_MESSAGE("Wrong ref data state.", !aRef.IsRowRel());
    CPPUNIT_ASSERT_MESSAGE("Wrong ref data state.", !aRef.IsColRel());
    CPPUNIT_ASSERT_MESSAGE("Wrong ref data state.", !aRef.IsTabRel());
    CPPUNIT_ASSERT_EQUAL(SCCOL(4), aRef.Col());
    CPPUNIT_ASSERT_EQUAL(SCROW(5), aRef.Row());
    CPPUNIT_ASSERT_EQUAL(SCTAB(3), aRef.Tab());

    aRef.SetRowRel(true);
    aRef.SetColRel(true);
    aRef.SetTabRel(true);
    aRef.SetAddress(pDoc->GetSheetLimits(), aAddr, aPos);
    CPPUNIT_ASSERT_EQUAL(SCCOL(2), aRef.Col());
    CPPUNIT_ASSERT_EQUAL(SCROW(3), aRef.Row());
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), aRef.Tab());

    // Test extension of range reference.

    ScComplexRefData aDoubleRef;
    aDoubleRef.InitRange(ScRange(2,2,0,4,4,0));

    aRef.InitAddress(ScAddress(6,5,0));

    aDoubleRef.Extend(pDoc->GetSheetLimits(), aRef, ScAddress());
    ScRange aTest = aDoubleRef.toAbs(*pDoc, ScAddress());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start position of extended range.", ScAddress(2,2,0), aTest.aStart);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end position of extended range.", ScAddress(6,5,0), aTest.aEnd);

    ScComplexRefData aDoubleRef2;
    aDoubleRef2.InitRangeRel(*pDoc, ScRange(1,2,0,8,6,0), ScAddress(5,5,0));
    aDoubleRef.Extend(pDoc->GetSheetLimits(), aDoubleRef2, ScAddress(5,5,0));
    aTest = aDoubleRef.toAbs(*pDoc, ScAddress(5,5,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start position of extended range.", ScAddress(1,2,0), aTest.aStart);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end position of extended range.", ScAddress(8,6,0), aTest.aEnd);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaCompiler)
{
    static const struct {
        const char* pInput; FormulaGrammar::Grammar eInputGram;
        const char* pOutput; FormulaGrammar::Grammar eOutputGram;
    } aTests[] = {
        { "=B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE, "[.B1]-[.$C2]+[.D$3]-[.$E$4]", FormulaGrammar::GRAM_ODFF },
        { "=B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE, "B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE },
        { "=B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE, "B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE_XL_A1 },
        { "=B1-$C2+D$3-$E$4", FormulaGrammar::GRAM_NATIVE, "RC[1]-R[1]C3+R3C[3]-R4C5", FormulaGrammar::GRAM_NATIVE_XL_R1C1 },
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aTests); ++i)
    {
        std::unique_ptr<ScTokenArray> pArray = compileFormula(m_pDoc, OUString::createFromAscii(aTests[i].pInput), aTests[i].eInputGram);
        CPPUNIT_ASSERT_MESSAGE("Token array shouldn't be NULL!", pArray);

        ScCompiler aComp(*m_pDoc, ScAddress(), *pArray, aTests[i].eOutputGram);
        OUStringBuffer aBuf;
        aComp.CreateStringFromTokenArray(aBuf);
        OUString aFormula = aBuf.makeStringAndClear();

        CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(aTests[i].pOutput), aFormula);
    }
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaCompilerJumpReordering)
{
    struct TokenCheck
    {
        OpCode meOp;
        StackVar meType;
    };

    {
        // Compile formula string first.
        std::unique_ptr<ScTokenArray> pCode(compileFormula(m_pDoc, u"=IF(B1;12;\"text\")"_ustr));
        CPPUNIT_ASSERT(pCode);

        // Then generate RPN tokens.
        ScCompiler aCompRPN(*m_pDoc, ScAddress(), *pCode, FormulaGrammar::GRAM_NATIVE);
        aCompRPN.CompileTokenArray();

        // RPN tokens should be ordered: B1, ocIf, C1, ocSep, D1, ocClose.
        static const TokenCheck aCheckRPN[] =
        {
            { ocPush,  svSingleRef },
            { ocIf,    svUnknown   }, // type is context dependent, don't test it
            { ocPush,  svDouble    },
            { ocSep,   svSep       },
            { ocPush,  svString    },
            { ocClose, svSep       },
        };

        sal_uInt16 nLen = pCode->GetCodeLen();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong RPN token count.", static_cast<sal_uInt16>(SAL_N_ELEMENTS(aCheckRPN)), nLen);

        FormulaToken** ppTokens = pCode->GetCode();
        for (sal_uInt16 i = 0; i < nLen; ++i)
        {
            const FormulaToken* p = ppTokens[i];
            CPPUNIT_ASSERT_EQUAL(aCheckRPN[i].meOp, p->GetOpCode());
            if (aCheckRPN[i].meOp != ocIf )
                CPPUNIT_ASSERT_EQUAL(static_cast<int>(aCheckRPN[i].meType), static_cast<int>(p->GetType()));
        }

        // Generate RPN tokens again, but this time no jump command reordering.
        pCode->DelRPN();
        ScCompiler aCompRPN2(*m_pDoc, ScAddress(), *pCode, FormulaGrammar::GRAM_NATIVE);
        aCompRPN2.EnableJumpCommandReorder(false);
        aCompRPN2.CompileTokenArray();

        static const TokenCheck aCheckRPN2[] =
        {
            { ocPush,  svSingleRef },
            { ocPush,  svDouble    },
            { ocPush,  svString    },
            { ocIf,    svUnknown   }, // type is context dependent, don't test it
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

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaCompilerImplicitIntersection2Param)
{
    struct TestCaseFormula
    {
        OUString  aFormula;
        ScAddress aCellAddress;
        ScRange   aSumRange;
        bool      bStartColRel;  // SumRange-StartCol
        bool      bEndColRel;    // SumRange-EndCol
    };

    m_pDoc->InsertTab(0, u"Formula"_ustr);
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    {
        TestCaseFormula aTestCases[] =
        {
            // Formula, FormulaCellAddress, SumRange with Implicit Intersection

            // Sumrange is single cell, address is abs
            {
                u"=SUMIF($B$2:$B$10;F2;$D$5)"_ustr,
                ScAddress(7, 5, 0),
                ScRange( ScAddress(3, 4, 0), ScAddress(3, 12, 0) ),
                false,
                false
            },

            // Sumrange is single cell, address is relative
            {
                u"=SUMIF($B$2:$B$10;F2;D5)"_ustr,
                ScAddress(7, 5, 0),
                ScRange( ScAddress(3, 4, 0), ScAddress(3, 12, 0) ),
                true,
                true
            },

            // Baserange(abs,abs), Sumrange(abs,abs)
            {
                u"=SUMIF($B$2:$B$10;F2;$D$5:$D$10)"_ustr,
                ScAddress(7, 5, 0),
                ScRange( ScAddress(3, 4, 0), ScAddress(3, 12, 0) ),
                false,
                false
            },

            // Baserange(abs,rel), Sumrange(abs,abs)
            {
                u"=SUMIF($B$2:B10;F2;$D$5:$D$10)"_ustr,
                ScAddress(7, 5, 0),
                ScRange( ScAddress(3, 4, 0), ScAddress(3, 12, 0) ),
                false,
                false
            },

            // Baserange(rel,abs), Sumrange(abs,abs)
            {
                u"=SUMIF(B2:$B$10;F2;$D$5:$D$10)"_ustr,
                ScAddress(7, 5, 0),
                ScRange( ScAddress(3, 4, 0), ScAddress(3, 12, 0) ),
                false,
                false
            },

            // Baserange(rel,rel), Sumrange(abs,abs)
            {
                u"=SUMIF(B2:B10;F2;$D$5:$D$10)"_ustr,
                ScAddress(7, 5, 0),
                ScRange( ScAddress(3, 4, 0), ScAddress(3, 12, 0) ),
                false,
                false
            },

            // Baserange(abs,abs), Sumrange(abs,rel)
            {
                u"=SUMIF($B$2:$B$10;F2;$D$5:D10)"_ustr,
                ScAddress(7, 5, 0),
                ScRange( ScAddress(3, 4, 0), ScAddress(3, 12, 0) ),
                false,
                true
            },

            // Baserange(abs,abs), Sumrange(rel,abs)
            {
                u"=SUMIF($B$2:$B$10;F2;D5:$D$10)"_ustr,
                ScAddress(7, 5, 0),
                ScRange( ScAddress(3, 4, 0), ScAddress(3, 12, 0) ),
                true,
                false
            },

            // Baserange(abs,abs), Sumrange(rel,rel)
            {
                u"=SUMIF($B$2:$B$10;F2;D5:D10)"_ustr,
                ScAddress(7, 5, 0),
                ScRange( ScAddress(3, 4, 0), ScAddress(3, 12, 0) ),
                true,
                true
            }
        };

        for (const auto& rCase : aTestCases)
        {
            m_pDoc->SetString(rCase.aCellAddress, rCase.aFormula);
            const ScFormulaCell* pCell = m_pDoc->GetFormulaCell(rCase.aCellAddress);
            const ScTokenArray* pCode = pCell->GetCode();
            CPPUNIT_ASSERT(pCode);

            sal_uInt16 nLen = pCode->GetCodeLen();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong RPN token count.", static_cast<sal_uInt16>(4), nLen);

            FormulaToken** ppTokens = pCode->GetCode();

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong type of token(first argument to SUMIF)", svDoubleRef, ppTokens[0]->GetType());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong type of token(third argument to SUMIF)", svDoubleRef, ppTokens[2]->GetType());

            ScComplexRefData aSumRangeData = *ppTokens[2]->GetDoubleRef();
            ScRange aSumRange = aSumRangeData.toAbs(*m_pDoc, rCase.aCellAddress);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sum-range in RPN array", rCase.aSumRange, aSumRange);

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong IsRel type for start column address in sum-range", rCase.bStartColRel, aSumRangeData.Ref1.IsColRel());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong IsRel type for end column address in sum-range", rCase.bEndColRel, aSumRangeData.Ref2.IsColRel());
        }
    }
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaCompilerImplicitIntersection1ParamNoChange)
{
    struct TestCaseFormulaNoChange
    {
        OUString  aFormula;
        ScAddress aCellAddress;
        bool      bMatrixFormula;
        bool      bForcedArray;
    };

    m_pDoc->InsertTab(0, u"Formula"_ustr);
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    {
        ScAddress aStartAddr(4, 5, 0);
        TestCaseFormulaNoChange aCasesNoChange[] =
        {
            {
                u"=COS(A$2:A$100)"_ustr,  // No change because of abs col ref.
                aStartAddr,
                false,
                false
            },
            {
                u"=COS($A7:$A100)"_ustr,  // No intersection
                aStartAddr,
                false,
                false
            },
            {
                u"=COS($A5:$C7)"_ustr,   // No intersection 2-D range
                aStartAddr,
                false,
                false
            },
            {
                u"=SUMPRODUCT(COS(A6:A10))"_ustr,  // COS() in forced array mode
                aStartAddr,
                false,
                true
            },
            {
                u"=COS(A6:A10)"_ustr,  // Matrix formula
                aStartAddr,
                true,
                false
            }
        };

        for (const auto& rCase : aCasesNoChange)
        {
            if (rCase.bMatrixFormula)
            {
                ScMarkData aMark(m_pDoc->GetSheetLimits());
                aMark.SelectOneTable(0);
                SCCOL nColStart = rCase.aCellAddress.Col();
                SCROW nRowStart = rCase.aCellAddress.Row();
                m_pDoc->InsertMatrixFormula(nColStart, nRowStart, nColStart, nRowStart + 4,
                                            aMark, rCase.aFormula);
            }
            else
                m_pDoc->SetString(rCase.aCellAddress, rCase.aFormula);

            const ScFormulaCell* pCell = m_pDoc->GetFormulaCell(rCase.aCellAddress);
            const ScTokenArray* pCode = pCell->GetCode();
            CPPUNIT_ASSERT(pCode);

            sal_uInt16 nRPNLen = pCode->GetCodeLen();
            sal_uInt16 nRawLen = pCode->GetLen();
            sal_uInt16 nRawArgPos;
            if (rCase.bForcedArray)
            {
                nRawArgPos = 4;
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong raw token count.", static_cast<sal_uInt16>(7), nRawLen);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong RPN token count.", static_cast<sal_uInt16>(3), nRPNLen);
            }
            else
            {
                nRawArgPos = 2;
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong raw token count.", static_cast<sal_uInt16>(4), nRawLen);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong RPN token count.", static_cast<sal_uInt16>(2), nRPNLen);
            }

            FormulaToken** ppRawTokens = pCode->GetArray();
            FormulaToken** ppRPNTokens = pCode->GetCode();

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong type of raw token(argument to COS)", svDoubleRef, ppRawTokens[nRawArgPos]->GetType());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong type of RPN token(argument to COS)", svDoubleRef, ppRPNTokens[0]->GetType());

            ScComplexRefData aArgRangeRaw = *ppRawTokens[nRawArgPos]->GetDoubleRef();
            ScComplexRefData aArgRangeRPN = *ppRPNTokens[0]->GetDoubleRef();
            bool bRawMatchRPNToken(aArgRangeRaw == aArgRangeRPN);
            CPPUNIT_ASSERT_MESSAGE("raw arg token and RPN arg token contents do not match", bRawMatchRPNToken);
        }
    }
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaCompilerImplicitIntersection1ParamWithChange)
{
    struct TestCaseFormula
    {
        OUString  aFormula;
        ScAddress aCellAddress;
        ScAddress aArgAddr;
    };

    m_pDoc->InsertTab(0, u"Formula"_ustr);
    m_pDoc->InsertTab(1, u"Formula1"_ustr);
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    {
        ScAddress aStartAddr(10, 5, 0);
        TestCaseFormula aCasesWithChange[] =
        {
            {
                u"=COS($A6:$A100)"_ustr,  // Corner case with intersection
                aStartAddr,
                ScAddress(0, 5, 0)
            },
            {
                u"=COS($A2:$A6)"_ustr,    // Corner case with intersection
                aStartAddr,
                ScAddress(0, 5, 0)
            },
            {
                u"=COS($A2:$A100)"_ustr,    // Typical 1D case
                aStartAddr,
                ScAddress(0, 5, 0)
            },
            {
                u"=COS($Formula.$A1:$C3)"_ustr,      // 2D corner case
                ScAddress(0, 0, 1),                      // Formula in sheet 1
                ScAddress(0, 0, 0)
            },
            {
                u"=COS($Formula.$A1:$C3)"_ustr,      // 2D corner case
                ScAddress(0, 2, 1),                      // Formula in sheet 1
                ScAddress(0, 2, 0)
            },
            {
                u"=COS($Formula.$A1:$C3)"_ustr,      // 2D corner case
                ScAddress(2, 0, 1),                      // Formula in sheet 1
                ScAddress(2, 0, 0)
            },
            {
                u"=COS($Formula.$A1:$C3)"_ustr,      // 2D corner case
                ScAddress(2, 2, 1),                      // Formula in sheet 1
                ScAddress(2, 2, 0)
            },
            {
                u"=COS($Formula.$A1:$C3)"_ustr,      // Typical 2D case
                ScAddress(1, 1, 1),                      // Formula in sheet 1
                ScAddress(1, 1, 0)
            }
        };

        for (const auto& rCase : aCasesWithChange)
        {
            m_pDoc->SetString(rCase.aCellAddress, rCase.aFormula);

            const ScFormulaCell* pCell = m_pDoc->GetFormulaCell(rCase.aCellAddress);
            const ScTokenArray* pCode = pCell->GetCode();
            CPPUNIT_ASSERT(pCode);

            sal_uInt16 nRPNLen = pCode->GetCodeLen();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong RPN token count.", static_cast<sal_uInt16>(2), nRPNLen);

            FormulaToken** ppRPNTokens = pCode->GetCode();

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong type of RPN token(argument to COS)", svSingleRef, ppRPNTokens[0]->GetType());

            ScSingleRefData aArgAddrRPN = *ppRPNTokens[0]->GetSingleRef();
            ScAddress aArgAddrActual = aArgAddrRPN.toAbs(*m_pDoc, rCase.aCellAddress);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Computed implicit intersection singleref is wrong", rCase.aArgAddr, aArgAddrActual);
        }
    }
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaCompilerImplicitIntersection1NoGroup)
{
    m_pDoc->InsertTab(0, u"Formula"_ustr);
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->SetString(ScAddress(1,2,0), u"=COS(A1:A5)"_ustr); // B3
    m_pDoc->SetString(ScAddress(1,3,0), u"=COS(A1:A5)"_ustr); // B4

    // Implicit intersection optimization in ScCompiler::HandleIIOpCode() internally changes
    // these to "=COS(A3)" and "=COS(A4)", but these shouldn't be merged into a formula group,
    // otherwise B4's formula would then be "=COS(A2:A6)".
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula in B3 has changed.", u"=COS(A1:A5)"_ustr, m_pDoc->GetFormula(1,2,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula in B4 has changed.", u"=COS(A1:A5)"_ustr, m_pDoc->GetFormula(1,3,0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaCompilerImplicitIntersectionOperators)
{
    struct TestCase
    {
        OUString formula[3];
        double result[3];
    };

    m_pDoc->InsertTab(0, u"Test"_ustr);
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->SetValue(2, 0, 0, 5); // C1
    m_pDoc->SetValue(2, 1, 0, 4); // C2
    m_pDoc->SetValue(2, 2, 0, 3); // C3
    m_pDoc->SetValue(3, 0, 0, 1); // D1
    m_pDoc->SetValue(3, 1, 0, 2); // D2
    m_pDoc->SetValue(3, 2, 0, 3); // D3

    TestCase tests[] =
    {
        { u"=C:C/D:D"_ustr, u"=C:C/D:D"_ustr, u"=C:C/D:D"_ustr, 5, 2, 1 },
        { u"=C1:C2/D1:D2"_ustr, u"=C2:C3/D2:D3"_ustr, u"=C3:C4/D3:D4"_ustr, 5, 2, 1 }
    };

    for (const TestCase& test : tests)
    {
        for(int i = 0; i < 2; ++i )
            m_pDoc->SetString(ScAddress(4,i,0), test.formula[i]); // E1-3
        for(int i = 0; i < 2; ++i )
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OUString( test.formula[i] + " result incorrect in row " + OUString::number(i+1)).toUtf8().getStr(),
                test.result[i], m_pDoc->GetValue(ScAddress(4,i,0)));
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaAnnotateTrimOnDoubleRefs)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    constexpr sal_Int32 nCols = 2;
    constexpr sal_Int32 nRows = 5;

    // Values in A1:B5
    constexpr sal_Int32 aMat[nRows][nCols] = {
        {4, 50},
        {5, 30},
        {4, 40},
        {0, 70},
        {5, 90}
    };

    for (sal_Int32 nCol = 0; nCol < nCols; ++nCol)
    {
        for (sal_Int32 nRow = 0; nRow < nRows; ++nRow)
            m_pDoc->SetValue(nCol, nRow, 0, aMat[nRow][nCol]);
    }

    m_pDoc->SetValue(2, 0, 0, 4); // C1 = 4
    m_pDoc->SetValue(3, 0, 0, 5); // D1 = 5

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);

    struct TestCase
    {
        OUString aFormula;
        ScRange aTrimmableRange;
        double fResult;
        bool bMatrixFormula;
    };

    constexpr sal_Int32 nTestCases = 5;
    TestCase aTestCases[nTestCases] = {
        {
            u"=SUM(IF($C$1=A:A;B:B)/10*D1)"_ustr,
            ScRange(0, 0, 0, 0, 1048575, 0),
            45.0,
            true
        },

        {
            u"=SUM(IF(A:A=5;B:B)/10*D1)"_ustr,
            ScRange(0, 0, 0, 0, 1048575, 0),
            60.0,
            true
        },

        {
            u"=SUM(IF($C$1=A:A;B:B;B:B)/10*D1)"_ustr,  // IF has else clause
            ScRange(-1, -1, -1, -1, -1, -1),     // Has no trimmable double-ref.
            140.0,
            true
        },

        {
            u"=SUM(IF($C$1=A:A;B:B)/10*D1)"_ustr,
            ScRange(-1, -1, -1, -1, -1, -1),     // Has no trimmable double-ref.
            25,
            false                                // Not in matrix mode.
        },

        {
            u"=SUMPRODUCT(A:A=$C$1; 1-(A:A=$C$1))"_ustr,
            ScRange(-1, -1, -1, -1, -1, -1),     // Has no trimmable double-ref.
            0.0,
            false                                // Not in matrix mode.
        },
    };

    for (sal_Int32 nTestIdx = 0; nTestIdx < nTestCases; ++nTestIdx)
    {
        TestCase& rTestCase = aTestCases[nTestIdx];
        if (rTestCase.bMatrixFormula)
            m_pDoc->InsertMatrixFormula(4, 0, 4, 0, aMark, rTestCase.aFormula); // Formula in E1
        else
            m_pDoc->SetString(ScAddress(4, 0, 0), rTestCase.aFormula);          // Formula in E1

        std::string aMsgStart = "TestCase#" + std::to_string(nTestIdx + 1) + " : ";
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsgStart + "Incorrect formula result", rTestCase.fResult, m_pDoc->GetValue(ScAddress(4, 0, 0)));

        ScFormulaCell* pCell = m_pDoc->GetFormulaCell(ScAddress(4, 0, 0));
        ScTokenArray* pCode = pCell->GetCode();
        sal_Int32 nLen = pCode->GetCodeLen();
        FormulaToken** pRPNArray = pCode->GetCode();

        for (sal_Int32 nIdx = 0; nIdx < nLen; ++nIdx)
        {
            FormulaToken* pTok = pRPNArray[nIdx];
            if (pTok && pTok->GetType() == svDoubleRef)
            {
                ScRange aRange = pTok->GetDoubleRef()->toAbs(*m_pDoc, ScAddress(4, 0, 0));
                if (aRange == rTestCase.aTrimmableRange)
                    CPPUNIT_ASSERT_MESSAGE(aMsgStart + "Double ref is incorrectly flagged as not trimmable to data",
                        pTok->GetDoubleRef()->IsTrimToData());
                else
                    CPPUNIT_ASSERT_MESSAGE(aMsgStart + "Double ref is incorrectly flagged as trimmable to data",
                        !pTok->GetDoubleRef()->IsTrimToData());
            }
        }
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdate)
{
    m_pDoc->InsertTab(0, u"Formula"_ustr);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->SetValue(ScAddress(0,0,0), 2.0); // A1
    m_pDoc->SetString(ScAddress(2,2,0), u"=A1"_ustr);   // C3
    m_pDoc->SetString(ScAddress(2,3,0), u"=$A$1"_ustr); // C4

    ScAddress aPos(2,2,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C3.", u"=A1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(2,3,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C4.", u"=$A$1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Delete row 2 to push formula cells up (to C2:C3).
    m_pDoc->DeleteRow(ScRange(0,1,0,m_pDoc->MaxCol(),1,0));

    aPos = ScAddress(2,1,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C2.", u"=A1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(2,2,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C3.", u"=$A$1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Insert one row at row 2 to move them back.
    m_pDoc->InsertRow(ScRange(0,1,0,m_pDoc->MaxCol(),1,0));

    aPos = ScAddress(2,2,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C3.", u"=A1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(2,3,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C4.", u"=$A$1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Insert 2 rows at row 1 to shift all of A1 and C3:C4 down.
    m_pDoc->InsertRow(ScRange(0,0,0,m_pDoc->MaxCol(),1,0));

    aPos = ScAddress(2,4,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C5.", u"=A3"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(2,5,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C6.", u"=$A$3"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Delete 2 rows at row 1 to shift them back.
    m_pDoc->DeleteRow(ScRange(0,0,0,m_pDoc->MaxCol(),1,0));

    aPos = ScAddress(2,2,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C3.", u"=A1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(2,3,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C4.", u"=$A$1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Insert 3 columns at column B. to shift C3:C4 to F3:F4.
    m_pDoc->InsertCol(ScRange(1,0,0,3,m_pDoc->MaxRow(),0));

    aPos = ScAddress(5,2,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in F3.", u"=A1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(5,3,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in F4.", u"=$A$1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Delete columns B:D to shift them back.
    m_pDoc->DeleteCol(ScRange(1,0,0,3,m_pDoc->MaxRow(),0));

    aPos = ScAddress(2,2,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C3.", u"=A1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(2,3,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C4.", u"=$A$1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Insert cells over A1:A3 to only shift A1 down to A4.
    m_pDoc->InsertRow(ScRange(0,0,0,0,2,0));

    aPos = ScAddress(2,2,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C3.", u"=A4"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(2,3,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C4.", u"=$A$4"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // .. and back.
    m_pDoc->DeleteRow(ScRange(0,0,0,0,2,0));

    aPos = ScAddress(2,2,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C3.", u"=A1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(2,3,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in C4.", u"=$A$1"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Delete row 1 which will delete the value cell (A1).
    m_pDoc->DeleteRow(ScRange(0,0,0,m_pDoc->MaxCol(),0,0));

    aPos = ScAddress(2,1,0);
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(int(FormulaError::NoRef), static_cast<int>(pFC->GetErrCode()));
    aPos = ScAddress(2,2,0);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(int(FormulaError::NoRef), static_cast<int>(pFC->GetErrCode()));

    // Clear all and start over.
    clearRange(m_pDoc, ScRange(0,0,0,10,10,0));

    // Test range updates

    // Fill B2:C3 with values.
    m_pDoc->SetValue(ScAddress(1,1,0), 1);
    m_pDoc->SetValue(ScAddress(1,2,0), 2);
    m_pDoc->SetValue(ScAddress(2,1,0), 3);
    m_pDoc->SetValue(ScAddress(2,2,0), 4);

    m_pDoc->SetString(ScAddress(0,5,0), u"=SUM(B2:C3)"_ustr);
    m_pDoc->SetString(ScAddress(0,6,0), u"=SUM($B$2:$C$3)"_ustr);

    aPos = ScAddress(0,5,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM(B2:C3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(0,6,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A7.", u"=SUM($B$2:$C$3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Insert a row at row 1.
    m_pDoc->InsertRow(ScRange(0,0,0,m_pDoc->MaxCol(),0,0));

    aPos = ScAddress(0,6,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A7.", u"=SUM(B3:C4)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(0,7,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A8.", u"=SUM($B$3:$C$4)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // ... and back.
    m_pDoc->DeleteRow(ScRange(0,0,0,m_pDoc->MaxCol(),0,0));

    aPos = ScAddress(0,5,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM(B2:C3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(0,6,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A7.", u"=SUM($B$2:$C$3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Insert columns B:C to shift only the value range.
    m_pDoc->InsertCol(ScRange(1,0,0,2,m_pDoc->MaxRow(),0));

    aPos = ScAddress(0,5,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM(D2:E3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(0,6,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A7.", u"=SUM($D$2:$E$3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // ... and back.
    m_pDoc->DeleteCol(ScRange(1,0,0,2,m_pDoc->MaxRow(),0));

    aPos = ScAddress(0,5,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM(B2:C3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(0,6,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A7.", u"=SUM($B$2:$C$3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Insert rows 5:6 to shift the formula cells only.
    m_pDoc->InsertRow(ScRange(0,4,0,m_pDoc->MaxCol(),5,0));

    aPos = ScAddress(0,7,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A8.", u"=SUM(B2:C3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(0,8,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A9.", u"=SUM($B$2:$C$3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // ... and back.
    m_pDoc->DeleteRow(ScRange(0,4,0,m_pDoc->MaxCol(),5,0));

    aPos = ScAddress(0,5,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM(B2:C3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    aPos = ScAddress(0,6,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A7.", u"=SUM($B$2:$C$3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

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
    m_pDoc->DeleteRow(ScRange(0,1,0,m_pDoc->MaxCol(),2,0));

    // Both A4 and A5 should show #REF! errors.
    pFC = m_pDoc->GetFormulaCell(ScAddress(0,3,0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(int(FormulaError::NoRef), static_cast<int>(pFC->GetErrCode()));

    pFC = m_pDoc->GetFormulaCell(ScAddress(0,4,0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(int(FormulaError::NoRef), static_cast<int>(pFC->GetErrCode()));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateRange)
{
    m_pDoc->InsertTab(0, u"Formula"_ustr);

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
    m_pDoc->SetString(ScAddress(0,6,0), u"=SUM(B2:C5)"_ustr);
    m_pDoc->SetString(ScAddress(0,7,0), u"=SUM($B$2:$C$5)"_ustr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A7.", u"=SUM(B2:C5)"_ustr, m_pDoc->GetFormula(0,6,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A8.", u"=SUM($B$2:$C$5)"_ustr, m_pDoc->GetFormula(0,7,0));

    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,7,0)));

    // Delete row 3. This should shrink the range references by one row.
    m_pDoc->DeleteRow(ScRange(0,2,0,m_pDoc->MaxCol(),2,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM(B2:C4)"_ustr, m_pDoc->GetFormula(0,5,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A7.", u"=SUM($B$2:$C$4)"_ustr, m_pDoc->GetFormula(0,6,0));

    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,6,0)));

    // Delete row 4 - bottom of range
    m_pDoc->DeleteRow(ScRange(0,3,0,m_pDoc->MaxCol(),3,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A5.", u"=SUM(B2:C3)"_ustr, m_pDoc->GetFormula(0,4,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM($B$2:$C$3)"_ustr, m_pDoc->GetFormula(0,5,0));

    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Delete row 2 - top of range
    m_pDoc->DeleteRow(ScRange(0,1,0,m_pDoc->MaxCol(),1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A4.", u"=SUM(B2:C2)"_ustr, m_pDoc->GetFormula(0,3,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A5.", u"=SUM($B$2:$C$2)"_ustr, m_pDoc->GetFormula(0,4,0));

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
    m_pDoc->SetString(ScAddress(0,1,0), u"=SUM(C2:F3)"_ustr);
    m_pDoc->SetString(ScAddress(0,2,0), u"=SUM($C$2:$F$3)"_ustr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A2.", u"=SUM(C2:F3)"_ustr, m_pDoc->GetFormula(0,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A3.", u"=SUM($C$2:$F$3)"_ustr, m_pDoc->GetFormula(0,2,0));

    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Delete column D.
    m_pDoc->DeleteCol(ScRange(3,0,0,3,m_pDoc->MaxRow(),0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A2.", u"=SUM(C2:E3)"_ustr, m_pDoc->GetFormula(0,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A3.", u"=SUM($C$2:$E$3)"_ustr, m_pDoc->GetFormula(0,2,0));

    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Delete column E - the right edge of reference range.
    m_pDoc->DeleteCol(ScRange(4,0,0,4,m_pDoc->MaxRow(),0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A2.", u"=SUM(C2:D3)"_ustr, m_pDoc->GetFormula(0,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A3.", u"=SUM($C$2:$D$3)"_ustr, m_pDoc->GetFormula(0,2,0));

    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(16.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Delete column C - the left edge of reference range.
    m_pDoc->DeleteCol(ScRange(2,0,0,2,m_pDoc->MaxRow(),0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A2.", u"=SUM(C2:C3)"_ustr, m_pDoc->GetFormula(0,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A3.", u"=SUM($C$2:$C$3)"_ustr, m_pDoc->GetFormula(0,2,0));

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
    m_pDoc->SetString(ScAddress(0,4,0), u"=SUM(C2:D3)"_ustr);
    m_pDoc->SetString(ScAddress(0,5,0), u"=SUM($C$2:$D$3)"_ustr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A5.", u"=SUM(C2:D3)"_ustr, m_pDoc->GetFormula(0,4,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM($C$2:$D$3)"_ustr, m_pDoc->GetFormula(0,5,0));

    // Insert a column at column C. This should simply shift the reference without expansion.
    m_pDoc->InsertCol(ScRange(2,0,0,2,m_pDoc->MaxRow(),0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A5.", u"=SUM(D2:E3)"_ustr, m_pDoc->GetFormula(0,4,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM($D$2:$E$3)"_ustr, m_pDoc->GetFormula(0,5,0));

    // Shift it back.
    m_pDoc->DeleteCol(ScRange(2,0,0,2,m_pDoc->MaxRow(),0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A5.", u"=SUM(C2:D3)"_ustr, m_pDoc->GetFormula(0,4,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM($C$2:$D$3)"_ustr, m_pDoc->GetFormula(0,5,0));

    // Insert at column D. This should expand the reference by one column length.
    m_pDoc->InsertCol(ScRange(3,0,0,3,m_pDoc->MaxRow(),0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A5.", u"=SUM(C2:E3)"_ustr, m_pDoc->GetFormula(0,4,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM($C$2:$E$3)"_ustr, m_pDoc->GetFormula(0,5,0));

    // Insert at column F. No expansion should occur since the edge expansion is turned off.
    m_pDoc->InsertCol(ScRange(5,0,0,5,m_pDoc->MaxRow(),0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A5.", u"=SUM(C2:E3)"_ustr, m_pDoc->GetFormula(0,4,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM($C$2:$E$3)"_ustr, m_pDoc->GetFormula(0,5,0));

    // Insert at row 2. No expansion should occur with edge expansion turned off.
    m_pDoc->InsertRow(ScRange(0,1,0,m_pDoc->MaxCol(),1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A6.", u"=SUM(C3:E4)"_ustr, m_pDoc->GetFormula(0,5,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A7.", u"=SUM($C$3:$E$4)"_ustr, m_pDoc->GetFormula(0,6,0));

    // Insert at row 4 to expand the reference range.
    m_pDoc->InsertRow(ScRange(0,3,0,m_pDoc->MaxCol(),3,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A7.", u"=SUM(C3:E5)"_ustr, m_pDoc->GetFormula(0,6,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A8.", u"=SUM($C$3:$E$5)"_ustr, m_pDoc->GetFormula(0,7,0));

    // Insert at row 6. No expansion with edge expansion turned off.
    m_pDoc->InsertRow(ScRange(0,5,0,m_pDoc->MaxCol(),5,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A8.", u"=SUM(C3:E5)"_ustr, m_pDoc->GetFormula(0,7,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A9.", u"=SUM($C$3:$E$5)"_ustr, m_pDoc->GetFormula(0,8,0));

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
    m_pDoc->SetString(ScAddress(0,1,0), u"=SUM(C6:D7)"_ustr);
    m_pDoc->SetString(ScAddress(0,2,0), u"=SUM($C$6:$D$7)"_ustr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A2.", u"=SUM(C6:D7)"_ustr, m_pDoc->GetFormula(0,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A3.", u"=SUM($C$6:$D$7)"_ustr, m_pDoc->GetFormula(0,2,0));

    // Insert at column E. This should expand the reference range by one column.
    m_pDoc->InsertCol(ScRange(4,0,0,4,m_pDoc->MaxRow(),0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A2.", u"=SUM(C6:E7)"_ustr, m_pDoc->GetFormula(0,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A3.", u"=SUM($C$6:$E$7)"_ustr, m_pDoc->GetFormula(0,2,0));

    // Insert at column C to edge-expand the reference range.
    m_pDoc->InsertCol(ScRange(2,0,0,2,m_pDoc->MaxRow(),0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A2.", u"=SUM(C6:F7)"_ustr, m_pDoc->GetFormula(0,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A3.", u"=SUM($C$6:$F$7)"_ustr, m_pDoc->GetFormula(0,2,0));

    // Insert at row 8 to edge-expand.
    m_pDoc->InsertRow(ScRange(0,7,0,m_pDoc->MaxCol(),7,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A2.", u"=SUM(C6:F8)"_ustr, m_pDoc->GetFormula(0,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A3.", u"=SUM($C$6:$F$8)"_ustr, m_pDoc->GetFormula(0,2,0));

    // Insert at row 6 to edge-expand.
    m_pDoc->InsertRow(ScRange(0,5,0,m_pDoc->MaxCol(),5,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A2.", u"=SUM(C6:F9)"_ustr, m_pDoc->GetFormula(0,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in A3.", u"=SUM($C$6:$F$9)"_ustr, m_pDoc->GetFormula(0,2,0));

    m_pDoc->InsertTab(1, u"StickyRange"_ustr);

    // A3:A18 all possible combinations of relative and absolute addressing,
    // leaving one row above and below unreferenced.
    ScAddress aPos(0,2,1);
    m_pDoc->SetString( aPos, u"=B2:B1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=B2:B$1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=B2:$B1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=B2:$B$1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=B$2:B1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=B$2:B$1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=B$2:$B1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=B$2:$B$1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=$B2:B1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=$B2:B$1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=$B2:$B1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=$B2:$B$1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=$B$2:B1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=$B$2:B$1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=$B$2:$B1048575"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=$B$2:$B$1048575"_ustr);
    aPos.IncRow();
    // A19 reference to two cells on one row.
    m_pDoc->SetString( aPos, u"=B1048575:C1048575"_ustr);
    aPos.IncRow();

    // Insert 2 rows in the middle to shift bottom reference down and make it
    // sticky.
    m_pDoc->InsertRow( ScRange( 0, aPos.Row(), 1, m_pDoc->MaxCol(), aPos.Row()+1, 1));

    // A3:A18 must not result in #REF! anywhere.
    aPos.Set(0,2,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A3 after insertion.", u"=B2:B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A4 after insertion.", u"=B2:B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A5 after insertion.", u"=B2:$B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A6 after insertion.", u"=B2:$B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A7 after insertion.", u"=B$2:B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A8 after insertion.", u"=B$2:B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A9 after insertion.", u"=B$2:$B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A10 after insertion.", u"=B$2:$B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A11 after insertion.", u"=$B2:B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A12 after insertion.", u"=$B2:B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A13 after insertion.", u"=$B2:$B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A14 after insertion.", u"=$B2:$B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A15 after insertion.", u"=$B$2:B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A16 after insertion.", u"=$B$2:B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A17 after insertion.", u"=$B$2:$B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A18 after insertion.", u"=$B$2:$B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();

    // A19 reference to one row shifted out should be #REF!
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A19 after insertion.", u"=B#REF!:C#REF!"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    // A19 enter reference to last row.
    m_pDoc->SetString( aPos, u"=B1048576:C1048576"_ustr);
    aPos.IncRow();

    // Delete row 1 to shift top reference up, bottom reference stays sticky.
    m_pDoc->DeleteRow(ScRange(0,0,1,m_pDoc->MaxCol(),0,1));

    // Check sticky bottom references and display of entire column references,
    // now in A2:A17.
    aPos.Set(0,1,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A2 after deletion.", u"=B:B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A3 after deletion.", u"=B1:B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A4 after deletion.", u"=B:$B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A5 after deletion.", u"=B1:$B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A6 after deletion.", u"=B$1:B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A7 after deletion.", u"=B:B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A8 after deletion.", u"=B$1:$B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A9 after deletion.", u"=B:$B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A10 after deletion.", u"=$B:B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A11 after deletion.", u"=$B1:B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A12 after deletion.", u"=$B:$B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A13 after deletion.", u"=$B1:$B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A14 after deletion.", u"=$B$1:B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A15 after deletion.", u"=$B:B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A16 after deletion.", u"=$B$1:$B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A17 after deletion.", u"=$B:$B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();

    // A18 reference to one last row should be shifted up.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A18 after deletion.", u"=B1048575:C1048575"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();

    // Insert 4 rows in the middle.
    m_pDoc->InsertRow( ScRange( 0, aPos.Row(), 1, m_pDoc->MaxCol(), aPos.Row()+3, 1));
    // Delete 2 rows in the middle.
    m_pDoc->DeleteRow( ScRange( 0, aPos.Row(), 1, m_pDoc->MaxCol(), aPos.Row()+1, 1));

    // References in A2:A17 must still be the same.
    aPos.Set(0,1,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A2 after deletion.", u"=B:B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A3 after deletion.", u"=B1:B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A4 after deletion.", u"=B:$B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A5 after deletion.", u"=B1:$B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A6 after deletion.", u"=B$1:B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A7 after deletion.", u"=B:B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A8 after deletion.", u"=B$1:$B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A9 after deletion.", u"=B:$B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A10 after deletion.", u"=$B:B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A11 after deletion.", u"=$B1:B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A12 after deletion.", u"=$B:$B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A13 after deletion.", u"=$B1:$B$1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A14 after deletion.", u"=$B$1:B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A15 after deletion.", u"=$B:B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A16 after deletion.", u"=$B$1:$B1048576"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference in A17 after deletion.", u"=$B:$B"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    aPos.IncRow();

    // Enter values in B1 and B1048576/B16777216 (last row).
    m_pDoc->SetValue( 1,0,1, 1.0);
    m_pDoc->SetValue( 1,m_pDoc->MaxRow(),1, 2.0);
    // Sticky reference including last row.
    m_pDoc->SetString( 2,0,1, u"=SUM(B:B)"_ustr);
    // Reference to last row.
    CPPUNIT_ASSERT_MESSAGE("m_pDoc->MaxRow() changed, adapt unit test.",
        m_pDoc->MaxRow() == 1048575 || m_pDoc->MaxRow() == 16777215);
    m_pDoc->SetString( 2,1,1, m_pDoc->MaxRow() == 1048575 ? u"=SUM(B1048576:C1048576)"_ustr
                                                          : u"=SUM(B16777216:C16777216)"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result in C1.", 3.0, m_pDoc->GetValue(2,0,1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result in C2.", 2.0, m_pDoc->GetValue(2,1,1));
    // Delete last row.
    m_pDoc->DeleteRow( ScRange( 0, m_pDoc->MaxRow(), 1, m_pDoc->MaxCol(), m_pDoc->MaxRow(), 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result in C1.", 1.0, m_pDoc->GetValue(2,0,1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Reference in C2 not invalidated.", u"#REF!"_ustr, m_pDoc->GetString(2,1,1));

    // Enter values in A23 and AMJ23/XFD23 (last column).
    m_pDoc->SetValue( 0,22,1, 1.0);
    m_pDoc->SetValue( m_pDoc->MaxCol(),22,1, 2.0);
    // C3 with sticky reference including last column.
    m_pDoc->SetString( 2,2,1, u"=SUM(23:23)"_ustr);
    // C4 with reference to last column.
    m_pDoc->SetString( 2,3,1, "=SUM(" + m_pDoc->MaxColAsString() + "22:" + m_pDoc->MaxColAsString() + "23)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result in C3.", 3.0, m_pDoc->GetValue(2,2,1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result in C4.", 2.0, m_pDoc->GetValue(2,3,1));
    // Delete last column.
    m_pDoc->DeleteCol( ScRange( m_pDoc->MaxCol(), 0, 1, m_pDoc->MaxCol(), m_pDoc->MaxRow(), 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result in C3.", 1.0, m_pDoc->GetValue(2,2,1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Reference in C4 not invalidated.", u"#REF!"_ustr, m_pDoc->GetString(2,3,1));

    m_pDoc->DeleteTab(1);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateSheets)
{
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);
    m_pDoc->InsertTab(1, u"Sheet2"_ustr);

    OUString aName;
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(u"Sheet1"_ustr, aName);
    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(u"Sheet2"_ustr, aName);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    // Set values to B2:C3 on sheet Sheet1.
    m_pDoc->SetValue(ScAddress(1,1,0), 1);
    m_pDoc->SetValue(ScAddress(1,2,0), 2);
    m_pDoc->SetValue(ScAddress(2,1,0), 3);
    m_pDoc->SetValue(ScAddress(2,2,0), 4);

    // Set formulas to B2 and B3 on sheet Sheet2.
    m_pDoc->SetString(ScAddress(1,1,1), u"=SUM(Sheet1.B2:C3)"_ustr);
    m_pDoc->SetString(ScAddress(1,2,1), u"=SUM($Sheet1.$B$2:$C$3)"_ustr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(Sheet1.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($Sheet1.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,1));

    // Swap the sheets.
    m_pDoc->MoveTab(0, 1);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(u"Sheet2"_ustr, aName);
    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(u"Sheet1"_ustr, aName);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(Sheet1.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($Sheet1.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,0));

    // Swap back.
    m_pDoc->MoveTab(0, 1);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(u"Sheet1"_ustr, aName);
    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(u"Sheet2"_ustr, aName);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(Sheet1.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($Sheet1.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,1));

    // Insert a new sheet between the two.
    m_pDoc->InsertTab(1, u"Temp"_ustr);

    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(u"Temp"_ustr, aName);
    m_pDoc->GetName(2, aName);
    CPPUNIT_ASSERT_EQUAL(u"Sheet2"_ustr, aName);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(Sheet1.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,2));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($Sheet1.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,2));

    // Move the last sheet (Sheet2) to the first position.
    m_pDoc->MoveTab(2, 0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(Sheet1.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($Sheet1.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,0));

    // Move back.
    m_pDoc->MoveTab(0, 2);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(Sheet1.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,2));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($Sheet1.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,2));

    // Move the "Temp" sheet to the last position.
    m_pDoc->MoveTab(1, 2);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(Sheet1.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($Sheet1.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,1));

    // Move back.
    m_pDoc->MoveTab(2, 1);

    // Delete the temporary sheet.
    m_pDoc->DeleteTab(1);

    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(u"Sheet2"_ustr, aName);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(Sheet1.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($Sheet1.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,1));

    // Insert a new sheet before the first one.
    m_pDoc->InsertTab(0, u"Temp"_ustr);

    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(u"Sheet1"_ustr, aName);
    m_pDoc->GetName(2, aName);
    CPPUNIT_ASSERT_EQUAL(u"Sheet2"_ustr, aName);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(Sheet1.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,2));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($Sheet1.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,2));

    // Delete the temporary sheet.
    m_pDoc->DeleteTab(0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(Sheet1.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($Sheet1.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,1));

    // Append a bunch of sheets.
    m_pDoc->InsertTab(2, u"Temp1"_ustr);
    m_pDoc->InsertTab(3, u"Temp2"_ustr);
    m_pDoc->InsertTab(4, u"Temp3"_ustr);

    // Move these tabs around. This shouldn't affects the first 2 sheets.
    m_pDoc->MoveTab(2, 4);
    m_pDoc->MoveTab(3, 2);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(Sheet1.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($Sheet1.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,1));

    // Delete the temp sheets.
    m_pDoc->DeleteTab(4);
    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);

    // Delete Sheet1.
    m_pDoc->DeleteTab(0);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(u"Sheet2"_ustr, aName);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B2.", u"=SUM(#REF!.B2:C3)"_ustr, m_pDoc->GetFormula(1,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in Sheet2.B3.", u"=SUM($#REF!.$B$2:$C$3)"_ustr, m_pDoc->GetFormula(1,2,0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateInsertRows)
{
    setExpandRefs(false);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, u"Formula"_ustr);

    // Insert raw values in B2:B4.
    m_pDoc->SetValue(ScAddress(1,1,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,2,0), 2.0);
    m_pDoc->SetValue(ScAddress(1,3,0), 3.0);

    // Insert a formula in B5 to sum up B2:B4.
    m_pDoc->SetString(ScAddress(1,4,0), u"=SUM(B2:B4)"_ustr);

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    // Insert rows over rows 1:2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    rFunc.InsertCells(ScRange(0,0,0,m_pDoc->MaxCol(),1,0), &aMark, INS_INSROWS_BEFORE, false, true);

    // The raw data should have shifted to B4:B6.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,4,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,5,0)));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(B4:B6)"_ustr, m_pDoc->GetFormula(1,6,0));

    // Clear and start over.
    clearSheet(m_pDoc, 0);

    // Set raw values in A4:A6.
    m_pDoc->SetValue(ScAddress(0,3,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,4,0), 2.0);
    m_pDoc->SetValue(ScAddress(0,5,0), 3.0);

    // Set formula in A3 to reference A4:A6.
    m_pDoc->SetString(ScAddress(0,2,0), u"=MAX(A4:A6)"_ustr);

    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Insert 3 rows over 2:4.  This should push A3:A6 to A6:A9.
    rFunc.InsertCells(ScRange(0,1,0,m_pDoc->MaxCol(),3,0), &aMark, INS_INSROWS_BEFORE, false, true);
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0,5,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula cell should not be an error.", 0, static_cast<int>(pFC->GetErrCode()));
    ASSERT_DOUBLES_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=MAX(A7:A9)"_ustr, m_pDoc->GetFormula(0,5,0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateSheetsDelete)
{
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);
    m_pDoc->InsertTab(1, u"Sheet2"_ustr);
    m_pDoc->InsertTab(2, u"Sheet3"_ustr);
    m_pDoc->InsertTab(3, u"Sheet4"_ustr);

    m_pDoc->SetString(ScAddress(4,1,0), u"=SUM(Sheet2.A4:Sheet4.A4)"_ustr);
    m_pDoc->SetString(ScAddress(4,2,0), u"=SUM($Sheet2.A4:$Sheet4.A4)"_ustr);
    m_pDoc->DeleteTab(1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM(Sheet3.A4:Sheet4.A4)"_ustr, m_pDoc->GetFormula(4,1,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM($Sheet3.A4:$Sheet4.A4)"_ustr, m_pDoc->GetFormula(4,2,0));

    m_pDoc->InsertTab(1, u"Sheet2"_ustr);

    m_pDoc->SetString(ScAddress(5,1,3), u"=SUM(Sheet1.A5:Sheet3.A5)"_ustr);
    m_pDoc->SetString(ScAddress(5,2,3), u"=SUM($Sheet1.A5:$Sheet3.A5)"_ustr);
    m_pDoc->DeleteTab(2);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM(Sheet1.A5:Sheet2.A5)"_ustr, m_pDoc->GetFormula(5,1,2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM($Sheet1.A5:$Sheet2.A5)"_ustr, m_pDoc->GetFormula(5,2,2));

    m_pDoc->InsertTab(2, u"Sheet3"_ustr);

    m_pDoc->SetString(ScAddress(6,1,3), u"=SUM(Sheet1.A6:Sheet3.A6)"_ustr);
    m_pDoc->SetString(ScAddress(6,2,3), u"=SUM($Sheet1.A6:$Sheet3.A6)"_ustr);
    m_pDoc->DeleteTabs(0,3);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM(#REF!.A6:#REF!.A6)"_ustr, m_pDoc->GetFormula(6,1,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM($#REF!.A6:$#REF!.A6)"_ustr, m_pDoc->GetFormula(6,2,0));

    m_pDoc->InsertTab(0, u"Sheet1"_ustr);
    m_pDoc->InsertTab(1, u"Sheet2"_ustr);
    m_pDoc->InsertTab(2, u"Sheet3"_ustr);

    m_pDoc->SetString(ScAddress(1,1,1), u"=SUM(Sheet1.A2:Sheet3.A2"_ustr);
    m_pDoc->SetString(ScAddress(2,1,1), u"=SUM(Sheet1.A1:Sheet2.A1"_ustr);
    m_pDoc->SetString(ScAddress(3,1,1), u"=SUM(Sheet2.A3:Sheet4.A3"_ustr);

    m_pDoc->SetString(ScAddress(1,2,1), u"=SUM($Sheet1.A2:$Sheet3.A2"_ustr);
    m_pDoc->SetString(ScAddress(2,2,1), u"=SUM($Sheet1.A1:$Sheet2.A1"_ustr);
    m_pDoc->SetString(ScAddress(3,2,1), u"=SUM($Sheet2.A3:$Sheet4.A3"_ustr);

    m_pDoc->DeleteTab(2);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM(Sheet1.A2:Sheet2.A2)"_ustr, m_pDoc->GetFormula(1,1,1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM(Sheet1.A1:Sheet2.A1)"_ustr, m_pDoc->GetFormula(2,1,1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM(Sheet2.A3:Sheet4.A3)"_ustr, m_pDoc->GetFormula(3,1,1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM($Sheet1.A2:$Sheet2.A2)"_ustr, m_pDoc->GetFormula(1,2,1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM($Sheet1.A1:$Sheet2.A1)"_ustr, m_pDoc->GetFormula(2,2,1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM($Sheet2.A3:$Sheet4.A3)"_ustr, m_pDoc->GetFormula(3,2,1));

    m_pDoc->DeleteTab(0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM(Sheet2.A2:Sheet2.A2)"_ustr, m_pDoc->GetFormula(1,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM(Sheet2.A1:Sheet2.A1)"_ustr, m_pDoc->GetFormula(2,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM(Sheet2.A3:Sheet4.A3)"_ustr, m_pDoc->GetFormula(3,1,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM($Sheet2.A2:$Sheet2.A2)"_ustr, m_pDoc->GetFormula(1,2,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM($Sheet2.A1:$Sheet2.A1)"_ustr, m_pDoc->GetFormula(2,2,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Formula", u"=SUM($Sheet2.A3:$Sheet4.A3)"_ustr, m_pDoc->GetFormula(3,2,0));

    m_pDoc->DeleteTab(0);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateInsertColumns)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    setExpandRefs(false);

    m_pDoc->InsertTab(0, u"Formula"_ustr);

    // Set named range for B2 with absolute column and relative same row.
    const ScAddress aNamePos(0,1,0);
    bool bInserted = m_pDoc->InsertNewRangeName(u"RowRelativeRange"_ustr, aNamePos, u"$Formula.$B2"_ustr);
    CPPUNIT_ASSERT(bInserted);

    // Set named range for entire absolute column B.
    bInserted = m_pDoc->InsertNewRangeName(u"EntireColumn"_ustr, aNamePos, u"$B:$B"_ustr);
    CPPUNIT_ASSERT(bInserted);

    // Set named range for entire absolute row 2.
    bInserted = m_pDoc->InsertNewRangeName(u"EntireRow"_ustr, aNamePos, u"$2:$2"_ustr);
    CPPUNIT_ASSERT(bInserted);

    // Set values in B1:B3.
    m_pDoc->SetValue(ScAddress(1,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,1,0), 2.0);
    m_pDoc->SetValue(ScAddress(1,2,0), 3.0);

    // Reference them in B4.
    m_pDoc->SetString(ScAddress(1,3,0), u"=SUM(B1:B3)"_ustr);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,3,0)));

    // Use named range in C2 to reference B2.
    m_pDoc->SetString(ScAddress(2,1,0), u"=RowRelativeRange"_ustr);
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,1,0)));

    // Use named range in C3 to reference column B, values in B1,B2,B3,B4
    m_pDoc->SetString(ScAddress(2,2,0), u"=SUM(EntireColumn)"_ustr);
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(2,2,0)));

    // Use named range in C4 to reference row 2, values in B2 and C2.
    m_pDoc->SetString(ScAddress(2,3,0), u"=SUM(EntireRow)"_ustr);
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(2,3,0)));

    // Insert columns over A:B.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    rFunc.InsertCells(ScRange(0,0,0,1,m_pDoc->MaxRow(),0), &aMark, INS_INSCOLS_BEFORE, false, true);

    // Now, the original column B has moved to column D.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in D4 after column insertion.", u"=SUM(D1:D3)"_ustr, m_pDoc->GetFormula(3,3,0));

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(3,3,0)));

    // Check that the named reference points to the moved cell, now D2.
    ScRangeData* pName = m_pDoc->GetRangeName()->findByUpperName(u"ROWRELATIVERANGE"_ustr);
    CPPUNIT_ASSERT(pName);
    OUString aSymbol = pName->GetSymbol(aNamePos, formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL(u"$Formula.$D2"_ustr, aSymbol);

    // Check that the formula using the name, now in E2, still has the same result.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in E2 after column insertion.", u"=RowRelativeRange"_ustr, m_pDoc->GetFormula(4,1,0));

    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(4,1,0)));

    // Check that the named column reference points to the moved column, now D.
    pName = m_pDoc->GetRangeName()->findByUpperName(u"ENTIRECOLUMN"_ustr);
    CPPUNIT_ASSERT(pName);
    aSymbol = pName->GetSymbol(aNamePos, formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL(u"$D:$D"_ustr, aSymbol);

    // Check that the formula using the name, now in E3, still has the same result.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in E3 after column insertion.", u"=SUM(EntireColumn)"_ustr, m_pDoc->GetFormula(4,2,0));

    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(4,2,0)));

    // Check that the named row reference still points to the same entire row
    // and does not have a #REF! error due to inserted columns.
    pName = m_pDoc->GetRangeName()->findByUpperName(u"ENTIREROW"_ustr);
    CPPUNIT_ASSERT(pName);
    aSymbol = pName->GetSymbol(aNamePos, formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL(u"$2:$2"_ustr, aSymbol);

    // Check that the formula using the name, now in E4, still has the same result.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in E4 after column insertion.", u"=SUM(EntireRow)"_ustr, m_pDoc->GetFormula(4,3,0));

    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(4,3,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateMove)
{
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    // Set value to B4:B6.
    m_pDoc->SetValue(ScAddress(1,3,0), 1);
    m_pDoc->SetValue(ScAddress(1,4,0), 2);
    m_pDoc->SetValue(ScAddress(1,5,0), 3);

    // Set formulas to A9:A12 that references B4:B6.
    m_pDoc->SetString(ScAddress(0,8,0), u"=SUM(B4:B6)"_ustr);
    m_pDoc->SetString(ScAddress(0,9,0), u"=SUM($B$4:$B$6)"_ustr);
    m_pDoc->SetString(ScAddress(0,10,0), u"=B5"_ustr);
    m_pDoc->SetString(ScAddress(0,11,0), u"=$B$6"_ustr);

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,8,0));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,9,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0,10,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(0,11,0));

    // Move B4:B6 to D4 (two columns to the right).
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(1,3,0,1,5,0), ScAddress(3,3,0), true, false, false, false);
    CPPUNIT_ASSERT_MESSAGE("Failed to move B4:B6.", bMoved);

    // The results of the formula cells that reference the moved range should remain the same.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,8,0));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,9,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0,10,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(0,11,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(D4:D6)"_ustr, m_pDoc->GetFormula(0,8,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM($D$4:$D$6)"_ustr, m_pDoc->GetFormula(0,9,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=D5"_ustr, m_pDoc->GetFormula(0,10,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=$D$6"_ustr, m_pDoc->GetFormula(0,11,0));

    // Move A9:A12 to B10:B13.
    bMoved = rFunc.MoveBlock(ScRange(0,8,0,0,11,0), ScAddress(1,9,0), true, false, false, false);
    CPPUNIT_ASSERT_MESSAGE("Failed to move A9:A12 to B10:B13", bMoved);

    // The results of these formula cells should still stay the same.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(1,9,0));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(1,10,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1,11,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,12,0));

    // Displayed formulas should stay the same since the referenced range hasn't moved.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(D4:D6)"_ustr, m_pDoc->GetFormula(1,9,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM($D$4:$D$6)"_ustr, m_pDoc->GetFormula(1,10,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=D5"_ustr, m_pDoc->GetFormula(1,11,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=$D$6"_ustr, m_pDoc->GetFormula(1,12,0));

    // The value cells are in D4:D6. Move D4:D5 to the right but leave D6
    // where it is.
    bMoved = rFunc.MoveBlock(ScRange(3,3,0,3,4,0), ScAddress(4,3,0), true, false, false, false);
    CPPUNIT_ASSERT_MESSAGE("Failed to move D4:D5 to E4:E5", bMoved);

    // Only the values of B10 and B11 should be updated.
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,9,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,10,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1,11,0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(1,12,0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(D4:D6)"_ustr, m_pDoc->GetFormula(1,9,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM($D$4:$D$6)"_ustr, m_pDoc->GetFormula(1,10,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=E5"_ustr, m_pDoc->GetFormula(1,11,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=$D$6"_ustr, m_pDoc->GetFormula(1,12,0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateMoveUndo)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    // Set values in A1:A4.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 2.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 3.0);
    m_pDoc->SetValue(ScAddress(0,3,0), 4.0);

    // Set formulas with single cell references in A6:A8.
    m_pDoc->SetString(ScAddress(0,5,0), u"=A1"_ustr);
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A1"_ustr, m_pDoc->GetFormula(0,5,0));

    m_pDoc->SetString(ScAddress(0,6,0), u"=A1+A2+A3"_ustr);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A1+A2+A3"_ustr, m_pDoc->GetFormula(0,6,0));

    m_pDoc->SetString(ScAddress(0,7,0), u"=A1+A3+A4"_ustr);
    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(0,7,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A1+A3+A4"_ustr, m_pDoc->GetFormula(0,7,0));

    // Set formulas with range references in A10:A12.
    m_pDoc->SetString(ScAddress(0,9,0), u"=SUM(A1:A2)"_ustr);
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,9,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(A1:A2)"_ustr, m_pDoc->GetFormula(0,9,0));

    m_pDoc->SetString(ScAddress(0,10,0), u"=SUM(A1:A3)"_ustr);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,10,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(A1:A3)"_ustr, m_pDoc->GetFormula(0,10,0));

    m_pDoc->SetString(ScAddress(0,11,0), u"=SUM(A1:A4)"_ustr);
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,11,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(A1:A4)"_ustr, m_pDoc->GetFormula(0,11,0));

    // Move A1:A3 to C1:C3. Note that A4 remains.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(0,0,0,0,2,0), ScAddress(2,0,0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=C1"_ustr, m_pDoc->GetFormula(0,5,0));

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=C1+C2+C3"_ustr, m_pDoc->GetFormula(0,6,0));

    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(0,7,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=C1+C3+A4"_ustr, m_pDoc->GetFormula(0,7,0));

    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,9,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(C1:C2)"_ustr, m_pDoc->GetFormula(0,9,0));

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,10,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(C1:C3)"_ustr, m_pDoc->GetFormula(0,10,0));

    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(0,11,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(A1:A4)"_ustr, m_pDoc->GetFormula(0,11,0));

    // Undo the move.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A1"_ustr, m_pDoc->GetFormula(0,5,0));

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A1+A2+A3"_ustr, m_pDoc->GetFormula(0,6,0));

    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(0,7,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A1+A3+A4"_ustr, m_pDoc->GetFormula(0,7,0));

    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,9,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(A1:A2)"_ustr, m_pDoc->GetFormula(0,9,0));

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,10,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(A1:A3)"_ustr, m_pDoc->GetFormula(0,10,0));

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,11,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=SUM(A1:A4)"_ustr, m_pDoc->GetFormula(0,11,0));

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

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateMoveUndo2)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    std::vector<std::vector<const char*>> aData = {
        { "1", "2", "=A2*10",      "=SUM(A1:B1)" },
        { "3", "4", "=SUM(A2:B2)", "=SUM(A2:B2)" },
        { "=SUM(A1:B1)" },
    };

    ScRange aOutRange = insertRangeData(m_pDoc, ScAddress(0,0,0), aData);

    std::vector<std::vector<const char*>> aCheckInitial = {
        { "1",     "2",    "30",     "3" },
        { "3",     "4",     "7",     "7" },
        { "3", nullptr, nullptr, nullptr },
    };

    bool bGood = checkOutput(m_pDoc, aOutRange, aCheckInitial, "initial data");
    CPPUNIT_ASSERT(bGood);

    // D1:D2 should be grouped.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(3,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(SCROW(2), pFC->GetSharedLength());

    // Drag A1:B1 into A2:B2 thereby overwriting the old A2:B2 content.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(0,0,0,1,0,0), ScAddress(0,1,0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    std::vector<std::vector<const char*>> aCheckAfter = {
        { nullptr, nullptr,    "10",     "3" },
        {     "1",     "2",     "3",     "3" },
        {     "3", nullptr, nullptr, nullptr },
    };

    bGood = checkOutput(m_pDoc, aOutRange, aCheckAfter, "A1:B1 moved to A2:B2");
    CPPUNIT_ASSERT(bGood);

    // Undo the move.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    bGood = checkOutput(m_pDoc, aOutRange, aCheckInitial, "after undo");
    CPPUNIT_ASSERT(bGood);

    // D1:D2 should be grouped.
    pFC = m_pDoc->GetFormulaCell(ScAddress(3,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(SCROW(2), pFC->GetSharedLength());

    // Redo and check.
    pUndoMgr->Redo();

    bGood = checkOutput(m_pDoc, aOutRange, aCheckAfter, "after redo");
    CPPUNIT_ASSERT(bGood);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateMoveUndo3NonShared)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    std::vector<std::vector<const char*>> aData = {
        { "10",       nullptr,  nullptr },
        { "=A1",      nullptr,  nullptr },
        { "=A2+A1",   nullptr,  nullptr },
    };

    ScRange aOutRange = insertRangeData(m_pDoc, ScAddress(0,0,0), aData);

    std::vector<std::vector<const char*>> aCheckInitial = {
        { "10", nullptr,  nullptr },
        { "10", nullptr,  nullptr },
        { "20", nullptr,  nullptr },
    };

    bool bGood = checkOutput(m_pDoc, aOutRange, aCheckInitial, "initial data");
    CPPUNIT_ASSERT(bGood);

    // Drag A2:A3 into C2:C3.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(0,1,0,0,2,0), ScAddress(2,1,0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    std::vector<std::vector<const char*>> aCheckAfter = {
        { "10",    nullptr, nullptr},
        { nullptr, nullptr, "10" },
        { nullptr, nullptr, "20" },
    };

    bGood = checkOutput(m_pDoc, aOutRange, aCheckAfter, "A2:A3 moved to C2:C3");
    CPPUNIT_ASSERT(bGood);

    // Undo the move.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    bGood = checkOutput(m_pDoc, aOutRange, aCheckInitial, "after undo");
    CPPUNIT_ASSERT(bGood);

    // Redo and check.
    pUndoMgr->Redo();

    bGood = checkOutput(m_pDoc, aOutRange, aCheckAfter, "after redo");
    CPPUNIT_ASSERT(bGood);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateMoveUndo3Shared)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    std::vector<std::vector<const char*>> aData = {
        { "10",       nullptr,  nullptr },
        { "=A1",      nullptr,  nullptr },
        { "=A2+$A$1", nullptr,  nullptr },
        { "=A3+$A$1", nullptr,  nullptr },
    };

    ScRange aOutRange = insertRangeData(m_pDoc, ScAddress(0,0,0), aData);

    std::vector<std::vector<const char*>> aCheckInitial = {
        { "10", nullptr,  nullptr },
        { "10", nullptr,  nullptr },
        { "20", nullptr,  nullptr },
        { "30", nullptr,  nullptr },
    };

    bool bGood = checkOutput(m_pDoc, aOutRange, aCheckInitial, "initial data");
    CPPUNIT_ASSERT(bGood);

    // A3:A4 should be grouped.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0,2,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(SCROW(2), pFC->GetSharedLength());

    // Drag A2:A4 into C2:C4.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(0,1,0,0,3,0), ScAddress(2,1,0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    std::vector<std::vector<const char*>> aCheckAfter = {
        { "10",    nullptr, nullptr},
        { nullptr, nullptr, "10" },
        { nullptr, nullptr, "20" },
        { nullptr, nullptr, "30" },
    };

    bGood = checkOutput(m_pDoc, aOutRange, aCheckAfter, "A2:A4 moved to C2:C4");
    CPPUNIT_ASSERT(bGood);

    // C3:C4 should be grouped.
    pFC = m_pDoc->GetFormulaCell(ScAddress(2,2,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(SCROW(2), pFC->GetSharedLength());

    // Undo the move.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    bGood = checkOutput(m_pDoc, aOutRange, aCheckInitial, "after undo");
    CPPUNIT_ASSERT(bGood);

    // A3:A4 should be grouped.
    pFC = m_pDoc->GetFormulaCell(ScAddress(0,2,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(SCROW(2), pFC->GetSharedLength());

    // Redo and check.
    pUndoMgr->Redo();

    bGood = checkOutput(m_pDoc, aOutRange, aCheckAfter, "after redo");
    CPPUNIT_ASSERT(bGood);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateMoveUndoDependents)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    std::vector<std::vector<const char*>> aData = {
        { "1"           },
        { "22"          },
        { "3"           },
        { "4"           },
        { "5"           },
        { "=SUM(C1:C5)" },
        { "=C6"         },
    };

    ScRange aOutRange = insertRangeData(m_pDoc, ScAddress(2,0,0), aData);

    std::vector<std::vector<const char*>> aCheckInitial = {
        { "1"   },
        { "22"  },
        { "3"   },
        { "4"   },
        { "5"   },
        { "35"  },
        { "35"  },
    };

    bool bGood = checkOutput(m_pDoc, aOutRange, aCheckInitial, "initial data");
    CPPUNIT_ASSERT(bGood);

    // Drag C2 into D2.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(2, 1, 0, 2, 1, 0), ScAddress(3, 1, 0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    std::vector<std::vector<const char*>> aCheckAfter = {
        { "1"     },
        { nullptr },
        { "3"     },
        { "4"     },
        { "5"     },
        { "13"    },
        { "13"    },
    };

    bGood = checkOutput(m_pDoc, aOutRange, aCheckAfter, "C2 moved to D2");
    CPPUNIT_ASSERT(bGood);

    // Undo the move.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    bGood = checkOutput(m_pDoc, aOutRange, aCheckInitial, "after undo");
    CPPUNIT_ASSERT(bGood);

    // Redo and check.
    pUndoMgr->Redo();

    bGood = checkOutput(m_pDoc, aOutRange, aCheckAfter, "after redo");
    CPPUNIT_ASSERT(bGood);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateMoveUndo4)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    std::vector<std::vector<const char*>> aData = {
        { "1",  nullptr,  "=B1", "=A1" },
        { "2",  nullptr,  "=B2", "=A2" },
    };

    ScRange aOutRange = insertRangeData(m_pDoc, ScAddress(0,0,0), aData);

    std::vector<std::vector<const char*>> aCheckInitial = {
        { "1",  nullptr,  "0", "1" },
        { "2",  nullptr,  "0", "2" },
    };

    bool bGood = checkOutput(m_pDoc, aOutRange, aCheckInitial, "initial data");
    CPPUNIT_ASSERT(bGood);

    // Drag A1:A2 into B1:B2.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(0, 0, 0, 0, 1, 0), ScAddress(1, 0, 0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    std::vector<std::vector<const char*>> aCheckAfter = {
        { nullptr, "1", "1", "1" },
        { nullptr, "2", "2", "2" },
    };

    bGood = checkOutput(m_pDoc, aOutRange, aCheckAfter, "A1:A2 moved to B1:B2");
    CPPUNIT_ASSERT(bGood);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=B1"_ustr, m_pDoc->GetFormula(2,0,0)); // C1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=B2"_ustr, m_pDoc->GetFormula(2,1,0)); // C2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=B1"_ustr, m_pDoc->GetFormula(3,0,0)); // D1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=B2"_ustr, m_pDoc->GetFormula(3,1,0)); // D2

    // Undo the move.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    bGood = checkOutput(m_pDoc, aOutRange, aCheckInitial, "after undo");
    CPPUNIT_ASSERT(bGood);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=B1"_ustr, m_pDoc->GetFormula(2,0,0)); // C1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=B2"_ustr, m_pDoc->GetFormula(2,1,0)); // C2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=A1"_ustr, m_pDoc->GetFormula(3,0,0)); // D1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=A2"_ustr, m_pDoc->GetFormula(3,1,0)); // D2

    // Redo and check.
    pUndoMgr->Redo();

    bGood = checkOutput(m_pDoc, aOutRange, aCheckAfter, "after redo");
    CPPUNIT_ASSERT(bGood);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=B1"_ustr, m_pDoc->GetFormula(2,0,0)); // C1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=B2"_ustr, m_pDoc->GetFormula(2,1,0)); // C2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=B1"_ustr, m_pDoc->GetFormula(3,0,0)); // D1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=B2"_ustr, m_pDoc->GetFormula(3,1,0)); // D2

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateMoveToSheet)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, u"Sheet1"_ustr);
    m_pDoc->InsertTab(1, u"Sheet2"_ustr);

    // Set values to A1:A2 on Sheet1, and B1:B2 to reference them.
    m_pDoc->SetValue(ScAddress(0,0,0), 11);
    m_pDoc->SetValue(ScAddress(0,1,0), 12);
    m_pDoc->SetString(ScAddress(1,0,0), u"=A1"_ustr);
    m_pDoc->SetString(ScAddress(1,1,0), u"=A2"_ustr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=A1"_ustr, m_pDoc->GetFormula(1,0,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=A2"_ustr, m_pDoc->GetFormula(1,1,0));

    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(1,1,0)));

    // Move A1:A2 on Sheet1 to B3:B4 on Sheet2.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(0,0,0,0,1,0), ScAddress(1,2,1), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=Sheet2.B3"_ustr, m_pDoc->GetFormula(1,0,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=Sheet2.B4"_ustr, m_pDoc->GetFormula(1,1,0));

    // Undo and check again.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    pUndoMgr->Undo();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=A1"_ustr, m_pDoc->GetFormula(1,0,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=A2"_ustr, m_pDoc->GetFormula(1,1,0));

    // Redo and check.
    pUndoMgr->Redo();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=Sheet2.B3"_ustr, m_pDoc->GetFormula(1,0,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"=Sheet2.B4"_ustr, m_pDoc->GetFormula(1,1,0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateDeleteContent)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Set value in B2.
    m_pDoc->SetValue(ScAddress(1,1,0), 2.0);
    // Set formula in C2 to reference B2.
    m_pDoc->SetString(ScAddress(2,1,0), u"=B2"_ustr);

    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,1,0)));

    // Delete B2.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    ScMarkData aMark(m_pDoc->GetSheetLimits());
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

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateDeleteAndShiftLeft)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Insert 1,2,3,4,5 in C1:G1.
    for (SCCOL i = 0; i <= 4; ++i)
        m_pDoc->SetValue(ScAddress(i+2,0,0), i+1);

    // Insert formula in H1.
    ScAddress aPos(7,0,0);
    m_pDoc->SetString(aPos, u"=SUM(C1:G1)"_ustr);

    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));

    // Delete columns D:E (middle of the reference).
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bDeleted = rFunc.DeleteCells(ScRange(3,0,0,4,m_pDoc->MaxRow(),0), &aMark, DelCellCmd::CellsLeft, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncCol(-2);
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(C1:E1)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Undo and check.
    SfxUndoManager* pUndo = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndo);

    pUndo->Undo();
    aPos.IncCol(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(C1:G1)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Delete columns C:D (left end of the reference).
    bDeleted = rFunc.DeleteCells(ScRange(2,0,0,3,m_pDoc->MaxRow(),0), &aMark, DelCellCmd::CellsLeft, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncCol(-2);
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(C1:E1)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Undo and check again.
    pUndo->Undo();
    aPos.IncCol(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(C1:G1)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Delete columns B:E (overlaps on the left).
    bDeleted = rFunc.DeleteCells(ScRange(1,0,0,4,m_pDoc->MaxRow(),0), &aMark, DelCellCmd::CellsLeft, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncCol(-4);
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(B1:C1)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Undo and check again.
    pUndo->Undo();
    aPos.IncCol(4);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(C1:G1)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Start over with a new scenario.
    clearSheet(m_pDoc, 0);

    // Insert 1,2,3,4,5,6 into C1:H1.
    for (SCCOL i = 0; i <= 5; ++i)
        m_pDoc->SetValue(ScAddress(i+2,0,0), i+1);

    // Set formula in B1.
    aPos = ScAddress(1,0,0);
    m_pDoc->SetString(aPos, u"=SUM(C1:H1)"_ustr);
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));

    // Delete columns F:H (right end of the reference).
    bDeleted = rFunc.DeleteCells(ScRange(5,0,0,7,m_pDoc->MaxRow(),0), &aMark, DelCellCmd::CellsLeft, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(C1:E1)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Undo and check.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(C1:H1)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Delete columns G:I (overlaps on the right).
    bDeleted = rFunc.DeleteCells(ScRange(6,0,0,8,m_pDoc->MaxRow(),0), &aMark, DelCellCmd::CellsLeft, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(C1:F1)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Undo and check again.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(C1:H1)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateDeleteAndShiftLeft2)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, u"Test"_ustr);

    std::vector<std::vector<const char*>> aData = {
        { "1", "=COUNT($A$1:$A$4)", "=COUNT(A1)" },
        { "2", "=COUNT($A$1:$A$4)", "=COUNT(A2)" },
        { "3", "=COUNT($A$1:$A$4)", "=COUNT(A3)" },
        { "4", "=COUNT($A$1:$A$4)", "=COUNT(A4)" },
    };

    insertRangeData(m_pDoc, ScAddress(), aData);

    auto funcCheckOriginal = [&]()
    {
        CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,0,0))); // A1
        CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0,1,0))); // A2
        CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,2,0))); // A3
        CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(0,3,0))); // A4

        CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,0,0))); // B1
        CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,1,0))); // B2
        CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,2,0))); // B3
        CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,3,0))); // B4

        CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(2,0,0))); // C1
        CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(2,1,0))); // C2
        CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(2,2,0))); // C3
        CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(2,3,0))); // C4
    };

    auto funcCheckDeleted = [&]()
    {
        CPPUNIT_ASSERT_EQUAL(u"#REF!"_ustr, m_pDoc->GetString(ScAddress(0,0,0))); // A1
        CPPUNIT_ASSERT_EQUAL(u"#REF!"_ustr, m_pDoc->GetString(ScAddress(0,1,0))); // A2
        CPPUNIT_ASSERT_EQUAL(u"#REF!"_ustr, m_pDoc->GetString(ScAddress(0,2,0))); // A3
        CPPUNIT_ASSERT_EQUAL(u"#REF!"_ustr, m_pDoc->GetString(ScAddress(0,3,0))); // A4

        CPPUNIT_ASSERT_EQUAL(u"#REF!"_ustr, m_pDoc->GetString(ScAddress(1,0,0))); // B1
        CPPUNIT_ASSERT_EQUAL(u"#REF!"_ustr, m_pDoc->GetString(ScAddress(1,1,0))); // B2
        CPPUNIT_ASSERT_EQUAL(u"#REF!"_ustr, m_pDoc->GetString(ScAddress(1,2,0))); // B3
        CPPUNIT_ASSERT_EQUAL(u"#REF!"_ustr, m_pDoc->GetString(ScAddress(1,3,0))); // B4
    };

    funcCheckOriginal();

    // Delete Column A.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bDeleted = rFunc.DeleteCells(ScRange(0,0,0,0,m_pDoc->MaxRow(),0), &aMark, DelCellCmd::CellsLeft, true);
    CPPUNIT_ASSERT(bDeleted);

    funcCheckDeleted();

    // Undo and check.
    SfxUndoManager* pUndo = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndo);

    pUndo->Undo();
    funcCheckOriginal();

    // Redo and check.
    pUndo->Redo();
    funcCheckDeleted();

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateDeleteAndShiftUp)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Insert 1,2,3,4,5 in A3:A7.
    for (SCROW i = 0; i <= 4; ++i)
        m_pDoc->SetValue(ScAddress(0,i+2,0), i+1);

    // Insert formula in A8.
    ScAddress aPos(0,7,0);
    m_pDoc->SetString(aPos, u"=SUM(A3:A7)"_ustr);

    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));

    // Delete rows 4:5 (middle of the reference).
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bDeleted = rFunc.DeleteCells(ScRange(0,3,0,m_pDoc->MaxCol(),4,0), &aMark, DelCellCmd::CellsUp, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncRow(-2);
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A3:A5)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Undo and check.
    SfxUndoManager* pUndo = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndo);

    pUndo->Undo();
    aPos.IncRow(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A3:A7)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Delete rows 3:4 (top end of the reference).
    bDeleted = rFunc.DeleteCells(ScRange(0,2,0,m_pDoc->MaxCol(),3,0), &aMark, DelCellCmd::CellsUp, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncRow(-2);
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A3:A5)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Undo and check again.
    pUndo->Undo();
    aPos.IncRow(2);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A3:A7)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Delete rows 2:5 (overlaps on the top).
    bDeleted = rFunc.DeleteCells(ScRange(0,1,0,m_pDoc->MaxCol(),4,0), &aMark, DelCellCmd::CellsUp, true);
    CPPUNIT_ASSERT(bDeleted);

    aPos.IncRow(-4);
    CPPUNIT_ASSERT_EQUAL(9.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A2:A3)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Undo and check again.
    pUndo->Undo();
    aPos.IncRow(4);
    CPPUNIT_ASSERT_EQUAL(15.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A3:A7)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Start over with a new scenario.
    clearSheet(m_pDoc, 0);

    // Insert 1,2,3,4,5,6 into A3:A8.
    for (SCROW i = 0; i <= 5; ++i)
        m_pDoc->SetValue(ScAddress(0,i+2,0), i+1);

    // Set formula in B1.
    aPos = ScAddress(0,1,0);
    m_pDoc->SetString(aPos, u"=SUM(A3:A8)"_ustr);
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));

    // Delete rows 6:8 (bottom end of the reference).
    bDeleted = rFunc.DeleteCells(ScRange(0,5,0,m_pDoc->MaxCol(),7,0), &aMark, DelCellCmd::CellsUp, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A3:A5)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Undo and check.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A3:A8)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Delete rows 7:9 (overlaps on the bottom).
    bDeleted = rFunc.DeleteCells(ScRange(0,6,0,m_pDoc->MaxCol(),8,0), &aMark, DelCellCmd::CellsUp, true);
    CPPUNIT_ASSERT(bDeleted);

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A3:A6)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Undo and check again.
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=SUM(A3:A8)"_ustr, m_pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateName)
{
    m_pDoc->InsertTab(0, u"Formula"_ustr);

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
        *m_pDoc, u"ToLeft"_ustr, u"RC[-1]"_ustr, ScAddress(2,1,0),
        ScRangeData::Type::Name, formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1);

    bool bInserted = pGlobalNames->insert(pName);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert a new name.", bInserted);

    // Insert formulas in D2:D5 using the named expression.
    m_pDoc->SetString(ScAddress(3,1,0), u"=ToLeft"_ustr);
    m_pDoc->SetString(ScAddress(3,2,0), u"=ToLeft"_ustr);
    m_pDoc->SetString(ScAddress(3,3,0), u"=ToLeft"_ustr);
    m_pDoc->SetString(ScAddress(3,4,0), u"=ToLeft"_ustr);

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
        *m_pDoc, u"MyRange"_ustr, u"$B$10:$B$12"_ustr, ScAddress(0,0,0), ScRangeData::Type::Name, formula::FormulaGrammar::GRAM_NATIVE);
    bInserted = pGlobalNames->insert(pName);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert a new name.", bInserted);

    // Set formula at C8 that references this named expression.
    m_pDoc->SetString(ScAddress(2,7,0), u"=SUM(MyRange)"_ustr);
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(ScAddress(2,7,0)));

    // Shift B10:B12 to right by 2 columns.
    m_pDoc->InsertCol(ScRange(1,9,0,2,11,0));

    // This should shift the absolute range B10:B12 that MyRange references.
    pName = pGlobalNames->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Failed to find named expression 'MyRange' in the global scope.", pName);
    OUString aExpr = pName->GetSymbol();
    CPPUNIT_ASSERT_EQUAL(u"$D$10:$D$12"_ustr, aExpr);

    // This move shouldn't affect the value of C8.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(2,7,0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(ScAddress(2,7,0)));

    // Update the value of D10 and make sure C8 gets updated.
    m_pDoc->SetValue(ScAddress(3,9,0), 20);
    CPPUNIT_ASSERT_EQUAL(43.0, m_pDoc->GetValue(ScAddress(2,7,0)));

    // Insert a new sheet before the current.
    m_pDoc->InsertTab(0, u"New"_ustr);
    OUString aName;
    m_pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(u"Formula"_ustr, aName);

    pName = pGlobalNames->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Failed to find named expression 'MyRange' in the global scope.", pName);

    m_pDoc->SetValue(ScAddress(3,9,1), 10);
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(ScAddress(2,7,1)));

    // Delete the inserted sheet, which will shift the 'Formula' sheet to the left.
    m_pDoc->DeleteTab(0);

    aName.clear();
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(u"Formula"_ustr, aName);

    pName = pGlobalNames->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Failed to find named expression 'MyRange' in the global scope.", pName);

    m_pDoc->SetValue(ScAddress(3,9,0), 11);
    CPPUNIT_ASSERT_EQUAL(34.0, m_pDoc->GetValue(ScAddress(2,7,0)));

    // Clear all and start over.
    clearRange(m_pDoc, ScRange(0,0,0,100,100,0));
    pGlobalNames->clear();

    pName = new ScRangeData(
        *m_pDoc, u"MyRange"_ustr, u"$B$1:$C$6"_ustr, ScAddress(0,0,0), ScRangeData::Type::Name, formula::FormulaGrammar::GRAM_NATIVE);
    bInserted = pGlobalNames->insert(pName);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert a new name.", bInserted);
    aExpr = pName->GetSymbol();
    CPPUNIT_ASSERT_EQUAL(u"$B$1:$C$6"_ustr, aExpr);

    // Insert range of cells to shift right. The range partially overlaps the named range.
    m_pDoc->InsertCol(ScRange(2,4,0,3,8,0));

    // This should not alter the range.
    aExpr = pName->GetSymbol();
    CPPUNIT_ASSERT_EQUAL(u"$B$1:$C$6"_ustr, aExpr);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateNameMove)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Set values to B2:B4.
    m_pDoc->SetValue(ScAddress(1,1,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,2,0), 2.0);
    m_pDoc->SetValue(ScAddress(1,3,0), 3.0);

    // Set named range for B2:B4.
    bool bInserted = m_pDoc->InsertNewRangeName(u"MyRange"_ustr, ScAddress(0,0,0), u"$Test.$B$2:$B$4"_ustr);
    CPPUNIT_ASSERT(bInserted);

    // Set formula in A10.
    m_pDoc->SetString(ScAddress(0,9,0), u"=SUM(MyRange)"_ustr);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,9,0)));

    ScRangeData* pData = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pData);
    OUString aSymbol = pData->GetSymbol(m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(u"$Test.$B$2:$B$4"_ustr, aSymbol);

    // Move B2:B4 to D3.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(1,1,0,1,3,0), ScAddress(3,2,0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    // The named range should have moved as well.
    aSymbol = pData->GetSymbol(m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(u"$Test.$D$3:$D$5"_ustr, aSymbol);

    // The value of A10 should remain unchanged.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,9,0)));

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    // Undo and check.
    pUndoMgr->Undo();

    pData = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pData);
    aSymbol = pData->GetSymbol(m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(u"$Test.$B$2:$B$4"_ustr, aSymbol);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,9,0)));

    // Redo and check.
    pUndoMgr->Redo();

    pData = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pData);
    aSymbol = pData->GetSymbol(m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(u"$Test.$D$3:$D$5"_ustr, aSymbol);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,9,0)));

    // Undo again to bring it back to the initial condition, and clear the undo buffer.
    pUndoMgr->Undo();
    pUndoMgr->Clear();

    // Add an identical formula to A11 and make a formula group over A10:A11.
    m_pDoc->SetString(ScAddress(0,10,0), u"=SUM(MyRange)"_ustr);
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
    bInserted = m_pDoc->InsertNewRangeName(u"MyCell"_ustr, ScAddress(0,0,0), u"$Test.$B$2"_ustr);
    CPPUNIT_ASSERT(bInserted);

    // Set formula to B3 that references B2 via MyCell.
    m_pDoc->SetString(ScAddress(1,2,0), u"=MyCell*2"_ustr);
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    // Move B2 to D2.
    bMoved = rFunc.MoveBlock(ScRange(1,1,0,1,1,0), ScAddress(3,1,0), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    // Value in B3 should remain unchanged.
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateNameExpandRef)
{
    setExpandRefs(true);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, u"Test"_ustr);

    bool bInserted = m_pDoc->InsertNewRangeName(u"MyRange"_ustr, ScAddress(0,0,0), u"$A$1:$A$3"_ustr);
    CPPUNIT_ASSERT(bInserted);

    // Set values to A1:A3.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 2.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 3.0);

    m_pDoc->SetString(ScAddress(0,5,0), u"=SUM(MyRange)"_ustr);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Insert a new row at row 4, which should expand the named range to A1:A4.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    rFunc.InsertCells(ScRange(0,3,0,m_pDoc->MaxCol(),3,0), &aMark, INS_INSROWS_BEFORE, false, true);
    ScRangeData* pName = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pName);
    OUString aSymbol = pName->GetSymbol(m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(u"$A$1:$A$4"_ustr, aSymbol);

    // Make sure the listening area has been expanded as well.  Note the
    // formula cell has been pushed downward by one cell.
    m_pDoc->SetValue(ScAddress(0,3,0), 4.0);
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0,6,0)));

    // Insert a new column at column 2, which should not expand the named
    // range as it is only one column wide.
    rFunc.InsertCells(ScRange(1,0,0,1,m_pDoc->MaxRow(),0), &aMark, INS_INSCOLS_BEFORE, false, true);
    pName = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pName);
    aSymbol = pName->GetSymbol(m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(u"$A$1:$A$4"_ustr, aSymbol);

    // Make sure the referenced area has not changed.
    m_pDoc->SetValue(ScAddress(0,3,0), 2.0);
    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(0,6,0)));
    m_pDoc->SetValue(ScAddress(1,3,0), 2.0);
    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(0,6,0)));

    // Clear the document and start over.
    m_pDoc->GetRangeName()->clear();
    clearSheet(m_pDoc, 0);

    // Set values to B4:B6.
    m_pDoc->SetValue(ScAddress(1,3,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,4,0), 2.0);
    m_pDoc->SetValue(ScAddress(1,5,0), 3.0);

    bInserted = m_pDoc->InsertNewRangeName(u"MyRange"_ustr, ScAddress(0,0,0), u"$B$4:$B$6"_ustr);
    CPPUNIT_ASSERT(bInserted);

    // Set formula to A1.
    m_pDoc->SetString(ScAddress(0,0,0), u"=SUM(MyRange)"_ustr);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(0,0,0));

    // Insert rows over 3:5 which should expand the range by 3 rows.
    rFunc.InsertCells(ScRange(0,2,0,m_pDoc->MaxCol(),4,0), &aMark, INS_INSROWS_BEFORE, false, true);

    pName = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pName);

    aSymbol = pName->GetSymbol(m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(u"$B$4:$B$9"_ustr, aSymbol);

    // Clear the document and start over.
    m_pDoc->GetRangeName()->clear();
    clearSheet(m_pDoc, 0);

    // Set values to A1:A3.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 2.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 3.0);

    // Name A1:A3 'MyData'.
    bInserted = m_pDoc->InsertNewRangeName(u"MyData"_ustr, ScAddress(0,0,0), u"$A$1:$A$3"_ustr);
    CPPUNIT_ASSERT(bInserted);

    // Set formulas to C1:C2 and E1.
    m_pDoc->SetString(ScAddress(2,0,0), u"=SUM(MyData)"_ustr);
    m_pDoc->SetString(ScAddress(2,1,0), u"=SUM(MyData)"_ustr);
    m_pDoc->SetString(ScAddress(4,0,0), u"=SUM(MyData)"_ustr);

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
    rFunc.InsertCells(ScRange(0,2,0,m_pDoc->MaxCol(),2,0), &aMark, INS_INSROWS_BEFORE, false, true);

    // Set new value to A3.
    m_pDoc->SetValue(ScAddress(0,2,0), 4.0);

    // Check the results again.
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(2,1,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(4,0,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateNameExpandRef2)
{
    setExpandRefs(true);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, u"Test"_ustr);

    bool bInserted = m_pDoc->InsertNewRangeName(u"MyRange"_ustr, ScAddress(0,0,0), u"$A$1:$B$3"_ustr);
    CPPUNIT_ASSERT(bInserted);

    // Insert a new row at row 4, which should expand the named range to A1:A4.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);

    // Insert a new column at column 3, which should expand the named
    rFunc.InsertCells(ScRange(1,0,0,1,m_pDoc->MaxRow(),0), &aMark, INS_INSCOLS_BEFORE, false, true);
    ScRangeData* pName = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pName);
    OUString aSymbol = pName->GetSymbol(m_pDoc->GetGrammar());
    CPPUNIT_ASSERT_EQUAL(u"$A$1:$C$3"_ustr, aSymbol);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateNameDeleteRow)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Insert a new name 'MyRange' to reference B2:B4.
    bool bInserted = m_pDoc->InsertNewRangeName(u"MyRange"_ustr, ScAddress(0,0,0), u"$B$2:$B$4"_ustr);
    CPPUNIT_ASSERT(bInserted);

    const ScRangeData* pName = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pName);

    sc::TokenStringContext aCxt(*m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH);
    const ScTokenArray* pCode = pName->GetCode();
    OUString aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$2:$B$4"_ustr, aExpr);

    // Insert a new name 'MyAddress' to reference $B$3. Note absolute row.
    bInserted = m_pDoc->InsertNewRangeName(u"MyAddress"_ustr, ScAddress(0,0,0), u"$B$3"_ustr);
    CPPUNIT_ASSERT(bInserted);

    const ScRangeData* pName2 = m_pDoc->GetRangeName()->findByUpperName(u"MYADDRESS"_ustr);
    CPPUNIT_ASSERT(pName2);

    sc::TokenStringContext aCxt2(*m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH);
    const ScTokenArray* pCode2 = pName2->GetCode();
    OUString aExpr2 = pCode2->CreateString(aCxt2, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$3"_ustr, aExpr2);

    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();

    // Delete row 3.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    rFunc.DeleteCells(ScRange(0,2,0,m_pDoc->MaxCol(),2,0), &aMark, DelCellCmd::CellsUp, true);

    // The reference in the 'MyRange' name should get updated to B2:B3.
    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$2:$B$3"_ustr, aExpr);

    // The reference in the 'MyAddress' name should get updated to $B$#REF!.
    aExpr2 = pCode2->CreateString(aCxt2, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$#REF!"_ustr, aExpr2);

    // Delete row 3 again.
    rFunc.DeleteCells(ScRange(0,2,0,m_pDoc->MaxCol(),2,0), &aMark, DelCellCmd::CellsUp, true);
    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$2:$B$2"_ustr, aExpr);

    // Undo and check.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    pUndoMgr->Undo();

    pName = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pName);
    pCode = pName->GetCode();

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$2:$B$3"_ustr, aExpr);

    // Undo again and check.
    pUndoMgr->Undo();

    pName = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pName);
    pCode = pName->GetCode();

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$2:$B$4"_ustr, aExpr);

    // Delete row 2-3.
    rFunc.DeleteCells(ScRange(0,1,0,m_pDoc->MaxCol(),2,0), &aMark, DelCellCmd::CellsUp, true);

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$2:$B$2"_ustr, aExpr);

    // Undo and check.
    pUndoMgr->Undo();

    pName = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pName);
    pCode = pName->GetCode();

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$2:$B$4"_ustr, aExpr);

    pName2 = m_pDoc->GetRangeName()->findByUpperName(u"MYADDRESS"_ustr);
    CPPUNIT_ASSERT(pName2);
    pCode2 = pName2->GetCode();

    aExpr2 = pCode2->CreateString(aCxt2, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$3"_ustr, aExpr2);

    m_pDoc->InsertTab(1, u"test2"_ustr);

    ScMarkData aMark2(m_pDoc->GetSheetLimits());
    aMark2.SelectOneTable(1);
    rFunc.DeleteCells(ScRange(0,2,1,m_pDoc->MaxCol(),2,1), &aMark2, DelCellCmd::CellsUp, true);

    pName = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pName);
    pCode = pName->GetCode();

    aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$2:$B$4"_ustr, aExpr);

    pName2 = m_pDoc->GetRangeName()->findByUpperName(u"MYADDRESS"_ustr);
    CPPUNIT_ASSERT(pName2);
    pCode2 = pName2->GetCode();

    // Deleting a range the 'MyAddress' name points into due to its implicit
    // relative sheet reference to the sheet where used does not invalidate
    // the named expression because when updating the sheet reference is
    // relative to its base position on sheet 0 (same for the 'MyRange' range,
    // which is the reason why it is not updated either).
    // This is a tad confusing...
    aExpr2 = pCode2->CreateString(aCxt2, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$B$3"_ustr, aExpr2);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateNameCopySheet)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);
    m_pDoc->InsertTab(1, u"Test2"_ustr);

    bool bInserted = m_pDoc->InsertNewRangeName(u"RED"_ustr, ScAddress(0,0,0), u"$Test.$B$2"_ustr);
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName(u"BLUE"_ustr, ScAddress(0,0,0), u"$Test.$B$3"_ustr);
    CPPUNIT_ASSERT(bInserted);
    m_pDoc->SetValue(1, 1, 0, 1);
    m_pDoc->SetValue(1, 2, 0, 2);

    // insert formula into Test2 that is =RED+BLUE
    m_pDoc->SetString(ScAddress(2,2,1), u"=RED+BLUE"_ustr);

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

    m_pDoc->InsertTab(0, u"Test1"_ustr);
    // Global name referencing sheet Test1.
    bInserted = m_pDoc->InsertNewRangeName(u"sheetnumber"_ustr, ScAddress(0,0,0), u"$Test1.$A$1"_ustr);
    CPPUNIT_ASSERT(bInserted);
    m_pDoc->SetString(ScAddress(0,0,0), u"=SHEET()"_ustr);
    m_pDoc->SetString(ScAddress(1,0,0), u"=sheetnumber"_ustr);
    nVal = m_pDoc->GetValue(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet number should be 1", 1.0, nVal);

    // Copy sheet after.
    m_pDoc->CopyTab(0, 1);
    nVal = m_pDoc->GetValue(1,0,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("New sheet number should be 2", 2.0, nVal);
    nVal = m_pDoc->GetValue(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Org sheet number should be 1", 1.0, nVal);
    const ScRangeData* pName = m_pDoc->GetRangeName(1)->findByUpperName(u"SHEETNUMBER"_ustr);
    CPPUNIT_ASSERT_MESSAGE("New sheet-local name should exist", pName);

    // Copy sheet before, shifting following now two sheets.
    m_pDoc->CopyTab(0, 0);
    nVal = m_pDoc->GetValue(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("New sheet number should be 1", 1.0, nVal);
    pName = m_pDoc->GetRangeName(0)->findByUpperName(u"SHEETNUMBER"_ustr);
    CPPUNIT_ASSERT_MESSAGE("New sheet-local name should exist", pName);
    nVal = m_pDoc->GetValue(1,0,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Org sheet number should be 2", 2.0, nVal);
    pName = m_pDoc->GetRangeName(1)->findByUpperName(u"SHEETNUMBER"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Org sheet-local name should not exist", !pName);
    nVal = m_pDoc->GetValue(1,0,2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Old sheet number should be 3", 3.0, nVal);
    pName = m_pDoc->GetRangeName(2)->findByUpperName(u"SHEETNUMBER"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Old sheet-local name should exist", pName);

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);

    m_pDoc->InsertTab(0, u"Test2"_ustr);
    // Local name referencing sheet Test2.
    bInserted = m_pDoc->GetRangeName(0)->insert( new ScRangeData( *m_pDoc, u"localname"_ustr, u"$Test2.$A$1"_ustr));
    CPPUNIT_ASSERT(bInserted);
    m_pDoc->SetString(ScAddress(0,0,0), u"=SHEET()"_ustr);
    m_pDoc->SetString(ScAddress(1,0,0), u"=localname"_ustr);
    nVal = m_pDoc->GetValue(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Localname sheet number should be 1", 1.0, nVal);

    // Insert sheet before and shift sheet with local name.
    m_pDoc->InsertTab(0, u"Test1"_ustr);
    pName = m_pDoc->GetRangeName(1)->findByUpperName(u"LOCALNAME"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Org sheet-local name should exist", pName);
    nVal = m_pDoc->GetValue(1,0,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Localname sheet number should be 2", 2.0, nVal);

    // Copy sheet before, shifting following now two sheets.
    m_pDoc->CopyTab(1, 0);
    pName = m_pDoc->GetRangeName(0)->findByUpperName(u"LOCALNAME"_ustr);
    CPPUNIT_ASSERT_MESSAGE("New sheet-local name should exist", pName);
    nVal = m_pDoc->GetValue(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("New sheet number should be 1", 1.0, nVal);
    pName = m_pDoc->GetRangeName(1)->findByUpperName(u"LOCALNAME"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Old sheet-local name should not exist", !pName);
    pName = m_pDoc->GetRangeName(2)->findByUpperName(u"LOCALNAME"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Org sheet-local name should exist", pName);
    nVal = m_pDoc->GetValue(1,0,2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("New sheet number should be 3", 3.0, nVal);

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
    m_pDoc->SetRangeName(nullptr);

    // Test nested names during copying sheet.

    m_pDoc->InsertTab(0, u"Test2"_ustr);
    ScAddress aPos(0,0,0);
    bInserted = m_pDoc->InsertNewRangeName( u"global"_ustr, aPos, u"$Test2.$A$1"_ustr);
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( aPos.Tab(), u"local"_ustr, aPos, u"$Test2.$A$2"_ustr);
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( u"global_global"_ustr, aPos, u"global*100"_ustr);
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( u"global_local"_ustr, aPos, u"local*1000"_ustr);
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( u"global_unused"_ustr, aPos, u"$Test2.$A$1"_ustr);
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( u"global_unused_noref"_ustr, aPos, u"42"_ustr);
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( aPos.Tab(), u"local_global"_ustr, aPos, u"global*10000"_ustr);
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( aPos.Tab(), u"local_local"_ustr, aPos, u"local*100000"_ustr);
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( aPos.Tab(), u"local_unused"_ustr, aPos, u"$Test2.$A$2"_ustr);
    CPPUNIT_ASSERT(bInserted);
    bInserted = m_pDoc->InsertNewRangeName( aPos.Tab(), u"local_unused_noref"_ustr, aPos, u"23"_ustr);
    CPPUNIT_ASSERT(bInserted);

    m_pDoc->SetString(aPos, u"=SHEET()"_ustr);
    aPos.IncRow();
    m_pDoc->SetString(aPos, u"=A1*10+SHEET()"_ustr);
    aPos.IncRow();
    m_pDoc->SetString(aPos, u"=global_global"_ustr);
    aPos.IncRow();
    m_pDoc->SetString(aPos, u"=global_local"_ustr);
    aPos.IncRow();
    m_pDoc->SetString(aPos, u"=local_global"_ustr);
    aPos.IncRow();
    m_pDoc->SetString(aPos, u"=local_local"_ustr);

    testFormulaRefUpdateNameCopySheetCheckTab( m_pDoc, 0, false);

    // Copy sheet after.
    m_pDoc->CopyTab(0, 1);
    testFormulaRefUpdateNameCopySheetCheckTab( m_pDoc, 0, false);
    testFormulaRefUpdateNameCopySheetCheckTab( m_pDoc, 1, true);

    // Copy sheet before, shifting following now two sheets.
    m_pDoc->CopyTab(1, 0);
    testFormulaRefUpdateNameCopySheetCheckTab( m_pDoc, 0, true);
    testFormulaRefUpdateNameCopySheetCheckTab( m_pDoc, 1, false);
    testFormulaRefUpdateNameCopySheetCheckTab( m_pDoc, 2, true);

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateSheetLocalMove)
{
    SCTAB nSheet1 = 0;
    SCTAB nSheet2 = 1;
    m_pDoc->InsertTab( nSheet1, u"Sheet1"_ustr);
    m_pDoc->InsertTab( nSheet2, u"Sheet2"_ustr);

    ScAddress aPos(0,0,nSheet1);
    bool bOk;
    bOk = m_pDoc->InsertNewRangeName( 0, u"MyCell"_ustr, aPos, u"$Sheet1.$B$2"_ustr);
    CPPUNIT_ASSERT(bOk);
    aPos.SetTab(nSheet2);
    bOk = m_pDoc->InsertNewRangeName( 1, u"MyCell"_ustr, aPos, u"$Sheet2.$B$2"_ustr);
    CPPUNIT_ASSERT(bOk);

    aPos.SetTab(nSheet1);
    aPos.IncCol();
    m_pDoc->SetString( aPos, u"x"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"1.0"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=MyCell"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1.B3", 1.0, m_pDoc->GetValue(aPos));

    aPos.SetTab(nSheet2);
    aPos.SetRow(1);
    m_pDoc->SetString( aPos, u"2.0"_ustr);
    aPos.IncRow();
    m_pDoc->SetString( aPos, u"=MyCell"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2.B3", 2.0, m_pDoc->GetValue(aPos));

    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    OUString aFormula;

    // Move Sheet1.B1 ("x") to Sheet2.B1
    bOk = rFunc.MoveBlock( ScRange(1,0,nSheet1,1,0,nSheet1), ScAddress(1,0,nSheet2), true, false, false, false);
    CPPUNIT_ASSERT(bOk);
    // Results not changed.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move x: Sheet1.B3", 1.0, m_pDoc->GetValue(ScAddress(1,2,nSheet1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move x: Sheet2.B3", 2.0, m_pDoc->GetValue(ScAddress(1,2,nSheet2)));
    // Formulas not changed.
    aFormula = m_pDoc->GetFormula( 1,2,nSheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move x: Sheet1.B3", u"=MyCell"_ustr, aFormula);
    aFormula = m_pDoc->GetFormula( 1,2,nSheet2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move x: Sheet2.B3", u"=MyCell"_ustr, aFormula);

    // Move Sheet2.B2 ("2.0") to Sheet1.C2
    bOk = rFunc.MoveBlock( ScRange(1,1,nSheet2,1,1,nSheet2), ScAddress(2,1,nSheet1), true, false, false, false);
    CPPUNIT_ASSERT(bOk);
    // Results not changed.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move 2.0: Sheet1.B3", 1.0, m_pDoc->GetValue(ScAddress(1,2,nSheet1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move 2.0: Sheet2.B3", 2.0, m_pDoc->GetValue(ScAddress(1,2,nSheet2)));
    // Formulas not changed.
    aFormula = m_pDoc->GetFormula( 1,2,nSheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move 2.0: Sheet1.B3", u"=MyCell"_ustr, aFormula);
    aFormula = m_pDoc->GetFormula( 1,2,nSheet2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move 2.0: Sheet2.B3", u"=MyCell"_ustr, aFormula);

    ScRangeData* pName;

    // Check that the sheet-local named reference points to the moved cell, now
    // Sheet1.C2
    pName = m_pDoc->GetRangeName(nSheet2)->findByUpperName(u"MYCELL"_ustr);
    CPPUNIT_ASSERT(pName);
    aFormula = pName->GetSymbol( ScAddress(), formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move 2.0: Sheet2 sheet-local name", u"$Sheet1.$C$2"_ustr, aFormula);

    // Move Sheet2.B3 ("=MyCell") to Sheet1.C3
    bOk = rFunc.MoveBlock( ScRange(1,2,nSheet2,1,2,nSheet2), ScAddress(2,2,nSheet1), true, false, false, false);
    CPPUNIT_ASSERT(bOk);
    // Results changed.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move =MyCell: Sheet1.B3", 1.0, m_pDoc->GetValue(ScAddress(1,2,nSheet1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move =MyCell: Sheet2.B3", 0.0, m_pDoc->GetValue(ScAddress(1,2,nSheet2)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move =MyCell: Sheet1.C3", 2.0, m_pDoc->GetValue(ScAddress(2,2,nSheet1)));
    // One formula identical, one adjusted.
    aFormula = m_pDoc->GetFormula( 1,2,nSheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move =MyCell: Sheet1.B3", u"=MyCell"_ustr, aFormula);
    aFormula = m_pDoc->GetFormula( 2,2,nSheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move =MyCell: Sheet1.C3", u"=Sheet2.MyCell"_ustr, aFormula);

    // Check that the sheet-local named reference in Sheet1 still points to the
    // original cell Sheet1.B2
    pName = m_pDoc->GetRangeName(nSheet1)->findByUpperName(u"MYCELL"_ustr);
    CPPUNIT_ASSERT(pName);
    aFormula = pName->GetSymbol( ScAddress(), formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move =MyCell: Sheet1 sheet-local name", u"$Sheet1.$B$2"_ustr, aFormula);

    // Check that the sheet-local named reference in Sheet2 still points to the
    // moved cell, now Sheet1.C2
    pName = m_pDoc->GetRangeName(nSheet2)->findByUpperName(u"MYCELL"_ustr);
    CPPUNIT_ASSERT(pName);
    aFormula = pName->GetSymbol( ScAddress(), formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Move =MyCell: Sheet2 sheet-local name", u"$Sheet1.$C$2"_ustr, aFormula);

    // Insert sheet before the others.
    m_pDoc->InsertTab(0, u"Sheet0"_ustr);
    ++nSheet1;
    ++nSheet2;

    // Nothing changed.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Insert Sheet0: Sheet1.B3", 1.0, m_pDoc->GetValue(ScAddress(1,2,nSheet1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Insert Sheet0: Sheet1.C3", 2.0, m_pDoc->GetValue(ScAddress(2,2,nSheet1)));
    aFormula = m_pDoc->GetFormula( 1,2,nSheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Insert Sheet0: Sheet1.B3", u"=MyCell"_ustr, aFormula);
    aFormula = m_pDoc->GetFormula( 2,2,nSheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Insert Sheet0: Sheet1.C3", u"=Sheet2.MyCell"_ustr, aFormula);
    pName = m_pDoc->GetRangeName(nSheet1)->findByUpperName(u"MYCELL"_ustr);
    CPPUNIT_ASSERT(pName);
    aFormula = pName->GetSymbol( ScAddress(), formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Insert Sheet0: Sheet1 sheet-local name", u"$Sheet1.$B$2"_ustr, aFormula);
    pName = m_pDoc->GetRangeName(nSheet2)->findByUpperName(u"MYCELL"_ustr);
    CPPUNIT_ASSERT(pName);
    aFormula = pName->GetSymbol( ScAddress(), formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Insert Sheet0: Sheet2 sheet-local name", u"$Sheet1.$C$2"_ustr, aFormula);

    // Delete sheet before the others.
    m_pDoc->DeleteTab(0);
    --nSheet1;
    --nSheet2;

    // Nothing changed.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet0: Sheet1.B3", 1.0, m_pDoc->GetValue(ScAddress(1,2,nSheet1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet0: Sheet1.C3", 2.0, m_pDoc->GetValue(ScAddress(2,2,nSheet1)));
    aFormula = m_pDoc->GetFormula( 1,2,nSheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet0: Sheet1.B3", u"=MyCell"_ustr, aFormula);
    aFormula = m_pDoc->GetFormula( 2,2,nSheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet0: Sheet1.C3", u"=Sheet2.MyCell"_ustr, aFormula);
    pName = m_pDoc->GetRangeName(nSheet1)->findByUpperName(u"MYCELL"_ustr);
    CPPUNIT_ASSERT(pName);
    aFormula = pName->GetSymbol( ScAddress(), formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet0: Sheet1 sheet-local name", u"$Sheet1.$B$2"_ustr, aFormula);
    pName = m_pDoc->GetRangeName(nSheet2)->findByUpperName(u"MYCELL"_ustr);
    CPPUNIT_ASSERT(pName);
    aFormula = pName->GetSymbol( ScAddress(), formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet0: Sheet2 sheet-local name", u"$Sheet1.$C$2"_ustr, aFormula);

    // Delete last sheet with sheet-local name.
    m_pDoc->DeleteTab(nSheet2);

    // XXX we *could* analyze whether the expression points to a different
    // sheet and then move the name to a remaining sheet. If so, adapt this
    // test.
    // Nothing changed except the sheet-local name and its use.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet2: Sheet1.B3", 1.0, m_pDoc->GetValue(ScAddress(1,2,nSheet1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet2: Sheet1.C3", 0.0, m_pDoc->GetValue(ScAddress(2,2,nSheet1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet2: Sheet1.C3", u"#NAME?"_ustr, m_pDoc->GetString(ScAddress(2,2,nSheet1)));
    aFormula = m_pDoc->GetFormula( 1,2,nSheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet2: Sheet1.B3", u"=MyCell"_ustr, aFormula);
    aFormula = m_pDoc->GetFormula( 2,2,nSheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet2: Sheet1.C3", u"=#NAME?"_ustr, aFormula);
    pName = m_pDoc->GetRangeName(nSheet1)->findByUpperName(u"MYCELL"_ustr);
    CPPUNIT_ASSERT(pName);
    aFormula = pName->GetSymbol( ScAddress(), formula::FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Delete Sheet2: Sheet1 sheet-local name", u"$Sheet1.$B$2"_ustr, aFormula);
    CPPUNIT_ASSERT(!m_pDoc->GetRangeName(nSheet2));
    nSheet2 = -1;

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateNameDelete)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Insert a new name 'MyRange' to reference B1
    bool bInserted = m_pDoc->InsertNewRangeName(u"MyRange"_ustr, ScAddress(0,0,0), u"$Test.$B$1"_ustr);
    CPPUNIT_ASSERT(bInserted);

    const ScRangeData* pName = m_pDoc->GetRangeName()->findByUpperName(u"MYRANGE"_ustr);
    CPPUNIT_ASSERT(pName);

    m_pDoc->DeleteCol(1, 0, 3, 0, 0, 1);
    const ScTokenArray* pCode = pName->GetCode();
    sc::TokenStringContext aCxt(*m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH);
    OUString aExpr = pCode->CreateString(aCxt, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"$Test.$B$1"_ustr, aExpr);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFormulaRefUpdateValidity)
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

    m_pDoc->InsertTab(0, u"Formula"_ustr);

    // Set values in C2:C4.
    m_pDoc->SetValue(ScAddress(2,1,0), 1.0);
    m_pDoc->SetValue(ScAddress(2,2,0), 2.0);
    m_pDoc->SetValue(ScAddress(2,3,0), 3.0);

    // Set validity in A2.
    ScValidationData aData(
        SC_VALID_LIST, ScConditionMode::Equal, u"C2:C4"_ustr, u""_ustr, *m_pDoc, ScAddress(0,1,0), u""_ustr, u""_ustr,
        m_pDoc->GetGrammar(), m_pDoc->GetGrammar());

    sal_uInt32 nIndex = m_pDoc->AddValidationEntry(aData);
    SfxUInt32Item aItem(ATTR_VALIDDATA, nIndex);

    ScPatternAttr aNewAttrs(m_pDoc->getCellAttributeHelper());
    aNewAttrs.GetItemSet().Put(aItem);

    m_pDoc->ApplyPattern(0, 1, 0, aNewAttrs);

    const ScValidationData* pData = m_pDoc->GetValidationEntry(nIndex);
    CPPUNIT_ASSERT(pData);

    // Make sure the list is correct.
    std::vector<ScTypedStrData> aList;
    pData->FillSelectionList(aList, ScAddress(0,1,0));
    bool bGood = aCheck.checkList(aList);
    CPPUNIT_ASSERT_MESSAGE("Initial list is incorrect.", bGood);

    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);

    // Insert a new column at Column B, to move the list from C2:C4 to D2:D4.
    bool bInserted = rFunc.InsertCells(ScRange(1,0,0,1,m_pDoc->MaxRow(),0), &aMark, INS_INSCOLS_BEFORE, true, true);
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

CPPUNIT_TEST_FIXTURE(TestFormula, testTokenArrayRefUpdateMove)
{
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);
    m_pDoc->InsertTab(1, u"Sheet2"_ustr);

    ScAddress aPos(0,0,0); // A1

    sc::TokenStringContext aCxt(*m_pDoc, m_pDoc->GetGrammar());

    // Emulate cell movement from Sheet1.C3 to Sheet2.C3.
    sc::RefUpdateContext aRefCxt(*m_pDoc);
    aRefCxt.meMode = URM_MOVE;
    aRefCxt.maRange = ScAddress(2,2,1); // C3 on Sheet2.
    aRefCxt.mnTabDelta = -1;

    std::vector<OUString> aTests = {
        u"B1*C1"_ustr,
        u"SUM(B1:C1)"_ustr,
        u"$Sheet1.B1"_ustr,
        u"SUM(Sheet1.B1:Sheet2.B1)"_ustr
    };

    // Since C3 is not referenced in any of the above formulas, moving C3 from
    // Sheet1 to Sheet2 should NOT change the displayed formula string at all.

    for (const OUString& aTest : aTests)
    {
        ScCompiler aComp(*m_pDoc, aPos, m_pDoc->GetGrammar());
        std::unique_ptr<ScTokenArray> pArray(aComp.CompileString(aTest));

        OUString aStr = pArray->CreateString(aCxt, aPos);

        CPPUNIT_ASSERT_EQUAL(aTest, aStr);

        // This formula cell isn't moving its position. The displayed formula
        // string should not change.
        pArray->AdjustReferenceOnMove(aRefCxt, aPos, aPos);

        aStr = pArray->CreateString(aCxt, aPos);
        CPPUNIT_ASSERT_EQUAL(aTest, aStr);
    }

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testMultipleOperations)
{
    m_pDoc->InsertTab(0, u"MultiOp"_ustr);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    // Insert the reference formula at top row.
    m_pDoc->SetValue(ScAddress(0,0,0), 1);
    m_pDoc->SetString(ScAddress(1,0,0), u"=A1*10"_ustr);
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(1,0,0)));

    // Insert variable inputs in A3:A5.
    m_pDoc->SetValue(ScAddress(0,2,0), 2);
    m_pDoc->SetValue(ScAddress(0,3,0), 3);
    m_pDoc->SetValue(ScAddress(0,4,0), 4);

    // Set multiple operations range.
    ScTabOpParam aParam;
    aParam.aRefFormulaCell = ScRefAddress(1,0,0);
    aParam.aRefFormulaEnd = aParam.aRefFormulaCell;
    aParam.aRefColCell = ScRefAddress(0,0,0);
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(ScRange(0,2,0,1,4,0)); // Select A3:B5.
    m_pDoc->InsertTableOp(aParam, 0, 2, 1, 4, aMark);
    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(1,2,0));
    CPPUNIT_ASSERT_EQUAL(30.0, m_pDoc->GetValue(1,3,0));
    CPPUNIT_ASSERT_EQUAL(40.0, m_pDoc->GetValue(1,4,0));

    // Clear A3:B5.
    clearRange(m_pDoc, ScRange(0,2,0,1,4,0));

    // This time, use indirect reference formula cell.
    m_pDoc->SetString(ScAddress(2,0,0), u"=B1"_ustr); // C1 simply references B1.
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

CPPUNIT_TEST_FIXTURE(TestFormula, testFuncCOLUMN)
{
    m_pDoc->InsertTab(0, u"Formula"_ustr);
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->SetString(ScAddress(5,10,0), u"=COLUMN()"_ustr);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(5,10,0)));

    m_pDoc->SetString(ScAddress(0,1,0), u"=F11"_ustr);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Move the formula cell with COLUMN() function to change its value.
    m_pDoc->InsertCol(ScRange(5,0,0,5,m_pDoc->MaxRow(),0));
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(ScAddress(6,10,0)));

    // The cell that references the moved cell should update its value as well.
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Move the column in the other direction.
    m_pDoc->DeleteCol(ScRange(5,0,0,5,m_pDoc->MaxRow(),0));

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(5,10,0)));

    // The cell that references the moved cell should update its value as well.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFuncCOUNT)
{
    m_pDoc->InsertTab(0, u"Formula"_ustr);
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->SetValue(ScAddress(0,0,0), 2); // A1
    m_pDoc->SetValue(ScAddress(0,1,0), 4); // A2
    m_pDoc->SetValue(ScAddress(0,2,0), 6); // A3

    ScAddress aPos(1,0,0);
    m_pDoc->SetString(aPos, u"=COUNT(A1:A3)"_ustr);
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, u"=COUNT(A1:A3;2)"_ustr);
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, u"=COUNT(A1:A3;2;4)"_ustr);
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(aPos));

    aPos.IncRow();
    m_pDoc->SetString(aPos, u"=COUNT(A1:A3;2;4;6)"_ustr);
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(aPos));

    // Matrix in C1.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(2, 0, 2, 0, aMark, u"=COUNT(SEARCH(\"a\";{\"a\";\"b\";\"a\"}))"_ustr);
    // Check that the #VALUE! error of "a" not found in "b" is not counted.
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,0,0)));

    // Matrix in C3.
    m_pDoc->InsertMatrixFormula(2, 2, 2, 2, aMark, u"=COUNTA(SEARCH(\"a\";{\"a\";\"b\";\"a\"}))"_ustr);
    // Check that the #VALUE! error of "a" not found in "b" is counted.
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(2,2,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFuncCOUNTBLANK)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.
    m_pDoc->InsertTab(0, u"Formula"_ustr);

    std::vector<std::vector<const char*>> aData = {
        { "1", nullptr, "=B1", "=\"\"" },
        { "2", nullptr, "=B2", "=\"\"" },
        { "A", nullptr, "=B3", "=\"\"" },
        { "B", nullptr, "=B4", "=D3" },
        {   nullptr, nullptr, "=B5", "=D4" },
        { "=COUNTBLANK(A1:A5)", "=COUNTBLANK(B1:B5)", "=COUNTBLANK(C1:C5)", "=COUNTBLANK(D1:D5)" }
    };

    ScAddress aPos(0,0,0);
    ScRange aRange = insertRangeData(m_pDoc, aPos, aData);
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(1,5,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(2,5,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(3,5,0)));

    // Test single cell reference cases.

    clearSheet(m_pDoc, 0);

    std::vector<std::vector<const char*>> aData2 = {
        { "1",     "=COUNTBLANK(A1)" },
        { "A",     "=COUNTBLANK(A2)" },
        {   nullptr,     "=COUNTBLANK(A3)" },
        { "=\"\"", "=COUNTBLANK(A4)" },
        { "=A4"  , "=COUNTBLANK(A5)" },
    };

    aRange = insertRangeData(m_pDoc, aPos, aData2);
    CPPUNIT_ASSERT_EQUAL(aPos, aRange.aStart);

    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFuncROW)
{
    m_pDoc->InsertTab(0, u"Formula"_ustr);
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->SetString(ScAddress(5,10,0), u"=ROW()"_ustr);
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(5,10,0)));

    m_pDoc->SetString(ScAddress(0,1,0), u"=F11"_ustr);
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Insert 2 new rows at row 4.
    m_pDoc->InsertRow(ScRange(0,3,0,m_pDoc->MaxCol(),4,0));
    CPPUNIT_ASSERT_EQUAL(13.0, m_pDoc->GetValue(ScAddress(5,12,0)));

    // The cell that references the moved cell should update its value as well.
    CPPUNIT_ASSERT_EQUAL(13.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Delete 2 rows to move it back.
    m_pDoc->DeleteRow(ScRange(0,3,0,m_pDoc->MaxCol(),4,0));

    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(5,10,0)));

    // The cell that references the moved cell should update its value as well.
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Clear sheet and start over.
    clearSheet(m_pDoc, 0);

    m_pDoc->SetString(ScAddress(0,1,0), u"=ROW(A5)"_ustr);
    m_pDoc->SetString(ScAddress(1,1,0), u"=ROW(B5)"_ustr);
    m_pDoc->SetString(ScAddress(1,2,0), u"=ROW(B6)"_ustr);
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    // B2:B3 should be shared.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,1,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    // Insert a new row at row 4.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    rFunc.InsertCells(ScRange(0,3,0,m_pDoc->MaxCol(),3,0), &aMark, INS_INSROWS_BEFORE, false, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=ROW(A6)"_ustr, m_pDoc->GetFormula(0,1,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=ROW(B6)"_ustr, m_pDoc->GetFormula(1,1,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=ROW(B7)"_ustr, m_pDoc->GetFormula(1,2,0));

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(7.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFuncSUM)
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, u"foo"_ustr));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    // Single argument case.
    m_pDoc->SetValue(ScAddress(0,0,0), 1);
    m_pDoc->SetValue(ScAddress(0,1,0), 1);
    m_pDoc->SetString(ScAddress(0,2,0), u"=SUM(A1:A2)"_ustr);
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

    m_pDoc->SetString(ScAddress(3,0,0), u"=SUM(A1:A2;B1:B2)"_ustr);
    m_pDoc->SetString(ScAddress(3,1,0), u"=SUM(A2:A3;B2:B3)"_ustr);
    m_pDoc->SetString(ScAddress(3,2,0), u"=SUM(A3:A4;B3:B4)"_ustr);
    CPPUNIT_ASSERT_EQUAL(30.0, m_pDoc->GetValue(ScAddress(3,0,0)));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(ScAddress(3,1,0)));
    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(ScAddress(3,2,0)));

    // Clear and start over.
    clearRange(m_pDoc, ScRange(0,0,0,3,m_pDoc->MaxRow(),0));

    // SUM needs to take the first error in case the range contains an error.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 10.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 100.0);
    m_pDoc->SetString(ScAddress(0,3,0), u"=SUM(A1:A3)"_ustr);
    CPPUNIT_ASSERT_EQUAL(111.0, m_pDoc->GetValue(ScAddress(0,3,0)));

    // Set #DIV/0! error to A3. A4 should also inherit this error.
    m_pDoc->SetString(ScAddress(0,2,0), u"=1/0"_ustr);
    FormulaError nErr = m_pDoc->GetErrCode(ScAddress(0,2,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should have a division by zero error.",
                           int(FormulaError::DivisionByZero), static_cast<int>(nErr));
    nErr = m_pDoc->GetErrCode(ScAddress(0,3,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SUM should have also inherited a div-by-zero error.",
                           int(FormulaError::DivisionByZero), static_cast<int>(nErr));

    // Set #NA! to A2. A4 should now inherit this error.
    m_pDoc->SetString(ScAddress(0,1,0), u"=NA()"_ustr);
    nErr = m_pDoc->GetErrCode(ScAddress(0,1,0));
    CPPUNIT_ASSERT_MESSAGE("A2 should be an error.", nErr != FormulaError::NONE);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A4 should have inherited the same error as A2.",
                           static_cast<int>(nErr), static_cast<int>(m_pDoc->GetErrCode(ScAddress(0,3,0))));

    // Test the dreaded 0.1 + 0.2 - 0.3 != 0.0
    m_pDoc->SetString(ScAddress(1,0,0), u"=SUM(0.1;0.2;-0.3)"_ustr);
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    // Also for +/- operators
    m_pDoc->SetString(ScAddress(1,1,0), u"=0.1+0.2-0.3"_ustr);
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1,1,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFuncPRODUCT)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto recalc.

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab(0, u"foo"_ustr));

    ScAddress aPos(3, 0, 0);
    m_pDoc->SetValue(0, 0, 0, 3.0); // A1
    m_pDoc->SetString(aPos, u"=PRODUCT(A1)"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", 3.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(0, 0, 0, -3.0); // A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", -3.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, u"=PRODUCT(B1)"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", 0.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetValue(1, 0, 0, 10.0); // B1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT failed", 10.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetString(aPos, u"=PRODUCT(A1:C3)"_ustr);
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

    m_pDoc->SetString(aPos, u"=PRODUCT({2;3;4})"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", 24.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, u"=PRODUCT({2;-2;2})"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", -8.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, u"=PRODUCT({8;0.125;-1})"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", -1.0, m_pDoc->GetValue(aPos));

    m_pDoc->SetString(aPos, u"=PRODUCT({2;3};{4;5})"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", 120.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, u"=PRODUCT({10;-8};{3;-1};{15;30};{7})"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", 756000.0, m_pDoc->GetValue(aPos));
    m_pDoc->SetString(aPos, u"=PRODUCT({10;-0.1;8};{0.125;4;0.25;2};{0.5};{1};{-1})"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Calculation of PRODUCT with inline array failed", 1.0, m_pDoc->GetValue(aPos));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFuncSUMPRODUCT)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto recalc.

    ScAddress aPos(0,0,0);
    m_pDoc->SetString(aPos, u"=SUMPRODUCT(B1:B3;C1:C3)"_ustr);
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
    m_pDoc->SetString( 2, 1, 0, u"=1/0"_ustr);
    FormulaError nError = m_pDoc->GetErrCode(aPos);
    CPPUNIT_ASSERT_MESSAGE("Formula result should be a propagated error", nError != FormulaError::NONE);

    // Test ForceArray propagation of SUMPRODUCT parameters to ABS and + operator.
    // => ABS({-3,4})*({-3,4}+{-3,4}) => {3,4}*{-6,8} => {-18,32} => 14
    m_pDoc->SetValue(ScAddress(4,0,0), -3.0); // E1
    m_pDoc->SetValue(ScAddress(4,1,0),  4.0); // E2
    // Non-intersecting formula in F3.
    m_pDoc->SetString(ScAddress(5,2,0), u"=SUMPRODUCT(ABS(E1:E2);E1:E2+E1:E2)"_ustr);
    CPPUNIT_ASSERT_EQUAL(14.0, m_pDoc->GetValue(ScAddress(5,2,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFuncSUMXMY2)
{
    m_pDoc->InsertTab(0, u"Test SumXMY2"_ustr);

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto recalc.

    ScAddress aPos(0,0,0);
    m_pDoc->SetString(aPos, u"=SUMXMY2(B1:B3;C1:C3)"_ustr);
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

    m_pDoc->SetString(0, 4, 0, u"=SUMXMY2({2;3;4};{4;3;2})"_ustr);
    double result = m_pDoc->GetValue(0, 4, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Calculation of SUMXMY2 with inline arrays failed", 8.0, result);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFuncMIN)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto recalc.
    m_pDoc->InsertTab(0, u"Formula"_ustr);

    // A1:A2
    m_pDoc->SetString(ScAddress(0,0,0), u"a"_ustr);
    m_pDoc->SetString(ScAddress(0,1,0), u"b"_ustr);

    // B1:B2
    m_pDoc->SetValue(ScAddress(1,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,1,0), 2.0);

    // Matrix in C1:C2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(2, 0, 2, 1, aMark, u"=MIN(IF(A1:A2=\"c\";B1:B2))"_ustr);

    // Formula cell in C1:C2 should be a 1x2 matrix array.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(2,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This formula should be an array.", ScMatrixMode::Formula, pFC->GetMatrixFlag());

    SCCOL nCols;
    SCROW nRows;
    pFC->GetMatColsRows(nCols, nRows);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCols);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), nRows);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula in C1 is invalid.", 0, static_cast<int>(m_pDoc->GetErrCode(ScAddress(2,0,0))));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula in C2 is invalid.", 0, static_cast<int>(m_pDoc->GetErrCode(ScAddress(2,1,0))));

    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(2,1,0)));

    // Inline array input (A4).
    m_pDoc->SetString(ScAddress(0,3,0), u"=MIN({-2;4;3})"_ustr);
    CPPUNIT_ASSERT_EQUAL(-2.0, m_pDoc->GetValue(ScAddress(0,3,0)));

    // Add more values to B3:B4.
    m_pDoc->SetValue(ScAddress(1,2,0),  20.0);
    m_pDoc->SetValue(ScAddress(1,3,0), -20.0);

    // Get the MIN of B1:B4.
    m_pDoc->SetString(ScAddress(2,4,0), u"=MIN(B1:B4)"_ustr);
    CPPUNIT_ASSERT_EQUAL(-20.0, m_pDoc->GetValue(ScAddress(2,4,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFuncN)
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, u"foo"_ustr));

    double result;

    // Clear the area first.
    clearRange(m_pDoc, ScRange(0, 0, 0, 1, 20, 0));

    // Put values to reference.
    double val = 0;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->SetString(0, 2, 0, u"Text"_ustr);
    val = 1;
    m_pDoc->SetValue(0, 3, 0, val);
    val = -1;
    m_pDoc->SetValue(0, 4, 0, val);
    val = 12.3;
    m_pDoc->SetValue(0, 5, 0, val);
    m_pDoc->SetString(0, 6, 0, u"'12.3"_ustr);

    // Cell references
    m_pDoc->SetString(1, 0, 0, u"=N(A1)"_ustr);
    m_pDoc->SetString(1, 1, 0, u"=N(A2)"_ustr);
    m_pDoc->SetString(1, 2, 0, u"=N(A3)"_ustr);
    m_pDoc->SetString(1, 3, 0, u"=N(A4)"_ustr);
    m_pDoc->SetString(1, 4, 0, u"=N(A5)"_ustr);
    m_pDoc->SetString(1, 5, 0, u"=N(A6)"_ustr);
    m_pDoc->SetString(1, 6, 0, u"=N(A9)"_ustr);

    // In-line values
    m_pDoc->SetString(1, 7, 0, u"=N(0)"_ustr);
    m_pDoc->SetString(1, 8, 0, u"=N(1)"_ustr);
    m_pDoc->SetString(1, 9, 0, u"=N(-1)"_ustr);
    m_pDoc->SetString(1, 10, 0, u"=N(123)"_ustr);
    m_pDoc->SetString(1, 11, 0, u"=N(\"\")"_ustr);
    m_pDoc->SetString(1, 12, 0, u"=N(\"12\")"_ustr);
    m_pDoc->SetString(1, 13, 0, u"=N(\"foo\")"_ustr);

    // Range references
    m_pDoc->SetString(2, 2, 0, u"=N(A1:A8)"_ustr);
    m_pDoc->SetString(2, 3, 0, u"=N(A1:A8)"_ustr);
    m_pDoc->SetString(2, 4, 0, u"=N(A1:A8)"_ustr);
    m_pDoc->SetString(2, 5, 0, u"=N(A1:A8)"_ustr);

    // Calculate and check the results.
    m_pDoc->CalcAll();
    double checks1[] = {
        0, 0,  0,    1, -1, 12.3, 0, // cell reference
        0, 1, -1, 123,  0,    0, 0   // in-line values
    };
    for (size_t i = 0; i < SAL_N_ELEMENTS(checks1); ++i)
    {
        result = m_pDoc->GetValue(1, i, 0);
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
        result = m_pDoc->GetValue(1, i+2, 0);
        bool bGood = result == checks2[i];
        if (!bGood)
        {
            cerr << "row " << (i+2+1) << ": expected=" << checks2[i] << " actual=" << result << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for N", false);
        }
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestFormula, testFuncCOUNTIF)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    // COUNTIF (test case adopted from OOo i#36381)

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, u"foo"_ustr));

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
    static const struct {
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
        SCROW nRow = 20 + i;
        double result = m_pDoc->GetValue(0, nRow, 0);
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

    m_pDoc->SetString(0, 0, 0, u"=\"\""_ustr);
    m_pDoc->SetString(0, 1, 0, u"=COUNTIF(A1;1)"_ustr);

    double result = m_pDoc->GetValue(0, 1, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("We shouldn't count empty string as valid number.", 0.0, result);

    // Another test case adopted from fdo#77039.
    clearSheet(m_pDoc, 0);

    // Set formula cells with blank results in A1:A4.
    for (SCROW i = 0; i <=3; ++i)
        m_pDoc->SetString(ScAddress(0,i,0), u"=\"\""_ustr);

    // Insert formula into A5 to count all cells with empty strings.
    m_pDoc->SetString(ScAddress(0,4,0), u"=COUNTIF(A1:A4;\"\""_ustr);

    // We should correctly count with empty string key.
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(0,4,0)));

    // Another test case adopted from tdf#99291, empty array elements should
    // not match empty cells, but cells with 0.
    clearSheet(m_pDoc, 0);
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0,0, 0,1, aMark, u"=COUNTIF(B1:B5;C1:C2)"_ustr);
    // As we will be testing for 0.0 values, check that formulas are actually present.
    OUString aFormula = m_pDoc->GetFormula(0,0,0);
    CPPUNIT_ASSERT_EQUAL(u"{=COUNTIF(B1:B5;C1:C2)}"_ustr, aFormula);
    aFormula = m_pDoc->GetFormula(0,1,0);
    CPPUNIT_ASSERT_EQUAL(u"{=COUNTIF(B1:B5;C1:C2)}"_ustr, aFormula);
    // The 0.0 results expected.
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    // 0.0 in B2, 1.0 in B3 and B4
    m_pDoc->SetValue( ScAddress(1,1,0), 0.0);
    m_pDoc->SetValue( ScAddress(1,2,0), 1.0);
    m_pDoc->SetValue( ScAddress(1,3,0), 1.0);
    // Matched by 0.0 produced by empty cell in array, and 1.0 in C2.
    m_pDoc->SetValue( ScAddress(2,1,0), 1.0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One cell with 0.0",  1.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Two cells with 1.0", 2.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
