#include <osl/conditn.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include <avmedia/mediawindow.hxx>
#include "vlcframegrabber.hxx"
#include "vlcplayer.hxx"
#include <vlc/libvlc_media_player.h>

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

const ::rtl::OUString AVMEDIA_VLC_GRABBER_IMPLEMENTATIONNAME = "com.sun.star.comp.avmedia.VLCFrameGrabber_VLC";
const ::rtl::OUString AVMEDIA_VLC_GRABBER_SERVICENAME = "com.sun.star.media.VLCFrameGrabber_VLC";
const int MSEC_IN_SEC = 1000;

SAL_CALL VLCFrameGrabber::VLCFrameGrabber( boost::shared_ptr<libvlc_media_player_t>& player, const rtl::OUString& url )
    : FrameGrabber_BASE()
    , mPlayer( player )
    , mUrl( url )
{
}

namespace
{
    struct FrameData
    {
        ::osl::Condition mCondition;

        std::vector<sal_uInt8> buffer;

        libvlc_media_player_t *mpPlayer;

        FrameData( libvlc_media_player_t *pPlayer )
            : mpPlayer( pPlayer )
        {
        }

        void updateSize()
        {
            unsigned int w, h;
            libvlc_video_get_size( mpPlayer, 0, &w, &h );

            buffer.resize(w * h * 3);
        }

        ~FrameData()
        {
        }
    };

    void *FrameLock( void *data, void **pPixels )
    {
        FrameData *frameData = static_cast<FrameData*>( data );

        frameData->updateSize();

        *pPixels = frameData->buffer.data();

        return *pPixels;
    }

    void FrameUnlock( void *data, void */* id */, void *const * /* pPixels */ )
    {
        FrameData *frameData = static_cast<FrameData*>( data );

        frameData->mCondition.set();
    }

    void FrameDisplay( void */* data */, void */* id */ )
    {
    }
}

::uno::Reference< css::graphic::XGraphic > SAL_CALL VLCFrameGrabber::grabFrame( double fMediaTime )
{
    if ( mUrl.isEmpty() )
        return ::uno::Reference< css::graphic::XGraphic >();

    libvlc_media_player_t *pPlayer = mPlayer.get();
    FrameData frameData( pPlayer );
    libvlc_video_set_callbacks( pPlayer, FrameLock, FrameUnlock, FrameDisplay, &frameData );

    const unsigned int w = 480, h = 360;

    libvlc_video_set_format( pPlayer, "RV24", w, h, w * 3 );

    libvlc_media_player_set_time( pPlayer, fMediaTime * MSEC_IN_SEC );
    libvlc_media_player_play( pPlayer );

    const TimeValue t = {2, 0};
    frameData.mCondition.wait( &t );

    if ( !frameData.mCondition.check() )
        return ::uno::Reference< css::graphic::XGraphic >();

    Bitmap aBmp( Size( w, h ), 24 );
    std::cout << 1 << std::endl;
    sal_uInt8 *pData = frameData.buffer.data();
    BitmapWriteAccess *pWrite = aBmp.AcquireWriteAccess();
    if ( pWrite )
    {
        for ( std::size_t y = 0; y < h; ++y )
        {
            for ( std::size_t x = 0; x < w; ++x )
            {
                sal_uInt8 *p = pData + ( y * w + x ) * 3;
                BitmapColor col( p[0], p[1], p[2] );
                pWrite->SetPixel( y, x, col );
            }
        }
    }
    aBmp.ReleaseAccess( pWrite );

    libvlc_media_player_stop( pPlayer );

    return Graphic( aBmp ).GetXGraphic();
}

::rtl::OUString SAL_CALL VLCFrameGrabber::getImplementationName()
{
    return AVMEDIA_VLC_GRABBER_IMPLEMENTATIONNAME;
}

::sal_Bool SAL_CALL VLCFrameGrabber::supportsService( const ::rtl::OUString& serviceName )
{
    return AVMEDIA_VLC_GRABBER_SERVICENAME == serviceName;
}

::uno::Sequence< ::rtl::OUString > SAL_CALL VLCFrameGrabber::getSupportedServiceNames()
{
    ::uno::Sequence< OUString > aRet(1);
    aRet[0] = AVMEDIA_VLC_GRABBER_SERVICENAME;
    return aRet;
}

}
}
