/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/uno/Reference.hxx>
#include <cpo/uno/Sequence.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

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

namespace com::sun::star::geometry
{
    struct RealSize2D;
    struct IntegerSize2D;
    struct AffineMatrix2D;
    struct Matrix2D;
}

namespace vclcanvas
{
    struct RenderState;
    struct Texture;
    struct ViewState;
}
namespace com::sun::star::awt
{
    struct Rectangle;
    class  XWindow2;
}

namespace com::sun::star::beans {
    struct PropertyValue;
}

class Color;
class OutputDevice;

namespace canvastools
{
        /**
         *
         * Count the number of 1-bits of a n-bit value
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

        // View- and RenderState utilities


        CANVASTOOLS_DLLPUBLIC ::vclcanvas::RenderState&
            initRenderState( ::vclcanvas::RenderState&                      renderState );

        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DHomMatrix&
            mergeViewAndRenderTransform( ::basegfx::B2DHomMatrix&              transform,
                                         const ::vclcanvas::ViewState&      viewState,
                                         const ::vclcanvas::RenderState&    renderState );


        // Matrix utilities


        CANVASTOOLS_DLLPUBLIC css::geometry::Matrix2D&
            setIdentityMatrix2D( css::geometry::Matrix2D&              matrix );


        // Special utilities


        /** Calc the bounding rectangle of a transformed rectangle.

            The method applies the given transformation to the
            specified input rectangle, and returns the bounding box of
            the resulting output area.

            @param i_Rect
            Input rectangle

            @param i_Transformation
            Transformation to apply to the input rectangle

            @return the resulting rectangle
         */
        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DRange calcTransformedRectBounds(
                                                        const ::basegfx::B2DRange&      i_Rect,
                                                        const ::basegfx::B2DHomMatrix&  i_Transformation );

        /** Calc a transform that maps the upper, left corner of a
             rectangle to the origin.

            The method is a specialized version of
            calcRectToRectTransform() (Removed now), mapping the input rectangle's
            the upper, left corner to the origin, and leaving the size
            untouched.

            @param i_srcRect
            Input parameter, specifies the original source
            rectangle. The resulting transformation will exactly map
            the source rectangle's upper, left corner to the origin.

            @param i_transformation
            The original transformation matrix. This is changed with
            translations (if necessary), to exactly map the source
            rectangle to the origin.

            @return the resulting transformation matrix

            @see calcRectToRectTransform()
            @see calcTransformedRectBounds()
        */
        CANVASTOOLS_DLLPUBLIC ::basegfx::B2DHomMatrix calcRectToOriginTransform(
                                                            const ::basegfx::B2DRange&      i_srcRect,
                                                            const ::basegfx::B2DHomMatrix&  i_transformation );

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
                    u"numeric_cast detected data loss"_ustr,
                    nullptr );
            }

            return static_cast<Target>(arg);
        }

        /** Calculate number of gradient "strips" to generate (takes
           into account device resolution)

           @param nColorSteps
           Maximal integer difference between all color stops, needed
           for smooth gradient color differences
         */
        CANVASTOOLS_DLLPUBLIC int calcGradientStepCount( ::basegfx::B2DHomMatrix&   rTotalTransform,
                                   const ::vclcanvas::ViewState&   viewState,
                                   const ::vclcanvas::RenderState& renderState,
                                   const ::vclcanvas::Texture&     texture,
                                   int                                nColorSteps );

        CANVASTOOLS_DLLPUBLIC void clipOutDev(const ::vclcanvas::ViewState& viewState,
                        const ::vclcanvas::RenderState& renderState,
                        OutputDevice& rOutDev);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
