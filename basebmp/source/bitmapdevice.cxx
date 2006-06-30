/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bitmapdevice.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: thb $ $Date: 2006-06-30 11:05:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "basebmp/bitmapdevice.hxx"
#include "basebmp/packedpixeliterator.hxx"
#include "basebmp/pixeliterator.hxx"
#include "basebmp/paletteimageaccessor.hxx"
#include "basebmp/color.hxx"
#include "basebmp/accessor.hxx"
#include "basebmp/accessoradapters.hxx"
#include "basebmp/scanlineformats.hxx"
#include "basebmp/clippedlinerenderer.hxx"
#include "basebmp/compositeiterator.hxx"

#include <rtl/alloc.h>
#include <rtl/memory.h>
#include <osl/diagnose.h>

#include <basegfx/tools/tools.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygonrasterconverter.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/vector/b2ivector.hxx>

#include <vigra/basicimage.hxx>
#include <vigra/imageiterator.hxx>
#include <vigra/resizeimage.hxx>
#include <vigra/copyimage.hxx>
#include <vigra/tuple.hxx>


namespace basebmp
{

namespace
{
    // Common accessor and iterator types
    //------------------------------------------------------------------------

    typedef PaletteImageAccessor<Color,sal_uInt8>                 PaletteAccessor;

    typedef PackedPixelIterator< sal_uInt8,
                                 1,
                                 true >                           MaskIterator;
    typedef StandardAccessor< sal_uInt8 >                         MaskAccessor;

    typedef PixelIterator< sal_uInt8 >                            AlphaMaskIterator;
    typedef vigra::AccessorTraits< sal_uInt8 >::default_accessor  AlphaMaskAccessor;

    typedef PixelIterator<Color>                                  ThirtyTwoBitPixelIterator;
    typedef vigra::AccessorTraits<Color>::default_accessor        ThirtyTwoBitAccessor;


    // metafunctions to retrieve correct POD from/to basebmp::Color
    //------------------------------------------------------------------------

    /// type-safe conversion between color and pod
    template< typename DataType > struct GreyScaleFromColor
    {
        typedef DataType value_type;
        DataType operator()( Color c ) { return c.getGrayscale(); }
    };

    /// type-safe conversion between pod and color
    template< typename DataType > struct ColorFromGreyScale
    {
        typedef DataType value_type;
        Color operator()( DataType c ) { return Color(c,c,c); }
    };

    /// type-safe conversion from Color to packed int32
    struct UInt32FromColor
    {
        sal_uInt32 operator()( const Color& c ) { return c.toInt32(); }
    };

    /// Get converter from color to given data type
    template< typename DataType > struct fromColorConverter;
    template<> struct fromColorConverter< sal_uInt8 >
    {
        typedef GreyScaleFromColor<sal_uInt8> type;
    };
    template<> struct fromColorConverter< Color >
    {
        typedef std::identity<Color> type;
    };

    /// Get converter from given data type to Color
    template< typename DataType > struct toColorConverter;
    template<> struct toColorConverter< sal_uInt8 >
    {
        typedef ColorFromGreyScale<sal_uInt8> type;
    };
    template<> struct toColorConverter< Color >
    {
        typedef std::identity<Color> type;
    };

    /// Get converter from given data type to sal_uInt32
    template< typename DataType > struct toUInt32Converter
    {
        typedef std::identity<DataType> type;
    };
    template<> struct toUInt32Converter< Color >
    {
        typedef UInt32FromColor type;
    };


    // Polygon scanline conversion
    //------------------------------------------------------------------------

    template< class DestIterator, class DestAccessor > class Renderer :
        public basegfx::B2DPolyPolygonRasterConverter
    {
    private:
        basegfx::B2IRange                 bounds_;
        typename DestAccessor::value_type fillColor_;
        typename DestAccessor::value_type clearColor_;
        DestIterator                      begin_;
        DestAccessor                      accessor_;

    public:
        Renderer(const basegfx::B2DPolyPolygon&     rPolyPolyRaster,
                 typename DestAccessor::value_type  fillColor,
                 const basegfx::B2IRange&           bounds,
                 DestIterator                       begin,
                 DestAccessor                       accessor ) :
            B2DPolyPolygonRasterConverter(rPolyPolyRaster,
                                          basegfx::B2DRange(bounds) ),
            bounds_(bounds),
            fillColor_( fillColor ),
            begin_( begin ),
            accessor_( accessor )
        {
        }

        virtual void span(const double& rfXLeft,
                          const double& rfXRight,
                          sal_Int32     nY,
                          bool          bOn )
        {
            if( !bOn ||
                nY < bounds_.getMinY() ||
                nY >= bounds_.getMaxY() ||
                rfXLeft >= bounds_.getMaxX() ||
                rfXRight < bounds_.getMinX() )
            {
                return;
            }

            // clip span to bitmap bounds
            const sal_Int32 nStartX( std::max( bounds_.getMinX(),
                                               std::min( bounds_.getMaxX()-1,
                                                         basegfx::fround( rfXLeft ))));
            const sal_Int32 nEndX  ( std::max( bounds_.getMinX(),
                                               std::min( bounds_.getMaxX(),
                                                         basegfx::fround( rfXRight ))));

            DestIterator currIter( begin_ + vigra::Diff2D(0,nY) );
            typename DestIterator::row_iterator rowIter( currIter.rowIterator() + nStartX);
            typename DestIterator::row_iterator rowEnd( currIter.rowIterator() + nEndX );

            // TODO(P2): Provide specialized span fill methods on the
            // iterator/accessor
            while( rowIter != rowEnd )
                accessor_.set(fillColor_, rowIter++);
        }
    };

    template< class DestIterator, class DestAccessor >
    std::auto_ptr< Renderer< DestIterator, DestAccessor > > makeRenderer(
        const basegfx::B2DPolyPolygon&                          rPolyPolyRaster,
        typename DestAccessor::value_type                       fillColor,
        const basegfx::B2IRange&                                outRange,
        vigra::triple<DestIterator, DestIterator, DestAccessor> dest )
    {
        basegfx::B2IRange aBmpRange(0,0,
                                    dest.second.x - dest.first.x,
                                    dest.second.y - dest.first.y );
        aBmpRange.intersect( outRange );

        return std::auto_ptr< Renderer< DestIterator, DestAccessor > >(
            new Renderer< DestIterator, DestAccessor >(rPolyPolyRaster,
                                                       fillColor,
                                                       aBmpRange,
                                                       dest.first,
                                                       dest.third));
    }


    // Actual BitmapDevice implementation (templatized by accessor and iterator)
    //--------------------------------------------------------------------------

    template< class DestIterator,
              class DestAccessor > class BitmapRenderer : public BitmapDevice
    {
    public:
        typedef BitmapRenderer<MaskIterator,
                               MaskAccessor>                MaskBitmap;
        typedef BitmapRenderer<AlphaMaskIterator,
                               AlphaMaskAccessor>           AlphaMaskBitmap;
        typedef typename colorLookup<DestAccessor>::type    ColorLookupFunctor;
        typedef typename fromColorConverter<
            typename DestAccessor::value_type>::type        FromColorFunctor;
        typedef typename toColorConverter<
            typename DestAccessor::value_type>::type        ToColorFunctor;
        typedef typename rawAccessor<DestAccessor>::type    RawAccessor;
        typedef typename toUInt32Converter<
            typename RawAccessor::value_type>::type         ToUInt32Functor;
        typedef typename xorAccessor<DestAccessor>::type    XorAccessor;
        typedef typename xorAccessor<RawAccessor>::type     RawXorAccessor;
        typedef typename maskedAccessor<DestAccessor,
                                        MaskAccessor,
                                        DestIterator,
                                        MaskIterator>::type MaskedAccessor;
        typedef typename maskedAccessor<RawAccessor,
                                        MaskAccessor,
                                        DestIterator,
                                        MaskIterator>::type RawMaskedAccessor;
        typedef typename maskedAccessor<XorAccessor,
                                        MaskAccessor,
                                        DestIterator,
                                        MaskIterator>::type MaskedXorAccessor;
        typedef ConstantColorBlendSetterAccessorAdapter<
            DestAccessor,
            typename AlphaMaskAccessor::value_type>         ColorBlendAccessor;
        typedef typename maskedAccessor<ColorBlendAccessor,
                                        MaskAccessor,
                                        DestIterator,
                                        MaskIterator>::type MaskedColorBlendAcc;
        typedef typename maskedAccessor<RawXorAccessor,
                                        MaskAccessor,
                                        DestIterator,
                                        MaskIterator>::type RawMaskedXorAccessor;

        typedef DestIterator                                    dest_iterator;
        typedef DestAccessor                                    dest_accessor;
        typedef CompositeIterator2D< DestIterator,
                                     MaskIterator >             composite_iterator_type;

        DestIterator          maBegin;
        DestIterator          maEnd;
        ColorLookupFunctor    maColorLookup;
        FromColorFunctor      maFromColorConverter;
        ToColorFunctor        maToColorConverter;
        ToUInt32Functor       maToUInt32Converter;
        DestAccessor          maAccessor;
        RawAccessor           maRawAccessor;
        XorAccessor           maXorAccessor;
        RawXorAccessor        maRawXorAccessor;
        MaskedAccessor        maMaskedAccessor;
        MaskedColorBlendAcc   maMaskedColorBlendAccessor;
        MaskedXorAccessor     maMaskedXorAccessor;
        RawMaskedAccessor     maRawMaskedAccessor;
        RawMaskedXorAccessor  maRawMaskedXorAccessor;
        int                   mnWidth;
        int                   mnHeight;

        BitmapRenderer( const basegfx::B2IVector&        rSize,
                        bool                             bTopDown,
                        sal_Int32                        nScanlineFormat,
                        sal_Int32                        nScanlineStride,
                        DestIterator                     begin,
                        DestIterator                     end,
                        DestAccessor                     accessor,
                        const RawMemorySharedArray&      rMem,
                        const PaletteMemorySharedVector& rPalette ) :
            BitmapDevice( rSize, bTopDown, nScanlineFormat, nScanlineStride, rMem, rPalette ),
            maBegin( begin ),
            maEnd( end ),
            maColorLookup(),
            maFromColorConverter(),
            maToColorConverter(),
            maToUInt32Converter(),
            maAccessor( accessor ),
            maRawAccessor( accessor ),
            maXorAccessor( accessor ),
            maRawXorAccessor( maRawAccessor ),
            maMaskedAccessor( accessor ),
            maMaskedColorBlendAccessor(
                ColorBlendAccessor(accessor) ),
            maMaskedXorAccessor( maXorAccessor ),
            maRawMaskedAccessor( maRawAccessor ),
            maRawMaskedXorAccessor( maRawXorAccessor ),
            mnWidth( maEnd.x - maBegin.x ),
            mnHeight( maEnd.y - maBegin.y )
        {}

    private:
        boost::shared_ptr<BitmapRenderer> getCompatibleBitmap( const BitmapDeviceSharedPtr& bmp ) const
        {
            return boost::dynamic_pointer_cast< BitmapRenderer >( bmp );
        }

        virtual bool isCompatibleBitmap( const BitmapDeviceSharedPtr& bmp ) const
        {
            // TODO(P1): dynamic_cast usually called twice for
            // compatible formats
            return getCompatibleBitmap(bmp).get() != NULL;
        }

        boost::shared_ptr<MaskBitmap> getCompatibleClipMask( const BitmapDeviceSharedPtr& bmp ) const
        {
            boost::shared_ptr<MaskBitmap> pMask( boost::dynamic_pointer_cast<MaskBitmap>( bmp ));

            if( !pMask )
                return pMask;

            if( pMask->getSize() != getSize() )
                pMask.reset();

            return pMask;
        }

        virtual bool isCompatibleClipMask( const BitmapDeviceSharedPtr& bmp ) const
        {
            // TODO(P1): dynamic_cast usually called twice for
            // compatible formats
            return boost::dynamic_pointer_cast<MaskBitmap>( bmp ).get() != NULL;
        }

        boost::shared_ptr<AlphaMaskBitmap> getCompatibleAlphaMask( const BitmapDeviceSharedPtr& bmp ) const
        {
            return boost::dynamic_pointer_cast<AlphaMaskBitmap>( bmp );
        }

        virtual bool isCompatibleAlphaMask( const BitmapDeviceSharedPtr& bmp ) const
        {
            // TODO(P1): dynamic_cast usually called twice for
            // compatible formats
            return getCompatibleAlphaMask( bmp ).get() != NULL;
        }

        virtual void clear_i( Color fillColor )
        {
            const typename dest_iterator::value_type colorIndex( maColorLookup(
                                                                     maAccessor,
                                                                     maFromColorConverter(
                                                                         fillColor)));
            DestIterator       currIter( maBegin );
            const DestIterator endIter ( maBegin + vigra::Diff2D(0,mnHeight) );
            while( currIter != endIter )
            {
                typename DestIterator::row_iterator rowIter( currIter.rowIterator() );
                const typename DestIterator::row_iterator rowEnd( rowIter + mnWidth );

                // TODO(P2): Provide specialized span fill methods on the
                // iterator/accessor
                while( rowIter != rowEnd )
                    maRawAccessor.set(colorIndex, rowIter++);

                ++currIter.y;
            }
        }

        virtual void setPixel_i( const basegfx::B2IPoint& rPt,
                                 Color                    lineColor,
                                 DrawMode                 drawMode )
        {
            const DestIterator pixel( maBegin +
                                      vigra::Diff2D(rPt.getX(),
                                                    rPt.getY()) );
            if( drawMode == DrawMode_XOR )
                maXorAccessor.set( maFromColorConverter(
                                       lineColor),
                                   pixel );
            else
                maAccessor.set( maFromColorConverter(
                                    lineColor),
                                pixel );
        }

        virtual void setPixel_i( const basegfx::B2IPoint&     rPt,
                                 Color                        lineColor,
                                 DrawMode                     drawMode,
                                 const BitmapDeviceSharedPtr& rClip )
        {
            boost::shared_ptr<MaskBitmap> pMask( getCompatibleClipMask(rClip) );
            OSL_ASSERT( pMask );

            const vigra::Diff2D offset(rPt.getX(),
                                       rPt.getY());

            const composite_iterator_type aIter(
                maBegin + offset,
                pMask->maBegin + offset );

            if( drawMode == DrawMode_XOR )
                maMaskedXorAccessor.set( maFromColorConverter(
                                             lineColor),
                                         aIter );
            else
                maMaskedAccessor.set( maFromColorConverter(
                                          lineColor),
                                      aIter );
        }

        virtual Color getPixel_i(const basegfx::B2IPoint& rPt )
        {
            const DestIterator pixel( maBegin +
                                      vigra::Diff2D(rPt.getX(),
                                                    rPt.getY()) );
            return maToColorConverter(maAccessor(pixel));
        }

        virtual sal_uInt32 getPixelData_i( const basegfx::B2IPoint& rPt )
        {
            const DestIterator pixel( maBegin +
                                      vigra::Diff2D(rPt.getX(),
                                                    rPt.getY()) );
            return maToUInt32Converter(maRawAccessor(pixel));
        }

        template< typename Range, typename Col, typename RawAccessor >
        void implRenderLine2( const basegfx::B2IPoint& rPt1,
                              const basegfx::B2IPoint& rPt2,
                              const basegfx::B2IRange& rBounds,
                              Col                      col,
                              const Range&             range,
                              const RawAccessor&       rawAcc )
        {
            renderClippedLine( rPt1,
                               rPt2,
                               rBounds,
                               col,
                               range.first,
                               rawAcc );
        }

        template< typename Range, typename Accessor, typename RawAccessor >
        void implRenderLine( const basegfx::B2IPoint& rPt1,
                             const basegfx::B2IPoint& rPt2,
                             const basegfx::B2IRange& rBounds,
                             Color                    col,
                             const Range&             range,
                             const Accessor&          acc,
                             const RawAccessor&       rawAcc )
        {
            implRenderLine2( rPt1,rPt2,rBounds,
                             maColorLookup( acc,
                                            maFromColorConverter(
                                                col)),
                             range,
                             rawAcc );
        }

        template< typename Range, typename RawAccessor, typename XorAccessor >
        void implDrawLine( const basegfx::B2IPoint& rPt1,
                           const basegfx::B2IPoint& rPt2,
                           const basegfx::B2IRange& rBounds,
                           Color                    col,
                           const Range&             range,
                           const RawAccessor&       rawAcc,
                           const XorAccessor&       xorAcc,
                           DrawMode                 drawMode )
        {
            if( drawMode == DrawMode_XOR )
                implRenderLine( rPt1, rPt2, rBounds, col,
                                range, maAccessor, xorAcc );
            else
                implRenderLine( rPt1, rPt2, rBounds, col,
                                range, maAccessor, rawAcc );
        }

        virtual void drawLine_i(const basegfx::B2IPoint& rPt1,
                                const basegfx::B2IPoint& rPt2,
                                const basegfx::B2IRange& rBounds,
                                Color                    lineColor,
                                DrawMode                 drawMode )
        {
            implDrawLine(rPt1,rPt2,rBounds,lineColor,
                         std::make_pair(maBegin,maEnd),
                         maRawAccessor,maRawXorAccessor,drawMode);
        }

        vigra::pair<composite_iterator_type,composite_iterator_type>
          getMaskedRange( const BitmapDeviceSharedPtr& rClip ) const
        {
            boost::shared_ptr<MaskBitmap> pMask( getCompatibleClipMask(rClip) );
            OSL_ASSERT( pMask );

            return std::make_pair(
                composite_iterator_type(
                    maBegin,
                    pMask->maBegin ),
                composite_iterator_type(
                    maEnd,
                    pMask->maEnd ));
        }

        virtual void drawLine_i(const basegfx::B2IPoint&     rPt1,
                                const basegfx::B2IPoint&     rPt2,
                                const basegfx::B2IRange&     rBounds,
                                Color                        lineColor,
                                DrawMode                     drawMode,
                                const BitmapDeviceSharedPtr& rClip )
        {
            implDrawLine(rPt1,rPt2,rBounds,lineColor,
                         getMaskedRange(rClip),
                         maRawMaskedAccessor,maRawMaskedXorAccessor,drawMode);
        }

        template< typename Range, typename RawAccessor >
        void implDrawPolygon( const basegfx::B2DPolygon& rPoly,
                              const basegfx::B2IRange&   rBounds,
                              Color                      col,
                              const Range&               range,
                              const RawAccessor&         acc )
        {
            basegfx::B2DPolygon aPoly( rPoly );
            if( rPoly.areControlVectorsUsed() )
                aPoly = basegfx::tools::adaptiveSubdivideByCount( rPoly );

            const typename dest_iterator::value_type colorIndex( maColorLookup(
                                                                     maAccessor,
                                                                     maFromColorConverter(
                                                                         col)));
            const sal_uInt32                         nVertices( aPoly.count() );
            for( sal_uInt32 i=1; i<nVertices; ++i )
                implRenderLine2( basegfx::fround(aPoly.getB2DPoint(i-1)),
                                 basegfx::fround(aPoly.getB2DPoint(i)),
                                 rBounds,
                                 colorIndex,
                                 range,
                                 acc );

            if( nVertices > 1 && aPoly.isClosed() )
                implRenderLine2( basegfx::fround(aPoly.getB2DPoint(nVertices-1)),
                                 basegfx::fround(aPoly.getB2DPoint(0)),
                                 rBounds,
                                 colorIndex,
                                 range,
                                 acc );
        }

        virtual void drawPolygon_i(const basegfx::B2DPolygon& rPoly,
                                   const basegfx::B2IRange&   rBounds,
                                   Color                      lineColor,
                                   DrawMode                   drawMode )
        {
            if( drawMode == DrawMode_XOR )
                implDrawPolygon( rPoly, rBounds, lineColor,
                                 std::make_pair(maBegin,
                                                maEnd),
                                 maRawXorAccessor );
            else
                implDrawPolygon( rPoly, rBounds, lineColor,
                                 std::make_pair(maBegin,
                                                maEnd),
                                 maRawAccessor );
        }

        virtual void drawPolygon_i(const basegfx::B2DPolygon&   rPoly,
                                   const basegfx::B2IRange&     rBounds,
                                   Color                        lineColor,
                                   DrawMode                     drawMode,
                                   const BitmapDeviceSharedPtr& rClip )
        {
            if( drawMode == DrawMode_XOR )
                implDrawPolygon( rPoly, rBounds, lineColor,
                                 getMaskedRange(rClip),
                                 maRawMaskedXorAccessor );
            else
                implDrawPolygon( rPoly, rBounds, lineColor,
                                 getMaskedRange(rClip),
                                 maRawMaskedAccessor );
        }

        template< typename Range, typename RawAccessor >
        void implFillPolyPolygon( const basegfx::B2DPolyPolygon& rPoly,
                                  Color                          col,
                                  const Range&                   range,
                                  const RawAccessor&             acc,
                                  const basegfx::B2IRange&       rBounds )
        {
            basegfx::B2DPolyPolygon aPoly( rPoly );
            if( rPoly.areControlVectorsUsed() )
                aPoly = basegfx::tools::adaptiveSubdivideByCount( rPoly );

            makeRenderer( aPoly,
                          maColorLookup( maAccessor,
                                         maFromColorConverter(
                                             col)),
                          rBounds,
                          vigra::make_triple(
                              range.first,
                              range.second,
                              acc) )->rasterConvert(
                                  basegfx::FillRule_NONZERO_WINDING_NUMBER );
        }

        virtual void fillPolyPolygon_i(const basegfx::B2DPolyPolygon& rPoly,
                                       Color                          fillColor,
                                       DrawMode                       drawMode,
                                       const basegfx::B2IRange&       rBounds )
        {
            if( drawMode == DrawMode_XOR )
                implFillPolyPolygon( rPoly, fillColor,
                                     std::make_pair(maBegin,maEnd),
                                     maRawXorAccessor,
                                     rBounds );
            else
                implFillPolyPolygon( rPoly, fillColor,
                                     std::make_pair(maBegin,maEnd),
                                     maRawAccessor,
                                     rBounds );
        }

        virtual void fillPolyPolygon_i(const basegfx::B2DPolyPolygon& rPoly,
                                       Color                          fillColor,
                                       DrawMode                       drawMode,
                                       const basegfx::B2IRange&       rBounds,
                                       const BitmapDeviceSharedPtr&   rClip )
        {
            if( drawMode == DrawMode_XOR )
                implFillPolyPolygon( rPoly, fillColor,
                                     getMaskedRange(rClip),
                                     maRawMaskedXorAccessor,
                                     rBounds );
            else
                implFillPolyPolygon( rPoly, fillColor,
                                     getMaskedRange(rClip),
                                     maRawMaskedAccessor,
                                     rBounds );
        }

        template< typename Range, typename RawAccessor >
        void implDrawBitmap(const BitmapDeviceSharedPtr& rSrcBitmap,
                            const basegfx::B2IRange&     rSrcRect,
                            const basegfx::B2IRange&     rDstRect,
                            const Range&                 range,
                            const RawAccessor&           acc)
        {
            boost::shared_ptr<BitmapRenderer> pSrcBmp( getCompatibleBitmap(rSrcBitmap) );
            OSL_ASSERT( pSrcBmp );

            // since resizeImageNoInterpolation() internally copyies
            // to a temporary buffer, also works with *this == rSrcBitmap
            vigra::resizeImageNoInterpolation(
                vigra::make_triple(
                    pSrcBmp->maBegin + vigra::Diff2D(rSrcRect.getMinX(),
                                                     rSrcRect.getMinY()),
                    pSrcBmp->maBegin + vigra::Diff2D(rSrcRect.getMaxX(),
                                                     rSrcRect.getMaxY()),
                    pSrcBmp->maRawAccessor),
                vigra::make_triple(
                    range.first + vigra::Diff2D(rDstRect.getMinX(),
                                                rDstRect.getMinY()),
                    range.first + vigra::Diff2D(rDstRect.getMaxX(),
                                                rDstRect.getMaxY()),
                    acc));
        }

        virtual void drawBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                  const basegfx::B2IRange&     rSrcRect,
                                  const basegfx::B2IRange&     rDstRect,
                                  DrawMode                     drawMode )
        {
            if( drawMode == DrawMode_XOR )
                implDrawBitmap(rSrcBitmap, rSrcRect, rDstRect,
                               std::make_pair(maBegin,maEnd),
                               maRawXorAccessor);
            else
                implDrawBitmap(rSrcBitmap, rSrcRect, rDstRect,
                               std::make_pair(maBegin,maEnd),
                               maRawAccessor);
        }

        virtual void drawBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                  const basegfx::B2IRange&     rSrcRect,
                                  const basegfx::B2IRange&     rDstRect,
                                  DrawMode                     drawMode,
                                  const BitmapDeviceSharedPtr& rClip )
        {
            if( drawMode == DrawMode_XOR )
                implDrawBitmap(rSrcBitmap, rSrcRect, rDstRect,
                               getMaskedRange(rClip),
                               maRawMaskedXorAccessor);
            else
                implDrawBitmap(rSrcBitmap, rSrcRect, rDstRect,
                               getMaskedRange(rClip),
                               maRawMaskedAccessor);
        }

        virtual void drawMaskedColor_i(Color                        aSrcColor,
                                       const BitmapDeviceSharedPtr& rAlphaMask,
                                       const basegfx::B2IRange&     rSrcRect,
                                       const basegfx::B2IPoint&     rDstPoint )
        {
            boost::shared_ptr<AlphaMaskBitmap> pAlpha( getCompatibleAlphaMask(rAlphaMask) );
            OSL_ASSERT( pAlpha );

            vigra::copyImage( pAlpha->maBegin + vigra::Diff2D(rSrcRect.getMinX(),
                                                              rSrcRect.getMinY()),
                              pAlpha->maBegin + vigra::Diff2D(rSrcRect.getMaxX(),
                                                              rSrcRect.getMaxY()),
                              pAlpha->maAccessor,
                              maBegin + vigra::Diff2D(rDstPoint.getX(),
                                                      rDstPoint.getY()),
                              ColorBlendAccessor( maAccessor,
                                                  maFromColorConverter(aSrcColor)) );
        }

        virtual void drawMaskedColor_i(Color                        aSrcColor,
                                       const BitmapDeviceSharedPtr& rAlphaMask,
                                       const basegfx::B2IRange&     rSrcRect,
                                       const basegfx::B2IPoint&     rDstPoint,
                                       const BitmapDeviceSharedPtr& rClip )
        {
            boost::shared_ptr<AlphaMaskBitmap> pAlpha( getCompatibleAlphaMask(rAlphaMask) );
            OSL_ASSERT( pAlpha );

            const vigra::pair<composite_iterator_type,
                              composite_iterator_type> aRange( getMaskedRange(rClip) );
            maMaskedColorBlendAccessor.setColor( maFromColorConverter(aSrcColor) );

            vigra::copyImage( pAlpha->maBegin + vigra::Diff2D(rSrcRect.getMinX(),
                                                              rSrcRect.getMinY()),
                              pAlpha->maBegin + vigra::Diff2D(rSrcRect.getMaxX(),
                                                              rSrcRect.getMaxY()),
                              pAlpha->maAccessor,
                              aRange.first + vigra::Diff2D(rDstPoint.getX(),
                                                           rDstPoint.getY()),
                              maMaskedColorBlendAccessor );
        }

        virtual void drawMaskedBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                        const BitmapDeviceSharedPtr& rMask,
                                        const basegfx::B2IRange&     rSrcRect,
                                        const basegfx::B2IRange&     rDstRect,
                                        DrawMode                     drawMode )
        {
            // TODO(F3): This is a hack, at best. The mask must be the
            // size of the source bitmap, and accordingly
            // translated. Let alone interpolated.
            if( drawMode == DrawMode_XOR )
                implDrawBitmap(rSrcBitmap, rSrcRect, rDstRect,
                               getMaskedRange(rMask),
                               maRawMaskedXorAccessor);
            else
                implDrawBitmap(rSrcBitmap, rSrcRect, rDstRect,
                               getMaskedRange(rMask),
                               maRawMaskedAccessor);
        }

        virtual void drawMaskedBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                        const BitmapDeviceSharedPtr& rMask,
                                        const basegfx::B2IRange&     rSrcRect,
                                        const basegfx::B2IRange&     rDstRect,
                                        DrawMode                     drawMode,
                                        const BitmapDeviceSharedPtr& rClip )
        {
            // TODO(F3): Clipped drawMaskedBitmap_i() not yet implemented
            (void)rClip;
            drawMaskedBitmap_i( rSrcBitmap,
                                rMask,
                                rSrcRect,
                                rDstRect,
                                drawMode );
            OSL_ENSURE( false, "Method not yet implemented, falling back to unclipped version!" );
        }
    };
} // namespace

struct ImplBitmapDevice
{
    /** Bitmap memory plus deleter.

        Always points to the start of the mem
     */
    RawMemorySharedArray      mpMem;

    /// Palette memory plus deleter (might be NULL)
    PaletteMemorySharedVector mpPalette;

    /** Bounds of the device.

        maBounds.getWidth()/getHeight() yield the true size of the
        device (i.e. the rectangle given by maBounds covers the device
        area under the excluding-bottommost-and-rightmost-pixels fill rule)
     */
    basegfx::B2IRange         maBounds;

    /** Bounds of the device.

        maBounds.getWidth()/getHeight() yield the true size of the
        device minus 1 (i.e. the rectangle given by maBounds covers
        the device area under the
        including-the-bottommost-and-rightmost-pixels fill rule).

        The member is used to clip line stroking against the device
        bounds.
     */
    basegfx::B2IRange         maLineClipRect;

    /// Scanline format, as provided at the constructor
    sal_Int32                 mnScanlineFormat;

    /// Scanline stride. Negative for bottom-to-top formats
    sal_Int32                 mnScanlineStride;
};


BitmapDevice::BitmapDevice( const basegfx::B2IVector&        rSize,
                            bool                             bTopDown,
                            sal_Int32                        nScanlineFormat,
                            sal_Int32                        nScanlineStride,
                            const RawMemorySharedArray&      rMem,
                            const PaletteMemorySharedVector& rPalette ) :
    mpImpl( new ImplBitmapDevice )
{
    mpImpl->mpMem = rMem;
    mpImpl->mpPalette = rPalette;
    mpImpl->maBounds = basegfx::B2IRange( 0,0,rSize.getX(),rSize.getY() );
    mpImpl->maLineClipRect = basegfx::B2IRange( 0,0,rSize.getX()-1,rSize.getY()-1 );
    mpImpl->mnScanlineFormat = nScanlineFormat;
    mpImpl->mnScanlineStride = bTopDown ? nScanlineStride : -nScanlineStride;
}

BitmapDevice::~BitmapDevice()
{
    // outline, because of internal ImplBitmapDevice
}

basegfx::B2IVector BitmapDevice::getSize() const
{
    return basegfx::B2IVector( mpImpl->maBounds.getWidth(),
                               mpImpl->maBounds.getHeight() );
}

bool BitmapDevice::isTopDown() const
{
    return mpImpl->mnScanlineStride >= 0;
}

sal_Int32 BitmapDevice::getScanlineFormat() const
{
    return mpImpl->mnScanlineFormat;
}

sal_Int32 BitmapDevice::getScanlineStride() const
{
    return mpImpl->mnScanlineStride;
}

RawMemorySharedArray BitmapDevice::getBuffer() const
{
    return mpImpl->mpMem;
}

PaletteMemorySharedVector BitmapDevice::getPalette() const
{
    return mpImpl->mpPalette;
}

const sal_Int32 BitmapDevice::getPaletteEntryCount() const
{
    return mpImpl->mpPalette ? mpImpl->mpPalette->size() : 0;
}

void BitmapDevice::clear( Color fillColor )
{
    clear_i( fillColor );
}

void BitmapDevice::setPixel( const basegfx::B2IPoint& rPt,
                             Color                    lineColor,
                             DrawMode                 drawMode )
{
    if( mpImpl->maLineClipRect.isInside(rPt) )
        setPixel_i(rPt,lineColor,drawMode);
}

void BitmapDevice::setPixel( const basegfx::B2IPoint&     rPt,
                             Color                        lineColor,
                             DrawMode                     drawMode,
                             const BitmapDeviceSharedPtr& rClip )
{
    if( !rClip )
    {
        setPixel(rPt,lineColor,drawMode);
        return;
    }

    if( mpImpl->maLineClipRect.isInside(rPt) )
    {
        if( isCompatibleClipMask( rClip ) )
            setPixel_i(rPt,lineColor,drawMode,rClip);
        else
            OSL_ENSURE( false, "Generic output not yet implemented!" );
    }
}

Color BitmapDevice::getPixel( const basegfx::B2IPoint& rPt )
{
    if( mpImpl->maLineClipRect.isInside(rPt) )
        return getPixel_i(rPt);

    return Color();
}

sal_uInt32 BitmapDevice::getPixelData( const basegfx::B2IPoint& rPt )
{
    if( mpImpl->maLineClipRect.isInside(rPt) )
        return getPixelData_i(rPt);

    return 0;
}

void BitmapDevice::drawLine( const basegfx::B2IPoint& rPt1,
                             const basegfx::B2IPoint& rPt2,
                             Color                    lineColor,
                             DrawMode                 drawMode )
{
    drawLine_i( rPt1,
                rPt2,
                mpImpl->maLineClipRect,
                lineColor,
                drawMode );
}

void BitmapDevice::drawLine( const basegfx::B2IPoint&     rPt1,
                             const basegfx::B2IPoint&     rPt2,
                             Color                        lineColor,
                             DrawMode                     drawMode,
                             const BitmapDeviceSharedPtr& rClip )
{
    if( !rClip )
    {
        drawLine(rPt1,rPt2,lineColor,drawMode);
        return;
    }

    if( isCompatibleClipMask( rClip ) )
        drawLine_i( rPt1,
                    rPt2,
                    mpImpl->maLineClipRect,
                    lineColor,
                    drawMode,
                    rClip );
    else
        OSL_ENSURE( false, "Generic output not yet implemented!" );
}

void BitmapDevice::drawPolygon( const basegfx::B2DPolygon& rPoly,
                                Color                      lineColor,
                                DrawMode                   drawMode )
{
    const sal_uInt32 numVertices( rPoly.count() );
    if( numVertices )
        drawPolygon_i( rPoly,
                       mpImpl->maLineClipRect,
                       lineColor, drawMode );
}

void BitmapDevice::drawPolygon( const basegfx::B2DPolygon&   rPoly,
                                Color                        lineColor,
                                DrawMode                     drawMode,
                                const BitmapDeviceSharedPtr& rClip )
{
    if( !rClip )
    {
        drawPolygon(rPoly,lineColor,drawMode);
        return;
    }

    const sal_uInt32 numVertices( rPoly.count() );
    if( numVertices )
        if( isCompatibleClipMask( rClip ) )
            drawPolygon_i( rPoly,
                           mpImpl->maLineClipRect,
                           lineColor, drawMode, rClip );
        else
            OSL_ENSURE( false, "Generic output not yet implemented!" );
}

void BitmapDevice::fillPolyPolygon( const basegfx::B2DPolyPolygon& rPoly,
                                    Color                          fillColor,
                                    DrawMode                       drawMode )
{
    fillPolyPolygon_i( rPoly, fillColor, drawMode, mpImpl->maBounds );
}

void BitmapDevice::fillPolyPolygon( const basegfx::B2DPolyPolygon& rPoly,
                                    Color                          fillColor,
                                    DrawMode                       drawMode,
                                    const BitmapDeviceSharedPtr&   rClip )
{
    if( !rClip )
    {
        fillPolyPolygon(rPoly,fillColor,drawMode);
        return;
    }

    if( isCompatibleClipMask( rClip ) )
        fillPolyPolygon_i( rPoly, fillColor, drawMode, mpImpl->maBounds, rClip );
    else
        OSL_ENSURE( false, "Generic output not yet implemented!" );
}


namespace
{
    void assertImagePoint( const basegfx::B2IPoint& rPt,
                           const basegfx::B2IRange& rPermittedRange )
    {
        OSL_ASSERT( rPermittedRange.isInside(rPt) );
    }

    void assertImageRange( const basegfx::B2IRange& rRange,
                           const basegfx::B2IRange& rPermittedRange )
    {
#if OSL_DEBUG_LEVEL > 0
        basegfx::B2IRange aRange( rRange );
        aRange.intersect( rPermittedRange );

        OSL_ASSERT( aRange == rRange );
#endif
    }

    // TODO(Q3): Move canvas/canvastools.hxx clipBlit() down
    // to basegfx, and use here!
    bool clipAreaImpl( ::basegfx::B2IRange&       io_rSourceArea,
                       ::basegfx::B2IPoint&       io_rDestPoint,
                       const ::basegfx::B2IRange& rSourceBounds,
                       const ::basegfx::B2IRange& rDestBounds )
    {
        const ::basegfx::B2IPoint aSourceTopLeft(
            io_rSourceArea.getMinimum() );

        ::basegfx::B2IRange aLocalSourceArea( io_rSourceArea );

        // clip source area (which must be inside rSourceBounds)
        aLocalSourceArea.intersect( rSourceBounds );

        if( aLocalSourceArea.isEmpty() )
            return false;

        // calc relative new source area points (relative to orig
        // source area)
        const ::basegfx::B2IVector aUpperLeftOffset(
            aLocalSourceArea.getMinimum()-aSourceTopLeft );
        const ::basegfx::B2IVector aLowerRightOffset(
            aLocalSourceArea.getMaximum()-aSourceTopLeft );

        ::basegfx::B2IRange aLocalDestArea( io_rDestPoint + aUpperLeftOffset,
                                            io_rDestPoint + aLowerRightOffset );

        // clip dest area (which must be inside rDestBounds)
        aLocalDestArea.intersect( rDestBounds );

        if( aLocalDestArea.isEmpty() )
            return false;

        // calc relative new dest area points (relative to orig
        // source area)
        const ::basegfx::B2IVector aDestUpperLeftOffset(
            aLocalDestArea.getMinimum()-io_rDestPoint );
        const ::basegfx::B2IVector aDestLowerRightOffset(
            aLocalDestArea.getMaximum()-io_rDestPoint );

        io_rSourceArea = ::basegfx::B2IRange( aSourceTopLeft + aDestUpperLeftOffset,
                                              aSourceTopLeft + aDestLowerRightOffset );
        io_rDestPoint  = aLocalDestArea.getMinimum();

        return true;
    }

    // TODO(Q3): Move canvas/canvastools.hxx clipBlit() down
    // to basegfx, and use here!
    bool clipAreaImpl( ::basegfx::B2IRange&       io_rDestArea,
                       ::basegfx::B2IRange&       io_rSourceArea,
                       const ::basegfx::B2IRange& rDestBounds,
                       const ::basegfx::B2IRange& rSourceBounds )
    {
        // extract inherent scale
        const double nScaleX( io_rDestArea.getWidth() / (double)io_rSourceArea.getWidth() );
        const double nScaleY( io_rDestArea.getHeight() / (double)io_rSourceArea.getHeight() );

        // extract range origins
        const basegfx::B2IPoint   aDestTopLeft(
            io_rDestArea.getMinimum() );
        const ::basegfx::B2IPoint aSourceTopLeft(
            io_rSourceArea.getMinimum() );

        ::basegfx::B2IRange aLocalSourceArea( io_rSourceArea );

        // clip source area (which must be inside rSourceBounds)
        aLocalSourceArea.intersect( rSourceBounds );

        if( aLocalSourceArea.isEmpty() )
            return false;

        // calc relative new source area points (relative to orig
        // source area)
        const ::basegfx::B2IVector aUpperLeftOffset(
            aLocalSourceArea.getMinimum()-aSourceTopLeft );
        const ::basegfx::B2IVector aLowerRightOffset(
            aLocalSourceArea.getMaximum()-aSourceTopLeft );

        ::basegfx::B2IRange aLocalDestArea( basegfx::fround(aDestTopLeft.getX() + nScaleX*aUpperLeftOffset.getX()),
                                            basegfx::fround(aDestTopLeft.getY() + nScaleY*aUpperLeftOffset.getY()),
                                            basegfx::fround(aDestTopLeft.getX() + nScaleX*aLowerRightOffset.getX()),
                                            basegfx::fround(aDestTopLeft.getY() + nScaleY*aLowerRightOffset.getY()) );

        // clip dest area (which must be inside rDestBounds)
        aLocalDestArea.intersect( rDestBounds );

        if( aLocalDestArea.isEmpty() )
            return false;

        // calc relative new dest area points (relative to orig
        // source area)
        const ::basegfx::B2IVector aDestUpperLeftOffset(
            aLocalDestArea.getMinimum()-aDestTopLeft );
        const ::basegfx::B2IVector aDestLowerRightOffset(
            aLocalDestArea.getMaximum()-aDestTopLeft );

        io_rSourceArea = ::basegfx::B2IRange( basegfx::fround(aSourceTopLeft.getX() + aDestUpperLeftOffset.getX()/nScaleX),
                                              basegfx::fround(aSourceTopLeft.getY() + aDestUpperLeftOffset.getY()/nScaleY),
                                              basegfx::fround(aSourceTopLeft.getX() + aDestLowerRightOffset.getX()/nScaleX),
                                              basegfx::fround(aSourceTopLeft.getY() + aDestLowerRightOffset.getY()/nScaleY) );
        io_rDestArea   = aLocalDestArea;

        // final source area clip (chopping round-offs)
        io_rSourceArea.intersect( rSourceBounds );

        if( io_rSourceArea.isEmpty() )
            return false;


        return true;
    }
}

void BitmapDevice::drawBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                               const basegfx::B2IRange&     rSrcRect,
                               const basegfx::B2IRange&     rDstRect,
                               DrawMode                     drawMode )
{
    const basegfx::B2IVector& rSrcSize( rSrcBitmap->getSize() );
    const basegfx::B2IRange   aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IRange         aSrcRange( rSrcRect );
    basegfx::B2IRange         aDestRange( rDstRect );

    if( clipAreaImpl( aDestRange,
                      aSrcRange,
                      mpImpl->maBounds,
                      aSrcBounds ))
    {
        assertImageRange(aDestRange,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isCompatibleBitmap( rSrcBitmap ) )
            drawBitmap_i( rSrcBitmap, aSrcRange, aDestRange, drawMode );
        else
            OSL_ENSURE( false, "Generic output not yet implemented!" );
    }
}

void BitmapDevice::drawBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                               const basegfx::B2IRange&     rSrcRect,
                               const basegfx::B2IRange&     rDstRect,
                               DrawMode                     drawMode,
                               const BitmapDeviceSharedPtr& rClip )
{
    if( !rClip )
    {
        drawBitmap(rSrcBitmap,rSrcRect,rDstRect,drawMode);
        return;
    }

    const basegfx::B2IVector& rSrcSize( rSrcBitmap->getSize() );
    const basegfx::B2IRange   aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IRange         aSrcRange( rSrcRect );
    basegfx::B2IRange         aDestRange( rDstRect );

    if( clipAreaImpl( aDestRange,
                      aSrcRange,
                      mpImpl->maBounds,
                      aSrcBounds ))
    {
        assertImageRange(aDestRange,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isCompatibleBitmap( rSrcBitmap ) &&
            isCompatibleClipMask( rClip ) )
        {
            drawBitmap_i( rSrcBitmap, aSrcRange, aDestRange, drawMode, rClip );
        }
        else
        {
            OSL_ENSURE( false, "Generic output not yet implemented!" );
        }
    }
}

void BitmapDevice::drawMaskedColor( Color                        rSrcColor,
                                    const BitmapDeviceSharedPtr& rAlphaMask,
                                    const basegfx::B2IRange&     rSrcRect,
                                    const basegfx::B2IPoint&     rDstPoint )
{
    const basegfx::B2IVector& rSrcSize( rAlphaMask->getSize() );
    const basegfx::B2IRange   aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IRange         aSrcRange( rSrcRect );
    basegfx::B2IPoint         aDestPoint( rDstPoint );

    if( clipAreaImpl( aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
        assertImagePoint(aDestPoint,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isCompatibleAlphaMask( rAlphaMask ) )
            drawMaskedColor_i( rSrcColor, rAlphaMask, aSrcRange, aDestPoint );
        else
            OSL_ENSURE( false, "Generic output not yet implemented!" );
    }
}

void BitmapDevice::drawMaskedColor( Color                        aSrcColor,
                                    const BitmapDeviceSharedPtr& rAlphaMask,
                                    const basegfx::B2IRange&     rSrcRect,
                                    const basegfx::B2IPoint&     rDstPoint,
                                    const BitmapDeviceSharedPtr& rClip )
{
    if( !rClip )
    {
        drawMaskedColor(aSrcColor,rAlphaMask,rSrcRect,rDstPoint);
        return;
    }

    const basegfx::B2IVector& rSrcSize( rAlphaMask->getSize() );
    const basegfx::B2IRange   aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IRange         aSrcRange( rSrcRect );
    basegfx::B2IPoint         aDestPoint( rDstPoint );

    if( clipAreaImpl( aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
        assertImagePoint(aDestPoint,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isCompatibleAlphaMask( rAlphaMask ) &&
            isCompatibleClipMask( rClip ) )
        {
            drawMaskedColor_i( aSrcColor, rAlphaMask, aSrcRange, aDestPoint, rClip );
        }
        else
        {
            OSL_ENSURE( false, "Generic output not yet implemented!" );
        }
    }
}

void BitmapDevice::drawMaskedBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                                     const BitmapDeviceSharedPtr& rMask,
                                     const basegfx::B2IRange&     rSrcRect,
                                     const basegfx::B2IRange&     rDstRect,
                                     DrawMode                     drawMode )
{
    OSL_ASSERT( rMask->getSize() == rSrcBitmap->getSize() );

    const basegfx::B2IVector& rSrcSize( rSrcBitmap->getSize() );
    const basegfx::B2IRange   aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IRange         aSrcRange( rSrcRect );
    basegfx::B2IRange         aDestRange( rDstRect );

    if( clipAreaImpl( aDestRange,
                      aSrcRange,
                      mpImpl->maBounds,
                      aSrcBounds ))
    {
        assertImageRange(aDestRange,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isCompatibleBitmap( rSrcBitmap ) &&
            isCompatibleClipMask( rMask ) )
        {
            drawMaskedBitmap_i( rSrcBitmap, rMask, aSrcRange, aDestRange, drawMode );
        }
        else
        {
            OSL_ENSURE( false, "Generic output not yet implemented!" );
        }
    }
}

void BitmapDevice::drawMaskedBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                                     const BitmapDeviceSharedPtr& rMask,
                                     const basegfx::B2IRange&     rSrcRect,
                                     const basegfx::B2IRange&     rDstRect,
                                     DrawMode                     drawMode,
                                     const BitmapDeviceSharedPtr& rClip )
{
    if( !rClip )
    {
        drawMaskedBitmap(rSrcBitmap,rMask,rSrcRect,rDstRect,drawMode);
        return;
    }

    OSL_ASSERT( rMask->getSize() == rSrcBitmap->getSize() );

    const basegfx::B2IVector& rSrcSize( rSrcBitmap->getSize() );
    const basegfx::B2IRange   aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IRange         aSrcRange( rSrcRect );
    basegfx::B2IRange         aDestRange( rDstRect );

    if( clipAreaImpl( aDestRange,
                      aSrcRange,
                      mpImpl->maBounds,
                      aSrcBounds ))
    {
        assertImageRange(aDestRange,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isCompatibleBitmap( rSrcBitmap ) &&
            isCompatibleClipMask( rMask ) &&
            isCompatibleClipMask( rClip ) )
        {
            drawMaskedBitmap_i( rSrcBitmap, rMask, aSrcRange, aDestRange, drawMode, rClip );
        }
        else
        {
            OSL_ENSURE( false, "Generic output not yet implemented!" );
        }
    }
}


//----------------------------------------------------------------------------------

typedef BitmapRenderer<MaskIterator,
                       MaskAccessor >                   OneBitMsbMaskRenderer;
typedef BitmapRenderer<MaskIterator,PaletteAccessor>    OneBitMsbPaletteRenderer;
typedef BitmapRenderer<AlphaMaskIterator,
                       AlphaMaskAccessor >              EightBitGrayRenderer;
typedef BitmapRenderer<ThirtyTwoBitPixelIterator,
                       ThirtyTwoBitAccessor>            ThirtyTwoBitTrueColorRenderer;

namespace
{
BitmapDeviceSharedPtr createBitmapDeviceImpl( const basegfx::B2IVector&               rSize,
                                              bool                                    bTopDown,
                                              sal_Int32                               nScanlineFormat,
                                              boost::shared_array< sal_uInt8 >        pMem,
                                              PaletteMemorySharedVector               pPal )
{
    sal_Int32  nScanlineStride(0);

    // HACK: 1bpp and 32bpp only, currently
    if( nScanlineFormat == Format::ONE_BIT_MSB_PAL
        || nScanlineFormat == Format::ONE_BIT_MSB_GRAY )
        nScanlineStride = (rSize.getX() + 7) >> 3;
    else if( nScanlineFormat == Format::EIGHT_BIT_GRAY )
        nScanlineStride = rSize.getX();
    else if( nScanlineFormat == Format::THIRTYTWO_BIT_TC_MASK )
        nScanlineStride = 4*rSize.getX();

    nScanlineStride *= bTopDown ? 1 : -1;

    const std::size_t nMemSize(
        (nScanlineStride < 0 ? -nScanlineStride : nScanlineStride)*rSize.getY() );

    if( !pMem )
    {
        pMem.reset(
            reinterpret_cast<sal_uInt8*>(rtl_allocateMemory( nMemSize )),
            &rtl_freeMemory );
        rtl_zeroMemory(pMem.get(),nMemSize);
    }

    sal_uInt8* pFirstScanline = nScanlineStride < 0 ?
        pMem.get() + nMemSize + nScanlineStride : pMem.get();

    switch( nScanlineFormat )
    {
        case Format::ONE_BIT_MSB_GRAY:
        {
            return BitmapDeviceSharedPtr(
                new OneBitMsbMaskRenderer(
                    rSize,
                    bTopDown,
                    nScanlineFormat,
                    nScanlineStride,
                    MaskIterator(pFirstScanline,
                                 nScanlineStride),
                    MaskIterator(pFirstScanline,
                                 nScanlineStride)
                    + vigra::Diff2D(rSize.getX(),
                                    rSize.getY()),
                    MaskAccessor(),
                    pMem,
                    pPal ));
        }
        case Format::ONE_BIT_MSB_PAL:
        {
            if( !pPal )
            {
                boost::shared_ptr< std::vector<Color> > pLocalPal(
                    new std::vector<Color>(2) );
                pLocalPal->at(0) = Color(0x00000000);
                pLocalPal->at(1) = Color(0xFFFFFFFF);

                pPal = pLocalPal;
            }

            return BitmapDeviceSharedPtr(
                new OneBitMsbPaletteRenderer(
                    rSize,
                    bTopDown,
                    nScanlineFormat,
                    nScanlineStride,
                    MaskIterator(pFirstScanline,
                                 nScanlineStride),
                    MaskIterator(pFirstScanline,
                                 nScanlineStride)
                    + vigra::Diff2D(rSize.getX(),
                                    rSize.getY()),
                    PaletteAccessor( &pPal->at(0),
                                     pPal->size() ),
                    pMem,
                    pPal ));
        }

        case Format::EIGHT_BIT_GRAY:
        {
            return BitmapDeviceSharedPtr(
                new EightBitGrayRenderer(
                    rSize,
                    bTopDown,
                    nScanlineFormat,
                    nScanlineStride,
                    AlphaMaskIterator(pFirstScanline,
                                      nScanlineStride),
                    AlphaMaskIterator(pFirstScanline,
                                      nScanlineStride)
                    + vigra::Diff2D(rSize.getX(),
                                    rSize.getY()),
                    AlphaMaskAccessor(),
                    pMem,
                    pPal ));
        }

        case Format::THIRTYTWO_BIT_TC_MASK:
        {
            return BitmapDeviceSharedPtr(
                new ThirtyTwoBitTrueColorRenderer(
                    rSize,
                    bTopDown,
                    nScanlineFormat,
                    nScanlineStride,
                    ThirtyTwoBitPixelIterator(reinterpret_cast<Color*>(pFirstScanline),
                                              nScanlineStride),
                    ThirtyTwoBitPixelIterator(reinterpret_cast<Color*>(pFirstScanline),
                                              nScanlineStride)
                    + vigra::Diff2D(rSize.getX(),
                                    rSize.getY()),
                    ThirtyTwoBitAccessor(),
                    pMem,
                    pPal ));
        }

        default:
            // TODO(F3): other formats not yet implemented
            return BitmapDeviceSharedPtr();
    }
}
} // namespace


BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector& rSize,
                                          bool                      bTopDown,
                                          sal_Int32                 nScanlineFormat )
{
    return createBitmapDeviceImpl( rSize,
                                   bTopDown,
                                   nScanlineFormat,
                                   boost::shared_array< sal_uInt8 >(),
                                   PaletteMemorySharedVector() );
}

BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector&        rSize,
                                          bool                             bTopDown,
                                          sal_Int32                        nScanlineFormat,
                                          const RawMemorySharedArray&      rMem,
                                          const PaletteMemorySharedVector& rPalette )
{
    return createBitmapDeviceImpl( rSize,
                                   bTopDown,
                                   nScanlineFormat,
                                   rMem,
                                   rPalette );
}

BitmapDeviceSharedPtr cloneBitmapDevice( const basegfx::B2IVector&        rSize,
                                         const BitmapDeviceSharedPtr&     rProto )
{
    return createBitmapDeviceImpl( rSize,
                                   rProto->isTopDown(),
                                   rProto->getScanlineFormat(),
                                   boost::shared_array< sal_uInt8 >(),
                                   rProto->getPalette() );
}

} // namespace basebmp
