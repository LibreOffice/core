/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include <preextstl.h>
#include <graphite/GrClient.h>
#include <graphite/Font.h>
#include <graphite/GrConstants.h>
#include <graphite/GrAppData.h>
#include <graphite/SegmentAux.h>
#include <postextstl.h>
// Platform
#include <sallayout.hxx>
#include <vcl/dllapi.h>
// Module

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
