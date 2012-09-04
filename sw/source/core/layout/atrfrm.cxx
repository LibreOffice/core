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
#include <svx/svdundo.hxx> // #111827#
#include <sortedobjs.hxx>
#include <HandleAnchorNodeChg.hxx>
#include <switerator.hxx>
#include <pagedeschint.hxx>
#ifndef NDEBUG
#include <ndtxt.hxx>
#endif

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
        SwClientIter aIter( *pFmt );        // TODO
        SwClient *pLast = aIter.GoStart();
        if( pLast )
            do {
                bDel = pLast->IsA( TYPE(SwFrm) )
                    || SwXHeadFootText::IsXHeadFootText(pLast);
            } while( bDel && 0 != ( pLast = ++aIter ));
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
                // is deleted on below made method call <pDoc->DeleteSection(pNode)>
//                SwNodeIndex aIdx( *rCnt.GetCntntIdx(), 1 );
                SwNodeIndex aIdx( *rCnt.GetCntntIdx(), 0 );
                // If there is a Crsr registered in one of the nodes, we need to call the
                // ParkCrsr in an (arbitrary) shell.
                pNode = & aIdx.GetNode();
                sal_uInt32 nEnd = pNode->EndOfSectionIndex();
                while ( aIdx < nEnd )
                {
                    if ( pNode->IsCntntNode() &&
                         ((SwCntntNode*)pNode)->GetDepends() )
                    {
                        SwCrsrShell *pShell = SwIterator<SwCrsrShell,SwCntntNode>::FirstElement( *(SwCntntNode*)pNode );
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
            pDoc->DeleteSection( pNode );
        }
        delete pFmt;
    }
}

//  class SwFmtFrmSize
//  Partially implemented inline in hxx

SwFmtFrmSize::SwFmtFrmSize( SwFrmSize eSize, SwTwips nWidth, SwTwips nHeight )
    : SfxPoolItem( RES_FRM_SIZE ),
    aSize( nWidth, nHeight ),
    eFrmHeightType( eSize ),
    eFrmWidthType( ATT_FIX_SIZE )
{
    nWidthPercent = nHeightPercent = 0;
}

SwFmtFrmSize& SwFmtFrmSize::operator=( const SwFmtFrmSize& rCpy )
{
    aSize = rCpy.GetSize();
    eFrmHeightType = rCpy.GetHeightSizeType();
    eFrmWidthType = rCpy.GetWidthSizeType();
    nHeightPercent = rCpy.GetHeightPercent();
    nWidthPercent  = rCpy.GetWidthPercent();
    return *this;
}

int  SwFmtFrmSize::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return( eFrmHeightType  == ((SwFmtFrmSize&)rAttr).eFrmHeightType &&
            eFrmWidthType  == ((SwFmtFrmSize&)rAttr).eFrmWidthType &&
            aSize           == ((SwFmtFrmSize&)rAttr).GetSize()&&
            nWidthPercent   == ((SwFmtFrmSize&)rAttr).GetWidthPercent() &&
            nHeightPercent  == ((SwFmtFrmSize&)rAttr).GetHeightPercent() );
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
            aTmp.Height = TWIP_TO_MM100(aSize.Height());
            aTmp.Width = TWIP_TO_MM100(aSize.Width());
            rVal.setValue(&aTmp, ::getCppuType((const awt::Size*)0));
        }
        break;
        case MID_FRMSIZE_REL_HEIGHT:
            rVal <<= (sal_Int16)(GetHeightPercent() != 0xFF ? GetHeightPercent() : 0);
        break;
        case MID_FRMSIZE_REL_WIDTH:
            rVal <<= (sal_Int16)(GetWidthPercent() != 0xFF ? GetWidthPercent() : 0);
        break;
        case MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH:
        {
            sal_Bool bTmp = 0xFF == GetHeightPercent();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
        break;
        case MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT:
        {
            sal_Bool bTmp = 0xFF == GetWidthPercent();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
        break;
        case MID_FRMSIZE_WIDTH :
            rVal <<= (sal_Int32)TWIP_TO_MM100(aSize.Width());
        break;
        case MID_FRMSIZE_HEIGHT:
            // #95848# returned size should never be zero.
            // (there was a bug that allowed for setting height to 0.
            // Thus there some documents existing with that not allowed
            // attribut value which may cause problems on import.)
            rVal <<= (sal_Int32)TWIP_TO_MM100(aSize.Height() < MINLAY ? MINLAY : aSize.Height() );
        break;
        case MID_FRMSIZE_SIZE_TYPE:
            rVal <<= (sal_Int16)GetHeightSizeType();
        break;
        case MID_FRMSIZE_IS_AUTO_HEIGHT:
        {
            sal_Bool bTmp = ATT_FIX_SIZE != GetHeightSizeType();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
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
                    aTmp.Height() = MM100_TO_TWIP(aTmp.Height());
                    aTmp.Width() = MM100_TO_TWIP(aTmp.Width());
                }
                if(aTmp.Height() && aTmp.Width())
                    aSize = aTmp;
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
        case MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH:
        {
            sal_Bool bSet = *(sal_Bool*)rVal.getValue();
            if(bSet)
                SetHeightPercent(0xff);
            else if( 0xff == GetHeightPercent() )
                SetHeightPercent( 0 );
        }
        break;
        case MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT:
        {
            sal_Bool bSet = *(sal_Bool*)rVal.getValue();
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
                    nWd = MM100_TO_TWIP(nWd);
                if(nWd < MINLAY)
                   nWd = MINLAY;
                aSize.Width() = nWd;
            }
            else
                bRet = sal_False;
        }
        break;
        case MID_FRMSIZE_HEIGHT:
        {
            sal_Int32 nHg = 0;
            if(rVal >>= nHg)
            {
                if(bConvert)
                    nHg = MM100_TO_TWIP(nHg);
                if(nHg < MINLAY)
                    nHg = MINLAY;
                aSize.Height() = nHg;
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
            sal_Bool bSet = *(sal_Bool*)rVal.getValue();
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

//  class SwFmtFillOrder
//  Partially implemented inline in hxx

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

//  class SwFmtHeader
//  Partially implemented inline in hxx

SwFmtHeader::SwFmtHeader( SwFrmFmt *pHeaderFmt )
    : SfxPoolItem( RES_HEADER ),
    SwClient( pHeaderFmt ),
    bActive( pHeaderFmt ? sal_True : sal_False )
{
}

SwFmtHeader::SwFmtHeader( const SwFmtHeader &rCpy )
    : SfxPoolItem( RES_HEADER ),
    SwClient( (SwModify*)rCpy.GetRegisteredIn() ),
    bActive( rCpy.IsActive() )
{
}

SwFmtHeader::SwFmtHeader( sal_Bool bOn )
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

int  SwFmtHeader::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( GetRegisteredIn() == ((SwFmtHeader&)rAttr).GetRegisteredIn() &&
             bActive == ((SwFmtHeader&)rAttr).IsActive() );
}

SfxPoolItem*  SwFmtHeader::Clone( SfxItemPool* ) const
{
    return new SwFmtHeader( *this );
}

void SwFmtHeader::RegisterToFormat( SwFmt& rFmt )
{
    rFmt.Add(this);
}

//  class SwFmtFooter
//  Partially implemented inline in hxx

SwFmtFooter::SwFmtFooter( SwFrmFmt *pFooterFmt )
    : SfxPoolItem( RES_FOOTER ),
    SwClient( pFooterFmt ),
    bActive( pFooterFmt ? sal_True : sal_False )
{
}

SwFmtFooter::SwFmtFooter( const SwFmtFooter &rCpy )
    : SfxPoolItem( RES_FOOTER ),
    SwClient( (SwModify*)rCpy.GetRegisteredIn() ),
    bActive( rCpy.IsActive() )
{
}

SwFmtFooter::SwFmtFooter( sal_Bool bOn )
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

int  SwFmtFooter::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( GetRegisteredIn() == ((SwFmtFooter&)rAttr).GetRegisteredIn() &&
             bActive == ((SwFmtFooter&)rAttr).IsActive() );
}

SfxPoolItem*  SwFmtFooter::Clone( SfxItemPool* ) const
{
    return new SwFmtFooter( *this );
}

//  class SwFmtCntnt
//  Partially implemented inline in hxx

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

int  SwFmtCntnt::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    if( (sal_IntPtr)pStartNode ^ (sal_IntPtr)((SwFmtCntnt&)rAttr).pStartNode )
        return 0;
    if( pStartNode )
        return ( *pStartNode == *((SwFmtCntnt&)rAttr).GetCntntIdx() );
    return 1;
}

SfxPoolItem*  SwFmtCntnt::Clone( SfxItemPool* ) const
{
    return new SwFmtCntnt( *this );
}

//  class SwFmtPageDesc
//  Partially implemented inline in hxx

SwFmtPageDesc::SwFmtPageDesc( const SwFmtPageDesc &rCpy )
    : SfxPoolItem( RES_PAGEDESC ),
    SwClient( (SwPageDesc*)rCpy.GetPageDesc() ),
    nNumOffset( rCpy.nNumOffset ),
    nDescNameIdx( rCpy.nDescNameIdx ),
    pDefinedIn( 0 )
{
}

SwFmtPageDesc::SwFmtPageDesc( const SwPageDesc *pDesc )
    : SfxPoolItem( RES_PAGEDESC ),
    SwClient( (SwPageDesc*)pDesc ),
    nNumOffset( 0 ),
    nDescNameIdx( 0xFFFF ), // IDX_NO_VALUE
    pDefinedIn( 0 )
{
}

SwFmtPageDesc &SwFmtPageDesc::operator=(const SwFmtPageDesc &rCpy)
{
    if (rCpy.GetPageDesc())
        RegisterToPageDesc(*const_cast<SwPageDesc*>(rCpy.GetPageDesc()));
    nNumOffset = rCpy.nNumOffset;
    nDescNameIdx = rCpy.nDescNameIdx;
    pDefinedIn = 0;

    return *this;
}

 SwFmtPageDesc::~SwFmtPageDesc() {}

bool SwFmtPageDesc::KnowsPageDesc() const
{
    return (GetRegisteredIn() != 0);
}

int  SwFmtPageDesc::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return  ( pDefinedIn == ((SwFmtPageDesc&)rAttr).pDefinedIn ) &&
            ( nNumOffset == ((SwFmtPageDesc&)rAttr).nNumOffset ) &&
            ( GetPageDesc() == ((SwFmtPageDesc&)rAttr).GetPageDesc() );
}

SfxPoolItem*  SwFmtPageDesc::Clone( SfxItemPool* ) const
{
    return new SwFmtPageDesc( *this );
}

void SwFmtPageDesc::SwClientNotify( const SwModify&, const SfxHint& rHint )
{
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
                ((SwCntntNode*)pMod)->SetAttr( aDfltDesc );
            else if( pMod->ISA( SwFmt ))
                ((SwFmt*)pMod)->SetFmtAttr( aDfltDesc );
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
            rVal <<= (sal_Int16)GetNumOffset();
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
            OSL_ENSURE( !this, "unknown MemberId" );
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
            if(rVal >>= nOffset)
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
            OSL_ENSURE( !this, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
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

sal_Bool SwColumn::operator==( const SwColumn &rCmp ) const
{
    return (nWish    == rCmp.GetWishWidth() &&
            GetLeft()  == rCmp.GetLeft() &&
            GetRight() == rCmp.GetRight() &&
            GetUpper() == rCmp.GetUpper() &&
            GetLower() == rCmp.GetLower()) ? sal_True : sal_False;
}

SwFmtCol::SwFmtCol( const SwFmtCol& rCpy )
    : SfxPoolItem( RES_COL ),
    eLineStyle( rCpy.eLineStyle ),
    nLineWidth( rCpy.nLineWidth),
    aLineColor( rCpy.aLineColor),
    nLineHeight( rCpy.GetLineHeight() ),
    eAdj( rCpy.GetLineAdj() ),
    aColumns( (sal_Int8)rCpy.GetNumCols() ),
    nWidth( rCpy.GetWishWidth() ),
    aWidthAdjustValue( rCpy.aWidthAdjustValue ),
    bOrtho( rCpy.IsOrtho() )
{
    for ( sal_uInt16 i = 0; i < rCpy.GetNumCols(); ++i )
    {
        SwColumn *pCol = new SwColumn( rCpy.GetColumns()[i] );
        aColumns.push_back( pCol );
    }
}

SwFmtCol::~SwFmtCol() {}

SwFmtCol& SwFmtCol::operator=( const SwFmtCol& rCpy )
{
    eLineStyle  = rCpy.eLineStyle;
    nLineWidth  = rCpy.nLineWidth;
    aLineColor  = rCpy.aLineColor;
    nLineHeight = rCpy.GetLineHeight();
    eAdj        = rCpy.GetLineAdj();
    nWidth      = rCpy.GetWishWidth();
    aWidthAdjustValue = rCpy.aWidthAdjustValue;
    bOrtho      = rCpy.IsOrtho();

    if ( !aColumns.empty() )
        aColumns.clear();
    for ( sal_uInt16 i = 0; i < rCpy.GetNumCols(); ++i )
    {
        SwColumn *pCol = new SwColumn( rCpy.GetColumns()[i] );
        aColumns.push_back( pCol );
    }
    return *this;
}

SwFmtCol::SwFmtCol()
    : SfxPoolItem( RES_COL )
    , eLineStyle( table::BorderLineStyle::NONE)
    ,
    nLineWidth(0),
    nLineHeight( 100 ),
    eAdj( COLADJ_NONE ),
    nWidth( USHRT_MAX ),
    aWidthAdjustValue( 0 ),
    bOrtho( sal_True )
{
}

int SwFmtCol::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "no equal attributes" );
    const SwFmtCol &rCmp = (const SwFmtCol&)rAttr;
    if( !(eLineStyle        == rCmp.eLineStyle  &&
          nLineWidth        == rCmp.nLineWidth  &&
          aLineColor        == rCmp.aLineColor  &&
          nLineHeight        == rCmp.GetLineHeight() &&
          eAdj               == rCmp.GetLineAdj() &&
          nWidth             == rCmp.GetWishWidth() &&
          bOrtho             == rCmp.IsOrtho() &&
          aColumns.size() == rCmp.GetNumCols() &&
          aWidthAdjustValue == rCmp.GetAdjustValue()
         ) )
        return 0;

    for ( sal_uInt16 i = 0; i < aColumns.size(); ++i )
        if ( !(aColumns[i] == rCmp.GetColumns()[i]) )
            return 0;

    return 1;
}

SfxPoolItem*  SwFmtCol::Clone( SfxItemPool* ) const
{
    return new SwFmtCol( *this );
}

sal_uInt16 SwFmtCol::GetGutterWidth( sal_Bool bMin ) const
{
    sal_uInt16 nRet = 0;
    if ( aColumns.size() == 2 )
        nRet = aColumns[0].GetRight() + aColumns[1].GetLeft();
    else if ( aColumns.size() > 2 )
    {
        bool bSet = false;
        for ( sal_uInt16 i = 1; i < aColumns.size()-1; ++i )
        {
            const sal_uInt16 nTmp = aColumns[i].GetRight() + aColumns[i+1].GetLeft();
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
            {   bSet = true;
                nRet = nTmp;
            }
        }
    }
    return nRet;
}

void SwFmtCol::SetGutterWidth( sal_uInt16 nNew, sal_uInt16 nAct )
{
    if ( bOrtho )
        Calc( nNew, nAct );
    else
    {
        sal_uInt16 nHalf = nNew / 2;
        for ( sal_uInt16 i = 0; i < aColumns.size(); ++i )
        {   SwColumn *pCol = &aColumns[i];
            pCol->SetLeft ( nHalf );
            pCol->SetRight( nHalf );
            if ( i == 0 )
                pCol->SetLeft( 0 );
            else if ( i == (aColumns.size() - 1) )
                pCol->SetRight( 0 );
        }
    }
}

void SwFmtCol::Init( sal_uInt16 nNumCols, sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    // Deleting seems to be a bit radical on the first sight; but otherwise we
    // have to initialize all values of the remaining SwCloumns.
    if ( !aColumns.empty() )
        aColumns.clear();
    for ( sal_uInt16 i = 0; i < nNumCols; ++i )
    {   SwColumn *pCol = new SwColumn;
        aColumns.push_back( pCol );
    }
    bOrtho = sal_True;
    nWidth = USHRT_MAX;
    if( nNumCols )
        Calc( nGutterWidth, nAct );
}

void SwFmtCol::SetOrtho( sal_Bool bNew, sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    bOrtho = bNew;
    if ( bNew && !aColumns.empty() )
        Calc( nGutterWidth, nAct );
}

sal_uInt16 SwFmtCol::CalcColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const
{
    assert(nCol < aColumns.size());
    if ( nWidth != nAct )
    {
        long nW = aColumns[nCol].GetWishWidth();
        nW *= nAct;
        nW /= nWidth;
        return sal_uInt16(nW);
    }
    else
        return aColumns[nCol].GetWishWidth();
}

sal_uInt16 SwFmtCol::CalcPrtColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const
{
    assert(nCol < aColumns.size());
    sal_uInt16 nRet = CalcColWidth( nCol, nAct );
    const SwColumn *pCol = &aColumns[nCol];
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
    SwColumn *pCol = &aColumns.front();
    pCol->SetWishWidth( nLeftWidth );
    pCol->SetRight( nGutterHalf );
    pCol->SetLeft ( 0 );
    nAvail = nAvail - nLeftWidth;

    //Column 2 to n-1 is PrtWidth + gap width
    const sal_uInt16 nMidWidth = nPrtWidth + nGutterWidth;
    sal_uInt16 i;

    for ( i = 1; i < GetNumCols()-1; ++i )
    {
        pCol = &aColumns[i];
        pCol->SetWishWidth( nMidWidth );
        pCol->SetLeft ( nGutterHalf );
        pCol->SetRight( nGutterHalf );
        nAvail = nAvail - nMidWidth;
    }

    //The last column is equivalent to the first one - to compensate rounding
    //errors we add the remaining space of the other columns to the last one.
    pCol = &aColumns.back();
    pCol->SetWishWidth( nAvail );
    pCol->SetLeft ( nGutterHalf );
    pCol->SetRight( 0 );

    //Convert the current width to the requested width.
    for ( i = 0; i < aColumns.size(); ++i )
    {
        pCol = &aColumns[i];
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
        rVal.setValue(&xCols, ::getCppuType((uno::Reference< text::XTextColumns>*)0));
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
            aColumns.clear();
            //max count is 64k here - this is something the array can't do
            sal_uInt16 nCount = std::min( (sal_uInt16)aSetColumns.getLength(),
                                     (sal_uInt16) 0x3fff );
            sal_uInt16 nWidthSum = 0;
            // #101224# one column is no column
            //
            if(nCount > 1)
                for(sal_uInt16 i = 0; i < nCount; i++)
                {
                    SwColumn* pCol = new SwColumn;
                    pCol->SetWishWidth( static_cast<sal_uInt16>(pArray[i].Width) );
                    nWidthSum = static_cast<sal_uInt16>(nWidthSum + pArray[i].Width);
                    pCol->SetLeft ( static_cast<sal_uInt16>(MM100_TO_TWIP(pArray[i].LeftMargin)) );
                    pCol->SetRight( static_cast<sal_uInt16>(MM100_TO_TWIP(pArray[i].RightMargin)) );
                    aColumns.insert(aColumns.begin() + i, pCol);
                }
            bRet = true;
            nWidth = nWidthSum;
            bOrtho = sal_False;

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
                bOrtho = pSwColums->IsAutomaticWidth();
                nLineWidth = pSwColums->GetSepLineWidth();
                aLineColor.SetColor(pSwColums->GetSepLineColor());
                nLineHeight = pSwColums->GetSepLineHeightRelative();
                switch ( pSwColums->GetSepLineStyle() )
                {
                    default:
                    case 0: eLineStyle = table::BorderLineStyle::NONE; break;
                    case 1: eLineStyle = table::BorderLineStyle::SOLID; break;
                    case 2: eLineStyle = table::BorderLineStyle::DOTTED; break;
                    case 3: eLineStyle = table::BorderLineStyle::DASHED; break;
                }
                if(!pSwColums->GetSepLineIsOn())
                    eAdj = COLADJ_NONE;
                else switch(pSwColums->GetSepLineVertAlign())
                {
                    case 0: eAdj = COLADJ_TOP;  break;  //VerticalAlignment_TOP
                    case 1: eAdj = COLADJ_CENTER;break; //VerticalAlignment_MIDDLE
                    case 2: eAdj = COLADJ_BOTTOM;break; //VerticalAlignment_BOTTOM
                    default: OSL_ENSURE( !this, "unknown alignment" ); break;
                }
            }
        }
    }
    return bRet;
}


//  class SwFmtSurround
//  Partially implemented inline in hxx

SwFmtSurround::SwFmtSurround( SwSurround eFly ) :
    SfxEnumItem( RES_SURROUND, sal_uInt16( eFly ) )
{
    bAnchorOnly = bContour = bOutside = sal_False;
}

SwFmtSurround::SwFmtSurround( const SwFmtSurround &rCpy ) :
    SfxEnumItem( RES_SURROUND, rCpy.GetValue() )
{
    bAnchorOnly = rCpy.bAnchorOnly;
    bContour = rCpy.bContour;
    bOutside = rCpy.bOutside;
}

int  SwFmtSurround::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( GetValue() == ((SwFmtSurround&)rAttr).GetValue() &&
             bAnchorOnly== ((SwFmtSurround&)rAttr).bAnchorOnly &&
             bContour== ((SwFmtSurround&)rAttr).bContour &&
             bOutside== ((SwFmtSurround&)rAttr).bOutside );
}

SfxPoolItem*  SwFmtSurround::Clone( SfxItemPool* ) const
{
    return new SwFmtSurround( *this );
}

sal_uInt16  SwFmtSurround::GetValueCount() const
{
    return SURROUND_END - SURROUND_BEGIN;
}


bool SwFmtSurround::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_SURROUND_SURROUNDTYPE:
                rVal <<= (text::WrapTextMode)GetSurround();
        break;
        case MID_SURROUND_ANCHORONLY:
        {
            sal_Bool bTmp = IsAnchorOnly();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
                break;
        case MID_SURROUND_CONTOUR:
        {
            sal_Bool bTmp = IsContour();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
                break;
        case MID_SURROUND_CONTOUROUTSIDE:
        {
            sal_Bool bTmp = IsOutside();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
                break;
        default:
            OSL_ENSURE( !this, "unknown MemberId" );
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
            if( eVal >= 0 && eVal < (sal_Int16)SURROUND_END )
                SetValue( static_cast<sal_uInt16>(eVal) );
            else {
                //exception
                ;
            }
        }
        break;

        case MID_SURROUND_ANCHORONLY:
            SetAnchorOnly( *(sal_Bool*)rVal.getValue() );
            break;
        case MID_SURROUND_CONTOUR:
            SetContour( *(sal_Bool*)rVal.getValue() );
            break;
        case MID_SURROUND_CONTOUROUTSIDE:
            SetOutside( *(sal_Bool*)rVal.getValue() );
            break;
        default:
            OSL_ENSURE( !this, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

SvStream& SwFmtVertOrient::Store(SvStream &rStream, sal_uInt16 /*version*/) const
{
#if SAL_TYPES_SIZEOFLONG == 8
    rStream.WriteInt64(nYPos);
#else
    rStream << static_cast<sal_Int32>(nYPos);
#endif
    rStream << eOrient << eRelation;
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
    rStream >> n;
    yPos = n;
#endif
    rStream >> orient >> relation;

    return new SwFmtVertOrient(yPos, orient, relation);
}

//  class SwFmtVertOrient
//  Partially implemented inline in hxx

SwFmtVertOrient::SwFmtVertOrient( SwTwips nY, sal_Int16 eVert,
                                  sal_Int16 eRel )
    : SfxPoolItem( RES_VERT_ORIENT ),
    nYPos( nY ),
    eOrient( eVert ),
    eRelation( eRel )
{}

int  SwFmtVertOrient::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "not the same attributes" );
    return ( nYPos     == ((SwFmtVertOrient&)rAttr).nYPos &&
             eOrient   == ((SwFmtVertOrient&)rAttr).eOrient &&
             eRelation == ((SwFmtVertOrient&)rAttr).eRelation );
}

SfxPoolItem*  SwFmtVertOrient::Clone( SfxItemPool* ) const
{
    return new SwFmtVertOrient( nYPos, eOrient, eRelation );
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
            rVal <<= (sal_Int16)eOrient;
        }
        break;
        case MID_VERTORIENT_RELATION:
                rVal <<= (sal_Int16)eRelation;
        break;
        case MID_VERTORIENT_POSITION:
                rVal <<= (sal_Int32)TWIP_TO_MM100(GetPos());
                break;
        default:
            OSL_ENSURE( !this, "unknown MemberId" );
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
            eOrient = nVal;
        }
        break;
        case MID_VERTORIENT_RELATION:
        {
            eRelation = lcl_IntToRelation(rVal);
        }
        break;
        case MID_VERTORIENT_POSITION:
        {
            sal_Int32 nVal = 0;
            rVal >>= nVal;
            if(bConvert)
                nVal = MM100_TO_TWIP(nVal);
            SetPos( nVal );
        }
        break;
        default:
            OSL_ENSURE( !this, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}



//  class SwFmtHoriOrient
//  Partially implemented inline in hxx

SwFmtHoriOrient::SwFmtHoriOrient( SwTwips nX, sal_Int16 eHori,
                              sal_Int16 eRel, sal_Bool bPos )
    : SfxPoolItem( RES_HORI_ORIENT ),
    nXPos( nX ),
    eOrient( eHori ),
    eRelation( eRel ),
    bPosToggle( bPos )
{}

int  SwFmtHoriOrient::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( nXPos == ((SwFmtHoriOrient&)rAttr).nXPos &&
             eOrient == ((SwFmtHoriOrient&)rAttr).eOrient &&
             eRelation == ((SwFmtHoriOrient&)rAttr).eRelation &&
             bPosToggle == ((SwFmtHoriOrient&)rAttr).bPosToggle );
}

SfxPoolItem*  SwFmtHoriOrient::Clone( SfxItemPool* ) const
{
    return new SwFmtHoriOrient( nXPos, eOrient, eRelation, bPosToggle );
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
            rVal <<= (sal_Int16)eOrient;
        }
        break;
        case MID_HORIORIENT_RELATION:
            rVal <<= (sal_Int16)eRelation;
        break;
        case MID_HORIORIENT_POSITION:
                rVal <<= (sal_Int32)TWIP_TO_MM100(GetPos());
                break;
        case MID_HORIORIENT_PAGETOGGLE:
        {
            sal_Bool bTmp = IsPosToggle();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
                break;
        default:
            OSL_ENSURE( !this, "unknown MemberId" );
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
            eOrient = nVal;
        }
        break;
        case MID_HORIORIENT_RELATION:
        {
            eRelation = lcl_IntToRelation(rVal);
        }
        break;
        case MID_HORIORIENT_POSITION:
        {
            sal_Int32 nVal = 0;
            if(!(rVal >>= nVal))
                bRet = false;
            if(bConvert)
                nVal = MM100_TO_TWIP(nVal);
            SetPos( nVal );
        }
        break;
        case MID_HORIORIENT_PAGETOGGLE:
                SetPosToggle( *(sal_Bool*)rVal.getValue());
            break;
        default:
            OSL_ENSURE( !this, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}



//  class SwFmtAnchor
//  Partially implemented inline in hxx

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

int  SwFmtAnchor::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
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
sal_uInt32 SwFmtAnchor::GetOrder() const
{
    return mnOrder;
}

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
                    uno::Reference<container::XNamed> xNamed = SwXFrames::GetObject( *pFmt, FLYCNTTYPE_FRM );
                    uno::Reference<text::XTextFrame> xRet(xNamed, uno::UNO_QUERY);
                    rVal <<= xRet;
                }
            }
        }
        break;
        default:
            OSL_ENSURE( !this, "unknown MemberId" );
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
            OSL_ENSURE( !this, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

//  class SwFmtURL
//  Partially implemented inline in hxx


SwFmtURL::SwFmtURL() :
    SfxPoolItem( RES_URL ),
    pMap( 0 ),
    bIsServerMap( sal_False )
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

int SwFmtURL::operator==( const SfxPoolItem &rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "not the same attributes" );
    const SwFmtURL &rCmp = (SwFmtURL&)rAttr;
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

extern const SvEventDescription* sw_GetSupportedMacroItems();

bool SwFmtURL::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // here we convert always!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_URL_URL:
        {
            OUString sRet = GetURL();
            rVal <<= sRet;
        }
        break;
        case MID_URL_TARGET:
        {
            OUString sRet = GetTargetFrameName();
            rVal <<= sRet;
        }
        break;
        case MID_URL_HYPERLINKNAME:
            rVal <<= OUString( GetName() );
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
        {
            sal_Bool bTmp = IsServerMap();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
            break;
        default:
            OSL_ENSURE( !this, "unknown MemberId" );
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
            bIsServerMap = *(sal_Bool*)rVal.getValue();
            break;
        default:
            OSL_ENSURE( !this, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}


// class SwNoReadOnly

SfxPoolItem* SwFmtEditInReadonly::Clone( SfxItemPool* ) const
{
    return new SwFmtEditInReadonly( Which(), GetValue() );
}

// class SwFmtLayoutSplit

SfxPoolItem* SwFmtLayoutSplit::Clone( SfxItemPool* ) const
{
    return new SwFmtLayoutSplit( GetValue() );
}

// class SwFmtRowSplit

SfxPoolItem* SwFmtRowSplit::Clone( SfxItemPool* ) const
{
    return new SwFmtRowSplit( GetValue() );
}


// class SwFmtNoBalancedColumns

SfxPoolItem* SwFmtNoBalancedColumns::Clone( SfxItemPool* ) const
{
    return new SwFmtNoBalancedColumns( GetValue() );
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

int SwFmtFtnEndAtTxtEnd::operator==( const SfxPoolItem& rItem ) const
{
    const SwFmtFtnEndAtTxtEnd& rAttr = (SwFmtFtnEndAtTxtEnd&)rItem;
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
        {
            sal_Bool bVal = GetValue() >= FTNEND_ATTXTEND;
            rVal.setValue(&bVal, ::getBooleanCppuType());
        }
        break;
        case MID_RESTART_NUM :
        {
            sal_Bool bVal = GetValue() >= FTNEND_ATTXTEND_OWNNUMSEQ;
            rVal.setValue(&bVal, ::getBooleanCppuType());
        }
        break;
        case MID_NUM_START_AT: rVal <<= (sal_Int16) nOffset; break;
        case MID_OWN_NUM     :
        {
            sal_Bool bVal = GetValue() >= FTNEND_ATTXTEND_OWNNUMANDFMT;
            rVal.setValue(&bVal, ::getBooleanCppuType());
        }
        break;
        case MID_NUM_TYPE    : rVal <<= aFmt.GetNumberingType(); break;
        case MID_PREFIX      : rVal <<= OUString(sPrefix); break;
        case MID_SUFFIX      : rVal <<= OUString(sSuffix); break;
        default: return sal_False;
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
            sal_Bool bVal = *(sal_Bool*)rVal.getValue();
            if(!bVal && GetValue() >= FTNEND_ATTXTEND)
                SetValue(FTNEND_ATPGORDOCEND);
            else if(bVal && GetValue() < FTNEND_ATTXTEND)
                SetValue(FTNEND_ATTXTEND);
        }
        break;
        case MID_RESTART_NUM :
        {
            sal_Bool bVal = *(sal_Bool*)rVal.getValue();
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
            sal_Bool bVal = *(sal_Bool*)rVal.getValue();
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


int SwFmtChain::operator==( const SfxPoolItem &rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );

    return GetPrev() == ((SwFmtChain&)rAttr).GetPrev() &&
           GetNext() == ((SwFmtChain&)rAttr).GetNext();
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
        ((SwModify*)aPrev.GetRegisteredIn())->Remove( &aPrev );
}

void SwFmtChain::SetNext( SwFlyFrmFmt *pFmt )
{
    if ( pFmt )
        pFmt->Add( &aNext );
    else if ( aNext.GetRegisteredIn() )
        ((SwModify*)aNext.GetRegisteredIn())->Remove( &aNext );
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
            OSL_ENSURE( !this, "unknown MemberId" );
            bRet = false;
    }
    rVal <<= aRet;
    return bRet;
}




//class SwFmtLineNumber

SwFmtLineNumber::SwFmtLineNumber() :
    SfxPoolItem( RES_LINENUMBER )
{
    nStartValue = 0;
    bCountLines = sal_True;
}

SwFmtLineNumber::~SwFmtLineNumber()
{
}

int SwFmtLineNumber::operator==( const SfxPoolItem &rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );

    return nStartValue  == ((SwFmtLineNumber&)rAttr).GetStartValue() &&
           bCountLines  == ((SwFmtLineNumber&)rAttr).IsCount();
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
        {
            sal_Bool bTmp = IsCount();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
        break;
        case MID_LINENUMBER_STARTVALUE:
            rVal <<= (sal_Int32)GetStartValue();
            break;
        default:
            OSL_ENSURE( !this, "unknown MemberId" );
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
            SetCountLines( *(sal_Bool*)rVal.getValue() );
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
            OSL_ENSURE( !this, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

/*************************************************************************
 *    class SwTextGridItem
 *************************************************************************/

SwTextGridItem::SwTextGridItem()
    : SfxPoolItem( RES_TEXTGRID ), aColor( COL_LIGHTGRAY ), nLines( 20 ),
      nBaseHeight( 400 ), nRubyHeight( 200 ), eGridType( GRID_NONE ),
      bRubyTextBelow( 0 ), bPrintGrid( 1 ), bDisplayGrid( 1 ),
      nBaseWidth(400), bSnapToChars( 1 ), bSquaredMode(1)
{
}

SwTextGridItem::~SwTextGridItem()
{
}

int SwTextGridItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return eGridType == ((SwTextGridItem&)rAttr).GetGridType() &&
           nLines == ((SwTextGridItem&)rAttr).GetLines() &&
           nBaseHeight == ((SwTextGridItem&)rAttr).GetBaseHeight() &&
           nRubyHeight == ((SwTextGridItem&)rAttr).GetRubyHeight() &&
           bRubyTextBelow == ((SwTextGridItem&)rAttr).GetRubyTextBelow() &&
           bDisplayGrid == ((SwTextGridItem&)rAttr).GetDisplayGrid() &&
           bPrintGrid == ((SwTextGridItem&)rAttr).GetPrintGrid() &&
           aColor == ((SwTextGridItem&)rAttr).GetColor() &&
           nBaseWidth == ((SwTextGridItem&)rAttr).GetBaseWidth() &&
           bSnapToChars == ((SwTextGridItem&)rAttr).GetSnapToChars() &&
           bSquaredMode == ((SwTextGridItem&)rAttr).GetSquaredMode();
}

SfxPoolItem* SwTextGridItem::Clone( SfxItemPool* ) const
{
    return new SwTextGridItem( *this );
}

SwTextGridItem& SwTextGridItem::operator=( const SwTextGridItem& rCpy )
{
    aColor = rCpy.GetColor();
    nLines = rCpy.GetLines();
    nBaseHeight = rCpy.GetBaseHeight();
    nRubyHeight = rCpy.GetRubyHeight();
    eGridType = rCpy.GetGridType();
    bRubyTextBelow = rCpy.GetRubyTextBelow();
    bPrintGrid = rCpy.GetPrintGrid();
    bDisplayGrid = rCpy.GetDisplayGrid();
    nBaseWidth = rCpy.GetBaseWidth();
    bSnapToChars = rCpy.GetSnapToChars();
    bSquaredMode = rCpy.GetSquaredMode();

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
            rVal.setValue( &bRubyTextBelow, ::getBooleanCppuType() );
            break;
        case MID_GRID_PRINT:
            rVal.setValue( &bPrintGrid, ::getBooleanCppuType() );
            break;
        case MID_GRID_DISPLAY:
            rVal.setValue( &bDisplayGrid, ::getBooleanCppuType() );
            break;
        case MID_GRID_BASEHEIGHT:
            OSL_ENSURE( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            rVal <<= (sal_Int32) TWIP_TO_MM100_UNSIGNED(nBaseHeight);
            break;
        case MID_GRID_BASEWIDTH:
            OSL_ENSURE( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            rVal <<= (sal_Int32) TWIP_TO_MM100_UNSIGNED(nBaseWidth);
            break;
        case MID_GRID_RUBYHEIGHT:
            OSL_ENSURE( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            rVal <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(nRubyHeight);
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
            rVal.setValue( &bSnapToChars, ::getBooleanCppuType() );
            break;
        case MID_GRID_STANDARD_MODE:
            {
                sal_Bool bStandardMode = !bSquaredMode;
                rVal.setValue( &bStandardMode, ::getBooleanCppuType() );
            }
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
            SetRubyTextBelow( *(sal_Bool*)rVal.getValue() );
            break;
        case MID_GRID_PRINT:
            SetPrintGrid( *(sal_Bool*)rVal.getValue() );
            break;
        case MID_GRID_DISPLAY:
            SetDisplayGrid( *(sal_Bool*)rVal.getValue() );
            break;
        case MID_GRID_BASEHEIGHT:
        case MID_GRID_BASEWIDTH:
        case MID_GRID_RUBYHEIGHT:
        {
            OSL_ENSURE( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            sal_Int32 nTmp = 0;
            bRet = (rVal >>= nTmp);
            nTmp = MM100_TO_TWIP( nTmp );
            if( bRet && (nTmp >= 0) && ( nTmp <= USHRT_MAX) )
                if( (nMemberId & ~CONVERT_TWIPS) == MID_GRID_BASEHEIGHT )
                    SetBaseHeight( (sal_uInt16)nTmp );
                else if( (nMemberId & ~CONVERT_TWIPS) == MID_GRID_BASEWIDTH )
                    SetBaseWidth( (sal_uInt16)nTmp );
                else
                    SetRubyHeight( (sal_uInt16)nTmp );
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
            SetSnapToChars( *(sal_Bool*)rVal.getValue() );
            break;
        case MID_GRID_STANDARD_MODE:
        {
            sal_Bool bStandard = *(sal_Bool*)rVal.getValue();
               SetSquaredMode( !bStandard );
            break;
        }
        default:
            OSL_FAIL("Unknown SwTextGridItem member");
            bRet = false;
    }

    return bRet;
}

void SwTextGridItem::SwitchPaperMode(sal_Bool bNew)
{
    if( bNew == bSquaredMode )
    {
        //same paper mode, not switch
        return;
    }

    // use default value when grid is disable
    if( eGridType == GRID_NONE )
    {
        bSquaredMode = bNew;
        Init();
        return;
    }

    if( bSquaredMode )
    {
        //switch from "squared mode" to "standard mode"
        nBaseWidth = nBaseHeight;
        nBaseHeight = nBaseHeight + nRubyHeight;
        nRubyHeight = 0;
    }
    else
    {
        //switch from "standard mode" to "squared mode"
        nRubyHeight = nBaseHeight/3;
        nBaseHeight = nBaseHeight - nRubyHeight;
        nBaseWidth = nBaseHeight;
    }
    bSquaredMode = !bSquaredMode;
}

void SwTextGridItem::Init()
{
    if( bSquaredMode )
    {
        nLines = 20;
        nBaseHeight = 400;
        nRubyHeight = 200;
        eGridType = GRID_NONE;
        bRubyTextBelow = 0;
        bPrintGrid = 1;
        bDisplayGrid = 1;
        bSnapToChars = 1;
        nBaseWidth = 400;
    }
    else
    {
        nLines = 44;
        nBaseHeight = 312;
        nRubyHeight = 0;
        eGridType = GRID_NONE;
        bRubyTextBelow = 0;
        bPrintGrid = 1;
        bDisplayGrid = 1;
        nBaseWidth = 210;
        bSnapToChars = 1;

        //default grid type is line only in CJK env
        //disable this function due to type area change
        //if grid type change.
        //if(SvtCJKOptions().IsAsianTypographyEnabled())
        //{
        //  bDisplayGrid = 0;
        //  eGridType = GRID_LINES_ONLY;
        //}
    }
}
// class SwHeaderAndFooterEatSpacingItem

SfxPoolItem* SwHeaderAndFooterEatSpacingItem::Clone( SfxItemPool* ) const
{
    return new SwHeaderAndFooterEatSpacingItem( Which(), GetValue() );
}


//  class SwFrmFmt
//  Partially implemented inline in hxx

TYPEINIT1( SwFrmFmt, SwFmt );
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( SwFrmFmt )

void SwFrmFmt::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    SwFmtHeader *pH = 0;
    SwFmtFooter *pF = 0;

    sal_uInt16 nWhich = pNew ? pNew->Which() : 0;

    if( RES_ATTRSET_CHG == nWhich )
    {
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
            RES_HEADER, sal_False, (const SfxPoolItem**)&pH );
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
            RES_FOOTER, sal_False, (const SfxPoolItem**)&pF );
    }
    else if( RES_HEADER == nWhich )
        pH = (SwFmtHeader*)pNew;
    else if( RES_FOOTER == nWhich )
        pF = (SwFmtFooter*)pNew;

    if( pH && pH->IsActive() && !pH->GetHeaderFmt() )
    {   //If he doesn't have one, I'll add one
        SwFrmFmt *pFmt = GetDoc()->MakeLayoutFmt( RND_STD_HEADER, 0 );
        pH->RegisterToFormat( *pFmt );
    }

    if( pF && pF->IsActive() && !pF->GetFooterFmt() )
    {   //If he doesn't have one, I'll add one
        SwFrmFmt *pFmt = GetDoc()->MakeLayoutFmt( RND_STD_FOOTER, 0 );
        pF->RegisterToFormat( *pFmt );
    }

    // MIB 24.3.98: We always have to call Modify of the baseclass, for example
    // because of RESET_FMTWRITTEN.
//  if ( GetDepends() )
        SwFmt::Modify( pOld, pNew );

    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached uno object
        SetXObject(uno::Reference<uno::XInterface>(0));
    }
}

void SwFrmFmt::RegisterToFormat( SwFmt& rFmt )
{
    rFmt.Add( this );
}

//Deletes all Frms which are registered in aDepend.

void SwFrmFmt::DelFrms()
{
    SwIterator<SwFrm,SwFmt> aIter( *this );
    SwFrm * pLast = aIter.First();
    if( pLast )
        do {
                pLast->Cut();
                delete pLast;
        } while( 0 != ( pLast = aIter.Next() ));
}

void SwFrmFmt::MakeFrms()
{
    OSL_ENSURE( !this, "Sorry not implemented." );
}



SwRect SwFrmFmt::FindLayoutRect( const sal_Bool bPrtArea, const Point* pPoint,
                                 const sal_Bool bCalcFrm ) const
{
    SwRect aRet;
    SwFrm *pFrm = 0;
    if( ISA( SwSectionFmt ) )
    {
        // get the Frame using Node2Layout
        SwSectionNode* pSectNd = ((SwSectionFmt*)this)->GetSectionNode();
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
        sal_uInt16 nFrmType = RES_FLYFRMFMT == Which() ? FRM_FLY : USHRT_MAX;
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
    return SwIterator<SwContact,SwFmt>::FirstElement( *this );
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
        SwFlyFrm* pFly = (SwFlyFrm*)::GetFrmOfModify( 0, *this, FRM_FLY,
                                                    &aNullPt, 0, sal_False );
        return pFly ? pFly->GetVirtDrawObj() : 0;
    }
    return FindSdrObject();
}


sal_Bool SwFrmFmt::IsLowerOf( const SwFrmFmt& rFmt ) const
{
    //Also linking from inside to outside or from outside to inside is not
    //allowed.
    SwFlyFrm *pSFly = SwIterator<SwFlyFrm,SwFmt>::FirstElement(*this);
    if( pSFly )
    {
        SwFlyFrm *pAskFly = SwIterator<SwFlyFrm,SwFmt>::FirstElement(rFmt);
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
            sal_uInt16 n;
            for( n = 0; n < rFmts.size(); ++n )
            {
                const SwFrmFmt* pFmt = rFmts[ n ];
                const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                if( pIdx && pFlyNd == &pIdx->GetNode() )
                {
                    if( pFmt == this )
                        return sal_True;

                    pAnchor = &pFmt->GetAnchor();
                    if ((FLY_AT_PAGE == pAnchor->GetAnchorId()) ||
                        !pAnchor->GetCntntAnchor() )
                    {
                        return sal_False;
                    }

                    pFlyNd = pAnchor->GetCntntAnchor()->nNode.GetNode().
                                FindFlyStartNode();
                    break;
                }
            }
            if( n >= rFmts.size() )
            {
                OSL_ENSURE( !this, "Fly section but no format found" );
                return sal_False;
            }
        }
    }
    return sal_False;
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
String SwFrmFmt::GetDescription() const
{
    return SW_RES(STR_FRAME);
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
        do {
                delete pLast;
        } while( 0 != ( pLast = aIter.Next() ));

    SwIterator<SwFlyDrawContact,SwFmt> a2ndIter( *this );
    SwFlyDrawContact* pC = a2ndIter.First();
    if( pC )
        do {
                delete pC;

        } while( 0 != ( pC = a2ndIter.Next() ));
}

//Creates the Frms if the format describes a paragraph-bound frame.
//MA: 1994-02-14, creates the Frms also for frames anchored at page.

void SwFlyFrmFmt::MakeFrms()
{
    // is there a layout?
    if( !GetDoc()->GetCurrentViewShell() )
        return; //swmod 071108//swmod 071225

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
                if( SwIterator<SwFrm,SwCntntNode>::FirstElement( *pCNd ) )
                {
                    pModify = pCNd;
                }
            }
            // #i105535#
            if ( pModify == 0 )
            {
                const SwNodeIndex &rIdx = aAnchorAttr.GetCntntAnchor()->nNode;
                SwFrmFmts& rFmts = *GetDoc()->GetSpzFrmFmts();
                for( sal_uInt16 i = 0; i < rFmts.size(); ++i )
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
            SwPageFrm *pPage = (SwPageFrm*)GetDoc()->GetCurrentLayout()->Lower();   //swmod 080218
            if( !nPgNum && aAnchorAttr.GetCntntAnchor() )
            {
                SwCntntNode *pCNd =
                    aAnchorAttr.GetCntntAnchor()->nNode.GetNode().GetCntntNode();
                SwIterator<SwFrm,SwCntntNode> aIter( *pCNd );
                for (SwFrm* pFrm = aIter.First();
                     pFrm;
                     /* unreachable, note unconditional break below
                        pFrm = aIter.Next()
                     */ )
                {
                        pPage = pFrm->FindPageFrm();
                        if( pPage )
                        {
                            nPgNum = pPage->GetPhyPageNum();
                            // OD 24.07.2003 #111032# - update anchor attribute
                            aAnchorAttr.SetPageNum( nPgNum );
                            aAnchorAttr.SetAnchor( 0 );
                            SetFmtAttr( aAnchorAttr );
                        }
                        break;
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
                pPage = (SwPageFrm*)pPage->GetNext();
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
                            !((SwCntntFrm*)pFrm)->IsFollow();

            if ( FLY_AT_FLY == aAnchorAttr.GetAnchorId() && !pFrm->IsFlyFrm() )
            {
                // #i105535#
                // fallback to anchor type at-paragraph, if no fly frame is found.
//                pFrm = pFrm->FindFlyFrm();
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
                for( sal_uInt16 i = 0; i < rObjs.Count(); ++i)
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
                SwFlyFrm *pFly = 0;
                switch( aAnchorAttr.GetAnchorId() )
                {
                case FLY_AT_FLY:
                    pFly = new SwFlyLayFrm( this, pFrm, pFrm );
                    break;

                case FLY_AT_PARA:
                case FLY_AT_CHAR:
                    pFly = new SwFlyAtCntFrm( this, pFrm, pFrm );
                    break;

                case FLY_AS_CHAR:
                    pFly = new SwFlyInCntFrm( this, pFrm, pFrm );
                    break;
                default:
                    OSL_ENSURE( !this, "Neuer Ankertyp" );
                    break;
                }
                pFrm->AppendFly( pFly );
                SwPageFrm *pPage = pFly->FindPageFrm();
                if( pPage )
                    ::RegistFlys( pPage, pFly );
            }
        }
    }
}

SwFlyFrm* SwFlyFrmFmt::GetFrm( const Point* pPoint, const sal_Bool bCalcFrm ) const
{
    return (SwFlyFrm*)::GetFrmOfModify( 0, *(SwModify*)this, FRM_FLY,
                                            pPoint, 0, bCalcFrm );
}

SwAnchoredObject* SwFlyFrmFmt::GetAnchoredObj( const Point* pPoint, const sal_Bool bCalcFrm ) const
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
    switch( rInfo.Which() )
    {
    case RES_CONTENT_VISIBLE:
        {
            ((SwPtrMsgPoolItem&)rInfo).pObject = SwIterator<SwFrm,SwFmt>::FirstElement( *this );
        }
        return false;

    default:
        return SwFrmFmt::GetInfo( rInfo );
    }
}

// #i73249#
void SwFlyFrmFmt::SetObjTitle( const String& rTitle, bool bBroadcast )
{
    SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject,
            "<SwNoTxtNode::SetObjTitle(..)> - missing <SdrObject> instance" );
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

const String SwFlyFrmFmt::GetObjTitle() const
{
    const SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject,
            "<SwFlyFrmFmt::GetObjTitle(..)> - missing <SdrObject> instance" );
    if ( !pMasterObject )
    {
        return aEmptyStr;
    }

    return pMasterObject->GetTitle();
}

void SwFlyFrmFmt::SetObjDescription( const String& rDescription, bool bBroadcast )
{
    SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject,
            "<SwFlyFrmFmt::SetDescription(..)> - missing <SdrObject> instance" );
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

const String SwFlyFrmFmt::GetObjDescription() const
{
    const SdrObject* pMasterObject = FindSdrObject();
    OSL_ENSURE( pMasterObject,
            "<SwNoTxtNode::GetDescription(..)> - missing <SdrObject> instance" );
    if ( !pMasterObject )
    {
        return aEmptyStr;
    }

    return pMasterObject->GetDescription();
}

/** SwFlyFrmFmt::IsBackgroundTransparent - for #99657#

    OD 22.08.2002 - overloading virtual method and its default implementation,
    because format of fly frame provides transparent backgrounds.
    Method determines, if background of fly frame is transparent.

    @author OD

    @return true, if background color is transparent, but not "no fill"
    or the transparency of a existing background graphic is set.
*/
sal_Bool SwFlyFrmFmt::IsBackgroundTransparent() const
{
    sal_Bool bReturn = sal_False;

    /// NOTE: If background color is "no fill"/"auto fill" (COL_TRANSPARENT)
    ///     and there is no background graphic, it "inherites" the background
    ///     from its anchor.
    if ( (GetBackground().GetColor().GetTransparency() != 0) &&
         (GetBackground().GetColor() != COL_TRANSPARENT)
       )
    {
        bReturn = sal_True;
    }
    else
    {
        const GraphicObject *pTmpGrf =
                static_cast<const GraphicObject*>(GetBackground().GetGraphicObject());
        if ( (pTmpGrf) &&
             (pTmpGrf->GetAttr().GetTransparency() != 0)
           )
        {
            bReturn = sal_True;
        }
    }

    return bReturn;
}

/** SwFlyFrmFmt::IsBackgroundBrushInherited - for #103898#

    OD 08.10.2002 - method to determine, if the brush for drawing the
    background is "inherited" from its parent/grandparent.
    This is the case, if no background graphic is set and the background
    color is "no fill"/"auto fill"
    NOTE: condition is "copied" from method <SwFrm::GetBackgroundBrush(..).

    @author OD

    @return true, if background brush is "inherited" from parent/grandparent
*/
sal_Bool SwFlyFrmFmt::IsBackgroundBrushInherited() const
{
    sal_Bool bReturn = sal_False;

    if ( (GetBackground().GetColor() == COL_TRANSPARENT) &&
         !(GetBackground().GetGraphicObject()) )
    {
        bReturn = sal_True;
    }

    return bReturn;
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
                            delete pFrm;
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
    SwDrawContact *pContact = (SwDrawContact*)FindContactObj();
    if ( pContact )
         pContact->ConnectToLayout();
}

void SwDrawFrmFmt::DelFrms()
{
    SwDrawContact *pContact = (SwDrawContact *)FindContactObj();
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

String SwDrawFrmFmt::GetDescription() const
{
    String aResult;
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
        pFly = SwIterator<SwFlyFrm,SwFmt>::FirstElement( *this );
        if( !pFly )
            return 0;
    }

    //Original size for OLE and graphic is TwipSize, otherwise the size of
    //FrmFmt of the Fly.
    const SwFrm *pRef;
    SwNoTxtNode *pNd = 0;
    Size aOrigSz;
    if( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
    {
        pRef = pFly->Lower();
        pNd = ((SwCntntFrm*)pRef)->GetNode()->GetNoTxtNode();
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
        return ((ImageMap*)rURL.GetMap())->GetHitIMapObject( aOrigSz,
                                                aActSz, aPos, nFlags );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
