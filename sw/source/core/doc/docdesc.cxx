/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <cmdid.h>
#include <hintids.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/paperinf.hxx>
#include "editeng/frmdiritem.hxx"
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <fmtpdsc.hxx>
#include <ftninfo.hxx>
#include <fesh.hxx>
#include <ndole.hxx>
#include <mdiexp.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <pagefrm.hxx>  // for DelPageDesc
#include <rootfrm.hxx>  // for DelPageDesc
#include <ndtxt.hxx>
#include <frmtool.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <docsh.hxx>
#include <ndindex.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <fntcache.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <fldbas.hxx>
#include <swwait.hxx>
#include <GetMetricVal.hxx>
#include <unotools/syslocale.hxx>
#include <statstr.hrc>
#include <switerator.hxx>
#include <hints.hxx>
#include <SwUndoPageDesc.hxx>
#include <pagedeschint.hxx>
#include <tgrditem.hxx>
#include <drawdoc.hxx>

using namespace com::sun::star;

static void lcl_DefaultPageFmt( sal_uInt16 nPoolFmtId,
                                SwFrmFmt &rFmt1,
                                SwFrmFmt &rFmt2 )
{
    // --> FME 2005-01-21 #i41075# Printer on demand
    // This function does not require a printer anymore.
    // The default page size is obtained from the application
    //locale
    // <--

    SwFmtFrmSize aFrmSize( ATT_FIX_SIZE );
    const Size aPhysSize = SvxPaperInfo::GetDefaultPaperSize();
    aFrmSize.SetSize( aPhysSize );

    // Prepare for default margins.
    // Margins have a default minimum size.
    // If the printer gives a larger margin, then it's OK as well.
    // MIB 06/25/2002, #99397#: The HTML page desc had A4 as page size
    // always. This has been changed to take the page size from the printer.
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
        nMinTop = nMinBottom = nMinLeft = nMinRight = 1134; //2 centimeters
    }
    else
    {
        nMinTop = nMinBottom = 1440;    //al la WW: 1 Inch
        nMinLeft = nMinRight = 1800;    //          1,25 Inch
    }

    // set the margins
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
}

/*************************************************************************
|*
|*  SwDoc::ChgPageDesc()
|*
|*  Created         MA 25. Jan. 93
|*  Last change     MA 01. Mar. 95
|*
|*************************************************************************/

void lcl_DescSetAttr( const SwFrmFmt &rSource, SwFrmFmt &rDest,
                         const sal_Bool bPage = sal_True )
{
/////////////// !!!!!!!!!!!!!!!!
//JP 03.03.99:
// Actually the Intersect from ItemSet should be used here but that doesn't
// work correctly when you have different WhichRanges.
/////////////// !!!!!!!!!!!!!!!!
    // Take over the interesting attributes.
    sal_uInt16 __READONLY_DATA aIdArr[] = {
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
                    // at SwFmt::GetItemState and not really in state SFX_ITEM_SET
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
                if(SFX_ITEM_SET == rSource.GetItemState(nId,sal_False,&pItem))
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

    // transfer also the Pool-, Help-ID's
    rDest.SetPoolFmtId( rSource.GetPoolFmtId() );
    rDest.SetPoolHelpId( rSource.GetPoolHelpId() );
    rDest.SetPoolHlpFileId( rSource.GetPoolHlpFileId() );
}


void SwDoc::ChgPageDesc( sal_uInt16 i, const SwPageDesc &rChged )
{
    ASSERT( i < aPageDescs.Count(), "PageDescs ueberindiziert." );

    SwPageDesc *pDesc = aPageDescs[i];
    SwRootFrm* pTmpRoot = GetCurrentLayout();//swmod 080219

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo *const pUndo(new SwUndoPageDesc(*pDesc, rChged, this));
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // first mirror if necessary
    if ( rChged.GetUseOn() == nsUseOnPage::PD_MIRROR )
        ((SwPageDesc&)rChged).Mirror();
    else
        // otherwise transfer the values from Master to Left.
        ::lcl_DescSetAttr( ((SwPageDesc&)rChged).GetMaster(),
                       ((SwPageDesc&)rChged).GetLeft() );

    // take over NumType.
    if( rChged.GetNumType().GetNumberingType() != pDesc->GetNumType().GetNumberingType() )
    {
        pDesc->SetNumType( rChged.GetNumType() );
        // JP 30.03.99: Bug 64121 - tell it to the page number fields
        //      that the numbering format has changed
        GetSysFldType( RES_PAGENUMBERFLD )->UpdateFlds();
        GetSysFldType( RES_REFPAGEGETFLD )->UpdateFlds();

        // When the numbering type has changed, it's possible that there are QuoVadis/
        // ErgoSum texts, that are referring to a changed page,
        // therefore the foot notes will be invalidated
        SwFtnIdxs& rFtnIdxs = GetFtnIdxs();
        for( sal_uInt16 nPos = 0; nPos < rFtnIdxs.Count(); ++nPos )
        {
            SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
            const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
            pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
        }
    }

    // take over the orientation
    pDesc->SetLandscape( rChged.GetLandscape() );

    // #i46909# no undo if header or footer changed
    bool bHeaderFooterChanged = false;

    // align header
    const SwFmtHeader &rHead = rChged.GetMaster().GetHeader();
    if (undoGuard.UndoWasEnabled())
    {
        // #i46909# no undo if header or footer changed
        // are there changes in the Nodes?
        const SwFmtHeader &rOldHead = pDesc->GetMaster().GetHeader();
        bHeaderFooterChanged |=
            ( rHead.IsActive() != rOldHead.IsActive() ||
              rChged.IsHeaderShared() != pDesc->IsHeaderShared() );
    }
    pDesc->GetMaster().SetFmtAttr( rHead );
    if ( rChged.IsHeaderShared() || !rHead.IsActive() )
    {
        // Left is sharing the Header with the Master
        pDesc->GetLeft().SetFmtAttr( pDesc->GetMaster().GetHeader() );
    }
    else if ( rHead.IsActive() )
    {   // Left get an own Header, when the Format hasn't one already.
        // If it has already one and it refers to the same section like the Right,
        // then it has to get an own Header. The content will be copied accordingly.

        const SwFmtHeader &rLeftHead = pDesc->GetLeft().GetHeader();
        if ( !rLeftHead.IsActive() )
        {
            SwFmtHeader aHead( MakeLayoutFmt( RND_STD_HEADERL, 0 ) );
            pDesc->GetLeft().SetFmtAttr( aHead );
            // take over further attributes (margins, borders ...)
            ::lcl_DescSetAttr( *rHead.GetHeaderFmt(), *aHead.GetHeaderFmt(), sal_False);
        }
        else
        {
            const SwFrmFmt *pRight = rHead.GetHeaderFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aLCnt = rLeftHead.GetHeaderFmt()->GetCntnt();
            if( !aLCnt.GetCntntIdx() )
                pDesc->GetLeft().SetFmtAttr( rChged.GetLeft().GetHeader() );
            else if( (*aRCnt.GetCntntIdx()) == (*aLCnt.GetCntntIdx()) )
            {
                SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), "Header",
                                                GetDfltFrmFmt() );
                ::lcl_DescSetAttr( *pRight, *pFmt, sal_False );
                // The section that refers to the right head attribute will be copied and
                // the index on the StartNode will be attached into the left head attribute.
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwHeaderStartNode );
                SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
                            *aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes()._Copy( aRange, aTmp, sal_False );

                pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ) );
                pDesc->GetLeft().SetFmtAttr( SwFmtHeader( pFmt ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *(SwFrmFmt*)rLeftHead.GetHeaderFmt(), sal_False );

        }
    }
    pDesc->ChgHeaderShare( rChged.IsHeaderShared() );

    // align footer
    const SwFmtFooter &rFoot = rChged.GetMaster().GetFooter();
    if (undoGuard.UndoWasEnabled())
    {
        // #i46909# no undo if header or footer changed
        // are there changes in the Nodes?
        const SwFmtFooter &rOldFoot = pDesc->GetMaster().GetFooter();
        bHeaderFooterChanged |=
            ( rFoot.IsActive() != rOldFoot.IsActive() ||
              rChged.IsFooterShared() != pDesc->IsFooterShared() );
    }
    pDesc->GetMaster().SetFmtAttr( rFoot );
    if ( rChged.IsFooterShared() || !rFoot.IsActive() )
        // Left is sharing the Footer with the Master
        pDesc->GetLeft().SetFmtAttr( pDesc->GetMaster().GetFooter() );
    else if ( rFoot.IsActive() )
    {   // Left get an own Footer, when the Format hasn't one already.
        // If it has already one and it refers to the same section like the Right,
        // then it has to get an own Footer. The content will be copied accordingly.
        const SwFmtFooter &rLeftFoot = pDesc->GetLeft().GetFooter();
        if ( !rLeftFoot.IsActive() )
        {
            SwFmtFooter aFoot( MakeLayoutFmt( RND_STD_FOOTER, 0 ) );
            pDesc->GetLeft().SetFmtAttr( aFoot );
            // take over further attributes (margins, borders ...)
            ::lcl_DescSetAttr( *rFoot.GetFooterFmt(), *aFoot.GetFooterFmt(), sal_False);
        }
        else
        {
            const SwFrmFmt *pRight = rFoot.GetFooterFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aLCnt = rLeftFoot.GetFooterFmt()->GetCntnt();
            if( !aLCnt.GetCntntIdx() )
                pDesc->GetLeft().SetFmtAttr( rChged.GetLeft().GetFooter() );
            else if( (*aRCnt.GetCntntIdx()) == (*aLCnt.GetCntntIdx()) )
            {
                SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), "Footer",
                                                GetDfltFrmFmt() );
                ::lcl_DescSetAttr( *pRight, *pFmt, sal_False );
                // The section that refers to the right head attribute will be copied and
                // the index on the StartNode will be attached into the left head attribute.
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwFooterStartNode );
                SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
                            *aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes()._Copy( aRange, aTmp, sal_False );

                pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ) );
                pDesc->GetLeft().SetFmtAttr( SwFmtFooter( pFmt ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *(SwFrmFmt*)rLeftFoot.GetFooterFmt(), sal_False );
        }
    }
    pDesc->ChgFooterShare( rChged.IsFooterShared() );

    if ( pDesc->GetName() != rChged.GetName() )
        pDesc->SetName( rChged.GetName() );

    // this will trigger a RegisterChange if necessary
    pDesc->SetRegisterFmtColl( rChged.GetRegisterFmtColl() );

    // when UseOn or Follow are changed, then the paragraphs have to know this
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
        // notify Layot!
    {
        std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllCheckPageDescs));//swmod 080304
    }

    // now take over the page attributes
    ::lcl_DescSetAttr( rChged.GetMaster(), pDesc->GetMaster() );
    ::lcl_DescSetAttr( rChged.GetLeft(), pDesc->GetLeft() );

    // when FootnotesInfo are changed, then trigger the pages
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
    }
    SetModified();

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

}

/*************************************************************************
|*
|*  SwDoc::DelPageDesc()
|*
|*  Description     All descriptors have to be adapted whose Follow
|*              refers to the one that has to be deleted.
|*
|*  Created         MA 25. Jan. 93
|*  Last change     JP 04.09.95
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

    for ( sal_uInt16 j = 0; j < aPageDescs.Count(); ++j )
    {
        if ( aPageDescs[j]->GetFollow() == pDel )
        {
            aPageDescs[j]->SetFollow( 0 );
            if( bHasLayout )
            {
                std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
                std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllCheckPageDescs));//swmod 080228
            }
        }
    }
}

// #116530#
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
    ASSERT( i < aPageDescs.Count(), "PageDescs are over-indexed." );
    ASSERT( i != 0, "Default Pagedesc cannot be deleted." );
    if ( i == 0 )
        return;

    SwPageDesc *pDel = aPageDescs[i];

    // -> #116530#
    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SFX_STYLE_FAMILY_PAGE,
                                SFX_STYLESHEET_ERASED);
    // <- #116530#

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo *const pUndo(new SwUndoPageDescDelete(*pDel, this));
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    PreDelPageDesc(pDel); // #i7983#

    aPageDescs.Remove( i );
    delete pDel;
    SetModified();
}



/*************************************************************************
|*
|*  SwDoc::MakePageDesc()
|*
|*  Created         MA 25. Jan. 93
|*  Last change     MA 20. Aug. 93
|*
|*************************************************************************/

sal_uInt16 SwDoc::MakePageDesc( const String &rName, const SwPageDesc *pCpy,
                            sal_Bool bRegardLanguage, sal_Bool bBroadcast) // #116530#
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
        // set default page format
        lcl_DefaultPageFmt( USHRT_MAX, pNew->GetMaster(), pNew->GetLeft() );

        SvxFrameDirection aFrameDirection = bRegardLanguage ?
            GetDefaultFrameDirection(GetAppLanguage())
            : FRMDIR_HORI_LEFT_TOP;

        pNew->GetMaster().SetFmtAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
        pNew->GetLeft().SetFmtAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
    }
    aPageDescs.Insert( pNew, aPageDescs.Count() );

    // -> #116530#
    if (bBroadcast)
        BroadcastStyleOperation(rName, SFX_STYLE_FAMILY_PAGE,
                                SFX_STYLESHEET_CREATED);
    // <- #116530#

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        // #116530#
        GetIDocumentUndoRedo().AppendUndo(new SwUndoPageDescCreate(pNew, this));
    }

    SetModified();
    return (aPageDescs.Count()-1);
}

SwPageDesc* SwDoc::FindPageDescByName( const String& rName, sal_uInt16* pPos ) const
{
    SwPageDesc* pRet = 0;
    if( pPos ) *pPos = USHRT_MAX;

    for( sal_uInt16 n = 0, nEnd = aPageDescs.Count(); n < nEnd; ++n )
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
 *  Method          void SwDoc::PrtDataChanged()
 *  Description
 *  Created         OK 27.10.94 10:20
 *  Last change     MA 26. Mar. 98
 ******************************************************************************/

void SwDoc::PrtDataChanged()
{
//!!!!!!!! In case of changes please look after InJobSetup in Sw3io if necessary

    // --> FME 2005-01-21 #i41075#
    ASSERT( get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) ||
            0 != getPrinter( sal_False ), "PrtDataChanged will be called recursive!" )
    // <--
    SwRootFrm* pTmpRoot = GetCurrentLayout();//swmod 080219
    SwWait *pWait = 0;
    sal_Bool bEndAction = sal_False;

    if( GetDocShell() )
        GetDocShell()->UpdateFontList();

    sal_Bool bDraw = sal_True;
    if ( pTmpRoot )
    {
        ViewShell *pSh = GetCurrentViewShell();
        if( !pSh->GetViewOptions()->getBrowseMode() ||
            pSh->GetViewOptions()->IsPrtFormat() )
        {
            if ( GetDocShell() )
                pWait = new SwWait( *GetDocShell(), true );

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
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::InvalidateAllCntnt), INV_SIZE));//swmod 080304

            if ( pSh )
            {
                do
                {
                    pSh->InitPrt( pPrt );
                    pSh = (ViewShell*)pSh->GetNext();
                }
                while ( pSh != GetCurrentViewShell() );
            }

        }
    }   //swmod 080218
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
        pTmpRoot->EndAllAction();   //swmod 080218
    delete pWait;
}

// At run time collect the GlobalNames of the server that don't want
// to be notified when printer have changed.
// Due to this many objects don't need to be loaded (luckily on top of
// this all alien objects are mapped to a single ID). Init and DeInit
// of the array can be found in init.cxx.
extern SvPtrarr *pGlobalOLEExcludeList;

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
    }   //swmod 071107//swmod 071225
    if ( !pShell )
    {
        // This doesn't make sense without Shell and therefore without Client because
        // communication relating to change in size is implemented only in this way.
        // As there is no Shell, note this unfavorable status in the Document, this
        // will be catched up when creating the first Shell.
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
                             0, pNodes->Count(), GetDocShell());
            GetCurrentLayout()->StartAllAction();   //swmod 080218

            for( sal_uInt16 i = 0; i < pNodes->Count(); ++i )
            {
                ::SetProgressState( i, GetDocShell() );

                SwOLENode* pOLENd = (*pNodes)[i];
                pOLENd->SetOLESizeInvalid( sal_False );

                // first load the information and determine whether it is already in the exclude list
                SvGlobalName aName;

                svt::EmbeddedObjectRef& xObj = pOLENd->GetOLEObj().GetObject();
                if ( xObj.is() )
                    aName = SvGlobalName( xObj->getClassID() );
                else  // not yet loaded
                {
                        // TODO/LATER: retrieve ClassID of an unloaded object
                        // aName = ????
                }

                sal_Bool bFound = sal_False;
                for ( sal_uInt16 j = 0;
                      j < pGlobalOLEExcludeList->Count() && !bFound;
                      ++j )
                {
                    bFound = *(SvGlobalName*)(*pGlobalOLEExcludeList)[j] ==
                                    aName;
                }
                if ( bFound )
                    continue;

                // not known, therefore the object has to be loaded
                // when notification is not requested
                if ( xObj.is() )
                {
                    //TODO/LATER: needs MiscStatus for ResizeOnPrinterChange
                    /*
                    if ( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & xRef->GetMiscStatus())
                    {
                        if ( pOLENd->GetFrm() )
                        {
                            xObj->OnDocumentPrinterChanged( pPrt );
                            pShell->CalcAndSetScale( xObj );// create client
                        }
                        else
                            pOLENd->SetOLESizeInvalid( sal_True );
                    }
                    else */
                        pGlobalOLEExcludeList->Insert(
                                new SvGlobalName( aName ),
                                pGlobalOLEExcludeList->Count() );
                }
            }
            delete pNodes;
            GetCurrentLayout()->EndAllAction(); //swmod 080218
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
                             0, pNodes->Count(), GetDocShell());
            GetCurrentLayout()->StartAllAction();   //swmod 080218
            SwMsgPoolItem aMsgHint( RES_UPDATE_ATTR );

            for( sal_uInt16 i = 0; i < pNodes->Count(); ++i )
            {
                ::SetProgressState( i, GetDocShell() );

                SwOLENode* pOLENd = (*pNodes)[i];
                pOLENd->SetOLESizeInvalid( sal_False );

                // not known, therefore the object has to be loaded
                // when notification is not requested
                if( pOLENd->GetOLEObj().GetOleRef().is() ) // broken?
                {
                    //TODO/LATER: needs MiscStatus for ResizeOnPrinterChange
                    /*
                    if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE &
                            xRef->GetMiscStatus() )
                    {
                        if( pOLENd->GetFrm() )
                        {
                            xRef->OnDocumentPrinterChanged( pPrt );
                            pSh->CalcAndSetScale( xRef );// create client
                        }
                        else
                            pOLENd->SetOLESizeInvalid( sal_True );
                    }*/
                    // repaint it
                    pOLENd->ModifyNotification( &aMsgHint, &aMsgHint );
                }
            }
            GetCurrentLayout()->EndAllAction(); //swmod 080218
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
    for (nI = 0; nI < aPageDescs.Count(); nI++)
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

void SwDoc::DelPageDesc( const String & rName, sal_Bool bBroadcast ) // #116530#
{
    sal_uInt16 nI;

    if (FindPageDesc(rName, &nI))
        DelPageDesc(nI, bBroadcast); // #116530#
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
        SwPageDesc& rDesc = _GetPageDesc( i );

        SwFrmFmt& rMaster = rDesc.GetMaster();
        SwFrmFmt& rLeft   = rDesc.GetLeft();

        const SwFmtFrmSize& rMasterSize  = rMaster.GetFrmSize();
        const SwFmtFrmSize& rLeftSize    = rLeft.GetFrmSize();

        const bool bSetSize = LONG_MAX == rMasterSize.GetWidth() ||
                              LONG_MAX == rMasterSize.GetHeight() ||
                              LONG_MAX == rLeftSize.GetWidth() ||
                              LONG_MAX == rLeftSize.GetHeight();

        if ( bSetSize )
            lcl_DefaultPageFmt( rDesc.GetPoolFmtId(), rDesc.GetMaster(), rDesc.GetLeft() );
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
        SwPageDesc& rDesc = _GetPageDesc( i );

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
