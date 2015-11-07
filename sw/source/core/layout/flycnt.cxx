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

#include <tools/bigint.hxx>
#include "pagefrm.hxx"
#include "txtfrm.hxx"
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include "frmtool.hxx"
#include "dflyobj.hxx"
#include "hints.hxx"
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <txatbase.hxx>

#include "tabfrm.hxx"
#include "flyfrms.hxx"
#include "crstate.hxx"
#include "sectfrm.hxx"

#include <tocntntanchoredobjectposition.hxx>
#include <dcontact.hxx>
#include <sortedobjs.hxx>
#include <layouter.hxx>
#include <objectformattertxtfrm.hxx>
#include <HandleAnchorNodeChg.hxx>

using namespace ::com::sun::star;

namespace
{

static inline SwTwips lcl_GetTopForObjPos(const SwContentFrm* pCnt, const bool bVert, const bool bVertL2R)
{
    if ( bVert )
    {
        SwTwips aResult = pCnt->Frm().Left();
        if ( bVertL2R )
            aResult += pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
        else
            aResult += pCnt->Frm().Width() - pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
        return aResult;
    }
    else
        return pCnt->Frm().Top() + pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
}

}

SwFlyAtCntFrm::SwFlyAtCntFrm( SwFlyFrameFormat *pFormat, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFreeFrm( pFormat, pSib, pAnch )
{
    m_bAtCnt = true;
    m_bAutoPosition = (FLY_AT_CHAR == pFormat->GetAnchor().GetAnchorId());
}

// #i28701#

void SwFlyAtCntFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    const sal_uInt16 nWhich = pNew ? pNew->Which() : 0;
    const SwFormatAnchor *pAnch = nullptr;

    if( RES_ATTRSET_CHG == nWhich && SfxItemState::SET ==
        static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState( RES_ANCHOR, false,
            reinterpret_cast<const SfxPoolItem**>(&pAnch) ))
        ;       // The anchor pointer is set at GetItemState!

    else if( RES_ANCHOR == nWhich )
    {
        //Change anchor, I move myself to a new place.
        //The anchor type must not change, this is only possible using
        //SwFEShell.
        pAnch = static_cast<const SwFormatAnchor*>(pNew);
    }

    if( pAnch )
    {
        OSL_ENSURE( pAnch->GetAnchorId() == GetFormat()->GetAnchor().GetAnchorId(),
                "Illegal change of anchor type. " );

        //Unregister, get hold of a new anchor and attach it
        SwRect aOld( GetObjRectWithSpaces() );
        SwPageFrm *pOldPage = FindPageFrm();
        const SwFrm *pOldAnchor = GetAnchorFrm();
        SwContentFrm *pContent = const_cast<SwContentFrm*>(static_cast<const SwContentFrm*>(GetAnchorFrm()));
        AnchorFrm()->RemoveFly( this );

        const bool bBodyFootnote = (pContent->IsInDocBody() || pContent->IsInFootnote());

        // Search the new anchor using the NodeIdx; the relation between old
        // and new NodeIdx determines the search direction
        const SwNodeIndex aNewIdx( pAnch->GetContentAnchor()->nNode );
        SwNodeIndex aOldIdx( *pContent->GetNode() );

        //fix: depending on which index was smaller, searching in the do-while
        //loop previously was done forward or backwards respectively. This however
        //could lead to an infinite loop. To at least avoid the loop, searching
        //is now done in only one direction. Getting hold of a frame from the node
        //is still possible if the new anchor could not be found. Chances are
        //good that this will be the correct one.
        const bool bNext = aOldIdx < aNewIdx;
        // consider the case that at found anchor frame candidate already a
        // fly frame of the given fly format is registered.
        // consider, that <pContent> is the already
        // the new anchor frame.
        bool bFound( aOldIdx == aNewIdx );
        while ( pContent && !bFound )
        {
            do
            {
                if ( bNext )
                    pContent = pContent->GetNextContentFrm();
                else
                    pContent = pContent->GetPrevContentFrm();
            } while ( pContent &&
                      !( bBodyFootnote == ( pContent->IsInDocBody() ||
                                       pContent->IsInFootnote() ) ) );
            if ( pContent )
                aOldIdx = *pContent->GetNode();

            // check, if at found anchor frame candidate already a fly frame
            // of the given fly frame format is registered.
            bFound = aOldIdx == aNewIdx;
            if (bFound && pContent && pContent->GetDrawObjs())
            {
                SwFrameFormat* pMyFlyFrameFormat( &GetFrameFormat() );
                SwSortedObjs &rObjs = *pContent->GetDrawObjs();
                for( size_t i = 0; i < rObjs.size(); ++i)
                {
                    SwFlyFrm* pFlyFrm = dynamic_cast<SwFlyFrm*>(rObjs[i]);
                    if ( pFlyFrm &&
                         &(pFlyFrm->GetFrameFormat()) == pMyFlyFrameFormat )
                    {
                        bFound = false;
                        break;
                    }
                }
            }
        }
        if ( !pContent )
        {
            SwContentNode *pNode = aNewIdx.GetNode().GetContentNode();
            pContent = pNode->getLayoutFrm( getRootFrm(), &pOldAnchor->Frm().Pos(), nullptr, false );
            OSL_ENSURE( pContent, "Neuen Anker nicht gefunden" );
        }
        //Flys are never attached to a follow, but always on the master which
        //we are going to search now.
        SwContentFrm* pFlow = pContent;
        while ( pFlow->IsFollow() )
            pFlow = pFlow->FindMaster();
        pContent = pFlow;

        //and *puff* it's attached...
        pContent->AppendFly( this );
        if ( pOldPage && pOldPage != FindPageFrm() )
            NotifyBackground( pOldPage, aOld, PREP_FLY_LEAVE );

        //Fix(3495)
        _InvalidatePos();
        InvalidatePage();
        SetNotifyBack();
        // #i28701# - reset member <maLastCharRect> and
        // <mnLastTopOfLine> for to-character anchored objects.
        ClearCharRectAndTopOfLine();
    }
    else
        SwFlyFrm::Modify( pOld, pNew );
}

//We need some helper classes to monitor the oscillation and a few functions
//to not get lost.

// #i3317# - re-factoring of the position stack
class SwOszControl
{
    static const SwFlyFrm *pStk1;
    static const SwFlyFrm *pStk2;
    static const SwFlyFrm *pStk3;
    static const SwFlyFrm *pStk4;
    static const SwFlyFrm *pStk5;

    const SwFlyFrm *pFly;
    // #i3317#
    sal_uInt8 mnPosStackSize;
    std::vector<Point*> maObjPositions;

public:
    explicit SwOszControl( const SwFlyFrm *pFrm );
    ~SwOszControl();
    bool ChkOsz();
    static bool IsInProgress( const SwFlyFrm *pFly );
};

const SwFlyFrm *SwOszControl::pStk1 = nullptr;
const SwFlyFrm *SwOszControl::pStk2 = nullptr;
const SwFlyFrm *SwOszControl::pStk3 = nullptr;
const SwFlyFrm *SwOszControl::pStk4 = nullptr;
const SwFlyFrm *SwOszControl::pStk5 = nullptr;

SwOszControl::SwOszControl( const SwFlyFrm *pFrm )
    : pFly( pFrm ),
      // #i3317#
      mnPosStackSize( 20 )
{
    if ( !SwOszControl::pStk1 )
        SwOszControl::pStk1 = pFly;
    else if ( !SwOszControl::pStk2 )
        SwOszControl::pStk2 = pFly;
    else if ( !SwOszControl::pStk3 )
        SwOszControl::pStk3 = pFly;
    else if ( !SwOszControl::pStk4 )
        SwOszControl::pStk4 = pFly;
    else if ( !SwOszControl::pStk5 )
        SwOszControl::pStk5 = pFly;
}

SwOszControl::~SwOszControl()
{
    if ( SwOszControl::pStk1 == pFly )
        SwOszControl::pStk1 = nullptr;
    else if ( SwOszControl::pStk2 == pFly )
        SwOszControl::pStk2 = nullptr;
    else if ( SwOszControl::pStk3 == pFly )
        SwOszControl::pStk3 = nullptr;
    else if ( SwOszControl::pStk4 == pFly )
        SwOszControl::pStk4 = nullptr;
    else if ( SwOszControl::pStk5 == pFly )
        SwOszControl::pStk5 = nullptr;
    // #i3317#
    while ( !maObjPositions.empty() )
    {
        Point* pPos = maObjPositions.back();
        delete pPos;

        maObjPositions.pop_back();
    }
}

bool SwOszControl::IsInProgress( const SwFlyFrm *pFly )
{
    if ( SwOszControl::pStk1 && !pFly->IsLowerOf( SwOszControl::pStk1 ) )
        return true;
    if ( SwOszControl::pStk2 && !pFly->IsLowerOf( SwOszControl::pStk2 ) )
        return true;
    if ( SwOszControl::pStk3 && !pFly->IsLowerOf( SwOszControl::pStk3 ) )
        return true;
    if ( SwOszControl::pStk4 && !pFly->IsLowerOf( SwOszControl::pStk4 ) )
        return true;
    if ( SwOszControl::pStk5 && !pFly->IsLowerOf( SwOszControl::pStk5 ) )
        return true;
    return false;
}

bool SwOszControl::ChkOsz()
{
    bool bOscillationDetected = false;

    if ( maObjPositions.size() == mnPosStackSize )
    {
        // position stack is full -> oscillation
        bOscillationDetected = true;
    }
    else
    {
        Point* pNewObjPos = new Point( pFly->GetObjRect().Pos() );
        for ( std::vector<Point*>::iterator aObjPosIter = maObjPositions.begin();
              aObjPosIter != maObjPositions.end();
              ++aObjPosIter )
        {
            if ( *(pNewObjPos) == *(*aObjPosIter) )
            {
                // position already occurred -> oscillation
                bOscillationDetected = true;
                delete pNewObjPos;
                break;
            }
        }
        if ( !bOscillationDetected )
        {
            maObjPositions.push_back( pNewObjPos );
        }
    }

    return bOscillationDetected;
}

/**
|*      With a paragraph-anchored fly it's absolutely possible that
|*      the anchor reacts to changes of the fly. To this reaction the fly must
|*      certainly react too. Sadly this can lead to oscillations; for example the
|*      fly wants to go down therefore the content can go up - this leads to a
|*      smaller TextFrm thus the fly needs to go up again whereby the text will
|*      get pushed down...
|*      To avoid such oscillations, a small position stack is built. If the fly
|*      reaches a position which it already had once, the action is stopped.
|*      To not run into problems, the stack is designed to hold five positions.
|*      If the stack flows over, the action is stopped too.
|*      Cancellation leads to the situation that the fly has a bad position in
|*      the end. In case of cancellation, the frame is set to automatic top
|*      alignment to not trigger a 'big oscillation' when calling from outside
|*      again.
|*/
void SwFlyAtCntFrm::MakeAll(vcl::RenderContext* pRenderContext)
{
    if ( !GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !SwOszControl::IsInProgress( this ) && !IsLocked() && !IsColLocked() )
    {
        // #i28701# - use new method <GetPageFrm()>
        if( !GetPageFrm() && GetAnchorFrm() && GetAnchorFrm()->IsInFly() )
        {
            SwFlyFrm* pFly = AnchorFrm()->FindFlyFrm();
            SwPageFrm *pTmpPage = pFly ? pFly->FindPageFrm() : nullptr;
            if( pTmpPage )
                pTmpPage->AppendFlyToPage( this );
        }
        // #i28701# - use new method <GetPageFrm()>
        if( GetPageFrm() )
        {
            bSetCompletePaintOnInvalidate = true;
            {
                SwFlyFrameFormat *pFormat = GetFormat();
                const SwFormatFrmSize &rFrmSz = GetFormat()->GetFrmSize();
                if( rFrmSz.GetHeightPercent() != SwFormatFrmSize::SYNCED &&
                    rFrmSz.GetHeightPercent() >= 100 )
                {
                    pFormat->LockModify();
                    SwFormatSurround aMain( pFormat->GetSurround() );
                    if ( aMain.GetSurround() == SURROUND_NONE )
                    {
                        aMain.SetSurround( SURROUND_THROUGHT );
                        pFormat->SetFormatAttr( aMain );
                    }
                    pFormat->UnlockModify();
                }
            }

            SwOszControl aOszCntrl( this );

            // #i43255#
            // #i50356# - format the anchor frame, which
            // contains the anchor position. E.g., for at-character anchored
            // object this can be the follow frame of the anchor frame.
            const bool bFormatAnchor =
                    !static_cast<const SwTextFrm*>( GetAnchorFrmContainingAnchPos() )->IsAnyJoinLocked() &&
                    !ConsiderObjWrapInfluenceOnObjPos() &&
                    !ConsiderObjWrapInfluenceOfOtherObjs();

            const SwFrm* pFooter = GetAnchorFrm()->FindFooterOrHeader();
            if( pFooter && !pFooter->IsFooterFrm() )
                pFooter = nullptr;
            bool bOsz = false;
            bool bExtra = Lower() && Lower()->IsColumnFrm();
            // #i3317# - boolean, to apply temporarly the
            // 'straightforward positioning process' for the frame due to its
            // overlapping with a previous column.
            bool bConsiderWrapInfluenceDueToOverlapPrevCol( false );
            //  #i35911# - boolean, to apply temporarly the
            // 'straightforward positioning process' for the frame due to fact
            // that it causes the complete content of its layout environment
            // to move forward.
            // #i40444# - extend usage of this boolean:
            // apply temporarly the 'straightforward positioning process' for
            // the frame due to the fact that the frame clears the area for
            // the anchor frame, thus it has to move forward.
            bool bConsiderWrapInfluenceDueToMovedFwdAnchor( false );
            do {
                SWRECTFN( this )
                Point aOldPos( (Frm().*fnRect->fnGetPos)() );
                SwFlyFreeFrm::MakeAll(pRenderContext);
                const bool bPosChgDueToOwnFormat =
                                        aOldPos != (Frm().*fnRect->fnGetPos)();
                // #i3317#
                if ( !ConsiderObjWrapInfluenceOnObjPos() &&
                     OverlapsPrevColumn() )
                {
                    bConsiderWrapInfluenceDueToOverlapPrevCol = true;
                }
                // #i28701# - no format of anchor frame, if
                // wrapping style influence is considered on object positioning
                if ( bFormatAnchor )
                {
                    SwTextFrm& rAnchPosAnchorFrm =
                            dynamic_cast<SwTextFrm&>(*GetAnchorFrmContainingAnchPos());
                    // #i58182# - For the usage of new method
                    // <SwObjectFormatterTextFrm::CheckMovedFwdCondition(..)>
                    // to check move forward of anchor frame due to the object
                    // positioning it's needed to know, if the object is anchored
                    // at the master frame before the anchor frame is formatted.
                    const bool bAnchoredAtMaster(!rAnchPosAnchorFrm.IsFollow());

                    // #i56300#
                    // perform complete format of anchor text frame and its
                    // previous frames, which have become invalid due to the
                    // fly frame format.
                    SwObjectFormatterTextFrm::FormatAnchorFrmAndItsPrevs( rAnchPosAnchorFrm );
                    // #i35911#
                    // #i40444#
                    // #i58182# - usage of new method
                    // <SwObjectFormatterTextFrm::CheckMovedFwdCondition(..)>
                    sal_uInt32 nToPageNum( 0L );
                    bool bDummy( false );
                    if ( SwObjectFormatterTextFrm::CheckMovedFwdCondition(
                                        *this, GetPageFrm()->GetPhyPageNum(),
                                        bAnchoredAtMaster, nToPageNum, bDummy ) )
                    {
                        bConsiderWrapInfluenceDueToMovedFwdAnchor = true;
                        // mark anchor text frame
                        // directly, that it is moved forward by object positioning.
                        SwTextFrm* pAnchorTextFrm( static_cast<SwTextFrm*>(AnchorFrm()) );
                        bool bInsert( true );
                        sal_uInt32 nAnchorFrmToPageNum( 0L );
                        const SwDoc& rDoc = *(GetFrameFormat().GetDoc());
                        if ( SwLayouter::FrmMovedFwdByObjPos(
                                                rDoc, *pAnchorTextFrm, nAnchorFrmToPageNum ) )
                        {
                            if ( nAnchorFrmToPageNum < nToPageNum )
                                SwLayouter::RemoveMovedFwdFrm( rDoc, *pAnchorTextFrm );
                            else
                                bInsert = false;
                        }
                        if ( bInsert )
                        {
                            SwLayouter::InsertMovedFwdFrm( rDoc, *pAnchorTextFrm,
                                                           nToPageNum );
                        }
                    }
                }

                if ( aOldPos != (Frm().*fnRect->fnGetPos)() ||
                     ( !GetValidPosFlag() &&
                       ( pFooter || bPosChgDueToOwnFormat ) ) )
                {
                    bOsz = aOszCntrl.ChkOsz();

                    // special loop prevention for dedicated document:
                    if ( bOsz &&
                         HasFixSize() && IsClipped() &&
                         GetAnchorFrm()->GetUpper()->IsCellFrm() )
                    {
                        SwFrameFormat* pFormat = GetFormat();
                        const SwFormatFrmSize& rFrmSz = pFormat->GetFrmSize();
                        if ( rFrmSz.GetWidthPercent() &&
                             rFrmSz.GetHeightPercent() == SwFormatFrmSize::SYNCED )
                        {
                            SwFormatSurround aSurround( pFormat->GetSurround() );
                            if ( aSurround.GetSurround() == SURROUND_NONE )
                            {
                                pFormat->LockModify();
                                aSurround.SetSurround( SURROUND_THROUGHT );
                                pFormat->SetFormatAttr( aSurround );
                                pFormat->UnlockModify();
                                bOsz = false;
                                OSL_FAIL( "<SwFlyAtCntFrm::MakeAll()> - special loop prevention for dedicated document of b6403541 applied" );
                            }
                        }
                    }
                }

                if ( bExtra && Lower() && !Lower()->GetValidPosFlag() )
                {
                    // If a multi column frame leaves invalid columns because of
                    // a position change, we loop once more and format
                    // our content using FormatWidthCols again.
                    _InvalidateSize();
                    bExtra = false; // Ensure only one additional loop run
                }
            } while ( !IsValid() && !bOsz &&
                      // #i3317#
                      !bConsiderWrapInfluenceDueToOverlapPrevCol &&
                      // #i40444#
                      !bConsiderWrapInfluenceDueToMovedFwdAnchor &&
                      GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) );

            // #i3317# - instead of attribute change apply
            // temporarly the 'straightforward positioning process'.
            // #i80924#
            // handle special case during splitting of table rows
            if ( bConsiderWrapInfluenceDueToMovedFwdAnchor &&
                 GetAnchorFrm()->IsInTab() &&
                 GetAnchorFrm()->IsInFollowFlowRow() )
            {
                const SwFrm* pCellFrm = GetAnchorFrm();
                while ( pCellFrm && !pCellFrm->IsCellFrm() )
                {
                    pCellFrm = pCellFrm->GetUpper();
                }
                if ( pCellFrm )
                {
                    SWRECTFN( pCellFrm )
                    if ( (pCellFrm->Frm().*fnRect->fnGetTop)() == 0 &&
                         (pCellFrm->Frm().*fnRect->fnGetHeight)() == 0 )
                    {
                        bConsiderWrapInfluenceDueToMovedFwdAnchor = false;
                    }
                }
            }
            if ( bOsz || bConsiderWrapInfluenceDueToOverlapPrevCol ||
                 // #i40444#
                 bConsiderWrapInfluenceDueToMovedFwdAnchor )
            {
                SetTmpConsiderWrapInfluence( true );
                SetRestartLayoutProcess( true );
                SetTmpConsiderWrapInfluenceOfOtherObjs( true );
            }
            bSetCompletePaintOnInvalidate = false;
        }
    }
}

/** method to determine, if a <MakeAll()> on the Writer fly frame is possible

    #i28701#
*/
bool SwFlyAtCntFrm::IsFormatPossible() const
{
    return SwFlyFreeFrm::IsFormatPossible() &&
           !SwOszControl::IsInProgress( this );
}

class SwDistance
{
public:
    SwTwips nMain, nSub;
    SwDistance() { nMain = nSub = 0; }
    SwDistance& operator=( const SwDistance &rTwo )
        { nMain = rTwo.nMain; nSub = rTwo.nSub; return *this; }
    bool operator<( const SwDistance& rTwo ) const
        { return nMain < rTwo.nMain || ( nMain == rTwo.nMain && nSub &&
          rTwo.nSub && nSub < rTwo.nSub ); }
    bool operator<=( const SwDistance& rTwo ) const
        { return nMain < rTwo.nMain || ( nMain == rTwo.nMain && ( !nSub ||
          !rTwo.nSub || nSub <= rTwo.nSub ) ); }
};

static const SwFrm * lcl_CalcDownDist( SwDistance &rRet,
                                         const Point &rPt,
                                         const SwContentFrm *pCnt )
{
    rRet.nSub = 0;
    //If the point stays inside the Cnt everything is clear already; the Content
    //automatically has a distance of 0.
    if ( pCnt->Frm().IsInside( rPt ) )
    {
        rRet.nMain = 0;
        return pCnt;
    }
    else
    {
        const SwLayoutFrm *pUp = pCnt->IsInTab() ? pCnt->FindTabFrm()->GetUpper() : pCnt->GetUpper();
        // single column sections need to interconnect to their upper
        while( pUp->IsSctFrm() )
            pUp = pUp->GetUpper();
        const bool bVert = pUp->IsVertical();

        const bool bVertL2R = pUp->IsVertLR();

        //Follow the text flow.
        // #i70582#
        // --> OD 2009-03-05 - adopted for Support for Classical Mongolian Script
        const SwTwips nTopForObjPos = lcl_GetTopForObjPos(pCnt, bVert, bVertL2R);
        if ( pUp->Frm().IsInside( rPt ) )
        {
            // <rPt> point is inside environment of given content frame
            // #i70582#
            if( bVert )
            {
                if ( bVertL2R )
                    rRet.nMain =  rPt.X() - nTopForObjPos;
                else
                    rRet.nMain =  nTopForObjPos - rPt.X();
            }
            else
                rRet.nMain =  rPt.Y() - nTopForObjPos;
            return pCnt;
        }
        else if ( rPt.Y() <= pUp->Frm().Top() )
        {
            // <rPt> point is above environment of given content frame
            // correct for vertical layout?
            rRet.nMain = LONG_MAX;
        }
        else if( rPt.X() < pUp->Frm().Left() &&
                 rPt.Y() <= ( bVert ? pUp->Frm().Top() : pUp->Frm().Bottom() ) )
        {
            // <rPt> point is left of environment of given content frame
            // seems not to be correct for vertical layout!?
            const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, false, pCnt );
            if( !pLay ||
                (bVert && (pLay->Frm().Top() + pLay->Prt().Bottom()) <rPt.Y())||
                (!bVert && (pLay->Frm().Left() + pLay->Prt().Right())<rPt.X()) )
            {
                // <rPt> point is in left border of environment
                // #i70582#
                if( bVert )
                {
                    if ( bVertL2R )
                        rRet.nMain = rPt.X() - nTopForObjPos;
                    else
                        rRet.nMain =  nTopForObjPos - rPt.X();
                }
                else
                    rRet.nMain = rPt.Y() - nTopForObjPos;
                return pCnt;
            }
            else
                rRet.nMain = LONG_MAX;
        }
        else
        {
            rRet.nMain = bVert
                ? ( bVertL2R
                    ? ( (pUp->Frm().Left() + pUp->Prt().Right()) - nTopForObjPos )
                    : ( nTopForObjPos - (pUp->Frm().Left() + pUp->Prt().Left() ) ) )
                : ( (pUp->Frm().Top() + pUp->Prt().Bottom()) - nTopForObjPos );

            const SwFrm *pPre = pCnt;
            const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, true, pCnt );
            SwTwips nFrmTop = 0;
            SwTwips nPrtHeight = 0;
            bool bSct = false;
            const SwSectionFrm *pSect = pUp->FindSctFrm();
            if( pSect )
            {
                rRet.nSub = rRet.nMain;
                rRet.nMain = 0;
            }
            if( pSect && !pSect->IsAnLower( pLay ) )
            {
                bSct = false;
                const SwSectionFrm* pNxtSect = pLay ? pLay->FindSctFrm() : nullptr;
                if (pSect->IsAnFollow(pNxtSect) && pLay)
                {
                    if( pLay->IsVertical() )
                    {
                        if ( pLay->IsVertLR() )
                            nFrmTop = pLay->Frm().Left();
                        else
                            nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
                        nPrtHeight = pLay->Prt().Width();
                    }
                    else
                    {
                        nFrmTop = pLay->Frm().Top();
                        nPrtHeight = pLay->Prt().Height();
                    }
                    pSect = pNxtSect;
                }
                else
                {
                    pLay = pSect->GetUpper();
                    if( pLay->IsVertical() )
                    {
                        if ( pLay->IsVertLR() )
                        {
                            nFrmTop = pSect->Frm().Right();
                            nPrtHeight = pLay->Frm().Left() + pLay->Prt().Left()
                                         + pLay->Prt().Width() - pSect->Frm().Left()
                                         - pSect->Frm().Width();
                         }
                         else
                         {
                             nFrmTop = pSect->Frm().Left();
                             nPrtHeight = pSect->Frm().Left() - pLay->Frm().Left()
                                     - pLay->Prt().Left();
                          }
                    }
                    else
                    {
                        nFrmTop = pSect->Frm().Bottom();
                        nPrtHeight = pLay->Frm().Top() + pLay->Prt().Top()
                                     + pLay->Prt().Height() - pSect->Frm().Top()
                                     - pSect->Frm().Height();
                    }
                    pSect = nullptr;
                }
            }
            else if( pLay )
            {
                if( pLay->IsVertical() )
                {
                    if ( pLay->IsVertLR() )
                    {
                        nFrmTop = pLay->Frm().Left();
                        nPrtHeight = pLay->Prt().Width();
                    }
                    else
                    {
                        nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
                        nPrtHeight = pLay->Prt().Width();
                    }
                }
                else
                {
                    nFrmTop = pLay->Frm().Top();
                    nPrtHeight = pLay->Prt().Height();
                }
                bSct = nullptr != pSect;
            }
            while ( pLay && !pLay->Frm().IsInside( rPt ) &&
                    ( pLay->Frm().Top() <= rPt.Y() || pLay->IsInFly() ||
                      ( pLay->IsInSct() &&
                      pLay->FindSctFrm()->GetUpper()->Frm().Top() <= rPt.Y())) )
            {
                if ( pLay->IsFootnoteContFrm() )
                {
                    if ( !static_cast<const SwLayoutFrm*>(pLay)->Lower() )
                    {
                        SwFrm *pDel = const_cast<SwFrm*>(pLay);
                        pDel->Cut();
                        SwFrm::DestroyFrm(pDel);
                        return pPre;
                    }
                    return nullptr;
                }
                else
                {
                    if( bSct || pSect )
                        rRet.nSub += nPrtHeight;
                    else
                        rRet.nMain += nPrtHeight;
                    pPre = pLay;
                    pLay = pLay->GetLeaf( MAKEPAGE_NONE, true, pCnt );
                    if( pSect && !pSect->IsAnLower( pLay ) )
                    {   // If we're leaving a SwSectionFrm, the next Leaf-Frm
                        // is the part of the upper below the SectionFrm.
                        const SwSectionFrm* pNxtSect = pLay ?
                            pLay->FindSctFrm() : nullptr;
                        bSct = false;
                        if (pLay && pSect->IsAnFollow(pNxtSect))
                        {
                            pSect = pNxtSect;
                            if( pLay->IsVertical() )
                            {
                                if ( pLay->IsVertLR() )
                                {
                                    nFrmTop = pLay->Frm().Left();
                                    nPrtHeight = pLay->Prt().Width();
                                }
                                else
                                {
                                    nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
                                    nPrtHeight = pLay->Prt().Width();
                                }
                            }
                            else
                            {
                                nFrmTop = pLay->Frm().Top();
                                nPrtHeight = pLay->Prt().Height();
                            }
                        }
                        else
                        {
                            pLay = pSect->GetUpper();
                            if( pLay->IsVertical() )
                            {
                                if ( pLay->IsVertLR() )
                                {
                                    nFrmTop = pSect->Frm().Right();
                                    nPrtHeight = pLay->Frm().Left()+pLay->Prt().Left()
                                             + pLay->Prt().Width() - pSect->Frm().Left()
                                             - pSect->Frm().Width();
                                }
                                else
                                {
                                    nFrmTop = pSect->Frm().Left();
                                    nPrtHeight = pSect->Frm().Left() -
                                            pLay->Frm().Left() - pLay->Prt().Left();
                                }
                            }
                            else
                            {
                                nFrmTop = pSect->Frm().Bottom();
                                nPrtHeight = pLay->Frm().Top()+pLay->Prt().Top()
                                     + pLay->Prt().Height() - pSect->Frm().Top()
                                     - pSect->Frm().Height();
                            }
                            pSect = nullptr;
                        }
                    }
                    else if( pLay )
                    {
                        if( pLay->IsVertical() )
                        {
                             if ( pLay->IsVertLR() )
                              {
                                 nFrmTop = pLay->Frm().Left();
                                 nPrtHeight = pLay->Prt().Width();
                             }
                             else
                             {
                                 nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
                                 nPrtHeight = pLay->Prt().Width();
                             }
                        }
                        else
                        {
                            nFrmTop = pLay->Frm().Top();
                            nPrtHeight = pLay->Prt().Height();
                        }
                        bSct = nullptr != pSect;
                    }
                }
            }
            if ( pLay )
            {
                if ( pLay->Frm().IsInside( rPt ) )
                {
                    SwTwips nDiff = pLay->IsVertical() ? ( pLay->IsVertLR() ? ( rPt.X() - nFrmTop ) : ( nFrmTop - rPt.X() ) )
                                                       : ( rPt.Y() - nFrmTop );
                    if( bSct || pSect )
                        rRet.nSub += nDiff;
                    else
                        rRet.nMain += nDiff;
                }
                if ( pLay->IsFootnoteContFrm() && !static_cast<const SwLayoutFrm*>(pLay)->Lower() )
                {
                    SwFrm *pDel = const_cast<SwFrm*>(pLay);
                    pDel->Cut();
                    SwFrm::DestroyFrm(pDel);
                    return nullptr;
                }
                return pLay;
            }
            else
                rRet.nMain = LONG_MAX;
        }
    }
    return nullptr;
}

static sal_uInt64 lcl_FindCntDiff( const Point &rPt, const SwLayoutFrm *pLay,
                          const SwContentFrm *& rpCnt,
                          const bool bBody, const bool bFootnote )
{
    // Searches below pLay the nearest Cnt to the point. The reference point of
    //the Contents is always the left upper corner.
    //The Cnt should preferably be above the point.

    rpCnt = nullptr;
    sal_uInt64 nDistance = SAL_MAX_UINT64;
    sal_uInt64 nNearest  = SAL_MAX_UINT64;
    const SwContentFrm *pCnt = pLay ? pLay->ContainsContent() : nullptr;

    while ( pCnt && (bBody != pCnt->IsInDocBody() || bFootnote != pCnt->IsInFootnote()))
    {
        pCnt = pCnt->GetNextContentFrm();
        if ( !pLay->IsAnLower( pCnt ) )
            pCnt = nullptr;
    }
    const SwContentFrm *pNearest = pCnt;
    if ( pCnt )
    {
        do
        {
            //Calculate the distance between those two points.
            //'delta' X^2 + 'delta' Y^2 = 'distance'^2
            sal_uInt64 dX = std::max( pCnt->Frm().Left(), rPt.X() ) -
                       std::min( pCnt->Frm().Left(), rPt.X() ),
                  dY = std::max( pCnt->Frm().Top(), rPt.Y() ) -
                       std::min( pCnt->Frm().Top(), rPt.Y() );
            // square of the difference will do fine here
            const sal_uInt64 nDiff = (dX * dX) + (dY * dY);
            if ( pCnt->Frm().Top() <= rPt.Y() )
            {
                if ( nDiff < nDistance )
                {
                    //This one is the nearer one
                    nDistance = nNearest = nDiff;
                    rpCnt = pNearest = pCnt;
                }
            }
            else if ( nDiff < nNearest )
            {
                nNearest = nDiff;
                pNearest = pCnt;
            }
            pCnt = pCnt->GetNextContentFrm();
            while ( pCnt &&
                    (bBody != pCnt->IsInDocBody() || bFootnote != pCnt->IsInFootnote()))
                pCnt = pCnt->GetNextContentFrm();

        }  while ( pCnt && pLay->IsAnLower( pCnt ) );
    }
    if (nDistance == SAL_MAX_UINT64)
    {   rpCnt = pNearest;
        return nNearest;
    }
    return nDistance;
}

static const SwContentFrm * lcl_FindCnt( const Point &rPt, const SwContentFrm *pCnt,
                                  const bool bBody, const bool bFootnote )
{
    //Starting from pCnt searches the ContentFrm whose left upper corner is the
    //nearest to the point.
    //Always returns a ContentFrm.

    //First the nearest Content inside the page which contains the Content is
    //searched. Starting from this page the pages in both directions need to
    //be considered. If possible a Content is returned whose Y-position is
    //above the point.
    const SwContentFrm  *pRet, *pNew;
    const SwLayoutFrm *pLay = pCnt->FindPageFrm();
    sal_uInt64 nDist; // not sure if a sal_Int32 would be enough?

    nDist = ::lcl_FindCntDiff( rPt, pLay, pNew, bBody, bFootnote );
    if ( pNew )
        pRet = pNew;
    else
    {   pRet  = pCnt;
        nDist = SAL_MAX_UINT64;
    }
    const SwContentFrm *pNearest = pRet;
    sal_uInt64 nNearest = nDist;

    if ( pLay )
    {
        const SwLayoutFrm *pPge = pLay;
        sal_uInt64 nOldNew = SAL_MAX_UINT64;
        for ( int i = 0; pPge->GetPrev() && (i < 3); ++i )
        {
            pPge = static_cast<const SwLayoutFrm*>(pPge->GetPrev());
            const sal_uInt64 nNew = ::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFootnote );
            if ( nNew < nDist )
            {
                if ( pNew->Frm().Top() <= rPt.Y() )
                {
                    pRet = pNearest = pNew;
                    nDist = nNearest = nNew;
                }
                else if ( nNew < nNearest )
                {
                    pNearest = pNew;
                    nNearest = nNew;
                }
            }
            else if (nOldNew != SAL_MAX_UINT64 && nNew > nOldNew)
                break;
            else
                nOldNew = nNew;

        }
        pPge = pLay;
        nOldNew = SAL_MAX_UINT64;
        for ( int j = 0; pPge->GetNext() && (j < 3); ++j )
        {
            pPge = static_cast<const SwLayoutFrm*>(pPge->GetNext());
            const sal_uInt64 nNew = ::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFootnote );
            if ( nNew < nDist )
            {
                if ( pNew->Frm().Top() <= rPt.Y() )
                {
                    pRet = pNearest = pNew;
                    nDist = nNearest = nNew;
                }
                else if ( nNew < nNearest )
                {
                    pNearest = pNew;
                    nNearest = nNew;
                }
            }
            else if (nOldNew != SAL_MAX_UINT64 && nNew > nOldNew)
                break;
            else
                nOldNew = nNew;
        }
    }
    if ( (pRet->Frm().Top() > rPt.Y()) )
        return pNearest;
    else
        return pRet;
}

static void lcl_PointToPrt( Point &rPoint, const SwFrm *pFrm )
{
    SwRect aTmp( pFrm->Prt() );
    aTmp += pFrm->Frm().Pos();
    if ( rPoint.getX() < aTmp.Left() )
        rPoint.setX(aTmp.Left());
    else if ( rPoint.getX() > aTmp.Right() )
        rPoint.setX(aTmp.Right());
    if ( rPoint.getY() < aTmp.Top() )
        rPoint.setY(aTmp.Top());
    else if ( rPoint.getY() > aTmp.Bottom() )
        rPoint.setY(aTmp.Bottom());

}

/** Searches an anchor for paragraph bound objects starting from pOldAnch.
 *
 *  This is used to show anchors as well as changing anchors
 *  when dragging paragraph bound objects.
 */
const SwContentFrm *FindAnchor( const SwFrm *pOldAnch, const Point &rNew,
                              const bool bBodyOnly )
{
    //Search the nearest Cnt around the given document position in the text
    //flow. The given anchor is the starting Frm.
    const SwContentFrm* pCnt;
    if ( pOldAnch->IsContentFrm() )
    {
        pCnt = static_cast<const SwContentFrm*>(pOldAnch);
    }
    else
    {
        Point aTmp( rNew );
        const SwLayoutFrm *pTmpLay = static_cast<const SwLayoutFrm*>(pOldAnch);
        if( pTmpLay->IsRootFrm() )
        {
            SwRect aTmpRect( aTmp, Size(0,0) );
            pTmpLay = static_cast<const SwLayoutFrm*>(::FindPage( aTmpRect, pTmpLay->Lower() ));
        }
        pCnt = pTmpLay->GetContentPos( aTmp, false, bBodyOnly );
    }

    //Take care to use meaningful ranges during search. This means to not enter
    //or leave header/footer in this case.
    const bool bBody = pCnt->IsInDocBody() || bBodyOnly;
    const bool bFootnote  = !bBodyOnly && pCnt->IsInFootnote();

    Point aNew( rNew );
    if ( bBody )
    {
        //#38848 drag from page margin into the body.
        const SwFrm *pPage = pCnt->FindPageFrm();
        ::lcl_PointToPrt( aNew, pPage->GetUpper() );
        SwRect aTmp( aNew, Size( 0, 0 ) );
        pPage = ::FindPage( aTmp, pPage );
        ::lcl_PointToPrt( aNew, pPage );
    }

    if ( pCnt->IsInDocBody() == bBody && pCnt->Frm().IsInside( aNew ) )
        return pCnt;
    else if ( pOldAnch->IsInDocBody() || pOldAnch->IsPageFrm() )
    {
        // Maybe the selected anchor is on the same page as the current anchor.
        // With this we won't run into problems with the columns.
        Point aTmp( aNew );
        const SwContentFrm *pTmp = pCnt->FindPageFrm()->
                                        GetContentPos( aTmp, false, true );
        if ( pTmp && pTmp->Frm().IsInside( aNew ) )
            return pTmp;
    }

    //Starting from the anchor we now search in both directions until we found
    //the nearest one respectively.
    //Not the direct distance is relevant but the distance which needs to be
    //traveled through the text flow.
    const SwContentFrm *pUpLst;
    const SwContentFrm *pUpFrm = pCnt;
    SwDistance nUp, nUpLst;
    ::lcl_CalcDownDist( nUp, aNew, pUpFrm );
    SwDistance nDown = nUp;
    bool bNegAllowed = true;// Make it possible to leave the negative section once.
    do
    {
        pUpLst = pUpFrm; nUpLst = nUp;
        pUpFrm = pUpLst->GetPrevContentFrm();
        while ( pUpFrm &&
                (bBody != pUpFrm->IsInDocBody() || bFootnote != pUpFrm->IsInFootnote()))
            pUpFrm = pUpFrm->GetPrevContentFrm();
        if ( pUpFrm )
        {
            ::lcl_CalcDownDist( nUp, aNew, pUpFrm );
            //It makes sense to search further, if the distance grows inside
            //a table.
            if ( pUpLst->IsInTab() && pUpFrm->IsInTab() )
            {
                while ( pUpFrm && ((nUpLst < nUp && pUpFrm->IsInTab()) ||
                        bBody != pUpFrm->IsInDocBody()) )
                {
                    pUpFrm = pUpFrm->GetPrevContentFrm();
                    if ( pUpFrm )
                        ::lcl_CalcDownDist( nUp, aNew, pUpFrm );
                }
            }
        }
        if ( !pUpFrm )
            nUp.nMain = LONG_MAX;
        if ( nUp.nMain >= 0 && LONG_MAX != nUp.nMain )
        {
            bNegAllowed = false;
            if ( nUpLst.nMain < 0 ) //don't take the wrong one, if the value
                                    //just changed from negative to positive.
            {   pUpLst = pUpFrm;
                nUpLst = nUp;
            }
        }
    } while ( pUpFrm && ( ( bNegAllowed && nUp.nMain < 0 ) || ( nUp <= nUpLst ) ) );

    const SwContentFrm *pDownLst;
    const SwContentFrm *pDownFrm = pCnt;
    SwDistance nDownLst;
    if ( nDown.nMain < 0 )
        nDown.nMain = LONG_MAX;
    do
    {
        pDownLst = pDownFrm; nDownLst = nDown;
        pDownFrm = pDownLst->GetNextContentFrm();
        while ( pDownFrm &&
                (bBody != pDownFrm->IsInDocBody() || bFootnote != pDownFrm->IsInFootnote()))
            pDownFrm = pDownFrm->GetNextContentFrm();
        if ( pDownFrm )
        {
            ::lcl_CalcDownDist( nDown, aNew, pDownFrm );
            if ( nDown.nMain < 0 )
                nDown.nMain = LONG_MAX;
            //It makes sense to search further, if the distance grows inside
            //a table.
            if ( pDownLst->IsInTab() && pDownFrm->IsInTab() )
            {
                while ( pDownFrm && ( ( nDown.nMain != LONG_MAX && pDownFrm->IsInTab()) || bBody != pDownFrm->IsInDocBody() ) )
                {
                    pDownFrm = pDownFrm->GetNextContentFrm();
                    if ( pDownFrm )
                        ::lcl_CalcDownDist( nDown, aNew, pDownFrm );
                    if ( nDown.nMain < 0 )
                        nDown.nMain = LONG_MAX;
                }
            }
        }
        if ( !pDownFrm )
            nDown.nMain = LONG_MAX;

    } while ( pDownFrm && nDown <= nDownLst &&
              nDown.nMain != LONG_MAX && nDownLst.nMain != LONG_MAX );

    //If we couldn't find one in both directions, we'll search the Content whose
    //left upper corner is the nearest to the point. Such a situation may
    //happen, if the point doesn't lay in the text flow but in any margin.
    if ( nDownLst.nMain == LONG_MAX && nUpLst.nMain == LONG_MAX )
    {
        // If an OLE objects, which is contained in a fly frame
        // is resized in inplace mode and the new Position is outside the
        // fly frame, we do not want to leave our fly frame.
        if ( pCnt->IsInFly() )
            return pCnt;

        return ::lcl_FindCnt( aNew, pCnt, bBody, bFootnote );
    }
    else
        return nDownLst < nUpLst ? pDownLst : pUpLst;
}

void SwFlyAtCntFrm::SetAbsPos( const Point &rNew )
{
    SwPageFrm *pOldPage = FindPageFrm();
    const SwRect aOld( GetObjRectWithSpaces() );
    Point aNew( rNew );

    if( ( GetAnchorFrm()->IsVertical() && !GetAnchorFrm()->IsVertLR() ) || GetAnchorFrm()->IsRightToLeft() )
        aNew.setX(aNew.getX() + Frm().Width());
    SwContentFrm *pCnt = const_cast<SwContentFrm*>(::FindAnchor( GetAnchorFrm(), aNew ));
    if( pCnt->IsProtected() )
        pCnt = const_cast<SwContentFrm*>(static_cast<const SwContentFrm*>(GetAnchorFrm()));

    SwPageFrm *pTmpPage = nullptr;
    const bool bVert = pCnt->IsVertical();

    const bool bVertL2R = pCnt->IsVertLR();
    const bool bRTL = pCnt->IsRightToLeft();

    if( ( !bVert != !GetAnchorFrm()->IsVertical() ) ||
        ( !bRTL !=  !GetAnchorFrm()->IsRightToLeft() ) )
    {
        if( bVert || bRTL )
            aNew.setX(aNew.getX() + Frm().Width());
        else
            aNew.setX(aNew.getX() - Frm().Width());
    }

    if ( pCnt->IsInDocBody() )
    {
        //#38848 drag from page margin into the body.
        pTmpPage = pCnt->FindPageFrm();
        ::lcl_PointToPrt( aNew, pTmpPage->GetUpper() );
        SwRect aTmp( aNew, Size( 0, 0 ) );
        pTmpPage = const_cast<SwPageFrm*>(static_cast<const SwPageFrm*>(::FindPage( aTmp, pTmpPage )));
        ::lcl_PointToPrt( aNew, pTmpPage );
    }

    //Setup RelPos, only invalidate if requested.
    //rNew is an absolute position. We need to calculate the distance from rNew
    //to the anchor inside the text flow to correctly set RelPos.
//!!!!!We can optimize here: FindAnchor could also return RelPos!
    const SwFrm *pFrm = nullptr;
    SwTwips nY;
    if ( pCnt->Frm().IsInside( aNew ) )
    {
        // #i70582#
        if ( bVert )
        {
            nY = pCnt->Frm().Left() - rNew.X();
            if ( bVertL2R )
                nY = -nY;
            nY -= pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
        }
        else
            nY = rNew.Y() - pCnt->Frm().Top() + pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
    }
    else
    {
        SwDistance aDist;
        pFrm = ::lcl_CalcDownDist( aDist, aNew, pCnt );
        nY = aDist.nMain + aDist.nSub;
    }

    SwTwips nX = 0;

    if ( pCnt->IsFollow() )
    {
        // Flys are never attached to the follow but always to the master,
        // which we're going to search now.
        const SwContentFrm *pOriginal = pCnt;
        const SwContentFrm *pFollow = pCnt;
        while ( pCnt->IsFollow() )
        {
            do
            {
                SwContentFrm* pPrev = pCnt->GetPrevContentFrm();
                if (!pPrev)
                {
                    SAL_WARN("sw.core", "very unexpected missing PrevContentFrm");
                    break;
                }
                pCnt = pPrev;
            }
            while ( pCnt->GetFollow() != pFollow );
            pFollow = pCnt;
        }
        SwTwips nDiff = 0;
        do
        {   const SwFrm *pUp = pFollow->GetUpper();
            if( pUp->IsVertical() )
            {
                if ( pUp->IsVertLR()  )
                    nDiff += pUp->Prt().Width() - pFollow->GetRelPos().getX();
                else
                       nDiff += pFollow->Frm().Left() + pFollow->Frm().Width()
                             - pUp->Frm().Left() - pUp->Prt().Left();
            }
            else
                nDiff += pUp->Prt().Height() - pFollow->GetRelPos().Y();
            pFollow = pFollow->GetFollow();
        } while ( pFollow != pOriginal );
        nY += nDiff;
        if( bVert )
            nX = pCnt->Frm().Top() - pOriginal->Frm().Top();
        else
            nX = pCnt->Frm().Left() - pOriginal->Frm().Left();
    }

    if ( nY == LONG_MAX )
    {
        // #i70582#
        const SwTwips nTopForObjPos = lcl_GetTopForObjPos(pCnt, bVert, bVertL2R);
        if( bVert )
        {
            if ( bVertL2R )
                nY = rNew.X() - nTopForObjPos;
            else
                nY = nTopForObjPos - rNew.X();
        }
        else
        {
            nY = rNew.Y() - nTopForObjPos;
        }
    }

    SwFlyFrameFormat *pFormat = GetFormat();

    if( bVert )
    {
        if( !pFrm )
            nX += rNew.Y() - pCnt->Frm().Top();
        else
            nX = rNew.Y() - pFrm->Frm().Top();
    }
    else
    {
        if( !pFrm )
        {
            if ( pCnt->IsRightToLeft() )
                nX += pCnt->Frm().Right() - rNew.X() - Frm().Width();
            else
                nX += rNew.X() - pCnt->Frm().Left();
        }
        else
        {
            if ( pFrm->IsRightToLeft() )
                nX += pFrm->Frm().Right() - rNew.X() - Frm().Width();
            else
                nX = rNew.X() - pFrm->Frm().Left();
        }
    }
    GetFormat()->GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );

    if( pCnt != GetAnchorFrm() || ( IsAutoPos() && pCnt->IsTextFrm() &&
                                  GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE)) )
    {
        //Set the anchor attribute according to the new Cnt.
        SwFormatAnchor aAnch( pFormat->GetAnchor() );
        SwPosition pos = *aAnch.GetContentAnchor();
        if( IsAutoPos() && pCnt->IsTextFrm() )
        {
            SwCrsrMoveState eTmpState( MV_SETONLYTEXT );
            Point aPt( rNew );
            if( pCnt->GetCrsrOfst( &pos, aPt, &eTmpState )
                && pos.nNode == *pCnt->GetNode() )
            {
                if ( pCnt->GetNode()->GetTextNode() != nullptr )
                {
                    const SwTextAttr* pTextInputField =
                        pCnt->GetNode()->GetTextNode()->GetTextAttrAt( pos.nContent.GetIndex(), RES_TXTATR_INPUTFIELD, SwTextNode::PARENT );
                    if ( pTextInputField != nullptr )
                    {
                        pos.nContent = pTextInputField->GetStart();
                    }
                }
                ResetLastCharRectHeight();
                if( text::RelOrientation::CHAR == pFormat->GetVertOrient().GetRelationOrient() )
                    nY = LONG_MAX;
                if( text::RelOrientation::CHAR == pFormat->GetHoriOrient().GetRelationOrient() )
                    nX = LONG_MAX;
            }
            else
            {
                pos.nNode = *pCnt->GetNode();
                pos.nContent.Assign( pCnt->GetNode(), 0 );
            }
        }
        else
        {
            pos.nNode = *pCnt->GetNode();
            pos.nContent.Assign( pCnt->GetNode(), 0 );
        }
        aAnch.SetAnchor( &pos );

        // handle change of anchor node:
        // if count of the anchor frame also change, the fly frames have to be
        // re-created. Thus, delete all fly frames except the <this> before the
        // anchor attribute is change and re-create them afterwards.
        {
            SwHandleAnchorNodeChg aHandleAnchorNodeChg( *pFormat, aAnch, this );
            pFormat->GetDoc()->SetAttr( aAnch, *pFormat );
        }
    }
    else if ( pTmpPage && pTmpPage != GetPageFrm() )
        GetPageFrm()->MoveFly( this, pTmpPage );

    const Point aRelPos = bVert ? Point( -nY, nX ) : Point( nX, nY );

    ChgRelPos( aRelPos );

    GetFormat()->GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );

    if ( pOldPage != FindPageFrm() )
        ::Notify_Background( GetVirtDrawObj(), pOldPage, aOld, PREP_FLY_LEAVE, false );
}

/** method to assure that anchored object is registered at the correct
    page frame

    #i28701#
    takes over functionality of deleted method <SwFlyAtCntFrm::AssertPage()>
*/
void SwFlyAtCntFrm::RegisterAtCorrectPage()
{
    SwPageFrm* pPageFrm( nullptr );
    if ( GetVertPosOrientFrm() )
    {
        pPageFrm = const_cast<SwPageFrm*>(GetVertPosOrientFrm()->FindPageFrm());
    }
    if ( pPageFrm && GetPageFrm() != pPageFrm )
    {
        if ( GetPageFrm() )
            GetPageFrm()->MoveFly( this, pPageFrm );
        else
            pPageFrm->AppendFlyToPage( this );
    }
}

// #i26791#
void SwFlyAtCntFrm::MakeObjPos()
{
    // if fly frame position is valid, nothing is to do. Thus, return
    if ( mbValidPos )
    {
        return;
    }

    // #i26791# - validate position flag here.
    mbValidPos = true;

    // #i35911# - no calculation of new position, if
    // anchored object is marked that it clears its environment and its
    // environment is already cleared.
    // before checking for cleared environment
    // check, if member <mpVertPosOrientFrm> is set.
    if ( GetVertPosOrientFrm() &&
         ClearedEnvironment() && HasClearedEnvironment() )
    {
        return;
    }

    // use new class to position object
    objectpositioning::SwToContentAnchoredObjectPosition
            aObjPositioning( *GetVirtDrawObj() );
    aObjPositioning.CalcPosition();

    SetVertPosOrientFrm ( aObjPositioning.GetVertPosOrientFrm() );
}

// #i28701#
bool SwFlyAtCntFrm::_InvalidationAllowed( const InvalidationType _nInvalid ) const
{
    bool bAllowed( SwFlyFreeFrm::_InvalidationAllowed( _nInvalid ) );

    // forbiddance of base instance can't be over ruled.
    if ( bAllowed )
    {
        if ( _nInvalid == INVALID_POS ||
             _nInvalid == INVALID_ALL )
        {
            bAllowed = InvalidationOfPosAllowed();
        }
    }

    return bAllowed;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
