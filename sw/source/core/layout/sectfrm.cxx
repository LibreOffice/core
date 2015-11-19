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

#include <svl/itemiter.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <fmtclbl.hxx>
#include "sectfrm.hxx"
#include "section.hxx"
#include <IDocumentSettingAccess.hxx>
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "txtfrm.hxx"
#include "fmtclds.hxx"
#include "colfrm.hxx"
#include "tabfrm.hxx"
#include "ftnfrm.hxx"
#include "layouter.hxx"
#include "dbg_lay.hxx"
#include "viewopt.hxx"
#include "viewimp.hxx"
#include <editeng/brushitem.hxx>
#include <fmtftntx.hxx>
#include <flyfrms.hxx>
#include <sortedobjs.hxx>

SwSectionFrame::SwSectionFrame( SwSection &rSect, SwFrame* pSib )
    : SwLayoutFrame( rSect.GetFormat(), pSib )
    , SwFlowFrame( static_cast<SwFrame&>(*this) )
    , m_pSection( &rSect )
    , m_bFootnoteAtEnd(false)
    , m_bEndnAtEnd(false)
    , m_bContentLock(false)
    , m_bOwnFootnoteNum(false)
    , m_bFootnoteLock(false)
{
    mnFrameType = FRM_SECTION;

    CalcFootnoteAtEndFlag();
    CalcEndAtEndFlag();
}

SwSectionFrame::SwSectionFrame( SwSectionFrame &rSect, bool bMaster ) :
    SwLayoutFrame( rSect.GetFormat(), rSect.getRootFrame() ),
    SwFlowFrame( (SwFrame&)*this ),
    m_pSection( rSect.GetSection() ),
    m_bFootnoteAtEnd( rSect.IsFootnoteAtEnd() ),
    m_bEndnAtEnd( rSect.IsEndnAtEnd() ),
    m_bContentLock( false ),
    m_bOwnFootnoteNum( false ),
    m_bFootnoteLock( false )
{
    mnFrameType = FRM_SECTION;

    PROTOCOL( this, PROT_SECTION, bMaster ? ACT_CREATE_MASTER : ACT_CREATE_FOLLOW, &rSect )

    if( bMaster )
    {
        if( rSect.IsFollow() )
        {
            SwSectionFrame* pMaster = rSect.FindMaster();
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

// NOTE: call <SwSectionFrame::Init()> directly after creation of a new section
//       frame and its insert in the layout.
void SwSectionFrame::Init()
{
    OSL_ENSURE( GetUpper(), "SwSectionFrame::Init before insertion?!" );
    SWRECTFN( this )
    long nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
    (Frame().*fnRect->fnSetWidth)( nWidth );
    (Frame().*fnRect->fnSetHeight)( 0 );

    // #109700# LRSpace for sections
    const SvxLRSpaceItem& rLRSpace = GetFormat()->GetLRSpace();
    (Prt().*fnRect->fnSetLeft)( rLRSpace.GetLeft() );
    (Prt().*fnRect->fnSetWidth)( nWidth - rLRSpace.GetLeft() -
                                 rLRSpace.GetRight() );
    (Prt().*fnRect->fnSetHeight)( 0 );

    const SwFormatCol &rCol = GetFormat()->GetCol();
    if( ( rCol.GetNumCols() > 1 || IsAnyNoteAtEnd() ) && !IsInFootnote() )
    {
        const SwFormatCol *pOld = Lower() ? &rCol : new SwFormatCol;
        ChgColumns( *pOld, rCol, IsAnyNoteAtEnd() );
        if( pOld != &rCol )
            delete pOld;
    }
}

void SwSectionFrame::DestroyImpl()
{
    if( GetFormat() && !GetFormat()->GetDoc()->IsInDtor() )
    {
        SwRootFrame *pRootFrame = getRootFrame();
        if( pRootFrame )
            pRootFrame->RemoveFromList( this );
        if( IsFollow() )
        {
            SwSectionFrame *pMaster = FindMaster();
            if( pMaster )
            {
                PROTOCOL( this, PROT_SECTION, ACT_DEL_FOLLOW, pMaster )
                pMaster->SetFollow( GetFollow() );
                // A Master always grabs the space until the lower edge of his
                // Upper. If he doesn't have a Follow anymore, he can
                // release it, which is why the Size of the Master is
                // invalidated.
                if( !GetFollow() )
                    pMaster->InvalidateSize();
            }
        }
        else if( HasFollow() )
        {
            PROTOCOL( this, PROT_SECTION, ACT_DEL_MASTER, GetFollow() )
        }
    }

    SwLayoutFrame::DestroyImpl();
}

SwSectionFrame::~SwSectionFrame()
{
}

void SwSectionFrame::DelEmpty( bool bRemove )
{
    if( IsColLocked() )
    {
        OSL_ENSURE( !bRemove, "Don't delete locked SectionFrames" );
        return;
    }
    SwFrame* pUp = GetUpper();
    if( pUp )
    {
        // #i27138#
        // notify accessibility paragraphs objects about changed
        // CONTENT_FLOWS_FROM/_TO relation.
        // Relation CONTENT_FLOWS_FROM for current next paragraph will change
        // and relation CONTENT_FLOWS_TO for current previous paragraph will change.
        {
            SwViewShell* pViewShell( getRootFrame()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                pViewShell->InvalidateAccessibleParaFlowRelation(
                                dynamic_cast<SwTextFrame*>(FindNextCnt( true )),
                                dynamic_cast<SwTextFrame*>(FindPrevCnt( true )) );
            }
        }
        _Cut( bRemove );
    }
    SwSectionFrame *pMaster = IsFollow() ? FindMaster() : nullptr;
    if (pMaster)
    {
        pMaster->SetFollow( GetFollow() );
        // A Master always grabs the space until the lower edge of his
        // Upper. If he doesn't have a Follow anymore, he can
        // release it, which is why the Size of the Master is
        // invalidated.
        if( !GetFollow() && !pMaster->IsColLocked() )
            pMaster->InvalidateSize();
    }
    SetFollow(nullptr);
    if( pUp )
    {
        Frame().Height( 0 );
        // If we are destroyed immediately anyway, we don't need
        // to put us into the list
        if( bRemove )
        {   // If we already were half dead before this DelEmpty,
            // we are likely in the list and have to remove us from
            // it
            if( !m_pSection && getRootFrame() )
                getRootFrame()->RemoveFromList( this );
        }
        else if( getRootFrame() )
            getRootFrame()->InsertEmptySct( this );
        m_pSection = nullptr;  // like this a reanimation is virtually impossible though
    }
}

void SwSectionFrame::Cut()
{
    _Cut( true );
}

void SwSectionFrame::_Cut( bool bRemove )
{
    OSL_ENSURE( GetUpper(), "Cut ohne Upper()." );

    PROTOCOL( this, PROT_CUT, 0, GetUpper() )

    SwPageFrame *pPage = FindPageFrame();
    InvalidatePage( pPage );
    SwFrame *pFrame = GetNext();
    SwFrame* pPrepFrame = nullptr;
    while( pFrame && pFrame->IsSctFrame() && !static_cast<SwSectionFrame*>(pFrame)->GetSection() )
        pFrame = pFrame->GetNext();
    if( pFrame )
    {   // The former successor might have calculated a gap to the predecessor
        // which is now obsolete since he becomes the first
        pFrame->_InvalidatePrt();
        pFrame->_InvalidatePos();
        if( pFrame->IsSctFrame() )
            pFrame = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
        if ( pFrame && pFrame->IsContentFrame() )
        {
            pFrame->InvalidatePage( pPage );
            if( IsInFootnote() && !GetIndPrev() )
                pPrepFrame = pFrame;
        }
    }
    else
    {
        InvalidateNextPos();
        // Someone has to take over the retouching: predecessor or Upper
        if ( nullptr != (pFrame = GetPrev()) )
        {   pFrame->SetRetouche();
            pFrame->Prepare( PREP_WIDOWS_ORPHANS );
            if ( pFrame->IsContentFrame() )
                pFrame->InvalidatePage( pPage );
        }
        // If I am (was) the only FlowFrame in my Upper, then he has to take over
        // the retouching.
        // Furthermore a blank page could have emerged
        else
        {   SwRootFrame *pRoot = static_cast<SwRootFrame*>(pPage->GetUpper());
            pRoot->SetSuperfluous();
            GetUpper()->SetCompletePaint();
        }
    }
    // First remove, then shrink Upper
    SwLayoutFrame *pUp = GetUpper();
    if( bRemove )
    {
        RemoveFromLayout();
        if( pUp && !pUp->Lower() && pUp->IsFootnoteFrame() && !pUp->IsColLocked() &&
            pUp->GetUpper() )
        {
            pUp->Cut();
            SwFrame::DestroyFrame(pUp);
            pUp = nullptr;
        }
    }
    if( pPrepFrame )
        pPrepFrame->Prepare( PREP_FTN );
    if ( pUp )
    {
        SWRECTFN( this );
        SwTwips nFrameHeight = (Frame().*fnRect->fnGetHeight)();
        if( nFrameHeight > 0 )
        {
            if( !bRemove )
            {
                (Frame().*fnRect->fnSetHeight)( 0 );
                (Prt().*fnRect->fnSetHeight)( 0 );
            }
            pUp->Shrink( nFrameHeight );
        }
    }
}

void SwSectionFrame::Paste( SwFrame* pParent, SwFrame* pSibling )
{
    OSL_ENSURE( pParent, "No parent for Paste()." );
    OSL_ENSURE( pParent->IsLayoutFrame(), "Parent is ContentFrame." );
    OSL_ENSURE( pParent != this, "I'm my own parent." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetUpper(),
            "I am still registered somewhere." );

    PROTOCOL( this, PROT_PASTE, 0, GetUpper() )

    // Add to the tree
    SwSectionFrame* pSect = pParent->FindSctFrame();
    // Assure that parent is not inside a table frame, which is inside the found section frame.
    if ( pSect )
    {
        SwTabFrame* pTableFrame = pParent->FindTabFrame();
        if ( pTableFrame &&
             pSect->IsAnLower( pTableFrame ) )
        {
            pSect = nullptr;
        }
    }

    SWRECTFN( pParent )
    if( pSect && HasToBreak( pSect ) )
    {
        if( pParent->IsColBodyFrame() ) // dealing with a single-column area
        {
            // If we are coincidentally at the end of a column, pSibling
            // has to point to the first frame of the next column in order
            // for the content of the next column to be moved correctly to the
            // newly created pSect by the InsertGroup
            SwColumnFrame *pCol = static_cast<SwColumnFrame*>(pParent->GetUpper());
            while( !pSibling && nullptr != ( pCol = static_cast<SwColumnFrame*>(pCol->GetNext()) ) )
                pSibling = static_cast<SwLayoutFrame*>(static_cast<SwColumnFrame*>(pCol)->Lower())->Lower();
            if( pSibling )
            {
                // Even worse: every following column content has to
                // be attached to the pSibling-chain in order to be
                // taken along
                SwFrame *pTmp = pSibling;
                while ( nullptr != ( pCol = static_cast<SwColumnFrame*>(pCol->GetNext()) ) )
                {
                    while ( pTmp->GetNext() )
                        pTmp = pTmp->GetNext();
                    SwFrame* pSave = ::SaveContent( pCol );
                    if (pSave)
                        ::RestoreContent( pSave, pSibling->GetUpper(), pTmp, true );
                }
            }
        }
        pParent = pSect;
        pSect = new SwSectionFrame( *static_cast<SwSectionFrame*>(pParent)->GetSection(), pParent );
        // if pParent is decomposed into two parts, its Follow has to be attached
        // to the new second part
        pSect->SetFollow( static_cast<SwSectionFrame*>(pParent)->GetFollow() );
        static_cast<SwSectionFrame*>(pParent)->SetFollow( nullptr );
        if( pSect->GetFollow() )
            pParent->_InvalidateSize();

        const bool bInserted = InsertGroupBefore( pParent, pSibling, pSect );
        if (bInserted)
        {
            pSect->Init();
            (pSect->*fnRect->fnMakePos)( pSect->GetUpper(), pSect->GetPrev(), true);
        }
        if( !static_cast<SwLayoutFrame*>(pParent)->Lower() )
        {
            SwSectionFrame::MoveContentAndDelete( static_cast<SwSectionFrame*>(pParent), false );
            pParent = this;
        }
    }
    else
        InsertGroupBefore( pParent, pSibling, nullptr );

    _InvalidateAll();
    SwPageFrame *pPage = FindPageFrame();
    InvalidatePage( pPage );

    if ( pSibling )
    {
        pSibling->_InvalidatePos();
        pSibling->_InvalidatePrt();
        if ( pSibling->IsContentFrame() )
            pSibling->InvalidatePage( pPage );
    }

    SwTwips nFrameHeight = (Frame().*fnRect->fnGetHeight)();
    if( nFrameHeight )
        pParent->Grow( nFrameHeight );

    if ( GetPrev() )
    {
        if ( !IsFollow() )
        {
            GetPrev()->InvalidateSize();
            if ( GetPrev()->IsContentFrame() )
                GetPrev()->InvalidatePage( pPage );
        }
    }
}

/**
|*  Here it's decided whether the this-SectionFrame should break up
|*  the passed (Section)frm (or not).
|*  Initially, all superior sections are broken up. Later on that could
|*  be made configurable.
|*/
bool SwSectionFrame::HasToBreak( const SwFrame* pFrame ) const
{
    if( !pFrame->IsSctFrame() )
        return false;

    const SwSectionFormat *pTmp = static_cast<const SwSectionFormat*>(GetFormat());

    const SwFrameFormat *pOtherFormat = static_cast<const SwSectionFrame*>(pFrame)->GetFormat();
    do
    {
        pTmp = pTmp->GetParent();
        if( !pTmp )
            return false;
        if( pTmp == pOtherFormat )
            return true;
    } while( true ); // ( pTmp->GetSect().GetValue() );
}

/**
|*  Merges two SectionFrames, in case it's about the same section.
|*  This can be necessary when a (sub)section is deleted that had
|*  divided another part into two.
|*/
void SwSectionFrame::MergeNext( SwSectionFrame* pNxt )
{
    if (pNxt->IsDeleteForbidden())
        return;

    if (!pNxt->IsJoinLocked() && GetSection() == pNxt->GetSection())
    {
        PROTOCOL( this, PROT_SECTION, ACT_MERGE, pNxt )

        SwFrame* pTmp = ::SaveContent( pNxt );
        if( pTmp )
        {
            SwFrame* pLast = Lower();
            SwLayoutFrame* pLay = this;
            if( pLast )
            {
                while( pLast->GetNext() )
                    pLast = pLast->GetNext();
                if( pLast->IsColumnFrame() )
                {   // Columns now with BodyFrame
                    pLay = static_cast<SwLayoutFrame*>(static_cast<SwLayoutFrame*>(pLast)->Lower());
                    pLast = pLay->Lower();
                    if( pLast )
                        while( pLast->GetNext() )
                            pLast = pLast->GetNext();
                }
            }
            ::RestoreContent( pTmp, pLay, pLast, true );
        }
        SetFollow( pNxt->GetFollow() );
        pNxt->SetFollow( nullptr );
        pNxt->Cut();
        SwFrame::DestroyFrame(pNxt);
        InvalidateSize();
    }
}

/**
|*  Divides a SectionFrame into two parts. The second one starts with the
|*  passed frame.
|*  This is required when inserting an inner section, because the MoveFwd
|*  cannot have the desired effect within a frame or a table cell.
|*/
bool SwSectionFrame::SplitSect( SwFrame* pFrame, bool bApres )
{
    OSL_ENSURE( pFrame, "SplitSect: Why?" );
    SwFrame* pOther = bApres ? pFrame->FindNext() : pFrame->FindPrev();
    if( !pOther )
        return false;
    SwSectionFrame* pSect = pOther->FindSctFrame();
    if( pSect != this )
        return false;
    // Put the content aside
    SwFrame* pSav = ::SaveContent( this, bApres ? pOther : pFrame );
    OSL_ENSURE( pSav, "SplitSect: What's on?" );
    if( pSav ) // be robust
    {   // Create a new SctFrame, not as a Follower/master
        SwSectionFrame* pNew = new SwSectionFrame( *pSect->GetSection(), pSect );
        pNew->InsertBehind( pSect->GetUpper(), pSect );
        pNew->Init();
        SWRECTFN( this )
        (pNew->*fnRect->fnMakePos)( nullptr, pSect, true );
        // OD 25.03.2003 #108339# - restore content:
        // determine layout frame for restoring content after the initialization
        // of the section frame. In the section initialization the columns are
        // created.
        {
            SwLayoutFrame* pLay = pNew;
            // Search for last layout frame, e.g. for columned sections.
            while( pLay->Lower() && pLay->Lower()->IsLayoutFrame() )
                pLay = static_cast<SwLayoutFrame*>(pLay->Lower());
            ::RestoreContent( pSav, pLay, nullptr, true );
        }
        _InvalidateSize();
        if( HasFollow() )
        {
            pNew->SetFollow( GetFollow() );
            SetFollow( nullptr );
        }
        return true;
    }
    return false;
}

/**
|*  MoveContent is called for destroying a SectionFrames, due to
|*  the cancellation or hiding of a section, to handle the content.
|*  If the SectionFrame hasn't broken up another one, then the content
|*  is moved to the Upper. Otherwise the content is moved to another
|*  SectionFrame, which has to be potentially merged.
|*/
// If a multi-column section is cancelled, the ContentFrames have to be
// invalidated
static void lcl_InvalidateInfFlags( SwFrame* pFrame, bool bInva )
{
    while ( pFrame )
    {
        pFrame->InvalidateInfFlags();
        if( bInva )
        {
            pFrame->_InvalidatePos();
            pFrame->_InvalidateSize();
            pFrame->_InvalidatePrt();
        }
        if( pFrame->IsLayoutFrame() )
            lcl_InvalidateInfFlags( static_cast<SwLayoutFrame*>(pFrame)->GetLower(), false );
        pFrame = pFrame->GetNext();
    }
}

// Works like SwContentFrame::ImplGetNextContentFrame, but starts with a LayoutFrame
static SwContentFrame* lcl_GetNextContentFrame( const SwLayoutFrame* pLay, bool bFwd )
{
    if ( bFwd )
    {
        if ( pLay->GetNext() && pLay->GetNext()->IsContentFrame() )
            return const_cast<SwContentFrame*>(static_cast<const SwContentFrame*>(pLay->GetNext()));
    }
    else
    {
        if ( pLay->GetPrev() && pLay->GetPrev()->IsContentFrame() )
            return const_cast<SwContentFrame*>(static_cast<const SwContentFrame*>(pLay->GetPrev()));
    }

    const SwFrame* pFrame = pLay;
    SwContentFrame *pContentFrame = nullptr;
    bool bGoingUp = true;
    do {
        const SwFrame *p = nullptr;
        bool bGoingFwdOrBwd = false;

        bool bGoingDown = !bGoingUp && ( nullptr !=  ( p = pFrame->IsLayoutFrame() ? static_cast<const SwLayoutFrame*>(pFrame)->Lower() : nullptr ) );
        if ( !bGoingDown )
        {
            bGoingFwdOrBwd = ( nullptr != ( p = pFrame->IsFlyFrame() ?
                                          ( bFwd ? static_cast<const SwFlyFrame*>(pFrame)->GetNextLink() : static_cast<const SwFlyFrame*>(pFrame)->GetPrevLink() ) :
                                          ( bFwd ? pFrame->GetNext() :pFrame->GetPrev() ) ) );
            if ( !bGoingFwdOrBwd )
            {
                bGoingUp = (nullptr != (p = pFrame->GetUpper() ) );
                if ( !bGoingUp )
                    return nullptr;
            }
        }

        bGoingUp = !( bGoingFwdOrBwd || bGoingDown );

        if( !bFwd && bGoingDown && p )
            while ( p->GetNext() )
                p = p->GetNext();

        pFrame = p;
    } while ( nullptr == (pContentFrame = (pFrame->IsContentFrame() ? const_cast<SwContentFrame*>(static_cast<const SwContentFrame*>(pFrame)) : nullptr) ));

    return pContentFrame;
}

#define FIRSTLEAF( pLayFrame ) ( ( pLayFrame->Lower() && pLayFrame->Lower()->IsColumnFrame() )\
                    ? pLayFrame->GetNextLayoutLeaf() \
                    : pLayFrame )

void SwSectionFrame::MoveContentAndDelete( SwSectionFrame* pDel, bool bSave )
{
    bool bSize = pDel->Lower() && pDel->Lower()->IsColumnFrame();
    SwFrame* pPrv = pDel->GetPrev();
    SwLayoutFrame* pUp = pDel->GetUpper();
    // OD 27.03.2003 #i12711# - initialize local pointer variables.
    SwSectionFrame* pPrvSct = nullptr;
    SwSectionFrame* pNxtSct = nullptr;
    SwSectionFormat* pParent = static_cast<SwSectionFormat*>(pDel->GetFormat())->GetParent();
    if( pDel->IsInTab() && pParent )
    {
        SwTabFrame *pTab = pDel->FindTabFrame();
        // If we are within a table, we can only have broken up sections that
        // are inside as well, but not a section that contains the whole table.
        if( pTab->IsInSct() && pParent == pTab->FindSctFrame()->GetFormat() )
            pParent = nullptr;
    }
    // If our Format has a parent, we have probably broken up another
    // SectionFrame, which has to be checked. To do so we first acquire the
    // succeeding and the preceding ContentFrame, let's see if they
    // lay in the SectionFrames.
    // OD 27.03.2003 #i12711# - check, if previous and next section belonging
    // together and can be joined, *not* only if deleted section contains content.
    if ( pParent )
    {
        SwFrame* pPrvContent = lcl_GetNextContentFrame( pDel, false );
        pPrvSct = pPrvContent ? pPrvContent->FindSctFrame() : nullptr;
        SwFrame* pNxtContent = lcl_GetNextContentFrame( pDel, true );
        pNxtSct = pNxtContent ? pNxtContent->FindSctFrame() : nullptr;
    }
    else
    {
        pParent = nullptr;
        pPrvSct = pNxtSct = nullptr;
    }

    // Now the content is put aside and the frame is destroyed
    SwFrame *pSave = bSave ? ::SaveContent( pDel ) : nullptr;
    bool bOldFootnote = true;
    if( pSave && pUp->IsFootnoteFrame() )
    {
        bOldFootnote = static_cast<SwFootnoteFrame*>(pUp)->IsColLocked();
        static_cast<SwFootnoteFrame*>(pUp)->ColLock();
    }
    pDel->DelEmpty( true );
    SwFrame::DestroyFrame(pDel);
    if( pParent )
    {   // Search for the appropriate insert position
        if( pNxtSct && pNxtSct->GetFormat() == pParent )
        {   // Here we can insert outselves at the beginning
            pUp = FIRSTLEAF( pNxtSct );
            pPrv = nullptr;
            if( pPrvSct && !( pPrvSct->GetFormat() == pParent ) )
                pPrvSct = nullptr; // In order that nothing is merged
        }
        else if( pPrvSct && pPrvSct->GetFormat() == pParent )
        {   // Wonderful, here we can insert ourselves at the end
            pUp = pPrvSct;
            if( pUp->Lower() && pUp->Lower()->IsColumnFrame() )
            {
                pUp = static_cast<SwLayoutFrame*>(pUp->GetLastLower());
                // The body of the last column
                pUp = static_cast<SwLayoutFrame*>(pUp->Lower());
            }
            // In order to perform the insertion after the last one
            pPrv = pUp->GetLastLower();
            pPrvSct = nullptr; // Such that nothing is merged
        }
        else
        {
            if( pSave )
            {   // Following situations: before and after the section-to-be
                // deleted there is the section boundary of the enclosing
                // section, or another (sibling) section connects subsequently,
                // that derives from the same Parent.
                // In that case, there's not (yet) a part of our parent available
                // that can store the content, so we create it here.
                pPrvSct = new SwSectionFrame( *pParent->GetSection(), pUp );
                pPrvSct->InsertBehind( pUp, pPrv );
                pPrvSct->Init();
                SWRECTFN( pUp )
                (pPrvSct->*fnRect->fnMakePos)( pUp, pPrv, true );
                pUp = FIRSTLEAF( pPrvSct );
                pPrv = nullptr;
            }
            pPrvSct = nullptr; // Such that nothing will be merged
        }
    }
    // The content is going to be inserted..
    if( pSave )
    {
        lcl_InvalidateInfFlags( pSave, bSize );
        ::RestoreContent( pSave, pUp, pPrv, true );
        pUp->FindPageFrame()->InvalidateContent();
        if( !bOldFootnote )
            static_cast<SwFootnoteFrame*>(pUp)->ColUnlock();
    }
    // Now two parts of the superior section could possibly be merged
    if( pPrvSct && !pPrvSct->IsJoinLocked() )
    {
        OSL_ENSURE( pNxtSct, "MoveContent: No Merge" );
        pPrvSct->MergeNext( pNxtSct );
    }
}

void SwSectionFrame::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    if ( IsJoinLocked() || IsColLocked() || StackHack::IsLocked() || StackHack::Count() > 50 )
        return;
    if( !m_pSection ) // Via DelEmpty
    {
#ifdef DBG_UTIL
        OSL_ENSURE( getRootFrame()->IsInDelList( this ), "SectionFrame without Section" );
#endif
        if( !mbValidPos )
        {
            if( GetUpper() )
            {
                SWRECTFN( GetUpper() )
                (this->*fnRect->fnMakePos)( GetUpper(), GetPrev(), false );
            }
        }
        mbValidSize = mbValidPos = mbValidPrtArea = true;
        return;
    }
    LockJoin(); // I don't let myself to be destroyed on the way

    while( GetNext() && GetNext() == GetFollow() )
    {
        const SwFrame* pFoll = GetFollow();
        MergeNext( static_cast<SwSectionFrame*>(GetNext()) );
        if( pFoll == GetFollow() )
            break;
    }

    // OD 2004-03-15 #116561# - In online layout join the follows, if section
    // can grow.
    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() &&
         ( Grow( LONG_MAX, true ) > 0 ) )
    {
        while( GetFollow() )
        {
            const SwFrame* pFoll = GetFollow();
            MergeNext( GetFollow() );
            if( pFoll == GetFollow() )
                break;
        }
    }

    // A section with Follow uses all the space until the lower edge of the
    // Upper. If it moves, its size can grow or decrease...
    if( !mbValidPos && ToMaximize( false ) )
        mbValidSize = false;

#if OSL_DEBUG_LEVEL > 1
    const SwFormatCol &rCol = GetFormat()->GetCol();
    (void)rCol;
#endif
    SwLayoutFrame::MakeAll(getRootFrame()->GetCurrShell()->GetOut());
    UnlockJoin();
    if( m_pSection && IsSuperfluous() )
        DelEmpty( false );
}

bool SwSectionFrame::ShouldBwdMoved( SwLayoutFrame *, bool , bool & )
{
    OSL_FAIL( "Hups, wo ist meine Tarnkappe?" );
    return false;
}

const SwSectionFormat* SwSectionFrame::_GetEndSectFormat() const
{
    const SwSectionFormat *pFormat = m_pSection->GetFormat();
    while( !pFormat->GetEndAtTextEnd().IsAtEnd() )
    {
        if( dynamic_cast< const SwSectionFormat *>( pFormat->GetRegisteredIn()) !=  nullptr )
            pFormat = static_cast<const SwSectionFormat*>(pFormat->GetRegisteredIn());
        else
            return nullptr;
    }
    return pFormat;
}

static void lcl_FindContentFrame( SwContentFrame* &rpContentFrame, SwFootnoteFrame* &rpFootnoteFrame,
    SwFrame* pFrame, bool &rbChkFootnote )
{
    if( pFrame )
    {
        while( pFrame->GetNext() )
            pFrame = pFrame->GetNext();
        while( !rpContentFrame && pFrame )
        {
            if( pFrame->IsContentFrame() )
                rpContentFrame = static_cast<SwContentFrame*>(pFrame);
            else if( pFrame->IsLayoutFrame() )
            {
                if( pFrame->IsFootnoteFrame() )
                {
                    if( rbChkFootnote )
                    {
                        rpFootnoteFrame = static_cast<SwFootnoteFrame*>(pFrame);
                        rbChkFootnote = rpFootnoteFrame->GetAttr()->GetFootnote().IsEndNote();
                    }
                }
                else
                    lcl_FindContentFrame( rpContentFrame, rpFootnoteFrame,
                        static_cast<SwLayoutFrame*>(pFrame)->Lower(), rbChkFootnote );
            }
            pFrame = pFrame->GetPrev();
        }
    }
}

SwContentFrame *SwSectionFrame::FindLastContent( sal_uInt8 nMode )
{
    SwContentFrame *pRet = nullptr;
    SwFootnoteFrame *pFootnoteFrame = nullptr;
    SwSectionFrame *pSect = this;
    if( nMode )
    {
        const SwSectionFormat *pFormat = IsEndnAtEnd() ? GetEndSectFormat() :
                                     m_pSection->GetFormat();
        do {
            while( pSect->HasFollow() )
                pSect = pSect->GetFollow();
            SwFrame* pTmp = pSect->FindNext();
            while( pTmp && pTmp->IsSctFrame() &&
                   !static_cast<SwSectionFrame*>(pTmp)->GetSection() )
                pTmp = pTmp->FindNext();
            if( pTmp && pTmp->IsSctFrame() &&
                static_cast<SwSectionFrame*>(pTmp)->IsDescendantFrom( pFormat ) )
                pSect = static_cast<SwSectionFrame*>(pTmp);
            else
                break;
        } while( true );
    }
    bool bFootnoteFound = nMode == FINDMODE_ENDNOTE;
    do
    {
        lcl_FindContentFrame( pRet, pFootnoteFrame, pSect->Lower(), bFootnoteFound );
        if( pRet || !pSect->IsFollow() || !nMode ||
            ( FINDMODE_MYLAST == nMode && this == pSect ) )
            break;
        pSect = pSect->FindMaster();
    } while( pSect );
    if( ( nMode == FINDMODE_ENDNOTE ) && pFootnoteFrame )
        pRet = pFootnoteFrame->ContainsContent();
    return pRet;
}

bool SwSectionFrame::CalcMinDiff( SwTwips& rMinDiff ) const
{
    if( ToMaximize( true ) )
    {
        SWRECTFN( this )
        rMinDiff = (GetUpper()->*fnRect->fnGetPrtBottom)();
        rMinDiff = (Frame().*fnRect->fnBottomDist)( rMinDiff );
        return true;
    }
    return false;
}

/**
 *  CollectEndnotes looks for endnotes in the sectionfrm and his follows,
 *  the endnotes will cut off the layout and put into the array.
 *  If the first endnote is not a master-SwFootnoteFrame, the whole sectionfrm
 *  contains only endnotes and it is not necessary to collect them.
 */
static SwFootnoteFrame* lcl_FindEndnote( SwSectionFrame* &rpSect, bool &rbEmpty,
    SwLayouter *pLayouter )
{
    // if rEmpty is set, the rpSect is already searched
    SwSectionFrame* pSect = rbEmpty ? rpSect->GetFollow() : rpSect;
    while( pSect )
    {
       OSL_ENSURE( (pSect->Lower() && pSect->Lower()->IsColumnFrame()) || pSect->GetUpper()->IsFootnoteFrame(),
                "InsertEndnotes: Where's my column?" );

        // i73332: Columned section in endnote
        SwColumnFrame* pCol = nullptr;
        if(pSect->Lower() && pSect->Lower()->IsColumnFrame())
            pCol = static_cast<SwColumnFrame*>(pSect->Lower());

        while( pCol ) // check all columns
        {
            SwFootnoteContFrame* pFootnoteCont = pCol->FindFootnoteCont();
            if( pFootnoteCont )
            {
                SwFootnoteFrame* pRet = static_cast<SwFootnoteFrame*>(pFootnoteCont->Lower());
                while( pRet ) // look for endnotes
                {
                    /* CollectEndNode can destroy pRet so we need to get the
                       next early
                    */
                    SwFootnoteFrame* pRetNext = static_cast<SwFootnoteFrame*>(pRet->GetNext());
                    if( pRet->GetAttr()->GetFootnote().IsEndNote() )
                    {
                        if( pRet->GetMaster() )
                        {
                            if( pLayouter )
                                pLayouter->CollectEndnote( pRet );
                            else
                                return nullptr;
                        }
                        else
                            return pRet; // Found
                    }
                    pRet = pRetNext;
                }
            }
            pCol = static_cast<SwColumnFrame*>(pCol->GetNext());
        }
        rpSect = pSect;
        pSect = pLayouter ? pSect->GetFollow() : nullptr;
        rbEmpty = true;
    }
    return nullptr;
}

static void lcl_ColumnRefresh( SwSectionFrame* pSect, bool bFollow )
{
    vcl::RenderContext* pRenderContext = pSect->getRootFrame()->GetCurrShell()->GetOut();
    while( pSect )
    {
        bool bOldLock = pSect->IsColLocked();
        pSect->ColLock();
        if( pSect->Lower() && pSect->Lower()->IsColumnFrame() )
        {
            SwColumnFrame *pCol = static_cast<SwColumnFrame*>(pSect->Lower());
            do
            {   pCol->_InvalidateSize();
                pCol->_InvalidatePos();
                static_cast<SwLayoutFrame*>(pCol)->Lower()->_InvalidateSize();
                pCol->Calc(pRenderContext);   // calculation of column and
                static_cast<SwLayoutFrame*>(pCol)->Lower()->Calc(pRenderContext);  // body
                pCol = static_cast<SwColumnFrame*>(pCol->GetNext());
            } while ( pCol );
        }
        if( !bOldLock )
            pSect->ColUnlock();
        if( bFollow )
            pSect = pSect->GetFollow();
        else
            pSect = nullptr;
    }
}

void SwSectionFrame::CollectEndnotes( SwLayouter* pLayouter )
{
    OSL_ENSURE( IsColLocked(), "CollectEndnotes: You love the risk?" );
    // i73332: Section in footnode does not have columns!
    OSL_ENSURE( (Lower() && Lower()->IsColumnFrame()) || GetUpper()->IsFootnoteFrame(), "Where's my column?" );

    SwSectionFrame* pSect = this;
    SwFootnoteFrame* pFootnote;
    bool bEmpty = false;
    // pSect is the last sectionfrm without endnotes or the this-pointer
    // the first sectionfrm with endnotes may be destroyed, when the endnotes
    // is cutted
    while( nullptr != (pFootnote = lcl_FindEndnote( pSect, bEmpty, pLayouter )) )
        pLayouter->CollectEndnote( pFootnote );
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
void SwSectionFrame::_CheckClipping( bool bGrow, bool bMaximize )
{
    SWRECTFN( this )
    long nDiff;
    SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
    if( bGrow && ( !IsInFly() || !GetUpper()->IsColBodyFrame() ||
                   !FindFlyFrame()->IsLocked() ) )
    {
        nDiff = -(Frame().*fnRect->fnBottomDist)( nDeadLine );
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
    nDiff = -(Frame().*fnRect->fnBottomDist)( nDeadLine );
    SetUndersized( !bMaximize && nDiff >= 0 );
    const bool bCalc = ( IsUndersized() || bMaximize ) &&
                       ( nDiff ||
                         (Prt().*fnRect->fnGetTop)() > (Frame().*fnRect->fnGetHeight)() );
    // OD 03.11.2003 #i19737# - introduce local variable <bExtraCalc> to indicate
    // that a calculation has to be done beside the value of <bCalc>.
    bool bExtraCalc = false;
    if( !bCalc && !bGrow && IsAnyNoteAtEnd() && !IsInFootnote() )
    {
        SwSectionFrame *pSect = this;
        bool bEmpty = false;
        SwLayoutFrame* pFootnote = IsEndnAtEnd() ?
            lcl_FindEndnote( pSect, bEmpty, nullptr ) : nullptr;
        if( pFootnote )
        {
            pFootnote = pFootnote->FindFootnoteBossFrame();
            SwFrame* pTmp = FindLastContent( FINDMODE_LASTCNT );
            // OD 08.11.2002 #104840# - use <SwLayoutFrame::IsBefore(..)>
            if ( pTmp && pFootnote->IsBefore( pTmp->FindFootnoteBossFrame() ) )
                bExtraCalc = true;
        }
        else if( GetFollow() && !GetFollow()->ContainsAny() )
            bExtraCalc = true;
    }
    if ( bCalc || bExtraCalc )
    {
        nDiff = (*fnRect->fnYDiff)( nDeadLine, (Frame().*fnRect->fnGetTop)() );
        if( nDiff < 0 )
            nDeadLine = (Frame().*fnRect->fnGetTop)();
        const Size aOldSz( Prt().SSize() );
        long nTop = (this->*fnRect->fnGetTopMargin)();
        (Frame().*fnRect->fnSetBottom)( nDeadLine );
        nDiff = (Frame().*fnRect->fnGetHeight)();
        if( nTop > nDiff )
            nTop = nDiff;
        (this->*fnRect->fnSetYMargins)( nTop, 0 );

        // OD 18.09.2002 #100522#
        // Determine, if height has changed.
        // Note: In vertical layout the height equals the width value.
        bool bHeightChanged = bVert ?
                            (aOldSz.Width() != Prt().Width()) :
                            (aOldSz.Height() != Prt().Height());
        // Last but not least we have changed the height again, thus the inner
        // layout (columns) is calculated and the content as well.
        // OD 18.09.2002 #100522#
        // calculate content, only if height has changed.
        // OD 03.11.2003 #i19737# - restriction of content calculation too strong.
        // If an endnote has an incorrect position or a follow section contains
        // no content except footnotes/endnotes, the content has also been calculated.
        if ( ( bHeightChanged || bExtraCalc ) && Lower() )
        {
            if( Lower()->IsColumnFrame() )
            {
                lcl_ColumnRefresh( this, false );
                ::CalcContent( this );
            }
            else
            {
                ChgLowersProp( aOldSz );
                if( !bMaximize && !IsContentLocked() )
                    ::CalcContent( this );
            }
        }
    }
}

void SwSectionFrame::SimpleFormat()
{
    if ( IsJoinLocked() || IsColLocked() )
        return;
    LockJoin();
    SWRECTFN( this )
    if( GetPrev() || GetUpper() )
    {
        // assure notifications on position changes.
        const SwLayNotify aNotify( this );
        (this->*fnRect->fnMakePos)( GetUpper(), GetPrev(), false );
        mbValidPos = true;
    }
    SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
    // OD 22.10.2002 #97265# - call always method <lcl_ColumnRefresh(..)>, in
    // order to get calculated lowers, not only if there space left in its upper.
    if( (Frame().*fnRect->fnBottomDist)( nDeadLine ) >= 0 )
    {
        (Frame().*fnRect->fnSetBottom)( nDeadLine );
        long nHeight = (Frame().*fnRect->fnGetHeight)();
        long nTop = CalcUpperSpace();
        if( nTop > nHeight )
            nTop = nHeight;
        (this->*fnRect->fnSetYMargins)( nTop, 0 );
    }
    lcl_ColumnRefresh( this, false );
    UnlockJoin();
}

// #i40147# - helper class to perform extra section format
// to position anchored objects and to keep the position of whose objects locked.
class ExtraFormatToPositionObjs
{
    private:
        SwSectionFrame* mpSectFrame;
        bool mbExtraFormatPerformed;

    public:
        explicit ExtraFormatToPositionObjs( SwSectionFrame& _rSectFrame)
            : mpSectFrame( &_rSectFrame ),
              mbExtraFormatPerformed( false )
        {}

        ~ExtraFormatToPositionObjs()
        {
            if ( mbExtraFormatPerformed )
            {
                // release keep locked position of lower floating screen objects
                SwPageFrame* pPageFrame = mpSectFrame->FindPageFrame();
                SwSortedObjs* pObjs = pPageFrame ? pPageFrame->GetSortedObjs() : nullptr;
                if ( pObjs )
                {
                    for ( size_t i = 0; i < pObjs->size(); ++i )
                    {
                        SwAnchoredObject* pAnchoredObj = (*pObjs)[i];

                        if ( mpSectFrame->IsAnLower( pAnchoredObj->GetAnchorFrame() ) )
                        {
                            pAnchoredObj->SetKeepPosLocked( false );
                        }
                    }
                }
            }
        }

        // #i81555#
        void InitObjs( SwFrame& rFrame )
        {
            SwSortedObjs* pObjs = rFrame.GetDrawObjs();
            if ( pObjs )
            {
                for ( size_t i = 0; i < pObjs->size(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pObjs)[i];

                    pAnchoredObj->UnlockPosition();
                    pAnchoredObj->SetClearedEnvironment( false );
                }
            }
            SwLayoutFrame* pLayoutFrame = dynamic_cast<SwLayoutFrame*>(&rFrame);
            if ( pLayoutFrame != nullptr )
            {
                SwFrame* pLowerFrame = pLayoutFrame->GetLower();
                while ( pLowerFrame != nullptr )
                {
                    InitObjs( *pLowerFrame );

                    pLowerFrame = pLowerFrame->GetNext();
                }
            }
        }

        void FormatSectionToPositionObjs()
        {
            vcl::RenderContext* pRenderContext = mpSectFrame->getRootFrame()->GetCurrShell()->GetOut();
            // perform extra format for multi-columned section.
            if ( mpSectFrame->Lower() && mpSectFrame->Lower()->IsColumnFrame() &&
                 mpSectFrame->Lower()->GetNext() )
            {
                // grow section till bottom of printing area of upper frame
                SWRECTFN( mpSectFrame );
                SwTwips nTopMargin = (mpSectFrame->*fnRect->fnGetTopMargin)();
                Size aOldSectPrtSize( mpSectFrame->Prt().SSize() );
                SwTwips nDiff = (mpSectFrame->Frame().*fnRect->fnBottomDist)(
                                        (mpSectFrame->GetUpper()->*fnRect->fnGetPrtBottom)() );
                (mpSectFrame->Frame().*fnRect->fnAddBottom)( nDiff );
                (mpSectFrame->*fnRect->fnSetYMargins)( nTopMargin, 0 );
                // #i59789#
                // suppress formatting, if printing area of section is too narrow
                if ( (mpSectFrame->Prt().*fnRect->fnGetHeight)() <= 0 )
                {
                    return;
                }
                mpSectFrame->ChgLowersProp( aOldSectPrtSize );

                // format column frames and its body and footnote container
                SwColumnFrame* pColFrame = static_cast<SwColumnFrame*>(mpSectFrame->Lower());
                while ( pColFrame )
                {
                    pColFrame->Calc(pRenderContext);
                    pColFrame->Lower()->Calc(pRenderContext);
                    if ( pColFrame->Lower()->GetNext() )
                    {
                        pColFrame->Lower()->GetNext()->Calc(pRenderContext);
                    }

                    pColFrame = static_cast<SwColumnFrame*>(pColFrame->GetNext());
                }

                // unlock position of lower floating screen objects for the extra format
                // #i81555#
                // Section frame can already have changed the page and its content
                // can still be on the former page.
                // Thus, initialize objects via lower-relationship
                InitObjs( *mpSectFrame );

                // format content - first with collecting its foot-/endnotes before content
                // format, second without collecting its foot-/endnotes.
                ::CalcContent( mpSectFrame );
                ::CalcContent( mpSectFrame, true );

                // keep locked position of lower floating screen objects
                SwPageFrame* pPageFrame = mpSectFrame->FindPageFrame();
                SwSortedObjs* pObjs = pPageFrame ? pPageFrame->GetSortedObjs() : nullptr;
                if ( pObjs )
                {
                    for ( size_t i = 0; i < pObjs->size(); ++i )
                    {
                        SwAnchoredObject* pAnchoredObj = (*pObjs)[i];

                        if ( mpSectFrame->IsAnLower( pAnchoredObj->GetAnchorFrame() ) )
                        {
                            pAnchoredObj->SetKeepPosLocked( true );
                        }
                    }
                }

                mbExtraFormatPerformed = true;
            }
        }
};

/// "formats" the frame; Frame and PrtArea
void SwSectionFrame::Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttr )
{
    if( !m_pSection ) // via DelEmpty
    {
#ifdef DBG_UTIL
        OSL_ENSURE( getRootFrame()->IsInDelList( this ), "SectionFrame without Section" );
#endif
        mbValidSize = mbValidPos = mbValidPrtArea = true;
        return;
    }
    SWRECTFN( this )
    if ( !mbValidPrtArea )
    {
        PROTOCOL( this, PROT_PRTAREA, 0, nullptr )
        mbValidPrtArea = true;
        SwTwips nUpper = CalcUpperSpace();

        // #109700# LRSpace for sections
        const SvxLRSpaceItem& rLRSpace = GetFormat()->GetLRSpace();
        (this->*fnRect->fnSetXMargins)( rLRSpace.GetLeft(), rLRSpace.GetRight() );

        if( nUpper != (this->*fnRect->fnGetTopMargin)() )
        {
            mbValidSize = false;
            SwFrame* pOwn = ContainsAny();
            if( pOwn )
                pOwn->_InvalidatePos();
        }
        (this->*fnRect->fnSetYMargins)( nUpper, 0 );
    }

    if ( !mbValidSize )
    {
        PROTOCOL_ENTER( this, PROT_SIZE, 0, nullptr )
        const long nOldHeight = (Frame().*fnRect->fnGetHeight)();
        bool bOldLock = IsColLocked();
        ColLock();

        mbValidSize = true;

        // The size is only determined by the content, if the SectFrame does not have a
        // Follow. Otherwise it fills (occupies) the Upper down to the lower edge.
        // It is not responsible for the text flow, but the content is.
        bool bMaximize = ToMaximize( false );

        // OD 2004-05-17 #i28701# - If the wrapping style has to be considered
        // on object positioning, an extra formatting has to be performed
        // to determine the correct positions the floating screen objects.
        // #i40147#
        // use new helper class <ExtraFormatToPositionObjs>.
        // This class additionally keep the locked position of the objects
        // and releases this position lock keeping on destruction.
        ExtraFormatToPositionObjs aExtraFormatToPosObjs( *this );
        if ( !bMaximize &&
             GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) &&
             !GetFormat()->GetBalancedColumns().GetValue() )
        {
            aExtraFormatToPosObjs.FormatSectionToPositionObjs();
        }

        // Column widths have to be adjusted before calling _CheckClipping.
        // _CheckClipping can cause the formatting of the lower frames
        // which still have a width of 0.
        const bool bHasColumns = Lower() && Lower()->IsColumnFrame();
        if ( bHasColumns && Lower()->GetNext() )
            AdjustColumns( nullptr, false );

        if( GetUpper() )
        {
            long nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            (maFrame.*fnRect->fnSetWidth)( nWidth );

            // #109700# LRSpace for sections
            const SvxLRSpaceItem& rLRSpace = GetFormat()->GetLRSpace();
            (maPrt.*fnRect->fnSetWidth)( nWidth - rLRSpace.GetLeft() -
                                        rLRSpace.GetRight() );

            // OD 15.10.2002 #103517# - allow grow in online layout
            // Thus, set <..IsBrowseMode()> as parameter <bGrow> on calling
            // method <_CheckClipping(..)>.
            const SwViewShell *pSh = getRootFrame()->GetCurrShell();
            _CheckClipping( pSh && pSh->GetViewOptions()->getBrowseMode(), bMaximize );
            bMaximize = ToMaximize( false );
            mbValidSize = true;
        }

        // Check the width of the columns and adjust if necessary
        if ( bHasColumns && ! Lower()->GetNext() && bMaximize )
            static_cast<SwColumnFrame*>(Lower())->Lower()->Calc(pRenderContext);

        if ( !bMaximize )
        {
            SwTwips nRemaining = (this->*fnRect->fnGetTopMargin)();
            SwFrame *pFrame = m_pLower;
            if( pFrame )
            {
                if( pFrame->IsColumnFrame() && pFrame->GetNext() )
                {
                    // #i61435#
                    // suppress formatting, if upper frame has height <= 0
                    if ( (GetUpper()->Frame().*fnRect->fnGetHeight)() > 0 )
                    {
                        FormatWidthCols( *pAttr, nRemaining, MINLAY );
                    }
                    // #126020# - adjust check for empty section
                    // #130797# - correct fix #126020#
                    while( HasFollow() && !GetFollow()->ContainsContent() &&
                           !GetFollow()->ContainsAny( true ) )
                    {
                        SwFrame* pOld = GetFollow();
                        GetFollow()->DelEmpty( false );
                        if( pOld == GetFollow() )
                            break;
                    }
                    bMaximize = ToMaximize( false );
                    nRemaining += (pFrame->Frame().*fnRect->fnGetHeight)();
                }
                else
                {
                    if( pFrame->IsColumnFrame() )
                    {
                        pFrame->Calc(pRenderContext);
                        pFrame = static_cast<SwColumnFrame*>(pFrame)->Lower();
                        pFrame->Calc(pRenderContext);
                        pFrame = static_cast<SwLayoutFrame*>(pFrame)->Lower();
                        CalcFootnoteContent();
                    }
                    // If we are in a columned frame which calls a CalcContent
                    // in the FormatWidthCols, the content might need calculating
                    if( pFrame && !pFrame->IsValid() && IsInFly() &&
                        FindFlyFrame()->IsColLocked() )
                        ::CalcContent( this );
                    nRemaining += InnerHeight();
                    bMaximize = HasFollow();
                }
            }

            SwTwips nDiff = (Frame().*fnRect->fnGetHeight)() - nRemaining;
            if( nDiff < 0)
            {
                SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
                {
                    long nBottom = (Frame().*fnRect->fnGetBottom)();
                    nBottom = (*fnRect->fnYInc)( nBottom, -nDiff );
                    long nTmpDiff = (*fnRect->fnYDiff)( nBottom, nDeadLine );
                    if( nTmpDiff > 0 )
                    {
                        nTmpDiff = GetUpper()->Grow( nTmpDiff, true );
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
                long nTmp = nRemaining - (Frame().*fnRect->fnGetHeight)();
                long nTop = (this->*fnRect->fnGetTopMargin)();
                (Frame().*fnRect->fnAddBottom)( nTmp );
                (this->*fnRect->fnSetYMargins)( nTop, 0 );
                InvalidateNextPos();
                if (m_pLower && (!m_pLower->IsColumnFrame() || !m_pLower->GetNext()))
                {
                    // If a single-column section just created the space that
                    // was requested by the "undersized" paragraphs, then they
                    // have to be invalidated and calculated, so they fully cover it
                    pFrame = m_pLower;
                    if( pFrame->IsColumnFrame() )
                    {
                        pFrame->_InvalidateSize();
                        pFrame->_InvalidatePos();
                        pFrame->Calc(pRenderContext);
                        pFrame = static_cast<SwColumnFrame*>(pFrame)->Lower();
                        pFrame->Calc(pRenderContext);
                        pFrame = static_cast<SwLayoutFrame*>(pFrame)->Lower();
                        CalcFootnoteContent();
                    }
                    bool bUnderSz = false;
                    while( pFrame )
                    {
                        if( pFrame->IsTextFrame() && static_cast<SwTextFrame*>(pFrame)->IsUndersized() )
                        {
                            pFrame->Prepare( PREP_ADJUST_FRM );
                            bUnderSz = true;
                        }
                        pFrame = pFrame->GetNext();
                    }
                    if( bUnderSz && !IsContentLocked() )
                        ::CalcContent( this );
                }
            }
        }

        // Do not exceed the lower edge of the Upper.
        // Do not extend below the lower edge with Sections with Follows
        if ( GetUpper() )
            _CheckClipping( true, bMaximize );
        if( !bOldLock )
            ColUnlock();
        long nDiff = nOldHeight - (Frame().*fnRect->fnGetHeight)();
        if( nDiff > 0 )
        {
            if( !GetNext() )
                SetRetouche(); // Take over the retouching ourselves
            if( GetUpper() && !GetUpper()->IsFooterFrame() )
                GetUpper()->Shrink( nDiff );
        }
        if( IsUndersized() )
            mbValidPrtArea = true;
    }
}

/// Returns the next layout sheet where the frame can be moved in.
/// New pages are created only if specified by the parameter.
SwLayoutFrame *SwFrame::GetNextSctLeaf( MakePageType eMakePage )
{
    // Attention: Nested sections are currently not supported

    PROTOCOL_ENTER( this, PROT_LEAF, ACT_NEXT_SECT, GetUpper()->FindSctFrame() )

    // Shortcuts for "columned" sections, if we're not in the last column
    // Can we slide to the next column of the section?
    if( IsColBodyFrame() && GetUpper()->GetNext() )
        return static_cast<SwLayoutFrame*>(static_cast<SwLayoutFrame*>(GetUpper()->GetNext())->Lower());
    if( GetUpper()->IsColBodyFrame() && GetUpper()->GetUpper()->GetNext() )
        return static_cast<SwLayoutFrame*>(static_cast<SwLayoutFrame*>(GetUpper()->GetUpper()->GetNext())->Lower());
    // Inside a section, in tables, or sections of headers/footers, there can be only
    // one column shift be made, one of the above shortcuts should have applied!
    if( GetUpper()->IsInTab() || FindFooterOrHeader() )
        return nullptr;

    SwSectionFrame *pSect = FindSctFrame();
    bool bWrongPage = false;
    OSL_ENSURE( pSect, "GetNextSctLeaf: Missing SectionFrame" );

    // Shortcut for sections with Follows. That's ok,
    // if no columns or pages (except dummy pages) lie in between.
    // In case of linked frames and in footnotes the shortcut would get
    // even more costly
    if( pSect->HasFollow() && pSect->IsInDocBody() )
    {
        if( pSect->GetFollow() == pSect->GetNext() )
        {
            SwPageFrame *pPg = pSect->GetFollow()->FindPageFrame();
            if( WrongPageDesc( pPg ) )
                bWrongPage = true;
            else
                return FIRSTLEAF( pSect->GetFollow() );
        }
        else
        {
            SwFrame* pTmp;
            if( !pSect->GetUpper()->IsColBodyFrame() ||
                nullptr == ( pTmp = pSect->GetUpper()->GetUpper()->GetNext() ) )
                pTmp = pSect->FindPageFrame()->GetNext();
            if( pTmp ) // is now the next column or page
            {
                SwFrame* pTmpX = pTmp;
                if( pTmp->IsPageFrame() && static_cast<SwPageFrame*>(pTmp)->IsEmptyPage() )
                    pTmp = pTmp->GetNext(); // skip dummy pages
                SwFrame *pUp = pSect->GetFollow()->GetUpper();
                // pUp becomes the next column if the Follow lies in a column
                // that is not a "not first" one, otherwise the page
                if( !pUp->IsColBodyFrame() ||
                    !( pUp = pUp->GetUpper() )->GetPrev() )
                    pUp = pUp->FindPageFrame();
                // Now pUp and pTmp have to be the same page/column, otherwise
                // pages or columns lie between Master and Follow
                if( pUp == pTmp || pUp->GetNext() == pTmpX )
                {
                    SwPageFrame* pNxtPg = pUp->IsPageFrame() ?
                                        static_cast<SwPageFrame*>(pUp) : pUp->FindPageFrame();
                    if( WrongPageDesc( pNxtPg ) )
                        bWrongPage = true;
                    else
                        return FIRSTLEAF( pSect->GetFollow() );
                }
            }
        }
    }

    // Always end up in the same section: Body again inside Body etc.
    const bool bBody = IsInDocBody();
    const bool bFootnotePage = FindPageFrame()->IsFootnotePage();

    SwLayoutFrame *pLayLeaf;
    // A shortcut for TabFrames such that not all cells need to be visited
    if( bWrongPage )
        pLayLeaf = nullptr;
    else if( IsTabFrame() )
    {
        SwContentFrame* pTmpCnt = static_cast<SwTabFrame*>(this)->FindLastContent();
        pLayLeaf = pTmpCnt ? pTmpCnt->GetUpper() : nullptr;
    }
    else
    {
        pLayLeaf = GetNextLayoutLeaf();
        if( IsColumnFrame() )
        {
            while( pLayLeaf && static_cast<SwColumnFrame*>(this)->IsAnLower( pLayLeaf ) )
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        }
    }

    SwLayoutFrame *pOldLayLeaf = nullptr;           // Such that in case of newly
                                            // created pages, the search is
                                            // not started over at the beginning

    while( true )
    {
        if( pLayLeaf )
        {
            // A layout leaf was found, let's see whether it can store me or
            // another SectionFrame can be inserted here, or we have to continue
            // searching
            SwPageFrame* pNxtPg = pLayLeaf->FindPageFrame();
            if ( !bFootnotePage && pNxtPg->IsFootnotePage() )
            {   // If I reached the end note pages it's over
                pLayLeaf = nullptr;
                continue;
            }
            // Once inBody always inBody, don't step into tables and not into other sections
            if ( (bBody && !pLayLeaf->IsInDocBody()) ||
                 (IsInFootnote() != pLayLeaf->IsInFootnote() ) ||
                 pLayLeaf->IsInTab() ||
                 ( pLayLeaf->IsInSct() && ( !pSect->HasFollow()
                   || pSect->GetFollow() != pLayLeaf->FindSctFrame() ) ) )
            {
                // Rejected - try again.
                pOldLayLeaf = pLayLeaf;
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
                continue;
            }
            if( WrongPageDesc( pNxtPg ) )
            {
                if( bWrongPage )
                    break; // there's a column between me and my right page
                pLayLeaf = nullptr;
                bWrongPage = true;
                pOldLayLeaf = nullptr;
                continue;
            }
        }
        // There is no further LayoutFrame that fits, so a new page
        // has to be created, although new pages are worthless within a frame
        else if( !pSect->IsInFly() &&
            ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT ) )
        {
            InsertPage(pOldLayLeaf ? pOldLayLeaf->FindPageFrame() : FindPageFrame(),
                       false );
            // and again the whole thing
            pLayLeaf = pOldLayLeaf ? pOldLayLeaf : GetNextLayoutLeaf();
            continue;
        }
        break;
    }

    if( pLayLeaf )
    {
        // We have found the suitable layout sheet. If there (in the sheet) is
        // already a Follow of our section, we take its first layout sheet,
        // otherwise it is time to create a section follow
        SwSectionFrame* pNew;

        // This can be omitted if existing Follows were cut short
        SwFrame* pFirst = pLayLeaf->Lower();
        // Here SectionFrames that are to be deleted must be ignored
        while( pFirst && pFirst->IsSctFrame() && !static_cast<SwSectionFrame*>(pFirst)->GetSection() )
            pFirst = pFirst->GetNext();
        if( pFirst && pFirst->IsSctFrame() && pSect->GetFollow() == pFirst )
            pNew = pSect->GetFollow();
        else if( MAKEPAGE_NOSECTION == eMakePage )
            return pLayLeaf;
        else
        {
            pNew = new SwSectionFrame( *pSect, false );
            pNew->InsertBefore( pLayLeaf, pLayLeaf->Lower() );
            pNew->Init();
            SWRECTFN( pNew )
            (pNew->*fnRect->fnMakePos)( pLayLeaf, nullptr, true );

            // If our section frame has a successor then that has to be
            // moved behind the new Follow of the section frames
            SwFrame* pTmp = pSect->GetNext();
            if( pTmp && pTmp != pSect->GetFollow() )
            {
                SwFlowFrame* pNxt;
                SwContentFrame* pNxtContent = nullptr;
                if( pTmp->IsContentFrame() )
                {
                    pNxt = static_cast<SwContentFrame*>(pTmp);
                    pNxtContent = static_cast<SwContentFrame*>(pTmp);
                }
                else
                {
                    pNxtContent = static_cast<SwLayoutFrame*>(pTmp)->ContainsContent();
                    if( pTmp->IsSctFrame() )
                        pNxt = static_cast<SwSectionFrame*>(pTmp);
                    else
                    {
                        OSL_ENSURE( pTmp->IsTabFrame(), "GetNextSctLeaf: Wrong Type" );
                        pNxt = static_cast<SwTabFrame*>(pTmp);
                    }
                    while( !pNxtContent && nullptr != ( pTmp = pTmp->GetNext() ) )
                    {
                        if( pTmp->IsContentFrame() )
                            pNxtContent = static_cast<SwContentFrame*>(pTmp);
                        else
                            pNxtContent = static_cast<SwLayoutFrame*>(pTmp)->ContainsContent();
                    }
                }
                if( pNxtContent )
                {
                    SwFootnoteBossFrame* pOldBoss = pSect->FindFootnoteBossFrame( true );
                    if( pOldBoss == pNxtContent->FindFootnoteBossFrame( true ) )
                    {
                        SwSaveFootnoteHeight aHeight( pOldBoss,
                            pOldBoss->Frame().Top() + pOldBoss->Frame().Height() );
                        pSect->GetUpper()->MoveLowerFootnotes( pNxtContent, pOldBoss,
                                    pLayLeaf->FindFootnoteBossFrame( true ), false );
                    }
                }
                pNxt->MoveSubTree( pLayLeaf, pNew->GetNext() );
            }
            if( pNew->GetFollow() )
                pNew->SimpleFormat();
        }
        // The wanted layout sheet is now the first of the determined SctFrames:
        pLayLeaf = FIRSTLEAF( pNew );
    }
    return pLayLeaf;
}

/// Returns the preceding layout sheet where the frame can be moved into
SwLayoutFrame *SwFrame::GetPrevSctLeaf( MakePageType )
{
    PROTOCOL_ENTER( this, PROT_LEAF, ACT_PREV_SECT, GetUpper()->FindSctFrame() )

    SwLayoutFrame* pCol;
    // ColumnFrame always contain a BodyFrame now
    if( IsColBodyFrame() )
        pCol = GetUpper();
    else if( GetUpper()->IsColBodyFrame() )
        pCol = GetUpper()->GetUpper();
    else
        pCol = nullptr;
    bool bJump = false;
    if( pCol )
    {
        if( pCol->GetPrev() )
        {
            do
            {
                pCol = static_cast<SwLayoutFrame*>(pCol->GetPrev());
                // Is there any content?
                if( static_cast<SwLayoutFrame*>(pCol->Lower())->Lower() )
                {
                    if( bJump )     // Did we skip a blank page?
                        SwFlowFrame::SetMoveBwdJump( true );
                    return static_cast<SwLayoutFrame*>(pCol->Lower());  // The columnm body
                }
                bJump = true;
            } while( pCol->GetPrev() );

            // We get here when all columns are empty, pCol is now the
            // first column, we need the body though
            pCol = static_cast<SwLayoutFrame*>(pCol->Lower());
        }
        else
            pCol = nullptr;
    }

    if( bJump )     // Did we skip a blank page?
        SwFlowFrame::SetMoveBwdJump( true );

    // Within sections in tables or section in headers/footers there can
    // be only one column change be made, one of the above shortcuts should
    // have applied, also when the section has a pPrev.
    // Now we even consider an empty column...
    OSL_ENSURE( FindSctFrame(), "GetNextSctLeaf: Missing SectionFrame" );
    if( ( IsInTab() && !IsTabFrame() ) || FindFooterOrHeader() )
        return pCol;

    // === IMPORTANT ===
    // Precondition, which needs to be hold, is that the <this> frame can be
    // inside a table, but then the found section frame <pSect> is also inside
    // this table.
    SwSectionFrame *pSect = FindSctFrame();

    // #i95698#
    // A table cell containing directly a section does not break - see lcl_FindSectionsInRow(..)
    // Thus, a table inside a section, which is inside another table can only
    // flow backward in the columns of its section.
    // Note: The table cell, which contains the section, can not have a master table cell.
    if ( IsTabFrame() && pSect->IsInTab() )
    {
        return pCol;
    }

    {
        SwFrame *pPrv;
        if( nullptr != ( pPrv = pSect->GetIndPrev() ) )
        {
            // Mooching, half dead SectionFrames shouldn't confuse us
            while( pPrv && pPrv->IsSctFrame() && !static_cast<SwSectionFrame*>(pPrv)->GetSection() )
                pPrv = pPrv->GetPrev();
            if( pPrv )
                return pCol;
        }
    }

    const bool bBody = IsInDocBody();
    const bool bFly  = IsInFly();

    SwLayoutFrame *pLayLeaf = GetPrevLayoutLeaf();
    SwLayoutFrame *pPrevLeaf = nullptr;

    while ( pLayLeaf )
    {
        // Never step into tables or sections
        if ( pLayLeaf->IsInTab() || pLayLeaf->IsInSct() )
        {
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
        }
        else if ( bBody && pLayLeaf->IsInDocBody() )
        {
            // If there is a pLayLeaf has a lower pLayLeaf is the frame we are looking for.
            // Exception: pLayLeaf->Lower() is a zombie section frame
            const SwFrame* pTmp = pLayLeaf->Lower();
            // OD 11.04.2003 #108824# - consider, that the zombie section frame
            // can have frame below it in the found layout leaf.
            // Thus, skipping zombie section frame, if possible.
            while ( pTmp && pTmp->IsSctFrame() &&
                    !( static_cast<const SwSectionFrame*>(pTmp)->GetSection() ) &&
                    pTmp->GetNext()
                  )
            {
                pTmp = pTmp->GetNext();
            }
            if ( pTmp &&
                 ( !pTmp->IsSctFrame() ||
                   ( static_cast<const SwSectionFrame*>(pTmp)->GetSection() )
                 )
               )
            {
                break;
            }
            pPrevLeaf = pLayLeaf;
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
            if ( pLayLeaf )
                SwFlowFrame::SetMoveBwdJump( true );
        }
        else if ( bFly )
            break;  // Contents in Flys every layout sheet should be right. Why?
        else
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
    }
    if( !pLayLeaf )
    {
        if( !pPrevLeaf )
            return pCol;
        pLayLeaf = pPrevLeaf;
    }

    SwSectionFrame* pNew = nullptr;
    // At first go to the end of the layout sheet
    SwFrame *pTmp = pLayLeaf->Lower();
    if( pTmp )
    {
        while( pTmp->GetNext() )
            pTmp = pTmp->GetNext();
        if( pTmp->IsSctFrame() )
        {
            // Half dead ones only interfere here
            while( !static_cast<SwSectionFrame*>(pTmp)->GetSection() && pTmp->GetPrev() &&
                    pTmp->GetPrev()->IsSctFrame() )
                pTmp = pTmp->GetPrev();
            if( static_cast<SwSectionFrame*>(pTmp)->GetFollow() == pSect )
                pNew = static_cast<SwSectionFrame*>(pTmp);
        }
    }
    if( !pNew )
    {
        pNew = new SwSectionFrame( *pSect, true );
        pNew->InsertBefore( pLayLeaf, nullptr );
        pNew->Init();
        SWRECTFN( pNew )
        (pNew->*fnRect->fnMakePos)( pLayLeaf, pNew->GetPrev(), true );

        pLayLeaf = FIRSTLEAF( pNew );
        if( !pNew->Lower() )    // Format single column sections
        {
            pNew->MakePos();
            pLayLeaf->Format(getRootFrame()->GetCurrShell()->GetOut()); // In order that the PrtArea is correct for the MoveBwd
        }
        else
            pNew->SimpleFormat();
    }
    else
    {
        pLayLeaf = FIRSTLEAF( pNew );
        if( pLayLeaf->IsColBodyFrame() )
        {
            // In existent section columns we're looking for the last not empty
            // column.
            SwLayoutFrame *pTmpLay = pLayLeaf;
            while( pLayLeaf->GetUpper()->GetNext() )
            {
                pLayLeaf = static_cast<SwLayoutFrame*>(static_cast<SwLayoutFrame*>(pLayLeaf->GetUpper()->GetNext())->Lower());
                if( pLayLeaf->Lower() )
                    pTmpLay = pLayLeaf;
            }
            // If we skipped an empty column, we've to set the jump-flag
            if( pLayLeaf != pTmpLay )
            {
                pLayLeaf = pTmpLay;
                SwFlowFrame::SetMoveBwdJump( true );
            }
        }
    }
    return pLayLeaf;
}

static SwTwips lcl_DeadLine( const SwFrame* pFrame )
{
    const SwLayoutFrame* pUp = pFrame->GetUpper();
    while( pUp && pUp->IsInSct() )
    {
        if( pUp->IsSctFrame() )
            pUp = pUp->GetUpper();
        // Columns now with BodyFrame
        else if( pUp->IsColBodyFrame() && pUp->GetUpper()->GetUpper()->IsSctFrame() )
            pUp = pUp->GetUpper()->GetUpper();
        else
            break;
    }
    SWRECTFN( pFrame )
    return pUp ? (pUp->*fnRect->fnGetPrtBottom)() :
                 (pFrame->Frame().*fnRect->fnGetBottom)();
}

/// checks whether the SectionFrame is still able to grow, as case may be the environment has to be asked
bool SwSectionFrame::Growable() const
{
    SWRECTFN( this )
    if( (*fnRect->fnYDiff)( lcl_DeadLine( this ),
        (Frame().*fnRect->fnGetBottom)() ) > 0 )
        return true;

    return ( GetUpper() && const_cast<SwFrame*>(static_cast<SwFrame const *>(GetUpper()))->Grow( LONG_MAX, true ) );
}

SwTwips SwSectionFrame::_Grow( SwTwips nDist, bool bTst )
{
    if ( !IsColLocked() && !HasFixSize() )
    {
        SWRECTFN( this )
        long nFrameHeight = (Frame().*fnRect->fnGetHeight)();
        if( nFrameHeight > 0 && nDist > (LONG_MAX - nFrameHeight) )
            nDist = LONG_MAX - nFrameHeight;

        if ( nDist <= 0L )
            return 0L;

        bool bInCalcContent = GetUpper() && IsInFly() && FindFlyFrame()->IsLocked();
        // OD 2004-03-15 #116561# - allow grow in online layout
        bool bGrow = !Lower() || !Lower()->IsColumnFrame() || !Lower()->GetNext() ||
             GetSection()->GetFormat()->GetBalancedColumns().GetValue();
        if( !bGrow )
        {
             const SwViewShell *pSh = getRootFrame()->GetCurrShell();
             bGrow = pSh && pSh->GetViewOptions()->getBrowseMode();
        }
        if( bGrow )
        {
            SwTwips nGrow;
            if( IsInFootnote() )
                nGrow = 0;
            else
            {
                nGrow = lcl_DeadLine( this );
                nGrow = (*fnRect->fnYDiff)( nGrow,
                                           (Frame().*fnRect->fnGetBottom)() );
            }
            SwTwips nSpace = nGrow;
            if( !bInCalcContent && nGrow < nDist && GetUpper() )
                nGrow += GetUpper()->Grow( LONG_MAX, true );

            if( nGrow > nDist )
                nGrow = nDist;
            if( nGrow <= 0 )
            {
                nGrow = 0;
                if( nDist && !bTst )
                {
                    if( bInCalcContent )
                        _InvalidateSize();
                    else
                        InvalidateSize();
                }
            }
            else if( !bTst )
            {
                if( bInCalcContent )
                    _InvalidateSize();
                else if( nSpace < nGrow &&  nDist != nSpace + GetUpper()->
                         Grow( nGrow - nSpace ) )
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
                    if( GetUpper() && GetUpper()->IsHeaderFrame() )
                        GetUpper()->InvalidateSize();
                }
                (Frame().*fnRect->fnAddBottom)( nGrow );
                long nPrtHeight = (Prt().*fnRect->fnGetHeight)() + nGrow;
                (Prt().*fnRect->fnSetHeight)( nPrtHeight );

                if( Lower() && Lower()->IsColumnFrame() && Lower()->GetNext() )
                {
                    SwFrame* pTmp = Lower();
                    do
                    {
                        pTmp->_InvalidateSize();
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                    _InvalidateSize();
                }
                if( GetNext() )
                {
                    SwFrame *pFrame = GetNext();
                    while( pFrame && pFrame->IsSctFrame() && !static_cast<SwSectionFrame*>(pFrame)->GetSection() )
                        pFrame = pFrame->GetNext();
                    if( pFrame )
                    {
                        if( bInCalcContent )
                            pFrame->_InvalidatePos();
                        else
                            pFrame->InvalidatePos();
                    }
                }
                // #i28701# - Due to the new object positioning
                // the frame on the next page/column can flow backward (e.g. it
                // was moved forward due to the positioning of its objects ).
                // Thus, invalivate this next frame, if document compatibility
                // option 'Consider wrapping style influence on object positioning' is ON.
                else if ( GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) )
                {
                    InvalidateNextPos();
                }
            }
            return nGrow;
        }
        if ( !bTst )
        {
            if( bInCalcContent )
                _InvalidateSize();
            else
                InvalidateSize();
        }
    }
    return 0L;
}

SwTwips SwSectionFrame::_Shrink( SwTwips nDist, bool bTst )
{
    if ( Lower() && !IsColLocked() && !HasFixSize() )
    {
        if( ToMaximize( false ) )
        {
            if( !bTst )
                InvalidateSize();
        }
        else
        {
            SWRECTFN( this )
            long nFrameHeight = (Frame().*fnRect->fnGetHeight)();
            if ( nDist > nFrameHeight )
                nDist = nFrameHeight;

            if ( Lower()->IsColumnFrame() && Lower()->GetNext() && // FootnoteAtEnd
                 !GetSection()->GetFormat()->GetBalancedColumns().GetValue() )
            {   // With column bases the format takes over the control of the
                // growth (because of the balance)
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
                (Frame().*fnRect->fnAddBottom)( -nDist );
                long nPrtHeight = (Prt().*fnRect->fnGetHeight)() - nDist;
                (Prt().*fnRect->fnSetHeight)( nPrtHeight );

                // We do not allow a section frame to shrink the its upper
                // footer frame. This is because in the calculation of a
                // footer frame, the content of the section frame is _not_
                // calculated. If there is a fly frame overlapping with the
                // footer frame, the section frame is not affected by this
                // during the calculation of the footer frame size.
                // The footer frame does not grow in its FormatSize function
                // but during the calculation of the content of the section
                // frame. The section frame grows until some of its text is
                // located on top of the fly frame. The next call of CalcContent
                // tries to shrink the section and here it would also shrink
                // the footer. This may not happen, because shrinking the footer
                // would cause the top of the section frame to overlap with the
                // fly frame again, this would result in a perfect loop.
                if( GetUpper() && !GetUpper()->IsFooterFrame() )
                    GetUpper()->Shrink( nDist, bTst );

                if( Lower() && Lower()->IsColumnFrame() && Lower()->GetNext() )
                {
                    SwFrame* pTmp = Lower();
                    do
                    {
                        pTmp->_InvalidateSize();
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                }
                if( GetNext() )
                {
                    SwFrame* pFrame = GetNext();
                    while( pFrame && pFrame->IsSctFrame() && !static_cast<SwSectionFrame*>(pFrame)->GetSection() )
                        pFrame = pFrame->GetNext();
                    if( pFrame )
                        pFrame->InvalidatePos();
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
|*  When are Frames within a SectionFrames moveable?
|*  If they are not in the last column of a SectionFrames yet,
|*  if there is no Follow,
|*  if the SectionFrame cannot grow anymore, then it gets more complicated,
|*  in that case it depends on whether the SectionFrame can find a next
|*  layout sheet. In (column based/chained) Flys this is checked via
|*  GetNextLayout, in tables and headers/footers there is none, however in the
|*  DocBody and in foot notes there is always one.
|*
|*  This routine is used in the TextFormatter to decided whether it's allowed to
|*  create a (paragraph-)Follow or whether the paragraph has to stick together
|*/
bool SwSectionFrame::MoveAllowed( const SwFrame* pFrame) const
{
    // Is there a Follow or is the Frame not in the last column?
    if( HasFollow() || ( pFrame->GetUpper()->IsColBodyFrame() &&
        pFrame->GetUpper()->GetUpper()->GetNext() ) )
        return true;
    if( pFrame->IsInFootnote() )
    {
        if( IsInFootnote() )
        {
            if( GetUpper()->IsInSct() )
            {
                if( Growable() )
                    return false;
                return GetUpper()->FindSctFrame()->MoveAllowed( this );
            }
            else
                return true;
        }
        // The content of footnote inside a columned sectionfrm is moveable
        // except in the last column
        const SwLayoutFrame *pLay = pFrame->FindFootnoteFrame()->GetUpper()->GetUpper();
        if( pLay->IsColumnFrame() && pLay->GetNext() )
        {
            // The first paragraph in the first footnote in the first column
            // in the sectionfrm at the top of the page is not moveable,
            // if the columnbody is empty.
            bool bRet = false;
            if( pLay->GetIndPrev() || pFrame->GetIndPrev() ||
                pFrame->FindFootnoteFrame()->GetPrev() )
                bRet = true;
            else
            {
                const SwLayoutFrame* pBody = static_cast<const SwColumnFrame*>(pLay)->FindBodyCont();
                if( pBody && pBody->Lower() )
                    bRet = true;
            }
            if( bRet && ( IsFootnoteAtEnd() || !Growable() ) )
                return true;
        }
    }
    // Or can the section still grow?
    if( !IsColLocked() && Growable() )
        return false;
    // Now it has to be examined whether there is a layout sheet wherein
    // a section Follow can be created
    if( IsInTab() || ( !IsInDocBody() && FindFooterOrHeader() ) )
        return false; // It doesn't work in tables/headers/footers
    if( IsInFly() ) // In column based or chained frames
        return nullptr != const_cast<SwFrame*>(static_cast<SwFrame const *>(GetUpper()))->GetNextLeaf( MAKEPAGE_NONE );
    return true;
}

/** Called for a frame inside a section with no direct previous frame (or only
    previous empty section frames) the previous frame of the outer section is
    returned, if the frame is the first flowing content of this section.

    Note: For a frame inside a table frame, which is inside a section frame,
          NULL is returned.
*/
SwFrame* SwFrame::_GetIndPrev() const
{
    SwFrame *pRet = nullptr;
    // #i79774#
    // Do not assert, if the frame has a direct previous frame, because it
    // could be an empty section frame. The caller has to assure, that the
    // frame has no direct previous frame or only empty section frames as
    // previous frames.
    OSL_ENSURE( /*!pPrev &&*/ IsInSct(), "Why?" );
    const SwFrame* pSct = GetUpper();
    if( !pSct )
        return nullptr;
    if( pSct->IsSctFrame() )
        pRet = pSct->GetIndPrev();
    else if( pSct->IsColBodyFrame() && (pSct = pSct->GetUpper()->GetUpper())->IsSctFrame() )
    {
        // Do not return the previous frame of the outer section, if in one
        // of the previous columns is content.
        const SwFrame* pCol = GetUpper()->GetUpper()->GetPrev();
        while( pCol )
        {
            OSL_ENSURE( pCol->IsColumnFrame(), "GetIndPrev(): ColumnFrame expected" );
            OSL_ENSURE( pCol->GetLower() && pCol->GetLower()->IsBodyFrame(),
                    "GetIndPrev(): Where's the body?");
            if( static_cast<const SwLayoutFrame*>(static_cast<const SwLayoutFrame*>(pCol)->Lower())->Lower() )
                return nullptr;
            pCol = pCol->GetPrev();
        }
        pRet = pSct->GetIndPrev();
    }

    // skip empty section frames
    while( pRet && pRet->IsSctFrame() && !static_cast<SwSectionFrame*>(pRet)->GetSection() )
        pRet = pRet->GetIndPrev();
    return pRet;
}

SwFrame* SwFrame::_GetIndNext()
{
    OSL_ENSURE( !mpNext && IsInSct(), "Why?" );
    SwFrame* pSct = GetUpper();
    if( !pSct )
        return nullptr;
    if( pSct->IsSctFrame() )
        return pSct->GetIndNext();
    if( pSct->IsColBodyFrame() && (pSct = pSct->GetUpper()->GetUpper())->IsSctFrame() )
    {   // We can only return the successor of the SectionFrames if there is no
        // content in the successing columns
        SwFrame* pCol = GetUpper()->GetUpper()->GetNext();
        while( pCol )
        {
            OSL_ENSURE( pCol->IsColumnFrame(), "GetIndNext(): ColumnFrame expected" );
            OSL_ENSURE( pCol->GetLower() && pCol->GetLower()->IsBodyFrame(),
                    "GetIndNext(): Where's the body?");
            if( static_cast<SwLayoutFrame*>(static_cast<SwLayoutFrame*>(pCol)->Lower())->Lower() )
                return nullptr;
            pCol = pCol->GetNext();
        }
        return pSct->GetIndNext();
    }
    return nullptr;
}

bool SwSectionFrame::IsDescendantFrom( const SwSectionFormat* pFormat ) const
{
    if( !m_pSection || !pFormat )
        return false;
    const SwSectionFormat *pMyFormat = m_pSection->GetFormat();
    while( pFormat != pMyFormat )
    {
        if( dynamic_cast< const SwSectionFormat *>( pMyFormat->GetRegisteredIn()) !=  nullptr )
            pMyFormat = static_cast<const SwSectionFormat*>(pMyFormat->GetRegisteredIn());
        else
            return false;
    }
    return true;
}

void SwSectionFrame::CalcFootnoteAtEndFlag()
{
    SwSectionFormat *pFormat = GetSection()->GetFormat();
    sal_uInt16 nVal = pFormat->GetFootnoteAtTextEnd( false ).GetValue();
    m_bFootnoteAtEnd = FTNEND_ATPGORDOCEND != nVal;
    m_bOwnFootnoteNum = FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
                 FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
    while( !m_bFootnoteAtEnd && !m_bOwnFootnoteNum )
    {
        if( dynamic_cast< const SwSectionFormat *>( pFormat->GetRegisteredIn()) !=  nullptr )
            pFormat = static_cast<SwSectionFormat*>(pFormat->GetRegisteredIn());
        else
            break;
        nVal = pFormat->GetFootnoteAtTextEnd( false ).GetValue();
        if( FTNEND_ATPGORDOCEND != nVal )
        {
            m_bFootnoteAtEnd = true;
            m_bOwnFootnoteNum = m_bOwnFootnoteNum ||FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
                         FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
        }
    }
}

bool SwSectionFrame::IsEndnoteAtMyEnd() const
{
    return m_pSection->GetFormat()->GetEndAtTextEnd( false ).IsAtEnd();
}

void SwSectionFrame::CalcEndAtEndFlag()
{
    SwSectionFormat *pFormat = GetSection()->GetFormat();
    m_bEndnAtEnd = pFormat->GetEndAtTextEnd( false ).IsAtEnd();
    while( !m_bEndnAtEnd )
    {
        if( dynamic_cast< const SwSectionFormat *>( pFormat->GetRegisteredIn()) !=  nullptr )
            pFormat = static_cast<SwSectionFormat*>(pFormat->GetRegisteredIn());
        else
            break;
        m_bEndnAtEnd = pFormat->GetEndAtTextEnd( false ).IsAtEnd();
    }
}

void SwSectionFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_uInt8 nInvFlags = 0;

    if( pNew && RES_ATTRSET_CHG == pNew->Which() )
    {
        SfxItemIter aNIter( *static_cast<const SwAttrSetChg*>(pNew)->GetChgSet() );
        SfxItemIter aOIter( *static_cast<const SwAttrSetChg*>(pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *static_cast<const SwAttrSetChg*>(pOld) );
        SwAttrSetChg aNewSet( *static_cast<const SwAttrSetChg*>(pNew) );
        while( true )
        {
            _UpdateAttr( aOIter.GetCurItem(),
                         aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrame::Modify( &aOldSet, &aNewSet );
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

void SwSectionFrame::SwClientNotify( const SwModify& rMod, const SfxHint& rHint )
{
    SwClient::SwClientNotify(rMod, rHint);
    // #i117863#
    const SwSectionFrameMoveAndDeleteHint* pHint =
                    dynamic_cast<const SwSectionFrameMoveAndDeleteHint*>(&rHint);
    if ( pHint && pHint->GetId() == SFX_HINT_DYING && &rMod == GetRegisteredIn() )
    {
        SwSectionFrame::MoveContentAndDelete( this, pHint->IsSaveContent() );
    }
}

void SwSectionFrame::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                            sal_uInt8 &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {   // Suppress multi columns in foot notes
        case RES_FMT_CHG:
        {
            const SwFormatCol& rNewCol = GetFormat()->GetCol();
            if( !IsInFootnote() )
            {
                // Nasty case. When allocating a template we can not count
                // on the old column attribute. We're left with creating a
                // temporary attribute here.
                SwFormatCol aCol;
                if ( Lower() && Lower()->IsColumnFrame() )
                {
                    sal_uInt16 nCol = 0;
                    SwFrame *pTmp = Lower();
                    do
                    {   ++nCol;
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                    aCol.Init( nCol, 0, 1000 );
                }
                bool bChgFootnote = IsFootnoteAtEnd();
                bool const bChgEndn = IsEndnAtEnd();
                bool const bChgMyEndn = IsEndnoteAtMyEnd();
                CalcFootnoteAtEndFlag();
                CalcEndAtEndFlag();
                bChgFootnote = ( bChgFootnote != IsFootnoteAtEnd() ) ||
                          ( bChgEndn != IsEndnAtEnd() ) ||
                          ( bChgMyEndn != IsEndnoteAtMyEnd() );
                ChgColumns( aCol, rNewCol, bChgFootnote );
                rInvFlags |= 0x10;
            }
            rInvFlags |= 0x01;
            bClear = false;
        }
            break;

        case RES_COL:
            if( !IsInFootnote() )
            {
                assert(pOld && pNew);
                if (pOld && pNew)
                {
                    ChgColumns( *static_cast<const SwFormatCol*>(pOld), *static_cast<const SwFormatCol*>(pNew) );
                    rInvFlags |= 0x11;
                }
            }
            break;

        case RES_FTN_AT_TXTEND:
            if( !IsInFootnote() )
            {
                bool const bOld = IsFootnoteAtEnd();
                CalcFootnoteAtEndFlag();
                if (bOld != IsFootnoteAtEnd())
                {
                    const SwFormatCol& rNewCol = GetFormat()->GetCol();
                    ChgColumns( rNewCol, rNewCol, true );
                    rInvFlags |= 0x01;
                }
            }
            break;

        case RES_END_AT_TXTEND:
            if( !IsInFootnote() )
            {
                bool const bOld = IsEndnAtEnd();
                bool const bMyOld = IsEndnoteAtMyEnd();
                CalcEndAtEndFlag();
                if (bOld != IsEndnAtEnd() || bMyOld != IsEndnoteAtMyEnd())
                {
                    const SwFormatCol& rNewCol = GetFormat()->GetCol();
                    ChgColumns( rNewCol, rNewCol, true );
                    rInvFlags |= 0x01;
                }
            }
            break;
        case RES_COLUMNBALANCE:
            rInvFlags |= 0x01;
            break;

        case RES_FRAMEDIR :
            SetDerivedR2L( false );
            CheckDirChange();
            break;

        case RES_PROTECT:
            {
                SwViewShell *pSh = getRootFrame()->GetCurrShell();
                if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
                    pSh->Imp()->InvalidateAccessibleEditableState( true, this );
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
            SwLayoutFrame::Modify( pOld, pNew );
    }
}

/// A follow or a ftncontainer at the end of the page causes a maximal Size of the sectionframe.
bool SwSectionFrame::ToMaximize( bool bCheckFollow ) const
{
    if( HasFollow() )
    {
        if( !bCheckFollow ) // Don't check superfluous follows
            return true;
        const SwSectionFrame* pFoll = GetFollow();
        while( pFoll && pFoll->IsSuperfluous() )
            pFoll = pFoll->GetFollow();
        if( pFoll )
            return true;
    }
    if( IsFootnoteAtEnd() )
        return false;
    const SwFootnoteContFrame* pCont = ContainsFootnoteCont();
    if( !IsEndnAtEnd() )
        return nullptr != pCont;
    bool bRet = false;
    while( pCont && !bRet )
    {
        if( pCont->FindFootNote() )
            bRet = true;
        else
            pCont = ContainsFootnoteCont( pCont );
    }
    return bRet;
}

/// Check every Column for FootnoteContFrames.
SwFootnoteContFrame* SwSectionFrame::ContainsFootnoteCont( const SwFootnoteContFrame* pCont ) const
{
    SwFootnoteContFrame* pRet = nullptr;
    const SwLayoutFrame* pLay;
    if( pCont )
    {
        pLay = pCont->FindFootnoteBossFrame();
        OSL_ENSURE( IsAnLower( pLay ), "ConatainsFootnoteCont: Wrong FootnoteContainer" );
        pLay = static_cast<const SwLayoutFrame*>(pLay->GetNext());
    }
    else if( Lower() && Lower()->IsColumnFrame() )
        pLay = static_cast<const SwLayoutFrame*>(Lower());
    else
        pLay = nullptr;
    while ( !pRet && pLay )
    {
        if( pLay->Lower() && pLay->Lower()->GetNext() )
        {
            OSL_ENSURE( pLay->Lower()->GetNext()->IsFootnoteContFrame(),
                    "ToMaximize: Unexpected Frame" );
            pRet = const_cast<SwFootnoteContFrame*>(static_cast<const SwFootnoteContFrame*>(pLay->Lower()->GetNext()));
        }
        OSL_ENSURE( !pLay->GetNext() || pLay->GetNext()->IsLayoutFrame(),
                "ToMaximize: ColFrame expected" );
        pLay = static_cast<const SwLayoutFrame*>(pLay->GetNext());
    }
    return pRet;
}

void SwSectionFrame::InvalidateFootnotePos()
{
    SwFootnoteContFrame* pCont = ContainsFootnoteCont();
    if( pCont )
    {
        SwFrame *pTmp = pCont->ContainsContent();
        if( pTmp )
            pTmp->_InvalidatePos();
    }
}

SwTwips SwSectionFrame::CalcUndersize() const
{
    SWRECTFN(this);
    return InnerHeight() - (Prt().*fnRect->fnGetHeight)();
}

SwTwips SwSectionFrame::Undersize(bool bOverSize)
{
    const auto nRet = CalcUndersize();
    m_bUndersized = (nRet > 0);
    return (nRet <= 0 && !bOverSize) ? 0 : nRet;
}

void SwSectionFrame::CalcFootnoteContent()
{
    vcl::RenderContext* pRenderContext = getRootFrame()->GetCurrShell()->GetOut();
    SwFootnoteContFrame* pCont = ContainsFootnoteCont();
    if( pCont )
    {
        SwFrame* pFrame = pCont->ContainsAny();
        if( pFrame )
            pCont->Calc(pRenderContext);
        while( pFrame && IsAnLower( pFrame ) )
        {
            SwFootnoteFrame* pFootnote = pFrame->FindFootnoteFrame();
            if( pFootnote )
                pFootnote->Calc(pRenderContext);
            pFrame->Calc(pRenderContext);
            if( pFrame->IsSctFrame() )
            {
                SwFrame *pTmp = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
                if( pTmp )
                {
                    pFrame = pTmp;
                    continue;
                }
            }
            pFrame = pFrame->FindNext();
        }
    }
}

/*
 * If a SectionFrame gets empty, e.g. because its content changes the page/column,
 * it is not destroyed immediately (there could be a pointer left to it on the
 * stack), instead it puts itself in a list at the RootFrame, which is processed
 * later on (in Layaction::Action among others). Its size is set to Null and
 * the pointer to its page as well. Such SectionFrames that are to be deleted
 * must be ignored by the layout/during formatting.
 *
 * With InsertEmptySct the RootFrame stores a SectionFrame in the list,
 * with RemoveFromList it can be removed from the list (Dtor),
 * with DeleteEmptySct the list is processed and the SectionFrames are destroyed.
 */
void SwRootFrame::InsertEmptySct( SwSectionFrame* pDel )
{
    if( !mpDestroy )
        mpDestroy = new SwDestroyList;
    mpDestroy->insert( pDel );
}

void SwRootFrame::_DeleteEmptySct()
{
    OSL_ENSURE( mpDestroy, "Keine Liste, keine Kekse" );
    while( !mpDestroy->empty() )
    {
        SwSectionFrame* pSect = *mpDestroy->begin();
        mpDestroy->erase( mpDestroy->begin() );
        OSL_ENSURE( !pSect->IsColLocked() && !pSect->IsJoinLocked(),
                "DeleteEmptySct: Locked SectionFrame" );
        if( !pSect->Frame().HasArea() && !pSect->ContainsContent() )
        {
            SwLayoutFrame* pUp = pSect->GetUpper();
            pSect->RemoveFromLayout();
            SwFrame::DestroyFrame(pSect);
            if( pUp && !pUp->Lower() )
            {
                if( pUp->IsPageBodyFrame() )
                    pUp->getRootFrame()->SetSuperfluous();
                else if( pUp->IsFootnoteFrame() && !pUp->IsColLocked() &&
                    pUp->GetUpper() )
                {
                    pUp->Cut();
                    SwFrame::DestroyFrame(pUp);
                }
            }
        }
        else {
            OSL_ENSURE( pSect->GetSection(), "DeleteEmptySct: Halbtoter SectionFrame?!" );
        }
    }
}

void SwRootFrame::_RemoveFromList( SwSectionFrame* pSct )
{
    OSL_ENSURE( mpDestroy, "Where's my list?" );
    mpDestroy->erase( pSct );
}

#ifdef DBG_UTIL
bool SwRootFrame::IsInDelList( SwSectionFrame* pSct ) const
{
    return mpDestroy && mpDestroy->find( pSct ) != mpDestroy->end();
}
#endif

bool SwSectionFrame::IsBalancedSection() const
{
    bool bRet = false;
    if ( GetSection() && Lower() && Lower()->IsColumnFrame() && Lower()->GetNext() )
    {
        bRet = !GetSection()->GetFormat()->GetBalancedColumns().GetValue();
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
