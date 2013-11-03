/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SLIDESHOW_CANVAS_HXX
#define INCLUDED_SLIDESHOW_CANVAS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <drawinglayer/geometry/viewinformation2d.hxx>

namespace slideshow
{
    namespace internal
    {
        /// XCanvas and state parameters to render stuff
        struct Canvas
        {
            Canvas();

            /// render target
            css::uno::Reference< css::rendering::XCanvas > mxCanvas;
            /// view setup, clip etc
            css::rendering::ViewState                      maViewState;
            /// drawing layer view setup
            ::drawinglayer::geometry::ViewInformation2D    maViewInfo;

            /// default-state view, for output into raw device state
            css::rendering::ViewState                      maDummyDefaultViewState;
            /// default-state render, for output into raw device state
            css::rendering::RenderState                    maDummyDefaultRenderState;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_CANVAS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
