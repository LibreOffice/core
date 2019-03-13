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

#include <memory>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <viewopt.hxx>
#include <frmatr.hxx>
#include <frmtool.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/keepitem.hxx>
#include <svx/sdtaitm.hxx>

#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <fmtclbl.hxx>

#include <tabfrm.hxx>
#include <ftnfrm.hxx>
#include <txtfrm.hxx>
#include <sectfrm.hxx>
#include <dbg_lay.hxx>

#include <sortedobjs.hxx>
#include <layouter.hxx>
#include <flyfrms.hxx>

#include <DocumentSettingManager.hxx>
#include <IDocumentLayoutAccess.hxx>

// Move methods

/// Return value tells whether the Frame should be moved.
bool SwContentFrame::ShouldBwdMoved( SwLayoutFrame *pNewUpper, bool, bool & )
{
    if ( SwFlowFrame::IsMoveBwdJump() || !IsPrevObjMove() )
    {
        // Floating back a frm uses a bit of time unfortunately.
        // The most common case is the following: The Frame wants to float to
        // somewhere where the FixSize is the same that the Frame itself has already.
        // In that case it's pretty easy to check if the Frame has enough space
        // for its VarSize. If this is NOT the case, we already know that
        // we don't need to move.
        // The Frame checks itself whether it has enough space - respecting the fact
        // that it could possibly split itself if needed.
        // If, however, the FixSize differs from the Frame or Flys are involved
        // (either in the old or the new position), checking is pointless,
        // and we have to move the Frame just to see what happens - if there's
        // some space available to do it, that is.

        // The FixSize of the containers of Contents is always the width.

        // If we moved more than one sheet back (for example jumping over empty
        // pages), we have to move either way. Otherwise, if the Frame doesn't fit
        // into the page, empty pages wouldn't be respected anymore.
        sal_uInt8 nMoveAnyway = 0;
        SwPageFrame * const pNewPage = pNewUpper->FindPageFrame();
        SwPageFrame *pOldPage = FindPageFrame();

        if ( SwFlowFrame::IsMoveBwdJump() )
            return true;

        if( IsInFootnote() && IsInSct() )
        {
            SwFootnoteFrame* pFootnote = FindFootnoteFrame();
            SwSectionFrame* pMySect = pFootnote->FindSctFrame();
            if( pMySect && pMySect->IsFootnoteLock() )
            {
                SwSectionFrame *pSect = pNewUpper->FindSctFrame();
                while( pSect && pSect->IsInFootnote() )
                    pSect = pSect->GetUpper()->FindSctFrame();
                OSL_ENSURE( pSect, "Escaping footnote" );
                if( pSect != pMySect )
                    return false;
            }
        }
        SwRectFnSet aRectFnSet(this);
        SwRectFnSet fnRectX(pNewUpper);
        if( std::abs( fnRectX.GetWidth(pNewUpper->getFramePrintArea()) -
                 aRectFnSet.GetWidth(GetUpper()->getFramePrintArea()) ) > 1 ) {
            // In this case, only a WouldFit_ with test move is possible
            nMoveAnyway = 2;
        }

        // Do *not* move backward, if <nMoveAnyway> equals 3 and no space is left in new upper.
        nMoveAnyway |= BwdMoveNecessary( pOldPage, getFrameArea() );
        {
            const IDocumentSettingAccess& rIDSA = pNewPage->GetFormat()->getIDocumentSettingAccess();
            SwTwips nSpace = 0;
            SwRect aRect( pNewUpper->getFramePrintArea() );
            aRect.Pos() += pNewUpper->getFrameArea().Pos();
            const SwFrame *pPrevFrame = pNewUpper->Lower();
            while ( pPrevFrame )
            {
                SwTwips nNewTop = fnRectX.GetBottom(pPrevFrame->getFrameArea());
                // Consider lower spacing of last frame in a table cell
                {
                    // Check if last frame is inside table and if it includes its lower spacing.
                    if ( !pPrevFrame->GetNext() && pPrevFrame->IsInTab() &&
                         rIDSA.get(DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS) )
                    {
                        const SwFrame* pLastFrame = pPrevFrame;
                        // if last frame is a section, take its last content
                        if ( pPrevFrame->IsSctFrame() )
                        {
                            pLastFrame = static_cast<const SwSectionFrame*>(pPrevFrame)->FindLastContent();
                            if ( pLastFrame &&
                                 pLastFrame->FindTabFrame() != pPrevFrame->FindTabFrame() )
                            {
                                pLastFrame = pLastFrame->FindTabFrame();
                            }
                        }

                        if ( pLastFrame )
                        {
                            SwBorderAttrAccess aAccess( SwFrame::GetCache(), pLastFrame );
                            const SwBorderAttrs& rAttrs = *aAccess.Get();
                            nNewTop -= rAttrs.GetULSpace().GetLower();
                        }
                    }
                }
                fnRectX.SetTop( aRect, nNewTop );

                pPrevFrame = pPrevFrame->GetNext();
            }

            nMoveAnyway |= BwdMoveNecessary( pNewPage, aRect);

            //determine space left in new upper frame
            nSpace = fnRectX.GetHeight(aRect);
            const SwViewShell *pSh = pNewUpper->getRootFrame()->GetCurrShell();
            if ( IsInFootnote() ||
                 (pSh && pSh->GetViewOptions()->getBrowseMode()) ||
                 pNewUpper->IsCellFrame() ||
                 ( pNewUpper->IsInSct() && ( pNewUpper->IsSctFrame() ||
                   ( pNewUpper->IsColBodyFrame() &&
                     !pNewUpper->GetUpper()->GetPrev() &&
                     !pNewUpper->GetUpper()->GetNext() ) ) ) )
                nSpace += pNewUpper->Grow( LONG_MAX, true );

            if ( nMoveAnyway < 3 )
            {
                if ( nSpace )
                {
                    // Do not notify footnotes which are stuck to the paragraph:
                    // This would require extremely confusing code, taking into
                    // account the widths
                    // and Flys, that in turn influence the footnotes, ...

                    // WouldFit_ can only be used if the width is the same and
                    // ONLY self-anchored Flys are present.

                    // WouldFit_ can also be used if ONLY Flys anchored
                    // somewhere else are present.
                    // In this case, the width doesn't even matter,
                    // because we're running a TestFormat in the new upper.
                    const sal_uInt8 nBwdMoveNecessaryResult =
                                            BwdMoveNecessary( pNewPage, aRect);
                    const bool bObjsInNewUpper( nBwdMoveNecessaryResult == 2 ||
                                                nBwdMoveNecessaryResult == 3 );

                    return WouldFit_( nSpace, pNewUpper, nMoveAnyway == 2,
                                      bObjsInNewUpper );
                }
                // It's impossible for WouldFit_ to return a usable result if
                // we have a fresh multi-column section - so we really have to
                // float back unless there is no space.
                return pNewUpper->IsInSct() && pNewUpper->IsColBodyFrame() &&
                       !fnRectX.GetWidth(pNewUpper->getFramePrintArea()) &&
                       ( pNewUpper->GetUpper()->GetPrev() ||
                         pNewUpper->GetUpper()->GetNext() );
            }

            // Check for space left in new upper
            return nSpace != 0;
        }
    }
    return false;
}

// Calc methods

// Two little friendships form a secret society
inline void PrepareLock( SwFlowFrame *pTab )
{
    pTab->LockJoin();
}
inline void PrepareUnlock( SwFlowFrame *pTab )
{
    pTab->UnlockJoin();

}

// hopefully, one day this function simply will return 'false'
static bool lcl_IsCalcUpperAllowed( const SwFrame& rFrame )
{
    return !rFrame.GetUpper()->IsSctFrame() &&
           !rFrame.GetUpper()->IsFooterFrame() &&
           // No format of upper Writer fly frame
           !rFrame.GetUpper()->IsFlyFrame() &&
           !( rFrame.GetUpper()->IsTabFrame() && rFrame.GetUpper()->GetUpper()->IsInTab() ) &&
           !( rFrame.IsTabFrame() && rFrame.GetUpper()->IsInTab() );
}

/** Prepares the Frame for "formatting" (MakeAll()).
 *
 * This method serves to save stack space: To calculate the position of the Frame
 * we have to make sure that the positions of Upper and Prev respectively are
 * valid. This may require a recursive call (a loop would be quite expensive,
 * as it's not required very often).
 *
 * Every call of MakeAll requires around 500 bytes on the stack - you easily
 * see where this leads to. This method requires only a little bit of stack
 * space, so the recursive call should not be a problem here.
 *
 * Another advantage is that one nice day, this method and with it the
 * formatting of predecessors could be avoided. Then it could probably be
 * possible to jump "quickly" to the document's end.
 *
 * @see MakeAll()
 */
void SwFrame::PrepareMake(vcl::RenderContext* pRenderContext)
{
    StackHack aHack;
    if ( GetUpper() )
    {
        SwFrameDeleteGuard aDeleteGuard(this);
        if ( lcl_IsCalcUpperAllowed( *this ) )
            GetUpper()->Calc(pRenderContext);
        OSL_ENSURE( GetUpper(), ":-( Layout unstable (Upper gone)." );
        if ( !GetUpper() )
            return;

        const bool bCnt = IsContentFrame();
        const bool bTab = IsTabFrame();
        bool bNoSect = IsInSct();
        bool bOldTabLock = false, bFoll = false;
        SwFlowFrame* pThis = bCnt ? static_cast<SwContentFrame*>(this) : nullptr;

        if ( bTab )
        {
            pThis = static_cast<SwTabFrame*>(this);
            bOldTabLock = static_cast<SwTabFrame*>(this)->IsJoinLocked();
            ::PrepareLock( static_cast<SwTabFrame*>(this) );
            bFoll = pThis->IsFollow();
        }
        else if( IsSctFrame() )
        {
            pThis = static_cast<SwSectionFrame*>(this);
            bFoll = pThis->IsFollow();
            bNoSect = false;
        }
        else if ( bCnt )
        {
            bFoll = pThis->IsFollow();
            if ( bFoll && GetPrev() )
            {
                // Do not follow the chain when we need only one instance
                const SwTextFrame* pMaster = static_cast<SwContentFrame*>(this)->FindMaster();
                if ( pMaster && pMaster->IsLocked() )
                {
                    MakeAll(pRenderContext);
                    return;
                }
            }
        }

        // There is no format of previous frame, if current frame is a table
        // frame and its previous frame wants to keep with it.
        const bool bFormatPrev = !bTab ||
                                 !GetPrev() ||
                                 !GetPrev()->GetAttrSet()->GetKeep().GetValue();
        if ( bFormatPrev )
        {
            SwFrame *pFrame = GetUpper()->Lower();
            while ( pFrame != this )
            {
                OSL_ENSURE( pFrame, ":-( Layout unstable (this not found)." );
                if ( !pFrame )
                    return; //Oioioioi ...

                if ( !pFrame->isFrameAreaDefinitionValid() )
                {
                    // A small interference that hopefully improves on the stability:
                    // If I'm Follow AND neighbor of a Frame before me, it would delete
                    // me when formatting. This as you can see could easily become a
                    // confusing situation that we want to avoid.
                    if ( bFoll && pFrame->IsFlowFrame() &&
                         SwFlowFrame::CastFlowFrame(pFrame)->IsAnFollow( pThis ) )
                        break;

                    pFrame->MakeAll(pRenderContext);
                    if( IsSctFrame() && !static_cast<SwSectionFrame*>(this)->GetSection() )
                        break;
                }
                // With ContentFrames, the chain may be broken while walking through
                // it. Therefore we have to figure out the follower in a bit more
                // complicated way. However, I'll HAVE to get back to myself
                // sometime again.
                pFrame = pFrame->FindNext();

                // If we started out in a SectionFrame, it might have happened that
                // we landed in a Section Follow via the MakeAll calls.
                // FindNext only gives us the SectionFrame, not it's content - we
                // won't find ourselves anymore!
                if( bNoSect && pFrame && pFrame->IsSctFrame() )
                {
                    SwFrame* pCnt = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
                    if( pCnt )
                        pFrame = pCnt;
                }
            }
            OSL_ENSURE( GetUpper(), "Layout unstable (Upper gone II)." );
            if ( !GetUpper() )
                return;

            if ( lcl_IsCalcUpperAllowed( *this ) )
                GetUpper()->Calc(pRenderContext);

            OSL_ENSURE( GetUpper(), "Layout unstable (Upper gone III)." );
        }

        if ( bTab && !bOldTabLock )
            ::PrepareUnlock( static_cast<SwTabFrame*>(this) );
    }
    MakeAll(pRenderContext);
}

void SwFrame::OptPrepareMake()
{
    // #i23129#, #i36347# - no format of upper Writer fly frame
    if ( GetUpper() && !GetUpper()->IsFooterFrame() &&
         !GetUpper()->IsFlyFrame() )
    {
        {
            SwFrameDeleteGuard aDeleteGuard(this);
            GetUpper()->Calc(getRootFrame()->GetCurrShell() ? getRootFrame()->GetCurrShell()->GetOut() : nullptr);
        }
        OSL_ENSURE( GetUpper(), ":-( Layout unstable (Upper gone)." );
        if ( !GetUpper() )
            return;
    }
    if ( GetPrev() && !GetPrev()->isFrameAreaDefinitionValid() )
    {
        PrepareMake(getRootFrame()->GetCurrShell() ? getRootFrame()->GetCurrShell()->GetOut() : nullptr);
    }
    else
    {
        StackHack aHack;
        MakeAll(IsRootFrame() ? nullptr : getRootFrame()->GetCurrShell()->GetOut());
    }
}

void SwFrame::PrepareCursor()
{
    StackHack aHack;
    if( GetUpper() && !GetUpper()->IsSctFrame() )
    {
        GetUpper()->PrepareCursor();
        GetUpper()->Calc(getRootFrame()->GetCurrShell() ? getRootFrame()->GetCurrShell()->GetOut() : nullptr);

        OSL_ENSURE( GetUpper(), ":-( Layout unstable (Upper gone)." );
        if ( !GetUpper() )
            return;

        const bool bCnt = IsContentFrame();
        const bool bTab = IsTabFrame();
        bool bNoSect = IsInSct();

        bool bOldTabLock = false, bFoll;
        SwFlowFrame* pThis = bCnt ? static_cast<SwContentFrame*>(this) : nullptr;

        if ( bTab )
        {
            bOldTabLock = static_cast<SwTabFrame*>(this)->IsJoinLocked();
            ::PrepareLock( static_cast<SwTabFrame*>(this) );
            pThis = static_cast<SwTabFrame*>(this);
        }
        else if( IsSctFrame() )
        {
            pThis = static_cast<SwSectionFrame*>(this);
            bNoSect = false;
        }
        bFoll = pThis && pThis->IsFollow();

        SwFrame *pFrame = GetUpper()->Lower();
        while ( pFrame != this )
        {
            OSL_ENSURE( pFrame, ":-( Layout unstable (this not found)." );
            if ( !pFrame )
                return; //Oioioioi ...

            if ( !pFrame->isFrameAreaDefinitionValid() )
            {
                // A small interference that hopefully improves on the stability:
                // If I'm Follow AND neighbor of a Frame before me, it would delete
                // me when formatting. This as you can see could easily become a
                // confusing situation that we want to avoid.
                if ( bFoll && pFrame->IsFlowFrame() &&
                     SwFlowFrame::CastFlowFrame(pFrame)->IsAnFollow( pThis ) )
                    break;

                pFrame->MakeAll(getRootFrame()->GetCurrShell()->GetOut());
            }
            // With ContentFrames, the chain may be broken while walking through
            // it. Therefore we have to figure out the follower in a bit more
            // complicated way. However, I'll HAVE to get back to myself
            // sometime again.
            pFrame = pFrame->FindNext();
            if( bNoSect && pFrame && pFrame->IsSctFrame() )
            {
                SwFrame* pCnt = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
                if( pCnt )
                    pFrame = pCnt;
            }
        }
        OSL_ENSURE( GetUpper(), "Layout unstable (Upper gone II)." );
        if ( !GetUpper() )
            return;

        GetUpper()->Calc(getRootFrame()->GetCurrShell()->GetOut());

        OSL_ENSURE( GetUpper(), "Layout unstable (Upper gone III)." );

        if ( bTab && !bOldTabLock )
            ::PrepareUnlock( static_cast<SwTabFrame*>(this) );
    }
    Calc(getRootFrame()->GetCurrShell() ? getRootFrame()->GetCurrShell()->GetOut() : nullptr);
}

// Here we return GetPrev(); however we will ignore empty SectionFrames
static SwFrame* lcl_Prev( SwFrame* pFrame, bool bSectPrv = true )
{
    SwFrame* pRet = pFrame->GetPrev();
    if( !pRet && pFrame->GetUpper() && pFrame->GetUpper()->IsSctFrame() &&
        bSectPrv && !pFrame->IsColumnFrame() )
        pRet = pFrame->GetUpper()->GetPrev();
    while( pRet && pRet->IsSctFrame() &&
           !static_cast<SwSectionFrame*>(pRet)->GetSection() )
        pRet = pRet->GetPrev();
    return pRet;
}

static SwFrame* lcl_NotHiddenPrev( SwFrame* pFrame )
{
    SwFrame *pRet = pFrame;
    do
    {
        pRet = lcl_Prev( pRet );
    } while ( pRet && pRet->IsTextFrame() && static_cast<SwTextFrame*>(pRet)->IsHiddenNow() );
    return pRet;
}

void SwFrame::MakePos()
{
    if ( !isFrameAreaPositionValid() )
    {
        setFrameAreaPositionValid(true);
        bool bUseUpper = false;
        SwFrame* pPrv = lcl_Prev( this );
        if ( pPrv &&
             ( !pPrv->IsContentFrame() ||
               ( static_cast<SwContentFrame*>(pPrv)->GetFollow() != this ) )
           )
        {
            if ( !StackHack::IsLocked() &&
                 ( !IsInSct() || IsSctFrame() ) &&
                 !pPrv->IsSctFrame() &&
                 !pPrv->GetAttrSet()->GetKeep().GetValue()
               )
            {
                pPrv->Calc(getRootFrame()->GetCurrShell() ? getRootFrame()->GetCurrShell()->GetOut() : nullptr);   // This may cause Prev to vanish!
            }
            else if ( pPrv->getFrameArea().Top() == 0 )
            {
                bUseUpper = true;
            }
        }

        pPrv = lcl_Prev( this, false );
        const SwFrameType nMyType = GetType();
        SwRectFnSet aRectFnSet((IsCellFrame() && GetUpper() ? GetUpper() : this));
        if ( !bUseUpper && pPrv )
        {
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
            aFrm.Pos( pPrv->getFrameArea().Pos() );

            if( FRM_NEIGHBOUR & nMyType )
            {
                const bool bR2L = IsRightToLeft();

                if( bR2L )
                {
                    aRectFnSet.SetPosX( aFrm, aRectFnSet.GetLeft(aFrm) - aRectFnSet.GetWidth(aFrm) );
                }
                else
                {
                    aRectFnSet.SetPosX( aFrm, aRectFnSet.GetLeft(aFrm) + aRectFnSet.GetWidth(pPrv->getFrameArea()) );
                }

                // cells may now leave their uppers
                if( aRectFnSet.IsVert() && SwFrameType::Cell & nMyType )
                {
                    aFrm.Pos().setX(aFrm.Pos().getX() - aFrm.Width() + pPrv->getFrameArea().Width());
                }
            }
            else if( aRectFnSet.IsVert() && FRM_NOTE_VERT & nMyType )
            {
                if ( aRectFnSet.IsVertL2R() )
                {
                    aFrm.Pos().setX(aFrm.Pos().getX() + pPrv->getFrameArea().Width());
                }
                else
                {
                    aFrm.Pos().setX(aFrm.Pos().getX() - aFrm.Width());
                }
            }
            else
            {
                aFrm.Pos().setY(aFrm.Pos().getY() + pPrv->getFrameArea().Height());
            }
        }
        else if ( GetUpper() )
        {
            // If parent frame is a footer frame and its <ColLocked()>, then
            // do *not* calculate it.
            // NOTE: Footer frame is <ColLocked()> during its
            //     <FormatSize(..)>, which is called from <Format(..)>, which
            //     is called from <MakeAll()>, which is called from <Calc()>.
            // #i56850#
            // - no format of upper Writer fly frame, which is anchored
            //   at-paragraph or at-character.
            if ( !GetUpper()->IsTabFrame() &&
                 !( IsTabFrame() && GetUpper()->IsInTab() ) &&
                 !GetUpper()->IsSctFrame() &&
                 !dynamic_cast<SwFlyAtContentFrame*>(GetUpper()) &&
                 !( GetUpper()->IsFooterFrame() &&
                    GetUpper()->IsColLocked() )
               )
            {
                GetUpper()->Calc(getRootFrame()->GetCurrShell()->GetOut());
            }
            pPrv = lcl_Prev( this, false );
            if ( !bUseUpper && pPrv )
            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                aFrm.Pos( pPrv->getFrameArea().Pos() );

                if( FRM_NEIGHBOUR & nMyType )
                {
                    const bool bR2L = IsRightToLeft();

                    if( bR2L )
                    {
                        aRectFnSet.SetPosX( aFrm, aRectFnSet.GetLeft(aFrm) - aRectFnSet.GetWidth(aFrm) );
                    }
                    else
                    {
                        aRectFnSet.SetPosX( aFrm, aRectFnSet.GetLeft(aFrm) + aRectFnSet.GetWidth(pPrv->getFrameArea()) );
                    }

                    // cells may now leave their uppers
                    if( aRectFnSet.IsVert() && SwFrameType::Cell & nMyType )
                    {
                        aFrm.Pos().setX(aFrm.Pos().getX() - aFrm.Width() + pPrv->getFrameArea().Width());
                    }
                }
                else if( aRectFnSet.IsVert() && FRM_NOTE_VERT & nMyType )
                {
                    aFrm.Pos().setX(aFrm.Pos().getX() - aFrm.Width());
                }
                else
                {
                    aFrm.Pos().setY(aFrm.Pos().getY() + pPrv->getFrameArea().Height());
                }
            }
            else
            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                aFrm.Pos( GetUpper()->getFrameArea().Pos() );

                if( GetUpper()->IsFlyFrame() )
                {
                    aFrm.Pos() += static_cast<SwFlyFrame*>(GetUpper())->ContentPos();
                }
                else
                {
                    aFrm.Pos() += GetUpper()->getFramePrintArea().Pos();
                }

                if( FRM_NEIGHBOUR & nMyType && IsRightToLeft() )
                {
                    if( aRectFnSet.IsVert() )
                    {
                        aFrm.Pos().setY(aFrm.Pos().getY() + GetUpper()->getFramePrintArea().Height() - aFrm.Height());
                    }
                    else
                    {
                        aFrm.Pos().setX(aFrm.Pos().getX() + GetUpper()->getFramePrintArea().Width() - aFrm.Width());
                    }
                }
                else if( aRectFnSet.IsVert() && !aRectFnSet.IsVertL2R() && FRM_NOTE_VERT & nMyType )
                {
                    aFrm.Pos().setX(aFrm.Pos().getX() - aFrm.Width() + GetUpper()->getFramePrintArea().Width());
                }
            }
        }
        else
        {
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
            aFrm.Pos().setX(0);
            aFrm.Pos().setY(0);
        }

        if( IsBodyFrame() && aRectFnSet.IsVert() && !aRectFnSet.IsVertL2R() && GetUpper() )
        {
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
            aFrm.Pos().setX(aFrm.Pos().getX() + GetUpper()->getFramePrintArea().Width() - aFrm.Width());
        }

        setFrameAreaPositionValid(true);
    }
}

// #i28701# - new type <SwSortedObjs>
static void lcl_CheckObjects(SwSortedObjs& rSortedObjs, const SwFrame* pFrame, long& rBot)
{
    // And then there can be paragraph anchored frames that sit below their paragraph.
    long nMax = 0;
    for (SwAnchoredObject* pObj : rSortedObjs)
    {
        // #i28701# - consider changed type of <SwSortedObjs>
        // entries.
        long nTmp = 0;
        if ( dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr )
        {
            SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pObj);
            if( pFly->getFrameArea().Top() != FAR_AWAY &&
                ( pFrame->IsPageFrame() ? pFly->IsFlyLayFrame() :
                  ( pFly->IsFlyAtContentFrame() &&
                    ( pFrame->IsBodyFrame() ? pFly->GetAnchorFrame()->IsInDocBody() :
                                          pFly->GetAnchorFrame()->IsInFootnote() ) ) ) )
            {
                nTmp = pFly->getFrameArea().Bottom();
            }
        }
        else
            nTmp = pObj->GetObjRect().Bottom();
        nMax = std::max( nTmp, nMax );
    }
    ++nMax; // Lower edge vs. height!
    rBot = std::max( rBot, nMax );
}

size_t SwPageFrame::GetContentHeight(const long nTop, const long nBottom) const
{
    OSL_ENSURE(!(FindBodyCont() && FindBodyCont()->Lower() && FindBodyCont()->Lower()->IsColumnFrame()),
               "SwPageFrame::GetContentHeight(): No support for columns.");

    // In pages without columns, the content defines the size.
    long nBot = getFrameArea().Top() + nTop;
    const SwFrame *pFrame = Lower();
    while (pFrame)
    {
        long nTmp = 0;
        const SwFrame *pCnt = static_cast<const SwLayoutFrame*>(pFrame)->ContainsAny();
        while (pCnt && (pCnt->GetUpper() == pFrame ||
               static_cast<const SwLayoutFrame*>(pFrame)->IsAnLower(pCnt)))
        {
            nTmp += pCnt->getFrameArea().Height();
            if (pCnt->IsTextFrame() &&
                static_cast<const SwTextFrame*>(pCnt)->IsUndersized())
            {
                // This TextFrame would like to be a bit bigger.
                nTmp += static_cast<const SwTextFrame*>(pCnt)->GetParHeight()
                      - pCnt->getFramePrintArea().Height();
            }
            else if (pCnt->IsSctFrame())
            {
                // Grow if undersized, but don't shrink if oversized.
                const auto delta = static_cast<const SwSectionFrame*>(pCnt)->CalcUndersize();
                if (delta > 0)
                    nTmp += delta;
            }

            pCnt = pCnt->FindNext();
        }
        // Consider invalid body frame properties
        if (pFrame->IsBodyFrame() &&
            (!pFrame->isFrameAreaSizeValid() ||
            !pFrame->isFramePrintAreaValid()) &&
            (pFrame->getFrameArea().Height() < pFrame->getFramePrintArea().Height())
            )
        {
            nTmp = std::min(nTmp, pFrame->getFrameArea().Height());
        }
        else
        {
            // Assert invalid lower property
            OSL_ENSURE(!(pFrame->getFrameArea().Height() < pFrame->getFramePrintArea().Height()),
                "SwPageFrame::GetContentHeight(): Lower with frame height < printing height");
            nTmp += pFrame->getFrameArea().Height() - pFrame->getFramePrintArea().Height();
        }
        if (!pFrame->IsBodyFrame())
            nTmp = std::min(nTmp, pFrame->getFrameArea().Height());
        nBot += nTmp;
        // Here we check whether paragraph anchored objects
        // protrude outside the Body/FootnoteCont.
        if (m_pSortedObjs && !pFrame->IsHeaderFrame() &&
            !pFrame->IsFooterFrame())
            lcl_CheckObjects(*m_pSortedObjs, pFrame, nBot);
        pFrame = pFrame->GetNext();
    }
    nBot += nBottom;
    // And the page anchored ones
    if (m_pSortedObjs)
        lcl_CheckObjects(*m_pSortedObjs, this, nBot);
    nBot -= getFrameArea().Top();

    return nBot;
}

void SwPageFrame::MakeAll(vcl::RenderContext* pRenderContext)
{
    PROTOCOL_ENTER( this, PROT::MakeAll, DbgAction::NONE, nullptr )

    const SwRect aOldRect( getFrameArea() );     // Adjust root size
    const SwLayNotify aNotify( this );  // takes care of the notification in the dtor
    std::unique_ptr<SwBorderAttrAccess> pAccess;
    const SwBorderAttrs*pAttrs = nullptr;

    while ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
    {
        if ( !isFrameAreaPositionValid() )
        {
            setFrameAreaPositionValid(true); // positioning of the pages is taken care of by the root frame
        }

        if ( !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
        {
            if ( IsEmptyPage() )
            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                aFrm.Width( 0 );
                aFrm.Height( 0 );

                SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                aPrt.Width( 0 );
                aPrt.Height( 0 );
                aPrt.Left( 0 );
                aPrt.Top( 0 );

                setFrameAreaSizeValid(true);
                setFramePrintAreaValid(true);
            }
            else
            {
                if (!pAccess)
                {
                    pAccess = std::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), this);
                    pAttrs = pAccess->Get();
                }
                assert(pAttrs);

                SwRootFrame* pRootFrame = getRootFrame();
                SwViewShell* pSh = pRootFrame->GetCurrShell();
                if (pSh && pSh->GetViewOptions()->getBrowseMode())
                {
                    // In BrowseView, we use fixed settings
                    const Size aBorder = pRenderContext->PixelToLogic( pSh->GetBrowseBorder() );
                    const long nTop    = pAttrs->CalcTopLine()   + aBorder.Height();
                    const long nBottom = pAttrs->CalcBottomLine()+ aBorder.Height();

                    long nWidth = GetUpper() ? static_cast<SwRootFrame*>(GetUpper())->GetBrowseWidth() : 0;
                    const auto nDefWidth = pSh->GetBrowseWidth();
                    if (nWidth < nDefWidth)
                        nWidth = nDefWidth;
                    nWidth += + 2 * aBorder.Width();
                    nWidth = std::max( nWidth, 2L * aBorder.Width() + 4*MM50 );

                    {
                        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                        aFrm.Width( nWidth );

                        SwLayoutFrame *pBody = FindBodyCont();
                        if ( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrame() )
                        {
                            // Columns have a fixed height
                            aFrm.Height( pAttrs->GetSize().Height() );
                        }
                        else
                        {
                            // In pages without columns, the content defines the size.
                            long nBot = GetContentHeight(nTop, nBottom);

                            // #i35143# - If second page frame
                            // exists, the first page doesn't have to fulfill the
                            // visible area.
                            if ( !GetPrev() && !GetNext() )
                            {
                                nBot = std::max( nBot, pSh->VisArea().Height() );
                            }
                            // #i35143# - Assure, that the page
                            // doesn't exceed the defined browse height.
                            aFrm.Height( std::min( nBot, BROWSE_HEIGHT ) );
                        }
                    }

                    {
                        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                        aPrt.Left ( pAttrs->CalcLeftLine() + aBorder.Width() );
                        aPrt.Top  ( nTop );
                        aPrt.Width( getFrameArea().Width() - ( aPrt.Left() + pAttrs->CalcRightLine() + aBorder.Width() ) );
                        aPrt.Height( getFrameArea().Height() - (nTop + nBottom) );
                    }

                    setFrameAreaSizeValid(true);
                    setFramePrintAreaValid(true);
                    continue;
                }
                else if (pSh && pSh->GetViewOptions()->IsWhitespaceHidden() && pRootFrame->GetLastPage() != this)
                {
                    long height = 0;
                    SwLayoutFrame *pBody = FindBodyCont();
                    if ( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrame() )
                    {
                        // Columns have a fixed height
                        height = pAttrs->GetSize().Height();
                    }
                    else
                    {
                        // No need for borders.
                        height = GetContentHeight(0, 0);
                    }

                    if (height > 0)
                    {
                        ChgSize(Size(getFrameArea().Width(), height));
                        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                        aPrt.Top(0);
                        aPrt.Height(height);
                        setFrameAreaSizeValid(true);
                        setFramePrintAreaValid(true);
                        continue;
                    }

                    // Fallback to default formatting. Especially relevant
                    // when loading a doc when Hide Whitespace is enabled.
                    // Heights are zero initially.
                }

                // Set FixSize. For pages, this is not done from Upper, but from
                // the attribute.
                //FIXME: This resets the size when (isFrameAreaSizeValid() && !isFramePrintAreaValid()).
                {
                    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                    aFrm.SSize( pAttrs->GetSize() );
                }
                Format( pRenderContext, pAttrs );
            }
        }
    } //while ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() )

    if ( getFrameArea() != aOldRect && GetUpper() )
        static_cast<SwRootFrame*>(GetUpper())->CheckViewLayout( nullptr, nullptr );

    OSL_ENSURE( !GetUpper() || GetUpper()->getFramePrintArea().Width() >= getFrameArea().Width(),
        "Upper (Root) must be wide enough to contain the widest page");
}

void SwLayoutFrame::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    PROTOCOL_ENTER( this, PROT::MakeAll, DbgAction::NONE, nullptr )

    // takes care of the notification in the dtor
    const SwLayNotify aNotify( this );
    bool bVert = IsVertical();

    SwRectFn fnRect = ( IsNeighbourFrame() == bVert )? fnRectHori : ( IsVertLR() ? (IsVertLRBT() ? fnRectVertL2RB2T : fnRectVertL2R) : fnRectVert );

    std::unique_ptr<SwBorderAttrAccess> pAccess;
    const SwBorderAttrs*pAttrs = nullptr;

    while ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
    {
        if ( !isFrameAreaPositionValid() )
            MakePos();

        if ( GetUpper() )
        {
            // NEW TABLES
            if ( IsLeaveUpperAllowed() )
            {
                if ( !isFrameAreaSizeValid() )
                {
                    setFramePrintAreaValid(false);
                }
            }
            else
            {
                if ( !isFrameAreaSizeValid() )
                {
                    // Set FixSize; VarSize is set by Format() after calculating the PrtArea
                    setFramePrintAreaValid(false);

                    SwTwips nPrtWidth = (GetUpper()->getFramePrintArea().*fnRect->fnGetWidth)();
                    if( bVert && ( IsBodyFrame() || IsFootnoteContFrame() ) )
                    {
                        SwFrame* pNxt = GetPrev();
                        while( pNxt && !pNxt->IsHeaderFrame() )
                            pNxt = pNxt->GetPrev();
                        if( pNxt )
                            nPrtWidth -= pNxt->getFrameArea().Height();
                        pNxt = GetNext();
                        while( pNxt && !pNxt->IsFooterFrame() )
                            pNxt = pNxt->GetNext();
                        if( pNxt )
                            nPrtWidth -= pNxt->getFrameArea().Height();
                    }

                    const long nDiff = nPrtWidth - (getFrameArea().*fnRect->fnGetWidth)();
                    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);

                    if( IsNeighbourFrame() && IsRightToLeft() )
                    {
                        (aFrm.*fnRect->fnSubLeft)( nDiff );
                    }
                    else
                    {
                        (aFrm.*fnRect->fnAddRight)( nDiff );
                    }
                }
                else
                {
                    // Don't leave your upper
                    const SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
                    if( (getFrameArea().*fnRect->fnOverStep)( nDeadLine ) )
                    {
                        setFrameAreaSizeValid(false);
                    }
                }
            }
        }

        if ( !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
        {
            if ( !pAccess )
            {
                pAccess = std::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), this);
                pAttrs  = pAccess->Get();
            }
            Format( getRootFrame()->GetCurrShell()->GetOut(), pAttrs );
        }
    } //while ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
}

bool SwTextNode::IsCollapse() const
{
    if (GetDoc()->GetDocumentSettingManager().get( DocumentSettingId::COLLAPSE_EMPTY_CELL_PARA )
        &&  GetText().isEmpty())
    {
        sal_uLong nIdx=GetIndex();
        const SwEndNode *pNdBefore=GetNodes()[nIdx-1]->GetEndNode();
        const SwEndNode *pNdAfter=GetNodes()[nIdx+1]->GetEndNode();

        // The paragraph is collapsed only if the NdAfter is the end of a cell
        bool bInTable = FindTableNode( ) != nullptr;

        SwSortedObjs* pObjs = getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() )->GetDrawObjs( );
        const size_t nObjs = ( pObjs != nullptr ) ? pObjs->size( ) : 0;

        return pNdBefore!=nullptr && pNdAfter!=nullptr && nObjs == 0 && bInTable;
    }

    return false;
}

bool SwFrame::IsCollapse() const
{
    if (!IsTextFrame())
        return false;

    const SwTextFrame *pTextFrame = static_cast<const SwTextFrame*>(this);
    const SwTextNode *pTextNode = pTextFrame->GetTextNodeForParaProps();
    // TODO this SwTextNode function is pointless and should be merged in here
    return pTextFrame->GetText().isEmpty() && pTextNode && pTextNode->IsCollapse();
}

void SwContentFrame::MakePrtArea( const SwBorderAttrs &rAttrs )
{
    if ( !isFramePrintAreaValid() )
    {
        setFramePrintAreaValid(true);
        SwRectFnSet aRectFnSet(this);
        const bool bTextFrame = IsTextFrame();
        SwTwips nUpper = 0;
        if ( bTextFrame && static_cast<SwTextFrame*>(this)->IsHiddenNow() )
        {
            if ( static_cast<SwTextFrame*>(this)->HasFollow() )
                static_cast<SwTextFrame*>(this)->JoinFrame();

            if( aRectFnSet.GetHeight(getFramePrintArea()) )
            {
                static_cast<SwTextFrame*>(this)->HideHidden();
            }

            {
                SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                aPrt.Pos().setX(0);
                aPrt.Pos().setY(0);
                aRectFnSet.SetWidth( aPrt, aRectFnSet.GetWidth(getFrameArea()) );
                aRectFnSet.SetHeight( aPrt, 0 );
            }

            nUpper = -( aRectFnSet.GetHeight(getFrameArea()) );
        }
        else
        {
            // Simplification: ContentFrames are always variable in height!

            // At the FixSize, the surrounding Frame enforces the size;
            // the borders are simply subtracted.
            const long nLeft = rAttrs.CalcLeft( this );
            const long nRight = rAttrs.CalcRight( this );
            aRectFnSet.SetXMargins( *this, nLeft, nRight );

            SwViewShell *pSh = getRootFrame()->GetCurrShell();
            SwTwips nWidthArea;
            if( pSh && 0!=(nWidthArea=aRectFnSet.GetWidth(pSh->VisArea())) &&
                GetUpper()->IsPageBodyFrame() && // but not for BodyFrames in Columns
                pSh->GetViewOptions()->getBrowseMode() )
            {
                // Do not protrude the edge of the visible area. The page may be
                // wider, because there may be objects with excess width
                // (RootFrame::ImplCalcBrowseWidth())
                long nMinWidth = 0;

                for (size_t i = 0; GetDrawObjs() && i < GetDrawObjs()->size(); ++i)
                {
                    // #i28701# - consider changed type of
                    // <SwSortedObjs> entries
                    SwAnchoredObject* pObj = (*GetDrawObjs())[i];
                    const SwFrameFormat& rFormat = pObj->GetFrameFormat();
                    const bool bFly = dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr;
                    if ((bFly && (FAR_AWAY == pObj->GetObjRect().Width()))
                        || rFormat.GetFrameSize().GetWidthPercent())
                    {
                        continue;
                    }

                    if ( RndStdIds::FLY_AS_CHAR == rFormat.GetAnchor().GetAnchorId() )
                    {
                        nMinWidth = std::max( nMinWidth,
                                         bFly ? rFormat.GetFrameSize().GetWidth()
                                              : pObj->GetObjRect().Width() );
                    }
                }

                const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
                long nWidth = nWidthArea - 2 * ( IsVertical() ? aBorder.Height() : aBorder.Width() );
                nWidth -= aRectFnSet.GetLeft(getFramePrintArea());
                nWidth -= rAttrs.CalcRightLine();
                nWidth = std::max( nMinWidth, nWidth );

                SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                aRectFnSet.SetWidth( aPrt, std::min( nWidth, aRectFnSet.GetWidth(aPrt) ) );
            }

            if ( aRectFnSet.GetWidth(getFramePrintArea()) <= MINLAY )
            {
                // The PrtArea should already be at least MINLAY wide, matching the
                // minimal values of the UI
                SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                aRectFnSet.SetWidth( aPrt, std::min( long(MINLAY), aRectFnSet.GetWidth(getFrameArea()) ) );
                SwTwips nTmp = aRectFnSet.GetWidth(getFrameArea()) - aRectFnSet.GetWidth(aPrt);

                if( aRectFnSet.GetLeft(aPrt) > nTmp )
                {
                    aRectFnSet.SetLeft( aPrt, nTmp );
                }
            }

            // The following rules apply for VarSize:
            // 1. The first entry of a chain has no top border
            // 2. There is never a bottom border
            // 3. The top border is the maximum of the distance
            //    of Prev downwards and our own distance upwards
            // Those three rules apply when calculating spacings
            // that are given by UL- and LRSpace. There might be a spacing
            // in all directions however; this may be caused by borders
            // and / or shadows.
            // 4. The spacing for TextFrames corresponds to the interline lead,
            //    at a minimum.

            nUpper = CalcUpperSpace( &rAttrs );

            SwTwips nLower = CalcLowerSpace( &rAttrs );
            if (IsCollapse()) {
                nUpper=0;
                nLower=0;
            }

            {
                SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                aRectFnSet.SetPosY( aPrt, !aRectFnSet.IsVert() ? nUpper : nLower);
            }

            nUpper += nLower;
            nUpper -= aRectFnSet.GetHeight(getFrameArea()) - aRectFnSet.GetHeight(getFramePrintArea());
        }
        // If there's a difference between old and new size, call Grow() or
        // Shrink() respectively.
        if ( nUpper )
        {
            if ( nUpper > 0 )
                GrowFrame( nUpper );
            else
                ShrinkFrame( -nUpper );
        }
    }
}

#define STOP_FLY_FORMAT 10
// - loop prevention
const int cnStopFormat = 15;

inline void ValidateSz( SwFrame *pFrame )
{
    if ( pFrame )
    {
        pFrame->setFrameAreaSizeValid(true);
        pFrame->setFramePrintAreaValid(true);
    }
}

void SwContentFrame::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    OSL_ENSURE( GetUpper(), "no Upper?" );
    OSL_ENSURE( IsTextFrame(), "MakeAll(), NoText" );

    if ( !IsFollow() && StackHack::IsLocked() )
        return;

    if ( IsJoinLocked() )
        return;

    OSL_ENSURE( !static_cast<SwTextFrame*>(this)->IsSwapped(), "Calculation of a swapped frame" );

    StackHack aHack;

    if ( static_cast<SwTextFrame*>(this)->IsLocked() )
    {
        OSL_FAIL( "Format for locked TextFrame." );
        return;
    }

    auto xDeleteGuard = std::make_unique<SwFrameDeleteGuard>(this);
    LockJoin();
    long nFormatCount = 0;
    // - loop prevention
    int nConsecutiveFormatsWithoutChange = 0;
    PROTOCOL_ENTER( this, PROT::MakeAll, DbgAction::NONE, nullptr )

    // takes care of the notification in the dtor
    std::unique_ptr<SwContentNotify, o3tl::default_delete<SwContentNotify>> pNotify(new SwContentNotify( this ));

    // as long as bMakePage is true, a new page can be created (exactly once)
    bool bMakePage = true;
    // bMovedBwd gets set to true when the frame flows backwards
    bool bMovedBwd = false;
    // as long as bMovedFwd is false, the Frame may flow backwards (until
    // it has been moved forward once)
    bool bMovedFwd = false;
    sal_Bool bFormatted = false;        // For the widow/orphan rules, we encourage the
                                            // last ContentFrame of a chain to format. This only
                                            // needs to happen once. Every time the Frame is
                                            // moved, the flag will have to be reset.
    bool bMustFit = false;                  // Once the emergency brake is pulled,
                                            // no other prepares will be triggered
    bool bFitPromise = false;               // If a paragraph didn't fit, but promises
                                            // with WouldFit that it would adjust accordingly,
                                            // this flag is set. If it turns out that it
                                            // didn't keep it's promise, we can act in a
                                            // controlled fashion.
    const bool bFly = IsInFly();
    const bool bTab = IsInTab();
    const bool bFootnote = IsInFootnote();
    const bool bSct = IsInSct();
    Point aOldFramePos;               // This is so we can compare with the last pos
    Point aOldPrtPos;               // and determine whether it makes sense to Prepare

    SwBorderAttrAccess aAccess( SwFrame::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    if ( !IsFollow() && rAttrs.JoinedWithPrev( *(this) ) )
    {
        pNotify->SetBordersJoinedWithPrev();
    }

    const bool bKeep = IsKeep(rAttrs.GetAttrSet().GetKeep(), GetBreakItem());

    std::unique_ptr<SwSaveFootnoteHeight> pSaveFootnote;
    if ( bFootnote )
    {
        SwFootnoteFrame *pFootnote = FindFootnoteFrame();
        SwSectionFrame* pSct = pFootnote->FindSctFrame();
        if ( !static_cast<SwTextFrame*>(pFootnote->GetRef())->IsLocked() )
        {
            SwFootnoteBossFrame* pBoss = pFootnote->GetRef()->FindFootnoteBossFrame(
                                    pFootnote->GetAttr()->GetFootnote().IsEndNote() );
            if( !pSct || pSct->IsColLocked() || !pSct->Growable() )
                pSaveFootnote.reset( new SwSaveFootnoteHeight( pBoss,
                    static_cast<SwTextFrame*>(pFootnote->GetRef())->GetFootnoteLine( pFootnote->GetAttr() ) ) );
        }
    }

    if ( GetUpper()->IsSctFrame() &&
         HasFollow() &&
         &GetFollow()->GetFrame() == GetNext() )
    {
        dynamic_cast<SwTextFrame&>(*this).JoinFrame();
    }

    // #i28701# - move master forward, if it has to move,
    // because of its object positioning.
    if ( !static_cast<SwTextFrame*>(this)->IsFollow() )
    {
        sal_uInt32 nToPageNum = 0;
        const bool bMoveFwdByObjPos = SwLayouter::FrameMovedFwdByObjPos(
                                                    *(GetAttrSet()->GetDoc()),
                                                    *static_cast<SwTextFrame*>(this),
                                                    nToPageNum );
        // #i58182#
        // Also move a paragraph forward, which is the first one inside a table cell.
        if ( bMoveFwdByObjPos &&
             FindPageFrame()->GetPhyPageNum() < nToPageNum &&
             ( lcl_Prev( this ) ||
               GetUpper()->IsCellFrame() ||
               ( GetUpper()->IsSctFrame() &&
                 GetUpper()->GetUpper()->IsCellFrame() ) ) &&
             IsMoveable() )
        {
            bMovedFwd = true;
            MoveFwd( bMakePage, false );
        }
    }

    // If a Follow sits next to its Master and doesn't fit, we know it can
    // be moved right now.
    if ( lcl_Prev( this ) && static_cast<SwTextFrame*>(this)->IsFollow() && IsMoveable() )
    {
        bMovedFwd = true;
        // If follow frame is in table, its master will be the last in the
        // current table cell. Thus, invalidate the printing area of the master.
        if ( IsInTab() )
        {
            lcl_Prev( this )->InvalidatePrt();
        }
        MoveFwd( bMakePage, false );
    }

    // Check footnote content for forward move.
    // If a content of a footnote is on a prior page/column as its invalid
    // reference, it can be moved forward.
    if ( bFootnote && !isFrameAreaPositionValid() )
    {
        SwFootnoteFrame* pFootnote = FindFootnoteFrame();
        SwContentFrame* pRefCnt = pFootnote ? pFootnote->GetRef() : nullptr;

        if ( pRefCnt && !pRefCnt->isFrameAreaDefinitionValid() )
        {
            SwFootnoteBossFrame* pFootnoteBossOfFootnote = pFootnote->FindFootnoteBossFrame();
            SwFootnoteBossFrame* pFootnoteBossOfRef = pRefCnt->FindFootnoteBossFrame();
            //<loop of movefwd until condition held or no move>
            if ( pFootnoteBossOfFootnote && pFootnoteBossOfRef &&
                 pFootnoteBossOfFootnote != pFootnoteBossOfRef &&
                 pFootnoteBossOfFootnote->IsBefore( pFootnoteBossOfRef ) )
            {
                bMovedFwd = true;
                MoveFwd( bMakePage, false );
            }
        }
    }

    SwRectFnSet aRectFnSet(this);

    SwFrame const* pMoveBwdPre(nullptr);
    bool isMoveBwdPreValid(false);

    SwRect aOldFrame_StopFormat, aOldFrame_StopFormat2;
    SwRect aOldPrt_StopFormat, aOldPrt_StopFormat2;

    while ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
    {
        // - loop prevention
        aOldFrame_StopFormat2 = aOldFrame_StopFormat;
        aOldPrt_StopFormat2 = aOldPrt_StopFormat;
        aOldFrame_StopFormat = getFrameArea();
        aOldPrt_StopFormat = getFramePrintArea();

        bool bMoveable = IsMoveable();
        if (bMoveable)
        {
            SwFrame *pPre = GetIndPrev();
            if ( CheckMoveFwd( bMakePage, bKeep, bMovedBwd ) )
            {
                aRectFnSet.Refresh(this);
                bMovedFwd = true;
                if ( bMovedBwd )
                {
                    // While flowing back, the Upper was encouraged to
                    // completely re-paint itself. We can skip this now after
                    // flowing back and forth.
                    GetUpper()->ResetCompletePaint();
                    // The predecessor was invalidated, so this is obsolete as well now.
                    assert(pPre);
                    if ((pPre == pMoveBwdPre && isMoveBwdPreValid) && !pPre->IsSctFrame())
                        ::ValidateSz( pPre );
                }
                bMoveable = IsMoveable();
            }
        }

        aOldFramePos = aRectFnSet.GetPos(getFrameArea());
        aOldPrtPos = aRectFnSet.GetPos(getFramePrintArea());

        if ( !isFrameAreaPositionValid() )
            MakePos();

        //Set FixSize. VarSize is being adjusted by Format().
        if ( !isFrameAreaSizeValid() )
        {
            // invalidate printing area flag, if the following conditions are hold:
            // - current frame width is 0.
            // - current printing area width is 0.
            // - frame width is adjusted to a value greater than 0.
            // - printing area flag is true.
            // Thus, it's assured that the printing area is adjusted, if the
            // frame area width changes its width from 0 to something greater
            // than 0.
            // Note: A text frame can be in such a situation, if the format is
            //       triggered by method call <SwCursorShell::SetCursor()> after
            //       loading the document.
            const SwTwips nNewFrameWidth = aRectFnSet.GetWidth(GetUpper()->getFramePrintArea());

            if ( isFramePrintAreaValid() &&
                nNewFrameWidth > 0 &&
                aRectFnSet.GetWidth(getFrameArea()) == 0 &&
                aRectFnSet.GetWidth(getFramePrintArea()) == 0 )
            {
                setFramePrintAreaValid(false);
            }

            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                aRectFnSet.SetWidth( aFrm, nNewFrameWidth );
            }

            // When a lower of a vertically aligned fly frame changes its size we need to recalculate content pos.
            if( GetUpper() && GetUpper()->IsFlyFrame() &&
                GetUpper()->GetFormat()->GetTextVertAdjust().GetValue() != SDRTEXTVERTADJUST_TOP )
            {
                static_cast<SwFlyFrame*>(GetUpper())->InvalidateContentPos();
                GetUpper()->SetCompletePaint();
            }
        }
        if ( !isFramePrintAreaValid() )
        {
            const long nOldW = aRectFnSet.GetWidth(getFramePrintArea());
            // #i34730# - keep current frame height
            const SwTwips nOldH = aRectFnSet.GetHeight(getFrameArea());
            MakePrtArea( rAttrs );
            if ( nOldW != aRectFnSet.GetWidth(getFramePrintArea()) )
                Prepare( PREP_FIXSIZE_CHG );
            // #i34730# - check, if frame height has changed.
            // If yes, send a PREP_ADJUST_FRM and invalidate the size flag to
            // force a format. The format will check in its method
            // <SwTextFrame::CalcPreps()>, if the already formatted lines still
            // fit and if not, performs necessary actions.
            // #i40150# - no check, if frame is undersized.
            if ( isFrameAreaSizeValid() && !IsUndersized() && nOldH != aRectFnSet.GetHeight(getFrameArea()) )
            {
                // #115759# - no PREP_ADJUST_FRM and size
                // invalidation, if height decreases only by the additional
                // lower space as last content of a table cell and an existing
                // follow containing one line exists.
                const SwTwips nHDiff = nOldH - aRectFnSet.GetHeight(getFrameArea());
                const bool bNoPrepAdjustFrame =
                    nHDiff > 0 && IsInTab() && GetFollow() &&
                    (1 == static_cast<SwTextFrame*>(GetFollow())->GetLineCount(TextFrameIndex(COMPLETE_STRING))
                     || aRectFnSet.GetWidth(static_cast<SwTextFrame*>(GetFollow())->getFrameArea()) < 0) &&
                    GetFollow()->CalcAddLowerSpaceAsLastInTableCell() == nHDiff;
                if ( !bNoPrepAdjustFrame )
                {
                    Prepare( PREP_ADJUST_FRM );
                    setFrameAreaSizeValid(false);
                }
            }
        }

        // To make the widow and orphan rules work, we need to notify the ContentFrame.
        // Criteria:
        // - It needs to be movable (otherwise, splitting doesn't make sense)
        // - It needs to overlap with the lower edge of the PrtArea of the Upper
        if ( !bMustFit )
        {
            bool bWidow = true;
            const SwTwips nDeadLine = aRectFnSet.GetPrtBottom(*GetUpper());
            if( bMoveable && !bFormatted &&
                ( GetFollow() || aRectFnSet.OverStep( getFrameArea(), nDeadLine ) ) )
            {
                Prepare( PREP_WIDOWS_ORPHANS, nullptr, false );
                setFrameAreaSizeValid(false);
                bWidow = false;
            }
            if( aRectFnSet.GetPos(getFrameArea()) != aOldFramePos ||
                aRectFnSet.GetPos(getFramePrintArea()) != aOldPrtPos )
            {
                // In this Prepare, an InvalidateSize_() might happen.
                // isFrameAreaSizeValid() becomes false and Format() gets called.
                Prepare( PREP_POS_CHGD, static_cast<const void*>(&bFormatted), false );
                if ( bWidow && GetFollow() )
                {
                    Prepare( PREP_WIDOWS_ORPHANS, nullptr, false );
                    setFrameAreaSizeValid(false);
                }
            }
        }
        if ( !isFrameAreaSizeValid() )
        {
            setFrameAreaSizeValid(true);
            bFormatted = true;
            ++nFormatCount;
            if( nFormatCount > STOP_FLY_FORMAT )
                SetFlyLock( true );
            // - loop prevention
            // No format any longer, if <cnStopFormat> consecutive formats
            // without change occur.
            if ( nConsecutiveFormatsWithoutChange <= cnStopFormat )
            {
                Format(getRootFrame()->GetCurrShell()->GetOut());
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OSL_FAIL( "debug assertion: <SwContentFrame::MakeAll()> - format of text frame suppressed by fix b6448963" );
            }
#endif
        }

        // If this is the first one in a chain, check if this can flow
        // backwards (if this is movable at all).
        // To prevent oscillations/loops, check that this has not just
        // flowed forwards.
        bool bDummy;
        auto const pTemp(GetIndPrev());
        auto const bTemp(pTemp && pTemp->isFrameAreaSizeValid()
                               && pTemp->isFramePrintAreaValid());
        if ( !lcl_Prev( this ) &&
             !bMovedFwd &&
             ( bMoveable || ( bFly && !bTab ) ) &&
             ( !bFootnote || !GetUpper()->FindFootnoteFrame()->GetPrev() )
             && MoveBwd( bDummy ) )
        {
            aRectFnSet.Refresh(this);
            pMoveBwdPre = pTemp;
            isMoveBwdPreValid = bTemp;
            bMovedBwd = true;
            bFormatted = false;
            if ( bKeep && bMoveable )
            {
                if( CheckMoveFwd( bMakePage, false, bMovedBwd ) )
                {
                    bMovedFwd = true;
                    bMoveable = IsMoveable();
                    aRectFnSet.Refresh(this);
                }
                Point aOldPos = aRectFnSet.GetPos(getFrameArea());
                MakePos();
                if( aOldPos != aRectFnSet.GetPos(getFrameArea()) )
                {
                    Prepare( PREP_POS_CHGD, static_cast<const void*>(&bFormatted), false );
                    if ( !isFrameAreaSizeValid() )
                    {
                        {
                            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                            aRectFnSet.SetWidth( aFrm, aRectFnSet.GetWidth(GetUpper()->getFramePrintArea()) );
                        }

                        if ( !isFramePrintAreaValid() )
                        {
                            const long nOldW = aRectFnSet.GetWidth(getFramePrintArea());
                            MakePrtArea( rAttrs );
                            if( nOldW != aRectFnSet.GetWidth(getFramePrintArea()) )
                                Prepare( PREP_FIXSIZE_CHG, nullptr, false );
                        }
                        if( GetFollow() )
                        {
                            Prepare( PREP_WIDOWS_ORPHANS, nullptr, false );
                        }

                        setFrameAreaSizeValid(true);
                        bFormatted = true;
                        Format(getRootFrame()->GetCurrShell()->GetOut());
                    }
                }
                SwFrame *pNxt = HasFollow() ? nullptr : FindNext();
                while( pNxt && pNxt->IsSctFrame() )
                {   // Leave empty sections out, go into the other ones.
                    if( static_cast<SwSectionFrame*>(pNxt)->GetSection() )
                    {
                        SwFrame* pTmp = static_cast<SwSectionFrame*>(pNxt)->ContainsAny();
                        if( pTmp )
                        {
                            pNxt = pTmp;
                            break;
                        }
                    }
                    pNxt = pNxt->FindNext();
                }
                if ( pNxt )
                {
                    pNxt->Calc(getRootFrame()->GetCurrShell()->GetOut());
                    if( isFrameAreaPositionValid() && !GetIndNext() )
                    {
                        SwSectionFrame *pSct = FindSctFrame();
                        if( pSct && !pSct->isFrameAreaSizeValid() )
                        {
                            SwSectionFrame* pNxtSct = pNxt->FindSctFrame();
                            if( pNxtSct && pSct->IsAnFollow( pNxtSct ) )
                            {
                                setFrameAreaPositionValid(false);
                            }
                        }
                        else
                        {
                            setFrameAreaPositionValid(false);
                        }
                    }
                }
            }
        }

        // In footnotes, the TextFrame may validate itself, which can lead to the
        // situation that it's position is wrong despite being "valid".
        if ( isFrameAreaPositionValid() )
        {
            // #i59341#
            // Workaround for inadequate layout algorithm:
            // suppress invalidation and calculation of position, if paragraph
            // has formatted itself at least STOP_FLY_FORMAT times and
            // has anchored objects.
            // Thus, the anchored objects get the possibility to format itself
            // and this probably solve the layout loop.
            if ( bFootnote &&
                 nFormatCount <= STOP_FLY_FORMAT &&
                 !GetDrawObjs() )
            {
                setFrameAreaPositionValid(false);
                MakePos();
                aOldFramePos = aRectFnSet.GetPos(getFrameArea());
                aOldPrtPos = aRectFnSet.GetPos(getFramePrintArea());
            }
        }

        // - loop prevention
        {
            if ( (aOldFrame_StopFormat == getFrameArea() || aOldFrame_StopFormat2 == getFrameArea() ) &&
                 (aOldPrt_StopFormat == getFramePrintArea() || aOldPrt_StopFormat2 == getFramePrintArea()))
            {
                ++nConsecutiveFormatsWithoutChange;
            }
            else
            {
                nConsecutiveFormatsWithoutChange = 0;
            }
        }

        // Yet again an invalid value? Repeat from the start...
        if ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
            continue;

        // Done?
        // Attention: because height == 0, it's better to use Top()+Height() instead of
        // Bottom(). This might happen with undersized TextFrames on the lower edge of a
        // multi-column section
        const long nPrtBottom = aRectFnSet.GetPrtBottom(*GetUpper());
        long nBottomDist = aRectFnSet.BottomDist(getFrameArea(), nPrtBottom);

        // Hide whitespace may require not to insert a new page.
        SwPageFrame* pPageFrame = FindPageFrame();
        const bool bHeightValid = pPageFrame->CheckPageHeightValidForHideWhitespace(nBottomDist);
        if (!bHeightValid)
        {
            pPageFrame->InvalidateSize();
            nBottomDist = 0;
        }

        if( nBottomDist >= 0 )
        {
            if ( bKeep && bMoveable )
            {
                // We make sure the successor will be formatted the same.
                // This way, we keep control until (almost) everything is stable,
                // allowing us to avoid endless loops caused by ever repeating
                // retries.

                // bMoveFwdInvalid is required for #38407#. This was originally solved
                // in flowfrm.cxx rev 1.38, but broke the above schema and
                // preferred to play towers of hanoi (#43669#).
                SwFrame *pNxt = HasFollow() ? nullptr : FindNext();
                // For sections we prefer the content, because it can change
                // the page if required.
                while( pNxt && pNxt->IsSctFrame() )
                {
                    if( static_cast<SwSectionFrame*>(pNxt)->GetSection() )
                    {
                        pNxt = static_cast<SwSectionFrame*>(pNxt)->ContainsAny();
                        break;
                    }
                    pNxt = pNxt->FindNext();
                }
                if ( pNxt )
                {
                    const bool bMoveFwdInvalid = nullptr != GetIndNext();
                    const bool bNxtNew =
                        ( 0 == aRectFnSet.GetHeight(pNxt->getFramePrintArea()) ) &&
                        (!pNxt->IsTextFrame() ||!static_cast<SwTextFrame*>(pNxt)->IsHiddenNow());

                    pNxt->Calc(getRootFrame()->GetCurrShell()->GetOut());

                    if ( !bMovedBwd &&
                         ((bMoveFwdInvalid && !GetIndNext()) ||
                          bNxtNew) )
                    {
                        if( bMovedFwd )
                            pNotify->SetInvaKeep();
                        bMovedFwd = false;
                    }
                }
            }
            continue;
        }

        // I don't fit into my parents, so it's time to make changes
        // as constructively as possible.

        //If I'm NOT allowed to leave the parent Frame, I've got a problem.
        // Following Arthur Dent, we do the only thing that you can do with
        // an unsolvable problem: We ignore it with all our power.
        if ( !bMoveable || IsUndersized() )
        {
            if( !bMoveable && IsInTab() )
            {
                long nDiff = -aRectFnSet.BottomDist( getFrameArea(), aRectFnSet.GetPrtBottom(*GetUpper()) );
                long nReal = GetUpper()->Grow( nDiff );
                if( nReal )
                    continue;
            }
            break;
        }

        // If there's no way I can make myself fit into my Upper, the situation
        // could still probably be mitigated by splitting up.
        // This situation arises with freshly created Follows that had been moved
        // to the next page but is still too big for it - ie. needs to be split
        // as well.

        // If I'm unable to split (WouldFit()) and can't be fitted, I'm going
        // to tell my TextFrame part that, if possible, we still need to split despite
        // the "don't split" attribute.
        bool bMoveOrFit = false;
        bool bDontMoveMe = !GetIndPrev();
        if( bDontMoveMe && IsInSct() )
        {
            SwFootnoteBossFrame* pBoss = FindFootnoteBossFrame();
            bDontMoveMe = !pBoss->IsInSct() ||
                          ( !pBoss->Lower()->GetNext() && !pBoss->GetPrev() );
        }

        // Finally, we are able to split table rows. Therefore, bDontMoveMe
        // can be set to false:
        if( bDontMoveMe && IsInTab() &&
            nullptr != GetNextCellLeaf() )
            bDontMoveMe = false;

        assert(bMoveable);

        if ( bDontMoveMe && aRectFnSet.GetHeight(getFrameArea()) >
                            aRectFnSet.GetHeight(GetUpper()->getFramePrintArea()) )
        {
            if ( !bFitPromise )
            {
                SwTwips nTmp = aRectFnSet.GetHeight(GetUpper()->getFramePrintArea()) -
                               aRectFnSet.GetTop(getFramePrintArea());
                bool bSplit = !IsFwdMoveAllowed();
                if ( nTmp > 0 && WouldFit( nTmp, bSplit, false ) )
                {
                    Prepare( PREP_WIDOWS_ORPHANS, nullptr, false );
                    setFrameAreaSizeValid(false);
                    bFitPromise = true;
                    continue;
                }
                /*
                 * In earlier days, we never tried to fit TextFrames in
                 * frames and sections using bMoveOrFit by ignoring
                 * its attributes (Widows, Keep).
                 * This should have been done at least for column frames;
                 * as it must be tried anyway with linked frames and sections.
                 * Exception: If we sit in FormatWidthCols, we must not ignore
                 * the attributes.
                 */
                else if ( !bFootnote &&
                      ( !bFly || !FindFlyFrame()->IsColLocked() ) &&
                      ( !bSct || !FindSctFrame()->IsColLocked() ) )
                    bMoveOrFit = true;
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OSL_FAIL( "+TextFrame didn't respect WouldFit promise." );
            }
#endif
        }

        // Let's see if I can find some space somewhere...
        // footnotes in the neighbourhood are moved into _MoveFootnoteCntFwd
        SwFrame *pPre = GetIndPrev();
        SwFrame *pOldUp = GetUpper();

/* MA 13. Oct. 98: What is this supposed to be!?
 * AMA 14. Dec 98: If a column section can't find any space for its first ContentFrame, it should be
 *                 moved not only to the next column, but probably even to the next page, creating
 *                 a section-follow there.
 */
        if( IsInSct() && bMovedFwd && bMakePage && pOldUp->IsColBodyFrame() &&
            pOldUp->GetUpper()->GetUpper()->IsSctFrame() &&
            ( pPre || pOldUp->GetUpper()->GetPrev() ) &&
            static_cast<SwSectionFrame*>(pOldUp->GetUpper()->GetUpper())->MoveAllowed(this) )
        {
            bMovedFwd = false;
        }

        const bool bCheckForGrownBody = pOldUp->IsBodyFrame();
        const long nOldBodyHeight = aRectFnSet.GetHeight(pOldUp->getFrameArea());

        if ( !bMovedFwd && !MoveFwd( bMakePage, false ) )
            bMakePage = false;
        aRectFnSet.Refresh(this);
        if (!bMovedFwd && bFootnote && GetIndPrev() != pPre)
        {   // SwFlowFrame::CutTree() could have formatted and deleted pPre
            auto const pPrevFootnoteFrame(static_cast<SwFootnoteFrame const*>(
                        FindFootnoteFrame())->GetMaster());
            bool bReset = true;
            if (pPrevFootnoteFrame)
            {   // use GetIndNext() in case there are sections
                for (auto p = pPrevFootnoteFrame->Lower(); p; p = p->GetIndNext())
                {
                    if (p == pPre)
                    {
                        bReset = false;
                        break;
                    }
                }
            }
            if (bReset)
            {
                pPre = nullptr;
            }
        }

        // If MoveFwd moves the paragraph to the next page, a following
        // paragraph, which contains footnotes can cause the old upper
        // frame to grow. In this case we explicitly allow a new check
        // for MoveBwd. Robust: We also check the bMovedBwd flag again.
        // If pOldUp was a footnote frame, it has been deleted inside MoveFwd.
        // Therefore we only check for growing body frames.
        bMovedFwd = !bCheckForGrownBody || bMovedBwd || pOldUp == GetUpper() ||
                    aRectFnSet.GetHeight(pOldUp->getFrameArea()) <= nOldBodyHeight;

        bFormatted = false;
        if ( bMoveOrFit && GetUpper() == pOldUp )
        {
            // FME 2007-08-30 #i81146# new loop control
            if ( nConsecutiveFormatsWithoutChange <= cnStopFormat )
            {
                Prepare( PREP_MUST_FIT, nullptr, false );
                setFrameAreaSizeValid(false);
                bMustFit = true;
                continue;
            }

#if OSL_DEBUG_LEVEL > 0
            OSL_FAIL( "LoopControl in SwContentFrame::MakeAll" );
#endif
        }
        if ( bMovedBwd && GetUpper() )
        {   // Retire invalidations that have become useless.
            GetUpper()->ResetCompletePaint();
            if( pPre && !pPre->IsSctFrame() )
                ::ValidateSz( pPre );
        }

    } //while ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() )

    // NEW: Looping Louie (Light). Should not be applied in balanced sections.
    // Should only be applied if there is no better solution!
    LOOPING_LOUIE_LIGHT( bMovedFwd && bMovedBwd && !IsInBalancedSection() &&
                            (

                                ( bFootnote && !FindFootnoteFrame()->GetRef()->IsInSct() ) ||

                                // #i33887#
                                ( IsInSct() && bKeep )

                                // ... add your conditions here ...

                            ),
                         static_cast<SwTextFrame&>(*this) );

    pSaveFootnote.reset();

    UnlockJoin();
    xDeleteGuard.reset();
    if ( bMovedFwd || bMovedBwd )
        pNotify->SetInvaKeep();
    if ( bMovedFwd )
    {
        pNotify->SetInvalidatePrevPrtArea();
    }
    pNotify.reset();
    SetFlyLock( false );
}

void MakeNxt( SwFrame *pFrame, SwFrame *pNxt )
{
    // fix(25455): Validate, otherwise this leads to a recursion.
    // The first try, cancelling with pFrame = 0 if !Valid, leads to a problem, as
    // the Keep may not be considered properly anymore (27417).
    const bool bOldPos = pFrame->isFrameAreaPositionValid();
    const bool bOldSz  = pFrame->isFrameAreaSizeValid();
    const bool bOldPrt = pFrame->isFramePrintAreaValid();
    pFrame->setFrameAreaPositionValid(true);
    pFrame->setFrameAreaSizeValid(true);
    pFrame->setFramePrintAreaValid(true);

    // fix(29272): Don't call MakeAll - there, pFrame might be invalidated again, and
    // we recursively end up in here again.
    if ( pNxt->IsContentFrame() )
    {
        SwContentNotify aNotify( static_cast<SwContentFrame*>(pNxt) );
        SwBorderAttrAccess aAccess( SwFrame::GetCache(), pNxt );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( !pNxt->isFrameAreaSizeValid() )
        {
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pNxt);

            if( pNxt->IsVertical() )
            {
                aFrm.Height( pNxt->GetUpper()->getFramePrintArea().Height() );
            }
            else
            {
                aFrm.Width( pNxt->GetUpper()->getFramePrintArea().Width() );
            }
        }
        static_cast<SwContentFrame*>(pNxt)->MakePrtArea( rAttrs );
        pNxt->Format( pNxt->getRootFrame()->GetCurrShell()->GetOut(), &rAttrs );
    }
    else
    {
        SwLayNotify aNotify( static_cast<SwLayoutFrame*>(pNxt) );
        SwBorderAttrAccess aAccess( SwFrame::GetCache(), pNxt );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( !pNxt->isFrameAreaSizeValid() )
        {
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pNxt);

            if( pNxt->IsVertical() )
            {
                aFrm.Height( pNxt->GetUpper()->getFramePrintArea().Height() );
            }
            else
            {
                aFrm.Width( pNxt->GetUpper()->getFramePrintArea().Width() );
            }
        }
        pNxt->Format( pNxt->getRootFrame()->GetCurrShell()->GetOut(), &rAttrs );
    }

    pFrame->setFrameAreaPositionValid(bOldPos);
    pFrame->setFrameAreaSizeValid(bOldSz);
    pFrame->setFramePrintAreaValid(bOldPrt);
}

/// This routine checks whether there are no other FootnoteBosses
/// between the pFrame's FootnoteBoss and the pNxt's FootnoteBoss.
static bool lcl_IsNextFootnoteBoss( const SwFrame *pFrame, const SwFrame* pNxt )
{
    assert(pFrame && pNxt && "lcl_IsNextFootnoteBoss: No Frames?");
    pFrame = pFrame->FindFootnoteBossFrame();
    pNxt = pNxt->FindFootnoteBossFrame();
    // If pFrame is a last column, we use the page instead.
    while( pFrame && pFrame->IsColumnFrame() && !pFrame->GetNext() )
        pFrame = pFrame->GetUpper()->FindFootnoteBossFrame();
    // If pNxt is a first column, we use the page instead.
    while( pNxt && pNxt->IsColumnFrame() && !pNxt->GetPrev() )
        pNxt = pNxt->GetUpper()->FindFootnoteBossFrame();
    // So.. now pFrame and pNxt are either two adjacent pages or columns.
    return pFrame && pNxt && pFrame->GetNext() == pNxt;
}

bool SwContentFrame::WouldFit_( SwTwips nSpace,
                            SwLayoutFrame *pNewUpper,
                            bool bTstMove,
                            const bool bObjsInNewUpper )
{
    // To have the footnote select its place carefully, it needs
    // to be moved in any case if there is at least one page/column
    // between the footnote and the new Upper.
    SwFootnoteFrame* pFootnoteFrame = nullptr;
    if ( IsInFootnote() )
    {
        if( !lcl_IsNextFootnoteBoss( pNewUpper, this ) )
            return true;
        pFootnoteFrame = FindFootnoteFrame();
    }

    bool bRet;
    bool bSplit = !pNewUpper->Lower();
    SwContentFrame *pFrame = this;
    const SwFrame *pTmpPrev = pNewUpper->Lower();
    if( pTmpPrev && pTmpPrev->IsFootnoteFrame() )
        pTmpPrev = static_cast<const SwFootnoteFrame*>(pTmpPrev)->Lower();
    while ( pTmpPrev && pTmpPrev->GetNext() )
        pTmpPrev = pTmpPrev->GetNext();
    do
    {
        // #i46181#
        SwTwips nSecondCheck = 0;
        SwTwips nOldSpace = nSpace;
        bool bOldSplit = bSplit;

        if ( bTstMove || IsInFly() || ( IsInSct() &&
             ( pFrame->GetUpper()->IsColBodyFrame() || ( pFootnoteFrame &&
               pFootnoteFrame->GetUpper()->GetUpper()->IsColumnFrame() ) ) ) )
        {
            // This is going to get a bit insidious now. If you're faint of heart,
            // you'd better look away here. If a Fly contains columns, then the Contents
            // are movable, except ones in the last column (see SwFrame::IsMoveable()).
            // Of course they're allowed to float back. WouldFit() only returns a usable
            // value if the Frame is movable. To fool WouldFit() into believing there's
            // a movable Frame, I'm just going to hang it somewhere else for the time.
            // The same procedure applies for column sections to make SwSectionFrame::Growable()
            // return the proper value.
            // Within footnotes, we may even need to put the SwFootnoteFrame somewhere else, if
            // there's no SwFootnoteFrame there.
            SwFrame* pTmpFrame = pFrame->IsInFootnote() && !pNewUpper->FindFootnoteFrame() ?
                             static_cast<SwFrame*>(pFrame->FindFootnoteFrame()) : pFrame;
            SwLayoutFrame *pUp = pTmpFrame->GetUpper();
            SwFrame *pOldNext = pTmpFrame->GetNext();
            pTmpFrame->RemoveFromLayout();
            pTmpFrame->InsertBefore( pNewUpper, nullptr );
            // tdf#107126 for a section in a footnote, we have only inserted
            // the SwTextFrame but no SwSectionFrame - reset mbInfSct flag
            // to avoid crashing (but perhaps we should create a temp
            // SwSectionFrame here because WidowsAndOrphans checks for that?)
            pTmpFrame->InvalidateInfFlags();
            if ( pFrame->IsTextFrame() &&
                 ( bTstMove ||
                   static_cast<SwTextFrame*>(pFrame)->HasFollow() ||
                   ( !static_cast<SwTextFrame*>(pFrame)->HasPara() &&
                     !static_cast<SwTextFrame*>(pFrame)->IsEmpty()
                   )
                 )
               )
            {
                bTstMove = true;
                bRet = static_cast<SwTextFrame*>(pFrame)->TestFormat( pTmpPrev, nSpace, bSplit );
            }
            else
                bRet = pFrame->WouldFit( nSpace, bSplit, false );

            pTmpFrame->RemoveFromLayout();
            pTmpFrame->InsertBefore( pUp, pOldNext );
            pTmpFrame->InvalidateInfFlags(); // restore flags
        }
        else
        {
            bRet = pFrame->WouldFit( nSpace, bSplit, false );
            nSecondCheck = !bSplit ? 1 : 0;
        }

        SwBorderAttrAccess aAccess( SwFrame::GetCache(), pFrame );
        const SwBorderAttrs &rAttrs = *aAccess.Get();

        // Sad but true: We need to consider the spacing in our calculation.
        // This already happened in TestFormat.
        if ( bRet && !bTstMove )
        {
            SwTwips nUpper;

            if ( pTmpPrev )
            {
                nUpper = CalcUpperSpace( nullptr, pTmpPrev );

                // in balanced columned section frames we do not want the
                // common border
                bool bCommonBorder = true;
                if ( pFrame->IsInSct() && pFrame->GetUpper()->IsColBodyFrame() )
                {
                    const SwSectionFrame* pSct = pFrame->FindSctFrame();
                    bCommonBorder = pSct->GetFormat()->GetBalancedColumns().GetValue();
                }

                // #i46181#
                nSecondCheck = ( 1 == nSecondCheck &&
                                 pFrame == this &&
                                 IsTextFrame() &&
                                 bCommonBorder &&
                                 !static_cast<const SwTextFrame*>(this)->IsEmpty() ) ?
                                 nUpper :
                                 0;

                nUpper += bCommonBorder ?
                          rAttrs.GetBottomLine( *pFrame ) :
                          rAttrs.CalcBottomLine();

            }
            else
            {
                // #i46181#
                nSecondCheck = 0;

                if( pFrame->IsVertical() )
                    nUpper = pFrame->getFrameArea().Width() - pFrame->getFramePrintArea().Width();
                else
                    nUpper = pFrame->getFrameArea().Height() - pFrame->getFramePrintArea().Height();
            }

            nSpace -= nUpper;

            if ( nSpace < 0 )
            {
                bRet = false;

                // #i46181#
                if ( nSecondCheck > 0 )
                {
                    // The following code is intended to solve a (rare) problem
                    // causing some frames not to move backward:
                    // SwTextFrame::WouldFit() claims that the whole paragraph
                    // fits into the given space and subtracts the height of
                    // all lines from nSpace. nSpace - nUpper is not a valid
                    // indicator if the frame should be allowed to move backward.
                    // We do a second check with the original remaining space
                    // reduced by the required upper space:
                    nOldSpace -= nSecondCheck;
                    const bool bSecondRet = nOldSpace >= 0 && pFrame->WouldFit( nOldSpace, bOldSplit, false );
                    if ( bSecondRet && bOldSplit && nOldSpace >= 0 )
                    {
                        bRet = true;
                        bSplit = true;
                    }
                }
            }
        }

        // Also consider lower spacing in table cells
        if ( bRet && IsInTab() &&
             pNewUpper->GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS) )
        {
            nSpace -= rAttrs.GetULSpace().GetLower();
            if ( nSpace < 0 )
            {
                bRet = false;
            }
        }

        if (bRet && !bSplit && pFrame->IsKeep(rAttrs.GetAttrSet().GetKeep(), GetBreakItem()))
        {
            if( bTstMove )
            {
                while( pFrame->IsTextFrame() && static_cast<SwTextFrame*>(pFrame)->HasFollow() )
                {
                    pFrame = static_cast<SwTextFrame*>(pFrame)->GetFollow();
                }
                // If last follow frame of <this> text frame isn't valid,
                // a formatting of the next content frame doesn't makes sense.
                // Thus, return true.
                if ( IsAnFollow( pFrame ) && !pFrame->isFrameAreaDefinitionValid() )
                {
                    OSL_FAIL( "Only a warning for task 108824:/n<SwContentFrame::WouldFit_(..) - follow not valid!" );
                    return true;
                }
            }
            SwFrame *pNxt;
            if( nullptr != (pNxt = pFrame->FindNext()) && pNxt->IsContentFrame() &&
                ( !pFootnoteFrame || ( pNxt->IsInFootnote() &&
                  pNxt->FindFootnoteFrame()->GetAttr() == pFootnoteFrame->GetAttr() ) ) )
            {
                // TestFormat(?) does not like paragraph- or character anchored objects.

                // current solution for the test formatting doesn't work, if
                // objects are present in the remaining area of the new upper
                if ( bTstMove &&
                     ( pNxt->GetDrawObjs() || bObjsInNewUpper ) )
                {
                    return true;
                }

                if ( !pNxt->isFrameAreaDefinitionValid() )
                {
                    MakeNxt( pFrame, pNxt );
                }

                // Little trick: if the next has a predecessor, then the paragraph
                // spacing has been calculated already, and we don't need to re-calculate
                // it in an expensive way.
                if( lcl_NotHiddenPrev( pNxt ) )
                    pTmpPrev = nullptr;
                else
                {
                    if( pFrame->IsTextFrame() && static_cast<SwTextFrame*>(pFrame)->IsHiddenNow() )
                        pTmpPrev = lcl_NotHiddenPrev( pFrame );
                    else
                        pTmpPrev = pFrame;
                }
                pFrame = static_cast<SwContentFrame*>(pNxt);
            }
            else
                pFrame = nullptr;
        }
        else
            pFrame = nullptr;

    } while ( bRet && pFrame );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
