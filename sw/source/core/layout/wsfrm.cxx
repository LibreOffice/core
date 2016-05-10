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

#include <hints.hxx>
#include <svl/itemiter.hxx>
#include <editeng/brushitem.hxx>
#include <fmtornt.hxx>
#include <pagefrm.hxx>
#include <section.hxx>
#include <rootfrm.hxx>
#include <anchoreddrawobject.hxx>
#include <fmtanchr.hxx>
#include <viewimp.hxx>
#include "viewopt.hxx"
#include <IDocumentSettingAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <fesh.hxx>
#include <docsh.hxx>
#include <ftninfo.hxx>
#include <fmtclbl.hxx>
#include <fmtfsize.hxx>
#include <fmtpdsc.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <fmtsrnd.hxx>
#include <ftnfrm.hxx>
#include <tabfrm.hxx>
#include <flyfrms.hxx>
#include <sectfrm.hxx>
#include <fmtclds.hxx>
#include <txtfrm.hxx>
#include <bodyfrm.hxx>
#include <cellfrm.hxx>
#include <dbg_lay.hxx>
#include <editeng/frmdiritem.hxx>
#include <sortedobjs.hxx>

using namespace ::com::sun::star;

SwFrame::SwFrame( SwModify *pMod, SwFrame* pSib ) :
    SwClient( pMod ),
    mbIfAccTableShouldDisposing( false ), //A member to identify if the acc table should dispose
    mbInDtor(false),
    mnFrameId( SwFrame::mnLastFrameId++ ),
    mpRoot( pSib ? pSib->getRootFrame() : nullptr ),
    mpUpper(nullptr),
    mpNext(nullptr),
    mpPrev(nullptr),
    mpDrawObjs(nullptr),
    mnFrameType(0),
    mbInfBody( false ),
    mbInfTab ( false ),
    mbInfFly ( false ),
    mbInfFootnote ( false ),
    mbInfSct ( false )
    , m_isInDestroy(false)
{
    OSL_ENSURE( pMod, "No frame format given." );
    mbInvalidR2L = mbInvalidVert = true;
    mbDerivedR2L = mbDerivedVert = mbRightToLeft = mbVertical = mbReverse = mbVertLR = false;

    mbValidPos = mbValidPrtArea = mbValidSize = mbValidLineNum = mbRetouche =
    mbFixSize = mbColLocked = false;
    mbCompletePaint = mbInfInvalid = true;
    mbForbidDelete = false;
}

const IDocumentDrawModelAccess& SwFrame::getIDocumentDrawModelAccess()
{
    return GetUpper()->GetFormat()->getIDocumentDrawModelAccess();
}

bool SwFrame::KnowsFormat( const SwFormat& rFormat ) const
{
    return GetRegisteredIn() == &rFormat;
}

void SwFrame::RegisterToFormat( SwFormat& rFormat )
{
    rFormat.Add( this );
}

void SwFrame::CheckDir( sal_uInt16 nDir, bool bVert, bool bOnlyBiDi, bool bBrowse )
{
    if( FRMDIR_ENVIRONMENT == nDir || ( bVert && bOnlyBiDi ) )
    {
        mbDerivedVert = true;
        if( FRMDIR_ENVIRONMENT == nDir )
            mbDerivedR2L = true;
        SetDirFlags( bVert );
    }
    else if( bVert )
    {
        mbInvalidVert = false;
        if( FRMDIR_HORI_LEFT_TOP == nDir || FRMDIR_HORI_RIGHT_TOP == nDir
            || bBrowse )
        {
            mbVertical = false;
            mbVertLR = false;
        }
        else
           {
            mbVertical = true;
            if(FRMDIR_VERT_TOP_RIGHT == nDir)
                mbVertLR = false;
               else if(FRMDIR_VERT_TOP_LEFT==nDir)
                       mbVertLR = true;
        }
    }
    else
    {
        mbInvalidR2L = false;
        if( FRMDIR_HORI_RIGHT_TOP == nDir )
            mbRightToLeft = true;
        else
            mbRightToLeft = false;
    }
}

void SwFrame::CheckDirection( bool bVert )
{
    if( bVert )
    {
        if( !IsHeaderFrame() && !IsFooterFrame() )
        {
            mbDerivedVert = true;
            SetDirFlags( bVert );
        }
    }
    else
    {
        mbDerivedR2L = true;
        SetDirFlags( bVert );
    }
}

void SwSectionFrame::CheckDirection( bool bVert )
{
    const SwFrameFormat* pFormat = GetFormat();
    if( pFormat )
    {
        const SwViewShell *pSh = getRootFrame()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        CheckDir(static_cast<const SvxFrameDirectionItem&>(pFormat->GetFormatAttr(RES_FRAMEDIR)).GetValue(),
                    bVert, true, bBrowseMode );
    }
    else
        SwFrame::CheckDirection( bVert );
}

void SwFlyFrame::CheckDirection( bool bVert )
{
    const SwFrameFormat* pFormat = GetFormat();
    if( pFormat )
    {
        const SwViewShell *pSh = getRootFrame()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        CheckDir(static_cast<const SvxFrameDirectionItem&>(pFormat->GetFormatAttr(RES_FRAMEDIR)).GetValue(),
                    bVert, false, bBrowseMode );
    }
    else
        SwFrame::CheckDirection( bVert );
}

void SwTabFrame::CheckDirection( bool bVert )
{
    const SwFrameFormat* pFormat = GetFormat();
    if( pFormat )
    {
        const SwViewShell *pSh = getRootFrame()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        CheckDir(static_cast<const SvxFrameDirectionItem&>(pFormat->GetFormatAttr(RES_FRAMEDIR)).GetValue(),
                    bVert, true, bBrowseMode );
    }
    else
        SwFrame::CheckDirection( bVert );
}

void SwCellFrame::CheckDirection( bool bVert )
{
    const SwFrameFormat* pFormat = GetFormat();
    const SfxPoolItem* pItem;
    // Check if the item is set, before actually
    // using it. Otherwise the dynamic pool default is used, which may be set
    // to LTR in case of OOo 1.0 documents.
    if( pFormat && SfxItemState::SET == pFormat->GetItemState( RES_FRAMEDIR, true, &pItem ) )
    {
        const SvxFrameDirectionItem* pFrameDirItem = static_cast<const SvxFrameDirectionItem*>(pItem);
        const SwViewShell *pSh = getRootFrame()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        CheckDir( pFrameDirItem->GetValue(), bVert, false, bBrowseMode );
    }
    else
        SwFrame::CheckDirection( bVert );
}

void SwTextFrame::CheckDirection( bool bVert )
{
    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
    CheckDir( GetNode()->GetSwAttrSet().GetFrameDir().GetValue(), bVert,
              true, bBrowseMode );
}

void SwFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_uInt8 nInvFlags = 0;

    if( pOld && pNew && RES_ATTRSET_CHG == pNew->Which() )
    {
        SfxItemIter aNIter( *static_cast<const SwAttrSetChg*>(pNew)->GetChgSet() );
        SfxItemIter aOIter( *static_cast<const SwAttrSetChg*>(pOld)->GetChgSet() );
        while( true )
        {
            _UpdateAttrFrame( aOIter.GetCurItem(),
                         aNIter.GetCurItem(), nInvFlags );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
    }
    else
        _UpdateAttrFrame( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        SwPageFrame *pPage = FindPageFrame();
        InvalidatePage( pPage );
        if ( nInvFlags & 0x01 )
        {
            _InvalidatePrt();
            if( !GetPrev() && IsTabFrame() && IsInSct() )
                FindSctFrame()->_InvalidatePrt();
        }
        if ( nInvFlags & 0x02 )
            _InvalidateSize();
        if ( nInvFlags & 0x04 )
            _InvalidatePos();
        if ( nInvFlags & 0x08 )
            SetCompletePaint();
        SwFrame *pNxt;
        if ( nInvFlags & 0x30 && nullptr != (pNxt = GetNext()) )
        {
            pNxt->InvalidatePage( pPage );
            if ( nInvFlags & 0x10 )
                pNxt->_InvalidatePos();
            if ( nInvFlags & 0x20 )
                pNxt->SetCompletePaint();
        }
    }
}

void SwFrame::_UpdateAttrFrame( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                         sal_uInt8 &rInvFlags )
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_BOX:
        case RES_SHADOW:
            Prepare( PREP_FIXSIZE_CHG );
            SAL_FALLTHROUGH;
        case RES_LR_SPACE:
        case RES_UL_SPACE:
            rInvFlags |= 0x0B;
            break;

        case RES_HEADER_FOOTER_EAT_SPACING:
            rInvFlags |= 0x03;
            break;

        case RES_BACKGROUND:
            rInvFlags |= 0x28;
            break;

        case RES_KEEP:
            rInvFlags |= 0x04;
            break;

        case RES_FRM_SIZE:
            ReinitializeFrameSizeAttrFlags();
            rInvFlags |= 0x13;
            break;

        case RES_FMT_CHG:
            rInvFlags |= 0x0F;
            break;

        case RES_ROW_SPLIT:
        {
            if ( IsRowFrame() )
            {
                bool bInFollowFlowRow = nullptr != IsInFollowFlowRow();
                if ( bInFollowFlowRow || nullptr != IsInSplitTableRow() )
                {
                    SwTabFrame* pTab = FindTabFrame();
                    if ( bInFollowFlowRow )
                        pTab = pTab->FindMaster();
                    pTab->SetRemoveFollowFlowLinePending( true );
                }
            }
            break;
        }
        case RES_COL:
            OSL_FAIL( "Columns for new FrameTyp?" );
            break;

        default:
            //UUUU the new FillStyle has to do the same as previous RES_BACKGROUND
            if(nWhich >= XATTR_FILL_FIRST && nWhich <= XATTR_FILL_LAST)
            {
                rInvFlags |= 0x28;
            }
            /* do Nothing */;
    }
}

bool SwFrame::Prepare( const PrepareHint, const void *, bool )
{
    /* Do nothing */
    return false;
}

/**
 * Invalidates the page in which the Frame is currently placed.
 * The page is invalidated depending on the type (Layout, Content, FlyFrame)
 */
void SwFrame::InvalidatePage( const SwPageFrame *pPage ) const
{
    if ( !pPage )
    {
        pPage = FindPageFrame();
        // #i28701# - for at-character and as-character
        // anchored Writer fly frames additionally invalidate also page frame
        // its 'anchor character' is on.
        if ( pPage && pPage->GetUpper() && IsFlyFrame() )
        {
            const SwFlyFrame* pFlyFrame = static_cast<const SwFlyFrame*>(this);
            if ( pFlyFrame->IsAutoPos() || pFlyFrame->IsFlyInContentFrame() )
            {
                // #i33751#, #i34060# - method <GetPageFrameOfAnchor()>
                // is replaced by method <FindPageFrameOfAnchor()>. It's return value
                // have to be checked.
                SwPageFrame* pPageFrameOfAnchor =
                        const_cast<SwFlyFrame*>(pFlyFrame)->FindPageFrameOfAnchor();
                if ( pPageFrameOfAnchor && pPageFrameOfAnchor != pPage )
                {
                    InvalidatePage( pPageFrameOfAnchor );
                }
            }
        }
    }

    if ( pPage && pPage->GetUpper() )
    {
        if ( pPage->GetFormat()->GetDoc()->IsInDtor() )
            return;

        SwRootFrame *pRoot = const_cast<SwRootFrame*>(static_cast<const SwRootFrame*>(pPage->GetUpper()));
        const SwFlyFrame *pFly = FindFlyFrame();
        if ( IsContentFrame() )
        {
            if ( pRoot->IsTurboAllowed() )
            {
                // If a ContentFrame wants to register for a second time, make it a TurboAction.
                if ( !pRoot->GetTurbo() || this == pRoot->GetTurbo() )
                    pRoot->SetTurbo( static_cast<const SwContentFrame*>(this) );
                else
                {
                    pRoot->DisallowTurbo();
                    //The page of the Turbo could be a different one then mine,
                    //therefore we have to invalidate it.
                    const SwFrame *pTmp = pRoot->GetTurbo();
                    pRoot->ResetTurbo();
                    pTmp->InvalidatePage();
                }
            }
            if ( !pRoot->GetTurbo() )
            {
                if ( pFly )
                {   if( !pFly->IsLocked() )
                    {
                        if ( pFly->IsFlyInContentFrame() )
                        {   pPage->InvalidateFlyInCnt();
                            static_cast<const SwFlyInContentFrame*>(pFly)->InvalidateContent();
                            pFly->GetAnchorFrame()->InvalidatePage();
                        }
                        else
                            pPage->InvalidateFlyContent();
                    }
                }
                else
                    pPage->InvalidateContent();
            }
        }
        else
        {
            pRoot->DisallowTurbo();
            if ( pFly )
            {
                if ( !pFly->IsLocked() )
                {
                    if ( pFly->IsFlyInContentFrame() )
                    {
                        pPage->InvalidateFlyInCnt();
                        static_cast<const SwFlyInContentFrame*>(pFly)->InvalidateLayout();
                        pFly->GetAnchorFrame()->InvalidatePage();
                    }
                    else
                        pPage->InvalidateFlyLayout();
                }
            }
            else
                pPage->InvalidateLayout();

            if ( pRoot->GetTurbo() )
            {   const SwFrame *pTmp = pRoot->GetTurbo();
                pRoot->ResetTurbo();
                pTmp->InvalidatePage();
            }
        }
        pRoot->SetIdleFlags();

        const SwTextNode *pTextNode = dynamic_cast< const SwTextNode * >(GetDep());
        if (pTextNode && pTextNode->IsGrammarCheckDirty())
            pRoot->SetNeedGrammarCheck( true );
    }
}

Size SwFrame::ChgSize( const Size& aNewSize )
{
    mbFixSize = true;
    const Size aOldSize( Frame().SSize() );
    if ( aNewSize == aOldSize )
        return aOldSize;

    if ( GetUpper() )
    {
        SWRECTFN2( this )
        SwRect aNew( Point(0,0), aNewSize );
        (maFrame.*fnRect->fnSetWidth)( (aNew.*fnRect->fnGetWidth)() );
        long nNew = (aNew.*fnRect->fnGetHeight)();
        long nDiff = nNew - (maFrame.*fnRect->fnGetHeight)();
        if( nDiff )
        {
            if ( GetUpper()->IsFootnoteBossFrame() && HasFixSize() &&
                 NA_GROW_SHRINK !=
                 static_cast<SwFootnoteBossFrame*>(GetUpper())->NeighbourhoodAdjustment( this ) )
            {
                (maFrame.*fnRect->fnSetHeight)( nNew );
                SwTwips nReal = static_cast<SwLayoutFrame*>(this)->AdjustNeighbourhood(nDiff);
                if ( nReal != nDiff )
                    (maFrame.*fnRect->fnSetHeight)( nNew - nDiff + nReal );
            }
            else
            {
                // OD 24.10.2002 #97265# - grow/shrink not for neighbour frames
                // NOTE: neighbour frames are cell and column frames.
                if ( !bNeighb )
                {
                    if ( nDiff > 0 )
                        Grow( nDiff );
                    else
                        Shrink( -nDiff );

                    if ( GetUpper() && (maFrame.*fnRect->fnGetHeight)() != nNew )
                        GetUpper()->_InvalidateSize();
                }

                // Even if grow/shrink did not yet set the desired width, for
                // example when called by ChgColumns to set the column width, we
                // set the right width now.
                (maFrame.*fnRect->fnSetHeight)( nNew );
            }
        }
    }
    else
        maFrame.SSize( aNewSize );

    if ( Frame().SSize() != aOldSize )
    {
        SwPageFrame *pPage = FindPageFrame();
        if ( GetNext() )
        {
            GetNext()->_InvalidatePos();
            GetNext()->InvalidatePage( pPage );
        }
        if( IsLayoutFrame() )
        {
            if( IsRightToLeft() )
                _InvalidatePos();
            if( static_cast<SwLayoutFrame*>(this)->Lower() )
                static_cast<SwLayoutFrame*>(this)->Lower()->_InvalidateSize();
        }
        _InvalidatePrt();
        _InvalidateSize();
        InvalidatePage( pPage );
    }

    return maFrame.SSize();
}

/** Insert SwFrame into existing structure.
 *
 * Insertion is done below the parent either before pBehind or
 * at the end of the chain if pBehind is empty.
 */
void SwFrame::InsertBefore( SwLayoutFrame* pParent, SwFrame* pBehind )
{
    OSL_ENSURE( pParent, "No parent for insert." );
    OSL_ENSURE( (!pBehind || pParent == pBehind->GetUpper()),
            "Frame tree is inconsistent." );

    mpUpper = pParent;
    mpNext = pBehind;
    if( pBehind )
    {   //Insert before pBehind.
        if( nullptr != (mpPrev = pBehind->mpPrev) )
            mpPrev->mpNext = this;
        else
            mpUpper->m_pLower = this;
        pBehind->mpPrev = this;
    }
    else
    {   //Insert at the end, or as first node in the sub tree
        mpPrev = mpUpper->Lower();
        if ( mpPrev )
        {
            while( mpPrev->mpNext )
                mpPrev = mpPrev->mpNext;
            mpPrev->mpNext = this;
        }
        else
            mpUpper->m_pLower = this;
    }
}

/** Insert SwFrame into existing structure.
 *
 * Insertion is done below the parent either after pBehind or
 * at the beginning of the chain if pBehind is empty.
 */
void SwFrame::InsertBehind( SwLayoutFrame *pParent, SwFrame *pBefore )
{
    OSL_ENSURE( pParent, "No Parent for Insert." );
    OSL_ENSURE( (!pBefore || pParent == pBefore->GetUpper()),
            "Frame tree is inconsistent." );

    mpUpper = pParent;
    mpPrev = pBefore;
    if ( pBefore )
    {
        //Insert after pBefore
        if ( nullptr != (mpNext = pBefore->mpNext) )
            mpNext->mpPrev = this;
        pBefore->mpNext = this;
    }
    else
    {
        //Insert at the beginning of the chain
        mpNext = pParent->Lower();
        if ( pParent->Lower() )
            pParent->Lower()->mpPrev = this;
        pParent->m_pLower = this;
    }
}

/** Insert a chain of SwFrames into an existing structure
 *
 * Currently, this method is used to insert a SectionFrame (which may have some siblings) into an
 * existing structure. If the third parameter is NULL, this method is (besides handling the
 * siblings) equal to SwFrame::InsertBefore(..).
 *
 * If the third parameter is passed, the following happens:
 *  - this becomes mpNext of pParent
 *  - pSct becomes mpNext of the last one in the this-chain
 *  - pBehind is reconnected from pParent to pSct
 * The purpose is: a SectionFrame (this) won't become a child of another SectionFrame (pParent), but
 * pParent gets split into two siblings (pParent+pSect) and this is inserted between.
 */
bool SwFrame::InsertGroupBefore( SwFrame* pParent, SwFrame* pBehind, SwFrame* pSct )
{
    OSL_ENSURE( pParent, "No parent for insert." );
    OSL_ENSURE( (!pBehind || ( (pBehind && (pParent == pBehind->GetUpper()))
            || ((pParent->IsSctFrame() && pBehind->GetUpper()->IsColBodyFrame())) ) ),
            "Frame tree inconsistent." );
    if( pSct )
    {
        mpUpper = pParent->GetUpper();
        SwFrame *pLast = this;
        while( pLast->GetNext() )
        {
            pLast = pLast->GetNext();
            pLast->mpUpper = GetUpper();
        }
        if( pBehind )
        {
            pLast->mpNext = pSct;
            pSct->mpPrev = pLast;
            pSct->mpNext = pParent->GetNext();
        }
        else
        {
            pLast->mpNext = pParent->GetNext();
            if( pLast->GetNext() )
                pLast->GetNext()->mpPrev = pLast;
        }
        pParent->mpNext = this;
        mpPrev = pParent;
        if( pSct->GetNext() )
            pSct->GetNext()->mpPrev = pSct;
        while( pLast->GetNext() )
        {
            pLast = pLast->GetNext();
            pLast->mpUpper = GetUpper();
        }
        if( pBehind )
        {   // Insert before pBehind.
            if( pBehind->GetPrev() )
                pBehind->GetPrev()->mpNext = nullptr;
            else
                pBehind->GetUpper()->m_pLower = nullptr;
            pBehind->mpPrev = nullptr;
            SwLayoutFrame* pTmp = static_cast<SwLayoutFrame*>(pSct);
            if( pTmp->Lower() )
            {
                OSL_ENSURE( pTmp->Lower()->IsColumnFrame(), "InsertGrp: Used SectionFrame" );
                pTmp = static_cast<SwLayoutFrame*>(static_cast<SwLayoutFrame*>(pTmp->Lower())->Lower());
                OSL_ENSURE( pTmp, "InsertGrp: Missing ColBody" );
            }
            pBehind->mpUpper = pTmp;
            pBehind->GetUpper()->m_pLower = pBehind;
            pLast = pBehind->GetNext();
            while ( pLast )
            {
                pLast->mpUpper = pBehind->GetUpper();
                pLast = pLast->GetNext();
            }
        }
        else
        {
            OSL_ENSURE( pSct->IsSctFrame(), "InsertGroup: For SectionFrames only" );
            SwFrame::DestroyFrame(pSct);
            return false;
        }
    }
    else
    {
        mpUpper = static_cast<SwLayoutFrame*>(pParent);
        SwFrame *pLast = this;
        while( pLast->GetNext() )
        {
            pLast = pLast->GetNext();
            pLast->mpUpper = GetUpper();
        }
        pLast->mpNext = pBehind;
        if( pBehind )
        {   // Insert before pBehind.
            if( nullptr != (mpPrev = pBehind->mpPrev) )
                mpPrev->mpNext = this;
            else
                mpUpper->m_pLower = this;
            pBehind->mpPrev = pLast;
        }
        else
        {
            //Insert at the end, or ... the first node in the subtree
            mpPrev = mpUpper->Lower();
            if ( mpPrev )
            {
                while( mpPrev->mpNext )
                    mpPrev = mpPrev->mpNext;
                mpPrev->mpNext = this;
            }
            else
                mpUpper->m_pLower = this;
        }
    }
    return true;
}

void SwFrame::RemoveFromLayout()
{
    OSL_ENSURE( mpUpper, "Remove without upper?" );

    if( mpPrev )
        // one out of the middle is removed
        mpPrev->mpNext = mpNext;
    else
    {   // the first in a list is removed //TODO
        OSL_ENSURE( mpUpper->m_pLower == this, "Layout is inconsistent." );
        mpUpper->m_pLower = mpNext;
    }
    if( mpNext )
        mpNext->mpPrev = mpPrev;

    // Remove link
    mpNext  = mpPrev  = nullptr;
    mpUpper = nullptr;
}

void SwContentFrame::Paste( SwFrame* pParent, SwFrame* pSibling)
{
    OSL_ENSURE( pParent, "No parent for pasting." );
    OSL_ENSURE( pParent->IsLayoutFrame(), "Parent is ContentFrame." );
    OSL_ENSURE( pParent != this, "I'm the parent." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I'm still registered somewhere" );
    OSL_ENSURE( !pSibling || pSibling->IsFlowFrame(),
            "<SwContentFrame::Paste(..)> - sibling not of expected type." );

    //Insert in the tree.
    InsertBefore( static_cast<SwLayoutFrame*>(pParent), pSibling );

    SwPageFrame *pPage = FindPageFrame();
    _InvalidateAll();
    InvalidatePage( pPage );

    if( pPage )
    {
        pPage->InvalidateSpelling();
        pPage->InvalidateSmartTags();
        pPage->InvalidateAutoCompleteWords();
        pPage->InvalidateWordCount();
    }

    if ( GetNext() )
    {
        SwFrame* pNxt = GetNext();
        pNxt->_InvalidatePrt();
        pNxt->_InvalidatePos();
        pNxt->InvalidatePage( pPage );
        if( pNxt->IsSctFrame() )
            pNxt = static_cast<SwSectionFrame*>(pNxt)->ContainsContent();
        if( pNxt && pNxt->IsTextFrame() && pNxt->IsInFootnote() )
            pNxt->Prepare( PREP_FTN, nullptr, false );
    }

    if ( Frame().Height() )
        pParent->Grow( Frame().Height() );

    if ( Frame().Width() != pParent->Prt().Width() )
        Prepare( PREP_FIXSIZE_CHG );

    if ( GetPrev() )
    {
        if ( IsFollow() )
            //I'm a direct follower of my master now
            static_cast<SwContentFrame*>(GetPrev())->Prepare( PREP_FOLLOW_FOLLOWS );
        else
        {
            if ( GetPrev()->Frame().Height() !=
                 GetPrev()->Prt().Height() + GetPrev()->Prt().Top() )
            {
                // Take the border into account?
                GetPrev()->_InvalidatePrt();
            }
            // OD 18.02.2003 #104989# - force complete paint of previous frame,
            // if frame is inserted at the end of a section frame, in order to
            // get subsidiary lines repainted for the section.
            if ( pParent->IsSctFrame() && !GetNext() )
            {
                // force complete paint of previous frame, if new inserted frame
                // in the section is the last one.
                GetPrev()->SetCompletePaint();
            }
            GetPrev()->InvalidatePage( pPage );
        }
    }
    if ( IsInFootnote() )
    {
        SwFrame* pFrame = GetIndPrev();
        if( pFrame && pFrame->IsSctFrame() )
            pFrame = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
        if( pFrame )
            pFrame->Prepare( PREP_QUOVADIS, nullptr, false );
        if( !GetNext() )
        {
            pFrame = FindFootnoteFrame()->GetNext();
            if( pFrame && nullptr != (pFrame=static_cast<SwLayoutFrame*>(pFrame)->ContainsAny()) )
                pFrame->_InvalidatePrt();
        }
    }

    _InvalidateLineNum();
    SwFrame *pNxt = FindNextCnt();
    if ( pNxt  )
    {
        while ( pNxt && pNxt->IsInTab() )
        {
            if( nullptr != (pNxt = pNxt->FindTabFrame()) )
                pNxt = pNxt->FindNextCnt();
        }
        if ( pNxt )
        {
            pNxt->_InvalidateLineNum();
            if ( pNxt != GetNext() )
                pNxt->InvalidatePage();
        }
    }
}

void SwContentFrame::Cut()
{
    OSL_ENSURE( GetUpper(), "Cut without Upper()." );

    SwPageFrame *pPage = FindPageFrame();
    InvalidatePage( pPage );
    SwFrame *pFrame = GetIndPrev();
    if( pFrame )
    {
        if( pFrame->IsSctFrame() )
            pFrame = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
        if ( pFrame && pFrame->IsContentFrame() )
        {
            pFrame->_InvalidatePrt();
            if( IsInFootnote() )
                pFrame->Prepare( PREP_QUOVADIS, nullptr, false );
        }
        // #i26250# - invalidate printing area of previous
        // table frame.
        else if ( pFrame && pFrame->IsTabFrame() )
        {
            pFrame->InvalidatePrt();
        }
    }

    SwFrame *pNxt = FindNextCnt();
    if ( pNxt )
    {
        while ( pNxt && pNxt->IsInTab() )
        {
            if( nullptr != (pNxt = pNxt->FindTabFrame()) )
                pNxt = pNxt->FindNextCnt();
        }
        if ( pNxt )
        {
            pNxt->_InvalidateLineNum();
            if ( pNxt != GetNext() )
                pNxt->InvalidatePage();
        }
    }

    if( nullptr != (pFrame = GetIndNext()) )
    {
        // The old follow may have calculated a gap to the predecessor which
        // now becomes obsolete or different as it becomes the first one itself
        pFrame->_InvalidatePrt();
        pFrame->_InvalidatePos();
        pFrame->InvalidatePage( pPage );
        if( pFrame->IsSctFrame() )
        {
            pFrame = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
            if( pFrame )
            {
                pFrame->_InvalidatePrt();
                pFrame->_InvalidatePos();
                pFrame->InvalidatePage( pPage );
            }
        }
        if( pFrame && IsInFootnote() )
            pFrame->Prepare( PREP_ERGOSUM, nullptr, false );
        if( IsInSct() && !GetPrev() )
        {
            SwSectionFrame* pSct = FindSctFrame();
            if( !pSct->IsFollow() )
            {
                pSct->_InvalidatePrt();
                pSct->InvalidatePage( pPage );
            }
        }
    }
    else
    {
        InvalidateNextPos();
        //Someone needs to do the retouching: predecessor or upper
        if ( nullptr != (pFrame = GetPrev()) )
        {   pFrame->SetRetouche();
            pFrame->Prepare( PREP_WIDOWS_ORPHANS );
            pFrame->_InvalidatePos();
            pFrame->InvalidatePage( pPage );
        }
        // If I'm (was) the only ContentFrame in my upper, it has to do the
        // retouching. Also, perhaps a page became empty.
        else
        {   SwRootFrame *pRoot = getRootFrame();
            if ( pRoot )
            {
                pRoot->SetSuperfluous();
                GetUpper()->SetCompletePaint();
                GetUpper()->InvalidatePage( pPage );
            }
            if( IsInSct() )
            {
                SwSectionFrame* pSct = FindSctFrame();
                if( !pSct->IsFollow() )
                {
                    pSct->_InvalidatePrt();
                    pSct->InvalidatePage( pPage );
                }
            }
            // #i52253# The master table should take care
            // of removing the follow flow line.
            if ( IsInTab() )
            {
                SwTabFrame* pThisTab = FindTabFrame();
                SwTabFrame* pMasterTab = pThisTab && pThisTab->IsFollow() ? pThisTab->FindMaster() : nullptr;
                if ( pMasterTab )
                {
                    pMasterTab->_InvalidatePos();
                    pMasterTab->SetRemoveFollowFlowLinePending( true );
                }
            }
        }
    }
    //Remove first, then shrink the upper.
    SwLayoutFrame *pUp = GetUpper();
    RemoveFromLayout();
    if ( pUp )
    {
        SwSectionFrame *pSct = nullptr;
        if ( !pUp->Lower() &&
             ( ( pUp->IsFootnoteFrame() && !pUp->IsColLocked() ) ||
               ( pUp->IsInSct() &&
                 // #i29438#
                 // We have to consider the case that the section may be "empty"
                 // except from a temporary empty table frame.
                 // This can happen due to the new cell split feature.
                 !pUp->IsCellFrame() &&
                 // #126020# - adjust check for empty section
                 // #130797# - correct fix #126020#
                 !(pSct = pUp->FindSctFrame())->ContainsContent() &&
                 !pSct->ContainsAny( true ) ) ) )
        {
            if ( pUp->GetUpper() )
            {

                // prevent delete of <ColLocked> footnote frame
                if ( pUp->IsFootnoteFrame() && !pUp->IsColLocked())
                {
                    if( pUp->GetNext() && !pUp->GetPrev() )
                    {
                        SwFrame* pTmp = static_cast<SwLayoutFrame*>(pUp->GetNext())->ContainsAny();
                        if( pTmp )
                            pTmp->_InvalidatePrt();
                    }
                    pUp->Cut();
                    SwFrame::DestroyFrame(pUp);
                }
                else
                {

                    if ( pSct->IsColLocked() || !pSct->IsInFootnote() ||
                         ( pUp->IsFootnoteFrame() && pUp->IsColLocked() ) )
                    {
                        pSct->DelEmpty( false );
                        // If a locked section may not be deleted then at least
                        // its size became invalid after removing its last
                        // content.
                        pSct->_InvalidateSize();
                    }
                    else
                    {
                        pSct->DelEmpty( true );
                        SwFrame::DestroyFrame(pSct);
                    }
                }
            }
        }
        else
        {
            SWRECTFN( this )
            long nFrameHeight = (Frame().*fnRect->fnGetHeight)();
            if( nFrameHeight )
                pUp->Shrink( nFrameHeight );
        }
    }
}

void SwLayoutFrame::Paste( SwFrame* pParent, SwFrame* pSibling)
{
    OSL_ENSURE( pParent, "No parent for pasting." );
    OSL_ENSURE( pParent->IsLayoutFrame(), "Parent is ContentFrame." );
    OSL_ENSURE( pParent != this, "I'm the parent oneself." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I'm still registered somewhere." );

    //Insert in the tree.
    InsertBefore( static_cast<SwLayoutFrame*>(pParent), pSibling );

    // OD 24.10.2002 #103517# - correct setting of variable <fnRect>
    // <fnRect> is used for the following:
    // (1) To invalidate the frame's size, if its size, which has to be the
    //      same as its upper/parent, differs from its upper's/parent's.
    // (2) To adjust/grow the frame's upper/parent, if it has a dimension in its
    //      size, which is not determined by its upper/parent.
    // Which size is which depends on the frame type and the layout direction
    // (vertical or horizontal).
    // There are the following cases:
    // (A) Header and footer frames both in vertical and in horizontal layout
    //      have to size the width to the upper/parent. A dimension in the height
    //      has to cause a adjustment/grow of the upper/parent.
    //      --> <fnRect> = fnRectHori
    // (B) Cell and column frames in vertical layout, the width has to be the
    //          same as upper/parent and a dimension in height causes adjustment/grow
    //          of the upper/parent.
    //          --> <fnRect> = fnRectHori
    //      in horizontal layout the other way around
    //          --> <fnRect> = fnRectVert
    // (C) Other frames in vertical layout, the height has to be the
    //          same as upper/parent and a dimension in width causes adjustment/grow
    //          of the upper/parent.
    //          --> <fnRect> = fnRectVert
    //      in horizontal layout the other way around
    //          --> <fnRect> = fnRectHori
    //SwRectFn fnRect = IsVertical() ? fnRectHori : fnRectVert;
    SwRectFn fnRect;
    if ( IsHeaderFrame() || IsFooterFrame() )
        fnRect = fnRectHori;
    else if ( IsCellFrame() || IsColumnFrame() )
        fnRect = GetUpper()->IsVertical() ? fnRectHori : ( GetUpper()->IsVertLR() ? fnRectVertL2R : fnRectVert );
    else
        fnRect = GetUpper()->IsVertical() ? ( GetUpper()->IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;

    if( (Frame().*fnRect->fnGetWidth)() != (pParent->Prt().*fnRect->fnGetWidth)())
        _InvalidateSize();
    _InvalidatePos();
    const SwPageFrame *pPage = FindPageFrame();
    InvalidatePage( pPage );
    if( !IsColumnFrame() )
    {
        SwFrame *pFrame = GetIndNext();
        if( nullptr != pFrame )
        {
            pFrame->_InvalidatePos();
            if( IsInFootnote() )
            {
                if( pFrame->IsSctFrame() )
                    pFrame = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
                if( pFrame )
                    pFrame->Prepare( PREP_ERGOSUM, nullptr, false );
            }
        }
        if ( IsInFootnote() && nullptr != ( pFrame = GetIndPrev() ) )
        {
            if( pFrame->IsSctFrame() )
                pFrame = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
            if( pFrame )
                pFrame->Prepare( PREP_QUOVADIS, nullptr, false );
        }
    }

    if( (Frame().*fnRect->fnGetHeight)() )
    {
        // AdjustNeighbourhood is now also called in columns which are not
        // placed inside a frame
        sal_uInt8 nAdjust = GetUpper()->IsFootnoteBossFrame() ?
                static_cast<SwFootnoteBossFrame*>(GetUpper())->NeighbourhoodAdjustment( this )
                : NA_GROW_SHRINK;
        SwTwips nGrow = (Frame().*fnRect->fnGetHeight)();
        if( NA_ONLY_ADJUST == nAdjust )
            AdjustNeighbourhood( nGrow );
        else
        {
            SwTwips nReal = 0;
            if( NA_ADJUST_GROW == nAdjust )
                nReal = AdjustNeighbourhood( nGrow );
            if( nReal < nGrow )
                nReal += pParent->Grow( nGrow - nReal );
            if( NA_GROW_ADJUST == nAdjust && nReal < nGrow )
                AdjustNeighbourhood( nGrow - nReal );
        }
    }
}

void SwLayoutFrame::Cut()
{
    if ( GetNext() )
        GetNext()->_InvalidatePos();

    SWRECTFN( this )
    SwTwips nShrink = (Frame().*fnRect->fnGetHeight)();

    // Remove first, then shrink upper.
    SwLayoutFrame *pUp = GetUpper();

    // AdjustNeighbourhood is now also called in columns which are not
    // placed inside a frame.

    // Remove must not be called before an AdjustNeighbourhood, but it has to
    // be called before the upper-shrink-call, if the upper-shrink takes care
    // of its content.
    if ( pUp && nShrink )
    {
        if( pUp->IsFootnoteBossFrame() )
        {
            sal_uInt8 nAdjust= static_cast<SwFootnoteBossFrame*>(pUp)->NeighbourhoodAdjustment( this );
            if( NA_ONLY_ADJUST == nAdjust )
                AdjustNeighbourhood( -nShrink );
            else
            {
                SwTwips nReal = 0;
                if( NA_ADJUST_GROW == nAdjust )
                    nReal = -AdjustNeighbourhood( -nShrink );
                if( nReal < nShrink )
                {
                    SwTwips nOldHeight = (Frame().*fnRect->fnGetHeight)();
                    (Frame().*fnRect->fnSetHeight)( 0 );
                    nReal += pUp->Shrink( nShrink - nReal );
                    (Frame().*fnRect->fnSetHeight)( nOldHeight );
                }
                if( NA_GROW_ADJUST == nAdjust && nReal < nShrink )
                    AdjustNeighbourhood( nReal - nShrink );
            }
            RemoveFromLayout();
        }
        else
        {
            RemoveFromLayout();
            pUp->Shrink( nShrink );
        }
    }
    else
        RemoveFromLayout();

    if( pUp && !pUp->Lower() )
    {
        pUp->SetCompletePaint();
        pUp->InvalidatePage();
    }
}

SwTwips SwFrame::Grow( SwTwips nDist, bool bTst, bool bInfo )
{
    OSL_ENSURE( nDist >= 0, "Negative growth?" );

    PROTOCOL_ENTER( this, bTst ? PROT_GROW_TST : PROT_GROW, 0, &nDist )

    if ( nDist )
    {
        SWRECTFN( this )

        SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();
        if( nPrtHeight > 0 && nDist > (LONG_MAX - nPrtHeight) )
            nDist = LONG_MAX - nPrtHeight;

        if ( IsFlyFrame() )
            return static_cast<SwFlyFrame*>(this)->_Grow( nDist, bTst );
        else if( IsSctFrame() )
            return static_cast<SwSectionFrame*>(this)->_Grow( nDist, bTst );
        else
        {
            const SwCellFrame* pThisCell = dynamic_cast<const SwCellFrame*>(this);
            if ( pThisCell )
            {
                const SwTabFrame* pTab = FindTabFrame();

                // NEW TABLES
                if ( pTab->IsVertical() != IsVertical() ||
                     pThisCell->GetLayoutRowSpan() < 1 )
                    return 0;
            }

            const SwTwips nReal = GrowFrame( nDist, bTst, bInfo );
            if( !bTst )
            {
                nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                (Prt().*fnRect->fnSetHeight)( nPrtHeight +
                        ( IsContentFrame() ? nDist : nReal ) );
            }
            return nReal;
        }
    }
    return 0L;
}

SwTwips SwFrame::Shrink( SwTwips nDist, bool bTst, bool bInfo )
{
    OSL_ENSURE( nDist >= 0, "Negative reduction?" );

    PROTOCOL_ENTER( this, bTst ? PROT_SHRINK_TST : PROT_SHRINK, 0, &nDist )

    if ( nDist )
    {
        if ( IsFlyFrame() )
            return static_cast<SwFlyFrame*>(this)->_Shrink( nDist, bTst );
        else if( IsSctFrame() )
            return static_cast<SwSectionFrame*>(this)->_Shrink( nDist, bTst );
        else
        {
            const SwCellFrame* pThisCell = dynamic_cast<const SwCellFrame*>(this);
            if ( pThisCell )
            {
                const SwTabFrame* pTab = FindTabFrame();

                // NEW TABLES
                if ( pTab->IsVertical() != IsVertical() ||
                     pThisCell->GetLayoutRowSpan() < 1 )
                    return 0;
            }

            SWRECTFN( this )
            SwTwips nReal = (Frame().*fnRect->fnGetHeight)();
            ShrinkFrame( nDist, bTst, bInfo );
            nReal -= (Frame().*fnRect->fnGetHeight)();
            if( !bTst )
            {
                const SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                (Prt().*fnRect->fnSetHeight)( nPrtHeight -
                        ( IsContentFrame() ? nDist : nReal ) );
            }
            return nReal;
        }
    }
    return 0L;
}

/** Adjust surrounding neighbourhood after insertion
 *
 * A Frame needs "normalization" if it is directly placed below a footnote boss (page/column) and its
 * size changes. There is always a frame that takes the maximum possible space (the frame that
 * contains the Body text) and zero or more frames which only take the space needed (header/footer
 * area, footnote container). If one of these frames changes, the body-text-frame has to grow or
 * shrink accordingly, even tough it's fixed.
 *
 * !! Is it possible to do this in a generic way and not restrict it to the page and a distinct
 * frame which takes the maximum space (controlled using the FrameSize attribute)?
 * Problems:
 *   - What if multiple frames taking the maximum space are placed next to each other?
 *   - How is the maximum space calculated?
 *   - How small can those frames become?
 *
 * In any case, only a certain amount of space is allowed, so we never go below a minimum value for
 * the height of the body.
 *
 * @param nDiff the value around which the space has to be allocated
 */
SwTwips SwFrame::AdjustNeighbourhood( SwTwips nDiff, bool bTst )
{
    PROTOCOL_ENTER( this, PROT_ADJUSTN, 0, &nDiff );

    if ( !nDiff || !GetUpper()->IsFootnoteBossFrame() ) // only inside pages/columns
        return 0L;

    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();

    //The (Page-)Body only changes in BrowseMode, but only if it does not
    //contain columns.
    if ( IsPageBodyFrame() && (!bBrowse ||
          (static_cast<SwLayoutFrame*>(this)->Lower() &&
           static_cast<SwLayoutFrame*>(this)->Lower()->IsColumnFrame())) )
        return 0L;

    //In BrowseView mode the PageFrame can handle some of the requests.
    long nBrowseAdd = 0;
    if ( bBrowse && GetUpper()->IsPageFrame() ) // only (Page-)BodyFrames
    {
        SwViewShell *pViewShell = getRootFrame()->GetCurrShell();
        SwLayoutFrame *pUp = GetUpper();
        long nChg;
        const long nUpPrtBottom = pUp->Frame().Height() -
                                  pUp->Prt().Height() - pUp->Prt().Top();
        SwRect aInva( pUp->Frame() );
        if ( pViewShell )
        {
            aInva.Pos().X() = pViewShell->VisArea().Left();
            aInva.Width( pViewShell->VisArea().Width() );
        }
        if ( nDiff > 0 )
        {
            nChg = BROWSE_HEIGHT - pUp->Frame().Height();
            nChg = std::min( nDiff, nChg );

            if ( !IsBodyFrame() )
            {
                SetCompletePaint();
                if ( !pViewShell || pViewShell->VisArea().Height() >= pUp->Frame().Height() )
                {
                    //First minimize Body, it will grow again later.
                    SwFrame *pBody = static_cast<SwFootnoteBossFrame*>(pUp)->FindBodyCont();
                    const long nTmp = nChg - pBody->Prt().Height();
                    if ( !bTst )
                    {
                        pBody->Frame().Height(std::max( 0L, pBody->Frame().Height() - nChg ));
                        pBody->_InvalidatePrt();
                        pBody->_InvalidateSize();
                        if ( pBody->GetNext() )
                            pBody->GetNext()->_InvalidatePos();
                        if ( !IsHeaderFrame() )
                            pBody->SetCompletePaint();
                    }
                    nChg = nTmp <= 0 ? 0 : nTmp;
                }
            }

            const long nTmp = nUpPrtBottom + 20;
            aInva.Top( aInva.Bottom() - nTmp );
            aInva.Height( nChg + nTmp );
        }
        else
        {
            //The page can shrink to 0. The fist page keeps the same size like
            //VisArea.
            nChg = nDiff;
            long nInvaAdd = 0;
            if ( pViewShell && !pUp->GetPrev() &&
                 pUp->Frame().Height() + nDiff < pViewShell->VisArea().Height() )
            {
                // This means that we have to invalidate adequately.
                nChg = pViewShell->VisArea().Height() - pUp->Frame().Height();
                nInvaAdd = -(nDiff - nChg);
            }

            //Invalidate including bottom border.
            long nBorder = nUpPrtBottom + 20;
            nBorder -= nChg;
            aInva.Top( aInva.Bottom() - (nBorder+nInvaAdd) );
            if ( !IsBodyFrame() )
            {
                SetCompletePaint();
                if ( !IsHeaderFrame() )
                    static_cast<SwFootnoteBossFrame*>(pUp)->FindBodyCont()->SetCompletePaint();
            }
            //Invalidate the page because of the frames. Thereby the page becomes
            //the right size again if a frame didn't fit. This only works
            //randomly for paragraph bound frames otherwise (NotifyFlys).
            pUp->InvalidateSize();
        }
        if ( !bTst )
        {
            //Independent from nChg
            if ( pViewShell && aInva.HasArea() && pUp->GetUpper() )
                pViewShell->InvalidateWindows( aInva );
        }
        if ( !bTst && nChg )
        {
            pUp->Frame().SSize().Height() += nChg;
            pUp->Prt().SSize().Height() += nChg;
            if ( pViewShell )
                pViewShell->Imp()->SetFirstVisPageInvalid();

            if ( GetNext() )
                GetNext()->_InvalidatePos();

            //Trigger a repaint if necessary.
            SvxBrushItem aBack(pUp->GetFormat()->makeBackgroundBrushItem());
            const SvxGraphicPosition ePos = aBack.GetGraphicPos();
            if ( ePos != GPOS_NONE && ePos != GPOS_TILED )
                pViewShell->InvalidateWindows( pUp->Frame() );

            if ( pUp->GetUpper() )
            {
                if ( pUp->GetNext() )
                    pUp->GetNext()->InvalidatePos();

                //Sad but true: during notify on ViewImp a Calc on the page and
                //its Lower may be called. The values should not be changed
                //because the caller takes care of the adjustment of Frame and
                //Prt.
                const long nOldFrameHeight = Frame().Height();
                const long nOldPrtHeight = Prt().Height();
                const bool bOldComplete = IsCompletePaint();
                if ( IsBodyFrame() )
                    Prt().SSize().Height() = nOldFrameHeight;

                if ( pUp->GetUpper() )
                    static_cast<SwRootFrame*>(pUp->GetUpper())->CheckViewLayout( nullptr, nullptr );
                //static_cast<SwPageFrame*>(pUp)->AdjustRootSize( CHG_CHGPAGE, &aOldRect );

                Frame().SSize().Height() = nOldFrameHeight;
                Prt().SSize().Height() = nOldPrtHeight;
                mbCompletePaint = bOldComplete;
            }
            if ( !IsBodyFrame() )
                pUp->_InvalidateSize();
            InvalidatePage( static_cast<SwPageFrame*>(pUp) );
        }
        nDiff -= nChg;
        if ( !nDiff )
            return nChg;
        else
            nBrowseAdd = nChg;
    }

    const SwFootnoteBossFrame *pBoss = static_cast<SwFootnoteBossFrame*>(GetUpper());

    SwTwips nReal = 0,
            nAdd  = 0;
    SwFrame *pFrame = nullptr;
    SWRECTFN( this )

    if( IsBodyFrame() )
    {
        if( IsInSct() )
        {
            SwSectionFrame *pSect = FindSctFrame();
            if( nDiff > 0 && pSect->IsEndnAtEnd() && GetNext() &&
                GetNext()->IsFootnoteContFrame() )
            {
                SwFootnoteContFrame* pCont = static_cast<SwFootnoteContFrame*>(GetNext());
                SwTwips nMinH = 0;
                SwFootnoteFrame* pFootnote = static_cast<SwFootnoteFrame*>(pCont->Lower());
                bool bFootnote = false;
                while( pFootnote )
                {
                    if( !pFootnote->GetAttr()->GetFootnote().IsEndNote() )
                    {
                        nMinH += (pFootnote->Frame().*fnRect->fnGetHeight)();
                        bFootnote = true;
                    }
                    pFootnote = static_cast<SwFootnoteFrame*>(pFootnote->GetNext());
                }
                if( bFootnote )
                    nMinH += (pCont->Prt().*fnRect->fnGetTop)();
                nReal = (pCont->Frame().*fnRect->fnGetHeight)() - nMinH;
                if( nReal > nDiff )
                    nReal = nDiff;
                if( nReal > 0 )
                    pFrame = GetNext();
                else
                    nReal = 0;
            }
            if( !bTst && !pSect->IsColLocked() )
                pSect->InvalidateSize();
        }
        if( !pFrame )
            return nBrowseAdd;
    }
    else
    {
        const bool bFootnotePage = pBoss->IsPageFrame() && static_cast<const SwPageFrame*>(pBoss)->IsFootnotePage();
        if ( bFootnotePage && !IsFootnoteContFrame() )
            pFrame = const_cast<SwFrame*>(static_cast<SwFrame const *>(pBoss->FindFootnoteCont()));
        if ( !pFrame )
            pFrame = const_cast<SwFrame*>(static_cast<SwFrame const *>(pBoss->FindBodyCont()));

        if ( !pFrame )
            return 0;

        //If not one is found, everything else is solved.
        nReal = (pFrame->Frame().*fnRect->fnGetHeight)();
        if( nReal > nDiff )
            nReal = nDiff;
        if( !bFootnotePage )
        {
            //Respect the minimal boundary!
            if( nReal )
            {
                const SwTwips nMax = pBoss->GetVarSpace();
                if ( nReal > nMax )
                    nReal = nMax;
            }
            if( !IsFootnoteContFrame() && nDiff > nReal &&
                pFrame->GetNext() && pFrame->GetNext()->IsFootnoteContFrame()
                && ( pFrame->GetNext()->IsVertical() == IsVertical() )
                )
            {
                //If the Body doesn't return enough, we look for a footnote, if
                //there is one, we steal there accordingly.
                const SwTwips nAddMax = (pFrame->GetNext()->Frame().*fnRect->
                                        fnGetHeight)();
                nAdd = nDiff - nReal;
                if ( nAdd > nAddMax )
                    nAdd = nAddMax;
                if ( !bTst )
                {
                    (pFrame->GetNext()->Frame().*fnRect->fnSetHeight)(nAddMax-nAdd);
                    if( bVert && !bVertL2R && !bRev )
                        pFrame->GetNext()->Frame().Pos().X() += nAdd;
                    pFrame->GetNext()->InvalidatePrt();
                    if ( pFrame->GetNext()->GetNext() )
                        pFrame->GetNext()->GetNext()->_InvalidatePos();
                }
            }
        }
    }

    if ( !bTst && nReal )
    {
        SwTwips nTmp = (pFrame->Frame().*fnRect->fnGetHeight)();
        (pFrame->Frame().*fnRect->fnSetHeight)( nTmp - nReal );
        if( bVert && !bVertL2R && !bRev )
            pFrame->Frame().Pos().X() += nReal;
        pFrame->InvalidatePrt();
        if ( pFrame->GetNext() )
            pFrame->GetNext()->_InvalidatePos();
        if( nReal < 0 && pFrame->IsInSct() )
        {
            SwLayoutFrame* pUp = pFrame->GetUpper();
            if( pUp && nullptr != ( pUp = pUp->GetUpper() ) && pUp->IsSctFrame() &&
                !pUp->IsColLocked() )
                pUp->InvalidateSize();
        }
        if( ( IsHeaderFrame() || IsFooterFrame() ) && pBoss->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pBoss->GetDrawObjs();
            OSL_ENSURE( pBoss->IsPageFrame(), "Header/Footer out of page?" );
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
                {
                    SwFlyFrame* pFly = static_cast<SwFlyFrame*>(pAnchoredObj);
                    OSL_ENSURE( !pFly->IsFlyInContentFrame(), "FlyInCnt at Page?" );
                    const SwFormatVertOrient &rVert =
                                        pFly->GetFormat()->GetVertOrient();
                    // When do we have to invalidate?
                    // If a frame is aligned on a PageTextArea and the header
                    // changes a TOP, MIDDLE or NONE aligned frame needs to
                    // recalculate it's position; if the footer changes a BOTTOM
                    // or MIDDLE aligned frame needs to recalculate it's
                    // position.
                    if( ( rVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA ||
                          rVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA )    &&
                        ((IsHeaderFrame() && rVert.GetVertOrient()!=text::VertOrientation::BOTTOM) ||
                         (IsFooterFrame() && rVert.GetVertOrient()!=text::VertOrientation::NONE &&
                          rVert.GetVertOrient() != text::VertOrientation::TOP)) )
                    {
                        pFly->_InvalidatePos();
                        pFly->_Invalidate();
                    }
                }
            }
        }
    }
    return (nBrowseAdd + nReal + nAdd);
}

/** method to perform additional actions on an invalidation (2004-05-19 #i28701#) */
void SwFrame::_ActionOnInvalidation( const InvalidationType )
{
    // default behaviour is to perform no additional action
}

/** method to determine, if an invalidation is allowed (2004-05-19 #i28701#) */
bool SwFrame::_InvalidationAllowed( const InvalidationType ) const
{
    // default behaviour is to allow invalidation
    return true;
}

void SwFrame::ImplInvalidateSize()
{
    if ( _InvalidationAllowed( INVALID_SIZE ) )
    {
        mbValidSize = false;
        if ( IsFlyFrame() )
            static_cast<SwFlyFrame*>(this)->_Invalidate();
        else
            InvalidatePage();

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_SIZE );
    }
}

void SwFrame::ImplInvalidatePrt()
{
    if ( _InvalidationAllowed( INVALID_PRTAREA ) )
    {
        mbValidPrtArea = false;
        if ( IsFlyFrame() )
            static_cast<SwFlyFrame*>(this)->_Invalidate();
        else
            InvalidatePage();

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_PRTAREA );
    }
}

void SwFrame::ImplInvalidatePos()
{
    if ( _InvalidationAllowed( INVALID_POS ) )
    {
        mbValidPos = false;
        if ( IsFlyFrame() )
        {
            static_cast<SwFlyFrame*>(this)->_Invalidate();
        }
        else
        {
            InvalidatePage();
        }

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_POS );
    }
}

void SwFrame::ImplInvalidateLineNum()
{
    if ( _InvalidationAllowed( INVALID_LINENUM ) )
    {
        mbValidLineNum = false;
        OSL_ENSURE( IsTextFrame(), "line numbers are implemented for text only" );
        InvalidatePage();

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_LINENUM );
    }
}

void SwFrame::ReinitializeFrameSizeAttrFlags()
{
    const SwFormatFrameSize &rFormatSize = GetAttrSet()->GetFrameSize();
    if ( ATT_VAR_SIZE == rFormatSize.GetHeightSizeType() ||
         ATT_MIN_SIZE == rFormatSize.GetHeightSizeType())
    {
        mbFixSize = false;
        if ( GetType() & (FRM_HEADER | FRM_FOOTER | FRM_ROW) )
        {
            SwFrame *pFrame = static_cast<SwLayoutFrame*>(this)->Lower();
            while ( pFrame )
            {   pFrame->_InvalidateSize();
                pFrame->_InvalidatePrt();
                pFrame = pFrame->GetNext();
            }
            SwContentFrame *pCnt = static_cast<SwLayoutFrame*>(this)->ContainsContent();
            // #i36991# - be save.
            // E.g., a row can contain *no* content.
            if ( pCnt )
            {
                pCnt->InvalidatePage();
                do
                {
                    pCnt->Prepare( PREP_ADJUST_FRM );
                    pCnt->_InvalidateSize();
                    pCnt = pCnt->GetNextContentFrame();
                } while ( static_cast<SwLayoutFrame*>(this)->IsAnLower( pCnt ) );
            }
        }
    }
    else if ( rFormatSize.GetHeightSizeType() == ATT_FIX_SIZE )
    {
        if( IsVertical() )
            ChgSize( Size( rFormatSize.GetWidth(), Frame().Height()));
        else
            ChgSize( Size( Frame().Width(), rFormatSize.GetHeight()));
    }
}

void SwFrame::ValidateThisAndAllLowers( const sal_uInt16 nStage )
{
    // Stage 0: Only validate frames. Do not process any objects.
    // Stage 1: Only validate fly frames and all of their contents.
    // Stage 2: Validate all.

    const bool bOnlyObject = 1 == nStage;
    const bool bIncludeObjects = 1 <= nStage;

    if ( !bOnlyObject || dynamic_cast< const SwFlyFrame *>( this ) !=  nullptr )
    {
        mbValidSize = true;
        mbValidPrtArea = true;
        mbValidPos = true;
    }

    if ( bIncludeObjects )
    {
        const SwSortedObjs* pObjs = GetDrawObjs();
        if ( pObjs )
        {
            const size_t nCnt = pObjs->size();
            for ( size_t i = 0; i < nCnt; ++i )
            {
                SwAnchoredObject* pAnchObj = (*pObjs)[i];
                if ( dynamic_cast< const SwFlyFrame *>( pAnchObj ) !=  nullptr )
                    static_cast<SwFlyFrame*>(pAnchObj)->ValidateThisAndAllLowers( 2 );
                else if ( dynamic_cast< const SwAnchoredDrawObject *>( pAnchObj ) !=  nullptr )
                    static_cast<SwAnchoredDrawObject*>(pAnchObj)->ValidateThis();
            }
        }
    }

    if ( IsLayoutFrame() )
    {
        SwFrame* pLower = static_cast<SwLayoutFrame*>(this)->Lower();
        while ( pLower )
        {
            pLower->ValidateThisAndAllLowers( nStage );
            pLower = pLower->GetNext();
        }
    }
}

SwTwips SwContentFrame::GrowFrame( SwTwips nDist, bool bTst, bool bInfo )
{
    SWRECTFN( this )

    SwTwips nFrameHeight = (Frame().*fnRect->fnGetHeight)();
    if( nFrameHeight > 0 &&
         nDist > (LONG_MAX - nFrameHeight ) )
        nDist = LONG_MAX - nFrameHeight;

    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();
    const sal_uInt16 nTmpType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse with Body
    if( !(GetUpper()->GetType() & nTmpType) && GetUpper()->HasFixSize() )
    {
        if ( !bTst )
        {
            (Frame().*fnRect->fnSetHeight)( nFrameHeight + nDist );
            if( IsVertical() && !IsVertLR() && !IsReverse() )
                Frame().Pos().X() -= nDist;
            if ( GetNext() )
            {
                GetNext()->InvalidatePos();
            }
            // #i28701# - Due to the new object positioning the
            // frame on the next page/column can flow backward (e.g. it was moved forward
            // due to the positioning of its objects ). Thus, invalivate this next frame,
            // if document compatibility option 'Consider wrapping style influence on
            // object positioning' is ON.
            else if ( GetUpper()->GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) )
            {
                InvalidateNextPos();
            }
        }
        return 0;
    }

    SwTwips nReal = (GetUpper()->Prt().*fnRect->fnGetHeight)();
    SwFrame *pFrame = GetUpper()->Lower();
    while( pFrame && nReal > 0 )
    {   nReal -= (pFrame->Frame().*fnRect->fnGetHeight)();
        pFrame = pFrame->GetNext();
    }

    if ( !bTst )
    {
        //Contents are always resized to the wished value.
        long nOld = (Frame().*fnRect->fnGetHeight)();
        (Frame().*fnRect->fnSetHeight)( nOld + nDist );
        if( IsVertical()&& !IsVertLR() && !IsReverse() )
            Frame().Pos().X() -= nDist;
        SwTabFrame *pTab = (nOld && IsInTab()) ? FindTabFrame() : nullptr;
        if (pTab)
        {
            if ( pTab->GetTable()->GetHTMLTableLayout() &&
                 !pTab->IsJoinLocked() &&
                 !pTab->GetFormat()->GetDoc()->GetDocShell()->IsReadOnly() )
            {
                pTab->InvalidatePos();
                pTab->SetResizeHTMLTable();
            }
        }
    }

    //Only grow Upper if necessary.
    if ( nReal < nDist )
    {
        if( GetUpper() )
        {
            if( bTst || !GetUpper()->IsFooterFrame() )
                nReal = GetUpper()->Grow( nDist - (nReal > 0 ? nReal : 0),
                                          bTst, bInfo );
            else
            {
                nReal = 0;
                GetUpper()->InvalidateSize();
            }
        }
        else
            nReal = 0;
    }
    else
        nReal = nDist;

    // #i28701# - Due to the new object positioning the
    // frame on the next page/column can flow backward (e.g. it was moved forward
    // due to the positioning of its objects ). Thus, invalivate this next frame,
    // if document compatibility option 'Consider wrapping style influence on
    // object positioning' is ON.
    if ( !bTst )
    {
        if ( GetNext() )
        {
            GetNext()->InvalidatePos();
        }
        else if ( GetUpper()->GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) )
        {
            InvalidateNextPos();
        }
    }

    return nReal;
}

SwTwips SwContentFrame::ShrinkFrame( SwTwips nDist, bool bTst, bool bInfo )
{
    SWRECTFN( this )
    OSL_ENSURE( nDist >= 0, "nDist < 0" );
    OSL_ENSURE( nDist <= (Frame().*fnRect->fnGetHeight)(),
            "nDist > than current size." );

    if ( !bTst )
    {
        SwTwips nRstHeight;
        if( GetUpper() )
            nRstHeight = (Frame().*fnRect->fnBottomDist)
                         ( (GetUpper()->*fnRect->fnGetPrtBottom)() );
        else
            nRstHeight = 0;
        if( nRstHeight < 0 )
        {
            SwTwips nNextHeight = 0;
            if( GetUpper()->IsSctFrame() && nDist > LONG_MAX/2 )
            {
                SwFrame *pNxt = GetNext();
                while( pNxt )
                {
                    nNextHeight += (pNxt->Frame().*fnRect->fnGetHeight)();
                    pNxt = pNxt->GetNext();
                }
            }
            nRstHeight = nDist + nRstHeight - nNextHeight;
        }
        else
            nRstHeight = nDist;
        (Frame().*fnRect->fnSetHeight)( (Frame().*fnRect->fnGetHeight)() - nDist );
        if( IsVertical() && !IsVertLR() )
            Frame().Pos().X() += nDist;
        nDist = nRstHeight;
        SwTabFrame *pTab = IsInTab() ? FindTabFrame() : nullptr;
        if (pTab)
        {
            if ( pTab->GetTable()->GetHTMLTableLayout() &&
                 !pTab->IsJoinLocked() &&
                 !pTab->GetFormat()->GetDoc()->GetDocShell()->IsReadOnly() )
            {
                pTab->InvalidatePos();
                pTab->SetResizeHTMLTable();
            }
        }
    }

    SwTwips nReal;
    if( GetUpper() && nDist > 0 )
    {
        if( bTst || !GetUpper()->IsFooterFrame() )
            nReal = GetUpper()->Shrink( nDist, bTst, bInfo );
        else
        {
            nReal = 0;

            // #108745# Sorry, dear old footer friend, I'm not gonna invalidate you,
            // if there are any objects anchored inside your content, which
            // overlap with the shrinking frame.
            // This may lead to a footer frame that is too big, but this is better
            // than looping.
            // #109722# : The fix for #108745# was too strict.

            bool bInvalidate = true;
            const SwRect aRect( Frame() );
            const SwPageFrame* pPage = FindPageFrame();
            const SwSortedObjs* pSorted = pPage ? pPage->GetSortedObjs() : nullptr;
            if( pSorted )
            {
                for ( size_t i = 0; i < pSorted->size(); ++i )
                {
                    const SwAnchoredObject* pAnchoredObj = (*pSorted)[i];
                    const SwRect aBound( pAnchoredObj->GetObjRectWithSpaces() );

                    if( aBound.Left() > aRect.Right() )
                        continue;

                    if( aBound.IsOver( aRect ) )
                    {
                        const SwFrameFormat& rFormat = pAnchoredObj->GetFrameFormat();
                        if( SURROUND_THROUGHT != rFormat.GetSurround().GetSurround() )
                        {
                            const SwFrame* pAnchor = pAnchoredObj->GetAnchorFrame();
                            if ( pAnchor && pAnchor->FindFooterOrHeader() == GetUpper() )
                            {
                                bInvalidate = false;
                                break;
                            }
                        }
                    }
                }
            }

            if ( bInvalidate )
                GetUpper()->InvalidateSize();
        }
    }
    else
        nReal = 0;

    if ( !bTst )
    {
        //The position of the next Frame changes for sure.
        InvalidateNextPos();

        //If I don't have a successor I have to do the retouch by myself.
        if ( !GetNext() )
            SetRetouche();
    }
    return nReal;
}

void SwContentFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_uInt8 nInvFlags = 0;

    if( pNew && RES_ATTRSET_CHG == pNew->Which() && pOld )
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
            SwFrame::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        SwPageFrame *pPage = FindPageFrame();
        InvalidatePage( pPage );
        if ( nInvFlags & 0x01 )
            SetCompletePaint();
        if ( nInvFlags & 0x02 )
            _InvalidatePos();
        if ( nInvFlags & 0x04 )
            _InvalidateSize();
        if ( nInvFlags & 0x88 )
        {
            if( IsInSct() && !GetPrev() )
            {
                SwSectionFrame *pSect = FindSctFrame();
                if( pSect->ContainsAny() == this )
                {
                    pSect->_InvalidatePrt();
                    pSect->InvalidatePage( pPage );
                }
            }
            _InvalidatePrt();
        }
        SwFrame* mpNextFrame = GetIndNext();
        if ( mpNextFrame && nInvFlags & 0x10)
        {
            mpNextFrame->_InvalidatePrt();
            mpNextFrame->InvalidatePage( pPage );
        }
        if ( mpNextFrame && nInvFlags & 0x80 )
        {
            mpNextFrame->SetCompletePaint();
        }
        if ( nInvFlags & 0x20 )
        {
            SwFrame* pPrevFrame = GetPrev();
            if ( pPrevFrame )
            {
                pPrevFrame->_InvalidatePrt();
                pPrevFrame->InvalidatePage( pPage );
            }
        }
        if ( nInvFlags & 0x40 )
            InvalidateNextPos();
    }
}

void SwContentFrame::_UpdateAttr( const SfxPoolItem* pOld, const SfxPoolItem* pNew,
                              sal_uInt8 &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch ( nWhich )
    {
        case RES_FMT_CHG:
            rInvFlags = 0xFF;
            SAL_FALLTHROUGH;

        case RES_PAGEDESC:                      //attribute changes (on/off)
            if ( IsInDocBody() && !IsInTab() )
            {
                rInvFlags |= 0x02;
                SwPageFrame *pPage = FindPageFrame();
                if ( !GetPrev() )
                    CheckPageDescs( pPage );
                if ( GetAttrSet()->GetPageDesc().GetNumOffset() )
                    static_cast<SwRootFrame*>(pPage->GetUpper())->SetVirtPageNum( true );
                SwDocPosUpdate aMsgHint( pPage->Frame().Top() );
                pPage->GetFormat()->GetDoc()->getIDocumentFieldsAccess().UpdatePageFields( &aMsgHint );
            }
            break;

        case RES_UL_SPACE:
            {
                // OD 2004-02-18 #106629# - correction
                // Invalidation of the printing area of next frame, not only
                // for footnote content.
                if ( !GetIndNext() )
                {
                    SwFrame* pNxt = FindNext();
                    if ( pNxt )
                    {
                        SwPageFrame* pPg = pNxt->FindPageFrame();
                        pNxt->InvalidatePage( pPg );
                        pNxt->_InvalidatePrt();
                        if( pNxt->IsSctFrame() )
                        {
                            SwFrame* pCnt = static_cast<SwSectionFrame*>(pNxt)->ContainsAny();
                            if( pCnt )
                            {
                                pCnt->_InvalidatePrt();
                                pCnt->InvalidatePage( pPg );
                            }
                        }
                        pNxt->SetCompletePaint();
                    }
                }
                // OD 2004-03-17 #i11860#
                if ( GetIndNext() &&
                     !GetUpper()->GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::USE_FORMER_OBJECT_POS) )
                {
                    // OD 2004-07-01 #i28701# - use new method <InvalidateObjs(..)>
                    GetIndNext()->InvalidateObjs( true );
                }
                Prepare( PREP_UL_SPACE );   //TextFrame has to correct line spacing.
                rInvFlags |= 0x80;
                SAL_FALLTHROUGH;
            }
        case RES_LR_SPACE:
        case RES_BOX:
        case RES_SHADOW:
            Prepare( PREP_FIXSIZE_CHG );
            SwFrame::Modify( pOld, pNew );
            rInvFlags |= 0x30;
            break;

        case RES_BREAK:
            {
                rInvFlags |= 0x42;
                const IDocumentSettingAccess& rIDSA = GetUpper()->GetFormat()->getIDocumentSettingAccess();
                if( rIDSA.get(DocumentSettingId::PARA_SPACE_MAX) ||
                    rIDSA.get(DocumentSettingId::PARA_SPACE_MAX_AT_PAGES) )
                {
                    rInvFlags |= 0x1;
                    SwFrame* pNxt = FindNext();
                    if( pNxt )
                    {
                        SwPageFrame* pPg = pNxt->FindPageFrame();
                        pNxt->InvalidatePage( pPg );
                        pNxt->_InvalidatePrt();
                        if( pNxt->IsSctFrame() )
                        {
                            SwFrame* pCnt = static_cast<SwSectionFrame*>(pNxt)->ContainsAny();
                            if( pCnt )
                            {
                                pCnt->_InvalidatePrt();
                                pCnt->InvalidatePage( pPg );
                            }
                        }
                        pNxt->SetCompletePaint();
                    }
                }
            }
            break;

        // OD 2004-02-26 #i25029#
        case RES_PARATR_CONNECT_BORDER:
        {
            rInvFlags |= 0x01;
            if ( IsTextFrame() )
            {
                InvalidateNextPrtArea();
            }
            if ( !GetIndNext() && IsInTab() && IsInSplitTableRow() )
            {
                FindTabFrame()->InvalidateSize();
            }
        }
        break;

        case RES_PARATR_TABSTOP:
        case RES_CHRATR_PROPORTIONALFONTSIZE:
        case RES_CHRATR_SHADOWED:
        case RES_CHRATR_AUTOKERN:
        case RES_CHRATR_UNDERLINE:
        case RES_CHRATR_OVERLINE:
        case RES_CHRATR_KERNING:
        case RES_CHRATR_FONT:
        case RES_CHRATR_FONTSIZE:
        case RES_CHRATR_ESCAPEMENT:
        case RES_CHRATR_CONTOUR:
        case RES_PARATR_NUMRULE:
            rInvFlags |= 0x01;
            break;

        case RES_FRM_SIZE:
            rInvFlags |= 0x01;
            SAL_FALLTHROUGH;

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
            SwFrame::Modify( pOld, pNew );
    }
}

SwLayoutFrame::SwLayoutFrame(SwFrameFormat *const pFormat, SwFrame *const pSib)
    : SwFrame(pFormat, pSib)
    , m_pLower(nullptr)
{
    const SwFormatFrameSize &rFormatSize = pFormat->GetFrameSize();
    if ( rFormatSize.GetHeightSizeType() == ATT_FIX_SIZE )
        mbFixSize = true;
}

// #i28701#

SwTwips SwLayoutFrame::InnerHeight() const
{
    const SwFrame* pCnt = Lower();
    if (!pCnt)
        return 0;

    SWRECTFN( this )
    SwTwips nRet = 0;
    if( pCnt->IsColumnFrame() || pCnt->IsCellFrame() )
    {
        do
        {
            SwTwips nTmp = static_cast<const SwLayoutFrame*>(pCnt)->InnerHeight();
            if( pCnt->GetValidPrtAreaFlag() )
                nTmp += (pCnt->Frame().*fnRect->fnGetHeight)() -
                        (pCnt->Prt().*fnRect->fnGetHeight)();
            if( nRet < nTmp )
                nRet = nTmp;
            pCnt = pCnt->GetNext();
        } while ( pCnt );
    }
    else
    {
        do
        {
            nRet += (pCnt->Frame().*fnRect->fnGetHeight)();
            if( pCnt->IsContentFrame() && static_cast<const SwTextFrame*>(pCnt)->IsUndersized() )
                nRet += static_cast<const SwTextFrame*>(pCnt)->GetParHeight() -
                        (pCnt->Prt().*fnRect->fnGetHeight)();
            if( pCnt->IsLayoutFrame() && !pCnt->IsTabFrame() )
                nRet += static_cast<const SwLayoutFrame*>(pCnt)->InnerHeight() -
                        (pCnt->Prt().*fnRect->fnGetHeight)();
            pCnt = pCnt->GetNext();
        } while( pCnt );

    }
    return nRet;
}

SwTwips SwLayoutFrame::GrowFrame( SwTwips nDist, bool bTst, bool bInfo )
{
    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();
    const sal_uInt16 nTmpType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse with Body
    if( !(GetType() & nTmpType) && HasFixSize() )
        return 0;

    SWRECTFN( this )
    const SwTwips nFrameHeight = (Frame().*fnRect->fnGetHeight)();
    const SwTwips nFramePos = Frame().Pos().X();

    if ( nFrameHeight > 0 && nDist > (LONG_MAX - nFrameHeight) )
        nDist = LONG_MAX - nFrameHeight;

    SwTwips nMin = 0;
    if ( GetUpper() && !IsCellFrame() )
    {
        SwFrame *pFrame = GetUpper()->Lower();
        while( pFrame )
        {   nMin += (pFrame->Frame().*fnRect->fnGetHeight)();
            pFrame = pFrame->GetNext();
        }
        nMin = (GetUpper()->Prt().*fnRect->fnGetHeight)() - nMin;
        if ( nMin < 0 )
            nMin = 0;
    }

    SwRect aOldFrame( Frame() );
    bool bMoveAccFrame = false;

    bool bChgPos = IsVertical() && !IsReverse();
    if ( !bTst )
    {
        (Frame().*fnRect->fnSetHeight)( nFrameHeight + nDist );
        if( bChgPos && !IsVertLR() )
            Frame().Pos().X() -= nDist;
        bMoveAccFrame = true;
    }

    SwTwips nReal = nDist - nMin;
    if ( nReal > 0 )
    {
        if ( GetUpper() )
        {   // AdjustNeighbourhood now only for the columns (but not in frames)
            sal_uInt8 nAdjust = GetUpper()->IsFootnoteBossFrame() ?
                static_cast<SwFootnoteBossFrame*>(GetUpper())->NeighbourhoodAdjustment( this )
                : NA_GROW_SHRINK;
            if( NA_ONLY_ADJUST == nAdjust )
                nReal = AdjustNeighbourhood( nReal, bTst );
            else
            {
                if( NA_ADJUST_GROW == nAdjust )
                    nReal += AdjustNeighbourhood( nReal, bTst );

                SwTwips nGrow = 0;
                if( 0 < nReal )
                {
                    SwFrame* pToGrow = GetUpper();
                    // NEW TABLES
                    // A cell with a row span of > 1 is allowed to grow the
                    // line containing the end of the row span if it is
                    // located in the same table frame:
                    const SwCellFrame* pThisCell = dynamic_cast<const SwCellFrame*>(this);
                    if ( pThisCell && pThisCell->GetLayoutRowSpan() > 1 )
                    {
                        SwCellFrame& rEndCell = const_cast<SwCellFrame&>(pThisCell->FindStartEndOfRowSpanCell( false, true ));
                        if ( -1 == rEndCell.GetTabBox()->getRowSpan() )
                            pToGrow = rEndCell.GetUpper();
                        else
                            pToGrow = nullptr;
                    }

                    nGrow = pToGrow ? pToGrow->Grow( nReal, bTst, bInfo ) : 0;
                }

                if( NA_GROW_ADJUST == nAdjust && nGrow < nReal )
                    nReal += AdjustNeighbourhood( nReal - nGrow, bTst );

                if ( IsFootnoteFrame() && (nGrow != nReal) && GetNext() )
                {
                    //Footnotes can replace their successor.
                    SwTwips nSpace = bTst ? 0 : -nDist;
                    const SwFrame *pFrame = GetUpper()->Lower();
                    do
                    {   nSpace += (pFrame->Frame().*fnRect->fnGetHeight)();
                        pFrame = pFrame->GetNext();
                    } while ( pFrame != GetNext() );
                    nSpace = (GetUpper()->Prt().*fnRect->fnGetHeight)() -nSpace;
                    if ( nSpace < 0 )
                        nSpace = 0;
                    nSpace += nGrow;
                    if ( nReal > nSpace )
                        nReal = nSpace;
                    if ( nReal && !bTst )
                        static_cast<SwFootnoteFrame*>(this)->InvalidateNxtFootnoteCnts( FindPageFrame() );
                }
                else
                    nReal = nGrow;
            }
        }
        else
            nReal = 0;

        nReal += nMin;
    }
    else
        nReal = nDist;

    if ( !bTst )
    {
        if( nReal != nDist &&
            // NEW TABLES
            ( !IsCellFrame() || static_cast<SwCellFrame*>(this)->GetLayoutRowSpan() > 1 ) )
        {
            (Frame().*fnRect->fnSetHeight)( nFrameHeight + nReal );
            if( bChgPos && !IsVertLR() )
                Frame().Pos().X() = nFramePos - nReal;
            bMoveAccFrame = true;
        }

        if ( nReal )
        {
            SwPageFrame *pPage = FindPageFrame();
            if ( GetNext() )
            {
                GetNext()->_InvalidatePos();
                if ( GetNext()->IsContentFrame() )
                    GetNext()->InvalidatePage( pPage );
            }
            if ( !IsPageBodyFrame() )
            {
                _InvalidateAll();
                InvalidatePage( pPage );
            }
            if (!(GetType() & (FRM_ROW|FRM_TAB|FRM_FTNCONT|FRM_PAGE|FRM_ROOT)))
                NotifyLowerObjs();

            if( IsCellFrame() )
                InvaPercentLowers( nReal );

            SvxBrushItem aBack(GetFormat()->makeBackgroundBrushItem());
            const SvxGraphicPosition ePos = aBack.GetGraphicPos();
            if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
                SetCompletePaint();
        }
    }

    if( bMoveAccFrame && IsAccessibleFrame() )
    {
        SwRootFrame *pRootFrame = getRootFrame();
        if( pRootFrame && pRootFrame->IsAnyShellAccessible() &&
            pRootFrame->GetCurrShell() )
        {
            pRootFrame->GetCurrShell()->Imp()->MoveAccessibleFrame( this, aOldFrame );
        }
    }
    return nReal;
}

SwTwips SwLayoutFrame::ShrinkFrame( SwTwips nDist, bool bTst, bool bInfo )
{
    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();
    const sal_uInt16 nTmpType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse by Body.

    if (pSh && pSh->GetViewOptions()->IsWhitespaceHidden())
    {
        if (IsBodyFrame())
        {
            // Whitespace is hidden and this body frame will not shrink, as it
            // has a fix size.
            // Invalidate the page frame size, so in case the reason for the
            // shrink was that there is more whitespace on this page, the size
            // without whitespace will be recalculated correctly.
            SwPageFrame* pPageFrame = FindPageFrame();
            pPageFrame->InvalidateSize();
        }
    }

    if( !(GetType() & nTmpType) && HasFixSize() )
        return 0;

    OSL_ENSURE( nDist >= 0, "nDist < 0" );
    SWRECTFN( this )
    SwTwips nFrameHeight = (Frame().*fnRect->fnGetHeight)();
    if ( nDist > nFrameHeight )
        nDist = nFrameHeight;

    SwTwips nMin = 0;
    bool bChgPos = IsVertical() && !IsReverse();
    if ( Lower() )
    {
        if( !Lower()->IsNeighbourFrame() )
        {   const SwFrame *pFrame = Lower();
            const long nTmp = (Prt().*fnRect->fnGetHeight)();
            while( pFrame && nMin < nTmp )
            {   nMin += (pFrame->Frame().*fnRect->fnGetHeight)();
                pFrame = pFrame->GetNext();
            }
        }
    }
    SwTwips nReal = nDist;
    SwTwips nMinDiff = (Prt().*fnRect->fnGetHeight)() - nMin;
    if( nReal > nMinDiff )
        nReal = nMinDiff;
    if( nReal <= 0 )
        return nDist;

    SwRect aOldFrame( Frame() );
    bool bMoveAccFrame = false;

    SwTwips nRealDist = nReal;
    if ( !bTst )
    {
        (Frame().*fnRect->fnSetHeight)( nFrameHeight - nReal );
        if( bChgPos && !IsVertLR() )
            Frame().Pos().X() += nReal;
        bMoveAccFrame = true;
    }

    sal_uInt8 nAdjust = GetUpper() && GetUpper()->IsFootnoteBossFrame() ?
                   static_cast<SwFootnoteBossFrame*>(GetUpper())->NeighbourhoodAdjustment( this )
                   : NA_GROW_SHRINK;

    // AdjustNeighbourhood also in columns (but not in frames)
    if( NA_ONLY_ADJUST == nAdjust )
    {
        if ( IsPageBodyFrame() && !bBrowse )
            nReal = nDist;
        else
        {   nReal = AdjustNeighbourhood( -nReal, bTst );
            nReal *= -1;
            if ( !bTst && IsBodyFrame() && nReal < nRealDist )
            {
                (Frame().*fnRect->fnSetHeight)( (Frame().*fnRect->fnGetHeight)()
                                            + nRealDist - nReal );
                if( bChgPos && !IsVertLR() )
                    Frame().Pos().X() += nRealDist - nReal;
                OSL_ENSURE( !IsAccessibleFrame(), "bMoveAccFrame has to be set!" );
            }
        }
    }
    else if( IsColumnFrame() || IsColBodyFrame() )
    {
        SwTwips nTmp = GetUpper()->Shrink( nReal, bTst, bInfo );
        if ( nTmp != nReal )
        {
            (Frame().*fnRect->fnSetHeight)( (Frame().*fnRect->fnGetHeight)()
                                          + nReal - nTmp );
            if( bChgPos && !IsVertLR() )
                Frame().Pos().X() += nTmp - nReal;
            OSL_ENSURE( !IsAccessibleFrame(), "bMoveAccFrame has to be set!" );
            nReal = nTmp;
        }
    }
    else
    {
        SwTwips nShrink = nReal;
        SwFrame* pToShrink = GetUpper();
        const SwCellFrame* pThisCell = dynamic_cast<const SwCellFrame*>(this);
        // NEW TABLES
        if ( pThisCell && pThisCell->GetLayoutRowSpan() > 1 )
        {
            SwCellFrame& rEndCell = const_cast<SwCellFrame&>(pThisCell->FindStartEndOfRowSpanCell( false, true ));
            pToShrink = rEndCell.GetUpper();
        }

        nReal = pToShrink ? pToShrink->Shrink( nShrink, bTst, bInfo ) : 0;
        if( ( NA_GROW_ADJUST == nAdjust || NA_ADJUST_GROW == nAdjust )
            && nReal < nShrink )
            AdjustNeighbourhood( nReal - nShrink );
    }

    if( bMoveAccFrame && IsAccessibleFrame() )
    {
        SwRootFrame *pRootFrame = getRootFrame();
        if( pRootFrame && pRootFrame->IsAnyShellAccessible() &&
            pRootFrame->GetCurrShell() )
        {
            pRootFrame->GetCurrShell()->Imp()->MoveAccessibleFrame( this, aOldFrame );
        }
    }
    if ( !bTst && (IsCellFrame() || IsColumnFrame() ? nReal : nRealDist) )
    {
        SwPageFrame *pPage = FindPageFrame();
        if ( GetNext() )
        {
            GetNext()->_InvalidatePos();
            if ( GetNext()->IsContentFrame() )
                GetNext()->InvalidatePage( pPage );
            if ( IsTabFrame() )
                static_cast<SwTabFrame*>(this)->SetComplete();
        }
        else
        {   if ( IsRetoucheFrame() )
                SetRetouche();
            if ( IsTabFrame() )
            {
                if( IsTabFrame() )
                    static_cast<SwTabFrame*>(this)->SetComplete();
                if ( Lower() )  // Can also be in the Join and be empty!
                    InvalidateNextPos();
            }
        }
        if ( !IsBodyFrame() )
        {
            _InvalidateAll();
            InvalidatePage( pPage );
            bool bCompletePaint = true;
            const SwFrameFormat* pFormat = GetFormat();
            if (pFormat)
            {
                SvxBrushItem aBack(pFormat->makeBackgroundBrushItem());
                const SvxGraphicPosition ePos = aBack.GetGraphicPos();
                if ( GPOS_NONE == ePos || GPOS_TILED == ePos )
                    bCompletePaint = false;
            }
            if (bCompletePaint)
                SetCompletePaint();
        }

        if (!(GetType() & (FRM_ROW|FRM_TAB|FRM_FTNCONT|FRM_PAGE|FRM_ROOT)))
            NotifyLowerObjs();

        if( IsCellFrame() )
            InvaPercentLowers( nReal );

        SwContentFrame *pCnt;
        if( IsFootnoteFrame() && !static_cast<SwFootnoteFrame*>(this)->GetAttr()->GetFootnote().IsEndNote() &&
            ( GetFormat()->GetDoc()->GetFootnoteInfo().ePos != FTNPOS_CHAPTER ||
              ( IsInSct() && FindSctFrame()->IsFootnoteAtEnd() ) ) &&
              nullptr != (pCnt = static_cast<SwFootnoteFrame*>(this)->GetRefFromAttr() ) )
        {
            if ( pCnt->IsFollow() )
            {   // If we are in an other column/page than the frame with the
                // reference, we don't need to invalidate its master.
                SwFrame *pTmp = pCnt->FindFootnoteBossFrame(true) == FindFootnoteBossFrame(true)
                              ?  &pCnt->FindMaster()->GetFrame() : pCnt;
                pTmp->Prepare( PREP_ADJUST_FRM );
                pTmp->InvalidateSize();
            }
            else
                pCnt->InvalidatePos();
        }
    }
    return nReal;
}

/**
 * Changes the size of the directly subsidiary Frame's that have a fixed size, proportionally to the
 * size change of the PrtArea of the Frame's.
 *
 * The variable Frames are also proportionally adapted; they will grow/shrink again by themselves.
 */
void SwLayoutFrame::ChgLowersProp( const Size& rOldSize )
{
    // no change of lower properties for root frame or if no lower exists.
    if ( IsRootFrame() || !Lower() )
        return;

    // declare and init <SwFrame* pLowerFrame> with first lower
    SwFrame *pLowerFrame = Lower();

    // declare and init const booleans <bHeightChgd> and <bWidthChg>
    const bool bHeightChgd = rOldSize.Height() != Prt().Height();
    const bool bWidthChgd  = rOldSize.Width()  != Prt().Width();

    // declare and init variables <bVert>, <bRev> and <fnRect>
    SWRECTFN( this )

    // This shortcut basically tries to handle only lower frames that
    // are affected by the size change. Otherwise much more lower frames
    // are invalidated.
    if ( !( bVert ? bHeightChgd : bWidthChgd ) &&
         ! Lower()->IsColumnFrame() &&
           ( ( IsBodyFrame() && IsInDocBody() && ( !IsInSct() || !FindSctFrame()->IsColLocked() ) ) ||
                // #i10826# Section frames without columns should not
                // invalidate all lowers!
               IsSctFrame() ) )
    {
        // Determine page frame the body frame resp. the section frame belongs to.
        SwPageFrame *pPage = FindPageFrame();
        // Determine last lower by traveling through them using <GetNext()>.
        // During travel check each section frame, if it will be sized to
        // maximum. If Yes, invalidate size of section frame and set
        // corresponding flags at the page.
        do
        {
            if( pLowerFrame->IsSctFrame() && static_cast<SwSectionFrame*>(pLowerFrame)->_ToMaximize() )
            {
                pLowerFrame->_InvalidateSize();
                pLowerFrame->InvalidatePage( pPage );
            }
            if( pLowerFrame->GetNext() )
                pLowerFrame = pLowerFrame->GetNext();
            else
                break;
        } while( true );
        // If found last lower is a section frame containing no section
        // (section frame isn't valid and will be deleted in the future),
        // travel backwards.
        while( pLowerFrame->IsSctFrame() && !static_cast<SwSectionFrame*>(pLowerFrame)->GetSection() &&
               pLowerFrame->GetPrev() )
            pLowerFrame = pLowerFrame->GetPrev();
        // If found last lower is a section frame, set <pLowerFrame> to its last
        // content, if the section frame is valid and is not sized to maximum.
        // Otherwise set <pLowerFrame> to NULL - In this case body frame only
        //      contains invalid section frames.
        if( pLowerFrame->IsSctFrame() )
            pLowerFrame = static_cast<SwSectionFrame*>(pLowerFrame)->GetSection() &&
                   !static_cast<SwSectionFrame*>(pLowerFrame)->ToMaximize( false ) ?
                   static_cast<SwSectionFrame*>(pLowerFrame)->FindLastContent() : nullptr;

        // continue with found last lower, probably the last content of a section
        if ( pLowerFrame )
        {
            // If <pLowerFrame> is in a table frame, set <pLowerFrame> to this table
            // frame and continue.
            if ( pLowerFrame->IsInTab() )
            {
                // OD 28.10.2002 #97265# - safeguard for setting <pLowerFrame> to
                // its table frame - check, if the table frame is also a lower
                // of the body frame, in order to assure that <pLowerFrame> is not
                // set to a frame, which is an *upper* of the body frame.
                SwFrame* pTableFrame = pLowerFrame->FindTabFrame();
                if ( IsAnLower( pTableFrame ) )
                {
                    pLowerFrame = pTableFrame;
                }
            }
            // Check, if variable size of body frame resp. section frame has grown
            // OD 28.10.2002 #97265# - correct check, if variable size has grown.
            SwTwips nOldHeight = bVert ? rOldSize.Width() : rOldSize.Height();
            if( nOldHeight < (Prt().*fnRect->fnGetHeight)() )
            {
                // If variable size of body|section frame has grown, only found
                // last lower and the position of the its next have to be invalidated.
                pLowerFrame->_InvalidateAll();
                pLowerFrame->InvalidatePage( pPage );
                if( !pLowerFrame->IsFlowFrame() ||
                    !SwFlowFrame::CastFlowFrame( pLowerFrame )->HasFollow() )
                    pLowerFrame->InvalidateNextPos( true );
                if ( pLowerFrame->IsTextFrame() )
                    static_cast<SwContentFrame*>(pLowerFrame)->Prepare( PREP_ADJUST_FRM );
            }
            else
            {
                // variable size of body|section frame has shrunk. Thus,
                // invalidate all lowers not matching the new body|section size
                // and the dedicated new last lower.
                if( bVert )
                {
                    SwTwips nBot = Frame().Left() + Prt().Left();
                    while ( pLowerFrame && pLowerFrame->GetPrev() && pLowerFrame->Frame().Left() < nBot )
                    {
                        pLowerFrame->_InvalidateAll();
                        pLowerFrame->InvalidatePage( pPage );
                        pLowerFrame = pLowerFrame->GetPrev();
                    }
                }
                else
                {
                    SwTwips nBot = Frame().Top() + Prt().Bottom();
                    while ( pLowerFrame && pLowerFrame->GetPrev() && pLowerFrame->Frame().Top() > nBot )
                    {
                        pLowerFrame->_InvalidateAll();
                        pLowerFrame->InvalidatePage( pPage );
                        pLowerFrame = pLowerFrame->GetPrev();
                    }
                }
                if ( pLowerFrame )
                {
                    pLowerFrame->_InvalidateSize();
                    pLowerFrame->InvalidatePage( pPage );
                    if ( pLowerFrame->IsTextFrame() )
                        static_cast<SwContentFrame*>(pLowerFrame)->Prepare( PREP_ADJUST_FRM );
                }
            }
            // #i41694# - improvement by removing duplicates
            if ( pLowerFrame )
            {
                if ( pLowerFrame->IsInSct() )
                {
                    // #i41694# - follow-up of issue #i10826#
                    // No invalidation of section frame, if it's the this.
                    SwFrame* pSectFrame = pLowerFrame->FindSctFrame();
                    if( pSectFrame != this && IsAnLower( pSectFrame ) )
                    {
                        pSectFrame->_InvalidateSize();
                        pSectFrame->InvalidatePage( pPage );
                    }
                }
            }
        }
        return;
    } // end of { special case }

    // Invalidate page for content only once.
    bool bInvaPageForContent = true;

    // Declare booleans <bFixChgd> and <bVarChgd>, indicating for text frame
    // adjustment, if fixed/variable size has changed.
    bool bFixChgd, bVarChgd;
    if( bVert == pLowerFrame->IsNeighbourFrame() )
    {
        bFixChgd = bWidthChgd;
        bVarChgd = bHeightChgd;
    }
    else
    {
        bFixChgd = bHeightChgd;
        bVarChgd = bWidthChgd;
    }

    // Declare const unsigned short <nFixWidth> and init it this frame types
    // which has fixed width in vertical respectively horizontal layout.
    // In vertical layout these are neighbour frames (cell and column frames),
    //      header frames and footer frames.
    // In horizontal layout these are all frames, which aren't neighbour frames.
    const sal_uInt16 nFixWidth = bVert ? (FRM_NEIGHBOUR | FRM_HEADFOOT)
                                   : ~FRM_NEIGHBOUR;

    // Declare const unsigned short <nFixHeight> and init it this frame types
    // which has fixed height in vertical respectively horizontal layout.
    // In vertical layout these are all frames, which aren't neighbour frames,
    //      header frames, footer frames, body frames or foot note container frames.
    // In horizontal layout these are neighbour frames.
    const sal_uInt16 nFixHeight= bVert ? ~(FRM_NEIGHBOUR | FRM_HEADFOOT | FRM_BODYFTNC)
                                   : FRM_NEIGHBOUR;

    // Travel through all lowers using <GetNext()>
    while ( pLowerFrame )
    {
        if ( pLowerFrame->IsTextFrame() )
        {
            // Text frames will only be invalidated - prepare invalidation
            if ( bFixChgd )
                static_cast<SwContentFrame*>(pLowerFrame)->Prepare( PREP_FIXSIZE_CHG );
            if ( bVarChgd )
                static_cast<SwContentFrame*>(pLowerFrame)->Prepare( PREP_ADJUST_FRM );
        }
        else
        {
            // If lower isn't a table, row, cell or section frame, adjust its
            // frame size.
            const sal_uInt16 nLowerType = pLowerFrame->GetType();
            if ( !(nLowerType & (FRM_TAB|FRM_ROW|FRM_CELL|FRM_SECTION)) )
            {
                if ( bWidthChgd )
                {
                    if( nLowerType & nFixWidth )
                    {
                        // Considering previous conditions:
                        // In vertical layout set width of column, header and
                        // footer frames to its upper width.
                        // In horizontal layout set width of header, footer,
                        // foot note container, foot note, body and no-text
                        // frames to its upper width.
                        pLowerFrame->Frame().Width( Prt().Width() );
                    }
                    else if( rOldSize.Width() && !pLowerFrame->IsFootnoteFrame() )
                    {
                        // Adjust frame width proportional, if lower isn't a
                        // foot note frame and condition <nLowerType & nFixWidth>
                        // isn't true.
                        // Considering previous conditions:
                        // In vertical layout these are foot note container,
                        // body and no-text frames.
                        // In horizontal layout these are column and no-text frames.
                        // OD 24.10.2002 #97265# - <double> calculation
                        // Perform <double> calculation of new width, if
                        // one of the coefficients is greater than 50000
                        SwTwips nNewWidth;
                        if ( (pLowerFrame->Frame().Width() > 50000) ||
                             (Prt().Width() > 50000) )
                        {
                            double nNewWidthTmp =
                                ( double(pLowerFrame->Frame().Width())
                                  * double(Prt().Width()) )
                                / double(rOldSize.Width());
                            nNewWidth = SwTwips(nNewWidthTmp);
                        }
                        else
                        {
                            nNewWidth =
                                (pLowerFrame->Frame().Width() * Prt().Width()) / rOldSize.Width();
                        }
                        pLowerFrame->Frame().Width( nNewWidth );
                    }
                }
                if ( bHeightChgd )
                {
                    if( nLowerType & nFixHeight )
                    {
                        // Considering previous conditions:
                        // In vertical layout set height of foot note and
                        // no-text frames to its upper height.
                        // In horizontal layout set height of column frames
                        // to its upper height.
                        pLowerFrame->Frame().Height( Prt().Height() );
                    }
                    // OD 01.10.2002 #102211#
                    // add conditions <!pLowerFrame->IsHeaderFrame()> and
                    // <!pLowerFrame->IsFooterFrame()> in order to avoid that
                    // the <Grow> of header or footer are overwritten.
                    // NOTE: Height of header/footer frame is determined by contents.
                    else if ( rOldSize.Height() &&
                              !pLowerFrame->IsFootnoteFrame() &&
                              !pLowerFrame->IsHeaderFrame() &&
                              !pLowerFrame->IsFooterFrame()
                            )
                    {
                        // Adjust frame height proportional, if lower isn't a
                        // foot note, a header or a footer frame and
                        // condition <nLowerType & nFixHeight> isn't true.
                        // Considering previous conditions:
                        // In vertical layout these are column, foot note container,
                        // body and no-text frames.
                        // In horizontal layout these are column, foot note
                        // container, body and no-text frames.

                        // OD 29.10.2002 #97265# - special case for page lowers
                        // The page lowers that have to be adjusted on page height
                        // change are the body frame and the foot note container
                        // frame.
                        // In vertical layout the height of both is directly
                        // adjusted to the page height change.
                        // In horizontal layout the height of the body frame is
                        // directly adjusted to the page height change and the
                        // foot note frame height isn't touched, because its
                        // determined by its content.
                        // OD 31.03.2003 #108446# - apply special case for page
                        // lowers - see description above - also for section columns.
                        if ( IsPageFrame() ||
                             ( IsColumnFrame() && IsInSct() )
                           )
                        {
                            OSL_ENSURE( pLowerFrame->IsBodyFrame() || pLowerFrame->IsFootnoteContFrame(),
                                    "ChgLowersProp - only for body or foot note container" );
                            if ( pLowerFrame->IsBodyFrame() || pLowerFrame->IsFootnoteContFrame() )
                            {
                                if ( IsVertical() || pLowerFrame->IsBodyFrame() )
                                {
                                    SwTwips nNewHeight =
                                            pLowerFrame->Frame().Height() +
                                            ( Prt().Height() - rOldSize.Height() );
                                    if ( nNewHeight < 0)
                                    {
                                        // OD 01.04.2003 #108446# - adjust assertion condition and text
                                        OSL_ENSURE( !( IsPageFrame() &&
                                                   (pLowerFrame->Frame().Height()>0) &&
                                                   (pLowerFrame->IsValid()) ),
                                                    "ChgLowersProg - negative height for lower.");
                                        nNewHeight = 0;
                                    }
                                    pLowerFrame->Frame().Height( nNewHeight );
                                }
                            }
                        }
                        else
                        {
                            SwTwips nNewHeight;
                            // OD 24.10.2002 #97265# - <double> calculation
                            // Perform <double> calculation of new height, if
                            // one of the coefficients is greater than 50000
                            if ( (pLowerFrame->Frame().Height() > 50000) ||
                                 (Prt().Height() > 50000) )
                            {
                                double nNewHeightTmp =
                                    ( double(pLowerFrame->Frame().Height())
                                      * double(Prt().Height()) )
                                    / double(rOldSize.Height());
                                nNewHeight = SwTwips(nNewHeightTmp);
                            }
                            else
                            {
                                nNewHeight = ( pLowerFrame->Frame().Height()
                                             * Prt().Height() ) / rOldSize.Height();
                            }
                            if( !pLowerFrame->GetNext() )
                            {
                                SwTwips nSum = Prt().Height();
                                SwFrame* pTmp = Lower();
                                while( pTmp->GetNext() )
                                {
                                    if( !pTmp->IsFootnoteContFrame() || !pTmp->IsVertical() )
                                        nSum -= pTmp->Frame().Height();
                                    pTmp = pTmp->GetNext();
                                }
                                if( nSum - nNewHeight == 1 &&
                                    nSum == pLowerFrame->Frame().Height() )
                                    nNewHeight = nSum;
                            }
                            pLowerFrame->Frame().Height( nNewHeight );
                        }
                    }
                }
            }
        } // end of else { NOT text frame }

        pLowerFrame->_InvalidateAll();
        if ( bInvaPageForContent && pLowerFrame->IsContentFrame() )
        {
            pLowerFrame->InvalidatePage();
            bInvaPageForContent = false;
        }

        if ( !pLowerFrame->GetNext() && pLowerFrame->IsRetoucheFrame() )
        {
            //If a growth took place and the subordinate elements can retouch
            //itself (currently Tabs, Sections and Content) we trigger it.
            if ( rOldSize.Height() < Prt().SSize().Height() ||
                 rOldSize.Width() < Prt().SSize().Width() )
                pLowerFrame->SetRetouche();
        }
        pLowerFrame = pLowerFrame->GetNext();
    }

    // Finally adjust the columns if width is set to auto
    // Possible optimization: execute this code earlier in this function and
    // return???
    if ( ( (bVert && bHeightChgd) || (! bVert && bWidthChgd) ) &&
           Lower()->IsColumnFrame() )
    {
        // get column attribute
        const SwFormatCol* pColAttr = nullptr;
        if ( IsPageBodyFrame() )
        {
            OSL_ENSURE( GetUpper()->IsPageFrame(), "Upper is not page frame" );
            pColAttr = &GetUpper()->GetFormat()->GetCol();
        }
        else
        {
            OSL_ENSURE( IsFlyFrame() || IsSctFrame(), "Columns not in fly or section" );
            pColAttr = &GetFormat()->GetCol();
        }

        if ( pColAttr->IsOrtho() && pColAttr->GetNumCols() > 1 )
            AdjustColumns( pColAttr, false );
    }
}

/** "Formats" the Frame; Frame and PrtArea.
 *
 * The Fixsize is not set here.
 */
void SwLayoutFrame::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "LayoutFrame::Format, pAttrs is 0." );

    if ( mbValidPrtArea && mbValidSize )
        return;

    bool bHideWhitespace = false;
    if (IsPageFrame())
    {
        SwViewShell* pShell = getRootFrame()->GetCurrShell();
        if (pShell && pShell->GetViewOptions()->IsWhitespaceHidden())
        {
            // This is needed so that no space is reserved for the margin on
            // the last page of the document. Other pages would have no margin
            // set even without this, as their frame height is the content
            // height already.
            bHideWhitespace = true;
        }
    }

    const sal_uInt16 nLeft = (sal_uInt16)pAttrs->CalcLeft(this);
    const sal_uInt16 nUpper = bHideWhitespace ? 0 : pAttrs->CalcTop();

    const sal_uInt16 nRight = (sal_uInt16)pAttrs->CalcRight(this);
    const sal_uInt16 nLower = bHideWhitespace ? 0 : pAttrs->CalcBottom();

    const bool bVert = IsVertical() && !IsPageFrame();
    SwRectFn fnRect = bVert ? ( IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;
    if ( !mbValidPrtArea )
    {
        mbValidPrtArea = true;
        (this->*fnRect->fnSetXMargins)( nLeft, nRight );
        (this->*fnRect->fnSetYMargins)( nUpper, nLower );
    }

    if ( !mbValidSize )
    {
        if ( !HasFixSize() )
        {
            const SwTwips nBorder = nUpper + nLower;
            const SwFormatFrameSize &rSz = GetFormat()->GetFrameSize();
            SwTwips nMinHeight = rSz.GetHeightSizeType() == ATT_MIN_SIZE ? rSz.GetHeight() : 0;
            do
            {   mbValidSize = true;

                //The size in VarSize is calculated using the content plus the
                // borders.
                SwTwips nRemaining = 0;
                SwFrame *pFrame = Lower();
                while ( pFrame )
                {   nRemaining += (pFrame->Frame().*fnRect->fnGetHeight)();
                    if( pFrame->IsTextFrame() && static_cast<SwTextFrame*>(pFrame)->IsUndersized() )
                    // This TextFrame would like to be a bit bigger
                        nRemaining += static_cast<SwTextFrame*>(pFrame)->GetParHeight()
                                      - (pFrame->Prt().*fnRect->fnGetHeight)();
                    else if( pFrame->IsSctFrame() && static_cast<SwSectionFrame*>(pFrame)->IsUndersized() )
                        nRemaining += static_cast<SwSectionFrame*>(pFrame)->Undersize();
                    pFrame = pFrame->GetNext();
                }
                nRemaining += nBorder;
                nRemaining = std::max( nRemaining, nMinHeight );
                const SwTwips nDiff = nRemaining-(Frame().*fnRect->fnGetHeight)();
                const long nOldLeft = (Frame().*fnRect->fnGetLeft)();
                const long nOldTop = (Frame().*fnRect->fnGetTop)();
                if ( nDiff )
                {
                    if ( nDiff > 0 )
                        Grow( nDiff );
                    else
                        Shrink( -nDiff );
                    //Updates the positions using the fast channel.
                    MakePos();
                }
                //Don't exceed the bottom edge of the Upper.
                if ( GetUpper() && (Frame().*fnRect->fnGetHeight)() )
                {
                    const SwTwips nLimit = (GetUpper()->*fnRect->fnGetPrtBottom)();
                    if( (this->*fnRect->fnSetLimit)( nLimit ) &&
                        nOldLeft == (Frame().*fnRect->fnGetLeft)() &&
                        nOldTop  == (Frame().*fnRect->fnGetTop)() )
                        mbValidSize = mbValidPrtArea = true;
                }
            } while ( !mbValidSize );
        }
        else if (GetType() & FRM_HEADFOOT)
        {
            do
            {   if ( Frame().Height() != pAttrs->GetSize().Height() )
                    ChgSize( Size( Frame().Width(), pAttrs->GetSize().Height()));
                mbValidSize = true;
                MakePos();
            } while ( !mbValidSize );
        }
        else
            mbValidSize = true;

        // While updating the size, PrtArea might be invalidated.
        if (!mbValidPrtArea)
        {
            mbValidPrtArea = true;
            (this->*fnRect->fnSetXMargins)(nLeft, nRight);
            (this->*fnRect->fnSetYMargins)(nUpper, nLower);
        }
    }
}

static void InvaPercentFlys( SwFrame *pFrame, SwTwips nDiff )
{
    OSL_ENSURE( pFrame->GetDrawObjs(), "Can't find any Objects" );
    for ( size_t i = 0; i < pFrame->GetDrawObjs()->size(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = (*pFrame->GetDrawObjs())[i];
        if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
        {
            SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pAnchoredObj);
            const SwFormatFrameSize &rSz = pFly->GetFormat()->GetFrameSize();
            if ( rSz.GetWidthPercent() || rSz.GetHeightPercent() )
            {
                bool bNotify = true;
                // If we've a fly with more than 90% relative height...
                if( rSz.GetHeightPercent() > 90 && pFly->GetAnchorFrame() &&
                    rSz.GetHeightPercent() != SwFormatFrameSize::SYNCED && nDiff )
                {
                    const SwFrame *pRel = pFly->IsFlyLayFrame() ? pFly->GetAnchorFrame():
                                        pFly->GetAnchorFrame()->GetUpper();
                    // ... and we have already more than 90% height and we
                    // not allow the text to go through...
                    // then a notifycation could cause an endless loop, e.g.
                    // 100% height and no text wrap inside a cell of a table.
                    if( pFly->Frame().Height()*10 >
                        ( nDiff + pRel->Prt().Height() )*9 &&
                        pFly->GetFormat()->GetSurround().GetSurround() !=
                        SURROUND_THROUGHT )
                       bNotify = false;
                }
                if( bNotify )
                    pFly->InvalidateSize();
            }
        }
    }
}

void SwLayoutFrame::InvaPercentLowers( SwTwips nDiff )
{
    if ( GetDrawObjs() )
        ::InvaPercentFlys( this, nDiff );

    SwFrame *pFrame = ContainsContent();
    if ( pFrame )
        do
        {
            if ( pFrame->IsInTab() && !IsTabFrame() )
            {
                SwFrame *pTmp = pFrame->FindTabFrame();
                OSL_ENSURE( pTmp, "Where's my TabFrame?" );
                if( IsAnLower( pTmp ) )
                    pFrame = pTmp;
            }

            if ( pFrame->IsTabFrame() )
            {
                const SwFormatFrameSize &rSz = static_cast<SwLayoutFrame*>(pFrame)->GetFormat()->GetFrameSize();
                if ( rSz.GetWidthPercent() || rSz.GetHeightPercent() )
                    pFrame->InvalidatePrt();
            }
            else if ( pFrame->GetDrawObjs() )
                ::InvaPercentFlys( pFrame, nDiff );
            pFrame = pFrame->FindNextCnt();
        } while ( pFrame && IsAnLower( pFrame ) ) ;
}

long SwLayoutFrame::CalcRel( const SwFormatFrameSize &rSz, bool ) const
{
    long nRet     = rSz.GetWidth(),
         nPercent = rSz.GetWidthPercent();

    if ( nPercent )
    {
        const SwFrame *pRel = GetUpper();
        long nRel = LONG_MAX;
        const SwViewShell *pSh = getRootFrame()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        if( pRel->IsPageBodyFrame() && pSh && bBrowseMode && pSh->VisArea().Width() )
        {
            nRel = pSh->GetBrowseWidth();
            long nDiff = nRel - pRel->Prt().Width();
            if ( nDiff > 0 )
                nRel -= nDiff;
        }
        nRel = std::min( nRel, pRel->Prt().Width() );
        nRet = nRel * nPercent / 100;
    }
    return nRet;
}

// Local helpers for SwLayoutFrame::FormatWidthCols()

static long lcl_CalcMinColDiff( SwLayoutFrame *pLayFrame )
{
    long nDiff = 0, nFirstDiff = 0;
    SwLayoutFrame *pCol = static_cast<SwLayoutFrame*>(pLayFrame->Lower());
    OSL_ENSURE( pCol, "Where's the columnframe?" );
    SwFrame *pFrame = pCol->Lower();
    do
    {
        if( pFrame && pFrame->IsBodyFrame() )
            pFrame = static_cast<SwBodyFrame*>(pFrame)->Lower();
        if ( pFrame && pFrame->IsTextFrame() )
        {
            const long nTmp = static_cast<SwTextFrame*>(pFrame)->FirstLineHeight();
            if ( nTmp != USHRT_MAX )
            {
                if ( pCol == pLayFrame->Lower() )
                    nFirstDiff = nTmp;
                else
                    nDiff = nDiff ? std::min( nDiff, nTmp ) : nTmp;
            }
        }
        //Skip empty columns!
        pCol = static_cast<SwLayoutFrame*>(pCol->GetNext());
        while ( pCol && nullptr == (pFrame = pCol->Lower()) )
            pCol = static_cast<SwLayoutFrame*>(pCol->GetNext());

    } while ( pFrame && pCol );

    return nDiff ? nDiff : nFirstDiff ? nFirstDiff : 240;
}

static bool lcl_IsFlyHeightClipped( SwLayoutFrame *pLay )
{
    SwFrame *pFrame = pLay->ContainsContent();
    while ( pFrame )
    {
        if ( pFrame->IsInTab() )
            pFrame = pFrame->FindTabFrame();

        if ( pFrame->GetDrawObjs() )
        {
            const size_t nCnt = pFrame->GetDrawObjs()->size();
            for ( size_t i = 0; i < nCnt; ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pFrame->GetDrawObjs())[i];
                if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
                {
                    SwFlyFrame* pFly = static_cast<SwFlyFrame*>(pAnchoredObj);
                    if ( pFly->IsHeightClipped() &&
                         ( !pFly->IsFlyFreeFrame() || pFly->GetPageFrame() ) )
                        return true;
                }
            }
        }
        pFrame = pFrame->FindNextCnt();
    }
    return false;
}

void SwLayoutFrame::FormatWidthCols( const SwBorderAttrs &rAttrs,
                                   const SwTwips nBorder, const SwTwips nMinHeight )
{
    //If there are columns involved, the size is adjusted using the last column.
    //1. Format content.
    //2. Calculate height of the last column: if it's too big, the Fly has to
    //   grow. The amount by which the Fly grows is not the amount of the
    //   overhang because we have to act on the assumption that some text flows
    //   back which will generate some more space.
    //   The amount which we grow by equals the overhang
    //   divided by the amount of columns or the overhang itself if it's smaller
    //   than the amount of columns.
    //3. Go back to 1. until everything is stable.

    const SwFormatCol &rCol = rAttrs.GetAttrSet().GetCol();
    const sal_uInt16 nNumCols = rCol.GetNumCols();

    bool bEnd = false;
    bool bBackLock = false;
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    SwViewShellImp *pImp = pSh ? pSh->Imp() : nullptr;
    vcl::RenderContext* pRenderContext = pSh ? pSh->GetOut() : nullptr;
    {
        // Underlying algorithm
        // We try to find the optimal height for the column.
        // nMinimum starts with the passed minimum height and is then remembered
        // as the maximum height on which column content still juts out of a
        // column.
        // nMaximum starts with LONG_MAX and is then remembered as the minimum
        // width on which the content fitted.
        // In column based sections nMaximum starts at the maximum value which
        // the surrounding defines, this can certainly be a value on which
        // content still juts out.
        // The columns are formatted. If content still juts out, nMinimum is
        // adjusted accordingly, then we grow, at least by uMinDiff but not
        // over a certain nMaximum. If no content juts out but there is still
        // some space left in the column, shrinking is done accordingly, at
        // least by nMindIff but not below the nMinimum.
        // Cancel as soon as no content juts out and the difference from minimum
        // to maximum is less than MinDiff or the maximum which was defined by
        // the surrounding is reached even if some content still juts out.

        // Criticism of this implementation
        // 1. Theoretically situations are possible in which the content fits in
        // a lower height but not in a higher height. To ensure that the code
        // handles such situations the code contains a few checks concerning
        // minimum and maximum which probably are never triggered.
        // 2. We use the same nMinDiff for shrinking and growing, but nMinDiff
        // is more or less the smallest first line height and doesn't seem ideal
        // as minimum value.

        long nMinimum = nMinHeight;
        long nMaximum;
        bool bNoBalance = false;
        SWRECTFN( this )
        if( IsSctFrame() )
        {
            nMaximum = (Frame().*fnRect->fnGetHeight)() - nBorder +
                       (Frame().*fnRect->fnBottomDist)(
                                        (GetUpper()->*fnRect->fnGetPrtBottom)() );
            nMaximum += GetUpper()->Grow( LONG_MAX, true );
            if( nMaximum < nMinimum )
            {
                if( nMaximum < 0 )
                    nMinimum = nMaximum = 0;
                else
                    nMinimum = nMaximum;
            }
            if( nMaximum > BROWSE_HEIGHT )
                nMaximum = BROWSE_HEIGHT;

            bNoBalance = static_cast<SwSectionFrame*>(this)->GetSection()->GetFormat()->
                         GetBalancedColumns().GetValue();
            SwFrame* pAny = ContainsAny();
            if( bNoBalance ||
                ( !(Frame().*fnRect->fnGetHeight)() && pAny ) )
            {
                long nTop = (this->*fnRect->fnGetTopMargin)();
                // #i23129# - correction
                // to the calculated maximum height.
                (Frame().*fnRect->fnAddBottom)( nMaximum -
                                              (Frame().*fnRect->fnGetHeight)() );
                if( nTop > nMaximum )
                    nTop = nMaximum;
                (this->*fnRect->fnSetYMargins)( nTop, 0 );
            }
            if( !pAny && !static_cast<SwSectionFrame*>(this)->IsFootnoteLock() )
            {
                SwFootnoteContFrame* pFootnoteCont = static_cast<SwSectionFrame*>(this)->ContainsFootnoteCont();
                if( pFootnoteCont )
                {
                    SwFrame* pFootnoteAny = pFootnoteCont->ContainsAny();
                    if( pFootnoteAny && pFootnoteAny->IsValid() )
                    {
                        bBackLock = true;
                        static_cast<SwSectionFrame*>(this)->SetFootnoteLock( true );
                    }
                }
            }
        }
        else
            nMaximum = LONG_MAX;

        // #i3317# - reset temporarly consideration
        // of wrapping style influence
        SwPageFrame* pPageFrame = FindPageFrame();
        SwSortedObjs* pObjs = pPageFrame ? pPageFrame->GetSortedObjs() : nullptr;
        if ( pObjs )
        {
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];

                if ( IsAnLower( pAnchoredObj->GetAnchorFrame() ) )
                {
                    pAnchoredObj->SetTmpConsiderWrapInfluence( false );
                }
            }
        }
        do
        {
            //Could take a while therefore check for Waitcrsr here.
            if ( pImp )
                pImp->CheckWaitCursor();

            mbValidSize = true;
            //First format the column as this will relieve the stack a bit.
            //Also set width and height of the column (if they are wrong)
            //while we are at it.
            SwLayoutFrame *pCol = static_cast<SwLayoutFrame*>(Lower());

            // #i27399#
            // Simply setting the column width based on the values returned by
            // CalcColWidth does not work for automatic column width.
            AdjustColumns( &rCol, false );

            for ( sal_uInt16 i = 0; i < nNumCols; ++i )
            {
                pCol->Calc(pRenderContext);
                // ColumnFrames have a BodyFrame now, which needs to be calculated
                pCol->Lower()->Calc(pRenderContext);
                if( pCol->Lower()->GetNext() )
                    pCol->Lower()->GetNext()->Calc(pRenderContext);  // SwFootnoteCont
                pCol = static_cast<SwLayoutFrame*>(pCol->GetNext());
            }

            ::CalcContent( this );

            pCol = static_cast<SwLayoutFrame*>(Lower());
            OSL_ENSURE( pCol && pCol->GetNext(), ":-( column making holidays?");
            // set bMinDiff if no empty columns exist
            bool bMinDiff = true;
            // OD 28.03.2003 #108446# - check for all column content and all columns
            while ( bMinDiff && pCol )
            {
                bMinDiff = nullptr != pCol->ContainsContent();
                pCol = static_cast<SwLayoutFrame*>(pCol->GetNext());
            }
            pCol = static_cast<SwLayoutFrame*>(Lower());
            // OD 28.03.2003 #108446# - initialize local variable
            SwTwips nDiff = 0;
            SwTwips nMaxFree = 0;
            SwTwips nAllFree = LONG_MAX;
            // set bFoundLower if there is at least one non-empty column
            bool bFoundLower = false;
            while( pCol )
            {
                SwLayoutFrame* pLay = static_cast<SwLayoutFrame*>(pCol->Lower());
                SwTwips nInnerHeight = (pLay->Frame().*fnRect->fnGetHeight)() -
                                       (pLay->Prt().*fnRect->fnGetHeight)();
                if( pLay->Lower() )
                {
                    bFoundLower = true;
                    nInnerHeight += pLay->InnerHeight();
                }
                else if( nInnerHeight < 0 )
                    nInnerHeight = 0;

                if( pLay->GetNext() )
                {
                    bFoundLower = true;
                    pLay = static_cast<SwLayoutFrame*>(pLay->GetNext());
                    OSL_ENSURE( pLay->IsFootnoteContFrame(),"FootnoteContainer expected" );
                    nInnerHeight += pLay->InnerHeight();
                    nInnerHeight += (pLay->Frame().*fnRect->fnGetHeight)() -
                                    (pLay->Prt().*fnRect->fnGetHeight)();
                }
                nInnerHeight -= (pCol->Prt().*fnRect->fnGetHeight)();
                if( nInnerHeight > nDiff )
                {
                    nDiff = nInnerHeight;
                    nAllFree = 0;
                }
                else
                {
                    if( nMaxFree < -nInnerHeight )
                        nMaxFree = -nInnerHeight;
                    if( nAllFree > -nInnerHeight )
                        nAllFree = -nInnerHeight;
                }
                pCol = static_cast<SwLayoutFrame*>(pCol->GetNext());
            }

            if ( bFoundLower || ( IsSctFrame() && static_cast<SwSectionFrame*>(this)->HasFollow() ) )
            {
                SwTwips nMinDiff = ::lcl_CalcMinColDiff( this );
                // Here we decide if growing is needed - this is the case, if
                // column content (nDiff) or a Fly juts over.
                // In sections with columns we take into account to set the size
                // when having a non-empty Follow.
                if ( nDiff || ::lcl_IsFlyHeightClipped( this ) ||
                     ( IsSctFrame() && static_cast<SwSectionFrame*>(this)->CalcMinDiff( nMinDiff ) ) )
                {
                    long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                    // The minimum must not be smaller than our PrtHeight as
                    // long as something juts over.
                    if( nMinimum < nPrtHeight )
                        nMinimum = nPrtHeight;
                    // The maximum must not be smaller than PrtHeight if
                    // something still juts over.
                    if( nMaximum < nPrtHeight )
                        nMaximum = nPrtHeight;  // Robust, but will this ever happen?
                    if( !nDiff ) // If only Flys jut over, we grow by nMinDiff
                        nDiff = nMinDiff;
                    // If we should grow more than by nMinDiff we split it over
                    // the columns
                    if ( std::abs(nDiff - nMinDiff) > nNumCols && nDiff > (long)nNumCols )
                        nDiff /= nNumCols;

                    if ( bMinDiff )
                    {   // If no empty column exists, we want to grow at least
                        // by nMinDiff. Special case: If we are smaller than the
                        // minimal FrameHeight and PrtHeight is smaller than
                        // nMindiff we grow in a way that PrtHeight is exactly
                        // nMinDiff afterwards.
                        long nFrameHeight = (Frame().*fnRect->fnGetHeight)();
                        if ( nFrameHeight > nMinHeight || nPrtHeight >= nMinDiff )
                            nDiff = std::max( nDiff, nMinDiff );
                        else if( nDiff < nMinDiff )
                            nDiff = nMinDiff - nPrtHeight + 1;
                    }
                    // nMaximum has a size which fits the content or the
                    // requested value from the surrounding therefore we don't
                    // need to exceed this value.
                    if( nDiff + nPrtHeight > nMaximum )
                        nDiff = nMaximum - nPrtHeight;
                }
                else if( nMaximum > nMinimum ) // We fit, do we still have some margin?
                {
                    long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                    if ( nMaximum < nPrtHeight )
                        nDiff = nMaximum - nPrtHeight; // We grew over a working
                        // height and shrink back to it, but will this ever
                        // happen?
                    else
                    {   // We have a new maximum, a size which fits for the content.
                        nMaximum = nPrtHeight;
                        // If the margin in the column is bigger than nMinDiff
                        // and we therefore drop under the minimum, we deflate
                        // a bit.
                        if ( !bNoBalance &&
                             // #i23129# - <nMinDiff> can be
                             // big, because of an object at the beginning of
                             // a column. Thus, decrease optimization here.
                             //nMaxFree >= nMinDiff &&
                             nMaxFree > 0 &&
                             ( !nAllFree ||
                               nMinimum < nPrtHeight - nMinDiff ) )
                        {
                            nMaxFree /= nNumCols; // disperse over the columns
                            nDiff = nMaxFree < nMinDiff ? -nMinDiff : -nMaxFree; // min nMinDiff
                            if( nPrtHeight + nDiff <= nMinimum ) // below the minimum?
                                nDiff = ( nMinimum - nMaximum ) / 2; // Take the center
                        }
                        else if( nAllFree )
                        {
                            nDiff = -nAllFree;
                            if( nPrtHeight + nDiff <= nMinimum ) // Less than minimum?
                                nDiff = ( nMinimum - nMaximum ) / 2; // Take the center
                        }
                    }
                }
                if( nDiff ) // now we shrink or grow...
                {
                    Size aOldSz( Prt().SSize() );
                    long nTop = (this->*fnRect->fnGetTopMargin)();
                    nDiff = (Prt().*fnRect->fnGetHeight)() + nDiff + nBorder -
                            (Frame().*fnRect->fnGetHeight)();
                    (Frame().*fnRect->fnAddBottom)( nDiff );
                    // #i68520#
                    SwFlyFrame *pFlyFrame = dynamic_cast<SwFlyFrame*>(this);
                    if (pFlyFrame)
                    {
                        pFlyFrame->InvalidateObjRectWithSpaces();
                    }
                    (this->*fnRect->fnSetYMargins)( nTop, nBorder - nTop );
                    ChgLowersProp( aOldSz );
                    NotifyLowerObjs();

                    // #i3317# - reset temporarly consideration
                    // of wrapping style influence
                    SwPageFrame* pTmpPageFrame = FindPageFrame();
                    SwSortedObjs* pTmpObjs = pTmpPageFrame ? pTmpPageFrame->GetSortedObjs() : nullptr;
                    if ( pTmpObjs )
                    {
                        for ( size_t i = 0; i < pTmpObjs->size(); ++i )
                        {
                            SwAnchoredObject* pAnchoredObj = (*pTmpObjs)[i];

                            if ( IsAnLower( pAnchoredObj->GetAnchorFrame() ) )
                            {
                                pAnchoredObj->SetTmpConsiderWrapInfluence( false );
                            }
                        }
                    }
                    //Invalidate suitable to nicely balance the Frames.
                    //- Every first one after the second column gets a
                    //  InvalidatePos();
                    pCol = static_cast<SwLayoutFrame*>(Lower()->GetNext());
                    while ( pCol )
                    {
                        SwFrame *pLow = pCol->Lower();
                        if ( pLow )
                            pLow->_InvalidatePos();
                        pCol = static_cast<SwLayoutFrame*>(pCol->GetNext());
                    }
                    if( IsSctFrame() && static_cast<SwSectionFrame*>(this)->HasFollow() )
                    {
                        // If we created a Follow, we need to give its content
                        // the opportunity to flow back inside the CalcContent
                        SwContentFrame* pTmpContent =
                            static_cast<SwSectionFrame*>(this)->GetFollow()->ContainsContent();
                        if( pTmpContent )
                            pTmpContent->_InvalidatePos();
                    }
                }
                else
                    bEnd = true;
            }
            else
                bEnd = true;

        } while ( !bEnd || !mbValidSize );
    }
    // OD 01.04.2003 #108446# - Don't collect endnotes for sections. Thus, set
    // 2nd parameter to <true>.
    ::CalcContent( this, true );
    if( IsSctFrame() )
    {
        // OD 14.03.2003 #i11760# - adjust 2nd parameter - sal_True --> true
        ::CalcContent( this, true );
        if( bBackLock )
            static_cast<SwSectionFrame*>(this)->SetFootnoteLock( false );
    }
}

static SwContentFrame* lcl_InvalidateSection( SwFrame *pCnt, SwInvalidateFlags nInv )
{
    SwSectionFrame* pSect = pCnt->FindSctFrame();
    // If our ContentFrame is placed inside a table or a footnote, only sections
    // which are also placed inside are meant.
    // Exception: If a table is directly passed.
    if( ( ( pCnt->IsInTab() && !pSect->IsInTab() ) ||
        ( pCnt->IsInFootnote() && !pSect->IsInFootnote() ) ) && !pCnt->IsTabFrame() )
        return nullptr;
    if( nInv & SwInvalidateFlags::Size )
        pSect->_InvalidateSize();
    if( nInv & SwInvalidateFlags::Pos )
        pSect->_InvalidatePos();
    if( nInv & SwInvalidateFlags::PrtArea )
        pSect->_InvalidatePrt();
    SwFlowFrame *pFoll = pSect->GetFollow();
    // Temporary separation from follow
    pSect->SetFollow( nullptr );
    SwContentFrame* pRet = pSect->FindLastContent();
    pSect->SetFollow( pFoll );
    return pRet;
}

static SwContentFrame* lcl_InvalidateTable( SwTabFrame *pTable, SwInvalidateFlags nInv )
{
    if( ( nInv & SwInvalidateFlags::Section ) && pTable->IsInSct() )
        lcl_InvalidateSection( pTable, nInv );
    if( nInv & SwInvalidateFlags::Size )
        pTable->_InvalidateSize();
    if( nInv & SwInvalidateFlags::Pos )
        pTable->_InvalidatePos();
    if( nInv & SwInvalidateFlags::PrtArea )
        pTable->_InvalidatePrt();
    return pTable->FindLastContent();
}

static void lcl_InvalidateAllContent( SwContentFrame *pCnt, SwInvalidateFlags nInv );

static void lcl_InvalidateContent( SwContentFrame *pCnt, SwInvalidateFlags nInv )
{
    SwContentFrame *pLastTabCnt = nullptr;
    SwContentFrame *pLastSctCnt = nullptr;
    while ( pCnt )
    {
        if( nInv & SwInvalidateFlags::Section )
        {
            if( pCnt->IsInSct() )
            {
                // See above at tables
                if( !pLastSctCnt )
                    pLastSctCnt = lcl_InvalidateSection( pCnt, nInv );
                if( pLastSctCnt == pCnt )
                    pLastSctCnt = nullptr;
            }
#if OSL_DEBUG_LEVEL > 0
            else
                OSL_ENSURE( !pLastSctCnt, "Where's the last SctContent?" );
#endif
        }
        if( nInv & SwInvalidateFlags::Table )
        {
            if( pCnt->IsInTab() )
            {
                // To not call FindTabFrame() for each ContentFrame of a table and
                // then invalidate the table, we remember the last ContentFrame of
                // the table and ignore IsInTab() until we are past it.
                // When entering the table, LastSctCnt is set to null, so
                // sections inside the table are correctly invalidated.
                // If the table itself is in a section the
                // invalidation is done three times, which is acceptable.
                if( !pLastTabCnt )
                {
                    pLastTabCnt = lcl_InvalidateTable( pCnt->FindTabFrame(), nInv );
                    pLastSctCnt = nullptr;
                }
                if( pLastTabCnt == pCnt )
                {
                    pLastTabCnt = nullptr;
                    pLastSctCnt = nullptr;
                }
            }
#if OSL_DEBUG_LEVEL > 0
            else
                OSL_ENSURE( !pLastTabCnt, "Where's the last TabContent?" );
#endif
        }

        if( nInv & SwInvalidateFlags::Size )
            pCnt->Prepare( PREP_CLEAR, nullptr, false );
        if( nInv & SwInvalidateFlags::Pos )
            pCnt->_InvalidatePos();
        if( nInv & SwInvalidateFlags::PrtArea )
            pCnt->_InvalidatePrt();
        if ( nInv & SwInvalidateFlags::LineNum )
            pCnt->InvalidateLineNum();
        if ( pCnt->GetDrawObjs() )
            lcl_InvalidateAllContent( pCnt, nInv );
        pCnt = pCnt->GetNextContentFrame();
    }
}

static void lcl_InvalidateAllContent( SwContentFrame *pCnt, SwInvalidateFlags nInv )
{
    SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
    for ( size_t i = 0; i < rObjs.size(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = rObjs[i];
        if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
        {
            SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pAnchoredObj);
            if ( pFly->IsFlyInContentFrame() )
            {
                ::lcl_InvalidateContent( pFly->ContainsContent(), nInv );
                if( nInv & SwInvalidateFlags::Direction )
                    pFly->CheckDirChange();
            }
        }
    }
}

void SwRootFrame::InvalidateAllContent( SwInvalidateFlags nInv )
{
    // First process all page bound FlyFrames.
    SwPageFrame *pPage = static_cast<SwPageFrame*>(Lower());
    while( pPage )
    {
        pPage->InvalidateFlyLayout();
        pPage->InvalidateFlyContent();
        pPage->InvalidateFlyInCnt();
        pPage->InvalidateLayout();
        pPage->InvalidateContent();
        pPage->InvalidatePage( pPage ); // So even the Turbo disappears if applicable

        if ( pPage->GetSortedObjs() )
        {
            const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
                {
                    SwFlyFrame* pFly = static_cast<SwFlyFrame*>(pAnchoredObj);
                    ::lcl_InvalidateContent( pFly->ContainsContent(), nInv );
                    if ( nInv & SwInvalidateFlags::Direction )
                        pFly->CheckDirChange();
                }
            }
        }
        if( nInv & SwInvalidateFlags::Direction )
            pPage->CheckDirChange();
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    }

    //Invalidate the whole document content and the character bound Flys here.
    ::lcl_InvalidateContent( ContainsContent(), nInv );

    if( nInv & SwInvalidateFlags::PrtArea )
    {
        SwViewShell *pSh  = getRootFrame()->GetCurrShell();
        if( pSh )
            pSh->InvalidateWindows( Frame() );
    }
}

/**
 * Invalidate/re-calculate the position of all floating screen objects (Writer fly frames and
 * drawing objects), that are anchored to paragraph or to character. (2004-03-16 #i11860#)
 */
void SwRootFrame::InvalidateAllObjPos()
{
    const SwPageFrame* pPageFrame = static_cast<const SwPageFrame*>(Lower());
    while( pPageFrame )
    {
        pPageFrame->InvalidateFlyLayout();

        if ( pPageFrame->GetSortedObjs() )
        {
            const SwSortedObjs& rObjs = *(pPageFrame->GetSortedObjs());
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                const SwFormatAnchor& rAnch = pAnchoredObj->GetFrameFormat().GetAnchor();
                if ((rAnch.GetAnchorId() != FLY_AT_PARA) &&
                    (rAnch.GetAnchorId() != FLY_AT_CHAR))
                {
                    // only to paragraph and to character anchored objects are considered.
                    continue;
                }
                // #i28701# - special invalidation for anchored
                // objects, whose wrapping style influence has to be considered.
                if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() )
                    pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
                else
                    pAnchoredObj->InvalidateObjPos();
            }
        }

        pPageFrame = static_cast<const SwPageFrame*>(pPageFrame->GetNext());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
