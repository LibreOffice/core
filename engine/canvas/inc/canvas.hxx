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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/util/XUpdatable.hpp>

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>
#include <canvas/vclcanvasdllapi.h>
#include <sal/types.h>

#include "canvasfont.hxx"
#include "impltools.hxx"
#include "Texture.hxx"
#include "parametricpolypolygon.hxx"
#include "verifyinput.hxx"
#include "cachedbitmap.hxx"

class OutputDevice;

namespace vclcanvas
{
    /** Product of this component's factory.

        The Canvas object combines the actual Window canvas with
        the XGraphicDevice interface. This is because there's a
        one-to-one relation between them, anyway, since each window
        can have exactly one canvas and one associated
        XGraphicDevice. And to avoid messing around with circular
        references, this is implemented as one single object.
     */
    class SAL_DLLPUBLIC_RTTI Canvas
    {
    public:
        VCLCANVAS_DLLPUBLIC Canvas( OutputDevice& rOutDev );

        /// For resource tracking
        VCLCANVAS_DLLPUBLIC ~Canvas();

        rtl::Reference< ::canvas::ParametricPolyPolygon > createParametricPolyPolygon( std::u16string_view GradientService, const std::vector<::Color>& colors, const ::cpo::uno::Sequence< double >& stops, double aspectRatio );

        void drawPoint(const css::geometry::RealPoint2D&     aPoint,
                                        const ::vclcanvas::ViewState&      viewState,
                                        const ::vclcanvas::RenderState&    renderState)
        {
            canvastools::verifyArgs(aPoint, viewState, renderState,
                              __func__);
        }

        bool repaint( const GraphicObjectSharedPtr&                 rGrf,
                              const ::vclcanvas::ViewState&              viewState,
                              const ::vclcanvas::RenderState&            renderState,
                              const ::Point&                                rPt,
                              const ::Size&                                 rSz,
                              const GraphicAttr&                            rAttr ) const;

        VCLCANVAS_DLLPUBLIC void drawLine(const ::basegfx::B2DPoint&  aStartPoint,
                                       const ::basegfx::B2DPoint&  aEndPoint,
                                       const ::vclcanvas::ViewState&   viewState,
                                       const ::vclcanvas::RenderState& renderState);

        rtl::Reference< vclcanvas::CachedBitmap >
            drawBitmap( const Bitmap&                                                   rBitmap,
                        const ::vclcanvas::ViewState&                                   viewState,
                        const ::vclcanvas::RenderState&                                 renderState );

        rtl::Reference< vclcanvas::CachedBitmap >
            drawBitmapModulated( const Bitmap& rBitmap,
                                 const ::vclcanvas::ViewState&      viewState,
                                 const ::vclcanvas::RenderState&    renderState );

        VCLCANVAS_DLLPUBLIC void
            strokePolyPolygon(const basegfx::B2DPolyPolygon&                              xPolyPolygon,
                              const ::vclcanvas::ViewState&                               viewState,
                              const ::vclcanvas::RenderState&                             renderState,
                              const css::rendering::StrokeAttributes&                        strokeAttributes);

        rtl::Reference< vclcanvas::CachedBitmap >
            fillPolyPolygon(const basegfx::B2DPolyPolygon&                                         xPolyPolygon,
                             const ::vclcanvas::ViewState&                                          viewState,
                             const ::vclcanvas::RenderState&                                        renderState);

        rtl::Reference< vclcanvas::CachedBitmap >
            fillTexturedPolyPolygon(const basegfx::B2DPolyPolygon&                     xPolyPolygon,
                                    const ::vclcanvas::ViewState&                             viewState,
                                    const ::vclcanvas::RenderState&                           renderState,
                                    const std::vector< vclcanvas::Texture >&           textures);

        rtl::Reference< vclcanvas::CanvasFont >
            createFont( const css::rendering::FontRequest&                                     fontRequest,
                        FontEmphasisMark                                                       eMark,
                        const css::geometry::Matrix2D&                                         fontMatrix );

        void
            drawTextLayout(const rtl::Reference< vclcanvas::TextLayout >&               laidOutText,
                            const ::vclcanvas::ViewState&                                       viewState,
                            const ::vclcanvas::RenderState&                                     renderState);

        void
            drawPolyPolygon(const basegfx::B2DPolyPolygon&                            xPolyPolygon,
                            const ::vclcanvas::ViewState&                             viewState,
                            const ::vclcanvas::RenderState&                           renderState);

    private:

        enum ColorType
        {
            LINE_COLOR, FILL_COLOR, TEXT_COLOR, IGNORE_COLOR
        };

         // returns alpha of color
        int setupOutDevState( const ::vclcanvas::ViewState&     viewState,
                              const ::vclcanvas::RenderState&   renderState,
                              ColorType                            eColorType ) const;

        rtl::Reference< vclcanvas::CachedBitmap >
            implDrawBitmap( const Bitmap&     rBitmap,
                            const ::vclcanvas::ViewState&   viewState,
                            const ::vclcanvas::RenderState& renderState,
                            bool                                            bModulateColors );

        bool setupTextOutput( ::Point&                                                                              o_rOutPos,
                              const ::vclcanvas::ViewState&                                         viewState,
                              const ::vclcanvas::RenderState&                                       renderState,
                              const rtl::Reference< vclcanvas::CanvasFont >&   xFont ) const;

        /// For retrieving device info
        VclPtr<OutputDevice> mxOutDev;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
