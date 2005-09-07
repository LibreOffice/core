/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shapeattributelayer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:28:51 $
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
#include <shapeattributelayer.hxx>

#ifndef _CANVAS_VERBOSETRACE_HXX
#include <canvas/verbosetrace.hxx>
#endif


#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONADDITIVEMODE_HPP_
#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif


using namespace ::com::sun::star;


namespace presentation
{
    namespace internal
    {
        /** Update state ids

            This method updates all state IDs from possible
            children. Whenever a child's state ID changed, we
            increment ours.
        */
        void ShapeAttributeLayer::updateStateIds()
        {
            if( haveChild() )
            {
                if( mnTransformationState != mpChild->getTransformationState() )
                    ++mnTransformationState;
                if( mnClipState != mpChild->getClipState() )
                    ++mnClipState;
                if( mnAlphaState != mpChild->getAlphaState() )
                    ++mnAlphaState;
                if( mnPositionState != mpChild->getPositionState() )
                    ++mnPositionState;
                if( mnContentState != mpChild->getContentState() )
                    ++mnContentState;
            }
        }

        /** Calc attribute value.

            This method determines the current attribute value,
            appropriately combining it with children values (by
            evaluating the mnAdditiveMode member).
         */
        template< typename T > T ShapeAttributeLayer::calcValue( const T&                   rCurrValue,
                                                                 bool                       bThisInstanceValid,
                                                                 bool (ShapeAttributeLayer::*pIsValid)() const,
                                                                 T   (ShapeAttributeLayer::*pGetValue)() const ) const
        {
            // deviated from the (*shared_ptr).*mpFuncPtr notation
            // here, since gcc does not seem to parse that as a member
            // function call anymore.
            const bool bChildInstanceValueValid( haveChild() ? (mpChild.get()->*pIsValid)() : false );

            if( bThisInstanceValid )
            {
                if( bChildInstanceValueValid )
                {
                    // merge with child value
                    switch( mnAdditiveMode )
                    {
                        default:
                            // FALTHROUGH intended
                        case animations::AnimationAdditiveMode::NONE:
                            // FALTHROUGH intended
                        case animations::AnimationAdditiveMode::BASE:
                            // FALTHROUGH intended
                        case animations::AnimationAdditiveMode::REPLACE:
                            // TODO(F2): reverse-engineer the semantics of these
                            // values

                            // currently, treat them the same and replace
                            // the child value by our own
                            return rCurrValue;

                        case animations::AnimationAdditiveMode::SUM:
                            return rCurrValue + ((*mpChild).*pGetValue)();

                        case animations::AnimationAdditiveMode::MULTIPLY:
                            return rCurrValue * ((*mpChild).*pGetValue)();
                    }
                }
                else
                {
                    // this object is the only one defining
                    // the value, so take it
                    return rCurrValue;
                }
            }
            else
            {
                return bChildInstanceValueValid ?
                    ((*mpChild).*pGetValue)() :
                    T();            // pass on child value, regardless
                                    // if it's valid or not. If not, it's
                                    // a default anyway
            }
        }

        ShapeAttributeLayer::ShapeAttributeLayer( const ShapeAttributeLayerSharedPtr& rChildLayer ) :
            mpChild( rChildLayer ),

            maSize(),
            maPosition(),
            maClip(),

            maFontFamily(),

            mnRotationAngle(),
            mnShearXAngle(),
            mnShearYAngle(),
            mnAlpha(),
            mnCharRotationAngle(),
            mnCharScale(),
            mnCharWeight(),

            meFillStyle( drawing::FillStyle_NONE ),
            meLineStyle( drawing::LineStyle_NONE ),
            meCharPosture( awt::FontSlant_NONE ),
            mnUnderlineMode(),

            maDimColor(),
            maFillColor(),
            maLineColor(),
            maCharColor(),

            mnTransformationState( rChildLayer.get() ? rChildLayer->getTransformationState() : 0 ),
            mnClipState( rChildLayer.get() ? rChildLayer->getClipState() : 0),
            mnAlphaState( rChildLayer.get() ? rChildLayer->getAlphaState() : 0),
            mnPositionState( rChildLayer.get() ? rChildLayer->getPositionState() : 0 ),
            mnContentState( rChildLayer.get() ? rChildLayer->getContentState() : 0 ),

            mnAdditiveMode( animations::AnimationAdditiveMode::BASE ),

            mbVisibility( false ),

            mbWidthValid( false ),
            mbHeightValid( false ),
            mbPosXValid( false ),
            mbPosYValid( false ),
            mbClipValid( false ),

            mbFontFamilyValid( false ),

            mbRotationAngleValid( false ),
            mbShearXAngleValid( false ),
            mbShearYAngleValid( false ),

            mbAlphaValid( false ),

            mbCharRotationAngleValid( false ),
            mbCharScaleValid( false ),

            mbDimColorValid( false ),
            mbFillColorValid( false ),
            mbLineColorValid( false ),
            mbCharColorValid( false ),

            mbFillStyleValid( false ),
            mbLineStyleValid( false ),
            mbCharWeightValid( false ),
            mbUnderlineModeValid( false ),
            mbCharPostureValid( false ),
            mbVisibilityValid( false )
        {
        }

        bool ShapeAttributeLayer::revokeChildLayer( const ShapeAttributeLayerSharedPtr& rChildLayer )
        {
            ENSURE_AND_RETURN( rChildLayer.get(),
                               "ShapeAttributeLayer::revokeChildLayer(): Will not remove NULL child" );

            if( !haveChild() )
                return false; // no children, nothing to revoke.

            if( mpChild == rChildLayer )
            {
                // we have it - replace by removed child's sibling.
                mpChild = rChildLayer->getChildLayer();

                // if we're now the first one, defensively increment _all_
                // state ids: possibly all underlying attributes have now
                // changed to default
                if( !haveChild() )
                {
                    // TODO(P1): Check whether it pays off to check more
                    // detailed, which attributes really change
                    ++mnTransformationState;
                    ++mnClipState;
                    ++mnAlphaState;
                    ++mnPositionState;
                    ++mnContentState;
                }
            }
            else
            {
                // we don't have it - pass on the request
                if( !mpChild->revokeChildLayer( rChildLayer ) )
                    return false; // nobody has it - bail out
            }

            // something might have changed - update ids.
            updateStateIds();

            return true;
        }

        ShapeAttributeLayerSharedPtr  ShapeAttributeLayer::getChildLayer() const
        {
            return mpChild;
        }

        void ShapeAttributeLayer::setAdditiveMode( sal_Int16 nMode )
        {
            if( mnAdditiveMode != nMode )
            {
                // TODO(P1): Check whether it pays off to check more
                // detailed, which attributes really change

                // defensively increment all states - possibly each of them
                // will change with different additive mode
                ++mnTransformationState;
                ++mnClipState;
                ++mnAlphaState;
                ++mnPositionState;
                ++mnContentState;
            }

            mnAdditiveMode = nMode;
        }

        bool ShapeAttributeLayer::isWidthValid() const
        {
            return mbWidthValid ? true : haveChild() ? mpChild->isWidthValid() : false;
        }

        double ShapeAttributeLayer::getWidth() const
        {
            return calcValue< double >(
                maSize.getX(),
                mbWidthValid,
                &ShapeAttributeLayer::isWidthValid,
                &ShapeAttributeLayer::getWidth );
        }

        void ShapeAttributeLayer::setWidth( const double& rNewWidth )
        {
            ENSURE_AND_THROW( ::rtl::math::isFinite(rNewWidth),
                              "ShapeAttributeLayer::setWidth(): Invalid width" );

            maSize.setX( rNewWidth );
            mbWidthValid = true;
            ++mnTransformationState;
        }

        bool ShapeAttributeLayer::isHeightValid() const
        {
            return mbHeightValid ? true : haveChild() ? mpChild->isHeightValid() : false;
        }

        double ShapeAttributeLayer::getHeight() const
        {
            return calcValue< double >(
                maSize.getY(),
                mbHeightValid,
                &ShapeAttributeLayer::isHeightValid,
                &ShapeAttributeLayer::getHeight );
        }

        void ShapeAttributeLayer::setHeight( const double& rNewHeight )
        {
            ENSURE_AND_THROW( ::rtl::math::isFinite(rNewHeight),
                              "ShapeAttributeLayer::setHeight(): Invalid height" );

            maSize.setY( rNewHeight );
            mbHeightValid = true;
            ++mnTransformationState;
        }

        void ShapeAttributeLayer::setSize( const ::basegfx::B2DSize& rNewSize )
        {
            ENSURE_AND_THROW( ::rtl::math::isFinite(rNewSize.getX()) &&
                              ::rtl::math::isFinite(rNewSize.getY()),
                              "ShapeAttributeLayer::setSize(): Invalid size" );

            maSize = rNewSize;
            mbWidthValid = mbHeightValid = true;
            ++mnTransformationState;
        }

        bool ShapeAttributeLayer::isPosXValid() const
        {
            return mbPosXValid ? true : haveChild() ? mpChild->isPosXValid() : false;
        }

        double ShapeAttributeLayer::getPosX() const
        {
            return calcValue< double >(
                maPosition.getX(),
                mbPosXValid,
                &ShapeAttributeLayer::isPosXValid,
                &ShapeAttributeLayer::getPosX );
        }

        void ShapeAttributeLayer::setPosX( const double& rNewX )
        {
            ENSURE_AND_THROW( ::rtl::math::isFinite(rNewX),
                              "ShapeAttributeLayer::setPosX(): Invalid position" );

            maPosition.setX( rNewX );
            mbPosXValid = true;
            ++mnPositionState;
        }

        bool ShapeAttributeLayer::isPosYValid() const
        {
            return mbPosYValid ? true : haveChild() ? mpChild->isPosYValid() : false;
        }

        double ShapeAttributeLayer::getPosY() const
        {
            return calcValue< double >(
                maPosition.getY(),
                mbPosYValid,
                &ShapeAttributeLayer::isPosYValid,
                &ShapeAttributeLayer::getPosY );
        }

        void ShapeAttributeLayer::setPosY( const double& rNewY )
        {
            ENSURE_AND_THROW( ::rtl::math::isFinite(rNewY),
                              "ShapeAttributeLayer::setPosY(): Invalid position" );

            maPosition.setY( rNewY );
            mbPosYValid = true;
            ++mnPositionState;
        }

        void ShapeAttributeLayer::setPosition( const ::basegfx::B2DPoint& rNewPos )
        {
            maPosition = rNewPos;
            mbPosXValid = mbPosYValid = true;
            ++mnPositionState;
        }

        bool ShapeAttributeLayer::isRotationAngleValid() const
        {
            return mbRotationAngleValid ? true : haveChild() ? mpChild->isRotationAngleValid() : false;
        }

        double ShapeAttributeLayer::getRotationAngle() const
        {
            return calcValue< double >(
                mnRotationAngle,
                mbRotationAngleValid,
                &ShapeAttributeLayer::isRotationAngleValid,
                &ShapeAttributeLayer::getRotationAngle );
        }

        void ShapeAttributeLayer::setRotationAngle( const double& rNewAngle )
        {
            ENSURE_AND_THROW( ::rtl::math::isFinite(rNewAngle),
                              "ShapeAttributeLayer::setRotationAngle(): Invalid angle" );

            mnRotationAngle = rNewAngle;
            mbRotationAngleValid = true;
            ++mnTransformationState;
        }

        bool ShapeAttributeLayer::isShearXAngleValid() const
        {
            return mbShearXAngleValid ? true : haveChild() ? mpChild->isShearXAngleValid() : false;
        }

        double ShapeAttributeLayer::getShearXAngle() const
        {
            return calcValue( mnShearXAngle,
                              mbShearXAngleValid,
                              &ShapeAttributeLayer::isShearXAngleValid,
                              &ShapeAttributeLayer::getShearXAngle );
        }

        void ShapeAttributeLayer::setShearXAngle( const double& rNewAngle )
        {
            ENSURE_AND_THROW( ::rtl::math::isFinite(rNewAngle),
                              "ShapeAttributeLayer::setShearXAngle(): Invalid angle" );

            mnShearXAngle = rNewAngle;
            mbShearXAngleValid = true;
            ++mnTransformationState;
        }

        bool ShapeAttributeLayer::isShearYAngleValid() const
        {
            return mbShearYAngleValid ? true : haveChild() ? mpChild->isShearYAngleValid() : false;
        }

        double ShapeAttributeLayer::getShearYAngle() const
        {
            return calcValue( mnShearYAngle,
                              mbShearYAngleValid,
                              &ShapeAttributeLayer::isShearYAngleValid,
                              &ShapeAttributeLayer::getShearYAngle );
        }

        void ShapeAttributeLayer::setShearYAngle( const double& rNewAngle )
        {
            ENSURE_AND_THROW( ::rtl::math::isFinite(rNewAngle),
                              "ShapeAttributeLayer::setShearYAngle(): Invalid angle" );

            mnShearYAngle = rNewAngle;
            mbShearYAngleValid = true;
            ++mnTransformationState;
        }

        bool ShapeAttributeLayer::isAlphaValid() const
        {
            return mbAlphaValid ? true : haveChild() ? mpChild->isAlphaValid() : false;
        }

        double ShapeAttributeLayer::getAlpha() const
        {
            return calcValue( mnAlpha,
                              mbAlphaValid,
                              &ShapeAttributeLayer::isAlphaValid,
                              &ShapeAttributeLayer::getAlpha );
        }

        void ShapeAttributeLayer::setAlpha( const double& rNewValue )
        {
            ENSURE_AND_THROW( ::rtl::math::isFinite(rNewValue),
                              "ShapeAttributeLayer::setAlpha(): Invalid alpha" );

            mnAlpha = rNewValue;
            mbAlphaValid = true;
            ++mnAlphaState;
        }

        bool ShapeAttributeLayer::isClipValid() const
        {
            return mbClipValid ? true : haveChild() ? mpChild->isClipValid() : false;
        }

        ::basegfx::B2DPolyPolygon ShapeAttributeLayer::getClip() const
        {
            // TODO(F1): Implement polygon algebra for additive modes
            if( mbClipValid )
                return maClip;
            else if( haveChild() )
                return mpChild->getClip();
            else
                return ::basegfx::B2DPolyPolygon();
        }

        void ShapeAttributeLayer::setClip( const ::basegfx::B2DPolyPolygon& rNewClip )
        {
            maClip = rNewClip;
            mbClipValid = true;
            ++mnClipState;
        }

        bool ShapeAttributeLayer::isDimColorValid() const
        {
            return mbDimColorValid ? true : haveChild() ? mpChild->isDimColorValid() : false;
        }

        RGBColor ShapeAttributeLayer::getDimColor() const
        {
            return calcValue( maDimColor,
                              mbDimColorValid,
                              &ShapeAttributeLayer::isDimColorValid,
                              &ShapeAttributeLayer::getDimColor );
        }

        void ShapeAttributeLayer::setDimColor( const RGBColor& nNewColor )
        {
            maDimColor = nNewColor;
            mbDimColorValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isFillColorValid() const
        {
            return mbFillColorValid ? true : haveChild() ? mpChild->isFillColorValid() : false;
        }

        RGBColor ShapeAttributeLayer::getFillColor() const
        {
            return calcValue( maFillColor,
                              mbFillColorValid,
                              &ShapeAttributeLayer::isFillColorValid,
                              &ShapeAttributeLayer::getFillColor );
        }

        void ShapeAttributeLayer::setFillColor( const RGBColor& nNewColor )
        {
            maFillColor = nNewColor;
            mbFillColorValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isLineColorValid() const
        {
            return mbLineColorValid ? true : haveChild() ? mpChild->isLineColorValid() : false;
        }

        RGBColor  ShapeAttributeLayer::getLineColor() const
        {
            return calcValue( maLineColor,
                              mbLineColorValid,
                              &ShapeAttributeLayer::isLineColorValid,
                              &ShapeAttributeLayer::getLineColor );
        }

        void ShapeAttributeLayer::setLineColor( const RGBColor& nNewColor )
        {
            maLineColor = nNewColor;
            mbLineColorValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isFillStyleValid() const
        {
            return mbFillStyleValid ? true : haveChild() ? mpChild->isFillStyleValid() : false;
        }

        sal_Int16 ShapeAttributeLayer::getFillStyle() const
        {
            // mnAdditiveMode is ignored, cannot combine strings in
            // any sensible way
            if( mbFillStyleValid )
                return meFillStyle;
            else if( haveChild() )
                return mpChild->getFillStyle();
            else
                return drawing::FillStyle_SOLID;
        }

        void ShapeAttributeLayer::setFillStyle( const sal_Int16& rStyle )
        {
            // TODO(Q1): Check range here.
            meFillStyle = (drawing::FillStyle)rStyle;
            mbFillStyleValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isLineStyleValid() const
        {
            return mbLineStyleValid ? true : haveChild() ? mpChild->isLineStyleValid() : false;
        }

        sal_Int16 ShapeAttributeLayer::getLineStyle() const
        {
            // mnAdditiveMode is ignored, cannot combine strings in
            // any sensible way
            if( mbLineStyleValid )
                return meLineStyle;
            else if( haveChild() )
                return mpChild->getLineStyle();
            else
                return drawing::LineStyle_SOLID;
        }

        void ShapeAttributeLayer::setLineStyle( const sal_Int16& rStyle )
        {
            // TODO(Q1): Check range here.
            meLineStyle = (drawing::LineStyle)rStyle;
            mbLineStyleValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isVisibilityValid() const
        {
            return mbVisibilityValid ? true : haveChild() ? mpChild->isVisibilityValid() : false;
        }

        bool ShapeAttributeLayer::getVisibility() const
        {
            // mnAdditiveMode is ignored, SMIL spec requires to not combine
            // bools in any sensible way
            if( mbVisibilityValid )
                return mbVisibility;
            else if( haveChild() )
                return mpChild->getVisibility();
            else
                return true; // default is always visible
        }

        void ShapeAttributeLayer::setVisibility( const bool& bVisible )
        {
            mbVisibility = bVisible;
            mbVisibilityValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isCharColorValid() const
        {
            return mbCharColorValid ? true : haveChild() ? mpChild->isCharColorValid() : false;
        }

        RGBColor ShapeAttributeLayer::getCharColor() const
        {
            return calcValue( maCharColor,
                              mbCharColorValid,
                              &ShapeAttributeLayer::isCharColorValid,
                              &ShapeAttributeLayer::getCharColor );
        }

        void ShapeAttributeLayer::setCharColor( const RGBColor& nNewColor )
        {
            maCharColor = nNewColor;
            mbCharColorValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isCharRotationAngleValid() const
        {
            return mbCharRotationAngleValid ? true : haveChild() ? mpChild->isCharRotationAngleValid() : false;
        }

        double ShapeAttributeLayer::getCharRotationAngle() const
        {
            return calcValue( mnCharRotationAngle,
                              mbCharRotationAngleValid,
                              &ShapeAttributeLayer::isCharRotationAngleValid,
                              &ShapeAttributeLayer::getCharRotationAngle );
        }

        void ShapeAttributeLayer::setCharRotationAngle( const double& rNewAngle )
        {
            ENSURE_AND_THROW( ::rtl::math::isFinite(rNewAngle),
                              "ShapeAttributeLayer::setCharRotationAngle(): Invalid angle" );

            mnCharRotationAngle = rNewAngle;
            mbCharRotationAngleValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isCharWeightValid() const
        {
            return mbCharWeightValid ? true : haveChild() ? mpChild->isCharWeightValid() : false;
        }

        double ShapeAttributeLayer::getCharWeight() const
        {
            // mnAdditiveMode is ignored, cannot combine strings in
            // any sensible way
            if( mbCharWeightValid )
                return mnCharWeight;
            else if( haveChild() )
                return mpChild->getCharWeight();
            else
                return awt::FontWeight::NORMAL;
        }

        void ShapeAttributeLayer::setCharWeight( const double& rValue )
        {
            // TODO(Q1): Check range here.
            mnCharWeight = rValue;
            mbCharWeightValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isUnderlineModeValid() const
        {
            return mbUnderlineModeValid ? true : haveChild() ? mpChild->isUnderlineModeValid() : false;
        }

        sal_Int16 ShapeAttributeLayer::getUnderlineMode() const
        {
            // mnAdditiveMode is ignored, SMIL spec requires to not combine
            // bools in any sensible way
            if( mbUnderlineModeValid )
                return mnUnderlineMode;
            else if( haveChild() )
                return mpChild->getUnderlineMode();
            else
                return awt::FontUnderline::NONE; // default is no underline
        }

        void ShapeAttributeLayer::setUnderlineMode( const sal_Int16& rUnderlineMode )
        {
            // TODO(Q1): Check range here.
            mnUnderlineMode = rUnderlineMode;
            mbUnderlineModeValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isFontFamilyValid() const
        {
            return mbFontFamilyValid ? true : haveChild() ? mpChild->isFontFamilyValid() : false;
        }

        ::rtl::OUString ShapeAttributeLayer::getFontFamily() const
        {
            // mnAdditiveMode is ignored, cannot combine strings in
            // any sensible way
            if( mbFontFamilyValid )
                return maFontFamily;
            else if( haveChild() )
                return mpChild->getFontFamily();
            else
                return ::rtl::OUString();
        }

        void ShapeAttributeLayer::setFontFamily( const ::rtl::OUString& rName )
        {
            maFontFamily = rName;
            mbFontFamilyValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isCharPostureValid() const
        {
            return mbCharPostureValid ? true : haveChild() ? mpChild->isCharPostureValid() : false;
        }

        sal_Int16 ShapeAttributeLayer::getCharPosture() const
        {
            // mnAdditiveMode is ignored, cannot combine strings in
            // any sensible way
            if( mbCharPostureValid )
                return meCharPosture;
            else if( haveChild() )
                return mpChild->getCharPosture();
            else
                return awt::FontSlant_NONE;
        }

        void ShapeAttributeLayer::setCharPosture( const sal_Int16& rStyle )
        {
            // TODO(Q1): Check range here.
            meCharPosture = (awt::FontSlant)rStyle;
            mbCharPostureValid = true;
            ++mnContentState;
        }

        bool ShapeAttributeLayer::isCharScaleValid() const
        {
            return mbCharScaleValid ? true : haveChild() ? mpChild->isCharScaleValid() : false;
        }

        double ShapeAttributeLayer::getCharScale() const
        {
            return calcValue( mnCharScale,
                              mbCharScaleValid,
                              &ShapeAttributeLayer::isCharScaleValid,
                              &ShapeAttributeLayer::getCharScale );
        }

        void ShapeAttributeLayer::setCharScale( const double& rNewHeight )
        {
            ENSURE_AND_THROW( ::rtl::math::isFinite(rNewHeight),
                              "ShapeAttributeLayer::setCharScale(): Invalid height" );

            mnCharScale = rNewHeight;
            mbCharScaleValid = true;
            ++mnContentState;
        }

        State::StateId ShapeAttributeLayer::getTransformationState() const
        {
            return haveChild() ?
                ::std::max( mnTransformationState,
                            mpChild->getTransformationState() ) :
                mnTransformationState;
        }

        State::StateId ShapeAttributeLayer::getClipState() const
        {
            return haveChild() ?
                ::std::max( mnClipState,
                            mpChild->getClipState() ) :
                mnClipState;
        }

        State::StateId ShapeAttributeLayer::getAlphaState() const
        {
            return haveChild() ?
                ::std::max( mnAlphaState,
                            mpChild->getAlphaState() ) :
                mnAlphaState;
        }

        State::StateId ShapeAttributeLayer::getPositionState() const
         {
            return haveChild() ?
                ::std::max( mnPositionState,
                            mpChild->getPositionState() ) :
                mnPositionState;
        }

        State::StateId ShapeAttributeLayer::getContentState() const
        {
            return haveChild() ?
                ::std::max( mnContentState,
                            mpChild->getContentState() ) :
                mnContentState;
        }

    }
}
