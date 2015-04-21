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

#include <cmdid.h>
#include <svx/svdmodel.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/frmdiritem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <ftninfo.hxx>
#include <fesh.hxx>
#include <ndole.hxx>
#include <mdiexp.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include <rootfrm.hxx>
#include <frmtool.hxx>
#include <poolfmt.hxx>
#include <docsh.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <fntcache.hxx>
#include <viewopt.hxx>
#include <fldbas.hxx>
#include <swwait.hxx>
#include <GetMetricVal.hxx>
#include <statstr.hrc>
#include <hints.hxx>
#include <SwUndoPageDesc.hxx>
#include <pagedeschint.hxx>
#include <tgrditem.hxx>
#include <unotools/syslocale.hxx>

#include <boost/scoped_ptr.hpp>

using namespace com::sun::star;

static void lcl_DefaultPageFmt( sal_uInt16 nPoolFmtId,
                                SwFrmFmt &rFmt1,
                                SwFrmFmt &rFmt2,
                                SwFrmFmt &rFmt3,
                                SwFrmFmt &rFmt4)
{
    // --> #i41075# Printer on demand
    // This function does not require a printer anymore.
    // The default page size is obtained from the application
    //locale

    SwFmtFrmSize aFrmSize( ATT_FIX_SIZE );
    const Size aPhysSize = SvxPaperInfo::GetDefaultPaperSize();
    aFrmSize.SetSize( aPhysSize );

    // Prepare for default margins.
    // Margins have a default minimum size.
    // If the printer forces a larger margins, that's ok too.
    // The HTML page desc had A4 as page size always.
    // This has been changed to take the page size from the printer.
    // Unfortunately, the margins of the HTML page desc are smaller than
    // the margins used here in general, so one extra case is required.
    // In the long term, this needs to be changed to always keep the
    // margins from the page desc.
    sal_Int32 nMinTop, nMinBottom, nMinLeft, nMinRight;
    if( RES_POOLPAGE_HTML == nPoolFmtId )
    {
        nMinRight = nMinTop = nMinBottom = GetMetricVal( CM_1 );
        nMinLeft = nMinRight * 2;
    }
    else if( MEASURE_METRIC == SvtSysLocale().GetLocaleData().getMeasurementSystemEnum() )
    {
        nMinTop = nMinBottom = nMinLeft = nMinRight = 1134; // 2 centimetres
    }
    else
    {
        nMinTop = nMinBottom = 1440;    // as in MS Word: 1 Inch
        nMinLeft = nMinRight = 1800;    //                1,25 Inch
    }

    // set margins
    SvxLRSpaceItem aLR( RES_LR_SPACE );
    SvxULSpaceItem aUL( RES_UL_SPACE );

    aUL.SetUpper( (sal_uInt16)nMinTop );
    aUL.SetLower( (sal_uInt16)nMinBottom );
    aLR.SetRight( nMinRight );
    aLR.SetLeft( nMinLeft );

    rFmt1.SetFmtAttr( aFrmSize );
    rFmt1.SetFmtAttr( aLR );
    rFmt1.SetFmtAttr( aUL );

    rFmt2.SetFmtAttr( aFrmSize );
    rFmt2.SetFmtAttr( aLR );
    rFmt2.SetFmtAttr( aUL );

    rFmt3.SetFmtAttr( aFrmSize );
    rFmt3.SetFmtAttr( aLR );
    rFmt3.SetFmtAttr( aUL );

    rFmt4.SetFmtAttr( aFrmSize );
    rFmt4.SetFmtAttr( aLR );
    rFmt4.SetFmtAttr( aUL );
}

static void lcl_DescSetAttr( const SwFrmFmt &rSource, SwFrmFmt &rDest,
                         const bool bPage = true )
{
    // We should actually use ItemSet's Intersect here, but that doesn't work
    // correctly if we have different WhichRanges.

    // Take over the attributes which are of interest.
    sal_uInt16 const aIdArr[] = {
        RES_FRM_SIZE,                   RES_UL_SPACE,                   // [83..86
        RES_BACKGROUND,                 RES_SHADOW,                     // [99..101
        RES_COL,                        RES_COL,                        // [103
        RES_TEXTGRID,                   RES_TEXTGRID,                   // [109
        RES_FRAMEDIR,                   RES_FRAMEDIR,                   // [114
        RES_HEADER_FOOTER_EAT_SPACING,  RES_HEADER_FOOTER_EAT_SPACING,  // [115
        RES_UNKNOWNATR_CONTAINER,       RES_UNKNOWNATR_CONTAINER,       // [143

        //UUUU take over DrawingLayer FillStyles
        XATTR_FILL_FIRST,               XATTR_FILL_LAST,                // [1014

        0};

    const SfxPoolItem* pItem;
    for( sal_uInt16 n = 0; aIdArr[ n ]; n += 2 )
    {
        for( sal_uInt16 nId = aIdArr[ n ]; nId <= aIdArr[ n+1]; ++nId )
        {
            // #i45539#
            // bPage == true:
            // All in aIdArr except from RES_HEADER_FOOTER_EAT_SPACING
            // bPage == false:
            // All in aIdArr except from RES_COL and RES_PAPER_BIN:
            bool bExecuteId(true);

            if(bPage)
            {
                // When Page
                switch(nId)
                {
                    // All in aIdArr except from RES_HEADER_FOOTER_EAT_SPACING
                    case RES_HEADER_FOOTER_EAT_SPACING:
                    //UUUU take out SvxBrushItem; it's the result of the fallback
                    // at SwFmt::GetItemState and not really in state SfxItemState::SET
                    case RES_BACKGROUND:
                        bExecuteId = false;
                        break;
                    default:
                        break;
                }
            }
            else
            {
                // When not Page
                switch(nId)
                {
                    // When not Page: All in aIdArr except from RES_COL and RES_PAPER_BIN:
                    case RES_COL:
                    case RES_PAPER_BIN:
                        bExecuteId = false;
                        break;
                    default:
                        break;
                }
            }

            if(bExecuteId)
            {
                if (SfxItemState::SET == rSource.GetItemState(nId, false, &pItem))
                {
                    rDest.SetFmtAttr(*pItem);
                }
                else
                {
                    rDest.ResetFmtAttr(nId);
                }
            }
        }
    }

    // Transmit pool and help IDs too
    rDest.SetPoolFmtId( rSource.GetPoolFmtId() );
    rDest.SetPoolHelpId( rSource.GetPoolHelpId() );
    rDest.SetPoolHlpFileId( rSource.GetPoolHlpFileId() );
}

namespace
{
    SwFrmFmt& getFrmFmt(SwPageDesc &rDesc, bool bLeft, bool bFirst)
    {
        if (bFirst)
        {
            if (bLeft)
                return rDesc.GetFirstLeft();
            return rDesc.GetFirstMaster();
        }
        return rDesc.GetLeft();
    }

    const SwFrmFmt& getConstFrmFmt(const SwPageDesc &rDesc, bool bLeft, bool bFirst)
    {
        return getFrmFmt(const_cast<SwPageDesc&>(rDesc), bLeft, bFirst);
    }
}

void SwDoc::CopyMasterHeader(const SwPageDesc &rChged, const SwFmtHeader &rHead, SwPageDesc &rDesc, bool bLeft, bool bFirst)
{
    assert(bLeft || bFirst);
    SwFrmFmt& rDescFrmFmt = getFrmFmt(rDesc, bLeft, bFirst);
    if (bFirst && bLeft)
    {
        // special case: always shared with something
        rDescFrmFmt.SetFmtAttr( rChged.IsFirstShared()
                ? rDesc.GetLeft().GetHeader()
                : rDesc.GetFirstMaster().GetHeader());
    }
    else if ((bFirst ? rChged.IsFirstShared() : rChged.IsHeaderShared())
         || !rHead.IsActive())
    {
        // Left or first shares the header with the Master.
        rDescFrmFmt.SetFmtAttr( rDesc.GetMaster().GetHeader() );
    }
    else if ( rHead.IsActive() )
    {   // Left or first gets its own header if the Format doesn't alrady have one.
        // If it already has one and it points to the same Section as the
        // Right one, it needs to get an own Header.
        // The content is evidently copied.
        const SwFmtHeader &rFmtHead = rDescFrmFmt.GetHeader();
        if ( !rFmtHead.IsActive() )
        {
            SwFmtHeader aHead( getIDocumentLayoutAccess().MakeLayoutFmt( RND_STD_HEADERL, 0 ) );
            rDescFrmFmt.SetFmtAttr( aHead );
            // take over additional attributes (margins, borders ...)
            ::lcl_DescSetAttr( *rHead.GetHeaderFmt(), *aHead.GetHeaderFmt(), false);
        }
        else
        {
            const SwFrmFmt *pRight = rHead.GetHeaderFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aCnt = rFmtHead.GetHeaderFmt()->GetCntnt();

            if (!aCnt.GetCntntIdx())
            {
                const SwFrmFmt& rChgedFrmFmt = getConstFrmFmt(rChged, bLeft, bFirst);
                rDescFrmFmt.SetFmtAttr( rChgedFrmFmt.GetHeader() );
            }
            else if ((*aRCnt.GetCntntIdx() == *aCnt.GetCntntIdx()) ||
                // The CntntIdx is _always_ different when called from
                // SwDocStyleSheet::SetItemSet, because it deep-copies the
                // PageDesc.  So check if it was previously shared.
                 ((bFirst) ? rDesc.IsFirstShared() : rDesc.IsHeaderShared()))
            {
                SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(),
                        (bFirst) ? "First header" : "Left header",
                                                GetDfltFrmFmt() );
                ::lcl_DescSetAttr( *pRight, *pFmt, false );
                // The section which the right header attribute is pointing
                // is copied, and the Index to the StartNode is set to
                // the left or first header attribute.
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwHeaderStartNode );
                SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
                            *aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes()._Copy( aRange, aTmp, false );
                aTmp = *pSttNd;
                GetDocumentContentOperationsManager().CopyFlyInFlyImpl(aRange, 0, aTmp);

                pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ) );
                rDescFrmFmt.SetFmtAttr( SwFmtHeader( pFmt ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *const_cast<SwFrmFmt*>(rFmtHead.GetHeaderFmt()), false );
        }
    }
}

void SwDoc::CopyMasterFooter(const SwPageDesc &rChged, const SwFmtFooter &rFoot, SwPageDesc &rDesc, bool bLeft, bool bFirst)
{
    assert(bLeft || bFirst);
    SwFrmFmt& rDescFrmFmt = getFrmFmt(rDesc, bLeft, bFirst);
    if (bFirst && bLeft)
    {
        // special case: always shared with something
        rDescFrmFmt.SetFmtAttr( rChged.IsFirstShared()
                ? rDesc.GetLeft().GetFooter()
                : rDesc.GetFirstMaster().GetFooter());
    }
    else if ((bFirst ? rChged.IsFirstShared() : rChged.IsFooterShared())
        || !rFoot.IsActive())
    {
        // Left or first shares the Header with the Master.
        rDescFrmFmt.SetFmtAttr( rDesc.GetMaster().GetFooter() );
    }
    else if ( rFoot.IsActive() )
    {   // Left or first gets its own Footer if the Format does not already have one.
        // If the Format already has a Footer and it points to the same section as the Right one,
        // it needs to get an own one.
        // The content is evidently copied.
        const SwFmtFooter &rFmtFoot = rDescFrmFmt.GetFooter();
        if ( !rFmtFoot.IsActive() )
        {
            SwFmtFooter aFoot( getIDocumentLayoutAccess().MakeLayoutFmt( RND_STD_FOOTER, 0 ) );
            rDescFrmFmt.SetFmtAttr( aFoot );
            // Take over additional attributes (margins, borders ...).
            ::lcl_DescSetAttr( *rFoot.GetFooterFmt(), *aFoot.GetFooterFmt(), false);
        }
        else
        {
            const SwFrmFmt *pRight = rFoot.GetFooterFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aLCnt = rFmtFoot.GetFooterFmt()->GetCntnt();
            if( !aLCnt.GetCntntIdx() )
            {
                const SwFrmFmt& rChgedFrmFmt = getConstFrmFmt(rChged, bLeft, bFirst);
                rDescFrmFmt.SetFmtAttr( rChgedFrmFmt.GetFooter() );
            }
            else if ((*aRCnt.GetCntntIdx() == *aLCnt.GetCntntIdx()) ||
                // The CntntIdx is _always_ different when called from
                // SwDocStyleSheet::SetItemSet, because it deep-copies the
                // PageDesc.  So check if it was previously shared.
                 ((bFirst) ? rDesc.IsFirstShared() : rDesc.IsFooterShared()))
            {
                SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(),
                        (bFirst) ? "First footer" : "Left footer",
                                                GetDfltFrmFmt() );
                ::lcl_DescSetAttr( *pRight, *pFmt, false );
                // The section to which the right footer attribute is pointing
                // is copied, and the Index to the StartNode is set to
                // the left footer attribute.
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwFooterStartNode );
                SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
                            *aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes()._Copy( aRange, aTmp, false );
                aTmp = *pSttNd;
                GetDocumentContentOperationsManager().CopyFlyInFlyImpl(aRange, 0, aTmp);

                pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ) );
                rDescFrmFmt.SetFmtAttr( SwFmtFooter( pFmt ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *const_cast<SwFrmFmt*>(rFmtFoot.GetFooterFmt()), false );
        }
    }
}

void SwDoc::ChgPageDesc( sal_uInt16 i, const SwPageDesc &rChged )
{
    OSL_ENSURE( i < maPageDescs.size(), "PageDescs is out of range." );

    SwPageDesc& rDesc = maPageDescs[i];
    SwRootFrm* pTmpRoot = getIDocumentLayoutAccess().GetCurrentLayout();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo *const pUndo(new SwUndoPageDesc(rDesc, rChged, this));
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // Mirror at first if needed.
    if ( rChged.GetUseOn() == nsUseOnPage::PD_MIRROR )
        const_cast<SwPageDesc&>(rChged).Mirror();
    else
    {
        // Or else transfer values from Master to Left
        ::lcl_DescSetAttr(rChged.GetMaster(),
                   const_cast<SwPageDesc&>(rChged).GetLeft());
    }
    ::lcl_DescSetAttr(rChged.GetMaster(),
                   const_cast<SwPageDesc&>(rChged).GetFirstMaster());
    ::lcl_DescSetAttr(rChged.GetLeft(),
                   const_cast<SwPageDesc&>(rChged).GetFirstLeft());

    // Take over NumType.
    if( rChged.GetNumType().GetNumberingType() != rDesc.GetNumType().GetNumberingType() )
    {
        rDesc.SetNumType( rChged.GetNumType() );
        // Notify page number fields that NumFormat has changed
        getIDocumentFieldsAccess().GetSysFldType( RES_PAGENUMBERFLD )->UpdateFlds();
        getIDocumentFieldsAccess().GetSysFldType( RES_REFPAGEGETFLD )->UpdateFlds();

        // If the numbering scheme has changed we could have QuoVadis/ErgoSum texts
        // that refer to a changed page, so we invalidate foot notes.
        SwFtnIdxs& rFtnIdxs = GetFtnIdxs();
        for( SwFtnIdxs::size_type nPos = 0; nPos < rFtnIdxs.size(); ++nPos )
        {
            SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
            const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
            pTxtFtn->SetNumber(rFtn.GetNumber(), rFtn.GetNumStr());
        }
    }

    // Take over orientation
    rDesc.SetLandscape( rChged.GetLandscape() );

    // #i46909# no undo if header or footer changed
    bool bHeaderFooterChanged = false;

    // Synch header.
    const SwFmtHeader &rHead = rChged.GetMaster().GetHeader();
    if (undoGuard.UndoWasEnabled())
    {
        // #i46909# no undo if header or footer changed
        // Did something change in the nodes?
        const SwFmtHeader &rOldHead = rDesc.GetMaster().GetHeader();
        bHeaderFooterChanged |=
            ( rHead.IsActive() != rOldHead.IsActive() ||
              rChged.IsHeaderShared() != rDesc.IsHeaderShared() ||
              rChged.IsFirstShared() != rDesc.IsFirstShared() );
    }
    rDesc.GetMaster().SetFmtAttr( rHead );
    CopyMasterHeader(rChged, rHead, rDesc, true, false); // Copy left header
    CopyMasterHeader(rChged, rHead, rDesc, false, true); // Copy first master
    CopyMasterHeader(rChged, rHead, rDesc, true, true);  // Copy first left
    rDesc.ChgHeaderShare( rChged.IsHeaderShared() );

    // Synch Footer.
    const SwFmtFooter &rFoot = rChged.GetMaster().GetFooter();
    if (undoGuard.UndoWasEnabled())
    {
        // #i46909# no undo if header or footer changed
        // Did something change in the Nodes?
        const SwFmtFooter &rOldFoot = rDesc.GetMaster().GetFooter();
        bHeaderFooterChanged |=
            ( rFoot.IsActive() != rOldFoot.IsActive() ||
              rChged.IsFooterShared() != rDesc.IsFooterShared() );
    }
    rDesc.GetMaster().SetFmtAttr( rFoot );
    CopyMasterFooter(rChged, rFoot, rDesc, true, false); // Copy left footer
    CopyMasterFooter(rChged, rFoot, rDesc, false, true); // Copy first master
    CopyMasterFooter(rChged, rFoot, rDesc, true, true);  // Copy first left
    rDesc.ChgFooterShare( rChged.IsFooterShared() );
    // there is just one first shared flag for both header and footer?
    rDesc.ChgFirstShare( rChged.IsFirstShared() );

    if ( rDesc.GetName() != rChged.GetName() )
        rDesc.SetName( rChged.GetName() );

    // A RegisterChange is triggered, if necessary
    rDesc.SetRegisterFmtColl( rChged.GetRegisterFmtColl() );

    // If UseOn or the Follow change, the paragraphs need to know about it.
    bool bUseOn  = false;
    bool bFollow = false;
    if (rDesc.GetUseOn() != rChged.GetUseOn())
    {
        rDesc.SetUseOn( rChged.GetUseOn() );
        bUseOn = true;
    }
    if (rDesc.GetFollow() != rChged.GetFollow())
    {
        if (rChged.GetFollow() == &rChged)
        {
            if (rDesc.GetFollow() != &rDesc)
            {
                rDesc.SetFollow( &rDesc );
                bFollow = true;
            }
        }
        else
        {
            rDesc.SetFollow( rChged.m_pFollow );
            bFollow = true;
        }
    }

    if ( (bUseOn || bFollow) && pTmpRoot)
        // Inform layout!
    {
        std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllCheckPageDescs));
    }

    // Take over the page attributes.
    ::lcl_DescSetAttr( rChged.GetMaster(), rDesc.GetMaster() );
    ::lcl_DescSetAttr( rChged.GetLeft(), rDesc.GetLeft() );
    ::lcl_DescSetAttr( rChged.GetFirstMaster(), rDesc.GetFirstMaster() );
    ::lcl_DescSetAttr( rChged.GetFirstLeft(), rDesc.GetFirstLeft() );

    // If the FootnoteInfo changes, the pages are triggered.
    if( !(rDesc.GetFtnInfo() == rChged.GetFtnInfo()) )
    {
        rDesc.SetFtnInfo( rChged.GetFtnInfo() );
        sw::PageFootnoteHint aHint;
        rDesc.GetMaster().CallSwClientNotify(aHint);
        rDesc.GetLeft().CallSwClientNotify(aHint);
        rDesc.GetFirstMaster().CallSwClientNotify(aHint);
        rDesc.GetFirstLeft().CallSwClientNotify(aHint);
    }
    getIDocumentState().SetModified();

    // #i46909# no undo if header or footer changed
    if( bHeaderFooterChanged )
    {
        GetIDocumentUndoRedo().DelAllUndoObj();
    }

    SfxBindings* pBindings =
        ( GetDocShell() && GetDocShell()->GetDispatcher() ) ? GetDocShell()->GetDispatcher()->GetBindings() : 0;
    if ( pBindings )
    {
        pBindings->Invalidate( SID_ATTR_PAGE_COLUMN );
        pBindings->Invalidate( SID_ATTR_PAGE );
        pBindings->Invalidate( SID_ATTR_PAGE_SIZE );
        pBindings->Invalidate( SID_ATTR_PAGE_ULSPACE );
        pBindings->Invalidate( SID_ATTR_PAGE_LRSPACE );
    }

    //h/f of first-left page must not be unique but same as first master or left
    assert((rDesc.IsFirstShared())
        ? rDesc.GetFirstLeft().GetHeader().GetHeaderFmt() == rDesc.GetLeft().GetHeader().GetHeaderFmt()
        : rDesc.GetFirstLeft().GetHeader().GetHeaderFmt() == rDesc.GetFirstMaster().GetHeader().GetHeaderFmt());
    assert((rDesc.IsFirstShared())
        ? rDesc.GetFirstLeft().GetFooter().GetFooterFmt() == rDesc.GetLeft().GetFooter().GetFooterFmt()
        : rDesc.GetFirstLeft().GetFooter().GetFooterFmt() == rDesc.GetFirstMaster().GetFooter().GetFooterFmt());
}

/// All descriptors whose Follow point to the to-be-deleted have to be adapted.
// #i7983#
void SwDoc::PreDelPageDesc(SwPageDesc * pDel)
{
    if (0 == pDel)
        return;

    // mba: test iteration as clients are removed while iteration
    SwPageDescHint aHint( &maPageDescs[0] );
    pDel->CallSwClientNotify( aHint );

    bool bHasLayout = getIDocumentLayoutAccess().HasLayout();
    if ( mpFtnInfo->DependsOn( pDel ) )
    {
        mpFtnInfo->ChgPageDesc( &maPageDescs[0] );
        if ( bHasLayout )
        {
            std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::CheckFtnPageDescs), false));
        }
    }
    else if ( mpEndNoteInfo->DependsOn( pDel ) )
    {
        mpEndNoteInfo->ChgPageDesc( &maPageDescs[0] );
        if ( bHasLayout )
        {
            std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::CheckFtnPageDescs), true));
        }
    }

    for ( SwPageDescs::size_type j = 0; j < maPageDescs.size(); ++j )
    {
        if ( maPageDescs[j].GetFollow() == pDel )
        {
            maPageDescs[j].SetFollow( 0 );
            if( bHasLayout )
            {
                std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
                std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllCheckPageDescs));
            }
        }
    }
}

void SwDoc::BroadcastStyleOperation(const OUString& rName, SfxStyleFamily eFamily,
                                    sal_uInt16 nOp)
{
    if (mpDocShell)
    {
        SfxStyleSheetBasePool * pPool = mpDocShell->GetStyleSheetPool();

        if (pPool)
        {
            pPool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
            SfxStyleSheetBase * pBase = pPool->Find(rName);

            if (pBase != NULL)
                pPool->Broadcast(SfxStyleSheetHint( nOp, *pBase ));
        }
    }
}

void SwDoc::DelPageDesc( sal_uInt16 i, bool bBroadcast )
{
    OSL_ENSURE( i < maPageDescs.size(), "PageDescs is out of range." );
    OSL_ENSURE( i != 0, "You cannot delete the default Pagedesc.");
    if ( i == 0 )
        return;

    SwPageDesc &rDel = maPageDescs[i];

    if (bBroadcast)
        BroadcastStyleOperation(rDel.GetName(), SFX_STYLE_FAMILY_PAGE,
                                SfxStyleSheetHintId::ERASED);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo *const pUndo(new SwUndoPageDescDelete(rDel, this));
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    PreDelPageDesc(&rDel); // #i7983#

    maPageDescs.erase( maPageDescs.begin() + i );
    getIDocumentState().SetModified();
}

SwPageDesc* SwDoc::MakePageDesc(const OUString &rName, const SwPageDesc *pCpy,
                            bool bRegardLanguage, bool bBroadcast)
{
    SwPageDesc *pNew;
    if( pCpy )
    {
        pNew = new SwPageDesc( *pCpy );
        pNew->SetName( rName );
        if( rName != pCpy->GetName() )
        {
            pNew->SetPoolFmtId( USHRT_MAX );
            pNew->SetPoolHelpId( USHRT_MAX );
            pNew->SetPoolHlpFileId( UCHAR_MAX );
        }
    }
    else
    {
        pNew = new SwPageDesc( rName, GetDfltFrmFmt(), this );
        // Set the default page format.
        lcl_DefaultPageFmt( USHRT_MAX, pNew->GetMaster(), pNew->GetLeft(), pNew->GetFirstMaster(), pNew->GetFirstLeft() );

        SvxFrameDirection aFrameDirection = bRegardLanguage ?
            GetDefaultFrameDirection(GetAppLanguage())
            : FRMDIR_HORI_LEFT_TOP;

        pNew->GetMaster().SetFmtAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
        pNew->GetLeft().SetFmtAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
        pNew->GetFirstMaster().SetFmtAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
        pNew->GetFirstLeft().SetFmtAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
    }
    maPageDescs.push_back( pNew );

    if (bBroadcast)
        BroadcastStyleOperation(rName, SFX_STYLE_FAMILY_PAGE,
                                SfxStyleSheetHintId::CREATED);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(new SwUndoPageDescCreate(pNew, this));
    }

    getIDocumentState().SetModified();
    return pNew;
}

// We collect the GlobalNames of the servers at runtime, who don't want to be notified
// about printer changes. Thereby saving loading a lot of objects (luckily all foreign
// objects are mapped to one ID).
// Initialisation and deinitialisation can be found in init.cxx
extern std::vector<SvGlobalName*> *pGlobalOLEExcludeList;

void SwDoc::PrtOLENotify( bool bAll )
{
    SwFEShell *pShell = nullptr;
    {
        SwViewShell *pSh = getIDocumentLayoutAccess().GetCurrentViewShell();
        if ( pSh )
        {
            for(SwViewShell& rShell : pSh->GetRingContainer())
            {
                if(rShell.ISA(SwFEShell))
                {
                    pShell = static_cast<SwFEShell*>(&rShell);
                    break;
                }
            }
        }
    }
    if ( !pShell )
    {
        // This doesn't make sense without a Shell and thus without a client, because
        // the communication about size changes is implemented by these components.
        // Because we don't have a Shell we remember this unfortunate situation
        // in the document,
        // which is made up for later on when creating the first Shell.
        mbOLEPrtNotifyPending = true;
        if ( bAll )
            mbAllOLENotify = true;
    }
    else
    {
        if ( mbAllOLENotify )
            bAll = true;

        mbOLEPrtNotifyPending = mbAllOLENotify = false;

        SwOLENodes *pNodes = SwCntntNode::CreateOLENodesArray( *GetDfltGrfFmtColl(), !bAll );
        if ( pNodes )
        {
            ::StartProgress( STR_STATSTR_SWGPRTOLENOTIFY,
                             0, pNodes->size(), GetDocShell());
            getIDocumentLayoutAccess().GetCurrentLayout()->StartAllAction();

            for( SwOLENodes::size_type i = 0; i < pNodes->size(); ++i )
            {
                ::SetProgressState( i, GetDocShell() );

                SwOLENode* pOLENd = (*pNodes)[i];
                pOLENd->SetOLESizeInvalid( false );

                // At first load the Infos and see if it's not already in the exclude list.
                SvGlobalName aName;

                svt::EmbeddedObjectRef& xObj = pOLENd->GetOLEObj().GetObject();
                if ( xObj.is() )
                    aName = SvGlobalName( xObj->getClassID() );
                else  // Not yet loaded
                {
                        // TODO/LATER: retrieve ClassID of an unloaded object
                        // aName = ????
                }

                bool bFound = false;
                for ( std::vector<SvGlobalName*>::size_type j = 0;
                      j < pGlobalOLEExcludeList->size() && !bFound;
                      ++j )
                {
                    bFound = *(*pGlobalOLEExcludeList)[j] == aName;
                }
                if ( bFound )
                    continue;

                // We don't know it, so the object has to be loaded.
                // If it doesn't want to be informed
                if ( xObj.is() )
                {
                        pGlobalOLEExcludeList->push_back( new SvGlobalName( aName ) );
                }
            }
            delete pNodes;
            getIDocumentLayoutAccess().GetCurrentLayout()->EndAllAction();
            ::EndProgress( GetDocShell() );
        }
    }
}

IMPL_LINK( SwDoc, DoUpdateModifiedOLE, Timer *, )
{
    SwFEShell* pSh = static_cast<SwFEShell*>(GetEditShell());
    if( pSh )
    {
        mbOLEPrtNotifyPending = mbAllOLENotify = false;

        SwOLENodes *pNodes = SwCntntNode::CreateOLENodesArray( *GetDfltGrfFmtColl(), true );
        if( pNodes )
        {
            ::StartProgress( STR_STATSTR_SWGPRTOLENOTIFY,
                             0, pNodes->size(), GetDocShell());
            getIDocumentLayoutAccess().GetCurrentLayout()->StartAllAction();
            SwMsgPoolItem aMsgHint( RES_UPDATE_ATTR );

            for( SwOLENodes::size_type i = 0; i < pNodes->size(); ++i )
            {
                ::SetProgressState( i, GetDocShell() );

                SwOLENode* pOLENd = (*pNodes)[i];
                pOLENd->SetOLESizeInvalid( false );

                // We don't know it, so the object has to be loaded.
                // If it doesn't want to be informed
                if( pOLENd->GetOLEObj().GetOleRef().is() ) // Broken?
                {
                    pOLENd->ModifyNotification( &aMsgHint, &aMsgHint );
                }
            }
            getIDocumentLayoutAccess().GetCurrentLayout()->EndAllAction();
            ::EndProgress( GetDocShell() );
            delete pNodes;
        }
    }
    return 0;
}

struct CompareSwPageDescName {
    CompareSwPageDescName(const OUString &rName) : mName(rName) {}
    bool operator () (const SwPageDesc& other) const { return other.GetName() == mName; }
    const OUString &mName;
};

template <class UnaryPredicate>
static SwPageDesc* lcl_FindPageDesc( SwPageDescs *pPageDescs,
                                     sal_uInt16 *pPos, UnaryPredicate pred )
{
    SwPageDescs::iterator it = std::find_if(
        pPageDescs->begin(), pPageDescs->end(), pred);
    SwPageDesc* res = NULL;
    if( it != pPageDescs->end() )
    {
        res = const_cast <SwPageDesc *>( &( *it ) );;
        if( pPos )
            *pPos = std::distance( pPageDescs->begin(), it );
    }
    else if( pPos )
        *pPos = USHRT_MAX;
    return res;
}

SwPageDesc* SwDoc::FindPageDesc( const OUString & rName, sal_uInt16* pPos )
{
    return lcl_FindPageDesc<CompareSwPageDescName>(
        &maPageDescs, pPos, CompareSwPageDescName(rName) );
}

SwPageDesc* SwDoc::FindPageDesc( const OUString & rName, sal_uInt16* pPos ) const
{
    return lcl_FindPageDesc<CompareSwPageDescName>(
        const_cast <SwPageDescs *>( &maPageDescs ), pPos,
        CompareSwPageDescName(rName) );
}

struct CompareSwPageDescToPtr {
    CompareSwPageDescToPtr(const SwPageDesc* ptr) : mPtr(ptr) {}
    bool operator () (const SwPageDesc& other) const { return &other == mPtr; }
    const SwPageDesc *mPtr;
};

bool SwDoc::ContainsPageDesc( const SwPageDesc *pDesc, sal_uInt16* pPos )
{
    if (pDesc == NULL)
        return false;
    SwPageDesc *res = lcl_FindPageDesc<CompareSwPageDescToPtr>(
        const_cast <SwPageDescs *>( &maPageDescs ), pPos,
        CompareSwPageDescToPtr(pDesc) );
    return res != NULL;
}

void SwDoc::DelPageDesc( const OUString & rName, bool bBroadcast )
{
    sal_uInt16 nI;

    if (FindPageDesc(rName, &nI))
        DelPageDesc(nI, bBroadcast);
}

void SwDoc::ChgPageDesc( const OUString & rName, const SwPageDesc & rDesc)
{
    sal_uInt16 nI;

    if (FindPageDesc(rName, &nI))
        ChgPageDesc(nI, rDesc);
}

/*
 * The HTML import cannot resist changing the page descriptions, I don't
 * know why. This function is meant to check the page descriptors for invalid
 * values.
 */
void SwDoc::CheckDefaultPageFmt()
{
    for ( sal_uInt16 i = 0; i < GetPageDescCnt(); ++i )
    {
        SwPageDesc& rDesc = GetPageDesc( i );

        SwFrmFmt& rMaster = rDesc.GetMaster();
        SwFrmFmt& rLeft   = rDesc.GetLeft();

        const SwFmtFrmSize& rMasterSize  = rMaster.GetFrmSize();
        const SwFmtFrmSize& rLeftSize    = rLeft.GetFrmSize();

        const bool bSetSize = LONG_MAX == rMasterSize.GetWidth() ||
                              LONG_MAX == rMasterSize.GetHeight() ||
                              LONG_MAX == rLeftSize.GetWidth() ||
                              LONG_MAX == rLeftSize.GetHeight();

        if ( bSetSize )
            lcl_DefaultPageFmt( rDesc.GetPoolFmtId(), rDesc.GetMaster(), rDesc.GetLeft(), rDesc.GetFirstMaster(), rDesc.GetFirstLeft() );
    }
}

void SwDoc::SetDefaultPageMode(bool bSquaredPageMode)
{
    if( !bSquaredPageMode == !IsSquaredPageMode() )
        return;

    const SwTextGridItem& rGrid =
                    static_cast<const SwTextGridItem&>(GetDefault( RES_TEXTGRID ));
    SwTextGridItem aNewGrid = rGrid;
    aNewGrid.SetSquaredMode(bSquaredPageMode);
    aNewGrid.Init();
    SetDefault(aNewGrid);

    for ( sal_uInt16 i = 0; i < GetPageDescCnt(); ++i )
    {
        SwPageDesc& rDesc = GetPageDesc( i );

        SwFrmFmt& rMaster = rDesc.GetMaster();
        SwFrmFmt& rLeft = rDesc.GetLeft();

        SwTextGridItem aGrid(static_cast<const SwTextGridItem&>(rMaster.GetFmtAttr(RES_TEXTGRID)));
        aGrid.SwitchPaperMode( bSquaredPageMode );
        rMaster.SetFmtAttr(aGrid);
        rLeft.SetFmtAttr(aGrid);
    }
}

bool SwDoc::IsSquaredPageMode() const
{
    const SwTextGridItem& rGrid =
                        static_cast<const SwTextGridItem&>(GetDefault( RES_TEXTGRID ));
    return rGrid.IsSquaredMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
