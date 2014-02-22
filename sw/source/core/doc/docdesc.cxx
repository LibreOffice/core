/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <cmdid.h>
#include <svx/svdmodel.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/paperinf.hxx>
#include "editeng/frmdiritem.hxx"
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
    
    
    
    

    SwFmtFrmSize aFrmSize( ATT_FIX_SIZE );
    const Size aPhysSize = SvxPaperInfo::GetDefaultPaperSize();
    aFrmSize.SetSize( aPhysSize );

    
    
    
    
    
    
    
    
    
    sal_Int32 nMinTop, nMinBottom, nMinLeft, nMinRight;
    if( RES_POOLPAGE_HTML == nPoolFmtId )
    {
        nMinRight = nMinTop = nMinBottom = GetMetricVal( CM_1 );
        nMinLeft = nMinRight * 2;
    }
    else if( MEASURE_METRIC == SvtSysLocale().GetLocaleData().getMeasurementSystemEnum() )
    {
        nMinTop = nMinBottom = nMinLeft = nMinRight = 1134; 
    }
    else
    {
        nMinTop = nMinBottom = 1440;    
        nMinLeft = nMinRight = 1800;    
    }

    
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
    
    

    
    sal_uInt16 const aIdArr[] = { RES_FRM_SIZE, RES_UL_SPACE,
                                        RES_BACKGROUND, RES_SHADOW,
                                        RES_COL, RES_COL,
                                        RES_FRAMEDIR, RES_FRAMEDIR,
                                        RES_TEXTGRID, RES_TEXTGRID,
                                        
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

    
    rDest.SetPoolFmtId( rSource.GetPoolFmtId() );
    rDest.SetPoolHelpId( rSource.GetPoolHelpId() );
    rDest.SetPoolHlpFileId( rSource.GetPoolHlpFileId() );
}

void SwDoc::CopyMasterHeader(const SwPageDesc &rChged, const SwFmtHeader &rHead, SwPageDesc *pDesc, bool bLeft, bool bFirst)
{
    assert(bLeft || bFirst);
    SwFrmFmt& rDescFrmFmt = (bFirst)
            ? (bLeft) ? pDesc->GetFirstLeft() : pDesc->GetFirstMaster()
            : pDesc->GetLeft();
    if (bFirst && bLeft)
    {
        
        rDescFrmFmt.SetFmtAttr( rChged.IsFirstShared()
                ? pDesc->GetLeft().GetHeader()
                : pDesc->GetFirstMaster().GetHeader());
    }
    else if ((bFirst ? rChged.IsFirstShared() : rChged.IsHeaderShared())
         || !rHead.IsActive())
    {
        
        rDescFrmFmt.SetFmtAttr( pDesc->GetMaster().GetHeader() );
    }
    else if ( rHead.IsActive() )
    {   
        
        
        
        const SwFmtHeader &rFmtHead = rDescFrmFmt.GetHeader();
        if ( !rFmtHead.IsActive() )
        {
            SwFmtHeader aHead( MakeLayoutFmt( RND_STD_HEADERL, 0 ) );
            rDescFrmFmt.SetFmtAttr( aHead );
            
            ::lcl_DescSetAttr( *rHead.GetHeaderFmt(), *aHead.GetHeaderFmt(), false);
        }
        else
        {
            const SwFrmFmt *pRight = rHead.GetHeaderFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aCnt = rFmtHead.GetHeaderFmt()->GetCntnt();

            if (!aCnt.GetCntntIdx())
            {
                const SwFrmFmt& rChgedFrmFmt = (bFirst)
                    ? (bLeft) ? rChged.GetFirstLeft() : rChged.GetFirstMaster()
                    : rChged.GetLeft();
                rDescFrmFmt.SetFmtAttr( rChgedFrmFmt.GetHeader() );
            }
            else if ((*aRCnt.GetCntntIdx() == *aCnt.GetCntntIdx()) ||
                
                
                
                 ((bFirst) ? pDesc->IsFirstShared() : pDesc->IsHeaderShared()))
            {
                SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(),
                        (bFirst) ? "First header" : "Left header",
                                                GetDfltFrmFmt() );
                ::lcl_DescSetAttr( *pRight, *pFmt, false );
                
                
                
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwHeaderStartNode );
                SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
                            *aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes()._Copy( aRange, aTmp, sal_False );
                aTmp = *pSttNd;
                CopyFlyInFlyImpl(aRange, 0, aTmp);

                pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ) );
                rDescFrmFmt.SetFmtAttr( SwFmtHeader( pFmt ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *(SwFrmFmt*)rFmtHead.GetHeaderFmt(), false );
        }
    }
}

void SwDoc::CopyMasterFooter(const SwPageDesc &rChged, const SwFmtFooter &rFoot, SwPageDesc *pDesc, bool bLeft, bool bFirst)
{
    assert(bLeft || bFirst);
    SwFrmFmt& rDescFrmFmt = (bFirst)
            ? (bLeft) ? pDesc->GetFirstLeft() : pDesc->GetFirstMaster()
            : pDesc->GetLeft();
    if (bFirst && bLeft)
    {
        
        rDescFrmFmt.SetFmtAttr( rChged.IsFirstShared()
                ? pDesc->GetLeft().GetFooter()
                : pDesc->GetFirstMaster().GetFooter());
    }
    else if ((bFirst ? rChged.IsFirstShared() : rChged.IsFooterShared())
        || !rFoot.IsActive())
    {
        
        rDescFrmFmt.SetFmtAttr( pDesc->GetMaster().GetFooter() );
    }
    else if ( rFoot.IsActive() )
    {   
        
        
        
        const SwFmtFooter &rFmtFoot = rDescFrmFmt.GetFooter();
        if ( !rFmtFoot.IsActive() )
        {
            SwFmtFooter aFoot( MakeLayoutFmt( RND_STD_FOOTER, 0 ) );
            rDescFrmFmt.SetFmtAttr( aFoot );
            
            ::lcl_DescSetAttr( *rFoot.GetFooterFmt(), *aFoot.GetFooterFmt(), false);
        }
        else
        {
            const SwFrmFmt *pRight = rFoot.GetFooterFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aLCnt = rFmtFoot.GetFooterFmt()->GetCntnt();
            if( !aLCnt.GetCntntIdx() )
            {
                const SwFrmFmt& rChgedFrmFmt = (bFirst)
                    ? (bLeft) ? rChged.GetFirstLeft() : rChged.GetFirstMaster()
                    : rChged.GetLeft();
                rDescFrmFmt.SetFmtAttr( rChgedFrmFmt.GetFooter() );
            }
            else if ((*aRCnt.GetCntntIdx() == *aLCnt.GetCntntIdx()) ||
                
                
                
                 ((bFirst) ? pDesc->IsFirstShared() : pDesc->IsFooterShared()))
            {
                SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(),
                        (bFirst) ? "First footer" : "Left footer",
                                                GetDfltFrmFmt() );
                ::lcl_DescSetAttr( *pRight, *pFmt, false );
                
                
                
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwFooterStartNode );
                SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
                            *aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes()._Copy( aRange, aTmp, sal_False );
                aTmp = *pSttNd;
                CopyFlyInFlyImpl(aRange, 0, aTmp);

                pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ) );
                rDescFrmFmt.SetFmtAttr( SwFmtFooter( pFmt ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *(SwFrmFmt*)rFmtFoot.GetFooterFmt(), false );
        }
    }
}

void SwDoc::ChgPageDesc( sal_uInt16 i, const SwPageDesc &rChged )
{
    OSL_ENSURE( i < maPageDescs.size(), "PageDescs is out of range." );

    SwPageDesc *pDesc = maPageDescs[i];
    SwRootFrm* pTmpRoot = GetCurrentLayout();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo *const pUndo(new SwUndoPageDesc(*pDesc, rChged, this));
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    
    if ( rChged.GetUseOn() == nsUseOnPage::PD_MIRROR )
        ((SwPageDesc&)rChged).Mirror();
    else
    {
        
        ::lcl_DescSetAttr(rChged.GetMaster(),
                   const_cast<SwPageDesc&>(rChged).GetLeft());
    }
    ::lcl_DescSetAttr(rChged.GetMaster(),
                   const_cast<SwPageDesc&>(rChged).GetFirstMaster());
    ::lcl_DescSetAttr(rChged.GetLeft(),
                   const_cast<SwPageDesc&>(rChged).GetFirstLeft());

    
    if( rChged.GetNumType().GetNumberingType() != pDesc->GetNumType().GetNumberingType() )
    {
        pDesc->SetNumType( rChged.GetNumType() );
        
        GetSysFldType( RES_PAGENUMBERFLD )->UpdateFlds();
        GetSysFldType( RES_REFPAGEGETFLD )->UpdateFlds();

        
        
        SwFtnIdxs& rFtnIdxs = GetFtnIdxs();
        for( sal_uInt16 nPos = 0; nPos < rFtnIdxs.size(); ++nPos )
        {
            SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
            const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
            pTxtFtn->SetNumber(rFtn.GetNumber(), rFtn.GetNumStr());
        }
    }

    
    pDesc->SetLandscape( rChged.GetLandscape() );

    
    bool bHeaderFooterChanged = false;

    
    const SwFmtHeader &rHead = rChged.GetMaster().GetHeader();
    if (undoGuard.UndoWasEnabled())
    {
        
        
        const SwFmtHeader &rOldHead = pDesc->GetMaster().GetHeader();
        bHeaderFooterChanged |=
            ( rHead.IsActive() != rOldHead.IsActive() ||
              rChged.IsHeaderShared() != pDesc->IsHeaderShared() ||
              rChged.IsFirstShared() != pDesc->IsFirstShared() );
    }
    pDesc->GetMaster().SetFmtAttr( rHead );
    CopyMasterHeader(rChged, rHead, pDesc, true, false); 
    CopyMasterHeader(rChged, rHead, pDesc, false, true); 
    CopyMasterHeader(rChged, rHead, pDesc, true, true);  
    pDesc->ChgHeaderShare( rChged.IsHeaderShared() );

    
    const SwFmtFooter &rFoot = rChged.GetMaster().GetFooter();
    if (undoGuard.UndoWasEnabled())
    {
        
        
        const SwFmtFooter &rOldFoot = pDesc->GetMaster().GetFooter();
        bHeaderFooterChanged |=
            ( rFoot.IsActive() != rOldFoot.IsActive() ||
              rChged.IsFooterShared() != pDesc->IsFooterShared() );
    }
    pDesc->GetMaster().SetFmtAttr( rFoot );
    CopyMasterFooter(rChged, rFoot, pDesc, true, false); 
    CopyMasterFooter(rChged, rFoot, pDesc, false, true); 
    CopyMasterFooter(rChged, rFoot, pDesc, true, true);  
    pDesc->ChgFooterShare( rChged.IsFooterShared() );
    
    pDesc->ChgFirstShare( rChged.IsFirstShared() );

    if ( pDesc->GetName() != rChged.GetName() )
        pDesc->SetName( rChged.GetName() );

    
    pDesc->SetRegisterFmtColl( rChged.GetRegisterFmtColl() );

    
    bool bUseOn  = false;
    bool bFollow = false;
    if ( pDesc->GetUseOn() != rChged.GetUseOn() )
    {   pDesc->SetUseOn( rChged.GetUseOn() );
        bUseOn = true;
    }
    if ( pDesc->GetFollow() != rChged.GetFollow() )
    {   if ( rChged.GetFollow() == &rChged )
        {   if ( pDesc->GetFollow() != pDesc )
            {   pDesc->SetFollow( pDesc );
                bFollow = true;
            }
        }
        else
        {   pDesc->SetFollow( rChged.pFollow );
            bFollow = true;
        }
    }

    if ( (bUseOn || bFollow) && pTmpRoot)
        
    {
        std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllCheckPageDescs));
    }

    
    ::lcl_DescSetAttr( rChged.GetMaster(), pDesc->GetMaster() );
    ::lcl_DescSetAttr( rChged.GetLeft(), pDesc->GetLeft() );
    ::lcl_DescSetAttr( rChged.GetFirstMaster(), pDesc->GetFirstMaster() );
    ::lcl_DescSetAttr( rChged.GetFirstLeft(), pDesc->GetFirstLeft() );

    
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
            pDesc->GetFirstMaster().ModifyBroadcast( &aInfo, 0, TYPE(SwFrm) );
        }
        {
            pDesc->GetFirstLeft().ModifyBroadcast( &aInfo, 0, TYPE(SwFrm) );
        }
    }
    SetModified();

    
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

    
    assert((pDesc->IsFirstShared())
        ? pDesc->GetFirstLeft().GetHeader().GetHeaderFmt() == pDesc->GetLeft().GetHeader().GetHeaderFmt()
        : pDesc->GetFirstLeft().GetHeader().GetHeaderFmt() == pDesc->GetFirstMaster().GetHeader().GetHeaderFmt());
    assert((pDesc->IsFirstShared())
        ? pDesc->GetFirstLeft().GetFooter().GetFooterFmt() == pDesc->GetLeft().GetFooter().GetFooterFmt()
        : pDesc->GetFirstLeft().GetFooter().GetFooterFmt() == pDesc->GetFirstMaster().GetFooter().GetFooterFmt());
}



void SwDoc::PreDelPageDesc(SwPageDesc * pDel)
{
    if (0 == pDel)
        return;

    
    SwPageDescHint aHint( maPageDescs[0] );
    pDel->CallSwClientNotify( aHint );

    bool bHasLayout = HasLayout();
    if ( mpFtnInfo->DependsOn( pDel ) )
    {
        mpFtnInfo->ChgPageDesc( maPageDescs[0] );
        if ( bHasLayout )
        {
            std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::CheckFtnPageDescs), false));
        }
    }
    else if ( mpEndNoteInfo->DependsOn( pDel ) )
    {
        mpEndNoteInfo->ChgPageDesc( maPageDescs[0] );
        if ( bHasLayout )
        {
            std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::CheckFtnPageDescs), true));
        }
    }

    for ( sal_uInt16 j = 0; j < maPageDescs.size(); ++j )
    {
        if ( maPageDescs[j]->GetFollow() == pDel )
        {
            maPageDescs[j]->SetFollow( 0 );
            if( bHasLayout )
            {
                std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
                std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllCheckPageDescs));
            }
        }
    }
}

void SwDoc::BroadcastStyleOperation(OUString rName, SfxStyleFamily eFamily,
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

    SwPageDesc *pDel = maPageDescs[i];

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SFX_STYLE_FAMILY_PAGE,
                                SFX_STYLESHEET_ERASED);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo *const pUndo(new SwUndoPageDescDelete(*pDel, this));
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    PreDelPageDesc(pDel); 

    maPageDescs.erase( maPageDescs.begin() + i );
    delete pDel;
    SetModified();
}

sal_uInt16 SwDoc::MakePageDesc( const OUString &rName, const SwPageDesc *pCpy,
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
                                SFX_STYLESHEET_CREATED);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(new SwUndoPageDescCreate(pNew, this));
    }

    SetModified();
    return (maPageDescs.size()-1);
}

SwPageDesc* SwDoc::FindPageDescByName( const OUString& rName, sal_uInt16* pPos ) const
{
    SwPageDesc* pRet = 0;
    if( pPos ) *pPos = USHRT_MAX;

    for( sal_uInt16 n = 0, nEnd = maPageDescs.size(); n < nEnd; ++n )
        if( maPageDescs[ n ]->GetName() == rName )
        {
            pRet = maPageDescs[ n ];
            if( pPos )
                *pPos = n;
            break;
        }
    return pRet;
}

void SwDoc::PrtDataChanged()
{


    
    OSL_ENSURE( get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) ||
            0 != getPrinter( false ), "PrtDataChanged will be called recursively!" );
    SwRootFrm* pTmpRoot = GetCurrentLayout();
    boost::scoped_ptr<SwWait> pWait;
    bool bEndAction = false;

    if( GetDocShell() )
        GetDocShell()->UpdateFontList();

    bool bDraw = true;
    if ( pTmpRoot )
    {
        SwViewShell *pSh = GetCurrentViewShell();
        if( pSh &&
            (!pSh->GetViewOptions()->getBrowseMode() ||
             pSh->GetViewOptions()->IsPrtFormat()) )
        {
            if ( GetDocShell() )
                pWait.reset(new SwWait( *GetDocShell(), true ));

            pTmpRoot->StartAllAction();
            bEndAction = true;

            bDraw = false;
            if( mpDrawModel )
            {
                mpDrawModel->SetAddExtLeading( get(IDocumentSettingAccess::ADD_EXT_LEADING) );
                mpDrawModel->SetRefDevice( getReferenceDevice( false ) );
            }

            pFntCache->Flush();

            std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::InvalidateAllCntnt), INV_SIZE));

            do
            {
                pSh->InitPrt( mpPrt );
                pSh = (SwViewShell*)pSh->GetNext();
            }
            while ( pSh != GetCurrentViewShell() );
        }
    }
    if ( bDraw && mpDrawModel )
    {
        const sal_Bool bTmpAddExtLeading = get(IDocumentSettingAccess::ADD_EXT_LEADING);
        if ( bTmpAddExtLeading != mpDrawModel->IsAddExtLeading() )
            mpDrawModel->SetAddExtLeading( bTmpAddExtLeading );

        OutputDevice* pOutDev = getReferenceDevice( false );
        if ( pOutDev != mpDrawModel->GetRefDevice() )
            mpDrawModel->SetRefDevice( pOutDev );
    }

    PrtOLENotify( sal_True );

    if ( bEndAction )
        pTmpRoot->EndAllAction();
}





extern std::vector<SvGlobalName*> *pGlobalOLEExcludeList;

void SwDoc::PrtOLENotify( sal_Bool bAll )
{
    SwFEShell *pShell = 0;
    if ( GetCurrentViewShell() )
    {
        SwViewShell *pSh = GetCurrentViewShell();
        if ( !pSh->ISA(SwFEShell) )
            do
            {   pSh = (SwViewShell*)pSh->GetNext();
            } while ( !pSh->ISA(SwFEShell) &&
                      pSh != GetCurrentViewShell() );

        if ( pSh->ISA(SwFEShell) )
            pShell = (SwFEShell*)pSh;
    }
    if ( !pShell )
    {
        
        
        
        
        
        mbOLEPrtNotifyPending = true;
        if ( bAll )
            mbAllOLENotify = true;
    }
    else
    {
        if ( mbAllOLENotify )
            bAll = sal_True;

        mbOLEPrtNotifyPending = mbAllOLENotify = false;

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

                
                SvGlobalName aName;

                svt::EmbeddedObjectRef& xObj = pOLENd->GetOLEObj().GetObject();
                if ( xObj.is() )
                    aName = SvGlobalName( xObj->getClassID() );
                else  
                {
                        
                        
                }

                bool bFound = false;
                for ( sal_uInt16 j = 0;
                      j < pGlobalOLEExcludeList->size() && !bFound;
                      ++j )
                {
                    bFound = *(*pGlobalOLEExcludeList)[j] == aName;
                }
                if ( bFound )
                    continue;

                
                
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
        mbOLEPrtNotifyPending = mbAllOLENotify = false;

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

                
                
                if( pOLENd->GetOLEObj().GetOleRef().is() ) 
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

bool SwDoc::FindPageDesc( const OUString & rName, sal_uInt16 * pFound)
{
    bool bResult = false;
    sal_uInt16 nI;
    for (nI = 0; nI < maPageDescs.size(); nI++)
    {
        if (maPageDescs[nI]->GetName() == rName)
        {
            *pFound = nI;
            bResult = true;
            break;
        }
    }

    return bResult;
}

SwPageDesc * SwDoc::GetPageDesc( const OUString & rName )
{
    SwPageDesc * aResult = NULL;

    sal_uInt16 nI;

    if (FindPageDesc(rName, &nI))
        aResult = maPageDescs[nI];

    return aResult;
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
