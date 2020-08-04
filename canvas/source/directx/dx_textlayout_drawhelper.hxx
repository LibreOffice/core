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

#pragma once

#include <memory>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/rendering/StringContext.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2isize.hxx>

namespace com::sun::star::rendering { class XCanvasFont; }

namespace Gdiplus { class Graphics; }

namespace dxcanvas
{
    struct Bitmap;
    class TextLayoutDrawHelper
    {
    public:
        explicit TextLayoutDrawHelper(
            const css::uno::Reference< css::rendering::XGraphicDevice >& xGraphicDevice);
        ~TextLayoutDrawHelper();

        // draw text
        void drawText( const std::shared_ptr<Gdiplus::Graphics>&          rGraphics,
                       const css::rendering::ViewState&        rViewState,
                       const css::rendering::RenderState&      rRenderState,
                       const ::basegfx::B2ISize&               rOutputOffset,
                       const css::rendering::StringContext&    rText,
                       const css::uno::Sequence< double >&     rLogicalAdvancements,
                       const css::uno::Reference<
                               css::rendering::XCanvasFont >&  rCanvasFont,
                       const css::geometry::Matrix2D&          rFontMatrix,
                       bool                                                 bAlphaSurface,
                       bool bIsRTL);

        css::geometry::RealRectangle2D queryTextBounds(
            const css::rendering::StringContext&   rText,
            const css::uno::Sequence< double >&    rLogicalAdvancements,
            const css::uno::Reference<
                css::rendering::XCanvasFont >&     rCanvasFont,
              const css::geometry::Matrix2D&       rFontMatrix );

#ifdef DBG_UTIL
        void test();
#endif

    protected:
        css::uno::Reference< css::rendering::XGraphicDevice > mxGraphicDevice;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
