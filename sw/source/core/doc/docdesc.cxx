/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <svx/svdmodel.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/paperinf.hxx>
#include "editeng/frmdiritem.hxx"
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
#include <docary.hxx>
#include <rootfrm.hxx>  //For DelPageDesc
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

using namespace com::sun::star;

static void lcl_DefaultPageFmt( sal_uInt16 nPoolFmtId,
                                SwFrmFmt &rFmt1,
                                SwFrmFmt &rFmt2,
                                SwFrmFmt &rFmt3 )
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
}

/*************************************************************************
|*
|*  SwDoc::ChgPageDesc()
|*
|*************************************************************************/

static void lcl_DescSetAttr( const SwFrmFmt &rSource, SwFrmFmt &rDest,
                         const sal_Bool bPage = sal_True )
{
    // We should actually use ItemSet's Intersect here, but that doesn't work
    // correctly if we have different WhichRanges.

    // Take over the attributes which are of interest.
    sal_uInt16 const aIdArr[] = { RES_FRM_SIZE, RES_UL_SPACE,
                                        RES_BACKGROUND, RES_SHADOW,
                                        RES_COL, RES_COL,
                                        RES_FRAMEDIR, RES_FRAMEDIR,
                                        RES_TEXTGRID, RES_TEXTGRID,
                                        // #i45539#
                                        RES_HEADER_FOOTER_EAT_SPACING,
                                        RES_HEADER_FOOTER_EAT_SPACING,
                                        RES_UNKNOWNATR_CONTAINER,
                                        RES_UNKNOWNATR_CONTAINER,
                                        0 };

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
            if( (  bPage && RES_HEADER_FOOTER_EAT_SPACING != nId ) ||
                ( !bPage && RES_COL != nId && RES_PAPER_BIN != nId ))
            {
                if( SFX_ITEM_SET == rSource.GetItemState( nId, sal_False, &pItem ))
                    rDest.SetFmtAttr( *pItem );
                else
                    rDest.ResetFmtAttr( nId );
            }
        }
    }

    // Transmit pool and help IDs too
    rDest.SetPoolFmtId( rSource.GetPoolFmtId() );
    rDest.SetPoolHelpId( rSource.GetPoolHelpId() );
    rDest.SetPoolHlpFileId( rSource.GetPoolHlpFileId() );
}

void SwDoc::CopyMasterHeader(const SwPageDesc &rChged, const SwFmtHeader &rHead, SwPageDesc *pDesc, bool bLeft)
{
    SwFrmFmt& rDescFrmFmt = (bLeft ? pDesc->GetLeft() : pDesc->GetFirst());
    if ( (bLeft ? rChged.IsHeaderShared() : rChged.IsFirstShared() ) || !rHead.IsActive() )
    {
        // Left or first shares the header with the Master.
        rDescFrmFmt.SetFmtAttr( pDesc->GetMaster().GetHeader() );
    }
    else if ( rHead.IsActive() )
    {   // Left or first gets its own header if the Format doesn't alrady have one.
        // If it already has one and it points to the same Section as the
        // Right one, it needs to get an own Header.
        // The content is evidently copied.
        const SwFmtHeader &rFmtHead = rDescFrmFmt.GetHeader();
        if ( !rFmtHead.IsActive() )
        {
            SwFmtHeader aHead( MakeLayoutFmt( RND_STD_HEADERL, 0 ) );
            rDescFrmFmt.SetFmtAttr( aHead );
            // take over additional attributes (margins, borders ...)
            ::lcl_DescSetAttr( *rHead.GetHeaderFmt(), *aHead.GetHeaderFmt(), sal_False);
        }
        else
        {
            const SwFrmFmt *pRight = rHead.GetHeaderFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aCnt = rFmtHead.GetHeaderFmt()->GetCntnt();
            if( !aCnt.GetCntntIdx() )
            {
                const SwFrmFmt& rChgedFrmFmt = (bLeft ? rChged.GetLeft() : rChged.GetFirst());
                rDescFrmFmt.SetFmtAttr( rChgedFrmFmt.GetHeader() );
            }
            else if( (*aRCnt.GetCntntIdx()) == (*aCnt.GetCntntIdx()) )
            {
                SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), (bLeft ? "Left header" : "First header"),
                                                GetDfltFrmFmt() );
                ::lcl_DescSetAttr( *pRight, *pFmt, sal_False );
                // The section which the right header attribute is pointing
                // is copied, and the Index to the StartNode is set to
                // the left or first header attribute.
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwHeaderStartNode );
                SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
                            *aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes()._Copy( aRange, aTmp, sal_False );

                pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ) );
                rDescFrmFmt.SetFmtAttr( SwFmtHeader( pFmt ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *(SwFrmFmt*)rFmtHead.GetHeaderFmt(), sal_False );

        }
    }
}

void SwDoc::CopyMasterFooter(const SwPageDesc &rChged, const SwFmtFooter &rFoot, SwPageDesc *pDesc, bool bLeft)
{
    SwFrmFmt& rDescFrmFmt = (bLeft ? pDesc->GetLeft() : pDesc->GetFirst());
    if ( (bLeft ? rChged.IsFooterShared() : rChged.IsFirstShared() ) || !rFoot.IsActive() )
        // Left or first shares the Header with the Master.
        rDescFrmFmt.SetFmtAttr( pDesc->GetMaster().GetFooter() );
    else if ( rFoot.IsActive() )
    {   // Left or first gets its own Footer if the Format does not already have one.
        // If the Format already has a Footer and it points to the same section as the Right one,
        // it needs to get an own one.
        // The content is evidently copied.
        const SwFmtFooter &rFmtFoot = rDescFrmFmt.GetFooter();
        if ( !rFmtFoot.IsActive() )
        {
            SwFmtFooter aFoot( MakeLayoutFmt( RND_STD_FOOTER, 0 ) );
            rDescFrmFmt.SetFmtAttr( aFoot );
            // Take over additional attributes (margins, borders ...).
            ::lcl_DescSetAttr( *rFoot.GetFooterFmt(), *aFoot.GetFooterFmt(), sal_False);
        }
        else
        {
            const SwFrmFmt *pRight = rFoot.GetFooterFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aLCnt = rFmtFoot.GetFooterFmt()->GetCntnt();
            if( !aLCnt.GetCntntIdx() )
            {
                const SwFrmFmt& rChgedFrmFmt = (bLeft ? rChged.GetLeft() : rChged.GetFirst());
                rDescFrmFmt.SetFmtAttr( rChgedFrmFmt.GetFooter() );
            }
            else if( (*aRCnt.GetCntntIdx()) == (*aLCnt.GetCntntIdx()) )
            {
                SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), (bLeft ? "Left footer" : "First footer"),
                                                GetDfltFrmFmt() );
                ::lcl_DescSetAttr( *pRight, *pFmt, sal_False );
                // The section to which the right footer attribute is pointing
                // is copied, and the Index to the StartNode is set to
                // the left footer attribute.
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwFooterStartNode );
                SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
                            *aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes()._Copy( aRange, aTmp, sal_False );

                pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ) );
                rDescFrmFmt.SetFmtAttr( SwFmtFooter( pFmt ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *(SwFrmFmt*)rFmtFoot.GetFooterFmt(), sal_False );
        }
    }
}

void SwDoc::ChgPageDesc( sal_uInt16 i, const SwPageDesc &rChged )
{
    OSL_ENSURE( i < aPageDescs.size(), "PageDescs is out of range." );

    SwPageDesc *pDesc = aPageDescs[i];
    SwRootFrm* pTmpRoot = GetCurrentLayout();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo *const pUndo(new SwUndoPageDesc(*pDesc, rChged, this));
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // Mirror at first if needed.
    if ( rChged.GetUseOn() == nsUseOnPage::PD_MIRROR )
        ((SwPageDesc&)rChged).Mirror();
    else
    {
        // Or else transfer values from Master to Left and First.
        ::lcl_DescSetAttr( ((SwPageDesc&)rChged).GetMaster(),
                       ((SwPageDesc&)rChged).GetLeft() );
        ::lcl_DescSetAttr( ((SwPageDesc&)rChged).GetMaster(),
                       ((SwPageDesc&)rChged).GetFirst() );
    }

    // Take over NumType.
    if( rChged.GetNumType().GetNumberingType() != pDesc->GetNumType().GetNumberingType() )
    {
        pDesc->SetNumType( rChged.GetNumType() );
        // Notify page number fields that NumFormat has changed
        GetSysFldType( RES_PAGENUMBERFLD )->UpdateFlds();
        GetSysFldType( RES_REFPAGEGETFLD )->UpdateFlds();

        // If the numbering scheme has changed we could have QuoVadis/ErgoSum texts
        // that refer to a changed page, so we invalidate foot notes.
        SwFtnIdxs& rFtnIdxs = GetFtnIdxs();
        for( sal_uInt16 nPos = 0; nPos < rFtnIdxs.size(); ++nPos )
        {
            SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
            const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
            pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
        }
    }

    // Take over orientation
    pDesc->SetLandscape( rChged.GetLandscape() );
    pDesc->ChgFirstShare( rChged.IsFirstShared() );

    // #i46909# no undo if header or footer changed
    bool bHeaderFooterChanged = false;

    // Synch header.
    const SwFmtHeader &rHead = rChged.GetMaster().GetHeader();
    if (undoGuard.UndoWasEnabled())
    {
        // #i46909# no undo if header or footer changed
        // Did something change in the nodes?
        const SwFmtHeader &rOldHead = pDesc->GetMaster().GetHeader();
        bHeaderFooterChanged |=
            ( rHead.IsActive() != rOldHead.IsActive() ||
              rChged.IsHeaderShared() != pDesc->IsHeaderShared() ||
              rChged.IsFirstShared() != pDesc->IsFirstShared() );
    }
    pDesc->GetMaster().SetFmtAttr( rHead );
    CopyMasterHeader(rChged, rHead, pDesc, true); // Copy left header
    CopyMasterHeader(rChged, rHead, pDesc, false); // Copy first header
    pDesc->ChgHeaderShare( rChged.IsHeaderShared() );

    // Synch Footer.
    const SwFmtFooter &rFoot = rChged.GetMaster().GetFooter();
    if (undoGuard.UndoWasEnabled())
    {
        // #i46909# no undo if header or footer changed
        // Did something change in the Nodes?
        const SwFmtFooter &rOldFoot = pDesc->GetMaster().GetFooter();
        bHeaderFooterChanged |=
            ( rFoot.IsActive() != rOldFoot.IsActive() ||
              rChged.IsFooterShared() != pDesc->IsFooterShared() ||
              rChged.IsFirstShared() != pDesc->IsFirstShared() );
    }
    pDesc->GetMaster().SetFmtAttr( rFoot );
    CopyMasterFooter(rChged, rFoot, pDesc, true); // Copy left footer
    CopyMasterFooter(rChged, rFoot, pDesc, false); // Copy first footer
    pDesc->ChgFooterShare( rChged.IsFooterShared() );

    if ( pDesc->GetName() != rChged.GetName() )
        pDesc->SetName( rChged.GetName() );

    // A RegisterChange is triggered, if necessary
    pDesc->SetRegisterFmtColl( rChged.GetRegisterFmtColl() );

    // If UseOn or the Follow change, the paragraphs need to know about it.
    sal_Bool bUseOn  = sal_False;
    sal_Bool bFollow = sal_False;
    if ( pDesc->GetUseOn() != rChged.GetUseOn() )
    {   pDesc->SetUseOn( rChged.GetUseOn() );
        bUseOn = sal_True;
    }
    if ( pDesc->GetFollow() != rChged.GetFollow() )
    {   if ( rChged.GetFollow() == &rChged )
        {   if ( pDesc->GetFollow() != pDesc )
            {   pDesc->SetFollow( pDesc );
                bFollow = sal_True;
            }
        }
        else
        {   pDesc->SetFollow( rChged.pFollow );
            bFollow = sal_True;
        }
    }

    if ( (bUseOn || bFollow) && pTmpRoot)
        // Inform layout!
    {
        std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllCheckPageDescs));
    }

    // Take over the page attributes.
    ::lcl_DescSetAttr( rChged.GetMaster(), pDesc->GetMaster() );
    ::lcl_DescSetAttr( rChged.GetLeft(), pDesc->GetLeft() );
    ::lcl_DescSetAttr( rChged.GetFirst(), pDesc->GetFirst() );

    // If the FootnoteInfo changes, the pages are triggered.
    if( !(pDesc->GetFtnInfo() == rChged.GetFtnInfo()) )
    {
        pDesc->SetFtnInfo( rChged.GetFtnInfo() );
        SwMsgPoolItem  aInfo( RES_PAGEDESC_FTNINFO );
        {
            pDesc->GetMaster().ModifyBroadcast( &aInfo, 0, TYPE(SwFrm) );
        }
        {
            pDesc->GetLeft().ModifyBroadcast( &aInfo, 0, TYPE(SwFrm) );
        }
        {
            pDesc->GetFirst().ModifyBroadcast( &aInfo, 0, TYPE(SwFrm) );
        }
    }
    SetModified();

    // #i46909# no undo if header or footer changed
    if( bHeaderFooterChanged )
    {
        GetIDocumentUndoRedo().DelAllUndoObj();
    }
}

/*************************************************************************
|*
|*  SwDoc::DelPageDesc()
|*
|*  Description: All descriptors whose Follow point to the to-be-deleted
|*  have to be adapted.
|*
|*************************************************************************/

// #i7983#
void SwDoc::PreDelPageDesc(SwPageDesc * pDel)
{
    if (0 == pDel)
        return;

    // mba: test iteration as clients are removed while iteration
    SwPageDescHint aHint( aPageDescs[0] );
    pDel->CallSwClientNotify( aHint );

    bool bHasLayout = HasLayout();
    if ( pFtnInfo->DependsOn( pDel ) )
    {
        pFtnInfo->ChgPageDesc( aPageDescs[0] );
        if ( bHasLayout )
        {
            std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::CheckFtnPageDescs), false));
        }
    }
    else if ( pEndNoteInfo->DependsOn( pDel ) )
    {
        pEndNoteInfo->ChgPageDesc( aPageDescs[0] );
        if ( bHasLayout )
        {
            std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::CheckFtnPageDescs), true));
        }
    }

    for ( sal_uInt16 j = 0; j < aPageDescs.size(); ++j )
    {
        if ( aPageDescs[j]->GetFollow() == pDel )
        {
            aPageDescs[j]->SetFollow( 0 );
            if( bHasLayout )
            {
                std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
                std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllCheckPageDescs));
            }
        }
    }
}

void SwDoc::BroadcastStyleOperation(String rName, SfxStyleFamily eFamily,
                                    sal_uInt16 nOp)
{
    if (pDocShell)
    {
        SfxStyleSheetBasePool * pPool = pDocShell->GetStyleSheetPool();

        if (pPool)
        {
            pPool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
            SfxStyleSheetBase * pBase = pPool->Find(rName);

            if (pBase != NULL)
                pPool->Broadcast(SfxStyleSheetHint( nOp, *pBase ));
        }
    }
}

void SwDoc::DelPageDesc( sal_uInt16 i, sal_Bool bBroadcast )
{
    OSL_ENSURE( i < aPageDescs.size(), "PageDescs is out of range." );
    OSL_ENSURE( i != 0, "You cannot delete the default Pagedesc.");
    if ( i == 0 )
        return;

    SwPageDesc *pDel = aPageDescs[i];

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SFX_STYLE_FAMILY_PAGE,
                                SFX_STYLESHEET_ERASED);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo *const pUndo(new SwUndoPageDescDelete(*pDel, this));
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    PreDelPageDesc(pDel); // #i7983#

    aPageDescs.erase( aPageDescs.begin() + i );
    delete pDel;
    SetModified();
}



/*************************************************************************
|*
|*  SwDoc::MakePageDesc()
|*
|*************************************************************************/

sal_uInt16 SwDoc::MakePageDesc( const String &rName, const SwPageDesc *pCpy,
                            sal_Bool bRegardLanguage, sal_Bool bBroadcast)
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
        lcl_DefaultPageFmt( USHRT_MAX, pNew->GetMaster(), pNew->GetLeft(), pNew->GetFirst() );

        SvxFrameDirection aFrameDirection = bRegardLanguage ?
            GetDefaultFrameDirection(GetAppLanguage())
            : FRMDIR_HORI_LEFT_TOP;

        pNew->GetMaster().SetFmtAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
        pNew->GetLeft().SetFmtAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
        pNew->GetFirst().SetFmtAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
    }
    aPageDescs.push_back( pNew );

    if (bBroadcast)
        BroadcastStyleOperation(rName, SFX_STYLE_FAMILY_PAGE,
                                SFX_STYLESHEET_CREATED);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(new SwUndoPageDescCreate(pNew, this));
    }

    SetModified();
    return (aPageDescs.size()-1);
}

SwPageDesc* SwDoc::FindPageDescByName( const String& rName, sal_uInt16* pPos ) const
{
    SwPageDesc* pRet = 0;
    if( pPos ) *pPos = USHRT_MAX;

    for( sal_uInt16 n = 0, nEnd = aPageDescs.size(); n < nEnd; ++n )
        if( aPageDescs[ n ]->GetName() == rName )
        {
            pRet = aPageDescs[ n ];
            if( pPos )
                *pPos = n;
            break;
        }
    return pRet;
}

/******************************************************************************
 *  Method      :   void SwDoc::PrtDataChanged()
 *  Description :
 ******************************************************************************/

void SwDoc::PrtDataChanged()
{
// If you change this, also modify InJobSetup in Sw3io if appropriate.

    // #i41075#
    OSL_ENSURE( get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) ||
            0 != getPrinter( sal_False ), "PrtDataChanged will be called recursively!" );
    SwRootFrm* pTmpRoot = GetCurrentLayout();
    SwWait *pWait = 0;
    sal_Bool bEndAction = sal_False;

    if( GetDocShell() )
        GetDocShell()->UpdateFontList();

    sal_Bool bDraw = sal_True;
    if ( pTmpRoot )
    {
        ViewShell *pSh = GetCurrentViewShell();
        if( pSh &&
            (!pSh->GetViewOptions()->getBrowseMode() ||
             pSh->GetViewOptions()->IsPrtFormat()) )
        {
            if ( GetDocShell() )
                pWait = new SwWait( *GetDocShell(), sal_True );

            pTmpRoot->StartAllAction();
            bEndAction = sal_True;

            bDraw = sal_False;
            if( pDrawModel )
            {
                pDrawModel->SetAddExtLeading( get(IDocumentSettingAccess::ADD_EXT_LEADING) );
                pDrawModel->SetRefDevice( getReferenceDevice( false ) );
            }

            pFntCache->Flush();

            std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::InvalidateAllCntnt), INV_SIZE));

            do
            {
                pSh->InitPrt( pPrt );
                pSh = (ViewShell*)pSh->GetNext();
            }
            while ( pSh != GetCurrentViewShell() );
        }
    }
    if ( bDraw && pDrawModel )
    {
        const sal_Bool bTmpAddExtLeading = get(IDocumentSettingAccess::ADD_EXT_LEADING);
        if ( bTmpAddExtLeading != pDrawModel->IsAddExtLeading() )
            pDrawModel->SetAddExtLeading( bTmpAddExtLeading );

        OutputDevice* pOutDev = getReferenceDevice( false );
        if ( pOutDev != pDrawModel->GetRefDevice() )
            pDrawModel->SetRefDevice( pOutDev );
    }

    PrtOLENotify( sal_True );

    if ( bEndAction )
        pTmpRoot->EndAllAction();
    delete pWait;
}

// We collect the GlobalNames of the servers at runtime, who don't want to be notified
// about printer changes. Thereby saving loading a lot of objects (luckily all foreign
// objects are mapped to one ID).
// Initialisation and deinitialisation can be found in init.cxx
extern std::vector<SvGlobalName*> *pGlobalOLEExcludeList;

void SwDoc::PrtOLENotify( sal_Bool bAll )
{
    SwFEShell *pShell = 0;
    if ( GetCurrentViewShell() )
    {
        ViewShell *pSh = GetCurrentViewShell();
        if ( !pSh->ISA(SwFEShell) )
            do
            {   pSh = (ViewShell*)pSh->GetNext();
            } while ( !pSh->ISA(SwFEShell) &&
                      pSh != GetCurrentViewShell() );

        if ( pSh->ISA(SwFEShell) )
            pShell = (SwFEShell*)pSh;
    }
    if ( !pShell )
    {
        // This doesn't make sense without a Shell and thus without a client, because
        // the communication about size changes is implemented by these components.
        // Because we don't have a Shell we remember this unfortunate situation
        // in the document,
        // which is made up for later on when creating the first Shell.
        mbOLEPrtNotifyPending = sal_True;
        if ( bAll )
            mbAllOLENotify = sal_True;
    }
    else
    {
        if ( mbAllOLENotify )
            bAll = sal_True;

        mbOLEPrtNotifyPending = mbAllOLENotify = sal_False;

        SwOLENodes *pNodes = SwCntntNode::CreateOLENodesArray( *GetDfltGrfFmtColl(), !bAll );
        if ( pNodes )
        {
            ::StartProgress( STR_STATSTR_SWGPRTOLENOTIFY,
                             0, pNodes->size(), GetDocShell());
            GetCurrentLayout()->StartAllAction();

            for( sal_uInt16 i = 0; i < pNodes->size(); ++i )
            {
                ::SetProgressState( i, GetDocShell() );

                SwOLENode* pOLENd = (*pNodes)[i];
                pOLENd->SetOLESizeInvalid( sal_False );

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

                sal_Bool bFound = sal_False;
                for ( sal_uInt16 j = 0;
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
            GetCurrentLayout()->EndAllAction();
            ::EndProgress( GetDocShell() );
        }
    }
}

IMPL_LINK( SwDoc, DoUpdateModifiedOLE, Timer *, )
{
    SwFEShell* pSh = (SwFEShell*)GetEditShell();
    if( pSh )
    {
        mbOLEPrtNotifyPending = mbAllOLENotify = sal_False;

        SwOLENodes *pNodes = SwCntntNode::CreateOLENodesArray( *GetDfltGrfFmtColl(), true );
        if( pNodes )
        {
            ::StartProgress( STR_STATSTR_SWGPRTOLENOTIFY,
                             0, pNodes->size(), GetDocShell());
            GetCurrentLayout()->StartAllAction();
            SwMsgPoolItem aMsgHint( RES_UPDATE_ATTR );

            for( sal_uInt16 i = 0; i < pNodes->size(); ++i )
            {
                ::SetProgressState( i, GetDocShell() );

                SwOLENode* pOLENd = (*pNodes)[i];
                pOLENd->SetOLESizeInvalid( sal_False );

                // We don't know it, so the object has to be loaded.
                // If it doesn't want to be informed
                if( pOLENd->GetOLEObj().GetOleRef().is() ) // Broken?
                {
                    pOLENd->ModifyNotification( &aMsgHint, &aMsgHint );
                }
            }
            GetCurrentLayout()->EndAllAction();
            ::EndProgress( GetDocShell() );
            delete pNodes;
        }
    }
    return 0;
}

sal_Bool SwDoc::FindPageDesc( const String & rName, sal_uInt16 * pFound)
{
    sal_Bool bResult = sal_False;
    sal_uInt16 nI;
    for (nI = 0; nI < aPageDescs.size(); nI++)
    {
        if (aPageDescs[nI]->GetName() == rName)
        {
            *pFound = nI;
            bResult = sal_True;
            break;
        }
    }

    return bResult;
}

SwPageDesc * SwDoc::GetPageDesc( const String & rName )
{
    SwPageDesc * aResult = NULL;

    sal_uInt16 nI;

    if (FindPageDesc(rName, &nI))
        aResult = aPageDescs[nI];

    return aResult;
}

void SwDoc::DelPageDesc( const String & rName, sal_Bool bBroadcast )
{
    sal_uInt16 nI;

    if (FindPageDesc(rName, &nI))
        DelPageDesc(nI, bBroadcast);
}

void SwDoc::ChgPageDesc( const String & rName, const SwPageDesc & rDesc)
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
            lcl_DefaultPageFmt( rDesc.GetPoolFmtId(), rDesc.GetMaster(), rDesc.GetLeft(), rDesc.GetFirst() );
    }
}

void SwDoc::SetDefaultPageMode(bool bSquaredPageMode)
{
    if( !bSquaredPageMode == !IsSquaredPageMode() )
        return;

    const SwTextGridItem& rGrid =
                    (const SwTextGridItem&)GetDefault( RES_TEXTGRID );
    SwTextGridItem aNewGrid = rGrid;
    aNewGrid.SetSquaredMode(bSquaredPageMode);
    aNewGrid.Init();
    SetDefault(aNewGrid);

    for ( sal_uInt16 i = 0; i < GetPageDescCnt(); ++i )
    {
        SwPageDesc& rDesc = GetPageDesc( i );

        SwFrmFmt& rMaster = rDesc.GetMaster();
        SwFrmFmt& rLeft = rDesc.GetLeft();

        SwTextGridItem aGrid((SwTextGridItem&)rMaster.GetFmtAttr(RES_TEXTGRID));
        aGrid.SwitchPaperMode( bSquaredPageMode );
        rMaster.SetFmtAttr(aGrid);
        rLeft.SetFmtAttr(aGrid);
    }
}

sal_Bool SwDoc::IsSquaredPageMode() const
{
    const SwTextGridItem& rGrid =
                        (const SwTextGridItem&)GetDefault( RES_TEXTGRID );
    return rGrid.IsSquaredMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
