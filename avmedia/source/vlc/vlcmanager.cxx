#include "vlcmanager.hxx"
#include "vlcplayer.hxx"

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

const rtl::OUString VLC_IMPLEMENTATION_NAME = "com.sun.star.comp.avmedia.Manager_VLC";

Manager::Manager( const uno::Reference< lang::XMultiServiceFactory >& rxMgr )
    : mxMgr( rxMgr )
{
}

Manager::~Manager()
{
}

uno::Reference< media::XPlayer > SAL_CALL Manager::createPlayer( const rtl::OUString& rURL )
    throw (uno::RuntimeException)
{
    VLCPlayer* pPlayer( new VLCPlayer( /* mxMgr */ ) );
    uno::Reference< media::XPlayer > xRet( pPlayer );

    return xRet;
}

rtl::OUString SAL_CALL Manager::getImplementationName()
    throw (uno::RuntimeException)
{
    return VLC_IMPLEMENTATION_NAME;
}

sal_Bool SAL_CALL Manager::supportsService( const rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return false;
}

uno::Sequence< rtl::OUString > SAL_CALL Manager::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return uno::Sequence< rtl::OUString >();
}

} // end namespace vlc
} // end namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
