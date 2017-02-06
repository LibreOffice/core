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

#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/text/TextGridMode.hpp>
#include <o3tl/any.hxx>
#include <svtools/unoimap.hxx>
#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>
#include <frmfmt.hxx>
#include <unocoll.hxx>
#include <unosett.hxx>
#include <unostyle.hxx>
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
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <crsrsh.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <frmtool.hxx>
#include <flyfrms.hxx>
#include <pagedesc.hxx>
#include <grfatr.hxx>
#include <ndnotxt.hxx>
#include <docary.hxx>
#include <node2lay.hxx>
#include <fmtclbl.hxx>
#include <swunohelper.hxx>
#include <unoframe.hxx>
#include <unotextbodyhf.hxx>
#include <SwStyleNameMapper.hxx>
#include <editeng/brushitem.hxx>
#include <svtools/grfmgr.hxx>
#include <unomid.h>
#include <comcore.hrc>
#include <svx/svdundo.hxx>
#include <sortedobjs.hxx>
#include <HandleAnchorNodeChg.hxx>
#include <calbck.hxx>
#include <pagedeschint.hxx>
#ifndef NDEBUG
#include <ndtxt.hxx>
#endif

//UUUU
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <svx/xfillit0.hxx>
#include <svl/itemiter.hxx>

using namespace ::com::sun::star;


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
    pFormat->Remove( pToRemove );
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
            if(dynamic_cast<const SwFrame*>( pLast ) ==  nullptr || !SwXHeadFootText::IsXHeadFootText(pLast))
                bDel = false;
    }

    if ( bDel )
    {
        // If there is a Cursor registered in one of the nodes, we need to call the
        // ParkCursor in an (arbitrary) shell.
        SwFormatContent& rCnt = (SwFormatContent&)pFormat->GetContent();
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
                sal_uInt32 nEnd = pNode->EndOfSectionIndex();
                while ( aIdx < nEnd )
                {
                    if ( pNode->IsContentNode() &&
                         static_cast<SwContentNode*>(pNode)->HasWriterListeners() )
                    {
                        SwCursorShell *pShell = SwIterator<SwCursorShell,SwContentNode>( *static_cast<SwContentNode*>(pNode) ).First();
                        if( pShell )
                        {
                            pShell->ParkCursor( aIdx );
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
}

// Partially implemented inline in hxx
SwFormatFrameSize::SwFormatFrameSize( SwFrameSize eSize, SwTwips nWidth, SwTwips nHeight )
    : SfxPoolItem( RES_FRM_SIZE ),
    m_aSize( nWidth, nHeight ),
    m_eFrameHeightType( eSize ),
    m_eFrameWidthType( ATT_FIX_SIZE )
{
    m_nWidthPercent = m_eWidthPercentRelation = m_nHeightPercent = m_eHeightPercentRelation = 0;
}

SwFormatFrameSize& SwFormatFrameSize::operator=( const SwFormatFrameSize& rCpy )
{
    m_aSize = rCpy.GetSize();
    m_eFrameHeightType = rCpy.GetHeightSizeType();
    m_eFrameWidthType = rCpy.GetWidthSizeType();
    m_nHeightPercent = rCpy.GetHeightPercent();
    m_eHeightPercentRelation  = rCpy.GetHeightPercentRelation();
    m_nWidthPercent  = rCpy.GetWidthPercent();
    m_eWidthPercentRelation  = rCpy.GetWidthPercentRelation();
    return *this;
}

bool SwFormatFrameSize::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return( m_eFrameHeightType  == static_cast<const SwFormatFrameSize&>(rAttr).m_eFrameHeightType &&
            m_eFrameWidthType  == static_cast<const SwFormatFrameSize&>(rAttr).m_eFrameWidthType &&
            m_aSize           == static_cast<const SwFormatFrameSize&>(rAttr).GetSize()&&
            m_nWidthPercent   == static_cast<const SwFormatFrameSize&>(rAttr).GetWidthPercent() &&
            m_eWidthPercentRelation == static_cast<const SwFormatFrameSize&>(rAttr).GetWidthPercentRelation() &&
            m_nHeightPercent  == static_cast<const SwFormatFrameSize&>(rAttr).GetHeightPercent() &&
            m_eHeightPercentRelation == static_cast<const SwFormatFrameSize&>(rAttr).GetHeightPercentRelation() );
}

SfxPoolItem*  SwFormatFrameSize::Clone( SfxItemPool* ) const
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
            aTmp.Height = convertTwipToMm100(m_aSize.Height());
            aTmp.Width = convertTwipToMm100(m_aSize.Width());
            rVal <<= aTmp;
        }
        break;
        case MID_FRMSIZE_REL_HEIGHT:
            rVal <<= (sal_Int16)(GetHeightPercent() != SwFormatFrameSize::SYNCED ? GetHeightPercent() : 0);
        break;
        case MID_FRMSIZE_REL_HEIGHT_RELATION:
            rVal <<= GetHeightPercentRelation();
        break;
        case MID_FRMSIZE_REL_WIDTH:
            rVal <<= (sal_Int16)(GetWidthPercent() != SwFormatFrameSize::SYNCED ? GetWidthPercent() : 0);
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
            rVal <<= (sal_Int32)convertTwipToMm100(m_aSize.Width());
        break;
        case MID_FRMSIZE_HEIGHT:
            // #95848# returned size should never be zero.
            // (there was a bug that allowed for setting height to 0.
            // Thus there some documents existing with that not allowed
            // attribute value which may cause problems on import.)
            rVal <<= (sal_Int32)convertTwipToMm100(m_aSize.Height() < MINLAY ? MINLAY : m_aSize.Height() );
        break;
        case MID_FRMSIZE_SIZE_TYPE:
            rVal <<= (sal_Int16)GetHeightSizeType();
        break;
        case MID_FRMSIZE_IS_AUTO_HEIGHT:
            rVal <<= ATT_FIX_SIZE != GetHeightSizeType();
        break;
        case MID_FRMSIZE_WIDTH_TYPE:
            rVal <<= (sal_Int16)GetWidthSizeType();
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
                    aTmp.Height() = convertMm100ToTwip(aTmp.Height());
                    aTmp.Width() = convertMm100ToTwip(aTmp.Width());
                }
                m_aSize = aTmp;
            }
        }
        break;
        case MID_FRMSIZE_REL_HEIGHT:
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0 && nSet < SwFormatFrameSize::SYNCED)
                SetHeightPercent((sal_uInt8)nSet);
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
                SetWidthPercent((sal_uInt8)nSet);
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
                    nWd = convertMm100ToTwip(nWd);
                if(nWd < MINLAY)
                   nWd = MINLAY;
                m_aSize.Width() = nWd;
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
                    nHg = convertMm100ToTwip(nHg);
                if(nHg < MINLAY)
                    nHg = MINLAY;
                m_aSize.Height() = nHg;
            }
            else
                bRet = false;
        }
        break;
        case MID_FRMSIZE_SIZE_TYPE:
        {
            sal_Int16 nType = 0;
            if((rVal >>= nType) && nType >= 0 && nType <= ATT_MIN_SIZE )
            {
                SetHeightSizeType((SwFrameSize)nType);
            }
            else
                bRet = false;
        }
        break;
        case MID_FRMSIZE_IS_AUTO_HEIGHT:
        {
            bool bSet = *o3tl::doAccess<bool>(rVal);
            SetHeightSizeType(bSet ? ATT_VAR_SIZE : ATT_FIX_SIZE);
        }
        break;
        case MID_FRMSIZE_WIDTH_TYPE:
        {
            sal_Int16 nType = 0;
            if((rVal >>= nType) && nType >= 0 && nType <= ATT_MIN_SIZE )
            {
                SetWidthSizeType((SwFrameSize)nType);
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatFrameSize"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));

    std::stringstream aSize;
    aSize << m_aSize;
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("size"), BAD_CAST(aSize.str().c_str()));

    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eFrameHeightType"), BAD_CAST(OString::number(m_eFrameHeightType).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eFrameWidthType"), BAD_CAST(OString::number(m_eFrameWidthType).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWidthPercent"), BAD_CAST(OString::number(m_nWidthPercent).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eWidthPercentRelation"), BAD_CAST(OString::number(m_eWidthPercentRelation).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nHeightPercent"), BAD_CAST(OString::number(m_nHeightPercent).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eHeightPercentRelation"), BAD_CAST(OString::number(m_eHeightPercentRelation).getStr()));

    xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFormatFillOrder::SwFormatFillOrder( SwFillOrder nFO )
    : SfxEnumItem( RES_FILL_ORDER, sal_uInt16(nFO) )
{}
SfxPoolItem*  SwFormatFillOrder::Clone( SfxItemPool* ) const
{
    return new SwFormatFillOrder( SwFillOrder(GetValue()) );
}

sal_uInt16  SwFormatFillOrder::GetValueCount() const
{
    return SW_FILL_ORDER_END - SW_FILL_ORDER_BEGIN;
}

// Partially implemented inline in hxx
SwFormatHeader::SwFormatHeader( SwFrameFormat *pHeaderFormat )
    : SfxPoolItem( RES_HEADER ),
    SwClient( pHeaderFormat ),
    bActive( pHeaderFormat )
{
}

SwFormatHeader::SwFormatHeader( const SwFormatHeader &rCpy )
    : SfxPoolItem( RES_HEADER ),
    SwClient( const_cast<SwModify*>(rCpy.GetRegisteredIn()) ),
    bActive( rCpy.IsActive() )
{
}

SwFormatHeader::SwFormatHeader( bool bOn )
    : SfxPoolItem( RES_HEADER ),
    SwClient( nullptr ),
    bActive( bOn )
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
             bActive == static_cast<const SwFormatHeader&>(rAttr).IsActive() );
}

SfxPoolItem*  SwFormatHeader::Clone( SfxItemPool* ) const
{
    return new SwFormatHeader( *this );
}

void SwFormatHeader::RegisterToFormat( SwFormat& rFormat )
{
    rFormat.Add(this);
}

// Partially implemented inline in hxx
SwFormatFooter::SwFormatFooter( SwFrameFormat *pFooterFormat )
    : SfxPoolItem( RES_FOOTER ),
    SwClient( pFooterFormat ),
    bActive( pFooterFormat )
{
}

SwFormatFooter::SwFormatFooter( const SwFormatFooter &rCpy )
    : SfxPoolItem( RES_FOOTER ),
    SwClient( const_cast<SwModify*>(rCpy.GetRegisteredIn()) ),
    bActive( rCpy.IsActive() )
{
}

SwFormatFooter::SwFormatFooter( bool bOn )
    : SfxPoolItem( RES_FOOTER ),
    SwClient( nullptr ),
    bActive( bOn )
{
}

 SwFormatFooter::~SwFormatFooter()
{
    if ( GetFooterFormat() )
        lcl_DelHFFormat( this, GetFooterFormat() );
}

void SwFormatFooter::RegisterToFormat( SwFormat& rFormat )
{
    rFormat.Add(this);
}

bool SwFormatFooter::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( GetRegisteredIn() == static_cast<const SwFormatFooter&>(rAttr).GetRegisteredIn() &&
             bActive == static_cast<const SwFormatFooter&>(rAttr).IsActive() );
}

SfxPoolItem*  SwFormatFooter::Clone( SfxItemPool* ) const
{
    return new SwFormatFooter( *this );
}

// Partially implemented inline in hxx
SwFormatContent::SwFormatContent( const SwFormatContent &rCpy )
    : SfxPoolItem( RES_CNTNT )
{
    pStartNode.reset( rCpy.GetContentIdx() ?
                      new SwNodeIndex( *rCpy.GetContentIdx() ) : nullptr);
}

SwFormatContent::SwFormatContent( const SwStartNode *pStartNd )
    : SfxPoolItem( RES_CNTNT )
{
    pStartNode.reset( pStartNd ? new SwNodeIndex( *pStartNd ) : nullptr);
}

SwFormatContent::~SwFormatContent()
{
}

void SwFormatContent::SetNewContentIdx( const SwNodeIndex *pIdx )
{
    pStartNode.reset( pIdx ? new SwNodeIndex( *pIdx ) : nullptr );
}

bool SwFormatContent::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    if( (bool)pStartNode != (bool)static_cast<const SwFormatContent&>(rAttr).pStartNode )
        return false;
    if( pStartNode )
        return ( *pStartNode == *static_cast<const SwFormatContent&>(rAttr).GetContentIdx() );
    return true;
}

SfxPoolItem*  SwFormatContent::Clone( SfxItemPool* ) const
{
    return new SwFormatContent( *this );
}

void SwFormatContent::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatContent"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("startNode"), BAD_CAST(OString::number(pStartNode->GetNode().GetIndex()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFormatPageDesc::SwFormatPageDesc( const SwFormatPageDesc &rCpy )
    : SfxPoolItem( RES_PAGEDESC ),
    SwClient( const_cast<SwPageDesc*>(rCpy.GetPageDesc()) ),
    oNumOffset( rCpy.oNumOffset ),
    pDefinedIn( nullptr )
{
}

SwFormatPageDesc::SwFormatPageDesc( const SwPageDesc *pDesc )
    : SfxPoolItem( RES_PAGEDESC ),
    SwClient( const_cast<SwPageDesc*>(pDesc) ),
    pDefinedIn( nullptr )
{
}

SwFormatPageDesc &SwFormatPageDesc::operator=(const SwFormatPageDesc &rCpy)
{
    if (rCpy.GetPageDesc())
        RegisterToPageDesc(*const_cast<SwPageDesc*>(rCpy.GetPageDesc()));
    oNumOffset = rCpy.oNumOffset;
    pDefinedIn = nullptr;

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
    return  ( pDefinedIn == static_cast<const SwFormatPageDesc&>(rAttr).pDefinedIn ) &&
            ( oNumOffset == static_cast<const SwFormatPageDesc&>(rAttr).oNumOffset ) &&
            ( GetPageDesc() == static_cast<const SwFormatPageDesc&>(rAttr).GetPageDesc() );
}

SfxPoolItem*  SwFormatPageDesc::Clone( SfxItemPool* ) const
{
    return new SwFormatPageDesc( *this );
}

void SwFormatPageDesc::SwClientNotify( const SwModify& rModify, const SfxHint& rHint )
{
    SwClient::SwClientNotify(rModify, rHint);
    const SwPageDescHint* pHint = dynamic_cast<const SwPageDescHint*>(&rHint);
    if ( pHint )
    {
        // mba: shouldn't that be broadcasted also?
        SwFormatPageDesc aDfltDesc( pHint->GetPageDesc() );
        SwPageDesc* pDesc = pHint->GetPageDesc();
        const SwModify* pMod = GetDefinedIn();
        if ( pMod )
        {
            if( dynamic_cast<const SwContentNode*>( pMod) !=  nullptr )
                const_cast<SwContentNode*>(static_cast<const SwContentNode*>(pMod))->SetAttr( aDfltDesc );
            else if( dynamic_cast<const SwFormat*>( pMod) !=  nullptr)
                const_cast<SwFormat*>(static_cast<const SwFormat*>(pMod))->SetFormatAttr( aDfltDesc );
            else
            {
                OSL_FAIL( "What kind of SwModify is this?" );
                RegisterToPageDesc( *pDesc );
            }
        }
        else
            // there could be an Undo-copy
            RegisterToPageDesc( *pDesc );
    }
}

void SwFormatPageDesc::RegisterToPageDesc( SwPageDesc& rDesc )
{
    rDesc.Add( this );
}

void SwFormatPageDesc::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( !pDefinedIn )
        return;

    const sal_uInt16 nWhichId = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhichId )
    {
        case RES_OBJECTDYING:
                //The Pagedesc where I'm registered dies, therefore I unregister
                //from that format. During this I get deleted!
            if( typeid(SwFormat) == typeid( pDefinedIn ))
            {
                bool const bResult =
                    static_cast<SwFormat*>(pDefinedIn)->ResetFormatAttr(RES_PAGEDESC);
                OSL_ENSURE( bResult, "FormatPageDesc not deleted" );
                (void) bResult; // unused in non-debug
            }
            else if( typeid(SwContentNode) == typeid( pDefinedIn ))
            {
                bool const bResult = static_cast<SwContentNode*>(pDefinedIn)
                        ->ResetAttr(RES_PAGEDESC);
                OSL_ENSURE( bResult, "FormatPageDesc not deleted" );
                (void) bResult; // unused in non-debug
            }
            break;

        default:
            /* do nothing */;
    }
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
                ::boost::optional<sal_uInt16> oOffset = GetNumOffset();
                if (oOffset)
                {
                    rVal <<= (sal_Int16)oOffset.get();
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
                    SwStyleNameMapper::FillProgName(pDesc->GetName(), aString, SwGetPoolIdFromName::PageDesc, true );
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
                SetNumOffset(boost::none);
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatPageDesc"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    if (oNumOffset)
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("oNumOffset"), BAD_CAST(OString::number(*oNumOffset).getStr()));
    else
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("oNumOffset"), BAD_CAST("none"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("pPageDesc"), "%p", GetPageDesc());
    if (const SwPageDesc* pPageDesc = GetPageDesc())
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(pPageDesc->GetName().toUtf8().getStr()));
    xmlTextWriterEndElement(pWriter);
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwColumn"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWish"), BAD_CAST(OString::number(m_nWish).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nUpper"), BAD_CAST(OString::number(0).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLower"), BAD_CAST(OString::number(0).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLeft"), BAD_CAST(OString::number(m_nLeft).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nRight"), BAD_CAST(OString::number(m_nRight).getStr()));
    xmlTextWriterEndElement(pWriter);
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
        m_aColumns.push_back( SwColumn(rCpy.GetColumns()[i]) );
    }
}

SwFormatCol::~SwFormatCol() {}

SwFormatCol& SwFormatCol::operator=( const SwFormatCol& rCpy )
{
    m_eLineStyle  = rCpy.m_eLineStyle;
    m_nLineWidth  = rCpy.m_nLineWidth;
    m_aLineColor  = rCpy.m_aLineColor;
    m_nLineHeight = rCpy.GetLineHeight();
    m_eAdj        = rCpy.GetLineAdj();
    m_nWidth      = rCpy.GetWishWidth();
    m_aWidthAdjustValue = rCpy.m_aWidthAdjustValue;
    m_bOrtho      = rCpy.IsOrtho();

    if ( !m_aColumns.empty() )
        m_aColumns.clear();
    for ( sal_uInt16 i = 0; i < rCpy.GetNumCols(); ++i )
    {
        m_aColumns.push_back( SwColumn(rCpy.GetColumns()[i]) );
    }
    return *this;
}

SwFormatCol::SwFormatCol()
    : SfxPoolItem( RES_COL )
    , m_eLineStyle( table::BorderLineStyle::NONE)
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

SfxPoolItem*  SwFormatCol::Clone( SfxItemPool* ) const
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
        for ( size_t i = 0; i < m_aColumns.size(); ++i )
        {
            SwColumn *pCol = &m_aColumns[i];
            pCol->SetLeft ( nHalf );
            pCol->SetRight( nHalf );
            if ( i == 0 )
                pCol->SetLeft( 0 );
            else if ( i+1 == m_aColumns.size() )
                pCol->SetRight( 0 );
        }
    }
}

void SwFormatCol::Init( sal_uInt16 nNumCols, sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    // Deleting seems to be a bit radical on the first sight; but otherwise we
    // have to initialize all values of the remaining SwCloumns.
    if ( !m_aColumns.empty() )
        m_aColumns.clear();
    for ( sal_uInt16 i = 0; i < nNumCols; ++i )
    {
        m_aColumns.push_back( SwColumn() );
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
        long nW = m_aColumns[nCol].GetWishWidth();
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
    if(!GetNumCols())
        return;
    //First set the column widths with the current width, then calculate the
    //column's requested width using the requested total width.

    const sal_uInt16 nGutterHalf = nGutterWidth ? nGutterWidth / 2 : 0;

    //Width of PrtAreas is totalwidth - spacings / count
    const sal_uInt16 nPrtWidth =
                (nAct - ((GetNumCols()-1) * nGutterWidth)) / GetNumCols();
    sal_uInt16 nAvail = nAct;

    //The fist column is PrtWidth + (gap width / 2)
    const sal_uInt16 nLeftWidth = nPrtWidth + nGutterHalf;
    SwColumn *pCol = &m_aColumns.front();
    pCol->SetWishWidth( nLeftWidth );
    pCol->SetRight( nGutterHalf );
    pCol->SetLeft ( 0 );
    nAvail = nAvail - nLeftWidth;

    //Column 2 to n-1 is PrtWidth + gap width
    const sal_uInt16 nMidWidth = nPrtWidth + nGutterWidth;
    sal_uInt16 i;

    for ( i = 1; i < GetNumCols()-1; ++i )
    {
        pCol = &m_aColumns[i];
        pCol->SetWishWidth( nMidWidth );
        pCol->SetLeft ( nGutterHalf );
        pCol->SetRight( nGutterHalf );
        nAvail = nAvail - nMidWidth;
    }

    //The last column is equivalent to the first one - to compensate rounding
    //errors we add the remaining space of the other columns to the last one.
    pCol = &m_aColumns.back();
    pCol->SetWishWidth( nAvail );
    pCol->SetLeft ( nGutterHalf );
    pCol->SetRight( 0 );

    //Convert the current width to the requested width.
    for ( i = 0; i < m_aColumns.size(); ++i )
    {
        pCol = &m_aColumns[i];
        long nTmp = pCol->GetWishWidth();
        nTmp *= GetWishWidth();
        nTmp /= nAct;
        pCol->SetWishWidth( sal_uInt16(nTmp) );
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
        uno::Reference< text::XTextColumns >  xCols = new SwXTextColumns(*this);
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
            sal_uInt16 nCount = std::min( (sal_uInt16)aSetColumns.getLength(),
                                     (sal_uInt16) 0x3fff );
            sal_uInt16 nWidthSum = 0;
            // #101224# one column is no column

            if(nCount > 1)
                for(sal_uInt16 i = 0; i < nCount; i++)
                {
                    SwColumn aCol;
                    aCol.SetWishWidth(static_cast<sal_uInt16>(pArray[i].Width) );
                    nWidthSum = static_cast<sal_uInt16>(nWidthSum + pArray[i].Width);
                    aCol.SetLeft (static_cast<sal_uInt16>(convertMm100ToTwip(pArray[i].LeftMargin)));
                    aCol.SetRight(static_cast<sal_uInt16>(convertMm100ToTwip(pArray[i].RightMargin)));
                    m_aColumns.insert(m_aColumns.begin() + i, aCol);
                }
            bRet = true;
            m_nWidth = nWidthSum;
            m_bOrtho = false;

            uno::Reference<lang::XUnoTunnel> xNumTunnel(xCols, uno::UNO_QUERY);
            SwXTextColumns* pSwColums = nullptr;
            if(xNumTunnel.is())
            {
                pSwColums = reinterpret_cast< SwXTextColumns * >(
                    sal::static_int_cast< sal_IntPtr >(
                    xNumTunnel->getSomething( SwXTextColumns::getUnoTunnelId() )));
            }
            if(pSwColums)
            {
                m_bOrtho = pSwColums->IsAutomaticWidth();
                m_nLineWidth = pSwColums->GetSepLineWidth();
                m_aLineColor.SetColor(pSwColums->GetSepLineColor());
                m_nLineHeight = pSwColums->GetSepLineHeightRelative();
                switch ( pSwColums->GetSepLineStyle() )
                {
                    default:
                    case 0: m_eLineStyle = table::BorderLineStyle::NONE; break;
                    case 1: m_eLineStyle = table::BorderLineStyle::SOLID; break;
                    case 2: m_eLineStyle = table::BorderLineStyle::DOTTED; break;
                    case 3: m_eLineStyle = table::BorderLineStyle::DASHED; break;
                }
                if(!pSwColums->GetSepLineIsOn())
                    m_eAdj = COLADJ_NONE;
                else switch(pSwColums->GetSepLineVertAlign())
                {
                    case 0: m_eAdj = COLADJ_TOP;  break;  //VerticalAlignment_TOP
                    case 1: m_eAdj = COLADJ_CENTER;break; //VerticalAlignment_MIDDLE
                    case 2: m_eAdj = COLADJ_BOTTOM;break; //VerticalAlignment_BOTTOM
                    default: OSL_ENSURE( false, "unknown alignment" ); break;
                }
            }
        }
    }
    return bRet;
}

void SwFormatCol::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatCol"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eLineStyle"), BAD_CAST(OString::number(m_eLineStyle).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLineWidth"), BAD_CAST(OString::number(m_nLineWidth).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("aLineColor"), BAD_CAST(m_aLineColor.AsRGBHexString().toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLineHeight"), BAD_CAST(OString::number(m_nLineHeight).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eAdj"), BAD_CAST(OString::number(m_eAdj).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWidth"), BAD_CAST(OString::number(m_nWidth).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWidthAdjustValue"), BAD_CAST(OString::number(m_aWidthAdjustValue).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bOrtho"), BAD_CAST(OString::boolean(m_bOrtho).getStr()));

    xmlTextWriterStartElement(pWriter, BAD_CAST("aColumns"));
    for (const SwColumn& rColumn : m_aColumns)
        rColumn.dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);

    xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFormatSurround::SwFormatSurround( SwSurround eFly ) :
    SfxEnumItem( RES_SURROUND, sal_uInt16( eFly ) )
{
    bAnchorOnly = bContour = bOutside = false;
}

SwFormatSurround::SwFormatSurround( const SwFormatSurround &rCpy ) :
    SfxEnumItem( RES_SURROUND, rCpy.GetValue() )
{
    bAnchorOnly = rCpy.bAnchorOnly;
    bContour = rCpy.bContour;
    bOutside = rCpy.bOutside;
}

bool SwFormatSurround::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( GetValue() == static_cast<const SwFormatSurround&>(rAttr).GetValue() &&
             bAnchorOnly== static_cast<const SwFormatSurround&>(rAttr).bAnchorOnly &&
             bContour== static_cast<const SwFormatSurround&>(rAttr).bContour &&
             bOutside== static_cast<const SwFormatSurround&>(rAttr).bOutside );
}

SfxPoolItem*  SwFormatSurround::Clone( SfxItemPool* ) const
{
    return new SwFormatSurround( *this );
}

sal_uInt16  SwFormatSurround::GetValueCount() const
{
    return SURROUND_END - SURROUND_BEGIN;
}

namespace
{
    text::WrapTextMode SwSurroundToWrapMode(SwSurround eSurround)
    {
        text::WrapTextMode eRet;
        switch(eSurround)
        {
            case SURROUND_THROUGHT:
                eRet = css::text::WrapTextMode_THROUGHT;
                break;
            case SURROUND_PARALLEL:
                eRet = css::text::WrapTextMode_PARALLEL;
                break;
            case SURROUND_IDEAL:
                eRet = css::text::WrapTextMode_DYNAMIC;
                break;
            case SURROUND_LEFT:
                eRet = css::text::WrapTextMode_LEFT;
                break;
            case SURROUND_RIGHT:
                eRet = css::text::WrapTextMode_RIGHT;
                break;
            default:
                eRet = css::text::WrapTextMode_NONE;
                break;
        }
        return eRet;
    }
}

bool SwFormatSurround::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_SURROUND_SURROUNDTYPE:
            rVal <<= SwSurroundToWrapMode(GetSurround());
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
            sal_Int32 eVal = SWUnoHelper::GetEnumAsInt32( rVal );
            if( eVal >= 0 && eVal < SURROUND_END )
                SetValue( static_cast<sal_uInt16>(eVal) );
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatSurround"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));

    OUString aPresentation;
    GetPresentation(SfxItemPresentation::Nameless, MapUnit::Map100thMM, MapUnit::Map100thMM, aPresentation);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(aPresentation.toUtf8().getStr()));

    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bAnchorOnly"), BAD_CAST(OString::boolean(bAnchorOnly).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bContour"), BAD_CAST(OString::boolean(bContour).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bOutside"), BAD_CAST(OString::boolean(bOutside).getStr()));

    xmlTextWriterEndElement(pWriter);
}

SvStream& SwFormatVertOrient::Store(SvStream &rStream, sal_uInt16 /*version*/) const
{
#if SAL_TYPES_SIZEOFLONG == 8
    rStream.WriteInt64(m_nYPos);
#else
    rStream.WriteInt32(m_nYPos);
#endif
    rStream.WriteInt16( m_eOrient ).WriteInt16( m_eRelation );
    return rStream;
}

SfxPoolItem* SwFormatVertOrient::Create(SvStream &rStream, sal_uInt16 /*itemVersion*/) const
{
    SwTwips yPos(0);
    sal_Int16 orient(0);
    sal_Int16 relation(0);
    // compatibility hack for Table Auto Format: SwTwips is "long" :(
    // (this means that the file format is platform dependent)
#if SAL_TYPES_SIZEOFLONG == 8
    rStream.ReadInt64(yPos);
#else
    sal_Int32 n;
    rStream.ReadInt32(n);
    yPos = n;
#endif
    rStream.ReadInt16( orient ).ReadInt16( relation );

    return new SwFormatVertOrient(yPos, orient, relation);
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

SfxPoolItem*  SwFormatVertOrient::Clone( SfxItemPool* ) const
{
    return new SwFormatVertOrient( m_nYPos, m_eOrient, m_eRelation );
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
            rVal <<= (sal_Int16)m_eOrient;
        }
        break;
        case MID_VERTORIENT_RELATION:
                rVal <<= (sal_Int16)m_eRelation;
        break;
        case MID_VERTORIENT_POSITION:
                rVal <<= (sal_Int32)convertTwipToMm100(GetPos());
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
                nVal = convertMm100ToTwip(nVal);
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatVertOrient"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nYPos"), BAD_CAST(OString::number(m_nYPos).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eOrient"), BAD_CAST(OString::number(m_eOrient).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eRelation"), BAD_CAST(OString::number(m_eRelation).getStr()));
    xmlTextWriterEndElement(pWriter);
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

SfxPoolItem*  SwFormatHoriOrient::Clone( SfxItemPool* ) const
{
    return new SwFormatHoriOrient( m_nXPos, m_eOrient, m_eRelation, m_bPosToggle );
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
            rVal <<= (sal_Int16)m_eOrient;
        }
        break;
        case MID_HORIORIENT_RELATION:
            rVal <<= (sal_Int16)m_eRelation;
        break;
        case MID_HORIORIENT_POSITION:
                rVal <<= (sal_Int32)convertTwipToMm100(GetPos());
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
                nVal = convertMm100ToTwip(nVal);
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatHoriOrient"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nXPos"), BAD_CAST(OString::number(m_nXPos).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eOrient"), BAD_CAST(OString::number(m_eOrient).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eRelation"), BAD_CAST(OString::number(m_eRelation).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bPosToggle"), BAD_CAST(OString::boolean(m_bPosToggle).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFormatAnchor::SwFormatAnchor( RndStdIds nRnd, sal_uInt16 nPage )
    : SfxPoolItem( RES_ANCHOR ),
    nAnchorId( nRnd ),
    nPageNum( nPage ),
    // OD 2004-05-05 #i28701# - get always new increased order number
    mnOrder( ++mnOrderCounter )
{}

SwFormatAnchor::SwFormatAnchor( const SwFormatAnchor &rCpy )
    : SfxPoolItem( RES_ANCHOR )
    , m_pContentAnchor( (rCpy.GetContentAnchor())
            ?  new SwPosition( *rCpy.GetContentAnchor() ) : nullptr )
    , nAnchorId( rCpy.GetAnchorId() )
    , nPageNum( rCpy.GetPageNum() )
    // OD 2004-05-05 #i28701# - get always new increased order number
    , mnOrder( ++mnOrderCounter )
{
}

SwFormatAnchor::~SwFormatAnchor()
{
}

void SwFormatAnchor::SetAnchor( const SwPosition *pPos )
{
    // anchor only to paragraphs, or start nodes in case of FLY_AT_FLY
    // also allow table node, this is used when a table is selected and is converted to a frame by the UI
    assert(!pPos
            || ((FLY_AT_FLY == nAnchorId) &&
                    dynamic_cast<SwStartNode*>(&pPos->nNode.GetNode()))
            || (FLY_AT_PARA == nAnchorId && dynamic_cast<SwTableNode*>(&pPos->nNode.GetNode()))
            || dynamic_cast<SwTextNode*>(&pPos->nNode.GetNode()));
    m_pContentAnchor .reset( (pPos) ? new SwPosition( *pPos ) : nullptr );
    // Flys anchored AT paragraph should not point into the paragraph content
    if (m_pContentAnchor &&
        ((FLY_AT_PARA == nAnchorId) || (FLY_AT_FLY == nAnchorId)))
    {
        m_pContentAnchor->nContent.Assign( nullptr, 0 );
    }
}

SwFormatAnchor& SwFormatAnchor::operator=(const SwFormatAnchor& rAnchor)
{
    nAnchorId  = rAnchor.GetAnchorId();
    nPageNum   = rAnchor.GetPageNum();
    // OD 2004-05-05 #i28701# - get always new increased order number
    mnOrder = ++mnOrderCounter;

    m_pContentAnchor.reset( (rAnchor.GetContentAnchor())
        ? new SwPosition(*(rAnchor.GetContentAnchor()))
        : nullptr );
    return *this;
}

bool SwFormatAnchor::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    SwFormatAnchor const& rFormatAnchor(static_cast<SwFormatAnchor const&>(rAttr));
    // OD 2004-05-05 #i28701# - Note: <mnOrder> hasn't to be considered.
    return ( nAnchorId == rFormatAnchor.GetAnchorId() &&
             nPageNum == rFormatAnchor.GetPageNum()   &&
                // compare anchor: either both do not point into a textnode or
                // both do (valid m_pContentAnchor) and the positions are equal
             ((m_pContentAnchor.get() == rFormatAnchor.m_pContentAnchor.get()) ||
              (m_pContentAnchor && rFormatAnchor.GetContentAnchor() &&
               (*m_pContentAnchor == *rFormatAnchor.GetContentAnchor()))));
}

SfxPoolItem*  SwFormatAnchor::Clone( SfxItemPool* ) const
{
    return new SwFormatAnchor( *this );
}

// OD 2004-05-05 #i28701#
sal_uInt32 SwFormatAnchor::mnOrderCounter = 0;

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
            switch (GetAnchorId())
            {
                case  FLY_AT_CHAR:
                    eRet = text::TextContentAnchorType_AT_CHARACTER;
                    break;
                case  FLY_AT_PAGE:
                    eRet = text::TextContentAnchorType_AT_PAGE;
                    break;
                case  FLY_AT_FLY:
                    eRet = text::TextContentAnchorType_AT_FRAME;
                    break;
                case  FLY_AS_CHAR:
                    eRet = text::TextContentAnchorType_AS_CHARACTER;
                    break;
                //case  FLY_AT_PARA:
                default:
                    eRet = text::TextContentAnchorType_AT_PARAGRAPH;
            }
            rVal <<= eRet;
        break;
        case MID_ANCHOR_PAGENUM:
            rVal <<= (sal_Int16)GetPageNum();
        break;
        case MID_ANCHOR_ANCHORFRAME:
        {
            if (m_pContentAnchor && FLY_AT_FLY == nAnchorId)
            {
                SwFrameFormat* pFormat = m_pContentAnchor->nNode.GetNode().GetFlyFormat();
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
            switch( SWUnoHelper::GetEnumAsInt32( rVal ) )
            {
                case  text::TextContentAnchorType_AS_CHARACTER:
                    eAnchor = FLY_AS_CHAR;
                    break;
                case  text::TextContentAnchorType_AT_PAGE:
                    eAnchor = FLY_AT_PAGE;
                    if( GetPageNum() > 0 )
                    {
                        // If the anchor type is page and a valid page number
                        // has been set, the content position isn't required
                        // any longer.
                        m_pContentAnchor.reset();
                    }
                    break;
                case  text::TextContentAnchorType_AT_FRAME:
                    eAnchor = FLY_AT_FLY;
                    break;
                case  text::TextContentAnchorType_AT_CHARACTER:
                    eAnchor = FLY_AT_CHAR;
                    break;
                //case  text::TextContentAnchorType_AT_PARAGRAPH:
                default:
                    eAnchor = FLY_AT_PARA;
                    break;
            }
            SetType( eAnchor );
        }
        break;
        case MID_ANCHOR_PAGENUM:
        {
            sal_Int16 nVal = 0;
            if((rVal >>= nVal) && nVal > 0)
            {
                SetPageNum( nVal );
                if (FLY_AT_PAGE == GetAnchorId())
                {
                    // If the anchor type is page and a valid page number
                    // is set, the content position has to be deleted to not
                    // confuse the layout (frmtool.cxx). However, if the
                    // anchor type is not page, any content position will
                    // be kept.
                    m_pContentAnchor.reset();
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatAnchor"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));

    if (m_pContentAnchor)
    {
        std::stringstream aContentAnchor;
        aContentAnchor << *m_pContentAnchor;
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("pContentAnchor"), BAD_CAST(aContentAnchor.str().c_str()));
    }
    else
        xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("pContentAnchor"), "%p", m_pContentAnchor.get());
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nAnchorType"), BAD_CAST(OString::number(nAnchorId).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nPageNum"), BAD_CAST(OString::number(nPageNum).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nOrder"), BAD_CAST(OString::number(mnOrder).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nOrderCounter"), BAD_CAST(OString::number(mnOrderCounter).getStr()));
    OUString aPresentation;
    GetPresentation(SfxItemPresentation::Nameless, MapUnit::Map100thMM, MapUnit::Map100thMM, aPresentation);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(aPresentation.toUtf8().getStr()));

    xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFormatURL::SwFormatURL() :
    SfxPoolItem( RES_URL ),
    pMap( nullptr ),
    bIsServerMap( false )
{
}

SwFormatURL::SwFormatURL( const SwFormatURL &rURL) :
    SfxPoolItem( RES_URL ),
    sTargetFrameName( rURL.GetTargetFrameName() ),
    sURL( rURL.GetURL() ),
    sName( rURL.GetName() ),
    bIsServerMap( rURL.IsServerMap() )
{
    if (rURL.GetMap())
        pMap.reset( new ImageMap( *rURL.GetMap() ) );
}

SwFormatURL::~SwFormatURL()
{
}

bool SwFormatURL::operator==( const SfxPoolItem &rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    const SwFormatURL &rCmp = static_cast<const SwFormatURL&>(rAttr);
    bool bRet = bIsServerMap     == rCmp.IsServerMap() &&
                sURL             == rCmp.GetURL() &&
                sTargetFrameName == rCmp.GetTargetFrameName() &&
                sName            == rCmp.GetName();
    if ( bRet )
    {
        if ( pMap && rCmp.GetMap() )
            bRet = *pMap == *rCmp.GetMap();
        else
            bRet = pMap.get() == rCmp.GetMap();
    }
    return bRet;
}

SfxPoolItem* SwFormatURL::Clone( SfxItemPool* ) const
{
    return new SwFormatURL( *this );
}

void SwFormatURL::SetURL(const OUString &rURL, bool bServerMap)
{
    sURL = rURL;
    bIsServerMap = bServerMap;
}

void SwFormatURL::SetMap( const ImageMap *pM )
{
    pMap.reset( pM ? new ImageMap( *pM ) : nullptr);
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
            if(pMap)
            {
                xInt = SvUnoImageMap_createInstance( *pMap, sw_GetSupportedMacroItems() );
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
            SetURL( sTmp, bIsServerMap );
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
                pMap.reset();
            else if(rVal >>= xCont)
            {
                if(!pMap)
                    pMap.reset(new ImageMap);
                bRet = SvUnoImageMap_fillImageMap( xCont, *pMap );
            }
            else
                bRet = false;
        }
        break;
        case MID_URL_SERVERMAP:
            bIsServerMap = *o3tl::doAccess<bool>(rVal);
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

SfxPoolItem* SwFormatEditInReadonly::Clone( SfxItemPool* ) const
{
    return new SwFormatEditInReadonly( Which(), GetValue() );
}

SfxPoolItem* SwFormatLayoutSplit::Clone( SfxItemPool* ) const
{
    return new SwFormatLayoutSplit( GetValue() );
}

SfxPoolItem* SwFormatRowSplit::Clone( SfxItemPool* ) const
{
    return new SwFormatRowSplit( GetValue() );
}

SfxPoolItem* SwFormatNoBalancedColumns::Clone( SfxItemPool* ) const
{
    return new SwFormatNoBalancedColumns( GetValue() );
}

void SwFormatNoBalancedColumns::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatNoBalancedColumns"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::boolean(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
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
    aFormat = rAttr.aFormat;
    nOffset = rAttr.nOffset;
    sPrefix = rAttr.sPrefix;
    sSuffix = rAttr.sSuffix;
    return *this;
}

bool SwFormatFootnoteEndAtTextEnd::operator==( const SfxPoolItem& rItem ) const
{
    const SwFormatFootnoteEndAtTextEnd& rAttr = static_cast<const SwFormatFootnoteEndAtTextEnd&>(rItem);
    return SfxEnumItem::operator==( rAttr ) &&
            aFormat.GetNumberingType() == rAttr.aFormat.GetNumberingType() &&
            nOffset == rAttr.nOffset &&
            sPrefix == rAttr.sPrefix &&
            sSuffix == rAttr.sSuffix;
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
        case MID_NUM_START_AT: rVal <<= (sal_Int16) nOffset; break;
        case MID_OWN_NUM     :
            rVal <<= GetValue() >= FTNEND_ATTXTEND_OWNNUMANDFMT;
        break;
        case MID_NUM_TYPE    : rVal <<= aFormat.GetNumberingType(); break;
        case MID_PREFIX      : rVal <<= OUString(sPrefix); break;
        case MID_SUFFIX      : rVal <<= OUString(sSuffix); break;
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
                nOffset = nVal;
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
                aFormat.SetNumberingType(nVal);
            else
                bRet = false;
        }
        break;
        case MID_PREFIX      :
        {
            OUString sVal; rVal >>= sVal;
            sPrefix = sVal;
        }
        break;
        case MID_SUFFIX      :
        {
            OUString sVal; rVal >>= sVal;
            sSuffix = sVal;
        }
        break;
        default: bRet = false;
    }
    return bRet;
}

// class SwFormatFootnoteAtTextEnd

SfxPoolItem* SwFormatFootnoteAtTextEnd::Clone( SfxItemPool* ) const
{
    SwFormatFootnoteAtTextEnd* pNew = new SwFormatFootnoteAtTextEnd;
    *pNew = *this;
    return pNew;
}

// class SwFormatEndAtTextEnd

SfxPoolItem* SwFormatEndAtTextEnd::Clone( SfxItemPool* ) const
{
    SwFormatEndAtTextEnd* pNew = new SwFormatEndAtTextEnd;
    *pNew = *this;
    return pNew;
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
    SetPrev( rCpy.GetPrev() );
    SetNext( rCpy.GetNext() );
}

SfxPoolItem* SwFormatChain::Clone( SfxItemPool* ) const
{
    SwFormatChain *pRet = new SwFormatChain;
    pRet->SetPrev( GetPrev() );
    pRet->SetNext( GetNext() );
    return pRet;
}

void SwFormatChain::SetPrev( SwFlyFrameFormat *pFormat )
{
    if ( pFormat )
        pFormat->Add( &aPrev );
    else if ( aPrev.GetRegisteredIn() )
        aPrev.GetRegisteredIn()->Remove( &aPrev );
}

void SwFormatChain::SetNext( SwFlyFrameFormat *pFormat )
{
    if ( pFormat )
        pFormat->Add( &aNext );
    else if ( aNext.GetRegisteredIn() )
        aNext.GetRegisteredIn()->Remove( &aNext );
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
    nStartValue = 0;
    bCountLines = true;
}

SwFormatLineNumber::~SwFormatLineNumber()
{
}

bool SwFormatLineNumber::operator==( const SfxPoolItem &rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return nStartValue  == static_cast<const SwFormatLineNumber&>(rAttr).GetStartValue() &&
           bCountLines  == static_cast<const SwFormatLineNumber&>(rAttr).IsCount();
}

SfxPoolItem* SwFormatLineNumber::Clone( SfxItemPool* ) const
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
            rVal <<= (sal_Int32)GetStartValue();
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

SfxPoolItem* SwTextGridItem::Clone( SfxItemPool* ) const
{
    return new SwTextGridItem( *this );
}

SwTextGridItem& SwTextGridItem::operator=( const SwTextGridItem& rCpy )
{
    m_aColor = rCpy.GetColor();
    m_nLines = rCpy.GetLines();
    m_nBaseHeight = rCpy.GetBaseHeight();
    m_nRubyHeight = rCpy.GetRubyHeight();
    m_eGridType = rCpy.GetGridType();
    m_bRubyTextBelow = rCpy.GetRubyTextBelow();
    m_bPrintGrid = rCpy.GetPrintGrid();
    m_bDisplayGrid = rCpy.GetDisplayGrid();
    m_nBaseWidth = rCpy.GetBaseWidth();
    m_bSnapToChars = rCpy.GetSnapToChars();
    m_bSquaredMode = rCpy.GetSquaredMode();

    return *this;
}

bool SwTextGridItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bRet = true;

    switch( nMemberId & ~CONVERT_TWIPS )
    {
        case MID_GRID_COLOR:
            rVal <<= GetColor().GetColor();
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
            rVal <<= (sal_Int32) convertTwipToMm100(m_nBaseHeight);
            break;
        case MID_GRID_BASEWIDTH:
            OSL_ENSURE( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            rVal <<= (sal_Int32) convertTwipToMm100(m_nBaseWidth);
            break;
        case MID_GRID_RUBYHEIGHT:
            OSL_ENSURE( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            rVal <<= (sal_Int32)convertTwipToMm100(m_nRubyHeight);
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
            sal_Int32 nTmp = 0;
            bRet = (rVal >>= nTmp);
            if( bRet )
                SetColor( Color(nTmp) );
        }
        break;
        case MID_GRID_LINES:
        {
            sal_Int16 nTmp = 0;
            bRet = (rVal >>= nTmp);
            if( bRet && (nTmp >= 0) )
                SetLines( (sal_uInt16)nTmp );
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
            nTmp = convertMm100ToTwip( nTmp );
            if( bRet && (nTmp >= 0) && ( nTmp <= USHRT_MAX) )
            {
                // rhbz#1043551 round up to 5pt -- 0 causes divide-by-zero
                // in layout; 1pt ties the painting code up in knots for
                // minutes with bazillion lines...
#define MIN_TEXTGRID_SIZE 100
                if( (nMemberId & ~CONVERT_TWIPS) == MID_GRID_BASEHEIGHT )
                {
                    nTmp = std::max<sal_Int32>(nTmp, MIN_TEXTGRID_SIZE);
                    SetBaseHeight( (sal_uInt16)nTmp );
                }
                else if( (nMemberId & ~CONVERT_TWIPS) == MID_GRID_BASEWIDTH )
                {
                    nTmp = std::max<sal_Int32>(nTmp, MIN_TEXTGRID_SIZE);
                    SetBaseWidth( (sal_uInt16)nTmp );
                }
                else
                    SetRubyHeight( (sal_uInt16)nTmp );
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

SfxPoolItem* SwHeaderAndFooterEatSpacingItem::Clone( SfxItemPool* ) const
{
    return new SwHeaderAndFooterEatSpacingItem( Which(), GetValue() );
}

// Partially implemented inline in hxx
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( SwFrameFormat )

SwFrameFormat::SwFrameFormat(
    SwAttrPool& rPool,
    const sal_Char* pFormatNm,
    SwFrameFormat *pDrvdFrame,
    sal_uInt16 nFormatWhich,
    const sal_uInt16* pWhichRange)
:   SwFormat(rPool, pFormatNm, (pWhichRange ? pWhichRange : aFrameFormatSetRange), pDrvdFrame, nFormatWhich),
    m_wXObject(),
    maFillAttributes(),
    m_ffList(nullptr),
    m_pOtherTextBoxFormat(nullptr)
{
}

SwFrameFormat::SwFrameFormat(
    SwAttrPool& rPool,
    const OUString &rFormatNm,
    SwFrameFormat *pDrvdFrame,
    sal_uInt16 nFormatWhich,
    const sal_uInt16* pWhichRange)
:   SwFormat(rPool, rFormatNm, (pWhichRange ? pWhichRange : aFrameFormatSetRange), pDrvdFrame, nFormatWhich),
    m_wXObject(),
    maFillAttributes(),
    m_ffList(nullptr),
    m_pOtherTextBoxFormat(nullptr)
{
}

SwFrameFormat::~SwFrameFormat()
{
    if( !GetDoc()->IsInDtor())
    {
        const SwFormatAnchor& rAnchor = GetAnchor();
        if (rAnchor.GetContentAnchor() != nullptr)
        {
            rAnchor.GetContentAnchor()->nNode.GetNode().RemoveAnchoredFly(this);
        }
    }

    if( nullptr != m_pOtherTextBoxFormat )
    {
        m_pOtherTextBoxFormat->SetOtherTextBoxFormat( nullptr );
        m_pOtherTextBoxFormat = nullptr;
    }
}

void SwFrameFormat::SetName( const OUString& rNewName, bool bBroadcast )
{
    if (m_ffList != nullptr) {
        SwFrameFormats::iterator it = m_ffList->find( this );
        assert( m_ffList->end() != it );
        SAL_INFO_IF(m_aFormatName == rNewName, "sw.core", "SwFrmFmt not really renamed, as both names are equal");

        SwStringMsgPoolItem aOld( RES_NAME_CHANGED, m_aFormatName );
        // As it's a non-unique list, rename should never fail!
        bool const renamed =
            m_ffList->m_PosIndex.modify( it,
                change_name( rNewName ), change_name( m_aFormatName ) );
        assert(renamed);
        (void)renamed; // unused in NDEBUG
        if (bBroadcast) {
            SwStringMsgPoolItem aNew( RES_NAME_CHANGED, rNewName );
            ModifyNotification( &aOld, &aNew );
        }
    }
    else
        SwFormat::SetName( rNewName, bBroadcast );
}

void SwFrameFormat::SetOtherTextBoxFormat( SwFrameFormat *pFormat )
{
    if( nullptr != pFormat )
    {
        assert( (Which() == RES_DRAWFRMFMT && pFormat->Which() == RES_FLYFRMFMT)
             || (Which() == RES_FLYFRMFMT && pFormat->Which() == RES_DRAWFRMFMT) );
        assert( nullptr == m_pOtherTextBoxFormat );
    }
    else
    {
        assert( nullptr != m_pOtherTextBoxFormat );
    }
    m_pOtherTextBoxFormat = pFormat;
}

bool SwFrameFormat::supportsFullDrawingLayerFillAttributeSet() const
{
    return true;
}

void SwFrameFormat::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    SwFormatHeader const *pH = nullptr;
    SwFormatFooter const *pF = nullptr;

    const sal_uInt16 nWhich = pNew ? pNew->Which() : 0;

    if( RES_ATTRSET_CHG == nWhich )
    {
        static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState(
            RES_HEADER, false, reinterpret_cast<const SfxPoolItem**>(&pH) );
        static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState(
            RES_FOOTER, false, reinterpret_cast<const SfxPoolItem**>(&pF) );

        //UUUU reset fill information
        if (maFillAttributes.get() && supportsFullDrawingLayerFillAttributeSet())
        {
            SfxItemIter aIter(*static_cast<const SwAttrSetChg*>(pNew)->GetChgSet());
            bool bReset(false);

            for(const SfxPoolItem* pItem = aIter.FirstItem(); pItem && !bReset; pItem = aIter.NextItem())
            {
                bReset = !IsInvalidItem(pItem) && pItem->Which() >= XATTR_FILL_FIRST && pItem->Which() <= XATTR_FILL_LAST;
            }

            if(bReset)
            {
                maFillAttributes.reset();
            }
        }
    }
    else if(RES_FMT_CHG == nWhich)
    {
        //UUUU reset fill information on format change (e.g. style changed)
        if (maFillAttributes.get() && supportsFullDrawingLayerFillAttributeSet())
        {
            maFillAttributes.reset();
        }
    }
    else if( RES_HEADER == nWhich )
        pH = static_cast<const SwFormatHeader*>(pNew);
    else if( RES_FOOTER == nWhich )
        pF = static_cast<const SwFormatFooter*>(pNew);

    if( pH && pH->IsActive() && !pH->GetHeaderFormat() )
    {   //If he doesn't have one, I'll add one
        SwFrameFormat *pFormat = GetDoc()->getIDocumentLayoutAccess().MakeLayoutFormat( RND_STD_HEADER, nullptr );
        const_cast<SwFormatHeader *>(pH)->RegisterToFormat( *pFormat );
    }

    if( pF && pF->IsActive() && !pF->GetFooterFormat() )
    {   //If he doesn't have one, I'll add one
        SwFrameFormat *pFormat = GetDoc()->getIDocumentLayoutAccess().MakeLayoutFormat( RND_STD_FOOTER, nullptr );
        const_cast<SwFormatFooter *>(pF)->RegisterToFormat( *pFormat );
    }

    SwFormat::Modify( pOld, pNew );

    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached uno object
        SetXObject(uno::Reference<uno::XInterface>(nullptr));
    }

    const SwPosition* oldAnchorPosition = nullptr;
    const SwPosition* newAnchorPosition = nullptr;
    if( pNew && pNew->Which() == RES_ATTRSET_CHG )
    {
        const SfxPoolItem* tmp = nullptr;
        static_cast< const SwAttrSetChg* >(pNew)->GetChgSet()->GetItemState( RES_ANCHOR, false, &tmp );
        if( tmp )
            newAnchorPosition = static_cast< const SwFormatAnchor* >( tmp )->GetContentAnchor();
    }
    if( pNew && pNew->Which() == RES_ANCHOR )
        newAnchorPosition = static_cast< const SwFormatAnchor* >( pNew )->GetContentAnchor();
    if( pOld && pOld->Which() == RES_ATTRSET_CHG )
    {
        const SfxPoolItem* tmp = nullptr;
        static_cast< const SwAttrSetChg* >(pOld)->GetChgSet()->GetItemState( RES_ANCHOR, false, &tmp );
        if( tmp )
            oldAnchorPosition = static_cast< const SwFormatAnchor* >( tmp )->GetContentAnchor();
    }
    if( pOld && pOld->Which() == RES_ANCHOR )
        oldAnchorPosition = static_cast< const SwFormatAnchor* >( pOld )->GetContentAnchor();
    if( oldAnchorPosition != nullptr && ( newAnchorPosition == nullptr || oldAnchorPosition->nNode.GetIndex() != newAnchorPosition->nNode.GetIndex()))
    {
        oldAnchorPosition->nNode.GetNode().RemoveAnchoredFly(this);
    }
    if( newAnchorPosition != nullptr && ( oldAnchorPosition == nullptr || oldAnchorPosition->nNode.GetIndex() != newAnchorPosition->nNode.GetIndex()))
    {
        newAnchorPosition->nNode.GetNode().AddAnchoredFly(this);
    }
}

void SwFrameFormat::RegisterToFormat( SwFormat& rFormat )
{
    rFormat.Add( this );
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
    OSL_ENSURE( false, "Sorry not implemented." );
}

SwRect SwFrameFormat::FindLayoutRect( const bool bPrtArea, const Point* pPoint ) const
{
    SwRect aRet;
    SwFrame *pFrame = nullptr;
    if( dynamic_cast<const SwSectionFormat*>( this ) !=  nullptr )
    {
        // get the Frame using Node2Layout
        const SwSectionNode* pSectNd = static_cast<const SwSectionFormat*>(this)->GetSectionNode();
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
                    aRet = pFrame->Prt();
                else
                {
                    aRet = pFrame->Frame();
                    --aRet.Pos().Y();
                }
                pFrame = nullptr;       // the rect is finished by now
            }
        }
    }
    else
    {
        const SwFrameType nFrameType = RES_FLYFRMFMT == Which() ? SwFrameType::Fly : FRM_ALL;
        pFrame = ::GetFrameOfModify( nullptr, *const_cast<SwModify*>(static_cast<SwModify const *>(this)), nFrameType, pPoint);
    }

    if( pFrame )
    {
        if( bPrtArea )
            aRet = pFrame->Prt();
        else
            aRet = pFrame->Frame();
    }
    return aRet;
}

SdrObject* SwFrameFormat::FindRealSdrObject()
{
    if( RES_FLYFRMFMT == Which() )
    {
        Point aNullPt;
        SwFlyFrame* pFly = static_cast<SwFlyFrame*>(::GetFrameOfModify( nullptr, *this, SwFrameType::Fly,
                                                    &aNullPt ));
        return pFly ? pFly->GetVirtDrawObj() : nullptr;
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
    if ((FLY_AT_PAGE != pAnchor->GetAnchorId()) && pAnchor->GetContentAnchor())
    {
        const SwFrameFormats& rFormats = *GetDoc()->GetSpzFrameFormats();
        const SwNode* pFlyNd = pAnchor->GetContentAnchor()->nNode.GetNode().
                                FindFlyStartNode();
        while( pFlyNd )
        {
            // then we walk up using the anchor
            size_t n;
            for( n = 0; n < rFormats.size(); ++n )
            {
                const SwFrameFormat* pFormat = rFormats[ n ];
                const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
                if( pIdx && pFlyNd == &pIdx->GetNode() )
                {
                    if( pFormat == this )
                        return true;

                    pAnchor = &pFormat->GetAnchor();
                    if ((FLY_AT_PAGE == pAnchor->GetAnchorId()) ||
                        !pAnchor->GetContentAnchor() )
                    {
                        return false;
                    }

                    pFlyNd = pAnchor->GetContentAnchor()->nNode.GetNode().
                                FindFlyStartNode();
                    break;
                }
            }
            if( n >= rFormats.size() )
            {
                OSL_ENSURE( false, "Fly section but no format found" );
                return false;
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
    return SW_RES(STR_FRAME);
}

void SwFrameFormat::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFrameFormat"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetName().toUtf8().getStr()));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("whichId"), "%d", Which());

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
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("which"), BAD_CAST(pWhich));

    GetAttrSet().dumpAsXml(pWriter);

    xmlTextWriterEndElement(pWriter);
}

void SwFrameFormats::dumpAsXml(xmlTextWriterPtr pWriter, const char* pName) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST(pName));
    for (const SwFrameFormat *pFormat : m_PosIndex)
        pFormat->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

//  class SwFlyFrameFormat
//  Partially implemented inline in hxx

IMPL_FIXEDMEMPOOL_NEWDEL( SwFlyFrameFormat )

SwFlyFrameFormat::~SwFlyFrameFormat()
{
    SwIterator<SwFlyFrame,SwFormat> aIter( *this );
    SwFlyFrame * pLast = aIter.First();
    if( pLast )
        do
        {
            SwFrame::DestroyFrame(pLast);
        } while( nullptr != ( pLast = aIter.Next() ));

    CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::DYING_FLYFRAMEFORMAT));
}

/// Creates the Frames if the format describes a paragraph-bound frame.
/// MA: 1994-02-14: creates the Frames also for frames anchored at page.
void SwFlyFrameFormat::MakeFrames()
{
    // is there a layout?
    if( !GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell() )
        return;

    SwModify *pModify = nullptr;
    // OD 24.07.2003 #111032# - create local copy of anchor attribute for possible changes.
    SwFormatAnchor aAnchorAttr( GetAnchor() );
    switch( aAnchorAttr.GetAnchorId() )
    {
    case FLY_AS_CHAR:
    case FLY_AT_PARA:
    case FLY_AT_CHAR:
        if( aAnchorAttr.GetContentAnchor() )
        {
            pModify = aAnchorAttr.GetContentAnchor()->nNode.GetNode().GetContentNode();
        }
        break;

    case FLY_AT_FLY:
        if( aAnchorAttr.GetContentAnchor() )
        {
            //First search in the content because this is O(1)
            //This can go wrong for linked frames because in this case it's
            //possible, that no Frame exists for this content.
            //In such a situation we also need to search from StartNode to
            //FrameFormat.
            SwNodeIndex aIdx( aAnchorAttr.GetContentAnchor()->nNode );
            SwContentNode *pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
            // #i105535#
            if ( pCNd == nullptr )
            {
                pCNd = aAnchorAttr.GetContentAnchor()->nNode.GetNode().GetContentNode();
            }
            if ( pCNd )
            {
                if( SwIterator<SwFrame,SwContentNode>( *pCNd ).First() )
                {
                    pModify = pCNd;
                }
            }
            // #i105535#
            if ( pModify == nullptr )
            {
                const SwNodeIndex &rIdx = aAnchorAttr.GetContentAnchor()->nNode;
                SwFrameFormats& rFormats = *GetDoc()->GetSpzFrameFormats();
                for( size_t i = 0; i < rFormats.size(); ++i )
                {
                    SwFrameFormat* pFlyFormat = rFormats[i];
                    if( pFlyFormat->GetContent().GetContentIdx() &&
                        rIdx == *pFlyFormat->GetContent().GetContentIdx() )
                    {
                        pModify = pFlyFormat;
                        break;
                    }
                }
            }
        }
        break;

    case FLY_AT_PAGE:
        {
            sal_uInt16 nPgNum = aAnchorAttr.GetPageNum();
            SwPageFrame *pPage = static_cast<SwPageFrame*>(GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout()->Lower());
            if( nPgNum == 0 && aAnchorAttr.GetContentAnchor() )
            {
                SwContentNode *pCNd = aAnchorAttr.GetContentAnchor()->nNode.GetNode().GetContentNode();
                SwIterator<SwFrame,SwContentNode> aIter( *pCNd );
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

    if( pModify )
    {
        SwIterator<SwFrame,SwModify> aIter( *pModify );
        for( SwFrame *pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
        {
            bool bAdd = !pFrame->IsContentFrame() ||
                            !static_cast<SwContentFrame*>(pFrame)->IsFollow();

            if ( FLY_AT_FLY == aAnchorAttr.GetAnchorId() && !pFrame->IsFlyFrame() )
            {
                SwFrame* pFlyFrame = pFrame->FindFlyFrame();
                if ( pFlyFrame )
                {
                    pFrame = pFlyFrame;
                }
                else
                {
                    aAnchorAttr.SetType( FLY_AT_PARA );
                    SetFormatAttr( aAnchorAttr );
                    MakeFrames();
                    return;
                }
            }

            if( pFrame->GetDrawObjs() )
            {
                // #i28701# - new type <SwSortedObjs>
                SwSortedObjs &rObjs = *pFrame->GetDrawObjs();
                for(SwAnchoredObject* pObj : rObjs)
                {
                    // #i28701# - consider changed type of
                    // <SwSortedObjs> entries.
                    if( dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr &&
                        (&pObj->GetFrameFormat()) == this )
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
                case FLY_AT_FLY:
                    pFly = new SwFlyLayFrame( this, pFrame, pFrame );
                    break;

                case FLY_AT_PARA:
                case FLY_AT_CHAR:
                    pFly = new SwFlyAtContentFrame( this, pFrame, pFrame );
                    break;

                case FLY_AS_CHAR:
                    pFly = new SwFlyInContentFrame( this, pFrame, pFrame );
                    break;

                default:
                    assert(false && "Neuer Ankertyp" );
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
}

SwFlyFrame* SwFlyFrameFormat::GetFrame( const Point* pPoint ) const
{
    return static_cast<SwFlyFrame*>(::GetFrameOfModify( nullptr, *const_cast<SwModify*>(static_cast<SwModify const *>(this)), SwFrameType::Fly,
                                            pPoint ));
}

SwAnchoredObject* SwFlyFrameFormat::GetAnchoredObj() const
{
    SwFlyFrame* pFlyFrame( GetFrame() );
    if ( pFlyFrame )
    {
        return dynamic_cast<SwAnchoredObject*>(pFlyFrame);
    }
    else
    {
        return nullptr;
    }
}

bool SwFlyFrameFormat::GetInfo( SfxPoolItem& rInfo ) const
{
    bool bRet = true;
    switch( rInfo.Which() )
    {
    case RES_CONTENT_VISIBLE:
        {
            static_cast<SwPtrMsgPoolItem&>(rInfo).pObject = SwIterator<SwFrame,SwFormat>( *this ).First();
        }
        bRet = false;
        break;

    default:
        bRet = SwFrameFormat::GetInfo( rInfo );
        break;
    }
    return bRet;
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

    if( bBroadcast )
    {
        SwStringMsgPoolItem aOld( RES_TITLE_CHANGED, pMasterObject->GetTitle() );
        SwStringMsgPoolItem aNew( RES_TITLE_CHANGED, rTitle );
        pMasterObject->SetTitle( rTitle );
        ModifyNotification( &aOld, &aNew );
    }
    else
    {
        pMasterObject->SetTitle( rTitle );
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

void SwFlyFrameFormat::SetObjDescription( const OUString& rDescription, bool bBroadcast )
{
    SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject, "<SwFlyFrameFormat::SetDescription(..)> - missing <SdrObject> instance" );
    msDesc = rDescription;
    if ( !pMasterObject )
    {
        return;
    }

    if( bBroadcast )
    {
        SwStringMsgPoolItem aOld( RES_DESCRIPTION_CHANGED, pMasterObject->GetDescription() );
        SwStringMsgPoolItem aNew( RES_DESCRIPTION_CHANGED, rDescription );
        pMasterObject->SetDescription( rDescription );
        ModifyNotification( &aOld, &aNew );
    }
    else
    {
        pMasterObject->SetDescription( rDescription );
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

/** SwFlyFrameFormat::IsBackgroundTransparent - for #99657#

    OD 22.08.2002 - overriding virtual method and its default implementation,
    because format of fly frame provides transparent backgrounds.
    Method determines, if background of fly frame is transparent.

    @return true, if background color is transparent, but not "no fill"
    or the transparency of a existing background graphic is set.
*/
bool SwFlyFrameFormat::IsBackgroundTransparent() const
{
    //UUUU
    if (supportsFullDrawingLayerFillAttributeSet() && getSdrAllFillAttributesHelper())
    {
        return getSdrAllFillAttributesHelper()->isTransparent();
    }

    // NOTE: If background color is "no fill"/"auto fill" (COL_TRANSPARENT)
    //     and there is no background graphic, it "inherites" the background
    //     from its anchor.
    SvxBrushItem aBackground(makeBackgroundBrushItem());
    if ( (aBackground.GetColor().GetTransparency() != 0) &&
         (aBackground.GetColor() != COL_TRANSPARENT)
       )
    {
        return true;
    }
    else
    {
        const GraphicObject *pTmpGrf = aBackground.GetGraphicObject();
        if ( (pTmpGrf) &&
             (pTmpGrf->GetAttr().GetTransparency() != 0)
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
    //UUUU
    if (supportsFullDrawingLayerFillAttributeSet() && getSdrAllFillAttributesHelper())
    {
        return !getSdrAllFillAttributesHelper()->isUsed();
    }
    else
    {
        SvxBrushItem aBackground(makeBackgroundBrushItem());
        if ( (aBackground.GetColor() == COL_TRANSPARENT) &&
             !(aBackground.GetGraphicObject()) )
        {
            return true;
        }
    }

    return false;
}

SwHandleAnchorNodeChg::SwHandleAnchorNodeChg( SwFlyFrameFormat& _rFlyFrameFormat,
                                              const SwFormatAnchor& _rNewAnchorFormat,
                                              SwFlyFrame* _pKeepThisFlyFrame )
    : mrFlyFrameFormat( _rFlyFrameFormat ),
      mbAnchorNodeChanged( false )
{
    const RndStdIds nNewAnchorType( _rNewAnchorFormat.GetAnchorId() );
    if ( ((nNewAnchorType == FLY_AT_PARA) ||
          (nNewAnchorType == FLY_AT_CHAR)) &&
         _rNewAnchorFormat.GetContentAnchor() &&
         _rNewAnchorFormat.GetContentAnchor()->nNode.GetNode().GetContentNode() )
    {
        const SwFormatAnchor& aOldAnchorFormat( _rFlyFrameFormat.GetAnchor() );
        if ( aOldAnchorFormat.GetAnchorId() == nNewAnchorType &&
             aOldAnchorFormat.GetContentAnchor() &&
             aOldAnchorFormat.GetContentAnchor()->nNode.GetNode().GetContentNode() &&
             aOldAnchorFormat.GetContentAnchor()->nNode !=
                                    _rNewAnchorFormat.GetContentAnchor()->nNode )
        {
            // determine 'old' number of anchor frames
            sal_uInt32 nOldNumOfAnchFrame( 0L );
            SwIterator<SwFrame,SwContentNode> aOldIter( *(aOldAnchorFormat.GetContentAnchor()->nNode.GetNode().GetContentNode()) );
            for( SwFrame* pOld = aOldIter.First(); pOld; pOld = aOldIter.Next() )
            {
                ++nOldNumOfAnchFrame;
            }
            // determine 'new' number of anchor frames
            sal_uInt32 nNewNumOfAnchFrame( 0L );
            SwIterator<SwFrame,SwContentNode> aNewIter( *(_rNewAnchorFormat.GetContentAnchor()->nNode.GetNode().GetContentNode()) );
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
}

SwHandleAnchorNodeChg::~SwHandleAnchorNodeChg()
{
    if ( mbAnchorNodeChanged )
    {
        mrFlyFrameFormat.MakeFrames();
    }
}

//  class SwDrawFrameFormat
//  Partially implemented inline in hxx

IMPL_FIXEDMEMPOOL_NEWDEL( SwDrawFrameFormat )

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
            SdrObject * pSdrObjCopy = pSdrObj->Clone();
            SdrUndoNewObj * pSdrUndo = new SdrUndoNewObj(*pSdrObjCopy);
            m_sSdrObjectCachedComment = pSdrUndo->GetComment();

            delete pSdrUndo;

            m_pSdrObjectCached = pSdrObj;
        }

        aResult = m_sSdrObjectCachedComment;
    }
    else
        aResult = SW_RESSTR(STR_GRAPHIC);

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
        pNd = static_cast<const SwContentFrame*>(pRef)->GetNode()->GetNoTextNode();
        aOrigSz = pNd->GetTwipSize();
    }
    else
    {
        pRef = pFly;
        aOrigSz = pFly->GetFormat()->GetFrameSize().GetSize();
    }

    if( aOrigSz.Width() != 0 && aOrigSz.Height() != 0 )
    {
        Point aPos( rPoint );
        Size aActSz ( pRef == pFly ? pFly->Frame().SSize() : pRef->Prt().SSize() );
        const MapMode aSrc ( MapUnit::MapTwip );
        const MapMode aDest( MapUnit::Map100thMM );
        aOrigSz = OutputDevice::LogicToLogic( aOrigSz, aSrc, aDest );
        aActSz  = OutputDevice::LogicToLogic( aActSz,  aSrc, aDest );
        aPos -= pRef->Frame().Pos();
        aPos -= pRef->Prt().Pos();
        aPos    = OutputDevice::LogicToLogic( aPos, aSrc, aDest );
        sal_uInt32 nFlags = 0;
        if ( pFly != pRef && pNd->IsGrfNode() )
        {
            const sal_uInt16 nMirror = pNd->GetSwAttrSet().
                                        GetMirrorGrf().GetValue();
            if ( RES_MIRROR_GRAPH_BOTH == nMirror )
                nFlags = IMAP_MIRROR_HORZ | IMAP_MIRROR_VERT;
            else if ( RES_MIRROR_GRAPH_VERT == nMirror )
                nFlags = IMAP_MIRROR_VERT;
            else if ( RES_MIRROR_GRAPH_HOR == nMirror )
                nFlags = IMAP_MIRROR_HORZ;

        }
        return const_cast<ImageMap*>(rURL.GetMap())->GetHitIMapObject( aOrigSz,
                                                aActSz, aPos, nFlags );
    }

    return nullptr;
}

//UUUU
drawinglayer::attribute::SdrAllFillAttributesHelperPtr SwFrameFormat::getSdrAllFillAttributesHelper() const
{
    if (supportsFullDrawingLayerFillAttributeSet())
    {
        // create FillAttributes on demand
        if(!maFillAttributes.get())
        {
            const_cast< SwFrameFormat* >(this)->maFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(GetAttrSet()));
        }
    }
    else
    {
        // FALLBACKBREAKHERE assert wrong usage
        OSL_ENSURE(false, "getSdrAllFillAttributesHelper() call only valid for RES_FLYFRMFMT and RES_FRMFMT (!)");
    }

    return maFillAttributes;
}

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
#if OSL_DEBUG_LEVEL > 0
    SwNodes const& rNodes(rDoc.GetNodes());
    sal_uLong const count(rNodes.Count());
    for (sal_uLong i = 0; i != count; ++i)
    {
        SwNode const*const pNode(rNodes[i]);
        std::vector<SwFrameFormat*> const*const pFlys(pNode->GetAnchoredFlys());
        if (pFlys)
        {
            for (auto it = pFlys->begin(); it != pFlys->end(); ++it)
            {
                SwFormatAnchor const& rAnchor((**it).GetAnchor(false));
                assert(&rAnchor.GetContentAnchor()->nNode.GetNode() == pNode);
            }
        }
    }
    SwFrameFormats const*const pSpzFrameFormats(rDoc.GetSpzFrameFormats());
    if (pSpzFrameFormats)
    {
        for (auto it = pSpzFrameFormats->begin(); it != pSpzFrameFormats->end(); ++it)
        {
            SwFormatAnchor const& rAnchor((**it).GetAnchor(false));
            if (FLY_AT_PAGE == rAnchor.GetAnchorId())
            {
                assert(!rAnchor.GetContentAnchor()
                    // for invalid documents that lack text:anchor-page-number
                    // it may have an anchor before MakeFrames() is called
                    || (!SwIterator<SwFrame, SwFrameFormat>(**it).First()));
            }
            else
            {
                SwNode & rNode(rAnchor.GetContentAnchor()->nNode.GetNode());
                std::vector<SwFrameFormat*> const*const pFlys(rNode.GetAnchoredFlys());
                assert(std::find(pFlys->begin(), pFlys->end(), *it) != pFlys->end());
                switch (rAnchor.GetAnchorId())
                {
                    case FLY_AT_FLY:
                        assert(rNode.IsStartNode());
                    break;
                    case FLY_AT_PARA:
                        assert(rNode.IsTextNode() || rNode.IsTableNode());
                    break;
                    case FLY_AS_CHAR:
                    case FLY_AT_CHAR:
                        assert(rNode.IsTextNode());
                    break;
                    default:
                        assert(false);
                    break;
                }
            }
        }
    }
#else
    (void) rDoc;
#endif
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
