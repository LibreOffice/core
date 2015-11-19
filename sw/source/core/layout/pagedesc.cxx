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

#include <editeng/pbinitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/frmdiritem.hxx>
#include <fmtclds.hxx>
#include <fmtfsize.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <swtable.hxx>
#include <frmtool.hxx>
#include <doc.hxx>
#include <poolfmt.hrc>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <poolfmt.hxx>
#include <calbck.hxx>

SwPageDesc::SwPageDesc(const OUString& rName, SwFrameFormat *pFormat, SwDoc *const pDoc)
    : SwModify(nullptr)
    , m_StyleName( rName )
    , m_Master( pDoc->GetAttrPool(), rName, pFormat )
    , m_Left( pDoc->GetAttrPool(), rName, pFormat )
    , m_FirstMaster( pDoc->GetAttrPool(), rName, pFormat )
    , m_FirstLeft( pDoc->GetAttrPool(), rName, pFormat )
    , m_Depend( this, nullptr )
    , m_pFollow( this )
    , m_nRegHeight( 0 )
    , m_nRegAscent( 0 )
    , m_nVerticalAdjustment( drawing::TextVerticalAdjust_TOP )
    , m_eUse( (UseOnPage)(nsUseOnPage::PD_ALL | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE | nsUseOnPage::PD_FIRSTSHARE) )
    , m_IsLandscape( false )
    , m_IsHidden( false )
{
}

SwPageDesc::SwPageDesc( const SwPageDesc &rCpy )
    : SwModify(nullptr)
    , m_StyleName( rCpy.GetName() )
    , m_NumType( rCpy.GetNumType() )
    , m_Master( rCpy.GetMaster() )
    , m_Left( rCpy.GetLeft() )
    , m_FirstMaster( rCpy.GetFirstMaster() )
    , m_FirstLeft( rCpy.GetFirstLeft() )
    , m_Depend( this, const_cast<SwModify*>(rCpy.m_Depend.GetRegisteredIn()) )
    , m_pFollow( rCpy.m_pFollow )
    , m_nRegHeight( rCpy.GetRegHeight() )
    , m_nRegAscent( rCpy.GetRegAscent() )
    , m_nVerticalAdjustment( rCpy.GetVerticalAdjustment() )
    , m_eUse( rCpy.ReadUseOn() )
    , m_IsLandscape( rCpy.GetLandscape() )
    , m_IsHidden( rCpy.IsHidden() )
    , m_IsFootnoteInfo( rCpy.GetFootnoteInfo() )
{
}

SwPageDesc & SwPageDesc::operator = (const SwPageDesc & rSrc)
{
    m_StyleName = rSrc.m_StyleName;
    m_NumType = rSrc.m_NumType;
    m_Master = rSrc.m_Master;
    m_Left = rSrc.m_Left;
    m_FirstMaster = rSrc.m_FirstMaster;
    m_FirstLeft = rSrc.m_FirstLeft;

    if (rSrc.m_pFollow == &rSrc)
        m_pFollow = this;
    else
        m_pFollow = rSrc.m_pFollow;

    m_nRegHeight = rSrc.m_nRegHeight;
    m_nRegAscent = rSrc.m_nRegAscent;
    m_nVerticalAdjustment = rSrc.m_nVerticalAdjustment;
    m_eUse = rSrc.m_eUse;
    m_IsLandscape = rSrc.m_IsLandscape;
    return *this;
}

SwPageDesc::~SwPageDesc()
{
}

/// Only the margin is mirrored.
/// Attributes like borders and so on are copied 1:1.
void SwPageDesc::Mirror()
{
    //Only the margins are mirrored, all other values are just copied.
    SvxLRSpaceItem aLR( RES_LR_SPACE );
    const SvxLRSpaceItem &rLR = m_Master.GetLRSpace();
    aLR.SetLeft(  rLR.GetRight() );
    aLR.SetRight( rLR.GetLeft() );

    SfxItemSet aSet( *m_Master.GetAttrSet().GetPool(),
                     m_Master.GetAttrSet().GetRanges() );
    aSet.Put( aLR );
    aSet.Put( m_Master.GetFrameSize() );
    aSet.Put( m_Master.GetPaperBin() );
    aSet.Put( m_Master.GetULSpace() );
    aSet.Put( m_Master.GetBox() );
    aSet.Put( m_Master.makeBackgroundBrushItem() );
    aSet.Put( m_Master.GetShadow() );
    aSet.Put( m_Master.GetCol() );
    aSet.Put( m_Master.GetFrameDir() );
    m_Left.SetFormatAttr( aSet );
}

void SwPageDesc::ResetAllAttr( bool bLeft )
{
    SwFrameFormat& rFormat = bLeft ? GetLeft() : GetMaster();

    // #i73790# - method renamed
    rFormat.ResetAllFormatAttr();
    rFormat.SetFormatAttr( SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR) );
}

// gets information from Modify
bool SwPageDesc::GetInfo( SfxPoolItem & rInfo ) const
{
    if (!m_Master.GetInfo(rInfo))
        return false;       // found
    if (!m_Left.GetInfo(rInfo))
        return false ;
    if ( !m_FirstMaster.GetInfo( rInfo ) )
        return false;
    return m_FirstLeft.GetInfo( rInfo );
}

/// set the style for the grid alignment
void SwPageDesc::SetRegisterFormatColl( const SwTextFormatColl* pFormat )
{
    if( pFormat != GetRegisterFormatColl() )
    {
        if( pFormat )
            const_cast<SwTextFormatColl*>(pFormat)->Add(&m_Depend);
        else
            const_cast<SwTextFormatColl*>(GetRegisterFormatColl())->Remove(&m_Depend);

        RegisterChange();
    }
}

/// retrieve the style for the grid alignment
const SwTextFormatColl* SwPageDesc::GetRegisterFormatColl() const
{
    const SwModify* pReg = m_Depend.GetRegisteredIn();
    return static_cast<const SwTextFormatColl*>(pReg);
}

/// notify all affected page frames
void SwPageDesc::RegisterChange()
{
    // #117072# - During destruction of the document <SwDoc>
    // the page description is modified. Thus, do nothing, if the document
    // is in destruction respectively if no viewshell exists.
    SwDoc* pDoc = GetMaster().GetDoc();
    if ( !pDoc || pDoc->IsInDtor() )
    {
        return;
    }
    SwViewShell* pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    if ( !pSh )
    {
        return;
    }

    m_nRegHeight = 0;
    {
        SwIterator<SwFrame,SwFormat> aIter( GetMaster() );
        for( SwFrame* pLast = aIter.First(); pLast; pLast = aIter.Next() )
        {
            if( pLast->IsPageFrame() )
                static_cast<SwPageFrame*>(pLast)->PrepareRegisterChg();
        }
    }
    {
        SwIterator<SwFrame,SwFormat> aIter( GetLeft() );
        for( SwFrame* pLast = aIter.First(); pLast; pLast = aIter.Next() )
        {
            if( pLast->IsPageFrame() )
                static_cast<SwPageFrame*>(pLast)->PrepareRegisterChg();
        }
    }
    {
        SwIterator<SwFrame,SwFormat> aIter( GetFirstMaster() );
        for( SwFrame* pLast = aIter.First(); pLast; pLast = aIter.Next() )
        {
            if( pLast->IsPageFrame() )
                static_cast<SwPageFrame*>(pLast)->PrepareRegisterChg();
        }
    }
    {
        SwIterator<SwFrame,SwFormat> aIter( GetFirstLeft() );
        for( SwFrame* pLast = aIter.First(); pLast; pLast = aIter.Next() )
        {
            if( pLast->IsPageFrame() )
                static_cast<SwPageFrame*>(pLast)->PrepareRegisterChg();
        }
    }
}

/// special handling if the style of the grid alignment changes
void SwPageDesc::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    NotifyClients( pOld, pNew );

    if ( (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich)
        || isCHRATR(nWhich) || (RES_PARATR_LINESPACING == nWhich) )
    {
        RegisterChange();
    }
}

static const SwFrame* lcl_GetFrameOfNode( const SwNode& rNd )
{
    const SwModify* pMod;
    sal_uInt16 nFrameType = FRM_CNTNT;

    if( rNd.IsContentNode() )
    {
        pMod = &static_cast<const SwContentNode&>(rNd);
    }
    else if( rNd.IsTableNode() )
    {
        pMod = static_cast<const SwTableNode&>(rNd).GetTable().GetFrameFormat();
        nFrameType = FRM_TAB;
    }
    else
        pMod = nullptr;

    Point aNullPt;
    return pMod ? ::GetFrameOfModify( nullptr, *pMod, nFrameType, &aNullPt )
                : nullptr;
}

const SwPageDesc* SwPageDesc::GetPageDescOfNode(const SwNode& rNd)
{
    const SwPageDesc* pRet = nullptr;
    const SwFrame* pChkFrame = lcl_GetFrameOfNode( rNd );
    if (pChkFrame && nullptr != (pChkFrame = pChkFrame->FindPageFrame()))
        pRet = static_cast<const SwPageFrame*>(pChkFrame)->GetPageDesc();
    return pRet;
}

const SwFrameFormat* SwPageDesc::GetPageFormatOfNode( const SwNode& rNd,
                                              bool bCheckForThisPgDc ) const
{
    // which PageDescFormat is valid for this node?
    const SwFrameFormat* pRet;
    const SwFrame* pChkFrame = lcl_GetFrameOfNode( rNd );

    if( pChkFrame && nullptr != ( pChkFrame = pChkFrame->FindPageFrame() ))
    {
        const SwPageDesc* pPd = bCheckForThisPgDc ? this :
                                static_cast<const SwPageFrame*>(pChkFrame)->GetPageDesc();
        pRet = &pPd->GetMaster();
        OSL_ENSURE( static_cast<const SwPageFrame*>(pChkFrame)->GetPageDesc() == pPd, "Wrong node for detection of page format!" );
        // this page is assigned to which format?
        if( !pChkFrame->KnowsFormat(*pRet) )
        {
            pRet = &pPd->GetLeft();
            OSL_ENSURE( pChkFrame->KnowsFormat(*pRet), "Wrong node for detection of page format!" );
        }
    }
    else
        pRet = &GetMaster();
    return pRet;
}

bool SwPageDesc::IsFollowNextPageOfNode( const SwNode& rNd ) const
{
    bool bRet = false;
    if( GetFollow() && this != GetFollow() )
    {
        const SwFrame* pChkFrame = lcl_GetFrameOfNode( rNd );
        if( pChkFrame && nullptr != ( pChkFrame = pChkFrame->FindPageFrame() ) &&
            pChkFrame->IsPageFrame() &&
            ( !pChkFrame->GetNext() || GetFollow() ==
                        static_cast<const SwPageFrame*>(pChkFrame->GetNext())->GetPageDesc() ))
            // the page on which the follow points was found
            bRet = true;
    }
    return bRet;
}

SwFrameFormat *SwPageDesc::GetLeftFormat(bool const bFirst)
{
    return (nsUseOnPage::PD_LEFT & m_eUse)
            ? ((bFirst) ? &m_FirstLeft : &m_Left)
            : nullptr;
}

SwFrameFormat *SwPageDesc::GetRightFormat(bool const bFirst)
{
    return (nsUseOnPage::PD_RIGHT & m_eUse)
            ? ((bFirst) ? &m_FirstMaster : &m_Master)
            : nullptr;
}

bool SwPageDesc::IsFirstShared() const
{
    return (m_eUse & nsUseOnPage::PD_FIRSTSHARE) != 0;
}

void SwPageDesc::ChgFirstShare( bool bNew )
{
    if ( bNew )
        m_eUse = (UseOnPage) (m_eUse | nsUseOnPage::PD_FIRSTSHARE);
    else
        m_eUse = (UseOnPage) (m_eUse & nsUseOnPage::PD_NOFIRSTSHARE);
}

SwPageDesc* SwPageDesc::GetByName(SwDoc& rDoc, const OUString& rName)
{
    const size_t nDCount = rDoc.GetPageDescCnt();

    for( size_t i = 0; i < nDCount; i++ )
    {
        SwPageDesc* pDsc = &rDoc.GetPageDesc( i );
        if(pDsc->GetName() == rName)
        {
            return pDsc;
        }
    }

    for( sal_Int32 i = RC_POOLPAGEDESC_BEGIN; i <= STR_POOLPAGE_LANDSCAPE; ++i)
    {
        if (rName==SW_RESSTR(i))
        {
            return rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool( static_cast< sal_uInt16 >(
                        i - RC_POOLPAGEDESC_BEGIN + RES_POOLPAGE_BEGIN) );
        }
    }

    return nullptr;
}

SwPageFootnoteInfo::SwPageFootnoteInfo()
    : m_nMaxHeight( 0 )
    , m_nLineWidth(10)
    , m_eLineStyle( table::BorderLineStyle::SOLID )
    , m_Width( 25, 100 )
    , m_nTopDist( 57 )         //1mm
    , m_nBottomDist( 57 )
{
    m_eAdjust = FRMDIR_HORI_RIGHT_TOP == GetDefaultFrameDirection(GetAppLanguage()) ?
           FTNADJ_RIGHT :
           FTNADJ_LEFT;
}

SwPageFootnoteInfo::SwPageFootnoteInfo( const SwPageFootnoteInfo &rCpy )
    : m_nMaxHeight(rCpy.GetHeight())
    , m_nLineWidth(rCpy.m_nLineWidth)
    , m_eLineStyle(rCpy.m_eLineStyle)
    , m_LineColor(rCpy.m_LineColor)
    , m_Width(rCpy.GetWidth())
    , m_eAdjust(rCpy.GetAdj())
    , m_nTopDist(rCpy.GetTopDist())
    , m_nBottomDist(rCpy.GetBottomDist())
{
}

SwPageFootnoteInfo &SwPageFootnoteInfo::operator=( const SwPageFootnoteInfo& rCpy )
{
    m_nMaxHeight  = rCpy.GetHeight();
    m_nLineWidth  = rCpy.m_nLineWidth;
    m_eLineStyle  = rCpy.m_eLineStyle;
    m_LineColor   = rCpy.m_LineColor;
    m_Width       = rCpy.GetWidth();
    m_eAdjust     = rCpy.GetAdj();
    m_nTopDist    = rCpy.GetTopDist();
    m_nBottomDist = rCpy.GetBottomDist();
    return *this;
}

bool SwPageFootnoteInfo::operator==( const SwPageFootnoteInfo& rCmp ) const
{
    return m_nMaxHeight == rCmp.GetHeight()
        && m_nLineWidth == rCmp.m_nLineWidth
        && m_eLineStyle == rCmp.m_eLineStyle
        && m_LineColor  == rCmp.m_LineColor
        && m_Width      == rCmp.GetWidth()
        && m_eAdjust    == rCmp.GetAdj()
        && m_nTopDist   == rCmp.GetTopDist()
        && m_nBottomDist== rCmp.GetBottomDist();
}

SwPageDescExt::SwPageDescExt(const SwPageDesc & rPageDesc, SwDoc *const pDoc)
    : m_PageDesc(rPageDesc)
    , m_pDoc(pDoc)
{
    SetPageDesc(rPageDesc);
}

SwPageDescExt::SwPageDescExt(const SwPageDescExt & rSrc)
    : m_PageDesc(rSrc.m_PageDesc)
    , m_pDoc(rSrc.m_pDoc)
{
    SetPageDesc(rSrc.m_PageDesc);
}

SwPageDescExt::~SwPageDescExt()
{
}

OUString SwPageDescExt::GetName() const
{
    return m_PageDesc.GetName();
}

void SwPageDescExt::SetPageDesc(const SwPageDesc & rPageDesc)
{
    m_PageDesc = rPageDesc;

    if (m_PageDesc.GetFollow())
        m_sFollow = m_PageDesc.GetFollow()->GetName();
}

SwPageDescExt & SwPageDescExt::operator = (const SwPageDesc & rSrc)
{
    SetPageDesc(rSrc);

    return *this;
}

SwPageDescExt & SwPageDescExt::operator = (const SwPageDescExt & rSrc)
{
    SetPageDesc(rSrc.m_PageDesc);

    return *this;
}

SwPageDescExt::operator SwPageDesc() const
{
    SwPageDesc aResult(m_PageDesc);

    SwPageDesc * pPageDesc = m_pDoc->FindPageDesc(m_sFollow);

    if ( nullptr != pPageDesc )
        aResult.SetFollow(pPageDesc);

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
