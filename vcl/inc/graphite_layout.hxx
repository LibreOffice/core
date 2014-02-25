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

#ifndef INCLUDED_VCL_INC_GRAPHITE_LAYOUT_HXX
#define INCLUDED_VCL_INC_GRAPHITE_LAYOUT_HXX
// Description: An implementation of the SalLayout interface that uses the
//              Graphite engine.

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

// Standard Library
#include <memory>
#include <vector>
#include <map>
#include <utility>
// Libraries
#include <graphite_static.hxx>
#include <graphite2/Font.h>
#include <graphite2/Segment.h>
// Platform
#include <sallayout.hxx>
#include <vcl/dllapi.h>
// Module

// Module type definitions and forward declarations.
// SAL/VCL types
class ServerFont;

// Graphite types
namespace grutils { class GrFeatureParser; }

class GraphiteFaceWrapper
{
public:
    typedef std::map<int, gr_font*> GrFontMap;
    GraphiteFaceWrapper(gr_face * pFace) : m_pFace(pFace) {}
    ~GraphiteFaceWrapper()
    {
        GrFontMap::iterator i = m_fonts.begin();
        while (i != m_fonts.end())
            gr_font_destroy((*i++).second);
        m_fonts.clear();
        gr_face_destroy(m_pFace);
    }
    const gr_face * face() const { return m_pFace; }
    gr_font * font(int ppm) const
    {
        GrFontMap::const_iterator i = m_fonts.find(ppm);
        if (i != m_fonts.end())
            return i->second;
        return NULL;
    };
    void addFont(int ppm, gr_font * pFont)
    {
        if (m_fonts[ppm])
            gr_font_destroy(m_fonts[ppm]);
        m_fonts[ppm] = pFont;
    }
private:
    gr_face * m_pFace;
    GrFontMap m_fonts;
};

// This class uses the SIL Graphite engine to provide complex text layout services to the VCL
// @author tse

class VCL_PLUGIN_PUBLIC GraphiteLayout : public SalLayout
{
public:

    class Glyphs : public std::vector<GlyphItem>
    {
    public:
        typedef std::pair<Glyphs::const_iterator, Glyphs::const_iterator> iterator_pair_t;

    };

    mutable Glyphs          mvGlyphs;
    void clear();

private:
    const gr_face *         mpFace; // not owned by layout
    gr_font *               mpFont; // not owned by layout
    int                     mnSegCharOffset; // relative to ImplLayoutArgs::mpStr
    long                    mnWidth;
    std::vector<int>        mvChar2BaseGlyph;
    std::vector<int>        mvGlyph2Char;
    std::vector<int>        mvCharDxs;
    std::vector<int>        mvCharBreaks;
    float                   mfScaling;
    const grutils::GrFeatureParser * mpFeatures;

public:
    GraphiteLayout(const gr_face * pFace, gr_font * pFont = NULL,
        const grutils::GrFeatureParser * features = NULL) throw();

    // used by upper layers
    virtual bool  LayoutText( ImplLayoutArgs& );    // first step of layout
    // split into two stages to allow dc to be restored on the segment
    gr_segment * CreateSegment(ImplLayoutArgs& rArgs);
    bool LayoutGlyphs(ImplLayoutArgs& rArgs, gr_segment * pSegment);

    virtual void  AdjustLayout( ImplLayoutArgs& );  // adjusting positions

    // methods using string indexing
    virtual sal_Int32 GetTextBreak(long nMaxWidth, long nCharExtra=0, int nFactor=1) const SAL_OVERRIDE;
    virtual long  FillDXArray( sal_Int32* pDXArray ) const;
    virtual void  ApplyDXArray(ImplLayoutArgs &rArgs, std::vector<int> & rDeltaWidth);

    virtual void  GetCaretPositions( int nArraySize, sal_Int32* pCaretXArray ) const;

    // methods using glyph indexing
    virtual int   GetNextGlyphs(int nLen, sal_GlyphId* pGlyphIdxAry, ::Point & rPos, int&,
            sal_Int32* pGlyphAdvAry = NULL, int* pCharPosAry = NULL,
            const PhysicalFontFace** pFallbackFonts = NULL ) const;

    // used by glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos );
    virtual void    DropGlyph( int nStart );
    virtual void    Simplify( bool bIsBase );

    // Dummy implementation so layout can be shared between Linux/Windows
    virtual void    DrawText(SalGraphics&) const {};

    virtual ~GraphiteLayout() throw();
    void SetFont(gr_font * pFont) { mpFont = pFont; }
    gr_font * GetFont() { return mpFont; }
    void SetFeatures(grutils::GrFeatureParser * aFeature) { mpFeatures = aFeature; }
    void SetFontScale(float s) { mfScaling = s; };
    virtual sal_GlyphId getKashidaGlyph(int & width) = 0;
    void kashidaJustify(std::vector<int> & rDeltaWidth, sal_GlyphId, int width);

    static const int EXTRA_CONTEXT_LENGTH;
private:
    void expandOrCondense(ImplLayoutArgs &rArgs);
    void    fillFrom(gr_segment * rSeg, ImplLayoutArgs & rArgs, float fScaling);

    float append(gr_segment * pSeg,
                ImplLayoutArgs & rArgs,
                const gr_slot * pSlot, float gOrigin,
                float nextGlyphOrigin, float fScaling,
                long & rDXOffset, bool bIsBase, int baseChar);
};

#endif // INCLUDED_VCL_INC_GRAPHITE_LAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
