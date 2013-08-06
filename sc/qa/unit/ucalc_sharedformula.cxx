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
    ScCellValue aCell("Test");
    CPPUNIT_ASSERT_MESSAGE("This should be a string value.", aCell.meType == CELLTYPE_STRING);
    aCell.commit(*m_pDoc, aPos);
    CPPUNIT_ASSERT_EQUAL(*aCell.mpString, m_pDoc->GetString(aPos));
    aPos.SetRow(16);
    pFC = m_pDoc->GetFormulaCell(aPos);
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
    OUString aRS;
    aDestRange.Format(aRS, SCR_ABS);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
