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

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lspcitem.hxx>

#include <fmtornt.hxx>
#include <fmthdft.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <docary.hxx>
#include <lineinfo.hxx>
#include <swmodule.hxx>
#include "pagefrm.hxx"
#include "colfrm.hxx"
#include "fesh.hxx"
#include "viewimp.hxx"
#include "viewopt.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "frmtool.hxx"
#include "tabfrm.hxx"
#include "rowfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "layact.hxx"
#include "pagedesc.hxx"
#include "section.hxx"
#include "sectfrm.hxx"
#include "node2lay.hxx"
#include "ndole.hxx"
#include "hints.hxx"
#include <layhelp.hxx>
#include <laycache.hxx>
#include <rootfrm.hxx>
#include <paratr.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <switerator.hxx>


void sw_RemoveFtns( SwFtnBossFrm* pBoss, sal_Bool bPageOnly, sal_Bool bEndNotes );

using namespace ::com::sun::star;

bool bObjsDirect = true;
bool bDontCreateObjects = false;
bool bSetCompletePaintOnInvalidate = false;

sal_uInt8 StackHack::nCnt = 0;
sal_Bool StackHack::bLocked = sal_False;

SwFrmNotify::SwFrmNotify( SwFrm *pF ) :
    pFrm( pF ),
    aFrm( pF->Frm() ),
    aPrt( pF->Prt() ),
    bInvaKeep( sal_False ),
    bValidSize( pF->GetValidSizeFlag() ),
    mbFrmDeleted( false )     
{
    if ( pF->IsTxtFrm() )
    {
        mnFlyAnchorOfst = ((SwTxtFrm*)pF)->GetBaseOfstForFly( true );
        mnFlyAnchorOfstNoWrap = ((SwTxtFrm*)pF)->GetBaseOfstForFly( false );
    }
    else
    {
        mnFlyAnchorOfst = 0;
        mnFlyAnchorOfstNoWrap = 0;
    }

    bHadFollow = pF->IsCntntFrm() ?
                    (((SwCntntFrm*)pF)->GetFollow() ? sal_True : sal_False) :
                    sal_False;
}

SwFrmNotify::~SwFrmNotify()
{
    
    if ( mbFrmDeleted )
    {
        return;
    }

    SWRECTFN( pFrm )
    const bool bAbsP = POS_DIFF( aFrm, pFrm->Frm() );
    const bool bChgWidth =
            (aFrm.*fnRect->fnGetWidth)() != (pFrm->Frm().*fnRect->fnGetWidth)();
    const bool bChgHeight =
            (aFrm.*fnRect->fnGetHeight)()!=(pFrm->Frm().*fnRect->fnGetHeight)();
    const bool bChgFlyBasePos = pFrm->IsTxtFrm() &&
       ( ( mnFlyAnchorOfst != ((SwTxtFrm*)pFrm)->GetBaseOfstForFly( true ) ) ||
         ( mnFlyAnchorOfstNoWrap != ((SwTxtFrm*)pFrm)->GetBaseOfstForFly( false ) ) );

    if ( pFrm->IsFlowFrm() && !pFrm->IsInFtn() )
    {
        SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( pFrm );

        if ( !pFlow->IsFollow() )
        {
            if ( !pFrm->GetIndPrev() )
            {
                if ( bInvaKeep )
                {
                    SwFrm *pPre = pFrm->FindPrev();
                    if ( pPre && pPre->IsFlowFrm() )
                    {
                        
                        bool bInvalidPrePos = SwFlowFrm::CastFlowFrm( pPre )->IsKeep( *pPre->GetAttrSet() ) && pPre->GetIndPrev();

                        
                        if ( !bInvalidPrePos && pPre->IsTabFrm() )
                        {
                            SwTabFrm* pPreTab = static_cast<SwTabFrm*>(pPre);
                            if ( pPreTab->GetFmt()->GetDoc()->get(IDocumentSettingAccess::TABLE_ROW_KEEP) )
                            {
                                SwRowFrm* pLastRow = static_cast<SwRowFrm*>(pPreTab->GetLastLower());
                                if ( pLastRow && pLastRow->ShouldRowKeepWithNext() )
                                    bInvalidPrePos = true;
                            }
                        }

                        if ( bInvalidPrePos )
                            pPre->InvalidatePos();
                    }
                }
            }
            else if ( !pFlow->HasFollow() )
            {
                long nOldHeight = (aFrm.*fnRect->fnGetHeight)();
                long nNewHeight = (pFrm->Frm().*fnRect->fnGetHeight)();
                if( (nOldHeight > nNewHeight) || (!nOldHeight && nNewHeight) )
                    pFlow->CheckKeep();
            }
        }
    }

    if ( bAbsP )
    {
        pFrm->SetCompletePaint();

        SwFrm* pNxt = pFrm->GetIndNext();
        
        while ( pNxt &&
                pNxt->IsSctFrm() && !static_cast<SwSectionFrm*>(pNxt)->GetSection() )
        {
            pNxt = pNxt->GetIndNext();
        }

        if ( pNxt )
            pNxt->InvalidatePos();
        else
        {
            
            
            if( pFrm->IsRetoucheFrm() &&
                (aFrm.*fnRect->fnTopDist)( (pFrm->Frm().*fnRect->fnGetTop)() ) > 0 )
            {
                pFrm->SetRetouche();
            }

            
            
            if ( bHadFollow || !pFrm->IsCntntFrm() || !((SwCntntFrm*)pFrm)->GetFollow() )
            {
                if ( !pFrm->IsTabFrm() || !((SwTabFrm*)pFrm)->GetFollow() )
                    pFrm->InvalidateNextPos();
            }
        }
    }

    
    const bool bPrtWidth =
            (aPrt.*fnRect->fnGetWidth)() != (pFrm->Prt().*fnRect->fnGetWidth)();
    const bool bPrtHeight =
            (aPrt.*fnRect->fnGetHeight)()!=(pFrm->Prt().*fnRect->fnGetHeight)();
    if ( bPrtWidth || bPrtHeight )
    {
        const SvxGraphicPosition ePos = pFrm->GetAttrSet()->GetBackground().GetGraphicPos();
        if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
            pFrm->SetCompletePaint();
    }
    else
    {
        
        
        
        if ( !bAbsP && (bChgWidth || bChgHeight) )
        {
            pFrm->SetCompletePaint();
        }
    }

    const bool bPrtP = POS_DIFF( aPrt, pFrm->Prt() );
    if ( bAbsP || bPrtP || bChgWidth || bChgHeight ||
         bPrtWidth || bPrtHeight || bChgFlyBasePos )
    {
        if( pFrm->IsAccessibleFrm() )
        {
            SwRootFrm *pRootFrm = pFrm->getRootFrm();
            if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
                pRootFrm->GetCurrShell() )
            {
                pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( pFrm, aFrm );
            }
        }

        
        if ( pFrm->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pFrm->GetDrawObjs();
            SwPageFrm* pPageFrm = 0;
            for ( sal_uInt32 i = 0; i < rObjs.Count(); ++i )
            {
                
                
                bool bNotify = false;
                bool bNotifySize = false;
                SwAnchoredObject* pObj = rObjs[i];
                SwContact* pContact = ::GetUserCall( pObj->GetDrawObj() );
                
                const bool bAnchoredAsChar = pContact->ObjAnchoredAsChar();
                if ( !bAnchoredAsChar )
                {
                    

                    
                    
                    if ( bAbsP ||
                         pContact->ObjAnchoredAtPage() ||
                         pContact->ObjAnchoredAtFly() )
                    {
                        bNotify = true;

                        
                        
                        
                        if ( bAbsP && pContact->ObjAnchoredAtFly() &&
                             pObj->ISA(SwFlyFrm) )
                        {
                            
                            SwFlyFrm* pFlyFrm = static_cast<SwFlyFrm*>(pObj);
                            
                            
                            SwPageFrm* pFlyPageFrm = pFlyFrm->FindPageFrm();
                            
                            if ( !pPageFrm )
                            {
                                pPageFrm = pFrm->FindPageFrm();
                            }
                            if ( pPageFrm != pFlyPageFrm )
                            {
                                OSL_ENSURE( pFlyPageFrm, "~SwFrmNotify: Fly from Nowhere" );
                                if( pFlyPageFrm )
                                    pFlyPageFrm->MoveFly( pFlyFrm, pPageFrm );
                                else
                                    pPageFrm->AppendFlyToPage( pFlyFrm );
                            }
                        }
                    }
                    
                    
                    else
                    {
                        const SwFmtVertOrient& rVert =
                                        pContact->GetFmt()->GetVertOrient();
                        if ( ( rVert.GetVertOrient() == text::VertOrientation::CENTER ||
                               rVert.GetVertOrient() == text::VertOrientation::BOTTOM ||
                               rVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA ) &&
                             ( bChgHeight || bPrtHeight ) )
                        {
                            bNotify = true;
                        }
                        if ( !bNotify )
                        {
                            const SwFmtHoriOrient& rHori =
                                        pContact->GetFmt()->GetHoriOrient();
                            if ( ( rHori.GetHoriOrient() != text::HoriOrientation::NONE ||
                                   rHori.GetRelationOrient()== text::RelOrientation::PRINT_AREA ||
                                   rHori.GetRelationOrient()== text::RelOrientation::FRAME ) &&
                                 ( bChgWidth || bPrtWidth || bChgFlyBasePos ) )
                            {
                                bNotify = true;
                            }
                        }
                    }
                }
                else if ( bPrtWidth )
                {
                    
                    
                    bNotify = true;
                    bNotifySize = true;
                }

                
                if ( bNotify )
                {
                    if ( pObj->ISA(SwFlyFrm) )
                    {
                        SwFlyFrm* pFlyFrm = static_cast<SwFlyFrm*>(pObj);
                        if ( bNotifySize )
                            pFlyFrm->_InvalidateSize();
                        
                        
                        if ( !bAnchoredAsChar )
                        {
                            pFlyFrm->_InvalidatePos();
                        }
                        pFlyFrm->_Invalidate();
                    }
                    else if ( pObj->ISA(SwAnchoredDrawObject) )
                    {
                        
                        
                        if ( !bAnchoredAsChar )
                        {
                            pObj->InvalidateObjPos();
                        }
                    }
                    else
                    {
                        OSL_FAIL( "<SwCntntNotify::~SwCntntNotify()> - unknown anchored object type. Please inform OD." );
                    }
                }
            }
        }
    }
    else if( pFrm->IsTxtFrm() && bValidSize != pFrm->GetValidSizeFlag() )
    {
        SwRootFrm *pRootFrm = pFrm->getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->InvalidateAccessibleFrmContent( pFrm );
        }
    }

    
    SwFlyFrm* pFly = 0;
    
    
    if ( !pFrm->IsFlyFrm() && 0 != ( pFly = pFrm->ImplFindFlyFrm() ) )
    {
        
        
        
        if ( pFly->Lower() && !pFly->Lower()->IsColumnFrm() )
        {
            const SwFmtFrmSize &rFrmSz = pFly->GetFmt()->GetFrmSize();

            
            
            
            
            
            
            
            if ( ATT_FIX_SIZE != rFrmSz.GetWidthSizeType() )
            {
                
                
                
                if ( !pFly->ISA(SwFlyInCntFrm) )
                {
                    pFly->InvalidatePos();
                }
                pFly->InvalidateSize();
            }
        }
    }
}

SwLayNotify::SwLayNotify( SwLayoutFrm *pLayFrm ) :
    SwFrmNotify( pLayFrm ),
    bLowersComplete( sal_False )
{
}




static void lcl_InvalidatePosOfLowers( SwLayoutFrm& _rLayoutFrm )
{
    if( _rLayoutFrm.IsFlyFrm() && _rLayoutFrm.GetDrawObjs() )
    {
        _rLayoutFrm.InvalidateObjs( true, false );
    }

    SwFrm* pLowerFrm = _rLayoutFrm.Lower();
    while ( pLowerFrm )
    {
        pLowerFrm->InvalidatePos();
        if ( pLowerFrm->IsTxtFrm() )
        {
            static_cast<SwTxtFrm*>(pLowerFrm)->Prepare( PREP_POS_CHGD );
        }
        else if ( pLowerFrm->IsTabFrm() )
        {
            pLowerFrm->InvalidatePrt();
        }

        pLowerFrm->InvalidateObjs( true, false );

        pLowerFrm = pLowerFrm->GetNext();
    };
}

SwLayNotify::~SwLayNotify()
{
    
    if ( mbFrmDeleted )
    {
        return;
    }

    SwLayoutFrm *pLay = GetLay();
    SWRECTFN( pLay )
    bool bNotify = false;
    if ( pLay->Prt().SSize() != aPrt.SSize() )
    {
        if ( !IsLowersComplete() )
        {
            bool bInvaPercent;

            if ( pLay->IsRowFrm() )
            {
                bInvaPercent = true;
                long nNew = (pLay->Prt().*fnRect->fnGetHeight)();
                if( nNew != (aPrt.*fnRect->fnGetHeight)() )
                     ((SwRowFrm*)pLay)->AdjustCells( nNew, sal_True);
                if( (pLay->Prt().*fnRect->fnGetWidth)()
                    != (aPrt.*fnRect->fnGetWidth)() )
                     ((SwRowFrm*)pLay)->AdjustCells( 0, sal_False );
            }
            else
            {
                
                
                
                
                
                
                
                bool bLow;
                if( pLay->IsFlyFrm() )
                {
                    if ( pLay->Lower() )
                    {
                        bLow = !pLay->Lower()->IsColumnFrm() ||
                            (pLay->Lower()->Frm().*fnRect->fnGetHeight)()
                             != (pLay->Prt().*fnRect->fnGetHeight)();
                    }
                    else
                        bLow = false;
                }
                else if( pLay->IsSctFrm() )
                {
                    if ( pLay->Lower() )
                    {
                        if( pLay->Lower()->IsColumnFrm() && pLay->Lower()->GetNext() )
                            bLow = pLay->Lower()->Frm().Height() != pLay->Prt().Height();
                        else
                            bLow = pLay->Prt().Width() != aPrt.Width();
                    }
                    else
                        bLow = false;
                }
                else if( pLay->IsFooterFrm() && !pLay->HasFixSize() )
                    bLow = pLay->Prt().Width() != aPrt.Width();
                else
                    bLow = true;
                bInvaPercent = bLow;
                if ( bLow )
                {
                    pLay->ChgLowersProp( aPrt.SSize() );
                }
                
                
                
                
                if ( (pLay->Prt().Height() > aPrt.Height() ||
                      pLay->Prt().Width()  > aPrt.Width()) &&
                     (pLay->IsMoveable() || pLay->IsFlyFrm()) )
                {
                    SwFrm *pTmpFrm = pLay->Lower();
                    if ( pTmpFrm && pTmpFrm->IsFlowFrm() )
                    {
                        while ( pTmpFrm->GetNext() )
                            pTmpFrm = pTmpFrm->GetNext();
                        pTmpFrm->InvalidateNextPos();
                    }
                }
            }
            bNotify = true;
            
            if( bInvaPercent )
                pLay->InvaPercentLowers( pLay->Prt().Height() - aPrt.Height() );
        }
        if ( pLay->IsTabFrm() )
            
            ((SwTabFrm*)pLay)->SetComplete();
        else
        {
            const SwViewShell *pSh = pLay->getRootFrm()->GetCurrShell();
            if( !( pSh && pSh->GetViewOptions()->getBrowseMode() ) ||
                  !(pLay->GetType() & (FRM_BODY | FRM_PAGE)) )
            
            
            
            pLay->SetCompletePaint();
        }
    }
    
    const bool bPrtPos = POS_DIFF( aPrt, pLay->Prt() );
    const bool bPos = bPrtPos || POS_DIFF( aFrm, pLay->Frm() );
    const bool bSize = pLay->Frm().SSize() != aFrm.SSize();

    if ( bPos && pLay->Lower() && !IsLowersComplete() )
        pLay->Lower()->InvalidatePos();

    if ( bPrtPos )
        pLay->SetCompletePaint();

    
    if ( bSize )
    {
        if( pLay->GetNext() )
        {
            if ( pLay->GetNext()->IsLayoutFrm() )
                pLay->GetNext()->_InvalidatePos();
            else
                pLay->GetNext()->InvalidatePos();
        }
        else if( pLay->IsSctFrm() )
            pLay->InvalidateNextPos();
    }
    if ( !IsLowersComplete() &&
         !(pLay->GetType()&(FRM_FLY|FRM_SECTION) &&
            pLay->Lower() && pLay->Lower()->IsColumnFrm()) &&
         (bPos || bNotify) && !(pLay->GetType() & 0x1823) )  
    {
        
        
        
        
        
        
        bool bUnlockPosOfObjs( bPos );
        if ( bUnlockPosOfObjs && pLay->IsCellFrm() )
        {
            SwTabFrm* pTabFrm( pLay->FindTabFrm() );
            if ( pTabFrm &&
                 ( pTabFrm->IsJoinLocked() ||
                   ( pTabFrm->IsFollow() &&
                     pTabFrm->FindMaster()->IsJoinLocked() ) ) )
            {
                bUnlockPosOfObjs = false;
            }
        }
        
        
        else if ( bUnlockPosOfObjs && pLay->IsFtnFrm() )
        {
            bUnlockPosOfObjs = static_cast<SwFtnFrm*>(pLay)->IsUnlockPosOfLowerObjs();
        }
        
        else if ( bUnlockPosOfObjs && pLay->IsSctFrm() )
        {
            bUnlockPosOfObjs = false;
        }
        pLay->NotifyLowerObjs( bUnlockPosOfObjs );
    }
    if ( bPos && pLay->IsFtnFrm() && pLay->Lower() )
    {
        
        ::lcl_InvalidatePosOfLowers( *pLay );
    }
    if( ( bPos || bSize ) && pLay->IsFlyFrm() && ((SwFlyFrm*)pLay)->GetAnchorFrm()
          && ((SwFlyFrm*)pLay)->GetAnchorFrm()->IsFlyFrm() )
        ((SwFlyFrm*)pLay)->AnchorFrm()->InvalidateSize();
}

SwFlyNotify::SwFlyNotify( SwFlyFrm *pFlyFrm ) :
    SwLayNotify( pFlyFrm ),
    
    
    pOldPage( pFlyFrm->GetPageFrm() ),
    aFrmAndSpace( pFlyFrm->GetObjRectWithSpaces() )
{
}

SwFlyNotify::~SwFlyNotify()
{
    
    if ( mbFrmDeleted )
    {
        return;
    }

    SwFlyFrm *pFly = GetFly();
    if ( pFly->IsNotifyBack() )
    {
        SwViewShell *pSh = pFly->getRootFrm()->GetCurrShell();
        SwViewImp *pImp = pSh ? pSh->Imp() : 0;
        if ( !pImp || !pImp->IsAction() || !pImp->GetLayAction().IsAgain() )
        {
            
            
            ::Notify( pFly, pOldPage, aFrmAndSpace, &aPrt );
            
            
            if ( pFly->GetAnchorFrm()->IsTxtFrm() &&
                 pFly->GetPageFrm() != pOldPage )
            {
                pFly->AnchorFrm()->Prepare( PREP_FLY_LEAVE );
            }
        }
        pFly->ResetNotifyBack();
    }

    
    
    SWRECTFN( pFly )
    const bool bPosChgd = POS_DIFF( aFrm, pFly->Frm() );
    const bool bFrmChgd = pFly->Frm().SSize() != aFrm.SSize();
    const bool bPrtChgd = aPrt != pFly->Prt();
    if ( bPosChgd || bFrmChgd || bPrtChgd )
    {
        pFly->NotifyDrawObj();
    }
    if ( bPosChgd && aFrm.Pos().X() != FAR_AWAY )
    {
        
        
        
        
        

        if ( pFly->IsFlyAtCntFrm() )
        {
            SwFrm *pNxt = pFly->AnchorFrm()->FindNext();
            if ( pNxt )
            {
                pNxt->InvalidatePos();
            }
        }

        
        
        if ( pFly->GetAnchorFrm()->IsTxtFrm() )
        {
            pFly->AnchorFrm()->Prepare( PREP_FLY_LEAVE );
        }
    }

    
    
    
    if ( pFly->ConsiderObjWrapInfluenceOnObjPos() &&
         ( !pFly->ISA(SwFlyFreeFrm) ||
           !static_cast<SwFlyFreeFrm*>(pFly)->IsNoMoveOnCheckClip() ) )
    {
        
        
        
        if ( bPosChgd )
        {
            
            pFly->SetRestartLayoutProcess( true );
        }
        else
        {
            
            pFly->LockPosition();

            if ( !pFly->ConsiderForTextWrap() )
            {
                
                pFly->SetConsiderForTextWrap( true );
                
                
                pFly->NotifyBackground( pFly->GetPageFrm(),
                                        pFly->GetObjRectWithSpaces(),
                                        PREP_FLY_ARRIVE );
                
                
                
                pFly->AnchorFrm()->InvalidatePos();
            }
        }
    }
}

SwCntntNotify::SwCntntNotify( SwCntntFrm *pCntntFrm ) :
    SwFrmNotify( pCntntFrm ),
    
    mbChkHeightOfLastLine( false ),
    mnHeightOfLastLine( 0L ),
    
    mbInvalidatePrevPrtArea( false ),
    mbBordersJoinedWithPrev( false )
{
    
    if ( pCntntFrm->IsTxtFrm() )
    {
        SwTxtFrm* pTxtFrm = static_cast<SwTxtFrm*>(pCntntFrm);
        if ( !pTxtFrm->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::OLD_LINE_SPACING) )
        {
            const SwAttrSet* pSet = pTxtFrm->GetAttrSet();
            const SvxLineSpacingItem &rSpace = pSet->GetLineSpacing();
            if ( rSpace.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            {
                mbChkHeightOfLastLine = true;
                mnHeightOfLastLine = pTxtFrm->GetHeightOfLastLine();
            }
        }
    }
}

SwCntntNotify::~SwCntntNotify()
{
    
    if ( mbFrmDeleted )
    {
        return;
    }

    SwCntntFrm *pCnt = GetCnt();
    if ( bSetCompletePaintOnInvalidate )
        pCnt->SetCompletePaint();

    SWRECTFN( pCnt )
    if ( pCnt->IsInTab() && ( POS_DIFF( pCnt->Frm(), aFrm ) ||
                             pCnt->Frm().SSize() != aFrm.SSize()))
    {
        SwLayoutFrm* pCell = pCnt->GetUpper();
        while( !pCell->IsCellFrm() && pCell->GetUpper() )
            pCell = pCell->GetUpper();
        OSL_ENSURE( pCell->IsCellFrm(), "Where's my cell?" );
        if ( text::VertOrientation::NONE != pCell->GetFmt()->GetVertOrient().GetVertOrient() )
            pCell->InvalidatePrt(); 
    }

    
    if ( mbInvalidatePrevPrtArea && mbBordersJoinedWithPrev &&
         pCnt->IsTxtFrm() &&
         !pCnt->IsFollow() && !pCnt->GetIndPrev() )
    {
        
        SwFrm* pPrevFrm = pCnt->FindPrev();
        
        {
            while ( pPrevFrm &&
                    ( ( pPrevFrm->IsSctFrm() &&
                        !static_cast<SwSectionFrm*>(pPrevFrm)->GetSection() ) ||
                      ( pPrevFrm->IsTxtFrm() &&
                        static_cast<SwTxtFrm*>(pPrevFrm)->IsHiddenNow() ) ) )
            {
                pPrevFrm = pPrevFrm->FindPrev();
            }
        }

        
        if ( pPrevFrm )
        {
            if ( pPrevFrm->IsSctFrm() )
            {
                if ( pCnt->IsInSct() )
                {
                    
                    
                    
                    
                    
                    
                    SwFrm* pLstCntntOfSctFrm =
                            static_cast<SwSectionFrm*>(pPrevFrm)->FindLastCntnt();
                    if ( pLstCntntOfSctFrm )
                    {
                        pLstCntntOfSctFrm->InvalidatePrt();
                    }
                }
            }
            else
            {
                pPrevFrm->InvalidatePrt();
            }
        }
    }

    const bool bFirst = (aFrm.*fnRect->fnGetWidth)() == 0;

    if ( pCnt->IsNoTxtFrm() )
    {
        
        
        SwViewShell *pSh  = pCnt->getRootFrm()->GetCurrShell();
        if ( pSh )
        {
            SwOLENode *pNd;
            if ( 0 != (pNd = pCnt->GetNode()->GetOLENode()) &&
                 (pNd->GetOLEObj().IsOleRef() ||
                  pNd->IsOLESizeInvalid()) )
            {
                const bool bNoTxtFrmPrtAreaChanged =
                        ( aPrt.SSize().Width() != 0 &&
                          aPrt.SSize().Height() != 0 ) &&
                        aPrt.SSize() != pCnt->Prt().SSize();
                OSL_ENSURE( pCnt->IsInFly(), "OLE not in FlyFrm" );
                SwFlyFrm *pFly = pCnt->FindFlyFrm();
                svt::EmbeddedObjectRef& xObj = pNd->GetOLEObj().GetObject();
                SwFEShell *pFESh = 0;
                SwViewShell *pTmp = pSh;
                do
                {   if ( pTmp->ISA( SwCrsrShell ) )
                    {
                        pFESh = (SwFEShell*)pTmp;
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        if ( !pNd->IsOLESizeInvalid() &&
                             !pSh->GetDoc()->IsUpdateExpFld() )
                            pFESh->CalcAndSetScale( xObj,
                                                    &pFly->Prt(), &pFly->Frm(),
                                                    bNoTxtFrmPrtAreaChanged );
                    }
                    pTmp = (SwViewShell*)pTmp->GetNext();
                } while ( pTmp != pSh );

                if ( pFESh && pNd->IsOLESizeInvalid() )
                {
                    pNd->SetOLESizeInvalid( sal_False );
                    pFESh->CalcAndSetScale( xObj ); 
                }
            }
            
            if ( Frm().HasArea() && ((SwNoTxtFrm*)pCnt)->HasAnimation() )
            {
                ((SwNoTxtFrm*)pCnt)->StopAnimation();
                pSh->InvalidateWindows( Frm() );
            }
        }
    }

    if ( bFirst )
    {
        pCnt->SetRetouche();    

        SwDoc *pDoc = pCnt->GetNode()->GetDoc();
        if ( !pDoc->GetSpzFrmFmts()->empty() &&
             pDoc->DoesContainAtPageObjWithContentAnchor() && !pDoc->IsNewDoc() )
        {
            
            
            
            
            
            
            

            const SwPageFrm *pPage = 0;
            SwNodeIndex *pIdx  = 0;
            SwFrmFmts *pTbl = pDoc->GetSpzFrmFmts();

            for ( sal_uInt16 i = 0; i < pTbl->size(); ++i )
            {
                SwFrmFmt *pFmt = (*pTbl)[i];
                const SwFmtAnchor &rAnch = pFmt->GetAnchor();
                if ( FLY_AT_PAGE != rAnch.GetAnchorId() ||
                     rAnch.GetCntntAnchor() == 0 )
                {
                    continue;
                }

                if ( !pIdx )
                {
                    pIdx = new SwNodeIndex( *pCnt->GetNode() );
                }
                if ( rAnch.GetCntntAnchor()->nNode == *pIdx )
                {
                    OSL_FAIL( "<SwCntntNotify::~SwCntntNotify()> - to page anchored object with content position. Please inform OD." );
                    if ( !pPage )
                    {
                        pPage = pCnt->FindPageFrm();
                    }
                    SwFmtAnchor aAnch( rAnch );
                    aAnch.SetAnchor( 0 );
                    aAnch.SetPageNum( pPage->GetPhyPageNum() );
                    pFmt->SetFmtAttr( aAnch );
                    if ( RES_DRAWFRMFMT != pFmt->Which() )
                    {
                        pFmt->MakeFrms();
                    }
                }
            }
            delete pIdx;
        }
    }

    
    
    if ( pCnt->IsTxtFrm() && mbChkHeightOfLastLine )
    {
        if ( mnHeightOfLastLine != static_cast<SwTxtFrm*>(pCnt)->GetHeightOfLastLine() )
        {
            pCnt->InvalidateNextPrtArea();
        }
    }

    
    if ( pCnt->IsTxtFrm() && POS_DIFF( aFrm, pCnt->Frm() ) )
    {
        pCnt->InvalidateObjs( true );
    }

    
    
    
    if ( pCnt->IsTxtFrm() )
    {
        SwTxtFrm* pMasterFrm = pCnt->IsFollow()
                               ? static_cast<SwTxtFrm*>(pCnt)->FindMaster()
                               : static_cast<SwTxtFrm*>(pCnt);
        if ( pMasterFrm && !pMasterFrm->IsFlyLock() &&
             pMasterFrm->GetDrawObjs() )
        {
            SwSortedObjs* pObjs = pMasterFrm->GetDrawObjs();
            for ( sal_uInt32 i = 0; i < pObjs->Count(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                if ( pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId()
                        == FLY_AT_CHAR )
                {
                    pAnchoredObj->CheckCharRectAndTopOfLine( !pMasterFrm->IsEmpty() );
                }
            }
        }
    }
}

void AppendObjs( const SwFrmFmts *pTbl, sal_uLong nIndex,
                        SwFrm *pFrm, SwPageFrm *pPage )
{
    for ( sal_uInt16 i = 0; i < pTbl->size(); ++i )
    {
        SwFrmFmt *pFmt = (SwFrmFmt*)(*pTbl)[i];
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        if ( rAnch.GetCntntAnchor() &&
             (rAnch.GetCntntAnchor()->nNode.GetIndex() == nIndex) )
        {
            const bool bFlyAtFly = rAnch.GetAnchorId() == FLY_AT_FLY; 
            
            const bool bSdrObj = RES_DRAWFRMFMT == pFmt->Which();
            
            
            const bool bDrawObjInCntnt = bSdrObj &&
                                         (rAnch.GetAnchorId() == FLY_AS_CHAR);

            if( bFlyAtFly ||
                (rAnch.GetAnchorId() == FLY_AT_PARA) ||
                (rAnch.GetAnchorId() == FLY_AT_CHAR) ||
                bDrawObjInCntnt )
            {
                SdrObject* pSdrObj = 0;
                if ( bSdrObj && 0 == (pSdrObj = pFmt->FindSdrObject()) )
                {
                    OSL_ENSURE( !bSdrObj, "DrawObject not found." );
                    pFmt->GetDoc()->DelFrmFmt( pFmt );
                    --i;
                    continue;
                }
                if ( pSdrObj )
                {
                    if ( !pSdrObj->GetPage() )
                    {
                        pFmt->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage(0)->
                                InsertObject(pSdrObj, pSdrObj->GetOrdNumDirect());
                    }

                    SwDrawContact* pNew =
                        static_cast<SwDrawContact*>(GetUserCall( pSdrObj ));
                    if ( !pNew->GetAnchorFrm() )
                    {
                        pFrm->AppendDrawObj( *(pNew->GetAnchoredObj( 0L )) );
                    }
                    
                    
                    else if ( !::CheckControlLayer( pSdrObj ) &&
                              pNew->GetAnchorFrm() != pFrm &&
                              !pNew->GetDrawObjectByAnchorFrm( *pFrm ) )
                    {
                        SwDrawVirtObj* pDrawVirtObj = pNew->AddVirtObj();
                        pFrm->AppendDrawObj( *(pNew->GetAnchoredObj( pDrawVirtObj )) );

                        pDrawVirtObj->ActionChanged();
                    }

                }
                else
                {
                    SwFlyFrm *pFly;
                    if( bFlyAtFly )
                        pFly = new SwFlyLayFrm( (SwFlyFrmFmt*)pFmt, pFrm, pFrm );
                    else
                        pFly = new SwFlyAtCntFrm( (SwFlyFrmFmt*)pFmt, pFrm, pFrm );
                    pFly->Lock();
                    pFrm->AppendFly( pFly );
                    pFly->Unlock();
                    if ( pPage )
                        ::RegistFlys( pPage, pFly );
                }
            }
        }
    }
}

static bool lcl_ObjConnected( SwFrmFmt *pFmt, const SwFrm* pSib )
{
    SwIterator<SwFlyFrm,SwFmt> aIter( *pFmt );
    if ( RES_FLYFRMFMT == pFmt->Which() )
    {
        const SwRootFrm* pRoot = pSib ? pSib->getRootFrm() : 0;
        const SwFlyFrm* pTmpFrm;
        for( pTmpFrm = aIter.First(); pTmpFrm; pTmpFrm = aIter.Next() )
        {
            if(! pRoot || pRoot == pTmpFrm->getRootFrm() )
                return true;
        }
    }
    else
    {
        SwDrawContact *pContact = SwIterator<SwDrawContact,SwFmt>::FirstElement(*pFmt);
        if ( pContact )
            return pContact->GetAnchorFrm() != 0;
    }
    return false;
}

/** helper method to determine, if a <SwFrmFmt>, which has an object connected,
    is located in header or footer.

    OD 23.06.2003 #108784#
*/
static bool lcl_InHeaderOrFooter( SwFrmFmt& _rFmt )
{
    bool bRetVal = false;

    const SwFmtAnchor& rAnch = _rFmt.GetAnchor();

    if (rAnch.GetAnchorId() != FLY_AT_PAGE)
    {
        bRetVal = _rFmt.GetDoc()->IsInHeaderFooter( rAnch.GetCntntAnchor()->nNode );
    }

    return bRetVal;
}

void AppendAllObjs( const SwFrmFmts *pTbl, const SwFrm* pSib )
{
    
    
    
    
    

    SwFrmFmts aCpy( *pTbl );

    sal_uInt16 nOldCnt = USHRT_MAX;

    while ( !aCpy.empty() && aCpy.size() != nOldCnt )
    {
        nOldCnt = aCpy.size();
        for ( int i = 0; i < int(aCpy.size()); ++i )
        {
            SwFrmFmt *pFmt = (SwFrmFmt*)aCpy[ sal_uInt16(i) ];
            const SwFmtAnchor &rAnch = pFmt->GetAnchor();
            sal_Bool bRemove = sal_False;
            if ((rAnch.GetAnchorId() == FLY_AT_PAGE) ||
                (rAnch.GetAnchorId() == FLY_AS_CHAR))
            {
                
                
                bRemove = sal_True;
            }
            else if ( sal_False == (bRemove = ::lcl_ObjConnected( pFmt, pSib )) ||
                      ::lcl_InHeaderOrFooter( *pFmt ) )
            {
            
            
            
                
                
                
                pFmt->MakeFrms();
                bRemove = ::lcl_ObjConnected( pFmt, pSib );
            }
            if ( bRemove )
            {
                aCpy.erase( aCpy.begin() + i );
                --i;
            }
        }
    }
    aCpy.clear();
}

/** local method to set 'working' position for newly inserted frames

    OD 12.08.2003 #i17969#
*/
static void lcl_SetPos( SwFrm&             _rNewFrm,
                 const SwLayoutFrm& _rLayFrm )
{
    SWRECTFN( (&_rLayFrm) )
    (_rNewFrm.Frm().*fnRect->fnSetPos)( (_rLayFrm.Frm().*fnRect->fnGetPos)() );
    
    
    if ( bVert )
        _rNewFrm.Frm().Pos().X() -= 1;
    else
        _rNewFrm.Frm().Pos().Y() += 1;
}

void _InsertCnt( SwLayoutFrm *pLay, SwDoc *pDoc,
                             sal_uLong nIndex, sal_Bool bPages, sal_uLong nEndIndex,
                             SwFrm *pPrv )
{
    pDoc->BlockIdling();
    SwRootFrm* pLayout = pLay->getRootFrm();
    const sal_Bool bOldCallbackActionEnabled = pLayout ? pLayout->IsCallbackActionEnabled() : sal_False;
    if( bOldCallbackActionEnabled )
        pLayout->SetCallbackActionEnabled( sal_False );

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    const bool bStartPercent = bPages && !nEndIndex;

    SwPageFrm *pPage = pLay->FindPageFrm();
    const SwFrmFmts *pTbl = pDoc->GetSpzFrmFmts();
    SwFrm       *pFrm = 0;
    sal_Bool   bBreakAfter   = sal_False;

    SwActualSection *pActualSection = 0;
    SwLayHelper *pPageMaker;

    
    
    
    if ( bPages )
    {
        
        
        pPageMaker = new SwLayHelper( pDoc, pFrm, pPrv, pPage, pLay,
                pActualSection, bBreakAfter, nIndex, 0 == nEndIndex );
        if( bStartPercent )
        {
            const sal_uLong nPageCount = pPageMaker->CalcPageCount();
            if( nPageCount )
                bObjsDirect = false;
        }
    }
    else
        pPageMaker = NULL;

    if( pLay->IsInSct() &&
        ( pLay->IsSctFrm() || pLay->GetUpper() ) ) 
            
            
    {
        SwSectionFrm* pSct = pLay->FindSctFrm();
        
        
        
        
        
        
        
        if( ( !pLay->IsInFtn() || pSct->IsInFtn() ) &&
            ( !pLay->IsInTab() || pSct->IsInTab() ) )
        {
            pActualSection = new SwActualSection( 0, pSct, 0 );
            OSL_ENSURE( !pLay->Lower() || !pLay->Lower()->IsColumnFrm(),
                "_InsertCnt: Wrong Call" );
        }
    }

    
    
    
    
    

    while( true )
    {
        SwNode *pNd = pDoc->GetNodes()[nIndex];
        if ( pNd->IsCntntNode() )
        {
            SwCntntNode* pNode = (SwCntntNode*)pNd;
            pFrm = pNode->MakeFrm(pLay);
            if( pPageMaker )
                pPageMaker->CheckInsert( nIndex );

            pFrm->InsertBehind( pLay, pPrv );
            
            
            
            
            
            if ( pFrm->IsTxtFrm() )
            {
                SwViewShell* pViewShell( pFrm->getRootFrm()->GetCurrShell() );
                
                if ( pViewShell && !pViewShell->IsInConstructor() &&
                     pViewShell->GetLayout() &&
                     pViewShell->GetLayout()->IsAnyShellAccessible() )
                {
                    pViewShell->InvalidateAccessibleParaFlowRelation(
                        dynamic_cast<SwTxtFrm*>(pFrm->FindNextCnt( true )),
                        dynamic_cast<SwTxtFrm*>(pFrm->FindPrevCnt( true )) );
                    
                    
                    
                    
                    
                    
                    pFrm->InvalidateInfFlags();
                }
            }
            
            
            lcl_SetPos( *pFrm, *pLay );
            pPrv = pFrm;

            if ( !pTbl->empty() && bObjsDirect && !bDontCreateObjects )
                AppendObjs( pTbl, nIndex, pFrm, pPage );
        }
        else if ( pNd->IsTableNode() )
        {   
            SwTableNode *pTblNode = (SwTableNode*)pNd;

            
            
            
            SwTableFmlUpdate aMsgHnt( &pTblNode->GetTable() );
            aMsgHnt.eFlags = TBL_BOXPTR;
            pDoc->UpdateTblFlds( &aMsgHnt );
            pTblNode->GetTable().GCLines();

            pFrm = pTblNode->MakeFrm( pLay );

            if( pPageMaker )
                pPageMaker->CheckInsert( nIndex );

            pFrm->InsertBehind( pLay, pPrv );
            
            
            
            
            
            {
                SwViewShell* pViewShell( pFrm->getRootFrm()->GetCurrShell() );
                
                if ( pViewShell && !pViewShell->IsInConstructor() &&
                     pViewShell->GetLayout() &&
                     pViewShell->GetLayout()->IsAnyShellAccessible() )
                {
                    pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pFrm->FindNextCnt( true )),
                            dynamic_cast<SwTxtFrm*>(pFrm->FindPrevCnt( true )) );
                }
            }
            if ( bObjsDirect && !pTbl->empty() )
                ((SwTabFrm*)pFrm)->RegistFlys();
            
            
            lcl_SetPos( *pFrm, *pLay );

            pPrv = pFrm;
            
            nIndex = pTblNode->EndOfSectionIndex();

            SwTabFrm* pTmpFrm = (SwTabFrm*)pFrm;
            while ( pTmpFrm )
            {
                pTmpFrm->CheckDirChange();
                pTmpFrm = pTmpFrm->IsFollow() ? pTmpFrm->FindMaster() : NULL;
            }

        }
        else if ( pNd->IsSectionNode() )
        {
            SwSectionNode *pNode = (SwSectionNode*)pNd;
            if( pNode->GetSection().CalcHiddenFlag() )
                
                nIndex = pNode->EndOfSectionIndex();
            else
            {
                pFrm = pNode->MakeFrm( pLay );
                pActualSection = new SwActualSection( pActualSection,
                                                (SwSectionFrm*)pFrm, pNode );
                if ( pActualSection->GetUpper() )
                {
                    
                    
                    SwSectionFrm *pTmp = pActualSection->GetUpper()->GetSectionFrm();
                    pFrm->InsertBehind( pTmp->GetUpper(), pTmp );
                    
                    
                    static_cast<SwSectionFrm*>(pFrm)->Init();
                }
                else
                {
                    pFrm->InsertBehind( pLay, pPrv );
                    
                    
                    static_cast<SwSectionFrm*>(pFrm)->Init();

                    
                    
                    
                    
                    if( pPrv && 0 != pPrv->ImplFindFtnFrm() )
                    {
                        if( pPrv->IsSctFrm() )
                            pPrv = ((SwSectionFrm*)pPrv)->ContainsCntnt();
                        if( pPrv && pPrv->IsTxtFrm() )
                            ((SwTxtFrm*)pPrv)->Prepare( PREP_QUOVADIS, 0, sal_False );
                    }
                }
                
                
                
                
                
                {
                    SwViewShell* pViewShell( pFrm->getRootFrm()->GetCurrShell() );
                    
                    if ( pViewShell && !pViewShell->IsInConstructor() &&
                         pViewShell->GetLayout() &&
                         pViewShell->GetLayout()->IsAnyShellAccessible() )
                    {
                        pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pFrm->FindNextCnt( true )),
                            dynamic_cast<SwTxtFrm*>(pFrm->FindPrevCnt( true )) );
                    }
                }
                pFrm->CheckDirChange();

                
                
                lcl_SetPos( *pFrm, *pLay );

                
                if ( pPage )
                {
                    
                    
                    
                    pFrm->InvalidatePage( pPage );

                    
                    
                    if ( pFrm->IsInFly() )
                        pPage->InvalidateFlyCntnt();

                    
                    
                    pPage->InvalidateCntnt();
                }

                pLay = (SwLayoutFrm*)pFrm;
                if ( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
                    pLay = pLay->GetNextLayoutLeaf();
                pPrv = 0;
            }
        }
        else if ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode() )
        {
            OSL_ENSURE( pActualSection, "Sectionende ohne Anfang?" );
            OSL_ENSURE( pActualSection->GetSectionNode() == pNd->StartOfSectionNode(),
                            "Sectionende mit falschen Start Node?" );

            
            
            SwActualSection *pTmp = pActualSection->GetUpper();
            delete pActualSection;
            pLay = pLay->FindSctFrm();
            if ( 0 != (pActualSection = pTmp) )
            {
                
                
                if ( !pLay->ContainsCntnt() )
                {
                    SwFrm *pTmpFrm = pLay;
                    pLay = pTmpFrm->GetUpper();
                    pPrv = pTmpFrm->GetPrev();
                    pTmpFrm->Remove();
                    delete pTmpFrm;
                }
                else
                {
                    pPrv = pLay;
                    pLay = pLay->GetUpper();
                }

                
                pFrm = pActualSection->GetSectionNode()->MakeFrm( pLay );
                pFrm->InsertBehind( pLay, pPrv );
                static_cast<SwSectionFrm*>(pFrm)->Init();

                
                
                lcl_SetPos( *pFrm, *pLay );

                SwSectionFrm* pOuterSectionFrm = pActualSection->GetSectionFrm();

                
                SwSectionFrm* pFollow = pOuterSectionFrm->GetFollow();
                if ( pFollow )
                {
                    pOuterSectionFrm->SetFollow( NULL );
                    pOuterSectionFrm->InvalidateSize();
                    ((SwSectionFrm*)pFrm)->SetFollow( pFollow );
                }

                
                if( ! pOuterSectionFrm->IsColLocked() &&
                    ! pOuterSectionFrm->ContainsCntnt() )
                {
                    pOuterSectionFrm->DelEmpty( sal_True );
                    delete pOuterSectionFrm;
                }
                pActualSection->SetSectionFrm( (SwSectionFrm*)pFrm );

                pLay = (SwLayoutFrm*)pFrm;
                if ( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
                    pLay = pLay->GetNextLayoutLeaf();
                pPrv = 0;
            }
            else
            {
                
                
                pPrv = pLay;
                pLay = pLay->GetUpper();
            }
        }
        else if( pNd->IsStartNode() &&
                 SwFlyStartNode == ((SwStartNode*)pNd)->GetStartNodeType() )
        {
            if ( !pTbl->empty() && bObjsDirect && !bDontCreateObjects )
            {
                SwFlyFrm* pFly = pLay->FindFlyFrm();
                if( pFly )
                    AppendObjs( pTbl, nIndex, pFly, pPage );
            }
        }
        else
            
            
            break;

        ++nIndex;
        
        
        if ( nEndIndex && nIndex >= nEndIndex )
            break;
    }

    if ( pActualSection )
    {
        
        if ( !(pLay = pActualSection->GetSectionFrm())->ContainsCntnt() )
        {
            pLay->Remove();
            delete pLay;
        }
        delete pActualSection;
    }

    if ( bPages ) 
    {
        if ( !bDontCreateObjects )
            AppendAllObjs( pTbl, pLayout );
        bObjsDirect = true;
    }

    if( pPageMaker )
    {
        pPageMaker->CheckFlyCache( pPage );
        delete pPageMaker;
        if( pDoc->GetLayoutCache() )
        {
#ifdef DBG_UTIL
            pDoc->GetLayoutCache()->CompareLayout( *pDoc );
#endif
            pDoc->GetLayoutCache()->ClearImpl();
        }
    }

    pDoc->UnblockIdling();
    if( bOldCallbackActionEnabled )
        pLayout->SetCallbackActionEnabled( bOldCallbackActionEnabled );
}

void MakeFrms( SwDoc *pDoc, const SwNodeIndex &rSttIdx,
               const SwNodeIndex &rEndIdx )
{
    bObjsDirect = false;

    SwNodeIndex aTmp( rSttIdx );
    sal_uLong nEndIdx = rEndIdx.GetIndex();
    SwNode* pNd = pDoc->GetNodes().FindPrvNxtFrmNode( aTmp,
                                            pDoc->GetNodes()[ nEndIdx-1 ]);
    if ( pNd )
    {
        sal_Bool bApres = aTmp < rSttIdx;
        SwNode2Layout aNode2Layout( *pNd, rSttIdx.GetIndex() );
        SwFrm* pFrm;
        while( 0 != (pFrm = aNode2Layout.NextFrm()) )
        {
            SwLayoutFrm *pUpper = pFrm->GetUpper();
            SwFtnFrm* pFtnFrm = pUpper->FindFtnFrm();
            sal_Bool bOldLock, bOldFtn;
            if( pFtnFrm )
            {
                bOldFtn = pFtnFrm->IsColLocked();
                pFtnFrm->ColLock();
            }
            else
                bOldFtn = sal_True;
            SwSectionFrm* pSct = pUpper->FindSctFrm();
            
            
            
            if( pSct && ((pFtnFrm && !pSct->IsInFtn()) || pUpper->IsCellFrm()) )
                pSct = NULL;
            if( pSct )
            {   
                bOldLock = pSct->IsColLocked();
                pSct->ColLock();
            }
            else
                bOldLock = sal_True;

            
            
            
            bool bMoveNext = nEndIdx - rSttIdx.GetIndex() > 120;
            bool bAllowMove = !pFrm->IsInFly() && pFrm->IsMoveable() &&
                 (!pFrm->IsInTab() || pFrm->IsTabFrm() );
            if ( bMoveNext && bAllowMove )
            {
                SwFrm *pMove = pFrm;
                SwFrm *pPrev = pFrm->GetPrev();
                SwFlowFrm *pTmp = SwFlowFrm::CastFlowFrm( pMove );
                OSL_ENSURE( pTmp, "Missing FlowFrm" );

                if ( bApres )
                {
                    
                    
                    OSL_ENSURE( !pTmp->HasFollow(), "Follows forbidden" );
                    pPrev = pFrm;
                    
                    
                    pMove = pFrm->GetIndNext();
                    SwColumnFrm* pCol = (SwColumnFrm*)pFrm->FindColFrm();
                    if( pCol )
                        pCol = (SwColumnFrm*)pCol->GetNext();
                    do
                    {
                        if( pCol && !pMove )
                        {   
                            pMove = pCol->ContainsAny();
                            if( pCol->GetNext() )
                                pCol = (SwColumnFrm*)pCol->GetNext();
                            else if( pCol->IsInSct() )
                            {   
                                
                                pCol = (SwColumnFrm*)pCol->FindSctFrm()->FindColFrm();
                                if( pCol )
                                    pCol = (SwColumnFrm*)pCol->GetNext();
                            }
                            else
                                pCol = NULL;
                        }
                        
                        while( pMove && pMove->IsSctFrm() &&
                               !((SwSectionFrm*)pMove)->GetSection() )
                            pMove = pMove->GetNext();
                    } while( !pMove && pCol );

                    if( pMove )
                    {
                        if ( pMove->IsCntntFrm() )
                            pTmp = (SwCntntFrm*)pMove;
                        else if ( pMove->IsTabFrm() )
                            pTmp = (SwTabFrm*)pMove;
                        else if ( pMove->IsSctFrm() )
                        {
                            pMove = ((SwSectionFrm*)pMove)->ContainsAny();
                            if( pMove )
                                pTmp = SwFlowFrm::CastFlowFrm( pMove );
                            else
                                pTmp = NULL;
                        }
                    }
                    else
                        pTmp = 0;
                }
                else
                {
                    OSL_ENSURE( !pTmp->IsFollow(), "Follows really forbidden" );
                    
                    if( pMove->IsSctFrm() )
                    {
                        while( pMove && pMove->IsSctFrm() &&
                               !((SwSectionFrm*)pMove)->GetSection() )
                            pMove = pMove->GetNext();
                        if( pMove && pMove->IsSctFrm() )
                            pMove = ((SwSectionFrm*)pMove)->ContainsAny();
                        if( pMove )
                            pTmp = SwFlowFrm::CastFlowFrm( pMove );
                        else
                            pTmp = NULL;
                    }
                }

                if( pTmp )
                {
                    SwFrm* pOldUp = pTmp->GetFrm()->GetUpper();
                    
                    
                    sal_Bool bTmpOldLock = pTmp->IsJoinLocked();
                    pTmp->LockJoin();
                    while( pTmp->MoveFwd( sal_True, sal_False, sal_True ) )
                    {
                        if( pOldUp == pTmp->GetFrm()->GetUpper() )
                            break;
                        pOldUp = pTmp->GetFrm()->GetUpper();
                    }
                    if( !bTmpOldLock )
                        pTmp->UnlockJoin();
                }
                ::_InsertCnt( pUpper, pDoc, rSttIdx.GetIndex(),
                              pFrm->IsInDocBody(), nEndIdx, pPrev );
            }
            else
            {
                sal_Bool bSplit;
                SwFrm* pPrv = bApres ? pFrm : pFrm->GetPrev();
                
                if( pSct && rSttIdx.GetNode().IsSectionNode() )
                {
                    bSplit = pSct->SplitSect( pFrm, bApres );
                    if( !bSplit && !bApres )
                    {
                        pUpper = pSct->GetUpper();
                        pPrv = pSct->GetPrev();
                    }
                }
                else
                    bSplit = sal_False;

                ::_InsertCnt( pUpper, pDoc, rSttIdx.GetIndex(), sal_False,
                              nEndIdx, pPrv );
                
                
                if( !bDontCreateObjects )
                {
                    const SwFrmFmts *pTbl = pDoc->GetSpzFrmFmts();
                    if( !pTbl->empty() )
                        AppendAllObjs( pTbl, pUpper );
                }

                
                if( bSplit && pSct && pSct->GetNext()
                    && pSct->GetNext()->IsSctFrm() )
                    pSct->MergeNext( (SwSectionFrm*)pSct->GetNext() );
                if( pFrm->IsInFly() )
                    pFrm->FindFlyFrm()->_Invalidate();
                if( pFrm->IsInTab() )
                    pFrm->InvalidateSize();
            }

            SwPageFrm *pPage = pUpper->FindPageFrm();
            SwFrm::CheckPageDescs( pPage, sal_False );
            if( !bOldFtn )
                pFtnFrm->ColUnlock();
            if( !bOldLock )
            {
                pSct->ColUnlock();
                
                
                if( !pSct->ContainsCntnt() )
                {
                    pSct->DelEmpty( sal_True );
                    pUpper->getRootFrm()->RemoveFromList( pSct );
                    delete pSct;
                }
            }
        }
    }

    bObjsDirect = true;
}

SwBorderAttrs::SwBorderAttrs( const SwModify *pMod, const SwFrm *pConstructor ) :
    SwCacheObj( pMod ),
    rAttrSet( pConstructor->IsCntntFrm()
                    ? ((SwCntntFrm*)pConstructor)->GetNode()->GetSwAttrSet()
                    : ((SwLayoutFrm*)pConstructor)->GetFmt()->GetAttrSet() ),
    rUL     ( rAttrSet.GetULSpace() ),
    
    
    rLR     ( rAttrSet.GetLRSpace() ),
    rBox    ( rAttrSet.GetBox()     ),
    rShadow ( rAttrSet.GetShadow()  ),
    aFrmSize( rAttrSet.GetFrmSize().GetSize() )
{
    
    const SwTxtFrm* pTxtFrm = dynamic_cast<const SwTxtFrm*>(pConstructor);
    if ( pTxtFrm )
    {
        pTxtFrm->GetTxtNode()->ClearLRSpaceItemDueToListLevelIndents( rLR );
    }
    else if ( pConstructor->IsNoTxtFrm() )
    {
        rLR = SvxLRSpaceItem ( RES_LR_SPACE );
    }

    

    
    bTopLine = bBottomLine = bLeftLine = bRightLine =
    bTop     = bBottom     = bLine   = sal_True;

    bCacheGetLine = bCachedGetTopLine = bCachedGetBottomLine = sal_False;
    
    
    bCachedJoinedWithPrev = sal_False;
    bCachedJoinedWithNext = sal_False;

    bBorderDist = 0 != (pConstructor->GetType() & (FRM_CELL));
}

SwBorderAttrs::~SwBorderAttrs()
{
    ((SwModify*)pOwner)->SetInCache( sal_False );
}

/* All calc methods calculate a safety distance in addition to the values given by the attributes.
 * This safety distance is only added when working with borders and/or shadows to prevent that
 * e.g. borders are painted over.
 */

void SwBorderAttrs::_CalcTop()
{
    nTop = CalcTopLine() + rUL.GetUpper();
    bTop = sal_False;
}

void SwBorderAttrs::_CalcBottom()
{
    nBottom = CalcBottomLine() + rUL.GetLower();
    bBottom = sal_False;
}

long SwBorderAttrs::CalcRight( const SwFrm* pCaller ) const
{
    long nRight=0;

    if (!pCaller->IsTxtFrm() || !((SwTxtFrm*)pCaller)->GetTxtNode()->GetDoc()->get(IDocumentSettingAccess::INVERT_BORDER_SPACING)) {
    
    
    if ( pCaller->IsCellFrm() && pCaller->IsRightToLeft() )
        nRight = CalcLeftLine();
    else
        nRight = CalcRightLine();

    }
    
    if ( pCaller->IsTxtFrm() && pCaller->IsRightToLeft() )
        nRight += rLR.GetLeft();
    else
        nRight += rLR.GetRight();

    
    if ( pCaller->IsTxtFrm() && pCaller->IsRightToLeft() )
    {
        nRight += ((SwTxtFrm*)pCaller)->GetTxtNode()->GetLeftMarginWithNum();
    }

    return nRight;
}


static bool lcl_hasTabFrm(const SwTxtFrm* pTxtFrm)
{
    if (pTxtFrm->GetDrawObjs())
    {
        const SwSortedObjs* pSortedObjs = pTxtFrm->GetDrawObjs();
        if (pSortedObjs->Count() > 0)
        {
            SwAnchoredObject* pObject = (*pSortedObjs)[0];
            if (pObject->IsA(TYPE(SwFlyFrm)))
            {
                SwFlyFrm* pFly = (SwFlyFrm*)pObject;
                if (pFly->Lower() && pFly->Lower()->IsTabFrm())
                    return true;
            }
        }
    }
    return false;
}

long SwBorderAttrs::CalcLeft( const SwFrm *pCaller ) const
{
    long nLeft=0;

    if (!pCaller->IsTxtFrm() || !((SwTxtFrm*)pCaller)->GetTxtNode()->GetDoc()->get(IDocumentSettingAccess::INVERT_BORDER_SPACING)) {
    
    
    if ( pCaller->IsCellFrm() && pCaller->IsRightToLeft() )
        nLeft = CalcRightLine();
    else
        nLeft = CalcLeftLine();
    }

    
    if ( pCaller->IsTxtFrm() && pCaller->IsRightToLeft() )
        nLeft += rLR.GetRight();
    else
    {
        bool bIgnoreMargin = false;
        if (pCaller->IsTxtFrm())
        {
            const SwTxtFrm* pTxtFrm = (const SwTxtFrm*)pCaller;
            if (pTxtFrm->GetTxtNode()->GetDoc()->get(IDocumentSettingAccess::FLOATTABLE_NOMARGINS))
            {
                
                if (lcl_hasTabFrm(pTxtFrm))
                    bIgnoreMargin = true;
                
                else if (pTxtFrm->FindPrev() && pTxtFrm->FindPrev()->IsTxtFrm() && lcl_hasTabFrm((const SwTxtFrm*)pTxtFrm->FindPrev()))
                    bIgnoreMargin = true;
            }
        }
        if (!bIgnoreMargin)
            nLeft += rLR.GetLeft();
    }

    
    if ( pCaller->IsTxtFrm() && !pCaller->IsRightToLeft() )
    {
        nLeft += ((SwTxtFrm*)pCaller)->GetTxtNode()->GetLeftMarginWithNum();
    }

    return nLeft;
}

/* Calculated values for borders and shadows.
 * It might be that a distance is wanted even without lines. This will be
 * considered here and not by the attribute (e.g. bBorderDist for cells).
 */

void SwBorderAttrs::_CalcTopLine()
{
    nTopLine = (bBorderDist && !rBox.GetTop())
                            ? rBox.GetDistance  (BOX_LINE_TOP)
                            : rBox.CalcLineSpace(BOX_LINE_TOP);
    nTopLine = nTopLine + rShadow.CalcShadowSpace(SHADOW_TOP);
    bTopLine = sal_False;
}

void SwBorderAttrs::_CalcBottomLine()
{
    nBottomLine = (bBorderDist && !rBox.GetBottom())
                            ? rBox.GetDistance  (BOX_LINE_BOTTOM)
                            : rBox.CalcLineSpace(BOX_LINE_BOTTOM);
    nBottomLine = nBottomLine + rShadow.CalcShadowSpace(SHADOW_BOTTOM);
    bBottomLine = sal_False;
}

void SwBorderAttrs::_CalcLeftLine()
{
    nLeftLine = (bBorderDist && !rBox.GetLeft())
                            ? rBox.GetDistance  (BOX_LINE_LEFT)
                            : rBox.CalcLineSpace(BOX_LINE_LEFT);
    nLeftLine = nLeftLine + rShadow.CalcShadowSpace(SHADOW_LEFT);
    bLeftLine = sal_False;
}

void SwBorderAttrs::_CalcRightLine()
{
    nRightLine = (bBorderDist && !rBox.GetRight())
                            ? rBox.GetDistance  (BOX_LINE_RIGHT)
                            : rBox.CalcLineSpace(BOX_LINE_RIGHT);
    nRightLine = nRightLine + rShadow.CalcShadowSpace(SHADOW_RIGHT);
    bRightLine = sal_False;
}

void SwBorderAttrs::_IsLine()
{
    bIsLine = rBox.GetTop() || rBox.GetBottom() ||
              rBox.GetLeft()|| rBox.GetRight();
    bLine = sal_False;
}

/* The borders of neighboring paragraphs are condensed by following algorithm:
 *
 * 1. No top border if the predecessor has the same top border and (3) applies.
 *    In addition, the paragraph needs to have a border at at least one side (left/right/bottom).
 * 2. No bottom border if the successor has the same bottom border and (3) applies.
 *    In addition, the paragraph needs to have a border at at least one side (left/right/top).
 * 3. The borders on the left and right side are identical between the current and the
 *    pre-/succeeding paragraph.
 */

inline bool CmpLines( const editeng::SvxBorderLine *pL1, const editeng::SvxBorderLine *pL2 )
{
    return ( ((pL1 && pL2) && (*pL1 == *pL2)) || (!pL1 && !pL2) );
}





sal_Bool SwBorderAttrs::CmpLeftRight( const SwBorderAttrs &rCmpAttrs,
                                  const SwFrm *pCaller,
                                  const SwFrm *pCmp ) const
{
    return ( CmpLines( rCmpAttrs.GetBox().GetLeft(), GetBox().GetLeft()  ) &&
             CmpLines( rCmpAttrs.GetBox().GetRight(),GetBox().GetRight() ) &&
             CalcLeft( pCaller ) == rCmpAttrs.CalcLeft( pCmp ) &&
             
             CalcRight( pCaller ) == rCmpAttrs.CalcRight( pCmp ) );
}

sal_Bool SwBorderAttrs::_JoinWithCmp( const SwFrm& _rCallerFrm,
                                  const SwFrm& _rCmpFrm ) const
{
    sal_Bool bReturnVal = sal_False;

    SwBorderAttrAccess aCmpAccess( SwFrm::GetCache(), &_rCmpFrm );
    const SwBorderAttrs &rCmpAttrs = *aCmpAccess.Get();
    if ( rShadow == rCmpAttrs.GetShadow() &&
         CmpLines( rBox.GetTop(), rCmpAttrs.GetBox().GetTop() ) &&
         CmpLines( rBox.GetBottom(), rCmpAttrs.GetBox().GetBottom() ) &&
         CmpLeftRight( rCmpAttrs, &_rCallerFrm, &_rCmpFrm )
       )
    {
        bReturnVal = sal_True;
    }

    return bReturnVal;
}




void SwBorderAttrs::_CalcJoinedWithPrev( const SwFrm& _rFrm,
                                         const SwFrm* _pPrevFrm )
{
    
    bJoinedWithPrev = sal_False;

    if ( _rFrm.IsTxtFrm() )
    {
        
        
        
        
        const SwFrm* pPrevFrm = _pPrevFrm ? _pPrevFrm : _rFrm.GetPrev();
        
        while ( pPrevFrm && pPrevFrm->IsTxtFrm() &&
                static_cast<const SwTxtFrm*>(pPrevFrm)->IsHiddenNow() )
        {
            pPrevFrm = pPrevFrm->GetPrev();
        }
        if ( pPrevFrm && pPrevFrm->IsTxtFrm() &&
             pPrevFrm->GetAttrSet()->GetParaConnectBorder().GetValue()
           )
        {
            bJoinedWithPrev = _JoinWithCmp( _rFrm, *(pPrevFrm) );
        }
    }

    
    
    
    bCachedJoinedWithPrev = bCacheGetLine && !_pPrevFrm;
}



void SwBorderAttrs::_CalcJoinedWithNext( const SwFrm& _rFrm )
{
    
    bJoinedWithNext = sal_False;

    if ( _rFrm.IsTxtFrm() )
    {
        
        
        
        const SwFrm* pNextFrm = _rFrm.GetNext();
        while ( pNextFrm && pNextFrm->IsTxtFrm() &&
                static_cast<const SwTxtFrm*>(pNextFrm)->IsHiddenNow() )
        {
            pNextFrm = pNextFrm->GetNext();
        }
        if ( pNextFrm && pNextFrm->IsTxtFrm() &&
             _rFrm.GetAttrSet()->GetParaConnectBorder().GetValue()
           )
        {
            bJoinedWithNext = _JoinWithCmp( _rFrm, *(pNextFrm) );
        }
    }

    
    bCachedJoinedWithNext = bCacheGetLine;
}




sal_Bool SwBorderAttrs::JoinedWithPrev( const SwFrm& _rFrm,
                                    const SwFrm* _pPrevFrm ) const
{
    if ( !bCachedJoinedWithPrev || _pPrevFrm )
    {
        
        const_cast<SwBorderAttrs*>(this)->_CalcJoinedWithPrev( _rFrm, _pPrevFrm );
    }

    return bJoinedWithPrev;
}

sal_Bool SwBorderAttrs::JoinedWithNext( const SwFrm& _rFrm ) const
{
    if ( !bCachedJoinedWithNext )
    {
        const_cast<SwBorderAttrs*>(this)->_CalcJoinedWithNext( _rFrm );
    }

    return bJoinedWithNext;
}



void SwBorderAttrs::_GetTopLine( const SwFrm& _rFrm,
                                 const SwFrm* _pPrevFrm )
{
    sal_uInt16 nRet = CalcTopLine();

    
    
    if ( JoinedWithPrev( _rFrm, _pPrevFrm ) )
    {
        nRet = 0;
    }

    bCachedGetTopLine = bCacheGetLine;

    nGetTopLine = nRet;
}

void SwBorderAttrs::_GetBottomLine( const SwFrm& _rFrm )
{
    sal_uInt16 nRet = CalcBottomLine();

    
    if ( JoinedWithNext( _rFrm ) )
    {
        nRet = 0;
    }

    bCachedGetBottomLine = bCacheGetLine;

    nGetBottomLine = nRet;
}

/*************************************************************************/

SwBorderAttrAccess::SwBorderAttrAccess( SwCache &rCach, const SwFrm *pFrm ) :
    SwCacheAccess( rCach, (pFrm->IsCntntFrm() ?
                                (void*)((SwCntntFrm*)pFrm)->GetNode() :
                                (void*)((SwLayoutFrm*)pFrm)->GetFmt()),
                           (sal_Bool)(pFrm->IsCntntFrm() ?
                ((SwModify*)((SwCntntFrm*)pFrm)->GetNode())->IsInCache() :
                ((SwModify*)((SwLayoutFrm*)pFrm)->GetFmt())->IsInCache()) ),
    pConstructor( pFrm )
{
}

SwCacheObj *SwBorderAttrAccess::NewObj()
{
    ((SwModify*)pOwner)->SetInCache( sal_True );
    return new SwBorderAttrs( (SwModify*)pOwner, pConstructor );
}

SwBorderAttrs *SwBorderAttrAccess::Get()
{
    return (SwBorderAttrs*)SwCacheAccess::Get();
}

SwOrderIter::SwOrderIter( const SwPageFrm *pPg, sal_Bool bFlys ) :
    pPage( pPg ),
    pCurrent( 0 ),
    bFlysOnly( bFlys )
{
}

const SdrObject *SwOrderIter::Top()
{
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        const SwSortedObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->Count() )
        {
            sal_uInt32 nTopOrd = 0;
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  
            for ( sal_uInt16 i = 0; i < pObjs->Count(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( bFlysOnly && !pObj->ISA(SwVirtFlyDrawObj) )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp >= nTopOrd )
                {
                    nTopOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}

const SdrObject *SwOrderIter::Bottom()
{
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        sal_uInt32 nBotOrd = USHRT_MAX;
        const SwSortedObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->Count() )
        {
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  
            for ( sal_uInt16 i = 0; i < pObjs->Count(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( bFlysOnly && !pObj->ISA(SwVirtFlyDrawObj) )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp < nBotOrd )
                {
                    nBotOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}

const SdrObject *SwOrderIter::Next()
{
    const sal_uInt32 nCurOrd = pCurrent ? pCurrent->GetOrdNumDirect() : 0;
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        sal_uInt32 nOrd = USHRT_MAX;
        const SwSortedObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->Count() )
        {
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  
            for ( sal_uInt16 i = 0; i < pObjs->Count(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( bFlysOnly && !pObj->ISA(SwVirtFlyDrawObj) )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp > nCurOrd && nTmp < nOrd )
                {
                    nOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}

const SdrObject *SwOrderIter::Prev()
{
    const sal_uInt32 nCurOrd = pCurrent ? pCurrent->GetOrdNumDirect() : 0;
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        const SwSortedObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->Count() )
        {
            sal_uInt32 nOrd = 0;
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  
            for ( sal_uInt16 i = 0; i < pObjs->Count(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( bFlysOnly && !pObj->ISA(SwVirtFlyDrawObj) )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp < nCurOrd && nTmp >= nOrd )
                {
                    nOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}








//


static void lcl_RemoveObjsFromPage( SwFrm* _pFrm )
{
    OSL_ENSURE( _pFrm->GetDrawObjs(), "no DrawObjs in lcl_RemoveObjsFromPage." );
    SwSortedObjs &rObjs = *_pFrm->GetDrawObjs();
    for ( sal_uInt16 i = 0; i < rObjs.Count(); ++i )
    {
        SwAnchoredObject* pObj = rObjs[i];
        
        
        pObj->ClearVertPosOrientFrm();
        
        pObj->ResetLayoutProcessBools();
        
        
        if ( pObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm* pFlyFrm = static_cast<SwFlyFrm*>(pObj);

            
            
            if ( pFlyFrm->GetDrawObjs() )
            {
                ::lcl_RemoveObjsFromPage( pFlyFrm );
            }

            SwCntntFrm* pCnt = pFlyFrm->ContainsCntnt();
            while ( pCnt )
            {
                if ( pCnt->GetDrawObjs() )
                    ::lcl_RemoveObjsFromPage( pCnt );
                pCnt = pCnt->GetNextCntntFrm();
            }
            if ( pFlyFrm->IsFlyFreeFrm() )
            {
                
                pFlyFrm->GetPageFrm()->RemoveFlyFromPage( pFlyFrm );
            }
        }
        
        else if ( pObj->ISA(SwAnchoredDrawObject) )
        {
            if (pObj->GetFrmFmt().GetAnchor().GetAnchorId() != FLY_AS_CHAR)
            {
                pObj->GetPageFrm()->RemoveDrawObjFromPage(
                                *(static_cast<SwAnchoredDrawObject*>(pObj)) );
            }
        }
    }
}

SwFrm *SaveCntnt( SwLayoutFrm *pLay, SwFrm *pStart )
{
    if( pLay->IsSctFrm() && pLay->Lower() && pLay->Lower()->IsColumnFrm() )
        sw_RemoveFtns( (SwColumnFrm*)pLay->Lower(), sal_True, sal_True );

    SwFrm *pSav;
    if ( 0 == (pSav = pLay->ContainsAny()) )
        return 0;

    if( pSav->IsInFtn() && !pLay->IsInFtn() )
    {
        do
            pSav = pSav->FindNext();
        while( pSav && pSav->IsInFtn() );
        if( !pSav || !pLay->IsAnLower( pSav ) )
            return NULL;
    }

    
    
    if ( pSav->IsInTab() && !( ( pLay->IsSctFrm() || pLay->IsCellFrm() ) && pLay->IsInTab() ) )
        while ( !pSav->IsTabFrm() )
            pSav = pSav->GetUpper();

    if( pSav->IsInSct() )
    { 
        SwFrm* pSect = pLay->FindSctFrm();
        SwFrm *pTmp = pSav;
        do
        {
            pSav = pTmp;
            pTmp = pSav->GetUpper() ? pSav->GetUpper()->FindSctFrm() : NULL;
        } while ( pTmp != pSect );
    }

    SwFrm *pFloat = pSav;
    if( !pStart )
        pStart = pSav;
    bool bGo = pStart == pSav;
    do
    {
        if( bGo )
            pFloat->GetUpper()->pLower = 0; 

        
        do
        {
            if( bGo )
            {
                if ( pFloat->IsCntntFrm() )
                {
                    if ( pFloat->GetDrawObjs() )
                        ::lcl_RemoveObjsFromPage( (SwCntntFrm*)pFloat );
                }
                else if ( pFloat->IsTabFrm() || pFloat->IsSctFrm() )
                {
                    SwCntntFrm *pCnt = ((SwLayoutFrm*)pFloat)->ContainsCntnt();
                    if( pCnt )
                    {
                        do
                        {   if ( pCnt->GetDrawObjs() )
                                ::lcl_RemoveObjsFromPage( pCnt );
                            pCnt = pCnt->GetNextCntntFrm();
                        } while ( pCnt && ((SwLayoutFrm*)pFloat)->IsAnLower( pCnt ) );
                    }
                }
                else {
                    OSL_ENSURE( !pFloat, "new FloatFrame?" );
                }
            }
            if ( pFloat->GetNext()  )
            {
                if( bGo )
                    pFloat->mpUpper = NULL;
                pFloat = pFloat->GetNext();
                if( !bGo && pFloat == pStart )
                {
                    bGo = true;
                    pFloat->mpPrev->mpNext = NULL;
                    pFloat->mpPrev = NULL;
                }
            }
            else
                break;

        } while ( pFloat );

        
        SwFrm *pTmp = pFloat->FindNext();
        if( bGo )
            pFloat->mpUpper = NULL;

        if( !pLay->IsInFtn() )
            while( pTmp && pTmp->IsInFtn() )
                pTmp = pTmp->FindNext();

        if ( !pLay->IsAnLower( pTmp ) )
            pTmp = 0;

        if ( pTmp && bGo )
        {
            pFloat->mpNext = pTmp; 
            pFloat->mpNext->mpPrev = pFloat;
        }
        pFloat = pTmp;
        bGo = bGo || ( pStart == pFloat );
    }  while ( pFloat );

    return bGo ? pStart : NULL;
}



static void lcl_AddObjsToPage( SwFrm* _pFrm, SwPageFrm* _pPage )
{
    OSL_ENSURE( _pFrm->GetDrawObjs(), "no DrawObjs in lcl_AddObjsToPage." );
    SwSortedObjs &rObjs = *_pFrm->GetDrawObjs();
    for ( sal_uInt16 i = 0; i < rObjs.Count(); ++i )
    {
        SwAnchoredObject* pObj = rObjs[i];

        
        
        pObj->UnlockPosition();
        
        
        if ( pObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm* pFlyFrm = static_cast<SwFlyFrm*>(pObj);
            if ( pObj->ISA(SwFlyFreeFrm) )
            {
                _pPage->AppendFlyToPage( pFlyFrm );
            }
            pFlyFrm->_InvalidatePos();
            pFlyFrm->_InvalidateSize();
            pFlyFrm->InvalidatePage( _pPage );

            
            
            if ( pFlyFrm->GetDrawObjs() )
            {
                ::lcl_AddObjsToPage( pFlyFrm, _pPage );
            }

            SwCntntFrm *pCnt = pFlyFrm->ContainsCntnt();
            while ( pCnt )
            {
                if ( pCnt->GetDrawObjs() )
                    ::lcl_AddObjsToPage( pCnt, _pPage );
                pCnt = pCnt->GetNextCntntFrm();
            }
        }
        
        else if ( pObj->ISA(SwAnchoredDrawObject) )
        {
            if (pObj->GetFrmFmt().GetAnchor().GetAnchorId() != FLY_AS_CHAR)
            {
                pObj->InvalidateObjPos();
                _pPage->AppendDrawObjToPage(
                                *(static_cast<SwAnchoredDrawObject*>(pObj)) );
            }
        }
    }
}

void RestoreCntnt( SwFrm *pSav, SwLayoutFrm *pParent, SwFrm *pSibling, bool bGrow )
{
    OSL_ENSURE( pSav && pParent, "no Save or Parent provided for RestoreCntnt." );
    SWRECTFN( pParent )

    
    
    

    SwPageFrm *pPage = pParent->FindPageFrm();

    if ( pPage )
        pPage->InvalidatePage( pPage );

    
    pSav->mpPrev = pSibling;
    SwFrm* pNxt;
    if ( pSibling )
    {
        pNxt = pSibling->mpNext;
        pSibling->mpNext = pSav;
        pSibling->_InvalidatePrt();
        ((SwCntntFrm*)pSibling)->InvalidatePage( pPage );
        if ( ((SwCntntFrm*)pSibling)->GetFollow() )
            pSibling->Prepare( PREP_CLEAR, 0, sal_False );
    }
    else
    {   pNxt = pParent->pLower;
        pParent->pLower = pSav;
        pSav->mpUpper = pParent; 

        if ( pSav->IsCntntFrm() )
            ((SwCntntFrm*)pSav)->InvalidatePage( pPage );
        else
        {   
            SwCntntFrm* pCnt = pParent->ContainsCntnt();
            if( pCnt )
                pCnt->InvalidatePage( pPage );
        }
    }

    
    SwTwips nGrowVal = 0;
    SwFrm* pLast;
    do
    {   pSav->mpUpper = pParent;
        nGrowVal += (pSav->Frm().*fnRect->fnGetHeight)();
        pSav->_InvalidateAll();

        
        if ( pSav->IsCntntFrm() )
        {
            if ( pSav->IsTxtFrm() &&
                 ((SwTxtFrm*)pSav)->GetCacheIdx() != USHRT_MAX )
                ((SwTxtFrm*)pSav)->Init();  

            if ( pPage && pSav->GetDrawObjs() )
                ::lcl_AddObjsToPage( (SwCntntFrm*)pSav, pPage );
        }
        else
        {   SwCntntFrm *pBlub = ((SwLayoutFrm*)pSav)->ContainsCntnt();
            if( pBlub )
            {
                do
                {   if ( pPage && pBlub->GetDrawObjs() )
                        ::lcl_AddObjsToPage( pBlub, pPage );
                    if( pBlub->IsTxtFrm() && ((SwTxtFrm*)pBlub)->HasFtn() &&
                         ((SwTxtFrm*)pBlub)->GetCacheIdx() != USHRT_MAX )
                        ((SwTxtFrm*)pBlub)->Init(); 
                    pBlub = pBlub->GetNextCntntFrm();
                } while ( pBlub && ((SwLayoutFrm*)pSav)->IsAnLower( pBlub ));
            }
        }
        pLast = pSav;
        pSav = pSav->GetNext();

    } while ( pSav );

    if( pNxt )
    {
        pLast->mpNext = pNxt;
        pNxt->mpPrev = pLast;
    }

    if ( bGrow )
        pParent->Grow( nGrowVal );
}

SwPageFrm * InsertNewPage( SwPageDesc &rDesc, SwFrm *pUpper,
                          bool bOdd, bool bFirst, bool bInsertEmpty, sal_Bool bFtn,
                          SwFrm *pSibling )
{
    SwPageFrm *pRet;
    SwDoc *pDoc = ((SwLayoutFrm*)pUpper)->GetFmt()->GetDoc();
    if (bFirst)
    {
        if (rDesc.IsFirstShared())
        {
            
            
            if (bOdd)
            {
                rDesc.GetFirstMaster().SetFmtAttr( rDesc.GetMaster().GetHeader() );
                rDesc.GetFirstMaster().SetFmtAttr( rDesc.GetMaster().GetFooter() );
                
                rDesc.GetFirstMaster().SetFmtAttr( rDesc.GetMaster().GetLRSpace() );
            }
            else
            {
                rDesc.GetFirstLeft().SetFmtAttr( rDesc.GetLeft().GetHeader() );
                rDesc.GetFirstLeft().SetFmtAttr( rDesc.GetLeft().GetFooter() );
                rDesc.GetFirstLeft().SetFmtAttr( rDesc.GetLeft().GetLRSpace() );
            }
        }
    }
    SwFrmFmt *pFmt(bOdd ? rDesc.GetRightFmt(bFirst) : rDesc.GetLeftFmt(bFirst));
    
    if ( !pFmt )
    {
        pFmt = bOdd ? rDesc.GetLeftFmt() : rDesc.GetRightFmt();
        OSL_ENSURE( pFmt, "Descriptor without any format?!" );
        bInsertEmpty = !bInsertEmpty;
    }
    if( bInsertEmpty )
    {
        SwPageDesc *pTmpDesc = pSibling && pSibling->GetPrev() ?
                ((SwPageFrm*)pSibling->GetPrev())->GetPageDesc() : &rDesc;
        pRet = new SwPageFrm( pDoc->GetEmptyPageFmt(), pUpper, pTmpDesc );
        pRet->Paste( pUpper, pSibling );
        pRet->PreparePage( bFtn );
    }
    pRet = new SwPageFrm( pFmt, pUpper, &rDesc );
    pRet->Paste( pUpper, pSibling );
    pRet->PreparePage( bFtn );
    if ( pRet->GetNext() )
        ((SwRootFrm*)pRet->GetUpper())->AssertPageFlys( pRet );
    return pRet;
}

/* The following two methods search the layout structure recursively and
 * register all Flys at the page that have a Frm in this structure as an anchor.
 */

static void lcl_Regist( SwPageFrm *pPage, const SwFrm *pAnch )
{
    SwSortedObjs *pObjs = (SwSortedObjs*)pAnch->GetDrawObjs();
    for ( sal_uInt16 i = 0; i < pObjs->Count(); ++i )
    {
        SwAnchoredObject* pObj = (*pObjs)[i];
        if ( pObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pObj);
            
            
            SwPageFrm *pPg = pFly->IsFlyFreeFrm()
                             ? pFly->GetPageFrm() : pFly->FindPageFrm();
            if ( pPg != pPage )
            {
                if ( pPg )
                    pPg->RemoveFlyFromPage( pFly );
                pPage->AppendFlyToPage( pFly );
            }
            ::RegistFlys( pPage, pFly );
        }
        else
        {
            
            if ( pPage != pObj->GetPageFrm() )
            {
                
                if ( pObj->GetPageFrm() )
                    pObj->GetPageFrm()->RemoveDrawObjFromPage( *pObj );
                pPage->AppendDrawObjToPage( *pObj );
            }
        }

        const SwFlyFrm* pFly = pAnch->FindFlyFrm();
        if ( pFly &&
             pObj->GetDrawObj()->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() &&
             pObj->GetDrawObj()->GetPage() )
        {
            
            pObj->DrawObj()->GetPage()->SetObjectOrdNum( pFly->GetVirtDrawObj()->GetOrdNumDirect(),
                                                         pObj->GetDrawObj()->GetOrdNumDirect() );
        }
    }
}

void RegistFlys( SwPageFrm *pPage, const SwLayoutFrm *pLay )
{
    if ( pLay->GetDrawObjs() )
        ::lcl_Regist( pPage, pLay );
    const SwFrm *pFrm = pLay->Lower();
    while ( pFrm )
    {
        if ( pFrm->IsLayoutFrm() )
            ::RegistFlys( pPage, (const SwLayoutFrm*)pFrm );
        else if ( pFrm->GetDrawObjs() )
            ::lcl_Regist( pPage, pFrm );
        pFrm = pFrm->GetNext();
    }
}


void Notify( SwFlyFrm *pFly, SwPageFrm *pOld, const SwRect &rOld,
             const SwRect* pOldPrt )
{
    const SwRect aFrm( pFly->GetObjRectWithSpaces() );
    if ( rOld.Pos() != aFrm.Pos() )
    {   
        if ( rOld.HasArea() &&
             rOld.Left()+pFly->GetFmt()->GetLRSpace().GetLeft() < FAR_AWAY )
        {
            pFly->NotifyBackground( pOld, rOld, PREP_FLY_LEAVE );
        }
        pFly->NotifyBackground( pFly->FindPageFrm(), aFrm, PREP_FLY_ARRIVE );
    }
    else if ( rOld.SSize() != aFrm.SSize() )
    {   
        

        SwViewShell *pSh = pFly->getRootFrm()->GetCurrShell();
        if( pSh && rOld.HasArea() )
            pSh->InvalidateWindows( rOld );

        
        
        SwPageFrm* pPageFrm = pFly->FindPageFrm();
        if ( pOld != pPageFrm )
        {
            pFly->NotifyBackground( pPageFrm, aFrm, PREP_FLY_ARRIVE );
        }

        if ( rOld.Left() != aFrm.Left() )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Left(  std::min(aFrm.Left(), rOld.Left()) );
            aTmp.Right( std::max(aFrm.Left(), rOld.Left()) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        SwTwips nOld = rOld.Right();
        SwTwips nNew = aFrm.Right();
        if ( nOld != nNew )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Left(  std::min(nNew, nOld) );
            aTmp.Right( std::max(nNew, nOld) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        if ( rOld.Top() != aFrm.Top() )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Top(    std::min(aFrm.Top(), rOld.Top()) );
            aTmp.Bottom( std::max(aFrm.Top(), rOld.Top()) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        nOld = rOld.Bottom();
        nNew = aFrm.Bottom();
        if ( nOld != nNew )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Top(    std::min(nNew, nOld) );
            aTmp.Bottom( std::max(nNew, nOld) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
    }
    else if ( pOldPrt && *pOldPrt != pFly->Prt() &&
              pFly->GetFmt()->GetSurround().IsContour() )
    {
        
        pFly->NotifyBackground( pFly->FindPageFrm(), aFrm, PREP_FLY_ARRIVE );
    }
}

static void lcl_CheckFlowBack( SwFrm* pFrm, const SwRect &rRect )
{
    SwTwips nBottom = rRect.Bottom();
    while( pFrm )
    {
        if( pFrm->IsLayoutFrm() )
        {
            if( rRect.IsOver( pFrm->Frm() ) )
                lcl_CheckFlowBack( ((SwLayoutFrm*)pFrm)->Lower(), rRect );
        }
        else if( !pFrm->GetNext() && nBottom > pFrm->Frm().Bottom() )
        {
            if( pFrm->IsCntntFrm() && ((SwCntntFrm*)pFrm)->HasFollow() )
                pFrm->InvalidateSize();
            else
                pFrm->InvalidateNextPos();
        }
        pFrm = pFrm->GetNext();
    }
}

static void lcl_NotifyCntnt( const SdrObject *pThis, SwCntntFrm *pCnt,
    const SwRect &rRect, const PrepareHint eHint )
{
    if ( pCnt->IsTxtFrm() )
    {
        SwRect aCntPrt( pCnt->Prt() );
        aCntPrt.Pos() += pCnt->Frm().Pos();
        if ( eHint == PREP_FLY_ATTR_CHG )
        {
            
            
            if ( aCntPrt.IsOver( rRect ) )
                pCnt->Prepare( PREP_FLY_ATTR_CHG );
        }
        
        
        else if ( aCntPrt.IsOver( rRect ) )
            pCnt->Prepare( eHint, (void*)&aCntPrt._Intersection( rRect ) );
        if ( pCnt->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
            for ( sal_uInt16 i = 0; i < rObjs.Count(); ++i )
            {
                SwAnchoredObject* pObj = rObjs[i];
                if ( pObj->ISA(SwFlyFrm) )
                {
                    SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pObj);
                    if ( pFly->IsFlyInCntFrm() )
                    {
                        SwCntntFrm *pCntnt = pFly->ContainsCntnt();
                        while ( pCntnt )
                        {
                            ::lcl_NotifyCntnt( pThis, pCntnt, rRect, eHint );
                            pCntnt = pCntnt->GetNextCntntFrm();
                        }
                    }
                }
            }
        }
    }
}

void Notify_Background( const SdrObject* pObj,
                        SwPageFrm* pPage,
                        const SwRect& rRect,
                        const PrepareHint eHint,
                        const sal_Bool bInva )
{
    
    if ( eHint == PREP_FLY_LEAVE && rRect.Top() == FAR_AWAY )
         return;

    SwLayoutFrm* pArea;
    SwFlyFrm *pFlyFrm = 0;
    SwFrm* pAnchor;
    if( pObj->ISA(SwVirtFlyDrawObj) )
    {
        pFlyFrm = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
        pAnchor = pFlyFrm->AnchorFrm();
    }
    else
    {
        pFlyFrm = NULL;
        pAnchor = const_cast<SwFrm*>(
                    GetUserCall(pObj)->GetAnchoredObj( pObj )->GetAnchorFrm() );
    }
    if( PREP_FLY_LEAVE != eHint && pAnchor->IsInFly() )
        pArea = pAnchor->FindFlyFrm();
    else
        pArea = pPage;
    SwCntntFrm *pCnt = 0;
    if ( pArea )
    {
        if( PREP_FLY_ARRIVE != eHint )
            lcl_CheckFlowBack( pArea, rRect );

        
        
        
        
        
        
        
        
        
        
        
        {
            pCnt = pArea->ContainsCntnt();
        }
    }
    SwFrm *pLastTab = 0;

    while ( pCnt && pArea && pArea->IsAnLower( pCnt ) )
    {
        ::lcl_NotifyCntnt( pObj, pCnt, rRect, eHint );
        if ( pCnt->IsInTab() )
        {
            SwLayoutFrm* pCell = pCnt->GetUpper();
            
            
            
            if ( pCell->IsCellFrm() &&
                 ( pCell->Frm().IsOver( pObj->GetLastBoundRect() ) ||
                   pCell->Frm().IsOver( rRect ) ) )
            {
                const SwFmtVertOrient &rOri = pCell->GetFmt()->GetVertOrient();
                if ( text::VertOrientation::NONE != rOri.GetVertOrient() )
                    pCell->InvalidatePrt();
            }
            SwTabFrm *pTab = pCnt->FindTabFrm();
            if ( pTab != pLastTab )
            {
                pLastTab = pTab;
                
                
                
                if ( pTab->Frm().IsOver( pObj->GetLastBoundRect() ) ||
                     pTab->Frm().IsOver( rRect ) )
                {
                    if ( !pFlyFrm || !pFlyFrm->IsLowerOf( pTab ) )
                        pTab->InvalidatePrt();
                }
            }
        }
        pCnt = pCnt->GetNextCntntFrm();
    }
    
    if ( pPage && pPage->GetSortedObjs() )
    {
        pObj->GetOrdNum();
        const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
        for ( sal_uInt16 i = 0; i < rObjs.Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = rObjs[i];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                if( pAnchoredObj->GetDrawObj() == pObj )
                    continue;
                SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                if ( pFly->Frm().Top() == FAR_AWAY )
                    continue;

                if ( !pFlyFrm ||
                        (!pFly->IsLowerOf( pFlyFrm ) &&
                        pFly->GetVirtDrawObj()->GetOrdNumDirect() < pObj->GetOrdNumDirect()))
                {
                    pCnt = pFly->ContainsCntnt();
                    while ( pCnt )
                    {
                        ::lcl_NotifyCntnt( pObj, pCnt, rRect, eHint );
                        pCnt = pCnt->GetNextCntntFrm();
                    }
                }
                if( pFly->IsFlyLayFrm() )
                {
                    if( pFly->Lower() && pFly->Lower()->IsColumnFrm() &&
                        pFly->Frm().Bottom() >= rRect.Top() &&
                        pFly->Frm().Top() <= rRect.Bottom() &&
                        pFly->Frm().Right() >= rRect.Left() &&
                        pFly->Frm().Left() <= rRect.Right() )
                     {
                        pFly->InvalidateSize();
                     }
                }
                
                
                
                else if ( pFly->IsFlyAtCntFrm() &&
                        pObj->GetOrdNumDirect() <
                        pFly->GetVirtDrawObj()->GetOrdNumDirect() &&
                        pFlyFrm && !pFly->IsLowerOf( pFlyFrm ) )
                {
                    const SwFmtHoriOrient &rH = pFly->GetFmt()->GetHoriOrient();
                    if ( text::HoriOrientation::NONE != rH.GetHoriOrient()  &&
                            text::HoriOrientation::CENTER != rH.GetHoriOrient()  &&
                            ( !pFly->IsAutoPos() || text::RelOrientation::CHAR != rH.GetRelationOrient() ) &&
                            (pFly->Frm().Bottom() >= rRect.Top() &&
                            pFly->Frm().Top() <= rRect.Bottom()) )
                        pFly->InvalidatePos();
                }
            }
        }
    }
    if ( pFlyFrm && pAnchor->GetUpper() && pAnchor->IsInTab() )
        pAnchor->GetUpper()->InvalidateSize();

    
    SwViewShell* pSh = 0;
    if ( bInva && pPage &&
        0 != (pSh = pPage->getRootFrm()->GetCurrShell()) )
    {
        pSh->InvalidateWindows( rRect );
    }
}



const SwFrm* GetVirtualUpper( const SwFrm* pFrm, const Point& rPos )
{
    if( pFrm->IsTxtFrm() )
    {
        pFrm = pFrm->GetUpper();
        if( !pFrm->Frm().IsInside( rPos ) )
        {
            if( pFrm->IsFtnFrm() )
            {
                const SwFtnFrm* pTmp = ((SwFtnFrm*)pFrm)->GetFollow();
                while( pTmp )
                {
                    if( pTmp->Frm().IsInside( rPos ) )
                        return pTmp;
                    pTmp = pTmp->GetFollow();
                }
            }
            else
            {
                SwFlyFrm* pTmp = (SwFlyFrm*)pFrm->FindFlyFrm();
                while( pTmp )
                {
                    if( pTmp->Frm().IsInside( rPos ) )
                        return pTmp;
                    pTmp = pTmp->GetNextLink();
                }
            }
        }
    }
    return pFrm;
}

bool Is_Lower_Of( const SwFrm *pCurrFrm, const SdrObject* pObj )
{
    Point aPos;
    const SwFrm* pFrm;
    if( pObj->ISA(SwVirtFlyDrawObj) )
    {
        const SwFlyFrm* pFly = ( (SwVirtFlyDrawObj*)pObj )->GetFlyFrm();
        pFrm = pFly->GetAnchorFrm();
        aPos = pFly->Frm().Pos();
    }
    else
    {
        pFrm = ( (SwDrawContact*)GetUserCall(pObj) )->GetAnchorFrm(pObj);
        aPos = pObj->GetCurrentBoundRect().TopLeft();
    }
    OSL_ENSURE( pFrm, "8-( Fly is lost in Space." );
    pFrm = GetVirtualUpper( pFrm, aPos );
    do
    {   if ( pFrm == pCurrFrm )
            return true;
        if( pFrm->IsFlyFrm() )
        {
            aPos = pFrm->Frm().Pos();
            pFrm = GetVirtualUpper( ((const SwFlyFrm*)pFrm)->GetAnchorFrm(), aPos );
        }
        else
            pFrm = pFrm->GetUpper();
    } while ( pFrm );
    return false;
}


const SwFrm *FindKontext( const SwFrm *pFrm, sal_uInt16 nAdditionalKontextTyp )
{
    const sal_uInt16 nTyp = FRM_ROOT | FRM_HEADER   | FRM_FOOTER | FRM_FTNCONT  |
                        FRM_FTN  | FRM_FLY      |
                        FRM_TAB  | FRM_ROW      | FRM_CELL |
                        nAdditionalKontextTyp;
    do
    {   if ( pFrm->GetType() & nTyp )
            break;
        pFrm = pFrm->GetUpper();
    } while( pFrm );
    return pFrm;
}

bool IsFrmInSameKontext( const SwFrm *pInnerFrm, const SwFrm *pFrm )
{
    const SwFrm *pKontext = FindKontext( pInnerFrm, 0 );

    const sal_uInt16 nTyp = FRM_ROOT | FRM_HEADER   | FRM_FOOTER | FRM_FTNCONT  |
                        FRM_FTN  | FRM_FLY      |
                        FRM_TAB  | FRM_ROW      | FRM_CELL;
    do
    {   if ( pFrm->GetType() & nTyp )
        {
            if( pFrm == pKontext )
                return true;
            if( pFrm->IsCellFrm() )
                return false;
        }
        if( pFrm->IsFlyFrm() )
        {
            Point aPos( pFrm->Frm().Pos() );
            pFrm = GetVirtualUpper( ((const SwFlyFrm*)pFrm)->GetAnchorFrm(), aPos );
        }
        else
            pFrm = pFrm->GetUpper();
    } while( pFrm );

    return false;
}

static SwTwips lcl_CalcCellRstHeight( SwLayoutFrm *pCell )
{
    if ( pCell->Lower()->IsCntntFrm() || pCell->Lower()->IsSctFrm() )
    {
        SwFrm *pLow = pCell->Lower();
        long nHeight = 0, nFlyAdd = 0;
        do
        {
            long nLow = pLow->Frm().Height();
            if( pLow->IsTxtFrm() && ((SwTxtFrm*)pLow)->IsUndersized() )
                nLow += ((SwTxtFrm*)pLow)->GetParHeight()-pLow->Prt().Height();
            else if( pLow->IsSctFrm() && ((SwSectionFrm*)pLow)->IsUndersized() )
                nLow += ((SwSectionFrm*)pLow)->Undersize();
            nFlyAdd = std::max( 0L, nFlyAdd - nLow );
            nFlyAdd = std::max( nFlyAdd, ::CalcHeightWidthFlys( pLow ) );
            nHeight += nLow;
            pLow = pLow->GetNext();
        } while ( pLow );
        if ( nFlyAdd )
            nHeight += nFlyAdd;

        
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pCell );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        nHeight += rAttrs.CalcTop() + rAttrs.CalcBottom();

        return pCell->Frm().Height() - nHeight;
    }
    else
    {
        long nRstHeight = 0;
        SwFrm *pLow = pCell->Lower();
        do
        {   nRstHeight += ::CalcRowRstHeight( (SwLayoutFrm*)pLow );
            pLow = pLow->GetNext();

        } while ( pLow );

        return nRstHeight;
    }
}

SwTwips CalcRowRstHeight( SwLayoutFrm *pRow )
{
    SwTwips nRstHeight = LONG_MAX;
    SwLayoutFrm *pLow = (SwLayoutFrm*)pRow->Lower();
    while ( pLow )
    {
        nRstHeight = std::min( nRstHeight, ::lcl_CalcCellRstHeight( pLow ) );
        pLow = (SwLayoutFrm*)pLow->GetNext();
    }
    return nRstHeight;
}

const SwFrm* FindPage( const SwRect &rRect, const SwFrm *pPage )
{
    if ( !rRect.IsOver( pPage->Frm() ) )
    {
        const SwRootFrm* pRootFrm = static_cast<const SwRootFrm*>(pPage->GetUpper());
        const SwFrm* pTmpPage = pRootFrm ? pRootFrm->GetPageAtPos( rRect.TopLeft(), &rRect.SSize(), true ) : 0;
        if ( pTmpPage )
            pPage = pTmpPage;
    }

    return pPage;
}

class SwFrmHolder : private SfxListener
{
    SwFrm* pFrm;
    bool bSet;
    virtual void Notify(  SfxBroadcaster& rBC, const SfxHint& rHint );
public:
    SwFrmHolder() : pFrm(0), bSet(false) {}
    void SetFrm( SwFrm* pHold );
    SwFrm* GetFrm() { return pFrm; }
    void Reset();
    bool IsSet() { return bSet; }
};

void SwFrmHolder::SetFrm( SwFrm* pHold )
{
    bSet = true;
    pFrm = pHold;
    StartListening(*pHold);
}

void SwFrmHolder::Reset()
{
    if (pFrm)
        EndListening(*pFrm);
    bSet = false;
    pFrm = 0;
}

void SwFrmHolder::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        if ( ( (SfxSimpleHint&) rHint ).GetId() == SFX_HINT_DYING && &rBC == pFrm )
            pFrm = 0;
    }
}

SwFrm* GetFrmOfModify( const SwRootFrm* pLayout, SwModify const& rMod, sal_uInt16 const nFrmType,
        const Point* pPoint, const SwPosition *pPos, const sal_Bool bCalcFrm )
{
    SwFrm *pMinFrm = 0, *pTmpFrm;
    SwFrmHolder aHolder;
    SwRect aCalcRect;
    bool bClientIterChanged = false;

    SwIterator<SwFrm,SwModify> aIter( rMod );
    do {
        pMinFrm = 0;
        aHolder.Reset();
        sal_uInt64 nMinDist = 0;
        bClientIterChanged = false;

        for( pTmpFrm = aIter.First(); pTmpFrm; pTmpFrm = aIter.Next() )
        {
            if( pTmpFrm->GetType() & nFrmType &&
                ( !pLayout || pLayout == pTmpFrm->getRootFrm() ) &&
                (!pTmpFrm->IsFlowFrm() ||
                 !SwFlowFrm::CastFlowFrm( pTmpFrm )->IsFollow() ))
            {
                if( pPoint )
                {
                    
                    if ( pMinFrm )
                        aHolder.SetFrm( pMinFrm );
                    else
                        aHolder.Reset();

                    if( bCalcFrm )
                    {
                        
                        
                        
                        SwFlyFrm* pFlyFrm( pTmpFrm->FindFlyFrm() );
                        if ( pFlyFrm &&
                             pFlyFrm->Frm().Pos().X() == FAR_AWAY &&
                             pFlyFrm->Frm().Pos().Y() == FAR_AWAY )
                        {
                            SwObjectFormatter::FormatObj( *pFlyFrm );
                        }
                        pTmpFrm->Calc();
                    }

                    
                    
                    
                    
                    if( aIter.IsChanged() || ( aHolder.IsSet() && !aHolder.GetFrm() ) )
                    {
                        
                        bClientIterChanged = true;
                        break;
                    }

                    
                    if( !bCalcFrm && nFrmType & FRM_FLY &&
                        ((SwFlyFrm*)pTmpFrm)->GetAnchorFrm() &&
                        FAR_AWAY == pTmpFrm->Frm().Pos().getX() &&
                        FAR_AWAY == pTmpFrm->Frm().Pos().getY() )
                        aCalcRect = ((SwFlyFrm*)pTmpFrm)->GetAnchorFrm()->Frm();
                    else
                        aCalcRect = pTmpFrm->Frm();

                    if ( aCalcRect.IsInside( *pPoint ) )
                    {
                        pMinFrm = pTmpFrm;
                        break;
                    }

                    
                    const Point aCalcRectCenter = aCalcRect.Center();
                    const Point aDiff = aCalcRectCenter - *pPoint;
                    const sal_uInt64 nCurrentDist = aDiff.getX() * aDiff.getX() + aDiff.getY() * aDiff.getY(); 
                    if ( !pMinFrm || nCurrentDist < nMinDist )
                    {
                        pMinFrm = pTmpFrm;
                        nMinDist = nCurrentDist;
                    }
                }
                else
                {
                    
                    pMinFrm = pTmpFrm;
                    break;
                }
            }
        }
    } while( bClientIterChanged );

    if( pPos && pMinFrm && pMinFrm->IsTxtFrm() )
        return ((SwTxtFrm*)pMinFrm)->GetFrmAtPos( *pPos );

    return pMinFrm;
}

bool IsExtraData( const SwDoc *pDoc )
{
    const SwLineNumberInfo &rInf = pDoc->GetLineNumberInfo();
    return rInf.IsPaintLineNumbers() ||
           rInf.IsCountInFlys() ||
           ((sal_Int16)SW_MOD()->GetRedlineMarkPos() != text::HoriOrientation::NONE &&
            !pDoc->GetRedlineTbl().empty());
}


const SwRect SwPageFrm::PrtWithoutHeaderAndFooter() const
{
    SwRect aPrtWithoutHeaderFooter( Prt() );
    aPrtWithoutHeaderFooter.Pos() += Frm().Pos();

    const SwFrm* pLowerFrm = Lower();
    while ( pLowerFrm )
    {
        
        
        if ( pLowerFrm->IsHeaderFrm() )
        {
            aPrtWithoutHeaderFooter.Top( aPrtWithoutHeaderFooter.Top() +
                                         pLowerFrm->Frm().Height() );
        }
        if ( pLowerFrm->IsFooterFrm() )
        {
            aPrtWithoutHeaderFooter.Bottom( aPrtWithoutHeaderFooter.Bottom() -
                                            pLowerFrm->Frm().Height() );
        }

        pLowerFrm = pLowerFrm->GetNext();
    }

    return aPrtWithoutHeaderFooter;
}

/** method to determine the spacing values of a frame

    OD 2004-03-10 #i28701#
    OD 2009-08-28 #i102458#
    Add output parameter <obIsLineSpacingProportional>
*/
void GetSpacingValuesOfFrm( const SwFrm& rFrm,
                            SwTwips& onLowerSpacing,
                            SwTwips& onLineSpacing,
                            bool& obIsLineSpacingProportional )
{
    if ( !rFrm.IsFlowFrm() )
    {
        onLowerSpacing = 0;
        onLineSpacing = 0;
    }
    else
    {
        const SvxULSpaceItem& rULSpace = rFrm.GetAttrSet()->GetULSpace();
        onLowerSpacing = rULSpace.GetLower();

        onLineSpacing = 0;
        obIsLineSpacingProportional = false;
        if ( rFrm.IsTxtFrm() )
        {
            onLineSpacing = static_cast<const SwTxtFrm&>(rFrm).GetLineSpace();
            obIsLineSpacingProportional =
                onLineSpacing != 0 &&
                static_cast<const SwTxtFrm&>(rFrm).GetLineSpace( true ) == 0;
        }

        OSL_ENSURE( onLowerSpacing >= 0 && onLineSpacing >= 0,
                "<GetSpacingValuesOfFrm(..)> - spacing values aren't positive!" );
    }
}


const SwCntntFrm* GetCellCntnt( const SwLayoutFrm& rCell )
{
    const SwCntntFrm* pCntnt = rCell.ContainsCntnt();
    const SwTabFrm* pTab = rCell.FindTabFrm();

    while ( pCntnt && rCell.IsAnLower( pCntnt ) )
    {
        const SwTabFrm* pTmpTab = pCntnt->FindTabFrm();
        if ( pTmpTab != pTab )
        {
            pCntnt = pTmpTab->FindLastCntnt();
            if ( pCntnt )

                pCntnt = pCntnt->FindNextCnt();

        }
        else
            break;
    }
    return pCntnt;
}


bool SwDeletionChecker::HasBeenDeleted()
{
    if ( !mpFrm || !mpRegIn )
        return false;

    SwIterator<SwFrm,SwModify> aIter(*mpRegIn);
    SwFrm* pLast = aIter.First();
    while ( pLast )
    {
        if ( pLast == mpFrm )
            return false;
        pLast = aIter.Next();
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
