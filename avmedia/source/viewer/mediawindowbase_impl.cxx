/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediawindowbase_impl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:44:11 $
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

#include "mediawindowbase_impl.hxx"
#include "mediaitem.hxx"
#include "mediamisc.hxx"
#include "mediawindow.hrc"
#include <tools/urlobj.hxx>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_MEDIA_XMANAGER_HPP_
#include <com/sun/star/media/XManager.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HDL_
#include <com/sun/star/lang/XComponent.hdl>
#endif

#define MEDIA_TIMER_TIMEOUT 100

using namespace ::com::sun::star;

namespace avmedia { namespace priv {

// -----------------------
// - MediaWindowBaseImpl -
// -----------------------

MediaWindowBaseImpl::MediaWindowBaseImpl( MediaWindow* pMediaWindow ) :
    mpMediaWindow( pMediaWindow )
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

            uno::Reference< ::com::sun::star::media::XManager > xManager(
                xFactory->createInstance( ::rtl::OUString::createFromAscii( AVMEDIA_MANAGER_SERVICE_NAME ) ),
                uno::UNO_QUERY );

            if( xManager.is() )
            {
                xPlayer = uno::Reference< ::com::sun::star::media::XPlayer >(
                    xManager->createPlayer( rURL ), uno::UNO_QUERY );
            }
        }
        catch( ... )
        {
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

        mxPlayer = createPlayer( maFileURL );
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
        if( bStop )
            mxPlayer->start();
        else
            mxPlayer->stop();
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
