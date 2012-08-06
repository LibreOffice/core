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

#ifndef INCLUDED_CANVAS_CANVASTOOLS_HXX
#define INCLUDED_CANVAS_CANVASTOOLS_HXX

#include <rtl/math.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>

#include <string.h> // for strcmp
#include <vector>
#include <limits>
#include <algorithm>

#include <canvas/canvastoolsdllapi.h>

namespace basegfx
{
    class B2DHomMatrix;
    class B2DRange;
    class B2IRange;
    class B2IPoint;
    class B2DPolyPolygon;
}

namespace com { namespace sun { namespace star { namespace geometry
{
    struct RealSize2D;
    struct IntegerSize2D;
    struct AffineMatrix2D;
    struct Matrix2D;
} } } }

namespace com { namespace sun { namespace star { namespace rendering
{
    struct RenderState;
    struct ViewState;
    struct IntegerBitmapLayout;
    class  XCanvas;
    struct Texture;
    class  XIntegerBitmapColorSpace;
    class  XPolyPolygon2D;

    bool operator==( const RenderState& rLHS,
                     const RenderState& rRHS );

    bool operator==( const ViewState& rLHS,
                     const ViewState& rRHS );
} } } }

namespace com { namespace sun { namespace star { namespace awt
{
    struct Rectangle;
    class  XWindow2;
} } } }

class Color;

namespace canvas
{
    namespace tools
    {
        /** Compute the next highest power of 2 of a 32-bit value

            Code devised by Sean Anderson, in good ole HAKMEM
            tradition.

            @return 1 << (lg(x - 1) + 1)
        */
        inline sal_uInt32 nextPow2( sal_uInt32 x )
        {
            --x;
            x |= x >> 1;
            x |= x >> 2;
            x |= x >> 4;
            x |= x >> 8;
            x |= x >> 16;

            return ++x;
        }

        /**
         *
         * Count the number of 1-bits of an n-bit value
         *
         */

        // mickey's math tricks...
        inline unsigned int pow2( unsigned int c ) { return 0x1 << c; }
        inline unsigned int mask( unsigned int c ) { return ((unsigned int)(-1)) / (pow2(pow2(c)) + 1); }
        inline unsigned int count( unsigned int x, unsigned int c ) { return ((x) & mask(c)) + (((x) >> (pow2(c))) & mask(c)); }
        template<typename T>
        inline unsigned int bitcount( T c ) {
            unsigned int nByteIndex = 0;
            unsigned int nNumBytes = sizeof(T)<<2;
            do {
                c=count(c,nByteIndex++);
                nNumBytes >>= 1;
            } while(nNumBytes);
            return c;
        }
        inline sal_uInt32 bitcount32( sal_uInt32 c ) {
            c=count(c,0);
            c=count(c,1);
            c=count(c,2);
            c=count(c,3);
            c=count(c,4);
            return c;
        }

        /** Round given floating point value down to next integer
         */
        inline sal_Int32 roundDown( const double& rVal )
        {
            return static_cast< sal_Int32 >( floor( rVal ) );
        }

        /** Round given floating point value up to next integer
         */
        inline sal_Int32 roundUp( const double& rVal )
        {
            return static_cast< sal_Int32 >( ceil( rVal ) );
        }

        /** Create a RealSize2D with both coordinate values set to +infinity
         */
        CANVASTOOLS_DLLPUBLIC ::com::sun::star::geometry::RealSize2D createInfiniteSize2D();


        // View- and RenderState utilities
        // ===================================================================

        CANVASTOOLS_DLLPUBLIC ::com::sun::star::rendering::RenderState&
            initRenderState( ::com::sun::star::rendering::RenderState&                      renderState );

        CANVASTOOLS_DLLPUBLIC ::com::sun::star::rendering::ViewState&
            initViewState( ::com::sun::star::rendering::ViewState&                          viewState );

        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DHomMatrix&
            getViewStateTransform( ::basegfx::B2DHomMatrix&                                 transform,
                                   const ::com::sun::star::rendering::ViewState&            viewState );

        CANVASTOOLS_DLLPUBLIC ::com::sun::star::rendering::ViewState&
            setViewStateTransform( ::com::sun::star::rendering::ViewState&                  viewState,
                                   const ::basegfx::B2DHomMatrix&                           transform );

        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DHomMatrix&
            getRenderStateTransform( ::basegfx::B2DHomMatrix&                               transform,
                                     const ::com::sun::star::rendering::RenderState&        renderState );

        CANVASTOOLS_DLLPUBLIC ::com::sun::star::rendering::RenderState&
            setRenderStateTransform( ::com::sun::star::rendering::RenderState&              renderState,
                                     const ::basegfx::B2DHomMatrix&                         transform );

        CANVASTOOLS_DLLPUBLIC ::com::sun::star::rendering::RenderState&
            appendToRenderState( ::com::sun::star::rendering::RenderState&                  renderState,
                                 const ::basegfx::B2DHomMatrix&                             transform );

        CANVASTOOLS_DLLPUBLIC ::com::sun::star::rendering::RenderState&
            prependToRenderState( ::com::sun::star::rendering::RenderState&                 renderState,
                                  const ::basegfx::B2DHomMatrix&                            transform );

        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DHomMatrix&
            mergeViewAndRenderTransform( ::basegfx::B2DHomMatrix&                           transform,
                                         const ::com::sun::star::rendering::ViewState&      viewState,
                                         const ::com::sun::star::rendering::RenderState&    renderState );


        // Matrix utilities
        // ===================================================================

        CANVASTOOLS_DLLPUBLIC ::com::sun::star::geometry::AffineMatrix2D&
            setIdentityAffineMatrix2D( ::com::sun::star::geometry::AffineMatrix2D&  matrix );

        CANVASTOOLS_DLLPUBLIC ::com::sun::star::geometry::Matrix2D&
            setIdentityMatrix2D( ::com::sun::star::geometry::Matrix2D&              matrix );


        // Special utilities
        // ===================================================================

        /** Calc the bounding rectangle of a transformed rectangle.

            The method applies the given transformation to the
            specified input rectangle, and returns the bounding box of
            the resulting output area.

            @param o_Rect
            Output rectangle

            @param i_Rect
            Input rectangle

            @param i_Transformation
            Transformation to apply to the input rectangle

            @return a reference to the resulting rectangle
         */
        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DRange& calcTransformedRectBounds( ::basegfx::B2DRange&          o_Rect,
                                                        const ::basegfx::B2DRange&      i_Rect,
                                                        const ::basegfx::B2DHomMatrix&  i_Transformation );

        /** Calc a transform that maps the upper, left corner of a
             rectangle to the origin.

            The method is a specialized version of
            calcRectToRectTransform() (Removed now), mapping the input rectangle's
            the upper, left corner to the origin, and leaving the size
            untouched.

            @param o_transform
            Output parameter, to receive the resulting transformation
            matrix.

            @param i_srcRect
            Input parameter, specifies the original source
            rectangle. The resulting transformation will exactly map
            the source rectangle's upper, left corner to the origin.

            @param i_transformation
            The original transformation matrix. This is changed with
            translations (if necessary), to exactly map the source
            rectangle to the origin.

            @return a reference to the resulting transformation matrix

            @see calcRectToRectTransform()
            @see calcTransformedRectBounds()
        */
        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DHomMatrix& calcRectToOriginTransform( ::basegfx::B2DHomMatrix&      o_transform,
                                                            const ::basegfx::B2DRange&      i_srcRect,
                                                            const ::basegfx::B2DHomMatrix&  i_transformation );

        /** Check whether a given rectangle is within another
            transformed rectangle.

            This method checks for polygonal containedness, i.e. the
            transformed rectangle is not represented as an axis-alignd
            rectangle anymore (like calcTransformedRectBounds()), but
            polygonal. Thus, the insideness test is based on tight
            bounds.

            @param rContainedRect
            This rectangle is checked, whether it is fully within the
            transformed rTransformRect.

            @param rTransformRect
            This rectangle is transformed, and then checked whether it
            fully contains rContainedRect.

            @param rTransformation
            This transformation is applied to rTransformRect
         */
        CANVASTOOLS_DLLPUBLIC bool isInside( const ::basegfx::B2DRange&         rContainedRect,
                       const ::basegfx::B2DRange&       rTransformRect,
                       const ::basegfx::B2DHomMatrix&   rTransformation );

        /** Clip a scroll to the given bound rect

            @param io_rSourceArea
            Source area to scroll. The resulting clipped source area
            is returned therein.

            @param io_rDestPoint
            Destination point of the scroll (upper, left corner of
            rSourceArea after the scroll). The new, resulting
            destination point is returned therein.q

            @param o_ClippedAreas
            Vector of rectangles in the <em>destination</em> area
            coordinate system, which are clipped away from the source
            area, and thus need extra updates (i.e. they are not
            correctly copy from the scroll operation, since there was
            no information about them in the source).

            @param rBounds
            Bounds to clip against.

            @return false, if the resulting scroll area is empty
         */
        CANVASTOOLS_DLLPUBLIC bool clipScrollArea( ::basegfx::B2IRange&                  io_rSourceArea,
                             ::basegfx::B2IPoint&                  io_rDestPoint,
                             ::std::vector< ::basegfx::B2IRange >& o_ClippedAreas,
                             const ::basegfx::B2IRange&            rBounds );

        /** Clip a blit between two differently surfaces.

            This method clips source and dest rect for a clip between
            two differently clipped surfaces, such that the resulting
            blit rects are fully within both clip areas.

            @param io_rSourceArea
            Source area of the blit. Returned therein is the computed
            clipped source area.

            @param io_rDestPoint
            Dest area of the blit. Returned therein is the computed
            clipped dest area.

            @param rSourceBounds
            Clip bounds of the source surface

            @param rDestBounds
            Clip bounds of the dest surface

            @return false, if the resulting blit is empty, i.e. fully
            clipped away.
         */
        CANVASTOOLS_DLLPUBLIC ::basegfx::B2IRange spritePixelAreaFromB2DRange( const ::basegfx::B2DRange& rRange );

        /** Retrieve various internal properties of the actual canvas implementation.

            This method retrieves a bunch of internal, implementation-
            and platform-dependent values from the canvas
            implementation. Among them are for example operating
            system window handles. The actual layout and content of
            the returned sequence is dependent on the component
            implementation, undocumented and subject to change.

            @param i_rxCanvas
            Input parameter, the canvas representation for which the device information
            is to be retrieveds

            @param o_rxParams
            Output parameter, the sequence of Anys that hold the device parameters. Layout is as described above

            @return A reference to the resulting sequence of parameters
        */
        CANVASTOOLS_DLLPUBLIC ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& getDeviceInfo(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >& i_rxCanvas,
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& o_rxParams );

        /** Return a color space for a default RGBA integer format

            Use this method for dead-simple bitmap implementations,
            that map all their formats to 8888 RGBA color.
         */
        CANVASTOOLS_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XIntegerBitmapColorSpace> getStdColorSpace();

        /** Return a memory layout for a default RGBA integer format

            Use this method for dead-simple bitmap implementations,
            that map all their formats to 8888 RGBA color.
         */
        CANVASTOOLS_DLLPUBLIC ::com::sun::star::rendering::IntegerBitmapLayout getStdMemoryLayout(
            const ::com::sun::star::geometry::IntegerSize2D& rBitmapSize );

        /// Convert standard 8888 RGBA color to vcl color
        CANVASTOOLS_DLLPUBLIC ::Color stdIntSequenceToColor( const ::com::sun::star::uno::Sequence<sal_Int8>& rColor );

        /// Convert standard 8888 RGBA color to vcl color
        CANVASTOOLS_DLLPUBLIC ::com::sun::star::uno::Sequence<sal_Int8> colorToStdIntSequence( const ::Color& rColor );

        // Modeled closely after boost::numeric_cast, only that we
        // issue some trace output here and throw a RuntimeException

        /** Cast numeric value into another (numeric) data type

            Apart from converting the numeric value, this template
            also checks if any overflow, underflow, or sign
            information is lost (if yes, it throws an
            uno::RuntimeException.
         */
        template< typename Target, typename Source > inline Target numeric_cast( Source arg )
        {
            // typedefs abbreviating respective trait classes
            typedef ::std::numeric_limits< Source > SourceLimits;
            typedef ::std::numeric_limits< Target > TargetLimits;

#undef min
#undef max

            if( ( arg<0 && !TargetLimits::is_signed) ||                     // loosing the sign here
                ( SourceLimits::is_signed && arg<TargetLimits::min()) ||    // underflow will happen
                ( arg>TargetLimits::max() ) )                               // overflow will happen
            {
# if OSL_DEBUG_LEVEL > 2
                OSL_TRACE("numeric_cast detected data loss");
#endif
                throw ::com::sun::star::uno::RuntimeException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "numeric_cast detected data loss" )),
                    NULL );
            }

            return static_cast<Target>(arg);
        }

        CANVASTOOLS_DLLPUBLIC ::com::sun::star::awt::Rectangle getAbsoluteWindowRect(
            const ::com::sun::star::awt::Rectangle&                                    rRect,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow2 >& xWin  );

        /** Retrieve for small bound marks around each corner of the given rectangle
         */
        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DPolyPolygon getBoundMarksPolyPolygon( const ::basegfx::B2DRange& rRange );

        /** Calculate number of gradient "strips" to generate (takes
           into account device resolution)

           @param nColorSteps
           Maximal integer difference between all color stops, needed
           for smooth gradient color differences
         */
        CANVASTOOLS_DLLPUBLIC int calcGradientStepCount( ::basegfx::B2DHomMatrix&                        rTotalTransform,
                                   const ::com::sun::star::rendering::ViewState&   viewState,
                                   const ::com::sun::star::rendering::RenderState& renderState,
                                   const ::com::sun::star::rendering::Texture&     texture,
                                   int                                             nColorSteps );

        /** A very simplistic map for ASCII strings and arbitrary value
            types.

            This class internally references a constant, static array of
            sorted MapEntries, and performs a binary search to look up
            values for a given query string. Note that this map is static,
            i.e. not meant to be extented at runtime.

            @tpl ValueType
            The value type this map should store, associated with an ASCII
            string.
        */
        template< typename ValueType > class ValueMap
        {
        public:
            struct MapEntry
            {
                const char*     maKey;
                ValueType       maValue;
            };

            /** Create a ValueMap for the given array of MapEntries.

                @param pMap
                Pointer to a <em>static</em> array of MapEntries. Must
                live longer than this object! Make absolutely sure that
                the string entries passed via pMap are ASCII-only -
                everything else might not yield correct string
                comparisons, and thus will result in undefined behaviour.

                @param nEntries
                Number of entries for pMap

                @param bCaseSensitive
                Whether the map query should be performed case sensitive
                or not. When bCaseSensitive is false, all MapEntry strings
                must be lowercase!
            */
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
                    OSL_FAIL( "ValueMap::ValueMap(): Key is not lowercase" );
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
                            OSL_FAIL( "ValueMap::ValueMap(): Map is not sorted" );
                        }

                        const ::rtl::OString aStr2( pMap[1].maKey );
                        if( !mbCaseSensitive &&
                            aStr2 != aStr2.toAsciiLowerCase() )
                        {
                            OSL_TRACE("ValueMap::ValueMap(): Key %s is not lowercase",
                                      pMap[1].maKey);
                            OSL_FAIL( "ValueMap::ValueMap(): Key is not lowercase" );
                        }
                    }
                }
#endif
            }

            /** Lookup a value for the given query string

                @param rName
                The string to lookup. If the map was created with the case
                insensitive flag, the lookup is performed
                case-insensitive, otherwise, case-sensitive.

                @param o_rResult
                Output parameter, which receives the value associated with
                the query string. If no value was found, the referenced
                object is kept unmodified.

                @return true, if a matching entry was found.
            */
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
    }
}

#endif /* INCLUDED_CANVAS_CANVASTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
