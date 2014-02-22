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
#include <iostream>
#include "mediawindow_impl.hxx"
#include "mediaevent_impl.hxx"
#include "mediamisc.hxx"
#include "mediawindow.hrc"
#include "helpids.hrc"

#include <algorithm>
#include <cmath>

#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#include <unotools/securityoptions.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/media/XManager.hpp>

using namespace ::com::sun::star;

namespace avmedia { namespace priv {


// - MediaWindowControl -


MediaWindowControl::MediaWindowControl( Window* pParent ) :
    MediaControl( pParent, MEDIACONTROLSTYLE_MULTILINE )
{
}



MediaWindowControl::~MediaWindowControl()
{
}



void MediaWindowControl::update()
{
    MediaItem aItem;

    static_cast< MediaWindowImpl* >( GetParent() )->updateMediaItem( aItem );
    setState( aItem );
}



void  MediaWindowControl::execute( const MediaItem& rItem )
{
    static_cast< MediaWindowImpl* >( GetParent() )->executeMediaItem( rItem );
}


// - MediaChildWindow -


MediaChildWindow::MediaChildWindow( Window* pParent ) :
    SystemChildWindow( pParent, WB_CLIPCHILDREN )
{
}



MediaChildWindow::~MediaChildWindow()
{
}



void MediaChildWindow::MouseMove( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                          rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseMove( rMEvt );
    GetParent()->MouseMove( aTransformedEvent );
}



void MediaChildWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                          rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseButtonDown( rMEvt );
    GetParent()->MouseButtonDown( aTransformedEvent );
}



void MediaChildWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                          rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseButtonUp( rMEvt );
    GetParent()->MouseButtonUp( aTransformedEvent );
}



void MediaChildWindow::KeyInput( const KeyEvent& rKEvt )
{
    SystemChildWindow::KeyInput( rKEvt );
    GetParent()->KeyInput( rKEvt );
}



void MediaChildWindow::KeyUp( const KeyEvent& rKEvt )
{
    SystemChildWindow::KeyUp( rKEvt );
    GetParent()->KeyUp( rKEvt );
}



void MediaChildWindow::Command( const CommandEvent& rCEvt )
{
    const CommandEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rCEvt.GetMousePosPixel() ) ),
                                            rCEvt.GetCommand(), rCEvt.IsMouseEvent(), rCEvt.GetData() );

    SystemChildWindow::Command( rCEvt );
    GetParent()->Command( aTransformedEvent );
}


// - MediaWindowImpl -


MediaWindowImpl::MediaWindowImpl( Window* pParent, MediaWindow* pMediaWindow, bool bInternalMediaControl ) :
    Control( pParent ),
    DropTargetHelper( this ),
    DragSourceHelper( this ),
    mpMediaWindow( pMediaWindow ),
    mxEventsIf( static_cast< ::cppu::OWeakObject* >( mpEvents = new MediaEventListenersImpl( maChildWindow ) ) ),
    maChildWindow( this ),
    mpMediaWindowControl( bInternalMediaControl ? new MediaWindowControl( this ) : NULL ),
    mpEmptyBmpEx( NULL ),
    mpAudioBmpEx( NULL )
{
    maChildWindow.SetBackground( Color( COL_BLACK ) );
    maChildWindow.SetHelpId( HID_AVMEDIA_PLAYERWINDOW );
    maChildWindow.Hide();

    if( mpMediaWindowControl )
    {
        mpMediaWindowControl->SetSizePixel( mpMediaWindowControl->getMinSizePixel() );
        mpMediaWindowControl->Show();
    }
}



MediaWindowImpl::~MediaWindowImpl()
{
    mpEvents->cleanUp();

    if( mxPlayerWindow.is() )
    {
        mxPlayerWindow->removeKeyListener( uno::Reference< awt::XKeyListener >( mxEventsIf, uno::UNO_QUERY ) );
        mxPlayerWindow->removeMouseListener( uno::Reference< awt::XMouseListener >( mxEventsIf, uno::UNO_QUERY ) );
        mxPlayerWindow->removeMouseMotionListener( uno::Reference< awt::XMouseMotionListener >( mxEventsIf, uno::UNO_QUERY ) );

        uno::Reference< lang::XComponent > xComponent( mxPlayerWindow, uno::UNO_QUERY );
        if( xComponent.is() )
            xComponent->dispose();

        mxPlayerWindow.clear();
    }

    uno::Reference< lang::XComponent > xComponent( mxPlayer, uno::UNO_QUERY );
    if( xComponent.is() ) // this stops the player
        xComponent->dispose();

    mxPlayer.clear();

    mpMediaWindow = NULL;

    delete mpEmptyBmpEx;
    delete mpAudioBmpEx;
    delete mpMediaWindowControl;
}

uno::Reference< media::XPlayer > MediaWindowImpl::createPlayer( const OUString& rURL, const OUString& rReferer )
{
    uno::Reference< media::XPlayer > xPlayer;
    if (SvtSecurityOptions().isUntrustedReferer(rReferer)) {
        return xPlayer;
    }

    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    static const char * aServiceManagers[] = {
        AVMEDIA_MANAGER_SERVICE_PREFERRED,
        AVMEDIA_MANAGER_SERVICE_NAME,
// a fallback path just for gstreamer which has
// two significant versions deployed at once ...
#ifdef AVMEDIA_MANAGER_SERVICE_NAME_OLD
        AVMEDIA_MANAGER_SERVICE_NAME_OLD
#endif
    };

    for( sal_uInt32 i = 0; !xPlayer.is() && i < SAL_N_ELEMENTS( aServiceManagers ); ++i )
    {
        const OUString aServiceName( aServiceManagers[ i ],
                                          strlen( aServiceManagers[ i ] ),
                                          RTL_TEXTENCODING_ASCII_US );

        try {
            uno::Reference< media::XManager > xManager (
                    xContext->getServiceManager()->createInstanceWithContext(aServiceName, xContext),
                    uno::UNO_QUERY );
            if( xManager.is() )
                xPlayer = uno::Reference< media::XPlayer >( xManager->createPlayer( rURL ),
                                                            uno::UNO_QUERY );
            else
                SAL_WARN( "avmedia",
                          "failed to create media player service " << aServiceName );
        } catch ( const uno::Exception &e ) {
            SAL_WARN( "avmedia",
                      "couldn't create media player " << aServiceName
                          << ", exception '" << e.Message << '\'');
        }
    }

    return xPlayer;
}

void MediaWindowImpl::setURL( const OUString& rURL,
        OUString const& rTempURL, OUString const& rReferer)
{
    maReferer = rReferer;
    if( rURL != getURL() )
    {
        if( mxPlayer.is() )
            mxPlayer->stop();

        if( mxPlayerWindow.is() )
        {
            mxPlayerWindow->setVisible( false );
            mxPlayerWindow.clear();
        }

        mxPlayer.clear();
        mTempFileURL = OUString();

        if (!rTempURL.isEmpty())
        {
            maFileURL = rURL;
            mTempFileURL = rTempURL;
        }
        else
        {
            INetURLObject aURL( rURL );

            if (aURL.GetProtocol() != INET_PROT_NOT_VALID)
                maFileURL = aURL.GetMainURL(INetURLObject::DECODE_UNAMBIGUOUS);
            else
                maFileURL = rURL;
        }

        mxPlayer = createPlayer(
                (!mTempFileURL.isEmpty()) ? mTempFileURL : maFileURL, rReferer );
        onURLChanged();
    }
}

const OUString& MediaWindowImpl::getURL() const
{
    return maFileURL;
}

bool MediaWindowImpl::isValid() const
{
    return( mxPlayer.is() );
}

Size MediaWindowImpl::getPreferredSize() const
{
    Size aRet;

    if( mxPlayer.is() )
    {
        awt::Size aPrefSize( mxPlayer->getPreferredPlayerWindowSize() );

        aRet.Width() = aPrefSize.Width;
        aRet.Height() = aPrefSize.Height;
    }

    return aRet;
}

bool MediaWindowImpl::start()
{
    return( mxPlayer.is() ? ( mxPlayer->start(), true ) : false );
}

void MediaWindowImpl::updateMediaItem( MediaItem& rItem ) const
{
    if( isPlaying() )
        rItem.setState( ( getRate() > 1.0 ) ? MEDIASTATE_PLAYFFW : MEDIASTATE_PLAY );
    else
        rItem.setState( ( 0.0 == getMediaTime() ) ? MEDIASTATE_STOP : MEDIASTATE_PAUSE );

    rItem.setDuration( getDuration() );
    rItem.setTime( getMediaTime() );
    rItem.setLoop( isPlaybackLoop() );
    rItem.setMute( isMute() );
    rItem.setVolumeDB( getVolumeDB() );
    rItem.setZoom( getZoom() );
    rItem.setURL( getURL(), mTempFileURL, maReferer );
}

void MediaWindowImpl::executeMediaItem( const MediaItem& rItem )
{
    const sal_uInt32 nMaskSet = rItem.getMaskSet();

    // set URL first
    if( nMaskSet & AVMEDIA_SETMASK_URL )
        setURL( rItem.getURL(), rItem.getTempURL(), rItem.getReferer() );

    // set different states next
    if( nMaskSet & AVMEDIA_SETMASK_TIME )
        setMediaTime( ::std::min( rItem.getTime(), getDuration() ) );

    if( nMaskSet & AVMEDIA_SETMASK_LOOP )
        setPlaybackLoop( rItem.isLoop() );

    if( nMaskSet & AVMEDIA_SETMASK_MUTE )
        setMute( rItem.isMute() );

    if( nMaskSet & AVMEDIA_SETMASK_VOLUMEDB )
        setVolumeDB( rItem.getVolumeDB() );

    if( nMaskSet & AVMEDIA_SETMASK_ZOOM )
        setZoom( rItem.getZoom() );

    // set play state at last
    if( nMaskSet & AVMEDIA_SETMASK_STATE )
    {
        switch( rItem.getState() )
        {
            case( MEDIASTATE_PLAY ):
            case( MEDIASTATE_PLAYFFW ):
            {

                if( !isPlaying() )
                    start();
            }
            break;

            case( MEDIASTATE_PAUSE ):
            {
                if( isPlaying() )
                    stop();
            }
            break;

            case( MEDIASTATE_STOP ):
            {
                if( isPlaying() )
                {
                    setMediaTime( 0.0 );
                    stop();
                    setMediaTime( 0.0 );
                }
            }
            break;
        }
    }
}

bool MediaWindowImpl::setZoom( ::com::sun::star::media::ZoomLevel eLevel )
{
    return( mxPlayerWindow.is() ? mxPlayerWindow->setZoomLevel( eLevel ) : false );
}

::com::sun::star::media::ZoomLevel MediaWindowImpl::getZoom() const
{
    return( mxPlayerWindow.is() ? mxPlayerWindow->getZoomLevel() : media::ZoomLevel_NOT_AVAILABLE );
}

void MediaWindowImpl::stop()
{
    if( mxPlayer.is() )
        mxPlayer->stop();
}

bool MediaWindowImpl::isPlaying() const
{
    return( mxPlayer.is() && mxPlayer->isPlaying() );
}

double MediaWindowImpl::getDuration() const
{
    return( mxPlayer.is() ? mxPlayer->getDuration() : 0.0 );
}

void MediaWindowImpl::setMediaTime( double fTime )
{
    if( mxPlayer.is() )
        mxPlayer->setMediaTime( fTime );
}

double MediaWindowImpl::getMediaTime() const
{
    return( mxPlayer.is() ? mxPlayer->getMediaTime() : 0.0 );
}

double MediaWindowImpl::getRate() const
{
    return( mxPlayer.is() ? mxPlayer->getRate() : 0.0 );
}

void MediaWindowImpl::setPlaybackLoop( bool bSet )
{
    if( mxPlayer.is() )
        mxPlayer->setPlaybackLoop( bSet );
}

bool MediaWindowImpl::isPlaybackLoop() const
{
    return( mxPlayer.is() ? mxPlayer->isPlaybackLoop() : false );
}

void MediaWindowImpl::setMute( bool bSet )
{
    if( mxPlayer.is() )
        mxPlayer->setMute( bSet );
}

bool MediaWindowImpl::isMute() const
{
    return( mxPlayer.is() ? mxPlayer->isMute() : false );
}

void MediaWindowImpl::setVolumeDB( sal_Int16 nVolumeDB )
{
    if( mxPlayer.is() )
        mxPlayer->setVolumeDB( nVolumeDB );
}

sal_Int16 MediaWindowImpl::getVolumeDB() const
{
    return( mxPlayer.is() ? mxPlayer->getVolumeDB() : 0 );
}

void MediaWindowImpl::stopPlayingInternal( bool bStop )
{
    if( isPlaying() )
    {
        bStop ? mxPlayer->stop() : mxPlayer->start();
    }
}

void MediaWindowImpl::onURLChanged()
{
    if( mxPlayer.is() )
    {
        uno::Sequence< uno::Any >              aArgs( 3 );
        uno::Reference< media::XPlayerWindow > xPlayerWindow;
        const Point                            aPoint;
        const Size                             aSize( maChildWindow.GetSizePixel() );
        const sal_Int32                        nWndHandle = 0;

        aArgs[ 0 ] = uno::makeAny( nWndHandle );
        aArgs[ 1 ] = uno::makeAny( awt::Rectangle( aPoint.X(), aPoint.Y(), aSize.Width(), aSize.Height() ) );
        aArgs[ 2 ] = uno::makeAny( reinterpret_cast< sal_IntPtr >( &maChildWindow ) );

        try
        {
            xPlayerWindow = mxPlayer->createPlayerWindow( aArgs );
        }
        catch( uno::RuntimeException )
        {
            // happens eg, on MacOSX where Java frames cannot be created from X11 window handles
        }

        mxPlayerWindow = xPlayerWindow;

        if( xPlayerWindow.is() )
        {
            xPlayerWindow->addKeyListener( uno::Reference< awt::XKeyListener >( mxEventsIf, uno::UNO_QUERY ) );
            xPlayerWindow->addMouseListener( uno::Reference< awt::XMouseListener >( mxEventsIf, uno::UNO_QUERY ) );
            xPlayerWindow->addMouseMotionListener( uno::Reference< awt::XMouseMotionListener >( mxEventsIf, uno::UNO_QUERY ) );
            xPlayerWindow->addFocusListener( uno::Reference< awt::XFocusListener >( mxEventsIf, uno::UNO_QUERY ) );
        }
    }
    else
        mxPlayerWindow.clear();

    if( mxPlayerWindow.is() )
        maChildWindow.Show();
    else
        maChildWindow.Hide();

    if( mpMediaWindowControl )
    {
        MediaItem aItem;

        updateMediaItem( aItem );
        mpMediaWindowControl->setState( aItem );
    }
}



void MediaWindowImpl::setPosSize( const Rectangle& rRect )
{
    SetPosSizePixel( rRect.TopLeft(), rRect.GetSize() );
}



void MediaWindowImpl::setPointer( const Pointer& rPointer )
{
    SetPointer( rPointer );
    maChildWindow.SetPointer( rPointer );

    if( mxPlayerWindow.is() )
    {
        long nPointer;

        switch( rPointer.GetStyle() )
        {
            case( POINTER_CROSS ): nPointer = awt::SystemPointer::CROSS; break;
            case( POINTER_HAND ): nPointer = awt::SystemPointer::HAND; break;
            case( POINTER_MOVE ): nPointer = awt::SystemPointer::MOVE; break;
            case( POINTER_WAIT ): nPointer = awt::SystemPointer::WAIT; break;

            default: nPointer = awt::SystemPointer::ARROW; break;
        }

        mxPlayerWindow->setPointerType( nPointer );
    }
}



void MediaWindowImpl::Resize()
{
    const Size                              aCurSize( GetOutputSizePixel() );
    const sal_Int32                         nOffset( mpMediaWindowControl ? AVMEDIA_CONTROLOFFSET : 0 );
    Size                                    aPlayerWindowSize( aCurSize.Width() - ( nOffset << 1 ),
                                                               aCurSize.Height() - ( nOffset << 1 ) );

    if( mpMediaWindowControl )
    {
        const sal_Int32 nControlHeight = mpMediaWindowControl->GetSizePixel().Height();
        const sal_Int32 nControlY = ::std::max( aCurSize.Height() - nControlHeight - nOffset, 0L );

        aPlayerWindowSize.Height() = ( nControlY - ( nOffset << 1 ) );
        mpMediaWindowControl->SetPosSizePixel( Point( nOffset, nControlY ), Size( aCurSize.Width() - ( nOffset << 1 ), nControlHeight ) );
    }

    if( mxPlayerWindow.is() )
        mxPlayerWindow->setPosSize( 0, 0, aPlayerWindowSize.Width(), aPlayerWindowSize.Height(), 0 );

    maChildWindow.SetPosSizePixel( Point( 0, 0 ), aPlayerWindowSize );
}



void MediaWindowImpl::StateChanged( StateChangedType eType )
{
    if( mxPlayerWindow.is() )
    {
        // stop playing when going disabled or hidden
        switch( eType )
        {
            case STATE_CHANGE_VISIBLE:
            {
                stopPlayingInternal( !IsVisible() );
                mxPlayerWindow->setVisible( IsVisible() );
            }
            break;

            case STATE_CHANGE_ENABLE:
            {
                stopPlayingInternal( !IsEnabled() );
                mxPlayerWindow->setEnable( IsEnabled() );
            }
            break;

            default:
            break;
        }
    }
}



void MediaWindowImpl::Paint( const Rectangle& )
{
    BitmapEx* pLogo = NULL;

    if( !mxPlayer.is() )
    {
        if( !mpEmptyBmpEx )
            mpEmptyBmpEx = new BitmapEx( AVMEDIA_RESID( AVMEDIA_BMP_EMPTYLOGO ) );

        pLogo = mpEmptyBmpEx;
    }
    else if( !mxPlayerWindow.is() )
    {
        if( !mpAudioBmpEx )
            mpAudioBmpEx = new BitmapEx( AVMEDIA_RESID( AVMEDIA_BMP_AUDIOLOGO ) );

        pLogo = mpAudioBmpEx;
    }

    const Point     aBasePos( maChildWindow.GetPosPixel() );
    const Rectangle aVideoRect( aBasePos, maChildWindow.GetSizePixel() );

    if( pLogo && !pLogo->IsEmpty() && ( aVideoRect.GetWidth() > 0 ) && ( aVideoRect.GetHeight() > 0 ) )
    {
        Size        aLogoSize( pLogo->GetSizePixel() );
        const Color aBackgroundColor( 67, 67, 67 );

        SetLineColor( aBackgroundColor );
        SetFillColor( aBackgroundColor );
        DrawRect( aVideoRect );

        if( ( aLogoSize.Width() > aVideoRect.GetWidth() || aLogoSize.Height() > aVideoRect.GetHeight() ) &&
            ( aLogoSize.Height() > 0 ) )
        {
            const double fLogoWH = (double) aLogoSize.Width() / aLogoSize.Height();

            if( fLogoWH < ( (double) aVideoRect.GetWidth() / aVideoRect.GetHeight() ) )
            {
                aLogoSize.Width() = (long) ( aVideoRect.GetHeight() * fLogoWH );
                aLogoSize.Height()= aVideoRect.GetHeight();
            }
            else
            {
                aLogoSize.Width() = aVideoRect.GetWidth();
                aLogoSize.Height()= (long) ( aVideoRect.GetWidth() / fLogoWH );
            }
        }

        DrawBitmapEx( Point( aBasePos.X() + ( ( aVideoRect.GetWidth() - aLogoSize.Width() ) >> 1 ),
                             aBasePos.Y() + ( ( aVideoRect.GetHeight() - aLogoSize.Height() ) >> 1 ) ),
                      aLogoSize, *pLogo );
    }
}



void MediaWindowImpl::GetFocus()
{
}



void MediaWindowImpl::MouseMove( const MouseEvent& rMEvt )
{
    if( mpMediaWindow )
        mpMediaWindow->MouseMove( rMEvt );
}



void MediaWindowImpl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( mpMediaWindow )
        mpMediaWindow->MouseButtonDown( rMEvt );
}



void MediaWindowImpl::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( mpMediaWindow )
        mpMediaWindow->MouseButtonUp( rMEvt );
}



void MediaWindowImpl::KeyInput( const KeyEvent& rKEvt )
{
    if( mpMediaWindow )
        mpMediaWindow->KeyInput( rKEvt );
}



void MediaWindowImpl::KeyUp( const KeyEvent& rKEvt )
{
    if( mpMediaWindow )
        mpMediaWindow->KeyUp( rKEvt );
}



void MediaWindowImpl::Command( const CommandEvent& rCEvt )
{
    if( mpMediaWindow )
        mpMediaWindow->Command( rCEvt );
}



sal_Int8 MediaWindowImpl::AcceptDrop( const AcceptDropEvent& rEvt )
{
    return( mpMediaWindow ? mpMediaWindow->AcceptDrop( rEvt ) : 0 );
}



sal_Int8 MediaWindowImpl::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    return( mpMediaWindow ? mpMediaWindow->ExecuteDrop( rEvt ) : 0 );
}



void MediaWindowImpl::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
    if( mpMediaWindow )
        mpMediaWindow->StartDrag( nAction, rPosPixel );
}

} // namespace priv
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
