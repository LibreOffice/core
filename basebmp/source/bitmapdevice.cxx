/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bitmapdevice.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2006-05-31 09:49:44 $
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
#include "basebmp/linerenderer.hxx"

#include <rtl/alloc.h>
#include <rtl/memory.h>
#include <osl/diagnose.h>

#include <basegfx/tools/tools.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygonrasterconverter.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/vector/b2ivector.hxx>

#include <vigra/basicimage.hxx>
#include <vigra/imageiterator.hxx>


namespace basebmp
{

namespace
{
    typedef PackedPixelIterator< sal_uInt8,
                                 sal_uInt8,
                                 1,
                                 true >          MaskIterator;
    typedef StandardAccessor< sal_uInt8 >        MaskAccessor;

    typedef vigra::ImageIterator< sal_uInt8 >                    AlphaMaskIterator;
    typedef vigra::AccessorTraits< sal_uInt8 >::default_accessor AlphaMaskAccessor;


    template< class DestIterator, class DestAccessor > class Renderer :
        public basegfx::B2DPolyPolygonRasterConverter
    {
    private:
        basegfx::B2IRange                 bounds_;
        typename DestIterator::value_type fillColor_;
        typename DestIterator::value_type clearColor_;
        DestIterator                      begin_;
        DestAccessor                      accessor_;

    public:
        Renderer(const basegfx::B2DPolyPolygon&     rPolyPolyRaster,
                 typename DestIterator::value_type  fillColor,
                 const basegfx::B2IRange&           bounds,
                 DestIterator                       begin,
                 DestIterator                       end,
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
                                               std::min( bounds_.getMaxX()-1,
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
        typename DestIterator::value_type                       fillColor,
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
                                                       dest.second,
                                                       dest.third));
    }

    template< class DestIterator, class DestAccessor > class BitmapRenderer : public BitmapDevice
    {
    public:
        typedef BitmapRenderer<MaskIterator,
                               MaskAccessor>                MaskBitmap;
        typedef BitmapRenderer<AlphaMaskIterator,
                               AlphaMaskAccessor>           AlphaMaskBitmap;
        typedef typename xorAccessor<DestAccessor>::type    XorAccessor;
        typedef typename maskedAccessor<DestAccessor,
                                        MaskAccessor,
                                        DestIterator,
                                        MaskIterator>::type MaskedAccessor;
        typedef typename maskedAccessor<XorAccessor,
                                        MaskAccessor,
                                        DestIterator,
                                        MaskIterator>::type MaskedXorAccessor;

        DestIterator      maBegin;
        DestIterator      maEnd;
        DestAccessor      maAccessor;
        XorAccessor       maXorAccessor;
        MaskedAccessor    maMaskedAccessor;
        MaskedXorAccessor maMaskedXorAccessor;
        int               mnWidth;
        int               mnHeight;

        BitmapRenderer( const basegfx::B2IVector& rSize,
                        bool                      bTopDown,
                        sal_Int32                 nScanlineFormat,
                        sal_Int32                 nScanlineStride,
                        DestIterator              begin,
                        DestIterator              end,
                        DestAccessor              accessor,
                        const RawMemorySharedPtr& rMem ) :
            BitmapDevice( rSize, bTopDown, nScanlineFormat, nScanlineStride, rMem ),
            maBegin( begin ),
            maEnd( end ),
            maAccessor( accessor ),
            maXorAccessor( accessor ),
            maMaskedAccessor( accessor ),
            maMaskedXorAccessor( maXorAccessor ),
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

        boost::shared_ptr<MaskBitmap> getCompatibleMask( const BitmapDeviceSharedPtr& bmp ) const
        {
            return boost::dynamic_pointer_cast<MaskBitmap>( bmp );
        }

        virtual bool isCompatibleClipMask( const BitmapDeviceSharedPtr& bmp ) const
        {
            // TODO(P1): dynamic_cast usually called twice for
            // compatible formats
            return getCompatibleMask( bmp ).get() != NULL;
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

        virtual const Color* getPalette_i() const
        {
            // TODO(F3): Palette
            return NULL;
        }

        virtual const sal_Int32 getPaletteEntryCount_i() const
        {
            // TODO(F3): Palette
            return 0;
        }

        virtual void clear_i( Color fillColor )
        {
            DestIterator currIter( maBegin );
            const DestIterator endIter( maBegin + vigra::Diff2D(0,mnHeight) );
            while( currIter != endIter )
            {
                typename DestIterator::row_iterator rowIter( currIter.rowIterator() );
                const typename DestIterator::row_iterator rowEnd( rowIter + mnWidth );

                // TODO(P2): Provide specialized span fill methods on the
                // iterator/accessor
                while( rowIter != rowEnd )
                    maAccessor.set(fillColor, rowIter++);

                currIter += vigra::Diff2D( 0,1 );
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
                maXorAccessor.set( lineColor, pixel );
            else
                maAccessor.set( lineColor, pixel );
        }

        virtual void setPixel_i( const basegfx::B2IPoint&     rPt,
                                 Color                        lineColor,
                                 DrawMode                     drawMode,
                                 const BitmapDeviceSharedPtr& rClip )
        {
            boost::shared_ptr<MaskBitmap> pMask( getCompatibleMask(rClip) );

            const vigra::Diff2D offset(rPt.getX(),
                                       rPt.getY());

            const DestIterator pixel( maBegin + offset );
            const MaskIterator maskPixel( pMask->maBegin + offset );

            if( drawMode == DrawMode_XOR )
                maMaskedXorAccessor.set( lineColor, pixel, maskPixel );
            else
                maMaskedAccessor.set( lineColor, pixel, maskPixel );
        }

        virtual Color getPixel_i(const basegfx::B2IPoint& rPt )
        {
            const DestIterator pixel( maBegin +
                                      vigra::Diff2D(rPt.getX(),
                                                    rPt.getY()) );
            return maAccessor(pixel);
        }

        virtual void drawLine_i(const basegfx::B2DPoint& rPt1,
                                const basegfx::B2DPoint& rPt2,
                                Color                    lineColor,
                                DrawMode                 drawMode )
        {
            renderLine( basegfx::fround(rPt1),
                        basegfx::fround(rPt2),
                        lineColor,
                        maBegin,
                        maEnd,
                        maAccessor );
        }

        virtual void drawLine_i(const basegfx::B2DPoint&     rPt1,
                                const basegfx::B2DPoint&     rPt2,
                                Color                        lineColor,
                                DrawMode                     drawMode,
                                const BitmapDeviceSharedPtr& rClip )
        {
            OSL_ENSURE( false, "drawLine_i(): Clipped output not yet implemented!" );
        }

        virtual void drawPolygon_i(const basegfx::B2DPolygon& rPoly,
                                   Color                      lineColor,
                                   DrawMode                   drawMode )
        {
            const sal_uInt32 nVertices( rPoly.count() );
            for( sal_uInt32 i=1; i<nVertices; ++i )
            {
                renderLine( basegfx::fround(rPoly.getB2DPoint(i-1)),
                            basegfx::fround(rPoly.getB2DPoint(i)),
                            lineColor,
                            maBegin,
                            maEnd,
                            maAccessor );
            }

            if( nVertices > 1 && rPoly.isClosed() )
                renderLine( basegfx::fround(rPoly.getB2DPoint(nVertices-1)),
                            basegfx::fround(rPoly.getB2DPoint(0)),
                            lineColor,
                            maBegin,
                            maEnd,
                            maAccessor );
        }

        virtual void drawPolygon_i(const basegfx::B2DPolygon&   rPoly,
                                   Color                        lineColor,
                                   DrawMode                     drawMode,
                                   const BitmapDeviceSharedPtr& rClip )
        {
            OSL_ENSURE( false, "drawPolygon_i(): clipped output not yet implemented!" );
        }

        virtual void fillPolyPolygon_i(const basegfx::B2DPolyPolygon& rPoly,
                                       Color                          fillColor,
                                       DrawMode                       drawMode,
                                       const basegfx::B2IRange&       rBounds )
        {
/*
            if( drawMode == DrawMode_XOR )
                makeRenderer( rPoly,
                              fillColor,
                              rBounds,
                              maBegin,
                              maEnd,
                              maXorAccessor )->rasterConvert(
                                  basegfx::FillRule_NONZERO_WINDING_NUMBER );
            else
*/
                makeRenderer( rPoly,
                              fillColor,
                              rBounds,
                              vigra::make_triple(
                                  maBegin,
                                  maEnd,
                                  maAccessor) )->rasterConvert(
                                  basegfx::FillRule_NONZERO_WINDING_NUMBER );
        }

        virtual void fillPolyPolygon_i(const basegfx::B2DPolyPolygon& rPoly,
                                       Color                          fillColor,
                                       DrawMode                       drawMode,
                                       const basegfx::B2IRange&       rBounds,
                                       const BitmapDeviceSharedPtr&   rClip )
        {
            OSL_ENSURE( false, "fillPolyPolygon_i(): Clipped output not yet implemented!" );
/*          Need uniform accessor for both plain and masked output. Combine
            mask and bitmap iterator into proxy iterator, and pass that one
            to the render method

            if( drawMode == DrawMode_XOR )
                makeRenderer( rPoly,
                              fillColor,
                              rBounds,
                              maBegin,
                              maEnd,
                              maMaskedXorAccessor )->rasterConvert(
                                  basegfx::FillRule_NONZERO_WINDING_NUMBER );
            else
                makeRenderer( rPoly,
                              fillColor,
                              rBounds,
                              maBegin,
                              maEnd,
                              maMaskedAccessor )->rasterConvert(
                                  basegfx::FillRule_NONZERO_WINDING_NUMBER );
*/
        }

        // must work with *this == rSrcBitmap!
        virtual void drawBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                  const basegfx::B2IRange&     rSrcRect,
                                  const basegfx::B2IRange&     rDstRect,
                                  DrawMode                     drawMode )
        {
            OSL_ENSURE( false, "Method not yet implemented!" );
        }

        virtual void drawBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                  const basegfx::B2IRange&     rSrcRect,
                                  const basegfx::B2IRange&     rDstRect,
                                  DrawMode                     drawMode,
                                  const BitmapDeviceSharedPtr& rClip )
        {
            OSL_ENSURE( false, "Method not yet implemented!" );
        }

        // must work with *this == rSrcBitmap!
        virtual void drawMaskedColor_i(Color                        rSrcColor,
                                       const BitmapDeviceSharedPtr& rAlphaMask,
                                       const basegfx::B2IRange&     rSrcRect,
                                       const basegfx::B2IPoint&     rDstPoint )
        {
            OSL_ENSURE( false, "Method not yet implemented!" );
        }

        virtual void drawMaskedColor_i(Color                        rSrcColor,
                                       const BitmapDeviceSharedPtr& rAlphaMask,
                                       const basegfx::B2IRange&     rSrcRect,
                                       const basegfx::B2IPoint&     rDstPoint,
                                       const BitmapDeviceSharedPtr& rClip )
         {
            OSL_ENSURE( false, "Method not yet implemented!" );
        }

        // must work with *this == rSrcBitmap!
        virtual void drawMaskedBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                        const BitmapDeviceSharedPtr& rMask,
                                        const basegfx::B2IRange&     rSrcRect,
                                        const basegfx::B2IRange&     rDstRect,
                                        DrawMode                     drawMode )
        {
            OSL_ENSURE( false, "Method not yet implemented!" );
        }

        virtual void drawMaskedBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                        const BitmapDeviceSharedPtr& rMask,
                                        const basegfx::B2IRange&     rSrcRect,
                                        const basegfx::B2IRange&     rDstRect,
                                        DrawMode                     drawMode,
                                        const

 BitmapDeviceSharedPtr& rClip )
        {
            OSL_ENSURE( false, "Method not yet implemented!" );
        }
    };
} // namespace

struct ImplBitmapDevice
{
    /// Bitmap memory plus deleter
    RawMemorySharedPtr mpMem;

    basegfx::B2IRange  maBounds;
    basegfx::B2DRange  maFloatBounds;
    sal_Int32          mnScanlineFormat;
    sal_Int32          mnScanlineStride;
};


BitmapDevice::BitmapDevice( const basegfx::B2IVector& rSize,
                            bool                      bTopDown,
                            sal_Int32                 nScanlineFormat,
                            sal_Int32                 nScanlineStride,
                            const RawMemorySharedPtr& rMem ) :
    mpImpl( new ImplBitmapDevice )
{
    mpImpl->mpMem = rMem;
    mpImpl->maBounds = basegfx::B2IRange( 0,0,rSize.getX(),rSize.getY() );
    mpImpl->maFloatBounds = basegfx::B2DRange( 0,0,rSize.getX(),rSize.getY() );
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

RawMemorySharedPtr BitmapDevice::getBuffer() const
{
    return mpImpl->mpMem;
}

const Color* BitmapDevice::getPalette() const
{
    return getPalette_i();
}

const sal_Int32 BitmapDevice::getPaletteEntryCount() const
{
    return getPaletteEntryCount_i();
}

void BitmapDevice::clear( Color fillColor )
{
    clear_i( fillColor );
}

void BitmapDevice::setPixel( const basegfx::B2IPoint& rPt,
                             Color                    lineColor,
                             DrawMode                 drawMode )
{
    if( mpImpl->maBounds.isInside(rPt) )
        setPixel_i(rPt,lineColor,drawMode);
}

void BitmapDevice::setPixel( const basegfx::B2IPoint&     rPt,
                             Color                        lineColor,
                             DrawMode                     drawMode,
                             const BitmapDeviceSharedPtr& rClip )
{
    if( mpImpl->maBounds.isInside(rPt) )
    {
        if( isCompatibleClipMask( rClip ) )
            setPixel_i(rPt,lineColor,drawMode,rClip);
        else
            OSL_ENSURE( false, "Generic output not yet implemented!" );
    }
}

Color BitmapDevice::getPixel( const basegfx::B2IPoint& rPt )
{
    if( mpImpl->maBounds.isInside(rPt) )
        return getPixel_i(rPt);

    return Color();
}

void BitmapDevice::drawLine( const basegfx::B2IPoint& rPt1,
                             const basegfx::B2IPoint& rPt2,
                             Color                    lineColor,
                             DrawMode                 drawMode )
{
    basegfx::B2DPoint aPt1( rPt1 );
    basegfx::B2DPoint aPt2( rPt2 );

    if( basegfx::tools::liangBarskyClip2D(aPt1,aPt2,mpImpl->maFloatBounds) )
        drawLine_i( aPt1, aPt2,
                    lineColor,
                    drawMode );
}

void BitmapDevice::drawLine( const basegfx::B2IPoint&     rPt1,
                             const basegfx::B2IPoint&     rPt2,
                             Color                        lineColor,
                             DrawMode                     drawMode,
                             const BitmapDeviceSharedPtr& rClip )
{
#if 1
    drawLine( rPt1, rPt2, lineColor, drawMode );
#else
    basegfx::B2DPoint aPt1( rPt1 );
    basegfx::B2DPoint aPt2( rPt2 );

    if( basegfx::tools::liangBarskyClip2D(aPt1,aPt2,mpImpl->maFloatBounds) )
    {
        if( isCompatibleClipMask( rClip ) )
            drawLine_i( aPt1, aPt2,
                        lineColor,
                        drawMode,
                        rClip );
        else
            OSL_ENSURE( false, "Generic output not yet implemented!" );
    }
#endif
}

void BitmapDevice::drawPolygon( const basegfx::B2DPolygon& rPoly,
                                Color                      lineColor,
                                DrawMode                   drawMode )
{
    basegfx::B2DPolyPolygon aPoly(
        basegfx::tools::clipPolygonOnRange( rPoly,
                                            mpImpl->maFloatBounds,
                                            true,
                                            true ));
    const sal_uInt32 numPolies( aPoly.count() );
    for( sal_uInt32 i=0; i<numPolies; ++i )
        if( aPoly.getB2DPolygon(i).count() )
            drawPolygon_i( aPoly.getB2DPolygon(i), lineColor, drawMode );
}

void BitmapDevice::drawPolygon( const basegfx::B2DPolygon&   rPoly,
                                Color                        lineColor,
                                DrawMode                     drawMode,
                                const BitmapDeviceSharedPtr& rClip )
{
#if 1
    drawPolygon( rPoly, lineColor, drawMode );
#else
    basegfx::B2DPolyPolygon aPoly(
        basegfx::tools::clipPolygonOnRange( rPoly,
                                            mpImpl->maFloatBounds,
                                            true,
                                            true ));
    const sal_uInt32 numPolies( aPoly.count() );
    for( sal_uInt32 i=0; i<numPolies; ++i )
        if( aPoly.getB2DPolygon(i).count() )
            if( isCompatibleClipMask( rClip ) )
                drawPolygon_i( aPoly.getB2DPolygon(i), lineColor, drawMode, rClip );
            else
                OSL_ENSURE( false, "Generic output not yet implemented!" );
#endif
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
#if 1
    fillPolyPolygon( rPoly, fillColor, drawMode );
#else
    if( isCompatibleClipMask( rClip ) )
        fillPolyPolygon_i( rPoly, fillColor, drawMode, mpImpl->maBounds, rClip );
    else
        OSL_ENSURE( false, "Generic output not yet implemented!" );
#endif
}


namespace
{
    // TODO(Q3): Move canvas/canvastools.hxx clipBlit() down
    // to basegfx, and use here!
    bool clipAreaImpl( ::basegfx::B2IRange*       o_pDestArea,
                       ::basegfx::B2IRange&       io_rSourceArea,
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

        if( o_pDestArea )
            *o_pDestArea = aLocalDestArea;

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

    basegfx::B2IRange aSrcRange( rSrcRect );
    basegfx::B2IPoint aDestPoint( rDstRect.getMinimum() );
    basegfx::B2IRange aDestRange;

    // TODO(F2): Scaling/shrinking
    if( clipAreaImpl( &aDestRange,
                      aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
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
    const basegfx::B2IVector& rSrcSize( rSrcBitmap->getSize() );
    const basegfx::B2IRange   aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );

    basegfx::B2IRange aSrcRange( rSrcRect );
    basegfx::B2IPoint aDestPoint( rDstRect.getMinimum() );
    basegfx::B2IRange aDestRange;

    // TODO(F2): Scaling/shrinking
    if( clipAreaImpl( &aDestRange,
                      aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
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

    basegfx::B2IRange aSrcRange( rSrcRect );
    basegfx::B2IPoint aDestPoint( rDstPoint );

    if( clipAreaImpl( NULL,
                      aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
        if( isCompatibleAlphaMask( rAlphaMask ) )
            drawMaskedColor_i( rSrcColor, rAlphaMask, aSrcRange, aDestPoint );
        else
            OSL_ENSURE( false, "Generic output not yet implemented!" );
    }
}

void BitmapDevice::drawMaskedColor( Color                        rSrcColor,
                                    const BitmapDeviceSharedPtr& rAlphaMask,
                                    const basegfx::B2IRange&     rSrcRect,
                                    const basegfx::B2IPoint&     rDstPoint,
                                    const BitmapDeviceSharedPtr& rClip )
{
    const basegfx::B2IVector& rSrcSize( rAlphaMask->getSize() );
    const basegfx::B2IRange   aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );

    basegfx::B2IRange aSrcRange( rSrcRect );
    basegfx::B2IPoint aDestPoint( rDstPoint );

    if( clipAreaImpl( NULL,
                      aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
        if( isCompatibleAlphaMask( rAlphaMask ) &&
            isCompatibleClipMask( rClip ) )
        {
            drawMaskedColor_i( rSrcColor, rAlphaMask, aSrcRange, aDestPoint, rClip );
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

    basegfx::B2IRange aSrcRange( rSrcRect );
    basegfx::B2IPoint aDestPoint( rDstRect.getMinimum() );
    basegfx::B2IRange aDestRange;

    // TODO(F2): Scaling/shrinking
    if( clipAreaImpl( &aDestRange,
                      aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
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
    OSL_ASSERT( rMask->getSize() == rSrcBitmap->getSize() );

    const basegfx::B2IVector& rSrcSize( rSrcBitmap->getSize() );
    const basegfx::B2IRange   aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );

    basegfx::B2IRange aSrcRange( rSrcRect );
    basegfx::B2IPoint aDestPoint( rDstRect.getMinimum() );
    basegfx::B2IRange aDestRange;

    // TODO(F2): Scaling/shrinking
    if( clipAreaImpl( &aDestRange,
                      aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
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

typedef PaletteImageAccessor<Color,sal_uInt8>                               PaletteAccessor;
typedef BitmapRenderer<MaskIterator,PaletteAccessor >                       OneBitMsbPaletteRenderer;
typedef BitmapRenderer<AlphaMaskIterator,AlphaMaskAccessor>                 EightBitTrueColorRenderer;
typedef PixelIterator<Color>                                                ThirtyTwoBitPixelIterator;
typedef BitmapRenderer<ThirtyTwoBitPixelIterator,
                       vigra::AccessorTraits<Color>::default_accessor>      ThirtyTwoBitTrueColorRenderer;

BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector& rSize,
                                          bool                      bTopDown,
                                          sal_Int32                 nScanlineFormat )
{
    sal_Int32  nScanlineStride(0);

    // HACK: 1bpp and 24bpp only, currently
    if( nScanlineFormat == Format::ONE_BIT_MSB_PAL )
        nScanlineStride = (rSize.getX() + 7) >> 3;
    else if( nScanlineFormat == Format::EIGHT_BIT_TC_MASK )
        nScanlineStride = rSize.getX();
    else if( nScanlineFormat == Format::THIRTYTWO_BIT_TC_MASK )
        nScanlineStride = 4*rSize.getX();

    nScanlineStride *= bTopDown ? 1 : -1;

    const std::size_t nMemSize(
        (nScanlineStride < 0 ? -nScanlineStride : nScanlineStride)*rSize.getY() );

    boost::shared_ptr< sal_uInt8 > pMem(
        reinterpret_cast<sal_uInt8*>(rtl_allocateMemory( nMemSize )),
        &rtl_freeMemory );
    sal_uInt8* pFirstScanline = nScanlineStride < 0 ?
        pMem.get() + nMemSize : pMem.get();

    rtl_zeroMemory(pMem.get(),nMemSize);

    switch( nScanlineFormat )
    {
        case Format::ONE_BIT_MSB_PAL:
        {
            static Color bwPalette[] = { Color(0x00000000),
                                         Color(0xFFFFFFFF) };

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
                    PaletteAccessor( bwPalette,
                                     sizeof(bwPalette)/sizeof(*bwPalette)),
                    pMem ));
        }

        case Format::EIGHT_BIT_TC_MASK:
        {
            return BitmapDeviceSharedPtr(
                new EightBitTrueColorRenderer(
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
                    pMem ));
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
                    vigra::AccessorTraits<Color>::default_accessor(),
                    pMem ));
        }

        default:
            // TODO(F3): other formats not yet implemented
            return BitmapDeviceSharedPtr();
    }
}

} // namespace basebmp
