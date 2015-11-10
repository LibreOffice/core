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
    inline ShellMoveCrsr( SwWrtShell* pWrtSh, bool bSel )
    {
        bAct = !pWrtSh->ActionPend() && (pWrtSh->GetFrmType(nullptr,false) & FrmTypeFlags::FLY_ANY);
        ( pSh = pWrtSh )->MoveCrsr( bSel );
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

void SwWrtShell::MoveCrsr( bool bWithSelect )
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
        (this->*m_fnKillSel)( nullptr, false );
    }
}

bool SwWrtShell::SimpleMove( FNSimpleMove FnSimpleMove, bool bSelect )
{
    bool bRet;
    if( bSelect )
    {
        SttCrsrMove();
        MoveCrsr( true );
        bRet = (this->*FnSimpleMove)();
        EndCrsrMove();
    }
    else if( ( bRet = (this->*FnSimpleMove)() ) )
        MoveCrsr();
    return bRet;
}

bool SwWrtShell::Left( sal_uInt16 nMode, bool bSelect,
                            sal_uInt16 nCount, bool bBasicCall, bool bVisual )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly()  && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() -= VisArea().Width() * nReadOnlyScrollOfst / 100;
        m_rView.SetVisArea( aTmp );
        return true;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Left( nCount, nMode, bVisual );
    }
}

bool SwWrtShell::Right( sal_uInt16 nMode, bool bSelect,
                            sal_uInt16 nCount, bool bBasicCall, bool bVisual )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() && !GetViewOptions()->IsSelectionInReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() += VisArea().Width() * nReadOnlyScrollOfst / 100;
        aTmp.X() = m_rView.SetHScrollMax( aTmp.X() );
        m_rView.SetVisArea( aTmp );
        return true;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Right( nCount, nMode, bVisual );
    }
}

bool SwWrtShell::Up( bool bSelect, sal_uInt16 nCount, bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly()  && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.Y() -= VisArea().Height() * nReadOnlyScrollOfst / 100;
        m_rView.SetVisArea( aTmp );
        return true;
    }

    ShellMoveCrsr aTmp( this, bSelect );
    return SwCrsrShell::Up( nCount );
}

bool SwWrtShell::Down( bool bSelect, sal_uInt16 nCount, bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.Y() += VisArea().Height() * nReadOnlyScrollOfst / 100;
        aTmp.Y() = m_rView.SetVScrollMax( aTmp.Y() );
        m_rView.SetVisArea( aTmp );
        return true;
    }

    ShellMoveCrsr aTmp( this, bSelect );
    return SwCrsrShell::Down( nCount );
}

bool SwWrtShell::LeftMargin( bool bSelect, bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() = DOCUMENTBORDER;
        m_rView.SetVisArea( aTmp );
        return true;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::LeftMargin();
    }
}

bool SwWrtShell::RightMargin( bool bSelect, bool bBasicCall  )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() = GetDocSize().Width() - VisArea().Width() + DOCUMENTBORDER;
        if( DOCUMENTBORDER > aTmp.X() )
            aTmp.X() = DOCUMENTBORDER;
        m_rView.SetVisArea( aTmp );
        return true;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::RightMargin(bBasicCall);
    }
}

bool SwWrtShell::GoStart( bool bKeepArea, bool *pMoveTable,
                          bool bSelect, bool bDontMoveRegion )
{
    if ( IsCrsrInTable() )
    {
        const bool bBoxSelection = HasBoxSelection();
        if( !m_bBlockMode )
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
                *pMoveTable = false;
            return true;
        }
        if( MoveTable( fnTableCurr, fnTableStart ) || bDontMoveRegion )
        {
            if ( pMoveTable )
                *pMoveTable = true;
            return true;
        }
        else if( bBoxSelection && pMoveTable )
        {
            // JP 09.01.96: We have a box selection (or a empty cell)
            //              and we want select (pMoveTable will be
            //              set in SelAll). Then the table must not
            //              be left, otherwise there is no selection
            //              of the entire table possible!
            *pMoveTable = true;
            return true;
        }
    }

    if( !m_bBlockMode )
    {
        if ( !bSelect )
            EnterStdMode();
        else
            SttSelect();
    }
    const FrmTypeFlags nFrmType = GetFrmType(nullptr,false);
    if ( FrmTypeFlags::FLY_ANY & nFrmType )
    {
        if( MoveSection( fnSectionCurr, fnSectionStart ) )
            return true;
        else if ( FrmTypeFlags::FLY_FREE & nFrmType || bDontMoveRegion )
            return false;
    }
    if(( FrmTypeFlags::HEADER | FrmTypeFlags::FOOTER | FrmTypeFlags::FOOTNOTE ) & nFrmType )
    {
        if ( MoveSection( fnSectionCurr, fnSectionStart ) )
            return true;
        else if ( bKeepArea )
            return true;
    }
    // Regions ???
    return SwCrsrShell::MoveRegion( fnRegionCurrAndSkip, fnRegionStart ) ||
           SwCrsrShell::SttEndDoc(true);
}

bool SwWrtShell::GoEnd(bool bKeepArea, bool *pMoveTable)
{
    if ( pMoveTable && *pMoveTable )
        return MoveTable( fnTableCurr, fnTableEnd );

    if ( IsCrsrInTable() )
    {
        if ( MoveSection( fnSectionCurr, fnSectionEnd ) ||
             MoveTable( fnTableCurr, fnTableEnd ) )
            return true;
    }
    else
    {
        const FrmTypeFlags nFrmType = GetFrmType(nullptr,false);
        if ( FrmTypeFlags::FLY_ANY & nFrmType )
        {
            if ( MoveSection( fnSectionCurr, fnSectionEnd ) )
                return true;
            else if ( FrmTypeFlags::FLY_FREE & nFrmType )
                return false;
        }
        if(( FrmTypeFlags::HEADER | FrmTypeFlags::FOOTER | FrmTypeFlags::FOOTNOTE ) & nFrmType )
        {
            if ( MoveSection( fnSectionCurr, fnSectionEnd) )
                return true;
            else if ( bKeepArea )
                return true;
        }
    }
    // Regions ???
    return SwCrsrShell::MoveRegion( fnRegionCurrAndSkip, fnRegionEnd ) ||
           SwCrsrShell::SttEndDoc(false);
}

bool SwWrtShell::SttDoc( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return GoStart(false, nullptr, bSelect );
}

bool SwWrtShell::EndDoc( bool bSelect)
{
    ShellMoveCrsr aTmp( this, bSelect );
    return GoEnd();
}

bool SwWrtShell::SttNxtPg( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageStart );
}

bool SwWrtShell::SttPrvPg( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageStart );
}

bool SwWrtShell::EndNxtPg( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageEnd );
}

bool SwWrtShell::EndPrvPg( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageEnd );
}

bool SwWrtShell::SttPg( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageStart );
}

bool SwWrtShell::EndPg( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageEnd );
}

bool SwWrtShell::SttPara( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePara( fnParaCurr, fnParaStart );
}

bool SwWrtShell::EndPara( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePara(fnParaCurr,fnParaEnd);
}

// Column-by-jumping.
// SSelection with or without
// returns success or failure

bool SwWrtShell::StartOfColumn( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnStart);
}

bool SwWrtShell::EndOfColumn( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnEnd);
}

bool SwWrtShell::StartOfNextColumn( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn( fnColumnNext, fnColumnStart);
}

bool SwWrtShell::EndOfNextColumn( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnNext, fnColumnEnd);
}

bool SwWrtShell::StartOfPrevColumn( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnPrev, fnColumnStart);
}

bool SwWrtShell::EndOfPrevColumn( bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnPrev, fnColumnEnd);
}

bool SwWrtShell::PushCrsr(SwTwips lOffset, bool bSelect)
{
    bool bDiff = false;
    SwRect aOldRect( GetCharRect() ), aTmpArea( VisArea() );

    // m_bDestOnStack indicates if I could not set the coursor at the current
    // position, because in this region is no content.
    if( !m_bDestOnStack )
    {
        Point aPt( aOldRect.Center() );

        if( !IsCrsrVisible() )
            // set CrsrPos to top-/bottom left pos. So the pagescroll is not
            // be dependent on the current cursor, but on the visarea.
            aPt.Y() = aTmpArea.Top() + aTmpArea.Height() / 2;

        aPt.Y() += lOffset;
        m_aDest = GetContentPos(aPt,lOffset > 0);
        m_aDest.X() = aPt.X();
        m_bDestOnStack = true;
    }

    // If we had a frame selection, it must be removed after the m_fnSetCrsr
    // and we have to remember the position on the stack to return to it later.
    bool bIsFrmSel = false;

    //Target position is now within the viewable region -->
    //Place the cursor at the target position; remember that no target
    //position is longer on the stack.
    //The new visible region is to be determined beforehand.
    aTmpArea.Pos().Y() += lOffset;
    if( aTmpArea.IsInside(m_aDest) )
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
                GetView().SetDrawFuncPtr( nullptr );
                GetView().LeaveDrawCreate();
            }

            CallChgLnk();
        }

        (this->*m_fnSetCrsr)( &m_aDest, true );

        bDiff = aOldRect != GetCharRect();

        if( bIsFrmSel )
        {
            // In frames take only the upper corner
            // so that it can be re-selected.
            aOldRect.SSize( 5, 5 );
        }

            // reset Dest. SPoint Flags
        m_bDestOnStack = false;
    }

    // Position into the stack; bDiff indicates if there is a
    // difference between the old and the new cursor position.
    m_pCrsrStack = new CrsrStack( bDiff, bIsFrmSel, aOldRect.Center(),
                                lOffset, m_pCrsrStack );
    return !m_bDestOnStack && bDiff;
}

bool SwWrtShell::PopCrsr(bool bUpdate, bool bSelect)
{
    if( nullptr == m_pCrsrStack)
        return false;

    const bool bValidPos = m_pCrsrStack->bValidCurPos;
    if( bUpdate && bValidPos )
    {
            // If a predecessor is on the stack,
            // use the flag for a valid position.
        SwRect aTmpArea(VisArea());
        aTmpArea.Pos().Y() -= m_pCrsrStack->lOffset;
        if( aTmpArea.IsInside( m_pCrsrStack->aDocPos ) )
        {
            if( bSelect )
                SttSelect();
            else
                EndSelect();

            (this->*m_fnSetCrsr)(&m_pCrsrStack->aDocPos, !m_pCrsrStack->bIsFrmSel);
            if( m_pCrsrStack->bIsFrmSel && IsObjSelectable(m_pCrsrStack->aDocPos))
            {
                HideCrsr();
                SelectObj( m_pCrsrStack->aDocPos );
                EnterSelFrmMode( &m_pCrsrStack->aDocPos );
            }
        }
            // If a discrepancy between the visible range and the
            // remembered cursor position occurs, all of the remembered
            // positions are thrown away.
        else
        {
            _ResetCursorStack();
            return false;
        }
    }
    CrsrStack *pTmp = m_pCrsrStack;
    m_pCrsrStack = m_pCrsrStack->pNext;
    delete pTmp;
    if( nullptr == m_pCrsrStack )
    {
        m_ePageMove = MV_NO;
        m_bDestOnStack = false;
    }
    return bValidPos;
}

// Reset of all pushed cursor positions; these will
// not be displayed ( --> No Start-/EndAction!!)

void SwWrtShell::_ResetCursorStack()
{
    while(m_pCrsrStack)
    {
        CrsrStack* const pTmp = m_pCrsrStack->pNext;
        delete m_pCrsrStack;
        m_pCrsrStack = pTmp;
    }
    m_ePageMove = MV_NO;
    m_bDestOnStack = false;
}
/**
    if no stack exists --> cancel selection
    if stack && change of direction
        --> pop cursor and return
    else
        --> push cursor
            transpose cursor
*/

bool SwWrtShell::PageCrsr(SwTwips lOffset, bool bSelect)
{
    // Do nothing if an offset of 0 was indicated
    if(!lOffset) return false;
        // Was once used to force a reformat of the layout.
        // This has not work that way, because the cursor was not set
        // because this does not happen within a
        // Start-/EndActionParentheses.
        // Because only SwViewShell::EndAction() is called at the end,
        // no updating of the display of the cursor position takes place.
        // The CrsrShell-Actionparentheses cannot be used, because it
        // always leads to displaying the cursor, thus also,
        // if after the scroll scrolled in a region without a valid postition.
        // SwViewShell::StartAction();
    PageMove eDir = lOffset > 0? MV_PAGE_DOWN: MV_PAGE_UP;
        // Change of direction and stack present
    if( eDir != m_ePageMove && m_ePageMove != MV_NO && PopCrsr( true, bSelect ))
        return true;

    const bool bRet = PushCrsr(lOffset, bSelect);
    m_ePageMove = eDir;
    return bRet;
}

bool SwWrtShell::GotoPage(sal_uInt16 nPage, bool bRecord)
{
    ShellMoveCrsr aTmp( this, false);
    if( SwCrsrShell::GotoPage(nPage) && bRecord)
    {
        if(IsSelFrmMode())
        {
            UnSelectFrm();
            LeaveSelFrmMode();
        }
        return true;
    }
    return false;
}

bool SwWrtShell::GotoMark( const ::sw::mark::IMark* const pMark, bool bSelect, bool bStart )
{
    ShellMoveCrsr aTmp( this, bSelect );
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoMark( pMark, bStart );
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoFly( const OUString& rName, FlyCntType eType, bool bSelFrame )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwFEShell::GotoFly(rName, eType, bSelFrame);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoINetAttr( const SwTextINetFormat& rAttr )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoINetAttr(rAttr);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

void SwWrtShell::GotoOutline( sal_uInt16 nIdx )
{
    addCurrentPosition();
    SwCrsrShell::GotoOutline (nIdx);
}

bool SwWrtShell::GotoOutline( const OUString& rName )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoOutline (rName);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoRegion( const OUString& rName )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoRegion (rName);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
 }

bool SwWrtShell::GotoRefMark( const OUString& rRefMark, sal_uInt16 nSubType,
                                    sal_uInt16 nSeqNo )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoRefMark(rRefMark, nSubType, nSeqNo);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoNextTOXBase( const OUString* pName )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoNextTOXBase(pName);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoTable( const OUString& rName )
{
   SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoTable(rName);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoFormatField( const SwFormatField& rField ) {
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoFormatField(rField);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

const SwRangeRedline* SwWrtShell::GotoRedline( sal_uInt16 nArrPos, bool bSelect ) {
    SwPosition aPos = *GetCrsr()->GetPoint();
    const SwRangeRedline *pRedline = SwCrsrShell::GotoRedline(nArrPos, bSelect);
    if (pRedline)
        m_aNavigationMgr.addEntry(aPos);
    return pRedline;
}

bool SwWrtShell::SelectTextAttr( sal_uInt16 nWhich, const SwTextAttr* pAttr )
{
    bool bRet;
    {
        SwMvContext aMvContext(this);
        SttSelect();
        bRet = SwCrsrShell::SelectTextAttr( nWhich, false, pAttr );
    }
    EndSelect();
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
