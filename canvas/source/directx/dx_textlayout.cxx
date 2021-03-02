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

#include <sal/config.h>
#include <sal/log.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "dx_bitmap.hxx"
#include "dx_spritecanvas.hxx"
#include "dx_textlayout.hxx"
#include "dx_textlayout_drawhelper.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    TextLayout::TextLayout( const rendering::StringContext&     aText,
                            sal_Int8                            nDirection,
                            sal_Int64                           /*nRandomSeed*/,
                            const CanvasFont::ImplRef&          rFont ) :
        TextLayout_Base( m_aMutex ),
        maText( aText ),
        maLogicalAdvancements(),
        mpFont( rFont ),
        mnTextDirection( nDirection )
    {
    }

    TextLayout::~TextLayout()
    {
    }

    void SAL_CALL TextLayout::disposing()
    {
        mpFont.clear();
    }

    // XTextLayout
    uno::Sequence< uno::Reference< rendering::XPolyPolygon2D > > SAL_CALL TextLayout::queryTextShapes(  )
    {
        // TODO
        return uno::Sequence< uno::Reference< rendering::XPolyPolygon2D > >();
    }

    uno::Sequence< geometry::RealRectangle2D > SAL_CALL TextLayout::queryInkMeasures(  )
    {
        // TODO
        return uno::Sequence< geometry::RealRectangle2D >();
    }

    uno::Sequence< geometry::RealRectangle2D > SAL_CALL TextLayout::queryMeasures(  )
    {
        // TODO
        return uno::Sequence< geometry::RealRectangle2D >();
    }

    uno::Sequence< double > SAL_CALL TextLayout::queryLogicalAdvancements(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maLogicalAdvancements;
    }

    void SAL_CALL TextLayout::applyLogicalAdvancements( const uno::Sequence< double >& aAdvancements )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if( aAdvancements.getLength() != maText.Length )
        {
            SAL_WARN("canvas.directx", "TextLayout::applyLogicalAdvancements(): mismatching number of advancements" );
            throw lang::IllegalArgumentException();
        }

        maLogicalAdvancements = aAdvancements;
    }

    geometry::RealRectangle2D SAL_CALL TextLayout::queryTextBounds( )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        uno::Reference< rendering::XGraphicDevice > xGraphicDevice;
        ::dxcanvas::TextLayoutDrawHelper aDrawHelper(xGraphicDevice);

        // render text
        const geometry::RealRectangle2D aBounds(
            aDrawHelper.queryTextBounds(
                maText,
                maLogicalAdvancements,
                mpFont,
                mpFont->getFontMatrix()));

        return aBounds;
    }

    double SAL_CALL TextLayout::justify( double /*nSize*/ )
    {
        // TODO
        return 0.0;
    }

    double SAL_CALL TextLayout::combinedJustify( const uno::Sequence< uno::Reference< rendering::XTextLayout > >& /*aNextLayouts*/,
                                                 double /*nSize*/ )
    {
        // TODO
        return 0.0;
    }

    rendering::TextHit SAL_CALL TextLayout::getTextHit( const geometry::RealPoint2D& /*aHitPoint*/ )
    {
        // TODO
        return rendering::TextHit();
    }

    rendering::Caret SAL_CALL TextLayout::getCaret( sal_Int32 /*nInsertionIndex*/,
                                                    sal_Bool /*bExcludeLigatures*/ )
    {
        // TODO
        return rendering::Caret();
    }

    sal_Int32 SAL_CALL TextLayout::getNextInsertionIndex( sal_Int32 /*nStartIndex*/,
                                                          sal_Int32 /*nCaretAdvancement*/,
                                                          sal_Bool /*bExcludeLigatures*/ )
    {
        // TODO
        return 0;
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL TextLayout::queryVisualHighlighting( sal_Int32 /*nStartIndex*/,
                                                                                              sal_Int32 /*nEndIndex*/ )
    {
        // TODO
        return uno::Reference< rendering::XPolyPolygon2D >();
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL TextLayout::queryLogicalHighlighting( sal_Int32 /*nStartIndex*/,
                                                                                               sal_Int32 /*nEndIndex*/ )
    {
        // TODO
        return uno::Reference< rendering::XPolyPolygon2D >();
    }

    double SAL_CALL TextLayout::getBaselineOffset(  )
    {
        // TODO
        return 0.0;
    }

    sal_Int8 SAL_CALL TextLayout::getMainTextDirection(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return mnTextDirection;
    }

    uno::Reference< rendering::XCanvasFont > SAL_CALL TextLayout::getFont(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return mpFont;
    }

    rendering::StringContext SAL_CALL TextLayout::getText(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maText;
    }

    bool TextLayout::draw( const GraphicsSharedPtr&                           rGraphics,
                           const rendering::ViewState&                        rViewState,
                           const rendering::RenderState&                      rRenderState,
                           const ::basegfx::B2ISize&                          rOutputOffset,
                           const uno::Reference< rendering::XGraphicDevice >& xGraphicDevice,
                           bool                                               bAlphaSurface ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ::dxcanvas::TextLayoutDrawHelper aDrawHelper(xGraphicDevice);

        // render text
        aDrawHelper.drawText(
            rGraphics,
            rViewState,
            rRenderState,
            rOutputOffset,
            maText,
            maLogicalAdvancements,
            mpFont,
            mpFont->getFontMatrix(),
            bAlphaSurface,
            mnTextDirection != 0);

        return true;
    }

    OUString SAL_CALL TextLayout::getImplementationName()
    {
        return "DXCanvas::TextLayout";
    }

    sal_Bool SAL_CALL TextLayout::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL TextLayout::getSupportedServiceNames()
    {
        return { "com.sun.star.rendering.TextLayout" };
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
