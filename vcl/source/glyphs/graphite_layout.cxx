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

// Description: An implementation of the SalLayout interface that uses the
//              Graphite engine.

// Enable lots of debug info
#if OSL_DEBUG_LEVEL > 1
#include <cstdio>
#define GRLAYOUT_DEBUG 1
#undef NDEBUG
#endif

// #define GRLAYOUT_DEBUG 1

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
#include <svsys.h>

#include <salgdi.hxx>

#include <unicode/uchar.h>
#include <unicode/ubidi.h>
#include <unicode/uscript.h>

// Graphite Libraries (must be after vcl headers on windows)
#include <graphite2/Segment.h>

#include <graphite_layout.hxx>
#include <graphite_features.hxx>

// Module private type definitions and forward declarations.
//
// Module private names.
//

#ifdef GRLAYOUT_DEBUG
static FILE * grLog()
{
#ifdef WNT
    static FILE * grLogFile = NULL;
    if (grLogFile == NULL)
    {
        std::string logFileName(getenv("TEMP"));
        logFileName.append("/graphitelayout.log");
        grLogFile = fopen(logFileName.c_str(),"w");
    }
    else
        fflush(grLogFile);
    return grLogFile;
#else
    fflush(stdout);
    return stdout;
#endif
}
#endif

namespace
{
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

    int findSameDirLimit(const sal_Unicode* buffer, int charCount, bool rtl)
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

    template <typename T>
    T maximum(T a, T b)
    {
        return (a > b)? a : b;
    }
    template <typename T>
    T minimum(T a, T b)
    {
        return (a < b)? a : b;
    }

} // namespace

// Impementation of the GraphiteLayout::Glyphs container class.
//    This is an extended vector class with methods added to enable
//        o Correctly filling with glyphs.
//        o Querying clustering relationships.
//        o manipulations that affect neighouring glyphs.

const int GraphiteLayout::EXTRA_CONTEXT_LENGTH = 10;

// find first slot of cluster and first slot of subsequent cluster
static void findFirstClusterSlot(const gr_slot* base, gr_slot const** first, gr_slot const** after, int * firstChar, int * lastChar, bool bRtl)
{
    if (gr_slot_attached_to(base) == NULL)
    {
        *first = base;
        *after = (bRtl)? gr_slot_prev_in_segment(base) :
            gr_slot_next_in_segment(base);
        *firstChar = gr_slot_before(base);
        *lastChar = gr_slot_after(base);
    }
    const gr_slot * attachment = gr_slot_first_attachment(base);
    while (attachment)
    {
        if (gr_slot_origin_X(*first) > gr_slot_origin_X(attachment))
            *first = attachment;
        const gr_slot* attachmentNext = (bRtl)?
            gr_slot_prev_in_segment(attachment) : gr_slot_next_in_segment(attachment);
        if (attachmentNext)
        {
            if (*after && (gr_slot_origin_X(*after) < gr_slot_origin_X(attachmentNext)))
                *after = attachmentNext;
        }
        else
        {
            *after = NULL;
        }
        if (gr_slot_before(attachment) < *firstChar)
            *firstChar = gr_slot_before(attachment);
        if (gr_slot_after(attachment) > *lastChar)
            *lastChar = gr_slot_after(attachment);
        if (gr_slot_first_attachment(attachment))
            findFirstClusterSlot(attachment, first, after, firstChar, lastChar, bRtl);
        attachment = gr_slot_next_sibling_attachment(attachment);
    }
}

// The Graphite glyph stream is really a sequence of glyph attachment trees
//  each rooted at a non-attached base glyph.  fill_from walks the glyph stream,
//  finds each non-attached base glyph and calls append to record them as a
//  sequence of clusters.
void
GraphiteLayout::fillFrom(gr_segment * pSegment, ImplLayoutArgs &rArgs, float fScaling)
{
    bool bRtl = (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL);
    int nCharRequested = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
    int nChar = gr_seg_n_cinfo(pSegment);
    float fMinX = gr_seg_advance_X(pSegment);
    float fMaxX = 0.0f;
    long nDxOffset = 0; // from dropped glyphs
    int nFirstCharInCluster = 0;
    int nLastCharInCluster = 0;
    unsigned int nGlyphs = gr_seg_n_slots(pSegment);
    mvGlyph2Char.assign(nGlyphs, -1);
    mvGlyphs.reserve(nGlyphs);

    if (bRtl)
    {
        const gr_slot* baseSlot = gr_seg_last_slot(pSegment);
        // find first base
        while (baseSlot && (gr_slot_attached_to(baseSlot) != NULL))
            baseSlot = gr_slot_prev_in_segment(baseSlot);
        int iChar = nChar - 1;
        int iNextChar = nChar - 1;
        bool reordered = false;
        int nBaseGlyphIndex = 0;
        // now loop over bases
        while (baseSlot)
        {
            bool bCluster = !reordered;
            const gr_slot * clusterFirst = NULL;
            const gr_slot * clusterAfter = NULL;
            int firstChar = -1;
            int lastChar = -1;
            findFirstClusterSlot(baseSlot, &clusterFirst, &clusterAfter, &firstChar, &lastChar, bRtl);
            iNextChar = minimum<int>(firstChar, iNextChar);
            if (bCluster)
            {
                nBaseGlyphIndex = mvGlyphs.size();
                mvGlyph2Char[nBaseGlyphIndex] = iChar + mnSegCharOffset;
                nFirstCharInCluster = firstChar;
                nLastCharInCluster = lastChar;
            }
            else
            {
                mvGlyph2Char[mvGlyphs.size()] = firstChar + mnSegCharOffset;
                nFirstCharInCluster = minimum<int>(firstChar, nFirstCharInCluster);
                nLastCharInCluster = maximum<int>(firstChar, nLastCharInCluster);
            }
            float leftBoundary = gr_slot_origin_X(clusterFirst);
            float rightBoundary = (clusterAfter)?
                gr_slot_origin_X(clusterAfter) : gr_seg_advance_X(pSegment);
            if (
                lastChar < iChar &&
                 (gr_cinfo_after(gr_seg_cinfo(pSegment, iChar)) >
                 static_cast<int>(gr_slot_index(clusterAfter)))
               )
            {
                reordered = true;
            }
            else
            {
                reordered = false;
                iChar = iNextChar - 1;
            }
            if (mnSegCharOffset + nFirstCharInCluster >= mnMinCharPos &&
                mnSegCharOffset + nFirstCharInCluster < mnEndCharPos)
            {
                fMinX = minimum<float>(fMinX, leftBoundary);
                fMaxX = maximum<float>(fMaxX, rightBoundary);
                if (!reordered)
                {
                    for (int i = nFirstCharInCluster; i <= nLastCharInCluster; i++)
                    {
                        if (mnSegCharOffset + i >= mnEndCharPos)
                            break;
                        // from the point of view of the dx array, the xpos is
                        // the origin of the first glyph of the cluster rtl
                        mvCharDxs[mnSegCharOffset + i - mnMinCharPos] =
                            static_cast<int>(leftBoundary * fScaling) + nDxOffset;
                        mvCharBreaks[mnSegCharOffset + i - mnMinCharPos] = gr_cinfo_break_weight(gr_seg_cinfo(pSegment, i));
                    }
                    mvChar2BaseGlyph[mnSegCharOffset + nFirstCharInCluster - mnMinCharPos] = nBaseGlyphIndex;
                }
                append(pSegment, rArgs, baseSlot, gr_slot_origin_X(baseSlot), rightBoundary, fScaling,
                       nDxOffset, bCluster, mnSegCharOffset + firstChar);
            }
            if (mnSegCharOffset + nLastCharInCluster < mnMinCharPos)
                break;
            baseSlot = gr_slot_next_sibling_attachment(baseSlot);
        }
    }
    else
    {
        const gr_slot* baseSlot = gr_seg_first_slot(pSegment);
        // find first base
        while (baseSlot && (gr_slot_attached_to(baseSlot) != NULL))
            baseSlot = gr_slot_next_in_segment(baseSlot);
        int iChar = 0; // relative to segment
        int iNextChar = 0;
        bool reordered = false;
        int nBaseGlyphIndex = 0;
        // now loop over bases
        while (baseSlot)
        {
            bool bCluster = !reordered;
            const gr_slot * clusterFirst = NULL;
            const gr_slot * clusterAfter = NULL;
            int firstChar = -1;
            int lastChar = -1;
            findFirstClusterSlot(baseSlot, &clusterFirst, &clusterAfter, &firstChar, &lastChar, bRtl);
            iNextChar = maximum<int>(lastChar, iNextChar);
            if (bCluster)
            {
                nBaseGlyphIndex = mvGlyphs.size();
                mvGlyph2Char[nBaseGlyphIndex] = iChar + mnSegCharOffset;
                nFirstCharInCluster = firstChar;
                nLastCharInCluster = lastChar;
            }
            else
            {
                mvGlyph2Char[mvGlyphs.size()] = firstChar + mnSegCharOffset;
                nFirstCharInCluster = minimum<int>(firstChar, nFirstCharInCluster);
                nLastCharInCluster = maximum<int>(lastChar, nLastCharInCluster);
            }
            if (
                firstChar > iChar &&
                 (gr_cinfo_before(gr_seg_cinfo(pSegment, iChar)) >
                 static_cast<int>(gr_slot_index(clusterFirst)))
               )
            {
                reordered = true;
            }
            else
            {
                reordered = false;
                iChar = iNextChar + 1;
            }
            float leftBoundary = gr_slot_origin_X(clusterFirst);
            float rightBoundary = (clusterAfter)?
                gr_slot_origin_X(clusterAfter) : gr_seg_advance_X(pSegment);
            int bFirstChar = gr_cinfo_base(gr_seg_cinfo(pSegment, nFirstCharInCluster));
            if (mnSegCharOffset + bFirstChar >= mnMinCharPos &&
                mnSegCharOffset + bFirstChar < mnEndCharPos)
            {
                fMinX = minimum<float>(fMinX, leftBoundary);
                fMaxX = maximum<float>(fMaxX, rightBoundary);
                if (!reordered)
                {
                    for (int i = nFirstCharInCluster; i <= nLastCharInCluster; i++)
                    {
                        int ibase = gr_cinfo_base(gr_seg_cinfo(pSegment, i));
                        if (mnSegCharOffset + ibase >= mnEndCharPos)
                            break;
                        // from the point of view of the dx array, the xpos is
                        // the origin of the first glyph of the next cluster ltr
                        mvCharDxs[mnSegCharOffset + ibase - mnMinCharPos] =
                            static_cast<int>(rightBoundary * fScaling) + nDxOffset;
                        mvCharBreaks[mnSegCharOffset + ibase - mnMinCharPos] = gr_cinfo_break_weight(gr_seg_cinfo(pSegment, i));
                    }
                    // only set mvChar2BaseGlyph for first character of cluster
                    mvChar2BaseGlyph[mnSegCharOffset + bFirstChar - mnMinCharPos] = nBaseGlyphIndex;
                }
                append(pSegment, rArgs, baseSlot, gr_slot_origin_X(baseSlot), rightBoundary, fScaling,
                       nDxOffset, true, mnSegCharOffset + firstChar);
            }
            if (mnSegCharOffset + bFirstChar >= mnEndCharPos)
                break;
            baseSlot = gr_slot_next_sibling_attachment(baseSlot);
        }
    }
    long nXOffset = round(fMinX * fScaling);
    mnWidth = round(fMaxX * fScaling) - nXOffset + nDxOffset;
    if (mnWidth < 0)
    {
        // This can happen when there was no base inside the range
        mnWidth = 0;
    }
    // fill up non-base char dx with cluster widths from previous base glyph
    if (bRtl)
    {
        if (mvCharDxs[nCharRequested-1] == -1)
            mvCharDxs[nCharRequested-1] = 0;
        else
            mvCharDxs[nCharRequested-1] -= nXOffset;
        for (int i = nCharRequested - 2; i >= 0; i--)
        {
            if (mvCharDxs[i] == -1) mvCharDxs[i] = mvCharDxs[i+1];
            else mvCharDxs[i] -= nXOffset;
        }
    }
    else
    {
        if (mvCharDxs[0] == -1)
            mvCharDxs[0] = 0;
        else
            mvCharDxs[0] -= nXOffset;
        for (int i = 1; i < nCharRequested; i++)
        {
            if (mvCharDxs[i] == -1) mvCharDxs[i] = mvCharDxs[i-1];
            else mvCharDxs[i] -= nXOffset;
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"%d,%d ", (int)i, (int)mvCharDxs[i]);
#endif
        }
    }
    // remove offset due to context if there is one
    if (nXOffset != 0)
    {
        for (size_t i = 0; i < mvGlyphs.size(); i++)
            mvGlyphs[i].maLinearPos.X() -= nXOffset;
    }
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(), "fillFrom %" SAL_PRI_SIZET "u glyphs offset %ld width %ld\n", mvGlyphs.size(), nXOffset, mnWidth);
#endif
}

// append walks an attachment tree, flattening it, and converting it into a
// sequence of GlyphItem objects which we can later manipulate.
float
GraphiteLayout::append(gr_segment *pSeg, ImplLayoutArgs &rArgs,
    const gr_slot * gi, float gOrigin, float nextGlyphOrigin, float scaling, long & rDXOffset,
    bool bIsBase, int baseChar)
{
    bool bRtl = (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL);
    float nextOrigin;
    assert(gi);
    assert(gr_slot_before(gi) <= gr_slot_after(gi));
    int firstChar = gr_slot_before(gi) + mnSegCharOffset;
    assert(mvGlyphs.size() < mvGlyph2Char.size());
    if (!bIsBase) mvGlyph2Char[mvGlyphs.size()] = baseChar;//firstChar;
    // is the next glyph attached or in the next cluster?
    //glyph_set_range_t iAttached = gi.attachedClusterGlyphs();
    const gr_slot * pFirstAttached = gr_slot_first_attachment(gi);
    const gr_slot * pNextSibling = gr_slot_next_sibling_attachment(gi);
    if (pFirstAttached)
        nextOrigin = gr_slot_origin_X(pFirstAttached);
    else if (!bIsBase && pNextSibling)
        nextOrigin = gr_slot_origin_X(pNextSibling);
    else
        nextOrigin = nextGlyphOrigin;
    long glyphId = gr_slot_gid(gi);
    long deltaOffset = 0;
    int scaledGlyphPos = round(gr_slot_origin_X(gi) * scaling);
    int glyphWidth = round((nextOrigin - gOrigin) * scaling);
//    if (glyphWidth < 0)
//    {
//        nextOrigin = gOrigin;
//        glyphWidth = 0;
//    }
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"c%d g%ld,X%d W%d nX%f ", firstChar, glyphId,
        (int)(gr_slot_origin_X(gi) * scaling), glyphWidth, nextOrigin * scaling);
#endif
    if (glyphId == 0)
    {
        rArgs.NeedFallback(firstChar, bRtl);
        if( (SAL_LAYOUT_FOR_FALLBACK & rArgs.mnFlags ))
        {
            glyphId = GF_DROPPED;
            deltaOffset -= glyphWidth;
            glyphWidth = 0;
        }
    }
    else if(rArgs.mnFlags & SAL_LAYOUT_FOR_FALLBACK)
    {
#ifdef GRLAYOUT_DEBUG
        fprintf(grLog(),"fallback c%d %x in run %d\n", firstChar, rArgs.mpStr[firstChar],
            rArgs.maRuns.PosIsInAnyRun(firstChar));
#endif
        // glyphs that aren't requested for fallback will be taken from base
        // layout, so mark them as dropped (should this wait until Simplify(false) is called?)
        if (!rArgs.maRuns.PosIsInAnyRun(firstChar) &&
            in_range(firstChar, rArgs.mnMinCharPos, rArgs.mnEndCharPos))
        {
            glyphId = GF_DROPPED;
            deltaOffset -= glyphWidth;
            glyphWidth = 0;
        }
    }
    // append this glyph. Set the cluster flag if this glyph is attached to another
    long nGlyphFlags = bIsBase ? 0 : GlyphItem::IS_IN_CLUSTER;
    nGlyphFlags |= (bRtl)? GlyphItem::IS_RTL_GLYPH : 0;
    GlyphItem aGlyphItem(mvGlyphs.size(),
        glyphId,
        Point(scaledGlyphPos + rDXOffset,
            round((-gr_slot_origin_Y(gi) * scaling))),
        nGlyphFlags,
        glyphWidth);
    if (glyphId != static_cast<long>(GF_DROPPED))
        aGlyphItem.mnOrigWidth = round(gr_slot_advance_X(gi, mpFace, mpFont) * scaling);
    mvGlyphs.push_back(aGlyphItem);

    // update the offset if this glyph was dropped
    rDXOffset += deltaOffset;

    // Recursively append all the attached glyphs.
    float cOrigin = nextOrigin;
    for (const gr_slot * agi = gr_slot_first_attachment(gi); agi != NULL; agi = gr_slot_next_sibling_attachment(agi))
        cOrigin = append(pSeg, rArgs, agi, cOrigin, nextGlyphOrigin, scaling, rDXOffset, false, baseChar);

    return cOrigin;
}

//
// An implementation of the SalLayout interface to enable Graphite enabled fonts to be used.
//
GraphiteLayout::GraphiteLayout(const gr_face * face, gr_font * font,
                               const grutils::GrFeatureParser * pFeatures) throw()
  : mpFace(face),
    mpFont(font),
    mnWidth(0),
    mfScaling(1.0),
    mpFeatures(pFeatures)
{

}

GraphiteLayout::~GraphiteLayout() throw()
{
    clear();
    // the features and font are owned by the platform layers
    mpFeatures = NULL;
    mpFont = NULL;
}

void GraphiteLayout::clear()
{
    // Destroy the segment and text source from any previous invocation of
    // LayoutText
    mvGlyphs.clear();
    mvCharDxs.clear();
    mvChar2BaseGlyph.clear();
    mvGlyph2Char.clear();

    // Reset the state to the empty state.
    mnWidth = 0;
    // Don't reset the scaling, because it is set before LayoutText
}

// This method shouldn't be called on windows, since it needs the dc reset
bool GraphiteLayout::LayoutText(ImplLayoutArgs & rArgs)
{
    bool success = true;
    if (rArgs.mnMinCharPos < rArgs.mnEndCharPos)
    {
        gr_segment * pSegment = CreateSegment(rArgs);
        if (!pSegment)
            return false;
        success = LayoutGlyphs(rArgs, pSegment);
        if (pSegment)
        {
            gr_seg_destroy(pSegment);
            pSegment = NULL;
        }
    }
    else
    {
        clear();
    }
    return success;
}


gr_segment * GraphiteLayout::CreateSegment(ImplLayoutArgs& rArgs)
{
    assert(rArgs.mnLength >= 0);

    gr_segment * pSegment = NULL;

    // Set the SalLayouts values to be the initial ones.
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
        //if (bRtl && (mrFont.getSupportedScriptDirections() & gr::kfsdcHorizRtl))
        //    maLayout.setRightToLeft(bRtl);

        // Context is often needed beyond the specified end, however, we don't
        // want it if there has been a direction change, since it is hard
        // to tell between reordering within one direction and multi-directional
        // text. Extra context, can also cause problems with ligatures stradling
        // a hyphenation point, so disable if CTL is disabled.
        mnSegCharOffset = rArgs.mnMinCharPos;
        int limit = rArgs.mnEndCharPos;
        if (!(SAL_LAYOUT_COMPLEX_DISABLED & rArgs.mnFlags))
        {
            const int nSegCharMin = maximum<int>(0, mnMinCharPos - EXTRA_CONTEXT_LENGTH);
            const int nSegCharLimit = minimum(rArgs.mnLength, mnEndCharPos + EXTRA_CONTEXT_LENGTH);
            if (nSegCharMin < mnSegCharOffset)
            {
                int sameDirEnd = findSameDirLimit(rArgs.mpStr + nSegCharMin,
                    rArgs.mnEndCharPos - nSegCharMin, bRtl);
                if (sameDirEnd == rArgs.mnEndCharPos)
                    mnSegCharOffset = nSegCharMin;
            }
            if (nSegCharLimit > limit)
            {
                limit += findSameDirLimit(rArgs.mpStr + rArgs.mnEndCharPos,
                    nSegCharLimit - rArgs.mnEndCharPos, bRtl);
            }
        }
//        int numchars = gr_count_unicode_characters(gr_utf16, rArgs.mpStr + mnSegCharOffset,
//                rArgs.mpStr + (rArgs.mnLength > limit + 64 ? limit + 64 : rArgs.mnLength), NULL);
          int numchars = rArgs.mnEndCharPos - mnSegCharOffset; // fdo#52540, fdo#68313, FIXME
        if (mpFeatures)
            pSegment = gr_make_seg(mpFont, mpFace, 0, mpFeatures->values(), gr_utf16,
                                        rArgs.mpStr + mnSegCharOffset, numchars, bRtl);
        else
            pSegment = gr_make_seg(mpFont, mpFace, 0, NULL, gr_utf16,
                                        rArgs.mpStr + mnSegCharOffset, numchars, bRtl);

        //pSegment = new gr::RangeSegment((gr::Font *)&mrFont, mpTextSrc, &maLayout, mnMinCharPos, limit);
        if (pSegment != NULL)
        {
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"Gr::LayoutText %d-%d, context %d, len %d, numchars %d, rtl %d scaling %f:", rArgs.mnMinCharPos,
               rArgs.mnEndCharPos, limit, rArgs.mnLength, numchars, bRtl, mfScaling);
            for (int i = mnSegCharOffset; i < limit; ++i)
                fprintf(grLog(), " %04X", rArgs.mpStr[i]);
            fprintf(grLog(), "\n");
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

bool GraphiteLayout::LayoutGlyphs(ImplLayoutArgs& rArgs, gr_segment * pSegment)
{
    // Calculate the initial character dxs.
    mvCharDxs.assign(mnEndCharPos - mnMinCharPos, -1);
    mvChar2BaseGlyph.assign(mnEndCharPos - mnMinCharPos, -1);
    mvCharBreaks.assign(mnEndCharPos - mnMinCharPos, 0);
    mnWidth = 0;
    if (mvCharDxs.size() > 0)
    {
        // Discover all the clusters.
        try
        {
            bool bRtl = mnLayoutFlags & SAL_LAYOUT_BIDI_RTL;
            fillFrom(pSegment, rArgs, mfScaling);

            if (bRtl)
            {
                // not needed for adjacent differences, but for mouse clicks to char
                std::transform(mvCharDxs.begin(), mvCharDxs.end(), mvCharDxs.begin(),
                    std::bind1st(std::minus<long>(), mnWidth));
                // fixup last dx to ensure it always equals the width
                mvCharDxs[mvCharDxs.size() - 1] = mnWidth;
            }
        }
        catch (const std::exception &e)
        {
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"LayoutGlyphs failed %s\n", e.what());
#else
            (void)e;
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
    long wLastBreak = 0;
    int nLastBreak = -1;
    int nEmergency = -1;
    for (size_t i = 1; i < mvCharDxs.size(); i++)
    {
        nWidth += char_extra;
        if (nWidth > maxmnWidth) break;
        if (mvChar2BaseGlyph[i] != -1)
        {
            if (
                (mvCharBreaks[i] > -35 || (mvCharBreaks[i-1] > 0 && mvCharBreaks[i-1] < 35)) &&
                (mvCharBreaks[i-1] < 35 || (mvCharBreaks[i] < 0 && mvCharBreaks[i] > -35))
               )
            {
                nLastBreak = static_cast<int>(i);
                wLastBreak = nWidth;
            }
            nEmergency = static_cast<int>(i);
        }
        nWidth += (mvCharDxs[i] - mvCharDxs[i-1]) * factor;
    }
    int nBreak = mnMinCharPos;
    if (wLastBreak > 9 * maxmnWidth / 10)
        nBreak += nLastBreak;
    else
        if (nEmergency > -1)
            nBreak += nEmergency;

#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(), "Gr::GetTextBreak break after %d, weights(%d, %d)\n", nBreak - mnMinCharPos, mvCharBreaks[nBreak - mnMinCharPos], mvCharBreaks[nBreak - mnMinCharPos - 1]);
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
                ((signed)(mvChar2BaseGlyph[i]) < (signed)mvGlyphs.size()));
            if (mvChar2BaseGlyph[i] != -1 &&
                mvGlyphs[mvChar2BaseGlyph[i]].mnGlyphIndex == GF_DROPPED)
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
    fprintf(grLog(),"FillDXArray %d-%d=%ld\n", mnMinCharPos, mnEndCharPos, mnWidth);
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
        // NOTE: for expansion we can use base glyphs (which have IsClusterStart set)
        // even though they may have been reordered in which case they will have
        // been placed in a bigger cluster for other purposes.
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
                    nOffset = static_cast<int>(fExtraPerCluster * nCluster);
                    int nCharIndex = mvGlyph2Char[i];
                    assert(nCharIndex > -1);
                    if (nCharIndex < mnMinCharPos ||
                        static_cast<size_t>(nCharIndex-mnMinCharPos)
                            >= mvCharDxs.size())
                    {
                        continue;
                    }
                    mvCharDxs[nCharIndex-mnMinCharPos] += nOffset;
                    // adjust char dxs for rest of characters in cluster
                    while (++nCharIndex - mnMinCharPos < static_cast<int>(mvChar2BaseGlyph.size()))
                    {
                        int nChar2Base = mvChar2BaseGlyph[nCharIndex-mnMinCharPos];
                        if (nChar2Base == -1 || nChar2Base == static_cast<int>(i))
                            mvCharDxs[nCharIndex-mnMinCharPos] += nOffset;
                        else
                            break;
                    }
                    ++nCluster;
                }
                mvGlyphs[i].maLinearPos.X() += nOffset;
            }
        }
    }
    else if (nDeltaWidth < 0)// condense - apply a factor to all glyph positions
    {
        if (mvGlyphs.empty()) return;
        Glyphs::iterator iLastGlyph = mvGlyphs.begin() + (mvGlyphs.size() - 1);
        // position last glyph using original width
        float fXFactor = static_cast<float>(rArgs.mnLayoutWidth - iLastGlyph->mnOrigWidth) / static_cast<float>(iLastGlyph->maLinearPos.X());
#ifdef GRLAYOUT_DEBUG
        fprintf(grLog(), "Condense by factor %f last x%ld\n", fXFactor, iLastGlyph->maLinearPos.X());
#endif
        if (fXFactor < 0)
            return; // probably a bad mnOrigWidth value
        iLastGlyph->maLinearPos.X() = rArgs.mnLayoutWidth - iLastGlyph->mnOrigWidth;
        Glyphs::iterator iGlyph = mvGlyphs.begin();
        while (iGlyph != iLastGlyph)
        {
            iGlyph->maLinearPos.X() = static_cast<int>(static_cast<float>(iGlyph->maLinearPos.X()) * fXFactor);
            ++iGlyph;
        }
        for (size_t i = 0; i < mvCharDxs.size(); i++)
        {
            mvCharDxs[i] = static_cast<int>(fXFactor * static_cast<float>(mvCharDxs[i]));
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
        int nChar2Base = mvChar2BaseGlyph[i];
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
            int nChar2BaseJ = -1;
            for (; j < nChars; j++)
            {
                nChar2BaseJ = mvChar2BaseGlyph[j];
                assert((nChar2BaseJ >= -1) && (nChar2BaseJ < (signed)mvGlyphs.size()));
                if (nChar2BaseJ != -1 )
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
            int nBaseCount = 0;
            // count bases within cluster - may be more than 1 with reordering
            for (int k = nChar2Base; k <= nLastGlyph; k++)
            {
                if (mvGlyphs[k].IsClusterStart()) ++nBaseCount;
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
            long nDOriginPerBase = (nBaseCount > 0)? nDWidth / nBaseCount : 0;
            nBaseCount = -1;
            // update glyph positions
            if (bRtl)
            {
                for (int n = nChar2Base; n <= nLastGlyph; n++)
                {
                    if (mvGlyphs[n].IsClusterStart()) ++nBaseCount;
                    assert((n > - 1) && (n < (signed)mvGlyphs.size()));
                    mvGlyphs[n].maLinearPos.X() += -(nDGlyphOrigin + nDOriginPerBase * nBaseCount) + nXOffset;
                }
            }
            else
            {
                for (int n = nChar2Base; n <= nLastGlyph; n++)
                {
                    if (mvGlyphs[n].IsClusterStart()) ++nBaseCount;
                    assert((n > - 1) && (n < (signed)mvGlyphs.size()));
                    mvGlyphs[n].maLinearPos.X() += nDGlyphOrigin + (nDOriginPerBase * nBaseCount) + nXOffset;
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
    // immediately before and after that character.
    // This is used for underlines in the GUI amongst other things.
    // It may be used from MultiSalLayout, in which case it must take into account
    // glyphs that have been moved.
    std::fill(pCaretXArray, pCaretXArray + nArraySize, -1);
    // the layout method doesn't modify the layout even though it isn't
    // const in the interface
    bool bRtl = (mnLayoutFlags & SAL_LAYOUT_BIDI_RTL);//const_cast<GraphiteLayout*>(this)->maLayout.rightToLeft();
    int prevBase = -1;
    long prevClusterWidth = 0;
    for (int i = 0, nCharSlot = 0; i < nArraySize && nCharSlot < static_cast<int>(mvCharDxs.size()); ++nCharSlot, i+=2)
    {
        if (mvChar2BaseGlyph[nCharSlot] != -1)
        {
            int nChar2Base = mvChar2BaseGlyph[nCharSlot];
            assert((nChar2Base > -1) && (nChar2Base < (signed)mvGlyphs.size()));
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
                    nMin = minimum(nMin, mvGlyphs[nCluster].maLinearPos.X());
                    nMax = maximum(nMax, mvGlyphs[nCluster].maLinearPos.X() + mvGlyphs[nCluster].mnNewWidth);
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
        ::Point & aPosOut, int &glyph_slot, sal_Int32 * glyph_adv, int *char_index,
        const PhysicalFontFace** /*pFallbackFonts*/ ) const
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
  const int nGlyphSlotEnd = minimum(size_t(glyph_slot + length), mvGlyphs.size());

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
         if (glyph_slot >= (signed)mvGlyph2Char.size())
         {
            *char_index++ = mnMinCharPos + mvCharDxs.size();
         }
         else
         {
            assert(glyph_slot > -1);
            if (mvGlyph2Char[glyph_slot] == -1)
                *char_index++ = mnMinCharPos + mvCharDxs.size();
            else
                *char_index++ = mvGlyph2Char[glyph_slot];
         }
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
    fprintf(grLog(),"GetNextGlyphs g%d gid%d c%d x%ld,%ld adv%ld, pos %ld,%ld\n",
            glyph_slot - 1, glyph_itr->mnGlyphIndex,
            mvGlyph2Char[glyph_slot-1], glyph_itr->maLinearPos.X(), glyph_itr->maLinearPos.Y(), nGlyphAdvance,
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
      ++gi;
  }
#ifdef GRLAYOUT_DEBUG
  fprintf(grLog(),"Simplify base%d dx=%ld newW=%ld\n", isBaseLayout, deltaX, mnWidth - deltaX);
#endif
  // discard width from trailing dropped glyphs, but not those in the middle
  mnWidth -= deltaX;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
