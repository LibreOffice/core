/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <globstr.hrc>
#include <scresid.hxx>
#include <docfunc.hxx>
#include <scdll.hxx>
#include <scitems.hxx>
#include <attrib.hxx>
#include <fillinfo.hxx>
#include <compiler.hxx>

#include <svl/sharedstringpool.hxx>

namespace {

struct ScDataBarLengthData
{
    double nVal;
    double nLength;
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

sal_uInt32 addSingleCellCondFormat(ScDocument* pDoc, const ScAddress& rAddr, sal_uInt32 nKey, const OUString& rCondition)
{
    auto pFormat = std::make_unique<ScConditionalFormat>(nKey, pDoc);
    ScRange aCondFormatRange(rAddr);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct, rCondition, "",
            *pDoc, ScAddress(0,0,0), ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    return pDoc->AddCondFormat(std::move(pFormat), 0);
}

}


class TestCondformat : public test::BootstrapFixture
{
public:
    TestCondformat();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testCondFormatINSDEL();
    void testCondFormatInsertRow();
    void testCondFormatInsertCol();
    void testCondFormatInsertDeleteSheets();
    void testCondCopyPaste();
    void testCondCopyPasteSingleCell(); //e.g. fdo#82503
    void testCondCopyPasteSingleCellToRange(); //e.g. fdo#82503
    void testCondCopyPasteSingleCellIntoSameFormatRange(); // e.g., tdf#95295
    void testCondCopyPasteSingleRowToRange(); //e.g. tdf#106242
    void testCondCopyPasteSingleRowToRange2();
    void testCondCopyPasteSheetBetweenDoc();
    void testCondCopyPasteSheet();

    void testIconSet();
    void testDataBarLengthAutomaticAxis();
    void testDataBarLengthMiddleAxis();

    // Tests for the ScFormulaListener class
    void testFormulaListenerSingleCellToSingleCell();
    void testFormulaListenerMultipleCellsToSingleCell();
    void testFormulaListenerSingleCellToMultipleCells();
    void testFormulaListenerMultipleCellsToMultipleCells();
    void testFormulaListenerUpdateInsertTab();
    void testFormulaListenerUpdateDeleteTab();

    // Check that the Listeners are correctly updated when we
    // call an operation
    void testCondFormatUpdateMoveTab();
    void testCondFormatUpdateDeleteTab();
    void testCondFormatUpdateInsertTab();
    void testCondFormatUpdateReference();
    void testCondFormatUpdateReferenceDelRow();
    void testCondFormatUpdateReferenceInsRow();

    void testCondFormatEndsWithStr();
    void testCondFormatEndsWithVal();

    void testCondFormatUndoList();
    void testMultipleSingleCellCondFormatCopyPaste();
    void testDeduplicateMultipleCondFormats();
    void testCondFormatListenToOwnRange();
    void testCondFormatVolatileFunctionRecalc();

    CPPUNIT_TEST_SUITE(TestCondformat);

    CPPUNIT_TEST(testCondFormatINSDEL);
    CPPUNIT_TEST(testCondFormatInsertRow);
    CPPUNIT_TEST(testCondFormatInsertCol);
    CPPUNIT_TEST(testCondFormatInsertDeleteSheets);
    CPPUNIT_TEST(testCondCopyPaste);
    CPPUNIT_TEST(testCondCopyPasteSingleCell);
    CPPUNIT_TEST(testCondCopyPasteSingleCellToRange);
    CPPUNIT_TEST(testCondCopyPasteSingleCellIntoSameFormatRange);
    CPPUNIT_TEST(testCondCopyPasteSingleRowToRange);
    CPPUNIT_TEST(testCondCopyPasteSingleRowToRange2);
    CPPUNIT_TEST(testCondCopyPasteSheetBetweenDoc);
    CPPUNIT_TEST(testCondCopyPasteSheet);
    CPPUNIT_TEST(testCondFormatEndsWithStr);
    CPPUNIT_TEST(testCondFormatEndsWithVal);
    CPPUNIT_TEST(testCondFormatUpdateMoveTab);
    CPPUNIT_TEST(testCondFormatUpdateDeleteTab);
    CPPUNIT_TEST(testCondFormatUpdateInsertTab);
    CPPUNIT_TEST(testCondFormatUpdateReference);
    CPPUNIT_TEST(testCondFormatUpdateReferenceDelRow);
    CPPUNIT_TEST(testCondFormatUpdateReferenceInsRow);
    CPPUNIT_TEST(testCondFormatUndoList);
    CPPUNIT_TEST(testMultipleSingleCellCondFormatCopyPaste);
    CPPUNIT_TEST(testDeduplicateMultipleCondFormats);
    CPPUNIT_TEST(testCondFormatListenToOwnRange);
    CPPUNIT_TEST(testCondFormatVolatileFunctionRecalc);
    CPPUNIT_TEST(testIconSet);
    CPPUNIT_TEST(testDataBarLengthAutomaticAxis);
    CPPUNIT_TEST(testDataBarLengthMiddleAxis);
    CPPUNIT_TEST(testFormulaListenerSingleCellToSingleCell);
    CPPUNIT_TEST(testFormulaListenerSingleCellToMultipleCells);
    CPPUNIT_TEST(testFormulaListenerMultipleCellsToSingleCell);
    CPPUNIT_TEST(testFormulaListenerMultipleCellsToMultipleCells);
    CPPUNIT_TEST(testFormulaListenerUpdateInsertTab);
    CPPUNIT_TEST(testFormulaListenerUpdateDeleteTab);

    CPPUNIT_TEST_SUITE_END();

private:
    ScDocShellRef m_xDocShell;
    ScDocument* m_pDoc;
};

TestCondformat::TestCondformat()
{
}

void TestCondformat::setUp()
{
    BootstrapFixture::setUp();

    ScDLL::Init();

    m_xDocShell = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    m_xDocShell->SetIsInUcalc();
    m_xDocShell->DoInitUnitTest();

    m_pDoc = &m_xDocShell->GetDocument();
}

void TestCondformat::tearDown()
{
    m_xDocShell->DoClose();
    m_xDocShell.clear();

    test::BootstrapFixture::tearDown();
}

void TestCondformat::testCondFormatINSDEL()
{
    // fdo#62206
    m_pDoc->InsertTab(0, "Test");
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,0,3,0));
    pFormat->SetRange(aRangeList);
    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
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

void TestCondformat::testCondFormatInsertCol()
{
    m_pDoc->InsertTab(0, "Test");
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,3,3,0));
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    auto pFormatTmp = pFormat.get();
    pList->InsertNew(std::move(pFormat));

    m_pDoc->InsertCol(0,0,MAXROW,0,4,2);
    const ScRangeList& rRange = pFormatTmp->GetRange();
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(0,0,0,5,3,0)), rRange);

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testCondFormatInsertRow()
{
    m_pDoc->InsertTab(0, "Test");
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,3,3,0));
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    auto pFormatTmp = pFormat.get();
    pList->InsertNew(std::move(pFormat));

    m_pDoc->InsertRow(0,0,MAXCOL,0,4,2);
    const ScRangeList& rRange = pFormatTmp->GetRange();
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(0,0,0,3,5,0)), rRange);

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testCondFormatInsertDeleteSheets()
{
    m_pDoc->InsertTab(0, "Test");

    // Add a conditional format to B2:B4.
    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    pFormat->SetRange(ScRange(1,1,0,1,3,0));

    auto pFormatTmp = pFormat.get();
    sal_uLong nKey = m_pDoc->AddCondFormat(std::move(pFormat), 0);

    // Add condition in which if the value equals 2, set the "Result" style.
    ScCondFormatEntry* pEntry = new ScCondFormatEntry(
        ScConditionMode::Equal, "=2", "" , *m_pDoc, ScAddress(0,0,0), ScResId(STR_STYLENAME_RESULT));
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

    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();

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

void TestCondformat::testCondCopyPaste()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
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
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pCondFormatItem->GetCondFormatData().front());

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testCondCopyPasteSingleCell()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
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
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pCondFormatItem->GetCondFormatData().front() );

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testCondCopyPasteSingleCellToRange()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
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
            CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pCondFormatItem->GetCondFormatData().front() );
        }
    }

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testCondCopyPasteSingleCellIntoSameFormatRange()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0, 0, 0, 3, 3, 0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct, "=B2", "", *m_pDoc, ScAddress(0, 0, 0), ScResId(STR_STYLENAME_RESULT));
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
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pCondFormatItem->GetCondFormatData().front());

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testCondCopyPasteSingleRowToRange()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,0,0,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
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

void TestCondformat::testCondCopyPasteSingleRowToRange2()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,0,0,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
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

void TestCondformat::testCondCopyPasteSheetBetweenDoc()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aDoc;
    aDoc.TransferTab(*m_pDoc, 0, 0);

    ScConditionalFormatList* pList = aDoc.GetCondFormList(0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testCondCopyPasteSheet()
{
    m_pDoc->InsertTab(0, "Test");

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,"=B2","",*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
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
    CPPUNIT_ASSERT_EQUAL( nKey, pCondFormatItem->GetCondFormatData().front() );

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void TestCondformat::testIconSet()
{
    m_pDoc->InsertTab(0, "Test");
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,0,0,0));
    pFormat->SetRange(aRangeList);

    ScIconSetFormat* pEntry = new ScIconSetFormat(m_pDoc);
    ScIconSetFormatData* pData = new ScIconSetFormatData;
    pData->m_Entries.emplace_back(new ScColorScaleEntry(0, COL_BLUE));
    pData->m_Entries.emplace_back(new ScColorScaleEntry(1, COL_GREEN));
    pData->m_Entries.emplace_back(new ScColorScaleEntry(2, COL_RED));
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

void TestCondformat::testDataBarLengthAutomaticAxis()
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

void TestCondformat::testDataBarLengthMiddleAxis()
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

void TestCondformat::testCondFormatEndsWithStr()
{
    m_pDoc->InsertTab(0, "Test");

    ScConditionEntry aEntry(ScConditionMode::EndsWith, "\"TestString\"", "", *m_pDoc, ScAddress(),
            "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    svl::SharedStringPool& rStringPool = m_pDoc->GetSharedStringPool();
    svl::SharedString aStr = rStringPool.intern("SimpleTestString");
    ScRefCellValue aVal(&aStr);
    ScAddress aPos(0, 0, 0);

    bool bValid = aEntry.IsCellValid(aVal, aPos);
    CPPUNIT_ASSERT(bValid);

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testCondFormatEndsWithVal()
{
    m_pDoc->InsertTab(0, "Test");

    ScConditionEntry aEntry(ScConditionMode::EndsWith, "2", "", *m_pDoc, ScAddress(),
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

void TestCondformat::testFormulaListenerSingleCellToSingleCell()
{
    m_pDoc->InsertTab(0, "test");

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(*m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testFormulaListenerSingleCellToMultipleCells()
{
    m_pDoc->InsertTab(0, "test");

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(*m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testFormulaListenerMultipleCellsToSingleCell()
{
    m_pDoc->InsertTab(0, "test");

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(*m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testFormulaListenerMultipleCellsToMultipleCells()
{
    m_pDoc->InsertTab(0, "test");

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(*m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testFormulaListenerUpdateInsertTab()
{
    m_pDoc->InsertTab(0, "test");

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);
    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(*m_pDoc);
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

void TestCondformat::testFormulaListenerUpdateDeleteTab()
{
    m_pDoc->InsertTab(0, "test");
    m_pDoc->InsertTab(0, "to_delete");

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 1), formula::FormulaGrammar::GRAM_ENGLISH);
    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString("A1"));

    ScFormulaListener aListener(*m_pDoc);
    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 1));
    CPPUNIT_ASSERT(!aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);

    // check that the listener has moved
    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testCondFormatUpdateMoveTab()
{
    m_pDoc->InsertTab(0, "test");
    m_pDoc->InsertTab(1, "Test2");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "A1", "", *m_pDoc, ScAddress(10, 10, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

void TestCondformat::testCondFormatUpdateInsertTab()
{
    m_pDoc->InsertTab(0, "test");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "A1", "", *m_pDoc, ScAddress(10, 10, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

void TestCondformat::testCondFormatUpdateDeleteTab()
{
    m_pDoc->InsertTab(0, "test");
    m_pDoc->InsertTab(1, "Test2");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "A1", "", *m_pDoc, ScAddress(10, 10, 1), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

void TestCondformat::testCondFormatUpdateReference()
{
    m_pDoc->InsertTab(0, "test");
    m_pDoc->InsertTab(1, "Test2");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "A1", "", *m_pDoc, ScAddress(10, 10, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

void TestCondformat::testCondFormatUpdateReferenceDelRow()
{
    m_pDoc->InsertTab(0, "test");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "B6", "", *m_pDoc, ScAddress(0, 5, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

void TestCondformat::testCondFormatUpdateReferenceInsRow()
{
    m_pDoc->InsertTab(0, "test");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "B6", "", *m_pDoc, ScAddress(0, 5, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

void TestCondformat::testCondFormatUndoList()
{
    m_pDoc->InsertTab(0, "test");

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, "B6", "", *m_pDoc, ScAddress(0, 5, 0), "", "", formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    auto pFormat = std::make_unique<ScConditionalFormat>(0, m_pDoc);
    pFormat->AddEntry(pEntry);
    pFormat->SetRange(ScRange(0, 0, 0, 0, 5, 0));
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 0);
    m_pDoc->AddCondFormatData(pFormatTmp->GetRange(), 0, pFormatTmp->GetKey());

    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();

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

void TestCondformat::testMultipleSingleCellCondFormatCopyPaste()
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

void TestCondformat::testDeduplicateMultipleCondFormats()
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

void TestCondformat::testCondFormatListenToOwnRange()
{
    m_pDoc->InsertTab(0, "Test");

    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,10,0,0));
    pFormat->SetRange(aRangeList);

    ScIconSetFormat* pEntry = new ScIconSetFormat(m_pDoc);
    ScIconSetFormatData* pData = new ScIconSetFormatData;
    pData->m_Entries.emplace_back(new ScColorScaleEntry(0, COL_BLUE));
    pData->m_Entries.emplace_back(new ScColorScaleEntry(1, COL_GREEN));
    pData->m_Entries.emplace_back(new ScColorScaleEntry(2, COL_RED));
    pEntry->SetIconSetData(pData);
    pEntry->SetParent(pFormat.get());

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    pFormat->AddEntry(pEntry);
    pList->InsertNew(std::move(pFormat));

    bool bFirstCallbackCalled = false;
    std::function<void()> aFirstCallback = [&]() {bFirstCallbackCalled = true;};
    pData->m_Entries[0]->SetType(COLORSCALE_PERCENT);
    pData->m_Entries[0]->SetRepaintCallback(aFirstCallback);

    m_pDoc->SetValue(0, 0, 0, -1.0);

    CPPUNIT_ASSERT(bFirstCallbackCalled);

    m_pDoc->DeleteTab(0);
}

void TestCondformat::testCondFormatVolatileFunctionRecalc()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetValue(0, 0, 0, 0.5);

    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,10,0,0));
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Greater,"RAND()","",*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
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

CPPUNIT_TEST_SUITE_REGISTRATION(TestCondformat);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
