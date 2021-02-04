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
#include <svl/hint.hxx>
#include <viewsh.hxx>
#include <view.hxx>
#include <drawdoc.hxx>
#include <rootfrm.hxx>
#include <fmtanchr.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdview.hxx>
#include <svl/srchitem.hxx>
#include <unotools/configmgr.hxx>
#include <sal/log.hxx>

class SdrOutliner;

namespace sw
{

DocumentDrawModelManager::DocumentDrawModelManager(SwDoc& i_rSwdoc)
    : m_rDoc(i_rSwdoc)
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

    // set FontHeight pool defaults without changing static SdrEngineDefaults
    m_rDoc.GetAttrPool().SetPoolDefaultItem(SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT ));

    SAL_INFO( "sw.doc", "before create DrawDocument" );
    // The document owns the SwDrawModel. We always have two layers and one page.
    mpDrawModel.reset(new SwDrawModel(m_rDoc));

    mpDrawModel->EnableUndo( m_rDoc.GetIDocumentUndoRedo().DoesUndo() );

    OUString sLayerNm;
    sLayerNm = "Hell";
    mnHell   = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    sLayerNm = "Heaven";
    mnHeaven = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    sLayerNm = "Controls";
    mnControls = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
    mpDrawModel->GetLayerAdmin().SetControlLayerName(sLayerNm);

    // add invisible layers corresponding to the visible ones.
    {
        sLayerNm = "InvisibleHell";
        mnInvisibleHell   = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

        sLayerNm = "InvisibleHeaven";
        mnInvisibleHeaven = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

        sLayerNm = "InvisibleControls";
        mnInvisibleControls = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
    }

    rtl::Reference<SdrPage> pMasterPage = mpDrawModel->AllocPage( false );
    mpDrawModel->InsertPage( pMasterPage.get() );
    SAL_INFO( "sw.doc", "after create DrawDocument" );
    SdrOutliner& rOutliner = mpDrawModel->GetDrawOutliner();
    if (!utl::ConfigManager::IsFuzzing())
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

    mpDrawModel->SetNotifyUndoActionHdl( std::bind( &SwDoc::AddDrawUndo, &m_rDoc, std::placeholders::_1 ));
    SwViewShell* const pSh = m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell();
    if ( !pSh )
        return;

    for(const SwViewShell& rViewSh : pSh->GetRingContainer())
    {
        SwRootFrame* pRoot =  rViewSh.GetLayout();
        if( pRoot && !pRoot->GetDrawPage() )
        {
            // Disable "multiple layout" for the moment:
            // use pMasterPage instead of a new created SdrPage
            // mpDrawModel->AllocPage( FALSE );
            // mpDrawModel->InsertPage( pDrawPage );
            SdrPage* pDrawPage = pMasterPage.get();
            pRoot->SetDrawPage( pDrawPage );
            pDrawPage->SetSize( pRoot->getFrameArea().SSize() );
        }
    }
}


void DocumentDrawModelManager::ReleaseDrawModel()
{
    // !! Also maintain the code in the sw3io for inserting documents!!
    mpDrawModel.reset();
}


const SwDrawModel* DocumentDrawModelManager::GetDrawModel() const
{
    return mpDrawModel.get();
}

SwDrawModel* DocumentDrawModelManager::GetDrawModel()
{
    return mpDrawModel.get();
}

SwDrawModel* DocumentDrawModelManager::MakeDrawModel_()
{
    OSL_ENSURE( !mpDrawModel, "MakeDrawModel_: Why?" );
    InitDrawModel();
    SwViewShell* const pSh = m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell();
    if ( pSh )
    {
        for(SwViewShell& rViewSh : pSh->GetRingContainer())
            rViewSh.MakeDrawView();

        // Broadcast, so that the FormShell can be connected to the DrawView
        if( m_rDoc.GetDocShell() )
        {
            SfxHint aHint( SfxHintId::SwDrawViewsCreated );
            m_rDoc.GetDocShell()->Broadcast( aHint );
        }
    }
    return mpDrawModel.get();
}

SwDrawModel* DocumentDrawModelManager::GetOrCreateDrawModel()
{
    return GetDrawModel() ? GetDrawModel() : MakeDrawModel_();
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

bool DocumentDrawModelManager::IsVisibleLayerId( SdrLayerID _nLayerId ) const
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

SdrLayerID DocumentDrawModelManager::GetInvisibleLayerIdByVisibleOne( SdrLayerID _nVisibleLayerId )
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
    SwPosFlyFrames aFrames = m_rDoc.GetAllFlyFormats(&rPaM, /*bDrawAlso=*/true);

    for (const SwPosFlyFramePtr& pPosFlyFrame : aFrames)
    {
        // Filter for at-paragraph anchored draw frames.
        const SwFrameFormat& rFrameFormat = pPosFlyFrame->GetFormat();
        const SwFormatAnchor& rAnchor = rFrameFormat.GetAnchor();
        if (rAnchor.GetAnchorId() != RndStdIds::FLY_AT_PARA || rFrameFormat.Which() != RES_DRAWFRMFMT)
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
    mpDrawModel->SetNotifyUndoActionHdl( nullptr );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
