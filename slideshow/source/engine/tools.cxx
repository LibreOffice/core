/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <canvas/debug.hxx>
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
#include <basegfx/tools/lerp.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <cppcanvas/basegfxfactory.hxx>

#include "unoview.hxx"
#include "smilfunctionparser.hxx"
#include "tools.hxx"

#include <limits>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        namespace
        {
            class NamedValueComparator
            {
            public:
                NamedValueComparator( const beans::NamedValue& rKey ) :
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
                                        pAttr->getRotationAngle()*M_PI/180.0 :
                                        0.0 );

                
                
                aTransform.translate( -0.5, -0.5 );

                
                
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

                
                
                
                
                
                aTransform.translate( rShapeBounds.getCenterX(),
                                      rShapeBounds.getCenterY() );

                return aTransform;
            }
        }

        
        

        
        bool extractValue( double&                      o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        rShape,
                           const ::basegfx::B2DVector&  rSlideBounds )
        {
            
            if( (rSourceAny >>= o_rValue) )
            {
                
                return true;
            }

            
            OUString aString;
            if( !(rSourceAny >>= aString) )
                return false; 

            
            try
            {
                
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

        
        bool extractValue( sal_Int32&                       o_rValue,
                           const uno::Any&                  rSourceAny,
                           const ShapeSharedPtr&            /*rShape*/,
                           const ::basegfx::B2DVector&      /*rSlideBounds*/ )
        {
            
            if( (rSourceAny >>= o_rValue) )
            {
                
                return true;
            }

            
            drawing::FillStyle eFillStyle;
            if( (rSourceAny >>= eFillStyle) )
            {
                o_rValue = sal::static_int_cast<sal_Int16>(eFillStyle);

                
                return true;
            }

            drawing::LineStyle eLineStyle;
            if( (rSourceAny >>= eLineStyle) )
            {
                o_rValue = sal::static_int_cast<sal_Int16>(eLineStyle);

                
                return true;
            }

            awt::FontSlant eFontSlant;
            if( (rSourceAny >>= eFontSlant) )
            {
                o_rValue = sal::static_int_cast<sal_Int16>(eFontSlant);

                
                return true;
            }

            
            return false;
        }

        
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

        
        bool extractValue( RGBColor&                    o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        /*rShape*/,
                           const ::basegfx::B2DVector&  /*rSlideBounds*/ )
        {
            
            {
                double nTmp = 0;
                if( (rSourceAny >>= nTmp) )
                {
                    sal_uInt32 aIntColor( static_cast< sal_uInt32 >(nTmp) );

                    
                    o_rValue = unoColor2RGBColor( aIntColor );

                    
                    return true;
                }
            }

            
            {
                uno::Sequence< double > aTmp;
                if( (rSourceAny >>= aTmp) )
                {
                    ENSURE_OR_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for RGB color value" );

                    o_rValue = RGBColor( aTmp[0], aTmp[1], aTmp[2] );

                    
                    return true;
                }
            }

            
            {
                uno::Sequence< sal_Int32 > aTmp;
                if( (rSourceAny >>= aTmp) )
                {
                    ENSURE_OR_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for RGB color value" );

                    
                    o_rValue = RGBColor( ::cppcanvas::makeColor(
                                             static_cast<sal_uInt8>(aTmp[0]),
                                             static_cast<sal_uInt8>(aTmp[1]),
                                             static_cast<sal_uInt8>(aTmp[2]),
                                             255 ) );

                    
                    return true;
                }
            }

            
            {
                uno::Sequence< sal_Int8 > aTmp;
                if( (rSourceAny >>= aTmp) )
                {
                    ENSURE_OR_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for RGB color value" );

                    o_rValue = RGBColor( ::cppcanvas::makeColor( aTmp[0], aTmp[1], aTmp[2], 255 ) );

                    
                    return true;
                }
            }

            
            OUString aString;
            if( !(rSourceAny >>= aString) )
                return false; 

            
            o_rValue = RGBColor( 0.5, 0.5, 0.5 );

            return true;
        }

        
        bool extractValue( HSLColor&                    o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        /*rShape*/,
                           const ::basegfx::B2DVector&  /*rSlideBounds*/ )
        {
            
            {
                uno::Sequence< double > aTmp;
                if( (rSourceAny >>= aTmp) )
                {
                    ENSURE_OR_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for HSL color value" );

                    o_rValue = HSLColor( aTmp[0], aTmp[1], aTmp[2] );

                    
                    return true;
                }
            }

            
            {
                uno::Sequence< sal_Int8 > aTmp;
                if( (rSourceAny >>= aTmp) )
                {
                    ENSURE_OR_THROW( aTmp.getLength() == 3,
                                      "extractValue(): inappropriate length for HSL color value" );

                    o_rValue = HSLColor( aTmp[0]*360.0/255.0, aTmp[1]/255.0, aTmp[2]/255.0 );

                    
                    return true;
                }
            }

            return false; 
        }

        
        bool extractValue( OUString&             o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        /*rShape*/,
                           const ::basegfx::B2DVector&  /*rSlideBounds*/ )
        {
            
            if( !(rSourceAny >>= o_rValue) )
                return false; 

            return true;
        }

        
        bool extractValue( bool&                        o_rValue,
                           const uno::Any&              rSourceAny,
                           const ShapeSharedPtr&        /*rShape*/,
                           const ::basegfx::B2DVector&  /*rSlideBounds*/ )
        {
            sal_Bool nTmp = sal_Bool();
            
            if( (rSourceAny >>= nTmp) )
            {
                o_rValue = nTmp;

                
                return true;
            }

            
            OUString aString;
            if( !(rSourceAny >>= aString) )
                return false; 

            
            
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

            
            return false;
        }

        
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
            const beans::NamedValue*    pArray = rSequence.getConstArray();
            const size_t                nLen( rSequence.getLength() );

            if( nLen == 0 )
                return false;

            const beans::NamedValue* pFound = ::std::find_if( pArray,
                                                              pArray + nLen,
                                                              NamedValueComparator( rSearchKey ) );

            if( pFound == rSequence.getConstArray() + nLen )
                return false;

            return true;
        }

        basegfx::B2DRange calcRelativeShapeBounds( const basegfx::B2DVector& rPageSize,
                                                   const basegfx::B2DRange&  rShapeBounds )
        {
            return basegfx::B2DRange( rShapeBounds.getMinX() / rPageSize.getX(),
                                      rShapeBounds.getMinY() / rPageSize.getY(),
                                      rShapeBounds.getMaxX() / rPageSize.getX(),
                                      rShapeBounds.getMaxY() / rPageSize.getY() );
        }

        
        
        
        
        
        

        ::basegfx::B2DHomMatrix getShapeTransformation( const ::basegfx::B2DRectangle&      rShapeBounds,
                                                        const ShapeAttributeLayerSharedPtr& pAttr )
        {
            if( !pAttr )
            {
                const basegfx::B2DHomMatrix aTransform(basegfx::tools::createScaleTranslateB2DHomMatrix(
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
                                        pAttr->getRotationAngle()*M_PI/180.0 :
                                        0.0 );

                
                
                aTransform.translate( -0.5*rPixelSize.getX(),
                                      -0.5*rPixelSize.getY() );

                const ::basegfx::B2DSize aSize(
                    pAttr->isWidthValid() ? pAttr->getWidth() : rOrigSize.getX(),
                    pAttr->isHeightValid() ? pAttr->getHeight() : rOrigSize.getY() );

                
                
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

                
                
                
                aTransform.translate( 0.5*rPixelSize.getX(),
                                      0.5*rPixelSize.getY() );
            }

            
            
            
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
                
                

                const double nCharScale( pAttr->getCharScale() );

                
                aTransform.translate( -0.5, -0.5 );
                aTransform.scale( nCharScale, nCharScale );
                aTransform.translate( 0.5, 0.5 );
            }

            aTransform *= rShapeTransform;

            ::basegfx::B2DRectangle aRes;

            
            return ::canvas::tools::calcTransformedRectBounds(
                aRes,
                rUnitBounds,
                aTransform );
        }

        ::basegfx::B2DRange getShapeUpdateArea( const ::basegfx::B2DRange&      rUnitBounds,
                                                    const ::basegfx::B2DRange&      rShapeBounds )
        {
            return ::basegfx::B2DRectangle(
                basegfx::tools::lerp( rShapeBounds.getMinX(),
                                      rShapeBounds.getMaxX(),
                                      rUnitBounds.getMinX() ),
                basegfx::tools::lerp( rShapeBounds.getMinY(),
                                      rShapeBounds.getMaxY(),
                                      rUnitBounds.getMinY() ),
                basegfx::tools::lerp( rShapeBounds.getMinX(),
                                      rShapeBounds.getMaxX(),
                                      rUnitBounds.getMaxX() ),
                basegfx::tools::lerp( rShapeBounds.getMinY(),
                                      rShapeBounds.getMaxY(),
                                      rUnitBounds.getMaxY() ) );
        }

        ::basegfx::B2DRectangle getShapePosSize( const ::basegfx::B2DRectangle&         rOrigBounds,
                                                 const ShapeAttributeLayerSharedPtr&    pAttr )
        {
            
            
            
            
            if( !pAttr ||
                rOrigBounds.isEmpty() )
            {
                return rOrigBounds;
            }
            else
            {
                
                
                
                
                
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

                
                
                
                return ::basegfx::B2DRectangle( aPos - 0.5*aSize,
                                                aPos + 0.5*aSize );
            }
        }

        RGBColor unoColor2RGBColor( sal_Int32 nColor )
        {
            return RGBColor(
                ::cppcanvas::makeColor(
                    
                    
                    static_cast< sal_uInt8 >( nColor >> 16U ),
                    static_cast< sal_uInt8 >( nColor >> 8U ),
                    static_cast< sal_uInt8 >( nColor ),
                    static_cast< sal_uInt8 >( nColor >> 24U ) ) );
        }

        sal_Int32 RGBAColor2UnoColor( ::cppcanvas::Color::IntSRGBA aColor )
        {
            return ::cppcanvas::makeColorARGB(
                
                
                static_cast< sal_uInt8 >(0),
                ::cppcanvas::getRed(aColor),
                ::cppcanvas::getGreen(aColor),
                ::cppcanvas::getBlue(aColor));
        }

        void fillRect( const ::cppcanvas::CanvasSharedPtr& rCanvas,
                       const ::basegfx::B2DRectangle&      rRect,
                       ::cppcanvas::Color::IntSRGBA        aFillColor )
        {
            const ::basegfx::B2DPolygon aPoly(
                ::basegfx::tools::createPolygonFromRect( rRect ));

            ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
                ::cppcanvas::BaseGfxFactory::getInstance().createPolyPolygon( rCanvas,
                                                                              aPoly ) );

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

            
            pCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

            
            
            
            
            
            fillRect( pCanvas,
                      ::basegfx::B2DRectangle( 0.0, 0.0,
                                               rSize.getX(),
                                               rSize.getY() ),
                      0x000000FFU );

            
            
            
            
            
            
            
            
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
            
            awt::Rectangle aTmpRect;
            if( !(xPropSet->getPropertyValue(
                      OUString("BoundRect") ) >>= aTmpRect) )
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
            
            sal_Int32 nPrio(0);
            if( !(xPropSet->getPropertyValue(
                      OUString("ZOrder") ) >>= nPrio) )
            {
                ENSURE_OR_THROW( false,
                                  "getAPIShapePrio(): Could not get \"ZOrder\" property from shape" );
            }

            
            
            return nPrio / 65535.0;
        }
*/

        basegfx::B2IVector getSlideSizePixel( const basegfx::B2DVector& rSlideSize,
                                              const UnoViewSharedPtr&   pView )
        {
            ENSURE_OR_THROW(pView, "getSlideSizePixel(): invalid view");

            
            const basegfx::B2DRange aRect( 0,0,
                                           rSlideSize.getX(),
                                           rSlideSize.getY() );
            basegfx::B2DRange aTmpRect;
            canvas::tools::calcTransformedRectBounds( aTmpRect,
                                                      aRect,
                                                      pView->getTransformation() );

            
            
            
            return basegfx::B2IVector(
                basegfx::fround( aTmpRect.getRange().getX() ) + 1,
                basegfx::fround( aTmpRect.getRange().getY() ) + 1 );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
