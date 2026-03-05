/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cliputil.hxx>
#include <attrib.hxx>
#include <viewdata.hxx>
#include <tabvwsh.hxx>
#include <transobj.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <docfunc.hxx>
#include <dpobject.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <clipparam.hxx>
#include <clipoptions.hxx>
#include <rangelst.hxx>
#include <viewutil.hxx>
#include <markdata.hxx>
#include <gridwin.hxx>
#include <scitems.hxx>
#include <formulacell.hxx>

#include <sfx2/classificationhelper.hxx>
#include <comphelper/lok.hxx>
#include <inputopt.hxx>
#include <scmod.hxx>
#include <warnbox.hxx>

namespace
{

/// Paste only if SfxClassificationHelper recommends so.
bool lcl_checkClassification(ScDocument* pSourceDoc, const ScDocument& rDestinationDoc)
{
    if (!pSourceDoc)
        return true;

    ScClipOptions* pSourceOptions = pSourceDoc->GetClipOptions();
    ScDocShell* pDestinationShell = rDestinationDoc.GetDocumentShell();
    if (!pSourceOptions || !pDestinationShell)
        return true;

    SfxClassificationCheckPasteResult eResult = SfxClassificationHelper::CheckPaste(pSourceOptions->m_xDocumentProperties, pDestinationShell->getDocProperties());
    return SfxClassificationHelper::ShowPasteInfo(eResult);
}

}

void ScClipUtil::PasteFromClipboard( ScViewData& rViewData, ScTabViewShell* pTabViewShell, bool bShowDialog )
{
    const ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard(ScTabViewShell::GetClipData(rViewData.GetActiveWin()));
    ScDocument& rThisDoc = rViewData.GetDocument();
    SCCOL nThisCol = rViewData.GetCurX();
    SCROW nThisRow = rViewData.GetCurY();
    SCTAB nThisTab = rViewData.CurrentTabForData();
    ScDPObject* pDPObj = rThisDoc.GetDPAtCursor( nThisCol, nThisRow, nThisTab );

    if ( pOwnClip && pDPObj )
    {
        // paste from Calc into DataPilot table: sort (similar to drag & drop)

        ScDocument* pClipDoc = pOwnClip->GetDocument();
        SCTAB nSourceTab = pOwnClip->GetVisibleTab();

        SCCOL nClipStartX;
        SCROW nClipStartY;
        SCCOL nClipEndX;
        SCROW nClipEndY;
        pClipDoc->GetClipStart( nClipStartX, nClipStartY );
        pClipDoc->GetClipArea( nClipEndX, nClipEndY, true );
        nClipEndX = nClipEndX + nClipStartX;
        nClipEndY = nClipEndY + nClipStartY;   // GetClipArea returns the difference

        ScRange aSource( nClipStartX, nClipStartY, nSourceTab, nClipEndX, nClipEndY, nSourceTab );
        bool bDone = pTabViewShell->DataPilotMove( aSource, rViewData.GetCurPos() );
        if ( !bDone )
            pTabViewShell->ErrorMessage( STR_ERR_DATAPILOT_INPUT );
    }
    else
    {
        // normal paste
        weld::WaitObject aWait( rViewData.GetDialogParent() );
        if (!pOwnClip)
        {
            pTabViewShell->PasteFromSystem(true);
            // Anchor To Cell rather than To Page
            ScDrawView* pDrawView = pTabViewShell->GetScDrawView();
            if(pDrawView)
            {
                const SdrMarkList& rMarkList = pDrawView->GetMarkedObjectList();
                if (1 == rMarkList.GetMarkCount())
                {
                    SdrObject* pPickObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                    if(pPickObj)
                    {
                        ScDrawLayer::SetCellAnchoredFromPosition( *pPickObj,  rThisDoc, nThisTab, false );
                    }
                }
            }
        }
        else
        {
            ScDocument* pClipDoc = pOwnClip->GetDocument();
            const ScClipParam& rClipParam = pClipDoc->GetClipParam();

            if (!lcl_checkClassification(pClipDoc, rThisDoc))
                return;

            InsertDeleteFlags nFlags = InsertDeleteFlags::ALL;
            // For multi-range paste, we paste values by default.
            if (rClipParam.isMultiRange())
                nFlags &= ~InsertDeleteFlags::FORMULA;

            // --- Origin-matrix handling ---
            // The clip doc contains the expanded bounding box (original +
            // origin matrices).  Paste the original selection (non-matrix
            // cells only), then let EnterMatrix expand each matrix origin
            // to its full array range.
            if (!rClipParam.maOriginMatrixRanges.empty())
            {
                const ScRange& rOrigRange = rClipParam.maOriginalRange;
                SCTAB nClipTab = rOrigRange.aStart.Tab();

                // Pre-compute destination ranges and grab token arrays
                // from the clip doc (before any paste modifies anything).
                struct OriginMatrixInfo
                {
                    ScRange maDestRange;   // destination range for the full matrix
                    const ScTokenArray* mpCode;  // token array from the origin formula cell
                };
                std::vector<OriginMatrixInfo> aOriginInfos;
                aOriginInfos.reserve(rClipParam.maOriginMatrixRanges.size());

                formula::FormulaGrammar::Grammar eGram = rThisDoc.GetGrammar();
                for (size_t i = 0; i < rClipParam.maOriginMatrixRanges.size(); ++i)
                {
                    const ScRange& rMat = rClipParam.maOriginMatrixRanges[i];
                    SCCOL nDestCol = nThisCol + (rMat.aStart.Col() - rOrigRange.aStart.Col());
                    SCROW nDestRow = nThisRow + (rMat.aStart.Row() - rOrigRange.aStart.Row());
                    ScRange aDestMatRange(nDestCol, nDestRow, nThisTab,
                            nDestCol + rMat.aEnd.Col() - rMat.aStart.Col(),
                            nDestRow + rMat.aEnd.Row() - rMat.aStart.Row(), nThisTab);

                    // Get the token array from the clip doc's origin cell.
                    // Passing the token array (instead of a formula string)
                    // to EnterMatrix ensures relative references are correctly
                    // shifted to the destination position.
                    const ScTokenArray* pCode = nullptr;
                    ScFormulaCell* pFC = pClipDoc->GetFormulaCell(rMat.aStart);
                    if (pFC)
                        pCode = pFC->GetCode();

                    aOriginInfos.push_back({ aDestMatRange, pCode });
                }

                // Pre-check: detect existing matrices at the destination.
                // If fully covered by one of our expanded ranges, queue
                // for clearing.  If only partially covered, abort.
                ScRangeList aMatricesToClear;
                for (const auto& rInfo : aOriginInfos)
                {
                    const ScRange& rDest = rInfo.maDestRange;
                    for (SCROW r = rDest.aStart.Row(); r <= rDest.aEnd.Row(); ++r)
                    {
                        for (SCCOL c = rDest.aStart.Col(); c <= rDest.aEnd.Col(); ++c)
                        {
                            ScRange aExisting;
                            if (rThisDoc.GetMatrixFormulaRange(ScAddress(c, r, nThisTab), aExisting))
                            {
                                if (rDest.Contains(aExisting))
                                {
                                    if (!aMatricesToClear.Contains(aExisting))
                                        aMatricesToClear.push_back(aExisting);
                                }
                                else
                                {
                                    pTabViewShell->ErrorMessage(STR_MATRIXFRAGMENTERR);
                                    return;
                                }
                            }
                        }
                    }
                }

                // Check if the paste area would overwrite non-empty cells.
                if (bShowDialog
                    && (nFlags & InsertDeleteFlags::CONTENTS)
                    && ScModule::get()->GetInputOptions().GetReplaceCellsWarn())
                {
                    bool bIsEmpty = true;
                    SCCOL nOrigEndCol = nThisCol + (rOrigRange.aEnd.Col() - rOrigRange.aStart.Col());
                    SCROW nOrigEndRow = nThisRow + (rOrigRange.aEnd.Row() - rOrigRange.aStart.Row());

                    // Check the original selection area.
                    bIsEmpty = rThisDoc.IsBlockEmpty(
                            nThisCol, nThisRow, nOrigEndCol, nOrigEndRow, nThisTab);

                    // Check the expanded matrix areas (beyond original selection).
                    for (size_t i = 0; i < aOriginInfos.size() && bIsEmpty; ++i)
                    {
                        const ScRange& rDest = aOriginInfos[i].maDestRange;
                        if (rDest.aEnd.Col() > nOrigEndCol)
                        {
                            SCCOL nCheckStart = std::max(rDest.aStart.Col(), static_cast<SCCOL>(nOrigEndCol + 1));
                            bIsEmpty = rThisDoc.IsBlockEmpty(
                                    nCheckStart, rDest.aStart.Row(),
                                    rDest.aEnd.Col(), rDest.aEnd.Row(), nThisTab);
                        }
                        if (bIsEmpty && rDest.aEnd.Row() > nOrigEndRow)
                        {
                            SCROW nCheckStart = std::max(rDest.aStart.Row(), static_cast<SCROW>(nOrigEndRow + 1));
                            SCCOL nColEnd = std::min(rDest.aEnd.Col(), nOrigEndCol);
                            bIsEmpty = rThisDoc.IsBlockEmpty(
                                    rDest.aStart.Col(), nCheckStart,
                                    nColEnd, rDest.aEnd.Row(), nThisTab);
                        }
                    }

                    if (!bIsEmpty)
                    {
                        ScReplaceWarnBox aBox(rViewData.GetDialogParent());
                        if (aBox.run() != RET_YES)
                            return;
                    }
                }

                // --- All checks passed.  Now modify. ---
                ScDocShell* pDocSh = rViewData.GetDocShell();
                SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();
                OUString aUndo = ScResId(pClipDoc->IsCutMode() ? STR_UNDO_MOVE : STR_UNDO_COPY);
                pUndoMgr->EnterListAction(aUndo, aUndo, 0,
                        rViewData.GetViewShell()->GetViewShellId());

                // Clear existing complete matrices that will be replaced.
                for (size_t i = 0; i < aMatricesToClear.size(); ++i)
                {
                    const ScRange& rClear = aMatricesToClear[i];
                    ScMarkData aClearMark(rThisDoc.GetSheetLimits());
                    aClearMark.SelectTable(nThisTab, true);
                    aClearMark.SetMarkArea(rClear);
                    pDocSh->GetDocFunc().DeleteContents(
                            aClearMark, InsertDeleteFlags::ALL, true /*bRecord*/, true /*bApi*/);
                }

                // Check whether the original selection contains any
                // non-matrix cells.  If tracked matrices fully cover it
                // we can skip the expensive CopyToDocument + PasteFromClip
                // round-trip — EnterMatrix alone handles the paste.
                sal_uLong nOrigCells
                    = static_cast<sal_uLong>(rOrigRange.aEnd.Col() - rOrigRange.aStart.Col() + 1)
                      * (rOrigRange.aEnd.Row() - rOrigRange.aStart.Row() + 1);
                sal_uLong nMatrixCells = 0;
                auto countMatrixCells = [&](const ScRangeList& rList) {
                    for (size_t i = 0; i < rList.size(); ++i)
                    {
                        ScRange aInt = rOrigRange.Intersection(rList[i]);
                        if (aInt.IsValid())
                            nMatrixCells
                                += static_cast<sal_uLong>(aInt.aEnd.Col() - aInt.aStart.Col() + 1)
                                   * (aInt.aEnd.Row() - aInt.aStart.Row() + 1);
                    }
                };
                countMatrixCells(rClipParam.maOriginMatrixRanges);
                countMatrixCells(rClipParam.maMatrixRanges);

                if (nMatrixCells < nOrigCells)
                {
                    // Build a temp clip doc with the original selection
                    // and clear tracked matrix cells so that only
                    // non-matrix cells remain for pasting.
                    auto pOrigDoc = std::make_shared<ScDocument>(SCDOCMODE_CLIP);
                    pOrigDoc->ResetClip(pClipDoc, nClipTab);
                    pClipDoc->CopyToDocument(rOrigRange, InsertDeleteFlags::ALL, false, *pOrigDoc);

                    auto clearTrackedMatrixCells = [&](const ScRangeList& rList) {
                        for (size_t i = 0; i < rList.size(); ++i)
                        {
                            ScRange aInt = rOrigRange.Intersection(rList[i]);
                            if (aInt.IsValid())
                                pOrigDoc->DeleteAreaTab(aInt.aStart.Col(), aInt.aStart.Row(),
                                                        aInt.aEnd.Col(), aInt.aEnd.Row(),
                                                        nClipTab, InsertDeleteFlags::ALL);
                        }
                    };
                    clearTrackedMatrixCells(rClipParam.maOriginMatrixRanges);
                    clearTrackedMatrixCells(rClipParam.maMatrixRanges);

                    ScClipParam aOrigParam(rOrigRange, false);
                    pOrigDoc->SetClipParam(aOrigParam);

                    pTabViewShell->PasteFromClip(nFlags, pOrigDoc.get(),
                            ScPasteFunc::NONE, true /*bSkipEmpty*/, false, false,
                            INS_NONE, InsertDeleteFlags::NONE, false /*bShowDialog*/);
                }

                // Expand each origin matrix using EnterMatrix.
                for (const auto& rInfo : aOriginInfos)
                {
                    if (!rInfo.mpCode)
                        continue;

                    const ScRange& rDest = rInfo.maDestRange;
                    ScMarkData aMark(rThisDoc.GetSheetLimits());
                    aMark.SelectTable(nThisTab, true);

                    pDocSh->GetDocFunc().EnterMatrix(
                            rDest, &aMark, rInfo.mpCode, OUString(),
                            true /*bApi*/, false /*bEnglish*/,
                            OUString(), eGram);
                }

                pUndoMgr->LeaveListAction();

                // Restore cursor and selection to the original paste range.
                SCROW nOrigRows = rOrigRange.aEnd.Row() - rOrigRange.aStart.Row();
                SCCOL nOrigCols = rOrigRange.aEnd.Col() - rOrigRange.aStart.Col();
                ScRange aDestRange(nThisCol, nThisRow, nThisTab,
                                   nThisCol + nOrigCols, nThisRow + nOrigRows, nThisTab);
                rViewData.GetMarkData().ResetMark();
                pTabViewShell->SetCursor(nThisCol, nThisRow);
                rViewData.GetMarkData().SetMarkArea(aDestRange);
                pTabViewShell->MarkRange(aDestRange);
            }
            // --- Non-origin matrix handling ---
            else if (!rClipParam.maMatrixRanges.empty()
                     && rClipParam.maRanges.size() == 1
                     && rClipParam.maRanges[0] == rClipParam.maOriginalRange)
            {
                ScRange aClipRange = rClipParam.getWholeRange();
                SCTAB nClipTab = aClipRange.aStart.Tab();

                // If non-origin matrices fully cover the selection,
                // there are no non-matrix cells to paste — nothing to do.
                sal_uLong nClipCells
                    = static_cast<sal_uLong>(aClipRange.aEnd.Col() - aClipRange.aStart.Col() + 1)
                      * (aClipRange.aEnd.Row() - aClipRange.aStart.Row() + 1);
                sal_uLong nMatCells = 0;
                for (size_t i = 0; i < rClipParam.maMatrixRanges.size(); ++i)
                {
                    ScRange aInt = aClipRange.Intersection(rClipParam.maMatrixRanges[i]);
                    if (aInt.IsValid())
                        nMatCells
                            += static_cast<sal_uLong>(aInt.aEnd.Col() - aInt.aStart.Col() + 1)
                               * (aInt.aEnd.Row() - aInt.aStart.Row() + 1);
                }
                if (nMatCells >= nClipCells)
                    return;

                auto pTempDoc = std::make_shared<ScDocument>(SCDOCMODE_CLIP);
                pTempDoc->ResetClip(pClipDoc, nClipTab);
                pClipDoc->CopyToDocument(aClipRange, InsertDeleteFlags::ALL, false, *pTempDoc);

                for (size_t i = 0; i < rClipParam.maMatrixRanges.size(); ++i)
                {
                    ScRange aClearRange = aClipRange.Intersection(rClipParam.maMatrixRanges[i]);
                    if (aClearRange.IsValid())
                        pTempDoc->DeleteAreaTab(aClearRange.aStart.Col(), aClearRange.aStart.Row(),
                                                aClearRange.aEnd.Col(), aClearRange.aEnd.Row(),
                                                nClipTab, InsertDeleteFlags::ALL);
                }

                ScClipParam aTempParam(rClipParam);
                pTempDoc->SetClipParam(aTempParam);

                pTabViewShell->PasteFromClip( nFlags, pTempDoc.get(),
                        ScPasteFunc::NONE, true /*bSkipEmpty*/, false, false,
                        INS_NONE, InsertDeleteFlags::NONE,
                        bShowDialog );
            }
            else
            {
                // Normal paste: no matrix issues.
                pTabViewShell->PasteFromClip( nFlags, pClipDoc,
                        ScPasteFunc::NONE, false, false, false, INS_NONE, InsertDeleteFlags::NONE,
                        bShowDialog );
            }
        }
    }
    if (comphelper::LibreOfficeKit::isActive())
    {
        bool entireColumnOrRowSelected = false;
        if (pOwnClip)
        {
            ScClipParam clipParam = pOwnClip->GetDocument()->GetClipParam();
            if (clipParam.maRanges.size() > 0)
            {
                if (clipParam.maRanges[0].aEnd.Col() == pOwnClip->GetDocument()->MaxCol()
                    || clipParam.maRanges[0].aEnd.Row() == pOwnClip->GetDocument()->MaxRow())
                {
                    entireColumnOrRowSelected = true;
                }
            }
        }
        const SfxBoolItem& rItem = rThisDoc.GetAttr(nThisCol, nThisRow, nThisTab, ATTR_LINEBREAK);
        if (rItem.GetValue() || entireColumnOrRowSelected)
        {
            pTabViewShell->OnLOKSetWidthOrHeight(nThisCol, true);
            pTabViewShell->OnLOKSetWidthOrHeight(nThisRow, false);

            ScTabViewShell::notifyAllViewsSheetGeomInvalidation(
                pTabViewShell, true /* bColumns */, true /* bRows */, true /* bSizes*/,
                true /* bHidden */, true /* bFiltered */, true /* bGroups */, rViewData.GetTabNumber());
        }
    }
    pTabViewShell->CellContentChanged();        // => PasteFromSystem() ???
}

bool ScClipUtil::CheckDestRanges(
    const ScDocument& rDoc, SCCOL nSrcCols, SCROW nSrcRows, const ScMarkData& rMark, const ScRangeList& rDest)
{
    for (size_t i = 0, n = rDest.size(); i < n; ++i)
    {
        ScRange aTest = rDest[i];
        // Check for filtered rows in all selected sheets.
        for (const auto& rTab : rMark)
        {
            aTest.aStart.SetTab(rTab);
            aTest.aEnd.SetTab(rTab);
            if (ScViewUtil::HasFiltered(aTest, rDoc))
            {
                // I don't know how to handle pasting into filtered rows yet.
                return false;
            }
        }

        // Destination range must be an exact multiple of the source range.
        SCROW nRows = aTest.aEnd.Row() - aTest.aStart.Row() + 1;
        SCCOL nCols = aTest.aEnd.Col() - aTest.aStart.Col() + 1;
        SCROW nRowTest = (nRows / nSrcRows) * nSrcRows;
        SCCOL nColTest = (nCols / nSrcCols) * nSrcCols;
        if ( rDest.size() > 1 && ( nRows != nRowTest || nCols != nColTest ) )
        {
            // Destination range is not a multiple of the source range. Bail out.
            return false;
        }
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
