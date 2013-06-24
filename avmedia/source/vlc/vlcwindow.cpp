#include "vlcwindow.hxx"

using namespace ;

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

void SAL_CALL VLCWindow::dispose() throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addEventListener( const ::uno::Reference< ::lang::XEventListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeEventListener( const ::uno::Reference< :::lang::XEventListener >& aListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw (::uno::RuntimeException)
{
}

::awt::Rectangle SAL_CALL VLCWindow::getPosSize() throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::setVisible( sal_Bool Visible ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::setEnable( sal_Bool Enable ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::setFocus() throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addWindowListener( const ::uno::Reference< ::awt::XWindowListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeWindowListener( const ::uno::Reference< ::awt::XWindowListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addFocusListener( const ::uno::Reference< ::awt::XFocusListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeFocusListener( const ::uno::Reference< ::awt::XFocusListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addKeyListener( const ::uno::Reference< ::awt::XKeyListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeKeyListener( const ::uno::Reference< ::awt::XKeyListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addMouseListener( const ::uno::Reference< ::awt::XMouseListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeMouseListener( const ::uno::Reference< ::awt::XMouseListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addMouseMotionListener( const ::uno::Reference< ::awt::XMouseMotionListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeMouseMotionListener( const ::uno::Reference< ::awt::XMouseMotionListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addPaintListener( const ::uno::Reference< ::awt::XPaintListener >& xListener ) throw (::uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removePaintListener( const ::uno::Reference< ::awt::XPaintListener >& xListener ) throw (::uno::RuntimeException)
{
}

}
}