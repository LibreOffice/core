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

// ---------------
// - MediaPlayer -
// ---------------

MediaPlayer::MediaPlayer( Window* _pParent, sal_uInt16 nId, SfxBindings* _pBindings, SfxChildWinInfo* pInfo ) :
    SfxChildWindow( _pParent, nId )
{
    pWindow = new MediaFloater( _pBindings, this, _pParent );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    static_cast< MediaFloater* >( pWindow )->Initialize( pInfo );
};

// -----------------------------------------------------------------------------

MediaPlayer::~MediaPlayer()
{
}

// -----------------------------------------------------------------------------

SFX_IMPL_DOCKINGWINDOW_WITHID( MediaPlayer, SID_AVMEDIA_PLAYER )

// ----------------
// - MediaFloater -
// ----------------

MediaFloater::MediaFloater( SfxBindings* _pBindings, SfxChildWindow* pCW, Window* pParent ) :
    SfxDockingWindow( _pBindings, pCW, pParent, WB_CLOSEABLE | WB_MOVEABLE | WB_SIZEABLE | WB_DOCKABLE ),
    mpMediaWindow( new MediaWindow( this, true ) )
{
    const Size aSize( 378, 256 );

    SetPosSizePixel( Point( 0, 0 ), aSize );
    SetMinOutputSizePixel( aSize );
    SetText( String( AVMEDIA_RESID( AVMEDIA_STR_MEDIAPLAYER ) ) );
    implInit();
    mpMediaWindow->show();
}

// -----------------------------------------------------------------------------

MediaFloater::~MediaFloater()
{
    delete mpMediaWindow;
    mpMediaWindow = NULL;
}

// -----------------------------------------------------------------------------

void MediaFloater::implInit()
{
}

// -------------------------------------------------------------------------

void MediaFloater::Resize()
{
    SfxDockingWindow::Resize();

    if( mpMediaWindow )
        mpMediaWindow->setPosSize( Rectangle( Point(), GetOutputSizePixel() ) );
}

// -----------------------------------------------------------------------------

void MediaFloater::ToggleFloatingMode()
{
    ::avmedia::MediaItem aRestoreItem;

    mpMediaWindow->updateMediaItem( aRestoreItem );
    delete mpMediaWindow;
    mpMediaWindow = NULL;

    SfxDockingWindow::ToggleFloatingMode();

    mpMediaWindow = new MediaWindow( this, true );

    mpMediaWindow->setPosSize( Rectangle( Point(), GetOutputSizePixel() ) );
    mpMediaWindow->executeMediaItem( aRestoreItem );

    Window* pWindow = mpMediaWindow->getWindow();

    if( pWindow )
        pWindow->SetHelpId( HID_AVMEDIA_PLAYERWINDOW );

    mpMediaWindow->show();
}

// -----------------------------------------------------------------------------

void MediaFloater::setURL( const OUString& rURL, bool bPlayImmediately )
{
    if( mpMediaWindow )
    {
        mpMediaWindow->setURL( rURL );

        if( mpMediaWindow->isValid() && bPlayImmediately )
            mpMediaWindow->start();
    }
}

// -----------------------------------------------------------------------------

const OUString& MediaFloater::getURL() const
{
    static const OUString aEmptyStr;
    return( mpMediaWindow ? mpMediaWindow->getURL() : aEmptyStr );
}

// -----------------------------------------------------------------------------

void MediaFloater::dispatchCurrentURL()
{
    SfxDispatcher* pDispatcher = GetBindings().GetDispatcher();

    if( pDispatcher )
    {
        const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, getURL() );
        pDispatcher->Execute( SID_INSERT_AVMEDIA, SFX_CALLMODE_RECORD, &aMediaURLItem, 0L );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
