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

// Description: An implementation of the SalLayout interface that uses the
//              Graphite engine.

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

// Enable lots of debug info
#ifdef DEBUG
//#define GRLAYOUT_DEBUG 1
//#undef NDEBUG
#endif

// Header files
//
// Standard Library
#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <numeric>
#include <deque>

// Platform
#ifdef WNT
#include <tools/svwin.h>
#include <svsys.h>
#endif

#ifdef UNX
#include <vcl/graphite_adaptors.hxx>
#endif

#include <vcl/salgdi.hxx>

#include <unicode/uchar.h>
#include <unicode/ubidi.h>
#include <unicode/uscript.h>

// Graphite Libraries (must be after vcl headers on windows)
#include <graphite/GrClient.h>
#include <graphite/Font.h>
#include <graphite/ITextSource.h>
#include <graphite/Segment.h>
#include <graphite/SegmentPainter.h>

#include <vcl/graphite_layout.hxx>
#include <vcl/graphite_features.hxx>
#include "graphite_textsrc.hxx"


// Module private type definitions and forward declarations.
//
// Module private names.
//

#ifdef GRLAYOUT_DEBUG
FILE * grLogFile = NULL;
FILE * grLog()
{
#ifdef WNT
    std::string logFileName(getenv("TEMP"));
    logFileName.append("\\graphitelayout.log");
    if (grLogFile == NULL) grLogFile = fopen(logFileName.c_str(),"w");
    else fflush(grLogFile);
    return grLogFile;
#else
    return stdout;
#endif
}
#endif

#ifdef GRCACHE
#include <vcl/graphite_cache.hxx>
#endif


namespace
{
    typedef std::pair<gr::GlyphIterator, gr::GlyphIterator>       glyph_range_t;
    typedef std::pair<gr::GlyphSetIterator, gr::GlyphSetIterator> glyph_set_range_t;

    inline long round(const float n) {
        return long(n + (n < 0 ? -0.5 : 0.5));
    }


    template<typename T>
    inline bool in_range(const T i, const T b, const T e) {
        return !(b > i) && i < e;
    }


    template<typename T>
    inline bool is_subrange(const T sb, const T se, const T b, const T e) {
        return !(b > sb || se > e);
    }


    template<typename T>
    inline bool is_subrange(const std::pair<T, T> &s, const T b, const T e) {
        return is_subrange(s.first, s.second, b, e);
    }

    int findSameDirLimit(const xub_Unicode* buffer, int charCount, bool rtl)
    {
        UErrorCode status = U_ZERO_ERROR;
        UBiDi *ubidi = ubidi_openSized(charCount, 0, &status);
        int limit = 0;
        ubidi_setPara(ubidi, reinterpret_cast<const UChar *>(buffer), charCount,
            (rtl)?UBIDI_DEFAULT_RTL:UBIDI_DEFAULT_LTR, NULL, &status);
        UBiDiLevel level = 0;
        ubidi_getLogicalRun(ubidi, 0, &limit, &level);
        ubidi_close(ubidi);
        if ((rtl && !(level & 1)) || (!rtl && (level & 1)))
        {
            limit = 0;
        }
        return limit;
    }

} // namespace



// Impementation of the GraphiteLayout::Glyphs container class.
//    This is an extended vector class with methods added to enable
//        o Correctly filling with glyphs.
//        o Querying clustering relationships.
//        o manipulations that affect neighouring glyphs.

const int GraphiteLayout::EXTRA_CONTEXT_LENGTH = 10;
#ifdef GRCACHE
GraphiteCacheHandler GraphiteCacheHandler::instance;
#endif

// The Graphite glyph stream is really a sequence of glyph attachment trees
//  each rooted at a non-attached base glyph.  fill_from walks the glyph stream
//  find each non-attached base glyph and calls append to record them as a
//  sequence of clusters.
void
GraphiteLayout::Glyphs::fill_from(gr::Segment & rSegment, ImplLayoutArgs &rArgs,
    bool bRtl, long &rWidth, float fScaling, std::vector<int> & rChar2Base, std::vector<int> & rGlyph2Char, std::vector<int> & rCharDxs)
{
    // Create a glyph item for each of the glyph and append it to the base class glyph list.
    typedef std::pair< gr::GlyphSetIterator, gr::GlyphSetIterator > GrGlyphSet;
    int nChar = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
    glyph_range_t iGlyphs = rSegment.glyphs();
    int nGlyphs = iGlyphs.second - iGlyphs.first;
    float fSegmentAdvance = rSegment.advanceWidth();
    float fMinX = fSegmentAdvance;
    float fMaxX = 0.0f;
    rGlyph2Char.assign(nGlyphs, -1);
    long nDxOffset = 0;
    int nGlyphIndex = (bRtl)? (nGlyphs - 1) : 0;
    // OOo always expects the glyphs in ltr order
    int nDelta = (bRtl)? -1 : 1;

    int nLastGlyph = (bRtl)? nGlyphs - 1: 0;
    int nNextChar = (bRtl)? (rSegment.stopCharacter() - 1) : rSegment.startCharacter();//rArgs.mnMinCharPos;
    // current glyph number (Graphite glyphs)
    //int currGlyph = 0;
    int nFirstCharInCluster = nNextChar;
    int nFirstGlyphInCluster = nLastGlyph;

    // ltr first char in cluster is lowest, same is true for rtl
    // ltr first glyph in cluster is lowest, rtl first glyph is highest

    // loop over the glyphs determining which characters are linked to them
    gr::GlyphIterator gi;
    for (gi = iGlyphs.first + nGlyphIndex;
         nGlyphIndex >= 0 && nGlyphIndex < nGlyphs;
         nGlyphIndex+= nDelta, gi = iGlyphs.first + nGlyphIndex)
    {
        gr::GlyphInfo info = (*gi);
#ifdef GRLAYOUT_DEBUG
        fprintf(grLog(),"Glyph %d %f,%f\n", (int)info.logicalIndex(), info.origin(), info.yOffset());
#endif
        // the last character associated with this glyph is after
        // our current cluster buffer position
        if ((bRtl && ((signed)info.firstChar() <= nNextChar)) ||
            (!bRtl && ((signed)info.lastChar() >= nNextChar)))
        {
            if ((bRtl && nGlyphIndex < nLastGlyph) ||
                (!bRtl && nGlyphIndex > nLastGlyph))
            {
                // this glyph is after the previous one left->right
                // if insertion is allowed before it then we are in a
                // new cluster
                int nAttachedBase = (*(info.attachedClusterBase())).logicalIndex();
                if (!info.isAttached() ||
                    !in_range(nAttachedBase, nFirstGlyphInCluster, nGlyphIndex))
                {
                    if (in_range(nFirstCharInCluster, rArgs.mnMinCharPos, rArgs.mnEndCharPos) &&
                        nFirstGlyphInCluster != nGlyphIndex)
                    {
                        std::pair <float,float> aBounds =
                            appendCluster(rSegment, rArgs, bRtl,
                            fSegmentAdvance, nFirstCharInCluster,
                            nNextChar, nFirstGlyphInCluster, nGlyphIndex, fScaling,
                            rChar2Base, rGlyph2Char, rCharDxs, nDxOffset);
                        fMinX = std::min(aBounds.first, fMinX);
                        fMaxX = std::max(aBounds.second, fMaxX);
                    }
                    nFirstCharInCluster = (bRtl)? info.lastChar() : info.firstChar();
                    nFirstGlyphInCluster = nGlyphIndex;
                }
                nLastGlyph = (bRtl)? std::min(nGlyphIndex, nAttachedBase) :
                    std::max(nGlyphIndex, nAttachedBase);
            }
            // loop over chacters associated with this glyph and characters
            // between nextChar and the last character associated with this glyph
            // giving them the current cluster id.  This allows for character /glyph
            // order reversal.
            // For each character we do a reverse glyph id look up
            // and store the glyph id with the highest logical index in nLastGlyph
            while ((bRtl && ((signed)info.firstChar() <= nNextChar)) ||
                   (!bRtl && (signed)info.lastChar() >= nNextChar))
            {
                GrGlyphSet charGlyphs = rSegment.charToGlyphs(nNextChar);
                nNextChar += nDelta;
                gr::GlyphSetIterator gj = charGlyphs.first;
                while (gj != charGlyphs.second)
                {
                    nLastGlyph = (bRtl)? min(nLastGlyph, (signed)(*gj).logicalIndex()) : max(nLastGlyph, (signed)(*gj).logicalIndex());
                    ++gj;
                }
            }
            // Loop over attached glyphs and make sure they are all in the cluster since you
            // can have glyphs attached with another base glyph in between
            glyph_set_range_t iAttached = info.attachedClusterGlyphs();
            for (gr::GlyphSetIterator agi = iAttached.first; agi != iAttached.second; ++agi)
            {
                nLastGlyph = (bRtl)? min(nLastGlyph, (signed)(*agi).logicalIndex()) : max(nLastGlyph, (signed)(*agi).logicalIndex());
            }

            // if this is a rtl attached glyph, then we need to include its
            // base in the cluster, which will have a lower graphite index
            if (bRtl)
            {
                if ((signed)info.attachedClusterBase()->logicalIndex() < nLastGlyph)
                {
                    nLastGlyph = info.attachedClusterBase()->logicalIndex();
                }
            }
        }

        // it is possible for the lastChar to be after nextChar and
        // firstChar to be before the nFirstCharInCluster in rare
        // circumstances e.g. Myanmar word for cemetery
        if ((bRtl && ((signed)info.lastChar() > nFirstCharInCluster)) ||
            (!bRtl && ((signed)info.firstChar() < nFirstCharInCluster)))
        {
            nFirstCharInCluster = info.firstChar();
        }
    }
    // process last cluster
    if (in_range(nFirstCharInCluster, rArgs.mnMinCharPos, rArgs.mnEndCharPos) &&
        nFirstGlyphInCluster != nGlyphIndex)
    {
        std::pair <float,float> aBounds =
            appendCluster(rSegment, rArgs, bRtl, fSegmentAdvance,
                          nFirstCharInCluster, nNextChar,
                          nFirstGlyphInCluster, nGlyphIndex, fScaling,
                          rChar2Base, rGlyph2Char, rCharDxs, nDxOffset);
        fMinX = std::min(aBounds.first, fMinX);
        fMaxX = std::max(aBounds.second, fMaxX);
    }
    long nXOffset = round(fMinX * fScaling);
    rWidth = round(fMaxX * fScaling) - nXOffset + nDxOffset;
    if (rWidth < 0)
    {
        // This can happen when there was no base inside the range
        rWidth = 0;
    }
    // fill up non-base char dx with cluster widths from previous base glyph
    if (bRtl)
    {
        if (rCharDxs[nChar-1] == -1)
            rCharDxs[nChar-1] = 0;
        else
            rCharDxs[nChar-1] -= nXOffset;
        for (int i = nChar - 2; i >= 0; i--)
        {
            if (rCharDxs[i] == -1) rCharDxs[i] = rCharDxs[i+1];
            else rCharDxs[i] -= nXOffset;
        }
    }
    else
    {
        if (rCharDxs[0] == -1)
            rCharDxs[0] = 0;
        else
            rCharDxs[0] -= nXOffset;
        for (int i = 1; i < nChar; i++)
        {
            if (rCharDxs[i] == -1) rCharDxs[i] = rCharDxs[i-1];
            else rCharDxs[i] -= nXOffset;
        }
    }
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"Glyphs xOff%ld dropDx%ld w%ld\n", nXOffset, nDxOffset, rWidth);
#endif
    // remove offset due to context if there is one
    if (nXOffset != 0)
    {
        for (size_t i = 0; i < size(); i++)
            (*this)[i].maLinearPos.X() -= nXOffset;
    }
}

std::pair<float,float> GraphiteLayout::Glyphs::appendCluster(gr::Segment& rSeg,
    ImplLayoutArgs & rArgs, bool bRtl,float fSegmentAdvance,
    int nFirstCharInCluster, int nNextChar, int nFirstGlyphInCluster,
    int nNextGlyph, float fScaling, std::vector<int> & rChar2Base,
    std::vector<int> & rGlyph2Char, std::vector<int> & rCharDxs, long & rDXOffset)
{
    glyph_range_t iGlyphs = rSeg.glyphs();
    int nGlyphs = iGlyphs.second - iGlyphs.first;
    int nDelta = (bRtl)? -1 : 1;
    gr::GlyphInfo aFirstGlyph = *(iGlyphs.first + nFirstGlyphInCluster);
    std::pair <float, float> aBounds;
    aBounds.first = aFirstGlyph.origin();
    aBounds.second = aFirstGlyph.origin();
    // before we add the glyphs to this vector, we record the
    // glyph's index in the vector (which is not the same as
    // the Segment's glyph index!)
    assert(size() < rGlyph2Char.size());
    rChar2Base[nFirstCharInCluster-rArgs.mnMinCharPos] = size();
    rGlyph2Char[size()] = nFirstCharInCluster;

    // can we break before this cluster?
    // Glyphs may have either a positive or negative breakWeight refering to
    // the position after or before the glyph respectively
    int nPrevBreakWeight = 0;
    if (nFirstGlyphInCluster > 0)
    {
        nPrevBreakWeight = (iGlyphs.first + (nFirstGlyphInCluster - 1))->breakweight();
    }
    int nBreakWeight = aFirstGlyph.breakweight();
    if (nBreakWeight < 0)
    {
        // negative means it applies to the position before the glyph's character
        nBreakWeight *= -1;
        if (nPrevBreakWeight > 0 && nPrevBreakWeight < nBreakWeight)
        {
            // prevBreakWeight wins
            nBreakWeight = nPrevBreakWeight;
        }
    }
    else
    {
        nBreakWeight = 0;
        // positive means break after
        if (nPrevBreakWeight > 0)
            nBreakWeight = nPrevBreakWeight;
    }
    if (nBreakWeight > gr::klbNoBreak/*0*/ &&
        // nBreakWeight <= gr::klbHyphenBreak) // uses Graphite hyphenation
        nBreakWeight <= gr::klbLetterBreak) // Needed for issue 111272
    {
        if (nBreakWeight < gr::klbHyphenBreak)
            rChar2Base[nFirstCharInCluster-rArgs.mnMinCharPos] |= WORD_BREAK_BEFORE;
        else
            rChar2Base[nFirstCharInCluster-rArgs.mnMinCharPos] |= HYPHEN_BREAK_BEFORE;
    }
    // always allow a break before a space even if graphite doesn't
    if (rArgs.mpStr[nFirstCharInCluster] == 0x20)
        rChar2Base[nFirstCharInCluster-rArgs.mnMinCharPos] |= WORD_BREAK_BEFORE;

    bool bBaseGlyph = true;
    for (int j = nFirstGlyphInCluster;
        j != nNextGlyph; j += nDelta)
    {
        long nNextOrigin;
        float fNextOrigin;
        gr::GlyphInfo aGlyph = *(iGlyphs.first + j);
        if (j + nDelta >= nGlyphs || j + nDelta < 0) // at rhs ltr,rtl
        {
            fNextOrigin = fSegmentAdvance;
            nNextOrigin = round(fSegmentAdvance * fScaling + rDXOffset);
            aBounds.second = std::max(fSegmentAdvance, aBounds.second);
        }
        else
        {
            gr::GlyphInfo aNextGlyph = *(iGlyphs.first + j + nDelta);
            fNextOrigin = std::max(aNextGlyph.attachedClusterBase()->origin(), aNextGlyph.origin());
            aBounds.second = std::max(fNextOrigin, aBounds.second);
            nNextOrigin = round(fNextOrigin * fScaling + rDXOffset);
        }
        aBounds.first = std::min(aGlyph.origin(), aBounds.first);
        if ((signed)aGlyph.firstChar() < rArgs.mnEndCharPos &&
            (signed)aGlyph.firstChar() >= rArgs.mnMinCharPos)
        {
            rCharDxs[aGlyph.firstChar()-rArgs.mnMinCharPos] = nNextOrigin;
        }
        if ((signed)aGlyph.attachedClusterBase()->logicalIndex() == j)
        {
            append(rSeg, rArgs, aGlyph, fNextOrigin, fScaling, rChar2Base, rGlyph2Char, rCharDxs, rDXOffset, bBaseGlyph);
            bBaseGlyph = false;
        }
    }
    // from the point of view of the dx array, the xpos is
    // the origin of the first glyph of the next cluster ltr
    // rtl it is the origin of the 1st glyph of the cluster
    long nXPos = (bRtl)?
        round(aFirstGlyph.attachedClusterBase()->origin() * fScaling) + rDXOffset :
        round(aBounds.second * fScaling) + rDXOffset;
    // force the last char in range to have the width of the cluster
    if (bRtl)
    {
        for (int n = nNextChar + 1; n <= nFirstCharInCluster; n++)
        {
            if ((n < rArgs.mnEndCharPos) && (n >= rArgs.mnMinCharPos))
                rCharDxs[n-rArgs.mnMinCharPos] = nXPos;
        }
    }
    else
    {
        for (int n = nNextChar - 1; n >= nFirstCharInCluster; n--)
        {
            if (n < rArgs.mnEndCharPos && n >= rArgs.mnMinCharPos)
                rCharDxs[n-rArgs.mnMinCharPos] = nXPos;
        }
    }
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"Cluster g[%d-%d) c[%d-%d)%x x%ld y%f bw%d\n", nFirstGlyphInCluster, nNextGlyph, nFirstCharInCluster, nNextChar, rArgs.mpStr[nFirstCharInCluster], nXPos, aFirstGlyph.yOffset(), nBreakWeight);
#endif
    return aBounds;
}

// append walks an attachment tree, flattening it, and converting it into a
// sequence of GlyphItem objects which we can later manipulate.
void
GraphiteLayout::Glyphs::append(gr::Segment &segment, ImplLayoutArgs &args, gr::GlyphInfo & gi, float nextGlyphOrigin, float scaling, std::vector<int> & rChar2Base, std::vector<int> & rGlyph2Char, std::vector<int> & rCharDxs, long & rDXOffset, bool bIsBase)
{
    float nextOrigin = nextGlyphOrigin;
    int firstChar = std::min(gi.firstChar(), gi.lastChar());
    assert(size() < rGlyph2Char.size());
    if (!bIsBase) rGlyph2Char[size()] = firstChar;
    // is the next glyph attached or in the next cluster?
    glyph_set_range_t iAttached = gi.attachedClusterGlyphs();
    if (iAttached.first != iAttached.second)
    {
        nextOrigin = iAttached.first->origin();
    }
    long glyphId = gi.glyphID();
    long deltaOffset = 0;
    int glyphWidth = round(nextOrigin * scaling) - round(gi.origin() * scaling);
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"c%d g%d gWidth%d x%f ", firstChar, (int)gi.logicalIndex(), glyphWidth, nextOrigin);
#endif
    if (glyphId == 0)
    {
        args.NeedFallback(
            firstChar,
            gr::RightToLeftDir(gr::DirCode(gi.directionality())));
        if( (SAL_LAYOUT_FOR_FALLBACK & args.mnFlags ))
        {
            glyphId = GF_DROPPED;
            deltaOffset -= glyphWidth;
            glyphWidth = 0;
        }
    }
    else if(args.mnFlags & SAL_LAYOUT_FOR_FALLBACK)
    {
#ifdef GRLAYOUT_DEBUG
        fprintf(grLog(),"fallback c%d %x in run %d\n", firstChar, args.mpStr[firstChar],
            args.maRuns.PosIsInAnyRun(firstChar));
#endif
        // glyphs that aren't requested for fallback will be taken from base
        // layout, so mark them as dropped (should this wait until Simplify(false) is called?)
        if (!args.maRuns.PosIsInAnyRun(firstChar) &&
            in_range(firstChar, args.mnMinCharPos, args.mnEndCharPos))
        {
            glyphId = GF_DROPPED;
            deltaOffset -= glyphWidth;
            glyphWidth = 0;
        }
    }
    // append this glyph.
    long nGlyphFlags = bIsBase ? 0 : GlyphItem::IS_IN_CLUSTER;
    // directionality seems to be unreliable
    //nGlyphFlags |= gr::RightToLeftDir(gr::DirCode(gi.attachedClusterBase()->directionality())) ? GlyphItem::IS_RTL_GLYPH : 0;
    nGlyphFlags |= (gi.directionLevel() & 0x1)? GlyphItem::IS_RTL_GLYPH : 0;
    GlyphItem aGlyphItem(size(),//gi.logicalIndex(),
        glyphId,
        Point(round(gi.origin() * scaling + rDXOffset),
            round((-gi.yOffset() * scaling) - segment.AscentOffset()* scaling)),
        nGlyphFlags,
        glyphWidth);
    aGlyphItem.mnOrigWidth = round(gi.advanceWidth() * scaling);
    push_back(aGlyphItem);

    // update the offset if this glyph was dropped
    rDXOffset += deltaOffset;

    // Recursively apply append all the attached glyphs.
    for (gr::GlyphSetIterator agi = iAttached.first; agi != iAttached.second; ++agi)
    {
        if (agi + 1 == iAttached.second)
            append(segment, args, *agi, nextGlyphOrigin, scaling, rChar2Base, rGlyph2Char,rCharDxs, rDXOffset, false);
        else
            append(segment, args, *agi, (agi + 1)->origin(), scaling, rChar2Base, rGlyph2Char, rCharDxs, rDXOffset, false);
    }
}

//
// An implementation of the SalLayout interface to enable Graphite enabled fonts to be used.
//
GraphiteLayout::GraphiteLayout(const gr::Font & font, const grutils::GrFeatureParser * pFeatures) throw()
  : mpTextSrc(0),
    mrFont(font),
    mnWidth(0),
    mfScaling(1.0),
    mpFeatures(pFeatures)
{
    // Line settings can have subtle affects on space handling
    // since we don't really know whether it is the end of a line or just a run
    // in the middle, it is hard to know what to set them to.
    // If true, it can cause end of line spaces to be hidden e.g. Doulos SIL
    maLayout.setStartOfLine(false);
    maLayout.setEndOfLine(false);
    maLayout.setDumbFallback(true);
    // trailing ws doesn't seem to always take affect if end of line is true
    maLayout.setTrailingWs(gr::ktwshAll);
#ifdef GRLAYOUT_DEBUG
    gr::ScriptDirCode aDirCode = font.getSupportedScriptDirections();
    fprintf(grLog(),"GraphiteLayout scripts %x %lx\n", aDirCode, long(this));
#endif
}


GraphiteLayout::~GraphiteLayout() throw()
{
    clear();
    // the features are owned by the platform layers
    mpFeatures = NULL;
}

void GraphiteLayout::clear()
{
    // Destroy the segment and text source from any previous invocation of
    // LayoutText
    mvGlyphs.clear();
    mvCharDxs.clear();
    mvChar2BaseGlyph.clear();
    mvGlyph2Char.clear();

#ifndef GRCACHE
    delete mpTextSrc;
#endif

    // Reset the state to the empty state.
    mpTextSrc=0;
    mnWidth = 0;
    // Don't reset the scaling, because it is set before LayoutText
}

// This method shouldn't be called on windows, since it needs the dc reset
bool GraphiteLayout::LayoutText(ImplLayoutArgs & rArgs)
{
#ifdef GRCACHE
    GrSegRecord * pSegRecord = NULL;
    gr::Segment * pSegment = NULL;
    // Graphite can in rare cases crash with a zero length
    if (rArgs.mnMinCharPos < rArgs.mnEndCharPos)
    {
        pSegment = CreateSegment(rArgs, &pSegRecord);
        if (!pSegment)
            return false;
    }
    else
    {
        clear();
        return true;
    }
    // layout the glyphs as required by OpenOffice
    bool success = LayoutGlyphs(rArgs, pSegment, pSegRecord);

    if (pSegRecord) pSegRecord->unlock();
    else delete pSegment;
#else
    gr::Segment * pSegment = NULL;
    bool success = true;
    if (rArgs.mnMinCharPos < rArgs.mnEndCharPos)
    {
        pSegment = CreateSegment(rArgs);
        if (!pSegment)
            return false;
        success = LayoutGlyphs(rArgs, pSegment);
        if (pSegment) delete pSegment;
    }
    else
    {
        clear();
    }
#endif
    return success;
}

#ifdef GRCACHE
class GrFontHasher : public gr::Font
{
public:
    GrFontHasher(const gr::Font & aFont) : gr::Font(aFont), mrRealFont(const_cast<gr::Font&>(aFont)) {};
    ~GrFontHasher(){};
    virtual bool bold() { return mrRealFont.bold(); };
    virtual bool italic() { return mrRealFont.italic(); };
    virtual float ascent()  { return mrRealFont.ascent(); };
    virtual float descent()  { return mrRealFont.descent(); };
    virtual float height()  { return mrRealFont.height(); };
    virtual gr::Font* copyThis() { return mrRealFont.copyThis(); };
    virtual unsigned int getDPIx() { return mrRealFont.getDPIx(); };
    virtual unsigned int getDPIy() { return mrRealFont.getDPIy(); };
    virtual const void* getTable(gr::fontTableId32 nId, size_t* nSize)
    { return mrRealFont.getTable(nId,nSize); }
    virtual void getFontMetrics(float*pA, float*pB, float*pC) { mrRealFont.getFontMetrics(pA,pB,pC); };

    sal_Int32 hashCode(const grutils::GrFeatureParser * mpFeatures)
    {
        // is this sufficient?
        std::wstring aFace;
        bool bBold;
        bool bItalic;
        UniqueCacheInfo(aFace, bBold, bItalic);
        sal_Unicode uName[32]; // max length used in gr::Font
        // Note: graphite stores font names as UTF-16 even if wchar_t is 32bit
        // this conversion should be OK.
        for (size_t i = 0; i < aFace.size() && i < 32; i++)
        {
            uName[i] = aFace[i];
        }
        size_t iSize = aFace.size();
        if (0 == iSize) return 0;
        sal_Int32 hash = rtl_ustr_hashCode_WithLength(uName, iSize);
        hash ^= static_cast<sal_Int32>(height());
        hash |= (bBold)? 0x1000000 : 0;
        hash |= (bItalic)? 0x2000000 : 0;
        if (mpFeatures)
            hash ^= mpFeatures->hashCode();
#ifdef GRLAYOUT_DEBUG
        fprintf(grLog(), "font hash %x size %f\n", (int)hash, height());
#endif
        return hash;
    };
protected:
    virtual void UniqueCacheInfo( std::wstring& stuFace, bool& fBold, bool& fItalic )
    {
#ifdef WIN32
        dynamic_cast<GraphiteWinFont&>(mrRealFont).UniqueCacheInfo(stuFace, fBold, fItalic);
#else
#ifdef UNX
        dynamic_cast<GraphiteFontAdaptor&>(mrRealFont).UniqueCacheInfo(stuFace, fBold, fItalic);
#else
#error Unknown base type for gr::Font::UniqueCacheInfo
#endif
#endif
    }
private:
    gr::Font & mrRealFont;
};
#endif

#ifdef GRCACHE
gr::Segment * GraphiteLayout::CreateSegment(ImplLayoutArgs& rArgs, GrSegRecord ** pSegRecord)
#else
gr::Segment * GraphiteLayout::CreateSegment(ImplLayoutArgs& rArgs)
#endif
{
    assert(rArgs.mnLength >= 0);

    gr::Segment * pSegment = NULL;

    // Set the SalLayouts values to be the inital ones.
    SalLayout::AdjustLayout(rArgs);
    // TODO check if this is needed
    if (mnUnitsPerPixel > 1)
        mfScaling = 1.0f / mnUnitsPerPixel;

    // Clear out any previous buffers
    clear();
    bool bRtl = mnLayoutFlags & SAL_LAYOUT_BIDI_RTL;
    try
    {
        // Don't set RTL if font doesn't support it otherwise it forces rtl on
        // everything
        if (bRtl && (mrFont.getSupportedScriptDirections() & gr::kfsdcHorizRtl))
            maLayout.setRightToLeft(bRtl);

        // Context is often needed beyond the specified end, however, we don't
        // want it if there has been a direction change, since it is hard
        // to tell between reordering within one direction and multi-directional
        // text. Extra context, can also cause problems with ligatures stradling
        // a hyphenation point, so disable if CTL is disabled.
        const int  nSegCharLimit = min(rArgs.mnLength, mnEndCharPos + EXTRA_CONTEXT_LENGTH);
        int limit = rArgs.mnEndCharPos;
        if ((nSegCharLimit > limit) && !(SAL_LAYOUT_COMPLEX_DISABLED & rArgs.mnFlags))
        {
            limit += findSameDirLimit(rArgs.mpStr + rArgs.mnEndCharPos,
                nSegCharLimit - rArgs.mnEndCharPos, bRtl);
        }

#ifdef GRCACHE
        GrFontHasher hasher(mrFont);
        sal_Int32 aFontHash = hasher.hashCode(mpFeatures);
        GraphiteSegmentCache * pCache =
            (GraphiteCacheHandler::instance).getCache(aFontHash);
        if (pCache)
        {
            *pSegRecord = pCache->getSegment(rArgs, bRtl, limit);
            if (*pSegRecord)
            {
                pSegment = (*pSegRecord)->getSegment();
                mpTextSrc = (*pSegRecord)->getTextSrc();
                maLayout.setRightToLeft((*pSegRecord)->isRtl());
                if (rArgs.mpStr != mpTextSrc->getLayoutArgs().mpStr ||
                    rArgs.mnMinCharPos != mpTextSrc->getLayoutArgs().mnMinCharPos ||
                    rArgs.mnEndCharPos != mpTextSrc->getLayoutArgs().mnEndCharPos ||
                    (SAL_LAYOUT_FOR_FALLBACK & rArgs.mnFlags) )
                {
                    (*pSegRecord)->clearVectors();
                }
                mpTextSrc->switchLayoutArgs(rArgs);
                if (limit > rArgs.mnMinCharPos && limit == rArgs.mnEndCharPos
                    && pSegment->stopCharacter() != limit)
                {
                    // check that the last character is not part of a ligature
                    glyph_set_range_t aGlyphSet = pSegment->charToGlyphs(limit - 1);
                    if (aGlyphSet.first == aGlyphSet.second)
                    {
                        // no glyphs associated with this glyph - occurs mid ligature
                        pSegment = NULL;
                        *pSegRecord = NULL;
                    }
                    else
                    {
                        while (aGlyphSet.first != aGlyphSet.second)
                        {
                            int lastChar = static_cast<int>((*aGlyphSet.first).lastChar());
                            if (lastChar >= limit)
                            {
                                pSegment = NULL;
                                *pSegRecord = NULL;
                                break;
                            }
                            aGlyphSet.first++;
                        }
                    }
                }
                if (pSegment)
                    return pSegment;
            }
        }
#endif

        // Create a new TextSource object for the engine.
        mpTextSrc = new TextSourceAdaptor(rArgs, limit);
        if (mpFeatures) mpTextSrc->setFeatures(mpFeatures);

        pSegment = new gr::RangeSegment((gr::Font *)&mrFont, mpTextSrc, &maLayout, mnMinCharPos, limit);
        if (pSegment != NULL)
        {
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"Gr::LayoutText %d-%d, context %d,len%d rtl%d/%d scaling %f\n", rArgs.mnMinCharPos,
               rArgs.mnEndCharPos, limit, rArgs.mnLength, maLayout.rightToLeft(), pSegment->rightToLeft(), mfScaling);
#endif
#ifdef GRCACHE
            // on a new segment rightToLeft should be correct
            *pSegRecord = pCache->cacheSegment(mpTextSrc, pSegment, pSegment->rightToLeft());
#endif
        }
        else
        {
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(), "Gr::LayoutText failed: ");
            for (int i = mnMinCharPos; i < limit; i++)
            {
                fprintf(grLog(), "%04x ", rArgs.mpStr[i]);
            }
            fprintf(grLog(), "\n");
#endif
            clear();
            return NULL;
        }
    }
    catch (...)
    {
        clear();  // destroy the text source and any partially built segments.
        return NULL;
    }
    return pSegment;
}

#ifdef GRCACHE
bool GraphiteLayout::LayoutGlyphs(ImplLayoutArgs& rArgs, gr::Segment * pSegment, GrSegRecord * pSegRecord)
#else
bool GraphiteLayout::LayoutGlyphs(ImplLayoutArgs& rArgs, gr::Segment * pSegment)
#endif
{
#ifdef GRCACHE
#ifdef GRCACHE_REUSE_VECTORS
    // if we have an exact match, then we can reuse the glyph vectors from before
    if (pSegRecord && (pSegRecord->glyphs().size() > 0) &&
        (pSegRecord->fontScale() == mfScaling) &&
        !(SAL_LAYOUT_FOR_FALLBACK & rArgs.mnFlags) )
    {
        mnWidth = pSegRecord->width();
        mvGlyphs = pSegRecord->glyphs();
        mvCharDxs = pSegRecord->charDxs();
        mvChar2BaseGlyph = pSegRecord->char2BaseGlyph();
        mvGlyph2Char = pSegRecord->glyph2Char();
        return true;
    }
#endif
#endif
    // Calculate the initial character dxs.
    mvCharDxs.assign(mnEndCharPos - mnMinCharPos, -1);
    mvChar2BaseGlyph.assign(mnEndCharPos - mnMinCharPos, -1);
    mnWidth = 0;
    if (mvCharDxs.size() > 0)
    {
        // Discover all the clusters.
        try
        {
            // Note: we use the layout rightToLeft() because in cached segments
            // rightToLeft() may no longer be valid if the engine has been run
            // ltr since the segment was created.
#ifdef GRCACHE
            bool bRtl = pSegRecord? pSegRecord->isRtl() : pSegment->rightToLeft();
#else
            bool bRtl = pSegment->rightToLeft();
#endif
            mvGlyphs.fill_from(*pSegment, rArgs, bRtl,
                mnWidth, mfScaling, mvChar2BaseGlyph, mvGlyph2Char, mvCharDxs);

            if (bRtl)
            {
                // not needed for adjacent differences, but for mouse clicks to char
                std::transform(mvCharDxs.begin(), mvCharDxs.end(), mvCharDxs.begin(),
                    std::bind1st(std::minus<long>(), mnWidth));
                // fixup last dx to ensure it always equals the width
                mvCharDxs[mvCharDxs.size() - 1] = mnWidth;
            }
#ifdef GRCACHE
#ifdef GRCACHE_REUSE_VECTORS
            if (pSegRecord && rArgs.maReruns.IsEmpty() &&
                !(SAL_LAYOUT_FOR_FALLBACK & rArgs.mnFlags))
            {
                pSegRecord->setGlyphVectors(mnWidth, mvGlyphs, mvCharDxs,
                                            mvChar2BaseGlyph, mvGlyph2Char,
                                            mfScaling);
            }
#endif
#endif
        }
        catch (std::exception e)
        {
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"LayoutGlyphs failed %s\n", e.what());
#endif
            return false;
        }
        catch (...)
        {
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"LayoutGlyphs failed with exception");
#endif
            return false;
        }
    }
    else
    {
        mnWidth = 0;
    }
    return true;
}

int GraphiteLayout::GetTextBreak(long maxmnWidth, long char_extra, int factor) const
{
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"Gr::GetTextBreak c[%d-%d) maxWidth %ld char extra %ld factor %d\n",
        mnMinCharPos, mnEndCharPos, maxmnWidth, char_extra, factor);
#endif

    // return quickly if this segment is narrower than the target width
    if (maxmnWidth > mnWidth * factor + char_extra * (mnEndCharPos - mnMinCharPos - 1))
        return STRING_LEN;

    long nWidth = mvCharDxs[0] * factor;
    int nLastBreak = -1;
    for (size_t i = 1; i < mvCharDxs.size(); i++)
    {
        nWidth += char_extra;
        if (nWidth > maxmnWidth) break;
        if (mvChar2BaseGlyph[i] != -1)
        {
            if (mvChar2BaseGlyph[i] & (WORD_BREAK_BEFORE | HYPHEN_BREAK_BEFORE))
                nLastBreak = static_cast<int>(i);
        }
        nWidth += (mvCharDxs[i] - mvCharDxs[i-1]) * factor;
    }
    int nBreak = mnMinCharPos;
    if (nLastBreak > -1)
        nBreak += nLastBreak;

#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(), "Gr::GetTextBreak break after %d\n", nBreak - mnMinCharPos);
#endif

    if (nBreak > mnEndCharPos) nBreak = STRING_LEN;
    else if (nBreak < mnMinCharPos) nBreak = mnMinCharPos;
    return nBreak;
}


long GraphiteLayout::FillDXArray( sal_Int32* pDXArray ) const
{
    if (mnEndCharPos == mnMinCharPos)
        // Then we must be zero width!
        return 0;

    if (pDXArray)
    {
        for (size_t i = 0; i < mvCharDxs.size(); i++)
        {
            assert( (mvChar2BaseGlyph[i] == -1) ||
                ((signed)(mvChar2BaseGlyph[i] & GLYPH_INDEX_MASK) < (signed)mvGlyphs.size()));
            if (mvChar2BaseGlyph[i] != -1 &&
                mvGlyphs[mvChar2BaseGlyph[i] & GLYPH_INDEX_MASK].mnGlyphIndex == GF_DROPPED)
            {
                // when used in MultiSalLayout::GetTextBreak dropped glyphs
                // must have zero width
                pDXArray[i] = 0;
            }
            else
            {
                pDXArray[i] = mvCharDxs[i];
                if (i > 0) pDXArray[i] -= mvCharDxs[i-1];
            }
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"%d,%d,%d ", (int)i, (int)mvCharDxs[i], pDXArray[i]);
#endif
        }
        //std::adjacent_difference(mvCharDxs.begin(), mvCharDxs.end(), pDXArray);
        //for (size_t i = 0; i < mvCharDxs.size(); i++)
        //    fprintf(grLog(),"%d,%d,%d ", (int)i, (int)mvCharDxs[i], pDXArray[i]);
        //fprintf(grLog(),"FillDX %ld,%d\n", mnWidth, std::accumulate(pDXArray, pDXArray + mvCharDxs.size(), 0));
    }
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"FillDXArray %d-%d,%d=%ld\n", mnMinCharPos, mnEndCharPos, (int)mpTextSrc->getLength(), mnWidth);
#endif
    return mnWidth;
}


void  GraphiteLayout::AdjustLayout(ImplLayoutArgs& rArgs)
{
    SalLayout::AdjustLayout(rArgs);
    if(rArgs.mpDXArray)
    {
        std::vector<int> vDeltaWidths(mvGlyphs.size(), 0);
        ApplyDXArray(rArgs, vDeltaWidths);

        if( (mnLayoutFlags & SAL_LAYOUT_BIDI_RTL) &&
           !(rArgs.mnFlags & SAL_LAYOUT_FOR_FALLBACK) )
        {
            // check if this is a kashida script
            bool bKashidaScript = false;
            for (int i = rArgs.mnMinCharPos; i < rArgs.mnEndCharPos; i++)
            {
                UErrorCode aStatus = U_ZERO_ERROR;
                UScriptCode scriptCode = uscript_getScript(rArgs.mpStr[i], &aStatus);
                if (scriptCode == USCRIPT_ARABIC || scriptCode == USCRIPT_SYRIAC)
                {
                    bKashidaScript = true;
                    break;
                }
            }
            int nKashidaWidth = 0;
            int nKashidaIndex = getKashidaGlyph(nKashidaWidth);
            if( nKashidaIndex != 0 && bKashidaScript)
            {
                kashidaJustify( vDeltaWidths, nKashidaIndex, nKashidaWidth );
            }
        }
    }
    else if (rArgs.mnLayoutWidth > 0)
    {
#ifdef GRLAYOUT_DEBUG
        fprintf(grLog(), "AdjustLayout width %ld=>%ld\n", mnWidth, rArgs.mnLayoutWidth);
#endif
        expandOrCondense(rArgs);
    }
}

void GraphiteLayout::expandOrCondense(ImplLayoutArgs &rArgs)
{
    int nDeltaWidth = rArgs.mnLayoutWidth - mnWidth;
    if (nDeltaWidth > 0) // expand, just expand between clusters
    {
        int nClusterCount = 0;
        for (size_t j = 0; j < mvGlyphs.size(); j++)
        {
            if (mvGlyphs[j].IsClusterStart())
            {
                ++nClusterCount;
            }
        }
        if (nClusterCount > 1)
        {
            float fExtraPerCluster = static_cast<float>(nDeltaWidth) / static_cast<float>(nClusterCount - 1);
            int nCluster = 0;
            int nOffset = 0;
            for (size_t i = 0; i < mvGlyphs.size(); i++)
            {
                if (mvGlyphs[i].IsClusterStart())
                {
                    nOffset = FRound( fExtraPerCluster * nCluster );
                    size_t nCharIndex = mvGlyph2Char[i];
                    mvCharDxs[nCharIndex] += nOffset;
                    // adjust char dxs for rest of characters in cluster
                    while (++nCharIndex < mvGlyph2Char.size())
                    {
                        int nChar2Base = (mvChar2BaseGlyph[nCharIndex] == -1)? -1 : (int)(mvChar2BaseGlyph[nCharIndex] & GLYPH_INDEX_MASK);
                        if (nChar2Base == -1 || nChar2Base == static_cast<int>(i))
                            mvCharDxs[nCharIndex] += nOffset;
                    }
                    ++nCluster;
                }
                mvGlyphs[i].maLinearPos.X() += nOffset;
            }
        }
    }
    else // condense - apply a factor to all glyph positions
    {
        if (mvGlyphs.size() == 0) return;
        Glyphs::iterator iLastGlyph = mvGlyphs.begin() + (mvGlyphs.size() - 1);
        // position last glyph using original width
        float fXFactor = static_cast<float>(rArgs.mnLayoutWidth - iLastGlyph->mnOrigWidth) / static_cast<float>(iLastGlyph->maLinearPos.X());
#ifdef GRLAYOUT_DEBUG
        fprintf(grLog(), "Condense by factor %f\n", fXFactor);
#endif
        iLastGlyph->maLinearPos.X() = rArgs.mnLayoutWidth - iLastGlyph->mnOrigWidth;
        Glyphs::iterator iGlyph = mvGlyphs.begin();
        while (iGlyph != iLastGlyph)
        {
            iGlyph->maLinearPos.X() = FRound( fXFactor * iGlyph->maLinearPos.X() );
            ++iGlyph;
        }
        for (size_t i = 0; i < mvCharDxs.size(); i++)
        {
            mvCharDxs[i] = FRound( fXFactor * mvCharDxs[i] );
        }
    }
    mnWidth = rArgs.mnLayoutWidth;
}

void GraphiteLayout::ApplyDXArray(ImplLayoutArgs &args, std::vector<int> & rDeltaWidth)
{
    const size_t nChars = args.mnEndCharPos - args.mnMinCharPos;
    if (nChars == 0) return;

#ifdef GRLAYOUT_DEBUG
    for (size_t iDx = 0; iDx < mvCharDxs.size(); iDx++)
         fprintf(grLog(),"%d,%d,%d ", (int)iDx, (int)mvCharDxs[iDx], args.mpDXArray[iDx]);
    fprintf(grLog(),"ApplyDx\n");
#endif
    bool bRtl = mnLayoutFlags & SAL_LAYOUT_BIDI_RTL;
    int nXOffset = 0;
    if (bRtl)
    {
        nXOffset = args.mpDXArray[nChars - 1] - mvCharDxs[nChars - 1];
    }
    int nPrevClusterGlyph = (bRtl)? (signed)mvGlyphs.size() : -1;
    int nPrevClusterLastChar = -1;
    for (size_t i = 0; i < nChars; i++)
    {
        int nChar2Base = (mvChar2BaseGlyph[i] == -1)? -1 : (int)(mvChar2BaseGlyph[i] & GLYPH_INDEX_MASK);
        if ((nChar2Base > -1) && (nChar2Base != nPrevClusterGlyph))
        {
            assert((nChar2Base > -1) && (nChar2Base < (signed)mvGlyphs.size()));
            GlyphItem & gi = mvGlyphs[nChar2Base];
            if (!gi.IsClusterStart())
                continue;

            // find last glyph of this cluster
            size_t j = i + 1;
            int nLastChar = i;
            int nLastGlyph = nChar2Base;
            for (; j < nChars; j++)
            {
                int nChar2BaseJ = (mvChar2BaseGlyph[j] == -1)? -1 : (int)(mvChar2BaseGlyph[j] & GLYPH_INDEX_MASK);
                assert((nChar2BaseJ >= -1) && (nChar2BaseJ < (signed)mvGlyphs.size()));
                if (nChar2BaseJ != -1 && mvGlyphs[nChar2BaseJ].IsClusterStart())
                {
                    nLastGlyph = nChar2BaseJ + ((bRtl)? +1 : -1);
                    nLastChar = j - 1;
                    break;
                }
            }
            if (nLastGlyph < 0)
            {
                nLastGlyph = nChar2Base;
            }
            // Its harder to find the last glyph rtl, since the first of
            // cluster is still on the left so we need to search towards
            // the previous cluster to the right
            if (bRtl)
            {
                nLastGlyph = nChar2Base;
                while (nLastGlyph + 1 < (signed)mvGlyphs.size() &&
                       !mvGlyphs[nLastGlyph+1].IsClusterStart())
                {
                    ++nLastGlyph;
                }
            }
            if (j == nChars)
            {
                nLastChar = nChars - 1;
                if (!bRtl) nLastGlyph = mvGlyphs.size() - 1;
            }
            assert((nLastChar > -1) && (nLastChar < (signed)nChars));
            long nNewClusterWidth = args.mpDXArray[nLastChar];
            long nOrigClusterWidth = mvCharDxs[nLastChar];
            long nDGlyphOrigin = 0;
            if (nPrevClusterLastChar > - 1)
            {
                assert(nPrevClusterLastChar < (signed)nChars);
                nNewClusterWidth -= args.mpDXArray[nPrevClusterLastChar];
                nOrigClusterWidth -= mvCharDxs[nPrevClusterLastChar];
                nDGlyphOrigin = args.mpDXArray[nPrevClusterLastChar] - mvCharDxs[nPrevClusterLastChar];
            }
            long nDWidth = nNewClusterWidth - nOrigClusterWidth;
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(), "c%lu last glyph %d/%lu\n", i, nLastGlyph, mvGlyphs.size());
#endif
            assert((nLastGlyph > -1) && (nLastGlyph < (signed)mvGlyphs.size()));
            mvGlyphs[nLastGlyph].mnNewWidth += nDWidth;
            if (gi.mnGlyphIndex != GF_DROPPED)
                mvGlyphs[nLastGlyph].mnNewWidth += nDWidth;
            else
                nDGlyphOrigin += nDWidth;
            // update glyph positions
            if (bRtl)
            {
                for (int n = nChar2Base; n <= nLastGlyph; n++)
                {
                    assert((n > - 1) && (n < (signed)mvGlyphs.size()));
                    mvGlyphs[n].maLinearPos.X() += -nDGlyphOrigin + nXOffset;
                }
            }
            else
            {
                for (int n = nChar2Base; n <= nLastGlyph; n++)
                {
                    assert((n > - 1) && (n < (signed)mvGlyphs.size()));
                    mvGlyphs[n].maLinearPos.X() += nDGlyphOrigin + nXOffset;
                }
            }
            rDeltaWidth[nChar2Base] = nDWidth;
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"c%d g%d-%d dW%ld-%ld=%ld dX%ld x%ld\t", (int)i, nChar2Base, nLastGlyph, nNewClusterWidth, nOrigClusterWidth, nDWidth, nDGlyphOrigin, mvGlyphs[nChar2Base].maLinearPos.X());
#endif
            nPrevClusterGlyph = nChar2Base;
            nPrevClusterLastChar = nLastChar;
            i = nLastChar;
        }
    }
    // Update the dx vector with the new values.
    std::copy(args.mpDXArray, args.mpDXArray + nChars,
      mvCharDxs.begin() + (args.mnMinCharPos - mnMinCharPos));
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"ApplyDx %d(%ld)\n", args.mpDXArray[nChars - 1], mnWidth);
#endif
    mnWidth = args.mpDXArray[nChars - 1];
}

void GraphiteLayout::kashidaJustify(std::vector<int>& rDeltaWidths, sal_GlyphId nKashidaIndex, int nKashidaWidth)
{
    // skip if the kashida glyph in the font looks suspicious
    if( nKashidaWidth <= 0 )
        return;

    // calculate max number of needed kashidas
    Glyphs::iterator i = mvGlyphs.begin();
    int nKashidaCount = 0;
    int nOrigGlyphIndex = -1;
    int nGlyphIndex = -1;
    while (i != mvGlyphs.end())
    {
        nOrigGlyphIndex++;
        nGlyphIndex++;
        // only inject kashidas in RTL contexts
        if( !(*i).IsRTLGlyph() )
        {
            ++i;
            continue;
        }
        // no kashida-injection for blank justified expansion either
        if( IsSpacingGlyph( (*i).mnGlyphIndex ) )
        {
            ++i;
            continue;
        }
        // calculate gap, ignore if too small
        int nGapWidth = rDeltaWidths[nOrigGlyphIndex];
        // worst case is one kashida even for mini-gaps
        if( 3 * nGapWidth < nKashidaWidth )
        {
            ++i;
            continue;
        }
        nKashidaCount = 1 + (nGapWidth / nKashidaWidth);
#ifdef GRLAYOUT_DEBUG
        printf("inserting %d kashidas at %u\n", nKashidaCount, (*i).mnGlyphIndex);
#endif
        GlyphItem glyphItem = *i;
        Point aPos(0, 0);
        aPos.X() = (*i).maLinearPos.X();
        GlyphItem newGi(glyphItem.mnCharPos, nKashidaIndex, aPos,
                GlyphItem::IS_IN_CLUSTER|GlyphItem::IS_RTL_GLYPH, nKashidaWidth);
        mvGlyphs.reserve(mvGlyphs.size() + nKashidaCount);
        i = mvGlyphs.begin() + nGlyphIndex;
        mvGlyphs.insert(i, nKashidaCount, newGi);
        i = mvGlyphs.begin() + nGlyphIndex;
        nGlyphIndex += nKashidaCount;
        // now fix up the kashida positions
        for (int j = 0; j < nKashidaCount; j++)
        {
            (*(i)).maLinearPos.X() -= nGapWidth;
            nGapWidth -= nKashidaWidth;
            ++i;
        }

        // fixup rightmost kashida for gap remainder
        if( nGapWidth < 0 )
        {
            if( nKashidaCount <= 1 )
                nGapWidth /= 2;               // for small gap move kashida to middle
            (*(i-1)).mnNewWidth += nGapWidth;  // adjust kashida width to gap width
            (*(i-1)).maLinearPos.X() += nGapWidth;
        }

        (*i).mnNewWidth = (*i).mnOrigWidth;
        ++i;
    }

}

void GraphiteLayout::GetCaretPositions( int nArraySize, sal_Int32* pCaretXArray ) const
{
    // For each character except the last discover the caret positions
    // immediatly before and after that character.
    // This is used for underlines in the GUI amongst other things.
    // It may be used from MultiSalLayout, in which case it must take into account
    // glyphs that have been moved.
    std::fill(pCaretXArray, pCaretXArray + nArraySize, -1);
    // the layout method doesn't modify the layout even though it isn't
    // const in the interface
    bool bRtl = const_cast<GraphiteLayout*>(this)->maLayout.rightToLeft();
    int prevBase = -1;
    long prevClusterWidth = 0;
    for (int i = 0, nCharSlot = 0; i < nArraySize && nCharSlot < static_cast<int>(mvCharDxs.size()); ++nCharSlot, i+=2)
    {
        if (mvChar2BaseGlyph[nCharSlot] != -1)
        {
            int nChar2Base = mvChar2BaseGlyph[nCharSlot] & GLYPH_INDEX_MASK;
            assert((mvChar2BaseGlyph[nCharSlot] > -1) && (nChar2Base < (signed)mvGlyphs.size()));
            GlyphItem gi = mvGlyphs[nChar2Base];
            if (gi.mnGlyphIndex == GF_DROPPED)
            {
                continue;
            }
            int nCluster = nChar2Base;
            long origClusterWidth = gi.mnNewWidth;
            long nMin = gi.maLinearPos.X();
            long nMax = gi.maLinearPos.X() + gi.mnNewWidth;
            // attached glyphs are always stored after their base rtl or ltr
            while (++nCluster < static_cast<int>(mvGlyphs.size()) &&
                !mvGlyphs[nCluster].IsClusterStart())
            {
                origClusterWidth += mvGlyphs[nCluster].mnNewWidth;
                if (mvGlyph2Char[nCluster] == nCharSlot)
                {
                    nMin = std::min(nMin, mvGlyphs[nCluster].maLinearPos.X());
                    nMax = std::min(nMax, mvGlyphs[nCluster].maLinearPos.X() + mvGlyphs[nCluster].mnNewWidth);
                }
            }
            if (bRtl)
            {
                pCaretXArray[i+1] = nMin;
                pCaretXArray[i] = nMax;
            }
            else
            {
                pCaretXArray[i] = nMin;
                pCaretXArray[i+1] = nMax;
            }
            prevBase = nChar2Base;
            prevClusterWidth = origClusterWidth;
        }
        else if (prevBase > -1)
        {
            // this could probably be improved
            assert((prevBase > -1) && (prevBase < (signed)mvGlyphs.size()));
            GlyphItem gi = mvGlyphs[prevBase];
            int nGlyph = prevBase + 1;
            // try to find a better match, otherwise default to complete cluster
            for (; nGlyph < static_cast<int>(mvGlyphs.size()) &&
                 !mvGlyphs[nGlyph].IsClusterStart(); nGlyph++)
            {
                if (mvGlyph2Char[nGlyph] == nCharSlot)
                {
                    gi = mvGlyphs[nGlyph];
                    break;
                }
            }
            // if no match position at end of cluster
            if (nGlyph == static_cast<int>(mvGlyphs.size()) ||
                mvGlyphs[nGlyph].IsClusterStart())
            {
                if (bRtl)
                {
                    pCaretXArray[i+1] = gi.maLinearPos.X();
                    pCaretXArray[i] = gi.maLinearPos.X();
                }
                else
                {
                    pCaretXArray[i] = gi.maLinearPos.X() + prevClusterWidth;
                    pCaretXArray[i+1] = gi.maLinearPos.X() + prevClusterWidth;
                }
            }
            else
            {
                if (bRtl)
                {
                    pCaretXArray[i+1] = gi.maLinearPos.X();
                    pCaretXArray[i] = gi.maLinearPos.X() + gi.mnNewWidth;
                }
                else
                {
                    pCaretXArray[i] = gi.maLinearPos.X();
                    pCaretXArray[i+1] = gi.maLinearPos.X() + gi.mnNewWidth;
                }
            }
        }
        else
        {
            pCaretXArray[i] = pCaretXArray[i+1] = 0;
        }
#ifdef GRLAYOUT_DEBUG
        fprintf(grLog(),"%d,%d-%d\t", nCharSlot, pCaretXArray[i], pCaretXArray[i+1]);
#endif
    }
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"\n");
#endif
}


// GetNextGlyphs returns a contiguous sequence of glyphs that can be
// rendered together. It should never return a dropped glyph.
// The glyph_slot returned should be the index of the next visible
// glyph after the last glyph returned by this call.
// The char_index array should be filled with the characters corresponding
// to each glyph returned.
// glyph_adv array should be a virtual width such that if successive
// glyphs returned by this method are added one after the other they
// have the correct spacing.
// The logic in this method must match that expected in MultiSalLayout which
// is used when glyph fallback is in operation.
int GraphiteLayout::GetNextGlyphs( int length, sal_GlyphId * glyph_out,
        ::Point & aPosOut, int &glyph_slot, sal_Int32 * glyph_adv, int *char_index) const
{
  // Sanity check on the slot index.
  if (glyph_slot >= signed(mvGlyphs.size()))
  {
    glyph_slot = mvGlyphs.size();
    return 0;
  }
  assert(glyph_slot >= 0);
  // Find the first glyph in the substring.
  for (; glyph_slot < signed(mvGlyphs.size()) &&
          ((mvGlyphs.begin() + glyph_slot)->mnGlyphIndex == GF_DROPPED);
          ++glyph_slot) {};

  // Update the length
  const int nGlyphSlotEnd = std::min(size_t(glyph_slot + length), mvGlyphs.size());

  // We're all out of glyphs here.
  if (glyph_slot == nGlyphSlotEnd)
  {
    return 0;
  }

  // Find as many glyphs as we can which can be drawn in one go.
  Glyphs::const_iterator glyph_itr = mvGlyphs.begin() + glyph_slot;
  const int         glyph_slot_begin = glyph_slot;
  const int            initial_y_pos = glyph_itr->maLinearPos.Y();

  // Set the position to the position of the start glyph.
  ::Point aStartPos = glyph_itr->maLinearPos;
  //aPosOut = glyph_itr->maLinearPos;
  aPosOut = GetDrawPosition(aStartPos);


  for (;;)  // Forever
  {
     // last index of the range from glyph_to_chars does not include this glyph
     if (char_index)
     {
        assert((glyph_slot >= -1) && (glyph_slot < (signed)mvGlyph2Char.size()));
        if (mvGlyph2Char[glyph_slot] == -1)
            *char_index++ = mvCharDxs.size();
        else
            *char_index++ = mvGlyph2Char[glyph_slot];
     }
     // Copy out this glyphs data.
     ++glyph_slot;
     *glyph_out++ = glyph_itr->mnGlyphIndex;

     // Find the actual advance - this must be correct if called from
     // MultiSalLayout::AdjustLayout which requests one glyph at a time.
     const long nGlyphAdvance = (glyph_slot == static_cast<int>(mvGlyphs.size()))?
          glyph_itr->mnNewWidth :
          ((glyph_itr+1)->maLinearPos.X() - glyph_itr->maLinearPos.X());

#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"GetNextGlyphs g%d c%d x%ld,%ld adv%ld, pos %ld,%ld\n", glyph_slot - 1,
            GLYPH_INDEX_MASK&mvGlyph2Char[glyph_slot-1], glyph_itr->maLinearPos.X(), glyph_itr->maLinearPos.Y(), nGlyphAdvance,
            aPosOut.X(), aPosOut.Y());
#endif

     if (glyph_adv)  // If we are returning advance store it.
       *glyph_adv++ = nGlyphAdvance;
     else // Stop when next advance is unexpected.
       if (glyph_itr->mnOrigWidth != nGlyphAdvance)  break;

     // Have fetched all the glyphs we need to
     if (glyph_slot == nGlyphSlotEnd)
         break;

     ++glyph_itr;
     // Stop when next y position is unexpected.
     if (initial_y_pos != glyph_itr->maLinearPos.Y())
       break;

     // Stop if glyph dropped
     if (glyph_itr->mnGlyphIndex == GF_DROPPED)
       break;
  }
  int numGlyphs = glyph_slot - glyph_slot_begin;
  // move the next glyph_slot to a glyph that hasn't been dropped
  while (glyph_slot < static_cast<int>(mvGlyphs.size()) &&
         (mvGlyphs.begin() + glyph_slot)->mnGlyphIndex == GF_DROPPED)
         ++glyph_slot;
  return numGlyphs;
}


void GraphiteLayout::MoveGlyph( int nGlyphIndex, long nNewPos )
{
    // TODO it might be better to actualy implement simplify properly, but this
    // needs to be done carefully so the glyph/char maps are maintained
    // If a glyph has been dropped then it wasn't returned by GetNextGlyphs, so
    // the index here may be wrong
    while ((mvGlyphs[nGlyphIndex].mnGlyphIndex == GF_DROPPED) &&
           (nGlyphIndex < (signed)mvGlyphs.size()))
    {
        nGlyphIndex++;
    }
    const long dx = nNewPos - mvGlyphs[nGlyphIndex].maLinearPos.X();

    if (dx == 0)  return;
    // GenericSalLayout only changes maLinearPos, mvCharDxs doesn't change
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"Move %d (%ld,%ld) c%d by %ld\n", nGlyphIndex, mvGlyphs[nGlyphIndex].maLinearPos.X(), nNewPos, mvGlyph2Char[nGlyphIndex], dx);
#endif
    for (size_t gi = nGlyphIndex; gi < mvGlyphs.size(); gi++)
    {
        mvGlyphs[gi].maLinearPos.X() += dx;
    }
    // width does need to be updated for correct fallback
    mnWidth += dx;
}


void GraphiteLayout::DropGlyph( int nGlyphIndex )
{
    if(nGlyphIndex >= signed(mvGlyphs.size()))
        return;

    GlyphItem & glyph = mvGlyphs[nGlyphIndex];
    glyph.mnGlyphIndex = GF_DROPPED;
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"Dropped %d\n", nGlyphIndex);
#endif
}

void GraphiteLayout::Simplify( bool isBaseLayout )
{
  const sal_GlyphId dropMarker = isBaseLayout ? GF_DROPPED : 0;

  Glyphs::iterator gi = mvGlyphs.begin();
  // TODO check whether we need to adjust positions here
  // MultiSalLayout seems to move the glyphs itself, so it may not be needed.
  long deltaX = 0;
  while (gi != mvGlyphs.end())
  {
      if (gi->mnGlyphIndex == dropMarker)
      {
        deltaX += gi->mnNewWidth;
        gi->mnNewWidth = 0;
      }
      else
      {
        deltaX = 0;
      }
      //mvCharDxs[mvGlyph2Char[gi->mnCharPos]] -= deltaX;
      ++gi;
  }
#ifdef GRLAYOUT_DEBUG
  fprintf(grLog(),"Simplify base%d dx=%ld newW=%ld\n", isBaseLayout, deltaX, mnWidth - deltaX);
#endif
  // discard width from trailing dropped glyphs, but not those in the middle
  mnWidth -= deltaX;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
