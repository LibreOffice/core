/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tools.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:22:24 $
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

#ifndef _SLIDESHOW_TOOLS_HXX
#define _SLIDESHOW_TOOLS_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif
#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#include <basegfx/tuple/b2dtuple.hxx>
#endif
#ifndef _BGFX_VECTOR_B2DSIZE_HXX
#include <basegfx/vector/b2dsize.hxx>
#endif

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#include <shapeattributelayer.hxx>

#include <string.h> // for strcmp
#include <algorithm>

#include <shape.hxx>
#include <rgbcolor.hxx>
#include <hslcolor.hxx>
#include <layermanager.hxx>

#include "boost/optional.hpp"
#include <cstdlib>


namespace com { namespace sun { namespace star { namespace beans
{
    struct NamedValue;
} } } }


/* Definition of some animation tools */

namespace presentation
{
    namespace internal
    {
        // Value extraction from Any
        // =========================

        /// extract unary double value from Any
        bool extractValue( double&                              o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const ShapeSharedPtr&                rShape,
                           const LayerManagerSharedPtr&         rLayerManager );

        /// extract enum/constant group value from Any
        bool extractValue( sal_Int16&                           o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const ShapeSharedPtr&                rShape,
                           const LayerManagerSharedPtr&         rLayerManager );

        /// extract color value from Any
        bool extractValue( RGBColor&                            o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const ShapeSharedPtr&                rShape,
                           const LayerManagerSharedPtr&         rLayerManager );

        /// extract color value from Any
        bool extractValue( HSLColor&                            o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const ShapeSharedPtr&                rShape,
                           const LayerManagerSharedPtr&         rLayerManager );

        /// extract plain string from Any
        bool extractValue( ::rtl::OUString&                     o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const ShapeSharedPtr&                rShape,
                           const LayerManagerSharedPtr&         rLayerManager );

        /// extract bool value from Any
        bool extractValue( bool&                                o_rValue,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const ShapeSharedPtr&                rShape,
                           const LayerManagerSharedPtr&         rLayerManager );

        /// extract double 2-tuple from Any
        bool extractValue( ::basegfx::B2DTuple&                 o_rPair,
                           const ::com::sun::star::uno::Any&    rSourceAny,
                           const ShapeSharedPtr&                rShape,
                           const LayerManagerSharedPtr&         rLayerManager );

        /** Search a sequence of NamedValues for a given element.

            @return true, if the sequence contains the specified
            element.
         */
        bool findNamedValue( ::com::sun::star::uno::Sequence<
                                 ::com::sun::star::beans::NamedValue >&     rSequence,
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

        template< typename ValueType > class ValueMap
        {
        public:
            struct MapEntry
            {
                const char*     maKey;
                ValueType       maValue;
            };

            ValueMap( const MapEntry*   pMap,
                      ::std::size_t     nEntries,
                      bool              bCaseSensitive ) :
                mpMap( pMap ),
                mnEntries( nEntries ),
                mbCaseSensitive( bCaseSensitive )
            {
#ifdef DBG_UTIL
                // Ensure that map entries are sorted (and all lowercase, if this
                // map is case insensitive)
                const ::rtl::OString aStr( pMap->maKey );
                if( !mbCaseSensitive &&
                    aStr != aStr.toAsciiLowerCase() )
                {
                    OSL_TRACE("ValueMap::ValueMap(): Key %s is not lowercase",
                              pMap->maKey);
                    OSL_ENSURE( false, "ValueMap::ValueMap(): Key is not lowercase" );
                }

                if( mnEntries > 1 )
                {
                    for( ::std::size_t i=0; i<mnEntries-1; ++i, ++pMap )
                    {
                        if( !mapComparator(pMap[0], pMap[1]) &&
                            mapComparator(pMap[1], pMap[0]) )
                        {
                            OSL_TRACE("ValueMap::ValueMap(): Map is not sorted, keys %s and %s are wrong",
                                       pMap[0].maKey,
                                       pMap[1].maKey);
                            OSL_ENSURE( false,
                                        "ValueMap::ValueMap(): Map is not sorted" );
                        }

                        const ::rtl::OString aStr( pMap[1].maKey );
                        if( !mbCaseSensitive &&
                            aStr != aStr.toAsciiLowerCase() )
                        {
                            OSL_TRACE("ValueMap::ValueMap(): Key %s is not lowercase",
                                      pMap[1].maKey);
                            OSL_ENSURE( false, "ValueMap::ValueMap(): Key is not lowercase" );
                        }
                    }
                }
#endif
            }

            bool lookup( const ::rtl::OUString& rName,
                         ValueType&             o_rResult ) const
            {
                // rName is required to contain only ASCII characters.
                // TODO(Q1): Enforce this at upper layers
                ::rtl::OString aKey( ::rtl::OUStringToOString( mbCaseSensitive ? rName : rName.toAsciiLowerCase(),
                                                               RTL_TEXTENCODING_ASCII_US ) );
                MapEntry aSearchKey =
                    {
                        aKey.getStr(),
                        ValueType()
                    };

                const MapEntry* pRes;
                const MapEntry* pEnd = mpMap+mnEntries;
                if( (pRes=::std::lower_bound( mpMap,
                                              pEnd,
                                              aSearchKey,
                                              &mapComparator )) != pEnd )
                {
                    // place to _insert before_ found - is it equal to
                    // the search key?
                    if( strcmp( pRes->maKey, aSearchKey.maKey ) == 0 )
                    {
                        // yep, correct entry found
                        o_rResult = pRes->maValue;
                        return true;
                    }
                }

                // not found
                return false;
            }

        private:
            static bool mapComparator( const MapEntry& rLHS,
                                       const MapEntry& rRHS )
            {
                return strcmp( rLHS.maKey,
                               rRHS.maKey ) < 0;
            }

            const MapEntry*     mpMap;
            ::std::size_t       mnEntries;
            bool                mbCaseSensitive;
        };

        inline ::basegfx::B2DRectangle calcRelativeShapeBounds( const ::basegfx::B2DRectangle& rPageBounds,
                                                                const ::basegfx::B2DRectangle& rShapeBounds )
        {
            return ::basegfx::B2DRectangle( rShapeBounds.getMinX() / rPageBounds.getWidth(),
                                            rShapeBounds.getMinY() / rPageBounds.getHeight(),
                                            rShapeBounds.getMaxX() / rPageBounds.getWidth(),
                                            rShapeBounds.getMaxY() / rPageBounds.getHeight() );
        }

        /** Get the shape transformation from the attribute set

            @param rBounds
            Original shape bound rect (to substitute default attribute
            layer values)

            @param pAttr
            Attribute set. Might be NULL (then, rBounds is used to set
            a simple scale and translate of the unit rect to rBounds).
        */
        ::basegfx::B2DHomMatrix getShapeTransformation( const ::basegfx::B2DRectangle&      rBounds,
                                                        const ShapeAttributeLayerSharedPtr& pAttr );

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
        ::basegfx::B2DHomMatrix getSpriteTransformation( const ::basegfx::B2DSize&              rPixelSize,
                                                         const ::basegfx::B2DSize&              rOrigSize,
                                                         const ShapeAttributeLayerSharedPtr&    pAttr );

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
        ::basegfx::B2DRectangle getShapeUpdateArea( const ::basegfx::B2DRectangle&      rUnitBounds,
                                                    const ::basegfx::B2DHomMatrix&      rShapeTransform,
                                                    const ShapeAttributeLayerSharedPtr& pAttr );

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
        ::basegfx::B2DRectangle getShapeUpdateArea( const ::basegfx::B2DRectangle&      rUnitBounds,
                                                    const ::basegfx::B2DRectangle&      rShapeBounds );

        /** Calc output position and size of shape, according to given
            attribute layer.

            Rotations, shears etc. and not taken into account,
            i.e. the returned rectangle is NOT the bounding box. Use
            it as if aBounds.getMinimum() is the output position and
            aBounds.getRange() the scaling of the shape.
         */
        ::basegfx::B2DRectangle getShapePosSize( const ::basegfx::B2DRectangle&         rOrigBounds,
                                                 const ShapeAttributeLayerSharedPtr&    pAttr );

        /** Convert a plain UNO API 32 bit int to RGBColor
         */
        RGBColor unoColor2RGBColor( sal_Int32 );

        /** Init canvas with default background (white)
         */
        void initSlideBackground( const ::cppcanvas::CanvasSharedPtr& rCanvas,
                                  const ::basegfx::B2ISize&           rSize );

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
    }
}

#endif /* _SLIDESHOW_TOOLS_HXX */
