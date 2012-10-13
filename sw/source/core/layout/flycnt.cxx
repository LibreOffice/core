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


#include <tools/bigint.hxx>
#include "pagefrm.hxx"
#include "cntfrm.hxx"
#include "flyfrm.hxx"
#include "txtfrm.hxx"
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include "viewsh.hxx"
#include "viewimp.hxx"
#include "pam.hxx"
#include "frmfmt.hxx"
#include "frmtool.hxx"
#include "dflyobj.hxx"
#include "hints.hxx"
#include "ndtxt.hxx"
#include "swundo.hxx"
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>

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


/*************************************************************************
|*
|*  SwFlyAtCntFrm::SwFlyAtCntFrm()
|*
|*************************************************************************/

SwFlyAtCntFrm::SwFlyAtCntFrm( SwFlyFrmFmt *pFmt, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFreeFrm( pFmt, pSib, pAnch )
{
    bAtCnt = sal_True;
    bAutoPosition = (FLY_AT_CHAR == pFmt->GetAnchor().GetAnchorId());
}

// #i28701#
TYPEINIT1(SwFlyAtCntFrm,SwFlyFreeFrm);
/*************************************************************************
|*
|*  SwFlyAtCntFrm::Modify()
|*
|*************************************************************************/

void SwFlyAtCntFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    sal_uInt16 nWhich = pNew ? pNew->Which() : 0;
    const SwFmtAnchor *pAnch = 0;

    if( RES_ATTRSET_CHG == nWhich && SFX_ITEM_SET ==
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_ANCHOR, sal_False,
            (const SfxPoolItem**)&pAnch ))
        ;       // The anchor pointer is set at GetItemState!

    else if( RES_ANCHOR == nWhich )
    {
        //Change anchor, I move myself to a new place.
        //The anchor type must not change, this is only possible using
        //SwFEShell.
        pAnch = (const SwFmtAnchor*)pNew;
    }

    if( pAnch )
    {
        OSL_ENSURE( pAnch->GetAnchorId() == GetFmt()->GetAnchor().GetAnchorId(),
                "Illegal change of anchor type. " );

        //Unregister, get hold of a new anchor and attach it
        SwRect aOld( GetObjRectWithSpaces() );
        SwPageFrm *pOldPage = FindPageFrm();
        const SwFrm *pOldAnchor = GetAnchorFrm();
        SwCntntFrm *pCntnt = (SwCntntFrm*)GetAnchorFrm();
        AnchorFrm()->RemoveFly( this );

        const sal_Bool bBodyFtn = (pCntnt->IsInDocBody() || pCntnt->IsInFtn());

        // Search the new anchor using the NodeIdx; the relation between old
        // and new NodeIdx determines the search direction
        const SwNodeIndex aNewIdx( pAnch->GetCntntAnchor()->nNode );
        SwNodeIndex aOldIdx( *pCntnt->GetNode() );

        //fix: depending on which index was smaller, searching in the do-while
        //loop previously was done forward or backwards respectively. This however
        //could lead to an infinite loop. To at least avoid the loop, searching
        //is now done in only one direction. Getting hold of a frame from the node
        //is still possible if the new anchor could not be found. Chances are
        //good that this will be the correct one.
        const bool bNext = aOldIdx < aNewIdx;
        // consider the case that at found anchor frame candidate already a
        // fly frame of the given fly format is registered.
        // consider, that <pCntnt> is the already
        // the new anchor frame.
        bool bFound( aOldIdx == aNewIdx );
        while ( pCntnt && !bFound )
        {
            do
            {
                if ( bNext )
                    pCntnt = pCntnt->GetNextCntntFrm();
                else
                    pCntnt = pCntnt->GetPrevCntntFrm();
            } while ( pCntnt &&
                      !( bBodyFtn == ( pCntnt->IsInDocBody() ||
                                       pCntnt->IsInFtn() ) ) );
            if ( pCntnt )
                aOldIdx = *pCntnt->GetNode();

            // check, if at found anchor frame candidate already a fly frame
            // of the given fly frame format is registered.
            bFound = aOldIdx == aNewIdx;
            if ( bFound && pCntnt->GetDrawObjs() )
            {
                SwFrmFmt* pMyFlyFrmFmt( &GetFrmFmt() );
                SwSortedObjs &rObjs = *pCntnt->GetDrawObjs();
                for( sal_uInt16 i = 0; i < rObjs.Count(); ++i)
                {
                    SwFlyFrm* pFlyFrm = dynamic_cast<SwFlyFrm*>(rObjs[i]);
                    if ( pFlyFrm &&
                         &(pFlyFrm->GetFrmFmt()) == pMyFlyFrmFmt )
                    {
                        bFound = false;
                        break;
                    }
                }
            }
        }
        if ( !pCntnt )
        {
            SwCntntNode *pNode = aNewIdx.GetNode().GetCntntNode();
            pCntnt = pNode->getLayoutFrm( getRootFrm(), &pOldAnchor->Frm().Pos(), 0, sal_False );
            OSL_ENSURE( pCntnt, "Neuen Anker nicht gefunden" );
        }
        //Flys are never attached to a follow, but always on the master which
        //we are going to search now.
        SwCntntFrm* pFlow = pCntnt;
        while ( pFlow->IsFollow() )
            pFlow = pFlow->FindMaster();
        pCntnt = pFlow;

        //and *puff* it's attached...
        pCntnt->AppendFly( this );
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

/*************************************************************************
|*
|*  SwFlyAtCntFrm::MakeAll()
|*
|*  Description         With a paragraph-anchored fly it's absolutely possible that
|*      the anchor reacts to changes of the fly. To this reaction the fly must
|*      certaily react too. Sadly this can lead to oscillations; for example the
|*      fly wants to go down therefore the content can go up - this leads to a
|*      smaller TxtFrm thus the fly needs to go up again whereby the text will
|*      get pushed down...
|*      To avoid such oscillations, a small position stack is built. If the fly
|*      reaches a position which it already had once, the action is stopped.
|*      To not run into problems, the stack is designed to hold five positions.
|*      If the stack flows over, the action is stopped too.
|*      Cancellation leads to the situation that the fly has a bad position in
|*      the end. In case of cancellation, the frame is set to automatic top
|*      alignment to not trigger a 'big oscillation' when calling from outside
|*      again.
|*************************************************************************/
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
    SwOszControl( const SwFlyFrm *pFrm );
    ~SwOszControl();
    bool ChkOsz();
    static sal_Bool IsInProgress( const SwFlyFrm *pFly );
};
const SwFlyFrm *SwOszControl::pStk1 = 0;
const SwFlyFrm *SwOszControl::pStk2 = 0;
const SwFlyFrm *SwOszControl::pStk3 = 0;
const SwFlyFrm *SwOszControl::pStk4 = 0;
const SwFlyFrm *SwOszControl::pStk5 = 0;

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
        SwOszControl::pStk1 = 0;
    else if ( SwOszControl::pStk2 == pFly )
        SwOszControl::pStk2 = 0;
    else if ( SwOszControl::pStk3 == pFly )
        SwOszControl::pStk3 = 0;
    else if ( SwOszControl::pStk4 == pFly )
        SwOszControl::pStk4 = 0;
    else if ( SwOszControl::pStk5 == pFly )
        SwOszControl::pStk5 = 0;
    // #i3317#
    while ( !maObjPositions.empty() )
    {
        Point* pPos = maObjPositions.back();
        delete pPos;

        maObjPositions.pop_back();
    }
}

sal_Bool SwOszControl::IsInProgress( const SwFlyFrm *pFly )
{
    if ( SwOszControl::pStk1 && !pFly->IsLowerOf( SwOszControl::pStk1 ) )
        return sal_True;
    if ( SwOszControl::pStk2 && !pFly->IsLowerOf( SwOszControl::pStk2 ) )
        return sal_True;
    if ( SwOszControl::pStk3 && !pFly->IsLowerOf( SwOszControl::pStk3 ) )
        return sal_True;
    if ( SwOszControl::pStk4 && !pFly->IsLowerOf( SwOszControl::pStk4 ) )
        return sal_True;
    if ( SwOszControl::pStk5 && !pFly->IsLowerOf( SwOszControl::pStk5 ) )
        return sal_True;
    return sal_False;
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

void SwFlyAtCntFrm::MakeAll()
{
    if ( !GetFmt()->GetDoc()->IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !SwOszControl::IsInProgress( this ) && !IsLocked() && !IsColLocked() )
    {
        // #i28701# - use new method <GetPageFrm()>
        if( !GetPageFrm() && GetAnchorFrm() && GetAnchorFrm()->IsInFly() )
        {
            SwFlyFrm* pFly = AnchorFrm()->FindFlyFrm();
            SwPageFrm *pTmpPage = pFly ? pFly->FindPageFrm() : NULL;
            if( pTmpPage )
                pTmpPage->AppendFlyToPage( this );
        }
        // #i28701# - use new method <GetPageFrm()>
        if( GetPageFrm() )
        {
            bSetCompletePaintOnInvalidate = sal_True;
            {
                SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
                const SwFmtFrmSize &rFrmSz = GetFmt()->GetFrmSize();
                if( rFrmSz.GetHeightPercent() != 0xFF &&
                    rFrmSz.GetHeightPercent() >= 100 )
                {
                    pFmt->LockModify();
                    SwFmtSurround aMain( pFmt->GetSurround() );
                    if ( aMain.GetSurround() == SURROUND_NONE )
                    {
                        aMain.SetSurround( SURROUND_THROUGHT );
                        pFmt->SetFmtAttr( aMain );
                    }
                    pFmt->UnlockModify();
                }
            }

            SwOszControl aOszCntrl( this );

            // #i43255#
            // #i50356# - format the anchor frame, which
            // contains the anchor position. E.g., for at-character anchored
            // object this can be the follow frame of the anchor frame.
            const bool bFormatAnchor =
                    !static_cast<const SwTxtFrm*>( GetAnchorFrmContainingAnchPos() )->IsAnyJoinLocked() &&
                    !ConsiderObjWrapInfluenceOnObjPos() &&
                    !ConsiderObjWrapInfluenceOfOtherObjs();

            const SwFrm* pFooter = GetAnchorFrm()->FindFooterOrHeader();
            if( pFooter && !pFooter->IsFooterFrm() )
                pFooter = NULL;
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
                SwFlyFreeFrm::MakeAll();
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
                    SwTxtFrm* pAnchPosAnchorFrm =
                            dynamic_cast<SwTxtFrm*>(GetAnchorFrmContainingAnchPos());
                    OSL_ENSURE( pAnchPosAnchorFrm,
                            "<SwFlyAtCntFrm::MakeAll()> - anchor frame of wrong type -> crash" );
                    // #i58182# - For the usage of new method
                    // <SwObjectFormatterTxtFrm::CheckMovedFwdCondition(..)>
                    // to check move forward of anchor frame due to the object
                    // positioning it's needed to know, if the object is anchored
                    // at the master frame before the anchor frame is formatted.
                    const bool bAnchoredAtMaster( !pAnchPosAnchorFrm->IsFollow() );

                    // #i56300#
                    // perform complete format of anchor text frame and its
                    // previous frames, which have become invalid due to the
                    // fly frame format.
                    SwObjectFormatterTxtFrm::FormatAnchorFrmAndItsPrevs( *pAnchPosAnchorFrm );
                    // #i35911#
                    // #i40444#
                    // #i58182# - usage of new method
                    // <SwObjectFormatterTxtFrm::CheckMovedFwdCondition(..)>
                    sal_uInt32 nToPageNum( 0L );
                    bool bDummy( false );
                    if ( SwObjectFormatterTxtFrm::CheckMovedFwdCondition(
                                        *this, GetPageFrm()->GetPhyPageNum(),
                                        bAnchoredAtMaster, nToPageNum, bDummy ) )
                    {
                        bConsiderWrapInfluenceDueToMovedFwdAnchor = true;
                        // mark anchor text frame
                        // directly, that it is moved forward by object positioning.
                        SwTxtFrm* pAnchorTxtFrm( static_cast<SwTxtFrm*>(AnchorFrm()) );
                        bool bInsert( true );
                        sal_uInt32 nAnchorFrmToPageNum( 0L );
                        const SwDoc& rDoc = *(GetFrmFmt().GetDoc());
                        if ( SwLayouter::FrmMovedFwdByObjPos(
                                                rDoc, *pAnchorTxtFrm, nAnchorFrmToPageNum ) )
                        {
                            if ( nAnchorFrmToPageNum < nToPageNum )
                                SwLayouter::RemoveMovedFwdFrm( rDoc, *pAnchorTxtFrm );
                            else
                                bInsert = false;
                        }
                        if ( bInsert )
                        {
                            SwLayouter::InsertMovedFwdFrm( rDoc, *pAnchorTxtFrm,
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
                        SwFrmFmt* pFmt = GetFmt();
                        const SwFmtFrmSize& rFrmSz = pFmt->GetFrmSize();
                        if ( rFrmSz.GetWidthPercent() &&
                             rFrmSz.GetHeightPercent() == 0xFF )
                        {
                            SwFmtSurround aSurround( pFmt->GetSurround() );
                            if ( aSurround.GetSurround() == SURROUND_NONE )
                            {
                                pFmt->LockModify();
                                aSurround.SetSurround( SURROUND_THROUGHT );
                                pFmt->SetFmtAttr( aSurround );
                                pFmt->UnlockModify();
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
                      GetFmt()->GetDoc()->IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) );

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
            bSetCompletePaintOnInvalidate = sal_False;
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

/*************************************************************************
|*
|*  FindAnchor() und Hilfsfunktionen.
|*
|*  Description:        Searches an anchor for paragraph bound objects
|*      starting from pOldAnch. This is used to show anchors as well as changing
|*      anchors when dragging paragraph bound objects.
|*
|*************************************************************************/

class SwDistance
{
public:
    SwTwips nMain, nSub;
    SwDistance() { nMain = nSub = 0; }
    SwDistance& operator=( const SwDistance &rTwo )
        { nMain = rTwo.nMain; nSub = rTwo.nSub; return *this; }
    sal_Bool operator<( const SwDistance& rTwo ) const
        { return nMain < rTwo.nMain || ( nMain == rTwo.nMain && nSub &&
          rTwo.nSub && nSub < rTwo.nSub ); }
    sal_Bool operator<=( const SwDistance& rTwo ) const
        { return nMain < rTwo.nMain || ( nMain == rTwo.nMain && ( !nSub ||
          !rTwo.nSub || nSub <= rTwo.nSub ) ); }
};

static const SwFrm * lcl_CalcDownDist( SwDistance &rRet,
                                         const Point &rPt,
                                         const SwCntntFrm *pCnt )
{
    rRet.nSub = 0;
    //If the point stays inside the Cnt everything is clear already; the Cntnt
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
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        const bool bVertL2R = pUp->IsVertLR();

        //Follow the text flow.
        // #i70582#
        // --> OD 2009-03-05 - adopted for Support for Classical Mongolian Script
        const SwTwips nTopForObjPos =
            bVert
            ? ( bVertL2R
                ? ( pCnt->Frm().Left() +
                    pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() )
                : ( pCnt->Frm().Left() +
                    pCnt->Frm().Width() -
                    pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() ) )
            : ( pCnt->Frm().Top() +
                pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() );
        if ( pUp->Frm().IsInside( rPt ) )
        {
            // <rPt> point is inside environment of given content frame
            // #i70582#
            if( bVert )
            //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
            const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, sal_False, pCnt );
            if( !pLay ||
                (bVert && (pLay->Frm().Top() + pLay->Prt().Bottom()) <rPt.Y())||
                (!bVert && (pLay->Frm().Left() + pLay->Prt().Right())<rPt.X()) )
            {
                // <rPt> point is in left border of environment
                // #i70582#
                if( bVert )
                //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
            // Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
            rRet.nMain = bVert
                ? ( bVertL2R
                    ? ( (pUp->Frm().Left() + pUp->Prt().Right()) - nTopForObjPos )
                    : ( nTopForObjPos - (pUp->Frm().Left() + pUp->Prt().Left() ) ) )
                : ( (pUp->Frm().Top() + pUp->Prt().Bottom()) - nTopForObjPos );

            const SwFrm *pPre = pCnt;
            const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, sal_True, pCnt );
            SwTwips nFrmTop = 0;
            SwTwips nPrtHeight = 0;
            sal_Bool bSct = sal_False;
            const SwSectionFrm *pSect = pUp->FindSctFrm();
            if( pSect )
            {
                rRet.nSub = rRet.nMain;
                rRet.nMain = 0;
            }
            if( pSect && !pSect->IsAnLower( pLay ) )
            {
                bSct = sal_False;
                const SwSectionFrm* pNxtSect = pLay ? pLay->FindSctFrm() : 0;
                if( pSect->IsAnFollow( pNxtSect ) )
                {
                    if( pLay->IsVertical() )
                    {
                        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
                        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
                    pSect = 0;
                }
            }
            else if( pLay )
            {
                if( pLay->IsVertical() )
                {
                    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
                bSct = 0 != pSect;
            }
            while ( pLay && !pLay->Frm().IsInside( rPt ) &&
                    ( pLay->Frm().Top() <= rPt.Y() || pLay->IsInFly() ||
                      ( pLay->IsInSct() &&
                      pLay->FindSctFrm()->GetUpper()->Frm().Top() <= rPt.Y())) )
            {
                if ( pLay->IsFtnContFrm() )
                {
                    if ( !((SwLayoutFrm*)pLay)->Lower() )
                    {
                        SwFrm *pDel = (SwFrm*)pLay;
                        pDel->Cut();
                        delete pDel;
                        return pPre;
                    }
                    return 0;
                }
                else
                {
                    if( bSct || pSect )
                        rRet.nSub += nPrtHeight;
                    else
                        rRet.nMain += nPrtHeight;
                    pPre = pLay;
                    pLay = pLay->GetLeaf( MAKEPAGE_NONE, sal_True, pCnt );
                    if( pSect && !pSect->IsAnLower( pLay ) )
                    {   // If we're leaving a SwSectionFrm, the next Leaf-Frm
                        // is the part of the upper below the SectionFrm.
                        const SwSectionFrm* pNxtSect = pLay ?
                            pLay->FindSctFrm() : NULL;
                        bSct = sal_False;
                        if( pSect->IsAnFollow( pNxtSect ) )
                        {
                            pSect = pNxtSect;
                            if( pLay->IsVertical() )
                            {
                                //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
                                //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
                            pSect = 0;
                        }
                    }
                    else if( pLay )
                    {
                        if( pLay->IsVertical() )
                        {
                             //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
                        bSct = 0 != pSect;
                    }
                }
            }
            if ( pLay )
            {
                if ( pLay->Frm().IsInside( rPt ) )
                {
                    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
                    SwTwips nDiff = pLay->IsVertical() ? ( pLay->IsVertLR() ? ( rPt.X() - nFrmTop ) : ( nFrmTop - rPt.X() ) )
                                                       : ( rPt.Y() - nFrmTop );
                    if( bSct || pSect )
                        rRet.nSub += nDiff;
                    else
                        rRet.nMain += nDiff;
                }
                if ( pLay->IsFtnContFrm() && !((SwLayoutFrm*)pLay)->Lower() )
                {
                    SwFrm *pDel = (SwFrm*)pLay;
                    pDel->Cut();
                    delete pDel;
                    return 0;
                }
                return pLay;
            }
            else
                rRet.nMain = LONG_MAX;
        }
    }
    return 0;
}

static sal_uLong lcl_FindCntDiff( const Point &rPt, const SwLayoutFrm *pLay,
                          const SwCntntFrm *& rpCnt,
                          const sal_Bool bBody, const sal_Bool bFtn )
{
    // Searches below pLay the nearest Cnt to the point. The reference point of
    //the Cntnts is always the left upper corner.
    //The Cnt should preferably be above the point.

#if OSL_DEBUG_LEVEL > 1
    Point arPoint( rPt );
#endif

    rpCnt = 0;
    sal_uLong nDistance = ULONG_MAX;
    sal_uLong nNearest  = ULONG_MAX;
    const SwCntntFrm *pCnt = pLay->ContainsCntnt();

    while ( pCnt && (bBody != pCnt->IsInDocBody() || bFtn != pCnt->IsInFtn()))
    {
        pCnt = pCnt->GetNextCntntFrm();
        if ( !pLay->IsAnLower( pCnt ) )
            pCnt = 0;
    }
    const SwCntntFrm *pNearest = pCnt;
    if ( pCnt )
    {
        do
        {
            //Calculate the distance between those two points.
            //'delta' X^2 + 'delta' Y^2 = 'distance'^2
            sal_uInt32 dX = Max( pCnt->Frm().Left(), rPt.X() ) -
                       Min( pCnt->Frm().Left(), rPt.X() ),
                  dY = Max( pCnt->Frm().Top(), rPt.Y() ) -
                       Min( pCnt->Frm().Top(), rPt.Y() );
            BigInt dX1( dX ), dY1( dY );
            dX1 *= dX1; dY1 *= dY1;
            const sal_uLong nDiff = ::SqRt( dX1 + dY1 );
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
            pCnt = pCnt->GetNextCntntFrm();
            while ( pCnt &&
                    (bBody != pCnt->IsInDocBody() || bFtn != pCnt->IsInFtn()))
                pCnt = pCnt->GetNextCntntFrm();

        }  while ( pCnt && pLay->IsAnLower( pCnt ) );
    }
    if ( nDistance == ULONG_MAX )
    {   rpCnt = pNearest;
        return nNearest;
    }
    return nDistance;
}

static const SwCntntFrm * lcl_FindCnt( const Point &rPt, const SwCntntFrm *pCnt,
                                  const sal_Bool bBody, const sal_Bool bFtn )
{
    //Starting from pCnt searches the CntntFrm whose left upper corner is the
    //nearest to the point.
    //Always returns a CntntFrm.

    //First the nearest Cntnt inside the page which contains the Cntnt is
    //searched. Starting from this page the pages in both directions need to
    //be considered. If possible a Cntnt is returned whose Y-position is
    //above the point.
    const SwCntntFrm  *pRet, *pNew;
    const SwLayoutFrm *pLay = pCnt->FindPageFrm();
    sal_uLong nDist;

    nDist = ::lcl_FindCntDiff( rPt, pLay, pNew, bBody, bFtn );
    if ( pNew )
        pRet = pNew;
    else
    {   pRet  = pCnt;
        nDist = ULONG_MAX;
    }
    const SwCntntFrm *pNearest = pRet;
    sal_uLong nNearest = nDist;

    if ( pLay )
    {
        const SwLayoutFrm *pPge = pLay;
        sal_uLong nOldNew = ULONG_MAX;
        for ( sal_uInt16 i = 0; pPge->GetPrev() && (i < 3); ++i )
        {
            pPge = (SwLayoutFrm*)pPge->GetPrev();
            const sal_uLong nNew = ::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFtn );
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
            else if ( nOldNew != ULONG_MAX && nNew > nOldNew )
                break;
            else
                nOldNew = nNew;

        }
        pPge = pLay;
        nOldNew = ULONG_MAX;
        for ( sal_uInt16 j = 0; pPge->GetNext() && (j < 3); ++j )
        {
            pPge = (SwLayoutFrm*)pPge->GetNext();
            const sal_uLong nNew = ::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFtn );
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
            else if ( nOldNew != ULONG_MAX && nNew > nOldNew )
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
    if ( rPoint.X() < aTmp.Left() )
        rPoint.X() = aTmp.Left();
    else if ( rPoint.X() > aTmp.Right() )
        rPoint.X() = aTmp.Right();
    if ( rPoint.Y() < aTmp.Top() )
        rPoint.Y() = aTmp.Top();
    else if ( rPoint.Y() > aTmp.Bottom() )
        rPoint.Y() = aTmp.Bottom();

}

const SwCntntFrm *FindAnchor( const SwFrm *pOldAnch, const Point &rNew,
                              const sal_Bool bBodyOnly )
{
    //Search the nearest Cnt around the given document position in the text
    //flow. The given anchor is the starting Frm.
    const SwCntntFrm* pCnt;
    if ( pOldAnch->IsCntntFrm() )
    {
        pCnt = (const SwCntntFrm*)pOldAnch;
    }
    else
    {
        Point aTmp( rNew );
        SwLayoutFrm *pTmpLay = (SwLayoutFrm*)pOldAnch;
        if( pTmpLay->IsRootFrm() )
        {
            SwRect aTmpRect( aTmp, Size(0,0) );
            pTmpLay = (SwLayoutFrm*)::FindPage( aTmpRect, pTmpLay->Lower() );
        }
        pCnt = pTmpLay->GetCntntPos( aTmp, sal_False, bBodyOnly );
    }

    //Take care to use meaningful ranges during search. This means to not enter
    //or leave header/footer in this case.
    const sal_Bool bBody = pCnt->IsInDocBody() || bBodyOnly;
    const sal_Bool bFtn  = !bBodyOnly && pCnt->IsInFtn();

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
        const SwCntntFrm *pTmp = pCnt->FindPageFrm()->
                                        GetCntntPos( aTmp, sal_False, sal_True, sal_False );
        if ( pTmp && pTmp->Frm().IsInside( aNew ) )
            return pTmp;
    }

    //Starting from the anchor we now search in both directions until we found
    //the nearest one respectively.
    //Not the direct distance is relevant but the distance which needs to be
    //traveled through the text flow.
    const SwCntntFrm *pUpLst;
    const SwCntntFrm *pUpFrm = pCnt;
    SwDistance nUp, nUpLst;
    ::lcl_CalcDownDist( nUp, aNew, pUpFrm );
    SwDistance nDown = nUp;
    sal_Bool bNegAllowed = sal_True;// Make it possible to leave the negative section once.
    do
    {
        pUpLst = pUpFrm; nUpLst = nUp;
        pUpFrm = pUpLst->GetPrevCntntFrm();
        while ( pUpFrm &&
                (bBody != pUpFrm->IsInDocBody() || bFtn != pUpFrm->IsInFtn()))
            pUpFrm = pUpFrm->GetPrevCntntFrm();
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
                    pUpFrm = pUpFrm->GetPrevCntntFrm();
                    if ( pUpFrm )
                        ::lcl_CalcDownDist( nUp, aNew, pUpFrm );
                }
            }
        }
        if ( !pUpFrm )
            nUp.nMain = LONG_MAX;
        if ( nUp.nMain >= 0 && LONG_MAX != nUp.nMain )
        {
            bNegAllowed = sal_False;
            if ( nUpLst.nMain < 0 ) //don't take the wrong one, if the value
                                    //just changed from negative to positive.
            {   pUpLst = pUpFrm;
                nUpLst = nUp;
            }
        }
    } while ( pUpFrm && ( ( bNegAllowed && nUp.nMain < 0 ) || ( nUp <= nUpLst ) ) );

    const SwCntntFrm *pDownLst;
    const SwCntntFrm *pDownFrm = pCnt;
    SwDistance nDownLst;
    if ( nDown.nMain < 0 )
        nDown.nMain = LONG_MAX;
    do
    {
        pDownLst = pDownFrm; nDownLst = nDown;
        pDownFrm = pDownLst->GetNextCntntFrm();
        while ( pDownFrm &&
                (bBody != pDownFrm->IsInDocBody() || bFtn != pDownFrm->IsInFtn()))
            pDownFrm = pDownFrm->GetNextCntntFrm();
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
                    pDownFrm = pDownFrm->GetNextCntntFrm();
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

    //If we couldn't find one in both directions, we'll search the Cntnt whose
    //left upper corner is the nearest to the point. Such a situation may
    //happen, if the point doesn't lay in the text flow but in any margin.
    if ( nDownLst.nMain == LONG_MAX && nUpLst.nMain == LONG_MAX )
    {
        // If an OLE objects, which is contained in a fly frame
        // is resized in inplace mode and the new Position is outside the
        // fly frame, we do not want to leave our fly frame.
        if ( pCnt->IsInFly() )
            return pCnt;

        return ::lcl_FindCnt( aNew, pCnt, bBody, bFtn );
    }
    else
        return nDownLst < nUpLst ? pDownLst : pUpLst;
}

/*************************************************************************
|*
|*  SwFlyAtCntFrm::SetAbsPos()
|*
|*************************************************************************/

void SwFlyAtCntFrm::SetAbsPos( const Point &rNew )
{
    SwPageFrm *pOldPage = FindPageFrm();
    const SwRect aOld( GetObjRectWithSpaces() );
    Point aNew( rNew );
    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
      if( ( GetAnchorFrm()->IsVertical() && !GetAnchorFrm()->IsVertLR() ) || GetAnchorFrm()->IsRightToLeft() )
        aNew.X() += Frm().Width();
    SwCntntFrm *pCnt = (SwCntntFrm*)::FindAnchor( GetAnchorFrm(), aNew );
    if( pCnt->IsProtected() )
        pCnt = (SwCntntFrm*)GetAnchorFrm();

    SwPageFrm *pTmpPage = 0;
    const bool bVert = pCnt->IsVertical();
    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    const bool bVertL2R = pCnt->IsVertLR();
    const sal_Bool bRTL = pCnt->IsRightToLeft();

    if( ( !bVert != !GetAnchorFrm()->IsVertical() ) ||
        ( !bRTL !=  !GetAnchorFrm()->IsRightToLeft() ) )
    {
        if( bVert || bRTL )
            aNew.X() += Frm().Width();
        else
            aNew.X() -= Frm().Width();
    }

    if ( pCnt->IsInDocBody() )
    {
        //#38848 drag from page margin into the body.
        pTmpPage = pCnt->FindPageFrm();
        ::lcl_PointToPrt( aNew, pTmpPage->GetUpper() );
        SwRect aTmp( aNew, Size( 0, 0 ) );
        pTmpPage = (SwPageFrm*)::FindPage( aTmp, pTmpPage );
        ::lcl_PointToPrt( aNew, pTmpPage );
    }

    //Setup RelPos, only invalidate if requested.
    //rNew is an absolute position. We need to calculate the distance from rNew
    //to the anchor inside the text flow to correctly set RelPos.
//!!!!!We can optimize here: FindAnchor could also return RelPos!
    const SwFrm *pFrm = 0;
    SwTwips nY;
    if ( pCnt->Frm().IsInside( aNew ) )
    {
        // #i70582#
        const SwTwips nTopForObjPos =
                bVert
                ? ( bVertL2R
                    ? ( pCnt->Frm().Left() +
                        pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() )
                    : ( pCnt->Frm().Left() +
                        pCnt->Frm().Width() -
                        pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() ) )
                : ( pCnt->Frm().Top() +
                    pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() );
        if( bVert )
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        {
            if ( bVertL2R )
                nY = rNew.X() - nTopForObjPos;
            else
                nY = nTopForObjPos - rNew.X() - Frm().Width();
        }
        else
        {
            nY = rNew.Y() - nTopForObjPos;
        }
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
        const SwCntntFrm *pOriginal = pCnt;
        const SwCntntFrm *pFollow = pCnt;
        while ( pCnt->IsFollow() )
        {
            do
            {   pCnt = pCnt->GetPrevCntntFrm();
            } while ( pCnt->GetFollow() != pFollow );
            pFollow = pCnt;
        }
        SwTwips nDiff = 0;
        do
        {   const SwFrm *pUp = pFollow->GetUpper();
            if( pUp->IsVertical() )
            //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
            {
                if ( pUp->IsVertLR()  )
                    nDiff += pUp->Prt().Width() - pFollow->GetRelPos().X();
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
        const SwTwips nTopForObjPos =
                bVert
                ? ( bVertL2R
                    ? ( pCnt->Frm().Left() +
                        pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() )
                    : ( pCnt->Frm().Left() +
                        pCnt->Frm().Width() -
                        pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() ) )
                : ( pCnt->Frm().Top() +
                    pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() );
        if( bVert )
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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

    SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
    const SwFmtSurround& rSurround = pFmt->GetSurround();
    const sal_Bool bWrapThrough =
        rSurround.GetSurround() == SURROUND_THROUGHT;
    SwTwips nBaseOfstForFly = 0;
    const SwFrm* pTmpFrm = pFrm ? pFrm : pCnt;
    if ( pTmpFrm->IsTxtFrm() )
        nBaseOfstForFly =
            ((SwTxtFrm*)pTmpFrm)->GetBaseOfstForFly( !bWrapThrough );

    if( bVert )
    {
        if( !pFrm )
            nX += rNew.Y() - pCnt->Frm().Top() - nBaseOfstForFly;
        else
            nX = rNew.Y() - pFrm->Frm().Top() - nBaseOfstForFly;
    }
    else
    {
        if( !pFrm )
        {
            if ( pCnt->IsRightToLeft() )
                nX += pCnt->Frm().Right() - rNew.X() - Frm().Width() +
                      nBaseOfstForFly;
            else
                nX += rNew.X() - pCnt->Frm().Left() - nBaseOfstForFly;
        }
        else
        {
            if ( pFrm->IsRightToLeft() )
                nX += pFrm->Frm().Right() - rNew.X() - Frm().Width() +
                      nBaseOfstForFly;
            else
                nX = rNew.X() - pFrm->Frm().Left() - nBaseOfstForFly;
        }
    }
    GetFmt()->GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );

    if( pCnt != GetAnchorFrm() || ( IsAutoPos() && pCnt->IsTxtFrm() &&
                                  GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::HTML_MODE)) )
    {
        //Set the anchor attribute according to the new Cnt.
        SwFmtAnchor aAnch( pFmt->GetAnchor() );
        SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
        if( IsAutoPos() && pCnt->IsTxtFrm() )
        {
            SwCrsrMoveState eTmpState( MV_SETONLYTEXT );
            Point aPt( rNew );
            if( pCnt->GetCrsrOfst( pPos, aPt, &eTmpState )
                && pPos->nNode == *pCnt->GetNode() )
            {
                ResetLastCharRectHeight();
                if( text::RelOrientation::CHAR == pFmt->GetVertOrient().GetRelationOrient() )
                    nY = LONG_MAX;
                if( text::RelOrientation::CHAR == pFmt->GetHoriOrient().GetRelationOrient() )
                    nX = LONG_MAX;
            }
            else
            {
                pPos->nNode = *pCnt->GetNode();
                pPos->nContent.Assign( pCnt->GetNode(), 0 );
            }
        }
        else
        {
            pPos->nNode = *pCnt->GetNode();
            pPos->nContent.Assign( pCnt->GetNode(), 0 );
        }

        // handle change of anchor node:
        // if count of the anchor frame also change, the fly frames have to be
        // re-created. Thus, delete all fly frames except the <this> before the
        // anchor attribute is change and re-create them afterwards.
        {
            SwHandleAnchorNodeChg aHandleAnchorNodeChg( *pFmt, aAnch, this );
            pFmt->GetDoc()->SetAttr( aAnch, *pFmt );
        }
    }
    // #i28701# - use new method <GetPageFrm()>
    else if ( pTmpPage && pTmpPage != GetPageFrm() )
        GetPageFrm()->MoveFly( this, pTmpPage );

    const Point aRelPos = bVert ? Point( -nY, nX ) : Point( nX, nY );

    ChgRelPos( aRelPos );

    GetFmt()->GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );

    if ( pOldPage != FindPageFrm() )
        ::Notify_Background( GetVirtDrawObj(), pOldPage, aOld, PREP_FLY_LEAVE,
                             sal_False );
}

/** method to assure that anchored object is registered at the correct
    page frame

    #i28701#
    takes over functionality of deleted method <SwFlyAtCntFrm::AssertPage()>
*/
void SwFlyAtCntFrm::RegisterAtCorrectPage()
{
    SwPageFrm* pPageFrm( 0L );
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
//void SwFlyAtCntFrm::MakeFlyPos()
void SwFlyAtCntFrm::MakeObjPos()
{
    // if fly frame position is valid, nothing is to do. Thus, return
    if ( bValidPos )
    {
        return;
    }

    // #i26791# - validate position flag here.
    bValidPos = sal_True;

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
    objectpositioning::SwToCntntAnchoredObjectPosition
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
