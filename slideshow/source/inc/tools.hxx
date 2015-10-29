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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_TOOLS_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_TOOLS_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <comphelper/random.hxx>
#include <vector>

#include <cppcanvas/color.hxx>

#include "shapeattributelayer.hxx"
#include "shape.hxx"
#include "rgbcolor.hxx"
#include "hslcolor.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/current_function.hpp>

#include <functional>
#include <cstdlib>
#include <string.h>
#include <algorithm>

namespace com { namespace sun { namespace star { namespace beans {
    struct NamedValue;
} } } }
namespace basegfx
{
    class B2DRange;
    class B2DVector;
    class B2IVector;
    class B2DHomMatrix;
}
namespace cppcanvas{ class Canvas; }

class GDIMetaFile;

/* Definition of some animation tools */
namespace slideshow
{
    namespace internal
    {
        class UnoView;
        class Shape;
        class ShapeAttributeLayer;

        typedef ::boost::shared_ptr< GDIMetaFile > GDIMetaFileSharedPtr;

        template <typename T>
        inline ::std::size_t hash_value( T const * p )
        {
            ::std::size_t d = static_cast< ::std::size_t >(
                reinterpret_cast< ::std::ptrdiff_t >(p) );
            return d + (d >> 3);
        }

        // xxx todo: remove with boost::hash when 1.33 is available
        template <typename T>
        struct hash : ::std::unary_function<T, ::std::size_t>
        {
            ::std::size_t operator()( T const& val ) const {
                return hash_value(val);
            }
        };
    }
}

namespace com { namespace sun { namespace star { namespace uno {

        template <typename T>
        inline ::std::size_t hash_value(
            css::uno::Reference<T> const& x )
        {
            // normalize to object root, because _only_ XInterface is defined
            // to be stable during object lifetime:
            css::uno::Reference< css::uno::XInterface> const xRoot( x, css::uno::UNO_QUERY );
            return slideshow::internal::hash<void *>()(xRoot.get());
        }

} } } }

namespace slideshow
{
    namespace internal
    {
        /** Cycle mode of intrinsic animations
         */
        enum CycleMode
        {
            /// loop the animation back to back
            CYCLE_LOOP,
            /// loop, but play backwards from end to start
            CYCLE_PINGPONGLOOP
        };


        // Value extraction from Any
        // =========================

        /// extract unary double value from Any
        bool extractValue( double&                              o_rValue,
                           const css::uno::Any&                rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract int from Any
        bool extractValue( sal_Int32&                           o_rValue,
                           const css::uno::Any&                 rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract enum/constant group value from Any
        bool extractValue( sal_Int16&                           o_rValue,
                           const css::uno::Any&                 rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract color value from Any
        bool extractValue( RGBColor&                            o_rValue,
                           const css::uno::Any&                 rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract color value from Any
        bool extractValue( HSLColor&                            o_rValue,
                           const css::uno::Any&                 rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract plain string from Any
        bool extractValue( OUString&                            o_rValue,
                           const css::uno::Any&                 rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract bool value from Any
        bool extractValue( bool&                                o_rValue,
                           const css::uno::Any&    rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract double 2-tuple from Any
        bool extractValue( basegfx::B2DTuple&                   o_rPair,
                           const css::uno::Any&                 rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /** Search a sequence of NamedValues for a given element.

            @return true, if the sequence contains the specified
            element.
         */
        bool findNamedValue( css::uno::Sequence< css::beans::NamedValue > const& rSequence,
                             const css::beans::NamedValue& rSearchKey );

        basegfx::B2DRange calcRelativeShapeBounds( const basegfx::B2DVector& rPageSize,
                                                   const basegfx::B2DRange&  rShapeBounds );

        /** Get the shape transformation from the attribute set

            @param rBounds
            Original shape bound rect (to substitute default attribute
            layer values)

            @param pAttr
            Attribute set. Might be NULL (then, rBounds is used to set
            a simple scale and translate of the unit rect to rBounds).
        */
        basegfx::B2DHomMatrix getShapeTransformation(
            const basegfx::B2DRange&                      rBounds,
            const boost::shared_ptr<ShapeAttributeLayer>& pAttr );

        /** Get a shape's sprite transformation from the attribute set

            @param rPixelSize
            Pixel size of the sprite

            @param rOrigSize
            Original shape size (i.e. the size of the actual sprite
            content, in the user coordinate system)

            @param pAttr
            Attribute set. Might be NULL (then, rBounds is used to set
            a simple scale and translate of the unit rect to rBounds).

            @return the transformation to be applied to the sprite.
        */
        basegfx::B2DHomMatrix getSpriteTransformation(
            const basegfx::B2DVector&                     rPixelSize,
            const basegfx::B2DVector&                     rOrigSize,
            const boost::shared_ptr<ShapeAttributeLayer>& pAttr );

        /** Calc update area for a shape.

            This method calculates the 'covered' area for the shape,
            i.e. the rectangle that is affected when rendering the
            shape. Apart from applying the given transformation to the
            shape rectangle, this method also takes attributes into
            account, which further scale the output (e.g. character
            sizes).

            @param rUnitBounds
            Shape bounds, in the unit rect coordinate space

            @param rShapeTransform
            Transformation matrix the shape should undergo.

            @param pAttr
            Current shape attributes
         */
        basegfx::B2DRange getShapeUpdateArea(
            const basegfx::B2DRange&                      rUnitBounds,
            const basegfx::B2DHomMatrix&                  rShapeTransform,
            const boost::shared_ptr<ShapeAttributeLayer>& pAttr );

        /** Calc update area for a shape.

            This method calculates the 'covered' area for the shape,
            i.e. the rectangle that is affected when rendering the
            shape. The difference from the other getShapeUpdateArea()
            method is the fact that this one works without
            ShapeAttributeLayer, and only scales up the given shape
            user coordinate bound rect. The method is typically used
            to retrieve user coordinate system bound rects for shapes
            which are smaller than the default unit bound rect
            (because e.g. of subsetting)

            @param rUnitBounds
            Shape bounds, in the unit rect coordinate space

            @param rShapeBounds
            Current shape bounding box in user coordinate space.
         */
        basegfx::B2DRange getShapeUpdateArea( const basegfx::B2DRange& rUnitBounds,
                                              const basegfx::B2DRange& rShapeBounds );

        /** Calc output position and size of shape, according to given
            attribute layer.

            Rotations, shears etc. and not taken into account,
            i.e. the returned rectangle is NOT the bounding box. Use
            it as if aBounds.getMinimum() is the output position and
            aBounds.getRange() the scaling of the shape.
         */
        basegfx::B2DRange getShapePosSize(
            const basegfx::B2DRange&                      rOrigBounds,
            const boost::shared_ptr<ShapeAttributeLayer>& pAttr );

        /** Convert a plain UNO API 32 bit int to RGBColor
         */
        RGBColor unoColor2RGBColor( sal_Int32 );
        /** Convert an IntSRGBA to plain UNO API 32 bit int
         */
        sal_Int32 RGBAColor2UnoColor( cppcanvas::Color::IntSRGBA );

        /** Fill a plain rectangle on the given canvas with the given color
         */
        void fillRect( const std::shared_ptr< cppcanvas::Canvas >& rCanvas,
                       const basegfx::B2DRange&                      rRect,
                       cppcanvas::Color::IntSRGBA                    aFillColor );

        /** Init canvas with default background (white)
         */
        void initSlideBackground( const std::shared_ptr< cppcanvas::Canvas >& rCanvas,
                                  const basegfx::B2IVector&                     rSize );

        /// Gets a random ordinal [0,n)
        inline ::std::size_t getRandomOrdinal( const ::std::size_t n )
        {
            return comphelper::rng::uniform_size_distribution(0, n-1);
        }

        /// To work around ternary operator in initializer lists
        /// (Solaris compiler problems)
#ifdef SOLARIS
        template <typename T>
        inline T const & ternary_op(
            const bool cond, T const & arg1, T const & arg2 )
        {
            if (cond)
                return arg1;
            else
                return arg2;
        }
#endif

        template <typename ValueType>
        inline bool getPropertyValue(
            ValueType & rValue,
            css::uno::Reference<
            css::beans::XPropertySet> const & xPropSet,
            OUString const & propName )
        {
            try {
                const css::uno::Any& a(
                    xPropSet->getPropertyValue( propName ) );
                bool const bRet = (a >>= rValue);
#if OSL_DEBUG_LEVEL > 0
                if( !bRet )
                    OSL_TRACE( "%s: while retrieving property %s, cannot extract Any of type %s\n",
                               OUStringToOString( propName,
                                                         RTL_TEXTENCODING_ASCII_US ).getStr(),
                               BOOST_CURRENT_FUNCTION,
                               OUStringToOString( a.getValueTypeRef()->pTypeName,
                                                         RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
                return bRet;
            }
            catch (css::uno::RuntimeException &)
            {
                throw;
            }
            catch (css::uno::Exception &)
            {
                return false;
            }
        }

        template <typename ValueType>
        inline bool getPropertyValue(
            css::uno::Reference< ValueType >& rIfc,
            css::uno::Reference< css::beans::XPropertySet> const & xPropSet,
            OUString const & propName )
        {
            try
            {
                const css::uno::Any& a(xPropSet->getPropertyValue( propName ));
                rIfc.set( a, css::uno::UNO_QUERY );

                bool const bRet = rIfc.is();
#if OSL_DEBUG_LEVEL > 0
                if( !bRet )
                    OSL_TRACE( "%s: while retrieving property %s, cannot extract Any of type %s to interface\n",
                               OUStringToOString( propName,
                                                         RTL_TEXTENCODING_ASCII_US ).getStr(),
                               BOOST_CURRENT_FUNCTION,
                               OUStringToOString( a.getValueTypeRef()->pTypeName,
                                                         RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
                return bRet;
            }
            catch (css::uno::RuntimeException &)
            {
                throw;
            }
            catch (css::uno::Exception &)
            {
                return false;
            }
        }

        /// Get the content of the BoundRect shape property
        basegfx::B2DRange getAPIShapeBounds( const css::uno::Reference< css::drawing::XShape >& xShape );

/*
        TODO(F1): When ZOrder someday becomes usable enable this

        /// Get the content of the ZOrder shape property
        double getAPIShapePrio( const css::uno::Reference< css::drawing::XShape >& xShape );
*/

        basegfx::B2IVector getSlideSizePixel( const basegfx::B2DVector&         rSize,
                                              const std::shared_ptr<UnoView>& pView );
    }

    // TODO(Q1): this could possibly be implemented with a somewhat
    // more lightweight template, by having the actual worker receive
    // only a function pointer, and a thin templated wrapper around
    // that which converts member functions into that.

    /** Apply given functor to every animation node child.

        @param xNode
        Parent node

        @param rFunctor
        Functor to apply. The functor must have an appropriate
        operator()( const css::uno::Reference< css::animations::XAnimationNode >& ) member.

        @return true, if the functor was successfully applied to
        all children, false otherwise.
    */
    template< typename Functor > inline bool for_each_childNode( const css::uno::Reference< css::animations::XAnimationNode >&    xNode,
                                                                 Functor&                                                                                  rFunctor )
    {
        try
        {
            // get an XEnumerationAccess to the children
            css::uno::Reference< css::container::XEnumerationAccess >
                   xEnumerationAccess( xNode,
                                       css::uno::UNO_QUERY_THROW );
            css::uno::Reference< css::container::XEnumeration >
                   xEnumeration( xEnumerationAccess->createEnumeration(),
                                 css::uno::UNO_QUERY_THROW );

            while( xEnumeration->hasMoreElements() )
            {
                css::uno::Reference< css::animations::XAnimationNode >
                           xChildNode( xEnumeration->nextElement(),
                                      css::uno::UNO_QUERY_THROW );
                rFunctor( xChildNode );
            }
                                                                                                                                                                                                                           return true;
        }
        catch( css::uno::Exception& )
        {
            return false;
        }
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_TOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
