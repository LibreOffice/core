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

#ifndef _SV_GRAPHITELAYOUT_HXX
#define _SV_GRAPHITELAYOUT_HXX
// Description: An implementation of the SalLayout interface that uses the
//              Graphite engine.

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

#define GRCACHE 1

// Standard Library
#include <memory>
#include <vector>
#include <utility>
// Libraries
#include <graphite/GrClient.h>
#include <graphite/Font.h>
#include <graphite/GrConstants.h>
#include <graphite/GrAppData.h>
#include <graphite/SegmentAux.h>
// Platform
#include <vcl/sallayout.hxx>
#include <vcl/dllapi.h>
// Module

// For backwards compatibility with 2.4.x
#if (SUPD == 680)
typedef sal_Int32 sal_GlyphId;
#endif


// Module type definitions and forward declarations.
//
class TextSourceAdaptor;
class GraphiteFontAdaptor;
class GrSegRecord;
// SAL/VCL types
class ServerFont;

#ifdef WNT
// The GraphiteWinFont is just a wrapper to enable GrFontHasher to be a friend
// so that UniqueCacheInfo can be called.
#include <graphite/WinFont.h>
class GraphiteWinFont : public gr::WinFont
{
    friend class GrFontHasher;
public:
    GraphiteWinFont(HDC hdc) : gr::WinFont(hdc) {};
    virtual ~GraphiteWinFont() {};
};
#endif
// Graphite types
namespace gr { class Segment; class GlyphIterator; }
namespace grutils { class GrFeatureParser; }

// This class uses the SIL Graphite engine to provide complex text layout services to the VCL
// @author tse
//
class VCL_PLUGIN_PUBLIC GraphiteLayout : public SalLayout
{
public:
    // Mask to allow Word break status to be stored within mvChar2BaseGlyph
    enum {
        WORD_BREAK_BEFORE   = 0x40000000,
        HYPHEN_BREAK_BEFORE = 0x80000000,
        BREAK_MASK          = 0xC0000000,
        GLYPH_INDEX_MASK    = 0x3FFFFFFF
    } LineBreakMask;

    class Glyphs : public std::vector<GlyphItem>
    {
    public:
        typedef std::pair<Glyphs::const_iterator, Glyphs::const_iterator> iterator_pair_t;

        void    fill_from(gr::Segment & rSeg, ImplLayoutArgs & rArgs,
            bool bRtl, long &rWidth, float fScaling,
            std::vector<int> & rChar2Base, std::vector<int> & rGlyph2Char,
            std::vector<int> & rCharDxs);
        void    move_glyph(Glyphs::iterator, long dx);

        const_iterator    cluster_base(const_iterator) const;
        iterator_pair_t    neighbour_clusters(const_iterator) const;
    private:
        std::pair<float,float> appendCluster(gr::Segment & rSeg, ImplLayoutArgs & rArgs,
            bool bRtl, float fSegmentAdvance, int nFirstCharInCluster, int nNextChar,
            int nFirstGlyphInCluster, int nNextGlyph, float fScaling,
            std::vector<int> & rChar2Base, std::vector<int> & rGlyph2Char,
            std::vector<int> & rCharDxs, long & rDXOffset);
        void         append(gr::Segment & rSeg, ImplLayoutArgs & rArgs, gr::GlyphInfo & rGi, float nextGlyphOrigin, float fScaling, std::vector<int> & rChar2Base, std::vector<int> & rGlyph2Char, std::vector<int> & rCharDxs, long & rDXOffset, bool bIsBase);
    };

    mutable Glyphs          mvGlyphs;
    void clear();

private:
    TextSourceAdaptor     * mpTextSrc; // Text source.
    gr::LayoutEnvironment   maLayout;
    const gr::Font         &mrFont;
    long                    mnWidth;
    std::vector<int>        mvCharDxs;
    std::vector<int>        mvChar2BaseGlyph;
    std::vector<int>        mvGlyph2Char;
    float                   mfScaling;
    const grutils::GrFeatureParser * mpFeatures;

public:
    explicit GraphiteLayout( const gr::Font& font, const grutils::GrFeatureParser* features = NULL ) throw();

    // used by upper layers
    virtual bool  LayoutText( ImplLayoutArgs& );    // first step of layout
    // split into two stages to allow dc to be restored on the segment
#ifdef GRCACHE
    gr::Segment * CreateSegment(ImplLayoutArgs& rArgs, GrSegRecord ** pRecord = NULL);
    bool LayoutGlyphs(ImplLayoutArgs& rArgs, gr::Segment * pSegment, GrSegRecord * pSegRecord);
#else
    gr::Segment * CreateSegment(ImplLayoutArgs& rArgs);
    bool LayoutGlyphs(ImplLayoutArgs& rArgs, gr::Segment * pSegment);
#endif

    virtual void  AdjustLayout( ImplLayoutArgs& );  // adjusting positions

    // methods using string indexing
    virtual int   GetTextBreak( long nMaxWidth, long nCharExtra=0, int nFactor=1 ) const;
    virtual long  FillDXArray( sal_Int32* pDXArray ) const;
    virtual void  ApplyDXArray(ImplLayoutArgs &rArgs, std::vector<int> & rDeltaWidth);

    virtual void  GetCaretPositions( int nArraySize, sal_Int32* pCaretXArray ) const;

    // methods using glyph indexing
    virtual int   GetNextGlyphs(int nLen, sal_GlyphId* pGlyphIdxAry, ::Point & rPos, int&,
            sal_Int32* pGlyphAdvAry = 0, int* pCharPosAry = 0 ) const;

    // used by glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos );
    virtual void    DropGlyph( int nStart );
    virtual void    Simplify( bool bIsBase );

    // Dummy implementation so layout can be shared between Linux/Windows
    virtual void    DrawText(SalGraphics&) const {};

    virtual ~GraphiteLayout() throw();
    void SetFeatures(grutils::GrFeatureParser * aFeature) { mpFeatures = aFeature; }
    void SetFontScale(float s) { mfScaling = s; };
    const TextSourceAdaptor * textSrc() const { return mpTextSrc; };
    virtual sal_GlyphId getKashidaGlyph(int & width) = 0;
    void kashidaJustify(std::vector<int> & rDeltaWidth, sal_GlyphId, int width);

    static const int EXTRA_CONTEXT_LENGTH;
private:
    int                   glyph_to_char(Glyphs::iterator);
    std::pair<int,int>    glyph_to_chars(const GlyphItem &) const;

    std::pair<long,long>  caret_positions(size_t) const;
    void expandOrCondense(ImplLayoutArgs &rArgs);
};

#endif // _SV_GRAPHITELAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
