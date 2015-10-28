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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESHOW_SLIDESHOWVIEWIMPL_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESHOW_SLIDESHOWVIEWIMPL_HXX

#include <cppuhelper/compbase.hxx>
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

struct WrappedMouseEvent : public css::lang::EventObject
{
    enum EventType
    {
        PRESSED,
        RELEASED,
        ENTERED,
        EXITED
    };

    EventType              meType;
    css::awt::MouseEvent   maEvent;
};

struct WrappedMouseMotionEvent : public css::lang::EventObject
{
    enum EventType
    {
        DRAGGED,
        MOVED
    };

    EventType              meType;
    css::awt::MouseEvent   maEvent;
};

// SlideShowViewListeners
typedef std::vector< css::uno::WeakReference< css::util::XModifyListener > > ViewListenerVector;
class SlideShowViewListeners
{
public:
    SlideShowViewListeners( ::osl::Mutex& rMutex );

    void    addListener( const css::uno::Reference< css::util::XModifyListener >& _rxListener );
    void    removeListener( const css::uno::Reference< css::util::XModifyListener >& _rxListener );
    bool    notify( const css::lang::EventObject& _rEvent ) throw( css::uno::Exception );
    void    disposing( const css::lang::EventObject& _rEventSource );

protected:
    ViewListenerVector maListeners;
    ::osl::Mutex& mrMutex;
};

// SlideShowViewPaintListeners
typedef ::comphelper::OListenerContainerBase< css::awt::XPaintListener,
                                                css::awt::PaintEvent >         SlideShowViewPaintListeners_Base;

class SlideShowViewPaintListeners : public SlideShowViewPaintListeners_Base
{
public:
    SlideShowViewPaintListeners( ::osl::Mutex& rMutex );

protected:
    virtual bool implTypedNotify( const css::uno::Reference< css::awt::XPaintListener >& rListener, const css::awt::PaintEvent& rEvent ) throw( css::uno::Exception ) override;
};

// SlideShowViewMouseListeners
typedef ::comphelper::OListenerContainerBase< css::awt::XMouseListener, WrappedMouseEvent > SlideShowViewMouseListeners_Base;

class SlideShowViewMouseListeners : public SlideShowViewMouseListeners_Base
{
public:
    SlideShowViewMouseListeners( ::osl::Mutex& rMutex );

protected:
    virtual bool implTypedNotify( const css::uno::Reference< css::awt::XMouseListener >&  rListener,
                             const WrappedMouseEvent&                   rEvent ) throw( css::uno::Exception ) override;
};


// SlideShowViewMouseMotionListeners
typedef ::comphelper::OListenerContainerBase< css::awt::XMouseMotionListener,
                                                WrappedMouseMotionEvent > SlideShowViewMouseMotionListeners_Base;

class SlideShowViewMouseMotionListeners : public SlideShowViewMouseMotionListeners_Base
{
public:
    SlideShowViewMouseMotionListeners( ::osl::Mutex& rMutex );

protected:
    virtual bool implTypedNotify( const css::uno::Reference< css::awt::XMouseMotionListener >&    rListener,
                             const WrappedMouseMotionEvent&                 rEvent ) throw( css::uno::Exception ) override;
};

// SlideShowView
class ShowWindow;
class SlideshowImpl;

typedef ::cppu::WeakComponentImplHelper< css::presentation::XSlideShowView,
                                            css::awt::XWindowListener,
                                            css::awt::XMouseListener,
                                            css::awt::XMouseMotionListener > SlideShowView_Base;

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
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;

    /// Disposing our broadcaster
    virtual void SAL_CALL disposing( const css::lang::EventObject& ) throw(css::uno::RuntimeException, std::exception) override;

    void SAL_CALL paint( const css::awt::PaintEvent& e ) throw (css::uno::RuntimeException);

    // XSlideShowView methods
    virtual css::uno::Reference< css::rendering::XSpriteCanvas > SAL_CALL getCanvas(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clear(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::geometry::AffineMatrix2D SAL_CALL getTransformation(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::geometry::IntegerSize2D SAL_CALL getTranslationOffset( ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addTransformationChangedListener( const css::uno::Reference< css::util::XModifyListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTransformationChangedListener( const css::uno::Reference< css::util::XModifyListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMouseCursor( sal_Int16 nPointerShape ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getCanvasArea(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XWindowListener methods
    virtual void SAL_CALL windowResized( const css::awt::WindowEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowShown( const css::lang::EventObject& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowHidden( const css::lang::EventObject& e ) throw (css::uno::RuntimeException, std::exception) override;

    // XMouseListener implementation
    virtual void SAL_CALL mousePressed( const css::awt::MouseEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL mouseReleased( const css::awt::MouseEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL mouseEntered( const css::awt::MouseEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL mouseExited( const css::awt::MouseEvent& e ) throw (css::uno::RuntimeException, std::exception) override;

    // XMouseMotionListener implementation
    virtual void SAL_CALL mouseDragged( const css::awt::MouseEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL mouseMoved( const css::awt::MouseEvent& e ) throw (css::uno::RuntimeException, std::exception) override;

    using cppu::WeakComponentImplHelperBase::disposing;

protected:
    virtual ~SlideShowView() {}

private:
    void init();

    void updateimpl( ::osl::ClearableMutexGuard& rGuard, SlideshowImpl* pSlideShow );

    ::cppcanvas::SpriteCanvasSharedPtr                    mpCanvas;
    css::uno::Reference< css::awt::XWindow >              mxWindow;
    css::uno::Reference< css::awt::XWindowPeer >          mxWindowPeer;
    css::uno::Reference< css::awt::XPointer >             mxPointer;
    SlideshowImpl*                          mpSlideShow;
    ShowWindow&                             mrOutputWindow;
    ::std::unique_ptr< SlideShowViewListeners >
                                            mpViewListeners;
    ::std::unique_ptr< SlideShowViewPaintListeners >
                                            mpPaintListeners;
    ::std::unique_ptr< SlideShowViewMouseListeners >
                                            mpMouseListeners;
    ::std::unique_ptr< SlideShowViewMouseMotionListeners >
                                            mpMouseMotionListeners;
    SdDrawDocument*                         mpDoc;
    bool                                    mbIsMouseMotionListener;
    Rectangle                               maPresentationArea;
    AnimationMode                           meAnimationMode;
    bool                                    mbFirstPaint;
    bool                                    mbFullScreen;
    bool                                    mbMousePressedEaten;
    css::geometry::IntegerSize2D            mTranslationOffset;
};

} // namespace ::sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
