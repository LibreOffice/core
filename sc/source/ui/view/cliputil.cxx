/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cliputil.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "transobj.hxx"
#include "document.hxx"
#include "dpobject.hxx"
#include "globstr.hrc"
#include "clipparam.hxx"
#include "clipoptions.hxx"
#include "rangelst.hxx"
#include "viewutil.hxx"
#include "markdata.hxx"
#include <gridwin.hxx>

#include <vcl/waitobj.hxx>
#include <sfx2/classificationhelper.hxx>

namespace
{

/// Paste only if SfxClassificationHelper recommends so.
bool lcl_checkClassification(ScDocument* pSourceDoc, ScDocument* pDestinationDoc)
{
    if (!pSourceDoc || !pDestinationDoc)
        return true;

    ScClipOptions* pSourceOptions = pSourceDoc->GetClipOptions();
    SfxObjectShell* pDestinationShell = pDestinationDoc->GetDocumentShell();
    if (!pSourceOptions || !pDestinationShell)
        return true;

    SfxClassificationCheckPasteResult eResult = SfxClassificationHelper::CheckPaste(pSourceOptions->m_xDocumentProperties, pDestinationShell->getDocProperties());
    return SfxClassificationHelper::ShowPasteInfo(eResult);
}

}

void ScClipUtil::PasteFromClipboard( ScViewData* pViewData, ScTabViewShell* pTabViewShell, bool bShowDialog )
{
    vcl::Window* pWin = pViewData->GetActiveWin();
    ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard( pWin );
    ScDocument* pThisDoc = pViewData->GetDocument();
    ScDPObject* pDPObj = pThisDoc->GetDPAtCursor( pViewData->GetCurX(),
                         pViewData->GetCurY(), pViewData->GetTabNo() );
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
        bool bDone = pTabViewShell->DataPilotMove( aSource, pViewData->GetCurPos() );
        if ( !bDone )
            pTabViewShell->ErrorMessage( STR_ERR_DATAPILOT_INPUT );
    }
    else
    {
        // normal paste
        WaitObject aWait( pViewData->GetDialogParent() );
        if (!pOwnClip)
            pTabViewShell->PasteFromSystem();
        else
        {
            ScDocument* pClipDoc = pOwnClip->GetDocument();
            InsertDeleteFlags nFlags = InsertDeleteFlags::ALL;
            if (pClipDoc->GetClipParam().isMultiRange())
                // For multi-range paste, we paste values by default.
                nFlags &= ~InsertDeleteFlags::FORMULA;

            if (lcl_checkClassification(pClipDoc, pThisDoc))
                pTabViewShell->PasteFromClip( nFlags, pClipDoc,
                        ScPasteFunc::NONE, false, false, false, INS_NONE, InsertDeleteFlags::NONE,
                        bShowDialog );      // allow warning dialog
        }
    }
    pTabViewShell->CellContentChanged();        // => PasteFromSystem() ???
}

bool ScClipUtil::CheckDestRanges(
    ScDocument* pDoc, SCCOL nSrcCols, SCROW nSrcRows, const ScMarkData& rMark, const ScRangeList& rDest)
{
    for (size_t i = 0, n = rDest.size(); i < n; ++i)
    {
        ScRange aTest = *rDest[i];
        // Check for filtered rows in all selected sheets.
        ScMarkData::const_iterator itrTab = rMark.begin(), itrTabEnd = rMark.end();
        for (; itrTab != itrTabEnd; ++itrTab)
        {
            aTest.aStart.SetTab(*itrTab);
            aTest.aEnd.SetTab(*itrTab);
            if (ScViewUtil::HasFiltered(aTest, pDoc))
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
