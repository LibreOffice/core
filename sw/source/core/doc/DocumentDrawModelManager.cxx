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
#include <docsh.hxx>
#include <swtypes.hxx>
#include <swhints.hxx>
#include <viewsh.hxx>
#include <drawdoc.hxx>
#include <rootfrm.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svl/smplhint.hxx>
#include <tools/link.hxx>

class SdrOutliner;
class XSpellChecker1;

namespace sw
{

DocumentDrawModelManager::DocumentDrawModelManager( SwDoc& i_rSwdoc ) : m_rSwdoc(i_rSwdoc), mpDrawModel(0) {}


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
//      pSdrPool->SetPoolDefaultItem(SdrShadowXDistItem((300 * 72) / 127));
//      pSdrPool->SetPoolDefaultItem(SdrShadowYDistItem((300 * 72) / 127));
//  }
//  SfxItemPool *pEEgPool = EditEngine::CreatePool( false );
//  pSdrPool->SetSecondaryPool( pEEgPool );
//   if ( !GetAttrPool().GetFrozenIdRanges () )
//      GetAttrPool().FreezeIdRanges();
//  else
//      pSdrPool->FreezeIdRanges();

    // set FontHeight pool defaults without changing static SdrEngineDefaults
    m_rSwdoc.GetAttrPool().SetPoolDefaultItem(SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT ));

    SAL_INFO( "sw.doc", "before create DrawDocument" );
    // The document owns the SdrModel. We always have two layers and one page.
    mpDrawModel = new SwDrawDocument( &m_rSwdoc );

    mpDrawModel->EnableUndo( m_rSwdoc.GetIDocumentUndoRedo().DoesUndo() );

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
    SAL_INFO( "sw.doc", "before create Spellchecker/Hyphenator" );
    SdrOutliner& rOutliner = mpDrawModel->GetDrawOutliner();
    ::com::sun::star::uno::Reference< com::sun::star::linguistic2::XSpellChecker1 > xSpell = ::GetSpellChecker();
    rOutliner.SetSpeller( xSpell );
    ::com::sun::star::uno::Reference< com::sun::star::linguistic2::XHyphenator > xHyphenator( ::GetHyphenator() );
    rOutliner.SetHyphenator( xHyphenator );
    SAL_INFO( "sw.doc", "after create Spellchecker/Hyphenator" );
    m_rSwdoc.SetCalcFieldValueHdl(&rOutliner);
    m_rSwdoc.SetCalcFieldValueHdl(&mpDrawModel->GetHitTestOutliner());

    // Set the LinkManager in the model so that linked graphics can be inserted.
    // The WinWord import needs it too.
    mpDrawModel->SetLinkManager( & m_rSwdoc.GetLinkManager() );
    mpDrawModel->SetAddExtLeading( m_rSwdoc.getIDocumentSettingAccess().get(IDocumentSettingAccess::ADD_EXT_LEADING) );

    OutputDevice* pRefDev = m_rSwdoc.getIDocumentDeviceAccess().getReferenceDevice( false );
    if ( pRefDev )
        mpDrawModel->SetRefDevice( pRefDev );

    mpDrawModel->SetNotifyUndoActionHdl( LINK( &m_rSwdoc, SwDoc, AddDrawUndo ));
    if ( m_rSwdoc.GetCurrentViewShell() )
    {
        SwViewShell* pViewSh = m_rSwdoc.GetCurrentViewShell();
        do
        {
            SwRootFrm* pRoot =  pViewSh->GetLayout();
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
            pViewSh = (SwViewShell*)pViewSh->GetNext();
        }while( pViewSh != m_rSwdoc.GetCurrentViewShell() );
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





const SdrModel* DocumentDrawModelManager::GetDrawModel() const
{
    return mpDrawModel;
}

SdrModel* DocumentDrawModelManager::GetDrawModel()
{
    return mpDrawModel;
}

SdrModel* DocumentDrawModelManager::_MakeDrawModel()
{
    OSL_ENSURE( !mpDrawModel, "_MakeDrawModel: Why?" );
    InitDrawModel();
    if ( m_rSwdoc.GetCurrentViewShell() )
    {
        SwViewShell* pTmp = m_rSwdoc.GetCurrentViewShell();
        do
        {
            pTmp->MakeDrawView();
            pTmp = (SwViewShell*) pTmp->GetNext();
        } while ( pTmp != m_rSwdoc.GetCurrentViewShell() );

        // Broadcast, so that the FormShell can be connected to the DrawView
        if( m_rSwdoc.GetDocShell() )
        {
            SfxSimpleHint aHnt( SW_BROADCAST_DRAWVIEWS_CREATED );
            m_rSwdoc.GetDocShell()->Broadcast( aHnt );
        }
    }
    return mpDrawModel;
}

SdrModel* DocumentDrawModelManager::GetOrCreateDrawModel()
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

SdrLayerID DocumentDrawModelManager::GetVisibleLayerIdByInvisibleOne( const SdrLayerID& _nInvisibleLayerId )
{
    SdrLayerID nVisibleLayerId;

    if ( _nInvisibleLayerId == GetInvisibleHeavenId() )
    {
        nVisibleLayerId = GetHeavenId();
    }
    else if ( _nInvisibleLayerId == GetInvisibleHellId() )
    {
        nVisibleLayerId = GetHellId();
    }
    else if ( _nInvisibleLayerId == GetInvisibleControlsId() )
    {
        nVisibleLayerId = GetControlsId();
    }
    else if ( _nInvisibleLayerId == GetHeavenId() ||
              _nInvisibleLayerId == GetHellId() ||
              _nInvisibleLayerId == GetControlsId() )
    {
        OSL_FAIL( "<SwDoc::GetVisibleLayerIdByInvisibleOne(..)> - given layer ID already an invisible one." );
        nVisibleLayerId = _nInvisibleLayerId;
    }
    else
    {
        OSL_FAIL( "<SwDoc::GetVisibleLayerIdByInvisibleOne(..)> - given layer ID is unknown." );
        nVisibleLayerId = _nInvisibleLayerId;
    }

    return nVisibleLayerId;
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

void DocumentDrawModelManager::DrawNotifyUndoHdl()
{
    mpDrawModel->SetNotifyUndoActionHdl( Link() );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
