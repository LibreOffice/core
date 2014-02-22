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

#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <ftnidx.hxx>
#include <pagefrm.hxx>
#include <colfrm.hxx>
#include <rootfrm.hxx>
#include <frmtool.hxx>
#include <ftnfrm.hxx>
#include <txtfrm.hxx>
#include <tabfrm.hxx>
#include <pagedesc.hxx>
#include <ftninfo.hxx>
#include <sectfrm.hxx>
#include <objectformatter.hxx>
#include "viewopt.hxx"
#include <switerator.hxx>

#define ENDNOTE 0x80000000



static sal_uLong lcl_FindFtnPos( const SwDoc *pDoc, const SwTxtFtn *pAttr )
{
    const SwFtnIdxs &rFtnIdxs = pDoc->GetFtnIdxs();

    SwTxtFtn* pBla = (SwTxtFtn*)pAttr;
    SwFtnIdxs::const_iterator it = rFtnIdxs.find( pBla );
    if ( it != rFtnIdxs.end() )
    {
        sal_uInt16 nRet = it - rFtnIdxs.begin();
        if( pAttr->GetFtn().IsEndNote() )
            return sal_uLong(nRet) + ENDNOTE;
        return nRet;
    }
    OSL_ENSURE( !pDoc, "FtnPos not found." );
    return 0;
}

sal_Bool SwFtnFrm::operator<( const SwTxtFtn* pTxtFtn ) const
{
    const SwDoc* pDoc = GetFmt()->GetDoc();
    OSL_ENSURE( pDoc, "SwFtnFrm: Missing doc!" );
    return lcl_FindFtnPos( pDoc, GetAttr() ) <
           lcl_FindFtnPos( pDoc, pTxtFtn );
}



static sal_Bool lcl_NextFtnBoss( SwFtnBossFrm* &rpBoss, SwPageFrm* &rpPage,
    sal_Bool bDontLeave )
{
    if( rpBoss->IsColumnFrm() )
    {
        if( rpBoss->GetNext() )
        {
            rpBoss = (SwFtnBossFrm*)rpBoss->GetNext(); 
            return sal_False;
        }
        if( rpBoss->IsInSct() )
        {
            SwSectionFrm* pSct = rpBoss->FindSctFrm()->GetFollow();
            if( pSct )
            {
                OSL_ENSURE( pSct->Lower() && pSct->Lower()->IsColumnFrm(),
                        "Where's the column?" );
                rpBoss = (SwColumnFrm*)pSct->Lower();
                SwPageFrm* pOld = rpPage;
                rpPage = pSct->FindPageFrm();
                return pOld != rpPage;
            }
            else if( bDontLeave )
            {
                rpPage = NULL;
                rpBoss = NULL;
                return sal_False;
            }
        }
    }
    rpPage = (SwPageFrm*)rpPage->GetNext(); 
    rpBoss = rpPage;
    if( rpPage )
    {
        SwLayoutFrm* pBody = rpPage->FindBodyCont();
        if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            rpBoss = (SwFtnBossFrm*)pBody->Lower(); 
    }
    return sal_True;
}


static sal_uInt16 lcl_ColumnNum( const SwFrm* pBoss )
{
    sal_uInt16 nRet = 0;
    if( !pBoss->IsColumnFrm() )
        return 0;
    const SwFrm* pCol;
    if( pBoss->IsInSct() )
    {
        pCol = pBoss->GetUpper()->FindColFrm();
        if( pBoss->GetNext() || pBoss->GetPrev() )
        {
            while( pBoss )
            {
                ++nRet;                     
                pBoss = pBoss->GetPrev();
            }
        }
    }
    else
        pCol = pBoss;
    while( pCol )
    {
        nRet += 256;                    
        pCol = pCol->GetPrev();
    }
    return nRet;
}

SwFtnContFrm::SwFtnContFrm( SwFrmFmt *pFmt, SwFrm* pSib ):
    SwLayoutFrm( pFmt, pSib )
{
    mnType = FRMC_FTNCONT;
}



static long lcl_Undersize( const SwFrm* pFrm )
{
    long nRet = 0;
    SWRECTFN( pFrm )
    if( pFrm->IsTxtFrm() )
    {
        if( ((SwTxtFrm*)pFrm)->IsUndersized() )
        {
            
            nRet = ((SwTxtFrm*)pFrm)->GetParHeight() -
                    (pFrm->Prt().*fnRect->fnGetHeight)();
            if( nRet < 0 )
                nRet = 0;
        }
    }
    else if( pFrm->IsLayoutFrm() )
    {
        const SwFrm* pNxt = ((SwLayoutFrm*)pFrm)->Lower();
        while( pNxt )
        {
            nRet += lcl_Undersize( pNxt );
            pNxt = pNxt->GetNext();
        }
    }
    return nRet;
}


void SwFtnContFrm::Format( const SwBorderAttrs * )
{
    
    const SwPageFrm* pPage = FindPageFrm();
    const SwPageFtnInfo &rInf = pPage->GetPageDesc()->GetFtnInfo();
    const SwTwips nBorder = rInf.GetTopDist() + rInf.GetBottomDist() +
                            rInf.GetLineWidth();
    SWRECTFN( this )
    if ( !mbValidPrtArea )
    {
        mbValidPrtArea = sal_True;
        (Prt().*fnRect->fnSetTop)( nBorder );
        (Prt().*fnRect->fnSetWidth)( (Frm().*fnRect->fnGetWidth)() );
        (Prt().*fnRect->fnSetHeight)((Frm().*fnRect->fnGetHeight)() - nBorder );
        if( (Prt().*fnRect->fnGetHeight)() < 0 && !pPage->IsFtnPage() )
            mbValidSize = sal_False;
    }

    if ( !mbValidSize )
    {
        bool bGrow = pPage->IsFtnPage();
        if( bGrow )
        {
            const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                bGrow = false;
        }
        if( bGrow )
                Grow( LONG_MAX, sal_False );
        else
        {
            
            SwTwips nRemaining = 0;
            SwFrm *pFrm = pLower;
            while ( pFrm )
            {
                nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)() + lcl_Undersize( pFrm );
                pFrm = pFrm->GetNext();
            }
            
            nRemaining += nBorder;

            SwTwips nDiff;
            if( IsInSct() )
            {
                nDiff = -(Frm().*fnRect->fnBottomDist)(
                                        (GetUpper()->*fnRect->fnGetPrtBottom)() );
                if( nDiff > 0 )
                {
                    if( nDiff > (Frm().*fnRect->fnGetHeight)() )
                        nDiff = (Frm().*fnRect->fnGetHeight)();
                    (Frm().*fnRect->fnAddBottom)( -nDiff );
                    (Prt().*fnRect->fnAddHeight)( -nDiff );
                }
            }
            nDiff = (Frm().*fnRect->fnGetHeight)() - nRemaining;
            if ( nDiff > 0 )
                Shrink( nDiff );
            else if ( nDiff < 0 )
            {
                Grow( -nDiff );
                
                
                SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                if( nPrtHeight < 0 )
                {
                    const SwTwips nTmpDiff = std::max( (Prt().*fnRect->fnGetTop)(),
                                                -nPrtHeight );
                    (Prt().*fnRect->fnSubTop)( nTmpDiff );
                }
            }
        }
        mbValidSize = sal_True;
    }
}

SwTwips SwFtnContFrm::GrowFrm( SwTwips nDist, sal_Bool bTst, sal_Bool )
{
    
    
    
#if OSL_DEBUG_LEVEL > 1
    if ( !GetUpper() || !GetUpper()->IsFtnBossFrm() )
    { OSL_ENSURE( !this, "no FtnBoss." );
        return 0;
    }
#endif

    SWRECTFN( this )
    if( (Frm().*fnRect->fnGetHeight)() > 0 &&
         nDist > ( LONG_MAX - (Frm().*fnRect->fnGetHeight)() ) )
        nDist = LONG_MAX - (Frm().*fnRect->fnGetHeight)();

    SwFtnBossFrm *pBoss = (SwFtnBossFrm*)GetUpper();
    if( IsInSct() )
    {
        SwSectionFrm* pSect = FindSctFrm();
        OSL_ENSURE( pSect, "GrowFrm: Missing SectFrm" );
        
        
        if( !bTst && !pSect->IsColLocked() &&
            pSect->ToMaximize( sal_False ) && pSect->Growable() )
        {
            pSect->InvalidateSize();
            return 0;
        }
    }
    const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
    const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
    SwPageFrm *pPage = pBoss->FindPageFrm();
    if ( bBrowseMode || !pPage->IsFtnPage() )
    {
        if ( pBoss->GetMaxFtnHeight() != LONG_MAX )
        {
            nDist = std::min( nDist, pBoss->GetMaxFtnHeight()
                         - (Frm().*fnRect->fnGetHeight)() );
            if ( nDist <= 0 )
                return 0L;
        }
        
        if( !IsInSct() )
        {
            const SwTwips nMax = pBoss->GetVarSpace();
            if ( nDist > nMax )
                nDist = nMax;
            if ( nDist <= 0 )
                return 0L;
        }
    }
    else if( nDist > (GetPrev()->Frm().*fnRect->fnGetHeight)() )
        
        nDist = (GetPrev()->Frm().*fnRect->fnGetHeight)();

    long nAvail = 0;
    if ( bBrowseMode )
    {
        nAvail = GetUpper()->Prt().Height();
        const SwFrm *pAvail = GetUpper()->Lower();
        do
        {   nAvail -= pAvail->Frm().Height();
            pAvail = pAvail->GetNext();
        } while ( pAvail );
        if ( nAvail > nDist )
            nAvail = nDist;
    }

    if ( !bTst )
    {
        (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)() + nDist );
        
        if( IsVertical() && !IsVertLR() && !IsReverse() )
            Frm().Pos().X() -= nDist;
    }
    long nGrow = nDist - nAvail,
         nReal = 0;
    if ( nGrow > 0 )
    {
        sal_uInt8 nAdjust = pBoss->NeighbourhoodAdjustment( this );
        if( NA_ONLY_ADJUST == nAdjust )
            nReal = AdjustNeighbourhood( nGrow, bTst );
        else
        {
            if( NA_GROW_ADJUST == nAdjust )
            {
                SwFrm* pFtn = Lower();
                if( pFtn )
                {
                    while( pFtn->GetNext() )
                        pFtn = pFtn->GetNext();
                    if( ((SwFtnFrm*)pFtn)->GetAttr()->GetFtn().IsEndNote() )
                    {
                        nReal = AdjustNeighbourhood( nGrow, bTst );
                        nAdjust = NA_GROW_SHRINK; 
                    }
                }
            }
            nReal += pBoss->Grow( nGrow - nReal, bTst );
            if( ( NA_GROW_ADJUST == nAdjust || NA_ADJUST_GROW == nAdjust )
                  && nReal < nGrow )
                nReal += AdjustNeighbourhood( nGrow - nReal, bTst );
        }
    }

    nReal += nAvail;

    if ( !bTst )
    {
        if ( nReal != nDist )
        {
            nDist -= nReal;
            Frm().SSize().Height() -= nDist;
            
            if( IsVertical() && !IsVertLR() && !IsReverse() )
                Frm().Pos().X() += nDist;
        }

        
        if( nReal )
        {
            _InvalidateSize();
            _InvalidatePos();
            InvalidatePage( pPage );
        }
    }
    return nReal;
}

SwTwips SwFtnContFrm::ShrinkFrm( SwTwips nDiff, sal_Bool bTst, sal_Bool bInfo )
{
    SwPageFrm *pPage = FindPageFrm();
    bool bShrink = false;
    if ( pPage )
    {
        if( !pPage->IsFtnPage() )
            bShrink = true;
        else
        {
            const SwViewShell *pSh = getRootFrm()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                bShrink = true;
        }
    }
    if( bShrink )
    {
        SwTwips nRet = SwLayoutFrm::ShrinkFrm( nDiff, bTst, bInfo );
        if( IsInSct() && !bTst )
            FindSctFrm()->InvalidateNextPos();
        if ( !bTst && nRet )
        {
            _InvalidatePos();
            InvalidatePage( pPage );
        }
        return nRet;
    }
    return 0;
}

SwFtnFrm::SwFtnFrm( SwFrmFmt *pFmt, SwFrm* pSib, SwCntntFrm *pCnt, SwTxtFtn *pAt ):
    SwLayoutFrm( pFmt, pSib ),
    pFollow( 0 ),
    pMaster( 0 ),
    pRef( pCnt ),
    pAttr( pAt ),
    bBackMoveLocked( sal_False ),
    
    mbUnlockPosOfLowerObjs( true )
{
    mnType = FRMC_FTN;
}

void SwFtnFrm::InvalidateNxtFtnCnts( SwPageFrm *pPage )
{
    if ( GetNext() )
    {
        SwFrm *pCnt = ((SwLayoutFrm*)GetNext())->ContainsAny();
        if( pCnt )
        {
            pCnt->InvalidatePage( pPage );
            pCnt->_InvalidatePrt();
            do
            {   pCnt->_InvalidatePos();
                if( pCnt->IsSctFrm() )
                {
                    SwFrm* pTmp = ((SwSectionFrm*)pCnt)->ContainsAny();
                    if( pTmp )
                        pTmp->_InvalidatePos();
                }
                pCnt->GetUpper()->_InvalidateSize();
                pCnt = pCnt->FindNext();
            } while ( pCnt && GetUpper()->IsAnLower( pCnt ) );
        }
    }
}

#ifdef DBG_UTIL
SwTwips SwFtnFrm::GrowFrm( SwTwips nDist, sal_Bool bTst, sal_Bool bInfo )
{
    static sal_uInt16 nNum = USHRT_MAX;
    SwTxtFtn* pTxtFtn = GetAttr();
    if ( pTxtFtn->GetFtn().GetNumber() == nNum )
    {
        int bla = 5;
        (void)bla;

    }
    return SwLayoutFrm::GrowFrm( nDist, bTst, bInfo );
}

SwTwips SwFtnFrm::ShrinkFrm( SwTwips nDist, sal_Bool bTst, sal_Bool bInfo )
{
    static sal_uInt16 nNum = USHRT_MAX;
    if( nNum != USHRT_MAX )
    {
        SwTxtFtn* pTxtFtn = GetAttr();
        if( &pTxtFtn->GetAttr() && pTxtFtn->GetFtn().GetNumber() == nNum )
        {
            int bla = 5;
            (void)bla;
        }
    }
    return SwLayoutFrm::ShrinkFrm( nDist, bTst, bInfo );
}
#endif

void SwFtnFrm::Cut()
{
    if ( GetNext() )
        GetNext()->InvalidatePos();
    else if ( GetPrev() )
        GetPrev()->SetRetouche();

    
    SwLayoutFrm *pUp = GetUpper();

    
    SwFtnFrm *pFtn = (SwFtnFrm*)this;
    if ( pFtn->GetFollow() )
        pFtn->GetFollow()->SetMaster( pFtn->GetMaster() );
    if ( pFtn->GetMaster() )
        pFtn->GetMaster()->SetFollow( pFtn->GetFollow() );
    pFtn->SetFollow( 0 );
    pFtn->SetMaster( 0 );

    
    Remove();

    if ( pUp )
    {
        
        if ( !pUp->Lower() )
        {
            SwPageFrm *pPage = pUp->FindPageFrm();
            if ( pPage )
            {
                SwLayoutFrm *pBody = pPage->FindBodyCont();
                if( pBody && !pBody->ContainsCntnt() )
                    pPage->getRootFrm()->SetSuperfluous();
            }
            SwSectionFrm* pSect = pUp->FindSctFrm();
            pUp->Cut();
            delete pUp;
            
            
            if( pSect && !pSect->ToMaximize( sal_False ) && !pSect->IsColLocked() )
                pSect->_InvalidateSize();
        }
        else
        {   if ( Frm().Height() )
                pUp->Shrink( Frm().Height() );
            pUp->SetCompletePaint();
            pUp->InvalidatePage();
        }
    }
}

void SwFtnFrm::Paste(  SwFrm* pParent, SwFrm* pSibling )
{
    OSL_ENSURE( pParent, "no parent in Paste." );
    OSL_ENSURE( pParent->IsLayoutFrm(), "Parent is CntntFrm." );
    OSL_ENSURE( pParent != this, "I am my own parent." );
    OSL_ENSURE( pSibling != this, "I am my own sibling." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I am still somewhere registered." );

    
    InsertBefore( (SwLayoutFrm*)pParent, pSibling );

    SWRECTFN( this )
    if( (Frm().*fnRect->fnGetWidth)()!=(pParent->Prt().*fnRect->fnGetWidth)() )
        _InvalidateSize();
    _InvalidatePos();
    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    if ( GetNext() )
        GetNext()->_InvalidatePos();
    if( (Frm().*fnRect->fnGetHeight)() )
        pParent->Grow( (Frm().*fnRect->fnGetHeight)() );

    
    
    if ( GetPrev() && GetPrev() == GetMaster() )
    { OSL_ENSURE( SwFlowFrm::CastFlowFrm( GetPrev()->GetLower() ),
                "Footnote without content?" );
        (SwFlowFrm::CastFlowFrm( GetPrev()->GetLower()))->
            MoveSubTree( this, GetLower() );
        SwFrm *pDel = GetPrev();
        pDel->Cut();
        delete pDel;
    }
    if ( GetNext() && GetNext() == GetFollow() )
    { OSL_ENSURE( SwFlowFrm::CastFlowFrm( GetNext()->GetLower() ),
                "Footnote without content?" );
        (SwFlowFrm::CastFlowFrm( GetNext()->GetLower()))->MoveSubTree( this );
        SwFrm *pDel = GetNext();
        pDel->Cut();
        delete pDel;
    }
#if OSL_DEBUG_LEVEL > 0
    SwDoc *pDoc = GetFmt()->GetDoc();
    if ( GetPrev() )
    {
        OSL_ENSURE( lcl_FindFtnPos( pDoc, ((SwFtnFrm*)GetPrev())->GetAttr() ) <=
                lcl_FindFtnPos( pDoc, GetAttr() ), "Prev is not FtnPrev" );
    }
    if ( GetNext() )
    {
        OSL_ENSURE( lcl_FindFtnPos( pDoc, GetAttr() ) <=
                lcl_FindFtnPos( pDoc, ((SwFtnFrm*)GetNext())->GetAttr() ),
                "Next is not FtnNext" );
    }
#endif
    InvalidateNxtFtnCnts( pPage );
}


SwLayoutFrm *SwFrm::GetNextFtnLeaf( MakePageType eMakePage )
{
    SwFtnBossFrm *pOldBoss = FindFtnBossFrm();
    SwPageFrm* pOldPage = pOldBoss->FindPageFrm();
    SwPageFrm* pPage;
    SwFtnBossFrm *pBoss = pOldBoss->IsColumnFrm() ?
        (SwFtnBossFrm*)pOldBoss->GetNext() : 0; 
    if( pBoss )
        pPage = NULL;
    else
    {
        if( pOldBoss->GetUpper()->IsSctFrm() )
        {   
            SwLayoutFrm* pNxt = pOldBoss->GetNextSctLeaf( eMakePage );
            if( pNxt )
            {
                OSL_ENSURE( pNxt->IsColBodyFrm(), "GetNextFtnLeaf: Funny Leaf" );
                pBoss = (SwFtnBossFrm*)pNxt->GetUpper();
                pPage = pBoss->FindPageFrm();
            }
            else
                return 0;
        }
        else
        {
            
            pPage = (SwPageFrm*)pOldPage->GetNext();
            
            if( pPage && pPage->IsEmptyPage() )
                pPage = (SwPageFrm*)pPage->GetNext();
            pBoss = pPage;
        }
    }
    
    
    
    

    
    
    
    SwFtnFrm *pFtn = FindFtnFrm();
    if ( pFtn && pFtn->GetFollow() )
    {
        SwFtnBossFrm* pTmpBoss = pFtn->GetFollow()->FindFtnBossFrm();
        
        
        
        
        while( pTmpBoss != pBoss && pTmpBoss && !pTmpBoss->GetPrev() )
            pTmpBoss = pTmpBoss->GetUpper()->FindFtnBossFrm();
        if( pTmpBoss == pBoss )
            return pFtn->GetFollow();
    }

    
    if ( !pBoss || ( pPage && pPage->IsEndNotePage() && !pOldPage->IsEndNotePage() ) )
    {
        if ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT )
        {
            pBoss = InsertPage( pOldPage, pOldPage->IsFtnPage() );
            ((SwPageFrm*)pBoss)->SetEndNotePage( pOldPage->IsEndNotePage() );
        }
        else
            return 0;
    }
    if( pBoss->IsPageFrm() )
    {
        
        SwLayoutFrm* pLay = pBoss->FindBodyCont();
        if( pLay && pLay->Lower() && pLay->Lower()->IsColumnFrm() )
            pBoss = (SwFtnBossFrm*)pLay->Lower();
    }
    
    SwFtnContFrm *pCont = pBoss->FindFtnCont();
    if ( !pCont && pBoss->GetMaxFtnHeight() &&
         ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT ) )
        pCont = pBoss->MakeFtnCont();
    return pCont;
}


SwLayoutFrm *SwFrm::GetPrevFtnLeaf( MakePageType eMakeFtn )
{
    
    
    SwFtnFrm *pFtn = FindFtnFrm();
    SwLayoutFrm *pRet = pFtn->GetMaster();

    SwFtnBossFrm* pOldBoss = FindFtnBossFrm();
    SwPageFrm *pOldPage = pOldBoss->FindPageFrm();

    if ( !pOldBoss->GetPrev() && !pOldPage->GetPrev() )
        return pRet; 

    if ( !pRet )
    {
        bool bEndn = pFtn->GetAttr()->GetFtn().IsEndNote();
        SwFrm* pTmpRef = NULL;
        if( bEndn && pFtn->IsInSct() )
        {
            SwSectionFrm* pSect = pFtn->FindSctFrm();
            if( pSect->IsEndnAtEnd() )
                pTmpRef = pSect->FindLastCntnt( FINDMODE_LASTCNT );
        }
        if( !pTmpRef )
            pTmpRef = pFtn->GetRef();
        SwFtnBossFrm* pStop = pTmpRef->FindFtnBossFrm( !bEndn );

        const sal_uInt16 nNum = pStop->GetPhyPageNum();

        
        
        const sal_Bool bEndNote = pOldPage->IsEndNotePage();
        const sal_Bool bFtnEndDoc = pOldPage->IsFtnPage();
        SwFtnBossFrm* pNxtBoss = pOldBoss;
        SwSectionFrm *pSect = pNxtBoss->GetUpper()->IsSctFrm() ?
                              (SwSectionFrm*)pNxtBoss->GetUpper() : 0;

        do
        {
            if( pNxtBoss->IsColumnFrm() && pNxtBoss->GetPrev() )
                pNxtBoss = (SwFtnBossFrm*)pNxtBoss->GetPrev();  
            else 
            {
                SwLayoutFrm* pBody = 0;
                if( pSect )
                {
                    if( pSect->IsFtnLock() )
                    {
                        if( pNxtBoss == pOldBoss )
                            return 0;
                        pStop = pNxtBoss;
                    }
                    else
                    {
                        pSect = (SwSectionFrm*)pSect->FindMaster();
                        if( !pSect || !pSect->Lower() )
                            return 0;
                        OSL_ENSURE( pSect->Lower()->IsColumnFrm(),
                                "GetPrevFtnLeaf: Where's the column?" );
                        pNxtBoss = (SwFtnBossFrm*)pSect->Lower();
                        pBody = pSect;
                    }
                }
                else
                {
                    SwPageFrm* pPage = (SwPageFrm*)pNxtBoss->FindPageFrm()->GetPrev();
                    if( !pPage || pPage->GetPhyPageNum() < nNum ||
                        bEndNote != pPage->IsEndNotePage() || bFtnEndDoc != pPage->IsFtnPage() )
                        return NULL; 
                    pNxtBoss = pPage;
                    pBody = pPage->FindBodyCont();
                }
                
                if( pBody )
                {
                    if ( pBody->Lower() && pBody->Lower()->IsColumnFrm() )
                    {
                        pNxtBoss = static_cast<SwFtnBossFrm*>(pBody->GetLastLower());
                    }
                }
            }
            SwFtnContFrm *pCont = pNxtBoss->FindFtnCont();
            if ( pCont )
            {
                pRet = pCont;
                break;
            }
            if ( pStop == pNxtBoss )
            {
                
                
                if ( eMakeFtn == MAKEPAGE_FTN && pNxtBoss->GetMaxFtnHeight() )
                    pRet = pNxtBoss->MakeFtnCont();
                break;
            }
        } while( !pRet );
    }
    if ( pRet )
    {
        const SwFtnBossFrm* pNewBoss = pRet->FindFtnBossFrm();
        bool bJump = false;
        if( pOldBoss->IsColumnFrm() && pOldBoss->GetPrev() ) 
            bJump = pOldBoss->GetPrev() != (SwFrm*)pNewBoss; 
        else if( pNewBoss->IsColumnFrm() && pNewBoss->GetNext() )
            bJump = true; 
        else
        {
            
            
            sal_uInt16 nDiff = pOldPage->GetPhyPageNum() - pRet->FindPageFrm()->GetPhyPageNum();
            if ( nDiff > 2 ||
                 (nDiff > 1 && !((SwPageFrm*)pOldPage->GetPrev())->IsEmptyPage()) )
                bJump = true;
        }
        if( bJump )
            SwFlowFrm::SetMoveBwdJump( sal_True );
    }
    return pRet;
}

sal_Bool SwFrm::IsFtnAllowed() const
{
    if ( !IsInDocBody() )
        return sal_False;

    if ( IsInTab() )
    {
        
        const SwTabFrm *pTab = ((SwFrm*)this)->ImplFindTabFrm();
        if ( pTab->IsFollow() )
            return !pTab->IsInHeadline( *this );
    }
    return sal_True;
}

void SwRootFrm::UpdateFtnNums()
{
    
    if ( GetFmt()->GetDoc()->GetFtnInfo().eNum == FTNNUM_PAGE )
    {
        SwPageFrm *pPage = (SwPageFrm*)Lower();
        while ( pPage && !pPage->IsFtnPage() )
        {
            pPage->UpdateFtnNum();
            pPage = (SwPageFrm*)pPage->GetNext();
        }
    }
}


void sw_RemoveFtns( SwFtnBossFrm* pBoss, sal_Bool bPageOnly, sal_Bool bEndNotes )
{
    do
    {
        SwFtnContFrm *pCont = pBoss->FindFtnCont();
        if ( pCont )
        {
            SwFtnFrm *pFtn = (SwFtnFrm*)pCont->Lower();
            OSL_ENSURE( pFtn, "Footnote content without footnote." );
            if ( bPageOnly )
                while ( pFtn->GetMaster() )
                    pFtn = pFtn->GetMaster();
            do
            {
                SwFtnFrm *pNxt = (SwFtnFrm*)pFtn->GetNext();
                if ( !pFtn->GetAttr()->GetFtn().IsEndNote() ||
                        bEndNotes )
                {
                    pFtn->GetRef()->Prepare( PREP_FTN, (void*)pFtn->GetAttr() );
                    if ( bPageOnly && !pNxt )
                        pNxt = pFtn->GetFollow();
                    pFtn->Cut();
                    delete pFtn;
                }
                pFtn = pNxt;

            } while ( pFtn );
        }
        if( !pBoss->IsInSct() )
        {
            
            
            
            SwLayoutFrm* pBody = pBoss->FindBodyCont();
            if( pBody && pBody->Lower() )
            {
                SwFrm* pLow = pBody->Lower();
                while (pLow)
                {
                    if( pLow->IsSctFrm() && ( !pLow->GetNext() ||
                        ((SwSectionFrm*)pLow)->IsAnyNoteAtEnd() ) &&
                        ((SwSectionFrm*)pLow)->Lower() &&
                        ((SwSectionFrm*)pLow)->Lower()->IsColumnFrm() )
                        sw_RemoveFtns( (SwColumnFrm*)((SwSectionFrm*)pLow)->Lower(),
                            bPageOnly, bEndNotes );
                    pLow = pLow->GetNext();
                }
            }
        }
        
        pBoss = pBoss->IsColumnFrm() ? (SwColumnFrm*)pBoss->GetNext() : NULL;
    } while( pBoss );
}

void SwRootFrm::RemoveFtns( SwPageFrm *pPage, sal_Bool bPageOnly, sal_Bool bEndNotes )
{
    if ( !pPage )
        pPage = (SwPageFrm*)Lower();

    do
    {
        
        SwFtnBossFrm* pBoss;
        SwLayoutFrm* pBody = pPage->FindBodyCont();
        if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            pBoss = (SwFtnBossFrm*)pBody->Lower(); 
        else
            pBoss = pPage; 
        sw_RemoveFtns( pBoss, bPageOnly, bEndNotes );
        if ( !bPageOnly )
        {
            if ( pPage->IsFtnPage() &&
                 (!pPage->IsEndNotePage() || bEndNotes) )
            {
                SwFrm *pDel = pPage;
                pPage = (SwPageFrm*)pPage->GetNext();
                pDel->Cut();
                delete pDel;
            }
            else
                pPage = (SwPageFrm*)pPage->GetNext();
        }
        else
            break;

    } while ( pPage );
}

void SwRootFrm::CheckFtnPageDescs( sal_Bool bEndNote )
{
    SwPageFrm *pPage = (SwPageFrm*)Lower();
    while ( pPage && !pPage->IsFtnPage() )
        pPage = (SwPageFrm*)pPage->GetNext();
    while ( pPage && pPage->IsEndNotePage() != bEndNote )
        pPage = (SwPageFrm*)pPage->GetNext();

    if ( pPage )
        SwFrm::CheckPageDescs( pPage, sal_False );
}

/** Insert a footnote container
 *
 * A footnote container is always placed directly behind the body text.
 *
 * The frame format (FrmFmt) is always the default frame format.
 *
 * @return footnote container frame
 */
SwFtnContFrm *SwFtnBossFrm::MakeFtnCont()
{
#if OSL_DEBUG_LEVEL > 1
    if ( FindFtnCont() )
    {   OSL_ENSURE( !this, "footnote container exists already." );
        return 0;
    }
#endif

    SwFtnContFrm *pNew = new SwFtnContFrm( GetFmt()->GetDoc()->GetDfltFrmFmt(), this );
    SwLayoutFrm *pLay = FindBodyCont();
    pNew->Paste( this, pLay->GetNext() );
    return pNew;
}

SwFtnContFrm *SwFtnBossFrm::FindFtnCont()
{
    SwFrm *pFrm = Lower();
    while( pFrm && !pFrm->IsFtnContFrm() )
        pFrm = pFrm->GetNext();

#if OSL_DEBUG_LEVEL > 0
    if ( pFrm )
    {
        SwFrm *pFtn = pFrm->GetLower();
        OSL_ENSURE( pFtn, "Content without footnote." );
        while ( pFtn )
        {
            OSL_ENSURE( pFtn->IsFtnFrm(), "Neighbor of footnote is not a footnote." );
            pFtn = pFtn->GetNext();
        }
    }
#endif

    return (SwFtnContFrm*)pFrm;
}

SwFtnContFrm *SwFtnBossFrm::FindNearestFtnCont( sal_Bool bDontLeave )
{
    SwFtnContFrm *pCont = 0;
    if ( !GetFmt()->GetDoc()->GetFtnIdxs().empty() )
    {
        pCont = FindFtnCont();
        if ( !pCont )
        {
            SwPageFrm *pPage = FindPageFrm();
            SwFtnBossFrm* pBoss = this;
            sal_Bool bEndNote = pPage->IsEndNotePage();
            do
            {
                sal_Bool bChgPage = lcl_NextFtnBoss( pBoss, pPage, bDontLeave );
                
                if( pBoss && ( !bChgPage || pPage->IsEndNotePage() == bEndNote ) )
                    pCont = pBoss->FindFtnCont();
            } while ( !pCont && pPage );
        }
    }
    return pCont;
}

SwFtnFrm *SwFtnBossFrm::FindFirstFtn()
{
    
    SwFtnContFrm *pCont = FindNearestFtnCont();
    if ( !pCont )
        return 0;

    
    

    SwFtnFrm *pRet = (SwFtnFrm*)pCont->Lower();
    const sal_uInt16 nRefNum = FindPageFrm()->GetPhyPageNum();
    const sal_uInt16 nRefCol = lcl_ColumnNum( this );
    sal_uInt16 nPgNum, nColNum; 
    SwFtnBossFrm* pBoss;
    SwPageFrm* pPage;
    if( pRet )
    {
        pBoss = pRet->GetRef()->FindFtnBossFrm();
        OSL_ENSURE( pBoss, "FindFirstFtn: No boss found" );
        if( !pBoss )
            return NULL; 
        pPage = pBoss->FindPageFrm();
        nPgNum = pPage->GetPhyPageNum();
        if ( nPgNum == nRefNum )
        {
            nColNum = lcl_ColumnNum( pBoss );
            if( nColNum == nRefCol )
                return pRet; 
            else if( nColNum > nRefCol )
                return NULL; 
        }
        else if ( nPgNum > nRefNum )
            return NULL;    
    }
    else
        return NULL;
    

    do
    {
        while ( pRet->GetFollow() )
            pRet = pRet->GetFollow();

        SwFtnFrm *pNxt = (SwFtnFrm*)pRet->GetNext();
        if ( !pNxt )
        {
            pBoss = pRet->FindFtnBossFrm();
            pPage = pBoss->FindPageFrm();
            lcl_NextFtnBoss( pBoss, pPage, sal_False ); 
            pCont = pBoss ? pBoss->FindNearestFtnCont() : 0;
            if ( pCont )
                pNxt = (SwFtnFrm*)pCont->Lower();
        }
        if ( pNxt )
        {
            pRet = pNxt;
            pBoss = pRet->GetRef()->FindFtnBossFrm();
            pPage = pBoss->FindPageFrm();
            nPgNum = pPage->GetPhyPageNum();
            if ( nPgNum == nRefNum )
            {
                nColNum = lcl_ColumnNum( pBoss );
                if( nColNum == nRefCol )
                    break; 
                else if( nColNum > nRefCol )
                    pRet = 0; 
            }
            else if ( nPgNum > nRefNum )
                pRet = 0;   
        }
        else
            pRet = 0; 
    } while( pRet );
    return pRet;
}


const SwFtnFrm *SwFtnBossFrm::FindFirstFtn( SwCntntFrm *pCnt ) const
{
    const SwFtnFrm *pRet = ((SwFtnBossFrm*)this)->FindFirstFtn();
    if ( pRet )
    {
        const sal_uInt16 nColNum = lcl_ColumnNum( this );
        const sal_uInt16 nPageNum = GetPhyPageNum();
        while ( pRet && (pRet->GetRef() != pCnt) )
        {
            while ( pRet->GetFollow() )
                pRet = pRet->GetFollow();

            if ( pRet->GetNext() )
                pRet = (const SwFtnFrm*)pRet->GetNext();
            else
            {   SwFtnBossFrm *pBoss = (SwFtnBossFrm*)pRet->FindFtnBossFrm();
                SwPageFrm *pPage = pBoss->FindPageFrm();
                lcl_NextFtnBoss( pBoss, pPage, sal_False ); 
                SwFtnContFrm *pCont = pBoss ? pBoss->FindNearestFtnCont() : 0;
                pRet = pCont ? (SwFtnFrm*)pCont->Lower() : 0;
            }
            if ( pRet )
            {
                const SwFtnBossFrm* pBoss = pRet->GetRef()->FindFtnBossFrm();
                if( pBoss->GetPhyPageNum() != nPageNum ||
                    nColNum != lcl_ColumnNum( pBoss ) )
                pRet = 0;
            }
        }
    }
    return pRet;
}

void SwFtnBossFrm::ResetFtn( const SwFtnFrm *pCheck )
{
    OSL_ENSURE( !pCheck->GetMaster(), "given master is not a Master." );

    SwNodeIndex aIdx( *pCheck->GetAttr()->GetStartNode(), 1 );
    SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
    if ( !pNd )
        pNd = pCheck->GetFmt()->GetDoc()->
              GetNodes().GoNextSection( &aIdx, true, false );
    SwIterator<SwFrm,SwCntntNode> aIter( *pNd );
    SwFrm* pFrm = aIter.First();
    while( pFrm )
    {
            if( pFrm->getRootFrm() == pCheck->getRootFrm() )
            {
            SwFrm *pTmp = pFrm->GetUpper();
            while ( pTmp && !pTmp->IsFtnFrm() )
                pTmp = pTmp->GetUpper();

            SwFtnFrm *pFtn = (SwFtnFrm*)pTmp;
            while ( pFtn && pFtn->GetMaster() )
                pFtn = pFtn->GetMaster();
            if ( pFtn != pCheck )
            {
                while ( pFtn )
                {
                    SwFtnFrm *pNxt = pFtn->GetFollow();
                    pFtn->Cut();
                    delete pFtn;
                    pFtn = pNxt;
                }
            }
        }

        pFrm = aIter.Next();
    }
}

void SwFtnBossFrm::InsertFtn( SwFtnFrm* pNew )
{
    
    
    
    
    

    ResetFtn( pNew );
    SwFtnFrm *pSibling = FindFirstFtn();
    sal_Bool bDontLeave = sal_False;

    
    
    if( IsInSct() )
    {
        SwSectionFrm* pMySect = ImplFindSctFrm();
        bool bEndnt = pNew->GetAttr()->GetFtn().IsEndNote();
        if( bEndnt )
        {
            const SwSectionFmt* pEndFmt = pMySect->GetEndSectFmt();
            bDontLeave = 0 != pEndFmt;
            if( pSibling )
            {
                if( pEndFmt )
                {
                    if( !pSibling->IsInSct() ||
                        !pSibling->ImplFindSctFrm()->IsDescendantFrom( pEndFmt ) )
                        pSibling = NULL;
                }
                else if( pSibling->IsInSct() )
                    pSibling = NULL;
            }
        }
        else
        {
            bDontLeave = pMySect->IsFtnAtEnd();
            if( pSibling )
            {
                if( pMySect->IsFtnAtEnd() )
                {
                    if( !pSibling->IsInSct() ||
                        !pMySect->IsAnFollow( pSibling->ImplFindSctFrm() ) )
                        pSibling = NULL;
                }
                else if( pSibling->IsInSct() )
                    pSibling = NULL;
            }
        }
    }

    if( pSibling && pSibling->FindPageFrm()->IsEndNotePage() !=
        FindPageFrm()->IsEndNotePage() )
        pSibling = NULL;

    
    SwDoc *pDoc = GetFmt()->GetDoc();
    const sal_uLong nStPos = ::lcl_FindFtnPos( pDoc, pNew->GetAttr() );

    sal_uLong nCmpPos = 0;
    sal_uLong nLastPos = 0;
    SwFtnContFrm *pParent = 0;
    if( pSibling )
    {
        nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
        if( nCmpPos > nStPos )
            pSibling = NULL;
    }

    if ( !pSibling )
    {   pParent = FindFtnCont();
        if ( !pParent )
        {
            
            
            
            pParent = FindNearestFtnCont( bDontLeave );
            if ( pParent )
            {
                SwFtnFrm *pFtn = (SwFtnFrm*)pParent->Lower();
                if ( pFtn )
                {

                    nCmpPos = ::lcl_FindFtnPos( pDoc, pFtn->GetAttr() );
                    if ( nCmpPos > nStPos )
                        pParent = 0;
                }
                else
                    pParent = 0;
            }
        }
        if ( !pParent )
            
            pParent = MakeFtnCont();
        else
        {
            
            
            pSibling = (SwFtnFrm*)pParent->Lower();
            if ( !pSibling )
            { OSL_ENSURE( !this, "No place for a footnote.");
                return;
            }
            nCmpPos  = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );

            SwFtnBossFrm *pNxtB = this; 
            SwFtnFrm  *pLastSib = 0;    

            while ( pSibling && nCmpPos <= nStPos )
            {
                pLastSib = pSibling; 
                nLastPos = nCmpPos;

                while ( pSibling->GetFollow() )
                    pSibling = pSibling->GetFollow();

                if ( pSibling->GetNext() )
                {
                    pSibling = (SwFtnFrm*)pSibling->GetNext();
                    OSL_ENSURE( !pSibling->GetMaster() || ( ENDNOTE > nStPos &&
                            pSibling->GetAttr()->GetFtn().IsEndNote() ),
                            "InsertFtn: Master expected I" );
                }
                else
                {
                    pNxtB = pSibling->FindFtnBossFrm();
                    SwPageFrm *pSibPage = pNxtB->FindPageFrm();
                    sal_Bool bEndNote = pSibPage->IsEndNotePage();
                    sal_Bool bChgPage = lcl_NextFtnBoss( pNxtB, pSibPage, bDontLeave );
                    
                    SwFtnContFrm *pCont = pNxtB && ( !bChgPage ||
                        pSibPage->IsEndNotePage() == bEndNote )
                        ? pNxtB->FindNearestFtnCont( bDontLeave ) : 0;
                    if( pCont )
                        pSibling = (SwFtnFrm*)pCont->Lower();
                    else 
                        break;
                }
                if ( pSibling )
                {
                    nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
                    OSL_ENSURE( nCmpPos > nLastPos, "InsertFtn: Order of FtnFrm's buggy" );
                }
            }
            
            
            if ( pSibling && pLastSib && (pSibling != pLastSib) )
            {
                
                if ( nCmpPos > nStPos )
                    pSibling = pLastSib;
            }
            else if ( !pSibling )
            {
                
                
                
                pSibling = pLastSib;
                while( pSibling->GetFollow() )
                    pSibling = pSibling->GetFollow();
                OSL_ENSURE( !pSibling->GetNext(), "InsertFtn: Who's that guy?" );
            }
        }
    }
    else
    {
        
        
        SwFtnBossFrm* pBoss = pNew->GetRef()->FindFtnBossFrm(
            !pNew->GetAttr()->GetFtn().IsEndNote() );
        sal_uInt16 nRefNum = pBoss->GetPhyPageNum();    
        sal_uInt16 nRefCol = lcl_ColumnNum( pBoss );    
        bool bEnd = false;
        SwFtnFrm *pLastSib = 0;
        while ( pSibling && !bEnd && (nCmpPos <= nStPos) )
        {
            pLastSib = pSibling;
            nLastPos = nCmpPos;

            while ( pSibling->GetFollow() )
                pSibling = pSibling->GetFollow();

            SwFtnFrm *pFoll = (SwFtnFrm*)pSibling->GetNext();
            if ( pFoll )
            {
                pBoss = pSibling->GetRef()->FindFtnBossFrm( !pSibling->
                                            GetAttr()->GetFtn().IsEndNote() );
                sal_uInt16 nTmpRef;
                if( nStPos >= ENDNOTE ||
                    (nTmpRef = pBoss->GetPhyPageNum()) < nRefNum ||
                    ( nTmpRef == nRefNum && lcl_ColumnNum( pBoss ) <= nRefCol ))
                    pSibling = pFoll;
                else
                    bEnd = true;
            }
            else
            {
                SwFtnBossFrm* pNxtB = pSibling->FindFtnBossFrm();
                SwPageFrm *pSibPage = pNxtB->FindPageFrm();
                sal_Bool bEndNote = pSibPage->IsEndNotePage();
                sal_Bool bChgPage = lcl_NextFtnBoss( pNxtB, pSibPage, bDontLeave );
                
                SwFtnContFrm *pCont = pNxtB && ( !bChgPage ||
                    pSibPage->IsEndNotePage() == bEndNote )
                    ? pNxtB->FindNearestFtnCont( bDontLeave ) : 0;
                if ( pCont )
                    pSibling = (SwFtnFrm*)pCont->Lower();
                else
                    bEnd = true;
            }
            if ( !bEnd && pSibling )
                nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
            if ( pSibling && pLastSib && (pSibling != pLastSib) )
            {
                
                if ( (nLastPos < nCmpPos) && (nCmpPos > nStPos) )
                {
                    pSibling = pLastSib;
                    bEnd = true;
                }
            }
        }
    }
    if ( pSibling )
    {
        nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
        if ( nCmpPos < nStPos )
        {
            while ( pSibling->GetFollow() )
                pSibling = pSibling->GetFollow();
            pParent = (SwFtnContFrm*)pSibling->GetUpper();
            pSibling = (SwFtnFrm*)pSibling->GetNext();
        }
        else
        {
            if( pSibling->GetMaster() )
            {
                if( ENDNOTE > nCmpPos || nStPos >= ENDNOTE )
                {
                    OSL_FAIL( "InsertFtn: Master expected II" );
                    do
                        pSibling = pSibling->GetMaster();
                    while ( pSibling->GetMaster() );
                }
            }
            pParent = (SwFtnContFrm*)pSibling->GetUpper();
        }
    }
    OSL_ENSURE( pParent, "paste in space?" );
    pNew->Paste( pParent, pSibling );
}

void SwFtnBossFrm::AppendFtn( SwCntntFrm *pRef, SwTxtFtn *pAttr )
{
    
    if ( FindFtn( pRef, pAttr ) )
        return;

    
    
    
    SwDoc *pDoc = GetFmt()->GetDoc();
    SwFtnBossFrm *pBoss = this;
    SwPageFrm *pPage = FindPageFrm();
    SwPageFrm *pMyPage = pPage;
    bool bChgPage = false;
    sal_Bool bEnd = sal_False;
    if ( pAttr->GetFtn().IsEndNote() )
    {
        bEnd = sal_True;
        if( GetUpper()->IsSctFrm() &&
            ((SwSectionFrm*)GetUpper())->IsEndnAtEnd() )
        {
            SwFrm* pLast =
                ((SwSectionFrm*)GetUpper())->FindLastCntnt( FINDMODE_ENDNOTE );
            if( pLast )
            {
                pBoss = pLast->FindFtnBossFrm();
                pPage = pBoss->FindPageFrm();
            }
        }
        else
        {
            while ( pPage->GetNext() && !pPage->IsEndNotePage() )
            {
                pPage = (SwPageFrm*)pPage->GetNext();
                bChgPage = true;
            }
            if ( !pPage->IsEndNotePage() )
            {
                SwPageDesc *pDesc = pDoc->GetEndNoteInfo().GetPageDesc( *pDoc );
                pPage = ::InsertNewPage( *pDesc, pPage->GetUpper(),
                        !pPage->OnRightPage(), false, false, sal_True, 0 );
                pPage->SetEndNotePage( sal_True );
                bChgPage = true;
            }
            else
            {
                
                
                SwPageFrm *pNxt = (SwPageFrm*)pPage->GetNext();
                const sal_uLong nStPos = ::lcl_FindFtnPos( pDoc, pAttr );
                while ( pNxt && pNxt->IsEndNotePage() )
                {
                    SwFtnContFrm *pCont = pNxt->FindFtnCont();
                    if ( pCont && pCont->Lower() )
                    {
                        OSL_ENSURE( pCont->Lower()->IsFtnFrm(), "no footnote in the container" );
                        if ( nStPos > ::lcl_FindFtnPos( pDoc,
                                        ((SwFtnFrm*)pCont->Lower())->GetAttr()))
                        {
                            pPage = pNxt;
                            pNxt = (SwPageFrm*)pPage->GetNext();
                            continue;
                        }
                    }
                    break;
                }
            }
        }
    }
    else if( FTNPOS_CHAPTER == pDoc->GetFtnInfo().ePos && ( !GetUpper()->
             IsSctFrm() || !((SwSectionFrm*)GetUpper())->IsFtnAtEnd() ) )
    {
        while ( pPage->GetNext() && !pPage->IsFtnPage() &&
                !((SwPageFrm*)pPage->GetNext())->IsEndNotePage() )
        {
            pPage = (SwPageFrm*)pPage->GetNext();
            bChgPage = true;
        }

        if ( !pPage->IsFtnPage() )
        {
            SwPageDesc *pDesc = pDoc->GetFtnInfo().GetPageDesc( *pDoc );
            pPage = ::InsertNewPage( *pDesc, pPage->GetUpper(),
                !pPage->OnRightPage(), false, false, sal_True, pPage->GetNext() );
            bChgPage = true;
        }
        else
        {
            
            
            SwPageFrm *pNxt = (SwPageFrm*)pPage->GetNext();
            const sal_uLong nStPos = ::lcl_FindFtnPos( pDoc, pAttr );
            while ( pNxt && pNxt->IsFtnPage() && !pNxt->IsEndNotePage() )
            {
                SwFtnContFrm *pCont = pNxt->FindFtnCont();
                if ( pCont && pCont->Lower() )
                {
                    OSL_ENSURE( pCont->Lower()->IsFtnFrm(), "no footnote in the container" );
                    if ( nStPos > ::lcl_FindFtnPos( pDoc,
                                        ((SwFtnFrm*)pCont->Lower())->GetAttr()))
                    {
                        pPage = pNxt;
                        pNxt = (SwPageFrm*)pPage->GetNext();
                        continue;
                    }
                }
                break;
            }
        }
    }

    
    if ( !pAttr->GetStartNode() )
    {
        OSL_ENSURE( !this, "no footnote content." );
        return;
    }

    
    
    if( pBoss->IsInSct() && pBoss->IsColumnFrm() && !pPage->IsFtnPage() )
    {
        SwSectionFrm* pSct = pBoss->FindSctFrm();
        if( bEnd ? !pSct->IsEndnAtEnd() : !pSct->IsFtnAtEnd() )
        {
            SwFtnContFrm* pFtnCont = pSct->FindFtnBossFrm(!bEnd)->FindFtnCont();
            if( pFtnCont )
            {
                SwFtnFrm* pTmp = (SwFtnFrm*)pFtnCont->Lower();
                if( bEnd )
                    while( pTmp && !pTmp->GetAttr()->GetFtn().IsEndNote() )
                        pTmp = (SwFtnFrm*)pTmp->GetNext();
                if( pTmp && *pTmp < pAttr )
                    return;
            }
        }
    }

    SwFtnFrm *pNew = new SwFtnFrm( pDoc->GetDfltFrmFmt(), this, pRef, pAttr );
    {
        SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
        ::_InsertCnt( pNew, pDoc, aIdx.GetIndex() );
    }
    
    if( bChgPage )
    {
        SwLayoutFrm* pBody = pPage->FindBodyCont();
        OSL_ENSURE( pBody, "AppendFtn: NoPageBody?" );
        if( pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            pBoss = (SwFtnBossFrm*)pBody->Lower();
        else
            pBoss = pPage; 
    }
    pBoss->InsertFtn( pNew );
    if ( pNew->GetUpper() ) 
    {
        ::RegistFlys( pNew->FindPageFrm(), pNew );
        SwSectionFrm* pSect = FindSctFrm();
        
        
        if( pSect && !pSect->IsJoinLocked() && ( bEnd ? !pSect->IsEndnAtEnd() :
            !pSect->IsFtnAtEnd() ) && pSect->Growable() )
            pSect->InvalidateSize();
        else
        {
            
            
            const bool bOldFtnFrmLocked( pNew->IsColLocked() );
            pNew->ColLock();
            pNew->KeepLockPosOfLowerObjs();
            
            SwCntntFrm *pCnt = pNew->ContainsCntnt();
            while ( pCnt && pCnt->FindFtnFrm()->GetAttr() == pAttr )
            {
                pCnt->Calc();
                
                if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
                {
                    if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                              *(pCnt->FindPageFrm()) ) )
                    {
                        
                        pCnt = pNew->ContainsCntnt();
                        continue;
                    }
                }
                pCnt = (SwCntntFrm*)pCnt->FindNextCnt();
            }
            
            if ( !bOldFtnFrmLocked )
            {
                pNew->ColUnlock();
            }
            
            
            pNew->UnlockPosOfLowerObjs();
            pNew->Calc();
            
            if ( !bOldFtnFrmLocked && !pNew->GetLower() &&
                 !pNew->IsColLocked() && !pNew->IsBackMoveLocked() )
            {
                pNew->Cut();
                delete pNew;
            }
        }
        pMyPage->UpdateFtnNum();
    }
    else
        delete pNew;
}

SwFtnFrm *SwFtnBossFrm::FindFtn( const SwCntntFrm *pRef, const SwTxtFtn *pAttr )
{
    
    OSL_ENSURE( pAttr->GetStartNode(), "FtnAtr without StartNode." );
    SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
    SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
    if ( !pNd )
        pNd = pRef->GetAttrSet()->GetDoc()->
              GetNodes().GoNextSection( &aIdx, true, false );
    if ( !pNd )
        return 0;
    SwIterator<SwFrm,SwCntntNode> aIter( *pNd );
    SwFrm* pFrm = aIter.First();
    if( pFrm )
        do
        {
                pFrm = pFrm->GetUpper();
                
                
                
                
                SwFtnFrm *pFtn = pFrm->ImplFindFtnFrm();
                if ( pFtn && pFtn->GetRef() == pRef )
                {
                    
                    
                    
                    
                    if( pFtn->GetAttr() != pAttr )
                        return 0;
                    while ( pFtn && pFtn->GetMaster() )
                        pFtn = pFtn->GetMaster();
                    return pFtn;
                }

        } while ( 0 != (pFrm = aIter.Next()) );

    return 0;
}

void SwFtnBossFrm::RemoveFtn( const SwCntntFrm *pRef, const SwTxtFtn *pAttr,
                              sal_Bool bPrep )
{
    SwFtnFrm *pFtn = FindFtn( pRef, pAttr );
    if( pFtn )
    {
        do
        {
            SwFtnFrm *pFoll = pFtn->GetFollow();
            pFtn->Cut();
            delete pFtn;
            pFtn = pFoll;
        } while ( pFtn );
        if( bPrep && pRef->IsFollow() )
        {
            OSL_ENSURE( pRef->IsTxtFrm(), "NoTxtFrm has Footnote?" );
            SwTxtFrm* pMaster = (SwTxtFrm*)pRef->FindMaster();
            if( !pMaster->IsLocked() )
                pMaster->Prepare( PREP_FTN_GONE );
        }
    }
    FindPageFrm()->UpdateFtnNum();
}

void SwFtnBossFrm::ChangeFtnRef( const SwCntntFrm *pOld, const SwTxtFtn *pAttr,
                                 SwCntntFrm *pNew )
{
    SwFtnFrm *pFtn = FindFtn( pOld, pAttr );
    while ( pFtn )
    {
        pFtn->SetRef( pNew );
        pFtn = pFtn->GetFollow();
    }
}




void SwFtnBossFrm::CollectFtns( const SwCntntFrm* _pRef,
                                SwFtnBossFrm*     _pOld,
                                SwFtnFrms&        _rFtnArr,
                                const sal_Bool    _bCollectOnlyPreviousFtns )
{
    SwFtnFrm *pFtn = _pOld->FindFirstFtn();
    while( !pFtn )
    {
        if( _pOld->IsColumnFrm() )
        {
            
            while ( !pFtn && _pOld->GetPrev() )
            {
                
                
                _pOld = (SwFtnBossFrm*)_pOld->GetPrev();
                pFtn = _pOld->FindFirstFtn();
            }
        }
        if( !pFtn )
        {
            
            SwPageFrm* pPg;
            for ( SwFrm* pTmp = _pOld;
                  0 != ( pPg = (SwPageFrm*)pTmp->FindPageFrm()->GetPrev())
                    && pPg->IsEmptyPage() ;
                )
            {
                pTmp = pPg;
            }
            if( !pPg )
                return;

            SwLayoutFrm* pBody = pPg->FindBodyCont();
            if( pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            {
                
                _pOld = static_cast<SwFtnBossFrm*>(pBody->GetLastLower());
            }
            else
                _pOld = pPg; 
            pFtn = _pOld->FindFirstFtn();
        }
    }
    
    SwFtnBossFrm* pRefBossFrm = NULL;
    if ( _bCollectOnlyPreviousFtns )
    {
        pRefBossFrm = this;
    }
    _CollectFtns( _pRef, pFtn, _rFtnArr, _bCollectOnlyPreviousFtns, pRefBossFrm );
}

inline void FtnInArr( SwFtnFrms& rFtnArr, SwFtnFrm* pFtn )
{
    if ( rFtnArr.end() == std::find( rFtnArr.begin(), rFtnArr.end(), pFtn ) )
        rFtnArr.push_back( pFtn );
}







void SwFtnBossFrm::_CollectFtns( const SwCntntFrm*   _pRef,
                                 SwFtnFrm*           _pFtn,
                                 SwFtnFrms&          _rFtnArr,
                                 sal_Bool            _bCollectOnlyPreviousFtns,
                                 const SwFtnBossFrm* _pRefFtnBossFrm)
{
    
    
    
    OSL_ENSURE( !_bCollectOnlyPreviousFtns || _pRefFtnBossFrm,
            "<SwFtnBossFrm::_CollectFtns(..)> - No reference footnote boss frame for collecting only previous footnotes set.\nCrash will be caused!" );

    
    

    
    
    
    SwFtnFrms aNotFtnArr;

    
    OSL_ENSURE( !_pFtn->GetMaster() || _pFtn->GetRef() != _pRef, "move FollowFtn?" );
    while ( _pFtn->GetMaster() )
        _pFtn = _pFtn->GetMaster();

    bool bFound = false;

    while ( _pFtn )
    {
        
        
        SwFtnFrm *pNxtFtn = _pFtn;
        while ( pNxtFtn->GetFollow() )
            pNxtFtn = pNxtFtn->GetFollow();
        pNxtFtn = (SwFtnFrm*)pNxtFtn->GetNext();

        if ( !pNxtFtn )
        {
            SwFtnBossFrm* pBoss = _pFtn->FindFtnBossFrm();
            SwPageFrm* pPage = pBoss->FindPageFrm();
            do
            {
                lcl_NextFtnBoss( pBoss, pPage, sal_False );
                if( pBoss )
                {
                    SwLayoutFrm* pCont = pBoss->FindFtnCont();
                    if( pCont )
                    {
                        pNxtFtn = (SwFtnFrm*)pCont->Lower();
                        if( pNxtFtn )
                        {
                            while( pNxtFtn->GetMaster() )
                                pNxtFtn = pNxtFtn->GetMaster();
                            if( pNxtFtn == _pFtn )
                                pNxtFtn = NULL;
                        }
                    }
                }
            } while( !pNxtFtn && pBoss );
        }
        else if( !pNxtFtn->GetAttr()->GetFtn().IsEndNote() )
        { OSL_ENSURE( !pNxtFtn->GetMaster(), "_CollectFtn: Master exspected" );
            while ( pNxtFtn->GetMaster() )
                pNxtFtn = pNxtFtn->GetMaster();
        }
        if ( pNxtFtn == _pFtn )
        {
            OSL_FAIL(   "_CollectFtn: Devil's circle" );
            pNxtFtn = 0;
        }

        
        bool bCollectFoundFtn = false;
        if ( _pFtn->GetRef() == _pRef && !_pFtn->GetAttr()->GetFtn().IsEndNote() )
        {
            if ( _bCollectOnlyPreviousFtns )
            {
                SwFtnBossFrm* pBossOfFoundFtn = _pFtn->FindFtnBossFrm( sal_True );
                OSL_ENSURE( pBossOfFoundFtn,
                        "<SwFtnBossFrm::_CollectFtns(..)> - footnote boss frame of found footnote frame missing.\nWrong layout!" );
                if ( !pBossOfFoundFtn ||    
                     pBossOfFoundFtn->IsBefore( _pRefFtnBossFrm )
                   )
                {
                    bCollectFoundFtn = true;
                }
            }
            else
            {
                bCollectFoundFtn = true;
            }
        }

        if ( bCollectFoundFtn )
        {
            OSL_ENSURE( !_pFtn->GetMaster(), "move FollowFtn?" );
            SwFtnFrm *pNxt = _pFtn->GetFollow();
            while ( pNxt )
            {
                SwFrm *pCnt = pNxt->ContainsAny();
                if ( pCnt )
                {
                    
                    do
                    {   SwFrm *pNxtCnt = pCnt->GetNext();
                        pCnt->Cut();
                        pCnt->Paste( _pFtn );
                        pCnt = pNxtCnt;
                    } while ( pCnt );
                }
                else
                {
                    OSL_ENSURE( !pNxt, "footnote without content?" );
                    pNxt->Cut();
                    delete pNxt;
                }
                pNxt = _pFtn->GetFollow();
            }
            _pFtn->Cut();
            FtnInArr( _rFtnArr, _pFtn );
            bFound = true;
        }
        else
        {
            FtnInArr( aNotFtnArr, _pFtn );
            if( bFound )
                break;
        }
        if ( pNxtFtn &&
             _rFtnArr.end() == std::find( _rFtnArr.begin(), _rFtnArr.end(), pNxtFtn ) &&
             aNotFtnArr.end() == std::find( aNotFtnArr.begin(), aNotFtnArr.end(), pNxtFtn ) )
            _pFtn = pNxtFtn;
        else
            break;
    }
}

void SwFtnBossFrm::_MoveFtns( SwFtnFrms &rFtnArr, sal_Bool bCalc )
{
    
    
    const sal_uInt16 nMyNum = FindPageFrm()->GetPhyPageNum();
    const sal_uInt16 nMyCol = lcl_ColumnNum( this );
    SWRECTFN( this )

    
    
    SwFtnFrm* pLastInsertedFtn = 0L;
    for ( sal_uInt16 i = 0; i < rFtnArr.size(); ++i )
    {
        SwFtnFrm *pFtn = rFtnArr[i];

        SwFtnBossFrm* pRefBoss = pFtn->GetRef()->FindFtnBossFrm( sal_True );
        if( pRefBoss != this )
        {
            const sal_uInt16 nRefNum = pRefBoss->FindPageFrm()->GetPhyPageNum();
            const sal_uInt16 nRefCol = lcl_ColumnNum( this );
            if( nRefNum < nMyNum || ( nRefNum == nMyNum && nRefCol <= nMyCol ) )
                pRefBoss = this;
        }
        pRefBoss->InsertFtn( pFtn );

        if ( pFtn->GetUpper() ) 
        {
            
            
            
            SwFrm *pCnt = pFtn->ContainsAny();
            while( pCnt )
            {
                if( pCnt->IsLayoutFrm() )
                {
                    SwFrm* pTmp = ((SwLayoutFrm*)pCnt)->ContainsAny();
                    while( pTmp && ((SwLayoutFrm*)pCnt)->IsAnLower( pTmp ) )
                    {
                        pTmp->Prepare( PREP_MOVEFTN );
                        (pTmp->Frm().*fnRect->fnSetHeight)(0);
                        (pTmp->Prt().*fnRect->fnSetHeight)(0);
                        pTmp = pTmp->FindNext();
                    }
                }
                else
                    pCnt->Prepare( PREP_MOVEFTN );
                (pCnt->Frm().*fnRect->fnSetHeight)(0);
                (pCnt->Prt().*fnRect->fnSetHeight)(0);
                pCnt = pCnt->GetNext();
            }
            (pFtn->Frm().*fnRect->fnSetHeight)(0);
            (pFtn->Prt().*fnRect->fnSetHeight)(0);
            pFtn->Calc();
            pFtn->GetUpper()->Calc();

            if( bCalc )
            {
                SwTxtFtn *pAttr = pFtn->GetAttr();
                pCnt = pFtn->ContainsAny();
                bool bUnlock = !pFtn->IsBackMoveLocked();
                pFtn->LockBackMove();

                
                
                pFtn->KeepLockPosOfLowerObjs();
                

                while ( pCnt && pCnt->FindFtnFrm()->GetAttr() == pAttr )
                {
                    pCnt->_InvalidatePos();
                    pCnt->Calc();
                    
                    if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                                  *(pCnt->FindPageFrm()) ) )
                        {
                            
                            pCnt = pFtn->ContainsAny();
                            continue;
                        }
                    }
                    if( pCnt->IsSctFrm() )
                    {
                        
                        SwFrm* pTmp = ((SwSectionFrm*)pCnt)->ContainsAny();
                        if( pTmp )
                            pCnt = pTmp;
                        else
                            pCnt = pCnt->FindNext();
                    }
                    else
                        pCnt = pCnt->FindNext();
                }
                if( bUnlock )
                {
                    pFtn->UnlockBackMove();
                    if( !pFtn->ContainsAny() && !pFtn->IsColLocked() )
                    {
                        pFtn->Cut();
                        delete pFtn;
                        
                        pFtn = 0L;
                    }
                }
                
                if ( pFtn )
                {
                    
                    
                    pFtn->UnlockPosOfLowerObjs();
                    pFtn->Calc();
                }
            }
        }
        else
        { OSL_ENSURE( !pFtn->GetMaster() && !pFtn->GetFollow(),
                    "DelFtn and Master/Follow?" );
            delete pFtn;
            
            pFtn = 0L;
        }

        
        if ( pFtn )
        {
            pLastInsertedFtn = pFtn;
        }
    }

    
    
    if ( bCalc && pLastInsertedFtn )
    {
        if ( pLastInsertedFtn->GetNext() )
        {
            SwFtnFrm* pNextFtn = static_cast<SwFtnFrm*>(pLastInsertedFtn->GetNext());
            SwTxtFtn* pAttr = pNextFtn->GetAttr();
            SwFrm* pCnt = pNextFtn->ContainsAny();

            bool bUnlock = !pNextFtn->IsBackMoveLocked();
            pNextFtn->LockBackMove();
            
            
            pNextFtn->KeepLockPosOfLowerObjs();
            

            while ( pCnt && pCnt->FindFtnFrm()->GetAttr() == pAttr )
            {
                pCnt->_InvalidatePos();
                pCnt->Calc();
                
                if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
                {
                    if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                              *(pCnt->FindPageFrm()) ) )
                    {
                        
                        pCnt = pNextFtn->ContainsAny();
                        continue;
                    }
                }
                if( pCnt->IsSctFrm() )
                {
                    
                    SwFrm* pTmp = ((SwSectionFrm*)pCnt)->ContainsAny();
                    if( pTmp )
                        pCnt = pTmp;
                    else
                        pCnt = pCnt->FindNext();
                }
                else
                    pCnt = pCnt->FindNext();
            }
            if( bUnlock )
            {
                pNextFtn->UnlockBackMove();
            }
            
            
            
            pNextFtn->UnlockPosOfLowerObjs();
            pNextFtn->Calc();
        }
    }
}

void SwFtnBossFrm::MoveFtns( const SwCntntFrm *pSrc, SwCntntFrm *pDest,
                             SwTxtFtn *pAttr )
{
    if( ( GetFmt()->GetDoc()->GetFtnInfo().ePos == FTNPOS_CHAPTER &&
        (!GetUpper()->IsSctFrm() || !((SwSectionFrm*)GetUpper())->IsFtnAtEnd()))
        || pAttr->GetFtn().IsEndNote() )
        return;

    OSL_ENSURE( this == pSrc->FindFtnBossFrm( sal_True ),
            "SwPageFrm::MoveFtns: source frame isn't on that FtnBoss" );

    SwFtnFrm *pFtn = FindFirstFtn();
    if( pFtn )
    {
        ChangeFtnRef( pSrc, pAttr, pDest );
        SwFtnBossFrm *pDestBoss = pDest->FindFtnBossFrm( sal_True );
        OSL_ENSURE( pDestBoss, "+SwPageFrm::MoveFtns: no destination boss" );
        if( pDestBoss )     
        {
            SwFtnFrms aFtnArr;
            pDestBoss->_CollectFtns( pDest, pFtn, aFtnArr );
            if ( !aFtnArr.empty() )
            {
                pDestBoss->_MoveFtns( aFtnArr, sal_True );
                SwPageFrm* pSrcPage = FindPageFrm();
                SwPageFrm* pDestPage = pDestBoss->FindPageFrm();
                
                if( pSrcPage != pDestPage )
                {
                    if( pSrcPage->GetPhyPageNum() > pDestPage->GetPhyPageNum() )
                        pSrcPage->UpdateFtnNum();
                    pDestPage->UpdateFtnNum();
                }
            }
        }
    }
}

void SwFtnBossFrm::RearrangeFtns( const SwTwips nDeadLine, const sal_Bool bLock,
                                  const SwTxtFtn *pAttr )
{
    

    SwSaveFtnHeight aSave( this, nDeadLine );
    SwFtnFrm *pFtn = FindFirstFtn();
    if( pFtn && pFtn->GetPrev() && bLock )
    {
        SwFtnFrm* pFirst = (SwFtnFrm*)pFtn->GetUpper()->Lower();
        SwFrm* pCntnt = pFirst->ContainsAny();
        if( pCntnt )
        {
            bool bUnlock = !pFirst->IsBackMoveLocked();
            pFirst->LockBackMove();
            pFirst->Calc();
            pCntnt->Calc();
            
            if ( pCntnt->IsTxtFrm() && pCntnt->IsValid() )
            {
                SwObjectFormatter::FormatObjsAtFrm( *pCntnt,
                                                    *(pCntnt->FindPageFrm()) );
            }
            if( bUnlock )
                pFirst->UnlockBackMove();
        }
        pFtn = FindFirstFtn();
    }
    SwDoc *pDoc = GetFmt()->GetDoc();
    const sal_uLong nFtnPos = pAttr ? ::lcl_FindFtnPos( pDoc, pAttr ) : 0;
    SwFrm *pCnt = pFtn ? pFtn->ContainsAny() : 0;
    if ( pCnt )
    {
        bool bMore = true;
        bool bStart = pAttr == 0; 
        
        
        SwFtnFrm* pLastFtnFrm( 0L );
        
        bool bUnlockLastFtnFrm( false );
        do
        {
            if( !bStart )
                bStart = ::lcl_FindFtnPos( pDoc, pCnt->FindFtnFrm()->GetAttr() )
                         == nFtnPos;
            if( bStart )
            {
                pCnt->_InvalidatePos();
                pCnt->_InvalidateSize();
                pCnt->Prepare( PREP_ADJUST_FRM );
                SwFtnFrm* pFtnFrm = pCnt->FindFtnFrm();
                
                if ( pFtnFrm != pLastFtnFrm )
                {
                    if ( pLastFtnFrm )
                    {
                        if ( !bLock && bUnlockLastFtnFrm )
                        {
                            pLastFtnFrm->ColUnlock();
                        }
                        
                        
                        pLastFtnFrm->UnlockPosOfLowerObjs();
                        pLastFtnFrm->Calc();
                        if ( !bLock && bUnlockLastFtnFrm &&
                             !pLastFtnFrm->GetLower() &&
                             !pLastFtnFrm->IsColLocked() &&
                             !pLastFtnFrm->IsBackMoveLocked() )
                        {
                            pLastFtnFrm->Cut();
                            delete pLastFtnFrm;
                            pLastFtnFrm = 0L;
                        }
                    }
                    if ( !bLock )
                    {
                        bUnlockLastFtnFrm = !pFtnFrm->IsColLocked();
                        pFtnFrm->ColLock();
                    }
                    pFtnFrm->KeepLockPosOfLowerObjs();
                    pLastFtnFrm = pFtnFrm;
                }
                
                
                
                
                {
                    SWRECTFN( this );
                    SwFrm* aFtnContFrm = pFtnFrm->GetUpper();
                    if ( (pFtnFrm->Frm().*fnRect->fnTopDist)((aFtnContFrm->*fnRect->fnGetPrtBottom)()) > 0 )
                    {
                        pFtnFrm->_InvalidatePos();
                    }
                }
                if ( bLock )
                {
                    bool bUnlock = !pFtnFrm->IsBackMoveLocked();
                    pFtnFrm->LockBackMove();
                    pFtnFrm->Calc();
                    pCnt->Calc();
                    
                    if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                                  *(pCnt->FindPageFrm()) ) )
                        {
                            
                            pCnt = pFtn->ContainsAny();
                            continue;
                        }
                    }
                    if( bUnlock )
                    {
                        pFtnFrm->UnlockBackMove();
                        if( !pFtnFrm->Lower() &&
                            !pFtnFrm->IsColLocked() )
                        {
                            
                            OSL_ENSURE( pLastFtnFrm == pFtnFrm,
                                    "<SwFtnBossFrm::RearrangeFtns(..)> - <pLastFtnFrm> != <pFtnFrm>" );
                            pLastFtnFrm = 0L;
                            pFtnFrm->Cut();
                            delete pFtnFrm;
                        }
                    }
                }
                else
                {
                    pFtnFrm->Calc();
                    pCnt->Calc();
                    
                    if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                                  *(pCnt->FindPageFrm()) ) )
                        {
                            
                            pCnt = pFtn->ContainsAny();
                            continue;
                        }
                    }
                }
            }
            SwSectionFrm *pDel = NULL;
            if( pCnt->IsSctFrm() )
            {
                SwFrm* pTmp = ((SwSectionFrm*)pCnt)->ContainsAny();
                if( pTmp )
                {
                    pCnt = pTmp;
                    continue;
                }
                pDel = (SwSectionFrm*)pCnt;
            }
            if ( pCnt->GetNext() )
                pCnt = pCnt->GetNext();
            else
            {
                pCnt = pCnt->FindNext();
                if ( pCnt )
                {
                    SwFtnFrm* pFtnFrm = pCnt->FindFtnFrm();
                    if( pFtnFrm->GetRef()->FindFtnBossFrm(
                        pFtnFrm->GetAttr()->GetFtn().IsEndNote() ) != this )
                        bMore = false;
                }
                else
                    bMore = false;
            }
            if( pDel )
            {
                pDel->Cut();
                delete pDel;
            }
            if ( bMore )
            {
                
                if ( pAttr &&
                     (::lcl_FindFtnPos( pDoc,
                                    pCnt->FindFtnFrm()->GetAttr()) > nFtnPos ) )
                    bMore = false;
            }
        } while ( bMore );
        
        if ( pLastFtnFrm )
        {
            if ( !bLock && bUnlockLastFtnFrm )
            {
                pLastFtnFrm->ColUnlock();
            }
            
            
            pLastFtnFrm->UnlockPosOfLowerObjs();
            pLastFtnFrm->Calc();
            if ( !bLock && bUnlockLastFtnFrm &&
                 !pLastFtnFrm->GetLower() &&
                 !pLastFtnFrm->IsColLocked() &&
                 !pLastFtnFrm->IsBackMoveLocked() )
            {
                pLastFtnFrm->Cut();
                delete pLastFtnFrm;
            }
        }
    }
}

void SwPageFrm::UpdateFtnNum()
{
    
    if ( GetFmt()->GetDoc()->GetFtnInfo().eNum != FTNNUM_PAGE )
        return;

    SwLayoutFrm* pBody = FindBodyCont();
    if( !pBody || !pBody->Lower() )
        return;

    SwCntntFrm* pCntnt = pBody->ContainsCntnt();
    sal_uInt16 nNum = 0;

    while( pCntnt && pCntnt->FindPageFrm() == this )
    {
        if( ((SwTxtFrm*)pCntnt)->HasFtn() )
        {
            SwFtnBossFrm* pBoss = pCntnt->FindFtnBossFrm( sal_True );
            if( pBoss->GetUpper()->IsSctFrm() &&
                ((SwSectionFrm*)pBoss->GetUpper())->IsOwnFtnNum() )
                pCntnt = ((SwSectionFrm*)pBoss->GetUpper())->FindLastCntnt();
            else
            {
                SwFtnFrm* pFtn = (SwFtnFrm*)pBoss->FindFirstFtn( pCntnt );
                while( pFtn )
                {
                    SwTxtFtn* pTxtFtn = pFtn->GetAttr();
                    if( !pTxtFtn->GetFtn().IsEndNote() &&
                         pTxtFtn->GetFtn().GetNumStr().isEmpty() &&
                         !pFtn->GetMaster() &&
                         (pTxtFtn->GetFtn().GetNumber() != ++nNum) )
                    {
                        pTxtFtn->SetNumber( nNum, OUString() );
                    }
                    if ( pFtn->GetNext() )
                        pFtn = (SwFtnFrm*)pFtn->GetNext();
                    else
                    {
                        SwFtnBossFrm* pTmpBoss = pFtn->FindFtnBossFrm( sal_True );
                        if( pTmpBoss )
                        {
                            SwPageFrm* pPage = pTmpBoss->FindPageFrm();
                            pFtn = NULL;
                            lcl_NextFtnBoss( pTmpBoss, pPage, sal_False );
                            SwFtnContFrm *pCont = pTmpBoss ? pTmpBoss->FindNearestFtnCont() : NULL;
                            if ( pCont )
                                pFtn = (SwFtnFrm*)pCont->Lower();
                        }
                    }
                    if( pFtn && pFtn->GetRef() != pCntnt )
                        pFtn = NULL;
                }
            }
        }
        pCntnt = pCntnt->FindNextCnt();
    }
}

void SwFtnBossFrm::SetFtnDeadLine( const SwTwips nDeadLine )
{
    SwFrm *pBody = FindBodyCont();
    pBody->Calc();

    SwFrm *pCont = FindFtnCont();
    const SwTwips nMax = nMaxFtnHeight; 
    SWRECTFN( this )
    if ( pCont )
    {
        pCont->Calc();
        nMaxFtnHeight = -(pCont->Frm().*fnRect->fnBottomDist)( nDeadLine );
    }
    else
        nMaxFtnHeight = -(pBody->Frm().*fnRect->fnBottomDist)( nDeadLine );

    const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
        nMaxFtnHeight += pBody->Grow( LONG_MAX, sal_True );
    if ( IsInSct() )
        nMaxFtnHeight += FindSctFrm()->Grow( LONG_MAX, sal_True );

    if ( nMaxFtnHeight < 0 )
        nMaxFtnHeight = 0;
    if ( nMax != LONG_MAX && nMaxFtnHeight > nMax )
        nMaxFtnHeight = nMax;
}

SwTwips SwFtnBossFrm::GetVarSpace() const
{
    
    

    const SwPageFrm* pPg = FindPageFrm();
    OSL_ENSURE( pPg || IsInSct(), "Footnote lost page" );

    const SwFrm *pBody = FindBodyCont();
    SwTwips nRet;
    if( pBody )
    {
        SWRECTFN( this )
        if( IsInSct() )
        {
            nRet = 0;
            SwTwips nTmp = (*fnRect->fnYDiff)( (pBody->*fnRect->fnGetPrtTop)(),
                                               (Frm().*fnRect->fnGetTop)() );
            const SwSectionFrm* pSect = FindSctFrm();
            
            
            if( pSect->IsEndnAtEnd() ) 
            {
                OSL_ENSURE( !Lower() || !Lower()->GetNext() || Lower()->GetNext()->
                        IsFtnContFrm(), "FtnContainer exspected" );
                const SwFtnContFrm* pCont = Lower() ?
                    (SwFtnContFrm*)Lower()->GetNext() : 0;
                if( pCont )
                {
                    SwFtnFrm* pFtn = (SwFtnFrm*)pCont->Lower();
                    while( pFtn)
                    {
                        if( pFtn->GetAttr()->GetFtn().IsEndNote() )
                        { 
                            SwFrm* pFrm = ((SwLayoutFrm*)Lower())->Lower();
                            if( pFrm )
                            {
                                while( pFrm->GetNext() )
                                    pFrm = pFrm->GetNext(); 
                                nTmp += (*fnRect->fnYDiff)(
                                         (Frm().*fnRect->fnGetTop)(),
                                         (pFrm->Frm().*fnRect->fnGetBottom)() );
                            }
                            break;
                        }
                        pFtn = (SwFtnFrm*)pFtn->GetNext();
                    }
                }
            }
            if( nTmp < nRet )
                nRet = nTmp;
        }
        else
            nRet = - (pPg->Prt().*fnRect->fnGetHeight)()/5;
        nRet += (pBody->Frm().*fnRect->fnGetHeight)();
        if( nRet < 0 )
            nRet = 0;
    }
    else
        nRet = 0;
    if ( IsPageFrm() )
    {
        const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
        if( pSh && pSh->GetViewOptions()->getBrowseMode() )
        nRet += BROWSE_HEIGHT - Frm().Height();
    }
    return nRet;
}

/** Obtain if pFrm's size adjustment should be processed
 *
 * For a page frame of columns directly below the page AdjustNeighbourhood() needs
 * to be called, or Grow()/ Shrink() for frame columns respectively.
 *
 * A column section is special, since if there is a footnote container in a column
 * and those footnotes are not collected, it is handled like a page frame.
 *
 * @see AdjustNeighbourhood()
 * @see Grow()
 * @see Shrink()
 */
sal_uInt8 SwFtnBossFrm::_NeighbourhoodAdjustment( const SwFrm* ) const
{
    sal_uInt8 nRet = NA_ONLY_ADJUST;
    if( GetUpper() && !GetUpper()->IsPageBodyFrm() )
    {
        
        if( GetUpper()->IsFlyFrm() )
            nRet = NA_GROW_SHRINK;
        else
        {
            OSL_ENSURE( GetUpper()->IsSctFrm(), "NeighbourhoodAdjustment: Unexspected Upper" );
            if( !GetNext() && !GetPrev() )
                nRet = NA_GROW_ADJUST; 
            else
            {
                const SwFrm* pTmp = Lower();
                OSL_ENSURE( pTmp, "NeighbourhoodAdjustment: Missing Lower()" );
                if( !pTmp->GetNext() )
                    nRet = NA_GROW_SHRINK;
                else if( !GetUpper()->IsColLocked() )
                    nRet = NA_ADJUST_GROW;
                OSL_ENSURE( !pTmp->GetNext() || pTmp->GetNext()->IsFtnContFrm(),
                        "NeighbourhoodAdjustment: Who's that guy?" );
            }
        }
    }
    return nRet;
}

void SwPageFrm::SetColMaxFtnHeight()
{
    SwLayoutFrm *pBody = FindBodyCont();
    if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
    {
        SwColumnFrm* pCol = (SwColumnFrm*)pBody->Lower();
        do
        {
            pCol->SetMaxFtnHeight( GetMaxFtnHeight() );
            pCol = (SwColumnFrm*)pCol->GetNext();
        } while ( pCol );
    }
}

sal_Bool SwLayoutFrm::MoveLowerFtns( SwCntntFrm *pStart, SwFtnBossFrm *pOldBoss,
                                 SwFtnBossFrm *pNewBoss, const sal_Bool bFtnNums )
{
    SwDoc *pDoc = GetFmt()->GetDoc();
    if ( pDoc->GetFtnIdxs().empty() )
        return sal_False;
    if( pDoc->GetFtnInfo().ePos == FTNPOS_CHAPTER &&
        ( !IsInSct() || !FindSctFrm()->IsFtnAtEnd() ) )
        return sal_True;

    if ( !pNewBoss )
        pNewBoss = FindFtnBossFrm( sal_True );
    if ( pNewBoss == pOldBoss )
        return sal_False;

    sal_Bool bMoved = sal_False;
    if( !pStart )
        pStart = ContainsCntnt();

    SwFtnFrms aFtnArr;

    while ( IsAnLower( pStart ) )
    {
        if ( ((SwTxtFrm*)pStart)->HasFtn() )
        {
            
            
            
            
            
            pNewBoss->CollectFtns( pStart, pOldBoss, aFtnArr, sal_True );
        }
        pStart = pStart->GetNextCntntFrm();
    }

    OSL_ENSURE( pOldBoss->IsInSct() == pNewBoss->IsInSct(),
            "MoveLowerFtns: Section confusion" );
    SwFtnFrms *pFtnArr;
    SwLayoutFrm* pNewChief = 0;
    SwLayoutFrm* pOldChief = 0;
    if( pStart && pOldBoss->IsInSct() && ( pOldChief = pOldBoss->FindSctFrm() )
        != ( pNewChief = pNewBoss->FindSctFrm() ) )
    {
        pFtnArr = new SwFtnFrms;
        pOldChief = pOldBoss->FindFtnBossFrm( sal_True );
        pNewChief = pNewBoss->FindFtnBossFrm( sal_True );
        while( pOldChief->IsAnLower( pStart ) )
        {
            if ( ((SwTxtFrm*)pStart)->HasFtn() )
                ((SwFtnBossFrm*)pNewChief)->CollectFtns( pStart,
                                        (SwFtnBossFrm*)pOldBoss, *pFtnArr );
            pStart = pStart->GetNextCntntFrm();
        }
        if( pFtnArr->empty() )
        {
            delete pFtnArr;
            pFtnArr = NULL;
        }
    }
    else
        pFtnArr = NULL;

    if ( !aFtnArr.empty() || pFtnArr )
    {
        if( !aFtnArr.empty() )
            pNewBoss->_MoveFtns( aFtnArr, sal_True );
        if( pFtnArr )
        {
            ((SwFtnBossFrm*)pNewChief)->_MoveFtns( *pFtnArr, sal_True );
            delete pFtnArr;
        }
        bMoved = sal_True;

        
        if ( bFtnNums )
        {
            SwPageFrm* pOldPage = pOldBoss->FindPageFrm();
            SwPageFrm* pNewPage =pNewBoss->FindPageFrm();
            if( pOldPage != pNewPage )
            {
                pOldPage->UpdateFtnNum();
                pNewPage->UpdateFtnNum();
            }
        }
    }
    return bMoved;
}

sal_Bool SwCntntFrm::MoveFtnCntFwd( sal_Bool bMakePage, SwFtnBossFrm *pOldBoss )
{
    OSL_ENSURE( IsInFtn(), "no footnote." );
    SwLayoutFrm *pFtn = FindFtnFrm();

    
    
    
    if( pOldBoss->IsInSct() && !pOldBoss->GetIndPrev() && !GetIndPrev() &&
        !pFtn->GetPrev() )
    {
        SwLayoutFrm* pBody = pOldBoss->FindBodyCont();
        if( !pBody || !pBody->Lower() )
            return sal_True;
    }

    
    SwLayoutFrm *pNxt = (SwLayoutFrm*)pFtn->GetNext();
    SwLayoutFrm *pLst = 0;
    while ( pNxt )
    {
        while ( pNxt->GetNext() )
            pNxt = (SwLayoutFrm*)pNxt->GetNext();
        if ( pNxt == pLst )
            pNxt = 0;
        else
        {   pLst = pNxt;
            SwCntntFrm *pCnt = pNxt->ContainsCntnt();
            if( pCnt )
                pCnt->MoveFtnCntFwd( sal_True, pOldBoss );
            pNxt = (SwLayoutFrm*)pFtn->GetNext();
        }
    }

    sal_Bool bSamePage = sal_True;
    SwLayoutFrm *pNewUpper =
                GetLeaf( bMakePage ? MAKEPAGE_INSERT : MAKEPAGE_NONE, sal_True );

    if ( pNewUpper )
    {
        bool bSameBoss = true;
        SwFtnBossFrm * const pNewBoss = pNewUpper->FindFtnBossFrm();
        
        if ( !( bSameBoss = (pNewBoss == pOldBoss) ) )
        {
            bSamePage = pOldBoss->FindPageFrm() == pNewBoss->FindPageFrm(); 
            pNewUpper->Calc();
        }

        
        
        
        
        
        
        SwFtnFrm* pTmpFtn = pNewUpper->IsFtnFrm() ? ((SwFtnFrm*)pNewUpper) : 0;
        if( !pTmpFtn )
        {
            OSL_ENSURE( pNewUpper->IsFtnContFrm(), "Neuer Upper kein FtnCont.");
            SwFtnContFrm *pCont = (SwFtnContFrm*)pNewUpper;

            
            SwFtnFrm *pOld = FindFtnFrm();
            pTmpFtn = new SwFtnFrm( pOld->GetFmt()->GetDoc()->GetDfltFrmFmt(),
                                    pOld, pOld->GetRef(), pOld->GetAttr() );
            
            if ( pOld->GetFollow() )
            {
                pTmpFtn->SetFollow( pOld->GetFollow() );
                pOld->GetFollow()->SetMaster( pTmpFtn );
            }
            pOld->SetFollow( pTmpFtn );
            pTmpFtn->SetMaster( pOld );
            SwFrm* pNx = pCont->Lower();
            if( pNx && pTmpFtn->GetAttr()->GetFtn().IsEndNote() )
                while(pNx && !((SwFtnFrm*)pNx)->GetAttr()->GetFtn().IsEndNote())
                    pNx = pNx->GetNext();
            pTmpFtn->Paste( pCont, pNx );
            pTmpFtn->Calc();
        }
        OSL_ENSURE( pTmpFtn->GetAttr() == FindFtnFrm()->GetAttr(), "Wrong Footnote!" );
        
        SwLayoutFrm *pNewUp = pTmpFtn;
        if( IsInSct() )
        {
            SwSectionFrm* pSect = FindSctFrm();
            
            if( pSect->IsInFtn() )
            {
                if( pTmpFtn->Lower() && pTmpFtn->Lower()->IsSctFrm() &&
                    pSect->GetFollow() == (SwSectionFrm*)pTmpFtn->Lower() )
                    pNewUp = (SwSectionFrm*)pTmpFtn->Lower();
                else
                {
                    pNewUp = new SwSectionFrm( *pSect, sal_False );
                    pNewUp->InsertBefore( pTmpFtn, pTmpFtn->Lower() );
                    static_cast<SwSectionFrm*>(pNewUp)->Init();
                    pNewUp->Frm().Pos() = pTmpFtn->Frm().Pos();
                    pNewUp->Frm().Pos().Y() += 1; 

                    
                    
                    SwFrm* pTmp = pSect->GetNext();
                    if( pTmp )
                    {
                        SwFlowFrm* pTmpNxt;
                        if( pTmp->IsCntntFrm() )
                            pTmpNxt = (SwCntntFrm*)pTmp;
                        else if( pTmp->IsSctFrm() )
                            pTmpNxt = (SwSectionFrm*)pTmp;
                        else
                        {
                            OSL_ENSURE( pTmp->IsTabFrm(), "GetNextSctLeaf: Wrong Type" );
                            pTmpNxt = (SwTabFrm*)pTmp;
                        }
                        pTmpNxt->MoveSubTree( pTmpFtn, pNewUp->GetNext() );
                    }
                }
            }
        }

        MoveSubTree( pNewUp, pNewUp->Lower() );

        if( !bSameBoss )
            Prepare( PREP_BOSS_CHGD );
    }
    return bSamePage;
}

SwSaveFtnHeight::SwSaveFtnHeight( SwFtnBossFrm *pBs, const SwTwips nDeadLine ) :
    pBoss( pBs ),
    nOldHeight( pBs->GetMaxFtnHeight() )
{
    pBoss->SetFtnDeadLine( nDeadLine );
    nNewHeight = pBoss->GetMaxFtnHeight();
}

SwSaveFtnHeight::~SwSaveFtnHeight()
{
    
    if ( nNewHeight == pBoss->GetMaxFtnHeight() )
        pBoss->nMaxFtnHeight = nOldHeight;
}

#ifdef DBG_UTIL








const SwCntntFrm* SwFtnFrm::GetRef() const
{
    const SwCntntFrm* pRefAttr = GetRefFromAttr();
    SAL_WARN_IF( pRef != pRefAttr && !pRef->IsAnFollow( pRefAttr )
            && !pRefAttr->IsAnFollow( pRef ),
            "sw", "access to deleted Frame? pRef != pAttr->GetRef()" );
    return pRef;
}

SwCntntFrm* SwFtnFrm::GetRef()
{
    const SwCntntFrm* pRefAttr = GetRefFromAttr();
    SAL_WARN_IF( pRef != pRefAttr && !pRef->IsAnFollow( pRefAttr )
            && !pRefAttr->IsAnFollow( pRef ),
            "sw", "access to deleted Frame? pRef != pAttr->GetRef()" );
    return pRef;
}
#endif

const SwCntntFrm* SwFtnFrm::GetRefFromAttr()  const
{
    SwFtnFrm* pThis = (SwFtnFrm*)this;
    return pThis->GetRefFromAttr();
}

SwCntntFrm* SwFtnFrm::GetRefFromAttr()
{
    OSL_ENSURE( pAttr, "invalid Attribute" );
    SwTxtNode& rTNd = (SwTxtNode&)pAttr->GetTxtNode();
    SwPosition aPos( rTNd, SwIndex( &rTNd, *pAttr->GetStart() ));
    SwCntntFrm* pCFrm = rTNd.getLayoutFrm( getRootFrm(), 0, &aPos, sal_False );
    return pCFrm;
}

/** search for last content in the current footnote frame

    OD 2005-12-02 #i27138#
*/
SwCntntFrm* SwFtnFrm::FindLastCntnt()
{
    SwCntntFrm* pLastCntntFrm( 0L );

    
    
    SwFrm* pLastLowerOfFtn( GetLower() );
    SwFrm* pTmpLastLower( pLastLowerOfFtn );
    while ( pTmpLastLower && pTmpLastLower->GetNext() )
    {
        pTmpLastLower = pTmpLastLower->GetNext();
        if ( ( pTmpLastLower->IsTxtFrm() &&
               !static_cast<SwTxtFrm*>(pTmpLastLower)->IsHiddenNow() ) ||
             ( pTmpLastLower->IsSctFrm() &&
               static_cast<SwSectionFrm*>(pTmpLastLower)->GetSection() &&
               static_cast<SwSectionFrm*>(pTmpLastLower)->ContainsCntnt() ) ||
             ( pTmpLastLower->IsTabFrm() &&
               static_cast<SwTabFrm*>(pTmpLastLower)->ContainsCntnt() ) )
        {
            pLastLowerOfFtn = pTmpLastLower;
        }
    }

    
    if ( pLastLowerOfFtn && pLastLowerOfFtn->IsTabFrm() )
    {
        pLastCntntFrm = static_cast<SwTabFrm*>(pLastLowerOfFtn)->FindLastCntnt();
    }
    else if ( pLastLowerOfFtn && pLastLowerOfFtn->IsSctFrm() )
    {
        pLastCntntFrm = static_cast<SwSectionFrm*>(pLastLowerOfFtn)->FindLastCntnt();
    }
    else
    {
        pLastCntntFrm = dynamic_cast<SwCntntFrm*>(pLastLowerOfFtn);
    }

    return pLastCntntFrm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
