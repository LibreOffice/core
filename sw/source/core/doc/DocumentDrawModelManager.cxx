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

#include <DocumentDrawModelManager.hxx>

#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <swtypes.hxx>
#include <ndtxt.hxx>
#include <swhints.hxx>
#include <viewsh.hxx>
#include <view.hxx>
#include <drawdoc.hxx>
#include <rootfrm.hxx>
#include <fmtanchr.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdotext.hxx>
#include <svl/smplhint.hxx>
#include <svl/srchitem.hxx>
#include <tools/link.hxx>
#include <unotools/configmgr.hxx>

class SdrOutliner;
class XSpellChecker1;

namespace sw
{

DocumentDrawModelManager::DocumentDrawModelManager(SwDoc& i_rSwdoc)
    : m_rDoc(i_rSwdoc)
    , mpDrawModel(0)
    , mnHeaven(0)
    , mnHell(0)
    , mnControls(0)
    , mnInvisibleHeaven(0)
    , mnInvisibleHell(0)
    , mnInvisibleControls(0)
{
}

// Is also called by the Sw3 Reader, if there was an error when reading the
// drawing layer. If it is called by the Sw3 Reader the layer is rebuilt
// from scratch.
void DocumentDrawModelManager::InitDrawModel()
{
    // !! Attention: there is similar code in the Sw3 Reader (sw3imp.cxx) that
    // also has to be maintained!!
    if ( mpDrawModel )
        ReleaseDrawModel();

//UUUU
//  // Setup DrawPool and EditEnginePool. Ownership is ours and only gets passed
//  // to the Drawing.
//  // The pools are destroyed in the ReleaseDrawModel.
//  // for loading the drawing items. This must be loaded without RefCounts!
//  SfxItemPool *pSdrPool = new SdrItemPool( &GetAttrPool() );
//  // change DefaultItems for the SdrEdgeObj distance items to TWIPS.
//  if(pSdrPool)
//  {
//      const long nDefEdgeDist = ((500 * 72) / 127); // 1/100th mm in twips
//      pSdrPool->SetPoolDefaultItem(SdrEdgeNode1HorzDistItem(nDefEdgeDist));
//      pSdrPool->SetPoolDefaultItem(SdrEdgeNode1VertDistItem(nDefEdgeDist));
//      pSdrPool->SetPoolDefaultItem(SdrEdgeNode2HorzDistItem(nDefEdgeDist));
//      pSdrPool->SetPoolDefaultItem(SdrEdgeNode2VertDistItem(nDefEdgeDist));
//
//      // #i33700#
//      // Set shadow distance defaults as PoolDefaultItems. Details see bug.
//      pSdrPool->SetPoolDefaultItem(makeSdrShadowXDistItem((300 * 72) / 127));
//      pSdrPool->SetPoolDefaultItem(makeSdrShadowYDistItem((300 * 72) / 127));
//  }
//  SfxItemPool *pEEgPool = EditEngine::CreatePool( false );
//  pSdrPool->SetSecondaryPool( pEEgPool );
//   if ( !GetAttrPool().GetFrozenIdRanges () )
//      GetAttrPool().FreezeIdRanges();
//  else
//      pSdrPool->FreezeIdRanges();

    // set FontHeight pool defaults without changing static SdrEngineDefaults
    m_rDoc.GetAttrPool().SetPoolDefaultItem(SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT ));

    SAL_INFO( "sw.doc", "before create DrawDocument" );
    // The document owns the SwDrawModel. We always have two layers and one page.
    mpDrawModel = new SwDrawModel( &m_rDoc );

    mpDrawModel->EnableUndo( m_rDoc.GetIDocumentUndoRedo().DoesUndo() );

    OUString sLayerNm;
    sLayerNm = "Hell";
    mnHell   = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    sLayerNm = "Heaven";
    mnHeaven = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    sLayerNm = "Controls";
    mnControls = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    // add invisible layers corresponding to the visible ones.
    {
        sLayerNm = "InvisibleHell";
        mnInvisibleHell   = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

        sLayerNm = "InvisibleHeaven";
        mnInvisibleHeaven = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

        sLayerNm = "InvisibleControls";
        mnInvisibleControls = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
    }

    SdrPage* pMasterPage = mpDrawModel->AllocPage( false );
    mpDrawModel->InsertPage( pMasterPage );
    SAL_INFO( "sw.doc", "after create DrawDocument" );
    SdrOutliner& rOutliner = mpDrawModel->GetDrawOutliner();
    if (!utl::ConfigManager::IsAvoidConfig())
    {
        SAL_INFO( "sw.doc", "before create Spellchecker/Hyphenator" );
        css::uno::Reference< css::linguistic2::XSpellChecker1 > xSpell = ::GetSpellChecker();
        rOutliner.SetSpeller( xSpell );
        css::uno::Reference< css::linguistic2::XHyphenator > xHyphenator( ::GetHyphenator() );
        rOutliner.SetHyphenator( xHyphenator );
        SAL_INFO( "sw.doc", "after create Spellchecker/Hyphenator" );
    }
    m_rDoc.SetCalcFieldValueHdl(&rOutliner);
    m_rDoc.SetCalcFieldValueHdl(&mpDrawModel->GetHitTestOutliner());

    // Set the LinkManager in the model so that linked graphics can be inserted.
    // The WinWord import needs it too.
    mpDrawModel->SetLinkManager( & m_rDoc.getIDocumentLinksAdministration().GetLinkManager() );
    mpDrawModel->SetAddExtLeading( m_rDoc.getIDocumentSettingAccess().get(DocumentSettingId::ADD_EXT_LEADING) );

    OutputDevice* pRefDev = m_rDoc.getIDocumentDeviceAccess().getReferenceDevice( false );
    if ( pRefDev )
        mpDrawModel->SetRefDevice( pRefDev );

    mpDrawModel->SetNotifyUndoActionHdl( LINK( &m_rDoc, SwDoc, AddDrawUndo ));
    SwViewShell* const pSh = m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell();
    if ( pSh )
    {
        for(SwViewShell& rViewSh : pSh->GetRingContainer())
        {
            SwRootFrm* pRoot =  rViewSh.GetLayout();
            if( pRoot && !pRoot->GetDrawPage() )
            {
                // Disable "multiple layout" for the moment:
                // use pMasterPage instead of a new created SdrPage
                // mpDrawModel->AllocPage( FALSE );
                // mpDrawModel->InsertPage( pDrawPage );
                SdrPage* pDrawPage = pMasterPage;
                pRoot->SetDrawPage( pDrawPage );
                pDrawPage->SetSize( pRoot->Frm().SSize() );
            }
        }
    }
}


void DocumentDrawModelManager::ReleaseDrawModel()
{
    if ( mpDrawModel )
    {
        // !! Also maintain the code in the sw3io for inserting documents!!

        delete mpDrawModel; mpDrawModel = 0;
//UUUU
//      SfxItemPool *pSdrPool = GetAttrPool().GetSecondaryPool();
//
//      OSL_ENSURE( pSdrPool, "missing pool" );
//      SfxItemPool *pEEgPool = pSdrPool->GetSecondaryPool();
//      OSL_ENSURE( !pEEgPool->GetSecondaryPool(), "I don't accept additional pools");
//      pSdrPool->Delete();                 // First have the items destroyed,
//                                          // then destroy the chain!
//      GetAttrPool().SetSecondaryPool( 0 );    // This one's a must!
//      pSdrPool->SetSecondaryPool( 0 );    // That one's safer
//      SfxItemPool::Free(pSdrPool);
//      SfxItemPool::Free(pEEgPool);
    }
}





const SwDrawModel* DocumentDrawModelManager::GetDrawModel() const
{
    return mpDrawModel;
}

SwDrawModel* DocumentDrawModelManager::GetDrawModel()
{
    return mpDrawModel;
}

SwDrawModel* DocumentDrawModelManager::_MakeDrawModel()
{
    OSL_ENSURE( !mpDrawModel, "_MakeDrawModel: Why?" );
    InitDrawModel();
    SwViewShell* const pSh = m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell();
    if ( pSh )
    {
        for(SwViewShell& rViewSh : pSh->GetRingContainer())
            rViewSh.MakeDrawView();

        // Broadcast, so that the FormShell can be connected to the DrawView
        if( m_rDoc.GetDocShell() )
        {
            SfxSimpleHint aHint( SW_BROADCAST_DRAWVIEWS_CREATED );
            m_rDoc.GetDocShell()->Broadcast( aHint );
        }
    }
    return mpDrawModel;
}

SwDrawModel* DocumentDrawModelManager::GetOrCreateDrawModel()
{
    return GetDrawModel() ? GetDrawModel() : _MakeDrawModel();
}

SdrLayerID DocumentDrawModelManager::GetHeavenId() const
{
    return mnHeaven;
}

SdrLayerID DocumentDrawModelManager::GetHellId() const
{
    return mnHell;
}

SdrLayerID DocumentDrawModelManager::GetControlsId() const
{
    return mnControls;
}

SdrLayerID DocumentDrawModelManager::GetInvisibleHeavenId() const
{
    return mnInvisibleHeaven;
}

SdrLayerID DocumentDrawModelManager::GetInvisibleHellId() const
{
    return mnInvisibleHell;
}

SdrLayerID DocumentDrawModelManager::GetInvisibleControlsId() const
{
    return mnInvisibleControls;
}

void DocumentDrawModelManager::NotifyInvisibleLayers( SdrPageView& _rSdrPageView )
{
    OUString sLayerNm;
    sLayerNm = "InvisibleHell";
    _rSdrPageView.SetLayerVisible( sLayerNm, false );

    sLayerNm = "InvisibleHeaven";
    _rSdrPageView.SetLayerVisible( sLayerNm, false );

    sLayerNm = "InvisibleControls";
    _rSdrPageView.SetLayerVisible( sLayerNm, false );
}

bool DocumentDrawModelManager::IsVisibleLayerId( const SdrLayerID& _nLayerId ) const
{
    bool bRetVal;

    if ( _nLayerId == GetHeavenId() ||
         _nLayerId == GetHellId() ||
         _nLayerId == GetControlsId() )
    {
        bRetVal = true;
    }
    else if ( _nLayerId == GetInvisibleHeavenId() ||
              _nLayerId == GetInvisibleHellId() ||
              _nLayerId == GetInvisibleControlsId() )
    {
        bRetVal = false;
    }
    else
    {
        OSL_FAIL( "<SwDoc::IsVisibleLayerId(..)> - unknown layer ID." );
        bRetVal = false;
    }

    return bRetVal;
}

SdrLayerID DocumentDrawModelManager::GetInvisibleLayerIdByVisibleOne( const SdrLayerID& _nVisibleLayerId )
{
    SdrLayerID nInvisibleLayerId;

    if ( _nVisibleLayerId == GetHeavenId() )
    {
        nInvisibleLayerId = GetInvisibleHeavenId();
    }
    else if ( _nVisibleLayerId == GetHellId() )
    {
        nInvisibleLayerId = GetInvisibleHellId();
    }
    else if ( _nVisibleLayerId == GetControlsId() )
    {
        nInvisibleLayerId = GetInvisibleControlsId();
    }
    else if ( _nVisibleLayerId == GetInvisibleHeavenId() ||
              _nVisibleLayerId == GetInvisibleHellId() ||
              _nVisibleLayerId == GetInvisibleControlsId() )
    {
        OSL_FAIL( "<SwDoc::GetInvisibleLayerIdByVisibleOne(..)> - given layer ID already an invisible one." );
        nInvisibleLayerId = _nVisibleLayerId;
    }
    else
    {
        OSL_FAIL( "<SwDoc::GetInvisibleLayerIdByVisibleOne(..)> - given layer ID is unknown." );
        nInvisibleLayerId = _nVisibleLayerId;
    }

    return nInvisibleLayerId;
}

bool DocumentDrawModelManager::Search(const SwPaM& rPaM, const SvxSearchItem& rSearchItem)
{
    SwPosFlyFrms aFrames = m_rDoc.GetAllFlyFormats(&rPaM, /*bDrawAlso=*/true);

    for (const SwPosFlyFrmPtr& pPosFlyFrm : aFrames)
    {
        // Filter for at-paragraph anchored draw frames.
        const SwFrameFormat& rFrameFormat = pPosFlyFrm->GetFormat();
        const SwFormatAnchor& rAnchor = rFrameFormat.GetAnchor();
        if (rAnchor.GetAnchorId() != FLY_AT_PARA || rFrameFormat.Which() != RES_DRAWFRMFMT)
            continue;

        // Does the shape have matching text?
        SdrOutliner& rOutliner = GetDrawModel()->GetDrawOutliner();
        SdrObject* pObject = const_cast<SdrObject*>(rFrameFormat.FindSdrObject());
        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>(pObject);
        if (!pTextObj)
            continue;
        const OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
        if (!pParaObj)
            continue;
        rOutliner.SetText(*pParaObj);
        SwDocShell* pDocShell = m_rDoc.GetDocShell();
        if (!pDocShell)
            return false;
        SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
        if (!pWrtShell)
            return false;
        if (!rOutliner.HasText(rSearchItem))
            continue;

        // If so, then select highlight the search result.
        pWrtShell->SelectObj(Point(), 0, pObject);
        SwView* pView = pDocShell->GetView();
        if (!pView)
            return false;
        if (!pView->EnterShapeDrawTextMode(pObject))
            continue;
        SdrView* pSdrView = pWrtShell->GetDrawView();
        if (!pSdrView)
            return false;
        OutlinerView* pOutlinerView = pSdrView->GetTextEditOutlinerView();
        if (!rSearchItem.GetBackward())
            pOutlinerView->SetSelection(ESelection(0, 0, 0, 0));
        else
            pOutlinerView->SetSelection(ESelection(EE_PARA_MAX_COUNT, EE_TEXTPOS_MAX_COUNT, EE_PARA_MAX_COUNT, EE_TEXTPOS_MAX_COUNT));
        pOutlinerView->StartSearchAndReplace(rSearchItem);
        return true;
    }

    return false;
}

void DocumentDrawModelManager::DrawNotifyUndoHdl()
{
    mpDrawModel->SetNotifyUndoActionHdl( Link<SdrUndoAction*,void>() );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
