/*************************************************************************
 *
 *  $RCSfile: tools.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:00:12 $
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

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

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
        ::basegfx::B2DHomMatrix getShapeTransformation( const ::basegfx::B2DRectangle&      rOrigBounds,
                                                        const ::basegfx::B2DRectangle&      rBounds,
                                                        const ShapeAttributeLayerSharedPtr& pAttr,
                                                        bool                                bWithTranslation )
        {
            ::basegfx::B2DHomMatrix aTransform;

            if( !pAttr.get() )
            {
                aTransform.scale( rBounds.getWidth(), rBounds.getHeight() );

                if( bWithTranslation )
                    aTransform.translate( rBounds.getMinX(), rBounds.getMinY() );
            }
            else
            {
                const ::basegfx::B2DSize& rSize( rBounds.getRange() );

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
                aTransform.scale( ::std::max( 0.0001,
                                              rSize.getX() ),
                                  ::std::max( 0.0001,
                                              rSize.getY() ) );

                if( !::basegfx::fTools::equalZero(nShearX) )
                    aTransform.shearX( nShearX );

                if( !::basegfx::fTools::equalZero(nShearY) )
                    aTransform.shearY( nShearY );

                if( !::basegfx::fTools::equalZero(nRotation) )
                    aTransform.rotate( nRotation );

                if( bWithTranslation )
                {
                    // move left, top corner back to position of the
                    // shape. Since we've already translated the
                    // center of the shape to the origin (the
                    // translate( -0.5, -0.5 ) above), translate to
                    // center of final shape position here.
                    aTransform.translate( rBounds.getCenterX(),
                                          rBounds.getCenterY() );
                }
                else
                {
                    // move left, top corner back to origin,
                    // offset calculations take place outside
                    // of this method (sprite offsets etc.)
                    aTransform.translate( rSize.getX() * .5,
                                          rSize.getY() * .5 );
                }
            }

            return aTransform;
        }

        ::basegfx::B2DRectangle getShapeUpdateArea( const ::basegfx::B2DRectangle&      rShapeBounds,
                                                    const ::basegfx::B2DHomMatrix&      rShapeTransform,
                                                    const ShapeAttributeLayerSharedPtr& pAttr )
        {
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
                ::basegfx::B2DRectangle( 0.0, 0.0,
                                         1.0, 1.0 ),
                aTransform );
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
    }
}
