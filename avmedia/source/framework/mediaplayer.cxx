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

#include <avmedia/mediaplayer.hxx>
#include <avmedia/mediawindow.hxx>
#include <avmedia/mediaitem.hxx>
#include "mediamisc.hxx"
#include "mediacontrol.hrc"
#include "helpids.hrc"

#include <svl/stritem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

namespace avmedia
{

MediaPlayer::MediaPlayer( vcl::Window* _pParent, sal_uInt16 nId, SfxBindings* _pBindings, SfxChildWinInfo* pInfo ) :
    SfxChildWindow( _pParent, nId )
{
    SetWindow( VclPtr<MediaFloater>::Create( _pBindings, this, _pParent ) );
    static_cast< MediaFloater* >( GetWindow() )->Initialize( pInfo );
};


MediaPlayer::~MediaPlayer()
{
}


SFX_IMPL_DOCKINGWINDOW_WITHID( MediaPlayer, SID_AVMEDIA_PLAYER )


MediaFloater::MediaFloater( SfxBindings* _pBindings, SfxChildWindow* pCW, vcl::Window* pParent ) :
    SfxDockingWindow( _pBindings, pCW, pParent, WB_CLOSEABLE | WB_MOVEABLE | WB_SIZEABLE | WB_DOCKABLE ),
    mpMediaWindow( new MediaWindow( this, true ) )
{
    const Size aSize( 378, 256 );

    SetPosSizePixel( Point( 0, 0 ), aSize );
    SetMinOutputSizePixel( aSize );
    SetText( OUString( AVMEDIA_RESID( AVMEDIA_STR_MEDIAPLAYER ) ) );
    mpMediaWindow->show();
}


MediaFloater::~MediaFloater()
{
    disposeOnce();
}


void MediaFloater::dispose()
{
    delete mpMediaWindow;
    mpMediaWindow = nullptr;
    SfxDockingWindow::dispose();
}


void MediaFloater::Resize()
{
    SfxDockingWindow::Resize();

    if( mpMediaWindow )
        mpMediaWindow->setPosSize( Rectangle( Point(), GetOutputSizePixel() ) );
}


void MediaFloater::ToggleFloatingMode()
{
    ::avmedia::MediaItem aRestoreItem;

    if (mpMediaWindow)
        mpMediaWindow->updateMediaItem( aRestoreItem );
    delete mpMediaWindow;
    mpMediaWindow = nullptr;

    SfxDockingWindow::ToggleFloatingMode();

    if (isDisposed())
        return;

    mpMediaWindow = new MediaWindow( this, true );

    mpMediaWindow->setPosSize( Rectangle( Point(), GetOutputSizePixel() ) );
    mpMediaWindow->executeMediaItem( aRestoreItem );

    vcl::Window* pWindow = mpMediaWindow->getWindow();

    if( pWindow )
        pWindow->SetHelpId( HID_AVMEDIA_PLAYERWINDOW );

    mpMediaWindow->show();
}


void MediaFloater::setURL( const OUString& rURL, const OUString& rReferer, bool bPlayImmediately )
{
    if( mpMediaWindow )
    {
        mpMediaWindow->setURL( rURL, rReferer );

        if( mpMediaWindow->isValid() && bPlayImmediately )
            mpMediaWindow->start();
    }
}


void MediaFloater::dispatchCurrentURL()
{
    SfxDispatcher* pDispatcher = GetBindings().GetDispatcher();

    if( pDispatcher )
    {
        OUString url;
        if (mpMediaWindow != nullptr) {
            url = mpMediaWindow->getURL();
        }
        const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, url );
        pDispatcher->ExecuteList(SID_INSERT_AVMEDIA, SfxCallMode::RECORD,
                { &aMediaURLItem });
    }
}

} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
