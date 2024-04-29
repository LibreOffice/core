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

#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/text/ColumnSeparatorStyle.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/text/TextGridMode.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <sal/log.hxx>
#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <svtools/unoimap.hxx>
#include <tools/UnitConversion.hxx>
#include <vcl/imap.hxx>
#include <vcl/imapobj.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <frmfmt.hxx>
#include <unocoll.hxx>
#include <fmtclds.hxx>
#include <fmtornt.hxx>
#include <fmthdft.hxx>
#include <fmtpdsc.hxx>
#include <fmtcntnt.hxx>
#include <fmtfsize.hxx>
#include <fmtfordr.hxx>
#include <fmtsrnd.hxx>
#include <fmtlsplt.hxx>
#include <fmtrowsplt.hxx>
#include <fmtftntx.hxx>
#include <fmteiro.hxx>
#include <fmturl.hxx>
#include <fmtcnct.hxx>
#include <section.hxx>
#include <fmtline.hxx>
#include <tgrditem.hxx>
#include <hfspacingitem.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <notxtfrm.hxx>
#include <txtfrm.hxx>
#include <crsrsh.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <frmtool.hxx>
#include <flyfrms.hxx>
#include <pagedesc.hxx>
#include <grfatr.hxx>
#include <ndnotxt.hxx>
#include <node2lay.hxx>
#include <fmtclbl.hxx>
#include <swunohelper.hxx>
#include <unoframe.hxx>
#include <SwStyleNameMapper.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/GraphicObject.hxx>
#include <unomid.h>
#include <strings.hrc>
#include <svx/svdundo.hxx>
#include <svx/SvxXTextColumns.hxx>
#include <sortedobjs.hxx>
#include <HandleAnchorNodeChg.hxx>
#include <calbck.hxx>
#include <pagedeschint.hxx>
#include <drawdoc.hxx>
#include <hints.hxx>
#include <frameformats.hxx>
#include <unoprnms.hxx>

#include <ndtxt.hxx>

#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <svl/itemiter.hxx>
#include <wrtsh.hxx>
#include <txtfld.hxx>
#include <cellatr.hxx>

using namespace ::com::sun::star;

namespace sw {

bool GetAtPageRelOrientation(sal_Int16 & rOrientation, bool const isIgnorePrintArea)
{
    switch (rOrientation)
    {
        case text::RelOrientation::CHAR:
        case text::RelOrientation::FRAME:
            rOrientation = text::RelOrientation::PAGE_FRAME;
            return true;
        case text::RelOrientation::PRINT_AREA:
            if (isIgnorePrintArea)
            {
                return false;
            }
            else
            {
                rOrientation = text::RelOrientation::PAGE_PRINT_AREA;
                return true;
            }
        case text::RelOrientation::FRAME_LEFT:
            rOrientation = text::RelOrientation::PAGE_LEFT;
            return true;
        case text::RelOrientation::FRAME_RIGHT:
            rOrientation = text::RelOrientation::PAGE_RIGHT;
            return true;
        default:
            return false;
    }
}

} // namespace sw

SfxPoolItem* SwFormatLineNumber::CreateDefault() { return new SwFormatLineNumber; }

static sal_Int16 lcl_IntToRelation(const uno::Any& rVal)
{
    sal_Int16 nVal = text::RelOrientation::FRAME;
    if (!(rVal >>= nVal))
        SAL_WARN("sw.core", "lcl_IntToRelation: read from Any failed!");
    return nVal;
}

static void lcl_DelHFFormat( SwClient *pToRemove, SwFrameFormat *pFormat )
{
    //If the client is the last one who uses this format, then we have to delete
    //it - before this is done, we may need to delete the content-section.
    SwDoc* pDoc = pFormat->GetDoc();
    pFormat->Remove(*pToRemove);
    if( pDoc->IsInDtor() )
    {
        delete pFormat;
        return;
    }

    // Anything other than frames registered?
    bool bDel = true;
    {
        // nested scope because DTOR of SwClientIter resets the flag bTreeChg.
        // It's suboptimal if the format is deleted beforehand.
        SwIterator<SwClient,SwFrameFormat> aIter(*pFormat);
        for(SwClient* pLast = aIter.First(); bDel && pLast; pLast = aIter.Next())
            if (dynamic_cast<const SwFrame*>(pLast) == nullptr)
                bDel = false;
    }

    if ( !bDel )
        return;

    // If there is a Cursor registered in one of the nodes, we need to call the
    // ParkCursor in an (arbitrary) shell.
    SwFormatContent& rCnt = const_cast<SwFormatContent&>(pFormat->GetContent());
    if ( rCnt.GetContentIdx() )
    {
        SwNode *pNode = nullptr;
        {
            // #i92993#
            // Begin with start node of page header/footer to assure that
            // complete content is checked for cursors and the complete content
            // is deleted on below made method call <pDoc->getIDocumentContentOperations().DeleteSection(pNode)>
            SwNodeIndex aIdx( *rCnt.GetContentIdx(), 0 );
            // If there is a Cursor registered in one of the nodes, we need to call the
            // ParkCursor in an (arbitrary) shell.
            pNode = & aIdx.GetNode();
            SwNodeOffset nEnd = pNode->EndOfSectionIndex();
            while ( aIdx < nEnd )
            {
                if ( pNode->IsContentNode() &&
                     static_cast<SwContentNode*>(pNode)->HasWriterListeners() )
                {
                    SwCursorShell *pShell = SwIterator<SwCursorShell,SwContentNode>( *static_cast<SwContentNode*>(pNode) ).First();
                    if( pShell )
                    {
                        pShell->ParkCursor( aIdx.GetNode() );
                        aIdx = nEnd-1;
                    }
                }
                ++aIdx;
                pNode = & aIdx.GetNode();
            }
        }
        rCnt.SetNewContentIdx( nullptr );

        // When deleting a header/footer-format, we ALWAYS need to disable
        // the undo function (Bug 31069)
        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

        OSL_ENSURE( pNode, "A big problem." );
        pDoc->getIDocumentContentOperations().DeleteSection( pNode );
    }
    delete pFormat;
}

void SwFormatFrameSize::ScaleMetrics(tools::Long lMult, tools::Long lDiv) {
    // Don't inherit the SvxSizeItem override (might or might not be relevant; added "just in case"
    // when changing SwFormatFrameSize to derive from SvxSizeItem instead of directly from
    // SfxPoolItem):
    return SfxPoolItem::ScaleMetrics(lMult, lDiv);
}

bool SwFormatFrameSize::HasMetrics() const {
    // Don't inherit the SvxSizeItem override (might or might not be relevant; added "just in case"
    // when changing SwFormatFrameSize to derive from SvxSizeItem instead of directly from
    // SfxPoolItem):
    return SfxPoolItem::HasMetrics();
}

// Partially implemented inline in hxx
SwFormatFrameSize::SwFormatFrameSize( SwFrameSize eSize, SwTwips nWidth, SwTwips nHeight )
    : SvxSizeItem( RES_FRM_SIZE, {nWidth, nHeight} ),
    m_eFrameHeightType( eSize ),
    m_eFrameWidthType( SwFrameSize::Fixed )
{
    m_nWidthPercent = m_eWidthPercentRelation = m_nHeightPercent = m_eHeightPercentRelation = 0;
}

bool SwFormatFrameSize::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return( m_eFrameHeightType  == static_cast<const SwFormatFrameSize&>(rAttr).m_eFrameHeightType &&
            m_eFrameWidthType  == static_cast<const SwFormatFrameSize&>(rAttr).m_eFrameWidthType &&
            SvxSizeItem::operator==(rAttr)&&
            m_nWidthPercent   == static_cast<const SwFormatFrameSize&>(rAttr).GetWidthPercent() &&
            m_eWidthPercentRelation == static_cast<const SwFormatFrameSize&>(rAttr).GetWidthPercentRelation() &&
            m_nHeightPercent  == static_cast<const SwFormatFrameSize&>(rAttr).GetHeightPercent() &&
            m_eHeightPercentRelation == static_cast<const SwFormatFrameSize&>(rAttr).GetHeightPercentRelation() );
}

SwFormatFrameSize* SwFormatFrameSize::Clone( SfxItemPool* ) const
{
    return new SwFormatFrameSize( *this );
}

bool SwFormatFrameSize::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_FRMSIZE_SIZE:
        {
            awt::Size aTmp;
            aTmp.Height = convertTwipToMm100(GetHeight());
            aTmp.Width = convertTwipToMm100(GetWidth());
            rVal <<= aTmp;
        }
        break;
        case MID_FRMSIZE_REL_HEIGHT:
            rVal <<= static_cast<sal_Int16>(GetHeightPercent() != SwFormatFrameSize::SYNCED ? GetHeightPercent() : 0);
        break;
        case MID_FRMSIZE_REL_HEIGHT_RELATION:
            rVal <<= GetHeightPercentRelation();
        break;
        case MID_FRMSIZE_REL_WIDTH:
            rVal <<= static_cast<sal_Int16>(GetWidthPercent() != SwFormatFrameSize::SYNCED ? GetWidthPercent() : 0);
        break;
        case MID_FRMSIZE_REL_WIDTH_RELATION:
            rVal <<= GetWidthPercentRelation();
        break;
        case MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH:
            rVal <<= SwFormatFrameSize::SYNCED == GetHeightPercent();
        break;
        case MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT:
            rVal <<= SwFormatFrameSize::SYNCED == GetWidthPercent();
        break;
        case MID_FRMSIZE_WIDTH :
            rVal <<= static_cast<sal_Int32>(convertTwipToMm100(GetWidth()));
        break;
        case MID_FRMSIZE_HEIGHT:
            // #95848# returned size should never be zero.
            // (there was a bug that allowed for setting height to 0.
            // Thus there some documents existing with that not allowed
            // attribute value which may cause problems on import.)
            rVal <<= static_cast<sal_Int32>(convertTwipToMm100(GetHeight() < MINLAY ? MINLAY : GetHeight() ));
        break;
        case MID_FRMSIZE_SIZE_TYPE:
            rVal <<= static_cast<sal_Int16>(GetHeightSizeType());
        break;
        case MID_FRMSIZE_IS_AUTO_HEIGHT:
            rVal <<= SwFrameSize::Fixed != GetHeightSizeType();
        break;
        case MID_FRMSIZE_WIDTH_TYPE:
            rVal <<= static_cast<sal_Int16>(GetWidthSizeType());
        break;
    }
    return true;
}

bool SwFormatFrameSize::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_FRMSIZE_SIZE:
        {
            awt::Size aVal;
            if(!(rVal >>= aVal))
                bRet = false;
            else
            {
                Size aTmp(aVal.Width, aVal.Height);
                if(bConvert)
                {
                    aTmp.setHeight(o3tl::toTwips(aTmp.Height(), o3tl::Length::mm100));
                    aTmp.setWidth(o3tl::toTwips(aTmp.Width(), o3tl::Length::mm100));
                }
                SetSize(aTmp);
            }
        }
        break;
        case MID_FRMSIZE_REL_HEIGHT:
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0 && nSet < SwFormatFrameSize::SYNCED)
                SetHeightPercent(static_cast<sal_uInt8>(nSet));
            else
                bRet = false;
        }
        break;
        case MID_FRMSIZE_REL_HEIGHT_RELATION:
        {
            sal_Int16 eSet = 0;
            rVal >>= eSet;
            SetHeightPercentRelation(eSet);
        }
        break;
        case MID_FRMSIZE_REL_WIDTH:
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0 && nSet < SwFormatFrameSize::SYNCED)
                SetWidthPercent(static_cast<sal_uInt8>(nSet));
            else
                bRet = false;
        }
        break;
        case MID_FRMSIZE_REL_WIDTH_RELATION:
        {
            sal_Int16 eSet = 0;
            rVal >>= eSet;
            SetWidthPercentRelation(eSet);
        }
        break;
        case MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH:
        {
            bool bSet = *o3tl::doAccess<bool>(rVal);
            if(bSet)
                SetHeightPercent(SwFormatFrameSize::SYNCED);
            else if( SwFormatFrameSize::SYNCED == GetHeightPercent() )
                SetHeightPercent( 0 );
        }
        break;
        case MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT:
        {
            bool bSet = *o3tl::doAccess<bool>(rVal);
            if(bSet)
                SetWidthPercent(SwFormatFrameSize::SYNCED);
            else if( SwFormatFrameSize::SYNCED == GetWidthPercent() )
                SetWidthPercent(0);
        }
        break;
        case MID_FRMSIZE_WIDTH :
        {
            sal_Int32 nWd = 0;
            if(rVal >>= nWd)
            {
                if(bConvert)
                    nWd = o3tl::toTwips(nWd, o3tl::Length::mm100);
                if(nWd < MINLAY)
                   nWd = MINLAY;
                SetWidth(nWd);
            }
            else
                bRet = false;
        }
        break;
        case MID_FRMSIZE_HEIGHT:
        {
            sal_Int32 nHg = 0;
            if(rVal >>= nHg)
            {
                if(bConvert)
                    nHg = o3tl::toTwips(nHg, o3tl::Length::mm100);
                if(nHg < MINLAY)
                    nHg = MINLAY;
                SetHeight(nHg);
            }
            else
                bRet = false;
        }
        break;
        case MID_FRMSIZE_SIZE_TYPE:
        {
            sal_Int16 nType = 0;
            if((rVal >>= nType) && nType >= 0 && nType <= static_cast<int>(SwFrameSize::Minimum) )
            {
                SetHeightSizeType(static_cast<SwFrameSize>(nType));
            }
            else
                bRet = false;
        }
        break;
        case MID_FRMSIZE_IS_AUTO_HEIGHT:
        {
            bool bSet = *o3tl::doAccess<bool>(rVal);
            SetHeightSizeType(bSet ? SwFrameSize::Variable : SwFrameSize::Fixed);
        }
        break;
        case MID_FRMSIZE_WIDTH_TYPE:
        {
            sal_Int16 nType = 0;
            if((rVal >>= nType) && nType >= 0 && nType <= static_cast<int>(SwFrameSize::Minimum) )
            {
                SetWidthSizeType(static_cast<SwFrameSize>(nType));
            }
            else
                bRet = false;
        }
        break;
        default:
            bRet = false;
    }
    return bRet;
}

void SwFormatFrameSize::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatFrameSize"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));

    std::stringstream aSize;
    aSize << GetSize();
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("size"), BAD_CAST(aSize.str().c_str()));

    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eFrameHeightType"), BAD_CAST(OString::number(static_cast<int>(m_eFrameHeightType)).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eFrameWidthType"), BAD_CAST(OString::number(static_cast<int>(m_eFrameWidthType)).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWidthPercent"), BAD_CAST(OString::number(m_nWidthPercent).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eWidthPercentRelation"), BAD_CAST(OString::number(m_eWidthPercentRelation).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nHeightPercent"), BAD_CAST(OString::number(m_nHeightPercent).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eHeightPercentRelation"), BAD_CAST(OString::number(m_eHeightPercentRelation).getStr()));

    (void)xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFormatFillOrder::SwFormatFillOrder( SwFillOrder nFO )
    : SfxEnumItem( RES_FILL_ORDER, nFO )
{}

SwFormatFillOrder* SwFormatFillOrder::Clone( SfxItemPool* ) const
{
    return new SwFormatFillOrder( GetValue() );
}

sal_uInt16  SwFormatFillOrder::GetValueCount() const
{
    return SW_FILL_ORDER_END - SW_FILL_ORDER_BEGIN;
}

// Partially implemented inline in hxx
SwFormatHeader::SwFormatHeader( SwFrameFormat *pHeaderFormat )
    : SfxPoolItem( RES_HEADER ),
    SwClient( pHeaderFormat ),
    m_bActive( pHeaderFormat )
{
}

SwFormatHeader::SwFormatHeader( const SwFormatHeader &rCpy )
    : SfxPoolItem( RES_HEADER ),
    SwClient( const_cast<sw::BroadcastingModify*>(static_cast<const sw::BroadcastingModify*>(rCpy.GetRegisteredIn())) ),
    m_bActive( rCpy.IsActive() )
{
}

SwFormatHeader::SwFormatHeader( bool bOn )
    : SfxPoolItem( RES_HEADER ),
    SwClient( nullptr ),
    m_bActive( bOn )
{
}

 SwFormatHeader::~SwFormatHeader()
{
    if ( GetHeaderFormat() )
        lcl_DelHFFormat( this, GetHeaderFormat() );
}

bool SwFormatHeader::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( GetRegisteredIn() == static_cast<const SwFormatHeader&>(rAttr).GetRegisteredIn() &&
             m_bActive == static_cast<const SwFormatHeader&>(rAttr).IsActive() );
}

SwFormatHeader* SwFormatHeader::Clone( SfxItemPool* ) const
{
    return new SwFormatHeader( *this );
}

void SwFormatHeader::RegisterToFormat( SwFormat& rFormat )
{
    rFormat.Add(*this);
}

// Partially implemented inline in hxx
SwFormatFooter::SwFormatFooter( SwFrameFormat *pFooterFormat )
    : SfxPoolItem( RES_FOOTER ),
    SwClient( pFooterFormat ),
    m_bActive( pFooterFormat )
{
}

SwFormatFooter::SwFormatFooter( const SwFormatFooter &rCpy )
    : SfxPoolItem( RES_FOOTER ),
    SwClient( const_cast<sw::BroadcastingModify*>(static_cast<const sw::BroadcastingModify*>(rCpy.GetRegisteredIn())) ),
    m_bActive( rCpy.IsActive() )
{
}

SwFormatFooter::SwFormatFooter( bool bOn )
    : SfxPoolItem( RES_FOOTER ),
    SwClient( nullptr ),
    m_bActive( bOn )
{
}

 SwFormatFooter::~SwFormatFooter()
{
    if ( GetFooterFormat() )
        lcl_DelHFFormat( this, GetFooterFormat() );
}

void SwFormatFooter::RegisterToFormat( SwFormat& rFormat )
{
    rFormat.Add(*this);
}

bool SwFormatFooter::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( GetRegisteredIn() == static_cast<const SwFormatFooter&>(rAttr).GetRegisteredIn() &&
             m_bActive == static_cast<const SwFormatFooter&>(rAttr).IsActive() );
}

SwFormatFooter* SwFormatFooter::Clone( SfxItemPool* ) const
{
    return new SwFormatFooter( *this );
}

// Partially implemented inline in hxx
SwFormatContent::SwFormatContent( const SwFormatContent &rCpy )
    : SfxPoolItem( RES_CNTNT )
    , m_oStartNode( rCpy.m_oStartNode )
{
    setNonShareable();
}

SwFormatContent::SwFormatContent( const SwStartNode *pStartNd )
    : SfxPoolItem( RES_CNTNT )
{
    setNonShareable();
    if (pStartNd)
        m_oStartNode = *pStartNd;
}

SwFormatContent::~SwFormatContent()
{
}

void SwFormatContent::SetNewContentIdx( const SwNodeIndex *pIdx )
{
    if (pIdx)
        m_oStartNode = *pIdx;
    else
        m_oStartNode.reset();
}

bool SwFormatContent::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return m_oStartNode == static_cast<const SwFormatContent&>(rAttr).m_oStartNode;
}

SwFormatContent* SwFormatContent::Clone( SfxItemPool* ) const
{
    return new SwFormatContent( *this );
}

void SwFormatContent::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatContent"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    if (m_oStartNode)
    {
        (void)xmlTextWriterWriteAttribute(
            pWriter, BAD_CAST("startNode"),
            BAD_CAST(OString::number(sal_Int32(m_oStartNode->GetNode().GetIndex())).getStr()));
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("startNodePtr"), "%p",
                                          &m_oStartNode->GetNode());
    }
    (void)xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFormatPageDesc::SwFormatPageDesc( const SwFormatPageDesc &rCpy )
    : SfxPoolItem( RES_PAGEDESC ),
    SwClient( const_cast<SwPageDesc*>(rCpy.GetPageDesc()) ),
    m_oNumOffset( rCpy.m_oNumOffset ),
    m_pDefinedIn( nullptr )
{
    setNonShareable();
}

SwFormatPageDesc::SwFormatPageDesc( const SwPageDesc *pDesc )
    : SfxPoolItem( RES_PAGEDESC ),
    SwClient( const_cast<SwPageDesc*>(pDesc) ),
    m_pDefinedIn( nullptr )
{
    setNonShareable();
}

SwFormatPageDesc &SwFormatPageDesc::operator=(const SwFormatPageDesc &rCpy)
{
    if (SfxPoolItem::areSame(*this, rCpy))
        return *this;

    if (rCpy.GetPageDesc())
        RegisterToPageDesc(*const_cast<SwPageDesc*>(rCpy.GetPageDesc()));
    m_oNumOffset = rCpy.m_oNumOffset;
    m_pDefinedIn = nullptr;

    return *this;
}

 SwFormatPageDesc::~SwFormatPageDesc() {}

bool SwFormatPageDesc::KnowsPageDesc() const
{
    return (GetRegisteredIn() != nullptr);
}

bool SwFormatPageDesc::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return  ( m_pDefinedIn == static_cast<const SwFormatPageDesc&>(rAttr).m_pDefinedIn ) &&
            ( m_oNumOffset == static_cast<const SwFormatPageDesc&>(rAttr).m_oNumOffset ) &&
            ( GetPageDesc() == static_cast<const SwFormatPageDesc&>(rAttr).GetPageDesc() );
}

SwFormatPageDesc* SwFormatPageDesc::Clone( SfxItemPool* ) const
{
    return new SwFormatPageDesc( *this );
}

void SwFormatPageDesc::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::SwAutoFormatUsedHint)
    {
        if(GetRegisteredIn())
            static_cast<const sw::AutoFormatUsedHint&>(rHint).SetUsed(); //TODO: recheck if this is really the right way to check for use
    }
    else if (const SwPageDescHint* pHint = dynamic_cast<const SwPageDescHint*>(&rHint))
    {
        // mba: shouldn't that be broadcasted also?
        SwFormatPageDesc aDfltDesc(pHint->GetPageDesc());
        SwPageDesc* pDesc = pHint->GetPageDesc();
        const sw::BroadcastingModify* pMod = GetDefinedIn();
        if(pMod)
        {
            if(auto pContentNode = dynamic_cast<const SwContentNode*>(pMod))
                const_cast<SwContentNode*>(pContentNode)->SetAttr(aDfltDesc);
            else if(auto pFormat = dynamic_cast<const SwFormat*>(pMod))
                const_cast<SwFormat*>(pFormat)->SetFormatAttr( aDfltDesc );
            else
            {
                SAL_WARN("sw.core", "SwFormatPageDesc registered at " << typeid(pMod).name() << ".");
                RegisterToPageDesc(*pDesc);
            }
        }
        else
            // there could be an Undo-copy
            RegisterToPageDesc(*pDesc);
    }
    else if (rHint.GetId() == SfxHintId::SwLegacyModify)
    {
        auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
        if(RES_OBJECTDYING == pLegacy->GetWhich())
        {
            m_pDefinedIn = nullptr;
            EndListeningAll();
        }
    }
}

void SwFormatPageDesc::RegisterToPageDesc( SwPageDesc& rDesc )
{
    rDesc.Add(*this);
}

bool SwFormatPageDesc::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool    bRet = true;
    switch ( nMemberId )
    {
        case MID_PAGEDESC_PAGENUMOFFSET:
            {
                ::std::optional<sal_uInt16> oOffset = GetNumOffset();
                if (oOffset)
                {
                    rVal <<= static_cast<sal_Int16>(*oOffset);
                }
                else
                {
                    rVal.clear();
                }
            }
            break;

        case MID_PAGEDESC_PAGEDESCNAME:
            {
                const SwPageDesc* pDesc = GetPageDesc();
                if( pDesc )
                {
                    OUString aString;
                    SwStyleNameMapper::FillProgName(pDesc->GetName(), aString, SwGetPoolIdFromName::PageDesc);
                    rVal <<= aString;
                }
                else
                    rVal.clear();
            }
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

bool SwFormatPageDesc::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_PAGEDESC_PAGENUMOFFSET:
        {
            sal_Int16 nOffset = 0;
            if (!rVal.hasValue())
            {
                SetNumOffset(std::nullopt);
            }
            else if (rVal >>= nOffset)
                SetNumOffset( nOffset );
            else
                bRet = false;
        }
        break;

        case MID_PAGEDESC_PAGEDESCNAME:
            /* Doesn't work, because the attribute doesn't need the name but a
             * pointer to the PageDesc (it's a client of it). The pointer can
             * only be requested from the document using the name.
             */
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

void SwFormatPageDesc::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatPageDesc"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    if (m_oNumOffset)
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("oNumOffset"), BAD_CAST(OString::number(*m_oNumOffset).getStr()));
    else
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("oNumOffset"), BAD_CAST("none"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("pPageDesc"), "%p", GetPageDesc());
    if (const SwPageDesc* pPageDesc = GetPageDesc())
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(pPageDesc->GetName().toUtf8().getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

//  class SwFormatCol
//  Partially implemented inline in hxx

SwColumn::SwColumn() :
    m_nWish ( 0 ),
    m_nLeft ( 0 ),
    m_nRight( 0 )
{
}

bool SwColumn::operator==( const SwColumn &rCmp ) const
{
    return  m_nWish    == rCmp.GetWishWidth() &&
            GetLeft()  == rCmp.GetLeft() &&
            GetRight() == rCmp.GetRight();
}

void SwColumn::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwColumn"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWish"), BAD_CAST(OString::number(m_nWish).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nUpper"), BAD_CAST(OString::number(0).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLower"), BAD_CAST(OString::number(0).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLeft"), BAD_CAST(OString::number(m_nLeft).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nRight"), BAD_CAST(OString::number(m_nRight).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

SwFormatCol::SwFormatCol( const SwFormatCol& rCpy )
    : SfxPoolItem( RES_COL ),
    m_eLineStyle( rCpy.m_eLineStyle ),
    m_nLineWidth( rCpy.m_nLineWidth),
    m_aLineColor( rCpy.m_aLineColor),
    m_nLineHeight( rCpy.GetLineHeight() ),
    m_eAdj( rCpy.GetLineAdj() ),
    m_nWidth( rCpy.GetWishWidth() ),
    m_aWidthAdjustValue( rCpy.m_aWidthAdjustValue ),
    m_bOrtho( rCpy.IsOrtho() )
{
    m_aColumns.reserve(rCpy.GetNumCols());
    for ( sal_uInt16 i = 0; i < rCpy.GetNumCols(); ++i )
    {
        m_aColumns.emplace_back(rCpy.GetColumns()[i] );
    }
}

SwFormatCol::~SwFormatCol() {}

SwFormatCol& SwFormatCol::operator=( const SwFormatCol& rCpy )
{
    if (!SfxPoolItem::areSame(*this, rCpy))
    {
        m_eLineStyle  = rCpy.m_eLineStyle;
        m_nLineWidth  = rCpy.m_nLineWidth;
        m_aLineColor  = rCpy.m_aLineColor;
        m_nLineHeight = rCpy.GetLineHeight();
        m_eAdj        = rCpy.GetLineAdj();
        m_nWidth      = rCpy.GetWishWidth();
        m_aWidthAdjustValue = rCpy.m_aWidthAdjustValue;
        m_bOrtho      = rCpy.IsOrtho();

        m_aColumns.clear();
        for ( sal_uInt16 i = 0; i < rCpy.GetNumCols(); ++i )
        {
            m_aColumns.emplace_back(rCpy.GetColumns()[i] );
        }
    }
    return *this;
}

SwFormatCol::SwFormatCol()
    : SfxPoolItem( RES_COL )
    , m_eLineStyle( SvxBorderLineStyle::NONE)
    ,
    m_nLineWidth(0),
    m_nLineHeight( 100 ),
    m_eAdj( COLADJ_NONE ),
    m_nWidth( USHRT_MAX ),
    m_aWidthAdjustValue( 0 ),
    m_bOrtho( true )
{
}

bool SwFormatCol::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    const SwFormatCol &rCmp = static_cast<const SwFormatCol&>(rAttr);
    if( !(m_eLineStyle        == rCmp.m_eLineStyle  &&
          m_nLineWidth        == rCmp.m_nLineWidth  &&
          m_aLineColor        == rCmp.m_aLineColor  &&
          m_nLineHeight        == rCmp.GetLineHeight() &&
          m_eAdj               == rCmp.GetLineAdj() &&
          m_nWidth             == rCmp.GetWishWidth() &&
          m_bOrtho             == rCmp.IsOrtho() &&
          m_aColumns.size() == rCmp.GetNumCols() &&
          m_aWidthAdjustValue == rCmp.GetAdjustValue()
         ) )
        return false;

    for ( size_t i = 0; i < m_aColumns.size(); ++i )
        if ( !(m_aColumns[i] == rCmp.GetColumns()[i]) )
            return false;

    return true;
}

SwFormatCol* SwFormatCol::Clone( SfxItemPool* ) const
{
    return new SwFormatCol( *this );
}

sal_uInt16 SwFormatCol::GetGutterWidth( bool bMin ) const
{
    sal_uInt16 nRet = 0;
    if ( m_aColumns.size() == 2 )
        nRet = m_aColumns[0].GetRight() + m_aColumns[1].GetLeft();
    else if ( m_aColumns.size() > 2 )
    {
        bool bSet = false;
        for ( size_t i = 1; i+1 < m_aColumns.size(); ++i )
        {
            const sal_uInt16 nTmp = m_aColumns[i].GetRight() + m_aColumns[i+1].GetLeft();
            if ( bSet )
            {
                if ( nTmp != nRet )
                {
                    if ( !bMin )
                        return USHRT_MAX;
                    if ( nRet > nTmp )
                        nRet = nTmp;
                }
            }
            else
            {
                bSet = true;
                nRet = nTmp;
            }
        }
    }
    return nRet;
}

void SwFormatCol::SetGutterWidth( sal_uInt16 nNew, sal_uInt16 nAct )
{
    if ( m_bOrtho )
        Calc( nNew, nAct );
    else
    {
        sal_uInt16 nHalf = nNew / 2;
        for (size_t i = 0; i < m_aColumns.size(); ++i)
        {
            SwColumn &rCol = m_aColumns[i];
            rCol.SetLeft(nHalf);
            rCol.SetRight(nHalf);
            if ( i == 0 )
                rCol.SetLeft(0);
            else if ( i+1 == m_aColumns.size() )
                rCol.SetRight(0);
        }
    }
}

void SwFormatCol::Init( sal_uInt16 nNumCols, sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    // Deleting seems to be a bit radical on the first sight; but otherwise we
    // have to initialize all values of the remaining SwColumns.
    m_aColumns.clear();
    for ( sal_uInt16 i = 0; i < nNumCols; ++i )
    {
        m_aColumns.emplace_back( );
    }
    m_bOrtho = true;
    m_nWidth = USHRT_MAX;
    if( nNumCols )
        Calc( nGutterWidth, nAct );
}

void SwFormatCol::SetOrtho( bool bNew, sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    m_bOrtho = bNew;
    if ( bNew && !m_aColumns.empty() )
        Calc( nGutterWidth, nAct );
}

sal_uInt16 SwFormatCol::CalcColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const
{
    assert(nCol < m_aColumns.size());
    if ( m_nWidth != nAct )
    {
        tools::Long nW = m_aColumns[nCol].GetWishWidth();
        nW *= nAct;
        nW /= m_nWidth;
        return sal_uInt16(nW);
    }
    else
        return m_aColumns[nCol].GetWishWidth();
}

sal_uInt16 SwFormatCol::CalcPrtColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const
{
    assert(nCol < m_aColumns.size());
    sal_uInt16 nRet = CalcColWidth( nCol, nAct );
    const SwColumn *pCol = &m_aColumns[nCol];
    nRet = nRet - pCol->GetLeft();
    nRet = nRet - pCol->GetRight();
    return nRet;
}

void SwFormatCol::Calc( sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    if (!GetNumCols())
        return;

    //First set the column widths with the current width, then calculate the
    //column's requested width using the requested total width.
    const sal_uInt16 nGutterHalf = nGutterWidth ? nGutterWidth / 2 : 0;

    //Width of PrtAreas is totalwidth - spacings / count
    sal_uInt16 nSpacings;
    bool bFail = o3tl::checked_multiply<sal_uInt16>(GetNumCols() - 1, nGutterWidth, nSpacings);
    if (bFail)
    {
        SAL_WARN("sw.core", "SwFormatVertOrient::Calc: overflow");
        return;
    }

    const sal_uInt16 nPrtWidth = (nAct - nSpacings) / GetNumCols();
    sal_uInt16 nAvail = nAct;

    //The first column is PrtWidth + (gap width / 2)
    const sal_uInt16 nLeftWidth = nPrtWidth + nGutterHalf;
    SwColumn &rFirstCol = m_aColumns.front();
    rFirstCol.SetWishWidth(nLeftWidth);
    rFirstCol.SetRight(nGutterHalf);
    rFirstCol.SetLeft(0);
    nAvail = nAvail - nLeftWidth;

    //Column 2 to n-1 is PrtWidth + gap width
    const sal_uInt16 nMidWidth = nPrtWidth + nGutterWidth;

    for (sal_uInt16 i = 1; i < GetNumCols()-1; ++i)
    {
        SwColumn &rCol = m_aColumns[i];
        rCol.SetWishWidth(nMidWidth);
        rCol.SetLeft(nGutterHalf);
        rCol.SetRight(nGutterHalf);
        nAvail = nAvail - nMidWidth;
    }

    //The last column is equivalent to the first one - to compensate rounding
    //errors we add the remaining space of the other columns to the last one.
    SwColumn &rLastCol = m_aColumns.back();
    rLastCol.SetWishWidth(nAvail);
    rLastCol.SetLeft(nGutterHalf);
    rLastCol.SetRight(0);

    assert(nAct != 0);
    //Convert the current width to the requested width.
    for (SwColumn &rCol: m_aColumns)
    {
        tools::Long nTmp = rCol.GetWishWidth();
        nTmp *= GetWishWidth();
        nTmp = nAct == 0 ? nTmp : nTmp / nAct;
        rCol.SetWishWidth(sal_uInt16(nTmp));
    }
}

bool SwFormatCol::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    if(MID_COLUMN_SEPARATOR_LINE == nMemberId)
    {
        OSL_FAIL("not implemented");
    }
    else
    {
        uno::Reference<text::XTextColumns> xCols(SvxXTextColumns_createInstance(),
                                                 css::uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xCols, css::uno::UNO_QUERY_THROW);

        if (GetNumCols() > 0)
        {
            xCols->setColumnCount(GetNumCols());
            const sal_uInt16 nItemGutterWidth = GetGutterWidth();
            sal_Int32 nAutoDistance = IsOrtho() ? USHRT_MAX == nItemGutterWidth
                                                      ? DEF_GUTTER_WIDTH
                                                      : static_cast<sal_Int32>(nItemGutterWidth)
                                                : 0;
            nAutoDistance = convertTwipToMm100(nAutoDistance);
            xProps->setPropertyValue(UNO_NAME_AUTOMATIC_DISTANCE, uno::Any(nAutoDistance));

            if (!IsOrtho())
            {
                auto aTextColumns = xCols->getColumns();
                text::TextColumn* pColumns = aTextColumns.getArray();
                const SwColumns& rCols = GetColumns();
                for (sal_Int32 i = 0; i < aTextColumns.getLength(); ++i)
                {
                    const SwColumn* pCol = &rCols[i];

                    pColumns[i].Width = pCol->GetWishWidth();
                    pColumns[i].LeftMargin = convertTwipToMm100(pCol->GetLeft());
                    pColumns[i].RightMargin = convertTwipToMm100(pCol->GetRight());
                }
                xCols->setColumns(aTextColumns); // sets "IsAutomatic" property to false
            }
        }
        uno::Any aVal;
        aVal <<= o3tl::narrowing<sal_Int32>(
            o3tl::convert(GetLineWidth(), o3tl::Length::twip, o3tl::Length::mm100));
        xProps->setPropertyValue(UNO_NAME_SEPARATOR_LINE_WIDTH, aVal);
        aVal <<= GetLineColor();
        xProps->setPropertyValue(UNO_NAME_SEPARATOR_LINE_COLOR, aVal);
        aVal <<= static_cast<sal_Int32>(GetLineHeight());
        xProps->setPropertyValue(UNO_NAME_SEPARATOR_LINE_RELATIVE_HEIGHT, aVal);
        aVal <<= GetLineAdj() != COLADJ_NONE;
        xProps->setPropertyValue(UNO_NAME_SEPARATOR_LINE_IS_ON, aVal);
        sal_Int16 nStyle;
        switch (GetLineStyle())
        {
            case SvxBorderLineStyle::SOLID:
                nStyle = css::text::ColumnSeparatorStyle::SOLID;
                break;
            case SvxBorderLineStyle::DOTTED:
                nStyle = css::text::ColumnSeparatorStyle::DOTTED;
                break;
            case SvxBorderLineStyle::DASHED:
                nStyle = css::text::ColumnSeparatorStyle::DASHED;
                break;
            case SvxBorderLineStyle::NONE:
            default:
                nStyle = css::text::ColumnSeparatorStyle::NONE;
                break;
        }
        aVal <<= nStyle;
        xProps->setPropertyValue(UNO_NAME_SEPARATOR_LINE_STYLE, aVal);
        style::VerticalAlignment eAlignment;
        switch (GetLineAdj())
        {
            case COLADJ_TOP:
                eAlignment = style::VerticalAlignment_TOP;
                break;
            case COLADJ_BOTTOM:
                eAlignment = style::VerticalAlignment_BOTTOM;
                break;
            case COLADJ_CENTER:
            case COLADJ_NONE:
            default:
                eAlignment = style::VerticalAlignment_MIDDLE;
        }
        aVal <<= eAlignment;
        xProps->setPropertyValue(UNO_NAME_SEPARATOR_LINE_VERTIVAL_ALIGNMENT, aVal);
        rVal <<= xCols;
    }
    return true;
}

bool SwFormatCol::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = false;
    if(MID_COLUMN_SEPARATOR_LINE == nMemberId)
    {
        OSL_FAIL("not implemented");
    }
    else
    {
        uno::Reference< text::XTextColumns > xCols;
        rVal >>= xCols;
        if(xCols.is())
        {
            uno::Sequence<text::TextColumn> aSetColumns = xCols->getColumns();
            const text::TextColumn* pArray = aSetColumns.getConstArray();
            m_aColumns.clear();
            //max count is 64k here - this is something the array can't do
            sal_uInt16 nCount = std::min( o3tl::narrowing<sal_uInt16>(aSetColumns.getLength()),
                                     sal_uInt16(0x3fff) );
            sal_uInt16 nWidthSum = 0;
            // #101224# one column is no column

            if(nCount > 1)
                for(sal_uInt16 i = 0; i < nCount; i++)
                {
                    SwColumn aCol;
                    aCol.SetWishWidth(pArray[i].Width );
                    nWidthSum = nWidthSum + pArray[i].Width;
                    aCol.SetLeft (o3tl::toTwips(pArray[i].LeftMargin, o3tl::Length::mm100));
                    aCol.SetRight(o3tl::toTwips(pArray[i].RightMargin, o3tl::Length::mm100));
                    m_aColumns.insert(m_aColumns.begin() + i, aCol);
                }
            bRet = true;
            m_nWidth = nWidthSum;
            m_bOrtho = false;

            if (uno::Reference<beans::XPropertySet> xProps{ xCols, css::uno::UNO_QUERY })
            {
                xProps->getPropertyValue(UNO_NAME_IS_AUTOMATIC) >>= m_bOrtho;
                xProps->getPropertyValue(UNO_NAME_SEPARATOR_LINE_WIDTH) >>= m_nLineWidth;
                m_nLineWidth = o3tl::toTwips(m_nLineWidth, o3tl::Length::mm100);
                xProps->getPropertyValue(UNO_NAME_SEPARATOR_LINE_COLOR) >>= m_aLineColor;
                if (sal_Int32 nHeight;
                    xProps->getPropertyValue(UNO_NAME_SEPARATOR_LINE_RELATIVE_HEIGHT) >>= nHeight)
                    m_nLineHeight = nHeight;
                switch (xProps->getPropertyValue(UNO_NAME_SEPARATOR_LINE_STYLE).get<sal_Int16>())
                {
                    default:
                    case css::text::ColumnSeparatorStyle::NONE:
                        m_eLineStyle = SvxBorderLineStyle::NONE;
                        break;
                    case css::text::ColumnSeparatorStyle::SOLID:
                        m_eLineStyle = SvxBorderLineStyle::SOLID;
                        break;
                    case css::text::ColumnSeparatorStyle::DOTTED:
                        m_eLineStyle = SvxBorderLineStyle::DOTTED;
                        break;
                    case css::text::ColumnSeparatorStyle::DASHED:
                        m_eLineStyle = SvxBorderLineStyle::DASHED;
                        break;
                }
                if (!xProps->getPropertyValue(UNO_NAME_SEPARATOR_LINE_IS_ON).get<bool>())
                    m_eAdj = COLADJ_NONE;
                else switch (xProps->getPropertyValue(UNO_NAME_SEPARATOR_LINE_VERTIVAL_ALIGNMENT).get<style::VerticalAlignment>())
                {
                    case style::VerticalAlignment_TOP: m_eAdj = COLADJ_TOP;  break;
                    case style::VerticalAlignment_MIDDLE: m_eAdj = COLADJ_CENTER; break;
                    case style::VerticalAlignment_BOTTOM: m_eAdj = COLADJ_BOTTOM; break;
                    default: OSL_ENSURE( false, "unknown alignment" ); break;
                }
            }
        }
    }
    return bRet;
}

void SwFormatCol::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatCol"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eLineStyle"), BAD_CAST(OString::number(static_cast<sal_Int16>(m_eLineStyle)).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLineWidth"), BAD_CAST(OString::number(m_nLineWidth).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("aLineColor"), BAD_CAST(m_aLineColor.AsRGBHexString().toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLineHeight"), BAD_CAST(OString::number(m_nLineHeight).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eAdj"), BAD_CAST(OString::number(m_eAdj).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWidth"), BAD_CAST(OString::number(m_nWidth).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWidthAdjustValue"), BAD_CAST(OString::number(m_aWidthAdjustValue).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bOrtho"), BAD_CAST(OString::boolean(m_bOrtho).getStr()));

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("aColumns"));
    for (const SwColumn& rColumn : m_aColumns)
        rColumn.dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFormatSurround::SwFormatSurround( css::text::WrapTextMode eFly ) :
    SfxEnumItem( RES_SURROUND, eFly )
{
    m_bAnchorOnly = m_bContour = m_bOutside = false;
}

bool SwFormatSurround::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( GetValue() == static_cast<const SwFormatSurround&>(rAttr).GetValue() &&
             m_bAnchorOnly== static_cast<const SwFormatSurround&>(rAttr).m_bAnchorOnly &&
             m_bContour== static_cast<const SwFormatSurround&>(rAttr).m_bContour &&
             m_bOutside== static_cast<const SwFormatSurround&>(rAttr).m_bOutside );
}

SwFormatSurround* SwFormatSurround::Clone( SfxItemPool* ) const
{
    return new SwFormatSurround( *this );
}

sal_uInt16  SwFormatSurround::GetValueCount() const
{
    return 6;
}

bool SwFormatSurround::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_SURROUND_SURROUNDTYPE:
            rVal <<= GetSurround();
            break;
        case MID_SURROUND_ANCHORONLY:
            rVal <<= IsAnchorOnly();
            break;
        case MID_SURROUND_CONTOUR:
            rVal <<= IsContour();
            break;
        case MID_SURROUND_CONTOUROUTSIDE:
            rVal <<= IsOutside();
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

bool SwFormatSurround::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_SURROUND_SURROUNDTYPE:
        {
            css::text::WrapTextMode eVal = static_cast<css::text::WrapTextMode>(SWUnoHelper::GetEnumAsInt32( rVal ));
            if( eVal >= css::text::WrapTextMode_NONE && eVal <= css::text::WrapTextMode_RIGHT )
                SetValue( eVal );
            else {
                //exception
                ;
            }
        }
        break;

        case MID_SURROUND_ANCHORONLY:
            SetAnchorOnly( *o3tl::doAccess<bool>(rVal) );
            break;
        case MID_SURROUND_CONTOUR:
            SetContour( *o3tl::doAccess<bool>(rVal) );
            break;
        case MID_SURROUND_CONTOUROUTSIDE:
            SetOutside( *o3tl::doAccess<bool>(rVal) );
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

void SwFormatSurround::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatSurround"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(static_cast<sal_Int32>(GetValue())).getStr()));

    OUString aPresentation;
    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    GetPresentation(SfxItemPresentation::Nameless, MapUnit::Map100thMM, MapUnit::Map100thMM, aPresentation, aIntlWrapper);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(aPresentation.toUtf8().getStr()));

    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bAnchorOnly"), BAD_CAST(OString::boolean(m_bAnchorOnly).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bContour"), BAD_CAST(OString::boolean(m_bContour).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bOutside"), BAD_CAST(OString::boolean(m_bOutside).getStr()));

    (void)xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFormatVertOrient::SwFormatVertOrient( SwTwips nY, sal_Int16 eVert,
                                  sal_Int16 eRel )
    : SfxPoolItem( RES_VERT_ORIENT ),
    m_nYPos( nY ),
    m_eOrient( eVert ),
    m_eRelation( eRel )
{}

bool SwFormatVertOrient::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( m_nYPos     == static_cast<const SwFormatVertOrient&>(rAttr).m_nYPos &&
             m_eOrient   == static_cast<const SwFormatVertOrient&>(rAttr).m_eOrient &&
             m_eRelation == static_cast<const SwFormatVertOrient&>(rAttr).m_eRelation );
}

SwFormatVertOrient* SwFormatVertOrient::Clone( SfxItemPool* ) const
{
    return new SwFormatVertOrient( *this );
}

bool SwFormatVertOrient::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_VERTORIENT_ORIENT:
        {
            rVal <<= m_eOrient;
        }
        break;
        case MID_VERTORIENT_RELATION:
                rVal <<= m_eRelation;
        break;
        case MID_VERTORIENT_POSITION:
                rVal <<= static_cast<sal_Int32>(convertTwipToMm100(GetPos()));
                break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

bool SwFormatVertOrient::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_VERTORIENT_ORIENT:
        {
            sal_uInt16 nVal = text::VertOrientation::NONE;
            rVal >>= nVal;
            m_eOrient = nVal;
        }
        break;
        case MID_VERTORIENT_RELATION:
        {
            m_eRelation = lcl_IntToRelation(rVal);
        }
        break;
        case MID_VERTORIENT_POSITION:
        {
            sal_Int32 nVal = 0;
            rVal >>= nVal;
            if(bConvert)
                nVal = o3tl::toTwips(nVal, o3tl::Length::mm100);
            SetPos( nVal );
        }
        break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

void SwFormatVertOrient::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatVertOrient"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nYPos"), BAD_CAST(OString::number(m_nYPos).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eOrient"), BAD_CAST(OString::number(m_eOrient).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eRelation"), BAD_CAST(OString::number(m_eRelation).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFormatHoriOrient::SwFormatHoriOrient( SwTwips nX, sal_Int16 eHori,
                              sal_Int16 eRel, bool bPos )
    : SfxPoolItem( RES_HORI_ORIENT ),
    m_nXPos( nX ),
    m_eOrient( eHori ),
    m_eRelation( eRel ),
    m_bPosToggle( bPos )
{}

bool SwFormatHoriOrient::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( m_nXPos == static_cast<const SwFormatHoriOrient&>(rAttr).m_nXPos &&
             m_eOrient == static_cast<const SwFormatHoriOrient&>(rAttr).m_eOrient &&
             m_eRelation == static_cast<const SwFormatHoriOrient&>(rAttr).m_eRelation &&
             m_bPosToggle == static_cast<const SwFormatHoriOrient&>(rAttr).m_bPosToggle );
}

SwFormatHoriOrient* SwFormatHoriOrient::Clone( SfxItemPool* ) const
{
    return new SwFormatHoriOrient( *this );
}

bool SwFormatHoriOrient::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_HORIORIENT_ORIENT:
        {
            rVal <<= m_eOrient;
        }
        break;
        case MID_HORIORIENT_RELATION:
            rVal <<= m_eRelation;
        break;
        case MID_HORIORIENT_POSITION:
                rVal <<= static_cast<sal_Int32>(convertTwipToMm100(GetPos()));
                break;
        case MID_HORIORIENT_PAGETOGGLE:
            rVal <<= IsPosToggle();
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

bool SwFormatHoriOrient::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_HORIORIENT_ORIENT:
        {
            sal_Int16 nVal = text::HoriOrientation::NONE;
            rVal >>= nVal;
            m_eOrient = nVal;
        }
        break;
        case MID_HORIORIENT_RELATION:
        {
            m_eRelation = lcl_IntToRelation(rVal);
        }
        break;
        case MID_HORIORIENT_POSITION:
        {
            sal_Int32 nVal = 0;
            if(!(rVal >>= nVal))
                bRet = false;
            if(bConvert)
                nVal = o3tl::toTwips(nVal, o3tl::Length::mm100);
            SetPos( nVal );
        }
        break;
        case MID_HORIORIENT_PAGETOGGLE:
                SetPosToggle( *o3tl::doAccess<bool>(rVal));
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

void SwFormatHoriOrient::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatHoriOrient"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nXPos"), BAD_CAST(OString::number(m_nXPos).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eOrient"), BAD_CAST(OString::number(m_eOrient).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eRelation"), BAD_CAST(OString::number(m_eRelation).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bPosToggle"), BAD_CAST(OString::boolean(m_bPosToggle).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

SwFormatAnchor::SwFormatAnchor( RndStdIds nRnd, sal_uInt16 nPage )
    : SfxPoolItem( RES_ANCHOR ),
    m_eAnchorId( nRnd ),
    m_nPageNumber( nPage ),
    // OD 2004-05-05 #i28701# - get always new increased order number
    m_nOrder( ++s_nOrderCounter )
{
    setNonShareable();
    assert( m_eAnchorId == RndStdIds::FLY_AT_PARA
        || m_eAnchorId == RndStdIds::FLY_AS_CHAR
        || m_eAnchorId == RndStdIds::FLY_AT_PAGE
        || m_eAnchorId == RndStdIds::FLY_AT_FLY
        || m_eAnchorId == RndStdIds::FLY_AT_CHAR);
    // only FLY_AT_PAGE should have a valid page
    assert( m_eAnchorId == RndStdIds::FLY_AT_PAGE || nPage == 0 );
}

SwFormatAnchor::SwFormatAnchor( const SwFormatAnchor &rCpy )
    : SfxPoolItem( RES_ANCHOR )
    , m_oContentAnchor( rCpy.m_oContentAnchor )
    , m_eAnchorId( rCpy.m_eAnchorId )
    , m_nPageNumber( rCpy.m_nPageNumber )
    // OD 2004-05-05 #i28701# - get always new increased order number
    , m_nOrder( ++s_nOrderCounter )
{
    setNonShareable();
}

SwFormatAnchor::~SwFormatAnchor()
{
}

void SwFormatAnchor::SetAnchor( const SwPosition *pPos )
{
    if (!pPos)
    {
        m_oContentAnchor.reset();
        return;
    }
    // anchor only to paragraphs, or start nodes in case of RndStdIds::FLY_AT_FLY
    // also allow table node, this is used when a table is selected and is converted to a frame by the UI
    assert((RndStdIds::FLY_AT_FLY == m_eAnchorId && pPos->GetNode().GetStartNode())
            || (RndStdIds::FLY_AT_PARA == m_eAnchorId && pPos->GetNode().GetTableNode())
            || pPos->GetNode().GetTextNode());
    // verify that the SwPosition being passed to us is not screwy
    assert(!pPos->nContent.GetContentNode()
            || &pPos->nNode.GetNode() == pPos->nContent.GetContentNode());
    m_oContentAnchor.emplace(*pPos);
    // Flys anchored AT paragraph should not point into the paragraph content
    if ((RndStdIds::FLY_AT_PARA == m_eAnchorId) || (RndStdIds::FLY_AT_FLY == m_eAnchorId))
        m_oContentAnchor->nContent.Assign( nullptr, 0 );
}

SwNode* SwFormatAnchor::GetAnchorNode() const
{
    if (!m_oContentAnchor)
        return nullptr;
    if (auto pCntNd = m_oContentAnchor->nContent.GetContentNode())
        return const_cast<SwContentNode*>(pCntNd);
    return &m_oContentAnchor->nNode.GetNode();
}

SwContentNode* SwFormatAnchor::GetAnchorContentNode() const
{
    SwNode* pAnchorNode = GetAnchorNode();
    if (pAnchorNode)
        return pAnchorNode->GetContentNode();
    return nullptr;
}

sal_Int32 SwFormatAnchor::GetAnchorContentOffset() const
{
    if (!m_oContentAnchor)
        return 0;
    if (m_oContentAnchor->nContent.GetContentNode())
        return m_oContentAnchor->nContent.GetIndex();
    return 0;
}

SwFormatAnchor& SwFormatAnchor::operator=(const SwFormatAnchor& rAnchor)
{
    if (!SfxPoolItem::areSame(*this, rAnchor))
    {
        m_eAnchorId  = rAnchor.m_eAnchorId;
        m_nPageNumber   = rAnchor.m_nPageNumber;
        // OD 2004-05-05 #i28701# - get always new increased order number
        m_nOrder = ++s_nOrderCounter;
        m_oContentAnchor  = rAnchor.m_oContentAnchor;
    }
    return *this;
}

bool SwFormatAnchor::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    SwFormatAnchor const& rFormatAnchor(static_cast<SwFormatAnchor const&>(rAttr));
    // OD 2004-05-05 #i28701# - Note: <mnOrder> hasn't to be considered.
    return ( m_eAnchorId == rFormatAnchor.m_eAnchorId &&
             m_nPageNumber == rFormatAnchor.m_nPageNumber   &&
                // compare anchor: either both do not point into a textnode or
                // both do (valid m_oContentAnchor) and the positions are equal
             (m_oContentAnchor == rFormatAnchor.m_oContentAnchor) );
}

SwFormatAnchor* SwFormatAnchor::Clone( SfxItemPool* ) const
{
    return new SwFormatAnchor( *this );
}

// OD 2004-05-05 #i28701#
sal_uInt32 SwFormatAnchor::s_nOrderCounter = 0;

// OD 2004-05-05 #i28701#

bool SwFormatAnchor::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_ANCHOR_ANCHORTYPE:

            text::TextContentAnchorType eRet;
            switch (m_eAnchorId)
            {
                case  RndStdIds::FLY_AT_CHAR:
                    eRet = text::TextContentAnchorType_AT_CHARACTER;
                    break;
                case  RndStdIds::FLY_AT_PAGE:
                    eRet = text::TextContentAnchorType_AT_PAGE;
                    break;
                case  RndStdIds::FLY_AT_FLY:
                    eRet = text::TextContentAnchorType_AT_FRAME;
                    break;
                case  RndStdIds::FLY_AS_CHAR:
                    eRet = text::TextContentAnchorType_AS_CHARACTER;
                    break;
                //case  RndStdIds::FLY_AT_PARA:
                default:
                    eRet = text::TextContentAnchorType_AT_PARAGRAPH;
            }
            rVal <<= eRet;
        break;
        case MID_ANCHOR_PAGENUM:
            rVal <<= static_cast<sal_Int16>(GetPageNum());
        break;
        case MID_ANCHOR_ANCHORFRAME:
        {
            if (m_oContentAnchor && RndStdIds::FLY_AT_FLY == m_eAnchorId)
            {
                SwFrameFormat* pFormat = m_oContentAnchor->GetNode().GetFlyFormat();
                if(pFormat)
                {
                    uno::Reference<text::XTextFrame> const xRet(
                        SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat));
                    rVal <<= xRet;
                }
            }
        }
        break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

bool SwFormatAnchor::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_ANCHOR_ANCHORTYPE:
        {
            RndStdIds   eAnchor;
            switch( static_cast<text::TextContentAnchorType>(SWUnoHelper::GetEnumAsInt32( rVal )) )
            {
                case  text::TextContentAnchorType_AS_CHARACTER:
                    eAnchor = RndStdIds::FLY_AS_CHAR;
                    break;
                case  text::TextContentAnchorType_AT_PAGE:
                    eAnchor = RndStdIds::FLY_AT_PAGE;
                    if( GetPageNum() > 0 )
                    {
                        // If the anchor type is page and a valid page number
                        // has been set, the content position isn't required
                        // any longer.
                        m_oContentAnchor.reset();
                    }
                    break;
                case  text::TextContentAnchorType_AT_FRAME:
                    eAnchor = RndStdIds::FLY_AT_FLY;
                    break;
                case  text::TextContentAnchorType_AT_CHARACTER:
                    eAnchor = RndStdIds::FLY_AT_CHAR;
                    break;
                case text::TextContentAnchorType_AT_PARAGRAPH:
                    eAnchor = RndStdIds::FLY_AT_PARA;
                    break;
                default:
                    eAnchor = RndStdIds::FLY_AT_PARA; // just to keep some compilers happy
                    assert(false);
            }
            SetType( eAnchor );
        }
        break;
        case MID_ANCHOR_PAGENUM:
        {
            sal_Int16 nVal = 0;
            if((rVal >>= nVal) && nVal > 0)
            {
                if (RndStdIds::FLY_AT_PAGE == m_eAnchorId)
                {
                    SetPageNum( nVal );
                    // If the anchor type is page and a valid page number
                    // is set, the content position has to be deleted to not
                    // confuse the layout (frmtool.cxx). However, if the
                    // anchor type is not page, any content position will
                    // be kept.
                    m_oContentAnchor.reset();
                }
                else
                {
                    assert(false && "cannot set page number on this anchor type");
                    bRet = false;
                }
            }
            else
                bRet = false;
        }
        break;
        case MID_ANCHOR_ANCHORFRAME:
        //no break here!;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

void SwFormatAnchor::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatAnchor"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));

    if (m_oContentAnchor)
    {
        std::stringstream aContentAnchor;
        aContentAnchor << *m_oContentAnchor;
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_pContentAnchor"), BAD_CAST(aContentAnchor.str().c_str()));
    }
    else
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_pContentAnchor"), BAD_CAST("(nil)"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_eAnchorType"), BAD_CAST(OString::number(static_cast<int>(m_eAnchorId)).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_nPageNumber"), BAD_CAST(OString::number(m_nPageNumber).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_nOrder"), BAD_CAST(OString::number(m_nOrder).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("s_nOrderCounter"), BAD_CAST(OString::number(s_nOrderCounter).getStr()));
    OUString aPresentation;
    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    GetPresentation(SfxItemPresentation::Nameless, MapUnit::Map100thMM, MapUnit::Map100thMM, aPresentation, aIntlWrapper);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(aPresentation.toUtf8().getStr()));

    (void)xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFormatURL::SwFormatURL() :
    SfxPoolItem( RES_URL ),
    m_bIsServerMap( false )
{
}

SwFormatURL::SwFormatURL( const SwFormatURL &rURL) :
    SfxPoolItem( RES_URL ),
    m_sTargetFrameName( rURL.GetTargetFrameName() ),
    m_sURL( rURL.GetURL() ),
    m_sName( rURL.GetName() ),
    m_bIsServerMap( rURL.IsServerMap() )
{
    if (rURL.GetMap())
        m_pMap.reset( new ImageMap( *rURL.GetMap() ) );
}

SwFormatURL::~SwFormatURL()
{
}

bool SwFormatURL::operator==( const SfxPoolItem &rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    const SwFormatURL &rCmp = static_cast<const SwFormatURL&>(rAttr);
    bool bRet = m_bIsServerMap     == rCmp.IsServerMap() &&
                m_sURL             == rCmp.GetURL() &&
                m_sTargetFrameName == rCmp.GetTargetFrameName() &&
                m_sName            == rCmp.GetName();
    if ( bRet )
    {
        if ( m_pMap && rCmp.GetMap() )
            bRet = *m_pMap == *rCmp.GetMap();
        else
            bRet = m_pMap.get() == rCmp.GetMap();
    }
    return bRet;
}

SwFormatURL* SwFormatURL::Clone( SfxItemPool* ) const
{
    return new SwFormatURL( *this );
}

void SwFormatURL::SetURL(const OUString &rURL, bool bServerMap)
{
    m_sURL = rURL;
    m_bIsServerMap = bServerMap;
}

void SwFormatURL::SetMap( const ImageMap *pM )
{
    m_pMap.reset( pM ? new ImageMap( *pM ) : nullptr);
}

bool SwFormatURL::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_URL_URL:
            rVal <<= GetURL();
        break;
        case MID_URL_TARGET:
            rVal <<= GetTargetFrameName();
        break;
        case MID_URL_HYPERLINKNAME:
            rVal <<= GetName();
            break;
        case MID_URL_CLIENTMAP:
        {
            uno::Reference< uno::XInterface > xInt;
            if(m_pMap)
            {
                xInt = SvUnoImageMap_createInstance( *m_pMap, sw_GetSupportedMacroItems() );
            }
            else
            {
                ImageMap aEmptyMap;
                xInt = SvUnoImageMap_createInstance( aEmptyMap, sw_GetSupportedMacroItems() );
            }
            uno::Reference< container::XIndexContainer > xCont(xInt, uno::UNO_QUERY);
            rVal <<= xCont;
        }
        break;
        case MID_URL_SERVERMAP:
            rVal <<= IsServerMap();
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

bool SwFormatURL::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_URL_URL:
        {
            OUString sTmp;
            rVal >>= sTmp;
            SetURL( sTmp, m_bIsServerMap );
        }
        break;
        case MID_URL_TARGET:
        {
            OUString sTmp;
            rVal >>= sTmp;
            SetTargetFrameName( sTmp );
        }
        break;
        case MID_URL_HYPERLINKNAME:
        {
            OUString sTmp;
            rVal >>= sTmp;
            SetName( sTmp );
        }
        break;
        case MID_URL_CLIENTMAP:
        {
            uno::Reference<container::XIndexContainer> xCont;
            if(!rVal.hasValue())
                m_pMap.reset();
            else if(rVal >>= xCont)
            {
                if(!m_pMap)
                    m_pMap.reset(new ImageMap);
                bRet = SvUnoImageMap_fillImageMap( xCont, *m_pMap );
            }
            else
                bRet = false;
        }
        break;
        case MID_URL_SERVERMAP:
            m_bIsServerMap = *o3tl::doAccess<bool>(rVal);
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

SwFormatEditInReadonly* SwFormatEditInReadonly::Clone( SfxItemPool* ) const
{
    return new SwFormatEditInReadonly( *this );
}

SwFormatLayoutSplit* SwFormatLayoutSplit::Clone( SfxItemPool* ) const
{
    return new SwFormatLayoutSplit( *this );
}

SwFormatRowSplit* SwFormatRowSplit::Clone( SfxItemPool* ) const
{
    return new SwFormatRowSplit( *this );
}

SwFormatNoBalancedColumns* SwFormatNoBalancedColumns::Clone( SfxItemPool* ) const
{
    return new SwFormatNoBalancedColumns( *this );
}

void SwFormatNoBalancedColumns::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatNoBalancedColumns"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::boolean(GetValue()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

// class SwFormatFootnoteEndAtTextEnd

sal_uInt16 SwFormatFootnoteEndAtTextEnd::GetValueCount() const
{
    return sal_uInt16( FTNEND_ATTXTEND_END );
}

SwFormatFootnoteEndAtTextEnd& SwFormatFootnoteEndAtTextEnd::operator=(
                        const SwFormatFootnoteEndAtTextEnd& rAttr )
{
    SfxEnumItem::SetValue( rAttr.GetValue() );
    m_aFormat = rAttr.m_aFormat;
    m_nOffset = rAttr.m_nOffset;
    m_sPrefix = rAttr.m_sPrefix;
    m_sSuffix = rAttr.m_sSuffix;
    return *this;
}

bool SwFormatFootnoteEndAtTextEnd::operator==( const SfxPoolItem& rItem ) const
{
    const SwFormatFootnoteEndAtTextEnd& rAttr = static_cast<const SwFormatFootnoteEndAtTextEnd&>(rItem);
    return SfxEnumItem::operator==( rItem ) &&
            m_aFormat.GetNumberingType() == rAttr.m_aFormat.GetNumberingType() &&
            m_nOffset == rAttr.m_nOffset &&
            m_sPrefix == rAttr.m_sPrefix &&
            m_sSuffix == rAttr.m_sSuffix;
}

bool SwFormatFootnoteEndAtTextEnd::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_COLLECT     :
            rVal <<= GetValue() >= FTNEND_ATTXTEND;
        break;
        case MID_RESTART_NUM :
            rVal <<= GetValue() >= FTNEND_ATTXTEND_OWNNUMSEQ;
        break;
        case MID_NUM_START_AT: rVal <<= static_cast<sal_Int16>(m_nOffset); break;
        case MID_OWN_NUM     :
            rVal <<= GetValue() >= FTNEND_ATTXTEND_OWNNUMANDFMT;
        break;
        case MID_NUM_TYPE    : rVal <<= static_cast<sal_Int16>(m_aFormat.GetNumberingType()); break;
        case MID_PREFIX      : rVal <<= m_sPrefix; break;
        case MID_SUFFIX      : rVal <<= m_sSuffix; break;
        default: return false;
    }
    return true;
}

bool SwFormatFootnoteEndAtTextEnd::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bRet = true;
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_COLLECT     :
        {
            bool bVal = *o3tl::doAccess<bool>(rVal);
            if(!bVal && GetValue() >= FTNEND_ATTXTEND)
                SetValue(FTNEND_ATPGORDOCEND);
            else if(bVal && GetValue() < FTNEND_ATTXTEND)
                SetValue(FTNEND_ATTXTEND);
        }
        break;
        case MID_RESTART_NUM :
        {
            bool bVal = *o3tl::doAccess<bool>(rVal);
            if(!bVal && GetValue() >= FTNEND_ATTXTEND_OWNNUMSEQ)
                SetValue(FTNEND_ATTXTEND);
            else if(bVal && GetValue() < FTNEND_ATTXTEND_OWNNUMSEQ)
                SetValue(FTNEND_ATTXTEND_OWNNUMSEQ);
        }
        break;
        case MID_NUM_START_AT:
        {
            sal_Int16 nVal = 0;
            rVal >>= nVal;
            if(nVal >= 0)
                m_nOffset = nVal;
            else
                bRet = false;
        }
        break;
        case MID_OWN_NUM     :
        {
            bool bVal = *o3tl::doAccess<bool>(rVal);
            if(!bVal && GetValue() >= FTNEND_ATTXTEND_OWNNUMANDFMT)
                SetValue(FTNEND_ATTXTEND_OWNNUMSEQ);
            else if(bVal && GetValue() < FTNEND_ATTXTEND_OWNNUMANDFMT)
                SetValue(FTNEND_ATTXTEND_OWNNUMANDFMT);
        }
        break;
        case MID_NUM_TYPE    :
        {
            sal_Int16 nVal = 0;
            rVal >>= nVal;
            if(nVal >= 0 &&
                (nVal <= SVX_NUM_ARABIC ||
                    SVX_NUM_CHARS_UPPER_LETTER_N == nVal ||
                        SVX_NUM_CHARS_LOWER_LETTER_N == nVal ))
                m_aFormat.SetNumberingType(static_cast<SvxNumType>(nVal));
            else
                bRet = false;
        }
        break;
        case MID_PREFIX      :
        {
            OUString sVal; rVal >>= sVal;
            m_sPrefix = sVal;
        }
        break;
        case MID_SUFFIX      :
        {
            OUString sVal; rVal >>= sVal;
            m_sSuffix = sVal;
        }
        break;
        default: bRet = false;
    }
    return bRet;
}

// class SwFormatFootnoteAtTextEnd

SwFormatFootnoteAtTextEnd* SwFormatFootnoteAtTextEnd::Clone( SfxItemPool* ) const
{
    return new SwFormatFootnoteAtTextEnd(*this);
}

// class SwFormatEndAtTextEnd

SwFormatEndAtTextEnd* SwFormatEndAtTextEnd::Clone( SfxItemPool* ) const
{
    return new SwFormatEndAtTextEnd(*this);
}

//class SwFormatChain

bool SwFormatChain::operator==( const SfxPoolItem &rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return GetPrev() == static_cast<const SwFormatChain&>(rAttr).GetPrev() &&
           GetNext() == static_cast<const SwFormatChain&>(rAttr).GetNext();
}

SwFormatChain::SwFormatChain( const SwFormatChain &rCpy ) :
    SfxPoolItem( RES_CHAIN )
{
    setNonShareable();
    SetPrev( rCpy.GetPrev() );
    SetNext( rCpy.GetNext() );
}

SwFormatChain* SwFormatChain::Clone( SfxItemPool* ) const
{
    SwFormatChain *pRet = new SwFormatChain;
    pRet->SetPrev( GetPrev() );
    pRet->SetNext( GetNext() );
    return pRet;
}

void SwFormatChain::SetPrev( SwFlyFrameFormat *pFormat )
{
    if ( pFormat )
        pFormat->Add(m_aPrev);
    else
        m_aPrev.EndListeningAll();
}

void SwFormatChain::SetNext( SwFlyFrameFormat *pFormat )
{
    if ( pFormat )
        pFormat->Add(m_aNext);
    else
        m_aNext.EndListeningAll();
}

bool SwFormatChain::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool   bRet = true;
    OUString aRet;
    switch ( nMemberId )
    {
        case MID_CHAIN_PREVNAME:
            if ( GetPrev() )
                aRet = GetPrev()->GetName();
            break;
        case MID_CHAIN_NEXTNAME:
            if ( GetNext() )
                aRet = GetNext()->GetName();
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    rVal <<= aRet;
    return bRet;
}

SwFormatLineNumber::SwFormatLineNumber() :
    SfxPoolItem( RES_LINENUMBER )
{
    m_nStartValue = 0;
    m_bCountLines = true;
}

SwFormatLineNumber::~SwFormatLineNumber()
{
}

bool SwFormatLineNumber::operator==( const SfxPoolItem &rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return m_nStartValue  == static_cast<const SwFormatLineNumber&>(rAttr).GetStartValue() &&
           m_bCountLines  == static_cast<const SwFormatLineNumber&>(rAttr).IsCount();
}

SwFormatLineNumber* SwFormatLineNumber::Clone( SfxItemPool* ) const
{
    return new SwFormatLineNumber( *this );
}

bool SwFormatLineNumber::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_LINENUMBER_COUNT:
            rVal <<= IsCount();
            break;
        case MID_LINENUMBER_STARTVALUE:
            rVal <<= static_cast<sal_Int32>(GetStartValue());
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

bool SwFormatLineNumber::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_LINENUMBER_COUNT:
            SetCountLines( *o3tl::doAccess<bool>(rVal) );
            break;
        case MID_LINENUMBER_STARTVALUE:
        {
            sal_Int32 nVal = 0;
            if(rVal >>= nVal)
                SetStartValue( nVal );
            else
                bRet = false;
        }
        break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

SwTextGridItem::SwTextGridItem()
    : SfxPoolItem( RES_TEXTGRID ), m_aColor( COL_LIGHTGRAY ), m_nLines( 20 )
    , m_nBaseHeight( 400 ), m_nRubyHeight( 200 ), m_eGridType( GRID_NONE )
    , m_bRubyTextBelow( false ), m_bPrintGrid( true ), m_bDisplayGrid( true )
    , m_nBaseWidth(400), m_bSnapToChars( true ), m_bSquaredMode(true)
{
}

SwTextGridItem::~SwTextGridItem()
{
}

bool SwTextGridItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    SwTextGridItem const& rOther(static_cast<SwTextGridItem const&>(rAttr));
    return m_eGridType == rOther.GetGridType()
        && m_nLines == rOther.GetLines()
        && m_nBaseHeight == rOther.GetBaseHeight()
        && m_nRubyHeight == rOther.GetRubyHeight()
        && m_bRubyTextBelow == rOther.GetRubyTextBelow()
        && m_bDisplayGrid == rOther.GetDisplayGrid()
        && m_bPrintGrid == rOther.GetPrintGrid()
        && m_aColor == rOther.GetColor()
        && m_nBaseWidth == rOther.GetBaseWidth()
        && m_bSnapToChars == rOther.GetSnapToChars()
        && m_bSquaredMode == rOther.GetSquaredMode();
}

SwTextGridItem* SwTextGridItem::Clone( SfxItemPool* ) const
{
    return new SwTextGridItem( *this );
}

bool SwTextGridItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bRet = true;

    switch( nMemberId & ~CONVERT_TWIPS )
    {
        case MID_GRID_COLOR:
            rVal <<= GetColor();
            break;
        case MID_GRID_LINES:
            rVal <<= GetLines();
            break;
        case MID_GRID_RUBY_BELOW:
            rVal <<= m_bRubyTextBelow;
            break;
        case MID_GRID_PRINT:
            rVal <<= m_bPrintGrid;
            break;
        case MID_GRID_DISPLAY:
            rVal <<= m_bDisplayGrid;
            break;
        case MID_GRID_BASEHEIGHT:
            OSL_ENSURE( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            rVal <<= static_cast<sal_Int32>(convertTwipToMm100(m_nBaseHeight));
            break;
        case MID_GRID_BASEWIDTH:
            OSL_ENSURE( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            rVal <<= static_cast<sal_Int32>(convertTwipToMm100(m_nBaseWidth));
            break;
        case MID_GRID_RUBYHEIGHT:
            OSL_ENSURE( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            rVal <<= static_cast<sal_Int32>(convertTwipToMm100(m_nRubyHeight));
            break;
        case MID_GRID_TYPE:
            switch( GetGridType() )
            {
                case GRID_NONE:
                    rVal <<= text::TextGridMode::NONE;
                    break;
                case GRID_LINES_ONLY:
                    rVal <<= text::TextGridMode::LINES;
                    break;
                case GRID_LINES_CHARS:
                    rVal <<= text::TextGridMode::LINES_AND_CHARS;
                    break;
                default:
                    OSL_FAIL("unknown SwTextGrid value");
                    bRet = false;
                    break;
            }
            break;
        case MID_GRID_SNAPTOCHARS:
            rVal <<= m_bSnapToChars;
            break;
        case MID_GRID_STANDARD_MODE:
            rVal <<= !m_bSquaredMode;
            break;
        default:
            OSL_FAIL("Unknown SwTextGridItem member");
            bRet = false;
            break;
    }

    return bRet;
}

bool SwTextGridItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bRet = true;
    switch( nMemberId & ~CONVERT_TWIPS )
    {
        case MID_GRID_COLOR:
        {
            Color nTmp;
            bRet = (rVal >>= nTmp);
            if( bRet )
                SetColor( nTmp );
        }
        break;
        case MID_GRID_LINES:
        {
            sal_Int16 nTmp = 0;
            bRet = (rVal >>= nTmp);
            if( bRet && (nTmp >= 0) )
                SetLines( nTmp );
            else
                bRet = false;
        }
        break;
        case MID_GRID_RUBY_BELOW:
            SetRubyTextBelow( *o3tl::doAccess<bool>(rVal) );
            break;
        case MID_GRID_PRINT:
            SetPrintGrid( *o3tl::doAccess<bool>(rVal) );
            break;
        case MID_GRID_DISPLAY:
            SetDisplayGrid( *o3tl::doAccess<bool>(rVal) );
            break;
        case MID_GRID_BASEHEIGHT:
        case MID_GRID_BASEWIDTH:
        case MID_GRID_RUBYHEIGHT:
        {
            OSL_ENSURE( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            sal_Int32 nTmp = 0;
            bRet = (rVal >>= nTmp);
            nTmp = o3tl::toTwips(nTmp, o3tl::Length::mm100);
            if( bRet && (nTmp >= 0) && ( nTmp <= SAL_MAX_UINT16) )
            {
                // rhbz#1043551 round up to 5pt -- 0 causes divide-by-zero
                // in layout; 1pt ties the painting code up in knots for
                // minutes with bazillion lines...
#define MIN_TEXTGRID_SIZE 100
                if( (nMemberId & ~CONVERT_TWIPS) == MID_GRID_BASEHEIGHT )
                {
                    nTmp = std::max<sal_Int32>(nTmp, MIN_TEXTGRID_SIZE);
                    SetBaseHeight( o3tl::narrowing<sal_uInt16>(nTmp) );
                }
                else if( (nMemberId & ~CONVERT_TWIPS) == MID_GRID_BASEWIDTH )
                {
                    nTmp = std::max<sal_Int32>(nTmp, MIN_TEXTGRID_SIZE);
                    SetBaseWidth( o3tl::narrowing<sal_uInt16>(nTmp) );
                }
                else
                    SetRubyHeight( o3tl::narrowing<sal_uInt16>(nTmp) );
            }
            else
                bRet = false;
        }
        break;
        case MID_GRID_TYPE:
        {
            sal_Int16 nTmp = 0;
            bRet = (rVal >>= nTmp);
            if( bRet )
            {
                switch( nTmp )
                {
                    case text::TextGridMode::NONE:
                        SetGridType( GRID_NONE );
                        break;
                    case text::TextGridMode::LINES:
                        SetGridType( GRID_LINES_ONLY );
                        break;
                    case text::TextGridMode::LINES_AND_CHARS:
                        SetGridType( GRID_LINES_CHARS );
                        break;
                    default:
                        bRet = false;
                        break;
                }
            }
            break;
        }
        case MID_GRID_SNAPTOCHARS:
            SetSnapToChars( *o3tl::doAccess<bool>(rVal) );
            break;
        case MID_GRID_STANDARD_MODE:
        {
            bool bStandard = *o3tl::doAccess<bool>(rVal);
            SetSquaredMode( !bStandard );
            break;
        }
        default:
            OSL_FAIL("Unknown SwTextGridItem member");
            bRet = false;
    }

    return bRet;
}

void SwTextGridItem::SwitchPaperMode(bool bNew)
{
    if (bNew == m_bSquaredMode)
    {
        //same paper mode, not switch
        return;
    }

    // use default value when grid is disable
    if (m_eGridType == GRID_NONE)
    {
        m_bSquaredMode = bNew;
        Init();
        return;
    }

    if (m_bSquaredMode)
    {
        //switch from "squared mode" to "standard mode"
        m_nBaseWidth = m_nBaseHeight;
        m_nBaseHeight = m_nBaseHeight + m_nRubyHeight;
        m_nRubyHeight = 0;
    }
    else
    {
        //switch from "standard mode" to "squared mode"
        m_nRubyHeight = m_nBaseHeight/3;
        m_nBaseHeight = m_nBaseHeight - m_nRubyHeight;
        m_nBaseWidth = m_nBaseHeight;
    }
    m_bSquaredMode = !m_bSquaredMode;
}

void SwTextGridItem::Init()
{
    if (m_bSquaredMode)
    {
        m_nLines = 20;
        m_nBaseHeight = 400;
        m_nRubyHeight = 200;
        m_eGridType = GRID_NONE;
        m_bRubyTextBelow = false;
        m_bPrintGrid = true;
        m_bDisplayGrid = true;
        m_bSnapToChars = true;
        m_nBaseWidth = 400;
    }
    else
    {
        m_nLines = 44;
        m_nBaseHeight = 312;
        m_nRubyHeight = 0;
        m_eGridType = GRID_NONE;
        m_bRubyTextBelow = false;
        m_bPrintGrid = true;
        m_bDisplayGrid = true;
        m_nBaseWidth = 210;
        m_bSnapToChars = true;
    }
}

SwHeaderAndFooterEatSpacingItem* SwHeaderAndFooterEatSpacingItem::Clone( SfxItemPool* ) const
{
    return new SwHeaderAndFooterEatSpacingItem( Which(), GetValue() );
}

SwFrameFormat::SwFrameFormat(
    SwAttrPool& rPool,
    const char* pFormatNm,
    SwFrameFormat *pDrvdFrame,
    sal_uInt16 nFormatWhich,
    const WhichRangesContainer& pWhichRange)
:   SwFormat(rPool, pFormatNm, pWhichRange, pDrvdFrame, nFormatWhich),
    m_ffList(nullptr)
{
}

SwFrameFormat::SwFrameFormat(
    SwAttrPool& rPool,
    const OUString &rFormatNm,
    SwFrameFormat *pDrvdFrame,
    sal_uInt16 nFormatWhich,
    const WhichRangesContainer& pWhichRange)
:   SwFormat(rPool, rFormatNm, pWhichRange, pDrvdFrame, nFormatWhich),
    m_ffList(nullptr)
{
}

SwFrameFormat::~SwFrameFormat()
{
    if( !GetDoc()->IsInDtor())
    {
        const SwFormatAnchor& rAnchor = GetAnchor();
        if (SwNode* pAnchorNode = rAnchor.GetAnchorNode())
        {
            pAnchorNode->RemoveAnchoredFly(this);
        }
    }

    // Check if there any textboxes attached to this format.
    if( nullptr == m_pOtherTextBoxFormats )
        return;

    // This is a fly-frame-format just delete this
    // textbox entry from the textbox collection.
    // Note: Do not delete it from the doc, as that
    // is already in progress.
    if (Which() == RES_FLYFRMFMT)
        m_pOtherTextBoxFormats->DelTextBox(this);

    // This is a draw-frame-format what belongs to
    // a shape with textbox(es). Delete all of them.
    if (Which() == RES_DRAWFRMFMT)
        m_pOtherTextBoxFormats->ClearAll();

    // Release the pointer.
    m_pOtherTextBoxFormats.reset();
}

void SwFrameFormat::SetFormatName( const OUString& rNewName, bool bBroadcast )
{
    if (m_ffList != nullptr) {
        SAL_INFO_IF(m_aFormatName == rNewName, "sw.core", "SwFrmFmt not really renamed, as both names are equal");
        sw::NameChanged aHint(m_aFormatName, rNewName);
        m_ffList->Rename(*this, rNewName);
        if (bBroadcast) {
            GetNotifier().Broadcast(aHint);
        }

        // update accessibility sidebar object name if we modify the object name on the navigator bar
        const bool bUpdateA11yName = !aHint.m_sOld.isEmpty() && aHint.m_sOld != aHint.m_sNew;
        if (!bUpdateA11yName)
            return;
        SwFlyFrame* pSFly = SwIterator<SwFlyFrame, SwFormat>(*this).First();
        if (!pSFly)
            return;
        SwFrame *pSFlyLower = pSFly->Lower();
        if (!pSFlyLower)
            return;
        if (!pSFlyLower->IsNoTextFrame())
        {
            SwContentFrame* pContent = pSFly->ContainsContent();
            if (SwTextNode* pSwTxtNode = pContent ? static_cast<SwTextFrame*>(pContent)->GetTextNodeFirst() : nullptr)
                pSwTxtNode->resetAndQueueAccessibilityCheck(true);
        }
        else
        {
            if (SwNode* pSwNode = static_cast<SwNoTextFrame*>(pSFlyLower)->GetNode())
                pSwNode->resetAndQueueAccessibilityCheck(true);
        }
    }
    else
        SwFormat::SetFormatName( rNewName, bBroadcast );
}

bool SwFrameFormat::supportsFullDrawingLayerFillAttributeSet() const
{
    return true;
}

void SwFrameFormat::SwClientNotify(const SwModify& rMod, const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
    const sal_uInt16 nNewWhich = pLegacy->m_pNew ? pLegacy->m_pNew->Which() : 0;
    const SwAttrSetChg* pNewAttrSetChg = nullptr;
    const SwFormatHeader* pH = nullptr;
    const SwFormatFooter* pF = nullptr;
    SwNode* pNewAnchorNode = nullptr;
    switch(nNewWhich)
    {
        case RES_ATTRSET_CHG:
        {
            pNewAttrSetChg = static_cast<const SwAttrSetChg*>(pLegacy->m_pNew);
            pH = pNewAttrSetChg->GetChgSet()->GetItem(RES_HEADER, false);
            pF = pNewAttrSetChg->GetChgSet()->GetItem(RES_FOOTER, false);

            // reset fill information
            if(maFillAttributes && supportsFullDrawingLayerFillAttributeSet())
            {
                SfxItemIter aIter(*pNewAttrSetChg->GetChgSet());
                for(const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
                {
                    if(!IsInvalidItem(pItem) && pItem->Which() >= XATTR_FILL_FIRST && pItem->Which() <= XATTR_FILL_LAST)
                    {
                        maFillAttributes.reset();
                        break;
                    }
                }
            }
            const SwFormatAnchor* pAnchor = pNewAttrSetChg->GetChgSet()->GetItem(RES_ANCHOR, false);
            if(pAnchor)
            {
                pNewAnchorNode = pAnchor->GetAnchorNode();
                assert(pNewAnchorNode == nullptr || // style's set must not contain position!
                        pNewAttrSetChg->GetTheChgdSet() == &m_aSet);
            }
            break;
        }
        case RES_FMT_CHG:
        {
            // reset fill information on format change (e.g. style changed)
            if(maFillAttributes && supportsFullDrawingLayerFillAttributeSet())
                maFillAttributes.reset();
            break;
        }
        case RES_HEADER:
            pH = static_cast<const SwFormatHeader*>(pLegacy->m_pNew);
            break;
        case RES_FOOTER:
            pF = static_cast<const SwFormatFooter*>(pLegacy->m_pNew);
            break;
        case RES_ANCHOR:
            pNewAnchorNode = static_cast<const SwFormatAnchor*>(pLegacy->m_pNew)->GetAnchorNode();
            break;
    }
    const sal_uInt16 nOldWhich = pLegacy->m_pOld ? pLegacy->m_pOld->Which() : 0;
    SwNode* pOldAnchorNode = nullptr;
    switch(nOldWhich)
    {
        case RES_ATTRSET_CHG:
        {
            const SwAttrSetChg* pOldAttrSetChg = nullptr;
            pOldAttrSetChg = static_cast<const SwAttrSetChg*>(pLegacy->m_pOld);
            const SwFormatAnchor* pAnchor = pOldAttrSetChg->GetChgSet()->GetItem(RES_ANCHOR, false);
            if(pAnchor)
            {
                pOldAnchorNode = pAnchor->GetAnchorNode();
                assert(pOldAnchorNode == nullptr || // style's set must not contain position!
                        pOldAttrSetChg->GetTheChgdSet() == &m_aSet);
            }
            break;
        }
        case RES_ANCHOR:
            pOldAnchorNode = static_cast<const SwFormatAnchor*>(pLegacy->m_pOld)->GetAnchorNode();
            break;
        case RES_REMOVE_UNO_OBJECT:
            SetXObject(nullptr);
            break;
    }

    assert(nOldWhich == nNewWhich || !nOldWhich || !nNewWhich);
    if(pH && pH->IsActive() && !pH->GetHeaderFormat())
    {   //If he doesn't have one, I'll add one
        SwFrameFormat* pFormat = GetDoc()->getIDocumentLayoutAccess().MakeLayoutFormat(RndStdIds::HEADER, nullptr);
        const_cast<SwFormatHeader*>(pH)->RegisterToFormat(*pFormat);
    }
    if(pF && pF->IsActive() && !pF->GetFooterFormat())
    {   //If he doesn't have one, I'll add one
        SwFrameFormat* pFormat = GetDoc()->getIDocumentLayoutAccess().MakeLayoutFormat(RndStdIds::FOOTER, nullptr);
        const_cast<SwFormatFooter*>(pF)->RegisterToFormat(*pFormat);
    }
    SwFormat::SwClientNotify(rMod, rHint);
    if(pOldAnchorNode != nullptr && (pNewAnchorNode == nullptr || pOldAnchorNode->GetIndex() != pNewAnchorNode->GetIndex()))
        pOldAnchorNode->RemoveAnchoredFly(this);
    if(pNewAnchorNode != nullptr && (pOldAnchorNode == nullptr || pOldAnchorNode->GetIndex() != pNewAnchorNode->GetIndex()))
        pNewAnchorNode->AddAnchoredFly(this);
}

void SwFrameFormat::RegisterToFormat( SwFormat& rFormat )
{
    rFormat.Add(*this);
}

/// Delete all Frames that are registered in aDepend.
void SwFrameFormat::DelFrames()
{
    SwIterator<SwFrame,SwFormat> aIter( *this );
    SwFrame * pLast = aIter.First();
    if( pLast )
        do {
                pLast->Cut();
                SwFrame::DestroyFrame(pLast);
        } while( nullptr != ( pLast = aIter.Next() ));
}

void SwFrameFormat::MakeFrames()
{
    assert(false); // unimplemented in base class
}

SwRect SwFrameFormat::FindLayoutRect( const bool bPrtArea, const Point* pPoint ) const
{
    SwRect aRet;
    SwFrame *pFrame = nullptr;
    if( auto pSectionFormat = dynamic_cast<const SwSectionFormat*>( this ))
    {
        // get the Frame using Node2Layout
        const SwSectionNode* pSectNd = pSectionFormat->GetSectionNode();
        if( pSectNd )
        {
            SwNode2Layout aTmp( *pSectNd, pSectNd->GetIndex() - 1 );
            pFrame = aTmp.NextFrame();

            if( pFrame && !pFrame->KnowsFormat(*this) )
            {
                // the Section doesn't have his own Frame, so if someone
                // needs the real size, we have to implement this by requesting
                // the matching Frame from the end.
                // PROBLEM: what happens if SectionFrames overlaps multiple
                //          pages?
                if( bPrtArea )
                    aRet = pFrame->getFramePrintArea();
                else
                {
                    aRet = pFrame->getFrameArea();
                    aRet.Pos().AdjustY( -1 );
                }
                pFrame = nullptr;       // the rect is finished by now
            }
        }
    }
    else
    {
        const SwFrameType nFrameType = RES_FLYFRMFMT == Which() ? SwFrameType::Fly : FRM_ALL;
        std::pair<Point, bool> tmp;
        if (pPoint)
        {
            tmp.first = *pPoint;
            tmp.second = false;
        }
        pFrame = ::GetFrameOfModify(nullptr, *this, nFrameType, nullptr, pPoint ? &tmp : nullptr);
    }

    if( pFrame )
    {
        if( bPrtArea )
            aRet = pFrame->getFramePrintArea();
        else
            aRet = pFrame->getFrameArea();
    }
    return aRet;
}

SdrObject* SwFrameFormat::FindRealSdrObject()
{
    if( RES_FLYFRMFMT == Which() )
    {
        Point aNullPt;
        std::pair<Point, bool> const tmp(aNullPt, false);
        SwFlyFrame* pFly = static_cast<SwFlyFrame*>(::GetFrameOfModify( nullptr, *this, SwFrameType::Fly,
                                                    nullptr, &tmp));
        if( pFly )
            return pFly->GetVirtDrawObj();

        if( !GetDoc() || !GetDoc()->GetDocShell() ||
            GetDoc()->GetDocShell()->GetCreateMode() != SfxObjectCreateMode::EMBEDDED )
            return nullptr;

        // tdf#126477 fix lost charts in embedded documents
    }
    return FindSdrObject();
}

bool SwFrameFormat::IsLowerOf( const SwFrameFormat& rFormat ) const
{
    //Also linking from inside to outside or from outside to inside is not
    //allowed.
    SwFlyFrame *pSFly = SwIterator<SwFlyFrame,SwFormat>(*this).First();
    if( pSFly )
    {
        SwFlyFrame *pAskFly = SwIterator<SwFlyFrame,SwFormat>(rFormat).First();
        if( pAskFly )
            return pSFly->IsLowerOf( pAskFly );
    }

    // let's try it using the node positions
    const SwFormatAnchor* pAnchor = &rFormat.GetAnchor();
    if ((RndStdIds::FLY_AT_PAGE != pAnchor->GetAnchorId()) && pAnchor->GetAnchorNode())
    {
        const SwNode* pFlyNd = pAnchor->GetAnchorNode()->FindFlyStartNode();
        while( pFlyNd )
        {
            // then we walk up using the anchor
            for(const sw::SpzFrameFormat* pFormat: *GetDoc()->GetSpzFrameFormats())
            {
                const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
                if( pIdx && pFlyNd == &pIdx->GetNode() )
                {
                    if( pFormat == this )
                        return true;

                    pAnchor = &pFormat->GetAnchor();
                    if ((RndStdIds::FLY_AT_PAGE == pAnchor->GetAnchorId()) ||
                        !pAnchor->GetAnchorNode() )
                    {
                        return false;
                    }

                    pFlyNd = pAnchor->GetAnchorNode()->FindFlyStartNode();
                    break;
                }
            }
        }
    }
    return false;
}

// #i31698#
SwFrameFormat::tLayoutDir SwFrameFormat::GetLayoutDir() const
{
    return SwFrameFormat::HORI_L2R;
}

void SwFrameFormat::SetLayoutDir( const SwFrameFormat::tLayoutDir )
{
    // empty body, because default implementation does nothing
}

// #i28749#
sal_Int16 SwFrameFormat::GetPositionLayoutDir() const
{
    return text::PositionLayoutDir::PositionInLayoutDirOfAnchor;
}
void SwFrameFormat::SetPositionLayoutDir( const sal_Int16 )
{
    // empty body, because default implementation does nothing
}

OUString SwFrameFormat::GetDescription() const
{
    return SwResId(STR_FRAME);
}

void SwFrameFormat::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFrameFormat"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetName().toUtf8().getStr()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("whichId"), "%d", Which());

    const char* pWhich = nullptr;
    switch (Which())
    {
    case RES_FLYFRMFMT:
        pWhich = "fly frame format";
        break;
    case RES_DRAWFRMFMT:
        pWhich = "draw frame format";
        break;
    }
    if (pWhich)
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("which"), BAD_CAST(pWhich));

    if (m_pOtherTextBoxFormats)
    {
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("OtherTextBoxFormat"), "%p", m_pOtherTextBoxFormats.get());
    }

    GetAttrSet().dumpAsXml(pWriter);

    if (const SdrObject* pSdrObject = FindSdrObject())
        pSdrObject->dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

SwFlyFrameFormat::SwFlyFrameFormat(SwAttrPool& rPool, const OUString &rFormatName, SwFrameFormat* pDerivedFrame)
    : sw::SpzFrameFormat(rPool, rFormatName, pDerivedFrame, RES_FLYFRMFMT)
{}

SwFlyFrameFormat::~SwFlyFrameFormat()
{
    SwIterator<SwFlyFrame,SwFormat> aIter( *this );
    SwFlyFrame * pLast = aIter.First();
    if( pLast )
        do
        {
            SwFrame::DestroyFrame(pLast);
        } while( nullptr != ( pLast = aIter.Next() ));

}

SwFlyDrawContact* SwFlyFrameFormat::GetOrCreateContact()
{
    if(!m_pContact)
    {
        SwDrawModel* pDrawModel(GetDoc()->getIDocumentDrawModelAccess().GetOrCreateDrawModel());
        m_pContact.reset(new SwFlyDrawContact(this, *pDrawModel));
    }

    return m_pContact.get();
}

/// Creates the Frames if the format describes a paragraph-bound frame.
/// MA: 1994-02-14: creates the Frames also for frames anchored at page.
void SwFlyFrameFormat::MakeFrames()
{
    // is there a layout?
    if( !GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell() )
        return;

    sw::BroadcastingModify *pModify = nullptr;
    // OD 24.07.2003 #111032# - create local copy of anchor attribute for possible changes.
    SwFormatAnchor aAnchorAttr( GetAnchor() );
    switch( aAnchorAttr.GetAnchorId() )
    {
    case RndStdIds::FLY_AS_CHAR:
    case RndStdIds::FLY_AT_PARA:
    case RndStdIds::FLY_AT_CHAR:
        if( aAnchorAttr.GetAnchorNode() )
        {
            pModify = aAnchorAttr.GetAnchorNode()->GetContentNode();
        }
        break;

    case RndStdIds::FLY_AT_FLY:
        if( aAnchorAttr.GetAnchorNode() )
        {
            //First search in the content because this is O(1)
            //This can go wrong for linked frames because in this case it's
            //possible, that no Frame exists for this content.
            //In such a situation we also need to search from StartNode to
            //FrameFormat.
            SwNodeIndex aIdx( *aAnchorAttr.GetAnchorNode() );
            SwContentNode* pCNd = SwNodes::GoNext(&aIdx);
            // #i105535#
            if ( pCNd == nullptr )
            {
                pCNd = aAnchorAttr.GetAnchorNode()->GetContentNode();
            }
            if ( pCNd )
            {
                if (SwIterator<SwFrame, SwContentNode, sw::IteratorMode::UnwrapMulti>(*pCNd).First())
                {
                    pModify = pCNd;
                }
            }
            // #i105535#
            if ( pModify == nullptr )
            {
                const SwNode & rNd = *aAnchorAttr.GetAnchorNode();
                for(sw::SpzFrameFormat* pFlyFormat: *GetDoc()->GetSpzFrameFormats())
                {
                    if( pFlyFormat->GetContent().GetContentIdx() &&
                        rNd == pFlyFormat->GetContent().GetContentIdx()->GetNode() )
                    {
                        pModify = pFlyFormat;
                        break;
                    }
                }
            }
        }
        break;

    case RndStdIds::FLY_AT_PAGE:
        {
            sal_uInt16 nPgNum = aAnchorAttr.GetPageNum();
            SwPageFrame *pPage = static_cast<SwPageFrame*>(GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout()->Lower());
            if( nPgNum == 0 && aAnchorAttr.GetAnchorNode() )
            {
                SwContentNode *pCNd = aAnchorAttr.GetAnchorNode()->GetContentNode();
                SwIterator<SwFrame, SwContentNode, sw::IteratorMode::UnwrapMulti> aIter(*pCNd);
                for ( SwFrame* pFrame = aIter.First(); pFrame != nullptr; pFrame = aIter.Next() )
                {
                    pPage = pFrame->FindPageFrame();
                    if( pPage )
                    {
                        nPgNum = pPage->GetPhyPageNum();
                        aAnchorAttr.SetPageNum( nPgNum );
                        aAnchorAttr.SetAnchor( nullptr );
                        SetFormatAttr( aAnchorAttr );
                        break;
                    }
                }
            }
            while ( pPage )
            {
                if ( pPage->GetPhyPageNum() == nPgNum )
                {
                    // #i50432# - adjust synopsis of <PlaceFly(..)>
                    pPage->PlaceFly( nullptr, this );
                    break;
                }
                pPage = static_cast<SwPageFrame*>(pPage->GetNext());
            }
        }
        break;
    default:
        break;
    }

    if( !pModify )
        return;

    SwIterator<SwFrame, sw::BroadcastingModify, sw::IteratorMode::UnwrapMulti> aIter(*pModify);
    for( SwFrame *pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
    {
        bool bAdd = !pFrame->IsContentFrame() ||
                        !static_cast<SwContentFrame*>(pFrame)->IsFollow();

        if ( RndStdIds::FLY_AT_FLY == aAnchorAttr.GetAnchorId() && !pFrame->IsFlyFrame() )
        {
            SwFrame* pFlyFrame = pFrame->FindFlyFrame();
            if ( pFlyFrame )
            {
                pFrame = pFlyFrame;
            }
            else
            {
                aAnchorAttr.SetType( RndStdIds::FLY_AT_PARA );
                SetFormatAttr( aAnchorAttr );
                MakeFrames();
                return;
            }
        }

        if (bAdd)
        {
            switch (aAnchorAttr.GetAnchorId())
            {
                case RndStdIds::FLY_AS_CHAR:
                case RndStdIds::FLY_AT_PARA:
                case RndStdIds::FLY_AT_CHAR:
                {
                    assert(pFrame->IsTextFrame());
                    bAdd = IsAnchoredObjShown(*static_cast<SwTextFrame*>(pFrame), aAnchorAttr);
                }
                break;
                default:
                break;
            }
        }

        if (bAdd && pFrame->GetDrawObjs())
        {
            // #i28701# - new type <SwSortedObjs>
            SwSortedObjs &rObjs = *pFrame->GetDrawObjs();
            for(SwAnchoredObject* pObj : rObjs)
            {
                // #i28701# - consider changed type of
                // <SwSortedObjs> entries.
                if( pObj->DynCastFlyFrame() !=  nullptr &&
                    (pObj->GetFrameFormat()) == this )
                {
                    bAdd = false;
                    break;
                }
            }
        }

        if( bAdd )
        {
            SwFlyFrame *pFly = nullptr; // avoid warnings
            switch( aAnchorAttr.GetAnchorId() )
            {
            case RndStdIds::FLY_AT_FLY:
                pFly = new SwFlyLayFrame( this, pFrame, pFrame );
                break;

            case RndStdIds::FLY_AT_PARA:
            case RndStdIds::FLY_AT_CHAR:
                pFly = new SwFlyAtContentFrame( this, pFrame, pFrame );
                break;

            case RndStdIds::FLY_AS_CHAR:
                pFly = new SwFlyInContentFrame( this, pFrame, pFrame );
                break;

            default:
                assert(false && "New anchor type" );
            }
            pFrame->AppendFly( pFly );
            pFly->GetFormat()->SetObjTitle(GetObjTitle());
            pFly->GetFormat()->SetObjDescription(GetObjDescription());
            SwPageFrame *pPage = pFly->FindPageFrame();
            if( pPage )
                ::RegistFlys( pPage, pFly );
        }
    }
}

SwFlyFrame* SwFlyFrameFormat::GetFrame( const Point* pPoint ) const
{
    std::pair<Point, bool> tmp;
    if (pPoint)
    {
        tmp.first = *pPoint;
        tmp.second = false;
    }
    return static_cast<SwFlyFrame*>(::GetFrameOfModify( nullptr, *this, SwFrameType::Fly,
                                            nullptr, &tmp));
}

SwAnchoredObject* SwFlyFrameFormat::GetAnchoredObj() const
{
    SwFlyFrame* pFlyFrame( GetFrame() );
    if ( pFlyFrame )
    {
        return pFlyFrame;
    }
    else
    {
        return nullptr;
    }
}

// #i73249#
void SwFlyFrameFormat::SetObjTitle( const OUString& rTitle, bool bBroadcast )
{
    SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject, "<SwFlyFrameFormat::SetObjTitle(..)> - missing <SdrObject> instance" );
    msTitle = rTitle;
    if ( !pMasterObject )
    {
        return;
    }

    const sw::TitleChanged aHint(pMasterObject->GetTitle(), rTitle);
    pMasterObject->SetTitle(rTitle);
    if(bBroadcast)
    {
        GetNotifier().Broadcast(aHint);
    }
}

OUString SwFlyFrameFormat::GetObjTitle() const
{
    const SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject, "<SwFlyFrameFormat::GetObjTitle(..)> - missing <SdrObject> instance" );
    if ( !pMasterObject )
    {
        return msTitle;
    }
    if (!pMasterObject->GetTitle().isEmpty())
        return pMasterObject->GetTitle();
    else
        return msTitle;
}

void SwFlyFrameFormat::SetObjTooltip(const OUString& rTooltip)
{
    msTooltip = rTooltip;
}

const OUString & SwFlyFrameFormat::GetObjTooltip() const
{
    return msTooltip;
}

void SwFlyFrameFormat::SetObjDescription( const OUString& rDescription, bool bBroadcast )
{
    SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject, "<SwFlyFrameFormat::SetDescription(..)> - missing <SdrObject> instance" );
    msDesc = rDescription;
    if ( !pMasterObject )
    {
        return;
    }

    const sw::DescriptionChanged aHint;
    pMasterObject->SetDescription(rDescription);
    if(bBroadcast)
    {
        GetNotifier().Broadcast(aHint);
    }
}

OUString SwFlyFrameFormat::GetObjDescription() const
{
    const SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject, "<SwFlyFrameFormat::GetDescription(..)> - missing <SdrObject> instance" );
    if ( !pMasterObject )
    {
        return msDesc;
    }
    if (!pMasterObject->GetDescription().isEmpty())
        return pMasterObject->GetDescription();
    else
        return msDesc;
}

bool SwFlyFrameFormat::IsDecorative() const
{
    const SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE(pMasterObject, "<SwFlyFrameFormat::SetDescription(..)> - missing <SdrObject> instance");
    if (!pMasterObject)
    {
        return false;
    }

    return pMasterObject->IsDecorative();
}

void SwFlyFrameFormat::SetObjDecorative(bool const isDecorative)
{
    SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject, "<SwFlyFrameFormat::SetDescription(..)> - missing <SdrObject> instance" );
    if ( !pMasterObject )
    {
        return;
    }

    SetFormatAttr(SfxBoolItem(RES_DECORATIVE, isDecorative));
    pMasterObject->SetDecorative(isDecorative);
    // does anybody care about a broadcast?
}


/** SwFlyFrameFormat::IsBackgroundTransparent - for #99657#

    OD 22.08.2002 - overriding virtual method and its default implementation,
    because format of fly frame provides transparent backgrounds.
    Method determines, if background of fly frame is transparent.

    @return true, if background color is transparent, but not "no fill"
    or the transparency of an existing background graphic is set.
*/
bool SwFlyFrameFormat::IsBackgroundTransparent() const
{
    if (supportsFullDrawingLayerFillAttributeSet() && getSdrAllFillAttributesHelper())
    {
        return getSdrAllFillAttributesHelper()->isTransparent();
    }

    // NOTE: If background color is "no fill"/"auto fill" (COL_TRANSPARENT)
    //     and there is no background graphic, it "inherites" the background
    //     from its anchor.
    std::unique_ptr<SvxBrushItem> aBackground(makeBackgroundBrushItem());
    if ( aBackground->GetColor().IsTransparent() &&
         aBackground->GetColor() != COL_TRANSPARENT
       )
    {
        return true;
    }
    else
    {
        const GraphicObject *pTmpGrf = aBackground->GetGraphicObject();
        if ( pTmpGrf &&
             pTmpGrf->GetAttr().IsTransparent()
           )
        {
            return true;
        }
    }

    return false;
}

/** SwFlyFrameFormat::IsBackgroundBrushInherited - for #103898#

    OD 08.10.2002 - method to determine, if the brush for drawing the
    background is "inherited" from its parent/grandparent.
    This is the case, if no background graphic is set and the background
    color is "no fill"/"auto fill"
    NOTE: condition is "copied" from method <SwFrame::GetBackgroundBrush(..).

    @return true, if background brush is "inherited" from parent/grandparent
*/
bool SwFlyFrameFormat::IsBackgroundBrushInherited() const
{
    if (supportsFullDrawingLayerFillAttributeSet() && getSdrAllFillAttributesHelper())
    {
        return !getSdrAllFillAttributesHelper()->isUsed();
    }
    else
    {
        std::unique_ptr<SvxBrushItem> aBackground(makeBackgroundBrushItem());
        if ( (aBackground->GetColor() == COL_TRANSPARENT) &&
             !(aBackground->GetGraphicObject()) )
        {
            return true;
        }
    }

    return false;
}

SwHandleAnchorNodeChg::SwHandleAnchorNodeChg( SwFlyFrameFormat& _rFlyFrameFormat,
                                              const SwFormatAnchor& _rNewAnchorFormat,
                                              SwFlyFrame const * _pKeepThisFlyFrame )
    : mrFlyFrameFormat( _rFlyFrameFormat ),
      mbAnchorNodeChanged( false ),
      mpWrtShell(nullptr)
{
    const SwFormatAnchor& aOldAnchorFormat(_rFlyFrameFormat.GetAnchor());
    const RndStdIds nNewAnchorType( _rNewAnchorFormat.GetAnchorId() );
    if ( ((nNewAnchorType == RndStdIds::FLY_AT_PARA) ||
          (nNewAnchorType == RndStdIds::FLY_AT_CHAR)) &&
         _rNewAnchorFormat.GetAnchorNode() &&
         _rNewAnchorFormat.GetAnchorNode()->GetContentNode() )
    {
        if ( aOldAnchorFormat.GetAnchorId() == nNewAnchorType &&
             aOldAnchorFormat.GetAnchorNode() &&
             aOldAnchorFormat.GetAnchorNode()->GetContentNode() &&
             aOldAnchorFormat.GetContentAnchor()->GetNode() !=
                                    _rNewAnchorFormat.GetContentAnchor()->GetNode() )
        {
            // determine 'old' number of anchor frames
            sal_uInt32 nOldNumOfAnchFrame( 0 );
            SwIterator<SwFrame, SwContentNode, sw::IteratorMode::UnwrapMulti> aOldIter(
                *(aOldAnchorFormat.GetAnchorNode()->GetContentNode()) );
            for( SwFrame* pOld = aOldIter.First(); pOld; pOld = aOldIter.Next() )
            {
                ++nOldNumOfAnchFrame;
            }
            // determine 'new' number of anchor frames
            sal_uInt32 nNewNumOfAnchFrame( 0 );
            SwIterator<SwFrame, SwContentNode, sw::IteratorMode::UnwrapMulti> aNewIter(
                *(_rNewAnchorFormat.GetAnchorNode()->GetContentNode()) );
            for( SwFrame* pNew = aNewIter.First(); pNew; pNew = aNewIter.Next() )
            {
                ++nNewNumOfAnchFrame;
            }
            if ( nOldNumOfAnchFrame != nNewNumOfAnchFrame )
            {
                // delete existing fly frames except <_pKeepThisFlyFrame>
                SwIterator<SwFrame,SwFormat> aIter( mrFlyFrameFormat );
                SwFrame* pFrame = aIter.First();
                if ( pFrame )
                {
                    do {
                        if ( pFrame != _pKeepThisFlyFrame )
                        {
                            pFrame->Cut();
                            SwFrame::DestroyFrame(pFrame);
                        }
                    } while( nullptr != ( pFrame = aIter.Next() ));
                }
                // indicate, that re-creation of fly frames necessary
                mbAnchorNodeChanged = true;
            }
        }
    }

    if (aOldAnchorFormat.GetAnchorNode()
        && aOldAnchorFormat.GetAnchorId() == RndStdIds::FLY_AT_CHAR)
    {
        moCommentAnchor.emplace(*aOldAnchorFormat.GetContentAnchor());
    }

    if (_pKeepThisFlyFrame)
    {
        SwViewShell* pViewShell = _pKeepThisFlyFrame->getRootFrame()->GetCurrShell();
        mpWrtShell = dynamic_cast<SwWrtShell*>(pViewShell);
    }
}

void SwHandleAnchorNodeChg::ImplDestroy()
{
    if ( mbAnchorNodeChanged )
    {
        mrFlyFrameFormat.MakeFrames();
    }

    // See if the fly frame had a comment: if so, move it to the new anchor as well.
    if (!moCommentAnchor)
    {
        return;
    }

    SwTextNode* pTextNode = moCommentAnchor->GetNode().GetTextNode();
    if (!pTextNode)
    {
        return;
    }

    const SwTextField* pField = pTextNode->GetFieldTextAttrAt(moCommentAnchor->GetContentIndex());
    if (!pField || pField->GetFormatField().GetField()->GetTyp()->Which() != SwFieldIds::Postit)
    {
        return;
    }

    if (!mpWrtShell)
    {
        return;
    }

    // Save current cursor position, so we can restore it later.
    mpWrtShell->Push();

    // Set up the source of the move: the old comment anchor.
    {
        SwPaM& rCursor = mpWrtShell->GetCurrentShellCursor();
        *rCursor.GetPoint() = *moCommentAnchor;
        rCursor.SetMark();
        *rCursor.GetMark() = *moCommentAnchor;
        rCursor.GetMark()->AdjustContent(+1);
    }

    // Set up the target of the move: the new comment anchor.
    const SwFormatAnchor& rNewAnchorFormat = mrFlyFrameFormat.GetAnchor();
    mpWrtShell->CreateCursor();
    *mpWrtShell->GetCurrentShellCursor().GetPoint() = *rNewAnchorFormat.GetContentAnchor();

    // Move by copying and deleting.
    mpWrtShell->SwEditShell::Copy(*mpWrtShell);
    mpWrtShell->DestroyCursor();

    mpWrtShell->Delete(false);

    mpWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
}

SwHandleAnchorNodeChg::~SwHandleAnchorNodeChg()
{
    suppress_fun_call_w_exception(ImplDestroy());
}

namespace sw
{
    DrawFrameFormatHint::~DrawFrameFormatHint() {}
    CheckDrawFrameFormatLayerHint::~CheckDrawFrameFormatLayerHint() {}
    ContactChangedHint::~ContactChangedHint() {}
    DrawFormatLayoutCopyHint::~DrawFormatLayoutCopyHint() {}
    WW8AnchorConvHint::~WW8AnchorConvHint() {}
    RestoreFlyAnchorHint::~RestoreFlyAnchorHint() {}
    CreatePortionHint::~CreatePortionHint() {}
    FindSdrObjectHint::~FindSdrObjectHint() {}
    CollectTextObjectsHint::~CollectTextObjectsHint() {}
    GetZOrderHint::~GetZOrderHint() {}
    GetObjectConnectedHint::~GetObjectConnectedHint() {}
}

SwDrawFrameFormat::~SwDrawFrameFormat()
{
    CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::DYING));
}

void SwDrawFrameFormat::MakeFrames()
{
    CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::MAKE_FRAMES));
}

void SwDrawFrameFormat::DelFrames()
{
    CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::DELETE_FRAMES));
}

// #i31698#
SwFrameFormat::tLayoutDir SwDrawFrameFormat::GetLayoutDir() const
{
    return meLayoutDir;
}

void SwDrawFrameFormat::SetLayoutDir( const SwFrameFormat::tLayoutDir _eLayoutDir )
{
    meLayoutDir = _eLayoutDir;
}

// #i28749#
sal_Int16 SwDrawFrameFormat::GetPositionLayoutDir() const
{
    return mnPositionLayoutDir;
}
void SwDrawFrameFormat::SetPositionLayoutDir( const sal_Int16 _nPositionLayoutDir )
{
    switch ( _nPositionLayoutDir )
    {
        case text::PositionLayoutDir::PositionInHoriL2R:
        case text::PositionLayoutDir::PositionInLayoutDirOfAnchor:
        {
            mnPositionLayoutDir = _nPositionLayoutDir;
        }
        break;
        default:
        {
            OSL_FAIL( "<SwDrawFrameFormat::SetPositionLayoutDir(..)> - invalid attribute value." );
        }
    }
}

OUString SwDrawFrameFormat::GetDescription() const
{
    OUString aResult;
    const SdrObject * pSdrObj = FindSdrObject();

    if (pSdrObj)
    {
        if (pSdrObj != m_pSdrObjectCached)
        {
            m_sSdrObjectCachedComment = SdrUndoNewObj::GetComment(*pSdrObj);
            m_pSdrObjectCached = pSdrObj;
        }

        aResult = m_sSdrObjectCachedComment;
    }
    else
        aResult = SwResId(STR_GRAPHIC);

    return aResult;
}

IMapObject* SwFrameFormat::GetIMapObject( const Point& rPoint,
                                        const SwFlyFrame *pFly ) const
{
    const SwFormatURL &rURL = GetURL();
    if( !rURL.GetMap() )
        return nullptr;

    if( !pFly )
    {
        pFly = SwIterator<SwFlyFrame,SwFormat>( *this ).First();
        if( !pFly )
            return nullptr;
    }

    //Original size for OLE and graphic is TwipSize, otherwise the size of
    //FrameFormat of the Fly.
    const SwFrame *pRef;
    const SwNoTextNode *pNd = nullptr;
    Size aOrigSz;
    if( pFly->Lower() && pFly->Lower()->IsNoTextFrame() )
    {
        pRef = pFly->Lower();
        pNd = static_cast<const SwNoTextFrame*>(pRef)->GetNode()->GetNoTextNode();
        aOrigSz = pNd->GetTwipSize();
    }
    else
    {
        pRef = pFly;
        aOrigSz = pFly->GetFormat()->GetFrameSize().GetSize();
    }

    if( !aOrigSz.IsEmpty() )
    {
        Point aPos( rPoint );
        Size aActSz ( pRef == pFly ? pFly->getFrameArea().SSize() : pRef->getFramePrintArea().SSize() );
        const o3tl::Length aSrc ( o3tl::Length::twip );
        const o3tl::Length aDest( o3tl::Length::mm100 );
        aOrigSz = o3tl::convert( aOrigSz, aSrc, aDest );
        aActSz  = o3tl::convert( aActSz,  aSrc, aDest );
        aPos -= pRef->getFrameArea().Pos();
        aPos -= pRef->getFramePrintArea().Pos();
        aPos    = o3tl::convert( aPos, aSrc, aDest );
        sal_uInt32 nFlags = 0;
        if ( pFly != pRef && pNd->IsGrfNode() )
        {
            const MirrorGraph nMirror = pNd->GetSwAttrSet().
                                        GetMirrorGrf().GetValue();
            if ( MirrorGraph::Both == nMirror )
                nFlags = IMAP_MIRROR_HORZ | IMAP_MIRROR_VERT;
            else if ( MirrorGraph::Vertical == nMirror )
                nFlags = IMAP_MIRROR_VERT;
            else if ( MirrorGraph::Horizontal == nMirror )
                nFlags = IMAP_MIRROR_HORZ;

        }
        return rURL.GetMap()->GetHitIMapObject( aOrigSz, aActSz, aPos, nFlags );
    }

    return nullptr;
}

drawinglayer::attribute::SdrAllFillAttributesHelperPtr SwFrameFormat::getSdrAllFillAttributesHelper() const
{
    if (supportsFullDrawingLayerFillAttributeSet())
    {
        // create FillAttributes on demand
        if(!maFillAttributes)
        {
            const_cast< SwFrameFormat* >(this)->maFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(GetAttrSet());
        }
    }
    else
    {
        // FALLBACKBREAKHERE assert wrong usage
        OSL_ENSURE(false, "getSdrAllFillAttributesHelper() call only valid for RES_FLYFRMFMT and RES_FRMFMT (!)");
    }

    return maFillAttributes;
}

void SwFrameFormat::MoveTableBox(SwTableBox& rTableBox, const SwFrameFormat* pOldFormat)
{
    Add(rTableBox);
    if(!pOldFormat)
        return;
    const auto& rOld = pOldFormat->GetFormatAttr(RES_BOXATR_FORMAT);
    const auto& rNew = GetFormatAttr(RES_BOXATR_FORMAT);
    if(rOld != rNew)
        SwClientNotify(*this, sw::LegacyModifyHint(&rOld, &rNew));
}

bool SwFrameFormat::IsVisible() const
{
    return SwIterator<SwFrame, SwFrameFormat>(*this).First();
};

namespace sw {

bool IsFlyFrameFormatInHeader(const SwFrameFormat& rFormat)
{
    const SwFlyFrameFormat* pFlyFrameFormat = dynamic_cast<const SwFlyFrameFormat*>(&rFormat);
    if (!pFlyFrameFormat)
        return false;
    SwFlyFrame* pFlyFrame = pFlyFrameFormat->GetFrame();
    if (!pFlyFrame) // fdo#54648: "hidden" drawing object has no layout frame
    {
        return false;
    }
    SwPageFrame* pPageFrame = pFlyFrame->FindPageFrameOfAnchor();
    SwFrame* pHeader = pPageFrame->Lower();
    if (pHeader->GetType() == SwFrameType::Header)
    {
        const SwFrame* pFrame = pFlyFrame->GetAnchorFrame();
        while (pFrame)
        {
            if (pFrame == pHeader)
                return true;
            pFrame = pFrame->GetUpper();
        }
    }
    return false;
}

void CheckAnchoredFlyConsistency(SwDoc const& rDoc)
{
#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    SwNodes const& rNodes(rDoc.GetNodes());
    SwNodeOffset const count(rNodes.Count());
    for (SwNodeOffset i(0); i != count; ++i)
    {
        SwNode const*const pNode(rNodes[i]);
        std::vector<SwFrameFormat*> const & rFlys(pNode->GetAnchoredFlys());
        for (const auto& rpFly : rFlys)
        {
            SwFormatAnchor const& rAnchor((*rpFly).GetAnchor(false));
            assert(rAnchor.GetAnchorNode() == pNode);
        }
    }
    if(!rDoc.GetSpzFrameFormats())
        return;

    for(sw::SpzFrameFormat* pSpz: *rDoc.GetSpzFrameFormats())
    {
        SwFormatAnchor const& rAnchor(pSpz->GetAnchor(false));
        if (RndStdIds::FLY_AT_PAGE == rAnchor.GetAnchorId())
        {
            assert(!rAnchor.GetAnchorNode()
                // for invalid documents that lack text:anchor-page-number
                // it may have an anchor before MakeFrames() is called
                || (!SwIterator<SwFrame, SwFrameFormat>(*pSpz).First()));
        }
        else
        {
            SwNode & rNode(*rAnchor.GetAnchorNode());
            std::vector<SwFrameFormat*> const& rFlys(rNode.GetAnchoredFlys());
            assert(std::find(rFlys.begin(), rFlys.end(), pSpz) != rFlys.end());
            switch (rAnchor.GetAnchorId())
            {
                case RndStdIds::FLY_AT_FLY:
                    assert(rNode.IsStartNode());
                break;
                case RndStdIds::FLY_AT_PARA:
                    assert(rNode.IsTextNode() || rNode.IsTableNode());
                break;
                case RndStdIds::FLY_AS_CHAR:
                case RndStdIds::FLY_AT_CHAR:
                    assert(rNode.IsTextNode());
                break;
                default:
                    assert(false);
                break;
            }
        }
    }
#else
    (void) rDoc;
#endif
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
