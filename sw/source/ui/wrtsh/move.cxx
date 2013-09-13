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

#include <sfx2/bindings.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <crsskip.hxx>

/**
   Always:
    -   Reset of the cursor stack
    -   retrigger timer
    -   if applicable: GCAttr

    on selection
    -   SttSelect()

    else
    -   EndSelect()
 */

const long nReadOnlyScrollOfst = 10;

class ShellMoveCrsr
{
    SwWrtShell* pSh;
    bool bAct;
public:
    inline ShellMoveCrsr( SwWrtShell* pWrtSh, sal_Bool bSel )
    {
        bAct = !pWrtSh->ActionPend() && (pWrtSh->GetFrmType(0,sal_False) & FRMTYPE_FLY_ANY);
        ( pSh = pWrtSh )->MoveCrsr( sal_Bool(bSel) );
        pWrtSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_HYPERLINK_GETLINK);
    }
    inline ~ShellMoveCrsr()
    {
        if( bAct )
        {
            // The action is used for scrolling in "single paragraph"
            // frames with fixed height.
            pSh->StartAllAction();
            pSh->EndAllAction();
        }
    }
};

void SwWrtShell::MoveCrsr( sal_Bool bWithSelect )
{
    ResetCursorStack();
    if ( IsGCAttr() )
    {
        GCAttr();
        ClearGCAttr();
    }
    if ( bWithSelect )
        SttSelect();
    else
    {
        EndSelect();
        (this->*fnKillSel)( 0, sal_False );
    }
}

sal_Bool SwWrtShell::SimpleMove( FNSimpleMove FnSimpleMove, sal_Bool bSelect )
{
    sal_Bool nRet;
    if( bSelect )
    {
        SttCrsrMove();
        MoveCrsr( sal_True );
        nRet = (this->*FnSimpleMove)();
        EndCrsrMove();
    }
    else if( 0 != ( nRet = (this->*FnSimpleMove)() ) )
        MoveCrsr( sal_False );
    return nRet;
}

sal_Bool SwWrtShell::Left( sal_uInt16 nMode, sal_Bool bSelect,
                            sal_uInt16 nCount, sal_Bool bBasicCall, sal_Bool bVisual )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly()  && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() -= VisArea().Width() * nReadOnlyScrollOfst / 100;
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Left( nCount, nMode, bVisual );
    }
}

sal_Bool SwWrtShell::Right( sal_uInt16 nMode, sal_Bool bSelect,
                            sal_uInt16 nCount, sal_Bool bBasicCall, sal_Bool bVisual )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() && !GetViewOptions()->IsSelectionInReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() += VisArea().Width() * nReadOnlyScrollOfst / 100;
        aTmp.X() = rView.SetHScrollMax( aTmp.X() );
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Right( nCount, nMode, bVisual );
    }
}

sal_Bool SwWrtShell::Up( sal_Bool bSelect, sal_uInt16 nCount, sal_Bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly()  && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.Y() -= VisArea().Height() * nReadOnlyScrollOfst / 100;
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Up( nCount );
    }
}

sal_Bool SwWrtShell::Down( sal_Bool bSelect, sal_uInt16 nCount, sal_Bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.Y() += VisArea().Height() * nReadOnlyScrollOfst / 100;
        aTmp.Y() = rView.SetVScrollMax( aTmp.Y() );
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Down( nCount );
    }
}

sal_Bool SwWrtShell::LeftMargin( sal_Bool bSelect, sal_Bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() = DOCUMENTBORDER;
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::LeftMargin();
    }
}

sal_Bool SwWrtShell::RightMargin( sal_Bool bSelect, sal_Bool bBasicCall  )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() = GetDocSize().Width() - VisArea().Width() + DOCUMENTBORDER;
        if( DOCUMENTBORDER > aTmp.X() )
            aTmp.X() = DOCUMENTBORDER;
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::RightMargin(bBasicCall);
    }
}

sal_Bool SwWrtShell::GoStart( sal_Bool bKeepArea, sal_Bool *pMoveTable,
                            sal_Bool bSelect, sal_Bool bDontMoveRegion )
{
    if ( IsCrsrInTbl() )
    {
        const sal_Bool bBoxSelection = HasBoxSelection();
        if( !bBlockMode )
        {
            if ( !bSelect )
                EnterStdMode();
            else
                SttSelect();
        }
            // Table cell ?
        if ( !bBoxSelection && (MoveSection( fnSectionCurr, fnSectionStart)
                || bDontMoveRegion))
        {
            if ( pMoveTable )
                *pMoveTable = sal_False;
            return sal_True;
        }
        if( MoveTable( fnTableCurr, fnTableStart ) || bDontMoveRegion )
        {
            if ( pMoveTable )
                *pMoveTable = sal_True;
            return sal_True;
        }
        else if( bBoxSelection && pMoveTable )
        {
            // JP 09.01.96: We have a box selection (or a empty cell)
            //              and we want select (pMoveTable will be
            //              set in SelAll). Then the table must not
            //              be left, otherwise there is no selection
            //              of the entire table possible!
            *pMoveTable = sal_True;
            return sal_True;
        }
    }

    if( !bBlockMode )
    {
        if ( !bSelect )
            EnterStdMode();
        else
            SttSelect();
    }
    const sal_uInt16 nFrmType = GetFrmType(0,sal_False);
    if ( FRMTYPE_FLY_ANY & nFrmType )
    {
        if( MoveSection( fnSectionCurr, fnSectionStart ) )
            return sal_True;
        else if ( FRMTYPE_FLY_FREE & nFrmType || bDontMoveRegion )
            return sal_False;
    }
    if(( FRMTYPE_HEADER | FRMTYPE_FOOTER | FRMTYPE_FOOTNOTE ) & nFrmType )
    {
        if ( MoveSection( fnSectionCurr, fnSectionStart ) )
            return sal_True;
        else if ( bKeepArea )
            return sal_True;
    }
    // Regions ???
    return SwCrsrShell::MoveRegion( fnRegionCurrAndSkip, fnRegionStart ) ||
           SwCrsrShell::SttEndDoc(sal_True);
}

sal_Bool SwWrtShell::GoEnd(sal_Bool bKeepArea, sal_Bool *pMoveTable)
{
    if ( pMoveTable && *pMoveTable )
        return MoveTable( fnTableCurr, fnTableEnd );

    if ( IsCrsrInTbl() )
    {
        if ( MoveSection( fnSectionCurr, fnSectionEnd ) ||
             MoveTable( fnTableCurr, fnTableEnd ) )
            return sal_True;
    }
    else
    {
        const sal_uInt16 nFrmType = GetFrmType(0,sal_False);
        if ( FRMTYPE_FLY_ANY & nFrmType )
        {
            if ( MoveSection( fnSectionCurr, fnSectionEnd ) )
                return sal_True;
            else if ( FRMTYPE_FLY_FREE & nFrmType )
                return sal_False;
        }
        if(( FRMTYPE_HEADER | FRMTYPE_FOOTER | FRMTYPE_FOOTNOTE ) & nFrmType )
        {
            if ( MoveSection( fnSectionCurr, fnSectionEnd) )
                return sal_True;
            else if ( bKeepArea )
                return sal_True;
        }
    }
    // Regions ???
    return SwCrsrShell::MoveRegion( fnRegionCurrAndSkip, fnRegionEnd ) ||
           SwCrsrShell::SttEndDoc(sal_False);
}

sal_Bool SwWrtShell::SttDoc( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return GoStart(sal_False, 0, bSelect );
}

sal_Bool SwWrtShell::EndDoc( sal_Bool bSelect)
{
    ShellMoveCrsr aTmp( this, bSelect );
    return GoEnd();
}

sal_Bool SwWrtShell::SttNxtPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageStart );
}

sal_Bool SwWrtShell::SttPrvPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageStart );
}

sal_Bool SwWrtShell::EndNxtPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageEnd );
}

sal_Bool SwWrtShell::EndPrvPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageEnd );
}

sal_Bool SwWrtShell::SttPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageStart );
}

sal_Bool SwWrtShell::EndPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageEnd );
}

sal_Bool SwWrtShell::SttPara( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePara( fnParaCurr, fnParaStart );
}

sal_Bool SwWrtShell::EndPara( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePara(fnParaCurr,fnParaEnd);
}

// Column-by-jumping.
// SSelection with or without
// returns success or failure

sal_Bool SwWrtShell::StartOfColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnStart);
}

sal_Bool SwWrtShell::EndOfColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnEnd);
}

sal_Bool SwWrtShell::StartOfNextColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn( fnColumnNext, fnColumnStart);
}

sal_Bool SwWrtShell::EndOfNextColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnNext, fnColumnEnd);
}

sal_Bool SwWrtShell::StartOfPrevColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnPrev, fnColumnStart);
}

sal_Bool SwWrtShell::EndOfPrevColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnPrev, fnColumnEnd);
}

sal_Bool SwWrtShell::PushCrsr(SwTwips lOffset, sal_Bool bSelect)
{
    sal_Bool bDiff = sal_False;
    SwRect aOldRect( GetCharRect() ), aTmpArea( VisArea() );

    // bDestOnStack indicates if I could not set the coursor at the current
    // position, because in this region is no content.
    if( !bDestOnStack )
    {
        Point aPt( aOldRect.Center() );

        if( !IsCrsrVisible() )
            // set CrsrPos to top-/bottom left pos. So the pagescroll is not
            // be dependent on the current cursor, but on the visarea.
            aPt.Y() = aTmpArea.Top() + aTmpArea.Height() / 2;

        aPt.Y() += lOffset;
        aDest = GetCntntPos(aPt,lOffset > 0);
        aDest.X() = aPt.X();
        bDestOnStack = true;
    }

    // If we had a frame selection, it must be removed after the fnSetCrsr
    // and we have to remember the position on the stack to return to it later.
    sal_Bool bIsFrmSel = sal_False;


    //Target position is now within the viewable region -->
    //Place the cursor at the target position; remember that no target
    //position is longer on the stack.
    //The new visible region is to be determined beforehand.
    aTmpArea.Pos().Y() += lOffset;
    if( aTmpArea.IsInside(aDest) )
    {
        if( bSelect )
            SttSelect();
        else
            EndSelect();

        bIsFrmSel = IsFrmSelected();
        bool bIsObjSel = 0 != IsObjSelected();

        // unselect frame
        if( bIsFrmSel || bIsObjSel )
        {
            UnSelectFrm();
            LeaveSelFrmMode();
            if ( bIsObjSel )
            {
                GetView().SetDrawFuncPtr( NULL );
                GetView().LeaveDrawCreate();
            }

            CallChgLnk();
        }

        (this->*fnSetCrsr)( &aDest, sal_True );

        bDiff = aOldRect != GetCharRect();

        if( bIsFrmSel )
        {
            // In frames take only the upper corner
            // so that it can be re-selected.
            aOldRect.SSize( 5, 5 );
        }

            // reset Dest. SPoint Flags
        bDestOnStack = false;
    }

    // Position into the stack; bDiff indicates if there is a
    // difference between the old and the new cursor position.
    pCrsrStack = new CrsrStack( bDiff, bIsFrmSel, aOldRect.Center(),
                                lOffset, pCrsrStack );
    return !bDestOnStack && bDiff;
}



sal_Bool SwWrtShell::PopCrsr(sal_Bool bUpdate, sal_Bool bSelect)
{
    if( 0 == pCrsrStack)
        return sal_False;

    const sal_Bool bValidPos = pCrsrStack->bValidCurPos;
    if( bUpdate && bValidPos )
    {
            // If a predecessor is on the stack,
            // use the flag for a valid position.
        SwRect aTmpArea(VisArea());
        aTmpArea.Pos().Y() -= pCrsrStack->lOffset;
        if( aTmpArea.IsInside( pCrsrStack->aDocPos ) )
        {
            if( bSelect )
                SttSelect();
            else
                EndSelect();

            (this->*fnSetCrsr)(&pCrsrStack->aDocPos, !pCrsrStack->bIsFrmSel);
            if( pCrsrStack->bIsFrmSel && IsObjSelectable(pCrsrStack->aDocPos))
            {
                HideCrsr();
                SelectObj( pCrsrStack->aDocPos );
                EnterSelFrmMode( &pCrsrStack->aDocPos );
            }
        }
            // If a discrepancy between the visible range and the
            // remembered cursor position occurs, all of the remembered
            // positions are thrown away.
        else
        {
            _ResetCursorStack();
            return sal_False;
        }
    }
    CrsrStack *pTmp = pCrsrStack;
    pCrsrStack = pCrsrStack->pNext;
    delete pTmp;
    if( 0 == pCrsrStack )
    {
        ePageMove = MV_NO;
        bDestOnStack = false;
    }
    return bValidPos;
}

// Reset of all pushed cursor positions; these will
// not be displayed ( --> No Start-/EndAction!!)

void SwWrtShell::_ResetCursorStack()
{
    CrsrStack *pTmp = pCrsrStack;
    while(pCrsrStack)
    {
        pTmp = pCrsrStack->pNext;
        delete pCrsrStack;
        pCrsrStack = pTmp;
    }
    ePageMove = MV_NO;
    bDestOnStack = false;
}
/**
    if no stack exists --> cancel selection
    if stack && change of direction
        --> pop cursor and return
    else
        --> push cursor
            transpose cursor
*/



sal_Bool SwWrtShell::PageCrsr(SwTwips lOffset, sal_Bool bSelect)
{
    // Do nothing if an offset of 0 was indicated
    if(!lOffset) return sal_False;
        // Was once used to force a reformat of the layout.
        // This has not work that way, because the cursor was not set
        // because this does not happen within a
        // Start-/EndActionParentheses.
        // Because only ViewShell::EndAction() is called at the end,
        // no updating of the display of the cursor position takes place.
        // The CrsrShell-Actionparentheses cannot be used, because it
        // always leads to displaying the cursor, thus also,
        // if after the scroll scrolled in a region without a valid postition.
        // ViewShell::StartAction();
    PageMove eDir = lOffset > 0? MV_PAGE_DOWN: MV_PAGE_UP;
        // Change of direction and stack present
    if( eDir != ePageMove && ePageMove != MV_NO && PopCrsr( sal_True, bSelect ))
        return sal_True;

    const sal_Bool bRet = PushCrsr(lOffset, bSelect);
    ePageMove = eDir;
    return bRet;
}

sal_Bool SwWrtShell::GotoPage(sal_uInt16 nPage, sal_Bool bRecord)
{
    ShellMoveCrsr aTmp( this, sal_False);
    if( SwCrsrShell::GotoPage(nPage) && bRecord)
    {
        if(IsSelFrmMode())
        {
            UnSelectFrm();
            LeaveSelFrmMode();
        }
        return sal_True;
    }
    return sal_False;
}

sal_Bool SwWrtShell::GotoMark( const ::sw::mark::IMark* const pMark, sal_Bool bSelect, sal_Bool bStart )
{
    ShellMoveCrsr aTmp( this, bSelect );
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoMark( pMark, bStart );
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

sal_Bool SwWrtShell::GotoFly( const String& rName, FlyCntType eType, sal_Bool bSelFrame )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwFEShell::GotoFly(rName, eType, bSelFrame);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoINetAttr( const SwTxtINetFmt& rAttr )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoINetAttr(rAttr);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

void SwWrtShell::GotoOutline( sal_uInt16 nIdx )
{
    addCurrentPosition();
    SwCrsrShell::GotoOutline (nIdx);
}

bool SwWrtShell::GotoOutline( const String& rName )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoOutline (rName);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoRegion( const String& rName )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoRegion (rName);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
 }

sal_Bool SwWrtShell::GotoRefMark( const String& rRefMark, sal_uInt16 nSubType,
                                    sal_uInt16 nSeqNo )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoRefMark(rRefMark, nSubType, nSeqNo);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

sal_Bool SwWrtShell::GotoNextTOXBase( const OUString* pName )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoNextTOXBase(pName);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoTable( const String& rName )
{
   SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoTable(rName);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

sal_Bool SwWrtShell::GotoFld( const SwFmtFld& rFld ) {
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoFld(rFld);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

const SwRedline* SwWrtShell::GotoRedline( sal_uInt16 nArrPos, sal_Bool bSelect ) {
    SwPosition aPos = *GetCrsr()->GetPoint();
    const SwRedline *pRedline = SwCrsrShell::GotoRedline(nArrPos, bSelect);
    if (pRedline)
        aNavigationMgr.addEntry(aPos);
    return pRedline;
}

sal_Bool SwWrtShell::SelectTxtAttr( sal_uInt16 nWhich, const SwTxtAttr* pAttr )
{
    sal_Bool bRet;
    {
        SwMvContext aMvContext(this);
        SttSelect();
        bRet = SwCrsrShell::SelectTxtAttr( nWhich, sal_False, pAttr );
    }
    EndSelect();
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
