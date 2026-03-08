/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/MoveBlockOperation.hxx>

#include <osl/diagnose.h>
#include <sfx2/app.hxx>

#include <clipparam.hxx>
#include <docsh.hxx>
#include <docfunc.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <editable.hxx>
#include <globstr.hrc>
#include <markdata.hxx>
#include <undoblk.hxx>

namespace sc
{
MoveBlockOperation::MoveBlockOperation(ScDocShell& rDocShell, const ScRange& rSource,
                                       const ScAddress& rDestPos, bool bCut, bool bRecord,
                                       bool bPaint, bool bApi)
    : Operation(OperationType::MoveBlock, bRecord, bApi)
    , mrDocShell(rDocShell)
    , maSource(rSource)
    , maDestPos(rDestPos)
    , mbCut(bCut)
    , mbPaint(bPaint)
{
}

bool MoveBlockOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    SCCOL nStartCol = maSource.aStart.Col();
    SCROW nStartRow = maSource.aStart.Row();
    SCTAB nStartTab = maSource.aStart.Tab();
    SCCOL nEndCol = maSource.aEnd.Col();
    SCROW nEndRow = maSource.aEnd.Row();
    SCTAB nEndTab = maSource.aEnd.Tab();
    SCCOL nDestCol = maDestPos.Col();
    SCROW nDestRow = maDestPos.Row();
    SCTAB nDestTab = maDestPos.Tab();

    ScDocument& rDoc = mrDocShell.GetDocument();
    if (!rDoc.ValidRow(nStartRow) || !rDoc.ValidRow(nEndRow) || !rDoc.ValidRow(nDestRow))
    {
        OSL_FAIL("invalid row in MoveBlock");
        return false;
    }

    //  adjust related scenarios too - but only when moved within one sheet
    bool bScenariosAdded = false;
    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    SCTAB nTabCount = rDoc.GetTableCount();
    if (nDestTab == nStartTab && !rDoc.IsScenario(nEndTab))
        while (nEndTab + 1 < nTabCount && rDoc.IsScenario(nEndTab + 1))
        {
            ++nEndTab;
            bScenariosAdded = true;
        }

    SCTAB nSrcTabCount = nEndTab - nStartTab + 1;
    SCTAB nDestEndTab = nDestTab + nSrcTabCount - 1;
    SCTAB nTab;

    ScDocumentUniquePtr pClipDoc(new ScDocument(SCDOCMODE_CLIP));

    ScMarkData aSourceMark(rDoc.GetSheetLimits());
    for (nTab = nStartTab; nTab <= nEndTab; nTab++)
        aSourceMark.SelectTable(nTab, true); // select source
    aSourceMark.SetMarkArea(maSource);

    ScDocShellRef aDragShellRef;
    if (rDoc.HasOLEObjectsInArea(maSource))
    {
        aDragShellRef = new ScDocShell; // DocShell needs a Ref immediately
        aDragShellRef->DoInitNew();
    }
    ScDrawLayer::SetGlobalDrawPersist(aDragShellRef.get());

    ScClipParam aClipParam(ScRange(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nStartTab),
                           mbCut);
    rDoc.CopyToClip(aClipParam, pClipDoc.get(), &aSourceMark, bScenariosAdded, true);

    ScDrawLayer::SetGlobalDrawPersist(nullptr);

    SCCOL nOldEndCol = nEndCol;
    SCROW nOldEndRow = nEndRow;
    bool bClipOver = false;
    for (nTab = nStartTab; nTab <= nEndTab; nTab++)
    {
        SCCOL nTmpEndCol = nOldEndCol;
        SCROW nTmpEndRow = nOldEndRow;
        if (rDoc.ExtendMerge(nStartCol, nStartRow, nTmpEndCol, nTmpEndRow, nTab))
            bClipOver = true;
        if (nTmpEndCol > nEndCol)
            nEndCol = nTmpEndCol;
        if (nTmpEndRow > nEndRow)
            nEndRow = nTmpEndRow;
    }

    SCCOL nDestEndCol = nDestCol + (nOldEndCol - nStartCol);
    SCROW nDestEndRow = nDestRow + (nOldEndRow - nStartRow);

    SCCOL nUndoEndCol = nDestCol + (nEndCol - nStartCol); // extended in destination block
    SCROW nUndoEndRow = nDestRow + (nEndRow - nStartRow);

    bool bIncludeFiltered = mbCut;
    if (!bIncludeFiltered)
    {
        //  adjust sizes to include only non-filtered rows

        SCCOL nClipX;
        SCROW nClipY;
        pClipDoc->GetClipArea(nClipX, nClipY, false);
        SCROW nUndoAdd = nUndoEndRow - nDestEndRow;
        nDestEndRow = nDestRow + nClipY;
        nUndoEndRow = nDestEndRow + nUndoAdd;
    }

    if (!rDoc.ValidCol(nUndoEndCol) || !rDoc.ValidRow(nUndoEndRow))
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_PASTE_FULL);
        return false;
    }

    if (!checkSheetViewProtection())
        return false;

    //  Test for cell protection

    ScEditableTester aTester;
    for (nTab = nDestTab; nTab <= nDestEndTab; nTab++)
        aTester.TestBlock(rDoc, nTab, nDestCol, nDestRow, nUndoEndCol, nUndoEndRow);
    if (mbCut)
        for (nTab = nStartTab; nTab <= nEndTab; nTab++)
            aTester.TestBlock(rDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow);

    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    //  Test for merged cells- when moving after delete

    if (bClipOver && !mbCut)
        if (rDoc.HasAttrib(nDestCol, nDestRow, nDestTab, nUndoEndCol, nUndoEndRow, nDestEndTab,
                           HasAttrFlags::Merged | HasAttrFlags::Overlapped))
        { // "Merge of already merged cells not possible"
            if (!mbApi)
                mrDocShell.ErrorMessage(STR_MSSG_MOVEBLOCKTO_0);
            return false;
        }

    //  Are there borders in the cells? (for painting)

    sal_uInt16 nSourceExt = 0;
    mrDocShell.UpdatePaintExt(nSourceExt, nStartCol, nStartRow, nStartTab, nEndCol, nEndRow,
                              nEndTab);
    sal_uInt16 nDestExt = 0;
    mrDocShell.UpdatePaintExt(nDestExt, nDestCol, nDestRow, nDestTab, nDestEndCol, nDestEndRow,
                              nDestEndTab);

    //  do it

    ScDocumentUniquePtr pUndoDoc;

    if (mbRecord)
    {
        bool bWholeCols = (nStartRow == 0 && nEndRow == rDoc.MaxRow());
        bool bWholeRows = (nStartCol == 0 && nEndCol == rDoc.MaxCol());
        InsertDeleteFlags nUndoFlags = (InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS)
                                       | InsertDeleteFlags::NOCAPTIONS;

        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo(rDoc, nStartTab, nEndTab, bWholeCols, bWholeRows);

        if (mbCut)
        {
            rDoc.CopyToDocument(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                                nUndoFlags, false, *pUndoDoc);
        }

        if (nDestTab != nStartTab)
            pUndoDoc->AddUndoTab(nDestTab, nDestEndTab, bWholeCols, bWholeRows);
        rDoc.CopyToDocument(nDestCol, nDestRow, nDestTab, nDestEndCol, nDestEndRow, nDestEndTab,
                            nUndoFlags, false, *pUndoDoc);
        rDoc.BeginDrawUndo();
    }

    bool bSourceHeight = false; // adjust heights?
    if (mbCut)
    {
        ScMarkData aDelMark(rDoc.GetSheetLimits()); // only for tables
        for (nTab = nStartTab; nTab <= nEndTab; nTab++)
        {
            rDoc.DeleteAreaTab(nStartCol, nStartRow, nOldEndCol, nOldEndRow, nTab,
                               InsertDeleteFlags::ALL);
            aDelMark.SelectTable(nTab, true);
        }
        rDoc.DeleteObjectsInArea(nStartCol, nStartRow, nOldEndCol, nOldEndRow, aDelMark);

        //  Test for merged cells

        if (bClipOver)
            if (rDoc.HasAttrib(nDestCol, nDestRow, nDestTab, nUndoEndCol, nUndoEndRow, nDestEndTab,
                               HasAttrFlags::Merged | HasAttrFlags::Overlapped))
            {
                rDoc.CopyFromClip(maSource, aSourceMark, InsertDeleteFlags::ALL, nullptr,
                                  pClipDoc.get());
                for (nTab = nStartTab; nTab <= nEndTab; nTab++)
                {
                    SCCOL nTmpEndCol = nEndCol;
                    SCROW nTmpEndRow = nEndRow;
                    rDoc.ExtendMerge(nStartCol, nStartRow, nTmpEndCol, nTmpEndRow, nTab, true);
                }

                // Report error only after restoring content
                if (!mbApi) // "Merge of already merged cells not possible"
                    mrDocShell.ErrorMessage(STR_MSSG_MOVEBLOCKTO_0);

                return false;
            }

        bSourceHeight = mrDocShell.GetDocFunc().AdjustRowHeight(maSource, false, mbApi);
    }

    ScRange aPasteDest(nDestCol, nDestRow, nDestTab, nDestEndCol, nDestEndRow, nDestEndTab);

    ScMarkData aDestMark(rDoc.GetSheetLimits());
    for (nTab = nDestTab; nTab <= nDestEndTab; nTab++)
        aDestMark.SelectTable(nTab, true); // select destination
    aDestMark.SetMarkArea(aPasteDest);

    /*  Do not copy drawing objects here. While pasting, the
        function ScDocument::UpdateReference() is called which calls
        ScDrawLayer::MoveCells() which may move away inserted objects to wrong
        positions (e.g. if source and destination range overlaps).*/

    rDoc.CopyFromClip(aPasteDest, aDestMark, InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS,
                      pUndoDoc.get(), pClipDoc.get(), true, false, bIncludeFiltered);

    // skipped rows and merged cells don't mix
    if (!bIncludeFiltered && pClipDoc->HasClipFilteredRows())
        mrDocShell.GetDocFunc().UnmergeCells(aPasteDest, false, nullptr);

    bool bDestHeight = mrDocShell.GetDocFunc().AdjustRowHeight(
        ScRange(0, nDestRow, nDestTab, rDoc.MaxCol(), nDestEndRow, nDestEndTab), false, mbApi);

    /*  Paste drawing objects after adjusting formula references
        and row heights. There are no cell notes or drawing objects, if the
        clipdoc does not contain a drawing layer.*/
    if (pClipDoc->GetDrawLayer())
        rDoc.CopyFromClip(aPasteDest, aDestMark, InsertDeleteFlags::OBJECTS, nullptr,
                          pClipDoc.get(), true, false, bIncludeFiltered);

    if (mbRecord)
    {
        ScRange aUndoRange(nStartCol, nStartRow, nStartTab, nOldEndCol, nOldEndRow, nEndTab);
        ScAddress aDestPosUndo(nDestCol, nDestRow, nDestTab);

        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoDragDrop>(
            &mrDocShell, aUndoRange, aDestPosUndo, mbCut, std::move(pUndoDoc), bScenariosAdded));
    }

    SCCOL nDestPaintEndCol = nDestEndCol;
    SCROW nDestPaintEndRow = nDestEndRow;
    for (nTab = nDestTab; nTab <= nDestEndTab; nTab++)
    {
        SCCOL nTmpEndCol = nDestEndCol;
        SCROW nTmpEndRow = nDestEndRow;
        rDoc.ExtendMerge(nDestCol, nDestRow, nTmpEndCol, nTmpEndRow, nTab, true);
        if (nTmpEndCol > nDestPaintEndCol)
            nDestPaintEndCol = nTmpEndCol;
        if (nTmpEndRow > nDestPaintEndRow)
            nDestPaintEndRow = nTmpEndRow;
    }

    if (mbCut)
        for (nTab = nStartTab; nTab <= nEndTab; nTab++)
            rDoc.RefreshAutoFilter(nStartCol, nStartRow, nEndCol, nEndRow, nTab);

    if (mbPaint)
    {
        //  destination range:

        SCCOL nPaintStartX = nDestCol;
        SCROW nPaintStartY = nDestRow;
        SCCOL nPaintEndX = nDestPaintEndCol;
        SCROW nPaintEndY = nDestPaintEndRow;
        PaintPartFlags nFlags = PaintPartFlags::Grid;

        if (nStartRow == 0 && nEndRow == rDoc.MaxRow()) // copy widths too?
        {
            nPaintEndX = rDoc.MaxCol();
            nPaintStartY = 0;
            nPaintEndY = rDoc.MaxRow();
            nFlags |= PaintPartFlags::Top;
        }
        if (bDestHeight || (nStartCol == 0 && nEndCol == rDoc.MaxCol()))
        {
            nPaintEndY = rDoc.MaxRow();
            nPaintStartX = 0;
            nPaintEndX = rDoc.MaxCol();
            nFlags |= PaintPartFlags::Left;
        }
        if (bScenariosAdded)
        {
            nPaintStartX = 0;
            nPaintStartY = 0;
            nPaintEndX = rDoc.MaxCol();
            nPaintEndY = rDoc.MaxRow();
        }

        mrDocShell.PostPaint(nPaintStartX, nPaintStartY, nDestTab, nPaintEndX, nPaintEndY,
                             nDestEndTab, nFlags, nSourceExt | nDestExt);

        if (mbCut)
        {
            //  source range:

            nPaintStartX = nStartCol;
            nPaintStartY = nStartRow;
            nPaintEndX = nEndCol;
            nPaintEndY = nEndRow;
            nFlags = PaintPartFlags::Grid;

            if (bSourceHeight)
            {
                nPaintEndY = rDoc.MaxRow();
                nPaintStartX = 0;
                nPaintEndX = rDoc.MaxCol();
                nFlags |= PaintPartFlags::Left;
            }
            if (bScenariosAdded)
            {
                nPaintStartX = 0;
                nPaintStartY = 0;
                nPaintEndX = rDoc.MaxCol();
                nPaintEndY = rDoc.MaxRow();
            }

            mrDocShell.PostPaint(nPaintStartX, nPaintStartY, nStartTab, nPaintEndX, nPaintEndY,
                                 nEndTab, nFlags, nSourceExt);
        }
    }

    aModificator.SetDocumentModified();

    SfxGetpApp()->Broadcast(SfxHint(SfxHintId::ScAreaLinksChanged));

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
