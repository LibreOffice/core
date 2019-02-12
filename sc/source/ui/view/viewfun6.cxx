/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/svdundo.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <editeng/editview.hxx>
#include <sal/log.hxx>

#include <viewfunc.hxx>
#include <detfunc.hxx>
#include <detdata.hxx>
#include <viewdata.hxx>
#include <drwlayer.hxx>
#include <docsh.hxx>
#include <undocell.hxx>
#include <futext.hxx>
#include <docfunc.hxx>
#include <sc.hrc>
#include <fusel.hxx>
#include <reftokenhelper.hxx>
#include <externalrefmgr.hxx>
#include <formulacell.hxx>
#include <markdata.hxx>
#include <drawview.hxx>
#include <globalnames.hxx>
#include <inputhdl.hxx>
#include <tabvwsh.hxx>
#include <scmod.hxx>

#include <vector>

using ::std::vector;

void ScViewFunc::DetectiveAddPred()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    pDocSh->GetDocFunc().DetectiveAddPred( GetViewData().GetCurPos() );
    RecalcPPT();    //! use broadcast in DocFunc instead?
}

void ScViewFunc::DetectiveDelPred()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    pDocSh->GetDocFunc().DetectiveDelPred( GetViewData().GetCurPos() );
    RecalcPPT();
}

void ScViewFunc::DetectiveAddSucc()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    pDocSh->GetDocFunc().DetectiveAddSucc( GetViewData().GetCurPos() );
    RecalcPPT();
}

void ScViewFunc::DetectiveDelSucc()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    pDocSh->GetDocFunc().DetectiveDelSucc( GetViewData().GetCurPos() );
    RecalcPPT();
}

void ScViewFunc::DetectiveAddError()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    pDocSh->GetDocFunc().DetectiveAddError( GetViewData().GetCurPos() );
    RecalcPPT();
}

void ScViewFunc::DetectiveDelAll()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    pDocSh->GetDocFunc().DetectiveDelAll( GetViewData().GetTabNo() );
    RecalcPPT();
}

void ScViewFunc::DetectiveMarkInvalid()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    pDocSh->GetDocFunc().DetectiveMarkInvalid( GetViewData().GetTabNo() );
    RecalcPPT();
}

void ScViewFunc::DetectiveRefresh()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    pDocSh->GetDocFunc().DetectiveRefresh();
    RecalcPPT();
}

static void lcl_jumpToRange(const ScRange& rRange, ScViewData* pView, const ScDocument* pDoc)
{
    OUString aAddrText(rRange.Format(ScRefFlags::RANGE_ABS_3D, pDoc));
    SfxStringItem aPosItem(SID_CURRENTCELL, aAddrText);
    SfxBoolItem aUnmarkItem(FN_PARAM_1, true);        // remove existing selection
    pView->GetDispatcher().ExecuteList(
        SID_CURRENTCELL, SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
        { &aPosItem, &aUnmarkItem });
}

void ScViewFunc::MarkAndJumpToRanges(const ScRangeList& rRanges)
{
    ScViewData& rView = GetViewData();
    ScDocShell* pDocSh = rView.GetDocShell();

    ScRangeList aRanges(rRanges);
    ScRangeList aRangesToMark;
    ScAddress aCurPos = rView.GetCurPos();
    size_t ListSize = aRanges.size();
    for ( size_t i = 0; i < ListSize; ++i )
    {
        const ScRange & r = aRanges[i];
        // Collect only those ranges that are on the same sheet as the current
        // cursor.
        if (r.aStart.Tab() == aCurPos.Tab())
            aRangesToMark.push_back(r);
    }

    if (aRangesToMark.empty())
        return;

    // Jump to the first range of all precedent ranges.
    const ScRange & r = aRangesToMark.front();
    lcl_jumpToRange(r, &rView, &pDocSh->GetDocument());

    ListSize = aRangesToMark.size();
    for ( size_t i = 0; i < ListSize; ++i )
    {
        MarkRange(aRangesToMark[i], false, true);
    }
}

void ScViewFunc::DetectiveMarkPred()
{
    ScViewData& rView = GetViewData();
    ScDocShell* pDocSh = rView.GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScMarkData& rMarkData = rView.GetMarkData();
    ScAddress aCurPos = rView.GetCurPos();
    ScRangeList aRanges;
    if (rMarkData.IsMarked() || rMarkData.IsMultiMarked())
        rMarkData.FillRangeListWithMarks(&aRanges, false);
    else
        aRanges.push_back(aCurPos);

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
        ScExternalRefManager* pRefMgr = rDoc.GetExternalRefManager();
        const OUString* pPath = pRefMgr->getExternalFileName(nFileId);

        ScRange aRange;
        if (pPath && ScRefTokenHelper::getRangeFromToken(aRange, p, aCurPos, true))
        {
            OUString aTabName = p->GetString().getString();
            OUStringBuffer aBuf;
            aBuf.append(*pPath);
            aBuf.append('#');
            aBuf.append(aTabName);
            aBuf.append('.');

            OUString aRangeStr(aRange.Format(ScRefFlags::VALID));
            aBuf.append(aRangeStr);

            ScGlobal::OpenURL(aBuf.makeStringAndClear(), OUString());
        }
        return;
    }
    else
    {
        ScRange aRange;
        ScRefTokenHelper::getRangeFromToken(aRange, p, aCurPos);
        if (aRange.aStart.Tab() != aCurPos.Tab())
        {
            // The first precedent range is on a different sheet.  Jump to it
            // immediately and forget the rest.
            lcl_jumpToRange(aRange, &rView, &rDoc);
            return;
        }
    }

    ScRangeList aDestRanges;
    ScRefTokenHelper::getRangeListFromTokens(aDestRanges, aRefTokens, aCurPos);
    MarkAndJumpToRanges(aDestRanges);
}

void ScViewFunc::DetectiveMarkSucc()
{
    ScViewData& rView = GetViewData();
    ScDocShell* pDocSh = rView.GetDocShell();
    ScMarkData& rMarkData = rView.GetMarkData();
    ScAddress aCurPos = rView.GetCurPos();
    ScRangeList aRanges;
    if (rMarkData.IsMarked() || rMarkData.IsMultiMarked())
        rMarkData.FillRangeListWithMarks(&aRanges, false);
    else
        aRanges.push_back(aCurPos);

    vector<ScTokenRef> aRefTokens;
    pDocSh->GetDocFunc().DetectiveCollectAllSuccs(aRanges, aRefTokens);

    if (aRefTokens.empty())
        // No dependents found.  Nothing to do.
        return;

    ScRangeList aDestRanges;
    ScRefTokenHelper::getRangeListFromTokens(aDestRanges, aRefTokens, aCurPos);
    MarkAndJumpToRanges(aDestRanges);
}

/** Insert date or time into current cell.

    If cell is in input or edit mode, insert date/time at cursor position, else
    create a date or time or date+time cell as follows:

    - key date on time cell  =>  current date + time of cell  =>  date+time formatted cell
      - unless time cell was empty or 00:00 time  =>  current date  =>  date formatted cell
    - key date on date+time cell  =>  current date + 00:00 time  =>  date+time formatted cell
      - unless date was current date  =>  current date  =>  date formatted cell
    - key date on other cell  =>  current date  =>  date formatted cell
    - key time on date cell  =>  date of cell + current time  =>  date+time formatted cell
      - unless date cell was empty  =>  current time  =>  time formatted cell
    - key time on date+time cell  =>  current time  =>  time formatted cell
      - unless cell was empty  =>  current date+time  =>  date+time formatted cell
    - key time on other cell  =>  current time  =>  time formatted cell
 */
void ScViewFunc::InsertCurrentTime(SvNumFormatType nReqFmt, const OUString& rUndoStr)
{
    ScViewData& rViewData = GetViewData();

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl( rViewData.GetViewShell());
    bool bInputMode = (pInputHdl && pInputHdl->IsInputMode());

    ScDocShell* pDocSh = rViewData.GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScAddress aCurPos = rViewData.GetCurPos();
    const sal_uInt32 nCurNumFormat = rDoc.GetNumberFormat(aCurPos);
    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
    const SvNumberformat* pCurNumFormatEntry = pFormatter->GetEntry(nCurNumFormat);
    const SvNumFormatType nCurNumFormatType = (pCurNumFormatEntry ?
            pCurNumFormatEntry->GetMaskedType() : SvNumFormatType::UNDEFINED);

    if (bInputMode)
    {
        double fVal = 0.0;
        sal_uInt32 nFormat = 0;
        switch (nReqFmt)
        {
            case SvNumFormatType::DATE:
                {
                    Date aActDate( Date::SYSTEM );
                    fVal = aActDate - pFormatter->GetNullDate();
                    if (nCurNumFormatType == SvNumFormatType::DATE)
                        nFormat = nCurNumFormat;
                }
                break;
            case SvNumFormatType::TIME:
                {
                    tools::Time aActTime( tools::Time::SYSTEM );
                    fVal = aActTime.GetTimeInDays();
                    if (nCurNumFormatType == SvNumFormatType::TIME)
                        nFormat = nCurNumFormat;
                }
                break;
            default:
                SAL_WARN("sc.ui","unhandled current date/time request");
                nReqFmt = SvNumFormatType::DATETIME;
                [[fallthrough]];
            case SvNumFormatType::DATETIME:
                {
                    DateTime aActDateTime( DateTime::SYSTEM );
                    fVal = aActDateTime - DateTime( pFormatter->GetNullDate());
                    if (nCurNumFormatType == SvNumFormatType::DATETIME)
                        nFormat = nCurNumFormat;
                }
                break;
        }

        if (!nFormat)
        {
            const LanguageType nLang = (pCurNumFormatEntry ? pCurNumFormatEntry->GetLanguage() : ScGlobal::eLnge);
            nFormat = pFormatter->GetStandardFormat( nReqFmt, nLang);
            // This would return a more precise format with seconds and 100th
            // seconds for a time request.
            //nFormat = pFormatter->GetStandardFormat( fVal, nFormat, nReqFmt, nLang);
        }
        OUString aString;
        Color* pColor;
        pFormatter->GetOutputString( fVal, nFormat, aString, &pColor);

        pInputHdl->DataChanging();
        EditView* pTopView = pInputHdl->GetTopView();
        if (pTopView)
            pTopView->InsertText( aString);
        EditView* pTableView = pInputHdl->GetTableView();
        if (pTableView)
            pTableView->InsertText( aString);
        pInputHdl->DataChanged();
    }
    else
    {
        bool bForceReqFmt = false;
        const double fCell = rDoc.GetValue( aCurPos);
        // Combine requested date/time stamp with existing cell time/date, if any.
        switch (nReqFmt)
        {
            case SvNumFormatType::DATE:
                switch (nCurNumFormatType)
                {
                    case SvNumFormatType::TIME:
                        // An empty cell formatted as time (or 00:00 time) shall
                        // not result in the current date with 00:00 time, but only
                        // in current date.
                        if (fCell != 0.0)
                            nReqFmt = SvNumFormatType::DATETIME;
                        break;
                    case SvNumFormatType::DATETIME:
                        {
                            // Force to only date if the existing date+time is the
                            // current date. This way inserting current date twice
                            // on an existing date+time cell can be used to force
                            // date, which otherwise would only be possible by
                            // applying a date format.
                            double fDate = rtl::math::approxFloor( fCell);
                            if (fDate == (Date( Date::SYSTEM) - pFormatter->GetNullDate()))
                                bForceReqFmt = true;
                        }
                        break;
                    default: break;
                }
                break;
            case SvNumFormatType::TIME:
                switch (nCurNumFormatType)
                {
                    case SvNumFormatType::DATE:
                        // An empty cell formatted as date shall not result in the
                        // null date and current time, but only in current time.
                        if (fCell != 0.0)
                            nReqFmt = SvNumFormatType::DATETIME;
                        break;
                    case SvNumFormatType::DATETIME:
                        // Requesting current time on an empty date+time cell
                        // inserts both current date+time.
                        if (fCell == 0.0)
                            nReqFmt = SvNumFormatType::DATETIME;
                        else
                        {
                            // Add current time to an existing date+time where time is
                            // zero and date is current date, else force time only.
                            double fDate = rtl::math::approxFloor( fCell);
                            double fTime = fCell - fDate;
                            if (fTime == 0.0 && fDate == (Date( Date::SYSTEM) - pFormatter->GetNullDate()))
                                nReqFmt = SvNumFormatType::DATETIME;
                            else
                                bForceReqFmt = true;
                        }
                        break;
                    default: break;
                }
                break;
            default:
                SAL_WARN("sc.ui","unhandled current date/time request");
                nReqFmt = SvNumFormatType::DATETIME;
                [[fallthrough]];
            case SvNumFormatType::DATETIME:
                break;
        }
        double fVal = 0.0;
        switch (nReqFmt)
        {
            case SvNumFormatType::DATE:
                {
                    Date aActDate( Date::SYSTEM );
                    fVal = aActDate - pFormatter->GetNullDate();
                }
                break;
            case SvNumFormatType::TIME:
                {
                    tools::Time aActTime( tools::Time::SYSTEM );
                    fVal = aActTime.GetTimeInDays();
                }
                break;
            case SvNumFormatType::DATETIME:
                switch (nCurNumFormatType)
                {
                    case SvNumFormatType::DATE:
                        {
                            double fDate = rtl::math::approxFloor( fCell);
                            tools::Time aActTime( tools::Time::SYSTEM );
                            fVal = fDate + aActTime.GetTimeInDays();
                        }
                        break;
                    case SvNumFormatType::TIME:
                        {
                            double fTime = fCell - rtl::math::approxFloor( fCell);
                            Date aActDate( Date::SYSTEM );
                            fVal = (aActDate - pFormatter->GetNullDate()) + fTime;
                        }
                        break;
                    default:
                        {
                            DateTime aActDateTime( DateTime::SYSTEM );
                            // Converting the null date to DateTime forces the
                            // correct operator-() to be used, resulting in a
                            // fractional date+time instead of only date value.
                            fVal = aActDateTime - DateTime( pFormatter->GetNullDate());
                        }
                }
                break;
            default: break;

        }

        SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();
        pUndoMgr->EnterListAction(rUndoStr, rUndoStr, 0, rViewData.GetViewShell()->GetViewShellId());

        pDocSh->GetDocFunc().SetValueCell(aCurPos, fVal, true);

        // Set the new cell format only when it differs from the current cell
        // format type. Preserve a date+time format unless we force a format
        // through.
        if (bForceReqFmt || (nReqFmt != nCurNumFormatType && nCurNumFormatType != SvNumFormatType::DATETIME))
            SetNumberFormat(nReqFmt);
        else
            rViewData.UpdateInputHandler();     // update input bar with new value

        pUndoMgr->LeaveListAction();
    }
}

void ScViewFunc::ShowNote( bool bShow )
{
    if( bShow )
        HideNoteMarker();
    const ScViewData& rViewData = GetViewData();
    ScAddress aPos( rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo() );
    // show note moved to ScDocFunc, to be able to use it in notesuno.cxx
    rViewData.GetDocShell()->GetDocFunc().ShowNote( aPos, bShow );
}

void ScViewFunc::EditNote()
{
    // for editing display and activate

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    SCCOL nCol = GetViewData().GetCurX();
    SCROW nRow = GetViewData().GetCurY();
    SCTAB nTab = GetViewData().GetTabNo();
    ScAddress aPos( nCol, nRow, nTab );

    // start drawing undo to catch undo action for insertion of the caption object
    pDocSh->MakeDrawLayer();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    pDrawLayer->BeginCalcUndo(true);
    // generated undo action is processed in FuText::StopEditMode

    // get existing note or create a new note (including caption drawing object)
    if( ScPostIt* pNote = rDoc.GetOrCreateNote( aPos ) )
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
            if ( ScDrawView* pScDrawView = GetScDrawView() )
               pScDrawView->SyncForGrid( pCaption );
            // #i33764# enable the resize handles before starting edit mode
            if( FuPoor* pDraw = GetDrawFuncPtr() )
                static_cast< FuSelection* >( pDraw )->ActivateNoteHandles( pCaption );

            // activate object (as in FuSelection::TestComment)
            GetViewData().GetDispatcher().Execute( SID_DRAW_NOTEEDIT, SfxCallMode::SYNCHRON | SfxCallMode::RECORD );
            // now get the created FuText and set into EditMode
            FuText* pFuText = dynamic_cast<FuText*>(GetDrawFuncPtr());
            if (pFuText)
            {
                ScrollToObject( pCaption );         // make object fully visible
                pFuText->SetInEditMode( pCaption );

                ScTabView::OnLOKNoteStateChanged( pNote );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
