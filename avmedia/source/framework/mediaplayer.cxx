/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

SFX_IMPL_DOCKINGWINDOW( MediaPlayer, SID_AVMEDIA_PLAYER )

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
