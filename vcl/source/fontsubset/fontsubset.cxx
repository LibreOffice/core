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


#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <fontsubset.hxx>
#include <sft.hxx>

FontSubsetInfo::FontSubsetInfo()
    : m_nAscent( 0)
    , m_nDescent( 0)
    , m_nCapHeight( 0)
    , m_nFontType( FontType::NO_FONT)
    , m_bFilled(false)
    , mpInFontBytes( nullptr)
    , mnInByteLength( 0)
    , meInFontType( FontType::NO_FONT)
    , mnReqFontTypeMask( FontType::NO_FONT )
    , mpOutFile(nullptr)
    , mpReqGlyphIds(nullptr)
    , mpReqEncodedIds(nullptr)
    , mnReqGlyphCount(0)
{
}

FontSubsetInfo::~FontSubsetInfo()
{
}

// prepare subsetting for fonts where the input font file is mapped
void FontSubsetInfo::LoadFont(
    FontType eInFontType,
    const unsigned char* pInFontBytes, int nInByteLength)
{
    meInFontType = eInFontType;
    mpInFontBytes = pInFontBytes;
    mnInByteLength = nInByteLength;
}

bool FontSubsetInfo::CreateFontSubset(
    FontType nReqFontTypeMask,
    SvStream* pOutFile,
    const sal_GlyphId* pReqGlyphIds, const sal_uInt8* pReqEncodedIds, int nReqGlyphCount)
{
    // prepare request details needed by all underlying subsetters
    mnReqFontTypeMask = nReqFontTypeMask;
    mpOutFile       = pOutFile;
    mpReqGlyphIds   = pReqGlyphIds;
    mpReqEncodedIds = pReqEncodedIds;
    mnReqGlyphCount = nReqGlyphCount;
    maReqFontName = m_aPSName.toUtf8();

    // TODO: move the glyphid/encid/notdef reshuffling from the callers to here

    // dispatch to underlying subsetters
    bool bOK = false;

    // TODO: better match available input-type to possible subset-types
    switch( meInFontType) {
    case FontType::CFF_FONT:
        bOK = CreateFontSubsetFromCff();
        break;
    default:
        OSL_FAIL( "unhandled type in CreateFontSubset()");
        break;
    }

    return bOK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
