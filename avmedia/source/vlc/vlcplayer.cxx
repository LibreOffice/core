#include "vlcplayer.hxx"
#include "vlcwindow.hxx"

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
    : VLC_Base(m_aMutex)
    , mInstance( libvlc_new( sizeof( VLC_ARGS ) / sizeof( VLC_ARGS[0] ), VLC_ARGS ), libvlc_release )
    , mPlayer( libvlc_media_player_new(mInstance.get()), libvlc_media_player_release )
{
}

void SAL_CALL VLCPlayer::start()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    libvlc_media_player_play( mPlayer.get() );
}

void SAL_CALL VLCPlayer::stop()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    libvlc_media_player_stop( mPlayer.get() );
}

::sal_Bool SAL_CALL VLCPlayer::isPlaying()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return (libvlc_media_player_is_playing( mPlayer.get() ) == 1);
}

double SAL_CALL VLCPlayer::getDuration()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    libvlc_media_t* media = libvlc_media_player_get_media( mPlayer.get() );
    return libvlc_media_get_duration( media );
}

void SAL_CALL VLCPlayer::setMediaTime( double fTime )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    libvlc_media_player_set_time( mPlayer.get(), fTime );
}

double SAL_CALL VLCPlayer::getMediaTime()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return libvlc_media_player_get_time( mPlayer.get() );
}

double SAL_CALL VLCPlayer::getRate()
{
    ::osl::MutexGuard aGuard(m_aMutex);
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
    ::osl::MutexGuard aGuard(m_aMutex);
    libvlc_audio_set_volume( mPlayer.get(), nDB );
}

::sal_Int16 SAL_CALL VLCPlayer::getVolumeDB()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return libvlc_audio_get_volume( mPlayer.get() );
}

void SAL_CALL VLCPlayer::setMute( ::sal_Bool bSet )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    libvlc_audio_set_mute( mPlayer.get(), bSet );
}

::sal_Bool SAL_CALL VLCPlayer::isMute()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return libvlc_audio_get_mute( mPlayer.get() );
}

css::awt::Size SAL_CALL VLCPlayer::getPreferredPlayerWindowSize()
{
    return css::awt::Size( 1, 1 );
}

uno::Reference< css::media::XPlayerWindow > SAL_CALL VLCPlayer::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return uno::Reference< css::media::XPlayerWindow >(new VLCWindow());
}

uno::Reference< css::media::XFrameGrabber > SAL_CALL VLCPlayer::createFrameGrabber()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return uno::Reference< css::media::XFrameGrabber >();
}

::rtl::OUString SAL_CALL VLCPlayer::getImplementationName()
{
    return ::rtl::OUString();
}

::sal_Bool SAL_CALL VLCPlayer::supportsService( const ::rtl::OUString& ServiceName )
{
    return false;
}

::uno::Sequence< ::rtl::OUString > SAL_CALL VLCPlayer::getSupportedServiceNames()
{
    return ::uno::Sequence< ::rtl::OUString >();
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */