/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ApplyAttributesOperation.hxx>

#include <docfuncutil.hxx>
#include <docfunc.hxx>
#include <address.hxx>
#include <editable.hxx>
#include <undoblk.hxx>
#include <undocell.hxx>
#include <validat.hxx>
#include <globstr.hrc>
#include <cellvalue.hxx>
#include <editeng/editobj.hxx>

#include <memory>

namespace sc
{
ApplyAttributesOperation::ApplyAttributesOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                                   const ScMarkData& rMark,
                                                   const ScPatternAttr& rPattern, bool bApi)
    : Operation(OperationType::ApplyAttributes, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrMark(rMark)
    , mrPattern(rPattern)
{
}

bool ApplyAttributesOperation::runImplementation()
{
    ScDocument& rDoc = mrDocShell.GetDocument();
    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark = mrMark;

    bool bImportingXML = rDoc.IsImportingXML();
    bool bImportingXLSX = rDoc.IsImportingXLSX();
    // Cell formats can still be set if the range isn't editable only because of matrix formulas.
    // #i62483# When loading XML, the check can be skipped altogether.
    bool bOnlyNotBecauseOfMatrix;
    if (!bImportingXML && !rDoc.IsSelectionEditable(aMark, &bOnlyNotBecauseOfMatrix)
        && !bOnlyNotBecauseOfMatrix)
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_PROTECTIONERR);
        return false;
    }

    ScDocShellModificator aModificator(mrDocShell);

    //! Border

    ScRange aMultiRange;
    bool bMulti = aMark.IsMultiMarked();
    if (bMulti)
        aMultiRange = aMark.GetMultiMarkArea();
    else
        aMultiRange = aMark.GetMarkArea();

    if (bRecord)
    {
        ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo(rDoc, aMultiRange.aStart.Tab(), aMultiRange.aEnd.Tab());
        rDoc.CopyToDocument(aMultiRange, InsertDeleteFlags::ATTRIB, bMulti, *pUndoDoc, &aMark);

        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoSelectionAttr>(
            &mrDocShell, aMark, aMultiRange.aStart.Col(), aMultiRange.aStart.Row(),
            aMultiRange.aStart.Tab(), aMultiRange.aEnd.Col(), aMultiRange.aEnd.Row(),
            aMultiRange.aEnd.Tab(), std::move(pUndoDoc), bMulti, &mrPattern));
    }

    // While loading XML it is not necessary to ask HasAttrib. It needs too much time.
    sal_uInt16 nExtFlags = 0;
    if (!bImportingXML && !bImportingXLSX)
        mrDocShell.UpdatePaintExt(nExtFlags, aMultiRange); // content before the change

    bool bChanged = false;
    rDoc.ApplySelectionPattern(mrPattern, aMark, nullptr, &bChanged);

    if (bChanged)
    {
        if (!bImportingXML && !bImportingXLSX)
            mrDocShell.UpdatePaintExt(nExtFlags, aMultiRange); // content after the change

        if (!mrDocFunc.AdjustRowHeight(aMultiRange, true, mbApi))
            mrDocShell.PostPaint(aMultiRange, PaintPartFlags::Grid, nExtFlags);
        else if (nExtFlags & SC_PF_LINES)
            ScDocFunc::PaintAbove(mrDocShell, aMultiRange); // because of lines above the range

        aModificator.SetDocumentModified();
    }

    return true;
}

ApplyAttributesWithChangedRangeOperation::ApplyAttributesWithChangedRangeOperation(
    ScDocShell& rDocShell, const ScMarkData& rMark, bool bMultiMarked,
    const ScPatternAttr& rPattern, sal_uInt16 nExtFlags, bool bApi)
    : Operation(OperationType::ApplyAttributes, true, bApi)
    , mrDocShell(rDocShell)
    , mrMark(rMark)
    , mrPattern(rPattern)
    , mbMultiMarked(bMultiMarked)
    , mnExtFlags(nExtFlags)
{
}

bool ApplyAttributesWithChangedRangeOperation::runImplementation()
{
    ScDocument& rDoc = mrDocShell.GetDocument();

    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return false;

    bool bRecord = mbRecord;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark = mrMark;

    ScDocShellModificator aModificator(mrDocShell);

    const ScRange& aMarkRange = aMark.GetMultiMarkArea();
    SCTAB nTabCount = rDoc.GetTableCount();
    for (const auto& rTab : aMark)
    {
        ScRange aChangeRange(aMarkRange);
        aChangeRange.aStart.SetTab(rTab);
        aChangeRange.aEnd.SetTab(rTab);
        maChangeRanges.push_back(aChangeRange);
    }

    SCCOL nStartCol = aMarkRange.aStart.Col();
    SCROW nStartRow = aMarkRange.aStart.Row();
    SCTAB nStartTab = aMarkRange.aStart.Tab();
    SCCOL nEndCol = aMarkRange.aEnd.Col();
    SCROW nEndRow = aMarkRange.aEnd.Row();
    SCTAB nEndTab = aMarkRange.aEnd.Tab();

    ScEditDataArray* pEditDataArray = nullptr;
    if (bRecord)
    {
        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount - 1);

        ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo(rDoc, nStartTab, nStartTab);
        for (const auto& rTab : aMark)
            if (rTab != nStartTab)
                pUndoDoc->AddUndoTab(rTab, rTab);
        rDoc.CopyToDocument(aCopyRange, InsertDeleteFlags::ATTRIB, mbMultiMarked, *pUndoDoc,
                            &aMark);

        aMark.MarkToMulti();

        ScUndoSelectionAttr* pUndoAttr = new ScUndoSelectionAttr(
            &mrDocShell, aMark, nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
            std::move(pUndoDoc), mbMultiMarked, &mrPattern);
        mrDocShell.GetUndoManager()->AddUndoAction(std::unique_ptr<ScUndoSelectionAttr>(pUndoAttr));
        pEditDataArray = pUndoAttr->GetDataArray();
    }

    rDoc.ApplySelectionPattern(mrPattern, aMark, pEditDataArray);

    mrDocShell.PostPaint(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                         PaintPartFlags::Grid, mnExtFlags | SC_PF_TESTMERGE);
    mrDocShell.UpdateOle(*pViewData);

    aModificator.SetDocumentModified();

    return true;
}

ApplyAttributesToCellOperation::ApplyAttributesToCellOperation(ScDocShell& rDocShell,
                                                               ScAddress const& rPosition,
                                                               const ScPatternAttr& rPattern,
                                                               sal_uInt16 nExtFlags, bool bApi)
    : Operation(OperationType::ApplyAttributes, true, bApi)
    , mrDocShell(rDocShell)
    , mrPosition(rPosition)
    , mrPattern(rPattern)
    , mnExtFlags(nExtFlags)
{
}
bool ApplyAttributesToCellOperation::runImplementation()
{
    ScDocument& rDoc = mrDocShell.GetDocument();

    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return false;

    bool bRecord = mbRecord;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    ScAddress aPosition = mrPosition;

    ScDocShellModificator aModificator(mrDocShell);

    SCCOL nCol = aPosition.Col();
    SCROW nRow = aPosition.Row();
    SCTAB nTab = aPosition.Tab();

    std::unique_ptr<EditTextObject> pOldEditData;
    std::unique_ptr<EditTextObject> pNewEditData;

    ScRefCellValue aCell(rDoc, aPosition);
    if (aCell.getType() == CELLTYPE_EDIT)
    {
        const EditTextObject* pEditObj = aCell.getEditText();
        pOldEditData = pEditObj->Clone();
        rDoc.RemoveEditTextCharAttribs(aPosition, mrPattern);
        pEditObj = rDoc.GetEditText(aPosition);
        pNewEditData = pEditObj->Clone();
    }

    maChangeRanges.push_back(ScRange(aPosition));
    std::optional<ScPatternAttr> pOldPat(*rDoc.GetPattern(nCol, nRow, nTab));

    rDoc.ApplyPattern(nCol, nRow, nTab, mrPattern);

    const ScPatternAttr* pNewPat = rDoc.GetPattern(nCol, nRow, nTab);

    if (bRecord)
    {
        std::unique_ptr<ScUndoCursorAttr> pUndo(
            new ScUndoCursorAttr(&mrDocShell, nCol, nRow, nTab, &*pOldPat, pNewPat, &mrPattern));
        pUndo->SetEditData(std::move(pOldEditData), std::move(pNewEditData));
        mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndo));
    }
    pOldPat.reset(); // is copied in undo (Pool)

    mrDocShell.PostPaint(nCol, nRow, nTab, nCol, nRow, nTab, PaintPartFlags::Grid,
                         mnExtFlags | SC_PF_TESTMERGE);
    mrDocShell.UpdateOle(*pViewData);
    aModificator.SetDocumentModified();

    return true;
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
