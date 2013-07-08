#include <vcl/graph.hxx>
#include "vlcframegrabber.hxx"

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

const ::rtl::OUString AVMEDIA_VLC_GRABBER_IMPLEMENTATIONNAME = "com.sun.star.comp.avmedia.VLCFrameGrabber_VLC";
const ::rtl::OUString AVMEDIA_VLC_GRABBER_SERVICENAME = "com.sun.star.media.VLCFrameGrabber_VLC";

SAL_CALL VLCFrameGrabber::VLCFrameGrabber()
{
}

::uno::Reference< css::graphic::XGraphic > SAL_CALL VLCFrameGrabber::grabFrame( double fMediaTime )
{
    return Graphic().GetXGraphic();
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