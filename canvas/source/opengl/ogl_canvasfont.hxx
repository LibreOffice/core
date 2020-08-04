/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/rendering/XCanvasFont.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>

#include <rtl/ref.hxx>


/* Definition of CanvasFont class */

namespace oglcanvas
{
    class SpriteCanvas;

    typedef ::cppu::WeakComponentImplHelper< css::rendering::XCanvasFont > CanvasFontBaseT;

    class CanvasFont : public ::cppu::BaseMutex,
                       public CanvasFontBaseT
    {
    public:
        typedef rtl::Reference<CanvasFont> ImplRef;

        /// make noncopyable
        CanvasFont(const CanvasFont&) = delete;
        const CanvasFont& operator=(const CanvasFont&) = delete;

        CanvasFont( const css::rendering::FontRequest&                                     fontRequest,
                    const css::uno::Sequence< css::beans::PropertyValue >&    extraFontProperties,
                    const css::geometry::Matrix2D&                                         fontMatrix );

        // XCanvasFont
        virtual css::uno::Reference< css::rendering::XTextLayout > SAL_CALL createTextLayout( const css::rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 nRandomSeed ) override;
        virtual css::rendering::FontRequest SAL_CALL getFontRequest(  ) override;
        virtual css::rendering::FontMetrics SAL_CALL getFontMetrics(  ) override;
        virtual css::uno::Sequence< double > SAL_CALL getAvailableSizes(  ) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getExtraFontProperties(  ) override;

        const css::geometry::Matrix2D& getFontMatrix() const { return maFontMatrix; }

        sal_uInt32 getEmphasisMark() const { return mnEmphasisMark; }

    private:
        css::rendering::FontRequest    maFontRequest;
        sal_uInt32                     mnEmphasisMark;
        css::geometry::Matrix2D        maFontMatrix;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
