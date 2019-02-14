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

#include <MediaObjectBar.hxx>
#include <avmedia/mediaitem.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>

#include <app.hrc>

#include <strings.hrc>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <Window.hxx>
#include <drawview.hxx>
#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <memory>

using namespace sd;

#define ShellClass_MediaObjectBar
#include <sdslots.hxx>

namespace sd {


SFX_IMPL_INTERFACE(MediaObjectBar, SfxShell)

void MediaObjectBar::InitInterface_Impl()
{
}

MediaObjectBar::MediaObjectBar( ViewShell* pSdViewShell, ::sd::View* pSdView ) :
    SfxShell( pSdViewShell->GetViewShell() ),
    mpView( pSdView ),
    mpViewSh( pSdViewShell )
{
    DrawDocShell* pDocShell = mpViewSh->GetDocSh();

    SetPool( &pDocShell->GetPool() );
    SetUndoManager( pDocShell->GetUndoManager() );
    SetRepeatTarget( mpView );
    SetName(SdResId(RID_DRAW_MEDIA_TOOLBOX));
}

MediaObjectBar::~MediaObjectBar()
{
    SetRepeatTarget( nullptr );
}

void MediaObjectBar::GetState( SfxItemSet& rSet )
{
    SfxWhichIter    aIter( rSet );
    sal_uInt16          nWhich = aIter.FirstWhich();

    while( nWhich )
    {
        if( SID_AVMEDIA_TOOLBOX == nWhich )
        {
            std::unique_ptr<SdrMarkList> pMarkList(new SdrMarkList( mpView->GetMarkedObjectList() ));
            bool         bDisable = true;

            if( 1 == pMarkList->GetMarkCount() )
            {
                SdrObject* pObj =pMarkList->GetMark( 0 )->GetMarkedSdrObj();

                if( dynamic_cast< SdrMediaObj *>( pObj ) )
                {
                    ::avmedia::MediaItem aItem( SID_AVMEDIA_TOOLBOX );

                    static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).updateMediaItem( aItem );
                    rSet.Put( aItem );
                    bDisable = false;
                }
            }

            if( bDisable )
                rSet.DisableItem( SID_AVMEDIA_TOOLBOX );
        }

        nWhich = aIter.NextWhich();
    }
}

void MediaObjectBar::Execute( SfxRequest const & rReq )
{
    if( SID_AVMEDIA_TOOLBOX != rReq.GetSlot() )
        return;

    const SfxItemSet*   pArgs = rReq.GetArgs();
    const SfxPoolItem*  pItem;

    if( !pArgs || ( SfxItemState::SET != pArgs->GetItemState( SID_AVMEDIA_TOOLBOX, false, &pItem ) ) )
        pItem = nullptr;

    if( !pItem )
        return;

    std::unique_ptr<SdrMarkList> pMarkList(new SdrMarkList( mpView->GetMarkedObjectList() ));

    if( 1 != pMarkList->GetMarkCount() )
        return;

    SdrObject* pObj = pMarkList->GetMark( 0 )->GetMarkedSdrObj();

    if( !dynamic_cast< SdrMediaObj *>( pObj ) )
        return;

    static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).executeMediaItem(
        static_cast< const ::avmedia::MediaItem& >( *pItem ) );


    //if only changing state then don't set modified flag (e.g. playing a video)
    if( !(static_cast< const ::avmedia::MediaItem& >( *pItem ).getMaskSet() & AVMediaSetMask::STATE))
    {
        //fdo #32598: after changing playback opts, set document's modified flag
        SdDrawDocument& rDoc = mpView->GetDoc();
        rDoc.SetChanged();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
