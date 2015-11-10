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

#include "cmdid.h"
#include "hintids.hxx"

#include <svl/stritem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/shaditem.hxx>
#include <svx/swframevalidation.hxx>
#include <svx/xdef.hxx>
#include <fmtclds.hxx>
#include "wrtsh.hxx"
#include "view.hxx"
#include "viewopt.hxx"
#include "uitool.hxx"
#include "frmmgr.hxx"
#include "format.hxx"
#include "mdiexp.hxx"
#include "poolfmt.hxx"
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

using namespace ::com::sun::star;

static sal_uInt16 aFrmMgrRange[] = {
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1,

                            //UUUU FillAttribute support
                            XATTR_FILL_FIRST, XATTR_FILL_LAST,

                            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                            FN_SET_FRM_NAME, FN_SET_FRM_NAME,
                            0};

// determine frame attributes via Shell
SwFlyFrmAttrMgr::SwFlyFrmAttrMgr( bool bNew, SwWrtShell* pSh, sal_uInt8 nType ) :
    m_aSet( static_cast<SwAttrPool&>(pSh->GetAttrPool()), aFrmMgrRange ),
    m_pOwnSh( pSh ),
    m_bAbsPos( false ),
    m_bNewFrm( bNew ),
    m_bIsInVertical( false ),
    m_bIsInVerticalL2R( false )
{
    if ( m_bNewFrm )
    {
        // set defaults:
        sal_uInt16 nId = 0;
        switch ( nType )
        {
            case FRMMGR_TYPE_TEXT:  nId = RES_POOLFRM_FRAME;    break;
            case FRMMGR_TYPE_OLE:   nId = RES_POOLFRM_OLE;      break;
            case FRMMGR_TYPE_GRF:   nId = RES_POOLFRM_GRAPHIC;  break;
        }
        m_aSet.SetParent( &m_pOwnSh->GetFormatFromPool( nId )->GetAttrSet());
        m_aSet.Put( SwFormatFrmSize( ATT_MIN_SIZE, DFLT_WIDTH, DFLT_HEIGHT ));
        if ( 0 != ::GetHtmlMode(pSh->GetView().GetDocShell()) )
            m_aSet.Put( SwFormatHoriOrient( 0, text::HoriOrientation::LEFT, text::RelOrientation::PRINT_AREA ) );
    }
    else if ( nType == FRMMGR_TYPE_NONE )
    {
        m_pOwnSh->GetFlyFrmAttr( m_aSet );
        bool bRightToLeft;
        m_bIsInVertical = m_pOwnSh->IsFrmVertical(true, bRightToLeft, m_bIsInVerticalL2R);
    }
    ::PrepareBoxInfo( m_aSet, *m_pOwnSh );
}

SwFlyFrmAttrMgr::SwFlyFrmAttrMgr( bool bNew, SwWrtShell* pSh, const SfxItemSet &rSet ) :
    m_aSet( rSet ),
    m_pOwnSh( pSh ),
    m_bAbsPos( false ),
    m_bNewFrm( bNew ),
    m_bIsInVertical(false),
    m_bIsInVerticalL2R(false)
{
    if(!bNew)
    {
        bool bRightToLeft;
        m_bIsInVertical = pSh->IsFrmVertical(true, bRightToLeft, m_bIsInVerticalL2R);
    }
}

// Initialise
void SwFlyFrmAttrMgr::UpdateAttrMgr()
{
    if ( !m_bNewFrm && m_pOwnSh->IsFrmSelected() )
        m_pOwnSh->GetFlyFrmAttr( m_aSet );
    ::PrepareBoxInfo( m_aSet, *m_pOwnSh );
}

void SwFlyFrmAttrMgr::_UpdateFlyFrm()
{
    const SfxPoolItem* pItem = nullptr;

    if (m_aSet.GetItemState(FN_SET_FRM_NAME, false, &pItem) == SfxItemState::SET)
        m_pOwnSh->SetFlyName(static_cast<const SfxStringItem *>(pItem)->GetValue());

    m_pOwnSh->SetModified();

    if ( m_bAbsPos )
    {
        m_pOwnSh->SetFlyPos( m_aAbsPos );
        m_bAbsPos = false;
    }
}

// change existing Fly-Frame
void SwFlyFrmAttrMgr::UpdateFlyFrm()
{
    OSL_ENSURE( m_pOwnSh->IsFrmSelected(),
        "no frame selected or no shell, update not possible");

    if( m_pOwnSh->IsFrmSelected() )
    {
        //JP 6.8.2001: set never an invalid anchor into the core.
        const SfxPoolItem *pGItem, *pItem;
        if( SfxItemState::SET == m_aSet.GetItemState( RES_ANCHOR, false, &pItem ))
        {
            SfxItemSet aGetSet( *m_aSet.GetPool(), RES_ANCHOR, RES_ANCHOR );
            if( m_pOwnSh->GetFlyFrmAttr( aGetSet ) && 1 == aGetSet.Count() &&
                SfxItemState::SET == aGetSet.GetItemState( RES_ANCHOR, false, &pGItem )
                && static_cast<const SwFormatAnchor*>(pGItem)->GetAnchorId() ==
                   static_cast<const SwFormatAnchor*>(pItem)->GetAnchorId() )
                m_aSet.ClearItem( RES_ANCHOR );
        }

        // return wg. BASIC
        if( m_aSet.Count() )
        {
            m_pOwnSh->StartAllAction();
            m_pOwnSh->SetFlyFrmAttr( m_aSet );
            _UpdateFlyFrm();
            m_pOwnSh->EndAllAction();
        }
    }
}

// insert frame
bool SwFlyFrmAttrMgr::InsertFlyFrm()
{
    m_pOwnSh->StartAllAction();

    bool bRet = nullptr != m_pOwnSh->NewFlyFrm( m_aSet );

    // turn on the right mode at the shell, frame got selected automatically.
    if ( bRet )
    {
        _UpdateFlyFrm();
        m_pOwnSh->EnterSelFrmMode();
        FrameNotify(m_pOwnSh, FLY_DRAG_START);
    }
    m_pOwnSh->EndAllAction();
    return bRet;
}

// Insert frames of type eAnchorType. Position and size are being set explicitly.
// Not-allowed values of the enumeration type get corrected.
void SwFlyFrmAttrMgr::InsertFlyFrm(RndStdIds    eAnchorType,
                                   const Point  &rPos,
                                   const Size   &rSize,
                                   bool bAbs )
{
    OSL_ENSURE( eAnchorType == FLY_AT_PAGE ||
            eAnchorType == FLY_AT_PARA ||
            eAnchorType == FLY_AT_CHAR ||
            eAnchorType == FLY_AT_FLY  ||
            eAnchorType == FLY_AS_CHAR,     "invalid frame type" );

    if ( bAbs )
        SetAbsPos( rPos );
    else
        SetPos( rPos );

    SetSize( rSize );
    SetAnchor( eAnchorType );
    InsertFlyFrm();
}

// set anchor
void SwFlyFrmAttrMgr::SetAnchor( RndStdIds eId )
{
    sal_uInt16 nPhyPageNum, nVirtPageNum;
    m_pOwnSh->GetPageNum( nPhyPageNum, nVirtPageNum );

    m_aSet.Put( SwFormatAnchor( eId, nPhyPageNum ) );
    if ((FLY_AT_PAGE == eId) || (FLY_AT_PARA == eId) || (FLY_AT_CHAR == eId)
        || (FLY_AT_FLY == eId))
    {
        SwFormatVertOrient aVertOrient( GetVertOrient() );
        SwFormatHoriOrient aHoriOrient( GetHoriOrient() );
        aHoriOrient.SetRelationOrient( text::RelOrientation::FRAME );
        aVertOrient.SetRelationOrient( text::RelOrientation::FRAME );
        m_aSet.Put( aVertOrient );
        m_aSet.Put( aHoriOrient );
    }
}

// set the attribute for columns
void SwFlyFrmAttrMgr::SetCol( const SwFormatCol &rCol )
{
    m_aSet.Put( rCol );
}

//  set absolute position
void SwFlyFrmAttrMgr::SetAbsPos( const Point& rPoint )
{
    m_bAbsPos = true;
    m_aAbsPos = rPoint;
    SwFormatVertOrient aVertOrient( GetVertOrient() );
    SwFormatHoriOrient aHoriOrient( GetHoriOrient() );
    aHoriOrient.SetHoriOrient( text::HoriOrientation::NONE );
    aVertOrient.SetVertOrient( text::VertOrientation::NONE );
    m_aSet.Put( aVertOrient );
    m_aSet.Put( aHoriOrient );
}

// check metrics for correctness
void SwFlyFrmAttrMgr::ValidateMetrics( SvxSwFrameValidation& rVal,
        const SwPosition* pToCharContentPos,
        bool bOnlyPercentRefValue )
{
    if (!bOnlyPercentRefValue)
    {
        rVal.nMinHeight = MINFLY + CalcTopSpace() + CalcBottomSpace();
        rVal.nMinWidth =  MINFLY + CalcLeftSpace()+ CalcRightSpace();
    }

    SwRect aBoundRect;

    // OD 18.09.2003 #i18732# - adjustment for allowing vertical position
    //      aligned to page for fly frame anchored to paragraph or to character.
    const RndStdIds eAnchorType = static_cast<RndStdIds >(rVal.nAnchorType);
    const SwFormatFrmSize& rSize = static_cast<const SwFormatFrmSize&>(m_aSet.Get(RES_FRM_SIZE));
    m_pOwnSh->CalcBoundRect( aBoundRect, eAnchorType,
                           rVal.nHRelOrient,
                           rVal.nVRelOrient,
                           pToCharContentPos,
                           rVal.bFollowTextFlow,
                           rVal.bMirror, nullptr, &rVal.aPercentSize,
                           &rSize);

    if (bOnlyPercentRefValue)
        return;

    // --> OD 2009-09-01 #mongolianlayout#
    if ( m_bIsInVertical || m_bIsInVerticalL2R )
    {
        Point aPos(aBoundRect.Pos());
        long nTmp = aPos.X();
        aPos.X() = aPos.Y();
        aPos.Y() = nTmp;
        Size aSize(aBoundRect.SSize());
        nTmp = aSize.Width();
        aSize.Width() = aSize.Height();
        aSize.Height() = nTmp;
        aBoundRect.Chg( aPos, aSize );
        //exchange width/height to enable correct values
        nTmp = rVal.nWidth;
        rVal.nWidth = rVal.nHeight;
        rVal.nHeight = nTmp;
    }
    if ((eAnchorType == FLY_AT_PAGE) || (eAnchorType == FLY_AT_FLY))
    {
        // MinimalPosition
        rVal.nMinHPos = aBoundRect.Left();
        rVal.nMinVPos = aBoundRect.Top();
        SwTwips nH = rVal.nHPos;
        SwTwips nV = rVal.nVPos;

        if (rVal.nHPos + rVal.nWidth > aBoundRect.Right())
        {
            if (rVal.nHoriOrient == text::HoriOrientation::NONE)
            {
                rVal.nHPos -= ((rVal.nHPos + rVal.nWidth) - aBoundRect.Right());
                nH = rVal.nHPos;
            }
            else
                rVal.nWidth = aBoundRect.Right() - rVal.nHPos;
        }

        if (rVal.nHPos + rVal.nWidth > aBoundRect.Right())
            rVal.nWidth = aBoundRect.Right() - rVal.nHPos;

        if (rVal.nVPos + rVal.nHeight > aBoundRect.Bottom())
        {
            if (rVal.nVertOrient == text::VertOrientation::NONE)
            {
                rVal.nVPos -= ((rVal.nVPos + rVal.nHeight) - aBoundRect.Bottom());
                nV = rVal.nVPos;
            }
            else
                rVal.nHeight = aBoundRect.Bottom() - rVal.nVPos;
        }

        if (rVal.nVPos + rVal.nHeight > aBoundRect.Bottom())
            rVal.nHeight = aBoundRect.Bottom() - rVal.nVPos;

        if ( rVal.nVertOrient != text::VertOrientation::NONE )
            nV = aBoundRect.Top();

        if ( rVal.nHoriOrient != text::HoriOrientation::NONE )
            nH = aBoundRect.Left();

        rVal.nMaxHPos   = aBoundRect.Right()  - rVal.nWidth;
        rVal.nMaxHeight = aBoundRect.Bottom() - nV;

        rVal.nMaxVPos   = aBoundRect.Bottom() - rVal.nHeight;
        rVal.nMaxWidth  = aBoundRect.Right()  - nH;
    }
    // OD 12.11.2003 #i22341# - handle to character anchored objects vertical
    // aligned at character or top of line in a special case
    else if ((eAnchorType == FLY_AT_PARA) ||
                ((eAnchorType == FLY_AT_CHAR) &&
                !(rVal.nVRelOrient == text::RelOrientation::CHAR) &&
                !(rVal.nVRelOrient == text::RelOrientation::TEXT_LINE) ) )
    {
        if (rVal.nHPos + rVal.nWidth > aBoundRect.Right())
        {
            if (rVal.nHoriOrient == text::HoriOrientation::NONE)
            {
                rVal.nHPos -= ((rVal.nHPos + rVal.nWidth) - aBoundRect.Right());
            }
            else
                rVal.nWidth = aBoundRect.Right() - rVal.nHPos;
        }

        // OD 29.09.2003 #i17567#, #i18732# - consider following the text flow
        // and alignment at page areas.
        const bool bMaxVPosAtBottom = !rVal.bFollowTextFlow ||
                                      rVal.nVRelOrient == text::RelOrientation::PAGE_FRAME ||
                                      rVal.nVRelOrient == text::RelOrientation::PAGE_PRINT_AREA;
        {
            SwTwips nTmpMaxVPos = ( bMaxVPosAtBottom
                                    ? aBoundRect.Bottom()
                                    : aBoundRect.Height() ) -
                                  rVal.nHeight;
            if ( rVal.nVPos > nTmpMaxVPos )
            {
                if (rVal.nVertOrient == text::VertOrientation::NONE)
                {
                    rVal.nVPos = nTmpMaxVPos;
                }
                else
                {
                    rVal.nHeight = ( bMaxVPosAtBottom
                                     ? aBoundRect.Bottom()
                                     : aBoundRect.Height() ) - rVal.nVPos;
                }
            }
        }

        rVal.nMinHPos  = aBoundRect.Left();
        rVal.nMaxHPos  = aBoundRect.Right() - rVal.nWidth;

        rVal.nMinVPos  = aBoundRect.Top();
        // OD 26.09.2003 #i17567#, #i18732# - determine maximum vertical position
        if ( bMaxVPosAtBottom )
        {
            rVal.nMaxVPos  = aBoundRect.Bottom() - rVal.nHeight;
        }
        else
        {
            rVal.nMaxVPos  = aBoundRect.Height() - rVal.nHeight;
        }

        // maximum width height
        const SwTwips nH = ( rVal.nHoriOrient != text::HoriOrientation::NONE )
                           ? aBoundRect.Left()
                           : rVal.nHPos;
        const SwTwips nV = ( rVal.nVertOrient != text::VertOrientation::NONE )
                           ? aBoundRect.Top()
                           : rVal.nVPos;
        rVal.nMaxHeight  = rVal.nMaxVPos + rVal.nHeight - nV;
        rVal.nMaxWidth   = rVal.nMaxHPos + rVal.nWidth - nH;
    }
    // OD 12.11.2003 #i22341# - special case for to character anchored objects
    // vertical aligned at character or top of line.
    // Note: (1) positive vertical values are positions above the top of line
    //       (2) negative vertical values are positions below the top of line
    else if ( (eAnchorType == FLY_AT_CHAR) &&
              ( rVal.nVRelOrient == text::RelOrientation::CHAR ||
                rVal.nVRelOrient == text::RelOrientation::TEXT_LINE ) )
    {
        // determine horizontal values
        rVal.nMinHPos  = aBoundRect.Left();

        rVal.nMaxHPos  = aBoundRect.Right() - rVal.nWidth;
        if (rVal.nHPos + rVal.nWidth > aBoundRect.Right())
        {
            if (rVal.nHoriOrient == text::HoriOrientation::NONE)
            {
                rVal.nHPos -= ((rVal.nHPos + rVal.nWidth) - aBoundRect.Right());
            }
            else
                rVal.nWidth = aBoundRect.Right() - rVal.nHPos;
        }

        const SwTwips nH = ( rVal.nHoriOrient != text::HoriOrientation::NONE )
                           ? aBoundRect.Left()
                           : rVal.nHPos;
        rVal.nMaxWidth   = rVal.nMaxHPos + rVal.nWidth - nH;

        // determine vertical values
        rVal.nMinVPos = -( aBoundRect.Bottom() - rVal.nHeight );
        if ( rVal.nVPos < rVal.nMinVPos &&
             rVal.nVertOrient == text::VertOrientation::NONE )
        {
            rVal.nVPos = rVal.nMinVPos;
        }

        rVal.nMaxVPos  = -aBoundRect.Top();
        if ( rVal.nVPos > rVal.nMaxVPos &&
             rVal.nVertOrient == text::VertOrientation::NONE )
        {
            rVal.nVPos = rVal.nMaxVPos;
        }

        if ( rVal.nVertOrient == text::VertOrientation::NONE )
        {
            rVal.nMaxHeight = aBoundRect.Bottom() + rVal.nVPos;
        }
        else
        {
            rVal.nMaxHeight = aBoundRect.Height();
        }
    }
    else if ( eAnchorType == FLY_AS_CHAR )
    {
        rVal.nMinHPos = 0;
        rVal.nMaxHPos = 0;

        rVal.nMaxHeight = aBoundRect.Height();
        rVal.nMaxWidth  = aBoundRect.Width();

        rVal.nMaxVPos   = aBoundRect.Height();
        rVal.nMinVPos   = -aBoundRect.Height() + rVal.nHeight;
        if (rVal.nMaxVPos < rVal.nMinVPos)
        {
            rVal.nMinVPos = rVal.nMaxVPos;
            rVal.nMaxVPos = -aBoundRect.Height();
        }
    }
    // --> OD 2009-09-01 #mongolianlayout#
    if ( m_bIsInVertical || m_bIsInVerticalL2R )
    {
        //restore width/height exchange
        long nTmp = rVal.nWidth;
        rVal.nWidth = rVal.nHeight;
        rVal.nHeight = nTmp;
    }

    if (rVal.nMaxWidth < rVal.nWidth)
        rVal.nWidth = rVal.nMaxWidth;
    if (rVal.nMaxHeight < rVal.nHeight)
        rVal.nHeight = rVal.nMaxHeight;
}

// correction for border
SwTwips SwFlyFrmAttrMgr::CalcTopSpace()
{
    const SvxShadowItem& rShadow = GetShadow();
    const SvxBoxItem&    rBox    = GetBox();
    return rShadow.CalcShadowSpace(SvxShadowItemSide::TOP ) + rBox.CalcLineSpace(SvxBoxItemLine::TOP);
}

SwTwips SwFlyFrmAttrMgr::CalcBottomSpace()
{
    const SvxShadowItem& rShadow = GetShadow();
    const SvxBoxItem& rBox       = GetBox();
    return rShadow.CalcShadowSpace(SvxShadowItemSide::BOTTOM) + rBox.CalcLineSpace(SvxBoxItemLine::BOTTOM);
}

SwTwips SwFlyFrmAttrMgr::CalcLeftSpace()
{
    const SvxShadowItem& rShadow = GetShadow();
    const SvxBoxItem&    rBox    = GetBox();
    return rShadow.CalcShadowSpace(SvxShadowItemSide::LEFT) + rBox.CalcLineSpace(SvxBoxItemLine::LEFT);
}

SwTwips SwFlyFrmAttrMgr::CalcRightSpace()
{
    const SvxShadowItem& rShadow = GetShadow();
    const SvxBoxItem&    rBox    = GetBox();
    return rShadow.CalcShadowSpace(SvxShadowItemSide::RIGHT) + rBox.CalcLineSpace(SvxBoxItemLine::RIGHT);
}

// erase attribute from the set
void SwFlyFrmAttrMgr::DelAttr( sal_uInt16 nId )
{
    m_aSet.ClearItem( nId );
}

void SwFlyFrmAttrMgr::SetLRSpace( long nLeft, long nRight )
{
    OSL_ENSURE( LONG_MAX != nLeft && LONG_MAX != nRight, "Welchen Raend setzen?" );

    SvxLRSpaceItem aTmp( static_cast<const SvxLRSpaceItem&>(m_aSet.Get( RES_LR_SPACE )) );
    if( LONG_MAX != nLeft )
        aTmp.SetLeft( sal_uInt16(nLeft) );
    if( LONG_MAX != nRight )
        aTmp.SetRight( sal_uInt16(nRight) );
    m_aSet.Put( aTmp );
}

void SwFlyFrmAttrMgr::SetULSpace( long nTop, long nBottom )
{
    OSL_ENSURE(LONG_MAX != nTop && LONG_MAX != nBottom, "Welchen Raend setzen?" );

    SvxULSpaceItem aTmp( static_cast<const SvxULSpaceItem&>(m_aSet.Get( RES_UL_SPACE )) );
    if( LONG_MAX != nTop )
        aTmp.SetUpper( sal_uInt16(nTop) );
    if( LONG_MAX != nBottom )
        aTmp.SetLower( sal_uInt16(nBottom) );
    m_aSet.Put( aTmp );
}

void SwFlyFrmAttrMgr::SetPos( const Point& rPoint )
{
    SwFormatVertOrient aVertOrient( GetVertOrient() );
    SwFormatHoriOrient aHoriOrient( GetHoriOrient() );

    aHoriOrient.SetPos       ( rPoint.X() );
    aHoriOrient.SetHoriOrient( text::HoriOrientation::NONE  );

    aVertOrient.SetPos       ( rPoint.Y() );
    aVertOrient.SetVertOrient( text::VertOrientation::NONE  );

    m_aSet.Put( aVertOrient );
    m_aSet.Put( aHoriOrient );
}

void SwFlyFrmAttrMgr::SetHorzOrientation( sal_Int16 eOrient )
{
    SwFormatHoriOrient aHoriOrient( GetHoriOrient() );
    aHoriOrient.SetHoriOrient( eOrient );
    m_aSet.Put( aHoriOrient );
}

void SwFlyFrmAttrMgr::SetVertOrientation( sal_Int16 eOrient )
{
    SwFormatVertOrient aVertOrient( GetVertOrient() );
    aVertOrient.SetVertOrient( eOrient );
    m_aSet.Put( aVertOrient );
}

void SwFlyFrmAttrMgr::SetHeightSizeType( SwFrmSize eType )
{
    SwFormatFrmSize aSize( GetFrmSize() );
    aSize.SetHeightSizeType( eType );
    m_aSet.Put( aSize );
}

void SwFlyFrmAttrMgr::SetSize( const Size& rSize )
{
    SwFormatFrmSize aSize( GetFrmSize() );
    aSize.SetSize(Size(std::max(rSize.Width(), long(MINFLY)), std::max(rSize.Height(), long(MINFLY))));
    m_aSet.Put( aSize );
}

void SwFlyFrmAttrMgr::SetAttrSet(const SfxItemSet& rSet)
{
    m_aSet.ClearItem();
    m_aSet.Put( rSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
