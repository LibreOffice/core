/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationfactory.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:23:32 $
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

#ifndef _CANVAS_VERBOSETRACE_HXX
#include <canvas/verbosetrace.hxx>
#endif

#include <animationfactory.hxx>
#include <attributemap.hxx>

#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONTRANSFORMTYPE_HPP_
#include <com/sun/star/animations/AnimationTransformType.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
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
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#include <functional>


using namespace ::com::sun::star;


namespace presentation
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
                TupleAnimation( const LayerManagerSharedPtr&        rLayerManager,
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
                    mpLayerManager( rLayerManager ),
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
                    ENSURE_AND_THROW( rLayerManager.get(),
                                      "TupleAnimation::TupleAnimation(): Invalid LayerManager" );
                    ENSURE_AND_THROW( pIs1stValid && pIs2ndValid && pGet1stValue && pGet2ndValue && pSetValue,
                                      "TupleAnimation::TupleAnimation(): One of the method pointers is NULL" );
                }

                // Animation interface
                // -------------------
                virtual void start( const AnimatableShapeSharedPtr&     rShape,
                                    const ShapeAttributeLayerSharedPtr& rAttrLayer )
                {
                    OSL_ENSURE( !mpShape.get(),
                                "TupleAnimation::start(): Shape already set" );
                    OSL_ENSURE( !mpAttrLayer.get(),
                                "TupleAnimation::start(): Attribute layer already set" );

                    mpShape = rShape;
                    mpAttrLayer = rAttrLayer;

                    ENSURE_AND_THROW( rShape.get(),
                                      "TupleAnimation::start(): Invalid shape" );
                    ENSURE_AND_THROW( rAttrLayer.get(),
                                      "TupleAnimation::start(): Invalid attribute layer" );

                    if( !mbAnimationStarted )
                    {
                        mbAnimationStarted = true;

                        if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                            mpLayerManager->enterAnimationMode( mpShape );
                    }
                }

                virtual void end()
                {
                    if( mbAnimationStarted )
                    {
                        mbAnimationStarted = false;

                        if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                            mpLayerManager->leaveAnimationMode( mpShape );

                        if( mpShape->isUpdateNecessary() )
                            mpLayerManager->notifyShapeUpdate( mpShape );
                    }
                }

                // PairAnimation interface
                // -----------------------

                virtual bool operator()( const ::basegfx::B2DTuple& rValue )
                {
                    ENSURE_AND_RETURN( mpAttrLayer.get() && mpShape.get(),
                                       "TupleAnimation::operator(): Invalid ShapeAttributeLayer" );

                    ValueT aValue( rValue.getX(),
                                   rValue.getY() );

                    // Activitis get values from the expression parser,
                    // which returns _relative_ sizes/positions.
                    // Convert back relative to reference coordinate system
                    aValue *= maReferenceSize;

                    ((*mpAttrLayer).*mpSetValueFunc)( aValue );

                    if( mpShape->isUpdateNecessary() )
                        mpLayerManager->notifyShapeUpdate( mpShape );

                    return true;
                }

                virtual ::basegfx::B2DTuple getUnderlyingValue() const
                {
                    ENSURE_AND_THROW( mpAttrLayer.get(),
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
                LayerManagerSharedPtr              mpLayerManager;
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
                PathAnimation( const ::rtl::OUString&       rSVGDPath,
                               const LayerManagerSharedPtr& rLayerManager,
                               int                          nFlags ) :
                    maPathPoly(),
                    mpShape(),
                    mpAttrLayer(),
                    mpLayerManager( rLayerManager ),
                    maPageSize( rLayerManager->getPageBounds().getRange() ),
                    maShapeOrig(),
                    mnFlags( nFlags ),
                    mbAnimationStarted( false )
                {
                    ENSURE_AND_THROW( rLayerManager.get(),
                                      "PathAnimation::PathAnimation(): Invalid LayerManager" );

                    ::basegfx::B2DPolyPolygon aPolyPoly;

                    ENSURE_AND_THROW( ::basegfx::tools::importFromSvgD( aPolyPoly, rSVGDPath ),
                                      "PathAnimation::PathAnimation(): failed to parse SVG:d path" );
                    ENSURE_AND_THROW( aPolyPoly.count() == 1,
                                      "PathAnimation::PathAnimation(): motion path consists of multiple/zero polygon(s)" );

                    // TODO(F2): Since getPositionRelative() currently
                    // cannot handle beziers, have to subdivide.
                    maPathPoly = ::basegfx::tools::adaptiveSubdivideByAngle(
                        aPolyPoly.getB2DPolygon(0) );
                }

                // Animation interface
                // -------------------
                virtual void start( const AnimatableShapeSharedPtr&     rShape,
                                    const ShapeAttributeLayerSharedPtr& rAttrLayer )
                {
                    OSL_ENSURE( !mpShape.get(),
                                "PathAnimation::start(): Shape already set" );
                    OSL_ENSURE( !mpAttrLayer.get(),
                                "PathAnimation::start(): Attribute layer already set" );

                    mpShape = rShape;
                    mpAttrLayer = rAttrLayer;

                    ENSURE_AND_THROW( rShape.get(),
                                      "PathAnimation::start(): Invalid shape" );
                    ENSURE_AND_THROW( rAttrLayer.get(),
                                      "PathAnimation::start(): Invalid attribute layer" );

                    // TODO(F1): Check whether _shape_ bounds are correct here.
                    // Theoretically, our AttrLayer is way down the stack, and
                    // we only have to consider _that_ value, not the one from
                    // the top of the stack as returned by Shape::getBounds()
                    maShapeOrig = mpShape->getPosSize().getCenter();

                    if( !mbAnimationStarted )
                    {
                        mbAnimationStarted = true;

                        if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                            mpLayerManager->enterAnimationMode( mpShape );
                    }
                }

                virtual void end()
                {
                    if( mbAnimationStarted )
                    {
                        mbAnimationStarted = false;

                        if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                            mpLayerManager->leaveAnimationMode( mpShape );

                        if( mpShape->isUpdateNecessary() )
                            mpLayerManager->notifyShapeUpdate( mpShape );
                    }
                }

                // NumberAnimation interface
                // -----------------------

                virtual bool operator()( double nValue )
                {
                    ENSURE_AND_RETURN( mpAttrLayer.get() && mpShape.get(),
                                       "PathAnimation::operator(): Invalid ShapeAttributeLayer" );

                    ::basegfx::B2DPoint rOutPos = ::basegfx::tools::getPositionRelative( maPathPoly,
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

                    if( mpShape->isUpdateNecessary() )
                        mpLayerManager->notifyShapeUpdate( mpShape );

                    return true;
                }

                virtual double getUnderlyingValue() const
                {
                    ENSURE_AND_THROW( mpAttrLayer.get(),
                                      "PathAnimation::getUnderlyingValue(): Invalid ShapeAttributeLayer" );

                    return 0.0; // though this should be used in concert with
                                // ActivitiesFactory::createSimpleActivity, better
                                // explicitely name our start value.
                                // Permissible range for operator() above is [0,1]
                }

            private:
                ::basegfx::B2DPolygon              maPathPoly;
                AnimatableShapeSharedPtr           mpShape;
                ShapeAttributeLayerSharedPtr       mpAttrLayer;
                LayerManagerSharedPtr              mpLayerManager;
                const ::basegfx::B2DSize           maPageSize;
                ::basegfx::B2DPoint                maShapeOrig;
                const int                          mnFlags;
                bool                               mbAnimationStarted;
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
                GenericAnimation( const LayerManagerSharedPtr&          rLayerManager,
                                  int                                   nFlags,
                                  bool           (ShapeAttributeLayer::*pIsValid)() const,
                                  const ValueT&                         rDefaultValue,
                                  ValueT         (ShapeAttributeLayer::*pGetValue)() const,
                                  void           (ShapeAttributeLayer::*pSetValue)( const ValueT& ),
                                  const ModifierFunctor&                rGetterModifier,
                                  const ModifierFunctor&                rSetterModifier ) :
                    mpShape(),
                    mpAttrLayer(),
                    mpLayerManager( rLayerManager ),
                    mpIsValidFunc(pIsValid),
                    mpGetValueFunc(pGetValue),
                    mpSetValueFunc(pSetValue),
                    maGetterModifier( rGetterModifier ),
                    maSetterModifier( rSetterModifier ),
                    mnFlags( nFlags ),
                    maDefaultValue(rDefaultValue),
                    mbAnimationStarted( false )
                {
                    ENSURE_AND_THROW( rLayerManager.get(),
                                      "GenericAnimation::GenericAnimation(): Invalid LayerManager" );
                    ENSURE_AND_THROW( pIsValid && pGetValue && pSetValue,
                                      "GenericAnimation::GenericAnimation(): One of the method pointers is NULL" );
                }

                // Animation interface
                // -------------------
                virtual void start( const AnimatableShapeSharedPtr&     rShape,
                                    const ShapeAttributeLayerSharedPtr& rAttrLayer )
                {
                    OSL_ENSURE( !mpShape.get(),
                                "GenericAnimation::start(): Shape already set" );
                    OSL_ENSURE( !mpAttrLayer.get(),
                                "GenericAnimation::start(): Attribute layer already set" );

                    mpShape = rShape;
                    mpAttrLayer = rAttrLayer;

                    ENSURE_AND_THROW( rShape.get(),
                                      "GenericAnimation::start(): Invalid shape" );
                    ENSURE_AND_THROW( rAttrLayer.get(),
                                      "GenericAnimation::start(): Invalid attribute layer" );

                    // only start animation once per repeated start() call,
                    // and only if sprites should be used for display
                    if( !mbAnimationStarted )
                    {
                        mbAnimationStarted = true;

                        if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                            mpLayerManager->enterAnimationMode( mpShape );
                    }
                }

                virtual void end()
                {
                    // TODO(Q2): Factor out common code (most
                    // prominently start() and end()) into base class

                    // only stop animation once per repeated end() call,
                    // and only if sprites are used for display
                    if( mbAnimationStarted )
                    {
                        mbAnimationStarted = false;

                        if( !(mnFlags & AnimationFactory::FLAG_NO_SPRITE) )
                            mpLayerManager->leaveAnimationMode( mpShape );

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
                        //
                        // To reiterate: normally, we're called from
                        // the Activity first, thus the
                        // notifyShapeUpdate() below will update to
                        // the last activity value.

                        // force shape update, activity might have changed
                        // state in the last round.
                        if( mpShape->isUpdateNecessary() )
                            mpLayerManager->notifyShapeUpdate( mpShape );
                    }
                }

                // Derived Animation interface
                // ---------------------------

                /** For by-reference interfaces (B2DTuple, OUString)
                 */
                bool operator()( const ValueT& x )
                {
                    ENSURE_AND_RETURN( mpAttrLayer.get() && mpShape.get(),
                                       "GenericAnimation::operator(): Invalid ShapeAttributeLayer" );

                    ((*mpAttrLayer).*mpSetValueFunc)( maSetterModifier( x ) );

                    if( mpShape->isUpdateNecessary() )
                        mpLayerManager->notifyShapeUpdate( mpShape );

                    return true;
                }

                /** For by-value interfaces (bool, double)
                 */
                bool operator()( ValueT x )
                {
                    ENSURE_AND_RETURN( mpAttrLayer.get() && mpShape.get(),
                                       "GenericAnimation::operator(): Invalid ShapeAttributeLayer" );

                    ((*mpAttrLayer).*mpSetValueFunc)( maSetterModifier( x ) );

                    if( mpShape->isUpdateNecessary() )
                        mpLayerManager->notifyShapeUpdate( mpShape );

                    return true;
                }

                ValueT getUnderlyingValue() const
                {
                    ENSURE_AND_THROW( mpAttrLayer.get(),
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
                LayerManagerSharedPtr              mpLayerManager;
                bool        (ShapeAttributeLayer::*mpIsValidFunc)() const;
                ValueT      (ShapeAttributeLayer::*mpGetValueFunc)() const;
                void        (ShapeAttributeLayer::*mpSetValueFunc)( const ValueT& );

                ModifierFunctor                    maGetterModifier;
                ModifierFunctor                    maSetterModifier;

                const int                          mnFlags;

                const ValueT                       maDefaultValue;
                bool                               mbAnimationStarted;
            };

            /** Function template wrapper around GenericAnimation template

                @tpl AnimationBase
                Type of animation to generate (determines the
                interface GenericAnimation will implement).
             */
            template< typename AnimationBase > ::boost::shared_ptr< AnimationBase >
                makeGenericAnimation( const LayerManagerSharedPtr&                             rLayerManager,
                                      int                                                      nFlags,
                                      bool                              (ShapeAttributeLayer::*pIsValid)() const,
                                      const typename AnimationBase::ValueType&                 rDefaultValue,
                                      typename AnimationBase::ValueType (ShapeAttributeLayer::*pGetValue)() const,
                                      void                              (ShapeAttributeLayer::*pSetValue)( const typename AnimationBase::ValueType& ) )
            {
                return ::boost::shared_ptr< AnimationBase >(
                    new GenericAnimation< AnimationBase,
                                          ::std::identity< typename AnimationBase::ValueType > >(
                                              rLayerManager,
                                              nFlags,
                                              pIsValid,
                                              rDefaultValue,
                                              pGetValue,
                                              pSetValue,
                                              // no modification necessary, use identity functor here
                                              ::std::identity< typename AnimationBase::ValueType >(),
                                              ::std::identity< typename AnimationBase::ValueType >() ) );
            }

            class Scaler
            {
            public:
                Scaler( double nScale ) :
                    mnScale( nScale )
                {
                }

                double operator()( double nVal ) const
                {
                    return mnScale * nVal;
                }

            private:
                double mnScale;
            };

            /** Overload for NumberAnimations which need scaling (width,height,x,y currently)
             */
            NumberAnimationSharedPtr makeGenericAnimation( const LayerManagerSharedPtr&                             rLayerManager,
                                                           int                                                      nFlags,
                                                           bool                              (ShapeAttributeLayer::*pIsValid)() const,
                                                           double                                                   nDefaultValue,
                                                           double                            (ShapeAttributeLayer::*pGetValue)() const,
                                                           void                              (ShapeAttributeLayer::*pSetValue)( const double& ),
                                                           double                                                   nScaleValue )
            {
                return NumberAnimationSharedPtr(
                    new GenericAnimation< NumberAnimation, Scaler >( rLayerManager,
                                                                     nFlags,
                                                                     pIsValid,
                                                                     nDefaultValue / nScaleValue,
                                                                     pGetValue,
                                                                     pSetValue,
                                                                     Scaler( 1.0/nScaleValue ),
                                                                     Scaler( nScaleValue ) ) );
            }


            uno::Any getShapeDefault( const AnimatableShapeSharedPtr&   rShape,
                                      const ::rtl::OUString&            rPropertyName )
            {
                uno::Reference< drawing::XShape > xShape( rShape->getXShape() );

                if( !xShape.is() )
                    return uno::Any(); // no regular shape, no defaults available


                // extract relevant value from XShape's PropertySet
                uno::Reference< beans::XPropertySet > xPropSet( xShape,
                                                                uno::UNO_QUERY );

                ENSURE_AND_THROW( xPropSet.is(),
                                  "getShapeDefault(): Cannot query property set from shape" );

                return xPropSet->getPropertyValue( rPropertyName );
            }

            template< typename ValueType > ValueType getDefault( const AnimatableShapeSharedPtr&    rShape,
                                                                 const ::rtl::OUString&             rPropertyName )
            {
                const uno::Any& rAny( getShapeDefault( rShape,
                                                       rPropertyName ) );

                if( !rAny.hasValue() )
                {
                    OSL_ENSURE( false, "getDefault(): cannot get requested shape property" );
                    OSL_TRACE( "getDefault(): cannot get '%s' shape property",
                               ::rtl::OUStringToOString( rPropertyName,
                                                         RTL_TEXTENCODING_ASCII_US ).getStr() );
                    return ValueType();
                }
                else
                {
                    ValueType aValue;

                    if( !(rAny >>= aValue) )
                    {
                        OSL_ENSURE( false, "getDefault(): cannot extract requested shape property" );
                        OSL_TRACE( "getDefault(): cannot extract '%s' shape property",
                                   ::rtl::OUStringToOString( rPropertyName,
                                                             RTL_TEXTENCODING_ASCII_US ).getStr() );
                        return ValueType();
                    }

                    return aValue;
                }
            }

            template<> RGBColor getDefault< RGBColor >( const AnimatableShapeSharedPtr& rShape,
                                                        const ::rtl::OUString&          rPropertyName )
            {
                const uno::Any& rAny( getShapeDefault( rShape,
                                                       rPropertyName ) );

                if( !rAny.hasValue() )
                {
                    OSL_ENSURE( false, "getDefault(): cannot get requested shape color property" );
                    OSL_TRACE( "getDefault(): cannot get '%s' shape color property",
                               ::rtl::OUStringToOString( rPropertyName,
                                                         RTL_TEXTENCODING_ASCII_US ).getStr() );
                    return RGBColor();
                }
                else
                {
                    sal_Int32 nValue;

                    if( !(rAny >>= nValue) )
                    {
                        OSL_ENSURE( false, "getDefault(): cannot extract requested shape color property" );
                        OSL_TRACE( "getDefault(): cannot extract '%s' shape color property",
                                   ::rtl::OUStringToOString( rPropertyName,
                                                             RTL_TEXTENCODING_ASCII_US ).getStr() );
                        return RGBColor();
                    }

                    // convert from 0xAARRGGBB API color to 0xRRGGBB00
                    // canvas color
                    return RGBColor( (nValue << 8U) & 0xFFFFFF00U );
                }
            }
        }

        AnimationFactory::AttributeClass AnimationFactory::classifyAttributeName( const ::rtl::OUString& rAttrName )
        {
            // ATTENTION: When changing this map, also the create*PropertyAnimation() methods must
            // be checked and possibly adapted in their switch statements

            // TODO(Q2): Since this map must be coherent with the various switch statements
            // in the create*PropertyAnimation methods, try to unify into a single method or table
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                    // FALLTHROUGH intended
                case ATTRIBUTE_INVALID:
                    return CLASS_UNKNOWN_PROPERTY;

                case ATTRIBUTE_CHAR_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_DIMCOLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_FILL_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_LINE_COLOR:
                    return CLASS_COLOR_PROPERTY;

                case ATTRIBUTE_CHAR_FONT_NAME:
                    return CLASS_STRING_PROPERTY;

                case ATTRIBUTE_VISIBILITY:
                    return CLASS_BOOL_PROPERTY;

                case ATTRIBUTE_CHAR_HEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_WEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_ROTATION:
                    // FALLTHROUGH intended
                case ATTRIBUTE_HEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_OPACITY:
                    // FALLTHROUGH intended
                case ATTRIBUTE_ROTATE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_SKEW_X:
                    // FALLTHROUGH intended
                case ATTRIBUTE_SKEW_Y:
                    // FALLTHROUGH intended
                case ATTRIBUTE_WIDTH:
                    // FALLTHROUGH intended
                case ATTRIBUTE_POS_X:
                    // FALLTHROUGH intended
                case ATTRIBUTE_POS_Y:
                    return CLASS_NUMBER_PROPERTY;

                case ATTRIBUTE_CHAR_UNDERLINE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_FILL_STYLE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_LINE_STYLE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_POSTURE:
                    return CLASS_ENUM_PROPERTY;
            }
        }

        NumberAnimationSharedPtr AnimationFactory::createNumberPropertyAnimation( const ::rtl::OUString&                rAttrName,
                                                                                  const AnimatableShapeSharedPtr&       rShape,
                                                                                  const LayerManagerSharedPtr&          rLayerManager,
                                                                                  int                                   nFlags )
        {
            // ATTENTION: When changing this map, also the classifyAttributeName() method must
            // be checked and possibly adapted in their switch statement
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                    // FALLTHROUGH intended
                case ATTRIBUTE_INVALID:
                    ENSURE_AND_THROW( false,
                                      "AnimationFactory::createNumberPropertyAnimation(): Unknown attribute" );
                    break;

                case ATTRIBUTE_CHAR_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_FONT_NAME:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_POSTURE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_UNDERLINE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_DIMCOLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_FILL_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_FILL_STYLE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_LINE_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_LINE_STYLE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_VISIBILITY:
                    ENSURE_AND_THROW( false,
                                      "AnimationFactory::createNumberPropertyAnimation(): Attribute type mismatch" );
                    break;

                case ATTRIBUTE_CHAR_HEIGHT:
                    return makeGenericAnimation<NumberAnimation>( rLayerManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isCharScaleValid,
                                                                  1.0, // CharHeight is a relative attribute, thus
                                                                         // default is 1.0
                                                                  &ShapeAttributeLayer::getCharScale,
                                                                  &ShapeAttributeLayer::setCharScale );

                case ATTRIBUTE_CHAR_WEIGHT:
                    return makeGenericAnimation<NumberAnimation>( rLayerManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isCharWeightValid,
                                                                  getDefault<double>( rShape, rAttrName ),
                                                                  &ShapeAttributeLayer::getCharWeight,
                                                                  &ShapeAttributeLayer::setCharWeight );

                case ATTRIBUTE_CHAR_ROTATION:
                    return makeGenericAnimation<NumberAnimation>( rLayerManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isCharRotationAngleValid,
                                                                  getDefault<double>( rShape, rAttrName ),
                                                                  &ShapeAttributeLayer::getCharRotationAngle,
                                                                  &ShapeAttributeLayer::setCharRotationAngle );

                case ATTRIBUTE_HEIGHT:
                    return makeGenericAnimation( rLayerManager,
                                                 nFlags,
                                                 &ShapeAttributeLayer::isHeightValid,
                                                 // TODO(F1): Check whether _shape_ bounds are correct here.
                                                 // Theoretically, our AttrLayer is way down the stack, and
                                                 // we only have to consider _that_ value, not the one from
                                                 // the top of the stack as returned by Shape::getBounds()
                                                 rShape->getPosSize().getHeight(),
                                                 &ShapeAttributeLayer::getHeight,
                                                 &ShapeAttributeLayer::setHeight,
                                                 // convert expression parser value from relative page size
                                                 rLayerManager->getPageBounds().getHeight() );

                case ATTRIBUTE_OPACITY:
                    return makeGenericAnimation<NumberAnimation>( rLayerManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isAlphaValid,
                                                                  // TODO(F1): Provide shape default here (FillTransparency?)
                                                                  1.0,
                                                                  &ShapeAttributeLayer::getAlpha,
                                                                  &ShapeAttributeLayer::setAlpha );

                case ATTRIBUTE_ROTATE:
                    return makeGenericAnimation<NumberAnimation>( rLayerManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isRotationAngleValid,
                                                                  // NOTE: Since we paint the shape as-is from metafile,
                                                                  // rotation angle is always 0.0, even for rotated shapes
                                                                  0.0,
                                                                  &ShapeAttributeLayer::getRotationAngle,
                                                                  &ShapeAttributeLayer::setRotationAngle );

                case ATTRIBUTE_SKEW_X:
                    return makeGenericAnimation<NumberAnimation>( rLayerManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isShearXAngleValid,
                                                                  // TODO(F1): Is there any shape property for skew?
                                                                  0.0,
                                                                  &ShapeAttributeLayer::getShearXAngle,
                                                                  &ShapeAttributeLayer::setShearXAngle );

                case ATTRIBUTE_SKEW_Y:
                    return makeGenericAnimation<NumberAnimation>( rLayerManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isShearYAngleValid,
                                                                  // TODO(F1): Is there any shape property for skew?
                                                                  0.0,
                                                                  &ShapeAttributeLayer::getShearYAngle,
                                                                  &ShapeAttributeLayer::setShearYAngle );

                case ATTRIBUTE_WIDTH:
                    return makeGenericAnimation( rLayerManager,
                                                 nFlags,
                                                 &ShapeAttributeLayer::isWidthValid,
                                                 // TODO(F1): Check whether _shape_ bounds are correct here.
                                                 // Theoretically, our AttrLayer is way down the stack, and
                                                 // we only have to consider _that_ value, not the one from
                                                 // the top of the stack as returned by Shape::getBounds()
                                                 rShape->getPosSize().getWidth(),
                                                 &ShapeAttributeLayer::getWidth,
                                                 &ShapeAttributeLayer::setWidth,
                                                 // convert expression parser value from relative page size
                                                 rLayerManager->getPageBounds().getWidth() );

                case ATTRIBUTE_POS_X:
                    return makeGenericAnimation( rLayerManager,
                                                 nFlags,
                                                 &ShapeAttributeLayer::isPosXValid,
                                                 // TODO(F1): Check whether _shape_ bounds are correct here.
                                                 // Theoretically, our AttrLayer is way down the stack, and
                                                 // we only have to consider _that_ value, not the one from
                                                 // the top of the stack as returned by Shape::getBounds()
                                                 rShape->getPosSize().getCenterX(),
                                                 &ShapeAttributeLayer::getPosX,
                                                 &ShapeAttributeLayer::setPosX,
                                                 // convert expression parser value from relative page size
                                                 rLayerManager->getPageBounds().getWidth() );

                case ATTRIBUTE_POS_Y:
                    return makeGenericAnimation( rLayerManager,
                                                 nFlags,
                                                 &ShapeAttributeLayer::isPosYValid,
                                                 // TODO(F1): Check whether _shape_ bounds are correct here.
                                                 // Theoretically, our AttrLayer is way down the stack, and
                                                 // we only have to consider _that_ value, not the one from
                                                 // the top of the stack as returned by Shape::getBounds()
                                                 rShape->getPosSize().getCenterY(),
                                                 &ShapeAttributeLayer::getPosY,
                                                 &ShapeAttributeLayer::setPosY,
                                                 // convert expression parser value from relative page size
                                                 rLayerManager->getPageBounds().getHeight() );
            }

            return NumberAnimationSharedPtr();
        }

        EnumAnimationSharedPtr AnimationFactory::createEnumPropertyAnimation( const ::rtl::OUString&                rAttrName,
                                                                              const AnimatableShapeSharedPtr&       rShape,
                                                                              const LayerManagerSharedPtr&          rLayerManager,
                                                                              int                                   nFlags )
        {
            // ATTENTION: When changing this map, also the classifyAttributeName() method must
            // be checked and possibly adapted in their switch statement
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                    // FALLTHROUGH intended
                case ATTRIBUTE_INVALID:
                    ENSURE_AND_THROW( false,
                                      "AnimationFactory::createEnumPropertyAnimation(): Unknown attribute" );
                    break;

                case ATTRIBUTE_CHAR_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_FONT_NAME:
                    // FALLTHROUGH intended
                case ATTRIBUTE_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_DIMCOLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_FILL_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_LINE_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_VISIBILITY:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_HEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_WEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_ROTATION:
                    // FALLTHROUGH intended
                case ATTRIBUTE_HEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_OPACITY:
                    // FALLTHROUGH intended
                case ATTRIBUTE_ROTATE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_SKEW_X:
                    // FALLTHROUGH intended
                case ATTRIBUTE_SKEW_Y:
                    // FALLTHROUGH intended
                case ATTRIBUTE_WIDTH:
                    // FALLTHROUGH intended
                case ATTRIBUTE_POS_X:
                    // FALLTHROUGH intended
                case ATTRIBUTE_POS_Y:
                    ENSURE_AND_THROW( false,
                                      "AnimationFactory::createEnumPropertyAnimation(): Attribute type mismatch" );
                    break;


                case ATTRIBUTE_FILL_STYLE:
                    return makeGenericAnimation<EnumAnimation>( rLayerManager,
                                                                nFlags,
                                                                &ShapeAttributeLayer::isFillStyleValid,
                                                                (sal_Int16)getDefault<drawing::FillStyle>( rShape, rAttrName ),
                                                                &ShapeAttributeLayer::getFillStyle,
                                                                &ShapeAttributeLayer::setFillStyle );

                case ATTRIBUTE_LINE_STYLE:
                    return makeGenericAnimation<EnumAnimation>( rLayerManager,
                                                                nFlags,
                                                                &ShapeAttributeLayer::isLineStyleValid,
                                                                (sal_Int16)getDefault<drawing::LineStyle>( rShape, rAttrName ),
                                                                &ShapeAttributeLayer::getLineStyle,
                                                                &ShapeAttributeLayer::setLineStyle );

                case ATTRIBUTE_CHAR_POSTURE:
                    return makeGenericAnimation<EnumAnimation>( rLayerManager,
                                                                nFlags,
                                                                &ShapeAttributeLayer::isCharPostureValid,
                                                                (sal_Int16)getDefault<awt::FontSlant>( rShape, rAttrName ),
                                                                &ShapeAttributeLayer::getCharPosture,
                                                                &ShapeAttributeLayer::setCharPosture );

                case ATTRIBUTE_CHAR_UNDERLINE:
                    return makeGenericAnimation<EnumAnimation>( rLayerManager,
                                                                nFlags,
                                                                &ShapeAttributeLayer::isUnderlineModeValid,
                                                                getDefault<sal_Int16>( rShape, rAttrName ),
                                                                &ShapeAttributeLayer::getUnderlineMode,
                                                                &ShapeAttributeLayer::setUnderlineMode );
            }

            return EnumAnimationSharedPtr();
        }

        ColorAnimationSharedPtr AnimationFactory::createColorPropertyAnimation( const ::rtl::OUString&              rAttrName,
                                                                                const AnimatableShapeSharedPtr&     rShape,
                                                                                const LayerManagerSharedPtr&        rLayerManager,
                                                                                int                                 nFlags )
        {
            // ATTENTION: When changing this map, also the classifyAttributeName() method must
            // be checked and possibly adapted in their switch statement
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                    // FALLTHROUGH intended
                case ATTRIBUTE_INVALID:
                    ENSURE_AND_THROW( false,
                                      "AnimationFactory::createColorPropertyAnimation(): Unknown attribute" );
                    break;

                case ATTRIBUTE_CHAR_FONT_NAME:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_HEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_POSTURE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_ROTATION:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_UNDERLINE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_WEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_FILL_STYLE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_HEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_LINE_STYLE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_OPACITY:
                    // FALLTHROUGH intended
                case ATTRIBUTE_ROTATE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_SKEW_X:
                    // FALLTHROUGH intended
                case ATTRIBUTE_SKEW_Y:
                    // FALLTHROUGH intended
                case ATTRIBUTE_VISIBILITY:
                    // FALLTHROUGH intended
                case ATTRIBUTE_WIDTH:
                    // FALLTHROUGH intended
                case ATTRIBUTE_POS_X:
                    // FALLTHROUGH intended
                case ATTRIBUTE_POS_Y:
                    ENSURE_AND_THROW( false,
                                      "AnimationFactory::createColorPropertyAnimation(): Attribute type mismatch" );
                    break;

                case ATTRIBUTE_CHAR_COLOR:
                    return makeGenericAnimation<ColorAnimation>( rLayerManager,
                                                                 nFlags,
                                                                 &ShapeAttributeLayer::isCharColorValid,
                                                                 getDefault<RGBColor>( rShape, rAttrName ),
                                                                 &ShapeAttributeLayer::getCharColor,
                                                                 &ShapeAttributeLayer::setCharColor );

                case ATTRIBUTE_COLOR:
                    // TODO(F2): This is just mapped to fill color to make it work
                    return makeGenericAnimation<ColorAnimation>( rLayerManager,
                                                                 nFlags,
                                                                 &ShapeAttributeLayer::isFillColorValid,
                                                                 getDefault<RGBColor>( rShape, rAttrName ),
                                                                 &ShapeAttributeLayer::getFillColor,
                                                                 &ShapeAttributeLayer::setFillColor );

                case ATTRIBUTE_DIMCOLOR:
                    return makeGenericAnimation<ColorAnimation>( rLayerManager,
                                                                 nFlags,
                                                                 &ShapeAttributeLayer::isDimColorValid,
                                                                 getDefault<RGBColor>( rShape, rAttrName ),
                                                                 &ShapeAttributeLayer::getDimColor,
                                                                 &ShapeAttributeLayer::setDimColor );

                case ATTRIBUTE_FILL_COLOR:
                    return makeGenericAnimation<ColorAnimation>( rLayerManager,
                                                                 nFlags,
                                                                 &ShapeAttributeLayer::isFillColorValid,
                                                                 getDefault<RGBColor>( rShape, rAttrName ),
                                                                 &ShapeAttributeLayer::getFillColor,
                                                                 &ShapeAttributeLayer::setFillColor );

                case ATTRIBUTE_LINE_COLOR:
                    return makeGenericAnimation<ColorAnimation>( rLayerManager,
                                                                 nFlags,
                                                                 &ShapeAttributeLayer::isLineColorValid,
                                                                 getDefault<RGBColor>( rShape, rAttrName ),
                                                                 &ShapeAttributeLayer::getLineColor,
                                                                 &ShapeAttributeLayer::setLineColor );
            }

            return ColorAnimationSharedPtr();
        }

        PairAnimationSharedPtr AnimationFactory::createPairPropertyAnimation( const AnimatableShapeSharedPtr&       rShape,
                                                                              const LayerManagerSharedPtr&          rLayerManager,
                                                                              sal_Int16                             nTransformType,
                                                                              int                                   nFlags )
        {
            const ::basegfx::B2DRectangle& rBounds( rShape->getPosSize() );

            switch( nTransformType )
            {
                case animations::AnimationTransformType::SCALE:
                    return PairAnimationSharedPtr(
                        new TupleAnimation< ::basegfx::B2DSize >(
                            rLayerManager,
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
                            rLayerManager,
                            nFlags,
                            &ShapeAttributeLayer::isPosXValid,
                            &ShapeAttributeLayer::isPosYValid,
                            // TODO(F1): Check whether _shape_ bounds are correct here.
                            // Theoretically, our AttrLayer is way down the stack, and
                            // we only have to consider _that_ value, not the one from
                            // the top of the stack as returned by Shape::getBounds()
                            rBounds.getCenter(),
                            rLayerManager->getPageBounds().getRange(),
                            &ShapeAttributeLayer::getPosX,
                            &ShapeAttributeLayer::getPosY,
                            &ShapeAttributeLayer::setPosition ) );

                default:
                    ENSURE_AND_THROW( false,
                                      "AnimationFactory::createPairPropertyAnimation(): Attribute type mismatch" );
                    break;
            }

            return PairAnimationSharedPtr();
        }

        StringAnimationSharedPtr AnimationFactory::createStringPropertyAnimation( const ::rtl::OUString&                rAttrName,
                                                                                  const AnimatableShapeSharedPtr&       rShape,
                                                                                  const LayerManagerSharedPtr&          rLayerManager,
                                                                                  int                                   nFlags )
        {
            // ATTENTION: When changing this map, also the classifyAttributeName() method must
            // be checked and possibly adapted in their switch statement
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                    // FALLTHROUGH intended
                case ATTRIBUTE_INVALID:
                    ENSURE_AND_THROW( false,
                                      "AnimationFactory::createStringPropertyAnimation(): Unknown attribute" );
                    break;

                case ATTRIBUTE_CHAR_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_HEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_ROTATION:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_UNDERLINE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_DIMCOLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_FILL_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_HEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_LINE_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_OPACITY:
                    // FALLTHROUGH intended
                case ATTRIBUTE_ROTATE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_SKEW_X:
                    // FALLTHROUGH intended
                case ATTRIBUTE_SKEW_Y:
                    // FALLTHROUGH intended
                case ATTRIBUTE_VISIBILITY:
                    // FALLTHROUGH intended
                case ATTRIBUTE_WIDTH:
                    // FALLTHROUGH intended
                case ATTRIBUTE_POS_X:
                    // FALLTHROUGH intended
                case ATTRIBUTE_POS_Y:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_POSTURE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_WEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_FILL_STYLE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_LINE_STYLE:
                    ENSURE_AND_THROW( false,
                                      "AnimationFactory::createStringPropertyAnimation(): Attribute type mismatch" );
                    break;

                case ATTRIBUTE_CHAR_FONT_NAME:
                    return makeGenericAnimation<StringAnimation>( rLayerManager,
                                                                  nFlags,
                                                                  &ShapeAttributeLayer::isFontFamilyValid,
                                                                  getDefault< ::rtl::OUString >( rShape, rAttrName ),
                                                                  &ShapeAttributeLayer::getFontFamily,
                                                                  &ShapeAttributeLayer::setFontFamily );
            }

            return StringAnimationSharedPtr();
        }

        BoolAnimationSharedPtr AnimationFactory::createBoolPropertyAnimation( const ::rtl::OUString&                rAttrName,
                                                                              const AnimatableShapeSharedPtr&       rShape,
                                                                              const LayerManagerSharedPtr&          rLayerManager,
                                                                              int                                   nFlags )
        {
            // ATTENTION: When changing this map, also the classifyAttributeName() method must
            // be checked and possibly adapted in their switch statement
            switch( mapAttributeName( rAttrName ) )
            {
                default:
                    // FALLTHROUGH intended
                case ATTRIBUTE_INVALID:
                    ENSURE_AND_THROW( false,
                                      "AnimationFactory::createBoolPropertyAnimation(): Unknown attribute" );
                    break;

                case ATTRIBUTE_CHAR_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_FONT_NAME:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_HEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_POSTURE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_ROTATION:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_WEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_DIMCOLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_FILL_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_FILL_STYLE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_HEIGHT:
                    // FALLTHROUGH intended
                case ATTRIBUTE_LINE_COLOR:
                    // FALLTHROUGH intended
                case ATTRIBUTE_LINE_STYLE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_OPACITY:
                    // FALLTHROUGH intended
                case ATTRIBUTE_ROTATE:
                    // FALLTHROUGH intended
                case ATTRIBUTE_SKEW_X:
                    // FALLTHROUGH intended
                case ATTRIBUTE_SKEW_Y:
                    // FALLTHROUGH intended
                case ATTRIBUTE_WIDTH:
                    // FALLTHROUGH intended
                case ATTRIBUTE_POS_X:
                    // FALLTHROUGH intended
                case ATTRIBUTE_POS_Y:
                    // FALLTHROUGH intended
                case ATTRIBUTE_CHAR_UNDERLINE:
                    ENSURE_AND_THROW( false,
                                      "AnimationFactory::createBoolPropertyAnimation(): Attribute type mismatch" );
                    break;

                case ATTRIBUTE_VISIBILITY:
                    return makeGenericAnimation<BoolAnimation>( rLayerManager,
                                                                nFlags,
                                                                &ShapeAttributeLayer::isVisibilityValid,
                                                                // TODO(F1): Is there a corresponding shape property?
                                                                true,
                                                                &ShapeAttributeLayer::getVisibility,
                                                                &ShapeAttributeLayer::setVisibility );
            }

            return BoolAnimationSharedPtr();
        }

        NumberAnimationSharedPtr AnimationFactory::createPathMotionAnimation( const ::rtl::OUString&            rSVGDPath,
                                                                              const AnimatableShapeSharedPtr&   rShape,
                                                                              const LayerManagerSharedPtr&      rLayerManager,
                                                                              int                               nFlags )
        {
            return NumberAnimationSharedPtr(
                new PathAnimation( rSVGDPath,
                                   rLayerManager,
                                   nFlags ) );
        }

    }
}
