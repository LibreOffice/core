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

#ifndef INCLUDED_CANVAS_CANVASTOOLS_HXX
#define INCLUDED_CANVAS_CANVASTOOLS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include <math.h>
#include <string.h>
#include <vector>
#include <limits>

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
} } } }

namespace com { namespace sun { namespace star { namespace awt
{
    struct Rectangle;
    class  XWindow2;
} } } }

namespace com { namespace sun { namespace star { namespace beans {
    struct PropertyValue;
} } } }

class Color;
class OutputDevice;

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
        CANVASTOOLS_DLLPUBLIC css::geometry::RealSize2D createInfiniteSize2D();


        // View- and RenderState utilities


        CANVASTOOLS_DLLPUBLIC css::rendering::RenderState&
            initRenderState( css::rendering::RenderState&                      renderState );

        CANVASTOOLS_DLLPUBLIC css::rendering::ViewState&
            initViewState( css::rendering::ViewState&                          viewState );

        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DHomMatrix&
            getViewStateTransform( ::basegfx::B2DHomMatrix&                    transform,
                                   const css::rendering::ViewState&            viewState );

        CANVASTOOLS_DLLPUBLIC css::rendering::ViewState&
            setViewStateTransform( css::rendering::ViewState&                  viewState,
                                   const ::basegfx::B2DHomMatrix&              transform );

        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DHomMatrix&
            getRenderStateTransform( ::basegfx::B2DHomMatrix&                  transform,
                                     const css::rendering::RenderState&        renderState );

        CANVASTOOLS_DLLPUBLIC css::rendering::RenderState&
            setRenderStateTransform( css::rendering::RenderState&              renderState,
                                     const ::basegfx::B2DHomMatrix&            transform );

        CANVASTOOLS_DLLPUBLIC css::rendering::RenderState&
            appendToRenderState( css::rendering::RenderState&                  renderState,
                                 const ::basegfx::B2DHomMatrix&                transform );

        CANVASTOOLS_DLLPUBLIC css::rendering::RenderState&
            prependToRenderState( css::rendering::RenderState&                 renderState,
                                  const ::basegfx::B2DHomMatrix&               transform );

        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DHomMatrix&
            mergeViewAndRenderTransform( ::basegfx::B2DHomMatrix&              transform,
                                         const css::rendering::ViewState&      viewState,
                                         const css::rendering::RenderState&    renderState );


        // Matrix utilities


        CANVASTOOLS_DLLPUBLIC css::geometry::AffineMatrix2D&
            setIdentityAffineMatrix2D( css::geometry::AffineMatrix2D&  matrix );

        CANVASTOOLS_DLLPUBLIC css::geometry::Matrix2D&
            setIdentityMatrix2D( css::geometry::Matrix2D&              matrix );


        // Special utilities


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
            transformed rectangle is not represented as an axis-aligned
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
            is to be retrieved

            @param o_rxParams
            Output parameter, the sequence of Anys that hold the device parameters. Layout is as described above

            @return A reference to the resulting sequence of parameters
        */
        CANVASTOOLS_DLLPUBLIC css::uno::Sequence< css::uno::Any >& getDeviceInfo(
            const css::uno::Reference< css::rendering::XCanvas >& i_rxCanvas,
            css::uno::Sequence< css::uno::Any >& o_rxParams );

        /** Return a color space for a default RGBA integer format

            Use this method for dead-simple bitmap implementations,
            that map all their formats to 8888 RGBA color.
         */
        CANVASTOOLS_DLLPUBLIC css::uno::Reference< css::rendering::XIntegerBitmapColorSpace> const & getStdColorSpace();

        /** Return a color space for a default RGB integer format

            Use this method for dead-simple bitmap implementations,
            that map all their formats to 8888 RGB color (the last byte
            is unused).
         */
        CANVASTOOLS_DLLPUBLIC css::uno::Reference< css::rendering::XIntegerBitmapColorSpace> const & getStdColorSpaceWithoutAlpha();

        /** Return a memory layout for a default RGBA integer format

            Use this method for dead-simple bitmap implementations,
            that map all their formats to 8888 RGBA color.
         */
        CANVASTOOLS_DLLPUBLIC css::rendering::IntegerBitmapLayout getStdMemoryLayout(
            const css::geometry::IntegerSize2D& rBitmapSize );

        /// Convert standard 8888 RGBA color to vcl color
        CANVASTOOLS_DLLPUBLIC css::uno::Sequence<sal_Int8> colorToStdIntSequence( const ::Color& rColor );

        // Modelled closely after boost::numeric_cast, only that we
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

            if( ( arg<0 && !TargetLimits::is_signed) ||                     // losing the sign here
                ( SourceLimits::is_signed && arg<TargetLimits::min()) ||    // underflow will happen
                ( arg>TargetLimits::max() ) )                               // overflow will happen
            {
# if OSL_DEBUG_LEVEL > 2
                SAL_WARN("canvas", "numeric_cast detected data loss");
#endif
                throw css::uno::RuntimeException(
                    "numeric_cast detected data loss",
                    nullptr );
            }

            return static_cast<Target>(arg);
        }

        CANVASTOOLS_DLLPUBLIC css::awt::Rectangle getAbsoluteWindowRect(
            const css::awt::Rectangle&                       rRect,
            const css::uno::Reference< css::awt::XWindow2 >& xWin  );

        /** Retrieve for small bound marks around each corner of the given rectangle
         */
        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DPolyPolygon getBoundMarksPolyPolygon( const ::basegfx::B2DRange& rRange );

        /** Calculate number of gradient "strips" to generate (takes
           into account device resolution)

           @param nColorSteps
           Maximal integer difference between all color stops, needed
           for smooth gradient color differences
         */
        CANVASTOOLS_DLLPUBLIC int calcGradientStepCount( ::basegfx::B2DHomMatrix&   rTotalTransform,
                                   const css::rendering::ViewState&   viewState,
                                   const css::rendering::RenderState& renderState,
                                   const css::rendering::Texture&     texture,
                                   int                                nColorSteps );

        /** A very simplistic map for ASCII strings and arbitrary value
            types.

            This class internally references a constant, static array of
            sorted MapEntries, and performs a binary search to look up
            values for a given query string. Note that this map is static,
            i.e. not meant to be extended at runtime.

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
                const OString aStr( pMap->maKey );
                if( !mbCaseSensitive &&
                    aStr != aStr.toAsciiLowerCase() )
                {
                    SAL_WARN("canvas", "ValueMap::ValueMap(): Key is not lowercase " << pMap->maKey);
                }

                if( mnEntries > 1 )
                {
                    for( ::std::size_t i=0; i<mnEntries-1; ++i, ++pMap )
                    {
                        if( !mapComparator(pMap[0], pMap[1]) &&
                            mapComparator(pMap[1], pMap[0]) )
                        {
                            SAL_WARN("canvas", "ValueMap::ValueMap(): Map is not sorted, keys are wrong, "
                                      << pMap[0].maKey << " and " << pMap[1].maKey);
                            OSL_FAIL( "ValueMap::ValueMap(): Map is not sorted" );
                        }

                        const OString aStr2( pMap[1].maKey );
                        if( !mbCaseSensitive &&
                            aStr2 != aStr2.toAsciiLowerCase() )
                        {
                            SAL_WARN("canvas", "ValueMap::ValueMap(): Key is not lowercase" << pMap[1].maKey);
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
            bool lookup( const OUString& rName,
                         ValueType&             o_rResult ) const
            {
                // rName is required to contain only ASCII characters.
                // TODO(Q1): Enforce this at upper layers
                OString aKey( OUStringToOString( mbCaseSensitive ? rName : rName.toAsciiLowerCase(),
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

        CANVASTOOLS_DLLPUBLIC void clipOutDev(const css::rendering::ViewState& viewState,
                        const css::rendering::RenderState& renderState,
                        OutputDevice& rOutDev,
                        OutputDevice* p2ndOutDev=nullptr);

        CANVASTOOLS_DLLPUBLIC void extractExtraFontProperties(const css::uno::Sequence<css::beans::PropertyValue>& rExtraFontProperties,
                        sal_uInt32& rEmphasisMark);
    }
}

#endif /* INCLUDED_CANVAS_CANVASTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
