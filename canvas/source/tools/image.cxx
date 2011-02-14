/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <canvas/canvastools.hxx>
#include <canvas/parametricpolypolygon.hxx>

#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>

#include <vcl/canvastools.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>

#include "image.hxx"

#define CANVAS_IMAGE_CXX
#include "image_sysprereq.h"

//////////////////////////////////////////////////////////////////////////////////
// platform-dependend includes [wrapped into their own namepsaces]
//////////////////////////////////////////////////////////////////////////////////

#if defined(WNT)
# if defined _MSC_VER
# pragma warning(push,1)
# endif

    namespace  win32
    {
        #undef DECLARE_HANDLE
        #undef WB_LEFT
        #undef WB_RIGHT
        #undef APIENTRY
        #define WIN32_LEAN_AND_MEAN
        #define NOMINMAX
        #include <windows.h>
    }

# if defined _MSC_VER
# pragma warning(pop)
# endif
#elif defined(OS2)
    namespace os2
    {
        #include <svpm.h>
    }
#else
#if !defined(QUARTZ)
    namespace unx
    {
        #include <X11/Xlib.h>
    }
#endif
#endif

#include <algorithm>

using namespace ::com::sun::star;

namespace canvas { namespace
{
    //////////////////////////////////////////////////////////////////////////////////
    // TransAffineFromAffineMatrix
    //////////////////////////////////////////////////////////////////////////////////

    ::agg::trans_affine transAffineFromAffineMatrix( const geometry::AffineMatrix2D& m )
    {
        return agg::trans_affine(m.m00,
                                 m.m10,
                                 m.m01,
                                 m.m11,
                                 m.m02,
                                 m.m12);
    }

    //////////////////////////////////////////////////////////////////////////////////
    // TransAffineFromB2DHomMatrix
    //////////////////////////////////////////////////////////////////////////////////

    ::agg::trans_affine transAffineFromB2DHomMatrix( const ::basegfx::B2DHomMatrix& m )
    {
        return agg::trans_affine(m.get(0,0),
                                 m.get(1,0),
                                 m.get(0,1),
                                 m.get(1,1),
                                 m.get(0,2),
                                 m.get(1,2));
    }

    //////////////////////////////////////////////////////////////////////////////////
    // ARGB
    //////////////////////////////////////////////////////////////////////////////////

    struct ARGBColor
    {
        sal_uInt8 a;
        sal_uInt8 r;
        sal_uInt8 g;
        sal_uInt8 b;
    };

    /// ARGB color
    union ARGB
    {
        ARGBColor  Color;
        sal_uInt32 color;

        ARGB() :
            color(0)
        {
        }

        explicit ARGB( sal_uInt32 _color ) :
            color(_color)
        {
        }

        ARGB( sal_uInt8 _a,
              sal_uInt8 _r,
              sal_uInt8 _g,
              sal_uInt8 _b )
        {
            Color.a = _a;
            Color.r = _r;
            Color.g = _g;
            Color.b = _b;
        }

        ARGB( sal_uInt32                                       default_color,
              const ::com::sun::star::uno::Sequence< double >& sequence ) :
            color(default_color)
        {
            if(sequence.getLength() > 2)
            {
                Color.r = static_cast<sal_uInt8>(255.0f*sequence[0]);
                Color.g = static_cast<sal_uInt8>(255.0f*sequence[1]);
                Color.b = static_cast<sal_uInt8>(255.0f*sequence[2]);
                if(sequence.getLength() > 3)
                    Color.a = static_cast<sal_uInt8>(255.0f*sequence[3]);
            }
        }

        ARGB( const ARGB& rhs ) :
            color( rhs.color )
        {
        }

        ARGB &operator=( const ARGB &rhs )
        {
            color=rhs.color;
            return *this;
        }
    };

    //////////////////////////////////////////////////////////////////////////////////
    // setupState
    //////////////////////////////////////////////////////////////////////////////////

    /// Calc common output state from XCanvas parameters
    void setupState( ::basegfx::B2DHomMatrix&                        o_rViewTransform,
                     ::basegfx::B2DHomMatrix&                        o_rRenderTransform,
                     ::std::auto_ptr< ::basegfx::B2DPolyPolygon >&   o_rViewClip,
                     ::std::auto_ptr< ::basegfx::B2DPolyPolygon >&   o_rRenderClip,
                     ARGB&                                           o_rRenderColor,
                     const rendering::ViewState&                     viewState,
                     const rendering::RenderState&                   renderState )
    {
        ::basegfx::unotools::homMatrixFromAffineMatrix(o_rRenderTransform,
                                                       renderState.AffineTransform);
        ::basegfx::unotools::homMatrixFromAffineMatrix(o_rViewTransform,
                                                       viewState.AffineTransform);

        o_rRenderColor = ARGB(0xFFFFFFFF,
                              renderState.DeviceColor);

        // TODO(F3): handle compositing modes

        if( viewState.Clip.is() )
        {
            ::basegfx::B2DPolyPolygon aViewClip(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D( viewState.Clip ));

            if(aViewClip.areControlPointsUsed())
                aViewClip = ::basegfx::tools::adaptiveSubdivideByAngle(aViewClip);

            o_rViewClip.reset( new ::basegfx::B2DPolyPolygon( aViewClip ) );
        }

        if( renderState.Clip.is() )
        {
            ::basegfx::B2DPolyPolygon aRenderClip(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D( viewState.Clip ) );

            if(aRenderClip.areControlPointsUsed())
                aRenderClip = ::basegfx::tools::adaptiveSubdivideByAngle(aRenderClip);

            o_rRenderClip.reset( new ::basegfx::B2DPolyPolygon( aRenderClip ) );
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // clipAndTransformPolygon
    //////////////////////////////////////////////////////////////////////////////////

    /** Clip and transform given polygon

        @param io_rClippee
        Polygon to clip

        @param bIsFilledPolyPolygon
        When true, the polygon is clipped as if it was to be rendered
        with fill, when false, the polygon is clipped as if it was to
        be rendered with stroking.
     */
    void clipAndTransformPolygon( ::basegfx::B2DPolyPolygon&         io_rClippee,
                                  bool                               bIsFilledPolyPolygon,
                                  const ::basegfx::B2DHomMatrix&     rViewTransform,
                                  const ::basegfx::B2DHomMatrix&     rRenderTransform,
                                  const ::basegfx::B2DPolyPolygon*   pViewClip,
                                  const ::basegfx::B2DPolyPolygon*   pRenderClip )
    {
        ::basegfx::B2DPolyPolygon aPolyPolygon(io_rClippee);
        io_rClippee.clear();

        // clip contour against renderclip
        if( pRenderClip )
        {
            // AW: Simplified
            aPolyPolygon = basegfx::tools::clipPolyPolygonOnPolyPolygon(
                aPolyPolygon, *pRenderClip, true, !bIsFilledPolyPolygon);
        }

        if( !aPolyPolygon.count() )
            return;

        // transform result into view space
        aPolyPolygon.transform(rRenderTransform);

        // clip contour against viewclip
        if( pViewClip )
        {
            // AW: Simplified
            aPolyPolygon = basegfx::tools::clipPolyPolygonOnPolyPolygon(
                aPolyPolygon, *pViewClip, true, !bIsFilledPolyPolygon);
        }

        if(!(aPolyPolygon.count()))
            return;

        // transform result into device space
        aPolyPolygon.transform(rViewTransform);

        io_rClippee = aPolyPolygon;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // setupPolyPolygon
    //////////////////////////////////////////////////////////////////////////////////

    void setupPolyPolygon( ::basegfx::B2DPolyPolygon&         io_rClippee,
                           bool                               bIsFilledPolyPolygon,
                           ARGB&                              o_rRenderColor,
                           const rendering::ViewState&        viewState,
                           const rendering::RenderState&      renderState )
    {
        ::basegfx::B2DHomMatrix                      aViewTransform;
        ::basegfx::B2DHomMatrix                      aRenderTransform;
        ::std::auto_ptr< ::basegfx::B2DPolyPolygon > pViewClip;
        ::std::auto_ptr< ::basegfx::B2DPolyPolygon > pRenderClip;

        setupState( aViewTransform,
                    aRenderTransform,
                    pViewClip,
                    pRenderClip,
                    o_rRenderColor,
                    viewState,
                    renderState );

        clipAndTransformPolygon( io_rClippee,
                                 bIsFilledPolyPolygon,
                                 aViewTransform,
                                 aRenderTransform,
                                 pViewClip.get(),
                                 pRenderClip.get() );
    }

    //////////////////////////////////////////////////////////////////////////////////
    // RawABGRBitmap
    //////////////////////////////////////////////////////////////////////////////////

    // Raw ABGR [AABBGGRR] 32bit continous
    struct RawABGRBitmap
    {
        sal_Int32  mnWidth;
        sal_Int32  mnHeight;
        sal_uInt8* mpBitmapData;
    };

    //////////////////////////////////////////////////////////////////////////////////
    // vclBitmapEx2Raw
    //////////////////////////////////////////////////////////////////////////////////

    void vclBitmapEx2Raw( const ::BitmapEx& rBmpEx, RawABGRBitmap& rBmpData )
    {
        Bitmap aBitmap( rBmpEx.GetBitmap() );

        ScopedBitmapReadAccess pReadAccess( aBitmap.AcquireReadAccess(),
                                            aBitmap );

        const sal_Int32 nWidth( rBmpData.mnWidth );
        const sal_Int32 nHeight( rBmpData.mnHeight );

        ENSURE_OR_THROW( pReadAccess.get() != NULL,
                          "vclBitmapEx2Raw(): "
                          "Unable to acquire read acces to bitmap" );

        if( rBmpEx.IsTransparent())
        {
            if( rBmpEx.IsAlpha() )
            {
                // 8bit alpha mask
                Bitmap aAlpha( rBmpEx.GetAlpha().GetBitmap() );

                ScopedBitmapReadAccess pAlphaReadAccess( aAlpha.AcquireReadAccess(),
                                                         aAlpha );

                // By convention, the access buffer always has
                // one of the following formats:
                //
                //    BMP_FORMAT_1BIT_MSB_PAL
                //    BMP_FORMAT_4BIT_MSN_PAL
                //    BMP_FORMAT_8BIT_PAL
                //    BMP_FORMAT_16BIT_TC_LSB_MASK
                //    BMP_FORMAT_24BIT_TC_BGR
                //    BMP_FORMAT_32BIT_TC_MASK
                //
                // and is always BMP_FORMAT_BOTTOM_UP
                //
                // This is the way
                // WinSalBitmap::AcquireBuffer() sets up the
                // buffer

                ENSURE_OR_THROW( pAlphaReadAccess.get() != NULL,
                                  "vclBitmapEx2Raw(): "
                                  "Unable to acquire read acces to alpha" );

                ENSURE_OR_THROW( pAlphaReadAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL ||
                                  pAlphaReadAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_TC_MASK,
                                  "vclBitmapEx2Raw(): "
                                  "Unsupported alpha scanline format" );

                BitmapColor     aCol;
                sal_uInt8*      pCurrOutput( rBmpData.mpBitmapData );
                int             x, y;

                for( y=0; y<nHeight; ++y )
                {
                    switch( pReadAccess->GetScanlineFormat() )
                    {
                        case BMP_FORMAT_8BIT_PAL:
                        {
                            Scanline pScan  = pReadAccess->GetScanline( y );
                            Scanline pAScan = pAlphaReadAccess->GetScanline( y );

                            for( x=0; x<nWidth; ++x )
                            {
                                aCol = pReadAccess->GetPaletteColor( *pScan++ );

                                *pCurrOutput++ = aCol.GetBlue();
                                *pCurrOutput++ = aCol.GetGreen();
                                *pCurrOutput++ = aCol.GetRed();

                                // out notion of alpha is
                                // different from the rest
                                // of the world's
                                *pCurrOutput++ = 255 - (sal_uInt8)*pAScan++;
                            }
                        }
                        break;

                        case BMP_FORMAT_24BIT_TC_BGR:
                        {
                            Scanline pScan  = pReadAccess->GetScanline( y );
                            Scanline pAScan = pAlphaReadAccess->GetScanline( y );

                            for( x=0; x<nWidth; ++x )
                            {
                                // store as RGBA
                                *pCurrOutput++ = *pScan++;
                                *pCurrOutput++ = *pScan++;
                                *pCurrOutput++ = *pScan++;

                                // out notion of alpha is
                                // different from the rest
                                // of the world's
                                *pCurrOutput++ = 255 - (sal_uInt8)*pAScan++;
                            }
                        }
                        break;

                        // TODO(P2): Might be advantageous
                        // to hand-formulate the following
                        // formats, too.
                        case BMP_FORMAT_1BIT_MSB_PAL:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_4BIT_MSN_PAL:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_16BIT_TC_LSB_MASK:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_32BIT_TC_MASK:
                        {
                            Scanline pAScan = pAlphaReadAccess->GetScanline( y );

                            // using fallback for those
                            // seldom formats
                            for( x=0; x<nWidth; ++x )
                            {
                                // yes. x and y are swapped on Get/SetPixel
                                aCol = pReadAccess->GetColor(y,x);

                                *pCurrOutput++ = aCol.GetBlue();
                                *pCurrOutput++ = aCol.GetGreen();
                                *pCurrOutput++ = aCol.GetRed();

                                // out notion of alpha is
                                // different from the rest
                                // of the world's
                                *pCurrOutput++ = 255 - (sal_uInt8)*pAScan++;
                            }
                        }
                        break;

                        case BMP_FORMAT_1BIT_LSB_PAL:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_4BIT_LSN_PAL:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_8BIT_TC_MASK:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_24BIT_TC_RGB:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_24BIT_TC_MASK:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_16BIT_TC_MSB_MASK:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_32BIT_TC_ABGR:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_32BIT_TC_ARGB:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_32BIT_TC_BGRA:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_32BIT_TC_RGBA:
                            // FALLTHROUGH intended
                        default:
                            ENSURE_OR_THROW( false,
                                              "vclBitmapEx2Raw(): "
                                              "Unexpected scanline format - has "
                                              "WinSalBitmap::AcquireBuffer() changed?" );
                    }
                }
            }
            else
            {
                // 1bit alpha mask
                Bitmap aMask( rBmpEx.GetMask() );

                ScopedBitmapReadAccess pMaskReadAccess( aMask.AcquireReadAccess(),
                                                        aMask );

                // By convention, the access buffer always has
                // one of the following formats:
                //
                //    BMP_FORMAT_1BIT_MSB_PAL
                //    BMP_FORMAT_4BIT_MSN_PAL
                //    BMP_FORMAT_8BIT_PAL
                //    BMP_FORMAT_16BIT_TC_LSB_MASK
                //    BMP_FORMAT_24BIT_TC_BGR
                //    BMP_FORMAT_32BIT_TC_MASK
                //
                // and is always BMP_FORMAT_BOTTOM_UP
                //
                // This is the way
                // WinSalBitmap::AcquireBuffer() sets up the
                // buffer

                ENSURE_OR_THROW( pMaskReadAccess.get() != NULL,
                                  "vclBitmapEx2Raw(): "
                                  "Unable to acquire read acces to mask" );

                ENSURE_OR_THROW( pMaskReadAccess->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL,
                                  "vclBitmapEx2Raw(): "
                                  "Unsupported mask scanline format" );

                BitmapColor     aCol;
                int             nCurrBit;
                const int       nMask( 1L );
                const int       nInitialBit(7);
                sal_uInt32 *pBuffer = reinterpret_cast<sal_uInt32 *>(rBmpData.mpBitmapData);
                int             x, y;

                // mapping table, to get from mask index color to
                // alpha value (which depends on the mask's palette)
                sal_uInt8 aColorMap[2];

                const BitmapColor& rCol0( pMaskReadAccess->GetPaletteColor( 0 ) );
                const BitmapColor& rCol1( pMaskReadAccess->GetPaletteColor( 1 ) );

                // shortcut for true luminance calculation
                // (assumes that palette is grey-level). Note the
                // swapped the indices here, to account for the
                // fact that VCL's notion of alpha is inverted to
                // the rest of the world's.
                aColorMap[0] = rCol1.GetRed();
                aColorMap[1] = rCol0.GetRed();

                for( y=0; y<nHeight; ++y )
                {
                    switch( pReadAccess->GetScanlineFormat() )
                    {
                        case BMP_FORMAT_8BIT_PAL:
                        {
                            Scanline pScan  = pReadAccess->GetScanline( y );
                            Scanline pMScan = pMaskReadAccess->GetScanline( y );

                            for( x=0, nCurrBit=nInitialBit; x<nWidth; ++x )
                            {
                                aCol = pReadAccess->GetPaletteColor( *pScan++ );

                                // RGB -> ABGR
                                unsigned int color = aCol.GetRed();
                                color |= aCol.GetGreen()<<8;
                                color |= aCol.GetBlue()<<16;
                                color |= aColorMap[ (pMScan[ (x & ~7L) >> 3L ] >> nCurrBit ) & nMask ]<<24;
                                *pBuffer++ = color;
                                nCurrBit = ((nCurrBit - 1) % 8L) & 7L;
                            }
                        }
                        break;

                        case BMP_FORMAT_24BIT_TC_BGR:
                        {
                            Scanline pScan  = pReadAccess->GetScanline( y );
                            Scanline pMScan = pMaskReadAccess->GetScanline( y );

                            for( x=0, nCurrBit=nInitialBit; x<nWidth; ++x )
                            {
                                // BGR -> ABGR
                                unsigned int color = (*pScan++)<<16;
                                color |= (*pScan++)<<8;
                                color |= (*pScan++);
                                color |= (aColorMap[ (pMScan[ (x & ~7L) >> 3L ] >> nCurrBit ) & nMask ])<<24;
                                *pBuffer++ = color;
                                nCurrBit = ((nCurrBit - 1) % 8L) & 7L;
                            }
                        }
                        break;

                        // TODO(P2): Might be advantageous
                        // to hand-formulate the following
                        // formats, too.
                        case BMP_FORMAT_1BIT_MSB_PAL:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_4BIT_MSN_PAL:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_16BIT_TC_LSB_MASK:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_32BIT_TC_MASK:
                        {
                            Scanline pMScan = pMaskReadAccess->GetScanline( y );

                            // using fallback for those
                            // seldom formats
                            for( x=0, nCurrBit=nInitialBit; x<nWidth; ++x )
                            {
                                // yes. x and y are swapped on Get/SetPixel
                                aCol = pReadAccess->GetColor(y,x);

                                // -> ABGR
                                unsigned int color = aCol.GetBlue()<<16;
                                color |= aCol.GetGreen()<<8;
                                color |= aCol.GetRed();
                                color |= (aColorMap[ (pMScan[ (x & ~7L) >> 3L ] >> nCurrBit ) & nMask ])<<24;
                                *pBuffer++ = color;
                                nCurrBit = ((nCurrBit - 1) % 8L) & 7L;
                            }
                        }
                        break;

                        case BMP_FORMAT_1BIT_LSB_PAL:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_4BIT_LSN_PAL:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_8BIT_TC_MASK:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_24BIT_TC_RGB:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_24BIT_TC_MASK:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_16BIT_TC_MSB_MASK:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_32BIT_TC_ABGR:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_32BIT_TC_ARGB:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_32BIT_TC_BGRA:
                            // FALLTHROUGH intended
                        case BMP_FORMAT_32BIT_TC_RGBA:
                            // FALLTHROUGH intended
                        default:
                            ENSURE_OR_THROW( false,
                                              "vclBitmapEx2Raw(): "
                                              "Unexpected scanline format - has "
                                              "WinSalBitmap::AcquireBuffer() changed?" );
                    }
                }
            }
        }
        else
        {
            // *no* alpha mask
            sal_uIntPtr nFormat = pReadAccess->GetScanlineFormat();
            sal_uInt8 *pBuffer = reinterpret_cast<sal_uInt8 *>(rBmpData.mpBitmapData);

            switch(nFormat)
            {
                case BMP_FORMAT_24BIT_TC_BGR:

                    {
                        sal_Int32 height = pReadAccess->Height();
                        for(sal_Int32 y=0; y<height; ++y)
                        {
                            sal_uInt8 *pScanline=pReadAccess->GetScanline(y);
                            sal_Int32 width = pReadAccess->Width();
                            for(sal_Int32 x=0; x<width; ++x)
                            {
                                // BGR -> RGB
                                sal_uInt8 b(*pScanline++);
                                sal_uInt8 g(*pScanline++);
                                sal_uInt8 r(*pScanline++);
                                *pBuffer++ = r;
                                *pBuffer++ = g;
                                *pBuffer++ = b;
                            }
                        }
                    }
                    break;

                case BMP_FORMAT_24BIT_TC_RGB:

                    {
                        sal_Int32 height = pReadAccess->Height();
                        for(sal_Int32 y=0; y<height; ++y)
                        {
                            sal_uInt8 *pScanline=pReadAccess->GetScanline(y);
                            sal_Int32 width = pReadAccess->Width();
                            for(sal_Int32 x=0; x<width; ++x)
                            {
                                // RGB -> RGB
                                sal_uInt8 r(*pScanline++);
                                sal_uInt8 g(*pScanline++);
                                sal_uInt8 b(*pScanline++);
                                *pBuffer++ = r;
                                *pBuffer++ = g;
                                *pBuffer++ = b;
                            }
                        }
                    }
                    break;

                case BMP_FORMAT_1BIT_MSB_PAL:
                case BMP_FORMAT_1BIT_LSB_PAL:
                case BMP_FORMAT_4BIT_MSN_PAL:
                case BMP_FORMAT_4BIT_LSN_PAL:
                case BMP_FORMAT_8BIT_PAL:

                    {
                        sal_Int32 height = pReadAccess->Height();
                        for(sal_Int32 y=0; y<height; ++y)
                        {
                            sal_uInt8 *pScanline=pReadAccess->GetScanline(y);
                            sal_Int32 width = pReadAccess->Width();
                            for(sal_Int32 x=0; x<width; ++x)
                            {
                                BitmapColor aCol(pReadAccess->GetPaletteColor(*pScanline++));

                                *pBuffer++ = aCol.GetRed();
                                *pBuffer++ = aCol.GetGreen();
                                *pBuffer++ = aCol.GetBlue();
                            }
                        }
                    }
                    break;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // color_generator_linear
    //////////////////////////////////////////////////////////////////////////////////

    template<typename T> struct color_generator_linear
    {
        typedef typename T::value_type value_type;

        color_generator_linear( const T &c1,
                                const T &c2,
                                unsigned int aSteps ) : maSteps(aSteps),
                                                        maColor1(c1),
                                                        maColor2(c2)
        {
        }

        unsigned size() const { return maSteps; }
        const T operator [] (unsigned v) const
        {
            const double w = double(v)/maSteps;
            return T( static_cast<value_type>(maColor1.r+(maColor2.r-maColor1.r)*w),
                      static_cast<value_type>(maColor1.g+(maColor2.g-maColor1.g)*w),
                      static_cast<value_type>(maColor1.b+(maColor2.b-maColor1.b)*w),
                      static_cast<value_type>(maColor1.a+(maColor2.a-maColor1.a)*w));
        }

        unsigned int maSteps;
        const T      maColor1;
        const T      maColor2;
    };

    //////////////////////////////////////////////////////////////////////////////////
    // color_generator_axial
    //////////////////////////////////////////////////////////////////////////////////

    template<typename T> struct color_generator_axial
    {
        typedef typename T::value_type value_type;

        color_generator_axial( const T &c1,
                               const T &c2,
                               unsigned int aSteps ) : maSteps(aSteps),
                                                       maColor1(c1),
                                                       maColor2(c2)
        {
        }

        unsigned size() const { return maSteps; }
        const T operator [] (unsigned v) const
        {
            const double aHalfSteps = maSteps/2.0;
            const double w = (v >= aHalfSteps) ?
                1.0-((double(v)-aHalfSteps)/aHalfSteps) :
                (double(v)*2.0)/maSteps;
            return T( static_cast<value_type>(maColor1.r+(maColor2.r-maColor1.r)*w),
                      static_cast<value_type>(maColor1.g+(maColor2.g-maColor1.g)*w),
                      static_cast<value_type>(maColor1.b+(maColor2.b-maColor1.b)*w),
                      static_cast<value_type>(maColor1.a+(maColor2.a-maColor1.a)*w));
        }

        unsigned int maSteps;
        const T      maColor1;
        const T      maColor2;
    };

    //////////////////////////////////////////////////////////////////////////////////
    // color_generator_adaptor
    //////////////////////////////////////////////////////////////////////////////////

    template<typename T> struct color_generator_adaptor
    {
        color_generator_adaptor( const T &c1,
                                 const T &c2,
                                 unsigned int aSteps ) : linear_generator(c1,c2,aSteps),
                                                         axial_generator(c1,c2,aSteps),
                                                         mbLinear(true) {}
        void set_linear( bool bLinear ) { mbLinear=bLinear; }
        unsigned size() const { return mbLinear ? linear_generator.size() : axial_generator.size(); }
        const T operator [] (unsigned v) const
        {
            return mbLinear ?
                linear_generator.operator [] (v) :
                axial_generator.operator [] (v);
        }

        color_generator_linear<T> linear_generator;
        color_generator_axial<T>  axial_generator;
        bool                      mbLinear;
    };

    //////////////////////////////////////////////////////////////////////////////////
    // gradient_polymorphic_wrapper_base
    //////////////////////////////////////////////////////////////////////////////////

    struct gradient_polymorphic_wrapper_base
    {
        virtual int calculate(int x, int y, int) const = 0;
    };

    //////////////////////////////////////////////////////////////////////////////////
    // gradient_polymorphic_wrapper
    //////////////////////////////////////////////////////////////////////////////////

    template<class GradientF> struct gradient_polymorphic_wrapper :
        public gradient_polymorphic_wrapper_base
    {
        virtual int calculate(int x, int y, int d) const
        {
            return m_gradient.calculate(x, y, d);
        }
        GradientF m_gradient;
    };

    //////////////////////////////////////////////////////////////////////////////////
    // gradient_rect
    //////////////////////////////////////////////////////////////////////////////////

    class gradient_rect
    {
    public:

        int width;
        int height;

        inline int calculate(int x, int y, int d) const
        {
            int ax = abs(x);
            int ay = abs(y);
            int clamp_x = height>width ? 0 : (width-height);
            int clamp_y = height>width ? (height-width) : 0;
            int value_x = (ax-clamp_x)*d/(width-clamp_x);
            int value_y = (ay-clamp_y)*d/(height-clamp_y);
            if(ax < (clamp_x))
                value_x = 0;
            if(ay < (clamp_y))
                value_y = 0;
            return value_x > value_y ? value_x : value_y;
        }
    };

    sal_uInt32 getBytesPerPixel( IColorBuffer::Format eFormat )
    {
        switch(eFormat)
        {
            default:
                OSL_ENSURE(false, "Unexpected pixel format");
                // FALLTHROUGH intended
            case IColorBuffer::FMT_R8G8B8:
                return 3L;
            case IColorBuffer::FMT_A8R8G8B8:
                return 4L;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Image::drawLinePolyPolygon
//////////////////////////////////////////////////////////////////////////////////

template<class pixel_format>
void Image::drawLinePolyPolygonImpl( const ::basegfx::B2DPolyPolygon&   rPolyPolygon,
                                     double                             fStrokeWidth,
                                     const rendering::ViewState&        viewState,
                                     const rendering::RenderState&      renderState )
{
    ::basegfx::B2DPolyPolygon aPolyPolygon( rPolyPolygon );
    ARGB                      aRenderColor;

    setupPolyPolygon( aPolyPolygon, false, aRenderColor, viewState, renderState );

    if( !aPolyPolygon.count() )
        return;

    // Class template pixel_formats_rgb24 has full knowledge about this
    // particular pixel format in memory. The only template parameter
    // can be order_rgb24 or order_bgr24 that determines the order of color channels.
    //typedef agg::pixfmt_rgba32 pixel_format;
    pixel_format pixf(maRenderingBuffer);

    // There are two basic renderers with almost the same functionality:
    // renderer_base and renderer_mclip. The first one is used most often
    // and it performs low level clipping.
    // This simply adds clipping to the graphics buffer, the clip rect
    // will be initialized to the area of the framebuffer.
    typedef agg::renderer_base<pixel_format> renderer_base;
    agg::renderer_base<pixel_format> renb(pixf);

    // To draw Anti-Aliased primitives one shoud *rasterize* them first.
    // The primary rasterization technique in AGG is scanline based.
    // That is, a polygon is converted into a number of horizontal
    // scanlines and then the scanlines are being rendered one by one.
    // To transfer information from a rasterizer to the scanline renderer
    // there scanline containers are used. A scanline consists of a
    // number of horizontal, non-intersecting spans. All spans must be ordered by X.
    // --> *packed* scanline container
    agg::scanline_p8 sl;

    typedef agg::renderer_outline_aa<renderer_base> renderer_type;
    typedef agg::rasterizer_outline_aa<renderer_type> rasterizer_type;
    agg::line_profile_aa profile;
    profile.width(fStrokeWidth);
    renderer_type ren(renb, profile);
    rasterizer_type ras(ren);

    const agg::rgba8 fillcolor(aRenderColor.Color.r,
                               aRenderColor.Color.g,
                               aRenderColor.Color.b,
                               aRenderColor.Color.a);
    ren.color(fillcolor);

    agg::path_storage path;
    agg::conv_curve<agg::path_storage> curve(path);

    for(sal_uInt32 nPolygon=0; nPolygon<aPolyPolygon.count(); ++nPolygon)
    {
        const basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(nPolygon));
        const sal_uInt32 nPointCount(aPolygon.count());

        if(nPointCount)
        {
            if(aPolygon.areControlPointsUsed())
            {
                // prepare edge-based loop
                basegfx::B2DPoint aCurrentPoint(aPolygon.getB2DPoint(0));
                const sal_uInt32 nEdgeCount(aPolygon.isClosed() ? nPointCount - 1 : nPointCount);

                // first vertex
                path.move_to(aCurrentPoint.getX(), aCurrentPoint.getY());

                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    // access next point
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    const basegfx::B2DPoint aNextPoint(aPolygon.getB2DPoint(nNextIndex));

                    // get control points
                    const basegfx::B2DPoint aControlNext(aPolygon.getNextControlPoint(a));
                    const basegfx::B2DPoint aControlPrev(aPolygon.getPrevControlPoint(nNextIndex));

                    // specify first cp, second cp, next vertex
                    path.curve4(
                        aControlNext.getX(), aControlNext.getY(),
                        aControlPrev.getX(), aControlPrev.getY(),
                        aNextPoint.getX(), aNextPoint.getY());

                    // prepare next step
                    aCurrentPoint = aNextPoint;
                }
            }
            else
            {
                const basegfx::B2DPoint aStartPoint(aPolygon.getB2DPoint(0));
                ras.move_to_d(aStartPoint.getX(), aStartPoint.getY());

                for(sal_uInt32 a(1); a < nPointCount; a++)
                {
                    const basegfx::B2DPoint aVertexPoint(aPolygon.getB2DPoint(a));
                    ras.line_to_d(aVertexPoint.getX(), aVertexPoint.getY());
                }

                ras.render(aPolygon.isClosed());
            }
        }
    }

    ras.add_path(curve);
    ras.render(false);
}

//////////////////////////////////////////////////////////////////////////////////
// Image::drawLinePolyPolygon
//////////////////////////////////////////////////////////////////////////////////

void Image::drawLinePolyPolygon( const ::basegfx::B2DPolyPolygon&   rPoly,
                                 double                             fStrokeWidth,
                                 const rendering::ViewState&        viewState,
                                 const rendering::RenderState&      renderState )
{
    switch(maDesc.eFormat)
    {
        case FMT_R8G8B8:
            drawLinePolyPolygonImpl<agg::pixfmt_rgb24>(rPoly,fStrokeWidth,viewState,renderState);
            break;
        case FMT_A8R8G8B8:
            drawLinePolyPolygonImpl<agg::pixfmt_rgba32>(rPoly,fStrokeWidth,viewState,renderState);
            break;
        default:
            OSL_ENSURE(false, "Unexpected pixel format");
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Image::implDrawBitmap
//////////////////////////////////////////////////////////////////////////////////

/** internal utility function to draw one image into another one.
    the source image will be drawn with respect to the given
    transform and clip settings.
 */
ImageCachedPrimitiveSharedPtr Image::implDrawBitmap(
    const Image&                     rBitmap,
    const rendering::ViewState&      viewState,
    const rendering::RenderState&    renderState )
{
    ::basegfx::B2DPolyPolygon aPoly(
        ::basegfx::tools::createPolygonFromRect(
            ::basegfx::B2DRange(0.0, 0.0,
                                rBitmap.maDesc.nWidth,
                                rBitmap.maDesc.nHeight ) ) );
    ARGB aFillColor;

    setupPolyPolygon( aPoly, true, aFillColor, viewState, renderState );

    if( !aPoly.count() )
        return ImageCachedPrimitiveSharedPtr();

    ::basegfx::B2DHomMatrix aViewTransform;
    ::basegfx::B2DHomMatrix aRenderTransform;
    ::basegfx::B2DHomMatrix aTextureTransform;

    ::basegfx::unotools::homMatrixFromAffineMatrix(aRenderTransform,
                                                   renderState.AffineTransform);
    ::basegfx::unotools::homMatrixFromAffineMatrix(aViewTransform,
                                                   viewState.AffineTransform);
    aTextureTransform *= aRenderTransform;

    // TODO(F2): Fill in texture
    rendering::Texture aTexture;

    return fillTexturedPolyPolygon( rBitmap,
                             aPoly,
                             aTextureTransform,
                             aViewTransform,
                             aTexture );
}

//////////////////////////////////////////////////////////////////////////////////
// cachedPrimitiveFTPP [cachedPrimitive for [F]ill[T]extured[P]oly[P]olygon]
//////////////////////////////////////////////////////////////////////////////////

#if AGG_VERSION >= 2400
template<class pixel_format_dst,class span_gen_type>
#else
template<class pixel_format,class span_gen_type>
#endif
class cachedPrimitiveFTPP : public ImageCachedPrimitive
{
    public:

        cachedPrimitiveFTPP( const ::basegfx::B2DHomMatrix &rTransform,
                            const ::basegfx::B2DHomMatrix &rViewTransform,
                            agg::rendering_buffer &dst,
                            const agg::rendering_buffer& src ) :
            aTransform(rTransform),
            inter(tm),
            filter(filter_kernel),
#if AGG_VERSION >= 2400
                        pixs(const_cast<agg::rendering_buffer&>(src)),
                        source(pixs),
                        sg(source,inter,filter),
                        pixd(dst),
                        rb(pixd),
                        ren(rb,sa,sg)
#else
            sg(sa,src,inter,filter),
            pixf(dst),
            rb(pixf),
            ren(rb,sg)
#endif
        {
            ::basegfx::B2DHomMatrix aFinalTransform(aTransform);
            aFinalTransform *= rViewTransform;
            tm = transAffineFromB2DHomMatrix(aFinalTransform);
            tm.invert();
        }

        virtual void setImage( const ::boost::shared_ptr< class Image >& rTargetImage )
        {
            pImage=rTargetImage;
        }

        virtual sal_Int8 redraw( const ::com::sun::star::rendering::ViewState& aState ) const
        {
            ::basegfx::B2DHomMatrix aViewTransform;
            ::basegfx::unotools::homMatrixFromAffineMatrix(aViewTransform,aState.AffineTransform);
            ::basegfx::B2DHomMatrix aFinalTransform(aTransform);
            aFinalTransform *= aViewTransform;
            tm = transAffineFromB2DHomMatrix(aFinalTransform);
            tm.invert();
            redraw();
            return ::com::sun::star::rendering::RepaintResult::REDRAWN;
        }

        inline void redraw() const { agg::render_scanlines(ras, sl, ren); }

        mutable agg::rasterizer_scanline_aa<> ras;

    private:

        typedef agg::span_interpolator_linear<> interpolator_type;
#if AGG_VERSION >= 2400
                typedef agg::renderer_base<pixel_format_dst> renderer_base;
                typedef agg::span_allocator< typename span_gen_type::color_type > span_alloc_type;
                typedef agg::renderer_scanline_aa<renderer_base, span_alloc_type, span_gen_type> renderer_type;
                typedef typename span_gen_type::source_type source_type;
                typedef typename span_gen_type::source_type::pixfmt_type pixel_format_src;
#else
        typedef agg::renderer_base<pixel_format> renderer_base;
        typedef agg::renderer_scanline_aa<renderer_base, span_gen_type> renderer_type;
#endif

        ::basegfx::B2DHomMatrix aTransform;
        interpolator_type inter;
        agg::image_filter_bilinear filter_kernel;
        agg::image_filter_lut filter;
#if AGG_VERSION >= 2400
                span_alloc_type sa;
                pixel_format_src pixs;
                source_type source;
#else
        agg::span_allocator< typename span_gen_type::color_type > sa;
#endif
        span_gen_type sg;
#if AGG_VERSION >= 2400
        pixel_format_dst pixd;
#else
        pixel_format pixf;
#endif
        renderer_base rb;
        mutable renderer_type ren;
        mutable agg::scanline_p8 sl;
        mutable agg::trans_affine tm;
        ImageSharedPtr pImage;
};

//////////////////////////////////////////////////////////////////////////////////
// Image::fillTexturedPolyPolygon
//////////////////////////////////////////////////////////////////////////////////

template<class pixel_format,class span_gen_type>
ImageCachedPrimitiveSharedPtr Image::fillTexturedPolyPolygonImpl(
                                     const Image&                     rTexture,
                                     const ::basegfx::B2DPolyPolygon& rPolyPolygon,
                                     const ::basegfx::B2DHomMatrix&   rOverallTransform,
                                     const ::basegfx::B2DHomMatrix&   rViewTransform,
                                     const rendering::Texture&         )
{
    // calculate final overall transform.
    ::basegfx::B2DHomMatrix aOverallTransform(rOverallTransform);
    aOverallTransform *= rViewTransform;

    // instead of always using the full-blown solution we
    // first check to see if this is a simple rectangular
    // 1-to-1 copy from source to destination image.
    ::basegfx::B2DTuple aTranslate(aOverallTransform.get(0,2),aOverallTransform.get(1,2));
    ::basegfx::B2DTuple aSize(rTexture.maDesc.nWidth,rTexture.maDesc.nHeight);
    ::basegfx::B2DRange aRange(aTranslate,aTranslate+aSize);
    ::basegfx::B2DPolyPolygon aPolyPolygon(rPolyPolygon);
    aPolyPolygon.transform(aOverallTransform);
    if(::basegfx::tools::isPolyPolygonEqualRectangle(aPolyPolygon,aRange))
    {
        // yes, we can take the shortcut.
        // but we need to clip the destination rectangle
        // against the boundary of the destination image.
        sal_Int32 dwSrcX(0);
        sal_Int32 dwSrcY(0);
        sal_Int32 dwDstX(static_cast<sal_Int32>(aTranslate.getX()));
        sal_Int32 dwDstY(static_cast<sal_Int32>(aTranslate.getY()));
        sal_Int32 dwWidth(rTexture.maDesc.nWidth);
        sal_Int32 dwHeight(rTexture.maDesc.nHeight);

        // prevent fast copy if destination position is not an
        // integer coordinate. otherwise we would most probably
        // introduce visual glitches while combining this with
        // high-accuracy rendering stuff.
        if( ::basegfx::fTools::equalZero(aTranslate.getX()-dwDstX) &&
            ::basegfx::fTools::equalZero(aTranslate.getY()-dwDstY))
        {
            // clip against destination boundary. shrink size if
            // necessary, modify destination position if we need to.
            if(dwDstX < 0) { dwWidth-=dwDstX; dwSrcX=-dwDstX; dwDstX=0; }
            if(dwDstY < 0) { dwHeight-=dwDstY; dwSrcY=-dwDstY; dwDstY=0; }
            const sal_Int32 dwRight(dwDstX+dwWidth);
            const sal_Int32 dwBottom(dwDstY+dwHeight);
            if(dwRight > dwWidth)
                dwWidth -= dwRight-dwWidth;
            if(dwBottom > dwHeight)
                dwHeight -= dwBottom-dwHeight;

            // calculate source buffer
            const Description &srcDesc = rTexture.maDesc;
            const sal_uInt32 dwSrcBytesPerPixel(getBytesPerPixel(srcDesc.eFormat));
            const sal_uInt32 dwSrcPitch(srcDesc.nWidth*dwSrcBytesPerPixel+srcDesc.nStride);
            sal_uInt8 *pSrcBuffer = rTexture.maDesc.pBuffer+(dwSrcPitch*dwSrcX)+(dwSrcBytesPerPixel*dwSrcY);

            // calculate destination buffer
            const Description &dstDesc = maDesc;
            const sal_uInt32 dwDstBytesPerPixel(getBytesPerPixel(dstDesc.eFormat));
            const sal_uInt32 dwDstPitch(dstDesc.nWidth*dwDstBytesPerPixel+dstDesc.nStride);
            sal_uInt8 *pDstBuffer = maDesc.pBuffer+(dwDstPitch*dwDstY)+(dwDstBytesPerPixel*dwDstX);

            // if source and destination format match, we can simply
            // copy whole scanlines.
            if(srcDesc.eFormat == dstDesc.eFormat)
            {
                const sal_Size dwNumBytesPerScanline(dwSrcBytesPerPixel*dwWidth);
                for(sal_Int32 y=0; y<dwHeight; ++y)
                {
                    rtl_copyMemory(pDstBuffer,pSrcBuffer,dwNumBytesPerScanline);
                    pSrcBuffer += dwSrcPitch;
                    pDstBuffer += dwDstPitch;
                }
            }
            else
            {
                // otherwise [formats do not match], we need to copy
                // each pixel one by one and convert from source to destination format.
                if(srcDesc.eFormat == FMT_A8R8G8B8 && dstDesc.eFormat == FMT_R8G8B8)
                {
                    for(sal_Int32 y=0; y<dwHeight; ++y)
                    {
                        sal_uInt8 *pSrc=pSrcBuffer;
                        sal_uInt8 *pDst=pDstBuffer;
                        for(sal_Int32 x=0; x<dwWidth; ++x)
                        {
                            sal_uInt8 r(*pSrc++);
                            sal_uInt8 g(*pSrc++);
                            sal_uInt8 b(*pSrc++);
                            sal_uInt8 Alpha(*pSrc++);
                            sal_uInt8 OneMinusAlpha(0xFF-Alpha);
                            *pDst=(((r*Alpha)+((*pDst)*OneMinusAlpha))/0xFF);
                            ++pDst;
                            *pDst=(((g*Alpha)+((*pDst)*OneMinusAlpha))/0xFF);
                            ++pDst;
                            *pDst=(((b*Alpha)+((*pDst)*OneMinusAlpha))/0xFF);
                            ++pDst;
                        }
                        pSrcBuffer += dwSrcPitch;
                        pDstBuffer += dwDstPitch;
                    }
                }
                else if(srcDesc.eFormat == FMT_R8G8B8 && dstDesc.eFormat == FMT_A8R8G8B8)
                {
                    for(sal_Int32 y=0; y<dwHeight; ++y)
                    {
                        sal_uInt8 *pSrc=pSrcBuffer;
                        sal_uInt8 *pDst=pDstBuffer;
                        for(sal_Int32 x=0; x<dwWidth; ++x)
                        {
                            sal_uInt8 r(*pSrc++);
                            sal_uInt8 g(*pSrc++);
                            sal_uInt8 b(*pSrc++);
                            *pDst++=r;
                            *pDst++=g;
                            *pDst++=b;
                            *pDst++=0xFF;
                        }
                        pSrcBuffer += dwSrcPitch;
                        pDstBuffer += dwDstPitch;
                    }
                }
            }

            return ImageCachedPrimitiveSharedPtr();
        }
    }

    typedef cachedPrimitiveFTPP<pixel_format,span_gen_type> cachedPrimitive_t;
    cachedPrimitive_t *pPrimitive = new cachedPrimitive_t( rOverallTransform,
                                                           rViewTransform,
                                                           maRenderingBuffer,
                                                           rTexture.maRenderingBuffer);

    agg::path_storage path;
    agg::conv_curve<agg::path_storage> curve(path);

    for(sal_uInt32 nPolygon(0); nPolygon < rPolyPolygon.count(); nPolygon++)
    {
        const basegfx::B2DPolygon aPolygon(rPolyPolygon.getB2DPolygon(nPolygon));
        const sal_uInt32 nPointCount(aPolygon.count());

        if(nPointCount)
        {
            if(aPolygon.areControlPointsUsed())
            {
                // prepare edge-based loop
                basegfx::B2DPoint aCurrentPoint(aPolygon.getB2DPoint(0));
                const sal_uInt32 nEdgeCount(aPolygon.isClosed() ? nPointCount - 1 : nPointCount);

                // first vertex
                path.move_to(aCurrentPoint.getX(), aCurrentPoint.getY());

                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    // access next point
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    const basegfx::B2DPoint aNextPoint(aPolygon.getB2DPoint(nNextIndex));

                    // get control points
                    const basegfx::B2DPoint aControlNext(aPolygon.getNextControlPoint(a));
                    const basegfx::B2DPoint aControlPrev(aPolygon.getPrevControlPoint(nNextIndex));

                    // specify first cp, second cp, next vertex
                    path.curve4(
                        aControlNext.getX(), aControlNext.getY(),
                        aControlPrev.getX(), aControlPrev.getY(),
                        aNextPoint.getX(), aNextPoint.getY());

                    // prepare next step
                    aCurrentPoint = aNextPoint;
                }
            }
            else
            {
                const basegfx::B2DPoint aPoint(aPolygon.getB2DPoint(0));
                pPrimitive->ras.move_to_d(aPoint.getX(), aPoint.getY());

                for(sal_uInt32 a(1); a < nPointCount; a++)
                {
                    const basegfx::B2DPoint aVertexPoint(aPolygon.getB2DPoint(a));
                    pPrimitive->ras.line_to_d(aVertexPoint.getX(), aVertexPoint.getY());
                }

                if(aPolygon.isClosed())
                {
                    pPrimitive->ras.close_polygon();
                }
            }
        }
    }

    pPrimitive->ras.add_path(curve);
    pPrimitive->redraw();

    return ImageCachedPrimitiveSharedPtr(pPrimitive);
}

//////////////////////////////////////////////////////////////////////////////////
// Image::fillTexturedPolyPolygon
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::fillTexturedPolyPolygon(
                                     const Image&                     rTexture,
                                     const ::basegfx::B2DPolyPolygon& rPolyPolygon,
                                     const ::basegfx::B2DHomMatrix&   rOverallTransform,
                                     const ::basegfx::B2DHomMatrix&   rViewTransform,
                                     const rendering::Texture&        texture )
{
    typedef agg::wrap_mode_repeat wrap_x_type;
    typedef agg::wrap_mode_repeat wrap_y_type;
    typedef agg::pixfmt_rgb24 pixfmt_rgb24;
    typedef agg::pixfmt_rgba32 pixfmt_rgba32;
#if AGG_VERSION >= 2400
        typedef agg::image_accessor_wrap< pixfmt_rgba32, wrap_x_type, wrap_y_type > img_source_type_rgba;
        typedef agg::image_accessor_wrap< pixfmt_rgb24, wrap_x_type, wrap_y_type > img_source_type_rgb;

        typedef agg::span_image_resample_rgba_affine< img_source_type_rgba > span_gen_type_rgba;
        typedef agg::span_image_resample_rgb_affine< img_source_type_rgb > span_gen_type_rgb;
#else
    typedef agg::span_pattern_resample_rgba_affine< pixfmt_rgba32::color_type,
                                                    pixfmt_rgba32::order_type,
                                                    wrap_x_type,
                                                    wrap_y_type> span_gen_type_rgba;
    typedef agg::span_pattern_resample_rgb_affine< pixfmt_rgb24::color_type,
                                                    pixfmt_rgb24::order_type,
                                                    wrap_x_type,
                                                    wrap_y_type> span_gen_type_rgb;
#endif

    const Format nDest = maDesc.eFormat;
    const Format nSource = rTexture.maDesc.eFormat;

    if(nDest == FMT_R8G8B8 && nSource == FMT_R8G8B8)
    {
        return fillTexturedPolyPolygonImpl< agg::pixfmt_rgb24,
                                            span_gen_type_rgb >(
                                                rTexture,
                                                rPolyPolygon,
                                                rOverallTransform,
                                                rViewTransform,
                                                texture );
    }
    else if(nDest == FMT_R8G8B8 && nSource == FMT_A8R8G8B8)
    {
        return fillTexturedPolyPolygonImpl< agg::pixfmt_rgb24,
                                            span_gen_type_rgba >(
                                                rTexture,
                                                rPolyPolygon,
                                                rOverallTransform,
                                                rViewTransform,
                                                texture );
    }
    else if(nDest == FMT_A8R8G8B8 && nSource == FMT_R8G8B8)
    {
        return fillTexturedPolyPolygonImpl< agg::pixfmt_rgba32,
                                            span_gen_type_rgb >(
                                                rTexture,
                                                rPolyPolygon,
                                                rOverallTransform,
                                                rViewTransform,
                                                texture );
    }
    else if(nDest == FMT_A8R8G8B8 && nSource == FMT_A8R8G8B8)
    {
        return fillTexturedPolyPolygonImpl< agg::pixfmt_rgba32,
                                            span_gen_type_rgba >(
                                                rTexture,
                                                rPolyPolygon,
                                                rOverallTransform,
                                                rViewTransform,
                                                texture );
    }
    else
    {
        OSL_ENSURE(false, "Unexpected pixel format");
    }

    return ImageCachedPrimitiveSharedPtr();
}

//////////////////////////////////////////////////////////////////////////////////
// Image::fillGradient
//////////////////////////////////////////////////////////////////////////////////

template<class pixel_format>
void Image::fillGradientImpl( const ParametricPolyPolygon::Values& rValues,
                              const uno::Sequence< double >&       rUnoColor1,
                              const uno::Sequence< double >&       rUnoColor2,
                              const ::basegfx::B2DPolyPolygon&     rPolyPolygon,
                              const ::basegfx::B2DHomMatrix&       rOverallTransform,
                              const rendering::Texture&             )
{
    const ARGB aColor1(0xFFFFFFFF,
                       rUnoColor1);
    const ARGB aColor2(0xFFFFFFFF,
                       rUnoColor2);

    // first of all we need to provide the framebuffer we want to render to.
    // the properties of the framebuffer are
    // 1) memory & layout [width, height, stride]
    // 2) pixelformat
    // 3) clipping

    // Class template pixel_formats_rgb24 has full knowledge about this
    // particular pixel format in memory. The only template parameter
    // can be order_rgb24 or order_bgr24 that determines the order of color channels.
    pixel_format pixf(maRenderingBuffer);

    // There are two basic renderers with almost the same functionality:
    // renderer_base and renderer_mclip. The first one is used most often
    // and it performs low level clipping.
    // This simply adds clipping to the graphics buffer, the clip rect
    // will be initialized to the area of the framebuffer.
    typedef agg::renderer_base<pixel_format> renderer_base;
    renderer_base rb(pixf);

    // bounding rectangle of untransformed polypolygon
    const ::basegfx::B2DRange& rBounds(::basegfx::tools::getRange(rPolyPolygon));

    // the color generator produces a specific color from
    // some given interpolation value.
    // number of steps for color interpolation
    typedef typename pixel_format::color_type color_type;
    color_type color1(agg::rgba8(aColor1.Color.r,
                                 aColor1.Color.g,
                                 aColor1.Color.b,
                                 255));
    color_type color2(agg::rgba8(aColor2.Color.r,
                                 aColor2.Color.g,
                                 aColor2.Color.b,
                                 255));
    typedef color_generator_adaptor<color_type> color_generator_type;
    unsigned int dwNumSteps = static_cast<unsigned int>(rBounds.getWidth());
    color_generator_type colors(color1,color2,dwNumSteps);
    colors.set_linear(true);

    // color = f(x,y)
    gradient_polymorphic_wrapper<agg::gradient_x> gf_x;
    gradient_polymorphic_wrapper<agg::gradient_radial> gf_radial;
    gradient_polymorphic_wrapper<gradient_rect> gf_rectangular;
    gf_rectangular.m_gradient.width = static_cast<int>(rBounds.getWidth())<<4;
    gf_rectangular.m_gradient.height = static_cast<int>(rBounds.getHeight())<<4;
    const gradient_polymorphic_wrapper_base *gf[] = { &gf_x,            // GRADIENT_LINEAR
                                                      &gf_x,            // GRADIENT_AXIAL
                                                      &gf_radial,       // GRADIENT_ELLIPTICAL
                                                      &gf_rectangular   // GRADIENT_RECTANGULAR
                                                    };

    // how do texture coordinates change when the pixel coordinate change?
    typedef agg::span_interpolator_linear<> interpolator_type;
    agg::trans_affine tm;
    tm *= agg::trans_affine_scaling(1.0f/rBounds.getWidth(),
                                    1.0f/rBounds.getHeight());
    if(rValues.meType == ParametricPolyPolygon::GRADIENT_ELLIPTICAL ||
       rValues.meType == ParametricPolyPolygon::GRADIENT_RECTANGULAR)
    {
        //tm *= trans_affine_scaling(mnAspectRatio,+1.0f);
        //const double fAspect = aBounds.getWidth()/aBounds.getHeight();
        //tm *= trans_affine_scaling(+0.5f,+0.5f*(1.0f/fAspect));
        //tm *= trans_affine_translation(+0.5f,+0.5f);
        tm *= agg::trans_affine_scaling(+0.5f,+0.5f);
        tm *= agg::trans_affine_translation(+0.5f,+0.5f);
    }
    tm *= transAffineFromB2DHomMatrix(rOverallTransform);
    tm.invert();
    interpolator_type inter(tm);

    // spanline allocators reserve memory for the color values
    // filled up by the spanline generators.
    typedef agg::span_allocator<color_type> gradient_span_alloc;
    gradient_span_alloc span_alloc;

    // scanline generators create the actual color values for
    // some specific coordinate range of a scanline.
    typedef agg::span_gradient<color_type,
                            interpolator_type,
                            gradient_polymorphic_wrapper_base,
                            color_generator_type > gradient_span_gen;
#if AGG_VERSION >= 2400
    gradient_span_gen span_gen(inter,
                               *gf[rValues.meType],
                               colors,
                               0,
                               dwNumSteps);
#else
    gradient_span_gen span_gen(span_alloc,
                               inter,
                               *gf[rValues.meType],
                               colors,
                               0,
                               dwNumSteps);
#endif

    // To draw Anti-Aliased primitives one shoud *rasterize* them first.
    // The primary rasterization technique in AGG is scanline based.
    // That is, a polygon is converted into a number of horizontal
    // scanlines and then the scanlines are being rendered one by one.
    // To transfer information from a rasterizer to the scanline renderer
    // there scanline containers are used. A scanline consists of a
    // number of horizontal, non-intersecting spans. All spans must be ordered by X.
    // --> packed scanline container
    agg::scanline_p8 sl;

    // antialiased scanline renderer with pattern filling capability
    // [in contrast to solid renderers, that is]
    // the instance of this particular renderer combines the
    // renderbuffer [i.e. destination] and the spanline generator [i.e. source]
#if AGG_VERSION >= 2400
        typedef agg::renderer_scanline_aa<renderer_base, gradient_span_alloc, gradient_span_gen> renderer_gradient;
        renderer_gradient r1(rb, span_alloc, span_gen);
#else
    typedef agg::renderer_scanline_aa<renderer_base, gradient_span_gen> renderer_gradient;
    renderer_gradient r1(rb, span_gen);
#endif

    // instantiate the rasterizer and feed the incoming polypolygon.
    agg::rasterizer_scanline_aa<> ras;
    agg::path_storage path;
    agg::conv_curve<agg::path_storage> curve(path);

    for(sal_uInt32 nPolygon(0); nPolygon < rPolyPolygon.count(); nPolygon++)
    {
        const basegfx::B2DPolygon aPolygon(rPolyPolygon.getB2DPolygon(nPolygon));
        const sal_uInt32 nPointCount(aPolygon.count());

        if(nPointCount)
        {
            if(aPolygon.areControlPointsUsed())
            {
                // prepare edge-based loop
                basegfx::B2DPoint aCurrentPoint(aPolygon.getB2DPoint(0));
                const sal_uInt32 nEdgeCount(aPolygon.isClosed() ? nPointCount - 1 : nPointCount);

                // first vertex
                path.move_to(aCurrentPoint.getX(), aCurrentPoint.getY());

                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    // access next point
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    const basegfx::B2DPoint aNextPoint(aPolygon.getB2DPoint(nNextIndex));

                    // get control points
                    const basegfx::B2DPoint aControlNext(aPolygon.getNextControlPoint(a));
                    const basegfx::B2DPoint aControlPrev(aPolygon.getPrevControlPoint(nNextIndex));

                    // specify first cp, second cp, next vertex
                    path.curve4(
                        aControlNext.getX(), aControlNext.getY(),
                        aControlPrev.getX(), aControlPrev.getY(),
                        aNextPoint.getX(), aNextPoint.getY());

                    // prepare next step
                    aCurrentPoint = aNextPoint;
                }
            }
            else
            {
                const basegfx::B2DPoint aPoint(aPolygon.getB2DPoint(0));
                ras.move_to_d(aPoint.getX(), aPoint.getY());

                for(sal_uInt32 a(1); a < nPointCount; a++)
                {
                    const basegfx::B2DPoint aVertexPoint(aPolygon.getB2DPoint(a));
                    ras.line_to_d(aVertexPoint.getX(), aVertexPoint.getY());
                }

                if(aPolygon.isClosed())
                {
                    ras.close_polygon();
                }
            }
        }
    }

    // everything is up and running, go...
    ras.add_path(curve);
    render_scanlines(ras,sl,r1);
}

//////////////////////////////////////////////////////////////////////////////////
// Image::fillGradient
//////////////////////////////////////////////////////////////////////////////////

void Image::fillGradient( const ParametricPolyPolygon::Values& rValues,
                          const uno::Sequence< double >&       rUnoColor1,
                          const uno::Sequence< double >&       rUnoColor2,
                          const ::basegfx::B2DPolyPolygon&     rPolyPolygon,
                          const ::basegfx::B2DHomMatrix&       rOverallTransform,
                          const rendering::Texture&            texture )
{
    switch(maDesc.eFormat)
    {
        case FMT_R8G8B8:
            fillGradientImpl<agg::pixfmt_rgb24>(rValues,rUnoColor1,rUnoColor2,rPolyPolygon,rOverallTransform,texture);
            break;
        case FMT_A8R8G8B8:
            fillGradientImpl<agg::pixfmt_rgba32>(rValues,rUnoColor1,rUnoColor2,rPolyPolygon,rOverallTransform,texture);
            break;
        default:
            OSL_ENSURE(false, "Unexpected pixel format");
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Image::fromVCLBitmap
//////////////////////////////////////////////////////////////////////////////////

bool Image::fromVCLBitmap( ::BitmapEx& rBmpEx )
{
    const ::Size aBmpSize( rBmpEx.GetSizePixel() );
    Image::Description desc;
    desc.eFormat = rBmpEx.IsTransparent() ? FMT_A8R8G8B8 : FMT_R8G8B8;
    desc.nWidth = aBmpSize.Width();
    desc.nHeight = aBmpSize.Height();
    desc.nStride = 0;
    const sal_uInt32 nPitch(desc.nWidth*getBytesPerPixel(desc.eFormat)+desc.nStride);
    desc.pBuffer = new sal_uInt8 [nPitch*desc.nHeight];
    maDesc = desc;
    mbBufferHasUserOwnership = false;
    maRenderingBuffer.attach(static_cast<agg::int8u *>(desc.pBuffer),
                             desc.nWidth,
                             desc.nHeight,
                             nPitch);
    RawABGRBitmap aBmpData;
    aBmpData.mnWidth = aBmpSize.Width();
    aBmpData.mnHeight = aBmpSize.Height();
    aBmpData.mpBitmapData = static_cast<sal_uInt8 *>(desc.pBuffer);
    vclBitmapEx2Raw(rBmpEx,aBmpData);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// Image::Image
//////////////////////////////////////////////////////////////////////////////////

Image::Image( const Description& rDesc ) :
    maDesc( rDesc ),
    maRenderingBuffer(),
    mbBufferHasUserOwnership( rDesc.pBuffer != NULL )
{
#if defined(PROFILER)
    for(int i=0; i<TIMER_MAX; ++i)
        maElapsedTime[i]=0.0;
#endif

    // allocate own buffer memory, if not provided
    sal_uInt8* pBuffer = maDesc.pBuffer;
    const sal_uInt32 nWidth(maDesc.nWidth);
    const sal_uInt32 nHeight(maDesc.nHeight);
    const sal_uInt32 nStride(maDesc.nStride);
    const sal_uInt32 nPitch(nWidth*getBytesPerPixel(maDesc.eFormat)
                            + nStride);

    if( !pBuffer )
        pBuffer = new sal_uInt8 [nPitch*nHeight];

    maDesc.pBuffer = pBuffer;

    // attach graphics buffer
    maRenderingBuffer.attach(
        static_cast<agg::int8u *>(pBuffer),
        nWidth,
        nHeight,
        nPitch );
}

//////////////////////////////////////////////////////////////////////////////////
// Image::Image
//////////////////////////////////////////////////////////////////////////////////

Image::Image( const uno::Reference< rendering::XBitmap >& xBitmap ) :
    maDesc(),
    maRenderingBuffer(),
    mbBufferHasUserOwnership( false )
{
#if defined(PROFILER)
    for(int i=0; i<TIMER_MAX; ++i)
        maElapsedTime[i]=0.0;
#endif

    // TODO(F1): Add support for floating point bitmap formats
    uno::Reference<rendering::XIntegerReadOnlyBitmap> xIntBmp(xBitmap,
                                                              uno::UNO_QUERY_THROW);
    ::BitmapEx aBmpEx = ::vcl::unotools::bitmapExFromXBitmap(xIntBmp);
    if( !!aBmpEx )
        fromVCLBitmap(aBmpEx);

    // TODO(F2): Fallback to XIntegerBitmap interface for import
    OSL_ENSURE(false,
               "Image::Image(): Cannot retrieve bitmap data!" );
}

//////////////////////////////////////////////////////////////////////////////////
// Image::~Image
//////////////////////////////////////////////////////////////////////////////////

Image::~Image()
{
#if defined(PROFILER)

    double aAccumulatedTime(0.0);
    for(int i=0; i<TIMER_MAX; ++i)
        aAccumulatedTime += maElapsedTime[i];

    OSL_TRACE("Image %d - %d %d %d %d %d\n",(int)(aAccumulatedTime*1000.0),
        (int)(maElapsedTime[TIMER_FILLTEXTUREDPOLYPOLYGON]*1000.0),
        (int)(maElapsedTime[TIMER_FILLB2DPOLYPOLYGON]*1000.0),
        (int)(maElapsedTime[TIMER_DRAWPOLYPOLYGON]*1000.0),
        (int)(maElapsedTime[TIMER_FILLPOLYPOLYGON]*1000.0),
        (int)(maElapsedTime[TIMER_DRAWBITMAP]*1000.0));

#endif

    if( !mbBufferHasUserOwnership )
        delete [] maDesc.pBuffer;
}

//////////////////////////////////////////////////////////////////////////////////
// Image::clear
//////////////////////////////////////////////////////////////////////////////////

template<class pixel_format>
void Image::clearImpl( sal_uInt8 a,
                       sal_uInt8 r,
                       sal_uInt8 g,
                       sal_uInt8 b )
{
    pixel_format pixf(maRenderingBuffer);
    agg::renderer_base<pixel_format> renb(pixf);

    renb.clear(agg::rgba8(r,g,b,a));
}

//////////////////////////////////////////////////////////////////////////////////
// Image::clear
//////////////////////////////////////////////////////////////////////////////////

void Image::clear( sal_uInt8 a,
                   sal_uInt8 r,
                   sal_uInt8 g,
                   sal_uInt8 b )
{
    switch(maDesc.eFormat)
    {
        case FMT_R8G8B8:
            return clearImpl<agg::pixfmt_rgb24>(a,r,g,b);
        case FMT_A8R8G8B8:
            return clearImpl<agg::pixfmt_rgba32>(a,r,g,b);
        default:
            OSL_ENSURE(false, "Unexpected pixel format");
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Image::fillB2DPolyPolygon
//////////////////////////////////////////////////////////////////////////////////

void Image::fillB2DPolyPolygon(
            const ::basegfx::B2DPolyPolygon&    rPolyPolygon,
            const rendering::ViewState&         viewState,
            const rendering::RenderState&       renderState )
{
#if defined(PROFILER)
    ScopeTimer aTimer(TIMER_FILLB2DPOLYPOLYGON,this);
#endif

    switch(maDesc.eFormat)
    {
        case FMT_R8G8B8:
            fillPolyPolygonImpl<agg::pixfmt_rgb24>(rPolyPolygon,viewState,renderState);
            break;
        case FMT_A8R8G8B8:
            fillPolyPolygonImpl<agg::pixfmt_rgba32>(rPolyPolygon,viewState,renderState);
            break;
        default:
            OSL_ENSURE(false, "Unexpected pixel format");
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Image::lock
//////////////////////////////////////////////////////////////////////////////////

sal_uInt8* Image::lock() const
{
    return maDesc.pBuffer;
}

//////////////////////////////////////////////////////////////////////////////////
// Image::unlock
//////////////////////////////////////////////////////////////////////////////////

void Image::unlock() const
{
}

//////////////////////////////////////////////////////////////////////////////////
// Image::getWidth
//////////////////////////////////////////////////////////////////////////////////

sal_uInt32 Image::getWidth() const
{
    return maDesc.nWidth;
}

//////////////////////////////////////////////////////////////////////////////////
// Image::getHeight
//////////////////////////////////////////////////////////////////////////////////

sal_uInt32 Image::getHeight() const
{
    return maDesc.nHeight;
}

//////////////////////////////////////////////////////////////////////////////////
// Image::getStride
//////////////////////////////////////////////////////////////////////////////////

sal_uInt32 Image::getStride() const
{
    return maDesc.nWidth*getBytesPerPixel(maDesc.eFormat)+maDesc.nStride;
}

//////////////////////////////////////////////////////////////////////////////////
// Image::getFormat
//////////////////////////////////////////////////////////////////////////////////

IColorBuffer::Format Image::getFormat() const
{
    return maDesc.eFormat;
}

//////////////////////////////////////////////////////////////////////////////////
// Image::drawPoint
//////////////////////////////////////////////////////////////////////////////////

void Image::drawPoint( const geometry::RealPoint2D&     /*aPoint*/,
                       const rendering::ViewState&      /*viewState*/,
                       const rendering::RenderState&    /*renderState*/ )
{
    OSL_ENSURE(false,
               "Image::drawPoint(): NYI" );
}

//////////////////////////////////////////////////////////////////////////////////
// Image::drawLine
//////////////////////////////////////////////////////////////////////////////////

void Image::drawLine( const geometry::RealPoint2D&  aStartPoint,
                      const geometry::RealPoint2D&  aEndPoint,
                      const rendering::ViewState&   viewState,
                      const rendering::RenderState& renderState )
{
    ::basegfx::B2DPolygon aLinePoly;
    aLinePoly.append(
        ::basegfx::unotools::b2DPointFromRealPoint2D( aStartPoint ) );
    aLinePoly.append(
        ::basegfx::unotools::b2DPointFromRealPoint2D( aEndPoint ) );

    drawLinePolyPolygon( ::basegfx::B2DPolyPolygon( aLinePoly ),
                         1.0,
                         viewState,
                         renderState );
}

//////////////////////////////////////////////////////////////////////////////////
// Image::drawBezier
//////////////////////////////////////////////////////////////////////////////////

void Image::drawBezier( const geometry::RealBezierSegment2D&    aBezierSegment,
                        const geometry::RealPoint2D&            aEndPoint,
                        const rendering::ViewState&             viewState,
                        const rendering::RenderState&           renderState )
{
    basegfx::B2DPolygon aBezierPoly;

    aBezierPoly.append(basegfx::B2DPoint(aBezierSegment.Px, aBezierSegment.Py));
    aBezierPoly.appendBezierSegment(
        basegfx::B2DPoint(aBezierSegment.C1x, aBezierSegment.C1y),
        basegfx::B2DPoint(aBezierSegment.C2x, aBezierSegment.C2y),
        basegfx::unotools::b2DPointFromRealPoint2D(aEndPoint));

    drawLinePolyPolygon( ::basegfx::B2DPolyPolygon( aBezierPoly ),
                         1.0,
                         viewState,
                         renderState );
}

//////////////////////////////////////////////////////////////////////////////////
// Image::drawPolyPolygon
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::drawPolyPolygon(
    const uno::Reference< rendering::XPolyPolygon2D >&  xPolyPolygon,
    const rendering::ViewState&                         viewState,
    const rendering::RenderState&                       renderState )
{
#if defined(PROFILER)
    ScopeTimer aTimer(TIMER_DRAWPOLYPOLYGON,this);
#endif

    if( !xPolyPolygon.is() )
        return ImageCachedPrimitiveSharedPtr();

    drawLinePolyPolygon( ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D( xPolyPolygon ),
                         1.0,
                         viewState,
                         renderState );

    // TODO(F2): Implement sensible ImageCachedPrimitive
    return ImageCachedPrimitiveSharedPtr();
}

//////////////////////////////////////////////////////////////////////////////////
// Image::strokePolyPolygon
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::strokePolyPolygon(
    const uno::Reference< rendering::XPolyPolygon2D >&  xPolyPolygon,
    const rendering::ViewState&                         viewState,
    const rendering::RenderState&                       renderState,
    const rendering::StrokeAttributes&                  strokeAttributes )
{
    if( !xPolyPolygon.is() )
        return ImageCachedPrimitiveSharedPtr();

    drawLinePolyPolygon( ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D( xPolyPolygon ),
                         strokeAttributes.StrokeWidth,
                         viewState,
                         renderState );

    // TODO(F2): Implement sensible ImageCachedPrimitive
    return ImageCachedPrimitiveSharedPtr();
}

//////////////////////////////////////////////////////////////////////////////////
// Image::strokeTexturedPolyPolygon
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::strokeTexturedPolyPolygon(
    const uno::Reference< rendering::XPolyPolygon2D >&  /*xPolyPolygon*/,
    const rendering::ViewState&                         /*viewState*/,
    const rendering::RenderState&                       /*renderState*/,
    const uno::Sequence< rendering::Texture >&          /*textures*/,
    const ::std::vector< ::boost::shared_ptr<Image> >&  /*textureAnnotations*/,
    const rendering::StrokeAttributes&                  /*strokeAttributes*/ )
{
    OSL_ENSURE(false,
               "Image::strokeTexturedPolyPolygon(): NYI" );

    // TODO(F2): Implement sensible ImageCachedPrimitive
    return ImageCachedPrimitiveSharedPtr();
}

//////////////////////////////////////////////////////////////////////////////////
// Image::strokeTextureMappedPolyPolygon
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::strokeTextureMappedPolyPolygon(
    const uno::Reference< rendering::XPolyPolygon2D >&  /*xPolyPolygon*/,
    const rendering::ViewState&                         /*viewState*/,
    const rendering::RenderState&                       /*renderState*/,
    const uno::Sequence< rendering::Texture >&          /*textures*/,
    const ::std::vector< ::boost::shared_ptr<Image> >&  /*textureAnnotations*/,
    const uno::Reference< geometry::XMapping2D >&       /*xMapping*/,
    const rendering::StrokeAttributes&                  /*strokeAttributes*/ )
{
    OSL_ENSURE(false,
               "Image::strokeTextureMappedPolyPolygon(): NYI" );

    // TODO(F2): Implement sensible ImageCachedPrimitive
    return ImageCachedPrimitiveSharedPtr();
}

//////////////////////////////////////////////////////////////////////////////////
// Image::fillPolyPolygon
//////////////////////////////////////////////////////////////////////////////////

template<class pixel_format>
ImageCachedPrimitiveSharedPtr Image::fillPolyPolygonImpl(
            const ::basegfx::B2DPolyPolygon&                    rPolyPolygon,
            const rendering::ViewState&                         viewState,
            const rendering::RenderState&                       renderState )
{
#if defined(PROFILER)
    ScopeTimer aTimer(TIMER_FILLPOLYPOLYGON,this);
#endif

    ARGB aFillColor;

    ::basegfx::B2DPolyPolygon aPolyPolygon(rPolyPolygon);
    setupPolyPolygon( aPolyPolygon, true, aFillColor, viewState, renderState );

    if( !aPolyPolygon.count() )
        return ImageCachedPrimitiveSharedPtr();

    pixel_format pixf(maRenderingBuffer);
    agg::renderer_base<pixel_format> renb(pixf);

    // Scanline renderer for solid filling.
    agg::renderer_scanline_aa_solid<agg::renderer_base<pixel_format> > ren(renb);

    // Rasterizer & scanline
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_p8 sl;

    agg::path_storage path;
    agg::conv_curve<agg::path_storage> curve(path);

    for(sal_uInt32 nPolygon(0); nPolygon < aPolyPolygon.count(); nPolygon++)
    {
        const basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(nPolygon));
        const sal_uInt32 nPointCount(aPolygon.count());

        if(nPointCount)
        {
            if(aPolygon.areControlPointsUsed())
            {
                // prepare edge-based loop
                basegfx::B2DPoint aCurrentPoint(aPolygon.getB2DPoint(0));
                const sal_uInt32 nEdgeCount(aPolygon.isClosed() ? nPointCount - 1 : nPointCount);

                // first vertex
                path.move_to(aCurrentPoint.getX(), aCurrentPoint.getY());

                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    // access next point
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    const basegfx::B2DPoint aNextPoint(aPolygon.getB2DPoint(nNextIndex));

                    // get control points
                    const basegfx::B2DPoint aControlNext(aPolygon.getNextControlPoint(a));
                    const basegfx::B2DPoint aControlPrev(aPolygon.getPrevControlPoint(nNextIndex));

                    // specify first cp, second cp, next vertex
                    path.curve4(
                        aControlNext.getX(), aControlNext.getY(),
                        aControlPrev.getX(), aControlPrev.getY(),
                        aNextPoint.getX(), aNextPoint.getY());

                    // prepare next step
                    aCurrentPoint = aNextPoint;
                }
            }
            else
            {
                const basegfx::B2DPoint aPoint(aPolygon.getB2DPoint(0));
                ras.move_to_d(aPoint.getX(), aPoint.getY());

                for(sal_uInt32 a(1); a < nPointCount; a++)
                {
                    const basegfx::B2DPoint aVertexPoint(aPolygon.getB2DPoint(a));
                    ras.line_to_d(aVertexPoint.getX(), aVertexPoint.getY());
                }

                if(aPolygon.isClosed())
                {
                    ras.close_polygon();
                }
            }
        }
    }

    ras.add_path(curve);
    agg::rgba8 fillcolor(aFillColor.Color.r,aFillColor.Color.g,aFillColor.Color.b,aFillColor.Color.a);
    ren.color(fillcolor);
    agg::render_scanlines(ras, sl, ren);

    // TODO(F2): Implement sensible ImageCachedPrimitive
    return ImageCachedPrimitiveSharedPtr();
}

//////////////////////////////////////////////////////////////////////////////////
// Image::fillPolyPolygon
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::fillPolyPolygon(
    const uno::Reference< rendering::XPolyPolygon2D >&  xPolyPolygon,
    const rendering::ViewState&                         viewState,
    const rendering::RenderState&                       renderState )
{
    if( !xPolyPolygon.is() )
        return ImageCachedPrimitiveSharedPtr();

    ::basegfx::B2DPolyPolygon aPoly(
        ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D( xPolyPolygon ) );

    switch(maDesc.eFormat)
    {
        case FMT_R8G8B8:
            return fillPolyPolygonImpl<agg::pixfmt_rgb24>(aPoly,viewState,renderState);
        case FMT_A8R8G8B8:
            return fillPolyPolygonImpl<agg::pixfmt_rgba32>(aPoly,viewState,renderState);
        default:
            OSL_ENSURE(false, "Unexpected pixel format");
            break;
    }

    return ImageCachedPrimitiveSharedPtr();
}

//////////////////////////////////////////////////////////////////////////////////
// Image::fillTexturedPolyPolygon
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::fillTexturedPolyPolygon(
    const uno::Reference< rendering::XPolyPolygon2D >&  xPolyPolygon,
    const rendering::ViewState&                         viewState,
    const rendering::RenderState&                       renderState,
    const uno::Sequence< rendering::Texture >&          textures,
    const ::std::vector< ::boost::shared_ptr<Image> >&  textureAnnotations )
{
#if defined(PROFILER)
    ScopeTimer aTimer(TIMER_FILLTEXTUREDPOLYPOLYGON,this);
#endif

    if( !xPolyPolygon.is() )
        return ImageCachedPrimitiveSharedPtr();

    ::basegfx::B2DPolyPolygon aPoly(
        ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D( xPolyPolygon ) );
    ARGB aFillColor;

    setupPolyPolygon( aPoly, true, aFillColor, viewState, renderState );

    if( !aPoly.count() )
        return ImageCachedPrimitiveSharedPtr();

    ::basegfx::B2DHomMatrix aViewTransform;
    ::basegfx::B2DHomMatrix aRenderTransform;
    ::basegfx::B2DHomMatrix aTextureTransform;

    ::basegfx::unotools::homMatrixFromAffineMatrix(aTextureTransform,
                                                   textures[0].AffineTransform);
    ::basegfx::unotools::homMatrixFromAffineMatrix(aRenderTransform,
                                                   renderState.AffineTransform);
    ::basegfx::unotools::homMatrixFromAffineMatrix(aViewTransform,
                                                   viewState.AffineTransform);
    aTextureTransform *= aRenderTransform;

    // TODO(F1): Multi-texturing
    if( textures[0].Gradient.is() )
    {
        aTextureTransform *= aViewTransform;

        // try to cast XParametricPolyPolygon2D reference to
        // our implementation class.
        ::canvas::ParametricPolyPolygon* pGradient =
            dynamic_cast< ::canvas::ParametricPolyPolygon* >( textures[0].Gradient.get() );

        if( pGradient )
        {
            const ParametricPolyPolygon::Values& rValues(
                pGradient->getValues() );

            // TODO: use all the colors and place them on given positions/stops
            // TODO(E1): Return value
            // TODO(F1): FillRule
            fillGradient( rValues,
                          rValues.maColors [0],
                          rValues.maColors [rValues.maColors.getLength () - 1],
                          aPoly,
                          aTextureTransform,
                          textures[0] );
        }
    }
    else if( textures[0].Bitmap.is() )
    {
        ImageSharedPtr pTexture;

        if( textureAnnotations[0].get() != NULL )
            pTexture = textureAnnotations[0];
        else
            pTexture.reset( new Image( textures[0].Bitmap ) );

        const sal_uInt32 nWidth(pTexture->maDesc.nWidth);
        const sal_uInt32 nHeight(pTexture->maDesc.nHeight);

        // scale texture into one-by-one unit rect.
        aTextureTransform.scale(1.0f/nWidth,
                                1.0f/nHeight);

        // TODO(E1): Return value
        // TODO(F1): FillRule
        return fillTexturedPolyPolygon( *pTexture,
                                 aPoly,
                                 aTextureTransform,
                                 aViewTransform,
                                 textures[0] );
    }

    // TODO(F2): Implement sensible ImageCachedPrimitive
    return ImageCachedPrimitiveSharedPtr();
}

//////////////////////////////////////////////////////////////////////////////////
// Image::fillTextureMappedPolyPolygon
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::fillTextureMappedPolyPolygon(
    const uno::Reference< rendering::XPolyPolygon2D >&  /*xPolyPolygon*/,
    const rendering::ViewState&                         /*viewState*/,
    const rendering::RenderState&                       /*renderState*/,
    const uno::Sequence< rendering::Texture >&          /*textures*/,
    const ::std::vector< ::boost::shared_ptr<Image> >&  /*textureAnnotations*/,
    const uno::Reference< geometry::XMapping2D >&       /*xMapping*/ )
{
    OSL_ENSURE(false,
               "Image::fillTextureMappedPolyPolygon(): NYI" );

    // TODO(F2): Implement sensible ImageCachedPrimitive
    return ImageCachedPrimitiveSharedPtr();
}

//////////////////////////////////////////////////////////////////////////////////
// Image::drawBitmap
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::drawBitmap(
    const uno::Reference< rendering::XBitmap >&   xBitmap,
    const rendering::ViewState&                   viewState,
    const rendering::RenderState&                 renderState )
{
#if defined(PROFILER)
    ScopeTimer aTimer(TIMER_DRAWBITMAP,this);
#endif

    // TODO(P3): Implement bitmap caching
    if( !xBitmap.is() )
        return ImageCachedPrimitiveSharedPtr();
/*
    XBitmapAccessor accessor( xBitmap );
    if(accessor.isValid())
    {
        Image aImage( accessor.getDesc() );

        implDrawBitmap( aImage,
                        viewState,
                        renderState );

        // TODO(F2): Implement sensible ImageCachedPrimitive
        return ImageCachedPrimitiveSharedPtr();
    }
*/
    Image aImage( xBitmap );

    return implDrawBitmap( aImage,viewState,renderState );
}

//////////////////////////////////////////////////////////////////////////////////
// Image::drawBitmap
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::drawBitmap(
    const ImageSharedPtr&           rImage,
    const rendering::ViewState&     viewState,
    const rendering::RenderState&   renderState )
{
#if defined(PROFILER)
    ScopeTimer aTimer(TIMER_DRAWBITMAP,this);
#endif

    // TODO(P3): Implement bitmap caching
    if( !rImage )
        return ImageCachedPrimitiveSharedPtr();

    return implDrawBitmap( *rImage,
                    viewState,
                    renderState );
}

//////////////////////////////////////////////////////////////////////////////////
// Image::drawBitmapModulated
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::drawBitmapModulated(
    const uno::Reference< rendering::XBitmap >&   xBitmap,
    const rendering::ViewState&                   viewState,
    const rendering::RenderState&                 renderState )
{
    // TODO(P3): Implement bitmap caching
    if( !xBitmap.is() )
        return ImageCachedPrimitiveSharedPtr();

    Image aImage( xBitmap );

    // TODO(F2): Distinguish modulated and unmodulated bitmap output
    return implDrawBitmap( aImage,viewState,renderState );
}

//////////////////////////////////////////////////////////////////////////////////
// Image::drawBitmapModulated
//////////////////////////////////////////////////////////////////////////////////

ImageCachedPrimitiveSharedPtr Image::drawBitmapModulated(
    const ImageSharedPtr&           rImage,
    const rendering::ViewState&     viewState,
    const rendering::RenderState&   renderState )
{
    // TODO(P3): Implement bitmap caching
    if( !rImage )
        return ImageCachedPrimitiveSharedPtr();

    // TODO(F2): Distinguish modulated and unmodulated bitmap output
    return implDrawBitmap( *rImage,viewState,renderState );
}

}
