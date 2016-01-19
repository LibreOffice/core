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

#include <sal/config.h>

#include <cassert>
#include <string.h>

#include <basebmp/bitmapdevice.hxx>

#include <compositeiterator.hxx>
#include <iteratortraits.hxx>

#include <accessor.hxx>
#include <accessortraits.hxx>
#include <accessoradapters.hxx>

#include <basebmp/color.hxx>
#include <colormisc.hxx>

#include <greylevelformats.hxx>
#include <paletteformats.hxx>
#include <rgbmaskpixelformats.hxx>
#include <rgb24pixelformats.hxx>

#include <basebmp/scanlineformats.hxx>
#include <vigra/copyimage.hxx>
#include <genericcolorimageaccessor.hxx>

#include <tools.hxx>

#include <rtl/alloc.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <o3tl/enumarray.hxx>

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

namespace basebmp
{

static const o3tl::enumarray<Format,sal_uInt8> bitsPerPixel =
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

namespace
{
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
     */
    template< class DestIterator,
              class RawAccessor,
              class AccessorSelector > class BitmapRenderer :
                  public BitmapDevice
    {
    public:
        typedef DestIterator                                               dest_iterator_type;
        typedef RawAccessor                                                raw_accessor_type;
        typedef AccessorSelector                                           accessor_selector;

        typedef typename AccessorSelector::template wrap_accessor<
            raw_accessor_type >::type                                      dest_accessor_type;

        typedef AccessorTraits< dest_accessor_type >                       accessor_traits;

        typedef AccessorTraits< raw_accessor_type >                        raw_accessor_traits;

        dest_iterator_type                      maBegin;
        dest_accessor_type                      maAccessor;
        raw_accessor_type                       maRawAccessor;




        BitmapRenderer( const basegfx::B2IBox&                     rBounds,
                        Format                                     nScanlineFormat,
                        sal_Int32                                  nScanlineStride,
                        sal_uInt8*                                 pFirstScanline,
                        dest_iterator_type                         begin,
                        raw_accessor_type                          rawAccessor,
                        dest_accessor_type                         accessor,
                        const RawMemorySharedArray&                rMem,
                        const PaletteMemorySharedVector&           rPalette ) :
            BitmapDevice( rBounds, nScanlineFormat,
                          nScanlineStride, pFirstScanline, rMem, rPalette ),
            maBegin( begin ),
            maAccessor( accessor ),
            maRawAccessor( rawAccessor )
        {}

    private:

        virtual Color getPixel_i(const basegfx::B2IPoint& rPt ) override
        {
            const DestIterator pixel( maBegin +
                                      vigra::Diff2D(rPt.getX(),
                                                    rPt.getY()) );
            return maAccessor(pixel);
        }

        template< typename Iterator, typename Acc > static
        void implDrawBitmapGeneric(const BitmapDeviceSharedPtr& rSrcBitmap,
                                   const Iterator&              begin,
                                   const Acc&                   acc)
        {
            const basegfx::B2IVector& rSrcSize( rSrcBitmap->getSize() );
            const basegfx::B2IBox     aRect(0, 0, rSrcSize.getX(),rSrcSize.getY());

            GenericColorImageAccessor aSrcAcc( rSrcBitmap );

            vigra::copyImage( vigra::Diff2D(), vigra::Diff2D() + bottomRight(aRect), aSrcAcc,
                              begin, acc );
        }

        virtual void convertBitmap_i(const BitmapDeviceSharedPtr& rSrcBitmap) override
        {
            implDrawBitmapGeneric(rSrcBitmap, maBegin, maAccessor);
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

    /// Scanline format, as provided at the constructor
    Format                    mnScanlineFormat;

    /// Scanline stride. Negative for bottom-to-top formats
    sal_Int32                 mnScanlineStride;

    /// raw ptr to 0th scanline. used for cloning a generic renderer
    sal_uInt8*                mpFirstScanline;
};


BitmapDevice::BitmapDevice( const basegfx::B2IBox&           rBounds,
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

Format BitmapDevice::getScanlineFormat() const
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

Color BitmapDevice::getPixel( const basegfx::B2IPoint& rPt )
{
    if( mpImpl->maBounds.isInside(rPt) )
        return getPixel_i(rPt);

    return Color();
}

void BitmapDevice::convertBitmap( const BitmapDeviceSharedPtr& rSrcBitmap )
{
    convertBitmap_i(rSrcBitmap);
}

/// Produces a specialized renderer for the given pixel format
template< class FormatTraits >
BitmapDeviceSharedPtr createRenderer(
    const basegfx::B2IBox&                                       rBounds,
    Format                                                       nScanlineFormat,
    sal_Int32                                                    nScanlineStride,
    sal_uInt8*                                                   pFirstScanline,
    typename FormatTraits::raw_accessor_type const&              rRawAccessor,
    typename FormatTraits::accessor_selector::template wrap_accessor<
          typename FormatTraits::raw_accessor_type>::type const& rAccessor,
    boost::shared_array< sal_uInt8 >                             pMem,
    const PaletteMemorySharedVector&                             pPal )
{
    typedef typename FormatTraits::iterator_type                Iterator;
    typedef BitmapRenderer< Iterator,
                            typename FormatTraits::raw_accessor_type,
                            typename FormatTraits::accessor_selector
                            >                        Renderer;

    return BitmapDeviceSharedPtr(
        new Renderer( rBounds,
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
                      pPal ));
}

/// Create standard grey level palette
PaletteMemorySharedVector createStandardPalette(
    const PaletteMemorySharedVector& pPal,
    sal_Int32                        nNumEntries )
{
    if( pPal || nNumEntries <= 0 )
        return pPal;

    std::shared_ptr< std::vector<Color> > pLocalPal(
        new std::vector<Color>(nNumEntries) );

    const sal_Int32 nIncrement( 0x00FFFFFF/nNumEntries );
    --nNumEntries;
    for( sal_Int32 i=0, c=0; i<nNumEntries; ++i,c+=nIncrement )
        pLocalPal->at(i) = Color(0xFF000000 | c);

    pLocalPal->at(nNumEntries) = Color(0xFFFFFFFF);

    return pLocalPal;
}

template< class FormatTraits >
BitmapDeviceSharedPtr createRenderer(
    const basegfx::B2IBox&                     rBounds,
    Format                                     nScanlineFormat,
    sal_Int32                                  nScanlineStride,
    sal_uInt8*                                 pFirstScanline,
    boost::shared_array< sal_uInt8 >           pMem,
    const PaletteMemorySharedVector&           pPal )
{
    return createRenderer<FormatTraits>(rBounds,
                                      nScanlineFormat,
                                      nScanlineStride,
                                      pFirstScanline,
                                      typename FormatTraits::raw_accessor_type(),
                                      typename FormatTraits::accessor_selector::template
                                      wrap_accessor<
                                          typename FormatTraits::raw_accessor_type>::type(),
                                      pMem,
                                      pPal );
}

template< class FormatTraits >
BitmapDeviceSharedPtr createRenderer(
    const basegfx::B2IBox&                     rBounds,
    Format                                     nScanlineFormat,
    sal_Int32                                  nScanlineStride,
    sal_uInt8*                                 pFirstScanline,
    boost::shared_array< sal_uInt8 >           pMem,
    PaletteMemorySharedVector                  pPal,
    int                                        nBitsPerPixel )
{
    pPal = createStandardPalette(pPal,
                                 1UL << nBitsPerPixel);

    OSL_ASSERT(pPal);
    return createRenderer<FormatTraits>(rBounds,
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
                                      pPal );
}

namespace
{
BitmapDeviceSharedPtr createBitmapDeviceImplInner( const basegfx::B2IVector&                  rSize,
                                                   Format                                     nScanlineFormat,
                                                   boost::shared_array< sal_uInt8 >           pMem,
                                                   PaletteMemorySharedVector                  pPal )
{
    OSL_ASSERT(rSize.getX() > 0 && rSize.getY() > 0);

    if( nScanlineFormat <= Format::NONE ||
        nScanlineFormat >  Format::LAST )
        return BitmapDeviceSharedPtr();

    sal_uInt8 nBitsPerPixel = bitsPerPixel[nScanlineFormat];
    if (rSize.getX() > (SAL_MAX_INT32-7) / nBitsPerPixel)
    {
        SAL_WARN("basebmp", "suspicious bitmap width " <<
                 rSize.getX() << " for depth " << nBitsPerPixel);
        return BitmapDeviceSharedPtr();
    }

    const sal_uInt32 nScanlineStride = getBitmapDeviceStrideForWidth(nScanlineFormat, rSize.getX());

    const sal_uInt32 nHeight(rSize.getY());

    if (nHeight && nScanlineStride && nScanlineStride > SAL_MAX_INT32 / nHeight)
    {
        SAL_WARN( "basebmp", "suspicious massive alloc " << nScanlineStride << " * " << nHeight);
        return BitmapDeviceSharedPtr();
    }

    const std::size_t nMemSize(nScanlineStride * nHeight);

    if( !pMem )
    {
        pMem.reset(
            static_cast<sal_uInt8*>(rtl_allocateMemory( nMemSize )),
            &rtl_freeMemory );
        if (pMem.get() == nullptr && nMemSize != 0)
            return BitmapDeviceSharedPtr();
        memset(pMem.get(), 0, nMemSize);
    }

    sal_uInt8* pFirstScanline = pMem.get();

    basegfx::B2IBox aBounds(0,0,rSize.getX(),rSize.getY());

    switch( nScanlineFormat )
    {

        // one bit formats

        case Format::OneBitMsbGrey:
            return createRenderer<PixelFormatTraits_GREY1_MSB>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::OneBitLsbGrey:
            return createRenderer<PixelFormatTraits_GREY1_LSB>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::OneBitMsbPal:
            return createRenderer<PixelFormatTraits_PAL1_MSB>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat] );

        case Format::OneBitLsbPal:
            return createRenderer<PixelFormatTraits_PAL1_LSB>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat] );



        // four bit formats

        case Format::FourBitMsbGrey:
            return createRenderer<PixelFormatTraits_GREY4_MSB>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::FourBitLsbGrey:
            return createRenderer<PixelFormatTraits_GREY4_LSB>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::FourBitMsbPal:
            return createRenderer<PixelFormatTraits_PAL4_MSB>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat] );

        case Format::FourBitLsbPal:
            return createRenderer<PixelFormatTraits_PAL4_LSB>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat] );



        // eight bit formats

        case Format::EightBitGrey:
            return createRenderer<PixelFormatTraits_GREY8>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::EightBitPal:
            return createRenderer<PixelFormatTraits_PAL8>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal,
                bitsPerPixel[nScanlineFormat] );



        // sixteen bit formats

        case Format::SixteenBitLsbTcMask:
            return createRenderer<PixelFormatTraits_RGB16_565_LSB>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::SixteenBitMsbTcMask:
            return createRenderer<PixelFormatTraits_RGB16_565_MSB>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        // twentyfour bit formats
        case Format::TwentyFourBitTcMask:
            return createRenderer<PixelFormatTraits_BGR24>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        // thirtytwo bit formats

        case Format::ThirtyTwoBitTcMaskBGRA:
            return createRenderer<PixelFormatTraits_BGRA32_8888>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::ThirtyTwoBitTcMaskARGB:
            return createRenderer<PixelFormatTraits_ARGB32_8888>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::ThirtyTwoBitTcMaskABGR:
            return createRenderer<PixelFormatTraits_ABGR32_8888>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        case Format::ThirtyTwoBitTcMaskRGBA:
            return createRenderer<PixelFormatTraits_RGBA32_8888>(
                aBounds, nScanlineFormat, nScanlineStride,
                pFirstScanline, pMem, pPal );

        default:
            assert(false); // this cannot happen
    }

    // TODO(F3): other formats not yet implemented
    return BitmapDeviceSharedPtr();
}

BitmapDeviceSharedPtr createBitmapDeviceImpl( const basegfx::B2IVector&                  rSize,
                                              Format                                     nScanlineFormat,
                                              boost::shared_array< sal_uInt8 >           pMem,
                                              PaletteMemorySharedVector                  pPal )
{
    BitmapDeviceSharedPtr result( createBitmapDeviceImplInner( rSize, nScanlineFormat, pMem, pPal ) );

#ifdef SAL_LOG_INFO
    std::ostringstream subset;

    SAL_INFO( "basebmp.bitmapdevice",
              "createBitmapDevice: "
              << rSize.getX() << "x" << rSize.getY()
              << subset.str()
              << " = " << result.get() );
#endif
    return result;
}
} // namespace

sal_Int32 getBitmapDeviceStrideForWidth(Format nScanlineFormat, sal_Int32 nWidth)
{
    sal_uInt8 nBitsPerPixel = bitsPerPixel[nScanlineFormat];
    // round up to full 8 bit, divide by 8
    sal_Int32 nScanlineStride = (nWidth*nBitsPerPixel + 7) >> 3;

    // pixman (cairo) and GDI (windows) pad to multiples of 32bits
    // so do the same to be easily compatible
    nScanlineStride = (nScanlineStride + 3) & ~0x3;

    return nScanlineStride;
}

BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector& rSize,
                                          Format                    nScanlineFormat )
{
    return createBitmapDeviceImpl( rSize,
                                   nScanlineFormat,
                                   boost::shared_array< sal_uInt8 >(),
                                   PaletteMemorySharedVector() );
}

BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector&        rSize,
                                          Format                           nScanlineFormat,
                                          const PaletteMemorySharedVector& rPalette )
{
    return createBitmapDeviceImpl( rSize,
                                   nScanlineFormat,
                                   boost::shared_array< sal_uInt8 >(),
                                   rPalette );
}

BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector&        rSize,
                                          Format                           nScanlineFormat,
                                          const RawMemorySharedArray&      rMem,
                                          const PaletteMemorySharedVector& rPalette )
{
    return createBitmapDeviceImpl( rSize,
                                   nScanlineFormat,
                                   rMem,
                                   rPalette );
}

BitmapDeviceSharedPtr cloneBitmapDevice(const BitmapDeviceSharedPtr& rProto)
{
    const basegfx::B2IVector& rSrcSize(rProto->getSize());

    BitmapDeviceSharedPtr aCopy = createBitmapDeviceImpl(rSrcSize,
                                   rProto->getScanlineFormat(),
                                   boost::shared_array< sal_uInt8 >(),
                                   rProto->getPalette() );

    const sal_Int32 nSrcHeight = rSrcSize.getY();
    char* dstBuf =  reinterpret_cast<char*>(aCopy->getBuffer().get());
    char* srcBuf =  reinterpret_cast<char*>(rProto->getBuffer().get());
    const sal_Int32 nStride = rProto->getScanlineStride();
    memcpy(dstBuf, srcBuf, nStride * nSrcHeight);

    return aCopy;
}

} // namespace basebmp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
