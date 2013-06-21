#include "vlcplayer.hxx"

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

const char * const VLC_ARGS[] = {
    "-I",
    "dummy",
    "--ignore-config",
    "--verbose=-1",
    "--quiet"
};

VLCPlayer::VLCPlayer()
    : mInstance( libvlc_new( sizeof( VLC_ARGS ) / sizeof( VLC_ARGS[0] ), VLC_ARGS ), libvlc_release )
    , mPlayer( libvlc_media_player_new(mInstance.get()), libvlc_media_player_release )
{
}

void SAL_CALL VLCPlayer::start()
{
    libvlc_media_player_play( mPlayer.get() );
}

void SAL_CALL VLCPlayer::stop()
{
    libvlc_media_player_stop( mPlayer.get() );
}

::sal_Bool SAL_CALL VLCPlayer::isPlaying()
{
    return (libvlc_media_player_is_playing( mPlayer.get() ) == 1);
}

double SAL_CALL VLCPlayer::getDuration()
{
    return 0.f;
}

void SAL_CALL VLCPlayer::setMediaTime( double fTime )
{
}

double SAL_CALL VLCPlayer::getMediaTime()
{
    return 0.f;
}

double SAL_CALL VLCPlayer::getRate()
{
    return libvlc_media_player_get_rate( mPlayer.get() );
}

void SAL_CALL VLCPlayer::setPlaybackLoop( ::sal_Bool bSet )
{
}

::sal_Bool SAL_CALL VLCPlayer::isPlaybackLoop()
{
    return false;
}

void SAL_CALL VLCPlayer::setVolumeDB( ::sal_Int16 nDB )
{
}

::sal_Int16 SAL_CALL VLCPlayer::getVolumeDB()
{
    return 1;
}

void SAL_CALL VLCPlayer::setMute( ::sal_Bool bSet )
{
}

::sal_Bool SAL_CALL VLCPlayer::isMute()
{
    return false;
}

css::awt::Size SAL_CALL VLCPlayer::getPreferredPlayerWindowSize()
{
    return css::awt::Size( 1, 1 );
}

uno::Reference< css::media::XPlayerWindow > SAL_CALL VLCPlayer::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
{
    return uno::Reference< css::media::XPlayerWindow >();
}
uno::Reference< css::media::XFrameGrabber > SAL_CALL VLCPlayer::createFrameGrabber()
{
    return uno::Reference< css::media::XFrameGrabber >();
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */