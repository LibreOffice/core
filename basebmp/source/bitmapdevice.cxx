/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bitmapdevice.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-12 15:09:44 $
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

#include "basebmp/compositeiterator.hxx"
#include "basebmp/iteratortraits.hxx"

#include "basebmp/accessor.hxx"
#include "basebmp/accessortraits.hxx"
#include "basebmp/accessoradapters.hxx"
#include "basebmp/colorblendaccessoradapter.hxx"

#include "basebmp/color.hxx"
#include "basebmp/colormisc.hxx"
#include "basebmp/colortraits.hxx"

#include "basebmp/greylevelformats.hxx"
#include "basebmp/paletteformats.hxx"
#include "basebmp/rgbmaskpixelformats.hxx"
#include "basebmp/rgb24pixelformats.hxx"

#include "basebmp/scanlineformats.hxx"
#include "basebmp/fillimage.hxx"
#include "basebmp/clippedlinerenderer.hxx"
//#include "basebmp/genericintegerimageaccessor.hxx"

#include "basebmp/tools.hxx"
#include "intconversion.hxx"

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

#include <vigra/iteratortraits.hxx>
#include <vigra/rgbvalue.hxx>
#include <vigra/resizeimage.hxx>
#include <vigra/copyimage.hxx>
#include <vigra/tuple.hxx>

#include <boost/mpl/identity.hpp>

namespace vigra
{
/// componentwise xor of an RGBValue (missing from rgbvalue.hxx)
template< class Value, unsigned int RedIndex, unsigned int BlueIndex, unsigned int GreenIndex >
inline RGBValue<Value, RedIndex, GreenIndex, BlueIndex>
operator^( RGBValue<Value, RedIndex, GreenIndex, BlueIndex> const& lhs,
           RGBValue<Value, RedIndex, GreenIndex, BlueIndex> const& rhs )
{
    RGBValue<Value, RedIndex, GreenIndex, BlueIndex> res(
        lhs[0] ^ rhs[0],
        lhs[1] ^ rhs[1],
        lhs[2] ^ rhs[2]);
    return res;
}
}

namespace basebmp
{

namespace
{
    // xxx TODO

    // TEMP TEMP TEMP

    class GenericImageAccessor
    {
    public:
        typedef Color value_type;

        BitmapDeviceSharedPtr mpDevice;

        explicit GenericImageAccessor( BitmapDeviceSharedPtr const& rTarget ) :
            mpDevice(rTarget)
        {}

        template< typename Iterator >
        Color operator()( Iterator const& i ) const
        { return mpDevice->getPixel( basegfx::B2IPoint( i->x,i->y ) ); }

        template< typename Iterator, typename Difference >
        Color operator()( Iterator const& i, Difference const& diff) const
        { return mpDevice->getPixel( basegfx::B2IPoint( i[diff]->x,
                                                        i[diff]->y ) ); }

        template< typename Iterator >
        void set(Color const& value, Iterator const& i) const
        { return mpDevice->setPixel( basegfx::B2IPoint( i->x,i->y ),
                                     value, DrawMode_PAINT ); }

        template< class Iterator, class Difference >
        void set(value_type const& value, Iterator const& i, Difference const& diff) const
        { return mpDevice->setPixel( basegfx::B2IPoint( i[diff]->x,
                                                        i[diff]->y ),
                                     value, DrawMode_PAINT ); }
    };



    /** Create the type for an accessor that takes the (mask,bitmap)
        input value generated from a JoinImageAccessorAdapter, and
        pipe that through a mask functor.

        @tpl DestAccessor
        Destination bitmap accessor

        @tpl JoinedAccessor
        Input accessor, is expected to generate a std::pair as the
        value type

        @tpl MaskFunctorMode
        Either FastMask or NoFastMask, depending on whether the mask
        is guaranteed to contain only 0s and 1s.
     */
    template< class    DestAccessor,
              class    JoinedAccessor,
              typename MaskFunctorMode > struct masked_input_splitting_accessor
    {
        typedef BinarySetterFunctionAccessorAdapter<
            DestAccessor,
            BinaryFunctorSplittingWrapper<
                typename outputMaskFunctorSelector<
                         typename JoinedAccessor::value_type::first_type,
                         typename JoinedAccessor::value_type::second_type,
                         MaskFunctorMode >::type > > type;
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
            typename vigra::IteratorTraits<DestIterator>::row_iterator
                rowIter( currIter.rowIterator() + nStartX);
            typename vigra::IteratorTraits<DestIterator>::row_iterator
                rowEnd( currIter.rowIterator() + nEndX );

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
        const DestIterator&                                     begin,
        const DestAccessor&                                     acc )
    {
        return std::auto_ptr< Renderer< DestIterator, DestAccessor > >(
            new Renderer< DestIterator, DestAccessor >(rPolyPolyRaster,
                                                       fillColor,
                                                       outRange,
                                                       begin,
                                                       acc));
    }


    // Actual BitmapDevice implementation (templatized by accessor and iterator)
    //--------------------------------------------------------------------------

    /** Implementation of the BitmapDevice interface

        @tpl DestIterator
        Iterator to access bitmap memory

        @tpl RawAccessor
        Raw accessor, to access pixel values directly

        @tpl AccessorSelector
        Accessor adapter selector, which, when applying the nested
        template metafunction wrap_accessor to one of the raw bitmap
        accessors, yields a member type named 'type', which is a
        wrapped accessor that map color values.
     */
    template< class DestIterator,
              class RawAccessor,
              class AccessorSelector,
              class Masks > class BitmapRenderer :
                  public BitmapDevice
    {
    public:
        typedef DestIterator                                               dest_iterator_type;
        typedef RawAccessor                                                raw_accessor_type;
        typedef AccessorSelector                                           accessor_selector;

        typedef typename Masks::clipmask_format_traits::iterator_type      mask_iterator_type;
        typedef typename Masks::clipmask_format_traits::raw_accessor_type  mask_rawaccessor_type;
        typedef typename Masks::clipmask_format_traits::accessor_selector  mask_accessorselector_type;

        typedef typename Masks::alphamask_format_traits::iterator_type     alphamask_iterator_type;
        typedef typename Masks::alphamask_format_traits::raw_accessor_type alphamask_rawaccessor_type;
        typedef typename Masks::alphamask_format_traits::accessor_selector alphamask_accessorselector_type;

        typedef typename AccessorSelector::template wrap_accessor<
            raw_accessor_type >::type                                      dest_accessor_type;

        typedef AccessorTraits< dest_accessor_type >                       accessor_traits;
        typedef CompositeIterator2D< dest_iterator_type,
                                     mask_iterator_type >                  composite_iterator_type;
        typedef CompositeIterator2D< vigra::Diff2D,
                                     vigra::Diff2D >                       generic_composite_iterator_type;

        typedef BitmapRenderer<mask_iterator_type,
                               mask_rawaccessor_type,
                               mask_accessorselector_type,
                               Masks>                                      mask_bitmap_type;
        typedef BitmapRenderer<alphamask_iterator_type,
                               alphamask_rawaccessor_type,
                               alphamask_accessorselector_type,
                               Masks>                                      alphamask_bitmap_type;

        // -------------------------------------------------------

        typedef AccessorTraits< raw_accessor_type >                        raw_accessor_traits;
        typedef typename uInt32Converter<
            typename raw_accessor_type::value_type>::to                    to_uint32_functor;

        // -------------------------------------------------------

        typedef typename raw_accessor_traits::xor_accessor                 raw_xor_accessor_type;
        typedef AccessorTraits<raw_xor_accessor_type>                      raw_xor_accessor_traits;
        typedef typename accessor_selector::template wrap_accessor<
            raw_xor_accessor_type >::type                                  xor_accessor_type;
        typedef AccessorTraits<xor_accessor_type>                          xor_accessor_traits;

        // -------------------------------------------------------

        typedef typename raw_accessor_traits::template masked_accessor<
            mask_rawaccessor_type,
            dest_iterator_type,
            mask_iterator_type>::type                                      raw_maskedaccessor_type;
        typedef typename accessor_selector::template wrap_accessor<
            raw_maskedaccessor_type >::type                                masked_accessor_type;
        typedef typename raw_xor_accessor_traits::template masked_accessor<
            mask_rawaccessor_type,
            dest_iterator_type,
            mask_iterator_type>::type                                      raw_maskedxor_accessor_type;
        typedef typename accessor_selector::template wrap_accessor<
            raw_maskedxor_accessor_type >::type                            masked_xoraccessor_type;

        // -------------------------------------------------------

        // ((iter,mask),mask) special case (e.g. for clipped
        // drawMaskedColor())
        typedef AccessorTraits< raw_maskedaccessor_type >                  raw_maskedaccessor_traits;
        typedef typename raw_maskedaccessor_traits::template masked_accessor<
            mask_rawaccessor_type,
            composite_iterator_type,
            mask_iterator_type>::type                                      raw_maskedmask_accessor_type;

        typedef CompositeIterator2D<
            composite_iterator_type,
            mask_iterator_type>                                            composite_composite_mask_iterator_type;

        // -------------------------------------------------------

        typedef ConstantColorBlendSetterAccessorAdapter<
            dest_accessor_type,
            typename alphamask_rawaccessor_type::value_type>               colorblend_accessor_type;
        typedef AccessorTraits<colorblend_accessor_type>                   colorblend_accessor_traits;
        typedef typename colorblend_accessor_traits::template masked_accessor<
            mask_rawaccessor_type,
            dest_iterator_type,
            mask_iterator_type>::type                                      masked_colorblend_accessor_type;

        // -------------------------------------------------------

        typedef JoinImageAccessorAdapter< dest_accessor_type,
                                          mask_rawaccessor_type >          joined_image_accessor_type;
        typedef JoinImageAccessorAdapter< GenericImageAccessor,
                                          GenericImageAccessor >           joined_generic_image_accessor_type;

        // -------------------------------------------------------

        dest_iterator_type                      maBegin;
        typename accessor_traits::color_lookup  maColorLookup;
        to_uint32_functor                       maToUInt32Converter;
        dest_accessor_type                      maAccessor;
        colorblend_accessor_type                maColorBlendAccessor;
        raw_accessor_type                       maRawAccessor;
        xor_accessor_type                       maXorAccessor;
        raw_xor_accessor_type                   maRawXorAccessor;
        masked_accessor_type                    maMaskedAccessor;
        masked_colorblend_accessor_type         maMaskedColorBlendAccessor;
        masked_xoraccessor_type                 maMaskedXorAccessor;
        raw_maskedaccessor_type                 maRawMaskedAccessor;
        raw_maskedxor_accessor_type             maRawMaskedXorAccessor;
        raw_maskedmask_accessor_type            maRawMaskedMaskAccessor;

        BitmapRenderer( const basegfx::B2IRange&         rBounds,
                        bool                             bTopDown,
                        sal_Int32                        nScanlineFormat,
                        sal_Int32                        nScanlineStride,
                        sal_uInt8*                       pFirstScanline,
                        dest_iterator_type               begin,
                        dest_accessor_type               accessor,
                        const RawMemorySharedArray&      rMem,
                        const PaletteMemorySharedVector& rPalette ) :
            BitmapDevice( rBounds, bTopDown, nScanlineFormat,
                          nScanlineStride, pFirstScanline, rMem, rPalette ),
            maBegin( begin ),
            maColorLookup(),
            maToUInt32Converter(),
            maAccessor( accessor ),
            maColorBlendAccessor( accessor ),
            maRawAccessor(),
            maXorAccessor( accessor ),
            maRawXorAccessor(),
            maMaskedAccessor( accessor ),
            maMaskedColorBlendAccessor( maColorBlendAccessor ),
            maMaskedXorAccessor( accessor ),
            maRawMaskedAccessor(),
            maRawMaskedXorAccessor(),
            maRawMaskedMaskAccessor()
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

        boost::shared_ptr<mask_bitmap_type> getCompatibleClipMask( const BitmapDeviceSharedPtr& bmp ) const
        {
            boost::shared_ptr<mask_bitmap_type> pMask( boost::dynamic_pointer_cast<mask_bitmap_type>( bmp ));

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
            return boost::dynamic_pointer_cast<mask_bitmap_type>( bmp ).get() != NULL;
        }

        boost::shared_ptr<alphamask_bitmap_type> getCompatibleAlphaMask( const BitmapDeviceSharedPtr& bmp ) const
        {
            return boost::dynamic_pointer_cast<alphamask_bitmap_type>( bmp );
        }

        virtual bool isCompatibleAlphaMask( const BitmapDeviceSharedPtr& bmp ) const
        {
            // TODO(P1): dynamic_cast usually called twice for
            // compatible formats
            return getCompatibleAlphaMask( bmp ).get() != NULL;
        }

        virtual void clear_i( Color                     fillColor,
                              const basegfx::B2IRange&  rBounds )
        {
            fillImage(destIterRange(maBegin,
                                    maRawAccessor,
                                    rBounds),
                      maColorLookup(
                          maAccessor,
                          fillColor) );
        }

        virtual void setPixel_i( const basegfx::B2IPoint& rPt,
                                 Color                    pixelColor,
                                 DrawMode                 drawMode )
        {
            const DestIterator pixel( maBegin +
                                      vigra::Diff2D(rPt.getX(),
                                                    rPt.getY()) );
            if( drawMode == DrawMode_XOR )
                maXorAccessor.set( pixelColor,
                                   pixel );
            else
                maAccessor.set( pixelColor,
                                pixel );
        }

        virtual void setPixel_i( const basegfx::B2IPoint&     rPt,
                                 Color                        pixelColor,
                                 DrawMode                     drawMode,
                                 const BitmapDeviceSharedPtr& rClip )
        {
            boost::shared_ptr<mask_bitmap_type> pMask( getCompatibleClipMask(rClip) );
            OSL_ASSERT( pMask );

            const vigra::Diff2D offset(rPt.getX(),
                                       rPt.getY());

            const composite_iterator_type aIter(
                maBegin + offset,
                pMask->maBegin + offset );

            if( drawMode == DrawMode_XOR )
                maMaskedXorAccessor.set( pixelColor,
                                         aIter );
            else
                maMaskedAccessor.set( pixelColor,
                                      aIter );
        }

        virtual Color getPixel_i(const basegfx::B2IPoint& rPt )
        {
            const DestIterator pixel( maBegin +
                                      vigra::Diff2D(rPt.getX(),
                                                    rPt.getY()) );
            return maAccessor(pixel);
        }

        virtual sal_uInt32 getPixelData_i( const basegfx::B2IPoint& rPt )
        {
            const DestIterator pixel( maBegin +
                                      vigra::Diff2D(rPt.getX(),
                                                    rPt.getY()) );
            return maToUInt32Converter(maRawAccessor(pixel));
        }

        template< typename Iterator, typename Col, typename RawAcc >
        void implRenderLine2( const basegfx::B2IPoint& rPt1,
                              const basegfx::B2IPoint& rPt2,
                              const basegfx::B2IRange& rBounds,
                              Col                      col,
                              const Iterator&          begin,
                              const RawAcc&            rawAcc )
        {
            renderClippedLine( rPt1,
                               rPt2,
                               rBounds,
                               col,
                               begin,
                               rawAcc );
        }

        template< typename Iterator, typename Accessor, typename RawAcc >
        void implRenderLine( const basegfx::B2IPoint& rPt1,
                             const basegfx::B2IPoint& rPt2,
                             const basegfx::B2IRange& rBounds,
                             Color                    col,
                             const Iterator&          begin,
                             const Accessor&          acc,
                             const RawAcc&            rawAcc )
        {
            implRenderLine2( rPt1,rPt2,rBounds,
                             maColorLookup( acc,
                                            col ),
                             begin,
                             rawAcc );
        }

        template< typename Iterator, typename RawAcc, typename XorAcc >
        void implDrawLine( const basegfx::B2IPoint& rPt1,
                           const basegfx::B2IPoint& rPt2,
                           const basegfx::B2IRange& rBounds,
                           Color                    col,
                           const Iterator&          begin,
                           const RawAcc&            rawAcc,
                           const XorAcc&            xorAcc,
                           DrawMode                 drawMode )
        {
            if( drawMode == DrawMode_XOR )
                implRenderLine( rPt1, rPt2, rBounds, col,
                                begin, maAccessor, xorAcc );
            else
                implRenderLine( rPt1, rPt2, rBounds, col,
                                begin, maAccessor, rawAcc );
        }

        virtual void drawLine_i(const basegfx::B2IPoint& rPt1,
                                const basegfx::B2IPoint& rPt2,
                                const basegfx::B2IRange& rBounds,
                                Color                    lineColor,
                                DrawMode                 drawMode )
        {
            implDrawLine(rPt1,rPt2,rBounds,lineColor,
                         maBegin,
                         maRawAccessor,maRawXorAccessor,drawMode);
        }

        composite_iterator_type getMaskedIter( const BitmapDeviceSharedPtr& rClip ) const
        {
            boost::shared_ptr<mask_bitmap_type> pMask( getCompatibleClipMask(rClip) );
            OSL_ASSERT( pMask );

            return composite_iterator_type( maBegin,
                                            pMask->maBegin );
        }

        virtual void drawLine_i(const basegfx::B2IPoint&     rPt1,
                                const basegfx::B2IPoint&     rPt2,
                                const basegfx::B2IRange&     rBounds,
                                Color                        lineColor,
                                DrawMode                     drawMode,
                                const BitmapDeviceSharedPtr& rClip )
        {
            implDrawLine(rPt1,rPt2,rBounds,lineColor,
                         getMaskedIter(rClip),
                         maRawMaskedAccessor,
                         maRawMaskedXorAccessor,drawMode);
        }

        template< typename Iterator, typename RawAcc >
        void implDrawPolygon( const basegfx::B2DPolygon& rPoly,
                              const basegfx::B2IRange&   rBounds,
                              Color                      col,
                              const Iterator&            begin,
                              const RawAcc&              acc )
        {
            basegfx::B2DPolygon aPoly( rPoly );
            if( rPoly.areControlVectorsUsed() )
                aPoly = basegfx::tools::adaptiveSubdivideByCount( rPoly );

            const typename dest_iterator_type::value_type colorIndex( maColorLookup(
                                                                          maAccessor,
                                                                          col));
            const sal_uInt32                              nVertices( aPoly.count() );
            for( sal_uInt32 i=1; i<nVertices; ++i )
                implRenderLine2( basegfx::fround(aPoly.getB2DPoint(i-1)),
                                 basegfx::fround(aPoly.getB2DPoint(i)),
                                 rBounds,
                                 colorIndex,
                                 begin,
                                 acc );

            if( nVertices > 1 && aPoly.isClosed() )
                implRenderLine2( basegfx::fround(aPoly.getB2DPoint(nVertices-1)),
                                 basegfx::fround(aPoly.getB2DPoint(0)),
                                 rBounds,
                                 colorIndex,
                                 begin,
                                 acc );
        }

        virtual void drawPolygon_i(const basegfx::B2DPolygon& rPoly,
                                   const basegfx::B2IRange&   rBounds,
                                   Color                      lineColor,
                                   DrawMode                   drawMode )
        {
            if( drawMode == DrawMode_XOR )
                implDrawPolygon( rPoly, rBounds, lineColor,
                                 maBegin,
                                 maRawXorAccessor );
            else
                implDrawPolygon( rPoly, rBounds, lineColor,
                                 maBegin,
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
                                 getMaskedIter(rClip),
                                 maRawMaskedXorAccessor );
            else
                implDrawPolygon( rPoly, rBounds, lineColor,
                                 getMaskedIter(rClip),
                                 maRawMaskedAccessor );
        }

        template< typename Iterator, typename RawAcc >
        void implFillPolyPolygon( const basegfx::B2DPolyPolygon& rPoly,
                                  Color                          col,
                                  const Iterator&                begin,
                                  const RawAcc&                  acc,
                                  const basegfx::B2IRange&       rBounds )
        {
            basegfx::B2DPolyPolygon aPoly( rPoly );
            if( rPoly.areControlVectorsUsed() )
                aPoly = basegfx::tools::adaptiveSubdivideByCount( rPoly );

            makeRenderer( aPoly,
                          maColorLookup( maAccessor,
                                         col),
                          rBounds,
                          begin,
                          acc )->rasterConvert(
                              basegfx::FillRule_NONZERO_WINDING_NUMBER );
        }

        virtual void fillPolyPolygon_i(const basegfx::B2DPolyPolygon& rPoly,
                                       Color                          fillColor,
                                       DrawMode                       drawMode,
                                       const basegfx::B2IRange&       rBounds )
        {
            if( drawMode == DrawMode_XOR )
                implFillPolyPolygon( rPoly, fillColor,
                                     maBegin,
                                     maRawXorAccessor,
                                     rBounds );
            else
                implFillPolyPolygon( rPoly, fillColor,
                                     maBegin,
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
                                     getMaskedIter(rClip),
                                     maRawMaskedXorAccessor,
                                     rBounds );
            else
                implFillPolyPolygon( rPoly, fillColor,
                                     getMaskedIter(rClip),
                                     maRawMaskedAccessor,
                                     rBounds );
        }

        template< typename Iterator, typename RawAcc >
        void implDrawBitmap(const BitmapDeviceSharedPtr& rSrcBitmap,
                            const basegfx::B2IRange&     rSrcRect,
                            const basegfx::B2IRange&     rDstRect,
                            const Iterator&              begin,
                            const RawAcc&                acc)
        {
            boost::shared_ptr<BitmapRenderer> pSrcBmp( getCompatibleBitmap(rSrcBitmap) );
            OSL_ASSERT( pSrcBmp );

            // since resizeImageNoInterpolation() internally copyies
            // to a temporary buffer, also works with *this == rSrcBitmap
            vigra::resizeImageNoInterpolation(
                srcIterRange(pSrcBmp->maBegin,
                             pSrcBmp->maRawAccessor,
                             rSrcRect),
                destIterRange(begin,
                              acc,
                              rDstRect));
        }

        // xxx TODO
        template< typename Iterator, typename Acc >
        void implDrawBitmapGeneric(const BitmapDeviceSharedPtr& rSrcBitmap,
                                   const basegfx::B2IRange&     rSrcRect,
                                   const basegfx::B2IRange&     rDstRect,
                                   const Iterator&              begin,
                                   const Acc&                   acc)
        {
            GenericImageAccessor aSrcAcc( rSrcBitmap );

            // since resizeImageNoInterpolation() internally copyies
            // to a temporary buffer, also works with *this == rSrcBitmap
            vigra::resizeImageNoInterpolation(
                srcIterRange(vigra::Diff2D(),
                             aSrcAcc,
                             rSrcRect),
                destIterRange(begin,
                              acc,
                              rDstRect));
        }

        virtual void drawBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                  const basegfx::B2IRange&     rSrcRect,
                                  const basegfx::B2IRange&     rDstRect,
                                  DrawMode                     drawMode )
        {
            // xxx TODO
            if( isCompatibleBitmap( rSrcBitmap ) )
            {
                if( drawMode == DrawMode_XOR )
                    implDrawBitmap(rSrcBitmap, rSrcRect, rDstRect,
                                   maBegin,
                                   maRawXorAccessor);
                else
                    implDrawBitmap(rSrcBitmap, rSrcRect, rDstRect,
                                   maBegin,
                                   maRawAccessor);
            }
            else
            {
                if( drawMode == DrawMode_XOR )
                    implDrawBitmapGeneric(rSrcBitmap, rSrcRect, rDstRect,
                                          maBegin,
                                          maXorAccessor);
                else
                    implDrawBitmapGeneric(rSrcBitmap, rSrcRect, rDstRect,
                                          maBegin,
                                          maAccessor);
            }
        }

        virtual void drawBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                  const basegfx::B2IRange&     rSrcRect,
                                  const basegfx::B2IRange&     rDstRect,
                                  DrawMode                     drawMode,
                                  const BitmapDeviceSharedPtr& rClip )
        {
            // xxx TODO
            if( isCompatibleBitmap( rSrcBitmap ) )
            {
                if( drawMode == DrawMode_XOR )
                    implDrawBitmap(rSrcBitmap, rSrcRect, rDstRect,
                                   getMaskedIter(rClip),
                                   maRawMaskedXorAccessor);
                else
                    implDrawBitmap(rSrcBitmap, rSrcRect, rDstRect,
                                   getMaskedIter(rClip),
                                   maRawMaskedAccessor);
            }
            else
            {
                if( drawMode == DrawMode_XOR )
                    implDrawBitmapGeneric(rSrcBitmap, rSrcRect, rDstRect,
                                          getMaskedIter(rClip),
                                          maMaskedXorAccessor);
                else
                    implDrawBitmapGeneric(rSrcBitmap, rSrcRect, rDstRect,
                                          getMaskedIter(rClip),
                                          maMaskedAccessor);
            }
        }

        virtual void drawMaskedColor_i(Color                        aSrcColor,
                                       const BitmapDeviceSharedPtr& rAlphaMask,
                                       const basegfx::B2IRange&     rSrcRect,
                                       const basegfx::B2IPoint&     rDstPoint )
        {
            boost::shared_ptr<mask_bitmap_type>      pMask( getCompatibleClipMask(rAlphaMask) );
            boost::shared_ptr<alphamask_bitmap_type> pAlpha( getCompatibleAlphaMask(rAlphaMask) );
            OSL_ASSERT( pAlpha || pMask );

            if( pAlpha )
            {
                maColorBlendAccessor.setColor( aSrcColor );

                vigra::copyImage( srcIterRange(pAlpha->maBegin,
                                               pAlpha->maRawAccessor,
                                               rSrcRect),
                                  destIter(maBegin,
                                           maColorBlendAccessor,
                                           rDstPoint) );
            }
            else if( pMask )
            {
                const composite_iterator_type aBegin(
                    maBegin + vigra::Diff2D(rDstPoint.getX(),
                                            rDstPoint.getY()),
                    pMask->maBegin + topLeft(rSrcRect) );

                fillImage(aBegin,
                          aBegin + vigra::Diff2D(rSrcRect.getWidth(),
                                                 rSrcRect.getHeight()),
                          maRawMaskedAccessor,
                          maColorLookup(
                              maAccessor,
                              aSrcColor) );
            }
        }

        virtual void drawMaskedColor_i(Color                        aSrcColor,
                                       const BitmapDeviceSharedPtr& rAlphaMask,
                                       const basegfx::B2IRange&     rSrcRect,
                                       const basegfx::B2IPoint&     rDstPoint,
                                       const BitmapDeviceSharedPtr& rClip )
        {
            boost::shared_ptr<mask_bitmap_type>      pMask( getCompatibleClipMask(rAlphaMask) );
            boost::shared_ptr<alphamask_bitmap_type> pAlpha( getCompatibleAlphaMask(rAlphaMask) );
            OSL_ASSERT( pAlpha || pMask );

            if( pAlpha )
            {
                const composite_iterator_type aBegin( getMaskedIter(rClip) );
                maMaskedColorBlendAccessor.get1stWrappedAccessor().setColor(
                    aSrcColor );

                vigra::copyImage( srcIterRange(pAlpha->maBegin,
                                               pAlpha->maRawAccessor,
                                               rSrcRect),
                                  destIter(aBegin,
                                           maMaskedColorBlendAccessor,
                                           rDstPoint) );
            }
            else if( pMask )
            {
                boost::shared_ptr<mask_bitmap_type> pClipMask( getCompatibleClipMask(rClip) );
                OSL_ASSERT( pClipMask );

                // setup a ((iter,mask),clipMask) composite composite
                // iterator, to pass both masks (clip and alpha mask)
                // to the algorithm
                const composite_composite_mask_iterator_type aBegin(
                    composite_iterator_type(
                        maBegin + vigra::Diff2D(rDstPoint.getX(),
                                                rDstPoint.getY()),
                        pMask->maBegin + topLeft(rSrcRect)),
                    pClipMask->maBegin + vigra::Diff2D(rDstPoint.getX(),
                                                       rDstPoint.getY()) );

                fillImage(aBegin,
                          aBegin + vigra::Diff2D(rSrcRect.getWidth(),
                                                 rSrcRect.getHeight()),
                          maRawMaskedMaskAccessor,
                          maColorLookup(
                              maAccessor,
                              aSrcColor) );
            }
        }

        template< typename Iterator, typename Acc >
        void implDrawMaskedBitmap(const BitmapDeviceSharedPtr& rSrcBitmap,
                                  const BitmapDeviceSharedPtr& rMask,
                                  const basegfx::B2IRange&     rSrcRect,
                                  const basegfx::B2IRange&     rDstRect,
                                  const Iterator&              begin,
                                  const Acc&                   acc)
        {
            boost::shared_ptr<BitmapRenderer>   pSrcBmp( getCompatibleBitmap(rSrcBitmap) );
            boost::shared_ptr<mask_bitmap_type> pMask( getCompatibleClipMask(rMask) );
            OSL_ASSERT( pMask && pSrcBmp );

            // since resizeImageNoInterpolation() internally copyies
            // to a temporary buffer, also works with *this == rSrcBitmap
            vigra::resizeImageNoInterpolation(
                srcIterRange(composite_iterator_type(
                                 pSrcBmp->maBegin,
                                 pMask->maBegin),
                             joined_image_accessor_type(
                                 pSrcBmp->maAccessor,
                                 pMask->maRawAccessor),
                             rSrcRect),
                destIterRange(begin,
                              typename masked_input_splitting_accessor<
                                       Acc,
                                       joined_image_accessor_type,
                                       FastMask >::type(acc),
                              rDstRect));
        }

        // xxx TODO
        template< typename Iterator, typename Acc >
        void implDrawMaskedBitmapGeneric(const BitmapDeviceSharedPtr& rSrcBitmap,
                                         const BitmapDeviceSharedPtr& rMask,
                                         const basegfx::B2IRange&     rSrcRect,
                                         const basegfx::B2IRange&     rDstRect,
                                         const Iterator&              begin,
                                         const Acc&                   acc)
        {
            GenericImageAccessor aSrcAcc( rSrcBitmap );
            GenericImageAccessor aMaskAcc( rMask );

            const vigra::Diff2D aTopLeft(rSrcRect.getMinX(),
                                         rSrcRect.getMinY());
            const vigra::Diff2D aBottomRight(rSrcRect.getMaxX(),
                                             rSrcRect.getMaxY());
            // since resizeImageNoInterpolation() internally copyies
            // to a temporary buffer, also works with *this == rSrcBitmap
            vigra::resizeImageNoInterpolation(
                vigra::make_triple(
                    generic_composite_iterator_type(
                        aTopLeft,aTopLeft),
                    generic_composite_iterator_type(
                        aBottomRight,aBottomRight),
                    joined_generic_image_accessor_type(
                        aSrcAcc,
                        aMaskAcc)),
                destIterRange(begin,
                              typename masked_input_splitting_accessor<
                                       Acc,
                                       joined_generic_image_accessor_type,
                                       NoFastMask >::type(acc),
                              rDstRect));
        }

        virtual void drawMaskedBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                        const BitmapDeviceSharedPtr& rMask,
                                        const basegfx::B2IRange&     rSrcRect,
                                        const basegfx::B2IRange&     rDstRect,
                                        DrawMode                     drawMode )
        {
            // xxx TODO
            if( isCompatibleClipMask(rMask) &&
                isCompatibleBitmap(rSrcBitmap) )
            {
                if( drawMode == DrawMode_XOR )
                    implDrawMaskedBitmap(rSrcBitmap, rMask,
                                         rSrcRect, rDstRect,
                                         maBegin,
                                         maXorAccessor);
                else
                    implDrawMaskedBitmap(rSrcBitmap, rMask,
                                         rSrcRect, rDstRect,
                                         maBegin,
                                         maAccessor);
            }
            else
            {
                if( drawMode == DrawMode_XOR )
                    implDrawMaskedBitmapGeneric(rSrcBitmap, rMask,
                                                rSrcRect, rDstRect,
                                                maBegin,
                                                maXorAccessor);
                else
                    implDrawMaskedBitmapGeneric(rSrcBitmap, rMask,
                                                rSrcRect, rDstRect,
                                                maBegin,
                                                maAccessor);
            }
        }

        virtual void drawMaskedBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                        const BitmapDeviceSharedPtr& rMask,
                                        const basegfx::B2IRange&     rSrcRect,
                                        const basegfx::B2IRange&     rDstRect,
                                        DrawMode                     drawMode,
                                        const BitmapDeviceSharedPtr& rClip )
        {
            // xxx TODO
            if( isCompatibleClipMask(rClip) &&
                isCompatibleBitmap(rSrcBitmap) )
            {
                if( drawMode == DrawMode_XOR )
                    implDrawMaskedBitmap(rSrcBitmap, rMask,
                                         rSrcRect, rDstRect,
                                         getMaskedIter(rClip),
                                         maMaskedXorAccessor);
                else
                    implDrawMaskedBitmap(rSrcBitmap, rMask,
                                         rSrcRect, rDstRect,
                                         getMaskedIter(rClip),
                                         maMaskedAccessor);
            }
            else
            {
                if( drawMode == DrawMode_XOR )
                    implDrawMaskedBitmapGeneric(rSrcBitmap, rMask,
                                                rSrcRect, rDstRect,
                                                getMaskedIter(rClip),
                                                maMaskedXorAccessor);
                else
                    implDrawMaskedBitmapGeneric(rSrcBitmap, rMask,
                                                rSrcRect, rDstRect,
                                                getMaskedIter(rClip),
                                                maMaskedAccessor);
            }
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

    /// raw ptr to 0th scanline. used for cloning a generic renderer
    sal_uInt8*                mpFirstScanline;

    /** (Optional) device sharing the same memory, and used for input
        clip masks/alpha masks/bitmaps that don't match our exact
        bitmap format.
     */
    BitmapDeviceSharedPtr     mpGenericRenderer;
};


BitmapDevice::BitmapDevice( const basegfx::B2IRange&         rBounds,
                            bool                             bTopDown,
                            sal_Int32                        nScanlineFormat,
                            sal_Int32                        nScanlineStride,
                            sal_uInt8*                       pFirstScanline,
                            const RawMemorySharedArray&      rMem,
                            const PaletteMemorySharedVector& rPalette ) :
    mpImpl( new ImplBitmapDevice )
{
    mpImpl->mpMem = rMem;
    mpImpl->mpPalette = rPalette;
    mpImpl->maBounds = rBounds;
    mpImpl->maLineClipRect = basegfx::B2IRange( rBounds.getMinX(),
                                                rBounds.getMinY(),
                                                rBounds.getMaxX()-1,
                                                rBounds.getMaxY()-1 );
    mpImpl->mnScanlineFormat = nScanlineFormat;
    mpImpl->mnScanlineStride = nScanlineStride;
    mpImpl->mpFirstScanline  = pFirstScanline;
}

BitmapDevice::~BitmapDevice()
{
    // outline, because of internal ImplBitmapDevice
}

basegfx::B2IVector BitmapDevice::getSize() const
{

    return basegfx::B2IVector(
        mpImpl->maBounds.getMaxX() - mpImpl->maBounds.getMinX(),
        mpImpl->maBounds.getMaxY() - mpImpl->maBounds.getMinY() );
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
    return mpImpl->mnScanlineStride < 0 ?
        -mpImpl->mnScanlineStride : mpImpl->mnScanlineStride;
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
    clear_i( fillColor, mpImpl->maBounds );
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
            getGenericRenderer()->setPixel( rPt, lineColor, drawMode, rClip );
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
        getGenericRenderer()->drawLine( rPt1, rPt2, lineColor,
                                        drawMode, rClip );
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
            getGenericRenderer()->drawPolygon( rPoly, lineColor,
                                               drawMode, rClip );
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
        getGenericRenderer()->fillPolyPolygon( rPoly, fillColor,
                                               drawMode, rClip );
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

#if 0
        if( isCompatibleBitmap( rSrcBitmap ) )
            drawBitmap_i( rSrcBitmap, aSrcRange, aDestRange, drawMode );
        else
            getGenericRenderer()->drawBitmap( rSrcBitmap, rSrcRect,
                                              rDstRect, drawMode );
#else
        drawBitmap_i( rSrcBitmap, aSrcRange, aDestRange, drawMode );
#endif
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

#if 0
        if( isCompatibleBitmap( rSrcBitmap ) &&
            isCompatibleClipMask( rClip ) )
        {
            drawBitmap_i( rSrcBitmap, aSrcRange, aDestRange, drawMode, rClip );
        }
        else
        {
            getGenericRenderer()->drawBitmap( rSrcBitmap, rSrcRect,
                                              rDstRect, drawMode, rClip );
        }
#else
        if( isCompatibleClipMask( rClip ) )
        {
            drawBitmap_i( rSrcBitmap, aSrcRange, aDestRange, drawMode, rClip );
        }
        else
        {
            getGenericRenderer()->drawBitmap( rSrcBitmap, rSrcRect,
                                              rDstRect, drawMode, rClip );
        }
#endif
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

#if 0
    if( clipAreaImpl( aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
        assertImagePoint(aDestPoint,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isCompatibleClipMask( rAlphaMask ) || isCompatibleAlphaMask( rAlphaMask ) )
            drawMaskedColor_i( rSrcColor, rAlphaMask, aSrcRange, aDestPoint );
        else
            getGenericRenderer()->drawMaskedColor( rSrcColor, rAlphaMask,
                                                   rSrcRect, rDstPoint );
    }
#else
    // drawMaskedBitmap is also used for OutputDevice::DrawMask
    if( clipAreaImpl( aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
        assertImagePoint(aDestPoint,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isCompatibleClipMask( rAlphaMask ) || isCompatibleAlphaMask( rAlphaMask ) )
            drawMaskedColor_i( rSrcColor, rAlphaMask, aSrcRange, aDestPoint );
        else
            OSL_ENSURE(false, "Generic output not yet implemented!");
    }
#endif
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

#if 0
        if( (isCompatibleClipMask( rAlphaMask ) ||
             isCompatibleAlphaMask( rAlphaMask )) &&
            isCompatibleClipMask( rClip ) )
        {
            drawMaskedColor_i( aSrcColor, rAlphaMask, aSrcRange, aDestPoint, rClip );
        }
        else
        {
            getGenericRenderer()->drawMaskedColor( aSrcColor, rAlphaMask,
                                                   rSrcRect, rDstPoint, rClip );
        }
#else
        // drawMaskedBitmap is also used for OutputDevice::DrawMask
        if( (isCompatibleClipMask( rAlphaMask ) ||
             isCompatibleAlphaMask( rAlphaMask )) &&
            isCompatibleClipMask( rClip ) )
        {
            drawMaskedColor_i( aSrcColor, rAlphaMask, aSrcRange, aDestPoint, rClip );
        }
        else
        {
            OSL_ENSURE(false, "Generic output not yet implemented!");
        }
#endif
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

#if 0
        if( isCompatibleBitmap( rSrcBitmap ) &&
            isCompatibleClipMask( rMask ) )
        {
            drawMaskedBitmap_i( rSrcBitmap, rMask, aSrcRange, aDestRange, drawMode );
        }
        else
        {
            getGenericRenderer()->drawMaskedBitmap( rSrcBitmap, rMask,
                                                    rSrcRect, rDstRect, drawMode );
        }
#else
        if( isCompatibleBitmap( rSrcBitmap ) &&
            isCompatibleClipMask( rMask ) )
        {
            drawMaskedBitmap_i( rSrcBitmap, rMask, aSrcRange, aDestRange, drawMode );
        }
        else
        {
            OSL_ENSURE(false, "Generic output not yet implemented!");
        }
#endif
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

#if 0
        if( isCompatibleBitmap( rSrcBitmap ) &&
            isCompatibleClipMask( rMask ) &&
            isCompatibleClipMask( rClip ) )
        {
            drawMaskedBitmap_i( rSrcBitmap, rMask, aSrcRange, aDestRange, drawMode, rClip );
        }
        else
        {
            getGenericRenderer()->drawMaskedBitmap( rSrcBitmap, rMask, rSrcRect,
                                                    rDstRect, drawMode, rClip );
        }
#else
        if( isCompatibleBitmap( rSrcBitmap ) &&
            isCompatibleClipMask( rMask ) &&
            isCompatibleClipMask( rClip ) )
        {
            drawMaskedBitmap_i( rSrcBitmap, rMask, aSrcRange, aDestRange, drawMode, rClip );
        }
        else
        {
            OSL_ENSURE(false, "Generic output not yet implemented!");
        }
#endif
    }
}


//----------------------------------------------------------------------------------

/** Standard clip and alpha masks
 */
struct StdMasks
{
    typedef PixelFormatTraits_GREY1_MSB   clipmask_format_traits;
    typedef PixelFormatTraits_GREY8       alphamask_format_traits;
};

#if 0
/** Clip and alpha masks for the generic renderer (of course, those
    need to be generic, too)
 */
struct MaskTraitsGeneric
{
    typedef PixelFormatTraits_GenericInteger   clipmask_format_traits;
    typedef PixelFormatTraits_GenericInteger   alphamask_format_traits;
};
#endif

//----------------------------------------------------------------------------------

/// Produces a specialized renderer for the given pixel format
template< class FormatTraits, class MaskTraits >
BitmapDeviceSharedPtr createRenderer(
    const basegfx::B2IRange&                                     rBounds,
    bool                                                         bTopDown,
    sal_Int32                                                    nScanlineFormat,
    sal_Int32                                                    nScanlineStride,
    sal_uInt8*                                                   pFirstScanline,
    typename FormatTraits::accessor_selector::template wrap_accessor<
          typename FormatTraits::raw_accessor_type>::type const& rAccessor,
    boost::shared_array< sal_uInt8 >                             pMem,
    const PaletteMemorySharedVector&                             pPal )
{
    typedef typename FormatTraits::iterator_type                Iterator;
    typedef BitmapRenderer< Iterator,
                            typename FormatTraits::raw_accessor_type,
                            typename FormatTraits::accessor_selector,
                            MaskTraits >                        Renderer;

    return BitmapDeviceSharedPtr(
        new Renderer( rBounds,
                      bTopDown,
                      nScanlineFormat,
                      nScanlineStride,
                      pFirstScanline,
                      Iterator(
                          reinterpret_cast<typename Iterator::value_type*>(
                              pFirstScanline),
                          nScanlineStride),
                      rAccessor,
                      pMem,
                      pPal ));
}

/// Create standard grey level palette
PaletteMemorySharedVector createStandardPalette(
    const PaletteMemorySharedVector& pPal,
    sal_Int32                        nNumEntries )
{
    if( pPal || nNumEntries <= 0 )
        return pPal;

    boost::shared_ptr< std::vector<Color> > pLocalPal(
        new std::vector<Color>(nNumEntries) );

    const sal_Int32 nIncrement( 0x00FFFFFF/nNumEntries );
    --nNumEntries;
    for( sal_Int32 i=0, c=0; i<nNumEntries; ++i,c+=nIncrement )
        pLocalPal->at(i) = Color(0xFF000000 | c);

    pLocalPal->at(nNumEntries) = Color(0xFFFFFFFF);

    return pLocalPal;
}

template< class FormatTraits, class MaskTraits >
BitmapDeviceSharedPtr createRenderer(
    const basegfx::B2IRange&                rBounds,
    bool                                    bTopDown,
    sal_Int32                               nScanlineFormat,
    sal_Int32                               nScanlineStride,
    sal_uInt8*                              pFirstScanline,
    boost::shared_array< sal_uInt8 >        pMem,
    const PaletteMemorySharedVector&        pPal )
{
    return createRenderer<FormatTraits,
                          MaskTraits>(rBounds,
                                      bTopDown,
                                      nScanlineFormat,
                                      nScanlineStride,
                                      pFirstScanline,
                                      typename FormatTraits::accessor_selector::template
                                      wrap_accessor<
                                          typename FormatTraits::raw_accessor_type>::type(),
                                      pMem,
                                      pPal);
}

template< class FormatTraits, class MaskTraits >
BitmapDeviceSharedPtr createRenderer(
    const basegfx::B2IRange&                rBounds,
    bool                                    bTopDown,
    sal_Int32                               nScanlineFormat,
    sal_Int32                               nScanlineStride,
    sal_uInt8*                              pFirstScanline,
    boost::shared_array< sal_uInt8 >        pMem,
    PaletteMemorySharedVector               pPal,
    int                                     nBitsPerPixel )
{
    pPal = createStandardPalette(pPal,
                                 1UL << nBitsPerPixel);

    OSL_ASSERT(pPal);
    return createRenderer<FormatTraits,
                          MaskTraits>(rBounds,
                                      bTopDown,
                                      nScanlineFormat,
                                      nScanlineStride,
                                      pFirstScanline,
                                      typename FormatTraits::accessor_selector::template
                                          wrap_accessor<
                                      typename FormatTraits::raw_accessor_type>::type(
                                          &pPal->at(0),
                                          pPal->size()),
                                      pMem,
                                      pPal);
}

//----------------------------------------------------------------------------------

// TODO(Q3): consolidate with canvas/canvastools.hxx! Best move this
// to o3tl or sal/bithacks.hxx ...

/** Compute the next highest power of 2 of a 32-bit value

    Code devised by Sean Anderson, in good ole HAKMEM
    tradition.

    @return 1 << (lg(x - 1) + 1)
*/
inline sal_uInt32 nextPow2( sal_uInt32 x )
{
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    return ++x;
}

//----------------------------------------------------------------------------------

namespace
{
BitmapDeviceSharedPtr createBitmapDeviceImpl( const basegfx::B2IVector&        rSize,
                                              bool                             bTopDown,
                                              sal_Int32                        nScanlineFormat,
                                              boost::shared_array< sal_uInt8 > pMem,
                                              PaletteMemorySharedVector        pPal,
                                              const basegfx::B2IRange*         pSubset )
{
    if( nScanlineFormat <= Format::NONE ||
        nScanlineFormat >  Format::MAX )
        return BitmapDeviceSharedPtr();

    static const sal_uInt8 bitsPerPixel[] =
    {
        0,  // NONE
        1,  // ONE_BIT_MSB_GREY
        1,  // ONE_BIT_LSB_GREY
        1,  // ONE_BIT_MSB_PAL
        1,  // ONE_BIT_LSB_PAL
        4,  // FOUR_BIT_MSB_GREY
        4,  // FOUR_BIT_LSB_GREY
        4,  // FOUR_BIT_MSB_PAL
        4,  // FOUR_BIT_LSB_PAL
        8,  // EIGHT_BIT_PAL
        8,  // EIGHT_BIT_GREY
        16, // SIXTEEN_BIT_LSB_TC_MASK
        16, // SIXTEEN_BIT_MSB_TC_MASK
        24, // TWENTYFOUR_BIT_TC_MASK
        32, // THIRTYTWO_BIT_TC_MASK
    };

    sal_Int32  nScanlineStride(0);

    // round up to full 8 bit, divide by 8
    nScanlineStride = (rSize.getX()*bitsPerPixel[nScanlineFormat] + 7) >> 3;

    // rounded up to next full power-of-two number of bytes
    const sal_uInt32 bytesPerPixel = nextPow2(
        (bitsPerPixel[nScanlineFormat] + 7) >> 3);

    // now make nScanlineStride a multiple of bytesPerPixel
    nScanlineStride = (nScanlineStride + bytesPerPixel - 1) / bytesPerPixel * bytesPerPixel;

    // factor in bottom-up scanline order case
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

    // shrink render area to given subset, if given
    basegfx::B2IRange aBounds(0,0,rSize.getX(),rSize.getY());
    if( pSubset )
        aBounds.intersect( *pSubset );

    switch( nScanlineFormat )
    {
        // ----------------------------------------------------------------------
        // one bit formats

        case Format::ONE_BIT_MSB_GREY:
            return createRenderer<PixelFormatTraits_GREY1_MSB,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::ONE_BIT_LSB_GREY:
            return createRenderer<PixelFormatTraits_GREY1_LSB,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::ONE_BIT_MSB_PAL:
            return createRenderer<PixelFormatTraits_PAL1_MSB,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat] );

        case Format::ONE_BIT_LSB_PAL:
            return createRenderer<PixelFormatTraits_PAL1_LSB,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat] );


        // ----------------------------------------------------------------------
        // four bit formats

        case Format::FOUR_BIT_MSB_GREY:
            return createRenderer<PixelFormatTraits_GREY4_MSB,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::FOUR_BIT_LSB_GREY:
            return createRenderer<PixelFormatTraits_GREY4_LSB,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::FOUR_BIT_MSB_PAL:
            return createRenderer<PixelFormatTraits_PAL4_MSB,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat] );

        case Format::FOUR_BIT_LSB_PAL:
            return createRenderer<PixelFormatTraits_PAL4_LSB,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat] );


        // ----------------------------------------------------------------------
        // eight bit formats

        case Format::EIGHT_BIT_GREY:
            return createRenderer<PixelFormatTraits_GREY8,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::EIGHT_BIT_PAL:
            return createRenderer<PixelFormatTraits_PAL8,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat] );


        // ----------------------------------------------------------------------
        // sixteen bit formats

        case Format::SIXTEEN_BIT_LSB_TC_MASK:
            return createRenderer<PixelFormatTraits_RGB16_565_LSB,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::SIXTEEN_BIT_MSB_TC_MASK:
            return createRenderer<PixelFormatTraits_RGB16_565_MSB,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );


        // ----------------------------------------------------------------------
        // twentyfour bit formats
        case Format::TWENTYFOUR_BIT_TC_MASK:
            return createRenderer<PixelFormatTraits_RGB24,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );


        // ----------------------------------------------------------------------
        // thirtytwo bit formats

        case Format::THIRTYTWO_BIT_TC_MASK:
            return createRenderer<PixelFormatTraits_RGB32_888,StdMasks>(
                aBounds, bTopDown, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );
    }

    // TODO(F3): other formats not yet implemented
    return BitmapDeviceSharedPtr();
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
                                   PaletteMemorySharedVector(),
                                   NULL );
}

BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector&        rSize,
                                          bool                             bTopDown,
                                          sal_Int32                        nScanlineFormat,
                                          const PaletteMemorySharedVector& rPalette )
{
    return createBitmapDeviceImpl( rSize,
                                   bTopDown,
                                   nScanlineFormat,
                                   boost::shared_array< sal_uInt8 >(),
                                   rPalette,
                                   NULL );
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
                                   rPalette,
                                   NULL );
}

BitmapDeviceSharedPtr subsetBitmapDevice( const BitmapDeviceSharedPtr&     rProto,
                                          const basegfx::B2IRange&         rSubset )
{
    return createBitmapDeviceImpl( rProto->getSize(),
                                   rProto->isTopDown(),
                                   rProto->getScanlineFormat(),
                                   rProto->getBuffer(),
                                   rProto->getPalette(),
                                   &rSubset );
}

BitmapDeviceSharedPtr cloneBitmapDevice( const basegfx::B2IVector&        rSize,
                                         const BitmapDeviceSharedPtr&     rProto )
{
    return createBitmapDeviceImpl( rSize,
                                   rProto->isTopDown(),
                                   rProto->getScanlineFormat(),
                                   boost::shared_array< sal_uInt8 >(),
                                   rProto->getPalette(),
                                   NULL );
}

//----------------------------------------------------------------------------------

/// Clone our device, with GenericImageAccessor to handle all formats
BitmapDeviceSharedPtr BitmapDevice::getGenericRenderer() const
{
#if 0
    // xxx TODO
    typedef BitmapRenderer< PixelFormatTraits_GenericInteger::iterator_type,
                            PixelFormatTraits_GenericInteger::raw_accessor_type,
                            PixelFormatTraits_GenericInteger::accessor_selector,
                            MaskTraitsGeneric >
            Renderer;

    if( !mpImpl->mpGenericRenderer )
    {
        mpImpl->mpGenericRenderer.reset(
            new Renderer(
                mpImpl->maBounds,
                isTopDown(),
                getScanlineFormat(),
                getScanlineStride(),
                mpImpl->mpFirstScanline,
                PixelFormatTraits_GenericInteger::iterator_type(),
                GenericIntegerImageAccessor<Color>(
                    const_cast<BitmapDevice*>(this)->shared_from_this()),
                getBuffer(),
                getPalette() ));
    }
#endif

    return mpImpl->mpGenericRenderer;
}

} // namespace basebmp
