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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

// Header files
//
// Standard Library
#include <string>
#include <cassert>
#include "graphite_textsrc.hxx"
#include <graphite_features.hxx>

// class TextSourceAdaptor implementation.
//
TextSourceAdaptor::~TextSourceAdaptor()
{
    delete mpFeatures;
}

gr::UtfType TextSourceAdaptor::utfEncodingForm() {
    return gr::kutf16;
}


size_t TextSourceAdaptor::getLength()
{
    return maLayoutArgs.mnLength;
}


size_t  TextSourceAdaptor::fetch(gr::toffset, size_t, gr::utf32 *)
{
    assert(false);
    return 0;
}


size_t  TextSourceAdaptor::fetch(gr::toffset offset, size_t char_count, gr::utf16 * char_buffer)
{
  assert(char_buffer);

  size_t copy_count =  std::min(size_t(maLayoutArgs.mnLength), char_count);
  std::copy(maLayoutArgs.mpStr + offset, maLayoutArgs.mpStr + offset + copy_count, char_buffer);

  return copy_count;
}


size_t TextSourceAdaptor::fetch(gr::toffset, size_t, gr::utf8  *)
{
    assert(false);
    return 0;
}


inline void TextSourceAdaptor::getCharProperties(const int nCharIdx, int & min, int & lim, size_t & depth)
{
    maLayoutArgs.ResetPos();
    bool rtl = maLayoutArgs.mnFlags & SAL_LAYOUT_BIDI_RTL;
    for(depth = ((rtl)? 1:0); maLayoutArgs.maRuns.GetRun(&min, &lim, &rtl); maLayoutArgs.maRuns.NextRun())
    {
        if (min > nCharIdx)
            break;
        // Only increase the depth when a change of direction occurs.
        depth += int(rtl ^ bool(depth & 0x1));
        if (min <= nCharIdx && nCharIdx < lim)
            break;
    }
    // If there is no run for this position increment the depth, but don't
    // change if this is out of bounds context
    if (lim > 0 && nCharIdx >= lim && nCharIdx < maLayoutArgs.mnEndCharPos)
        depth++;
}


bool TextSourceAdaptor::getRightToLeft(gr::toffset nCharIdx)
{
    size_t depth;
    int min, lim = 0;
    getCharProperties(nCharIdx, min, lim, depth);
    //printf("getRtl %d,%x=%d\n", nCharIdx, maLayoutArgs.mpStr[nCharIdx], depth & 0x1);
    return depth & 0x1;
}


unsigned int TextSourceAdaptor::getDirectionDepth(gr::toffset nCharIdx)
{
    size_t depth;
    int min, lim;
    getCharProperties(nCharIdx, min, lim, depth);
    //printf("getDirectionDepth %d,%x=%d\n", nCharIdx, maLayoutArgs.mpStr[nCharIdx], depth);
    return depth;
}


float TextSourceAdaptor::getVerticalOffset(gr::toffset)
{
    return 0.0f;    //TODO: Implement correctly
}

gr::isocode TextSourceAdaptor::getLanguage(gr::toffset)
{
    if (mpFeatures && mpFeatures->hasLanguage())
        return mpFeatures->getLanguage();
    gr::isocode unknown = {{0,0,0,0}};
    return unknown;
}

ext_std::pair<gr::toffset, gr::toffset> TextSourceAdaptor::propertyRange(gr::toffset nCharIdx)
{

    if (nCharIdx < unsigned(maLayoutArgs.mnMinCharPos))
        return ext_std::make_pair(0, maLayoutArgs.mnMinCharPos);

    if (nCharIdx < mnEnd)
        return ext_std::make_pair(maLayoutArgs.mnMinCharPos, mnEnd);

    return ext_std::make_pair(mnEnd, maLayoutArgs.mnLength);
}

size_t TextSourceAdaptor::getFontFeatures(gr::toffset, gr::FeatureSetting * settings)
{
    if (mpFeatures) return mpFeatures->getFontFeatures(settings);
    return 0;
}


bool TextSourceAdaptor::sameSegment(gr::toffset char_idx1, gr::toffset char_idx2)
{
    const ext_std::pair<gr::toffset, gr::toffset>
    range1 = propertyRange(char_idx1),
    range2 = propertyRange(char_idx2);

    return range1 == range2;
}

void TextSourceAdaptor::setFeatures(const grutils::GrFeatureParser * pFeatures)
{
    mpFeatures = new grutils::GrFeatureParser(*pFeatures);
}
