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

SwSectionFrm::SwSectionFrm( SwSection &rSect, SwFrm* pSib )
    : SwLayoutFrm( rSect.GetFormat(), pSib )
    , SwFlowFrm( static_cast<SwFrm&>(*this) )
    , pSection( &rSect )
    , bFootnoteAtEnd(false)
    , bEndnAtEnd(false)
    , bContentLock(false)
    , bOwnFootnoteNum(false)
    , bFootnoteLock(false)
{
    mnFrmType = FRM_SECTION;

    CalcFootnoteAtEndFlag();
    CalcEndAtEndFlag();
}

SwSectionFrm::SwSectionFrm( SwSectionFrm &rSect, bool bMaster ) :
    SwLayoutFrm( rSect.GetFormat(), rSect.getRootFrm() ),
    SwFlowFrm( (SwFrm&)*this ),
    pSection( rSect.GetSection() ),
    bFootnoteAtEnd( rSect.IsFootnoteAtEnd() ),
    bEndnAtEnd( rSect.IsEndnAtEnd() ),
    bContentLock( false ),
    bOwnFootnoteNum( false ),
    bFootnoteLock( false )
{
    mnFrmType = FRM_SECTION;

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

// NOTE: call <SwSectionFrm::Init()> directly after creation of a new section
//       frame and its insert in the layout.
void SwSectionFrm::Init()
{
    OSL_ENSURE( GetUpper(), "SwSectionFrm::Init before insertion?!" );
    SWRECTFN( this )
    long nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
    (Frm().*fnRect->fnSetWidth)( nWidth );
    (Frm().*fnRect->fnSetHeight)( 0 );

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

void SwSectionFrm::DestroyImpl()
{
    if( GetFormat() && !GetFormat()->GetDoc()->IsInDtor() )
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

    SwLayoutFrm::DestroyImpl();
}

SwSectionFrm::~SwSectionFrm()
{
}

void SwSectionFrm::DelEmpty( bool bRemove )
{
    if( IsColLocked() )
    {
        OSL_ENSURE( !bRemove, "Don't delete locked SectionFrms" );
        return;
    }
    SwFrm* pUp = GetUpper();
    if( pUp )
    {
        // #i27138#
        // notify accessibility paragraphs objects about changed
        // CONTENT_FLOWS_FROM/_TO relation.
        // Relation CONTENT_FLOWS_FROM for current next paragraph will change
        // and relation CONTENT_FLOWS_TO for current previous paragraph will change.
        {
            SwViewShell* pViewShell( getRootFrm()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                pViewShell->InvalidateAccessibleParaFlowRelation(
                                dynamic_cast<SwTextFrm*>(FindNextCnt( true )),
                                dynamic_cast<SwTextFrm*>(FindPrevCnt( true )) );
            }
        }
        _Cut( bRemove );
    }
    SwSectionFrm *pMaster = IsFollow() ? FindMaster() : nullptr;
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
    SetFollow(0);
    if( pUp )
    {
        Frm().Height( 0 );
        // If we are destroyed immediately anyway, we don't need
        // to put us into the list
        if( bRemove )
        {   // If we already were half dead before this DelEmpty,
            // we are likely in the list and have to remove us from
            // it
            if( !pSection && getRootFrm() )
                getRootFrm()->RemoveFromList( this );
        }
        else if( getRootFrm() )
            getRootFrm()->InsertEmptySct( this );
        pSection = NULL;  // like this a reanimation is virtually impossible though
    }
}

void SwSectionFrm::Cut()
{
    _Cut( true );
}

void SwSectionFrm::_Cut( bool bRemove )
{
    OSL_ENSURE( GetUpper(), "Cut ohne Upper()." );

    PROTOCOL( this, PROT_CUT, 0, GetUpper() )

    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    SwFrm *pFrm = GetNext();
    SwFrm* pPrepFrm = NULL;
    while( pFrm && pFrm->IsSctFrm() && !static_cast<SwSectionFrm*>(pFrm)->GetSection() )
        pFrm = pFrm->GetNext();
    if( pFrm )
    {   // The former successor might have calculated a gap to the predecessor
        // which is now obsolete since he becomes the first
        pFrm->_InvalidatePrt();
        pFrm->_InvalidatePos();
        if( pFrm->IsSctFrm() )
            pFrm = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
        if ( pFrm && pFrm->IsContentFrm() )
        {
            pFrm->InvalidatePage( pPage );
            if( IsInFootnote() && !GetIndPrev() )
                pPrepFrm = pFrm;
        }
    }
    else
    {
        InvalidateNextPos();
        // Someone has to take over the retouching: predecessor or Upper
        if ( 0 != (pFrm = GetPrev()) )
        {   pFrm->SetRetouche();
            pFrm->Prepare( PREP_WIDOWS_ORPHANS );
            if ( pFrm->IsContentFrm() )
                pFrm->InvalidatePage( pPage );
        }
        // If I am (was) the only FlowFrm in my Upper, then he has to take over
        // the retouching.
        // Furthermore a blank page could have emerged
        else
        {   SwRootFrm *pRoot = static_cast<SwRootFrm*>(pPage->GetUpper());
            pRoot->SetSuperfluous();
            GetUpper()->SetCompletePaint();
        }
    }
    // First remove, then shrink Upper
    SwLayoutFrm *pUp = GetUpper();
    if( bRemove )
    {
        RemoveFromLayout();
        if( pUp && !pUp->Lower() && pUp->IsFootnoteFrm() && !pUp->IsColLocked() &&
            pUp->GetUpper() )
        {
            pUp->Cut();
            SwFrm::DestroyFrm(pUp);
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
    OSL_ENSURE( pParent->IsLayoutFrm(), "Parent is ContentFrm." );
    OSL_ENSURE( pParent != this, "I'm my own parent." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetUpper(),
            "I am still registered somewhere." );

    PROTOCOL( this, PROT_PASTE, 0, GetUpper() )

    // Add to the tree
    SwSectionFrm* pSect = pParent->FindSctFrm();
    // Assure that parent is not inside a table frame, which is inside the found section frame.
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
        if( pParent->IsColBodyFrm() ) // dealing with a single-column area
        {
            // If we are coincidentally at the end of a column, pSibling
            // has to point to the first frame of the next column in order
            // for the content of the next column to be moved correctly to the
            // newly created pSect by the InsertGroup
            SwColumnFrm *pCol = static_cast<SwColumnFrm*>(pParent->GetUpper());
            while( !pSibling && 0 != ( pCol = static_cast<SwColumnFrm*>(pCol->GetNext()) ) )
                pSibling = static_cast<SwLayoutFrm*>(static_cast<SwColumnFrm*>(pCol)->Lower())->Lower();
            if( pSibling )
            {
                // Even worse: every following column content has to
                // be attached to the pSibling-chain in order to be
                // taken along
                SwFrm *pTmp = pSibling;
                while ( 0 != ( pCol = static_cast<SwColumnFrm*>(pCol->GetNext()) ) )
                {
                    while ( pTmp->GetNext() )
                        pTmp = pTmp->GetNext();
                    SwFrm* pSave = ::SaveContent( pCol );
                    if (pSave)
                        ::RestoreContent( pSave, pSibling->GetUpper(), pTmp, true );
                }
            }
        }
        pParent = pSect;
        pSect = new SwSectionFrm( *static_cast<SwSectionFrm*>(pParent)->GetSection(), pParent );
        // if pParent is decomposed into two parts, its Follow has to be attached
        // to the new second part
        pSect->SetFollow( static_cast<SwSectionFrm*>(pParent)->GetFollow() );
        static_cast<SwSectionFrm*>(pParent)->SetFollow( NULL );
        if( pSect->GetFollow() )
            pParent->_InvalidateSize();

        const bool bInserted = InsertGroupBefore( pParent, pSibling, pSect );
        if (bInserted)
        {
            pSect->Init();
            (pSect->*fnRect->fnMakePos)( pSect->GetUpper(), pSect->GetPrev(), true);
        }
        if( !static_cast<SwLayoutFrm*>(pParent)->Lower() )
        {
            SwSectionFrm::MoveContentAndDelete( static_cast<SwSectionFrm*>(pParent), false );
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
        if ( pSibling->IsContentFrm() )
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
            if ( GetPrev()->IsContentFrm() )
                GetPrev()->InvalidatePage( pPage );
        }
    }
}

/**
|*  Here it's decided whether the this-SectionFrm should break up
|*  the passed (Section)frm (or not).
|*  Initially, all superior sections are broken up. Later on that could
|*  be made configurable.
|*/
bool SwSectionFrm::HasToBreak( const SwFrm* pFrm ) const
{
    if( !pFrm->IsSctFrm() )
        return false;

    const SwSectionFormat *pTmp = static_cast<const SwSectionFormat*>(GetFormat());

    const SwFrameFormat *pOtherFormat = static_cast<const SwSectionFrm*>(pFrm)->GetFormat();
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
|*  Merges two SectionFrms, in case it's about the same section.
|*  This can be necessary when a (sub)section is deleted that had
|*  divided another part into two.
|*/
void SwSectionFrm::MergeNext( SwSectionFrm* pNxt )
{
    if (pNxt->IsDeleteForbidden())
        return;

    if (!pNxt->IsJoinLocked() && GetSection() == pNxt->GetSection())
    {
        PROTOCOL( this, PROT_SECTION, ACT_MERGE, pNxt )

        SwFrm* pTmp = ::SaveContent( pNxt );
        if( pTmp )
        {
            SwFrm* pLast = Lower();
            SwLayoutFrm* pLay = this;
            if( pLast )
            {
                while( pLast->GetNext() )
                    pLast = pLast->GetNext();
                if( pLast->IsColumnFrm() )
                {   // Columns now with BodyFrm
                    pLay = static_cast<SwLayoutFrm*>(static_cast<SwLayoutFrm*>(pLast)->Lower());
                    pLast = pLay->Lower();
                    if( pLast )
                        while( pLast->GetNext() )
                            pLast = pLast->GetNext();
                }
            }
            ::RestoreContent( pTmp, pLay, pLast, true );
        }
        SetFollow( pNxt->GetFollow() );
        pNxt->SetFollow( NULL );
        pNxt->Cut();
        SwFrm::DestroyFrm(pNxt);
        InvalidateSize();
    }
}

/**
|*  Divides a SectionFrm into two parts. The second one starts with the
|*  passed frame.
|*  This is required when inserting an inner section, because the MoveFwd
|*  cannot have the desired effect within a frame or a table cell.
|*/
bool SwSectionFrm::SplitSect( SwFrm* pFrm, bool bApres )
{
    OSL_ENSURE( pFrm, "SplitSect: Why?" );
    SwFrm* pOther = bApres ? pFrm->FindNext() : pFrm->FindPrev();
    if( !pOther )
        return false;
    SwSectionFrm* pSect = pOther->FindSctFrm();
    if( pSect != this )
        return false;
    // Put the content aside
    SwFrm* pSav = ::SaveContent( this, bApres ? pOther : pFrm );
    OSL_ENSURE( pSav, "SplitSect: What's on?" );
    if( pSav ) // be robust
    {   // Create a new SctFrm, not as a Follower/master
        SwSectionFrm* pNew = new SwSectionFrm( *pSect->GetSection(), pSect );
        pNew->InsertBehind( pSect->GetUpper(), pSect );
        pNew->Init();
        SWRECTFN( this )
        (pNew->*fnRect->fnMakePos)( NULL, pSect, true );
        // OD 25.03.2003 #108339# - restore content:
        // determine layout frame for restoring content after the initialization
        // of the section frame. In the section initialization the columns are
        // created.
        {
            SwLayoutFrm* pLay = pNew;
            // Search for last layout frame, e.g. for columned sections.
            while( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
                pLay = static_cast<SwLayoutFrm*>(pLay->Lower());
            ::RestoreContent( pSav, pLay, NULL, true );
        }
        _InvalidateSize();
        if( HasFollow() )
        {
            pNew->SetFollow( GetFollow() );
            SetFollow( NULL );
        }
        return true;
    }
    return false;
}

/**
|*  MoveContent is called for destroying a SectionFrms, due to
|*  the cancellation or hiding of a section, to handle the content.
|*  If the SectionFrm hasn't broken up another one, then the content
|*  is moved to the Upper. Otherwise the content is moved to another
|*  SectionFrm, which has to be potentially merged.
|*/
// If a multi-column section is cancelled, the ContentFrms have to be
// invalidated
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
            lcl_InvalidateInfFlags( static_cast<SwLayoutFrm*>(pFrm)->GetLower(), false );
        pFrm = pFrm->GetNext();
    }
}

// Works like SwContentFrm::ImplGetNextContentFrm, but starts with a LayoutFrm
static SwContentFrm* lcl_GetNextContentFrm( const SwLayoutFrm* pLay, bool bFwd )
{
    if ( bFwd )
    {
        if ( pLay->GetNext() && pLay->GetNext()->IsContentFrm() )
            return const_cast<SwContentFrm*>(static_cast<const SwContentFrm*>(pLay->GetNext()));
    }
    else
    {
        if ( pLay->GetPrev() && pLay->GetPrev()->IsContentFrm() )
            return const_cast<SwContentFrm*>(static_cast<const SwContentFrm*>(pLay->GetPrev()));
    }

    const SwFrm* pFrm = pLay;
    SwContentFrm *pContentFrm = 0;
    bool bGoingUp = true;
    do {
        const SwFrm *p = 0;
        bool bGoingFwdOrBwd = false;

        bool bGoingDown = !bGoingUp && ( 0 !=  ( p = pFrm->IsLayoutFrm() ? static_cast<const SwLayoutFrm*>(pFrm)->Lower() : 0 ) );
        if ( !bGoingDown )
        {
            bGoingFwdOrBwd = ( 0 != ( p = pFrm->IsFlyFrm() ?
                                          ( bFwd ? static_cast<const SwFlyFrm*>(pFrm)->GetNextLink() : static_cast<const SwFlyFrm*>(pFrm)->GetPrevLink() ) :
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
    } while ( 0 == (pContentFrm = (pFrm->IsContentFrm() ? const_cast<SwContentFrm*>(static_cast<const SwContentFrm*>(pFrm)) : 0) ));

    return pContentFrm;
}

#define FIRSTLEAF( pLayFrm ) ( ( pLayFrm->Lower() && pLayFrm->Lower()->IsColumnFrm() )\
                    ? pLayFrm->GetNextLayoutLeaf() \
                    : pLayFrm )

void SwSectionFrm::MoveContentAndDelete( SwSectionFrm* pDel, bool bSave )
{
    bool bSize = pDel->Lower() && pDel->Lower()->IsColumnFrm();
    SwFrm* pPrv = pDel->GetPrev();
    SwLayoutFrm* pUp = pDel->GetUpper();
    // OD 27.03.2003 #i12711# - initialize local pointer variables.
    SwSectionFrm* pPrvSct = NULL;
    SwSectionFrm* pNxtSct = NULL;
    SwSectionFormat* pParent = static_cast<SwSectionFormat*>(pDel->GetFormat())->GetParent();
    if( pDel->IsInTab() && pParent )
    {
        SwTabFrm *pTab = pDel->FindTabFrm();
        // If we are within a table, we can only have broken up sections that
        // are inside as well, but not a section that contains the whole table.
        if( pTab->IsInSct() && pParent == pTab->FindSctFrm()->GetFormat() )
            pParent = NULL;
    }
    // If our Format has a parent, we have probably broken up another
    // SectionFrm, which has to be checked. To do so we first acquire the
    // succeeding and the preceding ContentFrm, let's see if they
    // lay in the SectionFrms.
    // OD 27.03.2003 #i12711# - check, if previous and next section belonging
    // together and can be joined, *not* only if deleted section contains content.
    if ( pParent )
    {
        SwFrm* pPrvContent = lcl_GetNextContentFrm( pDel, false );
        pPrvSct = pPrvContent ? pPrvContent->FindSctFrm() : NULL;
        SwFrm* pNxtContent = lcl_GetNextContentFrm( pDel, true );
        pNxtSct = pNxtContent ? pNxtContent->FindSctFrm() : NULL;
    }
    else
    {
        pParent = NULL;
        pPrvSct = pNxtSct = NULL;
    }

    // Now the content is put aside and the frame is destroyed
    SwFrm *pSave = bSave ? ::SaveContent( pDel ) : NULL;
    bool bOldFootnote = true;
    if( pSave && pUp->IsFootnoteFrm() )
    {
        bOldFootnote = static_cast<SwFootnoteFrm*>(pUp)->IsColLocked();
        static_cast<SwFootnoteFrm*>(pUp)->ColLock();
    }
    pDel->DelEmpty( true );
    SwFrm::DestroyFrm(pDel);
    if( pParent )
    {   // Search for the appropriate insert position
        if( pNxtSct && pNxtSct->GetFormat() == pParent )
        {   // Here we can insert outselves at the beginning
            pUp = FIRSTLEAF( pNxtSct );
            pPrv = NULL;
            if( pPrvSct && !( pPrvSct->GetFormat() == pParent ) )
                pPrvSct = NULL; // In order that nothing is merged
        }
        else if( pPrvSct && pPrvSct->GetFormat() == pParent )
        {   // Wonderful, here we can insert ourselves at the end
            pUp = pPrvSct;
            if( pUp->Lower() && pUp->Lower()->IsColumnFrm() )
            {
                pUp = static_cast<SwLayoutFrm*>(pUp->GetLastLower());
                // The body of the last column
                pUp = static_cast<SwLayoutFrm*>(pUp->Lower());
            }
            // In order to perform the insertion after the last one
            pPrv = pUp->GetLastLower();
            pPrvSct = NULL; // Such that nothing is merged
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
                pPrvSct = new SwSectionFrm( *pParent->GetSection(), pUp );
                pPrvSct->InsertBehind( pUp, pPrv );
                pPrvSct->Init();
                SWRECTFN( pUp )
                (pPrvSct->*fnRect->fnMakePos)( pUp, pPrv, true );
                pUp = FIRSTLEAF( pPrvSct );
                pPrv = NULL;
            }
            pPrvSct = NULL; // Such that nothing will be merged
        }
    }
    // The content is going to be inserted..
    if( pSave )
    {
        lcl_InvalidateInfFlags( pSave, bSize );
        ::RestoreContent( pSave, pUp, pPrv, true );
        pUp->FindPageFrm()->InvalidateContent();
        if( !bOldFootnote )
            static_cast<SwFootnoteFrm*>(pUp)->ColUnlock();
    }
    // Now two parts of the superior section could possibly be merged
    if( pPrvSct && !pPrvSct->IsJoinLocked() )
    {
        OSL_ENSURE( pNxtSct, "MoveContent: No Merge" );
        pPrvSct->MergeNext( pNxtSct );
    }
}

void SwSectionFrm::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    if ( IsJoinLocked() || IsColLocked() || StackHack::IsLocked() || StackHack::Count() > 50 )
        return;
    if( !pSection ) // Via DelEmpty
    {
#ifdef DBG_UTIL
        OSL_ENSURE( getRootFrm()->IsInDelList( this ), "SectionFrm without Section" );
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
        const SwFrm* pFoll = GetFollow();
        MergeNext( static_cast<SwSectionFrm*>(GetNext()) );
        if( pFoll == GetFollow() )
            break;
    }

    // OD 2004-03-15 #116561# - In online layout join the follows, if section
    // can grow.
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

    // A section with Follow uses all the space until the lower edge of the
    // Upper. If it moves, its size can grow or decrease...
    if( !mbValidPos && ToMaximize( false ) )
        mbValidSize = false;

#if OSL_DEBUG_LEVEL > 1
    const SwFormatCol &rCol = GetFormat()->GetCol();
    (void)rCol;
#endif
    SwLayoutFrm::MakeAll(getRootFrm()->GetCurrShell()->GetOut());
    UnlockJoin();
    if( pSection && IsSuperfluous() )
        DelEmpty( false );
}

bool SwSectionFrm::ShouldBwdMoved( SwLayoutFrm *, bool , bool & )
{
    OSL_FAIL( "Hups, wo ist meine Tarnkappe?" );
    return false;
}

const SwSectionFormat* SwSectionFrm::_GetEndSectFormat() const
{
    const SwSectionFormat *pFormat = pSection->GetFormat();
    while( !pFormat->GetEndAtTextEnd().IsAtEnd() )
    {
        if( dynamic_cast< const SwSectionFormat *>( pFormat->GetRegisteredIn()) !=  nullptr )
            pFormat = static_cast<const SwSectionFormat*>(pFormat->GetRegisteredIn());
        else
            return NULL;
    }
    return pFormat;
}

static void lcl_FindContentFrm( SwContentFrm* &rpContentFrm, SwFootnoteFrm* &rpFootnoteFrm,
    SwFrm* pFrm, bool &rbChkFootnote )
{
    if( pFrm )
    {
        while( pFrm->GetNext() )
            pFrm = pFrm->GetNext();
        while( !rpContentFrm && pFrm )
        {
            if( pFrm->IsContentFrm() )
                rpContentFrm = static_cast<SwContentFrm*>(pFrm);
            else if( pFrm->IsLayoutFrm() )
            {
                if( pFrm->IsFootnoteFrm() )
                {
                    if( rbChkFootnote )
                    {
                        rpFootnoteFrm = static_cast<SwFootnoteFrm*>(pFrm);
                        rbChkFootnote = rpFootnoteFrm->GetAttr()->GetFootnote().IsEndNote();
                    }
                }
                else
                    lcl_FindContentFrm( rpContentFrm, rpFootnoteFrm,
                        static_cast<SwLayoutFrm*>(pFrm)->Lower(), rbChkFootnote );
            }
            pFrm = pFrm->GetPrev();
        }
    }
}

SwContentFrm *SwSectionFrm::FindLastContent( sal_uInt8 nMode )
{
    SwContentFrm *pRet = NULL;
    SwFootnoteFrm *pFootnoteFrm = NULL;
    SwSectionFrm *pSect = this;
    if( nMode )
    {
        const SwSectionFormat *pFormat = IsEndnAtEnd() ? GetEndSectFormat() :
                                     pSection->GetFormat();
        do {
            while( pSect->HasFollow() )
                pSect = pSect->GetFollow();
            SwFrm* pTmp = pSect->FindNext();
            while( pTmp && pTmp->IsSctFrm() &&
                   !static_cast<SwSectionFrm*>(pTmp)->GetSection() )
                pTmp = pTmp->FindNext();
            if( pTmp && pTmp->IsSctFrm() &&
                static_cast<SwSectionFrm*>(pTmp)->IsDescendantFrom( pFormat ) )
                pSect = static_cast<SwSectionFrm*>(pTmp);
            else
                break;
        } while( true );
    }
    bool bFootnoteFound = nMode == FINDMODE_ENDNOTE;
    do
    {
        lcl_FindContentFrm( pRet, pFootnoteFrm, pSect->Lower(), bFootnoteFound );
        if( pRet || !pSect->IsFollow() || !nMode ||
            ( FINDMODE_MYLAST == nMode && this == pSect ) )
            break;
        pSect = pSect->FindMaster();
    } while( pSect );
    if( ( nMode == FINDMODE_ENDNOTE ) && pFootnoteFrm )
        pRet = pFootnoteFrm->ContainsContent();
    return pRet;
}

bool SwSectionFrm::CalcMinDiff( SwTwips& rMinDiff ) const
{
    if( ToMaximize( true ) )
    {
        SWRECTFN( this )
        rMinDiff = (GetUpper()->*fnRect->fnGetPrtBottom)();
        rMinDiff = (Frm().*fnRect->fnBottomDist)( rMinDiff );
        return true;
    }
    return false;
}

/**
 *  CollectEndnotes looks for endnotes in the sectionfrm and his follows,
 *  the endnotes will cut off the layout and put into the array.
 *  If the first endnote is not a master-SwFootnoteFrm, the whole sectionfrm
 *  contains only endnotes and it is not necessary to collect them.
 */
static SwFootnoteFrm* lcl_FindEndnote( SwSectionFrm* &rpSect, bool &rbEmpty,
    SwLayouter *pLayouter )
{
    // if rEmpty is set, the rpSect is already searched
    SwSectionFrm* pSect = rbEmpty ? rpSect->GetFollow() : rpSect;
    while( pSect )
    {
       OSL_ENSURE( (pSect->Lower() && pSect->Lower()->IsColumnFrm()) || pSect->GetUpper()->IsFootnoteFrm(),
                "InsertEndnotes: Where's my column?" );

        // i73332: Columned section in endnote
        SwColumnFrm* pCol = 0;
        if(pSect->Lower() && pSect->Lower()->IsColumnFrm())
            pCol = static_cast<SwColumnFrm*>(pSect->Lower());

        while( pCol ) // check all columns
        {
            SwFootnoteContFrm* pFootnoteCont = pCol->FindFootnoteCont();
            if( pFootnoteCont )
            {
                SwFootnoteFrm* pRet = static_cast<SwFootnoteFrm*>(pFootnoteCont->Lower());
                while( pRet ) // look for endnotes
                {
                    /* CollectEndNode can destroy pRet so we need to get the
                       next early
                    */
                    SwFootnoteFrm* pRetNext = static_cast<SwFootnoteFrm*>(pRet->GetNext());
                    if( pRet->GetAttr()->GetFootnote().IsEndNote() )
                    {
                        if( pRet->GetMaster() )
                        {
                            if( pLayouter )
                                pLayouter->CollectEndnote( pRet );
                            else
                                return 0;
                        }
                        else
                            return pRet; // Found
                    }
                    pRet = pRetNext;
                }
            }
            pCol = static_cast<SwColumnFrm*>(pCol->GetNext());
        }
        rpSect = pSect;
        pSect = pLayouter ? pSect->GetFollow() : NULL;
        rbEmpty = true;
    }
    return NULL;
}

static void lcl_ColumnRefresh( SwSectionFrm* pSect, bool bFollow )
{
    vcl::RenderContext* pRenderContext = pSect->getRootFrm()->GetCurrShell()->GetOut();
    while( pSect )
    {
        bool bOldLock = pSect->IsColLocked();
        pSect->ColLock();
        if( pSect->Lower() && pSect->Lower()->IsColumnFrm() )
        {
            SwColumnFrm *pCol = static_cast<SwColumnFrm*>(pSect->Lower());
            do
            {   pCol->_InvalidateSize();
                pCol->_InvalidatePos();
                static_cast<SwLayoutFrm*>(pCol)->Lower()->_InvalidateSize();
                pCol->Calc(pRenderContext);   // calculation of column and
                static_cast<SwLayoutFrm*>(pCol)->Lower()->Calc(pRenderContext);  // body
                pCol = static_cast<SwColumnFrm*>(pCol->GetNext());
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
    // i73332: Section in footnode does not have columns!
    OSL_ENSURE( (Lower() && Lower()->IsColumnFrm()) || GetUpper()->IsFootnoteFrm(), "Where's my column?" );

    SwSectionFrm* pSect = this;
    SwFootnoteFrm* pFootnote;
    bool bEmpty = false;
    // pSect is the last sectionfrm without endnotes or the this-pointer
    // the first sectionfrm with endnotes may be destroyed, when the endnotes
    // is cutted
    while( 0 != (pFootnote = lcl_FindEndnote( pSect, bEmpty, pLayouter )) )
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
void SwSectionFrm::_CheckClipping( bool bGrow, bool bMaximize )
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
    // OD 03.11.2003 #i19737# - introduce local variable <bExtraCalc> to indicate
    // that a calculation has to be done beside the value of <bCalc>.
    bool bExtraCalc = false;
    if( !bCalc && !bGrow && IsAnyNoteAtEnd() && !IsInFootnote() )
    {
        SwSectionFrm *pSect = this;
        bool bEmpty = false;
        SwLayoutFrm* pFootnote = IsEndnAtEnd() ?
            lcl_FindEndnote( pSect, bEmpty, NULL ) : NULL;
        if( pFootnote )
        {
            pFootnote = pFootnote->FindFootnoteBossFrm();
            SwFrm* pTmp = FindLastContent( FINDMODE_LASTCNT );
            // OD 08.11.2002 #104840# - use <SwLayoutFrm::IsBefore(..)>
            if ( pTmp && pFootnote->IsBefore( pTmp->FindFootnoteBossFrm() ) )
                bExtraCalc = true;
        }
        else if( GetFollow() && !GetFollow()->ContainsAny() )
            bExtraCalc = true;
    }
    if ( bCalc || bExtraCalc )
    {
        nDiff = (*fnRect->fnYDiff)( nDeadLine, (Frm().*fnRect->fnGetTop)() );
        if( nDiff < 0 )
            nDeadLine = (Frm().*fnRect->fnGetTop)();
        const Size aOldSz( Prt().SSize() );
        long nTop = (this->*fnRect->fnGetTopMargin)();
        (Frm().*fnRect->fnSetBottom)( nDeadLine );
        nDiff = (Frm().*fnRect->fnGetHeight)();
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
            if( Lower()->IsColumnFrm() )
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

void SwSectionFrm::SimpleFormat()
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

// #i40147# - helper class to perform extra section format
// to position anchored objects and to keep the position of whose objects locked.
class ExtraFormatToPositionObjs
{
    private:
        SwSectionFrm* mpSectFrm;
        bool mbExtraFormatPerformed;

    public:
        explicit ExtraFormatToPositionObjs( SwSectionFrm& _rSectFrm)
            : mpSectFrm( &_rSectFrm ),
              mbExtraFormatPerformed( false )
        {}

        ~ExtraFormatToPositionObjs()
        {
            if ( mbExtraFormatPerformed )
            {
                // release keep locked position of lower floating screen objects
                SwPageFrm* pPageFrm = mpSectFrm->FindPageFrm();
                SwSortedObjs* pObjs = pPageFrm ? pPageFrm->GetSortedObjs() : 0L;
                if ( pObjs )
                {
                    for ( size_t i = 0; i < pObjs->size(); ++i )
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

        // #i81555#
        void InitObjs( SwFrm& rFrm )
        {
            SwSortedObjs* pObjs = rFrm.GetDrawObjs();
            if ( pObjs )
            {
                for ( size_t i = 0; i < pObjs->size(); ++i )
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
            vcl::RenderContext* pRenderContext = mpSectFrm->getRootFrm()->GetCurrShell()->GetOut();
            // perform extra format for multi-columned section.
            if ( mpSectFrm->Lower() && mpSectFrm->Lower()->IsColumnFrm() &&
                 mpSectFrm->Lower()->GetNext() )
            {
                // grow section till bottom of printing area of upper frame
                SWRECTFN( mpSectFrm );
                SwTwips nTopMargin = (mpSectFrm->*fnRect->fnGetTopMargin)();
                Size aOldSectPrtSize( mpSectFrm->Prt().SSize() );
                SwTwips nDiff = (mpSectFrm->Frm().*fnRect->fnBottomDist)(
                                        (mpSectFrm->GetUpper()->*fnRect->fnGetPrtBottom)() );
                (mpSectFrm->Frm().*fnRect->fnAddBottom)( nDiff );
                (mpSectFrm->*fnRect->fnSetYMargins)( nTopMargin, 0 );
                // #i59789#
                // suppress formatting, if printing area of section is too narrow
                if ( (mpSectFrm->Prt().*fnRect->fnGetHeight)() <= 0 )
                {
                    return;
                }
                mpSectFrm->ChgLowersProp( aOldSectPrtSize );

                // format column frames and its body and footnote container
                SwColumnFrm* pColFrm = static_cast<SwColumnFrm*>(mpSectFrm->Lower());
                while ( pColFrm )
                {
                    pColFrm->Calc(pRenderContext);
                    pColFrm->Lower()->Calc(pRenderContext);
                    if ( pColFrm->Lower()->GetNext() )
                    {
                        pColFrm->Lower()->GetNext()->Calc(pRenderContext);
                    }

                    pColFrm = static_cast<SwColumnFrm*>(pColFrm->GetNext());
                }

                // unlock position of lower floating screen objects for the extra format
                // #i81555#
                // Section frame can already have changed the page and its content
                // can still be on the former page.
                // Thus, initialize objects via lower-relationship
                InitObjs( *mpSectFrm );

                // format content - first with collecting its foot-/endnotes before content
                // format, second without collecting its foot-/endnotes.
                ::CalcContent( mpSectFrm );
                ::CalcContent( mpSectFrm, true );

                // keep locked position of lower floating screen objects
                SwPageFrm* pPageFrm = mpSectFrm->FindPageFrm();
                SwSortedObjs* pObjs = pPageFrm ? pPageFrm->GetSortedObjs() : 0L;
                if ( pObjs )
                {
                    for ( size_t i = 0; i < pObjs->size(); ++i )
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

/// "formats" the frame; Frm and PrtArea
void SwSectionFrm::Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttr )
{
    if( !pSection ) // via DelEmpty
    {
#ifdef DBG_UTIL
        OSL_ENSURE( getRootFrm()->IsInDelList( this ), "SectionFrm without Section" );
#endif
        mbValidSize = mbValidPos = mbValidPrtArea = true;
        return;
    }
    SWRECTFN( this )
    if ( !mbValidPrtArea )
    {
        PROTOCOL( this, PROT_PRTAREA, 0, 0 )
        mbValidPrtArea = true;
        SwTwips nUpper = CalcUpperSpace();

        // #109700# LRSpace for sections
        const SvxLRSpaceItem& rLRSpace = GetFormat()->GetLRSpace();
        (this->*fnRect->fnSetXMargins)( rLRSpace.GetLeft(), rLRSpace.GetRight() );

        if( nUpper != (this->*fnRect->fnGetTopMargin)() )
        {
            mbValidSize = false;
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
        bool bOldLock = IsColLocked();
        ColLock();

        mbValidSize = true;

        // The size is only determined by the content, if the SectFrm does not have a
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
        const bool bHasColumns = Lower() && Lower()->IsColumnFrm();
        if ( bHasColumns && Lower()->GetNext() )
            AdjustColumns( 0, false );

        if( GetUpper() )
        {
            long nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            (maFrm.*fnRect->fnSetWidth)( nWidth );

            // #109700# LRSpace for sections
            const SvxLRSpaceItem& rLRSpace = GetFormat()->GetLRSpace();
            (maPrt.*fnRect->fnSetWidth)( nWidth - rLRSpace.GetLeft() -
                                        rLRSpace.GetRight() );

            // OD 15.10.2002 #103517# - allow grow in online layout
            // Thus, set <..IsBrowseMode()> as parameter <bGrow> on calling
            // method <_CheckClipping(..)>.
            const SwViewShell *pSh = getRootFrm()->GetCurrShell();
            _CheckClipping( pSh && pSh->GetViewOptions()->getBrowseMode(), bMaximize );
            bMaximize = ToMaximize( false );
            mbValidSize = true;
        }

        // Check the width of the columns and adjust if necessary
        if ( bHasColumns && ! Lower()->GetNext() && bMaximize )
            static_cast<SwColumnFrm*>(Lower())->Lower()->Calc(pRenderContext);

        if ( !bMaximize )
        {
            SwTwips nRemaining = (this->*fnRect->fnGetTopMargin)();
            SwFrm *pFrm = m_pLower;
            if( pFrm )
            {
                if( pFrm->IsColumnFrm() && pFrm->GetNext() )
                {
                    // #i61435#
                    // suppress formatting, if upper frame has height <= 0
                    if ( (GetUpper()->Frm().*fnRect->fnGetHeight)() > 0 )
                    {
                        FormatWidthCols( *pAttr, nRemaining, MINLAY );
                    }
                    // #126020# - adjust check for empty section
                    // #130797# - correct fix #126020#
                    while( HasFollow() && !GetFollow()->ContainsContent() &&
                           !GetFollow()->ContainsAny( true ) )
                    {
                        SwFrm* pOld = GetFollow();
                        GetFollow()->DelEmpty( false );
                        if( pOld == GetFollow() )
                            break;
                    }
                    bMaximize = ToMaximize( false );
                    nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
                }
                else
                {
                    if( pFrm->IsColumnFrm() )
                    {
                        pFrm->Calc(pRenderContext);
                        pFrm = static_cast<SwColumnFrm*>(pFrm)->Lower();
                        pFrm->Calc(pRenderContext);
                        pFrm = static_cast<SwLayoutFrm*>(pFrm)->Lower();
                        CalcFootnoteContent();
                    }
                    // If we are in a columned frame which calls a CalcContent
                    // in the FormatWidthCols, the content might need calculating
                    if( pFrm && !pFrm->IsValid() && IsInFly() &&
                        FindFlyFrm()->IsColLocked() )
                        ::CalcContent( this );
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
                long nTmp = nRemaining - (Frm().*fnRect->fnGetHeight)();
                long nTop = (this->*fnRect->fnGetTopMargin)();
                (Frm().*fnRect->fnAddBottom)( nTmp );
                (this->*fnRect->fnSetYMargins)( nTop, 0 );
                InvalidateNextPos();
                if (m_pLower && (!m_pLower->IsColumnFrm() || !m_pLower->GetNext()))
                {
                    // If a single-column section just created the space that
                    // was requested by the "undersized" paragraphs, then they
                    // have to be invalidated and calculated, so they fully cover it
                    pFrm = m_pLower;
                    if( pFrm->IsColumnFrm() )
                    {
                        pFrm->_InvalidateSize();
                        pFrm->_InvalidatePos();
                        pFrm->Calc(pRenderContext);
                        pFrm = static_cast<SwColumnFrm*>(pFrm)->Lower();
                        pFrm->Calc(pRenderContext);
                        pFrm = static_cast<SwLayoutFrm*>(pFrm)->Lower();
                        CalcFootnoteContent();
                    }
                    bool bUnderSz = false;
                    while( pFrm )
                    {
                        if( pFrm->IsTextFrm() && static_cast<SwTextFrm*>(pFrm)->IsUndersized() )
                        {
                            pFrm->Prepare( PREP_ADJUST_FRM );
                            bUnderSz = true;
                        }
                        pFrm = pFrm->GetNext();
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
        long nDiff = nOldHeight - (Frm().*fnRect->fnGetHeight)();
        if( nDiff > 0 )
        {
            if( !GetNext() )
                SetRetouche(); // Take over the retouching ourselves
            if( GetUpper() && !GetUpper()->IsFooterFrm() )
                GetUpper()->Shrink( nDiff );
        }
        if( IsUndersized() )
            mbValidPrtArea = true;
    }
}

/// Returns the next layout sheet where the frame can be moved in.
/// New pages are created only if specified by the parameter.
SwLayoutFrm *SwFrm::GetNextSctLeaf( MakePageType eMakePage )
{
    // Attention: Nested sections are currently not supported

    PROTOCOL_ENTER( this, PROT_LEAF, ACT_NEXT_SECT, GetUpper()->FindSctFrm() )

    // Shortcuts for "columned" sections, if we're not in the last column
    // Can we slide to the next column of the section?
    if( IsColBodyFrm() && GetUpper()->GetNext() )
        return static_cast<SwLayoutFrm*>(static_cast<SwLayoutFrm*>(GetUpper()->GetNext())->Lower());
    if( GetUpper()->IsColBodyFrm() && GetUpper()->GetUpper()->GetNext() )
        return static_cast<SwLayoutFrm*>(static_cast<SwLayoutFrm*>(GetUpper()->GetUpper()->GetNext())->Lower());
    // Inside a section, in tables, or sections of headers/footers, there can be only
    // one column shift be made, one of the above shortcuts should have applied!
    if( GetUpper()->IsInTab() || FindFooterOrHeader() )
        return 0;

    SwSectionFrm *pSect = FindSctFrm();
    bool bWrongPage = false;
    OSL_ENSURE( pSect, "GetNextSctLeaf: Missing SectionFrm" );

    // Shortcut for sections with Follows. That's ok,
    // if no columns or pages (except dummy pages) lie in between.
    // In case of linked frames and in footnotes the shortcut would get
    // even more costly
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
            if( pTmp ) // is now the next column or page
            {
                SwFrm* pTmpX = pTmp;
                if( pTmp->IsPageFrm() && static_cast<SwPageFrm*>(pTmp)->IsEmptyPage() )
                    pTmp = pTmp->GetNext(); // skip dummy pages
                SwFrm *pUp = pSect->GetFollow()->GetUpper();
                // pUp becomes the next column if the Follow lies in a column
                // that is not a "not first" one, otherwise the page
                if( !pUp->IsColBodyFrm() ||
                    !( pUp = pUp->GetUpper() )->GetPrev() )
                    pUp = pUp->FindPageFrm();
                // Now pUp and pTmp have to be the same page/column, otherwise
                // pages or columns lie between Master and Follow
                if( pUp == pTmp || pUp->GetNext() == pTmpX )
                {
                    SwPageFrm* pNxtPg = pUp->IsPageFrm() ?
                                        static_cast<SwPageFrm*>(pUp) : pUp->FindPageFrm();
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
    const bool bFootnotePage = FindPageFrm()->IsFootnotePage();

    SwLayoutFrm *pLayLeaf;
    // A shortcut for TabFrms such that not all cells need to be visited
    if( bWrongPage )
        pLayLeaf = 0;
    else if( IsTabFrm() )
    {
        SwContentFrm* pTmpCnt = static_cast<SwTabFrm*>(this)->FindLastContent();
        pLayLeaf = pTmpCnt ? pTmpCnt->GetUpper() : 0;
    }
    else
    {
        pLayLeaf = GetNextLayoutLeaf();
        if( IsColumnFrm() )
        {
            while( pLayLeaf && static_cast<SwColumnFrm*>(this)->IsAnLower( pLayLeaf ) )
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        }
    }

    SwLayoutFrm *pOldLayLeaf = 0;           // Such that in case of newly
                                            // created pages, the search is
                                            // not started over at the beginning

    while( true )
    {
        if( pLayLeaf )
        {
            // A layout leaf was found, let's see whether it can store me or
            // another SectionFrm can be inserted here, or we have to continue
            // searching
            SwPageFrm* pNxtPg = pLayLeaf->FindPageFrm();
            if ( !bFootnotePage && pNxtPg->IsFootnotePage() )
            {   // If I reached the end note pages it's over
                pLayLeaf = 0;
                continue;
            }
            // Once inBody always inBody, don't step into tables and not into other sections
            if ( (bBody && !pLayLeaf->IsInDocBody()) ||
                 (IsInFootnote() != pLayLeaf->IsInFootnote() ) ||
                 pLayLeaf->IsInTab() ||
                 ( pLayLeaf->IsInSct() && ( !pSect->HasFollow()
                   || pSect->GetFollow() != pLayLeaf->FindSctFrm() ) ) )
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
                pLayLeaf = 0;
                bWrongPage = true;
                pOldLayLeaf = 0;
                continue;
            }
        }
        // There is no further LayoutFrm that fits, so a new page
        // has to be created, although new pages are worthless within a frame
        else if( !pSect->IsInFly() &&
            ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT ) )
        {
            InsertPage(pOldLayLeaf ? pOldLayLeaf->FindPageFrm() : FindPageFrm(),
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
        SwSectionFrm* pNew;

        // This can be omitted if existing Follows were cut short
        SwFrm* pFirst = pLayLeaf->Lower();
        // Here SectionFrms that are to be deleted must be ignored
        while( pFirst && pFirst->IsSctFrm() && !static_cast<SwSectionFrm*>(pFirst)->GetSection() )
            pFirst = pFirst->GetNext();
        if( pFirst && pFirst->IsSctFrm() && pSect->GetFollow() == pFirst )
            pNew = pSect->GetFollow();
        else if( MAKEPAGE_NOSECTION == eMakePage )
            return pLayLeaf;
        else
        {
            pNew = new SwSectionFrm( *pSect, false );
            pNew->InsertBefore( pLayLeaf, pLayLeaf->Lower() );
            pNew->Init();
            SWRECTFN( pNew )
            (pNew->*fnRect->fnMakePos)( pLayLeaf, NULL, true );

            // If our section frame has a successor then that has to be
            // moved behind the new Follow of the section frames
            SwFrm* pTmp = pSect->GetNext();
            if( pTmp && pTmp != pSect->GetFollow() )
            {
                SwFlowFrm* pNxt;
                SwContentFrm* pNxtContent = NULL;
                if( pTmp->IsContentFrm() )
                {
                    pNxt = static_cast<SwContentFrm*>(pTmp);
                    pNxtContent = static_cast<SwContentFrm*>(pTmp);
                }
                else
                {
                    pNxtContent = static_cast<SwLayoutFrm*>(pTmp)->ContainsContent();
                    if( pTmp->IsSctFrm() )
                        pNxt = static_cast<SwSectionFrm*>(pTmp);
                    else
                    {
                        OSL_ENSURE( pTmp->IsTabFrm(), "GetNextSctLeaf: Wrong Type" );
                        pNxt = static_cast<SwTabFrm*>(pTmp);
                    }
                    while( !pNxtContent && 0 != ( pTmp = pTmp->GetNext() ) )
                    {
                        if( pTmp->IsContentFrm() )
                            pNxtContent = static_cast<SwContentFrm*>(pTmp);
                        else
                            pNxtContent = static_cast<SwLayoutFrm*>(pTmp)->ContainsContent();
                    }
                }
                if( pNxtContent )
                {
                    SwFootnoteBossFrm* pOldBoss = pSect->FindFootnoteBossFrm( true );
                    if( pOldBoss == pNxtContent->FindFootnoteBossFrm( true ) )
                    {
                        SwSaveFootnoteHeight aHeight( pOldBoss,
                            pOldBoss->Frm().Top() + pOldBoss->Frm().Height() );
                        pSect->GetUpper()->MoveLowerFootnotes( pNxtContent, pOldBoss,
                                    pLayLeaf->FindFootnoteBossFrm( true ), false );
                    }
                }
                pNxt->MoveSubTree( pLayLeaf, pNew->GetNext() );
            }
            if( pNew->GetFollow() )
                pNew->SimpleFormat();
        }
        // The wanted layout sheet is now the first of the determined SctFrms:
        pLayLeaf = FIRSTLEAF( pNew );
    }
    return pLayLeaf;
}

/// Returns the preceding layout sheet where the frame can be moved into
SwLayoutFrm *SwFrm::GetPrevSctLeaf( MakePageType )
{
    PROTOCOL_ENTER( this, PROT_LEAF, ACT_PREV_SECT, GetUpper()->FindSctFrm() )

    SwLayoutFrm* pCol;
    // ColumnFrm always contain a BodyFrm now
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
                pCol = static_cast<SwLayoutFrm*>(pCol->GetPrev());
                // Is there any content?
                if( static_cast<SwLayoutFrm*>(pCol->Lower())->Lower() )
                {
                    if( bJump )     // Did we skip a blank page?
                        SwFlowFrm::SetMoveBwdJump( true );
                    return static_cast<SwLayoutFrm*>(pCol->Lower());  // The columnm body
                }
                bJump = true;
            } while( pCol->GetPrev() );

            // We get here when all columns are empty, pCol is now the
            // first column, we need the body though
            pCol = static_cast<SwLayoutFrm*>(pCol->Lower());
        }
        else
            pCol = NULL;
    }

    if( bJump )     // Did we skip a blank page?
        SwFlowFrm::SetMoveBwdJump( true );

    // Within sections in tables or section in headers/footers there can
    // be only one column change be made, one of the above shortcuts should
    // have applied, also when the section has a pPrev.
    // Now we even consider an empty column...
    OSL_ENSURE( FindSctFrm(), "GetNextSctLeaf: Missing SectionFrm" );
    if( ( IsInTab() && !IsTabFrm() ) || FindFooterOrHeader() )
        return pCol;

    // === IMPORTANT ===
    // Precondition, which needs to be hold, is that the <this> frame can be
    // inside a table, but then the found section frame <pSect> is also inside
    // this table.
    SwSectionFrm *pSect = FindSctFrm();

    // #i95698#
    // A table cell containing directly a section does not break - see lcl_FindSectionsInRow(..)
    // Thus, a table inside a section, which is inside another table can only
    // flow backward in the columns of its section.
    // Note: The table cell, which contains the section, can not have a master table cell.
    if ( IsTabFrm() && pSect->IsInTab() )
    {
        return pCol;
    }

    {
        SwFrm *pPrv;
        if( 0 != ( pPrv = pSect->GetIndPrev() ) )
        {
            // Mooching, half dead SectionFrms shouldn't confuse us
            while( pPrv && pPrv->IsSctFrm() && !static_cast<SwSectionFrm*>(pPrv)->GetSection() )
                pPrv = pPrv->GetPrev();
            if( pPrv )
                return pCol;
        }
    }

    const bool bBody = IsInDocBody();
    const bool bFly  = IsInFly();

    SwLayoutFrm *pLayLeaf = GetPrevLayoutLeaf();
    SwLayoutFrm *pPrevLeaf = 0;

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
            const SwFrm* pTmp = pLayLeaf->Lower();
            // OD 11.04.2003 #108824# - consider, that the zombie section frame
            // can have frame below it in the found layout leaf.
            // Thus, skipping zombie section frame, if possible.
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
                SwFlowFrm::SetMoveBwdJump( true );
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

    SwSectionFrm* pNew = NULL;
    // At first go to the end of the layout sheet
    SwFrm *pTmp = pLayLeaf->Lower();
    if( pTmp )
    {
        while( pTmp->GetNext() )
            pTmp = pTmp->GetNext();
        if( pTmp->IsSctFrm() )
        {
            // Half dead ones only interfere here
            while( !static_cast<SwSectionFrm*>(pTmp)->GetSection() && pTmp->GetPrev() &&
                    pTmp->GetPrev()->IsSctFrm() )
                pTmp = pTmp->GetPrev();
            if( static_cast<SwSectionFrm*>(pTmp)->GetFollow() == pSect )
                pNew = static_cast<SwSectionFrm*>(pTmp);
        }
    }
    if( !pNew )
    {
        pNew = new SwSectionFrm( *pSect, true );
        pNew->InsertBefore( pLayLeaf, NULL );
        pNew->Init();
        SWRECTFN( pNew )
        (pNew->*fnRect->fnMakePos)( pLayLeaf, pNew->GetPrev(), true );

        pLayLeaf = FIRSTLEAF( pNew );
        if( !pNew->Lower() )    // Format single column sections
        {
            pNew->MakePos();
            pLayLeaf->Format(getRootFrm()->GetCurrShell()->GetOut()); // In order that the PrtArea is correct for the MoveBwd
        }
        else
            pNew->SimpleFormat();
    }
    else
    {
        pLayLeaf = FIRSTLEAF( pNew );
        if( pLayLeaf->IsColBodyFrm() )
        {
            // In existent section columns we're looking for the last not empty
            // column.
            SwLayoutFrm *pTmpLay = pLayLeaf;
            while( pLayLeaf->GetUpper()->GetNext() )
            {
                pLayLeaf = static_cast<SwLayoutFrm*>(static_cast<SwLayoutFrm*>(pLayLeaf->GetUpper()->GetNext())->Lower());
                if( pLayLeaf->Lower() )
                    pTmpLay = pLayLeaf;
            }
            // If we skipped an empty column, we've to set the jump-flag
            if( pLayLeaf != pTmpLay )
            {
                pLayLeaf = pTmpLay;
                SwFlowFrm::SetMoveBwdJump( true );
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
        // Columns now with BodyFrm
        else if( pUp->IsColBodyFrm() && pUp->GetUpper()->GetUpper()->IsSctFrm() )
            pUp = pUp->GetUpper()->GetUpper();
        else
            break;
    }
    SWRECTFN( pFrm )
    return pUp ? (pUp->*fnRect->fnGetPrtBottom)() :
                 (pFrm->Frm().*fnRect->fnGetBottom)();
}

/// checks whether the SectionFrm is still able to grow, as case may be the environment has to be asked
bool SwSectionFrm::Growable() const
{
    SWRECTFN( this )
    if( (*fnRect->fnYDiff)( lcl_DeadLine( this ),
        (Frm().*fnRect->fnGetBottom)() ) > 0 )
        return true;

    return ( GetUpper() && const_cast<SwFrm*>(static_cast<SwFrm const *>(GetUpper()))->Grow( LONG_MAX, true ) );
}

SwTwips SwSectionFrm::_Grow( SwTwips nDist, bool bTst )
{
    if ( !IsColLocked() && !HasFixSize() )
    {
        SWRECTFN( this )
        long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
        if( nFrmHeight > 0 && nDist > (LONG_MAX - nFrmHeight) )
            nDist = LONG_MAX - nFrmHeight;

        if ( nDist <= 0L )
            return 0L;

        bool bInCalcContent = GetUpper() && IsInFly() && FindFlyFrm()->IsLocked();
        // OD 2004-03-15 #116561# - allow grow in online layout
        bool bGrow = !Lower() || !Lower()->IsColumnFrm() || !Lower()->GetNext() ||
             GetSection()->GetFormat()->GetBalancedColumns().GetValue();
        if( !bGrow )
        {
             const SwViewShell *pSh = getRootFrm()->GetCurrShell();
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
                                           (Frm().*fnRect->fnGetBottom)() );
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
                    while( pFrm && pFrm->IsSctFrm() && !static_cast<SwSectionFrm*>(pFrm)->GetSection() )
                        pFrm = pFrm->GetNext();
                    if( pFrm )
                    {
                        if( bInCalcContent )
                            pFrm->_InvalidatePos();
                        else
                            pFrm->InvalidatePos();
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

SwTwips SwSectionFrm::_Shrink( SwTwips nDist, bool bTst )
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
            long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
            if ( nDist > nFrmHeight )
                nDist = nFrmHeight;

            if ( Lower()->IsColumnFrm() && Lower()->GetNext() && // FootnoteAtEnd
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
                (Frm().*fnRect->fnAddBottom)( -nDist );
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
                    while( pFrm && pFrm->IsSctFrm() && !static_cast<SwSectionFrm*>(pFrm)->GetSection() )
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
|*  This routine is used in the TextFormatter to decided whether it's allowed to
|*  create a (paragraph-)Follow or whether the paragraph has to stick together
|*/
bool SwSectionFrm::MoveAllowed( const SwFrm* pFrm) const
{
    // Is there a Follow or is the Frame not in the last column?
    if( HasFollow() || ( pFrm->GetUpper()->IsColBodyFrm() &&
        pFrm->GetUpper()->GetUpper()->GetNext() ) )
        return true;
    if( pFrm->IsInFootnote() )
    {
        if( IsInFootnote() )
        {
            if( GetUpper()->IsInSct() )
            {
                if( Growable() )
                    return false;
                return GetUpper()->FindSctFrm()->MoveAllowed( this );
            }
            else
                return true;
        }
        // The content of footnote inside a columned sectionfrm is moveable
        // except in the last column
        const SwLayoutFrm *pLay = pFrm->FindFootnoteFrm()->GetUpper()->GetUpper();
        if( pLay->IsColumnFrm() && pLay->GetNext() )
        {
            // The first paragraph in the first footnote in the first column
            // in the sectionfrm at the top of the page is not moveable,
            // if the columnbody is empty.
            bool bRet = false;
            if( pLay->GetIndPrev() || pFrm->GetIndPrev() ||
                pFrm->FindFootnoteFrm()->GetPrev() )
                bRet = true;
            else
            {
                const SwLayoutFrm* pBody = static_cast<const SwColumnFrm*>(pLay)->FindBodyCont();
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
        return 0 != const_cast<SwFrm*>(static_cast<SwFrm const *>(GetUpper()))->GetNextLeaf( MAKEPAGE_NONE );
    return true;
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
    // #i79774#
    // Do not assert, if the frame has a direct previous frame, because it
    // could be an empty section frame. The caller has to assure, that the
    // frame has no direct previous frame or only empty section frames as
    // previous frames.
    OSL_ENSURE( /*!pPrev &&*/ IsInSct(), "Why?" );
    const SwFrm* pSct = GetUpper();
    if( !pSct )
        return NULL;
    if( pSct->IsSctFrm() )
        pRet = pSct->GetIndPrev();
    else if( pSct->IsColBodyFrm() && (pSct = pSct->GetUpper()->GetUpper())->IsSctFrm() )
    {
        // Do not return the previous frame of the outer section, if in one
        // of the previous columns is content.
        const SwFrm* pCol = GetUpper()->GetUpper()->GetPrev();
        while( pCol )
        {
            OSL_ENSURE( pCol->IsColumnFrm(), "GetIndPrev(): ColumnFrm expected" );
            OSL_ENSURE( pCol->GetLower() && pCol->GetLower()->IsBodyFrm(),
                    "GetIndPrev(): Where's the body?");
            if( static_cast<const SwLayoutFrm*>(static_cast<const SwLayoutFrm*>(pCol)->Lower())->Lower() )
                return NULL;
            pCol = pCol->GetPrev();
        }
        pRet = pSct->GetIndPrev();
    }

    // skip empty section frames
    while( pRet && pRet->IsSctFrm() && !static_cast<SwSectionFrm*>(pRet)->GetSection() )
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
    {   // We can only return the successor of the SectionFrms if there is no
        // content in the successing columns
        SwFrm* pCol = GetUpper()->GetUpper()->GetNext();
        while( pCol )
        {
            OSL_ENSURE( pCol->IsColumnFrm(), "GetIndNext(): ColumnFrm expected" );
            OSL_ENSURE( pCol->GetLower() && pCol->GetLower()->IsBodyFrm(),
                    "GetIndNext(): Where's the body?");
            if( static_cast<SwLayoutFrm*>(static_cast<SwLayoutFrm*>(pCol)->Lower())->Lower() )
                return NULL;
            pCol = pCol->GetNext();
        }
        return pSct->GetIndNext();
    }
    return NULL;
}

bool SwSectionFrm::IsDescendantFrom( const SwSectionFormat* pFormat ) const
{
    if( !pSection || !pFormat )
        return false;
    const SwSectionFormat *pMyFormat = pSection->GetFormat();
    while( pFormat != pMyFormat )
    {
        if( dynamic_cast< const SwSectionFormat *>( pMyFormat->GetRegisteredIn()) !=  nullptr )
            pMyFormat = static_cast<const SwSectionFormat*>(pMyFormat->GetRegisteredIn());
        else
            return false;
    }
    return true;
}

void SwSectionFrm::CalcFootnoteAtEndFlag()
{
    SwSectionFormat *pFormat = GetSection()->GetFormat();
    sal_uInt16 nVal = pFormat->GetFootnoteAtTextEnd( false ).GetValue();
    bFootnoteAtEnd = FTNEND_ATPGORDOCEND != nVal;
    bOwnFootnoteNum = FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
                 FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
    while( !bFootnoteAtEnd && !bOwnFootnoteNum )
    {
        if( dynamic_cast< const SwSectionFormat *>( pFormat->GetRegisteredIn()) !=  nullptr )
            pFormat = static_cast<SwSectionFormat*>(pFormat->GetRegisteredIn());
        else
            break;
        nVal = pFormat->GetFootnoteAtTextEnd( false ).GetValue();
        if( FTNEND_ATPGORDOCEND != nVal )
        {
            bFootnoteAtEnd = true;
            bOwnFootnoteNum = bOwnFootnoteNum ||FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
                         FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
        }
    }
}

bool SwSectionFrm::IsEndnoteAtMyEnd() const
{
    return pSection->GetFormat()->GetEndAtTextEnd( false ).IsAtEnd();
}

void SwSectionFrm::CalcEndAtEndFlag()
{
    SwSectionFormat *pFormat = GetSection()->GetFormat();
    bEndnAtEnd = pFormat->GetEndAtTextEnd( false ).IsAtEnd();
    while( !bEndnAtEnd )
    {
        if( dynamic_cast< const SwSectionFormat *>( pFormat->GetRegisteredIn()) !=  nullptr )
            pFormat = static_cast<SwSectionFormat*>(pFormat->GetRegisteredIn());
        else
            break;
        bEndnAtEnd = pFormat->GetEndAtTextEnd( false ).IsAtEnd();
    }
}

void SwSectionFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
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
    SwClient::SwClientNotify(rMod, rHint);
    // #i117863#
    const SwSectionFrmMoveAndDeleteHint* pHint =
                    dynamic_cast<const SwSectionFrmMoveAndDeleteHint*>(&rHint);
    if ( pHint && pHint->GetId() == SFX_HINT_DYING && &rMod == GetRegisteredIn() )
    {
        SwSectionFrm::MoveContentAndDelete( this, pHint->IsSaveContent() );
    }
}

void SwSectionFrm::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
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
                SwViewShell *pSh = getRootFrm()->GetCurrShell();
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
            SwLayoutFrm::Modify( pOld, pNew );
    }
}

/// A follow or a ftncontainer at the end of the page causes a maximal Size of the sectionframe.
bool SwSectionFrm::ToMaximize( bool bCheckFollow ) const
{
    if( HasFollow() )
    {
        if( !bCheckFollow ) // Don't check superfluous follows
            return true;
        const SwSectionFrm* pFoll = GetFollow();
        while( pFoll && pFoll->IsSuperfluous() )
            pFoll = pFoll->GetFollow();
        if( pFoll )
            return true;
    }
    if( IsFootnoteAtEnd() )
        return false;
    const SwFootnoteContFrm* pCont = ContainsFootnoteCont();
    if( !IsEndnAtEnd() )
        return 0 != pCont;
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

/// Check every Column for FootnoteContFrms.
SwFootnoteContFrm* SwSectionFrm::ContainsFootnoteCont( const SwFootnoteContFrm* pCont ) const
{
    SwFootnoteContFrm* pRet = NULL;
    const SwLayoutFrm* pLay;
    if( pCont )
    {
        pLay = pCont->FindFootnoteBossFrm();
        OSL_ENSURE( IsAnLower( pLay ), "ConatainsFootnoteCont: Wrong FootnoteContainer" );
        pLay = static_cast<const SwLayoutFrm*>(pLay->GetNext());
    }
    else if( Lower() && Lower()->IsColumnFrm() )
        pLay = static_cast<const SwLayoutFrm*>(Lower());
    else
        pLay = NULL;
    while ( !pRet && pLay )
    {
        if( pLay->Lower() && pLay->Lower()->GetNext() )
        {
            OSL_ENSURE( pLay->Lower()->GetNext()->IsFootnoteContFrm(),
                    "ToMaximize: Unexpected Frame" );
            pRet = const_cast<SwFootnoteContFrm*>(static_cast<const SwFootnoteContFrm*>(pLay->Lower()->GetNext()));
        }
        OSL_ENSURE( !pLay->GetNext() || pLay->GetNext()->IsLayoutFrm(),
                "ToMaximize: ColFrm expected" );
        pLay = static_cast<const SwLayoutFrm*>(pLay->GetNext());
    }
    return pRet;
}

void SwSectionFrm::InvalidateFootnotePos()
{
    SwFootnoteContFrm* pCont = ContainsFootnoteCont();
    if( pCont )
    {
        SwFrm *pTmp = pCont->ContainsContent();
        if( pTmp )
            pTmp->_InvalidatePos();
    }
}

/** Returns the value that the section would like to be
 * greater if it has undersized TextFrms in it,
 * otherwise Null.
 * If necessary the undersized-flag is corrected.
 */
long SwSectionFrm::Undersize( bool bOverSize )
{
    m_bUndersized = false;
    SWRECTFN( this )
    long nRet = InnerHeight() - (Prt().*fnRect->fnGetHeight)();
    if( nRet > 0 )
        m_bUndersized = true;
    else if( !bOverSize )
        nRet = 0;
    return nRet;
}

void SwSectionFrm::CalcFootnoteContent()
{
    vcl::RenderContext* pRenderContext = getRootFrm()->GetCurrShell()->GetOut();
    SwFootnoteContFrm* pCont = ContainsFootnoteCont();
    if( pCont )
    {
        SwFrm* pFrm = pCont->ContainsAny();
        if( pFrm )
            pCont->Calc(pRenderContext);
        while( pFrm && IsAnLower( pFrm ) )
        {
            SwFootnoteFrm* pFootnote = pFrm->FindFootnoteFrm();
            if( pFootnote )
                pFootnote->Calc(pRenderContext);
            pFrm->Calc(pRenderContext);
            if( pFrm->IsSctFrm() )
            {
                SwFrm *pTmp = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
                if( pTmp )
                {
                    pFrm = pTmp;
                    continue;
                }
            }
            pFrm = pFrm->FindNext();
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
    if( !mpDestroy )
        mpDestroy = new SwDestroyList;
    mpDestroy->insert( pDel );
}

void SwRootFrm::_DeleteEmptySct()
{
    OSL_ENSURE( mpDestroy, "Keine Liste, keine Kekse" );
    while( !mpDestroy->empty() )
    {
        SwSectionFrm* pSect = *mpDestroy->begin();
        mpDestroy->erase( mpDestroy->begin() );
        OSL_ENSURE( !pSect->IsColLocked() && !pSect->IsJoinLocked(),
                "DeleteEmptySct: Locked SectionFrm" );
        if( !pSect->Frm().HasArea() && !pSect->ContainsContent() )
        {
            SwLayoutFrm* pUp = pSect->GetUpper();
            pSect->RemoveFromLayout();
            SwFrm::DestroyFrm(pSect);
            if( pUp && !pUp->Lower() )
            {
                if( pUp->IsPageBodyFrm() )
                    pUp->getRootFrm()->SetSuperfluous();
                else if( pUp->IsFootnoteFrm() && !pUp->IsColLocked() &&
                    pUp->GetUpper() )
                {
                    pUp->Cut();
                    SwFrm::DestroyFrm(pUp);
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
    OSL_ENSURE( mpDestroy, "Where's my list?" );
    mpDestroy->erase( pSct );
}

#ifdef DBG_UTIL
bool SwRootFrm::IsInDelList( SwSectionFrm* pSct ) const
{
    return mpDestroy && mpDestroy->find( pSct ) != mpDestroy->end();
}
#endif

bool SwSectionFrm::IsBalancedSection() const
{
    bool bRet = false;
    if ( GetSection() && Lower() && Lower()->IsColumnFrm() && Lower()->GetNext() )
    {
        bRet = !GetSection()->GetFormat()->GetBalancedColumns().GetValue();
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
