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

#include <config_wasm_strip.h>

#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <dcontact.hxx>
#include <flyfrm.hxx>
#include <txtfrm.hxx>
#include <cellfrm.hxx>
#include <swtable.hxx>
#include <fmtfsize.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <fmtclds.hxx>
#include <viewimp.hxx>
#include <sortedobjs.hxx>
#include <hints.hxx>
#include <frmtool.hxx>
#include <ndtxt.hxx>
#include <osl/diagnose.h>

    // No inline cause we need the function pointers
tools::Long SwFrame::GetTopMargin() const
    { return getFramePrintArea().Top(); }
tools::Long SwFrame::GetBottomMargin() const
    { return getFrameArea().Height() -getFramePrintArea().Height() -getFramePrintArea().Top(); }
tools::Long SwFrame::GetLeftMargin() const
    { return getFramePrintArea().Left(); }
tools::Long SwFrame::GetRightMargin() const
    { return getFrameArea().Width() - getFramePrintArea().Width() - getFramePrintArea().Left(); }
tools::Long SwFrame::GetPrtLeft() const
    { return getFrameArea().Left() + getFramePrintArea().Left(); }
tools::Long SwFrame::GetPrtBottom() const
    { return getFrameArea().Top() + getFramePrintArea().Height() + getFramePrintArea().Top(); }
tools::Long SwFrame::GetPrtRight() const
    { return getFrameArea().Left() + getFramePrintArea().Width() + getFramePrintArea().Left(); }
tools::Long SwFrame::GetPrtTop() const
    { return getFrameArea().Top() + getFramePrintArea().Top(); }

bool SwFrame::SetMinLeft( tools::Long nDeadline )
{
    SwTwips nDiff = nDeadline - getFrameArea().Left();
    if( nDiff > 0 )
    {
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
        aFrm.Left( nDeadline );

        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
        aPrt.Width( aPrt.Width() - nDiff );

        return true;
    }
    return false;
}

bool SwFrame::SetMaxBottom( tools::Long nDeadline )
{
    SwTwips nDiff = getFrameArea().Top() + getFrameArea().Height() - nDeadline;
    if( nDiff > 0 )
    {
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
        aFrm.Height( aFrm.Height() - nDiff );

        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
        aPrt.Height( aPrt.Height() - nDiff );

        return true;
    }
    return false;
}

bool SwFrame::SetMaxRight( tools::Long nDeadline )
{
    SwTwips nDiff = getFrameArea().Left() + getFrameArea().Width() - nDeadline;
    if( nDiff > 0 )
    {
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
        aFrm.Width( aFrm.Width() - nDiff );

        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
        aPrt.Width( aPrt.Width() - nDiff );

        return true;
    }
    return false;
}

void SwFrame::MakeBelowPos( const SwFrame* pUp, const SwFrame* pPrv, bool bNotify )
{
    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);

    if( pPrv )
    {
        aFrm.Pos( pPrv->getFrameArea().Pos() );
        aFrm.Pos().AdjustY(pPrv->getFrameArea().Height() );
    }
    else
    {
        aFrm.Pos( pUp->getFrameArea().Pos() );
        aFrm.Pos() += pUp->getFramePrintArea().Pos();
    }

    if( bNotify )
    {
        aFrm.Pos().AdjustY(1 );
    }
}

void SwFrame::MakeLeftPos( const SwFrame* pUp, const SwFrame* pPrv, bool bNotify )
{
    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);

    if( pPrv )
    {
        aFrm.Pos( pPrv->getFrameArea().Pos() );
        aFrm.Pos().AdjustX( -(aFrm.Width()) );
    }
    else
    {
        aFrm.Pos( pUp->getFrameArea().Pos() );
        aFrm.Pos() += pUp->getFramePrintArea().Pos();
        aFrm.Pos().AdjustX(pUp->getFramePrintArea().Width() - aFrm.Width() );
    }

    if( bNotify )
    {
        aFrm.Pos().AdjustX( -1 );
    }
}

void SwFrame::MakeRightPos( const SwFrame* pUp, const SwFrame* pPrv, bool bNotify )
{
    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);

    if( pPrv )
    {
        aFrm.Pos( pPrv->getFrameArea().Pos() );
        aFrm.Pos().AdjustX(pPrv->getFrameArea().Width() );
    }
    else
    {
        aFrm.Pos( pUp->getFrameArea().Pos() );
        aFrm.Pos() += pUp->getFramePrintArea().Pos();
    }

    if( bNotify )
    {
        aFrm.Pos().AdjustX(1 );
    }
}

void SwFrame::SetTopBottomMargins( tools::Long nTop, tools::Long nBot )
{
    SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
    aPrt.Top( nTop );
    aPrt.Height( getFrameArea().Height() - nTop - nBot );
}

void SwFrame::SetLeftRightMargins( tools::Long nLeft, tools::Long nRight)
{
    SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
    aPrt.Left( nLeft );
    aPrt.Width( getFrameArea().Width() - nLeft - nRight );
}

void SwFrame::SetRightLeftMargins( tools::Long nRight, tools::Long nLeft)
{
    SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
    aPrt.Left( nLeft );
    aPrt.Width( getFrameArea().Width() - nLeft - nRight );
}

/// checks the layout direction and invalidates the lower frames recursively, if necessary.
void SwFrame::CheckDirChange()
{
    bool bOldVert = mbVertical;
    bool bOldR2L = mbRightToLeft;
    SetInvalidVert( true );
    mbInvalidR2L = true;
    bool bChg = bOldR2L != IsRightToLeft();
    bool bOldVertL2R = IsVertLR();
    if( !(( IsVertical() != bOldVert ) || bChg || bOldVertL2R != IsVertLR()) )
        return;

    InvalidateAll();
    if( IsLayoutFrame() )
    {
        // set minimum row height for vertical cells in horizontal table:
        if ( IsCellFrame() && GetUpper() )
        {
            if ( IsVertical() != GetUpper()->IsVertical() &&
                 static_cast<SwCellFrame*>(this)->GetTabBox()->getRowSpan() == 1 )
            {
                enum {
                    MIN_VERT_CELL_HEIGHT = 1135
                };

                SwTableLine* pLine = const_cast<SwTableLine*>(static_cast<SwCellFrame*>(this)->GetTabBox()->GetUpper());
                SwFrameFormat* pFrameFormat = pLine->GetFrameFormat();
                SwFormatFrameSize aNew( pFrameFormat->GetFrameSize() );
                if ( SwFrameSize::Fixed != aNew.GetHeightSizeType() )
                    aNew.SetHeightSizeType( SwFrameSize::Minimum );
                if ( aNew.GetHeight() < MIN_VERT_CELL_HEIGHT )
                    aNew.SetHeight( MIN_VERT_CELL_HEIGHT );
                SwDoc* pDoc = pFrameFormat->GetDoc();
                pDoc->SetAttr( aNew, *pLine->ClaimFrameFormat() );
            }
        }

        SwFrame* pFrame = static_cast<SwLayoutFrame*>(this)->Lower();
        const SwFormatCol* pCol = nullptr;
        SwLayoutFrame* pBody = nullptr;
        if( pFrame )
        {
            if( IsPageFrame() )
            {
                // If we're a page frame and we change our layout direction,
                // we have to look for columns and rearrange them.
                pBody = static_cast<SwPageFrame*>(this)->FindBodyCont();
                if(pBody && pBody->Lower() && pBody->Lower()->IsColumnFrame())
                    pCol = &static_cast<SwPageFrame*>(this)->GetFormat()->GetCol();
            }
            else if( pFrame->IsColumnFrame() )
            {
                pBody = static_cast<SwLayoutFrame*>(this);
                const SwFrameFormat *pFormat = pBody->GetFormat();
                if( pFormat )
                    pCol = &pFormat->GetCol();
            }
        }
        while( pFrame )
        {
            pFrame->CheckDirChange();
            pFrame = pFrame->GetNext();
        }
        if( pCol )
            pBody->AdjustColumns( pCol, true );
    }
    else if( IsTextFrame() )
        static_cast<SwTextFrame*>(this)->Prepare();

    // #i31698# - notify anchored objects also for page frames.
    // Remove code above for special handling of page frames
    if ( !GetDrawObjs() )
        return;

    const SwSortedObjs *pObjs = GetDrawObjs();
    const size_t nCnt = pObjs->size();
    for ( size_t i = 0; i < nCnt; ++i )
    {
        SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
        if( auto pFlyFrame = pAnchoredObj->DynCastFlyFrame() )
            pFlyFrame->CheckDirChange();
        else
        {
            // OD 2004-04-06 #i26791# - direct object
            // positioning no longer needed. Instead
            // invalidate
            pAnchoredObj->InvalidateObjPos();
        }
        // #i31698# - update layout direction of
        // anchored object
        {
            ::setContextWritingMode( pAnchoredObj->DrawObj(), pAnchoredObj->GetAnchorFrameContainingAnchPos() );
            pAnchoredObj->UpdateLayoutDir();
        }
    }
}

/// returns the position for anchors based on frame direction
// OD 2004-03-10 #i11860# - consider lower space and line spacing of
// previous frame according to new option 'Use former object positioning'
Point SwFrame::GetFrameAnchorPos( bool bIgnoreFlysAnchoredAtThisFrame ) const
{
    Point aAnchor = getFrameArea().Pos();

    if ( ( IsVertical() && !IsVertLR() ) || IsRightToLeft() )
        aAnchor.AdjustX(getFrameArea().Width() );

    if ( IsTextFrame() )
    {
        SwTwips nBaseOfstForFly =
            static_cast<const SwTextFrame*>(this)->GetBaseOffsetForFly( bIgnoreFlysAnchoredAtThisFrame );
        if ( IsVertical() )
            aAnchor.AdjustY(nBaseOfstForFly );
        else
            aAnchor.AdjustX(nBaseOfstForFly );

        // OD 2004-03-10 #i11860# - if option 'Use former object positioning'
        // is OFF, consider the lower space and the line spacing of the
        // previous frame and the spacing considered for the page grid
        const SwTextFrame* pThisTextFrame = static_cast<const SwTextFrame*>(this);
        const SwTwips nUpperSpaceAmountConsideredForPrevFrameAndPageGrid =
                pThisTextFrame->GetUpperSpaceAmountConsideredForPrevFrameAndPageGrid();
        if ( IsVertical() )
        {
            aAnchor.AdjustX( -nUpperSpaceAmountConsideredForPrevFrameAndPageGrid );
        }
        else
        {
            aAnchor.AdjustY(nUpperSpaceAmountConsideredForPrevFrameAndPageGrid );
        }
    }

    return aAnchor;
}

void SwFrame::DestroyImpl()
{
    mbInDtor = true;

    // accessible objects for fly and cell frames have been already disposed
    // by the destructors of the derived classes.
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    if (IsAccessibleFrame() && !(IsFlyFrame() || IsCellFrame())
        && (GetDep() || IsTextFrame())) // sw_redlinehide: text frame may not have Dep!
    {
        assert(!IsTextFrame() || GetDep() || static_cast<SwTextFrame*>(this)->GetMergedPara());
        SwRootFrame *pRootFrame = getRootFrame();
        if( pRootFrame && pRootFrame->IsAnyShellAccessible() )
        {
            SwViewShell *pVSh = pRootFrame->GetCurrShell();
            if( pVSh && pVSh->Imp() )
            {
                OSL_ENSURE( !GetLower(), "Lowers should be dispose already!" );
                pVSh->Imp()->DisposeAccessibleFrame( this );
            }
        }
    }
#endif

    if (!m_pDrawObjs)
        return;

    for (size_t i = m_pDrawObjs->size(); i; )
    {
        SwAnchoredObject* pAnchoredObj = (*m_pDrawObjs)[--i];
        if ( auto pFlyFrame = pAnchoredObj->DynCastFlyFrame() )
        {
            SwFrame::DestroyFrame(pFlyFrame);
        }
        else
        {
            SdrObject* pSdrObj = pAnchoredObj->DrawObj();
            SwDrawContact* pContact =
                    static_cast<SwDrawContact*>(pSdrObj->GetUserCall());
            OSL_ENSURE( pContact,
                    "<SwFrame::~SwFrame> - missing contact for drawing object" );
            if ( pContact )
            {
                pContact->DisconnectObjFromLayout( pSdrObj );
            }
        }
    }
    m_pDrawObjs.reset();
}

SwFrame::~SwFrame()
{
    assert(m_isInDestroy); // check that only DestroySwFrame does "delete"
    assert(!IsDeleteForbidden()); // check that it's not deleted while deletes are forbidden
#if OSL_DEBUG_LEVEL > 0
    // JP 15.10.2001: for detection of access to deleted frames
    mpRoot = reinterpret_cast<SwRootFrame*>(0x33333333);
#endif
}

void SwFrame::DestroyFrame(SwFrame *const pFrame)
{
    if (pFrame)
    {
        pFrame->m_isInDestroy = true;
        pFrame->DestroyImpl();
        assert(pFrame->mbInDtor); // check that nobody forgot to call base class
        delete pFrame;
    }
}

const SwFrameFormat * SwLayoutFrame::GetFormat() const
{
    return static_cast< const SwFrameFormat * >( GetDep() );
}

SwFrameFormat * SwLayoutFrame::GetFormat()
{
    return static_cast< SwFrameFormat * >( GetDep() );
}

void SwLayoutFrame::SetFrameFormat(SwFrameFormat* pNew)
{
    if(pNew == GetFormat())
        return;
    const SwFormatChg aOldFormat(GetFormat());
    pNew->Add(this);
    const SwFormatChg aNewFormat(pNew);
    SwClientNotify(*pNew, sw::LegacyModifyHint(&aOldFormat, &aNewFormat));
}

SwContentFrame::SwContentFrame( SwContentNode * const pContent, SwFrame* pSib ) :
    SwFrame( pContent, pSib ),
    SwFlowFrame( static_cast<SwFrame&>(*this) )
{
    assert(!getRootFrame()->HasMergedParas() || pContent->IsCreateFrameWhenHidingRedlines());
}

void SwContentFrame::DestroyImpl()
{
    const SwContentNode* pCNd(dynamic_cast<SwContentNode*>(GetDep()));
    if (nullptr == pCNd && IsTextFrame())
    {
        pCNd = static_cast<SwTextFrame*>(this)->GetTextNodeFirst();
    }
    // IsInDtor shouldn't be happening with ViewShell owning layout
    assert(nullptr == pCNd || !pCNd->GetDoc().IsInDtor());
    if (nullptr != pCNd && !pCNd->GetDoc().IsInDtor())
    {
        //Unregister from root if I'm still in turbo there.
        SwRootFrame *pRoot = getRootFrame();
        if( pRoot && pRoot->GetTurbo() == this )
        {
            pRoot->DisallowTurbo();
            pRoot->ResetTurbo();
        }
    }

    SwFrame::DestroyImpl();
}

SwContentFrame::~SwContentFrame()
{
}

void SwTextFrame::RegisterToNode(SwTextNode & rNode, bool const isForceNodeAsFirst)
{
    if (isForceNodeAsFirst && m_pMergedPara)
    {   // nothing registered here, in particular no delete redlines (insert
        // redline might end on empty node where delete rl starts, should be ok)
        assert(m_pMergedPara->pFirstNode->GetIndex() + 1 == rNode.GetIndex());
        assert(rNode.GetDoc().getIDocumentRedlineAccess().GetRedlinePos(
            *m_pMergedPara->pFirstNode, RedlineType::Delete) == SwRedlineTable::npos);
        assert(std::find_if(
            rNode.GetDoc().getIDocumentMarkAccess()->getFieldmarksBegin(),
            rNode.GetDoc().getIDocumentMarkAccess()->getFieldmarksEnd(),
            [this](::sw::mark::IMark const*const pMark) {
                return pMark->GetMarkStart().nNode == *m_pMergedPara->pFirstNode
                    && pMark->GetMarkEnd().nNode != *m_pMergedPara->pFirstNode;
            }) == rNode.GetDoc().getIDocumentMarkAccess()->getFieldmarksEnd());
    }
    assert(&rNode != GetDep());
    assert(!m_pMergedPara
        || (m_pMergedPara->pFirstNode->GetIndex() < rNode.GetIndex())
        || (rNode.GetIndex() + 1 == m_pMergedPara->pFirstNode->GetIndex()));
    SwTextNode & rFirstNode(
        (!isForceNodeAsFirst && m_pMergedPara && m_pMergedPara->pFirstNode->GetIndex() < rNode.GetIndex())
            ? *m_pMergedPara->pFirstNode
            : rNode);
    // sw_redlinehide: use New here, because the only caller also calls lcl_ChangeFootnoteRef
    m_pMergedPara = sw::CheckParaRedlineMerge(*this, rFirstNode, sw::FrameMode::New);
    if (!m_pMergedPara)
    {
        rNode.Add(this);
    }
}

void SwLayoutFrame::DestroyImpl()
{
    while (!m_VertPosOrientFramesFor.empty())
    {
        SwAnchoredObject *pObj = *m_VertPosOrientFramesFor.begin();
        pObj->ClearVertPosOrientFrame();
    }

    assert(m_VertPosOrientFramesFor.empty());

    SwFrame *pFrame = m_pLower;

    if( GetFormat() && !GetFormat()->GetDoc()->IsInDtor() )
    {
        while ( pFrame )
        {
            //First delete the Objs of the Frame because they can't unregister
            //from the page after remove.
            //We don't want to create an endless loop only because one couldn't
            //unregister.

            while ( pFrame->GetDrawObjs() && pFrame->GetDrawObjs()->size() )
            {
                const size_t nCnt = pFrame->GetDrawObjs()->size();
                // #i28701#
                SwAnchoredObject* pAnchoredObj = (*pFrame->GetDrawObjs())[0];
                if (SwFlyFrame* pFlyFrame = pAnchoredObj->DynCastFlyFrame())
                {
                    SwFrame::DestroyFrame(pFlyFrame);
                    assert(!pFrame->GetDrawObjs() || nCnt > pFrame->GetDrawObjs()->size());
                }
                else
                {
                    pAnchoredObj->ClearTmpConsiderWrapInfluence();
                    SdrObject* pSdrObj = pAnchoredObj->DrawObj();
                    SwDrawContact* pContact =
                            static_cast<SwDrawContact*>(pSdrObj->GetUserCall());
                    OSL_ENSURE( pContact,
                            "<SwFrame::~SwFrame> - missing contact for drawing object" );
                    if ( pContact )
                    {
                        pContact->DisconnectObjFromLayout( pSdrObj );
                    }

                    if ( pFrame->GetDrawObjs() &&
                         nCnt == pFrame->GetDrawObjs()->size() )
                    {
                        pFrame->GetDrawObjs()->Remove( *pAnchoredObj );
                    }
                }
            }
            pFrame->RemoveFromLayout();
            SwFrame::DestroyFrame(pFrame);
            pFrame = m_pLower;
        }
        //Delete the Flys, the last one also deletes the array.
        while ( GetDrawObjs() && GetDrawObjs()->size() )
        {
            const size_t nCnt = GetDrawObjs()->size();

            // #i28701#
            SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[0];
            if ( auto pFlyFrame = pAnchoredObj->DynCastFlyFrame() )
            {
                SwFrame::DestroyFrame(pFlyFrame);
                assert(!GetDrawObjs() || nCnt > GetDrawObjs()->size());
            }
            else
            {
                SdrObject* pSdrObj = pAnchoredObj->DrawObj();
                SwDrawContact* pContact =
                        static_cast<SwDrawContact*>(pSdrObj->GetUserCall());
                OSL_ENSURE( pContact,
                        "<SwFrame::~SwFrame> - missing contact for drawing object" );
                if ( pContact )
                {
                    pContact->DisconnectObjFromLayout( pSdrObj );
                }

                if ( GetDrawObjs() && nCnt == GetDrawObjs()->size() )
                {
                    GetDrawObjs()->Remove( *pAnchoredObj );
                }
            }
        }
    }
    else
    {
        while( pFrame )
        {
            SwFrame *pNxt = pFrame->GetNext();
            SwFrame::DestroyFrame(pFrame);
            pFrame = pNxt;
        }
    }

    SwFrame::DestroyImpl();
}

SwLayoutFrame::~SwLayoutFrame()
{
}

/**
|*  The paintarea is the area, in which the content of a frame is allowed
|*  to be displayed. This region could be larger than the printarea (getFramePrintArea())
|*  of the upper, it includes e.g. often the margin of the page.
|*/
SwRect SwFrame::GetPaintArea() const
{
    // NEW TABLES
    // Cell frames may not leave their upper:
    SwRect aRect = IsRowFrame() ? GetUpper()->getFrameArea() : getFrameArea();
    const bool bVert = IsVertical();
    SwRectFn fnRect = bVert ? ( IsVertLR() ? (IsVertLRBT() ? fnRectVertL2RB2T : fnRectVertL2R) : fnRectVert ) : fnRectHori;
    SwRectFnSet aRectFnSet(this);
    tools::Long nRight = (aRect.*fnRect->fnGetRight)();
    tools::Long nLeft  = (aRect.*fnRect->fnGetLeft)();
    const SwFrame* pTmp = this;
    bool bLeft = true;
    bool bRight = true;
    tools::Long nRowSpan = 0;
    while( pTmp )
    {
        if( pTmp->IsCellFrame() && pTmp->GetUpper() &&
            pTmp->GetUpper()->IsVertical() != pTmp->IsVertical() )
            nRowSpan = static_cast<const SwCellFrame*>(pTmp)->GetTabBox()->getRowSpan();
        tools::Long nTmpRight = (pTmp->getFrameArea().*fnRect->fnGetRight)();
        tools::Long nTmpLeft = (pTmp->getFrameArea().*fnRect->fnGetLeft)();
        if( pTmp->IsRowFrame() && nRowSpan > 1 )
        {
            const SwFrame* pNxt = pTmp;
            while( --nRowSpan > 0 && pNxt->GetNext() )
                pNxt = pNxt->GetNext();
            if( pTmp->IsVertical() )
                nTmpLeft = (pNxt->getFrameArea().*fnRect->fnGetLeft)();
            else
            {
                // pTmp is a row frame, but it's not vertical.
                if (IsVertLRBT())
                {
                    // This frame cell is OK to expand towards the physical down direction.
                    // Physical down is left.
                    nTmpLeft = (pNxt->getFrameArea().*fnRect->fnGetLeft)();
                }
                else
                {
                    nTmpRight = (pNxt->getFrameArea().*fnRect->fnGetRight)();
                }
            }
        }
        OSL_ENSURE( pTmp, "GetPaintArea lost in time and space" );
        if( pTmp->IsPageFrame() || pTmp->IsFlyFrame() ||
            pTmp->IsCellFrame() || pTmp->IsRowFrame() || //nobody leaves a table!
            pTmp->IsRootFrame() )
        {
            // BTLR is OK to expand towards the physical down direction. Physical down is left.
            if( bLeft || (aRectFnSet.XDiff(nTmpLeft, nLeft) > 0 && !IsVertLRBT()) )
                nLeft = nTmpLeft;
            if( bRight || aRectFnSet.XDiff(nRight, nTmpRight) > 0 )
                nRight = nTmpRight;
            if( pTmp->IsPageFrame() || pTmp->IsFlyFrame() || pTmp->IsRootFrame() )
                break;
            bLeft = false;
            bRight = false;
        }
        else if( pTmp->IsColumnFrame() )  // nobody enters neighbour columns
        {
            bool bR2L = pTmp->IsRightToLeft();
            // the first column has _no_ influence to the left range
            if( bR2L ? pTmp->GetNext() : pTmp->GetPrev() )
            {
                if( bLeft || aRectFnSet.XDiff(nTmpLeft, nLeft) > 0 )
                    nLeft = nTmpLeft;
                bLeft = false;
            }
             // the last column has _no_ influence to the right range
            if( bR2L ? pTmp->GetPrev() : pTmp->GetNext() )
            {
                if( bRight || aRectFnSet.XDiff(nRight, nTmpRight) > 0 )
                    nRight = nTmpRight;
                bRight = false;
            }
        }
        else if( bVert && pTmp->IsBodyFrame() )
        {
            // Header and footer frames have always horizontal direction and
            // limit the body frame.
            // A previous frame of a body frame must be a header,
            // the next frame of a body frame may be a footnotecontainer or
            // a footer. The footnotecontainer has the same direction like
            // the body frame.
            if( pTmp->GetPrev() && ( bLeft || aRectFnSet.XDiff(nTmpLeft, nLeft) > 0 ) )
            {
                nLeft = nTmpLeft;
                bLeft = false;
            }
            if( pTmp->GetNext() &&
                ( pTmp->GetNext()->IsFooterFrame() || pTmp->GetNext()->GetNext() )
                && ( bRight || aRectFnSet.XDiff(nRight, nTmpRight) > 0 ) )
            {
                nRight = nTmpRight;
                bRight = false;
            }
        }
        pTmp = pTmp->GetUpper();
    }
    (aRect.*fnRect->fnSetLeft)( nLeft );
    (aRect.*fnRect->fnSetRight)( nRight );
    return aRect;
}

/**
|*  The unionframe is the framearea (getFrameArea()) of a frame expanded by the
|*  printarea, if there's a negative margin at the left or right side.
|*/
SwRect SwFrame::UnionFrame( bool bBorder ) const
{
    bool bVert = IsVertical();
    SwRectFn fnRect = bVert ? ( IsVertLR() ? (IsVertLRBT() ? fnRectVertL2RB2T : fnRectVertL2R) : fnRectVert ) : fnRectHori;
    tools::Long nLeft = (getFrameArea().*fnRect->fnGetLeft)();
    tools::Long nWidth = (getFrameArea().*fnRect->fnGetWidth)();
    tools::Long nPrtLeft = (getFramePrintArea().*fnRect->fnGetLeft)();
    tools::Long nPrtWidth = (getFramePrintArea().*fnRect->fnGetWidth)();
    SwRectFnSet aRectFnSet(this);
    if (aRectFnSet.XInc(nPrtLeft, nPrtWidth) > nWidth)
        nWidth = nPrtLeft + nPrtWidth;
    if( nPrtLeft < 0 )
    {
        nLeft += nPrtLeft;
        nWidth -= nPrtLeft;
    }
    SwTwips nRight = aRectFnSet.XInc(nLeft, nWidth);
    tools::Long nAdd = 0;
    if( bBorder )
    {
        SwBorderAttrAccess aAccess( SwFrame::GetCache(), this );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        const SvxBoxItem &rBox = rAttrs.GetBox();
        if ( rBox.GetLeft() )
            nLeft -= rBox.CalcLineSpace( SvxBoxItemLine::LEFT );
        else
            nLeft -= rBox.GetDistance( SvxBoxItemLine::LEFT ) + 1;
        if ( rBox.GetRight() )
            nAdd += rBox.CalcLineSpace( SvxBoxItemLine::RIGHT );
        else
            nAdd += rBox.GetDistance( SvxBoxItemLine::RIGHT ) + 1;
        if( rAttrs.GetShadow().GetLocation() != SvxShadowLocation::NONE )
        {
            const SvxShadowItem &rShadow = rAttrs.GetShadow();
            nLeft -= rShadow.CalcShadowSpace( SvxShadowItemSide::LEFT );
            nAdd += rShadow.CalcShadowSpace( SvxShadowItemSide::RIGHT );
        }
    }
    if( IsTextFrame() && static_cast<const SwTextFrame*>(this)->HasPara() )
    {
        tools::Long nTmp = static_cast<const SwTextFrame*>(this)->HangingMargin();
        if( nTmp > nAdd )
            nAdd = nTmp;
    }
    nWidth = aRectFnSet.XDiff(aRectFnSet.XInc(nRight, nAdd), nLeft);
    SwRect aRet( getFrameArea() );
    (aRet.*fnRect->fnSetLeft)(nLeft);
    (aRet.*fnRect->fnSetWidth)( nWidth );
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
