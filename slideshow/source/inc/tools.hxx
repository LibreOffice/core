/*************************************************************************
 *
 *  $RCSfile: tools.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:22:22 $
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
            Shape bound rect

            @param pAttr
            Attribute set. Might be NULL

            @param bWithTranslation
            Whether the transformation should contain translation
            to shape output position, or not (e.g. for sprites)
        */
        ::basegfx::B2DHomMatrix getShapeTransformation( const ::basegfx::B2DRectangle&      rOrigBounds,
                                                        const ::basegfx::B2DRectangle&      rBounds,
                                                        const ShapeAttributeLayerSharedPtr& pAttr,
                                                        bool                                bWithTranslation );

        /** Calc update area for a shape.

            This method calculates the 'covered' area for the shape,
            i.e. the rectangle that is affected when rendering the
            shape.

            @param rShapeBounds
            Bound rect of the shape (without any transformations taken
            into account).

            @param rShapeTransform
            Transformation matrix the shape should undergo.

            @param pAttr
            Current shape attributes
         */
        ::basegfx::B2DRectangle getShapeUpdateArea( const ::basegfx::B2DRectangle&      rShapeBounds,
                                                    const ::basegfx::B2DHomMatrix&      rShapeTransform,
                                                    const ShapeAttributeLayerSharedPtr& pAttr );

        /** Convert a plain UNO API 32 bit int to RGBColor
         */
        RGBColor unoColor2RGBColor( sal_Int32 );

    }
}

#endif /* _SLIDESHOW_TOOLS_HXX */
