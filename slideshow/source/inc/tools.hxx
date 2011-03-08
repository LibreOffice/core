/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_SLIDESHOW_TOOLS_HXX
#define INCLUDED_SLIDESHOW_TOOLS_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <cppcanvas/color.hxx>

#include "shapeattributelayer.hxx"
#include "shape.hxx"
#include "rgbcolor.hxx"
#include "hslcolor.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/current_function.hpp>

#include <functional>
#include <cstdlib>
#include <string.h> // for strcmp
#include <algorithm>


#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


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

        // xxx todo: remove with boost::hash when 1.33 is available
        template <typename T>
        struct hash : ::std::unary_function<T, ::std::size_t>
        {
            ::std::size_t operator()( T const& val ) const {
                return hash_value(val);
            }
        };
        template <typename T>
        inline ::std::size_t hash_value( T * const& p )
        {
            ::std::size_t d = static_cast< ::std::size_t >(
                reinterpret_cast< ::std::ptrdiff_t >(p) );
            return d + (d >> 3);
        }

        // xxx todo: shift to namespace com::sun::star::uno when
        //           1.33 is available
        template <typename T>
        inline ::std::size_t hash_value(
            ::com::sun::star::uno::Reference<T> const& x )
        {
            // normalize to object root, because _only_ XInterface is defined
            // to be stable during object lifetime:
            ::com::sun::star::uno::Reference<
                  ::com::sun::star::uno::XInterface> const xRoot(
                      x, ::com::sun::star::uno::UNO_QUERY );
            return hash<void *>()(xRoot.get());
        }

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
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract int from Any
        bool extractValue( sal_Int32&                           o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract enum/constant group value from Any
        bool extractValue( sal_Int16&                           o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract color value from Any
        bool extractValue( RGBColor&                            o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract color value from Any
        bool extractValue( HSLColor&                            o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract plain string from Any
        bool extractValue( ::rtl::OUString&                     o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract bool value from Any
        bool extractValue( bool&                                o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /// extract double 2-tuple from Any
        bool extractValue( basegfx::B2DTuple&                   o_rPair,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const boost::shared_ptr<Shape>&      rShape,
                           const basegfx::B2DVector&            rSlideBounds );

        /** Search a sequence of NamedValues for a given element.

            @return true, if the sequence contains the specified
            element.
         */
        bool findNamedValue( ::com::sun::star::uno::Sequence<
                                 ::com::sun::star::beans::NamedValue > const& rSequence,
                             const ::com::sun::star::beans::NamedValue& rSearchKey );

        /** Search a sequence of NamedValues for an element with a given name.

            @param o_pRet
            If non-NULL, receives the full NamedValue found (if it was
            found, that is).

            @return true, if the sequence contains the specified
            element.
         */
        bool findNamedValue( ::com::sun::star::beans::NamedValue*       o_pRet,
                             const ::com::sun::star::uno::Sequence<
                                 ::com::sun::star::beans::NamedValue >&     rSequence,
                             const ::rtl::OUString&                     rSearchString );

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
        void fillRect( const boost::shared_ptr< cppcanvas::Canvas >& rCanvas,
                       const basegfx::B2DRange&                      rRect,
                       cppcanvas::Color::IntSRGBA                    aFillColor );

        /** Init canvas with default background (white)
         */
        void initSlideBackground( const boost::shared_ptr< cppcanvas::Canvas >& rCanvas,
                                  const basegfx::B2IVector&                     rSize );

        /// Gets a random ordinal [0,n)
        inline ::std::size_t getRandomOrdinal( const ::std::size_t n )
        {
            return static_cast< ::std::size_t >(
                double(n) * rand() / (RAND_MAX + 1.0) );
        }

        /// To work around ternary operator in initializer lists
        /// (Solaris compiler problems)
        template <typename T>
        inline T const & ternary_op(
            const bool cond, T const & arg1, T const & arg2 )
        {
            if (cond)
                return arg1;
            else
                return arg2;
        }

        template <typename ValueType>
        inline bool getPropertyValue(
            ValueType & rValue,
            com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertySet> const & xPropSet,
            rtl::OUString const & propName )
        {
            try {
                const com::sun::star::uno::Any& a(
                    xPropSet->getPropertyValue( propName ) );
                bool const bRet = (a >>= rValue);
#if OSL_DEBUG_LEVEL > 0
                if( !bRet )
                    OSL_TRACE( "%s: while retrieving property %s, cannot extract Any of type %s\n",
                               ::rtl::OUStringToOString( propName,
                                                         RTL_TEXTENCODING_ASCII_US ).getStr(),
                               BOOST_CURRENT_FUNCTION,
                               ::rtl::OUStringToOString( a.getValueTypeRef()->pTypeName,
                                                         RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
                return bRet;
            }
            catch (com::sun::star::uno::RuntimeException &)
            {
                throw;
            }
            catch (com::sun::star::uno::Exception &)
            {
                return false;
            }
        }

        template <typename ValueType>
        inline bool getPropertyValue(
            com::sun::star::uno::Reference< ValueType >& rIfc,
            com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertySet> const & xPropSet,
            rtl::OUString const & propName )
        {
            try
            {
                const com::sun::star::uno::Any& a(
                    xPropSet->getPropertyValue( propName ));
                rIfc.set( a,
                          com::sun::star::uno::UNO_QUERY );

                bool const bRet = rIfc.is();
#if OSL_DEBUG_LEVEL > 0
                if( !bRet )
                    OSL_TRACE( "%s: while retrieving property %s, cannot extract Any of type %s to interface\n",
                               ::rtl::OUStringToOString( propName,
                                                         RTL_TEXTENCODING_ASCII_US ).getStr(),
                               BOOST_CURRENT_FUNCTION,
                               ::rtl::OUStringToOString( a.getValueTypeRef()->pTypeName,
                                                         RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
                return bRet;
            }
            catch (com::sun::star::uno::RuntimeException &)
            {
                throw;
            }
            catch (com::sun::star::uno::Exception &)
            {
                return false;
            }
        }

        /// Get the content of the BoundRect shape property
        basegfx::B2DRange getAPIShapeBounds( const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::drawing::XShape >& xShape );

        /// Get the content of the ZOrder shape property
        double getAPIShapePrio( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::drawing::XShape >& xShape );

        basegfx::B2IVector getSlideSizePixel( const basegfx::B2DVector&         rSize,
                                              const boost::shared_ptr<UnoView>& pView );
    }
}

#endif /* INCLUDED_SLIDESHOW_TOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
