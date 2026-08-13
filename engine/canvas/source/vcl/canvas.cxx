/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <canvas.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/StrokeAttributes.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>

#include <textlayout.hxx>
#include <canvastools.hxx>

using namespace ::com::sun::star;

namespace vclcanvas
{
    namespace
    {
        basegfx::B2DLineJoin b2DJoineFromJoin( sal_Int8 nJoinType )
        {
            switch( nJoinType )
            {
                case rendering::PathJoinType::NONE:
                    return basegfx::B2DLineJoin::NONE;

                case rendering::PathJoinType::MITER:
                    return basegfx::B2DLineJoin::Miter;

                case rendering::PathJoinType::ROUND:
                    return basegfx::B2DLineJoin::Round;

                case rendering::PathJoinType::BEVEL:
                    return basegfx::B2DLineJoin::Bevel;

                default:
                    ENSURE_OR_THROW( false,
                                      "b2DJoineFromJoin(): Unexpected join type" );
            }

            return basegfx::B2DLineJoin::NONE;
        }

        drawing::LineCap unoCapeFromCap( sal_Int8 nCapType)
        {
            switch ( nCapType)
            {
                case rendering::PathCapType::BUTT:
                    return drawing::LineCap_BUTT;

                case rendering::PathCapType::ROUND:
                    return drawing::LineCap_ROUND;

                case rendering::PathCapType::SQUARE:
                    return drawing::LineCap_SQUARE;

                default:
                    ENSURE_OR_THROW( false,
                                      "unoCapeFromCap(): Unexpected cap type" );
            }
            return drawing::LineCap_BUTT;
        }
    }

    Canvas::Canvas( OutputDevice& rOutDev )
    {
        SolarMutexGuard aGuard;

        SAL_INFO("canvas.vcl", "vclcanvas::Canvas() called" );

        mxOutDev = &rOutDev;
    }


    Canvas::~Canvas()
    {
        SAL_INFO("canvas.vcl", "VCLCanvas destroyed" );
    }

    bool Canvas::repaint( const GraphicObjectSharedPtr& rGrf,
                          const vclcanvas::ViewState&   viewState,
                          const vclcanvas::RenderState& renderState,
                          const ::Point&                rPt,
                          const ::Size&                 rSz,
                          const GraphicAttr&            rAttr ) const
    {
        SolarMutexGuard aGuard;

        ENSURE_OR_RETURN_FALSE( rGrf,
                          "Invalid Graphic" );

        vclcanvastools::OutDevStateKeeper aStateKeeper( *mxOutDev );
        setupOutDevState( viewState, renderState, IGNORE_COLOR );

        if (!rGrf->Draw(*mxOutDev, rPt, rSz, &rAttr))
            return false;

        return true;
    }

    basegfx::B2DPolyPolygon Canvas::createCompatibleLinePolyPolygon( const cpo::uno::Sequence< cpo::uno::Sequence< css::geometry::RealPoint2D > >& points )
    {
        vclcanvastools::LocalGuard aGuard( m_aMutex );

        // vcl only handles even_odd polygons
        return ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points );
    }

    void Canvas::clear()
    {
        vclcanvastools::LocalGuard aGuard( m_aMutex );

        OutputDevice& rOutDev( *mxOutDev );
        vclcanvastools::OutDevStateKeeper aStateKeeper( rOutDev );

        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( AntialiasingFlags::Enable );
        rOutDev.SetLineColor( COL_WHITE );
        rOutDev.SetFillColor( COL_WHITE );
        rOutDev.SetClipRegion();
        rOutDev.DrawRect( ::tools::Rectangle( Point(),
                                     rOutDev.GetOutputSizePixel()) );
    }

    void Canvas::drawLine(const css::geometry::RealPoint2D&  aStartRealPoint2D,
                                   const css::geometry::RealPoint2D&  aEndRealPoint2D,
                                   const ::vclcanvas::ViewState&   viewState,
                                   const ::vclcanvas::RenderState& renderState)
    {
        canvastools::verifyArgs(aStartRealPoint2D, aEndRealPoint2D, viewState, renderState,
                          __func__);

        vclcanvastools::LocalGuard aGuard( m_aMutex );

        // nope, render
        vclcanvastools::OutDevStateKeeper aStateKeeper( *mxOutDev );
        setupOutDevState( viewState, renderState, LINE_COLOR );

        const Point aStartPoint( vclcanvastools::mapRealPoint2D( aStartRealPoint2D,
                                                        viewState, renderState ) );
        const Point aEndPoint( vclcanvastools::mapRealPoint2D( aEndRealPoint2D,
                                                      viewState, renderState ) );
        // TODO(F2): alpha
        mxOutDev->DrawLine( aStartPoint, aEndPoint );
    }

    rtl::Reference< vclcanvas::CachedBitmap >
        Canvas::drawBitmap( const Bitmap&                                                   rBitmap,
                    const ::vclcanvas::ViewState&                                   viewState,
                    const ::vclcanvas::RenderState&                                 renderState )
    {
        vclcanvastools::LocalGuard aGuard( m_aMutex );

        return implDrawBitmap( rBitmap,
                               viewState,
                               renderState,
                               false );
    }

    rtl::Reference< vclcanvas::CachedBitmap > Canvas::drawBitmapModulated( const Bitmap&    rBitmap,
                                                                         const vclcanvas::ViewState&                    viewState,
                                                                         const vclcanvas::RenderState&                  renderState )
    {
        return implDrawBitmap( rBitmap,
                               viewState,
                               renderState,
                               true );
    }

    void
        Canvas::strokePolyPolygon(const basegfx::B2DPolyPolygon&                      xPolyPolygon,
                          const ::vclcanvas::ViewState&                               viewState,
                          const ::vclcanvas::RenderState&                             renderState,
                          const css::rendering::StrokeAttributes&                        strokeAttributes)
    {
        canvastools::verifyArgs(xPolyPolygon, viewState, renderState, strokeAttributes,
                          __func__);

        vclcanvastools::LocalGuard aGuard( m_aMutex );

        ENSURE_ARG_OR_THROW( xPolyPolygon.count(),
                         "polygon is NULL");

        vclcanvastools::OutDevStateKeeper aStateKeeper( *mxOutDev );

        ::basegfx::B2DHomMatrix aMatrix;
        ::canvastools::mergeViewAndRenderTransform(aMatrix, viewState, renderState);

        ::basegfx::B2DPolyPolygon aPolyPoly( xPolyPolygon );

        // apply dashing, if any
        if( strokeAttributes.DashArray.hasElements() )
        {
            const std::vector<double> aDashArray(
                ::comphelper::sequenceToContainer< std::vector<double> >(strokeAttributes.DashArray) );

            ::basegfx::B2DPolyPolygon aDashedPolyPoly;

            for( sal_uInt32 i=0; i<aPolyPoly.count(); ++i )
            {
                // AW: new interface; You may also get gaps in the same run now
                basegfx::utils::applyLineDashing(aPolyPoly.getB2DPolygon(i), aDashArray, &aDashedPolyPoly);
                //aDashedPolyPoly.append(
                //    ::basegfx::utils::applyLineDashing( aPolyPoly.getB2DPolygon(i),
                //                                        aDashArray ) );
            }

            aPolyPoly = std::move(aDashedPolyPoly);
        }

        ::basegfx::B2DSize aLinePixelSize(strokeAttributes.StrokeWidth,
                                          strokeAttributes.StrokeWidth);
        aLinePixelSize *= aMatrix;
        ::basegfx::B2DPolyPolygon aStrokedPolyPoly;
        if( aLinePixelSize.getLength() < 1.42 )
        {
            // line width < 1.0 in device pixel, thus, output as a
            // simple hairline poly-polygon
            setupOutDevState( viewState, renderState, LINE_COLOR );

            aStrokedPolyPoly = std::move(aPolyPoly);
        }
        else
        {
            // render as a 'thick' line
            setupOutDevState( viewState, renderState, FILL_COLOR );

            for( sal_uInt32 i=0; i<aPolyPoly.count(); ++i )
            {
                double fMiterMinimumAngle;
                if (strokeAttributes.MiterLimit <= 1.0)
                {
                    fMiterMinimumAngle = M_PI_2;
                }
                else
                {
                    fMiterMinimumAngle = 2.0 * asin(1.0/strokeAttributes.MiterLimit);
                }

                // TODO(F2): Also use Cap settings from
                // StrokeAttributes, the
                // createAreaGeometryForLineStartEnd() method does not
                // seem to fit very well here

                // AW: New interface, will create bezier polygons now
                aStrokedPolyPoly.append(basegfx::utils::createAreaGeometry(
                    aPolyPoly.getB2DPolygon(i),
                    strokeAttributes.StrokeWidth*0.5,
                    b2DJoineFromJoin(strokeAttributes.JoinType),
                    unoCapeFromCap(strokeAttributes.StartCapType),
                    basegfx::deg2rad(12.5) /* default fMaxAllowedAngle*/ ,
                    0.4 /* default fMaxPartOfEdge*/ ,
                    fMiterMinimumAngle
                    ));
                //aStrokedPolyPoly.append(
                //    ::basegfx::utils::createAreaGeometryForPolygon( aPolyPoly.getB2DPolygon(i),
                //                                                    strokeAttributes.StrokeWidth*0.5,
                //                                                    b2DJoineFromJoin(strokeAttributes.JoinType) ) );
            }
        }

        // transform only _now_, all the StrokeAttributes are in
        // user coordinates.
        aStrokedPolyPoly.transform( aMatrix );

        // TODO(F2): When using alpha here, must handle that via
        // temporary surface or somesuch.

        // Note: the generated stroke poly-polygon is NOT free of
        // self-intersections. Therefore, if we would render it
        // via OutDev::DrawPolyPolygon(), on/off fill would
        // generate off areas on those self-intersections.
        for( sal_uInt32 i=0; i<aStrokedPolyPoly.count(); ++i )
        {
            const basegfx::B2DPolygon& polygon = aStrokedPolyPoly.getB2DPolygon( i );
            if( polygon.isClosed()) {
                mxOutDev->DrawPolygon( polygon );
            } else {
                mxOutDev->DrawPolyLine( polygon );
            }
        }
    }

    rtl::Reference< vclcanvas::CachedBitmap >
        Canvas::fillPolyPolygon(const basegfx::B2DPolyPolygon&                                  xPolyPolygon,
                         const ::vclcanvas::ViewState&                                          viewState,
                         const ::vclcanvas::RenderState&                                        renderState)
    {
        canvastools::verifyArgs(xPolyPolygon, viewState, renderState,
                          __func__);

        vclcanvastools::LocalGuard aGuard( m_aMutex );

        ENSURE_ARG_OR_THROW( xPolyPolygon.count(),
                         "polygon is NULL");

        vclcanvastools::OutDevStateKeeper aStateKeeper( *mxOutDev );

        const int nAlpha( setupOutDevState( viewState, renderState, FILL_COLOR ) );
        ::basegfx::B2DPolyPolygon aB2DPolyPoly(xPolyPolygon);
        aB2DPolyPoly.setClosed(true); // ensure closed poly, otherwise VCL does not fill
        const ::tools::PolyPolygon aPolyPoly( vclcanvastools::mapPolyPolygon(
                                         aB2DPolyPoly,
                                         viewState, renderState ) );
        if( nAlpha == 255 )
        {
            mxOutDev->DrawPolyPolygon( aPolyPoly );
        }
        else
        {
            const int nTransPercent( ((255 - nAlpha) * 100 + 128) / 255 );  // normal rounding, no truncation here
            mxOutDev->DrawTransparent( aPolyPoly, static_cast<sal_uInt16>(nTransPercent) );
        }

        // TODO(P1): Provide caching here.
        return rtl::Reference< vclcanvas::CachedBitmap >(nullptr);
    }

    rtl::Reference< vclcanvas::CanvasFont >
        Canvas::createFont( const css::rendering::FontRequest&                                     fontRequest,
                    FontEmphasisMark                                                       eMark,
                    const css::geometry::Matrix2D&                                         fontMatrix )
    {
        canvastools::verifyArgs(fontRequest,
                          // dummy, to keep argPos in sync
                          fontRequest,
                          fontMatrix,
                          __func__);

        vclcanvastools::LocalGuard aGuard( m_aMutex );

        // TODO(F2): font properties and font matrix
        return new CanvasFont(fontRequest, eMark, fontMatrix, *mxOutDev);
    }


    void
        Canvas::drawText(const css::rendering::StringContext&                                     text,
                 const rtl::Reference< vclcanvas::CanvasFont >&                xFont,
                 const ::vclcanvas::ViewState&                                         viewState,
                 const ::vclcanvas::RenderState&                                       renderState,
                 sal_Int8                                                                 textDirection)
    {
        canvastools::verifyArgs(xFont, viewState, renderState,
                          __func__);
        canvastools::verifyRange( textDirection,
                            css::rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
                            css::rendering::TextDirection::STRONG_RIGHT_TO_LEFT );

        vclcanvastools::LocalGuard aGuard( m_aMutex );

        ENSURE_ARG_OR_THROW( xFont.is(),
                         "font is NULL");

        vclcanvastools::OutDevStateKeeper aStateKeeper( *mxOutDev );

        ::Point aOutpos;
        if( !setupTextOutput( aOutpos, viewState, renderState, xFont ) )
            return; // no output necessary

        // change text direction and layout mode
        vcl::text::ComplexTextLayoutFlags nLayoutMode(vcl::text::ComplexTextLayoutFlags::Default);
        switch( textDirection )
        {
            case rendering::TextDirection::WEAK_LEFT_TO_RIGHT:
            case rendering::TextDirection::STRONG_LEFT_TO_RIGHT:
                nLayoutMode |= vcl::text::ComplexTextLayoutFlags::BiDiStrong;
                nLayoutMode |= vcl::text::ComplexTextLayoutFlags::TextOriginLeft;
                break;

            case rendering::TextDirection::WEAK_RIGHT_TO_LEFT:
                nLayoutMode |= vcl::text::ComplexTextLayoutFlags::BiDiRtl;
                [[fallthrough]];
            case rendering::TextDirection::STRONG_RIGHT_TO_LEFT:
                nLayoutMode |= vcl::text::ComplexTextLayoutFlags::BiDiRtl | vcl::text::ComplexTextLayoutFlags::BiDiStrong;
                nLayoutMode |= vcl::text::ComplexTextLayoutFlags::TextOriginRight;
                break;
        }

        // TODO(F2): alpha
        mxOutDev->SetLayoutMode( nLayoutMode );
        mxOutDev->DrawText( aOutpos,
                            text.Text,
                            ::canvastools::numeric_cast<sal_uInt16>(text.StartPosition),
                            ::canvastools::numeric_cast<sal_uInt16>(text.Length) );
    }


    void
        Canvas::drawTextLayout(const rtl::Reference< vclcanvas::TextLayout >&               xLayoutedText,
                        const ::vclcanvas::ViewState&                                       viewState,
                        const ::vclcanvas::RenderState&                                     renderState)
    {
        canvastools::verifyArgs(xLayoutedText, viewState, renderState,
                          __func__);

        vclcanvastools::LocalGuard aGuard( m_aMutex );

        ENSURE_ARG_OR_THROW( xLayoutedText.is(),
                         "layout is NULL");

        vclcanvastools::OutDevStateKeeper aStateKeeper( *mxOutDev );

        // TODO(T3): Race condition. We're taking the font
        // from xLayoutedText, and then calling draw() at it,
        // without exclusive access. Move setupTextOutput(),
        // e.g. to impltools?

        ::Point aOutpos;
        if( !setupTextOutput( aOutpos, viewState, renderState, xLayoutedText->getFont() ) )
            return; // no output necessary

        // TODO(F2): What about the offset scalings?
        // TODO(F2): alpha
        xLayoutedText->draw( *mxOutDev, aOutpos, viewState, renderState );
    }

    void
        Canvas::drawPolyPolygon(const basegfx::B2DPolyPolygon&                    xPolyPolygon,
                        const ::vclcanvas::ViewState&                             viewState,
                        const ::vclcanvas::RenderState&                           renderState)
    {
        canvastools::verifyArgs(xPolyPolygon, viewState, renderState,
                          __func__);

        vclcanvastools::LocalGuard aGuard( m_aMutex );

        ENSURE_ARG_OR_THROW( xPolyPolygon.count(),
                         "polygon is NULL");

        vclcanvastools::OutDevStateKeeper aStateKeeper( *mxOutDev );
        setupOutDevState( viewState, renderState, LINE_COLOR );

        const ::tools::PolyPolygon aPolyPoly( vclcanvastools::mapPolyPolygon( xPolyPolygon, viewState, renderState ) );

        if( xPolyPolygon.isClosed() )
        {
            mxOutDev->DrawPolyPolygon( aPolyPoly );
        }
        else
        {
            // mixed open/closed state. Cannot render open polygon
            // via DrawPolyPolygon(), since that implicitly
            // closed every polygon. OTOH, no need to distinguish
            // further and render closed polygons via
            // DrawPolygon(), and open ones via DrawPolyLine():
            // closed polygons will simply already contain the
            // closing segment.
            sal_uInt16 nSize( aPolyPoly.Count() );

            for( sal_uInt16 i=0; i<nSize; ++i )
            {
                mxOutDev->DrawPolyLine( aPolyPoly[i] );
            }
        }
    }


    css::geometry::IntegerSize2D Canvas::getSize(  )
    {
        vclcanvastools::LocalGuard aGuard( m_aMutex );

        return vcl::unotools::integerSize2DFromSize( mxOutDev->GetOutputSizePixel() );
    }

    int Canvas::setupOutDevState( const vclcanvas::ViewState&     viewState,
                                        const vclcanvas::RenderState&   renderState,
                                        ColorType                       eColorType ) const
    {
        ::canvastools::verifyInput( renderState,
                                      __func__,
                                      2,
                                      eColorType == IGNORE_COLOR ? 0 : 3 );

        OutputDevice& rOutDev( *mxOutDev );

        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( AntialiasingFlags::Enable );

        int nAlpha(255);

        // TODO(P2): Don't change clipping all the time, maintain current clip
        // state and change only when update is necessary
        ::canvastools::clipOutDev(viewState, renderState, rOutDev);

        Color aColor( COL_WHITE );

        if( renderState.DeviceColor.has_value() )
        {
            aColor = *renderState.DeviceColor;
        }

        // extract alpha, and make color opaque
        // afterwards. Otherwise, OutputDevice won't draw anything
        nAlpha = aColor.GetAlpha();
        aColor.SetAlpha(255);

        if( eColorType != IGNORE_COLOR )
        {
            switch( eColorType )
            {
                case LINE_COLOR:
                    rOutDev.SetLineColor( aColor );
                    rOutDev.SetFillColor();
                    break;

                case FILL_COLOR:
                    rOutDev.SetFillColor( aColor );
                    rOutDev.SetLineColor();
                    break;

                case TEXT_COLOR:
                    rOutDev.SetTextColor( aColor );
                    break;

                default:
                    ENSURE_OR_THROW( false,
                                     "Unexpected color type");
                    break;
            }
        }

        return nAlpha;
    }


    rtl::Reference< vclcanvas::CachedBitmap > Canvas::implDrawBitmap( const Bitmap& rBitmap,
                                                                    const vclcanvas::ViewState&                 viewState,
                                                                    const vclcanvas::RenderState&               renderState,
                                                                    bool                                        bModulateColors )
    {
        ::canvastools::verifyInput( renderState,
                                      __func__,
                                      4,
                                      bModulateColors ? 3 : 0 );

        vclcanvastools::OutDevStateKeeper aStateKeeper( *mxOutDev );
        setupOutDevState( viewState, renderState, IGNORE_COLOR );

        ::basegfx::B2DHomMatrix aMatrix;
        ::canvastools::mergeViewAndRenderTransform(aMatrix, viewState, renderState);

        ::basegfx::B2DPoint aOutputPos( 0.0, 0.0 );
        aOutputPos *= aMatrix;

        ::Bitmap aBmp( rBitmap );

        // TODO(F2): Implement modulation again for other color
        // channels (currently, works only for alpha). Note: this
        // is already implemented in transformBitmap()
        if( bModulateColors &&
            renderState.DeviceColor.has_value())
        {
            // optimize away the case where alpha modulation value
            // is 1.0 - we then simply switch off modulation at all
            bModulateColors = renderState.DeviceColor->GetAlpha() != 255;
        }

        // check whether we can render bitmap as-is: must not
        // modulate colors, matrix must either be the identity
        // transform (that's clear), _or_ contain only
        // translational components.
        if( !bModulateColors &&
            (aMatrix.isIdentity() ||
             (::basegfx::fTools::equalZero( aMatrix.get(0,1) ) &&
              ::basegfx::fTools::equalZero( aMatrix.get(1,0) ) &&
              ::rtl::math::approxEqual(aMatrix.get(0,0), 1.0) &&
              ::rtl::math::approxEqual(aMatrix.get(1,1), 1.0)) ) )
        {
            // optimized case: identity matrix, or only
            // translational components.
            mxOutDev->DrawBitmap( vcl::unotools::pointFromB2DPoint( aOutputPos ), aBmp );

            // Returning a cache object is not useful, the XBitmap
            // itself serves this purpose
            return rtl::Reference< vclcanvas::CachedBitmap >(nullptr);
        }
        else if( mxOutDev->HasFastDrawTransformedBitmap())
        {
            ::basegfx::B2DHomMatrix aSizeTransform;
            aSizeTransform.scale( aBmp.GetSizePixel().Width(), aBmp.GetSizePixel().Height() );
            aMatrix = aMatrix * aSizeTransform;
            const double fAlpha = bModulateColors ? renderState.DeviceColor->GetAlpha() : 1.0;

            mxOutDev->DrawTransformedBitmapEx( aMatrix, aBmp, fAlpha );
            return rtl::Reference< vclcanvas::CachedBitmap >(nullptr);
        }
        else
        {
            // Matrix contains non-trivial transformation (or
            // color modulation is requested), decompose to check
            // whether GraphicObject suffices
            ::basegfx::B2DVector aScale;
            double               nRotate;
            double               nShearX;
            aMatrix.decompose( aScale, aOutputPos, nRotate, nShearX );

            GraphicAttr             aGrfAttr;
            GraphicObjectSharedPtr  pGrfObj;

            ::Size aBmpSize( aBmp.GetSizePixel() );

            // setup alpha modulation
            if( bModulateColors )
            {
                // TODO(F1): Note that the GraphicManager has a
                // subtle difference in how it calculates the
                // resulting alpha value: it's using the inverse
                // alpha values (i.e. 'transparency'), and
                // calculates transOrig + transModulate, instead
                // of transOrig + transModulate -
                // transOrig*transModulate (which would be
                // equivalent to the origAlpha*modulateAlpha the
                // DX canvas performs)
                aGrfAttr.SetAlpha( renderState.DeviceColor->GetAlpha() );
            }

            if( ::basegfx::fTools::equalZero( nShearX ) )
            {
                // no shear, GraphicObject is enough (the
                // GraphicObject only supports scaling, rotation
                // and translation)

                // #i75339# don't apply mirror flags, having
                // negative size values is enough to make
                // GraphicObject flip the bitmap

                // The angle has to be mapped from radian to tenths of
                // degrees with the orientation reversed: [0,2Pi) ->
                // (3600,0].  Note that the original angle may have
                // values outside the [0,2Pi) interval.
                const double nAngleInTenthOfDegrees (3600.0 - basegfx::rad2deg<10>(nRotate));
                aGrfAttr.SetRotation( Degree10(::basegfx::fround(nAngleInTenthOfDegrees)) );

                pGrfObj = std::make_shared<GraphicObject>( aBmp );
            }
            else
            {
                // modify output position, to account for the fact
                // that transformBitmap() always normalizes its output
                // bitmap into the smallest enclosing box.
                ::basegfx::B2DRectangle aDestRect = ::canvastools::calcTransformedRectBounds(
                                                            ::basegfx::B2DRectangle(0,
                                                                                    0,
                                                                                    aBmpSize.Width(),
                                                                                    aBmpSize.Height()),
                                                            aMatrix );

                aOutputPos.setX( aDestRect.getMinX() );
                aOutputPos.setY( aDestRect.getMinY() );

                // complex transformation, use generic affine bitmap
                // transformation
                aBmp = vclcanvastools::transformBitmap( aBmp, aMatrix );

                pGrfObj = std::make_shared<GraphicObject>( aBmp );

                // clear scale values, generated bitmap already
                // contains scaling
                aScale.setX( 1.0 ); aScale.setY( 1.0 );

                // update bitmap size, bitmap has changed above.
                aBmpSize = aBmp.GetSizePixel();
            }

            // output GraphicObject
            const ::Point aPt( vcl::unotools::pointFromB2DPoint( aOutputPos ) );
            const ::Size  aSz( ::basegfx::fround<::tools::Long>( aScale.getX() * aBmpSize.Width() ),
                               ::basegfx::fround<::tools::Long>( aScale.getY() * aBmpSize.Height() ) );

            pGrfObj->Draw(*mxOutDev,
                          aPt,
                          aSz,
                          &aGrfAttr);

            // created GraphicObject, which possibly cached
            // display bitmap - return cache object, to retain
            // that information.
            return new CachedBitmap( std::move(pGrfObj),
                                  aPt,
                                  aSz,
                                  aGrfAttr,
                                  viewState,
                                  renderState,
                                  *this);
        }
    }

    bool Canvas::setupTextOutput( ::Point&                                        o_rOutPos,
                                        const vclcanvas::ViewState&                     viewState,
                                        const vclcanvas::RenderState&                   renderState,
                                        const rtl::Reference< vclcanvas::CanvasFont >& xFont   ) const
    {
        OutputDevice& rOutDev( *mxOutDev );

        setupOutDevState( viewState, renderState, TEXT_COLOR );

        ENSURE_ARG_OR_THROW( xFont,
                             "Font not compatible with this canvas" );

        vcl::Font aVCLFont = xFont->getVCLFont();

        Color aColor( COL_BLACK );

        if( renderState.DeviceColor.has_value() )
        {
            aColor = *renderState.DeviceColor;
        }

        // setup font color
        aVCLFont.SetColor( aColor );
        aVCLFont.SetFillColor( aColor );

        if( !vclcanvastools::setupFontTransform( o_rOutPos, aVCLFont, viewState, renderState, rOutDev ) )
            return false;

        rOutDev.SetFont( aVCLFont );

        return true;
    }

    rtl::Reference< ::canvas::ParametricPolyPolygon > Canvas::createParametricPolyPolygon( std::u16string_view GradientService, const std::vector<::Color>& colors, const ::cpo::uno::Sequence< double >& stops, double aspectRatio )
    {
        return canvas::ParametricPolyPolygon::create(
                                          GradientService,
                                          colors, stops, aspectRatio);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
