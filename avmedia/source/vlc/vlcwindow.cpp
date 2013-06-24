#include "vlcwindow.hxx"

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

void SAL_CALL VLCWindow::pdate()
{
}

::sal_Bool SAL_CALL VLCWindow::setZoomLevel( css::media::ZoomLevel ZoomLevel )
{
    return false;
}

css::media::ZoomLevel SAL_CALL VLCWindow::getZoomLevel()
{
    return css::media::ZoomLevel_NOT_AVAILABLE;
}

void SAL_CALL VLCWindow::setPointerType( ::sal_Int32 SystemPointerType )
{
}

::rtl::OUString SAL_CALL VLCWindow::getImplementationName()
{
    return ::rtl::OUString();
}

::sal_Bool SAL_CALL VLCWindow::supportsService( const ::rtl::OUString& ServiceName )
{
    return false;
}

::uno::Sequence< ::rtl::OUString > SAL_CALL VLCWindow::getSupportedServiceNames()
{
    return ::uno::Sequence< ::rtl::OUString >();
}

}
}