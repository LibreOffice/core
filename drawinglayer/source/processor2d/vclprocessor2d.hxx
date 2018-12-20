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

#ifndef INCLUDED_DRAWINGLAYER_SOURCE_PROCESSOR2D_VCLPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_SOURCE_PROCESSOR2D_VCLPROCESSOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <vcl/vclptr.hxx>


// predefines
class OutputDevice;

namespace drawinglayer { namespace primitive2d {
    class TextSimplePortionPrimitive2D;
    class PolygonHairlinePrimitive2D;
    class BitmapPrimitive2D;
    class FillGraphicPrimitive2D;
    class PolyPolygonGradientPrimitive2D;
    class PolyPolygonGraphicPrimitive2D;
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
    class ObjectInfoPrimitive2D;
    class SvgLinearAtomPrimitive2D;
    class SvgRadialAtomPrimitive2D;
}}


namespace drawinglayer
{
    namespace processor2d
    {
        /** VclProcessor2D class

            This processor is the base class for VCL-Based processors. It has no
            processBasePrimitive2D implementation and thus is not usable directly.
         */
        class VclProcessor2D : public BaseProcessor2D
        {
        protected:
            // the destination OutDev
            VclPtr<OutputDevice>                                    mpOutputDevice;

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

            // currently used ObjectInfoPrimitive2D
            const primitive2d::ObjectInfoPrimitive2D*               mpObjectInfoPrimitive2D;

            // common VCL rendering support
            void RenderTextSimpleOrDecoratedPortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate);
            void RenderPolygonHairlinePrimitive2D(const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate, bool bPixelBased);
            void RenderBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate);
            void RenderFillGraphicPrimitive2D(const primitive2d::FillGraphicPrimitive2D& rFillBitmapCandidate);
            void RenderPolyPolygonGraphicPrimitive2D(const primitive2d::PolyPolygonGraphicPrimitive2D& rPolygonCandidate);
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
            void RenderObjectInfoPrimitive2D(const primitive2d::ObjectInfoPrimitive2D& rObjectInfoPrimitive2D);
            void RenderSvgLinearAtomPrimitive2D(const primitive2d::SvgLinearAtomPrimitive2D& rCandidate);
            void RenderSvgRadialAtomPrimitive2D(const primitive2d::SvgRadialAtomPrimitive2D& rCandidate);

            // DrawMode adaption support
            void adaptLineToFillDrawMode() const;
            void adaptTextToFillDrawMode() const;

        public:
            // constructor/destructor
            VclProcessor2D(
                const geometry::ViewInformation2D& rViewInformation,
                OutputDevice& rOutDev);
            virtual ~VclProcessor2D() override;

            // access to Drawinglayer configuration options
            const SvtOptionsDrawinglayer& getOptionsDrawinglayer() const { return maDrawinglayerOpt; }

            // access to currently used ObjectInfoPrimitive2D
            const primitive2d::ObjectInfoPrimitive2D* getObjectInfoPrimitive2D() const { return mpObjectInfoPrimitive2D; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer


#endif // INCLUDED_DRAWINGLAYER_SOURCE_PROCESSOR2D_VCLPROCESSOR2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
