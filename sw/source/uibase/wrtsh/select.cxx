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

#include <limits.h>
#include <hintids.hxx>
#include <sfx2/bindings.hxx>
#include <svl/eitem.hxx>
#include <svl/macitem.hxx>
#include <unotools/charclass.hxx>
#include <editeng/scripttypeitem.hxx>
#include <cmdid.h>
#include <view.hxx>
#include <basesh.hxx>
#include <wrtsh.hxx>
#include <frmatr.hxx>
#include <initui.hxx>
#include <mdiexp.hxx>
#include <fmtcol.hxx>
#include <frmfmt.hxx>
#include <swundo.hxx>
#include <swevent.hxx>
#include <swdtflvr.hxx>
#include <crsskip.hxx>
#include <doc.hxx>
#include <wordcountdialog.hxx>
#include <boost/scoped_ptr.hpp>

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }

using namespace ::com::sun::star::util;

static long nStartDragX = 0, nStartDragY = 0;
static bool  bStartDrag = false;

void SwWrtShell::Invalidate()
{
    // to avoid making the slot volatile, invalidate it every time if something could have been changed
    // this is still much cheaper than asking for the state every 200 ms (and avoid background processing)
    GetView().GetViewFrame()->GetBindings().Invalidate( FN_STAT_SELMODE );
    SwWordCountWrapper *pWrdCnt = (SwWordCountWrapper*)GetView().GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId());
    if (pWrdCnt)
        pWrdCnt->UpdateCounts();
}

bool SwWrtShell::SelNearestWrd()
{
    SwMvContext aMvContext(this);
    if( !IsInWrd() && !IsEndWrd() && !IsSttWrd() )
        PrvWrd();
    if( IsEndWrd() )
        Left(CRSR_SKIP_CELLS, false, 1, false );
    return SelWrd();
}

bool SwWrtShell::SelWrd(const Point *pPt, bool )
{
    bool bRet;
    {
        SwMvContext aMvContext(this);
        SttSelect();
        bRet = SwCrsrShell::SelectWord( pPt );
    }
    EndSelect();
    if( bRet )
    {
        bSelWrd = true;
        if(pPt)
            aStart = *pPt;
    }
    return bRet;
}

void SwWrtShell::SelSentence(const Point *pPt, bool )
{
    {
        SwMvContext aMvContext(this);
        ClearMark();
        SwCrsrShell::GoStartSentence();
        SttSelect();
        SwCrsrShell::GoEndSentence();
    }
    EndSelect();
    if(pPt)
        aStart = *pPt;
    bSelLn = true;
    bSelWrd = false;  // disable SelWord, otherwise no SelLine goes on
}

void SwWrtShell::SelPara(const Point *pPt, bool )
{
    {
        SwMvContext aMvContext(this);
        ClearMark();
        SwCrsrShell::MovePara( fnParaCurr, fnParaStart );
        SttSelect();
        SwCrsrShell::MovePara( fnParaCurr, fnParaEnd );
    }
    EndSelect();
    if(pPt)
        aStart = *pPt;
    bSelLn = false;
    bSelWrd = false;  // disable SelWord, otherwise no SelLine goes on
}

long SwWrtShell::SelAll()
{
    const bool bLockedView = IsViewLocked();
    LockView( true );
    {
        if(bBlockMode)
            LeaveBlockMode();
        SwMvContext aMvContext(this);
        bool bMoveTable = false;
        boost::scoped_ptr<SwPosition> pStartPos;
        boost::scoped_ptr<SwPosition> pEndPos;
        SwShellCrsr* pTmpCrsr = 0;

        // Query these early, before we move the cursor.
        bool bHasWholeTabSelection = HasWholeTabSelection();
        bool bIsCursorInTable = IsCrsrInTbl();

        if (!bHasWholeTabSelection)
        {
            if ( IsSelection() && IsCrsrPtAtEnd() )
                SwapPam();
            pTmpCrsr = getShellCrsr( false );
            if( pTmpCrsr )
            {
                pStartPos.reset(new SwPosition( *pTmpCrsr->GetPoint() ));
                pEndPos.reset(new SwPosition( *pTmpCrsr->GetMark() ));
            }
            Push();
            bool bIsFullSel = !MoveSection( fnSectionCurr, fnSectionStart);
            SwapPam();
            bIsFullSel &= !MoveSection( fnSectionCurr, fnSectionEnd);
            Pop(false);
            GoStart(true, &bMoveTable, false, !bIsFullSel);
        }
        else
        {
            EnterStdMode();
            SttEndDoc(true);
        }
        SttSelect();
        GoEnd(true, &bMoveTable);

        bool bNeedsExtendedSelectAll = StartsWithTable();

        // If the cursor was in a table, then we only need the extended select
        // all if the whole table is already selected, to still allow selecting
        // only a single cell or a single table before selecting the whole
        // document.
        if (bNeedsExtendedSelectAll && bIsCursorInTable)
            bNeedsExtendedSelectAll = bHasWholeTabSelection;

        if (bNeedsExtendedSelectAll)
        {
            // Disable table cursor to make sure getShellCrsr() returns m_pCurCrsr, not m_pTblCrsr.
            if (IsTableMode())
                TblCrsrToCursor();
            // Do the extended select all on m_pCurCrsr.
            ExtendedSelectAll(/*bFootnotes =*/ false);
        }

        SwDoc *pDoc = GetDoc();
        if ( pDoc )
        {
            pDoc->SetPrepareSelAll();
        }

        if( pStartPos )
        {
            pTmpCrsr = getShellCrsr( false );
            if( pTmpCrsr )
            {
                // Some special handling for sections (e.g. TOC) at the beginning of the document body
                // to avoid the selection of the first section
                // if the last selection was behind the first section or
                // if the last selection was already the first section
                // In this both cases we select to the end of document
                if( ( *pTmpCrsr->GetPoint() < *pEndPos ||
                    ( *pStartPos == *pTmpCrsr->GetMark() &&
                      *pEndPos == *pTmpCrsr->GetPoint() ) ) && !bNeedsExtendedSelectAll)
                    SwCrsrShell::SttEndDoc(false);
            }
        }
    }
    EndSelect();
    LockView( bLockedView );
    return 1;
}

// Desciption: Text search

sal_uLong SwWrtShell::SearchPattern( const SearchOptions& rSearchOpt, bool bSearchInNotes,
                                SwDocPositions eStt, SwDocPositions eEnd,
                                FindRanges eFlags, bool bReplace )
{
        // no enhancement of existing selections
    if(!(eFlags & FND_IN_SEL))
        ClearMark();
    bool bCancel = false;
    sal_uLong nRet = Find( rSearchOpt, bSearchInNotes, eStt, eEnd, bCancel, eFlags, bReplace );
    if(bCancel)
    {
        Undo(1);
        nRet = ULONG_MAX;
    }
    return nRet;
}

// Description: search for templates

sal_uLong SwWrtShell::SearchTempl( const OUString &rTempl,
                               SwDocPositions eStt, SwDocPositions eEnd,
                               FindRanges eFlags, const OUString* pReplTempl )
{
        // no enhancement of existing selections
    if(!(eFlags & FND_IN_SEL))
        ClearMark();
    SwTxtFmtColl *pColl = GetParaStyle(rTempl, SwWrtShell::GETSTYLE_CREATESOME);
    SwTxtFmtColl *pReplaceColl = 0;
    if( pReplTempl )
        pReplaceColl = GetParaStyle(*pReplTempl, SwWrtShell::GETSTYLE_CREATESOME );

    bool bCancel = false;
    sal_uLong nRet = Find(pColl? *pColl: GetDfltTxtFmtColl(),
                               eStt,eEnd, bCancel, eFlags, pReplaceColl);
    if(bCancel)
    {
        Undo(1);
        nRet = ULONG_MAX;
    }
    return nRet;
}

// search for attributes

sal_uLong SwWrtShell::SearchAttr( const SfxItemSet& rFindSet, bool bNoColls,
                                SwDocPositions eStart, SwDocPositions eEnde,
                                FindRanges eFlags, const SearchOptions* pSearchOpt,
                                const SfxItemSet* pReplaceSet )
{
    // no enhancement of existing selections
    if (!(eFlags & FND_IN_SEL))
        ClearMark();

    // Searching
    bool bCancel = false;
    sal_uLong nRet = Find( rFindSet, bNoColls, eStart, eEnde, bCancel, eFlags, pSearchOpt, pReplaceSet);

    if(bCancel)
    {
        Undo(1);
        nRet = ULONG_MAX;
    }
    return nRet;
}

// Selection modes

void SwWrtShell::PushMode()
{
    pModeStack = new ModeStack( pModeStack, bIns, bExtMode, bAddMode, bBlockMode );
}

void SwWrtShell::PopMode()
{
    if ( 0 == pModeStack )
        return;

    if ( bExtMode && !pModeStack->bExt )
        LeaveExtMode();
    if ( bAddMode && !pModeStack->bAdd )
        LeaveAddMode();
    if ( bBlockMode && !pModeStack->bBlock )
        LeaveBlockMode();
    bIns = pModeStack->bIns;

    ModeStack *pTmp = pModeStack->pNext;
    delete pModeStack;
    pModeStack = pTmp;
}

// Two methodes for setting cursors: the first maps at the
// eponymous methodes in the CursorShell, the second removes
// all selections at first.

long SwWrtShell::SetCrsr(const Point *pPt, bool bTextOnly)
{
        // Remove a possibly present selection at the position
        // of the mouseclick

    if(!IsInSelect() && ChgCurrPam(*pPt)) {
        ClearMark();
    }

    return SwCrsrShell::SetCrsr(*pPt, bTextOnly);
}

long SwWrtShell::SetCrsrKillSel(const Point *pPt, bool bTextOnly )
{
    SwActContext aActContext(this);
    ResetSelect(pPt,false);
    return SwCrsrShell::SetCrsr(*pPt, bTextOnly);
}

void SwWrtShell::UnSelectFrm()
{
    // Remove Frame selection with guaranteed invalid position
    Point aPt(LONG_MIN, LONG_MIN);
    SelectObj(aPt, 0);
    SwTransferable::ClearSelection( *this );
}

// Remove of all selections

long SwWrtShell::ResetSelect(const Point *,bool)
{
    if(IsSelFrmMode())
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }
    else
    {
        //  SwActContext opens an Action -
        //  to avoid problems in the basic process with the
        //  shell switching, GetChgLnk().Call() may be called
        //  after EndAction().
        {
            SwActContext aActContext(this);
            bSelWrd = bSelLn = false;
            KillPams();
            ClearMark();
            fnKillSel = &SwWrtShell::Ignore;
            fnSetCrsr = &SwWrtShell::SetCrsr;
        }

        // After canceling of all selections an update of Attr-Controls
        // could be necessary.
        GetChgLnk().Call(this);
    }
    Invalidate();
    SwTransferable::ClearSelection( *this );
    return 1;
}

// Do nothing

long SwWrtShell::Ignore(const Point *, bool ) {
    return 1;
}

// Begin of a selection process.

void SwWrtShell::SttSelect()
{
    if(bInSelect)
        return;
    if(!HasMark())
        SetMark();
    if( bBlockMode )
    {
        SwShellCrsr* pTmp = getShellCrsr( true );
        if( !pTmp->HasMark() )
            pTmp->SetMark();
    }
    fnKillSel = &SwWrtShell::Ignore;
    fnSetCrsr = &SwWrtShell::SetCrsr;
    bInSelect = true;
    Invalidate();
    SwTransferable::CreateSelection( *this );
}

// End of a selection process.

void SwWrtShell::EndSelect()
{
    if(bInSelect && !bExtMode)
    {
        bInSelect = false;
        if (bAddMode)
        {
            AddLeaveSelect(0, false);
        }
        else
        {
            SttLeaveSelect(0, false);
            fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
            fnKillSel = &SwWrtShell::ResetSelect;
        }
    }
    SwWordCountWrapper *pWrdCnt = (SwWordCountWrapper*)GetView().GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId());
    if (pWrdCnt)
        pWrdCnt->UpdateCounts();
}

long SwWrtShell::ExtSelWrd(const Point *pPt, bool )
{
    SwMvContext aMvContext(this);
    if( IsTableMode() )
        return 1;

    // Bug 66823: actual crsr has in additional mode no selection?
    // Then destroy the actual an go to prev, this will be expand
    if( !HasMark() && GoPrevCrsr() )
    {
        bool bHasMark = HasMark(); // thats wrong!
        GoNextCrsr();
        if( bHasMark )
        {
            DestroyCrsr();
            GoPrevCrsr();
        }
    }

    // check the direction of the selection with the new point
    bool bRet = false, bMoveCrsr = true, bToTop = false;
    SwCrsrShell::SelectWord( &aStart );     // select the startword
    SwCrsrShell::Push();                    // save the cursor
    SwCrsrShell::SetCrsr( *pPt );           // and check the direction

    switch( SwCrsrShell::CompareCursor( StackMkCurrPt ))
    {
    case -1:    bToTop = false;     break;
    case 1:     bToTop = true;      break;
    default:    bMoveCrsr = false;  break;
    }

    SwCrsrShell::Pop( false );              // retore the saved cursor

    if( bMoveCrsr )
    {
        // select to Top but cursor select to Bottom? or
        // select to Bottom but cursor select to Top?       --> swap the cursor
        if( bToTop )
            SwapPam();

        SwCrsrShell::Push();                // save cur cursor
        if( SwCrsrShell::SelectWord( pPt )) // select the current word
        {
            if( bToTop )
                SwapPam();
            Combine();
            bRet = true;
        }
        else
        {
            SwCrsrShell::Pop( false );
            if( bToTop )
                SwapPam();
        }
    }
    else
        bRet = true;
    return bRet ? 1 : 0;
}

long SwWrtShell::ExtSelLn(const Point *pPt, bool )
{
    SwMvContext aMvContext(this);
    SwCrsrShell::SetCrsr(*pPt);
    if( IsTableMode() )
        return 1;

    // Bug 66823: actual crsr has in additional mode no selection?
    // Then destroy the actual an go to prev, this will be expand
    if( !HasMark() && GoPrevCrsr() )
    {
        bool bHasMark = HasMark(); // thats wrong!
        GoNextCrsr();
        if( bHasMark )
        {
            DestroyCrsr();
            GoPrevCrsr();
        }
    }

    // if applicable fit the selection to the "Mark"
    bool bToTop = !IsCrsrPtAtEnd();
    SwapPam();

    // The "Mark" has to be at the end or the beginning of the line.
    if( bToTop ? !IsEndSentence() : !IsStartSentence() )
    {
        if( bToTop )
        {
            if( !IsEndPara() )
                SwCrsrShell::Right(1,CRSR_SKIP_CHARS);
            SwCrsrShell::GoEndSentence();
        }
        else
            SwCrsrShell::GoStartSentence();
    }
    SwapPam();

    return (bToTop ? SwCrsrShell::GoStartSentence() : SwCrsrShell::GoEndSentence()) ? 1 : 0;
}

// Back into the standard mode: no mode, no selections.

void SwWrtShell::EnterStdMode()
{
    if(bAddMode)
        LeaveAddMode();
    if(bBlockMode)
        LeaveBlockMode();
    bBlockMode = false;
    bExtMode = false;
    bInSelect = false;
    if(IsSelFrmMode())
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }
    else
    {
        // SwActContext opens and action which has to be
        // closed prior to the call of
        // GetChgLnk().Call()
        {
            SwActContext aActContext(this);
            bSelWrd = bSelLn = false;
            if( !IsRetainSelection() )
                KillPams();
            ClearMark();
            fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
            fnKillSel = &SwWrtShell::ResetSelect;
        }
    }
    Invalidate();
    SwTransferable::ClearSelection( *this );
}

// Extended Mode

void SwWrtShell::EnterExtMode()
{
    if(bBlockMode)
    {
        LeaveBlockMode();
        KillPams();
        ClearMark();
    }
    bExtMode = true;
    bAddMode = false;
    bBlockMode = false;
    SttSelect();
}

void SwWrtShell::LeaveExtMode()
{
    bExtMode = false;
    EndSelect();
}

// End of a selection; if the selection is empty,
// ClearMark().

long SwWrtShell::SttLeaveSelect(const Point *, bool )
{
    if(SwCrsrShell::HasSelection() && !IsSelTblCells() && bClearMark) {
        return 0;
    }
    ClearMark();
    return 1;
}

// Leaving of the selection mode in additional mode

long SwWrtShell::AddLeaveSelect(const Point *, bool )
{
    if(IsTableMode()) LeaveAddMode();
    else if(SwCrsrShell::HasSelection())
        CreateCrsr();
    return 1;
}

// Additional Mode

void SwWrtShell::EnterAddMode()
{
    if(IsTableMode()) return;
    if(bBlockMode)
        LeaveBlockMode();
    fnKillSel = &SwWrtShell::Ignore;
    fnSetCrsr = &SwWrtShell::SetCrsr;
    bAddMode = true;
    bBlockMode = false;
    bExtMode = false;
    if(SwCrsrShell::HasSelection())
        CreateCrsr();
    Invalidate();
}

void SwWrtShell::LeaveAddMode()
{
    fnKillSel = &SwWrtShell::ResetSelect;
    fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
    bAddMode = false;
    Invalidate();
}

// Block Mode

void SwWrtShell::EnterBlockMode()
{
    bBlockMode = false;
    EnterStdMode();
    bBlockMode = true;
    CrsrToBlockCrsr();
    Invalidate();
}

void SwWrtShell::LeaveBlockMode()
{
    bBlockMode = false;
    BlockCrsrToCrsr();
    EndSelect();
    Invalidate();
}

// Insert mode

void SwWrtShell::SetInsMode( bool bOn )
{
    bIns = bOn;
    SwCrsrShell::SetOverwriteCrsr( !bIns );
    const SfxBoolItem aTmp( SID_ATTR_INSERT, bIns );
    GetView().GetViewFrame()->GetBindings().SetState( aTmp );
    StartAction();
    EndAction();
    Invalidate();
}
//Overwrite mode is incompatible with red-lining
void SwWrtShell::SetRedlineModeAndCheckInsMode( sal_uInt16 eMode )
{
   SetRedlineMode( eMode );
   if (IsRedlineOn())
       SetInsMode( true );
}

// Edit frame

long SwWrtShell::BeginFrmDrag(const Point *pPt, bool bIsShift)
{
    fnDrag = &SwFEShell::Drag;
    if(bStartDrag)
    {
        Point aTmp( nStartDragX, nStartDragY );
        SwFEShell::BeginDrag( &aTmp, bIsShift );
    }
    else
        SwFEShell::BeginDrag( pPt, bIsShift );
    return 1;
}

void SwWrtShell::EnterSelFrmMode(const Point *pPos)
{
    if(pPos)
    {
        nStartDragX = pPos->X();
        nStartDragY = pPos->Y();
        bStartDrag = true;
    }
    bLayoutMode = true;
    HideCrsr();

        // equal call of BeginDrag in the SwFEShell
    fnDrag          = &SwWrtShell::BeginFrmDrag;
    fnEndDrag       = &SwWrtShell::UpdateLayoutFrm;
    SwBaseShell::SetFrmMode( FLY_DRAG_START, this );
    Invalidate();
}

void SwWrtShell::LeaveSelFrmMode()
{
    fnDrag          = &SwWrtShell::BeginDrag;
    fnEndDrag       = &SwWrtShell::DefaultEndDrag;
    bLayoutMode = false;
    bStartDrag = false;
    Edit();
    SwBaseShell::SetFrmMode( FLY_DRAG_END, this );
    Invalidate();
}

// Description: execute framebound macro

IMPL_LINK( SwWrtShell, ExecFlyMac, void *, pFlyFmt )
{
    const SwFrmFmt *pFmt = pFlyFmt ? (SwFrmFmt*)pFlyFmt : GetFlyFrmFmt();
    OSL_ENSURE(pFmt, "no frame format");
    const SvxMacroItem &rFmtMac = pFmt->GetMacro();

    if(rFmtMac.HasMacro(SW_EVENT_OBJECT_SELECT))
    {
        const SvxMacro &rMac = rFmtMac.GetMacro(SW_EVENT_OBJECT_SELECT);
        if( IsFrmSelected() )
            bLayoutMode = true;
        CallChgLnk();
        ExecMacro( rMac );
    }
    return 0;
}

long SwWrtShell::UpdateLayoutFrm(const Point *pPt, bool )
{
        // still a dummy
    SwFEShell::EndDrag( pPt, false );
    fnDrag = &SwWrtShell::BeginFrmDrag;
    return 1;
}

// Handler for toggling the modes. Returns back the old mode.

bool SwWrtShell::ToggleAddMode()
{
    bAddMode ? LeaveAddMode(): EnterAddMode();
    Invalidate();
    return !bAddMode;
}

bool SwWrtShell::ToggleBlockMode()
{
    bBlockMode ? LeaveBlockMode(): EnterBlockMode();
    Invalidate();
    return !bBlockMode;
}

bool SwWrtShell::ToggleExtMode()
{
    bExtMode ? LeaveExtMode() : EnterExtMode();
    Invalidate();
    return !bExtMode;
}

// Dragging in standard mode (Selecting of content)

long SwWrtShell::BeginDrag(const Point * /*pPt*/, bool )
{
    if(bSelWrd)
    {
        bInSelect = true;
        if( !IsCrsrPtAtEnd() )
            SwapPam();

        fnDrag = &SwWrtShell::ExtSelWrd;
        fnSetCrsr = &SwWrtShell::Ignore;
    }
    else if(bSelLn)
    {
        bInSelect = true;
        fnDrag = &SwWrtShell::ExtSelLn;
        fnSetCrsr = &SwWrtShell::Ignore;
    }
    else
    {
        fnDrag = &SwWrtShell::DefaultDrag;
        SttSelect();
    }

    return 1;
}

long SwWrtShell::DefaultDrag(const Point *, bool )
{
    if( IsSelTblCells() )
        aSelTblLink.Call(this);

    return 1;
}

long SwWrtShell::DefaultEndDrag(const Point * /*pPt*/, bool )
{
    fnDrag = &SwWrtShell::BeginDrag;
    if( IsExtSel() )
        LeaveExtSel();

    if( IsSelTblCells() )
        aSelTblLink.Call(this);
    EndSelect();
    return 1;
}

// #i32329# Enhanced table selection
bool SwWrtShell::SelectTableRowCol( const Point& rPt, const Point* pEnd, bool bRowDrag )
{
    SwMvContext aMvContext(this);
    SttSelect();
    if(SelTblRowCol( rPt, pEnd, bRowDrag ))
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return true;
    }
    return false;
}

// Description: Selection of a table line or column

bool SwWrtShell::SelectTableRow()
{
    if ( SelTblRow() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return true;
    }
    return false;
}

bool SwWrtShell::SelectTableCol()
{
    if ( SelTblCol() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return true;
    }
    return false;
}

bool SwWrtShell::SelectTableCell()
{
    if ( SelTblBox() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return true;
    }
    return false;
}

// Description: Check if a word selection is present.
//              According to the rules for intelligent cut / paste
//              surrounding spaces are cut out.
// Return:      Delivers the type of the word selection.

int SwWrtShell::IntelligentCut(int nSelection, bool bCut)
{
        // On multiple selection no intelligent drag and drop
        // there are multiple cursors, since a second was placed
        // already at the target position.
    if( IsAddMode() || !(nSelection & nsSelectionType::SEL_TXT) )
        return sal_False;

    OUString sTxt;
    CharClass& rCC = GetAppCharClass();

        // If the first character is no word character,
        // no word selected.
    sal_Unicode cPrev = GetChar(false);
    sal_Unicode cNext = GetChar(true, -1);
    if( !cPrev || !cNext ||
        !rCC.isLetterNumeric( ( sTxt = OUString(cPrev) ), 0 ) ||
        !rCC.isLetterNumeric( ( sTxt = OUString(cNext) ), 0 ) )
        return NO_WORD;

    cPrev = GetChar(false, -1);
    cNext = GetChar(true);

    int cWord = NO_WORD;
        // is a word seleced?
    if(!cWord && cPrev && cNext &&
        CH_TXTATR_BREAKWORD != cPrev && CH_TXTATR_INWORD != cPrev &&
        CH_TXTATR_BREAKWORD != cNext && CH_TXTATR_INWORD != cNext &&
        !rCC.isLetterNumeric( ( sTxt = OUString(cPrev) ), 0 ) &&
        !rCC.isLetterNumeric( ( sTxt = OUString(cNext) ), 0 ) )
       cWord = WORD_NO_SPACE;

    if(cWord == WORD_NO_SPACE && ' ' == cPrev )
    {
        cWord = WORD_SPACE_BEFORE;
            // delete the space before
        if(bCut)
        {
            Push();
            if(IsCrsrPtAtEnd())
                SwapPam();
            ClearMark();
            SetMark();
            SwCrsrShell::Left(1,CRSR_SKIP_CHARS);
            SwFEShell::Delete();
            Pop( false );
        }
    }
    else if(cWord == WORD_NO_SPACE && cNext == ' ')
    {
        cWord = WORD_SPACE_AFTER;
            // delete the space behind
        if(bCut) {
            Push();
            if(!IsCrsrPtAtEnd()) SwapPam();
            ClearMark();
            SetMark();
            SwCrsrShell::Right(1,CRSR_SKIP_CHARS);
            SwFEShell::Delete();
            Pop( false );
        }
    }
    return cWord;
}

    // jump to the next / previous hyperlink - inside text and also
    // on graphics
bool SwWrtShell::SelectNextPrevHyperlink( bool bNext )
{
    StartAction();
    bool bRet = SwCrsrShell::SelectNxtPrvHyperlink( bNext );
    if( !bRet )
    {
        // will we have this feature?
        EnterStdMode();
        if( bNext )
            SttEndDoc(true);
        else
            SttEndDoc(false);
        bRet = SwCrsrShell::SelectNxtPrvHyperlink( bNext );
    }
    EndAction();

    bool bCreateXSelection = false;
    const bool bFrmSelected = IsFrmSelected() || IsObjSelected();
    if( IsSelection() )
    {
        if ( bFrmSelected )
            UnSelectFrm();

        // Set the function pointer for the canceling of the selection
        // set at cursor
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        bCreateXSelection = true;
    }
    else if( bFrmSelected )
    {
        EnterSelFrmMode();
        bCreateXSelection = true;
    }
    else if( (CNT_GRF | CNT_OLE ) & GetCntType() )
    {
        SelectObj( GetCharRect().Pos() );
        EnterSelFrmMode();
        bCreateXSelection = true;
    }

    if( bCreateXSelection )
        SwTransferable::CreateSelection( *this );

    return bRet;
}

// For the preservation of the selection the cursor will be moved left
// after SetMark(), so that the cursor is not moved by inserting text.
// Because a present selection at the CORE page is cleared at the
// current cursor position, the cursor will be pushed on the stack.
// After moving, they will again resummarized.

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
