/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tools/bigint.hxx>
#include "pagefrm.hxx"
#include "txtfrm.hxx"
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
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

SwFlyAtCntFrm::SwFlyAtCntFrm( SwFlyFrmFmt *pFmt, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFreeFrm( pFmt, pSib, pAnch )
{
    bAtCnt = sal_True;
    bAutoPosition = (FLY_AT_CHAR == pFmt->GetAnchor().GetAnchorId());
}


TYPEINIT1(SwFlyAtCntFrm,SwFlyFreeFrm);

void SwFlyAtCntFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    sal_uInt16 nWhich = pNew ? pNew->Which() : 0;
    const SwFmtAnchor *pAnch = 0;

    if( RES_ATTRSET_CHG == nWhich && SFX_ITEM_SET ==
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_ANCHOR, false,
            (const SfxPoolItem**)&pAnch ))
        ;       

    else if( RES_ANCHOR == nWhich )
    {
        
        
        
        pAnch = (const SwFmtAnchor*)pNew;
    }

    if( pAnch )
    {
        OSL_ENSURE( pAnch->GetAnchorId() == GetFmt()->GetAnchor().GetAnchorId(),
                "Illegal change of anchor type. " );

        
        SwRect aOld( GetObjRectWithSpaces() );
        SwPageFrm *pOldPage = FindPageFrm();
        const SwFrm *pOldAnchor = GetAnchorFrm();
        SwCntntFrm *pCntnt = (SwCntntFrm*)GetAnchorFrm();
        AnchorFrm()->RemoveFly( this );

        const bool bBodyFtn = (pCntnt->IsInDocBody() || pCntnt->IsInFtn());

        
        
        const SwNodeIndex aNewIdx( pAnch->GetCntntAnchor()->nNode );
        SwNodeIndex aOldIdx( *pCntnt->GetNode() );

        
        
        
        
        
        
        const bool bNext = aOldIdx < aNewIdx;
        
        
        
        
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
        
        
        SwCntntFrm* pFlow = pCntnt;
        while ( pFlow->IsFollow() )
            pFlow = pFlow->FindMaster();
        pCntnt = pFlow;

        
        pCntnt->AppendFly( this );
        if ( pOldPage && pOldPage != FindPageFrm() )
            NotifyBackground( pOldPage, aOld, PREP_FLY_LEAVE );

        
        _InvalidatePos();
        InvalidatePage();
        SetNotifyBack();
        
        
        ClearCharRectAndTopOfLine();
    }
    else
        SwFlyFrm::Modify( pOld, pNew );
}





class SwOszControl
{
    static const SwFlyFrm *pStk1;
    static const SwFlyFrm *pStk2;
    static const SwFlyFrm *pStk3;
    static const SwFlyFrm *pStk4;
    static const SwFlyFrm *pStk5;

    const SwFlyFrm *pFly;
    
    sal_uInt8 mnPosStackSize;
    std::vector<Point*> maObjPositions;

public:
    SwOszControl( const SwFlyFrm *pFrm );
    ~SwOszControl();
    bool ChkOsz();
    static bool IsInProgress( const SwFlyFrm *pFly );
};

const SwFlyFrm *SwOszControl::pStk1 = 0;
const SwFlyFrm *SwOszControl::pStk2 = 0;
const SwFlyFrm *SwOszControl::pStk3 = 0;
const SwFlyFrm *SwOszControl::pStk4 = 0;
const SwFlyFrm *SwOszControl::pStk5 = 0;

SwOszControl::SwOszControl( const SwFlyFrm *pFrm )
    : pFly( pFrm ),
      
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
|*/
void SwFlyAtCntFrm::MakeAll()
{
    if ( !GetFmt()->GetDoc()->IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !SwOszControl::IsInProgress( this ) && !IsLocked() && !IsColLocked() )
    {
        
        if( !GetPageFrm() && GetAnchorFrm() && GetAnchorFrm()->IsInFly() )
        {
            SwFlyFrm* pFly = AnchorFrm()->FindFlyFrm();
            SwPageFrm *pTmpPage = pFly ? pFly->FindPageFrm() : NULL;
            if( pTmpPage )
                pTmpPage->AppendFlyToPage( this );
        }
        
        if( GetPageFrm() )
        {
            bSetCompletePaintOnInvalidate = true;
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

            
            
            
            
            const bool bFormatAnchor =
                    !static_cast<const SwTxtFrm*>( GetAnchorFrmContainingAnchPos() )->IsAnyJoinLocked() &&
                    !ConsiderObjWrapInfluenceOnObjPos() &&
                    !ConsiderObjWrapInfluenceOfOtherObjs();

            const SwFrm* pFooter = GetAnchorFrm()->FindFooterOrHeader();
            if( pFooter && !pFooter->IsFooterFrm() )
                pFooter = NULL;
            bool bOsz = false;
            bool bExtra = Lower() && Lower()->IsColumnFrm();
            
            
            
            bool bConsiderWrapInfluenceDueToOverlapPrevCol( false );
            
            
            
            
            
            
            
            
            bool bConsiderWrapInfluenceDueToMovedFwdAnchor( false );
            do {
                SWRECTFN( this )
                Point aOldPos( (Frm().*fnRect->fnGetPos)() );
                SwFlyFreeFrm::MakeAll();
                const bool bPosChgDueToOwnFormat =
                                        aOldPos != (Frm().*fnRect->fnGetPos)();
                
                if ( !ConsiderObjWrapInfluenceOnObjPos() &&
                     OverlapsPrevColumn() )
                {
                    bConsiderWrapInfluenceDueToOverlapPrevCol = true;
                }
                
                
                if ( bFormatAnchor )
                {
                    SwTxtFrm* pAnchPosAnchorFrm =
                            dynamic_cast<SwTxtFrm*>(GetAnchorFrmContainingAnchPos());
                    OSL_ENSURE( pAnchPosAnchorFrm,
                            "<SwFlyAtCntFrm::MakeAll()> - anchor frame of wrong type -> crash" );
                    
                    
                    
                    
                    
                    const bool bAnchoredAtMaster( !pAnchPosAnchorFrm->IsFollow() );

                    
                    
                    
                    
                    SwObjectFormatterTxtFrm::FormatAnchorFrmAndItsPrevs( *pAnchPosAnchorFrm );
                    
                    
                    
                    
                    sal_uInt32 nToPageNum( 0L );
                    bool bDummy( false );
                    if ( SwObjectFormatterTxtFrm::CheckMovedFwdCondition(
                                        *this, GetPageFrm()->GetPhyPageNum(),
                                        bAnchoredAtMaster, nToPageNum, bDummy ) )
                    {
                        bConsiderWrapInfluenceDueToMovedFwdAnchor = true;
                        
                        
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
                    
                    
                    
                    _InvalidateSize();
                    bExtra = false; 
                }
            } while ( !IsValid() && !bOsz &&
                      
                      !bConsiderWrapInfluenceDueToOverlapPrevCol &&
                      
                      !bConsiderWrapInfluenceDueToMovedFwdAnchor &&
                      GetFmt()->GetDoc()->IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) );

            
            
            
            
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
                                         const SwCntntFrm *pCnt )
{
    rRet.nSub = 0;
    
    
    if ( pCnt->Frm().IsInside( rPt ) )
    {
        rRet.nMain = 0;
        return pCnt;
    }
    else
    {
        const SwLayoutFrm *pUp = pCnt->IsInTab() ? pCnt->FindTabFrm()->GetUpper() : pCnt->GetUpper();
        
        while( pUp->IsSctFrm() )
            pUp = pUp->GetUpper();
        const bool bVert = pUp->IsVertical();
        
        const bool bVertL2R = pUp->IsVertLR();

        
        
        
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
            
            
            rRet.nMain = LONG_MAX;
        }
        else if( rPt.X() < pUp->Frm().Left() &&
                 rPt.Y() <= ( bVert ? pUp->Frm().Top() : pUp->Frm().Bottom() ) )
        {
            
            
            const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, sal_False, pCnt );
            if( !pLay ||
                (bVert && (pLay->Frm().Top() + pLay->Prt().Bottom()) <rPt.Y())||
                (!bVert && (pLay->Frm().Left() + pLay->Prt().Right())<rPt.X()) )
            {
                
                
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
            const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, sal_True, pCnt );
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
                const SwSectionFrm* pNxtSect = pLay ? pLay->FindSctFrm() : 0;
                if( pSect->IsAnFollow( pNxtSect ) )
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
                    pSect = 0;
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
                    {   
                        
                        const SwSectionFrm* pNxtSect = pLay ?
                            pLay->FindSctFrm() : NULL;
                        bSct = false;
                        if( pSect->IsAnFollow( pNxtSect ) )
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
                            pSect = 0;
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
                        bSct = 0 != pSect;
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

static sal_uInt64 lcl_FindCntDiff( const Point &rPt, const SwLayoutFrm *pLay,
                          const SwCntntFrm *& rpCnt,
                          const bool bBody, const sal_Bool bFtn )
{
    
    
    

#if OSL_DEBUG_LEVEL > 1
    Point arPoint( rPt );
#endif

    rpCnt = 0;
    sal_uInt64 nDistance = SAL_MAX_UINT64;
    sal_uInt64 nNearest  = SAL_MAX_UINT64;
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
            
            
            sal_uInt64 dX = std::max( pCnt->Frm().Left(), rPt.X() ) -
                       std::min( pCnt->Frm().Left(), rPt.X() ),
                  dY = std::max( pCnt->Frm().Top(), rPt.Y() ) -
                       std::min( pCnt->Frm().Top(), rPt.Y() );
            
            const sal_uInt64 nDiff = (dX * dX) + (dY * dY);
            if ( pCnt->Frm().Top() <= rPt.Y() )
            {
                if ( nDiff < nDistance )
                {
                    
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
    if (nDistance == SAL_MAX_UINT64)
    {   rpCnt = pNearest;
        return nNearest;
    }
    return nDistance;
}

static const SwCntntFrm * lcl_FindCnt( const Point &rPt, const SwCntntFrm *pCnt,
                                  const bool bBody, const sal_Bool bFtn )
{
    
    
    

    
    
    
    
    const SwCntntFrm  *pRet, *pNew;
    const SwLayoutFrm *pLay = pCnt->FindPageFrm();
    sal_uInt64 nDist; 

    nDist = ::lcl_FindCntDiff( rPt, pLay, pNew, bBody, bFtn );
    if ( pNew )
        pRet = pNew;
    else
    {   pRet  = pCnt;
        nDist = SAL_MAX_UINT64;
    }
    const SwCntntFrm *pNearest = pRet;
    sal_uInt64 nNearest = nDist;

    if ( pLay )
    {
        const SwLayoutFrm *pPge = pLay;
        sal_uInt64 nOldNew = SAL_MAX_UINT64;
        for ( sal_uInt16 i = 0; pPge->GetPrev() && (i < 3); ++i )
        {
            pPge = (SwLayoutFrm*)pPge->GetPrev();
            const sal_uInt64 nNew = ::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFtn );
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
        for ( sal_uInt16 j = 0; pPge->GetNext() && (j < 3); ++j )
        {
            pPge = (SwLayoutFrm*)pPge->GetNext();
            const sal_uInt64 nNew = ::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFtn );
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
const SwCntntFrm *FindAnchor( const SwFrm *pOldAnch, const Point &rNew,
                              const sal_Bool bBodyOnly )
{
    
    
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

    
    
    const bool bBody = pCnt->IsInDocBody() || bBodyOnly;
    const sal_Bool bFtn  = !bBodyOnly && pCnt->IsInFtn();

    Point aNew( rNew );
    if ( bBody )
    {
        
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
        
        
        Point aTmp( aNew );
        const SwCntntFrm *pTmp = pCnt->FindPageFrm()->
                                        GetCntntPos( aTmp, sal_False, sal_True, sal_False );
        if ( pTmp && pTmp->Frm().IsInside( aNew ) )
            return pTmp;
    }

    
    
    
    
    const SwCntntFrm *pUpLst;
    const SwCntntFrm *pUpFrm = pCnt;
    SwDistance nUp, nUpLst;
    ::lcl_CalcDownDist( nUp, aNew, pUpFrm );
    SwDistance nDown = nUp;
    bool bNegAllowed = true;
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
            bNegAllowed = false;
            if ( nUpLst.nMain < 0 ) 
                                    
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

    
    
    
    if ( nDownLst.nMain == LONG_MAX && nUpLst.nMain == LONG_MAX )
    {
        
        
        
        if ( pCnt->IsInFly() )
            return pCnt;

        return ::lcl_FindCnt( aNew, pCnt, bBody, bFtn );
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
    SwCntntFrm *pCnt = (SwCntntFrm*)::FindAnchor( GetAnchorFrm(), aNew );
    if( pCnt->IsProtected() )
        pCnt = (SwCntntFrm*)GetAnchorFrm();

    SwPageFrm *pTmpPage = 0;
    const bool bVert = pCnt->IsVertical();
    
    const bool bVertL2R = pCnt->IsVertLR();
    const sal_Bool bRTL = pCnt->IsRightToLeft();

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
        
        pTmpPage = pCnt->FindPageFrm();
        ::lcl_PointToPrt( aNew, pTmpPage->GetUpper() );
        SwRect aTmp( aNew, Size( 0, 0 ) );
        pTmpPage = (SwPageFrm*)::FindPage( aTmp, pTmpPage );
        ::lcl_PointToPrt( aNew, pTmpPage );
    }

    
    
    

    const SwFrm *pFrm = 0;
    SwTwips nY;
    if ( pCnt->Frm().IsInside( aNew ) )
    {
        
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
        
        SwFmtAnchor aAnch( pFmt->GetAnchor() );
        SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
        if( IsAutoPos() && pCnt->IsTxtFrm() )
        {
            SwCrsrMoveState eTmpState( MV_SETONLYTEXT );
            Point aPt( rNew );
            if( pCnt->GetCrsrOfst( pPos, aPt, &eTmpState )
                && pPos->nNode == *pCnt->GetNode() )
            {
                if ( pCnt->GetNode()->GetTxtNode() != NULL )
                {
                    const SwTxtAttr* pTxtInputFld =
                        pCnt->GetNode()->GetTxtNode()->GetTxtAttrAt( pPos->nContent.GetIndex(), RES_TXTATR_INPUTFIELD, SwTxtNode::PARENT );
                    if ( pTxtInputFld != NULL )
                    {
                        pPos->nContent = *(pTxtInputFld->GetStart());
                    }
                }
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

        
        
        
        
        {
            SwHandleAnchorNodeChg aHandleAnchorNodeChg( *pFmt, aAnch, this );
            pFmt->GetDoc()->SetAttr( aAnch, *pFmt );
        }
    }
    else if ( pTmpPage && pTmpPage != GetPageFrm() )
        GetPageFrm()->MoveFly( this, pTmpPage );

    const Point aRelPos = bVert ? Point( -nY, nX ) : Point( nX, nY );

    ChgRelPos( aRelPos );

    GetFmt()->GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );

    if ( pOldPage != FindPageFrm() )
        ::Notify_Background( GetVirtDrawObj(), pOldPage, aOld, PREP_FLY_LEAVE, sal_False );
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


void SwFlyAtCntFrm::MakeObjPos()
{
    
    if ( mbValidPos )
    {
        return;
    }

    
    mbValidPos = sal_True;

    
    
    
    
    
    if ( GetVertPosOrientFrm() &&
         ClearedEnvironment() && HasClearedEnvironment() )
    {
        return;
    }

    
    objectpositioning::SwToCntntAnchoredObjectPosition
            aObjPositioning( *GetVirtDrawObj() );
    aObjPositioning.CalcPosition();

    SetVertPosOrientFrm ( aObjPositioning.GetVertPosOrientFrm() );
}


bool SwFlyAtCntFrm::_InvalidationAllowed( const InvalidationType _nInvalid ) const
{
    bool bAllowed( SwFlyFreeFrm::_InvalidationAllowed( _nInvalid ) );

    
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
