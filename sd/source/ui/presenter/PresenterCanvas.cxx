/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterCanvas.cxx,v $
 *
 * $Revision: 1.3 $
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

#include "precompiled_sd.hxx"

#include "PresenterCanvas.hxx"

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <canvas/canvastools.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <rtl/ref.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sd { namespace presenter {

//===== Service ===============================================================

Reference<XInterface> SAL_CALL PresenterCanvas_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    (void)rxContext;
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterCanvas()));
}




::rtl::OUString PresenterCanvas_getImplementationName (void) throw(RuntimeException)
{
    return OUString::createFromAscii("com.sun.star.comp.Draw.PresenterCanvasFactory");
}




Sequence<rtl::OUString> SAL_CALL PresenterCanvas_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.rendering.Canvas"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




//===== PresenterCustomSprite =================================================

/** Wrapper around a sprite that is displayed on a PresenterCanvas.
*/
namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
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
        const Reference<awt::XWindow>& rxBaseWindow);
    virtual ~PresenterCustomSprite (void);
    virtual void SAL_CALL disposing (void)
        throw (RuntimeException);

    // XSprite

    virtual void SAL_CALL setAlpha (double nAlpha)
        throw (lang::IllegalArgumentException,RuntimeException);

    virtual void SAL_CALL move (const geometry::RealPoint2D& rNewPos,
        const rendering::ViewState& rViewState,
        const rendering::RenderState& rRenderState)
        throw (lang::IllegalArgumentException,RuntimeException);

    virtual void SAL_CALL transform (const geometry::AffineMatrix2D& rTransformation)
        throw (lang::IllegalArgumentException,RuntimeException);

    virtual void SAL_CALL clip (const Reference<rendering::XPolyPolygon2D>& rClip)
        throw (RuntimeException);

    virtual void SAL_CALL setPriority (double nPriority)
        throw (RuntimeException);

    virtual void SAL_CALL show (void)
        throw (RuntimeException);

    virtual void SAL_CALL hide (void)
        throw (RuntimeException);


    // XCustomSprite

    virtual Reference<rendering::XCanvas> SAL_CALL getContentCanvas (void)
        throw (RuntimeException);

private:
    rtl::Reference<PresenterCanvas> mpCanvas;
    Reference<rendering::XCustomSprite> mxSprite;
    Reference<awt::XWindow> mxBaseWindow;
    void ThrowIfDisposed (void)
        throw (css::lang::DisposedException);
};




//===== PresenterCanvas =======================================================


PresenterCanvas::PresenterCanvas (void)
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




PresenterCanvas::~PresenterCanvas (void)
{
}




void SAL_CALL PresenterCanvas::disposing (void)
    throw (css::uno::RuntimeException)
{
    if (mxWindow.is())
        mxWindow->removeWindowListener(this);
}




//----- XInitialization -------------------------------------------------------

void SAL_CALL PresenterCanvas::initialize (
    const Sequence<Any>& rArguments)
    throw(Exception, RuntimeException)
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
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterCanvas: invalid shared window"),
                    static_cast<XWeak*>(this),
                    1);
            }

            if ( ! (rArguments[3] >>= mxSharedCanvas))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterCanvas: invalid shared canvas"),
                    static_cast<XWeak*>(this),
                    2);
            }

            if ( ! (rArguments[4] >>= mxWindow))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterCanvas: invalid window"),
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
        throw RuntimeException(
            OUString::createFromAscii("PresenterCanvas: invalid number of arguments"),
                static_cast<XWeak*>(this));
    }
}




//----- XCanvas ---------------------------------------------------------------

void SAL_CALL PresenterCanvas::clear (void)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    // ToDo: Clear the area covered by the child window.  A simple forward
    // would clear the whole shared canvas.
}




void SAL_CALL PresenterCanvas::drawPoint (
    const css::geometry::RealPoint2D& aPoint,
    const css::rendering::ViewState& aViewState,
    const css::rendering::RenderState& aRenderState)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    ThrowIfDisposed();
    mxSharedCanvas->drawPoint(aPoint,MergeViewState(aViewState),aRenderState);
}




void SAL_CALL PresenterCanvas::drawLine (
        const css::geometry::RealPoint2D& aStartPoint,
        const css::geometry::RealPoint2D& aEndPoint,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    ThrowIfDisposed();
    mxSharedCanvas->drawLine(aStartPoint,aEndPoint,MergeViewState(aViewState),aRenderState);
}




void SAL_CALL PresenterCanvas::drawBezier (
        const css::geometry::RealBezierSegment2D& aBezierSegment,
        const css::geometry::RealPoint2D& aEndPoint,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    ThrowIfDisposed();
    mxSharedCanvas->drawBezier(aBezierSegment,aEndPoint,MergeViewState(aViewState),aRenderState);
}




css::uno::Reference<css::rendering::XCachedPrimitive> SAL_CALL PresenterCanvas::drawPolyPolygon (
        const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
        const css::rendering::ViewState& aViewState,
        const css::rendering::RenderState& aRenderState)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
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
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
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
            css::uno::RuntimeException)
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
            css::uno::RuntimeException)
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
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
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
            css::uno::RuntimeException)
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
            css::uno::RuntimeException)
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
            css::uno::RuntimeException)
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
            css::uno::RuntimeException)
{
    ThrowIfDisposed();
    return mxSharedCanvas->createFont(
        aFontRequest, aExtraFontProperties, aFontMatrix);
}




css::uno::Sequence<css::rendering::FontInfo> SAL_CALL
        PresenterCanvas::queryAvailableFonts(
            const css::rendering::FontInfo& aFilter,
            const css::uno::Sequence< css::beans::PropertyValue >& aFontProperties)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
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
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
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
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
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
            css::uno::RuntimeException)
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
            css::uno::RuntimeException)
{
    ThrowIfDisposed();
    return mxSharedCanvas->drawBitmapModulated(
        xBitmap, MergeViewState(aViewState), aRenderState);
}




css::uno::Reference<css::rendering::XGraphicDevice> SAL_CALL
        PresenterCanvas::getDevice (void)
        throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    return mxSharedCanvas->getDevice();
}




//----- XBitmapCanvas ---------------------------------------------------------

void SAL_CALL PresenterCanvas::copyRect(
    const css::uno::Reference<css::rendering::XBitmapCanvas>& rxSourceCanvas,
    const css::geometry::RealRectangle2D& rSourceRect,
    const css::rendering::ViewState& rSourceViewState,
    const css::rendering::RenderState& rSourceRenderState,
    const css::geometry::RealRectangle2D& rDestRect,
    const css::rendering::ViewState& rDestViewState,
    const css::rendering::RenderState& rDestRenderState)
    throw (css::lang::IllegalArgumentException,
        css::rendering::VolatileContentDestroyedException,
        css::uno::RuntimeException)
{
    ThrowIfDisposed();

    Reference<rendering::XBitmapCanvas> xBitmapCanvas (mxSharedCanvas, UNO_QUERY);
    if (xBitmapCanvas.is())
    {
        rendering::ViewState aSourceViewState (rSourceViewState);
        if (rxSourceCanvas == Reference<rendering::XCanvas>(this))
            aSourceViewState = MergeViewState(aSourceViewState);
        xBitmapCanvas->copyRect(
            rxSourceCanvas, rSourceRect, aSourceViewState, rSourceRenderState,
            rDestRect, MergeViewState(rDestViewState), rDestRenderState);
    }
}




//----- XSpriteCanvas ---------------------------------------------------------

Reference<rendering::XAnimatedSprite> SAL_CALL
    PresenterCanvas::createSpriteFromAnimation (
        const css::uno::Reference<css::rendering::XAnimation>& rAnimation)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
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
            css::uno::RuntimeException)
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
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    ThrowIfDisposed();

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxSharedCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        return new PresenterCustomSprite(
            this,
            xSpriteCanvas->createCustomSprite(rSpriteSize),
            mxSharedWindow);

    if (mxUpdateCanvas.is())
        return new PresenterCustomSprite(
            this,
            mxUpdateCanvas->createCustomSprite(rSpriteSize),
            mxUpdateWindow);

    return NULL;
}




Reference<rendering::XSprite> SAL_CALL
    PresenterCanvas::createClonedSprite (
        const css::uno::Reference< css::rendering::XSprite >& rxOriginal)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    ThrowIfDisposed();

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxSharedCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        return xSpriteCanvas->createClonedSprite(rxOriginal);
    if (mxUpdateCanvas.is())
        return mxUpdateCanvas->createClonedSprite(rxOriginal);
    return NULL;
}




::sal_Bool SAL_CALL PresenterCanvas::updateScreen (::sal_Bool bUpdateAll)
    throw (css::uno::RuntimeException)
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
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    if (rEvent.Source == mxWindow)
        mxWindow = NULL;
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterCanvas::windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    mbOffsetUpdatePending = true;
}




void SAL_CALL PresenterCanvas::windowMoved (const css::awt::WindowEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    mbOffsetUpdatePending = true;
}




void SAL_CALL PresenterCanvas::windowShown (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    mbOffsetUpdatePending = true;
}




void SAL_CALL PresenterCanvas::windowHidden (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}




//----- XBitmap ---------------------------------------------------------------

geometry::IntegerSize2D SAL_CALL PresenterCanvas::getSize (void)
    throw (RuntimeException)
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




sal_Bool SAL_CALL PresenterCanvas::hasAlpha (void)
    throw (RuntimeException)
{
    Reference<rendering::XBitmap> xBitmap (mxSharedCanvas, UNO_QUERY);
    if (xBitmap.is())
        return xBitmap->hasAlpha();
    else
        return sal_False;
}




Reference<rendering::XBitmapCanvas> SAL_CALL PresenterCanvas::queryBitmapCanvas (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    return this;
}




Reference<rendering::XBitmap> SAL_CALL PresenterCanvas::getScaledBitmap(
    const css::geometry::RealSize2D& rNewSize,
    sal_Bool bFast)
    throw (css::uno::RuntimeException,
        css::lang::IllegalArgumentException,
        css::rendering::VolatileContentDestroyedException)
{
    (void)rNewSize;
    (void)bFast;

    ThrowIfDisposed();

    // Not implemented.

    return NULL;
}




//-----------------------------------------------------------------------------

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
            ::canvas::tools::polyPolygonFromXPolyPolygon2D(
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




Reference<rendering::XCanvas> PresenterCanvas::GetSharedCanvas (void) const
{
    return mxSharedCanvas;
}




void PresenterCanvas::SetClip (const awt::Rectangle& rClipRectangle)
{
    maClipRectangle = rClipRectangle;
}




awt::Point PresenterCanvas::GetOffset (const Reference<awt::XWindow>& rxBaseWindow)
{
    mbOffsetUpdatePending = false;
    if (mxWindow.is() && rxBaseWindow.is())
    {
        ::Window* pWindow = VCLUnoHelper::GetWindow(mxWindow);
        ::Window* pSharedWindow = VCLUnoHelper::GetWindow(rxBaseWindow);
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

    ::Window* pWindow = VCLUnoHelper::GetWindow(mxWindow);
    if (pWindow == NULL)
        return ::basegfx::B2DRectangle();

    ::Window* pSharedWindow = VCLUnoHelper::GetWindow(mxSharedWindow);
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




void PresenterCanvas::ThrowIfDisposed (void)
    throw (css::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose || ! mxSharedCanvas.is())
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterCanvas object has already been disposed")),
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
      mxBaseWindow(rxBaseWindow)
{
}




PresenterCustomSprite::~PresenterCustomSprite (void)
{
}




void SAL_CALL PresenterCustomSprite::disposing (void)
    throw (RuntimeException)
{
    Reference<XComponent> xComponent (mxSprite, UNO_QUERY);
    mxSprite = NULL;
    if (xComponent.is())
        xComponent->dispose();
    mpCanvas = rtl::Reference<PresenterCanvas>();
}




//----- XSprite ---------------------------------------------------------------

void SAL_CALL PresenterCustomSprite::setAlpha (const double nAlpha)
    throw (lang::IllegalArgumentException,RuntimeException)
{
    ThrowIfDisposed();
    mxSprite->setAlpha(nAlpha);
}




void SAL_CALL PresenterCustomSprite::move (const geometry::RealPoint2D& rNewPos,
    const rendering::ViewState& rViewState,
    const rendering::RenderState& rRenderState)
    throw (lang::IllegalArgumentException,RuntimeException)
{
    ThrowIfDisposed();
    mxSprite->move(
        rNewPos,
        mpCanvas->MergeViewState(rViewState, mpCanvas->GetOffset(mxBaseWindow)),
        rRenderState);
}




void SAL_CALL PresenterCustomSprite::transform (const geometry::AffineMatrix2D& rTransformation)
    throw (lang::IllegalArgumentException,RuntimeException)
{
    ThrowIfDisposed();
    mxSprite->transform(rTransformation);
}




void SAL_CALL PresenterCustomSprite::clip (const Reference<rendering::XPolyPolygon2D>& rxClip)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    mxSprite->clip(rxClip);
}




void SAL_CALL PresenterCustomSprite::setPriority (const double nPriority)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    mxSprite->setPriority(nPriority);
}



void SAL_CALL PresenterCustomSprite::show (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    mxSprite->show();
}




void SAL_CALL PresenterCustomSprite::hide (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    mxSprite->hide();
}




//----- XCustomSprite ---------------------------------------------------------

Reference<rendering::XCanvas> PresenterCustomSprite::getContentCanvas (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return mxSprite->getContentCanvas();
}




//-----------------------------------------------------------------------------

void PresenterCustomSprite::ThrowIfDisposed (void)
    throw (css::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose || ! mxSprite.is())
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterCustomSprite object has already been disposed")),
            static_cast<uno::XWeak*>(this));
    }
}



} } // end of namespace ::sd::presenter
