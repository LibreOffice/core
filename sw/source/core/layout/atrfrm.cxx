/*************************************************************************
 *
 *  $RCSfile: atrfrm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-20 14:29:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX //autogen
#include <svtools/imapobj.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _UNOSETT_HXX
#include <unosett.hxx>
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTFORDR_HXX //autogen
#include <fmtfordr.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTTSPLT_HXX //autogen
#include <fmtlsplt.hxx>
#endif
#ifndef _FMTFTNTX_HXX //autogen
#include <fmtftntx.hxx>
#endif
#ifndef _FMTEIRO_HXX //autogen
#include <fmteiro.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FMTCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _FMTLINE_HXX
#include <fmtline.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NODE2LAY_HXX
#include <node2lay.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _UNOMID_H
#include <unomid.h>
#endif

#ifndef _COM_SUN_STAR_TEXT_RELORIENTATION_HPP_
#include <com/sun/star/text/RelOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIZONTALADJUST_HPP_
#include <com/sun/star/text/HorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_DOCUMENTSTATISTIC_HPP_
#include <com/sun/star/text/DocumentStatistic.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/HoriOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_NOTEPRINTMODE_HPP_
#include <com/sun/star/text/NotePrintMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HPP_
#include <com/sun/star/text/SizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/VertOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_INVALIDTEXTCONTENTEXCEPTION_HPP_
#include <com/sun/star/text/InvalidTextContentException.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

SV_IMPL_PTRARR(SwColumns,SwColumn*)

TYPEINIT1(SwFrmFmt,     SwFmt );  //rtti fuer SwFrmFmt
TYPEINIT1(SwFlyFrmFmt,  SwFrmFmt);
TYPEINIT1(SwDrawFrmFmt, SwFrmFmt);
TYPEINIT1(SwFmtVertOrient, SfxPoolItem);
TYPEINIT1(SwFmtHoriOrient, SfxPoolItem);
TYPEINIT2(SwFmtHeader,  SfxPoolItem, SwClient );
TYPEINIT2(SwFmtFooter,  SfxPoolItem, SwClient );
TYPEINIT2(SwFmtPageDesc,  SfxPoolItem, SwClient );

/* -----------------19.05.98 09:26-------------------
 *  Umwandlung fuer QueryValue
 * --------------------------------------------------*/
sal_Int16 lcl_RelToINT(SwRelationOrient eRelation)
{
    sal_Int16 nRet = text::RelOrientation::FRAME;
    switch(eRelation)
    {
    case  PRTAREA:          nRet = text::RelOrientation::PRINT_AREA; break;
    case  REL_CHAR:         nRet = text::RelOrientation::CHAR; break;
    case  REL_PG_LEFT:      nRet = text::RelOrientation::PAGE_LEFT; break;
    case  REL_PG_RIGHT:     nRet = text::RelOrientation::PAGE_RIGHT; break;
    case  REL_FRM_LEFT:     nRet = text::RelOrientation::FRAME_LEFT; break;
    case  REL_FRM_RIGHT:    nRet = text::RelOrientation::FRAME_RIGHT; break;
    case  REL_PG_FRAME:     nRet = text::RelOrientation::PAGE_FRAME; break;
    case  REL_PG_PRTAREA:   nRet = text::RelOrientation::PAGE_PRINT_AREA; break;
    }
    return nRet;
}
SwRelationOrient    lcl_IntToRelation(const uno::Any& rVal)
{
    SwRelationOrient eRet = FRAME;
    switch(*(sal_Int16*)rVal.getValue())
    {
        case  text::RelOrientation::PRINT_AREA:     eRet =   PRTAREA           ; break;
        case  text::RelOrientation::CHAR:       eRet =   REL_CHAR          ; break;
        case  text::RelOrientation::PAGE_LEFT:    eRet =   REL_PG_LEFT       ; break;
        case  text::RelOrientation::PAGE_RIGHT:    eRet =   REL_PG_RIGHT      ; break;
        case  text::RelOrientation::FRAME_LEFT:    eRet =   REL_FRM_LEFT      ; break;
        case  text::RelOrientation::FRAME_RIGHT:    eRet =   REL_FRM_RIGHT     ; break;
        case  text::RelOrientation::PAGE_FRAME:    eRet =   REL_PG_FRAME      ; break;
        case  text::RelOrientation::PAGE_PRINT_AREA:    eRet =   REL_PG_PRTAREA    ; break;
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
                bDel = pLast->IsA( TYPE(SwFrm) );
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
                SwNodeIndex aIdx( *rCnt.GetCntntIdx(), 1 );
                //Wenn in einem der Nodes noch ein Crsr angemeldet ist, muss das
                //ParkCrsr einer (beliebigen) Shell gerufen werden.
                pNode = pDoc->GetNodes()[ aIdx ];
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
                    pNode = pDoc->GetNodes()[ aIdx ];
                }
            }
            rCnt.SetNewCntntIdx( (const SwNodeIndex*)0 );

            // beim Loeschen von Header/Footer-Formaten IMMER das Undo
            // abschalten! (Bug 31069)
            sal_Bool bDoesUndo = pDoc->DoesUndo();
            pDoc->DoUndo( sal_False );

            ASSERT( pNode, "Ein grosses Problem." );
            pDoc->DeleteSection( pNode );

            if( bDoesUndo )
            {
                pDoc->DelAllUndoObj();
                pDoc->DoUndo( sal_True );
            }
        }
        delete pFmt;
    }
}

//  class SwFmtFrmSize
//  Implementierung teilweise inline im hxx

SwFmtFrmSize::SwFmtFrmSize( SwFrmSize eSize, SwTwips nWidth, SwTwips nHeight )
    : SfxPoolItem( RES_FRM_SIZE ),
    aSize( nWidth, nHeight ),
    eFrmSize( eSize )
{
    nWidthPercent = nHeightPercent = 0;
}

SwFmtFrmSize& SwFmtFrmSize::operator=( const SwFmtFrmSize& rCpy )
{
    aSize = rCpy.GetSize();
    eFrmSize = rCpy.GetSizeType();
    nHeightPercent = rCpy.GetHeightPercent();
    nWidthPercent  = rCpy.GetWidthPercent();
    return *this;
}

int  SwFmtFrmSize::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return( eFrmSize        == ((SwFmtFrmSize&)rAttr).eFrmSize &&
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
BOOL SwFmtFrmSize::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
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
            rVal <<= (sal_Int8)GetHeightPercent();
        break;
        case MID_FRMSIZE_REL_WIDTH:
            rVal <<= (sal_Int8)GetWidthPercent();
        break;
        case MID_FRMSIZE_IS_SYNC_REL_SIZE:
        {
            BOOL bTmp = 0xFF == GetHeightPercent();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
        break;
        case MID_FRMSIZE_WIDTH :
            rVal <<= (sal_Int32)TWIP_TO_MM100(aSize.Width());
        break;
        case MID_FRMSIZE_HEIGHT:
            rVal <<= (sal_Int32)TWIP_TO_MM100(aSize.Height());
        break;
        case MID_FRMSIZE_SIZE_TYPE:
            rVal <<= (sal_Int16)GetSizeType();
        break;
        case MID_FRMSIZE_IS_AUTO_HEIGHT:
        {
            BOOL bTmp = ATT_FIX_SIZE != GetSizeType();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
        break;
    }
    return sal_True;
}

/* -----------------24.04.98 11:36-------------------
 *
 * --------------------------------------------------*/
BOOL SwFmtFrmSize::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_FRMSIZE_SIZE:
        {
            awt::Size aVal(*(awt::Size*)rVal.getValue());
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
        break;
        case MID_FRMSIZE_REL_HEIGHT:
        {
            sal_Int32 nSet;
            rVal >>= nSet;
            if(nSet > 0 && nSet <= 100)
                SetHeightPercent(nSet);
            else
                bRet = sal_False;
        }
        break;
        case MID_FRMSIZE_REL_WIDTH:
        {
            sal_Int32 nSet;
            rVal >>= nSet;
            if(nSet > 0 && nSet <= 100)
                SetWidthPercent(nSet);
            else
                bRet = sal_False;
        }
        break;
        case MID_FRMSIZE_IS_SYNC_REL_SIZE:
        {
            sal_Bool bSet = *(sal_Bool*)rVal.getValue();
            if(bSet)
                SetHeightPercent(0xff);
            else
                    // 0xff kann so nicht abgeschaltet werden
                    bRet = sal_False;
        }
        break;
        case MID_FRMSIZE_WIDTH :
        {
            sal_Int32 nWd = *(sal_Int32*)rVal.getValue();
            if(bConvert)
                nWd = MM100_TO_TWIP(nWd);
            if(nWd > 0)
                aSize.Width() = nWd;
            else
                bRet = sal_False;
        }
        break;
        case MID_FRMSIZE_HEIGHT:
        {
            sal_Int32 nHg = *(sal_Int32*)rVal.getValue();
            if(bConvert)
                nHg = MM100_TO_TWIP(nHg);
            if(nHg > 0)
                aSize.Height() = nHg;
            else
                bRet = sal_False;
        }
        break;
        case MID_FRMSIZE_SIZE_TYPE:
        {
            sal_Int16 nType = *(sal_Int16*)rVal.getValue();
            if(nType <= ATT_MIN_SIZE )
            {
                SetSizeType((SwFrmSize)nType);
            }
        }
        break;
        case MID_FRMSIZE_IS_AUTO_HEIGHT:
        {
            sal_Bool bSet = *(sal_Bool*)rVal.getValue();
            SetSizeType(bSet ? ATT_VAR_SIZE : ATT_FIX_SIZE);
        }
        break;
        default:
            bRet = sal_False;
    }
    return bRet;
}

Size  SwFmtFrmSize::GetSizeConvertedToSw31(
                const SvxLRSpaceItem *pLRSpace,
                const SvxULSpaceItem *pULSpace ) const
{
    // Sw4.0: Groesse enthaelt keine Raender
    // Sw3.x: Groesse enthaelt Raender
    // ==> Raender addieren
    Size aNewSize = GetSize();
    if( pLRSpace )
    {
        aNewSize.Width() += pLRSpace->GetLeft();
        aNewSize.Width() += pLRSpace->GetRight();
    }
    if( pULSpace )
    {
        aNewSize.Height() += pULSpace->GetUpper();
        aNewSize.Height() +=  pULSpace->GetLower();
    }
    return aNewSize;
}

Size  SwFmtFrmSize::GetSizeConvertedFromSw31(
                const SvxLRSpaceItem *pLRSpace,
                const SvxULSpaceItem *pULSpace ) const
{
    // Sw4.0: Groesse enthaelt keine Raender
    // Sw3.x: Groesse enthaelt Raender
    // ==> Raender subtrahieren
    Size aNewSize = GetSize();
    if( pLRSpace )
    {
        aNewSize.Width() -= pLRSpace->GetLeft();
        aNewSize.Width() -= pLRSpace->GetRight();
    }
    if( pULSpace )
    {
        aNewSize.Height() -= pULSpace->GetUpper();
        aNewSize.Height() -=  pULSpace->GetLower();
    }
    return aNewSize;
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
    pDefinedIn( 0 ),
    nNumOffset( rCpy.nNumOffset ),
    nDescNameIdx( rCpy.nDescNameIdx )
{
}

SwFmtPageDesc::SwFmtPageDesc( const SwPageDesc *pDesc )
    : SfxPoolItem( RES_PAGEDESC ),
    SwClient( (SwPageDesc*)pDesc ),
    pDefinedIn( 0 ),
    nNumOffset( 0 ),
    nDescNameIdx( 0xFFFF )  // IDX_NO_VALUE
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

    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_OBJECTDYING:
                //Der Pagedesc, bei dem ich angemeldet bin stirbt, ich trage
                //mich also bei meinem Format aus.
                //Dabei werden ich Deletet!!!
            if( IS_TYPE( SwFmt, pDefinedIn ))
#ifndef PRODUCT
            {
                sal_Bool bDel = ((SwFmt*)pDefinedIn)->ResetAttr( RES_PAGEDESC );
                ASSERT( bDel, ";-) FmtPageDesc nicht zerstoert." );
            }
#else
                ((SwFmt*)pDefinedIn)->ResetAttr( RES_PAGEDESC );
#endif
            else if( IS_TYPE( SwCntntNode, pDefinedIn ))
#ifndef PRODUCT
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

BOOL SwFmtPageDesc::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
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

BOOL SwFmtPageDesc::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_PAGEDESC_PAGENUMOFFSET:
            SetNumOffset( *(sal_uInt16*)rVal.getValue());
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
    nUpper( 0 ),
    nLower( 0 ),
    nLeft ( 0 ),
    nRight( 0 ),
    nWish ( 0 )
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
    nWidth( rCpy.GetWishWidth() ),
    bOrtho( rCpy.IsOrtho() ),
    aColumns( (sal_Int8)rCpy.GetNumCols(), 1 )
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
    nLineHeight( 100 ),
    eAdj( COLADJ_NONE ),
    nWidth( USHRT_MAX ),
    bOrtho( sal_True ),
    nLineWidth(0)
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
    nRet -= pCol->GetLeft();
    nRet -= pCol->GetRight();
    return nRet;
}

void SwFmtCol::Calc( sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
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
    nAvail -= nLeftWidth;

    //Spalte 2 bis n-1 ist PrtBreite + Zwischenraumbreite
    const sal_uInt16 nMidWidth = nPrtWidth + nGutterWidth;
    for ( sal_uInt16 i = 1; i < GetNumCols()-1; ++i )
    {
        pCol = aColumns[i];
        pCol->SetWishWidth( nMidWidth );
        pCol->SetLeft ( nGutterHalf );
        pCol->SetRight( nGutterHalf );
        nAvail -= nMidWidth;
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

BOOL SwFmtCol::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    if(MID_COLUMN_SEPARATOR_LINE == nMemberId)
    {
        DBG_ERROR("not implemented")
    }
    else
    {
        uno::Reference< text::XTextColumns >  xCols = new SwXTextColumns(*this);
        rVal.setValue(&xCols, ::getCppuType((uno::Reference< text::XTextColumns>*)0));
    }
    return sal_True;
}

BOOL SwFmtCol::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_False;
    if(MID_COLUMN_SEPARATOR_LINE == nMemberId)
    {
        DBG_ERROR("not implemented")
    }
    else
    {
        uno::Reference< text::XTextColumns > * pxCols = (uno::Reference< text::XTextColumns > *)rVal.getValue();
        if(pxCols)
        {
            uno::Sequence<text::TextColumn> aSetColumns = (*pxCols)->getColumns();
            const text::TextColumn* pArray = aSetColumns.getConstArray();
            aColumns.DeleteAndDestroy(0, aColumns.Count());
            //max. Count ist hier 64K - das kann das Array aber nicht
            sal_uInt16 nCount = min((sal_uInt16)aSetColumns.getLength(), 0x3fff);
            sal_uInt16 nWidthSum = 0;
            for(sal_uInt16 i = 0; i < nCount; i++)
            {
                SwColumn* pCol = new SwColumn;
                pCol->SetWishWidth( pArray[i].Width );
                nWidthSum += pArray[i].Width;
                pCol->SetLeft ( MM100_TO_TWIP(pArray[i].LeftMargin) );
                pCol->SetRight( MM100_TO_TWIP(pArray[i].RightMargin) );
                aColumns.Insert(pCol, i);
            }
            bRet = sal_True;
            nWidth = nWidthSum;
            bOrtho = sal_False;
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


BOOL SwFmtSurround::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
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
            BOOL bTmp = IsAnchorOnly();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
                break;
        case MID_SURROUND_CONTOUR:
        {
            BOOL bTmp = IsContour();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
                break;
        case MID_SURROUND_CONTOUROUTSIDE:
        {
            BOOL bTmp = IsOutside();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
                break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

BOOL SwFmtSurround::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_SURROUND_SURROUNDTYPE:
        {
            sal_Int32 eVal;
            try
            {
                eVal = comphelper::getEnumAsINT32(rVal);
            }
            catch(...) {}

            if( eVal >= 0 && eVal < (sal_Int16)SURROUND_END )
                SetValue( eVal );
            else
                //exception
                ;
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

SwFmtVertOrient::SwFmtVertOrient( SwTwips nY, SwVertOrient eVert,
                                  SwRelationOrient eRel )
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


SwTwips  SwFmtVertOrient::GetPosConvertedToSw31(
    const SvxULSpaceItem *pULSpace ) const
{
    SwTwips nNewPos = GetPos();

    if( VERT_NONE==GetVertOrient() && pULSpace )
    {
        nNewPos -= pULSpace->GetUpper();
    }

    return nNewPos;
}

SwTwips  SwFmtVertOrient::GetPosConvertedFromSw31(
    const SvxULSpaceItem *pULSpace ) const
{
    SwTwips nNewPos = GetPos();

    if( VERT_NONE==GetVertOrient() && pULSpace )
    {
        nNewPos += pULSpace->GetUpper();
    }

    return nNewPos;
}


BOOL SwFmtVertOrient::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
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
                case VERT_TOP        :  nRet = text::VertOrientation::TOP        ;break;
                case VERT_CENTER     :  nRet = text::VertOrientation::CENTER     ;break;
                case VERT_BOTTOM     :  nRet = text::VertOrientation::BOTTOM     ;break;
                case VERT_CHAR_TOP   :  nRet = text::VertOrientation::CHAR_TOP   ;break;
                case VERT_CHAR_CENTER:  nRet = text::VertOrientation::CHAR_CENTER;break;
                case VERT_CHAR_BOTTOM:  nRet = text::VertOrientation::CHAR_BOTTOM;break;
                case VERT_LINE_TOP   :  nRet = text::VertOrientation::LINE_TOP   ;break;
                case VERT_LINE_CENTER:  nRet = text::VertOrientation::LINE_CENTER;break;
                case VERT_LINE_BOTTOM:  nRet = text::VertOrientation::LINE_BOTTOM;break;
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

BOOL SwFmtVertOrient::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_VERTORIENT_ORIENT:
        {
            sal_uInt16 nVal;
            rVal >>= nVal;
            switch( nVal )
            {
                case text::VertOrientation::NONE:           eOrient = VERT_NONE;    break;
                case text::VertOrientation::TOP        :    eOrient = VERT_TOP;     break;
                case text::VertOrientation::CENTER     :    eOrient = VERT_CENTER;     break;
                case text::VertOrientation::BOTTOM     :    eOrient = VERT_BOTTOM;     break;
                case text::VertOrientation::CHAR_TOP   :    eOrient = VERT_CHAR_TOP;   break;
                case text::VertOrientation::CHAR_CENTER:    eOrient = VERT_CHAR_CENTER;break;
                case text::VertOrientation::CHAR_BOTTOM:    eOrient = VERT_CHAR_BOTTOM;break;
                case text::VertOrientation::LINE_TOP   :    eOrient = VERT_LINE_TOP;    break;
                case text::VertOrientation::LINE_CENTER:    eOrient = VERT_LINE_CENTER;break;
                case text::VertOrientation::LINE_BOTTOM:    eOrient = VERT_LINE_BOTTOM;break;
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
            sal_Int32 nVal;
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

SwFmtHoriOrient::SwFmtHoriOrient( SwTwips nX, SwHoriOrient eHori,
                              SwRelationOrient eRel, sal_Bool bPos )
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


SwTwips  SwFmtHoriOrient::GetPosConvertedToSw31(
    const SvxLRSpaceItem *pLRSpace ) const
{
    SwTwips nNewPos = GetPos();

    if( HORI_NONE==GetHoriOrient() && pLRSpace )
    {
        nNewPos -= pLRSpace->GetLeft();
    }

    return nNewPos;
}

SwTwips  SwFmtHoriOrient::GetPosConvertedFromSw31(
    const SvxLRSpaceItem *pLRSpace ) const
{
    SwTwips nNewPos = GetPos();

    if( HORI_NONE==GetHoriOrient() && pLRSpace )
    {
        nNewPos += pLRSpace->GetLeft();
    }

    return nNewPos;
}

BOOL SwFmtHoriOrient::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
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
                case HORI_RIGHT:    nRet = text::HoriOrientation::RIGHT; break;
                case HORI_CENTER :  nRet = text::HoriOrientation::CENTER; break;
                case HORI_LEFT   :  nRet = text::HoriOrientation::LEFT; break;
                case HORI_INSIDE :  nRet = text::HoriOrientation::INSIDE; break;
                case HORI_OUTSIDE:  nRet = text::HoriOrientation::OUTSIDE; break;
                case HORI_FULL:     nRet = text::HoriOrientation::FULL; break;
                case HORI_LEFT_AND_WIDTH :
                    nRet = text::HoriOrientation::LEFT_AND_WIDTH;
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
            BOOL bTmp = IsPosToggle();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
                break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

BOOL SwFmtHoriOrient::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_HORIORIENT_ORIENT:
        {
            switch( *(sal_Int16*)rVal.getValue() )
            {
                case text::HoriOrientation::NONE:       eOrient = HORI_NONE ;   break;
                case text::HoriOrientation::RIGHT:  eOrient = HORI_RIGHT;   break;
                case text::HoriOrientation::CENTER :    eOrient = HORI_CENTER;  break;
                case text::HoriOrientation::LEFT   :    eOrient = HORI_LEFT;    break;
                case text::HoriOrientation::INSIDE :    eOrient = HORI_INSIDE;  break;
                case text::HoriOrientation::OUTSIDE:    eOrient = HORI_OUTSIDE; break;
                case text::HoriOrientation::FULL:      eOrient = HORI_FULL;     break;
                case text::HoriOrientation::LEFT_AND_WIDTH:
                    eOrient = HORI_LEFT_AND_WIDTH;
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
            sal_Int32 nVal = *(sal_Int32*)rVal.getValue();
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
    nPageNum( nPage )
{}

SwFmtAnchor::SwFmtAnchor( const SwFmtAnchor &rCpy )
    : SfxPoolItem( RES_ANCHOR ),
    nAnchorId( rCpy.GetAnchorId() ),
    nPageNum( rCpy.GetPageNum() )
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
    if ( pCntntAnchor && ( FLY_AT_CNTNT == nAnchorId ||
                           FLY_AT_FLY == nAnchorId ))
        pCntntAnchor->nContent.Assign( 0, 0 );
}

SwFmtAnchor& SwFmtAnchor::operator=(const SwFmtAnchor& rAnchor)
{
    nAnchorId  = rAnchor.GetAnchorId();
    nPageNum   = rAnchor.GetPageNum();

    delete pCntntAnchor;
    pCntntAnchor = rAnchor.pCntntAnchor ?
                                    new SwPosition(*(rAnchor.pCntntAnchor)) : 0;
    return *this;
}

int  SwFmtAnchor::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
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

/*-----------------16.02.98 15:21-------------------

--------------------------------------------------*/
BOOL SwFmtAnchor::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_ANCHOR_ANCHORTYPE:

            text::TextContentAnchorType eRet;
            switch((sal_Int16)GetAnchorId())
            {
                case  FLY_AUTO_CNTNT : eRet = text::TextContentAnchorType_AT_CHARACTER;break;
                case  FLY_PAGE       : eRet = text::TextContentAnchorType_AT_PAGE;      break;
                case  FLY_AT_FLY     : eRet = text::TextContentAnchorType_AT_FRAME;    break;
                case  FLY_IN_CNTNT   : eRet = text::TextContentAnchorType_AS_CHARACTER;break;
                //case  FLY_AT_CNTNT  :
                default: eRet = text::TextContentAnchorType_AT_PARAGRAPH;
            }
            rVal <<= eRet;
        break;
        case MID_ANCHOR_PAGENUM:
            rVal <<= (sal_Int16)GetPageNum();
            break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

BOOL SwFmtAnchor::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_ANCHOR_ANCHORTYPE:
        {
            RndStdIds   eAnchor;
            sal_Int32 eVal;
            try
            {
                eVal = comphelper::getEnumAsINT32(rVal);
            }
            catch(...) {}
            switch(eVal)
            {
                case  text::TextContentAnchorType_AS_CHARACTER: eAnchor = FLY_IN_CNTNT; break;
                case  text::TextContentAnchorType_AT_PAGE      : eAnchor = FLY_PAGE;        break;
                case  text::TextContentAnchorType_AT_FRAME    : eAnchor = FLY_AT_FLY;       break;
                case  text::TextContentAnchorType_AT_CHARACTER: eAnchor = FLY_AUTO_CNTNT;   break;
                //case  text::TextContentAnchorType_AT_PARAGRAPH:
                default: eAnchor = FLY_AT_CNTNT;
            }
            SetType( eAnchor );
        }
        break;
        case MID_ANCHOR_PAGENUM:
        {
            sal_Int16 nVal = *(sal_Int16*)rVal.getValue();
            if(nVal > 0)
                SetPageNum( nVal );
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
    sURL( rURL.GetURL() ),
    sTargetFrameName( rURL.GetTargetFrameName() ),
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

SfxPoolItem* SwFmtURL::Clone( SfxItemPool* pPool ) const
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


BOOL SwFmtURL::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
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
            ASSERT( !this, "not implemented" );
            break;
        case MID_URL_SERVERMAP:
        {
            BOOL bTmp = IsServerMap();
            rVal.setValue(&bTmp, ::getBooleanCppuType());
        }
            break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}

BOOL SwFmtURL::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_URL_URL:
            SetURL( *(OUString*)rVal.getValue(), bIsServerMap );
        break;
        case MID_URL_TARGET:
            SetTargetFrameName( *(OUString*)rVal.getValue() );
        break;
        case MID_URL_HYPERLINKNAME:
            SetName( *(OUString*)rVal.getValue() );
        break;
        case MID_URL_CLIENTMAP:
            ASSERT( !this, "not implemented" );
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

SfxPoolItem* SwFmtEditInReadonly::Clone( SfxItemPool* pPool ) const
{
    return new SwFmtEditInReadonly( Which(), GetValue() );
}

// class SwFmtLayoutSplit

SfxPoolItem* SwFmtLayoutSplit::Clone( SfxItemPool* pPool ) const
{
    return new SwFmtLayoutSplit( GetValue() );
}

// class SwFmtNoBalancedColumns

SfxPoolItem* SwFmtNoBalancedColumns::Clone( SfxItemPool* pPool ) const
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
            aFmt.eType == rAttr.aFmt.eType &&
            nOffset == rAttr.nOffset &&
            sPrefix == rAttr.sPrefix &&
            sSuffix == rAttr.sSuffix;
}

BOOL SwFmtFtnEndAtTxtEnd::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    return SfxEnumItem::QueryValue( rVal, nMemberId );
}

BOOL SwFmtFtnEndAtTxtEnd::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    return SfxEnumItem::PutValue( rVal, nMemberId );
}


// class SwFmtFtnAtTxtEnd

SfxPoolItem* SwFmtFtnAtTxtEnd::Clone( SfxItemPool* pPool ) const
{
    SwFmtFtnAtTxtEnd* pNew = new SwFmtFtnAtTxtEnd;
    *pNew = *this;
    return pNew;
}

// class SwFmtEndAtTxtEnd

SfxPoolItem* SwFmtEndAtTxtEnd::Clone( SfxItemPool* pPool ) const
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

SfxPoolItem* SwFmtChain::Clone( SfxItemPool* pPool ) const
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

BOOL SwFmtChain::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
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

SfxPoolItem* SwFmtLineNumber::Clone( SfxItemPool* pPool ) const
{
    return new SwFmtLineNumber( *this );
}

BOOL SwFmtLineNumber::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_LINENUMBER_COUNT:
        {
            BOOL bTmp = IsCount();
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

BOOL SwFmtLineNumber::PutValue( const uno::Any& rVal, BYTE nMemberId )
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
            SetStartValue( *(sal_Int32*)rVal.getValue());
            break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}



//  class SwFrmFmt
//  Implementierung teilweise inline im hxx

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
        SwFrmFmt *pFmt = GetDoc()->MakeLayoutFmt( RND_STD_HEADER );
        pFmt->Add( pH );
    }

    if( pF && pF->IsActive() && !pF->GetFooterFmt() )
    {   //Hat er keinen, mach ich ihm einen
        SwFrmFmt *pFmt = GetDoc()->MakeLayoutFmt( RND_STD_FOOTER );
        pFmt->Add( pF );
    }

    // MIB 24.3.98: Modify der Basisklasse muss immer gerufen werden, z.B.
    // wegen RESET_FMTWRITTEN.
//  if ( GetDepends() )
        SwFmt::Modify( pOld, pNew );
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
        // dann den ::com::sun::star::frame::Frame per Node2Layout besorgen
        SwSectionNode* pSectNd = ((SwSectionFmt*)this)->GetSectionNode();
        if( pSectNd )
        {
            SwNode2Layout aTmp( *pSectNd, pSectNd->GetIndex() - 1 );
            pFrm = aTmp.NextFrm();

            if( pFrm && pFrm->GetRegisteredIn() != this )
            {
                // die Section hat keinen eigenen ::com::sun::star::frame::Frame, also falls
                // jemand die tatsaechliche Groeáe braucht, so muss das
                // noch implementier werden, in dem sich vom Ende noch
                // der entsprechende ::com::sun::star::frame::Frame besorgt wird.
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
    SwClientIter aIter( *this );
    SwClient* pFnd = aIter.First( TYPE( SwContact ) );
    return pFnd ? ((SwContact*)pFnd)->GetMaster() : 0;
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
    if( FLY_PAGE != pAnchor->GetAnchorId() && pAnchor->GetCntntAnchor() )
    {
        const SwSpzFrmFmts& rFmts = *GetDoc()->GetSpzFrmFmts();
        const SwNode* pFlyNd = pAnchor->GetCntntAnchor()->nNode.GetNode().
                                FindFlyStartNode();
        while( pFlyNd )
        {
            // dann ueber den Anker nach oben "hangeln"
            for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
            {
                const SwFrmFmt* pFmt = rFmts[ n ];
                const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                if( pIdx && pFlyNd == &pIdx->GetNode() )
                {
                    if( pFmt == this )
                        return sal_True;

                    pAnchor = &pFmt->GetAnchor();
                    if( FLY_PAGE == pAnchor->GetAnchorId() ||
                        !pAnchor->GetCntntAnchor() )
                        return sal_False;

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

//  class SwFlyFrmFmt
//  Implementierung teilweise inline im hxx

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
    const SwFmtAnchor &rAnch = GetAnchor();
    switch( rAnch.GetAnchorId() )
    {
    case FLY_IN_CNTNT:
    case FLY_AT_CNTNT:
    case FLY_AUTO_CNTNT:
        if( rAnch.GetCntntAnchor() )
            pModify = rAnch.GetCntntAnchor()->nNode.GetNode().GetCntntNode();
        break;

    case FLY_AT_FLY:
        if( rAnch.GetCntntAnchor() )
        {
            //Erst einmal ueber den Inhalt suchen, weil konstant schnell. Kann
            //Bei verketteten Rahmen aber auch schief gehen, weil dann evtl.
            //niemals ein ::com::sun::star::frame::Frame zu dem Inhalt existiert. Dann muss leider noch
            //die Suche vom StartNode zum FrameFormat sein.
            SwNodeIndex aIdx( rAnch.GetCntntAnchor()->nNode );
            SwCntntNode *pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
            SwClientIter aIter( *pCNd );
            if ( aIter.First( TYPE(SwFrm) ) )
                pModify = pCNd;
            else
            {
                const SwNodeIndex &rIdx = rAnch.GetCntntAnchor()->nNode;
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

    case FLY_PAGE:
        {
            const sal_uInt16 nPgNum = rAnch.GetPageNum();
            SwPageFrm *pPage = (SwPageFrm*)GetDoc()->GetRootFrm()->Lower();
            while ( pPage )
            {
                if ( pPage->GetPhyPageNum() == nPgNum )
                {
                    pPage->PlaceFly( 0, this, &rAnch );
                    break;
                }
                pPage = (SwPageFrm*)pPage->GetNext();
            }
        }
        break;
    }

    if( pModify )
    {
        SwClientIter aIter( *pModify );
        for( SwFrm *pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
             pFrm;
             pFrm = (SwFrm*)aIter.Next() )
        {
            FASTBOOL bAdd = !pFrm->IsCntntFrm() ||
                            !((SwCntntFrm*)pFrm)->IsFollow();

            if ( FLY_AT_FLY == rAnch.GetAnchorId() && !pFrm->IsFlyFrm() )
                pFrm = pFrm->FindFlyFrm();

            if( pFrm->GetDrawObjs() )
            {
                SwDrawObjs &rObjs = *pFrm->GetDrawObjs();
                for( sal_uInt16 i = 0; i < rObjs.Count(); ++i)
                {
                    SdrObject *pO = rObjs[i];
                    if( pO->ISA( SwVirtFlyDrawObj ) &&
                        ((SwVirtFlyDrawObj*)pO)->GetFmt() == this )
                    {
                        bAdd = sal_False;
                        break;
                    }
                }
            }

            if( bAdd )
            {
                SwFlyFrm *pFly = 0;
                switch( rAnch.GetAnchorId() )
                {
                case FLY_AT_FLY:
                    pFly = new SwFlyLayFrm( this, pFrm );
                    break;

                case FLY_AT_CNTNT:
                case FLY_AUTO_CNTNT:
                    pFly = new SwFlyAtCntFrm( this, pFrm );
                    break;

                case FLY_IN_CNTNT:
                    pFly = new SwFlyInCntFrm( this, pFrm );
                    break;
#ifndef PRODUCT
                default:
                    ASSERT( !this, "Neuer Ankertyp" );
#endif
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

//  class SwDrawFrmFmt
//  Implementierung teilweise inline im hxx

#pragma optimize( "e", off )

SwDrawFrmFmt::~SwDrawFrmFmt()
{
    SwContact *pContact = FindContactObj();
    delete pContact;
}

#pragma optimize( "e", on )

void SwDrawFrmFmt::MakeFrms()
{
    SwDrawContact *pContact = (SwDrawContact*)FindContactObj();
    pContact->ConnectToLayout();
}

void SwDrawFrmFmt::DelFrms()
{
    SwDrawContact *pContact = (SwDrawContact *)FindContactObj();
    if ( pContact ) //fuer den Reader und andere Unabwaegbarkeiten.
        pContact->DisconnectFromLayout();
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
    SwNoTxtNode *pNd;
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
            if ( RES_MIRROR_GRF_BOTH == nMirror )
                nFlags = IMAP_MIRROR_HORZ | IMAP_MIRROR_VERT;
            else if ( RES_MIRROR_GRF_VERT == nMirror )
                nFlags = IMAP_MIRROR_VERT;
            else if ( RES_MIRROR_GRF_HOR == nMirror )
                nFlags = IMAP_MIRROR_HORZ;

        }
        return ((ImageMap*)rURL.GetMap())->GetHitIMapObject( aOrigSz,
                                                aActSz, aPos, nFlags );
    }

    return 0;
}



