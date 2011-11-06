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



#include "mediawindowbase_impl.hxx"
#include <avmedia/mediaitem.hxx>
#include "mediamisc.hxx"
#include "mediawindow.hrc"
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/media/XManager.hpp>
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HDL_
#include <com/sun/star/lang/XComponent.hdl>
#endif

#define MEDIA_TIMER_TIMEOUT 100

using namespace ::com::sun::star;

namespace avmedia { namespace priv {

// -----------------------
// - MediaWindowBaseImpl -
// -----------------------

struct ServiceManager
{
    const char* pServiceName;
    sal_Bool    bIsJavaBased;
};

// ---------------------------------------------------------------------


MediaWindowBaseImpl::MediaWindowBaseImpl( MediaWindow* pMediaWindow ) :
    mpMediaWindow( pMediaWindow ),
    mbIsMediaWindowJavaBased( sal_False )
{
}

// ---------------------------------------------------------------------

MediaWindowBaseImpl::~MediaWindowBaseImpl()
{
    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
}

// -------------------------------------------------------------------------

uno::Reference< media::XPlayer > MediaWindowBaseImpl::createPlayer( const ::rtl::OUString& rURL, sal_Bool& rbJavaBased )
{
    uno::Reference< lang::XMultiServiceFactory >    xFactory( ::comphelper::getProcessServiceFactory() );
    uno::Reference< media::XPlayer >                xPlayer;

    rbJavaBased = sal_False;

    if( xFactory.is() )
    {
        static const ServiceManager aServiceManagers[] =
        {
            { AVMEDIA_MANAGER_SERVICE_NAME, AVMEDIA_MANAGER_SERVICE_IS_JAVABASED },
            { AVMEDIA_MANAGER_SERVICE_NAME_FALLBACK1, AVMEDIA_MANAGER_SERVICE_IS_JAVABASED_FALLBACK1 }
        };

        for( sal_uInt32 i = 0; !xPlayer.is() && ( i < ( sizeof( aServiceManagers ) / sizeof( ServiceManager ) ) ); ++i )
        {
            const String aServiceName( aServiceManagers[ i ].pServiceName, RTL_TEXTENCODING_ASCII_US );

            if( aServiceName.Len() )
            {
                OSL_TRACE( "Trying to create media manager service %s", aServiceManagers[ i ].pServiceName );

                try
                {
                    uno::Reference< media::XManager > xManager( xFactory->createInstance( aServiceName ), uno::UNO_QUERY );

                    if( xManager.is() )
                    {
                        xPlayer = uno::Reference< media::XPlayer >( xManager->createPlayer( rURL ), uno::UNO_QUERY );
                    }
                }
                catch( ... )
                {
                }
            }

            if( xPlayer.is() )
            {
                rbJavaBased = aServiceManagers[ i ].bIsJavaBased;
            }
        }
    }

    return xPlayer;
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::setURL( const ::rtl::OUString& rURL )
{
    if( rURL != getURL() )
    {
        INetURLObject aURL( maFileURL = rURL );

        if( mxPlayer.is() )
            mxPlayer->stop();

        if( mxPlayerWindow.is() )
        {
            mxPlayerWindow->setVisible( false );
            mxPlayerWindow.clear();
        }

        mxPlayer.clear();

        if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
            maFileURL = aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );

        mxPlayer = createPlayer( maFileURL, mbIsMediaWindowJavaBased );
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
    uno::Reference< lang::XComponent > xComponent( mxPlayer, uno::UNO_QUERY );
    if( xComponent.is() ) // this stops the player
        xComponent->dispose();

    mxPlayer.clear();

    mpMediaWindow = NULL;
}

// ---------------------------------------------------------------------

bool MediaWindowBaseImpl::hasPreferredSize() const
{
    return( mxPlayerWindow.is() );
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

void MediaWindowBaseImpl::setStopTime( double fTime )
{
    if( mxPlayer.is() )
        mxPlayer->setStopTime( fTime );
}

// ---------------------------------------------------------------------

double MediaWindowBaseImpl::getStopTime() const
{
    return( mxPlayer.is() ? mxPlayer->getStopTime() : 0.0 );
}

// ---------------------------------------------------------------------

void MediaWindowBaseImpl::setRate( double fRate )
{
    if( mxPlayer.is() )
        mxPlayer->setRate( fRate );
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
    rItem.setURL( getURL() );
}

// -------------------------------------------------------------------------

void MediaWindowBaseImpl::executeMediaItem( const MediaItem& rItem )
{
    const sal_uInt32 nMaskSet = rItem.getMaskSet();

    // set URL first
    if( nMaskSet & AVMEDIA_SETMASK_URL )
        setURL( rItem.getURL() );

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
/*
                const double fNewRate = ( ( MEDIASTATE_PLAYFFW == rItem.getState() ) ? AVMEDIA_FFW_PLAYRATE : 1.0 );

                if( getRate() != fNewRate )
                    setRate( fNewRate );
*/
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
