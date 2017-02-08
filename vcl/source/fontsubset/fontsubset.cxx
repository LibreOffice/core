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

#include <tools/debug.hxx>

#include <osl/diagnose.h>

#include <fontsubset.hxx>
#include <sft.hxx>

FontSubsetInfo::FontSubsetInfo()
    : m_nAscent( 0)
    , m_nDescent( 0)
    , m_nCapHeight( 0)
    , m_nFontType( FontType::NO_FONT)
    , mpInFontBytes( nullptr)
    , mnInByteLength( 0)
    , meInFontType( FontType::NO_FONT)
    , mpSftTTFont( nullptr)
    , mnReqFontTypeMask( FontType::NO_FONT )
    , mpOutFile(nullptr)
    , mpReqFontName(nullptr)
    , mpReqGlyphIds(nullptr)
    , mpReqEncodedIds(nullptr)
    , mnReqGlyphCount(0)
{
}

FontSubsetInfo::~FontSubsetInfo()
{
}

// prepare subsetting for fonts where the input font file is mapped
bool FontSubsetInfo::LoadFont(
    FontType eInFontType,
    const unsigned char* pInFontBytes, int nInByteLength)
{
    SAL_WARN_IF( (mpSftTTFont != nullptr), "vcl", "Subset from SFT and from mapped font-file requested");
    meInFontType = eInFontType;
    mpInFontBytes = pInFontBytes;
    mnInByteLength = nInByteLength;
    return (mnInByteLength > 0);
}

// prepare subsetting for fonts that are known to the SFT-parser
bool FontSubsetInfo::LoadFont( vcl::TrueTypeFont* pSftTTFont )
{
    SAL_WARN_IF( (mpInFontBytes != nullptr), "vcl", "Subset from SFT and from mapped font-file requested");
    mpSftTTFont = pSftTTFont;
    meInFontType = FontType::ANY_SFNT;
    return (mpSftTTFont == nullptr);
}

bool FontSubsetInfo::CreateFontSubset(
    FontType nReqFontTypeMask,
    FILE* pOutFile, const char* pReqFontName,
    const sal_GlyphId* pReqGlyphIds, const sal_uInt8* pReqEncodedIds, int nReqGlyphCount,
    sal_Int32* pOutGlyphWidths)
{
    // prepare request details needed by all underlying subsetters
    mnReqFontTypeMask = nReqFontTypeMask;
    mpOutFile       = pOutFile;
    mpReqFontName   = pReqFontName;
    mpReqGlyphIds   = pReqGlyphIds;
    mpReqEncodedIds = pReqEncodedIds;
    mnReqGlyphCount = nReqGlyphCount;

    // TODO: move the glyphid/encid/notdef reshuffling from the callers to here

    // dispatch to underlying subsetters
    bool bOK = false;

    // TODO: better match available input-type to possible subset-types
    switch( meInFontType) {
    case FontType::SFNT_TTF:
    case FontType::SFNT_CFF:
    case FontType::ANY_SFNT:
        bOK = CreateFontSubsetFromSfnt( pOutGlyphWidths);
        break;
    case FontType::CFF_FONT:
        bOK = CreateFontSubsetFromCff( pOutGlyphWidths);
        break;
    case FontType::TYPE1_PFA:
    case FontType::TYPE1_PFB:
    case FontType::ANY_TYPE1:
        bOK = CreateFontSubsetFromType1( pOutGlyphWidths);
        break;
        // fall through
    case FontType::NO_FONT:
        // fall through
    default:
        OSL_FAIL( "unhandled type in CreateFontSubset()");
        break;
    }

    return bOK;
}

// TODO: move function to sft.cxx to replace dummy implementation
bool FontSubsetInfo::CreateFontSubsetFromSfnt( sal_Int32* pOutGlyphWidths )
{
    // handle SFNT_CFF fonts
    int nCffLength = 0;
    const sal_uInt8* pCffBytes = nullptr;
    if( GetSfntTable( mpSftTTFont, O_CFF, &pCffBytes, &nCffLength))
    {
        LoadFont( FontType::CFF_FONT, pCffBytes, nCffLength);
        const bool bOK = CreateFontSubsetFromCff( pOutGlyphWidths);
        return bOK;
    }

    // handle SFNT_TTF fonts
    // by forwarding the subset request to AG's sft subsetter
#if 1 // TODO: remove conversion tp 16bit glyphids when sft-subsetter has been updated
    std::vector<sal_uInt16> aShortGlyphIds;
    aShortGlyphIds.reserve(mnReqGlyphCount);
    for (int i = 0; i < mnReqGlyphCount; ++i)
        aShortGlyphIds.push_back((sal_uInt16)mpReqGlyphIds[i]);
    // remove const_cast when sft-subsetter is const-correct
    sal_uInt8* pEncArray = const_cast<sal_uInt8*>( mpReqEncodedIds );
#endif
    int nSFTErr = vcl::SF_BADARG;
    if( mnReqFontTypeMask & FontType::TYPE42_FONT )
    {
        nSFTErr = CreateT42FromTTGlyphs( mpSftTTFont, mpOutFile, mpReqFontName,
            aShortGlyphIds.data(), pEncArray, mnReqGlyphCount );
    }
    else if( mnReqFontTypeMask & FontType::TYPE3_FONT )
    {
        nSFTErr = CreateT3FromTTGlyphs( mpSftTTFont, mpOutFile, mpReqFontName,
            aShortGlyphIds.data(), pEncArray, mnReqGlyphCount,
                    0 /* 0 = horizontal, 1 = vertical */ );
    }
    else if( mnReqFontTypeMask & FontType::SFNT_TTF )
    {
        // TODO: use CreateTTFromTTGlyphs()
        // TODO: move functionality from callers here
    }

    return (nSFTErr != vcl::SF_OK);
}

// TODO: replace dummy implementation
bool FontSubsetInfo::CreateFontSubsetFromType1( sal_Int32* pOutGlyphWidths)
{
    (void)pOutGlyphWidths;
    fprintf(stderr,"CreateFontSubsetFromType1: replace dummy implementation\n");
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
