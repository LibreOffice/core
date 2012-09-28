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
#include <swundo.hxx>                   // fuer Undo-Ids
#include <swevent.hxx>
#include <swdtflvr.hxx>
#include <crsskip.hxx>
#include <wordcountdialog.hxx>


namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }

using namespace ::com::sun::star::util;


static long nStartDragX = 0, nStartDragY = 0;
static sal_Bool  bStartDrag = sal_False;

void SwWrtShell::Invalidate()
{
    // to avoid making the slot volatile, invalidate it everytime if something could have been changed
    // this is still much cheaper than asking for the state every 200 ms (and avoid background processing)
    GetView().GetViewFrame()->GetBindings().Invalidate( FN_STAT_SELMODE );
    SwWordCountWrapper *pWrdCnt = (SwWordCountWrapper*)GetView().GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId());
    if (pWrdCnt)
        pWrdCnt->UpdateCounts();
}

sal_Bool SwWrtShell::SelNearestWrd()
{
    SwMvContext aMvContext(this);
    if( !IsInWrd() && !IsEndWrd() && !IsSttWrd() )
        PrvWrd();
    if( IsEndWrd() )
        Left(CRSR_SKIP_CELLS, sal_False, 1, sal_False );
    return SelWrd();
}



sal_Bool SwWrtShell::SelWrd(const Point *pPt, sal_Bool )
{
    sal_Bool bRet;
    {
        SwMvContext aMvContext(this);
        SttSelect();
        bRet = SwCrsrShell::SelectWord( pPt );
    }
    EndSelect();
    if( bRet )
    {
        bSelWrd = sal_True;
        if(pPt)
            aStart = *pPt;
    }
    return bRet;
}

void SwWrtShell::SelSentence(const Point *pPt, sal_Bool )
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
    bSelLn = sal_True;
    bSelWrd = sal_False;    // SelWord abschalten, sonst geht kein SelLine weiter
}

void SwWrtShell::SelPara(const Point *pPt, sal_Bool )
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
    bSelLn = sal_False;
    bSelWrd = sal_False;    // SelWord abschalten, sonst geht kein SelLine weiter
}


long SwWrtShell::SelAll()
{
    const sal_Bool bLockedView = IsViewLocked();
    LockView( sal_True );
    {
        if(bBlockMode)
            LeaveBlockMode();
        SwMvContext aMvContext(this);
        sal_Bool bMoveTable = sal_False;
        SwPosition *pStartPos = 0;
        SwPosition *pEndPos = 0;
        SwShellCrsr* pTmpCrsr = 0;
        if( !HasWholeTabSelection() )
        {
            if ( IsSelection() && IsCrsrPtAtEnd() )
                SwapPam();
            pTmpCrsr = getShellCrsr( false );
            if( pTmpCrsr )
            {
                pStartPos = new SwPosition( *pTmpCrsr->GetPoint() );
                pEndPos = new SwPosition( *pTmpCrsr->GetMark() );
            }
            Push();
            sal_Bool bIsFullSel = !MoveSection( fnSectionCurr, fnSectionStart);
            SwapPam();
            bIsFullSel &= !MoveSection( fnSectionCurr, fnSectionEnd);
            Pop(sal_False);
            GoStart(sal_True, &bMoveTable, sal_False, !bIsFullSel);
        }
        else
        {
            EnterStdMode();
            SttEndDoc(sal_True);
        }
        SttSelect();
        GoEnd(sal_True, &bMoveTable);
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
                if( *pTmpCrsr->GetPoint() < *pEndPos ||
                    ( *pStartPos == *pTmpCrsr->GetMark() &&
                      *pEndPos == *pTmpCrsr->GetPoint() ) )
                    SwCrsrShell::SttEndDoc(sal_False);
            }
            delete pStartPos;
            delete pEndPos;
        }
    }
    EndSelect();
    LockView( bLockedView );
    return 1;
}

/*------------------------------------------------------------------------
 Beschreibung:  Textsuche
------------------------------------------------------------------------*/


sal_uLong SwWrtShell::SearchPattern( const SearchOptions& rSearchOpt, sal_Bool bSearchInNotes,
                                SwDocPositions eStt, SwDocPositions eEnd,
                                FindRanges eFlags, int bReplace )
{
        // keine Erweiterung bestehender Selektionen
    if(!(eFlags & FND_IN_SEL))
        ClearMark();
    sal_Bool bCancel = sal_False;
    sal_uLong nRet = Find( rSearchOpt, bSearchInNotes, eStt, eEnd, bCancel, eFlags, bReplace );
    if(bCancel)
    {
        Undo(1);
        nRet = ULONG_MAX;
    }
    return nRet;
}
/*------------------------------------------------------------------------
 Beschreibung:  Suche nach Vorlagen
------------------------------------------------------------------------*/



sal_uLong SwWrtShell::SearchTempl( const String &rTempl,
                               SwDocPositions eStt, SwDocPositions eEnd,
                               FindRanges eFlags, const String* pReplTempl )
{
        // keine Erweiterung bestehender Selektionen
    if(!(eFlags & FND_IN_SEL))
        ClearMark();
    SwTxtFmtColl *pColl = GetParaStyle(rTempl, SwWrtShell::GETSTYLE_CREATESOME);
    SwTxtFmtColl *pReplaceColl = 0;
    if( pReplTempl )
        pReplaceColl = GetParaStyle(*pReplTempl, SwWrtShell::GETSTYLE_CREATESOME );

    sal_Bool bCancel = sal_False;
    sal_uLong nRet = Find(pColl? *pColl: GetDfltTxtFmtColl(),
                               eStt,eEnd, bCancel, eFlags, pReplaceColl);
    if(bCancel)
    {
        Undo(1);
        nRet = ULONG_MAX;
    }
    return nRet;
}

// Suche nach Attributen ----------------------------------------------------



sal_uLong SwWrtShell::SearchAttr( const SfxItemSet& rFindSet, sal_Bool bNoColls,
                                SwDocPositions eStart, SwDocPositions eEnde,
                                FindRanges eFlags, const SearchOptions* pSearchOpt,
                                const SfxItemSet* pReplaceSet )
{
    // Keine Erweiterung bestehender Selektionen
    if (!(eFlags & FND_IN_SEL))
        ClearMark();

    // Suchen
    sal_Bool bCancel = sal_False;
    sal_uLong nRet = Find( rFindSet, bNoColls, eStart, eEnde, bCancel, eFlags, pSearchOpt, pReplaceSet);

    if(bCancel)
    {
        Undo(1);
        nRet = ULONG_MAX;
    }
    return nRet;
}

// ---------- Selektionsmodi ----------



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

/*
 * Zwei Methoden fuer das Cursorsetzen; die erste mappt auf die
 * gleichnamige Methoden an der CursorShell, die zweite hebt
 * zuerst alle Selektionen auf.
 */



long SwWrtShell::SetCrsr(const Point *pPt, sal_Bool bTextOnly)
{
        /*
        * eine gfs.  bestehende Selektion an der Position des
        * Mausklicks aufheben
        */
    if(!IsInSelect() && ChgCurrPam(*pPt)) {
        ClearMark();
    }

    return SwCrsrShell::SetCrsr(*pPt, bTextOnly);
}


long SwWrtShell::SetCrsrKillSel(const Point *pPt, sal_Bool bTextOnly )
{
    SwActContext aActContext(this);
    ResetSelect(pPt,sal_False);
    return SwCrsrShell::SetCrsr(*pPt, bTextOnly);
}



void SwWrtShell::UnSelectFrm()
{
    // Rahmenselektion aufheben mit garantiert ungueltiger Position
    Point aPt(LONG_MIN, LONG_MIN);
    SelectObj(aPt, 0);
    SwTransferable::ClearSelection( *this );
}

/*
 * Aufheben aller Selektionen
 */



long SwWrtShell::ResetSelect(const Point *,sal_Bool)
{
    if(IsSelFrmMode())
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }
    else
    {
        /*  SwActContext macht eine Action auf -
            um im Basicablauf keine Probleme mit der
            Shellumschaltung zu bekommen, darf
            GetChgLnk().Call() erst nach
            EndAction() gerufen werden.
        */
        {
            SwActContext aActContext(this);
            bSelWrd = bSelLn = sal_False;
            KillPams();
            ClearMark();
            fnKillSel = &SwWrtShell::Ignore;
            fnSetCrsr = &SwWrtShell::SetCrsr;
        }
        /*
            * nach dem Aufheben aller Selektionen koennte ein Update der
            * Attr-Controls notwendig sein.
        */
        GetChgLnk().Call(this);
    }
    Invalidate();
    SwTransferable::ClearSelection( *this );
    return 1;
}



/*
 * tue nichts
 */
long SwWrtShell::Ignore(const Point *, sal_Bool ) {
    return 1;
}

/*
 * Start eines Selektionsvorganges.
 */



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
    bInSelect = sal_True;
    Invalidate();
    SwTransferable::CreateSelection( *this );
}
/*
 * Ende eines Selektionsvorganges.
 */



void SwWrtShell::EndSelect()
{
    if(bInSelect && !bExtMode)
    {
        bInSelect = sal_False;
        if (bAddMode)
        {
            AddLeaveSelect(0, sal_False);
        }
        else
        {
            SttLeaveSelect(0, sal_False);
            fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
            fnKillSel = &SwWrtShell::ResetSelect;
        }
    }
    SwWordCountWrapper *pWrdCnt = (SwWordCountWrapper*)GetView().GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId());
    if (pWrdCnt)
        pWrdCnt->UpdateCounts();
}
/* Methode, um eine bestehende wortweise oder zeilenweise Selektion
 * zu erweitern.
 */

inline sal_Bool operator<(const Point &rP1,const Point &rP2)
{
    return rP1.Y() < rP2.Y() || (rP1.Y() == rP2.Y() && rP1.X() < rP2.X());
}



long SwWrtShell::ExtSelWrd(const Point *pPt, sal_Bool )
{
    SwMvContext aMvContext(this);
    if( IsTableMode() )
        return 1;

    // Bug 66823: actual crsr has in additional mode no selection?
    // Then destroy the actual an go to prev, this will be expand
    if( !HasMark() && GoPrevCrsr() )
    {
        sal_Bool bHasMark = HasMark(); // thats wrong!
        GoNextCrsr();
        if( bHasMark )
        {
            DestroyCrsr();
            GoPrevCrsr();
        }
    }

    // check the direction of the selection with the new point
    sal_Bool bRet = sal_False, bMoveCrsr = sal_True, bToTop = sal_False;
    SwCrsrShell::SelectWord( &aStart );     // select the startword
    SwCrsrShell::Push();                    // save the cursor
    SwCrsrShell::SetCrsr( *pPt );           // and check the direction

    switch( SwCrsrShell::CompareCursor( StackMkCurrPt ))
    {
    case -1:    bToTop = sal_False;     break;
    case 1:     bToTop = sal_True;      break;
    default:    bMoveCrsr = sal_False;  break;
    }

    SwCrsrShell::Pop( sal_False );              // retore the saved cursor

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
            bRet = sal_True;
        }
        else
        {
            SwCrsrShell::Pop( sal_False );
            if( bToTop )
                SwapPam();
        }
    }
    else
        bRet = sal_True;
    return bRet;
}


long SwWrtShell::ExtSelLn(const Point *pPt, sal_Bool )
{
    SwMvContext aMvContext(this);
    SwCrsrShell::SetCrsr(*pPt);
    if( IsTableMode() )
        return 1;

    // Bug 66823: actual crsr has in additional mode no selection?
    // Then destroy the actual an go to prev, this will be expand
    if( !HasMark() && GoPrevCrsr() )
    {
        sal_Bool bHasMark = HasMark(); // thats wrong!
        GoNextCrsr();
        if( bHasMark )
        {
            DestroyCrsr();
            GoPrevCrsr();
        }
    }

    // ggfs. den Mark der Selektion anpassen
    sal_Bool bToTop = !IsCrsrPtAtEnd();
    SwapPam();

    // der "Mark" muss am Zeilenende/-anfang stehen
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

    return bToTop ? SwCrsrShell::GoStartSentence() : SwCrsrShell::GoEndSentence();
}


/*
 * zurueck in den Standard Mode: kein Mode, keine Selektionen.
 */

void SwWrtShell::EnterStdMode()
{
    if(bAddMode)
        LeaveAddMode();
    if(bBlockMode)
        LeaveBlockMode();
    bBlockMode = sal_False;
    bExtMode = sal_False;
    bInSelect = sal_False;
    if(IsSelFrmMode())
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }
    else
    {
        /*  SwActContext opens and action which has to be
            closed prior to the call of
            GetChgLnk().Call()
        */
        {
            SwActContext aActContext(this);
            bSelWrd = bSelLn = sal_False;
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

/*
 * Extended Mode
 */



void SwWrtShell::EnterExtMode()
{
    if(bBlockMode)
    {
        LeaveBlockMode();
        KillPams();
        ClearMark();
    }
    bExtMode = sal_True;
    bAddMode = sal_False;
    bBlockMode = sal_False;
    SttSelect();
}



void SwWrtShell::LeaveExtMode()
{
    bExtMode = sal_False;
    EndSelect();
}
/*
 * Ende einer Selektion; falls die Selektion leer ist,
 * ClearMark().
 */



long SwWrtShell::SttLeaveSelect(const Point *, sal_Bool )
{
    if(SwCrsrShell::HasSelection() && !IsSelTblCells() && bClearMark) {
        return 0;
    }
    ClearMark();
    return 1;
}
/*
 * Verlassen des Selektionsmodus in Additional Mode
 */



long SwWrtShell::AddLeaveSelect(const Point *, sal_Bool )
{
    if(IsTableMode()) LeaveAddMode();
    else if(SwCrsrShell::HasSelection())
        CreateCrsr();
    return 1;
}
/*
 * Additional Mode
 */



void SwWrtShell::EnterAddMode()
{
    if(IsTableMode()) return;
    if(bBlockMode)
        LeaveBlockMode();
    fnKillSel = &SwWrtShell::Ignore;
    fnSetCrsr = &SwWrtShell::SetCrsr;
    bAddMode = sal_True;
    bBlockMode = sal_False;
    bExtMode = sal_False;
    if(SwCrsrShell::HasSelection())
        CreateCrsr();
    Invalidate();
}



void SwWrtShell::LeaveAddMode()
{
    fnKillSel = &SwWrtShell::ResetSelect;
    fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
    bAddMode = sal_False;
    Invalidate();
}

/*
 * Block Mode
 */

void SwWrtShell::EnterBlockMode()
{
    bBlockMode = sal_False;
    EnterStdMode();
    bBlockMode = sal_True;
    CrsrToBlockCrsr();
    Invalidate();
}



void SwWrtShell::LeaveBlockMode()
{
    bBlockMode = sal_False;
    BlockCrsrToCrsr();
    EndSelect();
    Invalidate();
}

// Einfuegemodus



void SwWrtShell::SetInsMode( sal_Bool bOn )
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

/*
 * Rahmen bearbeiten
 */


long SwWrtShell::BeginFrmDrag(const Point *pPt, sal_Bool)
{
    fnDrag = &SwFEShell::Drag;
    if(bStartDrag)
    {
        Point aTmp( nStartDragX, nStartDragY );
        SwFEShell::BeginDrag( &aTmp, sal_False );
    }
    else
        SwFEShell::BeginDrag( pPt, sal_False );
    return 1;
}



void SwWrtShell::EnterSelFrmMode(const Point *pPos)
{
    if(pPos)
    {
        nStartDragX = pPos->X();
        nStartDragY = pPos->Y();
        bStartDrag = sal_True;
    }
    bNoEdit = bLayoutMode = sal_True;
    HideCrsr();

        // gleicher Aufruf von BeginDrag an der SwFEShell
    fnDrag          = &SwWrtShell::BeginFrmDrag;
    fnEndDrag       = &SwWrtShell::UpdateLayoutFrm;
    SwBaseShell::SetFrmMode( FLY_DRAG_START, this );
    Invalidate();
}



void SwWrtShell::LeaveSelFrmMode()
{
    fnDrag          = &SwWrtShell::BeginDrag;
    fnEndDrag       = &SwWrtShell::EndDrag;
    bLayoutMode = sal_False;
    bStartDrag = sal_False;
    Edit();
    SwBaseShell::SetFrmMode( FLY_DRAG_END, this );
    Invalidate();
}
/*------------------------------------------------------------------------
 Beschreibung:  Rahmengebundenes Macro ausfuehren
------------------------------------------------------------------------*/



IMPL_LINK( SwWrtShell, ExecFlyMac, void *, pFlyFmt )
{
    const SwFrmFmt *pFmt = pFlyFmt ? (SwFrmFmt*)pFlyFmt : GetFlyFrmFmt();
    OSL_ENSURE(pFmt, "no frame format");
    const SvxMacroItem &rFmtMac = pFmt->GetMacro();

    if(rFmtMac.HasMacro(SW_EVENT_OBJECT_SELECT))
    {
        const SvxMacro &rMac = rFmtMac.GetMacro(SW_EVENT_OBJECT_SELECT);
        if( IsFrmSelected() )
            bLayoutMode = sal_True;
        CallChgLnk();
        ExecMacro( rMac );
    }
    return 0;
}



long SwWrtShell::UpdateLayoutFrm(const Point *pPt, sal_Bool )
{
        // voerst Dummy
    SwFEShell::EndDrag( pPt, sal_False );
    fnDrag = &SwWrtShell::BeginFrmDrag;
    return 1;
}

/*
 * Handler fuer das Togglen der Modi. Liefern alten Mode zurueck.
 */



long SwWrtShell::ToggleAddMode()
{
    bAddMode ? LeaveAddMode(): EnterAddMode();
    Invalidate();
    return !bAddMode;
}


long SwWrtShell::ToggleBlockMode()
{
    bBlockMode ? LeaveBlockMode(): EnterBlockMode();
    Invalidate();
    return !bBlockMode;
}


long SwWrtShell::ToggleExtMode()
{
    bExtMode ? LeaveExtMode() : EnterExtMode();
    Invalidate();
    return !bExtMode;
}
/*
 * Draggen im Standard Modus (Selektieren von Inhalt)
 */



long SwWrtShell::BeginDrag(const Point * /*pPt*/, sal_Bool )
{
    if(bSelWrd)
    {
        bInSelect = sal_True;
        if( !IsCrsrPtAtEnd() )
            SwapPam();

        fnDrag = &SwWrtShell::ExtSelWrd;
        fnSetCrsr = &SwWrtShell::Ignore;
    }
    else if(bSelLn)
    {
        bInSelect = sal_True;
        fnDrag = &SwWrtShell::ExtSelLn;
        fnSetCrsr = &SwWrtShell::Ignore;
    }
    else
    {
        fnDrag = &SwWrtShell::Drag;
        SttSelect();
    }

    return 1;
}



long SwWrtShell::Drag(const Point *, sal_Bool )
{
    if( IsSelTblCells() )
        aSelTblLink.Call(this);

    return 1;
}



long SwWrtShell::EndDrag(const Point * /*pPt*/, sal_Bool )
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
sal_Bool SwWrtShell::SelectTableRowCol( const Point& rPt, const Point* pEnd, bool bRowDrag )
{
    SwMvContext aMvContext(this);
    SttSelect();
    if(SelTblRowCol( rPt, pEnd, bRowDrag ))
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return sal_True;
    }
    return sal_False;
}

/*------------------------------------------------------------------------
 Beschreibung:  Selektion einer Tabellenzeile / Spalte
------------------------------------------------------------------------*/

sal_Bool SwWrtShell::SelectTableRow()
{
    if ( SelTblRow() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return sal_True;
    }
    return sal_False;
}



sal_Bool SwWrtShell::SelectTableCol()
{
    if ( SelTblCol() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return sal_True;
    }
    return sal_False;
}

sal_Bool SwWrtShell::SelectTableCell()
{
    if ( SelTblBox() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return sal_True;
    }
    return sal_False;
}
/*------------------------------------------------------------------------
 Beschreibung:    Prueft, ob eine Wortselektion vorliegt.
                  Gemaess den Regeln fuer intelligentes Cut / Paste
                  werden umgebende Spaces rausgeschnitten.
 Return:          Liefert Art der Wortselektion zurueck.
------------------------------------------------------------------------*/



int SwWrtShell::IntelligentCut(int nSelection, sal_Bool bCut)
{
        // kein intelligentes Drag and Drop bei Mehrfachselektion
        // es existieren mehrere Cursor, da ein zweiter bereits
        // an die Zielposition gesetzt wurde
    if( IsAddMode() || !(nSelection & nsSelectionType::SEL_TXT) )
        return sal_False;

    String sTxt;
    CharClass& rCC = GetAppCharClass();

        // wenn das erste und das letzte Zeichen kein Wortzeichen ist,
        // ist kein Wort selektiert.
    sal_Unicode cPrev = GetChar(sal_False);
    sal_Unicode cNext = GetChar(sal_True, -1);
    if( !cPrev || !cNext ||
        !rCC.isLetterNumeric( ( sTxt = cPrev), 0 ) ||
        !rCC.isLetterNumeric( ( sTxt = cNext), 0 ) )
        return NO_WORD;

    cPrev = GetChar(sal_False, -1);
    cNext = GetChar(sal_True);

    int cWord = NO_WORD;
        // ist ein Wort selektiert?
    if(!cWord && cPrev && cNext &&
        CH_TXTATR_BREAKWORD != cPrev && CH_TXTATR_INWORD != cPrev &&
        CH_TXTATR_BREAKWORD != cNext && CH_TXTATR_INWORD != cNext &&
        !rCC.isLetterNumeric( ( sTxt = cPrev), 0 ) &&
        !rCC.isLetterNumeric( ( sTxt = cNext), 0 ) )
       cWord = WORD_NO_SPACE;

    if(cWord == WORD_NO_SPACE && ' ' == cPrev )
    {
        cWord = WORD_SPACE_BEFORE;
            // Space davor loeschen
        if(bCut)
        {
            Push();
            if(IsCrsrPtAtEnd())
                SwapPam();
            ClearMark();
            SetMark();
            SwCrsrShell::Left(1,CRSR_SKIP_CHARS);
            SwFEShell::Delete();
            Pop( sal_False );
        }
    }
    else if(cWord == WORD_NO_SPACE && cNext == ' ')
    {
        cWord = WORD_SPACE_AFTER;
            // Space dahinter loeschen
        if(bCut) {
            Push();
            if(!IsCrsrPtAtEnd()) SwapPam();
            ClearMark();
            SetMark();
            SwCrsrShell::Right(1,CRSR_SKIP_CHARS);
            SwFEShell::Delete();
            Pop( sal_False );
        }
    }
    return cWord;
}



    // jump to the next / previous hyperlink - inside text and also
    // on graphics
sal_Bool SwWrtShell::SelectNextPrevHyperlink( sal_Bool bNext )
{
    StartAction();
    sal_Bool bRet = SwCrsrShell::SelectNxtPrvHyperlink( bNext );
    if( !bRet )
    {
        // will we have this feature?
        EnterStdMode();
        if( bNext )
            SttEndDoc(sal_True);
        else
            SttEndDoc(sal_False);
        bRet = SwCrsrShell::SelectNxtPrvHyperlink( bNext );
    }
    EndAction();

    sal_Bool bCreateXSelection = sal_False;
    const sal_Bool bFrmSelected = IsFrmSelected() || IsObjSelected();
    if( IsSelection() )
    {
        if ( bFrmSelected )
            UnSelectFrm();

        // Funktionspointer fuer das Aufheben der Selektion setzen
        // bei Cursor setzen
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        bCreateXSelection = sal_True;
    }
    else if( bFrmSelected )
    {
        EnterSelFrmMode();
        bCreateXSelection = sal_True;
    }
    else if( (CNT_GRF | CNT_OLE ) & GetCntType() )
    {
        SelectObj( GetCharRect().Pos() );
        EnterSelFrmMode();
        bCreateXSelection = sal_True;
    }

    if( bCreateXSelection )
        SwTransferable::CreateSelection( *this );

    return bRet;
}


/* fuer den Erhalt der Selektion wird nach SetMark() der Cursor
 * nach links bewegt, damit er durch das Einfuegen von Text nicht
 * verschoben wird.  Da auf der CORE-Seite am aktuellen Cursor
 * eine bestehende Selektion aufgehoben wird, wird der Cursor auf
 * den Stack gepushed. Nach dem Verschieben werden sie wieder
 * zusammengefasst. */




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
