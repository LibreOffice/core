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

#ifndef INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_MTFTOOLS_HXX
#define INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_MTFTOOLS_HXX

#include <action.hxx>
#include <cppcanvas/canvas.hxx>


class VirtualDevice;
class Size;

namespace basegfx
{
    class B2DVector;
    class B2DPoint;
}
namespace com { namespace sun { namespace star { namespace rendering
{
    struct RenderState;
} } } }


namespace cppcanvas
{
    namespace internal
    {
        struct OutDevState;
    }

    namespace tools
    {
        /** Init render state from OutDevState

            This method initializes the given render state object,
            sets up the transformation and the clip from the
            OutDevState.
         */
        void initRenderState( css::rendering::RenderState&              renderState,
                              const ::cppcanvas::internal::OutDevState& outdevState );

        /** Calc output offset relative to baseline

            The XCanvas API always renders text relative to its
            baseline. This method calculates an offset in logical
            coordinates, depending on the OutDevState's
            textReferencePoint and the font currently set, to offset
            the text from the baseline.

            @param outdevState
            State to take textReferencePoint from

            @param rVDev
            VDev to obtain font metrics from.
         */
        ::Size getBaselineOffset( const ::cppcanvas::internal::OutDevState& outdevState,
                                  const VirtualDevice&                      rVDev );

        /** Construct a matrix that converts from logical to pixel
            coordinate system.

            This method calculates a matrix that approximates the
            VirtualDevice's LogicToPixel conversion (disregarding any
            offset components, thus the 'linear' in the method name -
            the returned matrix is guaranteed to be linear).

            @param o_rMatrix
            This matrix will receive the calculated transform, and is
            also returned from this method.

            @return the calculated transformation matrix.
         */
        ::basegfx::B2DHomMatrix& calcLogic2PixelLinearTransform( ::basegfx::B2DHomMatrix&   o_rMatrix,
                                                                 const VirtualDevice&       rVDev );

        /** Construct a matrix that converts from logical to pixel
            coordinate system.

            This method calculates a matrix that approximates the
            VirtualDevice's LogicToPixel conversion.

            @param o_rMatrix
            This matrix will receive the calculated transform, and is
            also returned from this method.

            @return the calculated transformation matrix.
         */
        ::basegfx::B2DHomMatrix& calcLogic2PixelAffineTransform( ::basegfx::B2DHomMatrix&   o_rMatrix,
                                                                 const VirtualDevice&       rVDev );

        /** This method modifies the clip, to cancel the given
            transformation.

            As the clip is relative to the render state
            transformation, offsetting or scaling the render state
            must modify the clip, to keep it at the same position
            relative to the primitive at hand

            @param o_rRenderState
            Render state to change the clip in

            @param rOutdevState
            Input state. Is used to retrieve the original clip from

            @param rOffset
            The clip is offsetted by the negative of this value.

            @param pScaling
            The clip is inversely scaled by this value (if given)

            @param pRotation
            The clip is inversely rotated by this value (if given)

            @return true, if the clip has changed, false if not
         */
        bool modifyClip( css::rendering::RenderState&                       o_rRenderState,
                         const struct ::cppcanvas::internal::OutDevState&   rOutdevState,
                         const CanvasSharedPtr&                             rCanvas,
                         const ::basegfx::B2DPoint&                         rOffset,
                         const ::basegfx::B2DVector*                        pScaling,
                         const double*                                      pRotation );

        struct TextLineInfo
        {
            TextLineInfo( const double& rLineHeight,
                          const double& rOverlineHeight,
                          const double& rOverlineOffset,
                          const double& rUnderlineOffset,
                          const double& rStrikeoutOffset,
                          sal_Int8      nOverlineStyle,
                          sal_Int8      nUnderlineStyle,
                          sal_Int8      nStrikeoutStyle ) :
                mnLineHeight( rLineHeight ),
                mnOverlineHeight( rOverlineHeight ),
                mnOverlineOffset( rOverlineOffset ),
                mnUnderlineOffset( rUnderlineOffset ),
                mnStrikeoutOffset( rStrikeoutOffset ),
                mnOverlineStyle( nOverlineStyle ),
                mnUnderlineStyle( nUnderlineStyle ),
                mnStrikeoutStyle( nStrikeoutStyle )
            {
            }

            double      mnLineHeight;
            double      mnOverlineHeight;
            double      mnOverlineOffset;
            double      mnUnderlineOffset;
            double      mnStrikeoutOffset;
            sal_Int8    mnOverlineStyle;
            sal_Int8    mnUnderlineStyle;
            sal_Int8    mnStrikeoutStyle;
        };

        /** Transform given bounds to device coordinate system.
         */
        ::basegfx::B2DRange calcDevicePixelBounds( const ::basegfx::B2DRange&          rBounds,
                                                   const css::rendering::ViewState&    viewState,
                                                   const css::rendering::RenderState&  renderState );

        /** Generate text underline/strikeout info struct from OutDev
            state.
         */
        TextLineInfo createTextLineInfo( const ::VirtualDevice&                     rVDev,
                                         const ::cppcanvas::internal::OutDevState&  rState );

        /** Create a poly-polygon representing the given combination
            of overline, strikeout and underline.

            @param rStartOffset
            Offset in X direction, where the underline starts

            @param rLineWidth
            Width of the line of text to overline/strikeout/underline

            @param rTextLineInfo
            Common info needed for overline/strikeout/underline generation
         */
        ::basegfx::B2DPolyPolygon createTextLinesPolyPolygon( const double&         rStartOffset,
                                                              const double&         rLineWidth,
                                                              const TextLineInfo&   rTextLineInfo );

        ::basegfx::B2DPolyPolygon createTextLinesPolyPolygon( const ::basegfx::B2DPoint& rStartPos,
                                                              const double&              rLineWidth,
                                                              const TextLineInfo&        rTextLineInfo );
    }
}

#endif // INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_MTFTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
