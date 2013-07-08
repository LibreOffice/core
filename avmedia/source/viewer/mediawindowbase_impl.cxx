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

#include "mediawindowbase_impl.hxx"
#include <avmedia/mediaitem.hxx>
#include "mediamisc.hxx"
#include "mediawindow.hrc"
#include <rtl/ustring.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/media/XManager.hpp>

using namespace ::com::sun::star;

namespace avmedia { namespace priv {

// -----------------------
// - MediaWindowBaseImpl -
// -----------------------


MediaWindowBaseImpl::MediaWindowBaseImpl( MediaWindow* pMediaWindow )
    : mpMediaWindow( pMediaWindow )
{
}

// ---------------------------------------------------------------------

MediaWindowBaseImpl::~MediaWindowBaseImpl()
{
}

// -------------------------------------------------------------------------

uno::Reference< media::XPlayer > MediaWindowBaseImpl::createPlayer( const OUString& rURL )
{
    uno::Reference< media::XPlayer > xPlayer;
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
                      "couldn't create media player " AVMEDIA_MANAGER_SERVICE_NAME
                      ", exception '" << e.Message << '\'');
        }
    }

    return xPlayer;
}

void MediaWindowBaseImpl::setURL( const OUString& rURL,
        OUString const& rTempURL)
{
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
                (!mTempFileURL.isEmpty()) ? mTempFileURL : maFileURL );
        onURLChanged();
    }
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::onURLChanged()
{
}

// ---------------------------------------------------------------------

const OUString& MediaWindowBaseImpl::getURL() const
{
    return maFileURL;
}

// ---------------------------------------------------------------------

bool MediaWindowBaseImpl::isValid() const
{
    return( getPlayer().is() );
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::stopPlayingInternal( bool bStop )
{
    if( isPlaying() )
    {
        bStop ? mxPlayer->stop() : mxPlayer->start();
    }
}

// ---------------------------------------------------------------------

MediaWindow* MediaWindowBaseImpl::getMediaWindow() const
{
    return mpMediaWindow;
}

// ---------------------------------------------------------------------

uno::Reference< media::XPlayer > MediaWindowBaseImpl::getPlayer() const
{
    return mxPlayer;
}

// ---------------------------------------------------------------------

uno::Reference< media::XPlayerWindow > MediaWindowBaseImpl::getPlayerWindow() const
{
    return mxPlayerWindow;
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::setPlayerWindow( const uno::Reference< media::XPlayerWindow >& rxPlayerWindow )
{
    mxPlayerWindow = rxPlayerWindow;
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::cleanUp()
{
    uno::Reference< lang::XComponent > xComponent( mxPlayer, uno::UNO_QUERY );
    if( xComponent.is() ) // this stops the player
        xComponent->dispose();

    mxPlayer.clear();

    mpMediaWindow = NULL;
}

// ---------------------------------------------------------------------

Size MediaWindowBaseImpl::getPreferredSize() const
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

// ---------------------------------------------------------------------

bool MediaWindowBaseImpl::setZoom( ::com::sun::star::media::ZoomLevel eLevel )
{
    return( mxPlayerWindow.is() ? mxPlayerWindow->setZoomLevel( eLevel ) : false );
}

// -------------------------------------------------------------------------

::com::sun::star::media::ZoomLevel MediaWindowBaseImpl::getZoom() const
{
    return( mxPlayerWindow.is() ? mxPlayerWindow->getZoomLevel() : media::ZoomLevel_NOT_AVAILABLE );
}

// ---------------------------------------------------------------------

bool MediaWindowBaseImpl::start()
{
    return( mxPlayer.is() ? ( mxPlayer->start(), true ) : false );
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::stop()
{
    if( mxPlayer.is() )
        mxPlayer->stop();
}

// ---------------------------------------------------------------------

bool MediaWindowBaseImpl::isPlaying() const
{
    return( mxPlayer.is() && mxPlayer->isPlaying() );
}

// ---------------------------------------------------------------------

double MediaWindowBaseImpl::getDuration() const
{
    return( mxPlayer.is() ? mxPlayer->getDuration() : 0.0 );
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::setMediaTime( double fTime )
{
    if( mxPlayer.is() )
        mxPlayer->setMediaTime( fTime );
}

// ---------------------------------------------------------------------

double MediaWindowBaseImpl::getMediaTime() const
{
    return( mxPlayer.is() ? mxPlayer->getMediaTime() : 0.0 );
}

// ---------------------------------------------------------------------

double MediaWindowBaseImpl::getRate() const
{
    return( mxPlayer.is() ? mxPlayer->getRate() : 0.0 );
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::setPlaybackLoop( bool bSet )
{
    if( mxPlayer.is() )
        mxPlayer->setPlaybackLoop( bSet );
}

// ---------------------------------------------------------------------

bool MediaWindowBaseImpl::isPlaybackLoop() const
{
    return( mxPlayer.is() ? mxPlayer->isPlaybackLoop() : false );
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::setMute( bool bSet )
{
    if( mxPlayer.is() )
        mxPlayer->setMute( bSet );
}

// ---------------------------------------------------------------------

bool MediaWindowBaseImpl::isMute() const
{
    return( mxPlayer.is() ? mxPlayer->isMute() : false );
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::setVolumeDB( sal_Int16 nVolumeDB )
{
    if( mxPlayer.is() )
        mxPlayer->setVolumeDB( nVolumeDB );
}

// ---------------------------------------------------------------------

sal_Int16 MediaWindowBaseImpl::getVolumeDB() const
{
    return( mxPlayer.is() ? mxPlayer->getVolumeDB() : 0 );
}

// -------------------------------------------------------------------------

void MediaWindowBaseImpl::updateMediaItem( MediaItem& rItem ) const
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
    rItem.setURL( getURL(), &mTempFileURL );
}

// -------------------------------------------------------------------------

void MediaWindowBaseImpl::executeMediaItem( const MediaItem& rItem )
{
    const sal_uInt32 nMaskSet = rItem.getMaskSet();

    // set URL first
    if( nMaskSet & AVMEDIA_SETMASK_URL )
        setURL( rItem.getURL(), rItem.getTempURL() );

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

} // namespace priv
} // namespace avemdia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
