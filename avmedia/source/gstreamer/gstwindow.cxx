/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <tools/prex.h>
#include <tools/postx.h>

#include <com/sun/star/awt/SystemPointer.hdl>

#include "gstwindow.hxx"
#include "gstplayer.hxx"
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

using namespace ::com::sun::star;

namespace avmedia
{
namespace gst
{
// ---------------------
// - PlayerChildWindow -
// ---------------------

class PlayerChildWindow : public SystemChildWindow
{
public:

                    PlayerChildWindow( Window* pParent );
                    ~PlayerChildWindow();

protected:

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    KeyUp( const KeyEvent& rKEvt );
    virtual void    Command( const CommandEvent& rCEvt );
};

// ---------------------------------------------------------------------

PlayerChildWindow::PlayerChildWindow( Window* pParent ) :
    SystemChildWindow( pParent, WB_CLIPCHILDREN )
{
}

// ---------------------------------------------------------------------

PlayerChildWindow::~PlayerChildWindow()
{
}

// ---------------------------------------------------------------------

void PlayerChildWindow::MouseMove( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                        rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseMove( rMEvt );
    GetParent()->MouseMove( aTransformedEvent );
}

// ---------------------------------------------------------------------

void PlayerChildWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                        rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseButtonDown( rMEvt );
    GetParent()->MouseButtonDown( aTransformedEvent );
}

// ---------------------------------------------------------------------

void PlayerChildWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                        rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseButtonUp( rMEvt );
    GetParent()->MouseButtonUp( aTransformedEvent );
}

// ---------------------------------------------------------------------

void PlayerChildWindow::KeyInput( const KeyEvent& rKEvt )
{
    SystemChildWindow::KeyInput( rKEvt );
    GetParent()->KeyInput( rKEvt );
}

// ---------------------------------------------------------------------

void PlayerChildWindow::KeyUp( const KeyEvent& rKEvt )
{
    SystemChildWindow::KeyUp( rKEvt );
    GetParent()->KeyUp( rKEvt );
}

// ---------------------------------------------------------------------

void PlayerChildWindow::Command( const CommandEvent& rCEvt )
{
    const CommandEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rCEvt.GetMousePosPixel() ) ),
                                          rCEvt.GetCommand(), rCEvt.IsMouseEvent(), rCEvt.GetData() );

    SystemChildWindow::Command( rCEvt );
    GetParent()->Command( aTransformedEvent );
}

// ---------------
// - Window -
// ---------------
Window::Window( ::avmedia::gst::Player& rPlayer ) :
    maListeners( maMutex ),
    maWindowRect( 0, 0, 0, 0 ),
    mrPlayer( rPlayer ),
    mpPlayerWindow( NULL ),
    meZoomLevel( media::ZoomLevel_NOT_AVAILABLE ),
    mnPointerType( awt::SystemPointer::ARROW )
{}

// ------------------------------------------------------------------------------

Window::~Window()
{
    dispose();
}

// ------------------------------------------------------------------------------

void Window::implLayoutVideoWindow()
{
    if( mpPlayerWindow && ( media::ZoomLevel_NOT_AVAILABLE != meZoomLevel ) )
    {
        awt::Size aPrefSize( mrPlayer.getPreferredPlayerWindowSize() );
        awt::Rectangle aRect = getPosSize();
        int nW = aRect.Width, nH = aRect.Height;
        int nVideoW = nW, nVideoH = nH;
        int nX = 0, nY = 0, nWidth = 0, nHeight = 0;
        bool bDone = false, bZoom = false;

        if( media::ZoomLevel_ORIGINAL == meZoomLevel )
        {
            bZoom = true;
        }
        else if( media::ZoomLevel_ZOOM_1_TO_4 == meZoomLevel )
        {
            aPrefSize.Width >>= 2;
            aPrefSize.Height >>= 2;
            bZoom = true;
        }
        else if( media::ZoomLevel_ZOOM_1_TO_2 == meZoomLevel )
        {
            aPrefSize.Width >>= 1;
            aPrefSize.Height >>= 1;
            bZoom = true;
        }
        else if( media::ZoomLevel_ZOOM_2_TO_1 == meZoomLevel )
        {
            aPrefSize.Width <<= 1;
            aPrefSize.Height <<= 1;
            bZoom = true;
        }
        else if( media::ZoomLevel_ZOOM_4_TO_1 == meZoomLevel )
        {
            aPrefSize.Width <<= 2;
            aPrefSize.Height <<= 2;
            bZoom = true;
        }
        else if( media::ZoomLevel_FIT_TO_WINDOW == meZoomLevel )
        {
            nWidth = nVideoW;
            nHeight = nVideoH;
            bDone = true;
        }

        if( bZoom )
        {
            if( ( aPrefSize.Width <= nVideoW ) && ( aPrefSize.Height <= nVideoH ) )
            {
                nX = ( nVideoW - aPrefSize.Width ) >> 1;
                nY = ( nVideoH - aPrefSize.Height ) >> 1;
                nWidth = aPrefSize.Width;
                nHeight = aPrefSize.Height;
                bDone = true;
            }
        }

        if( !bDone )
        {
            if( ( aPrefSize.Width > 0 ) && ( aPrefSize.Height > 0 ) && ( nVideoW > 0 ) && ( nVideoH > 0 ) )
            {
                double fPrefWH = (double) aPrefSize.Width / aPrefSize.Height;

                if( fPrefWH < ( (double) nVideoW / nVideoH ) )
                {
                    nVideoW = (int) ( nVideoH * fPrefWH );
                }
                else
                {
                    nVideoH = (int) ( nVideoW / fPrefWH );
                }

                nX = ( nW - nVideoW ) >> 1;
                nY = ( nH - nVideoH ) >> 1;
                nWidth = nVideoW;
                nHeight = nVideoH;
            }
            else
            {
                nX = nY = nWidth = nHeight = 0;
            }
        }

        mpPlayerWindow->SetPosSizePixel( Point( nX, nY ), Size( nWidth, nHeight ) );
    }
}

// ------------------------------------------------------------------------------

bool Window::create( const uno::Sequence< uno::Any >& rArguments )
{
    OSL_ASSERT( !mpPlayerWindow );
    OSL_ENSURE( rArguments.getLength() > 2,
"::avmedia::gst::Windiow::create: GStreamer backend needs \
valid parent SystemChildWindow pointer (sal_IntPtr) as third Any \
value in sequence of arguments" );

    bool bRet = false;

    if( !mpPlayerWindow && ( rArguments.getLength() > 2 ) )
    {
        SystemChildWindow* pParentWindow = NULL;
        sal_IntPtr pIntPtr = 0;

        // we are not interested in the first argument, which is a pure X11 window handle;
        // this backend relies on a SystemChildWindow as parent for the PlayerWindow, given
        // as third Sal_IntPtr value within the argument Sequence
        rArguments[ 1 ] >>= maWindowRect;

        rArguments[ 2 ] >>= pIntPtr;
        pParentWindow = reinterpret_cast< SystemChildWindow* >( pIntPtr );

        if( pParentWindow )
        {
            meZoomLevel = media::ZoomLevel_ORIGINAL;

            mpPlayerWindow = new PlayerChildWindow( pParentWindow );
            mpPlayerWindow->SetBackground( Color( COL_BLACK ) );
            mpPlayerWindow->SetPosSizePixel( Point( maWindowRect.X, maWindowRect.Y ),
                                            Size( maWindowRect.Width, maWindowRect.Height ) );
            mpPlayerWindow->Show();

            bRet = true;
        }
    }

    return( bRet );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::update()
     throw( uno::RuntimeException )
{
    if( mpPlayerWindow )
    {
        mpPlayerWindow->Invalidate( INVALIDATE_UPDATE );
        mpPlayerWindow->Sync();
    }
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Window::setZoomLevel( media::ZoomLevel eZoomLevel )
     throw(uno::RuntimeException)
{
    bool bRet = false;

    if( ( media::ZoomLevel_NOT_AVAILABLE != meZoomLevel ) &&
       ( media::ZoomLevel_NOT_AVAILABLE != eZoomLevel ) )
    {
        if( eZoomLevel != meZoomLevel )
        {
            meZoomLevel = eZoomLevel;
            implLayoutVideoWindow();
        }

        bRet = true;
    }

    return( bRet );
}

// ------------------------------------------------------------------------------

media::ZoomLevel SAL_CALL Window::getZoomLevel()
     throw( uno::RuntimeException )
{
    return( meZoomLevel );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setPointerType( sal_Int32 nPointerType )
     throw( uno::RuntimeException )
{
    if( mpPlayerWindow )
    {
        Pointer ePointer = POINTER_ARROW;

        switch( nPointerType )
        {
            case (awt::SystemPointer::CROSS) :
            {
                ePointer = POINTER_CROSS;
            }
            break;

            case (awt::SystemPointer::HAND) :
            {
                ePointer = POINTER_HAND;
            }
            break;

            case (awt::SystemPointer::MOVE) :
            {
                ePointer = POINTER_MOVE;
            }
            break;

            case (awt::SystemPointer::WAIT) :
            {
                ePointer = POINTER_WAIT;
            }
            break;

            default:
            break;
        }

        mpPlayerWindow->SetPointer( ePointer );
    }
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setPosSize( sal_Int32 nX,
                                  sal_Int32 nY,
                                  sal_Int32 nWidth,
                                  sal_Int32 nHeight,
                                  sal_Int16 /* Flags */ )
     throw( uno::RuntimeException )
{
    maWindowRect.X = nX;
    maWindowRect.Y = nY;
    maWindowRect.Width = nWidth;
    maWindowRect.Height = nHeight;

    implLayoutVideoWindow();
}

// ------------------------------------------------------------------------------

awt::Rectangle SAL_CALL Window::getPosSize()
     throw( uno::RuntimeException )
{
    return( maWindowRect );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setVisible( sal_Bool bVisible )
     throw( uno::RuntimeException )
{
    if( mpPlayerWindow )
    {
        bVisible ? mpPlayerWindow->Show() : mpPlayerWindow->Hide();
    }
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setEnable( sal_Bool bEnable )
     throw( uno::RuntimeException )
{
    if( mpPlayerWindow )
    {
        mpPlayerWindow->Enable( bEnable );
    }
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setFocus()
     throw( uno::RuntimeException )
{
    if( mpPlayerWindow )
    {
        mpPlayerWindow->GrabFocus();
    }
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addPaintListener( const uno::Reference< awt::XPaintListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removePaintListener( const uno::Reference< awt::XPaintListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::dispose()
     throw( uno::RuntimeException )
{
    delete mpPlayerWindow;
    mpPlayerWindow = NULL;
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
     throw( uno::RuntimeException )
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeEventListener( const uno::Reference< lang::XEventListener >&
                                           xListener )
     throw( uno::RuntimeException )
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void Window::fireMousePressedEvent( const ::com::sun::star::awt::MouseEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer =
        maListeners.getContainer( getCppuType( ( uno::Reference< awt::XMouseListener >* ) 0 ) );

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
        {
            uno::Reference< awt::XMouseListener >( aIter.next(),
                                                   uno::UNO_QUERY )->mousePressed( rEvt );
        }
    }
}

// -----------------------------------------------------------------------------

void Window::fireMouseReleasedEvent( const ::com::sun::star::awt::MouseEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer =
        maListeners.getContainer( getCppuType( ( uno::Reference< awt::XMouseListener >* ) 0 ) );

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
        {
            uno::Reference< awt::XMouseListener >( aIter.next(),
                                                   uno::UNO_QUERY )->mouseReleased( rEvt );
        }
    }
}

// -----------------------------------------------------------------------------

void Window::fireMouseMovedEvent( const ::com::sun::star::awt::MouseEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer =
        maListeners.getContainer( getCppuType( ( uno::Reference< awt::XMouseMotionListener >* )
                                              0 ) );

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
        {
            uno::Reference< awt::XMouseMotionListener >( aIter.next(),
                                                         uno::UNO_QUERY )->mouseMoved( rEvt );
        }
    }
}

// -----------------------------------------------------------------------------

void Window::fireSetFocusEvent( const ::com::sun::star::awt::FocusEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer =
        maListeners.getContainer( getCppuType( ( uno::Reference< awt::XFocusListener >* ) 0 ) );

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
        {
            uno::Reference< awt::XFocusListener >( aIter.next(),
                                                   uno::UNO_QUERY )->focusGained( rEvt );
        }
    }
}

// ------------------------------------------------------------------------------

long Window::getXWindowHandle() const
{
    return( mpPlayerWindow ? mpPlayerWindow->GetSystemData()->aWindow : 0 );
}


// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Window::getImplementationName()
     throw( uno::RuntimeException )
{
    return( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                AVMEDIA_GSTREAMER_WINDOW_IMPLEMENTATIONNAME ) ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Window::supportsService( const ::rtl::OUString& ServiceName )
     throw( uno::RuntimeException )
{
    return( ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                         AVMEDIA_GSTREAMER_WINDOW_SERVICENAME ) ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Window::getSupportedServiceNames()
     throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aRet( 1 );
    aRet[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_GSTREAMER_WINDOW_SERVICENAME ) );

    return( aRet );
}
} // namespace gst
} // namespace avmedia
