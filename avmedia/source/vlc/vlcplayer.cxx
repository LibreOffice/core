#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include "vlcplayer.hxx"
#include "vlcwindow.hxx"
#include "vlcframegrabber.hxx"
#include "wrapper/Instance.hxx"

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

const ::rtl::OUString AVMEDIA_VLC_PLAYER_IMPLEMENTATIONNAME = "com.sun.star.comp.avmedia.Player_VLC";
const ::rtl::OUString AVMEDIA_VLC_PLAYER_SERVICENAME = "com.sun.star.media.Player_VLC";

const char * const VLC_ARGS[] = {
    "-Vdummy",
    "--snapshot-format=png",
    "--ffmpeg-threads",
    "--verbose=-1",
};

const int MS_IN_SEC = 1000; // Millisec in sec

VLCPlayer::VLCPlayer( const rtl::OUString& url )
    : VLC_Base(m_aMutex)
    , mInstance( VLC_ARGS )
    , mMedia( url, mInstance )
    , mPlayer( mMedia )
    , mUrl( url )
    , mPlaybackLoop( false )
{
}

const rtl::OUString& VLCPlayer::url() const
{
    return mUrl;
}

void SAL_CALL VLCPlayer::start()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.play();
}

void SAL_CALL VLCPlayer::stop()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.pause();
}

::sal_Bool SAL_CALL VLCPlayer::isPlaying()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return mPlayer.isPlaying();
}

double SAL_CALL VLCPlayer::getDuration()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return static_cast<double>( mPlayer.getLength() ) / MS_IN_SEC;
}

void SAL_CALL VLCPlayer::setMediaTime( double fTime )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( fTime < 0.00000001 && !mPlayer.isPlaying() )
    {
        mPlayer.stop();
    }

    mPlayer.setTime( fTime * MS_IN_SEC );
}

double SAL_CALL VLCPlayer::getMediaTime()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return static_cast<double>( mPlayer.getTime() ) / MS_IN_SEC;
}

double SAL_CALL VLCPlayer::getRate()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return mPlayer.getRate();
}

namespace
{
    void EventHandler( const libvlc_event_t *evemt, void *pData )
    {
        switch (evemt->type)
        {
        case libvlc_MediaPlayerEndReached:
            VLC::Player& player = *static_cast< VLC::Player* >( pData );

            player.stop();
            player.play();
            break;
        }
    }
}

void SAL_CALL VLCPlayer::setPlaybackLoop( ::sal_Bool bSet )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlaybackLoop = bSet;

    libvlc_event_manager_t *manager = libvlc_media_player_event_manager( mPlayer );

    if ( bSet )
        libvlc_event_attach( manager, libvlc_MediaPlayerEndReached, EventHandler, &mPlayer );
    else
        libvlc_event_detach( manager, libvlc_MediaPlayerEndReached, EventHandler, &mPlayer );
}

::sal_Bool SAL_CALL VLCPlayer::isPlaybackLoop()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return mPlaybackLoop;
}

void SAL_CALL VLCPlayer::setVolumeDB( ::sal_Int16 nDB )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.setVolume( static_cast<sal_Int16>( ( nDB + 40 ) * 10.0  / 4 ) );
}

::sal_Int16 SAL_CALL VLCPlayer::getVolumeDB()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return static_cast<sal_Int16>( mPlayer.getVolume() / 10.0 * 4 - 40 );
}

void SAL_CALL VLCPlayer::setMute( ::sal_Bool bSet )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.setMute( bSet );
}

::sal_Bool SAL_CALL VLCPlayer::isMute()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return mPlayer.getMute();
}

css::awt::Size SAL_CALL VLCPlayer::getPreferredPlayerWindowSize()
{
    return css::awt::Size( 480, 360 );
}

namespace
{
    // TODO: Move this function to the common space for avoiding duplication with
    // gstreamer/gstwindow::createPlayerWindow functionality
    int GetWindowID( const uno::Sequence< uno::Any >& arguments )
    {
        if (arguments.getLength() <= 2)
            return -1;

        sal_IntPtr pIntPtr = 0;

        arguments[ 2 ] >>= pIntPtr;

        SystemChildWindow *pParentWindow = reinterpret_cast< SystemChildWindow* >( pIntPtr );

        const SystemEnvData* pEnvData = pParentWindow ? pParentWindow->GetSystemData() : NULL;

        if (pEnvData == NULL)
            return -1;

        // Explicit converts from long to int
        const int id = static_cast<int>( pEnvData->aWindow );

        return id;
    }
}

uno::Reference< css::media::XPlayerWindow > SAL_CALL VLCPlayer::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    VLCWindow * const window = new VLCWindow( *this );

    const int winID = GetWindowID( aArguments );

    if (winID != -1)
    {
#if defined(WIN32) && !defined(UNIX)
        //TODO: Not works, will be crashed
#else
        mPlayer.setXWindow( winID );
#endif
    }

    return uno::Reference< css::media::XPlayerWindow >( window );
}

uno::Reference< css::media::XFrameGrabber > SAL_CALL VLCPlayer::createFrameGrabber()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    VLCFrameGrabber *frameGrabber = new VLCFrameGrabber( mPlayer, mUrl );
    return uno::Reference< css::media::XFrameGrabber >( frameGrabber );
}

::rtl::OUString SAL_CALL VLCPlayer::getImplementationName()
{
    return AVMEDIA_VLC_PLAYER_IMPLEMENTATIONNAME;
}

::sal_Bool SAL_CALL VLCPlayer::supportsService( const ::rtl::OUString& serviceName )
{
    return serviceName == AVMEDIA_VLC_PLAYER_SERVICENAME;
}

::uno::Sequence< ::rtl::OUString > SAL_CALL VLCPlayer::getSupportedServiceNames()
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = AVMEDIA_VLC_PLAYER_SERVICENAME;
    return aRet;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
