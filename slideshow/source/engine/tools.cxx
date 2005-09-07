/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tools.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:31:18 $
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

// must be first
#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>

#ifndef  _USE_MATH_DEFINES
#define  _USE_MATH_DEFINES  // needed by Visual C++ for math constants
#endif
#include <math.h>           // M_PI definition

#include <tools.hxx>

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _COM_SUN_STAR_ANIMATIONS_VALUEPAIR_HPP_
#include <com/sun/star/animations/ValuePair.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _CPPCANVAS_BASEGFXFACTORY_HXX
#include <cppcanvas/basegfxfactory.hxx>
#endif

#include <lerp.hxx>
#include <smilfunctionparser.hxx>



using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        namespace
        {
            class NamedValueStringComparator
            {
            public:
                NamedValueStringComparator( const ::rtl::OUString& rSearchString ) :
                    mrSearchString( rSearchString )
                {
                }

                bool operator()( const beans::NamedValue& rValue )
                {
                    return rValue.Name == mrSearchString;
                }

            private:
                const ::rtl::OUString&      mrSearchString;
            };

            class NamedValueComparator
            {
            public:
                NamedValueComparator( const beans::NamedValue& rKey ) :
                    mrKey( rKey )
                {
                }

                bool operator()( const beans::NamedValue& rValue )
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
                                        pAttr->getRotationAngle()*M_PI/180.0 :
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
        bool extractValue( double&                          o_rValue,
                           const uno::Any&                  rSourceAny,
                           const ShapeSharedPtr&            rShape,
                           const LayerManagerSharedPtr&     rLayerManager )
        {
            // try to extract numeric value (double, or smaller POD, like float or int)
            if( (rSourceAny >>= o_rValue) )
            {
                // succeeded
                return true;
            }

            // try to extract string
            ::rtl::OUString aString;
            if( !(rSourceAny >>= aString) )
                return false; // nothing left to try

            // parse the string into an ExpressionNode
            try
            {
                // TODO(P2): Optimize the case that AttributeName and
                // Value are identical!

                ENSURE_AND_RETURN( rShape.get() && rLayerManager.get(),
                                   "extractValue(): need relative shape size for parsing, "
                                   "no shape or layer manager given" );

                // Parse string into ExpressionNode, eval node at time 0.0
                o_rValue = (*SmilFunctionParser::parseSmilValue(
                                aString,
                                calcRelativeShapeBounds(rLayerManager->getPageBounds(),
                                                        rShape->getPosSize()) ))(0.0);
            }
            catch( ParseError& )
            {
                return false;
            }

            return true;
        }

        /// extract enum/constant group value from Any
        bool extractValue( sal_Int16&                       o_rValue,
                           const uno::Any&                  rSourceAny,
                           const ShapeSharedPtr&            rShape,
                           const LayerManagerSharedPtr&     rLayerManager )
        {
            // try to extract numeric value (int, or smaller POD, like byte)
            if( (rSourceAny >>= o_rValue) )
            {
                // succeeded
                return true;
            }

            // okay, no plain int. Maybe one of the domain-specific enums?
            drawing::FillStyle eFillStyle;
            if( (rSourceAny >>= eFillStyle) )
            {
                o_rValue = eFillStyle;

                // succeeded
                return true;
            }

            drawing::LineStyle eLineStyle;
            if( (rSourceAny >>= eLineStyle) )
            {
                o_rValue = eLineStyle;

                // succeeded
                return true;
            }

            awt::FontSlant eFontSlant;
            if( (rSourceAny >>= eFontSlant) )
            {
                o_rValue = eFontSlant;

                // succeeded
                return true;
            }

            // nothing left to try. Failure
            return false;
        }

        /// extract color value from Any
        bool extractValue( RGBColor&                    o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        rShape,
                           const LayerManagerSharedPtr& rLayerManager )
        {
            // try to extract numeric value (double, or smaller POD, like float or int)
            {
                double nTmp;
                if( (rSourceAny >>= nTmp) )
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
                if( (rSourceAny >>= aTmp) )
                {
                    ENSURE_AND_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for RGB color value" );

                    o_rValue = RGBColor( aTmp[0], aTmp[1], aTmp[2] );

                    // succeeded
                    return true;
                }
            }

            // try sal_Int32 sequence
            {
                uno::Sequence< sal_Int32 > aTmp;
                if( (rSourceAny >>= aTmp) )
                {
                    ENSURE_AND_THROW( aTmp.getLength() == 3,
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
                if( (rSourceAny >>= aTmp) )
                {
                    ENSURE_AND_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for RGB color value" );

                    o_rValue = RGBColor( ::cppcanvas::makeColor( aTmp[0], aTmp[1], aTmp[2], 255 ) );

                    // succeeded
                    return true;
                }
            }

            // try to extract string
            ::rtl::OUString aString;
            if( !(rSourceAny >>= aString) )
                return false; // nothing left to try

            // TODO(F2): Provide symbolic color values here
            o_rValue = RGBColor( 0.5, 0.5, 0.5 );

            return true;
        }

        /// extract color value from Any
        bool extractValue( HSLColor&                    o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        rShape,
                           const LayerManagerSharedPtr& rLayerManager )
        {
            // try double sequence
            {
                uno::Sequence< double > aTmp;
                if( (rSourceAny >>= aTmp) )
                {
                    ENSURE_AND_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for HSL color value" );

                    o_rValue = HSLColor( aTmp[0], aTmp[1], aTmp[2] );

                    // succeeded
                    return true;
                }
            }

            // try sal_Int8 sequence
            {
                uno::Sequence< sal_Int8 > aTmp;
                if( (rSourceAny >>= aTmp) )
                {
                    ENSURE_AND_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for HSL color value" );

                    o_rValue = HSLColor( aTmp[0]*360.0/255.0, aTmp[1]/255.0, aTmp[2]/255.0 );

                    // succeeded
                    return true;
                }
            }

            return false; // nothing left to try
        }

        /// extract plain string from Any
        bool extractValue( ::rtl::OUString&                 o_rValue,
                           const uno::Any&                  rSourceAny,
                           const ShapeSharedPtr&            rShape,
                           const LayerManagerSharedPtr&     rLayerManager )
        {
            // try to extract string
            if( !(rSourceAny >>= o_rValue) )
                return false; // nothing left to try

            return true;
        }

        /// extract bool value from Any
        bool extractValue( bool&                            o_rValue,
                           const uno::Any&                  rSourceAny,
                           const ShapeSharedPtr&            rShape,
                           const LayerManagerSharedPtr&     rLayerManager )
        {
            sal_Bool nTmp;
            // try to extract bool value
            if( (rSourceAny >>= nTmp) )
            {
                o_rValue = nTmp;

                // succeeded
                return true;
            }

            // try to extract string
            ::rtl::OUString aString;
            if( !(rSourceAny >>= aString) )
                return false; // nothing left to try

            // we also take the strings "true" and "false",
            // as well as "on" and "off" here
            if( aString.equalsIgnoreAsciiCaseAscii("true") ||
                aString.equalsIgnoreAsciiCaseAscii("on") )
            {
                o_rValue = true;
                return true;
            }
            if( aString.equalsIgnoreAsciiCaseAscii("false") ||
                aString.equalsIgnoreAsciiCaseAscii("off") )
            {
                o_rValue = false;
                return true;
            }

            // ultimately failed.
            return false;
        }

        /// extract double 2-tuple from Any
        bool extractValue( ::basegfx::B2DTuple&             o_rPair,
                           const uno::Any&                  rSourceAny,
                           const ShapeSharedPtr&            rShape,
                           const LayerManagerSharedPtr&     rLayerManager )
        {
            animations::ValuePair aPair;

            if( !(rSourceAny >>= aPair) )
                return false;

            double nFirst;
            if( !extractValue( nFirst, aPair.First, rShape, rLayerManager ) )
                return false;

            double nSecond;
            if( !extractValue( nSecond, aPair.Second, rShape, rLayerManager ) )
                return false;

            o_rPair.setX( nFirst );
            o_rPair.setY( nSecond );

            return true;
        }

        bool findNamedValue( uno::Sequence< beans::NamedValue >&    rSequence,
                             const beans::NamedValue&               rSearchKey )
        {
            const beans::NamedValue*    pArray = rSequence.getArray();
            const size_t                nLen( rSequence.getLength() );

            if( nLen == 0 )
                return false;

            const beans::NamedValue* pFound = ::std::find_if( pArray,
                                                              pArray + nLen,
                                                              NamedValueComparator( rSearchKey ) );

            if( pFound == pArray + nLen )
                return false;

            return true;
        }

        bool findNamedValue( beans::NamedValue*                         o_pRet,
                             const uno::Sequence< beans::NamedValue >&  rSequence,
                             const ::rtl::OUString&                     rSearchString )
        {
            const beans::NamedValue*    pArray = rSequence.getConstArray();
            const size_t                nLen( rSequence.getLength() );

            if( nLen == 0 )
                return false;

            const beans::NamedValue* pFound = ::std::find_if( pArray,
                                                              pArray + nLen,
                                                              NamedValueStringComparator( rSearchString ) );
            if( pFound == pArray + nLen )
                return false;

            if( o_pRet )
                *o_pRet = *pFound;

            return true;
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
            if( !pAttr.get() )
            {
                ::basegfx::B2DHomMatrix aTransform;

                aTransform.scale( rShapeBounds.getWidth(), rShapeBounds.getHeight() );
                aTransform.translate( rShapeBounds.getMinX(), rShapeBounds.getMinY() );

                return aTransform;
            }
            else
            {
                return getAttributedShapeTransformation( rShapeBounds,
                                                         pAttr );
            }
        }

        ::basegfx::B2DHomMatrix getSpriteTransformation( const ::basegfx::B2DSize&              rPixelSize,
                                                         const ::basegfx::B2DSize&              rOrigSize,
                                                         const ShapeAttributeLayerSharedPtr&    pAttr )
        {
            ::basegfx::B2DHomMatrix aTransform;

            if( pAttr.get() )
            {
                const double nShearX( pAttr->isShearXAngleValid() ?
                                      pAttr->getShearXAngle() :
                                      0.0 );
                const double nShearY( pAttr->isShearYAngleValid() ?
                                      pAttr->getShearYAngle() :
                                      0.0 );
                const double nRotation( pAttr->isRotationAngleValid() ?
                                        pAttr->getRotationAngle()*M_PI/180.0 :
                                        0.0 );

                // scale, shear and rotation pivot point is the
                // sprite's pixel center center - adapt origin
                // accordingly
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
            // shapes. This renders the sprite as-is, in it's
            // document-supplied size.
            return aTransform;
        }

        ::basegfx::B2DRectangle getShapeUpdateArea( const ::basegfx::B2DRectangle&      rUnitBounds,
                                                    const ::basegfx::B2DHomMatrix&      rShapeTransform,
                                                    const ShapeAttributeLayerSharedPtr& pAttr )
        {
            ENSURE_AND_THROW( pAttr.get(),
                              "getShapeUpdateArea(): Invalid ShapeAttributeLayer" );

            ::basegfx::B2DHomMatrix aTransform;

            if( pAttr->isCharScaleValid() &&
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

        ::basegfx::B2DRectangle getShapeUpdateArea( const ::basegfx::B2DRectangle&      rUnitBounds,
                                                    const ::basegfx::B2DRectangle&      rShapeBounds )
        {
            return ::basegfx::B2DRectangle(
                lerp( rShapeBounds.getMinX(),
                      rShapeBounds.getMaxX(),
                      rUnitBounds.getMinX() ),
                lerp( rShapeBounds.getMinY(),
                      rShapeBounds.getMaxY(),
                      rUnitBounds.getMinY() ),
                lerp( rShapeBounds.getMinX(),
                      rShapeBounds.getMaxX(),
                      rUnitBounds.getMaxX() ),
                lerp( rShapeBounds.getMinY(),
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
            if( !pAttr.get() ||
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

        void initSlideBackground( const ::cppcanvas::CanvasSharedPtr& rCanvas,
                                  const ::basegfx::B2ISize&           rSize )
        {
            ::cppcanvas::CanvasSharedPtr pCanvas( rCanvas->clone() );

            // set transformation to identitiy (->device pixel)
            pCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

            // #i42440# Fill the _full_ background in
            // black. Since we had to extend the bitmap by one
            // pixel, and the bitmap is initialized white,
            // depending on the slide content a one pixel wide
            // line will show to the bottom and the right.
            const ::basegfx::B2DPolygon aPoly(
                ::basegfx::tools::createPolygonFromRect(
                    ::basegfx::B2DRectangle( 0.0, 0.0,
                                             rSize.getX(),
                                             rSize.getY() ) ) );

            ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
                ::cppcanvas::BaseGfxFactory::getInstance().createPolyPolygon( pCanvas, aPoly ) );

            if( pPolyPoly.get() )
            {
                pPolyPoly->setRGBAFillColor( 0x000000FFU );
                pPolyPoly->draw();
            }

            // fill the bounds rectangle in white. Subtract one pixel
            // from both width and height, because the slide size is
            // chosen one pixel larger than given by the drawing
            // layer. This is because shapes with line style, that
            // have the size of the slide would otherwise be cut
            // off. OTOH, every other slide background (solid fill,
            // gradient, bitmap) render one pixel less, thus revealing
            // ugly white pixel to the right and the bottom.
            const ::basegfx::B2DPolygon aPoly2(
                ::basegfx::tools::createPolygonFromRect(
                    ::basegfx::B2DRectangle( 0.0, 0.0,
                                             rSize.getX()-1,
                                             rSize.getY()-1 ) ) );

            pPolyPoly = ::cppcanvas::BaseGfxFactory::getInstance().createPolyPolygon(
                pCanvas, aPoly2 );

            if( pPolyPoly.get() )
            {
                pPolyPoly->setRGBAFillColor( 0xFFFFFFFFU );
                pPolyPoly->draw();
            }
        }
    }
}
