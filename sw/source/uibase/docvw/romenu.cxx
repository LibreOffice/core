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
#include <svl/stritem.hxx>
#include <svtools/imap.hxx>
#include <svtools/inetimg.hxx>
#include <svtools/transfer.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/gallery.hxx>
#include <svx/graphichelper.hxx>
#include <editeng/brushitem.hxx>

#include <frmatr.hxx>
#include <fmturl.hxx>
#include <fmtinfmt.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <swmodule.hxx>
#include <romenu.hxx>
#include <pagedesc.hxx>
#include <modcfg.hxx>

#include <cmdid.h>
#include <helpid.h>
#include <strings.hrc>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::sfx2;

SwReadOnlyPopup::~SwReadOnlyPopup()
{
    delete pImageMap;
    delete pTargetURL;
    xMenu.disposeAndClear();
}

void SwReadOnlyPopup::Check( sal_uInt16 nMID, sal_uInt16 nSID, SfxDispatcher &rDis )
{
    std::unique_ptr<SfxPoolItem> _pItem;
    SfxItemState eState = rDis.GetBindings()->QueryState( nSID, _pItem );
    if (eState >= SfxItemState::DEFAULT)
    {
        xMenu->EnableItem(nMID);
        if (_pItem)
        {
            xMenu->CheckItem(nMID, !_pItem->IsVoidItem() &&
                            dynamic_cast< const SfxBoolItem *>( _pItem.get() ) !=  nullptr &&
                            static_cast<SfxBoolItem*>(_pItem.get())->GetValue());
            //remove full screen entry when not in full screen mode
            if (SID_WIN_FULLSCREEN == nSID && !xMenu->IsItemChecked(nReadonlyFullscreen))
                xMenu->EnableItem(nMID, false);
        }
    }
    else
        xMenu->EnableItem(nMID, false);
}

#define MN_READONLY_GRAPHICTOGALLERY 1000
#define MN_READONLY_BACKGROUNDTOGALLERY 2000

SwReadOnlyPopup::SwReadOnlyPopup(const Point &rDPos, SwView &rV)
    : aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "modules/swriter/ui/readonlymenu.ui", "")
    , xMenu(aBuilder.get_menu("menu"))
    , nReadonlyOpenurl(xMenu->GetItemId("openurl"))
    , nReadonlyOpendoc(xMenu->GetItemId("opendoc"))
    , nReadonlyEditdoc(xMenu->GetItemId("edit"))
    , nReadonlySelectionMode(xMenu->GetItemId("selection"))
    , nReadonlyReload(xMenu->GetItemId("reload"))
    , nReadonlyReloadFrame(xMenu->GetItemId("reloadframe"))
    , nReadonlySourceview(xMenu->GetItemId("html"))
    , nReadonlyBrowseBackward(xMenu->GetItemId("backward"))
    , nReadonlyBrowseForward(xMenu->GetItemId("forward"))
    , nReadonlySaveGraphic(xMenu->GetItemId("savegraphic"))
    , nReadonlyGraphictogallery(xMenu->GetItemId("graphictogallery"))
    , nReadonlyTogallerylink(xMenu->GetItemId("graphicaslink"))
    , nReadonlyTogallerycopy(xMenu->GetItemId("graphicascopy"))
    , nReadonlySaveBackground(xMenu->GetItemId("savebackground"))
    , nReadonlyBackgroundtogallery(xMenu->GetItemId("backgroundtogallery"))
    , nReadonlyBackgroundTogallerylink(xMenu->GetItemId("backaslink"))
    , nReadonlyBackgroundTogallerycopy(xMenu->GetItemId("backascopy"))
    , nReadonlyCopylink(xMenu->GetItemId("copylink"))
    , nReadonlyCopyGraphic(xMenu->GetItemId("copygraphic"))
    , nReadonlyLoadGraphic(xMenu->GetItemId("loadgraphic"))
    , nReadonlyGraphicoff(xMenu->GetItemId("imagesoff"))
    , nReadonlyFullscreen(xMenu->GetItemId("fullscreen"))
    , nReadonlyCopy(xMenu->GetItemId("copy"))
    , rView(rV)
    , aBrushItem(RES_BACKGROUND)
    , rDocPos(rDPos)
    , pImageMap(nullptr)
    , pTargetURL(nullptr)
{
    bGrfToGalleryAsLnk = SW_MOD()->GetModuleConfig()->IsGrfToGalleryAsLnk();
    SwWrtShell &rSh = rView.GetWrtShell();
    rSh.IsURLGrfAtPos( rDocPos, &sURL, &sTargetFrameName, &sDescription );
    if ( sURL.isEmpty() )
    {
        SwContentAtPos aContentAtPos( IsAttrAtPos::InetAttr );
        if( rSh.GetContentAtPos( rDocPos, aContentAtPos))
        {
            const SwFormatINetFormat &rIItem = *static_cast<const SwFormatINetFormat*>(aContentAtPos.aFnd.pAttr);
            sURL = rIItem.GetValue();
            sTargetFrameName = rIItem.GetTargetFrame();
            sDescription = aContentAtPos.sStr;
        }
    }

    bool bLink = false;
    const Graphic *pGrf;
    if ( nullptr == (pGrf = rSh.GetGrfAtPos( rDocPos, sGrfName, bLink )) )
    {
        xMenu->EnableItem(nReadonlySaveGraphic, false);
        xMenu->EnableItem(nReadonlyCopyGraphic, false);
    }
    else
    {
        aGraphic = *pGrf;
        const SwFrameFormat* pGrfFormat = rSh.GetFormatFromObj( rDocPos );
        const SfxPoolItem* pURLItem;
        if( pGrfFormat && SfxItemState::SET == pGrfFormat->GetItemState(
            RES_URL, true, &pURLItem ))
        {
            const SwFormatURL& rURL = *static_cast<const SwFormatURL*>(pURLItem);
            if( rURL.GetMap() )
                pImageMap = new ImageMap( *rURL.GetMap() );
            else if( !rURL.GetURL().isEmpty() )
                pTargetURL = new INetImage( bLink ? sGrfName : OUString(),
                                            rURL.GetURL(),
                                            rURL.GetTargetFrameName() );
        }
    }

    bool bEnableGraphicToGallery = bLink;
    if ( bEnableGraphicToGallery )
    {
        if (GalleryExplorer::FillThemeList( aThemeList ))
        {
            PopupMenu *pMenu = xMenu->GetPopupMenu(nReadonlyGraphictogallery);
            pMenu->CheckItem(nReadonlyTogallerylink,  bGrfToGalleryAsLnk);
            pMenu->CheckItem(nReadonlyTogallerycopy, !bGrfToGalleryAsLnk);

            for ( size_t i=0; i < aThemeList.size(); ++i )
                pMenu->InsertItem(MN_READONLY_GRAPHICTOGALLERY + i, aThemeList[i]);
        }
        else
            bEnableGraphicToGallery = false;
    }

    xMenu->EnableItem(nReadonlyGraphictogallery, bEnableGraphicToGallery);

    SfxViewFrame * pVFrame = rV.GetViewFrame();
    SfxDispatcher &rDis = *pVFrame->GetDispatcher();
    const SwPageDesc &rDesc = rSh.GetPageDesc( rSh.GetCurPageDesc() );
    aBrushItem = rDesc.GetMaster().makeBackgroundBrushItem();
    bool bEnableBackGallery = false,
         bEnableBack = false;

    if ( GPOS_NONE != aBrushItem.GetGraphicPos() )
    {
        bEnableBack = true;
        if ( !aBrushItem.GetGraphicLink().isEmpty() )
        {
            if ( aThemeList.empty() )
                GalleryExplorer::FillThemeList( aThemeList );

            if ( !aThemeList.empty() )
            {
                PopupMenu *pMenu = xMenu->GetPopupMenu(nReadonlyBackgroundtogallery);
                pMenu->CheckItem(nReadonlyBackgroundTogallerylink,  bGrfToGalleryAsLnk);
                pMenu->CheckItem(nReadonlyBackgroundTogallerycopy, !bGrfToGalleryAsLnk);
                bEnableBackGallery = true;

                for ( size_t i=0; i < aThemeList.size(); ++i )
                    pMenu->InsertItem(MN_READONLY_BACKGROUNDTOGALLERY + i, aThemeList[i]);
            }
        }
    }
    xMenu->EnableItem(nReadonlySaveBackground, bEnableBack);
    xMenu->EnableItem(nReadonlyBackgroundtogallery, bEnableBackGallery);

    if ( !rSh.GetViewOptions()->IsGraphic() )
        xMenu->CheckItem(nReadonlyGraphicoff);
    else
        xMenu->EnableItem(nReadonlyLoadGraphic, false);

    xMenu->EnableItem(nReadonlyReloadFrame, false);
    xMenu->EnableItem(nReadonlyReload);

    Check(nReadonlyEditdoc, SID_EDITDOC, rDis);
    Check(nReadonlySelectionMode, FN_READONLY_SELECTION_MODE, rDis);
    Check(nReadonlySourceview, SID_SOURCEVIEW, rDis);
    Check(nReadonlyBrowseBackward, SID_BROWSE_BACKWARD, rDis);
    Check(nReadonlyBrowseForward,SID_BROWSE_FORWARD, rDis);
    Check(nReadonlyOpenurl, SID_OPENDOC, rDis);
    Check(nReadonlyOpendoc, SID_OPENDOC, rDis);

    std::unique_ptr<SfxPoolItem> pState;

    SfxItemState eState = pVFrame->GetBindings().QueryState( SID_COPY, pState );
    Check(nReadonlyCopy, SID_COPY, rDis);
    if (eState < SfxItemState::DEFAULT)
        xMenu->EnableItem(nReadonlyCopy, false);

    eState = pVFrame->GetBindings().QueryState( SID_EDITDOC, pState );
    if (
        eState < SfxItemState::DEFAULT ||
        (rSh.IsGlobalDoc() && rView.GetDocShell()->IsReadOnlyUI())
       )
    {
        xMenu->EnableItem(nReadonlyEditdoc, false);
    }

    if ( sURL.isEmpty() )
    {
        xMenu->EnableItem(nReadonlyOpenurl, false);
        xMenu->EnableItem(nReadonlyOpendoc, false);
        xMenu->EnableItem(nReadonlyCopylink, false);
    }
    Check(nReadonlyFullscreen, SID_WIN_FULLSCREEN, rDis);

    xMenu->RemoveDisabledEntries( true, true );
}

void SwReadOnlyPopup::Execute( vcl::Window* pWin, const Point &rPixPos )
{
    sal_uInt16 nId = xMenu->Execute(pWin, rPixPos);
    Execute(pWin, nId);
}

// execute the resulting ID only - necessary to support XContextMenuInterception
void SwReadOnlyPopup::Execute( vcl::Window* pWin, sal_uInt16 nId )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SfxDispatcher &rDis = *rView.GetViewFrame()->GetDispatcher();
    if (nId >= MN_READONLY_GRAPHICTOGALLERY)
    {
        OUString sTmp;
        sal_uInt16 nSaveId;
        if (nId >= MN_READONLY_BACKGROUNDTOGALLERY)
        {
            nId -= MN_READONLY_BACKGROUNDTOGALLERY;
            nSaveId = nReadonlySaveBackground;
            sTmp = aBrushItem.GetGraphicLink();
        }
        else
        {
            nId -= MN_READONLY_GRAPHICTOGALLERY;
            nSaveId = nReadonlySaveGraphic;
            sTmp = sGrfName;
        }
        if ( !bGrfToGalleryAsLnk )
            sTmp = SaveGraphic(nSaveId);

        if ( !sTmp.isEmpty() )
            GalleryExplorer::InsertURL( aThemeList[nId], sTmp );

        return;
    }

    rtl::Reference<TransferDataContainer> pClipCntnr;

    sal_uInt16 nExecId = USHRT_MAX;
    bool bFilterSet = false;
    LoadUrlFlags nFilter = LoadUrlFlags::NONE;
    if (nId == nReadonlyFullscreen)
        nExecId = SID_WIN_FULLSCREEN;
    else if (nId == nReadonlyOpenurl)
    {
        nFilter = LoadUrlFlags::NONE;
        bFilterSet = true;
    }
    else if (nId == nReadonlyOpendoc)
    {
        nFilter = LoadUrlFlags::NewView;
        bFilterSet = true;
    }
    else if (nId == nReadonlyCopy)
        nExecId = SID_COPY;
    else if (nId == nReadonlyEditdoc)
        nExecId = SID_EDITDOC;
    else if (nId == nReadonlySelectionMode)
        nExecId = FN_READONLY_SELECTION_MODE;
    else if (nId == nReadonlyReload || nId == nReadonlyReloadFrame)
        rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(SID_RELOAD);
    else if (nId == nReadonlyBrowseBackward)
        nExecId = SID_BROWSE_BACKWARD;
    else if (nId == nReadonlyBrowseForward)
        nExecId = SID_BROWSE_FORWARD;
    else if (nId == nReadonlySourceview)
        nExecId = SID_SOURCEVIEW;
    else if (nId == nReadonlySaveGraphic || nId == nReadonlySaveBackground)
        SaveGraphic(nId);
    else if (nId == nReadonlyCopylink)
    {
        pClipCntnr = new TransferDataContainer;
        pClipCntnr->CopyString( sURL );
    }
    else if (nId == nReadonlyCopyGraphic)
    {
        pClipCntnr = new TransferDataContainer;
        pClipCntnr->CopyGraphic( aGraphic );

        if( pImageMap )
            pClipCntnr->CopyImageMap( *pImageMap );
        if( pTargetURL )
            pClipCntnr->CopyINetImage( *pTargetURL );
    }
    else if (nId == nReadonlyLoadGraphic)
    {
        bool bModified = rSh.IsModified();
        SwViewOption aOpt( *rSh.GetViewOptions() );
        aOpt.SetGraphic( true );
        rSh.ApplyViewOptions( aOpt );
        if(!bModified)
            rSh.ResetModified();
    }
    else if (nId == nReadonlyGraphicoff)
        nExecId = FN_VIEW_GRAPHIC;
    else if (nId == nReadonlyTogallerylink || nId == nReadonlyBackgroundTogallerylink)
        SW_MOD()->GetModuleConfig()->SetGrfToGalleryAsLnk(true);
    else if (nId == nReadonlyTogallerycopy || nId == nReadonlyBackgroundTogallerycopy)
        SW_MOD()->GetModuleConfig()->SetGrfToGalleryAsLnk(false);

    if( USHRT_MAX != nExecId )
        rDis.GetBindings()->Execute( nExecId );
    if( bFilterSet )
        ::LoadURL(rSh, sURL, nFilter, sTargetFrameName);

    if( pClipCntnr && pClipCntnr->HasAnyData() )
    {
            pClipCntnr->CopyToClipboard( pWin );
    }
}

OUString SwReadOnlyPopup::SaveGraphic(sal_uInt16 nId)
{
    // fish out the graphic's name
    if (nId == nReadonlySaveBackground)
    {
        if ( !aBrushItem.GetGraphicLink().isEmpty() )
            sGrfName = aBrushItem.GetGraphicLink();
        const Graphic *pGrf = aBrushItem.GetGraphic();
        if ( pGrf )
        {
            aGraphic = *pGrf;
            if ( !aBrushItem.GetGraphicLink().isEmpty() )
                sGrfName = aBrushItem.GetGraphicLink();
        }
        else
            return OUString();
    }
    return GraphicHelper::ExportGraphic( aGraphic, sGrfName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
