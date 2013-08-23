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

#ifndef _SD_SLIDESHOWVIEWIMPL_HXX_
#define _SD_SLIDESHOWVIEWIMPL_HXX_

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/compbase4.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/listenernotification.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/WindowEvent.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XPointer.hpp>
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <comphelper/implementationreference.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <cppcanvas/spritecanvas.hxx>
#include <vcl/help.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/saveopt.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <basic/sbstar.hxx>
#include <svx/svdpagv.hxx>
#include <svx/fmshell.hxx>

#include <svx/svxids.hrc>
#include "sdmod.hxx"
#include "cusshow.hxx"
#include "ViewShellBase.hxx"
#include "PresentationViewShell.hxx"
#include "ViewShell.hxx"
#include "drawview.hxx"
#include "slideshow.hxx"
#include "drawdoc.hxx"
#include "showwindow.hxx"
#include "optsitem.hxx"
#include "FrameView.hxx"
#include "DrawDocShell.hxx"

#include "app.hrc"

namespace sd
{

struct WrappedMouseEvent : public ::com::sun::star::lang::EventObject
{
    enum EventType
    {
        PRESSED,
        RELEASED,
        ENTERED,
        EXITED
    };

    EventType       meType;
    ::com::sun::star::awt::MouseEvent   maEvent;
};

struct WrappedMouseMotionEvent : public ::com::sun::star::lang::EventObject
{
    enum EventType
    {
        DRAGGED,
        MOVED
    };

    EventType       meType;
    ::com::sun::star::awt::MouseEvent   maEvent;
};

///////////////////////////////////////////////////////////////////////
// SlideShowViewListeners
///////////////////////////////////////////////////////////////////////

typedef std::vector< ::com::sun::star::uno::WeakReference< ::com::sun::star::util::XModifyListener > > ViewListenerVector;
class SlideShowViewListeners
{
public:
    SlideShowViewListeners( ::osl::Mutex& rMutex );

    void    addListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& _rxListener );
    void    removeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& _rxListener );
    bool    notify( const ::com::sun::star::lang::EventObject& _rEvent ) throw( com::sun::star::uno::Exception );
    void    disposing( const ::com::sun::star::lang::EventObject& _rEventSource );

protected:
    ViewListenerVector maListeners;
    ::osl::Mutex& mrMutex;
};

typedef ::std::auto_ptr< SlideShowViewListeners >   SlideShowViewListenersPtr;

///////////////////////////////////////////////////////////////////////
// SlideShowViewPaintListeners
///////////////////////////////////////////////////////////////////////

typedef ::comphelper::OListenerContainerBase< ::com::sun::star::awt::XPaintListener,
                                                ::com::sun::star::awt::PaintEvent >         SlideShowViewPaintListeners_Base;

class SlideShowViewPaintListeners : public SlideShowViewPaintListeners_Base
{
public:
    SlideShowViewPaintListeners( ::osl::Mutex& rMutex );

protected:
    virtual bool implTypedNotify( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rListener, const ::com::sun::star::awt::PaintEvent& rEvent ) throw( ::com::sun::star::uno::Exception );
};
typedef ::std::auto_ptr< SlideShowViewPaintListeners >  SlideShowViewPaintListenersPtr;

///////////////////////////////////////////////////////////////////////
// SlideShowViewMouseListeners
///////////////////////////////////////////////////////////////////////

typedef ::comphelper::OListenerContainerBase< ::com::sun::star::awt::XMouseListener, WrappedMouseEvent > SlideShowViewMouseListeners_Base;

class SlideShowViewMouseListeners : public SlideShowViewMouseListeners_Base
{
public:
    SlideShowViewMouseListeners( ::osl::Mutex& rMutex );

protected:
    virtual bool implTypedNotify( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >&  rListener,
                             const WrappedMouseEvent&                   rEvent ) throw( ::com::sun::star::uno::Exception );
};

typedef ::std::auto_ptr< SlideShowViewMouseListeners >  SlideShowViewMouseListenersPtr;


///////////////////////////////////////////////////////////////////////
// SlideShowViewMouseMotionListeners
///////////////////////////////////////////////////////////////////////

typedef ::comphelper::OListenerContainerBase< ::com::sun::star::awt::XMouseMotionListener,
                                                WrappedMouseMotionEvent > SlideShowViewMouseMotionListeners_Base;

class SlideShowViewMouseMotionListeners : public SlideShowViewMouseMotionListeners_Base
{
public:
    SlideShowViewMouseMotionListeners( ::osl::Mutex& rMutex );

protected:
    virtual bool implTypedNotify( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >&    rListener,
                             const WrappedMouseMotionEvent&                 rEvent ) throw( ::com::sun::star::uno::Exception );
};
typedef ::std::auto_ptr< SlideShowViewMouseMotionListeners >    SlideShowViewMouseMotionListenersPtr;


///////////////////////////////////////////////////////////////////////
// SlideShowView
///////////////////////////////////////////////////////////////////////

class ShowWindow;
class SlideshowImpl;

typedef ::cppu::WeakComponentImplHelper4< ::com::sun::star::presentation::XSlideShowView,
                                            ::com::sun::star::awt::XWindowListener,
                                            ::com::sun::star::awt::XMouseListener,
                                            ::com::sun::star::awt::XMouseMotionListener > SlideShowView_Base;

class SlideShowView : public ::comphelper::OBaseMutex,
                    public SlideShowView_Base
{
public:
    SlideShowView( ShowWindow&     rOutputWindow,
                   SdDrawDocument* pDoc,
                   AnimationMode   eAnimationMode,
                   SlideshowImpl*  pSlideShow,
                   bool            bFullScreen );

    void ignoreNextMouseReleased() { mbMousePressedEaten = true; }

    /// Dispose all internal references
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

    /// Disposing our broadcaster
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL paint( const ::com::sun::star::awt::PaintEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    // XSlideShowView methods
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSpriteCanvas > SAL_CALL getCanvas(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clear(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::geometry::AffineMatrix2D SAL_CALL getTransformation(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::geometry::IntegerSize2D SAL_CALL getTranslationOffset( ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addTransformationChangedListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTransformationChangedListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMouseCursor( sal_Int16 nPointerShape ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getCanvasArea(  ) throw (::com::sun::star::uno::RuntimeException);

    // XWindowListener methods
    virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

    // XMouseListener implementation
    virtual void SAL_CALL mousePressed( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseReleased( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseEntered( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseExited( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    // XMouseMotionListener implementation
    virtual void SAL_CALL mouseDragged( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseMoved( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    using cppu::WeakComponentImplHelperBase::disposing;

protected:
    ~SlideShowView() {}

private:
    void init();

    void updateimpl( ::osl::ClearableMutexGuard& rGuard, SlideshowImpl* pSlideShow );

    ::cppcanvas::SpriteCanvasSharedPtr                                              mpCanvas;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >              mxWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >          mxWindowPeer;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer >             mxPointer;
    SlideshowImpl*                          mpSlideShow;
    ShowWindow&                             mrOutputWindow;
    SlideShowViewListenersPtr               mpViewListeners;
    SlideShowViewPaintListenersPtr          mpPaintListeners;
    SlideShowViewMouseListenersPtr          mpMouseListeners;
    SlideShowViewMouseMotionListenersPtr    mpMouseMotionListeners;
    SdDrawDocument*                         mpDoc;
    bool                                    mbIsMouseMotionListener;
    Rectangle                               maPresentationArea;
    AnimationMode                           meAnimationMode;
    bool                                    mbFirstPaint;
    bool                                    mbFullScreen;
    bool                                    mbMousePressedEaten;
    ::com::sun::star::geometry::IntegerSize2D mTranslationOffset;
};


} // namespace ::sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
