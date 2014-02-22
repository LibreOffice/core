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

#include <svl/smplhint.hxx>
#include <svl/itemiter.hxx>
#include <hints.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <fmtclbl.hxx>
#include "sectfrm.hxx"
#include "section.hxx"
#include "frmtool.hxx"
#include "doc.hxx"
#include "cntfrm.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "fmtpdsc.hxx"
#include "fmtcntnt.hxx"
#include "ndindex.hxx"
#include "ftnidx.hxx"
#include "txtfrm.hxx"
#include "fmtclds.hxx"
#include "colfrm.hxx"
#include "tabfrm.hxx"
#include "flyfrm.hxx"
#include "ftnfrm.hxx"
#include "layouter.hxx"
#include "dbg_lay.hxx"
#include "viewsh.hxx"
#include "viewopt.hxx"
#include "viewimp.hxx"
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brushitem.hxx>
#include <fmtftntx.hxx>

#include <dflyobj.hxx>
#include <flyfrms.hxx>
#include <sortedobjs.hxx>

SwSectionFrm::SwSectionFrm( SwSection &rSect, SwFrm* pSib )
    : SwLayoutFrm( rSect.GetFmt(), pSib )
    , SwFlowFrm( static_cast<SwFrm&>(*this) )
    , pSection( &rSect )
    , bFtnAtEnd(false)
    , bEndnAtEnd(false)
    , bCntntLock(false)
    , bOwnFtnNum(false)
    , bFtnLock(false)
{
    mnType = FRMC_SECTION;

    CalcFtnAtEndFlag();
    CalcEndAtEndFlag();
}

SwSectionFrm::SwSectionFrm( SwSectionFrm &rSect, sal_Bool bMaster ) :
    SwLayoutFrm( rSect.GetFmt(), rSect.getRootFrm() ),
    SwFlowFrm( (SwFrm&)*this ),
    pSection( rSect.GetSection() ),
    bFtnAtEnd( rSect.IsFtnAtEnd() ),
    bEndnAtEnd( rSect.IsEndnAtEnd() ),
    bCntntLock( false ),
    bOwnFtnNum( false ),
    bFtnLock( false )
{
    mnType = FRMC_SECTION;

    PROTOCOL( this, PROT_SECTION, bMaster ? ACT_CREATE_MASTER : ACT_CREATE_FOLLOW, &rSect )

    if( bMaster )
    {
        if( rSect.IsFollow() )
        {
            SwSectionFrm* pMaster = rSect.FindMaster();
            pMaster->SetFollow( this );
        }
        SetFollow( &rSect );
    }
    else
    {
        SetFollow( rSect.GetFollow() );
        rSect.SetFollow( this );
        if( !GetFollow() )
            rSect.SimpleFormat();
        if( !rSect.IsColLocked() )
            rSect.InvalidateSize();
    }
}



void SwSectionFrm::Init()
{
    OSL_ENSURE( GetUpper(), "SwSectionFrm::Init before insertion?!" );
    SWRECTFN( this )
    long nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
    (Frm().*fnRect->fnSetWidth)( nWidth );
    (Frm().*fnRect->fnSetHeight)( 0 );

    
    const SvxLRSpaceItem& rLRSpace = GetFmt()->GetLRSpace();
    (Prt().*fnRect->fnSetLeft)( rLRSpace.GetLeft() );
    (Prt().*fnRect->fnSetWidth)( nWidth - rLRSpace.GetLeft() -
                                 rLRSpace.GetRight() );
    (Prt().*fnRect->fnSetHeight)( 0 );

    const SwFmtCol &rCol = GetFmt()->GetCol();
    if( ( rCol.GetNumCols() > 1 || IsAnyNoteAtEnd() ) && !IsInFtn() )
    {
        const SwFmtCol *pOld = Lower() ? &rCol : new SwFmtCol;
        ChgColumns( *pOld, rCol, IsAnyNoteAtEnd() );
        if( pOld != &rCol )
            delete pOld;
    }
}

SwSectionFrm::~SwSectionFrm()
{
    if( GetFmt() && !GetFmt()->GetDoc()->IsInDtor() )
    {
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm )
            pRootFrm->RemoveFromList( this );
        if( IsFollow() )
        {
            SwSectionFrm *pMaster = FindMaster();
            if( pMaster )
            {
                PROTOCOL( this, PROT_SECTION, ACT_DEL_FOLLOW, pMaster )
                pMaster->SetFollow( GetFollow() );
                
                
                
                
                if( !GetFollow() )
                    pMaster->InvalidateSize();
            }
        }
        else if( HasFollow() )
        {
            PROTOCOL( this, PROT_SECTION, ACT_DEL_MASTER, GetFollow() )
        }
    }
}

void SwSectionFrm::DelEmpty( sal_Bool bRemove )
{
    if( IsColLocked() )
    {
        OSL_ENSURE( !bRemove, "Don't delete locked SectionFrms" );
        return;
    }
    SwFrm* pUp = GetUpper();
    if( pUp )
    {
        
        
        
        
        
        {
            SwViewShell* pViewShell( getRootFrm()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                pViewShell->InvalidateAccessibleParaFlowRelation(
                                dynamic_cast<SwTxtFrm*>(FindNextCnt( true )),
                                dynamic_cast<SwTxtFrm*>(FindPrevCnt( true )) );
            }
        }
        _Cut( bRemove );
    }
    if( IsFollow() )
    {
        SwSectionFrm *pMaster = FindMaster();
        pMaster->SetFollow( GetFollow() );
        
        
        
        
        if( !GetFollow() && !pMaster->IsColLocked() )
            pMaster->InvalidateSize();
    }
    SetFollow(0);
    if( pUp )
    {
        Frm().Height( 0 );
        
        
        if( bRemove )
        {   
            
            
            if( !pSection && getRootFrm() )
                getRootFrm()->RemoveFromList( this );
        }
        else if( getRootFrm() )
            getRootFrm()->InsertEmptySct( this );
        pSection = NULL;  
    }
}

void SwSectionFrm::Cut()
{
    _Cut( sal_True );
}

void SwSectionFrm::_Cut( sal_Bool bRemove )
{
    OSL_ENSURE( GetUpper(), "Cut ohne Upper()." );

    PROTOCOL( this, PROT_CUT, 0, GetUpper() )

    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    SwFrm *pFrm = GetNext();
    SwFrm* pPrepFrm = NULL;
    while( pFrm && pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() )
        pFrm = pFrm->GetNext();
    if( pFrm )
    {   
        
        pFrm->_InvalidatePrt();
        pFrm->_InvalidatePos();
        if( pFrm->IsSctFrm() )
            pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
        if ( pFrm && pFrm->IsCntntFrm() )
        {
            pFrm->InvalidatePage( pPage );
            if( IsInFtn() && !GetIndPrev() )
                pPrepFrm = pFrm;
        }
    }
    else
    {
        InvalidateNextPos();
        
        if ( 0 != (pFrm = GetPrev()) )
        {   pFrm->SetRetouche();
            pFrm->Prepare( PREP_WIDOWS_ORPHANS );
            if ( pFrm->IsCntntFrm() )
                pFrm->InvalidatePage( pPage );
        }
        
        
        
        else
        {   SwRootFrm *pRoot = (SwRootFrm*)pPage->GetUpper();
            pRoot->SetSuperfluous();
            GetUpper()->SetCompletePaint();
        }
    }
    
    SwLayoutFrm *pUp = GetUpper();
    if( bRemove )
    {
        Remove();
        if( pUp && !pUp->Lower() && pUp->IsFtnFrm() && !pUp->IsColLocked() &&
            pUp->GetUpper() )
        {
            pUp->Cut();
            delete pUp;
            pUp = NULL;
        }
    }
    if( pPrepFrm )
        pPrepFrm->Prepare( PREP_FTN );
    if ( pUp )
    {
        SWRECTFN( this );
        SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
        if( nFrmHeight > 0 )
        {
            if( !bRemove )
            {
                (Frm().*fnRect->fnSetHeight)( 0 );
                (Prt().*fnRect->fnSetHeight)( 0 );
            }
            pUp->Shrink( nFrmHeight );
        }
    }
}

void SwSectionFrm::Paste( SwFrm* pParent, SwFrm* pSibling )
{
    OSL_ENSURE( pParent, "No parent for Paste()." );
    OSL_ENSURE( pParent->IsLayoutFrm(), "Parent is CntntFrm." );
    OSL_ENSURE( pParent != this, "I'm my own parent." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetUpper(),
            "I am still registered somewhere." );

    PROTOCOL( this, PROT_PASTE, 0, GetUpper() )

    
    SwSectionFrm* pSect = pParent->FindSctFrm();
    
    
    if ( pSect )
    {
        SwTabFrm* pTableFrm = pParent->FindTabFrm();
        if ( pTableFrm &&
             pSect->IsAnLower( pTableFrm ) )
        {
            pSect = 0;
        }
    }

    SWRECTFN( pParent )
    if( pSect && HasToBreak( pSect ) )
    {
        if( pParent->IsColBodyFrm() ) 
        {
            
            
            
            
            SwColumnFrm *pCol = (SwColumnFrm*)pParent->GetUpper();
            while( !pSibling && 0 != ( pCol = (SwColumnFrm*)pCol->GetNext() ) )
                pSibling = ((SwLayoutFrm*)((SwColumnFrm*)pCol)->Lower())->Lower();
            if( pSibling )
            {
                
                
                
                SwFrm *pTmp = pSibling;
                while ( 0 != ( pCol = (SwColumnFrm*)pCol->GetNext() ) )
                {
                    while ( pTmp->GetNext() )
                        pTmp = pTmp->GetNext();
                    SwFrm* pSave = ::SaveCntnt( pCol );
                    ::RestoreCntnt( pSave, pSibling->GetUpper(), pTmp, true );
                }
            }
        }
        pParent = pSect;
        pSect = new SwSectionFrm( *((SwSectionFrm*)pParent)->GetSection(), pParent );
        
        
        pSect->SetFollow( ((SwSectionFrm*)pParent)->GetFollow() );
        ((SwSectionFrm*)pParent)->SetFollow( NULL );
        if( pSect->GetFollow() )
            pParent->_InvalidateSize();

        InsertGroupBefore( pParent, pSibling, pSect );
        pSect->Init();
        (pSect->*fnRect->fnMakePos)( pSect->GetUpper(), pSect->GetPrev(), sal_True);
        if( !((SwLayoutFrm*)pParent)->Lower() )
        {
            SwSectionFrm::MoveCntntAndDelete( (SwSectionFrm*)pParent, sal_False );
            pParent = this;
        }
    }
    else
        InsertGroupBefore( pParent, pSibling, NULL );

    _InvalidateAll();
    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );

    if ( pSibling )
    {
        pSibling->_InvalidatePos();
        pSibling->_InvalidatePrt();
        if ( pSibling->IsCntntFrm() )
            pSibling->InvalidatePage( pPage );
    }

    SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
    if( nFrmHeight )
        pParent->Grow( nFrmHeight );

    if ( GetPrev() )
    {
        if ( !IsFollow() )
        {
            GetPrev()->InvalidateSize();
            if ( GetPrev()->IsCntntFrm() )
                GetPrev()->InvalidatePage( pPage );
        }
    }
}

/**
|*  Here it's decided whether the this-SectionFrm should break up
|*  the passed (Section)frm (or not).
|*  Initiall, all superior sections are broken up. Later on that could
|*  be made configurable.
|*/
sal_Bool SwSectionFrm::HasToBreak( const SwFrm* pFrm ) const
{
    if( !pFrm->IsSctFrm() )
        return sal_False;

    SwSectionFmt *pTmp = (SwSectionFmt*)GetFmt();

    const SwFrmFmt *pOtherFmt = ((SwSectionFrm*)pFrm)->GetFmt();
    do
    {
        pTmp = pTmp->GetParent();
        if( !pTmp )
            return sal_False;
        if( pTmp == pOtherFmt )
            return sal_True;
    } while( true ); 
}

/**
|*  Merges two SectionFrms, in case it's about the same section.
|*  This can be necessary when a (sub)section is deleted that had
|*  divided another part into two.
|*/
void SwSectionFrm::MergeNext( SwSectionFrm* pNxt )
{
    if( !pNxt->IsJoinLocked() && GetSection() == pNxt->GetSection() )
    {
        PROTOCOL( this, PROT_SECTION, ACT_MERGE, pNxt )

        SwFrm* pTmp = ::SaveCntnt( pNxt );
        if( pTmp )
        {
            SwFrm* pLast = Lower();
            SwLayoutFrm* pLay = this;
            if( pLast )
            {
                while( pLast->GetNext() )
                    pLast = pLast->GetNext();
                if( pLast->IsColumnFrm() )
                {   
                    pLay = (SwLayoutFrm*)((SwLayoutFrm*)pLast)->Lower();
                    pLast = pLay->Lower();
                    if( pLast )
                        while( pLast->GetNext() )
                            pLast = pLast->GetNext();
                }
            }
            ::RestoreCntnt( pTmp, pLay, pLast, true );
        }
        SetFollow( pNxt->GetFollow() );
        pNxt->SetFollow( NULL );
        pNxt->Cut();
        delete pNxt;
        InvalidateSize();
    }
}

/**
|*  Divides a SectionFrm into two parts. The second one starts with the
|*  passed frame.
|*  This is required when inserting an inner section, because the MoveFwd
|*  cannot have the desired effect within a frame or a table cell.
|*/
sal_Bool SwSectionFrm::SplitSect( SwFrm* pFrm, sal_Bool bApres )
{
    OSL_ENSURE( pFrm, "SplitSect: Why?" );
    SwFrm* pOther = bApres ? pFrm->FindNext() : pFrm->FindPrev();
    if( !pOther )
        return sal_False;
    SwSectionFrm* pSect = pOther->FindSctFrm();
    if( pSect != this )
        return sal_False;
    
    SwFrm* pSav = ::SaveCntnt( this, bApres ? pOther : pFrm );
    OSL_ENSURE( pSav, "SplitSect: What's on?" );
    if( pSav ) 
    {   
        SwSectionFrm* pNew = new SwSectionFrm( *pSect->GetSection(), pSect );
        pNew->InsertBehind( pSect->GetUpper(), pSect );
        pNew->Init();
        SWRECTFN( this )
        (pNew->*fnRect->fnMakePos)( NULL, pSect, sal_True );
        
        
        
        
        {
            SwLayoutFrm* pLay = pNew;
            
            while( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
                pLay = (SwLayoutFrm*)pLay->Lower();
            ::RestoreCntnt( pSav, pLay, NULL, true );
        }
        _InvalidateSize();
        if( HasFollow() )
        {
            pNew->SetFollow( GetFollow() );
            SetFollow( NULL );
        }
        return sal_True;
    }
    return sal_False;
}

/**
|*  MoveCntnt is called for destroying a SectionFrms, due to
|*  the cancellation or hiding of a section, to handle the content.
|*  If the SectionFrm hasn't broken up another one, then the content
|*  is moved to the Upper. Otherwise the content is moved to another
|*  SectionFrm, which has to be potentially merged.
|*/


static void lcl_InvalidateInfFlags( SwFrm* pFrm, bool bInva )
{
    while ( pFrm )
    {
        pFrm->InvalidateInfFlags();
        if( bInva )
        {
            pFrm->_InvalidatePos();
            pFrm->_InvalidateSize();
            pFrm->_InvalidatePrt();
        }
        if( pFrm->IsLayoutFrm() )
            lcl_InvalidateInfFlags( ((SwLayoutFrm*)pFrm)->GetLower(), false );
        pFrm = pFrm->GetNext();
    }
}


static SwCntntFrm* lcl_GetNextCntntFrm( const SwLayoutFrm* pLay, bool bFwd )
{
    if ( bFwd )
    {
        if ( pLay->GetNext() && pLay->GetNext()->IsCntntFrm() )
            return (SwCntntFrm*)pLay->GetNext();
    }
    else
    {
        if ( pLay->GetPrev() && pLay->GetPrev()->IsCntntFrm() )
            return (SwCntntFrm*)pLay->GetPrev();
    }

    
    const SwFrm* pFrm = pLay;
    SwCntntFrm *pCntntFrm = 0;
    bool bGoingUp = true;
    do {
        const SwFrm *p = 0;
        bool bGoingFwdOrBwd = false;

        bool bGoingDown = !bGoingUp && ( 0 !=  ( p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0 ) );
        if ( !bGoingDown )
        {
            bGoingFwdOrBwd = ( 0 != ( p = pFrm->IsFlyFrm() ?
                                          ( bFwd ? ((SwFlyFrm*)pFrm)->GetNextLink() : ((SwFlyFrm*)pFrm)->GetPrevLink() ) :
                                          ( bFwd ? pFrm->GetNext() :pFrm->GetPrev() ) ) );
            if ( !bGoingFwdOrBwd )
            {
                bGoingUp = (0 != (p = pFrm->GetUpper() ) );
                if ( !bGoingUp )
                    return 0;
            }
        }

        bGoingUp = !( bGoingFwdOrBwd || bGoingDown );

        if( !bFwd && bGoingDown && p )
            while ( p->GetNext() )
                p = p->GetNext();

        pFrm = p;
    } while ( 0 == (pCntntFrm = (pFrm->IsCntntFrm() ? (SwCntntFrm*)pFrm:0) ));

    return pCntntFrm;
}

#define FIRSTLEAF( pLayFrm ) ( ( pLayFrm->Lower() && pLayFrm->Lower()->IsColumnFrm() )\
                    ? pLayFrm->GetNextLayoutLeaf() \
                    : pLayFrm )

void SwSectionFrm::MoveCntntAndDelete( SwSectionFrm* pDel, sal_Bool bSave )
{
    bool bSize = pDel->Lower() && pDel->Lower()->IsColumnFrm();
    SwFrm* pPrv = pDel->GetPrev();
    SwLayoutFrm* pUp = pDel->GetUpper();
    
    SwSectionFrm* pPrvSct = NULL;
    SwSectionFrm* pNxtSct = NULL;
    SwSectionFmt* pParent = static_cast<SwSectionFmt*>(pDel->GetFmt())->GetParent();
    if( pDel->IsInTab() && pParent )
    {
        SwTabFrm *pTab = pDel->FindTabFrm();
        
        
        if( pTab->IsInSct() && pParent == pTab->FindSctFrm()->GetFmt() )
            pParent = NULL;
    }
    
    
    
    
    
    
    if ( pParent )
    {
        SwFrm* pPrvCntnt = lcl_GetNextCntntFrm( pDel, false );
        pPrvSct = pPrvCntnt ? pPrvCntnt->FindSctFrm() : NULL;
        SwFrm* pNxtCntnt = lcl_GetNextCntntFrm( pDel, true );
        pNxtSct = pNxtCntnt ? pNxtCntnt->FindSctFrm() : NULL;
    }
    else
    {
        pParent = NULL;
        pPrvSct = pNxtSct = NULL;
    }

    
    SwFrm *pSave = bSave ? ::SaveCntnt( pDel ) : NULL;
    sal_Bool bOldFtn = sal_True;
    if( pSave && pUp->IsFtnFrm() )
    {
        bOldFtn = ((SwFtnFrm*)pUp)->IsColLocked();
        ((SwFtnFrm*)pUp)->ColLock();
    }
    pDel->DelEmpty( sal_True );
    delete pDel;
    if( pParent )
    {   
        if( pNxtSct && pNxtSct->GetFmt() == pParent )
        {   
            pUp = FIRSTLEAF( pNxtSct );
            pPrv = NULL;
            if( pPrvSct && !( pPrvSct->GetFmt() == pParent ) )
                pPrvSct = NULL; 
        }
        else if( pPrvSct && pPrvSct->GetFmt() == pParent )
        {   
            pUp = pPrvSct;
            if( pUp->Lower() && pUp->Lower()->IsColumnFrm() )
            {
                pUp = static_cast<SwLayoutFrm*>(pUp->GetLastLower());
                
                pUp = static_cast<SwLayoutFrm*>(pUp->Lower());
            }
            
            pPrv = pUp->GetLastLower();
            pPrvSct = NULL; 
        }
        else
        {
            if( pSave )
            {   
                
                
                
                
                
                pPrvSct = new SwSectionFrm( *pParent->GetSection(), pUp );
                pPrvSct->InsertBehind( pUp, pPrv );
                pPrvSct->Init();
                SWRECTFN( pUp )
                (pPrvSct->*fnRect->fnMakePos)( pUp, pPrv, sal_True );
                pUp = FIRSTLEAF( pPrvSct );
                pPrv = NULL;
            }
            pPrvSct = NULL; 
        }
    }
    
    if( pSave )
    {
        lcl_InvalidateInfFlags( pSave, bSize );
        ::RestoreCntnt( pSave, pUp, pPrv, true );
        pUp->FindPageFrm()->InvalidateCntnt();
        if( !bOldFtn )
            ((SwFtnFrm*)pUp)->ColUnlock();
    }
    
    if( pPrvSct && !pPrvSct->IsJoinLocked() )
    {
        OSL_ENSURE( pNxtSct, "MoveCntnt: No Merge" );
        pPrvSct->MergeNext( pNxtSct );
    }
}

void SwSectionFrm::MakeAll()
{
    if ( IsJoinLocked() || IsColLocked() || StackHack::IsLocked() || StackHack::Count() > 50 )
        return;
    if( !pSection ) 
    {
#ifdef DBG_UTIL
        OSL_ENSURE( getRootFrm()->IsInDelList( this ), "SectionFrm without Section" );
#endif
        if( !mbValidPos )
        {
            if( GetUpper() )
            {
                SWRECTFN( GetUpper() )
                (this->*fnRect->fnMakePos)( GetUpper(), GetPrev(), sal_False );
            }
        }
        mbValidSize = mbValidPos = mbValidPrtArea = sal_True;
        return;
    }
    LockJoin(); 

    while( GetNext() && GetNext() == GetFollow() )
    {
        const SwFrm* pFoll = GetFollow();
        MergeNext( (SwSectionFrm*)GetNext() );
        if( pFoll == GetFollow() )
            break;
    }

    
    
    const SwViewShell *pSh = getRootFrm()->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() &&
         ( Grow( LONG_MAX, true ) > 0 ) )
    {
        while( GetFollow() )
        {
            const SwFrm* pFoll = GetFollow();
            MergeNext( GetFollow() );
            if( pFoll == GetFollow() )
                break;
        }
    }

    
    
    if( !mbValidPos && ToMaximize( sal_False ) )
        mbValidSize = sal_False;

#if OSL_DEBUG_LEVEL > 1
    const SwFmtCol &rCol = GetFmt()->GetCol();
    (void)rCol;
#endif
    SwLayoutFrm::MakeAll();
    UnlockJoin();
    if( pSection && IsSuperfluous() )
        DelEmpty( sal_False );
}

sal_Bool SwSectionFrm::ShouldBwdMoved( SwLayoutFrm *, sal_Bool , sal_Bool & )
{
    OSL_FAIL( "Hups, wo ist meine Tarnkappe?" );
    return sal_False;
}

const SwSectionFmt* SwSectionFrm::_GetEndSectFmt() const
{
    const SwSectionFmt *pFmt = pSection->GetFmt();
    while( !pFmt->GetEndAtTxtEnd().IsAtEnd() )
    {
        if( pFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pFmt = (SwSectionFmt*)pFmt->GetRegisteredIn();
        else
            return NULL;
    }
    return pFmt;
}

static void lcl_FindCntntFrm( SwCntntFrm* &rpCntntFrm, SwFtnFrm* &rpFtnFrm,
    SwFrm* pFrm, sal_Bool &rbChkFtn )
{
    if( pFrm )
    {
        while( pFrm->GetNext() )
            pFrm = pFrm->GetNext();
        while( !rpCntntFrm && pFrm )
        {
            if( pFrm->IsCntntFrm() )
                rpCntntFrm = (SwCntntFrm*)pFrm;
            else if( pFrm->IsLayoutFrm() )
            {
                if( pFrm->IsFtnFrm() )
                {
                    if( rbChkFtn )
                    {
                        rpFtnFrm = (SwFtnFrm*)pFrm;
                        rbChkFtn = rpFtnFrm->GetAttr()->GetFtn().IsEndNote();
                    }
                }
                else
                    lcl_FindCntntFrm( rpCntntFrm, rpFtnFrm,
                        ((SwLayoutFrm*)pFrm)->Lower(), rbChkFtn );
            }
            pFrm = pFrm->GetPrev();
        }
    }
}

SwCntntFrm *SwSectionFrm::FindLastCntnt( sal_uInt8 nMode )
{
    SwCntntFrm *pRet = NULL;
    SwFtnFrm *pFtnFrm = NULL;
    SwSectionFrm *pSect = this;
    if( nMode )
    {
        const SwSectionFmt *pFmt = IsEndnAtEnd() ? GetEndSectFmt() :
                                     pSection->GetFmt();
        do {
            while( pSect->HasFollow() )
                pSect = pSect->GetFollow();
            SwFrm* pTmp = pSect->FindNext();
            while( pTmp && pTmp->IsSctFrm() &&
                   !((SwSectionFrm*)pTmp)->GetSection() )
                pTmp = pTmp->FindNext();
            if( pTmp && pTmp->IsSctFrm() &&
                ((SwSectionFrm*)pTmp)->IsDescendantFrom( pFmt ) )
                pSect = (SwSectionFrm*)pTmp;
            else
                break;
        } while( true );
    }
    sal_Bool bFtnFound = nMode == FINDMODE_ENDNOTE;
    do
    {
        lcl_FindCntntFrm( pRet, pFtnFrm, pSect->Lower(), bFtnFound );
        if( pRet || !pSect->IsFollow() || !nMode ||
            ( FINDMODE_MYLAST == nMode && this == pSect ) )
            break;
        pSect = pSect->FindMaster();
    } while( pSect );
    if( ( nMode == FINDMODE_ENDNOTE ) && pFtnFrm )
        pRet = pFtnFrm->ContainsCntnt();
    return pRet;
}

sal_Bool SwSectionFrm::CalcMinDiff( SwTwips& rMinDiff ) const
{
    if( ToMaximize( sal_True ) )
    {
        SWRECTFN( this )
        rMinDiff = (GetUpper()->*fnRect->fnGetPrtBottom)();
        rMinDiff = (Frm().*fnRect->fnBottomDist)( rMinDiff );
        return sal_True;
    }
    return sal_False;
}

/**
 *  CollectEndnotes looks for endnotes in the sectionfrm and his follows,
 *  the endnotes will cut off the layout and put into the array.
 *  If the first endnote is not a master-SwFtnFrm, the whole sectionfrm
 *  contains only endnotes and it is not necessary to collect them.
 */
static SwFtnFrm* lcl_FindEndnote( SwSectionFrm* &rpSect, bool &rbEmpty,
    SwLayouter *pLayouter )
{
    
    SwSectionFrm* pSect = rbEmpty ? rpSect->GetFollow() : rpSect;
    while( pSect )
    {
       OSL_ENSURE( (pSect->Lower() && pSect->Lower()->IsColumnFrm()) || pSect->GetUpper()->IsFtnFrm(),
                "InsertEndnotes: Where's my column?" );

        
        SwColumnFrm* pCol = 0;
        if(pSect->Lower() && pSect->Lower()->IsColumnFrm())
            pCol = (SwColumnFrm*)pSect->Lower();

        while( pCol ) 
        {
            SwFtnContFrm* pFtnCont = pCol->FindFtnCont();
            if( pFtnCont )
            {
                SwFtnFrm* pRet = (SwFtnFrm*)pFtnCont->Lower();
                while( pRet ) 
                {
                    /* CollectEndNode can destroy pRet so we need to get the
                       next early
                    */
                    SwFtnFrm* pRetNext = (SwFtnFrm*)pRet->GetNext();
                    if( pRet->GetAttr()->GetFtn().IsEndNote() )
                    {
                        if( pRet->GetMaster() )
                        {
                            if( pLayouter )
                                pLayouter->CollectEndnote( pRet );
                            else
                                return 0;
                        }
                        else
                            return pRet; 
                    }
                    pRet = pRetNext;
                }
            }
            pCol = (SwColumnFrm*)pCol->GetNext();
        }
        rpSect = pSect;
        pSect = pLayouter ? pSect->GetFollow() : NULL;
        rbEmpty = true;
    }
    return NULL;
}

static void lcl_ColumnRefresh( SwSectionFrm* pSect, bool bFollow )
{
    while( pSect )
    {
        sal_Bool bOldLock = pSect->IsColLocked();
        pSect->ColLock();
        if( pSect->Lower() && pSect->Lower()->IsColumnFrm() )
        {
            SwColumnFrm *pCol = (SwColumnFrm*)pSect->Lower();
            do
            {   pCol->_InvalidateSize();
                pCol->_InvalidatePos();
                ((SwLayoutFrm*)pCol)->Lower()->_InvalidateSize();
                pCol->Calc();   
                ((SwLayoutFrm*)pCol)->Lower()->Calc();  
                pCol = (SwColumnFrm*)pCol->GetNext();
            } while ( pCol );
        }
        if( !bOldLock )
            pSect->ColUnlock();
        if( bFollow )
            pSect = pSect->GetFollow();
        else
            pSect = NULL;
    }
}

void SwSectionFrm::CollectEndnotes( SwLayouter* pLayouter )
{
    OSL_ENSURE( IsColLocked(), "CollectEndnotes: You love the risk?" );
    
    OSL_ENSURE( (Lower() && Lower()->IsColumnFrm()) || GetUpper()->IsFtnFrm(), "Where's my column?" );

    SwSectionFrm* pSect = this;
    SwFtnFrm* pFtn;
    bool bEmpty = false;
    
    
    
    while( 0 != (pFtn = lcl_FindEndnote( pSect, bEmpty, pLayouter )) )
        pLayouter->CollectEndnote( pFtn );
    if( pLayouter->HasEndnotes() )
        lcl_ColumnRefresh( this, true );
}

/** Fits the size to the surroundings.
|*
|*  Those that have a Follow or foot notes, have to extend until
|*  the lower edge of a upper (bMaximize)
|*  They must not extend above the Upper, as the case may be one can
|*  try to grow its upper (bGrow)
|*  If the size had to be changed, the content is calculated.
|*
|*  @note: perform calculation of content, only if height has changed (OD 18.09.2002 #100522#)
|*/
void SwSectionFrm::_CheckClipping( sal_Bool bGrow, sal_Bool bMaximize )
{
    SWRECTFN( this )
    long nDiff;
    SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
    if( bGrow && ( !IsInFly() || !GetUpper()->IsColBodyFrm() ||
                   !FindFlyFrm()->IsLocked() ) )
    {
        nDiff = -(Frm().*fnRect->fnBottomDist)( nDeadLine );
        if( !bMaximize )
            nDiff += Undersize();
        if( nDiff > 0 )
        {
            long nAdd = GetUpper()->Grow( nDiff );
            if( bVert && !bRev )
                nDeadLine -= nAdd;
            else
                nDeadLine += nAdd;
        }
    }
    nDiff = -(Frm().*fnRect->fnBottomDist)( nDeadLine );
    SetUndersized( !bMaximize && nDiff >= 0 );
    const bool bCalc = ( IsUndersized() || bMaximize ) &&
                       ( nDiff ||
                         (Prt().*fnRect->fnGetTop)() > (Frm().*fnRect->fnGetHeight)() );
    
    
    bool bExtraCalc = false;
    if( !bCalc && !bGrow && IsAnyNoteAtEnd() && !IsInFtn() )
    {
        SwSectionFrm *pSect = this;
        bool bEmpty = false;
        SwLayoutFrm* pFtn = IsEndnAtEnd() ?
            lcl_FindEndnote( pSect, bEmpty, NULL ) : NULL;
        if( pFtn )
        {
            pFtn = pFtn->FindFtnBossFrm();
            SwFrm* pTmp = FindLastCntnt( FINDMODE_LASTCNT );
            
            if ( pTmp && pFtn->IsBefore( pTmp->FindFtnBossFrm() ) )
                bExtraCalc = true;
        }
        else if( GetFollow() && !GetFollow()->ContainsAny() )
            bExtraCalc = true;
    }
    if ( bCalc || bExtraCalc )
    {
        nDiff = (*fnRect->fnYDiff)( nDeadLine, (Frm().*fnRect->fnGetTop)() );
        if( nDiff < 0 )
        {
            nDiff = 0;
            nDeadLine = (Frm().*fnRect->fnGetTop)();
        }
        const Size aOldSz( Prt().SSize() );
        long nTop = (this->*fnRect->fnGetTopMargin)();
        (Frm().*fnRect->fnSetBottom)( nDeadLine );
        nDiff = (Frm().*fnRect->fnGetHeight)();
        if( nTop > nDiff )
            nTop = nDiff;
        (this->*fnRect->fnSetYMargins)( nTop, 0 );

        
        
        
        bool bHeightChanged = bVert ?
                            (aOldSz.Width() != Prt().Width()) :
                            (aOldSz.Height() != Prt().Height());
        
        
        
        
        
        
        
        if ( ( bHeightChanged || bExtraCalc ) && Lower() )
        {
            if( Lower()->IsColumnFrm() )
            {
                lcl_ColumnRefresh( this, false );
                ::CalcCntnt( this );
            }
            else
            {
                ChgLowersProp( aOldSz );
                if( !bMaximize && !IsCntntLocked() )
                    ::CalcCntnt( this );
            }
        }
    }
}

void SwSectionFrm::SimpleFormat()
{
    if ( IsJoinLocked() || IsColLocked() )
        return;
    LockJoin();
    SWRECTFN( this )
    if( GetPrev() || GetUpper() )
    {
        
        const SwLayNotify aNotify( this );
        (this->*fnRect->fnMakePos)( GetUpper(), GetPrev(), sal_False );
        mbValidPos = sal_True;
    }
    SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
    
    
    if( (Frm().*fnRect->fnBottomDist)( nDeadLine ) >= 0 )
    {
        (Frm().*fnRect->fnSetBottom)( nDeadLine );
        long nHeight = (Frm().*fnRect->fnGetHeight)();
        long nTop = CalcUpperSpace();
        if( nTop > nHeight )
            nTop = nHeight;
        (this->*fnRect->fnSetYMargins)( nTop, 0 );
    }
    lcl_ColumnRefresh( this, false );
    UnlockJoin();
}



class ExtraFormatToPositionObjs
{
    private:
        SwSectionFrm* mpSectFrm;
        bool mbExtraFormatPerformed;

    public:
        ExtraFormatToPositionObjs( SwSectionFrm& _rSectFrm)
            : mpSectFrm( &_rSectFrm ),
              mbExtraFormatPerformed( false )
        {}

        ~ExtraFormatToPositionObjs()
        {
            if ( mbExtraFormatPerformed )
            {
                
                SwPageFrm* pPageFrm = mpSectFrm->FindPageFrm();
                SwSortedObjs* pObjs = pPageFrm ? pPageFrm->GetSortedObjs() : 0L;
                if ( pObjs )
                {
                    sal_uInt32 i = 0;
                    for ( i = 0; i < pObjs->Count(); ++i )
                    {
                        SwAnchoredObject* pAnchoredObj = (*pObjs)[i];

                        if ( mpSectFrm->IsAnLower( pAnchoredObj->GetAnchorFrm() ) )
                        {
                            pAnchoredObj->SetKeepPosLocked( false );
                        }
                    }
                }
            }
        }

        
        void InitObjs( SwFrm& rFrm )
        {
            SwSortedObjs* pObjs = rFrm.GetDrawObjs();
            if ( pObjs )
            {
                sal_uInt32 i = 0;
                for ( i = 0; i < pObjs->Count(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pObjs)[i];

                    pAnchoredObj->UnlockPosition();
                    pAnchoredObj->SetClearedEnvironment( false );
                }
            }
            SwLayoutFrm* pLayoutFrm = dynamic_cast<SwLayoutFrm*>(&rFrm);
            if ( pLayoutFrm != 0 )
            {
                SwFrm* pLowerFrm = pLayoutFrm->GetLower();
                while ( pLowerFrm != 0 )
                {
                    InitObjs( *pLowerFrm );

                    pLowerFrm = pLowerFrm->GetNext();
                }
            }
        }

        void FormatSectionToPositionObjs()
        {
            
            if ( mpSectFrm->Lower() && mpSectFrm->Lower()->IsColumnFrm() &&
                 mpSectFrm->Lower()->GetNext() )
            {
                
                SWRECTFN( mpSectFrm );
                SwTwips nTopMargin = (mpSectFrm->*fnRect->fnGetTopMargin)();
                Size aOldSectPrtSize( mpSectFrm->Prt().SSize() );
                SwTwips nDiff = (mpSectFrm->Frm().*fnRect->fnBottomDist)(
                                        (mpSectFrm->GetUpper()->*fnRect->fnGetPrtBottom)() );
                (mpSectFrm->Frm().*fnRect->fnAddBottom)( nDiff );
                (mpSectFrm->*fnRect->fnSetYMargins)( nTopMargin, 0 );
                
                
                if ( (mpSectFrm->Prt().*fnRect->fnGetHeight)() <= 0 )
                {
                    return;
                }
                mpSectFrm->ChgLowersProp( aOldSectPrtSize );

                
                SwColumnFrm* pColFrm = static_cast<SwColumnFrm*>(mpSectFrm->Lower());
                while ( pColFrm )
                {
                    pColFrm->Calc();
                    pColFrm->Lower()->Calc();
                    if ( pColFrm->Lower()->GetNext() )
                    {
                        pColFrm->Lower()->GetNext()->Calc();
                    }

                    pColFrm = static_cast<SwColumnFrm*>(pColFrm->GetNext());
                }

                
                
                
                
                
                InitObjs( *mpSectFrm );

                
                
                ::CalcCntnt( mpSectFrm );
                ::CalcCntnt( mpSectFrm, true );

                
                SwPageFrm* pPageFrm = mpSectFrm->FindPageFrm();
                SwSortedObjs* pObjs = pPageFrm ? pPageFrm->GetSortedObjs() : 0L;
                if ( pObjs )
                {
                    sal_uInt32 i = 0;
                    for ( i = 0; i < pObjs->Count(); ++i )
                    {
                        SwAnchoredObject* pAnchoredObj = (*pObjs)[i];

                        if ( mpSectFrm->IsAnLower( pAnchoredObj->GetAnchorFrm() ) )
                        {
                            pAnchoredObj->SetKeepPosLocked( true );
                        }
                    }
                }

                mbExtraFormatPerformed = true;
            }
        }
};


void SwSectionFrm::Format( const SwBorderAttrs *pAttr )
{
    if( !pSection ) 
    {
#ifdef DBG_UTIL
        OSL_ENSURE( getRootFrm()->IsInDelList( this ), "SectionFrm without Section" );
#endif
        mbValidSize = mbValidPos = mbValidPrtArea = sal_True;
        return;
    }
    SWRECTFN( this )
    if ( !mbValidPrtArea )
    {
        PROTOCOL( this, PROT_PRTAREA, 0, 0 )
        mbValidPrtArea = sal_True;
        SwTwips nUpper = CalcUpperSpace();

        
        const SvxLRSpaceItem& rLRSpace = GetFmt()->GetLRSpace();
        (this->*fnRect->fnSetXMargins)( rLRSpace.GetLeft(), rLRSpace.GetRight() );

        if( nUpper != (this->*fnRect->fnGetTopMargin)() )
        {
            mbValidSize = sal_False;
            SwFrm* pOwn = ContainsAny();
            if( pOwn )
                pOwn->_InvalidatePos();
        }
        (this->*fnRect->fnSetYMargins)( nUpper, 0 );
    }

    if ( !mbValidSize )
    {
        PROTOCOL_ENTER( this, PROT_SIZE, 0, 0 )
        const long nOldHeight = (Frm().*fnRect->fnGetHeight)();
        sal_Bool bOldLock = IsColLocked();
        ColLock();

        mbValidSize = sal_True;

        
        
        
        sal_Bool bMaximize = ToMaximize( sal_False );

        
        
        
        
        
        
        
        ExtraFormatToPositionObjs aExtraFormatToPosObjs( *this );
        if ( !bMaximize &&
             GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) &&
             !GetFmt()->GetBalancedColumns().GetValue() )
        {
            aExtraFormatToPosObjs.FormatSectionToPositionObjs();
        }

        
        
        
        const bool bHasColumns = Lower() && Lower()->IsColumnFrm();
        if ( bHasColumns && Lower()->GetNext() )
            AdjustColumns( 0, sal_False );

        if( GetUpper() )
        {
            long nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            (maFrm.*fnRect->fnSetWidth)( nWidth );

            
            const SvxLRSpaceItem& rLRSpace = GetFmt()->GetLRSpace();
            (maPrt.*fnRect->fnSetWidth)( nWidth - rLRSpace.GetLeft() -
                                        rLRSpace.GetRight() );

            
            
            
            const SwViewShell *pSh = getRootFrm()->GetCurrShell();
            _CheckClipping( pSh && pSh->GetViewOptions()->getBrowseMode(), bMaximize );
            bMaximize = ToMaximize( sal_False );
            mbValidSize = sal_True;
        }

        
        if ( bHasColumns && ! Lower()->GetNext() && bMaximize )
            ((SwColumnFrm*)Lower())->Lower()->Calc();

        if ( !bMaximize )
        {
            SwTwips nRemaining = (this->*fnRect->fnGetTopMargin)();
            SwFrm *pFrm = pLower;
            if( pFrm )
            {
                if( pFrm->IsColumnFrm() && pFrm->GetNext() )
                {
                    
                    
                    if ( (GetUpper()->Frm().*fnRect->fnGetHeight)() > 0 )
                    {
                        FormatWidthCols( *pAttr, nRemaining, MINLAY );
                    }
                    
                    
                    while( HasFollow() && !GetFollow()->ContainsCntnt() &&
                           !GetFollow()->ContainsAny( true ) )
                    {
                        SwFrm* pOld = GetFollow();
                        GetFollow()->DelEmpty( sal_False );
                        if( pOld == GetFollow() )
                            break;
                    }
                    bMaximize = ToMaximize( sal_False );
                    nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
                }
                else
                {
                    if( pFrm->IsColumnFrm() )
                    {
                        pFrm->Calc();
                        pFrm = ((SwColumnFrm*)pFrm)->Lower();
                        pFrm->Calc();
                        pFrm = ((SwLayoutFrm*)pFrm)->Lower();
                        CalcFtnCntnt();
                    }
                    
                    
                    if( pFrm && !pFrm->IsValid() && IsInFly() &&
                        FindFlyFrm()->IsColLocked() )
                        ::CalcCntnt( this );
                    nRemaining += InnerHeight();
                    bMaximize = HasFollow();
                }
            }

            SwTwips nDiff = (Frm().*fnRect->fnGetHeight)() - nRemaining;
            if( nDiff < 0)
            {
                SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
                {
                    long nBottom = (Frm().*fnRect->fnGetBottom)();
                    nBottom = (*fnRect->fnYInc)( nBottom, -nDiff );
                    long nTmpDiff = (*fnRect->fnYDiff)( nBottom, nDeadLine );
                    if( nTmpDiff > 0 )
                    {
                        nTmpDiff = GetUpper()->Grow( nTmpDiff, sal_True );
                        nDeadLine = (*fnRect->fnYInc)( nDeadLine, nTmpDiff );
                        nTmpDiff = (*fnRect->fnYDiff)( nBottom, nDeadLine );
                        if( nTmpDiff > 0 )
                            nDiff += nTmpDiff;
                        if( nDiff > 0 )
                            nDiff = 0;
                    }
                }
            }
            if( nDiff )
            {
                long nTmp = nRemaining - (Frm().*fnRect->fnGetHeight)();
                long nTop = (this->*fnRect->fnGetTopMargin)();
                (Frm().*fnRect->fnAddBottom)( nTmp );
                (this->*fnRect->fnSetYMargins)( nTop, 0 );
                InvalidateNextPos();
                if( pLower && ( !pLower->IsColumnFrm() || !pLower->GetNext() ) )
                {
                    
                    
                    
                    pFrm = pLower;
                    if( pFrm->IsColumnFrm() )
                    {
                        pFrm->_InvalidateSize();
                        pFrm->_InvalidatePos();
                        pFrm->Calc();
                        pFrm = ((SwColumnFrm*)pFrm)->Lower();
                        pFrm->Calc();
                        pFrm = ((SwLayoutFrm*)pFrm)->Lower();
                        CalcFtnCntnt();
                    }
                    bool bUnderSz = false;
                    while( pFrm )
                    {
                        if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
                        {
                            pFrm->Prepare( PREP_ADJUST_FRM );
                            bUnderSz = true;
                        }
                        pFrm = pFrm->GetNext();
                    }
                    if( bUnderSz && !IsCntntLocked() )
                        ::CalcCntnt( this );
                }
            }
        }

        
        
        if ( GetUpper() )
            _CheckClipping( sal_True, bMaximize );
        if( !bOldLock )
            ColUnlock();
        long nDiff = nOldHeight - (Frm().*fnRect->fnGetHeight)();
        if( nDiff > 0 )
        {
            if( !GetNext() )
                SetRetouche(); 
            if( GetUpper() && !GetUpper()->IsFooterFrm() )
                GetUpper()->Shrink( nDiff );
        }
        if( IsUndersized() )
            mbValidPrtArea = sal_True;
    }
}



SwLayoutFrm *SwFrm::GetNextSctLeaf( MakePageType eMakePage )
{
    

    PROTOCOL_ENTER( this, PROT_LEAF, ACT_NEXT_SECT, GetUpper()->FindSctFrm() )

    
    
    if( IsColBodyFrm() && GetUpper()->GetNext() )
        return (SwLayoutFrm*)((SwLayoutFrm*)GetUpper()->GetNext())->Lower();
    if( GetUpper()->IsColBodyFrm() && GetUpper()->GetUpper()->GetNext() )
        return (SwLayoutFrm*)((SwLayoutFrm*)GetUpper()->GetUpper()->GetNext())->Lower();
    
    
    if( GetUpper()->IsInTab() || FindFooterOrHeader() )
        return 0;

    SwSectionFrm *pSect = FindSctFrm();
    bool bWrongPage = false;
    OSL_ENSURE( pSect, "GetNextSctLeaf: Missing SectionFrm" );

    
    
    
    
    if( pSect->HasFollow() && pSect->IsInDocBody() )
    {
        if( pSect->GetFollow() == pSect->GetNext() )
        {
            SwPageFrm *pPg = pSect->GetFollow()->FindPageFrm();
            if( WrongPageDesc( pPg ) )
                bWrongPage = true;
            else
                return FIRSTLEAF( pSect->GetFollow() );
        }
        else
        {
            SwFrm* pTmp;
            if( !pSect->GetUpper()->IsColBodyFrm() ||
                0 == ( pTmp = pSect->GetUpper()->GetUpper()->GetNext() ) )
                pTmp = pSect->FindPageFrm()->GetNext();
            if( pTmp ) 
            {
                SwFrm* pTmpX = pTmp;
                if( pTmp->IsPageFrm() && ((SwPageFrm*)pTmp)->IsEmptyPage() )
                    pTmp = pTmp->GetNext(); 
                SwFrm *pUp = pSect->GetFollow()->GetUpper();
                
                
                if( !pUp->IsColBodyFrm() ||
                    !( pUp = pUp->GetUpper() )->GetPrev() )
                    pUp = pUp->FindPageFrm();
                
                
                if( pUp == pTmp || pUp->GetNext() == pTmpX )
                {
                    SwPageFrm* pNxtPg = pUp->IsPageFrm() ?
                                        (SwPageFrm*)pUp : pUp->FindPageFrm();
                    if( WrongPageDesc( pNxtPg ) )
                        bWrongPage = true;
                    else
                        return FIRSTLEAF( pSect->GetFollow() );
                }
            }
        }
    }

    
    const sal_Bool bBody = IsInDocBody();
    const sal_Bool bFtnPage = FindPageFrm()->IsFtnPage();

    SwLayoutFrm *pLayLeaf;
    
    if( bWrongPage )
        pLayLeaf = 0;
    else if( IsTabFrm() )
    {
        SwCntntFrm* pTmpCnt = ((SwTabFrm*)this)->FindLastCntnt();
        pLayLeaf = pTmpCnt ? pTmpCnt->GetUpper() : 0;
    }
    else
    {
        pLayLeaf = GetNextLayoutLeaf();
        if( IsColumnFrm() )
        {
            while( pLayLeaf && ((SwColumnFrm*)this)->IsAnLower( pLayLeaf ) )
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        }
    }

    SwLayoutFrm *pOldLayLeaf = 0;           
                                            
                                            

    while( true )
    {
        if( pLayLeaf )
        {
            
            
            
            SwPageFrm* pNxtPg = pLayLeaf->FindPageFrm();
            if ( !bFtnPage && pNxtPg->IsFtnPage() )
            {   
                pLayLeaf = 0;
                continue;
            }
            
            if ( (bBody && !pLayLeaf->IsInDocBody()) ||
                 (IsInFtn() != pLayLeaf->IsInFtn() ) ||
                 pLayLeaf->IsInTab() ||
                 ( pLayLeaf->IsInSct() && ( !pSect->HasFollow()
                   || pSect->GetFollow() != pLayLeaf->FindSctFrm() ) ) )
            {
                
                pOldLayLeaf = pLayLeaf;
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
                continue;
            }
            if( WrongPageDesc( pNxtPg ) )
            {
                if( bWrongPage )
                    break; 
                pLayLeaf = 0;
                bWrongPage = true;
                pOldLayLeaf = 0;
                continue;
            }
        }
        
        
        else if( !pSect->IsInFly() &&
            ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT ) )
        {
            InsertPage(pOldLayLeaf ? pOldLayLeaf->FindPageFrm() : FindPageFrm(),
                       sal_False );
            
            pLayLeaf = pOldLayLeaf ? pOldLayLeaf : GetNextLayoutLeaf();
            continue;
        }
        break;
    }

    if( pLayLeaf )
    {
        
        
        
        SwSectionFrm* pNew;

        
        SwFrm* pFirst = pLayLeaf->Lower();
        
        while( pFirst && pFirst->IsSctFrm() && !((SwSectionFrm*)pFirst)->GetSection() )
            pFirst = pFirst->GetNext();
        if( pFirst && pFirst->IsSctFrm() && pSect->GetFollow() == pFirst )
            pNew = pSect->GetFollow();
        else if( MAKEPAGE_NOSECTION == eMakePage )
            return pLayLeaf;
        else
        {
            pNew = new SwSectionFrm( *pSect, sal_False );
            pNew->InsertBefore( pLayLeaf, pLayLeaf->Lower() );
            pNew->Init();
            SWRECTFN( pNew )
            (pNew->*fnRect->fnMakePos)( pLayLeaf, NULL, sal_True );

            
            
            SwFrm* pTmp = pSect->GetNext();
            if( pTmp && pTmp != pSect->GetFollow() )
            {
                SwFlowFrm* pNxt;
                SwCntntFrm* pNxtCntnt = NULL;
                if( pTmp->IsCntntFrm() )
                {
                    pNxt = (SwCntntFrm*)pTmp;
                    pNxtCntnt = (SwCntntFrm*)pTmp;
                }
                else
                {
                    pNxtCntnt = ((SwLayoutFrm*)pTmp)->ContainsCntnt();
                    if( pTmp->IsSctFrm() )
                        pNxt = (SwSectionFrm*)pTmp;
                    else
                    {
                        OSL_ENSURE( pTmp->IsTabFrm(), "GetNextSctLeaf: Wrong Type" );
                        pNxt = (SwTabFrm*)pTmp;
                    }
                    while( !pNxtCntnt && 0 != ( pTmp = pTmp->GetNext() ) )
                    {
                        if( pTmp->IsCntntFrm() )
                            pNxtCntnt = (SwCntntFrm*)pTmp;
                        else
                            pNxtCntnt = ((SwLayoutFrm*)pTmp)->ContainsCntnt();
                    }
                }
                if( pNxtCntnt )
                {
                    SwFtnBossFrm* pOldBoss = pSect->FindFtnBossFrm( sal_True );
                    if( pOldBoss == pNxtCntnt->FindFtnBossFrm( sal_True ) )
                    {
                        SwSaveFtnHeight aHeight( pOldBoss,
                            pOldBoss->Frm().Top() + pOldBoss->Frm().Height() );
                        pSect->GetUpper()->MoveLowerFtns( pNxtCntnt, pOldBoss,
                                    pLayLeaf->FindFtnBossFrm( sal_True ), sal_False );
                    }
                }
                ((SwFlowFrm*)pNxt)->MoveSubTree( pLayLeaf, pNew->GetNext() );
            }
            if( pNew->GetFollow() )
                pNew->SimpleFormat();
        }
        
        pLayLeaf = FIRSTLEAF( pNew );
    }
    return pLayLeaf;
}


SwLayoutFrm *SwFrm::GetPrevSctLeaf( MakePageType )
{
    PROTOCOL_ENTER( this, PROT_LEAF, ACT_PREV_SECT, GetUpper()->FindSctFrm() )

    SwLayoutFrm* pCol;
    
    if( IsColBodyFrm() )
        pCol = GetUpper();
    else if( GetUpper()->IsColBodyFrm() )
        pCol = GetUpper()->GetUpper();
    else
        pCol = NULL;
    bool bJump = false;
    if( pCol )
    {
        if( pCol->GetPrev() )
        {
            do
            {
                pCol = (SwLayoutFrm*)pCol->GetPrev();
                
                if( ((SwLayoutFrm*)pCol->Lower())->Lower() )
                {
                    if( bJump )     
                        SwFlowFrm::SetMoveBwdJump( sal_True );
                    return (SwLayoutFrm*)pCol->Lower();  
                }
                bJump = true;
            } while( pCol->GetPrev() );

            
            
            pCol = (SwLayoutFrm*)pCol->Lower();
        }
        else
            pCol = NULL;
    }

    if( bJump )     
        SwFlowFrm::SetMoveBwdJump( sal_True );

    
    
    
    
    OSL_ENSURE( FindSctFrm(), "GetNextSctLeaf: Missing SectionFrm" );
    if( ( IsInTab() && !IsTabFrm() ) || FindFooterOrHeader() )
        return pCol;

    
    
    
    
    SwSectionFrm *pSect = FindSctFrm();

    
    
    
    
    
    if ( IsTabFrm() && pSect->IsInTab() )
    {
        return pCol;
    }

    {
        SwFrm *pPrv;
        if( 0 != ( pPrv = pSect->GetIndPrev() ) )
        {
            
            while( pPrv && pPrv->IsSctFrm() && !((SwSectionFrm*)pPrv)->GetSection() )
                pPrv = pPrv->GetPrev();
            if( pPrv )
                return pCol;
        }
    }

    const sal_Bool bBody = IsInDocBody();
    const sal_Bool bFly  = IsInFly();

    SwLayoutFrm *pLayLeaf = GetPrevLayoutLeaf();
    SwLayoutFrm *pPrevLeaf = 0;

    while ( pLayLeaf )
    {
        
        if ( pLayLeaf->IsInTab() || pLayLeaf->IsInSct() )
        {
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
        }
        else if ( bBody && pLayLeaf->IsInDocBody() )
        {
            
            
            const SwFrm* pTmp = pLayLeaf->Lower();
            
            
            
            while ( pTmp && pTmp->IsSctFrm() &&
                    !( static_cast<const SwSectionFrm*>(pTmp)->GetSection() ) &&
                    pTmp->GetNext()
                  )
            {
                pTmp = pTmp->GetNext();
            }
            if ( pTmp &&
                 ( !pTmp->IsSctFrm() ||
                   ( static_cast<const SwSectionFrm*>(pTmp)->GetSection() )
                 )
               )
            {
                break;
            }
            pPrevLeaf = pLayLeaf;
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
            if ( pLayLeaf )
                SwFlowFrm::SetMoveBwdJump( sal_True );
        }
        else if ( bFly )
            break;  
        else
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
    }
    if( !pLayLeaf )
    {
        if( !pPrevLeaf )
            return pCol;
        pLayLeaf = pPrevLeaf;
    }

    SwSectionFrm* pNew = NULL;
    
    SwFrm *pTmp = pLayLeaf->Lower();
    if( pTmp )
    {
        while( pTmp->GetNext() )
            pTmp = pTmp->GetNext();
        if( pTmp->IsSctFrm() )
        {
            
            while( !((SwSectionFrm*)pTmp)->GetSection() && pTmp->GetPrev() &&
                    pTmp->GetPrev()->IsSctFrm() )
                pTmp = pTmp->GetPrev();
            if( ((SwSectionFrm*)pTmp)->GetFollow() == pSect )
                pNew = (SwSectionFrm*)pTmp;
        }
    }
    if( !pNew )
    {
        pNew = new SwSectionFrm( *pSect, sal_True );
        pNew->InsertBefore( pLayLeaf, NULL );
        pNew->Init();
        SWRECTFN( pNew )
        (pNew->*fnRect->fnMakePos)( pLayLeaf, pNew->GetPrev(), sal_True );

        pLayLeaf = FIRSTLEAF( pNew );
        if( !pNew->Lower() )    
        {
            pNew->MakePos();
            pLayLeaf->Format(); 
        }
        else
            pNew->SimpleFormat();
    }
    else
    {
        pLayLeaf = FIRSTLEAF( pNew );
        if( pLayLeaf->IsColBodyFrm() )
        {
            
            
            SwLayoutFrm *pTmpLay = pLayLeaf;
            while( pLayLeaf->GetUpper()->GetNext() )
            {
                pLayLeaf = (SwLayoutFrm*)((SwLayoutFrm*)pLayLeaf->GetUpper()->GetNext())->Lower();
                if( pLayLeaf->Lower() )
                    pTmpLay = pLayLeaf;
            }
            
            if( pLayLeaf != pTmpLay )
            {
                pLayLeaf = pTmpLay;
                SwFlowFrm::SetMoveBwdJump( sal_True );
            }
        }
    }
    return pLayLeaf;
}

static SwTwips lcl_DeadLine( const SwFrm* pFrm )
{
    const SwLayoutFrm* pUp = pFrm->GetUpper();
    while( pUp && pUp->IsInSct() )
    {
        if( pUp->IsSctFrm() )
            pUp = pUp->GetUpper();
        
        else if( pUp->IsColBodyFrm() && pUp->GetUpper()->GetUpper()->IsSctFrm() )
            pUp = pUp->GetUpper()->GetUpper();
        else
            break;
    }
    SWRECTFN( pFrm )
    return pUp ? (pUp->*fnRect->fnGetPrtBottom)() :
                 (pFrm->Frm().*fnRect->fnGetBottom)();
}


sal_Bool SwSectionFrm::Growable() const
{
    SWRECTFN( this )
    if( (*fnRect->fnYDiff)( lcl_DeadLine( this ),
        (Frm().*fnRect->fnGetBottom)() ) > 0 )
        return sal_True;

    return ( GetUpper() && ((SwFrm*)GetUpper())->Grow( LONG_MAX, sal_True ) );
}

SwTwips SwSectionFrm::_Grow( SwTwips nDist, sal_Bool bTst )
{
    if ( !IsColLocked() && !HasFixSize() )
    {
        SWRECTFN( this )
        long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
        if( nFrmHeight > 0 && nDist > (LONG_MAX - nFrmHeight) )
            nDist = LONG_MAX - nFrmHeight;

        if ( nDist <= 0L )
            return 0L;

        bool bInCalcCntnt = GetUpper() && IsInFly() && FindFlyFrm()->IsLocked();
        
        bool bGrow = !Lower() || !Lower()->IsColumnFrm() || !Lower()->GetNext() ||
             GetSection()->GetFmt()->GetBalancedColumns().GetValue();
        if( !bGrow )
        {
             const SwViewShell *pSh = getRootFrm()->GetCurrShell();
             bGrow = pSh && pSh->GetViewOptions()->getBrowseMode();
        }
        if( bGrow )
        {
            SwTwips nGrow;
            if( IsInFtn() )
                nGrow = 0;
            else
            {
                nGrow = lcl_DeadLine( this );
                nGrow = (*fnRect->fnYDiff)( nGrow,
                                           (Frm().*fnRect->fnGetBottom)() );
            }
            SwTwips nSpace = nGrow;
            if( !bInCalcCntnt && nGrow < nDist && GetUpper() )
                nGrow += GetUpper()->Grow( LONG_MAX, sal_True );

            if( nGrow > nDist )
                nGrow = nDist;
            if( nGrow <= 0 )
            {
                nGrow = 0;
                if( nDist && !bTst )
                {
                    if( bInCalcCntnt )
                        _InvalidateSize();
                    else
                        InvalidateSize();
                }
            }
            else if( !bTst )
            {
                if( bInCalcCntnt )
                    _InvalidateSize();
                else if( nSpace < nGrow &&  nDist != nSpace + GetUpper()->
                         Grow( nGrow - nSpace, sal_False ) )
                    InvalidateSize();
                else
                {
                    const SvxGraphicPosition ePos =
                        GetAttrSet()->GetBackground().GetGraphicPos();
                    if ( GPOS_RT < ePos && GPOS_TILED != ePos )
                    {
                        SetCompletePaint();
                        InvalidatePage();
                    }
                    if( GetUpper() && GetUpper()->IsHeaderFrm() )
                        GetUpper()->InvalidateSize();
                }
                (Frm().*fnRect->fnAddBottom)( nGrow );
                long nPrtHeight = (Prt().*fnRect->fnGetHeight)() + nGrow;
                (Prt().*fnRect->fnSetHeight)( nPrtHeight );

                if( Lower() && Lower()->IsColumnFrm() && Lower()->GetNext() )
                {
                    SwFrm* pTmp = Lower();
                    do
                    {
                        pTmp->_InvalidateSize();
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                    _InvalidateSize();
                }
                if( GetNext() )
                {
                    SwFrm *pFrm = GetNext();
                    while( pFrm && pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() )
                        pFrm = pFrm->GetNext();
                    if( pFrm )
                    {
                        if( bInCalcCntnt )
                            pFrm->_InvalidatePos();
                        else
                            pFrm->InvalidatePos();
                    }
                }
                
                
                
                
                
                else if ( GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) )
                {
                    InvalidateNextPos();
                }
            }
            return nGrow;
        }
        if ( !bTst )
        {
            if( bInCalcCntnt )
                _InvalidateSize();
            else
                InvalidateSize();
        }
    }
    return 0L;
}

SwTwips SwSectionFrm::_Shrink( SwTwips nDist, sal_Bool bTst )
{
    if ( Lower() && !IsColLocked() && !HasFixSize() )
    {
        if( ToMaximize( sal_False ) )
        {
            if( !bTst )
                InvalidateSize();
        }
        else
        {
            SWRECTFN( this )
            long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
            if ( nDist > nFrmHeight )
                nDist = nFrmHeight;

            if ( Lower()->IsColumnFrm() && Lower()->GetNext() && 
                 !GetSection()->GetFmt()->GetBalancedColumns().GetValue() )
            {   
                
                if ( !bTst )
                    InvalidateSize();
                return nDist;
            }
            else if( !bTst )
            {
                const SvxGraphicPosition ePos =
                    GetAttrSet()->GetBackground().GetGraphicPos();
                if ( GPOS_RT < ePos && GPOS_TILED != ePos )
                {
                    SetCompletePaint();
                    InvalidatePage();
                }
                (Frm().*fnRect->fnAddBottom)( -nDist );
                long nPrtHeight = (Prt().*fnRect->fnGetHeight)() - nDist;
                (Prt().*fnRect->fnSetHeight)( nPrtHeight );

                
                
                
                
                
                
                
                
                
                
                
                
                
                
                if( GetUpper() && !GetUpper()->IsFooterFrm() )
                    GetUpper()->Shrink( nDist, bTst );

                if( Lower() && Lower()->IsColumnFrm() && Lower()->GetNext() )
                {
                    SwFrm* pTmp = Lower();
                    do
                    {
                        pTmp->_InvalidateSize();
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                }
                if( GetNext() )
                {
                    SwFrm* pFrm = GetNext();
                    while( pFrm && pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() )
                        pFrm = pFrm->GetNext();
                    if( pFrm )
                        pFrm->InvalidatePos();
                    else
                        SetRetouche();
                }
                else
                    SetRetouche();
                return nDist;
            }
        }
    }
    return 0L;
}

/*
|*  When are Frms within a SectionFrms moveable?
|*  If they are not in the last column of a SectionFrms yet,
|*  if there is no Follow,
|*  if the SectionFrm cannot grow anymore, then it gets more complicated,
|*  in that case it depends on whether the SectionFrm can find a next
|*  layout sheet. In (column based/chained) Flys this is checked via
|*  GetNextLayout, in tables and headers/footers there is none, however in the
|*  DocBody and in foot notes there is always one.
|*
|*  This routine is used in the TxtFormatter to decided whether it's allowed to
|*  create a (paragraph-)Follow or whether the paragraph has to stick together
|*/
sal_Bool SwSectionFrm::MoveAllowed( const SwFrm* pFrm) const
{
    
    if( HasFollow() || ( pFrm->GetUpper()->IsColBodyFrm() &&
        pFrm->GetUpper()->GetUpper()->GetNext() ) )
        return sal_True;
    if( pFrm->IsInFtn() )
    {
        if( IsInFtn() )
        {
            if( GetUpper()->IsInSct() )
            {
                if( Growable() )
                    return sal_False;
                return GetUpper()->FindSctFrm()->MoveAllowed( this );
            }
            else
                return sal_True;
        }
        
        
        const SwLayoutFrm *pLay = pFrm->FindFtnFrm()->GetUpper()->GetUpper();
        if( pLay->IsColumnFrm() && pLay->GetNext() )
        {
            
            
            
            bool bRet = false;
            if( pLay->GetIndPrev() || pFrm->GetIndPrev() ||
                pFrm->FindFtnFrm()->GetPrev() )
                bRet = true;
            else
            {
                SwLayoutFrm* pBody = ((SwColumnFrm*)pLay)->FindBodyCont();
                if( pBody && pBody->Lower() )
                    bRet = true;
            }
            if( bRet && ( IsFtnAtEnd() || !Growable() ) )
                return sal_True;
        }
    }
    
    if( !IsColLocked() && Growable() )
        return sal_False;
    
    
    if( IsInTab() || ( !IsInDocBody() && FindFooterOrHeader() ) )
        return sal_False; 
    if( IsInFly() ) 
        return 0 != ((SwFrm*)GetUpper())->GetNextLeaf( MAKEPAGE_NONE );
    return sal_True;
}

/** Called for a frame inside a section with no direct previous frame (or only
    previous empty section frames) the previous frame of the outer section is
    returned, if the frame is the first flowing content of this section.

    Note: For a frame inside a table frame, which is inside a section frame,
          NULL is returned.
*/
SwFrm* SwFrm::_GetIndPrev() const
{
    SwFrm *pRet = NULL;
    
    
    
    
    
    OSL_ENSURE( /*!pPrev &&*/ IsInSct(), "Why?" );
    const SwFrm* pSct = GetUpper();
    if( !pSct )
        return NULL;
    if( pSct->IsSctFrm() )
        pRet = pSct->GetIndPrev();
    else if( pSct->IsColBodyFrm() && (pSct = pSct->GetUpper()->GetUpper())->IsSctFrm() )
    {
        
        
        const SwFrm* pCol = GetUpper()->GetUpper()->GetPrev();
        while( pCol )
        {
            OSL_ENSURE( pCol->IsColumnFrm(), "GetIndPrev(): ColumnFrm expected" );
            OSL_ENSURE( pCol->GetLower() && pCol->GetLower()->IsBodyFrm(),
                    "GetIndPrev(): Where's the body?");
            if( ((SwLayoutFrm*)((SwLayoutFrm*)pCol)->Lower())->Lower() )
                return NULL;
            pCol = pCol->GetPrev();
        }
        pRet = pSct->GetIndPrev();
    }

    
    while( pRet && pRet->IsSctFrm() && !((SwSectionFrm*)pRet)->GetSection() )
        pRet = pRet->GetIndPrev();
    return pRet;
}

SwFrm* SwFrm::_GetIndNext()
{
    OSL_ENSURE( !mpNext && IsInSct(), "Why?" );
    SwFrm* pSct = GetUpper();
    if( !pSct )
        return NULL;
    if( pSct->IsSctFrm() )
        return pSct->GetIndNext();
    if( pSct->IsColBodyFrm() && (pSct = pSct->GetUpper()->GetUpper())->IsSctFrm() )
    {   
        
        SwFrm* pCol = GetUpper()->GetUpper()->GetNext();
        while( pCol )
        {
            OSL_ENSURE( pCol->IsColumnFrm(), "GetIndNext(): ColumnFrm expected" );
            OSL_ENSURE( pCol->GetLower() && pCol->GetLower()->IsBodyFrm(),
                    "GetIndNext(): Where's the body?");
            if( ((SwLayoutFrm*)((SwLayoutFrm*)pCol)->Lower())->Lower() )
                return NULL;
            pCol = pCol->GetNext();
        }
        return pSct->GetIndNext();
    }
    return NULL;
}

sal_Bool SwSectionFrm::IsDescendantFrom( const SwSectionFmt* pFmt ) const
{
    if( !pSection || !pFmt )
        return sal_False;
    const SwSectionFmt *pMyFmt = pSection->GetFmt();
    while( pFmt != pMyFmt )
    {
        if( pMyFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pMyFmt = (SwSectionFmt*)pMyFmt->GetRegisteredIn();
        else
            return sal_False;
    }
    return sal_True;
}

void SwSectionFrm::CalcFtnAtEndFlag()
{
    SwSectionFmt *pFmt = GetSection()->GetFmt();
    sal_uInt16 nVal = pFmt->GetFtnAtTxtEnd( sal_False ).GetValue();
    bFtnAtEnd = FTNEND_ATPGORDOCEND != nVal;
    bOwnFtnNum = FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
                 FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
    while( !bFtnAtEnd && !bOwnFtnNum )
    {
        if( pFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pFmt = (SwSectionFmt*)pFmt->GetRegisteredIn();
        else
            break;
        nVal = pFmt->GetFtnAtTxtEnd( sal_False ).GetValue();
        if( FTNEND_ATPGORDOCEND != nVal )
        {
            bFtnAtEnd = true;
            bOwnFtnNum = bOwnFtnNum ||FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
                         FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
        }
    }
}

bool SwSectionFrm::IsEndnoteAtMyEnd() const
{
    return pSection->GetFmt()->GetEndAtTxtEnd( sal_False ).IsAtEnd();
}

void SwSectionFrm::CalcEndAtEndFlag()
{
    SwSectionFmt *pFmt = GetSection()->GetFmt();
    bEndnAtEnd = pFmt->GetEndAtTxtEnd( sal_False ).IsAtEnd();
    while( !bEndnAtEnd )
    {
        if( pFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pFmt = (SwSectionFmt*)pFmt->GetRegisteredIn();
        else
            break;
        bEndnAtEnd = pFmt->GetEndAtTxtEnd( sal_False ).IsAtEnd();
    }
}

void SwSectionFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_uInt8 nInvFlags = 0;

    if( pNew && RES_ATTRSET_CHG == pNew->Which() )
    {
        SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
        SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
        SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
        while( true )
        {
            _UpdateAttr( (SfxPoolItem*)aOIter.GetCurItem(),
                         (SfxPoolItem*)aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrm::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        if ( nInvFlags & 0x01 )
            InvalidateSize();
        if ( nInvFlags & 0x10 )
            SetCompletePaint();
    }
}

void SwSectionFrm::SwClientNotify( const SwModify& rMod, const SfxHint& rHint )
{
    
    const SwSectionFrmMoveAndDeleteHint* pHint =
                    dynamic_cast<const SwSectionFrmMoveAndDeleteHint*>(&rHint);
    if ( pHint && pHint->GetId() == SFX_HINT_DYING && &rMod == GetRegisteredIn() )
    {
        SwSectionFrm::MoveCntntAndDelete( this, pHint->IsSaveCntnt() );
    }
}

void SwSectionFrm::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                            sal_uInt8 &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {   
        case RES_FMT_CHG:
        {
            const SwFmtCol& rNewCol = GetFmt()->GetCol();
            if( !IsInFtn() )
            {
                
                
                
                SwFmtCol aCol;
                if ( Lower() && Lower()->IsColumnFrm() )
                {
                    sal_uInt16 nCol = 0;
                    SwFrm *pTmp = Lower();
                    do
                    {   ++nCol;
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                    aCol.Init( nCol, 0, 1000 );
                }
                bool bChgFtn = IsFtnAtEnd();
                bool const bChgEndn = IsEndnAtEnd();
                bool const bChgMyEndn = IsEndnoteAtMyEnd();
                CalcFtnAtEndFlag();
                CalcEndAtEndFlag();
                bChgFtn = ( bChgFtn != IsFtnAtEnd() ) ||
                          ( bChgEndn != IsEndnAtEnd() ) ||
                          ( bChgMyEndn != IsEndnoteAtMyEnd() );
                ChgColumns( aCol, rNewCol, bChgFtn );
                rInvFlags |= 0x10;
            }
            rInvFlags |= 0x01;
            bClear = false;
        }
            break;

        case RES_COL:
            if( !IsInFtn() )
            {
                ChgColumns( *(const SwFmtCol*)pOld, *(const SwFmtCol*)pNew );
                rInvFlags |= 0x11;
            }
            break;

        case RES_FTN_AT_TXTEND:
            if( !IsInFtn() )
            {
                bool const bOld = IsFtnAtEnd();
                CalcFtnAtEndFlag();
                if (bOld != IsFtnAtEnd())
                {
                    const SwFmtCol& rNewCol = GetFmt()->GetCol();
                    ChgColumns( rNewCol, rNewCol, sal_True );
                    rInvFlags |= 0x01;
                }
            }
            break;

        case RES_END_AT_TXTEND:
            if( !IsInFtn() )
            {
                bool const bOld = IsEndnAtEnd();
                bool const bMyOld = IsEndnoteAtMyEnd();
                CalcEndAtEndFlag();
                if (bOld != IsEndnAtEnd() || bMyOld != IsEndnoteAtMyEnd())
                {
                    const SwFmtCol& rNewCol = GetFmt()->GetCol();
                    ChgColumns( rNewCol, rNewCol, sal_True );
                    rInvFlags |= 0x01;
                }
            }
            break;
        case RES_COLUMNBALANCE:
            rInvFlags |= 0x01;
            break;

        case RES_FRAMEDIR :
            SetDerivedR2L( sal_False );
            CheckDirChange();
            break;

        case RES_PROTECT:
            {
                SwViewShell *pSh = getRootFrm()->GetCurrShell();
                if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
                    pSh->Imp()->InvalidateAccessibleEditableState( sal_True, this );
            }
            break;

        default:
            bClear = false;
    }
    if ( bClear )
    {
        if ( pOldSet || pNewSet )
        {
            if ( pOldSet )
                pOldSet->ClearItem( nWhich );
            if ( pNewSet )
                pNewSet->ClearItem( nWhich );
        }
        else
            SwLayoutFrm::Modify( pOld, pNew );
    }
}


sal_Bool SwSectionFrm::ToMaximize( sal_Bool bCheckFollow ) const
{
    if( HasFollow() )
    {
        if( !bCheckFollow ) 
            return sal_True;
        const SwSectionFrm* pFoll = GetFollow();
        while( pFoll && pFoll->IsSuperfluous() )
            pFoll = pFoll->GetFollow();
        if( pFoll )
            return sal_True;
    }
    if( IsFtnAtEnd() )
        return sal_False;
    const SwFtnContFrm* pCont = ContainsFtnCont();
    if( !IsEndnAtEnd() )
        return 0 != pCont;
    sal_Bool bRet = sal_False;
    while( pCont && !bRet )
    {
        if( pCont->FindFootNote() )
            bRet = sal_True;
        else
            pCont = ContainsFtnCont( pCont );
    }
    return bRet;
}


SwFtnContFrm* SwSectionFrm::ContainsFtnCont( const SwFtnContFrm* pCont ) const
{
    SwFtnContFrm* pRet = NULL;
    const SwLayoutFrm* pLay;
    if( pCont )
    {
        pLay = pCont->FindFtnBossFrm( 0 );
        OSL_ENSURE( IsAnLower( pLay ), "ConatainsFtnCont: Wrong FtnContainer" );
        pLay = (SwLayoutFrm*)pLay->GetNext();
    }
    else if( Lower() && Lower()->IsColumnFrm() )
        pLay = (SwLayoutFrm*)Lower();
    else
        pLay = NULL;
    while ( !pRet && pLay )
    {
        if( pLay->Lower() && pLay->Lower()->GetNext() )
        {
            OSL_ENSURE( pLay->Lower()->GetNext()->IsFtnContFrm(),
                    "ToMaximize: Unexspected Frame" );
            pRet = (SwFtnContFrm*)pLay->Lower()->GetNext();
        }
        OSL_ENSURE( !pLay->GetNext() || pLay->GetNext()->IsLayoutFrm(),
                "ToMaximize: ColFrm exspected" );
        pLay = (SwLayoutFrm*)pLay->GetNext();
    }
    return pRet;
}

void SwSectionFrm::InvalidateFtnPos()
{
    SwFtnContFrm* pCont = ContainsFtnCont( NULL );
    if( pCont )
    {
        SwFrm *pTmp = pCont->ContainsCntnt();
        if( pTmp )
            pTmp->_InvalidatePos();
    }
}

/** Returns the value that the section would like to be
 * greater if it has undersized TxtFrms in it,
 * otherwise Null.
 * If necessary the undersized-flag is corrected.
 */
long SwSectionFrm::Undersize( sal_Bool bOverSize )
{
    m_bUndersized = sal_False;
    SWRECTFN( this )
    long nRet = InnerHeight() - (Prt().*fnRect->fnGetHeight)();
    if( nRet > 0 )
        m_bUndersized = sal_True;
    else if( !bOverSize )
        nRet = 0;
    return nRet;
}






void SwSectionFrm::CalcFtnCntnt()
{
    SwFtnContFrm* pCont = ContainsFtnCont();
    if( pCont )
    {
        SwFrm* pFrm = pCont->ContainsAny();
        if( pFrm )
            pCont->Calc();
        while( pFrm && IsAnLower( pFrm ) )
        {
            SwFtnFrm* pFtn = pFrm->FindFtnFrm();
            if( pFtn )
                pFtn->Calc();
            
            SwFrm* pNextFrm = 0;
            {
                if( pFrm->IsSctFrm() )
                {
                    pNextFrm = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
                }
                if( !pNextFrm )
                {
                    pNextFrm = pFrm->FindNext();
                }
            }
            pFrm->Calc();
            pFrm = pNextFrm;
        }
    }
}

/*
 * If a SectionFrm gets empty, e.g. because its content changes the page/column,
 * it is not destroyed immediately (there could be a pointer left to it on the
 * stack), instead it puts itself in a list at the RootFrm, which is processed
 * later on (in Layaction::Action among others). Its size is set to Null and
 * the pointer to its page as well. Such SectionFrms that are to be deleted
 * must be ignored by the layout/during formatting.
 *
 * With InsertEmptySct the RootFrm stores a SectionFrm in the list,
 * with RemoveFromList it can be removed from the list (Dtor),
 * with DeleteEmptySct the list is processed and the SectionFrms are destroyed.
 */
void SwRootFrm::InsertEmptySct( SwSectionFrm* pDel )
{
    if( !pDestroy )
        pDestroy = new SwDestroyList;
    pDestroy->insert( pDel );
}

void SwRootFrm::_DeleteEmptySct()
{
    OSL_ENSURE( pDestroy, "Keine Liste, keine Kekse" );
    while( !pDestroy->empty() )
    {
        SwSectionFrm* pSect = *pDestroy->begin();
        pDestroy->erase( pDestroy->begin() );
        OSL_ENSURE( !pSect->IsColLocked() && !pSect->IsJoinLocked(),
                "DeleteEmptySct: Locked SectionFrm" );
        if( !pSect->Frm().HasArea() && !pSect->ContainsCntnt() )
        {
            SwLayoutFrm* pUp = pSect->GetUpper();
            pSect->Remove();
            delete pSect;
            if( pUp && !pUp->Lower() )
            {
                if( pUp->IsPageBodyFrm() )
                    pUp->getRootFrm()->SetSuperfluous();
                else if( pUp->IsFtnFrm() && !pUp->IsColLocked() &&
                    pUp->GetUpper() )
                {
                    pUp->Cut();
                    delete pUp;
                }
            }
        }
        else {
            OSL_ENSURE( pSect->GetSection(), "DeleteEmptySct: Halbtoter SectionFrm?!" );
        }
    }
}

void SwRootFrm::_RemoveFromList( SwSectionFrm* pSct )
{
    OSL_ENSURE( pDestroy, "Where's my list?" );
    pDestroy->erase( pSct );
}

#ifdef DBG_UTIL
bool SwRootFrm::IsInDelList( SwSectionFrm* pSct ) const
{
    return pDestroy && pDestroy->find( pSct ) != pDestroy->end();
}
#endif

bool SwSectionFrm::IsBalancedSection() const
{
    bool bRet = false;
    if ( GetSection() && Lower() && Lower()->IsColumnFrm() && Lower()->GetNext() )
    {
        bRet = !GetSection()->GetFmt()->GetBalancedColumns().GetValue();
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
