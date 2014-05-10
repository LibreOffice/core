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
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cells.", nResRow == -1);
    nResRow = m_pDoc->GetFirstEditTextRow(ScRange(0,0,0,0,0,0));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cells.", nResRow == -1);
    nResRow = m_pDoc->GetFirstEditTextRow(ScRange(0,0,0,0,10,0));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cells.", nResRow == -1);

    ScFieldEditEngine& rEE = m_pDoc->GetEditEngine();
    rEE.SetText("Test");
    m_pDoc->SetEditText(ScAddress(0,0,0), rEE.CreateTextObject());
    const EditTextObject* pObj = m_pDoc->GetEditText(ScAddress(0,0,0));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell here.", pObj);

    ScRange aRange(0,0,0,0,0,0);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_MESSAGE("There is an edit cell here.", nResRow == 0);

    aRange.aStart.SetRow(1);
    aRange.aEnd.SetRow(1);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be an edit cell in specified range.", nResRow == -1);

    aRange.aStart.SetRow(2);
    aRange.aEnd.SetRow(4);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be an edit cell in specified range.", nResRow == -1);

    aRange.aStart.SetRow(0);
    aRange.aEnd.SetRow(MAXROW);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in specified range.", nResRow == 0);

    m_pDoc->SetString(ScAddress(0,0,0), "Test");
    m_pDoc->SetValue(ScAddress(0,2,0), 1.0);
    ScRefCellValue aCell;
    aCell.assign(*m_pDoc, ScAddress(0,0,0));
    CPPUNIT_ASSERT_MESSAGE("This should be a string cell.", aCell.meType == CELLTYPE_STRING);
    aCell.assign(*m_pDoc, ScAddress(0,1,0));
    CPPUNIT_ASSERT_MESSAGE("This should be an empty cell.", aCell.meType == CELLTYPE_NONE);
    aCell.assign(*m_pDoc, ScAddress(0,2,0));
    CPPUNIT_ASSERT_MESSAGE("This should be a numeric cell.", aCell.meType == CELLTYPE_VALUE);
    aCell.assign(*m_pDoc, ScAddress(0,3,0));
    CPPUNIT_ASSERT_MESSAGE("This should be an empty cell.", aCell.meType == CELLTYPE_NONE);

    aRange.aStart.SetRow(1);
    aRange.aEnd.SetRow(1);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be an edit cell in specified range.", nResRow == -1);

    // Test with non-edit cell but with ambiguous script type.

    m_pDoc->SetString(ScAddress(1,11,0), "Some text");
    m_pDoc->SetString(ScAddress(1,12,0), "Some text");
    m_pDoc->SetString(ScAddress(1,13,0), "Other text");

    m_pDoc->SetScriptType(ScAddress(1,11,0), (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN));
    m_pDoc->SetScriptType(ScAddress(1,12,0), (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN));
    m_pDoc->SetScriptType(ScAddress(1,13,0), (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN));

    nResRow = m_pDoc->GetFirstEditTextRow(ScAddress(1,11,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(11), nResRow);
    nResRow = m_pDoc->GetFirstEditTextRow(ScAddress(1,12,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), nResRow);

    for (SCROW i = 0; i <= 5; ++i)
        m_pDoc->SetString(ScAddress(2,i,0), "Text");

    m_pDoc->SetScriptType(ScAddress(2,5,0), (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN));

    nResRow = m_pDoc->GetFirstEditTextRow(ScAddress(2,1,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(-1), nResRow);

    m_pDoc->DeleteTab(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
