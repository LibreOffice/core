/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ucalc.hxx>
#include <editutil.hxx>
#include <cellvalue.hxx>

void Test::testColumnFindEditCells()
{
    m_pDoc->InsertTab(0, "Test");

    bool bRes = m_pDoc->HasEditText(ScRange(0,0,0,0,MAXROW,0));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cells.", !bRes);
    bRes = m_pDoc->HasEditText(ScRange(0,0,0,0,0,0));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cells.", !bRes);
    bRes = m_pDoc->HasEditText(ScRange(0,0,0,0,10,0));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cells.", !bRes);

    ScFieldEditEngine& rEE = m_pDoc->GetEditEngine();
    rEE.SetText("Test");
    m_pDoc->SetEditText(ScAddress(0,0,0), rEE.CreateTextObject());
    const EditTextObject* pObj = m_pDoc->GetEditText(ScAddress(0,0,0));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell here.", pObj);

    ScRange aRange(0,0,0,0,0,0);
    bRes = m_pDoc->HasEditText(aRange);
    CPPUNIT_ASSERT_MESSAGE("There is an edit cell here.", bRes);

    aRange.aStart.SetRow(1);
    aRange.aEnd.SetRow(1);
    bRes = m_pDoc->HasEditText(aRange);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be an edit cell in specified range.", !bRes);

    aRange.aStart.SetRow(2);
    aRange.aEnd.SetRow(4);
    bRes = m_pDoc->HasEditText(aRange);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be an edit cell in specified range.", !bRes);

    aRange.aStart.SetRow(0);
    aRange.aEnd.SetRow(MAXROW);
    bRes = m_pDoc->HasEditText(aRange);
    CPPUNIT_ASSERT_MESSAGE("There shouldn be an edit cell in specified range.", bRes);

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
    bRes = m_pDoc->HasEditText(aRange);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be an edit cell in specified range.", !bRes);

    m_pDoc->DeleteTab(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
