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

#include <svx/imapdlg.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdview.hxx>
#include <svx/gallery.hxx>
#include <svx/hlnkitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <svl/whiter.hxx>

#include "imapwrap.hxx"
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "tabview.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "docsh.hxx"

#include <svx/galleryitem.hxx>
#include <com/sun/star/gallery/GalleryItemType.hpp>

class SvxIMapDlg;

void ScTabViewShell::ExecChildWin(SfxRequest& rReq)
{
    sal_uInt16 nSlot = rReq.GetSlot();
    switch(nSlot)
    {
        case SID_GALLERY:
        {
            // First make sure that the sidebar is visible
            GetViewFrame()->ShowChildWindow(SID_SIDEBAR);

            ::sfx2::sidebar::Sidebar::ShowPanel(
                "GalleryPanel",
                GetViewFrame()->GetFrame().GetFrameInterface());
        }
        break;
    }
}

void ScTabViewShell::ExecGallery( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();

    const SvxGalleryItem* pGalleryItem = SfxItemSet::GetItem<SvxGalleryItem>(pArgs, SID_GALLERY_FORMATS, false);
    if ( !pGalleryItem )
        return;

    sal_Int8 nType( pGalleryItem->GetType() );
    if ( nType == css::gallery::GalleryItemType::GRAPHIC )
    {
        MakeDrawLayer();

        Graphic aGraphic( pGalleryItem->GetGraphic() );
        Point   aPos     = GetInsertPos();

        OUString aPath, aFilter;
        PasteGraphic( aPos, aGraphic, aPath, aFilter );
    }
    else if ( nType == css::gallery::GalleryItemType::MEDIA )
    {
        //  for sounds (linked or not), insert a hyperlink button,
        //  like in Impress and Writer
        const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, pGalleryItem->GetURL() );
        GetViewFrame()->GetDispatcher()->Execute( SID_INSERT_AVMEDIA, SfxCallMode::SYNCHRON, &aMediaURLItem, 0L );
    }
}

void ScTabViewShell::ExecImageMap( SfxRequest& rReq )
{
    sal_uInt16 nSlot = rReq.GetSlot();
    switch(nSlot)
    {
        case SID_IMAP:
        {
            SfxViewFrame* pThisFrame = GetViewFrame();
            sal_uInt16 nId = ScIMapChildWindowId();
            pThisFrame->ToggleChildWindow( nId );
            GetViewFrame()->GetBindings().Invalidate( SID_IMAP );

            if ( pThisFrame->HasChildWindow( nId ) )
            {
                SvxIMapDlg* pDlg = GetIMapDlg();
                if ( pDlg )
                {
                    SdrView* pDrView = GetSdrView();
                    if ( pDrView )
                    {
                        const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                        if ( rMarkList.GetMarkCount() == 1 )
                            UpdateIMap( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );
                    }
                }
            }

            rReq.Ignore();
        }
        break;

        case SID_IMAP_EXEC:
        {
            SdrView* pDrView = GetSdrView();
            SdrMark* pMark = pDrView ? pDrView->GetMarkedObjectList().GetMark(0) : nullptr;

            if ( pMark )
            {
                SdrObject*  pSdrObj = pMark->GetMarkedSdrObj();
                SvxIMapDlg* pDlg = GetIMapDlg();

                if ( ScIMapDlgGetObj(pDlg) == static_cast<void*>(pSdrObj) )
                {
                    const ImageMap& rImageMap = ScIMapDlgGetMap(pDlg);
                    ScIMapInfo*     pIMapInfo = ScDrawLayer::GetIMapInfo( pSdrObj );

                    if ( !pIMapInfo )
                        pSdrObj->AppendUserData( new ScIMapInfo( rImageMap ) );
                    else
                        pIMapInfo->SetImageMap( rImageMap );

                    GetViewData().GetDocShell()->SetDrawModified();
                }
            }
        }
        break;
    }
}

void ScTabViewShell::GetImageMapState( SfxItemSet& rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_IMAP:
                {
                    // We don't disable this anymore

                    bool bThere = false;
                    SfxViewFrame* pThisFrame = GetViewFrame();
                    sal_uInt16 nId = ScIMapChildWindowId();
                    if ( pThisFrame->KnowsChildWindow(nId) )
                        if ( pThisFrame->HasChildWindow(nId) )
                            bThere = true;

                    ObjectSelectionType eType=GetCurObjectSelectionType();
                    bool bEnable=(eType==OST_OleObject) ||(eType==OST_Graphic);
                    if(!bThere && !bEnable)
                    {
                       rSet.DisableItem( nWhich );
                    }
                    else
                    {
                        rSet.Put( SfxBoolItem( nWhich, bThere ) );
                    }
                }
                break;

            case SID_IMAP_EXEC:
                {
                    bool bDisable = true;

                    SdrView* pDrView = GetSdrView();
                    if ( pDrView )
                    {
                        const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                        if ( rMarkList.GetMarkCount() == 1 )
                            if ( ScIMapDlgGetObj(GetIMapDlg()) ==
                                        static_cast<void*>(rMarkList.GetMark(0)->GetMarkedSdrObj()) )
                                bDisable = false;
                    }

                    rSet.Put( SfxBoolItem( SID_IMAP_EXEC, bDisable ) );
                }
                break;
        }

        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
