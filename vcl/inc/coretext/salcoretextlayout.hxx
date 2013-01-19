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

#ifndef _VCL_CORETEXT_SALCORETEXTLAYOUT_HXX
#define _VCL_CORETEXT_SALCORETEXTLAYOUT_HXX

#include <tools/poly.hxx>

#include "sallayout.hxx"
#include "coretext/salgdi.h"
#include "coretext/salcoretextstyle.hxx"

class CoreTextLayout : public SalLayout
{
public:
    CoreTextLayout( QuartzSalGraphics* graphics, CoreTextStyleInfo* style);
    virtual         ~CoreTextLayout();

    virtual void AdjustLayout( ImplLayoutArgs& );
    virtual void DrawText( SalGraphics& ) const;
    virtual void DropGlyph( int nStart );
    virtual long FillDXArray( sal_Int32* pDXArray ) const;
    virtual bool GetBoundRect( SalGraphics&, Rectangle& ) const;
    virtual void GetCaretPositions( int nArraySize, sal_Int32* pCaretXArray ) const;
    virtual bool GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const;
    virtual int GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                               sal_Int32* pGlyphAdvances, int* pCharIndexes ) const;
    virtual int GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
    virtual long GetTextWidth() const;
    virtual void InitFont() const;
    virtual bool LayoutText( ImplLayoutArgs& );
    virtual void MoveGlyph( int nStart, long nNewXPos );
    virtual void Simplify( bool bIsBase );

private:
    void Clean();
    bool InitGIA() const;

    QuartzSalGraphics* m_graphics;
    CoreTextStyleInfo* m_style;
    mutable int m_glyphs_count;
    mutable int m_chars_count;
    mutable int* m_chars2glyphs;
    mutable int* m_glyphs2chars;
    mutable CGGlyph* m_glyphs;
    mutable int* m_char_widths;
    mutable int* m_glyph_advances;
    mutable CGPoint* m_glyph_positions;
    CTTypesetterRef m_typesetter;
    CTLineRef m_line;
    mutable bool m_has_bound_rec;
    mutable Rectangle m_bound_rect;
    CGFloat m_base_advance;
    mutable CGFloat m_cached_width;
    mutable CFIndex m_current_run_index;
    mutable CFIndex m_current_glyph_index;
    mutable CFIndex m_current_glyphrun_index;
    mutable CFArrayRef m_runs;

};

#endif // _VCL_CORETEXT_SALCORETEXTLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
