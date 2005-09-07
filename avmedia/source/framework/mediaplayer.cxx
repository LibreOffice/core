/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediaplayer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:38:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "mediaplayer.hxx"
#include "mediawindow.hxx"
#include "mediaitem.hxx"
#include "mediamisc.hxx"
#include "mediacontrol.hrc"
#include "helpids.hrc"

#include <svtools/stritem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

namespace avmedia
{

// ---------------
// - MediaPlayer -
// ---------------

MediaPlayer::MediaPlayer( Window* pParent, USHORT nId, SfxBindings* pBindings, SfxChildWinInfo* pInfo ) :
    SfxChildWindow( pParent, nId )
{
    pWindow = new MediaFloater( pBindings, this, pParent );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    static_cast< MediaFloater* >( pWindow )->Initialize( pInfo );
};

// -----------------------------------------------------------------------------

MediaPlayer::~MediaPlayer()
{
}

// -----------------------------------------------------------------------------

SFX_IMPL_DOCKINGWINDOW( MediaPlayer, SID_AVMEDIA_PLAYER )

// ----------------
// - MediaFloater -
// ----------------

MediaFloater::MediaFloater( SfxBindings* pBindings, SfxChildWindow* pCW, Window* pParent ) :
    SfxDockingWindow( pBindings, pCW, pParent, WB_CLOSEABLE | WB_MOVEABLE | WB_SIZEABLE | WB_DOCKABLE ),
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

void MediaFloater::setURL( const ::rtl::OUString& rURL, bool bPlayImmediately )
{
    if( mpMediaWindow )
    {
        mpMediaWindow->setURL( rURL );

        if( mpMediaWindow->isValid() && bPlayImmediately )
            mpMediaWindow->start();
    }
}

// -----------------------------------------------------------------------------

const ::rtl::OUString& MediaFloater::getURL() const
{
    static const ::rtl::OUString aEmptyStr;
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
