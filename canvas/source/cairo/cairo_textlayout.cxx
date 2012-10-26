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

#include <math.h>

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <tools/diagnose_ex.h>

#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>

#ifdef WNT
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif
#include <vcl/sysdata.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <boost/scoped_array.hpp>

#include "cairo_textlayout.hxx"
#include "cairo_spritecanvas.hxx"

#ifdef CAIRO_HAS_QUARTZ_SURFACE
# include "cairo_quartz_cairo.hxx"
#elif defined CAIRO_HAS_WIN32_SURFACE
# include "cairo_win32_cairo.hxx"
# include <cairo-win32.h>
#elif defined CAIRO_HAS_XLIB_SURFACE
# include "cairo_xlib_cairo.hxx"
# include <cairo-ft.h>
#else
# error Native API needed.
#endif

#ifdef IOS
#include <CoreText/CoreText.h>
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
            sal_uLong nLayoutMode;
            switch( nTextDirection )
            {
                default:
                    nLayoutMode = 0;
                    break;
                case rendering::TextDirection::WEAK_LEFT_TO_RIGHT:
                    nLayoutMode = TEXT_LAYOUT_BIDI_LTR;
                    break;
                case rendering::TextDirection::STRONG_LEFT_TO_RIGHT:
                    nLayoutMode = TEXT_LAYOUT_BIDI_LTR | TEXT_LAYOUT_BIDI_STRONG;
                    break;
                case rendering::TextDirection::WEAK_RIGHT_TO_LEFT:
                    nLayoutMode = TEXT_LAYOUT_BIDI_RTL;
                    break;
                case rendering::TextDirection::STRONG_RIGHT_TO_LEFT:
                    nLayoutMode = TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_BIDI_STRONG;
                    break;
            }

            // set calculated layout mode. Origin is always the left edge,
            // as required at the API spec
            rOutDev.SetLayoutMode( nLayoutMode | TEXT_LAYOUT_TEXTORIGIN_LEFT );
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

        mpFont.reset();
        mpRefDevice.clear();
    }

    // XTextLayout
    uno::Sequence< uno::Reference< rendering::XPolyPolygon2D > > SAL_CALL TextLayout::queryTextShapes(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return uno::Sequence< uno::Reference< rendering::XPolyPolygon2D > >();
    }

    uno::Sequence< geometry::RealRectangle2D > SAL_CALL TextLayout::queryInkMeasures(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return uno::Sequence< geometry::RealRectangle2D >();
    }

    uno::Sequence< geometry::RealRectangle2D > SAL_CALL TextLayout::queryMeasures(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return uno::Sequence< geometry::RealRectangle2D >();
    }

    uno::Sequence< double > SAL_CALL TextLayout::queryLogicalAdvancements(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maLogicalAdvancements;
    }

    void SAL_CALL TextLayout::applyLogicalAdvancements( const uno::Sequence< double >& aAdvancements ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if( aAdvancements.getLength() != maText.Length )
        {
            OSL_TRACE( "TextLayout::applyLogicalAdvancements(): mismatching number of advancements" );
            throw lang::IllegalArgumentException();
        }

        maLogicalAdvancements = aAdvancements;
    }

    geometry::RealRectangle2D SAL_CALL TextLayout::queryTextBounds(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        OutputDevice* pOutDev = mpRefDevice->getOutputDevice();
        if( !pOutDev )
            return geometry::RealRectangle2D();

        VirtualDevice aVDev( *pOutDev );
        aVDev.SetFont( mpFont->getVCLFont() );

        // need metrics for Y offset, the XCanvas always renders
        // relative to baseline
        const ::FontMetric& aMetric( aVDev.GetFontMetric() );

        setupLayoutMode( aVDev, mnTextDirection );

        const sal_Int32 nAboveBaseline( -aMetric.GetIntLeading() - aMetric.GetAscent() );
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
                                              aVDev.GetTextWidth(
                                                  maText.Text,
                                                  ::canvas::tools::numeric_cast<sal_uInt16>(maText.StartPosition),
                                                  ::canvas::tools::numeric_cast<sal_uInt16>(maText.Length) ),
                                              nBelowBaseline );
        }
    }

    double SAL_CALL TextLayout::justify( double /*nSize*/ ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return 0.0;
    }

    double SAL_CALL TextLayout::combinedJustify( const uno::Sequence< uno::Reference< rendering::XTextLayout > >& /*aNextLayouts*/,
                                                 double /*nSize*/ ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return 0.0;
    }

    rendering::TextHit SAL_CALL TextLayout::getTextHit( const geometry::RealPoint2D& /*aHitPoint*/ ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return rendering::TextHit();
    }

    rendering::Caret SAL_CALL TextLayout::getCaret( sal_Int32 /*nInsertionIndex*/,
                                                    sal_Bool /*bExcludeLigatures*/ ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return rendering::Caret();
    }

    sal_Int32 SAL_CALL TextLayout::getNextInsertionIndex( sal_Int32 /*nStartIndex*/,
                                                          sal_Int32 /*nCaretAdvancement*/,
                                                          sal_Bool /*bExcludeLigatures*/ ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return 0;
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL TextLayout::queryVisualHighlighting( sal_Int32 /*nStartIndex*/,
                                                                                              sal_Int32 /*nEndIndex*/ ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return uno::Reference< rendering::XPolyPolygon2D >();
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL TextLayout::queryLogicalHighlighting( sal_Int32 /*nStartIndex*/,
                                                                                               sal_Int32 /*nEndIndex*/ ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return uno::Reference< rendering::XPolyPolygon2D >();
    }

    double SAL_CALL TextLayout::getBaselineOffset(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return 0.0;
    }

    sal_Int8 SAL_CALL TextLayout::getMainTextDirection(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return mnTextDirection;
    }

    uno::Reference< rendering::XCanvasFont > SAL_CALL TextLayout::getFont(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return mpFont.getRef();
    }

    rendering::StringContext SAL_CALL TextLayout::getText(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maText;
    }

    void TextLayout::useFont( Cairo* pCairo )
    {
        rendering::FontRequest aFontRequest = mpFont->getFontRequest();
        rendering::FontInfo aFontInfo = aFontRequest.FontDescription;

        cairo_select_font_face( pCairo, ::rtl::OUStringToOString( aFontInfo.FamilyName, RTL_TEXTENCODING_UTF8 ).getStr(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
        cairo_set_font_size( pCairo, aFontRequest.CellSize );
    }

  /** TextLayout:draw
   *
   * This function uses the "toy" api of the cairo library
   *
   **/
    bool TextLayout::draw( Cairo* pCairo )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ::rtl::OUString aSubText = maText.Text.copy( maText.StartPosition, maText.Length );
        ::rtl::OString aUTF8String = ::rtl::OUStringToOString( aSubText, RTL_TEXTENCODING_UTF8 );

        cairo_save( pCairo );
        /* move to 0, 0 as cairo_show_text advances current point and current point is not restored by cairo_restore.
           before we were depending on unmodified current point which I believed was preserved by save/restore */
        cairo_move_to( pCairo, 0, 0 );
        useFont( pCairo );
        cairo_show_text( pCairo, aUTF8String.getStr() );
        cairo_restore( pCairo );

        return true;
    }


  /**
   * TextLayout::isCairoRenderable
   *
   * Features currenly not supported by Cairo (VCL rendering is used as fallback):
   * - vertical glyphs
   *
   * @return true, if text/font can be rendered with cairo
   **/
    bool TextLayout::isCairoRenderable(SystemFontData aSysFontData) const
    {
#if defined UNX && !defined QUARTZ && !defined IOS
        // is font usable?
        if (!aSysFontData.nFontId)
            return false;
#endif

        // vertical glyph rendering is not supported in cairo for now
        if (aSysFontData.bVerticalCharacterType)
        {
            OSL_TRACE(":cairocanvas::TextLayout::isCairoRenderable(): ***************** VERTICAL CHARACTER STYLE!!! ****************");
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
   *
   * @return true, if successful
   **/
    bool TextLayout::draw( SurfaceSharedPtr&             pSurface,
                           OutputDevice&                 rOutDev,
                           const Point&                  rOutpos,
                           const rendering::ViewState&   viewState,
                           const rendering::RenderState& renderState ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        SystemTextLayoutData aSysLayoutData;
#if (defined CAIRO_HAS_WIN32_SURFACE) && (OSL_DEBUG_LEVEL > 1)
        LOGFONTW logfont;
#endif
        setupLayoutMode( rOutDev, mnTextDirection );

        // TODO(P2): cache that
        ::boost::scoped_array< sal_Int32 > aOffsets(new sal_Int32[maLogicalAdvancements.getLength()]);

        if( maLogicalAdvancements.getLength() )
        {
            setupTextOffsets( aOffsets.get(), maLogicalAdvancements, viewState, renderState );

            // TODO(F3): ensure correct length and termination for DX
            // array (last entry _must_ contain the overall width)
        }

        aSysLayoutData = rOutDev.GetSysTextLayoutData(rOutpos, maText.Text,
                                                      ::canvas::tools::numeric_cast<sal_uInt16>(maText.StartPosition),
                                                      ::canvas::tools::numeric_cast<sal_uInt16>(maText.Length),
                                                      maLogicalAdvancements.getLength() ? aOffsets.get() : NULL);

        // Sort them so that all glyphs on the same glyph fallback level are consecutive
        std::sort(aSysLayoutData.rGlyphData.begin(), aSysLayoutData.rGlyphData.end(), compareFallbacks);
        bool bCairoRenderable = true;

        //Pull all the fonts we need to render the text
        typedef std::pair<SystemFontData,int> FontLevel;
        typedef std::vector<FontLevel> FontLevelVector;
        FontLevelVector aFontData;
        SystemGlyphDataVector::const_iterator aGlyphIter=aSysLayoutData.rGlyphData.begin();
        const SystemGlyphDataVector::const_iterator aGlyphEnd=aSysLayoutData.rGlyphData.end();
        for( ; aGlyphIter != aGlyphEnd; ++aGlyphIter )
        {
            if( aFontData.empty() || aGlyphIter->fallbacklevel != aFontData.back().second )
            {
                aFontData.push_back(FontLevel(rOutDev.GetSysFontData(aGlyphIter->fallbacklevel),
                                              aGlyphIter->fallbacklevel));
                if( !isCairoRenderable(aFontData.back().first) )
                {
                    bCairoRenderable = false;
                    OSL_TRACE(":cairocanvas::TextLayout::draw(S,O,p,v,r): VCL FALLBACK %s%s%s%s - %s",
                              maLogicalAdvancements.getLength() ? "ADV " : "",
                              aFontData.back().first.bAntialias ? "AA " : "",
                              aFontData.back().first.bFakeBold ? "FB " : "",
                              aFontData.back().first.bFakeItalic ? "FI " : "",
                              ::rtl::OUStringToOString( maText.Text.copy( maText.StartPosition, maText.Length ),
                                                        RTL_TEXTENCODING_UTF8 ).getStr());
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
                return true;
            }
            else                                               // VCL FALLBACK - without advances
            {
                rOutDev.DrawText( rOutpos, maText.Text,
                                  ::canvas::tools::numeric_cast<sal_uInt16>(maText.StartPosition),
                                  ::canvas::tools::numeric_cast<sal_uInt16>(maText.Length) );
                return true;
            }
        }

        if (aSysLayoutData.rGlyphData.empty())
            return false; //??? false?

        /**
         * Setup platform independent glyph vector into cairo-based glyphs vector.
         **/

        // Loop through the fonts used and render the matching glyphs for each
        FontLevelVector::const_iterator aFontDataIter = aFontData.begin();
        const FontLevelVector::const_iterator aFontDataEnd = aFontData.end();
        for( ; aFontDataIter != aFontDataEnd; ++aFontDataIter )
        {
            const SystemFontData &rSysFontData = aFontDataIter->first;

            // setup glyphs
            std::vector<cairo_glyph_t> cairo_glyphs;
            cairo_glyphs.reserve( 256 );

            aGlyphIter=aSysLayoutData.rGlyphData.begin();
            for( ; aGlyphIter != aGlyphEnd; ++aGlyphIter )
            {
                SystemGlyphData systemGlyph = *aGlyphIter;
                if( systemGlyph.fallbacklevel != aFontDataIter->second )
                    continue;

                cairo_glyph_t aGlyph;
                aGlyph.index = systemGlyph.index;
#ifdef CAIRO_HAS_WIN32_SURFACE
                // Cairo requires standard glyph indexes (ETO_GLYPH_INDEX), while vcl/win/* uses ucs4 chars.
                // Convert to standard indexes
                aGlyph.index = cairo::ucs4toindex((unsigned int) aGlyph.index, rSysFontData.hFont);
#endif
                aGlyph.x = systemGlyph.x;
                aGlyph.y = systemGlyph.y;
                cairo_glyphs.push_back(aGlyph);
            }

            if (cairo_glyphs.empty())
                continue;

            /**
             * Setup font
             **/
            cairo_font_face_t* font_face = NULL;

#ifdef CAIRO_HAS_QUARTZ_SURFACE
# ifdef QUARTZ
            // TODO: use cairo_quartz_font_face_create_for_cgfont(cgFont)
            //       when CGFont (Mac OS X 10.5 API) is provided by the AQUA VCL backend.
            font_face = cairo_quartz_font_face_create_for_atsu_font_id((ATSUFontID) rSysFontData.aATSUFontID);
# else // iOS
            font_face = cairo_quartz_font_face_create_for_cgfont( CTFontCopyGraphicsFont( rSysFontData.rCTFont, NULL ) );
# endif

#elif defined CAIRO_HAS_WIN32_SURFACE
# if (OSL_DEBUG_LEVEL > 1)
            GetObjectW( rSysFontData.hFont, sizeof(logfont), &logfont );
# endif
            // Note: cairo library uses logfont fallbacks when lfEscapement, lfOrientation and lfWidth are not zero.
            // VCL always has non-zero value for lfWidth
            font_face = cairo_win32_font_face_create_for_hfont(rSysFontData.hFont);

#elif defined CAIRO_HAS_XLIB_SURFACE
            font_face = cairo_ft_font_face_create_for_ft_face((FT_Face)rSysFontData.nFontId,
                                                              rSysFontData.nFontFlags);
#else
# error Native API needed.
#endif

            CairoSharedPtr pSCairo = pSurface->getCairo();

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
            Color mTextColor = rOutDev.GetTextColor();
            cairo_set_source_rgb(pSCairo.get(),
                                 mTextColor.GetRed()/255.0,
                                 mTextColor.GetGreen()/255.0,
                                 mTextColor.GetBlue()/255.0);

            // Font rotation and scaling
            cairo_matrix_t m;
            Font aFont = rOutDev.GetFont();
            FontMetric aMetric( rOutDev.GetFontMetric(aFont) );
            long nWidth = 0;

            // width calculation is deep magic and platform/font dependant.
            // width == 0 means no scaling, and usually width == height means the same.
            // Other values mean horizontal scaling (narrow or stretching)
            // see issue #101566

            //proper scale calculation across platforms
            if (aFont.GetWidth() == 0)
            {
                nWidth = aFont.GetHeight();
            }
            else
            {
                // any scaling needs to be relative to the platform-dependent definition
                // of height of the font
                nWidth = aFont.GetWidth() * aFont.GetHeight() / aMetric.GetHeight();
            }

            cairo_matrix_init_identity(&m);

            if (aSysLayoutData.orientation)
                cairo_matrix_rotate(&m, (3600 - aSysLayoutData.orientation) * M_PI / 1800.0);

            cairo_matrix_scale(&m, nWidth, aFont.GetHeight());

            //faux italics
            if (rSysFontData.bFakeItalic)
                m.xy = -m.xx * 0x6000L / 0x10000L;

            cairo_set_font_matrix(pSCairo.get(), &m);

#if (defined CAIRO_HAS_WIN32_SURFACE) && (OSL_DEBUG_LEVEL > 1)
# define TEMP_TRACE_FONT ::rtl::OUStringToOString( reinterpret_cast<const sal_Unicode*> (logfont.lfFaceName), RTL_TEXTENCODING_UTF8 ).getStr()
#else
# define TEMP_TRACE_FONT ::rtl::OUStringToOString( aFont.GetName(), RTL_TEXTENCODING_UTF8 ).getStr()
#endif
            OSL_TRACE("\r\n:cairocanvas::TextLayout::draw(S,O,p,v,r): Size:(%d,%d), W:%d->%d, Pos (%d,%d), G(%d,%d,%d) %s%s%s%s || Name:%s - %s",
                      aFont.GetWidth(),
                      aFont.GetHeight(),
                      aMetric.GetWidth(),
                      nWidth,
                      (int) rOutpos.X(),
                      (int) rOutpos.Y(),
                      cairo_glyphs.size() > 0 ? cairo_glyphs[0].index : -1,
                      cairo_glyphs.size() > 1 ? cairo_glyphs[1].index : -1,
                      cairo_glyphs.size() > 2 ? cairo_glyphs[2].index : -1,
                      maLogicalAdvancements.getLength() ? "ADV " : "",
                      rSysFontData.bAntialias ? "AA " : "",
                      rSysFontData.bFakeBold ? "FB " : "",
                      rSysFontData.bFakeItalic ? "FI " : "",
                      TEMP_TRACE_FONT,
                      ::rtl::OUStringToOString( maText.Text.copy( maText.StartPosition, maText.Length ),
                                                RTL_TEXTENCODING_UTF8 ).getStr()
                );
#undef TEMP_TRACE_FONT

            cairo_show_glyphs(pSCairo.get(), &cairo_glyphs[0], cairo_glyphs.size());

            //faux bold
            if (rSysFontData.bFakeBold)
            {
                double bold_dx = 0.5 * sqrt( 0.7 * aFont.GetHeight() );
                int total_steps = 1 * ((int) (bold_dx + 0.5));

                // loop to draw the text for every half pixel of displacement
                for (int nSteps = 0; nSteps < total_steps; nSteps++)
                {
                    for(int nGlyphIdx = 0; nGlyphIdx < (int) cairo_glyphs.size(); nGlyphIdx++)
                    {
                        cairo_glyphs[nGlyphIdx].x += (bold_dx * nSteps / total_steps) / 4;
                        cairo_glyphs[nGlyphIdx].y -= (bold_dx * nSteps / total_steps) / 4;
                    }
                    cairo_show_glyphs(pSCairo.get(), &cairo_glyphs[0], cairo_glyphs.size());
                }
                OSL_TRACE(":cairocanvas::TextLayout::draw(S,O,p,v,r): FAKEBOLD - dx:%d", (int) bold_dx);
            }

            cairo_restore( pSCairo.get() );
            cairo_font_face_destroy(font_face);
        }
        return true;
    }


    namespace
    {
        class OffsetTransformer
        {
        public:
            OffsetTransformer( const ::basegfx::B2DHomMatrix& rMat ) :
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

    void TextLayout::setupTextOffsets( sal_Int32*                       outputOffsets,
                                       const uno::Sequence< double >&   inputOffsets,
                                       const rendering::ViewState&      viewState,
                                       const rendering::RenderState&    renderState     ) const
    {
        ENSURE_OR_THROW( outputOffsets!=NULL,
                          "TextLayout::setupTextOffsets offsets NULL" );

        ::basegfx::B2DHomMatrix aMatrix;

        ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                     viewState,
                                                     renderState);

        // fill integer offsets
        ::std::transform( const_cast< uno::Sequence< double >& >(inputOffsets).getConstArray(),
                          const_cast< uno::Sequence< double >& >(inputOffsets).getConstArray()+inputOffsets.getLength(),
                          outputOffsets,
                          OffsetTransformer( aMatrix ) );
    }

#define SERVICE_NAME "com.sun.star.rendering.TextLayout"
#define IMPLEMENTATION_NAME "CairoCanvas::TextLayout"

    ::rtl::OUString SAL_CALL TextLayout::getImplementationName() throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL TextLayout::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName == SERVICE_NAME;
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL TextLayout::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
