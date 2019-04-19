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

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/rendering/XTextLayout.hpp>

#include <basegfx/vector/b2isize.hxx>

#include "dx_canvasfont.hxx"
#include "dx_ibitmap.hxx"
#include "dx_winstuff.hxx"
#include "dx_gdiplususer.hxx"


/* Definition of TextLayout class */
namespace dxcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XTextLayout,
                                             css::lang::XServiceInfo > TextLayout_Base;

    class TextLayout : public ::cppu::BaseMutex,
                       public TextLayout_Base
    {
    public:
        TextLayout( const css::rendering::StringContext& aText,
                    sal_Int8                                                  nDirection,
                    sal_Int64                                                 nRandomSeed,
                    const CanvasFont::ImplRef&                                rFont );
        /// make noncopyable
        TextLayout(const TextLayout&) = delete;
        const TextLayout& operator=(const TextLayout&) = delete;

        /// Dispose all internal references
        virtual void SAL_CALL disposing() override;

        // XTextLayout
        virtual css::uno::Sequence< css::uno::Reference< css::rendering::XPolyPolygon2D > > SAL_CALL queryTextShapes(  ) override;
        virtual css::uno::Sequence< css::geometry::RealRectangle2D > SAL_CALL queryInkMeasures(  ) override;
        virtual css::uno::Sequence< css::geometry::RealRectangle2D > SAL_CALL queryMeasures(  ) override;
        virtual css::uno::Sequence< double > SAL_CALL queryLogicalAdvancements(  ) override;
        virtual void SAL_CALL applyLogicalAdvancements( const css::uno::Sequence< double >& aAdvancements ) override;
        virtual css::geometry::RealRectangle2D SAL_CALL queryTextBounds(  ) override;
        virtual double SAL_CALL justify( double nSize ) override;
        virtual double SAL_CALL combinedJustify( const css::uno::Sequence< css::uno::Reference< css::rendering::XTextLayout > >& aNextLayouts, double nSize ) override;
        virtual css::rendering::TextHit SAL_CALL getTextHit( const css::geometry::RealPoint2D& aHitPoint ) override;
        virtual css::rendering::Caret SAL_CALL getCaret( sal_Int32 nInsertionIndex, sal_Bool bExcludeLigatures ) override;
        virtual sal_Int32 SAL_CALL getNextInsertionIndex( sal_Int32 nStartIndex, sal_Int32 nCaretAdvancement, sal_Bool bExcludeLigatures ) override;
        virtual css::uno::Reference< css::rendering::XPolyPolygon2D > SAL_CALL queryVisualHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
        virtual css::uno::Reference< css::rendering::XPolyPolygon2D > SAL_CALL queryLogicalHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
        virtual double SAL_CALL getBaselineOffset(  ) override;
        virtual sal_Int8 SAL_CALL getMainTextDirection(  ) override;
        virtual css::uno::Reference< css::rendering::XCanvasFont > SAL_CALL getFont(  ) override;
        virtual css::rendering::StringContext SAL_CALL getText(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        bool draw( const GraphicsSharedPtr&                              rGraphics,
                   const css::rendering::ViewState&         rViewState,
                   const css::rendering::RenderState&       rRenderState,
                   const ::basegfx::B2ISize&                             rOutputOffset,
                   const css::uno::Reference<
                   css::rendering::XGraphicDevice >&        xGraphicDevice,
                   bool                                                  bAlphaSurface ) const;

    protected:
        ~TextLayout() override; // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        // NOTE: no need for GDIPlusUserSharedPtr, mpFont implicitly has one already

        css::rendering::StringContext              maText;
        css::uno::Sequence< double >               maLogicalAdvancements;
        CanvasFont::ImplRef                        mpFont;
        sal_Int8                                   mnTextDirection;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
