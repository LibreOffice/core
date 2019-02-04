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
#include <init.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/frmdiritem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include <tools/globname.hxx>
#include <sal/log.hxx>
#include <unotools/localedatawrapper.hxx>
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
#include <strings.hrc>
#include <hints.hxx>
#include <SwUndoPageDesc.hxx>
#include <pagedeschint.hxx>
#include <tgrditem.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocale.hxx>
#include <svx/swframetypes.hxx>
#include <svx/svxids.hrc>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

using namespace com::sun::star;

static void lcl_DefaultPageFormat( sal_uInt16 nPoolFormatId,
                                SwFrameFormat &rFormat1,
                                SwFrameFormat &rFormat2,
                                SwFrameFormat &rFormat3,
                                SwFrameFormat &rFormat4)
{
    // --> #i41075# Printer on demand
    // This function does not require a printer anymore.
    // The default page size is obtained from the application
    //locale

    SwFormatFrameSize aFrameSize( ATT_FIX_SIZE );
    const Size aPhysSize = SvxPaperInfo::GetDefaultPaperSize();
    aFrameSize.SetSize( aPhysSize );

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
    if( RES_POOLPAGE_HTML == nPoolFormatId )
    {
        nMinRight = nMinTop = nMinBottom = GetMetricVal( CM_1 );
        nMinLeft = nMinRight * 2;
    }
    else if (!utl::ConfigManager::IsFuzzing() && MeasurementSystem::Metric == SvtSysLocale().GetLocaleData().getMeasurementSystemEnum() )
    {
        nMinTop = nMinBottom = nMinLeft = nMinRight = 1134; // 2 centimeters
    }
    else
    {
        nMinTop = nMinBottom = 1440;    // as in MS Word: 1 Inch
        nMinLeft = nMinRight = 1800;    //                1,25 Inch
    }

    // set margins
    SvxLRSpaceItem aLR( RES_LR_SPACE );
    SvxULSpaceItem aUL( RES_UL_SPACE );

    aUL.SetUpper( static_cast<sal_uInt16>(nMinTop) );
    aUL.SetLower( static_cast<sal_uInt16>(nMinBottom) );
    aLR.SetRight( nMinRight );
    aLR.SetLeft( nMinLeft );

    rFormat1.SetFormatAttr( aFrameSize );
    rFormat1.SetFormatAttr( aLR );
    rFormat1.SetFormatAttr( aUL );

    rFormat2.SetFormatAttr( aFrameSize );
    rFormat2.SetFormatAttr( aLR );
    rFormat2.SetFormatAttr( aUL );

    rFormat3.SetFormatAttr( aFrameSize );
    rFormat3.SetFormatAttr( aLR );
    rFormat3.SetFormatAttr( aUL );

    rFormat4.SetFormatAttr( aFrameSize );
    rFormat4.SetFormatAttr( aLR );
    rFormat4.SetFormatAttr( aUL );
}

static void lcl_DescSetAttr( const SwFrameFormat &rSource, SwFrameFormat &rDest,
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

        // take over DrawingLayer FillStyles
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
                    // take out SvxBrushItem; it's the result of the fallback
                    // at SwFormat::GetItemState and not really in state SfxItemState::SET
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
                    rDest.SetFormatAttr(*pItem);
                }
                else
                {
                    rDest.ResetFormatAttr(nId);
                }
            }
        }
    }

    // Transmit pool and help IDs too
    rDest.SetPoolFormatId( rSource.GetPoolFormatId() );
    rDest.SetPoolHelpId( rSource.GetPoolHelpId() );
    rDest.SetPoolHlpFileId( rSource.GetPoolHlpFileId() );
}

namespace
{
    SwFrameFormat& getFrameFormat(SwPageDesc &rDesc, bool bLeft, bool bFirst)
    {
        if (bFirst)
        {
            if (bLeft)
                return rDesc.GetFirstLeft();
            return rDesc.GetFirstMaster();
        }
        return rDesc.GetLeft();
    }

    const SwFrameFormat& getConstFrameFormat(const SwPageDesc &rDesc, bool bLeft, bool bFirst)
    {
        return getFrameFormat(const_cast<SwPageDesc&>(rDesc), bLeft, bFirst);
    }
}

void SwDoc::CopyMasterHeader(const SwPageDesc &rChged, const SwFormatHeader &rHead, SwPageDesc &rDesc, bool bLeft, bool bFirst)
{
    assert(bLeft || bFirst);
    SwFrameFormat& rDescFrameFormat = getFrameFormat(rDesc, bLeft, bFirst);
    if (bFirst && bLeft)
    {
        // special case: always shared with something
        rDescFrameFormat.SetFormatAttr( rChged.IsFirstShared()
                ? rDesc.GetLeft().GetHeader()
                : rDesc.GetFirstMaster().GetHeader());
    }
    else if ((bFirst ? rChged.IsFirstShared() : rChged.IsHeaderShared())
         || !rHead.IsActive())
    {
        // Left or first shares the header with the Master.
        rDescFrameFormat.SetFormatAttr( rDesc.GetMaster().GetHeader() );
    }
    else if ( rHead.IsActive() )
    {   // Left or first gets its own header if the Format doesn't already have one.
        // If it already has one and it points to the same Section as the
        // Right one, it needs to get an own Header.
        // The content is evidently copied.
        const SwFormatHeader &rFormatHead = rDescFrameFormat.GetHeader();
        if ( !rFormatHead.IsActive() )
        {
            SwFormatHeader aHead( getIDocumentLayoutAccess().MakeLayoutFormat( RndStdIds::HEADERL, nullptr ) );
            rDescFrameFormat.SetFormatAttr( aHead );
            // take over additional attributes (margins, borders ...)
            ::lcl_DescSetAttr( *rHead.GetHeaderFormat(), *aHead.GetHeaderFormat(), false);
        }
        else
        {
            const SwFrameFormat *pRight = rHead.GetHeaderFormat();
            const SwFormatContent &aRCnt = pRight->GetContent();
            const SwFormatContent &aCnt = rFormatHead.GetHeaderFormat()->GetContent();

            if (!aCnt.GetContentIdx())
            {
                const SwFrameFormat& rChgedFrameFormat = getConstFrameFormat(rChged, bLeft, bFirst);
                rDescFrameFormat.SetFormatAttr( rChgedFrameFormat.GetHeader() );
            }
            else if ((*aRCnt.GetContentIdx() == *aCnt.GetContentIdx()) ||
                // The ContentIdx is _always_ different when called from
                // SwDocStyleSheet::SetItemSet, because it deep-copies the
                // PageDesc.  So check if it was previously shared.
                 (bFirst ? rDesc.IsFirstShared() : rDesc.IsHeaderShared()))
            {
                SwFrameFormat *pFormat = new SwFrameFormat( GetAttrPool(),
                        bFirst ? "First header" : "Left header",
                                                GetDfltFrameFormat() );
                ::lcl_DescSetAttr( *pRight, *pFormat, false );
                // The section which the right header attribute is pointing
                // is copied, and the Index to the StartNode is set to
                // the left or first header attribute.
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = SwNodes::MakeEmptySection( aTmp, SwHeaderStartNode );
                SwNodeRange aRange( aRCnt.GetContentIdx()->GetNode(), 0,
                            *aRCnt.GetContentIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes().Copy_( aRange, aTmp, false );
                aTmp = *pSttNd;
                GetDocumentContentOperationsManager().CopyFlyInFlyImpl(aRange, 0, aTmp);

                pFormat->SetFormatAttr( SwFormatContent( pSttNd ) );
                rDescFrameFormat.SetFormatAttr( SwFormatHeader( pFormat ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *const_cast<SwFrameFormat*>(rFormatHead.GetHeaderFormat()), false );
        }
    }
}

void SwDoc::CopyMasterFooter(const SwPageDesc &rChged, const SwFormatFooter &rFoot, SwPageDesc &rDesc, bool bLeft, bool bFirst)
{
    assert(bLeft || bFirst);
    SwFrameFormat& rDescFrameFormat = getFrameFormat(rDesc, bLeft, bFirst);
    if (bFirst && bLeft)
    {
        // special case: always shared with something
        rDescFrameFormat.SetFormatAttr( rChged.IsFirstShared()
                ? rDesc.GetLeft().GetFooter()
                : rDesc.GetFirstMaster().GetFooter());
    }
    else if ((bFirst ? rChged.IsFirstShared() : rChged.IsFooterShared())
        || !rFoot.IsActive())
    {
        // Left or first shares the Header with the Master.
        rDescFrameFormat.SetFormatAttr( rDesc.GetMaster().GetFooter() );
    }
    else if ( rFoot.IsActive() )
    {   // Left or first gets its own Footer if the Format does not already have one.
        // If the Format already has a Footer and it points to the same section as the Right one,
        // it needs to get an own one.
        // The content is evidently copied.
        const SwFormatFooter &rFormatFoot = rDescFrameFormat.GetFooter();
        if ( !rFormatFoot.IsActive() )
        {
            SwFormatFooter aFoot( getIDocumentLayoutAccess().MakeLayoutFormat( RndStdIds::FOOTER, nullptr ) );
            rDescFrameFormat.SetFormatAttr( aFoot );
            // Take over additional attributes (margins, borders ...).
            ::lcl_DescSetAttr( *rFoot.GetFooterFormat(), *aFoot.GetFooterFormat(), false);
        }
        else
        {
            const SwFrameFormat *pRight = rFoot.GetFooterFormat();
            const SwFormatContent &aRCnt = pRight->GetContent();
            const SwFormatContent &aLCnt = rFormatFoot.GetFooterFormat()->GetContent();
            if( !aLCnt.GetContentIdx() )
            {
                const SwFrameFormat& rChgedFrameFormat = getConstFrameFormat(rChged, bLeft, bFirst);
                rDescFrameFormat.SetFormatAttr( rChgedFrameFormat.GetFooter() );
            }
            else if ((*aRCnt.GetContentIdx() == *aLCnt.GetContentIdx()) ||
                // The ContentIdx is _always_ different when called from
                // SwDocStyleSheet::SetItemSet, because it deep-copies the
                // PageDesc.  So check if it was previously shared.
                 (bFirst ? rDesc.IsFirstShared() : rDesc.IsFooterShared()))
            {
                SwFrameFormat *pFormat = new SwFrameFormat( GetAttrPool(),
                        bFirst ? "First footer" : "Left footer",
                                                GetDfltFrameFormat() );
                ::lcl_DescSetAttr( *pRight, *pFormat, false );
                // The section to which the right footer attribute is pointing
                // is copied, and the Index to the StartNode is set to
                // the left footer attribute.
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = SwNodes::MakeEmptySection( aTmp, SwFooterStartNode );
                SwNodeRange aRange( aRCnt.GetContentIdx()->GetNode(), 0,
                            *aRCnt.GetContentIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes().Copy_( aRange, aTmp, false );
                aTmp = *pSttNd;
                GetDocumentContentOperationsManager().CopyFlyInFlyImpl(aRange, 0, aTmp);

                pFormat->SetFormatAttr( SwFormatContent( pSttNd ) );
                rDescFrameFormat.SetFormatAttr( SwFormatFooter( pFormat ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *const_cast<SwFrameFormat*>(rFormatFoot.GetFooterFormat()), false );
        }
    }
}

void SwDoc::ChgPageDesc( size_t i, const SwPageDesc &rChged )
{
    OSL_ENSURE(i < m_PageDescs.size(), "PageDescs is out of range.");

    SwPageDesc& rDesc = *m_PageDescs[i];
    SwRootFrame* pTmpRoot = getIDocumentLayoutAccess().GetCurrentLayout();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
                std::make_unique<SwUndoPageDesc>(rDesc, rChged, this));
    }
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // Mirror at first if needed.
    if ( rChged.GetUseOn() == UseOnPage::Mirror )
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
        getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::PageNumber )->UpdateFields();
        getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::RefPageGet )->UpdateFields();

        // If the numbering scheme has changed we could have QuoVadis/ErgoSum texts
        // that refer to a changed page, so we invalidate foot notes.
        SwFootnoteIdxs& rFootnoteIdxs = GetFootnoteIdxs();
        for( SwFootnoteIdxs::size_type nPos = 0; nPos < rFootnoteIdxs.size(); ++nPos )
        {
            SwTextFootnote *pTextFootnote = rFootnoteIdxs[ nPos ];
            const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
            pTextFootnote->SetNumber(rFootnote.GetNumber(), rFootnote.GetNumberRLHidden(), rFootnote.GetNumStr());
        }
    }

    // Take over orientation
    rDesc.SetLandscape( rChged.GetLandscape() );

    // #i46909# no undo if header or footer changed
    bool bHeaderFooterChanged = false;

    // Synch header.
    const SwFormatHeader &rHead = rChged.GetMaster().GetHeader();
    if (undoGuard.UndoWasEnabled())
    {
        // #i46909# no undo if header or footer changed
        // Did something change in the nodes?
        const SwFormatHeader &rOldHead = rDesc.GetMaster().GetHeader();
        bHeaderFooterChanged |=
            ( rHead.IsActive() != rOldHead.IsActive() ||
              rChged.IsHeaderShared() != rDesc.IsHeaderShared() ||
              rChged.IsFirstShared() != rDesc.IsFirstShared() );
    }
    rDesc.GetMaster().SetFormatAttr( rHead );
    CopyMasterHeader(rChged, rHead, rDesc, true, false); // Copy left header
    CopyMasterHeader(rChged, rHead, rDesc, false, true); // Copy first master
    CopyMasterHeader(rChged, rHead, rDesc, true, true);  // Copy first left
    rDesc.ChgHeaderShare( rChged.IsHeaderShared() );

    // Synch Footer.
    const SwFormatFooter &rFoot = rChged.GetMaster().GetFooter();
    if (undoGuard.UndoWasEnabled())
    {
        // #i46909# no undo if header or footer changed
        // Did something change in the Nodes?
        const SwFormatFooter &rOldFoot = rDesc.GetMaster().GetFooter();
        bHeaderFooterChanged |=
            ( rFoot.IsActive() != rOldFoot.IsActive() ||
              rChged.IsFooterShared() != rDesc.IsFooterShared() );
    }
    rDesc.GetMaster().SetFormatAttr( rFoot );
    CopyMasterFooter(rChged, rFoot, rDesc, true, false); // Copy left footer
    CopyMasterFooter(rChged, rFoot, rDesc, false, true); // Copy first master
    CopyMasterFooter(rChged, rFoot, rDesc, true, true);  // Copy first left
    rDesc.ChgFooterShare( rChged.IsFooterShared() );
    // there is just one first shared flag for both header and footer?
    rDesc.ChgFirstShare( rChged.IsFirstShared() );

    if ( rDesc.GetName() != rChged.GetName() )
        rDesc.SetName( rChged.GetName() );

    // A RegisterChange is triggered, if necessary
    rDesc.SetRegisterFormatColl( rChged.GetRegisterFormatColl() );

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
        for( auto aLayout : GetAllLayouts() )
            aLayout->AllCheckPageDescs();
    }

    // Take over the page attributes.
    ::lcl_DescSetAttr( rChged.GetMaster(), rDesc.GetMaster() );
    ::lcl_DescSetAttr( rChged.GetLeft(), rDesc.GetLeft() );
    ::lcl_DescSetAttr( rChged.GetFirstMaster(), rDesc.GetFirstMaster() );
    ::lcl_DescSetAttr( rChged.GetFirstLeft(), rDesc.GetFirstLeft() );

    // If the FootnoteInfo changes, the pages are triggered.
    if( !(rDesc.GetFootnoteInfo() == rChged.GetFootnoteInfo()) )
    {
        rDesc.SetFootnoteInfo( rChged.GetFootnoteInfo() );
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
        ( GetDocShell() && GetDocShell()->GetDispatcher() ) ? GetDocShell()->GetDispatcher()->GetBindings() : nullptr;
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
        ? rDesc.GetFirstLeft().GetHeader().GetHeaderFormat() == rDesc.GetLeft().GetHeader().GetHeaderFormat()
        : rDesc.GetFirstLeft().GetHeader().GetHeaderFormat() == rDesc.GetFirstMaster().GetHeader().GetHeaderFormat());
    assert((rDesc.IsFirstShared())
        ? rDesc.GetFirstLeft().GetFooter().GetFooterFormat() == rDesc.GetLeft().GetFooter().GetFooterFormat()
        : rDesc.GetFirstLeft().GetFooter().GetFooterFormat() == rDesc.GetFirstMaster().GetFooter().GetFooterFormat());
}

/// All descriptors whose Follow point to the to-be-deleted have to be adapted.
// #i7983#
void SwDoc::PreDelPageDesc(SwPageDesc const * pDel)
{
    if (nullptr == pDel)
        return;

    // mba: test iteration as clients are removed while iteration
    SwPageDescHint aHint( m_PageDescs[0] );
    pDel->CallSwClientNotify( aHint );

    bool bHasLayout = getIDocumentLayoutAccess().HasLayout();
    if ( mpFootnoteInfo->DependsOn( pDel ) )
    {
        mpFootnoteInfo->ChgPageDesc( m_PageDescs[0] );
        if ( bHasLayout )
        {
            for( auto aLayout : GetAllLayouts() )
                aLayout->CheckFootnotePageDescs(false);
        }
    }
    else if ( mpEndNoteInfo->DependsOn( pDel ) )
    {
        mpEndNoteInfo->ChgPageDesc( m_PageDescs[0] );
        if ( bHasLayout )
        {
            for( auto aLayout : GetAllLayouts() )
                aLayout->CheckFootnotePageDescs(true);
        }
    }

    for (SwPageDesc* pPageDesc : m_PageDescs)
    {
        if (pPageDesc->GetFollow() == pDel)
        {
            pPageDesc->SetFollow(nullptr);
            if( bHasLayout )
            {
                for( auto aLayout : GetAllLayouts() )
                    aLayout->AllCheckPageDescs();
            }
        }
    }
}

void SwDoc::BroadcastStyleOperation(const OUString& rName, SfxStyleFamily eFamily,
                                    SfxHintId nOp)
{
    if (mpDocShell)
    {
        SfxStyleSheetBasePool * pPool = mpDocShell->GetStyleSheetPool();

        if (pPool)
        {
            pPool->SetSearchMask(eFamily);
            SfxStyleSheetBase * pBase = pPool->Find(rName);

            if (pBase != nullptr)
                pPool->Broadcast(SfxStyleSheetHint( nOp, *pBase ));
        }
    }
}

void SwDoc::DelPageDesc( size_t i, bool bBroadcast )
{
    OSL_ENSURE(i < m_PageDescs.size(), "PageDescs is out of range.");
    OSL_ENSURE( i != 0, "You cannot delete the default Pagedesc.");
    if ( i == 0 )
        return;

    SwPageDesc &rDel = *m_PageDescs[i];

    if (bBroadcast)
        BroadcastStyleOperation(rDel.GetName(), SfxStyleFamily::Page,
                                SfxHintId::StyleSheetErased);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoPageDescDelete>(rDel, this));
    }

    PreDelPageDesc(&rDel); // #i7983#

    m_PageDescs.erase(m_PageDescs.begin() + i);
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
            pNew->SetPoolFormatId( USHRT_MAX );
            pNew->SetPoolHelpId( USHRT_MAX );
            pNew->SetPoolHlpFileId( UCHAR_MAX );
        }
    }
    else
    {
        pNew = new SwPageDesc( rName, GetDfltFrameFormat(), this );
        // Set the default page format.
        lcl_DefaultPageFormat( USHRT_MAX, pNew->GetMaster(), pNew->GetLeft(), pNew->GetFirstMaster(), pNew->GetFirstLeft() );

        SvxFrameDirection aFrameDirection = bRegardLanguage ?
            GetDefaultFrameDirection(GetAppLanguage())
            : SvxFrameDirection::Horizontal_LR_TB;

        pNew->GetMaster().SetFormatAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
        pNew->GetLeft().SetFormatAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
        pNew->GetFirstMaster().SetFormatAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
        pNew->GetFirstLeft().SetFormatAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
    }

    std::pair<SwPageDescs::const_iterator, bool> res = m_PageDescs.push_back( pNew );
    SAL_WARN_IF(!res.second, "sw", "MakePageDesc called with existing name" );

    if (bBroadcast)
        BroadcastStyleOperation(rName, SfxStyleFamily::Page,
                                SfxHintId::StyleSheetCreated);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(std::make_unique<SwUndoPageDescCreate>(pNew, this));
    }

    getIDocumentState().SetModified();
    return pNew;
}

void SwDoc::PrtOLENotify( bool bAll )
{
    SwFEShell *pShell = nullptr;
    {
        SwViewShell *pSh = getIDocumentLayoutAccess().GetCurrentViewShell();
        if ( pSh )
        {
            for(SwViewShell& rShell : pSh->GetRingContainer())
            {
                if(dynamic_cast<const SwFEShell*>( &rShell) !=  nullptr)
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

        std::unique_ptr<SwOLENodes> pNodes = SwContentNode::CreateOLENodesArray( *GetDfltGrfFormatColl(), !bAll );
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
                for ( std::vector<SvGlobalName>::size_type j = 0;
                      j < pGlobalOLEExcludeList->size() && !bFound;
                      ++j )
                {
                    bFound = (*pGlobalOLEExcludeList)[j] == aName;
                }
                if ( bFound )
                    continue;

                // We don't know it, so the object has to be loaded.
                // If it doesn't want to be informed
                if ( xObj.is() )
                {
                    pGlobalOLEExcludeList->push_back( aName );
                }
            }
            pNodes.reset();
            getIDocumentLayoutAccess().GetCurrentLayout()->EndAllAction();
            ::EndProgress( GetDocShell() );
        }
    }
}

IMPL_LINK_NOARG( SwDoc, DoUpdateModifiedOLE, Timer *, void )
{
    SwFEShell* pSh = static_cast<SwFEShell*>(GetEditShell());
    if( pSh )
    {
        mbOLEPrtNotifyPending = mbAllOLENotify = false;

        std::unique_ptr<SwOLENodes> pNodes = SwContentNode::CreateOLENodesArray( *GetDfltGrfFormatColl(), true );
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
        }
    }
}

static SwPageDesc* lcl_FindPageDesc( const SwPageDescs *pPageDescs,
                                     size_t *pPos, const OUString &rName )
{
    SwPageDesc* res = nullptr;
    SwPageDescs::const_iterator it = pPageDescs->find( rName );
    if( it != pPageDescs->end() )
    {
        res = *it;
        if( pPos )
            *pPos = std::distance( pPageDescs->begin(), it );
    }
    else if( pPos )
        *pPos = SIZE_MAX;
    return res;
}

SwPageDesc* SwDoc::FindPageDesc( const OUString & rName, size_t* pPos ) const
{
    return lcl_FindPageDesc( &m_PageDescs, pPos, rName );
}

bool SwDoc::ContainsPageDesc( const SwPageDesc *pDesc, size_t* pPos ) const
{
    if( pDesc == nullptr )
        return false;
    if( !m_PageDescs.contains( const_cast <SwPageDesc*>( pDesc ) ) ) {
        if( pPos )
            *pPos = SIZE_MAX;
        return false;
    }
    if( ! pPos )
        return true;

    SwPageDesc* desc = lcl_FindPageDesc(
        &m_PageDescs, pPos, pDesc->GetName() );
    SAL_WARN_IF( desc != pDesc, "sw", "SwPageDescs container is broken!" );
    return true;
}

void SwDoc::DelPageDesc( const OUString & rName, bool bBroadcast )
{
    size_t nI;

    if (FindPageDesc(rName, &nI))
        DelPageDesc(nI, bBroadcast);
}

void SwDoc::ChgPageDesc( const OUString & rName, const SwPageDesc & rDesc)
{
    size_t nI;

    if (FindPageDesc(rName, &nI))
        ChgPageDesc(nI, rDesc);
}

/*
 * The HTML import cannot resist changing the page descriptions, I don't
 * know why. This function is meant to check the page descriptors for invalid
 * values.
 */
void SwDoc::CheckDefaultPageFormat()
{
    for ( size_t i = 0; i < GetPageDescCnt(); ++i )
    {
        SwPageDesc& rDesc = GetPageDesc( i );

        SwFrameFormat& rMaster = rDesc.GetMaster();
        SwFrameFormat& rLeft   = rDesc.GetLeft();

        const SwFormatFrameSize& rMasterSize  = rMaster.GetFrameSize();
        const SwFormatFrameSize& rLeftSize    = rLeft.GetFrameSize();

        const bool bSetSize = INVALID_TWIPS == rMasterSize.GetWidth() ||
                              INVALID_TWIPS == rMasterSize.GetHeight() ||
                              INVALID_TWIPS == rLeftSize.GetWidth() ||
                              INVALID_TWIPS == rLeftSize.GetHeight();

        if ( bSetSize )
            lcl_DefaultPageFormat( rDesc.GetPoolFormatId(), rDesc.GetMaster(), rDesc.GetLeft(), rDesc.GetFirstMaster(), rDesc.GetFirstLeft() );
    }
}

void SwDoc::SetDefaultPageMode(bool bSquaredPageMode)
{
    if( !bSquaredPageMode == !IsSquaredPageMode() )
        return;

    const SwTextGridItem& rGrid = GetDefault( RES_TEXTGRID );
    SwTextGridItem aNewGrid = rGrid;
    aNewGrid.SetSquaredMode(bSquaredPageMode);
    aNewGrid.Init();
    SetDefault(aNewGrid);

    for ( size_t i = 0; i < GetPageDescCnt(); ++i )
    {
        SwPageDesc& rDesc = GetPageDesc( i );

        SwFrameFormat& rMaster = rDesc.GetMaster();
        SwFrameFormat& rLeft = rDesc.GetLeft();

        SwTextGridItem aGrid(rMaster.GetFormatAttr(RES_TEXTGRID));
        aGrid.SwitchPaperMode( bSquaredPageMode );
        rMaster.SetFormatAttr(aGrid);
        rLeft.SetFormatAttr(aGrid);
    }
}

bool SwDoc::IsSquaredPageMode() const
{
    const SwTextGridItem& rGrid = GetDefault( RES_TEXTGRID );
    return rGrid.IsSquaredMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
