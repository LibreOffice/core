#include <iostream>
#include "vlcwindow.hxx"
#include "vlcplayer.hxx"

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

namespace
{
    const ::rtl::OUString AVMEDIA_VLC_WINDOW_IMPLEMENTATIONNAME = "com.sun.star.comp.avmedia.Window_VLC";
    const ::rtl::OUString AVMEDIA_VLC_WINDOW_SERVICENAME = "com.sun.star.media.Window_VLC";
}

VLCWindow::VLCWindow( VLCPlayer& player, const intptr_t prevWinID )
    : mPlayer( player )
    , mPrevWinID( prevWinID )
    , meZoomLevel( media::ZoomLevel_ORIGINAL )
{
}

VLCWindow::~VLCWindow()
{
    if ( mPrevWinID != 0 )
        mPlayer.setWindowID( mPrevWinID );
}

void SAL_CALL VLCWindow::update() throw (css::uno::RuntimeException)
{
}

::sal_Bool SAL_CALL VLCWindow::setZoomLevel( css::media::ZoomLevel eZoomLevel ) throw (css::uno::RuntimeException)
{
    sal_Bool bRet = false;

    if( media::ZoomLevel_NOT_AVAILABLE != meZoomLevel &&
        media::ZoomLevel_NOT_AVAILABLE != eZoomLevel )
    {
        if( eZoomLevel != meZoomLevel )
        {
            meZoomLevel = eZoomLevel;
        }

        switch ( static_cast<int>( eZoomLevel ) )
        {
        case media::ZoomLevel_ORIGINAL:
        case media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT:
            mPlayer.setVideoSize( mSize.Width, mSize.Height );
            break;
        case media::ZoomLevel_ZOOM_1_TO_2:
            mPlayer.setVideoSize( mSize.Width / 2, mSize.Height / 2 );
            break;
        case media::ZoomLevel_ZOOM_2_TO_1:
            mPlayer.setVideoSize( mSize.Width * 2, mSize.Height * 2 );
            break;
        }

        bRet = true;
    }

    return bRet;
}

css::media::ZoomLevel SAL_CALL VLCWindow::getZoomLevel() throw (css::uno::RuntimeException)
{
    return meZoomLevel;
}

void SAL_CALL VLCWindow::setPointerType( ::sal_Int32 ) throw (css::uno::RuntimeException)
{
}

::rtl::OUString SAL_CALL VLCWindow::getImplementationName() throw (css::uno::RuntimeException)
{
    return AVMEDIA_VLC_WINDOW_IMPLEMENTATIONNAME;
}

::sal_Bool SAL_CALL VLCWindow::supportsService( const ::rtl::OUString& serviceName ) throw (css::uno::RuntimeException)
{
    return serviceName == AVMEDIA_VLC_WINDOW_SERVICENAME;
}

uno::Sequence< ::rtl::OUString > SAL_CALL VLCWindow::getSupportedServiceNames() throw (css::uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = AVMEDIA_VLC_WINDOW_SERVICENAME;
    return aRet;
}

void SAL_CALL VLCWindow::dispose() throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addEventListener( const uno::Reference< lang::XEventListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeEventListener( const uno::Reference< lang::XEventListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 /* Flags */ )
    throw (uno::RuntimeException)
{
    mSize.X = X;
    mSize.Y = Y;
    mSize.Width = Width;
    mSize.Height = Height;
}

awt::Rectangle SAL_CALL VLCWindow::getPosSize()
    throw (uno::RuntimeException)
{
    return mSize;
}

void SAL_CALL VLCWindow::setVisible( sal_Bool )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::setEnable( sal_Bool )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::setFocus()
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addWindowListener( const uno::Reference< awt::XWindowListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeWindowListener( const uno::Reference< awt::XWindowListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addFocusListener( const uno::Reference< awt::XFocusListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeFocusListener( const uno::Reference< awt::XFocusListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addKeyListener( const uno::Reference< awt::XKeyListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeKeyListener( const uno::Reference< awt::XKeyListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addMouseListener( const uno::Reference< awt::XMouseListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeMouseListener( const uno::Reference< awt::XMouseListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::addPaintListener( const uno::Reference< awt::XPaintListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL VLCWindow::removePaintListener( const uno::Reference< awt::XPaintListener >& )
    throw (uno::RuntimeException)
{
}

}
}
