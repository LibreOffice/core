/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_CANVAS_IMAGE_HXX
#define INCLUDED_CANVAS_IMAGE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <canvas/rendering/icolorbuffer.hxx>
#include <canvas/parametricpolypolygon.hxx>
#include "imagecachedprimitive.hxx"

#include <canvas/elapsedtime.hxx>

#include "image_sysprereq.h"

struct BitmapSystemData;
class  BitmapEx;

namespace canvas
{
    class Image : public IColorBuffer
    {
    public:
        /// The description of the image
        struct Description
        {
            IColorBuffer::Format eFormat;
            sal_uInt32           nWidth;
            sal_uInt32           nHeight;
            sal_uInt32           nStride;
            sal_uInt8*           pBuffer;
        };

        /** Create a new image with the attributes passed as argument.
         */
        explicit Image( const Description& desc );

        /** Create a new image from the XBitmap passed as argument
         */
        explicit Image( const ::com::sun::star::uno::Reference<
                              ::com::sun::star::rendering::XBitmap >& xBitmap );


        virtual ~Image();

        /** Retrieve desciption of image layout
         */
        const Description& getDescription() const { return maDesc; }

        /** Clear image with uniform color
         */
        void               clear( sal_uInt8 a,
                                  sal_uInt8 r,
                                  sal_uInt8 g,
                                  sal_uInt8 b );

        void fillB2DPolyPolygon(
            const ::basegfx::B2DPolyPolygon&                    rPolyPolygon,
            const ::com::sun::star::rendering::ViewState&       viewState,
            const ::com::sun::star::rendering::RenderState&     renderState );


        // IColorBuffer interface implementation
        // =====================================

        virtual sal_uInt8* lock() const;
        virtual void unlock() const;
        virtual sal_uInt32 getWidth() const;
        virtual sal_uInt32 getHeight() const;
        virtual sal_uInt32 getStride() const;
        virtual Format     getFormat() const;


        // High-level drawing operations (from the XCanvas interface)
        // ==========================================================

        void drawPoint( const ::com::sun::star::geometry::RealPoint2D&      aPoint,
                        const ::com::sun::star::rendering::ViewState&       viewState,
                        const ::com::sun::star::rendering::RenderState&     renderState );
        void drawLine( const ::com::sun::star::geometry::RealPoint2D&       aStartPoint,
                       const ::com::sun::star::geometry::RealPoint2D&       aEndPoint,
                       const ::com::sun::star::rendering::ViewState&        viewState,
                       const ::com::sun::star::rendering::RenderState&      renderState );
        void drawBezier( const ::com::sun::star::geometry::RealBezierSegment2D& aBezierSegment,
                         const ::com::sun::star::geometry::RealPoint2D&         aEndPoint,
                         const ::com::sun::star::rendering::ViewState&          viewState,
                         const ::com::sun::star::rendering::RenderState&        renderState );
        ImageCachedPrimitiveSharedPtr drawPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState );

        ImageCachedPrimitiveSharedPtr strokePolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState,
            const ::com::sun::star::rendering::StrokeAttributes&    strokeAttributes );
        ImageCachedPrimitiveSharedPtr strokeTexturedPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState,
            const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::rendering::Texture >&           textures,
            const ::std::vector< ::boost::shared_ptr<Image> >&      textureAnnotations,
            const ::com::sun::star::rendering::StrokeAttributes&    strokeAttributes );
        ImageCachedPrimitiveSharedPtr strokeTextureMappedPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState,
            const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::rendering::Texture >&           textures,
            const ::std::vector< ::boost::shared_ptr<Image> >&      textureAnnotations,
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::geometry::XMapping2D >&         xMapping,
            const ::com::sun::star::rendering::StrokeAttributes&    strokeAttributes );
        ImageCachedPrimitiveSharedPtr fillPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState );
        ImageCachedPrimitiveSharedPtr fillTexturedPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState,
            const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::rendering::Texture >&           textures,
            const ::std::vector< ::boost::shared_ptr<Image> >&      textureAnnotations );
        ImageCachedPrimitiveSharedPtr fillTextureMappedPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState,
            const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::rendering::Texture >&           textures,
            const ::std::vector< ::boost::shared_ptr<Image> >&      textureAnnotations,
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::geometry::XMapping2D >&         xMapping );

        ImageCachedPrimitiveSharedPtr drawBitmap(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XBitmap >&   xBitmap,
            const ::com::sun::star::rendering::ViewState&   viewState,
            const ::com::sun::star::rendering::RenderState& renderState );
        ImageCachedPrimitiveSharedPtr drawBitmap(
            const ::boost::shared_ptr<Image>&               rImage,
            const ::com::sun::star::rendering::ViewState&   viewState,
            const ::com::sun::star::rendering::RenderState& renderState );

        ImageCachedPrimitiveSharedPtr drawBitmapModulated(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XBitmap >&       xBitmap,
            const ::com::sun::star::rendering::ViewState&       viewState,
            const ::com::sun::star::rendering::RenderState&     renderState );
        ImageCachedPrimitiveSharedPtr drawBitmapModulated(
            const ::boost::shared_ptr<Image>&                   rImage,
            const ::com::sun::star::rendering::ViewState&       viewState,
            const ::com::sun::star::rendering::RenderState&     renderState );

    private:
        void drawLinePolyPolygon( const ::basegfx::B2DPolyPolygon&                     rPoly,
                                  double                                               fStrokeWidth,
                                  const ::com::sun::star::rendering::ViewState&        viewState,
                                  const ::com::sun::star::rendering::RenderState&      renderState );

        ImageCachedPrimitiveSharedPtr implDrawBitmap(
                             const Image&                                    rBitmap,
                             const ::com::sun::star::rendering::ViewState&   viewState,
                             const ::com::sun::star::rendering::RenderState& renderState );

        ImageCachedPrimitiveSharedPtr fillTexturedPolyPolygon(
                                      const Image&                                   rTexture,
                                      const ::basegfx::B2DPolyPolygon&               rPolyPolygon,
                                      const ::basegfx::B2DHomMatrix&                 rOverallTransform,
                                      const ::basegfx::B2DHomMatrix&                 rViewTransform,
                                      const ::com::sun::star::rendering::Texture&    texture );

        void fillGradient( const ParametricPolyPolygon::Values&                      rValues,
                           const ::com::sun::star::uno::Sequence< double >&          rColor1,
                           const ::com::sun::star::uno::Sequence< double >&          rColor2,
                           const ::basegfx::B2DPolyPolygon&             rPolyPolygon,
                           const ::basegfx::B2DHomMatrix&               rOverallTransform,
                           const ::com::sun::star::rendering::Texture&  texture );
        bool fromVCLBitmap( ::BitmapEx& rBmpEx );

        template<class pixel_format>
        void drawLinePolyPolygonImpl( const ::basegfx::B2DPolyPolygon&                  rPoly,
                                      double                                            fStrokeWidth,
                                      const ::com::sun::star::rendering::ViewState&     viewState,
                                      const ::com::sun::star::rendering::RenderState&   renderState );

        template<class pixel_format,class span_gen_type>
        ImageCachedPrimitiveSharedPtr fillTexturedPolyPolygonImpl(
                                      const Image&                                   rTexture,
                                      const ::basegfx::B2DPolyPolygon&               rPolyPolygon,
                                      const ::basegfx::B2DHomMatrix&                 rOverallTransform,
                                      const ::basegfx::B2DHomMatrix&                 rViewTransform,
                                      const ::com::sun::star::rendering::Texture&    texture );

        template<class pixel_format>
        void fillGradientImpl( const ParametricPolyPolygon::Values&             rValues,
                               const ::com::sun::star::uno::Sequence< double >& rUnoColor1,
                               const ::com::sun::star::uno::Sequence< double >& rUnoColor2,
                               const ::basegfx::B2DPolyPolygon&                 rPolyPolygon,
                               const ::basegfx::B2DHomMatrix&                   rOverallTransform,
                               const ::com::sun::star::rendering::Texture&      texture );

        template<class pixel_format>
        ImageCachedPrimitiveSharedPtr fillPolyPolygonImpl(
            const ::basegfx::B2DPolyPolygon&                    rPolyPolygon,
            const ::com::sun::star::rendering::ViewState&       viewState,
            const ::com::sun::star::rendering::RenderState&     renderState );

        template<class pixel_format> void clearImpl( sal_uInt8 a,
                                                     sal_uInt8 r,
                                                     sal_uInt8 g,
                                                     sal_uInt8 b );

        /** Image description
         */
        Description             maDesc;

        /** the graphics buffer is a simple array
            where each element points to the start
            of a scanline in consecutive order.
         */
        agg::rendering_buffer   maRenderingBuffer;

        /// Whether maRenderingBuffer is owned by the client of this object
        bool                    mbBufferHasUserOwnership;

#if defined(PROFILER)

        enum constant
        {
            TIMER_FILLTEXTUREDPOLYPOLYGON,
            TIMER_FILLB2DPOLYPOLYGON,
            TIMER_DRAWPOLYPOLYGON,
            TIMER_FILLPOLYPOLYGON,
            TIMER_DRAWBITMAP,
            TIMER_MAX
        };

        double maElapsedTime[TIMER_MAX];

        struct ScopeTimer
        {
            ScopeTimer( constant aConstant, Image *pImage ) :
                maConstant(aConstant),mpImage(pImage)
                    {}

            ~ScopeTimer()
            {
                mpImage->maElapsedTime[maConstant] += maTimer.getElapsedTime();
            }

            constant                     maConstant;
            Image*                       mpImage;
            ::canvas::tools::ElapsedTime maTimer;
        };

#endif
    };

    typedef ::boost::shared_ptr< Image > ImageSharedPtr;

}

#endif /* INCLUDED_CANVAS_IMAGE_HXX */
