/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <conditio.hxx>
#include <colorscale.hxx>

#include <hints.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <docfunc.hxx>
#include <scitems.hxx>
#include <attrib.hxx>
#include <fillinfo.hxx>
#include <compiler.hxx>
#include <undomanager.hxx>

#include <svl/sharedstringpool.hxx>

namespace {

struct PaintListener : public SfxListener
{
    bool mbPaintAllMergedCell = false;
    virtual void Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint) override
    {
        const ScPaintHint* pPaintHint = dynamic_cast<const ScPaintHint*>(&rHint);
        if (pPaintHint)
        {
            if (pPaintHint->GetStartCol() == 0 && pPaintHint->GetEndCol() == 0
                && pPaintHint->GetStartRow() == 0 && pPaintHint->GetEndRow() == 1)
            {
                mbPaintAllMergedCell = true;
            }
        }
    }
};

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

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct, rCondition, u""_ustr,
            *pDoc, ScAddress(0,0,0), ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    return pDoc->AddCondFormat(std::move(pFormat), 0);
}

}

class TestCondformat : public ScUcalcTestBase
{
};

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatINSDEL)
{
    // fdo#62206
    m_pDoc->InsertTab(0, u"Test"_ustr);
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,0,3,0));
    pFormat->SetRange(aRangeList);
    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,u"=B2"_ustr,u""_ustr,*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    auto pFormatTmp = pFormat.get();
    pList->InsertNew(std::move(pFormat));

    m_pDoc->InsertCol(0,0,m_pDoc->MaxRow(),0,0,2);
    const ScRangeList& rRange = pFormatTmp->GetRange();
    CPPUNIT_ASSERT_EQUAL(static_cast<const ScRangeList&>(ScRange(2,0,0,2,3,0)), rRange);

    OUString aExpr = pEntry->GetExpression(ScAddress(2,0,0), 0);
    CPPUNIT_ASSERT_EQUAL(u"D2"_ustr, aExpr);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatInsertCol)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,3,3,0));
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,u"=B2"_ustr,u""_ustr,*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    auto pFormatTmp = pFormat.get();
    pList->InsertNew(std::move(pFormat));

    m_pDoc->InsertCol(0,0,m_pDoc->MaxRow(),0,4,2);
    const ScRangeList& rRange = pFormatTmp->GetRange();
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(0,0,0,5,3,0)), rRange);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatInsertRow)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,3,3,0));
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,u"=B2"_ustr,u""_ustr,*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    auto pFormatTmp = pFormat.get();
    pList->InsertNew(std::move(pFormat));

    m_pDoc->InsertRow(0,0,m_pDoc->MaxCol(),0,4,2);
    const ScRangeList& rRange = pFormatTmp->GetRange();
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(0,0,0,3,5,0)), rRange);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatInsertDeleteSheets)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Add a conditional format to B2:B4.
    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    pFormat->SetRange(ScRange(1,1,0,1,3,0));

    auto pFormatTmp = pFormat.get();
    sal_uInt32 nKey = m_pDoc->AddCondFormat(std::move(pFormat), 0);

    // Add condition in which if the value equals 2, set the "Result" style.
    ScCondFormatEntry* pEntry = new ScCondFormatEntry(
        ScConditionMode::Equal, u"=2"_ustr, u""_ustr , *m_pDoc, ScAddress(0,0,0), ScResId(STR_STYLENAME_RESULT));
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
    bool bInserted = rFunc.InsertTable(0, u"Inserted"_ustr, true, true);
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

CPPUNIT_TEST_FIXTURE(TestCondformat, testDataBarCondCopyPaste)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0, 0, 0, 2, 0, 0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScDataBarFormat* pDatabar = new ScDataBarFormat(m_pDoc);
    ScDataBarFormatData* pFormatData = new ScDataBarFormatData();
    pFormatData->meAxisPosition = databar::AUTOMATIC;
    pFormatData->maPositiveColor = COL_BLUE;
    pFormatData->mxNegativeColor = COL_GREEN;
    pFormatData->mbGradient = true;

    pDatabar->SetDataBarData(pFormatData);
    pFormat->AddEntry(pDatabar);

    sal_uInt32 nIndex = m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aCondFormatRange, &aClipDoc);

    ScRange aTargetRange(0, 3, 0, 2, 3, 0);
    pasteFromClip(m_pDoc, aTargetRange, &aClipDoc);

    // Pasting the same conditional format must modify existing format, making its range
    // combined of previous range and newly pasted range having the conditional format.
    // No new conditional formats must be created.
    CPPUNIT_ASSERT_EQUAL(size_t(1), m_pDoc->GetCondFormList(0)->size());
    aRangeList.Join(aTargetRange);
    for (SCCOL nCol = 0; nCol < 3; ++nCol)
    {
        ScConditionalFormat* pPastedFormat = m_pDoc->GetCondFormat(nCol, 3, 0);
        CPPUNIT_ASSERT(pPastedFormat);
        CPPUNIT_ASSERT_EQUAL(aRangeList, pPastedFormat->GetRange());

        sal_uInt32 nPastedKey = pPastedFormat->GetKey();
        CPPUNIT_ASSERT_EQUAL(nIndex, nPastedKey);

        const SfxPoolItem* pItem = m_pDoc->GetAttr(nCol, 3, 0, ATTR_CONDITIONAL);
        const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);
        CPPUNIT_ASSERT(pCondFormatItem);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormatItem->GetCondFormatData().size());
        CPPUNIT_ASSERT_EQUAL(nIndex, pCondFormatItem->GetCondFormatData().front());
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testColorScaleInMergedCell)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);
    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);

    // Add a conditional format to A1.
    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    pFormat->SetRange(ScRange(0, 0, 0, 0, 0, 0));
    auto pFormatTmp = pFormat.get();
    sal_uInt32 nKey = m_pDoc->AddCondFormat(std::move(pFormat), 0);

    // Add color scale entries.
    // The coloring is based on the value. (BLUE (x <= 0), GREEN (x == 1), RED (x >= 2))
    ScColorScaleFormat* pColorScaleFormat = new ScColorScaleFormat(m_pDoc);
    ScColorScaleEntry* pEntryBlue = new ScColorScaleEntry(0, COL_BLUE);
    ScColorScaleEntry* pEntryGreen = new ScColorScaleEntry(1, COL_GREEN);
    ScColorScaleEntry* pEntryRed = new ScColorScaleEntry(2, COL_RED);
    pColorScaleFormat->AddEntry(pEntryBlue);
    pColorScaleFormat->AddEntry(pEntryGreen);
    pColorScaleFormat->AddEntry(pEntryRed);

    pFormatTmp->AddEntry(pColorScaleFormat);

    // Apply the format to the range.
    m_pDoc->AddCondFormatData(pFormatTmp->GetRange(), 0, nKey);

    m_pDoc->DoMerge(0, 0, 0, 1, 0);  // A1:A2
    CPPUNIT_ASSERT(m_pDoc->IsMerged(ScAddress(0, 0, 0)));

    ScTableInfo aTabInfo(0, 2, false);
    m_pDoc->FillInfo(aTabInfo, 0, 0, 0, 1, 0, 1, 1, false, false);
    RowInfo* pRowInfo = aTabInfo.mpRowInfo.get();

    RowInfo* pRowInfoA1 = &pRowInfo[1];
    ScCellInfo* pCellInfoA1 = &pRowInfoA1->cellInfo(0);
    // Check if there is a color scale in A1.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There is no color scale in cell A1!", true,
                                 pCellInfoA1->mxColorScale.has_value());

    RowInfo* pRowInfoA2 = &pRowInfo[2];
    ScCellInfo* pCellInfoA2 = &pRowInfoA2->cellInfo(0);
    // Check if there is a color scale in A2.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There is no color scale in cell A2!", true,
                                 pCellInfoA2->mxColorScale.has_value());

    // Check that cells A1 and A2 have the same color scale. (GREEN)
    CPPUNIT_ASSERT(pCellInfoA1->mxColorScale.value().IsRGBEqual(pCellInfoA2->mxColorScale.value()));

    m_pDoc->DeleteTab(0);
}


CPPUNIT_TEST_FIXTURE(TestCondformat, testColorScaleCondCopyPaste)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0, 0, 0, 2, 0, 0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScColorScaleFormat* pColorScaleFormat = new ScColorScaleFormat(m_pDoc);
    ScColorScaleEntry* pEntryBlue = new ScColorScaleEntry(0, COL_BLUE);
    ScColorScaleEntry* pEntryGreen = new ScColorScaleEntry(1, COL_GREEN);
    ScColorScaleEntry* pEntryRed = new ScColorScaleEntry(2, COL_RED);
    pColorScaleFormat->AddEntry(pEntryBlue);
    pColorScaleFormat->AddEntry(pEntryGreen);
    pColorScaleFormat->AddEntry(pEntryRed);

    pFormat->AddEntry(pColorScaleFormat);
    sal_uInt32 nIndex = m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aCondFormatRange, &aClipDoc);

    ScRange aTargetRange(0, 3, 0, 2, 3, 0);
    pasteFromClip(m_pDoc, aTargetRange, &aClipDoc);

    // Pasting the same conditional format must modify existing format, making its range
    // combined of previous range and newly pasted range having the conditional format.
    // No new conditional formats must be created.
    CPPUNIT_ASSERT_EQUAL(size_t(1), m_pDoc->GetCondFormList(0)->size());
    aRangeList.Join(aTargetRange);
    for (SCCOL nCol = 0; nCol < 3; ++nCol)
    {
        ScConditionalFormat* pPastedFormat = m_pDoc->GetCondFormat(nCol, 3, 0);
        CPPUNIT_ASSERT(pPastedFormat);
        CPPUNIT_ASSERT_EQUAL(aRangeList, pPastedFormat->GetRange());

        sal_uInt32 nPastedKey = pPastedFormat->GetKey();
        CPPUNIT_ASSERT_EQUAL(nIndex, nPastedKey);

        const SfxPoolItem* pItem = m_pDoc->GetAttr(nCol, 3, 0, ATTR_CONDITIONAL);
        const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);
        CPPUNIT_ASSERT(pCondFormatItem);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormatItem->GetCondFormatData().size());
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(nIndex), pCondFormatItem->GetCondFormatData().front());
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondCopyPaste)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,u"=B2"_ustr,u""_ustr,*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    sal_uInt32 nIndex = m_pDoc->AddCondFormat(std::move(pFormat), 0);

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
    CPPUNIT_ASSERT_EQUAL(nIndex, pPastedFormat->GetKey());
    const SfxPoolItem* pItem = m_pDoc->GetAttr( 7, 7, 0, ATTR_CONDITIONAL );
    const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);

    CPPUNIT_ASSERT(pCondFormatItem);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormatItem->GetCondFormatData().size());
    CPPUNIT_ASSERT_EQUAL(nIndex, pCondFormatItem->GetCondFormatData().front());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondCopyPasteSingleCell)
{
    //e.g. fdo#82503
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,u"=B2"_ustr,u""_ustr,*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    sal_uInt32 nIndex = m_pDoc->AddCondFormat(std::move(pFormat), 0);

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
    CPPUNIT_ASSERT_EQUAL(nIndex, pPastedFormat->GetKey());
    const SfxPoolItem* pItem = m_pDoc->GetAttr( 4, 4, 0, ATTR_CONDITIONAL );
    const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);

    CPPUNIT_ASSERT(pCondFormatItem);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormatItem->GetCondFormatData().size());
    CPPUNIT_ASSERT_EQUAL(nIndex, pCondFormatItem->GetCondFormatData().front() );

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondCopyPasteSingleCellToRange)
{
    //e.g. fdo#82503
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,u"=B2"_ustr,u""_ustr,*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    sal_uInt32 nIndex = m_pDoc->AddCondFormat(std::move(pFormat), 0);

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
            sal_uInt32 nPastedKey = pPastedFormat->GetKey();
            CPPUNIT_ASSERT_EQUAL(nIndex, nPastedKey);
            const SfxPoolItem* pItem = m_pDoc->GetAttr( nCol, nRow, 0, ATTR_CONDITIONAL );
            const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);

            CPPUNIT_ASSERT(pCondFormatItem);
            CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormatItem->GetCondFormatData().size());
            CPPUNIT_ASSERT_EQUAL(nIndex, pCondFormatItem->GetCondFormatData().front() );
        }
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondCopyPasteSingleCellIntoSameFormatRange)
{
    // e.g., tdf#95295
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0, 0, 0, 3, 3, 0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct, u"=B2"_ustr, u""_ustr, *m_pDoc, ScAddress(0, 0, 0), ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    sal_uInt32 nIndex = m_pDoc->AddCondFormat(std::move(pFormat), 0);

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
    CPPUNIT_ASSERT_EQUAL(nIndex, pPastedFormat->GetKey());
    const SfxPoolItem* pItem = m_pDoc->GetAttr(2, 2, 0, ATTR_CONDITIONAL);
    const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);

    CPPUNIT_ASSERT(pCondFormatItem);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormatItem->GetCondFormatData().size());
    CPPUNIT_ASSERT_EQUAL(nIndex, pCondFormatItem->GetCondFormatData().front());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondCopyPasteSingleRowToRange)
{
    //e.g. tdf#106242
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,0,0,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,u"=B2"_ustr,u""_ustr,*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(0,0,0,m_pDoc->MaxCol(),0,0), &aClipDoc);
    ScRange aTargetRange(0,4,0,m_pDoc->MaxCol(),4,0);
    pasteOneCellFromClip(m_pDoc, aTargetRange, &aClipDoc);

    ScConditionalFormat* pNewFormat = m_pDoc->GetCondFormat(0, 4, 0);
    CPPUNIT_ASSERT(pNewFormat);
    CPPUNIT_ASSERT_EQUAL(pNewFormat->GetKey(), pFormatTmp->GetKey());

    for (SCCOL nCol = 1; nCol <= m_pDoc->MaxCol(); ++nCol)
    {
        ScConditionalFormat* pNewFormat2 = m_pDoc->GetCondFormat(nCol, 4, 0);
        CPPUNIT_ASSERT(!pNewFormat2);
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondCopyPasteSingleRowToRange2)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,0,0,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,u"=B2"_ustr,u""_ustr,*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(0,0,0,3,0,0), &aClipDoc);
    ScRange aTargetRange(0,4,0,m_pDoc->MaxCol(),4,0);
    pasteOneCellFromClip(m_pDoc, aTargetRange, &aClipDoc);

    for (SCCOL nCol = 0; nCol <= m_pDoc->MaxCol(); ++nCol)
    {
        ScConditionalFormat* pNewFormat = m_pDoc->GetCondFormat(nCol, 4, 0);
        if (nCol % 4 == 0)
            CPPUNIT_ASSERT(pNewFormat);
        else
            CPPUNIT_ASSERT(!pNewFormat);
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondCopyPasteSheetBetweenDoc)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,u"=B2"_ustr,u""_ustr,*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    ScDocument aDoc;
    aDoc.TransferTab(*m_pDoc, 0, 0);

    ScConditionalFormatList* pList = aDoc.GetCondFormList(0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondCopyPasteSheet)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRange aCondFormatRange(0,0,0,3,3,0);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct,u"=B2"_ustr,u""_ustr,*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
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

CPPUNIT_TEST_FIXTURE(TestCondformat, testIconSet)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,0,0,0));
    pFormat->SetRange(aRangeList);

    ScIconSetFormat* pEntry = new ScIconSetFormat(m_pDoc);
    ScIconSetFormatData* pData = new ScIconSetFormatData;
    pData->m_Entries.emplace_back(new ScColorScaleEntry(0, COL_BLUE, COLORSCALE_VALUE, ScConditionMode::EqGreater));
    pData->m_Entries.emplace_back(new ScColorScaleEntry(1, COL_GREEN, COLORSCALE_VALUE, ScConditionMode::EqGreater));
    pData->m_Entries.emplace_back(new ScColorScaleEntry(2, COL_RED, COLORSCALE_VALUE, ScConditionMode::Equal));
    pEntry->SetIconSetData(pData);

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    pList->InsertNew(std::move(pFormat));

    static struct {
        double nVal; sal_Int32 nIndex;
    } const aTests[] = {
        { 0.0, 0 },
        { 1.0, 1 },
        { 2.0, 2 },
        { 3.0, 1 }
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

CPPUNIT_TEST_FIXTURE(TestCondformat, testDataBarLengthAutomaticAxis)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

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

CPPUNIT_TEST_FIXTURE(TestCondformat, testDataBarLengthMiddleAxis)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

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

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatEndsWithStr)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    // case insensitive matching
    ScConditionEntry aEntry(ScConditionMode::EndsWith, u"\"teststring\""_ustr, u""_ustr, *m_pDoc, ScAddress(),
            u""_ustr, u""_ustr, formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    svl::SharedStringPool& rStringPool = m_pDoc->GetSharedStringPool();
    svl::SharedString aStr = rStringPool.intern(u"SimpleTestString"_ustr);
    ScRefCellValue aVal(&aStr);
    ScAddress aPos(0, 0, 0);

    bool bValid = aEntry.IsCellValid(aVal, aPos);
    CPPUNIT_ASSERT(bValid);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatEndsWithVal)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    ScConditionEntry aEntry(ScConditionMode::EndsWith, u"2"_ustr, u""_ustr, *m_pDoc, ScAddress(),
            u""_ustr, u""_ustr, formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

CPPUNIT_TEST_FIXTURE(TestCondformat, testFormulaListenerSingleCellToSingleCell)
{
    m_pDoc->InsertTab(0, u"test"_ustr);

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString(u"A1"_ustr));

    ScFormulaListener aListener(*m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testFormulaListenerSingleCellToMultipleCells)
{
    m_pDoc->InsertTab(0, u"test"_ustr);

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString(u"A1"_ustr));

    ScFormulaListener aListener(*m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testFormulaListenerMultipleCellsToSingleCell)
{
    m_pDoc->InsertTab(0, u"test"_ustr);

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString(u"A1"_ustr));

    ScFormulaListener aListener(*m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testFormulaListenerMultipleCellsToMultipleCells)
{
    m_pDoc->InsertTab(0, u"test"_ustr);

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);

    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString(u"A1"_ustr));

    ScFormulaListener aListener(*m_pDoc);

    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testFormulaListenerUpdateInsertTab)
{
    m_pDoc->InsertTab(0, u"test"_ustr);

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 0), formula::FormulaGrammar::GRAM_ENGLISH);
    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString(u"A1"_ustr));

    ScFormulaListener aListener(*m_pDoc);
    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 0));
    CPPUNIT_ASSERT(!aListener.NeedsRepaint());

    m_pDoc->InsertTab(0, u"new_tab"_ustr);

    // check that the listener has moved to the new sheet
    m_pDoc->SetValue(ScAddress(0, 0, 1), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    // check that we are not listening to the old sheet
    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(!aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testFormulaListenerUpdateDeleteTab)
{
    m_pDoc->InsertTab(0, u"test"_ustr);
    m_pDoc->InsertTab(0, u"to_delete"_ustr);

    ScCompiler aCompiler(*m_pDoc, ScAddress(10, 10, 1), formula::FormulaGrammar::GRAM_ENGLISH);
    std::unique_ptr<ScTokenArray> pTokenArray(aCompiler.CompileString(u"A1"_ustr));

    ScFormulaListener aListener(*m_pDoc);
    aListener.addTokenArray(pTokenArray.get(), ScAddress(10, 10, 1));
    CPPUNIT_ASSERT(!aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);

    // check that the listener has moved
    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    CPPUNIT_ASSERT(aListener.NeedsRepaint());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatUpdateMoveTab)
{
    m_pDoc->InsertTab(0, u"test"_ustr);
    m_pDoc->InsertTab(1, u"Test2"_ustr);

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, u"A1"_ustr, u""_ustr, *m_pDoc, ScAddress(10, 10, 0), u""_ustr, u""_ustr, formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatUpdateInsertTab)
{
    m_pDoc->InsertTab(0, u"test"_ustr);

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, u"A1"_ustr, u""_ustr, *m_pDoc, ScAddress(10, 10, 0), u""_ustr, u""_ustr, formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

    m_pDoc->InsertTab(0, u"test2"_ustr);

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

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatUpdateDeleteTab)
{
    m_pDoc->InsertTab(0, u"test"_ustr);
    m_pDoc->InsertTab(1, u"Test2"_ustr);

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, u"A1"_ustr, u""_ustr, *m_pDoc, ScAddress(10, 10, 1), u""_ustr, u""_ustr, formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatUpdateReference)
{
    m_pDoc->InsertTab(0, u"test"_ustr);
    m_pDoc->InsertTab(1, u"Test2"_ustr);

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, u"A1"_ustr, u""_ustr, *m_pDoc, ScAddress(10, 10, 0), u""_ustr, u""_ustr, formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatUpdateReferenceDelRow)
{
    m_pDoc->InsertTab(0, u"test"_ustr);

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, u"B6"_ustr, u""_ustr, *m_pDoc, ScAddress(0, 5, 0), u""_ustr, u""_ustr, formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    auto pFormat = std::make_unique<ScConditionalFormat>(0, m_pDoc);
    pFormat->SetRange(ScRange(0, 5, 0, 0, 5, 0));
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    pFormatTmp->AddEntry(pEntry);

    m_pDoc->DeleteRow(0, 0, m_pDoc->MaxCol(), 0, 4, 1);

    OUString aStr = pEntry->GetExpression(ScAddress(0, 4, 0), 0);
    CPPUNIT_ASSERT_EQUAL(u"B5"_ustr, aStr);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatUpdateReferenceInsRow)
{
    m_pDoc->InsertTab(0, u"test"_ustr);

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, u"B6"_ustr, u""_ustr, *m_pDoc, ScAddress(0, 5, 0), u""_ustr, u""_ustr, formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

    auto pFormat = std::make_unique<ScConditionalFormat>(0, m_pDoc);
    pFormat->SetRange(ScRange(0, 5, 0, 0, 5, 0));
    auto pFormatTmp = pFormat.get();
    m_pDoc->AddCondFormat(std::move(pFormat), 0);

    pFormatTmp->AddEntry(pEntry);

    m_pDoc->InsertRow(0, 0, m_pDoc->MaxCol(), 0, 4, 1);

    OUString aStr = pEntry->GetExpression(ScAddress(0, 6, 0), 0);
    CPPUNIT_ASSERT_EQUAL(u"B7"_ustr, aStr);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatUndoList)
{
    m_pDoc->InsertTab(0, u"test"_ustr);

    ScConditionEntry* pEntry = new ScConditionEntry(ScConditionMode::Equal, u"B6"_ustr, u""_ustr, *m_pDoc, ScAddress(0, 5, 0), u""_ustr, u""_ustr, formula::FormulaGrammar::GRAM_DEFAULT, formula::FormulaGrammar::GRAM_DEFAULT);

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

CPPUNIT_TEST_FIXTURE(TestCondformat, testMultipleSingleCellCondFormatCopyPaste)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    sal_uInt32 nFirstCondFormatKey = addSingleCellCondFormat(m_pDoc, ScAddress(0, 0, 0), 1, u"=A2"_ustr);
    sal_uInt32 nSecondCondFormatKey = addSingleCellCondFormat(m_pDoc, ScAddress(1, 0, 0), 2, u"=B3"_ustr);

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

CPPUNIT_TEST_FIXTURE(TestCondformat, testDeduplicateMultipleCondFormats)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    sal_uInt32 nFirstCondFormatKey = addSingleCellCondFormat(m_pDoc, ScAddress(0, 0, 0), 1, u"=B2"_ustr);
    sal_uInt32 nSecondCondFormatKey = addSingleCellCondFormat(m_pDoc, ScAddress(1, 0, 0), 2, u"=B2"_ustr);

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

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatListenToOwnRange)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

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

CPPUNIT_TEST_FIXTURE(TestCondformat, testCondFormatVolatileFunctionRecalc)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    m_pDoc->SetValue(0, 0, 0, 0.5);

    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,10,0,0));
    pFormat->SetRange(aRangeList);

    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Greater,u"RAND()"_ustr,u""_ustr,*m_pDoc,ScAddress(0,0,0),ScResId(STR_STYLENAME_RESULT));
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

CPPUNIT_TEST_FIXTURE(TestCondformat, testConditionStyleInMergedCell)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    PaintListener aListener;
    aListener.StartListening(*m_xDocShell);

    m_pDoc->DoMerge(0, 0, 0, 1, 0); // A1:A2
    CPPUNIT_ASSERT(m_pDoc->IsMerged(ScAddress(0, 0, 0)));

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);

    // Add a conditional format.
    auto pFormat = std::make_unique<ScConditionalFormat>(1, m_pDoc);
    pFormat->SetRange(ScRange(0, 0, 0, 0, 0, 0));

    // Add condition in which if the value equals 1, set the "Good" style.
    ScCondFormatEntry* pEntry = new ScCondFormatEntry(
        ScConditionMode::Equal, u"=1"_ustr, u""_ustr, *m_pDoc, ScAddress(0, 0, 0), ScResId(STR_STYLENAME_GOOD));
    pFormat->AddEntry(pEntry);

    // Apply the format to the range.
    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);

    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    sal_uInt32 nOldFormat = pFormat->GetKey();
    const ScRangeList& rRangeList = pFormat->GetRange();
    rFunc.ReplaceConditionalFormat(nOldFormat, std::move(pFormat), 0, rRangeList);

    CPPUNIT_ASSERT_EQUAL(true, aListener.mbPaintAllMergedCell);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
