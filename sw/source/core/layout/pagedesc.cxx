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
#include <sal/log.hxx>
#include <fmtclds.hxx>
#include <fmtfsize.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <swtable.hxx>
#include <frmatr.hxx>
#include <frmtool.hxx>
#include <doc.hxx>
#include <node.hxx>
#include <strings.hrc>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <poolfmt.hxx>
#include <calbck.hxx>
#include <hints.hxx>

SwPageDesc::SwPageDesc(const OUString& rName, SwFrameFormat *pFormat, SwDoc *const pDoc)
    : SwModify(nullptr)
    , m_StyleName( rName )
    , m_Master( pDoc->GetAttrPool(), rName, pFormat )
    , m_Left( pDoc->GetAttrPool(), rName, pFormat )
    , m_FirstMaster( pDoc->GetAttrPool(), rName, pFormat )
    , m_FirstLeft( pDoc->GetAttrPool(), rName, pFormat )
    , m_aDepends(*this)
    , m_pTextFormatColl(nullptr)
    , m_pFollow( this )
    , m_nRegHeight( 0 )
    , m_nRegAscent( 0 )
    , m_nVerticalAdjustment( drawing::TextVerticalAdjust_TOP )
    , m_eUse( UseOnPage::All | UseOnPage::HeaderShare | UseOnPage::FooterShare | UseOnPage::FirstShare )
    , m_IsLandscape( false )
    , m_IsHidden( false )
    , m_pdList( nullptr )
{
}

SwPageDesc::SwPageDesc( const SwPageDesc &rCpy )
    : SwModify(nullptr)
    , BroadcasterMixin()
    , m_StyleName( rCpy.GetName() )
    , m_NumType( rCpy.GetNumType() )
    , m_Master( rCpy.GetMaster() )
    , m_Left( rCpy.GetLeft() )
    , m_FirstMaster( rCpy.GetFirstMaster() )
    , m_FirstLeft( rCpy.GetFirstLeft() )
    , m_aDepends(*this)
    , m_pTextFormatColl(nullptr)
    , m_pFollow( rCpy.m_pFollow )
    , m_nRegHeight( rCpy.GetRegHeight() )
    , m_nRegAscent( rCpy.GetRegAscent() )
    , m_nVerticalAdjustment( rCpy.GetVerticalAdjustment() )
    , m_eUse( rCpy.ReadUseOn() )
    , m_IsLandscape( rCpy.GetLandscape() )
    , m_IsHidden( rCpy.IsHidden() )
    , m_IsFootnoteInfo( rCpy.GetFootnoteInfo() )
    , m_pdList( nullptr )
{
    if (rCpy.m_pTextFormatColl && rCpy.m_aDepends.IsListeningTo(rCpy.m_pTextFormatColl))
    {
        m_pTextFormatColl = rCpy.m_pTextFormatColl;
        m_aDepends.StartListening(const_cast<SwTextFormatColl*>(m_pTextFormatColl));
    }
}

SwPageDesc & SwPageDesc::operator = (const SwPageDesc & rSrc)
{
    m_StyleName = rSrc.m_StyleName;
    m_NumType = rSrc.m_NumType;
    m_Master = rSrc.m_Master;
    m_Left = rSrc.m_Left;
    m_FirstMaster = rSrc.m_FirstMaster;
    m_FirstLeft = rSrc.m_FirstLeft;
    m_aDepends.EndListeningAll();
    if (rSrc.m_pTextFormatColl && rSrc.m_aDepends.IsListeningTo(rSrc.m_pTextFormatColl))
    {
        m_pTextFormatColl = rSrc.m_pTextFormatColl;
        m_aDepends.StartListening(const_cast<SwTextFormatColl*>(m_pTextFormatColl));
    }
    else
        m_pTextFormatColl = nullptr;

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

bool SwPageDesc::SetName( const OUString& rNewName )
{
    bool renamed = true;
    if (m_pdList) {
        SwPageDescs::iterator it = m_pdList->find_( m_StyleName );
        if( m_pdList->end() == it ) {
            SAL_WARN( "sw", "SwPageDesc not found in expected m_pdList" );
            return false;
        }
        renamed = m_pdList->m_PosIndex.modify( it,
            change_name( rNewName ), change_name( m_StyleName ) );
    }
    else
        m_StyleName = rNewName;
    return renamed;
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

void SwPageDesc::ResetAllAttr()
{
    SwFrameFormat& rFormat = GetMaster();

    // #i73790# - method renamed
    rFormat.ResetAllFormatAttr();
    rFormat.SetFormatAttr( SvxFrameDirectionItem(SvxFrameDirection::Horizontal_LR_TB, RES_FRAMEDIR) );
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
void SwPageDesc::SetRegisterFormatColl(const SwTextFormatColl* pFormat)
{
    if(pFormat != m_pTextFormatColl)
    {
        m_aDepends.EndListeningAll();
        m_pTextFormatColl = pFormat;
        m_aDepends.StartListening(const_cast<SwTextFormatColl*>(m_pTextFormatColl));
        RegisterChange();
    }
}

/// retrieve the style for the grid alignment
const SwTextFormatColl* SwPageDesc::GetRegisterFormatColl() const
{
    if (!m_aDepends.IsListeningTo(m_pTextFormatColl))
        m_pTextFormatColl = nullptr;
    return m_pTextFormatColl;
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
void SwPageDesc::SwClientNotify(const SwModify& rModify, const SfxHint& rHint)
{
    if(auto pLegacyHint = dynamic_cast<const sw::LegacyModifyHint*>(&rHint))
    {
        const sal_uInt16 nWhich = pLegacyHint->m_pOld
                ? pLegacyHint->m_pOld->Which()
                : pLegacyHint->m_pNew
                ? pLegacyHint->m_pNew->Which()
                : 0;
        NotifyClients(pLegacyHint->m_pOld, pLegacyHint->m_pNew);
        if((RES_ATTRSET_CHG == nWhich)
                || (RES_FMT_CHG == nWhich)
                || isCHRATR(nWhich)
                || (RES_PARATR_LINESPACING == nWhich))
            RegisterChange();
    }
    else if (auto pModifyChangedHint = dynamic_cast<const sw::ModifyChangedHint*>(&rHint))
    {
        if(m_pTextFormatColl == &rModify)
            m_pTextFormatColl = static_cast<const SwTextFormatColl*>(pModifyChangedHint->m_pNew);
        else
            assert(false);
    }
}

static const SwFrame* lcl_GetFrameOfNode( const SwNode& rNd )
{
    const SwModify* pMod;
    SwFrameType nFrameType = FRM_CNTNT;

    if( rNd.IsContentNode() )
    {
        pMod = &static_cast<const SwContentNode&>(rNd);
    }
    else if( rNd.IsTableNode() )
    {
        pMod = static_cast<const SwTableNode&>(rNd).GetTable().GetFrameFormat();
        nFrameType = SwFrameType::Tab;
    }
    else
        pMod = nullptr;

    Point aNullPt;
    std::pair<Point, bool> const tmp(aNullPt, false);
    return pMod ? ::GetFrameOfModify(nullptr, *pMod, nFrameType, nullptr, &tmp)
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
    return (UseOnPage::Left & m_eUse)
            ? (bFirst ? &m_FirstLeft : &m_Left)
            : nullptr;
}

SwFrameFormat *SwPageDesc::GetRightFormat(bool const bFirst)
{
    return (UseOnPage::Right & m_eUse)
            ? (bFirst ? &m_FirstMaster : &m_Master)
            : nullptr;
}

bool SwPageDesc::IsFirstShared() const
{
    return bool(m_eUse & UseOnPage::FirstShare);
}

void SwPageDesc::ChgFirstShare( bool bNew )
{
    if ( bNew )
        m_eUse |= UseOnPage::FirstShare;
    else
        m_eUse &= UseOnPage::NoFirstShare;
}

// Page styles
static const char* STR_POOLPAGE[] =
{
    STR_POOLPAGE_STANDARD,
    STR_POOLPAGE_FIRST,
    STR_POOLPAGE_LEFT,
    STR_POOLPAGE_RIGHT,
    STR_POOLPAGE_JAKET,
    STR_POOLPAGE_REGISTER,
    STR_POOLPAGE_HTML,
    STR_POOLPAGE_FOOTNOTE,
    STR_POOLPAGE_ENDNOTE,
    STR_POOLPAGE_LANDSCAPE
};

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

    for (size_t i = 0; i < SAL_N_ELEMENTS(STR_POOLPAGE); ++i)
    {
        if (rName == SwResId(STR_POOLPAGE[i]))
        {
            return rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool( static_cast< sal_uInt16 >(
                        i + RES_POOLPAGE_BEGIN) );
        }
    }

    return nullptr;
}

SwPageFootnoteInfo::SwPageFootnoteInfo()
    : m_nMaxHeight( 0 )
    , m_nLineWidth(10)
    , m_eLineStyle( SvxBorderLineStyle::SOLID )
    , m_Width( 25, 100 )
    , m_nTopDist( 57 )         //1mm
    , m_nBottomDist( 57 )
{
    m_eAdjust = SvxFrameDirection::Horizontal_RL_TB == GetDefaultFrameDirection(GetAppLanguage()) ?
           css::text::HorizontalAdjust_RIGHT :
           css::text::HorizontalAdjust_LEFT;
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

OUString const & SwPageDescExt::GetName() const
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

SwPageDescs::SwPageDescs()
    : m_PosIndex( m_Array.get<0>() )
    , m_NameIndex( m_Array.get<1>() )
{
}

SwPageDescs::~SwPageDescs()
{
    for(const_iterator it = begin(); it != end(); ++it)
        delete *it;
}

SwPageDescs::iterator SwPageDescs::find_(const OUString &name) const
{
    ByName::iterator it = m_NameIndex.find( name );
    return m_Array.iterator_to( *it );
}

std::pair<SwPageDescs::const_iterator,bool> SwPageDescs::push_back( const value_type& x )
{
    // SwPageDesc is not already in a SwPageDescs list!
    assert( x->m_pdList == nullptr );

    std::pair<iterator,bool> res = m_PosIndex.push_back( x );
    if( res.second )
        x->m_pdList = this;
    return res;
}

void SwPageDescs::erase( const value_type& x )
{
    // SwPageDesc is not in this SwPageDescs list!
    assert( x->m_pdList == this );

    iterator const ret = find_( x->GetName() );
    if (ret != end())
        m_PosIndex.erase( ret );
    else
        SAL_WARN( "sw", "SwPageDesc is not in SwPageDescs m_pdList!" );
    x->m_pdList = nullptr;
}

void SwPageDescs::erase( const_iterator const& position )
{
    // SwPageDesc is not in this SwPageDescs list!
    assert( (*position)->m_pdList == this );

    (*position)->m_pdList = nullptr;
    m_PosIndex.erase( position );
}

void SwPageDescs::erase( size_type index_ )
{
    erase( begin() + index_ );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
