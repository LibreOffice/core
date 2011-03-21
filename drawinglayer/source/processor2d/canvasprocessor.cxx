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
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/processor2d/canvasprocessor.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <vcl/canvastools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <canvas/canvastools.hxx>
#include <svl/ctloptions.hxx>
#include <vcl/svapp.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <cppcanvas/basegfxfactory.hxx>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <cppcanvas/vclfactory.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <vclhelperbitmaptransform.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <basegfx/tuple/b2i64tuple.hxx>
#include <basegfx/range/b2irange.hxx>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/rendering/PanoseProportion.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/StrokeAttributes.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <vclhelperbufferdevice.hxx>
#include <drawinglayer/primitive2d/chartprimitive2d.hxx>
#include <helperchartrenderer.hxx>
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <helperwrongspellrenderer.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// AW: Adding the canvas example from THB here to extract stuff later
/*
        // TODO(Q3): share impCreateEmptyBitmapWithPattern() and other
        // helper methods with vclprocessor.cxx
        Bitmap impCreateEmptyBitmapWithPattern(Bitmap aSource, const Size& aTargetSizePixel)
        {
            Bitmap aRetval;
            BitmapReadAccess* pReadAccess = aSource.AcquireReadAccess();

            if(pReadAccess)
            {
                if(aSource.GetBitCount() <= 8)
                {
                    BitmapPalette aPalette(pReadAccess->GetPalette());
                    aRetval = Bitmap(aTargetSizePixel, aSource.GetBitCount(), &aPalette);
                }
                else
                {
                    aRetval = Bitmap(aTargetSizePixel, aSource.GetBitCount());
                }

                delete pReadAccess;
            }

            return aRetval;
        }

        Bitmap impModifyBitmap(const basegfx::BColorModifier& rModifier, const Bitmap& rSource)
        {
            Bitmap aRetval(rSource);

            switch(rModifier.getMode())
            {
                case basegfx::BCOLORMODIFYMODE_REPLACE :
                {
                    aRetval = impCreateEmptyBitmapWithPattern(aRetval, Size(1L, 1L));
                    aRetval.Erase(Color(rModifier.getBColor()));
                    break;
                }

                default : // BCOLORMODIFYMODE_INTERPOLATE, BCOLORMODIFYMODE_GRAY, BCOLORMODIFYMODE_BLACKANDWHITE
                {
                    BitmapWriteAccess* pContent = aRetval.AcquireWriteAccess();

                    if(pContent)
                    {
                        for(sal_uInt32 y(0L); y < (sal_uInt32)pContent->Height(); y++)
                        {
                            for(sal_uInt32 x(0L); x < (sal_uInt32)pContent->Width(); x++)
                            {
                                const Color aColor = pContent->GetPixel(y, x);
                                const basegfx::BColor aBColor(rModifier.getModifiedColor(aColor.getBColor()));
                                pContent->SetPixel(y, x, BitmapColor(Color(aBColor)));
                            }
                        }

                        delete pContent;
                    }

                    break;
                }
            }

            return aRetval;
        }

        Bitmap impModifyBitmap(const basegfx::BColorModifierStack& rBColorModifierStack, const Bitmap& rSource)
        {
            Bitmap aRetval(rSource);

            for(sal_uInt32 a(rBColorModifierStack.count()); a; )
            {
                const basegfx::BColorModifier& rModifier = rBColorModifierStack.getBColorModifier(--a);
                aRetval = impModifyBitmap(rModifier, aRetval);
            }

            return aRetval;
        }

        sal_uInt32 impCalcGradientSteps(sal_uInt32 nSteps, const basegfx::B2DRange& rRange, sal_uInt32 nMaxDist)
        {
            if(nSteps == 0L)
                nSteps = (sal_uInt32)(rRange.getWidth() + rRange.getHeight()) / 8;

            if(nSteps < 2L)
            {
                nSteps = 2L;
            }

            if(nSteps > nMaxDist)
            {
                nSteps = nMaxDist;
            }

            return nSteps;
        }

        void canvasProcessor::impDrawGradientSimple(
            const basegfx::B2DPolyPolygon& rTargetForm,
            const ::std::vector< basegfx::B2DHomMatrix >& rMatrices,
            const ::std::vector< basegfx::BColor >& rColors,
            const basegfx::B2DPolygon& rUnitPolygon)
        {
            uno::Reference< rendering::XPolyPolygon2D > xPoly(
                basegfx::unotools::xPolyPolygonFromB2DPolygon(
                    mxCanvas->getDevice(),
                    rUnitPolygon));
            uno::Reference< rendering::XPolyPolygon2D > xTargetPoly(
                basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    mxCanvas->getDevice(),
                    rTargetForm));

            for(sal_uInt32 a(0L); a < rColors.size(); a++)
            {
                // set correct color
                const basegfx::BColor aFillColor(rColors[a]);

                maRenderState.DeviceColor = basegfx::unotools::colorToDoubleSequence(
                    mxCanvas->getDevice(),
                    aFillColor);

                if(a)
                {
                    if(a - 1L < rMatrices.size())
                    {
                        canvas::tools::setRenderStateTransform( maRenderState,
                                                                rMatrices[a - 1L] );
                        mxCanvas->fillPolyPolygon(xPoly,maViewState,maRenderState);
                    }
                }
                else
                {
                    canvas::tools::setRenderStateTransform( maRenderState,
                                                            basegfx::B2DHomMatrix() );
                    mxCanvas->fillPolyPolygon(xTargetPoly,maViewState,maRenderState);
                }
            }
        }

        void canvasProcessor::impDrawGradientComplex(
            const basegfx::B2DPolyPolygon& rTargetForm,
            const ::std::vector< basegfx::B2DHomMatrix >& rMatrices,
            const ::std::vector< basegfx::BColor >& rColors,
            const basegfx::B2DPolygon& rUnitPolygon)
        {
            uno::Reference< rendering::XPolyPolygon2D > xPoly(
                basegfx::unotools::xPolyPolygonFromB2DPolygon(
                    mxCanvas->getDevice(),
                    rUnitPolygon));
            uno::Reference< rendering::XPolyPolygon2D > xTargetPoly(
                basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    mxCanvas->getDevice(),
                    rTargetForm));

            maRenderState.Clip = xTargetPoly;

            // draw gradient PolyPolygons
            for(std::size_t a = 0L; a < rMatrices.size(); a++)
            {
                // set correct color
                if(rColors.size() > a)
                {
                    const basegfx::BColor aFillColor(rColors[a]);

                    maRenderState.DeviceColor = basegfx::unotools::colorToDoubleSequence(
                        mxCanvas->getDevice(),
                        aFillColor);
                }

                canvas::tools::setRenderStateTransform( maRenderState,
                                                        rMatrices[a] );

                if(a)
                    mxCanvas->fillPolyPolygon(xPoly,maViewState,maRenderState);
                else
                    mxCanvas->fillPolyPolygon(xTargetPoly,maViewState,maRenderState);
            }

            maRenderState.Clip.clear();
        }

        void canvasProcessor::impDrawGradient(
            const basegfx::B2DPolyPolygon& rTargetForm,
            ::drawinglayer::primitive::GradientStyle eGradientStyle,
            sal_uInt32 nSteps,
            const basegfx::BColor& rStart,
            const basegfx::BColor& rEnd,
            double fBorder, double fAngle, double fOffsetX, double fOffsetY, bool bSimple)
        {
            fprintf(stderr,"impDrawGradient\n");

            basegfx::B2DPolyPolygon aTmp(rTargetForm);
            aTmp.transform( maWorldToView );
            const basegfx::B2DRange aOutlineRangePixel(basegfx::tools::getRange(aTmp));
            const basegfx::B2DRange aOutlineRange(basegfx::tools::getRange(rTargetForm));

            fprintf(stderr,"impDrawGradient: #%d\n",nSteps);

            if( // step count is infinite, can use native canvas
                // gradients here
                nSteps == 0 ||
                // step count is sufficiently high, such that no
                // discernible difference should be visible.
                nSteps > 64 )
            {
                uno::Reference< rendering::XParametricPolyPolygon2DFactory > xFactory(
                    mxCanvas->getDevice()->getParametricPolyPolygonFactory() );

                if( xFactory.is() )
                {
                    fprintf(stderr,"native gradient #1\n");

                    basegfx::B2DHomMatrix aTextureTransformation;
                    rendering::Texture      aTexture;

                    aTexture.RepeatModeX = rendering::TexturingMode::CLAMP;
                    aTexture.RepeatModeY = rendering::TexturingMode::CLAMP;
                    aTexture.Alpha = 1.0;


                    // setup start/end color values
                    // ----------------------------

                    const uno::Sequence< double > aStartColor(
                        basegfx::unotools::colorToDoubleSequence( mxCanvas->getDevice(),
                                                                  rStart ));
                    const uno::Sequence< double > aEndColor(
                        basegfx::unotools::colorToDoubleSequence( mxCanvas->getDevice(),
                                                                  rEnd ));

                    // Setup texture transformation
                    // ----------------------------

                    const basegfx::B2DRange& rBounds(
                        basegfx::tools::getRange( rTargetForm ));

                    // setup rotation angle. VCL rotates
                    // counter-clockwise, while canvas transformation
                    // rotates clockwise
                    //fAngle = -fAngle;

                    switch(eGradientStyle)
                    {
                        case ::drawinglayer::primitive::GRADIENTSTYLE_LINEAR:
                            // FALLTHROUGH intended
                        case ::drawinglayer::primitive::GRADIENTSTYLE_AXIAL:
                        {
                            // standard orientation for VCL linear
                            // gradient is vertical, thus, rotate 90
                            // degrees
                            fAngle += M_PI/2.0;

                            // shrink texture, to account for border
                            // (only in x direction, linear gradient
                            // is constant in y direction, anyway)
                            aTextureTransformation.scale(
                                basegfx::pruneScaleValue(1.0 - fBorder),
                                1.0 );

                            double fBorderX(0.0);

                            // determine type of gradient (and necessary
                            // transformation matrix, should it be emulated by a
                            // generic gradient)
                            switch(eGradientStyle)
                            {
                                case ::drawinglayer::primitive::GRADIENTSTYLE_LINEAR:
                                    // linear gradients don't respect
                                    // offsets (they are implicitely
                                    // assumed to be 50%). linear
                                    // gradients don't have border on
                                    // both sides, only on the
                                    // startColor side. Gradient is
                                    // invariant in y direction: leave
                                    // y offset alone.
                                    fBorderX = fBorder;
                                    aTexture.Gradient = xFactory->createLinearHorizontalGradient( aStartColor,
                                                                                                  aEndColor );
                                    break;

                                case ::drawinglayer::primitive::GRADIENTSTYLE_AXIAL:
                                    // axial gradients have border on
                                    // both sides. Gradient is
                                    // invariant in y direction: leave
                                    // y offset alone.
                                    fBorderX = fBorder * .5;
                                    aTexture.Gradient = xFactory->createAxialHorizontalGradient( aStartColor,
                                                                                                 aEndColor );
                                    break;
                            }

                            // apply border offset values
                            aTextureTransformation.translate( fBorderX,
                                                              0.0 );

                            // rotate texture according to gradient rotation
                            aTextureTransformation.translate( -0.5, -0.5 );
                            aTextureTransformation.rotate( fAngle );

                            // to let the first strip of a rotated
                            // gradient start at the _edge_ of the
                            // bound rect (and not, due to rotation,
                            // slightly inside), slightly enlarge the
                            // gradient:
                            //
                            // y/2 sin(transparence) + x/2 cos(transparence)
                            //
                            // (values to change are not actual
                            // gradient scales, but original bound
                            // rect dimensions. Since we still want
                            // the border setting to apply after that,
                            // we multiply with that as above for
                            // nScaleX)
                            const double nScale(
                                basegfx::pruneScaleValue(
                                    fabs( rBounds.getHeight()*sin(fAngle) ) +
                                    fabs( rBounds.getWidth()*cos(fAngle) )));

                            aTextureTransformation.scale( nScale, nScale );

                            // translate back origin to center of
                            // primitive
                            aTextureTransformation.translate( 0.5*rBounds.getWidth(),
                                                              0.5*rBounds.getHeight() );
                            break;
                        }

                        case ::drawinglayer::primitive::GRADIENTSTYLE_RADIAL:
                            // FALLTHROUGH intended
                        case ::drawinglayer::primitive::GRADIENTSTYLE_ELLIPTICAL:
                            // FALLTHROUGH intended
                        case ::drawinglayer::primitive::GRADIENTSTYLE_SQUARE:
                            // FALLTHROUGH intended
                        case ::drawinglayer::primitive::GRADIENTSTYLE_RECT:
                        {
                            fprintf(stderr,"native gradient #2\n");

                            // determine scale factors for the gradient (must
                            // be scaled up from [0,1]x[0,1] rect to object
                            // bounds). Will potentially changed in switch
                            // statement below.
                            // Respect border value, while doing so, the VCL
                            // gradient's border will effectively shrink the
                            // resulting gradient.
                            double nScaleX( rBounds.getWidth() * (1.0 - fBorder) );
                            double nScaleY( rBounds.getHeight()* (1.0 - fBorder) );

                            // determine offset values. Since the
                            // border is divided half-by-half to both
                            // sides of the gradient, divide
                            // translation offset by an additional
                            // factor of 2. Also respect offset here,
                            // but since VCL gradients have their
                            // center at [0,0] for zero offset, but
                            // canvas gradients have their top, left
                            // edge aligned with the primitive, and
                            // offset of 50% effectively must yield
                            // zero shift. Both values will
                            // potentially be adapted in switch
                            // statement below.
                            double nOffsetX( rBounds.getWidth() *
                                             (2.0 * fOffsetX - 1.0 + fBorder)*.5 );
                            double nOffsetY( rBounds.getHeight() *
                                             (2.0 * fOffsetY - 1.0 + fBorder)*.5 );

                            // determine type of gradient (and necessary
                            // transformation matrix, should it be emulated by a
                            // generic gradient)
                            switch(eGradientStyle)
                            {
                                case ::drawinglayer::primitive::GRADIENTSTYLE_RADIAL:
                                {
                                    // create isotrophic scaling
                                    if( nScaleX > nScaleY )
                                    {
                                        nOffsetY -= (nScaleX - nScaleY) * 0.5;
                                        nScaleY = nScaleX;
                                    }
                                    else
                                    {
                                        nOffsetX -= (nScaleY - nScaleX) * 0.5;
                                        nScaleX = nScaleY;
                                    }

                                    // enlarge gradient to match bound rect diagonal
                                    aTextureTransformation.translate( -0.5, -0.5 );
                                    const double nScale( hypot(rBounds.getWidth(),
                                                               rBounds.getHeight()) / nScaleX );
                                    aTextureTransformation.scale( nScale, nScale );
                                    aTextureTransformation.translate( 0.5, 0.5 );

                                    aTexture.Gradient = xFactory->createEllipticalGradient(
                                        aEndColor,
                                        aStartColor,
                                        cssgeom::RealRectangle2D(0.0,0.0,
                                                                 1.0,1.0) );
                                }
                                break;

                                case ::drawinglayer::primitive::GRADIENTSTYLE_ELLIPTICAL:
                                {
                                    // enlarge gradient slightly
                                    aTextureTransformation.translate( -0.5, -0.5 );
                                    const double nSqrt2( sqrt(2.0) );
                                    aTextureTransformation.scale( nSqrt2,nSqrt2 );
                                    aTextureTransformation.translate( 0.5, 0.5 );

                                    aTexture.Gradient = xFactory->createEllipticalGradient(
                                        aEndColor,
                                        aStartColor,
                                        cssgeom::RealRectangle2D( rBounds.getMinX(),
                                                                  rBounds.getMinY(),
                                                                  rBounds.getMaxX(),
                                                                  rBounds.getMaxY() ));
                                }
                                break;

                                case ::drawinglayer::primitive::GRADIENTSTYLE_SQUARE:
                                {
                                    // create isotrophic scaling
                                    if( nScaleX > nScaleY )
                                    {
                                        nOffsetY -= (nScaleX - nScaleY) * 0.5;
                                        nScaleY = nScaleX;
                                    }
                                    else
                                    {
                                        nOffsetX -= (nScaleY - nScaleX) * 0.5;
                                        nScaleX = nScaleY;
                                    }

                                    aTexture.Gradient = xFactory->createRectangularGradient(
                                        aEndColor,
                                        aStartColor,
                                        cssgeom::RealRectangle2D(0.0,0.0,
                                                                 1.0,1.0));
                                }
                                break;

                                case ::drawinglayer::primitive::GRADIENTSTYLE_RECT:
                                {
                                    aTexture.Gradient = xFactory->createRectangularGradient(
                                        aEndColor,
                                        aStartColor,
                                        cssgeom::RealRectangle2D( rBounds.getMinX(),
                                                                  rBounds.getMinY(),
                                                                  rBounds.getMaxX(),
                                                                  rBounds.getMaxY() ));
                                }
                                break;
                            }

                            nScaleX = basegfx::pruneScaleValue( nScaleX );
                            nScaleY = basegfx::pruneScaleValue( nScaleY );

                            aTextureTransformation.scale( nScaleX, nScaleY );

                            // rotate texture according to gradient rotation
                            aTextureTransformation.translate( -0.5*nScaleX, -0.5*nScaleY );
                            aTextureTransformation.rotate( fAngle );
                            aTextureTransformation.translate( 0.5*nScaleX, 0.5*nScaleY );

                            aTextureTransformation.translate( nOffsetX, nOffsetY );
                        }
                        break;

                        default:
                            OSL_ENSURE( false,
                                        "canvasProcessor::impDrawGradient(): Unexpected gradient type" );
                            break;
                    }

                    // As the texture coordinate space is relative to
                    // the polygon coordinate space (NOT to the
                    // polygon itself), move gradient to the start of
                    // the actual polygon. If we skip this, the
                    // gradient will always display at the origin, and
                    // not within the polygon bound (which might be
                    // miles away from the origin).
                    aTextureTransformation.translate( rBounds.getMinX(),
                                                      rBounds.getMinY() );

                    basegfx::unotools::affineMatrixFromHomMatrix( aTexture.AffineTransform,
                                                                    aTextureTransformation );
                    uno::Sequence< rendering::Texture > aSeq(1);
                    aSeq[0] = aTexture;

                    mxCanvas->fillTexturedPolyPolygon(
                        basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                            mxCanvas->getDevice(),
                            rTargetForm),
                        maViewState,
                        maRenderState,
                        aSeq );

                    // done, using native gradients
                    return;
                }
            }
            else
            {
                // make sure steps is not too high/low
                nSteps = impCalcGradientSteps(nSteps,
                                              aOutlineRangePixel,
                                              sal_uInt32((rStart.getMaximumDistance(rEnd) * 127.5) + 0.5));


                ::std::vector< basegfx::B2DHomMatrix > aMatrices;
                ::std::vector< basegfx::BColor > aColors;
                basegfx::B2DPolygon aUnitPolygon;

                if( drawinglayer::primitive::GRADIENTSTYLE_RADIAL == eGradientStyle ||
                    drawinglayer::primitive::GRADIENTSTYLE_ELLIPTICAL == eGradientStyle)
                {
                    const basegfx::B2DPoint aCircleCenter(0.5, 0.5);
                    aUnitPolygon = basegfx::tools::createPolygonFromEllipse(aCircleCenter, 0.5, 0.5);
                    aUnitPolygon = basegfx::tools::adaptiveSubdivideByAngle(aUnitPolygon);
                }
                else
                {
                    aUnitPolygon = basegfx::tools::createUnitPolygon();
                }

                // create geometries
                switch(eGradientStyle)
                {
                    case ::drawinglayer::primitive::GRADIENTSTYLE_LINEAR:
                    {
                        ::drawinglayer::primitive::geoTexSvxGradientLinear aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fAngle);
                        aGradient.appendTransformations(aMatrices);
                        aGradient.appendColors(aColors);
                        break;
                    }
                    case ::drawinglayer::primitive::GRADIENTSTYLE_AXIAL:
                    {
                        ::drawinglayer::primitive::geoTexSvxGradientAxial aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fAngle);
                        aGradient.appendTransformations(aMatrices);
                        aGradient.appendColors(aColors);
                        break;
                    }
                    case ::drawinglayer::primitive::GRADIENTSTYLE_RADIAL:
                    {
                        ::drawinglayer::primitive::geoTexSvxGradientRadial aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetY);
                        aGradient.appendTransformations(aMatrices);
                        aGradient.appendColors(aColors);
                        break;
                    }
                    case ::drawinglayer::primitive::GRADIENTSTYLE_ELLIPTICAL:
                    {
                        ::drawinglayer::primitive::geoTexSvxGradientElliptical aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetX, fAngle);
                        aGradient.appendTransformations(aMatrices);
                        aGradient.appendColors(aColors);
                        break;
                    }
                    case ::drawinglayer::primitive::GRADIENTSTYLE_SQUARE:
                    {
                        ::drawinglayer::primitive::geoTexSvxGradientSquare aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetX, fAngle);
                        aGradient.appendTransformations(aMatrices);
                        aGradient.appendColors(aColors);
                        break;
                    }
                    case ::drawinglayer::primitive::GRADIENTSTYLE_RECT:
                    {
                        ::drawinglayer::primitive::geoTexSvxGradientRect aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetX, fAngle);
                        aGradient.appendTransformations(aMatrices);
                        aGradient.appendColors(aColors);
                        break;
                    }
                }

                // paint them with mask using the XOR method
                if(aMatrices.size())
                {
                    if(bSimple)
                    {
                        impDrawGradientSimple(rTargetForm, aMatrices, aColors, aUnitPolygon);
                    }
                    else
                    {
                        impDrawGradientComplex(rTargetForm, aMatrices, aColors, aUnitPolygon);
                    }
                }
            }
        }


        //////////////////////////////////////////////////////////////////////////////
        // rendering support

        // directdraw of text simple portion
        void canvasProcessor::impRender_STXP(const textSimplePortionPrimitive& rTextCandidate)
        {
            const fontAttributes&  rFontAttrs( rTextCandidate.getFontAttribute() );
            rendering::FontRequest aFontRequest;

            aFontRequest.FontDescription.FamilyName   = rFontAttrs.maFamilyName;
            aFontRequest.FontDescription.StyleName    = rFontAttrs.maStyleName;
            aFontRequest.FontDescription.IsSymbolFont = rFontAttrs.mbSymbol ? util::TriState_YES : util::TriState_NO;
            aFontRequest.FontDescription.IsVertical   = rFontAttrs.mbVertical ? util::TriState_YES : util::TriState_NO;

            // TODO(F2): improve vclenum->panose conversion
            aFontRequest.FontDescription.FontDescription.Weight =
                rFontAttrs.mnWeight;
            aFontRequest.FontDescription.FontDescription.Letterform =
                rFontAttrs.mbItalic ? 9 : 0;

            // font matrix should only be used for glyph rotations etc.
            css::geometry::Matrix2D aFontMatrix;
            canvas::tools::setIdentityMatrix2D( aFontMatrix );

            uno::Reference<rendering::XCanvasFont> xFont(
                mxCanvas->createFont( aFontRequest,
                                      uno::Sequence< beans::PropertyValue >(),
                                      aFontMatrix ));

            if( !xFont.is() )
                return;

            uno::Reference<rendering::XTextLayout> xLayout(
                xFont->createTextLayout(
                    rendering::StringContext( rTextCandidate.getText(),
                                              0,
                                              rTextCandidate.getText().Len() ),
                    // TODO(F3): Is this sufficient?
                    rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
                    0 ));
            if( !xLayout.is() )
                return;

            xLayout->applyLogicalAdvancements(
                uno::Sequence<double>(&rTextCandidate.getDXArray()[0],
                                      rTextCandidate.getDXArray().size() ));

            const basegfx::BColor aRGBColor(
                maBColorModifierStack.getModifiedColor(
                    rTextCandidate.getFontColor()));

            maRenderState.DeviceColor = basegfx::unotools::colorToDoubleSequence(
                mxCanvas->getDevice(),
                aRGBColor);

            // get render parameters and paint
            mxCanvas->drawTextLayout( xLayout,
                                      maViewState,
                                      maRenderState );
        }

        // direct draw of hairline
        void canvasProcessor::impRender_POHL(const polygonHairlinePrimitive& rPolygonCandidate)
        {
            const basegfx::BColor aRGBColor(
                maBColorModifierStack.getModifiedColor(
                    rPolygonCandidate.getBColor()));

            maRenderState.DeviceColor = basegfx::unotools::colorToDoubleSequence(
                mxCanvas->getDevice(),
                aRGBColor);

            mxCanvas->drawPolyPolygon( basegfx::unotools::xPolyPolygonFromB2DPolygon(
                                           mxCanvas->getDevice(),
                                           rPolygonCandidate.getB2DPolygon()),
                                       maViewState,
                                       maRenderState );
        }

        // direct draw of transformed BitmapEx primitive
        void canvasProcessor::impRender_BMPR(const bitmapPrimitive& rBitmapCandidate)
        {
            BitmapEx aBitmapEx(rBitmapCandidate.getBitmapEx());

            if(maBColorModifierStack.count())
            {
                // TODO(Q3): Share common bmp modification code with
                // vclprocessor.cxx
                Bitmap aChangedBitmap(impModifyBitmap(maBColorModifierStack, aBitmapEx.GetBitmap()));

                if(aBitmapEx.IsTransparent())
                {
                    if(aBitmapEx.IsAlpha())
                        aBitmapEx = BitmapEx(aChangedBitmap, aBitmapEx.GetAlpha());
                    else
                        aBitmapEx = BitmapEx(aChangedBitmap, aBitmapEx.GetMask());
                }
                else
                    aBitmapEx = BitmapEx(aChangedBitmap);
            }

            mxCanvas->drawBitmap(
                vcl::unotools::xBitmapFromBitmapEx( mxCanvas->getDevice(),
                                                    aBitmapEx ),
                maViewState,
                maRenderState);
        }

        void canvasProcessor::impRender_PPLB(const polyPolygonBitmapPrimitive& rPolyBitmapCandidate )
        {
            const fillBitmapAttribute& rFillBmpAttr( rPolyBitmapCandidate.getFillBitmap() );
            const basegfx::B2DPolyPolygon& rPoly( rPolyBitmapCandidate.getB2DPolyPolygon() );

            // TODO(Q3): Share common bmp modification code with
            // vclprocessor.cxx
            Bitmap aChangedBitmap(
                impModifyBitmap(maBColorModifierStack,
                                rFillBmpAttr.getBitmap()));

            rendering::Texture aTexture;
            const basegfx::B2DVector aBmpSize( rFillBmpAttr.getSize() );

            const basegfx::B2DRange& rBounds(
                basegfx::tools::getRange( rPoly ));

            basegfx::B2DHomMatrix aScale;
            aScale.scale( aBmpSize.getX() * rBounds.getWidth(),
                          aBmpSize.getY() * rBounds.getHeight() );

            basegfx::unotools::affineMatrixFromHomMatrix(
                aTexture.AffineTransform,
                aScale );

            aTexture.Alpha = 1.0;
            aTexture.Bitmap =
                ::vcl::unotools::xBitmapFromBitmapEx(
                    mxCanvas->getDevice(),
                    aChangedBitmap );
            aTexture.RepeatModeX = rendering::TexturingMode::REPEAT;
            aTexture.RepeatModeY = rendering::TexturingMode::REPEAT;

            uno::Sequence< rendering::Texture > aSeq(1);
            aSeq[0] = aTexture;

            mxCanvas->fillTexturedPolyPolygon(
                basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    mxCanvas->getDevice(),
                    rPoly),
                maViewState,
                maRenderState,
                aSeq );
        }

        // direct draw of gradient
        void canvasProcessor::impRender_PPLG(const polyPolygonGradientPrimitive& rPolygonCandidate)
        {
            const fillGradientAttribute& rGradient(rPolygonCandidate.getFillGradient());
            basegfx::BColor aStartColor(maBColorModifierStack.getModifiedColor(rGradient.getStartColor()));
            basegfx::BColor aEndColor(maBColorModifierStack.getModifiedColor(rGradient.getEndColor()));
            basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());

            if(aStartColor == aEndColor)
            {
                // no gradient at all, draw as polygon

                maRenderState.DeviceColor = basegfx::unotools::colorToDoubleSequence(
                    mxCanvas->getDevice(),
                    aStartColor);

                mxCanvas->drawPolyPolygon( basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                                               mxCanvas->getDevice(),
                                               aLocalPolyPolygon),
                                           maViewState,
                                           maRenderState );
            }
            else
            {
                // TODO(F3): if rGradient.getSteps() > 0, render
                // gradient manually!
                impDrawGradient(
                    aLocalPolyPolygon, rGradient.getStyle(), rGradient.getSteps(),
                    aStartColor, aEndColor, rGradient.getBorder(),
                    -rGradient.getAngle(), rGradient.getOffsetX(), rGradient.getOffsetY(), false);
            }
        }

        // direct draw of PolyPolygon with color
        void canvasProcessor::impRender_PPLC(const polyPolygonColorPrimitive& rPolygonCandidate)
        {
            const basegfx::BColor aRGBColor(
                maBColorModifierStack.getModifiedColor(
                    rPolygonCandidate.getBColor()));

            maRenderState.DeviceColor = basegfx::unotools::colorToDoubleSequence(
                mxCanvas->getDevice(),
                aRGBColor);

            mxCanvas->fillPolyPolygon( basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                                           mxCanvas->getDevice(),
                                           rPolygonCandidate.getB2DPolyPolygon()),
                                       maViewState,
                                       maRenderState );
        }

        // direct draw of MetaFile
        void canvasProcessor::impRender_META(const metafilePrimitive& rMetaCandidate)
        {
            // get metafile (copy it)
            GDIMetaFile aMetaFile;

            // TODO(Q3): Share common metafile modification code with
            // vclprocessor.cxx
            if(maBColorModifierStack.count())
            {
                const basegfx::BColor aRGBBaseColor(0, 0, 0);
                const basegfx::BColor aRGBColor(maBColorModifierStack.getModifiedColor(aRGBBaseColor));
                aMetaFile = rMetaCandidate.getMetaFile().GetMonochromeMtf(Color(aRGBColor));
            }
            else
            {
                aMetaFile = rMetaCandidate.getMetaFile();
            }

            cppcanvas::BitmapCanvasSharedPtr pCanvas(
                cppcanvas::VCLFactory::getInstance().createCanvas(
                    uno::Reference<rendering::XBitmapCanvas>(
                        mxCanvas,
                        uno::UNO_QUERY_THROW) ));
            cppcanvas::RendererSharedPtr pMtfRenderer(
                cppcanvas::VCLFactory::getInstance().createRenderer(
                    pCanvas,
                    aMetaFile,
                    cppcanvas::Renderer::Parameters() ));
            if( pMtfRenderer )
            {
                pCanvas->setTransformation(maWorldToView);
                pMtfRenderer->setTransformation(rMetaCandidate.getTransform());
                pMtfRenderer->draw();
            }
        }

        // mask group. Set mask polygon as clip
        void canvasProcessor::impRender_MASK(const maskPrimitive& rMaskCandidate)
        {
            const primitiveVector& rSubList = rMaskCandidate.getPrimitiveVector();

            if(!rSubList.empty())
            {
                // TODO(F3): cannot use state-global renderstate, when recursing!
                maRenderState.Clip =
                    basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        mxCanvas->getDevice(),
                        rMaskCandidate.getMask());

                // paint to it
                process(rSubList);

                maRenderState.Clip.clear();
            }
        }

        // modified color group. Force output to unified color.
        void canvasProcessor::impRender_MCOL(const modifiedColorPrimitive& rModifiedCandidate)
        {
            const primitiveVector& rSubList = rModifiedCandidate.getPrimitiveVector();

            if(!rSubList.empty())
            {
                maBColorModifierStack.push(rModifiedCandidate.getColorModifier());
                process(rModifiedCandidate.getPrimitiveVector());
                maBColorModifierStack.pop();
            }
        }

        // sub-transparence group. Draw to bitmap device first.
        void canvasProcessor::impRender_TRPR(const transparencePrimitive& rTransCandidate)
        {
            const primitiveVector& rSubList = rTransCandidate.getPrimitiveVector();

            if(!rSubList.empty())
            {
                basegfx::B2DRange aRange(
                    get2DRangeFromVector(rSubList,
                                         getViewInformation()));
                aRange.transform(maWorldToView);
                const basegfx::B2I64Tuple& rSize(
                    canvas::tools::spritePixelAreaFromB2DRange(aRange).getRange());
                uno::Reference< rendering::XCanvas > xBitmap(
                    mxCanvas->getDevice()->createCompatibleAlphaBitmap(
                        css::geometry::IntegerSize2D(rSize.getX(),
                                                     rSize.getY())),
                    uno::UNO_QUERY_THROW);

                //  remember last worldToView and add pixel offset
                basegfx::B2DHomMatrix aLastWorldToView(maWorldToView);
                basegfx::B2DHomMatrix aPixelOffset;
                aPixelOffset.translate(aRange.getMinX(),
                                       aRange.getMinY());
                setWorldToView(aPixelOffset * maWorldToView);

                // remember last canvas, set bitmap as target
                uno::Reference< rendering::XCanvas > xLastCanvas( mxCanvas );
                mxCanvas = xBitmap;

                // paint content to it
                process(rSubList);

                // TODO(F3): render transparent list to transparence
                // channel. Note that the OutDev implementation has a
                // shortcoming, in that nested transparency groups
                // don't work - transparence is not combined properly.

                // process(rTransCandidate.getTransparenceList());

                // back to old OutDev and worldToView
                mxCanvas = xLastCanvas;
                setWorldToView(aLastWorldToView);

                // DUMMY: add transparence modulation value to DeviceColor
                // TODO(F3): color management
                canvas::tools::setDeviceColor( maRenderState,
                                               1.0, 1.0, 1.0, 0.5 );
                // finally, draw bitmap
                mxCanvas->drawBitmapModulated(
                    uno::Reference< rendering::XBitmap >(
                        xBitmap,
                        uno::UNO_QUERY_THROW),
                    maViewState,
                    maRenderState );
            }
        }

        // transform group.
        void canvasProcessor::impRender_TRN2(const transformPrimitive& rTransformCandidate)
        {
            // remember current transformation
            basegfx::B2DHomMatrix aLastWorldToView(maWorldToView);

            // create new transformations
            setWorldToView(maWorldToView * rTransformCandidate.getTransformation());

            // let break down
            process(rTransformCandidate.getPrimitiveVector());

            // restore transformations
            setWorldToView(aLastWorldToView);
        }

        // marker
        void canvasProcessor::impRender_MARK(const markerPrimitive& rMarkCandidate)
        {
            const basegfx::BColor aRGBColor(maBColorModifierStack.getModifiedColor(rMarkCandidate.getRGBColor()));

            canvas::tools::initRenderState(maMarkerRenderState);
            maMarkerRenderState.DeviceColor = basegfx::unotools::colorToDoubleSequence(
                mxCanvas->getDevice(),
                aRGBColor);

            // Markers are special objects - their position is
            // determined by the view transformation, but their size
            // is always the same
            const basegfx::B2DPoint aViewPos(maWorldToView * rMarkCandidate.getPosition());

            uno::Reference< rendering::XPolyPolygon2D > xMarkerPoly;
            uno::Reference< rendering::XPolyPolygon2D > xHighlightMarkerPoly;
            switch(rMarkCandidate.getStyle())
            {
                default:
                case MARKERSTYLE_POINT:
                    mxCanvas->drawPoint( basegfx::unotools::point2DFromB2DPoint(aViewPos),
                                         maMarkerViewState,
                                         maMarkerRenderState );
                    return;

                case MARKERSTYLE_CROSS:
                    if( !mxCrossMarkerPoly.is() )
                    {
                        basegfx::B2DPolyPolygon aPoly;
                        basegfx::tools::importFromSvgD(
                            aPoly,
                            rtl::OUString::createFromAscii(
                                "m-1 0 h2 m0 -1 v2" ));
                        mxCrossMarkerPoly =
                            basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                                mxCanvas->getDevice(),
                                aPoly );
                    }
                    xMarkerPoly = mxCrossMarkerPoly;
                    break;

                case MARKERSTYLE_GLUEPOINT :
                    if( !mxGluePointPoly.is() )
                    {
                        basegfx::B2DPolyPolygon aPoly;
                        basegfx::tools::importFromSvgD(
                            aPoly,
                            rtl::OUString::createFromAscii(
                                "m-2 -3 l5 5 m-3 -2 l5 5 m-3 2 l5 -5 m-2 3 l5 -5" ));
                        mxGluePointPoly =
                            basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                                mxCanvas->getDevice(),
                                aPoly );
                    }
                    if( !mxGluePointHighlightPoly.is() )
                    {
                        basegfx::B2DPolyPolygon aPoly;
                        basegfx::tools::importFromSvgD(
                            aPoly,
                            rtl::OUString::createFromAscii(
                                "m-2 -2 l4 4 m-2 2 l4 -4" ));
                        mxGluePointHighlightPoly =
                            basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                                mxCanvas->getDevice(),
                                aPoly );
                    }
                    xMarkerPoly = mxGluePointPoly;
                    xHighlightMarkerPoly = mxGluePointHighlightPoly;
                    break;
            }

            basegfx::B2DRange aRange;
            rMarkCandidate.getRealtiveViewRange(aRange);
            const basegfx::B2DPoint aCenter(aRange.getCenter());

            basegfx::B2DHomMatrix aTranslate;
            aTranslate.translate(aViewPos.getX()+aCenter.getX(),
                                 aViewPos.getY()+aCenter.getY());

            canvas::tools::setRenderStateTransform( maMarkerRenderState,
                                                    aTranslate );


            mxCanvas->drawPolyPolygon( xMarkerPoly,
                                       maMarkerViewState,
                                       maMarkerRenderState );
            if( xHighlightMarkerPoly.is() )
            {
                // TODO(F3): color management
                canvas::tools::setDeviceColor(maMarkerRenderState,
                                              0.0, 0.0, 1.0, 1.0);
                mxCanvas->drawPolyPolygon( xMarkerPoly,
                                           maMarkerViewState,
                                           maMarkerRenderState );
            }
        }

        void canvasProcessor::setWorldToView(const basegfx::B2DHomMatrix& rMat)
        {
            maWorldToView = rMat;
            canvas::tools::setViewStateTransform(maViewState,
                                                 maWorldToView);
        }

        //////////////////////////////////////////////////////////////////////////////
        // internal processing support

        void canvasProcessor::process(const primitiveVector& rSource)
        {
            primitiveVector::const_iterator       aCurr = rSource.begin();
            const primitiveVector::const_iterator aEnd = rSource.end();
            while( aCurr != aEnd )
            {
                const referencedPrimitive& rCandidate = *aCurr;

                switch(rCandidate.getID())
                {
                    case CreatePrimitiveID('S', 'T', 'X', 'P'):
                    {
                        // directdraw of text simple portion
                        impRender_STXP(static_cast< const textSimplePortionPrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('P', 'O', 'H', 'L'):
                    {
                        // direct draw of hairline
                        impRender_POHL(static_cast< const polygonHairlinePrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('B', 'M', 'P', 'R'):
                    {
                        // direct draw of transformed BitmapEx primitive
                        impRender_BMPR(static_cast< const bitmapPrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('F', 'B', 'M', 'P'):
                    {
                        OSL_ENSURE(false,"fillBitmapPrimitive not yet implemented");
                        break;
                    }

                    case CreatePrimitiveID('P', 'P', 'L', 'B'):
                    {
                        // direct draw of polygon with bitmap fill
                        impRender_PPLB(static_cast< const polyPolygonBitmapPrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('P', 'P', 'L', 'G'):
                    {
                        // direct draw of gradient
                        impRender_PPLG(static_cast< const polyPolygonGradientPrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('P', 'P', 'L', 'C'):
                    {
                        // direct draw of PolyPolygon with color
                        impRender_PPLC(static_cast< const polyPolygonColorPrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('M', 'E', 'T', 'A'):
                    {
                        // direct draw of MetaFile
                        impRender_META(static_cast< const metafilePrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('M', 'A', 'S', 'K'):
                    {
                        // mask group. Force output to VDev and create mask from given mask
                        impRender_MASK(static_cast< const maskPrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('M', 'C', 'O', 'L'):
                    {
                        // modified color group. Force output to unified color.
                        impRender_MCOL(static_cast< const modifiedColorPrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('T', 'R', 'P', 'R'):
                    {
                        // sub-transparence group. Draw to VDev first.
                        impRender_TRPR(static_cast< const transparencePrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('T', 'R', 'N', '2'):
                    {
                        // transform group.
                        impRender_TRN2(static_cast< const transformPrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('M', 'A', 'R', 'K'):
                    {
                        // marker
                        impRender_MARK(static_cast< const markerPrimitive& >(rCandidate.getBasePrimitive()));
                        break;
                    }

                    case CreatePrimitiveID('A', 'N', 'S', 'W'):
                    case CreatePrimitiveID('A', 'N', 'B', 'L'):
                    case CreatePrimitiveID('A', 'N', 'I', 'N'):
                    {
                        // check timing, but do not accept
                        const animatedSwitchPrimitive& rAnimatedCandidate(static_cast< const animatedSwitchPrimitive& >(rCandidate.getBasePrimitive()));
                        const ::drawinglayer::animation::animationEntryList& rAnimationList = rAnimatedCandidate.getAnimationList();
                        const double fNewTime(rAnimationList.getNextEventTime(getViewInformation().getViewTime()));

                        // let break down
                        process(rAnimatedCandidate.getDecomposition(getViewInformation()));

                        break;
                    }

                    default:
                    {
                        // let break down
                        process(rCandidate.getBasePrimitive().getDecomposition(getViewInformation()));
                    }
                }

                ++aCurr;
            }
        }

        canvasProcessor::canvasProcessor( const ::drawinglayer::geometry::viewInformation& rViewInformation,
                                          const uno::Reference<rendering::XCanvas>&        rCanvas ) :
            processor(rViewInformation),
            mxCanvas( rCanvas ),
            mxCrossMarkerPoly(),
            mxGluePointPoly(),
            mxGluePointHighlightPoly(),
            maBColorModifierStack(),
            maWorldToView(),
            maViewState(),
            maRenderState(),
            maMarkerViewState(),
            maMarkerRenderState()
        {
            canvas::tools::initViewState(maViewState);
            canvas::tools::initRenderState(maRenderState);
            canvas::tools::initViewState(maMarkerViewState);
            canvas::tools::initRenderState(maMarkerRenderState);

            maWorldToView = maViewInformation.getViewTransformation();

            canvas::tools::setViewStateTransform(maViewState,
                                                 maWorldToView);
        }

        canvasProcessor::~canvasProcessor()
        {}
*/
//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        //////////////////////////////////////////////////////////////////////////////
        // single primitive renderers

        void canvasProcessor2D::impRenderMaskPrimitive2D(const primitive2d::MaskPrimitive2D& rMaskCandidate)
        {
            const primitive2d::Primitive2DSequence& rChildren = rMaskCandidate.getChildren();
            static bool bUseMaskBitmapMethod(true);

            if(rChildren.hasElements())
            {
                basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());

                if(!aMask.count())
                {
                    // no mask, no clipping. recursively paint content
                    process(rChildren);
                }
                else
                {
                    // there are principally two methods for implementing the mask primitive. One
                    // is to set a clip polygon at the canvas, the other is to create and use a
                    // transparence-using XBitmap for content and draw the mask as transparence. Both have their
                    // advantages and disadvantages, so here are both with a bool allowing simple
                    // change
                    if(bUseMaskBitmapMethod)
                    {
                        // get logic range of transparent part, clip with ViewRange
                        basegfx::B2DRange aLogicRange(aMask.getB2DRange());

                        if(!getViewInformation2D().getViewport().isEmpty())
                        {
                            aLogicRange.intersect(getViewInformation2D().getViewport());
                        }

                        if(!aLogicRange.isEmpty())
                        {
                            // get discrete range of transparent part
                            basegfx::B2DRange aDiscreteRange(aLogicRange);
                            aDiscreteRange.transform(getViewInformation2D().getObjectToViewTransformation());

                            // expand to next covering discrete values (pixel bounds)
                            aDiscreteRange.expand(basegfx::B2DTuple(floor(aDiscreteRange.getMinX()), floor(aDiscreteRange.getMinY())));
                            aDiscreteRange.expand(basegfx::B2DTuple(ceil(aDiscreteRange.getMaxX()), ceil(aDiscreteRange.getMaxY())));

                            // use VCL-based buffer device
                            impBufferDevice aBufferDevice(*mpOutputDevice, aDiscreteRange, false);

                            if(aBufferDevice.isVisible())
                            {
                                // remember current OutDev, Canvas and ViewInformation
                                OutputDevice* pLastOutputDevice = mpOutputDevice;
                                uno::Reference< rendering::XCanvas > xLastCanvas(mxCanvas);
                                const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

                                // prepare discrete offset for XBitmap, do not forget that the buffer bitmap
                                // may be truncated to discrete visible pixels
                                const basegfx::B2DHomMatrix aDiscreteOffset(basegfx::tools::createTranslateB2DHomMatrix(
                                    aDiscreteRange.getMinX() > 0.0 ? -aDiscreteRange.getMinX() : 0.0,
                                    aDiscreteRange.getMinY() > 0.0 ? -aDiscreteRange.getMinY() : 0.0));

                                // create new local ViewInformation2D with new transformation
                                const geometry::ViewInformation2D aViewInformation2D(
                                    getViewInformation2D().getObjectTransformation(),
                                    aDiscreteOffset * getViewInformation2D().getViewTransformation(),
                                    getViewInformation2D().getViewport(),
                                    getViewInformation2D().getVisualizedPage(),
                                    getViewInformation2D().getViewTime(),
                                    getViewInformation2D().getExtendedInformationSequence());
                                updateViewInformation(aViewInformation2D);

                                // set OutDev and Canvas to content target
                                mpOutputDevice = &aBufferDevice.getContent();
                                mxCanvas = mpOutputDevice->GetCanvas();
                                canvas::tools::setViewStateTransform(maViewState, getViewInformation2D().getViewTransformation());

                                // if ViewState transform is changed, the clipping polygon needs to be adapted, too
                                const basegfx::B2DPolyPolygon aOldClipPolyPolygon(maClipPolyPolygon);

                                if(maClipPolyPolygon.count())
                                {
                                    maClipPolyPolygon.transform(aDiscreteOffset);
                                    maViewState.Clip = basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mxCanvas->getDevice(), maClipPolyPolygon);
                                }

                                // paint content
                                process(rChildren);

                                // draw mask
                                const basegfx::BColor aBlack(0.0, 0.0, 0.0);
                                maRenderState.DeviceColor = aBlack.colorToDoubleSequence(mxCanvas->getDevice());

                                if(getOptionsDrawinglayer().IsAntiAliasing())
                                {
                                    // with AA, use 8bit AlphaMask to get nice borders
                                    VirtualDevice& rTransparence = aBufferDevice.getTransparence();
                                    rTransparence.GetCanvas()->fillPolyPolygon(
                                        basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mxCanvas->getDevice(), aMask),
                                        maViewState, maRenderState);
                                }
                                else
                                {
                                    // No AA, use 1bit mask
                                    VirtualDevice& rMask = aBufferDevice.getMask();
                                    rMask.GetCanvas()->fillPolyPolygon(
                                        basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mxCanvas->getDevice(), aMask),
                                        maViewState, maRenderState);
                                }

                                // back to old color stack, OutDev, Canvas and ViewTransform
                                mpOutputDevice = pLastOutputDevice;
                                mxCanvas = xLastCanvas;
                                updateViewInformation(aLastViewInformation2D);
                                canvas::tools::setViewStateTransform(maViewState, getViewInformation2D().getViewTransformation());

                                // restore clipping polygon
                                maClipPolyPolygon = aOldClipPolyPolygon;

                                if(maClipPolyPolygon.count())
                                {
                                    maViewState.Clip = basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mxCanvas->getDevice(), maClipPolyPolygon);
                                }

                                // dump buffer to outdev
                                aBufferDevice.paint();
                            }
                        }
                    }
                    else
                    {
                        // transform new mask polygon to view coordinates for processing. All masks
                        // are processed in view coordinates and clipped against each other evtl. to
                        // create multi-clips
                        aMask.transform(getViewInformation2D().getObjectTransformation());

                        // remember last current clip polygon
                        const basegfx::B2DPolyPolygon aLastClipPolyPolygon(maClipPolyPolygon);

                        if(maClipPolyPolygon.count())
                        {
                            // there is already a clip polygon set; build clipped union of
                            // current mask polygon and new one
                            maClipPolyPolygon = basegfx::tools::clipPolyPolygonOnPolyPolygon(aMask, maClipPolyPolygon, false, false);
                        }
                        else
                        {
                            // use mask directly
                            maClipPolyPolygon = aMask;
                        }

                        // set at ViewState
                        if(maClipPolyPolygon.count())
                        {
                            // set new as clip polygon
                            maViewState.Clip = basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mxCanvas->getDevice(), maClipPolyPolygon);
                        }
                        else
                        {
                            // empty, reset
                            maViewState.Clip.clear();
                        }

                        // paint content
                        process(rChildren);

                        // restore local current to rescued clip polygon
                        maClipPolyPolygon = aLastClipPolyPolygon;

                        // set at ViewState
                        if(maClipPolyPolygon.count())
                        {
                            // set new as clip polygon
                            maViewState.Clip = basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mxCanvas->getDevice(), maClipPolyPolygon);
                        }
                        else
                        {
                            // empty, reset
                            maViewState.Clip.clear();
                        }
                    }
                }
            }
        }

        void canvasProcessor2D::impRenderMetafilePrimitive2D(const primitive2d::MetafilePrimitive2D& rMetaCandidate)
        {
            GDIMetaFile aMetaFile;

            if(maBColorModifierStack.count())
            {
                const basegfx::BColor aRGBBaseColor(0, 0, 0);
                const basegfx::BColor aRGBColor(maBColorModifierStack.getModifiedColor(aRGBBaseColor));
                aMetaFile = rMetaCandidate.getMetaFile().GetMonochromeMtf(Color(aRGBColor));
            }
            else
            {
                aMetaFile = rMetaCandidate.getMetaFile();
            }

            cppcanvas::BitmapCanvasSharedPtr pCanvas(cppcanvas::VCLFactory::getInstance().createCanvas(
                    uno::Reference<rendering::XBitmapCanvas>(mxCanvas, uno::UNO_QUERY_THROW)));
            cppcanvas::RendererSharedPtr pMtfRenderer(cppcanvas::VCLFactory::getInstance().createRenderer(
                    pCanvas, aMetaFile, cppcanvas::Renderer::Parameters()));

            if(pMtfRenderer)
            {
                pCanvas->setTransformation(getViewInformation2D().getObjectToViewTransformation());
                pMtfRenderer->setTransformation(rMetaCandidate.getTransform());
                pMtfRenderer->draw();
            }
        }

        void canvasProcessor2D::impRenderTextSimplePortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate)
        {
            if(rTextCandidate.getTextLength())
            {
                double fShearX(0.0);
                {
                    const basegfx::B2DHomMatrix aLocalTransform(getViewInformation2D().getObjectToViewTransformation() * rTextCandidate.getTextTransform());
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate;
                    aLocalTransform.decompose(aScale, aTranslate, fRotate, fShearX);
                }

                if(!basegfx::fTools::equalZero(fShearX))
                {
                    // text is sheared. As long as the canvas renderers do not support this,
                    // use the decomposed primitive
                    process(rTextCandidate.get2DDecomposition(getViewInformation2D()));
                }
                else
                {
                    const attribute::FontAttribute& rFontAttr(rTextCandidate.getFontAttribute());
                    rendering::FontRequest aFontRequest;

                    aFontRequest.FontDescription.FamilyName = rFontAttr.getFamilyName();
                    aFontRequest.FontDescription.StyleName = rFontAttr.getStyleName();
                    aFontRequest.FontDescription.IsSymbolFont = rFontAttr.getSymbol() ? util::TriState_YES : util::TriState_NO;
                    aFontRequest.FontDescription.IsVertical = rFontAttr.getVertical() ? util::TriState_YES : util::TriState_NO;
                    // TODO(F2): improve vclenum->panose conversion
                    aFontRequest.FontDescription.FontDescription.Weight = static_cast< sal_uInt8 >(rFontAttr.getWeight());
                    aFontRequest.FontDescription.FontDescription.Proportion =
                        rFontAttr.getMonospaced()
                            ? rendering::PanoseProportion::MONO_SPACED
                            : rendering::PanoseProportion::ANYTHING;
                    aFontRequest.FontDescription.FontDescription.Letterform = rFontAttr.getItalic() ? 9 : 0;

                    // init CellSize to 1.0, else a default font height will be used
                    aFontRequest.CellSize = 1.0;
                    aFontRequest.Locale = rTextCandidate.getLocale();

                    // font matrix should only be used for glyph rotations etc.
                    com::sun::star::geometry::Matrix2D aFontMatrix;
                    canvas::tools::setIdentityMatrix2D(aFontMatrix);

                    uno::Reference<rendering::XCanvasFont> xFont(mxCanvas->createFont(
                        aFontRequest, uno::Sequence< beans::PropertyValue >(), aFontMatrix));

                    if(xFont.is())
                    {
                        // got a font, now try to get a TextLayout
                        const rendering::StringContext aStringContext(
                            rTextCandidate.getText(), rTextCandidate.getTextPosition(), rTextCandidate.getTextLength());
                        uno::Reference<rendering::XTextLayout> xLayout(xFont->createTextLayout(
                            aStringContext, com::sun::star::rendering::TextDirection::WEAK_LEFT_TO_RIGHT, 0));

                        if(xLayout.is())
                        {
                            // got a text layout, apply DXArray if given
                            const ::std::vector< double >& rDXArray = rTextCandidate.getDXArray();
                            const sal_uInt32 nDXCount(rDXArray.size());

                            if(nDXCount)
                            {
                                // DXArray does not need to be adapted to getTextPosition/getTextLength,
                                // it is already provided correctly
                                const uno::Sequence< double > aDXSequence(&rDXArray[0], nDXCount);
                                xLayout->applyLogicalAdvancements(aDXSequence);
                            }

                            // set text color
                            const basegfx::BColor aRGBColor(maBColorModifierStack.getModifiedColor(rTextCandidate.getFontColor()));
                            maRenderState.DeviceColor = aRGBColor.colorToDoubleSequence(mxCanvas->getDevice());

                            // set text transformation
                            canvas::tools::setRenderStateTransform(maRenderState,
                                getViewInformation2D().getObjectTransformation() * rTextCandidate.getTextTransform());

                            // paint
                            mxCanvas->drawTextLayout(xLayout, maViewState, maRenderState);
                        }
                    }
                }
            }
        }

        void canvasProcessor2D::impRenderBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate)
        {
            // apply possible color modification to BitmapEx
            BitmapEx aModifiedBitmapEx(impModifyBitmapEx(maBColorModifierStack, rBitmapCandidate.getBitmapEx()));

            if(aModifiedBitmapEx.IsEmpty())
            {
                // replace with color filled polygon
                const basegfx::BColor aModifiedColor(maBColorModifierStack.getModifiedColor(basegfx::BColor()));
                const basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());

                maRenderState.DeviceColor = aModifiedColor.colorToDoubleSequence(mxCanvas->getDevice());
                canvas::tools::setRenderStateTransform(maRenderState,
                    getViewInformation2D().getObjectTransformation() * rBitmapCandidate.getTransform());

                mxCanvas->fillPolyPolygon(basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    mxCanvas->getDevice(), basegfx::B2DPolyPolygon(aPolygon)), maViewState,  maRenderState);
            }
            else
            {
                // adapt object's transformation to the correct scale
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                const Size aSizePixel(aModifiedBitmapEx.GetSizePixel());

                if(0 != aSizePixel.Width() && 0 != aSizePixel.Height())
                {
                    rBitmapCandidate.getTransform().decompose(aScale, aTranslate, fRotate, fShearX);
                    const basegfx::B2DHomMatrix aNewMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                        aScale.getX() / aSizePixel.Width(), aScale.getY() / aSizePixel.Height(),
                        fShearX, fRotate, aTranslate.getX(), aTranslate.getY()));

                    canvas::tools::setRenderStateTransform(maRenderState,
                        getViewInformation2D().getObjectTransformation() * aNewMatrix);

                    mxCanvas->drawBitmap(
                        vcl::unotools::xBitmapFromBitmapEx(mxCanvas->getDevice(), aModifiedBitmapEx),
                        maViewState, maRenderState);
                }
            }
        }

        void canvasProcessor2D::impRenderTransparencePrimitive2D(const primitive2d::TransparencePrimitive2D& rTransparenceCandidate)
        {
            const primitive2d::Primitive2DSequence& rChildren = rTransparenceCandidate.getChildren();
            const primitive2d::Primitive2DSequence& rTransparence = rTransparenceCandidate.getTransparence();

            if(rChildren.hasElements() && rTransparence.hasElements())
            {
                // get logic range of transparent part and clip with ViewRange
                basegfx::B2DRange aLogicRange(primitive2d::getB2DRangeFromPrimitive2DSequence(rChildren, getViewInformation2D()));

                if(!getViewInformation2D().getViewport().isEmpty())
                {
                    aLogicRange.intersect(getViewInformation2D().getViewport());
                }

                if(!aLogicRange.isEmpty())
                {
                    // get discrete range of transparent part
                    basegfx::B2DRange aDiscreteRange(aLogicRange);
                    aDiscreteRange.transform(getViewInformation2D().getObjectToViewTransformation());

                    // expand to next covering discrete values (pixel bounds)
                    aDiscreteRange.expand(basegfx::B2DTuple(floor(aDiscreteRange.getMinX()), floor(aDiscreteRange.getMinY())));
                    aDiscreteRange.expand(basegfx::B2DTuple(ceil(aDiscreteRange.getMaxX()), ceil(aDiscreteRange.getMaxY())));

                    // use VCL-based buffer device
                    impBufferDevice aBufferDevice(*mpOutputDevice, aDiscreteRange, false);

                    if(aBufferDevice.isVisible())
                    {
                        // remember current OutDev, Canvas and ViewInformation
                        OutputDevice* pLastOutputDevice = mpOutputDevice;
                        uno::Reference< rendering::XCanvas > xLastCanvas(mxCanvas);
                        const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

                        // prepare discrete offset for XBitmap, do not forget that the buffer bitmap
                        // may be truncated to discrete visible pixels
                        const basegfx::B2DHomMatrix aDiscreteOffset(basegfx::tools::createTranslateB2DHomMatrix(
                            aDiscreteRange.getMinX() > 0.0 ? -aDiscreteRange.getMinX() : 0.0,
                            aDiscreteRange.getMinY() > 0.0 ? -aDiscreteRange.getMinY() : 0.0));

                        // create new local ViewInformation2D with new transformation
                        const geometry::ViewInformation2D aViewInformation2D(
                            getViewInformation2D().getObjectTransformation(),
                            aDiscreteOffset * getViewInformation2D().getViewTransformation(),
                            getViewInformation2D().getViewport(),
                            getViewInformation2D().getVisualizedPage(),
                            getViewInformation2D().getViewTime(),
                            getViewInformation2D().getExtendedInformationSequence());
                        updateViewInformation(aViewInformation2D);

                        // set OutDev and Canvas to content target
                        mpOutputDevice = &aBufferDevice.getContent();
                        mxCanvas = mpOutputDevice->GetCanvas();
                        canvas::tools::setViewStateTransform(maViewState, getViewInformation2D().getViewTransformation());

                        // if ViewState transform is changed, the clipping polygon needs to be adapted, too
                        const basegfx::B2DPolyPolygon aOldClipPolyPolygon(maClipPolyPolygon);

                        if(maClipPolyPolygon.count())
                        {
                            maClipPolyPolygon.transform(aDiscreteOffset);
                            maViewState.Clip = basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mxCanvas->getDevice(), maClipPolyPolygon);
                        }

                        // paint content
                        process(rChildren);

                        // set to mask
                        mpOutputDevice = &aBufferDevice.getTransparence();
                        mxCanvas = mpOutputDevice->GetCanvas();
                        canvas::tools::setViewStateTransform(maViewState, getViewInformation2D().getViewTransformation());

                        // when painting transparence masks, reset the color stack
                        basegfx::BColorModifierStack aLastBColorModifierStack(maBColorModifierStack);
                        maBColorModifierStack = basegfx::BColorModifierStack();

                        // paint mask to it (always with transparence intensities, evtl. with AA)
                        process(rTransparence);

                        // back to old color stack, OutDev, Canvas and ViewTransform
                        maBColorModifierStack = aLastBColorModifierStack;
                        mpOutputDevice = pLastOutputDevice;
                        mxCanvas = xLastCanvas;
                        updateViewInformation(aLastViewInformation2D);
                        canvas::tools::setViewStateTransform(maViewState, getViewInformation2D().getViewTransformation());

                        // restore clipping polygon
                        maClipPolyPolygon = aOldClipPolyPolygon;

                        if(maClipPolyPolygon.count())
                        {
                            maViewState.Clip = basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mxCanvas->getDevice(), maClipPolyPolygon);
                        }

                        // dump buffer to outdev
                        aBufferDevice.paint();
                    }
                }
            }
        }

        void canvasProcessor2D::impRenderPolygonStrokePrimitive2D(const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokePrimitive)
        {
            // support direct fat line geometry. This moves the decomposition to the canvas.
            // As long as our canvases are used (which also use basegfx tooling) this makes
            // no difference, but potentially canvases may better support this
            static bool bSupportFatLineDirectly(true);
            bool bOutputDone(false);

            if(bSupportFatLineDirectly)
            {
                const attribute::LineAttribute& rLineAttribute = rPolygonStrokePrimitive.getLineAttribute();
                const attribute::StrokeAttribute& rStrokeAttribute = rPolygonStrokePrimitive.getStrokeAttribute();

                if(0.0 < rLineAttribute.getWidth() || 0 != rStrokeAttribute.getDotDashArray().size())
                {
                    rendering::StrokeAttributes aStrokeAttribute;

                    aStrokeAttribute.StrokeWidth = rLineAttribute.getWidth();
                    aStrokeAttribute.MiterLimit = 15.0; // degrees; maybe here (15.0 * F_PI180) is needed, not clear in the documentation
                    const ::std::vector< double >& rDotDashArray = rStrokeAttribute.getDotDashArray();

                    if(rDotDashArray.size())
                    {
                        aStrokeAttribute.DashArray = uno::Sequence< double >(&rDotDashArray[0], rDotDashArray.size());
                    }

                    switch(rLineAttribute.getLineJoin())
                    {
                        default: // B2DLINEJOIN_NONE, B2DLINEJOIN_MIDDLE
                            aStrokeAttribute.JoinType = rendering::PathJoinType::NONE;
                            break;
                        case basegfx::B2DLINEJOIN_BEVEL:
                            aStrokeAttribute.JoinType = rendering::PathJoinType::BEVEL;
                            break;
                        case basegfx::B2DLINEJOIN_MITER:
                            aStrokeAttribute.JoinType = rendering::PathJoinType::MITER;
                            break;
                        case basegfx::B2DLINEJOIN_ROUND:
                            aStrokeAttribute.JoinType = rendering::PathJoinType::ROUND;
                            break;
                    }

                    const basegfx::BColor aHairlineColor(maBColorModifierStack.getModifiedColor(rLineAttribute.getColor()));
                    maRenderState.DeviceColor = aHairlineColor.colorToDoubleSequence(mxCanvas->getDevice());
                    canvas::tools::setRenderStateTransform(maRenderState, getViewInformation2D().getObjectTransformation());

                    mxCanvas->strokePolyPolygon(
                        basegfx::unotools::xPolyPolygonFromB2DPolygon(mxCanvas->getDevice(), rPolygonStrokePrimitive.getB2DPolygon()),
                        maViewState,  maRenderState, aStrokeAttribute);

                    bOutputDone = true;
                }
            }

            if(!bOutputDone)
            {
                // process decomposition
                process(rPolygonStrokePrimitive.get2DDecomposition(getViewInformation2D()));
            }
        }

        void canvasProcessor2D::impRenderFillBitmapPrimitive2D(const primitive2d::FillBitmapPrimitive2D& rFillBitmapPrimitive2D)
        {
            // support tiled fills directly when tiling is on
            static bool bSupportFillBitmapDirectly(true);
            bool bOutputDone(false);

            if(bSupportFillBitmapDirectly)
            {
                const attribute::FillBitmapAttribute& rFillBitmapAttribute = rFillBitmapPrimitive2D.getFillBitmap();

                if(rFillBitmapAttribute.getTiling())
                {
                    // apply possible color modification to Bitmap
                    const BitmapEx aChangedBitmapEx(impModifyBitmapEx(maBColorModifierStack, rFillBitmapAttribute.getBitmapEx()));

                    if(aChangedBitmapEx.IsEmpty())
                    {
                        // replace with color filled polygon
                        const basegfx::BColor aModifiedColor(maBColorModifierStack.getModifiedColor(basegfx::BColor()));
                        const basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());

                        maRenderState.DeviceColor = aModifiedColor.colorToDoubleSequence(mxCanvas->getDevice());
                        canvas::tools::setRenderStateTransform(maRenderState,
                            getViewInformation2D().getObjectTransformation() * rFillBitmapPrimitive2D.getTransformation());

                        mxCanvas->fillPolyPolygon(basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                            mxCanvas->getDevice(), basegfx::B2DPolyPolygon(aPolygon)), maViewState,  maRenderState);
                    }
                    else
                    {
                        const Size aSizePixel(aChangedBitmapEx.GetSizePixel());

                        if(0 != aSizePixel.Width() && 0 != aSizePixel.Height())
                        {
                            // create texture matrix from texture to object (where object is unit square here),
                            // so use values directly
                            const basegfx::B2DHomMatrix aTextureMatrix(basegfx::tools::createScaleTranslateB2DHomMatrix(
                                rFillBitmapAttribute.getSize().getX(), rFillBitmapAttribute.getSize().getY(),
                                rFillBitmapAttribute.getTopLeft().getX(), rFillBitmapAttribute.getTopLeft().getY()));

                            // create and fill texture
                            rendering::Texture aTexture;

                            basegfx::unotools::affineMatrixFromHomMatrix(aTexture.AffineTransform, aTextureMatrix);
                            aTexture.Alpha = 1.0;
                            aTexture.Bitmap = vcl::unotools::xBitmapFromBitmapEx(mxCanvas->getDevice(), aChangedBitmapEx);
                            aTexture.RepeatModeX = rendering::TexturingMode::REPEAT;
                            aTexture.RepeatModeY = rendering::TexturingMode::REPEAT;

                            // canvas needs a polygon to fill, create unit rectangle polygon
                            const basegfx::B2DPolygon aOutlineRectangle(basegfx::tools::createUnitPolygon());

                            // set primitive's transformation as render state transform
                            canvas::tools::setRenderStateTransform(maRenderState,
                                getViewInformation2D().getObjectTransformation() * rFillBitmapPrimitive2D.getTransformation());

                            // put texture into a uno sequence for handover
                            uno::Sequence< rendering::Texture > aSeq(1);
                            aSeq[0] = aTexture;

                            // draw textured rectangle
                            mxCanvas->fillTexturedPolyPolygon(
                                basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mxCanvas->getDevice(), basegfx::B2DPolyPolygon(aOutlineRectangle)),
                                maViewState, maRenderState, aSeq);
                        }
                    }

                    bOutputDone = true;
                }
            }

            if(!bOutputDone)
            {
                // process decomposition
                process(rFillBitmapPrimitive2D.get2DDecomposition(getViewInformation2D()));
            }
        }

        void canvasProcessor2D::impRenderUnifiedTransparencePrimitive2D(const primitive2d::UnifiedTransparencePrimitive2D& rUniTransparenceCandidate)
        {
            if(0.0 == rUniTransparenceCandidate.getTransparence())
            {
                // not transparent at all, directly use content
                process(rUniTransparenceCandidate.getChildren());
            }
            else if(rUniTransparenceCandidate.getTransparence() > 0.0 && rUniTransparenceCandidate.getTransparence() < 1.0)
            {
                const primitive2d::Primitive2DSequence rChildren = rUniTransparenceCandidate.getChildren();

                if(rChildren.hasElements())
                {
                    bool bOutputDone(false);

                    // Detect if a single PolyPolygonColorPrimitive2D is contained; in that case,
                    // use the fillPolyPolygon method with correctly set transparence. This is a often used
                    // case, so detectiong it is valuable
                    if(1 == rChildren.getLength())
                    {
                        const primitive2d::Primitive2DReference xReference(rChildren[0]);
                        const primitive2d::PolyPolygonColorPrimitive2D* pPoPoColor = dynamic_cast< const primitive2d::PolyPolygonColorPrimitive2D* >(xReference.get());

                        if(pPoPoColor && PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D == pPoPoColor->getPrimitive2DID())
                        {
                            // direct draw of PolyPolygon with color and transparence
                            const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(pPoPoColor->getBColor()));

                            // add transparence modulation value to DeviceColor
                            uno::Sequence< double > aColor(4);

                            aColor[0] = aPolygonColor.getRed();
                            aColor[1] = aPolygonColor.getGreen();
                            aColor[2] = aPolygonColor.getBlue();
                            aColor[3] = 1.0 - rUniTransparenceCandidate.getTransparence();
                            maRenderState.DeviceColor = aColor;

                            canvas::tools::setRenderStateTransform(maRenderState, getViewInformation2D().getObjectTransformation());
                            mxCanvas->fillPolyPolygon(
                                basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mxCanvas->getDevice(), pPoPoColor->getB2DPolyPolygon()),
                                maViewState,  maRenderState);
                            bOutputDone = true;
                        }
                    }

                    if(!bOutputDone)
                    {
                        // process decomposition. This will be decomposed to an TransparencePrimitive2D
                        // with the same child context and a single polygon for transparent context. This could be
                        // directly handled here with known VCL-buffer technology, but would only
                        // make a small difference compared to directly rendering the TransparencePrimitive2D
                        // using impRenderTransparencePrimitive2D above.
                        process(rUniTransparenceCandidate.get2DDecomposition(getViewInformation2D()));
                    }
                }
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        // internal processing support

        void canvasProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    // direct draw of hairline
                    const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate = static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate);
                    const basegfx::BColor aHairlineColor(maBColorModifierStack.getModifiedColor(rPolygonCandidate.getBColor()));

                    maRenderState.DeviceColor = aHairlineColor.colorToDoubleSequence(mxCanvas->getDevice());
                    canvas::tools::setRenderStateTransform(maRenderState, getViewInformation2D().getObjectTransformation());
                    mxCanvas->drawPolyPolygon(
                        basegfx::unotools::xPolyPolygonFromB2DPolygon(mxCanvas->getDevice(), rPolygonCandidate.getB2DPolygon()),
                        maViewState,  maRenderState);

                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    // direct draw of PolyPolygon with color
                       const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate = static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate);
                    const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rPolygonCandidate.getBColor()));

                    maRenderState.DeviceColor = aPolygonColor.colorToDoubleSequence(mxCanvas->getDevice());
                    canvas::tools::setRenderStateTransform(maRenderState, getViewInformation2D().getObjectTransformation());
                    mxCanvas->fillPolyPolygon(
                        basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mxCanvas->getDevice(), rPolygonCandidate.getB2DPolyPolygon()),
                        maViewState,  maRenderState);

                    break;
                }
                case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D :
                {
                    // modified color group. Force output to unified color.
                    const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate = static_cast< const primitive2d::ModifiedColorPrimitive2D& >(rCandidate);

                    if(rModifiedCandidate.getChildren().hasElements())
                    {
                        maBColorModifierStack.push(rModifiedCandidate.getColorModifier());
                        process(rModifiedCandidate.getChildren());
                        maBColorModifierStack.pop();
                    }

                    break;
                }
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    // mask group
                    impRenderMaskPrimitive2D(static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate));

                    break;
                }
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    // transform group. Remember current ViewInformation2D
                    const primitive2d::TransformPrimitive2D& rTransformCandidate = static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate);
                    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

                    // create new local ViewInformation2D with new transformation
                    const geometry::ViewInformation2D aViewInformation2D(
                        getViewInformation2D().getObjectTransformation()  * rTransformCandidate.getTransformation(),
                        getViewInformation2D().getViewTransformation(),
                        getViewInformation2D().getViewport(),
                        getViewInformation2D().getVisualizedPage(),
                        getViewInformation2D().getViewTime(),
                        getViewInformation2D().getExtendedInformationSequence());
                    updateViewInformation(aViewInformation2D);

                    // set at canvas
                    canvas::tools::setRenderStateTransform(maRenderState, getViewInformation2D().getObjectTransformation());

                    // proccess content
                    process(rTransformCandidate.getChildren());

                    // restore transformations
                    updateViewInformation(aLastViewInformation2D);

                    // restore at canvas
                    canvas::tools::setRenderStateTransform(maRenderState, getViewInformation2D().getObjectTransformation());

                    break;
                }
                case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D :
                {
                    // new XDrawPage for ViewInformation2D
                    const primitive2d::PagePreviewPrimitive2D& rPagePreviewCandidate = static_cast< const primitive2d::PagePreviewPrimitive2D& >(rCandidate);

                    // remember current transformation and ViewInformation
                    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

                    // create new local ViewInformation2D
                    const geometry::ViewInformation2D aViewInformation2D(
                        getViewInformation2D().getObjectTransformation(),
                        getViewInformation2D().getViewTransformation(),
                        getViewInformation2D().getViewport(),
                        rPagePreviewCandidate.getXDrawPage(),
                        getViewInformation2D().getViewTime(),
                        getViewInformation2D().getExtendedInformationSequence());
                    updateViewInformation(aViewInformation2D);

                    // proccess decomposed content
                    process(rPagePreviewCandidate.get2DDecomposition(getViewInformation2D()));

                    // restore transformations
                    updateViewInformation(aLastViewInformation2D);
                    break;
                }
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                {
                    // MetaFile primitive
                    impRenderMetafilePrimitive2D(static_cast< const primitive2d::MetafilePrimitive2D& >(rCandidate));

                    break;
                }
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                {
                    // PointArray primitive
                    const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate = static_cast< const primitive2d::PointArrayPrimitive2D& >(rCandidate);

                    // set point color
                    const basegfx::BColor aRGBColor(maBColorModifierStack.getModifiedColor(rPointArrayCandidate.getRGBColor()));
                    maRenderState.DeviceColor = aRGBColor.colorToDoubleSequence(mxCanvas->getDevice());
                    canvas::tools::setRenderStateTransform(maRenderState, getViewInformation2D().getObjectTransformation());

                    const std::vector< basegfx::B2DPoint >& rPointVector = rPointArrayCandidate.getPositions();
                    const sal_uInt32 nPointCount(rPointVector.size());

                    for(sal_uInt32 a(0); a < nPointCount; a++)
                    {
                        const basegfx::B2DPoint& rPoint = rPointVector[a];
                        mxCanvas->drawPoint(basegfx::unotools::point2DFromB2DPoint(rPoint), maViewState, maRenderState);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                {
                    // TextSimplePortion primitive
                    impRenderTextSimplePortionPrimitive2D(static_cast< const primitive2d::TextSimplePortionPrimitive2D& >(rCandidate));

                    break;
                }
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                {
                    // Bitmap primitive
                    impRenderBitmapPrimitive2D(static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate));

                    break;
                }
                case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D :
                {
                    // Transparence primitive
                    impRenderTransparencePrimitive2D(static_cast< const primitive2d::TransparencePrimitive2D& >(rCandidate));

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
                {
                    // PolygonStrokePrimitive
                    impRenderPolygonStrokePrimitive2D(static_cast< const primitive2d::PolygonStrokePrimitive2D& >(rCandidate));

                    break;
                }
                case PRIMITIVE2D_ID_FILLBITMAPPRIMITIVE2D :
                {
                    // FillBitmapPrimitive2D
                    impRenderFillBitmapPrimitive2D(static_cast< const primitive2d::FillBitmapPrimitive2D& >(rCandidate));

                    break;
                }
                case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D :
                {
                    // UnifiedTransparencePrimitive2D
                    impRenderUnifiedTransparencePrimitive2D(static_cast< const primitive2d::UnifiedTransparencePrimitive2D& >(rCandidate));

                    break;
                }
                case PRIMITIVE2D_ID_CHARTPRIMITIVE2D :
                {
                    // chart primitive in canvas renderer; restore original DrawMode during call
                    // since the evtl. used ChartPrettyPainter will use the MapMode
                    const primitive2d::ChartPrimitive2D& rChartPrimitive = static_cast< const primitive2d::ChartPrimitive2D& >(rCandidate);
                       mpOutputDevice->Push(PUSH_MAPMODE);
                    mpOutputDevice->SetMapMode(maOriginalMapMode);

                    if(!renderChartPrimitive2D(
                        rChartPrimitive,
                        *mpOutputDevice,
                        getViewInformation2D()))
                    {
                        // fallback to decomposition (MetaFile)
                        process(rChartPrimitive.get2DDecomposition(getViewInformation2D()));
                    }

                    mpOutputDevice->Pop();
                    break;
                }
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                {
                    // wrong spell primitive. Handled directly here using VCL since VCL has a nice and
                    // very direct waveline painting which is needed for this. If VCL is to be avoided,
                    // this can be removed anytime and the decomposition may be used
                    const primitive2d::WrongSpellPrimitive2D& rWrongSpellPrimitive = static_cast< const primitive2d::WrongSpellPrimitive2D& >(rCandidate);

                    if(!renderWrongSpellPrimitive2D(
                        rWrongSpellPrimitive,
                        *mpOutputDevice,
                        getViewInformation2D().getObjectToViewTransformation(),
                        maBColorModifierStack))
                    {
                        // fallback to decomposition (MetaFile)
                        process(rWrongSpellPrimitive.get2DDecomposition(getViewInformation2D()));
                    }

                    break;
                }

                // nice to have:
                //
                // case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D :
                // - support FormControls more direct eventually, not sure if this is needed
                //   with the canvas renderer. The decomposition provides a bitmap representation
                //   of the control which will work
                //

                default :
                {
                    // process recursively
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));

                    break;
                }
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        // process support

        canvasProcessor2D::canvasProcessor2D(
            const geometry::ViewInformation2D& rViewInformation,
            OutputDevice& rOutDev)
        :   BaseProcessor2D(rViewInformation),
            maOriginalMapMode(rOutDev.GetMapMode()),
            mpOutputDevice(&rOutDev),
            mxCanvas(rOutDev.GetCanvas()),
            maViewState(),
            maRenderState(),
            maBColorModifierStack(),
            maDrawinglayerOpt(),
            maClipPolyPolygon(),
            meLang(LANGUAGE_SYSTEM)
        {
            const SvtCTLOptions aSvtCTLOptions;

            canvas::tools::initViewState(maViewState);
            canvas::tools::initRenderState(maRenderState);
            canvas::tools::setViewStateTransform(maViewState, getViewInformation2D().getViewTransformation());

            // set digit language, derived from SvtCTLOptions to have the correct
            // number display for arabic/hindi numerals
            if(SvtCTLOptions::NUMERALS_HINDI == aSvtCTLOptions.GetCTLTextNumerals())
            {
                meLang = LANGUAGE_ARABIC_SAUDI_ARABIA;
            }
            else if(SvtCTLOptions::NUMERALS_ARABIC == aSvtCTLOptions.GetCTLTextNumerals())
            {
                meLang = LANGUAGE_ENGLISH;
            }
            else
            {
                meLang = (LanguageType)Application::GetSettings().GetLanguage();
            }

            rOutDev.SetDigitLanguage(meLang);

            // prepare output directly to pixels
               mpOutputDevice->Push(PUSH_MAPMODE);
            mpOutputDevice->SetMapMode();

            // react on AntiAliasing settings
            if(getOptionsDrawinglayer().IsAntiAliasing())
            {
                mpOutputDevice->SetAntialiasing(mpOutputDevice->GetAntialiasing() | ANTIALIASING_ENABLE_B2DDRAW);
            }
            else
            {
                mpOutputDevice->SetAntialiasing(mpOutputDevice->GetAntialiasing() & ~ANTIALIASING_ENABLE_B2DDRAW);
            }
        }

        canvasProcessor2D::~canvasProcessor2D()
        {
            // restore MapMode
               mpOutputDevice->Pop();

            // restore AntiAliasing
            mpOutputDevice->SetAntialiasing(mpOutputDevice->GetAntialiasing() & ~ANTIALIASING_ENABLE_B2DDRAW);
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
