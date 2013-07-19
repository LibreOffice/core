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

#include <boost/scoped_ptr.hpp>

using namespace formula;

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

void Test::testFormulaRefData()
{
    ScAddress aAddr(4,5,3), aPos(2,2,2);
    ScSingleRefData aRefData;
    aRefData.InitAddress(aAddr);
    CPPUNIT_ASSERT_MESSAGE("Wrong ref data state.", !aRefData.IsRowRel() && !aRefData.IsColRel() && !aRefData.IsTabRel());
    ASSERT_EQUAL_TYPE(SCCOL, 4, aRefData.GetCol());
    ASSERT_EQUAL_TYPE(SCROW, 5, aRefData.GetRow());
    ASSERT_EQUAL_TYPE(SCTAB, 3, aRefData.GetTab());

    aRefData.SetRowRel(true);
    aRefData.SetColRel(true);
    aRefData.SetTabRel(true);
    aRefData.SetAddress(aAddr, aPos);
    ASSERT_EQUAL_TYPE(SCCOL, 2, aRefData.GetCol());
    ASSERT_EQUAL_TYPE(SCROW, 3, aRefData.GetRow());
    ASSERT_EQUAL_TYPE(SCTAB, 1, aRefData.GetTab());
}

namespace {

OUString toString(
    ScDocument& rDoc, const ScAddress& rPos, ScTokenArray& rArray, FormulaGrammar::Grammar eGram = FormulaGrammar::GRAM_NATIVE)
{
    ScCompiler aComp(&rDoc, rPos, rArray);
    aComp.SetGrammar(eGram);
    OUStringBuffer aBuf;
    aComp.CreateStringFromTokenArray(aBuf);
    return aBuf.makeStringAndClear();
}

ScTokenArray* getTokens(ScDocument& rDoc, const ScAddress& rPos)
{
    ScFormulaCell* pCell = rDoc.GetFormulaCell(rPos);
    if (!pCell)
        return NULL;

    return pCell->GetCode();
}

bool checkFormula(ScDocument& rDoc, const ScAddress& rPos, const char* pExpected)
{
    ScTokenArray* pCode = getTokens(rDoc, rPos);
    if (!pCode)
    {
        cerr << "Empty token array." << endl;
        return false;
    }

    OUString aFormula = toString(rDoc, rPos, *pCode);
    if (aFormula != OUString::createFromAscii(pExpected))
    {
        cerr << "Formula '" << pExpected << "' expected, but '" << aFormula << "' found" << endl;
        return false;
    }

    return true;
}

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
            ScCompiler aComp(m_pDoc, ScAddress());
            aComp.SetGrammar(aTests[i].eInputGram);
            pArray.reset(aComp.CompileString(OUString::createFromAscii(aTests[i].pInput)));
            CPPUNIT_ASSERT_MESSAGE("Token array shouldn't be NULL!", pArray.get());
        }

        OUString aFormula = toString(*m_pDoc, ScAddress(), *pArray, aTests[i].eOutputGram);
        CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(aTests[i].pOutput), aFormula);
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

    // ------------------------------------------
    // Test range updates
    // ------------------------------------------

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

    // Delete row 3. This should shrink the range references by one row.
    m_pDoc->DeleteRow(ScRange(0,2,0,MAXCOL,2,0));

    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "SUM(B2:C4)"))
        CPPUNIT_FAIL("Wrong formula in A6.");

    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "SUM($B$2:$C$4)"))
        CPPUNIT_FAIL("Wrong formula in A7.");

    m_pDoc->DeleteTab(0);
}

void Test::testFuncSUM()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    double val = 1;
    double result;
    m_pDoc->SetValue (0, 0, 0, val);
    m_pDoc->SetValue (0, 1, 0, val);
    m_pDoc->SetString (0, 2, 0, OUString("=SUM(A1:A2)"));
    m_pDoc->CalcAll();
    m_pDoc->GetValue (0, 2, 0, result);
    CPPUNIT_ASSERT_MESSAGE ("calculation failed", result == 2.0);

    m_pDoc->DeleteTab(0);
}

void Test::testFuncPRODUCT()
{
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
    m_pDoc->CalcAll();
    m_pDoc->GetValue(0, 3, 0, result);
    CPPUNIT_ASSERT_MESSAGE("Calculation of PRODUCT failed", result == 6.0);

    m_pDoc->SetString(0, 4, 0, OUString("=PRODUCT({1;2;3})"));
    m_pDoc->CalcAll();
    m_pDoc->GetValue(0, 4, 0, result);
    CPPUNIT_ASSERT_MESSAGE("Calculation of PRODUCT with inline array failed", result == 6.0);

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
    m_pDoc->CalcAll();

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
    m_pDoc->CalcAll();

    double result = m_pDoc->GetValue(0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("We shouldn't count empty string as valid number.", result == 0.0);

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
                << " criterion='" << aChecks[i].pVal << "'" << endl;
            CPPUNIT_ASSERT_MESSAGE("Unexpected result for MATCH", false);
        }
    }
}

void Test::testFuncMATCH()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    clearRange(m_pDoc, ScRange(0, 0, 0, 4, 40, 0));
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
    ScInterpreter::SetGlobalConfig(aConfig);
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
    ScInterpreter::SetGlobalConfig(aConfig);
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
    ScInterpreter::SetGlobalConfig(aConfig);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
