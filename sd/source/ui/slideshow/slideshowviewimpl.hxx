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

#pragma once

#include <comphelper/compbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <cppcanvas/spritecanvas.hxx>
#include <cppuhelper/weakref.hxx>

#include <slideshow.hxx>

namespace com::sun::star::awt { class XPointer; }
namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::awt { class XWindowPeer; }
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

// SlideShowViewPaintListeners
typedef ::comphelper::OInterfaceContainerHelper4< css::awt::XPaintListener >  SlideShowViewPaintListeners;


// SlideShowViewMouseListeners
typedef ::comphelper::OInterfaceContainerHelper4< css::awt::XMouseListener > SlideShowViewMouseListeners_Base;

class SlideShowViewMouseListeners : public SlideShowViewMouseListeners_Base
{
public:
    void notify(std::unique_lock<std::mutex>& rGuard, const WrappedMouseEvent& rEvent);
};


// SlideShowViewMouseMotionListeners
typedef ::comphelper::OInterfaceContainerHelper4< css::awt::XMouseMotionListener > SlideShowViewMouseMotionListeners_Base;

class SlideShowViewMouseMotionListeners : public SlideShowViewMouseMotionListeners_Base
{
public:
    void notify( std::unique_lock<std::mutex>& rGuard, const WrappedMouseMotionEvent& rEvent );
};

// SlideShowView
class ShowWindow;
class SlideshowImpl;

typedef comphelper::WeakComponentImplHelper< css::presentation::XSlideShowView,
                                            css::awt::XWindowListener,
                                            css::awt::XMouseListener,
                                            css::awt::XMouseMotionListener > SlideShowView_Base;

class SlideShowView final : public SlideShowView_Base
{
public:
    SlideShowView( ShowWindow&     rOutputWindow,
                   SdDrawDocument* pDoc,
                   AnimationMode   eAnimationMode,
                   SlideshowImpl*  pSlideShow,
                   bool            bFullScreen );

    void ignoreNextMouseReleased();

    /// Dispose all internal references
    virtual void disposing(std::unique_lock<std::mutex>&) override;

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

protected:
    virtual ~SlideShowView() override {}

private:
    void updateimpl( std::unique_lock<std::mutex>& rGuard, SlideshowImpl* pSlideShow );

    void disposingImpl( std::unique_lock<std::mutex>& );

    ::cppcanvas::SpriteCanvasSharedPtr                    mpCanvas;
    css::uno::Reference< css::awt::XWindow >              mxWindow;
    css::uno::Reference< css::awt::XWindowPeer >          mxWindowPeer;
    css::uno::Reference< css::awt::XPointer >             mxPointer;
    SlideshowImpl*                          mpSlideShow;
    ShowWindow&                             mrOutputWindow;
    std::vector< css::uno::WeakReference< css::util::XModifyListener > >
                                            maViewListeners;
    SlideShowViewPaintListeners             maPaintListeners;
    SlideShowViewMouseListeners             maMouseListeners;
    SlideShowViewMouseMotionListeners       maMouseMotionListeners;
    SdDrawDocument*                         mpDoc;
    bool                                    mbIsMouseMotionListener;
    AnimationMode                           meAnimationMode;
    bool                                    mbFirstPaint;
    bool                                    mbMousePressedEaten;
    css::geometry::IntegerSize2D            mTranslationOffset;
};

} // namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
