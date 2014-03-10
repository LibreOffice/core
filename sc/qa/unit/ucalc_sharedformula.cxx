/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ucalc.hxx"
#include "editutil.hxx"
#include "formulacell.hxx"
#include "cellvalue.hxx"
#include "docsh.hxx"
#include "clipparam.hxx"
#include "undoblk.hxx"
#include "scopetools.hxx"
#include <docfunc.hxx>
#include <tokenarray.hxx>
#include <tokenstringcontext.hxx>
#include "svl/sharedstring.hxx"

#include "formula/grammar.hxx"

void Test::testSharedFormulas()
{
    m_pDoc->InsertTab(0, "Test");

    ScAddress aPos(1, 9, 0); // B10
    m_pDoc->SetString(aPos, "=A10*2"); // Insert into B10.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("Expected to be a non-shared cell.", pFC && !pFC->IsShared());

    aPos.SetRow(10); // B11
    m_pDoc->SetString(aPos, "=A11*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    aPos.SetRow(8); // B9
    m_pDoc->SetString(aPos, "=A9*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    aPos.SetRow(12); // B13
    m_pDoc->SetString(aPos, "=A13*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This formula cell shouldn't be shared yet.", pFC && !pFC->IsShared());

    // Insert a formula to B12, and B9:B13 should be shared.
    aPos.SetRow(11); // B12
    m_pDoc->SetString(aPos, "=A12*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(5), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Insert formulas to B15:B16.
    aPos.SetRow(14); // B15
    m_pDoc->SetString(aPos, "=A15*2");
    aPos.SetRow(15); // B16
    m_pDoc->SetString(aPos, "=A16*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(14), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Insert a formula to B14, and B9:B16 should be shared.
    aPos.SetRow(13); // B14
    m_pDoc->SetString(aPos, "=A14*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Insert an incompatible formula to B12, to split the shared range to B9:B11 and B13:B16.
    aPos.SetRow(11); // B12
    m_pDoc->SetString(aPos, "=$A$1*4");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell shouldn't be shared.", pFC && !pFC->IsShared());

    aPos.SetRow(8); // B9
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    aPos.SetRow(12); // B13
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Extend B13:B16 to B13:B20.
    aPos.SetRow(16); // B17
    m_pDoc->SetString(aPos, "=A17*2");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A18*2");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A19*2");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A20*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    // B13:B20 shuld be shared.
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Empty B19. This should split it into B13:B18, and B20 non-shared.
    aPos.SetRow(18);
    m_pDoc->SetEmptyCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell should have been emptied.", m_pDoc->GetCellType(aPos) == CELLTYPE_NONE);
    aPos.SetRow(12); // B13
    pFC = m_pDoc->GetFormulaCell(aPos);
    // B13:B18 should be shared.
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());
    // B20 shold be non-shared.
    aPos.SetRow(19); // B20
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B20 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This cell should be non-shared.", !pFC->IsShared());

    // Empty B14, to make B13 non-shared and B15:B18 shared.
    aPos.SetRow(13); // B14
    m_pDoc->SetEmptyCell(aPos);
    aPos.SetRow(12); // B13
    pFC = m_pDoc->GetFormulaCell(aPos);
    // B13 should be non-shared.
    CPPUNIT_ASSERT_MESSAGE("B13 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This cell should be non-shared.", !pFC->IsShared());
    // B15:B18 should be shared.
    aPos.SetRow(14); // B15
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(14), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Set numeric value to B15, to make B16:B18 shared.
    aPos.SetRow(14);
    m_pDoc->SetValue(aPos, 1.2);
    aPos.SetRow(15);
    pFC = m_pDoc->GetFormulaCell(aPos);
    // B16:B18 should be shared.
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(15), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Set string value to B16 to make B17:B18 shared.
    aPos.SetRow(15);
    ScCellValue aCell(svl::SharedString("Test"));
    CPPUNIT_ASSERT_MESSAGE("This should be a string value.", aCell.meType == CELLTYPE_STRING);
    aCell.commit(*m_pDoc, aPos);
    CPPUNIT_ASSERT_EQUAL(aCell.mpString->getString(), m_pDoc->GetString(aPos));
    aPos.SetRow(16);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT(pFC);
    // B17:B18 should be shared.
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(16), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Set edit text to B17. Now B18 should be non-shared.
    ScFieldEditEngine& rEditEngine = m_pDoc->GetEditEngine();
    rEditEngine.SetText("Edit Text");
    aPos.SetRow(16);
    m_pDoc->SetEditText(aPos, rEditEngine.CreateTextObject());
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, m_pDoc->GetCellType(aPos));
    aPos.SetRow(17);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B18 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("B18 should be non-shared.", !pFC->IsShared());

    // Set up a new group for shared formulas in B2:B10.
    clearRange(m_pDoc, ScRange(0,0,0,2,100,0));

    aPos.SetRow(1);
    m_pDoc->SetString(aPos, "=A2*10");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A3*10");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A4*10");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A5*10");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A6*10");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A7*10");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A8*10");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A9*10");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A10*10");

    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B10 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Delete A4:B8. This should splite the grouping to B2:B3 and B9:B10.
    clearRange(m_pDoc, ScRange(0,3,0,1,7,0));
    aPos.SetRow(1);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B2 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    aPos.SetRow(8);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B9 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Delete rows 4:8 and shift row 9 and below up to row 4.  This should
    // re-merge the two into a group of B2:B5.
    m_pDoc->DeleteRow(ScRange(0,3,0,MAXCOL,7,0));
    aPos.SetRow(1);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B2 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Insert 2 rows at row 4, to split it into B2:B3 and B6:B7.
    m_pDoc->InsertRow(ScRange(0,3,0,MAXCOL,4,0));
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B2 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    aPos.SetRow(5);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B6 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(5), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    m_pDoc->DeleteTab(0);
}

void Test::testSharedFormulasRefUpdate()
{
    m_pDoc->InsertTab(0, "Test");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, false); // turn off auto calculation.

    // Set values to A10:A12.
    m_pDoc->SetValue(ScAddress(0,9,0), 1);
    m_pDoc->SetValue(ScAddress(0,10,0), 2);
    m_pDoc->SetValue(ScAddress(0,11,0), 3);

    // Insert formulas that reference A10:A12 in B1:B3.
    m_pDoc->SetString(ScAddress(1,0,0), "=A10");
    m_pDoc->SetString(ScAddress(1,1,0), "=A11");
    m_pDoc->SetString(ScAddress(1,2,0), "=A12");

    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "A10"))
        CPPUNIT_FAIL("Wrong formula in B1");
    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "A11"))
        CPPUNIT_FAIL("Wrong formula in B2");
    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "A12"))
        CPPUNIT_FAIL("Wrong formula in B3");

    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("This must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());

    // Insert cells over A11:B11 to shift to right. This should split the B1:B3 grouping into 3.
    m_pDoc->InsertCol(ScRange(0,10,0,1,10,0));
    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "A10"))
        CPPUNIT_FAIL("Wrong formula in B1");
    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "C11"))
        CPPUNIT_FAIL("Wrong formula in B2");
    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "A12"))
        CPPUNIT_FAIL("Wrong formula in B3");

    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("B1 should be a non-shared formula cell.", pFC && !pFC->IsShared());
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,1,0));
    CPPUNIT_ASSERT_MESSAGE("B2 should be a non-shared formula cell.", pFC && !pFC->IsShared());
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,2,0));
    CPPUNIT_ASSERT_MESSAGE("B3 should be a non-shared formula cell.", pFC && !pFC->IsShared());

    // Delelte cells over A11:B11 to bring it back to the previous state.
    m_pDoc->DeleteCol(ScRange(0,10,0,1,10,0));

    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "A10"))
        CPPUNIT_FAIL("Wrong formula in B1");
    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "A11"))
        CPPUNIT_FAIL("Wrong formula in B2");
    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "A12"))
        CPPUNIT_FAIL("Wrong formula in B3");

    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("This must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());

    // Insert cells over A11:A12 and shift down.
    m_pDoc->InsertRow(ScRange(0,10,0,0,11,0));
    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "A10"))
        CPPUNIT_FAIL("Wrong formula in B1");
    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "A13"))
        CPPUNIT_FAIL("Wrong formula in B2");
    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "A14"))
        CPPUNIT_FAIL("Wrong formula in B3");

    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("B1 should be a non-shared formula cell.", pFC && !pFC->IsShared());
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,1,0));
    CPPUNIT_ASSERT_MESSAGE("This must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    // Delete A11:A12 to bring it back to the way it was.
    m_pDoc->DeleteRow(ScRange(0,10,0,0,11,0));

    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "A10"))
        CPPUNIT_FAIL("Wrong formula in B1");
    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "A11"))
        CPPUNIT_FAIL("Wrong formula in B2");
    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "A12"))
        CPPUNIT_FAIL("Wrong formula in B3");

    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("This must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());

    // Insert cells over A11:B11 to shift to right again.
    m_pDoc->InsertCol(ScRange(0,10,0,1,10,0));
    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "A10"))
        CPPUNIT_FAIL("Wrong formula in B1");
    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "C11"))
        CPPUNIT_FAIL("Wrong formula in B2");
    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "A12"))
        CPPUNIT_FAIL("Wrong formula in B3");

    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("B1 should be a non-shared formula cell.", pFC && !pFC->IsShared());
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,1,0));
    CPPUNIT_ASSERT_MESSAGE("B2 should be a non-shared formula cell.", pFC && !pFC->IsShared());
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,2,0));
    CPPUNIT_ASSERT_MESSAGE("B3 should be a non-shared formula cell.", pFC && !pFC->IsShared());

    // Insert cells over A12:B12 to shift to right.
    m_pDoc->InsertCol(ScRange(0,11,0,1,11,0));
    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "A10"))
        CPPUNIT_FAIL("Wrong formula in B1");
    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "C11"))
        CPPUNIT_FAIL("Wrong formula in B2");
    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "C12"))
        CPPUNIT_FAIL("Wrong formula in B3");

    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("B1 should be a non-shared formula cell.", pFC && !pFC->IsShared());
    // B2 and B3 should be grouped.
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,1,0));
    CPPUNIT_ASSERT_MESSAGE("This must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    // Insert cells over A10:B10 to shift to right.
    m_pDoc->InsertCol(ScRange(0,9,0,1,9,0));
    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "C10"))
        CPPUNIT_FAIL("Wrong formula in B1");
    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "C11"))
        CPPUNIT_FAIL("Wrong formula in B2");
    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "C12"))
        CPPUNIT_FAIL("Wrong formula in B3");

    // B1:B3 should be now grouped.
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("This must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());

    m_pDoc->DeleteTab(0);
}

void Test::testSharedFormulasRefUpdateRange()
{
    m_pDoc->InsertTab(0, "Test");

    // Insert values to A3:A5.
    m_pDoc->SetValue(ScAddress(0,2,0), 1);
    m_pDoc->SetValue(ScAddress(0,3,0), 2);
    m_pDoc->SetValue(ScAddress(0,4,0), 3);

    // Insert formulas to B3:B5.
    m_pDoc->SetString(ScAddress(1,2,0), "=SUM($A$3:$A$5)");
    m_pDoc->SetString(ScAddress(1,3,0), "=SUM($A$3:$A$5)");
    m_pDoc->SetString(ScAddress(1,4,0), "=SUM($A$3:$A$5)");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "SUM($A$3:$A$5)"))
        CPPUNIT_FAIL("Wrong formula");
    if (!checkFormula(*m_pDoc, ScAddress(1,3,0), "SUM($A$3:$A$5)"))
        CPPUNIT_FAIL("Wrong formula");
    if (!checkFormula(*m_pDoc, ScAddress(1,4,0), "SUM($A$3:$A$5)"))
        CPPUNIT_FAIL("Wrong formula");

    // B3:B5 should be shared.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,2,0));
    CPPUNIT_ASSERT_MESSAGE("B3 should be shared.", pFC && pFC->IsShared());
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,3,0));
    CPPUNIT_ASSERT_MESSAGE("B4 should be shared.", pFC && pFC->IsShared());
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,4,0));
    CPPUNIT_ASSERT_MESSAGE("B3 should be shared.", pFC && pFC->IsShared());

    // Insert 2 rows at row 1.
    m_pDoc->InsertRow(ScRange(0,0,0,MAXCOL,1,0));

    // B5:B7 should be shared.
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,4,0));
    CPPUNIT_ASSERT_MESSAGE("B5 should be shared.", pFC && pFC->IsShared());
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,5,0));
    CPPUNIT_ASSERT_MESSAGE("B6 should be shared.", pFC && pFC->IsShared());
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,6,0));
    CPPUNIT_ASSERT_MESSAGE("B7 should be shared.", pFC && pFC->IsShared());

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());

    if (!checkFormula(*m_pDoc, ScAddress(1,4,0), "SUM($A$5:$A$7)"))
        CPPUNIT_FAIL("Wrong formula");
    if (!checkFormula(*m_pDoc, ScAddress(1,5,0), "SUM($A$5:$A$7)"))
        CPPUNIT_FAIL("Wrong formula");
    if (!checkFormula(*m_pDoc, ScAddress(1,6,0), "SUM($A$5:$A$7)"))
        CPPUNIT_FAIL("Wrong formula");

    m_pDoc->DeleteTab(0);
}

void Test::testSharedFormulasRefUpdateExternal()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.
    m_pDoc->InsertTab(0, "Formula");

    // Launch an external document shell.
    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString aExtDocName("file:///extdata.fake");

    SfxMedium* pMed = new SfxMedium(aExtDocName, STREAM_STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    ScDocument* pExtDoc = xExtDocSh->GetDocument();

    // Populate A1:A3.
    pExtDoc->InsertTab(0, "Data");
    pExtDoc->SetString(ScAddress(0,0,0), "A");
    pExtDoc->SetString(ScAddress(0,1,0), "B");
    pExtDoc->SetString(ScAddress(0,2,0), "C");

    // Insert formula cells in A7:A10 of the host document, referencing A1:A3
    // of the external document.
    m_pDoc->SetString(ScAddress(0,6,0), "='file:///extdata.fake'#$Data.A1");
    m_pDoc->SetString(ScAddress(0,7,0), "='file:///extdata.fake'#$Data.A2");
    m_pDoc->SetString(ScAddress(0,8,0), "='file:///extdata.fake'#$Data.A3");
    m_pDoc->SetString(ScAddress(0,9,0), "=COUNTA('file:///extdata.fake'#$Data.A1:A3)");

    // Check the formula results.
    CPPUNIT_ASSERT_EQUAL(OUString("A"), m_pDoc->GetString(ScAddress(0,6,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("B"), m_pDoc->GetString(ScAddress(0,7,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("C"), m_pDoc->GetString(ScAddress(0,8,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,9,0)));

    // Check the formulas too.
    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "'file:///extdata.fake'#$Data.A1"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,7,0), "'file:///extdata.fake'#$Data.A2"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,8,0), "'file:///extdata.fake'#$Data.A3"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,9,0), "COUNTA('file:///extdata.fake'#$Data.A1:A3)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Delete rows 1 and 2. This should not change the references in the formula cells below.
    ScDocFunc& rDocFunc = getDocShell().GetDocFunc();
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    rDocFunc.DeleteCells(ScRange(0,0,0,MAXCOL,1,0), &aMark, DEL_CELLSUP, true, true);

    // Check the shifted formula cells now in A5:A8.
    if (!checkFormula(*m_pDoc, ScAddress(0,4,0), "'file:///extdata.fake'#$Data.A1"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "'file:///extdata.fake'#$Data.A2"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "'file:///extdata.fake'#$Data.A3"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,7,0), "COUNTA('file:///extdata.fake'#$Data.A1:A3)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Undo and check the formulas again.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();
    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "'file:///extdata.fake'#$Data.A1"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,7,0), "'file:///extdata.fake'#$Data.A2"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,8,0), "'file:///extdata.fake'#$Data.A3"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,9,0), "COUNTA('file:///extdata.fake'#$Data.A1:A3)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Redo the row deletion and check the formulas again.
    pUndoMgr->Redo();
    if (!checkFormula(*m_pDoc, ScAddress(0,4,0), "'file:///extdata.fake'#$Data.A1"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,5,0), "'file:///extdata.fake'#$Data.A2"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,6,0), "'file:///extdata.fake'#$Data.A3"))
        CPPUNIT_FAIL("Wrong formula!");
    if (!checkFormula(*m_pDoc, ScAddress(0,7,0), "COUNTA('file:///extdata.fake'#$Data.A1:A3)"))
        CPPUNIT_FAIL("Wrong formula!");

    xExtDocSh->DoClose();

    m_pDoc->DeleteTab(0);
}

void Test::testSharedFormulasDeleteRows()
{
    m_pDoc->InsertTab(0, "Test");
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    // Fill data cells A1:A10 and formula cells B1:B10
    for (SCROW i = 0; i <= 9 ; ++i)
    {
        m_pDoc->SetValue(0, i, 0, i);
        m_pDoc->SetString(1, i, 0, "=RC[-1]+1");
    }
    // Fill data cells A11:A20 and formula cells B11:B20 with a different formula.
    for (SCROW i = 10; i <= 19 ; ++i)
    {
        m_pDoc->SetValue(0, i, 0, i);
        m_pDoc->SetString(1, i, 0, "=RC[-1]+11");
    }

    // B1:B10 should be shared.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("1,0 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());
    // B11:B20 should be shared.
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,10,0));
    CPPUNIT_ASSERT_MESSAGE("1,10 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

    // Delete rows 9:12
    m_pDoc->DeleteRow(ScRange(0,8,0,MAXCOL,11,0));

    // B1:B8 should be shared.
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("1,0 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());
    // B9:B16 should be shared.
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,8,0));
    CPPUNIT_ASSERT_MESSAGE("1,8 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());

    // Delete row 3
    m_pDoc->DeleteRow(ScRange(0,2,0,MAXCOL,2,0));

    // B1:B7 should be shared.
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("1,0 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(7), pFC->GetSharedLength());
    // B8:B15 should be shared.
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,7,0));
    CPPUNIT_ASSERT_MESSAGE("1,7 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(7), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());

    // Delete row 5
    m_pDoc->DeleteRow(ScRange(0,4,0,MAXCOL,4,0));

    // B1:B6 should be shared.
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("1,0 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedLength());
    // B7:B14 should be shared.
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,6,0));
    CPPUNIT_ASSERT_MESSAGE("1,6 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());
}

void Test::testSharedFormulasDeleteColumns()
{
    using namespace formula;

    m_pDoc->InsertTab(0, "Test");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    ScMarkData aMark;
    aMark.SelectOneTable(0);

    // First, test a single cell case.  A value in B1 and formula in C1.
    m_pDoc->SetValue(ScAddress(1,0,0), 11.0);
    m_pDoc->SetString(ScAddress(2,0,0), "=RC[-1]");
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(2,0,0)));

    // Delete column B.
    rFunc.DeleteCells(ScRange(1,0,0,1,MAXROW,0), &aMark, DEL_CELLSLEFT, true, true);
    CPPUNIT_ASSERT_EQUAL(OUString("#REF!"), m_pDoc->GetString(ScAddress(1,0,0)));

    // The reference should still point to row 1 but the column status should be set to 'deleted'.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT(pFC);
    const ScTokenArray* pCode = pFC->GetCode();
    CPPUNIT_ASSERT(pCode && pCode->GetLen() == 1);
    const FormulaToken* pToken = pCode->GetArray()[0];
    CPPUNIT_ASSERT(pToken->GetType() == svSingleRef);
    const ScSingleRefData* pSRef = &static_cast<const ScToken*>(pToken)->GetSingleRef();
    CPPUNIT_ASSERT(pSRef->IsColDeleted());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pSRef->toAbs(ScAddress(1,0,0)).Row());

    // The formula string should show #REF! in lieu of the column position (only for Calc A1 syntax).
    sc::CompileFormulaContext aCFCxt(m_pDoc, FormulaGrammar::GRAM_ENGLISH);
    CPPUNIT_ASSERT_EQUAL(OUString("=#REF!1"), pFC->GetFormula(aCFCxt));

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    // Undo and make sure the deleted flag is gone.
    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(ScAddress(2,0,0)));
    pFC = m_pDoc->GetFormulaCell(ScAddress(2,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(OUString("=B1"), pFC->GetFormula(aCFCxt));

    // Clear row 1 and move over to a formula group case.
    clearRange(m_pDoc, ScRange(0,0,0,MAXCOL,0,0));

    // Fill A1:B2 with numbers, and C1:C2 with formula that reference those numbers.
    for (SCROW i = 0; i <= 1; ++i)
    {
        m_pDoc->SetValue(ScAddress(0,i,0), (i+1));
        m_pDoc->SetValue(ScAddress(1,i,0), (i+11));
        m_pDoc->SetString(ScAddress(2,i,0), "=RC[-2]+RC[-1]");
        double fCheck = m_pDoc->GetValue(ScAddress(0,i,0));
        fCheck += m_pDoc->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_EQUAL(fCheck, m_pDoc->GetValue(ScAddress(2,i,0)));
    }

    // Delete column B.
    rFunc.DeleteCells(ScRange(1,0,0,1,MAXROW,0), &aMark, DEL_CELLSLEFT, true, true);

    for (SCROW i = 0; i <= 1; ++i)
    {
        ScAddress aPos(1,i,0);
        CPPUNIT_ASSERT_EQUAL(OUString("#REF!"), m_pDoc->GetString(aPos));
    }

    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0)); // B1
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(OUString("=A1+#REF!1"), pFC->GetFormula(aCFCxt));
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,1,0)); // B2
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(OUString("=A2+#REF!2"), pFC->GetFormula(aCFCxt));

    // Undo deletion of column B and check the results of C1:C2.
    pUndoMgr->Undo();
    for (SCROW i = 0; i <= 1; ++i)
    {
        double fCheck = m_pDoc->GetValue(ScAddress(0,i,0));
        fCheck += m_pDoc->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_EQUAL(fCheck, m_pDoc->GetValue(ScAddress(2,i,0)));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testSharedFormulasRefUpdateMoveSheets()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");
    m_pDoc->InsertTab(2, "Sheet3");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // make sure auto calc is on.

    // Switch to R1C1 for ease of repeated formula insertions.
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    // Fill numbers in A1:A8 on Sheet2.
    for (SCROW i = 0; i <= 7; ++i)
        m_pDoc->SetValue(ScAddress(0,i,1), i+1);

    // Fill formula cells A1:A8 on Sheet1, to refer to the same cell address on Sheet2.
    for (SCROW i = 0; i <= 7; ++i)
        m_pDoc->SetString(ScAddress(0,i,0), "=Sheet2!RC");

    // Check the results.
    for (SCROW i = 0; i <= 7; ++i)
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(i+1), m_pDoc->GetValue(ScAddress(0,i,0)));

    // Move Sheet3 to the leftmost position before Sheet1.
    m_pDoc->MoveTab(2, 0);

    // Check sheet names.
    std::vector<OUString> aTabNames = m_pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_MESSAGE("There should be at least 3 sheets.", aTabNames.size() >= 3);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet3"), aTabNames[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aTabNames[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aTabNames[2]);

    // Check the results again on Sheet1.
    for (SCROW i = 0; i <= 7; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(i+1), m_pDoc->GetValue(ScAddress(0,i,1)));
        if (!checkFormula(*m_pDoc, ScAddress(0,i,1), "Sheet2!RC"))
            CPPUNIT_FAIL("Wrong formula expression.");
    }

    // Insert a new sheet at the left end.
    m_pDoc->InsertTab(0, "Sheet4");

    // Check sheet names.
    aTabNames = m_pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_MESSAGE("There should be at least 4 sheets.", aTabNames.size() >= 4);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet4"), aTabNames[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet3"), aTabNames[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aTabNames[2]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aTabNames[3]);

    // Check the results again on Sheet1.
    for (SCROW i = 0; i <= 7; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(i+1), m_pDoc->GetValue(ScAddress(0,i,2)));
        if (!checkFormula(*m_pDoc, ScAddress(0,i,2), "Sheet2!RC"))
            CPPUNIT_FAIL("Wrong formula expression.");
    }

    // Delete Sheet4.
    m_pDoc->DeleteTab(0);

    // Check sheet names.
    aTabNames = m_pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_MESSAGE("There should be at least 3 sheets.", aTabNames.size() >= 3);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet3"), aTabNames[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aTabNames[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aTabNames[2]);

    // Check the results again on Sheet1.
    for (SCROW i = 0; i <= 7; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(i+1), m_pDoc->GetValue(ScAddress(0,i,1)));
        if (!checkFormula(*m_pDoc, ScAddress(0,i,1), "Sheet2!RC"))
            CPPUNIT_FAIL("Wrong formula expression.");
    }

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testSharedFormulasRefUpdateCopySheets()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // make sure auto calc is on.

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetValue(ScAddress(0,0,1), 1.0); // A1 on Sheet2
    m_pDoc->SetValue(ScAddress(0,1,1), 2.0); // A2 on Sheet2

    // Reference values on Sheet2, but use absolute sheet references.
    m_pDoc->SetString(ScAddress(0,0,0), "=$Sheet2.A1");
    m_pDoc->SetString(ScAddress(0,1,0), "=$Sheet2.A2");

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Copy Sheet1 and insert the copied sheet before the current Sheet1 position.
    m_pDoc->CopyTab(0, 0);

    if (!checkFormula(*m_pDoc, ScAddress(0,0,0), "$Sheet2.A1"))
        CPPUNIT_FAIL("Wrong formula");

    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "$Sheet2.A2"))
        CPPUNIT_FAIL("Wrong formula");

    // Check the values on the copied sheet.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Check the values on the original sheet.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,0,1)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0,1,1)));

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testSharedFormulasCopyPaste()
{
    m_pDoc->InsertTab(0, "Test");
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    // Fill formula cells B1:B10.
    for (SCROW i = 0; i <= 9; ++i)
        m_pDoc->SetString(1, i, 0, "=RC[-1]");

    ScAddress aPos(1, 8, 0); // B9
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B9 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Copy formulas in B6:B9 to the clipboard doc.
    ScRange aSrcRange(1,5,0,1,8,0); // B6:B9
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aSrcRange, &aClipDoc);
    pFC = aClipDoc.GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B9 in the clip doc should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(5), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    // Paste them to C2:C10.
    ScRange aDestRange(2,1,0,2,9,0);
    pasteFromClip(m_pDoc, aDestRange, &aClipDoc);
    aPos.SetCol(2);
    aPos.SetRow(1);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("C2 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    ScRange aRange(1,0,0,1,9,0); // B1:B10
    ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
    pUndoDoc->InitUndo(m_pDoc, 0, 0, true, true);
    m_pDoc->CopyToDocument(aRange, IDF_CONTENTS, false, pUndoDoc);
    boost::scoped_ptr<ScUndoPaste> pUndo(createUndoPaste(getDocShell(), aRange, pUndoDoc));

    // First, make sure the formula cells are shared in the undo document.
    aPos.SetCol(1);
    for (SCROW i = 0; i <= 9; ++i)
    {
        aPos.SetRow(i);
        pFC = pUndoDoc->GetFormulaCell(aPos);
        CPPUNIT_ASSERT_MESSAGE("Must be a formula cell.", pFC);
        CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
        CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());
        CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());
    }

    // Overwrite B1:B10.
    for (SCROW i = 0; i <= 9; ++i)
        m_pDoc->SetValue(ScAddress(1,i,0), i*10);

    for (SCROW i = 0; i <= 9; ++i)
        CPPUNIT_ASSERT_MESSAGE("Numeric cell was expected.", m_pDoc->GetCellType(ScAddress(1,i,0)) == CELLTYPE_VALUE);

    // Undo the action to fill B1:B10 with formula cells again.
    pUndo->Undo();

    aPos.SetCol(1);
    for (SCROW i = 0; i <= 9; ++i)
    {
        aPos.SetRow(i);
        pFC = m_pDoc->GetFormulaCell(aPos);
        CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
        CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
        CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());
        CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());
    }

    m_pDoc->DeleteTab(0);
}

void Test::testSharedFormulaInsertColumn()
{
    m_pDoc->InsertTab(0, "Test");

    // Set shared formula group over H2:H3.
    m_pDoc->SetString(ScAddress(7,1,0), "=G3*B3");
    m_pDoc->SetString(ScAddress(7,2,0), "=G4*B4");

    // Insert a single column at Column F. This used to crash before fdo#74041.
    m_pDoc->InsertCol(ScRange(5,0,0,5,MAXROW,0));

    if (!checkFormula(*m_pDoc, ScAddress(8,1,0), "H3*B3"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(*m_pDoc, ScAddress(8,2,0), "H4*B4"))
        CPPUNIT_FAIL("Wrong formula!");

    m_pDoc->DeleteTab(0);
}

void Test::testSharedFormulaMoveBlock()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    m_pDoc->InsertTab(0, "Test");

    // Set values to A1:A3.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 2.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 3.0);

    // Set formulas in B1:B3 to reference A1:A3.
    m_pDoc->SetString(ScAddress(1,0,0), "=RC[-1]");
    m_pDoc->SetString(ScAddress(1,1,0), "=RC[-1]");
    m_pDoc->SetString(ScAddress(1,2,0), "=RC[-1]");

    ScRange aFormulaRange(1,0,0,1,2,0);

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    clearFormulaCellChangedFlag(*m_pDoc, aFormulaRange);

    // Move A1:A3 to D1:D3.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    rFunc.MoveBlock(ScRange(0,0,0,0,2,0), ScAddress(3,0,0), true, true, false, true);

    // The result should stay the same.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    clearFormulaCellChangedFlag(*m_pDoc, aFormulaRange);

    // Make sure these formula cells in B1:B3 have correct positions even after the move.
    std::vector<SCROW> aRows;
    aRows.push_back(0);
    aRows.push_back(1);
    aRows.push_back(2);
    bool bRes = checkFormulaPositions(*m_pDoc, 0, 1, &aRows[0], aRows.size());
    CPPUNIT_ASSERT(bRes);

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    // Undo and check the result.
    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    clearFormulaCellChangedFlag(*m_pDoc, aFormulaRange);

    // Redo and check the result.
    pUndoMgr->Redo();
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,2,0)));

    // Clear the range and start over.
    clearRange(m_pDoc, ScRange(0,0,0,MAXCOL,MAXROW,0));

    // Set values 1,2,3,4,5 to A1:A5.
    for (SCROW i = 0; i <= 4; ++i)
        m_pDoc->SetValue(ScAddress(0,i,0), (i+1));

    // Set formulas to B1:B5.
    for (SCROW i = 0; i <= 4; ++i)
        m_pDoc->SetString(ScAddress(1,i,0), "=RC[-1]");

    // Check the initial formula results.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    // Move A1:A2 to D2:D3.
    rFunc.MoveBlock(ScRange(0,0,0,0,1,0), ScAddress(3,1,0), true, true, false, true);

    // Check the formula values again.  They should not change.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    pUndoMgr->Redo();
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(1,4,0)));

    m_pDoc->DeleteTab(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
