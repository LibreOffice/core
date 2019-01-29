/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include "ucalc.hxx"
#include "helper/qahelper.hxx"

#include <conditio.hxx>
#include <colorscale.hxx>

#include <clipparam.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <docfunc.hxx>
#include <scitems.hxx>
#include <attrib.hxx>
#include <fillinfo.hxx>
#include <compiler.hxx>
#include <tokenarray.hxx>

#include <svl/sharedstringpool.hxx>

void Test::testCopyPasteSkipEmptyConditionalFormatting()
{
    m_pDoc->InsertTab(0, "Test");

    ScRange aDestRange(0,0,0,1,2,0);
    ScRange aSrcRange(3,3,0,5,4,0);

    ScMarkData aMark;
    aMark.SetMarkArea(aDestRange);

    m_pDoc->SetValue(0,0,0,1);
    m_pDoc->SetValue(1,0,0,1);
    m_pDoc->SetValue(0,1,0,1);
    m_pDoc->SetValue(0,2,0,1);
    m_pDoc->SetValue(1,2,0,1);

    //create conditional formatting for A1:B3
    ScConditionalFormatList* pCondFormatList = new ScConditionalFormatList();
    m_pDoc->SetCondFormList(pCondFormatList, 0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    pFormat->SetRange(aDestRange);
    sal_uLong nCondFormatKey = m_pDoc->AddCondFormat(std::move(pFormat), 0);

    // Prepare a clipboard content interleaved with empty cells.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    aClipDoc.ResetClip(m_pDoc, &aMark);
    ScClipParam aParam(aSrcRange, false);
    aClipDoc.SetClipParam(aParam);
    aClipDoc.SetValue(3,3,0,2);
    aClipDoc.SetValue(4,3,0,2);
    aClipDoc.SetValue(4,4,0,2);
    aClipDoc.SetValue(3,5,0,2);
    aClipDoc.SetValue(4,5,0,2);

    auto pClipFormat = std::make_unique<ScConditionalFormat>(2, &aClipDoc);
    pClipFormat->SetRange(aSrcRange);
    aClipDoc.AddCondFormat(std::move(pClipFormat), 0);

    // Create undo document.
    ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
    pUndoDoc->InitUndo(m_pDoc, 0, 0);
    m_pDoc->CopyToDocument(aDestRange, InsertDeleteFlags::CONTENTS, false, *pUndoDoc, &aMark);

    // Paste clipboard content onto A1:A5 but skip empty cells.
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::CONTENTS, pUndoDoc, &aClipDoc, true, false, false, true/*bSkipEmpty*/);

    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pList->size());
    CPPUNIT_ASSERT(m_pDoc->GetCondFormat(1,1,0));
    // empty cell in copy area does not overwrite conditional formatting
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nCondFormatKey), m_pDoc->GetCondFormat(1,1,0)->GetKey());
    for(SCCOL nCol = 0; nCol <= 1; ++nCol)
    {
        for(SCROW nRow = 0; nRow <= 2; ++nRow)
        {
            if(nRow == 1 && nCol == 1)
                continue;

            CPPUNIT_ASSERT(m_pDoc->GetCondFormat(nCol, nRow, 0));
            CPPUNIT_ASSERT(nCondFormatKey != m_pDoc->GetCondFormat(nCol, nRow, 0)->GetKey());
        }
    }
    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatINSDEL()
{
    // fdo#62206
    m_pDoc->InsertTab(0, "Test");
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,0,3,0));
    pFormat->SetRange(aRangeList);
    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    auto pFormatTmp = pFormat.get();
    pList->InsertNew(std::move(pFormat));

    m_pDoc->InsertCol(0,0,MAXROW,0,0,2);
    const ScRangeList& rRange = pFormatTmp->GetRange();
    CPPUNIT_ASSERT_EQUAL(static_cast<const ScRangeList&>(ScRange(2,0,0,2,3,0)), rRange);

    OUString aExpr = pEntry->GetExpression(ScAddress(2,0,0), 0);
    CPPUNIT_ASSERT_EQUAL(OUString("D2"), aExpr);

    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatInsertCol()
{
    m_pDoc->InsertTab(0, "Test");
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,3,3,0));
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    auto pFormatTmp = pFormat.get();
    pList->InsertNew(std::move(pFormat));

    m_pDoc->InsertCol(0,0,MAXROW,0,4,2);
    const ScRangeList& rRange = pFormatTmp->GetRange();
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(0,0,0,5,3,0)), rRange);

    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatInsertRow()
{
    m_pDoc->InsertTab(0, "Test");
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,3,3,0));
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    auto pFormatTmp = pFormat.get();
    pList->InsertNew(std::move(pFormat));

    m_pDoc->InsertRow(0,0,MAXCOL,0,4,2);
    const ScRangeList& rRange = pFormatTmp->GetRange();
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(0,0,0,3,5,0)), rRange);

    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatInsertDeleteSheets()
{
    m_pDoc->InsertTab(0, "Test");

    // Add a conditional format to B2:B4.
    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    pFormat->SetRange(ScRange(1,1,0,1,3,0));

    auto pFormatTmp = pFormat.get();
    sal_uLong nKey = m_pDoc->AddCondFormat(std::move(pFormat), 0);

    // Add condition in which if the value equals 2, set the "Result" style.
    ScCondFormatEntry* pEntry = new ScCondFormatEntry(
        ScConditionMode::Equal, "=2", "" , m_pDoc, ScAddress(0,0,0), ScResId(STR_STYLENAME_RESULT));
    pFormatTmp->AddEntry(pEntry);

    // Apply the format to the range.
    m_pDoc->AddCondFormatData(pFormatTmp->GetRange(), 0, nKey);

    // Make sure this conditional format entry is really there.
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT(pList);
    const ScConditionalFormat* pCheck = pList->GetFormat(nKey);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong conditional format instance.", pCheck, const_cast<const ScConditionalFormat*>(pFormatTmp));

    // ... and its range is B2:B4.
    ScRangeList aCheckRange = pCheck->GetRange();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be a single range.", size_t(1), aCheckRange.size());
    const ScRange* pRange = &aCheckRange[0];
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Format should be applied to B2:B4.", ScRange(1,1,0,1,3,0), *pRange);

    ScDocFunc& rFunc = getDocShell().GetDocFunc();

    // Insert a new sheet at the left.
    bool bInserted = rFunc.InsertTable(0, "Inserted", true, true);
    CPPUNIT_ASSERT(bInserted);

    pList = m_pDoc->GetCondFormList(1);
    CPPUNIT_ASSERT(pList);
    pCheck = pList->GetFormat(nKey);
    CPPUNIT_ASSERT(pCheck);

    // Make sure the range also got shifted.
    aCheckRange = pCheck->GetRange();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be a single range.", size_t(1), aCheckRange.size());
    pRange = &aCheckRange[0];
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Format should be applied to B2:B4 on the 2nd sheet after the sheet insertion.", ScRange(1,1,1,1,3,1), *pRange);

    // Delete the sheet to the left.
    bool bDeleted = rFunc.DeleteTable(0, true);
    CPPUNIT_ASSERT(bDeleted);

    pList = m_pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT(pList);
    pCheck = pList->GetFormat(nKey);
    CPPUNIT_ASSERT(pCheck);

    // Make sure the range got shifted back.
    aCheckRange = pCheck->GetRange();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be a single range.", size_t(1), aCheckRange.size());
    pRange = &aCheckRange[0];
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Format should be applied to B2:B4 on the 1st sheet after the sheet removal.", ScRange(1,1,0,1,3,0), *pRange);

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);

    // Undo and re-check.
    pUndoMgr->Undo();

    pList = m_pDoc->GetCondFormList(1);
    CPPUNIT_ASSERT(pList);
    pCheck = pList->GetFormat(nKey);
    CPPUNIT_ASSERT(pCheck);

    aCheckRange = pCheck->GetRange();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be a single range.", size_t(1), aCheckRange.size());
    pRange = &aCheckRange[0];
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Format should be applied to B2:B4 on the 2nd sheet after the undo of the sheet removal.", ScRange(1,1,1,1,3,1), *pRange);

#if 0 // TODO : Undo of sheet insertion currently depends on the presence of
      // view shell, and crashes when executed during cppunit run.

    // Undo again and re-check.
    pUndoMgr->Undo();

    pList = m_pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT(pList);
    pCheck = pList->GetFormat(nKey);
    CPPUNIT_ASSERT(pCheck);

    // Make sure the range got shifted back.
    aCheckRange = pCheck->GetRange();
    CPPUNIT_ASSERT_MESSAGE("This should be a single range.", aCheckRange.size() == 1);
    pRange = aCheckRange[0];
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_MESSAGE("Format should be applied to B2:B4 on the 1st sheet after the undo of sheet insertion.", *pRange == ScRange(1,1,0,1,3,0));
#else
    m_pDoc->DeleteTab(1);
#endif

    m_pDoc->DeleteTab(0);
}

void Test::testCondCopyPaste()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    sal_uLong nIndex = m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aCondFormatRange, &aClipDoc);

    ScRange aTargetRange(4,4,0,7,7,0);
    pasteFromClip(m_pDoc, aTargetRange, &aClipDoc);

    ScConditionalFormat* pPastedFormat = m_pDoc->GetCondFormat(7,7,0);
    CPPUNIT_ASSERT(pPastedFormat);

    // Pasting the same conditional format must modify existing format, making its range
    // combined of previous range and newly pasted range having the conditional format.
    // No new conditional formats must be created.
    CPPUNIT_ASSERT_EQUAL(size_t(1), m_pDoc->GetCondFormList(0)->size());
    aRangeList.Join(aTargetRange);
    CPPUNIT_ASSERT_EQUAL(aRangeList, pPastedFormat->GetRange());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pPastedFormat->GetKey());
    const SfxPoolItem* pItem = m_pDoc->GetAttr( 7, 7, 0, ATTR_CONDITIONAL );
    const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);

    CPPUNIT_ASSERT(pCondFormatItem);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormatItem->GetCondFormatData().size());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pCondFormatItem->GetCondFormatData().at(0));

    m_pDoc->DeleteTab(0);
}

void Test::testCondCopyPasteSingleCell()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    sal_uLong nIndex = m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(0,0,0,0,0,0), &aClipDoc);

    ScRange aTargetRange(4,4,0,4,4,0);
    pasteOneCellFromClip(m_pDoc, aTargetRange, &aClipDoc);

    ScConditionalFormat* pPastedFormat = m_pDoc->GetCondFormat(4,4,0);
    CPPUNIT_ASSERT(pPastedFormat);

    // Pasting the same conditional format must modify existing format, making its range
    // combined of previous range and newly pasted range having the conditional format.
    // No new conditional formats must be created.
    CPPUNIT_ASSERT_EQUAL(size_t(1), m_pDoc->GetCondFormList(0)->size());
    aRangeList.Join(aTargetRange);
    CPPUNIT_ASSERT_EQUAL(aRangeList, pPastedFormat->GetRange());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pPastedFormat->GetKey());
    const SfxPoolItem* pItem = m_pDoc->GetAttr( 4, 4, 0, ATTR_CONDITIONAL );
    const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);

    CPPUNIT_ASSERT(pCondFormatItem);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormatItem->GetCondFormatData().size());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pCondFormatItem->GetCondFormatData().at(0) );

    m_pDoc->DeleteTab(0);
}

void Test::testCondCopyPasteSingleCellToRange()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    sal_uLong nIndex = m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(0,0,0,0,0,0), &aClipDoc);
    ScRange aTargetRange(4,4,0,5,8,0);
    pasteOneCellFromClip(m_pDoc, aTargetRange, &aClipDoc);

    // Pasting the same conditional format must modify existing format, making its range
    // combined of previous range and newly pasted range having the conditional format.
    // No new conditional formats must be created.
    CPPUNIT_ASSERT_EQUAL(size_t(1), m_pDoc->GetCondFormList(0)->size());
    aRangeList.Join(aTargetRange);
    for(SCROW nRow = 4; nRow <= 8; ++nRow)
    {
        for (SCCOL nCol = 4; nCol <= 5; ++nCol)
        {
            ScConditionalFormat* pPastedFormat = m_pDoc->GetCondFormat(nCol, nRow, 0);
            CPPUNIT_ASSERT(pPastedFormat);

            CPPUNIT_ASSERT_EQUAL(aRangeList, pPastedFormat->GetRange());
            sal_uLong nPastedKey = pPastedFormat->GetKey();
            CPPUNIT_ASSERT_EQUAL(nIndex, nPastedKey);
            const SfxPoolItem* pItem = m_pDoc->GetAttr( nCol, nRow, 0, ATTR_CONDITIONAL );
            const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);

            CPPUNIT_ASSERT(pCondFormatItem);
            CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormatItem->GetCondFormatData().size());
            CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pCondFormatItem->GetCondFormatData().at(0) );
        }
    }

    m_pDoc->DeleteTab(0);
}

void Test::testCondCopyPasteSingleCellIntoSameFormatRange()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0, 0, 0, 3, 3, 0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct, "=B2", "", m_pDoc, ScAddress(0, 0, 0), ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    sal_uLong nIndex = m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(1, 1, 0, 1, 1, 0), &aClipDoc);

    ScRange aTargetRange(2, 2, 0, 2, 2, 0);
    pasteFromClip(m_pDoc, aTargetRange, &aClipDoc);

    ScConditionalFormat* pPastedFormat = m_pDoc->GetCondFormat(2, 2, 0);
    CPPUNIT_ASSERT(pPastedFormat);

    // Pasting the same conditional format into the same range must not modify existing format,
    // since it already covers the pasted range. No new conditional formats must be created.
    CPPUNIT_ASSERT_EQUAL(size_t(1), m_pDoc->GetCondFormList(0)->size());
    CPPUNIT_ASSERT_EQUAL(aRangeList, pPastedFormat->GetRange());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pPastedFormat->GetKey());
    const SfxPoolItem* pItem = m_pDoc->GetAttr(2, 2, 0, ATTR_CONDITIONAL);
    const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);

    CPPUNIT_ASSERT(pCondFormatItem);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormatItem->GetCondFormatData().size());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pCondFormatItem->GetCondFormatData().at(0));

    m_pDoc->DeleteTab(0);
}

void Test::testCondCopyPasteSingleRowToRange()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,0,0,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(0,0,0,MAXCOL,0,0), &aClipDoc);
    ScRange aTargetRange(0,4,0,MAXCOL,4,0);
    pasteOneCellFromClip(m_pDoc, aTargetRange, &aClipDoc);

    ScConditionalFormat* pNewFormat = m_pDoc->GetCondFormat(0, 4, 0);
    CPPUNIT_ASSERT(pNewFormat);
    CPPUNIT_ASSERT_EQUAL(pNewFormat->GetKey(), pFormatTmp->GetKey());

    for (SCCOL nCol = 1; nCol <= MAXCOL; ++nCol)
    {
        ScConditionalFormat* pNewFormat2 = m_pDoc->GetCondFormat(nCol, 4, 0);
        CPPUNIT_ASSERT(!pNewFormat2);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testCondCopyPasteSingleRowToRange2()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,0,0,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(0,0,0,3,0,0), &aClipDoc);
    ScRange aTargetRange(0,4,0,MAXCOL,4,0);
    pasteOneCellFromClip(m_pDoc, aTargetRange, &aClipDoc);

    for (SCCOL nCol = 0; nCol <= MAXCOL; ++nCol)
    {
        ScConditionalFormat* pNewFormat = m_pDoc->GetCondFormat(nCol, 4, 0);
        if (nCol % 4 == 0)
            CPPUNIT_ASSERT(pNewFormat);
        else
            CPPUNIT_ASSERT(!pNewFormat);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testCondCopyPasteSheetBetweenDoc()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aDoc;
    aDoc.TransferTab(m_pDoc, 0, 0);

    ScConditionalFormatList* pList = aDoc.GetCondFormList(0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());

    m_pDoc->DeleteTab(0);
}

void Test::testCondCopyPasteSheet()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    m_pDoc->CopyTab(0, SC_TAB_APPEND);

    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(1);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());

    ScConditionalFormat& rFormat = **pList->begin();
    const ScRangeList& rRange = rFormat.GetRange();
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(0,0,1,3,3,1)), rRange);
    sal_uInt32 nKey = rFormat.GetKey();
    const SfxPoolItem* pItem = m_pDoc->GetAttr( 2, 2, 1, ATTR_CONDITIONAL );
    const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);

    CPPUNIT_ASSERT(pCondFormatItem);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormatItem->GetCondFormatData().size());
    CPPUNIT_ASSERT_EQUAL( nKey, pCondFormatItem->GetCondFormatData().at(0) );

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testIconSet()
{
    m_pDoc->InsertTab(0, "Test");
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,0,0,0));
    pFormat->SetRange(aRangeList);

    ScIconSetFormat* pEntry = new ScIconSetFormat(m_pDoc);
    ScIconSetFormatData* pData = new ScIconSetFormatData;
    pData->m_Entries.push_back(std::make_unique<ScColorScaleEntry>(0, COL_BLUE));
    pData->m_Entries.push_back(std::make_unique<ScColorScaleEntry>(1, COL_GREEN));
    pData->m_Entries.push_back(std::make_unique<ScColorScaleEntry>(2, COL_RED));
    pEntry->SetIconSetData(pData);

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    pList->InsertNew(std::move(pFormat));

    static struct {
        double nVal; sal_Int32 nIndex;
    } const aTests[] = {
        { -1.0, 0 },
        { 0.0, 0 },
        { 1.0, 1 },
        { 2.0, 2 },
        { 3.0, 2 }
    };
    for(size_t i = 0; i < SAL_N_ELEMENTS(aTests); ++i)
    {
        m_pDoc->SetValue(0,0,0,aTests[i].nVal);
        std::unique_ptr<ScIconSetInfo> pInfo = pEntry->GetIconSetInfo(ScAddress(0,0,0));
        CPPUNIT_ASSERT_EQUAL(aTests[i].nIndex, pInfo->nIconIndex);
    }

    delete pEntry;
    m_pDoc->DeleteTab(0);
}

namespace {

struct ScDataBarLengthData
{
    double const nVal;
    double const nLength;
};

void testDataBarLengthImpl(ScDocument* pDoc, const ScDataBarLengthData* pData, const ScRange& rRange,
        double nMinVal, ScColorScaleEntryType eMinType,
        double nMaxVal, ScColorScaleEntryType eMaxType,
        double nZeroPos, databar::ScAxisPosition eAxisPos)
{
    std::unique_ptr<ScConditionalFormat> pFormat(new ScConditionalFormat(1, pDoc));
    ScRangeList aRangeList(rRange);
    pFormat->SetRange(aRangeList);

    SCCOL nCol = rRange.aStart.Col();

    ScDataBarFormat* pDatabar = new ScDataBarFormat(pDoc);
    pFormat->AddEntry(pDatabar);

    ScDataBarFormatData* pFormatData = new ScDataBarFormatData();
    pFormatData->meAxisPosition = eAxisPos;

    pFormatData->mpLowerLimit.reset(new ScColorScaleEntry());
    pFormatData->mpLowerLimit->SetValue(nMinVal);
    pFormatData->mpLowerLimit->SetType(eMinType);
    pFormatData->mpUpperLimit.reset(new ScColorScaleEntry());
    pFormatData->mpUpperLimit->SetValue(nMaxVal);
    pFormatData->mpUpperLimit->SetType(eMaxType);
    pDatabar->SetDataBarData(pFormatData);

    for (size_t i = 0; pData[i].nLength != -200; ++i)
    {
        pDoc->SetValue(nCol, i, 0, pData[i].nVal);
    }

    for (size_t i = 0; pData[i].nLength != -200; ++i)
    {
        std::unique_ptr<ScDataBarInfo> xInfo(pDatabar->GetDataBarInfo(ScAddress(nCol, i, 0)));
        CPPUNIT_ASSERT(xInfo);
        ASSERT_DOUBLES_EQUAL(pData[i].nLength, xInfo->mnLength);
        ASSERT_DOUBLES_EQUAL(nZeroPos, xInfo->mnZero);
    }
}

}

void Test::testDataBarLengthAutomaticAxis()
{
    m_pDoc->InsertTab(0, "Test");

    static const ScDataBarLengthData aValues[] = {
        { 2, 0 },
        { 3, 0 },
        { 4, 25.0 },
        { 5, 50.0 },
        { 6, 75.0 },
        { 7, 100.0 },
        { 8, 100.0 },
        { 9, 100.0 },
        { 0, -200 }
    };

    testDataBarLengthImpl(m_pDoc, aValues, ScRange(0,0,0,0,7,0),
            3, COLORSCALE_VALUE, 7, COLORSCALE_VALUE, 0.0, databar::AUTOMATIC);

    static const ScDataBarLengthData aValues2[] = {
        { -6, -100 },
        { -5, -100 },
        { -4, -100 },
        { -3, -75.0 },
        { -2, -50.0 },
        { -1, -25.0 },
        { 0, 0.0 },
        { 1, 12.5 },
        { 2, 25.0 },
        { 3, 37.5 },
        { 4, 50.0 },
        { 5, 62.5 },
        { 6, 75.0 },
        { 7, 87.5 },
        { 8, 100.0 },
        { 9, 100.0 },
        { 0, -200 }
    };
    testDataBarLengthImpl(m_pDoc, aValues2, ScRange(1,0,0,1,15,0),
            -4, COLORSCALE_VALUE, 8, COLORSCALE_VALUE, 1.0/3.0 * 100, databar::AUTOMATIC);

    static const ScDataBarLengthData aValues3[] = {
        { 2, 0.0 },
        { 3, 25.0 },
        { 4, 50.0 },
        { 6, 100.0 },
        { 0, -200 }
    };
    testDataBarLengthImpl(m_pDoc, aValues3, ScRange(2,0,0,2,3,0),
            0, COLORSCALE_MIN, 0, COLORSCALE_MAX, 0, databar::AUTOMATIC);

    static const ScDataBarLengthData aValues4[] = {
        { 2, 40.0 },
        { 3, 60.0 },
        { 4, 80.0 },
        { 5, 100.0 },
        { 0, -200 }
    };
    testDataBarLengthImpl(m_pDoc, aValues4, ScRange(3,0,0,3,3,0),
            0, COLORSCALE_AUTO, 0, COLORSCALE_AUTO, 0, databar::AUTOMATIC);

    m_pDoc->DeleteTab(0);
}

void Test::testDataBarLengthMiddleAxis()
{
    m_pDoc->InsertTab(0, "Test");

    static const ScDataBarLengthData aValues[] = {
        { 1, 25.0 },
        { 2, 25.0 },
        { 3, 37.5 },
        { 4, 50.0 },
        { 5, 62.5 },
        { 6, 75.0 },
        { 7, 87.5 },
        { 8, 100.0 },
        { 9, 100.0 },
        { 0, -200 }
    };

    testDataBarLengthImpl(m_pDoc, aValues, ScRange(0,0,0,0,8,0),
            2, COLORSCALE_VALUE, 8, COLORSCALE_VALUE, 50.0, databar::MIDDLE);

    static const ScDataBarLengthData aValues2[] = {
        { -6, -50 },
        { -5, -50 },
        { -4, -50 },
        { -3, -37.5 },
        { -2, -25.0 },
        { -1, -12.5 },
        { 0, 0.0 },
        { 1, 12.5 },
        { 2, 25.0 },
        { 3, 37.5 },
        { 4, 50.0 },
        { 5, 62.5 },
        { 6, 75.0 },
        { 7, 87.5 },
        { 8, 100.0 },
        { 9, 100.0 },
        { 0, -200 }
    };
    testDataBarLengthImpl(m_pDoc, aValues2, ScRange(1,0,0,1,15,0),
            -4, COLORSCALE_VALUE, 8, COLORSCALE_VALUE, 50.0, databar::MIDDLE);

    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatEndsWithStr()
{
    m_pDoc->InsertTab(0, "Test");

    ScConditionEntry aEntry(ScConditionMode::EndsWith, "\"TestString\"", "", m_pDoc, ScAddress(),
            "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    svl::SharedStringPool& rStringPool = m_pDoc->GetSharedStringPool();
    svl::SharedString aStr = rStringPool.intern("SimpleTestString");
    ScRefCellValue aVal(&aStr);
    ScAddress aPos(0, 0, 0);

    bool bValid = aEntry.IsCellValid(aVal, aPos);
    CPPUNIT_ASSERT(bValid);

    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatEndsWithVal()
{
    m_pDoc->InsertTab(0, "Test");

    ScConditionEntry aEntry(ScConditionMode::EndsWith, "2", "", m_pDoc, ScAddress(),
            "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    for (sal_Int32 i = 0; i < 15; ++i)
    {
        ScRefCellValue aVal(i);
        ScAddress aPos(0, 0, 0);

        bool bValid = aEntry.IsCellValid(aVal, aPos);
        bool bShouldBeValid = (i % 10) == 2;
        CPPUNIT_ASSERT_EQUAL(bShouldBeValid, bValid);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaListenerSingleCellToSingleCell()
{
    m_pDoc->InsertTab(0, "test");

    ScCompiler aCompiler(m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaListenerSingleCellToMultipleCells()
{
    m_pDoc->InsertTab(0, "test");

    ScCompiler aCompiler(m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaListenerMultipleCellsToSingleCell()
{
    m_pDoc->InsertTab(0, "test");

    ScCompiler aCompiler(m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaListenerMultipleCellsToMultipleCells()
{
    m_pDoc->InsertTab(0, "test");

    ScCompiler aCompiler(m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaListenerUpdateInsertTab()
{
    m_pDoc->InsertTab(0, "test");

    ScCompiler aCompiler(m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);
    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(m_pDoc);
    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));
    CPPUNIT_ASSERT(!aListener.NeedsRepaint());

    m_pDoc->InsertTab(0, "new_tab");

    // check that the listener has moved to the new sheet
    m_pDoc->SetValue(ScAddress(0, 0, 1), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    // check that we are not listening to the old sheet
    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(!aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaListenerUpdateDeleteTab()
{
    m_pDoc->InsertTab(0, "test");
    m_pDoc->InsertTab(0, "to_delete");

    ScCompiler aCompiler(m_pDoc, ScAddress(10, 10, 1), formula::FormulaGrammar::GRAM_ENGLISH);
    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(m_pDoc);
    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 1));
    CPPUNIT_ASSERT(!aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);

    // check that the listener has moved
    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatUpdateMoveTab()
{
    m_pDoc->InsertTab(0, "test");
    m_pDoc->InsertTab(1, "Test2");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "A1", "", m_pDoc, ScAddress(10, 10, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    auto pFormat = std::make_unique<ScConditionalFormat>(0, m_pDoc);
    pFormat->SetRange(ScRange(10, 10, 0, 10, 12, 0));
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    pFormatTmp->AddEntry(pEntry);

    // the conditional format should listen to A1:A3
    for (SCROW nRow = 0; nRow < 3; ++nRow)
    {
        m_pDoc->SetValue(ScAddress(0, nRow, 0), 1.0);
        CPPUNIT_ASSERT(pEntry->NeedsRepaint());
    }

    m_pDoc->MoveTab(0, 1);

    // the conditional format should listen to A1:A3 on the second sheet
    for (SCROW nRow = 0; nRow < 3; ++nRow)
    {
        m_pDoc->SetValue(ScAddress(0, nRow, 1), 1.0);
        CPPUNIT_ASSERT(pEntry->NeedsRepaint());

        m_pDoc->SetValue(ScAddress(0, nRow, 0), 1.0);
        CPPUNIT_ASSERT(!pEntry->NeedsRepaint());
    }

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatUpdateInsertTab()
{
    m_pDoc->InsertTab(0, "test");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "A1", "", m_pDoc, ScAddress(10, 10, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    auto pFormat = std::make_unique<ScConditionalFormat>(0, m_pDoc);
    pFormat->SetRange(ScRange(10, 10, 0, 10, 12, 0));
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    pFormatTmp->AddEntry(pEntry);

    // the conditional format should listen to A1:A3
    for (SCROW nRow = 0; nRow < 3; ++nRow)
    {
        m_pDoc->SetValue(ScAddress(0, nRow, 0), 1.0);
        CPPUNIT_ASSERT(pEntry->NeedsRepaint());

        m_pDoc->SetValue(ScAddress(0, nRow, 1), 1.0);
        CPPUNIT_ASSERT(!pEntry->NeedsRepaint());
    }

    m_pDoc->InsertTab(0, "test2");

    // the conditional format should listen to A1:A3 on the second sheet
    for (SCROW nRow = 0; nRow < 3; ++nRow)
    {
        m_pDoc->SetValue(ScAddress(0, nRow, 1), 1.0);
        CPPUNIT_ASSERT(pEntry->NeedsRepaint());

        m_pDoc->SetValue(ScAddress(0, nRow, 0), 1.0);
        CPPUNIT_ASSERT(!pEntry->NeedsRepaint());
    }

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatUpdateDeleteTab()
{
    m_pDoc->InsertTab(0, "test");
    m_pDoc->InsertTab(1, "Test2");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "A1", "", m_pDoc, ScAddress(10, 10, 1), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    auto pFormat = std::make_unique<ScConditionalFormat>(0, m_pDoc);
    pFormat->SetRange(ScRange(10, 10, 1, 10, 12, 1));
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 1);

    pFormatTmp->AddEntry(pEntry);

    // the conditional format should listen to A1:A3 on the second sheet
    for (SCROW nRow = 0; nRow < 3; ++nRow)
    {
        m_pDoc->SetValue(ScAddress(0, nRow, 1), 1.0);
        CPPUNIT_ASSERT(pEntry->NeedsRepaint());
    }

    m_pDoc->DeleteTab(0);

    // the conditional format should listen to A1:A3 on the second sheet
    for (SCROW nRow = 0; nRow < 3; ++nRow)
    {
        m_pDoc->SetValue(ScAddress(0, nRow, 0), 1.0);
        CPPUNIT_ASSERT(pEntry->NeedsRepaint());
    }

    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatUpdateReference()
{
    m_pDoc->InsertTab(0, "test");
    m_pDoc->InsertTab(1, "Test2");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "A1", "", m_pDoc, ScAddress(10, 10, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    auto pFormat = std::make_unique<ScConditionalFormat>(0, m_pDoc);
    pFormat->SetRange(ScRange(10, 10, 0, 10, 12, 0));
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    pFormatTmp->AddEntry(pEntry);

    // the conditional format should listen to A1:A3
    for (SCROW nRow = 0; nRow < 3; ++nRow)
    {
        m_pDoc->SetValue(ScAddress(0, nRow, 0), 1.0);
        CPPUNIT_ASSERT(pEntry->NeedsRepaint());
    }

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatUpdateReferenceDelRow()
{
    m_pDoc->InsertTab(0, "test");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "B6", "", m_pDoc, ScAddress(0, 5, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    auto pFormat = std::make_unique<ScConditionalFormat>(0, m_pDoc);
    pFormat->SetRange(ScRange(0, 5, 0, 0, 5, 0));
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    pFormatTmp->AddEntry(pEntry);

    m_pDoc->DeleteRow(0, 0, MAXCOL, 0, 4, 1);

    OUString aStr = pEntry->GetExpression(ScAddress(0, 4, 0), 0);
    CPPUNIT_ASSERT_EQUAL(OUString("B5"), aStr);

    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatUpdateReferenceInsRow()
{
    m_pDoc->InsertTab(0, "test");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "B6", "", m_pDoc, ScAddress(0, 5, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    auto pFormat = std::make_unique<ScConditionalFormat>(0, m_pDoc);
    pFormat->SetRange(ScRange(0, 5, 0, 0, 5, 0));
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    pFormatTmp->AddEntry(pEntry);

    m_pDoc->InsertRow(0, 0, MAXCOL, 0, 4, 1);

    OUString aStr = pEntry->GetExpression(ScAddress(0, 6, 0), 0);
    CPPUNIT_ASSERT_EQUAL(OUString("B7"), aStr);

    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatUndoList()
{
    m_pDoc->InsertTab(0, "test");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "B6", "", m_pDoc, ScAddress(0, 5, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    auto pFormat = std::make_unique<ScConditionalFormat>(0, m_pDoc);
    pFormat->AddEntry(pEntry);
    pFormat->SetRange(ScRange(0, 0, 0, 0, 5, 0));
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 0);
    m_pDoc->AddCondFormatData(pFormatTmp->GetRange(), 0, pFormatTmp->GetKey());

    ScDocFunc& rFunc = getDocShell().GetDocFunc();

    CPPUNIT_ASSERT_EQUAL(size_t(1), m_pDoc->GetCondFormList(0)->size());
    for (SCROW nRow = 0; nRow <= 5; ++nRow)
        CPPUNIT_ASSERT(m_pDoc->GetCondFormat(0, nRow, 0));

    ScConditionalFormatList* pNewList = new ScConditionalFormatList();

    rFunc.SetConditionalFormatList(pNewList, 0);

    CPPUNIT_ASSERT_EQUAL(size_t(0), m_pDoc->GetCondFormList(0)->size());
    for (SCROW nRow = 0; nRow <= 5; ++nRow)
        CPPUNIT_ASSERT(!m_pDoc->GetCondFormat(0, nRow, 0));

    m_pDoc->GetUndoManager()->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), m_pDoc->GetCondFormList(0)->size());
    for (SCROW nRow = 0; nRow <= 5; ++nRow)
        CPPUNIT_ASSERT(m_pDoc->GetCondFormat(0, nRow, 0));

    m_pDoc->GetUndoManager()->Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(0), m_pDoc->GetCondFormList(0)->size());
    for (SCROW nRow = 0; nRow <= 5; ++nRow)
        CPPUNIT_ASSERT(!m_pDoc->GetCondFormat(0, nRow, 0));

    m_pDoc->DeleteTab(0);
}

namespace {

sal_uInt32 addSingleCellCondFormat(ScDocument* pDoc, const ScAddress& rAddr, sal_uInt32 nKey, const OUString& rCondition)
{
    auto pFormat = std::make_unique<ScConditionalFormat>(nKey, pDoc);
    ScRange aCondFormatRange(rAddr);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct, rCondition, "",
            pDoc, ScAddress(0,0,0), ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    return pDoc->AddCondFormat(std::move(pFormat), 0);
}

}

void Test::testMultipleSingleCellCondFormatCopyPaste()
{
    m_pDoc->InsertTab(0, "Test");

    sal_uInt32 nFirstCondFormatKey = addSingleCellCondFormat(m_pDoc, ScAddress(0, 0, 0), 1, "=A2");
    sal_uInt32 nSecondCondFormatKey = addSingleCellCondFormat(m_pDoc, ScAddress(1, 0, 0), 2, "=B3");

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(0,0,0,2,0,0), &aClipDoc);
    ScRange aTargetRange(2,4,0,7,4,0);
    pasteOneCellFromClip(m_pDoc, aTargetRange, &aClipDoc);

    for (SCCOL nCol = 2; nCol <= 7; ++nCol)
    {
        ScConditionalFormat* pFormat = m_pDoc->GetCondFormat(nCol, 4, 0);
        if (((nCol - 2) % 3) == 0)
        {
            CPPUNIT_ASSERT_EQUAL(pFormat->GetKey(), nFirstCondFormatKey);
        }
        else if (((nCol - 2) % 3) == 1)
        {
            CPPUNIT_ASSERT_EQUAL(pFormat->GetKey(), nSecondCondFormatKey);
        }
        else
        {
            CPPUNIT_ASSERT(!pFormat);
        }
    }

    m_pDoc->DeleteTab(0);
}

void Test::testDeduplicateMultipleCondFormats()
{
    m_pDoc->InsertTab(0, "Test");

    sal_uInt32 nFirstCondFormatKey = addSingleCellCondFormat(m_pDoc, ScAddress(0, 0, 0), 1, "=B2");
    sal_uInt32 nSecondCondFormatKey = addSingleCellCondFormat(m_pDoc, ScAddress(1, 0, 0), 2, "=B2");

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(0,0,0,2,0,0), &aClipDoc);
    ScRange aTargetRange(2,4,0,7,4,0);
    pasteOneCellFromClip(m_pDoc, aTargetRange, &aClipDoc);

    for (SCCOL nCol = 2; nCol <= 7; ++nCol)
    {
        ScConditionalFormat* pFormat = m_pDoc->GetCondFormat(nCol, 4, 0);
        if (((nCol - 2) % 3) == 0)
        {
            CPPUNIT_ASSERT_EQUAL(pFormat->GetKey(), nFirstCondFormatKey);
        }
        else if (((nCol - 2) % 3) == 1)
        {
            CPPUNIT_ASSERT_EQUAL(pFormat->GetKey(), nSecondCondFormatKey);
        }
        else
        {
            CPPUNIT_ASSERT(!pFormat);
        }
    }

    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatListenToOwnRange()
{
    m_pDoc->InsertTab(0, "Test");

    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,10,0,0));
    pFormat->SetRange(aRangeList);

    ScIconSetFormat* pEntry = new ScIconSetFormat(m_pDoc);
    ScIconSetFormatData* pData = new ScIconSetFormatData;
    pData->m_Entries.push_back(std::make_unique<ScColorScaleEntry>(0, COL_BLUE));
    pData->m_Entries.push_back(std::make_unique<ScColorScaleEntry>(1, COL_GREEN));
    pData->m_Entries.push_back(std::make_unique<ScColorScaleEntry>(2, COL_RED));
    pEntry->SetIconSetData(pData);
    pEntry->SetParent(pFormat.get());

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    pFormat->AddEntry(pEntry);
    pList->InsertNew(std::move(pFormat));

    bool bFirstCallbackCalled = false;
    bool bSecondCallbackCalled = false;
    bool bThirdCallbackCalled = false;
    std::function<void()> aFirstCallback = [&]() {bFirstCallbackCalled = true;};
    std::function<void()> aSecondCallback = [&]() {bSecondCallbackCalled = true;};
    std::function<void()> aThirdCallback = [&]() {bThirdCallbackCalled = true;};
    pData->m_Entries[0]->SetType(COLORSCALE_PERCENT);
    pData->m_Entries[0]->SetRepaintCallback(aFirstCallback);

    m_pDoc->SetValue(0, 0, 0, -1.0);

    CPPUNIT_ASSERT(bFirstCallbackCalled);

    m_pDoc->DeleteTab(0);
}

void Test::testCondFormatVolatileFunctionRecalc()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetValue(0, 0, 0, 0.5);

    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,10,0,0));
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Greater,"RAND()","",m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pEntry->SetParent(pFormat.get());

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    pFormat->AddEntry(pEntry);
    auto pFormatTmp = pFormat.get();
    pList->InsertNew(std::move(pFormat));

    ScRefCellValue aCell(*m_pDoc, ScAddress(0, 0, 0));
    bool bValid = pEntry->IsCellValid(aCell, ScAddress(0, 0, 0));

    bool bNewValid = bValid;
    // chance of a random failure is 0.5^100, anyone hitting that will get a beer from me
    for (size_t i = 0; i < 100; ++i)
    {
        pFormatTmp->CalcAll();
        bNewValid = pEntry->IsCellValid(aCell, ScAddress(0, 0, 0));

        if (bValid != bNewValid)
            break;
    }

    CPPUNIT_ASSERT(bValid != bNewValid);

    m_pDoc->DeleteTab(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
