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


#include <tools/diagnose_ex.h>
#include <canvas/canvastools.hxx>

#include <math.h>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/FontSlant.hpp>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/utils/lerp.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <cppcanvas/basegfxfactory.hxx>

#include <unoview.hxx>
#include <slideshowexceptions.hxx>
#include <smilfunctionparser.hxx>
#include <tools.hxx>

#include <limits>


using namespace ::com::sun::star;

namespace slideshow::internal
{
        namespace
        {
            class NamedValueComparator
            {
            public:
                explicit NamedValueComparator( const beans::NamedValue& rKey ) :
                    mrKey( rKey )
                {
                }

                bool operator()( const beans::NamedValue& rValue ) const
                {
                    return rValue.Name == mrKey.Name && rValue.Value == mrKey.Value;
                }

            private:
                const beans::NamedValue&    mrKey;
            };

            ::basegfx::B2DHomMatrix getAttributedShapeTransformation( const ::basegfx::B2DRectangle&        rShapeBounds,
                                                                      const ShapeAttributeLayerSharedPtr&   pAttr )
            {
                ::basegfx::B2DHomMatrix     aTransform;
                const ::basegfx::B2DSize&   rSize( rShapeBounds.getRange() );

                const double nShearX( pAttr->isShearXAngleValid() ?
                                      pAttr->getShearXAngle() :
                                      0.0 );
                const double nShearY( pAttr->isShearYAngleValid() ?
                                      pAttr->getShearYAngle() :
                                      0.0 );
                const double nRotation( pAttr->isRotationAngleValid() ?
                                        basegfx::deg2rad(pAttr->getRotationAngle()) :
                                        0.0 );

                // scale, shear and rotation pivot point is the shape
                // center - adapt origin accordingly
                aTransform.translate( -0.5, -0.5 );

                // ensure valid size (zero size will inevitably lead
                // to a singular transformation matrix)
                aTransform.scale( ::basegfx::pruneScaleValue(
                                      rSize.getX() ),
                                  ::basegfx::pruneScaleValue(
                                      rSize.getY() ) );

                const bool bNeedShearX( !::basegfx::fTools::equalZero(nShearX) );
                const bool bNeedShearY( !::basegfx::fTools::equalZero(nShearY) );
                const bool bNeedRotation( !::basegfx::fTools::equalZero(nRotation) );

                if( bNeedRotation || bNeedShearX || bNeedShearY )
                {
                    if( bNeedShearX )
                        aTransform.shearX( nShearX );

                    if( bNeedShearY )
                        aTransform.shearY( nShearY );

                    if( bNeedRotation )
                        aTransform.rotate( nRotation );
                }

                // move left, top corner back to position of the
                // shape. Since we've already translated the
                // center of the shape to the origin (the
                // translate( -0.5, -0.5 ) above), translate to
                // center of final shape position here.
                aTransform.translate( rShapeBounds.getCenterX(),
                                      rShapeBounds.getCenterY() );

                return aTransform;
            }
        }

        // Value extraction from Any
        // =========================

        /// extract unary double value from Any
        bool extractValue( double&                      o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        rShape,
                           const ::basegfx::B2DVector&  rSlideBounds )
        {
            // try to extract numeric value (double, or smaller POD, like float or int)
            if( rSourceAny >>= o_rValue)
            {
                // succeeded
                return true;
            }

            // try to extract string
            OUString aString;
            if( !(rSourceAny >>= aString) )
                return false; // nothing left to try

            // parse the string into an ExpressionNode
            try
            {
                // Parse string into ExpressionNode, eval node at time 0.0
                o_rValue = (*SmilFunctionParser::parseSmilValue(
                                aString,
                                calcRelativeShapeBounds(rSlideBounds,
                                                        rShape->getBounds()) ))(0.0);
            }
            catch( ParseError& )
            {
                return false;
            }

            return true;
        }

        /// extract enum/constant group value from Any
        bool extractValue( sal_Int32&                       o_rValue,
                           const uno::Any&                  rSourceAny,
                           const ShapeSharedPtr&            /*rShape*/,
                           const ::basegfx::B2DVector&      /*rSlideBounds*/ )
        {
            // try to extract numeric value (int, or smaller POD, like byte)
            if( rSourceAny >>= o_rValue)
            {
                // succeeded
                return true;
            }

            // okay, no plain int. Maybe one of the domain-specific enums?
            drawing::FillStyle eFillStyle;
            if( rSourceAny >>= eFillStyle )
            {
                o_rValue = sal::static_int_cast<sal_Int16>(eFillStyle);

                // succeeded
                return true;
            }

            drawing::LineStyle eLineStyle;
            if( rSourceAny >>= eLineStyle )
            {
                o_rValue = sal::static_int_cast<sal_Int16>(eLineStyle);

                // succeeded
                return true;
            }

            awt::FontSlant eFontSlant;
            if( rSourceAny >>= eFontSlant )
            {
                o_rValue = sal::static_int_cast<sal_Int16>(eFontSlant);

                // succeeded
                return true;
            }

            // nothing left to try. Failure
            return false;
        }

        /// extract enum/constant group value from Any
        bool extractValue( sal_Int16&                       o_rValue,
                           const uno::Any&                  rSourceAny,
                           const ShapeSharedPtr&            rShape,
                           const ::basegfx::B2DVector&      rSlideBounds )
        {
            sal_Int32 aValue;
            if( !extractValue(aValue,rSourceAny,rShape,rSlideBounds) )
                return false;

            if( std::numeric_limits<sal_Int16>::max() < aValue ||
                std::numeric_limits<sal_Int16>::min() > aValue )
            {
                return false;
            }

            o_rValue = static_cast<sal_Int16>(aValue);

            return true;
        }

        /// extract color value from Any
        bool extractValue( RGBColor&                    o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        /*rShape*/,
                           const ::basegfx::B2DVector&  /*rSlideBounds*/ )
        {
            // try to extract numeric value (double, or smaller POD, like float or int)
            {
                double nTmp = 0;
                if( rSourceAny >>= nTmp )
                {
                    sal_uInt32 aIntColor( static_cast< sal_uInt32 >(nTmp) );

                    // TODO(F2): Handle color values correctly, here
                    o_rValue = unoColor2RGBColor( aIntColor );

                    // succeeded
                    return true;
                }
            }

            // try double sequence
            {
                uno::Sequence< double > aTmp;
                if( rSourceAny >>= aTmp )
                {
                    ENSURE_OR_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for RGB color value" );

                    o_rValue = RGBColor( aTmp[0], aTmp[1], aTmp[2] );

                    // succeeded
                    return true;
                }
            }

            // try sal_Int32 sequence
            {
                uno::Sequence< sal_Int32 > aTmp;
                if( rSourceAny >>= aTmp )
                {
                    ENSURE_OR_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for RGB color value" );

                    // truncate to byte
                    o_rValue = RGBColor( ::cppcanvas::makeColor(
                                             static_cast<sal_uInt8>(aTmp[0]),
                                             static_cast<sal_uInt8>(aTmp[1]),
                                             static_cast<sal_uInt8>(aTmp[2]),
                                             255 ) );

                    // succeeded
                    return true;
                }
            }

            // try sal_Int8 sequence
            {
                uno::Sequence< sal_Int8 > aTmp;
                if( rSourceAny >>= aTmp )
                {
                    ENSURE_OR_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for RGB color value" );

                    o_rValue = RGBColor( ::cppcanvas::makeColor( aTmp[0], aTmp[1], aTmp[2], 255 ) );

                    // succeeded
                    return true;
                }
            }

            // try to extract string
            OUString aString;
            if( !(rSourceAny >>= aString) )
                return false; // nothing left to try

            // TODO(F2): Provide symbolic color values here
            o_rValue = RGBColor( 0.5, 0.5, 0.5 );

            return true;
        }

        /// extract color value from Any
        bool extractValue( HSLColor&                    o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        /*rShape*/,
                           const ::basegfx::B2DVector&  /*rSlideBounds*/ )
        {
            // try double sequence
            {
                uno::Sequence< double > aTmp;
                if( rSourceAny >>= aTmp )
                {
                    ENSURE_OR_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for HSL color value" );

                    o_rValue = HSLColor( aTmp[0], aTmp[1], aTmp[2] );

                    // succeeded
                    return true;
                }
            }

            // try sal_Int8 sequence
            {
                uno::Sequence< sal_Int8 > aTmp;
                if( rSourceAny >>= aTmp )
                {
                    ENSURE_OR_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for HSL color value" );

                    o_rValue = HSLColor( aTmp[0]*360.0/255.0, aTmp[1]/255.0, aTmp[2]/255.0 );

                    // succeeded
                    return true;
                }
            }

            return false; // nothing left to try
        }

        /// extract plain string from Any
        bool extractValue( OUString&             o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        /*rShape*/,
                           const ::basegfx::B2DVector&  /*rSlideBounds*/ )
        {
            // try to extract string
            return rSourceAny >>= o_rValue;
        }

        /// extract bool value from Any
        bool extractValue( bool&                        o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        /*rShape*/,
                           const ::basegfx::B2DVector&  /*rSlideBounds*/ )
        {
            bool bTmp;
            // try to extract bool value
            if( rSourceAny >>= bTmp )
            {
                o_rValue = bTmp;

                // succeeded
                return true;
            }

            // try to extract string
            OUString aString;
            if( !(rSourceAny >>= aString) )
                return false; // nothing left to try

            // we also take the strings "true" and "false",
            // as well as "on" and "off" here
            if( aString.equalsIgnoreAsciiCase("true") ||
                aString.equalsIgnoreAsciiCase("on") )
            {
                o_rValue = true;
                return true;
            }
            if( aString.equalsIgnoreAsciiCase("false") ||
                aString.equalsIgnoreAsciiCase("off") )
            {
                o_rValue = false;
                return true;
            }

            // ultimately failed.
            return false;
        }

        /// extract double 2-tuple from Any
        bool extractValue( ::basegfx::B2DTuple&         o_rPair,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        rShape,
                           const ::basegfx::B2DVector&  rSlideBounds )
        {
            animations::ValuePair aPair;

            if( !(rSourceAny >>= aPair) )
                return false;

            double nFirst;
            if( !extractValue( nFirst, aPair.First, rShape, rSlideBounds ) )
                return false;

            double nSecond;
            if( !extractValue( nSecond, aPair.Second, rShape, rSlideBounds ) )
                return false;

            o_rPair.setX( nFirst );
            o_rPair.setY( nSecond );

            return true;
        }

        bool findNamedValue( uno::Sequence< beans::NamedValue > const& rSequence,
                             const beans::NamedValue&               rSearchKey )
        {
            return ::std::any_of( rSequence.begin(), rSequence.end(),
                                  NamedValueComparator( rSearchKey ) );
        }

        basegfx::B2DRange calcRelativeShapeBounds( const basegfx::B2DVector& rPageSize,
                                                   const basegfx::B2DRange&  rShapeBounds )
        {
            return basegfx::B2DRange( rShapeBounds.getMinX() / rPageSize.getX(),
                                      rShapeBounds.getMinY() / rPageSize.getY(),
                                      rShapeBounds.getMaxX() / rPageSize.getX(),
                                      rShapeBounds.getMaxY() / rPageSize.getY() );
        }

        // TODO(F2): Currently, the positional attributes DO NOT mirror the XShape properties.
        // First and foremost, this is because we must operate with the shape boundrect,
        // not position and size (the conversion between logic rect, snap rect and boundrect
        // are non-trivial for draw shapes, and I won't duplicate them here). Thus, shapes
        // rotated on the page will still have 0.0 rotation angle, as the metafile
        // representation fetched over the API is our default zero case.

        ::basegfx::B2DHomMatrix getShapeTransformation( const ::basegfx::B2DRectangle&      rShapeBounds,
                                                        const ShapeAttributeLayerSharedPtr& pAttr )
        {
            if( !pAttr )
            {
                const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(
                    rShapeBounds.getWidth(), rShapeBounds.getHeight(),
                    rShapeBounds.getMinX(), rShapeBounds.getMinY()));

                return aTransform;
            }
            else
            {
                return getAttributedShapeTransformation( rShapeBounds,
                                                         pAttr );
            }
        }

        ::basegfx::B2DHomMatrix getSpriteTransformation( const ::basegfx::B2DVector&            rPixelSize,
                                                         const ::basegfx::B2DVector&            rOrigSize,
                                                         const ShapeAttributeLayerSharedPtr&    pAttr )
        {
            ::basegfx::B2DHomMatrix aTransform;

            if( pAttr )
            {
                const double nShearX( pAttr->isShearXAngleValid() ?
                                      pAttr->getShearXAngle() :
                                      0.0 );
                const double nShearY( pAttr->isShearYAngleValid() ?
                                      pAttr->getShearYAngle() :
                                      0.0 );
                const double nRotation( pAttr->isRotationAngleValid() ?
                                        basegfx::deg2rad(pAttr->getRotationAngle()) :
                                        0.0 );

                // scale, shear and rotation pivot point is the
                // sprite's pixel center - adapt origin accordingly
                aTransform.translate( -0.5*rPixelSize.getX(),
                                      -0.5*rPixelSize.getY() );

                const ::basegfx::B2DSize aSize(
                    pAttr->isWidthValid() ? pAttr->getWidth() : rOrigSize.getX(),
                    pAttr->isHeightValid() ? pAttr->getHeight() : rOrigSize.getY() );

                // ensure valid size (zero size will inevitably lead
                // to a singular transformation matrix).
                aTransform.scale( ::basegfx::pruneScaleValue(
                                      aSize.getX() /
                                      ::basegfx::pruneScaleValue(
                                          rOrigSize.getX() ) ),
                                  ::basegfx::pruneScaleValue(
                                      aSize.getY() /
                                      ::basegfx::pruneScaleValue(
                                          rOrigSize.getY() ) ) );

                const bool bNeedShearX( !::basegfx::fTools::equalZero(nShearX) );
                const bool bNeedShearY( !::basegfx::fTools::equalZero(nShearY) );
                const bool bNeedRotation( !::basegfx::fTools::equalZero(nRotation) );

                if( bNeedRotation || bNeedShearX || bNeedShearY )
                {
                    if( bNeedShearX )
                        aTransform.shearX( nShearX );

                    if( bNeedShearY )
                        aTransform.shearY( nShearY );

                    if( bNeedRotation )
                        aTransform.rotate( nRotation );
                }

                // move left, top corner back to original position of
                // the sprite (we've translated the center of the
                // sprite to the origin above).
                aTransform.translate( 0.5*rPixelSize.getX(),
                                      0.5*rPixelSize.getY() );
            }

            // return identity transform for un-attributed
            // shapes. This renders the sprite as-is, in its
            // document-supplied size.
            return aTransform;
        }

        ::basegfx::B2DRectangle getShapeUpdateArea( const ::basegfx::B2DRectangle&      rUnitBounds,
                                                    const ::basegfx::B2DHomMatrix&      rShapeTransform,
                                                    const ShapeAttributeLayerSharedPtr& pAttr )
        {
            ::basegfx::B2DHomMatrix aTransform;

            if( pAttr &&
                pAttr->isCharScaleValid() &&
                fabs(pAttr->getCharScale()) > 1.0 )
            {
                // enlarge shape bounds. Have to consider the worst
                // case here (the text fully fills the shape)

                const double nCharScale( pAttr->getCharScale() );

                // center of scaling is the middle of the shape
                aTransform.translate( -0.5, -0.5 );
                aTransform.scale( nCharScale, nCharScale );
                aTransform.translate( 0.5, 0.5 );
            }

            aTransform *= rShapeTransform;

            ::basegfx::B2DRectangle aRes;

            // apply shape transformation to unit rect
            return ::canvas::tools::calcTransformedRectBounds(
                aRes,
                rUnitBounds,
                aTransform );
        }

        ::basegfx::B2DRange getShapeUpdateArea( const ::basegfx::B2DRange&      rUnitBounds,
                                                    const ::basegfx::B2DRange&      rShapeBounds )
        {
            return ::basegfx::B2DRectangle(
                basegfx::utils::lerp( rShapeBounds.getMinX(),
                                      rShapeBounds.getMaxX(),
                                      rUnitBounds.getMinX() ),
                basegfx::utils::lerp( rShapeBounds.getMinY(),
                                      rShapeBounds.getMaxY(),
                                      rUnitBounds.getMinY() ),
                basegfx::utils::lerp( rShapeBounds.getMinX(),
                                      rShapeBounds.getMaxX(),
                                      rUnitBounds.getMaxX() ),
                basegfx::utils::lerp( rShapeBounds.getMinY(),
                                      rShapeBounds.getMaxY(),
                                      rUnitBounds.getMaxY() ) );
        }

        ::basegfx::B2DRectangle getShapePosSize( const ::basegfx::B2DRectangle&         rOrigBounds,
                                                 const ShapeAttributeLayerSharedPtr&    pAttr )
        {
            // an already empty shape bound need no further
            // treatment. In fact, any changes applied below would
            // actually remove the special empty state, thus, don't
            // change!
            if( !pAttr ||
                rOrigBounds.isEmpty() )
            {
                return rOrigBounds;
            }
            else
            {
                // cannot use maBounds anymore, attributes might have been
                // changed by now.
                // Have to use absolute values here, as negative sizes
                // (aka mirrored shapes) _still_ have the same bounds,
                // only with mirrored content.
                ::basegfx::B2DSize aSize;
                aSize.setX( fabs( pAttr->isWidthValid() ?
                                  pAttr->getWidth() :
                                  rOrigBounds.getWidth() ) );
                aSize.setY( fabs( pAttr->isHeightValid() ?
                                  pAttr->getHeight() :
                                  rOrigBounds.getHeight() ) );

                ::basegfx::B2DPoint aPos;
                aPos.setX( pAttr->isPosXValid() ?
                           pAttr->getPosX() :
                           rOrigBounds.getCenterX() );
                aPos.setY( pAttr->isPosYValid() ?
                           pAttr->getPosY() :
                           rOrigBounds.getCenterY() );

                // the positional attribute retrieved from the
                // ShapeAttributeLayer actually denotes the _middle_
                // of the shape (do it as the PPTs do...)
                return ::basegfx::B2DRectangle( aPos - 0.5*aSize,
                                                aPos + 0.5*aSize );
            }
        }

        RGBColor unoColor2RGBColor( sal_Int32 nColor )
        {
            return RGBColor(
                ::cppcanvas::makeColor(
                    // convert from API color to IntSRGBA color
                    // (0xAARRGGBB -> 0xRRGGBBAA)
                    static_cast< sal_uInt8 >( nColor >> 16U ),
                    static_cast< sal_uInt8 >( nColor >> 8U ),
                    static_cast< sal_uInt8 >( nColor ),
                    static_cast< sal_uInt8 >( nColor >> 24U ) ) );
        }

        sal_Int32 RGBAColor2UnoColor( ::cppcanvas::IntSRGBA aColor )
        {
            return ::cppcanvas::makeColorARGB(
                // convert from IntSRGBA color to API color
                // (0xRRGGBBAA -> 0xAARRGGBB)
                static_cast< sal_uInt8 >(0),
                ::cppcanvas::getRed(aColor),
                ::cppcanvas::getGreen(aColor),
                ::cppcanvas::getBlue(aColor));
        }

        void fillRect( const ::cppcanvas::CanvasSharedPtr& rCanvas,
                       const ::basegfx::B2DRectangle&      rRect,
                       ::cppcanvas::IntSRGBA        aFillColor )
        {
            const ::basegfx::B2DPolygon aPoly(
                ::basegfx::utils::createPolygonFromRect( rRect ));

            ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
                ::cppcanvas::BaseGfxFactory::createPolyPolygon( rCanvas, aPoly ) );

            if( pPolyPoly )
            {
                pPolyPoly->setRGBAFillColor( aFillColor );
                pPolyPoly->draw();
            }
        }

        void initSlideBackground( const ::cppcanvas::CanvasSharedPtr& rCanvas,
                                  const ::basegfx::B2ISize&           rSize )
        {
            ::cppcanvas::CanvasSharedPtr pCanvas( rCanvas->clone() );

            // set transformation to identity (->device pixel)
            pCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

            // #i42440# Fill the _full_ background in
            // black. Since we had to extend the bitmap by one
            // pixel, and the bitmap is initialized white,
            // depending on the slide content a one pixel wide
            // line will show to the bottom and the right.
            fillRect( pCanvas,
                      ::basegfx::B2DRectangle( 0.0, 0.0,
                                               rSize.getX(),
                                               rSize.getY() ),
                      0x000000FFU );

            // fill the bounds rectangle in white. Subtract one pixel
            // from both width and height, because the slide size is
            // chosen one pixel larger than given by the drawing
            // layer. This is because shapes with line style, that
            // have the size of the slide would otherwise be cut
            // off. OTOH, every other slide background (solid fill,
            // gradient, bitmap) render one pixel less, thus revealing
            // ugly white pixel to the right and the bottom.
            fillRect( pCanvas,
                      ::basegfx::B2DRectangle( 0.0, 0.0,
                                               rSize.getX()-1,
                                               rSize.getY()-1 ),
                      0xFFFFFFFFU );
        }

        ::basegfx::B2DRectangle getAPIShapeBounds( const uno::Reference< drawing::XShape >& xShape )
        {
            uno::Reference< beans::XPropertySet > xPropSet( xShape,
                                                            uno::UNO_QUERY_THROW );
            // read bound rect
            awt::Rectangle aTmpRect;
            if( !(xPropSet->getPropertyValue("BoundRect") >>= aTmpRect) )
            {
                ENSURE_OR_THROW( false,
                                  "getAPIShapeBounds(): Could not get \"BoundRect\" property from shape" );
            }

            return ::basegfx::B2DRectangle( aTmpRect.X,
                                            aTmpRect.Y,
                                            aTmpRect.X+aTmpRect.Width,
                                            aTmpRect.Y+aTmpRect.Height );
        }

/*
        TODO(F1): When ZOrder someday becomes usable enable this

        double getAPIShapePrio( const uno::Reference< drawing::XShape >& xShape )
        {
            uno::Reference< beans::XPropertySet > xPropSet( xShape,
                                                            uno::UNO_QUERY_THROW );
            // read prio
            sal_Int32 nPrio(0);
            if( !(xPropSet->getPropertyValue(
                      OUString("ZOrder") ) >>= nPrio) )
            {
                ENSURE_OR_THROW( false,
                                  "getAPIShapePrio(): Could not get \"ZOrder\" property from shape" );
            }

            // TODO(F2): Check and adapt the range of possible values here.
            // Maybe we can also take the total number of shapes here
            return nPrio / 65535.0;
        }
*/

        basegfx::B2IVector getSlideSizePixel( const basegfx::B2DVector& rSlideSize,
                                              const UnoViewSharedPtr&   pView )
        {
            ENSURE_OR_THROW(pView, "getSlideSizePixel(): invalid view");

            // determine transformed page bounds
            const basegfx::B2DRange aRect( 0,0,
                                           rSlideSize.getX(),
                                           rSlideSize.getY() );
            basegfx::B2DRange aTmpRect;
            canvas::tools::calcTransformedRectBounds( aTmpRect,
                                                      aRect,
                                                      pView->getTransformation() );

            // #i42440# Returned slide size is one pixel too small, as
            // rendering happens one pixel to the right and below the
            // actual bound rect.
            return basegfx::B2IVector(
                basegfx::fround( aTmpRect.getRange().getX() ) + 1,
                basegfx::fround( aTmpRect.getRange().getY() ) + 1 );
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
