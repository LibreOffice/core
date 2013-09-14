#include <boost/bind.hpp>
#include <iostream>
#include <osl/conditn.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/pngread.hxx>
#include <avmedia/mediawindow.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/stream.hxx>

#include "vlcframegrabber.hxx"
#include "vlcplayer.hxx"
#include "wrapper/Player.hxx"
#include "wrapper/EventManager.hxx"

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

namespace
{
    const ::rtl::OUString AVMEDIA_VLC_GRABBER_IMPLEMENTATIONNAME = "com.sun.star.comp.avmedia.VLCFrameGrabber_VLC";
    const ::rtl::OUString AVMEDIA_VLC_GRABBER_SERVICENAME = "com.sun.star.media.VLCFrameGrabber_VLC";
    const int MSEC_IN_SEC = 1000;

    const char * const VLC_ARGS[] = {
        "-Vdummy",
        "--snapshot-format=png",
        "--ffmpeg-threads",
        "--verbose=-1",
        "--no-audio"
    };
}

VLCFrameGrabber::VLCFrameGrabber( wrapper::EventHandler& eh, const rtl::OUString& url )
    : FrameGrabber_BASE()
    , mInstance( sizeof( VLC_ARGS ) / sizeof( VLC_ARGS[0] ), VLC_ARGS )
    , mMedia( url, mInstance )
    , mPlayer( mMedia )
    , mEventHandler( eh )
{
}

::uno::Reference< css::graphic::XGraphic > SAL_CALL VLCFrameGrabber::grabFrame( double fMediaTime )
        throw ( ::com::sun::star::uno::RuntimeException )
{
    osl::Condition condition;

    const rtl::OUString& fileName = utl::TempFile::CreateTempName();
    {
        wrapper::EventManager manager( mPlayer, mEventHandler );
        manager.onPaused(boost::bind(&osl::Condition::set, &condition));

        if ( !mPlayer.play() )
        {
            SAL_WARN("avmedia", "Couldn't play when trying to grab frame");
            return ::uno::Reference< css::graphic::XGraphic >();
        }

        mPlayer.setTime( ( fMediaTime > 0 ? fMediaTime : 0 ) * MSEC_IN_SEC );
        mPlayer.pause();

        const TimeValue timeout = {2, 0};
        condition.wait(&timeout);

        if ( !mPlayer.hasVout() )
        {
            SAL_WARN("avmedia", "Couldn't grab frame");
            manager.onPaused();
            return ::uno::Reference< css::graphic::XGraphic >();
        }

        mPlayer.takeSnapshot( fileName );
        mPlayer.stop();

        manager.onPaused();
    }

    rtl::OUString url;
    utl::LocalFileHelper::ConvertPhysicalNameToURL( fileName, url );
    boost::shared_ptr<SvStream> stream( utl::UcbStreamHelper::CreateStream( url,
                                                                            STREAM_STD_READ ) );

    vcl::PNGReader reader( *stream );

    const BitmapEx& bitmap = reader.Read();

    return Graphic( bitmap ).GetXGraphic();
}

::rtl::OUString SAL_CALL VLCFrameGrabber::getImplementationName() throw ( ::com::sun::star::uno::RuntimeException )
{
    return AVMEDIA_VLC_GRABBER_IMPLEMENTATIONNAME;
}

::sal_Bool SAL_CALL VLCFrameGrabber::supportsService( const ::rtl::OUString& serviceName )
        throw ( ::com::sun::star::uno::RuntimeException )
{
    return AVMEDIA_VLC_GRABBER_SERVICENAME == serviceName;
}

::uno::Sequence< ::rtl::OUString > SAL_CALL VLCFrameGrabber::getSupportedServiceNames()
        throw ( ::com::sun::star::uno::RuntimeException )
{
    ::uno::Sequence< OUString > aRet(1);
    aRet[0] = AVMEDIA_VLC_GRABBER_SERVICENAME;
    return aRet;
}

}
}
