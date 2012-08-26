/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
************************************************************************/

#include "ios/common.h"
#include "ios/salcoretextstyle.hxx"
#include "ios/salcoretextlayout.hxx"
#include "ios/salgdi.h"


CoreTextLayout::CoreTextLayout(IosSalGraphics* graphics, CoreTextStyleInfo* style) :
    m_graphics(graphics),
    m_style(style),
    m_glyphs_count(-1),
    m_chars_count(-1),
    m_chars2glyphs(NULL),
    m_glyphs2chars(NULL),
    m_glyphs(NULL),
    m_char_widths(NULL),
    m_glyph_advances(NULL),
    m_glyph_positions(NULL),
    m_typesetter(NULL),
    m_line(NULL),
    m_has_bound_rec(false),
    m_base_advance(0),
    m_cached_width(0.0F),
    m_current_run_index(0),
    m_current_glyph_index(0),
    m_current_glyphrun_index(0),
    m_runs(NULL)
{
}

CoreTextLayout::~CoreTextLayout()
{
    Clean();
}

void CoreTextLayout::AdjustLayout( ImplLayoutArgs& /*rArgs*/ )
{
    msgs_debug(layout,"-->");
    msgs_debug(layout,"<--");
    /* TODO */
}

void CoreTextLayout::Clean()
{
    msgs_debug(layout,"-->");
    if (m_glyphs)
    {
        delete[] m_glyphs;
        m_glyphs = NULL;
    }
    if (m_chars2glyphs)
    {
        delete[] m_chars2glyphs;
        m_chars2glyphs = NULL;
    }
    if (m_glyphs2chars)
    {
        delete[] m_glyphs2chars;
        m_glyphs2chars = NULL;
    }
    if (m_char_widths)
    {
        delete[] m_char_widths;
        m_char_widths = NULL;
    }
    if (m_glyph_advances)
    {
        delete[] m_glyph_advances;
        m_glyph_advances = NULL;
    }
    if (m_glyph_positions)
    {
        delete[] m_glyph_positions;
        m_glyph_positions = NULL;
    }
    SafeCFRelease(m_typesetter);
    SafeCFRelease(m_line);
    m_has_bound_rec = false;
    msgs_debug(layout,"<--");
}

void CoreTextLayout::DrawText( SalGraphics& rGraphics ) const
{
    msgs_debug(layout,"-->");
    IosSalGraphics& gr = static_cast<IosSalGraphics&>(rGraphics);
    if (m_chars_count <= 0 || !gr.CheckContext())
    {
        return;
    }
    CGContextSaveGState( gr.mrContext );
    Point pos = GetDrawPosition(Point(0,0));
#if 0
    msgs_debug(layout,"at pos (%ld, %ld)", pos.X(), pos.Y());
    CGContextSetTextMatrix(gr.mrContext, CGAffineTransformMakeScale(1.0, -1.0));
    CGContextSetShouldAntialias( gr.mrContext, !gr.mbNonAntialiasedText );
    CGContextSetTextPosition(gr.mrContext, pos.X(), pos.Y());
    CTLineDraw(m_line, gr.mrContext);
#else
    InitGIA();
    msgs_debug(layout,"at- pos (%ld, %ld)", pos.X(), pos.Y());
    CGFontRef cg_font = CTFontCopyGraphicsFont(m_style->GetFont(), NULL);
    CGContextSetFont(gr.mrContext, cg_font);
    CGContextSetFontSize(gr.mrContext, CTFontGetSize(m_style->GetFont()));
    CGContextSetTextDrawingMode(gr.mrContext, kCGTextFill);
    CGContextSetShouldAntialias( gr.mrContext, true );
    if (m_style->GetColor())
    {
        CGContextSetFillColorWithColor(gr.mrContext, m_style->GetColor());
        CGContextSetStrokeColorWithColor(gr.mrContext, m_style->GetColor());
    }
    else
    {
        CGContextSetRGBFillColor(gr.mrContext, 0.0, 0.0, 0.0, 1.0);
    }
    CFRelease(cg_font);
//    CGContextSetTextPosition(gr.mrContext, pos.X(), pos.Y());
    CGContextSetTextMatrix(gr.mrContext, CGAffineTransformMakeScale(1.0, -1.0));
    CGContextSetShouldAntialias( gr.mrContext, !gr.mbNonAntialiasedText );
    CGContextTranslateCTM(gr.mrContext, pos.X(), pos.Y());
//    for(int i = 0; i < m_glyphs_count ; ++i)
//    {
//        msgs_debug(layout,"m_glyph=%p m_glyph_positions=%p count=%d", m_glyphs, m_glyph_positions, m_glyphs_count);
//        msgs_debug(layout,"glyph[%d]=0x%x position(%g,%g)", i, m_glyphs[i], m_glyph_positions[i].x, m_glyph_positions[i].y);
        CGContextShowGlyphs(gr.mrContext, m_glyphs, m_glyphs_count);
//        CGContextShowGlyphsAtPositions(gr.mrContext, m_glyphs, m_glyph_positions, m_glyphs_count);
//        CGContextShowGlyphsWidthAdvances(gr.mrContext, m_glyphs, m_glyph_advances, m_glyphs_count);

//        CGContextShowGlyphsAtPoint(gr.mrContext, pos.X(), pos.Y(), m_glyphs, m_glyphs_count);
//    }
#endif
    // restore the original graphic context transformations
    CGContextRestoreGState( gr.mrContext );
    msgs_debug(layout,"<--");

}

// not needed. CoreText manage fallback directly
void CoreTextLayout::DropGlyph( int /*nStart*/ ) {}

long CoreTextLayout::FillDXArray( long* pDXArray ) const
{
    msgs_debug(layout,"-->");
    // short circuit requests which don't need full details
    if ( !pDXArray )
    {
        return GetTextWidth();
    }
    // check assumptions
    // DBG_ASSERT( !mnTrailingSpaceWidth, "CoreText::FillDXArray() with nTSW!=0" );

    // initialize details about the resulting layout
    InitGIA();

    // distribute the widths among the string elements
    long width = 0;
    float scale = m_style->GetFontStretchFactor();
    m_cached_width = 0;

    for( int i = 0; i < m_chars_count; ++i )
    {
        // convert and adjust for accumulated rounding errors
        m_cached_width += m_char_widths[i];
        const long old_width = width;
        width = round_to_long(m_cached_width * scale);
        pDXArray[i] = width - old_width;
    }
    msgs_debug(layout," w=%ld <--", width);
    return width;
}

bool CoreTextLayout::GetBoundRect( SalGraphics &rGraphics, Rectangle& rVCLRect ) const
{
    msgs_debug(layout,"-->");
    IosSalGraphics& gr = static_cast<IosSalGraphics&>(rGraphics);
    if ( !m_has_bound_rec )
    {
        CGRect bound_rect = CTLineGetImageBounds( m_line, gr.mrContext );
        if ( !CGRectIsNull( bound_rect ) )
        {
            m_bound_rect = Rectangle(
                Point( round_to_long(bound_rect.origin.x * m_style->GetFontStretchFactor()),
                       round_to_long(bound_rect.origin.y - bound_rect.size.height )),
                Size( round_to_long(bound_rect.size.width * m_style->GetFontStretchFactor()), round_to_long(bound_rect.size.height)));
            m_bound_rect.Justify();
        }
        m_has_bound_rec = true;
    }
    rVCLRect = m_bound_rect;
    msgs_debug(layout,"<--");
    return true;
}

void CoreTextLayout::GetCaretPositions( int max_index, long* caret_position) const
{
    msgs_debug(layout,"max_index %d -->", max_index);
    int local_max = max_index < m_chars_count * 2 ? max_index : m_chars_count;
    for(int i = 0 ; i < max_index - 1; i+=2)
    {
        CGFloat primary, secondary;
        primary = CTLineGetOffsetForStringIndex(m_line, i >> 1, &secondary);
        caret_position[i] = round_to_long(m_base_advance + primary);
        caret_position[i+1] = round_to_long(m_base_advance + secondary);
        i += 2;
    }
    for(int i = local_max ; i < max_index ; ++i)
    {
        caret_position[i] = -1;
    }
    msgs_debug(layout,"<--");
}

bool CoreTextLayout::GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const { return false; }

int CoreTextLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIDs, Point& rPos, int& nStart,
                                   sal_Int32* pGlyphAdvances, int* pCharIndexes ) const
{
    msgs_debug(layout,"nLen=%d nStart=%d-->", nLen, nStart);
    // get glyph measurements
    InitGIA();

    if ( nStart < 0 )                // first glyph requested?
    {
        nStart = 0;
        m_current_run_index = 0;
        m_current_glyph_index = 0;
        m_current_glyphrun_index = 0;
    }
    else if (nStart >= m_glyphs_count)
    {
        m_current_run_index = 0;
        m_current_glyph_index = 0;
        m_current_glyphrun_index = 0;
        return 0;
    }
    if (!m_runs)
    {
        m_runs = CTLineGetGlyphRuns(m_line);
    }
    CFIndex nb_runs = CFArrayGetCount( m_runs );
    CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex( m_runs, m_current_run_index );
    CFIndex nb_glyphs = CTRunGetGlyphCount( run );

    int i = 0;
    bool first = true;
    while(i < nLen)
    {
        if (m_current_glyphrun_index >= nb_glyphs)
        {
            m_current_run_index += 1;
            if (m_current_run_index >= nb_runs)
            {
                break;
            }
            run = (CTRunRef)CFArrayGetValueAtIndex( m_runs, m_current_run_index );
            nb_glyphs = CTRunGetGlyphCount( run );
            m_current_glyphrun_index = 0;
        }
        if (first)
        {
            CGPoint first_pos;
            CTRunGetPositions(run, CFRangeMake(m_current_glyphrun_index,1), &first_pos);
            Point pos(first_pos.x, first_pos.y);
            rPos = GetDrawPosition(pos);
            msgs_debug(layout,"rPos(%ld, %ld)", rPos.X(),rPos.Y());
            first = false;
        }
        pGlyphIDs[i] = m_glyphs[m_current_glyph_index];
        if (pGlyphAdvances)
        {
            pGlyphAdvances[i] = m_glyph_advances[m_current_glyph_index];
        }
        if (pCharIndexes)
        {
            pCharIndexes[i] = m_glyphs2chars[m_current_glyph_index];
        }
        m_current_glyph_index += 1;
        m_current_glyphrun_index += 1;
        i += 1;
        nStart += 1;
    }
    msgs_debug(layout,"i=%d <--", i);
    return i;
}

int CoreTextLayout::GetTextBreak( long /*nMaxWidth*/, long /*nCharExtra*/, int /*nFactor*/ ) const
{
    /* TODO */
    return false;
}

long CoreTextLayout::GetTextWidth() const
{
    msgs_debug(layout,"-->");

    CGRect bound_rect = CTLineGetImageBounds(m_line, m_graphics->GetContext());
    long w = round_to_long(bound_rect.size.width * m_style->GetFontStretchFactor());
    msgs_debug(layout,"w=%ld <--", w);
    return w;
}

// not needed. CoreText manage fallback directly
void CoreTextLayout::InitFont()  const
{
}

bool CoreTextLayout::InitGIA() const
{
    msgs_debug(layout,"count=%d <--",  m_chars_count);

    if ( m_chars_count <= 0)
    {
        return false;
    }
    if (m_glyphs)
    {
        return true;
    }

    m_glyphs = new CGGlyph[m_glyphs_count];
    m_char_widths = new int[ m_chars_count ];
    m_chars2glyphs = new int[ m_chars_count ];
    for( int i = 0; i < m_chars_count; ++i)
    {
        m_char_widths[i] = 0.0;
        m_chars2glyphs[i] = -1;
    }
    m_glyphs2chars = new int[m_glyphs_count];
    m_glyph_advances = new int[m_glyphs_count];
    m_glyph_positions = new CGPoint[m_glyphs_count];


    CFArrayRef runs = CTLineGetGlyphRuns( m_line );
    CFIndex nb_runs = CFArrayGetCount( runs );
    int p = 0;
    for( CFIndex i = 0; i < nb_runs; ++i )
    {
        CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex( runs, i );
        if ( run )
        {
			CFIndex nb_glyphs = CTRunGetGlyphCount( run );
            if (nb_glyphs)
            {
                CFRange text_range = CTRunGetStringRange( run );
                if ( text_range.location != kCFNotFound && text_range.length > 0 )
                {
                    CFIndex indices[ nb_glyphs ];
                    CGGlyph glyphs[ nb_glyphs ];
                    CTRunGetStringIndices( run, CFRangeMake( 0, 0 ), indices );
                    CTRunGetGlyphs( run, CFRangeMake( 0, 0 ), glyphs );
                    CTRunGetPositions( run, CFRangeMake( 0, 0 ), &m_glyph_positions[p] );
                    bool is_vertical_run = false;
                    CFDictionaryRef aDict = CTRunGetAttributes( run );
                    if ( aDict )
                    {
                        const CFBooleanRef aValue = (const CFBooleanRef)CFDictionaryGetValue( aDict, kCTVerticalFormsAttributeName );
                        is_vertical_run =  (aValue == kCFBooleanTrue) ? true : false;
                    }

                    for (CFIndex j = 0 ; j < nb_glyphs; ++p, ++j )
                    {
                        m_glyphs[ p ] = glyphs[ j ];
                        CFIndex k = indices[ j ];
                        m_glyphs2chars[p] = k;
                        m_chars2glyphs[k] = p;

                        if ( j < nb_glyphs - 1 )
                        {
                            m_char_widths[ k ] += m_glyph_positions[ p + 1 ].x - m_glyph_positions[ p ].x;
                        }
                        if ( p > 0)
                        {
                            m_glyph_advances[p - 1] = m_glyph_positions[ p ].x - m_glyph_positions[p - 1].x;
                        }
                    }
                }
			}
        }
    }
    msgs_debug(layout,"<--");
    return true;
}

bool CoreTextLayout::LayoutText(ImplLayoutArgs& args)
{
    msgs_debug(layout,"-->");
    Clean();
    m_style->SetColor();
    /* retreive MinCharPos EndCharPos Flags and Orientation */
    SalLayout::AdjustLayout(args);
    m_chars_count = mnEndCharPos - mnMinCharPos;

    /* don't layout emptty (or worse negative size) strings */
    if (m_chars_count <= 0)
    {
        return false;
    }
    /* c0 and c1 are construction objects */
    CFStringRef c0 = CFStringCreateWithCharactersNoCopy( NULL, &(args.mpStr[args.mnMinCharPos]), m_chars_count, kCFAllocatorNull );
    if ( !c0 )
    {
        Clean();
        return false;
    }

    CFStringRef keys[6];
    CFTypeRef   values[6];
    int nb_attributes = 0;

    keys[nb_attributes]= kCTFontAttributeName;
    values[nb_attributes] = m_style->GetFont();
    nb_attributes += 1;

    CFDictionaryRef attributes = CFDictionaryCreate(kCFAllocatorDefault,
                                                    (const void**)&keys,
                                                    (const void**)&values,
                                                    nb_attributes,
                                                    &kCFTypeDictionaryKeyCallBacks,
                                                    &kCFTypeDictionaryValueCallBacks);


    CFAttributedStringRef string = CFAttributedStringCreate( NULL, c0, attributes );
    CFRelease( c0 );
    CFRelease( attributes );
    if ( !string )
    {
        Clean();
        return false;
    }
    m_typesetter = CTTypesetterCreateWithAttributedString(string);
    CFRelease(string);
    if (!m_typesetter)
    {
        Clean();
        return false;
    }
    m_line = CTTypesetterCreateLine(m_typesetter, CFRangeMake(0, 0));
    if (!m_line)
    {
        Clean();
        return false;
    }
    m_glyphs_count = CTLineGetGlyphCount(m_line);

    msgs_debug(layout,"glyph_count=%d <--", m_glyphs_count);
    return true;
}

// not needed. CoreText manage fallback directly
void CoreTextLayout::MoveGlyph( int /*nStart*/, long /*nNewXPos*/ ) {}

// not needed. CoreText manage fallback directly
void CoreTextLayout::Simplify( bool /*bIsBase*/ ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
