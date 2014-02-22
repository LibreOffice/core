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

#include "swtable.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "viewimp.hxx"
#include "viewopt.hxx"
#include "frmtool.hxx"
#include "dcontact.hxx"
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <fmtsrnd.hxx>
#include <fmtpdsc.hxx>
#include <editeng/ulspitem.hxx>
#include <tgrditem.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <editeng/pgrditem.hxx>
#include <paratr.hxx>
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "tabfrm.hxx"
#include "pagedesc.hxx"
#include "layact.hxx"
#include "flyfrms.hxx"
#include "sectfrm.hxx"
#include "section.hxx"
#include "dbg_lay.hxx"
#include "lineinfo.hxx"
#include <fmtclbl.hxx>
#include <sortedobjs.hxx>
#include <layouter.hxx>
#include <fmtfollowtextflow.hxx>
#include <switerator.hxx>

sal_Bool SwFlowFrm::m_bMoveBwdJump = sal_False;

SwFlowFrm::SwFlowFrm( SwFrm &rFrm ) :
    m_rThis( rFrm ),
    m_pFollow( 0 ),
    m_pPrecede( 0 ),
    m_bLockJoin( false ),
    m_bUndersized( false ),
    m_bFlyLock( false )
{}

SwFlowFrm::~SwFlowFrm()
{
    if (m_pFollow)
    {
        m_pFollow->m_pPrecede = 0;
    }
    if (m_pPrecede)
    {
        m_pPrecede->m_pFollow = 0;
    }
}

void SwFlowFrm::SetFollow(SwFlowFrm *const pFollow)
{
    if (m_pFollow)
    {
        assert(this == m_pFollow->m_pPrecede);
        m_pFollow->m_pPrecede = 0;
    }
    m_pFollow = pFollow;
    if (m_pFollow != NULL)
    {
        if (m_pFollow->m_pPrecede) 
        {
            assert(m_pFollow == m_pFollow->m_pPrecede->m_pFollow);
            m_pFollow->m_pPrecede->m_pFollow = 0;
        }
        m_pFollow->m_pPrecede = this;
    }
}


sal_Bool SwFlowFrm::HasLockedFollow() const
{
    const SwFlowFrm* pFrm = GetFollow();
    while( pFrm )
    {
        if( pFrm->IsJoinLocked() )
            return sal_True;
        pFrm = pFrm->GetFollow();
    }
    return sal_False;
}

sal_Bool SwFlowFrm::IsKeepFwdMoveAllowed()
{
    
    
    
    SwFrm *pFrm = &m_rThis;
    if ( !pFrm->IsInFtn() )
        do
        {   if ( pFrm->GetAttrSet()->GetKeep().GetValue() )
                pFrm = pFrm->GetIndPrev();
            else
                return sal_True;
        } while ( pFrm );

                  
    sal_Bool bRet = sal_False;
    if ( pFrm && pFrm->GetIndPrev() )
        bRet = sal_True;
    return bRet;
}

void SwFlowFrm::CheckKeep()
{
    
    
    SwFrm *pPre = m_rThis.GetIndPrev();
    if( pPre->IsSctFrm() )
    {
        SwFrm *pLast = ((SwSectionFrm*)pPre)->FindLastCntnt();
        if( pLast && pLast->FindSctFrm() == pPre )
            pPre = pLast;
        else
            return;
    }
    SwFrm* pTmp;
    sal_Bool bKeep;
    while ( sal_True == (bKeep = pPre->GetAttrSet()->GetKeep().GetValue()) &&
            0 != ( pTmp = pPre->GetIndPrev() ) )
    {
        if( pTmp->IsSctFrm() )
        {
            SwFrm *pLast = ((SwSectionFrm*)pTmp)->FindLastCntnt();
            if( pLast && pLast->FindSctFrm() == pTmp )
                pTmp = pLast;
            else
                break;
        }
        pPre = pTmp;
    }
    if ( bKeep )
        pPre->InvalidatePos();
}

sal_Bool SwFlowFrm::IsKeep( const SwAttrSet& rAttrs, bool bCheckIfLastRowShouldKeep ) const
{
    
    
    
    
    
    
    sal_Bool bKeep = bCheckIfLastRowShouldKeep ||
                 (  !m_rThis.IsInFtn() &&
                    ( !m_rThis.IsInTab() || m_rThis.IsTabFrm() ) &&
                    rAttrs.GetKeep().GetValue() );

    OSL_ENSURE( !bCheckIfLastRowShouldKeep || m_rThis.IsTabFrm(),
            "IsKeep with bCheckIfLastRowShouldKeep should only be used for tabfrms" );

    
    if ( bKeep )
    {
        switch ( rAttrs.GetBreak().GetBreak() )
        {
            case SVX_BREAK_COLUMN_AFTER:
            case SVX_BREAK_COLUMN_BOTH:
            case SVX_BREAK_PAGE_AFTER:
            case SVX_BREAK_PAGE_BOTH:
            {
                bKeep = sal_False;
            }
            default: break;
        }
        if ( bKeep )
        {
            SwFrm *pNxt;
            if( 0 != (pNxt = m_rThis.FindNextCnt()) &&
                (!m_pFollow || pNxt != m_pFollow->GetFrm()))
            {
                
                
                
                if ( bCheckIfLastRowShouldKeep )
                {
                    const SwSection* pThisSection = 0;
                    const SwSection* pNextSection = 0;
                    const SwSectionFrm* pThisSectionFrm = m_rThis.FindSctFrm();
                    const SwSectionFrm* pNextSectionFrm = pNxt->FindSctFrm();

                    if ( pThisSectionFrm )
                        pThisSection = pThisSectionFrm->GetSection();

                    if ( pNextSectionFrm )
                        pNextSection = pNextSectionFrm->GetSection();

                    if ( pThisSection != pNextSection )
                        bKeep = sal_False;
                }

                if ( bKeep )
                {
                    const SwAttrSet* pSet = NULL;

                    if ( pNxt->IsInTab() )
                    {
                        SwTabFrm* pTab = pNxt->FindTabFrm();
                        if ( ! m_rThis.IsInTab() || m_rThis.FindTabFrm() != pTab )
                            pSet = &pTab->GetFmt()->GetAttrSet();
                    }

                    if ( ! pSet )
                        pSet = pNxt->GetAttrSet();

                    OSL_ENSURE( pSet, "No AttrSet to check keep attribute" );

                    if ( pSet->GetPageDesc().GetPageDesc() )
                        bKeep = sal_False;
                    else switch ( pSet->GetBreak().GetBreak() )
                    {
                        case SVX_BREAK_COLUMN_BEFORE:
                        case SVX_BREAK_COLUMN_BOTH:
                        case SVX_BREAK_PAGE_BEFORE:
                        case SVX_BREAK_PAGE_BOTH:
                            bKeep = sal_False;
                        default: break;
                    }
                }
            }
        }
    }
    return bKeep;
}

sal_uInt8 SwFlowFrm::BwdMoveNecessary( const SwPageFrm *pPage, const SwRect &rRect )
{
    
    
    
    //
    
    

    
    
    
    
    
    
    

    
    
    
    sal_uInt8 nRet = 0;
    SwFlowFrm *pTmp = this;
    do
    {   
        
        
        
        if( pTmp->GetFrm()->GetDrawObjs() )
            nRet = 1;
        pTmp = pTmp->GetFollow();
    } while ( !nRet && pTmp );
    if ( pPage->GetSortedObjs() )
    {
        
        const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
        sal_uLong nIndex = ULONG_MAX;
        for ( sal_uInt16 i = 0; nRet < 3 && i < rObjs.Count(); ++i )
        {
            
            
            SwAnchoredObject* pObj = rObjs[i];
            const SwFrmFmt& rFmt = pObj->GetFrmFmt();
            const SwRect aRect( pObj->GetObjRect() );
            if ( aRect.IsOver( rRect ) &&
                 rFmt.GetSurround().GetSurround() != SURROUND_THROUGHT )
            {
                if( m_rThis.IsLayoutFrm() && 
                    Is_Lower_Of( &m_rThis, pObj->GetDrawObj() ) )
                    continue;
                if( pObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pObj);
                    if ( pFly->IsAnLower( &m_rThis ) )
                        continue;
                }

                const SwFrm* pAnchor = pObj->GetAnchorFrm();
                if ( pAnchor == &m_rThis )
                {
                    nRet |= 1;
                    continue;
                }

                
                
                if ( ::IsFrmInSameKontext( pAnchor, &m_rThis ) )
                {
                    if ( rFmt.GetAnchor().GetAnchorId() == FLY_AT_PARA )
                    {
                        
                        sal_uLong nTmpIndex = rFmt.GetAnchor().GetCntntAnchor()->nNode.GetIndex();
                        
                        
                        
                        
                        
                        if( ULONG_MAX == nIndex )
                        {
                            const SwNode *pNode;
                            if ( m_rThis.IsCntntFrm() )
                                pNode = ((SwCntntFrm&)m_rThis).GetNode();
                            else if( m_rThis.IsSctFrm() )
                                pNode = ((SwSectionFmt*)((SwSectionFrm&)m_rThis).
                                        GetFmt())->GetSectionNode();
                            else
                            {
                                OSL_ENSURE( m_rThis.IsTabFrm(), "new FowFrm?" );
                                pNode = ((SwTabFrm&)m_rThis).GetTable()->
                                    GetTabSortBoxes()[0]->GetSttNd()->FindTableNode();
                            }
                            nIndex = pNode->GetIndex();
                        }
                        if( nIndex < nTmpIndex )
                            continue;
                    }
                }
                else
                    continue;

                nRet |= 2;
            }
        }
    }
    return nRet;
}



SwLayoutFrm *SwFlowFrm::CutTree( SwFrm *pStart )
{
    
    
    

    SwLayoutFrm *pLay = pStart->GetUpper();
    if ( pLay->IsInFtn() )
        pLay = pLay->FindFtnFrm();

    
    
    if( pStart->IsInFtn() )
    {
        SwFrm* pTmp = pStart->GetIndPrev();
        if( pTmp )
            pTmp->Prepare( PREP_QUOVADIS );
    }

    
    
    if ( pStart == pStart->GetUpper()->Lower() )
        pStart->GetUpper()->pLower = 0;
    if ( pStart->GetPrev() )
    {
        pStart->GetPrev()->mpNext = 0;
        pStart->mpPrev = 0;
    }

    if ( pLay->IsFtnFrm() )
    {
        if ( !pLay->Lower() && !pLay->IsColLocked() &&
             !((SwFtnFrm*)pLay)->IsBackMoveLocked() )
        {
            pLay->Cut();
            delete pLay;
        }
        else
        {
            bool bUnlock = !((SwFtnFrm*)pLay)->IsBackMoveLocked();
            ((SwFtnFrm*)pLay)->LockBackMove();
            pLay->InvalidateSize();
            pLay->Calc();
            SwCntntFrm *pCnt = pLay->ContainsCntnt();
            while ( pCnt && pLay->IsAnLower( pCnt ) )
            {
                
                
                
                OSL_ENSURE( pCnt->IsTxtFrm(), "The Graphic has landed." );
                if ( ((SwTxtFrm*)pCnt)->IsLocked() ||
                     ((SwTxtFrm*)pCnt)->GetFollow() == pStart )
                    break;
                pCnt->Calc();
                pCnt = pCnt->GetNextCntntFrm();
            }
            if( bUnlock )
                ((SwFtnFrm*)pLay)->UnlockBackMove();
        }
        pLay = 0;
    }
    return pLay;
}



sal_Bool SwFlowFrm::PasteTree( SwFrm *pStart, SwLayoutFrm *pParent, SwFrm *pSibling,
                           SwFrm *pOldParent )
{
    
    sal_Bool bRet = sal_False;

    
    

    
    
    
    if ( pSibling )
    {
        if ( 0 != (pStart->mpPrev = pSibling->GetPrev()) )
            pStart->GetPrev()->mpNext = pStart;
        else
            pParent->pLower = pStart;
        pSibling->_InvalidatePos();
        pSibling->_InvalidatePrt();
    }
    else
    {
        if ( 0 == (pStart->mpPrev = pParent->Lower()) )
            pParent->pLower = pStart;
        else
        
        
        
        
        
        
        
        {
            SwFrm* pTemp = pParent->pLower;
            while (pTemp)
            {
                if (pTemp->mpNext)
                    pTemp = pTemp->mpNext;
                else
                {
                    pStart->mpPrev = pTemp;
                    pTemp->mpNext = pStart;
                    break;
                }
            }
        }
        

        
        if ( pParent->IsSctFrm() )
        {
            
            
            
            
            
            pParent->InvalidateNextPrtArea();
        }
    }
    SwFrm *pFloat = pStart;
    SwFrm *pLst = 0;
    SWRECTFN( pParent )
    SwTwips nGrowVal = 0;
    do
    {   pFloat->mpUpper = pParent;
        pFloat->_InvalidateAll();
        pFloat->CheckDirChange();

        
        
        if ( pFloat->IsTxtFrm() )
        {
            if ( ((SwTxtFrm*)pFloat)->GetCacheIdx() != USHRT_MAX )
                ((SwTxtFrm*)pFloat)->Init();    
        }
        else
            bRet = sal_True;

        nGrowVal += (pFloat->Frm().*fnRect->fnGetHeight)();
        if ( pFloat->GetNext() )
            pFloat = pFloat->GetNext();
        else
        {
            pLst = pFloat;
            pFloat = 0;
        }
    } while ( pFloat );

    if ( pSibling )
    {
        pLst->mpNext = pSibling;
        pSibling->mpPrev = pLst;
        if( pSibling->IsInFtn() )
        {
            if( pSibling->IsSctFrm() )
                pSibling = ((SwSectionFrm*)pSibling)->ContainsAny();
            if( pSibling )
                pSibling->Prepare( PREP_ERGOSUM );
        }
    }
    if ( nGrowVal )
    {
        if ( pOldParent && pOldParent->IsBodyFrm() ) 
            pOldParent->Shrink( nGrowVal );
        pParent->Grow( nGrowVal );
    }

    if ( pParent->IsFtnFrm() )
        ((SwFtnFrm*)pParent)->InvalidateNxtFtnCnts( pParent->FindPageFrm() );
    return bRet;
}

void SwFlowFrm::MoveSubTree( SwLayoutFrm* pParent, SwFrm* pSibling )
{
    OSL_ENSURE( pParent, "No parent given." );
    OSL_ENSURE( m_rThis.GetUpper(), "Where are we coming from?" );

    
    SwViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();
    const SwViewImp *pImp = pSh ? pSh->Imp() : 0;
    const bool bComplete = pImp && pImp->IsAction() && pImp->GetLayAction().IsComplete();

    if ( !bComplete )
    {
        SwFrm *pPre = m_rThis.GetIndPrev();
        if ( pPre )
        {
            pPre->SetRetouche();
            
            
            if ( pPre->GetUpper()->IsInTab() )
            {
                pPre->_InvalidatePrt();
            }
            pPre->InvalidatePage();
        }
        else
        {
            m_rThis.GetUpper()->SetCompletePaint();
            m_rThis.GetUpper()->InvalidatePage();
        }
    }

    SwPageFrm *pOldPage = m_rThis.FindPageFrm();

    SwLayoutFrm *pOldParent = CutTree( &m_rThis );
    const sal_Bool bInvaLay = PasteTree( &m_rThis, pParent, pSibling, pOldParent );

    
    
    SwSectionFrm *pSct;
    
    
    if ( pOldParent && !pOldParent->Lower() &&
         ( pOldParent->IsInSct() &&
           !(pSct = pOldParent->FindSctFrm())->ContainsCntnt() &&
           !pSct->ContainsAny( true ) ) )
    {
            pSct->DelEmpty( sal_False );
    }

    
    if( !m_rThis.IsInSct() &&
        ( !m_rThis.IsInTab() || ( m_rThis.IsTabFrm() && !m_rThis.GetUpper()->IsInTab() ) ) )
        m_rThis.GetUpper()->Calc();
    else if( m_rThis.GetUpper()->IsSctFrm() )
    {
        SwSectionFrm* pTmpSct = (SwSectionFrm*)m_rThis.GetUpper();
        sal_Bool bOld = pTmpSct->IsCntntLocked();
        pTmpSct->SetCntntLock( true );
        pTmpSct->Calc();
        if( !bOld )
            pTmpSct->SetCntntLock( false );
    }
    SwPageFrm *pPage = m_rThis.FindPageFrm();

    if ( pOldPage != pPage )
    {
        m_rThis.InvalidatePage( pPage );
        if ( m_rThis.IsLayoutFrm() )
        {
            SwCntntFrm *pCnt = ((SwLayoutFrm*)&m_rThis)->ContainsCntnt();
            if ( pCnt )
                pCnt->InvalidatePage( pPage );
        }
        else if ( pSh && pSh->GetDoc()->GetLineNumberInfo().IsRestartEachPage()
                  && pPage->FindFirstBodyCntnt() == &m_rThis )
        {
            m_rThis._InvalidateLineNum();
        }
    }
    if ( bInvaLay || (pSibling && pSibling->IsLayoutFrm()) )
        m_rThis.GetUpper()->InvalidatePage( pPage );
}

sal_Bool SwFlowFrm::IsAnFollow( const SwFlowFrm *pAssumed ) const
{
    const SwFlowFrm *pFoll = this;
    do
    {   if ( pAssumed == pFoll )
            return sal_True;
        pFoll = pFoll->GetFollow();
    } while ( pFoll );
    return sal_False;
}

SwTxtFrm* SwCntntFrm::FindMaster() const
{
    OSL_ENSURE( IsFollow(), "SwCntntFrm::FindMaster(): !IsFollow" );

    const SwCntntFrm* pPrec = GetPrecede();

    if ( pPrec && pPrec->HasFollow() && pPrec->GetFollow() == this )
    {
        OSL_ENSURE( pPrec->IsTxtFrm(), "NoTxtFrm with follow found" );
        return ( SwTxtFrm* )pPrec;
    }

    OSL_FAIL( "Follow ist lost in Space." );
    return 0;
}

SwSectionFrm* SwSectionFrm::FindMaster() const
{
    OSL_ENSURE( IsFollow(), "SwSectionFrm::FindMaster(): !IsFollow" );

    SwIterator<SwSectionFrm,SwFmt> aIter( *pSection->GetFmt() );
    SwSectionFrm* pSect = aIter.First();
    while ( pSect )
    {
            if( pSect->GetFollow() == this )
                return pSect;
        pSect = aIter.Next();
    }

    OSL_FAIL( "Follow ist lost in Space." );
    return 0;
}

SwTabFrm* SwTabFrm::FindMaster( bool bFirstMaster ) const
{
    OSL_ENSURE( IsFollow(), "SwTabFrm::FindMaster(): !IsFollow" );

    SwIterator<SwTabFrm,SwFmt> aIter( *GetTable()->GetFrmFmt() );
    SwTabFrm* pTab = aIter.First();
    while ( pTab )
    {
            if ( bFirstMaster )
            {
                
                
                

                if ( !pTab->IsFollow() )
                {
                    SwTabFrm* pNxt = pTab;
                    while ( pNxt )
                    {
                        if ( pNxt->GetFollow() == this )
                            return pTab;
                        pNxt = pNxt->GetFollow();
                    }
                }
            }
            else
            {
                if ( pTab->GetFollow() == this )
                    return pTab;
            }

        pTab = aIter.Next();
    }

    OSL_FAIL( "Follow ist lost in Space." );
    return 0;
}

/**
 * Returns the next/previous Layout leaf that's NOT below this (or even is this itself).
 * Also, that leaf must be in the same text flow as the pAnch origin frame (Body, Ftn)
 */
const SwLayoutFrm *SwFrm::GetLeaf( MakePageType eMakePage, sal_Bool bFwd,
                                   const SwFrm *pAnch ) const
{
    
    if ( !(IsInDocBody() || IsInFtn() || IsInFly()) )
        return 0;

    const SwFrm *pLeaf = this;
    bool bFound = false;

    do
    {   pLeaf = ((SwFrm*)pLeaf)->GetLeaf( eMakePage, bFwd );

        if ( pLeaf &&
            (!IsLayoutFrm() || !((SwLayoutFrm*)this)->IsAnLower( pLeaf )))
        {
            if ( pAnch->IsInDocBody() == pLeaf->IsInDocBody() &&
                 pAnch->IsInFtn()     == pLeaf->IsInFtn() )
            {
                bFound = true;
            }
        }
    } while ( !bFound && pLeaf );

    return (const SwLayoutFrm*)pLeaf;
}

SwLayoutFrm *SwFrm::GetLeaf( MakePageType eMakePage, sal_Bool bFwd )
{
    if ( IsInFtn() )
        return bFwd ? GetNextFtnLeaf( eMakePage ) : GetPrevFtnLeaf( eMakePage );

    
    
    
    bool bInTab( IsInTab() );
    bool bInSct( IsInSct() );
    if ( bInTab && bInSct )
    {
        const SwFrm* pUpperFrm( GetUpper() );
        while ( pUpperFrm )
        {
            if ( pUpperFrm->IsTabFrm() )
            {
                
                bInSct = false;
                break;
            }
            else if ( pUpperFrm->IsSctFrm() )
            {
                
                bInTab = false;
                break;
            }

            pUpperFrm = pUpperFrm->GetUpper();
        }
    }

    if ( bInTab && ( !IsTabFrm() || GetUpper()->IsCellFrm() ) ) 
        return bFwd ? GetNextCellLeaf( eMakePage ) : GetPrevCellLeaf( eMakePage );

    if ( bInSct )
        return bFwd ? GetNextSctLeaf( eMakePage ) : GetPrevSctLeaf( eMakePage );

    return bFwd ? GetNextLeaf( eMakePage ) : GetPrevLeaf( eMakePage );
}

sal_Bool SwFrm::WrongPageDesc( SwPageFrm* pNew )
{
    
    //
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    const SwFmtPageDesc &rFmtDesc = GetAttrSet()->GetPageDesc();

    
    SwPageDesc *pDesc = 0;
    sal_uInt16 nTmp = 0;
    SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( this );
    if ( !pFlow || !pFlow->IsFollow() )
    {
        pDesc = (SwPageDesc*)rFmtDesc.GetPageDesc();
        if( pDesc )
        {
            if( !pDesc->GetRightFmt() )
                nTmp = 2;
            else if( !pDesc->GetLeftFmt() )
                nTmp = 1;
            else if( rFmtDesc.GetNumOffset() )
                nTmp = rFmtDesc.GetNumOffset().get();
        }
    }

    
    
    
    const sal_Bool bOdd = nTmp ? ( (nTmp % 2) ? sal_True : sal_False )
                           : pNew->OnRightPage();
    if ( !pDesc )
        pDesc = pNew->FindPageDesc();

    bool bFirst = pNew->OnFirstPage();

    const SwFlowFrm *pNewFlow = pNew->FindFirstBodyCntnt();
    
    if( pNewFlow == pFlow )
        pNewFlow = NULL;
    if ( pNewFlow && pNewFlow->GetFrm()->IsInTab() )
        pNewFlow = pNewFlow->GetFrm()->FindTabFrm();
    const SwPageDesc *pNewDesc= ( pNewFlow && !pNewFlow->IsFollow() )
            ? pNewFlow->GetFrm()->GetAttrSet()->GetPageDesc().GetPageDesc() : 0;

    return (pNew->GetPageDesc() != pDesc)   
        || (pNew->GetFmt() !=
              (bOdd ? pDesc->GetRightFmt(bFirst) : pDesc->GetLeftFmt(bFirst)))
        || (pNewDesc && pNewDesc == pDesc);
}



SwLayoutFrm *SwFrm::GetNextLeaf( MakePageType eMakePage )
{
    OSL_ENSURE( !IsInFtn(), "GetNextLeaf(), don't call me for Ftn." );
    OSL_ENSURE( !IsInSct(), "GetNextLeaf(), don't call me for Sections." );

    const bool bBody = IsInDocBody();  
                                           

    
    
    if( IsInFly() )
        eMakePage = MAKEPAGE_NONE;

    
    
    SwLayoutFrm *pLayLeaf = 0;
    if ( IsTabFrm() )
    {
        SwCntntFrm* pTmp = ((SwTabFrm*)this)->FindLastCntnt();
        if ( pTmp )
            pLayLeaf = pTmp->GetUpper();
    }
    if ( !pLayLeaf )
        pLayLeaf = GetNextLayoutLeaf();

    SwLayoutFrm *pOldLayLeaf = 0;           
                                            
                                            
    bool bNewPg = false;    

    while ( true )
    {
        if ( pLayLeaf )
        {
            
            
            
            
            if ( pLayLeaf->FindPageFrm()->IsFtnPage() )
            {   
                pLayLeaf = 0;
                continue;
            }
            if ( (bBody && !pLayLeaf->IsInDocBody()) || pLayLeaf->IsInTab()
                 || pLayLeaf->IsInSct() )
            {
                
                pOldLayLeaf = pLayLeaf;
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
                continue;
            }

            
            
            

            if( !IsFlowFrm() && ( eMakePage == MAKEPAGE_NONE ||
                eMakePage==MAKEPAGE_APPEND || eMakePage==MAKEPAGE_NOSECTION ) )
                return pLayLeaf;

            SwPageFrm *pNew = pLayLeaf->FindPageFrm();
            const SwViewShell *pSh = getRootFrm()->GetCurrShell();
            
            if ( pNew != FindPageFrm() && !bNewPg && !IsInFly() &&
                 
                 
                 
                 !(pSh && pSh->GetViewOptions()->getBrowseMode() ) )
            {
                if( WrongPageDesc( pNew ) )
                {
                    SwFtnContFrm *pCont = pNew->FindFtnCont();
                    if( pCont )
                    {
                        
                        
                        
                        SwFtnFrm *pFtn = (SwFtnFrm*)pCont->Lower();
                        if( pFtn && pFtn->GetRef() )
                        {
                            const sal_uInt16 nRefNum = pNew->GetPhyPageNum();
                            if( pFtn->GetRef()->GetPhyPageNum() < nRefNum )
                                break;
                        }
                    }
                    
                    
                    if ( eMakePage == MAKEPAGE_INSERT )
                    {
                        bNewPg = true;

                        SwPageFrm *pPg = pOldLayLeaf ?
                                    pOldLayLeaf->FindPageFrm() : 0;
                        if ( pPg && pPg->IsEmptyPage() )
                            
                            pPg = (SwPageFrm*)pPg->GetPrev();

                        if ( !pPg || pPg == pNew )
                            pPg = FindPageFrm();

                        InsertPage( pPg, sal_False );
                        pLayLeaf = GetNextLayoutLeaf();
                        pOldLayLeaf = 0;
                        continue;
                    }
                    else
                        pLayLeaf = 0;
                }
            }
            break;
        }
        else
        {
            
            
            if ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT )
            {
                InsertPage(
                    pOldLayLeaf ? pOldLayLeaf->FindPageFrm() : FindPageFrm(),
                    sal_False );

                
                pLayLeaf = pOldLayLeaf ? pOldLayLeaf : GetNextLayoutLeaf();
            }
            else
                break;
        }
    }
    return pLayLeaf;
}


SwLayoutFrm *SwFrm::GetPrevLeaf( MakePageType )
{
    OSL_ENSURE( !IsInFtn(), "GetPrevLeaf(), don't call me for Ftn." );

    const bool bBody = IsInDocBody();  
                                           
    const sal_Bool bFly  = IsInFly();

    SwLayoutFrm *pLayLeaf = GetPrevLayoutLeaf();
    SwLayoutFrm *pPrevLeaf = 0;

    while ( pLayLeaf )
    {
        if ( pLayLeaf->IsInTab() ||     
             pLayLeaf->IsInSct() )      
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
        else if ( bBody && pLayLeaf->IsInDocBody() )
        {
            if ( pLayLeaf->Lower() )
                break;
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
    return pLayLeaf ? pLayLeaf : pPrevLeaf;
}

sal_Bool SwFlowFrm::IsPrevObjMove() const
{
    
    

    
    const SwViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
        return sal_False;

    SwFrm *pPre = m_rThis.FindPrev();

    if ( pPre && pPre->GetDrawObjs() )
    {
        OSL_ENSURE( SwFlowFrm::CastFlowFrm( pPre ), "new flowfrm?" );
        if( SwFlowFrm::CastFlowFrm( pPre )->IsAnFollow( this ) )
            return sal_False;
        SwLayoutFrm* pPreUp = pPre->GetUpper();
        
        
        
        if( pPreUp->IsInSct() )
        {
            if( pPreUp->IsSctFrm() )
                pPreUp = pPreUp->GetUpper();
            else if( pPreUp->IsColBodyFrm() &&
                     pPreUp->GetUpper()->GetUpper()->IsSctFrm() )
                pPreUp = pPreUp->GetUpper()->GetUpper()->GetUpper();
        }
        
        
        
        for ( sal_uInt16 i = 0; i < pPre->GetDrawObjs()->Count(); ++i )
        {
            
            
            const SwAnchoredObject* pObj = (*pPre->GetDrawObjs())[i];
            
            
            
            if ( pObj->GetFrmFmt().GetDoc()->IsVisibleLayerId(
                                            pObj->GetDrawObj()->GetLayer() ) &&
                 pObj->GetFrmFmt().GetFollowTextFlow().GetValue() )
            {
                const SwLayoutFrm* pVertPosOrientFrm = pObj->GetVertPosOrientFrm();
                if ( pVertPosOrientFrm &&
                     pPreUp != pVertPosOrientFrm &&
                     !pPreUp->IsAnLower( pVertPosOrientFrm ) )
                {
                    return sal_True;
                }
            }
        }
    }
    return sal_False;
}

/**
|*      If there's a hard page break before the Frm AND there's a
|*      predecessor on the same page, sal_True is returned (we need to create a
|*      new PageBreak). Otherwise, returns sal_False.
|*      If bAct is set to sal_True, this function returns sal_True if
|*      there's a PageBreak.
|*      Of course, we don't evaluate the hard page break for follows.
|*      The page break is in it's own FrmFmt (BEFORE) or in the FrmFmt of the
|*      predecessor (AFTER). If there's no predecessor on the page, we don't
|*      need to think further.
|*      Also, a page break (or the need for one) is also present if
|*      the FrmFmt contains a PageDesc.
|*      The implementation works only on CntntFrms! - the definition
|*      of the predecessor is not clear for LayoutFrms.
|*/
sal_Bool SwFlowFrm::IsPageBreak( sal_Bool bAct ) const
{
    if ( !IsFollow() && m_rThis.IsInDocBody() &&
         ( !m_rThis.IsInTab() || ( m_rThis.IsTabFrm() && !m_rThis.GetUpper()->IsInTab() ) ) ) 
    {
        const SwViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();
        if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            return sal_False;
        const SwAttrSet *pSet = m_rThis.GetAttrSet();

        
        const SwFrm *pPrev = m_rThis.FindPrev();
        while ( pPrev && ( !pPrev->IsInDocBody() ||
                ( pPrev->IsTxtFrm() && ((SwTxtFrm*)pPrev)->IsHiddenNow() ) ) )
            pPrev = pPrev->FindPrev();

        if ( pPrev )
        {
            OSL_ENSURE( pPrev->IsInDocBody(), "IsPageBreak: Not in DocBody?" );
            if ( bAct )
            {   if ( m_rThis.FindPageFrm() == pPrev->FindPageFrm() )
                    return sal_False;
            }
            else
            {   if ( m_rThis.FindPageFrm() != pPrev->FindPageFrm() )
                    return sal_False;
            }

            const SvxBreak eBreak = pSet->GetBreak().GetBreak();
            if ( eBreak == SVX_BREAK_PAGE_BEFORE || eBreak == SVX_BREAK_PAGE_BOTH )
                return sal_True;
            else
            {
                const SvxBreak &ePrB = pPrev->GetAttrSet()->GetBreak().GetBreak();
                if ( ePrB == SVX_BREAK_PAGE_AFTER ||
                     ePrB == SVX_BREAK_PAGE_BOTH  ||
                     pSet->GetPageDesc().GetPageDesc() )
                    return sal_True;
            }
        }
    }
    return sal_False;
}

/**
|*      If there's a hard column break before the Frm AND there is
|*      a predecessor in the same column, we return sal_True (we need to create
|*      a ColBreak). Otherwise, we return sal_False.
|*      If bAct is set to sal_True, we return sal_True if there's a ColBreak.
|*      Of course, we don't evaluate the hard column break for follows.
|*
|*      The column break is in it's own FrmFmt (BEFORE) or in the FrmFmt of the
|*      predecessor (AFTER). If there's no predecessor in the column, we don't
|*      need to think further.
|*      The implementation works only on CntntFrms! - the definition
|*      of the predecessor is not clear for LayoutFrms.
|*/
sal_Bool SwFlowFrm::IsColBreak( sal_Bool bAct ) const
{
    if ( !IsFollow() && (m_rThis.IsMoveable() || bAct) )
    {
        const SwFrm *pCol = m_rThis.FindColFrm();
        if ( pCol )
        {
            
            const SwFrm *pPrev = m_rThis.FindPrev();
            while( pPrev && ( ( !pPrev->IsInDocBody() && !m_rThis.IsInFly() ) ||
                   ( pPrev->IsTxtFrm() && ((SwTxtFrm*)pPrev)->IsHiddenNow() ) ) )
                    pPrev = pPrev->FindPrev();

            if ( pPrev )
            {
                if ( bAct )
                {   if ( pCol == pPrev->FindColFrm() )
                        return sal_False;
                }
                else
                {   if ( pCol != pPrev->FindColFrm() )
                        return sal_False;
                }

                const SvxBreak eBreak = m_rThis.GetAttrSet()->GetBreak().GetBreak();
                if ( eBreak == SVX_BREAK_COLUMN_BEFORE ||
                     eBreak == SVX_BREAK_COLUMN_BOTH )
                    return sal_True;
                else
                {
                    const SvxBreak &ePrB = pPrev->GetAttrSet()->GetBreak().GetBreak();
                    if ( ePrB == SVX_BREAK_COLUMN_AFTER ||
                         ePrB == SVX_BREAK_COLUMN_BOTH )
                        return sal_True;
                }
            }
        }
    }
    return sal_False;
}

sal_Bool SwFlowFrm::HasParaSpaceAtPages( sal_Bool bSct ) const
{
    if( m_rThis.IsInSct() )
    {
        const SwFrm* pTmp = m_rThis.GetUpper();
        while( pTmp )
        {
            if( pTmp->IsCellFrm() || pTmp->IsFlyFrm() ||
                pTmp->IsFooterFrm() || pTmp->IsHeaderFrm() ||
                ( pTmp->IsFtnFrm() && !((SwFtnFrm*)pTmp)->GetMaster() ) )
                return sal_True;
            if( pTmp->IsPageFrm() )
                return ( pTmp->GetPrev() && !IsPageBreak(sal_True) ) ? sal_False : sal_True;
            if( pTmp->IsColumnFrm() && pTmp->GetPrev() )
                return IsColBreak( sal_True );
            if( pTmp->IsSctFrm() && ( !bSct || pTmp->GetPrev() ) )
                return sal_False;
            pTmp = pTmp->GetUpper();
        }
        OSL_FAIL( "HasParaSpaceAtPages: Where's my page?" );
        return sal_False;
    }
    if( !m_rThis.IsInDocBody() || ( m_rThis.IsInTab() && !m_rThis.IsTabFrm()) ||
        IsPageBreak( sal_True ) || ( m_rThis.FindColFrm() && IsColBreak( sal_True ) ) )
        return sal_True;
    const SwFrm* pTmp = m_rThis.FindColFrm();
    if( pTmp )
    {
        if( pTmp->GetPrev() )
            return sal_False;
    }
    else
        pTmp = &m_rThis;
    pTmp = pTmp->FindPageFrm();
    return pTmp && !pTmp->GetPrev();
}

/** helper method to determine previous frame for calculation of the
    upper space

    OD 2004-03-10 #i11860#
*/
const SwFrm* SwFlowFrm::_GetPrevFrmForUpperSpaceCalc( const SwFrm* _pProposedPrevFrm ) const
{
    const SwFrm* pPrevFrm = _pProposedPrevFrm
                            ? _pProposedPrevFrm
                            : m_rThis.GetPrev();

    
    while ( pPrevFrm &&
            ( ( pPrevFrm->IsTxtFrm() &&
                static_cast<const SwTxtFrm*>(pPrevFrm)->IsHiddenNow() ) ||
              ( pPrevFrm->IsSctFrm() &&
                !static_cast<const SwSectionFrm*>(pPrevFrm)->GetSection() ) ) )
    {
        pPrevFrm = pPrevFrm->GetPrev();
    }

    
    
    
    if ( !pPrevFrm && m_rThis.IsInFtn() &&
         ( m_rThis.IsSctFrm() ||
           !m_rThis.IsInSct() || !m_rThis.FindSctFrm()->IsInFtn() ) )
    {
        const SwFtnFrm* pPrevFtnFrm =
                static_cast<const SwFtnFrm*>(m_rThis.FindFtnFrm()->GetPrev());
        if ( pPrevFtnFrm )
        {
            pPrevFrm = pPrevFtnFrm->GetLastLower();

            
            while ( pPrevFrm &&
                    ( ( pPrevFrm->IsTxtFrm() &&
                        static_cast<const SwTxtFrm*>(pPrevFrm)->IsHiddenNow() ) ||
                      ( pPrevFrm->IsSctFrm() &&
                        !static_cast<const SwSectionFrm*>(pPrevFrm)->GetSection() ) ) )
            {
                pPrevFrm = pPrevFrm->GetPrev();
            }
        }
    }
    
    
    if( pPrevFrm && pPrevFrm->IsSctFrm() )
    {
        const SwSectionFrm* pPrevSectFrm =
                                    static_cast<const SwSectionFrm*>(pPrevFrm);
        pPrevFrm = pPrevSectFrm->FindLastCntnt();
        
        
        
        
        
        if ( pPrevFrm && pPrevFrm->IsInTab() )
        {
            const SwTabFrm* pTableFrm = pPrevFrm->FindTabFrm();
            if ( pPrevSectFrm->IsAnLower( pTableFrm ) )
            {
                pPrevFrm = pTableFrm;
            }
        }
        
        while ( pPrevFrm &&
                pPrevFrm->IsTxtFrm() &&
                static_cast<const SwTxtFrm*>(pPrevFrm)->IsHiddenNow() )
        {
            pPrevFrm = pPrevFrm->GetPrev();
        }
    }

    return pPrevFrm;
}


static bool lcl_IdenticalStyles(const SwFrm* pPrevFrm, const SwFrm* pFrm)
{
    SwTxtFmtColl *pPrevFmtColl = 0;
    if (pPrevFrm && pPrevFrm->IsTxtFrm())
    {
        SwTxtFrm *pTxtFrm = ( SwTxtFrm * ) pPrevFrm;
        pPrevFmtColl = dynamic_cast<SwTxtFmtColl*>(pTxtFrm->GetTxtNode()->GetFmtColl());
    }

    bool bIdenticalStyles = false;
    if (pFrm && pFrm->IsTxtFrm())
    {
        SwTxtFrm *pTxtFrm = ( SwTxtFrm * ) pFrm;
        SwTxtFmtColl *pFmtColl = dynamic_cast<SwTxtFmtColl*>(pTxtFrm->GetTxtNode()->GetFmtColl());
        bIdenticalStyles = pPrevFmtColl == pFmtColl;
    }
    return bIdenticalStyles;
}

static bool lcl_getContextualSpacing(const SwFrm* pPrevFrm)
{
    bool bRet;
    SwBorderAttrAccess *pAccess = new SwBorderAttrAccess( SwFrm::GetCache(), pPrevFrm );
    const SwBorderAttrs *pAttrs = pAccess->Get();

    bRet = pAttrs->GetULSpace().GetContext();

    delete pAccess;
    return bRet;
}


SwTwips SwFlowFrm::CalcUpperSpace( const SwBorderAttrs *pAttrs,
                                   const SwFrm* pPr,
                                   const bool _bConsiderGrid ) const
{
    
    const SwFrm* pPrevFrm = _GetPrevFrmForUpperSpaceCalc( pPr );

    SwBorderAttrAccess *pAccess;
    SwFrm* pOwn;
    if( !pAttrs )
    {
        if( m_rThis.IsSctFrm() )
        {
            SwSectionFrm* pFoll = &((SwSectionFrm&)m_rThis);
            do
                pOwn = pFoll->ContainsAny();
            while( !pOwn && 0 != ( pFoll = pFoll->GetFollow() ) );
            if( !pOwn )
                return 0;
        }
        else
            pOwn = &m_rThis;
        pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), pOwn );
        pAttrs = pAccess->Get();
    }
    else
    {
        pAccess = NULL;
        pOwn = &m_rThis;
    }
    SwTwips nUpper = 0;
    
    {
        const IDocumentSettingAccess* pIDSA = m_rThis.GetUpper()->GetFmt()->getIDocumentSettingAccess();
        const bool bUseFormerLineSpacing = pIDSA->get(IDocumentSettingAccess::OLD_LINE_SPACING);
        if( pPrevFrm )
        {
            
            
            SwTwips nPrevLowerSpace = 0;
            SwTwips nPrevLineSpacing = 0;
            
            bool bPrevLineSpacingPorportional = false;
            GetSpacingValuesOfFrm( (*pPrevFrm),
                                   nPrevLowerSpace, nPrevLineSpacing,
                                   bPrevLineSpacingPorportional );
            if( pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX) )
            {
                nUpper = nPrevLowerSpace + pAttrs->GetULSpace().GetUpper();
                SwTwips nAdd = nPrevLineSpacing;
                
                
                if ( bUseFormerLineSpacing )
                {
                    
                    if ( pOwn->IsTxtFrm() )
                    {
                        nAdd = std::max( nAdd, static_cast<SwTxtFrm&>(m_rThis).GetLineSpace() );
                    }
                    nUpper += nAdd;
                }
                else
                {
                    
                    
                    
                    
                    
                    if ( pOwn->IsTxtFrm() )
                    {
                        
                        
                        
                        
                        
                        
                        
                        if ( bPrevLineSpacingPorportional )
                        {
                            nAdd += static_cast<SwTxtFrm&>(m_rThis).GetLineSpace( true );
                        }
                        else
                        {
                            nAdd = std::max( nAdd, static_cast<SwTxtFrm&>(m_rThis).GetLineSpace( true ) );
                        }
                    }
                    nUpper += nAdd;
                }
            }
            else
            {
                nUpper = std::max( static_cast<long>(nPrevLowerSpace),
                              static_cast<long>(pAttrs->GetULSpace().GetUpper()) );
                
                
                if ( bUseFormerLineSpacing )
                {
                    
                    if ( pOwn->IsTxtFrm() )
                        nUpper = std::max( nUpper, ((SwTxtFrm*)pOwn)->GetLineSpace() );
                    if ( nPrevLineSpacing != 0 )
                    {
                        nUpper = std::max( nUpper, nPrevLineSpacing );
                    }
                }
                else
                {
                    
                    
                    
                    
                    
                    
                    SwTwips nAdd = nPrevLineSpacing;
                    if ( pOwn->IsTxtFrm() )
                    {
                        
                        
                        
                        
                        
                        
                        
                        if ( bPrevLineSpacingPorportional )
                        {
                            nAdd += static_cast<SwTxtFrm&>(m_rThis).GetLineSpace( true );
                        }
                        else
                        {
                            nAdd = std::max( nAdd, static_cast<SwTxtFrm&>(m_rThis).GetLineSpace( true ) );
                        }
                    }
                    nUpper += nAdd;
                }
            }
        }
        else if ( pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES) &&
                  CastFlowFrm( pOwn )->HasParaSpaceAtPages( m_rThis.IsSctFrm() ) )
        {
            nUpper = pAttrs->GetULSpace().GetUpper();
        }
    }

    
    
    
    nUpper += pAttrs->GetTopLine( m_rThis, (pPr ? pPrevFrm : 0L) );

    
    

    
    if ( _bConsiderGrid && m_rThis.GetUpper()->GetFmt()->GetDoc()->IsSquaredPageMode() )
    {
        nUpper += _GetUpperSpaceAmountConsideredForPageGrid( nUpper );
    }

    bool bContextualSpacing = pAttrs->GetULSpace().GetContext();
    delete pAccess;

    if (bContextualSpacing && pPrevFrm && lcl_getContextualSpacing(pPrevFrm)
            && lcl_IdenticalStyles(pPrevFrm, &m_rThis))
    {
        return 0;
    }
    else
        return nUpper;
}

/** method to detemine the upper space amount, which is considered for
    the page grid

    OD 2004-03-12 #i11860#
    Precondition: Position of frame is valid.
*/
SwTwips SwFlowFrm::_GetUpperSpaceAmountConsideredForPageGrid(
                            const SwTwips _nUpperSpaceWithoutGrid ) const
{
    SwTwips nUpperSpaceAmountConsideredForPageGrid = 0;

    if ( m_rThis.IsInDocBody() && m_rThis.GetAttrSet()->GetParaGrid().GetValue() )
    {
        const SwPageFrm* pPageFrm = m_rThis.FindPageFrm();
        GETGRID( pPageFrm )
        if( pGrid )
        {
            const SwFrm* pBodyFrm = pPageFrm->FindBodyCont();
            if ( pBodyFrm )
            {
                const long nGridLineHeight =
                        pGrid->GetBaseHeight() + pGrid->GetRubyHeight();

                SWRECTFN( (&m_rThis) )
                const SwTwips nBodyPrtTop = (pBodyFrm->*fnRect->fnGetPrtTop)();
                const SwTwips nProposedPrtTop =
                        (*fnRect->fnYInc)( (m_rThis.Frm().*fnRect->fnGetTop)(),
                                           _nUpperSpaceWithoutGrid );

                const SwTwips nSpaceAbovePrtTop =
                        (*fnRect->fnYDiff)( nProposedPrtTop, nBodyPrtTop );
                const SwTwips nSpaceOfCompleteLinesAbove =
                        nGridLineHeight * ( nSpaceAbovePrtTop / nGridLineHeight );
                SwTwips nNewPrtTop =
                        (*fnRect->fnYInc)( nBodyPrtTop, nSpaceOfCompleteLinesAbove );
                if ( (*fnRect->fnYDiff)( nProposedPrtTop, nNewPrtTop ) > 0 )
                {
                    nNewPrtTop = (*fnRect->fnYInc)( nNewPrtTop, nGridLineHeight );
                }

                const SwTwips nNewUpperSpace =
                        (*fnRect->fnYDiff)( nNewPrtTop,
                                            (m_rThis.Frm().*fnRect->fnGetTop)() );

                nUpperSpaceAmountConsideredForPageGrid =
                        nNewUpperSpace - _nUpperSpaceWithoutGrid;

                OSL_ENSURE( nUpperSpaceAmountConsideredForPageGrid >= 0,
                        "<SwFlowFrm::GetUpperSpaceAmountConsideredForPageGrid(..)> - negative space considered for page grid!" );
            }
        }
    }
    return nUpperSpaceAmountConsideredForPageGrid;
}

/** method to determent the upper space amount, which is considered for
    the previous frame

    OD 2004-03-11 #i11860#
*/
SwTwips SwFlowFrm::_GetUpperSpaceAmountConsideredForPrevFrm() const
{
    SwTwips nUpperSpaceAmountOfPrevFrm = 0;

    const SwFrm* pPrevFrm = _GetPrevFrmForUpperSpaceCalc();
    if ( pPrevFrm )
    {
        SwTwips nPrevLowerSpace = 0;
        SwTwips nPrevLineSpacing = 0;
        
        bool bDummy = false;
        GetSpacingValuesOfFrm( (*pPrevFrm), nPrevLowerSpace, nPrevLineSpacing, bDummy );
        if ( nPrevLowerSpace > 0 || nPrevLineSpacing > 0 )
        {
            const IDocumentSettingAccess* pIDSA = m_rThis.GetUpper()->GetFmt()->getIDocumentSettingAccess();
            if (  pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX) ||
                 !pIDSA->get(IDocumentSettingAccess::OLD_LINE_SPACING) )
            {
                nUpperSpaceAmountOfPrevFrm = nPrevLowerSpace + nPrevLineSpacing;
            }
            else
            {
                nUpperSpaceAmountOfPrevFrm = std::max( nPrevLowerSpace, nPrevLineSpacing );
            }
        }
    }

    return nUpperSpaceAmountOfPrevFrm;
}

/** method to determine the upper space amount, which is considered for
    the previous frame and the page grid, if option 'Use former object
    positioning' is OFF

    OD 2004-03-18 #i11860#
*/
SwTwips SwFlowFrm::GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() const
{
    SwTwips nUpperSpaceAmountConsideredForPrevFrmAndPageGrid = 0;

    if ( !m_rThis.GetUpper()->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::USE_FORMER_OBJECT_POS) )
    {
        nUpperSpaceAmountConsideredForPrevFrmAndPageGrid =
            _GetUpperSpaceAmountConsideredForPrevFrm() +
            ( m_rThis.GetUpper()->GetFmt()->GetDoc()->IsSquaredPageMode()
              ? _GetUpperSpaceAmountConsideredForPageGrid( CalcUpperSpace( 0, 0, false ) )
              : 0 );
    }

    return nUpperSpaceAmountConsideredForPrevFrmAndPageGrid;
}

/** calculation of lower space

    OD 2004-03-02 #106629#
*/
SwTwips SwFlowFrm::CalcLowerSpace( const SwBorderAttrs* _pAttrs ) const
{
    SwTwips nLowerSpace = 0;

    SwBorderAttrAccess* pAttrAccess = 0L;
    if ( !_pAttrs )
    {
        pAttrAccess = new SwBorderAttrAccess( SwFrm::GetCache(), &m_rThis );
        _pAttrs = pAttrAccess->Get();
    }

    sal_Bool bCommonBorder = sal_True;
    if ( m_rThis.IsInSct() && m_rThis.GetUpper()->IsColBodyFrm() )
    {
        const SwSectionFrm* pSectFrm = m_rThis.FindSctFrm();
        bCommonBorder = pSectFrm->GetFmt()->GetBalancedColumns().GetValue();
    }
    nLowerSpace = bCommonBorder ?
                  _pAttrs->GetBottomLine( m_rThis ) :
                  _pAttrs->CalcBottomLine();

    
    
    
    if ( ( ( m_rThis.IsTabFrm() && m_rThis.GetUpper()->IsInTab() ) ||
           
           ( m_rThis.IsInTab() && !GetFollow() ) ) &&
         !m_rThis.GetIndNext() )
    {
        nLowerSpace += CalcAddLowerSpaceAsLastInTableCell( _pAttrs );
    }

    delete pAttrAccess;

    return nLowerSpace;
}

/** calculation of the additional space to be considered, if flow frame
    is the last inside a table cell

    OD 2004-07-16 #i26250#
*/
SwTwips SwFlowFrm::CalcAddLowerSpaceAsLastInTableCell(
                                            const SwBorderAttrs* _pAttrs ) const
{
    SwTwips nAdditionalLowerSpace = 0;

    if ( m_rThis.GetUpper()->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS) )
    {
        const SwFrm* pFrm = &m_rThis;
        if ( pFrm->IsSctFrm() )
        {
            const SwSectionFrm* pSectFrm = static_cast<const SwSectionFrm*>(pFrm);
            pFrm = pSectFrm->FindLastCntnt();
            if ( pFrm && pFrm->IsInTab() )
            {
                const SwTabFrm* pTableFrm = pFrm->FindTabFrm();
                if ( pSectFrm->IsAnLower( pTableFrm ) )
                {
                    pFrm = pTableFrm;
                }
            }
        }

        SwBorderAttrAccess* pAttrAccess = 0L;
        if ( !_pAttrs || pFrm != &m_rThis )
        {
            pAttrAccess = new SwBorderAttrAccess( SwFrm::GetCache(), pFrm );
            _pAttrs = pAttrAccess->Get();
        }

        nAdditionalLowerSpace += _pAttrs->GetULSpace().GetLower();

        delete pAttrAccess;
    }

    return nAdditionalLowerSpace;
}


sal_Bool SwFlowFrm::CheckMoveFwd( bool& rbMakePage, sal_Bool bKeep, sal_Bool )
{
    const SwFrm* pNxt = m_rThis.GetIndNext();

    if ( bKeep && 
         ( !pNxt || ( pNxt->IsTxtFrm() && ((SwTxtFrm*)pNxt)->IsEmptyMaster() ) ) &&
         ( 0 != (pNxt = m_rThis.FindNext()) ) && IsKeepFwdMoveAllowed() )
    {
        if( pNxt->IsSctFrm() )
        {   
            const SwFrm* pTmp = NULL;
            while( pNxt && pNxt->IsSctFrm() &&
                   ( !((SwSectionFrm*)pNxt)->GetSection() ||
                     0 == ( pTmp = ((SwSectionFrm*)pNxt)->ContainsAny() ) ) )
            {
                pNxt = pNxt->FindNext();
                pTmp = NULL;
            }
            if( pTmp )
                pNxt = pTmp; 
        }
        if( pNxt && pNxt->GetValidPosFlag() )
        {
            bool bMove = false;
            const SwSectionFrm *pSct = m_rThis.FindSctFrm();
            if( pSct && !pSct->GetValidSizeFlag() )
            {
                const SwSectionFrm* pNxtSct = pNxt->FindSctFrm();
                if( pNxtSct && pSct->IsAnFollow( pNxtSct ) )
                    bMove = true;
            }
            else
                bMove = true;
            if( bMove )
            {
                
                MoveFwd( rbMakePage, sal_False );
                return sal_True;
            }
        }
    }

    sal_Bool bMovedFwd = sal_False;

    if ( m_rThis.GetIndPrev() )
    {
        if ( IsPrevObjMove() ) 
        {
            bMovedFwd = sal_True;
            if ( !MoveFwd( rbMakePage, sal_False ) )
                rbMakePage = false;
        }
        else
        {
            if ( IsPageBreak( sal_False ) )
            {
                while ( MoveFwd( rbMakePage, sal_True ) )
                        /* do nothing */;
                rbMakePage = false;
                bMovedFwd = sal_True;
            }
            else if ( IsColBreak ( sal_False ) )
            {
                const SwPageFrm *pPage = m_rThis.FindPageFrm();
                SwFrm *pCol = m_rThis.FindColFrm();
                do
                {   MoveFwd( rbMakePage, sal_False );
                    SwFrm *pTmp = m_rThis.FindColFrm();
                    if( pTmp != pCol )
                    {
                        bMovedFwd = sal_True;
                        pCol = pTmp;
                    }
                    else
                        break;
                } while ( IsColBreak( sal_False ) );
                if ( pPage != m_rThis.FindPageFrm() )
                    rbMakePage = false;
            }
        }
    }
    return bMovedFwd;
}


sal_Bool SwFlowFrm::MoveFwd( sal_Bool bMakePage, sal_Bool bPageBreak, sal_Bool bMoveAlways )
{

    SwFtnBossFrm *pOldBoss = m_rThis.FindFtnBossFrm();
    if ( m_rThis.IsInFtn() )
        return ((SwCntntFrm&)m_rThis).MoveFtnCntFwd( bMakePage, pOldBoss );

    if( !IsFwdMoveAllowed() && !bMoveAlways )
    {
        bool bNoFwd = true;
        if( m_rThis.IsInSct() )
        {
            SwFtnBossFrm* pBoss = m_rThis.FindFtnBossFrm();
            bNoFwd = !pBoss->IsInSct() || ( !pBoss->Lower()->GetNext() &&
                     !pBoss->GetPrev() );
        }

        
        if ( m_rThis.IsInTab() &&
            ( !m_rThis.IsTabFrm() ||
                ( m_rThis.GetUpper()->IsInTab() &&
                  m_rThis.GetUpper()->FindTabFrm()->IsFwdMoveAllowed() ) ) &&
             0 != const_cast<SwFrm&>(m_rThis).GetNextCellLeaf( MAKEPAGE_NONE ) )
        {
            bNoFwd = false;
        }

        if( bNoFwd )
        {
            
            
            if ( !bPageBreak )
                return sal_False;

            const SwFrm *pCol = m_rThis.FindColFrm();
            if ( !pCol || !pCol->GetPrev() )
                return sal_False;
        }
    }

    sal_Bool bSamePage = sal_True;
    SwLayoutFrm *pNewUpper =
            m_rThis.GetLeaf( bMakePage ? MAKEPAGE_INSERT : MAKEPAGE_NONE, sal_True );

    if ( pNewUpper )
    {
        PROTOCOL_ENTER( &m_rThis, PROT_MOVE_FWD, 0, 0 );
        SwPageFrm *pOldPage = pOldBoss->FindPageFrm();
        
        

        
        
        SwSectionFrm* pSect = pNewUpper->FindSctFrm();
        if( pSect )
        {
            
            
            
            if( pSect != m_rThis.FindSctFrm() )
            {
                bool bUnlock = !pSect->IsColLocked();
                pSect->ColLock();
                pNewUpper->Calc();
                if( bUnlock )
                    pSect->ColUnlock();
            }
        }
        
        else if ( !pNewUpper->IsCellFrm() || ((SwLayoutFrm*)pNewUpper)->Lower() )
            pNewUpper->Calc();

        SwFtnBossFrm *pNewBoss = pNewUpper->FindFtnBossFrm();
        bool bBossChg = pNewBoss != pOldBoss;
        pNewBoss = pNewBoss->FindFtnBossFrm( sal_True );
        pOldBoss = pOldBoss->FindFtnBossFrm( sal_True );
        SwPageFrm* pNewPage = pOldPage;

        
        sal_Bool bFtnMoved = sal_False;

        
        
        
        
        
        
        
        const bool bForceSimpleFormat = pSect && pSect->HasFollow() &&
                                       !pSect->ContainsAny();

        if ( pNewBoss != pOldBoss )
        {
            pNewPage = pNewBoss->FindPageFrm();
            bSamePage = pNewPage == pOldPage;
            
            
            SWRECTFN( pOldBoss )
            SwSaveFtnHeight aHeight( pOldBoss,
                (pOldBoss->Frm().*fnRect->fnGetBottom)() );
            SwCntntFrm* pStart = m_rThis.IsCntntFrm() ?
                (SwCntntFrm*)&m_rThis : ((SwLayoutFrm&)m_rThis).ContainsCntnt();
            OSL_ENSURE( pStart || ( m_rThis.IsTabFrm() && !((SwTabFrm&)m_rThis).Lower() ),
                    "MoveFwd: Missing Content" );
            SwLayoutFrm* pBody = pStart ? ( pStart->IsTxtFrm() ?
                (SwLayoutFrm*)((SwTxtFrm*)pStart)->FindBodyFrm() : 0 ) : 0;
            if( pBody )
                bFtnMoved = pBody->MoveLowerFtns( pStart, pOldBoss, pNewBoss,
                                                  sal_False);
        }
        
        
        
        
        if( pNewUpper != m_rThis.GetUpper() )
        {
            
            SwSectionFrm* pOldSct = 0;
            if ( m_rThis.GetUpper()->IsSctFrm() )
            {
                pOldSct = static_cast<SwSectionFrm*>(m_rThis.GetUpper());
            }

            MoveSubTree( pNewUpper, pNewUpper->Lower() );

            
            if ( pOldSct && pOldSct->GetSection() )
            {
                
                
                
                
                
                
                
                
                
                pOldSct->SimpleFormat();
            }

            
            if ( bForceSimpleFormat )
            {
                pSect->SimpleFormat();
            }

            if ( bFtnMoved && !bSamePage )
            {
                pOldPage->UpdateFtnNum();
                pNewPage->UpdateFtnNum();
            }

            if( bBossChg )
            {
                m_rThis.Prepare( PREP_BOSS_CHGD, 0, sal_False );
                if( !bSamePage )
                {
                    SwViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();
                    if ( pSh && !pSh->Imp()->IsUpdateExpFlds() )
                        pSh->GetDoc()->SetNewFldLst(true);  

                    pNewPage->InvalidateSpelling();
                    pNewPage->InvalidateSmartTags();    
                    pNewPage->InvalidateAutoCompleteWords();
                    pNewPage->InvalidateWordCount();
                }
            }
        }
        
        const SwViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();

        if ( !( pSh && pSh->GetViewOptions()->getBrowseMode() ) )
        {
            
            
            if ( !bSamePage &&
                 ( m_rThis.GetAttrSet()->GetPageDesc().GetPageDesc() ||
                   pOldPage->GetPageDesc()->GetFollow() != pNewPage->GetPageDesc() ) )
            {
                SwFrm::CheckPageDescs( pNewPage, sal_False );
            }
        }
    }
    return bSamePage;
}

/** Return value tells whether the Frm should change the page.
 *
 * @note This should be called by derived classes.
 * @note The actual moving must be implemented in the subclasses.
 */
sal_Bool SwFlowFrm::MoveBwd( sal_Bool &rbReformat )
{
    SwFlowFrm::SetMoveBwdJump( sal_False );

    SwFtnFrm* pFtn = m_rThis.FindFtnFrm();
    if ( pFtn && pFtn->IsBackMoveLocked() )
        return sal_False;

    
    
    if ( m_rThis.IsTxtFrm() && m_rThis.IsInTab() )
    {
        const SwLayoutFrm* pUpperFrm = m_rThis.GetUpper();
        while ( pUpperFrm )
        {
            if ( pUpperFrm->IsTabFrm() )
            {
                return sal_False;
            }
            else if ( pUpperFrm->IsColumnFrm() && pUpperFrm->IsInSct() )
            {
                break;
            }
            pUpperFrm = pUpperFrm->GetUpper();
        }
    }

    SwFtnBossFrm * pOldBoss = m_rThis.FindFtnBossFrm();
    SwPageFrm * const pOldPage = pOldBoss->FindPageFrm();
    SwLayoutFrm *pNewUpper = 0;
    bool bCheckPageDescs = false;
    bool bCheckPageDescOfNextPage = false;

    if ( pFtn )
    {
        
        

        
        
        SwFrm* pRef = 0;
        const bool bEndnote = pFtn->GetAttr()->GetFtn().IsEndNote();
        if( bEndnote && pFtn->IsInSct() )
        {
            SwSectionFrm* pSect = pFtn->FindSctFrm();
            if( pSect->IsEndnAtEnd() )
                pRef = pSect->FindLastCntnt( FINDMODE_LASTCNT );
        }
        if( !pRef )
            pRef = pFtn->GetRef();

        OSL_ENSURE( pRef, "MoveBwd: Endnote for an empty section?" );

        if( !bEndnote )
            pOldBoss = pOldBoss->FindFtnBossFrm( sal_True );
        SwFtnBossFrm *pRefBoss = pRef->FindFtnBossFrm( !bEndnote );
        if ( pOldBoss != pRefBoss &&
             
             ( !bEndnote ||
               pRefBoss->IsBefore( pOldBoss ) )
           )
            pNewUpper = m_rThis.GetLeaf( MAKEPAGE_FTN, sal_False );
    }
    else if ( IsPageBreak( sal_True ) ) 
    {
        
        
        
        
        const SwFrm *pFlow = &m_rThis;
        do
        {
            pFlow = pFlow->FindPrev();
        } while ( pFlow &&
                  ( pFlow->FindPageFrm() == pOldPage ||
                    !pFlow->IsInDocBody() ) );
        if ( pFlow )
        {
            long nDiff = pOldPage->GetPhyPageNum() - pFlow->GetPhyPageNum();
            if ( nDiff > 1 )
            {
                if ( ((SwPageFrm*)pOldPage->GetPrev())->IsEmptyPage() )
                    nDiff -= 1;
                if ( nDiff > 1 )
                {
                    pNewUpper = m_rThis.GetLeaf( MAKEPAGE_NONE, sal_False );
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    if ( pNewUpper->Lower() )
                    {
                        SwLayoutFrm* pNewNextUpper = pNewUpper->GetLeaf( MAKEPAGE_NONE, sal_True );
                        if ( pNewNextUpper &&
                             pNewNextUpper != m_rThis.GetUpper() &&
                             pNewNextUpper->GetType() == pNewUpper->GetType() &&
                             pNewNextUpper->IsInDocBody() == pNewUpper->IsInDocBody() &&
                             pNewNextUpper->IsInFtn() == pNewUpper->IsInFtn() &&
                             pNewNextUpper->IsInTab() == pNewUpper->IsInTab() &&
                             pNewNextUpper->IsInSct() == pNewUpper->IsInSct() &&
                             !m_rThis.WrongPageDesc( pNewNextUpper->FindPageFrm() ) )
                        {
                            pNewUpper = pNewNextUpper;
                            bCheckPageDescOfNextPage = true;
                        }
                    }

                    bCheckPageDescs = true;
                }
            }
        }
    }
    else if ( IsColBreak( sal_True ) )
    {
        
        
        
        if( m_rThis.IsInSct() )
        {
            pNewUpper = m_rThis.GetLeaf( MAKEPAGE_NONE, sal_False );
            if( pNewUpper && !SwFlowFrm::IsMoveBwdJump() &&
                ( pNewUpper->ContainsCntnt() ||
                  ( ( !pNewUpper->IsColBodyFrm() ||
                      !pNewUpper->GetUpper()->GetPrev() ) &&
                    !pNewUpper->FindSctFrm()->GetPrev() ) ) )
            {
                pNewUpper = 0;
            }
            
            
            
            else if ( pNewUpper && !SwFlowFrm::IsMoveBwdJump() )
            {
                
                
                
                
                
                
                
                
                
                
                SwLayoutFrm* pNewNextUpper = pNewUpper->GetLeaf( MAKEPAGE_NOSECTION, sal_True );
                if ( pNewNextUpper &&
                     pNewNextUpper->GetType() == pNewUpper->GetType() &&
                     pNewNextUpper->IsInDocBody() == pNewUpper->IsInDocBody() &&
                     pNewNextUpper->IsInFtn() == pNewUpper->IsInFtn() &&
                     pNewNextUpper->IsInTab() == pNewUpper->IsInTab() &&
                     pNewNextUpper->IsInSct() == pNewUpper->IsInSct() &&
                     !m_rThis.WrongPageDesc( pNewNextUpper->FindPageFrm() ) )
                {
                    pNewUpper = pNewNextUpper;
                }
            }
        }
        else
        {
            const SwFrm *pCol = m_rThis.FindColFrm();
            bool bGoOn = true;
            bool bJump = false;
            do
            {
                if ( pCol->GetPrev() )
                    pCol = pCol->GetPrev();
                else
                {
                    bGoOn = false;
                    pCol = m_rThis.GetLeaf( MAKEPAGE_NONE, sal_False );
                }
                if ( pCol )
                {
                    
                    SwLayoutFrm* pColBody = pCol->IsColumnFrm() ?
                        (SwLayoutFrm*)((SwLayoutFrm*)pCol)->Lower() :
                        (SwLayoutFrm*)pCol;
                    if ( pColBody->ContainsCntnt() )
                    {
                        bGoOn = false; 
                        
                        if( SwFlowFrm::IsMoveBwdJump() )
                        {
                            pNewUpper = pColBody;
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            SwLayoutFrm* pNewNextUpper = pNewUpper->GetLeaf( MAKEPAGE_NONE, sal_True );
                            if ( pNewNextUpper &&
                                 pNewNextUpper != m_rThis.GetUpper() &&
                                 pNewNextUpper->GetType() == pNewUpper->GetType() &&
                                 pNewNextUpper->IsInDocBody() == pNewUpper->IsInDocBody() &&
                                 pNewNextUpper->IsInFtn() == pNewUpper->IsInFtn() &&
                                 pNewNextUpper->IsInTab() == pNewUpper->IsInTab() &&
                                 pNewNextUpper->IsInSct() == pNewUpper->IsInSct() &&
                                 !m_rThis.WrongPageDesc( pNewNextUpper->FindPageFrm() ) )
                            {
                                pNewUpper = pNewNextUpper;
                            }
                        }
                    }
                    else
                    {
                        if( pNewUpper )        
                            bJump = true;      
                        pNewUpper = pColBody;  
                                               
                    }
                }
            } while( bGoOn );
            if( bJump )
                SwFlowFrm::SetMoveBwdJump( sal_True );
        }
    }
    else 
        pNewUpper = m_rThis.GetLeaf( MAKEPAGE_NONE, sal_False );

    
    
    
    
    
    
    
    
    if ( pNewUpper &&
         m_rThis.IsTxtFrm() && !IsFollow() )
    {
        sal_uInt32 nToPageNum( 0L );
        const bool bMoveFwdByObjPos = SwLayouter::FrmMovedFwdByObjPos(
                                                *(pOldPage->GetFmt()->GetDoc()),
                                                static_cast<SwTxtFrm&>(m_rThis),
                                                nToPageNum );
        if ( bMoveFwdByObjPos &&
             pNewUpper->FindPageFrm()->GetPhyPageNum() < nToPageNum )
        {
            pNewUpper = 0;
        }
        
        
        else if ( m_rThis.GetDrawObjs() )
        {
            sal_uInt32 i = 0;
            for ( ; i < m_rThis.GetDrawObjs()->Count(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*m_rThis.GetDrawObjs())[i];
                
                if ( pAnchoredObj->RestartLayoutProcess() &&
                     !pAnchoredObj->IsTmpConsiderWrapInfluence() )
                {
                    pNewUpper = 0;
                    break;
                }
            }
        }
    }

    
    
    
    if ( pNewUpper && IsFollow() && pNewUpper->Lower() )
    {
        
        
        bool bProposedNewUpperContainsOnlyEmptySections( true );
        {
            const SwFrm* pLower( pNewUpper->Lower() );
            while ( pLower )
            {
                if ( pLower->IsSctFrm() &&
                     !dynamic_cast<const SwSectionFrm*>(pLower)->GetSection() )
                {
                    pLower = pLower->GetNext();
                    continue;
                }
                else
                {
                    bProposedNewUpperContainsOnlyEmptySections = false;
                    break;
                }
            }
        }
        if ( !bProposedNewUpperContainsOnlyEmptySections )
        {
            if ( SwFlowFrm::IsMoveBwdJump() )
            {
                
                SwFrm *pFrm = pNewUpper->Lower();
                while ( pFrm->GetNext() )
                    pFrm = pFrm->GetNext();
                pNewUpper = pFrm->GetLeaf( MAKEPAGE_INSERT, sal_True );
                if( pNewUpper == m_rThis.GetUpper() ) 
                    pNewUpper = NULL;               
            }
            else
                pNewUpper = 0;
        }
    }
    if ( pNewUpper && !ShouldBwdMoved( pNewUpper, sal_True, rbReformat ) )
    {
        if( !pNewUpper->Lower() )
        {
            if( pNewUpper->IsFtnContFrm() )
            {
                pNewUpper->Cut();
                delete pNewUpper;
            }
            else
            {
                SwSectionFrm* pSectFrm = pNewUpper->FindSctFrm();
                
                
                if ( pSectFrm && !pSectFrm->IsColLocked() &&
                     !pSectFrm->ContainsCntnt() && !pSectFrm->ContainsAny( true ) )
                {
                    pSectFrm->DelEmpty( sal_True );
                    delete pSectFrm;
                    m_rThis.mbValidPos = sal_True;
                }
            }
        }
        pNewUpper = 0;
    }

    
    
    
    
    if ( pNewUpper && !IsFollow() &&
         m_rThis.GetAttrSet()->GetKeep().GetValue() && m_rThis.GetIndNext() )
    {
        SwFrm* pIndNext = m_rThis.GetIndNext();
        
        if ( !pIndNext->IsTabFrm() )
        {
            
            while ( pIndNext && pIndNext->IsSctFrm() )
            {
                if( static_cast<SwSectionFrm*>(pIndNext)->GetSection() )
                {
                    SwFrm* pTmp = static_cast<SwSectionFrm*>(pIndNext)->ContainsAny();
                    if ( pTmp )
                    {
                        pIndNext = pTmp;
                        break;
                    }
                }
                pIndNext = pIndNext->GetIndNext();
            }
            OSL_ENSURE( !pIndNext || pIndNext->ISA(SwTxtFrm),
                    "<SwFlowFrm::MovedBwd(..)> - incorrect next found." );
            if ( pIndNext && pIndNext->IsFlowFrm() &&
                 SwFlowFrm::CastFlowFrm(pIndNext)->IsJoinLocked() )
            {
                pNewUpper = 0L;
            }
        }
    }

    
    
    
    if ( pNewUpper && !IsFollow() &&
         pNewUpper != m_rThis.GetUpper() &&
         SwLayouter::MoveBwdSuppressed( *(pOldPage->GetFmt()->GetDoc()),
                                        *this, *pNewUpper ) )
    {
        SwLayoutFrm* pNextNewUpper = pNewUpper->GetLeaf(
                                    ( !m_rThis.IsSctFrm() && m_rThis.IsInSct() )
                                    ? MAKEPAGE_NOSECTION
                                    : MAKEPAGE_NONE,
                                    sal_True );
        
        OSL_ENSURE( pNextNewUpper, "<SwFlowFrm::MoveBwd(..)> - missing next new upper" );
        if ( pNextNewUpper &&
             ( pNextNewUpper == m_rThis.GetUpper() ||
               pNextNewUpper->GetType() != m_rThis.GetUpper()->GetType() ) )
        {
            pNewUpper = 0L;
            OSL_FAIL( "<SwFlowFrm::MoveBwd(..)> - layout loop control for layout action <Move Backward> applied!" );
        }
    }

    OSL_ENSURE( pNewUpper != m_rThis.GetUpper(),
            "<SwFlowFrm::MoveBwd(..)> - moving backward to the current upper frame!? -> Please inform OD." );
    if ( pNewUpper )
    {
        PROTOCOL_ENTER( &m_rThis, PROT_MOVE_BWD, 0, 0 );
        if ( pNewUpper->IsFtnContFrm() )
        {
            
            SwFtnFrm *pOld = m_rThis.FindFtnFrm();
            SwFtnFrm *pNew = new SwFtnFrm( pOld->GetFmt(), pOld,
                                           pOld->GetRef(), pOld->GetAttr() );
            if ( pOld->GetMaster() )
            {
                pNew->SetMaster( pOld->GetMaster() );
                pOld->GetMaster()->SetFollow( pNew );
            }
            pNew->SetFollow( pOld );
            pOld->SetMaster( pNew );
            pNew->Paste( pNewUpper );
            pNewUpper = pNew;
        }
        if( pNewUpper->IsFtnFrm() && m_rThis.IsInSct() )
        {
            SwSectionFrm* pSct = m_rThis.FindSctFrm();
            
            
            if( pSct->IsInFtn() )
            {
                SwFrm* pTmp = pNewUpper->Lower();
                if( pTmp )
                {
                    while( pTmp->GetNext() )
                        pTmp = pTmp->GetNext();
                    if( !pTmp->IsSctFrm() ||
                        ((SwSectionFrm*)pTmp)->GetFollow() != pSct )
                        pTmp = NULL;
                }
                if( pTmp )
                    pNewUpper = (SwSectionFrm*)pTmp;
                else
                {
                    pSct = new SwSectionFrm( *pSct, sal_True );
                    pSct->Paste( pNewUpper );
                    pSct->Init();
                    pNewUpper = pSct;
                    pSct->SimpleFormat();
                }
            }
        }
        bool bUnlock = false;
        sal_Bool bFollow = sal_False;
        
        
        SwSectionFrm* pSect = pNewUpper->FindSctFrm();
        if( pSect )
        {
            bUnlock = !pSect->IsColLocked();
            pSect->ColLock();
            bFollow = pSect->HasFollow();
        }
        pNewUpper->Calc();
        m_rThis.Cut();
        //
        
        
        bool bFormatSect( false );
        if( bUnlock )
        {
            pSect->ColUnlock();
            if( pSect->HasFollow() != bFollow )
            {
                pSect->InvalidateSize();
                
                if ( pSect == pNewUpper )
                    bFormatSect = true;
            }
        }

        m_rThis.Paste( pNewUpper );
        
        if ( bFormatSect )
            pSect->Calc();

        SwPageFrm *pNewPage = m_rThis.FindPageFrm();
        if( pNewPage != pOldPage )
        {
            m_rThis.Prepare( PREP_BOSS_CHGD, (const void*)pOldPage, sal_False );
            SwViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();
            if ( pSh && !pSh->Imp()->IsUpdateExpFlds() )
                pSh->GetDoc()->SetNewFldLst(true);  

            pNewPage->InvalidateSpelling();
            pNewPage->InvalidateSmartTags();    
            pNewPage->InvalidateAutoCompleteWords();
            pNewPage->InvalidateWordCount();

            
            if ( !( pSh && pSh->GetViewOptions()->getBrowseMode() ) )
            {
                if ( bCheckPageDescs && pNewPage->GetNext() )
                {
                    SwPageFrm* pStartPage = bCheckPageDescOfNextPage ?
                                            pNewPage :
                                            (SwPageFrm*)pNewPage->GetNext();
                    SwFrm::CheckPageDescs( pStartPage, sal_False);
                }
                else if ( m_rThis.GetAttrSet()->GetPageDesc().GetPageDesc() )
                {
                    
                    
                    SwFrm::CheckPageDescs( (SwPageFrm*)pNewPage, sal_False);
                }
            }
        }
    }
    return pNewUpper != 0;
}

SwFlowFrm *SwFlowFrm::CastFlowFrm( SwFrm *pFrm )
{
    if ( pFrm->IsCntntFrm() )
        return (SwCntntFrm*)pFrm;
    if ( pFrm->IsTabFrm() )
        return (SwTabFrm*)pFrm;
    if ( pFrm->IsSctFrm() )
        return (SwSectionFrm*)pFrm;
    return 0;
}

const SwFlowFrm *SwFlowFrm::CastFlowFrm( const SwFrm *pFrm )
{
    if ( pFrm->IsCntntFrm() )
        return (SwCntntFrm*)pFrm;
    if ( pFrm->IsTabFrm() )
        return (SwTabFrm*)pFrm;
    if ( pFrm->IsSctFrm() )
        return (SwSectionFrm*)pFrm;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
