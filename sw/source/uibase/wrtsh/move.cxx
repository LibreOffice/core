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

class ShellMoveCursor
{
    SwWrtShell* pSh;
    bool bAct;
public:
    inline ShellMoveCursor( SwWrtShell* pWrtSh, bool bSel )
    {
        bAct = !pWrtSh->ActionPend() && (pWrtSh->GetFrameType(nullptr,false) & FrameTypeFlags::FLY_ANY);
        ( pSh = pWrtSh )->MoveCursor( bSel );
        pWrtSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_HYPERLINK_GETLINK);
    }
    inline ~ShellMoveCursor()
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

void SwWrtShell::MoveCursor( bool bWithSelect )
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
        SttCursorMove();
        MoveCursor( true );
        bRet = (this->*FnSimpleMove)();
        EndCursorMove();
    }
    else if( ( bRet = (this->*FnSimpleMove)() ) )
        MoveCursor();
    return bRet;
}

bool SwWrtShell::Left( sal_uInt16 nMode, bool bSelect,
                            sal_uInt16 nCount, bool bBasicCall, bool bVisual )
{
    if ( !bSelect && !bBasicCall && IsCursorReadonly()  && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() -= VisArea().Width() * nReadOnlyScrollOfst / 100;
        m_rView.SetVisArea( aTmp );
        return true;
    }
    else
    {
        ShellMoveCursor aTmp( this, bSelect );
        return SwCursorShell::Left( nCount, nMode, bVisual );
    }
}

bool SwWrtShell::Right( sal_uInt16 nMode, bool bSelect,
                            sal_uInt16 nCount, bool bBasicCall, bool bVisual )
{
    if ( !bSelect && !bBasicCall && IsCursorReadonly() && !GetViewOptions()->IsSelectionInReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() += VisArea().Width() * nReadOnlyScrollOfst / 100;
        aTmp.X() = m_rView.SetHScrollMax( aTmp.X() );
        m_rView.SetVisArea( aTmp );
        return true;
    }
    else
    {
        ShellMoveCursor aTmp( this, bSelect );
        return SwCursorShell::Right( nCount, nMode, bVisual );
    }
}

bool SwWrtShell::Up( bool bSelect, sal_uInt16 nCount, bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCursorReadonly()  && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.Y() -= VisArea().Height() * nReadOnlyScrollOfst / 100;
        m_rView.SetVisArea( aTmp );
        return true;
    }

    ShellMoveCursor aTmp( this, bSelect );
    return SwCursorShell::Up( nCount );
}

bool SwWrtShell::Down( bool bSelect, sal_uInt16 nCount, bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCursorReadonly() && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.Y() += VisArea().Height() * nReadOnlyScrollOfst / 100;
        aTmp.Y() = m_rView.SetVScrollMax( aTmp.Y() );
        m_rView.SetVisArea( aTmp );
        return true;
    }

    ShellMoveCursor aTmp( this, bSelect );
    return SwCursorShell::Down( nCount );
}

bool SwWrtShell::LeftMargin( bool bSelect, bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCursorReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() = DOCUMENTBORDER;
        m_rView.SetVisArea( aTmp );
        return true;
    }
    else
    {
        ShellMoveCursor aTmp( this, bSelect );
        return SwCursorShell::LeftMargin();
    }
}

bool SwWrtShell::RightMargin( bool bSelect, bool bBasicCall  )
{
    if ( !bSelect && !bBasicCall && IsCursorReadonly() )
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
        ShellMoveCursor aTmp( this, bSelect );
        return SwCursorShell::RightMargin(bBasicCall);
    }
}

bool SwWrtShell::GoStart( bool bKeepArea, bool *pMoveTable,
                          bool bSelect, bool bDontMoveRegion )
{
    if ( IsCursorInTable() )
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
    const FrameTypeFlags nFrameType = GetFrameType(nullptr,false);
    if ( FrameTypeFlags::FLY_ANY & nFrameType )
    {
        if( MoveSection( fnSectionCurr, fnSectionStart ) )
            return true;
        else if ( FrameTypeFlags::FLY_FREE & nFrameType || bDontMoveRegion )
            return false;
    }
    if(( FrameTypeFlags::HEADER | FrameTypeFlags::FOOTER | FrameTypeFlags::FOOTNOTE ) & nFrameType )
    {
        if ( MoveSection( fnSectionCurr, fnSectionStart ) )
            return true;
        else if ( bKeepArea )
            return true;
    }
    // Regions ???
    return SwCursorShell::MoveRegion( fnRegionCurrAndSkip, fnRegionStart ) ||
           SwCursorShell::SttEndDoc(true);
}

bool SwWrtShell::GoEnd(bool bKeepArea, bool *pMoveTable)
{
    if ( pMoveTable && *pMoveTable )
        return MoveTable( fnTableCurr, fnTableEnd );

    if ( IsCursorInTable() )
    {
        if ( MoveSection( fnSectionCurr, fnSectionEnd ) ||
             MoveTable( fnTableCurr, fnTableEnd ) )
            return true;
    }
    else
    {
        const FrameTypeFlags nFrameType = GetFrameType(nullptr,false);
        if ( FrameTypeFlags::FLY_ANY & nFrameType )
        {
            if ( MoveSection( fnSectionCurr, fnSectionEnd ) )
                return true;
            else if ( FrameTypeFlags::FLY_FREE & nFrameType )
                return false;
        }
        if(( FrameTypeFlags::HEADER | FrameTypeFlags::FOOTER | FrameTypeFlags::FOOTNOTE ) & nFrameType )
        {
            if ( MoveSection( fnSectionCurr, fnSectionEnd) )
                return true;
            else if ( bKeepArea )
                return true;
        }
    }
    // Regions ???
    return SwCursorShell::MoveRegion( fnRegionCurrAndSkip, fnRegionEnd ) ||
           SwCursorShell::SttEndDoc(false);
}

bool SwWrtShell::SttDoc( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect );
    return GoStart(false, nullptr, bSelect );
}

bool SwWrtShell::EndDoc( bool bSelect)
{
    ShellMoveCursor aTmp( this, bSelect );
    return GoEnd();
}

bool SwWrtShell::SttNxtPg( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageStart );
}

bool SwWrtShell::SttPrvPg( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageStart );
}

bool SwWrtShell::EndNxtPg( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageEnd );
}

bool SwWrtShell::EndPrvPg( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageEnd );
}

bool SwWrtShell::SttPg( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageStart );
}

bool SwWrtShell::EndPg( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageEnd );
}

bool SwWrtShell::SttPara( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect );
    return MovePara( fnParaCurr, fnParaStart );
}

bool SwWrtShell::EndPara( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect );
    return MovePara(fnParaCurr,fnParaEnd);
}

// Column-by-jumping.
// SSelection with or without
// returns success or failure

bool SwWrtShell::StartOfColumn( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnStart);
}

bool SwWrtShell::EndOfColumn( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnEnd);
}

bool SwWrtShell::StartOfNextColumn( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect);
    return MoveColumn( fnColumnNext, fnColumnStart);
}

bool SwWrtShell::EndOfNextColumn( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect);
    return MoveColumn(fnColumnNext, fnColumnEnd);
}

bool SwWrtShell::StartOfPrevColumn( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect);
    return MoveColumn(fnColumnPrev, fnColumnStart);
}

bool SwWrtShell::EndOfPrevColumn( bool bSelect )
{
    ShellMoveCursor aTmp( this, bSelect);
    return MoveColumn(fnColumnPrev, fnColumnEnd);
}

bool SwWrtShell::PushCursor(SwTwips lOffset, bool bSelect)
{
    bool bDiff = false;
    SwRect aOldRect( GetCharRect() ), aTmpArea( VisArea() );

    // m_bDestOnStack indicates if I could not set the coursor at the current
    // position, because in this region is no content.
    if( !m_bDestOnStack )
    {
        Point aPt( aOldRect.Center() );

        if( !IsCursorVisible() )
            // set CursorPos to top-/bottom left pos. So the pagescroll is not
            // be dependent on the current cursor, but on the visarea.
            aPt.Y() = aTmpArea.Top() + aTmpArea.Height() / 2;

        aPt.Y() += lOffset;
        m_aDest = GetContentPos(aPt,lOffset > 0);
        m_aDest.X() = aPt.X();
        m_bDestOnStack = true;
    }

    // If we had a frame selection, it must be removed after the m_fnSetCursor
    // and we have to remember the position on the stack to return to it later.
    bool bIsFrameSel = false;

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

        bIsFrameSel = IsFrameSelected();
        bool bIsObjSel = 0 != IsObjSelected();

        // unselect frame
        if( bIsFrameSel || bIsObjSel )
        {
            UnSelectFrame();
            LeaveSelFrameMode();
            if ( bIsObjSel )
            {
                GetView().SetDrawFuncPtr( nullptr );
                GetView().LeaveDrawCreate();
            }

            CallChgLnk();
        }

        (this->*m_fnSetCursor)( &m_aDest, true );

        bDiff = aOldRect != GetCharRect();

        if( bIsFrameSel )
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
    m_pCursorStack = new CursorStack( bDiff, bIsFrameSel, aOldRect.Center(),
                                lOffset, m_pCursorStack );
    return !m_bDestOnStack && bDiff;
}

bool SwWrtShell::PopCursor(bool bUpdate, bool bSelect)
{
    if( nullptr == m_pCursorStack)
        return false;

    const bool bValidPos = m_pCursorStack->bValidCurPos;
    if( bUpdate && bValidPos )
    {
            // If a predecessor is on the stack,
            // use the flag for a valid position.
        SwRect aTmpArea(VisArea());
        aTmpArea.Pos().Y() -= m_pCursorStack->lOffset;
        if( aTmpArea.IsInside( m_pCursorStack->aDocPos ) )
        {
            if( bSelect )
                SttSelect();
            else
                EndSelect();

            (this->*m_fnSetCursor)(&m_pCursorStack->aDocPos, !m_pCursorStack->bIsFrameSel);
            if( m_pCursorStack->bIsFrameSel && IsObjSelectable(m_pCursorStack->aDocPos))
            {
                HideCursor();
                SelectObj( m_pCursorStack->aDocPos );
                EnterSelFrameMode( &m_pCursorStack->aDocPos );
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
    CursorStack *pTmp = m_pCursorStack;
    m_pCursorStack = m_pCursorStack->pNext;
    delete pTmp;
    if( nullptr == m_pCursorStack )
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
    while(m_pCursorStack)
    {
        CursorStack* const pTmp = m_pCursorStack->pNext;
        delete m_pCursorStack;
        m_pCursorStack = pTmp;
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

bool SwWrtShell::PageCursor(SwTwips lOffset, bool bSelect)
{
    // Do nothing if an offset of 0 was indicated
    if(!lOffset) return false;
        // Was once used to force a reformat of the layout.
        // This has not work that way, because the cursor was not set
        // because this does not happen within a
        // Start-/EndActionParentheses.
        // Because only SwViewShell::EndAction() is called at the end,
        // no updating of the display of the cursor position takes place.
        // The CursorShell-Actionparentheses cannot be used, because it
        // always leads to displaying the cursor, thus also,
        // if after the scroll scrolled in a region without a valid position.
        // SwViewShell::StartAction();
    PageMove eDir = lOffset > 0? MV_PAGE_DOWN: MV_PAGE_UP;
        // Change of direction and stack present
    if( eDir != m_ePageMove && m_ePageMove != MV_NO && PopCursor( true, bSelect ))
        return true;

    const bool bRet = PushCursor(lOffset, bSelect);
    m_ePageMove = eDir;
    return bRet;
}

bool SwWrtShell::GotoPage(sal_uInt16 nPage, bool bRecord)
{
    ShellMoveCursor aTmp( this, false);
    if( SwCursorShell::GotoPage(nPage) && bRecord)
    {
        if(IsSelFrameMode())
        {
            UnSelectFrame();
            LeaveSelFrameMode();
        }
        return true;
    }
    return false;
}

bool SwWrtShell::GotoMark( const ::sw::mark::IMark* const pMark, bool bSelect, bool bStart )
{
    ShellMoveCursor aTmp( this, bSelect );
    SwPosition aPos = *GetCursor()->GetPoint();
    bool bRet = SwCursorShell::GotoMark( pMark, bStart );
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoFly( const OUString& rName, FlyCntType eType, bool bSelFrame )
{
    SwPosition aPos = *GetCursor()->GetPoint();
    bool bRet = SwFEShell::GotoFly(rName, eType, bSelFrame);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoINetAttr( const SwTextINetFormat& rAttr )
{
    SwPosition aPos = *GetCursor()->GetPoint();
    bool bRet = SwCursorShell::GotoINetAttr(rAttr);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

void SwWrtShell::GotoOutline( sal_uInt16 nIdx )
{
    addCurrentPosition();
    SwCursorShell::GotoOutline (nIdx);
}

bool SwWrtShell::GotoOutline( const OUString& rName )
{
    SwPosition aPos = *GetCursor()->GetPoint();
    bool bRet = SwCursorShell::GotoOutline (rName);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoRegion( const OUString& rName )
{
    SwPosition aPos = *GetCursor()->GetPoint();
    bool bRet = SwCursorShell::GotoRegion (rName);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
 }

bool SwWrtShell::GotoRefMark( const OUString& rRefMark, sal_uInt16 nSubType,
                                    sal_uInt16 nSeqNo )
{
    SwPosition aPos = *GetCursor()->GetPoint();
    bool bRet = SwCursorShell::GotoRefMark(rRefMark, nSubType, nSeqNo);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoNextTOXBase( const OUString* pName )
{
    SwPosition aPos = *GetCursor()->GetPoint();
    bool bRet = SwCursorShell::GotoNextTOXBase(pName);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoTable( const OUString& rName )
{
   SwPosition aPos = *GetCursor()->GetPoint();
    bool bRet = SwCursorShell::GotoTable(rName);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoFormatField( const SwFormatField& rField ) {
    SwPosition aPos = *GetCursor()->GetPoint();
    bool bRet = SwCursorShell::GotoFormatField(rField);
    if (bRet)
        m_aNavigationMgr.addEntry(aPos);
    return bRet;
}

const SwRangeRedline* SwWrtShell::GotoRedline( sal_uInt16 nArrPos, bool bSelect ) {
    SwPosition aPos = *GetCursor()->GetPoint();
    const SwRangeRedline *pRedline = SwCursorShell::GotoRedline(nArrPos, bSelect);
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
        bRet = SwCursorShell::SelectTextAttr( nWhich, false, pAttr );
    }
    EndSelect();
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
