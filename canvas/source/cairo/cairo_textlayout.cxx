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

#include <math.h>
#include <memory>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/metric.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>


#include "cairo_spritecanvas.hxx"
#include "cairo_textlayout.hxx"

#if defined CAIRO_HAS_FT_FONT
# include <cairo-ft.h>
#else
# error Native API needed.
#endif

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    namespace
    {
        void setupLayoutMode( OutputDevice& rOutDev,
                              sal_Int8      nTextDirection )
        {
            // TODO(P3): avoid if already correctly set
            ComplexTextLayoutFlags nLayoutMode = ComplexTextLayoutFlags::Default;
            switch( nTextDirection )
            {
                case rendering::TextDirection::WEAK_LEFT_TO_RIGHT:
                    break;
                case rendering::TextDirection::STRONG_LEFT_TO_RIGHT:
                    nLayoutMode = ComplexTextLayoutFlags::BiDiStrong;
                    break;
                case rendering::TextDirection::WEAK_RIGHT_TO_LEFT:
                    nLayoutMode = ComplexTextLayoutFlags::BiDiRtl;
                    break;
                case rendering::TextDirection::STRONG_RIGHT_TO_LEFT:
                    nLayoutMode = ComplexTextLayoutFlags::BiDiRtl | ComplexTextLayoutFlags::BiDiStrong;
                    break;
                default:
                    break;
            }

            // set calculated layout mode. Origin is always the left edge,
            // as required at the API spec
            rOutDev.SetLayoutMode( nLayoutMode | ComplexTextLayoutFlags::TextOriginLeft );
        }

        bool compareFallbacks(const SystemGlyphData&rA, const SystemGlyphData &rB)
        {
            return rA.fallbacklevel < rB.fallbacklevel;
        }
    }

    TextLayout::TextLayout( const rendering::StringContext&     aText,
                            sal_Int8                            nDirection,
                            sal_Int64                           /*nRandomSeed*/,
                            const CanvasFont::Reference&        rFont,
                            const SurfaceProviderRef&           rRefDevice ) :
        TextLayout_Base( m_aMutex ),
        maText( aText ),
        maLogicalAdvancements(),
        mpFont( rFont ),
        mpRefDevice( rRefDevice ),
        mnTextDirection( nDirection )
    {
    }

    TextLayout::~TextLayout()
    {
    }

    void SAL_CALL TextLayout::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpFont.clear();
        mpRefDevice.clear();
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
            SAL_WARN("canvas.cairo", "TextLayout::applyLogicalAdvancements(): mismatching number of advancements" );
            throw lang::IllegalArgumentException();
        }

        maLogicalAdvancements = aAdvancements;
    }

    geometry::RealRectangle2D SAL_CALL TextLayout::queryTextBounds(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        OutputDevice* pOutDev = mpRefDevice->getOutputDevice();
        if( !pOutDev )
            return geometry::RealRectangle2D();

        ScopedVclPtrInstance< VirtualDevice > pVDev( *pOutDev );
        pVDev->SetFont( mpFont->getVCLFont() );

        // need metrics for Y offset, the XCanvas always renders
        // relative to baseline
        const ::FontMetric& aMetric( pVDev->GetFontMetric() );

        setupLayoutMode( *pVDev, mnTextDirection );

        const sal_Int32 nAboveBaseline( -aMetric.GetAscent() );
        const sal_Int32 nBelowBaseline( aMetric.GetDescent() );

        if( maLogicalAdvancements.getLength() )
        {
            return geometry::RealRectangle2D( 0, nAboveBaseline,
                                              maLogicalAdvancements[ maLogicalAdvancements.getLength()-1 ],
                                              nBelowBaseline );
        }
        else
        {
            return geometry::RealRectangle2D( 0, nAboveBaseline,
                                              pVDev->GetTextWidth(
                                                  maText.Text,
                                                  ::canvas::tools::numeric_cast<sal_uInt16>(maText.StartPosition),
                                                  ::canvas::tools::numeric_cast<sal_uInt16>(maText.Length) ),
                                              nBelowBaseline );
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

        return mpFont.get();
    }

    rendering::StringContext SAL_CALL TextLayout::getText(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maText;
    }

  /**
   * TextLayout::isCairoRenderable
   *
   * Features currently not supported by Cairo (VCL rendering is used as fallback):
   * - vertical glyphs
   *
   * @return true, if text/font can be rendered with cairo
   **/
    bool TextLayout::isCairoRenderable(SystemFontData aSysFontData) const
    {
#if defined CAIRO_HAS_FT_FONT
        // is font usable?
        if (!aSysFontData.nFontId)
            return false;
#endif

        // vertical glyph rendering is not supported in cairo for now
        if (aSysFontData.bVerticalCharacterType)
        {
            SAL_WARN("canvas.cairo", ":cairocanvas::TextLayout::isCairoRenderable(): Vertical Character Style not supported");
            return false;
        }

        return true;
    }


  /**
   * TextLayout::draw
   *
   * Cairo-based text rendering. Draw text directly on the cairo surface with cairo fonts.
   * Avoid using VCL VirtualDevices for that, bypassing VCL DrawText functions, when possible
   *
   * Note: some text effects are not rendered due to lacking generic canvas or cairo canvas
   *       implementation. See issues 92657, 92658, 92659, 92660, 97529
   **/
    void TextLayout::draw( CairoSharedPtr const &        pSCairo,
                           OutputDevice&                 rOutDev,
                           const Point&                  rOutpos,
                           const rendering::ViewState&   viewState,
                           const rendering::RenderState& renderState ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        SystemTextLayoutData aSysLayoutData;
        setupLayoutMode( rOutDev, mnTextDirection );

        // TODO(P2): cache that
        std::unique_ptr< long []> aOffsets(new long[maLogicalAdvancements.getLength()]);

        if( maLogicalAdvancements.getLength() )
        {
            setupTextOffsets( aOffsets.get(), maLogicalAdvancements, viewState, renderState );

            // TODO(F3): ensure correct length and termination for DX
            // array (last entry _must_ contain the overall width)
        }

        aSysLayoutData = rOutDev.GetSysTextLayoutData(rOutpos, maText.Text,
                                                      ::canvas::tools::numeric_cast<sal_uInt16>(maText.StartPosition),
                                                      ::canvas::tools::numeric_cast<sal_uInt16>(maText.Length),
                                                      maLogicalAdvancements.getLength() ? aOffsets.get() : nullptr);

        // Sort them so that all glyphs on the same glyph fallback level are consecutive
        std::sort(aSysLayoutData.rGlyphData.begin(), aSysLayoutData.rGlyphData.end(), compareFallbacks);
        bool bCairoRenderable = true;

        //Pull all the fonts we need to render the text
        typedef std::pair<SystemFontData,int> FontLevel;
        std::vector<FontLevel> aFontData;
        for (auto const& glyph : aSysLayoutData.rGlyphData)
        {
            if( aFontData.empty() || glyph.fallbacklevel != aFontData.back().second )
            {
                aFontData.emplace_back(rOutDev.GetSysFontData(glyph.fallbacklevel),
                                              glyph.fallbacklevel);
                if( !isCairoRenderable(aFontData.back().first) )
                {
                    bCairoRenderable = false;
                    SAL_INFO("canvas.cairo", ":cairocanvas::TextLayout::draw(S,O,p,v,r): VCL FALLBACK " <<
                             (maLogicalAdvancements.getLength() ? "ADV " : "") <<
                             (aFontData.back().first.bAntialias ? "AA " : "") <<
                             (aFontData.back().first.bFakeBold ? "FB " : "") <<
                             (aFontData.back().first.bFakeItalic ? "FI " : "") <<
                             " - " <<
                             maText.Text.copy( maText.StartPosition, maText.Length));
                    break;
                }
            }
        }

        // The ::GetSysTextLayoutData(), i.e. layouting of text to glyphs can change the font being used.
        // The fallback checks need to be done after final font is known.
        if (!bCairoRenderable)    // VCL FALLBACKS
        {
            if (maLogicalAdvancements.getLength())        // VCL FALLBACK - with glyph advances
            {
                rOutDev.DrawTextArray( rOutpos, maText.Text, aOffsets.get(),
                                       ::canvas::tools::numeric_cast<sal_uInt16>(maText.StartPosition),
                                       ::canvas::tools::numeric_cast<sal_uInt16>(maText.Length) );
                return;
            }
            else                                               // VCL FALLBACK - without advances
            {
                rOutDev.DrawText( rOutpos, maText.Text,
                                  ::canvas::tools::numeric_cast<sal_uInt16>(maText.StartPosition),
                                  ::canvas::tools::numeric_cast<sal_uInt16>(maText.Length) );
                return;
            }
        }

        if (aSysLayoutData.rGlyphData.empty())
            return; //??? false?

        /**
         * Setup platform independent glyph vector into cairo-based glyphs vector.
         **/

        // Loop through the fonts used and render the matching glyphs for each
        for (auto const& elemFontData : aFontData)
        {
            const SystemFontData &rSysFontData = elemFontData.first;

            // setup glyphs
            std::vector<cairo_glyph_t> cairo_glyphs;
            cairo_glyphs.reserve( 256 );

            for (auto const& systemGlyph : aSysLayoutData.rGlyphData)
            {
                if( systemGlyph.fallbacklevel != elemFontData.second )
                    continue;

                cairo_glyph_t aGlyph;
                aGlyph.index = systemGlyph.index;
                aGlyph.x = systemGlyph.x;
                aGlyph.y = systemGlyph.y;
                cairo_glyphs.push_back(aGlyph);
            }

            if (cairo_glyphs.empty())
                continue;

            const vcl::Font& aFont = rOutDev.GetFont();
            long nWidth = aFont.GetAverageFontWidth();
            long nHeight = aFont.GetFontHeight();
            if (nWidth == 0)
                nWidth = nHeight;
            if (nWidth == 0 || nHeight == 0)
                continue;

            /**
             * Setup font
             **/
            cairo_font_face_t* font_face = nullptr;

#if defined CAIRO_HAS_FT_FONT
            font_face = cairo_ft_font_face_create_for_ft_face(static_cast<FT_Face>(rSysFontData.nFontId),
                                                              rSysFontData.nFontFlags);
#else
# error Native API needed.
#endif

            cairo_set_font_face( pSCairo.get(), font_face);

            // create default font options. cairo_get_font_options() does not retrieve the surface defaults,
            // only what has been set before with cairo_set_font_options()
            cairo_font_options_t* options = cairo_font_options_create();
            if (rSysFontData.bAntialias)
            {
                // CAIRO_ANTIALIAS_GRAY provides more similar result to VCL Canvas,
                // so we're not using CAIRO_ANTIALIAS_SUBPIXEL
                cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_GRAY);
            }
            cairo_set_font_options( pSCairo.get(), options);

            // Font color
            Color aTextColor = rOutDev.GetTextColor();
            cairo_set_source_rgb(pSCairo.get(),
                                 aTextColor.GetRed()/255.0,
                                 aTextColor.GetGreen()/255.0,
                                 aTextColor.GetBlue()/255.0);

            // Font rotation and scaling
            cairo_matrix_t m;

            cairo_matrix_init_identity(&m);

            if (aSysLayoutData.orientation)
                cairo_matrix_rotate(&m, (3600 - aSysLayoutData.orientation) * M_PI / 1800.0);

            cairo_matrix_scale(&m, nWidth, nHeight);

            //faux italics
            if (rSysFontData.bFakeItalic)
                m.xy = -m.xx * 0x6000 / 0x10000;

            cairo_set_font_matrix(pSCairo.get(), &m);

            SAL_INFO(
                "canvas.cairo",
                "Size:(" << aFont.GetAverageFontWidth() << "," << aFont.GetFontHeight()
                    << "), Pos (" << rOutpos.X() << "," << rOutpos.Y()
                    << "), G("
                    << (!cairo_glyphs.empty() ? cairo_glyphs[0].index : -1)
                    << ","
                    << (cairo_glyphs.size() > 1 ? cairo_glyphs[1].index : -1)
                    << ","
                    << (cairo_glyphs.size() > 2 ? cairo_glyphs[2].index : -1)
                    << ") " << (maLogicalAdvancements.getLength() ? "ADV " : "")
                    << (rSysFontData.bAntialias ? "AA " : "")
                    << (rSysFontData.bFakeBold ? "FB " : "")
                    << (rSysFontData.bFakeItalic ? "FI " : "") << " || Name:"
                    << aFont.GetFamilyName() << " - "
                    << maText.Text.copy(maText.StartPosition, maText.Length));

            cairo_show_glyphs(pSCairo.get(), &cairo_glyphs[0], cairo_glyphs.size());

            //faux bold
            if (rSysFontData.bFakeBold)
            {
                double bold_dx = 0.5 * sqrt( 0.7 * aFont.GetFontHeight() );
                int total_steps = 1 * static_cast<int>(bold_dx + 0.5);

                // loop to draw the text for every half pixel of displacement
                for (int nSteps = 0; nSteps < total_steps; nSteps++)
                {
                    for(cairo_glyph_t & cairo_glyph : cairo_glyphs)
                    {
                        cairo_glyph.x += (bold_dx * nSteps / total_steps) / 4;
                        cairo_glyph.y -= (bold_dx * nSteps / total_steps) / 4;
                    }
                    cairo_show_glyphs(pSCairo.get(), &cairo_glyphs[0], cairo_glyphs.size());
                }
                SAL_INFO("canvas.cairo",":cairocanvas::TextLayout::draw(S,O,p,v,r): FAKEBOLD - dx:" << static_cast<int>(bold_dx));
            }

            cairo_font_face_destroy(font_face);
            cairo_font_options_destroy(options);
        }
    }

    namespace
    {
        class OffsetTransformer
        {
        public:
            explicit OffsetTransformer( const ::basegfx::B2DHomMatrix& rMat ) :
                maMatrix( rMat )
            {
            }

            sal_Int32 operator()( const double& rOffset )
            {
                // This is an optimization of the normal rMat*[x,0]
                // transformation of the advancement vector (in x
                // direction), followed by a length calculation of the
                // resulting vector: advancement' =
                // ||rMat*[x,0]||. Since advancements are vectors, we
                // can ignore translational components, thus if [x,0],
                // it follows that rMat*[x,0]=[x',0] holds. Thus, we
                // just have to calc the transformation of the x
                // component.

                // TODO(F2): Handle non-horizontal advancements!
                return ::basegfx::fround( hypot(maMatrix.get(0,0)*rOffset,
                                                maMatrix.get(1,0)*rOffset) );
            }

        private:
            ::basegfx::B2DHomMatrix maMatrix;
        };
    }

    void TextLayout::setupTextOffsets( long*                       outputOffsets,
                                       const uno::Sequence< double >&   inputOffsets,
                                       const rendering::ViewState&      viewState,
                                       const rendering::RenderState&    renderState     ) const
    {
        ENSURE_OR_THROW( outputOffsets!=nullptr,
                          "TextLayout::setupTextOffsets offsets NULL" );

        ::basegfx::B2DHomMatrix aMatrix;

        ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                     viewState,
                                                     renderState);

        // fill integer offsets
        std::transform( inputOffsets.begin(),
                          inputOffsets.end(),
                          outputOffsets,
                          OffsetTransformer( aMatrix ) );
    }

    OUString SAL_CALL TextLayout::getImplementationName()
    {
        return OUString( "CairoCanvas::TextLayout" );
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
