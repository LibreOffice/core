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

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <numeric>
#include <deque>

#include <config_global.h>

#include <svsys.h>

#include <salgdi.hxx>

#include <unicode/uchar.h>
#include <unicode/ubidi.h>
#include <unicode/uscript.h>

#include <vcl/unohelp.hxx>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>

// Graphite Libraries (must be after vcl headers on windows)
#include <graphite_static.hxx>
#include <graphite2/Segment.h>

#include <graphite_layout.hxx>
#include <graphite_features.hxx>

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
    inline long round_to_long(const float n) {
        return long(n + (n < 0 ? -0.5 : 0.5));
    }

    template<typename T>
    inline bool in_range(const T i, const T b, const T e) {
        return !(b > i) && i < e;
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

const int GraphiteLayout::EXTRA_CONTEXT_LENGTH = 32;

const gr_slot *get_next_base(const gr_slot *slot, bool bRtl)
{
    for ( ; slot; slot = bRtl ? gr_slot_prev_in_segment(slot) : gr_slot_next_in_segment(slot))
        if (!gr_slot_attached_to(slot) || gr_slot_can_insert_before(slot))
            break;
    return slot;
}

bool isWhite(sal_Unicode nChar)
{
    if (nChar <= 0x0020 || nChar == 0x00A0 || (nChar >= 0x2000 && nChar <= 0x200F) || nChar == 0x3000)
        return true;
    return false;
}

// The Graphite glyph stream is really a sequence of glyph attachment trees
//  each rooted at a non-attached base glyph.  fill_from walks the glyph stream,
//  finds each non-attached base glyph and calls append to record them as a
//  sequence of clusters.
void
GraphiteLayout::fillFrom(gr_segment * pSegment, ImplLayoutArgs &rArgs, float fScaling, bool bRtl, int lastCharPos)
{
    float fMinX = gr_seg_advance_X(pSegment);
    float fMaxX = 0.0f;
    long nDxOffset = 0; // from dropped glyphs
    int origNumGlyphs = mvGlyphs.size();
    unsigned int nGlyphs = gr_seg_n_slots(pSegment);
    mvGlyph2Char.resize(mvGlyph2Char.size() + nGlyphs, -1);
    mvGlyphs.reserve(mvGlyphs.size() + nGlyphs);
    int clusterStart = -1;
    int clusterFirstChar = -1;
    const gr_slot *nextBaseSlot;
    const sal_Unicode *pStr = rArgs.mrStr.getStr();
    int firstChar;

    if (!nGlyphs || lastCharPos - mnSegCharOffset == 0) return;
    const gr_slot* baseSlot = bRtl ? gr_seg_last_slot(pSegment) : gr_seg_first_slot(pSegment);
    // find first base
    while (baseSlot && gr_slot_attached_to(baseSlot) != nullptr && !gr_slot_can_insert_before(baseSlot))
        baseSlot = bRtl ? gr_slot_prev_in_segment(baseSlot) : gr_slot_next_in_segment(baseSlot);
    assert(baseSlot);
    int nextChar = gr_cinfo_base(gr_seg_cinfo(pSegment, gr_slot_before(baseSlot))) + mnSegCharOffset;
    float thisBoundary = 0.;
    float nextBoundary = gr_slot_origin_X(baseSlot);
    // now loop over bases
    for ( ; baseSlot; baseSlot = nextBaseSlot)
    {
        firstChar = nextChar;
        thisBoundary = nextBoundary;
        nextBaseSlot = get_next_base(bRtl ? gr_slot_prev_in_segment(baseSlot) : gr_slot_next_in_segment(baseSlot), bRtl);
        nextChar = nextBaseSlot ? gr_cinfo_base(gr_seg_cinfo(pSegment, gr_slot_before(nextBaseSlot))) + mnSegCharOffset : -1;
        nextBoundary = nextBaseSlot ? gr_slot_origin_X(nextBaseSlot) : gr_seg_advance_X(pSegment);

        if (firstChar < mnMinCharPos || firstChar >= mnEndCharPos)
        {
            // handle clipping of diacritic from base
            nextBaseSlot = bRtl ? gr_slot_prev_in_segment(baseSlot) : gr_slot_next_in_segment(baseSlot);
            nextBoundary = nextBaseSlot ? gr_slot_origin_X(nextBaseSlot) : gr_seg_advance_X(pSegment);
            nextChar = nextBaseSlot ? gr_cinfo_base(gr_seg_cinfo(pSegment, gr_slot_before(nextBaseSlot))) + mnSegCharOffset : -1;
            continue;
        }
        // handle reordered clusters. Presumes reordered glyphs have monotonic opposite char index until the cluster base.
        bool isReordered = (nextBaseSlot && ((bRtl != (gr_cinfo_base(gr_seg_cinfo(pSegment, gr_slot_before(nextBaseSlot))) < firstChar - mnSegCharOffset))
                                             || gr_cinfo_base(gr_seg_cinfo(pSegment, gr_slot_before(nextBaseSlot))) == firstChar - mnSegCharOffset));
        if (clusterStart >= 0 && !isReordered)      // we hit the base (end) of a reordered cluster
        {
            int clusterEnd = mvGlyphs.size();
            for (int i = clusterStart; i < clusterEnd; ++i)
                mvGlyph2Char[i] = firstChar;
            if (bRtl)
            {
                for ( ; clusterFirstChar < firstChar; ++clusterFirstChar)
                    if (clusterFirstChar >= mnMinCharPos && clusterFirstChar < mnEndCharPos)
                    {
                        mvChar2BaseGlyph[clusterFirstChar - mnMinCharPos] = clusterStart;  // lowest glyphItem index
                        mvCharDxs[clusterFirstChar - mnMinCharPos] = static_cast<int>(thisBoundary * fScaling) + mnWidth + nDxOffset;
                    }
            }
            else
            {
                for ( ; clusterFirstChar > firstChar; --clusterFirstChar)
                    if (clusterFirstChar < mnEndCharPos && clusterFirstChar >= mnMinCharPos)
                    {
                        mvChar2BaseGlyph[clusterFirstChar - mnMinCharPos] = clusterStart;
                        mvCharDxs[clusterFirstChar - mnMinCharPos] = static_cast<int>(nextBoundary * fScaling) + mnWidth + nDxOffset;
                    }
            }
            clusterStart = -1;
            clusterFirstChar = -1;
        }
        else if (clusterStart < 0 && isReordered) // we hit the start of a reordered cluster
        {
            clusterStart = mvGlyphs.size();
            clusterFirstChar = firstChar;
        }

        int baseGlyph = mvGlyphs.size();
        int scaledGlyphPos = round_to_long(gr_slot_origin_X(baseSlot) * fScaling) + mnWidth + nDxOffset;
        if (mvChar2Glyph[firstChar - mnMinCharPos] == -1 || mvGlyphs[mvChar2Glyph[firstChar - mnMinCharPos]].maLinearPos.X() < scaledGlyphPos)
        {
            mvChar2Glyph[firstChar - mnMinCharPos] = mvGlyphs.size();
            mvCharDxs[firstChar - mnMinCharPos] = static_cast<int>((bRtl ? thisBoundary : nextBoundary) * fScaling) + mnWidth + nDxOffset;
            mvChar2BaseGlyph[firstChar - mnMinCharPos] = baseGlyph;
            mvCharBreaks[firstChar - mnMinCharPos] = gr_cinfo_break_weight(gr_seg_cinfo(pSegment, gr_slot_before(baseSlot)));
        }
        mvGlyph2Char[baseGlyph] = firstChar;
        append(pSegment, rArgs, baseSlot, thisBoundary, nextBoundary, fScaling, nDxOffset, true, firstChar, baseGlyph, bRtl);
        if (thisBoundary < fMinX) fMinX = thisBoundary;
        if (nextBoundary > fMaxX && (nextChar < mnMinCharPos || nextChar >= mnEndCharPos || !isWhite(pStr[nextChar]) || fMaxX <= 0.0f))
            fMaxX = nextBoundary;
    }

    long nXOffset = round_to_long(fMinX * fScaling);
    long nXEnd = round_to_long(fMaxX * fScaling);
    int nCharRequested = minimum<int>(lastCharPos, mnEndCharPos) - mnMinCharPos;
    int firstCharOffset = maximum<int>(mnSegCharOffset, mnMinCharPos) - mnMinCharPos;
    // fill up non-base char dx with cluster widths from previous base glyph
    if (bRtl)
    {
        if (mvCharDxs[nCharRequested-1] == -1)
            mvCharDxs[nCharRequested-1] = nXEnd - nXOffset + mnWidth + nDxOffset;
        else
            mvCharDxs[nCharRequested-1] = nXEnd - mvCharDxs[nCharRequested-1] + 2 * (mnWidth + nDxOffset);
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"%d,%d ", nCharRequested - 1, (int)mvCharDxs[nCharRequested-1]);
#endif
        for (int i = nCharRequested - 2; i >= firstCharOffset; i--)
        {
            if (mvCharDxs[i] == -1) mvCharDxs[i] = mvCharDxs[i+1];
            else mvCharDxs[i] = nXEnd - mvCharDxs[i] + 2 * (mnWidth + nDxOffset);
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"%d,%d ", (int)i, (int)mvCharDxs[i]);
#endif
        }
    }
    else
    {
        if (mvCharDxs[firstCharOffset] == -1)
            mvCharDxs[firstCharOffset] = 0;
        else
            mvCharDxs[firstCharOffset] -= nXOffset;
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"%d,%d ", firstCharOffset, (int)mvCharDxs[firstCharOffset]);
#endif
        for (int i = firstCharOffset + 1; i < nCharRequested; i++)
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
        for (size_t i = origNumGlyphs; i < mvGlyphs.size(); i++)
            mvGlyphs[i].maLinearPos.X() -= nXOffset;
    }
    mnWidth += nXEnd - nXOffset + nDxOffset;
    if (mnWidth < 0)
    {
        // This can happen when there was no base inside the range
        mnWidth = 0;
    }
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(), "fillFrom %" SAL_PRI_SIZET "u glyphs offset %ld width %ld for %d\n", mvGlyphs.size(), nXOffset, mnWidth, nCharRequested);
#endif
}

// append walks an attachment tree, flattening it, and converting it into a
// sequence of GlyphItem objects which we can later manipulate.
float
GraphiteLayout::append(gr_segment *pSeg, ImplLayoutArgs &rArgs,
    const gr_slot * gi, float gOrigin, float nextGlyphOrigin, float scaling, long & rDXOffset,
    bool bIsBase, int baseChar, int baseGlyph, bool bRtl)
{
    assert(gi);
    // assert(gr_slot_before(gi) <= gr_slot_after(gi));
    int firstChar = gr_cinfo_base(gr_seg_cinfo(pSeg, gr_slot_before(gi))) + mnSegCharOffset;
    assert(mvGlyphs.size() < mvGlyph2Char.size());
    if (firstChar < mnMinCharPos || firstChar >= mnEndCharPos)
        return nextGlyphOrigin;

    long glyphId = gr_slot_gid(gi);
    long deltaOffset = 0;
    int scaledGlyphPos = round_to_long(gr_slot_origin_X(gi) * scaling) + mnWidth + rDXOffset;
    int glyphWidth = round_to_long((nextGlyphOrigin - gOrigin) * scaling);
    if (!bIsBase)
    {
        mvChar2BaseGlyph[firstChar - mnMinCharPos] = baseGlyph;
        mvCharDxs[firstChar - mnMinCharPos] = mvCharDxs[baseChar - mnMinCharPos];
        mvCharBreaks[firstChar - mnMinCharPos] = gr_cinfo_break_weight(gr_seg_cinfo(pSeg, gr_slot_before(gi)));
    }

#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"c%d g%ld,X%d W%d nX%f @%d=%d ", firstChar, glyphId,
        scaledGlyphPos, glyphWidth, nextGlyphOrigin * scaling, mvChar2Glyph[firstChar-mnMinCharPos], mvCharDxs[firstChar-mnMinCharPos]);
#endif
    if (glyphId == 0)
    {
        rArgs.NeedFallback(firstChar, bRtl);
        if( (SalLayoutFlags::ForFallback & rArgs.mnFlags ))
        {
            glyphId = GF_DROPPED;
            deltaOffset -= glyphWidth;
            glyphWidth = 0;
        }
    }
    else if(rArgs.mnFlags & SalLayoutFlags::ForFallback)
    {
#ifdef GRLAYOUT_DEBUG
        fprintf(grLog(),"fallback c%d %x in run %d\n", firstChar, rArgs.mrStr[firstChar],
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
    if (gr_slot_attached_to(gi))
        nGlyphFlags |= GlyphItem::IS_DIACRITIC;
    nGlyphFlags |= (bRtl)? GlyphItem::IS_RTL_GLYPH : 0;
    GlyphItem aGlyphItem(mvGlyphs.size(),
        glyphId,
        Point(scaledGlyphPos, round_to_long((-gr_slot_origin_Y(gi) * scaling))),
        nGlyphFlags,
        glyphWidth);
    if (glyphId != static_cast<long>(GF_DROPPED))
        aGlyphItem.mnOrigWidth = round_to_long(gr_slot_advance_X(gi, mpFace, mpFont) * scaling);
    mvGlyphs.push_back(aGlyphItem);

    // update the offset if this glyph was dropped
    rDXOffset += deltaOffset;

    // Recursively append all the attached glyphs.
    float cOrigin = nextGlyphOrigin;
    for (const gr_slot * agi = gr_slot_first_attachment(gi); agi != nullptr; agi = gr_slot_next_sibling_attachment(agi))
        if (!gr_slot_can_insert_before(agi))
            cOrigin = append(pSeg, rArgs, agi, cOrigin, nextGlyphOrigin, scaling, rDXOffset, false, baseChar, baseGlyph, bRtl);

    return cOrigin;
}

// An implementation of the SalLayout interface to enable Graphite enabled fonts to be used.

GraphiteLayout::GraphiteLayout(const gr_face * face, gr_font * font,
                               const grutils::GrFeatureParser * pFeatures) throw()
    : mpFace(face)
    , mpFont(font)
    , mnSegCharOffset(0)
    , mnWidth(0)
    , mfScaling(1.0)
    , mpFeatures(pFeatures)
{

}

GraphiteLayout::~GraphiteLayout() throw()
{
    clear();
    // the features and font are owned by the platform layers
    mpFeatures = nullptr;
    mpFont = nullptr;
}

void GraphiteLayout::clear()
{
    // Destroy the segment and text source from any previous invocation of
    // LayoutText
    mvGlyphs.clear();
    mvCharDxs.clear();
    mvChar2BaseGlyph.clear();
    mvChar2Glyph.clear();
    mvGlyph2Char.clear();

    // Reset the state to the empty state.
    mnWidth = 0;
    // Don't reset the scaling, because it is set before LayoutText
}

// This method shouldn't be called on windows, since it needs the dc reset
bool GraphiteLayout::LayoutText(ImplLayoutArgs & rArgs)
{
    clear();
    bool success = true;
    if (rArgs.mnMinCharPos >= rArgs.mnEndCharPos)
        return success;
    // Set the SalLayouts values to be the initial ones.
    SalLayout::AdjustLayout(rArgs);
    // TODO check if this is needed
    if (mnUnitsPerPixel > 1)
        mfScaling = 1.0f / mnUnitsPerPixel;
    mvCharDxs.assign(mnEndCharPos - mnMinCharPos, -1);
    mvChar2BaseGlyph.assign(mnEndCharPos - mnMinCharPos, -1);
    mvChar2Glyph.assign(mnEndCharPos - mnMinCharPos, -1);
    mvCharBreaks.assign(mnEndCharPos - mnMinCharPos, 0);

#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(), "New Graphite LayoutText\n");
#endif
    success = false;
    const int nLength = rArgs.mrStr.getLength();
    while (true)
    {
        int nBidiMinRunPos, nBidiEndRunPos;
        bool bRightToLeft;
        if (!rArgs.GetNextRun(&nBidiMinRunPos, &nBidiEndRunPos, &bRightToLeft))
            break;

        if (nBidiEndRunPos < mnMinCharPos || nBidiMinRunPos >= mnEndCharPos)
            continue;

        if (nBidiMinRunPos == mnMinCharPos)
            nBidiMinRunPos = maximum<int>(0, nBidiMinRunPos - EXTRA_CONTEXT_LENGTH);
        if (nBidiEndRunPos == mnEndCharPos)
            nBidiEndRunPos = minimum<int>(nLength, nBidiEndRunPos + EXTRA_CONTEXT_LENGTH);
        const sal_Unicode *pStr = rArgs.mrStr.getStr();
        size_t numchars = gr_count_unicode_characters(gr_utf16, pStr + nBidiMinRunPos,
                 pStr + nBidiEndRunPos, nullptr);
        gr_segment * pSegment = gr_make_seg(mpFont, mpFace, 0, mpFeatures ? mpFeatures->values() : nullptr,
                                gr_utf16, pStr + nBidiMinRunPos, numchars, 2 | int(bRightToLeft));

        if (pSegment != nullptr)
        {
            success = true;
            mnSegCharOffset = nBidiMinRunPos;
#ifdef GRLAYOUT_DEBUG
            fprintf(grLog(),"Gr::LayoutText %d-%d, context %d-%d, len %d, numchars %" SAL_PRI_SIZET "u, rtl %d scaling %f:",
                rArgs.mnMinCharPos, rArgs.mnEndCharPos,
                nBidiMinRunPos, nBidiEndRunPos,
                nLength, numchars, bRightToLeft, mfScaling);
            for (int i = mnSegCharOffset; i < nBidiEndRunPos; ++i)
                fprintf(grLog(), " %04X", rArgs.mrStr[i]);
            fprintf(grLog(), "\n");
#endif
            fillFrom(pSegment, rArgs, mfScaling, bRightToLeft, nBidiEndRunPos);
            gr_seg_destroy(pSegment);
        }
    }
    return success;
}

sal_Int32 GraphiteLayout::GetTextBreak(DeviceCoordinate maxmnWidth, DeviceCoordinate char_extra, int factor) const
{
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"Gr::GetTextBreak c[%d-%d) maxWidth %ld char extra %ld factor %d\n",
        mnMinCharPos, mnEndCharPos, maxmnWidth, char_extra, factor);
#endif

    // return quickly if this segment is narrower than the target width
    if (maxmnWidth > mnWidth * factor + char_extra * (mnEndCharPos - mnMinCharPos - 1))
        return -1;

    DeviceCoordinate nWidth = mvCharDxs[0] * factor;
    long wLastBreak = 0;
    int nLastBreak = -1;
    int nEmergency = -1;
    for (size_t i = 1; i < mvCharDxs.size(); i++)
    {
        nWidth += char_extra;
        if (nWidth > maxmnWidth) break;
        int gi = mvChar2BaseGlyph[i];
        if (gi != -1)
        {
            if (!mvGlyphs[gi].IsDiacritic() &&
                (mvCharBreaks[i] > -35 || (mvCharBreaks[i-1] > 0 && mvCharBreaks[i-1] < 35)) &&
                (mvCharBreaks[i-1] < 35 || (mvCharBreaks[i] < 0 && mvCharBreaks[i] > -35)))
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

    if (nBreak > mnEndCharPos)
        nBreak = -1;
    else if (nBreak < mnMinCharPos)
        nBreak = mnMinCharPos;
    return nBreak;
}

DeviceCoordinate GraphiteLayout::FillDXArray( DeviceCoordinate* pDXArray ) const
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
                mvGlyphs[mvChar2BaseGlyph[i]].maGlyphId == GF_DROPPED)
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
            fprintf(grLog(),"%d,%d,%ld ", (int)i, (int)mvCharDxs[i], pDXArray[i]);
#endif
        }
    }
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"FillDXArray %d-%d=%g\n", mnMinCharPos, mnEndCharPos, (double)mnWidth);
#endif
    return mnWidth;
}

void  GraphiteLayout::AdjustLayout(ImplLayoutArgs& rArgs)
{
    SalLayout::AdjustLayout(rArgs);
    if(rArgs.mpDXArray && mvGlyphs.size())
    {
        std::vector<int> vDeltaWidths(mvGlyphs.size(), 0);
        ApplyDXArray(rArgs, vDeltaWidths);

        if( (mnLayoutFlags & SalLayoutFlags::BiDiRtl) &&
           !(rArgs.mnFlags & SalLayoutFlags::ForFallback) )
        {
            // check if this is a kashida script
            bool bKashidaScript = false;
            for (int i = rArgs.mnMinCharPos; i < rArgs.mnEndCharPos; i++)
            {
                UErrorCode aStatus = U_ZERO_ERROR;
                UScriptCode scriptCode = uscript_getScript(rArgs.mrStr[i], &aStatus);
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
            if (mvGlyphs[j].IsClusterStart() && !mvGlyphs[j].IsDiacritic())
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
                if (mvGlyphs[i].IsClusterStart() && !mvGlyphs[i].IsDiacritic())
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

unsigned int GraphiteLayout::ScanFwdForChar(int &findChar, bool fallback) const
{
    int res = mvChar2Glyph[findChar - mnMinCharPos];
    int done = 3;
    while (res == -1 && --done)
    {
        if (fallback)
        {
            for (++findChar; findChar - mnMinCharPos < int(mvChar2Glyph.size()); ++findChar)
                if ((res = mvChar2Glyph[findChar - mnMinCharPos]) != -1)
                    return res;
        }
        else
        {
            for (--findChar; findChar >= mnMinCharPos; --findChar)
                if ((res = mvChar2Glyph[findChar - mnMinCharPos]) != -1)
                    return res;
        }
        fallback = !fallback;
    }
    return unsigned(res);
}

void GraphiteLayout::ApplyDXArray(ImplLayoutArgs &args, std::vector<int> & rDeltaWidth)
{
    bool bRtl(mnLayoutFlags & SalLayoutFlags::BiDiRtl);
    int startChar = args.mnMinCharPos < mnMinCharPos ? mnMinCharPos : args.mnMinCharPos;
    int endChar = args.mnEndCharPos >= mnEndCharPos ? mnEndCharPos - 1 : args.mnEndCharPos;
    unsigned int startGi = ScanFwdForChar(startChar, !bRtl);
    unsigned int endGi = ScanFwdForChar(endChar, bRtl);
    int nChars = endChar - startChar + 1;
    if (nChars <= 0) return;
    if (startGi > endGi)
    {
        unsigned int temp = endGi;
        endGi = startGi;
        startGi = temp;
    }
    ++endGi;

#ifdef GRLAYOUT_DEBUG
    for (size_t iDx = 0; iDx < mvCharDxs.size(); iDx++)
         fprintf(grLog(),"%d,%d,%ld ", (int)iDx, (int)mvCharDxs[iDx], args.mpDXArray[iDx]);
    fprintf(grLog(),"ApplyDx %d-%d=%d-%d\n", startChar, endChar, startGi, endGi);
#endif

    for (unsigned int i = startGi; i < endGi; ++i)
    {
        // calculate visual cluster bounds
        int firstChar = mvGlyph2Char[i];
        unsigned int nBaseGlyph = mvChar2BaseGlyph[firstChar - mnMinCharPos];
        while (nBaseGlyph == ~0U && i < endGi)
        {
            ++i;
            firstChar = mvGlyph2Char[i];
            nBaseGlyph = unsigned(mvChar2BaseGlyph[firstChar - mnMinCharPos]);
        }
        int lastChar = firstChar;
        unsigned int nLastGlyph = i;
        // firstGlyph = i
        for ( ; nLastGlyph < endGi; nLastGlyph++)
        {
            int thisChar = mvGlyph2Char[nLastGlyph];
            if (thisChar == -1) continue;
            if (unsigned(mvChar2BaseGlyph[thisChar - mnMinCharPos]) != nBaseGlyph)
            {
                if (!mvGlyphs[nLastGlyph].IsDiacritic())
                    break;
                else
                    nBaseGlyph = mvChar2BaseGlyph[thisChar - mnMinCharPos];
            }
            if (thisChar > lastChar) lastChar = thisChar;
            if (thisChar < firstChar) firstChar = thisChar;
        }

        // calculate visual cluster widths
        if (lastChar > args.mnEndCharPos) lastChar = args.mnEndCharPos;
        if (firstChar < args.mnMinCharPos) firstChar = args.mnMinCharPos;
        long nNewClusterWidth = args.mpDXArray[lastChar - args.mnMinCharPos];
        long nOrigClusterWidth = mvCharDxs[lastChar - mnMinCharPos];
        long nDGlyphOrigin = 0;
        if (firstChar >= args.mnMinCharPos)
        {
            nNewClusterWidth -= args.mpDXArray[firstChar - args.mnMinCharPos];
            nOrigClusterWidth -= mvCharDxs[firstChar - mnMinCharPos];
            nDGlyphOrigin = args.mpDXArray[firstChar - args.mnMinCharPos]
                                - mvCharDxs[firstChar - mnMinCharPos];
        }

        // update visual cluster
        long nDWidth = nNewClusterWidth - nOrigClusterWidth;
        if (firstChar >= args.mnMinCharPos)
            for (int n = firstChar; n <= lastChar; ++n)
                if (mvCharDxs[n - mnMinCharPos] != -1)
                    mvCharDxs[n - mnMinCharPos] += nDWidth + nDGlyphOrigin;
        for (unsigned int n = i; n < nLastGlyph; n++)
            mvGlyphs[n].maLinearPos.X() += (nDGlyphOrigin + nDWidth) * (bRtl ? -1 : 1);

        rDeltaWidth[nBaseGlyph] = nDWidth;
#ifdef GRLAYOUT_DEBUG
        fprintf(grLog(),"c%d=%d g%d-%d dW%ld-%ld=%ld dX%ld x%ld @%d=%d\n", firstChar, lastChar, i, nLastGlyph, nNewClusterWidth, nOrigClusterWidth, nDWidth, nDGlyphOrigin, mvGlyphs[i].maLinearPos.X(), mvCharDxs[lastChar - mnMinCharPos], args.mpDXArray[lastChar - args.mnMinCharPos]);
#endif
        i = nLastGlyph - 1;
        if (i >= endGi - 1)
            mnWidth += nDGlyphOrigin + nDWidth;
    }
    // Update the dx vector with the new values.
    std::copy(args.mpDXArray, args.mpDXArray + nChars,
      mvCharDxs.begin() + (args.mnMinCharPos - mnMinCharPos));
#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"ApplyDx %ld(%ld)\n", args.mpDXArray[nChars - 1], mnWidth);
#endif
    if (bRtl)
    {
        int diff = mvGlyphs[0].maLinearPos.X();
        for (size_t i = 0; i < mvGlyphs.size(); ++i)
            mvGlyphs[i].maLinearPos.X() -= diff;
    }
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
        if( IsSpacingGlyph( (*i).maGlyphId ) )
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
        printf("inserting %d kashidas at %u\n", nKashidaCount, (*i).maGlyphId);
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

void GraphiteLayout::GetCaretPositions( int nArraySize, long* pCaretXArray ) const
{
    // For each character except the last discover the caret positions
    // immediately before and after that character.
    // This is used for underlines in the GUI amongst other things.
    // It may be used from MultiSalLayout, in which case it must take into account
    // glyphs that have been moved.
    std::fill(pCaretXArray, pCaretXArray + nArraySize, -1);
    // the layout method doesn't modify the layout even though it isn't
    // const in the interface
    bool bRtl(mnLayoutFlags & SalLayoutFlags::BiDiRtl);//const_cast<GraphiteLayout*>(this)->maLayout.rightToLeft();
    int prevBase = -1;
    long prevClusterWidth = 0;
    for (int i = 0, nCharSlot = 0; i < nArraySize && nCharSlot < static_cast<int>(mvCharDxs.size()); ++nCharSlot, i+=2)
    {
        if (mvChar2BaseGlyph[nCharSlot] != -1)
        {
            int nChar2Base = mvChar2BaseGlyph[nCharSlot];
            assert((nChar2Base > -1) && (nChar2Base < (signed)mvGlyphs.size()));
            GlyphItem gi = mvGlyphs[nChar2Base];
            if (gi.maGlyphId == GF_DROPPED)
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
        fprintf(grLog(),"%d,%ld-%ld\t", nCharSlot, pCaretXArray[i], pCaretXArray[i+1]);
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
                                   ::Point & aPosOut, int &glyph_slot, DeviceCoordinate* glyph_adv, int *char_index,
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
          ((mvGlyphs.begin() + glyph_slot)->maGlyphId == GF_DROPPED);
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
     *glyph_out++ = glyph_itr->maGlyphId;

     // Find the actual advance - this must be correct if called from
     // MultiSalLayout::AdjustLayout which requests one glyph at a time.
     const long nGlyphAdvance = (glyph_slot == static_cast<int>(mvGlyphs.size()))?
          glyph_itr->mnNewWidth :
          ((glyph_itr+1)->maLinearPos.X() - glyph_itr->maLinearPos.X());

#ifdef GRLAYOUT_DEBUG
    fprintf(grLog(),"GetNextGlyphs g%d gid%d c%d x%ld,%ld adv%ld, pos %ld,%ld\n",
            glyph_slot - 1, glyph_itr->maGlyphId,
            mvGlyph2Char[glyph_slot-1], glyph_itr->maLinearPos.X(), glyph_itr->maLinearPos.Y(), (long)nGlyphAdvance,
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
     if (glyph_itr->maGlyphId == GF_DROPPED)
       break;
  }
  int numGlyphs = glyph_slot - glyph_slot_begin;
  // move the next glyph_slot to a glyph that hasn't been dropped
  while (glyph_slot < static_cast<int>(mvGlyphs.size()) &&
         (mvGlyphs.begin() + glyph_slot)->maGlyphId == GF_DROPPED)
         ++glyph_slot;
  return numGlyphs;
}

void GraphiteLayout::MoveGlyph( int nGlyphIndex, long nNewPos )
{
    // TODO it might be better to actually implement simplify properly, but this
    // needs to be done carefully so the glyph/char maps are maintained
    // If a glyph has been dropped then it wasn't returned by GetNextGlyphs, so
    // the index here may be wrong
    while ((mvGlyphs[nGlyphIndex].maGlyphId == GF_DROPPED) &&
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
    glyph.maGlyphId = GF_DROPPED;
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
      if (gi->maGlyphId == dropMarker)
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
