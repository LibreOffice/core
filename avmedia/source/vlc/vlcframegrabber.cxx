#include <osl/conditn.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include <avmedia/mediawindow.hxx>
#include "vlcframegrabber.hxx"
#include "vlcplayer.hxx"
#include <vlc/libvlc_media_player.h>
#include <boost/bind.hpp>

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

::uno::Reference< css::graphic::XGraphic > SAL_CALL VLCFrameGrabber::grabFrame( double fMediaTime )
{
    if ( mUrl.isEmpty() )
        return ::uno::Reference< css::graphic::XGraphic >();

    // libvlc_video_take_snapshot must be used, but it doesn't work for PNG files
    //

    return ::uno::Reference< css::graphic::XGraphic >();
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
