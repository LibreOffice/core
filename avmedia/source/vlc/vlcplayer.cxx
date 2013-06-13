#include "vlcplayer.hxx"

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

void SAL_CALL VLCPlayer::start()
{
}

void SAL_CALL VLCPlayer::stop()
{
}

::sal_Bool SAL_CALL VLCPlayer::isPlaying()
{
    return false;
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
    return 0.f;
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