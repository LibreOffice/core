/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: select.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:10:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <limits.h>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>                   // fuer Undo-Ids
#endif
#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif

#if OSL_DEBUG_LEVEL > 1
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#endif

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }

using namespace ::com::sun::star::util;


static long nStartDragX = 0, nStartDragY = 0;
static BOOL  bStartDrag = FALSE;

BOOL SwWrtShell::SelNearestWrd()
{
    MV_KONTEXT(this);
    if( !IsInWrd() && !IsEndWrd() && !IsSttWrd() )
        PrvWrd();
    if( IsEndWrd() )
        Left(CRSR_SKIP_CELLS, FALSE, 1, FALSE );
    return SelWrd();
}



BOOL SwWrtShell::SelWrd(const Point *pPt, BOOL )
{
    BOOL bRet;
    {
        MV_KONTEXT(this);
        SttSelect();
        bRet = SwCrsrShell::SelectWord( pPt );
    }
    EndSelect();
    if( bRet )
    {
        bSelWrd = TRUE;
        if(pPt)
            aStart = *pPt;
    }
    return bRet;
}

void SwWrtShell::SelSentence(const Point *pPt, BOOL )
{
    {
        MV_KONTEXT(this);
        ClearMark();
        SwCrsrShell::GoStartSentence();
        SttSelect();
        SwCrsrShell::GoEndSentence();
    }
    EndSelect();
    if(pPt)
        aStart = *pPt;
    bSelLn = TRUE;
    bSelWrd = FALSE;    // SelWord abschalten, sonst geht kein SelLine weiter
}

void SwWrtShell::SelPara(const Point *pPt, BOOL )
{
    {
        MV_KONTEXT(this);
        ClearMark();
        SwCrsrShell::MovePara( fnParaCurr, fnParaStart );
        SttSelect();
        SwCrsrShell::MovePara( fnParaCurr, fnParaEnd );
    }
    EndSelect();
    if(pPt)
        aStart = *pPt;
    bSelLn = FALSE;
    bSelWrd = FALSE;    // SelWord abschalten, sonst geht kein SelLine weiter
}


long SwWrtShell::SelAll()
{
    const BOOL bLockedView = IsViewLocked();
    LockView( TRUE );

    {
        MV_KONTEXT(this);
        BOOL bMoveTable = FALSE;
        if( !HasWholeTabSelection() )
        {
            if ( IsSelection() && IsCrsrPtAtEnd() )
                SwapPam();
            Push();
            BOOL bIsFullSel = !MoveSection( fnSectionCurr, fnSectionStart);
            SwapPam();
            bIsFullSel &= !MoveSection( fnSectionCurr, fnSectionEnd);
            Pop(FALSE);
            GoStart(TRUE, &bMoveTable, FALSE, !bIsFullSel);
        }
        else
        {
            EnterStdMode();
            SttEndDoc(TRUE);
        }
        SttSelect();
        GoEnd(TRUE, &bMoveTable);
    }
    EndSelect();

    LockView( bLockedView );

    return 1;
}

/*------------------------------------------------------------------------
 Beschreibung:  Textsuche
------------------------------------------------------------------------*/


ULONG SwWrtShell::SearchPattern( const SearchOptions& rSearchOpt,
                                SwDocPositions eStt, SwDocPositions eEnd,
                                FindRanges eFlags, int bReplace )
{
        // keine Erweiterung bestehender Selektionen
    if(!(eFlags & FND_IN_SEL))
        ClearMark();
    BOOL bCancel = FALSE;
    ULONG nRet = Find( rSearchOpt, eStt, eEnd, bCancel, eFlags, bReplace );
    if(bCancel)
    {
        Undo(UNDO_EMPTY, 1);
        nRet = ULONG_MAX;
    }
    return nRet;
}
/*------------------------------------------------------------------------
 Beschreibung:  Suche nach Vorlagen
------------------------------------------------------------------------*/



ULONG SwWrtShell::SearchTempl( const String &rTempl,
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

    BOOL bCancel = FALSE;
    ULONG nRet = Find(pColl? *pColl: GetDfltTxtFmtColl(),
                               eStt,eEnd, bCancel, eFlags, pReplaceColl);
    if(bCancel)
    {
        Undo(UNDO_EMPTY, 1);
        nRet = ULONG_MAX;
    }
    return nRet;
}

// Suche nach Attributen ----------------------------------------------------



ULONG SwWrtShell::SearchAttr( const SfxItemSet& rFindSet, BOOL bNoColls,
                                SwDocPositions eStart, SwDocPositions eEnde,
                                FindRanges eFlags, const SearchOptions* pSearchOpt,
                                const SfxItemSet* pReplaceSet )
{
    // Keine Erweiterung bestehender Selektionen
    if (!(eFlags & FND_IN_SEL))
        ClearMark();

    // Suchen
    BOOL bCancel = FALSE;
    ULONG nRet = Find( rFindSet, bNoColls, eStart, eEnde, bCancel, eFlags, pSearchOpt, pReplaceSet);

    if(bCancel)
    {
        Undo(UNDO_EMPTY, 1);
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



long SwWrtShell::SetCrsr(const Point *pPt, BOOL bTextOnly)
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


long SwWrtShell::SetCrsrKillSel(const Point *pPt, BOOL bTextOnly )
{
    ACT_KONTEXT(this);
    ResetSelect(pPt,FALSE);
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



long SwWrtShell::ResetSelect(const Point *,BOOL)
{
    if(IsSelFrmMode())
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }
    else
    {
        /*  ACT_KONTEXT() macht eine Action auf -
            um im Basicablauf keine Probleme mit der
            Shellumschaltung zu bekommen, darf
            GetChgLnk().Call() erst nach
            EndAction() gerufen werden.
        */
        {
            ACT_KONTEXT(this);
            bSelWrd = bSelLn = FALSE;
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
    SwTransferable::ClearSelection( *this );
    return 1;
}



/*
 * tue nichts
 */
long SwWrtShell::Ignore(const Point *, BOOL ) {
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
    bInSelect = TRUE;
    SwTransferable::CreateSelection( *this );
}
/*
 * Ende eines Selektionsvorganges.
 */



void SwWrtShell::EndSelect()
{
    if(!bInSelect || bExtMode)
        return;
    bInSelect = FALSE;
    (this->*fnLeaveSelect)(0,FALSE);
    if(!bAddMode) {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
    }
}
/* Methode, um eine bestehende wortweise oder zeilenweise Selektion
 * zu erweitern.
 */

inline BOOL operator<(const Point &rP1,const Point &rP2)
{
    return rP1.Y() < rP2.Y() || (rP1.Y() == rP2.Y() && rP1.X() < rP2.X());
}



long SwWrtShell::ExtSelWrd(const Point *pPt, BOOL )
{
    MV_KONTEXT(this);
    if( IsTableMode() )
        return 1;

    // Bug 66823: actual crsr has in additional mode no selection?
    // Then destroy the actual an go to prev, this will be expand
    if( !HasMark() && GoPrevCrsr() )
    {
        BOOL bHasMark = HasMark(); // thats wrong!
        GoNextCrsr();
        if( bHasMark )
        {
            DestroyCrsr();
            GoPrevCrsr();
        }
    }

    // check the direction of the selection with the new point
    BOOL bRet = FALSE, bMoveCrsr = TRUE, bToTop = FALSE;
    SwCrsrShell::SelectWord( &aStart );     // select the startword
    SwCrsrShell::Push();                    // save the cursor
    SwCrsrShell::SetCrsr( *pPt );           // and check the direction

    switch( SwCrsrShell::CompareCursor( StackMkCurrPt ))
    {
    case -1:    bToTop = FALSE;     break;
    case 1:     bToTop = TRUE;      break;
    default:    bMoveCrsr = FALSE;  break;
    }

    SwCrsrShell::Pop( FALSE );              // retore the saved cursor

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
            bRet = TRUE;
        }
        else
        {
            SwCrsrShell::Pop( FALSE );
            if( bToTop )
                SwapPam();
        }
    }
    else
        bRet = TRUE;
    return bRet;
}


long SwWrtShell::ExtSelLn(const Point *pPt, BOOL )
{
    MV_KONTEXT(this);
    SwCrsrShell::SetCrsr(*pPt);
    if( IsTableMode() )
        return 1;

    // Bug 66823: actual crsr has in additional mode no selection?
    // Then destroy the actual an go to prev, this will be expand
    if( !HasMark() && GoPrevCrsr() )
    {
        BOOL bHasMark = HasMark(); // thats wrong!
        GoNextCrsr();
        if( bHasMark )
        {
            DestroyCrsr();
            GoPrevCrsr();
        }
    }

    // ggfs. den Mark der Selektion anpassen
    BOOL bToTop = !IsCrsrPtAtEnd();
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
    bBlockMode = FALSE;
    bExtMode = FALSE;
    bInSelect = FALSE;
    if(IsSelFrmMode())
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }
    else
    {
        /*  ACT_KONTEXT() opens and action which has to be
            closed prior to the call of
            GetChgLnk().Call()
        */
        {
            ACT_KONTEXT(this);
            bSelWrd = bSelLn = FALSE;
            if( !IsRetainSelection() )
                KillPams();
            ClearMark();
            fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
            fnKillSel = &SwWrtShell::ResetSelect;
        }
    }
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
    bExtMode = TRUE;
    bAddMode = FALSE;
    bBlockMode = FALSE;
    SttSelect();
}



void SwWrtShell::LeaveExtMode()
{
    bExtMode = FALSE;
    EndSelect();
}
/*
 * Ende einer Selektion; falls die Selektion leer ist,
 * ClearMark().
 */



long SwWrtShell::SttLeaveSelect(const Point *, BOOL )
{
    if(SwCrsrShell::HasSelection() && !IsSelTblCells() && bClearMark) {
        return 0;
    }
//  if( IsSelTblCells() ) aSelTblLink.Call(this);
    ClearMark();
    return 1;
}
/*
 * Verlassen des Selektionsmodus in Additional Mode
 */



long SwWrtShell::AddLeaveSelect(const Point *, BOOL )
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
    fnLeaveSelect = &SwWrtShell::AddLeaveSelect;
    fnKillSel = &SwWrtShell::Ignore;
    fnSetCrsr = &SwWrtShell::SetCrsr;
    bAddMode = TRUE;
    bBlockMode = FALSE;
    bExtMode = FALSE;
    if(SwCrsrShell::HasSelection())
        CreateCrsr();
}



void SwWrtShell::LeaveAddMode()
{
    fnLeaveSelect = &SwWrtShell::SttLeaveSelect;
    fnKillSel = &SwWrtShell::ResetSelect;
    fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
    bAddMode = FALSE;
}

/*
 * Block Mode
 */

void SwWrtShell::EnterBlockMode()
{
    bBlockMode = FALSE;
    EnterStdMode();
    bBlockMode = TRUE;
    CrsrToBlockCrsr();
}



void SwWrtShell::LeaveBlockMode()
{
    bBlockMode = FALSE;
    BlockCrsrToCrsr();
    EndSelect();
}

// Einfuegemodus



void SwWrtShell::SetInsMode( BOOL bOn )
{
    bIns = bOn;
    SwCrsrShell::SetOverwriteCrsr( !bIns );
    const SfxBoolItem aTmp( SID_ATTR_INSERT, bIns );
    GetView().GetViewFrame()->GetBindings().SetState( aTmp );
    StartAction();
    EndAction();
}

/*
 * Rahmen bearbeiten
 */


long SwWrtShell::BeginFrmDrag(const Point *pPt, BOOL)
{
    fnDrag = &SwFEShell::Drag;
    if(bStartDrag)
    {
        Point aTmp( nStartDragX, nStartDragY );
        SwFEShell::BeginDrag( &aTmp, FALSE );
    }
    else
        SwFEShell::BeginDrag( pPt, FALSE );
    return 1;
}



void SwWrtShell::EnterSelFrmMode(const Point *pPos)
{
    if(pPos)
    {
        nStartDragX = pPos->X();
        nStartDragY = pPos->Y();
        bStartDrag = TRUE;
    }
    bNoEdit = bLayoutMode = TRUE;
    HideCrsr();

        // gleicher Aufruf von BeginDrag an der SwFEShell
    fnDrag          = &SwWrtShell::BeginFrmDrag;
    fnEndDrag       = &SwWrtShell::UpdateLayoutFrm;
    SwBaseShell::SetFrmMode( FLY_DRAG_START, this );
}



void SwWrtShell::LeaveSelFrmMode()
{
    fnDrag          = &SwWrtShell::BeginDrag;
    fnEndDrag       = &SwWrtShell::EndDrag;
    bLayoutMode = FALSE;
    bStartDrag = FALSE;
    Edit();
    SwBaseShell::SetFrmMode( FLY_DRAG_END, this );
}
/*------------------------------------------------------------------------
 Beschreibung:  Rahmengebundenes Macro ausfuehren
------------------------------------------------------------------------*/



IMPL_LINK( SwWrtShell, ExecFlyMac, void *, pFlyFmt )
{
    const SwFrmFmt *pFmt = pFlyFmt ? (SwFrmFmt*)pFlyFmt : GetFlyFrmFmt();
    ASSERT(pFmt, kein FrameFormat.);
    const SvxMacroItem &rFmtMac = pFmt->GetMacro();

    if(rFmtMac.HasMacro(SW_EVENT_OBJECT_SELECT))
    {
        const SvxMacro &rMac = rFmtMac.GetMacro(SW_EVENT_OBJECT_SELECT);
        if( IsFrmSelected() )
            bLayoutMode = TRUE;
        CallChgLnk();
        ExecMacro( rMac );
    }
    return 0;
}



long SwWrtShell::UpdateLayoutFrm(const Point *pPt, BOOL )
{
        // voerst Dummy
    SwFEShell::EndDrag( pPt, FALSE );
    fnDrag = &SwWrtShell::BeginFrmDrag;
    return 1;
}

/*
 * Handler fuer das Togglen der Modi. Liefern alten Mode zurueck.
 */



long SwWrtShell::ToggleAddMode()
{
    bAddMode ? LeaveAddMode(): EnterAddMode();
    return !bAddMode;
}


long SwWrtShell::ToggleBlockMode()
{
    bBlockMode ? LeaveBlockMode(): EnterBlockMode();
    return !bBlockMode;
}


long SwWrtShell::ToggleExtMode()
{
    bExtMode ? LeaveExtMode() : EnterExtMode();
    return !bExtMode;
}
/*
 * Draggen im Standard Modus (Selektieren von Inhalt)
 */



long SwWrtShell::BeginDrag(const Point * /*pPt*/, BOOL )
{
    if(bSelWrd)
    {
        bInSelect = TRUE;
        if( !IsCrsrPtAtEnd() )
            SwapPam();

        fnDrag = &SwWrtShell::ExtSelWrd;
        fnSetCrsr = &SwWrtShell::Ignore;
    }
    else if(bSelLn)
    {
        bInSelect = TRUE;
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



long SwWrtShell::Drag(const Point *, BOOL )
{
    if( IsSelTblCells() )
        aSelTblLink.Call(this);

    return 1;
}



long SwWrtShell::EndDrag(const Point * /*pPt*/, BOOL )
{
    fnDrag = &SwWrtShell::BeginDrag;
    if( IsExtSel() )
        LeaveExtSel();

    if( IsSelTblCells() )
        aSelTblLink.Call(this);
    EndSelect();
    return 1;
}

// --> FME 2004-07-30 #i32329# Enhanced table selection
BOOL SwWrtShell::SelectTableRowCol( const Point& rPt, const Point* pEnd, bool bRowDrag )
{
    MV_KONTEXT(this);
    SttSelect();
    if(SelTblRowCol( rPt, pEnd, bRowDrag ))
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return TRUE;
    }
    return FALSE;
}
// <--

/*------------------------------------------------------------------------
 Beschreibung:  Selektion einer Tabellenzeile / Spalte
------------------------------------------------------------------------*/

BOOL SwWrtShell::SelectTableRow()
{
    if ( SelTblRow() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return TRUE;
    }
    return FALSE;
}



BOOL SwWrtShell::SelectTableCol()
{
    if ( SelTblCol() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return TRUE;
    }
    return FALSE;
}

BOOL SwWrtShell::SelectTableCell()
{
    if ( SelTblBox() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return TRUE;
    }
    return FALSE;
}
/*------------------------------------------------------------------------
 Beschreibung:    Prueft, ob eine Wortselektion vorliegt.
                  Gemaess den Regeln fuer intelligentes Cut / Paste
                  werden umgebende Spaces rausgeschnitten.
 Return:          Liefert Art der Wortselektion zurueck.
------------------------------------------------------------------------*/



int SwWrtShell::IntelligentCut(int nSelection, BOOL bCut)
{
        // kein intelligentes Drag and Drop bei Mehrfachselektion
        // es existieren mehrere Cursor, da ein zweiter bereits
        // an die Zielposition gesetzt wurde
    if( IsAddMode() || !(nSelection & nsSelectionType::SEL_TXT) )
        return FALSE;

    String sTxt;
    CharClass& rCC = GetAppCharClass();

        // wenn das erste und das letzte Zeichen kein Wortzeichen ist,
        // ist kein Wort selektiert.
    sal_Unicode cPrev = GetChar(FALSE);
    sal_Unicode cNext = GetChar(TRUE, -1);
    if( !cPrev || !cNext ||
        !rCC.isLetterNumeric( ( sTxt = cPrev), 0 ) ||
        !rCC.isLetterNumeric( ( sTxt = cNext), 0 ) )
        return NO_WORD;

    cPrev = GetChar(FALSE, -1);
    cNext = GetChar(TRUE);

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
            Pop( FALSE );
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
            Pop( FALSE );
        }
    }
    return cWord;
}



    // jump to the next / previous hyperlink - inside text and also
    // on graphics
BOOL SwWrtShell::SelectNextPrevHyperlink( BOOL bNext )
{
    StartAction();
    BOOL bRet = SwCrsrShell::SelectNxtPrvHyperlink( bNext );
    if( !bRet )
    {
        // will we have this feature?
        EnterStdMode();
        if( bNext )
            SttEndDoc(TRUE);
        else
            SttEndDoc(FALSE);
        bRet = SwCrsrShell::SelectNxtPrvHyperlink( bNext );
    }
    EndAction();

    BOOL bCreateXSelection = FALSE;
    const BOOL bFrmSelected = IsFrmSelected() || IsObjSelected();
    if( IsSelection() )
    {
        if ( bFrmSelected )
            UnSelectFrm();

        // Funktionspointer fuer das Aufheben der Selektion setzen
        // bei Cursor setzen
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        bCreateXSelection = TRUE;
    }
    else if( bFrmSelected )
    {
        EnterSelFrmMode();
        bCreateXSelection = TRUE;
    }
    else if( (CNT_GRF | CNT_OLE ) & GetCntType() )
    {
        SelectObj( GetCharRect().Pos() );
        EnterSelFrmMode();
        bCreateXSelection = TRUE;
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




