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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <vcl/fontsubset.hxx>
#include <sft.hxx>
#include <tools/debug.hxx>

// ====================================================================

FontSubsetInfo::FontSubsetInfo()
:   m_nAscent( 0)
,   m_nDescent( 0)
,   m_nCapHeight( 0)
,   m_nFontType( FontSubsetInfo::NO_FONT)
,   mpInFontBytes( NULL)
,   mnInByteLength( 0)
,   meInFontType( FontSubsetInfo::NO_FONT)
,   mpSftTTFont( NULL)
{}

// --------------------------------------------------------------------

FontSubsetInfo::~FontSubsetInfo()
{}

// --------------------------------------------------------------------

// prepare subsetting for fonts where the input font file is mapped
bool FontSubsetInfo::LoadFont(
    FontSubsetInfo::FontType eInFontType,
    const unsigned char* pInFontBytes, int nInByteLength)
{
    DBG_ASSERT( (mpSftTTFont == NULL), "Subset from SFT and from mapped font-file requested");
    meInFontType = eInFontType;
    mpInFontBytes = pInFontBytes;
    mnInByteLength = nInByteLength;
    return (mnInByteLength > 0);
}

// --------------------------------------------------------------------

// prepare subsetting for fonts that are known to the SFT-parser
bool FontSubsetInfo::LoadFont( vcl::_TrueTypeFont* pSftTTFont )
{
    DBG_ASSERT( (mpInFontBytes == NULL), "Subset from SFT and from mapped font-file requested");
    mpSftTTFont = pSftTTFont;
    meInFontType = ANY_SFNT;
    return (mpSftTTFont == NULL);
}

// --------------------------------------------------------------------

bool FontSubsetInfo::CreateFontSubset(
    int nReqFontTypeMask,
    FILE* pOutFile, const char* pReqFontName,
    const long* pReqGlyphIds, const sal_uInt8* pReqEncodedIds, int nReqGlyphCount,
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
    case SFNT_TTF:
    case SFNT_CFF:
    case ANY_SFNT:
        bOK = CreateFontSubsetFromSfnt( pOutGlyphWidths);
        break;
    case CFF_FONT:
        bOK = CreateFontSubsetFromCff( pOutGlyphWidths);
        break;
    case TYPE1_PFA:
    case TYPE1_PFB:
    case ANY_TYPE1:
        bOK = CreateFontSubsetFromType1( pOutGlyphWidths);
        break;
        // fall trough
    case NO_FONT:
        // fall trough
    default:
        OSL_FAIL( "unhandled type in CreateFontSubset()");
        break;
    }

    return bOK;
}

// --------------------------------------------------------------------

// TODO: move function to sft.cxx to replace dummy implementation
bool FontSubsetInfo::CreateFontSubsetFromSfnt( sal_Int32* pOutGlyphWidths )
{
    // handle SFNT_CFF fonts
    int nCffLength = 0;
    const sal_uInt8* pCffBytes = NULL;
    if( GetSfntTable( mpSftTTFont, O_CFF, &pCffBytes, &nCffLength))
    {
        LoadFont( CFF_FONT, pCffBytes, nCffLength);
        const bool bOK = CreateFontSubsetFromCff( pOutGlyphWidths);
        return bOK;
    }

    // handle SFNT_TTF fonts
    // by forwarding the subset request to AG's sft subsetter
#if 1 // TODO: remove conversion tp 16bit glyphids when sft-subsetter has been updated
    sal_uInt16 aShortGlyphIds[256];
    for( int i = 0; i < mnReqGlyphCount; ++i)
        aShortGlyphIds[i] = (sal_uInt16)mpReqGlyphIds[i];
    // remove const_cast when sft-subsetter is const-correct
    sal_uInt8* pEncArray = const_cast<sal_uInt8*>( mpReqEncodedIds );
#endif
    int nSFTErr = vcl::SF_BADARG;
    if( (mnReqFontTypeMask & TYPE42_FONT) != 0 )
    {
        nSFTErr = CreateT42FromTTGlyphs( mpSftTTFont, mpOutFile, mpReqFontName,
            aShortGlyphIds, pEncArray, mnReqGlyphCount );
    }
    else if( (mnReqFontTypeMask & TYPE3_FONT) != 0 )
    {
        nSFTErr = CreateT3FromTTGlyphs( mpSftTTFont, mpOutFile, mpReqFontName,
            aShortGlyphIds, pEncArray, mnReqGlyphCount,
                    0 /* 0 = horizontal, 1 = vertical */ );
    }
    else if( (mnReqFontTypeMask & SFNT_TTF) != 0 )
    {
        // TODO: use CreateTTFromTTGlyphs()
        // TODO: move functionality from callers here
    }

    return (nSFTErr != vcl::SF_OK);
}

// --------------------------------------------------------------------

// TODO: replace dummy implementation
bool FontSubsetInfo::CreateFontSubsetFromType1( sal_Int32* pOutGlyphWidths)
{
    (void)pOutGlyphWidths;
    fprintf(stderr,"CreateFontSubsetFromType1: replace dummy implementation\n");
    return false;
}

// ====================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
