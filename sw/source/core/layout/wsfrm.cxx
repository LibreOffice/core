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

SwFrm::SwFrm( SwModify *pMod, SwFrm* pSib ) :
    SwClient( pMod ),
    mbIfAccTableShouldDisposing( false ), //A member to identify if the acc table should dispose
    mbInDtor(false),
    mnFrmId( SwFrm::mnLastFrmId++ ),
    mpRoot( pSib ? pSib->getRootFrm() : nullptr ),
    mpUpper(nullptr),
    mpNext(nullptr),
    mpPrev(nullptr),
    mpDrawObjs(nullptr),
    mnFrmType(0),
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

const IDocumentDrawModelAccess& SwFrm::getIDocumentDrawModelAccess()
{
    return GetUpper()->GetFormat()->getIDocumentDrawModelAccess();
}

bool SwFrm::KnowsFormat( const SwFormat& rFormat ) const
{
    return GetRegisteredIn() == &rFormat;
}

void SwFrm::RegisterToFormat( SwFormat& rFormat )
{
    rFormat.Add( this );
}

void SwFrm::CheckDir( sal_uInt16 nDir, bool bVert, bool bOnlyBiDi, bool bBrowse )
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

void SwFrm::CheckDirection( bool bVert )
{
    if( bVert )
    {
        if( !IsHeaderFrm() && !IsFooterFrm() )
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

void SwSectionFrm::CheckDirection( bool bVert )
{
    const SwFrameFormat* pFormat = GetFormat();
    if( pFormat )
    {
        const SwViewShell *pSh = getRootFrm()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        CheckDir(static_cast<const SvxFrameDirectionItem&>(pFormat->GetFormatAttr(RES_FRAMEDIR)).GetValue(),
                    bVert, true, bBrowseMode );
    }
    else
        SwFrm::CheckDirection( bVert );
}

void SwFlyFrm::CheckDirection( bool bVert )
{
    const SwFrameFormat* pFormat = GetFormat();
    if( pFormat )
    {
        const SwViewShell *pSh = getRootFrm()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        CheckDir(static_cast<const SvxFrameDirectionItem&>(pFormat->GetFormatAttr(RES_FRAMEDIR)).GetValue(),
                    bVert, false, bBrowseMode );
    }
    else
        SwFrm::CheckDirection( bVert );
}

void SwTabFrm::CheckDirection( bool bVert )
{
    const SwFrameFormat* pFormat = GetFormat();
    if( pFormat )
    {
        const SwViewShell *pSh = getRootFrm()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        CheckDir(static_cast<const SvxFrameDirectionItem&>(pFormat->GetFormatAttr(RES_FRAMEDIR)).GetValue(),
                    bVert, true, bBrowseMode );
    }
    else
        SwFrm::CheckDirection( bVert );
}

void SwCellFrm::CheckDirection( bool bVert )
{
    const SwFrameFormat* pFormat = GetFormat();
    const SfxPoolItem* pItem;
    // Check if the item is set, before actually
    // using it. Otherwise the dynamic pool default is used, which may be set
    // to LTR in case of OOo 1.0 documents.
    if( pFormat && SfxItemState::SET == pFormat->GetItemState( RES_FRAMEDIR, true, &pItem ) )
    {
        const SvxFrameDirectionItem* pFrmDirItem = static_cast<const SvxFrameDirectionItem*>(pItem);
        const SwViewShell *pSh = getRootFrm()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        CheckDir( pFrmDirItem->GetValue(), bVert, false, bBrowseMode );
    }
    else
        SwFrm::CheckDirection( bVert );
}

void SwTextFrm::CheckDirection( bool bVert )
{
    const SwViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
    CheckDir( GetNode()->GetSwAttrSet().GetFrmDir().GetValue(), bVert,
              true, bBrowseMode );
}

void SwFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_uInt8 nInvFlags = 0;

    if( pOld && pNew && RES_ATTRSET_CHG == pNew->Which() )
    {
        SfxItemIter aNIter( *static_cast<const SwAttrSetChg*>(pNew)->GetChgSet() );
        SfxItemIter aOIter( *static_cast<const SwAttrSetChg*>(pOld)->GetChgSet() );
        while( true )
        {
            _UpdateAttrFrm( aOIter.GetCurItem(),
                         aNIter.GetCurItem(), nInvFlags );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
    }
    else
        _UpdateAttrFrm( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        SwPageFrm *pPage = FindPageFrm();
        InvalidatePage( pPage );
        if ( nInvFlags & 0x01 )
        {
            _InvalidatePrt();
            if( !GetPrev() && IsTabFrm() && IsInSct() )
                FindSctFrm()->_InvalidatePrt();
        }
        if ( nInvFlags & 0x02 )
            _InvalidateSize();
        if ( nInvFlags & 0x04 )
            _InvalidatePos();
        if ( nInvFlags & 0x08 )
            SetCompletePaint();
        SwFrm *pNxt;
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

void SwFrm::_UpdateAttrFrm( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                         sal_uInt8 &rInvFlags )
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_BOX:
        case RES_SHADOW:
            Prepare( PREP_FIXSIZE_CHG );
            // no break here!
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
            ReinitializeFrmSizeAttrFlags();
            rInvFlags |= 0x13;
            break;

        case RES_FMT_CHG:
            rInvFlags |= 0x0F;
            break;

        case RES_ROW_SPLIT:
        {
            if ( IsRowFrm() )
            {
                bool bInFollowFlowRow = nullptr != IsInFollowFlowRow();
                if ( bInFollowFlowRow || nullptr != IsInSplitTableRow() )
                {
                    SwTabFrm* pTab = FindTabFrm();
                    if ( bInFollowFlowRow )
                        pTab = pTab->FindMaster();
                    pTab->SetRemoveFollowFlowLinePending( true );
                }
            }
            break;
        }
        case RES_COL:
            OSL_FAIL( "Columns for new FrmTyp?" );
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

bool SwFrm::Prepare( const PrepareHint, const void *, bool )
{
    /* Do nothing */
    return false;
}

/**
 * Invalidates the page in which the Frm is currently placed.
 * The page is invalidated depending on the type (Layout, Content, FlyFrm)
 */
void SwFrm::InvalidatePage( const SwPageFrm *pPage ) const
{
    if ( !pPage )
    {
        pPage = FindPageFrm();
        // #i28701# - for at-character and as-character
        // anchored Writer fly frames additionally invalidate also page frame
        // its 'anchor character' is on.
        if ( pPage && pPage->GetUpper() && IsFlyFrm() )
        {
            const SwFlyFrm* pFlyFrm = static_cast<const SwFlyFrm*>(this);
            if ( pFlyFrm->IsAutoPos() || pFlyFrm->IsFlyInCntFrm() )
            {
                // #i33751#, #i34060# - method <GetPageFrmOfAnchor()>
                // is replaced by method <FindPageFrmOfAnchor()>. It's return value
                // have to be checked.
                SwPageFrm* pPageFrmOfAnchor =
                        const_cast<SwFlyFrm*>(pFlyFrm)->FindPageFrmOfAnchor();
                if ( pPageFrmOfAnchor && pPageFrmOfAnchor != pPage )
                {
                    InvalidatePage( pPageFrmOfAnchor );
                }
            }
        }
    }

    if ( pPage && pPage->GetUpper() )
    {
        if ( pPage->GetFormat()->GetDoc()->IsInDtor() )
            return;

        SwRootFrm *pRoot = const_cast<SwRootFrm*>(static_cast<const SwRootFrm*>(pPage->GetUpper()));
        const SwFlyFrm *pFly = FindFlyFrm();
        if ( IsContentFrm() )
        {
            if ( pRoot->IsTurboAllowed() )
            {
                // If a ContentFrame wants to register for a second time, make it a TurboAction.
                if ( !pRoot->GetTurbo() || this == pRoot->GetTurbo() )
                    pRoot->SetTurbo( static_cast<const SwContentFrm*>(this) );
                else
                {
                    pRoot->DisallowTurbo();
                    //The page of the Turbo could be a different one then mine,
                    //therefore we have to invalidate it.
                    const SwFrm *pTmp = pRoot->GetTurbo();
                    pRoot->ResetTurbo();
                    pTmp->InvalidatePage();
                }
            }
            if ( !pRoot->GetTurbo() )
            {
                if ( pFly )
                {   if( !pFly->IsLocked() )
                    {
                        if ( pFly->IsFlyInCntFrm() )
                        {   pPage->InvalidateFlyInCnt();
                            static_cast<const SwFlyInCntFrm*>(pFly)->InvalidateContent();
                            pFly->GetAnchorFrm()->InvalidatePage();
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
                    if ( pFly->IsFlyInCntFrm() )
                    {
                        pPage->InvalidateFlyInCnt();
                        static_cast<const SwFlyInCntFrm*>(pFly)->InvalidateLayout();
                        pFly->GetAnchorFrm()->InvalidatePage();
                    }
                    else
                        pPage->InvalidateFlyLayout();
                }
            }
            else
                pPage->InvalidateLayout();

            if ( pRoot->GetTurbo() )
            {   const SwFrm *pTmp = pRoot->GetTurbo();
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

Size SwFrm::ChgSize( const Size& aNewSize )
{
    mbFixSize = true;
    const Size aOldSize( Frm().SSize() );
    if ( aNewSize == aOldSize )
        return aOldSize;

    if ( GetUpper() )
    {
        SWRECTFN2( this )
        SwRect aNew( Point(0,0), aNewSize );
        (maFrm.*fnRect->fnSetWidth)( (aNew.*fnRect->fnGetWidth)() );
        long nNew = (aNew.*fnRect->fnGetHeight)();
        long nDiff = nNew - (maFrm.*fnRect->fnGetHeight)();
        if( nDiff )
        {
            if ( GetUpper()->IsFootnoteBossFrm() && HasFixSize() &&
                 NA_GROW_SHRINK !=
                 static_cast<SwFootnoteBossFrm*>(GetUpper())->NeighbourhoodAdjustment( this ) )
            {
                (maFrm.*fnRect->fnSetHeight)( nNew );
                SwTwips nReal = static_cast<SwLayoutFrm*>(this)->AdjustNeighbourhood(nDiff);
                if ( nReal != nDiff )
                    (maFrm.*fnRect->fnSetHeight)( nNew - nDiff + nReal );
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

                    if ( GetUpper() && (maFrm.*fnRect->fnGetHeight)() != nNew )
                        GetUpper()->_InvalidateSize();
                }

                // Even if grow/shrink did not yet set the desired width, for
                // example when called by ChgColumns to set the column width, we
                // set the right width now.
                (maFrm.*fnRect->fnSetHeight)( nNew );
            }
        }
    }
    else
        maFrm.SSize( aNewSize );

    if ( Frm().SSize() != aOldSize )
    {
        SwPageFrm *pPage = FindPageFrm();
        if ( GetNext() )
        {
            GetNext()->_InvalidatePos();
            GetNext()->InvalidatePage( pPage );
        }
        if( IsLayoutFrm() )
        {
            if( IsRightToLeft() )
                _InvalidatePos();
            if( static_cast<SwLayoutFrm*>(this)->Lower() )
                static_cast<SwLayoutFrm*>(this)->Lower()->_InvalidateSize();
        }
        _InvalidatePrt();
        _InvalidateSize();
        InvalidatePage( pPage );
    }

    return maFrm.SSize();
}

/** Insert SwFrm into existing structure.
 *
 * Insertion is done below the parent either before pBehind or
 * at the end of the chain if pBehind is empty.
 */
void SwFrm::InsertBefore( SwLayoutFrm* pParent, SwFrm* pBehind )
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

/** Insert SwFrm into existing structure.
 *
 * Insertion is done below the parent either after pBehind or
 * at the beginning of the chain if pBehind is empty.
 */
void SwFrm::InsertBehind( SwLayoutFrm *pParent, SwFrm *pBefore )
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

/** Insert a chain of SwFrms into an existing struction
 *
 * Currently, this method is used to insert a SectionFrame (which may have some siblings) into an
 * existing structure. If the third parameter is NULL, this method is (besides handling the
 * siblings) equal to SwFrm::InsertBefore(..).
 *
 * If the third parameter is passed, the following happens:
 *  - this becomes mpNext of pParent
 *  - pSct becomes mpNext of the last one in the this-chain
 *  - pBehind is reconnected from pParent to pSct
 * The purpose is: a SectionFrm (this) won't become a child of another SectionFrm (pParent), but
 * pParent gets split into two siblings (pParent+pSect) and this is inserted between.
 */
bool SwFrm::InsertGroupBefore( SwFrm* pParent, SwFrm* pBehind, SwFrm* pSct )
{
    OSL_ENSURE( pParent, "No parent for insert." );
    OSL_ENSURE( (!pBehind || ( (pBehind && (pParent == pBehind->GetUpper()))
            || ((pParent->IsSctFrm() && pBehind->GetUpper()->IsColBodyFrm())) ) ),
            "Frame tree inconsistent." );
    if( pSct )
    {
        mpUpper = pParent->GetUpper();
        SwFrm *pLast = this;
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
            SwLayoutFrm* pTmp = static_cast<SwLayoutFrm*>(pSct);
            if( pTmp->Lower() )
            {
                OSL_ENSURE( pTmp->Lower()->IsColumnFrm(), "InsertGrp: Used SectionFrm" );
                pTmp = static_cast<SwLayoutFrm*>(static_cast<SwLayoutFrm*>(pTmp->Lower())->Lower());
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
            OSL_ENSURE( pSct->IsSctFrm(), "InsertGroup: For SectionFrms only" );
            SwFrm::DestroyFrm(pSct);
            return false;
        }
    }
    else
    {
        mpUpper = static_cast<SwLayoutFrm*>(pParent);
        SwFrm *pLast = this;
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

void SwFrm::RemoveFromLayout()
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

void SwContentFrm::Paste( SwFrm* pParent, SwFrm* pSibling)
{
    OSL_ENSURE( pParent, "No parent for pasting." );
    OSL_ENSURE( pParent->IsLayoutFrm(), "Parent is ContentFrm." );
    OSL_ENSURE( pParent != this, "I'm the parent." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I'm still registered somewhere" );
    OSL_ENSURE( !pSibling || pSibling->IsFlowFrm(),
            "<SwContentFrm::Paste(..)> - sibling not of expected type." );

    //Insert in the tree.
    InsertBefore( static_cast<SwLayoutFrm*>(pParent), pSibling );

    SwPageFrm *pPage = FindPageFrm();
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
        SwFrm* pNxt = GetNext();
        pNxt->_InvalidatePrt();
        pNxt->_InvalidatePos();
        pNxt->InvalidatePage( pPage );
        if( pNxt->IsSctFrm() )
            pNxt = static_cast<SwSectionFrm*>(pNxt)->ContainsContent();
        if( pNxt && pNxt->IsTextFrm() && pNxt->IsInFootnote() )
            pNxt->Prepare( PREP_FTN, nullptr, false );
    }

    if ( Frm().Height() )
        pParent->Grow( Frm().Height() );

    if ( Frm().Width() != pParent->Prt().Width() )
        Prepare( PREP_FIXSIZE_CHG );

    if ( GetPrev() )
    {
        if ( IsFollow() )
            //I'm a direct follower of my master now
            static_cast<SwContentFrm*>(GetPrev())->Prepare( PREP_FOLLOW_FOLLOWS );
        else
        {
            if ( GetPrev()->Frm().Height() !=
                 GetPrev()->Prt().Height() + GetPrev()->Prt().Top() )
            {
                // Take the border into account?
                GetPrev()->_InvalidatePrt();
            }
            // OD 18.02.2003 #104989# - force complete paint of previous frame,
            // if frame is inserted at the end of a section frame, in order to
            // get subsidiary lines repainted for the section.
            if ( pParent->IsSctFrm() && !GetNext() )
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
        SwFrm* pFrm = GetIndPrev();
        if( pFrm && pFrm->IsSctFrm() )
            pFrm = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
        if( pFrm )
            pFrm->Prepare( PREP_QUOVADIS, nullptr, false );
        if( !GetNext() )
        {
            pFrm = FindFootnoteFrm()->GetNext();
            if( pFrm && nullptr != (pFrm=static_cast<SwLayoutFrm*>(pFrm)->ContainsAny()) )
                pFrm->_InvalidatePrt();
        }
    }

    _InvalidateLineNum();
    SwFrm *pNxt = FindNextCnt();
    if ( pNxt  )
    {
        while ( pNxt && pNxt->IsInTab() )
        {
            if( nullptr != (pNxt = pNxt->FindTabFrm()) )
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

void SwContentFrm::Cut()
{
    OSL_ENSURE( GetUpper(), "Cut without Upper()." );

    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    SwFrm *pFrm = GetIndPrev();
    if( pFrm )
    {
        if( pFrm->IsSctFrm() )
            pFrm = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
        if ( pFrm && pFrm->IsContentFrm() )
        {
            pFrm->_InvalidatePrt();
            if( IsInFootnote() )
                pFrm->Prepare( PREP_QUOVADIS, nullptr, false );
        }
        // #i26250# - invalidate printing area of previous
        // table frame.
        else if ( pFrm && pFrm->IsTabFrm() )
        {
            pFrm->InvalidatePrt();
        }
    }

    SwFrm *pNxt = FindNextCnt();
    if ( pNxt )
    {
        while ( pNxt && pNxt->IsInTab() )
        {
            if( nullptr != (pNxt = pNxt->FindTabFrm()) )
                pNxt = pNxt->FindNextCnt();
        }
        if ( pNxt )
        {
            pNxt->_InvalidateLineNum();
            if ( pNxt != GetNext() )
                pNxt->InvalidatePage();
        }
    }

    if( nullptr != (pFrm = GetIndNext()) )
    {
        // The old follow may have calculated a gap to the predecessor which
        // now becomes obsolete or different as it becomes the first one itself
        pFrm->_InvalidatePrt();
        pFrm->_InvalidatePos();
        pFrm->InvalidatePage( pPage );
        if( pFrm->IsSctFrm() )
        {
            pFrm = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
            if( pFrm )
            {
                pFrm->_InvalidatePrt();
                pFrm->_InvalidatePos();
                pFrm->InvalidatePage( pPage );
            }
        }
        if( pFrm && IsInFootnote() )
            pFrm->Prepare( PREP_ERGOSUM, nullptr, false );
        if( IsInSct() && !GetPrev() )
        {
            SwSectionFrm* pSct = FindSctFrm();
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
        if ( nullptr != (pFrm = GetPrev()) )
        {   pFrm->SetRetouche();
            pFrm->Prepare( PREP_WIDOWS_ORPHANS );
            pFrm->_InvalidatePos();
            pFrm->InvalidatePage( pPage );
        }
        // If I'm (was) the only ContentFrm in my upper, it has to do the
        // retouching. Also, perhaps a page became empty.
        else
        {   SwRootFrm *pRoot = getRootFrm();
            if ( pRoot )
            {
                pRoot->SetSuperfluous();
                GetUpper()->SetCompletePaint();
                GetUpper()->InvalidatePage( pPage );
            }
            if( IsInSct() )
            {
                SwSectionFrm* pSct = FindSctFrm();
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
                SwTabFrm* pThisTab = FindTabFrm();
                SwTabFrm* pMasterTab = pThisTab && pThisTab->IsFollow() ? pThisTab->FindMaster() : nullptr;
                if ( pMasterTab )
                {
                    pMasterTab->_InvalidatePos();
                    pMasterTab->SetRemoveFollowFlowLinePending( true );
                }
            }
        }
    }
    //Remove first, then shrink the upper.
    SwLayoutFrm *pUp = GetUpper();
    RemoveFromLayout();
    if ( pUp )
    {
        SwSectionFrm *pSct = nullptr;
        if ( !pUp->Lower() &&
             ( ( pUp->IsFootnoteFrm() && !pUp->IsColLocked() ) ||
               ( pUp->IsInSct() &&
                 // #i29438#
                 // We have to consider the case that the section may be "empty"
                 // except from a temporary empty table frame.
                 // This can happen due to the new cell split feature.
                 !pUp->IsCellFrm() &&
                 // #126020# - adjust check for empty section
                 // #130797# - correct fix #126020#
                 !(pSct = pUp->FindSctFrm())->ContainsContent() &&
                 !pSct->ContainsAny( true ) ) ) )
        {
            if ( pUp->GetUpper() )
            {

                // prevent delete of <ColLocked> footnote frame
                if ( pUp->IsFootnoteFrm() && !pUp->IsColLocked())
                {
                    if( pUp->GetNext() && !pUp->GetPrev() )
                    {
                        SwFrm* pTmp = static_cast<SwLayoutFrm*>(pUp->GetNext())->ContainsAny();
                        if( pTmp )
                            pTmp->_InvalidatePrt();
                    }
                    pUp->Cut();
                    SwFrm::DestroyFrm(pUp);
                }
                else
                {

                    if ( pSct->IsColLocked() || !pSct->IsInFootnote() ||
                         ( pUp->IsFootnoteFrm() && pUp->IsColLocked() ) )
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
                        SwFrm::DestroyFrm(pSct);
                    }
                }
            }
        }
        else
        {
            SWRECTFN( this )
            long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
            if( nFrmHeight )
                pUp->Shrink( nFrmHeight );
        }
    }
}

void SwLayoutFrm::Paste( SwFrm* pParent, SwFrm* pSibling)
{
    OSL_ENSURE( pParent, "No parent for pasting." );
    OSL_ENSURE( pParent->IsLayoutFrm(), "Parent is ContentFrm." );
    OSL_ENSURE( pParent != this, "I'm the parent oneself." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I'm still registered somewhere." );

    //Insert in the tree.
    InsertBefore( static_cast<SwLayoutFrm*>(pParent), pSibling );

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
    if ( IsHeaderFrm() || IsFooterFrm() )
        fnRect = fnRectHori;
    else if ( IsCellFrm() || IsColumnFrm() )
        fnRect = GetUpper()->IsVertical() ? fnRectHori : ( GetUpper()->IsVertLR() ? fnRectVertL2R : fnRectVert );
    else
        fnRect = GetUpper()->IsVertical() ? ( GetUpper()->IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;

    if( (Frm().*fnRect->fnGetWidth)() != (pParent->Prt().*fnRect->fnGetWidth)())
        _InvalidateSize();
    _InvalidatePos();
    const SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    if( !IsColumnFrm() )
    {
        SwFrm *pFrm = GetIndNext();
        if( nullptr != pFrm )
        {
            pFrm->_InvalidatePos();
            if( IsInFootnote() )
            {
                if( pFrm->IsSctFrm() )
                    pFrm = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
                if( pFrm )
                    pFrm->Prepare( PREP_ERGOSUM, nullptr, false );
            }
        }
        if ( IsInFootnote() && nullptr != ( pFrm = GetIndPrev() ) )
        {
            if( pFrm->IsSctFrm() )
                pFrm = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
            if( pFrm )
                pFrm->Prepare( PREP_QUOVADIS, nullptr, false );
        }
    }

    if( (Frm().*fnRect->fnGetHeight)() )
    {
        // AdjustNeighbourhood is now also called in columns which are not
        // placed inside a frame
        sal_uInt8 nAdjust = GetUpper()->IsFootnoteBossFrm() ?
                static_cast<SwFootnoteBossFrm*>(GetUpper())->NeighbourhoodAdjustment( this )
                : NA_GROW_SHRINK;
        SwTwips nGrow = (Frm().*fnRect->fnGetHeight)();
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

void SwLayoutFrm::Cut()
{
    if ( GetNext() )
        GetNext()->_InvalidatePos();

    SWRECTFN( this )
    SwTwips nShrink = (Frm().*fnRect->fnGetHeight)();

    // Remove first, then shrink upper.
    SwLayoutFrm *pUp = GetUpper();

    // AdjustNeighbourhood is now also called in columns which are not
    // placed inside a frame.

    // Remove must not be called before an AdjustNeighbourhood, but it has to
    // be called before the upper-shrink-call, if the upper-shrink takes care
    // of its content.
    if ( pUp && nShrink )
    {
        if( pUp->IsFootnoteBossFrm() )
        {
            sal_uInt8 nAdjust= static_cast<SwFootnoteBossFrm*>(pUp)->NeighbourhoodAdjustment( this );
            if( NA_ONLY_ADJUST == nAdjust )
                AdjustNeighbourhood( -nShrink );
            else
            {
                SwTwips nReal = 0;
                if( NA_ADJUST_GROW == nAdjust )
                    nReal = -AdjustNeighbourhood( -nShrink );
                if( nReal < nShrink )
                {
                    SwTwips nOldHeight = (Frm().*fnRect->fnGetHeight)();
                    (Frm().*fnRect->fnSetHeight)( 0 );
                    nReal += pUp->Shrink( nShrink - nReal );
                    (Frm().*fnRect->fnSetHeight)( nOldHeight );
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

SwTwips SwFrm::Grow( SwTwips nDist, bool bTst, bool bInfo )
{
    OSL_ENSURE( nDist >= 0, "Negative growth?" );

    PROTOCOL_ENTER( this, bTst ? PROT_GROW_TST : PROT_GROW, 0, &nDist )

    if ( nDist )
    {
        SWRECTFN( this )

        SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();
        if( nPrtHeight > 0 && nDist > (LONG_MAX - nPrtHeight) )
            nDist = LONG_MAX - nPrtHeight;

        if ( IsFlyFrm() )
            return static_cast<SwFlyFrm*>(this)->_Grow( nDist, bTst );
        else if( IsSctFrm() )
            return static_cast<SwSectionFrm*>(this)->_Grow( nDist, bTst );
        else
        {
            const SwCellFrm* pThisCell = dynamic_cast<const SwCellFrm*>(this);
            if ( pThisCell )
            {
                const SwTabFrm* pTab = FindTabFrm();

                // NEW TABLES
                if ( pTab->IsVertical() != IsVertical() ||
                     pThisCell->GetLayoutRowSpan() < 1 )
                    return 0;
            }

            const SwTwips nReal = GrowFrm( nDist, bTst, bInfo );
            if( !bTst )
            {
                nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                (Prt().*fnRect->fnSetHeight)( nPrtHeight +
                        ( IsContentFrm() ? nDist : nReal ) );
            }
            return nReal;
        }
    }
    return 0L;
}

SwTwips SwFrm::Shrink( SwTwips nDist, bool bTst, bool bInfo )
{
    OSL_ENSURE( nDist >= 0, "Negative reduction?" );

    PROTOCOL_ENTER( this, bTst ? PROT_SHRINK_TST : PROT_SHRINK, 0, &nDist )

    if ( nDist )
    {
        if ( IsFlyFrm() )
            return static_cast<SwFlyFrm*>(this)->_Shrink( nDist, bTst );
        else if( IsSctFrm() )
            return static_cast<SwSectionFrm*>(this)->_Shrink( nDist, bTst );
        else
        {
            const SwCellFrm* pThisCell = dynamic_cast<const SwCellFrm*>(this);
            if ( pThisCell )
            {
                const SwTabFrm* pTab = FindTabFrm();

                // NEW TABLES
                if ( pTab->IsVertical() != IsVertical() ||
                     pThisCell->GetLayoutRowSpan() < 1 )
                    return 0;
            }

            SWRECTFN( this )
            SwTwips nReal = (Frm().*fnRect->fnGetHeight)();
            ShrinkFrm( nDist, bTst, bInfo );
            nReal -= (Frm().*fnRect->fnGetHeight)();
            if( !bTst )
            {
                const SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                (Prt().*fnRect->fnSetHeight)( nPrtHeight -
                        ( IsContentFrm() ? nDist : nReal ) );
            }
            return nReal;
        }
    }
    return 0L;
}

/** Adjust surrounding neighbourhood after insertion
 *
 * A Frm needs "normalization" if it is directly placed below a footnote boss (page/column) and its
 * size changes. There is always a frame that takes the maximum possible space (the frame that
 * contains the Body text) and zero or more frames which only take the space needed (header/footer
 * area, footnote container). If one of these frames changes, the body-text-frame has to grow or
 * shrink accordingly, even tough it's fixed.
 *
 * !! Is it possible to do this in a generic way and not restrict it to the page and a distinct
 * frame which takes the maximum space (controlled using the FrmSize attribute)?
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
SwTwips SwFrm::AdjustNeighbourhood( SwTwips nDiff, bool bTst )
{
    PROTOCOL_ENTER( this, PROT_ADJUSTN, 0, &nDiff );

    if ( !nDiff || !GetUpper()->IsFootnoteBossFrm() ) // only inside pages/columns
        return 0L;

    const SwViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();

    //The (Page-)Body only changes in BrowseMode, but only if it does not
    //contain columns.
    if ( IsPageBodyFrm() && (!bBrowse ||
          (static_cast<SwLayoutFrm*>(this)->Lower() &&
           static_cast<SwLayoutFrm*>(this)->Lower()->IsColumnFrm())) )
        return 0L;

    //In BrowseView mode the PageFrm can handle some of the requests.
    long nBrowseAdd = 0;
    if ( bBrowse && GetUpper()->IsPageFrm() ) // only (Page-)BodyFrms
    {
        SwViewShell *pViewShell = getRootFrm()->GetCurrShell();
        SwLayoutFrm *pUp = GetUpper();
        long nChg;
        const long nUpPrtBottom = pUp->Frm().Height() -
                                  pUp->Prt().Height() - pUp->Prt().Top();
        SwRect aInva( pUp->Frm() );
        if ( pViewShell )
        {
            aInva.Pos().X() = pViewShell->VisArea().Left();
            aInva.Width( pViewShell->VisArea().Width() );
        }
        if ( nDiff > 0 )
        {
            nChg = BROWSE_HEIGHT - pUp->Frm().Height();
            nChg = std::min( nDiff, nChg );

            if ( !IsBodyFrm() )
            {
                SetCompletePaint();
                if ( !pViewShell || pViewShell->VisArea().Height() >= pUp->Frm().Height() )
                {
                    //First minimize Body, it will grow again later.
                    SwFrm *pBody = static_cast<SwFootnoteBossFrm*>(pUp)->FindBodyCont();
                    const long nTmp = nChg - pBody->Prt().Height();
                    if ( !bTst )
                    {
                        pBody->Frm().Height(std::max( 0L, pBody->Frm().Height() - nChg ));
                        pBody->_InvalidatePrt();
                        pBody->_InvalidateSize();
                        if ( pBody->GetNext() )
                            pBody->GetNext()->_InvalidatePos();
                        if ( !IsHeaderFrm() )
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
                 pUp->Frm().Height() + nDiff < pViewShell->VisArea().Height() )
            {
                // This means that we have to invalidate adequately.
                nChg = pViewShell->VisArea().Height() - pUp->Frm().Height();
                nInvaAdd = -(nDiff - nChg);
            }

            //Invalidate including bottom border.
            long nBorder = nUpPrtBottom + 20;
            nBorder -= nChg;
            aInva.Top( aInva.Bottom() - (nBorder+nInvaAdd) );
            if ( !IsBodyFrm() )
            {
                SetCompletePaint();
                if ( !IsHeaderFrm() )
                    static_cast<SwFootnoteBossFrm*>(pUp)->FindBodyCont()->SetCompletePaint();
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
            pUp->Frm().SSize().Height() += nChg;
            pUp->Prt().SSize().Height() += nChg;
            if ( pViewShell )
                pViewShell->Imp()->SetFirstVisPageInvalid();

            if ( GetNext() )
                GetNext()->_InvalidatePos();

            //Trigger a repaint if necessary.
            SvxBrushItem aBack(pUp->GetFormat()->makeBackgroundBrushItem());
            const SvxGraphicPosition ePos = aBack.GetGraphicPos();
            if ( ePos != GPOS_NONE && ePos != GPOS_TILED )
                pViewShell->InvalidateWindows( pUp->Frm() );

            if ( pUp->GetUpper() )
            {
                if ( pUp->GetNext() )
                    pUp->GetNext()->InvalidatePos();

                //Sad but true: during notify on ViewImp a Calc on the page and
                //its Lower may be called. The values should not be changed
                //because the caller takes care of the adjustment of Frm and
                //Prt.
                const long nOldFrmHeight = Frm().Height();
                const long nOldPrtHeight = Prt().Height();
                const bool bOldComplete = IsCompletePaint();
                if ( IsBodyFrm() )
                    Prt().SSize().Height() = nOldFrmHeight;

                if ( pUp->GetUpper() )
                    static_cast<SwRootFrm*>(pUp->GetUpper())->CheckViewLayout( nullptr, nullptr );
                //static_cast<SwPageFrm*>(pUp)->AdjustRootSize( CHG_CHGPAGE, &aOldRect );

                Frm().SSize().Height() = nOldFrmHeight;
                Prt().SSize().Height() = nOldPrtHeight;
                mbCompletePaint = bOldComplete;
            }
            if ( !IsBodyFrm() )
                pUp->_InvalidateSize();
            InvalidatePage( static_cast<SwPageFrm*>(pUp) );
        }
        nDiff -= nChg;
        if ( !nDiff )
            return nChg;
        else
            nBrowseAdd = nChg;
    }

    const SwFootnoteBossFrm *pBoss = static_cast<SwFootnoteBossFrm*>(GetUpper());

    SwTwips nReal = 0,
            nAdd  = 0;
    SwFrm *pFrm = nullptr;
    SWRECTFN( this )

    if( IsBodyFrm() )
    {
        if( IsInSct() )
        {
            SwSectionFrm *pSect = FindSctFrm();
            if( nDiff > 0 && pSect->IsEndnAtEnd() && GetNext() &&
                GetNext()->IsFootnoteContFrm() )
            {
                SwFootnoteContFrm* pCont = static_cast<SwFootnoteContFrm*>(GetNext());
                SwTwips nMinH = 0;
                SwFootnoteFrm* pFootnote = static_cast<SwFootnoteFrm*>(pCont->Lower());
                bool bFootnote = false;
                while( pFootnote )
                {
                    if( !pFootnote->GetAttr()->GetFootnote().IsEndNote() )
                    {
                        nMinH += (pFootnote->Frm().*fnRect->fnGetHeight)();
                        bFootnote = true;
                    }
                    pFootnote = static_cast<SwFootnoteFrm*>(pFootnote->GetNext());
                }
                if( bFootnote )
                    nMinH += (pCont->Prt().*fnRect->fnGetTop)();
                nReal = (pCont->Frm().*fnRect->fnGetHeight)() - nMinH;
                if( nReal > nDiff )
                    nReal = nDiff;
                if( nReal > 0 )
                    pFrm = GetNext();
                else
                    nReal = 0;
            }
            if( !bTst && !pSect->IsColLocked() )
                pSect->InvalidateSize();
        }
        if( !pFrm )
            return nBrowseAdd;
    }
    else
    {
        const bool bFootnotePage = pBoss->IsPageFrm() && static_cast<const SwPageFrm*>(pBoss)->IsFootnotePage();
        if ( bFootnotePage && !IsFootnoteContFrm() )
            pFrm = const_cast<SwFrm*>(static_cast<SwFrm const *>(pBoss->FindFootnoteCont()));
        if ( !pFrm )
            pFrm = const_cast<SwFrm*>(static_cast<SwFrm const *>(pBoss->FindBodyCont()));

        if ( !pFrm )
            return 0;

        //If not one is found, everything else is solved.
        nReal = (pFrm->Frm().*fnRect->fnGetHeight)();
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
            if( !IsFootnoteContFrm() && nDiff > nReal &&
                pFrm->GetNext() && pFrm->GetNext()->IsFootnoteContFrm()
                && ( pFrm->GetNext()->IsVertical() == IsVertical() )
                )
            {
                //If the Body doesn't return enough, we look for a footnote, if
                //there is one, we steal there accordingly.
                const SwTwips nAddMax = (pFrm->GetNext()->Frm().*fnRect->
                                        fnGetHeight)();
                nAdd = nDiff - nReal;
                if ( nAdd > nAddMax )
                    nAdd = nAddMax;
                if ( !bTst )
                {
                    (pFrm->GetNext()->Frm().*fnRect->fnSetHeight)(nAddMax-nAdd);
                    if( bVert && !bVertL2R && !bRev )
                        pFrm->GetNext()->Frm().Pos().X() += nAdd;
                    pFrm->GetNext()->InvalidatePrt();
                    if ( pFrm->GetNext()->GetNext() )
                        pFrm->GetNext()->GetNext()->_InvalidatePos();
                }
            }
        }
    }

    if ( !bTst && nReal )
    {
        SwTwips nTmp = (pFrm->Frm().*fnRect->fnGetHeight)();
        (pFrm->Frm().*fnRect->fnSetHeight)( nTmp - nReal );
        if( bVert && !bVertL2R && !bRev )
            pFrm->Frm().Pos().X() += nReal;
        pFrm->InvalidatePrt();
        if ( pFrm->GetNext() )
            pFrm->GetNext()->_InvalidatePos();
        if( nReal < 0 && pFrm->IsInSct() )
        {
            SwLayoutFrm* pUp = pFrm->GetUpper();
            if( pUp && nullptr != ( pUp = pUp->GetUpper() ) && pUp->IsSctFrm() &&
                !pUp->IsColLocked() )
                pUp->InvalidateSize();
        }
        if( ( IsHeaderFrm() || IsFooterFrm() ) && pBoss->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pBoss->GetDrawObjs();
            OSL_ENSURE( pBoss->IsPageFrm(), "Header/Footer out of page?" );
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )
                {
                    SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                    OSL_ENSURE( !pFly->IsFlyInCntFrm(), "FlyInCnt at Page?" );
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
                        ((IsHeaderFrm() && rVert.GetVertOrient()!=text::VertOrientation::BOTTOM) ||
                         (IsFooterFrm() && rVert.GetVertOrient()!=text::VertOrientation::NONE &&
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
void SwFrm::_ActionOnInvalidation( const InvalidationType )
{
    // default behaviour is to perform no additional action
}

/** method to determine, if an invalidation is allowed (2004-05-19 #i28701#) */
bool SwFrm::_InvalidationAllowed( const InvalidationType ) const
{
    // default behaviour is to allow invalidation
    return true;
}

void SwFrm::ImplInvalidateSize()
{
    if ( _InvalidationAllowed( INVALID_SIZE ) )
    {
        mbValidSize = false;
        if ( IsFlyFrm() )
            static_cast<SwFlyFrm*>(this)->_Invalidate();
        else
            InvalidatePage();

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_SIZE );
    }
}

void SwFrm::ImplInvalidatePrt()
{
    if ( _InvalidationAllowed( INVALID_PRTAREA ) )
    {
        mbValidPrtArea = false;
        if ( IsFlyFrm() )
            static_cast<SwFlyFrm*>(this)->_Invalidate();
        else
            InvalidatePage();

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_PRTAREA );
    }
}

void SwFrm::ImplInvalidatePos()
{
    if ( _InvalidationAllowed( INVALID_POS ) )
    {
        mbValidPos = false;
        if ( IsFlyFrm() )
        {
            static_cast<SwFlyFrm*>(this)->_Invalidate();
        }
        else
        {
            InvalidatePage();
        }

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_POS );
    }
}

void SwFrm::ImplInvalidateLineNum()
{
    if ( _InvalidationAllowed( INVALID_LINENUM ) )
    {
        mbValidLineNum = false;
        OSL_ENSURE( IsTextFrm(), "line numbers are implemented for text only" );
        InvalidatePage();

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_LINENUM );
    }
}

void SwFrm::ReinitializeFrmSizeAttrFlags()
{
    const SwFormatFrmSize &rFormatSize = GetAttrSet()->GetFrmSize();
    if ( ATT_VAR_SIZE == rFormatSize.GetHeightSizeType() ||
         ATT_MIN_SIZE == rFormatSize.GetHeightSizeType())
    {
        mbFixSize = false;
        if ( GetType() & (FRM_HEADER | FRM_FOOTER | FRM_ROW) )
        {
            SwFrm *pFrm = static_cast<SwLayoutFrm*>(this)->Lower();
            while ( pFrm )
            {   pFrm->_InvalidateSize();
                pFrm->_InvalidatePrt();
                pFrm = pFrm->GetNext();
            }
            SwContentFrm *pCnt = static_cast<SwLayoutFrm*>(this)->ContainsContent();
            // #i36991# - be save.
            // E.g., a row can contain *no* content.
            if ( pCnt )
            {
                pCnt->InvalidatePage();
                do
                {
                    pCnt->Prepare( PREP_ADJUST_FRM );
                    pCnt->_InvalidateSize();
                    pCnt = pCnt->GetNextContentFrm();
                } while ( static_cast<SwLayoutFrm*>(this)->IsAnLower( pCnt ) );
            }
        }
    }
    else if ( rFormatSize.GetHeightSizeType() == ATT_FIX_SIZE )
    {
        if( IsVertical() )
            ChgSize( Size( rFormatSize.GetWidth(), Frm().Height()));
        else
            ChgSize( Size( Frm().Width(), rFormatSize.GetHeight()));
    }
}

void SwFrm::ValidateThisAndAllLowers( const sal_uInt16 nStage )
{
    // Stage 0: Only validate frames. Do not process any objects.
    // Stage 1: Only validate fly frames and all of their contents.
    // Stage 2: Validate all.

    const bool bOnlyObject = 1 == nStage;
    const bool bIncludeObjects = 1 <= nStage;

    if ( !bOnlyObject || dynamic_cast< const SwFlyFrm *>( this ) !=  nullptr )
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
                if ( dynamic_cast< const SwFlyFrm *>( pAnchObj ) !=  nullptr )
                    static_cast<SwFlyFrm*>(pAnchObj)->ValidateThisAndAllLowers( 2 );
                else if ( dynamic_cast< const SwAnchoredDrawObject *>( pAnchObj ) !=  nullptr )
                    static_cast<SwAnchoredDrawObject*>(pAnchObj)->ValidateThis();
            }
        }
    }

    if ( IsLayoutFrm() )
    {
        SwFrm* pLower = static_cast<SwLayoutFrm*>(this)->Lower();
        while ( pLower )
        {
            pLower->ValidateThisAndAllLowers( nStage );
            pLower = pLower->GetNext();
        }
    }
}

SwTwips SwContentFrm::GrowFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    SWRECTFN( this )

    SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
    if( nFrmHeight > 0 &&
         nDist > (LONG_MAX - nFrmHeight ) )
        nDist = LONG_MAX - nFrmHeight;

    const SwViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();
    const sal_uInt16 nTmpType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse with Body
    if( !(GetUpper()->GetType() & nTmpType) && GetUpper()->HasFixSize() )
    {
        if ( !bTst )
        {
            (Frm().*fnRect->fnSetHeight)( nFrmHeight + nDist );
            if( IsVertical() && !IsVertLR() && !IsReverse() )
                Frm().Pos().X() -= nDist;
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
    SwFrm *pFrm = GetUpper()->Lower();
    while( pFrm && nReal > 0 )
    {   nReal -= (pFrm->Frm().*fnRect->fnGetHeight)();
        pFrm = pFrm->GetNext();
    }

    if ( !bTst )
    {
        //Contents are always resized to the wished value.
        long nOld = (Frm().*fnRect->fnGetHeight)();
        (Frm().*fnRect->fnSetHeight)( nOld + nDist );
        if( IsVertical()&& !IsVertLR() && !IsReverse() )
            Frm().Pos().X() -= nDist;
        SwTabFrm *pTab = (nOld && IsInTab()) ? FindTabFrm() : nullptr;
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
            if( bTst || !GetUpper()->IsFooterFrm() )
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

SwTwips SwContentFrm::ShrinkFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    SWRECTFN( this )
    OSL_ENSURE( nDist >= 0, "nDist < 0" );
    OSL_ENSURE( nDist <= (Frm().*fnRect->fnGetHeight)(),
            "nDist > than current size." );

    if ( !bTst )
    {
        SwTwips nRstHeight;
        if( GetUpper() )
            nRstHeight = (Frm().*fnRect->fnBottomDist)
                         ( (GetUpper()->*fnRect->fnGetPrtBottom)() );
        else
            nRstHeight = 0;
        if( nRstHeight < 0 )
        {
            SwTwips nNextHeight = 0;
            if( GetUpper()->IsSctFrm() && nDist > LONG_MAX/2 )
            {
                SwFrm *pNxt = GetNext();
                while( pNxt )
                {
                    nNextHeight += (pNxt->Frm().*fnRect->fnGetHeight)();
                    pNxt = pNxt->GetNext();
                }
            }
            nRstHeight = nDist + nRstHeight - nNextHeight;
        }
        else
            nRstHeight = nDist;
        (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)() - nDist );
        if( IsVertical() && !IsVertLR() )
            Frm().Pos().X() += nDist;
        nDist = nRstHeight;
        SwTabFrm *pTab = IsInTab() ? FindTabFrm() : nullptr;
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
        if( bTst || !GetUpper()->IsFooterFrm() )
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
            const SwRect aRect( Frm() );
            const SwPageFrm* pPage = FindPageFrm();
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
                            const SwFrm* pAnchor = pAnchoredObj->GetAnchorFrm();
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
        //The position of the next Frm changes for sure.
        InvalidateNextPos();

        //If I don't have a successor I have to do the retouch by myself.
        if ( !GetNext() )
            SetRetouche();
    }
    return nReal;
}

void SwContentFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
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
            SwFrm::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        SwPageFrm *pPage = FindPageFrm();
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
                SwSectionFrm *pSect = FindSctFrm();
                if( pSect->ContainsAny() == this )
                {
                    pSect->_InvalidatePrt();
                    pSect->InvalidatePage( pPage );
                }
            }
            _InvalidatePrt();
        }
        SwFrm* mpNextFrm = GetIndNext();
        if ( mpNextFrm && nInvFlags & 0x10)
        {
            mpNextFrm->_InvalidatePrt();
            mpNextFrm->InvalidatePage( pPage );
        }
        if ( mpNextFrm && nInvFlags & 0x80 )
        {
            mpNextFrm->SetCompletePaint();
        }
        if ( nInvFlags & 0x20 )
        {
            SwFrm* pPrevFrm = GetPrev();
            if ( pPrevFrm )
            {
                pPrevFrm->_InvalidatePrt();
                pPrevFrm->InvalidatePage( pPage );
            }
        }
        if ( nInvFlags & 0x40 )
            InvalidateNextPos();
    }
}

void SwContentFrm::_UpdateAttr( const SfxPoolItem* pOld, const SfxPoolItem* pNew,
                              sal_uInt8 &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch ( nWhich )
    {
        case RES_FMT_CHG:
            rInvFlags = 0xFF;
            /* no break here */

        case RES_PAGEDESC:                      //attribute changes (on/off)
            if ( IsInDocBody() && !IsInTab() )
            {
                rInvFlags |= 0x02;
                SwPageFrm *pPage = FindPageFrm();
                if ( !GetPrev() )
                    CheckPageDescs( pPage );
                if ( GetAttrSet()->GetPageDesc().GetNumOffset() )
                    static_cast<SwRootFrm*>(pPage->GetUpper())->SetVirtPageNum( true );
                SwDocPosUpdate aMsgHint( pPage->Frm().Top() );
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
                    SwFrm* pNxt = FindNext();
                    if ( pNxt )
                    {
                        SwPageFrm* pPg = pNxt->FindPageFrm();
                        pNxt->InvalidatePage( pPg );
                        pNxt->_InvalidatePrt();
                        if( pNxt->IsSctFrm() )
                        {
                            SwFrm* pCnt = static_cast<SwSectionFrm*>(pNxt)->ContainsAny();
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
                Prepare( PREP_UL_SPACE );   //TextFrm has to correct line spacing.
                rInvFlags |= 0x80;
                /* no break here */
            }
        case RES_LR_SPACE:
        case RES_BOX:
        case RES_SHADOW:
            Prepare( PREP_FIXSIZE_CHG );
            SwFrm::Modify( pOld, pNew );
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
                    SwFrm* pNxt = FindNext();
                    if( pNxt )
                    {
                        SwPageFrm* pPg = pNxt->FindPageFrm();
                        pNxt->InvalidatePage( pPg );
                        pNxt->_InvalidatePrt();
                        if( pNxt->IsSctFrm() )
                        {
                            SwFrm* pCnt = static_cast<SwSectionFrm*>(pNxt)->ContainsAny();
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
            if ( IsTextFrm() )
            {
                InvalidateNextPrtArea();
            }
            if ( !GetIndNext() && IsInTab() && IsInSplitTableRow() )
            {
                FindTabFrm()->InvalidateSize();
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
            /* no break here */

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
            SwFrm::Modify( pOld, pNew );
    }
}

SwLayoutFrm::SwLayoutFrm(SwFrameFormat *const pFormat, SwFrm *const pSib)
    : SwFrm(pFormat, pSib)
    , m_pLower(nullptr)
{
    const SwFormatFrmSize &rFormatSize = pFormat->GetFrmSize();
    if ( rFormatSize.GetHeightSizeType() == ATT_FIX_SIZE )
        mbFixSize = true;
}

// #i28701#
TYPEINIT1(SwLayoutFrm,SwFrm);

SwTwips SwLayoutFrm::InnerHeight() const
{
    const SwFrm* pCnt = Lower();
    if (!pCnt)
        return 0;

    SWRECTFN( this )
    SwTwips nRet = 0;
    if( pCnt->IsColumnFrm() || pCnt->IsCellFrm() )
    {
        do
        {
            SwTwips nTmp = static_cast<const SwLayoutFrm*>(pCnt)->InnerHeight();
            if( pCnt->GetValidPrtAreaFlag() )
                nTmp += (pCnt->Frm().*fnRect->fnGetHeight)() -
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
            nRet += (pCnt->Frm().*fnRect->fnGetHeight)();
            if( pCnt->IsContentFrm() && static_cast<const SwTextFrm*>(pCnt)->IsUndersized() )
                nRet += static_cast<const SwTextFrm*>(pCnt)->GetParHeight() -
                        (pCnt->Prt().*fnRect->fnGetHeight)();
            if( pCnt->IsLayoutFrm() && !pCnt->IsTabFrm() )
                nRet += static_cast<const SwLayoutFrm*>(pCnt)->InnerHeight() -
                        (pCnt->Prt().*fnRect->fnGetHeight)();
            pCnt = pCnt->GetNext();
        } while( pCnt );

    }
    return nRet;
}

SwTwips SwLayoutFrm::GrowFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    const SwViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();
    const sal_uInt16 nTmpType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse with Body
    if( !(GetType() & nTmpType) && HasFixSize() )
        return 0;

    SWRECTFN( this )
    const SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
    const SwTwips nFrmPos = Frm().Pos().X();

    if ( nFrmHeight > 0 && nDist > (LONG_MAX - nFrmHeight) )
        nDist = LONG_MAX - nFrmHeight;

    SwTwips nMin = 0;
    if ( GetUpper() && !IsCellFrm() )
    {
        SwFrm *pFrm = GetUpper()->Lower();
        while( pFrm )
        {   nMin += (pFrm->Frm().*fnRect->fnGetHeight)();
            pFrm = pFrm->GetNext();
        }
        nMin = (GetUpper()->Prt().*fnRect->fnGetHeight)() - nMin;
        if ( nMin < 0 )
            nMin = 0;
    }

    SwRect aOldFrm( Frm() );
    bool bMoveAccFrm = false;

    bool bChgPos = IsVertical() && !IsReverse();
    if ( !bTst )
    {
        (Frm().*fnRect->fnSetHeight)( nFrmHeight + nDist );
        if( bChgPos && !IsVertLR() )
            Frm().Pos().X() -= nDist;
        bMoveAccFrm = true;
    }

    SwTwips nReal = nDist - nMin;
    if ( nReal > 0 )
    {
        if ( GetUpper() )
        {   // AdjustNeighbourhood now only for the columns (but not in frames)
            sal_uInt8 nAdjust = GetUpper()->IsFootnoteBossFrm() ?
                static_cast<SwFootnoteBossFrm*>(GetUpper())->NeighbourhoodAdjustment( this )
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
                    SwFrm* pToGrow = GetUpper();
                    // NEW TABLES
                    // A cell with a row span of > 1 is allowed to grow the
                    // line containing the end of the row span if it is
                    // located in the same table frame:
                    const SwCellFrm* pThisCell = dynamic_cast<const SwCellFrm*>(this);
                    if ( pThisCell && pThisCell->GetLayoutRowSpan() > 1 )
                    {
                        SwCellFrm& rEndCell = const_cast<SwCellFrm&>(pThisCell->FindStartEndOfRowSpanCell( false, true ));
                        if ( -1 == rEndCell.GetTabBox()->getRowSpan() )
                            pToGrow = rEndCell.GetUpper();
                        else
                            pToGrow = nullptr;
                    }

                    nGrow = pToGrow ? pToGrow->Grow( nReal, bTst, bInfo ) : 0;
                }

                if( NA_GROW_ADJUST == nAdjust && nGrow < nReal )
                    nReal += AdjustNeighbourhood( nReal - nGrow, bTst );

                if ( IsFootnoteFrm() && (nGrow != nReal) && GetNext() )
                {
                    //Footnotes can replace their successor.
                    SwTwips nSpace = bTst ? 0 : -nDist;
                    const SwFrm *pFrm = GetUpper()->Lower();
                    do
                    {   nSpace += (pFrm->Frm().*fnRect->fnGetHeight)();
                        pFrm = pFrm->GetNext();
                    } while ( pFrm != GetNext() );
                    nSpace = (GetUpper()->Prt().*fnRect->fnGetHeight)() -nSpace;
                    if ( nSpace < 0 )
                        nSpace = 0;
                    nSpace += nGrow;
                    if ( nReal > nSpace )
                        nReal = nSpace;
                    if ( nReal && !bTst )
                        static_cast<SwFootnoteFrm*>(this)->InvalidateNxtFootnoteCnts( FindPageFrm() );
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
            ( !IsCellFrm() || static_cast<SwCellFrm*>(this)->GetLayoutRowSpan() > 1 ) )
        {
            (Frm().*fnRect->fnSetHeight)( nFrmHeight + nReal );
            if( bChgPos && !IsVertLR() )
                Frm().Pos().X() = nFrmPos - nReal;
            bMoveAccFrm = true;
        }

        if ( nReal )
        {
            SwPageFrm *pPage = FindPageFrm();
            if ( GetNext() )
            {
                GetNext()->_InvalidatePos();
                if ( GetNext()->IsContentFrm() )
                    GetNext()->InvalidatePage( pPage );
            }
            if ( !IsPageBodyFrm() )
            {
                _InvalidateAll();
                InvalidatePage( pPage );
            }
            if (!(GetType() & (FRM_ROW|FRM_TAB|FRM_FTNCONT|FRM_PAGE|FRM_ROOT)))
                NotifyLowerObjs();

            if( IsCellFrm() )
                InvaPercentLowers( nReal );

            SvxBrushItem aBack(GetFormat()->makeBackgroundBrushItem());
            const SvxGraphicPosition ePos = aBack.GetGraphicPos();
            if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
                SetCompletePaint();
        }
    }

    if( bMoveAccFrm && IsAccessibleFrm() )
    {
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( this, aOldFrm );
        }
    }
    return nReal;
}

SwTwips SwLayoutFrm::ShrinkFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    const SwViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();
    const sal_uInt16 nTmpType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse by Body.
    if( !(GetType() & nTmpType) && HasFixSize() )
        return 0;

    OSL_ENSURE( nDist >= 0, "nDist < 0" );
    SWRECTFN( this )
    SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
    if ( nDist > nFrmHeight )
        nDist = nFrmHeight;

    SwTwips nMin = 0;
    bool bChgPos = IsVertical() && !IsReverse();
    if ( Lower() )
    {
        if( !Lower()->IsNeighbourFrm() )
        {   const SwFrm *pFrm = Lower();
            const long nTmp = (Prt().*fnRect->fnGetHeight)();
            while( pFrm && nMin < nTmp )
            {   nMin += (pFrm->Frm().*fnRect->fnGetHeight)();
                pFrm = pFrm->GetNext();
            }
        }
    }
    SwTwips nReal = nDist;
    SwTwips nMinDiff = (Prt().*fnRect->fnGetHeight)() - nMin;
    if( nReal > nMinDiff )
        nReal = nMinDiff;
    if( nReal <= 0 )
        return nDist;

    SwRect aOldFrm( Frm() );
    bool bMoveAccFrm = false;

    SwTwips nRealDist = nReal;
    if ( !bTst )
    {
        (Frm().*fnRect->fnSetHeight)( nFrmHeight - nReal );
        if( bChgPos && !IsVertLR() )
            Frm().Pos().X() += nReal;
        bMoveAccFrm = true;
    }

    sal_uInt8 nAdjust = GetUpper() && GetUpper()->IsFootnoteBossFrm() ?
                   static_cast<SwFootnoteBossFrm*>(GetUpper())->NeighbourhoodAdjustment( this )
                   : NA_GROW_SHRINK;

    // AdjustNeighbourhood also in columns (but not in frames)
    if( NA_ONLY_ADJUST == nAdjust )
    {
        if ( IsPageBodyFrm() && !bBrowse )
            nReal = nDist;
        else
        {   nReal = AdjustNeighbourhood( -nReal, bTst );
            nReal *= -1;
            if ( !bTst && IsBodyFrm() && nReal < nRealDist )
            {
                (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)()
                                            + nRealDist - nReal );
                if( bChgPos && !IsVertLR() )
                    Frm().Pos().X() += nRealDist - nReal;
                OSL_ENSURE( !IsAccessibleFrm(), "bMoveAccFrm has to be set!" );
            }
        }
    }
    else if( IsColumnFrm() || IsColBodyFrm() )
    {
        SwTwips nTmp = GetUpper()->Shrink( nReal, bTst, bInfo );
        if ( nTmp != nReal )
        {
            (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)()
                                          + nReal - nTmp );
            if( bChgPos && !IsVertLR() )
                Frm().Pos().X() += nTmp - nReal;
            OSL_ENSURE( !IsAccessibleFrm(), "bMoveAccFrm has to be set!" );
            nReal = nTmp;
        }
    }
    else
    {
        SwTwips nShrink = nReal;
        SwFrm* pToShrink = GetUpper();
        const SwCellFrm* pThisCell = dynamic_cast<const SwCellFrm*>(this);
        // NEW TABLES
        if ( pThisCell && pThisCell->GetLayoutRowSpan() > 1 )
        {
            SwCellFrm& rEndCell = const_cast<SwCellFrm&>(pThisCell->FindStartEndOfRowSpanCell( false, true ));
            pToShrink = rEndCell.GetUpper();
        }

        nReal = pToShrink ? pToShrink->Shrink( nShrink, bTst, bInfo ) : 0;
        if( ( NA_GROW_ADJUST == nAdjust || NA_ADJUST_GROW == nAdjust )
            && nReal < nShrink )
            AdjustNeighbourhood( nReal - nShrink );
    }

    if( bMoveAccFrm && IsAccessibleFrm() )
    {
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( this, aOldFrm );
        }
    }
    if ( !bTst && (IsCellFrm() || IsColumnFrm() ? nReal : nRealDist) )
    {
        SwPageFrm *pPage = FindPageFrm();
        if ( GetNext() )
        {
            GetNext()->_InvalidatePos();
            if ( GetNext()->IsContentFrm() )
                GetNext()->InvalidatePage( pPage );
            if ( IsTabFrm() )
                static_cast<SwTabFrm*>(this)->SetComplete();
        }
        else
        {   if ( IsRetoucheFrm() )
                SetRetouche();
            if ( IsTabFrm() )
            {
                if( IsTabFrm() )
                    static_cast<SwTabFrm*>(this)->SetComplete();
                if ( Lower() )  // Can also be in the Join and be empty!
                    InvalidateNextPos();
            }
        }
        if ( !IsBodyFrm() )
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

        if( IsCellFrm() )
            InvaPercentLowers( nReal );

        SwContentFrm *pCnt;
        if( IsFootnoteFrm() && !static_cast<SwFootnoteFrm*>(this)->GetAttr()->GetFootnote().IsEndNote() &&
            ( GetFormat()->GetDoc()->GetFootnoteInfo().ePos != FTNPOS_CHAPTER ||
              ( IsInSct() && FindSctFrm()->IsFootnoteAtEnd() ) ) &&
              nullptr != (pCnt = static_cast<SwFootnoteFrm*>(this)->GetRefFromAttr() ) )
        {
            if ( pCnt->IsFollow() )
            {   // If we are in an other column/page than the frame with the
                // reference, we don't need to invalidate its master.
                SwFrm *pTmp = pCnt->FindFootnoteBossFrm(true) == FindFootnoteBossFrm(true)
                              ?  &pCnt->FindMaster()->GetFrm() : pCnt;
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
 * Changes the size of the directly subsidiary Frm's that have a fixed size, proportionally to the
 * size change of the PrtArea of the Frm's.
 *
 * The variable Frms are also proportionally adapted; they will grow/shrink again by themselves.
 */
void SwLayoutFrm::ChgLowersProp( const Size& rOldSize )
{
    // no change of lower properties for root frame or if no lower exists.
    if ( IsRootFrm() || !Lower() )
        return;

    // declare and init <SwFrm* pLowerFrm> with first lower
    SwFrm *pLowerFrm = Lower();

    // declare and init const booleans <bHeightChgd> and <bWidthChg>
    const bool bHeightChgd = rOldSize.Height() != Prt().Height();
    const bool bWidthChgd  = rOldSize.Width()  != Prt().Width();

    // declare and init variables <bVert>, <bRev> and <fnRect>
    SWRECTFN( this )

    // This shortcut basically tries to handle only lower frames that
    // are affected by the size change. Otherwise much more lower frames
    // are invalidated.
    if ( !( bVert ? bHeightChgd : bWidthChgd ) &&
         ! Lower()->IsColumnFrm() &&
           ( ( IsBodyFrm() && IsInDocBody() && ( !IsInSct() || !FindSctFrm()->IsColLocked() ) ) ||
                // #i10826# Section frames without columns should not
                // invalidate all lowers!
               IsSctFrm() ) )
    {
        // Determine page frame the body frame resp. the section frame belongs to.
        SwPageFrm *pPage = FindPageFrm();
        // Determine last lower by traveling through them using <GetNext()>.
        // During travel check each section frame, if it will be sized to
        // maximum. If Yes, invalidate size of section frame and set
        // corresponding flags at the page.
        do
        {
            if( pLowerFrm->IsSctFrm() && static_cast<SwSectionFrm*>(pLowerFrm)->_ToMaximize() )
            {
                pLowerFrm->_InvalidateSize();
                pLowerFrm->InvalidatePage( pPage );
            }
            if( pLowerFrm->GetNext() )
                pLowerFrm = pLowerFrm->GetNext();
            else
                break;
        } while( true );
        // If found last lower is a section frame containing no section
        // (section frame isn't valid and will be deleted in the future),
        // travel backwards.
        while( pLowerFrm->IsSctFrm() && !static_cast<SwSectionFrm*>(pLowerFrm)->GetSection() &&
               pLowerFrm->GetPrev() )
            pLowerFrm = pLowerFrm->GetPrev();
        // If found last lower is a section frame, set <pLowerFrm> to its last
        // content, if the section frame is valid and is not sized to maximum.
        // Otherwise set <pLowerFrm> to NULL - In this case body frame only
        //      contains invalid section frames.
        if( pLowerFrm->IsSctFrm() )
            pLowerFrm = static_cast<SwSectionFrm*>(pLowerFrm)->GetSection() &&
                   !static_cast<SwSectionFrm*>(pLowerFrm)->ToMaximize( false ) ?
                   static_cast<SwSectionFrm*>(pLowerFrm)->FindLastContent() : nullptr;

        // continue with found last lower, probably the last content of a section
        if ( pLowerFrm )
        {
            // If <pLowerFrm> is in a table frame, set <pLowerFrm> to this table
            // frame and continue.
            if ( pLowerFrm->IsInTab() )
            {
                // OD 28.10.2002 #97265# - safeguard for setting <pLowerFrm> to
                // its table frame - check, if the table frame is also a lower
                // of the body frame, in order to assure that <pLowerFrm> is not
                // set to a frame, which is an *upper* of the body frame.
                SwFrm* pTableFrm = pLowerFrm->FindTabFrm();
                if ( IsAnLower( pTableFrm ) )
                {
                    pLowerFrm = pTableFrm;
                }
            }
            // Check, if variable size of body frame resp. section frame has grown
            // OD 28.10.2002 #97265# - correct check, if variable size has grown.
            SwTwips nOldHeight = bVert ? rOldSize.Width() : rOldSize.Height();
            if( nOldHeight < (Prt().*fnRect->fnGetHeight)() )
            {
                // If variable size of body|section frame has grown, only found
                // last lower and the position of the its next have to be invalidated.
                pLowerFrm->_InvalidateAll();
                pLowerFrm->InvalidatePage( pPage );
                if( !pLowerFrm->IsFlowFrm() ||
                    !SwFlowFrm::CastFlowFrm( pLowerFrm )->HasFollow() )
                    pLowerFrm->InvalidateNextPos( true );
                if ( pLowerFrm->IsTextFrm() )
                    static_cast<SwContentFrm*>(pLowerFrm)->Prepare( PREP_ADJUST_FRM );
            }
            else
            {
                // variable size of body|section frame has shrunk. Thus,
                // invalidate all lowers not matching the new body|section size
                // and the dedicated new last lower.
                if( bVert )
                {
                    SwTwips nBot = Frm().Left() + Prt().Left();
                    while ( pLowerFrm && pLowerFrm->GetPrev() && pLowerFrm->Frm().Left() < nBot )
                    {
                        pLowerFrm->_InvalidateAll();
                        pLowerFrm->InvalidatePage( pPage );
                        pLowerFrm = pLowerFrm->GetPrev();
                    }
                }
                else
                {
                    SwTwips nBot = Frm().Top() + Prt().Bottom();
                    while ( pLowerFrm && pLowerFrm->GetPrev() && pLowerFrm->Frm().Top() > nBot )
                    {
                        pLowerFrm->_InvalidateAll();
                        pLowerFrm->InvalidatePage( pPage );
                        pLowerFrm = pLowerFrm->GetPrev();
                    }
                }
                if ( pLowerFrm )
                {
                    pLowerFrm->_InvalidateSize();
                    pLowerFrm->InvalidatePage( pPage );
                    if ( pLowerFrm->IsTextFrm() )
                        static_cast<SwContentFrm*>(pLowerFrm)->Prepare( PREP_ADJUST_FRM );
                }
            }
            // #i41694# - improvement by removing duplicates
            if ( pLowerFrm )
            {
                if ( pLowerFrm->IsInSct() )
                {
                    // #i41694# - follow-up of issue #i10826#
                    // No invalidation of section frame, if it's the this.
                    SwFrm* pSectFrm = pLowerFrm->FindSctFrm();
                    if( pSectFrm != this && IsAnLower( pSectFrm ) )
                    {
                        pSectFrm->_InvalidateSize();
                        pSectFrm->InvalidatePage( pPage );
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
    if( bVert == pLowerFrm->IsNeighbourFrm() )
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
    while ( pLowerFrm )
    {
        if ( pLowerFrm->IsTextFrm() )
        {
            // Text frames will only be invalidated - prepare invalidation
            if ( bFixChgd )
                static_cast<SwContentFrm*>(pLowerFrm)->Prepare( PREP_FIXSIZE_CHG );
            if ( bVarChgd )
                static_cast<SwContentFrm*>(pLowerFrm)->Prepare( PREP_ADJUST_FRM );
        }
        else
        {
            // If lower isn't a table, row, cell or section frame, adjust its
            // frame size.
            const sal_uInt16 nLowerType = pLowerFrm->GetType();
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
                        pLowerFrm->Frm().Width( Prt().Width() );
                    }
                    else if( rOldSize.Width() && !pLowerFrm->IsFootnoteFrm() )
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
                        if ( (pLowerFrm->Frm().Width() > 50000) ||
                             (Prt().Width() > 50000) )
                        {
                            double nNewWidthTmp =
                                ( double(pLowerFrm->Frm().Width())
                                  * double(Prt().Width()) )
                                / double(rOldSize.Width());
                            nNewWidth = SwTwips(nNewWidthTmp);
                        }
                        else
                        {
                            nNewWidth =
                                (pLowerFrm->Frm().Width() * Prt().Width()) / rOldSize.Width();
                        }
                        pLowerFrm->Frm().Width( nNewWidth );
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
                        pLowerFrm->Frm().Height( Prt().Height() );
                    }
                    // OD 01.10.2002 #102211#
                    // add conditions <!pLowerFrm->IsHeaderFrm()> and
                    // <!pLowerFrm->IsFooterFrm()> in order to avoid that
                    // the <Grow> of header or footer are overwritten.
                    // NOTE: Height of header/footer frame is determined by contents.
                    else if ( rOldSize.Height() &&
                              !pLowerFrm->IsFootnoteFrm() &&
                              !pLowerFrm->IsHeaderFrm() &&
                              !pLowerFrm->IsFooterFrm()
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
                        if ( IsPageFrm() ||
                             ( IsColumnFrm() && IsInSct() )
                           )
                        {
                            OSL_ENSURE( pLowerFrm->IsBodyFrm() || pLowerFrm->IsFootnoteContFrm(),
                                    "ChgLowersProp - only for body or foot note container" );
                            if ( pLowerFrm->IsBodyFrm() || pLowerFrm->IsFootnoteContFrm() )
                            {
                                if ( IsVertical() || pLowerFrm->IsBodyFrm() )
                                {
                                    SwTwips nNewHeight =
                                            pLowerFrm->Frm().Height() +
                                            ( Prt().Height() - rOldSize.Height() );
                                    if ( nNewHeight < 0)
                                    {
                                        // OD 01.04.2003 #108446# - adjust assertion condition and text
                                        OSL_ENSURE( !( IsPageFrm() &&
                                                   (pLowerFrm->Frm().Height()>0) &&
                                                   (pLowerFrm->IsValid()) ),
                                                    "ChgLowersProg - negative height for lower.");
                                        nNewHeight = 0;
                                    }
                                    pLowerFrm->Frm().Height( nNewHeight );
                                }
                            }
                        }
                        else
                        {
                            SwTwips nNewHeight;
                            // OD 24.10.2002 #97265# - <double> calculation
                            // Perform <double> calculation of new height, if
                            // one of the coefficients is greater than 50000
                            if ( (pLowerFrm->Frm().Height() > 50000) ||
                                 (Prt().Height() > 50000) )
                            {
                                double nNewHeightTmp =
                                    ( double(pLowerFrm->Frm().Height())
                                      * double(Prt().Height()) )
                                    / double(rOldSize.Height());
                                nNewHeight = SwTwips(nNewHeightTmp);
                            }
                            else
                            {
                                nNewHeight = ( pLowerFrm->Frm().Height()
                                             * Prt().Height() ) / rOldSize.Height();
                            }
                            if( !pLowerFrm->GetNext() )
                            {
                                SwTwips nSum = Prt().Height();
                                SwFrm* pTmp = Lower();
                                while( pTmp->GetNext() )
                                {
                                    if( !pTmp->IsFootnoteContFrm() || !pTmp->IsVertical() )
                                        nSum -= pTmp->Frm().Height();
                                    pTmp = pTmp->GetNext();
                                }
                                if( nSum - nNewHeight == 1 &&
                                    nSum == pLowerFrm->Frm().Height() )
                                    nNewHeight = nSum;
                            }
                            pLowerFrm->Frm().Height( nNewHeight );
                        }
                    }
                }
            }
        } // end of else { NOT text frame }

        pLowerFrm->_InvalidateAll();
        if ( bInvaPageForContent && pLowerFrm->IsContentFrm() )
        {
            pLowerFrm->InvalidatePage();
            bInvaPageForContent = false;
        }

        if ( !pLowerFrm->GetNext() && pLowerFrm->IsRetoucheFrm() )
        {
            //If a growth took place and the subordinate elements can retouch
            //itself (currently Tabs, Sections and Content) we trigger it.
            if ( rOldSize.Height() < Prt().SSize().Height() ||
                 rOldSize.Width() < Prt().SSize().Width() )
                pLowerFrm->SetRetouche();
        }
        pLowerFrm = pLowerFrm->GetNext();
    }

    // Finally adjust the columns if width is set to auto
    // Possible optimization: execute this code earlier in this function and
    // return???
    if ( ( (bVert && bHeightChgd) || (! bVert && bWidthChgd) ) &&
           Lower()->IsColumnFrm() )
    {
        // get column attribute
        const SwFormatCol* pColAttr = nullptr;
        if ( IsPageBodyFrm() )
        {
            OSL_ENSURE( GetUpper()->IsPageFrm(), "Upper is not page frame" );
            pColAttr = &GetUpper()->GetFormat()->GetCol();
        }
        else
        {
            OSL_ENSURE( IsFlyFrm() || IsSctFrm(), "Columns not in fly or section" );
            pColAttr = &GetFormat()->GetCol();
        }

        if ( pColAttr->IsOrtho() && pColAttr->GetNumCols() > 1 )
            AdjustColumns( pColAttr, false );
    }
}

/** "Formats" the Frame; Frm and PrtArea.
 *
 * The Fixsize is not set here.
 */
void SwLayoutFrm::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "LayoutFrm::Format, pAttrs is 0." );

    if ( mbValidPrtArea && mbValidSize )
        return;

    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool hideWS = (pSh && pSh->GetViewOptions()->IsWhitespaceHidden());
    const long hideWSBorderSize = (pSh ? pSh->GetViewOptions()->GetDocumentBorder() : 0);
    const sal_uInt16 nLeft = (sal_uInt16)pAttrs->CalcLeft(this);
    const sal_uInt16 nUpper = hideWS ? hideWSBorderSize : pAttrs->CalcTop();

    const sal_uInt16 nRight = (sal_uInt16)pAttrs->CalcRight(this);
    const sal_uInt16 nLower = hideWS ? hideWSBorderSize : pAttrs->CalcBottom();

    bool bVert = IsVertical() && !IsPageFrm();
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
            const SwFormatFrmSize &rSz = GetFormat()->GetFrmSize();
            SwTwips nMinHeight = rSz.GetHeightSizeType() == ATT_MIN_SIZE ? rSz.GetHeight() : 0;
            do
            {   mbValidSize = true;

                //The size in VarSize is calculated using the content plus the
                // borders.
                SwTwips nRemaining = 0;
                SwFrm *pFrm = Lower();
                while ( pFrm )
                {   nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
                    if( pFrm->IsTextFrm() && static_cast<SwTextFrm*>(pFrm)->IsUndersized() )
                    // This TextFrm would like to be a bit bigger
                        nRemaining += static_cast<SwTextFrm*>(pFrm)->GetParHeight()
                                      - (pFrm->Prt().*fnRect->fnGetHeight)();
                    else if( pFrm->IsSctFrm() && static_cast<SwSectionFrm*>(pFrm)->IsUndersized() )
                        nRemaining += static_cast<SwSectionFrm*>(pFrm)->Undersize();
                    pFrm = pFrm->GetNext();
                }
                nRemaining += nBorder;
                nRemaining = std::max( nRemaining, nMinHeight );
                const SwTwips nDiff = nRemaining-(Frm().*fnRect->fnGetHeight)();
                const long nOldLeft = (Frm().*fnRect->fnGetLeft)();
                const long nOldTop = (Frm().*fnRect->fnGetTop)();
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
                if ( GetUpper() && (Frm().*fnRect->fnGetHeight)() )
                {
                    const SwTwips nLimit = (GetUpper()->*fnRect->fnGetPrtBottom)();
                    if( (this->*fnRect->fnSetLimit)( nLimit ) &&
                        nOldLeft == (Frm().*fnRect->fnGetLeft)() &&
                        nOldTop  == (Frm().*fnRect->fnGetTop)() )
                        mbValidSize = mbValidPrtArea = true;
                }
            } while ( !mbValidSize );
        }
        else if (GetType() & FRM_HEADFOOT)
        {
            do
            {   if ( Frm().Height() != pAttrs->GetSize().Height() )
                    ChgSize( Size( Frm().Width(), pAttrs->GetSize().Height()));
                mbValidSize = true;
                MakePos();
            } while ( !mbValidSize );
        }
        else if (hideWS)
        {
            const auto newHeight = InnerHeight() + nUpper + nLower;
            ChgSize(Size(Frm().Width(), newHeight));
            mbValidSize = true;
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

static void InvaPercentFlys( SwFrm *pFrm, SwTwips nDiff )
{
    OSL_ENSURE( pFrm->GetDrawObjs(), "Can't find any Objects" );
    for ( size_t i = 0; i < pFrm->GetDrawObjs()->size(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
        if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )
        {
            SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
            const SwFormatFrmSize &rSz = pFly->GetFormat()->GetFrmSize();
            if ( rSz.GetWidthPercent() || rSz.GetHeightPercent() )
            {
                bool bNotify = true;
                // If we've a fly with more than 90% relative height...
                if( rSz.GetHeightPercent() > 90 && pFly->GetAnchorFrm() &&
                    rSz.GetHeightPercent() != SwFormatFrmSize::SYNCED && nDiff )
                {
                    const SwFrm *pRel = pFly->IsFlyLayFrm() ? pFly->GetAnchorFrm():
                                        pFly->GetAnchorFrm()->GetUpper();
                    // ... and we have already more than 90% height and we
                    // not allow the text to go through...
                    // then a notifycation could cause an endless loop, e.g.
                    // 100% height and no text wrap inside a cell of a table.
                    if( pFly->Frm().Height()*10 >
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

void SwLayoutFrm::InvaPercentLowers( SwTwips nDiff )
{
    if ( GetDrawObjs() )
        ::InvaPercentFlys( this, nDiff );

    SwFrm *pFrm = ContainsContent();
    if ( pFrm )
        do
        {
            if ( pFrm->IsInTab() && !IsTabFrm() )
            {
                SwFrm *pTmp = pFrm->FindTabFrm();
                OSL_ENSURE( pTmp, "Where's my TabFrm?" );
                if( IsAnLower( pTmp ) )
                    pFrm = pTmp;
            }

            if ( pFrm->IsTabFrm() )
            {
                const SwFormatFrmSize &rSz = static_cast<SwLayoutFrm*>(pFrm)->GetFormat()->GetFrmSize();
                if ( rSz.GetWidthPercent() || rSz.GetHeightPercent() )
                    pFrm->InvalidatePrt();
            }
            else if ( pFrm->GetDrawObjs() )
                ::InvaPercentFlys( pFrm, nDiff );
            pFrm = pFrm->FindNextCnt();
        } while ( pFrm && IsAnLower( pFrm ) ) ;
}

long SwLayoutFrm::CalcRel( const SwFormatFrmSize &rSz, bool ) const
{
    long nRet     = rSz.GetWidth(),
         nPercent = rSz.GetWidthPercent();

    if ( nPercent )
    {
        const SwFrm *pRel = GetUpper();
        long nRel = LONG_MAX;
        const SwViewShell *pSh = getRootFrm()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        if( pRel->IsPageBodyFrm() && pSh && bBrowseMode && pSh->VisArea().Width() )
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

// Local helpers for SwLayoutFrm::FormatWidthCols()

static long lcl_CalcMinColDiff( SwLayoutFrm *pLayFrm )
{
    long nDiff = 0, nFirstDiff = 0;
    SwLayoutFrm *pCol = static_cast<SwLayoutFrm*>(pLayFrm->Lower());
    OSL_ENSURE( pCol, "Where's the columnframe?" );
    SwFrm *pFrm = pCol->Lower();
    do
    {
        if( pFrm && pFrm->IsBodyFrm() )
            pFrm = static_cast<SwBodyFrm*>(pFrm)->Lower();
        if ( pFrm && pFrm->IsTextFrm() )
        {
            const long nTmp = static_cast<SwTextFrm*>(pFrm)->FirstLineHeight();
            if ( nTmp != USHRT_MAX )
            {
                if ( pCol == pLayFrm->Lower() )
                    nFirstDiff = nTmp;
                else
                    nDiff = nDiff ? std::min( nDiff, nTmp ) : nTmp;
            }
        }
        //Skip empty columns!
        pCol = static_cast<SwLayoutFrm*>(pCol->GetNext());
        while ( pCol && nullptr == (pFrm = pCol->Lower()) )
            pCol = static_cast<SwLayoutFrm*>(pCol->GetNext());

    } while ( pFrm && pCol );

    return nDiff ? nDiff : nFirstDiff ? nFirstDiff : 240;
}

static bool lcl_IsFlyHeightClipped( SwLayoutFrm *pLay )
{
    SwFrm *pFrm = pLay->ContainsContent();
    while ( pFrm )
    {
        if ( pFrm->IsInTab() )
            pFrm = pFrm->FindTabFrm();

        if ( pFrm->GetDrawObjs() )
        {
            const size_t nCnt = pFrm->GetDrawObjs()->size();
            for ( size_t i = 0; i < nCnt; ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
                if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )
                {
                    SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                    if ( pFly->IsHeightClipped() &&
                         ( !pFly->IsFlyFreeFrm() || pFly->GetPageFrm() ) )
                        return true;
                }
            }
        }
        pFrm = pFrm->FindNextCnt();
    }
    return false;
}

void SwLayoutFrm::FormatWidthCols( const SwBorderAttrs &rAttrs,
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
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
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
        if( IsSctFrm() )
        {
            nMaximum = (Frm().*fnRect->fnGetHeight)() - nBorder +
                       (Frm().*fnRect->fnBottomDist)(
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

            bNoBalance = static_cast<SwSectionFrm*>(this)->GetSection()->GetFormat()->
                         GetBalancedColumns().GetValue();
            SwFrm* pAny = ContainsAny();
            if( bNoBalance ||
                ( !(Frm().*fnRect->fnGetHeight)() && pAny ) )
            {
                long nTop = (this->*fnRect->fnGetTopMargin)();
                // #i23129# - correction
                // to the calculated maximum height.
                (Frm().*fnRect->fnAddBottom)( nMaximum -
                                              (Frm().*fnRect->fnGetHeight)() );
                if( nTop > nMaximum )
                    nTop = nMaximum;
                (this->*fnRect->fnSetYMargins)( nTop, 0 );
            }
            if( !pAny && !static_cast<SwSectionFrm*>(this)->IsFootnoteLock() )
            {
                SwFootnoteContFrm* pFootnoteCont = static_cast<SwSectionFrm*>(this)->ContainsFootnoteCont();
                if( pFootnoteCont )
                {
                    SwFrm* pFootnoteAny = pFootnoteCont->ContainsAny();
                    if( pFootnoteAny && pFootnoteAny->IsValid() )
                    {
                        bBackLock = true;
                        static_cast<SwSectionFrm*>(this)->SetFootnoteLock( true );
                    }
                }
            }
        }
        else
            nMaximum = LONG_MAX;

        // #i3317# - reset temporarly consideration
        // of wrapping style influence
        SwPageFrm* pPageFrm = FindPageFrm();
        SwSortedObjs* pObjs = pPageFrm ? pPageFrm->GetSortedObjs() : nullptr;
        if ( pObjs )
        {
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];

                if ( IsAnLower( pAnchoredObj->GetAnchorFrm() ) )
                {
                    pAnchoredObj->SetTmpConsiderWrapInfluence( false );
                }
            }
        }
        do
        {
            //Could take a while therefore check for Waitcrsr here.
            if ( pImp )
                pImp->CheckWaitCrsr();

            mbValidSize = true;
            //First format the column as this will relieve the stack a bit.
            //Also set width and height of the column (if they are wrong)
            //while we are at it.
            SwLayoutFrm *pCol = static_cast<SwLayoutFrm*>(Lower());

            // #i27399#
            // Simply setting the column width based on the values returned by
            // CalcColWidth does not work for automatic column width.
            AdjustColumns( &rCol, false );

            for ( sal_uInt16 i = 0; i < nNumCols; ++i )
            {
                pCol->Calc(pRenderContext);
                // ColumnFrms have a BodyFrm now, which needs to be calculated
                pCol->Lower()->Calc(pRenderContext);
                if( pCol->Lower()->GetNext() )
                    pCol->Lower()->GetNext()->Calc(pRenderContext);  // SwFootnoteCont
                pCol = static_cast<SwLayoutFrm*>(pCol->GetNext());
            }

            ::CalcContent( this );

            pCol = static_cast<SwLayoutFrm*>(Lower());
            OSL_ENSURE( pCol && pCol->GetNext(), ":-( column making holidays?");
            // set bMinDiff if no empty columns exist
            bool bMinDiff = true;
            // OD 28.03.2003 #108446# - check for all column content and all columns
            while ( bMinDiff && pCol )
            {
                bMinDiff = nullptr != pCol->ContainsContent();
                pCol = static_cast<SwLayoutFrm*>(pCol->GetNext());
            }
            pCol = static_cast<SwLayoutFrm*>(Lower());
            // OD 28.03.2003 #108446# - initialize local variable
            SwTwips nDiff = 0;
            SwTwips nMaxFree = 0;
            SwTwips nAllFree = LONG_MAX;
            // set bFoundLower if there is at least one non-empty column
            bool bFoundLower = false;
            while( pCol )
            {
                SwLayoutFrm* pLay = static_cast<SwLayoutFrm*>(pCol->Lower());
                SwTwips nInnerHeight = (pLay->Frm().*fnRect->fnGetHeight)() -
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
                    pLay = static_cast<SwLayoutFrm*>(pLay->GetNext());
                    OSL_ENSURE( pLay->IsFootnoteContFrm(),"FootnoteContainer expected" );
                    nInnerHeight += pLay->InnerHeight();
                    nInnerHeight += (pLay->Frm().*fnRect->fnGetHeight)() -
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
                pCol = static_cast<SwLayoutFrm*>(pCol->GetNext());
            }

            if ( bFoundLower || ( IsSctFrm() && static_cast<SwSectionFrm*>(this)->HasFollow() ) )
            {
                SwTwips nMinDiff = ::lcl_CalcMinColDiff( this );
                // Here we decide if growing is needed - this is the case, if
                // column content (nDiff) or a Fly juts over.
                // In sections with columns we take into account to set the size
                // when having a non-empty Follow.
                if ( nDiff || ::lcl_IsFlyHeightClipped( this ) ||
                     ( IsSctFrm() && static_cast<SwSectionFrm*>(this)->CalcMinDiff( nMinDiff ) ) )
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
                        // minimal FrmHeight and PrtHeight is smaller than
                        // nMindiff we grow in a way that PrtHeight is exactly
                        // nMinDiff afterwards.
                        long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
                        if ( nFrmHeight > nMinHeight || nPrtHeight >= nMinDiff )
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
                            (Frm().*fnRect->fnGetHeight)();
                    (Frm().*fnRect->fnAddBottom)( nDiff );
                    // #i68520#
                    SwFlyFrm *pFlyFrm = dynamic_cast<SwFlyFrm*>(this);
                    if (pFlyFrm)
                    {
                        pFlyFrm->InvalidateObjRectWithSpaces();
                    }
                    (this->*fnRect->fnSetYMargins)( nTop, nBorder - nTop );
                    ChgLowersProp( aOldSz );
                    NotifyLowerObjs();

                    // #i3317# - reset temporarly consideration
                    // of wrapping style influence
                    SwPageFrm* pTmpPageFrm = FindPageFrm();
                    SwSortedObjs* pTmpObjs = pTmpPageFrm ? pTmpPageFrm->GetSortedObjs() : nullptr;
                    if ( pTmpObjs )
                    {
                        for ( size_t i = 0; i < pTmpObjs->size(); ++i )
                        {
                            SwAnchoredObject* pAnchoredObj = (*pTmpObjs)[i];

                            if ( IsAnLower( pAnchoredObj->GetAnchorFrm() ) )
                            {
                                pAnchoredObj->SetTmpConsiderWrapInfluence( false );
                            }
                        }
                    }
                    //Invalidate suitable to nicely balance the Frms.
                    //- Every first one after the second column gets a
                    //  InvalidatePos();
                    pCol = static_cast<SwLayoutFrm*>(Lower()->GetNext());
                    while ( pCol )
                    {
                        SwFrm *pLow = pCol->Lower();
                        if ( pLow )
                            pLow->_InvalidatePos();
                        pCol = static_cast<SwLayoutFrm*>(pCol->GetNext());
                    }
                    if( IsSctFrm() && static_cast<SwSectionFrm*>(this)->HasFollow() )
                    {
                        // If we created a Follow, we need to give its content
                        // the opportunity to flow back inside the CalcContent
                        SwContentFrm* pTmpContent =
                            static_cast<SwSectionFrm*>(this)->GetFollow()->ContainsContent();
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
    if( IsSctFrm() )
    {
        // OD 14.03.2003 #i11760# - adjust 2nd parameter - sal_True --> true
        ::CalcContent( this, true );
        if( bBackLock )
            static_cast<SwSectionFrm*>(this)->SetFootnoteLock( false );
    }
}

static SwContentFrm* lcl_InvalidateSection( SwFrm *pCnt, sal_uInt8 nInv )
{
    SwSectionFrm* pSect = pCnt->FindSctFrm();
    // If our ContentFrm is placed inside a table or a footnote, only sections
    // which are also placed inside are meant.
    // Exception: If a table is directly passed.
    if( ( ( pCnt->IsInTab() && !pSect->IsInTab() ) ||
        ( pCnt->IsInFootnote() && !pSect->IsInFootnote() ) ) && !pCnt->IsTabFrm() )
        return nullptr;
    if( nInv & INV_SIZE )
        pSect->_InvalidateSize();
    if( nInv & INV_POS )
        pSect->_InvalidatePos();
    if( nInv & INV_PRTAREA )
        pSect->_InvalidatePrt();
    SwFlowFrm *pFoll = pSect->GetFollow();
    // Temporary separation from follow
    pSect->SetFollow( nullptr );
    SwContentFrm* pRet = pSect->FindLastContent();
    pSect->SetFollow( pFoll );
    return pRet;
}

static SwContentFrm* lcl_InvalidateTable( SwTabFrm *pTable, sal_uInt8 nInv )
{
    if( ( nInv & INV_SECTION ) && pTable->IsInSct() )
        lcl_InvalidateSection( pTable, nInv );
    if( nInv & INV_SIZE )
        pTable->_InvalidateSize();
    if( nInv & INV_POS )
        pTable->_InvalidatePos();
    if( nInv & INV_PRTAREA )
        pTable->_InvalidatePrt();
    return pTable->FindLastContent();
}

static void lcl_InvalidateAllContent( SwContentFrm *pCnt, sal_uInt8 nInv );

static void lcl_InvalidateContent( SwContentFrm *pCnt, sal_uInt8 nInv )
{
    SwContentFrm *pLastTabCnt = nullptr;
    SwContentFrm *pLastSctCnt = nullptr;
    while ( pCnt )
    {
        if( nInv & INV_SECTION )
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
        if( nInv & INV_TABLE )
        {
            if( pCnt->IsInTab() )
            {
                // To not call FindTabFrm() for each ContentFrm of a table and
                // then invalidate the table, we remember the last ContentFrm of
                // the table and ignore IsInTab() until we are past it.
                // When entering the table, LastSctCnt is set to null, so
                // sections inside the table are correctly invalidated.
                // If the table itself is in a section the
                // invalidation is done three times, which is acceptable.
                if( !pLastTabCnt )
                {
                    pLastTabCnt = lcl_InvalidateTable( pCnt->FindTabFrm(), nInv );
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

        if( nInv & INV_SIZE )
            pCnt->Prepare( PREP_CLEAR, nullptr, false );
        if( nInv & INV_POS )
            pCnt->_InvalidatePos();
        if( nInv & INV_PRTAREA )
            pCnt->_InvalidatePrt();
        if ( nInv & INV_LINENUM )
            pCnt->InvalidateLineNum();
        if ( pCnt->GetDrawObjs() )
            lcl_InvalidateAllContent( pCnt, nInv );
        pCnt = pCnt->GetNextContentFrm();
    }
}

static void lcl_InvalidateAllContent( SwContentFrm *pCnt, sal_uInt8 nInv )
{
    SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
    for ( size_t i = 0; i < rObjs.size(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = rObjs[i];
        if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )
        {
            SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
            if ( pFly->IsFlyInCntFrm() )
            {
                ::lcl_InvalidateContent( pFly->ContainsContent(), nInv );
                if( nInv & INV_DIRECTION )
                    pFly->CheckDirChange();
            }
        }
    }
}

void SwRootFrm::InvalidateAllContent( sal_uInt8 nInv )
{
    // First process all page bound FlyFrms.
    SwPageFrm *pPage = static_cast<SwPageFrm*>(Lower());
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
                if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )
                {
                    SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                    ::lcl_InvalidateContent( pFly->ContainsContent(), nInv );
                    if ( nInv & INV_DIRECTION )
                        pFly->CheckDirChange();
                }
            }
        }
        if( nInv & INV_DIRECTION )
            pPage->CheckDirChange();
        pPage = static_cast<SwPageFrm*>(pPage->GetNext());
    }

    //Invalidate the whole document content and the character bound Flys here.
    ::lcl_InvalidateContent( ContainsContent(), nInv );

    if( nInv & INV_PRTAREA )
    {
        SwViewShell *pSh  = getRootFrm()->GetCurrShell();
        if( pSh )
            pSh->InvalidateWindows( Frm() );
    }
}

/**
 * Invalidate/re-calculate the position of all floating screen objects (Writer fly frames and
 * drawing objects), that are anchored to paragraph or to character. (2004-03-16 #i11860#)
 */
void SwRootFrm::InvalidateAllObjPos()
{
    const SwPageFrm* pPageFrm = static_cast<const SwPageFrm*>(Lower());
    while( pPageFrm )
    {
        pPageFrm->InvalidateFlyLayout();

        if ( pPageFrm->GetSortedObjs() )
        {
            const SwSortedObjs& rObjs = *(pPageFrm->GetSortedObjs());
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

        pPageFrm = static_cast<const SwPageFrm*>(pPageFrm->GetNext());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
