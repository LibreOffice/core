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

#include <memory>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/listenernotification.hxx>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <cppcanvas/spritecanvas.hxx>
#include <tools/gen.hxx>

#include <slideshow.hxx>

namespace com { namespace sun { namespace star { namespace awt { class XPointer; } } } }
namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } }
namespace com { namespace sun { namespace star { namespace awt { class XWindowPeer; } } } }
namespace com { namespace sun { namespace star { namespace awt { struct WindowEvent; } } } }
namespace com { namespace sun { namespace star { namespace rendering { class XSpriteCanvas; } } } }
class SdDrawDocument;

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
class SlideShowViewListeners final
{
public:
    SlideShowViewListeners( ::osl::Mutex& rMutex );

    void    addListener( const css::uno::Reference< css::util::XModifyListener >& _rxListener );
    void    removeListener( const css::uno::Reference< css::util::XModifyListener >& _rxListener );
    /// @throws css::uno::Exception
    void    notify( const css::lang::EventObject& _rEvent );
    void    disposing( const css::lang::EventObject& _rEventSource );

private:
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
    virtual bool implTypedNotify( const css::uno::Reference< css::awt::XPaintListener >& rListener, const css::awt::PaintEvent& rEvent ) override;
};

// SlideShowViewMouseListeners
typedef ::comphelper::OListenerContainerBase< css::awt::XMouseListener, WrappedMouseEvent > SlideShowViewMouseListeners_Base;

class SlideShowViewMouseListeners : public SlideShowViewMouseListeners_Base
{
public:
    SlideShowViewMouseListeners( ::osl::Mutex& rMutex );

protected:
    virtual bool implTypedNotify( const css::uno::Reference< css::awt::XMouseListener >&  rListener,
                             const WrappedMouseEvent&                   rEvent ) override;
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
                             const WrappedMouseMotionEvent&                 rEvent ) override;
};

// SlideShowView
class ShowWindow;
class SlideshowImpl;

typedef ::cppu::WeakComponentImplHelper< css::presentation::XSlideShowView,
                                            css::awt::XWindowListener,
                                            css::awt::XMouseListener,
                                            css::awt::XMouseMotionListener > SlideShowView_Base;

class SlideShowView : public ::cppu::BaseMutex,
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
    virtual void SAL_CALL dispose() override;

    /// Disposing our broadcaster
    virtual void SAL_CALL disposing( const css::lang::EventObject& ) override;

    /// @throws css::uno::RuntimeException
    void paint( const css::awt::PaintEvent& e );

    // XSlideShowView methods
    virtual css::uno::Reference< css::rendering::XSpriteCanvas > SAL_CALL getCanvas(  ) override;
    virtual void SAL_CALL clear(  ) override;
    virtual css::geometry::AffineMatrix2D SAL_CALL getTransformation(  ) override;
    virtual css::geometry::IntegerSize2D SAL_CALL getTranslationOffset( ) override;
    virtual void SAL_CALL addTransformationChangedListener( const css::uno::Reference< css::util::XModifyListener >& xListener ) override;
    virtual void SAL_CALL removeTransformationChangedListener( const css::uno::Reference< css::util::XModifyListener >& xListener ) override;
    virtual void SAL_CALL addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;
    virtual void SAL_CALL removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;
    virtual void SAL_CALL addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    virtual void SAL_CALL removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    virtual void SAL_CALL addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    virtual void SAL_CALL removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    virtual void SAL_CALL setMouseCursor( sal_Int16 nPointerShape ) override;
    virtual css::awt::Rectangle SAL_CALL getCanvasArea(  ) override;

    // XWindowListener methods
    virtual void SAL_CALL windowResized( const css::awt::WindowEvent& e ) override;
    virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& e ) override;
    virtual void SAL_CALL windowShown( const css::lang::EventObject& e ) override;
    virtual void SAL_CALL windowHidden( const css::lang::EventObject& e ) override;

    // XMouseListener implementation
    virtual void SAL_CALL mousePressed( const css::awt::MouseEvent& e ) override;
    virtual void SAL_CALL mouseReleased( const css::awt::MouseEvent& e ) override;
    virtual void SAL_CALL mouseEntered( const css::awt::MouseEvent& e ) override;
    virtual void SAL_CALL mouseExited( const css::awt::MouseEvent& e ) override;

    // XMouseMotionListener implementation
    virtual void SAL_CALL mouseDragged( const css::awt::MouseEvent& e ) override;
    virtual void SAL_CALL mouseMoved( const css::awt::MouseEvent& e ) override;

    using cppu::WeakComponentImplHelperBase::disposing;

protected:
    virtual ~SlideShowView() override {}

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
    SdDrawDocument* const                   mpDoc;
    bool                                    mbIsMouseMotionListener;
    AnimationMode const                     meAnimationMode;
    bool                                    mbFirstPaint;
    bool const                              mbFullScreen;
    bool                                    mbMousePressedEaten;
    css::geometry::IntegerSize2D            mTranslationOffset;
};

} // namespace ::sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
