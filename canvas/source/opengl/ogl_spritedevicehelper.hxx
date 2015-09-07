/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CANVAS_SOURCE_OPENGL_OGL_SPRITEDEVICEHELPER_HXX
#define INCLUDED_CANVAS_SOURCE_OPENGL_OGL_SPRITEDEVICEHELPER_HXX

#include <vcl/opengl/OpenGLContext.hxx>

#include <rtl/ref.hxx>
#include <canvas/elapsedtime.hxx>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "ogl_buffercontext.hxx"

#include <set>

namespace vcl { class Window; }
class SystemChildWindow;
namespace basegfx{ class B2IVector; class B2DHomMatrix; }
namespace com { namespace sun { namespace star {
    namespace awt { struct Rectangle; }
    namespace geometry { struct AffineMatrix2D; }
}}}

namespace oglcanvas
{
    class TextureCache;
    class SpriteCanvas;
    class CanvasCustomSprite;
    class CanvasHelper;

    class SpriteDeviceHelper : private ::boost::noncopyable
    {
    public:
        SpriteDeviceHelper();
        ~SpriteDeviceHelper();

        void init( vcl::Window&                                 rWindow,
                   SpriteCanvas&                           rSpriteCanvas,
                   const ::com::sun::star::awt::Rectangle& rViewArea );

        /// Dispose all internal references
        void disposing();

        // XWindowGraphicDevice
        ::com::sun::star::geometry::RealSize2D getPhysicalResolution();
        ::com::sun::star::geometry::RealSize2D getPhysicalSize();
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XLinePolyPolygon2D > createCompatibleLinePolyPolygon(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&                               rDevice,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > >& points );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBezierPolyPolygon2D > createCompatibleBezierPolyPolygon(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&                                       rDevice,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > >& points );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > createCompatibleBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XVolatileBitmap > createVolatileBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > createCompatibleAlphaBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XVolatileBitmap > createVolatileAlphaBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );

        bool  showBuffer( bool bIsVisible, bool bUpdateAll );
        bool  switchBuffer( bool bIsVisible, bool bUpdateAll );

        ::com::sun::star::uno::Any isAccelerated() const;
        ::com::sun::star::uno::Any getDeviceHandle() const;
        ::com::sun::star::uno::Any getSurfaceHandle() const;
        ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XColorSpace > getColorSpace() const;

        void notifySizeUpdate( const ::com::sun::star::awt::Rectangle& rBounds );

        /** called when DumpScreenContent property is enabled on
            XGraphicDevice, and writes out bitmaps of current screen.
         */
        void dumpScreenContent() const;

        void show( const ::rtl::Reference< CanvasCustomSprite >& );
        void hide( const ::rtl::Reference< CanvasCustomSprite >& );

        /// enable linear gradient shader "texture" with given parameters
        void useLinearGradientShader( const ::com::sun::star::rendering::ARGBColor*    pColors,
                                      const ::com::sun::star::uno::Sequence< double >& rStops,
                                      const ::basegfx::B2DHomMatrix&                   rTexTransform );
        /// enable radial gradient shader "texture" with given parameters
        void useRadialGradientShader( const ::com::sun::star::rendering::ARGBColor*    pColors,
                                      const ::com::sun::star::uno::Sequence< double >& rStops,
                                      const ::basegfx::B2DHomMatrix&                   rTexTransform );
        /// enable rectangular gradient shader "texture" with given parameters
        void useRectangularGradientShader( const ::com::sun::star::rendering::ARGBColor*    pColors,
                                           const ::com::sun::star::uno::Sequence< double >& rStops,
                                           const ::basegfx::B2DHomMatrix&                   rTexTransform );

        /// create a pbuffer context (for rendering into background surface)
        IBufferContextSharedPtr createBufferContext(const ::basegfx::B2IVector& rSize) const;

        /// Get instance of internal texture cache
        TextureCache& getTextureCache() const;

        bool activateWindowContext();

    private:
        void resize( const ::basegfx::B2IVector& rNewSize );

        /** Phyical output device

            Deliberately not a refcounted reference, because of
            potential circular references for canvas. Needed to
            create bitmaps
         */
        com::sun::star::rendering::XGraphicDevice*         mpDevice;

        /// Pointer to sprite canvas (owner of this helper), needed to create bitmaps
        SpriteCanvas*                                      mpSpriteCanvas;

        std::set< ::rtl::Reference< CanvasCustomSprite > > maActiveSprites;

        /// For the frame counter timings
        ::canvas::tools::ElapsedTime                       maLastUpdate;

        boost::shared_ptr<TextureCache>                    mpTextureCache;

        unsigned int                                       mnLinearTwoColorGradientProgram;
        unsigned int                                       mnLinearMultiColorGradientProgram;
        unsigned int                                       mnRadialTwoColorGradientProgram;
        unsigned int                                       mnRadialMultiColorGradientProgram;
        unsigned int                                       mnRectangularTwoColorGradientProgram;
        unsigned int                                       mnRectangularMultiColorGradientProgram;

        rtl::Reference<OpenGLContext>                      mxContext;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
