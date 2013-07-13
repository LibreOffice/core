#include "vlcwindow.hxx"

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

const ::rtl::OUString AVMEDIA_VLC_WINDOW_IMPLEMENTATIONNAME = "com.sun.star.comp.avmedia.Window_VLC";
const ::rtl::OUString AVMEDIA_VLC_WINDOW_SERVICENAME = "com.sun.star.media.Window_VLC";

SAL_CALL VLCWindow::VLCWindow(VLCPlayer& player)
    : mPlayer( player )
{
}

void SAL_CALL VLCWindow::update()
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
    return AVMEDIA_VLC_WINDOW_IMPLEMENTATIONNAME;
}

::sal_Bool SAL_CALL VLCWindow::supportsService( const ::rtl::OUString& serviceName )
{
    return serviceName == AVMEDIA_VLC_WINDOW_SERVICENAME;
}

uno::Sequence< ::rtl::OUString > SAL_CALL VLCWindow::getSupportedServiceNames()
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = AVMEDIA_VLC_WINDOW_SERVICENAME;
    return aRet;
}

void SAL_CALL VLCWindow::dispose() throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags )
    throw (uno::RuntimeException)
{
}

awt::Rectangle SAL_CALL VLCWindow::getPosSize()
    throw (uno::RuntimeException)
{
    return awt::Rectangle();
}

void SAL_CALL VLCWindow::setVisible( sal_Bool Visible )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::setEnable( sal_Bool Enable )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::setFocus()
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addPaintListener( const uno::Reference< awt::XPaintListener >& xListener )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removePaintListener( const uno::Reference< awt::XPaintListener >& xListener )
    throw (uno::RuntimeException)
{
}

}
}
