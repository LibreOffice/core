/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "slideshowviewimpl.hxx"
#include "slideshowimpl.hxx"
#include <sdpage.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/awt/Pointer.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <cppcanvas/vclfactory.hxx>
#include <cppcanvas/basegfxfactory.hxx>
#include <basegfx/utils/canvastools.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::WeakReference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::presentation::XSlideShowView;

using namespace ::com::sun::star;

namespace sd
{

// SlideShowViewListeners
SlideShowViewListeners::SlideShowViewListeners( ::osl::Mutex& rMutex )
:   mrMutex( rMutex )
{
}

void SlideShowViewListeners::addListener( const Reference< util::XModifyListener >& _rxListener )
{
    ::osl::MutexGuard aGuard( mrMutex );

    WeakReference< util::XModifyListener > xWeak( _rxListener );
    if( std::find( maListeners.begin(), maListeners.end(), xWeak ) == maListeners.end() )
        maListeners.push_back( xWeak );
}

void SlideShowViewListeners::removeListener( const Reference< util::XModifyListener >& _rxListener )
{
    ::osl::MutexGuard aGuard( mrMutex );

    WeakReference< util::XModifyListener > xWeak( _rxListener );
    ViewListenerVector::iterator aIter( std::find( maListeners.begin(), maListeners.end(), xWeak ) );
    if( aIter != maListeners.end() )
        maListeners.erase( aIter );
}

void SlideShowViewListeners::notify( const lang::EventObject& _rEvent )
{
    ::osl::MutexGuard aGuard( mrMutex );

    ViewListenerVector::iterator aIter( maListeners.begin() );
    while( aIter != maListeners.end() )
    {
        Reference< util::XModifyListener > xListener( *aIter );
        if( xListener.is() )
        {
            xListener->modified( _rEvent );
            ++aIter;
        }
        else
        {
            aIter = maListeners.erase( aIter );
        }
    }
}

void SlideShowViewListeners::disposing( const lang::EventObject& _rEventSource )
{
    ::osl::MutexGuard aGuard( mrMutex );

    for( const auto& rxListener : maListeners )
    {
        Reference< util::XModifyListener > xListener( rxListener );
        if( xListener.is() )
            xListener->disposing( _rEventSource );
    }

    maListeners.clear();
}

// SlideShowViewPaintListeners
SlideShowViewPaintListeners::SlideShowViewPaintListeners( ::osl::Mutex& rMutex )
:   SlideShowViewPaintListeners_Base( rMutex )
{
}

bool SlideShowViewPaintListeners::implTypedNotify( const Reference< awt::XPaintListener >& rListener,
                                              const awt::PaintEvent&                  rEvent )
{
    rListener->windowPaint( rEvent );
    return true; // continue calling listeners
}

// SlideShowViewMouseListeners
SlideShowViewMouseListeners::SlideShowViewMouseListeners( ::osl::Mutex& rMutex ) :
    SlideShowViewMouseListeners_Base( rMutex )
{
}

bool SlideShowViewMouseListeners::implTypedNotify( const Reference< awt::XMouseListener >&  rListener,
                                              const WrappedMouseEvent&                  rEvent )
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

// SlideShowViewMouseMotionListeners
SlideShowViewMouseMotionListeners::SlideShowViewMouseMotionListeners( ::osl::Mutex& rMutex ) :
    SlideShowViewMouseMotionListeners_Base( rMutex )
{
}

bool SlideShowViewMouseMotionListeners::implTypedNotify( const Reference< awt::XMouseMotionListener >&  rListener,
                                                    const WrappedMouseMotionEvent&                  rEvent )
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

// SlideShowView
SlideShowView::SlideShowView( ShowWindow&     rOutputWindow,
                              SdDrawDocument* pDoc,
                              AnimationMode   eAnimationMode,
                              SlideshowImpl*  pSlideShow,
                              bool            bFullScreen )
:   SlideShowView_Base( m_aMutex ),
    mpCanvas( ::cppcanvas::VCLFactory::createSpriteCanvas( rOutputWindow ) ),
    mxWindow( VCLUnoHelper::GetInterface( &rOutputWindow ), uno::UNO_QUERY_THROW ),
    mxWindowPeer( mxWindow, uno::UNO_QUERY_THROW ),
    mxPointer(),
    mpSlideShow( pSlideShow ),
    mrOutputWindow( rOutputWindow ),
    mpViewListeners( new SlideShowViewListeners( m_aMutex ) ),
    mpPaintListeners( new SlideShowViewPaintListeners( m_aMutex ) ),
    mpMouseListeners( new SlideShowViewMouseListeners( m_aMutex ) ),
    mpMouseMotionListeners( new SlideShowViewMouseMotionListeners( m_aMutex ) ),
    mpDoc( pDoc ),
    mbIsMouseMotionListener( false ),
    meAnimationMode( eAnimationMode ),
    mbFirstPaint( true ),
    mbFullScreen( bFullScreen ),
    mbMousePressedEaten( false )
{
    init();

    mTranslationOffset.Width = 0;
    mTranslationOffset.Height = 0;
}

// Dispose all internal references
void SAL_CALL SlideShowView::dispose()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    mpSlideShow = nullptr;

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

// Disposing our broadcaster
void SAL_CALL SlideShowView::disposing( const lang::EventObject& )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // notify all listeners that _we_ are going down (send a disposing()),
    // then delete listener containers:
    lang::EventObject const evt( static_cast<OWeakObject *>(this) );
    if (mpViewListeners != nullptr)
    {
        mpViewListeners->disposing( evt );
        mpViewListeners.reset();
    }
    if (mpPaintListeners != nullptr)
    {
        mpPaintListeners->disposing( evt );
        mpPaintListeners.reset();
    }
    if (mpMouseListeners != nullptr)
    {
        mpMouseListeners->disposing( evt );
        mpMouseListeners.reset();
    }
    if (mpMouseMotionListeners != nullptr)
    {
        mpMouseMotionListeners->disposing( evt );
        mpMouseMotionListeners.reset();
    }
}

void SlideShowView::paint( const awt::PaintEvent& e )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    if( mbFirstPaint )
    {
        mbFirstPaint = false;
        SlideshowImpl* pSlideShow = mpSlideShow;
        aGuard.clear();
        if( pSlideShow )
            pSlideShow->onFirstPaint();
    }
    else
    {
        // Change event source, to enable listeners to match event
        // with view
        awt::PaintEvent aEvent( e );
        aEvent.Source = static_cast< ::cppu::OWeakObject* >( this );
        mpPaintListeners->notify( aEvent );
        updateimpl( aGuard, mpSlideShow ); // warning: clears guard!
    }
}

// XSlideShowView methods
Reference< rendering::XSpriteCanvas > SAL_CALL SlideShowView::getCanvas(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return mpCanvas.get() ? mpCanvas->getUNOSpriteCanvas() : Reference< rendering::XSpriteCanvas >();
}

void SAL_CALL SlideShowView::clear()
{
    // paint background in black
    ::osl::MutexGuard aGuard( m_aMutex );
    SolarMutexGuard aSolarGuard;

    // fill the bounds rectangle in black

    const Size aWindowSize( mrOutputWindow.GetSizePixel() );

    ::basegfx::B2DPolygon aPoly( ::basegfx::utils::createPolygonFromRect(
                                     ::basegfx::B2DRectangle(0.0,0.0,
                                                             aWindowSize.Width(),
                                                             aWindowSize.Height() ) ) );
    ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
        ::cppcanvas::BaseGfxFactory::createPolyPolygon( mpCanvas, aPoly ) );

    if( pPolyPoly.get() )
    {
        pPolyPoly->setRGBAFillColor( 0x000000FFU );
        pPolyPoly->draw();
    }
}

geometry::IntegerSize2D SAL_CALL SlideShowView::getTranslationOffset( )
{
        return mTranslationOffset;
}

geometry::AffineMatrix2D SAL_CALL SlideShowView::getTransformation(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    SolarMutexGuard aSolarGuard;

    const Size& rTmpSize( mrOutputWindow.GetSizePixel() );

    if (rTmpSize.Width()<=0 || rTmpSize.Height()<=0)
    {
        return geometry::AffineMatrix2D (1,0,0,0,1,0);
    }

    const Size aWindowSize( mrOutputWindow.GetSizePixel() );
    Size aOutputSize( aWindowSize );

    if( meAnimationMode != ANIMATIONMODE_SHOW )
    {
        aOutputSize.setWidth( static_cast<long>( aOutputSize.Width() / 1.03 ) );
        aOutputSize.setHeight( static_cast<long>( aOutputSize.Height() / 1.03 ) );
    }

    SdPage* pP = mpDoc->GetSdPage( 0, PageKind::Standard );
    Size aPageSize( pP->GetSize() );

    const double page_ratio = static_cast<double>(aPageSize.Width()) / static_cast<double>(aPageSize.Height());
    const double output_ratio = static_cast<double>(aOutputSize.Width()) / static_cast<double>(aOutputSize.Height());

    if( page_ratio > output_ratio )
    {
        aOutputSize.setHeight( ( aOutputSize.Width() * aPageSize.Height() ) / aPageSize.Width() );
    }
    else if( page_ratio < output_ratio )
    {
        aOutputSize.setWidth( ( aOutputSize.Height() * aPageSize.Width() ) / aPageSize.Height() );
    }

    Point aOutputOffset( ( aWindowSize.Width() - aOutputSize.Width() ) >> 1,
                            ( aWindowSize.Height() - aOutputSize.Height() ) >> 1 );

    // Reduce available width by one, as the slides might actually
    // render one pixel wider and higher as aPageSize below specifies
    // (when shapes of page size have visible border lines)
    aOutputSize.AdjustWidth( -1 );
    aOutputSize.AdjustHeight( -1 );

    // Record mTranslationOffset
    mTranslationOffset.Height = aOutputOffset.Y();
    mTranslationOffset.Width = aOutputOffset.X();

    // scale presentation into available window rect (minus 10%); center in the window
    const basegfx::B2DHomMatrix aMatrix(basegfx::utils::createScaleTranslateB2DHomMatrix(
        aOutputSize.Width(), aOutputSize.Height(), aOutputOffset.X(), aOutputOffset.Y()));

    geometry::AffineMatrix2D aRes;

    return ::basegfx::unotools::affineMatrixFromHomMatrix( aRes, aMatrix );
}

void SAL_CALL SlideShowView::addTransformationChangedListener( const Reference< util::XModifyListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (mpViewListeners)
        mpViewListeners->addListener( xListener );
}

void SAL_CALL SlideShowView::removeTransformationChangedListener( const Reference< util::XModifyListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (mpViewListeners)
        mpViewListeners->removeListener( xListener );
}

void SAL_CALL SlideShowView::addPaintListener( const Reference< awt::XPaintListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (mpPaintListeners)
        mpPaintListeners->addTypedListener( xListener );
}

void SAL_CALL SlideShowView::removePaintListener( const Reference< awt::XPaintListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (mpPaintListeners)
        mpPaintListeners->removeTypedListener( xListener );
}

void SAL_CALL SlideShowView::addMouseListener( const Reference< awt::XMouseListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (mpMouseListeners)
        mpMouseListeners->addTypedListener( xListener );
}

void SAL_CALL SlideShowView::removeMouseListener( const Reference< awt::XMouseListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (mpMouseListeners)
        mpMouseListeners->removeTypedListener( xListener );
}

void SAL_CALL SlideShowView::addMouseMotionListener( const Reference< awt::XMouseMotionListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( !mbIsMouseMotionListener && mxWindow.is() )
    {
        // delay motion event registration, until we really
        // need it
        mbIsMouseMotionListener = true;
        mxWindow->addMouseMotionListener( this );
    }

    if (mpMouseMotionListeners)
        mpMouseMotionListeners->addTypedListener( xListener );
}

void SAL_CALL SlideShowView::removeMouseMotionListener( const Reference< awt::XMouseMotionListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (mpMouseMotionListeners)
        mpMouseMotionListeners->removeTypedListener( xListener );

    // TODO(P1): Might be nice to deregister for mouse motion
    // events, when the last listener is gone.
}

void SAL_CALL SlideShowView::setMouseCursor( sal_Int16 nPointerShape )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // forward to window
    if( mxPointer.is() )
        mxPointer->setType( nPointerShape );

    if( mxWindowPeer.is() )
        mxWindowPeer->setPointer( mxPointer );
}

awt::Rectangle SAL_CALL SlideShowView::getCanvasArea(  )
{
    awt::Rectangle aRectangle;

    if( mxWindow.is() )
        return mxWindow->getPosSize();

    aRectangle.X = aRectangle.Y = aRectangle.Width = aRectangle.Height = 0;

    return aRectangle;
}

void SlideShowView::updateimpl( ::osl::ClearableMutexGuard& rGuard, SlideshowImpl* pSlideShow )
{
    if( !pSlideShow )
        return;

    ::rtl::Reference< SlideshowImpl > aSLGuard( pSlideShow );

    if( mbFirstPaint )
    {
        mbFirstPaint = false;
        SlideshowImpl* pTmpSlideShow = mpSlideShow;
        rGuard.clear();
        if( pTmpSlideShow )
            pTmpSlideShow->onFirstPaint();
    } else
        rGuard.clear();

    pSlideShow->startUpdateTimer();
}

// XWindowListener methods
void SAL_CALL SlideShowView::windowResized( const awt::WindowEvent& e )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    if (mpViewListeners)
    {
        // Change event source, to enable listeners to match event
        // with view
        awt::WindowEvent aEvent( e );
        aEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

        mpViewListeners->notify( aEvent );
        updateimpl( aGuard, mpSlideShow ); // warning: clears guard!
    }
}

void SAL_CALL SlideShowView::windowMoved( const awt::WindowEvent& )
{
    // ignored
}

void SAL_CALL SlideShowView::windowShown( const lang::EventObject& )
{
    // ignored
}

void SAL_CALL SlideShowView::windowHidden( const lang::EventObject& )
{
    // ignored
}

// XMouseListener implementation
void SAL_CALL SlideShowView::mousePressed( const awt::MouseEvent& e )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    if( mpSlideShow && mpSlideShow->isInputFreezed() )
    {
        mbMousePressedEaten = true;
    }
    else
    {
        mbMousePressedEaten = false;

        // Change event source, to enable listeners to match event
        // with view
        WrappedMouseEvent aEvent;
        aEvent.meType = WrappedMouseEvent::PRESSED;
        aEvent.maEvent = e;
        aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

        if (mpMouseListeners)
            mpMouseListeners->notify( aEvent );
        updateimpl( aGuard, mpSlideShow ); // warning: clears guard!
    }
}

void SAL_CALL SlideShowView::mouseReleased( const awt::MouseEvent& e )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    if( mbMousePressedEaten )
    {
        // if mouse button down was ignored, also ignore mouse button up
        mbMousePressedEaten = false;
    }
    else if( mpSlideShow && !mpSlideShow->isInputFreezed() )
    {
        // Change event source, to enable listeners to match event
        // with view
        WrappedMouseEvent aEvent;
        aEvent.meType = WrappedMouseEvent::RELEASED;
        aEvent.maEvent = e;
        aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

        if (mpMouseListeners)
            mpMouseListeners->notify( aEvent );
        updateimpl( aGuard, mpSlideShow ); // warning: clears guard!
    }
}

void SAL_CALL SlideShowView::mouseEntered( const awt::MouseEvent& e )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    WrappedMouseEvent aEvent;
    aEvent.meType = WrappedMouseEvent::ENTERED;
    aEvent.maEvent = e;
    aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    if (mpMouseListeners)
        mpMouseListeners->notify( aEvent );
    updateimpl( aGuard, mpSlideShow ); // warning: clears guard!
}

void SAL_CALL SlideShowView::mouseExited( const awt::MouseEvent& e )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    WrappedMouseEvent aEvent;
    aEvent.meType = WrappedMouseEvent::EXITED;
    aEvent.maEvent = e;
    aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    if (mpMouseListeners)
        mpMouseListeners->notify( aEvent );
    updateimpl( aGuard, mpSlideShow ); // warning: clears guard!
}

// XMouseMotionListener implementation
void SAL_CALL SlideShowView::mouseDragged( const awt::MouseEvent& e )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    WrappedMouseMotionEvent aEvent;
    aEvent.meType = WrappedMouseMotionEvent::DRAGGED;
    aEvent.maEvent = e;
    aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    if (mpMouseMotionListeners)
        mpMouseMotionListeners->notify( aEvent );
    updateimpl( aGuard, mpSlideShow ); // warning: clears guard!
}

void SAL_CALL SlideShowView::mouseMoved( const awt::MouseEvent& e )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    // Change event source, to enable listeners to match event
    // with view
    WrappedMouseMotionEvent aEvent;
    aEvent.meType = WrappedMouseMotionEvent::MOVED;
    aEvent.maEvent = e;
    aEvent.maEvent.Source = static_cast< ::cppu::OWeakObject* >( this );

    if (mpMouseMotionListeners)
        mpMouseMotionListeners->notify( aEvent );
    updateimpl( aGuard, mpSlideShow ); // warning: clears guard!
}

void SlideShowView::init()
{
    mxWindow->addWindowListener( this );
    mxWindow->addMouseListener( this );

    mxPointer = awt::Pointer::create( ::comphelper::getProcessComponentContext() );

    getTransformation();

    // #i48939# only switch on kind of hacky scroll optimization, when
    // running fullscreen. this minimizes the probability that other
    // windows partially cover the show.
    if( !mbFullScreen )
        return;

    try
    {
        Reference< beans::XPropertySet > xCanvasProps( getCanvas(),
                                                       uno::UNO_QUERY_THROW );
        xCanvasProps->setPropertyValue("UnsafeScrolling",
            uno::makeAny( true ) );
    }
    catch( uno::Exception& )
    {
    }
}

} // namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
