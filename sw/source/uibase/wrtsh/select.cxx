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
#include <sfx2/event.hxx>
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
#include <doc.hxx>
#include <wordcountdialog.hxx>
#include <memory>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions2;
} } } }

using namespace ::com::sun::star::util;

static long nStartDragX = 0, nStartDragY = 0;
static bool  bStartDrag = false;

void SwWrtShell::Invalidate()
{
    // to avoid making the slot volatile, invalidate it every time if something could have been changed
    // this is still much cheaper than asking for the state every 200 ms (and avoid background processing)
    GetView().GetViewFrame()->GetBindings().Invalidate( FN_STAT_SELMODE );
    SwWordCountWrapper *pWrdCnt = static_cast<SwWordCountWrapper*>(GetView().GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId()));
    if (pWrdCnt)
        pWrdCnt->UpdateCounts();
}

bool SwWrtShell::SelNearestWrd()
{
    SwMvContext aMvContext(this);
    if( !IsInWord() && !IsEndWrd() && !IsStartWord() )
        PrvWrd();
    if( IsEndWrd() )
        Left(CRSR_SKIP_CELLS, false, 1, false );
    return SelWrd();
}

bool SwWrtShell::SelWrd(const Point *pPt )
{
    bool bRet;
    {
        SwMvContext aMvContext(this);
        SttSelect();
        bRet = SwCursorShell::SelectWord( pPt );
    }
    EndSelect();
    if( bRet )
    {
        m_bSelWrd = true;
        if(pPt)
            m_aStart = *pPt;
    }
    return bRet;
}

void SwWrtShell::SelSentence(const Point *pPt )
{
    {
        SwMvContext aMvContext(this);
        ClearMark();
        SwCursorShell::GoStartSentence();
        SttSelect();
        SwCursorShell::GoEndSentence();
    }
    EndSelect();
    if(pPt)
        m_aStart = *pPt;
    m_bSelLn = true;
    m_bSelWrd = false;  // disable SelWord, otherwise no SelLine goes on
}

void SwWrtShell::SelPara(const Point *pPt )
{
    {
        SwMvContext aMvContext(this);
        ClearMark();
        SwCursorShell::MovePara( GoCurrPara, fnParaStart );
        SttSelect();
        SwCursorShell::MovePara( GoCurrPara, fnParaEnd );
    }
    EndSelect();
    if(pPt)
        m_aStart = *pPt;
    m_bSelLn = false;
    m_bSelWrd = false;  // disable SelWord, otherwise no SelLine goes on
}

void SwWrtShell::SelAll()
{
    const bool bLockedView = IsViewLocked();
    LockView( true );
    {
        if(m_bBlockMode)
            LeaveBlockMode();
        SwMvContext aMvContext(this);
        bool bMoveTable = false;
        std::unique_ptr<SwPosition> pStartPos;
        std::unique_ptr<SwPosition> pEndPos;
        SwShellCursor* pTmpCursor = nullptr;

        // Query these early, before we move the cursor.
        bool bHasWholeTabSelection = HasWholeTabSelection();
        bool bIsCursorInTable = IsCursorInTable();

        if (!bHasWholeTabSelection)
        {
            if ( IsSelection() && IsCursorPtAtEnd() )
                SwapPam();
            pTmpCursor = getShellCursor( false );
            if( pTmpCursor )
            {
                pStartPos.reset(new SwPosition( *pTmpCursor->GetPoint() ));
                pEndPos.reset(new SwPosition( *pTmpCursor->GetMark() ));
            }
            Push();
            bool bIsFullSel = !MoveSection( GoCurrSection, fnSectionStart);
            SwapPam();
            bIsFullSel &= !MoveSection( GoCurrSection, fnSectionEnd);
            Pop(SwCursorShell::PopMode::DeleteCurrent);
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
            // Disable table cursor to make sure getShellCursor() returns m_pCurrentCursor, not m_pTableCursor.
            if (IsTableMode())
                TableCursorToCursor();
            // Do the extended select all on m_pCurrentCursor.
            ExtendedSelectAll(/*bFootnotes =*/ false);
        }

        SwDoc *pDoc = GetDoc();
        if ( pDoc )
        {
            pDoc->SetPrepareSelAll();
        }

        if( pStartPos )
        {
            pTmpCursor = getShellCursor( false );
            if( pTmpCursor )
            {
                // Some special handling for sections (e.g. TOC) at the beginning of the document body
                // to avoid the selection of the first section
                // if the last selection was behind the first section or
                // if the last selection was already the first section
                // In this both cases we select to the end of document
                if( ( *pTmpCursor->GetPoint() < *pEndPos ||
                    ( *pStartPos == *pTmpCursor->GetMark() &&
                      *pEndPos == *pTmpCursor->GetPoint() ) ) && !bNeedsExtendedSelectAll)
                    SwCursorShell::SttEndDoc(false);
            }
        }
    }
    EndSelect();
    LockView( bLockedView );
}

// Description: Text search

sal_uLong SwWrtShell::SearchPattern( const i18nutil::SearchOptions2& rSearchOpt, bool bSearchInNotes,
                                SwDocPositions eStt, SwDocPositions eEnd,
                                FindRanges eFlags, bool bReplace )
{
        // no enhancement of existing selections
    if(!(eFlags & FindRanges::InSel))
        ClearMark();
    bool bCancel = false;
    sal_uLong nRet = Find( rSearchOpt, bSearchInNotes, eStt, eEnd, bCancel, eFlags, bReplace );
    if(bCancel)
    {
        Undo();
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
    if(!(eFlags & FindRanges::InSel))
        ClearMark();
    SwTextFormatColl *pColl = GetParaStyle(rTempl, SwWrtShell::GETSTYLE_CREATESOME);
    SwTextFormatColl *pReplaceColl = nullptr;
    if( pReplTempl )
        pReplaceColl = GetParaStyle(*pReplTempl, SwWrtShell::GETSTYLE_CREATESOME );

    bool bCancel = false;
    sal_uLong nRet = Find(pColl? *pColl: GetDfltTextFormatColl(),
                               eStt,eEnd, bCancel, eFlags, pReplaceColl);
    if(bCancel)
    {
        Undo();
        nRet = ULONG_MAX;
    }
    return nRet;
}

// search for attributes

sal_uLong SwWrtShell::SearchAttr( const SfxItemSet& rFindSet, bool bNoColls,
                                SwDocPositions eStart, SwDocPositions eEnd,
                                FindRanges eFlags, const i18nutil::SearchOptions2* pSearchOpt,
                                const SfxItemSet* pReplaceSet )
{
    // no enhancement of existing selections
    if (!(eFlags & FindRanges::InSel))
        ClearMark();

    // Searching
    bool bCancel = false;
    sal_uLong nRet = Find( rFindSet, bNoColls, eStart, eEnd, bCancel, eFlags, pSearchOpt, pReplaceSet);

    if(bCancel)
    {
        Undo();
        nRet = ULONG_MAX;
    }
    return nRet;
}

// Selection modes

void SwWrtShell::PushMode()
{
    m_pModeStack = new ModeStack( m_pModeStack, m_bIns, m_bExtMode, m_bAddMode, m_bBlockMode );
}

void SwWrtShell::PopMode()
{
    if ( nullptr == m_pModeStack )
        return;

    if ( m_bExtMode && !m_pModeStack->bExt )
        LeaveExtMode();
    if ( m_bAddMode && !m_pModeStack->bAdd )
        LeaveAddMode();
    if ( m_bBlockMode && !m_pModeStack->bBlock )
        LeaveBlockMode();
    m_bIns = m_pModeStack->bIns;

    ModeStack *pTmp = m_pModeStack->pNext;
    delete m_pModeStack;
    m_pModeStack = pTmp;
}

// Two methods for setting cursors: the first maps at the
// eponymous methods in the CursorShell, the second removes
// all selections at first.

long SwWrtShell::SetCursor(const Point *pPt, bool bTextOnly)
{
        // Remove a possibly present selection at the position
        // of the mouseclick

    if(!IsInSelect() && TestCurrPam(*pPt)) {
        ClearMark();
    }

    return SwCursorShell::SetCursor(*pPt, bTextOnly);
}

long SwWrtShell::SetCursorKillSel(const Point *pPt, bool bTextOnly )
{
    SwActContext aActContext(this);
    ResetSelect(pPt,false);
    return SwCursorShell::SetCursor(*pPt, bTextOnly);
}

void SwWrtShell::UnSelectFrame()
{
    // Remove Frame selection with guaranteed invalid position
    Point aPt(LONG_MIN, LONG_MIN);
    SelectObj(aPt);
    SwTransferable::ClearSelection( *this );
}

// Remove of all selections

long SwWrtShell::ResetSelect(const Point *,bool)
{
    if(IsSelFrameMode())
    {
        UnSelectFrame();
        LeaveSelFrameMode();
    }
    else
    {
        //  SwActContext opens an Action -
        //  to avoid problems in the basic process with the
        //  shell switching, GetChgLnk().Call() may be called
        //  after EndAction().
        {
            SwActContext aActContext(this);
            m_bSelWrd = m_bSelLn = false;
            KillPams();
            ClearMark();
            m_fnKillSel = &SwWrtShell::Ignore;
            m_fnSetCursor = &SwWrtShell::SetCursor;
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
    if(m_bInSelect)
        return;
    if(!HasMark())
        SetMark();
    if( m_bBlockMode )
    {
        SwShellCursor* pTmp = getShellCursor( true );
        if( !pTmp->HasMark() )
            pTmp->SetMark();
    }
    m_fnKillSel = &SwWrtShell::Ignore;
    m_fnSetCursor = &SwWrtShell::SetCursor;
    m_bInSelect = true;
    Invalidate();
    SwTransferable::CreateSelection( *this );
}

namespace {

void collectUIInformation(SwShellCursor* pCursor)
{
    EventDescription aDescription;
    OUString aSelStart = OUString::number(pCursor->Start()->nContent.GetIndex());
    OUString aSelEnd = OUString::number(pCursor->End()->nContent.GetIndex());

    aDescription.aParameters = {{"START_POS", aSelStart}, {"END_POS", aSelEnd}};
    aDescription.aAction = "SELECT";
    aDescription.aID = "writer_edit";
    aDescription.aKeyWord = "SwEditWinUIObject";
    aDescription.aParent = "MainWindow";

    UITestLogger::getInstance().logEvent(aDescription);
}

}

// End of a selection process.

void SwWrtShell::EndSelect()
{
    if(m_bInSelect && !m_bExtMode)
    {
        m_bInSelect = false;
        if (m_bAddMode)
        {
            AddLeaveSelect();
        }
        else
        {
            SttLeaveSelect();
            m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
            m_fnKillSel = &SwWrtShell::ResetSelect;
        }
    }
    SwWordCountWrapper *pWrdCnt = static_cast<SwWordCountWrapper*>(GetView().GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId()));
    if (pWrdCnt)
        pWrdCnt->UpdateCounts();

    collectUIInformation(GetCursor_());
}

void SwWrtShell::ExtSelWrd(const Point *pPt, bool )
{
    SwMvContext aMvContext(this);
    if( IsTableMode() )
        return;

    // Bug 66823: actual crsr has in additional mode no selection?
    // Then destroy the actual an go to prev, this will be expand
    if( !HasMark() && GoPrevCursor() )
    {
        bool bHasMark = HasMark(); // that's wrong!
        GoNextCursor();
        if( bHasMark )
        {
            DestroyCursor();
            GoPrevCursor();
        }
    }

    // check the direction of the selection with the new point
    bool bMoveCursor = true, bToTop = false;
    SwCursorShell::SelectWord( &m_aStart );     // select the startword
    SwCursorShell::Push();                    // save the cursor
    SwCursorShell::SetCursor( *pPt );           // and check the direction

    switch( SwCursorShell::CompareCursorStackMkCurrPt())
    {
    case -1:    bToTop = false;     break;
    case 1:     bToTop = true;      break;
    default:    bMoveCursor = false;  break;
    }

    SwCursorShell::Pop(SwCursorShell::PopMode::DeleteCurrent); // restore the saved cursor

    if( bMoveCursor )
    {
        // select to Top but cursor select to Bottom? or
        // select to Bottom but cursor select to Top?       --> swap the cursor
        if( bToTop )
            SwapPam();

        SwCursorShell::Push();                // save cur cursor
        if( SwCursorShell::SelectWord( pPt )) // select the current word
        {
            if( bToTop )
                SwapPam();
            Combine();
        }
        else
        {
            SwCursorShell::Pop(SwCursorShell::PopMode::DeleteCurrent);
            if( bToTop )
                SwapPam();
        }
    }
}

void SwWrtShell::ExtSelLn(const Point *pPt, bool )
{
    SwMvContext aMvContext(this);
    SwCursorShell::SetCursor(*pPt);
    if( IsTableMode() )
        return;

    // Bug 66823: actual crsr has in additional mode no selection?
    // Then destroy the actual an go to prev, this will be expand
    if( !HasMark() && GoPrevCursor() )
    {
        bool bHasMark = HasMark(); // that's wrong!
        GoNextCursor();
        if( bHasMark )
        {
            DestroyCursor();
            GoPrevCursor();
        }
    }

    // if applicable fit the selection to the "Mark"
    bool bToTop = !IsCursorPtAtEnd();
    SwapPam();

    // The "Mark" has to be at the end or the beginning of the line.
    if( bToTop ? !IsEndSentence() : !IsStartSentence() )
    {
        if( bToTop )
        {
            if( !IsEndPara() )
                SwCursorShell::Right(1,CRSR_SKIP_CHARS);
            SwCursorShell::GoEndSentence();
        }
        else
            SwCursorShell::GoStartSentence();
    }
    SwapPam();

    if (bToTop)
        SwCursorShell::GoStartSentence();
    else
        SwCursorShell::GoEndSentence();
}

// Back into the standard mode: no mode, no selections.

void SwWrtShell::EnterStdMode()
{
    if(m_bAddMode)
        LeaveAddMode();
    if(m_bBlockMode)
        LeaveBlockMode();
    m_bBlockMode = false;
    m_bExtMode = false;
    m_bInSelect = false;
    if(IsSelFrameMode())
    {
        UnSelectFrame();
        LeaveSelFrameMode();
    }
    else
    {
        // SwActContext opens and action which has to be
        // closed prior to the call of
        // GetChgLnk().Call()
        SwActContext aActContext(this);
        m_bSelWrd = m_bSelLn = false;
        if( !IsRetainSelection() )
            KillPams();
        ClearMark();
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
        m_fnKillSel = &SwWrtShell::ResetSelect;
    }
    Invalidate();
    SwTransferable::ClearSelection( *this );
}

// Extended Mode

void SwWrtShell::EnterExtMode()
{
    if(m_bBlockMode)
    {
        LeaveBlockMode();
        KillPams();
        ClearMark();
    }
    m_bExtMode = true;
    m_bAddMode = false;
    m_bBlockMode = false;
    SttSelect();
}

void SwWrtShell::LeaveExtMode()
{
    m_bExtMode = false;
    EndSelect();
}

// End of a selection; if the selection is empty,
// ClearMark().

void SwWrtShell::SttLeaveSelect()
{
    if(SwCursorShell::HasSelection() && !IsSelTableCells() && m_bClearMark) {
        return;
    }
    ClearMark();
}

// Leaving of the selection mode in additional mode

void SwWrtShell::AddLeaveSelect()
{
    if(IsTableMode()) LeaveAddMode();
    else if(SwCursorShell::HasSelection())
        CreateCursor();
}

// Additional Mode

void SwWrtShell::EnterAddMode()
{
    if(IsTableMode()) return;
    if(m_bBlockMode)
        LeaveBlockMode();
    m_fnKillSel = &SwWrtShell::Ignore;
    m_fnSetCursor = &SwWrtShell::SetCursor;
    m_bAddMode = true;
    m_bBlockMode = false;
    m_bExtMode = false;
    if(SwCursorShell::HasSelection())
        CreateCursor();
    Invalidate();
}

void SwWrtShell::LeaveAddMode()
{
    m_fnKillSel = &SwWrtShell::ResetSelect;
    m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
    m_bAddMode = false;
    Invalidate();
}

// Block Mode

void SwWrtShell::EnterBlockMode()
{
    m_bBlockMode = false;
    EnterStdMode();
    m_bBlockMode = true;
    CursorToBlockCursor();
    Invalidate();
}

void SwWrtShell::LeaveBlockMode()
{
    m_bBlockMode = false;
    BlockCursorToCursor();
    EndSelect();
    Invalidate();
}

// Insert mode

void SwWrtShell::SetInsMode( bool bOn )
{
    m_bIns = bOn;
    SwCursorShell::SetOverwriteCursor( !m_bIns );
    const SfxBoolItem aTmp( SID_ATTR_INSERT, m_bIns );
    GetView().GetViewFrame()->GetBindings().SetState( aTmp );
    StartAction();
    EndAction();
    Invalidate();
}
//Overwrite mode is incompatible with red-lining
void SwWrtShell::SetRedlineFlagsAndCheckInsMode( RedlineFlags eMode )
{
   SetRedlineFlags( eMode );
   if (IsRedlineOn())
       SetInsMode();
}

// Edit frame

void SwWrtShell::BeginFrameDrag(const Point *pPt, bool bIsShift)
{
    m_fnDrag = &SwFEShell::Drag;
    if(bStartDrag)
    {
        Point aTmp( nStartDragX, nStartDragY );
        SwFEShell::BeginDrag( &aTmp, bIsShift );
    }
    else
        SwFEShell::BeginDrag( pPt, bIsShift );
}

void SwWrtShell::EnterSelFrameMode(const Point *pPos)
{
    if(pPos)
    {
        nStartDragX = pPos->X();
        nStartDragY = pPos->Y();
        bStartDrag = true;
    }
    m_bLayoutMode = true;
    HideCursor();

        // equal call of BeginDrag in the SwFEShell
    m_fnDrag          = &SwWrtShell::BeginFrameDrag;
    m_fnEndDrag       = &SwWrtShell::UpdateLayoutFrame;
    SwBaseShell::SetFrameMode( FLY_DRAG_START, this );
    Invalidate();
}

void SwWrtShell::LeaveSelFrameMode()
{
    m_fnDrag          = &SwWrtShell::BeginDrag;
    m_fnEndDrag       = &SwWrtShell::DefaultEndDrag;
    m_bLayoutMode = false;
    bStartDrag = false;
    Edit();
    SwBaseShell::SetFrameMode( FLY_DRAG_END, this );
    Invalidate();
}

// Description: execute framebound macro

IMPL_LINK( SwWrtShell, ExecFlyMac, const SwFlyFrameFormat*, pFlyFormat, void )
{
    const SwFrameFormat *pFormat = pFlyFormat ? static_cast<const SwFrameFormat*>(pFlyFormat) : GetFlyFrameFormat();
    OSL_ENSURE(pFormat, "no frame format");
    const SvxMacroItem &rFormatMac = pFormat->GetMacro();

    if(rFormatMac.HasMacro(SvMacroItemId::SwObjectSelect))
    {
        const SvxMacro &rMac = rFormatMac.GetMacro(SvMacroItemId::SwObjectSelect);
        if( IsFrameSelected() )
            m_bLayoutMode = true;
        CallChgLnk();
        ExecMacro( rMac );
    }
}

void SwWrtShell::UpdateLayoutFrame(const Point *, bool )
{
        // still a dummy
    SwFEShell::EndDrag();
    m_fnDrag = &SwWrtShell::BeginFrameDrag;
}

// Handler for toggling the modes. Returns back the old mode.

void SwWrtShell::ToggleAddMode()
{
    m_bAddMode ? LeaveAddMode(): EnterAddMode();
    Invalidate();
}

bool SwWrtShell::ToggleBlockMode()
{
    m_bBlockMode ? LeaveBlockMode(): EnterBlockMode();
    Invalidate();
    return !m_bBlockMode;
}

bool SwWrtShell::ToggleExtMode()
{
    m_bExtMode ? LeaveExtMode() : EnterExtMode();
    Invalidate();
    return !m_bExtMode;
}

// Dragging in standard mode (Selecting of content)

void SwWrtShell::BeginDrag(const Point * /*pPt*/, bool )
{
    if(m_bSelWrd)
    {
        m_bInSelect = true;
        if( !IsCursorPtAtEnd() )
            SwapPam();

        m_fnDrag = &SwWrtShell::ExtSelWrd;
        m_fnSetCursor = &SwWrtShell::Ignore;
    }
    else if(m_bSelLn)
    {
        m_bInSelect = true;
        m_fnDrag = &SwWrtShell::ExtSelLn;
        m_fnSetCursor = &SwWrtShell::Ignore;
    }
    else
    {
        m_fnDrag = &SwWrtShell::DefaultDrag;
        SttSelect();
    }
}

void SwWrtShell::DefaultDrag(const Point *, bool )
{
    if( IsSelTableCells() )
        m_aSelTableLink.Call(*this);
}

void SwWrtShell::DefaultEndDrag(const Point * /*pPt*/, bool )
{
    m_fnDrag = &SwWrtShell::BeginDrag;
    if( IsExtSel() )
        LeaveExtSel();

    if( IsSelTableCells() )
        m_aSelTableLink.Call(*this);
    EndSelect();
}

// #i32329# Enhanced table selection
bool SwWrtShell::SelectTableRowCol( const Point& rPt, const Point* pEnd, bool bRowDrag )
{
    SwMvContext aMvContext(this);
    SttSelect();
    if(SelTableRowCol( rPt, pEnd, bRowDrag ))
    {
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
        m_fnKillSel = &SwWrtShell::ResetSelect;
        return true;
    }
    return false;
}

// Description: Selection of a table line or column

void SwWrtShell::SelectTableRow()
{
    if ( SelTableRow() )
    {
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
        m_fnKillSel = &SwWrtShell::ResetSelect;
    }
}

void SwWrtShell::SelectTableCol()
{
    if ( SelTableCol() )
    {
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
        m_fnKillSel = &SwWrtShell::ResetSelect;
    }
}

void SwWrtShell::SelectTableCell()
{
    if ( SelTableBox() )
    {
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
        m_fnKillSel = &SwWrtShell::ResetSelect;
    }
}

// Description: Check if a word selection is present.
//              According to the rules for intelligent cut / paste
//              surrounding spaces are cut out.
// Return:      Delivers the type of the word selection.

int SwWrtShell::IntelligentCut(SelectionType nSelection, bool bCut)
{
        // On multiple selection no intelligent drag and drop
        // there are multiple cursors, since a second was placed
        // already at the target position.
    if( IsAddMode() || !(nSelection & SelectionType::Text) )
        return NO_WORD;

    OUString sText;
    CharClass& rCC = GetAppCharClass();

        // If the first character is no word character,
        // no word selected.
    sal_Unicode cPrev = GetChar(false);
    sal_Unicode cNext = GetChar(true, -1);
    if( !cPrev || !cNext ||
        !rCC.isLetterNumeric( ( sText = OUString(cPrev) ), 0 ) ||
        !rCC.isLetterNumeric( ( sText = OUString(cNext) ), 0 ) )
        return NO_WORD;

    cPrev = GetChar(false, -1);
    cNext = GetChar();

    int cWord = NO_WORD;
        // is a word selected?
    if(!cWord && cPrev && cNext &&
        CH_TXTATR_BREAKWORD != cPrev && CH_TXTATR_INWORD != cPrev &&
        CH_TXTATR_BREAKWORD != cNext && CH_TXTATR_INWORD != cNext &&
        !rCC.isLetterNumeric( ( sText = OUString(cPrev) ), 0 ) &&
        !rCC.isLetterNumeric( ( sText = OUString(cNext) ), 0 ) )
       cWord = WORD_NO_SPACE;

    if(cWord == WORD_NO_SPACE && ' ' == cPrev )
    {
        cWord = WORD_SPACE_BEFORE;
            // delete the space before
        if(bCut)
        {
            Push();
            if(IsCursorPtAtEnd())
                SwapPam();
            ClearMark();
            SetMark();
            SwCursorShell::Left(1,CRSR_SKIP_CHARS);
            SwFEShell::Delete();
            Pop(SwCursorShell::PopMode::DeleteCurrent);
        }
    }
    else if(cWord == WORD_NO_SPACE && cNext == ' ')
    {
        cWord = WORD_SPACE_AFTER;
            // delete the space behind
        if(bCut) {
            Push();
            if(!IsCursorPtAtEnd()) SwapPam();
            ClearMark();
            SetMark();
            SwCursorShell::Right(1,CRSR_SKIP_CHARS);
            SwFEShell::Delete();
            Pop(SwCursorShell::PopMode::DeleteCurrent);
        }
    }
    return cWord;
}

    // jump to the next / previous hyperlink - inside text and also
    // on graphics
void SwWrtShell::SelectNextPrevHyperlink( bool bNext )
{
    StartAction();
    bool bRet = SwCursorShell::SelectNxtPrvHyperlink( bNext );
    if( !bRet )
    {
        // will we have this feature?
        EnterStdMode();
        if( bNext )
            SttEndDoc(true);
        else
            SttEndDoc(false);
        SwCursorShell::SelectNxtPrvHyperlink( bNext );
    }
    EndAction();

    bool bCreateXSelection = false;
    const bool bFrameSelected = IsFrameSelected() || IsObjSelected();
    if( IsSelection() )
    {
        if ( bFrameSelected )
            UnSelectFrame();

        // Set the function pointer for the canceling of the selection
        // set at cursor
        m_fnKillSel = &SwWrtShell::ResetSelect;
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
        bCreateXSelection = true;
    }
    else if( bFrameSelected )
    {
        EnterSelFrameMode();
        bCreateXSelection = true;
    }
    else if( (CNT_GRF | CNT_OLE ) & GetCntType() )
    {
        SelectObj( GetCharRect().Pos() );
        EnterSelFrameMode();
        bCreateXSelection = true;
    }

    if( bCreateXSelection )
        SwTransferable::CreateSelection( *this );
}

// For the preservation of the selection the cursor will be moved left
// after SetMark(), so that the cursor is not moved by inserting text.
// Because a present selection at the CORE page is cleared at the
// current cursor position, the cursor will be pushed on the stack.
// After moving, they will again resummarized.

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
