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

#include "PresenterCanvas.hxx"
#include "facreg.hxx"

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <canvas/canvastools.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <boost/noncopyable.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace presenter {

//===== PresenterCustomSprite =================================================

/** Wrapper around a sprite that is displayed on a PresenterCanvas.
*/
namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::rendering::XCustomSprite
    > PresenterCustomSpriteInterfaceBase;
}
class PresenterCustomSprite
    : private ::boost::noncopyable,
      protected ::cppu::BaseMutex,
      public PresenterCustomSpriteInterfaceBase
{
public:
    PresenterCustomSprite (
        const rtl::Reference<PresenterCanvas>& rpCanvas,
        const Reference<rendering::XCustomSprite>& rxSprite,
        const Reference<awt::XWindow>& rxBaseWindow,
        const css::geometry::RealSize2D& rSpriteSize);
    virtual ~PresenterCustomSprite();
    virtual void SAL_CALL disposing()
        throw (RuntimeException) override;

    // XSprite

    virtual void SAL_CALL setAlpha (double nAlpha)
        throw (lang::IllegalArgumentException,RuntimeException, std::exception) override;

    virtual void SAL_CALL move (const geometry::RealPoint2D& rNewPos,
        const rendering::ViewState& rViewState,
        const rendering::RenderState& rRenderState)
        throw (lang::IllegalArgumentException,RuntimeException, std::exception) override;

    virtual void SAL_CALL transform (const geometry::AffineMatrix2D& rTransformation)
        throw (lang::IllegalArgumentException,RuntimeException, std::exception) override;

    virtual void SAL_CALL clip (const Reference<rendering::XPolyPolygon2D>& rClip)
        throw (RuntimeException, std::exception) override;

    virtual void SAL_CALL setPriority (double nPriority)
        throw (RuntimeException, std::exception) override;

    virtual void SAL_CALL show()
        throw (RuntimeException, std::exception) override;

    virtual void SAL_CALL hide()
        throw (RuntimeException, std::exception) override;

    // XCustomSprite

    virtual Reference<rendering::XCanvas> SAL_CALL getContentCanvas()
        throw (RuntimeException, std::exception) override;

private:
    rtl::Reference<PresenterCanvas> mpCanvas;
    Reference<rendering::XCustomSprite> mxSprite;
    Reference<awt::XWindow> mxBaseWindow;
    geometry::RealPoint2D maPosition;
    geometry::RealSize2D maSpriteSize;

    void ThrowIfDisposed()
        throw (css::lang::DisposedException);
};

//===== PresenterCanvas =======================================================

PresenterCanvas::PresenterCanvas()
    : PresenterCanvasInterfaceBase(m_aMutex),
      mxUpdateCanvas(),
      mxSharedCanvas(),
      mxSharedWindow(),
      mxWindow(),
      maOffset(),
      mpUpdateRequester(),
      maClipRectangle(),
      mbOffsetUpdatePending(true)
{
}

PresenterCanvas::PresenterCanvas (
    const Reference<rendering::XSpriteCanvas>& rxUpdateCanvas,
    const Reference<awt::XWindow>& rxUpdateWindow,
    const Reference<rendering::XCanvas>& rxSharedCanvas,
    const Reference<awt::XWindow>& rxSharedWindow,
    const Reference<awt::XWindow>& rxWindow)
    : PresenterCanvasInterfaceBase(m_aMutex),
      mxUpdateCanvas(rxUpdateCanvas),
      mxUpdateWindow(rxUpdateWindow),
      mxSharedCanvas(rxSharedCanvas),
      mxSharedWindow(rxSharedWindow),
      mxWindow(rxWindow),
      maOffset(),
      mpUpdateRequester(),
      maClipRectangle(),
      mbOffsetUpdatePending(true)
{
    if (mxWindow.is())
        mxWindow->addWindowListener(this);

    if (mxUpdateCanvas.is())
        mpUpdateRequester = CanvasUpdateRequester::Instance(mxUpdateCanvas);
}

PresenterCanvas::~PresenterCanvas()
{
}

void SAL_CALL PresenterCanvas::disposing()
    throw (css::uno::RuntimeException)
{
    if (mxWindow.is())
        mxWindow->removeWindowListener(this);
}

//----- XInitialization -------------------------------------------------------

void SAL_CALL PresenterCanvas::initialize (
    const Sequence<Any>& rArguments)
    throw(Exception, RuntimeException, std::exception)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        ThrowIfDisposed();

    if (rArguments.getLength() == 5)
    {
        try
        {
            // First and second argument may be NULL.
            rArguments[0] >>= mxUpdateCanvas;
            rArguments[1] >>= mxUpdateWindow;

            if ( ! (rArguments[2] >>= mxSharedWindow))
            {
                throw lang::IllegalArgumentException("PresenterCanvas: invalid shared window",
                    static_cast<XWeak*>(this),
                    1);
            }

            if ( ! (rArguments[3] >>= mxSharedCanvas))
            {
                throw lang::IllegalArgumentException("PresenterCanvas: invalid shared canvas",
                    static_cast<XWeak*>(this),
                    2);
            }

            if ( ! (rArguments[4] >>= mxWindow))
            {
                throw lang::IllegalArgumentException("PresenterCanvas: invalid window",
                    static_cast<XWeak*>(this),
                    3);
            }

            mpUpdateRequester = CanvasUpdateRequester::Instance(mxUpdateCanvas);

            mbOffsetUpdatePending = true;
            if (mxWindow.is())
                mxWindow->addWindowListener(this);
        }
        catch (RuntimeException&)
        {
            mxSharedWindow = NULL;
            mxWindow = NULL;
            throw;
        }
    }
    else
    {
        throw RuntimeException("PresenterCanvas: invalid number of arguments",
                static_cast<XWeak*>(this));
    }
}

OUString PresenterCanvas::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.Draw.PresenterCanvasFactory");
}

sal_Bool PresenterCanvas::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> PresenterCanvas::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<OUString>{"com.sun.star.rendering.Canvas"};
}

//----- XCanvas ---------------------------------------------------------------

void SAL_CALL PresenterCanvas::clear()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    // ToDo: Clear the area covered by the child window.  A simple forward
    // would clear the whole shared canvas.
}

void SAL_CALL PresenterCanvas::drawPoint (
    const css::geometry::RealPoint2D& aPoint,
    const css::rendering::ViewState& aViewState,
    const css::rendering::RenderState& aRenderState)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    mxSharedCanvas->drawPoint(aPoint,MergeViewState(aViewState),aRenderState);
}

void SAL_CALL PresenterCanvas::drawLine (
        const css::geometry::RealPoint2D& aStartPoint,
        const css::geometry::RealPoint2D& aEndPoint,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    mxSharedCanvas->drawLine(aStartPoint,aEndPoint,MergeViewState(aViewState),aRenderState);
}

void SAL_CALL PresenterCanvas::drawBezier (
        const css::geometry::RealBezierSegment2D& aBezierSegment,
        const css::geometry::RealPoint2D& aEndPoint,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    mxSharedCanvas->drawBezier(aBezierSegment,aEndPoint,MergeViewState(aViewState),aRenderState);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL PresenterCanvas::drawPolyPolygon (
        const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->drawPolyPolygon(
        xPolyPolygon, MergeViewState(aViewState), aRenderState);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL PresenterCanvas::strokePolyPolygon (
        const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState,
        const css::rendering::StrokeAttributes& aStrokeAttributes)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->strokePolyPolygon(
        xPolyPolygon, MergeViewState(aViewState), aRenderState, aStrokeAttributes);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        PresenterCanvas::strokeTexturedPolyPolygon (
            const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            const css::uno::Sequence< css::rendering::Texture >& aTextures,
            const css::rendering::StrokeAttributes& aStrokeAttributes)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->strokeTexturedPolyPolygon(
        xPolyPolygon, MergeViewState(aViewState), aRenderState, aTextures, aStrokeAttributes);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        PresenterCanvas::strokeTextureMappedPolyPolygon(
            const css::uno::Reference<css::rendering::XPolyPolygon2D >& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            const css::uno::Sequence<css::rendering::Texture>& aTextures,
            const css::uno::Reference<css::geometry::XMapping2D>& xMapping,
            const css::rendering::StrokeAttributes& aStrokeAttributes)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->strokeTextureMappedPolyPolygon(
        xPolyPolygon,
        MergeViewState(aViewState),
        aRenderState,
        aTextures,
        xMapping,
        aStrokeAttributes);
}

css::uno::Reference<css::rendering::XPolyPolygon2D> SAL_CALL
        PresenterCanvas::queryStrokeShapes(
            const css::uno::Reference<css::rendering::XPolyPolygon2D>& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            const css::rendering::StrokeAttributes& aStrokeAttributes)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->queryStrokeShapes(
        xPolyPolygon, MergeViewState(aViewState), aRenderState, aStrokeAttributes);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        PresenterCanvas::fillPolyPolygon(
            const css::uno::Reference<css::rendering::XPolyPolygon2D>& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->fillPolyPolygon(
        xPolyPolygon, MergeViewState(aViewState), aRenderState);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        PresenterCanvas::fillTexturedPolyPolygon(
            const css::uno::Reference<css::rendering::XPolyPolygon2D>& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            const css::uno::Sequence<css::rendering::Texture>& xTextures)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->fillTexturedPolyPolygon(
        xPolyPolygon, MergeViewState(aViewState), aRenderState, xTextures);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        PresenterCanvas::fillTextureMappedPolyPolygon(
            const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            const css::uno::Sequence< css::rendering::Texture >& xTextures,
            const css::uno::Reference< css::geometry::XMapping2D >& xMapping)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->fillTextureMappedPolyPolygon(
        xPolyPolygon, MergeViewState(aViewState), aRenderState, xTextures, xMapping);
}

css::uno::Reference<css::rendering::XCanvasFont> SAL_CALL
        PresenterCanvas::createFont(
            const css::rendering::FontRequest& aFontRequest,
            const css::uno::Sequence< css::beans::PropertyValue >& aExtraFontProperties,
            const css::geometry::Matrix2D& aFontMatrix)
        throw (css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->createFont(
        aFontRequest, aExtraFontProperties, aFontMatrix);
}

css::uno::Sequence<css::rendering::FontInfo> SAL_CALL
        PresenterCanvas::queryAvailableFonts(
            const css::rendering::FontInfo& aFilter,
            const css::uno::Sequence< css::beans::PropertyValue >& aFontProperties)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->queryAvailableFonts(aFilter, aFontProperties);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        PresenterCanvas::drawText(
            const css::rendering::StringContext& aText,
            const css::uno::Reference< css::rendering::XCanvasFont >& xFont,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState,
            ::sal_Int8 nTextDirection)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->drawText(
        aText, xFont, MergeViewState(aViewState), aRenderState, nTextDirection);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        PresenterCanvas::drawTextLayout(
            const css::uno::Reference< css::rendering::XTextLayout >& xLayoutetText,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->drawTextLayout(
        xLayoutetText, MergeViewState(aViewState), aRenderState);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        PresenterCanvas::drawBitmap(
            const css::uno::Reference< css::rendering::XBitmap >& xBitmap,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->drawBitmap(
        xBitmap, MergeViewState(aViewState), aRenderState);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL
        PresenterCanvas::drawBitmapModulated(
            const css::uno::Reference< css::rendering::XBitmap>& xBitmap,
            const css::rendering::ViewState& aViewState,
            const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->drawBitmapModulated(
        xBitmap, MergeViewState(aViewState), aRenderState);
}

css::uno::Reference<css::rendering::XGraphicDevice> SAL_CALL
        PresenterCanvas::getDevice()
        throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSharedCanvas->getDevice();
}

//----- XSpriteCanvas ---------------------------------------------------------

Reference<rendering::XAnimatedSprite> SAL_CALL
    PresenterCanvas::createSpriteFromAnimation (
        const css::uno::Reference<css::rendering::XAnimation>& rAnimation)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxSharedCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        return xSpriteCanvas->createSpriteFromAnimation(rAnimation);
    else
        return NULL;
}

Reference<rendering::XAnimatedSprite> SAL_CALL
    PresenterCanvas::createSpriteFromBitmaps (
        const css::uno::Sequence<
            css::uno::Reference< css::rendering::XBitmap > >& rAnimationBitmaps,
    ::sal_Int8 nInterpolationMode)
        throw (css::lang::IllegalArgumentException,
            css::rendering::VolatileContentDestroyedException,
            css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxSharedCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        return xSpriteCanvas->createSpriteFromBitmaps(rAnimationBitmaps, nInterpolationMode);
    else
        return NULL;
}

Reference<rendering::XCustomSprite> SAL_CALL
    PresenterCanvas::createCustomSprite (
        const css::geometry::RealSize2D& rSpriteSize)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxSharedCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        return new PresenterCustomSprite(
            this,
            xSpriteCanvas->createCustomSprite(rSpriteSize),
            mxSharedWindow,
            rSpriteSize);
    else if (mxUpdateCanvas.is())
        return new PresenterCustomSprite(
            this,
            mxUpdateCanvas->createCustomSprite(rSpriteSize),
            mxUpdateWindow,
            rSpriteSize);
    else
        return NULL;
}

Reference<rendering::XSprite> SAL_CALL
    PresenterCanvas::createClonedSprite (
        const css::uno::Reference< css::rendering::XSprite >& rxOriginal)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxSharedCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        return xSpriteCanvas->createClonedSprite(rxOriginal);
    if (mxUpdateCanvas.is())
        return mxUpdateCanvas->createClonedSprite(rxOriginal);
    return NULL;
}

sal_Bool SAL_CALL PresenterCanvas::updateScreen (sal_Bool bUpdateAll)
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    mbOffsetUpdatePending = true;
    if (mpUpdateRequester.get() != NULL)
    {
        mpUpdateRequester->RequestUpdate(bUpdateAll);
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterCanvas::disposing (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (rEvent.Source == mxWindow)
        mxWindow = NULL;
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterCanvas::windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;
    ThrowIfDisposed();
    mbOffsetUpdatePending = true;
}

void SAL_CALL PresenterCanvas::windowMoved (const css::awt::WindowEvent& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;
    ThrowIfDisposed();
    mbOffsetUpdatePending = true;
}

void SAL_CALL PresenterCanvas::windowShown (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;
    ThrowIfDisposed();
    mbOffsetUpdatePending = true;
}

void SAL_CALL PresenterCanvas::windowHidden (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;
    ThrowIfDisposed();
}

//----- XBitmap ---------------------------------------------------------------

geometry::IntegerSize2D SAL_CALL PresenterCanvas::getSize()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    if (mxWindow.is())
    {
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        return geometry::IntegerSize2D(aWindowBox.Width, aWindowBox.Height);
    }
    else
        return geometry::IntegerSize2D(0,0);
}

sal_Bool SAL_CALL PresenterCanvas::hasAlpha()
    throw (RuntimeException, std::exception)
{
    Reference<rendering::XBitmap> xBitmap (mxSharedCanvas, UNO_QUERY);
    if (xBitmap.is())
        return xBitmap->hasAlpha();
    else
        return sal_False;
}

Reference<rendering::XBitmap> SAL_CALL PresenterCanvas::getScaledBitmap(
    const css::geometry::RealSize2D& rNewSize,
    sal_Bool bFast)
    throw (css::uno::RuntimeException,
        css::lang::IllegalArgumentException,
        css::rendering::VolatileContentDestroyedException, std::exception)
{
    (void)rNewSize;
    (void)bFast;

    ThrowIfDisposed();

    // Not implemented.

    return NULL;
}

rendering::ViewState PresenterCanvas::MergeViewState (
    const rendering::ViewState& rViewState)
{
    // Make sure the offset is up-to-date.
    if (mbOffsetUpdatePending)
        maOffset = GetOffset(mxSharedWindow);
    return MergeViewState(rViewState, maOffset);
}

css::rendering::ViewState PresenterCanvas::MergeViewState (
    const css::rendering::ViewState& rViewState,
    const css::awt::Point& rOffset)
{
    // Early rejects.
    if ( ! mxSharedCanvas.is())
        return rViewState;

    Reference<rendering::XGraphicDevice> xDevice (mxSharedCanvas->getDevice());
    if ( ! xDevice.is())
        return rViewState;

    // Create a modifiable copy of the given view state.
    rendering::ViewState aViewState (rViewState);

    // Prepare the local clip rectangle.
    ::basegfx::B2DRectangle aWindowRange (GetClipRectangle(aViewState.AffineTransform, rOffset));

    // Adapt the offset of the view state.
    aViewState.AffineTransform.m02 += rOffset.X;
    aViewState.AffineTransform.m12 += rOffset.Y;

    // Adapt the clip polygon.
    if ( ! aViewState.Clip.is())
    {
        // Cancel out the later multiplication with the view state
        // transformation.
        aViewState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
            xDevice,
            ::basegfx::B2DPolyPolygon(::basegfx::tools::createPolygonFromRect(aWindowRange)));
    }
    else
    {
        // Have to compute the intersection of the given clipping polygon in
        // the view state and the local clip rectangle.

        // Clip the view state clipping polygon against the local clip rectangle.
        const ::basegfx::B2DPolyPolygon aClipPolygon (
            ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(
                aViewState.Clip));
        const ::basegfx::B2DPolyPolygon aClippedClipPolygon (
            ::basegfx::tools::clipPolyPolygonOnRange(
                aClipPolygon,
                aWindowRange,
                true, /* bInside */
                false /* bStroke */));

        aViewState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
            xDevice,
            aClippedClipPolygon);
    }

    return aViewState;
}

awt::Point PresenterCanvas::GetOffset (const Reference<awt::XWindow>& rxBaseWindow)
{
    mbOffsetUpdatePending = false;
    if (mxWindow.is() && rxBaseWindow.is())
    {
        vcl::Window* pWindow = VCLUnoHelper::GetWindow(mxWindow);
        vcl::Window* pSharedWindow = VCLUnoHelper::GetWindow(rxBaseWindow);
        if (pWindow!=NULL && pSharedWindow!=NULL)
        {
            Rectangle aBox = pWindow->GetWindowExtentsRelative(pSharedWindow);

            // Calculate offset of this canvas with respect to the shared
            // canvas.
            return awt::Point(aBox.Left(), aBox.Top());
        }
    }

    return awt::Point(0, 0);
}

::basegfx::B2DRectangle PresenterCanvas::GetClipRectangle (
    const css::geometry::AffineMatrix2D& rViewTransform,
    const awt::Point& rOffset)
{
    ::basegfx::B2DRectangle aClipRectangle;

    vcl::Window* pWindow = VCLUnoHelper::GetWindow(mxWindow);
    if (pWindow == NULL)
        return ::basegfx::B2DRectangle();

    vcl::Window* pSharedWindow = VCLUnoHelper::GetWindow(mxSharedWindow);
    if (pSharedWindow == NULL)
        return ::basegfx::B2DRectangle();

    // Get the bounding box of the window and create a range in the
    // coordinate system of the child window.
    Rectangle aLocalClip;
    if (maClipRectangle.Width <= 0 || maClipRectangle.Height <= 0)
    {
        // No clip rectangle has been set via SetClip by the pane.
        // Use the window extents instead.
        aLocalClip = pWindow->GetWindowExtentsRelative(pSharedWindow);
    }
    else
    {
        // Use a previously given clip rectangle.
        aLocalClip = Rectangle(
            maClipRectangle.X + rOffset.X,
            maClipRectangle.Y + rOffset.Y,
            maClipRectangle.X + maClipRectangle.Width + rOffset.X,
            maClipRectangle.Y + maClipRectangle.Height + rOffset.Y);
    }

    // The local clip rectangle is used to clip the view state clipping
    // polygon.
    ::basegfx::B2DRectangle aWindowRectangle (
        aLocalClip.Left() - rOffset.X,
        aLocalClip.Top() - rOffset.Y,
        aLocalClip.Right() - rOffset.X + 1,
        aLocalClip.Bottom() - rOffset.Y + 1);

    // Calculate the inverted view state transformation to cancel out a
    // later transformation of the local clip polygon with the view state
    // transformation.
    ::basegfx::B2DHomMatrix aInvertedViewStateTransformation;
    ::basegfx::unotools::homMatrixFromAffineMatrix(
        aInvertedViewStateTransformation,
        rViewTransform);
    if (aInvertedViewStateTransformation.invert())
    {
        // Cancel out the later multiplication with the view state
        // transformation.
        aWindowRectangle.transform(aInvertedViewStateTransformation);
    }

    return aWindowRectangle;
}

Reference<rendering::XPolyPolygon2D> PresenterCanvas::UpdateSpriteClip (
    const Reference<rendering::XPolyPolygon2D>& rxOriginalClip,
    const geometry::RealPoint2D& rLocation,
    const geometry::RealSize2D& rSize)
{
    (void)rSize;

    // Check used resources and just return the original clip when not
    // every one of them is available.
    if ( ! mxWindow.is())
        return rxOriginalClip;

    Reference<rendering::XGraphicDevice> xDevice (mxSharedCanvas->getDevice());
    if ( ! xDevice.is())
        return rxOriginalClip;

    // Determine the bounds of the clip rectangle (the window border) in the
    // coordinate system of the sprite.
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    const double nMinX (-rLocation.X);
    const double nMinY (-rLocation.Y);
    const double nMaxX (aWindowBox.Width-rLocation.X);
    const double nMaxY (aWindowBox.Height-rLocation.Y);

    // Create a clip polygon.
    Reference<rendering::XPolyPolygon2D> xPolygon;
    if (rxOriginalClip.is())
    {
        // Combine the original clip with the window clip.
        const ::basegfx::B2DPolyPolygon aOriginalClip (
            ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rxOriginalClip));
        ::basegfx::B2DRectangle aWindowRange (nMinX, nMinY, nMaxX, nMaxY);
        const ::basegfx::B2DPolyPolygon aClippedClipPolygon (
            ::basegfx::tools::clipPolyPolygonOnRange(
                aOriginalClip,
                aWindowRange,
                true, /* bInside */
                false /* bStroke */));
        xPolygon = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
            xDevice,
            aClippedClipPolygon);
    }
    else
    {
        // Create a new clip polygon from the window clip rectangle.
        Sequence<Sequence<geometry::RealPoint2D> > aPoints (1);
        aPoints[0] = Sequence<geometry::RealPoint2D>(4);
        aPoints[0][0] = geometry::RealPoint2D(nMinX,nMinY);
        aPoints[0][1] = geometry::RealPoint2D(nMaxX,nMinY);
        aPoints[0][2] = geometry::RealPoint2D(nMaxX,nMaxY);
        aPoints[0][3] = geometry::RealPoint2D(nMinX,nMaxY);
        Reference<rendering::XLinePolyPolygon2D> xLinePolygon(
            xDevice->createCompatibleLinePolyPolygon(aPoints));
        if (xLinePolygon.is())
            xLinePolygon->setClosed(0, sal_True);
        xPolygon.set(xLinePolygon, UNO_QUERY);
    }

    return xPolygon;
}

void PresenterCanvas::ThrowIfDisposed()
    throw (css::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose || ! mxSharedCanvas.is())
    {
        throw lang::DisposedException ("PresenterCanvas object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}

//===== PresenterCustomSprite =================================================

PresenterCustomSprite::PresenterCustomSprite (
    const rtl::Reference<PresenterCanvas>& rpCanvas,
    const Reference<rendering::XCustomSprite>& rxSprite,
    const Reference<awt::XWindow>& rxBaseWindow,
    const css::geometry::RealSize2D& rSpriteSize)
    : PresenterCustomSpriteInterfaceBase(m_aMutex),
      mpCanvas(rpCanvas),
      mxSprite(rxSprite),
      mxBaseWindow(rxBaseWindow),
      maPosition(0,0),
      maSpriteSize(rSpriteSize)
{
}

PresenterCustomSprite::~PresenterCustomSprite()
{
}

void SAL_CALL PresenterCustomSprite::disposing()
    throw (RuntimeException)
{
    Reference<XComponent> xComponent (mxSprite, UNO_QUERY);
    mxSprite = NULL;
    if (xComponent.is())
        xComponent->dispose();
    mpCanvas.clear();
}

//----- XSprite ---------------------------------------------------------------

void SAL_CALL PresenterCustomSprite::setAlpha (const double nAlpha)
    throw (lang::IllegalArgumentException,RuntimeException, std::exception)
{
    ThrowIfDisposed();
    mxSprite->setAlpha(nAlpha);
}

void SAL_CALL PresenterCustomSprite::move (
    const geometry::RealPoint2D& rNewPos,
    const rendering::ViewState& rViewState,
    const rendering::RenderState& rRenderState)
    throw (lang::IllegalArgumentException,RuntimeException, std::exception)
{
    ThrowIfDisposed();
    maPosition = rNewPos;
    mxSprite->move(
        rNewPos,
        mpCanvas->MergeViewState(rViewState, mpCanvas->GetOffset(mxBaseWindow)),
        rRenderState);
    // Clip sprite against window bounds.  This call is necessary because
    // sprite clipping is done in the corrdinate system of the sprite.
    // Therefore, after each change of the sprites location the window
    // bounds have to be transformed into the sprites coordinate system.
    clip(NULL);
}

void SAL_CALL PresenterCustomSprite::transform (const geometry::AffineMatrix2D& rTransformation)
    throw (lang::IllegalArgumentException,RuntimeException, std::exception)
{
    ThrowIfDisposed();
    mxSprite->transform(rTransformation);
}

void SAL_CALL PresenterCustomSprite::clip (const Reference<rendering::XPolyPolygon2D>& rxClip)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    // The clip region is expected in the coordinate system of the sprite.
    // UpdateSpriteClip() integrates the window bounds, transformed into the
    // sprites coordinate system, with the given clip.
    mxSprite->clip(mpCanvas->UpdateSpriteClip(rxClip, maPosition, maSpriteSize));
}

void SAL_CALL PresenterCustomSprite::setPriority (const double nPriority)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    mxSprite->setPriority(nPriority);
}

void SAL_CALL PresenterCustomSprite::show()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    mxSprite->show();
}

void SAL_CALL PresenterCustomSprite::hide()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    mxSprite->hide();
}

//----- XCustomSprite ---------------------------------------------------------

Reference<rendering::XCanvas> PresenterCustomSprite::getContentCanvas()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxSprite->getContentCanvas();
}

void PresenterCustomSprite::ThrowIfDisposed()
    throw (css::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose || ! mxSprite.is())
    {
        throw lang::DisposedException ("PresenterCustomSprite object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}

} } // end of namespace ::sd::presenter


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_Draw_PresenterCanvasFactory_get_implementation(css::uno::XComponentContext*,
                                                                 css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::presenter::PresenterCanvas());
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
