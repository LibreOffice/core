#include <boost/bind.hpp>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include "vlcplayer.hxx"
#include "vlcwindow.hxx"
#include "vlcframegrabber.hxx"
#include "wrapper/Instance.hxx"

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

namespace
{
    const ::rtl::OUString AVMEDIA_VLC_PLAYER_IMPLEMENTATIONNAME = "com.sun.star.comp.avmedia.Player_VLC";
    const ::rtl::OUString AVMEDIA_VLC_PLAYER_SERVICENAME = "com.sun.star.media.Player_VLC";

    const int MS_IN_SEC = 1000; // Millisec in sec

    const char * const VLC_ARGS[] = {
        "-Vdummy",
        "--snapshot-format=png",
        "--ffmpeg-threads",
        "--verbose=2"
    };
}

VLCPlayer::VLCPlayer( const rtl::OUString& iurl, boost::shared_ptr<VLC::EventHandler> eh )
    : VLC_Base(m_aMutex)
    , mEventHandler( eh )
    , mInstance( sizeof( VLC_ARGS ) / sizeof( VLC_ARGS[0] ), VLC_ARGS )
    , mMedia( iurl, mInstance )
    , mPlayer( mMedia )
    , mEventManager( mPlayer, mEventHandler )
    , mUrl( iurl )
    , mPlaybackLoop( false )
{
    mPlayer.setMouseHandling(false);
}

const rtl::OUString& VLCPlayer::url() const
{
    return mUrl;
}

void SAL_CALL VLCPlayer::start() throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.play();
}

void SAL_CALL VLCPlayer::stop() throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.pause();
}

::sal_Bool SAL_CALL VLCPlayer::isPlaying() throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return mPlayer.isPlaying();
}

double SAL_CALL VLCPlayer::getDuration() throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return static_cast<double>( mPlayer.getLength() ) / MS_IN_SEC;
}

void SAL_CALL VLCPlayer::setMediaTime( double fTime ) throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( fTime < 0.00000001 && !mPlayer.isPlaying() )
    {
        mPlayer.stop();
    }

    mPlayer.setTime( fTime * MS_IN_SEC );
}

double SAL_CALL VLCPlayer::getMediaTime() throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return static_cast<double>( mPlayer.getTime() ) / MS_IN_SEC;
}

double SAL_CALL VLCPlayer::getRate() throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return mPlayer.getRate();
}

void VLCPlayer::replay()
{
    setPlaybackLoop( false );
    stop();
    setMediaTime( 0 );
    start();
}

void SAL_CALL VLCPlayer::setPlaybackLoop( ::sal_Bool bSet ) throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlaybackLoop = bSet;

    if ( bSet )
        mEventManager.onEndReached(boost::bind(&VLCPlayer::replay, this));
    else
        mEventManager.onEndReached();
}

::sal_Bool SAL_CALL VLCPlayer::isPlaybackLoop() throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return mPlaybackLoop;
}

void SAL_CALL VLCPlayer::setVolumeDB( ::sal_Int16 nDB ) throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.setVolume( static_cast<sal_Int16>( ( nDB + 40 ) * 10.0  / 4 ) );
}

::sal_Int16 SAL_CALL VLCPlayer::getVolumeDB() throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return static_cast<sal_Int16>( mPlayer.getVolume() / 10.0 * 4 - 40 );
}

void SAL_CALL VLCPlayer::setMute( ::sal_Bool bSet ) throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.setMute( bSet );
}

::sal_Bool SAL_CALL VLCPlayer::isMute() throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return mPlayer.getMute();
}

css::awt::Size SAL_CALL VLCPlayer::getPreferredPlayerWindowSize() throw ( ::com::sun::star::uno::RuntimeException )
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

#if defined WNT
        // Explicit converts from HWND to int
        const int id = reinterpret_cast<int>( pEnvData->hWnd );
#else
        // Explicit converts from long to int
        const int id = static_cast<int>( pEnvData->aWindow );
#endif

        return id;
    }
}

uno::Reference< css::media::XPlayerWindow > SAL_CALL VLCPlayer::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
     throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    VLCWindow * const window = new VLCWindow( *this );

    const int winID = GetWindowID( aArguments );

    if ( winID != -1 )
    {
        mPlayer.setWindow( winID );
    }

    return uno::Reference< css::media::XPlayerWindow >( window );
}

uno::Reference< css::media::XFrameGrabber > SAL_CALL VLCPlayer::createFrameGrabber()
     throw ( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    VLCFrameGrabber *frameGrabber = new VLCFrameGrabber( mPlayer, mEventHandler, mUrl );
    return uno::Reference< css::media::XFrameGrabber >( frameGrabber );
}

::rtl::OUString SAL_CALL VLCPlayer::getImplementationName()
     throw ( ::com::sun::star::uno::RuntimeException )
{
    return AVMEDIA_VLC_PLAYER_IMPLEMENTATIONNAME;
}

::sal_Bool SAL_CALL VLCPlayer::supportsService( const ::rtl::OUString& serviceName )
     throw ( ::com::sun::star::uno::RuntimeException )
{
    return serviceName == AVMEDIA_VLC_PLAYER_SERVICENAME;
}

::uno::Sequence< ::rtl::OUString > SAL_CALL VLCPlayer::getSupportedServiceNames()
     throw ( ::com::sun::star::uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = AVMEDIA_VLC_PLAYER_SERVICENAME;
    return aRet;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
