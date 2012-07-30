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

#include "mediawindowbase_impl.hxx"
#include <avmedia/mediaitem.hxx>
#include "mediamisc.hxx"
#include "mediawindow.hrc"
#include <rtl/oustringostreaminserter.hxx>
#include <sal/log.hxx>
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
    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
}

// -------------------------------------------------------------------------

uno::Reference< media::XPlayer > MediaWindowBaseImpl::createPlayer( const ::rtl::OUString& rURL )
{
    uno::Reference< lang::XMultiServiceFactory >    xFactory( ::comphelper::getProcessServiceFactory() );
    uno::Reference< media::XPlayer >                xPlayer;

    if( xFactory.is() )
    {
        try
        {
            uno::Reference< ::com::sun::star::media::XManager > xManager;

            try {
                xManager = uno::Reference< ::com::sun::star::media::XManager >(
                        xFactory->createInstance( AVMEDIA_MANAGER_SERVICE_NAME ),
                        uno::UNO_QUERY );
            } catch ( const uno::Exception &e ) {
            }

// a fallback path just for gstreamer which has
// two significant versions deployed at once ...
#ifdef AVMEDIA_MANAGER_SERVICE_NAME_OLD
            if( !xManager.is() )
            {
                xManager = uno::Reference< ::com::sun::star::media::XManager >(
                        xFactory->createInstance( AVMEDIA_MANAGER_SERVICE_NAME_OLD ),
                        uno::UNO_QUERY );
            }
#endif

            if( xManager.is() )
            {
                xPlayer = uno::Reference< ::com::sun::star::media::XPlayer >(
                    xManager->createPlayer( rURL ), uno::UNO_QUERY );
            }
            else
                SAL_WARN(
                    "avmedia",
                    ("failed to create media player service "
                     AVMEDIA_MANAGER_SERVICE_NAME));
        }
        catch( const uno::Exception &e )
        {
            SAL_WARN(
                "avmedia",
                "couldn't create media player " AVMEDIA_MANAGER_SERVICE_NAME
                    ", exception '" << e.Message << '\'');
        }
    }

    return xPlayer;
}

void MediaWindowBaseImpl::setURL( const ::rtl::OUString& rURL,
        ::rtl::OUString const& rTempURL)
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
        mTempFileURL = ::rtl::OUString();

        if (rTempURL.getLength())
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
                (mTempFileURL.getLength()) ? mTempFileURL : maFileURL );
        onURLChanged();
    }
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::onURLChanged()
{
}

// ---------------------------------------------------------------------

const ::rtl::OUString& MediaWindowBaseImpl::getURL() const
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
    if( mxPlayer.is() )
    {
        mxPlayer->stop();

        uno::Reference< lang::XComponent > xComponent( mxPlayer, uno::UNO_QUERY );

        if( xComponent.is() )
            xComponent->dispose();

        mxPlayer.clear();
    }

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
    return( mxPlayerWindow.is() ? mxPlayerWindow->getZoomLevel() : ::com::sun::star::media::ZoomLevel_NOT_AVAILABLE );
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
