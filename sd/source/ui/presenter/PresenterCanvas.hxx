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

#ifndef INCLUDED_SD_SOURCE_UI_PRESENTER_PRESENTERCANVAS_HXX
#define INCLUDED_SD_SOURCE_UI_PRESENTER_PRESENTERCANVAS_HXX

#include "CanvasUpdateRequester.hxx"
#include <basegfx/range/b2drectangle.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/VolatileContentDestroyedException.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <boost/noncopyable.hpp>
#include <memory>

namespace sd { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::rendering::XSpriteCanvas,
        css::rendering::XBitmap,
        css::awt::XWindowListener,
        css::lang::XInitialization,
        css::lang::XServiceInfo
    > PresenterCanvasInterfaceBase;
}

/** Wrapper around a shared canvas that forwards most of its methods to the
    shared canvas.  Most notable differences are:
    1. The transformation  of the ViewState of forwarded calls is modified by adding
    an offset.
    2. The clip polygon of the ViewState of forwarded calls is intersected
    with a clip rectangle that can be set via SetClip().
    3. Calls to updateScreen() are collected.  One call to the updateScreen()
    method of the shared canvas is made asynchronously.

    The canvas can use different canvases for sharing and for sprite
    construction.  This allows the shared canvas to be a canvas of sprite itself.
*/
class PresenterCanvas
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterCanvasInterfaceBase
{
public:
    /** This constructor is used when a PresenterCanvas object is created as
        a service.
    */
    PresenterCanvas();

    /** This constructor is used when a PresenterCanvas object is created
        directly, typically by the PresenterCanvasFactory.
        @param rxUpdateCanvas
            This canvas is used to call updateScreen() at and to create
            sprites.  In the typical case this canvas is identical to the
            rxSharedCanvas argument.
        @param rxUpdateWindow
            The window that belongs to the canvas given by the
            rxUpdateCanvas argument.
        @param rxSharedCanvas
            The canvas that is wrapped by the new instance of this class.
            Typically this is a regular XSpriteCanvas and then is identical
            to the one given by the rxUpdateCanvas argument.  It may be the
            canvas of a sprite which does not support the XSpriteCanvas
            interface.  In that case the canvas that created the sprite can
            be given as rxUpdateCanvas argument to allow to create further
            sprites and to have proper calls to updateScreen().
        @param rxSharedWindow
            The window that belongs to the canvas given by the
            rxSharedCanvas argument.
        @param rxWindow
            The window that is represented by the new PresenterCanvas
            object.  It is expected to be a direct descendant of
            rxSharedWindow.  Its position inside rxSharedWindow defines the
            offset of the canvas implemented by the new PresenterCanvas
            object and rxSharedCanvas.
    */
    PresenterCanvas (
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxUpdateCanvas,
        const css::uno::Reference<css::awt::XWindow>& rxUpdateWindow,
        const css::uno::Reference<css::rendering::XCanvas>& rxSharedCanvas,
        const css::uno::Reference<css::awt::XWindow>& rxSharedWindow,
        const css::uno::Reference<css::awt::XWindow>& rxWindow);
    virtual ~PresenterCanvas();

    virtual void SAL_CALL disposing()
        throw (css::uno::RuntimeException) SAL_OVERRIDE;

    css::awt::Point GetOffset (const css::uno::Reference<css::awt::XWindow>& rxBaseWindow);

    /** Merge the given view state with the view state that translates the
        (virtual) child canvas to the shared canvas.
    */
    css::rendering::ViewState MergeViewState (
        const css::rendering::ViewState& rViewState,
        const css::awt::Point& raOffset);

    /** Called by custom sprites to update their clip polygon so that they
        are clipped at the borders of the canvas.  This method has to be
        called after each change of the sprite location so that the bounds
        of the canvas can be transformed into the coordinate system of the
        sprite.
    */
    css::uno::Reference<css::rendering::XPolyPolygon2D> UpdateSpriteClip (
        const css::uno::Reference<css::rendering::XPolyPolygon2D>& rxOriginalClip,
        const css::geometry::RealPoint2D& rLocation,
        const css::geometry::RealSize2D& rSize);

    // XInitialization

    virtual void SAL_CALL initialize (
        const css::uno::Sequence<css::uno::Any>& rArguments)
        throw(css::uno::Exception, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XCanvas

    virtual void SAL_CALL clear()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL drawPoint (
        const css::geometry::RealPoint2D& aPoint,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL drawLine (
        const css::geometry::RealPoint2D& aStartPoint,
        const css::geometry::RealPoint2D& aEndPoint,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL drawBezier (
        const css::geometry::RealBezierSegment2D& aBezierSegment,
        const css::geometry::RealPoint2D& aEndPoint,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL drawPolyPolygon (
        const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL strokePolyPolygon (
        const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState,
        const css::rendering::StrokeAttributes& aStrokeAttributes)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        strokeTexturedPolyPolygon (
            const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            const css::uno::Sequence< css::rendering::Texture >& aTextures,
            const css::rendering::StrokeAttributes& aStrokeAttributes)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        strokeTextureMappedPolyPolygon(
            const css::uno::Reference<css::rendering::XPolyPolygon2D >& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            const css::uno::Sequence<css::rendering::Texture>& aTextures,
            const css::uno::Reference<css::geometry::XMapping2D>& xMapping,
            const css::rendering::StrokeAttributes& aStrokeAttributes)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XPolyPolygon2D> SAL_CALL
        queryStrokeShapes(
            const css::uno::Reference<css::rendering::XPolyPolygon2D>& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            const css::rendering::StrokeAttributes& aStrokeAttributes)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        fillPolyPolygon(
            const css::uno::Reference<css::rendering::XPolyPolygon2D>& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        fillTexturedPolyPolygon(
            const css::uno::Reference<css::rendering::XPolyPolygon2D>& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            const css::uno::Sequence<css::rendering::Texture>& xTextures)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        fillTextureMappedPolyPolygon(
            const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            const css::uno::Sequence< css::rendering::Texture >& xTextures,
            const css::uno::Reference< css::geometry::XMapping2D >& xMapping)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCanvasFont> SAL_CALL
        createFont(
            const css::rendering::FontRequest& aFontRequest,
            const css::uno::Sequence< css::beans::PropertyValue >& aExtraFontProperties,
            const css::geometry::Matrix2D& aFontMatrix)
        throw (css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Sequence<css::rendering::FontInfo> SAL_CALL
        queryAvailableFonts(
            const css::rendering::FontInfo& aFilter,
            const css::uno::Sequence< css::beans::PropertyValue >& aFontProperties)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        drawText(
            const css::rendering::StringContext& aText,
            const css::uno::Reference< css::rendering::XCanvasFont >& xFont,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            ::sal_Int8 nTextDirection)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        drawTextLayout(
            const css::uno::Reference< css::rendering::XTextLayout >& xLayoutetText,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        drawBitmap(
            const css::uno::Reference< css::rendering::XBitmap >& xBitmap,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        drawBitmapModulated(
            const css::uno::Reference< css::rendering::XBitmap>& xBitmap,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XGraphicDevice> SAL_CALL
        getDevice()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XSpriteCanvas

    css::uno::Reference< css::rendering::XAnimatedSprite > SAL_CALL
        createSpriteFromAnimation (
            const css::uno::Reference< css::rendering::XAnimation >& animation)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Reference< css::rendering::XAnimatedSprite > SAL_CALL
        createSpriteFromBitmaps (
            const css::uno::Sequence<
                css::uno::Reference< css::rendering::XBitmap > >& animationBitmaps,
            ::sal_Int8 interpolationMode)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Reference< css::rendering::XCustomSprite > SAL_CALL
        createCustomSprite (
            const css::geometry::RealSize2D& spriteSize)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Reference< css::rendering::XSprite > SAL_CALL
        createClonedSprite (
            const css::uno::Reference< css::rendering::XSprite >& original)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    sal_Bool SAL_CALL updateScreen (sal_Bool bUpdateAll)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XEventListener

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XBitmap

    virtual css::geometry::IntegerSize2D SAL_CALL getSize()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasAlpha()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XBitmap> SAL_CALL getScaledBitmap(
        const css::geometry::RealSize2D& rNewSize,
        sal_Bool bFast)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException, std::exception) SAL_OVERRIDE;

private:
    css::uno::Reference<css::rendering::XSpriteCanvas> mxUpdateCanvas;
    css::uno::Reference<css::awt::XWindow> mxUpdateWindow;
    css::uno::Reference<css::rendering::XCanvas> mxSharedCanvas;
    css::uno::Reference<css::awt::XWindow> mxSharedWindow;

    /** The window for which a canvas is emulated.
    */
    css::uno::Reference<css::awt::XWindow> mxWindow;

    /** Offset of the emulated canvas with respect to the shared canvas.
    */
    css::awt::Point maOffset;

    /** The UpdateRequester is used by updateScreen() to schedule
        updateScreen() calls at the shared canvas.
    */
    std::shared_ptr<CanvasUpdateRequester> mpUpdateRequester;

    /** The clip rectangle as given to SetClip().
    */
    css::awt::Rectangle maClipRectangle;

    /** When this flag is true (it is set to true after every call to
        updateScreen()) then the next call to MergeViewState updates the
        maOffset member.  A possible optimization would set this flag only
        to true when one of the windows between mxWindow and mxSharedWindow
        changes its position.
    */
    bool mbOffsetUpdatePending;

    ::basegfx::B2DRectangle GetClipRectangle (
        const css::geometry::AffineMatrix2D& rViewTransform,
        const css::awt::Point& rOffset);

    css::rendering::ViewState MergeViewState (const css::rendering::ViewState& rViewState);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed()
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
