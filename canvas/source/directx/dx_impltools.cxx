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


#include <ctype.h> // don't ask. msdev breaks otherwise...
#include <basegfx/numeric/ftools.hxx>

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2irectangle.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <canvas/canvastools.hxx>
#include <canvas/verifyinput.hxx>

#include "dx_impltools.hxx"
#include "dx_vcltools.hxx"
#include "dx_linepolypolygon.hxx"
#include "dx_canvasbitmap.hxx"
#include "dx_canvasfont.hxx"
#include "dx_canvas.hxx"
#include "dx_spritecanvas.hxx"

#include <boost/scoped_array.hpp>

#include <vector>
#include <algorithm>


using namespace ::com::sun::star;


namespace dxcanvas
{
    namespace tools
    {
        ::basegfx::B2DPolyPolygon polyPolygonFromXPolyPolygon2D( const uno::Reference< rendering::XPolyPolygon2D >& xPoly )
        {
            LinePolyPolygon* pPolyImpl = dynamic_cast< LinePolyPolygon* >( xPoly.get() );

            if( pPolyImpl )
            {
                return pPolyImpl->getPolyPolygon();
            }
            else
            {
                const sal_Int32 nPolys( xPoly->getNumberOfPolygons() );

                // not a known implementation object - try data source
                // interfaces
                uno::Reference< rendering::XBezierPolyPolygon2D > xBezierPoly(
                    xPoly,
                    uno::UNO_QUERY );

                if( xBezierPoly.is() )
                {
                    return ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence(
                        xBezierPoly->getBezierSegments( 0,
                                                        nPolys,
                                                        0,
                                                        -1 ) );
                }
                else
                {
                    uno::Reference< rendering::XLinePolyPolygon2D > xLinePoly(
                        xPoly,
                        uno::UNO_QUERY );

                    // no implementation class and no data provider
                    // found - contract violation.
                    ENSURE_ARG_OR_THROW( xLinePoly.is(),
                                     "VCLCanvas::polyPolygonFromXPolyPolygon2D(): Invalid input "
                                     "poly-polygon, cannot retrieve vertex data" );

                    return ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence(
                        xLinePoly->getPoints( 0,
                                              nPolys,
                                              0,
                                              -1 ) );
                }
            }
        }

        void setupGraphics( Gdiplus::Graphics& rGraphics )
        {
            // setup graphics with (somewhat arbitrary) defaults
            //rGraphics.SetCompositingQuality( Gdiplus::CompositingQualityHighQuality );
            rGraphics.SetCompositingQuality( Gdiplus::CompositingQualityHighSpeed );
            //rGraphics.SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBilinear ); // with prefiltering for shrinks
            rGraphics.SetInterpolationMode( Gdiplus::InterpolationModeBilinear );

            // #122683# Switched precedence of pixel offset
            // mode. Seemingly, polygon stroking needs
            // PixelOffsetModeNone to achieve visually pleasing
            // results, whereas all other operations (e.g. polygon
            // fills, bitmaps) look better with PixelOffsetModeHalf.
            rGraphics.SetPixelOffsetMode( Gdiplus::PixelOffsetModeHalf ); // Pixel center at (0.5, 0.5) etc.
            //rGraphics.SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );

            //rGraphics.SetSmoothingMode( Gdiplus::SmoothingModeHighSpeed ); // no line/curve antialiasing
            //rGraphics.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );
            rGraphics.SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );
            //rGraphics.SetTextRenderingHint( Gdiplus::TextRenderingHintAntiAlias );
            rGraphics.SetTextRenderingHint( Gdiplus::TextRenderingHintSystemDefault );
            rGraphics.SetPageUnit(Gdiplus::UnitPixel);
        }

        Gdiplus::Graphics* createGraphicsFromHDC(HDC aHDC)
        {
            Gdiplus::Graphics* pRet = new Gdiplus::Graphics(aHDC);
            if( pRet )
                setupGraphics( *pRet );
            return pRet;
        }

        Gdiplus::Graphics* createGraphicsFromBitmap(const BitmapSharedPtr& rBitmap)
        {
            Gdiplus::Graphics* pRet = Gdiplus::Graphics::FromImage(rBitmap.get());
            if( pRet )
                setupGraphics( *pRet );
            return pRet;
        }

        void gdiPlusMatrixFromB2DHomMatrix( Gdiplus::Matrix& rGdiplusMatrix, const ::basegfx::B2DHomMatrix& rMatrix )
        {
            rGdiplusMatrix.SetElements( static_cast<Gdiplus::REAL>(rMatrix.get(0,0)),
                                        static_cast<Gdiplus::REAL>(rMatrix.get(1,0)),
                                        static_cast<Gdiplus::REAL>(rMatrix.get(0,1)),
                                        static_cast<Gdiplus::REAL>(rMatrix.get(1,1)),
                                        static_cast<Gdiplus::REAL>(rMatrix.get(0,2)),
                                        static_cast<Gdiplus::REAL>(rMatrix.get(1,2)) );
        }

        void gdiPlusMatrixFromAffineMatrix2D( Gdiplus::Matrix&                  rGdiplusMatrix,
                                              const geometry::AffineMatrix2D&   rMatrix )
        {
            rGdiplusMatrix.SetElements( static_cast<Gdiplus::REAL>(rMatrix.m00),
                                        static_cast<Gdiplus::REAL>(rMatrix.m10),
                                        static_cast<Gdiplus::REAL>(rMatrix.m01),
                                        static_cast<Gdiplus::REAL>(rMatrix.m11),
                                        static_cast<Gdiplus::REAL>(rMatrix.m02),
                                        static_cast<Gdiplus::REAL>(rMatrix.m12) );
        }

        namespace
        {
            // TODO(P2): Check whether this gets inlined. If not, make functor
            // out of it
            inline Gdiplus::PointF implGdiPlusPointFromRealPoint2D( const ::com::sun::star::geometry::RealPoint2D& rPoint )
            {
                return Gdiplus::PointF( static_cast<Gdiplus::REAL>(rPoint.X),
                                        static_cast<Gdiplus::REAL>(rPoint.Y) );
            }

            void graphicsPathFromB2DPolygon( GraphicsPathSharedPtr&             rOutput,
                                             ::std::vector< Gdiplus::PointF >&  rPoints,
                                             const ::basegfx::B2DPolygon&       rPoly,
                                             bool bNoLineJoin)
            {
                const sal_uInt32 nPoints( rPoly.count() );

                if( nPoints < 2 )
                    return;

                rOutput->StartFigure();

                const bool bClosedPolygon( rPoly.isClosed() );

                if( rPoly.areControlPointsUsed() )
                {
                    // control points used -> for now, add all
                    // segments as curves to GraphicsPath

                    // If the polygon is closed, we need to add the
                    // first point, thus, one more (can't simply
                    // GraphicsPath::CloseFigure() it, since the last
                    // point cannot have any control points for GDI+)
                    rPoints.resize( 3*nPoints + bClosedPolygon );

                    sal_uInt32 nCurrOutput=0;
                    for( sal_uInt32 nCurrPoint=0; nCurrPoint<nPoints; ++nCurrPoint )
                    {
                        const ::basegfx::B2DPoint& rPoint( rPoly.getB2DPoint( nCurrPoint ) );
                        rPoints[nCurrOutput++] = Gdiplus::PointF( static_cast<Gdiplus::REAL>(rPoint.getX()),
                                                                  static_cast<Gdiplus::REAL>(rPoint.getY()) );

                        const ::basegfx::B2DPoint& rControlPointA( rPoly.getNextControlPoint( nCurrPoint ) );
                        rPoints[nCurrOutput++] = Gdiplus::PointF( static_cast<Gdiplus::REAL>(rControlPointA.getX()),
                                                                  static_cast<Gdiplus::REAL>(rControlPointA.getY()) );

                        const ::basegfx::B2DPoint& rControlPointB( rPoly.getPrevControlPoint( (nCurrPoint + 1) % nPoints) );
                        rPoints[nCurrOutput++] = Gdiplus::PointF( static_cast<Gdiplus::REAL>(rControlPointB.getX()),
                                                                  static_cast<Gdiplus::REAL>(rControlPointB.getY()) );
                    }

                    if( bClosedPolygon )
                    {
                        // add first point again (to be able to pass
                        // control points for the last point, see
                        // above)
                        const ::basegfx::B2DPoint& rPoint( rPoly.getB2DPoint(0) );
                        rPoints[nCurrOutput++] = Gdiplus::PointF( static_cast<Gdiplus::REAL>(rPoint.getX()),
                                                                  static_cast<Gdiplus::REAL>(rPoint.getY()) );

                        if(bNoLineJoin && nCurrOutput > 7)
                        {
                            for(sal_uInt32 a(3); a < nCurrOutput; a+=3)
                            {
                                rOutput->StartFigure();
                                rOutput->AddBezier(rPoints[a - 3], rPoints[a - 2], rPoints[a - 1], rPoints[a]);
                            }
                        }
                        else
                        {
                            rOutput->AddBeziers( &rPoints[0], nCurrOutput );
                        }
                    }
                    else
                    {
                        // GraphicsPath expects 3(n-1)+1 points (i.e. the
                        // last point must not have any trailing control
                        // points after it).
                        // Therefore, simply don't pass the last two
                        // points here.
                        if( nCurrOutput > 3 )
                        {
                            if(bNoLineJoin && nCurrOutput > 7)
                            {
                                for(sal_uInt32 a(3); a < nCurrOutput; a+=3)
                                {
                                    rOutput->StartFigure();
                                    rOutput->AddBezier(rPoints[a - 3], rPoints[a - 2], rPoints[a - 1], rPoints[a]);
                                }
                            }
                            else
                            {
                                rOutput->AddBeziers( &rPoints[0], nCurrOutput-2 );
                            }
                        }
                    }
                }
                else
                {
                    // no control points -> no curves, simply add
                    // straigt lines to GraphicsPath
                    rPoints.resize( nPoints );

                    for( sal_uInt32 nCurrPoint=0; nCurrPoint<nPoints; ++nCurrPoint )
                    {
                        const ::basegfx::B2DPoint& rPoint( rPoly.getB2DPoint( nCurrPoint ) );
                        rPoints[nCurrPoint] = Gdiplus::PointF( static_cast<Gdiplus::REAL>(rPoint.getX()),
                                                               static_cast<Gdiplus::REAL>(rPoint.getY()) );
                    }

                    if(bNoLineJoin && nPoints > 2)
                    {
                        for(sal_uInt32 a(1); a < nPoints; a++)
                        {
                            rOutput->StartFigure();
                            rOutput->AddLine(rPoints[a - 1], rPoints[a]);
                        }

                        if(bClosedPolygon)
                        {
                            rOutput->StartFigure();
                            rOutput->AddLine(rPoints[nPoints - 1], rPoints[0]);
                        }
                    }
                    else
                    {
                        rOutput->AddLines( &rPoints[0], nPoints );
                    }
                }

                if( bClosedPolygon && !bNoLineJoin )
                    rOutput->CloseFigure();
            }
        }

        Gdiplus::PointF gdiPlusPointFromRealPoint2D( const ::com::sun::star::geometry::RealPoint2D& rPoint )
        {
            return implGdiPlusPointFromRealPoint2D( rPoint );
        }

        Gdiplus::Rect gdiPlusRectFromIntegerRectangle2D( const geometry::IntegerRectangle2D& rRect )
        {
            return Gdiplus::Rect( rRect.X1,
                                  rRect.Y1,
                                  rRect.X2 - rRect.X1,
                                  rRect.Y2 - rRect.Y1 );
        }

        Gdiplus::RectF gdiPlusRectFFromRectangle2D( const geometry::RealRectangle2D& rRect )
        {
            return Gdiplus::RectF( static_cast<Gdiplus::REAL>(rRect.X1),
                                   static_cast<Gdiplus::REAL>(rRect.Y1),
                                   static_cast<Gdiplus::REAL>(rRect.X2 - rRect.X1),
                                   static_cast<Gdiplus::REAL>(rRect.Y2 - rRect.Y1) );
        }

        RECT gdiRectFromB2IRect( const ::basegfx::B2IRange& rRect )
        {
            RECT aRect = {rRect.getMinX(),
                          rRect.getMinY(),
                          rRect.getMaxX(),
                          rRect.getMaxY()};

            return aRect;
        }

        geometry::RealPoint2D realPoint2DFromGdiPlusPointF( const Gdiplus::PointF& rPoint )
        {
            return geometry::RealPoint2D( rPoint.X, rPoint.Y );
        }

        geometry::RealRectangle2D realRectangle2DFromGdiPlusRectF( const Gdiplus::RectF& rRect )
        {
            return geometry::RealRectangle2D( rRect.X, rRect.Y,
                                              rRect.X + rRect.Width,
                                              rRect.Y + rRect.Height );
        }

        ::basegfx::B2DPoint b2dPointFromGdiPlusPointF( const Gdiplus::PointF& rPoint )
        {
            return ::basegfx::B2DPoint( rPoint.X, rPoint.Y );
        }

        ::basegfx::B2DRange b2dRangeFromGdiPlusRectF( const Gdiplus::RectF& rRect )
        {
            return ::basegfx::B2DRange( rRect.X, rRect.Y,
                                        rRect.X + rRect.Width,
                                        rRect.Y + rRect.Height );
        }

        uno::Sequence< double > argbToDoubleSequence( const Gdiplus::ARGB& rColor )
        {
            // TODO(F1): handle color space conversions, when defined on canvas/graphicDevice
            uno::Sequence< double > aRet(4);

            aRet[0] = ((rColor >> 16) & 0xFF) / 255.0;  // red
            aRet[1] = ((rColor >> 8) & 0xFF) / 255.0;   // green
            aRet[2] = (rColor & 0xFF) / 255.0;          // blue
            aRet[3] = ((rColor >> 24) & 0xFF) / 255.0;  // alpha

            return aRet;
        }

        uno::Sequence< sal_Int8 > argbToIntSequence( const Gdiplus::ARGB& rColor )
        {
            // TODO(F1): handle color space conversions, when defined on canvas/graphicDevice
            uno::Sequence< sal_Int8 > aRet(4);

            aRet[0] = static_cast<sal_Int8>((rColor >> 16) & 0xFF); // red
            aRet[1] = static_cast<sal_Int8>((rColor >> 8) & 0xFF);  // green
            aRet[2] = static_cast<sal_Int8>(rColor & 0xFF);         // blue
            aRet[3] = static_cast<sal_Int8>((rColor >> 24) & 0xFF); // alpha

            return aRet;
        }

        Gdiplus::ARGB sequenceToArgb( const uno::Sequence< sal_Int8 >& rColor )
        {
            ENSURE_OR_THROW( rColor.getLength() > 2,
                              "sequenceToArgb: need at least three channels" );

            // TODO(F1): handle color space conversions, when defined on canvas/graphicDevice
            Gdiplus::ARGB aColor;

            aColor = (static_cast<sal_uInt8>(rColor[0]) << 16) | (static_cast<sal_uInt8>(rColor[1]) << 8) | static_cast<sal_uInt8>(rColor[2]);

            if( rColor.getLength() > 3 )
                aColor |= static_cast<sal_uInt8>(rColor[3]) << 24;

            return aColor;
        }

        Gdiplus::ARGB sequenceToArgb( const uno::Sequence< double >& rColor )
        {
            ENSURE_OR_THROW( rColor.getLength() > 2,
                              "sequenceToColor: need at least three channels" );

            // TODO(F1): handle color space conversions, when defined on canvas/graphicDevice
            Gdiplus::ARGB aColor;

            ::canvas::tools::verifyRange(rColor[0],0.0,1.0);
            ::canvas::tools::verifyRange(rColor[1],0.0,1.0);
            ::canvas::tools::verifyRange(rColor[2],0.0,1.0);

            aColor =
                (static_cast<sal_uInt8>( ::basegfx::fround( 255*rColor[0] ) ) << 16) |
                (static_cast<sal_uInt8>( ::basegfx::fround( 255*rColor[1] ) ) << 8) |
                static_cast<sal_uInt8>( ::basegfx::fround( 255*rColor[2] ) );

            if( rColor.getLength() > 3 )
            {
                ::canvas::tools::verifyRange(rColor[3],0.0,1.0);
                aColor |= static_cast<sal_uInt8>( ::basegfx::fround( 255*rColor[3] ) ) << 24;
            }

            return aColor;
        }

        GraphicsPathSharedPtr graphicsPathFromRealPoint2DSequence( const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >& points )
        {
            GraphicsPathSharedPtr pRes( new Gdiplus::GraphicsPath() );
            ::std::vector< Gdiplus::PointF > aPoints;

            sal_Int32 nCurrPoly;
            for( nCurrPoly=0; nCurrPoly<points.getLength(); ++nCurrPoly )
            {
                const sal_Int32 nCurrSize( points[nCurrPoly].getLength() );
                if( nCurrSize )
                {
                    aPoints.resize( nCurrSize );

                    // TODO(F1): Closed/open polygons

                    // convert from RealPoint2D array to Gdiplus::PointF array
                    ::std::transform( points[nCurrPoly].getConstArray(),
                                      points[nCurrPoly].getConstArray()+nCurrSize,
                                      aPoints.begin(),
                                      implGdiPlusPointFromRealPoint2D );

                    pRes->AddLines( &aPoints[0], nCurrSize );
                }
            }

            return pRes;
        }

        GraphicsPathSharedPtr graphicsPathFromB2DPolygon( const ::basegfx::B2DPolygon& rPoly, bool bNoLineJoin )
        {
            GraphicsPathSharedPtr               pRes( new Gdiplus::GraphicsPath() );
            ::std::vector< Gdiplus::PointF >    aPoints;

            graphicsPathFromB2DPolygon( pRes, aPoints, rPoly, bNoLineJoin );

            return pRes;
        }

        GraphicsPathSharedPtr graphicsPathFromB2DPolyPolygon( const ::basegfx::B2DPolyPolygon& rPoly, bool bNoLineJoin )
        {
            GraphicsPathSharedPtr               pRes( new Gdiplus::GraphicsPath() );
            ::std::vector< Gdiplus::PointF >    aPoints;

            const sal_uInt32 nPolies( rPoly.count() );
            for( sal_uInt32 nCurrPoly=0; nCurrPoly<nPolies; ++nCurrPoly )
            {
                graphicsPathFromB2DPolygon( pRes,
                                            aPoints,
                                            rPoly.getB2DPolygon( nCurrPoly ),
                                            bNoLineJoin);
            }

            return pRes;
        }

        GraphicsPathSharedPtr graphicsPathFromXPolyPolygon2D( const uno::Reference< rendering::XPolyPolygon2D >& xPoly, bool bNoLineJoin )
        {
            LinePolyPolygon* pPolyImpl = dynamic_cast< LinePolyPolygon* >( xPoly.get() );

            if( pPolyImpl )
            {
                return pPolyImpl->getGraphicsPath( bNoLineJoin );
            }
            else
            {
                return tools::graphicsPathFromB2DPolyPolygon(
                    polyPolygonFromXPolyPolygon2D( xPoly ), bNoLineJoin );
            }
        }

        bool drawGdiPlusBitmap( const GraphicsSharedPtr& rGraphics,
                                const BitmapSharedPtr&   rBitmap )
        {
            Gdiplus::PointF aPoint;
            return (Gdiplus::Ok == rGraphics->DrawImage( rBitmap.get(),
                                                         aPoint ) );
        }

        bool drawDIBits( const GraphicsSharedPtr& rGraphics,
                         const BITMAPINFO&        rBI,
                         const void*              pBits )
        {
            BitmapSharedPtr pBitmap(
                Gdiplus::Bitmap::FromBITMAPINFO( &rBI,
                                                 (void*)pBits ) );

            return drawGdiPlusBitmap( rGraphics,
                                      pBitmap );
        }

        bool drawRGBABits( const GraphicsSharedPtr& rGraphics,
                           const RawRGBABitmap&     rRawRGBAData )
        {
            BitmapSharedPtr pBitmap( new Gdiplus::Bitmap( rRawRGBAData.mnWidth,
                                                          rRawRGBAData.mnHeight,
                                                          PixelFormat32bppARGB ) );

            Gdiplus::BitmapData aBmpData;
            aBmpData.Width       = rRawRGBAData.mnWidth;
            aBmpData.Height      = rRawRGBAData.mnHeight;
            aBmpData.Stride      = 4*aBmpData.Width; // bottom-up format
            aBmpData.PixelFormat = PixelFormat32bppARGB;
            aBmpData.Scan0       = rRawRGBAData.mpBitmapData.get();

            const Gdiplus::Rect aRect( 0,0,aBmpData.Width,aBmpData.Height );
            if( Gdiplus::Ok != pBitmap->LockBits( &aRect,
                                                  Gdiplus::ImageLockModeWrite | Gdiplus::ImageLockModeUserInputBuf,
                                                  PixelFormat32bppARGB,
                                                  &aBmpData ) )
            {
                return false;
            }

            // commit data to bitmap
            pBitmap->UnlockBits( &aBmpData );

            return drawGdiPlusBitmap( rGraphics,
                                      pBitmap );
        }

        BitmapSharedPtr bitmapFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap )
        {
            BitmapProvider* pBitmapProvider = dynamic_cast< BitmapProvider* >(xBitmap.get());

            if( pBitmapProvider )
            {
                IBitmapSharedPtr pBitmap( pBitmapProvider->getBitmap() );
                return pBitmap->getBitmap();
            }
            else
            {
                // not a native CanvasBitmap, extract VCL bitmap and
                // render into GDI+ bitmap of similar size
                // =================================================

                const geometry::IntegerSize2D aBmpSize( xBitmap->getSize() );
                BitmapSharedPtr               pBitmap;

                if( xBitmap->hasAlpha() )
                {
                    // TODO(P2): At least for the alpha bitmap case, it
                    // would be possible to generate the corresponding
                    // bitmap directly
                    pBitmap.reset( new Gdiplus::Bitmap( aBmpSize.Width,
                                                        aBmpSize.Height,
                                                        PixelFormat32bppARGB ) );
                }
                else
                {
                    // TODO(F2): Might be wise to create bitmap compatible
                    // to the VCL bitmap. Also, check whether the VCL
                    // bitmap's system handles can be used to create the
                    // GDI+ bitmap (currently, it does not seem so).
                    pBitmap.reset( new Gdiplus::Bitmap( aBmpSize.Width,
                                                        aBmpSize.Height,
                                                        PixelFormat24bppRGB ) );
                }

                GraphicsSharedPtr pGraphics(createGraphicsFromBitmap(pBitmap));
                tools::setupGraphics(*pGraphics);
                if( !drawVCLBitmapFromXBitmap(
                        pGraphics,
                        xBitmap) )
                {
                    pBitmap.reset();
                }

                return pBitmap;
            }
        }

        CanvasFont::ImplRef canvasFontFromXFont( const uno::Reference< rendering::XCanvasFont >& xFont )
        {
            CanvasFont* pCanvasFont = dynamic_cast< CanvasFont* >(xFont.get());

            ENSURE_ARG_OR_THROW( pCanvasFont,
                             "canvasFontFromXFont(): Invalid XFont (or incompatible font for this XCanvas)" );

            return CanvasFont::ImplRef( pCanvasFont );
        }

        void setModulateImageAttributes( Gdiplus::ImageAttributes& o_rAttr,
                                         double                    nRedModulation,
                                         double                    nGreenModulation,
                                         double                    nBlueModulation,
                                         double                    nAlphaModulation )
        {
            // This gets rather verbose, but we have to setup a color
            // transformation matrix, in order to incorporate the global
            // alpha value mfAlpha into the bitmap rendering.
            Gdiplus::ColorMatrix     aColorMatrix;

            aColorMatrix.m[0][0] = static_cast<Gdiplus::REAL>(nRedModulation);
            aColorMatrix.m[0][1] = 0.0;
            aColorMatrix.m[0][2] = 0.0;
            aColorMatrix.m[0][3] = 0.0;
            aColorMatrix.m[0][4] = 0.0;

            aColorMatrix.m[1][0] = 0.0;
            aColorMatrix.m[1][1] = static_cast<Gdiplus::REAL>(nGreenModulation);
            aColorMatrix.m[1][2] = 0.0;
            aColorMatrix.m[1][3] = 0.0;
            aColorMatrix.m[1][4] = 0.0;

            aColorMatrix.m[2][0] = 0.0;
            aColorMatrix.m[2][1] = 0.0;
            aColorMatrix.m[2][2] = static_cast<Gdiplus::REAL>(nBlueModulation);
            aColorMatrix.m[2][3] = 0.0;
            aColorMatrix.m[2][4] = 0.0;

            aColorMatrix.m[3][0] = 0.0;
            aColorMatrix.m[3][1] = 0.0;
            aColorMatrix.m[3][2] = 0.0;
            aColorMatrix.m[3][3] = static_cast<Gdiplus::REAL>(nAlphaModulation);
            aColorMatrix.m[3][4] = 0.0;

            aColorMatrix.m[4][0] = 0.0;
            aColorMatrix.m[4][1] = 0.0;
            aColorMatrix.m[4][2] = 0.0;
            aColorMatrix.m[4][3] = 0.0;
            aColorMatrix.m[4][4] = 1.0;

            o_rAttr.SetColorMatrix( &aColorMatrix,
                                    Gdiplus::ColorMatrixFlagsDefault,
                                    Gdiplus::ColorAdjustTypeDefault );
        }

    } // namespace tools
} // namespace dxcanvas

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
