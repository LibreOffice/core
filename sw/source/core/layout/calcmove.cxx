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

#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "viewopt.hxx"
#include "frmtool.hxx"
#include "txtftn.hxx"
#include "fmtftn.hxx"
#include <editeng/ulspitem.hxx>
#include <editeng/keepitem.hxx>

#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <fmtclbl.hxx>

#include "tabfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "sectfrm.hxx"
#include "dbg_lay.hxx"


#include <sortedobjs.hxx>
#include <layouter.hxx>

#include <flyfrms.hxx>




sal_Bool SwCntntFrm::ShouldBwdMoved( SwLayoutFrm *pNewUpper, sal_Bool, sal_Bool & )
{
    if ( (SwFlowFrm::IsMoveBwdJump() || !IsPrevObjMove()))
    {
        
        
        
        
        
        
        
        
        
        
        
        

        

        
        
        
        sal_uInt8 nMoveAnyway = 0;
        SwPageFrm * const pNewPage = pNewUpper->FindPageFrm();
        SwPageFrm *pOldPage = FindPageFrm();

        if ( SwFlowFrm::IsMoveBwdJump() )
            return sal_True;

        if( IsInFtn() && IsInSct() )
        {
            SwFtnFrm* pFtn = FindFtnFrm();
            SwSectionFrm* pMySect = pFtn->FindSctFrm();
            if( pMySect && pMySect->IsFtnLock() )
            {
                SwSectionFrm *pSect = pNewUpper->FindSctFrm();
                while( pSect && pSect->IsInFtn() )
                    pSect = pSect->GetUpper()->FindSctFrm();
                OSL_ENSURE( pSect, "Escaping footnote" );
                if( pSect != pMySect )
                    return sal_False;
            }
        }
        SWRECTFN( this )
        SWRECTFNX( pNewUpper )
        if( std::abs( (pNewUpper->Prt().*fnRectX->fnGetWidth)() -
                 (GetUpper()->Prt().*fnRect->fnGetWidth)() ) > 1 ) {
            
            nMoveAnyway = 2;
        }

        
        
        nMoveAnyway |= BwdMoveNecessary( pOldPage, Frm() );
        {
            const IDocumentSettingAccess* pIDSA = pNewPage->GetFmt()->getIDocumentSettingAccess();
            SwTwips nSpace = 0;
            SwRect aRect( pNewUpper->Prt() );
            aRect.Pos() += pNewUpper->Frm().Pos();
            const SwFrm *pPrevFrm = pNewUpper->Lower();
            while ( pPrevFrm )
            {
                SwTwips nNewTop = (pPrevFrm->Frm().*fnRectX->fnGetBottom)();
                
                
                {
                    
                    
                    if ( !pPrevFrm->GetNext() && pPrevFrm->IsInTab() &&
                         pIDSA->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS) )
                    {
                        const SwFrm* pLastFrm = pPrevFrm;
                        
                        if ( pPrevFrm->IsSctFrm() )
                        {
                            pLastFrm = static_cast<const SwSectionFrm*>(pPrevFrm)->FindLastCntnt();
                            if ( pLastFrm &&
                                 pLastFrm->FindTabFrm() != pPrevFrm->FindTabFrm() )
                            {
                                pLastFrm = pLastFrm->FindTabFrm();
                            }
                        }

                        if ( pLastFrm )
                        {
                            SwBorderAttrAccess aAccess( SwFrm::GetCache(), pLastFrm );
                            const SwBorderAttrs& rAttrs = *aAccess.Get();
                            nNewTop -= rAttrs.GetULSpace().GetLower();
                        }
                    }
                }
                (aRect.*fnRectX->fnSetTop)( nNewTop );

                pPrevFrm = pPrevFrm->GetNext();
            }

            nMoveAnyway |= BwdMoveNecessary( pNewPage, aRect);

            
            nSpace = (aRect.*fnRectX->fnGetHeight)();
            const SwViewShell *pSh = pNewUpper->getRootFrm()->GetCurrShell();
            if ( IsInFtn() ||
                 (pSh && pSh->GetViewOptions()->getBrowseMode()) ||
                 pNewUpper->IsCellFrm() ||
                 ( pNewUpper->IsInSct() && ( pNewUpper->IsSctFrm() ||
                   ( pNewUpper->IsColBodyFrm() &&
                     !pNewUpper->GetUpper()->GetPrev() &&
                     !pNewUpper->GetUpper()->GetNext() ) ) ) )
                nSpace += pNewUpper->Grow( LONG_MAX, sal_True );

            if ( nMoveAnyway < 3 )
            {
                if ( nSpace )
                {
                    
                    
                    
                    

                    
                    

                    
                    
                    
                    
                    const sal_uInt8 nBwdMoveNecessaryResult =
                                            BwdMoveNecessary( pNewPage, aRect);
                    const bool bObjsInNewUpper( nBwdMoveNecessaryResult == 2 ||
                                                nBwdMoveNecessaryResult == 3 );

                    return _WouldFit( nSpace, pNewUpper, nMoveAnyway == 2,
                                      bObjsInNewUpper );
                }
                
                
                
                else if( pNewUpper->IsInSct() && pNewUpper->IsColBodyFrm() &&
                    !(pNewUpper->Prt().*fnRectX->fnGetWidth)() &&
                    ( pNewUpper->GetUpper()->GetPrev() ||
                      pNewUpper->GetUpper()->GetNext() ) )
                    return sal_True;
                else
                    return sal_False; 
            }
            else
            {
                
                if ( nSpace )
                    return sal_True;
                else
                    return sal_False;
            }
        }
    }
    return  sal_False;
}




inline void PrepareLock( SwFlowFrm *pTab )
{
    pTab->LockJoin();
}
inline void PrepareUnlock( SwFlowFrm *pTab )
{
    pTab->UnlockJoin();

}


static bool lcl_IsCalcUpperAllowed( const SwFrm& rFrm )
{
    return !rFrm.GetUpper()->IsSctFrm() &&
           !rFrm.GetUpper()->IsFooterFrm() &&
           
           !rFrm.GetUpper()->IsFlyFrm() &&
           !( rFrm.GetUpper()->IsTabFrm() && rFrm.GetUpper()->GetUpper()->IsInTab() ) &&
           !( rFrm.IsTabFrm() && rFrm.GetUpper()->IsInTab() );
}

/** Prepares the Frame for "formatting" (MakeAll()).
 *
 * This method serves to save stack space: To calculate the position of the Frm
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
void SwFrm::PrepareMake()
{
    StackHack aHack;
    if ( GetUpper() )
    {
        if ( lcl_IsCalcUpperAllowed( *this ) )
            GetUpper()->Calc();
        OSL_ENSURE( GetUpper(), ":-( Layout unstable (Upper gone)." );
        if ( !GetUpper() )
            return;

        const bool bCnt = IsCntntFrm();
        const bool bTab = IsTabFrm();
        sal_Bool bNoSect = IsInSct();
        sal_Bool bOldTabLock = sal_False, bFoll = sal_False;
        SwFlowFrm* pThis = bCnt ? (SwCntntFrm*)this : NULL;

        if ( bTab )
        {
            pThis = (SwTabFrm*)this;
            bOldTabLock = ((SwTabFrm*)this)->IsJoinLocked();
            ::PrepareLock( (SwTabFrm*)this );
            bFoll = pThis->IsFollow();
        }
        else if( IsSctFrm() )
        {
            pThis = (SwSectionFrm*)this;
            bFoll = pThis->IsFollow();
            bNoSect = sal_False;
        }
        else if ( bCnt && sal_True == (bFoll = pThis->IsFollow()) &&
             GetPrev() )
        {
            
            const SwTxtFrm* pMaster = ((SwCntntFrm*)this)->FindMaster();
            if ( pMaster && pMaster->IsLocked() )
            {
                MakeAll();
                return;
            }
        }

        
        
        const bool bFormatPrev = !bTab ||
                                 !GetPrev() ||
                                 !GetPrev()->GetAttrSet()->GetKeep().GetValue();
        if ( bFormatPrev )
        {
            SwFrm *pFrm = GetUpper()->Lower();
            while ( pFrm != this )
            {
                OSL_ENSURE( pFrm, ":-( Layout unstable (this not found)." );
                if ( !pFrm )
                    return; 

                if ( !pFrm->IsValid() )
                {
                    
                    
                    
                    
                    if ( bFoll && pFrm->IsFlowFrm() &&
                         (SwFlowFrm::CastFlowFrm(pFrm))->IsAnFollow( pThis ) )
                        break;

                    pFrm->MakeAll();
                    if( IsSctFrm() && !((SwSectionFrm*)this)->GetSection() )
                        break;
                }
                
                
                
                
                pFrm = pFrm->FindNext();

                
                
                
                
                if( bNoSect && pFrm && pFrm->IsSctFrm() )
                {
                    SwFrm* pCnt = ((SwSectionFrm*)pFrm)->ContainsAny();
                    if( pCnt )
                        pFrm = pCnt;
                }
            }
            OSL_ENSURE( GetUpper(), "Layout unstable (Upper gone II)." );
            if ( !GetUpper() )
                return;

            if ( lcl_IsCalcUpperAllowed( *this ) )
                GetUpper()->Calc();

            OSL_ENSURE( GetUpper(), "Layout unstable (Upper gone III)." );
        }

        if ( bTab && !bOldTabLock )
            ::PrepareUnlock( (SwTabFrm*)this );
    }
    MakeAll();
}

void SwFrm::OptPrepareMake()
{
    
    if ( GetUpper() && !GetUpper()->IsFooterFrm() &&
         !GetUpper()->IsFlyFrm() )
    {
        GetUpper()->Calc();
        OSL_ENSURE( GetUpper(), ":-( Layout unstable (Upper gone)." );
        if ( !GetUpper() )
            return;
    }
    if ( GetPrev() && !GetPrev()->IsValid() )
        PrepareMake();
    else
    {
        StackHack aHack;
        MakeAll();
    }
}

void SwFrm::PrepareCrsr()
{
    StackHack aHack;
    if( GetUpper() && !GetUpper()->IsSctFrm() )
    {
        GetUpper()->PrepareCrsr();
        GetUpper()->Calc();

        OSL_ENSURE( GetUpper(), ":-( Layout unstable (Upper gone)." );
        if ( !GetUpper() )
            return;

        const bool bCnt = IsCntntFrm();
        const bool bTab = IsTabFrm();
        sal_Bool bNoSect = IsInSct();

        sal_Bool bOldTabLock = sal_False, bFoll;
        SwFlowFrm* pThis = bCnt ? (SwCntntFrm*)this : NULL;

        if ( bTab )
        {
            bOldTabLock = ((SwTabFrm*)this)->IsJoinLocked();
            ::PrepareLock( (SwTabFrm*)this );
            pThis = (SwTabFrm*)this;
        }
        else if( IsSctFrm() )
        {
            pThis = (SwSectionFrm*)this;
            bNoSect = sal_False;
        }
        bFoll = pThis && pThis->IsFollow();

        SwFrm *pFrm = GetUpper()->Lower();
        while ( pFrm != this )
        {
            OSL_ENSURE( pFrm, ":-( Layout unstable (this not found)." );
            if ( !pFrm )
                return; 

            if ( !pFrm->IsValid() )
            {
                
                
                
                
                if ( bFoll && pFrm->IsFlowFrm() &&
                     (SwFlowFrm::CastFlowFrm(pFrm))->IsAnFollow( pThis ) )
                    break;

                pFrm->MakeAll();
            }
            
            
            
            
            pFrm = pFrm->FindNext();
            if( bNoSect && pFrm && pFrm->IsSctFrm() )
            {
                SwFrm* pCnt = ((SwSectionFrm*)pFrm)->ContainsAny();
                if( pCnt )
                    pFrm = pCnt;
            }
        }
        OSL_ENSURE( GetUpper(), "Layout unstable (Upper gone II)." );
        if ( !GetUpper() )
            return;

        GetUpper()->Calc();

        OSL_ENSURE( GetUpper(), "Layout unstable (Upper gone III)." );

        if ( bTab && !bOldTabLock )
            ::PrepareUnlock( (SwTabFrm*)this );
    }
    Calc();
}


static SwFrm* lcl_Prev( SwFrm* pFrm, bool bSectPrv = true )
{
    SwFrm* pRet = pFrm->GetPrev();
    if( !pRet && pFrm->GetUpper() && pFrm->GetUpper()->IsSctFrm() &&
        bSectPrv && !pFrm->IsColumnFrm() )
        pRet = pFrm->GetUpper()->GetPrev();
    while( pRet && pRet->IsSctFrm() &&
           !((SwSectionFrm*)pRet)->GetSection() )
        pRet = pRet->GetPrev();
    return pRet;
}

static SwFrm* lcl_NotHiddenPrev( SwFrm* pFrm )
{
    SwFrm *pRet = pFrm;
    do
    {
        pRet = lcl_Prev( pRet );
    } while ( pRet && pRet->IsTxtFrm() && ((SwTxtFrm*)pRet)->IsHiddenNow() );
    return pRet;
}

void SwFrm::MakePos()
{
    if ( !mbValidPos )
    {
        mbValidPos = sal_True;
        bool bUseUpper = false;
        SwFrm* pPrv = lcl_Prev( this );
        if ( pPrv &&
             ( !pPrv->IsCntntFrm() ||
               ( ((SwCntntFrm*)pPrv)->GetFollow() != this ) )
           )
        {
            if ( !StackHack::IsLocked() &&
                 ( !IsInSct() || IsSctFrm() ) &&
                 !pPrv->IsSctFrm() &&
                 !pPrv->GetAttrSet()->GetKeep().GetValue()
               )
            {
                pPrv->Calc();   
            }
            else if ( pPrv->Frm().Top() == 0 )
            {
                bUseUpper = true;
            }
        }

        pPrv = lcl_Prev( this, false );
        sal_uInt16 nMyType = GetType();
        SWRECTFN( ( IsCellFrm() && GetUpper() ? GetUpper() : this  ) )
        if ( !bUseUpper && pPrv )
        {
            maFrm.Pos( pPrv->Frm().Pos() );
            if( FRM_NEIGHBOUR & nMyType )
            {
                sal_Bool bR2L = IsRightToLeft();
                if( bR2L )
                    (maFrm.*fnRect->fnSetPosX)( (maFrm.*fnRect->fnGetLeft)() -
                                               (maFrm.*fnRect->fnGetWidth)() );
                else
                    (maFrm.*fnRect->fnSetPosX)( (maFrm.*fnRect->fnGetLeft)() +
                                          (pPrv->Frm().*fnRect->fnGetWidth)() );

                
                if( bVert && FRM_CELL & nMyType && !mbReverse )
                    maFrm.Pos().setX(maFrm.Pos().getX() - maFrm.Width() + pPrv->Frm().Width());
            }
            else if( bVert && FRM_NOTE_VERT & nMyType )
            {
                if( mbReverse )
                    maFrm.Pos().setX(maFrm.Pos().getX() + pPrv->Frm().Width());
                else
                    
                {
                    if ( bVertL2R )
                           maFrm.Pos().setX(maFrm.Pos().getX() + pPrv->Frm().Width());
                    else
                           maFrm.Pos().setX(maFrm.Pos().getX() - maFrm.Width());
                  }
            }
            else
                maFrm.Pos().setY(maFrm.Pos().getY() + pPrv->Frm().Height());
        }
        else if ( GetUpper() )
        {
            
            
            
            
            
            
            
            
            
            if ( !GetUpper()->IsTabFrm() &&
                 !( IsTabFrm() && GetUpper()->IsInTab() ) &&
                 !GetUpper()->IsSctFrm() &&
                 !dynamic_cast<SwFlyAtCntFrm*>(GetUpper()) &&
                 !( GetUpper()->IsFooterFrm() &&
                    GetUpper()->IsColLocked() )
               )
            {
                GetUpper()->Calc();
            }
            pPrv = lcl_Prev( this, false );
            if ( !bUseUpper && pPrv )
            {
                maFrm.Pos( pPrv->Frm().Pos() );
                if( FRM_NEIGHBOUR & nMyType )
                {
                    sal_Bool bR2L = IsRightToLeft();
                    if( bR2L )
                        (maFrm.*fnRect->fnSetPosX)( (maFrm.*fnRect->fnGetLeft)() -
                                                 (maFrm.*fnRect->fnGetWidth)() );
                    else
                        (maFrm.*fnRect->fnSetPosX)( (maFrm.*fnRect->fnGetLeft)() +
                                          (pPrv->Frm().*fnRect->fnGetWidth)() );

                    
                    if( bVert && FRM_CELL & nMyType && !mbReverse )
                        maFrm.Pos().setX(maFrm.Pos().getX() - maFrm.Width() + pPrv->Frm().Width());
                }
                else if( bVert && FRM_NOTE_VERT & nMyType )
                {
                    if( mbReverse )
                        maFrm.Pos().setX(maFrm.Pos().getX() + pPrv->Frm().Width());
                    else
                        maFrm.Pos().setX(maFrm.Pos().getX() - maFrm.Width());
                }
                else
                    maFrm.Pos().setY(maFrm.Pos().getY() + pPrv->Frm().Height());
            }
            else
            {
                maFrm.Pos( GetUpper()->Frm().Pos() );
                maFrm.Pos() += GetUpper()->Prt().Pos();
                if( FRM_NEIGHBOUR & nMyType && IsRightToLeft() )
                {
                    if( bVert )
                        maFrm.Pos().setY(maFrm.Pos().getY() + GetUpper()->Prt().Height()
                                          - maFrm.Height());
                    else
                        maFrm.Pos().setX(maFrm.Pos().getX() + GetUpper()->Prt().Width()
                                          - maFrm.Width());
                }
                
                else if( bVert && !bVertL2R && FRM_NOTE_VERT & nMyType && !mbReverse )
                    maFrm.Pos().setX(maFrm.Pos().getX() - maFrm.Width() + GetUpper()->Prt().Width());
            }
        }
        else
        {
            maFrm.Pos().setX(0);
            maFrm.Pos().setY(0);
        }
        
        if( IsBodyFrm() && bVert && !bVertL2R && !mbReverse && GetUpper() )
            maFrm.Pos().setX(maFrm.Pos().getX() + GetUpper()->Prt().Width() - maFrm.Width());
        mbValidPos = sal_True;
    }
}


static void lcl_CheckObjects( SwSortedObjs* pSortedObjs, SwFrm* pFrm, long& rBot )
{
    
    long nMax = 0;
    for ( sal_uInt16 i = 0; i < pSortedObjs->Count(); ++i )
    {
        
        
        SwAnchoredObject* pObj = (*pSortedObjs)[i];
        long nTmp = 0;
        if ( pObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pObj);
            if( pFly->Frm().Top() != FAR_AWAY &&
                ( pFrm->IsPageFrm() ? pFly->IsFlyLayFrm() :
                  ( pFly->IsFlyAtCntFrm() &&
                    ( pFrm->IsBodyFrm() ? pFly->GetAnchorFrm()->IsInDocBody() :
                                          pFly->GetAnchorFrm()->IsInFtn() ) ) ) )
            {
                nTmp = pFly->Frm().Bottom();
            }
        }
        else
            nTmp = pObj->GetObjRect().Bottom();
        nMax = std::max( nTmp, nMax );
    }
    ++nMax; 
    rBot = std::max( rBot, nMax );
}

void SwPageFrm::MakeAll()
{
    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

    const SwRect aOldRect( Frm() );     
    const SwLayNotify aNotify( this );  
    SwBorderAttrAccess *pAccess = 0;
    const SwBorderAttrs*pAttrs = 0;

    while ( !mbValidPos || !mbValidSize || !mbValidPrtArea )
    {
        if ( !mbValidPos )
        {
            mbValidPos = sal_True; 
        }

        if ( !mbValidSize || !mbValidPrtArea )
        {
            if ( IsEmptyPage() )
            {
                Frm().Width( 0 );  Prt().Width( 0 );
                Frm().Height( 0 ); Prt().Height( 0 );
                Prt().Left( 0 );   Prt().Top( 0 );
                mbValidSize = mbValidPrtArea = sal_True;
            }
            else
            {
                if ( !pAccess )
                {
                    pAccess = new SwBorderAttrAccess( SwFrm::GetCache(), this );
                    pAttrs = pAccess->Get();
                }
                
                SwViewShell *pSh = getRootFrm()->GetCurrShell();
                if ( pSh && pSh->GetViewOptions()->getBrowseMode() )
                {
                    const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
                    const long nTop    = pAttrs->CalcTopLine()   + aBorder.Height();
                    const long nBottom = pAttrs->CalcBottomLine()+ aBorder.Height();

                    long nWidth = GetUpper() ? ((SwRootFrm*)GetUpper())->GetBrowseWidth() : 0;
                    if ( nWidth < pSh->GetBrowseWidth() )
                        nWidth = pSh->GetBrowseWidth();
                    nWidth += + 2 * aBorder.Width();

                    nWidth = std::max( nWidth, 2L * aBorder.Width() + 4L*MM50 );
                    Frm().Width( nWidth );

                    SwLayoutFrm *pBody = FindBodyCont();
                    if ( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
                    {
                        
                        Frm().Height( pAttrs->GetSize().Height() );
                    }
                    else
                    {
                        
                        long nBot = Frm().Top() + nTop;
                        SwFrm *pFrm = Lower();
                        while ( pFrm )
                        {
                            long nTmp = 0;
                            SwFrm *pCnt = ((SwLayoutFrm*)pFrm)->ContainsAny();
                            while ( pCnt && (pCnt->GetUpper() == pFrm ||
                                             ((SwLayoutFrm*)pFrm)->IsAnLower( pCnt )))
                            {
                                nTmp += pCnt->Frm().Height();
                                if( pCnt->IsTxtFrm() &&
                                    ((SwTxtFrm*)pCnt)->IsUndersized() )
                                    nTmp += ((SwTxtFrm*)pCnt)->GetParHeight()
                                            - pCnt->Prt().Height();
                                else if( pCnt->IsSctFrm() &&
                                         ((SwSectionFrm*)pCnt)->IsUndersized() )
                                    nTmp += ((SwSectionFrm*)pCnt)->Undersize();
                                pCnt = pCnt->FindNext();
                            }
                            
                            if ( pFrm->IsBodyFrm() &&
                                 ( !pFrm->GetValidSizeFlag() ||
                                   !pFrm->GetValidPrtAreaFlag() ) &&
                                 ( pFrm->Frm().Height() < pFrm->Prt().Height() )
                               )
                            {
                                nTmp = std::min( nTmp, pFrm->Frm().Height() );
                            }
                            else
                            {
                                
                                OSL_ENSURE( !(pFrm->Frm().Height() < pFrm->Prt().Height()),
                                        "SwPageFrm::MakeAll(): Lower with frame height < printing height" );
                                nTmp += pFrm->Frm().Height() - pFrm->Prt().Height();
                            }
                            if ( !pFrm->IsBodyFrm() )
                                nTmp = std::min( nTmp, pFrm->Frm().Height() );
                            nBot += nTmp;
                            
                            
                            if( pSortedObjs && !pFrm->IsHeaderFrm() &&
                                !pFrm->IsFooterFrm() )
                                lcl_CheckObjects( pSortedObjs, pFrm, nBot );
                            pFrm = pFrm->GetNext();
                        }
                        nBot += nBottom;
                        
                        if ( pSortedObjs )
                            lcl_CheckObjects( pSortedObjs, this, nBot );
                        nBot -= Frm().Top();
                        
                        
                        
                        if ( !GetPrev() && !GetNext() )
                        {
                            nBot = std::max( nBot, pSh->VisArea().Height() );
                        }
                        
                        
                        Frm().Height( std::min( nBot, BROWSE_HEIGHT ) );
                    }
                    Prt().Left ( pAttrs->CalcLeftLine() + aBorder.Width() );
                    Prt().Top  ( nTop );
                    Prt().Width( Frm().Width() - ( Prt().Left()
                        + pAttrs->CalcRightLine() + aBorder.Width() ) );
                    Prt().Height( Frm().Height() - (nTop + nBottom) );
                    mbValidSize = mbValidPrtArea = sal_True;
                }
                else
                {   
                    
                    Frm().SSize( pAttrs->GetSize() );
                    Format( pAttrs );
                }
            }
        }
    } 
    delete pAccess;

    if ( Frm() != aOldRect && GetUpper() )
        static_cast<SwRootFrm*>(GetUpper())->CheckViewLayout( 0, 0 );

    OSL_ENSURE( !GetUpper() || GetUpper()->Prt().Width() >= maFrm.Width(),
        "Upper (Root) must be wide enough to contain the widest page");
}

void SwLayoutFrm::MakeAll()
{
    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

    
    const SwLayNotify aNotify( this );
    bool bVert = IsVertical();
    
    SwRectFn fnRect = ( IsNeighbourFrm() == bVert )? fnRectHori : ( IsVertLR() ? fnRectVertL2R : fnRectVert );

    SwBorderAttrAccess *pAccess = 0;
    const SwBorderAttrs*pAttrs = 0;

    while ( !mbValidPos || !mbValidSize || !mbValidPrtArea )
    {
        if ( !mbValidPos )
            MakePos();

        if ( GetUpper() )
        {
            
            if ( IsLeaveUpperAllowed() )
            {
                if ( !mbValidSize )
                    mbValidPrtArea = sal_False;
            }
            else
            {
                if ( !mbValidSize )
                {
                    
                    mbValidPrtArea = sal_False;

                    SwTwips nPrtWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
                    if( bVert && ( IsBodyFrm() || IsFtnContFrm() ) )
                    {
                        SwFrm* pNxt = GetPrev();
                        while( pNxt && !pNxt->IsHeaderFrm() )
                            pNxt = pNxt->GetPrev();
                        if( pNxt )
                            nPrtWidth -= pNxt->Frm().Height();
                        pNxt = GetNext();
                        while( pNxt && !pNxt->IsFooterFrm() )
                            pNxt = pNxt->GetNext();
                        if( pNxt )
                            nPrtWidth -= pNxt->Frm().Height();
                    }

                    const long nDiff = nPrtWidth - (Frm().*fnRect->fnGetWidth)();

                    if( IsNeighbourFrm() && IsRightToLeft() )
                        (Frm().*fnRect->fnSubLeft)( nDiff );
                    else
                        (Frm().*fnRect->fnAddRight)( nDiff );
                }
                else
                {
                    
                    const SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
                    if( (Frm().*fnRect->fnOverStep)( nDeadLine ) )
                        mbValidSize = sal_False;
                }
            }
        }
        if ( !mbValidSize || !mbValidPrtArea )
        {
            if ( !pAccess )
            {
                pAccess = new SwBorderAttrAccess( SwFrm::GetCache(), this );
                pAttrs  = pAccess->Get();
            }
            Format( pAttrs );
        }
    } 
    delete pAccess;
}

bool SwTxtNode::IsCollapse() const
{
    if (GetDoc()->get( IDocumentSettingAccess::COLLAPSE_EMPTY_CELL_PARA )
        &&  GetTxt().isEmpty())
    {
        sal_uLong nIdx=GetIndex();
        const SwEndNode *pNdBefore=GetNodes()[nIdx-1]->GetEndNode();
        const SwEndNode *pNdAfter=GetNodes()[nIdx+1]->GetEndNode();

        
        bool bInTable = this->FindTableNode( ) != NULL;

        SwSortedObjs* pObjs = this->getLayoutFrm( GetDoc()->GetCurrentLayout() )->GetDrawObjs( );
        sal_uInt32 nObjs = ( pObjs != NULL ) ? pObjs->Count( ) : 0;

        if ( pNdBefore!=NULL && pNdAfter!=NULL && nObjs == 0 && bInTable ) {
            return true;
        } else {
            return false;
        }
    } else
        return false;
}

bool SwFrm::IsCollapse() const
{
    if (IsTxtFrm()) {
        const SwTxtFrm *pTxtFrm=(SwTxtFrm*)this;
        const SwTxtNode *pTxtNode=pTxtFrm->GetTxtNode();
        if (pTxtNode && pTxtNode->IsCollapse()) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

sal_Bool SwCntntFrm::MakePrtArea( const SwBorderAttrs &rAttrs )
{
    sal_Bool bSizeChgd = sal_False;

    if ( !mbValidPrtArea )
    {
        mbValidPrtArea = sal_True;

        SWRECTFN( this )
        const bool bTxtFrm = IsTxtFrm();
        SwTwips nUpper = 0;
        if ( bTxtFrm && ((SwTxtFrm*)this)->IsHiddenNow() )
        {
            if ( ((SwTxtFrm*)this)->HasFollow() )
                ((SwTxtFrm*)this)->JoinFrm();

            if( (Prt().*fnRect->fnGetHeight)() )
                ((SwTxtFrm*)this)->HideHidden();
            Prt().Pos().setX(0);
            Prt().Pos().setY(0);
            (Prt().*fnRect->fnSetWidth)( (Frm().*fnRect->fnGetWidth)() );
            (Prt().*fnRect->fnSetHeight)( 0 );
            nUpper = -( (Frm().*fnRect->fnGetHeight)() );
        }
        else
        {
            

            
            
            const long nLeft = rAttrs.CalcLeft( this );
            const long nRight = ((SwBorderAttrs&)rAttrs).CalcRight( this );
            (this->*fnRect->fnSetXMargins)( nLeft, nRight );

            SwViewShell *pSh = getRootFrm()->GetCurrShell();
            SwTwips nWidthArea;
            if( pSh && 0!=(nWidthArea=(pSh->VisArea().*fnRect->fnGetWidth)()) &&
                GetUpper()->IsPageBodyFrm() && 
                pSh->GetViewOptions()->getBrowseMode() )
            {
                
                
                
                long nMinWidth = 0;

                for (sal_uInt16 i = 0; GetDrawObjs() && i < GetDrawObjs()->Count();++i)
                {
                    
                    
                    SwAnchoredObject* pObj = (*GetDrawObjs())[i];
                    const SwFrmFmt& rFmt = pObj->GetFrmFmt();
                    const sal_Bool bFly = pObj->ISA(SwFlyFrm);
                    if ((bFly && (FAR_AWAY == pObj->GetObjRect().Width()))
                        || rFmt.GetFrmSize().GetWidthPercent())
                    {
                        continue;
                    }

                    if ( FLY_AS_CHAR == rFmt.GetAnchor().GetAnchorId() )
                    {
                        nMinWidth = std::max( nMinWidth,
                                         bFly ? rFmt.GetFrmSize().GetWidth()
                                              : pObj->GetObjRect().Width() );
                    }
                }

                const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
                long nWidth = nWidthArea - 2 * ( IsVertical() ? aBorder.Height() : aBorder.Width() );
                nWidth -= (Prt().*fnRect->fnGetLeft)();
                nWidth -= rAttrs.CalcRightLine();
                nWidth = std::max( nMinWidth, nWidth );
                (Prt().*fnRect->fnSetWidth)( std::min( nWidth,
                                            (Prt().*fnRect->fnGetWidth)() ) );
            }

            if ( (Prt().*fnRect->fnGetWidth)() <= MINLAY )
            {
                
                
                (Prt().*fnRect->fnSetWidth)( std::min( long(MINLAY),
                                             (Frm().*fnRect->fnGetWidth)() ) );
                SwTwips nTmp = (Frm().*fnRect->fnGetWidth)() -
                               (Prt().*fnRect->fnGetWidth)();
                if( (Prt().*fnRect->fnGetLeft)() > nTmp )
                    (Prt().*fnRect->fnSetLeft)( nTmp );
            }

            
            
            
            
            
            
            
            
            
            
            

            nUpper = CalcUpperSpace( &rAttrs, NULL );

            SwTwips nLower = CalcLowerSpace( &rAttrs );
            if (IsCollapse()) {
                nUpper=0;
                nLower=0;
            }

            (Prt().*fnRect->fnSetPosY)( (!bVert || mbReverse) ? nUpper : nLower);
            nUpper += nLower;
            nUpper -= (Frm().*fnRect->fnGetHeight)() -
                      (Prt().*fnRect->fnGetHeight)();
        }
        
        
        if ( nUpper )
        {
            if ( nUpper > 0 )
                GrowFrm( nUpper );
            else
                ShrinkFrm( -nUpper );
            bSizeChgd = sal_True;
        }
    }
    return bSizeChgd;
}

#define STOP_FLY_FORMAT 10

const int cnStopFormat = 15;

inline void ValidateSz( SwFrm *pFrm )
{
    if ( pFrm )
    {
        pFrm->mbValidSize = sal_True;
        pFrm->mbValidPrtArea = sal_True;
    }
}

void SwCntntFrm::MakeAll()
{
    OSL_ENSURE( GetUpper(), "no Upper?" );
    OSL_ENSURE( IsTxtFrm(), "MakeAll(), NoTxt" );

    if ( !IsFollow() && StackHack::IsLocked() )
        return;

    if ( IsJoinLocked() )
        return;

    OSL_ENSURE( !((SwTxtFrm*)this)->IsSwapped(), "Calculation of a swapped frame" );

    StackHack aHack;

    if ( ((SwTxtFrm*)this)->IsLocked() )
    {
        OSL_FAIL( "Format for locked TxtFrm." );
        return;
    }

    LockJoin();
    long nFormatCount = 0;
    
    int nConsequetiveFormatsWithoutChange = 0;
    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

#ifdef DBG_UTIL
    const SwDoc *pDoc = GetAttrSet()->GetDoc();
    if( pDoc )
    {
        static bool bWarned = false;
        if( pDoc->InXMLExport() )
        {
            SAL_WARN_IF( !bWarned, "sw", "Formatting during XML-export!" );
            bWarned = true;
        }
        else
            bWarned = false;
    }
#endif

    
    SwCntntNotify *pNotify = new SwCntntNotify( this );

    
    bool bMakePage = true;
    
    bool bMovedBwd = false;
    
    
    bool bMovedFwd  = false;
    sal_Bool    bFormatted  = sal_False;    
                                            
                                            
                                            
    bool bMustFit = false;                  
                                            
    bool bFitPromise = false;               
                                            
                                            
                                            
                                            
    bool bMoveable;
    const sal_Bool bFly = IsInFly();
    const sal_Bool bTab = IsInTab();
    const sal_Bool bFtn = IsInFtn();
    const sal_Bool bSct = IsInSct();
    Point aOldFrmPos;               
    Point aOldPrtPos;               

    SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    
    if ( !IsFollow() && rAttrs.JoinedWithPrev( *(this) ) )
    {
        pNotify->SetBordersJoinedWithPrev();
    }

    const sal_Bool bKeep = IsKeep( rAttrs.GetAttrSet() );

    SwSaveFtnHeight *pSaveFtn = 0;
    if ( bFtn )
    {
        SwFtnFrm *pFtn = FindFtnFrm();
        SwSectionFrm* pSct = pFtn->FindSctFrm();
        if ( !((SwTxtFrm*)pFtn->GetRef())->IsLocked() )
        {
            SwFtnBossFrm* pBoss = pFtn->GetRef()->FindFtnBossFrm(
                                    pFtn->GetAttr()->GetFtn().IsEndNote() );
            if( !pSct || pSct->IsColLocked() || !pSct->Growable() )
                pSaveFtn = new SwSaveFtnHeight( pBoss,
                    ((SwTxtFrm*)pFtn->GetRef())->GetFtnLine( pFtn->GetAttr() ) );
        }
    }

    //
    if ( GetUpper()->IsSctFrm() &&
         HasFollow() &&
         GetFollow()->GetFrm() == GetNext() )
    {
        dynamic_cast<SwTxtFrm*>(this)->JoinFrm();
    }

    
    
    if ( !static_cast<SwTxtFrm*>(this)->IsFollow() )
    {
        sal_uInt32 nToPageNum = 0L;
        const bool bMoveFwdByObjPos = SwLayouter::FrmMovedFwdByObjPos(
                                                    *(GetAttrSet()->GetDoc()),
                                                    *(static_cast<SwTxtFrm*>(this)),
                                                    nToPageNum );
        
        
        if ( bMoveFwdByObjPos &&
             FindPageFrm()->GetPhyPageNum() < nToPageNum &&
             ( lcl_Prev( this ) ||
               GetUpper()->IsCellFrm() ||
               ( GetUpper()->IsSctFrm() &&
                 GetUpper()->GetUpper()->IsCellFrm() ) ) &&
             IsMoveable() )
        {
            bMovedFwd = true;
            MoveFwd( bMakePage, sal_False );
        }
    }

    
    
    if ( lcl_Prev( this ) && ((SwTxtFrm*)this)->IsFollow() && IsMoveable() )
    {
        bMovedFwd = true;
        
        
        
        if ( IsInTab() )
        {
            lcl_Prev( this )->InvalidatePrt();
        }
        MoveFwd( bMakePage, sal_False );
    }

    
    
    
    if ( bFtn && !mbValidPos )
    {
        SwFtnFrm* pFtn = FindFtnFrm();
        SwCntntFrm* pRefCnt = pFtn ? pFtn->GetRef() : 0;
        if ( pRefCnt && !pRefCnt->IsValid() )
        {
            SwFtnBossFrm* pFtnBossOfFtn = pFtn->FindFtnBossFrm();
            SwFtnBossFrm* pFtnBossOfRef = pRefCnt->FindFtnBossFrm();
            
            if ( pFtnBossOfFtn && pFtnBossOfRef &&
                 pFtnBossOfFtn != pFtnBossOfRef &&
                 pFtnBossOfFtn->IsBefore( pFtnBossOfRef ) )
            {
                bMovedFwd = true;
                MoveFwd( bMakePage, sal_False );
            }
        }
    }

    SWRECTFN( this )

    while ( !mbValidPos || !mbValidSize || !mbValidPrtArea )
    {
        
        SwRect aOldFrm_StopFormat( Frm() );
        SwRect aOldPrt_StopFormat( Prt() );
        if ( (bMoveable = IsMoveable()) )
        {
            SwFrm *pPre = GetIndPrev();
            if ( CheckMoveFwd( bMakePage, bKeep, bMovedBwd ) )
            {
                SWREFRESHFN( this )
                bMovedFwd = true;
                if ( bMovedBwd )
                {
                    
                    
                    
                    GetUpper()->ResetCompletePaint();
                    
                    OSL_ENSURE( pPre, "missing old Prev" );
                    if( !pPre->IsSctFrm() )
                        ::ValidateSz( pPre );
                }
                bMoveable = IsMoveable();
            }
        }

        aOldFrmPos = (Frm().*fnRect->fnGetPos)();
        aOldPrtPos = (Prt().*fnRect->fnGetPos)();

        if ( !mbValidPos )
            MakePos();

        
        if ( !mbValidSize )
        {
            
            
            
            
            
            
            
            
            
            
            
            
            const SwTwips nNewFrmWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            if ( mbValidPrtArea && nNewFrmWidth > 0 &&
                 (Frm().*fnRect->fnGetWidth)() == 0 &&
                 (Prt().*fnRect->fnGetWidth)() == 0 )
            {
                mbValidPrtArea = sal_False;
            }

            (Frm().*fnRect->fnSetWidth)( nNewFrmWidth );
        }
        if ( !mbValidPrtArea )
        {
            const long nOldW = (Prt().*fnRect->fnGetWidth)();
            
            const SwTwips nOldH = (Frm().*fnRect->fnGetHeight)();
            MakePrtArea( rAttrs );
            if ( nOldW != (Prt().*fnRect->fnGetWidth)() )
                Prepare( PREP_FIXSIZE_CHG );
            
            
            
            
            
            
            if ( mbValidSize && !IsUndersized() &&
                 nOldH != (Frm().*fnRect->fnGetHeight)() )
            {
                
                
                
                
                const SwTwips nHDiff = nOldH - (Frm().*fnRect->fnGetHeight)();
                const bool bNoPrepAdjustFrm =
                    nHDiff > 0 && IsInTab() && GetFollow() &&
                    ( 1 == static_cast<SwTxtFrm*>(GetFollow())->GetLineCount( COMPLETE_STRING ) || (static_cast<SwTxtFrm*>(GetFollow())->Frm().*fnRect->fnGetWidth)() < 0 ) &&
                    GetFollow()->CalcAddLowerSpaceAsLastInTableCell() == nHDiff;
                if ( !bNoPrepAdjustFrm )
                {
                    Prepare( PREP_ADJUST_FRM );
                    mbValidSize = sal_False;
                }
            }
        }

        
        
        
        
        if ( !bMustFit )
        {
            sal_Bool bWidow = sal_True;
            const SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
            if ( bMoveable && !bFormatted && ( GetFollow() ||
                 ( (Frm().*fnRect->fnOverStep)( nDeadLine ) ) ) )
            {
                Prepare( PREP_WIDOWS_ORPHANS, 0, sal_False );
                mbValidSize = bWidow = sal_False;
            }
            if( (Frm().*fnRect->fnGetPos)() != aOldFrmPos ||
                (Prt().*fnRect->fnGetPos)() != aOldPrtPos )
            {
                
                
                Prepare( PREP_POS_CHGD, (const void*)&bFormatted, sal_False );
                if ( bWidow && GetFollow() )
                {   Prepare( PREP_WIDOWS_ORPHANS, 0, sal_False );
                    mbValidSize = sal_False;
                }
            }
        }
        if ( !mbValidSize )
        {
            mbValidSize = bFormatted = sal_True;
            ++nFormatCount;
            if( nFormatCount > STOP_FLY_FORMAT )
                SetFlyLock( sal_True );
            
            
            
            if ( nConsequetiveFormatsWithoutChange <= cnStopFormat )
            {
                Format();
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OSL_FAIL( "debug assertion: <SwCntntFrm::MakeAll()> - format of text frame suppressed by fix b6448963" );
            }
#endif
        }

        
        
        
        
        sal_Bool bDummy;
        if ( !lcl_Prev( this ) &&
             !bMovedFwd &&
             ( bMoveable || ( bFly && !bTab ) ) &&
             ( !bFtn || !GetUpper()->FindFtnFrm()->GetPrev() )
             && MoveBwd( bDummy ) )
        {
            SWREFRESHFN( this )
            bMovedBwd = true;
            bFormatted = sal_False;
            if ( bKeep && bMoveable )
            {
                if( CheckMoveFwd( bMakePage, sal_False, bMovedBwd ) )
                {
                    bMovedFwd = true;
                    bMoveable = IsMoveable();
                    SWREFRESHFN( this )
                }
                Point aOldPos = (Frm().*fnRect->fnGetPos)();
                MakePos();
                if( aOldPos != (Frm().*fnRect->fnGetPos)() )
                {
                    Prepare( PREP_POS_CHGD, (const void*)&bFormatted, sal_False );
                    if ( !mbValidSize )
                    {
                        (Frm().*fnRect->fnSetWidth)( (GetUpper()->
                                                Prt().*fnRect->fnGetWidth)() );
                        if ( !mbValidPrtArea )
                        {
                            const long nOldW = (Prt().*fnRect->fnGetWidth)();
                            MakePrtArea( rAttrs );
                            if( nOldW != (Prt().*fnRect->fnGetWidth)() )
                                Prepare( PREP_FIXSIZE_CHG, 0, sal_False );
                        }
                        if( GetFollow() )
                            Prepare( PREP_WIDOWS_ORPHANS, 0, sal_False );
                        mbValidSize = bFormatted = sal_True;
                        Format();
                    }
                }
                SwFrm *pNxt = HasFollow() ? NULL : FindNext();
                while( pNxt && pNxt->IsSctFrm() )
                {   
                    if( ((SwSectionFrm*)pNxt)->GetSection() )
                    {
                        SwFrm* pTmp = ((SwSectionFrm*)pNxt)->ContainsAny();
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
                    pNxt->Calc();
                    if( mbValidPos && !GetIndNext() )
                    {
                        SwSectionFrm *pSct = FindSctFrm();
                        if( pSct && !pSct->GetValidSizeFlag() )
                        {
                            SwSectionFrm* pNxtSct = pNxt->FindSctFrm();
                            if( pNxtSct && pSct->IsAnFollow( pNxtSct ) )
                                mbValidPos = sal_False;
                        }
                        else
                            mbValidPos = sal_False;
                    }
                }
            }
        }

        
        
        if ( mbValidPos )
        {
            
            
            
            
            
            
            
            if ( bFtn &&
                 nFormatCount <= STOP_FLY_FORMAT &&
                 !GetDrawObjs() )
            {
                mbValidPos = sal_False;
                MakePos();
                aOldFrmPos = (Frm().*fnRect->fnGetPos)();
                aOldPrtPos = (Prt().*fnRect->fnGetPos)();
            }
        }

        
        {
            if ( aOldFrm_StopFormat == Frm() &&
                 aOldPrt_StopFormat == Prt() )
            {
                ++nConsequetiveFormatsWithoutChange;
            }
            else
            {
                nConsequetiveFormatsWithoutChange = 0;
            }
        }

        
        if ( !mbValidPos || !mbValidSize || !mbValidPrtArea )
            continue;

        
        
        
        
        const long nPrtBottom = (GetUpper()->*fnRect->fnGetPrtBottom)();
        const long nBottomDist =  (Frm().*fnRect->fnBottomDist)( nPrtBottom );
        if( nBottomDist >= 0 )
        {
            if ( bKeep && bMoveable )
            {
                
                
                
                
                //
                
                
                
                SwFrm *pNxt = HasFollow() ? NULL : FindNext();
                
                
                while( pNxt && pNxt->IsSctFrm() )
                {
                    if( ((SwSectionFrm*)pNxt)->GetSection() )
                    {
                        pNxt = ((SwSectionFrm*)pNxt)->ContainsAny();
                        break;
                    }
                    pNxt = pNxt->FindNext();
                }
                if ( pNxt )
                {
                    const bool bMoveFwdInvalid = 0 != GetIndNext();
                    const bool bNxtNew =
                        ( 0 == (pNxt->Prt().*fnRect->fnGetHeight)() ) &&
                        (!pNxt->IsTxtFrm() ||!((SwTxtFrm*)pNxt)->IsHiddenNow());

                    pNxt->Calc();

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

        
        

        
        
        
        if ( !bMoveable || IsUndersized() )
        {
            if( !bMoveable && IsInTab() )
            {
                long nDiff = -(Frm().*fnRect->fnBottomDist)(
                                        (GetUpper()->*fnRect->fnGetPrtBottom)() );
                long nReal = GetUpper()->Grow( nDiff );
                if( nReal )
                    continue;
            }
            break;
        }

        
        
        
        
        
        //
        
        
        
        bool bMoveOrFit = false;
        bool bDontMoveMe = !GetIndPrev();
        if( bDontMoveMe && IsInSct() )
        {
            SwFtnBossFrm* pBoss = FindFtnBossFrm();
            bDontMoveMe = !pBoss->IsInSct() ||
                          ( !pBoss->Lower()->GetNext() && !pBoss->GetPrev() );
        }

        
        
        if( bDontMoveMe && IsInTab() &&
            0 != const_cast<SwCntntFrm*>(this)->GetNextCellLeaf( MAKEPAGE_NONE ) )
            bDontMoveMe = false;

        if ( bDontMoveMe && (Frm().*fnRect->fnGetHeight)() >
                            (GetUpper()->Prt().*fnRect->fnGetHeight)() )
        {
            if ( !bFitPromise )
            {
                SwTwips nTmp = (GetUpper()->Prt().*fnRect->fnGetHeight)() -
                               (Prt().*fnRect->fnGetTop)();
                sal_Bool bSplit = !IsFwdMoveAllowed();
                if ( nTmp > 0 && WouldFit( nTmp, bSplit, sal_False ) )
                {
                    Prepare( PREP_WIDOWS_ORPHANS, 0, sal_False );
                    mbValidSize = sal_False;
                    bFitPromise = true;
                    continue;
                }
                /* --------------------------------------------------
                 * In earlier days, we never tried to fit TextFrms in
                 * frames and sections using bMoveOrFit by ignoring
                 * its attributes (Widows, Keep).
                 * This should have been done at least for column frames;
                 * as it must be tried anyway with linked frames and sections.
                 * Exception: If we sit in FormatWidthCols, we must not ignore
                 * the attributes.
                 * --------------------------------------------------*/
                else if ( !bFtn && bMoveable &&
                      ( !bFly || !FindFlyFrm()->IsColLocked() ) &&
                      ( !bSct || !FindSctFrm()->IsColLocked() ) )
                    bMoveOrFit = true;
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OSL_FAIL( "+TxtFrm didn't respect WouldFit promise." );
            }
#endif
        }

        
        
        SwFrm *pPre = GetIndPrev();
        SwFrm *pOldUp = GetUpper();

/* MA 13. Oct. 98: What is this supposed to be!?
 * AMA 14. Dec 98: If a column section can't find any space for its first ContentFrm, it should be
 *                 moved not only to the next column, but probably even to the next page, creating
 *                 a section-follow there.
 */
        if( IsInSct() && bMovedFwd && bMakePage && pOldUp->IsColBodyFrm() &&
            pOldUp->GetUpper()->GetUpper()->IsSctFrm() &&
            ( pPre || pOldUp->GetUpper()->GetPrev() ) &&
            ((SwSectionFrm*)pOldUp->GetUpper()->GetUpper())->MoveAllowed(this) )
        {
            bMovedFwd = false;
        }

        const sal_Bool bCheckForGrownBody = pOldUp->IsBodyFrm();
        const long nOldBodyHeight = (pOldUp->Frm().*fnRect->fnGetHeight)();

        if ( !bMovedFwd && !MoveFwd( bMakePage, sal_False ) )
            bMakePage = false;
        SWREFRESHFN( this )

        
        
        
        
        
        
        if ( bCheckForGrownBody && ! bMovedBwd && pOldUp != GetUpper() &&
             (pOldUp->Frm().*fnRect->fnGetHeight)() > nOldBodyHeight )
        {
            bMovedFwd = false;
        }
        else
        {
            bMovedFwd = true;
        }

        bFormatted = sal_False;
        if ( bMoveOrFit && GetUpper() == pOldUp )
        {
            
            if ( nConsequetiveFormatsWithoutChange <= cnStopFormat )
            {
                Prepare( PREP_MUST_FIT, 0, sal_False );
                mbValidSize = sal_False;
                bMustFit = true;
                continue;
            }

#if OSL_DEBUG_LEVEL > 0
            OSL_FAIL( "LoopControl in SwCntntFrm::MakeAll" );
#endif
        }
        if ( bMovedBwd && GetUpper() )
        {   
            GetUpper()->ResetCompletePaint();
            if( pPre && !pPre->IsSctFrm() )
                ::ValidateSz( pPre );
        }

    } 

    
    
    LOOPING_LOUIE_LIGHT( bMovedFwd && bMovedBwd && !IsInBalancedSection() &&
                            (

                                
                                ( bFtn && !FindFtnFrm()->GetRef()->IsInSct() ) ||

                                
                                ( IsInSct() && bKeep )

                                

                            ),
                         static_cast<SwTxtFrm&>(*this) );


    delete pSaveFtn;

    UnlockJoin();
    if ( bMovedFwd || bMovedBwd )
        pNotify->SetInvaKeep();
    
    if ( bMovedFwd )
    {
        pNotify->SetInvalidatePrevPrtArea();
    }
    delete pNotify;
    SetFlyLock( sal_False );
}

void MakeNxt( SwFrm *pFrm, SwFrm *pNxt )
{
    
    
    
    const sal_Bool bOldPos = pFrm->GetValidPosFlag();
    const sal_Bool bOldSz  = pFrm->GetValidSizeFlag();
    const sal_Bool bOldPrt = pFrm->GetValidPrtAreaFlag();
    pFrm->mbValidPos = pFrm->mbValidPrtArea = pFrm->mbValidSize = sal_True;

    
    
    if ( pNxt->IsCntntFrm() )
    {
        SwCntntNotify aNotify( (SwCntntFrm*)pNxt );
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pNxt );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( !pNxt->GetValidSizeFlag() )
        {
            if( pNxt->IsVertical() )
                pNxt->Frm().Height( pNxt->GetUpper()->Prt().Height() );
            else
                pNxt->Frm().Width( pNxt->GetUpper()->Prt().Width() );
        }
        ((SwCntntFrm*)pNxt)->MakePrtArea( rAttrs );
        pNxt->Format( &rAttrs );
    }
    else
    {
        SwLayNotify aNotify( (SwLayoutFrm*)pNxt );
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pNxt );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( !pNxt->GetValidSizeFlag() )
        {
            if( pNxt->IsVertical() )
                pNxt->Frm().Height( pNxt->GetUpper()->Prt().Height() );
            else
                pNxt->Frm().Width( pNxt->GetUpper()->Prt().Width() );
        }
        pNxt->Format( &rAttrs );
    }

    pFrm->mbValidPos      = bOldPos;
    pFrm->mbValidSize     = bOldSz;
    pFrm->mbValidPrtArea  = bOldPrt;
}



static bool lcl_IsNextFtnBoss( const SwFrm *pFrm, const SwFrm* pNxt )
{
    OSL_ENSURE( pFrm && pNxt, "lcl_IsNextFtnBoss: No Frames?" );
    pFrm = pFrm->FindFtnBossFrm();
    pNxt = pNxt->FindFtnBossFrm();
    
    while( pFrm && pFrm->IsColumnFrm() && !pFrm->GetNext() )
        pFrm = pFrm->GetUpper()->FindFtnBossFrm();
    
    while( pNxt && pNxt->IsColumnFrm() && !pNxt->GetPrev() )
        pNxt = pNxt->GetUpper()->FindFtnBossFrm();
    
    return ( pFrm && pNxt && pFrm->GetNext() == pNxt );
}

sal_Bool SwCntntFrm::_WouldFit( SwTwips nSpace,
                            SwLayoutFrm *pNewUpper,
                            sal_Bool bTstMove,
                            const bool bObjsInNewUpper )
{
    
    
    
    SwFtnFrm* pFtnFrm = 0;
    if ( IsInFtn() )
    {
        if( !lcl_IsNextFtnBoss( pNewUpper, this ) )
            return sal_True;
        pFtnFrm = FindFtnFrm();
    }

    sal_Bool bRet;
    sal_Bool bSplit = !pNewUpper->Lower();
    SwCntntFrm *pFrm = this;
    const SwFrm *pTmpPrev = pNewUpper->Lower();
    if( pTmpPrev && pTmpPrev->IsFtnFrm() )
        pTmpPrev = ((SwFtnFrm*)pTmpPrev)->Lower();
    while ( pTmpPrev && pTmpPrev->GetNext() )
        pTmpPrev = pTmpPrev->GetNext();
    do
    {
        
        SwTwips nSecondCheck = 0;
        SwTwips nOldSpace = nSpace;
        sal_Bool bOldSplit = bSplit;

        if ( bTstMove || IsInFly() || ( IsInSct() &&
             ( pFrm->GetUpper()->IsColBodyFrm() || ( pFtnFrm &&
               pFtnFrm->GetUpper()->GetUpper()->IsColumnFrm() ) ) ) )
        {
            
            
            
            
            
            
            
            
            
            
            SwFrm* pTmpFrm = pFrm->IsInFtn() && !pNewUpper->FindFtnFrm() ?
                             (SwFrm*)pFrm->FindFtnFrm() : pFrm;
            SwLayoutFrm *pUp = pTmpFrm->GetUpper();
            SwFrm *pOldNext = pTmpFrm->GetNext();
            pTmpFrm->Remove();
            pTmpFrm->InsertBefore( pNewUpper, 0 );
            if ( pFrm->IsTxtFrm() &&
                 ( bTstMove ||
                   ((SwTxtFrm*)pFrm)->HasFollow() ||
                   ( !((SwTxtFrm*)pFrm)->HasPara() &&
                     !((SwTxtFrm*)pFrm)->IsEmpty()
                   )
                 )
               )
            {
                bTstMove = sal_True;
                bRet = ((SwTxtFrm*)pFrm)->TestFormat( pTmpPrev, nSpace, bSplit );
            }
            else
                bRet = pFrm->WouldFit( nSpace, bSplit, sal_False );

            pTmpFrm->Remove();
            pTmpFrm->InsertBefore( pUp, pOldNext );
        }
        else
        {
            bRet = pFrm->WouldFit( nSpace, bSplit, sal_False );
            nSecondCheck = !bSplit ? 1 : 0;
        }

        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
        const SwBorderAttrs &rAttrs = *aAccess.Get();

        
        
        if ( bRet && !bTstMove )
        {
            SwTwips nUpper;

            if ( pTmpPrev )
            {
                nUpper = CalcUpperSpace( NULL, pTmpPrev );

                
                
                sal_Bool bCommonBorder = sal_True;
                if ( pFrm->IsInSct() && pFrm->GetUpper()->IsColBodyFrm() )
                {
                    const SwSectionFrm* pSct = pFrm->FindSctFrm();
                    bCommonBorder = pSct->GetFmt()->GetBalancedColumns().GetValue();
                }

                
                nSecondCheck = ( 1 == nSecondCheck &&
                                 pFrm == this &&
                                 IsTxtFrm() &&
                                 bCommonBorder &&
                                 !static_cast<const SwTxtFrm*>(this)->IsEmpty() ) ?
                                 nUpper :
                                 0;

                nUpper += bCommonBorder ?
                          rAttrs.GetBottomLine( *(pFrm) ) :
                          rAttrs.CalcBottomLine();

            }
            else
            {
                
                nSecondCheck = 0;

                if( pFrm->IsVertical() )
                    nUpper = pFrm->Frm().Width() - pFrm->Prt().Width();
                else
                    nUpper = pFrm->Frm().Height() - pFrm->Prt().Height();
            }

            nSpace -= nUpper;

            if ( nSpace < 0 )
            {
                bRet = sal_False;

                
                if ( nSecondCheck > 0 )
                {
                    
                    
                    
                    
                    
                    
                    
                    
                    nOldSpace -= nSecondCheck;
                    const bool bSecondRet = nOldSpace >= 0 && pFrm->WouldFit( nOldSpace, bOldSplit, sal_False );
                    if ( bSecondRet && bOldSplit && nOldSpace >= 0 )
                    {
                        bRet = sal_True;
                        bSplit = sal_True;
                    }
                }
            }
        }

        
        if ( bRet && IsInTab() &&
             pNewUpper->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS) )
        {
            nSpace -= rAttrs.GetULSpace().GetLower();
            if ( nSpace < 0 )
            {
                bRet = sal_False;
            }
        }

        if ( bRet && !bSplit && pFrm->IsKeep( rAttrs.GetAttrSet() ) )
        {
            if( bTstMove )
            {
                while( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->HasFollow() )
                {
                    pFrm = ((SwTxtFrm*)pFrm)->GetFollow();
                }
                
                
                
                if ( IsAnFollow( pFrm ) && !pFrm->IsValid() )
                {
                    OSL_FAIL( "Only a warning for task 108824:/n<SwCntntFrm::_WouldFit(..) - follow not valid!" );
                    return sal_True;
                }
            }
            SwFrm *pNxt;
            if( 0 != (pNxt = pFrm->FindNext()) && pNxt->IsCntntFrm() &&
                ( !pFtnFrm || ( pNxt->IsInFtn() &&
                  pNxt->FindFtnFrm()->GetAttr() == pFtnFrm->GetAttr() ) ) )
            {
                
                //
                
                
                if ( bTstMove &&
                     ( pNxt->GetDrawObjs() || bObjsInNewUpper ) )
                {
                    return sal_True;
                }

                if ( !pNxt->IsValid() )
                    MakeNxt( pFrm, pNxt );

                
                
                
                if( lcl_NotHiddenPrev( pNxt ) )
                    pTmpPrev = 0;
                else
                {
                    if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsHiddenNow() )
                        pTmpPrev = lcl_NotHiddenPrev( pFrm );
                    else
                        pTmpPrev = pFrm;
                }
                pFrm = (SwCntntFrm*)pNxt;
            }
            else
                pFrm = 0;
        }
        else
            pFrm = 0;

    } while ( bRet && pFrm );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
