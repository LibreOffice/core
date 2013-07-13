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

#include "sal/config.h"

#include <cassert>
#include <string.h>

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
#include "basebmp/scaleimage.hxx"
#include "basebmp/clippedlinerenderer.hxx"
#include "basebmp/polypolygonrenderer.hxx"
#include "basebmp/genericcolorimageaccessor.hxx"

#include "basebmp/tools.hxx"
#include "intconversion.hxx"

#include <rtl/alloc.h>
#include <osl/diagnose.h>

#include <basegfx/tools/tools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/vector/b2ivector.hxx>

#include <vigra/iteratortraits.hxx>
#include <vigra/rgbvalue.hxx>
#include <vigra/copyimage.hxx>
#include <vigra/tuple.hxx>


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
              bool     polarity,
              typename MaskFunctorMode > struct masked_input_splitting_accessor
    {
        typedef BinarySetterFunctionAccessorAdapter<
            DestAccessor,
            BinaryFunctorSplittingWrapper<
                typename outputMaskFunctorSelector<
                         typename JoinedAccessor::value_type::first_type,
                         typename JoinedAccessor::value_type::second_type,
                         polarity,
                         MaskFunctorMode >::type > > type;
    };



    // Actual BitmapDevice implementation (templatized by accessor and iterator)

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

        @tpl Masks
        Traits template, containing nested traits
        clipmask_format_traits and alphamask_format_traits, which
        determine what specialized formats are to be used for clip and
        alpha masks. With those mask formats, clipping and alpha
        blending is handled natively.
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
            mask_iterator_type,
            Masks::clipmask_polarity>::type                                raw_maskedaccessor_type;
        typedef typename accessor_selector::template wrap_accessor<
            raw_maskedaccessor_type >::type                                masked_accessor_type;
        typedef typename AccessorTraits<
            raw_maskedaccessor_type>::xor_accessor                         raw_maskedxor_accessor_type;
        typedef typename accessor_selector::template wrap_accessor<
            raw_maskedxor_accessor_type >::type                            masked_xoraccessor_type;

        // -------------------------------------------------------

        // ((iter,mask),mask) special case (e.g. for clipped
        // drawMaskedColor())
        typedef AccessorTraits< raw_maskedaccessor_type >                  raw_maskedaccessor_traits;
        typedef typename raw_maskedaccessor_traits::template masked_accessor<
            mask_rawaccessor_type,
            composite_iterator_type,
            mask_iterator_type,
            Masks::clipmask_polarity>::type                                raw_maskedmask_accessor_type;

        typedef CompositeIterator2D<
            composite_iterator_type,
            mask_iterator_type>                                            composite_composite_mask_iterator_type;

        // -------------------------------------------------------

        typedef ConstantColorBlendSetterAccessorAdapter<
            dest_accessor_type,
            typename alphamask_rawaccessor_type::value_type,
            Masks::alphamask_polarity>                                     colorblend_accessor_type;
        typedef AccessorTraits<colorblend_accessor_type>                   colorblend_accessor_traits;
        typedef typename colorblend_accessor_traits::template masked_accessor<
            mask_rawaccessor_type,
            dest_iterator_type,
            mask_iterator_type,
            Masks::clipmask_polarity>::type                                masked_colorblend_accessor_type;

        // -------------------------------------------------------

        typedef ConstantColorBlendSetterAccessorAdapter<
            dest_accessor_type,
            Color,
            Masks::alphamask_polarity>                                     colorblend_generic_accessor_type;
        typedef AccessorTraits<colorblend_generic_accessor_type>           colorblend_generic_accessor_traits;
        typedef typename colorblend_generic_accessor_traits::template masked_accessor<
            mask_rawaccessor_type,
            dest_iterator_type,
            mask_iterator_type,
            Masks::clipmask_polarity>::type                                masked_colorblend_generic_accessor_type;

        // -------------------------------------------------------

        typedef JoinImageAccessorAdapter< dest_accessor_type,
                                          mask_rawaccessor_type >          joined_image_accessor_type;
        typedef JoinImageAccessorAdapter< GenericColorImageAccessor,
                                          GenericColorImageAccessor >      joined_generic_image_accessor_type;

        // -------------------------------------------------------

        dest_iterator_type                      maBegin;
        typename accessor_traits::color_lookup  maColorLookup;
        IBitmapDeviceDamageTrackerSharedPtr     mpDamage;
        to_uint32_functor                       maToUInt32Converter;
        dest_accessor_type                      maAccessor;
        colorblend_accessor_type                maColorBlendAccessor;
        colorblend_generic_accessor_type        maGenericColorBlendAccessor;
        raw_accessor_type                       maRawAccessor;
        xor_accessor_type                       maXorAccessor;
        raw_xor_accessor_type                   maRawXorAccessor;
        masked_accessor_type                    maMaskedAccessor;
        masked_colorblend_accessor_type         maMaskedColorBlendAccessor;
        masked_colorblend_generic_accessor_type maGenericMaskedColorBlendAccessor;
        masked_xoraccessor_type                 maMaskedXorAccessor;
        raw_maskedaccessor_type                 maRawMaskedAccessor;
        raw_maskedxor_accessor_type             maRawMaskedXorAccessor;
        raw_maskedmask_accessor_type            maRawMaskedMaskAccessor;


        // -------------------------------------------------------

        BitmapRenderer( const basegfx::B2IBox&                     rBounds,
                        const basegfx::B2IVector&                  rBufferSize,
                        Format                                     nScanlineFormat,
                        sal_Int32                                  nScanlineStride,
                        sal_uInt8*                                 pFirstScanline,
                        dest_iterator_type                         begin,
                        raw_accessor_type                          rawAccessor,
                        dest_accessor_type                         accessor,
                        const RawMemorySharedArray&                rMem,
                        const PaletteMemorySharedVector&           rPalette,
                        const IBitmapDeviceDamageTrackerSharedPtr& rDamage ) :
            BitmapDevice( rBounds, rBufferSize, nScanlineFormat,
                          nScanlineStride, pFirstScanline, rMem, rPalette ),
            maBegin( begin ),
            maColorLookup(),
            mpDamage(rDamage),
            maToUInt32Converter(),
            maAccessor( accessor ),
            maColorBlendAccessor( accessor ),
            maGenericColorBlendAccessor( accessor ),
            maRawAccessor( rawAccessor ),
            maXorAccessor( accessor ),
            maRawXorAccessor( rawAccessor ),
            maMaskedAccessor( accessor ),
            maMaskedColorBlendAccessor( maColorBlendAccessor ),
            maGenericMaskedColorBlendAccessor( maGenericColorBlendAccessor ),
            maMaskedXorAccessor( accessor ),
            maRawMaskedAccessor( rawAccessor ),
            maRawMaskedXorAccessor( rawAccessor ),
            maRawMaskedMaskAccessor( rawAccessor )
        {}

    private:

        void damaged( const basegfx::B2IBox& rDamageRect ) const
        {
            if( mpDamage )
                mpDamage->damaged( rDamageRect );
        }

        void damagedPointSize( const basegfx::B2IPoint& rPoint,
                               const basegfx::B2IBox&   rSize ) const
        {
            if( mpDamage ) {
                basegfx::B2IPoint aLower( rPoint.getX() + rSize.getWidth(),
                                          rPoint.getY() + rSize.getHeight() );
                damaged( basegfx::B2IBox( rPoint, aLower ) );
            }
        }

        void damagedPixel( const basegfx::B2IPoint& rDamagePoint ) const
        {
            if( !mpDamage )
                return;

            sal_Int32 nX(rDamagePoint.getX());
            sal_Int32 nY(rDamagePoint.getY());
            if (nX < SAL_MAX_INT32)
                ++nX;
            if (nY < SAL_MAX_INT32)
                ++nY;

            basegfx::B2IPoint aEnd( nX, nY );
            damaged( basegfx::B2IBox( rDamagePoint, aEnd ) );
        }

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

        virtual void clear_i( Color                   fillColor,
                              const basegfx::B2IBox&  rBounds )
        {
            fillImage(destIterRange(maBegin,
                                    maRawAccessor,
                                    rBounds),
                      maColorLookup(
                          maAccessor,
                          fillColor) );
            damaged( rBounds );
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
            damagedPixel(rPt);
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
            damagedPixel(rPt);
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
                              const basegfx::B2IBox&   rBounds,
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
            // TODO(P2): perhaps this needs pushing up the stack a bit
            // to make more complex polygons more efficient ...
            damaged( basegfx::B2IBox( rPt1, rPt2 ) );
        }

        template< typename Iterator, typename Accessor, typename RawAcc >
        void implRenderLine( const basegfx::B2IPoint& rPt1,
                             const basegfx::B2IPoint& rPt2,
                             const basegfx::B2IBox&   rBounds,
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
                           const basegfx::B2IBox&   rBounds,
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
                                const basegfx::B2IBox&   rBounds,
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
                                const basegfx::B2IBox&       rBounds,
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
                              const basegfx::B2IBox&     rBounds,
                              Color                      col,
                              const Iterator&            begin,
                              const RawAcc&              acc )
        {
            basegfx::B2DPolygon aPoly( rPoly );
            if( rPoly.areControlPointsUsed() )
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
                                   const basegfx::B2IBox&     rBounds,
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
                                   const basegfx::B2IBox&       rBounds,
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
                                  const basegfx::B2IBox&         rBounds )
        {
            basegfx::B2DPolyPolygon aPoly( rPoly );
            if( rPoly.areControlPointsUsed() )
                aPoly = basegfx::tools::adaptiveSubdivideByCount( rPoly );

            renderClippedPolyPolygon( begin,
                                      acc,
                                      maColorLookup( maAccessor,
                                                     col),
                                      rBounds,
                                      aPoly,
                                      basegfx::FillRule_EVEN_ODD );

            if( mpDamage )
            {
                basegfx::B2DRange const aPolyBounds( basegfx::tools::getRange(aPoly) );
                damaged( basegfx::unotools::b2ISurroundingBoxFromB2DRange( aPolyBounds ) );
            }
        }

        virtual void fillPolyPolygon_i(const basegfx::B2DPolyPolygon& rPoly,
                                       Color                          fillColor,
                                       DrawMode                       drawMode,
                                       const basegfx::B2IBox&         rBounds )
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
                                       const basegfx::B2IBox&         rBounds,
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
                            const basegfx::B2IBox&       rSrcRect,
                            const basegfx::B2IBox&       rDstRect,
                            const Iterator&              begin,
                            const RawAcc&                acc)
        {
            boost::shared_ptr<BitmapRenderer> pSrcBmp( getCompatibleBitmap(rSrcBitmap) );
            OSL_ASSERT( pSrcBmp );

            scaleImage(
                srcIterRange(pSrcBmp->maBegin,
                             pSrcBmp->maRawAccessor,
                             rSrcRect),
                destIterRange(begin,
                              acc,
                              rDstRect),
                isSharedBuffer(rSrcBitmap) );
            damaged( rDstRect );
        }

        template< typename Iterator, typename Acc >
        void implDrawBitmapGeneric(const BitmapDeviceSharedPtr& rSrcBitmap,
                                   const basegfx::B2IBox&       rSrcRect,
                                   const basegfx::B2IBox&       rDstRect,
                                   const Iterator&              begin,
                                   const Acc&                   acc)
        {
            GenericColorImageAccessor aSrcAcc( rSrcBitmap );

            scaleImage(
                srcIterRange(vigra::Diff2D(),
                             aSrcAcc,
                             rSrcRect),
                destIterRange(begin,
                              acc,
                              rDstRect));
            damaged( rDstRect );
        }

        virtual void drawBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                  const basegfx::B2IBox&       rSrcRect,
                                  const basegfx::B2IBox&       rDstRect,
                                  DrawMode                     drawMode )
        {
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
            damaged( rDstRect );
        }

        virtual void drawBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                  const basegfx::B2IBox&       rSrcRect,
                                  const basegfx::B2IBox&       rDstRect,
                                  DrawMode                     drawMode,
                                  const BitmapDeviceSharedPtr& rClip )
        {
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
            damaged( rDstRect );
        }

        virtual void drawMaskedColor_i(Color                        aSrcColor,
                                       const BitmapDeviceSharedPtr& rAlphaMask,
                                       const basegfx::B2IBox&       rSrcRect,
                                       const basegfx::B2IPoint&     rDstPoint )
        {
            boost::shared_ptr<mask_bitmap_type>      pMask( getCompatibleClipMask(rAlphaMask) );
            boost::shared_ptr<alphamask_bitmap_type> pAlpha( getCompatibleAlphaMask(rAlphaMask) );

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
            else
            {
                GenericColorImageAccessor aSrcAcc( rAlphaMask );
                maGenericColorBlendAccessor.setColor( aSrcColor );

                vigra::copyImage( srcIterRange(vigra::Diff2D(),
                                               aSrcAcc,
                                               rSrcRect),
                                  destIter(maBegin,
                                           maGenericColorBlendAccessor,
                                           rDstPoint) );
            }
            damagedPointSize( rDstPoint, rSrcRect );
        }

        virtual void drawMaskedColor_i(Color                        aSrcColor,
                                       const BitmapDeviceSharedPtr& rAlphaMask,
                                       const basegfx::B2IBox&       rSrcRect,
                                       const basegfx::B2IPoint&     rDstPoint,
                                       const BitmapDeviceSharedPtr& rClip )
        {
            boost::shared_ptr<mask_bitmap_type>      pMask( getCompatibleClipMask(rAlphaMask) );
            boost::shared_ptr<alphamask_bitmap_type> pAlpha( getCompatibleAlphaMask(rAlphaMask) );

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
            else
            {
                GenericColorImageAccessor aSrcAcc( rAlphaMask );
                const composite_iterator_type aBegin( getMaskedIter(rClip) );
                maGenericMaskedColorBlendAccessor.get1stWrappedAccessor().setColor(
                    aSrcColor );

                vigra::copyImage( srcIterRange(vigra::Diff2D(),
                                               aSrcAcc,
                                               rSrcRect),
                                  destIter(aBegin,
                                           maGenericMaskedColorBlendAccessor,
                                           rDstPoint) );
            }
            damagedPointSize( rDstPoint, rSrcRect );
        }

        template< typename Iterator, typename Acc >
        void implDrawMaskedBitmap(const BitmapDeviceSharedPtr& rSrcBitmap,
                                  const BitmapDeviceSharedPtr& rMask,
                                  const basegfx::B2IBox&       rSrcRect,
                                  const basegfx::B2IBox&       rDstRect,
                                  const Iterator&              begin,
                                  const Acc&                   acc)
        {
            boost::shared_ptr<BitmapRenderer>   pSrcBmp( getCompatibleBitmap(rSrcBitmap) );
            boost::shared_ptr<mask_bitmap_type> pMask( getCompatibleClipMask(rMask) );
            OSL_ASSERT( pMask && pSrcBmp );

            scaleImage(
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
                                       Masks::clipmask_polarity,
                                       FastMask >::type(acc),
                              rDstRect),
                isSharedBuffer(rSrcBitmap));
            damaged( rDstRect );
        }

        template< typename Iterator, typename Acc >
        void implDrawMaskedBitmapGeneric(const BitmapDeviceSharedPtr& rSrcBitmap,
                                         const BitmapDeviceSharedPtr& rMask,
                                         const basegfx::B2IBox&       rSrcRect,
                                         const basegfx::B2IBox&       rDstRect,
                                         const Iterator&              begin,
                                         const Acc&                   acc)
        {
            GenericColorImageAccessor aSrcAcc( rSrcBitmap );
            GenericColorImageAccessor aMaskAcc( rMask );

            const vigra::Diff2D aTopLeft(rSrcRect.getMinX(),
                                         rSrcRect.getMinY());
            const vigra::Diff2D aBottomRight(rSrcRect.getMaxX(),
                                             rSrcRect.getMaxY());
            scaleImage(
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
                                       Masks::clipmask_polarity,
                                       NoFastMask >::type(acc),
                              rDstRect));
            damaged( rDstRect );
        }

        virtual void drawMaskedBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                        const BitmapDeviceSharedPtr& rMask,
                                        const basegfx::B2IBox&       rSrcRect,
                                        const basegfx::B2IBox&       rDstRect,
                                        DrawMode                     drawMode )
        {
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
            damaged( rDstRect );
        }

        virtual void drawMaskedBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap,
                                        const BitmapDeviceSharedPtr& rMask,
                                        const basegfx::B2IBox&       rSrcRect,
                                        const basegfx::B2IBox&       rDstRect,
                                        DrawMode                     drawMode,
                                        const BitmapDeviceSharedPtr& rClip )
        {
            if( isCompatibleClipMask(rMask) &&
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
            damaged( rDstRect );
        }

        IBitmapDeviceDamageTrackerSharedPtr getDamageTracker_i() const
        {
            return mpDamage;
        }
        void setDamageTracker_i( const IBitmapDeviceDamageTrackerSharedPtr& rDamage )
        {
            mpDamage = rDamage;
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
        area under the including-the-bottommost-and-rightmost-pixels
        fill rule)
     */
    basegfx::B2IBox           maBounds;

    //// Size of the actual frame buffer
    basegfx::B2IVector        maBufferSize;

    /// Scanline format, as provided at the constructor
    Format                    mnScanlineFormat;

    /// Scanline stride. Negative for bottom-to-top formats
    sal_Int32                 mnScanlineStride;

    /// raw ptr to 0th scanline. used for cloning a generic renderer
    sal_uInt8*                mpFirstScanline;

    /** (Optional) device sharing the same memory, and used for input
        clip masks/alpha masks/bitmaps that don't match our exact
        bitmap format.

        This is to avoid the combinatorical explosion when dealing
        with n bitmap formats, which could be combined with n clip
        masks, alpha masks and bitmap masks (yielding a total of n^4
        combinations). Since each BitmapRenderer is specialized for
        one specific combination of said formats, a lot of duplicate
        code would be generated, most of which probably never
        used. Therefore, only the most common combinations are
        specialized templates, the remainder gets handled by this
        generic renderer (via runtime polymorphism).
     */
    BitmapDeviceSharedPtr     mpGenericRenderer;
};


BitmapDevice::BitmapDevice( const basegfx::B2IBox&           rBounds,
                            const basegfx::B2IVector&        rBufferSize,
                            Format                           nScanlineFormat,
                            sal_Int32                        nScanlineStride,
                            sal_uInt8*                       pFirstScanline,
                            const RawMemorySharedArray&      rMem,
                            const PaletteMemorySharedVector& rPalette ) :
    mpImpl( new ImplBitmapDevice )
{
    mpImpl->mpMem = rMem;
    mpImpl->mpPalette = rPalette;
    mpImpl->maBounds = rBounds;
    mpImpl->maBufferSize = rBufferSize;
    mpImpl->mnScanlineFormat = nScanlineFormat;
    mpImpl->mnScanlineStride = nScanlineStride;
    mpImpl->mpFirstScanline  = pFirstScanline;
}

BitmapDevice::~BitmapDevice()
{
    // outline, because of internal ImplBitmapDevice
    SAL_INFO( "basebmp.bitmapdevice", "~BitmapDevice(" << this << ")" );
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

basegfx::B2IVector BitmapDevice::getBufferSize() const
{
    return mpImpl->maBufferSize;
}

Format BitmapDevice::getScanlineFormat() const
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

IBitmapDeviceDamageTrackerSharedPtr BitmapDevice::getDamageTracker() const
{
    return getDamageTracker_i();
}

void BitmapDevice::setDamageTracker( const IBitmapDeviceDamageTrackerSharedPtr& rDamage )
{
    setDamageTracker_i(rDamage);
}

PaletteMemorySharedVector BitmapDevice::getPalette() const
{
    return mpImpl->mpPalette;
}

bool BitmapDevice::isSharedBuffer( const BitmapDeviceSharedPtr& rOther ) const
{
    return rOther.get()->getBuffer().get() == getBuffer().get();
}

void BitmapDevice::clear( Color fillColor )
{
    clear_i( fillColor, mpImpl->maBounds );
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
    if( !rClip )
    {
        setPixel(rPt,lineColor,drawMode);
        return;
    }

    if( mpImpl->maBounds.isInside(rPt) )
    {
        if( isCompatibleClipMask( rClip ) )
            setPixel_i(rPt,lineColor,drawMode,rClip);
        else
            getGenericRenderer()->setPixel( rPt, lineColor, drawMode, rClip );
    }
}

Color BitmapDevice::getPixel( const basegfx::B2IPoint& rPt )
{
    if( mpImpl->maBounds.isInside(rPt) )
        return getPixel_i(rPt);

    return Color();
}

sal_uInt32 BitmapDevice::getPixelData( const basegfx::B2IPoint& rPt )
{
    if( mpImpl->maBounds.isInside(rPt) )
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
                mpImpl->maBounds,
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
                    mpImpl->maBounds,
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
                       mpImpl->maBounds,
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
    {
        if( isCompatibleClipMask( rClip ) )
            drawPolygon_i( rPoly,
                           mpImpl->maBounds,
                           lineColor, drawMode, rClip );
        else
            getGenericRenderer()->drawPolygon( rPoly, lineColor,
                                               drawMode, rClip );
    }
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
                           const basegfx::B2IBox&   rPermittedRange )
    {
        (void)rPt; (void)rPermittedRange;
        OSL_ASSERT( rPermittedRange.isInside(rPt) );
    }

    void assertImageRange( const basegfx::B2IBox& rRange,
                           const basegfx::B2IBox& rPermittedRange )
    {
#if OSL_DEBUG_LEVEL > 0
        basegfx::B2IBox aRange( rRange );
        aRange.intersect( rPermittedRange );

        OSL_ASSERT( aRange == rRange );
#else
        (void)rRange; (void)rPermittedRange;
#endif
    }

    // TODO(Q3): Move canvas/canvastools.hxx clipBlit() down
    // to basegfx, and use here!
    bool clipAreaImpl( ::basegfx::B2IBox&       io_rSourceArea,
                       ::basegfx::B2IPoint&     io_rDestPoint,
                       const ::basegfx::B2IBox& rSourceBounds,
                       const ::basegfx::B2IBox& rDestBounds )
    {
        const ::basegfx::B2IPoint aSourceTopLeft(
            io_rSourceArea.getMinimum() );

        ::basegfx::B2IBox aLocalSourceArea( io_rSourceArea );

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

        ::basegfx::B2IBox aLocalDestArea( io_rDestPoint + aUpperLeftOffset,
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

        io_rSourceArea = ::basegfx::B2IBox( aSourceTopLeft + aDestUpperLeftOffset,
                                            aSourceTopLeft + aDestLowerRightOffset );
        io_rDestPoint  = aLocalDestArea.getMinimum();

        return true;
    }

    // TODO(Q3): Move canvas/canvastools.hxx clipBlit() down
    // to basegfx, and use here!
    bool clipAreaImpl( ::basegfx::B2IBox&       io_rDestArea,
                       ::basegfx::B2IBox&       io_rSourceArea,
                       const ::basegfx::B2IBox& rDestBounds,
                       const ::basegfx::B2IBox& rSourceBounds )
    {
        // extract inherent scale
        const double nScaleX( io_rDestArea.getWidth() / (double)io_rSourceArea.getWidth() );
        const double nScaleY( io_rDestArea.getHeight() / (double)io_rSourceArea.getHeight() );

        // extract range origins
        const basegfx::B2IPoint   aDestTopLeft(
            io_rDestArea.getMinimum() );
        const ::basegfx::B2IPoint aSourceTopLeft(
            io_rSourceArea.getMinimum() );

        ::basegfx::B2IBox aLocalSourceArea( io_rSourceArea );

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

        ::basegfx::B2IBox aLocalDestArea( basegfx::fround(aDestTopLeft.getX() + nScaleX*aUpperLeftOffset.getX()),
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

        io_rSourceArea = ::basegfx::B2IBox( basegfx::fround(aSourceTopLeft.getX() + aDestUpperLeftOffset.getX()/nScaleX),
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
                               const basegfx::B2IBox&       rSrcRect,
                               const basegfx::B2IBox&       rDstRect,
                               DrawMode                     drawMode )
{
    const basegfx::B2IVector& rSrcSize( rSrcBitmap->getSize() );
    const basegfx::B2IBox     aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IBox           aSrcRange( rSrcRect );
    basegfx::B2IBox           aDestRange( rDstRect );

    if( clipAreaImpl( aDestRange,
                      aSrcRange,
                      mpImpl->maBounds,
                      aSrcBounds ))
    {
        assertImageRange(aDestRange,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        drawBitmap_i( rSrcBitmap, aSrcRange, aDestRange, drawMode );
    }
}

void BitmapDevice::drawBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                               const basegfx::B2IBox&       rSrcRect,
                               const basegfx::B2IBox&       rDstRect,
                               DrawMode                     drawMode,
                               const BitmapDeviceSharedPtr& rClip )
{
    if( !rClip )
    {
        drawBitmap(rSrcBitmap,rSrcRect,rDstRect,drawMode);
        return;
    }

    const basegfx::B2IVector& rSrcSize( rSrcBitmap->getSize() );
    const basegfx::B2IBox     aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IBox           aSrcRange( rSrcRect );
    basegfx::B2IBox           aDestRange( rDstRect );

    if( clipAreaImpl( aDestRange,
                      aSrcRange,
                      mpImpl->maBounds,
                      aSrcBounds ))
    {
        assertImageRange(aDestRange,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isCompatibleClipMask( rClip ) )
        {
            drawBitmap_i( rSrcBitmap, aSrcRange, aDestRange, drawMode, rClip );
        }
        else
        {
            getGenericRenderer()->drawBitmap( rSrcBitmap, rSrcRect,
                                              rDstRect, drawMode, rClip );
        }
    }
}

void BitmapDevice::drawMaskedColor( Color                        aSrcColor,
                                    const BitmapDeviceSharedPtr& rAlphaMask,
                                    const basegfx::B2IBox&       rSrcRect,
                                    const basegfx::B2IPoint&     rDstPoint )
{
    const basegfx::B2IVector& rSrcSize( rAlphaMask->getSize() );
    const basegfx::B2IBox     aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IBox           aSrcRange( rSrcRect );
    basegfx::B2IPoint         aDestPoint( rDstPoint );

    if( clipAreaImpl( aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
        assertImagePoint(aDestPoint,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isSharedBuffer(rAlphaMask) )
        {
            // src == dest, copy rAlphaMask beforehand
            // ---------------------------------------------------

            const basegfx::B2ITuple aSize( aSrcRange.getWidth(),
                                           aSrcRange.getHeight() );
            BitmapDeviceSharedPtr pAlphaCopy(
                cloneBitmapDevice( aSize,
                                   shared_from_this()) );
            basegfx::B2ITuple aGcc3WorkaroundTemporary;
            const basegfx::B2IBox aAlphaRange( aGcc3WorkaroundTemporary,
                                               aSize );
            pAlphaCopy->drawBitmap(rAlphaMask,
                                   aSrcRange,
                                   aAlphaRange,
                                   DrawMode_PAINT);
            drawMaskedColor_i( aSrcColor, pAlphaCopy, aAlphaRange, aDestPoint );
        }
        else
        {
            drawMaskedColor_i( aSrcColor, rAlphaMask, aSrcRange, aDestPoint );
        }
    }
}

void BitmapDevice::drawMaskedColor( Color                        aSrcColor,
                                    const BitmapDeviceSharedPtr& rAlphaMask,
                                    const basegfx::B2IBox&       rSrcRect,
                                    const basegfx::B2IPoint&     rDstPoint,
                                    const BitmapDeviceSharedPtr& rClip )
{
    if( !rClip )
    {
        drawMaskedColor(aSrcColor,rAlphaMask,rSrcRect,rDstPoint);
        return;
    }

    const basegfx::B2IVector& rSrcSize( rAlphaMask->getSize() );
    const basegfx::B2IBox     aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IBox           aSrcRange( rSrcRect );
    basegfx::B2IPoint         aDestPoint( rDstPoint );

    if( clipAreaImpl( aSrcRange,
                      aDestPoint,
                      aSrcBounds,
                      mpImpl->maBounds ))
    {
        assertImagePoint(aDestPoint,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isCompatibleClipMask( rClip ) )
        {
            if( isSharedBuffer(rAlphaMask) )
            {
                // src == dest, copy rAlphaMask beforehand
                // ---------------------------------------------------

                const basegfx::B2ITuple aSize( aSrcRange.getWidth(),
                                               aSrcRange.getHeight() );
                BitmapDeviceSharedPtr pAlphaCopy(
                    cloneBitmapDevice( aSize,
                                       shared_from_this()) );
                basegfx::B2ITuple aGcc3WorkaroundTemporary;
                const basegfx::B2IBox aAlphaRange( aGcc3WorkaroundTemporary,
                                                   aSize );
                pAlphaCopy->drawBitmap(rAlphaMask,
                                       aSrcRange,
                                       aAlphaRange,
                                       DrawMode_PAINT);
                drawMaskedColor_i( aSrcColor, pAlphaCopy, aAlphaRange, aDestPoint, rClip );
            }
            else
            {
                drawMaskedColor_i( aSrcColor, rAlphaMask, aSrcRange, aDestPoint, rClip );
            }
        }
        else
        {
            getGenericRenderer()->drawMaskedColor( aSrcColor, rAlphaMask,
                                                   rSrcRect, rDstPoint, rClip );
        }
    }
}

void BitmapDevice::drawMaskedBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                                     const BitmapDeviceSharedPtr& rMask,
                                     const basegfx::B2IBox&       rSrcRect,
                                     const basegfx::B2IBox&       rDstRect,
                                     DrawMode                     drawMode )
{
    OSL_ASSERT( rMask->getSize() == rSrcBitmap->getSize() );

    const basegfx::B2IVector& rSrcSize( rSrcBitmap->getSize() );
    const basegfx::B2IBox     aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IBox           aSrcRange( rSrcRect );
    basegfx::B2IBox           aDestRange( rDstRect );

    if( clipAreaImpl( aDestRange,
                      aSrcRange,
                      mpImpl->maBounds,
                      aSrcBounds ))
    {
        assertImageRange(aDestRange,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        drawMaskedBitmap_i( rSrcBitmap, rMask, aSrcRange, aDestRange, drawMode );
    }
}

void BitmapDevice::drawMaskedBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                                     const BitmapDeviceSharedPtr& rMask,
                                     const basegfx::B2IBox&       rSrcRect,
                                     const basegfx::B2IBox&       rDstRect,
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
    const basegfx::B2IBox     aSrcBounds( 0,0,rSrcSize.getX(),rSrcSize.getY() );
    basegfx::B2IBox           aSrcRange( rSrcRect );
    basegfx::B2IBox           aDestRange( rDstRect );

    if( clipAreaImpl( aDestRange,
                      aSrcRange,
                      mpImpl->maBounds,
                      aSrcBounds ))
    {
        assertImageRange(aDestRange,mpImpl->maBounds);
        assertImageRange(aSrcRange,aSrcBounds);

        if( isCompatibleClipMask( rClip ) )
        {
            drawMaskedBitmap_i( rSrcBitmap, rMask, aSrcRange, aDestRange, drawMode, rClip );
        }
        else
        {
            getGenericRenderer()->drawMaskedBitmap( rSrcBitmap, rMask, rSrcRect,
                                                    rDstRect, drawMode, rClip );
        }
    }
}



/** Standard clip and alpha masks
 */
struct StdMasks
{
    typedef PixelFormatTraits_GREY1_MSB   clipmask_format_traits;
    typedef PixelFormatTraits_GREY8       alphamask_format_traits;

    /// Clipmask: 0 means opaque
    static const bool clipmask_polarity  = false;

    /// Alpha mask: 0 means fully transparent
    static const bool alphamask_polarity = true;
};


// Some compilers don't like the nested template wrap_accessor
// reference in the parameter list - being slightly less type safe,
// then.
#ifndef BASEBMP_NO_NESTED_TEMPLATE_PARAMETER

/// Produces a specialized renderer for the given pixel format
template< class FormatTraits, class MaskTraits >
BitmapDeviceSharedPtr createRenderer(
    const basegfx::B2IBox&                                       rBounds,
    const basegfx::B2IVector&                                    rBufferSize,
    Format                                                       nScanlineFormat,
    sal_Int32                                                    nScanlineStride,
    sal_uInt8*                                                   pFirstScanline,
    typename FormatTraits::raw_accessor_type const&              rRawAccessor,
    typename FormatTraits::accessor_selector::template wrap_accessor<
          typename FormatTraits::raw_accessor_type>::type const& rAccessor,
    boost::shared_array< sal_uInt8 >                             pMem,
    const PaletteMemorySharedVector&                             pPal,
    const IBitmapDeviceDamageTrackerSharedPtr&                   pDamage )
#else

template< class FormatTraits, class MaskTraits, class Accessor >
BitmapDeviceSharedPtr createRenderer(
    const basegfx::B2IBox&                                       rBounds,
    const basegfx::B2IVector&                                    rBufferSize,
    Format                                                       nScanlineFormat,
    sal_Int32                                                    nScanlineStride,
    sal_uInt8*                                                   pFirstScanline,
    typename FormatTraits::raw_accessor_type const&              rRawAccessor,
    Accessor const&                                              rAccessor,
    boost::shared_array< sal_uInt8 >                             pMem,
    const PaletteMemorySharedVector&                             pPal,
    const IBitmapDeviceDamageTrackerSharedPtr&                   pDamage )

#endif
{
    typedef typename FormatTraits::iterator_type                Iterator;
    typedef BitmapRenderer< Iterator,
                            typename FormatTraits::raw_accessor_type,
                            typename FormatTraits::accessor_selector,
                            MaskTraits >                        Renderer;

    return BitmapDeviceSharedPtr(
        new Renderer( rBounds,
                      rBufferSize,
                      nScanlineFormat,
                      nScanlineStride,
                      pFirstScanline,
                      Iterator(
                          reinterpret_cast<typename Iterator::value_type*>(
                              pFirstScanline),
                          nScanlineStride),
                      rRawAccessor,
                      rAccessor,
                      pMem,
                      pPal,
                      pDamage ));
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
    const basegfx::B2IBox&                     rBounds,
    const basegfx::B2IVector&                  rBufferSize,
    Format                                     nScanlineFormat,
    sal_Int32                                  nScanlineStride,
    sal_uInt8*                                 pFirstScanline,
    boost::shared_array< sal_uInt8 >           pMem,
    const PaletteMemorySharedVector&           pPal,
    const IBitmapDeviceDamageTrackerSharedPtr& pDamage )
{
    return createRenderer<FormatTraits,
                          MaskTraits>(rBounds,
                                      rBufferSize,
                                      nScanlineFormat,
                                      nScanlineStride,
                                      pFirstScanline,
                                      typename FormatTraits::raw_accessor_type(),
                                      typename FormatTraits::accessor_selector::template
                                      wrap_accessor<
                                          typename FormatTraits::raw_accessor_type>::type(),
                                      pMem,
                                      pPal,
                                      pDamage);
}

template< class FormatTraits, class MaskTraits >
BitmapDeviceSharedPtr createRenderer(
    const basegfx::B2IBox&                     rBounds,
    const basegfx::B2IVector&                  rBufferSize,
    Format                                     nScanlineFormat,
    sal_Int32                                  nScanlineStride,
    sal_uInt8*                                 pFirstScanline,
    boost::shared_array< sal_uInt8 >           pMem,
    PaletteMemorySharedVector                  pPal,
    int                                        nBitsPerPixel,
    const IBitmapDeviceDamageTrackerSharedPtr& pDamage )
{
    pPal = createStandardPalette(pPal,
                                 1UL << nBitsPerPixel);

    OSL_ASSERT(pPal);
    return createRenderer<FormatTraits,
                          MaskTraits>(rBounds,
                                      rBufferSize,
                                      nScanlineFormat,
                                      nScanlineStride,
                                      pFirstScanline,
                                      typename FormatTraits::raw_accessor_type(),
                                      typename FormatTraits::accessor_selector::template
                                          wrap_accessor<
                                      typename FormatTraits::raw_accessor_type>::type(
                                          &pPal->at(0),
                                          pPal->size()),
                                      pMem,
                                      pPal,
                                      pDamage);
}


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


namespace
{
BitmapDeviceSharedPtr createBitmapDeviceImplInner( const basegfx::B2IVector&                  rSize,
                                                   bool                                       bTopDown,
                                                   Format                                     nScanlineFormat,
                                                   boost::shared_array< sal_uInt8 >           pMem,
                                                   PaletteMemorySharedVector                  pPal,
                                                   const basegfx::B2IBox*                     pSubset,
                                                   const IBitmapDeviceDamageTrackerSharedPtr& rDamage )
{
    OSL_ASSERT(rSize.getX() > 0 && rSize.getY() > 0);

    if( nScanlineFormat <= FORMAT_NONE ||
        nScanlineFormat >  FORMAT_MAX )
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
        32, // THIRTYTWO_BIT_TC_MASK_BGRA
        32, // THIRTYTWO_BIT_TC_MASK_ARGB
        32, // THIRTYTWO_BIT_TC_MASK_ABGR
        32, // THIRTYTWO_BIT_TC_MASK_RGBA
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

    const sal_uInt32 nWidth(nScanlineStride < 0 ? -nScanlineStride : nScanlineStride);
    const sal_uInt32 nHeight(rSize.getY());

    if (nHeight && nWidth && nWidth > SAL_MAX_INT32 / nHeight)
    {
        SAL_WARN( "basebmp", "suspicious massive alloc " << nWidth << " * " << nHeight);
        return BitmapDeviceSharedPtr();
    }

    const std::size_t nMemSize(nWidth * nHeight);

    if( !pMem )
    {
        pMem.reset(
            reinterpret_cast<sal_uInt8*>(rtl_allocateMemory( nMemSize )),
            &rtl_freeMemory );
        if (pMem.get() == 0 && nMemSize != 0)
            return BitmapDeviceSharedPtr();
        memset(pMem.get(), 0, nMemSize);
    }

    sal_uInt8* pFirstScanline = nScanlineStride < 0 ?
        pMem.get() + nMemSize + nScanlineStride : pMem.get();

    // shrink render area to given subset, if given
    basegfx::B2IBox aBounds(0,0,rSize.getX(),rSize.getY());
    if( pSubset )
        aBounds.intersect( *pSubset );

    switch( nScanlineFormat )
    {
        // ----------------------------------------------------------------------
        // one bit formats

        case FORMAT_ONE_BIT_MSB_GREY:
            return createRenderer<PixelFormatTraits_GREY1_MSB,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );

        case FORMAT_ONE_BIT_LSB_GREY:
            return createRenderer<PixelFormatTraits_GREY1_LSB,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );

        case FORMAT_ONE_BIT_MSB_PAL:
            return createRenderer<PixelFormatTraits_PAL1_MSB,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat], rDamage );

        case FORMAT_ONE_BIT_LSB_PAL:
            return createRenderer<PixelFormatTraits_PAL1_LSB,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat], rDamage );


        // ----------------------------------------------------------------------
        // four bit formats

        case FORMAT_FOUR_BIT_MSB_GREY:
            return createRenderer<PixelFormatTraits_GREY4_MSB,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );

        case FORMAT_FOUR_BIT_LSB_GREY:
            return createRenderer<PixelFormatTraits_GREY4_LSB,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );

        case FORMAT_FOUR_BIT_MSB_PAL:
            return createRenderer<PixelFormatTraits_PAL4_MSB,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat], rDamage );

        case FORMAT_FOUR_BIT_LSB_PAL:
            return createRenderer<PixelFormatTraits_PAL4_LSB,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat], rDamage );


        // ----------------------------------------------------------------------
        // eight bit formats

        case FORMAT_EIGHT_BIT_GREY:
            return createRenderer<PixelFormatTraits_GREY8,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );

        case FORMAT_EIGHT_BIT_PAL:
            return createRenderer<PixelFormatTraits_PAL8,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat], rDamage );


        // ----------------------------------------------------------------------
        // sixteen bit formats

        case FORMAT_SIXTEEN_BIT_LSB_TC_MASK:
            return createRenderer<PixelFormatTraits_RGB16_565_LSB,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );

        case FORMAT_SIXTEEN_BIT_MSB_TC_MASK:
            return createRenderer<PixelFormatTraits_RGB16_565_MSB,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );


        // ----------------------------------------------------------------------
        // twentyfour bit formats
        case FORMAT_TWENTYFOUR_BIT_TC_MASK:
            return createRenderer<PixelFormatTraits_BGR24,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );


        // ----------------------------------------------------------------------
        // thirtytwo bit formats

        case FORMAT_THIRTYTWO_BIT_TC_MASK_BGRA:
            return createRenderer<PixelFormatTraits_BGRX32_8888,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );

        case FORMAT_THIRTYTWO_BIT_TC_MASK_ARGB:
            return createRenderer<PixelFormatTraits_XRGB32_8888,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );

        case FORMAT_THIRTYTWO_BIT_TC_MASK_ABGR:
            return createRenderer<PixelFormatTraits_XBGR32_8888,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );

        case FORMAT_THIRTYTWO_BIT_TC_MASK_RGBA:
            return createRenderer<PixelFormatTraits_RGBX32_8888,StdMasks>(
                aBounds, rSize, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal, rDamage );

        default:
            assert(false); // this cannot happen
    }

    // TODO(F3): other formats not yet implemented
    return BitmapDeviceSharedPtr();
}

BitmapDeviceSharedPtr createBitmapDeviceImpl( const basegfx::B2IVector&                  rSize,
                                              bool                                       bTopDown,
                                              Format                                     nScanlineFormat,
                                              boost::shared_array< sal_uInt8 >           pMem,
                                              PaletteMemorySharedVector                  pPal,
                                              const basegfx::B2IBox*                     pSubset,
                                              const IBitmapDeviceDamageTrackerSharedPtr& rDamage )
{
    BitmapDeviceSharedPtr result( createBitmapDeviceImplInner( rSize, bTopDown, nScanlineFormat, pMem, pPal, pSubset, rDamage ) );

#ifdef SAL_LOG_INFO
    std::ostringstream subset;

    if (pSubset)
        subset << " subset=" << pSubset->getWidth() << "x" << pSubset->getHeight() << "@(" << pSubset->getMinX() << "," << pSubset->getMinY() << ")";

    SAL_INFO( "basebmp.bitmapdevice",
              "createBitmapDevice: "
              << rSize.getX() << "x" << rSize.getY()
              << (bTopDown ? " top-down " : " bottom-up ")
              << formatName(nScanlineFormat)
              << subset.str()
              << " = " << result.get() );
#endif
    return result;
}
} // namespace


BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector& rSize,
                                          bool                      bTopDown,
                                          Format                    nScanlineFormat )
{
    return createBitmapDeviceImpl( rSize,
                                   bTopDown,
                                   nScanlineFormat,
                                   boost::shared_array< sal_uInt8 >(),
                                   PaletteMemorySharedVector(),
                                   NULL,
                                   IBitmapDeviceDamageTrackerSharedPtr() );
}

BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector&        rSize,
                                          bool                             bTopDown,
                                          Format                           nScanlineFormat,
                                          const PaletteMemorySharedVector& rPalette )
{
    return createBitmapDeviceImpl( rSize,
                                   bTopDown,
                                   nScanlineFormat,
                                   boost::shared_array< sal_uInt8 >(),
                                   rPalette,
                                   NULL,
                                   IBitmapDeviceDamageTrackerSharedPtr() );
}

BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector&        rSize,
                                          bool                             bTopDown,
                                          Format                           nScanlineFormat,
                                          const RawMemorySharedArray&      rMem,
                                          const PaletteMemorySharedVector& rPalette )
{
    return createBitmapDeviceImpl( rSize,
                                   bTopDown,
                                   nScanlineFormat,
                                   rMem,
                                   rPalette,
                                   NULL,
                                   IBitmapDeviceDamageTrackerSharedPtr() );
}

BitmapDeviceSharedPtr subsetBitmapDevice( const BitmapDeviceSharedPtr& rProto,
                                          const basegfx::B2IBox&       rSubset )
{
    SAL_INFO( "basebmp.bitmapdevice", "subsetBitmapDevice: proto=" << rProto.get() );
    return createBitmapDeviceImpl( rProto->getSize(),
                                   rProto->isTopDown(),
                                   rProto->getScanlineFormat(),
                                   rProto->getBuffer(),
                                   rProto->getPalette(),
                                   &rSubset,
                                   rProto->getDamageTracker() );
}

BitmapDeviceSharedPtr cloneBitmapDevice( const basegfx::B2IVector&    rSize,
                                         const BitmapDeviceSharedPtr& rProto )
{
    return createBitmapDeviceImpl( rSize,
                                   rProto->isTopDown(),
                                   rProto->getScanlineFormat(),
                                   boost::shared_array< sal_uInt8 >(),
                                   rProto->getPalette(),
                                   NULL,
                                   rProto->getDamageTracker() );
}


/// Clone our device, with GenericImageAccessor to handle all formats
BitmapDeviceSharedPtr BitmapDevice::getGenericRenderer() const
{
    return mpImpl->mpGenericRenderer;
}

} // namespace basebmp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
