/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "ucalc.hxx"
#include "editutil.hxx"
#include "formulacell.hxx"
#include "cellvalue.hxx"
#include "docsh.hxx"
#include "clipparam.hxx"
#include "undoblk.hxx"
#include "scopetools.hxx"
#include "svl/sharedstring.hxx"

#include "formula/grammar.hxx"

void Test::testSharedFormulas()
{
    m_pDoc->InsertTab(0, "Test");

    ScAddress aPos(1, 9, 0); 
    m_pDoc->SetString(aPos, "=A10*2"); 
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("Expected to be a non-shared cell.", pFC && !pFC->IsShared());

    aPos.SetRow(10); 
    m_pDoc->SetString(aPos, "=A11*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    aPos.SetRow(8); 
    m_pDoc->SetString(aPos, "=A9*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    aPos.SetRow(12); 
    m_pDoc->SetString(aPos, "=A13*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This formula cell shouldn't be shared yet.", pFC && !pFC->IsShared());

    
    aPos.SetRow(11); 
    m_pDoc->SetString(aPos, "=A12*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(5), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    
    aPos.SetRow(14); 
    m_pDoc->SetString(aPos, "=A15*2");
    aPos.SetRow(15); 
    m_pDoc->SetString(aPos, "=A16*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(14), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    
    aPos.SetRow(13); 
    m_pDoc->SetString(aPos, "=A14*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    
    aPos.SetRow(11); 
    m_pDoc->SetString(aPos, "=$A$1*4");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell shouldn't be shared.", pFC && !pFC->IsShared());

    aPos.SetRow(8); 
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    aPos.SetRow(12); 
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    
    aPos.SetRow(16); 
    m_pDoc->SetString(aPos, "=A17*2");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A18*2");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A19*2");
    aPos.IncRow();
    m_pDoc->SetString(aPos, "=A20*2");
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell is expected to be a shared formula cell.", pFC && pFC->IsShared());
    
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    
    aPos.SetRow(18);
    m_pDoc->SetEmptyCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("This cell should have been emptied.", m_pDoc->GetCellType(aPos) == CELLTYPE_NONE);
    aPos.SetRow(12); 
    pFC = m_pDoc->GetFormulaCell(aPos);
    
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());
    
    aPos.SetRow(19); 
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B20 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This cell should be non-shared.", !pFC->IsShared());

    
    aPos.SetRow(13); 
    m_pDoc->SetEmptyCell(aPos);
    aPos.SetRow(12); 
    pFC = m_pDoc->GetFormulaCell(aPos);
    
    CPPUNIT_ASSERT_MESSAGE("B13 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This cell should be non-shared.", !pFC->IsShared());
    
    aPos.SetRow(14); 
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(14), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    
    aPos.SetRow(14);
    m_pDoc->SetValue(aPos, 1.2);
    aPos.SetRow(15);
    pFC = m_pDoc->GetFormulaCell(aPos);
    
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(15), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    
    aPos.SetRow(15);
    ScCellValue aCell(svl::SharedString("Test"));
    CPPUNIT_ASSERT_MESSAGE("This should be a string value.", aCell.meType == CELLTYPE_STRING);
    aCell.commit(*m_pDoc, aPos);
    CPPUNIT_ASSERT_EQUAL(aCell.mpString->getString(), m_pDoc->GetString(aPos));
    aPos.SetRow(16);
    pFC = m_pDoc->GetFormulaCell(aPos);
    
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(16), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    
    ScFieldEditEngine& rEditEngine = m_pDoc->GetEditEngine();
    rEditEngine.SetText("Edit Text");
    aPos.SetRow(16);
    m_pDoc->SetEditText(aPos, rEditEngine.CreateTextObject());
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, m_pDoc->GetCellType(aPos));
    aPos.SetRow(17);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B18 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("B18 should be non-shared.", !pFC->IsShared());

    
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

    
    
    m_pDoc->DeleteRow(ScRange(0,3,0,MAXCOL,7,0));
    aPos.SetRow(1);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B2 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    
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

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, false); 

    
    m_pDoc->SetValue(ScAddress(0,9,0), 1);
    m_pDoc->SetValue(ScAddress(0,10,0), 2);
    m_pDoc->SetValue(ScAddress(0,11,0), 3);

    
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

    
    m_pDoc->InsertCol(ScRange(0,11,0,1,11,0));
    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "A10"))
        CPPUNIT_FAIL("Wrong formula in B1");
    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "C11"))
        CPPUNIT_FAIL("Wrong formula in B2");
    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "C12"))
        CPPUNIT_FAIL("Wrong formula in B3");

    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("B1 should be a non-shared formula cell.", pFC && !pFC->IsShared());
    
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,1,0));
    CPPUNIT_ASSERT_MESSAGE("This must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    
    m_pDoc->InsertCol(ScRange(0,9,0,1,9,0));
    if (!checkFormula(*m_pDoc, ScAddress(1,0,0), "C10"))
        CPPUNIT_FAIL("Wrong formula in B1");
    if (!checkFormula(*m_pDoc, ScAddress(1,1,0), "C11"))
        CPPUNIT_FAIL("Wrong formula in B2");
    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "C12"))
        CPPUNIT_FAIL("Wrong formula in B3");

    
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("This must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());

    m_pDoc->DeleteTab(0);
}

void Test::testSharedFormulasRefUpdateRange()
{
    m_pDoc->InsertTab(0, "Test");

    
    m_pDoc->SetValue(ScAddress(0,2,0), 1);
    m_pDoc->SetValue(ScAddress(0,3,0), 2);
    m_pDoc->SetValue(ScAddress(0,4,0), 3);

    
    m_pDoc->SetString(ScAddress(1,2,0), "=SUM($A$3:$A$5)");
    m_pDoc->SetString(ScAddress(1,3,0), "=SUM($A$3:$A$5)");
    m_pDoc->SetString(ScAddress(1,4,0), "=SUM($A$3:$A$5)");

    if (!checkFormula(*m_pDoc, ScAddress(1,2,0), "SUM($A$3:$A$5)"))
        CPPUNIT_FAIL("Wrong formula");
    if (!checkFormula(*m_pDoc, ScAddress(1,3,0), "SUM($A$3:$A$5)"))
        CPPUNIT_FAIL("Wrong formula");
    if (!checkFormula(*m_pDoc, ScAddress(1,4,0), "SUM($A$3:$A$5)"))
        CPPUNIT_FAIL("Wrong formula");

    
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,2,0));
    CPPUNIT_ASSERT_MESSAGE("B3 should be shared.", pFC && pFC->IsShared());
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,3,0));
    CPPUNIT_ASSERT_MESSAGE("B4 should be shared.", pFC && pFC->IsShared());
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,4,0));
    CPPUNIT_ASSERT_MESSAGE("B3 should be shared.", pFC && pFC->IsShared());

    
    m_pDoc->InsertRow(ScRange(0,0,0,MAXCOL,1,0));

    
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

void Test::testSharedFormulasDeleteRows()
{
    m_pDoc->InsertTab(0, "Test");
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    
    for (SCROW i = 0; i <= 9 ; ++i)
    {
        m_pDoc->SetValue(0, i, 0, i);
        m_pDoc->SetString(1, i, 0, "=RC[-1]+1");
    }
    
    for (SCROW i = 10; i <= 19 ; ++i)
    {
        m_pDoc->SetValue(0, i, 0, i);
        m_pDoc->SetString(1, i, 0, "=RC[-1]+11");
    }

    
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("1,0 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());
    
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,10,0));
    CPPUNIT_ASSERT_MESSAGE("1,10 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

    
    m_pDoc->DeleteRow(ScRange(0,8,0,MAXCOL,11,0));

    
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("1,0 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());
    
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,8,0));
    CPPUNIT_ASSERT_MESSAGE("1,8 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());

    
    m_pDoc->DeleteRow(ScRange(0,2,0,MAXCOL,2,0));

    
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("1,0 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(7), pFC->GetSharedLength());
    
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,7,0));
    CPPUNIT_ASSERT_MESSAGE("1,7 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(7), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());

    
    m_pDoc->DeleteRow(ScRange(0,4,0,MAXCOL,4,0));

    
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("1,0 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedLength());
    
    pFC = m_pDoc->GetFormulaCell(ScAddress(1,6,0));
    CPPUNIT_ASSERT_MESSAGE("1,6 must be a shared formula cell.", pFC && pFC->IsShared());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedLength());
}

void Test::testSharedFormulasRefUpdateMoveSheets()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");
    m_pDoc->InsertTab(2, "Sheet3");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); 

    
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    
    for (SCROW i = 0; i <= 7; ++i)
        m_pDoc->SetValue(ScAddress(0,i,1), i+1);

    
    for (SCROW i = 0; i <= 7; ++i)
        m_pDoc->SetString(ScAddress(0,i,0), "=Sheet2!RC");

    
    for (SCROW i = 0; i <= 7; ++i)
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(i+1), m_pDoc->GetValue(ScAddress(0,i,0)));

    
    m_pDoc->MoveTab(2, 0);

    
    std::vector<OUString> aTabNames = m_pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_MESSAGE("There should be at least 3 sheets.", aTabNames.size() >= 3);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet3"), aTabNames[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aTabNames[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aTabNames[2]);

    
    for (SCROW i = 0; i <= 7; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(i+1), m_pDoc->GetValue(ScAddress(0,i,1)));
        if (!checkFormula(*m_pDoc, ScAddress(0,i,1), "Sheet2!RC"))
            CPPUNIT_FAIL("Wrong formula expression.");
    }

    
    m_pDoc->InsertTab(0, "Sheet4");

    
    aTabNames = m_pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_MESSAGE("There should be at least 4 sheets.", aTabNames.size() >= 4);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet4"), aTabNames[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet3"), aTabNames[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aTabNames[2]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aTabNames[3]);

    
    for (SCROW i = 0; i <= 7; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(i+1), m_pDoc->GetValue(ScAddress(0,i,2)));
        if (!checkFormula(*m_pDoc, ScAddress(0,i,2), "Sheet2!RC"))
            CPPUNIT_FAIL("Wrong formula expression.");
    }

    
    m_pDoc->DeleteTab(0);

    
    aTabNames = m_pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_MESSAGE("There should be at least 3 sheets.", aTabNames.size() >= 3);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet3"), aTabNames[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), aTabNames[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet2"), aTabNames[2]);

    
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
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); 

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetValue(ScAddress(0,0,1), 1.0); 
    m_pDoc->SetValue(ScAddress(0,1,1), 2.0); 

    
    m_pDoc->SetString(ScAddress(0,0,0), "=$Sheet2.A1");
    m_pDoc->SetString(ScAddress(0,1,0), "=$Sheet2.A2");

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    
    m_pDoc->CopyTab(0, 0);

    if (!checkFormula(*m_pDoc, ScAddress(0,0,0), "$Sheet2.A1"))
        CPPUNIT_FAIL("Wrong formula");

    if (!checkFormula(*m_pDoc, ScAddress(0,1,0), "$Sheet2.A2"))
        CPPUNIT_FAIL("Wrong formula");

    
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    
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

    
    for (SCROW i = 0; i <= 9; ++i)
        m_pDoc->SetString(1, i, 0, "=RC[-1]");

    ScAddress aPos(1, 8, 0); 
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B9 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    
    ScRange aSrcRange(1,5,0,1,8,0); 
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aSrcRange, &aClipDoc);
    pFC = aClipDoc.GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("B9 in the clip doc should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(5), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    
    ScRange aDestRange(2,1,0,2,9,0);
    pasteFromClip(m_pDoc, aDestRange, &aClipDoc);
    aPos.SetCol(2);
    aPos.SetRow(1);
    pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT_MESSAGE("C2 should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), pFC->GetSharedLength());
    CPPUNIT_ASSERT_MESSAGE("The token is expected to be shared.", pFC->GetCode() == pFC->GetSharedCode());

    ScRange aRange(1,0,0,1,9,0); 
    ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
    pUndoDoc->InitUndo(m_pDoc, 0, 0, true, true);
    m_pDoc->CopyToDocument(aRange, IDF_CONTENTS, false, pUndoDoc);
    boost::scoped_ptr<ScUndoPaste> pUndo(createUndoPaste(getDocShell(), aRange, pUndoDoc));

    
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

    
    for (SCROW i = 0; i <= 9; ++i)
        m_pDoc->SetValue(ScAddress(1,i,0), i*10);

    for (SCROW i = 0; i <= 9; ++i)
        CPPUNIT_ASSERT_MESSAGE("Numeric cell was expected.", m_pDoc->GetCellType(ScAddress(1,i,0)) == CELLTYPE_VALUE);

    
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

    
    m_pDoc->SetString(ScAddress(7,1,0), "=G3*B3");
    m_pDoc->SetString(ScAddress(7,2,0), "=G4*B4");

    
    m_pDoc->InsertCol(ScRange(5,0,0,5,MAXROW,0));

    if (!checkFormula(*m_pDoc, ScAddress(8,1,0), "H3*B3"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(*m_pDoc, ScAddress(8,2,0), "H4*B4"))
        CPPUNIT_FAIL("Wrong formula!");

    m_pDoc->DeleteTab(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
