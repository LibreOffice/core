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

#include "glyphset.hxx"

#include <sft.hxx>

#include <unx/printergfx.hxx>
#include <fontsubset.hxx>
#include <unx/fontmanager.hxx>

#include <tools/gen.hxx>

#include <osl/thread.h>

#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>

#include <unotools/tempfile.hxx>

#include <algorithm>

using namespace vcl;
using namespace psp;

GlyphSet::GlyphSet (sal_Int32 nFontID, bool bVertical)
        : mnFontID (nFontID),
          mbVertical (bVertical)
{
    PrintFontManager &rMgr = PrintFontManager::get();
    maBaseName          = OUStringToOString (rMgr.getPSName(mnFontID),
                                           RTL_TEXTENCODING_ASCII_US);
}

void
GlyphSet::GetGlyphID (
                      sal_GlyphId nGlyph,
                      unsigned char* nOutGlyphID,
                      sal_Int32* nOutGlyphSetID
                     )
{
    if (!LookupGlyphID(nGlyph, nOutGlyphID, nOutGlyphSetID))
        AddGlyphID(nGlyph, nOutGlyphID, nOutGlyphSetID);
}

bool
GlyphSet::LookupGlyphID (
                        sal_GlyphId nGlyph,
                        unsigned char* nOutGlyphID,
                        sal_Int32* nOutGlyphSetID
                        )
{
    sal_Int32             nGlyphSetID = 1;

    // loop through all the font subsets
    for (auto const& glyph : maGlyphList)
    {
        // check every subset if it contains the queried unicode char
        glyph_map_t::const_iterator aGlyph = glyph.find (nGlyph);
        if (aGlyph != glyph.end())
        {
            // success: found the glyph id, return the mapped glyphid and the glyphsetid
            *nOutGlyphSetID = nGlyphSetID;
            *nOutGlyphID    = aGlyph->second;
            return true;
        }
        ++nGlyphSetID;
    }

    *nOutGlyphSetID = -1;
    *nOutGlyphID    =  0;
    return false;
}

void
GlyphSet::AddNotdef (glyph_map_t &rGlyphMap)
{
    if (rGlyphMap.empty())
        rGlyphMap[0] = 0;
}

void
GlyphSet::AddGlyphID (
                     sal_GlyphId nGlyph,
                     unsigned char* nOutGlyphID,
                     sal_Int32* nOutGlyphSetID
                     )
{
    // create an empty glyphmap that is reserved for unencoded symbol glyphs,
    // and a second map that takes any other
    if (maGlyphList.empty())
    {
        glyph_map_t aMap, aMapp;

        maGlyphList.push_back (aMap);
        maGlyphList.push_back (aMapp);
    }
    // if the last map is full, create a new one
    if (maGlyphList.back().size() == 255)
    {
        glyph_map_t aMap;
        maGlyphList.push_back (aMap);
    }

    glyph_map_t& aGlyphSet = maGlyphList.back();
    AddNotdef (aGlyphSet);

    int nSize         = aGlyphSet.size();

    aGlyphSet [nGlyph] = nSize;
    *nOutGlyphSetID   = maGlyphList.size();
    *nOutGlyphID      = aGlyphSet [nGlyph];
}

OString
GlyphSet::GetGlyphSetName (sal_Int32 nGlyphSetID)
{
    OStringBuffer aSetName( maBaseName.getLength() + 32 );
    aSetName.append( maBaseName );
    aSetName.append( "FID" );
    aSetName.append( mnFontID );
    aSetName.append( mbVertical ? "VGSet" : "HGSet" );
    aSetName.append( nGlyphSetID );
    return aSetName.makeStringAndClear();
}

OString
GlyphSet::GetReencodedFontName (rtl_TextEncoding nEnc, std::string_view rFontName)
{
    if (   nEnc == RTL_TEXTENCODING_MS_1252
        || nEnc == RTL_TEXTENCODING_ISO_8859_1)
    {
        return OString::Concat(rFontName) + "-iso1252";
    }
    else
    if (nEnc >= RTL_TEXTENCODING_USER_START && nEnc <= RTL_TEXTENCODING_USER_END)
    {
        return OString::Concat(rFontName)
               + "-enc"
               + OString::number ((nEnc - RTL_TEXTENCODING_USER_START));
    }
    else
    {
        return OString();
    }
}

void GlyphSet::DrawGlyph(PrinterGfx& rGfx,
                         const Point& rPoint,
                         const sal_GlyphId nGlyphId)
{
    unsigned char nGlyphID;
    sal_Int32 nGlyphSetID;

    // convert to font glyph id and font subset
    GetGlyphID (nGlyphId, &nGlyphID, &nGlyphSetID);

    OString aGlyphSetName = GetGlyphSetName(nGlyphSetID);

    rGfx.PSSetFont  (aGlyphSetName, RTL_TEXTENCODING_DONTKNOW);
    rGfx.PSMoveTo   (rPoint);
    rGfx.PSShowGlyph(nGlyphID);
}

namespace {

struct EncEntry
{
    unsigned char  aEnc;
    tools::Long       aGID;

    EncEntry() : aEnc( 0 ), aGID( 0 ) {}

    bool operator<( const EncEntry& rRight ) const
    { return aEnc < rRight.aEnc; }
};

}

static void CreatePSUploadableFont( TrueTypeFont* pSrcFont, FILE* pTmpFile,
    const char* pGlyphSetName, int nGlyphCount,
    /*const*/ const sal_uInt16* pRequestedGlyphs, /*const*/ const unsigned char* pEncoding,
    bool bAllowType42 )
{
    // match the font-subset to the printer capabilities
     // TODO: allow CFF for capable printers
    FontType nTargetMask = FontType::TYPE1_PFA | FontType::TYPE3_FONT;
    if( bAllowType42 )
        nTargetMask |= FontType::TYPE42_FONT;

    std::vector< EncEntry > aSorted( nGlyphCount, EncEntry() );
    for( int i = 0; i < nGlyphCount; i++ )
    {
        aSorted[i].aEnc = pEncoding[i];
        aSorted[i].aGID = pRequestedGlyphs[i];
    }

    std::stable_sort( aSorted.begin(), aSorted.end() );

    std::vector< unsigned char > aEncoding( nGlyphCount );
    std::vector< sal_GlyphId > aRequestedGlyphs( nGlyphCount );

    for( int i = 0; i < nGlyphCount; i++ )
    {
        aEncoding[i]        = aSorted[i].aEnc;
        aRequestedGlyphs[i] = aSorted[i].aGID;
    }

    FontSubsetInfo aInfo;
    aInfo.LoadFont( pSrcFont );

    aInfo.CreateFontSubset( nTargetMask, pTmpFile, pGlyphSetName,
        aRequestedGlyphs.data(), aEncoding.data(), nGlyphCount );
}

void
GlyphSet::PSUploadFont (osl::File& rOutFile, PrinterGfx &rGfx, bool bAllowType42, std::vector< OString >& rSuppliedFonts )
{
    TrueTypeFont *pTTFont;
    OString aTTFileName (rGfx.GetFontMgr().getFontFileSysPath(mnFontID));
    int nFace = rGfx.GetFontMgr().getFontFaceNumber(mnFontID);
    SFErrCodes nSuccess = OpenTTFontFile(aTTFileName.getStr(), nFace, &pTTFont);
    if (nSuccess != SFErrCodes::Ok)
        return;

    utl::TempFile aTmpFile;
    aTmpFile.EnableKillingFile();
    FILE* pTmpFile = fopen(OUStringToOString(aTmpFile.GetFileName(), osl_getThreadTextEncoding()).getStr(), "w+b");
    if (pTmpFile == nullptr)
        return;

    // encoding vector maps character encoding to the ordinal number
    // of the glyph in the output file
    unsigned char  pEncoding[256];
    sal_uInt16 pTTGlyphMapping[256];

    // loop through all the font glyph subsets
    sal_Int32 nGlyphSetID = 1;
    for (auto const& glyph : maGlyphList)
    {
        if (glyph.empty())
        {
            ++nGlyphSetID;
            continue;
        }

        // loop through all the glyphs in the subset
        sal_Int32 n = 0;
        for (auto const& elem : glyph)
        {
            pTTGlyphMapping [n] = elem.first;
            pEncoding       [n] = elem.second;
            n++;
        }

        // create the current subset
        OString aGlyphSetName = GetGlyphSetName(nGlyphSetID);
        fprintf( pTmpFile, "%%%%BeginResource: font %s\n", aGlyphSetName.getStr() );
        CreatePSUploadableFont( pTTFont, pTmpFile, aGlyphSetName.getStr(), glyph.size(),
                                pTTGlyphMapping, pEncoding, bAllowType42 );
        fprintf( pTmpFile, "%%%%EndResource\n" );
        rSuppliedFonts.push_back( aGlyphSetName );
        ++nGlyphSetID;
    }

    // copy the file into the page header
    rewind(pTmpFile);
    fflush(pTmpFile);

    unsigned char  pBuffer[0x2000];
    sal_uInt64 nIn;
    sal_uInt64 nOut;
    do
    {
        nIn = fread(pBuffer, 1, sizeof(pBuffer), pTmpFile);
        rOutFile.write (pBuffer, nIn, nOut);
    }
    while ((nIn == nOut) && !feof(pTmpFile));

    // cleanup
    CloseTTFont (pTTFont);
    fclose (pTmpFile);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
