/*************************************************************************
 *
 *  $RCSfile: select.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 09:25:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
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

#ifdef DEBUG
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#endif

static long nStartDragX = 0, nStartDragY = 0;
static BOOL  bStartDrag = FALSE;

BOOL SwWrtShell::SelNearestWrd()
{
    MV_KONTEXT(this);
    if( !IsInWrd() && !IsEndWrd() && !IsSttWrd() )
        PrvWrd();
    if( IsEndWrd() )
        Left();
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

/*
BOOL SwWrtShell::SelSentence(const Point *pPt, BOOL )
{
    {
        MV_KONTEXT(this);
        ClearMark();
        if(!_BwdSentence()) {
            return FALSE;
        }
        SttSelect();
        if(!_FwdSentence()) {
            EndSelect();
            return FALSE;
        }
    }
    EndSelect();
    return TRUE;
}

*/



void SwWrtShell::SelLine(const Point *pPt, BOOL )
{
    {
        MV_KONTEXT(this);
        ClearMark();
        SwCrsrShell::LeftMargin();
        SttSelect();
        SwCrsrShell::RightMargin();
    }
    EndSelect();
    if(pPt)
        aStart = *pPt;
    bSelLn = TRUE;
    bSelWrd = FALSE;    // SelWord abschalten, sonst geht kein SelLine weiter
}



long SwWrtShell::SelAll()
{
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
            SwCrsrShell::SttDoc();
        }
        SttSelect();
        GoEnd(TRUE, &bMoveTable);
    }
    EndSelect();
    return 1;
}

/*------------------------------------------------------------------------
 Beschreibung:  Textsuche
------------------------------------------------------------------------*/


ULONG SwWrtShell::SearchPattern( const utl::SearchParam& rParam,
                                SwDocPositions eStt, SwDocPositions eEnd,
                                FindRanges eFlags, int bReplace )
{
        // keine Erweiterung bestehender Selektionen
    if(!(eFlags & FND_IN_SEL))
        ClearMark();
    return Find( rParam, eStt, eEnd, eFlags, bReplace );
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

    return Find(pColl? *pColl: GetDfltTxtFmtColl(),
                               eStt,eEnd,eFlags, pReplaceColl);
}

// Suche nach Attributen ----------------------------------------------------



ULONG SwWrtShell::SearchAttr( const SfxItemSet& rFindSet, BOOL bNoColls,
                                SwDocPositions eStart, SwDocPositions eEnde,
                                FindRanges eFlags, const utl::SearchParam* pParam,
                                const SfxItemSet* pReplaceSet )
{
    // Keine Erweiterung bestehender Selektionen
    if (!(eFlags & FND_IN_SEL))
        ClearMark();

    // Suchen
    return Find( rFindSet, bNoColls, eStart, eEnde, eFlags, pParam, pReplaceSet);
}

// ---------- Selektionsmodi ----------



void SwWrtShell::PushMode()
{
    pModeStack = new ModeStack( pModeStack, bIns, bExtMode, bAddMode );
}



void SwWrtShell::PopMode()
{
    if ( 0 == pModeStack )
        return;

    if ( bExtMode && !pModeStack->bExt )
        LeaveExtMode();
    if ( bAddMode && !pModeStack->bAdd )
        LeaveAddMode();
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
    SelectObj(aPt);
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
    fnKillSel = &SwWrtShell::Ignore;
    fnSetCrsr = &SwWrtShell::SetCrsr;
    bInSelect = TRUE;
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
    BOOL bRet = FALSE, bMoveCrsr = TRUE, bToTop;
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
    if( bToTop ? !IsAtRightMargin() : !IsAtLeftMargin() )
    {
        if( bToTop )
        {
            if( !IsEndPara() )
                SwCrsrShell::Right();
            SwCrsrShell::RightMargin();
        }
        else
            SwCrsrShell::LeftMargin();
    }
    SwapPam();

    return bToTop ? SwCrsrShell::LeftMargin() : SwCrsrShell::RightMargin();
}


/*
 * zurueck in den Standard Mode: kein Mode, keine Selektionen.
 */

void SwWrtShell::EnterStdMode()
{
    if(bAddMode)
        LeaveAddMode();
    bExtMode = FALSE;
    bInSelect = FALSE;
    ResetSelect(0,FALSE);
}

/*
 * Extended Mode
 */



void SwWrtShell::EnterExtMode()
{
    bExtMode = TRUE;
    bAddMode = FALSE;
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
    fnLeaveSelect = &SwWrtShell::AddLeaveSelect;
    fnKillSel = &SwWrtShell::Ignore;
    fnSetCrsr = &SwWrtShell::SetCrsr;
    bAddMode = TRUE;
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

// Einfuegemodus



void SwWrtShell::SetInsMode( BOOL bOn )
{
    bIns = bOn;
    const SfxBoolItem aTmp( SID_ATTR_INSERT, bIns );
    GetView().GetViewFrame()->GetBindings().SetState( aTmp );
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



long SwWrtShell::ToggleExtMode()
{
    bExtMode ? LeaveExtMode() : EnterExtMode();
    return !bExtMode;
}
/*
 * Draggen im Standard Modus (Selektieren von Inhalt)
 */



long SwWrtShell::BeginDrag(const Point *pPt, BOOL )
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



long SwWrtShell::EndDrag(const Point *pPt, BOOL )
{
    fnDrag = &SwWrtShell::BeginDrag;
    if( IsExtSel() )
        LeaveExtSel();

    if( IsSelTblCells() )
        aSelTblLink.Call(this);
    EndSelect();
    return 1;
}
/*------------------------------------------------------------------------
 Beschreibung:  Selektion einer Tabellenzeile / Spalte
------------------------------------------------------------------------*/



FASTBOOL SwWrtShell::SelectTableRow()
{
    if ( SelTblRow() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
        return TRUE;
    }
    return FALSE;
}



FASTBOOL SwWrtShell::SelectTableCol()
{
    if ( SelTblCol() )
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
    if( IsAddMode() || !(nSelection & SEL_TXT) )
        return FALSE;

    String sTxt;
    CharClass& rCC = GetAppCharClass();

        // wenn das erste und das letzte Zeichen kein Wortzeichen ist,
        // ist kein Wort selektiert.
    if( !rCC.isLetterNumeric( ( sTxt = GetChar(FALSE)), 0 ) ||
        !rCC.isLetterNumeric( ( sTxt = GetChar(TRUE, -1)), 0 ) )
        return NO_WORD;

    const sal_Unicode cPrev = GetChar(FALSE, -1);
    const sal_Unicode cNext = GetChar(TRUE);

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
            SwCrsrShell::Left();
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
            SwCrsrShell::Right();
            SwFEShell::Delete();
            Pop( FALSE );
        }
    }
    return cWord;
}
/* fuer den Erhalt der Selektion wird nach SetMark() der Cursor
 * nach links bewegt, damit er durch das Einfuegen von Text nicht
 * verschoben wird.  Da auf der CORE-Seite am aktuellen Cursor
 * eine bestehende Selektion aufgehoben wird, wird der Cursor auf
 * den Stack gepushed. Nach dem Verschieben werden sie wieder
 * zusammengefasst. */



#if 0
long SwWrtShell::MoveText(const Point *pPt,BOOL)
{
        // ueber einer bestehenden Selektion kein D&D
    if(ChgCurrPam(*pPt)) {
        LeaveDDMode();
        bTStart = bDD = FALSE;
        return 0;
    }
        // Create-Crsr muss nicht immer sein !!
        // im Add-Mode benutze den letzen, sonst erzeuge einen neuen
    if(!IsAddMode()) SwCrsrShell::CreateCrsr();
    SwCrsrShell::SetCrsr(*pPt);
        // kein D&D auf andere Inhaltsformen als Text
    const int nSelection = GetSelectionType();
    if(SEL_TXT != GetCntType()) {
        if(!IsAddMode()) {
            SwCrsrShell::DestroyCrsr();
            GoPrevCrsr();
        }
        LeaveDDMode();
        bTStart = bDD = FALSE;
        return 0;
    }
        // SSelection vor Start/ EndAction erfragen
    StartUndo(UNDO_INSERT);
    StartAllAction();
    GoPrevCrsr();
    const int cWord = IntelligentCut(nSelection);
    GoNextCrsr();
    const BOOL bInWrd = IsInWrd() || IsEndWrd();
    const BOOL bSttWrd = IsSttWrd();
    SwEditShell::Move();

    // nach dem Move sind alle Crsr geloescht und eine SSelection
    // umschliesst den verschobenen Bereich

    if(bInWrd && ( cWord == WORD_SPACE_AFTER || cWord == WORD_SPACE_BEFORE)) {
        if(!bSttWrd) SwapPam();
        SwEditShell::Insert(' ');
        if(!bSttWrd) SwapPam();
    }
    LeaveDDMode();
    bTStart = bDD = FALSE;
    EndAllAction();
    EndUndo(UNDO_INSERT);
    return 1;
}
#endif

/*************************************************************************

          Source Code Control System - Header

          $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/ui/wrtsh/select.cxx,v 1.3 2000-11-20 09:25:03 jp Exp $

          Source Code Control System - Update

          $Log: not supported by cvs2svn $
          Revision 1.2  2000/10/25 15:36:07  jp
          use CharClass/BreakIt instead of old WordSelection

          Revision 1.1.1.1  2000/09/18 17:14:53  hr
          initial import

          Revision 1.173  2000/09/18 16:06:26  willem.vandorp
          OpenOffice header added.

          Revision 1.172  2000/09/07 15:59:36  os
          change: SFX_DISPATCHER/SFX_BINDINGS removed

          Revision 1.171  2000/07/20 13:17:22  jp
          change old txtatr-character to the two new characters

          Revision 1.170  2000/07/03 17:45:46  jp
          ExtSelWrd - use SelWord

          Revision 1.169  2000/06/30 16:31:43  jp
          new: SelectWord - use the new UNO Api to select one Word

          Revision 1.168  2000/05/19 12:58:31  jp
          use WordSelection class for check chars

          Revision 1.167  2000/05/19 11:00:20  jp
          Changes for Unicode

          Revision 1.166  2000/03/14 13:37:25  jp
          remove unused code

          Revision 1.165  1999/06/11 08:28:04  JP
          Bug #66823#: ExtSelWord/ExtSelLn - destroy current not expanded cursor


      Rev 1.164   11 Jun 1999 10:28:04   JP
   Bug #66823#: ExtSelWord/ExtSelLn - destroy current not expanded cursor

      Rev 1.163   27 Apr 1999 15:29:18   JP
   ExtSelWrd/-Ln: im TableMode nur den Cursor setzen

      Rev 1.162   23 Mar 1999 14:13:22   JP
   Bug #63817#: ExtSelWord/Line - Anfangsselektion beibehalten

      Rev 1.161   25 Nov 1998 16:25:56   OS
   #59467# EnterStdMode: Funktionspointer zuruecksetzen

      Rev 1.160   24 Feb 1998 15:28:14   JP
   Search..: SWPOSDOC entfernt, auf enums umgestellt
*************************************************************************/



