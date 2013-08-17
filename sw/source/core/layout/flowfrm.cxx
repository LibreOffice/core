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


/*************************************************************************
|*
|*  SwFlowFrm::SwFlowFrm()
|*
|*************************************************************************/


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
        if (m_pFollow->m_pPrecede) // re-chaining pFollow?
        {
            assert(m_pFollow == m_pFollow->m_pPrecede->m_pFollow);
            m_pFollow->m_pPrecede->m_pFollow = 0;
        }
        m_pFollow->m_pPrecede = this;
    }
}

/*************************************************************************
|*
|*  SwFlowFrm::IsFollowLocked()
|*     return sal_True if any follow has the JoinLocked flag
|*
|*************************************************************************/

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

/*************************************************************************
|*
|*  SwFlowFrm::IsKeepFwdMoveAllowed()
|*
|*************************************************************************/


sal_Bool SwFlowFrm::IsKeepFwdMoveAllowed()
{
    // If all the predecessors up to the first of the chain have
    // the 'keep' attribute set, and the first of the chain's
    // IsFwdMoveAllowed returns sal_False, then we're not allowed to move.
    SwFrm *pFrm = &m_rThis;
    if ( !pFrm->IsInFtn() )
        do
        {   if ( pFrm->GetAttrSet()->GetKeep().GetValue() )
                pFrm = pFrm->GetIndPrev();
            else
                return sal_True;
        } while ( pFrm );

                  //See IsFwdMoveAllowed()
    sal_Bool bRet = sal_False;
    if ( pFrm && pFrm->GetIndPrev() )
        bRet = sal_True;
    return bRet;
}

/*************************************************************************
|*
|*    SwFlowFrm::CheckKeep()
|*
|*************************************************************************/


void SwFlowFrm::CheckKeep()
{
    // Kick off the "last" predecessor with a 'keep' attribute, because
    // it's possible for the whole troop to move back.
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

/*************************************************************************
|*
|*  SwFlowFrm::IsKeep()
|*
|*************************************************************************/

sal_Bool SwFlowFrm::IsKeep( const SwAttrSet& rAttrs, bool bCheckIfLastRowShouldKeep ) const
{
    // 1. The keep attribute is ignored inside footnotes
    // 2. For compatibility reasons, the keep attribute is
    //    ignored for frames inside table cells
    // 3. If bBreakCheck is set to true, this function only checks
    //    if there are any break after attributes set at rAttrs
    //    or break before attributes set for the next content (or next table)
    sal_Bool bKeep = bCheckIfLastRowShouldKeep ||
                 (  !m_rThis.IsInFtn() &&
                    ( !m_rThis.IsInTab() || m_rThis.IsTabFrm() ) &&
                    rAttrs.GetKeep().GetValue() );

    OSL_ENSURE( !bCheckIfLastRowShouldKeep || m_rThis.IsTabFrm(),
            "IsKeep with bCheckIfLastRowShouldKeep should only be used for tabfrms" );

    // Ignore keep attribute if there are break situations:
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
                // #135914#
                // The last row of a table only keeps with the next content
                // it they are in the same section:
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

/*************************************************************************
|*
|*  SwFlowFrm::BwdMoveNecessary()
|*
|*************************************************************************/


sal_uInt8 SwFlowFrm::BwdMoveNecessary( const SwPageFrm *pPage, const SwRect &rRect )
{
    // The return value helps deciding whether we need to flow back (3),
    // or whether we can use the good old WouldFit (0, 1), or if
    // it's reasonable to relocate and test-format (2).
    //
    // Bit 1 in this case means that there are objects anchored to myself,
    // bit 2 means that I have to evade other objects.

    // If a SurroundObj that desires to be wrapped around overlaps with the
    // Rect, it's required to flow (because we can't guess the relationships).
    // However it's possible for a test formatting to happen.
    // If the SurroundObj is a Fly and I'm a Lower, or the Fly is a Lower of
    // mine, then it doesn't matter.
    // If the SurroundObj is anchored in a character bound Fly, and I'm not
    // a Lower of that character bound Fly myself, then the Fly doesn't matter.

    // #32639# If the object is anchored with me, i can ignore it, because
    // it's likely that it will follow me with the flow. A test formatting is
    // not allowed in that case, however!
    sal_uInt8 nRet = 0;
    SwFlowFrm *pTmp = this;
    do
    {   // If there are objects hanging either on me or on a follow, we can't
        // do a test formatting, because paragraph bound objects wouldn't
        // be properly considered, and character bound objects shouldn't
        // be test formatted at all.
        if( pTmp->GetFrm()->GetDrawObjs() )
            nRet = 1;
        pTmp = pTmp->GetFollow();
    } while ( !nRet && pTmp );
    if ( pPage->GetSortedObjs() )
    {
        // #i28701# - new type <SwSortedObjs>
        const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
        sal_uLong nIndex = ULONG_MAX;
        for ( sal_uInt16 i = 0; nRet < 3 && i < rObjs.Count(); ++i )
        {
            // #i28701# - consider changed type of
            // <SwSortedObjs> entries.
            SwAnchoredObject* pObj = rObjs[i];
            const SwFrmFmt& rFmt = pObj->GetFrmFmt();
            const SwRect aRect( pObj->GetObjRect() );
            if ( aRect.IsOver( rRect ) &&
                 rFmt.GetSurround().GetSurround() != SURROUND_THROUGHT )
            {
                if( m_rThis.IsLayoutFrm() && //Fly Lower of This?
                    Is_Lower_Of( &m_rThis, pObj->GetDrawObj() ) )
                    continue;
                if( pObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pObj);
                    if ( pFly->IsAnLower( &m_rThis ) )//This Lower of Fly?
                        continue;
                }

                const SwFrm* pAnchor = pObj->GetAnchorFrm();
                if ( pAnchor == &m_rThis )
                {
                    nRet |= 1;
                    continue;
                }

                // Don't do this if the object is anchored behind me in the text
                // flow, because then I wouldn't evade it.
                if ( ::IsFrmInSameKontext( pAnchor, &m_rThis ) )
                {
                    if ( rFmt.GetAnchor().GetAnchorId() == FLY_AT_PARA )
                    {
                        // The index of the other one can be retrieved using the anchor attribute.
                        sal_uLong nTmpIndex = rFmt.GetAnchor().GetCntntAnchor()->nNode.GetIndex();
                        // Now we're going to check whether the current paragraph before
                        // the anchor of the displacing object sits in the text. If this
                        // is the case, we don't try to evade it.
                        // The index is being determined via SwFmtAnchor, because it's
                        // getting quite expensive otherwise.
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

/*************************************************************************
|*
|*  SwFlowFrm::CutTree(), PasteTree(), MoveSubTree()
|*
|*  Description        A specialized form of Cut() and Paste(), which
|*      relocates a whole chain (this and the following, in particular).
|*      During this process, only the minimum operations and notifications
|*      are done.
|*
|*************************************************************************/


SwLayoutFrm *SwFlowFrm::CutTree( SwFrm *pStart )
{
    // Cut the Start and all the neighbours; they are chained together and
    // a handle to the first one is returned. Residuals are invalidated
    // as appropriate.

    SwLayoutFrm *pLay = pStart->GetUpper();
    if ( pLay->IsInFtn() )
        pLay = pLay->FindFtnFrm();

    // #i58846#
    // <pPrepare( PREP_QUOVADIS )> only for frames in footnotes
    if( pStart->IsInFtn() )
    {
        SwFrm* pTmp = pStart->GetIndPrev();
        if( pTmp )
            pTmp->Prepare( PREP_QUOVADIS );
    }

    // Just cut quickly and take care that we don't cause problems with the
    // left-behinds. The pointers of the chain being cut can point who-knows where.
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
                // It's possible for the CntFrm to be locked, and we don't want
                // to end up in an endless page migration, so we're not even
                // going to call Calc!
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
    //returns sal_True if there's a LayoutFrm in the chain.
    sal_Bool bRet = sal_False;

    // The chain beginning with pStart is inserted before pSibling
    // under the parent. We take care to invalidate as required.

    // I'm receiving a finished chain. We need to update the pointers for
    // the beginning of the chain, then all the uppers and finally the end.
    // On the way there, we invalidate as required.
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
        //Modified for #i100782#,04/03/2009
        //If the pParent has more than 1 child nodes, former design will
        //ignore them directly without any collection work. It will make some
        //dangling pointers. This lead the crash...
        //The new design will find the last child of pParent in loop way, and
        //add the pStart after the last child.
        //  pParent->Lower()->pNext = pStart;
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
        //End modification for #i100782#,04/03/2009

        // #i27145#
        if ( pParent->IsSctFrm() )
        {
            // We have no sibling because pParent is a section frame and
            // has just been created to contain some content. The printing
            // area of the frame behind pParent has to be invalidated, so
            // that the correct distance between pParent and the next frame
            // can be calculated.
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

        // I'm a friend of the TxtFrm and thus am allowed to do many things.
        // The CacheIdx idea seems to be a bit risky!
        if ( pFloat->IsTxtFrm() )
        {
            if ( ((SwTxtFrm*)pFloat)->GetCacheIdx() != USHRT_MAX )
                ((SwTxtFrm*)pFloat)->Init();    // I'm his friend.
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
        if ( pOldParent && pOldParent->IsBodyFrm() ) // For variable page height while browsing
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

    // Be economical with notifications if an action is running.
    ViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();
    const SwViewImp *pImp = pSh ? pSh->Imp() : 0;
    const bool bComplete = pImp && pImp->IsAction() && pImp->GetLayAction().IsComplete();

    if ( !bComplete )
    {
        SwFrm *pPre = m_rThis.GetIndPrev();
        if ( pPre )
        {
            pPre->SetRetouche();
            // #115759# - follow-up of #i26250#
            // invalidate printing area of previous frame, if it's in a table
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

    // If, by cutting & pasting, an empty SectionFrm came into existence, it should
    // disappear automatically.
    SwSectionFrm *pSct;
    // #126020# - adjust check for empty section
    // #130797# - correct fix #126020#
    if ( pOldParent && !pOldParent->Lower() &&
         ( pOldParent->IsInSct() &&
           !(pSct = pOldParent->FindSctFrm())->ContainsCntnt() &&
           !pSct->ContainsAny( true ) ) )
    {
            pSct->DelEmpty( sal_False );
    }

    // If we're in a column section, we'd rather not call Calc "from below"
    if( !m_rThis.IsInSct() &&
        ( !m_rThis.IsInTab() || ( m_rThis.IsTabFrm() && !m_rThis.GetUpper()->IsInTab() ) ) )
        m_rThis.GetUpper()->Calc();
    else if( m_rThis.GetUpper()->IsSctFrm() )
    {
        SwSectionFrm* pTmpSct = (SwSectionFrm*)m_rThis.GetUpper();
        sal_Bool bOld = pTmpSct->IsCntntLocked();
        pTmpSct->SetCntntLock( sal_True );
        pTmpSct->Calc();
        if( !bOld )
            pTmpSct->SetCntntLock( sal_False );
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

/*************************************************************************
|*
|*  SwFlowFrm::IsAnFollow()
|*
|*************************************************************************/


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


/*************************************************************************
|*
|*  SwFlowFrm::FindMaster()
|*
|*************************************************************************/

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
                //
                // Optimization. This makes code like this obsolete:
                // while ( pTab->IsFollow() )
                //     pTab = pTab->FindMaster();
                //
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

/*************************************************************************
|*
|*  SwFrm::GetLeaf()
|*
|*  Description        Returns the next/previous Layout leaf that's NOT below
|*      this (or even is this itself). Also, that leaf must be in the same
|*      text flow as the pAnch origin frm (Body, Ftn)
|*
|*************************************************************************/


const SwLayoutFrm *SwFrm::GetLeaf( MakePageType eMakePage, sal_Bool bFwd,
                                   const SwFrm *pAnch ) const
{
    // No flow, no joy...
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

/*************************************************************************
|*
|*  SwFrm::GetLeaf()
|*
|*  Description        calls Get[Next|Prev]Leaf
|*
|*************************************************************************/


SwLayoutFrm *SwFrm::GetLeaf( MakePageType eMakePage, sal_Bool bFwd )
{
    if ( IsInFtn() )
        return bFwd ? GetNextFtnLeaf( eMakePage ) : GetPrevFtnLeaf( eMakePage );

    // #i53323#
    // A frame could be inside a table AND inside a section.
    // Thus, it has to be determined, which is the first parent.
    bool bInTab( IsInTab() );
    bool bInSct( IsInSct() );
    if ( bInTab && bInSct )
    {
        const SwFrm* pUpperFrm( GetUpper() );
        while ( pUpperFrm )
        {
            if ( pUpperFrm->IsTabFrm() )
            {
                // the table is the first.
                bInSct = false;
                break;
            }
            else if ( pUpperFrm->IsSctFrm() )
            {
                // the section is the first.
                bInTab = false;
                break;
            }

            pUpperFrm = pUpperFrm->GetUpper();
        }
    }

    if ( bInTab && ( !IsTabFrm() || GetUpper()->IsCellFrm() ) ) // TABLE IN TABLE
        return bFwd ? GetNextCellLeaf( eMakePage ) : GetPrevCellLeaf( eMakePage );

    if ( bInSct )
        return bFwd ? GetNextSctLeaf( eMakePage ) : GetPrevSctLeaf( eMakePage );

    return bFwd ? GetNextLeaf( eMakePage ) : GetPrevLeaf( eMakePage );
}



sal_Bool SwFrm::WrongPageDesc( SwPageFrm* pNew )
{
    // Now it's getting a bit complicated:
    //
    // Maybe i'm bringing a Pagedesc myself; in that case,
    // the pagedesc of the next page needs to correspond.
    // Otherwise, I'll have to dig a bit deeper to see where
    // the following Pagedesc is coming from.
    // If the following page itself tells me that it's pagedesc
    // is wrong, I can happily exchange it.
    // If the page however thinks that it's pagedesc is correct,
    // this doesn't mean it's useful to me:
    // If the first BodyCntnt asks for a PageDesc or a PageBreak,
    // I'll have to insert a new page - except the desired page is
    // the correct one.
    // If I inserted a new page, the problems only get started:
    // because then it's likely for the next page to have been
    // wrong and having been swapped because of that.
    // This in turn means that I have a new (and correct) page,
    // but the conditions to swap still apply.
    // Way out of the situation: Try to preliminarily insert a
    // new page once (empty pages are already inserted by InsertPage()
    // if required)
    const SwFmtPageDesc &rFmtDesc = GetAttrSet()->GetPageDesc();

    //My Pagedesc doesn't count if I'm a follow!
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
                nTmp = rFmtDesc.GetNumOffset();
        }
    }

    // Does the Cntnt bring a Pagedesc or do we need the
    // virtual page number of the new layout leaf?
    // PageDesc isn't allowed with Follows
    const sal_Bool bOdd = nTmp ? ( (nTmp % 2) ? sal_True : sal_False )
                           : pNew->OnRightPage();
    if ( !pDesc )
        pDesc = pNew->FindPageDesc();

    bool bFirst = pNew->OnFirstPage();

    const SwFlowFrm *pNewFlow = pNew->FindFirstBodyCntnt();
    // Did we find ourselves?
    if( pNewFlow == pFlow )
        pNewFlow = NULL;
    if ( pNewFlow && pNewFlow->GetFrm()->IsInTab() )
        pNewFlow = pNewFlow->GetFrm()->FindTabFrm();
    const SwPageDesc *pNewDesc= ( pNewFlow && !pNewFlow->IsFollow() )
            ? pNewFlow->GetFrm()->GetAttrSet()->GetPageDesc().GetPageDesc() : 0;

    return (pNew->GetPageDesc() != pDesc)   //  own desc ?
        || (pNew->GetFmt() !=
              (bOdd ? pDesc->GetRightFmt(bFirst) : pDesc->GetLeftFmt(bFirst)))
        || (pNewDesc && pNewDesc == pDesc);
}


/*************************************************************************
|*
|*  SwFrm::GetNextLeaf()
|*
|*  Description        Returns the next layout leaf in which we can move
|*      the frame.
|*
|*************************************************************************/

SwLayoutFrm *SwFrm::GetNextLeaf( MakePageType eMakePage )
{
    OSL_ENSURE( !IsInFtn(), "GetNextLeaf(), don't call me for Ftn." );
    OSL_ENSURE( !IsInSct(), "GetNextLeaf(), don't call me for Sections." );

    const bool bBody = IsInDocBody();  // If I'm coming from the DocBody,
                                           // I want to end up in the body.

    // It doesn't make sense to insert pages, as we only want to search the
    // chain.
    if( IsInFly() )
        eMakePage = MAKEPAGE_NONE;

    // For tables, we just take the big leap. A simple GetNext would
    // iterate through the first cells and, in turn, all other cells.
    SwLayoutFrm *pLayLeaf = 0;
    if ( IsTabFrm() )
    {
        SwCntntFrm* pTmp = ((SwTabFrm*)this)->FindLastCntnt();
        if ( pTmp )
            pLayLeaf = pTmp->GetUpper();
    }
    if ( !pLayLeaf )
        pLayLeaf = GetNextLayoutLeaf();

    SwLayoutFrm *pOldLayLeaf = 0;           // Make sure that we don't have to
                                            // start searching from top when we
                                            // have a freshly created page.
    bool bNewPg = false;    // Only insert a new page once.

    while ( true )
    {
        if ( pLayLeaf )
        {
            // There's yet another LayoutFrm. Let's see if it's ready to host
            // me as well.
            // It only needs to be of the same kind like my starting point
            // (DocBody or Footnote respectively)
            if ( pLayLeaf->FindPageFrm()->IsFtnPage() )
            {   // If I ended up at the end note pages, we're done.
                pLayLeaf = 0;
                continue;
            }
            if ( (bBody && !pLayLeaf->IsInDocBody()) || pLayLeaf->IsInTab()
                 || pLayLeaf->IsInSct() )
            {
                // They don't want me! Try again
                pOldLayLeaf = pLayLeaf;
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
                continue;
            }

            // I'm wanted, therefore I'm done. However, it may still be that,
            // during a page break, the page type isn't the desired one. In that
            // case we have to insert a page of the correct type.

            if( !IsFlowFrm() && ( eMakePage == MAKEPAGE_NONE ||
                eMakePage==MAKEPAGE_APPEND || eMakePage==MAKEPAGE_NOSECTION ) )
                return pLayLeaf;

            SwPageFrm *pNew = pLayLeaf->FindPageFrm();
            const ViewShell *pSh = getRootFrm()->GetCurrShell();
            // #111704# The pagedesc check does not make sense for frames in fly frames
            if ( pNew != FindPageFrm() && !bNewPg && !IsInFly() &&
                 // #i46683#
                 // Do not consider page descriptions in browse mode (since
                 // MoveBwd ignored them)
                 !(pSh && pSh->GetViewOptions()->getBrowseMode() ) )
            {
                if( WrongPageDesc( pNew ) )
                {
                    SwFtnContFrm *pCont = pNew->FindFtnCont();
                    if( pCont )
                    {
                        // If the reference of the first footnote of this page
                        // lies before the page, we'd rather not insert a new page.
                        // (Bug #55620#)
                        SwFtnFrm *pFtn = (SwFtnFrm*)pCont->Lower();
                        if( pFtn && pFtn->GetRef() )
                        {
                            const sal_uInt16 nRefNum = pNew->GetPhyPageNum();
                            if( pFtn->GetRef()->GetPhyPageNum() < nRefNum )
                                break;
                        }
                    }
                    //Gotcha! The following page is wrong, therefore we need to
                    //insert a new one.
                    if ( eMakePage == MAKEPAGE_INSERT )
                    {
                        bNewPg = true;

                        SwPageFrm *pPg = pOldLayLeaf ?
                                    pOldLayLeaf->FindPageFrm() : 0;
                        if ( pPg && pPg->IsEmptyPage() )
                            // Don't insert behind. Insert before the EmptyPage.
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
            // There's no other matching LayoutFrm, so we have to insert
            // a new page.
            if ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT )
            {
                InsertPage(
                    pOldLayLeaf ? pOldLayLeaf->FindPageFrm() : FindPageFrm(),
                    sal_False );

                // And again from the start.
                pLayLeaf = pOldLayLeaf ? pOldLayLeaf : GetNextLayoutLeaf();
            }
            else
                break;
        }
    }
    return pLayLeaf;
}

/*************************************************************************
|*
|*  SwFrm::GetPrevLeaf()
|*
|*  Beschreibung        Returns the previous layout leaf where we can move
|*      the frame.
|*
|*************************************************************************/


SwLayoutFrm *SwFrm::GetPrevLeaf( MakePageType )
{
    OSL_ENSURE( !IsInFtn(), "GetPrevLeaf(), don't call me for Ftn." );

    const bool bBody = IsInDocBody();  // If I'm coming from the DocBody,
                                           // I want to end up in the body.
    const sal_Bool bFly  = IsInFly();

    SwLayoutFrm *pLayLeaf = GetPrevLayoutLeaf();
    SwLayoutFrm *pPrevLeaf = 0;

    while ( pLayLeaf )
    {
        if ( pLayLeaf->IsInTab() ||     // Never go into tables.
             pLayLeaf->IsInSct() )      // Same goes for sections!
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
            break;  //Cntnts in Flys should accept any layout leaf.
        else
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
    }
    return pLayLeaf ? pLayLeaf : pPrevLeaf;
}

/*************************************************************************
|*
|*  SwFlowFrm::IsPrevObjMove()
|*
|*************************************************************************/


sal_Bool SwFlowFrm::IsPrevObjMove() const
{
    //sal_True The FlowFrm must respect the a border of the predecessor, also needs
    //         to insert a break if required.

    //!!!!!!!!!!!Hack!!!!!!!!!!!
    const ViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
        return sal_False;

    SwFrm *pPre = m_rThis.FindPrev();

    if ( pPre && pPre->GetDrawObjs() )
    {
        OSL_ENSURE( SwFlowFrm::CastFlowFrm( pPre ), "new flowfrm?" );
        if( SwFlowFrm::CastFlowFrm( pPre )->IsAnFollow( this ) )
            return sal_False;
        SwLayoutFrm* pPreUp = pPre->GetUpper();
        // If the upper is a SectionFrm, or a column of a SectionFrm, we're
        // allowed to protrude out of it.  However, we need to respect the
        // Upper of the SectionFrm.
        if( pPreUp->IsInSct() )
        {
            if( pPreUp->IsSctFrm() )
                pPreUp = pPreUp->GetUpper();
            else if( pPreUp->IsColBodyFrm() &&
                     pPreUp->GetUpper()->GetUpper()->IsSctFrm() )
                pPreUp = pPreUp->GetUpper()->GetUpper()->GetUpper();
        }
        // #i26945# - re-factoring
        // use <GetVertPosOrientFrm()> to determine, if object has followed the
        // text flow to the next layout frame
        for ( sal_uInt16 i = 0; i < pPre->GetDrawObjs()->Count(); ++i )
        {
            // #i28701# - consider changed type of
            // <SwSortedObjs> entries.
            const SwAnchoredObject* pObj = (*pPre->GetDrawObjs())[i];
            // OD 2004-01-20 #110582# - do not consider hidden objects
            // #i26945# - do not consider object, which
            // doesn't follow the text flow.
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

/*************************************************************************
|*
|*  sal_Bool SwFlowFrm::IsPageBreak()
|*
|*  Description        If there's a hard page break before the Frm AND there's a
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
|*
|*************************************************************************/


sal_Bool SwFlowFrm::IsPageBreak( sal_Bool bAct ) const
{
    if ( !IsFollow() && m_rThis.IsInDocBody() &&
         ( !m_rThis.IsInTab() || ( m_rThis.IsTabFrm() && !m_rThis.GetUpper()->IsInTab() ) ) ) // i66968
    {
        const ViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();
        if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            return sal_False;
        const SwAttrSet *pSet = m_rThis.GetAttrSet();

        // Determine predecessor
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

/*************************************************************************
|*
|*  sal_Bool SwFlowFrm::IsColBreak()
|*
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
|*
|*************************************************************************/

sal_Bool SwFlowFrm::IsColBreak( sal_Bool bAct ) const
{
    if ( !IsFollow() && (m_rThis.IsMoveable() || bAct) )
    {
        const SwFrm *pCol = m_rThis.FindColFrm();
        if ( pCol )
        {
            // Determine predecessor
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

    @author OD
*/
const SwFrm* SwFlowFrm::_GetPrevFrmForUpperSpaceCalc( const SwFrm* _pProposedPrevFrm ) const
{
    const SwFrm* pPrevFrm = _pProposedPrevFrm
                            ? _pProposedPrevFrm
                            : m_rThis.GetPrev();

    // Skip hidden paragraphs and empty sections
    while ( pPrevFrm &&
            ( ( pPrevFrm->IsTxtFrm() &&
                static_cast<const SwTxtFrm*>(pPrevFrm)->IsHiddenNow() ) ||
              ( pPrevFrm->IsSctFrm() &&
                !static_cast<const SwSectionFrm*>(pPrevFrm)->GetSection() ) ) )
    {
        pPrevFrm = pPrevFrm->GetPrev();
    }

    // Special case: no direct previous frame is found but frame is in footnote
    // Search for a previous frame in previous footnote,
    // if frame isn't in a section, which is also in the footnote
    if ( !pPrevFrm && m_rThis.IsInFtn() &&
         ( m_rThis.IsSctFrm() ||
           !m_rThis.IsInSct() || !m_rThis.FindSctFrm()->IsInFtn() ) )
    {
        const SwFtnFrm* pPrevFtnFrm =
                static_cast<const SwFtnFrm*>(m_rThis.FindFtnFrm()->GetPrev());
        if ( pPrevFtnFrm )
        {
            pPrevFrm = pPrevFtnFrm->GetLastLower();

            // Skip hidden paragraphs and empty sections
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
    // Special case: found previous frame is a section
    // Search for the last content in the section
    if( pPrevFrm && pPrevFrm->IsSctFrm() )
    {
        const SwSectionFrm* pPrevSectFrm =
                                    static_cast<const SwSectionFrm*>(pPrevFrm);
        pPrevFrm = pPrevSectFrm->FindLastCntnt();
        // If the last content is in a table _inside_ the section,
        // take the table herself.
        // OD 2004-02-18 #106629# - correction:
        // Check directly, if table is inside table, instead of indirectly
        // by checking, if section isn't inside a table
        if ( pPrevFrm && pPrevFrm->IsInTab() )
        {
            const SwTabFrm* pTableFrm = pPrevFrm->FindTabFrm();
            if ( pPrevSectFrm->IsAnLower( pTableFrm ) )
            {
                pPrevFrm = pTableFrm;
            }
        }
        // OD 2004-02-18 #106629# correction: skip hidden text frames
        while ( pPrevFrm &&
                pPrevFrm->IsTxtFrm() &&
                static_cast<const SwTxtFrm*>(pPrevFrm)->IsHiddenNow() )
        {
            pPrevFrm = pPrevFrm->GetPrev();
        }
    }

    return pPrevFrm;
}

/// Compare styles attached to these text frames.
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

// OD 2004-03-12 #i11860# - add 3rd parameter <_bConsiderGrid>
SwTwips SwFlowFrm::CalcUpperSpace( const SwBorderAttrs *pAttrs,
                                   const SwFrm* pPr,
                                   const bool _bConsiderGrid ) const
{
    // OD 2004-03-10 #i11860# - use new method <GetPrevFrmForUpperSpaceCalc(..)>
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
    // OD 06.01.2004 #i11859#
    {
        const IDocumentSettingAccess* pIDSA = m_rThis.GetUpper()->GetFmt()->getIDocumentSettingAccess();
        const bool bUseFormerLineSpacing = pIDSA->get(IDocumentSettingAccess::OLD_LINE_SPACING);
        if( pPrevFrm )
        {
            // OD 2004-03-10 #i11860# - use new method to determine needed spacing
            // values of found previous frame and use these values.
            SwTwips nPrevLowerSpace = 0;
            SwTwips nPrevLineSpacing = 0;
            // #i102458#
            bool bPrevLineSpacingPorportional = false;
            GetSpacingValuesOfFrm( (*pPrevFrm),
                                   nPrevLowerSpace, nPrevLineSpacing,
                                   bPrevLineSpacingPorportional );
            if( pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX) )
            {
                nUpper = nPrevLowerSpace + pAttrs->GetULSpace().GetUpper();
                SwTwips nAdd = nPrevLineSpacing;
                // OD 07.01.2004 #i11859# - consideration of the line spacing
                //      for the upper spacing of a text frame
                if ( bUseFormerLineSpacing )
                {
                    // former consideration
                    if ( pOwn->IsTxtFrm() )
                    {
                        nAdd = std::max( nAdd, static_cast<SwTxtFrm&>(m_rThis).GetLineSpace() );
                    }
                    nUpper += nAdd;
                }
                else
                {
                    // new consideration:
                    //      Only the proportional line spacing of the previous
                    //      text frame is considered for the upper spacing and
                    //      the line spacing values are add up instead of
                    //      building its maximum.
                    if ( pOwn->IsTxtFrm() )
                    {
                        // #i102458#
                        // Correction:
                        // A proportional line spacing of the previous text frame
                        // is added up to a own leading line spacing.
                        // Otherwise, the maximum of the leading line spacing
                        // of the previous text frame and the own leading line
                        // spacing is built.
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
                // OD 07.01.2004 #i11859# - consideration of the line spacing
                //      for the upper spacing of a text frame
                if ( bUseFormerLineSpacing )
                {
                    // former consideration
                    if ( pOwn->IsTxtFrm() )
                        nUpper = std::max( nUpper, ((SwTxtFrm*)pOwn)->GetLineSpace() );
                    if ( nPrevLineSpacing != 0 )
                    {
                        nUpper = std::max( nUpper, nPrevLineSpacing );
                    }
                }
                else
                {
                    // new consideration:
                    //      Only the proportional line spacing of the previous
                    //      text frame is considered for the upper spacing and
                    //      the line spacing values are add up and added to
                    //      the paragraph spacing instead of building the
                    //      maximum of the line spacings and the paragraph spacing.
                    SwTwips nAdd = nPrevLineSpacing;
                    if ( pOwn->IsTxtFrm() )
                    {
                        // #i102458#
                        // Correction:
                        // A proportional line spacing of the previous text frame
                        // is added up to a own leading line spacing.
                        // Otherwise, the maximum of the leading line spacing
                        // of the previous text frame and the own leading line
                        // spacing is built.
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

    // OD 2004-02-26 #i25029# - pass previous frame <pPrevFrm>
    // to method <GetTopLine(..)>, if parameter <pPr> is set.
    // Note: parameter <pPr> is set, if method is called from <SwTxtFrm::WouldFit(..)>
    nUpper += pAttrs->GetTopLine( m_rThis, (pPr ? pPrevFrm : 0L) );

    // OD 2004-03-12 #i11860# - consider value of new parameter <_bConsiderGrid>
    // and use new method <GetUpperSpaceAmountConsideredForPageGrid(..)>

    //consider grid in square page mode
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

    @author OD
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

    @author OD
*/
SwTwips SwFlowFrm::_GetUpperSpaceAmountConsideredForPrevFrm() const
{
    SwTwips nUpperSpaceAmountOfPrevFrm = 0;

    const SwFrm* pPrevFrm = _GetPrevFrmForUpperSpaceCalc();
    if ( pPrevFrm )
    {
        SwTwips nPrevLowerSpace = 0;
        SwTwips nPrevLineSpacing = 0;
        // #i102458#
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

    @author OD
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

    @author OD
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

    // #i26250#
    // - correct consideration of table frames
    // - use new method <CalcAddLowerSpaceAsLastInTableCell(..)>
    if ( ( ( m_rThis.IsTabFrm() && m_rThis.GetUpper()->IsInTab() ) ||
           // #115759# - no lower spacing, if frame has a follow
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

    @author OD
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

/*************************************************************************
|*
|*  sal_Bool SwFlowFrm::CheckMoveFwd()
|*
|*  Description        Moves the Frm forward if it seems necessary regarding
|*      the current conditions and attributes.
|*
|*************************************************************************/


sal_Bool SwFlowFrm::CheckMoveFwd( bool& rbMakePage, sal_Bool bKeep, sal_Bool )
{
    const SwFrm* pNxt = m_rThis.GetIndNext();

    if ( bKeep && //!bMovedBwd &&
         ( !pNxt || ( pNxt->IsTxtFrm() && ((SwTxtFrm*)pNxt)->IsEmptyMaster() ) ) &&
         ( 0 != (pNxt = m_rThis.FindNext()) ) && IsKeepFwdMoveAllowed() )
    {
        if( pNxt->IsSctFrm() )
        {   // Don't get fooled by empty SectionFrms
            const SwFrm* pTmp = NULL;
            while( pNxt && pNxt->IsSctFrm() &&
                   ( !((SwSectionFrm*)pNxt)->GetSection() ||
                     0 == ( pTmp = ((SwSectionFrm*)pNxt)->ContainsAny() ) ) )
            {
                pNxt = pNxt->FindNext();
                pTmp = NULL;
            }
            if( pTmp )
                pNxt = pTmp; // the content of the next notempty sectionfrm
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
                //Keep together with the following frame
                MoveFwd( rbMakePage, sal_False );
                return sal_True;
            }
        }
    }

    sal_Bool bMovedFwd = sal_False;

    if ( m_rThis.GetIndPrev() )
    {
        if ( IsPrevObjMove() ) // Should we care about objects of the Prev?
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

/*************************************************************************
|*
|*  sal_Bool SwFlowFrm::MoveFwd()
|*
|*  Description        Return value tells us whether the Frm has changed
|*      the page.
|*
|*************************************************************************/


sal_Bool SwFlowFrm::MoveFwd( sal_Bool bMakePage, sal_Bool bPageBreak, sal_Bool bMoveAlways )
{
//!!!!MoveFtnCntFwd might need to be updated as well.
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

        // Allow the MoveFwd even if we do not have an IndPrev in these cases:
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
            // It's allowed to move PageBreaks if the Frm isn't the first
            // one on the page.
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
        // We move ourself and all the direct successors before the
        // first CntntFrm below the new Upper.

        // If our NewUpper lies in a SectionFrm, we need to make sure
        // that it won't destroy itself in Calc.
        SwSectionFrm* pSect = pNewUpper->FindSctFrm();
        if( pSect )
        {
            // If we only switch column within our SectionFrm, we better don't
            // call Calc, as this would format the SectionFrm, which in turn would
            // call us again, etc.
            if( pSect != m_rThis.FindSctFrm() )
            {
                bool bUnlock = !pSect->IsColLocked();
                pSect->ColLock();
                pNewUpper->Calc();
                if( bUnlock )
                    pSect->ColUnlock();
            }
        }
        // Do not calculate split cell frames.
        else if ( !pNewUpper->IsCellFrm() || ((SwLayoutFrm*)pNewUpper)->Lower() )
            pNewUpper->Calc();

        SwFtnBossFrm *pNewBoss = pNewUpper->FindFtnBossFrm();
        bool bBossChg = pNewBoss != pOldBoss;
        pNewBoss = pNewBoss->FindFtnBossFrm( sal_True );
        pOldBoss = pOldBoss->FindFtnBossFrm( sal_True );
        SwPageFrm* pNewPage = pOldPage;

        // First, we move the footnotes.
        sal_Bool bFtnMoved = sal_False;

        // #i26831#
        // If pSect has just been created, the printing area of pSect has
        // been calculated based on the first content of its follow.
        // In this case we prefer to call a SimpleFormat for this new
        // section after we inserted the contents. Otherwise the section
        // frame will invalidate its lowers, if its printing area changes
        // in SwSectionFrm::Format, which can cause loops.
        const bool bForceSimpleFormat = pSect && pSect->HasFollow() &&
                                       !pSect->ContainsAny();

        if ( pNewBoss != pOldBoss )
        {
            pNewPage = pNewBoss->FindPageFrm();
            bSamePage = pNewPage == pOldPage;
            // Set deadline, so the footnotes don't think up
            // silly things...
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
        // It's possible when dealing with SectionFrms that we have been moved
        // by pNewUpper->Calc(), for instance into the pNewUpper.
        // MoveSubTree or PasteTree respectively is not prepared to handle such a
        // situation.
        if( pNewUpper != m_rThis.GetUpper() )
        {
            // #i27145#
            SwSectionFrm* pOldSct = 0;
            if ( m_rThis.GetUpper()->IsSctFrm() )
            {
                pOldSct = static_cast<SwSectionFrm*>(m_rThis.GetUpper());
            }

            MoveSubTree( pNewUpper, pNewUpper->Lower() );

            // #i27145#
            if ( pOldSct && pOldSct->GetSection() )
            {
                // Prevent loops by setting the new height at
                // the section frame if footnotes have been moved.
                // Otherwise the call of SwLayNotify::~SwLayNotify() for
                // the (invalid) section frame will invalidate the first
                // lower of its follow, because it grows due to the removed
                // footnotes.
                // Note: If pOldSct has become empty during MoveSubTree, it
                // has already been scheduled for removal. No SimpleFormat
                // for these.
                pOldSct->SimpleFormat();
            }

            // #i26831#
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
                    ViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();
                    if ( pSh && !pSh->Imp()->IsUpdateExpFlds() )
                        pSh->GetDoc()->SetNewFldLst(true);  // Will be done by CalcLayout() later on!

                    pNewPage->InvalidateSpelling();
                    pNewPage->InvalidateSmartTags();    // SMARTTAGS
                    pNewPage->InvalidateAutoCompleteWords();
                    pNewPage->InvalidateWordCount();
                }
            }
        }
        // OD 30.10.2002 #97265# - no <CheckPageDesc(..)> in online layout
        const ViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();

        if ( !( pSh && pSh->GetViewOptions()->getBrowseMode() ) )
        {
            // #i106452#
            // check page description not only in situation with sections.
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


/*************************************************************************
|*
|*  sal_Bool SwFlowFrm::MoveBwd()
|*
|*  Description        Return value tells whether the Frm should change the page.
|*                     This should be called by derived classes.
|*                     The actual moving must be implemented in the subclasses.
|*
|*************************************************************************/

sal_Bool SwFlowFrm::MoveBwd( sal_Bool &rbReformat )
{
    SwFlowFrm::SetMoveBwdJump( sal_False );

    SwFtnFrm* pFtn = m_rThis.FindFtnFrm();
    if ( pFtn && pFtn->IsBackMoveLocked() )
        return sal_False;

    // #115759# - text frames, which are directly inside
    // tables aren't allowed to move backward.
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
        // If the footnote already sits on the same page/column as the reference,
        // we can't flow back. The breaks don't need to be checked for footnotes.

        // #i37084# FindLastCntnt does not necessarily
        // have to have a result != 0
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
             // OD 08.11.2002 #104840# - use <SwLayoutFrm::IsBefore(..)>
             ( !bEndnote ||
               pRefBoss->IsBefore( pOldBoss ) )
           )
            pNewUpper = m_rThis.GetLeaf( MAKEPAGE_FTN, sal_False );
    }
    else if ( IsPageBreak( sal_True ) ) // Do we have to respect a PageBreak?
    {
        // If the previous page doesn't have an Frm in the body,
        // flowing back makes sense despite the PageBreak (otherwise,
        // we'd get an empty page).
        // Of course we need to overlook empty pages!
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
                    // #i53139#
                    // Now <pNewUpper> is a previous layout frame, which contains
                    // content. But the new upper layout frame has to be the next one.
                    // Thus, hack for issue i14206 no longer needed, but fix for issue 114442
                    // #136024# - correct fix for i53139
                    // Check for wrong page description before using next new upper.
                    // #i66051# - further correction of fix for i53139
                    // Check for correct type of new next upper layout frame
                    // #136538# - another correction of fix for i53139
                    // Assumption, that in all cases <pNewUpper> is a previous
                    // layout frame, which contains content, is wrong.
                    // #136538# - another correction of fix for i53139
                    // Beside type check, check also, if proposed new next upper
                    // frame is inside the same frame types.
                    // #i73194# - and yet another correction
                    // of fix for i53139:
                    // Assure that the new next upper layout frame doesn't
                    // equal the current one.
                    // E.g.: content is on page 3, on page 2 is only a 'ghost'
                    // section and on page 1 is normal content. Method <FindPrev(..)>
                    // will find the last content of page 1, but <GetLeaf(..)>
                    // returns new upper on page 2.
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
        // If the previous column doesn't contain a CntntFrm, flowing back
        // makes sense despite the ColumnBreak, as otherwise we'd get
        // an empty column.
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
            // #i53139#
            // #i69409# - check <pNewUpper>
            // #i71065# - check <SwFlowFrm::IsMoveBwdJump()>
            else if ( pNewUpper && !SwFlowFrm::IsMoveBwdJump() )
            {
                // Now <pNewUpper> is a previous layout frame, which
                // contains content. But the new upper layout frame
                // has to be the next one.
                // #136024# - correct fix for i53139
                // Check for wrong page description before using next new upper.
                // #i66051# - further correction of fix for i53139
                // Check for correct type of new next upper layout frame
                // #136538# - another correction of fix for i53139
                // Beside type check, check also, if proposed new next upper
                // frame is inside the same frame types.
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
                    // ColumnFrms now with BodyFrm
                    SwLayoutFrm* pColBody = pCol->IsColumnFrm() ?
                        (SwLayoutFrm*)((SwLayoutFrm*)pCol)->Lower() :
                        (SwLayoutFrm*)pCol;
                    if ( pColBody->ContainsCntnt() )
                    {
                        bGoOn = false; // We have content here! we accept this
                        // only if GetLeaf() has set the MoveBwdJump.
                        if( SwFlowFrm::IsMoveBwdJump() )
                        {
                            pNewUpper = pColBody;
                            // #i53139#
                            // Now <pNewUpper> is a previous layout frame, which
                            // contains content. But the new upper layout frame
                            // has to be the next one.
                            // #136024# - correct fix for i53139
                            // Check for wrong page description before using next new upper.
                            // #i66051# - further correction of fix for i53139
                            // Check for correct type of new next upper layout frame
                            // #136538# - another correction of fix for i53139
                            // Beside type check, check also, if proposed new next upper
                            // frame is inside the same frame types.
                            // #i71065#
                            // Check that the proposed new next upper layout
                            // frame isn't the current one.
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
                        if( pNewUpper )        // We already had an empty column, in other
                            bJump = true;      // words we skipped one.
                        pNewUpper = pColBody;  // this empty column could be considered,
                                               // but we continue searching nevertheless.
                    }
                }
            } while( bGoOn );
            if( bJump )
                SwFlowFrm::SetMoveBwdJump( sal_True );
        }
    }
    else // No breaks - we can flow back.
        pNewUpper = m_rThis.GetLeaf( MAKEPAGE_NONE, sal_False );

    // #i27801# - no move backward of 'master' text frame,
    // if - due to its object positioning - it isn't allowed to be on the new page frame
    // #i44049# - add another condition for not moving backward
    // If one of its objects has restarted the layout process, moving backward
    // isn't sensible either.
    // #i47697# - refine condition made for issue i44049
    // - allow move backward as long as the anchored object is only temporarily
    //   positions considering its wrapping style.
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
        // #i44049# - check, if one of its anchored objects
        // has restarted the layout process.
        else if ( m_rThis.GetDrawObjs() )
        {
            sal_uInt32 i = 0;
            for ( ; i < m_rThis.GetDrawObjs()->Count(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*m_rThis.GetDrawObjs())[i];
                // #i47697# - refine condition - see above
                if ( pAnchoredObj->RestartLayoutProcess() &&
                     !pAnchoredObj->IsTmpConsiderWrapInfluence() )
                {
                    pNewUpper = 0;
                    break;
                }
            }
        }
    }

    // With Follows, it's only allowed to flow back if there's no neighbor
    // in the new environment (because that would be the Master).
    // (6677) If however we skipped empty pages, we still have to move.
    if ( pNewUpper && IsFollow() && pNewUpper->Lower() )
    {
        // #i79774#
        // neglect empty sections in proposed new upper frame
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
                // Don't move after the Master, but into the next empty page.
                SwFrm *pFrm = pNewUpper->Lower();
                while ( pFrm->GetNext() )
                    pFrm = pFrm->GetNext();
                pNewUpper = pFrm->GetLeaf( MAKEPAGE_INSERT, sal_True );
                if( pNewUpper == m_rThis.GetUpper() ) // Did we end up in the same place?
                    pNewUpper = NULL;               // If so, moving is not needed.
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
                // #126020# - adjust check for empty section
                // #130797# - correct fix #126020#
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

    // OD 2004-05-26 #i21478# - don't move backward, if flow frame wants to
    // keep with next frame and next frame is locked.
    // #i38232# - If next frame is a table, do *not* check,
    // if it's locked.
    if ( pNewUpper && !IsFollow() &&
         m_rThis.GetAttrSet()->GetKeep().GetValue() && m_rThis.GetIndNext() )
    {
        SwFrm* pIndNext = m_rThis.GetIndNext();
        // #i38232#
        if ( !pIndNext->IsTabFrm() )
        {
            // get first content of section, while empty sections are skipped
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

    // #i65250#
    // layout loop control for flowing content again and again moving
    // backward under the same layout condition.
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
        // #i73194# - make code robust
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
            //Kann sein, dass ich einen Container bekam.
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
            // If we're in a section of a footnote, we may need to create
            // a SwSectionFrm in the new upper
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
        // Lock section. Otherwise, it could get destroyed if the only Cntnt
        // moves e.g. from the second into the first column.
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
        // optimization: format section, if its size is invalidated and if it's
        // the new parent of moved backward frame.
        bool bFormatSect( false );
        if( bUnlock )
        {
            pSect->ColUnlock();
            if( pSect->HasFollow() != bFollow )
            {
                pSect->InvalidateSize();
                // - optimization
                if ( pSect == pNewUpper )
                    bFormatSect = true;
            }
        }

        m_rThis.Paste( pNewUpper );
        // - optimization
        if ( bFormatSect )
            pSect->Calc();

        SwPageFrm *pNewPage = m_rThis.FindPageFrm();
        if( pNewPage != pOldPage )
        {
            m_rThis.Prepare( PREP_BOSS_CHGD, (const void*)pOldPage, sal_False );
            ViewShell *pSh = m_rThis.getRootFrm()->GetCurrShell();
            if ( pSh && !pSh->Imp()->IsUpdateExpFlds() )
                pSh->GetDoc()->SetNewFldLst(true);  // Will be done by CalcLayout() later on

            pNewPage->InvalidateSpelling();
            pNewPage->InvalidateSmartTags();    // SMARTTAGS
            pNewPage->InvalidateAutoCompleteWords();
            pNewPage->InvalidateWordCount();

            // OD 30.10.2002 #97265# - no <CheckPageDesc(..)> in online layout
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
                    // First page could get empty for example by disabling
                    // a section
                    SwFrm::CheckPageDescs( (SwPageFrm*)pNewPage, sal_False);
                }
            }
        }
    }
    return pNewUpper != 0;
}

/*************************************************************************
|*
|*  SwFlowFrm::CastFlowFrm
|*
|*************************************************************************/

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
