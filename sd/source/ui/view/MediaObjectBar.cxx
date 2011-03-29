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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "MediaObjectBar.hxx"
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

#include "app.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "DrawDocShell.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"

using namespace sd;

#define MediaObjectBar
#include "sdslots.hxx"

namespace sd {

// ------------------
// - MediaObjectBar -
// ------------------

TYPEINIT1( MediaObjectBar, SfxShell );

// -----------------------------------------------------------------------------

SFX_IMPL_INTERFACE( MediaObjectBar, SfxShell, SdResId( STR_MEDIAOBJECTBARSHELL ) )
{
}

// -----------------------------------------------------------------------------

MediaObjectBar::MediaObjectBar( ViewShell* pSdViewShell, ::sd::View* pSdView ) :
    SfxShell( pSdViewShell->GetViewShell() ),
    mpView( pSdView ),
    mpViewSh( pSdViewShell )
{
    DrawDocShell* pDocShell = mpViewSh->GetDocSh();

    SetPool( &pDocShell->GetPool() );
    SetUndoManager( pDocShell->GetUndoManager() );
    SetRepeatTarget( mpView );
    SetHelpId( SD_IF_SDDRAWMEDIAOBJECTBAR );
    SetName( String( SdResId( RID_DRAW_MEDIA_TOOLBOX ) ) );
}

// -----------------------------------------------------------------------------

MediaObjectBar::~MediaObjectBar()
{
    SetRepeatTarget( NULL );
}

// -----------------------------------------------------------------------------

void MediaObjectBar::GetState( SfxItemSet& rSet )
{
    SfxWhichIter    aIter( rSet );
    sal_uInt16          nWhich = aIter.FirstWhich();

    while( nWhich )
    {
        if( SID_AVMEDIA_TOOLBOX == nWhich )
        {
            SdrMarkList* pMarkList = new SdrMarkList( mpView->GetMarkedObjectList() );
            bool         bDisable = true;

            if( 1 == pMarkList->GetMarkCount() )
            {
                SdrObject* pObj =pMarkList->GetMark( 0 )->GetMarkedSdrObj();

                if( pObj && pObj->ISA( SdrMediaObj ) )
                {
                    ::avmedia::MediaItem aItem( SID_AVMEDIA_TOOLBOX );

                    static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).updateMediaItem( aItem );
                    rSet.Put( aItem );
                    bDisable = false;
                }
            }

            if( bDisable )
                rSet.DisableItem( SID_AVMEDIA_TOOLBOX );

            delete pMarkList;
        }

        nWhich = aIter.NextWhich();
    }
}

// -----------------------------------------------------------------------------

void MediaObjectBar::Execute( SfxRequest& rReq )
{
    if( SID_AVMEDIA_TOOLBOX == rReq.GetSlot() )
    {
        const SfxItemSet*   pArgs = rReq.GetArgs();
        const SfxPoolItem*  pItem;

        if( !pArgs || ( SFX_ITEM_SET != pArgs->GetItemState( SID_AVMEDIA_TOOLBOX, sal_False, &pItem ) ) )
            pItem = NULL;

        if( pItem )
        {
            SdrMarkList* pMarkList = new SdrMarkList( mpView->GetMarkedObjectList() );

            if( 1 == pMarkList->GetMarkCount() )
            {
                SdrObject* pObj = pMarkList->GetMark( 0 )->GetMarkedSdrObj();

                if( pObj && pObj->ISA( SdrMediaObj ) )
                {
                    static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).executeMediaItem(
                        static_cast< const ::avmedia::MediaItem& >( *pItem ) );
                }
            }

            delete pMarkList;
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
