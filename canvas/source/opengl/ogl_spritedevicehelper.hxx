/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/opengl/OpenGLContext.hxx>

#include <rtl/ref.hxx>
#include <canvas/elapsedtime.hxx>
#include <com/sun/star/rendering/XGraphicDevice.hpp>

#include "ogl_buffercontext.hxx"

#include <set>

namespace vcl { class Window; }
class SystemChildWindow;
namespace basegfx{ class B2IVector; class B2DHomMatrix; }
namespace com::sun::star::awt { struct Rectangle; }
namespace com::sun::star::geometry { struct AffineMatrix2D; }


namespace oglcanvas
{
    class TextureCache;
    class SpriteCanvas;
    class CanvasCustomSprite;
    class CanvasHelper;

    class SpriteDeviceHelper
    {
    public:
        SpriteDeviceHelper();
        ~SpriteDeviceHelper();

        /// make noncopyable
        SpriteDeviceHelper(const SpriteDeviceHelper&) = delete;
        const SpriteDeviceHelper& operator=(const SpriteDeviceHelper&) = delete;

        void init( vcl::Window&                            rWindow,
                   SpriteCanvas&                           rSpriteCanvas,
                   const css::awt::Rectangle&              rViewArea );

        /// Dispose all internal references
        void disposing();

        // XWindowGraphicDevice
        css::geometry::RealSize2D getPhysicalResolution();
        css::geometry::RealSize2D getPhysicalSize();
        css::uno::Reference< css::rendering::XLinePolyPolygon2D > createCompatibleLinePolyPolygon(
            const css::uno::Reference< css::rendering::XGraphicDevice >&                               rDevice,
            const css::uno::Sequence< css::uno::Sequence< css::geometry::RealPoint2D > >& points );
        css::uno::Reference< css::rendering::XBezierPolyPolygon2D > createCompatibleBezierPolyPolygon(
            const css::uno::Reference< css::rendering::XGraphicDevice >&                                       rDevice,
            const css::uno::Sequence< css::uno::Sequence< css::geometry::RealBezierSegment2D > >& points );
        css::uno::Reference< css::rendering::XBitmap > createCompatibleBitmap(
            const css::uno::Reference< css::rendering::XGraphicDevice >&  rDevice,
            const css::geometry::IntegerSize2D&                                        size );
        css::uno::Reference< css::rendering::XVolatileBitmap > createVolatileBitmap(
            const css::uno::Reference< css::rendering::XGraphicDevice >&  rDevice,
            const css::geometry::IntegerSize2D&                                        size );
        css::uno::Reference< css::rendering::XBitmap > createCompatibleAlphaBitmap(
            const css::uno::Reference< css::rendering::XGraphicDevice >&  rDevice,
            const css::geometry::IntegerSize2D&                                        size );
        css::uno::Reference< css::rendering::XVolatileBitmap > createVolatileAlphaBitmap(
            const css::uno::Reference< css::rendering::XGraphicDevice >&  rDevice,
            const css::geometry::IntegerSize2D&                                        size );

        bool  showBuffer( bool bIsVisible, bool bUpdateAll );
        bool  switchBuffer( bool bIsVisible, bool bUpdateAll );

        css::uno::Any isAccelerated() const;
        css::uno::Any getDeviceHandle() const;
        css::uno::Any getSurfaceHandle() const;
        css::uno::Reference<
            css::rendering::XColorSpace > getColorSpace() const;

        void notifySizeUpdate( const css::awt::Rectangle& rBounds );

        /** called when DumpScreenContent property is enabled on
            XGraphicDevice, and writes out bitmaps of current screen.
         */
        void dumpScreenContent() const;

        void show( const ::rtl::Reference< CanvasCustomSprite >& );
        void hide( const ::rtl::Reference< CanvasCustomSprite >& );

        /// enable linear gradient shader "texture" with given parameters
        void useLinearGradientShader( const css::rendering::ARGBColor*    pColors,
                                      const css::uno::Sequence< double >& rStops,
                                      const ::basegfx::B2DHomMatrix&                   rTexTransform );
        /// enable radial gradient shader "texture" with given parameters
        void useRadialGradientShader( const css::rendering::ARGBColor*    pColors,
                                      const css::uno::Sequence< double >& rStops,
                                      const ::basegfx::B2DHomMatrix&                   rTexTransform );
        /// enable rectangular gradient shader "texture" with given parameters
        void useRectangularGradientShader( const css::rendering::ARGBColor*    pColors,
                                           const css::uno::Sequence< double >& rStops,
                                           const ::basegfx::B2DHomMatrix&                   rTexTransform );

        /// create a pbuffer context (for rendering into background surface)
        IBufferContextSharedPtr createBufferContext(const ::basegfx::B2IVector& rSize) const;

        /// Get instance of internal texture cache
        TextureCache& getTextureCache() const;

    private:
        /// Pointer to sprite canvas (owner of this helper), needed to create bitmaps
        SpriteCanvas*                                      mpSpriteCanvas;

        std::set< ::rtl::Reference< CanvasCustomSprite > > maActiveSprites;

        /// For the frame counter timings
        ::canvas::tools::ElapsedTime                       maLastUpdate;

        std::shared_ptr<TextureCache>                    mpTextureCache;

        unsigned int                                       mnLinearTwoColorGradientProgram;
        unsigned int                                       mnLinearMultiColorGradientProgram;
        unsigned int                                       mnRadialTwoColorGradientProgram;
        unsigned int                                       mnRadialMultiColorGradientProgram;
        unsigned int                                       mnRectangularTwoColorGradientProgram;
        unsigned int                                       mnRectangularMultiColorGradientProgram;

        rtl::Reference<OpenGLContext>                      mxContext;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
