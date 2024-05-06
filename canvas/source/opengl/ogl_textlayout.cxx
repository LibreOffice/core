/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/log.hxx>
#include <utility>

#include <comphelper/diagnose_ex.hxx>

#include "ogl_textlayout.hxx"

using namespace ::com::sun::star;

namespace oglcanvas
{
    TextLayout::TextLayout( rendering::StringContext      aText,
                            sal_Int8                      nDirection,
                            sal_Int64                     /*nRandomSeed*/,
                            CanvasFont::ImplRef           rFont ) :
        maText(std::move( aText )),
        mpFont(std::move( rFont )),
        mnTextDirection( nDirection )
    {
    }

    void TextLayout::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
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
        std::unique_lock aGuard( m_aMutex );

        return maLogicalAdvancements;
    }

    void SAL_CALL TextLayout::applyLogicalAdvancements( const uno::Sequence< double >& aAdvancements )
    {
        std::unique_lock aGuard( m_aMutex );

        if( aAdvancements.getLength() != maText.Length )
        {
            SAL_INFO("canvas.ogl", "TextLayout::applyLogicalAdvancements(): mismatching number of advancements");
            throw lang::IllegalArgumentException();
        }

        maLogicalAdvancements = aAdvancements;
    }

    uno::Sequence< sal_Bool > SAL_CALL TextLayout::queryKashidaPositions(  )
    {
        std::unique_lock aGuard( m_aMutex );

        return maKashidaPositions;
    }

    void SAL_CALL TextLayout::applyKashidaPositions( const uno::Sequence< sal_Bool >& aPositions )
    {
        std::unique_lock aGuard( m_aMutex );

        if( aPositions.hasElements() && aPositions.getLength() != maText.Length )
        {
            SAL_WARN("canvas.ogl", "TextLayout::applyKashidaPositions(): mismatching number of positions" );
            throw lang::IllegalArgumentException(u"mismatching number of positions"_ustr, getXWeak(), 1);
        }

        maKashidaPositions = aPositions;
    }

    geometry::RealRectangle2D SAL_CALL TextLayout::queryTextBounds(  )
    {
        std::unique_lock aGuard( m_aMutex );

        ENSURE_OR_THROW( mpFont,
                         "TextLayout::queryTextBounds(): invalid font" );

        // fake text bounds by either taking the advancement values,
        // or assuming square glyph boxes (width similar to height)
        const rendering::FontRequest& rFontRequest( mpFont->getFontRequest() );
        const double nFontSize( std::max( rFontRequest.CellSize,
                                            rFontRequest.ReferenceAdvancement ) );
        if( maLogicalAdvancements.hasElements() )
        {
            return geometry::RealRectangle2D( 0, -nFontSize/2,
                                              maLogicalAdvancements[ maLogicalAdvancements.getLength()-1 ],
                                              nFontSize/2 );
        }
        else
        {
            return geometry::RealRectangle2D( 0, -nFontSize/2,
                                              nFontSize * maText.Length,
                                              nFontSize/2 );
        }
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
                                                    sal_Bool  /*bExcludeLigatures*/ )
    {
        // TODO
        return rendering::Caret();
    }

    sal_Int32 SAL_CALL TextLayout::getNextInsertionIndex( sal_Int32 /*nStartIndex*/,
                                                          sal_Int32 /*nCaretAdvancement*/,
                                                          sal_Bool  /*bExcludeLigatures*/ )
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
        return mnTextDirection;
    }

    uno::Reference< rendering::XCanvasFont > SAL_CALL TextLayout::getFont(  )
    {
        std::unique_lock aGuard( m_aMutex );

        return mpFont;
    }

    rendering::StringContext SAL_CALL TextLayout::getText(  )
    {
        return maText;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
