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
#include <svtools/unoimap.hxx>
#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>
#include <atrfrm.hxx>
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

TYPEINIT1(SwFmtVertOrient, SfxPoolItem);
TYPEINIT1(SwFmtHoriOrient, SfxPoolItem);
TYPEINIT2(SwFmtHeader,  SfxPoolItem, SwClient );
TYPEINIT2(SwFmtFooter,  SfxPoolItem, SwClient );
TYPEINIT2(SwFmtPageDesc,  SfxPoolItem, SwClient );
TYPEINIT1_AUTOFACTORY(SwFmtLineNumber, SfxPoolItem);

static sal_Int16 lcl_IntToRelation(const uno::Any& rVal)
{
    sal_Int16 nVal = text::RelOrientation::FRAME;
    if (!(rVal >>= nVal))
        SAL_WARN("sw.core", "lcl_IntToRelation: read from Any failed!");
    return nVal;
}

void DelHFFormat( SwClient *pToRemove, SwFrmFmt *pFmt )
{
    //If the client is the last one who uses this format, then we have to delete
    //it - before this is done, we may need to delete the content-section.
    SwDoc* pDoc = pFmt->GetDoc();
    pFmt->Remove( pToRemove );
    if( pDoc->IsInDtor() )
    {
        delete pFmt;
        return;
    }

    // Anything other than frames registered?
    bool bDel = true;
    {
        // nested scope because DTOR of SwClientIter resets the flag bTreeChg.
        // It's suboptimal if the format is deleted beforehand.
        SwIterator<SwClient,SwFrmFmt> aIter(*pFmt);
        for(SwClient* pLast = aIter.First(); bDel && pLast; pLast = aIter.Next())
            if(!pLast->IsA(TYPE(SwFrm)) || !SwXHeadFootText::IsXHeadFootText(pLast))
                bDel = false;
    }

    if ( bDel )
    {
        // If there is a Crsr registered in one of the nodes, we need to call the
        // ParkCrsr in an (arbitrary) shell.
        SwFmtCntnt& rCnt = (SwFmtCntnt&)pFmt->GetCntnt();
        if ( rCnt.GetCntntIdx() )
        {
            SwNode *pNode = 0;
            {
                // #i92993#
                // Begin with start node of page header/footer to assure that
                // complete content is checked for cursors and the complete content
                // is deleted on below made method call <pDoc->getIDocumentContentOperations().DeleteSection(pNode)>
                SwNodeIndex aIdx( *rCnt.GetCntntIdx(), 0 );
                // If there is a Crsr registered in one of the nodes, we need to call the
                // ParkCrsr in an (arbitrary) shell.
                pNode = & aIdx.GetNode();
                sal_uInt32 nEnd = pNode->EndOfSectionIndex();
                while ( aIdx < nEnd )
                {
                    if ( pNode->IsCntntNode() &&
                         static_cast<SwCntntNode*>(pNode)->HasWriterListeners() )
                    {
                        SwCrsrShell *pShell = SwIterator<SwCrsrShell,SwCntntNode>( *static_cast<SwCntntNode*>(pNode) ).First();
                        if( pShell )
                        {
                            pShell->ParkCrsr( aIdx );
                                aIdx = nEnd-1;
                        }
                    }
                    ++aIdx;
                    pNode = & aIdx.GetNode();
                }
            }
            rCnt.SetNewCntntIdx( (const SwNodeIndex*)0 );

            // When deleting a header/footer-format, we ALWAYS need to disable
            // the undo function (Bug 31069)
            ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

            OSL_ENSURE( pNode, "A big problem." );
            pDoc->getIDocumentContentOperations().DeleteSection( pNode );
        }
        delete pFmt;
    }
}

// Partially implemented inline in hxx
SwFmtFrmSize::SwFmtFrmSize( SwFrmSize eSize, SwTwips nWidth, SwTwips nHeight )
    : SfxPoolItem( RES_FRM_SIZE ),
    m_aSize( nWidth, nHeight ),
    m_eFrmHeightType( eSize ),
    m_eFrmWidthType( ATT_FIX_SIZE )
{
    m_nWidthPercent = m_eWidthPercentRelation = m_nHeightPercent = m_eHeightPercentRelation = 0;
}

SwFmtFrmSize& SwFmtFrmSize::operator=( const SwFmtFrmSize& rCpy )
{
    m_aSize = rCpy.GetSize();
    m_eFrmHeightType = rCpy.GetHeightSizeType();
    m_eFrmWidthType = rCpy.GetWidthSizeType();
    m_nHeightPercent = rCpy.GetHeightPercent();
    m_eHeightPercentRelation  = rCpy.GetHeightPercentRelation();
    m_nWidthPercent  = rCpy.GetWidthPercent();
    m_eWidthPercentRelation  = rCpy.GetWidthPercentRelation();
    return *this;
}

bool SwFmtFrmSize::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return( m_eFrmHeightType  == static_cast<const SwFmtFrmSize&>(rAttr).m_eFrmHeightType &&
            m_eFrmWidthType  == static_cast<const SwFmtFrmSize&>(rAttr).m_eFrmWidthType &&
            m_aSize           == static_cast<const SwFmtFrmSize&>(rAttr).GetSize()&&
            m_nWidthPercent   == static_cast<const SwFmtFrmSize&>(rAttr).GetWidthPercent() &&
            m_eWidthPercentRelation == static_cast<const SwFmtFrmSize&>(rAttr).GetWidthPercentRelation() &&
            m_nHeightPercent  == static_cast<const SwFmtFrmSize&>(rAttr).GetHeightPercent() &&
            m_eHeightPercentRelation == static_cast<const SwFmtFrmSize&>(rAttr).GetHeightPercentRelation() );
}

SfxPoolItem*  SwFmtFrmSize::Clone( SfxItemPool* ) const
{
    return new SwFmtFrmSize( *this );
}

bool SwFmtFrmSize::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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
            rVal.setValue(&aTmp, ::cppu::UnoType<awt::Size>::get());
        }
        break;
        case MID_FRMSIZE_REL_HEIGHT:
            rVal <<= (sal_Int16)(GetHeightPercent() != 0xFF ? GetHeightPercent() : 0);
        break;
        case MID_FRMSIZE_REL_HEIGHT_RELATION:
            rVal <<= GetHeightPercentRelation();
        break;
        case MID_FRMSIZE_REL_WIDTH:
            rVal <<= (sal_Int16)(GetWidthPercent() != 0xFF ? GetWidthPercent() : 0);
        break;
        case MID_FRMSIZE_REL_WIDTH_RELATION:
            rVal <<= GetWidthPercentRelation();
        break;
        case MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH:
            rVal <<= 0xFF == GetHeightPercent();
        break;
        case MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT:
            rVal <<= 0xFF == GetWidthPercent();
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

bool SwFmtFrmSize::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
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
                if(aTmp.Height() && aTmp.Width())
                    m_aSize = aTmp;
                else
                    bRet = false;
            }
        }
        break;
        case MID_FRMSIZE_REL_HEIGHT:
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0 && nSet <= 0xfe)
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
            if(nSet >= 0 && nSet <= 0xfe)
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
            bool bSet = *static_cast<sal_Bool const *>(rVal.getValue());
            if(bSet)
                SetHeightPercent(0xff);
            else if( 0xff == GetHeightPercent() )
                SetHeightPercent( 0 );
        }
        break;
        case MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT:
        {
            bool bSet = *static_cast<sal_Bool const *>(rVal.getValue());
            if(bSet)
                SetWidthPercent(0xff);
            else if( 0xff == GetWidthPercent() )
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
                SetHeightSizeType((SwFrmSize)nType);
            }
            else
                bRet = false;
        }
        break;
        case MID_FRMSIZE_IS_AUTO_HEIGHT:
        {
            bool bSet = *static_cast<sal_Bool const *>(rVal.getValue());
            SetHeightSizeType(bSet ? ATT_VAR_SIZE : ATT_FIX_SIZE);
        }
        break;
        case MID_FRMSIZE_WIDTH_TYPE:
        {
            sal_Int16 nType = 0;
            if((rVal >>= nType) && nType >= 0 && nType <= ATT_MIN_SIZE )
            {
                SetWidthSizeType((SwFrmSize)nType);
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

void SwFmtFrmSize::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFmtFrmSize"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));

    std::stringstream aSize;
    aSize << m_aSize;
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("size"), BAD_CAST(aSize.str().c_str()));

    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eFrmHeightType"), BAD_CAST(OString::number(m_eFrmHeightType).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eFrmWidthType"), BAD_CAST(OString::number(m_eFrmWidthType).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWidthPercent"), BAD_CAST(OString::number(m_nWidthPercent).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eWidthPercentRelation"), BAD_CAST(OString::number(m_eWidthPercentRelation).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nHeightPercent"), BAD_CAST(OString::number(m_nHeightPercent).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eHeightPercentRelation"), BAD_CAST(OString::number(m_eHeightPercentRelation).getStr()));

    xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFmtFillOrder::SwFmtFillOrder( SwFillOrder nFO )
    : SfxEnumItem( RES_FILL_ORDER, sal_uInt16(nFO) )
{}
SfxPoolItem*  SwFmtFillOrder::Clone( SfxItemPool* ) const
{
    return new SwFmtFillOrder( GetFillOrder() );
}

sal_uInt16  SwFmtFillOrder::GetValueCount() const
{
    return SW_FILL_ORDER_END - SW_FILL_ORDER_BEGIN;
}

// Partially implemented inline in hxx
SwFmtHeader::SwFmtHeader( SwFrmFmt *pHeaderFmt )
    : SfxPoolItem( RES_HEADER ),
    SwClient( pHeaderFmt ),
    bActive( pHeaderFmt )
{
}

SwFmtHeader::SwFmtHeader( const SwFmtHeader &rCpy )
    : SfxPoolItem( RES_HEADER ),
    SwClient( const_cast<SwModify*>(rCpy.GetRegisteredIn()) ),
    bActive( rCpy.IsActive() )
{
}

SwFmtHeader::SwFmtHeader( bool bOn )
    : SfxPoolItem( RES_HEADER ),
    SwClient( 0 ),
    bActive( bOn )
{
}

 SwFmtHeader::~SwFmtHeader()
{
    if ( GetHeaderFmt() )
        DelHFFormat( this, GetHeaderFmt() );
}

bool SwFmtHeader::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( GetRegisteredIn() == static_cast<const SwFmtHeader&>(rAttr).GetRegisteredIn() &&
             bActive == static_cast<const SwFmtHeader&>(rAttr).IsActive() );
}

SfxPoolItem*  SwFmtHeader::Clone( SfxItemPool* ) const
{
    return new SwFmtHeader( *this );
}

void SwFmtHeader::RegisterToFormat( SwFmt& rFmt )
{
    rFmt.Add(this);
}

// Partially implemented inline in hxx
SwFmtFooter::SwFmtFooter( SwFrmFmt *pFooterFmt )
    : SfxPoolItem( RES_FOOTER ),
    SwClient( pFooterFmt ),
    bActive( pFooterFmt )
{
}

SwFmtFooter::SwFmtFooter( const SwFmtFooter &rCpy )
    : SfxPoolItem( RES_FOOTER ),
    SwClient( const_cast<SwModify*>(rCpy.GetRegisteredIn()) ),
    bActive( rCpy.IsActive() )
{
}

SwFmtFooter::SwFmtFooter( bool bOn )
    : SfxPoolItem( RES_FOOTER ),
    SwClient( 0 ),
    bActive( bOn )
{
}

 SwFmtFooter::~SwFmtFooter()
{
    if ( GetFooterFmt() )
        DelHFFormat( this, GetFooterFmt() );
}

void SwFmtFooter::RegisterToFormat( SwFmt& rFmt )
{
    rFmt.Add(this);
}

bool SwFmtFooter::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( GetRegisteredIn() == static_cast<const SwFmtFooter&>(rAttr).GetRegisteredIn() &&
             bActive == static_cast<const SwFmtFooter&>(rAttr).IsActive() );
}

SfxPoolItem*  SwFmtFooter::Clone( SfxItemPool* ) const
{
    return new SwFmtFooter( *this );
}

// Partially implemented inline in hxx
SwFmtCntnt::SwFmtCntnt( const SwFmtCntnt &rCpy )
    : SfxPoolItem( RES_CNTNT )
{
    pStartNode = rCpy.GetCntntIdx() ?
                    new SwNodeIndex( *rCpy.GetCntntIdx() ) : 0;
}

SwFmtCntnt::SwFmtCntnt( const SwStartNode *pStartNd )
    : SfxPoolItem( RES_CNTNT )
{
    pStartNode = pStartNd ? new SwNodeIndex( *pStartNd ) : 0;
}

 SwFmtCntnt::~SwFmtCntnt()
{
    delete pStartNode;
}

void SwFmtCntnt::SetNewCntntIdx( const SwNodeIndex *pIdx )
{
    delete pStartNode;
    pStartNode = pIdx ? new SwNodeIndex( *pIdx ) : 0;
}

bool SwFmtCntnt::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    if( (bool)pStartNode != (bool)static_cast<const SwFmtCntnt&>(rAttr).pStartNode )
        return false;
    if( pStartNode )
        return ( *pStartNode == *static_cast<const SwFmtCntnt&>(rAttr).GetCntntIdx() );
    return true;
}

SfxPoolItem*  SwFmtCntnt::Clone( SfxItemPool* ) const
{
    return new SwFmtCntnt( *this );
}

void SwFmtCntnt::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFmtCntnt"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("startNode"), BAD_CAST(OString::number(pStartNode->GetNode().GetIndex()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFmtPageDesc::SwFmtPageDesc( const SwFmtPageDesc &rCpy )
    : SfxPoolItem( RES_PAGEDESC ),
    SwClient( const_cast<SwPageDesc*>(rCpy.GetPageDesc()) ),
    oNumOffset( rCpy.oNumOffset ),
    nDescNameIdx( rCpy.nDescNameIdx ),
    pDefinedIn( 0 )
{
}

SwFmtPageDesc::SwFmtPageDesc( const SwPageDesc *pDesc )
    : SfxPoolItem( RES_PAGEDESC ),
    SwClient( const_cast<SwPageDesc*>(pDesc) ),
    nDescNameIdx( 0xFFFF ), // IDX_NO_VALUE
    pDefinedIn( 0 )
{
}

SwFmtPageDesc &SwFmtPageDesc::operator=(const SwFmtPageDesc &rCpy)
{
    if (rCpy.GetPageDesc())
        RegisterToPageDesc(*const_cast<SwPageDesc*>(rCpy.GetPageDesc()));
    oNumOffset = rCpy.oNumOffset;
    nDescNameIdx = rCpy.nDescNameIdx;
    pDefinedIn = 0;

    return *this;
}

 SwFmtPageDesc::~SwFmtPageDesc() {}

bool SwFmtPageDesc::KnowsPageDesc() const
{
    return (GetRegisteredIn() != 0);
}

bool SwFmtPageDesc::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return  ( pDefinedIn == static_cast<const SwFmtPageDesc&>(rAttr).pDefinedIn ) &&
            ( oNumOffset == static_cast<const SwFmtPageDesc&>(rAttr).oNumOffset ) &&
            ( GetPageDesc() == static_cast<const SwFmtPageDesc&>(rAttr).GetPageDesc() );
}

SfxPoolItem*  SwFmtPageDesc::Clone( SfxItemPool* ) const
{
    return new SwFmtPageDesc( *this );
}

void SwFmtPageDesc::SwClientNotify( const SwModify& rModify, const SfxHint& rHint )
{
    SwClient::SwClientNotify(rModify, rHint);
    const SwPageDescHint* pHint = dynamic_cast<const SwPageDescHint*>(&rHint);
    if ( pHint )
    {
        // mba: shouldn't that be broadcasted also?
        SwFmtPageDesc aDfltDesc( pHint->GetPageDesc() );
        SwPageDesc* pDesc = pHint->GetPageDesc();
        const SwModify* pMod = GetDefinedIn();
        if ( pMod )
        {
            if( pMod->ISA( SwCntntNode ) )
                const_cast<SwCntntNode*>(static_cast<const SwCntntNode*>(pMod))->SetAttr( aDfltDesc );
            else if( pMod->ISA( SwFmt ))
                const_cast<SwFmt*>(static_cast<const SwFmt*>(pMod))->SetFmtAttr( aDfltDesc );
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

void SwFmtPageDesc::RegisterToPageDesc( SwPageDesc& rDesc )
{
    rDesc.Add( this );
}

void SwFmtPageDesc::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( !pDefinedIn )
        return;

    const sal_uInt16 nWhichId = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhichId )
    {
        case RES_OBJECTDYING:
                //The Pagedesc where I'm registered dies, therefore I unregister
                //from that format. During this I get deleted!
            if( IS_TYPE( SwFmt, pDefinedIn ))
            {
                bool const bResult =
                    static_cast<SwFmt*>(pDefinedIn)->ResetFmtAttr(RES_PAGEDESC);
                OSL_ENSURE( bResult, "FmtPageDesc not deleted" );
                (void) bResult; // unused in non-debug
            }
            else if( IS_TYPE( SwCntntNode, pDefinedIn ))
            {
                bool const bResult = static_cast<SwCntntNode*>(pDefinedIn)
                        ->ResetAttr(RES_PAGEDESC);
                OSL_ENSURE( bResult, "FmtPageDesc not deleted" );
                (void) bResult; // unused in non-debug
            }
            break;

        default:
            /* do nothing */;
    }
}

bool SwFmtPageDesc::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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
                    SwStyleNameMapper::FillProgName(pDesc->GetName(), aString, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true );
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

bool SwFmtPageDesc::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
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

void SwFmtPageDesc::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFmtPageDesc"));
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

//  class SwFmtCol
//  Partially implemented inline in hxx

SwColumn::SwColumn() :
    nWish ( 0 ),
    nUpper( 0 ),
    nLower( 0 ),
    nLeft ( 0 ),
    nRight( 0 )
{
}

bool SwColumn::operator==( const SwColumn &rCmp ) const
{
    return  nWish    == rCmp.GetWishWidth() &&
            GetLeft()  == rCmp.GetLeft() &&
            GetRight() == rCmp.GetRight() &&
            GetUpper() == rCmp.GetUpper() &&
            GetLower() == rCmp.GetLower();
}

void SwColumn::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swColumn"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWish"), BAD_CAST(OString::number(nWish).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nUpper"), BAD_CAST(OString::number(nUpper).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLower"), BAD_CAST(OString::number(nLower).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLeft"), BAD_CAST(OString::number(nLeft).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nRight"), BAD_CAST(OString::number(nRight).getStr()));
    xmlTextWriterEndElement(pWriter);
}

SwFmtCol::SwFmtCol( const SwFmtCol& rCpy )
    : SfxPoolItem( RES_COL ),
    m_eLineStyle( rCpy.m_eLineStyle ),
    m_nLineWidth( rCpy.m_nLineWidth),
    m_aLineColor( rCpy.m_aLineColor),
    m_nLineHeight( rCpy.GetLineHeight() ),
    m_eAdj( rCpy.GetLineAdj() ),
    m_aColumns( (sal_Int8)rCpy.GetNumCols() ),
    m_nWidth( rCpy.GetWishWidth() ),
    m_aWidthAdjustValue( rCpy.m_aWidthAdjustValue ),
    m_bOrtho( rCpy.IsOrtho() )
{
    for ( sal_uInt16 i = 0; i < rCpy.GetNumCols(); ++i )
    {
        SwColumn *pCol = new SwColumn( rCpy.GetColumns()[i] );
        m_aColumns.push_back( pCol );
    }
}

SwFmtCol::~SwFmtCol() {}

SwFmtCol& SwFmtCol::operator=( const SwFmtCol& rCpy )
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
        SwColumn *pCol = new SwColumn( rCpy.GetColumns()[i] );
        m_aColumns.push_back( pCol );
    }
    return *this;
}

SwFmtCol::SwFmtCol()
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

bool SwFmtCol::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    const SwFmtCol &rCmp = static_cast<const SwFmtCol&>(rAttr);
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

SfxPoolItem*  SwFmtCol::Clone( SfxItemPool* ) const
{
    return new SwFmtCol( *this );
}

sal_uInt16 SwFmtCol::GetGutterWidth( bool bMin ) const
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

void SwFmtCol::SetGutterWidth( sal_uInt16 nNew, sal_uInt16 nAct )
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

void SwFmtCol::Init( sal_uInt16 nNumCols, sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    // Deleting seems to be a bit radical on the first sight; but otherwise we
    // have to initialize all values of the remaining SwCloumns.
    if ( !m_aColumns.empty() )
        m_aColumns.clear();
    for ( sal_uInt16 i = 0; i < nNumCols; ++i )
    {
        SwColumn *pCol = new SwColumn;
        m_aColumns.push_back( pCol );
    }
    m_bOrtho = true;
    m_nWidth = USHRT_MAX;
    if( nNumCols )
        Calc( nGutterWidth, nAct );
}

void SwFmtCol::SetOrtho( bool bNew, sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    m_bOrtho = bNew;
    if ( bNew && !m_aColumns.empty() )
        Calc( nGutterWidth, nAct );
}

sal_uInt16 SwFmtCol::CalcColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const
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

sal_uInt16 SwFmtCol::CalcPrtColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const
{
    assert(nCol < m_aColumns.size());
    sal_uInt16 nRet = CalcColWidth( nCol, nAct );
    const SwColumn *pCol = &m_aColumns[nCol];
    nRet = nRet - pCol->GetLeft();
    nRet = nRet - pCol->GetRight();
    return nRet;
}

void SwFmtCol::Calc( sal_uInt16 nGutterWidth, sal_uInt16 nAct )
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

bool SwFmtCol::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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
        rVal.setValue(&xCols, cppu::UnoType<text::XTextColumns>::get());
    }
    return true;
}

bool SwFmtCol::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
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
                    SwColumn* pCol = new SwColumn;
                    pCol->SetWishWidth( static_cast<sal_uInt16>(pArray[i].Width) );
                    nWidthSum = static_cast<sal_uInt16>(nWidthSum + pArray[i].Width);
                    pCol->SetLeft ( static_cast<sal_uInt16>(convertMm100ToTwip(pArray[i].LeftMargin)) );
                    pCol->SetRight( static_cast<sal_uInt16>(convertMm100ToTwip(pArray[i].RightMargin)) );
                    m_aColumns.insert(m_aColumns.begin() + i, pCol);
                }
            bRet = true;
            m_nWidth = nWidthSum;
            m_bOrtho = false;

            uno::Reference<lang::XUnoTunnel> xNumTunnel(xCols, uno::UNO_QUERY);
            SwXTextColumns* pSwColums = 0;
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

void SwFmtCol::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFmtCol"));
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
SwFmtSurround::SwFmtSurround( SwSurround eFly ) :
    SfxEnumItem( RES_SURROUND, sal_uInt16( eFly ) )
{
    bAnchorOnly = bContour = bOutside = false;
}

SwFmtSurround::SwFmtSurround( const SwFmtSurround &rCpy ) :
    SfxEnumItem( RES_SURROUND, rCpy.GetValue() )
{
    bAnchorOnly = rCpy.bAnchorOnly;
    bContour = rCpy.bContour;
    bOutside = rCpy.bOutside;
}

bool SwFmtSurround::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( GetValue() == static_cast<const SwFmtSurround&>(rAttr).GetValue() &&
             bAnchorOnly== static_cast<const SwFmtSurround&>(rAttr).bAnchorOnly &&
             bContour== static_cast<const SwFmtSurround&>(rAttr).bContour &&
             bOutside== static_cast<const SwFmtSurround&>(rAttr).bOutside );
}

SfxPoolItem*  SwFmtSurround::Clone( SfxItemPool* ) const
{
    return new SwFmtSurround( *this );
}

sal_uInt16  SwFmtSurround::GetValueCount() const
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

bool SwFmtSurround::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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

bool SwFmtSurround::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
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
            SetAnchorOnly( *static_cast<sal_Bool const *>(rVal.getValue()) );
            break;
        case MID_SURROUND_CONTOUR:
            SetContour( *static_cast<sal_Bool const *>(rVal.getValue()) );
            break;
        case MID_SURROUND_CONTOUROUTSIDE:
            SetOutside( *static_cast<sal_Bool const *>(rVal.getValue()) );
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

void SwFmtSurround::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFmtSurround"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));

    OUString aPresentation;
    GetPresentation(SFX_ITEM_PRESENTATION_NAMELESS, SFX_MAPUNIT_100TH_MM, SFX_MAPUNIT_100TH_MM, aPresentation);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(aPresentation.toUtf8().getStr()));

    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bAnchorOnly"), BAD_CAST(OString::boolean(bAnchorOnly).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bContour"), BAD_CAST(OString::boolean(bContour).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bOutside"), BAD_CAST(OString::boolean(bOutside).getStr()));

    xmlTextWriterEndElement(pWriter);
}

SvStream& SwFmtVertOrient::Store(SvStream &rStream, sal_uInt16 /*version*/) const
{
#if SAL_TYPES_SIZEOFLONG == 8
    rStream.WriteInt64(m_nYPos);
#else
    rStream.WriteInt32(m_nYPos);
#endif
    rStream.WriteInt16( m_eOrient ).WriteInt16( m_eRelation );
    return rStream;
}

SfxPoolItem* SwFmtVertOrient::Create(SvStream &rStream, sal_uInt16 /*itemVersion*/) const
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

    return new SwFmtVertOrient(yPos, orient, relation);
}

// Partially implemented inline in hxx
SwFmtVertOrient::SwFmtVertOrient( SwTwips nY, sal_Int16 eVert,
                                  sal_Int16 eRel )
    : SfxPoolItem( RES_VERT_ORIENT ),
    m_nYPos( nY ),
    m_eOrient( eVert ),
    m_eRelation( eRel )
{}

bool SwFmtVertOrient::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( m_nYPos     == static_cast<const SwFmtVertOrient&>(rAttr).m_nYPos &&
             m_eOrient   == static_cast<const SwFmtVertOrient&>(rAttr).m_eOrient &&
             m_eRelation == static_cast<const SwFmtVertOrient&>(rAttr).m_eRelation );
}

SfxPoolItem*  SwFmtVertOrient::Clone( SfxItemPool* ) const
{
    return new SwFmtVertOrient( m_nYPos, m_eOrient, m_eRelation );
}

bool SwFmtVertOrient::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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

bool SwFmtVertOrient::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
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

void SwFmtVertOrient::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFmtVertOrient"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nYPos"), BAD_CAST(OString::number(m_nYPos).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eOrient"), BAD_CAST(OString::number(m_eOrient).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eRelation"), BAD_CAST(OString::number(m_eRelation).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFmtHoriOrient::SwFmtHoriOrient( SwTwips nX, sal_Int16 eHori,
                              sal_Int16 eRel, bool bPos )
    : SfxPoolItem( RES_HORI_ORIENT ),
    m_nXPos( nX ),
    m_eOrient( eHori ),
    m_eRelation( eRel ),
    m_bPosToggle( bPos )
{}

bool SwFmtHoriOrient::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( m_nXPos == static_cast<const SwFmtHoriOrient&>(rAttr).m_nXPos &&
             m_eOrient == static_cast<const SwFmtHoriOrient&>(rAttr).m_eOrient &&
             m_eRelation == static_cast<const SwFmtHoriOrient&>(rAttr).m_eRelation &&
             m_bPosToggle == static_cast<const SwFmtHoriOrient&>(rAttr).m_bPosToggle );
}

SfxPoolItem*  SwFmtHoriOrient::Clone( SfxItemPool* ) const
{
    return new SwFmtHoriOrient( m_nXPos, m_eOrient, m_eRelation, m_bPosToggle );
}

bool SwFmtHoriOrient::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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

bool SwFmtHoriOrient::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
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
                SetPosToggle( *static_cast<sal_Bool const *>(rVal.getValue()));
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

void SwFmtHoriOrient::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFmtHoriOrient"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nXPos"), BAD_CAST(OString::number(m_nXPos).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eOrient"), BAD_CAST(OString::number(m_eOrient).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eRelation"), BAD_CAST(OString::number(m_eRelation).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bPosToggle"), BAD_CAST(OString::boolean(m_bPosToggle).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFmtAnchor::SwFmtAnchor( RndStdIds nRnd, sal_uInt16 nPage )
    : SfxPoolItem( RES_ANCHOR ),
    nAnchorId( nRnd ),
    nPageNum( nPage ),
    // OD 2004-05-05 #i28701# - get always new increased order number
    mnOrder( ++mnOrderCounter )
{}

SwFmtAnchor::SwFmtAnchor( const SwFmtAnchor &rCpy )
    : SfxPoolItem( RES_ANCHOR )
    , m_pCntntAnchor( (rCpy.GetCntntAnchor())
            ?  new SwPosition( *rCpy.GetCntntAnchor() ) : 0 )
    , nAnchorId( rCpy.GetAnchorId() )
    , nPageNum( rCpy.GetPageNum() )
    // OD 2004-05-05 #i28701# - get always new increased order number
    , mnOrder( ++mnOrderCounter )
{
}

SwFmtAnchor::~SwFmtAnchor()
{
}

void SwFmtAnchor::SetAnchor( const SwPosition *pPos )
{
    // anchor only to paragraphs, or start nodes in case of FLY_AT_FLY
    // also allow table node, this is used when a table is selected and is converted to a frame by the UI
    assert(!pPos
            || ((FLY_AT_FLY == nAnchorId) &&
                    dynamic_cast<SwStartNode*>(&pPos->nNode.GetNode()))
            || (FLY_AT_PARA == nAnchorId && dynamic_cast<SwTableNode*>(&pPos->nNode.GetNode()))
            || dynamic_cast<SwTxtNode*>(&pPos->nNode.GetNode()));
    m_pCntntAnchor .reset( (pPos) ? new SwPosition( *pPos ) : 0 );
    // Flys anchored AT paragraph should not point into the paragraph content
    if (m_pCntntAnchor &&
        ((FLY_AT_PARA == nAnchorId) || (FLY_AT_FLY == nAnchorId)))
    {
        m_pCntntAnchor->nContent.Assign( 0, 0 );
    }
}

SwFmtAnchor& SwFmtAnchor::operator=(const SwFmtAnchor& rAnchor)
{
    nAnchorId  = rAnchor.GetAnchorId();
    nPageNum   = rAnchor.GetPageNum();
    // OD 2004-05-05 #i28701# - get always new increased order number
    mnOrder = ++mnOrderCounter;

    m_pCntntAnchor.reset( (rAnchor.GetCntntAnchor())
        ? new SwPosition(*(rAnchor.GetCntntAnchor()))
        : 0 );
    return *this;
}

bool SwFmtAnchor::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    SwFmtAnchor const& rFmtAnchor(static_cast<SwFmtAnchor const&>(rAttr));
    // OD 2004-05-05 #i28701# - Note: <mnOrder> hasn't to be considered.
    return ( nAnchorId == rFmtAnchor.GetAnchorId() &&
             nPageNum == rFmtAnchor.GetPageNum()   &&
                // compare anchor: either both do not point into a textnode or
                // both do (valid m_pCntntAnchor) and the positions are equal
             ((m_pCntntAnchor.get() == rFmtAnchor.m_pCntntAnchor.get()) ||
              (m_pCntntAnchor && rFmtAnchor.GetCntntAnchor() &&
               (*m_pCntntAnchor == *rFmtAnchor.GetCntntAnchor()))));
}

SfxPoolItem*  SwFmtAnchor::Clone( SfxItemPool* ) const
{
    return new SwFmtAnchor( *this );
}

// OD 2004-05-05 #i28701#
sal_uInt32 SwFmtAnchor::mnOrderCounter = 0;

// OD 2004-05-05 #i28701#

bool SwFmtAnchor::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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
            if (m_pCntntAnchor && FLY_AT_FLY == nAnchorId)
            {
                SwFrmFmt* pFmt = m_pCntntAnchor->nNode.GetNode().GetFlyFmt();
                if(pFmt)
                {
                    uno::Reference<text::XTextFrame> const xRet(
                        SwXTextFrame::CreateXTextFrame(*pFmt->GetDoc(), pFmt));
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

bool SwFmtAnchor::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
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
                        m_pCntntAnchor.reset();
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
                    // is set, the content paoition has to be deleted to not
                    // confuse the layout (frmtool.cxx). However, if the
                    // anchor type is not page, any content position will
                    // be kept.
                    m_pCntntAnchor.reset();
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

void SwFmtAnchor::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFmtAnchor"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));

    if (m_pCntntAnchor)
    {
        std::stringstream aCntntAnchor;
        aCntntAnchor << *m_pCntntAnchor;
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("pCntntAnchor"), BAD_CAST(aCntntAnchor.str().c_str()));
    }
    else
        xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("pCntntAnchor"), "%p", m_pCntntAnchor.get());
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nAnchorType"), BAD_CAST(OString::number(nAnchorId).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nPageNum"), BAD_CAST(OString::number(nPageNum).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nOrder"), BAD_CAST(OString::number(mnOrder).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nOrderCounter"), BAD_CAST(OString::number(mnOrderCounter).getStr()));
    OUString aPresentation;
    GetPresentation(SFX_ITEM_PRESENTATION_NAMELESS, SFX_MAPUNIT_100TH_MM, SFX_MAPUNIT_100TH_MM, aPresentation);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(aPresentation.toUtf8().getStr()));

    xmlTextWriterEndElement(pWriter);
}

// Partially implemented inline in hxx
SwFmtURL::SwFmtURL() :
    SfxPoolItem( RES_URL ),
    pMap( 0 ),
    bIsServerMap( false )
{
}

SwFmtURL::SwFmtURL( const SwFmtURL &rURL) :
    SfxPoolItem( RES_URL ),
    sTargetFrameName( rURL.GetTargetFrameName() ),
    sURL( rURL.GetURL() ),
    sName( rURL.GetName() ),
    bIsServerMap( rURL.IsServerMap() )
{
    pMap = rURL.GetMap() ? new ImageMap( *rURL.GetMap() ) : 0;
}

SwFmtURL::~SwFmtURL()
{
    delete pMap;
}

bool SwFmtURL::operator==( const SfxPoolItem &rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    const SwFmtURL &rCmp = static_cast<const SwFmtURL&>(rAttr);
    bool bRet = bIsServerMap     == rCmp.IsServerMap() &&
                sURL             == rCmp.GetURL() &&
                sTargetFrameName == rCmp.GetTargetFrameName() &&
                sName            == rCmp.GetName();
    if ( bRet )
    {
        if ( pMap && rCmp.GetMap() )
            bRet = *pMap == *rCmp.GetMap();
        else
            bRet = pMap == rCmp.GetMap();
    }
    return bRet;
}

SfxPoolItem* SwFmtURL::Clone( SfxItemPool* ) const
{
    return new SwFmtURL( *this );
}

void SwFmtURL::SetURL(const OUString &rURL, bool bServerMap)
{
    sURL = rURL;
    bIsServerMap = bServerMap;
}

void SwFmtURL::SetMap( const ImageMap *pM )
{
    delete pMap;
    pMap = pM ? new ImageMap( *pM ) : 0;
}

bool SwFmtURL::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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

bool SwFmtURL::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
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
                DELETEZ(pMap);
            else if(rVal >>= xCont)
            {
                if(!pMap)
                    pMap = new ImageMap;
                bRet = SvUnoImageMap_fillImageMap( xCont, *pMap );
            }
            else
                bRet = false;
        }
        break;
        case MID_URL_SERVERMAP:
            bIsServerMap = *static_cast<sal_Bool const *>(rVal.getValue());
            break;
        default:
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

SfxPoolItem* SwFmtEditInReadonly::Clone( SfxItemPool* ) const
{
    return new SwFmtEditInReadonly( Which(), GetValue() );
}

SfxPoolItem* SwFmtLayoutSplit::Clone( SfxItemPool* ) const
{
    return new SwFmtLayoutSplit( GetValue() );
}

SfxPoolItem* SwFmtRowSplit::Clone( SfxItemPool* ) const
{
    return new SwFmtRowSplit( GetValue() );
}

SfxPoolItem* SwFmtNoBalancedColumns::Clone( SfxItemPool* ) const
{
    return new SwFmtNoBalancedColumns( GetValue() );
}

void SwFmtNoBalancedColumns::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFmtNoBalancedColumns"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::boolean(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// class SwFmtFtnEndAtTxtEnd

sal_uInt16 SwFmtFtnEndAtTxtEnd::GetValueCount() const
{
    return sal_uInt16( FTNEND_ATTXTEND_END );
}

SwFmtFtnEndAtTxtEnd& SwFmtFtnEndAtTxtEnd::operator=(
                        const SwFmtFtnEndAtTxtEnd& rAttr )
{
    SfxEnumItem::SetValue( rAttr.GetValue() );
    aFmt = rAttr.aFmt;
    nOffset = rAttr.nOffset;
    sPrefix = rAttr.sPrefix;
    sSuffix = rAttr.sSuffix;
    return *this;
}

bool SwFmtFtnEndAtTxtEnd::operator==( const SfxPoolItem& rItem ) const
{
    const SwFmtFtnEndAtTxtEnd& rAttr = static_cast<const SwFmtFtnEndAtTxtEnd&>(rItem);
    return SfxEnumItem::operator==( rAttr ) &&
            aFmt.GetNumberingType() == rAttr.aFmt.GetNumberingType() &&
            nOffset == rAttr.nOffset &&
            sPrefix == rAttr.sPrefix &&
            sSuffix == rAttr.sSuffix;
}

bool SwFmtFtnEndAtTxtEnd::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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
        case MID_NUM_TYPE    : rVal <<= aFmt.GetNumberingType(); break;
        case MID_PREFIX      : rVal <<= OUString(sPrefix); break;
        case MID_SUFFIX      : rVal <<= OUString(sSuffix); break;
        default: return false;
    }
    return true;
}

bool SwFmtFtnEndAtTxtEnd::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bRet = true;
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_COLLECT     :
        {
            bool bVal = *static_cast<sal_Bool const *>(rVal.getValue());
            if(!bVal && GetValue() >= FTNEND_ATTXTEND)
                SetValue(FTNEND_ATPGORDOCEND);
            else if(bVal && GetValue() < FTNEND_ATTXTEND)
                SetValue(FTNEND_ATTXTEND);
        }
        break;
        case MID_RESTART_NUM :
        {
            bool bVal = *static_cast<sal_Bool const *>(rVal.getValue());
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
            bool bVal = *static_cast<sal_Bool const *>(rVal.getValue());
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
                aFmt.SetNumberingType(nVal);
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

// class SwFmtFtnAtTxtEnd

SfxPoolItem* SwFmtFtnAtTxtEnd::Clone( SfxItemPool* ) const
{
    SwFmtFtnAtTxtEnd* pNew = new SwFmtFtnAtTxtEnd;
    *pNew = *this;
    return pNew;
}

// class SwFmtEndAtTxtEnd

SfxPoolItem* SwFmtEndAtTxtEnd::Clone( SfxItemPool* ) const
{
    SwFmtEndAtTxtEnd* pNew = new SwFmtEndAtTxtEnd;
    *pNew = *this;
    return pNew;
}

//class SwFmtChain

bool SwFmtChain::operator==( const SfxPoolItem &rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return GetPrev() == static_cast<const SwFmtChain&>(rAttr).GetPrev() &&
           GetNext() == static_cast<const SwFmtChain&>(rAttr).GetNext();
}

SwFmtChain::SwFmtChain( const SwFmtChain &rCpy ) :
    SfxPoolItem( RES_CHAIN )
{
    SetPrev( rCpy.GetPrev() );
    SetNext( rCpy.GetNext() );
}

SfxPoolItem* SwFmtChain::Clone( SfxItemPool* ) const
{
    SwFmtChain *pRet = new SwFmtChain;
    pRet->SetPrev( GetPrev() );
    pRet->SetNext( GetNext() );
    return pRet;
}

void SwFmtChain::SetPrev( SwFlyFrmFmt *pFmt )
{
    if ( pFmt )
        pFmt->Add( &aPrev );
    else if ( aPrev.GetRegisteredIn() )
        static_cast<SwModify*>(aPrev.GetRegisteredIn())->Remove( &aPrev );
}

void SwFmtChain::SetNext( SwFlyFrmFmt *pFmt )
{
    if ( pFmt )
        pFmt->Add( &aNext );
    else if ( aNext.GetRegisteredIn() )
        static_cast<SwModify*>(aNext.GetRegisteredIn())->Remove( &aNext );
}

bool SwFmtChain::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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

SwFmtLineNumber::SwFmtLineNumber() :
    SfxPoolItem( RES_LINENUMBER )
{
    nStartValue = 0;
    bCountLines = true;
}

SwFmtLineNumber::~SwFmtLineNumber()
{
}

bool SwFmtLineNumber::operator==( const SfxPoolItem &rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return nStartValue  == static_cast<const SwFmtLineNumber&>(rAttr).GetStartValue() &&
           bCountLines  == static_cast<const SwFmtLineNumber&>(rAttr).IsCount();
}

SfxPoolItem* SwFmtLineNumber::Clone( SfxItemPool* ) const
{
    return new SwFmtLineNumber( *this );
}

bool SwFmtLineNumber::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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

bool SwFmtLineNumber::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_LINENUMBER_COUNT:
            SetCountLines( *static_cast<sal_Bool const *>(rVal.getValue()) );
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
            SetRubyTextBelow( *static_cast<sal_Bool const *>(rVal.getValue()) );
            break;
        case MID_GRID_PRINT:
            SetPrintGrid( *static_cast<sal_Bool const *>(rVal.getValue()) );
            break;
        case MID_GRID_DISPLAY:
            SetDisplayGrid( *static_cast<sal_Bool const *>(rVal.getValue()) );
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
            SetSnapToChars( *static_cast<sal_Bool const *>(rVal.getValue()) );
            break;
        case MID_GRID_STANDARD_MODE:
        {
            bool bStandard = *static_cast<sal_Bool const *>(rVal.getValue());
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
TYPEINIT1( SwFrmFmt, SwFmt );
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( SwFrmFmt )

SwFrmFmt::SwFrmFmt(
    SwAttrPool& rPool,
    const sal_Char* pFmtNm,
    SwFrmFmt *pDrvdFrm,
    sal_uInt16 nFmtWhich,
    const sal_uInt16* pWhichRange)
:   SwFmt(rPool, pFmtNm, (pWhichRange ? pWhichRange : aFrmFmtSetRange), pDrvdFrm, nFmtWhich),
    m_wXObject(),
    maFillAttributes()
{
}

SwFrmFmt::SwFrmFmt(
    SwAttrPool& rPool,
    const OUString &rFmtNm,
    SwFrmFmt *pDrvdFrm,
    sal_uInt16 nFmtWhich,
    const sal_uInt16* pWhichRange)
:   SwFmt(rPool, rFmtNm, (pWhichRange ? pWhichRange : aFrmFmtSetRange), pDrvdFrm, nFmtWhich),
    m_wXObject(),
    maFillAttributes()
{
}

SwFrmFmt::~SwFrmFmt()
{
    if( !GetDoc()->IsInDtor())
    {
        const SwFmtAnchor& rAnchor = GetAnchor();
        if (rAnchor.GetCntntAnchor() != nullptr)
        {
            rAnchor.GetCntntAnchor()->nNode.GetNode().RemoveAnchoredFly(this);
        }
    }
}

bool SwFrmFmt::supportsFullDrawingLayerFillAttributeSet() const
{
    return true;
}

void SwFrmFmt::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    SwFmtHeader const *pH = 0;
    SwFmtFooter const *pF = 0;

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
        pH = static_cast<const SwFmtHeader*>(pNew);
    else if( RES_FOOTER == nWhich )
        pF = static_cast<const SwFmtFooter*>(pNew);

    if( pH && pH->IsActive() && !pH->GetHeaderFmt() )
    {   //If he doesn't have one, I'll add one
        SwFrmFmt *pFmt = GetDoc()->getIDocumentLayoutAccess().MakeLayoutFmt( RND_STD_HEADER, 0 );
        const_cast<SwFmtHeader *>(pH)->RegisterToFormat( *pFmt );
    }

    if( pF && pF->IsActive() && !pF->GetFooterFmt() )
    {   //If he doesn't have one, I'll add one
        SwFrmFmt *pFmt = GetDoc()->getIDocumentLayoutAccess().MakeLayoutFmt( RND_STD_FOOTER, 0 );
        const_cast<SwFmtFooter *>(pF)->RegisterToFormat( *pFmt );
    }

    SwFmt::Modify( pOld, pNew );

    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached uno object
        SetXObject(uno::Reference<uno::XInterface>(0));
    }

    const SwPosition* oldAnchorPosition = NULL;
    const SwPosition* newAnchorPosition = NULL;
    if( pNew && pNew->Which() == RES_ATTRSET_CHG )
    {
        const SfxPoolItem* tmp = NULL;
        static_cast< const SwAttrSetChg* >(pNew)->GetChgSet()->GetItemState( RES_ANCHOR, false, &tmp );
        if( tmp )
            newAnchorPosition = static_cast< const SwFmtAnchor* >( tmp )->GetCntntAnchor();
    }
    if( pNew && pNew->Which() == RES_ANCHOR )
        newAnchorPosition = static_cast< const SwFmtAnchor* >( pNew )->GetCntntAnchor();
    if( pOld && pOld->Which() == RES_ATTRSET_CHG )
    {
        const SfxPoolItem* tmp = NULL;
        static_cast< const SwAttrSetChg* >(pOld)->GetChgSet()->GetItemState( RES_ANCHOR, false, &tmp );
        if( tmp )
            oldAnchorPosition = static_cast< const SwFmtAnchor* >( tmp )->GetCntntAnchor();
    }
    if( pOld && pOld->Which() == RES_ANCHOR )
        oldAnchorPosition = static_cast< const SwFmtAnchor* >( pOld )->GetCntntAnchor();
    if( oldAnchorPosition != NULL && ( newAnchorPosition == NULL || oldAnchorPosition->nNode.GetIndex() != newAnchorPosition->nNode.GetIndex()))
    {
        oldAnchorPosition->nNode.GetNode().RemoveAnchoredFly(this);
    }
    if( newAnchorPosition != NULL && ( oldAnchorPosition == NULL || oldAnchorPosition->nNode.GetIndex() != newAnchorPosition->nNode.GetIndex()))
    {
        newAnchorPosition->nNode.GetNode().AddAnchoredFly(this);
    }
}

void SwFrmFmt::RegisterToFormat( SwFmt& rFmt )
{
    rFmt.Add( this );
}

/// Delete all Frms that are registered in aDepend.
void SwFrmFmt::DelFrms()
{
    SwIterator<SwFrm,SwFmt> aIter( *this );
    SwFrm * pLast = aIter.First();
    if( pLast )
        do {
                pLast->Cut();
                SwFrm::DestroyFrm(pLast);
        } while( 0 != ( pLast = aIter.Next() ));
}

void SwFrmFmt::MakeFrms()
{
    OSL_ENSURE( false, "Sorry not implemented." );
}

SwRect SwFrmFmt::FindLayoutRect( const bool bPrtArea, const Point* pPoint,
                                 const bool bCalcFrm ) const
{
    SwRect aRet;
    SwFrm *pFrm = 0;
    if( ISA( SwSectionFmt ) )
    {
        // get the Frame using Node2Layout
        const SwSectionNode* pSectNd = static_cast<const SwSectionFmt*>(this)->GetSectionNode();
        if( pSectNd )
        {
            SwNode2Layout aTmp( *pSectNd, pSectNd->GetIndex() - 1 );
            pFrm = aTmp.NextFrm();

            if( pFrm && !pFrm->KnowsFormat(*this) )
            {
                // the Section doesn't have his own Frame, so if someone
                // needs the real size, we have to implement this by requesting
                // the matching Frame from the end.
                // PROBLEM: what happens if SectionFrames overlaps multiple
                //          pages?
                if( bPrtArea )
                    aRet = pFrm->Prt();
                else
                {
                    aRet = pFrm->Frm();
                    --aRet.Pos().Y();
                }
                pFrm = 0;       // the rect is finished by now
            }
        }
    }
    else
    {
        const sal_uInt16 nFrmType = RES_FLYFRMFMT == Which() ? FRM_FLY : USHRT_MAX;
        pFrm = ::GetFrmOfModify( 0, *(SwModify*)this, nFrmType, pPoint,
                                    0, bCalcFrm );
    }

    if( pFrm )
    {
        if( bPrtArea )
            aRet = pFrm->Prt();
        else
            aRet = pFrm->Frm();
    }
    return aRet;
}

SwContact* SwFrmFmt::FindContactObj()
{
    return SwIterator<SwContact,SwFmt>( *this ).First();
}

SdrObject* SwFrmFmt::FindSdrObject()
{
    // #i30669# - use method <FindContactObj()> instead of
    // duplicated code.
    SwContact* pFoundContact = FindContactObj();
    return pFoundContact ? pFoundContact->GetMaster() : 0;
}

SdrObject* SwFrmFmt::FindRealSdrObject()
{
    if( RES_FLYFRMFMT == Which() )
    {
        Point aNullPt;
        SwFlyFrm* pFly = static_cast<SwFlyFrm*>(::GetFrmOfModify( 0, *this, FRM_FLY,
                                                    &aNullPt, 0, false ));
        return pFly ? pFly->GetVirtDrawObj() : 0;
    }
    return FindSdrObject();
}

bool SwFrmFmt::IsLowerOf( const SwFrmFmt& rFmt ) const
{
    //Also linking from inside to outside or from outside to inside is not
    //allowed.
    SwFlyFrm *pSFly = SwIterator<SwFlyFrm,SwFmt>(*this).First();
    if( pSFly )
    {
        SwFlyFrm *pAskFly = SwIterator<SwFlyFrm,SwFmt>(rFmt).First();
        if( pAskFly )
            return pSFly->IsLowerOf( pAskFly );
    }

    // let's try it using the node positions
    const SwFmtAnchor* pAnchor = &rFmt.GetAnchor();
    if ((FLY_AT_PAGE != pAnchor->GetAnchorId()) && pAnchor->GetCntntAnchor())
    {
        const SwFrmFmts& rFmts = *GetDoc()->GetSpzFrmFmts();
        const SwNode* pFlyNd = pAnchor->GetCntntAnchor()->nNode.GetNode().
                                FindFlyStartNode();
        while( pFlyNd )
        {
            // then we walk up using the anchor
            size_t n;
            for( n = 0; n < rFmts.size(); ++n )
            {
                const SwFrmFmt* pFmt = rFmts[ n ];
                const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                if( pIdx && pFlyNd == &pIdx->GetNode() )
                {
                    if( pFmt == this )
                        return true;

                    pAnchor = &pFmt->GetAnchor();
                    if ((FLY_AT_PAGE == pAnchor->GetAnchorId()) ||
                        !pAnchor->GetCntntAnchor() )
                    {
                        return false;
                    }

                    pFlyNd = pAnchor->GetCntntAnchor()->nNode.GetNode().
                                FindFlyStartNode();
                    break;
                }
            }
            if( n >= rFmts.size() )
            {
                OSL_ENSURE( false, "Fly section but no format found" );
                return false;
            }
        }
    }
    return false;
}

// #i31698#
SwFrmFmt::tLayoutDir SwFrmFmt::GetLayoutDir() const
{
    return SwFrmFmt::HORI_L2R;
}

void SwFrmFmt::SetLayoutDir( const SwFrmFmt::tLayoutDir )
{
    // empty body, because default implementation does nothing
}

// #i28749#
sal_Int16 SwFrmFmt::GetPositionLayoutDir() const
{
    return text::PositionLayoutDir::PositionInLayoutDirOfAnchor;
}
void SwFrmFmt::SetPositionLayoutDir( const sal_Int16 )
{
    // empty body, because default implementation does nothing
}

OUString SwFrmFmt::GetDescription() const
{
    return SW_RES(STR_FRAME);
}

void SwFrmFmt::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFrmFmt"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetName().toUtf8().getStr()));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("whichId"), "%d", Which());

    const char* pWhich = 0;
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

void SwFrmFmts::dumpAsXml(xmlTextWriterPtr pWriter, const char* pName) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST(pName));
    for (size_t i = 0; i < size(); ++i)
        GetFmt(i)->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

//  class SwFlyFrmFmt
//  Partially implemented inline in hxx

TYPEINIT1( SwFlyFrmFmt, SwFrmFmt );
IMPL_FIXEDMEMPOOL_NEWDEL( SwFlyFrmFmt )

SwFlyFrmFmt::~SwFlyFrmFmt()
{
    SwIterator<SwFlyFrm,SwFmt> aIter( *this );
    SwFlyFrm * pLast = aIter.First();
    if( pLast )
        do
        {
            SwFrm::DestroyFrm(pLast);
        } while( 0 != ( pLast = aIter.Next() ));

    SwIterator<SwFlyDrawContact,SwFmt> a2ndIter( *this );
    SwFlyDrawContact* pC = a2ndIter.First();
    if( pC )
        do {
                delete pC;

        } while( 0 != ( pC = a2ndIter.Next() ));
}

/// Creates the Frms if the format describes a paragraph-bound frame.
/// MA: 1994-02-14: creates the Frms also for frames anchored at page.
void SwFlyFrmFmt::MakeFrms()
{
    // is there a layout?
    if( !GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell() )
        return;

    SwModify *pModify = 0;
    // OD 24.07.2003 #111032# - create local copy of anchor attribute for possible changes.
    SwFmtAnchor aAnchorAttr( GetAnchor() );
    switch( aAnchorAttr.GetAnchorId() )
    {
    case FLY_AS_CHAR:
    case FLY_AT_PARA:
    case FLY_AT_CHAR:
        if( aAnchorAttr.GetCntntAnchor() )
        {
            pModify = aAnchorAttr.GetCntntAnchor()->nNode.GetNode().GetCntntNode();
        }
        break;

    case FLY_AT_FLY:
        if( aAnchorAttr.GetCntntAnchor() )
        {
            //First search in the content because this is O(1)
            //This can go wrong for linked frames because in this case it's
            //possible, that no Frame exists for this content.
            //In such a situation we also need to search from StartNode to
            //FrameFormat.
            SwNodeIndex aIdx( aAnchorAttr.GetCntntAnchor()->nNode );
            SwCntntNode *pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
            // #i105535#
            if ( pCNd == 0 )
            {
                pCNd = aAnchorAttr.GetCntntAnchor()->nNode.GetNode().GetCntntNode();
            }
            if ( pCNd )
            {
                if( SwIterator<SwFrm,SwCntntNode>( *pCNd ).First() )
                {
                    pModify = pCNd;
                }
            }
            // #i105535#
            if ( pModify == 0 )
            {
                const SwNodeIndex &rIdx = aAnchorAttr.GetCntntAnchor()->nNode;
                SwFrmFmts& rFmts = *GetDoc()->GetSpzFrmFmts();
                for( size_t i = 0; i < rFmts.size(); ++i )
                {
                    SwFrmFmt* pFlyFmt = rFmts[i];
                    if( pFlyFmt->GetCntnt().GetCntntIdx() &&
                        rIdx == *pFlyFmt->GetCntnt().GetCntntIdx() )
                    {
                        pModify = pFlyFmt;
                        break;
                    }
                }
            }
        }
        break;

    case FLY_AT_PAGE:
        {
            sal_uInt16 nPgNum = aAnchorAttr.GetPageNum();
            SwPageFrm *pPage = static_cast<SwPageFrm*>(GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout()->Lower());
            if( nPgNum == 0 && aAnchorAttr.GetCntntAnchor() )
            {
                SwCntntNode *pCNd = aAnchorAttr.GetCntntAnchor()->nNode.GetNode().GetCntntNode();
                SwIterator<SwFrm,SwCntntNode> aIter( *pCNd );
                for ( SwFrm* pFrm = aIter.First(); pFrm != NULL; pFrm = aIter.Next() )
                {
                    pPage = pFrm->FindPageFrm();
                    if( pPage )
                    {
                        nPgNum = pPage->GetPhyPageNum();
                        aAnchorAttr.SetPageNum( nPgNum );
                        aAnchorAttr.SetAnchor( 0 );
                        SetFmtAttr( aAnchorAttr );
                        break;
                    }
                }
            }
            while ( pPage )
            {
                if ( pPage->GetPhyPageNum() == nPgNum )
                {
                    // #i50432# - adjust synopsis of <PlaceFly(..)>
                    pPage->PlaceFly( 0, this );
                    break;
                }
                pPage = static_cast<SwPageFrm*>(pPage->GetNext());
            }
        }
        break;
    default:
        break;
    }

    if( pModify )
    {
        SwIterator<SwFrm,SwModify> aIter( *pModify );
        for( SwFrm *pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
        {
            bool bAdd = !pFrm->IsCntntFrm() ||
                            !static_cast<SwCntntFrm*>(pFrm)->IsFollow();

            if ( FLY_AT_FLY == aAnchorAttr.GetAnchorId() && !pFrm->IsFlyFrm() )
            {
                SwFrm* pFlyFrm = pFrm->FindFlyFrm();
                if ( pFlyFrm )
                {
                    pFrm = pFlyFrm;
                }
                else
                {
                    aAnchorAttr.SetType( FLY_AT_PARA );
                    SetFmtAttr( aAnchorAttr );
                    MakeFrms();
                    return;
                }
            }

            if( pFrm->GetDrawObjs() )
            {
                // #i28701# - new type <SwSortedObjs>
                SwSortedObjs &rObjs = *pFrm->GetDrawObjs();
                for( size_t i = 0; i < rObjs.size(); ++i)
                {
                    // #i28701# - consider changed type of
                    // <SwSortedObjs> entries.
                    SwAnchoredObject* pObj = rObjs[i];
                    if( pObj->ISA(SwFlyFrm) &&
                        (&pObj->GetFrmFmt()) == this )
                    {
                        bAdd = false;
                        break;
                    }
                }
            }

            if( bAdd )
            {
                SwFlyFrm *pFly;
                switch( aAnchorAttr.GetAnchorId() )
                {
                case FLY_AT_FLY:
                    pFly = new SwFlyLayFrm( this, pFrm, pFrm );
                    break;

                case FLY_AT_PARA:
                case FLY_AT_CHAR:
                    pFly = new SwFlyAtCntFrm( this, pFrm, pFrm );
                    break;

                default:
                    assert(false && "Neuer Ankertyp" );
                    //fall-through
                case FLY_AS_CHAR:
                    pFly = new SwFlyInCntFrm( this, pFrm, pFrm );
                    break;
                }
                pFrm->AppendFly( pFly );
                pFly->GetFmt()->SetObjTitle(GetObjTitle());
                pFly->GetFmt()->SetObjDescription(GetObjDescription());
                SwPageFrm *pPage = pFly->FindPageFrm();
                if( pPage )
                    ::RegistFlys( pPage, pFly );
            }
        }
    }
}

SwFlyFrm* SwFlyFrmFmt::GetFrm( const Point* pPoint, const bool bCalcFrm ) const
{
    return static_cast<SwFlyFrm*>(::GetFrmOfModify( 0, *(SwModify*)this, FRM_FLY,
                                            pPoint, 0, bCalcFrm ));
}

SwAnchoredObject* SwFlyFrmFmt::GetAnchoredObj( const Point* pPoint, const bool bCalcFrm ) const
{
    SwFlyFrm* pFlyFrm( GetFrm( pPoint, bCalcFrm ) );
    if ( pFlyFrm )
    {
        return dynamic_cast<SwAnchoredObject*>(pFlyFrm);
    }
    else
    {
        return 0L;
    }
}

bool SwFlyFrmFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    bool bRet = true;
    switch( rInfo.Which() )
    {
    case RES_CONTENT_VISIBLE:
        {
            static_cast<SwPtrMsgPoolItem&>(rInfo).pObject = SwIterator<SwFrm,SwFmt>( *this ).First();
        }
        bRet = false;
        break;

    default:
        bRet = SwFrmFmt::GetInfo( rInfo );
        break;
    }
    return bRet;
}

// #i73249#
void SwFlyFrmFmt::SetObjTitle( const OUString& rTitle, bool bBroadcast )
{
    SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject, "<SwFlyFrmFmt::SetObjTitle(..)> - missing <SdrObject> instance" );
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

OUString SwFlyFrmFmt::GetObjTitle() const
{
    const SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject, "<SwFlyFrmFmt::GetObjTitle(..)> - missing <SdrObject> instance" );
    if ( !pMasterObject )
    {
        return msTitle;
    }
    if (!pMasterObject->GetTitle().isEmpty())
        return pMasterObject->GetTitle();
    else
        return msTitle;
}

void SwFlyFrmFmt::SetObjDescription( const OUString& rDescription, bool bBroadcast )
{
    SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject, "<SwFlyFrmFmt::SetDescription(..)> - missing <SdrObject> instance" );
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

OUString SwFlyFrmFmt::GetObjDescription() const
{
    const SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject, "<SwFlyFrmFmt::GetDescription(..)> - missing <SdrObject> instance" );
    if ( !pMasterObject )
    {
        return msDesc;
    }
    if (!pMasterObject->GetDescription().isEmpty())
        return pMasterObject->GetDescription();
    else
        return msDesc;
}

/** SwFlyFrmFmt::IsBackgroundTransparent - for #99657#

    OD 22.08.2002 - overriding virtual method and its default implementation,
    because format of fly frame provides transparent backgrounds.
    Method determines, if background of fly frame is transparent.

    @return true, if background color is transparent, but not "no fill"
    or the transparency of a existing background graphic is set.
*/
bool SwFlyFrmFmt::IsBackgroundTransparent() const
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
        const GraphicObject *pTmpGrf =
                static_cast<const GraphicObject*>(aBackground.GetGraphicObject());
        if ( (pTmpGrf) &&
             (pTmpGrf->GetAttr().GetTransparency() != 0)
           )
        {
            return true;
        }
    }

    return false;
}

/** SwFlyFrmFmt::IsBackgroundBrushInherited - for #103898#

    OD 08.10.2002 - method to determine, if the brush for drawing the
    background is "inherited" from its parent/grandparent.
    This is the case, if no background graphic is set and the background
    color is "no fill"/"auto fill"
    NOTE: condition is "copied" from method <SwFrm::GetBackgroundBrush(..).

    @return true, if background brush is "inherited" from parent/grandparent
*/
bool SwFlyFrmFmt::IsBackgroundBrushInherited() const
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

// #125892#
SwHandleAnchorNodeChg::SwHandleAnchorNodeChg( SwFlyFrmFmt& _rFlyFrmFmt,
                                              const SwFmtAnchor& _rNewAnchorFmt,
                                              SwFlyFrm* _pKeepThisFlyFrm )
    : mrFlyFrmFmt( _rFlyFrmFmt ),
      mbAnchorNodeChanged( false )
{
    const RndStdIds nNewAnchorType( _rNewAnchorFmt.GetAnchorId() );
    if ( ((nNewAnchorType == FLY_AT_PARA) ||
          (nNewAnchorType == FLY_AT_CHAR)) &&
         _rNewAnchorFmt.GetCntntAnchor() &&
         _rNewAnchorFmt.GetCntntAnchor()->nNode.GetNode().GetCntntNode() )
    {
        const SwFmtAnchor& aOldAnchorFmt( _rFlyFrmFmt.GetAnchor() );
        if ( aOldAnchorFmt.GetAnchorId() == nNewAnchorType &&
             aOldAnchorFmt.GetCntntAnchor() &&
             aOldAnchorFmt.GetCntntAnchor()->nNode.GetNode().GetCntntNode() &&
             aOldAnchorFmt.GetCntntAnchor()->nNode !=
                                    _rNewAnchorFmt.GetCntntAnchor()->nNode )
        {
            // determine 'old' number of anchor frames
            sal_uInt32 nOldNumOfAnchFrm( 0L );
            SwIterator<SwFrm,SwCntntNode> aOldIter( *(aOldAnchorFmt.GetCntntAnchor()->nNode.GetNode().GetCntntNode()) );
            for( SwFrm* pOld = aOldIter.First(); pOld; pOld = aOldIter.Next() )
            {
                ++nOldNumOfAnchFrm;
            }
            // determine 'new' number of anchor frames
            sal_uInt32 nNewNumOfAnchFrm( 0L );
            SwIterator<SwFrm,SwCntntNode> aNewIter( *(_rNewAnchorFmt.GetCntntAnchor()->nNode.GetNode().GetCntntNode()) );
            for( SwFrm* pNew = aNewIter.First(); pNew; pNew = aNewIter.Next() )
            {
                ++nNewNumOfAnchFrm;
            }
            if ( nOldNumOfAnchFrm != nNewNumOfAnchFrm )
            {
                // delete existing fly frames except <_pKeepThisFlyFrm>
                SwIterator<SwFrm,SwFmt> aIter( mrFlyFrmFmt );
                SwFrm* pFrm = aIter.First();
                if ( pFrm )
                {
                    do {
                        if ( pFrm != _pKeepThisFlyFrm )
                        {
                            pFrm->Cut();
                            SwFrm::DestroyFrm(pFrm);
                        }
                    } while( 0 != ( pFrm = aIter.Next() ));
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
        mrFlyFrmFmt.MakeFrms();
    }
}

//  class SwDrawFrmFmt
//  Partially implemented inline in hxx

TYPEINIT1( SwDrawFrmFmt, SwFrmFmt );
IMPL_FIXEDMEMPOOL_NEWDEL( SwDrawFrmFmt )

SwDrawFrmFmt::~SwDrawFrmFmt()
{
    SwContact *pContact = FindContactObj();
    delete pContact;
}

void SwDrawFrmFmt::MakeFrms()
{
    SwDrawContact *pContact = static_cast<SwDrawContact*>(FindContactObj());
    if ( pContact )
         pContact->ConnectToLayout();
}

void SwDrawFrmFmt::DelFrms()
{
    SwDrawContact *pContact = static_cast<SwDrawContact *>(FindContactObj());
    if ( pContact ) //for the reader and other unpredictable things.
        pContact->DisconnectFromLayout();
}

// #i31698#
SwFrmFmt::tLayoutDir SwDrawFrmFmt::GetLayoutDir() const
{
    return meLayoutDir;
}

void SwDrawFrmFmt::SetLayoutDir( const SwFrmFmt::tLayoutDir _eLayoutDir )
{
    meLayoutDir = _eLayoutDir;
}

// #i28749#
sal_Int16 SwDrawFrmFmt::GetPositionLayoutDir() const
{
    return mnPositionLayoutDir;
}
void SwDrawFrmFmt::SetPositionLayoutDir( const sal_Int16 _nPositionLayoutDir )
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
            OSL_FAIL( "<SwDrawFrmFmt::SetPositionLayoutDir(..)> - invalid attribute value." );
        }
    }
}

OUString SwDrawFrmFmt::GetDescription() const
{
    OUString aResult;
    const SdrObject * pSdrObj = FindSdrObject();

    if (pSdrObj)
    {
        if (pSdrObj != pSdrObjCached)
        {
            SdrObject * pSdrObjCopy = pSdrObj->Clone();
            SdrUndoNewObj * pSdrUndo = new SdrUndoNewObj(*pSdrObjCopy);
            sSdrObjCachedComment = pSdrUndo->GetComment();

            delete pSdrUndo;

            pSdrObjCached = pSdrObj;
        }

        aResult = sSdrObjCachedComment;
    }
    else
        aResult = SW_RESSTR(STR_GRAPHIC);

    return aResult;
}

IMapObject* SwFrmFmt::GetIMapObject( const Point& rPoint,
                                        const SwFlyFrm *pFly ) const
{
    const SwFmtURL &rURL = GetURL();
    if( !rURL.GetMap() )
        return 0;

    if( !pFly )
    {
        pFly = SwIterator<SwFlyFrm,SwFmt>( *this ).First();
        if( !pFly )
            return 0;
    }

    //Original size for OLE and graphic is TwipSize, otherwise the size of
    //FrmFmt of the Fly.
    const SwFrm *pRef;
    const SwNoTxtNode *pNd = 0;
    Size aOrigSz;
    if( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
    {
        pRef = pFly->Lower();
        pNd = static_cast<const SwCntntFrm*>(pRef)->GetNode()->GetNoTxtNode();
        aOrigSz = pNd->GetTwipSize();
    }
    else
    {
        pRef = pFly;
        aOrigSz = pFly->GetFmt()->GetFrmSize().GetSize();
    }

    if( aOrigSz.Width() != 0 && aOrigSz.Height() != 0 )
    {
        Point aPos( rPoint );
        Size aActSz ( pRef == pFly ? pFly->Frm().SSize() : pRef->Prt().SSize() );
        const MapMode aSrc ( MAP_TWIP );
        const MapMode aDest( MAP_100TH_MM );
        aOrigSz = OutputDevice::LogicToLogic( aOrigSz, aSrc, aDest );
        aActSz  = OutputDevice::LogicToLogic( aActSz,  aSrc, aDest );
        aPos -= pRef->Frm().Pos();
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

    return 0;
}

//UUUU
drawinglayer::attribute::SdrAllFillAttributesHelperPtr SwFrmFmt::getSdrAllFillAttributesHelper() const
{
    if (supportsFullDrawingLayerFillAttributeSet())
    {
        // create FillAttributes on demand
        if(!maFillAttributes.get())
        {
            const_cast< SwFrmFmt* >(this)->maFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(GetAttrSet()));
        }
    }
    else
    {
        // FALLBACKBREAKHERE assert wrong usage
        OSL_ENSURE(false, "getSdrAllFillAttributesHelper() call only valid for RES_FLYFRMFMT and RES_FRMFMT (!)");
    }

    return maFillAttributes;
}

bool IsFlyFrmFmtInHeader(const SwFrmFmt& rFmt)
{
    const SwFlyFrmFmt* pFlyFrmFmt = dynamic_cast<const SwFlyFrmFmt*>(&rFmt);
    if (!pFlyFrmFmt)
        return false;
    SwFlyFrm* pFlyFrm = const_cast<SwFlyFrm*>(pFlyFrmFmt->GetFrm());
    if (!pFlyFrm) // fdo#54648: "hidden" drawing object has no layout frame
    {
        return false;
    }
    SwPageFrm* pPageFrm = pFlyFrm->FindPageFrmOfAnchor();
    SwFrm* pHeader = pPageFrm->Lower();
    if (pHeader->GetType() == FRM_HEADER)
    {
        const SwFrm* pFrm = pFlyFrm->GetAnchorFrm();
        while (pFrm)
        {
            if (pFrm == pHeader)
                return true;
            pFrm = pFrm->GetUpper();
        }
    }
    return false;
}

namespace sw {

void CheckAnchoredFlyConsistency(SwDoc const& rDoc)
{
#if OSL_DEBUG_LEVEL > 0
    SwNodes const& rNodes(rDoc.GetNodes());
    sal_uLong const count(rNodes.Count());
    for (sal_uLong i = 0; i != count; ++i)
    {
        SwNode const*const pNode(rNodes[i]);
        std::vector<SwFrmFmt*> const*const pFlys(pNode->GetAnchoredFlys());
        if (pFlys)
        {
            for (auto it = pFlys->begin(); it != pFlys->end(); ++it)
            {
                SwFmtAnchor const& rAnchor((**it).GetAnchor(false));
                assert(&rAnchor.GetCntntAnchor()->nNode.GetNode() == pNode);
            }
        }
    }
    SwFrmFmts const*const pSpzFrmFmts(rDoc.GetSpzFrmFmts());
    if (pSpzFrmFmts)
    {
        for (auto it = pSpzFrmFmts->begin(); it != pSpzFrmFmts->end(); ++it)
        {
            SwFmtAnchor const& rAnchor((**it).GetAnchor(false));
            if (FLY_AT_PAGE == rAnchor.GetAnchorId())
            {
                assert(!rAnchor.GetCntntAnchor());
            }
            else
            {
                SwNode & rNode(rAnchor.GetCntntAnchor()->nNode.GetNode());
                std::vector<SwFrmFmt*> const*const pFlys(rNode.GetAnchoredFlys());
                assert(std::find(pFlys->begin(), pFlys->end(), *it) != pFlys->end());
                switch (rAnchor.GetAnchorId())
                {
                    case FLY_AT_FLY:
                        assert(rNode.IsStartNode());
                    break;
                    case FLY_AT_PARA:
                        assert(rNode.IsTxtNode() || rNode.IsTableNode());
                    break;
                    case FLY_AS_CHAR:
                    case FLY_AT_CHAR:
                        assert(rNode.IsTxtNode());
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
