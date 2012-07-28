/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <svx/svdundo.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include "svl/zforlist.hxx"
#include "svl/zformat.hxx"

#include "viewfunc.hxx"
#include "detfunc.hxx"
#include "detdata.hxx"
#include "viewdata.hxx"
#include "drwlayer.hxx"
#include "docsh.hxx"
#include "undocell.hxx"
#include "futext.hxx"
#include "docfunc.hxx"
#include "globstr.hrc"
#include "sc.hrc"
#include "fusel.hxx"
#include "reftokenhelper.hxx"
#include "externalrefmgr.hxx"
#include "cell.hxx"
#include "markdata.hxx"

#include <vector>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::std::vector;

#define D_TIMEFACTOR              86400.0

//==================================================================

void ScViewFunc::DetectiveAddPred()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    pDocSh->GetDocFunc().DetectiveAddPred( GetViewData()->GetCurPos() );
    RecalcPPT();    //! use broadcast in DocFunc instead?
}

void ScViewFunc::DetectiveDelPred()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    pDocSh->GetDocFunc().DetectiveDelPred( GetViewData()->GetCurPos() );
    RecalcPPT();
}

void ScViewFunc::DetectiveAddSucc()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    pDocSh->GetDocFunc().DetectiveAddSucc( GetViewData()->GetCurPos() );
    RecalcPPT();
}

void ScViewFunc::DetectiveDelSucc()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    pDocSh->GetDocFunc().DetectiveDelSucc( GetViewData()->GetCurPos() );
    RecalcPPT();
}

void ScViewFunc::DetectiveAddError()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    pDocSh->GetDocFunc().DetectiveAddError( GetViewData()->GetCurPos() );
    RecalcPPT();
}

void ScViewFunc::DetectiveDelAll()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    pDocSh->GetDocFunc().DetectiveDelAll( GetViewData()->GetTabNo() );
    RecalcPPT();
}

void ScViewFunc::DetectiveMarkInvalid()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    pDocSh->GetDocFunc().DetectiveMarkInvalid( GetViewData()->GetTabNo() );
    RecalcPPT();
}

void ScViewFunc::DetectiveRefresh()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    pDocSh->GetDocFunc().DetectiveRefresh();
    RecalcPPT();
}

static void lcl_jumpToRange(const ScRange& rRange, ScViewData* pView, ScDocument* pDoc)
{
    String aAddrText;
    rRange.Format(aAddrText, SCR_ABS_3D, pDoc);
    SfxStringItem aPosItem(SID_CURRENTCELL, aAddrText);
    SfxBoolItem aUnmarkItem(FN_PARAM_1, true);        // remove existing selection
    pView->GetDispatcher().Execute(
        SID_CURRENTCELL, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
        &aPosItem, &aUnmarkItem, 0L);
}

void ScViewFunc::MarkAndJumpToRanges(const ScRangeList& rRanges)
{
    ScViewData* pView = GetViewData();
    ScDocShell* pDocSh = pView->GetDocShell();

    ScRangeList aRanges(rRanges);
    ScRange* p = aRanges.front();
    ScRangeList aRangesToMark;
    ScAddress aCurPos = pView->GetCurPos();
    size_t ListSize = aRanges.size();
    for ( size_t i = 0; i < ListSize; ++i )
    {
        p = aRanges[i];
        // Collect only those ranges that are on the same sheet as the current
        // cursor.
        if (p->aStart.Tab() == aCurPos.Tab())
            aRangesToMark.Append(*p);
    }

    if (aRangesToMark.empty())
        return;

    // Jump to the first range of all precedent ranges.
    p = aRangesToMark.front();
    lcl_jumpToRange(*p, pView, pDocSh->GetDocument());

    ListSize = aRangesToMark.size();
    for ( size_t i = 0; i < ListSize; ++i )
    {
        p = aRangesToMark[i];
        MarkRange(*p, false, true);
    }
}

void ScViewFunc::DetectiveMarkPred()
{
    ScViewData* pView = GetViewData();
    ScDocShell* pDocSh = pView->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMarkData = pView->GetMarkData();
    ScAddress aCurPos = pView->GetCurPos();
    ScRangeList aRanges;
    if (rMarkData.IsMarked() || rMarkData.IsMultiMarked())
        rMarkData.FillRangeListWithMarks(&aRanges, false);
    else
        aRanges.Append(aCurPos);

    vector<ScTokenRef> aRefTokens;
    pDocSh->GetDocFunc().DetectiveCollectAllPreds(aRanges, aRefTokens);

    if (aRefTokens.empty())
        // No precedents found.  Nothing to do.
        return;

    ScTokenRef p = aRefTokens.front();
    if (ScRefTokenHelper::isExternalRef(p))
    {
        // This is external.  Open the external document if available, and
        // jump to the destination.

        sal_uInt16 nFileId = p->GetIndex();
        ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
        const OUString* pPath = pRefMgr->getExternalFileName(nFileId);

        ScRange aRange;
        if (pPath && ScRefTokenHelper::getRangeFromToken(aRange, p, true))
        {
            const String& rTabName = p->GetString();
            OUStringBuffer aBuf;
            aBuf.append(*pPath);
            aBuf.append(sal_Unicode('#'));
            aBuf.append(rTabName);
            aBuf.append(sal_Unicode('.'));

            String aRangeStr;
            aRange.Format(aRangeStr, SCA_VALID);
            aBuf.append(aRangeStr);

            ScGlobal::OpenURL(aBuf.makeStringAndClear(), String());
        }
        return;
    }
    else
    {
        ScRange aRange;
        ScRefTokenHelper::getRangeFromToken(aRange, p, false);
        if (aRange.aStart.Tab() != aCurPos.Tab())
        {
            // The first precedent range is on a different sheet.  Jump to it
            // immediately and forget the rest.
            lcl_jumpToRange(aRange, pView, pDoc);
            return;
        }
    }

    ScRangeList aDestRanges;
    ScRefTokenHelper::getRangeListFromTokens(aDestRanges, aRefTokens);
    MarkAndJumpToRanges(aDestRanges);
}

void ScViewFunc::DetectiveMarkSucc()
{
    ScViewData* pView = GetViewData();
    ScDocShell* pDocSh = pView->GetDocShell();
    ScMarkData& rMarkData = pView->GetMarkData();
    ScAddress aCurPos = pView->GetCurPos();
    ScRangeList aRanges;
    if (rMarkData.IsMarked() || rMarkData.IsMultiMarked())
        rMarkData.FillRangeListWithMarks(&aRanges, false);
    else
        aRanges.Append(aCurPos);

    vector<ScTokenRef> aRefTokens;
    pDocSh->GetDocFunc().DetectiveCollectAllSuccs(aRanges, aRefTokens);

    if (aRefTokens.empty())
        // No dependants found.  Nothing to do.
        return;

    ScRangeList aDestRanges;
    ScRefTokenHelper::getRangeListFromTokens(aDestRanges, aRefTokens);
    MarkAndJumpToRanges(aDestRanges);
}

void ScViewFunc::InsertCurrentTime(short nCellFmt, const OUString& rUndoStr)
{
    ScViewData* pViewData = GetViewData();
    ScAddress aCurPos = pViewData->GetCurPos();
    ScDocShell* pDocSh = pViewData->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ::svl::IUndoManager* pUndoMgr = pDocSh->GetUndoManager();
    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
    Date aActDate( Date::SYSTEM );
    double fDate = aActDate - *pFormatter->GetNullDate();
    Time aActTime( Time::SYSTEM );
    double fTime =
        aActTime.Get100Sec() / 100.0 + aActTime.GetSec() +
        (aActTime.GetMin() * 60.0) + (aActTime.GetHour() * 3600.0);
    fTime /= D_TIMEFACTOR;
    pUndoMgr->EnterListAction(rUndoStr, rUndoStr);
    pDocSh->GetDocFunc().PutCell(aCurPos, new ScValueCell(fDate+fTime), false);

    // Set the new cell format only when it differs from the current cell
    // format type.
    sal_uInt32 nCurNumFormat = pDoc->GetNumberFormat(aCurPos);
    const SvNumberformat* pEntry = pFormatter->GetEntry(nCurNumFormat);
    if (!pEntry || !(pEntry->GetType() & nCellFmt))
        SetNumberFormat(nCellFmt);
    pUndoMgr->LeaveListAction();
}

//---------------------------------------------------------------------------

void ScViewFunc::ShowNote( bool bShow )
{
    if( bShow )
        HideNoteMarker();
    const ScViewData& rViewData = *GetViewData();
    ScAddress aPos( rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo() );
    // show note moved to ScDocFunc, to be able to use it in notesuno.cxx
    rViewData.GetDocShell()->GetDocFunc().ShowNote( aPos, bShow );
}

void ScViewFunc::EditNote()
{
    //  zum Editieren einblenden und aktivieren

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCCOL nCol = GetViewData()->GetCurX();
    SCROW nRow = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScAddress aPos( nCol, nRow, nTab );

    // start drawing undo to catch undo action for insertion of the caption object
    pDocSh->MakeDrawLayer();
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    pDrawLayer->BeginCalcUndo();
    // generated undo action is processed in FuText::StopEditMode

    // get existing note or create a new note (including caption drawing object)
    if( ScPostIt* pNote = pDoc->GetNotes(aPos.Tab())->GetOrCreateNote( aPos ) )
    {
        // hide temporary note caption
        HideNoteMarker();
        // show caption object without changing internal visibility state
        pNote->ShowCaptionTemp( aPos );

        /*  Drawing object has been created in ScDocument::GetOrCreateNote() or
            in ScPostIt::ShowCaptionTemp(), so ScPostIt::GetCaption() should
            return a caption object. */
        if( SdrCaptionObj* pCaption = pNote->GetCaption() )
        {
            // #i33764# enable the resize handles before starting edit mode
            if( FuPoor* pDraw = GetDrawFuncPtr() )
                static_cast< FuSelection* >( pDraw )->ActivateNoteHandles( pCaption );

            // activate object (as in FuSelection::TestComment)
            GetViewData()->GetDispatcher().Execute( SID_DRAW_NOTEEDIT, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
            // jetzt den erzeugten FuText holen und in den EditModus setzen
            FuPoor* pPoor = GetDrawFuncPtr();
            if ( pPoor && (pPoor->GetSlotID() == SID_DRAW_NOTEEDIT) )    // hat keine RTTI
            {
                ScrollToObject( pCaption );         // Objekt komplett sichtbar machen
                static_cast< FuText* >( pPoor )->SetInEditMode( pCaption );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
