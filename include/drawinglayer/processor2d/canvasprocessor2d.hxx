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

#ifndef _DRAWINGLAYER_PROCESSOR_CANVASPROCESSOR2D_HXX
#define _DRAWINGLAYER_PROCESSOR_CANVASPROCESSOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <com/sun/star/rendering/ViewState.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////
// forward declaration

namespace basegfx {
    class BColor;
    class B2DPolygon;
}

namespace com { namespace sun { namespace star { namespace rendering {
    class XCanvas;
    class XPolyPolygon2D;
}}}}

namespace drawinglayer { namespace primitive2d {
    class TextSimplePortionPrimitive2D;
    class PolygonHairlinePrimitive2D;
    class BitmapPrimitive2D;
    class FillGraphicPrimitive2D;
    class PolyPolygonGradientPrimitive2D;
    class PolyPolygonGraphicPrimitive2D;
    class PolyPolygonColorPrimitive2D;
    class BackgroundColorPrimitive2D;
    class MaskPrimitive2D;
    class UnifiedTransparencePrimitive2D;
    class TransparencePrimitive2D;
    class TransformPrimitive2D;
    class PointArrayPrimitive2D;
    class ModifiedColorPrimitive2D;
    class PolygonStrokePrimitive2D;
    class PagePreviewPrimitive2D;
    class SvgLinearGradientPrimitive2D;
    class SvgRadialGradientPrimitive2D;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /** CanvasProcessor2D class

            An implementation of a renderer for com::sun::star::rendering::XCanvas
            as a target
         */
        class DRAWINGLAYER_DLLPUBLIC CanvasProcessor2D : public BaseProcessor2D
        {
        private:
            // the (current) destination canvas
            com::sun::star::uno::Reference<
                com::sun::star::rendering::XCanvas >        mxCanvas;
            com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory> mxParametricPolygonFactory;
            com::sun::star::rendering::ViewState            maViewState;

            // entirely volatile - but since we need it for ~every canvas call ...
            com::sun::star::rendering::RenderState          maVolatileRenderState;

            // the modifiedColorPrimitive stack
            basegfx::BColorModifierStack                    maBColorModifierStack;

            // SvtOptionsDrawinglayer incarnation to react on diverse settings
            const SvtOptionsDrawinglayer                    maDrawinglayerOpt;

            // the current clipping PolyPolygon from MaskPrimitive2D, always in
            // object coordinates
            basegfx::B2DPolyPolygon                         maClipPolyPolygon;

            // as tooling, the process() implementation takes over API handling and calls this
            // virtual render method when the primitive implementation is BasePrimitive2D-based.
            virtual void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);

            // direct primitive renderer support
            void impRenderTextSimplePortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate);
            void impRenderPolygonHairlinePrimitive2D(const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate);
            void impRenderBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate);
            void impRenderFillGraphicPrimitive2D(const primitive2d::FillGraphicPrimitive2D& rFillBitmapCandidate);
            void impRenderPolyPolygonGraphicPrimitive2D(const primitive2d::PolyPolygonGraphicPrimitive2D& rPolygonCandidate);
            void impRenderPolyPolygonColorPrimitive2D(const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate);
            void impRenderBackgroundColorPrimitive2D(const primitive2d::BackgroundColorPrimitive2D& rCandidate);
            void impRenderSvgLinearGradientPrimitive2D(const primitive2d::SvgLinearGradientPrimitive2D& rCandidate);
            void impRenderSvgRadialGradientPrimitive2D(const primitive2d::SvgRadialGradientPrimitive2D& rCandidate);
            void impRenderMaskPrimitive2D(const primitive2d::MaskPrimitive2D& rMaskCandidate);
            void impRenderModifiedColorPrimitive2D(const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate);
            void impRenderUnifiedTransparencePrimitive2D(const primitive2d::UnifiedTransparencePrimitive2D& rTransCandidate);
            void impRenderTransparencePrimitive2D(const primitive2d::TransparencePrimitive2D& rTransCandidate);
            void impRenderTransformPrimitive2D(const primitive2d::TransformPrimitive2D& rTransformCandidate);
            void impRenderPagePreviewPrimitive2D(const primitive2d::PagePreviewPrimitive2D& rPagePreviewCandidate);
            void impRenderPointArrayPrimitive2D(const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate);
            void impRenderPolygonStrokePrimitive2D(const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokeCandidate);

        public:
            CanvasProcessor2D(
                const geometry::ViewInformation2D& rViewInformation,
                const com::sun::star::uno::Reference<
                    com::sun::star::rendering::XCanvas >& xCanvas);

            // access to Drawinglayer configuration options
            const SvtOptionsDrawinglayer& getOptionsDrawinglayer() const { return maDrawinglayerOpt; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PROCESSOR_CANVASPROCESSOR2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
