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

#include <sal/config.h>

#include <bodyfrm.hxx>
#include "swtable.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "viewimp.hxx"
#include "viewopt.hxx"
#include "frmtool.hxx"
#include <IDocumentFieldsAccess.hxx>
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
#include <calbck.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <o3tl/make_unique.hxx>

bool SwFlowFrame::m_bMoveBwdJump = false;

SwFlowFrame::SwFlowFrame( SwFrame &rFrame ) :
    m_rThis( rFrame ),
    m_pFollow( nullptr ),
    m_pPrecede( nullptr ),
    m_bLockJoin( false ),
    m_bUndersized( false ),
    m_bFlyLock( false )
{}

SwFlowFrame::~SwFlowFrame()
{
    if (m_pFollow)
    {
        m_pFollow->m_pPrecede = nullptr;
    }
    if (m_pPrecede)
    {
        m_pPrecede->m_pFollow = nullptr;
    }
}

void SwFlowFrame::SetFollow(SwFlowFrame *const pFollow)
{
    if (m_pFollow)
    {
        assert(this == m_pFollow->m_pPrecede);
        m_pFollow->m_pPrecede = nullptr;
    }
    m_pFollow = pFollow;
    if (m_pFollow != nullptr)
    {
        if (m_pFollow->m_pPrecede) // re-chaining pFollow?
        {
            assert(m_pFollow == m_pFollow->m_pPrecede->m_pFollow);
            m_pFollow->m_pPrecede->m_pFollow = nullptr;
        }
        m_pFollow->m_pPrecede = this;
    }
}

/// @return true if any follow has the JoinLocked flag
bool SwFlowFrame::HasLockedFollow() const
{
    const SwFlowFrame* pFrame = GetFollow();
    while( pFrame )
    {
        if( pFrame->IsJoinLocked() )
            return true;
        pFrame = pFrame->GetFollow();
    }
    return false;
}

bool SwFlowFrame::IsKeepFwdMoveAllowed( bool bIgnoreMyOwnKeepValue )
{
    // If all the predecessors up to the first of the chain have
    // the 'keep' attribute set, and the first of the chain's
    // IsFwdMoveAllowed returns false, then we're not allowed to move.
    SwFrame *pFrame = &m_rThis;
    if ( !pFrame->IsInFootnote() ) {
        if ( bIgnoreMyOwnKeepValue && pFrame->GetIndPrev() )
            pFrame = pFrame->GetIndPrev();
        do
        {   if ( pFrame->GetAttrSet()->GetKeep().GetValue() )
                pFrame = pFrame->GetIndPrev();
            else
                return true;
        } while ( pFrame );
    }
                  //See IsFwdMoveAllowed()
    bool bRet = false;
    if ( pFrame && pFrame->GetIndPrev() )
        bRet = true;
    return bRet;
}

void SwFlowFrame::CheckKeep()
{
    // Kick off the "last" predecessor with a 'keep' attribute, because
    // it's possible for the whole troop to move back.
    SwFrame *pPre = m_rThis.GetIndPrev();
    if( pPre->IsSctFrame() )
    {
        SwFrame *pLast = static_cast<SwSectionFrame*>(pPre)->FindLastContent();
        if( pLast && pLast->FindSctFrame() == pPre )
            pPre = pLast;
        else
            return;
    }
    SwFrame* pTmp;
    bool bKeep;
    while ( (bKeep = pPre->GetAttrSet()->GetKeep().GetValue()) &&
            nullptr != ( pTmp = pPre->GetIndPrev() ) )
    {
        if( pTmp->IsSctFrame() )
        {
            SwFrame *pLast = static_cast<SwSectionFrame*>(pTmp)->FindLastContent();
            if( pLast && pLast->FindSctFrame() == pTmp )
                pTmp = pLast;
            else
                break;
        }
        pPre = pTmp;
    }
    if ( bKeep )
        pPre->InvalidatePos();
}

bool SwFlowFrame::IsKeep( const SwAttrSet& rAttrs, bool bCheckIfLastRowShouldKeep ) const
{
    // 1. The keep attribute is ignored inside footnotes
    // 2. For compatibility reasons, the keep attribute is
    //    ignored for frames inside table cells
    // 3. If bBreakCheck is set to true, this function only checks
    //    if there are any break after attributes set at rAttrs
    //    or break before attributes set for the next content (or next table)
    bool bKeep = bCheckIfLastRowShouldKeep ||
                 (  !m_rThis.IsInFootnote() &&
                    ( !m_rThis.IsInTab() || m_rThis.IsTabFrame() ) &&
                    rAttrs.GetKeep().GetValue() );

    OSL_ENSURE( !bCheckIfLastRowShouldKeep || m_rThis.IsTabFrame(),
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
                bKeep = false;
                break;
            }
            default: break;
        }
        if ( bKeep )
        {
            SwFrame *pNxt;
            if( nullptr != (pNxt = m_rThis.FindNextCnt()) &&
                (!m_pFollow || pNxt != &m_pFollow->GetFrame()))
            {
                // The last row of a table only keeps with the next content
                // it they are in the same section:
                if ( bCheckIfLastRowShouldKeep )
                {
                    const SwSection* pThisSection = nullptr;
                    const SwSection* pNextSection = nullptr;
                    const SwSectionFrame* pThisSectionFrame = m_rThis.FindSctFrame();
                    const SwSectionFrame* pNextSectionFrame = pNxt->FindSctFrame();

                    if ( pThisSectionFrame )
                        pThisSection = pThisSectionFrame->GetSection();

                    if ( pNextSectionFrame )
                        pNextSection = pNextSectionFrame->GetSection();

                    if ( pThisSection != pNextSection )
                        bKeep = false;
                }

                if ( bKeep )
                {
                    const SwAttrSet* pSet = nullptr;

                    SwTabFrame* pTab = pNxt->IsInTab() ? pNxt->FindTabFrame() : nullptr;
                    if (pTab)
                    {
                        if ( ! m_rThis.IsInTab() || m_rThis.FindTabFrame() != pTab )
                            pSet = &pTab->GetFormat()->GetAttrSet();
                    }

                    if ( ! pSet )
                        pSet = pNxt->GetAttrSet();

                    assert(pSet && "No AttrSet to check keep attribute");

                    if ( pSet->GetPageDesc().GetPageDesc() )
                        bKeep = false;
                    else switch ( pSet->GetBreak().GetBreak() )
                    {
                        case SVX_BREAK_COLUMN_BEFORE:
                        case SVX_BREAK_COLUMN_BOTH:
                        case SVX_BREAK_PAGE_BEFORE:
                        case SVX_BREAK_PAGE_BOTH:
                            bKeep = false;
                            break;
                        default: break;
                    }
                }
            }
        }
    }
    return bKeep;
}

sal_uInt8 SwFlowFrame::BwdMoveNecessary( const SwPageFrame *pPage, const SwRect &rRect )
{
    // The return value helps deciding whether we need to flow back (3),
    // or whether we can use the good old WouldFit (0, 1), or if
    // it's reasonable to relocate and test-format (2).

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
    SwFlowFrame *pTmp = this;
    do
    {   // If there are objects hanging either on me or on a follow, we can't
        // do a test formatting, because paragraph bound objects wouldn't
        // be properly considered, and character bound objects shouldn't
        // be test formatted at all.
        if( pTmp->GetFrame().GetDrawObjs() )
            nRet = 1;
        pTmp = pTmp->GetFollow();
    } while ( !nRet && pTmp );
    const SwSortedObjs *pObjs = pPage ? pPage->GetSortedObjs() : nullptr;
    if (pObjs)
    {
        // #i28701# - new type <SwSortedObjs>
        const SwSortedObjs &rObjs = *pObjs;
        sal_uLong nIndex = ULONG_MAX;
        for ( size_t i = 0; nRet < 3 && i < rObjs.size(); ++i )
        {
            // #i28701# - consider changed type of
            // <SwSortedObjs> entries.
            SwAnchoredObject* pObj = rObjs[i];
            const SwFrameFormat& rFormat = pObj->GetFrameFormat();
            const SwRect aRect( pObj->GetObjRect() );
            if ( aRect.IsOver( rRect ) &&
                 rFormat.GetSurround().GetSurround() != SURROUND_THROUGHT )
            {
                if( m_rThis.IsLayoutFrame() && //Fly Lower of This?
                    Is_Lower_Of( &m_rThis, pObj->GetDrawObj() ) )
                    continue;
                if( dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr )
                {
                    const SwFlyFrame *pFly = static_cast<const SwFlyFrame*>(pObj);
                    if ( pFly->IsAnLower( &m_rThis ) )//This Lower of Fly?
                        continue;
                }

                const SwFrame* pAnchor = pObj->GetAnchorFrame();
                if ( pAnchor == &m_rThis )
                {
                    nRet |= 1;
                    continue;
                }

                // Don't do this if the object is anchored behind me in the text
                // flow, because then I wouldn't evade it.
                if ( ::IsFrameInSameKontext( pAnchor, &m_rThis ) )
                {
                    if ( rFormat.GetAnchor().GetAnchorId() == FLY_AT_PARA )
                    {
                        // The index of the other one can be retrieved using the anchor attribute.
                        sal_uLong nTmpIndex = rFormat.GetAnchor().GetContentAnchor()->nNode.GetIndex();
                        // Now we're going to check whether the current paragraph before
                        // the anchor of the displacing object sits in the text. If this
                        // is the case, we don't try to evade it.
                        // The index is being determined via SwFormatAnchor, because it's
                        // getting quite expensive otherwise.
                        if( ULONG_MAX == nIndex )
                        {
                            const SwNode *pNode;
                            if ( m_rThis.IsContentFrame() )
                                pNode = static_cast<SwContentFrame&>(m_rThis).GetNode();
                            else if( m_rThis.IsSctFrame() )
                                pNode = static_cast<SwSectionFormat*>(static_cast<SwSectionFrame&>(m_rThis).
                                        GetFormat())->GetSectionNode();
                            else
                            {
                                OSL_ENSURE( m_rThis.IsTabFrame(), "new FowFrame?" );
                                pNode = static_cast<SwTabFrame&>(m_rThis).GetTable()->
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

/// A specialized form of Cut(), which relocates a whole chain (this and the following,
/// in particular). During this process, only the minimum operations and notifications are done.
SwLayoutFrame *SwFlowFrame::CutTree( SwFrame *pStart )
{
    // Cut the Start and all the neighbours; they are chained together and
    // a handle to the first one is returned. Residuals are invalidated
    // as appropriate.

    SwLayoutFrame *pLay = pStart->GetUpper();
    if ( pLay->IsInFootnote() )
        pLay = pLay->FindFootnoteFrame();

    // #i58846#
    // <pPrepare( PREP_QUOVADIS )> only for frames in footnotes
    if( pStart->IsInFootnote() )
    {
        SwFrame* pTmp = pStart->GetIndPrev();
        if( pTmp )
            pTmp->Prepare( PREP_QUOVADIS );
    }

    // Just cut quickly and take care that we don't cause problems with the
    // left-behinds. The pointers of the chain being cut can point who-knows where.
    if ( pStart == pStart->GetUpper()->Lower() )
        pStart->GetUpper()->m_pLower = nullptr;
    if ( pStart->GetPrev() )
    {
        pStart->GetPrev()->mpNext = nullptr;
        pStart->mpPrev = nullptr;
    }

    if ( pLay->IsFootnoteFrame() )
    {
        if ( !pLay->Lower() && !pLay->IsColLocked() &&
             !static_cast<SwFootnoteFrame*>(pLay)->IsBackMoveLocked() )
        {
            pLay->Cut();
            SwFrame::DestroyFrame(pLay);
        }
        else
        {
            bool bUnlock = !static_cast<SwFootnoteFrame*>(pLay)->IsBackMoveLocked();
            static_cast<SwFootnoteFrame*>(pLay)->LockBackMove();
            pLay->InvalidateSize();
            pLay->Calc(pLay->getRootFrame()->GetCurrShell()->GetOut());
            SwContentFrame *pCnt = pLay->ContainsContent();
            while ( pCnt && pLay->IsAnLower( pCnt ) )
            {
                // It's possible for the ContentFrame to be locked, and we don't want
                // to end up in an endless page migration, so we're not even
                // going to call Calc!
                OSL_ENSURE( pCnt->IsTextFrame(), "The Graphic has landed." );
                if ( static_cast<SwTextFrame*>(pCnt)->IsLocked() ||
                     static_cast<SwTextFrame*>(pCnt)->GetFollow() == pStart )
                    break;
                pCnt->Calc(pCnt->getRootFrame()->GetCurrShell()->GetOut());
                pCnt = pCnt->GetNextContentFrame();
            }
            if( bUnlock )
                static_cast<SwFootnoteFrame*>(pLay)->UnlockBackMove();
        }
        pLay = nullptr;
    }
    return pLay;
}

/// A specialized form of Paste(), which relocates a whole chain (this and the following,
/// in particular). During this process, only the minimum operations and notifications are done.
bool SwFlowFrame::PasteTree( SwFrame *pStart, SwLayoutFrame *pParent, SwFrame *pSibling,
                           SwFrame *pOldParent )
{
    // returns true if there's a LayoutFrame in the chain.
    bool bRet = false;

    // The chain beginning with pStart is inserted before pSibling
    // under the parent. We take care to invalidate as required.

    // I'm receiving a finished chain. We need to update the pointers for
    // the beginning of the chain, then all the uppers and finally the end.
    // On the way there, we invalidate as required.
    if ( pSibling )
    {
        if ( nullptr != (pStart->mpPrev = pSibling->GetPrev()) )
            pStart->GetPrev()->mpNext = pStart;
        else
            pParent->m_pLower = pStart;
        pSibling->InvalidatePos_();
        pSibling->InvalidatePrt_();
    }
    else
    {
        if ( nullptr == (pStart->mpPrev = pParent->Lower()) )
            pParent->m_pLower = pStart;
        else
        //Modified for #i100782#,04/03/2009
        //If the pParent has more than 1 child nodes, former design will
        //ignore them directly without any collection work. It will make some
        //dangling pointers. This lead the crash...
        //The new design will find the last child of pParent in loop way, and
        //add the pStart after the last child.
        //  pParent->Lower()->pNext = pStart;
        {
            SwFrame* pTemp = pParent->m_pLower;
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
        if ( pParent->IsSctFrame() )
        {
            // We have no sibling because pParent is a section frame and
            // has just been created to contain some content. The printing
            // area of the frame behind pParent has to be invalidated, so
            // that the correct distance between pParent and the next frame
            // can be calculated.
            pParent->InvalidateNextPrtArea();
        }
    }
    SwFrame *pFloat = pStart;
    SwFrame *pLst = nullptr;
    SWRECTFN( pParent )
    SwTwips nGrowVal = 0;
    do
    {   pFloat->mpUpper = pParent;
        pFloat->InvalidateAll_();
        pFloat->CheckDirChange();

        // I'm a friend of the TextFrame and thus am allowed to do many things.
        // The CacheIdx idea seems to be a bit risky!
        if ( pFloat->IsTextFrame() )
        {
            if ( static_cast<SwTextFrame*>(pFloat)->GetCacheIdx() != USHRT_MAX )
                static_cast<SwTextFrame*>(pFloat)->Init();    // I'm his friend.
        }
        else
            bRet = true;

        nGrowVal += (pFloat->Frame().*fnRect->fnGetHeight)();
        if ( pFloat->GetNext() )
            pFloat = pFloat->GetNext();
        else
        {
            pLst = pFloat;
            pFloat = nullptr;
        }
    } while ( pFloat );

    if ( pSibling )
    {
        pLst->mpNext = pSibling;
        pSibling->mpPrev = pLst;
        if( pSibling->IsInFootnote() )
        {
            if( pSibling->IsSctFrame() )
                pSibling = static_cast<SwSectionFrame*>(pSibling)->ContainsAny();
            if( pSibling )
                pSibling->Prepare( PREP_ERGOSUM );
        }
    }
    if ( nGrowVal )
    {
        if ( pOldParent && pOldParent->IsBodyFrame() ) // For variable page height while browsing
            pOldParent->Shrink( nGrowVal );
        pParent->Grow( nGrowVal );
    }

    if ( pParent->IsFootnoteFrame() )
        static_cast<SwFootnoteFrame*>(pParent)->InvalidateNxtFootnoteCnts( pParent->FindPageFrame() );
    return bRet;
}

void SwFlowFrame::MoveSubTree( SwLayoutFrame* pParent, SwFrame* pSibling )
{
    OSL_ENSURE( pParent, "No parent given." );
    OSL_ENSURE( m_rThis.GetUpper(), "Where are we coming from?" );

    // Be economical with notifications if an action is running.
    SwViewShell *pSh = m_rThis.getRootFrame()->GetCurrShell();
    const SwViewShellImp *pImp = pSh ? pSh->Imp() : nullptr;
    const bool bComplete = pImp && pImp->IsAction() && pImp->GetLayAction().IsComplete();

    if ( !bComplete )
    {
        SwFrame *pPre = m_rThis.GetIndPrev();
        if ( pPre )
        {
            pPre->SetRetouche();
            // #115759# - follow-up of #i26250#
            // invalidate printing area of previous frame, if it's in a table
            if ( pPre->GetUpper()->IsInTab() )
            {
                pPre->InvalidatePrt_();
            }
            pPre->InvalidatePage();
        }
        else
        {
            m_rThis.GetUpper()->SetCompletePaint();
            m_rThis.GetUpper()->InvalidatePage();
        }
    }

    SwPageFrame *pOldPage = m_rThis.FindPageFrame();

    SwLayoutFrame *pOldParent;
    bool bInvaLay;

    {
        //JoinLock pParent for the lifetime of the Cut/Paste call to avoid
        //SwSectionFrame::MergeNext removing the pParent we're trying to reparent
        //into
        FlowFrameJoinLockGuard aJoinGuard(pParent);
        pOldParent = CutTree( &m_rThis );
        bInvaLay = PasteTree( &m_rThis, pParent, pSibling, pOldParent );
    }

    // If, by cutting & pasting, an empty SectionFrame came into existence, it should
    // disappear automatically.
    SwSectionFrame *pSct;
    // #126020# - adjust check for empty section
    // #130797# - correct fix #126020#
    if ( pOldParent && !pOldParent->Lower() &&
         ( pOldParent->IsInSct() &&
           !(pSct = pOldParent->FindSctFrame())->ContainsContent() &&
           !pSct->ContainsAny( true ) ) )
    {
            pSct->DelEmpty( false );
    }

    // If we're in a column section, we'd rather not call Calc "from below"
    if( !m_rThis.IsInSct() &&
        ( !m_rThis.IsInTab() || ( m_rThis.IsTabFrame() && !m_rThis.GetUpper()->IsInTab() ) ) )
        m_rThis.GetUpper()->Calc(m_rThis.getRootFrame()->GetCurrShell()->GetOut());
    else if( m_rThis.GetUpper()->IsSctFrame() )
    {
        SwSectionFrame* pTmpSct = static_cast<SwSectionFrame*>(m_rThis.GetUpper());
        bool bOld = pTmpSct->IsContentLocked();
        pTmpSct->SetContentLock( true );
        pTmpSct->Calc(m_rThis.getRootFrame()->GetCurrShell()->GetOut());
        if( !bOld )
            pTmpSct->SetContentLock( false );
    }
    SwPageFrame *pPage = m_rThis.FindPageFrame();

    if ( pOldPage != pPage )
    {
        m_rThis.InvalidatePage( pPage );
        if ( m_rThis.IsLayoutFrame() )
        {
            SwContentFrame *pCnt = static_cast<SwLayoutFrame*>(&m_rThis)->ContainsContent();
            if ( pCnt )
                pCnt->InvalidatePage( pPage );
        }
        else if ( pSh && pSh->GetDoc()->GetLineNumberInfo().IsRestartEachPage()
                  && pPage->FindFirstBodyContent() == &m_rThis )
        {
            m_rThis.InvalidateLineNum_();
        }
    }
    if ( bInvaLay || (pSibling && pSibling->IsLayoutFrame()) )
        m_rThis.GetUpper()->InvalidatePage( pPage );
}

bool SwFlowFrame::IsAnFollow( const SwFlowFrame *pAssumed ) const
{
    const SwFlowFrame *pFoll = this;
    do
    {   if ( pAssumed == pFoll )
            return true;
        pFoll = pFoll->GetFollow();
    } while ( pFoll );
    return false;
}

SwTextFrame* SwContentFrame::FindMaster() const
{
    OSL_ENSURE( IsFollow(), "SwContentFrame::FindMaster(): !IsFollow" );

    const SwContentFrame* pPrec = GetPrecede();

    if ( pPrec && pPrec->HasFollow() && pPrec->GetFollow() == this )
    {
        OSL_ENSURE( pPrec->IsTextFrame(), "NoTextFrame with follow found" );
        return const_cast<SwTextFrame*>(static_cast< const SwTextFrame* >(pPrec));
    }

    OSL_FAIL( "Follow ist lost in Space." );
    return nullptr;
}

SwSectionFrame* SwSectionFrame::FindMaster() const
{
    OSL_ENSURE( IsFollow(), "SwSectionFrame::FindMaster(): !IsFollow" );

    if (!m_pSection)
        return nullptr;

    SwIterator<SwSectionFrame,SwFormat> aIter( *m_pSection->GetFormat() );
    SwSectionFrame* pSect = aIter.First();
    while ( pSect )
    {
        if (pSect->GetFollow() == this)
            return pSect;
        pSect = aIter.Next();
    }

    OSL_FAIL( "Follow ist lost in Space." );
    return nullptr;
}

SwTabFrame* SwTabFrame::FindMaster( bool bFirstMaster ) const
{
    OSL_ENSURE( IsFollow(), "SwTabFrame::FindMaster(): !IsFollow" );

    SwIterator<SwTabFrame,SwFormat> aIter( *GetTable()->GetFrameFormat() );
    SwTabFrame* pTab = aIter.First();
    while ( pTab )
    {
            if ( bFirstMaster )
            {
                // Optimization. This makes code like this obsolete:
                // while ( pTab->IsFollow() )
                //     pTab = pTab->FindMaster();

                if ( !pTab->IsFollow() )
                {
                    SwTabFrame* pNxt = pTab;
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
    return nullptr;
}

/**
 * Returns the next/previous Layout leaf that's NOT below this (or even is this itself).
 * Also, that leaf must be in the same text flow as the pAnch origin frame (Body, Footnote)
 */
const SwLayoutFrame *SwFrame::GetLeaf( MakePageType eMakePage, bool bFwd,
                                   const SwFrame *pAnch ) const
{
    // No flow, no joy...
    if ( !(IsInDocBody() || IsInFootnote() || IsInFly()) )
        return nullptr;

    const SwFrame *pLeaf = this;
    bool bFound = false;

    do
    {   pLeaf = const_cast<SwFrame*>(pLeaf)->GetLeaf( eMakePage, bFwd );

        if ( pLeaf &&
            (!IsLayoutFrame() || !static_cast<const SwLayoutFrame*>(this)->IsAnLower( pLeaf )))
        {
            if ( pAnch->IsInDocBody() == pLeaf->IsInDocBody() &&
                 pAnch->IsInFootnote()     == pLeaf->IsInFootnote() )
            {
                bFound = true;
            }
        }
    } while ( !bFound && pLeaf );

    return static_cast<const SwLayoutFrame*>(pLeaf);
}

SwLayoutFrame *SwFrame::GetLeaf( MakePageType eMakePage, bool bFwd )
{
    if ( IsInFootnote() )
        return bFwd ? GetNextFootnoteLeaf( eMakePage ) : GetPrevFootnoteLeaf( eMakePage );

    // #i53323#
    // A frame could be inside a table AND inside a section.
    // Thus, it has to be determined, which is the first parent.
    bool bInTab( IsInTab() );
    bool bInSct( IsInSct() );
    if ( bInTab && bInSct )
    {
        const SwFrame* pUpperFrame( GetUpper() );
        while ( pUpperFrame )
        {
            if ( pUpperFrame->IsTabFrame() )
            {
                // the table is the first.
                bInSct = false;
                break;
            }
            else if ( pUpperFrame->IsSctFrame() )
            {
                // the section is the first.
                bInTab = false;
                break;
            }

            pUpperFrame = pUpperFrame->GetUpper();
        }
    }

    if ( bInTab && ( !IsTabFrame() || GetUpper()->IsCellFrame() ) ) // TABLE IN TABLE
        return bFwd ? GetNextCellLeaf( eMakePage ) : GetPrevCellLeaf( eMakePage );

    if ( bInSct )
        return bFwd ? GetNextSctLeaf( eMakePage ) : GetPrevSctLeaf( eMakePage );

    return bFwd ? GetNextLeaf( eMakePage ) : GetPrevLeaf( eMakePage );
}

bool SwFrame::WrongPageDesc( SwPageFrame* pNew )
{
    // Now it's getting a bit complicated:

    // Maybe I'm bringing a Pagedesc myself; in that case,
    // the pagedesc of the next page needs to correspond.
    // Otherwise, I'll have to dig a bit deeper to see where
    // the following Pagedesc is coming from.
    // If the following page itself tells me that it's pagedesc
    // is wrong, I can happily exchange it.
    // If the page however thinks that it's pagedesc is correct,
    // this doesn't mean it's useful to me:
    // If the first BodyContent asks for a PageDesc or a PageBreak,
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
    const SwFormatPageDesc &rFormatDesc = GetAttrSet()->GetPageDesc();

    //My Pagedesc doesn't count if I'm a follow!
    SwPageDesc *pDesc = nullptr;
    int nTmp = 0;
    SwFlowFrame *pFlow = SwFlowFrame::CastFlowFrame( this );
    if ( !pFlow || !pFlow->IsFollow() )
    {
        pDesc = const_cast<SwPageDesc*>(rFormatDesc.GetPageDesc());
        if( pDesc )
        {
            if( !pDesc->GetRightFormat() )
                nTmp = 2;
            else if( !pDesc->GetLeftFormat() )
                nTmp = 1;
            else if( rFormatDesc.GetNumOffset() )
                nTmp = rFormatDesc.GetNumOffset().get();
        }
    }

    // Does the Content bring a Pagedesc or do we need the
    // virtual page number of the new layout leaf?
    // PageDesc isn't allowed with Follows
    const bool bOdd = nTmp ? (nTmp % 2) !=0 : pNew->OnRightPage();
    if ( !pDesc )
        pDesc = pNew->FindPageDesc();

    bool bFirst = pNew->OnFirstPage();

    const SwFlowFrame *pNewFlow = pNew->FindFirstBodyContent();
    // Did we find ourselves?
    if( pNewFlow == pFlow )
        pNewFlow = nullptr;
    if ( pNewFlow && pNewFlow->GetFrame().IsInTab() )
        pNewFlow = pNewFlow->GetFrame().FindTabFrame();
    const SwPageDesc *pNewDesc= ( pNewFlow && !pNewFlow->IsFollow() )
            ? pNewFlow->GetFrame().GetAttrSet()->GetPageDesc().GetPageDesc() : nullptr;

    return (pNew->GetPageDesc() != pDesc)   //  own desc ?
        || (pNew->GetFormat() !=
              (bOdd ? pDesc->GetRightFormat(bFirst) : pDesc->GetLeftFormat(bFirst)))
        || (pNewDesc && pNewDesc == pDesc);
}

/// Returns the next layout leaf in which we can move the frame.
SwLayoutFrame *SwFrame::GetNextLeaf( MakePageType eMakePage )
{
    OSL_ENSURE( !IsInFootnote(), "GetNextLeaf(), don't call me for Footnote." );
    OSL_ENSURE( !IsInSct(), "GetNextLeaf(), don't call me for Sections." );

    const bool bBody = IsInDocBody();  // If I'm coming from the DocBody,
                                           // I want to end up in the body.

    // It doesn't make sense to insert pages, as we only want to search the
    // chain.
    if( IsInFly() )
        eMakePage = MAKEPAGE_NONE;

    // For tables, we just take the big leap. A simple GetNext would
    // iterate through the first cells and, in turn, all other cells.
    SwLayoutFrame *pLayLeaf = nullptr;
    if ( IsTabFrame() )
    {
        SwContentFrame* pTmp = static_cast<SwTabFrame*>(this)->FindLastContent();
        if ( pTmp )
            pLayLeaf = pTmp->GetUpper();
    }
    if ( !pLayLeaf )
        pLayLeaf = GetNextLayoutLeaf();

    SwLayoutFrame *pOldLayLeaf = nullptr;           // Make sure that we don't have to
                                            // start searching from top when we
                                            // have a freshly created page.
    bool bNewPg = false;    // Only insert a new page once.

    while ( true )
    {
        if ( pLayLeaf )
        {
            // There's yet another LayoutFrame. Let's see if it's ready to host
            // me as well.
            // It only needs to be of the same kind like my starting point
            // (DocBody or Footnote respectively)
            if ( pLayLeaf->FindPageFrame()->IsFootnotePage() )
            {   // If I ended up at the end note pages, we're done.
                pLayLeaf = nullptr;
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

            if( !IsFlowFrame() && ( eMakePage == MAKEPAGE_NONE ||
                eMakePage==MAKEPAGE_APPEND || eMakePage==MAKEPAGE_NOSECTION ) )
                return pLayLeaf;

            SwPageFrame *pNew = pLayLeaf->FindPageFrame();
            const SwViewShell *pSh = getRootFrame()->GetCurrShell();
            // #111704# The pagedesc check does not make sense for frames in fly frames
            if ( pNew != FindPageFrame() && !bNewPg && !IsInFly() &&
                 // #i46683#
                 // Do not consider page descriptions in browse mode (since
                 // MoveBwd ignored them)
                 !(pSh && pSh->GetViewOptions()->getBrowseMode() ) )
            {
                if( WrongPageDesc( pNew ) )
                {
                    SwFootnoteContFrame *pCont = pNew->FindFootnoteCont();
                    if( pCont )
                    {
                        // If the reference of the first footnote of this page
                        // lies before the page, we'd rather not insert a new page.
                        // (Bug #55620#)
                        SwFootnoteFrame *pFootnote = static_cast<SwFootnoteFrame*>(pCont->Lower());
                        if( pFootnote && pFootnote->GetRef() )
                        {
                            const sal_uInt16 nRefNum = pNew->GetPhyPageNum();
                            if( pFootnote->GetRef()->GetPhyPageNum() < nRefNum )
                                break;
                        }
                    }
                    //Gotcha! The following page is wrong, therefore we need to
                    //insert a new one.
                    if ( eMakePage == MAKEPAGE_INSERT )
                    {
                        bNewPg = true;

                        SwPageFrame *pPg = pOldLayLeaf ?
                                    pOldLayLeaf->FindPageFrame() : nullptr;
                        if ( pPg && pPg->IsEmptyPage() )
                            // Don't insert behind. Insert before the EmptyPage.
                            pPg = static_cast<SwPageFrame*>(pPg->GetPrev());

                        if ( !pPg || pPg == pNew )
                            pPg = FindPageFrame();

                        InsertPage( pPg, false );
                        pLayLeaf = GetNextLayoutLeaf();
                        pOldLayLeaf = nullptr;
                        continue;
                    }
                    else
                        pLayLeaf = nullptr;
                }
            }
            break;
        }
        else
        {
            // There's no other matching LayoutFrame, so we have to insert
            // a new page.
            if ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT )
            {
                InsertPage(
                    pOldLayLeaf ? pOldLayLeaf->FindPageFrame() : FindPageFrame(),
                    false );

                // And again from the start.
                pLayLeaf = pOldLayLeaf ? pOldLayLeaf : GetNextLayoutLeaf();
            }
            else
                break;
        }
    }
    return pLayLeaf;
}

/// Returns the previous layout leaf where we can move the frame.
SwLayoutFrame *SwFrame::GetPrevLeaf( MakePageType )
{
    OSL_ENSURE( !IsInFootnote(), "GetPrevLeaf(), don't call me for Footnote." );

    const bool bBody = IsInDocBody();  // If I'm coming from the DocBody,
                                           // I want to end up in the body.
    const bool bFly  = IsInFly();

    SwLayoutFrame *pLayLeaf = GetPrevLayoutLeaf();
    SwLayoutFrame *pPrevLeaf = nullptr;

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
                SwFlowFrame::SetMoveBwdJump( true );
        }
        else if ( bFly )
            break;  //Contents in Flys should accept any layout leaf.
        else
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
    }
    return pLayLeaf ? pLayLeaf : pPrevLeaf;
}

bool SwFlowFrame::IsPrevObjMove() const
{
    // true:   The FlowFrame must respect the a border of the predecessor, also needs
    //         to insert a break if required.

    //!!!!!!!!!!!Hack!!!!!!!!!!!
    const SwViewShell *pSh = m_rThis.getRootFrame()->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
        return false;

    SwFrame *pPre = m_rThis.FindPrev();

    if ( pPre && pPre->GetDrawObjs() )
    {
        OSL_ENSURE( SwFlowFrame::CastFlowFrame( pPre ), "new flowfrm?" );
        if( SwFlowFrame::CastFlowFrame( pPre )->IsAnFollow( this ) )
            return false;
        SwLayoutFrame* pPreUp = pPre->GetUpper();
        // If the upper is a SectionFrame, or a column of a SectionFrame, we're
        // allowed to protrude out of it.  However, we need to respect the
        // Upper of the SectionFrame.
        if( pPreUp->IsInSct() )
        {
            if( pPreUp->IsSctFrame() )
                pPreUp = pPreUp->GetUpper();
            else if( pPreUp->IsColBodyFrame() &&
                     pPreUp->GetUpper()->GetUpper()->IsSctFrame() )
                pPreUp = pPreUp->GetUpper()->GetUpper()->GetUpper();
        }
        // #i26945# - re-factoring
        // use <GetVertPosOrientFrame()> to determine, if object has followed the
        // text flow to the next layout frame
        for (SwAnchoredObject* pObj : *pPre->GetDrawObjs())
        {
            // #i28701# - consider changed type of
            // <SwSortedObjs> entries.
            // OD 2004-01-20 #110582# - do not consider hidden objects
            // #i26945# - do not consider object, which
            // doesn't follow the text flow.
            if ( pObj->GetFrameFormat().GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId(
                                            pObj->GetDrawObj()->GetLayer() ) &&
                 pObj->GetFrameFormat().GetFollowTextFlow().GetValue() )
            {
                const SwLayoutFrame* pVertPosOrientFrame = pObj->GetVertPosOrientFrame();
                if ( pVertPosOrientFrame &&
                     pPreUp != pVertPosOrientFrame &&
                     !pPreUp->IsAnLower( pVertPosOrientFrame ) )
                {
                    return true;
                }
            }
        }
    }
    return false;
}

/**
|*      If there's a hard page break before the Frame AND there's a
|*      predecessor on the same page, true is returned (we need to create a
|*      new PageBreak). Otherwise, returns false.
|*      If bAct is set to true, this function returns true if
|*      there's a PageBreak.
|*      Of course, we don't evaluate the hard page break for follows.
|*      The page break is in its own FrameFormat (BEFORE) or in the FrameFormat of the
|*      predecessor (AFTER). If there's no predecessor on the page, we don't
|*      need to think further.
|*      Also, a page break (or the need for one) is also present if
|*      the FrameFormat contains a PageDesc.
|*      The implementation works only on ContentFrames! - the definition
|*      of the predecessor is not clear for LayoutFrames.
|*/
bool SwFlowFrame::IsPageBreak( bool bAct ) const
{
    if ( !IsFollow() && m_rThis.IsInDocBody() &&
         ( !m_rThis.IsInTab() || ( m_rThis.IsTabFrame() && !m_rThis.GetUpper()->IsInTab() ) ) ) // i66968
    {
        const SwViewShell *pSh = m_rThis.getRootFrame()->GetCurrShell();
        if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            return false;
        const SwAttrSet *pSet = m_rThis.GetAttrSet();

        // Determine predecessor
        const SwFrame *pPrev = m_rThis.FindPrev();
        while ( pPrev && ( !pPrev->IsInDocBody() ||
                ( pPrev->IsTextFrame() && static_cast<const SwTextFrame*>(pPrev)->IsHiddenNow() ) ) )
            pPrev = pPrev->FindPrev();

        if ( pPrev )
        {
            OSL_ENSURE( pPrev->IsInDocBody(), "IsPageBreak: Not in DocBody?" );
            if ( bAct )
            {   if ( m_rThis.FindPageFrame() == pPrev->FindPageFrame() )
                    return false;
            }
            else
            {   if ( m_rThis.FindPageFrame() != pPrev->FindPageFrame() )
                    return false;
            }

            //for compatibility, also break at column break if no columns exist
            const SvxBreak eBreak = pSet->GetBreak().GetBreak();
            if ( eBreak == SVX_BREAK_PAGE_BEFORE ||
                 eBreak == SVX_BREAK_PAGE_BOTH ||
                 (eBreak == SVX_BREAK_COLUMN_BEFORE && !m_rThis.FindColFrame()) )
                return true;
            else
            {
                const SvxBreak &ePrB = pPrev->GetAttrSet()->GetBreak().GetBreak();
                if ( ePrB == SVX_BREAK_PAGE_AFTER ||
                     ePrB == SVX_BREAK_PAGE_BOTH  ||
                     pSet->GetPageDesc().GetPageDesc() )
                    return true;
            }
        }
    }
    return false;
}

/**
|*      If there's a hard column break before the Frame AND there is
|*      a predecessor in the same column, we return true (we need to create
|*      a ColBreak). Otherwise, we return false.
|*      If bAct is set to true, we return true if there's a ColBreak.
|*      Of course, we don't evaluate the hard column break for follows.
|*
|*      The column break is in its own FrameFormat (BEFORE) or in the FrameFormat of the
|*      predecessor (AFTER). If there's no predecessor in the column, we don't
|*      need to think further.
|*      The implementation works only on ContentFrames! - the definition
|*      of the predecessor is not clear for LayoutFrames.
|*/
bool SwFlowFrame::IsColBreak( bool bAct ) const
{
    if ( !IsFollow() && (m_rThis.IsMoveable() || bAct) )
    {
        const SwFrame *pCol = m_rThis.FindColFrame();
        if ( pCol )
        {
            // Determine predecessor
            const SwFrame *pPrev = m_rThis.FindPrev();
            while( pPrev && ( ( !pPrev->IsInDocBody() && !m_rThis.IsInFly() ) ||
                   ( pPrev->IsTextFrame() && static_cast<const SwTextFrame*>(pPrev)->IsHiddenNow() ) ) )
                    pPrev = pPrev->FindPrev();

            if ( pPrev )
            {
                if ( bAct )
                {   if ( pCol == pPrev->FindColFrame() )
                        return false;
                }
                else
                {   if ( pCol != pPrev->FindColFrame() )
                        return false;
                }

                const SvxBreak eBreak = m_rThis.GetAttrSet()->GetBreak().GetBreak();
                if ( eBreak == SVX_BREAK_COLUMN_BEFORE ||
                     eBreak == SVX_BREAK_COLUMN_BOTH )
                    return true;
                else
                {
                    const SvxBreak &ePrB = pPrev->GetAttrSet()->GetBreak().GetBreak();
                    if ( ePrB == SVX_BREAK_COLUMN_AFTER ||
                         ePrB == SVX_BREAK_COLUMN_BOTH )
                        return true;
                }
            }
        }
    }
    return false;
}

bool SwFlowFrame::HasParaSpaceAtPages( bool bSct ) const
{
    if( m_rThis.IsInSct() )
    {
        const SwFrame* pTmp = m_rThis.GetUpper();
        while( pTmp )
        {
            if( pTmp->IsCellFrame() || pTmp->IsFlyFrame() ||
                pTmp->IsFooterFrame() || pTmp->IsHeaderFrame() ||
                ( pTmp->IsFootnoteFrame() && !static_cast<const SwFootnoteFrame*>(pTmp)->GetMaster() ) )
                return true;
            if( pTmp->IsPageFrame() )
                return !( pTmp->GetPrev() && !IsPageBreak(true) );
            if( pTmp->IsColumnFrame() && pTmp->GetPrev() )
                return IsColBreak( true );
            if( pTmp->IsSctFrame() && ( !bSct || pTmp->GetPrev() ) )
                return false;
            pTmp = pTmp->GetUpper();
        }
        OSL_FAIL( "HasParaSpaceAtPages: Where's my page?" );
        return false;
    }
    if( !m_rThis.IsInDocBody() || ( m_rThis.IsInTab() && !m_rThis.IsTabFrame()) ||
        IsPageBreak( true ) || ( m_rThis.FindColFrame() && IsColBreak( true ) ) )
        return true;
    const SwFrame* pTmp = m_rThis.FindColFrame();
    if( pTmp )
    {
        if( pTmp->GetPrev() )
            return false;
    }
    else
        pTmp = &m_rThis;
    pTmp = pTmp->FindPageFrame();
    return pTmp && !pTmp->GetPrev();
}

/** helper method to determine previous frame for calculation of the
    upper space

    OD 2004-03-10 #i11860#
*/
const SwFrame* SwFlowFrame::GetPrevFrameForUpperSpaceCalc_( const SwFrame* _pProposedPrevFrame ) const
{
    const SwFrame* pPrevFrame = _pProposedPrevFrame
                            ? _pProposedPrevFrame
                            : m_rThis.GetPrev();

    // Skip hidden paragraphs and empty sections
    while ( pPrevFrame &&
            ( ( pPrevFrame->IsTextFrame() &&
                static_cast<const SwTextFrame*>(pPrevFrame)->IsHiddenNow() ) ||
              ( pPrevFrame->IsSctFrame() &&
                !static_cast<const SwSectionFrame*>(pPrevFrame)->GetSection() ) ) )
    {
        pPrevFrame = pPrevFrame->GetPrev();
    }

    // Special case: no direct previous frame is found but frame is in footnote
    // Search for a previous frame in previous footnote,
    // if frame isn't in a section, which is also in the footnote
    if ( !pPrevFrame && m_rThis.IsInFootnote() &&
         ( m_rThis.IsSctFrame() ||
           !m_rThis.IsInSct() || !m_rThis.FindSctFrame()->IsInFootnote() ) )
    {
        const SwFootnoteFrame* pPrevFootnoteFrame =
                static_cast<const SwFootnoteFrame*>(m_rThis.FindFootnoteFrame()->GetPrev());
        if ( pPrevFootnoteFrame )
        {
            pPrevFrame = pPrevFootnoteFrame->GetLastLower();

            // Skip hidden paragraphs and empty sections
            while ( pPrevFrame &&
                    ( ( pPrevFrame->IsTextFrame() &&
                        static_cast<const SwTextFrame*>(pPrevFrame)->IsHiddenNow() ) ||
                      ( pPrevFrame->IsSctFrame() &&
                        !static_cast<const SwSectionFrame*>(pPrevFrame)->GetSection() ) ) )
            {
                pPrevFrame = pPrevFrame->GetPrev();
            }
        }
    }
    // Special case: found previous frame is a section
    // Search for the last content in the section
    if( pPrevFrame && pPrevFrame->IsSctFrame() )
    {
        const SwSectionFrame* pPrevSectFrame =
                                    static_cast<const SwSectionFrame*>(pPrevFrame);
        pPrevFrame = pPrevSectFrame->FindLastContent();
        // If the last content is in a table _inside_ the section,
        // take the table herself.
        // OD 2004-02-18 #106629# - correction:
        // Check directly, if table is inside table, instead of indirectly
        // by checking, if section isn't inside a table
        if ( pPrevFrame && pPrevFrame->IsInTab() )
        {
            const SwTabFrame* pTableFrame = pPrevFrame->FindTabFrame();
            if ( pPrevSectFrame->IsAnLower( pTableFrame ) )
            {
                pPrevFrame = pTableFrame;
            }
        }
        // OD 2004-02-18 #106629# correction: skip hidden text frames
        while ( pPrevFrame &&
                pPrevFrame->IsTextFrame() &&
                static_cast<const SwTextFrame*>(pPrevFrame)->IsHiddenNow() )
        {
            pPrevFrame = pPrevFrame->GetPrev();
        }
    }

    return pPrevFrame;
}

/// Compare styles attached to these text frames.
static bool lcl_IdenticalStyles(const SwFrame* pPrevFrame, const SwFrame* pFrame)
{
    SwTextFormatColl *pPrevFormatColl = nullptr;
    if (pPrevFrame && pPrevFrame->IsTextFrame())
    {
        const SwTextFrame *pTextFrame = static_cast< const SwTextFrame * >( pPrevFrame );
        pPrevFormatColl = dynamic_cast<SwTextFormatColl*>(pTextFrame->GetTextNode()->GetFormatColl());
    }

    bool bIdenticalStyles = false;
    if (pFrame && pFrame->IsTextFrame())
    {
        const SwTextFrame *pTextFrame = static_cast< const SwTextFrame * >( pFrame );
        SwTextFormatColl *pFormatColl = dynamic_cast<SwTextFormatColl*>(pTextFrame->GetTextNode()->GetFormatColl());
        bIdenticalStyles = pPrevFormatColl == pFormatColl;
    }
    return bIdenticalStyles;
}

static bool lcl_getContextualSpacing(const SwFrame* pPrevFrame)
{
    bool bRet;
    auto pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), pPrevFrame);
    const SwBorderAttrs *pAttrs = pAccess->Get();

    bRet = pAttrs->GetULSpace().GetContext();

    return bRet;
}

// OD 2004-03-12 #i11860# - add 3rd parameter <_bConsiderGrid>
SwTwips SwFlowFrame::CalcUpperSpace( const SwBorderAttrs *pAttrs,
                                   const SwFrame* pPr,
                                   const bool _bConsiderGrid ) const
{
    // OD 2004-03-10 #i11860# - use new method <GetPrevFrameForUpperSpaceCalc(..)>
    const SwFrame* pPrevFrame = GetPrevFrameForUpperSpaceCalc_( pPr );

    std::unique_ptr<SwBorderAttrAccess> pAccess;
    SwFrame* pOwn;
    if( !pAttrs )
    {
        if( m_rThis.IsSctFrame() )
        {
            SwSectionFrame* pFoll = &static_cast<SwSectionFrame&>(m_rThis);
            do
                pOwn = pFoll->ContainsAny();
            while( !pOwn && nullptr != ( pFoll = pFoll->GetFollow() ) );
            if( !pOwn )
                return 0;
        }
        else
            pOwn = &m_rThis;
        pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), pOwn);
        pAttrs = pAccess->Get();
    }
    else
    {
        pOwn = &m_rThis;
    }
    SwTwips nUpper = 0;
    // OD 06.01.2004 #i11859#
    {
        const IDocumentSettingAccess& rIDSA = m_rThis.GetUpper()->GetFormat()->getIDocumentSettingAccess();
        const bool bUseFormerLineSpacing = rIDSA.get(DocumentSettingId::OLD_LINE_SPACING);
        if( pPrevFrame )
        {
            // OD 2004-03-10 #i11860# - use new method to determine needed spacing
            // values of found previous frame and use these values.
            SwTwips nPrevLowerSpace = 0;
            SwTwips nPrevLineSpacing = 0;
            // #i102458#
            bool bPrevLineSpacingPorportional = false;
            GetSpacingValuesOfFrame( (*pPrevFrame),
                                   nPrevLowerSpace, nPrevLineSpacing,
                                   bPrevLineSpacingPorportional );
            if( rIDSA.get(DocumentSettingId::PARA_SPACE_MAX) )
            {
                nUpper = nPrevLowerSpace + pAttrs->GetULSpace().GetUpper();
                SwTwips nAdd = nPrevLineSpacing;
                // OD 07.01.2004 #i11859# - consideration of the line spacing
                //      for the upper spacing of a text frame
                if ( bUseFormerLineSpacing )
                {
                    // former consideration
                    if ( pOwn->IsTextFrame() )
                    {
                        nAdd = std::max( nAdd, static_cast<SwTextFrame*>(pOwn)->GetLineSpace() );
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
                    if ( pOwn->IsTextFrame() )
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
                            nAdd += static_cast<SwTextFrame*>(pOwn)->GetLineSpace( true );
                        }
                        else
                        {
                            nAdd = std::max( nAdd, static_cast<SwTextFrame*>(pOwn)->GetLineSpace( true ) );
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
                    if ( pOwn->IsTextFrame() )
                        nUpper = std::max( nUpper, static_cast<SwTextFrame*>(pOwn)->GetLineSpace() );
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
                    if ( pOwn->IsTextFrame() )
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
                            nAdd += static_cast<SwTextFrame*>(pOwn)->GetLineSpace( true );
                        }
                        else
                        {
                            nAdd = std::max( nAdd, static_cast<SwTextFrame*>(pOwn)->GetLineSpace( true ) );
                        }
                    }
                    nUpper += nAdd;
                }
            }
        }
        else if ( rIDSA.get(DocumentSettingId::PARA_SPACE_MAX_AT_PAGES) &&
                  CastFlowFrame( pOwn )->HasParaSpaceAtPages( m_rThis.IsSctFrame() ) )
        {
            nUpper = pAttrs->GetULSpace().GetUpper();
        }
    }

    // OD 2004-02-26 #i25029# - pass previous frame <pPrevFrame>
    // to method <GetTopLine(..)>, if parameter <pPr> is set.
    // Note: parameter <pPr> is set, if method is called from <SwTextFrame::WouldFit(..)>
    nUpper += pAttrs->GetTopLine( m_rThis, (pPr ? pPrevFrame : nullptr) );

    // OD 2004-03-12 #i11860# - consider value of new parameter <_bConsiderGrid>
    // and use new method <GetUpperSpaceAmountConsideredForPageGrid(..)>

    //consider grid in square page mode
    if ( _bConsiderGrid && m_rThis.GetUpper()->GetFormat()->GetDoc()->IsSquaredPageMode() )
    {
        nUpper += GetUpperSpaceAmountConsideredForPageGrid_( nUpper );
    }

    const bool bContextualSpacing = pAttrs->GetULSpace().GetContext();

    if (bContextualSpacing && pPrevFrame && lcl_getContextualSpacing(pPrevFrame)
            && lcl_IdenticalStyles(pPrevFrame, &m_rThis))
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
SwTwips SwFlowFrame::GetUpperSpaceAmountConsideredForPageGrid_(
                            const SwTwips _nUpperSpaceWithoutGrid ) const
{
    SwTwips nUpperSpaceAmountConsideredForPageGrid = 0;

    if ( m_rThis.IsInDocBody() && m_rThis.GetAttrSet()->GetParaGrid().GetValue() )
    {
        const SwPageFrame* pPageFrame = m_rThis.FindPageFrame();
        SwTextGridItem const*const pGrid(GetGridItem(pPageFrame));
        if( pGrid )
        {
            const SwFrame* pBodyFrame = pPageFrame->FindBodyCont();
            if ( pBodyFrame )
            {
                const long nGridLineHeight =
                        pGrid->GetBaseHeight() + pGrid->GetRubyHeight();

                SWRECTFN( (&m_rThis) )
                const SwTwips nBodyPrtTop = (pBodyFrame->*fnRect->fnGetPrtTop)();
                const SwTwips nProposedPrtTop =
                        (*fnRect->fnYInc)( (m_rThis.Frame().*fnRect->fnGetTop)(),
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
                                            (m_rThis.Frame().*fnRect->fnGetTop)() );

                nUpperSpaceAmountConsideredForPageGrid =
                        nNewUpperSpace - _nUpperSpaceWithoutGrid;

                OSL_ENSURE( nUpperSpaceAmountConsideredForPageGrid >= 0,
                        "<SwFlowFrame::GetUpperSpaceAmountConsideredForPageGrid(..)> - negative space considered for page grid!" );
            }
        }
    }
    return nUpperSpaceAmountConsideredForPageGrid;
}

/** method to determine the upper space amount, which is considered for
    the previous frame

    OD 2004-03-11 #i11860#
*/
SwTwips SwFlowFrame::GetUpperSpaceAmountConsideredForPrevFrame() const
{
    SwTwips nUpperSpaceAmountOfPrevFrame = 0;

    const SwFrame* pPrevFrame = GetPrevFrameForUpperSpaceCalc_();
    if ( pPrevFrame )
    {
        SwTwips nPrevLowerSpace = 0;
        SwTwips nPrevLineSpacing = 0;
        // #i102458#
        bool bDummy = false;
        GetSpacingValuesOfFrame( (*pPrevFrame), nPrevLowerSpace, nPrevLineSpacing, bDummy );
        if ( nPrevLowerSpace > 0 || nPrevLineSpacing > 0 )
        {
            const IDocumentSettingAccess& rIDSA = m_rThis.GetUpper()->GetFormat()->getIDocumentSettingAccess();
            if (  rIDSA.get(DocumentSettingId::PARA_SPACE_MAX) ||
                 !rIDSA.get(DocumentSettingId::OLD_LINE_SPACING) )
            {
                nUpperSpaceAmountOfPrevFrame = nPrevLowerSpace + nPrevLineSpacing;
            }
            else
            {
                nUpperSpaceAmountOfPrevFrame = std::max( nPrevLowerSpace, nPrevLineSpacing );
            }
        }
    }

    return nUpperSpaceAmountOfPrevFrame;
}

/** method to determine the upper space amount, which is considered for
    the previous frame and the page grid, if option 'Use former object
    positioning' is OFF

    OD 2004-03-18 #i11860#
*/
SwTwips SwFlowFrame::GetUpperSpaceAmountConsideredForPrevFrameAndPageGrid() const
{
    SwTwips nUpperSpaceAmountConsideredForPrevFrameAndPageGrid = 0;

    if ( !m_rThis.GetUpper()->GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::USE_FORMER_OBJECT_POS) )
    {
        nUpperSpaceAmountConsideredForPrevFrameAndPageGrid =
            GetUpperSpaceAmountConsideredForPrevFrame() +
            ( m_rThis.GetUpper()->GetFormat()->GetDoc()->IsSquaredPageMode()
              ? GetUpperSpaceAmountConsideredForPageGrid_( CalcUpperSpace( nullptr, nullptr, false ) )
              : 0 );
    }

    return nUpperSpaceAmountConsideredForPrevFrameAndPageGrid;
}

/** calculation of lower space

    OD 2004-03-02 #106629#
*/
SwTwips SwFlowFrame::CalcLowerSpace( const SwBorderAttrs* _pAttrs ) const
{
    SwTwips nLowerSpace = 0;

    std::unique_ptr<SwBorderAttrAccess> pAttrAccess;
    if ( !_pAttrs )
    {
        pAttrAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), &m_rThis);
        _pAttrs = pAttrAccess->Get();
    }

    bool bCommonBorder = true;
    if ( m_rThis.IsInSct() && m_rThis.GetUpper()->IsColBodyFrame() )
    {
        const SwSectionFrame* pSectFrame = m_rThis.FindSctFrame();
        bCommonBorder = pSectFrame->GetFormat()->GetBalancedColumns().GetValue();
    }
    nLowerSpace = bCommonBorder ?
                  _pAttrs->GetBottomLine( m_rThis ) :
                  _pAttrs->CalcBottomLine();

    // #i26250#
    // - correct consideration of table frames
    // - use new method <CalcAddLowerSpaceAsLastInTableCell(..)>
    if ( ( ( m_rThis.IsTabFrame() && m_rThis.GetUpper()->IsInTab() ) ||
           // #115759# - no lower spacing, if frame has a follow
           ( m_rThis.IsInTab() && !GetFollow() ) ) &&
         !m_rThis.GetIndNext() )
    {
        nLowerSpace += CalcAddLowerSpaceAsLastInTableCell( _pAttrs );
    }

    return nLowerSpace;
}

/** calculation of the additional space to be considered, if flow frame
    is the last inside a table cell

    OD 2004-07-16 #i26250#
*/
SwTwips SwFlowFrame::CalcAddLowerSpaceAsLastInTableCell(
                                            const SwBorderAttrs* _pAttrs ) const
{
    SwTwips nAdditionalLowerSpace = 0;

    if ( m_rThis.GetUpper()->GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS) )
    {
        const SwFrame* pFrame = &m_rThis;
        if ( pFrame->IsSctFrame() )
        {
            const SwSectionFrame* pSectFrame = static_cast<const SwSectionFrame*>(pFrame);
            pFrame = pSectFrame->FindLastContent();
            if ( pFrame && pFrame->IsInTab() )
            {
                const SwTabFrame* pTableFrame = pFrame->FindTabFrame();
                if ( pSectFrame->IsAnLower( pTableFrame ) )
                {
                    pFrame = pTableFrame;
                }
            }
        }

        std::unique_ptr<SwBorderAttrAccess> pAttrAccess;
        if (pFrame && (!_pAttrs || pFrame != &m_rThis))
        {
            pAttrAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), pFrame);
            _pAttrs = pAttrAccess->Get();
        }

        if (_pAttrs)
            nAdditionalLowerSpace += _pAttrs->GetULSpace().GetLower();
    }

    return nAdditionalLowerSpace;
}

/// Moves the Frame forward if it seems necessary regarding the current conditions and attributes.
bool SwFlowFrame::CheckMoveFwd( bool& rbMakePage, bool bKeep, bool, bool bIgnoreMyOwnKeepValue )
{
    const SwFrame* pNxt = m_rThis.GetIndNext();

    if ( bKeep && //!bMovedBwd &&
         ( !pNxt || ( pNxt->IsTextFrame() && static_cast<const SwTextFrame*>(pNxt)->IsEmptyMaster() ) ) &&
         ( nullptr != (pNxt = m_rThis.FindNext()) ) && IsKeepFwdMoveAllowed(bIgnoreMyOwnKeepValue) )
    {
        if( pNxt->IsSctFrame() )
        {   // Don't get fooled by empty SectionFrames
            const SwFrame* pTmp = nullptr;
            while( pNxt && pNxt->IsSctFrame() &&
                   ( !static_cast<const SwSectionFrame*>(pNxt)->GetSection() ||
                     nullptr == ( pTmp = static_cast<const SwSectionFrame*>(pNxt)->ContainsAny() ) ) )
            {
                pNxt = pNxt->FindNext();
                pTmp = nullptr;
            }
            if( pTmp )
                pNxt = pTmp; // the content of the next notempty sectionfrm
        }
        if( pNxt && pNxt->GetValidPosFlag() )
        {
            bool bMove = false;
            const SwSectionFrame *pSct = m_rThis.FindSctFrame();
            if( pSct && !pSct->GetValidSizeFlag() )
            {
                const SwSectionFrame* pNxtSct = pNxt->FindSctFrame();
                if( pNxtSct && pSct->IsAnFollow( pNxtSct ) )
                    bMove = true;
            }
            else
                bMove = true;
            if( bMove )
            {
                //Keep together with the following frame
                MoveFwd( rbMakePage, false );
                return true;
            }
        }
    }

    bool bMovedFwd = false;

    if ( m_rThis.GetIndPrev() )
    {
        if ( IsPrevObjMove() ) // Should we care about objects of the Prev?
        {
            bMovedFwd = true;
            if ( !MoveFwd( rbMakePage, false ) )
                rbMakePage = false;
        }
        else
        {
            if ( IsPageBreak( false ) )
            {
                while ( MoveFwd( rbMakePage, true ) )
                        /* do nothing */;
                rbMakePage = false;
                bMovedFwd = true;
            }
            else if ( IsColBreak ( false ) )
            {
                const SwPageFrame *pPage = m_rThis.FindPageFrame();
                SwFrame *pCol = m_rThis.FindColFrame();
                do
                {   MoveFwd( rbMakePage, false );
                    SwFrame *pTmp = m_rThis.FindColFrame();
                    if( pTmp != pCol )
                    {
                        bMovedFwd = true;
                        pCol = pTmp;
                    }
                    else
                        break;
                } while ( IsColBreak( false ) );
                if ( pPage != m_rThis.FindPageFrame() )
                    rbMakePage = false;
            }
        }
    }
    return bMovedFwd;
}

/// Return value tells us whether the Frame has changed the page.
bool SwFlowFrame::MoveFwd( bool bMakePage, bool bPageBreak, bool bMoveAlways )
{
//!!!!MoveFootnoteCntFwd might need to be updated as well.
    SwFootnoteBossFrame *pOldBoss = m_rThis.FindFootnoteBossFrame();
    if (m_rThis.IsInFootnote())
    {
        if (!m_rThis.IsContentFrame())
        {
            SAL_WARN("sw.core", "Tables in footnotes are not truly supported");
            return false;
        }
        return static_cast<SwContentFrame&>(m_rThis).MoveFootnoteCntFwd( bMakePage, pOldBoss );
    }

    if( !IsFwdMoveAllowed() && !bMoveAlways )
    {
        bool bNoFwd = true;
        if( m_rThis.IsInSct() )
        {
            SwFootnoteBossFrame* pBoss = m_rThis.FindFootnoteBossFrame();
            bNoFwd = !pBoss->IsInSct() || ( !pBoss->Lower()->GetNext() &&
                     !pBoss->GetPrev() );
        }

        // Allow the MoveFwd even if we do not have an IndPrev in these cases:
        if ( m_rThis.IsInTab() &&
            ( !m_rThis.IsTabFrame() ||
                ( m_rThis.GetUpper()->IsInTab() &&
                  m_rThis.GetUpper()->FindTabFrame()->IsFwdMoveAllowed() ) ) &&
             nullptr != const_cast<SwFrame&>(m_rThis).GetNextCellLeaf( MAKEPAGE_NONE ) )
        {
            bNoFwd = false;
        }

        if( bNoFwd )
        {
            // It's allowed to move PageBreaks if the Frame isn't the first
            // one on the page.
            if ( !bPageBreak )
                return false;

            const SwFrame *pCol = m_rThis.FindColFrame();
            if ( !pCol || !pCol->GetPrev() )
                return false;
        }
    }

    bool bSamePage = true;
    SwLayoutFrame *pNewUpper =
            m_rThis.GetLeaf( bMakePage ? MAKEPAGE_INSERT : MAKEPAGE_NONE, true );

    if ( pNewUpper )
    {
        PROTOCOL_ENTER( &m_rThis, PROT::MoveFwd, DbgAction::NONE, nullptr );
        SwPageFrame *pOldPage = pOldBoss->FindPageFrame();
        // We move ourself and all the direct successors before the
        // first ContentFrame below the new Upper.

        // If our NewUpper lies in a SectionFrame, we need to make sure
        // that it won't destroy itself in Calc.
        SwSectionFrame* pSect = pNewUpper->FindSctFrame();
        if( pSect )
        {
            // If we only switch column within our SectionFrame, we better don't
            // call Calc, as this would format the SectionFrame, which in turn would
            // call us again, etc.
            if( pSect != m_rThis.FindSctFrame() )
            {
                bool bUnlock = !pSect->IsColLocked();
                pSect->ColLock();
                pNewUpper->Calc(m_rThis.getRootFrame()->GetCurrShell()->GetOut());
                if( bUnlock )
                    pSect->ColUnlock();
            }
        }
        // Do not calculate split cell frames.
        else if ( !pNewUpper->IsCellFrame() || static_cast<SwLayoutFrame*>(pNewUpper)->Lower() )
            pNewUpper->Calc(m_rThis.getRootFrame()->GetCurrShell()->GetOut());

        SwFootnoteBossFrame *pNewBoss = pNewUpper->FindFootnoteBossFrame();
        bool bBossChg = pNewBoss != pOldBoss;
        pNewBoss = pNewBoss->FindFootnoteBossFrame( true );
        pOldBoss = pOldBoss->FindFootnoteBossFrame( true );
        SwPageFrame* pNewPage = pOldPage;

        // First, we move the footnotes.
        bool bFootnoteMoved = false;

        // #i26831#
        // If pSect has just been created, the printing area of pSect has
        // been calculated based on the first content of its follow.
        // In this case we prefer to call a SimpleFormat for this new
        // section after we inserted the contents. Otherwise the section
        // frame will invalidate its lowers, if its printing area changes
        // in SwSectionFrame::Format, which can cause loops.
        const bool bForceSimpleFormat = pSect && pSect->HasFollow() &&
                                       !pSect->ContainsAny();

        if ( pNewBoss != pOldBoss )
        {
            pNewPage = pNewBoss->FindPageFrame();
            bSamePage = pNewPage == pOldPage;
            // Set deadline, so the footnotes don't think up
            // silly things...
            SWRECTFN( pOldBoss )
            SwSaveFootnoteHeight aHeight( pOldBoss,
                (pOldBoss->Frame().*fnRect->fnGetBottom)() );
            SwContentFrame* pStart = m_rThis.IsContentFrame() ?
                static_cast<SwContentFrame*>(&m_rThis) : static_cast<SwLayoutFrame&>(m_rThis).ContainsContent();
            OSL_ENSURE( pStart || ( m_rThis.IsTabFrame() && !static_cast<SwTabFrame&>(m_rThis).Lower() ),
                    "MoveFwd: Missing Content" );
            SwLayoutFrame* pBody = pStart ? ( pStart->IsTextFrame() ?
                const_cast<SwBodyFrame *>(static_cast<SwTextFrame*>(pStart)->FindBodyFrame()) : nullptr ) : nullptr;
            if( pBody )
                bFootnoteMoved = pBody->MoveLowerFootnotes( pStart, pOldBoss, pNewBoss,
                                                  false);
        }
        // It's possible when dealing with SectionFrames that we have been moved
        // by pNewUpper->Calc(), for instance into the pNewUpper.
        // MoveSubTree or PasteTree respectively is not prepared to handle such a
        // situation.
        if( pNewUpper != m_rThis.GetUpper() )
        {
            // #i27145#
            SwSectionFrame* pOldSct = nullptr;
            if ( m_rThis.GetUpper()->IsSctFrame() )
            {
                pOldSct = static_cast<SwSectionFrame*>(m_rThis.GetUpper());
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

            if ( bFootnoteMoved && !bSamePage )
            {
                pOldPage->UpdateFootnoteNum();
                pNewPage->UpdateFootnoteNum();
            }

            if( bBossChg )
            {
                m_rThis.Prepare( PREP_BOSS_CHGD, nullptr, false );
                if( !bSamePage )
                {
                    SwViewShell *pSh = m_rThis.getRootFrame()->GetCurrShell();
                    if ( pSh && !pSh->Imp()->IsUpdateExpFields() )
                        pSh->GetDoc()->getIDocumentFieldsAccess().SetNewFieldLst(true);  // Will be done by CalcLayout() later on!

                    pNewPage->InvalidateSpelling();
                    pNewPage->InvalidateSmartTags();
                    pNewPage->InvalidateAutoCompleteWords();
                    pNewPage->InvalidateWordCount();
                }
            }
        }
        // OD 30.10.2002 #97265# - no <CheckPageDesc(..)> in online layout
        const SwViewShell *pSh = m_rThis.getRootFrame()->GetCurrShell();

        if ( !( pSh && pSh->GetViewOptions()->getBrowseMode() ) )
        {
            // #i106452#
            // check page description not only in situation with sections.
            if ( !bSamePage &&
                 ( m_rThis.GetAttrSet()->GetPageDesc().GetPageDesc() ||
                   pOldPage->GetPageDesc()->GetFollow() != pNewPage->GetPageDesc() ) )
            {
                SwFrame::CheckPageDescs( pNewPage, false );
            }
        }
    }
    return bSamePage;
}

/** Return value tells whether the Frame should change the page.
 *
 * @note This should be called by derived classes.
 * @note The actual moving must be implemented in the subclasses.
 */
bool SwFlowFrame::MoveBwd( bool &rbReformat )
{
    SwFlowFrame::SetMoveBwdJump( false );

    SwFootnoteFrame* pFootnote = m_rThis.FindFootnoteFrame();
    if ( pFootnote && pFootnote->IsBackMoveLocked() )
        return false;

    // #115759# - text frames, which are directly inside
    // tables aren't allowed to move backward.
    if ( m_rThis.IsTextFrame() && m_rThis.IsInTab() )
    {
        const SwLayoutFrame* pUpperFrame = m_rThis.GetUpper();
        while ( pUpperFrame )
        {
            if ( pUpperFrame->IsTabFrame() )
            {
                return false;
            }
            if ( pUpperFrame->IsColumnFrame() && pUpperFrame->IsInSct() )
            {
                break;
            }
            pUpperFrame = pUpperFrame->GetUpper();
        }
    }

    SwFootnoteBossFrame * pOldBoss = m_rThis.FindFootnoteBossFrame();
    if (!pOldBoss)
        return false;

    SwPageFrame * const pOldPage = pOldBoss->FindPageFrame();
    SwLayoutFrame *pNewUpper = nullptr;
    bool bCheckPageDescs = false;
    bool bCheckPageDescOfNextPage = false;

    if ( pFootnote )
    {
        // If the footnote already sits on the same page/column as the reference,
        // we can't flow back. The breaks don't need to be checked for footnotes.

        // #i37084# FindLastContent does not necessarily
        // have to have a result != 0
        SwFrame* pRef = nullptr;
        const bool bEndnote = pFootnote->GetAttr()->GetFootnote().IsEndNote();
        if( bEndnote && pFootnote->IsInSct() )
        {
            SwSectionFrame* pSect = pFootnote->FindSctFrame();
            if( pSect->IsEndnAtEnd() )
                pRef = pSect->FindLastContent( SwFindMode::LastCnt );
        }
        if( !pRef )
            pRef = pFootnote->GetRef();

        OSL_ENSURE( pRef, "MoveBwd: Endnote for an empty section?" );

        if( !bEndnote )
            pOldBoss = pOldBoss->FindFootnoteBossFrame( true );
        SwFootnoteBossFrame *pRefBoss = pRef->FindFootnoteBossFrame( !bEndnote );
        if ( pOldBoss != pRefBoss &&
             // OD 08.11.2002 #104840# - use <SwLayoutFrame::IsBefore(..)>
             ( !bEndnote ||
               pRefBoss->IsBefore( pOldBoss ) )
           )
            pNewUpper = m_rThis.GetLeaf( MAKEPAGE_FTN, false );
    }
    else if ( IsPageBreak( true ) ) // Do we have to respect a PageBreak?
    {
        // If the previous page doesn't have an Frame in the body,
        // flowing back makes sense despite the PageBreak (otherwise,
        // we'd get an empty page).
        // Of course we need to overlook empty pages!
        const SwFrame *pFlow = &m_rThis;
        do
        {
            pFlow = pFlow->FindPrev();
        } while ( pFlow &&
                  ( pFlow->FindPageFrame() == pOldPage ||
                    !pFlow->IsInDocBody() ) );
        if ( pFlow )
        {
            long nDiff = pOldPage->GetPhyPageNum() - pFlow->GetPhyPageNum();
            if ( nDiff > 1 )
            {
                if ( static_cast<SwPageFrame*>(pOldPage->GetPrev())->IsEmptyPage() )
                    nDiff -= 1;
                if ( nDiff > 1 )
                {
                    pNewUpper = m_rThis.GetLeaf( MAKEPAGE_NONE, false );
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
                    if (pNewUpper && pNewUpper->Lower())
                    {
                        SwLayoutFrame* pNewNextUpper = pNewUpper->GetLeaf( MAKEPAGE_NONE, true );
                        if ( pNewNextUpper &&
                             pNewNextUpper != m_rThis.GetUpper() &&
                             pNewNextUpper->GetType() == pNewUpper->GetType() &&
                             pNewNextUpper->IsInDocBody() == pNewUpper->IsInDocBody() &&
                             pNewNextUpper->IsInFootnote() == pNewUpper->IsInFootnote() &&
                             pNewNextUpper->IsInTab() == pNewUpper->IsInTab() &&
                             pNewNextUpper->IsInSct() == pNewUpper->IsInSct() &&
                             !m_rThis.WrongPageDesc( pNewNextUpper->FindPageFrame() ) )
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
    else if ( IsColBreak( true ) )
    {
        // If the previous column doesn't contain a ContentFrame, flowing back
        // makes sense despite the ColumnBreak, as otherwise we'd get
        // an empty column.
        if( m_rThis.IsInSct() )
        {
            pNewUpper = m_rThis.GetLeaf( MAKEPAGE_NONE, false );
            if( pNewUpper && !SwFlowFrame::IsMoveBwdJump() &&
                ( pNewUpper->ContainsContent() ||
                  ( ( !pNewUpper->IsColBodyFrame() ||
                      !pNewUpper->GetUpper()->GetPrev() ) &&
                    !pNewUpper->FindSctFrame()->GetPrev() ) ) )
            {
                pNewUpper = nullptr;
            }
            // #i53139#
            // #i69409# - check <pNewUpper>
            // #i71065# - check <SwFlowFrame::IsMoveBwdJump()>
            else if ( pNewUpper && !SwFlowFrame::IsMoveBwdJump() )
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
                SwLayoutFrame* pNewNextUpper = pNewUpper->GetLeaf( MAKEPAGE_NOSECTION, true );
                if ( pNewNextUpper &&
                     pNewNextUpper->GetType() == pNewUpper->GetType() &&
                     pNewNextUpper->IsInDocBody() == pNewUpper->IsInDocBody() &&
                     pNewNextUpper->IsInFootnote() == pNewUpper->IsInFootnote() &&
                     pNewNextUpper->IsInTab() == pNewUpper->IsInTab() &&
                     pNewNextUpper->IsInSct() == pNewUpper->IsInSct() &&
                     !m_rThis.WrongPageDesc( pNewNextUpper->FindPageFrame() ) )
                {
                    pNewUpper = pNewNextUpper;
                }
            }
        }
        else
        {
            const SwFrame *pCol = m_rThis.FindColFrame();
            bool bGoOn = true;
            bool bJump = false;
            do
            {
                if ( pCol->GetPrev() )
                    pCol = pCol->GetPrev();
                else
                {
                    bGoOn = false;
                    pCol = m_rThis.GetLeaf( MAKEPAGE_NONE, false );
                }
                if ( pCol )
                {
                    // ColumnFrames now with BodyFrame
                    SwLayoutFrame* pColBody = pCol->IsColumnFrame() ?
                        const_cast<SwLayoutFrame*>(static_cast<const SwLayoutFrame*>(static_cast<const SwLayoutFrame*>(pCol)->Lower())) :
                        const_cast<SwLayoutFrame*>(static_cast<const SwLayoutFrame*>(pCol));
                    if ( pColBody->ContainsContent() )
                    {
                        bGoOn = false; // We have content here! we accept this
                        // only if GetLeaf() has set the MoveBwdJump.
                        if( SwFlowFrame::IsMoveBwdJump() )
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
                            SwLayoutFrame* pNewNextUpper = pNewUpper->GetLeaf( MAKEPAGE_NONE, true );
                            if ( pNewNextUpper &&
                                 pNewNextUpper != m_rThis.GetUpper() &&
                                 pNewNextUpper->GetType() == pNewUpper->GetType() &&
                                 pNewNextUpper->IsInDocBody() == pNewUpper->IsInDocBody() &&
                                 pNewNextUpper->IsInFootnote() == pNewUpper->IsInFootnote() &&
                                 pNewNextUpper->IsInTab() == pNewUpper->IsInTab() &&
                                 pNewNextUpper->IsInSct() == pNewUpper->IsInSct() &&
                                 !m_rThis.WrongPageDesc( pNewNextUpper->FindPageFrame() ) )
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
                SwFlowFrame::SetMoveBwdJump( true );
        }
    }
    else // No breaks - we can flow back.
        pNewUpper = m_rThis.GetLeaf( MAKEPAGE_NONE, false );

    // #i27801# - no move backward of 'master' text frame,
    // if - due to its object positioning - it isn't allowed to be on the new page frame
    // #i44049# - add another condition for not moving backward
    // If one of its objects has restarted the layout process, moving backward
    // isn't sensible either.
    // #i47697# - refine condition made for issue i44049
    // - allow move backward as long as the anchored object is only temporarily
    //   positions considering its wrapping style.
    if ( pNewUpper &&
         m_rThis.IsTextFrame() && !IsFollow() )
    {
        sal_uInt32 nToPageNum( 0L );
        const bool bMoveFwdByObjPos = SwLayouter::FrameMovedFwdByObjPos(
                                                *(pOldPage->GetFormat()->GetDoc()),
                                                static_cast<SwTextFrame&>(m_rThis),
                                                nToPageNum );
        if ( bMoveFwdByObjPos &&
             pNewUpper->FindPageFrame()->GetPhyPageNum() < nToPageNum )
        {
            pNewUpper = nullptr;
        }
        // #i44049# - check, if one of its anchored objects
        // has restarted the layout process.
        else if ( m_rThis.GetDrawObjs() )
        {
            for (SwAnchoredObject* pAnchoredObj : *m_rThis.GetDrawObjs())
            {
                // #i47697# - refine condition - see above
                if ( pAnchoredObj->RestartLayoutProcess() &&
                     !pAnchoredObj->IsTmpConsiderWrapInfluence() )
                {
                    pNewUpper = nullptr;
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
            const SwFrame* pLower( pNewUpper->Lower() );
            while ( pLower )
            {
                if ( pLower->IsSctFrame() &&
                     !dynamic_cast<const SwSectionFrame*>(pLower)->GetSection() )
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
            if ( SwFlowFrame::IsMoveBwdJump() )
            {
                // Don't move after the Master, but into the next empty page.
                SwFrame *pFrame = pNewUpper->Lower();
                while ( pFrame->GetNext() )
                    pFrame = pFrame->GetNext();
                pNewUpper = pFrame->GetLeaf( MAKEPAGE_INSERT, true );
                if( pNewUpper == m_rThis.GetUpper() ) // Did we end up in the same place?
                    pNewUpper = nullptr;               // If so, moving is not needed.
            }
            else
                pNewUpper = nullptr;
        }
    }
    if ( pNewUpper && !ShouldBwdMoved( pNewUpper, true, rbReformat ) )
    {
        if( !pNewUpper->Lower() )
        {
            if( pNewUpper->IsFootnoteContFrame() )
            {
                pNewUpper->Cut();
                SwFrame::DestroyFrame(pNewUpper);
            }
            else
            {
                SwSectionFrame* pSectFrame = pNewUpper->FindSctFrame();
                // #126020# - adjust check for empty section
                // #130797# - correct fix #126020#
                if ( pSectFrame && !pSectFrame->IsColLocked() &&
                     !pSectFrame->ContainsContent() && !pSectFrame->ContainsAny( true ) )
                {
                    pSectFrame->DelEmpty( true );
                    SwFrame::DestroyFrame(pSectFrame);
                    m_rThis.mbValidPos = true;
                }
            }
        }
        pNewUpper = nullptr;
    }

    // OD 2004-05-26 #i21478# - don't move backward, if flow frame wants to
    // keep with next frame and next frame is locked.
    // #i38232# - If next frame is a table, do *not* check,
    // if it's locked.
    if ( pNewUpper && !IsFollow() &&
         m_rThis.GetAttrSet()->GetKeep().GetValue() && m_rThis.GetIndNext() )
    {
        SwFrame* pIndNext = m_rThis.GetIndNext();
        // #i38232#
        if ( !pIndNext->IsTabFrame() )
        {
            // get first content of section, while empty sections are skipped
            while ( pIndNext && pIndNext->IsSctFrame() )
            {
                if( static_cast<SwSectionFrame*>(pIndNext)->GetSection() )
                {
                    SwFrame* pTmp = static_cast<SwSectionFrame*>(pIndNext)->ContainsAny();
                    if ( pTmp )
                    {
                        pIndNext = pTmp;
                        break;
                    }
                }
                pIndNext = pIndNext->GetIndNext();
            }
            OSL_ENSURE( !pIndNext || dynamic_cast<const SwTextFrame*>( pIndNext) !=  nullptr,
                    "<SwFlowFrame::MovedBwd(..)> - incorrect next found." );
            if ( pIndNext && pIndNext->IsFlowFrame() &&
                 SwFlowFrame::CastFlowFrame(pIndNext)->IsJoinLocked() )
            {
                pNewUpper = nullptr;
            }
        }
    }

    // #i65250#
    // layout loop control for flowing content again and again moving
    // backward under the same layout condition.
    if ( pNewUpper && !IsFollow() &&
         pNewUpper != m_rThis.GetUpper() &&
         SwLayouter::MoveBwdSuppressed( *(pOldPage->GetFormat()->GetDoc()),
                                        *this, *pNewUpper ) )
    {
        SwLayoutFrame* pNextNewUpper = pNewUpper->GetLeaf(
                                    ( !m_rThis.IsSctFrame() && m_rThis.IsInSct() )
                                    ? MAKEPAGE_NOSECTION
                                    : MAKEPAGE_NONE,
                                    true );
        // #i73194# - make code robust
        OSL_ENSURE( pNextNewUpper, "<SwFlowFrame::MoveBwd(..)> - missing next new upper" );
        if ( pNextNewUpper &&
             ( pNextNewUpper == m_rThis.GetUpper() ||
               pNextNewUpper->GetType() != m_rThis.GetUpper()->GetType() ) )
        {
            pNewUpper = nullptr;
            OSL_FAIL( "<SwFlowFrame::MoveBwd(..)> - layout loop control for layout action <Move Backward> applied!" );
        }
    }

    OSL_ENSURE( pNewUpper != m_rThis.GetUpper(),
            "<SwFlowFrame::MoveBwd(..)> - moving backward to the current upper frame!?" );
    if ( pNewUpper )
    {
        PROTOCOL_ENTER( &m_rThis, PROT::MoveBack, DbgAction::NONE, nullptr );
        if ( pNewUpper->IsFootnoteContFrame() )
        {
            // I may have gotten a Container
            SwFootnoteFrame *pOld = m_rThis.FindFootnoteFrame();
            SwFootnoteFrame *pNew = new SwFootnoteFrame( pOld->GetFormat(), pOld,
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
        if( pNewUpper->IsFootnoteFrame() && m_rThis.IsInSct() )
        {
            SwSectionFrame* pSct = m_rThis.FindSctFrame();
            // If we're in a section of a footnote, we may need to create
            // a SwSectionFrame in the new upper
            if( pSct->IsInFootnote() )
            {
                SwFrame* pTmp = pNewUpper->Lower();
                if( pTmp )
                {
                    while( pTmp->GetNext() )
                        pTmp = pTmp->GetNext();
                    if( !pTmp->IsSctFrame() ||
                        static_cast<SwSectionFrame*>(pTmp)->GetFollow() != pSct )
                        pTmp = nullptr;
                }
                if( pTmp )
                    pNewUpper = static_cast<SwSectionFrame*>(pTmp);
                else
                {
                    pSct = new SwSectionFrame( *pSct, true );
                    pSct->Paste( pNewUpper );
                    pSct->Init();
                    pNewUpper = pSct;
                    pSct->SimpleFormat();
                }
            }
        }
        bool bUnlock = false;
        bool bFollow = false;
        // Lock section. Otherwise, it could get destroyed if the only Content
        // moves e.g. from the second into the first column.
        SwSectionFrame* pSect = pNewUpper->FindSctFrame();
        if( pSect )
        {
            bUnlock = !pSect->IsColLocked();
            pSect->ColLock();
            bFollow = pSect->HasFollow();
        }
        pNewUpper->Calc(m_rThis.getRootFrame()->GetCurrShell()->GetOut());
        m_rThis.Cut();

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
            pSect->Calc(m_rThis.getRootFrame()->GetCurrShell()->GetOut());

        SwPageFrame *pNewPage = m_rThis.FindPageFrame();
        if( pNewPage != pOldPage )
        {
            m_rThis.Prepare( PREP_BOSS_CHGD, static_cast<const void*>(pOldPage), false );
            SwViewShell *pSh = m_rThis.getRootFrame()->GetCurrShell();
            if ( pSh && !pSh->Imp()->IsUpdateExpFields() )
                pSh->GetDoc()->getIDocumentFieldsAccess().SetNewFieldLst(true);  // Will be done by CalcLayout() later on

            pNewPage->InvalidateSpelling();
            pNewPage->InvalidateSmartTags();
            pNewPage->InvalidateAutoCompleteWords();
            pNewPage->InvalidateWordCount();

            // OD 30.10.2002 #97265# - no <CheckPageDesc(..)> in online layout
            if ( !( pSh && pSh->GetViewOptions()->getBrowseMode() ) )
            {
                if ( bCheckPageDescs && pNewPage->GetNext() )
                {
                    SwPageFrame* pStartPage = bCheckPageDescOfNextPage ?
                                            pNewPage :
                                            static_cast<SwPageFrame*>(pNewPage->GetNext());
                    SwFrame::CheckPageDescs( pStartPage, false);
                }
                else if ( m_rThis.GetAttrSet()->GetPageDesc().GetPageDesc() )
                {
                    // First page could get empty for example by disabling
                    // a section
                    SwFrame::CheckPageDescs( pNewPage, false);
                }
            }
        }
    }
    return pNewUpper != nullptr;
}

SwFlowFrame *SwFlowFrame::CastFlowFrame( SwFrame *pFrame )
{
    if ( pFrame->IsContentFrame() )
        return static_cast<SwContentFrame*>(pFrame);
    if ( pFrame->IsTabFrame() )
        return static_cast<SwTabFrame*>(pFrame);
    if ( pFrame->IsSctFrame() )
        return static_cast<SwSectionFrame*>(pFrame);
    return nullptr;
}

const SwFlowFrame *SwFlowFrame::CastFlowFrame( const SwFrame *pFrame )
{
    if ( pFrame->IsContentFrame() )
        return static_cast<const SwContentFrame*>(pFrame);
    if ( pFrame->IsTabFrame() )
        return static_cast<const SwTabFrame*>(pFrame);
    if ( pFrame->IsSctFrame() )
        return static_cast<const SwSectionFrame*>(pFrame);
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
