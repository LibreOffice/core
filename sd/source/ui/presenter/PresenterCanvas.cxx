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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <canvas/canvastools.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>

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
    : protected ::cppu::BaseMutex,
      public PresenterCustomSpriteInterfaceBase
{
public:
    PresenterCustomSprite (
        const rtl::Reference<PresenterCanvas>& rpCanvas,
        const Reference<rendering::XCustomSprite>& rxSprite,
        const Reference<awt::XWindow>& rxBaseWindow);
    PresenterCustomSprite(const PresenterCustomSprite&) = delete;
    PresenterCustomSprite& operator=(const PresenterCustomSprite&) = delete;
    virtual void SAL_CALL disposing() override;

    // XSprite

    virtual void SAL_CALL setAlpha (double nAlpha) override;

    virtual void SAL_CALL move (const geometry::RealPoint2D& rNewPos,
        const rendering::ViewState& rViewState,
        const rendering::RenderState& rRenderState) override;

    virtual void SAL_CALL transform (const geometry::AffineMatrix2D& rTransformation) override;

    virtual void SAL_CALL clip (const Reference<rendering::XPolyPolygon2D>& rClip) override;

    virtual void SAL_CALL setPriority (double nPriority) override;

    virtual void SAL_CALL show() override;

    virtual void SAL_CALL hide() override;

    // XCustomSprite

    virtual Reference<rendering::XCanvas> SAL_CALL getContentCanvas() override;

private:
    rtl::Reference<PresenterCanvas> mpCanvas;
    Reference<rendering::XCustomSprite> mxSprite;
    Reference<awt::XWindow> mxBaseWindow;
    geometry::RealPoint2D maPosition;

    /// @throws css::lang::DisposedException
    void ThrowIfDisposed();
};

//===== PresenterCanvas =======================================================

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
      mbOffsetUpdatePending(true)
{
    if (mxWindow.is())
        mxWindow->addWindowListener(this);

    if (mxUpdateCanvas.is())
    {
        m_pUpdateRequester = CanvasUpdateRequester::Instance(mxUpdateCanvas);
    }
}

PresenterCanvas::~PresenterCanvas()
{
}

void SAL_CALL PresenterCanvas::disposing()
{
    if (mxWindow.is())
        mxWindow->removeWindowListener(this);
}

//----- XCanvas ---------------------------------------------------------------

void SAL_CALL PresenterCanvas::clear()
{
    ThrowIfDisposed();
    // ToDo: Clear the area covered by the child window.  A simple forward
    // would clear the whole shared canvas.
}

void SAL_CALL PresenterCanvas::drawPoint (
    const css::geometry::RealPoint2D& aPoint,
    const css::rendering::ViewState& aViewState,
    const css::rendering::RenderState& aRenderState)
{
    ThrowIfDisposed();
    mxSharedCanvas->drawPoint(aPoint,MergeViewState(aViewState),aRenderState);
}

void SAL_CALL PresenterCanvas::drawLine (
        const css::geometry::RealPoint2D& aStartPoint,
        const css::geometry::RealPoint2D& aEndPoint,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
{
    ThrowIfDisposed();
    mxSharedCanvas->drawLine(aStartPoint,aEndPoint,MergeViewState(aViewState),aRenderState);
}

void SAL_CALL PresenterCanvas::drawBezier (
        const css::geometry::RealBezierSegment2D& aBezierSegment,
        const css::geometry::RealPoint2D& aEndPoint,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
{
    ThrowIfDisposed();
    mxSharedCanvas->drawBezier(aBezierSegment,aEndPoint,MergeViewState(aViewState),aRenderState);
}

css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL PresenterCanvas::drawPolyPolygon (
        const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
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
{
    ThrowIfDisposed();
    return mxSharedCanvas->createFont(
        aFontRequest, aExtraFontProperties, aFontMatrix);
}

css::uno::Sequence<css::rendering::FontInfo> SAL_CALL
        PresenterCanvas::queryAvailableFonts(
            const css::rendering::FontInfo& aFilter,
            const css::uno::Sequence< css::beans::PropertyValue >& aFontProperties)
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
{
    ThrowIfDisposed();
    return mxSharedCanvas->drawBitmapModulated(
        xBitmap, MergeViewState(aViewState), aRenderState);
}

css::uno::Reference<css::rendering::XGraphicDevice> SAL_CALL
        PresenterCanvas::getDevice()
{
    ThrowIfDisposed();
    return mxSharedCanvas->getDevice();
}

//----- XSpriteCanvas ---------------------------------------------------------

Reference<rendering::XAnimatedSprite> SAL_CALL
    PresenterCanvas::createSpriteFromAnimation (
        const css::uno::Reference<css::rendering::XAnimation>& rAnimation)
{
    ThrowIfDisposed();

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxSharedCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        return xSpriteCanvas->createSpriteFromAnimation(rAnimation);
    else
        return nullptr;
}

Reference<rendering::XAnimatedSprite> SAL_CALL
    PresenterCanvas::createSpriteFromBitmaps (
        const css::uno::Sequence<
            css::uno::Reference< css::rendering::XBitmap > >& rAnimationBitmaps,
    ::sal_Int8 nInterpolationMode)
{
    ThrowIfDisposed();

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxSharedCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        return xSpriteCanvas->createSpriteFromBitmaps(rAnimationBitmaps, nInterpolationMode);
    else
        return nullptr;
}

Reference<rendering::XCustomSprite> SAL_CALL
    PresenterCanvas::createCustomSprite (
        const css::geometry::RealSize2D& rSpriteSize)
{
    ThrowIfDisposed();

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxSharedCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        return new PresenterCustomSprite(
            this,
            xSpriteCanvas->createCustomSprite(rSpriteSize),
            mxSharedWindow);
    else if (mxUpdateCanvas.is())
        return new PresenterCustomSprite(
            this,
            mxUpdateCanvas->createCustomSprite(rSpriteSize),
            mxUpdateWindow);
    else
        return nullptr;
}

Reference<rendering::XSprite> SAL_CALL
    PresenterCanvas::createClonedSprite (
        const css::uno::Reference< css::rendering::XSprite >& rxOriginal)
{
    ThrowIfDisposed();

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxSharedCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        return xSpriteCanvas->createClonedSprite(rxOriginal);
    if (mxUpdateCanvas.is())
        return mxUpdateCanvas->createClonedSprite(rxOriginal);
    return nullptr;
}

sal_Bool SAL_CALL PresenterCanvas::updateScreen (sal_Bool bUpdateAll)
{
    ThrowIfDisposed();

    mbOffsetUpdatePending = true;
    if (m_pUpdateRequester.get() != nullptr)
    {
        m_pUpdateRequester->RequestUpdate(bUpdateAll);
        return true;
    }
    else
    {
        return false;
    }
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterCanvas::disposing (const css::lang::EventObject& rEvent)
{
    ThrowIfDisposed();
    if (rEvent.Source == mxWindow)
        mxWindow = nullptr;
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterCanvas::windowResized (const css::awt::WindowEvent&)
{
    ThrowIfDisposed();
    mbOffsetUpdatePending = true;
}

void SAL_CALL PresenterCanvas::windowMoved (const css::awt::WindowEvent&)
{
    ThrowIfDisposed();
    mbOffsetUpdatePending = true;
}

void SAL_CALL PresenterCanvas::windowShown (const css::lang::EventObject&)
{
    ThrowIfDisposed();
    mbOffsetUpdatePending = true;
}

void SAL_CALL PresenterCanvas::windowHidden (const css::lang::EventObject&)
{
    ThrowIfDisposed();
}

//----- XBitmap ---------------------------------------------------------------

geometry::IntegerSize2D SAL_CALL PresenterCanvas::getSize()
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
{
    Reference<rendering::XBitmap> xBitmap (mxSharedCanvas, UNO_QUERY);
    if (xBitmap.is())
        return xBitmap->hasAlpha();
    else
        return false;
}

Reference<rendering::XBitmap> SAL_CALL PresenterCanvas::getScaledBitmap(
    const css::geometry::RealSize2D&,
    sal_Bool)
{
    ThrowIfDisposed();

    // Not implemented.

    return nullptr;
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
            ::basegfx::B2DPolyPolygon(::basegfx::utils::createPolygonFromRect(aWindowRange)));
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
            ::basegfx::utils::clipPolyPolygonOnRange(
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
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(mxWindow);
        VclPtr<vcl::Window> pSharedWindow = VCLUnoHelper::GetWindow(rxBaseWindow);
        if (pWindow && pSharedWindow)
        {
            ::tools::Rectangle aBox = pWindow->GetWindowExtentsRelative(pSharedWindow);

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
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(mxWindow);
    if (!pWindow)
        return ::basegfx::B2DRectangle();

    VclPtr<vcl::Window> pSharedWindow = VCLUnoHelper::GetWindow(mxSharedWindow);
    if (!pSharedWindow)
        return ::basegfx::B2DRectangle();

    // Get the bounding box of the window and create a range in the
    // coordinate system of the child window.
    // Use the window extents.
    ::tools::Rectangle aLocalClip = pWindow->GetWindowExtentsRelative(pSharedWindow);

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
    const geometry::RealPoint2D& rLocation)
{
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
            ::basegfx::utils::clipPolyPolygonOnRange(
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
            xLinePolygon->setClosed(0, true);
        xPolygon.set(xLinePolygon, UNO_QUERY);
    }

    return xPolygon;
}

void PresenterCanvas::ThrowIfDisposed()
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
    const Reference<awt::XWindow>& rxBaseWindow)
    : PresenterCustomSpriteInterfaceBase(m_aMutex),
      mpCanvas(rpCanvas),
      mxSprite(rxSprite),
      mxBaseWindow(rxBaseWindow),
      maPosition(0,0)
{
}

void SAL_CALL PresenterCustomSprite::disposing()
{
    Reference<XComponent> xComponent (mxSprite, UNO_QUERY);
    mxSprite = nullptr;
    if (xComponent.is())
        xComponent->dispose();
    mpCanvas.clear();
}

//----- XSprite ---------------------------------------------------------------

void SAL_CALL PresenterCustomSprite::setAlpha (const double nAlpha)
{
    ThrowIfDisposed();
    mxSprite->setAlpha(nAlpha);
}

void SAL_CALL PresenterCustomSprite::move (
    const geometry::RealPoint2D& rNewPos,
    const rendering::ViewState& rViewState,
    const rendering::RenderState& rRenderState)
{
    ThrowIfDisposed();
    maPosition = rNewPos;
    mxSprite->move(
        rNewPos,
        mpCanvas->MergeViewState(rViewState, mpCanvas->GetOffset(mxBaseWindow)),
        rRenderState);
    // Clip sprite against window bounds.  This call is necessary because
    // sprite clipping is done in the coordinate system of the sprite.
    // Therefore, after each change of the sprites location the window
    // bounds have to be transformed into the sprites coordinate system.
    clip(nullptr);
}

void SAL_CALL PresenterCustomSprite::transform (const geometry::AffineMatrix2D& rTransformation)
{
    ThrowIfDisposed();
    mxSprite->transform(rTransformation);
}

void SAL_CALL PresenterCustomSprite::clip (const Reference<rendering::XPolyPolygon2D>& rxClip)
{
    ThrowIfDisposed();
    // The clip region is expected in the coordinate system of the sprite.
    // UpdateSpriteClip() integrates the window bounds, transformed into the
    // sprites coordinate system, with the given clip.
    mxSprite->clip(mpCanvas->UpdateSpriteClip(rxClip, maPosition));
}

void SAL_CALL PresenterCustomSprite::setPriority (const double nPriority)
{
    ThrowIfDisposed();
    mxSprite->setPriority(nPriority);
}

void SAL_CALL PresenterCustomSprite::show()
{
    ThrowIfDisposed();
    mxSprite->show();
}

void SAL_CALL PresenterCustomSprite::hide()
{
    ThrowIfDisposed();
    mxSprite->hide();
}

//----- XCustomSprite ---------------------------------------------------------

Reference<rendering::XCanvas> PresenterCustomSprite::getContentCanvas()
{
    ThrowIfDisposed();
    return mxSprite->getContentCanvas();
}

void PresenterCustomSprite::ThrowIfDisposed()
{
    if (rBHelper.bDisposed || rBHelper.bInDispose || ! mxSprite.is())
    {
        throw lang::DisposedException ("PresenterCustomSprite object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}

} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
