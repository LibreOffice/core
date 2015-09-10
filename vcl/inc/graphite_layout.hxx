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

#include <memory>
#include <vector>
#include <map>
#include <utility>

#include <graphite_static.hxx>
#include <graphite2/Font.h>
#include <graphite2/Segment.h>

#include <vcl/dllapi.h>

#include "sallayout.hxx"

class ServerFont;
class PhysicalFontFace;

namespace grutils { class GrFeatureParser; }

class GraphiteFaceWrapper
{
public:
    typedef std::pair<int, int> GrFontMapKey;
    typedef std::map<GrFontMapKey, gr_font*> GrFontMap;
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
    gr_font * font(int ppm, bool isBold, bool isItalic) const
    {
        int styleKey = int(isBold) | (int(isItalic) << 1);
        GrFontMap::const_iterator i = m_fonts.find(GrFontMapKey(ppm, styleKey));
        if (i != m_fonts.end())
            return i->second;
        return NULL;
    };
    void addFont(int ppm, gr_font * pFont, bool isBold, bool isItalic)
    {
        int styleKey = int(isBold) | (int(isItalic) << 1);
        GrFontMapKey key(ppm, styleKey);
        if (m_fonts[key])
            gr_font_destroy(m_fonts[key]);
        m_fonts[key] = pFont;
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
    typedef std::vector<GlyphItem> Glyphs;

    mutable Glyphs          mvGlyphs;
    void clear();

private:
    const gr_face *         mpFace; // not owned by layout
    gr_font *               mpFont; // not owned by layout
    int                     mnSegCharOffset; // relative to ImplLayoutArgs::mpStr
    long                    mnWidth;
    std::vector<int>        mvChar2BaseGlyph;
    std::vector<int>        mvChar2Glyph;
    std::vector<int>        mvGlyph2Char;
    std::vector<int>        mvCharDxs;
    std::vector<int>        mvCharBreaks;
    float                   mfScaling;
    const grutils::GrFeatureParser * mpFeatures;

public:
    GraphiteLayout(const gr_face * pFace, gr_font * pFont = NULL,
        const grutils::GrFeatureParser * features = NULL) throw();

    // used by upper layers
    virtual bool  LayoutText( ImplLayoutArgs& ) SAL_OVERRIDE;    // first step of layout
    // split into two stages to allow dc to be restored on the segment

    virtual void  AdjustLayout( ImplLayoutArgs& ) SAL_OVERRIDE;  // adjusting positions

    // methods using string indexing
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra=0, int nFactor=1) const SAL_OVERRIDE;
    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const SAL_OVERRIDE;
    void  ApplyDXArray(ImplLayoutArgs &rArgs, std::vector<int> & rDeltaWidth);

    virtual void  GetCaretPositions( int nArraySize, long* pCaretXArray ) const SAL_OVERRIDE;

    // methods using glyph indexing
    virtual int   GetNextGlyphs(int nLen, sal_GlyphId* pGlyphIdxAry, ::Point & rPos, int&,
            long* pGlyphAdvAry = NULL, int* pCharPosAry = NULL,
            const PhysicalFontFace** pFallbackFonts = NULL ) const SAL_OVERRIDE;

    // used by glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos ) SAL_OVERRIDE;
    virtual void    DropGlyph( int nStart ) SAL_OVERRIDE;
    virtual void    Simplify( bool bIsBase ) SAL_OVERRIDE;

    // Dummy implementation so layout can be shared between Linux/Windows
    virtual void    DrawText(SalGraphics&) const SAL_OVERRIDE {};

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
    void    fillFrom(gr_segment * rSeg, ImplLayoutArgs & rArgs, float fScaling, bool bRtl, int firstCharOffset);

    float append(gr_segment * pSeg,
                ImplLayoutArgs & rArgs,
                const gr_slot * pSlot, float gOrigin,
                float nextGlyphOrigin, float fScaling,
                long & rDXOffset, bool bIsBase, int baseChar, int baseGlyph, bool bRtl);
    int ScanFwdForChar(int &findChar, bool fallback) const;
};

#endif // INCLUDED_VCL_INC_GRAPHITE_LAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
