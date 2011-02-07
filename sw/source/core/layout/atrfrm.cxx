/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/HorizontalAdjust.hpp>
#include <com/sun/star/text/DocumentStatistic.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/HoriOrientationFormat.hpp>
#include <com/sun/star/text/NotePrintMode.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientationFormat.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/InvalidTextContentException.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/text/TextGridMode.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <svtools/unoimap.hxx>
#include <svtools/unoevent.hxx>
#include <basic/sbxvar.hxx>
#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
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
#include <fmtanchr.hxx>
#include <fmtlsplt.hxx>
#include <fmtrowsplt.hxx>
#include <fmtftntx.hxx>
#include <fmteiro.hxx>
#include <fmturl.hxx>
#include <fmtcnct.hxx>
#include <node.hxx>
#include <section.hxx>
#include <fmtline.hxx>
#include <tgrditem.hxx>
#include <hfspacingitem.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <crsrsh.hxx>
#include <pam.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <flyfrm.hxx>
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
/// OD 22.08.2002 #99657#
///     include definition of class SvxBrushItem and GraphicObject
///     in order to determine, if background is transparent.
#include <editeng/brshitem.hxx>
#include <svtools/grfmgr.hxx>

#include <cmdid.h>
#include <unomid.h>
#include <comcore.hrc>
#include <svx/svdundo.hxx> // #111827#
// OD 2004-05-24 #i28701#
#include <sortedobjs.hxx>
// --> OD 2006-03-06 #125892#
#include <HandleAnchorNodeChg.hxx>
// <--
#include <svl/cjkoptions.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;

SV_IMPL_PTRARR(SwColumns,SwColumn*)

TYPEINIT1(SwFmtVertOrient, SfxPoolItem);
TYPEINIT1(SwFmtHoriOrient, SfxPoolItem);
TYPEINIT2(SwFmtHeader,  SfxPoolItem, SwClient );
TYPEINIT2(SwFmtFooter,  SfxPoolItem, SwClient );
TYPEINIT2(SwFmtPageDesc,  SfxPoolItem, SwClient );
TYPEINIT1_AUTOFACTORY(SwFmtLineNumber, SfxPoolItem);

/* -----------------19.05.98 09:26-------------------
 *  Umwandlung fuer QueryValue
 * --------------------------------------------------*/
sal_Int16 lcl_RelToINT(sal_Int16 eRelation)
{
    sal_Int16 nRet = text::RelOrientation::FRAME;
    switch(eRelation)
    {
    case  text::RelOrientation::PRINT_AREA:           nRet = text::RelOrientation::PRINT_AREA; break;
    case  text::RelOrientation::CHAR:         nRet = text::RelOrientation::CHAR; break;
    case  text::RelOrientation::PAGE_LEFT:        nRet = text::RelOrientation::PAGE_LEFT; break;
    case  text::RelOrientation::PAGE_RIGHT:       nRet = text::RelOrientation::PAGE_RIGHT; break;
    case  text::RelOrientation::FRAME_LEFT:       nRet = text::RelOrientation::FRAME_LEFT; break;
    case  text::RelOrientation::FRAME_RIGHT:  nRet = text::RelOrientation::FRAME_RIGHT; break;
    case  text::RelOrientation::PAGE_FRAME:       nRet = text::RelOrientation::PAGE_FRAME; break;
    case  text::RelOrientation::PAGE_PRINT_AREA:  nRet = text::RelOrientation::PAGE_PRINT_AREA; break;
    // OD 13.11.2003 #i22341#
    case  text::RelOrientation::TEXT_LINE:    nRet = text::RelOrientation::TEXT_LINE; break;
    default: break;
    }
    return nRet;
}

sal_Int16 lcl_IntToRelation(const uno::Any& rVal)
{
    sal_Int16 eRet = text::RelOrientation::FRAME;
    sal_Int16 nVal = 0;
    rVal >>= nVal;
    switch(nVal)
    {
        case  text::RelOrientation::PRINT_AREA:         eRet =   text::RelOrientation::PRINT_AREA           ; break;
        case  text::RelOrientation::CHAR:               eRet =   text::RelOrientation::CHAR          ; break;
        case  text::RelOrientation::PAGE_LEFT:          eRet =   text::RelOrientation::PAGE_LEFT       ; break;
        case  text::RelOrientation::PAGE_RIGHT:         eRet =   text::RelOrientation::PAGE_RIGHT      ; break;
        case  text::RelOrientation::FRAME_LEFT:         eRet =   text::RelOrientation::FRAME_LEFT      ; break;
        case  text::RelOrientation::FRAME_RIGHT:        eRet =   text::RelOrientation::FRAME_RIGHT     ; break;
        case  text::RelOrientation::PAGE_FRAME:         eRet =   text::RelOrientation::PAGE_FRAME      ; break;
        case  text::RelOrientation::PAGE_PRINT_AREA:    eRet =   text::RelOrientation::PAGE_PRINT_AREA    ; break;
        // OD 13.11.2003 #i22341#
        case  text::RelOrientation::TEXT_LINE: eRet = text::RelOrientation::TEXT_LINE; break;
    }
    return eRet;
}

void DelHFFormat( SwClient *pToRemove, SwFrmFmt *pFmt )
{
    //Wenn der Client der letzte ist der das Format benutzt, so muss dieses
    //vernichtet werden. Zuvor muss jedoch ggf. die Inhaltssection vernichtet
    //werden.
    SwDoc* pDoc = pFmt->GetDoc();
    pFmt->Remove( pToRemove );
    if( pDoc->IsInDtor() )
    {
        delete pFmt;
        return;
    }

    //Nur noch Frms angemeldet?
    sal_Bool bDel = sal_True;
    {
        // Klammer, weil im DTOR SwClientIter das Flag bTreeChg zurueck
        // gesetzt wird. Unguenstig, wenn das Format vorher zerstoert wird.
        SwClientIter aIter( *pFmt );
        SwClient *pLast = aIter.GoStart();
        if( pLast )
            do {
                bDel = pLast->IsA( TYPE(SwFrm) )
                    || SwXHeadFootText::IsXHeadFootText(pLast);
            } while( bDel && 0 != ( pLast = aIter++ ));
    }

    if ( bDel )
    {
        //Wenn in einem der Nodes noch ein Crsr angemeldet ist, muss das
        //ParkCrsr einer (beliebigen) Shell gerufen werden.
        SwFmtCntnt& rCnt = (SwFmtCntnt&)pFmt->GetCntnt();
        if ( rCnt.GetCntntIdx() )
        {
            SwNode *pNode = 0;
            {
                // --> OD 2008-10-07 #i92993#
                // Begin with start node of page header/footer to assure that
                // complete content is checked for cursors and the complete content
                // is deleted on below made method call <pDoc->DeleteSection(pNode)>
//                SwNodeIndex aIdx( *rCnt.GetCntntIdx(), 1 );
                SwNodeIndex aIdx( *rCnt.GetCntntIdx(), 0 );
                // <--
                //Wenn in einem der Nodes noch ein Crsr angemeldet ist, muss das
                //ParkCrsr einer (beliebigen) Shell gerufen werden.
                pNode = & aIdx.GetNode();
                sal_uInt32 nEnd = pNode->EndOfSectionIndex();
                while ( aIdx < nEnd )
                {
                    if ( pNode->IsCntntNode() &&
                         ((SwCntntNode*)pNode)->GetDepends() )
                    {
                        SwClientIter aIter( *(SwCntntNode*)pNode );
                        do
                        {
                            if( aIter()->ISA( SwCrsrShell ) )
                            {
                                ((SwCrsrShell*)aIter())->ParkCrsr( aIdx );
                                aIdx = nEnd-1;
                                break;
                            }
                        } while ( aIter++ );
                    }
                    aIdx++;
                    pNode = & aIdx.GetNode();
                }
            }
            rCnt.SetNewCntntIdx( (const SwNodeIndex*)0 );

            // beim Loeschen von Header/Footer-Formaten IMMER das Undo
            // abschalten! (Bug 31069)
            ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

            ASSERT( pNode, "Ein grosses Problem." );
            pDoc->DeleteSection( pNode );
        }
        delete pFmt;
    }
}

//  class SwFmtFrmSize
//  Implementierung teilweise inline im hxx

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
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
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


/* -----------------24.04.98 11:36-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwFmtFrmSize::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // hier wird immer konvertiert!
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
    return sal_True;
}

/* -----------------24.04.98 11:36-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwFmtFrmSize::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_FRMSIZE_SIZE:
        {
            awt::Size aVal;
            if(!(rVal >>= aVal))
                bRet = sal_False;
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
                    bRet = sal_False;
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
                bRet = sal_False;
        }
        break;
        case MID_FRMSIZE_REL_WIDTH:
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0 && nSet <= 0xfe)
                SetWidthPercent((sal_uInt8)nSet);
            else
                bRet = sal_False;
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
                bRet = sal_False;
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
                bRet = sal_False;
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
                bRet = sal_False;
        }
        break;
        default:
            bRet = sal_False;
    }
    return bRet;
}

//  class SwFmtFillOrder
//  Implementierung teilweise inline im hxx

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
//  Implementierung teilweise inline im hxx

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
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( pRegisteredIn == ((SwFmtHeader&)rAttr).GetRegisteredIn() &&
             bActive == ((SwFmtHeader&)rAttr).IsActive() );
}

SfxPoolItem*  SwFmtHeader::Clone( SfxItemPool* ) const
{
    return new SwFmtHeader( *this );
}

//  class SwFmtFooter
//  Implementierung teilweise inline im hxx

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

int  SwFmtFooter::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( pRegisteredIn == ((SwFmtFooter&)rAttr).GetRegisteredIn() &&
             bActive == ((SwFmtFooter&)rAttr).IsActive() );
}

SfxPoolItem*  SwFmtFooter::Clone( SfxItemPool* ) const
{
    return new SwFmtFooter( *this );
}

//  class SwFmtCntnt
//  Implementierung teilweise inline im hxx

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
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    if( (long)pStartNode ^ (long)((SwFmtCntnt&)rAttr).pStartNode )
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
//  Implementierung teilweise inline im hxx

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

 SwFmtPageDesc::~SwFmtPageDesc() {}

int  SwFmtPageDesc::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return  ( pDefinedIn == ((SwFmtPageDesc&)rAttr).pDefinedIn ) &&
            ( nNumOffset == ((SwFmtPageDesc&)rAttr).nNumOffset ) &&
            ( GetPageDesc() == ((SwFmtPageDesc&)rAttr).GetPageDesc() );
}

SfxPoolItem*  SwFmtPageDesc::Clone( SfxItemPool* ) const
{
    return new SwFmtPageDesc( *this );
}

void SwFmtPageDesc::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    if( !pDefinedIn )
        return;

    const sal_uInt16 nWhichId = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhichId )
    {
        case RES_OBJECTDYING:
                //Der Pagedesc, bei dem ich angemeldet bin stirbt, ich trage
                //mich also bei meinem Format aus.
                //Dabei werden ich Deletet!!!
            if( IS_TYPE( SwFmt, pDefinedIn ))
#ifdef DBG_UTIL
            {
                sal_Bool bDel = ((SwFmt*)pDefinedIn)->ResetFmtAttr( RES_PAGEDESC );
                ASSERT( bDel, ";-) FmtPageDesc nicht zerstoert." );
            }
#else
                ((SwFmt*)pDefinedIn)->ResetFmtAttr( RES_PAGEDESC );
#endif
            else if( IS_TYPE( SwCntntNode, pDefinedIn ))
#ifdef DBG_UTIL
            {
                sal_Bool bDel = ((SwCntntNode*)pDefinedIn)->ResetAttr( RES_PAGEDESC );
                ASSERT( bDel, ";-) FmtPageDesc nicht zerstoert." );
            }
#else
                ((SwCntntNode*)pDefinedIn)->ResetAttr( RES_PAGEDESC );
#endif
            break;

        default:
            /* do nothing */;
    }
}

sal_Bool SwFmtPageDesc::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool    bRet = sal_True;
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
                    String aString;
                    SwStyleNameMapper::FillProgName(pDesc->GetName(), aString, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, sal_True );
                    rVal <<= OUString( aString );
                }
                else
                    rVal.clear();
            }
            break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

sal_Bool SwFmtPageDesc::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_PAGEDESC_PAGENUMOFFSET:
        {
            sal_Int16 nOffset = 0;
            if(rVal >>= nOffset)
                SetNumOffset( nOffset );
            else
                bRet = sal_False;
        }
        break;

        case MID_PAGEDESC_PAGEDESCNAME:
            /* geht nicht, weil das Attribut eigentlich nicht den Namen
             * sondern einen Pointer auf den PageDesc braucht (ist Client davon).
             * Der Pointer waere aber ueber den Namen nur vom Dokument zu erfragen.
             */
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}


//  class SwFmtCol
//  Implementierung teilweise inline im hxx

SwColumn::SwColumn() :
    nWish ( 0 ),
    nUpper( 0 ),
    nLower( 0 ),
    nLeft ( 0 ),
    nRight( 0 )
{
}

sal_Bool SwColumn::operator==( const SwColumn &rCmp )
{
    return (nWish    == rCmp.GetWishWidth() &&
            GetLeft()  == rCmp.GetLeft() &&
            GetRight() == rCmp.GetRight() &&
            GetUpper() == rCmp.GetUpper() &&
            GetLower() == rCmp.GetLower()) ? sal_True : sal_False;
}

SwFmtCol::SwFmtCol( const SwFmtCol& rCpy )
    : SfxPoolItem( RES_COL ),
    nLineWidth( rCpy.nLineWidth),
    aLineColor( rCpy.aLineColor),
    nLineHeight( rCpy.GetLineHeight() ),
    eAdj( rCpy.GetLineAdj() ),
    aColumns( (sal_Int8)rCpy.GetNumCols(), 1 ),
    nWidth( rCpy.GetWishWidth() ),
    bOrtho( rCpy.IsOrtho() )
{
    for ( sal_uInt16 i = 0; i < rCpy.GetNumCols(); ++i )
    {
        SwColumn *pCol = new SwColumn( *rCpy.GetColumns()[i] );
        aColumns.Insert( pCol, aColumns.Count() );
    }
}

SwFmtCol::~SwFmtCol() {}

SwFmtCol& SwFmtCol::operator=( const SwFmtCol& rCpy )
{
    nLineWidth  = rCpy.nLineWidth;
    aLineColor  = rCpy.aLineColor;
    nLineHeight = rCpy.GetLineHeight();
    eAdj        = rCpy.GetLineAdj();
    nWidth      = rCpy.GetWishWidth();
    bOrtho      = rCpy.IsOrtho();

    if ( aColumns.Count() )
        aColumns.DeleteAndDestroy( 0, aColumns.Count() );
    for ( sal_uInt16 i = 0; i < rCpy.GetNumCols(); ++i )
    {
        SwColumn *pCol = new SwColumn( *rCpy.GetColumns()[i] );
        aColumns.Insert( pCol, aColumns.Count() );
    }
    return *this;
}

SwFmtCol::SwFmtCol()
    : SfxPoolItem( RES_COL ),
    nLineWidth(0),
    nLineHeight( 100 ),
    eAdj( COLADJ_NONE ),
    nWidth( USHRT_MAX ),
    bOrtho( sal_True )
{
}

int SwFmtCol::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    const SwFmtCol &rCmp = (const SwFmtCol&)rAttr;
    if( !(nLineWidth        == rCmp.nLineWidth  &&
          aLineColor        == rCmp.aLineColor  &&
          nLineHeight        == rCmp.GetLineHeight() &&
          eAdj               == rCmp.GetLineAdj() &&
          nWidth             == rCmp.GetWishWidth() &&
          bOrtho             == rCmp.IsOrtho() &&
          aColumns.Count() == rCmp.GetNumCols()) )
        return 0;

    for ( sal_uInt16 i = 0; i < aColumns.Count(); ++i )
        if ( !(*aColumns[i] == *rCmp.GetColumns()[i]) )
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
    if ( aColumns.Count() == 2 )
        nRet = aColumns[0]->GetRight() + aColumns[1]->GetLeft();
    else if ( aColumns.Count() > 2 )
    {
        sal_Bool bSet = sal_False;
        for ( sal_uInt16 i = 1; i < aColumns.Count()-1; ++i )
        {
            const sal_uInt16 nTmp = aColumns[i]->GetRight() + aColumns[i+1]->GetLeft();
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
            {   bSet = sal_True;
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
        for ( sal_uInt16 i = 0; i < aColumns.Count(); ++i )
        {   SwColumn *pCol = aColumns[i];
            pCol->SetLeft ( nHalf );
            pCol->SetRight( nHalf );
            if ( i == 0 )
                pCol->SetLeft( 0 );
            else if ( i == (aColumns.Count() - 1) )
                pCol->SetRight( 0 );
        }
    }
}

void SwFmtCol::Init( sal_uInt16 nNumCols, sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    //Loeschen scheint hier auf den erste Blick vielleicht etwas zu heftig;
    //anderfalls muessten allerdings alle Werte der verbleibenden SwColumn's
    //initialisiert werden.
    if ( aColumns.Count() )
        aColumns.DeleteAndDestroy( 0, aColumns.Count() );
    for ( sal_uInt16 i = 0; i < nNumCols; ++i )
    {   SwColumn *pCol = new SwColumn;
        aColumns.Insert( pCol, i );
    }
    bOrtho = sal_True;
    nWidth = USHRT_MAX;
    if( nNumCols )
        Calc( nGutterWidth, nAct );
}

void SwFmtCol::SetOrtho( sal_Bool bNew, sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    bOrtho = bNew;
    if ( bNew && aColumns.Count() )
        Calc( nGutterWidth, nAct );
}

sal_uInt16 SwFmtCol::CalcColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const
{
    ASSERT( nCol < aColumns.Count(), ":-( ColumnsArr ueberindiziert." );
    if ( nWidth != nAct )
    {
        long nW = aColumns[nCol]->GetWishWidth();
        nW *= nAct;
        nW /= nWidth;
        return sal_uInt16(nW);
    }
    else
        return aColumns[nCol]->GetWishWidth();
}

sal_uInt16 SwFmtCol::CalcPrtColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const
{
    ASSERT( nCol < aColumns.Count(), ":-( ColumnsArr ueberindiziert." );
    sal_uInt16 nRet = CalcColWidth( nCol, nAct );
    SwColumn *pCol = aColumns[nCol];
    nRet = nRet - pCol->GetLeft();
    nRet = nRet - pCol->GetRight();
    return nRet;
}

void SwFmtCol::Calc( sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    if(!GetNumCols())
        return;
    //Erstmal die Spalten mit der Aktuellen Breite einstellen, dann die
    //Wunschbreite der Spalten anhand der Gesamtwunschbreite hochrechnen.

    const sal_uInt16 nGutterHalf = nGutterWidth ? nGutterWidth / 2 : 0;

    //Breite der PrtAreas ist Gesamtbreite - Zwischenraeume / Anzahl
    const sal_uInt16 nPrtWidth =
                (nAct - ((GetNumCols()-1) * nGutterWidth)) / GetNumCols();
    sal_uInt16 nAvail = nAct;

    //Die erste Spalte ist PrtBreite + (Zwischenraumbreite/2)
    const sal_uInt16 nLeftWidth = nPrtWidth + nGutterHalf;
    SwColumn *pCol = aColumns[0];
    pCol->SetWishWidth( nLeftWidth );
    pCol->SetRight( nGutterHalf );
    pCol->SetLeft ( 0 );
    nAvail = nAvail - nLeftWidth;

    //Spalte 2 bis n-1 ist PrtBreite + Zwischenraumbreite
    const sal_uInt16 nMidWidth = nPrtWidth + nGutterWidth;
    sal_uInt16 i;

    for ( i = 1; i < GetNumCols()-1; ++i )
    {
        pCol = aColumns[i];
        pCol->SetWishWidth( nMidWidth );
        pCol->SetLeft ( nGutterHalf );
        pCol->SetRight( nGutterHalf );
        nAvail = nAvail - nMidWidth;
    }

    //Die Letzte Spalte entspricht wieder der ersten, um Rundungsfehler
    //auszugleichen wird der letzten Spalte alles zugeschlagen was die
    //anderen nicht verbraucht haben.
    pCol = aColumns[aColumns.Count()-1];
    pCol->SetWishWidth( nAvail );
    pCol->SetLeft ( nGutterHalf );
    pCol->SetRight( 0 );

    //Umrechnen der aktuellen Breiten in Wunschbreiten.
    for ( i = 0; i < aColumns.Count(); ++i )
    {
        pCol = aColumns[i];
        long nTmp = pCol->GetWishWidth();
        nTmp *= GetWishWidth();
        nTmp /= nAct;
        pCol->SetWishWidth( sal_uInt16(nTmp) );
    }
}

sal_Bool SwFmtCol::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    if(MID_COLUMN_SEPARATOR_LINE == nMemberId)
    {
        DBG_ERROR("not implemented");
    }
    else
    {
        uno::Reference< text::XTextColumns >  xCols = new SwXTextColumns(*this);
        rVal.setValue(&xCols, ::getCppuType((uno::Reference< text::XTextColumns>*)0));
    }
    return sal_True;
}

sal_Bool SwFmtCol::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_False;
    if(MID_COLUMN_SEPARATOR_LINE == nMemberId)
    {
        DBG_ERROR("not implemented");
    }
    else
    {
        uno::Reference< text::XTextColumns > xCols;
        rVal >>= xCols;
        if(xCols.is())
        {
            uno::Sequence<text::TextColumn> aSetColumns = xCols->getColumns();
            const text::TextColumn* pArray = aSetColumns.getConstArray();
            aColumns.DeleteAndDestroy(0, aColumns.Count());
            //max. Count ist hier 64K - das kann das Array aber nicht
            sal_uInt16 nCount = Min( (sal_uInt16)aSetColumns.getLength(),
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
                    aColumns.Insert(pCol, i);
                }
            bRet = sal_True;
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
                if(!pSwColums->GetSepLineIsOn())
                    eAdj = COLADJ_NONE;
                else switch(pSwColums->GetSepLineVertAlign())
                {
                    case 0: eAdj = COLADJ_TOP;  break;  //VerticalAlignment_TOP
                    case 1: eAdj = COLADJ_CENTER;break; //VerticalAlignment_MIDDLE
                    case 2: eAdj = COLADJ_BOTTOM;break; //VerticalAlignment_BOTTOM
                    default: ASSERT( !this, "unknown alignment" ); break;
                }
            }
        }
    }
    return bRet;
}


//  class SwFmtSurround
//  Implementierung teilweise inline im hxx

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
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
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


sal_Bool SwFmtSurround::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
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
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

sal_Bool SwFmtSurround::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
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
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

//  class SwFmtVertOrient
//  Implementierung teilweise inline im hxx

SwFmtVertOrient::SwFmtVertOrient( SwTwips nY, sal_Int16 eVert,
                                  sal_Int16 eRel )
    : SfxPoolItem( RES_VERT_ORIENT ),
    nYPos( nY ),
    eOrient( eVert ),
    eRelation( eRel )
{}

int  SwFmtVertOrient::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( nYPos     == ((SwFmtVertOrient&)rAttr).nYPos &&
             eOrient   == ((SwFmtVertOrient&)rAttr).eOrient &&
             eRelation == ((SwFmtVertOrient&)rAttr).eRelation );
}

SfxPoolItem*  SwFmtVertOrient::Clone( SfxItemPool* ) const
{
    return new SwFmtVertOrient( nYPos, eOrient, eRelation );
}

sal_Bool SwFmtVertOrient::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_VERTORIENT_ORIENT:
        {
            sal_Int16 nRet = text::VertOrientation::NONE;
            switch( eOrient )
            {
                case text::VertOrientation::TOP        :  nRet = text::VertOrientation::TOP        ;break;
                case text::VertOrientation::CENTER     :  nRet = text::VertOrientation::CENTER     ;break;
                case text::VertOrientation::BOTTOM     :  nRet = text::VertOrientation::BOTTOM     ;break;
                case text::VertOrientation::CHAR_TOP   :  nRet = text::VertOrientation::CHAR_TOP   ;break;
                case text::VertOrientation::CHAR_CENTER:  nRet = text::VertOrientation::CHAR_CENTER;break;
                case text::VertOrientation::CHAR_BOTTOM:  nRet = text::VertOrientation::CHAR_BOTTOM;break;
                case text::VertOrientation::LINE_TOP   :  nRet = text::VertOrientation::LINE_TOP   ;break;
                case text::VertOrientation::LINE_CENTER:  nRet = text::VertOrientation::LINE_CENTER;break;
                case text::VertOrientation::LINE_BOTTOM:  nRet = text::VertOrientation::LINE_BOTTOM;break;
                default: break;
            }
            rVal <<= nRet;
        }
        break;
        case MID_VERTORIENT_RELATION:
                rVal <<= lcl_RelToINT(eRelation);
        break;
        case MID_VERTORIENT_POSITION:
                rVal <<= (sal_Int32)TWIP_TO_MM100(GetPos());
                break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

sal_Bool SwFmtVertOrient::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_VERTORIENT_ORIENT:
        {
            sal_uInt16 nVal = 0;
            rVal >>= nVal;
            switch( nVal )
            {
                case text::VertOrientation::NONE:           eOrient = text::VertOrientation::NONE;    break;
                case text::VertOrientation::TOP        :    eOrient = text::VertOrientation::TOP;     break;
                case text::VertOrientation::CENTER     :    eOrient = text::VertOrientation::CENTER;     break;
                case text::VertOrientation::BOTTOM     :    eOrient = text::VertOrientation::BOTTOM;     break;
                case text::VertOrientation::CHAR_TOP   :    eOrient = text::VertOrientation::CHAR_TOP;   break;
                case text::VertOrientation::CHAR_CENTER:    eOrient = text::VertOrientation::CHAR_CENTER;break;
                case text::VertOrientation::CHAR_BOTTOM:    eOrient = text::VertOrientation::CHAR_BOTTOM;break;
                case text::VertOrientation::LINE_TOP   :    eOrient = text::VertOrientation::LINE_TOP;    break;
                case text::VertOrientation::LINE_CENTER:    eOrient = text::VertOrientation::LINE_CENTER;break;
                case text::VertOrientation::LINE_BOTTOM:    eOrient = text::VertOrientation::LINE_BOTTOM;break;
            }
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
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}



//  class SwFmtHoriOrient
//  Implementierung teilweise inline im hxx

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
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( nXPos == ((SwFmtHoriOrient&)rAttr).nXPos &&
             eOrient == ((SwFmtHoriOrient&)rAttr).eOrient &&
             eRelation == ((SwFmtHoriOrient&)rAttr).eRelation &&
             bPosToggle == ((SwFmtHoriOrient&)rAttr).bPosToggle );
}

SfxPoolItem*  SwFmtHoriOrient::Clone( SfxItemPool* ) const
{
    return new SwFmtHoriOrient( nXPos, eOrient, eRelation, bPosToggle );
}

sal_Bool SwFmtHoriOrient::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_HORIORIENT_ORIENT:
        {
            sal_Int16 nRet = text::HoriOrientation::NONE;
            switch( eOrient )
            {
                case text::HoriOrientation::RIGHT:    nRet = text::HoriOrientation::RIGHT; break;
                case text::HoriOrientation::CENTER :  nRet = text::HoriOrientation::CENTER; break;
                case text::HoriOrientation::LEFT   :  nRet = text::HoriOrientation::LEFT; break;
                case text::HoriOrientation::INSIDE :  nRet = text::HoriOrientation::INSIDE; break;
                case text::HoriOrientation::OUTSIDE:  nRet = text::HoriOrientation::OUTSIDE; break;
                case text::HoriOrientation::FULL:     nRet = text::HoriOrientation::FULL; break;
                case text::HoriOrientation::LEFT_AND_WIDTH :
                    nRet = text::HoriOrientation::LEFT_AND_WIDTH;
                    break;
                default:
                    break;

            }
            rVal <<= nRet;
        }
        break;
        case MID_HORIORIENT_RELATION:
            rVal <<= lcl_RelToINT(eRelation);
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
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

sal_Bool SwFmtHoriOrient::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_HORIORIENT_ORIENT:
        {
            sal_Int16 nVal = 0;
            rVal >>= nVal;
            switch( nVal )
            {
                case text::HoriOrientation::NONE:       eOrient = text::HoriOrientation::NONE ;   break;
                case text::HoriOrientation::RIGHT:  eOrient = text::HoriOrientation::RIGHT;   break;
                case text::HoriOrientation::CENTER :    eOrient = text::HoriOrientation::CENTER;  break;
                case text::HoriOrientation::LEFT   :    eOrient = text::HoriOrientation::LEFT;    break;
                case text::HoriOrientation::INSIDE :    eOrient = text::HoriOrientation::INSIDE;  break;
                case text::HoriOrientation::OUTSIDE:    eOrient = text::HoriOrientation::OUTSIDE; break;
                case text::HoriOrientation::FULL:      eOrient = text::HoriOrientation::FULL;     break;
                case text::HoriOrientation::LEFT_AND_WIDTH:
                    eOrient = text::HoriOrientation::LEFT_AND_WIDTH;
                break;
            }
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
                bRet = sal_False;
            if(bConvert)
                nVal = MM100_TO_TWIP(nVal);
            SetPos( nVal );
        }
        break;
        case MID_HORIORIENT_PAGETOGGLE:
                SetPosToggle( *(sal_Bool*)rVal.getValue());
            break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}



//  class SwFmtAnchor
//  Implementierung teilweise inline im hxx

SwFmtAnchor::SwFmtAnchor( RndStdIds nRnd, sal_uInt16 nPage )
    : SfxPoolItem( RES_ANCHOR ),
    pCntntAnchor( 0 ),
    nAnchorId( nRnd ),
    nPageNum( nPage ),
    // OD 2004-05-05 #i28701# - get always new increased order number
    mnOrder( ++mnOrderCounter )
{}

SwFmtAnchor::SwFmtAnchor( const SwFmtAnchor &rCpy )
    : SfxPoolItem( RES_ANCHOR ),
    nAnchorId( rCpy.GetAnchorId() ),
    nPageNum( rCpy.GetPageNum() ),
    // OD 2004-05-05 #i28701# - get always new increased order number
    mnOrder( ++mnOrderCounter )
{
    pCntntAnchor = rCpy.GetCntntAnchor() ?
                        new SwPosition( *rCpy.GetCntntAnchor() ) : 0;
}

 SwFmtAnchor::~SwFmtAnchor()
{
    delete pCntntAnchor;
}

void SwFmtAnchor::SetAnchor( const SwPosition *pPos )
{
    if ( pCntntAnchor )
        delete pCntntAnchor;
    pCntntAnchor = pPos ? new SwPosition( *pPos ) : 0;
        //AM Absatz gebundene Flys sollten nie in den Absatz hineinzeigen.
    if (pCntntAnchor &&
        ((FLY_AT_PARA == nAnchorId) || (FLY_AT_FLY == nAnchorId)))
    {
        pCntntAnchor->nContent.Assign( 0, 0 );
    }
}

SwFmtAnchor& SwFmtAnchor::operator=(const SwFmtAnchor& rAnchor)
{
    nAnchorId  = rAnchor.GetAnchorId();
    nPageNum   = rAnchor.GetPageNum();
    // OD 2004-05-05 #i28701# - get always new increased order number
    mnOrder = ++mnOrderCounter;

    delete pCntntAnchor;
    pCntntAnchor = rAnchor.pCntntAnchor ?
                                    new SwPosition(*(rAnchor.pCntntAnchor)) : 0;
    return *this;
}

int  SwFmtAnchor::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    // OD 2004-05-05 #i28701# - Note: <mnOrder> hasn't to be considered.
    return ( nAnchorId == ((SwFmtAnchor&)rAttr).GetAnchorId() &&
             nPageNum == ((SwFmtAnchor&)rAttr).GetPageNum()   &&
                    //Anker vergleichen. Entweder zeigen beide auf das gleiche
                    //Attribut bzw. sind 0 oder die SwPosition* sind beide
                    //gueltig und die SwPositions sind gleich.
             (pCntntAnchor == ((SwFmtAnchor&)rAttr).GetCntntAnchor() ||
              (pCntntAnchor && ((SwFmtAnchor&)rAttr).GetCntntAnchor() &&
               *pCntntAnchor == *((SwFmtAnchor&)rAttr).GetCntntAnchor())));
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

/*-----------------16.02.98 15:21-------------------

--------------------------------------------------*/
sal_Bool SwFmtAnchor::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
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
            if(pCntntAnchor && FLY_AT_FLY == nAnchorId)
            {
                SwFrmFmt* pFmt = pCntntAnchor->nNode.GetNode().GetFlyFmt();
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
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

sal_Bool SwFmtAnchor::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
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
                    if( GetPageNum() > 0 && pCntntAnchor )
                    {
                        // If the anchor type is page and a valid page number
                        // has been set, the content position isn't required
                        // any longer.
                        delete pCntntAnchor;
                        pCntntAnchor = 0;
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
                if ((FLY_AT_PAGE == GetAnchorId()) && pCntntAnchor)
                {
                    // If the anchor type is page and a valid page number
                    // is set, the content paoition has to be deleted to not
                    // confuse the layout (frmtool.cxx). However, if the
                    // anchor type is not page, any content position will
                    // be kept.
                    delete pCntntAnchor;
                    pCntntAnchor = 0;
                }
            }
            else
                bRet = sal_False;
        }
        break;
        case MID_ANCHOR_ANCHORFRAME:
        //no break here!;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

//  class SwFmtURL
//  Implementierung teilweise inline im hxx

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
    if ( pMap )
        delete pMap;
}

int SwFmtURL::operator==( const SfxPoolItem &rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    const SwFmtURL &rCmp = (SwFmtURL&)rAttr;
    sal_Bool bRet = bIsServerMap     == rCmp.IsServerMap() &&
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

void SwFmtURL::SetURL( const XubString &rURL, sal_Bool bServerMap )
{
    sURL = rURL;
    bIsServerMap = bServerMap;
}

void SwFmtURL::SetMap( const ImageMap *pM )
{
    if ( pMap )
        delete pMap;
    pMap = pM ? new ImageMap( *pM ) : 0;
}
extern const SvEventDescription* lcl_GetSupportedMacroItems();

sal_Bool SwFmtURL::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
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
                xInt = SvUnoImageMap_createInstance( *pMap, lcl_GetSupportedMacroItems() );
            }
            else
            {
                ImageMap aEmptyMap;
                xInt = SvUnoImageMap_createInstance( aEmptyMap, lcl_GetSupportedMacroItems() );
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
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

sal_Bool SwFmtURL::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
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
                bRet = sal_False;
        }
        break;
        case MID_URL_SERVERMAP:
            bIsServerMap = *(sal_Bool*)rVal.getValue();
            break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
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

sal_Bool SwFmtFtnEndAtTxtEnd::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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
    return sal_True;
}

sal_Bool SwFmtFtnEndAtTxtEnd::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bRet = sal_True;
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
                bRet = sal_False;
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
                bRet = sal_False;
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
        default: bRet = sal_False;
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
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );

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

sal_Bool SwFmtChain::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool   bRet = sal_True;
    XubString aRet;
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
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    rVal <<= OUString(aRet);
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
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );

    return nStartValue  == ((SwFmtLineNumber&)rAttr).GetStartValue() &&
           bCountLines  == ((SwFmtLineNumber&)rAttr).IsCount();
}

SfxPoolItem* SwFmtLineNumber::Clone( SfxItemPool* ) const
{
    return new SwFmtLineNumber( *this );
}

sal_Bool SwFmtLineNumber::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
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
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

sal_Bool SwFmtLineNumber::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
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
                bRet = sal_False;
        }
        break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
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
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
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

sal_Bool SwTextGridItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    sal_Bool bRet = sal_True;

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
            DBG_ASSERT( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            rVal <<= (sal_Int32) TWIP_TO_MM100_UNSIGNED(nBaseHeight);
            break;
        case MID_GRID_BASEWIDTH:
            DBG_ASSERT( (nMemberId & CONVERT_TWIPS) != 0,
                        "This value needs TWIPS-MM100 conversion" );
            rVal <<= (sal_Int32) TWIP_TO_MM100_UNSIGNED(nBaseWidth);
            break;
        case MID_GRID_RUBYHEIGHT:
            DBG_ASSERT( (nMemberId & CONVERT_TWIPS) != 0,
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
                    DBG_ERROR("unknown SwTextGrid value");
                    bRet = sal_False;
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
            DBG_ERROR("Unknown SwTextGridItem member");
            bRet = sal_False;
            break;
    }

    return bRet;
}

sal_Bool SwTextGridItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bRet = sal_True;
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
                bRet = sal_False;
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
            DBG_ASSERT( (nMemberId & CONVERT_TWIPS) != 0,
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
                bRet = sal_False;
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
                        bRet = sal_False;
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
            DBG_ERROR("Unknown SwTextGridItem member");
            bRet = sal_False;
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
//  Implementierung teilweise inline im hxx

TYPEINIT1( SwFrmFmt, SwFmt );
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( SwFrmFmt, 20, 20 )

void SwFrmFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
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
    {   //Hat er keinen, mach ich ihm einen
        SwFrmFmt *pFmt = GetDoc()->MakeLayoutFmt( RND_STD_HEADER, 0 );
        pFmt->Add( pH );
    }

    if( pF && pF->IsActive() && !pF->GetFooterFmt() )
    {   //Hat er keinen, mach ich ihm einen
        SwFrmFmt *pFmt = GetDoc()->MakeLayoutFmt( RND_STD_FOOTER, 0 );
        pFmt->Add( pF );
    }

    // MIB 24.3.98: Modify der Basisklasse muss immer gerufen werden, z.B.
    // wegen RESET_FMTWRITTEN.
//  if ( GetDepends() )
        SwFmt::Modify( pOld, pNew );

    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached uno object
        SetXObject(uno::Reference<uno::XInterface>(0));
    }
}

//Vernichtet alle Frms, die in aDepend angemeldet sind.

void SwFrmFmt::DelFrms()
{
    SwClientIter aIter( *this );
    SwClient * pLast = aIter.GoStart();
    if( pLast )
        do {
            if ( pLast->ISA(SwFrm) )
            {
                ((SwFrm*)pLast)->Cut();
                delete pLast;
            }
        } while( 0 != ( pLast = aIter++ ));
}

void SwFrmFmt::MakeFrms()
{
    ASSERT( !this, "Sorry not implemented." );
}



SwRect SwFrmFmt::FindLayoutRect( const sal_Bool bPrtArea, const Point* pPoint,
                                 const sal_Bool bCalcFrm ) const
{
    SwRect aRet;
    SwFrm *pFrm = 0;
    if( ISA( SwSectionFmt ) )
    {
        // dann den frame::Frame per Node2Layout besorgen
        SwSectionNode* pSectNd = ((SwSectionFmt*)this)->GetSectionNode();
        if( pSectNd )
        {
            SwNode2Layout aTmp( *pSectNd, pSectNd->GetIndex() - 1 );
            pFrm = aTmp.NextFrm();

            if( pFrm && pFrm->GetRegisteredIn() != this )
            {
                // die Section hat keinen eigenen frame::Frame, also falls
                // jemand die tatsaechliche Groe?e braucht, so muss das
                // noch implementier werden, in dem sich vom Ende noch
                // der entsprechende frame::Frame besorgt wird.
                // PROBLEM: was passiert bei SectionFrames, die auf unter-
                //          schiedlichen Seiten stehen??
                if( bPrtArea )
                    aRet = pFrm->Prt();
                else
                {
                    aRet = pFrm->Frm();
                    --aRet.Pos().Y();
                }
                pFrm = 0;       // das Rect ist ja jetzt fertig
            }
        }
    }
    else
    {
        sal_uInt16 nFrmType = RES_FLYFRMFMT == Which() ? FRM_FLY : USHRT_MAX;
        pFrm = ::GetFrmOfModify( *(SwModify*)this, nFrmType, pPoint,
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
    SwClientIter aIter( *this );
    return (SwContact*)aIter.First( TYPE( SwContact ) );
}

SdrObject* SwFrmFmt::FindSdrObject()
{
    // --> OD 2005-01-06 #i30669# - use method <FindContactObj()> instead of
    // duplicated code.
    SwContact* pFoundContact = FindContactObj();
    return pFoundContact ? pFoundContact->GetMaster() : 0;
    // <--
}

SdrObject* SwFrmFmt::FindRealSdrObject()
{
    if( RES_FLYFRMFMT == Which() )
    {
        Point aNullPt;
        SwFlyFrm* pFly = (SwFlyFrm*)::GetFrmOfModify( *this, FRM_FLY,
                                                    &aNullPt, 0, sal_False );
        return pFly ? pFly->GetVirtDrawObj() : 0;
    }
    return FindSdrObject();
}


sal_Bool SwFrmFmt::IsLowerOf( const SwFrmFmt& rFmt ) const
{
    //Auch eine Verkettung von Innen nach aussen oder von aussen
    //nach innen ist nicht zulaessig.
    SwClientIter aIter( *(SwModify*)this );
    SwFlyFrm *pSFly = (SwFlyFrm*)aIter.First( TYPE(SwFlyFrm) );
    if( pSFly )
    {
        SwClientIter aOtherIter( (SwModify&)rFmt );
        SwFlyFrm *pAskFly = (SwFlyFrm*)aOtherIter.First( TYPE(SwFlyFrm) );
        if( pAskFly )
            return pSFly->IsLowerOf( pAskFly );
    }

    // dann mal ueber die Node-Positionen versuchen
    const SwFmtAnchor* pAnchor = &rFmt.GetAnchor();
    if ((FLY_AT_PAGE != pAnchor->GetAnchorId()) && pAnchor->GetCntntAnchor())
    {
        const SwSpzFrmFmts& rFmts = *GetDoc()->GetSpzFrmFmts();
        const SwNode* pFlyNd = pAnchor->GetCntntAnchor()->nNode.GetNode().
                                FindFlyStartNode();
        while( pFlyNd )
        {
            // dann ueber den Anker nach oben "hangeln"
            sal_uInt16 n;
            for( n = 0; n < rFmts.Count(); ++n )
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
            if( n >= rFmts.Count() )
            {
                ASSERT( !this, "Fly-Section aber kein Format gefunden" );
                return sal_False;
            }
        }
    }
    return sal_False;
}

// --> OD 2004-07-27 #i31698#
SwFrmFmt::tLayoutDir SwFrmFmt::GetLayoutDir() const
{
    return SwFrmFmt::HORI_L2R;
}

void SwFrmFmt::SetLayoutDir( const SwFrmFmt::tLayoutDir )
{
    // empty body, because default implementation does nothing
}
// <--

// --> OD 2004-08-06 #i28749#
sal_Int16 SwFrmFmt::GetPositionLayoutDir() const
{
    return text::PositionLayoutDir::PositionInLayoutDirOfAnchor;
}
void SwFrmFmt::SetPositionLayoutDir( const sal_Int16 )
{
    // empty body, because default implementation does nothing
}
// <--
String SwFrmFmt::GetDescription() const
{
    return SW_RES(STR_FRAME);
}

//  class SwFlyFrmFmt
//  Implementierung teilweise inline im hxx

TYPEINIT1( SwFlyFrmFmt, SwFrmFmt );
IMPL_FIXEDMEMPOOL_NEWDEL( SwFlyFrmFmt,  10, 10 )

SwFlyFrmFmt::~SwFlyFrmFmt()
{
    SwClientIter aIter( *this );
    SwClient * pLast = aIter.GoStart();
    if( pLast )
        do {
            if ( pLast->ISA( SwFlyFrm ) )
                delete pLast;

        } while( 0 != ( pLast = aIter++ ));

    pLast = aIter.GoStart();
    if( pLast )
        do {
            if ( pLast->ISA( SwFlyDrawContact ) )
                delete pLast;

        } while( 0 != ( pLast = aIter++ ));
}

//Erzeugen der Frms wenn das Format einen Absatzgebundenen Rahmen beschreibt.
//MA: 14. Feb. 94, Erzeugen der Frms auch fuer Seitengebundene Rahmen.

void SwFlyFrmFmt::MakeFrms()
{
    // gibts ueberhaupt ein Layout ??
    if( !GetDoc()->GetRootFrm() )
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
            //Erst einmal ueber den Inhalt suchen, weil konstant schnell. Kann
            //Bei verketteten Rahmen aber auch schief gehen, weil dann evtl.
            //niemals ein frame::Frame zu dem Inhalt existiert. Dann muss leider noch
            //die Suche vom StartNode zum FrameFormat sein.
            SwNodeIndex aIdx( aAnchorAttr.GetCntntAnchor()->nNode );
            SwCntntNode *pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
            // --> OD 2009-12-28 #i105535#
            if ( pCNd == 0 )
            {
                pCNd = aAnchorAttr.GetCntntAnchor()->nNode.GetNode().GetCntntNode();
            }
            if ( pCNd )
            // <--
            {
                SwClientIter aIter( *pCNd );
                if ( aIter.First( TYPE(SwFrm) ) )
                {
                    pModify = pCNd;
                }
            }
            // --> OD 2009-12-28 #i105535#
            if ( pModify == 0 )
            // <--
            {
                const SwNodeIndex &rIdx = aAnchorAttr.GetCntntAnchor()->nNode;
                SwSpzFrmFmts& rFmts = *GetDoc()->GetSpzFrmFmts();
                for( sal_uInt16 i = 0; i < rFmts.Count(); ++i )
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
            SwPageFrm *pPage = (SwPageFrm*)GetDoc()->GetRootFrm()->Lower();
            if( !nPgNum && aAnchorAttr.GetCntntAnchor() )
            {
                SwCntntNode *pCNd =
                    aAnchorAttr.GetCntntAnchor()->nNode.GetNode().GetCntntNode();
                SwClientIter aIter( *pCNd );
                do
                {
                    if( aIter()->ISA( SwFrm ) )
                    {
                        pPage = ((SwFrm*)aIter())->FindPageFrm();
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
                } while ( aIter++ );
            }
            while ( pPage )
            {
                if ( pPage->GetPhyPageNum() == nPgNum )
                {
                    // --> OD 2005-06-09 #i50432# - adjust synopsis of <PlaceFly(..)>
                    pPage->PlaceFly( 0, this );
                    // <--
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
        SwClientIter aIter( *pModify );
        for( SwFrm *pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
             pFrm;
             pFrm = (SwFrm*)aIter.Next() )
        {
            sal_Bool bAdd = !pFrm->IsCntntFrm() ||
                            !((SwCntntFrm*)pFrm)->IsFollow();

            if ( FLY_AT_FLY == aAnchorAttr.GetAnchorId() && !pFrm->IsFlyFrm() )
            {
                // --> OD 2009-12-28 #i105535#
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
                // <--
            }

            if( pFrm->GetDrawObjs() )
            {
                // --> OD 2004-07-01 #i28701# - new type <SwSortedObjs>
                SwSortedObjs &rObjs = *pFrm->GetDrawObjs();
                for( sal_uInt16 i = 0; i < rObjs.Count(); ++i)
                {
                    // --> OD 2004-07-01 #i28701# - consider changed type of
                    // <SwSortedObjs> entries.
                    SwAnchoredObject* pObj = rObjs[i];
                    if( pObj->ISA(SwFlyFrm) &&
                        (&pObj->GetFrmFmt()) == this )
                    {
                        bAdd = sal_False;
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
                    pFly = new SwFlyLayFrm( this, pFrm );
                    break;

                case FLY_AT_PARA:
                case FLY_AT_CHAR:
                    pFly = new SwFlyAtCntFrm( this, pFrm );
                    break;

                case FLY_AS_CHAR:
                    pFly = new SwFlyInCntFrm( this, pFrm );
                    break;
                default:
                    ASSERT( !this, "Neuer Ankertyp" )
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
    return (SwFlyFrm*)::GetFrmOfModify( *(SwModify*)this, FRM_FLY,
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


sal_Bool SwFlyFrmFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    switch( rInfo.Which() )
    {
    case RES_CONTENT_VISIBLE:
        {
            ((SwPtrMsgPoolItem&)rInfo).pObject =
                SwClientIter( *(SwFlyFrmFmt*)this ).First( TYPE(SwFrm) );
        }
        return sal_False;

    default:
        return SwFrmFmt::GetInfo( rInfo );
    }
    return sal_True;
}

// --> OD 2009-07-14 #i73249#
void SwFlyFrmFmt::SetObjTitle( const String& rTitle, bool bBroadcast )
{
    SdrObject* pMasterObject = FindSdrObject();
    ASSERT( pMasterObject,
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
        Modify( &aOld, &aNew );
    }
    else
    {
        pMasterObject->SetTitle( rTitle );
    }
}

const String SwFlyFrmFmt::GetObjTitle() const
{
    const SdrObject* pMasterObject = FindSdrObject();
    ASSERT( pMasterObject,
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
    ASSERT( pMasterObject,
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
        Modify( &aOld, &aNew );
    }
    else
    {
        pMasterObject->SetDescription( rDescription );
    }
}

const String SwFlyFrmFmt::GetObjDescription() const
{
    const SdrObject* pMasterObject = FindSdrObject();
    ASSERT( pMasterObject,
            "<SwNoTxtNode::GetDescription(..)> - missing <SdrObject> instance" );
    if ( !pMasterObject )
    {
        return aEmptyStr;
    }

    return pMasterObject->GetDescription();
}
// <--

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

// --> OD 2006-02-28 #125892#
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
            SwClientIter aOldIter( *(aOldAnchorFmt.GetCntntAnchor()->nNode.GetNode().GetCntntNode()) );
            for( aOldIter.First( TYPE(SwFrm) ); aOldIter(); aOldIter.Next() )
            {
                ++nOldNumOfAnchFrm;
            }
            // determine 'new' number of anchor frames
            sal_uInt32 nNewNumOfAnchFrm( 0L );
            SwClientIter aNewIter( *(_rNewAnchorFmt.GetCntntAnchor()->nNode.GetNode().GetCntntNode()) );
            for( aNewIter.First( TYPE(SwFrm) ); aNewIter(); aNewIter.Next() )
            {
                ++nNewNumOfAnchFrm;
            }
            if ( nOldNumOfAnchFrm != nNewNumOfAnchFrm )
            {
                // delete existing fly frames except <_pKeepThisFlyFrm>
                SwClientIter aIter( mrFlyFrmFmt );
                SwClient* pLast = aIter.GoStart();
                if ( pLast )
                {
                    do {
                        SwFrm* pFrm( dynamic_cast<SwFrm*>(pLast) );
                        if ( pFrm && pFrm != _pKeepThisFlyFrm )
                        {
                            pFrm->Cut();
                            delete pFrm;
                        }
                    } while( 0 != ( pLast = aIter++ ));
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
// <--
//  class SwDrawFrmFmt
//  Implementierung teilweise inline im hxx

TYPEINIT1( SwDrawFrmFmt, SwFrmFmt );
IMPL_FIXEDMEMPOOL_NEWDEL( SwDrawFrmFmt, 10, 10 )

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
    if ( pContact ) //fuer den Reader und andere Unabwaegbarkeiten.
        pContact->DisconnectFromLayout();
}

// --> OD 2004-07-27 #i31698#
SwFrmFmt::tLayoutDir SwDrawFrmFmt::GetLayoutDir() const
{
    return meLayoutDir;
}

void SwDrawFrmFmt::SetLayoutDir( const SwFrmFmt::tLayoutDir _eLayoutDir )
{
    meLayoutDir = _eLayoutDir;
}
// <--

// --> OD 2004-08-06 #i28749#
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
            ASSERT( false,
                    "<SwDrawFrmFmt::SetPositionLayoutDir(..)> - invalid attribute value." );
        }
    }
}
// <--

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
        aResult = SW_RES(STR_GRAPHIC);

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
        pFly = (SwFlyFrm*) SwClientIter( *(SwFrmFmt*)this ).First( TYPE( SwFlyFrm ));
        if( !pFly )
            return 0;
    }

    //Orignialgroesse fuer OLE und Grafik ist die TwipSize,
    //ansonsten die Groesse vom FrmFmt des Fly.
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

