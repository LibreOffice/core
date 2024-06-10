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

#include <memory>
#include <hintids.hxx>

#include <svl/eitem.hxx>
#include <vcl/settings.hxx>
#include <vcl/transfer.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/gallery.hxx>
#include <svx/graphichelper.hxx>
#include <editeng/brushitem.hxx>

#include <fmtinfmt.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <swmodule.hxx>
#include "romenu.hxx"
#include <pagedesc.hxx>
#include <modcfg.hxx>

#include <cmdid.h>

using namespace ::com::sun::star;

SwReadOnlyPopup::~SwReadOnlyPopup()
{
    m_xMenu.disposeAndClear();
}

void SwReadOnlyPopup::Check( sal_uInt16 nMID, sal_uInt16 nSID, SfxDispatcher const &rDis )
{
    std::unique_ptr<SfxPoolItem> _pItem;
    SfxItemState eState = rDis.GetBindings()->QueryState( nSID, _pItem );
    if (eState >= SfxItemState::DEFAULT)
    {
        m_xMenu->EnableItem(nMID);
        if (_pItem)
        {
            m_xMenu->CheckItem(nMID, !IsDisabledItem(_pItem.get()) &&
                            dynamic_cast< const SfxBoolItem *>( _pItem.get() ) !=  nullptr &&
                            static_cast<SfxBoolItem*>(_pItem.get())->GetValue());
            //remove full screen entry when not in full screen mode
            if (SID_WIN_FULLSCREEN == nSID && !m_xMenu->IsItemChecked(m_nReadonlyFullscreen))
                m_xMenu->EnableItem(nMID, false);
        }
    }
    else
        m_xMenu->EnableItem(nMID, false);
}

#define MN_READONLY_GRAPHICTOGALLERY 1000
#define MN_READONLY_BACKGROUNDTOGALLERY 2000

SwReadOnlyPopup::SwReadOnlyPopup(const Point &rDPos, SwView &rV)
    : m_aBuilder(nullptr, AllSettings::GetUIRootDir(), u"modules/swriter/ui/readonlymenu.ui"_ustr, u""_ustr)
    , m_xMenu(m_aBuilder.get_menu(u"menu"))
    , m_nReadonlyOpenurl(m_xMenu->GetItemId(u"openurl"))
    , m_nReadonlyOpendoc(m_xMenu->GetItemId(u"opendoc"))
    , m_nReadonlyEditdoc(m_xMenu->GetItemId(u"edit"))
    , m_nReadonlySelectionMode(m_xMenu->GetItemId(u"selection"))
    , m_nReadonlyReload(m_xMenu->GetItemId(u"reload"))
    , m_nReadonlyReloadFrame(m_xMenu->GetItemId(u"reloadframe"))
    , m_nReadonlySourceview(m_xMenu->GetItemId(u"html"))
    , m_nReadonlyBrowseBackward(m_xMenu->GetItemId(u"backward"))
    , m_nReadonlyBrowseForward(m_xMenu->GetItemId(u"forward"))
    , m_nReadonlySaveGraphic(m_xMenu->GetItemId(u"savegraphic"))
    , m_nReadonlyGraphictogallery(m_xMenu->GetItemId(u"graphictogallery"))
    , m_nReadonlyTogallerylink(m_xMenu->GetItemId(u"graphicaslink"))
    , m_nReadonlyTogallerycopy(m_xMenu->GetItemId(u"graphicascopy"))
    , m_nReadonlySaveBackground(m_xMenu->GetItemId(u"savebackground"))
    , m_nReadonlyBackgroundtogallery(m_xMenu->GetItemId(u"backgroundtogallery"))
    , m_nReadonlyBackgroundTogallerylink(m_xMenu->GetItemId(u"backaslink"))
    , m_nReadonlyBackgroundTogallerycopy(m_xMenu->GetItemId(u"backascopy"))
    , m_nReadonlyCopylink(m_xMenu->GetItemId(u"copylink"))
    , m_nReadonlyLoadGraphic(m_xMenu->GetItemId(u"loadgraphic"))
    , m_nReadonlyGraphicoff(m_xMenu->GetItemId(u"imagesoff"))
    , m_nReadonlyFullscreen(m_xMenu->GetItemId(u"fullscreen"))
    , m_nReadonlyCopy(m_xMenu->GetItemId(u"copy"))
    , m_rView(rV)
    , m_xBrushItem(std::make_unique<SvxBrushItem>(RES_BACKGROUND))
{
    m_bGrfToGalleryAsLnk = SW_MOD()->GetModuleConfig()->IsGrfToGalleryAsLnk();
    SwWrtShell &rSh = m_rView.GetWrtShell();
    OUString sDescription;
    rSh.IsURLGrfAtPos( rDPos, &m_sURL, &m_sTargetFrameName, &sDescription );
    if ( m_sURL.isEmpty() )
    {
        SwContentAtPos aContentAtPos( IsAttrAtPos::InetAttr );
        if( rSh.GetContentAtPos( rDPos, aContentAtPos))
        {
            const SwFormatINetFormat &rIItem = *static_cast<const SwFormatINetFormat*>(aContentAtPos.aFnd.pAttr);
            m_sURL = rIItem.GetValue();
            m_sTargetFrameName = rIItem.GetTargetFrame();
        }
    }

    bool bLink = false;
    const Graphic *pGrf = rSh.GetGrfAtPos( rDPos, m_sGrfName, bLink );
    if ( nullptr == pGrf )
    {
        m_xMenu->EnableItem(m_nReadonlySaveGraphic, false);
    }
    else
    {
        m_aGraphic = *pGrf;
    }

    bool bEnableGraphicToGallery = bLink;
    if ( bEnableGraphicToGallery )
    {
        if (GalleryExplorer::FillThemeList( m_aThemeList ))
        {
            PopupMenu *pMenu = m_xMenu->GetPopupMenu(m_nReadonlyGraphictogallery);
            pMenu->CheckItem(m_nReadonlyTogallerylink,  m_bGrfToGalleryAsLnk);
            pMenu->CheckItem(m_nReadonlyTogallerycopy, !m_bGrfToGalleryAsLnk);

            for ( size_t i=0; i < m_aThemeList.size(); ++i )
                pMenu->InsertItem(MN_READONLY_GRAPHICTOGALLERY + i, m_aThemeList[i]);
        }
        else
            bEnableGraphicToGallery = false;
    }

    m_xMenu->EnableItem(m_nReadonlyGraphictogallery, bEnableGraphicToGallery);

    SfxViewFrame& rVFrame = rV.GetViewFrame();
    SfxDispatcher &rDis = *rVFrame.GetDispatcher();
    const SwPageDesc &rDesc = rSh.GetPageDesc( rSh.GetCurPageDesc() );
    m_xBrushItem = rDesc.GetMaster().makeBackgroundBrushItem();
    bool bEnableBackGallery = false,
         bEnableBack = false;

    if ( m_xBrushItem && GPOS_NONE != m_xBrushItem->GetGraphicPos() )
    {
        bEnableBack = true;
        if ( !m_xBrushItem->GetGraphicLink().isEmpty() )
        {
            if ( m_aThemeList.empty() )
                GalleryExplorer::FillThemeList( m_aThemeList );

            if ( !m_aThemeList.empty() )
            {
                PopupMenu *pMenu = m_xMenu->GetPopupMenu(m_nReadonlyBackgroundtogallery);
                pMenu->CheckItem(m_nReadonlyBackgroundTogallerylink,  m_bGrfToGalleryAsLnk);
                pMenu->CheckItem(m_nReadonlyBackgroundTogallerycopy, !m_bGrfToGalleryAsLnk);
                bEnableBackGallery = true;

                for ( size_t i=0; i < m_aThemeList.size(); ++i )
                    pMenu->InsertItem(MN_READONLY_BACKGROUNDTOGALLERY + i, m_aThemeList[i]);
            }
        }
    }
    m_xMenu->EnableItem(m_nReadonlySaveBackground, bEnableBack);
    m_xMenu->EnableItem(m_nReadonlyBackgroundtogallery, bEnableBackGallery);

    if ( !rSh.GetViewOptions()->IsGraphic() )
        m_xMenu->CheckItem(m_nReadonlyGraphicoff);
    else
        m_xMenu->EnableItem(m_nReadonlyLoadGraphic, false);

    m_xMenu->EnableItem(m_nReadonlyReloadFrame, false);
    m_xMenu->EnableItem(m_nReadonlyReload);

    Check(m_nReadonlyEditdoc, SID_EDITDOC, rDis);
    Check(m_nReadonlySelectionMode, FN_READONLY_SELECTION_MODE, rDis);
    Check(m_nReadonlySourceview, SID_SOURCEVIEW, rDis);
    Check(m_nReadonlyBrowseBackward, SID_BROWSE_BACKWARD, rDis);
    Check(m_nReadonlyBrowseForward,SID_BROWSE_FORWARD, rDis);
    Check(m_nReadonlyOpenurl, SID_OPENDOC, rDis);
    Check(m_nReadonlyOpendoc, SID_OPENDOC, rDis);

    std::unique_ptr<SfxPoolItem> pState;

    SfxItemState eState = rVFrame.GetBindings().QueryState( SID_COPY, pState );
    Check(m_nReadonlyCopy, SID_COPY, rDis);
    if (eState < SfxItemState::DEFAULT)
        m_xMenu->EnableItem(m_nReadonlyCopy, false);

    eState = rVFrame.GetBindings().QueryState( SID_EDITDOC, pState );
    if (
        eState < SfxItemState::DEFAULT ||
        (rSh.IsGlobalDoc() && m_rView.GetDocShell()->IsReadOnlyUI())
       )
    {
        m_xMenu->EnableItem(m_nReadonlyEditdoc, false);
    }

    if ( m_sURL.isEmpty() )
    {
        m_xMenu->EnableItem(m_nReadonlyOpenurl, false);
        m_xMenu->EnableItem(m_nReadonlyOpendoc, false);
        m_xMenu->EnableItem(m_nReadonlyCopylink, false);
    }
    Check(m_nReadonlyFullscreen, SID_WIN_FULLSCREEN, rDis);

    m_xMenu->RemoveDisabledEntries( true );
}

void SwReadOnlyPopup::Execute( vcl::Window* pWin, const Point &rPixPos )
{
    sal_uInt16 nId = m_xMenu->Execute(pWin, rPixPos);
    Execute(pWin, nId);
}

// execute the resulting ID only - necessary to support XContextMenuInterception
void SwReadOnlyPopup::Execute( vcl::Window* pWin, sal_uInt16 nId )
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    SfxDispatcher &rDis = *m_rView.GetViewFrame().GetDispatcher();
    if (nId >= MN_READONLY_GRAPHICTOGALLERY)
    {
        OUString sTmp;
        sal_uInt16 nSaveId;
        if (m_xBrushItem && nId >= MN_READONLY_BACKGROUNDTOGALLERY)
        {
            nId -= MN_READONLY_BACKGROUNDTOGALLERY;
            nSaveId = m_nReadonlySaveBackground;
            sTmp = m_xBrushItem->GetGraphicLink();
        }
        else
        {
            nId -= MN_READONLY_GRAPHICTOGALLERY;
            nSaveId = m_nReadonlySaveGraphic;
            sTmp = m_sGrfName;
        }
        if ( !m_bGrfToGalleryAsLnk )
            sTmp = SaveGraphic(nSaveId);

        if ( !sTmp.isEmpty() )
            GalleryExplorer::InsertURL( m_aThemeList[nId], sTmp );

        return;
    }

    rtl::Reference<TransferDataContainer> pClipCntnr;

    sal_uInt16 nExecId = USHRT_MAX;
    bool bFilterSet = false;
    LoadUrlFlags nFilter = LoadUrlFlags::NONE;
    if (nId == m_nReadonlyFullscreen)
        nExecId = SID_WIN_FULLSCREEN;
    else if (nId == m_nReadonlyOpenurl)
    {
        nFilter = LoadUrlFlags::NONE;
        bFilterSet = true;
    }
    else if (nId == m_nReadonlyOpendoc)
    {
        nFilter = LoadUrlFlags::NewView;
        bFilterSet = true;
    }
    else if (nId == m_nReadonlyCopy)
        nExecId = SID_COPY;
    else if (nId == m_nReadonlyEditdoc)
        nExecId = SID_EDITDOC;
    else if (nId == m_nReadonlySelectionMode)
        nExecId = FN_READONLY_SELECTION_MODE;
    else if (nId == m_nReadonlyReload || nId == m_nReadonlyReloadFrame)
        rSh.GetView().GetViewFrame().GetDispatcher()->Execute(SID_RELOAD);
    else if (nId == m_nReadonlyBrowseBackward)
        nExecId = SID_BROWSE_BACKWARD;
    else if (nId == m_nReadonlyBrowseForward)
        nExecId = SID_BROWSE_FORWARD;
    else if (nId == m_nReadonlySourceview)
        nExecId = SID_SOURCEVIEW;
    else if (nId == m_nReadonlySaveGraphic || nId == m_nReadonlySaveBackground)
        SaveGraphic(nId);
    else if (nId == m_nReadonlyCopylink)
    {
        pClipCntnr = new TransferDataContainer;
        pClipCntnr->CopyString( m_sURL );
    }
    else if (nId == m_nReadonlyLoadGraphic)
    {
        bool bModified = rSh.IsModified();
        SwViewOption aOpt( *rSh.GetViewOptions() );
        aOpt.SetGraphic( true );
        rSh.ApplyViewOptions( aOpt );
        if(!bModified)
            rSh.ResetModified();
    }
    else if (nId == m_nReadonlyGraphicoff)
        nExecId = FN_VIEW_GRAPHIC;
    else if (nId == m_nReadonlyTogallerylink || nId == m_nReadonlyBackgroundTogallerylink)
        SW_MOD()->GetModuleConfig()->SetGrfToGalleryAsLnk(true);
    else if (nId == m_nReadonlyTogallerycopy || nId == m_nReadonlyBackgroundTogallerycopy)
        SW_MOD()->GetModuleConfig()->SetGrfToGalleryAsLnk(false);

    if( USHRT_MAX != nExecId )
        rDis.GetBindings()->Execute( nExecId );
    if( bFilterSet )
        ::LoadURL(rSh, m_sURL, nFilter, m_sTargetFrameName);

    if( pClipCntnr && pClipCntnr->HasAnyData() )
    {
        pClipCntnr->CopyToClipboard( pWin );
    }
}

OUString SwReadOnlyPopup::SaveGraphic(sal_uInt16 nId)
{
    // fish out the graphic's name
    if (nId == m_nReadonlySaveBackground)
    {
        if ( m_xBrushItem && !m_xBrushItem->GetGraphicLink().isEmpty() )
            m_sGrfName = m_xBrushItem->GetGraphicLink();
        const Graphic *pGrf = m_xBrushItem ? m_xBrushItem->GetGraphic() : nullptr;
        if ( pGrf )
        {
            m_aGraphic = *pGrf;
            if ( !m_xBrushItem->GetGraphicLink().isEmpty() )
                m_sGrfName = m_xBrushItem->GetGraphicLink();
        }
        else
            return OUString();
    }
    return GraphicHelper::ExportGraphic(m_rView.GetFrameWeld(), m_aGraphic, m_sGrfName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
