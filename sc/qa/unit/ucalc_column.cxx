/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ucalc.hxx"
#include <editutil.hxx>
#include <cellvalue.hxx>
#include <svl/languageoptions.hxx>

void Test::testColumnFindEditCells()
{
    m_pDoc->InsertTab(0, "Test");

    // Test the basics with real edit cells, using Column A.

    SCROW nResRow = m_pDoc->GetFirstEditTextRow(ScRange(0,0,0,0,MAXROW,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no edit cells.", SCROW(-1), nResRow);
    nResRow = m_pDoc->GetFirstEditTextRow(ScRange(0,0,0,0,0,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no edit cells.", SCROW(-1), nResRow);
    nResRow = m_pDoc->GetFirstEditTextRow(ScRange(0,0,0,0,10,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no edit cells.", SCROW(-1), nResRow);

    ScFieldEditEngine& rEE = m_pDoc->GetEditEngine();
    rEE.SetText("Test");
    m_pDoc->SetEditText(ScAddress(0,0,0), rEE.CreateTextObject());
    const EditTextObject* pObj = m_pDoc->GetEditText(ScAddress(0,0,0));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell here.", pObj);

    ScRange aRange(0,0,0,0,0,0);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There is an edit cell here.", SCROW(0), nResRow);

    aRange.aStart.SetRow(1);
    aRange.aEnd.SetRow(1);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be an edit cell in specified range.", SCROW(-1), nResRow);

    aRange.aStart.SetRow(2);
    aRange.aEnd.SetRow(4);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be an edit cell in specified range.", SCROW(-1), nResRow);

    aRange.aStart.SetRow(0);
    aRange.aEnd.SetRow(MAXROW);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be an edit cell in specified range.", SCROW(0), nResRow);

    m_pDoc->SetString(ScAddress(0,0,0), "Test");
    m_pDoc->SetValue(ScAddress(0,2,0), 1.0);
    ScRefCellValue aCell;
    aCell.assign(*m_pDoc, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be a string cell.", CELLTYPE_STRING, aCell.meType);
    aCell.assign(*m_pDoc, ScAddress(0,1,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be an empty cell.", CELLTYPE_NONE, aCell.meType);
    aCell.assign(*m_pDoc, ScAddress(0,2,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be a numeric cell.", CELLTYPE_VALUE, aCell.meType);
    aCell.assign(*m_pDoc, ScAddress(0,3,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be an empty cell.", CELLTYPE_NONE, aCell.meType);

    aRange.aStart.SetRow(1);
    aRange.aEnd.SetRow(1);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be an edit cell in specified range.", SCROW(-1), nResRow);

    // Test with non-edit cell but with ambiguous script type.

    m_pDoc->SetString(ScAddress(1,11,0), "Some text");
    m_pDoc->SetString(ScAddress(1,12,0), "Some text");
    m_pDoc->SetString(ScAddress(1,13,0), "Other text");

    m_pDoc->SetScriptType(ScAddress(1,11,0), (SvtScriptType::LATIN | SvtScriptType::ASIAN));
    m_pDoc->SetScriptType(ScAddress(1,12,0), (SvtScriptType::LATIN | SvtScriptType::ASIAN));
    m_pDoc->SetScriptType(ScAddress(1,13,0), (SvtScriptType::LATIN | SvtScriptType::ASIAN));

    nResRow = m_pDoc->GetFirstEditTextRow(ScAddress(1,11,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(11), nResRow);
    nResRow = m_pDoc->GetFirstEditTextRow(ScAddress(1,12,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), nResRow);

    for (SCROW i = 0; i <= 5; ++i)
        m_pDoc->SetString(ScAddress(2,i,0), "Text");

    m_pDoc->SetScriptType(ScAddress(2,5,0), (SvtScriptType::LATIN | SvtScriptType::ASIAN));

    nResRow = m_pDoc->GetFirstEditTextRow(ScAddress(2,1,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(-1), nResRow);

    m_pDoc->DeleteTab(0);
}


void Test::testSetFormula()
{
    m_pDoc->InsertTab(0, "Test");

    struct aInputs
    {
        const char* aName;
        SCROW nRow;
        SCCOL nCol;
        const char* aFormula1;      // Represents the formula that is input to SetFormula function.
        const char* aFormula2;      // Represents the formula that is actually stored in the cell.
        formula::FormulaGrammar::Grammar eGram;

    } aTest[] = {
        { "Rock and Roll" ,5 , 4 , "=SUM($D$2:$F$3)"             ,"=SUM($D$2:$F$3)" , formula::FormulaGrammar::Grammar::GRAM_ENGLISH     },
        { "Blues"         ,5 , 5 , "=A1-$C2+B$3-$F$4"            ,"=A1-$C2+B$3-$F$4", formula::FormulaGrammar::Grammar::GRAM_NATIVE      },
        { "Acoustic"      ,6 , 6 , "=A1-$C2+B$3-$F$4"            ,"=A1-$C2+B$3-$F$4", formula::FormulaGrammar::Grammar::GRAM_NATIVE_XL_A1},
        { "Nursery Rhymes",7 , 8 , "=[.A1]-[.$C2]+[.G$3]-[.$F$4]","=A1-$C2+G$3-$F$4", formula::FormulaGrammar::Grammar::GRAM_ODFF        }
    };

    for(size_t i = 0; i < SAL_N_ELEMENTS(aTest); ++i)
    {
        OUString aBuffer;
        m_pDoc->SetFormula(ScAddress(aTest[i].nCol, aTest[i].nRow, 0), OUString::createFromAscii(aTest[i].aFormula1), aTest[i].eGram);
        m_pDoc->GetFormula(aTest[i].nCol, aTest[i].nRow, 0, aBuffer);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to set formula", OUString::createFromAscii(aTest[i].aFormula2), aBuffer);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testMultipleDataCellsInRange()
{
    m_pDoc->InsertTab(0, "Test");

    ScRange aRange(1,2,0); // B3
    sc::MultiDataCellState aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::Empty, aState.meState);

    // Set a numeric value to B3.
    m_pDoc->SetValue(ScAddress(1,2,0), 1.0);
    aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::HasOneCell, aState.meState);
    CPPUNIT_ASSERT_EQUAL(SCCOL(1), aState.mnCol1);
    CPPUNIT_ASSERT_EQUAL(SCROW(2), aState.mnRow1);

    // Set another numeric value to B4.
    m_pDoc->SetValue(ScAddress(1,3,0), 2.0);
    aRange.aEnd.SetRow(3); // B3:B4
    aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::HasMultipleCells, aState.meState);
    CPPUNIT_ASSERT_EQUAL(SCCOL(1), aState.mnCol1);
    CPPUNIT_ASSERT_EQUAL(SCROW(2), aState.mnRow1);

    // Set the query range to B4:B5.  Now it should only report one cell, with
    // B4 being the first non-empty cell.
    aRange.aStart.SetRow(3);
    aRange.aEnd.SetRow(4);
    aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::HasOneCell, aState.meState);
    CPPUNIT_ASSERT_EQUAL(SCCOL(1), aState.mnCol1);
    CPPUNIT_ASSERT_EQUAL(SCROW(3), aState.mnRow1);

    // Set the query range to A1:C3.  The first non-empty cell should be B3.
    aRange = ScRange(0,0,0,2,2,0);
    aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::HasOneCell, aState.meState);
    CPPUNIT_ASSERT_EQUAL(SCCOL(1), aState.mnCol1);
    CPPUNIT_ASSERT_EQUAL(SCROW(2), aState.mnRow1);

    // Set string cells to D4 and F5, and query D3:F5.  D4 should be the first
    // non-empty cell.
    m_pDoc->SetString(ScAddress(3,3,0), "foo");
    m_pDoc->SetString(ScAddress(5,4,0), "bar");
    aRange = ScRange(3,2,0,5,4,0);
    aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::HasMultipleCells, aState.meState);
    CPPUNIT_ASSERT_EQUAL(SCCOL(3), aState.mnCol1);
    CPPUNIT_ASSERT_EQUAL(SCROW(3), aState.mnRow1);

    // TODO : add more test cases as needed.

    m_pDoc->DeleteTab(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
