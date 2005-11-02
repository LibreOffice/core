/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mtftools.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:41:15 $
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

#ifndef _CPPCANVAS_RENDERER_MTFTOOLS_HXX
#define _CPPCANVAS_RENDERER_MTFTOOLS_HXX

#include <action.hxx>
#include <cppcanvas/canvas.hxx>


class VirtualDevice;
class Point;
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
        void initRenderState( ::com::sun::star::rendering::RenderState& renderState,
                              const ::cppcanvas::internal::OutDevState&         outdevState );

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
        bool modifyClip( ::com::sun::star::rendering::RenderState&          o_rRenderState,
                         const struct ::cppcanvas::internal::OutDevState&   rOutdevState,
                         const CanvasSharedPtr&                             rCanvas,
                         const ::Point&                                     rOffset,
                         const ::basegfx::B2DVector*                        pScaling,
                         const double*                                      pRotation );

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
        bool modifyClip( ::com::sun::star::rendering::RenderState&          o_rRenderState,
                         const struct ::cppcanvas::internal::OutDevState&   rOutdevState,
                         const CanvasSharedPtr&                             rCanvas,
                         const ::basegfx::B2DPoint&                         rOffset,
                         const ::basegfx::B2DVector*                        pScaling,
                         const double*                                      pRotation );

        /** This method modifies the clip, to cancel the given
            transformation.

            As the clip is relative to the render state
            transformation, transforming the render state further must
            modify the clip, to keep it at the same position relative
            to the primitive at hand

            @param o_rRenderState
            Render state to change the clip in

            @param rOutdevState
            Input state. Is used to retrieve the original clip from

            @param rTransform
            The clip is transformed by the inverse of this value.

            @return true, if the clip has changed, false if not
         */
        bool modifyClip( ::com::sun::star::rendering::RenderState&          o_rRenderState,
                         const struct ::cppcanvas::internal::OutDevState&   rOutdevState,
                         const CanvasSharedPtr&                             rCanvas,
                         const ::basegfx::B2DHomMatrix&                     rTransform );

        struct TextLineInfo
        {
            TextLineInfo( const double& rLineHeight,
                          const double& rUnderlineOffset,
                          const double& rStrikeoutOffset,
                          sal_Int8      nUnderlineStyle,
                          sal_Int8      nStrikeoutStyle ) :
                mnLineHeight( rLineHeight ),
                mnUnderlineOffset( rUnderlineOffset ),
                mnStrikeoutOffset( rStrikeoutOffset ),
                mnUnderlineStyle( nUnderlineStyle ),
                mnStrikeoutStyle( nStrikeoutStyle )
            {
            }

            double      mnLineHeight;
            double      mnUnderlineOffset;
            double      mnStrikeoutOffset;
            sal_Int8    mnUnderlineStyle;
            sal_Int8    mnStrikeoutStyle;
        };

        /** Transform given bounds to device coordinate system.
         */
        ::basegfx::B2DRange calcDevicePixelBounds( const ::basegfx::B2DRange&                       rBounds,
                                                   const ::com::sun::star::rendering::ViewState&    viewState,
                                                   const ::com::sun::star::rendering::RenderState&  renderState );

        /** Generate text underline/strikeout info struct from OutDev
            state.
         */
        TextLineInfo createTextLineInfo( const ::VirtualDevice&                     rVDev,
                                         const ::cppcanvas::internal::OutDevState&  rState );

        /** Create a poly-polygon representing the given combination
            of strikeout and underline.

            @param rStartOffset
            Offset in X direction, where the underline starts

            @param rLineWidth
            Width of the line of text to underline/strikeout

            @param rTextLineInfo
            Common info needed for strikeout/underline generation
         */
        ::basegfx::B2DPolyPolygon createTextLinesPolyPolygon( const double&         rStartOffset,
                                                              const double&         rLineWidth,
                                                              const TextLineInfo&   rTextLineInfo );

        ::basegfx::B2DPolyPolygon createTextLinesPolyPolygon( const ::basegfx::B2DPoint rStartPos,
                                                              const double&             rLineWidth,
                                                              const TextLineInfo&       rTextLineInfo );
    }
}

#endif /* _CPPCANVAS_RENDERER_MTFTOOLS_HXX */
