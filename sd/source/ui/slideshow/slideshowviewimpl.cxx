/*************************************************************************
 *
 *  $RCSfile: slideshowviewimpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:20:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SD_SLIDESHOWVIEWIMPL_HXX_
#include <slideshowviewimpl.hxx>
#endif

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <cppcanvas/basegfxfactory.hxx>


using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::presentation::XSlideShow;
using ::com::sun::star::presentation::XSlideShowView;
using ::com::sun::star::presentation::XShapeEventListener;
using ::com::sun::star::presentation::XSlideShowListener;
using ::comphelper::ImplementationReference;

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace sd
{

///////////////////////////////////////////////////////////////////////
// SlideShowViewListeners
///////////////////////////////////////////////////////////////////////

SlideShowViewListeners::SlideShowViewListeners( ::osl::Mutex& rMutex )
:   SlideShowViewListeners_Base( rMutex )
{
}

bool SlideShowViewListeners::implNotify( const Reference< util::XModifyListener >&  rListener,
                                         const awt::WindowEvent&                    rEvent ) throw( uno::Exception )
{
    rListener->modified( rEvent );
    return true; // continue calling listeners
}

///////////////////////////////////////////////////////////////////////
// SlideShowViewPaintListeners
///////////////////////////////////////////////////////////////////////

SlideShowViewPaintListeners::SlideShowViewPaintListeners( ::osl::Mutex& rMutex )
:   SlideShowViewPaintListeners_Base( rMutex )
{
}

bool SlideShowViewPaintListeners::implNotify( const Reference< awt::XPaintListener >& rListener,
                                              const awt::PaintEvent&                  rEvent ) throw( uno::Exception )
{
    rListener->windowPaint( rEvent );
    return true; // continue calling listeners
}

///////////////////////////////////////////////////////////////////////
// SlideShowViewMouseListeners
///////////////////////////////////////////////////////////////////////

SlideShowViewMouseListeners::SlideShowViewMouseListeners( ::osl::Mutex& rMutex ) :
    SlideShowViewMouseListeners_Base( rMutex )
{
}

bool SlideShowViewMouseListeners::implNotify( const Reference< awt::XMouseListener >&   rListener,
                                              const WrappedMouseEvent&                  rEvent ) throw( uno::Exception )
{
    switch( rEvent.meType )
    {
        case WrappedMouseEvent::PRESSED:
            rListener->mousePressed( rEvent.maEvent );
            break;

        case WrappedMouseEvent::RELEASED:
            rListener->mouseReleased( rEvent.maEvent );
            break;

        case WrappedMouseEvent::ENTERED:
            rListener->mouseEntered( rEvent.maEvent );
            break;

        case WrappedMouseEvent::EXITED:
            rListener->mouseExited( rEvent.maEvent );
            break;
    }

    return true; // continue calling listeners
}

///////////////////////////////////////////////////////////////////////
// SlideShowViewMouseMotionListeners
///////////////////////////////////////////////////////////////////////

SlideShowViewMouseMotionListeners::SlideShowViewMouseMotionListeners( ::osl::Mutex& rMutex ) :
    SlideShowViewMouseMotionListeners_Base( rMutex )
{
}

bool SlideShowViewMouseMotionListeners::implNotify( const Reference< awt::XMouseMotionListener >&   rListener,
                                                    const WrappedMouseMotionEvent&                  rEvent ) throw( uno::Exception )
{
    switch( rEvent.meType )
    {
        case WrappedMouseMotionEvent::DRAGGED:
            rListener->mouseDragged( rEvent.maEvent );
            break;

        case WrappedMouseMotionEvent::MOVED:
            rListener->mouseMoved( rEvent.maEvent );
            break;
    }

    return true; // continue calling listeners
}

///////////////////////////////////////////////////////////////////////
// SlideShowView
///////////////////////////////////////////////////////////////////////

SlideShowView::SlideShowView( ShowWindow& rOutputWindow, SdDrawDocument* pDoc, AnimationMode eAnimationMode )
:   SlideShowView_Base( m_aMutex ),
    mpCanvas( ::cppcanvas::VCLFactory::getInstance().createSpriteCanvas( rOutputWindow ) ),
    mxWindow( VCLUnoHelper::GetInterface( &rOutputWindow ), uno::UNO_QUERY_THROW ),
    mxWindowPeer( mxWindow, uno::UNO_QUERY_THROW ),
    mpDoc( pDoc ),
    mxPointer(),
    mrOutputWindow( rOutputWindow ),
    mpViewListeners( new SlideShowViewListeners( m_aMutex ) ),
    mpPaintListeners( new SlideShowViewPaintListeners( m_aMutex ) ),
    mpMouseListeners( new SlideShowViewMouseListeners( m_aMutex ) ),
    mpMouseMotionListeners( new SlideShowViewMouseMotionListeners( m_aMutex ) ),
    mbIsMouseMotionListener( false ),
    meAnimationMode( eAnimationMode )
{
    init();
}

/// Dispose all internal references
void SAL_CALL SlideShowView::dispose() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // deregister listeners
    if( mxWindow.is() )
    {
        mxWindow->removeWindowListener( this );
        mxWindow->removeMouseListener( this );

        if( mbIsMouseMotionListener )
            mxWindow->removeMouseMotionListener( this );
    }

    mpCanvas.reset();
    mxWindow.clear();

    // clear all listener containers
    disposing( lang::EventObject() );

    // call base
    WeakComponentImplHelperBase::dispose();
}

/// Disposing our broadcaster
void SAL_CALL SlideShowView::disposing( const lang::EventObject& ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // delete listener containers
    mpViewListeners.reset();
    mpPaintListeners.reset();
    mpMouseListeners.reset();
    mpMouseMotionListeners.reset();
}

void SAL_CALL SlideShowView::paint( const awt::PaintEvent& e ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    awt::PaintEvent aEvent( e );
    aEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    mpPaintListeners->notify( aEvent );
}

// XSlideShowView methods
Reference< rendering::XSpriteCanvas > SAL_CALL SlideShowView::getCanvas(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return mpCanvas.get() ? mpCanvas->getUNOSpriteCanvas() : Reference< rendering::XSpriteCanvas >();
}

void SAL_CALL SlideShowView::clear() throw (::com::sun::star::uno::RuntimeException)
{
    // paint background in black
    ::osl::MutexGuard aGuard( m_aMutex );

    // fill the bounds rectangle in black
    // ----------------------------------

    const Size aWindowSize( mrOutputWindow.GetSizePixel() );

    ::basegfx::B2DPolygon aPoly( ::basegfx::tools::createPolygonFromRect(
                                     ::basegfx::B2DRectangle(0.0,0.0,
                                                             aWindowSize.Width(),
                                                             aWindowSize.Height() ) ) );
    ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
        ::cppcanvas::BaseGfxFactory::getInstance().createPolyPolygon( mpCanvas, aPoly ) );

    if( pPolyPoly.get() )
    {
        pPolyPoly->setRGBAFillColor( 0x000000FFU );
        pPolyPoly->draw();
    }
}

geometry::AffineMatrix2D SAL_CALL SlideShowView::getTransformation(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    const Size& rWindowSize = mrOutputWindow.GetSizePixel();
    Size aOutputSize( rWindowSize );

    if( meAnimationMode != ANIMATIONMODE_SHOW )
    {
        aOutputSize.Width() = (long)( aOutputSize.Width() / 1.03 );
        aOutputSize.Height() = (long)( aOutputSize.Height() / 1.03 );
    }

    SdPage* pP = mpDoc->GetSdPage( 0, PK_STANDARD );
    Size aPageSize( pP->GetSize() );

    const double page_ratio = (double)aPageSize.Width() / (double)aPageSize.Height();
    const double output_ratio = (double)aOutputSize.Width() / (double)aOutputSize.Height();

    if( page_ratio > output_ratio )
    {
        aOutputSize.Height() = ( aOutputSize.Width() * aPageSize.Height() ) / aPageSize.Width();
    }
    else if( page_ratio < output_ratio )
    {
        aOutputSize.Width() = ( aOutputSize.Height() * aPageSize.Width() ) / aPageSize.Height();
    }

    Point aOutputOffset( ( rWindowSize.Width() - aOutputSize.Width() ) >> 1,
                            ( rWindowSize.Height() - aOutputSize.Height() ) >> 1 );

    ::basegfx::B2DHomMatrix aMatrix;

    maPresentationArea = Rectangle( aOutputOffset, aOutputSize );
    mrOutputWindow.SetPresentationArea( maPresentationArea );

    // scale presentation into available window rect (minus 10%)
    aMatrix.scale( aOutputSize.Width(), aOutputSize.Height() );

    // center in the window
    aMatrix.translate( aOutputOffset.X(), aOutputOffset.Y() );

    geometry::AffineMatrix2D aRes;

    return ::basegfx::unotools::affineMatrixFromHomMatrix( aRes, aMatrix );
}

void SAL_CALL SlideShowView::addTransformationChangedListener( const Reference< util::XModifyListener >& xListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    mpViewListeners->addListener( xListener );
}

void SAL_CALL SlideShowView::removeTransformationChangedListener( const Reference< util::XModifyListener >& xListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    mpViewListeners->removeListener( xListener );
}

void SAL_CALL SlideShowView::addPaintListener( const Reference< awt::XPaintListener >& xListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    mpPaintListeners->addListener( xListener );
}

void SAL_CALL SlideShowView::removePaintListener( const Reference< awt::XPaintListener >& xListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    mpPaintListeners->removeListener( xListener );
}

void SAL_CALL SlideShowView::addMouseListener( const Reference< awt::XMouseListener >& xListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    mpMouseListeners->addListener( xListener );
}

void SAL_CALL SlideShowView::removeMouseListener( const Reference< awt::XMouseListener >& xListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    mpMouseListeners->removeListener( xListener );
}

void SAL_CALL SlideShowView::addMouseMotionListener( const Reference< awt::XMouseMotionListener >& xListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( !mbIsMouseMotionListener )
    {
        // delay motion event registration, until we really
        // need it
        mbIsMouseMotionListener = true;
        mxWindow->addMouseMotionListener( this );
    }

    mpMouseMotionListeners->addListener( xListener );
}

void SAL_CALL SlideShowView::removeMouseMotionListener( const Reference< awt::XMouseMotionListener >& xListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    mpMouseMotionListeners->removeListener( xListener );

    // TODO(P1): Might be nice to deregister for mouse motion
    // events, when the last listener is gone.
}

void SAL_CALL SlideShowView::setMouseCursor( sal_Int16 nPointerShape ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // forward to window
    mxPointer->setType( nPointerShape );
    mxWindowPeer->setPointer( mxPointer );
}


// XWindowListener methods
void SAL_CALL SlideShowView::windowResized( const awt::WindowEvent& e ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    awt::WindowEvent aEvent( e );
    aEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    mpViewListeners->notify( aEvent );
}

void SAL_CALL SlideShowView::windowMoved( const awt::WindowEvent& e ) throw (RuntimeException)
{
    // ignored
}

void SAL_CALL SlideShowView::windowShown( const lang::EventObject& e ) throw (RuntimeException)
{
    // ignored
}

void SAL_CALL SlideShowView::windowHidden( const lang::EventObject& e ) throw (RuntimeException)
{
    // ignored
}

// XMouseListener implementation
void SAL_CALL SlideShowView::mousePressed( const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    WrappedMouseEvent aEvent;
    aEvent.meType = WrappedMouseEvent::PRESSED;
    aEvent.maEvent = e;
    aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    mpMouseListeners->notify( aEvent );
}

void SAL_CALL SlideShowView::mouseReleased( const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    WrappedMouseEvent aEvent;
    aEvent.meType = WrappedMouseEvent::RELEASED;
    aEvent.maEvent = e;
    aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    mpMouseListeners->notify( aEvent );
}

void SAL_CALL SlideShowView::mouseEntered( const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    WrappedMouseEvent aEvent;
    aEvent.meType = WrappedMouseEvent::ENTERED;
    aEvent.maEvent = e;
    aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    mpMouseListeners->notify( aEvent );
}

void SAL_CALL SlideShowView::mouseExited( const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    WrappedMouseEvent aEvent;
    aEvent.meType = WrappedMouseEvent::EXITED;
    aEvent.maEvent = e;
    aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    mpMouseListeners->notify( aEvent );
}

// XMouseMotionListener implementation
void SAL_CALL SlideShowView::mouseDragged( const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    WrappedMouseMotionEvent aEvent;
    aEvent.meType = WrappedMouseMotionEvent::DRAGGED;
    aEvent.maEvent = e;
    aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    mpMouseMotionListeners->notify( aEvent );
}

void SAL_CALL SlideShowView::mouseMoved( const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    WrappedMouseMotionEvent aEvent;
    aEvent.meType = WrappedMouseMotionEvent::MOVED;
    aEvent.maEvent = e;
    aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    mpMouseMotionListeners->notify( aEvent );
}

void SlideShowView::init()
{
    mxWindow->addWindowListener( this );
    mxWindow->addMouseListener( this );

    Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory(),
                                                            uno::UNO_QUERY_THROW );

    mxPointer.set( xFactory->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.Pointer")) ),
                    uno::UNO_QUERY );

    getTransformation();
}

} // namespace ::sd
