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
#include <sal/log.hxx>

#include <animationfactory.hxx>
#include <attributemap.hxx>

#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>


using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {
        namespace
        {
            // attention, there is a similar implementation of Animation in
            // transitions/transitionfactory.cxx

            template< typename ValueT > class TupleAnimation : public PairAnimation
            {
            public:
                TupleAnimation( const ShapeManagerSharedPtr&        rShapeManager,
                                int                                 nFlags,
                                bool         (ShapeAttributeLayer::*pIs1stValid)() const,
                                bool         (ShapeAttributeLayer::*pIs2ndValid)() const,
                                const ValueT&                       rDefaultValue,
                                const ::basegfx::B2DSize&           rReferenceSize,
                                double       (ShapeAttributeLayer::*pGet1stValue)() const,
                                double       (ShapeAttributeLayer::*pGet2ndValue)() const,
                                void         (ShapeAttributeLayer::*pSetValue)( const ValueT& ) ) :
                    mpShape(),
                    mpAttrLayer(),
                    mpShapeManager( rShapeManager ),
                    mpIs1stValidFunc(pIs1stValid),
                    mpIs2ndValidFunc(pIs2ndValid),
                    mpGet1stValueFunc(pGet1stValue),
                    mpGet2ndValueFunc(pGet2ndValue),
                    mpSetValueFunc(pSetValue),
                    mnFlags( nFlags ),
                    maReferenceSize( rReferenceSize ),
                    maDefaultValue( rDefaultValue ),
                    mbAnimationStarted( false )
                {
                    ENSURE_OR_THROW( rShapeManager,
                                      "TupleAnimation::TupleAnimation(): Invalid ShapeManager" );
                    ENSURE_OR_THROW( pIs1stValid && pIs2ndValid && pGet1stValue && pGet2ndValue && pSetValue,
                                      "TupleAnimation::TupleAnimation(): One of the method pointers is NULL" );
                }

                virtual ~TupleAnimation() override
                {
                    end_();
                }

                // Animation interface

                virtual void prefetch( const AnimatableShapeSharedPtr&,
                                       const ShapeAttributeLayerSharedPtr& ) override
                {}

                virtual void start( const AnimatableShapeSharedPtr&     rShape,
                                    const ShapeAttributeLayerSharedPtr& rAttrLayer ) override
                {
                    OSL_ENSURE( !mpShape,
                                "TupleAnimation::start(): Shape already set" );
                    OSL_ENSURE( !mpAttrLayer,
                                "TupleAnimation::start(): Attribute layer already set" );

                    mpShape = rShape;
                    mpAttrLayer = rAttrLayer;

                    ENSURE_OR_THROW( rShape,
                                      "TupleAnimation::start(): Invalid shape" );
                    ENSURE_OR_THROW( rAttrLayer,
                                      "TupleAnimation::start(): Invalid attribute layer" );

                    if( !mbAnimationStarted )
                    {
                        mbAnimationStarted = true;

                        if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                            mpShapeManager->enterAnimationMode( mpShape );
                    }
                }

                virtual void end() override { end_(); }
                void end_()
                {
                    if( mbAnimationStarted )
                    {
                        mbAnimationStarted = false;

                        if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                            mpShapeManager->leaveAnimationMode( mpShape );

                        if( mpShape->isContentChanged() )
                            mpShapeManager->notifyShapeUpdate( mpShape );
                    }
                }

                // PairAnimation interface


                virtual bool operator()( const ::basegfx::B2DTuple& rValue ) override
                {
                    ENSURE_OR_RETURN_FALSE( mpAttrLayer && mpShape,
                                       "TupleAnimation::operator(): Invalid ShapeAttributeLayer" );

                    ValueT aValue( rValue.getX(),
                                   rValue.getY() );

                    // Activities get values from the expression parser,
                    // which returns _relative_ sizes/positions.
                    // Convert back relative to reference coordinate system
                    aValue *= maReferenceSize;

                    ((*mpAttrLayer).*mpSetValueFunc)( aValue );

                    if( mpShape->isContentChanged() )
                        mpShapeManager->notifyShapeUpdate( mpShape );

                    return true;
                }

                virtual ::basegfx::B2DTuple getUnderlyingValue() const override
                {
                    ENSURE_OR_THROW( mpAttrLayer,
                                      "TupleAnimation::getUnderlyingValue(): Invalid ShapeAttributeLayer" );

                    ::basegfx::B2DTuple aRetVal;

                    // deviated from the (*shared_ptr).*mpFuncPtr
                    // notation here, since gcc does not seem to parse
                    // that as a member function call anymore.
                    aRetVal.setX( (mpAttrLayer.get()->*mpIs1stValidFunc)() ?
                                  (mpAttrLayer.get()->*mpGet1stValueFunc)() :
                                  maDefaultValue.getX() );
                    aRetVal.setY( (mpAttrLayer.get()->*mpIs2ndValidFunc)() ?
                                  (mpAttrLayer.get()->*mpGet2ndValueFunc)() :
                                  maDefaultValue.getY() );

                    // Activities get values from the expression
                    // parser, which returns _relative_
                    // sizes/positions.  Convert start value to the
                    // same coordinate space (i.e. relative to given
                    // reference size).
                    aRetVal /= maReferenceSize;

                    return aRetVal;
                }

            private:
                AnimatableShapeSharedPtr           mpShape;
                ShapeAttributeLayerSharedPtr       mpAttrLayer;
                ShapeManagerSharedPtr              mpShapeManager;
                bool        (ShapeAttributeLayer::*mpIs1stValidFunc)() const;
                bool        (ShapeAttributeLayer::*mpIs2ndValidFunc)() const;
                double      (ShapeAttributeLayer::*mpGet1stValueFunc)() const;
                double      (ShapeAttributeLayer::*mpGet2ndValueFunc)() const;
                void        (ShapeAttributeLayer::*mpSetValueFunc)( const ValueT& );

                const int                          mnFlags;

                const ::basegfx::B2DSize           maReferenceSize;
                const ValueT                       maDefaultValue;
                bool                               mbAnimationStarted;
            };


            class PathAnimation : public NumberAnimation
            {
            public:
                PathAnimation( const OUString&       rSVGDPath,
                               sal_Int16                    nAdditive,
                               const ShapeManagerSharedPtr& rShapeManager,
                               const ::basegfx::B2DVector&  rSlideSize,
                               int                          nFlags ) :
                    maPathPoly(),
                    mpShape(),
                    mpAttrLayer(),
                    mpShapeManager( rShapeManager ),
                    maPageSize( rSlideSize ),
                    maShapeOrig(),
                    mnFlags( nFlags ),
                    mbAnimationStarted( false ),
                    mnAdditive( nAdditive )
                {
                    ENSURE_OR_THROW( rShapeManager,
                                      "PathAnimation::PathAnimation(): Invalid ShapeManager" );

                    ::basegfx::B2DPolyPolygon aPolyPoly;

                    ENSURE_OR_THROW( ::basegfx::utils::importFromSvgD( aPolyPoly, rSVGDPath, false, nullptr ),
                                      "PathAnimation::PathAnimation(): failed to parse SVG:d path" );
                    ENSURE_OR_THROW( aPolyPoly.count() == 1,
                                      "PathAnimation::PathAnimation(): motion path consists of multiple/zero polygon(s)" );

                    // TODO(F2): Since getPositionRelative() currently
                    // cannot handle beziers, have to subdivide.
                    // AW: Should be no longer necessary; getPositionRelative is now bezier-safe
                    maPathPoly = ::basegfx::utils::adaptiveSubdivideByAngle(aPolyPoly.getB2DPolygon(0) );
                }

                virtual ~PathAnimation() override
                {
                    end_();
                }

                // Animation interface

                virtual void prefetch( const AnimatableShapeSharedPtr&,
                                       const ShapeAttributeLayerSharedPtr& ) override
                {}

                virtual void start( const AnimatableShapeSharedPtr&     rShape,
                                    const ShapeAttributeLayerSharedPtr& rAttrLayer ) override
                {
                    OSL_ENSURE( !mpShape,
                                "PathAnimation::start(): Shape already set" );
                    OSL_ENSURE( !mpAttrLayer,
                                "PathAnimation::start(): Attribute layer already set" );

                    mpShape = rShape;
                    mpAttrLayer = rAttrLayer;

                    ENSURE_OR_THROW( rShape,
                                      "PathAnimation::start(): Invalid shape" );
                    ENSURE_OR_THROW( rAttrLayer,
                                      "PathAnimation::start(): Invalid attribute layer" );

                    // TODO(F1): Check whether _shape_ bounds are correct here.
                    // Theoretically, our AttrLayer is way down the stack, and
                    // we only have to consider _that_ value, not the one from
                    // the top of the stack as returned by Shape::getBounds()
                    if( mnAdditive == animations::AnimationAdditiveMode::SUM )
                        maShapeOrig = mpShape->getBounds().getCenter();
                    else
                        maShapeOrig = mpShape->getDomBounds().getCenter();

                    if( !mbAnimationStarted )
                    {
                        mbAnimationStarted = true;

                        if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                            mpShapeManager->enterAnimationMode( mpShape );
                    }
                }

                virtual void end() override { end_(); }
                void end_()
                {
                    if( mbAnimationStarted )
                    {
                        mbAnimationStarted = false;

                        if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                            mpShapeManager->leaveAnimationMode( mpShape );

                        if( mpShape->isContentChanged() )
                            mpShapeManager->notifyShapeUpdate( mpShape );
                    }
                }

                // NumberAnimation interface


                virtual bool operator()( double nValue ) override
                {
                    ENSURE_OR_RETURN_FALSE( mpAttrLayer && mpShape,
                                       "PathAnimation::operator(): Invalid ShapeAttributeLayer" );

                    ::basegfx::B2DPoint rOutPos = ::basegfx::utils::getPositionRelative( maPathPoly,
                                                                                         nValue );

                    // TODO(F1): Determine whether the path is
                    // absolute, or shape-relative.

                    // interpret path as page-relative. Scale up with page size
                    rOutPos *= maPageSize;

                    // TODO(F1): Determine whether the path origin is
                    // absolute, or shape-relative.

                    // interpret path as shape-originated. Offset to shape position

                    rOutPos += maShapeOrig;

                    mpAttrLayer->setPosition( rOutPos );

                    if( mpShape->isContentChanged() )
                        mpShapeManager->notifyShapeUpdate( mpShape );

                    return true;
                }

                virtual double getUnderlyingValue() const override
                {
                    ENSURE_OR_THROW( mpAttrLayer,
                                      "PathAnimation::getUnderlyingValue(): Invalid ShapeAttributeLayer" );

                    return 0.0; // though this should be used in concert with
                                // ActivitiesFactory::createSimpleActivity, better
                                // explicitly name our start value.
                                // Permissible range for operator() above is [0,1]
                }

            private:
                ::basegfx::B2DPolygon              maPathPoly;
                AnimatableShapeSharedPtr           mpShape;
                ShapeAttributeLayerSharedPtr       mpAttrLayer;
                ShapeManagerSharedPtr              mpShapeManager;
                const ::basegfx::B2DSize           maPageSize;
                ::basegfx::B2DPoint                maShapeOrig;
                const int                          mnFlags;
                bool                               mbAnimationStarted;
                sal_Int16 const                    mnAdditive;
            };


            /** GenericAnimation template

                This template makes heavy use of SFINAE, only one of
                the operator()() methods will compile for each of the
                base classes.

                Note that we omit the virtual keyword on the
                operator()() overrides and getUnderlyingValue() methods on
                purpose; those that actually do override baseclass
                virtual methods inherit the property, and the others
                won't increase our vtable. What's more, having all
                those methods in the vtable actually creates POIs for
                them, which breaks the whole SFINAE concept (IOW, this
                template won't compile any longer).

                @tpl AnimationBase
                Type of animation to generate (determines the
                interface GenericAnimation will implement). Must be
                one of NumberAnimation, ColorAnimation,
                StringAnimation, PairAnimation or BoolAnimation.

                @tpl ModifierFunctor
                Type of a functor object, which can optionally be used to
                modify the getter/setter values.
             */
            template< typename AnimationBase, typename ModifierFunctor > class GenericAnimation : public AnimationBase
            {
            public:
                typedef typename AnimationBase::ValueType ValueT;

                /** Create generic animation

                    @param pIsValid
                    Function pointer to one of the is*Valid
                    methods. Used to either take the given getter
                    method, or the given default value for the start value.

                    @param rDefaultValue
                    Default value, to take as the start value if
                    is*Valid returns false.

                    @param pGetValue
                    Getter method, to fetch start value if valid.

                    @param pSetValue
                    Setter method. This one puts the current animation
                    value to the ShapeAttributeLayer.

                    @param rGetterModifier
                    Modifies up values retrieved from the pGetValue method.
                    Must provide operator()( const ValueT& ) method.

                    @param rSetterModifier
                    Modifies up values before passing them to the pSetValue method.
                    Must provide operator()( const ValueT& ) method.
                 */
                GenericAnimation( const ShapeManagerSharedPtr&          rShapeManager,
                                  int                                   nFlags,
                                  bool           (ShapeAttributeLayer::*pIsValid)() const,
                                  const ValueT&                         rDefaultValue,
                                  ValueT         (ShapeAttributeLayer::*pGetValue)() const,
                                  void           (ShapeAttributeLayer::*pSetValue)( const ValueT& ),
                                  const ModifierFunctor&                rGetterModifier,
                                  const ModifierFunctor&                rSetterModifier ) :
                    mpShape(),
                    mpAttrLayer(),
                    mpShapeManager( rShapeManager ),
                    mpIsValidFunc(pIsValid),
                    mpGetValueFunc(pGetValue),
                    mpSetValueFunc(pSetValue),
                    maGetterModifier( rGetterModifier ),
                    maSetterModifier( rSetterModifier ),
                    mnFlags( nFlags ),
                    maDefaultValue(rDefaultValue),
                    mbAnimationStarted( false )
                {
                    ENSURE_OR_THROW( rShapeManager,
                                      "GenericAnimation::GenericAnimation(): Invalid ShapeManager" );
                    ENSURE_OR_THROW( pIsValid && pGetValue && pSetValue,
                                      "GenericAnimation::GenericAnimation(): One of the method pointers is NULL" );
                }

                ~GenericAnimation()
                {
                    end();
                }

                // Animation interface

                virtual void prefetch( const AnimatableShapeSharedPtr&,
                                       const ShapeAttributeLayerSharedPtr& )
                {}

                virtual void start( const AnimatableShapeSharedPtr&     rShape,
                                    const ShapeAttributeLayerSharedPtr& rAttrLayer )
                {
                    OSL_ENSURE( !mpShape,
                                "GenericAnimation::start(): Shape already set" );
                    OSL_ENSURE( !mpAttrLayer,
                                "GenericAnimation::start(): Attribute layer already set" );

                    mpShape = rShape;
                    mpAttrLayer = rAttrLayer;

                    ENSURE_OR_THROW( rShape,
                                      "GenericAnimation::start(): Invalid shape" );
                    ENSURE_OR_THROW( rAttrLayer,
                                      "GenericAnimation::start(): Invalid attribute layer" );

                    // only start animation once per repeated start() call,
                    // and only if sprites should be used for display
                    if( !mbAnimationStarted )
                    {
                        mbAnimationStarted = true;

                        if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                            mpShapeManager->enterAnimationMode( mpShape );
                    }
                }

                void end()
                {
                    // TODO(Q2): Factor out common code (most
                    // prominently start() and end()) into base class

                    // only stop animation once per repeated end() call,
                    // and only if sprites are used for display
                    if( !mbAnimationStarted )
                        return;

                    mbAnimationStarted = false;

                    if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                        mpShapeManager->leaveAnimationMode( mpShape );

                    // Attention, this notifyShapeUpdate() is
                    // somewhat delicate here. Calling it
                    // unconditional (i.e. not guarded by
                    // mbAnimationStarted) will lead to shapes
                    // snapping back to their original state just
                    // before the slide ends. Not calling it at
                    // all might swallow final animation
                    // states. The current implementation relies
                    // on the fact that end() is either called by
                    // the Activity (then, the last animation
                    // state has been set, and corresponds to the
                    // shape's hold state), or by the animation
                    // node (then, it's a forced end, and we
                    // _have_ to snap back).

                    // To reiterate: normally, we're called from
                    // the Activity first, thus the
                    // notifyShapeUpdate() below will update to
                    // the last activity value.

                    // force shape update, activity might have changed
                    // state in the last round.
                    if( mpShape->isContentChanged() )
                        mpShapeManager->notifyShapeUpdate( mpShape );
                }

                // Derived Animation interface


                /** For by-reference interfaces (B2DTuple, OUString)
                 */
                bool operator()( const ValueT& x )
                {
                    ENSURE_OR_RETURN_FALSE( mpAttrLayer && mpShape,
                                       "GenericAnimation::operator(): Invalid ShapeAttributeLayer" );

                    ((*mpAttrLayer).*mpSetValueFunc)( maSetterModifier( x ) );

                    if( mpShape->isContentChanged() )
                        mpShapeManager->notifyShapeUpdate( mpShape );

                    return true;
                }

                /** For by-value interfaces (bool, double)
                 */
                bool operator()( ValueT x )
                {
                    ENSURE_OR_RETURN_FALSE( mpAttrLayer && mpShape,
                                       "GenericAnimation::operator(): Invalid ShapeAttributeLayer" );

                    ((*mpAttrLayer).*mpSetValueFunc)( maSetterModifier( x ) );

                    if( mpShape->isContentChanged() )
                        mpShapeManager->notifyShapeUpdate( mpShape );

                    return true;
                }

                ValueT getUnderlyingValue() const
                {
                    ENSURE_OR_THROW( mpAttrLayer,
                                      "GenericAnimation::getUnderlyingValue(): Invalid ShapeAttributeLayer" );

                    // deviated from the (*shared_ptr).*mpFuncPtr
                    // notation here, since gcc does not seem to parse
                    // that as a member function call anymore.
                    if( (mpAttrLayer.get()->*mpIsValidFunc)() )
                        return maGetterModifier( ((*mpAttrLayer).*mpGetValueFunc)() );
                    else
                        return maDefaultValue;
                }

            private:
                AnimatableShapeSharedPtr           mpShape;
                ShapeAttributeLayerSharedPtr       mpAttrLayer;
                ShapeManagerSharedPtr              mpShapeManager;
                bool        (ShapeAttributeLayer::*mpIsValidFunc)() const;
                ValueT      (ShapeAttributeLayer::*mpGetValueFunc)() const;
                void        (ShapeAttributeLayer::*mpSetValueFunc)( const ValueT& );

                ModifierFunctor                    maGetterModifier;
                ModifierFunctor                    maSetterModifier;

                const int                          mnFlags;

                const ValueT                       maDefaultValue;
                bool                               mbAnimationStarted;
            };

            //Current c++0x draft (apparently) has std::identity, but not operator()
            template<typename T> struct SGI_identity
            {
                T& operator()(T& x) const { return x; }
                const T& operator()(const T& x) const { return x; }
            };

            /** Function template wrapper around GenericAnimation template

                @tpl AnimationBase
                Type of animation to generate (determines the
                interface GenericAnimation will implement).
             */
            template< typename AnimationBase > ::std::shared_ptr< AnimationBase >
                makeGenericAnimation( const ShapeManagerSharedPtr&                             rShapeManager,
                                      int                                                      nFlags,
                                      bool                              (ShapeAttributeLayer::*pIsValid)() const,
                                      const typename AnimationBase::ValueType&                 rDefaultValue,
                                      typename AnimationBase::ValueType (ShapeAttributeLayer::*pGetValue)() const,
                                      void                              (ShapeAttributeLayer::*pSetValue)( const typename AnimationBase::ValueType& ) )
            {
                return ::std::shared_ptr< AnimationBase >(
                    new GenericAnimation< AnimationBase,
                                          SGI_identity< typename AnimationBase::ValueType > >(
                                              rShapeManager,
                                              nFlags,
                                              pIsValid,
                                              rDefaultValue,
                                              pGetValue,
                                              pSetValue,
                                              // no modification necessary, use identity functor here
                                              SGI_identity< typename AnimationBase::ValueType >(),
                                              SGI_identity< typename AnimationBase::ValueType >() ) );
            }

            class Scaler
            {
            public:
                explicit Scaler( double nScale ) :
                    mnScale( nScale )
                {
                }

                double operator()( double nVal ) const
                {
                    return mnScale * nVal;
                }

            private:
                double const mnScale;
            };

            /** Overload for NumberAnimations which need scaling (width,height,x,y currently)
             */
            NumberAnimationSharedPtr makeGenericAnimation( const ShapeManagerSharedPtr&                             rShapeManager,
                                                           int                                                      nFlags,
                                                           bool                              (ShapeAttributeLayer::*pIsValid)() const,
                                                           double                                                   nDefaultValue,
                                                           double                            (ShapeAttributeLayer::*pGetValue)() const,
                                                           void                              (ShapeAttributeLayer::*pSetValue)( const double& ),
                                                           double                                                   nScaleValue )
            {
                return NumberAnimationSharedPtr(
                    new GenericAnimation< NumberAnimation, Scaler >( rShapeManager,
                                                                     nFlags,
                                                                     pIsValid,
                                                                     nDefaultValue / nScaleValue,
                                                                     pGetValue,
                                                                     pSetValue,
                                                                     Scaler( 1.0/nScaleValue ),
                                                                     Scaler( nScaleValue ) ) );
            }


            uno::Any getShapeDefault( const AnimatableShapeSharedPtr&   rShape,
                                      const OUString&            rPropertyName )
            {
                uno::Reference< drawing::XShape > xShape( rShape->getXShape() );

                if( !xShape.is() )
                    return uno::Any(); // no regular shape, no defaults available


                // extract relevant value from XShape's PropertySet
                uno::Reference< beans::XPropertySet > xPropSet( xShape,
                                                                uno::UNO_QUERY );

                ENSURE_OR_THROW( xPropSet.is(),
                                  "getShapeDefault(): Cannot query property set from shape" );

                return xPropSet->getPropertyValue( rPropertyName );
            }

            template< typename ValueType > ValueType getDefault( const AnimatableShapeSharedPtr&    rShape,
                                                                 const OUString&             rPropertyName )
            {
                const uno::Any& rAny( getShapeDefault( rShape,
                                                       rPropertyName ) );

                if( !rAny.hasValue() )
                {
                    SAL_WARN("slideshow", "getDefault(): cannot get shape property " <<  rPropertyName );
                    return ValueType();
                }
                else
                {
                    ValueType aValue = ValueType();

                    if( !(rAny >>= aValue) )
                    {
                        SAL_WARN("slideshow", "getDefault(): cannot extract shape property " << rPropertyName);
                        return ValueType();
                    }

                    return aValue;
                }
            }

            template<> RGBColor getDefault< RGBColor >( const AnimatableShapeSharedPtr& rShape,
                                                        const OUString&          rPropertyName )
            {
                const uno::Any& rAny( getShapeDefault( rShape,
                                                       rPropertyName ) );

                if( !rAny.hasValue() )
                {
                    SAL_WARN("slideshow", "getDefault(): cannot get shape color property " << rPropertyName);
                    return RGBColor();
                }
                else
                {
                    sal_Int32 nValue = 0;

                    if( !(rAny >>= nValue) )
                    {
                        SAL_INFO("slideshow", "getDefault(): cannot extract shape color property " << rPropertyName);
                        return RGBColor();
                    }

                    // convert from 0xAARRGGBB API color to 0xRRGGBB00
                    // canvas color
                    return RGBColor( (nValue << 8U) & 0xFFFFFF00U );
                }
            }
        }

        AnimationFactory::AttributeClass AnimationFactory::classifyAttributeName( const OUString& rAttrName )
        {
            // ATTENTION: When changing this map, also the create*PropertyAnimation() methods must
            // be checked and possibly adapted in their switch statements

            // TODO(Q2): Since this map must be coherent with the various switch statements
            // in the create*PropertyAnimation methods, try to unify into a single method or table
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                case AttributeType::Invalid:
                    return CLASS_UNKNOWN_PROPERTY;

                case AttributeType::CharColor:
                case AttributeType::Color:
                case AttributeType::DimColor:
                case AttributeType::FillColor:
                case AttributeType::LineColor:
                    return CLASS_COLOR_PROPERTY;

                case AttributeType::CharFontName:
                    return CLASS_STRING_PROPERTY;

                case AttributeType::Visibility:
                    return CLASS_BOOL_PROPERTY;

                case AttributeType::CharHeight:
                case AttributeType::CharWeight:
                case AttributeType::Height:
                case AttributeType::Opacity:
                case AttributeType::Rotate:
                case AttributeType::SkewX:
                case AttributeType::SkewY:
                case AttributeType::Width:
                case AttributeType::PosX:
                case AttributeType::PosY:
                    return CLASS_NUMBER_PROPERTY;

                case AttributeType::CharUnderline:
                case AttributeType::FillStyle:
                case AttributeType::LineStyle:
                case AttributeType::CharPosture:
                    return CLASS_ENUM_PROPERTY;
            }
        }

        NumberAnimationSharedPtr AnimationFactory::createNumberPropertyAnimation( const OUString&                rAttrName,
                                                                                  const AnimatableShapeSharedPtr&       rShape,
                                                                                  const ShapeManagerSharedPtr&          rShapeManager,
                                                                                  const ::basegfx::B2DVector&           rSlideSize,
                                                                                  int                                   nFlags )
        {
            // ATTENTION: When changing this map, also the classifyAttributeName() method must
            // be checked and possibly adapted in their switch statement
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                case AttributeType::Invalid:
                    ENSURE_OR_THROW( false,
                                      "AnimationFactory::createNumberPropertyAnimation(): Unknown attribute" );
                    break;

                case AttributeType::CharColor:
                case AttributeType::CharFontName:
                case AttributeType::CharPosture:
                case AttributeType::CharUnderline:
                case AttributeType::Color:
                case AttributeType::DimColor:
                case AttributeType::FillColor:
                case AttributeType::FillStyle:
                case AttributeType::LineColor:
                case AttributeType::LineStyle:
                case AttributeType::Visibility:
                    ENSURE_OR_THROW( false,
                                      "AnimationFactory::createNumberPropertyAnimation(): Attribute type mismatch" );
                    break;

                case AttributeType::CharHeight:
                    return makeGenericAnimation<NumberAnimation>( rShapeManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isCharScaleValid,
                                                                  1.0, // CharHeight is a relative attribute, thus
                                                                         // default is 1.0
                                                                  &ShapeAttributeLayer::getCharScale,
                                                                  &ShapeAttributeLayer::setCharScale );

                case AttributeType::CharWeight:
                    return makeGenericAnimation<NumberAnimation>( rShapeManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isCharWeightValid,
                                                                  getDefault<double>( rShape, rAttrName ),
                                                                  &ShapeAttributeLayer::getCharWeight,
                                                                  &ShapeAttributeLayer::setCharWeight );

                case AttributeType::Height:
                    return makeGenericAnimation( rShapeManager,
                                                 nFlags,
                                                 &ShapeAttributeLayer::isHeightValid,
                                                 // TODO(F1): Check whether _shape_ bounds are correct here.
                                                 // Theoretically, our AttrLayer is way down the stack, and
                                                 // we only have to consider _that_ value, not the one from
                                                 // the top of the stack as returned by Shape::getBounds()
                                                 rShape->getBounds().getHeight(),
                                                 &ShapeAttributeLayer::getHeight,
                                                 &ShapeAttributeLayer::setHeight,
                                                 // convert expression parser value from relative page size
                                                 rSlideSize.getY() );

                case AttributeType::Opacity:
                    return makeGenericAnimation<NumberAnimation>( rShapeManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isAlphaValid,
                                                                  // TODO(F1): Provide shape default here (FillTransparency?)
                                                                  1.0,
                                                                  &ShapeAttributeLayer::getAlpha,
                                                                  &ShapeAttributeLayer::setAlpha );

                case AttributeType::Rotate:
                    return makeGenericAnimation<NumberAnimation>( rShapeManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isRotationAngleValid,
                                                                  // NOTE: Since we paint the shape as-is from metafile,
                                                                  // rotation angle is always 0.0, even for rotated shapes
                                                                  0.0,
                                                                  &ShapeAttributeLayer::getRotationAngle,
                                                                  &ShapeAttributeLayer::setRotationAngle );

                case AttributeType::SkewX:
                    return makeGenericAnimation<NumberAnimation>( rShapeManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isShearXAngleValid,
                                                                  // TODO(F1): Is there any shape property for skew?
                                                                  0.0,
                                                                  &ShapeAttributeLayer::getShearXAngle,
                                                                  &ShapeAttributeLayer::setShearXAngle );

                case AttributeType::SkewY:
                    return makeGenericAnimation<NumberAnimation>( rShapeManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isShearYAngleValid,
                                                                  // TODO(F1): Is there any shape property for skew?
                                                                  0.0,
                                                                  &ShapeAttributeLayer::getShearYAngle,
                                                                  &ShapeAttributeLayer::setShearYAngle );

                case AttributeType::Width:
                    return makeGenericAnimation( rShapeManager,
                                                 nFlags,
                                                 &ShapeAttributeLayer::isWidthValid,
                                                 // TODO(F1): Check whether _shape_ bounds are correct here.
                                                 // Theoretically, our AttrLayer is way down the stack, and
                                                 // we only have to consider _that_ value, not the one from
                                                 // the top of the stack as returned by Shape::getBounds()
                                                 rShape->getBounds().getWidth(),
                                                 &ShapeAttributeLayer::getWidth,
                                                 &ShapeAttributeLayer::setWidth,
                                                 // convert expression parser value from relative page size
                                                 rSlideSize.getX() );

                case AttributeType::PosX:
                    return makeGenericAnimation( rShapeManager,
                                                 nFlags,
                                                 &ShapeAttributeLayer::isPosXValid,
                                                 // TODO(F1): Check whether _shape_ bounds are correct here.
                                                 // Theoretically, our AttrLayer is way down the stack, and
                                                 // we only have to consider _that_ value, not the one from
                                                 // the top of the stack as returned by Shape::getBounds()
                                                 rShape->getBounds().getCenterX(),
                                                 &ShapeAttributeLayer::getPosX,
                                                 &ShapeAttributeLayer::setPosX,
                                                 // convert expression parser value from relative page size
                                                 rSlideSize.getX() );

                case AttributeType::PosY:
                    return makeGenericAnimation( rShapeManager,
                                                 nFlags,
                                                 &ShapeAttributeLayer::isPosYValid,
                                                 // TODO(F1): Check whether _shape_ bounds are correct here.
                                                 // Theoretically, our AttrLayer is way down the stack, and
                                                 // we only have to consider _that_ value, not the one from
                                                 // the top of the stack as returned by Shape::getBounds()
                                                 rShape->getBounds().getCenterY(),
                                                 &ShapeAttributeLayer::getPosY,
                                                 &ShapeAttributeLayer::setPosY,
                                                 // convert expression parser value from relative page size
                                                 rSlideSize.getY() );
            }

            return NumberAnimationSharedPtr();
        }

        EnumAnimationSharedPtr AnimationFactory::createEnumPropertyAnimation( const OUString&                rAttrName,
                                                                              const AnimatableShapeSharedPtr&       rShape,
                                                                              const ShapeManagerSharedPtr&          rShapeManager,
                                                                              const ::basegfx::B2DVector&           /*rSlideSize*/,
                                                                              int                                   nFlags )
        {
            // ATTENTION: When changing this map, also the classifyAttributeName() method must
            // be checked and possibly adapted in their switch statement
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                case AttributeType::Invalid:
                    ENSURE_OR_THROW( false,
                                      "AnimationFactory::createEnumPropertyAnimation(): Unknown attribute" );
                    break;

                case AttributeType::CharColor:
                case AttributeType::CharFontName:
                case AttributeType::Color:
                case AttributeType::DimColor:
                case AttributeType::FillColor:
                case AttributeType::LineColor:
                case AttributeType::Visibility:
                case AttributeType::CharHeight:
                case AttributeType::CharWeight:
                case AttributeType::Height:
                case AttributeType::Opacity:
                case AttributeType::Rotate:
                case AttributeType::SkewX:
                case AttributeType::SkewY:
                case AttributeType::Width:
                case AttributeType::PosX:
                case AttributeType::PosY:
                    ENSURE_OR_THROW( false,
                                      "AnimationFactory::createEnumPropertyAnimation(): Attribute type mismatch" );
                    break;


                case AttributeType::FillStyle:
                    return makeGenericAnimation<EnumAnimation>( rShapeManager,
                                                                nFlags,
                                                                &ShapeAttributeLayer::isFillStyleValid,
                                                                sal::static_int_cast<sal_Int16>(
                                                                    getDefault<drawing::FillStyle>( rShape, rAttrName )),
                                                                &ShapeAttributeLayer::getFillStyle,
                                                                &ShapeAttributeLayer::setFillStyle );

                case AttributeType::LineStyle:
                    return makeGenericAnimation<EnumAnimation>( rShapeManager,
                                                                nFlags,
                                                                &ShapeAttributeLayer::isLineStyleValid,
                                                                sal::static_int_cast<sal_Int16>(
                                                                    getDefault<drawing::LineStyle>( rShape, rAttrName )),
                                                                &ShapeAttributeLayer::getLineStyle,
                                                                &ShapeAttributeLayer::setLineStyle );

                case AttributeType::CharPosture:
                    return makeGenericAnimation<EnumAnimation>( rShapeManager,
                                                                nFlags,
                                                                &ShapeAttributeLayer::isCharPostureValid,
                                                                sal::static_int_cast<sal_Int16>(
                                                                    getDefault<awt::FontSlant>( rShape, rAttrName )),
                                                                &ShapeAttributeLayer::getCharPosture,
                                                                &ShapeAttributeLayer::setCharPosture );

                case AttributeType::CharUnderline:
                    return makeGenericAnimation<EnumAnimation>( rShapeManager,
                                                                nFlags,
                                                                &ShapeAttributeLayer::isUnderlineModeValid,
                                                                getDefault<sal_Int16>( rShape, rAttrName ),
                                                                &ShapeAttributeLayer::getUnderlineMode,
                                                                &ShapeAttributeLayer::setUnderlineMode );
            }

            return EnumAnimationSharedPtr();
        }

        ColorAnimationSharedPtr AnimationFactory::createColorPropertyAnimation( const OUString&              rAttrName,
                                                                                const AnimatableShapeSharedPtr&     rShape,
                                                                                const ShapeManagerSharedPtr&        rShapeManager,
                                                                                const ::basegfx::B2DVector&         /*rSlideSize*/,
                                                                                int                                 nFlags )
        {
            // ATTENTION: When changing this map, also the classifyAttributeName() method must
            // be checked and possibly adapted in their switch statement
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                case AttributeType::Invalid:
                    ENSURE_OR_THROW( false,
                                      "AnimationFactory::createColorPropertyAnimation(): Unknown attribute" );
                    break;

                case AttributeType::CharFontName:
                case AttributeType::CharHeight:
                case AttributeType::CharPosture:
                case AttributeType::CharUnderline:
                case AttributeType::CharWeight:
                case AttributeType::FillStyle:
                case AttributeType::Height:
                case AttributeType::LineStyle:
                case AttributeType::Opacity:
                case AttributeType::Rotate:
                case AttributeType::SkewX:
                case AttributeType::SkewY:
                case AttributeType::Visibility:
                case AttributeType::Width:
                case AttributeType::PosX:
                case AttributeType::PosY:
                    ENSURE_OR_THROW( false,
                                      "AnimationFactory::createColorPropertyAnimation(): Attribute type mismatch" );
                    break;

                case AttributeType::CharColor:
                    return makeGenericAnimation<ColorAnimation>( rShapeManager,
                                                                 nFlags,
                                                                 &ShapeAttributeLayer::isCharColorValid,
                                                                 getDefault<RGBColor>( rShape, rAttrName ),
                                                                 &ShapeAttributeLayer::getCharColor,
                                                                 &ShapeAttributeLayer::setCharColor );

                case AttributeType::Color:
                    // TODO(F2): This is just mapped to fill color to make it work
                    return makeGenericAnimation<ColorAnimation>( rShapeManager,
                                                                 nFlags,
                                                                 &ShapeAttributeLayer::isFillColorValid,
                                                                 getDefault<RGBColor>( rShape, rAttrName ),
                                                                 &ShapeAttributeLayer::getFillColor,
                                                                 &ShapeAttributeLayer::setFillColor );

                case AttributeType::DimColor:
                    return makeGenericAnimation<ColorAnimation>( rShapeManager,
                                                                 nFlags,
                                                                 &ShapeAttributeLayer::isDimColorValid,
                                                                 getDefault<RGBColor>( rShape, rAttrName ),
                                                                 &ShapeAttributeLayer::getDimColor,
                                                                 &ShapeAttributeLayer::setDimColor );

                case AttributeType::FillColor:
                    return makeGenericAnimation<ColorAnimation>( rShapeManager,
                                                                 nFlags,
                                                                 &ShapeAttributeLayer::isFillColorValid,
                                                                 getDefault<RGBColor>( rShape, rAttrName ),
                                                                 &ShapeAttributeLayer::getFillColor,
                                                                 &ShapeAttributeLayer::setFillColor );

                case AttributeType::LineColor:
                    return makeGenericAnimation<ColorAnimation>( rShapeManager,
                                                                 nFlags,
                                                                 &ShapeAttributeLayer::isLineColorValid,
                                                                 getDefault<RGBColor>( rShape, rAttrName ),
                                                                 &ShapeAttributeLayer::getLineColor,
                                                                 &ShapeAttributeLayer::setLineColor );
            }

            return ColorAnimationSharedPtr();
        }

        PairAnimationSharedPtr AnimationFactory::createPairPropertyAnimation( const AnimatableShapeSharedPtr&       rShape,
                                                                              const ShapeManagerSharedPtr&          rShapeManager,
                                                                              const ::basegfx::B2DVector&           rSlideSize,
                                                                              sal_Int16                             nTransformType,
                                                                              int                                   nFlags )
        {
            const ::basegfx::B2DRectangle& rBounds( rShape->getBounds() );

            switch( nTransformType )
            {
                case animations::AnimationTransformType::SCALE:
                    return PairAnimationSharedPtr(
                        new TupleAnimation< ::basegfx::B2DSize >(
                            rShapeManager,
                            nFlags,
                            &ShapeAttributeLayer::isWidthValid,
                            &ShapeAttributeLayer::isHeightValid,
                            // TODO(F1): Check whether _shape_ bounds are correct here.
                            // Theoretically, our AttrLayer is way down the stack, and
                            // we only have to consider _that_ value, not the one from
                            // the top of the stack as returned by Shape::getBounds()
                            rBounds.getRange(),
                            rBounds.getRange(),
                            &ShapeAttributeLayer::getWidth,
                            &ShapeAttributeLayer::getHeight,
                            &ShapeAttributeLayer::setSize ) );

                case animations::AnimationTransformType::TRANSLATE:
                    return PairAnimationSharedPtr(
                        new TupleAnimation< ::basegfx::B2DPoint >(
                            rShapeManager,
                            nFlags,
                            &ShapeAttributeLayer::isPosXValid,
                            &ShapeAttributeLayer::isPosYValid,
                            // TODO(F1): Check whether _shape_ bounds are correct here.
                            // Theoretically, our AttrLayer is way down the stack, and
                            // we only have to consider _that_ value, not the one from
                            // the top of the stack as returned by Shape::getBounds()
                            rBounds.getCenter(),
                            rSlideSize,
                            &ShapeAttributeLayer::getPosX,
                            &ShapeAttributeLayer::getPosY,
                            &ShapeAttributeLayer::setPosition ) );

                default:
                    ENSURE_OR_THROW( false,
                                      "AnimationFactory::createPairPropertyAnimation(): Attribute type mismatch" );
                    break;
            }

            return PairAnimationSharedPtr();
        }

        StringAnimationSharedPtr AnimationFactory::createStringPropertyAnimation( const OUString&                rAttrName,
                                                                                  const AnimatableShapeSharedPtr&       rShape,
                                                                                  const ShapeManagerSharedPtr&          rShapeManager,
                                                                                  const ::basegfx::B2DVector&           /*rSlideSize*/,
                                                                                  int                                   nFlags )
        {
            // ATTENTION: When changing this map, also the classifyAttributeName() method must
            // be checked and possibly adapted in their switch statement
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                case AttributeType::Invalid:
                    ENSURE_OR_THROW( false,
                                      "AnimationFactory::createStringPropertyAnimation(): Unknown attribute" );
                    break;

                case AttributeType::CharColor:
                case AttributeType::CharHeight:
                case AttributeType::CharUnderline:
                case AttributeType::Color:
                case AttributeType::DimColor:
                case AttributeType::FillColor:
                case AttributeType::Height:
                case AttributeType::LineColor:
                case AttributeType::Opacity:
                case AttributeType::Rotate:
                case AttributeType::SkewX:
                case AttributeType::SkewY:
                case AttributeType::Visibility:
                case AttributeType::Width:
                case AttributeType::PosX:
                case AttributeType::PosY:
                case AttributeType::CharPosture:
                case AttributeType::CharWeight:
                case AttributeType::FillStyle:
                case AttributeType::LineStyle:
                    ENSURE_OR_THROW( false,
                                      "AnimationFactory::createStringPropertyAnimation(): Attribute type mismatch" );
                    break;

                case AttributeType::CharFontName:
                    return makeGenericAnimation<StringAnimation>( rShapeManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isFontFamilyValid,
                                                                  getDefault< OUString >( rShape, rAttrName ),
                                                                  &ShapeAttributeLayer::getFontFamily,
                                                                  &ShapeAttributeLayer::setFontFamily );
            }

            return StringAnimationSharedPtr();
        }

        BoolAnimationSharedPtr AnimationFactory::createBoolPropertyAnimation( const OUString&                rAttrName,
                                                                              const AnimatableShapeSharedPtr&       /*rShape*/,
                                                                              const ShapeManagerSharedPtr&          rShapeManager,
                                                                              const ::basegfx::B2DVector&           /*rSlideSize*/,
                                                                              int                                   nFlags )
        {
            // ATTENTION: When changing this map, also the classifyAttributeName() method must
            // be checked and possibly adapted in their switch statement
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                case AttributeType::Invalid:
                    ENSURE_OR_THROW( false,
                                      "AnimationFactory::createBoolPropertyAnimation(): Unknown attribute" );
                    break;

                case AttributeType::CharColor:
                case AttributeType::CharFontName:
                case AttributeType::CharHeight:
                case AttributeType::CharPosture:
                case AttributeType::CharWeight:
                case AttributeType::Color:
                case AttributeType::DimColor:
                case AttributeType::FillColor:
                case AttributeType::FillStyle:
                case AttributeType::Height:
                case AttributeType::LineColor:
                case AttributeType::LineStyle:
                case AttributeType::Opacity:
                case AttributeType::Rotate:
                case AttributeType::SkewX:
                case AttributeType::SkewY:
                case AttributeType::Width:
                case AttributeType::PosX:
                case AttributeType::PosY:
                case AttributeType::CharUnderline:
                    ENSURE_OR_THROW( false,
                                      "AnimationFactory::createBoolPropertyAnimation(): Attribute type mismatch" );
                    break;

                case AttributeType::Visibility:
                    return makeGenericAnimation<BoolAnimation>( rShapeManager,
                                                                nFlags,
                                                                &ShapeAttributeLayer::isVisibilityValid,
                                                                // TODO(F1): Is there a corresponding shape property?
                                                                true,
                                                                &ShapeAttributeLayer::getVisibility,
                                                                &ShapeAttributeLayer::setVisibility );
            }

            return BoolAnimationSharedPtr();
        }

        NumberAnimationSharedPtr AnimationFactory::createPathMotionAnimation( const OUString&            rSVGDPath,
                                                                              sal_Int16                         nAdditive,
                                                                              const AnimatableShapeSharedPtr&   /*rShape*/,
                                                                              const ShapeManagerSharedPtr&      rShapeManager,
                                                                              const ::basegfx::B2DVector&       rSlideSize,
                                                                              int                               nFlags )
        {
            return NumberAnimationSharedPtr(
                new PathAnimation( rSVGDPath, nAdditive,
                                   rShapeManager,
                                   rSlideSize,
                                   nFlags ) );
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
