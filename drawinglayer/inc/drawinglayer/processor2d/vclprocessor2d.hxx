/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclprocessor2d.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:30:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPROCESSOR2D_HXX

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <svtools/optionsdrawinglayer.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines
class OutputDevice;

namespace drawinglayer { namespace primitive2d {
    class TextSimplePortionPrimitive2D;
    class PolygonHairlinePrimitive2D;
    class BitmapPrimitive2D;
    class FillBitmapPrimitive2D;
    class PolyPolygonGradientPrimitive2D;
    class PolyPolygonBitmapPrimitive2D;
    class PolyPolygonColorPrimitive2D;
    class MetafilePrimitive2D;
    class MaskPrimitive2D;
    class UnifiedAlphaPrimitive2D;
    class AlphaPrimitive2D;
    class TransformPrimitive2D;
    class MarkerArrayPrimitive2D;
    class PointArrayPrimitive2D;
    class ModifiedColorPrimitive2D;
    class PolygonStrokePrimitive2D;
    class ControlPrimitive2D;
    class PagePreviewPrimitive2D;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        class VclProcessor2D : public BaseProcessor2D
        {
        protected:
            // the destination OutDev
            OutputDevice*                                           mpOutputDevice;

            // the modifiedColorPrimitive stack
            basegfx::BColorModifierStack                            maBColorModifierStack;

            // the current transformation. Since VCL pixel renderer transforms to pixels
            // and VCL MetaFile renderer to World (logic) coordinates, the local
            // ViewInformation2D cannot directly be used, but needs to be kept up to date
            basegfx::B2DHomMatrix                                   maCurrentTransformation;

            // SvtOptionsDrawinglayer incarnation to react on diverse settings
            const SvtOptionsDrawinglayer                            maDrawinglayerOpt;

            // stack value (increment and decrement) to count how deep we are in
            // PolygonStrokePrimitive2D's decompositions (normally only one)
            sal_uInt32                                              mnPolygonStrokePrimitive2D;

            //////////////////////////////////////////////////////////////////////////////
            // common VCL rendering support

            void RenderTextSimpleOrDecoratedPortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate);
            void RenderPolygonHairlinePrimitive2D(const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate, bool bPixelBased);
            void RenderBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate);
            void RenderFillBitmapPrimitive2D(const primitive2d::FillBitmapPrimitive2D& rFillBitmapCandidate);
            void RenderPolyPolygonGradientPrimitive2D(const primitive2d::PolyPolygonGradientPrimitive2D& rPolygonCandidate);
            void RenderPolyPolygonBitmapPrimitive2D(const primitive2d::PolyPolygonBitmapPrimitive2D& rPolygonCandidate);
            void RenderPolyPolygonColorPrimitive2D(const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate);
            void RenderMetafilePrimitive2D(const primitive2d::MetafilePrimitive2D& rPolygonCandidate);
            void RenderMaskPrimitive2DPixel(const primitive2d::MaskPrimitive2D& rMaskCandidate);
            void RenderModifiedColorPrimitive2D(const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate);
            void RenderUnifiedAlphaPrimitive2D(const primitive2d::UnifiedAlphaPrimitive2D& rTransCandidate);
            void RenderAlphaPrimitive2D(const primitive2d::AlphaPrimitive2D& rTransCandidate);
            void RenderTransformPrimitive2D(const primitive2d::TransformPrimitive2D& rTransformCandidate);
            void RenderPagePreviewPrimitive2D(const primitive2d::PagePreviewPrimitive2D& rPagePreviewCandidate);
            void RenderMarkerArrayPrimitive2D(const primitive2d::MarkerArrayPrimitive2D& rMarkerArrayCandidate);
            void RenderPointArrayPrimitive2D(const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate);
            void RenderPolygonStrokePrimitive2D(const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokeCandidate);

            /////////////////////////////////////////////////////////////////////////////
            // DrawMode adaption support
            void adaptLineToFillDrawMode() const;
            void adaptTextToFillDrawMode() const;

        public:
            // constructor/destructor
            VclProcessor2D(
                const geometry::ViewInformation2D& rViewInformation,
                OutputDevice& rOutDev);
            virtual ~VclProcessor2D();

            // access to Drawinglayer configuration options
            const SvtOptionsDrawinglayer& getOptionsDrawinglayer() const { return maDrawinglayerOpt; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPROCESSOR2D_HXX

// eof
