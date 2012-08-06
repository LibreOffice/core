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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPROCESSOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

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
    class RenderGraphicPrimitive2D;
    class FillBitmapPrimitive2D;
    class PolyPolygonGradientPrimitive2D;
    class PolyPolygonBitmapPrimitive2D;
    class PolyPolygonColorPrimitive2D;
    class MetafilePrimitive2D;
    class MaskPrimitive2D;
    class UnifiedTransparencePrimitive2D;
    class TransparencePrimitive2D;
    class TransformPrimitive2D;
    class MarkerArrayPrimitive2D;
    class PointArrayPrimitive2D;
    class ModifiedColorPrimitive2D;
    class PolygonStrokePrimitive2D;
    class ControlPrimitive2D;
    class PagePreviewPrimitive2D;
    class EpsPrimitive2D;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /** VclProcessor2D class

            This processor is the base class for VCL-Based processors. It has no
            processBasePrimitive2D implementation and thus is not usable directly.
         */
        class DRAWINGLAYER_DLLPUBLIC VclProcessor2D : public BaseProcessor2D
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
            void RenderRenderGraphicPrimitive2D(const primitive2d::RenderGraphicPrimitive2D& rRenderGraphicCandidate);
            void RenderFillBitmapPrimitive2D(const primitive2d::FillBitmapPrimitive2D& rFillBitmapCandidate);
            void RenderPolyPolygonGradientPrimitive2D(const primitive2d::PolyPolygonGradientPrimitive2D& rPolygonCandidate);
            void RenderPolyPolygonBitmapPrimitive2D(const primitive2d::PolyPolygonBitmapPrimitive2D& rPolygonCandidate);
            void RenderPolyPolygonColorPrimitive2D(const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate);
            void RenderMetafilePrimitive2D(const primitive2d::MetafilePrimitive2D& rPolygonCandidate);
            void RenderMaskPrimitive2DPixel(const primitive2d::MaskPrimitive2D& rMaskCandidate);
            void RenderModifiedColorPrimitive2D(const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate);
            void RenderUnifiedTransparencePrimitive2D(const primitive2d::UnifiedTransparencePrimitive2D& rTransCandidate);
            void RenderTransparencePrimitive2D(const primitive2d::TransparencePrimitive2D& rTransCandidate);
            void RenderTransformPrimitive2D(const primitive2d::TransformPrimitive2D& rTransformCandidate);
            void RenderPagePreviewPrimitive2D(const primitive2d::PagePreviewPrimitive2D& rPagePreviewCandidate);
            void RenderMarkerArrayPrimitive2D(const primitive2d::MarkerArrayPrimitive2D& rMarkerArrayCandidate);
            void RenderPointArrayPrimitive2D(const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate);
            void RenderPolygonStrokePrimitive2D(const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokeCandidate);
            void RenderEpsPrimitive2D(const primitive2d::EpsPrimitive2D& rEpsPrimitive2D);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
