/*************************************************************************
 *
 *  $RCSfile: impltools.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 17:13:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <canvas/debug.hxx>

#ifndef  _USE_MATH_DEFINES
#define  _USE_MATH_DEFINES  // needed by Visual C++ for math constants
#endif
#include <math.h>           // M_PI definition

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_REALSIZE2D_HPP__
#include <drafts/com/sun/star/geometry/RealSize2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_REALPOINT2D_HPP__
#include <drafts/com/sun/star/geometry/RealPoint2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_REALRECTANGLE2D_HPP__
#include <drafts/com/sun/star/geometry/RealRectangle2D.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_RENDERSTATE_HPP__
#include <drafts/com/sun/star/rendering/RenderState.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XCANVAS_HPP__
#include <drafts/com/sun/star/rendering/XCanvas.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XBITMAP_HPP__
#include <drafts/com/sun/star/rendering/XBitmap.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP__
#include <drafts/com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_REALBEZIERSEGMENT2D_HPP__
#include <drafts/com/sun/star/geometry/RealBezierSegment2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XINTEGERBITMAP_HPP__
#include <drafts/com/sun/star/rendering/XIntegerBitmap.hpp>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#include <basegfx/tuple/b2dtuple.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif
#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#include <canvas/canvastools.hxx>

#include "impltools.hxx"
#include "linepolypolygon.hxx"
#include "canvasbitmap.hxx"


using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace vclcanvas
{
    namespace tools
    {
        ::basegfx::B2DPolyPolygon polyPolygonFromXPolyPolygon2D( const uno::Reference< rendering::XPolyPolygon2D >& xPoly )
        {
            uno::Reference< lang::XServiceInfo > xRef( xPoly,
                                                       uno::UNO_QUERY );

            if( xRef.is() &&
                xRef->getImplementationName().equals( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(LINEPOLYPOLYGON_IMPLEMENTATION_NAME))) )
            {
                // TODO(Q1): Maybe use dynamic_cast here

                // TODO(F1): Provide true beziers here!
                return static_cast<LinePolyPolygon*>(xPoly.get())->getPolyPolygon();
            }
            else
            {
                // TODO(F1): extract points from polygon interface
                ENSURE_AND_THROW( false,
                                  "polyPolygonFromXPolyPolygon2D(): could not extract points" );
            }

            return ::basegfx::B2DPolyPolygon();
        }

        ::BitmapEx bitmapExFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap )
        {
            uno::Reference< lang::XServiceInfo > xRef( xBitmap,
                                                       uno::UNO_QUERY );

            if( xRef.is() &&
                xRef->getImplementationName().equals( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CANVASBITMAP_IMPLEMENTATION_NAME))) )
            {
                // TODO(Q1): Maybe use dynamic_cast here
                return static_cast<CanvasBitmap*>(xBitmap.get())->getBitmap();
            }
            else
            {
                // TODO(F1): extract pixel from XBitmap interface
                ENSURE_AND_THROW( false,
                                  "bitmapExFromXBitmap(): could not extract bitmap" );
            }

            return ::BitmapEx();
        }

        bool setupFontTransform( ::Point&                       o_rPoint,
                                 ::Font&                        io_rVCLFont,
                                 const rendering::ViewState&    rViewState,
                                 const rendering::RenderState&  rRenderState,
                                 ::OutputDevice&                rOutDev )
        {
            ::basegfx::B2DHomMatrix aMatrix;

            ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                         rViewState,
                                                         rRenderState);

            ::basegfx::B2DTuple aScale;
            ::basegfx::B2DTuple aTranslate;
            double nRotate, nShearX;

            aMatrix.decompose( aScale, aTranslate, nRotate, nShearX );

            // query font metric _before_ tampering with width and height
            if( !::rtl::math::approxEqual(aScale.getX(), aScale.getY()) )
            {
                // retrieve true font width
                const int nFontWidth( rOutDev.GetFontMetric( io_rVCLFont ).GetWidth() );

                const int nScaledFontWidth( ::basegfx::fround(nFontWidth * aScale.getX()) );

                if( !nScaledFontWidth )
                {
                    // scale is smaller than one pixel - disable text
                    // output altogether
                    return false;
                }

                io_rVCLFont.SetWidth( nScaledFontWidth );
            }

            if( !::rtl::math::approxEqual(aScale.getY(), 1.0) )
            {
                const int nFontHeight( io_rVCLFont.GetHeight() );
                io_rVCLFont.SetHeight( ::basegfx::fround(nFontHeight * aScale.getY()) );
            }

            io_rVCLFont.SetOrientation( static_cast< short >( ::basegfx::fround(-fmod(nRotate, 2*M_PI)*(1800.0/M_PI)) ) );

            // TODO(F2): Missing functionality in VCL: shearing
            o_rPoint.X() = ::basegfx::fround(aTranslate.getX());
            o_rPoint.Y() = ::basegfx::fround(aTranslate.getY());

            return true;
        }

        // VCL-Canvas related
        //---------------------------------------------------------------------

        ::Point mapRealPoint2D( const geometry::RealPoint2D&    rPoint,
                                const rendering::ViewState&     rViewState,
                                const rendering::RenderState&   rRenderState )
        {
            ::basegfx::B2DPoint aPoint( ::basegfx::unotools::b2DPointFromRealPoint2D(rPoint) );

            ::basegfx::B2DHomMatrix aMatrix;
            aPoint *= ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                                   rViewState,
                                                                   rRenderState);

            return ::vcl::unotools::pointFromB2DPoint( aPoint );
        }

        ::PolyPolygon mapPolyPolygon( const ::basegfx::B2DPolyPolygon&  rPoly,
                                      const rendering::ViewState&       rViewState,
                                      const rendering::RenderState&     rRenderState )
        {
            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                         rViewState,
                                                         rRenderState);

            ::basegfx::B2DPolyPolygon aTemp( rPoly );

            aTemp.transform( aMatrix );

            return ::PolyPolygon( aTemp );
        }

        ::BitmapEx transformBitmap( const BitmapEx&                 rBitmap,
                                    const rendering::ViewState&     rViewState,
                                    const rendering::RenderState&   rRenderState,
                                    ModulationMode                  eModulationMode )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::tools::transformBitmap()" );
            RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::vclcanvas::tools::transformBitmap: 0x%X", &rBitmap );

            // calc transformation and size of bitmap to be
            // generated. Note, that the translational components are
            // deleted from the transformation; this can be handled by
            // an offset when painting the bitmap
            ::basegfx::B2DHomMatrix aTransform;
            ::canvas::tools::mergeViewAndRenderTransform(aTransform,
                                                         rViewState,
                                                         rRenderState);
            aTransform.set(0,2,0.0);
            aTransform.set(1,2,0.0);

            const Size                  aBmpSize( rBitmap.GetSizePixel() );
            ::basegfx::B2DRectangle     aDestRect;

            bool bCopyBack( false );

            ::canvas::tools::calcTransformedRectBounds( aDestRect,
                                                        ::basegfx::B2DRectangle(0,
                                                                                0,
                                                                                aBmpSize.Width(),
                                                                                aBmpSize.Height()),
                                                        aTransform );

            const bool bModulateColors( eModulationMode == MODULATE_WITH_DEVICECOLOR &&
                                        rRenderState.DeviceColor.getLength() > 2 );
            const double nRedModulation( bModulateColors ? rRenderState.DeviceColor[0] : 1.0 );
            const double nGreenModulation( bModulateColors ? rRenderState.DeviceColor[1] : 1.0 );
            const double nBlueModulation( bModulateColors ? rRenderState.DeviceColor[2] : 1.0 );
            const double nAlphaModulation( bModulateColors && rRenderState.DeviceColor.getLength() > 3 ?
                                           rRenderState.DeviceColor[3] : 1.0 );

            Bitmap aSrcBitmap( rBitmap.GetBitmap() );
            Bitmap aSrcAlpha;

            // differentiate mask and alpha channel (on-off
            // vs. multi-level transparency)
            if( rBitmap.IsTransparent() )
            {
                if( rBitmap.IsAlpha() )
                    aSrcAlpha = rBitmap.GetAlpha().GetBitmap();
                else
                    aSrcAlpha = rBitmap.GetMask();
            }

            ScopedBitmapReadAccess pReadAccess( aSrcBitmap.AcquireReadAccess(),
                                                aSrcBitmap );
            ScopedBitmapReadAccess pAlphaReadAccess( rBitmap.IsTransparent() ?
                                                     aSrcAlpha.AcquireReadAccess() :
                                                     (BitmapReadAccess*)NULL,
                                                     aSrcAlpha );

            if( pReadAccess.get() == NULL ||
                (pAlphaReadAccess.get() == NULL && rBitmap.IsTransparent()) )
            {
                // TODO(E2): Error handling!
                ENSURE_AND_THROW( false,
                                  "transformBitmap(): could not access source bitmap" );
            }

            const Size aDestBmpSize( ::basegfx::fround( aDestRect.getMaxX() ),
                                     ::basegfx::fround( aDestRect.getMaxY() ) );

            Bitmap aDstBitmap( aDestBmpSize, aSrcBitmap.GetBitCount(), &pReadAccess->GetPalette() );
            Bitmap aDstAlpha( AlphaMask( aDestBmpSize ).GetBitmap() );

            {
                // just to be on the safe side: let the
                // ScopedAccessors get destructed before
                // copy-constructing the resulting bitmap. This will
                // rule out the possibility that cached accessor data
                // is not yet written back.
                ScopedBitmapWriteAccess pWriteAccess( aDstBitmap.AcquireWriteAccess(),
                                                      aDstBitmap );
                ScopedBitmapWriteAccess pAlphaWriteAccess( aDstAlpha.AcquireWriteAccess(),
                                                           aDstAlpha );


                if( pWriteAccess.get() != NULL &&
                    pAlphaWriteAccess.get() != NULL &&
                    aTransform.isInvertible() )
                {
                    // we're doing inverse mapping here, i.e. mapping
                    // points from the destination bitmap back to the
                    // source
                    aTransform.invert();

                    // for the time being, always read as ARGB
                    for( int y=0; y<aDestBmpSize.Height(); ++y )
                    {
                        if( bModulateColors )
                        {
                            // TODO(P2): Have different branches for
                            // alpha-only modulation (color
                            // modulations eq. 1.0)

                            // modulate all color channels with given
                            // values

                            // differentiate mask and alpha channel (on-off
                            // vs. multi-level transparency)
                            if( rBitmap.IsTransparent() )
                            {
                                // Handling alpha and mask just the same...
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(255) );
                                    }
                                    else
                                    {
                                        // modulate alpha with
                                        // nAlphaModulation. This is a
                                        // little bit verbose, formula
                                        // is 255 - (255-pixAlpha)*nAlphaModulation
                                        // (invert 'alpha' pixel value,
                                        // to get the standard alpha
                                        // channel behaviour)
                                        pAlphaWriteAccess->SetPixel( y, x,
                                                                     BitmapColor(
                                                                         255U -
                                                                         static_cast<BYTE>(
                                                                             nAlphaModulation*
                                                                             (255U
                                                                              - pAlphaReadAccess->GetPixel(
                                                                                  nSrcY,
                                                                                  nSrcX ).GetIndex() ) + .5 ) ) );

                                        BitmapColor aColor( pReadAccess->GetPixel( nSrcY,
                                                                                   nSrcX ) );

                                        aColor.SetRed(
                                            static_cast<BYTE>(
                                                nRedModulation *
                                                aColor.GetRed() + .5 ));
                                        aColor.SetGreen(
                                            static_cast<BYTE>(
                                                nGreenModulation *
                                                aColor.GetGreen() + .5 ));
                                        aColor.SetBlue(
                                            static_cast<BYTE>(
                                                nBlueModulation *
                                                aColor.GetBlue() + .5 ));

                                        pWriteAccess->SetPixel( y, x,
                                                                aColor );
                                    }
                                }
                            }
                            else
                            {
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(255) );
                                    }
                                    else
                                    {
                                        // modulate alpha with
                                        // nAlphaModulation. This is a
                                        // little bit verbose, formula
                                        // is 255 - 255*nAlphaModulation
                                        // (invert 'alpha' pixel value,
                                        // to get the standard alpha
                                        // channel behaviour)
                                        pAlphaWriteAccess->SetPixel( y, x,
                                                                     BitmapColor(
                                                                         255U -
                                                                         static_cast<BYTE>(
                                                                             nAlphaModulation*255.0
                                                                             + .5 ) ) );

                                        BitmapColor aColor( pReadAccess->GetPixel( nSrcY,
                                                                                   nSrcX ) );

                                        aColor.SetRed(
                                            static_cast<BYTE>(
                                                nRedModulation *
                                                aColor.GetRed() + .5 ));
                                        aColor.SetGreen(
                                            static_cast<BYTE>(
                                                nGreenModulation *
                                                aColor.GetGreen() + .5 ));
                                        aColor.SetBlue(
                                            static_cast<BYTE>(
                                                nBlueModulation *
                                                aColor.GetBlue() + .5 ));

                                        pWriteAccess->SetPixel( y, x,
                                                                aColor );
                                    }
                                }
                            }
                        }
                        else
                        {
                            // differentiate mask and alpha channel (on-off
                            // vs. multi-level transparency)
                            if( rBitmap.IsTransparent() )
                            {
                                // Handling alpha and mask just the same...
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(255) );
                                    }
                                    else
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x,
                                                                     pAlphaReadAccess->GetPixel( nSrcY,
                                                                                                 nSrcX ) );

                                        pWriteAccess->SetPixel( y, x, pReadAccess->GetPixel( nSrcY,
                                                                                             nSrcX ) );
                                    }
                                }
                            }
                            else
                            {
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(255) );
                                    }
                                    else
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(0) );
                                        pWriteAccess->SetPixel( y, x, pReadAccess->GetPixel( nSrcY,
                                                                                             nSrcX ) );
                                    }
                                }
                            }
                        }
                    }

                    bCopyBack = true;
                }
                else
                {
                    // TODO(E2): Error handling!
                    ENSURE_AND_THROW( false,
                                      "transformBitmap(): could not access bitmap" );
                }
            }

            if( bCopyBack )
                return BitmapEx( aDstBitmap, AlphaMask( aDstAlpha ) );
            else
                return BitmapEx();
        }
    }
}
