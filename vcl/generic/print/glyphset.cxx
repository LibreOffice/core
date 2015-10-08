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
#include "psputil.hxx"

#include "sft.hxx"

#include "generic/printergfx.hxx"
#include "fontsubset.hxx"
#include "fontmanager.hxx"

#include <tools/gen.hxx>

#include "osl/thread.h"

#include "sal/alloca.h"

#include "rtl/ustring.hxx"
#include "rtl/strbuf.hxx"

#include <unotools/tempfile.hxx>

#include <set>
#include <map>
#include <algorithm>

using namespace vcl;
using namespace psp;

GlyphSet::GlyphSet (sal_Int32 nFontID, bool bVertical)
        : mnFontID (nFontID),
          mbVertical (bVertical)
{
    PrintFontManager &rMgr = PrintFontManager::get();
    meBaseType          = rMgr.getFontType (mnFontID);
    maBaseName          = OUStringToOString (rMgr.getPSName(mnFontID),
                                           RTL_TEXTENCODING_ASCII_US);
    mnBaseEncoding      = rMgr.getFontEncoding(mnFontID);
    mbUseFontEncoding   = rMgr.getUseOnlyFontEncoding(mnFontID);
}

GlyphSet::~GlyphSet ()
{
    /* FIXME delete the glyphlist ??? */
}

bool
GlyphSet::GetCharID (
                     sal_Unicode nChar,
                     unsigned char* nOutGlyphID,
                     sal_Int32* nOutGlyphSetID
                     )
{
    return    LookupCharID (nChar, nOutGlyphID, nOutGlyphSetID)
           || AddCharID    (nChar, nOutGlyphID, nOutGlyphSetID);
}

bool
GlyphSet::GetGlyphID (
                      sal_GlyphId nGlyph,
                      sal_Unicode nUnicode,
                      unsigned char* nOutGlyphID,
                      sal_Int32* nOutGlyphSetID
                     )
{
    return    LookupGlyphID (nGlyph, nOutGlyphID, nOutGlyphSetID)
           || AddGlyphID    (nGlyph, nUnicode, nOutGlyphID, nOutGlyphSetID);
}

bool
GlyphSet::LookupCharID (
                        sal_Unicode nChar,
                        unsigned char* nOutGlyphID,
                        sal_Int32* nOutGlyphSetID
                        )
{
    char_list_t::iterator aGlyphSet;
    sal_Int32             nGlyphSetID;

    // loop through all the font subsets
    for (aGlyphSet  = maCharList.begin(), nGlyphSetID = 1;
         aGlyphSet != maCharList.end();
         ++aGlyphSet, nGlyphSetID++)
    {
        // check every subset if it contains the queried unicode char
        char_map_t::const_iterator aGlyph = (*aGlyphSet).find (nChar);
        if (aGlyph != (*aGlyphSet).end())
        {
            // success: found the unicode char, return the glyphid and the glyphsetid
            *nOutGlyphSetID = nGlyphSetID;
            *nOutGlyphID    = (*aGlyph).second;
            return true;
        }
    }

    *nOutGlyphSetID = -1;
    *nOutGlyphID    =  0;
    return false;
}

bool
GlyphSet::LookupGlyphID (
                        sal_GlyphId nGlyph,
                        unsigned char* nOutGlyphID,
                        sal_Int32* nOutGlyphSetID
                        )
{
    glyph_list_t::iterator aGlyphSet;
    sal_Int32             nGlyphSetID;

    // loop through all the font subsets
    for (aGlyphSet  = maGlyphList.begin(), nGlyphSetID = 1;
         aGlyphSet != maGlyphList.end();
         ++aGlyphSet, nGlyphSetID++)
    {
        // check every subset if it contains the queried unicode char
        glyph_map_t::const_iterator aGlyph = (*aGlyphSet).find (nGlyph);
        if (aGlyph != (*aGlyphSet).end())
        {
            // success: found the glyph id, return the mapped glyphid and the glyphsetid
            *nOutGlyphSetID = nGlyphSetID;
            *nOutGlyphID    = (*aGlyph).second;
            return true;
        }
    }

    *nOutGlyphSetID = -1;
    *nOutGlyphID    =  0;
    return false;
}

unsigned char
GlyphSet::GetAnsiMapping (sal_Unicode nUnicodeChar)
{
    static rtl_UnicodeToTextConverter aConverter =
                rtl_createUnicodeToTextConverter(RTL_TEXTENCODING_MS_1252);
    static rtl_UnicodeToTextContext aContext =
             rtl_createUnicodeToTextContext( aConverter );

    sal_Char            nAnsiChar;
    sal_uInt32          nCvtInfo;
    sal_Size            nCvtChars;
       const sal_uInt32    nCvtFlags =  RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                                   | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR;

    sal_Size nSize = rtl_convertUnicodeToText( aConverter, aContext,
                &nUnicodeChar, 1, &nAnsiChar, 1,
                nCvtFlags, &nCvtInfo, &nCvtChars );

    return nSize == 1 ? (unsigned char)nAnsiChar : (unsigned char)0;
}

unsigned char
GlyphSet::GetSymbolMapping (sal_Unicode nUnicodeChar)
{
    if (0x0000 < nUnicodeChar && nUnicodeChar < 0x0100)
        return (unsigned char)nUnicodeChar;
    if (0xf000 < nUnicodeChar && nUnicodeChar < 0xf100)
        return (unsigned char)nUnicodeChar;

    return 0;
}

void
GlyphSet::AddNotdef (char_map_t &rCharMap)
{
    if (rCharMap.empty())
        rCharMap[0] = 0;
}

void
GlyphSet::AddNotdef (glyph_map_t &rGlyphMap)
{
    if (rGlyphMap.empty())
        rGlyphMap[0] = 0;
}
bool
GlyphSet::AddCharID (
                     sal_Unicode nChar,
                     unsigned char* nOutGlyphID,
                     sal_Int32* nOutGlyphSetID
                     )
{
    unsigned char nMappedChar;

    // XXX important: avoid to reencode type1 symbol fonts
    if (mnBaseEncoding == RTL_TEXTENCODING_SYMBOL)
        nMappedChar = GetSymbolMapping (nChar);
    else
        nMappedChar = GetAnsiMapping (nChar);

    // create an empty glyphmap that is reserved for iso1252 encoded glyphs
    // (or -- unencoded -- symbol glyphs) and a second map that takes any other
    if (maCharList.empty())
    {
        char_map_t aMap, aMapp;

        maCharList.push_back (aMap);
        maCharList.push_back (aMapp);
    }
    // if the last map is full, create a new one
    if ((!nMappedChar) && (maCharList.back().size() == 255))
    {
        char_map_t aMap;
        maCharList.push_back (aMap);
    }

    // insert a new glyph in the font subset
    if (nMappedChar)
    {
        // always put iso1252 chars into the first map, map them on itself
        char_map_t& aGlyphSet = maCharList.front();
        AddNotdef (aGlyphSet);

        aGlyphSet [nChar] = nMappedChar;
        *nOutGlyphSetID   = 1;
        *nOutGlyphID      = nMappedChar;
    }
    else
    {
        // other chars are just appended to the list
        char_map_t& aGlyphSet = maCharList.back();
        AddNotdef (aGlyphSet);

        int nSize         = aGlyphSet.size();

        aGlyphSet [nChar] = nSize;
        *nOutGlyphSetID   = maCharList.size();
        *nOutGlyphID      = aGlyphSet [nChar];
    }

    return true;
}

bool
GlyphSet::AddGlyphID (
                     sal_GlyphId nGlyph,
                     sal_Unicode nUnicode,
                     unsigned char* nOutGlyphID,
                     sal_Int32* nOutGlyphSetID
                     )
{
    unsigned char nMappedChar = 0;

    // XXX important: avoid to reencode type1 symbol fonts
    if (mnBaseEncoding == RTL_TEXTENCODING_SYMBOL)
        nMappedChar = GetSymbolMapping (nUnicode);

    // create an empty glyphmap that is reserved for unencoded symbol glyphs,
    // and a second map that takes any other
    if (maGlyphList.empty())
    {
        glyph_map_t aMap, aMapp;

        maGlyphList.push_back (aMap);
        maGlyphList.push_back (aMapp);
    }
    // if the last map is full, create a new one
    if ((!nMappedChar) && (maGlyphList.back().size() == 255))
    {
        glyph_map_t aMap;
        maGlyphList.push_back (aMap);
    }

    // insert a new glyph in the font subset
    if (nMappedChar)
    {
        // always put symbol glyphs into the first map, map them on itself
        glyph_map_t& aGlyphSet = maGlyphList.front();
        AddNotdef (aGlyphSet);

        aGlyphSet [nGlyph] = nMappedChar;
        *nOutGlyphSetID    = 1;
        *nOutGlyphID       = nMappedChar;
    }
    else
    {
        // other glyphs are just appended to the list
        glyph_map_t& aGlyphSet = maGlyphList.back();
        AddNotdef (aGlyphSet);

        int nSize         = aGlyphSet.size();

        aGlyphSet [nGlyph] = nSize;
        *nOutGlyphSetID   = maGlyphList.size();
        *nOutGlyphID      = aGlyphSet [nGlyph];
    }

    return true;
}

OString
GlyphSet::GetCharSetName (sal_Int32 nGlyphSetID)
{
    if (meBaseType == fonttype::TrueType)
    {
        OStringBuffer aSetName( maBaseName.getLength() + 32 );
        aSetName.append( maBaseName );
        aSetName.append( "FID" );
        aSetName.append( mnFontID );
        aSetName.append( mbVertical ? "VCSet" : "HCSet" );
        aSetName.append( nGlyphSetID );
        return aSetName.makeStringAndClear();
    }
    else
    {
        return maBaseName;
    }
}

OString
GlyphSet::GetGlyphSetName (sal_Int32 nGlyphSetID)
{
    if (meBaseType == fonttype::TrueType)
    {
        OStringBuffer aSetName( maBaseName.getLength() + 32 );
        aSetName.append( maBaseName );
        aSetName.append( "FID" );
        aSetName.append( mnFontID );
        aSetName.append( mbVertical ? "VGSet" : "HGSet" );
        aSetName.append( nGlyphSetID );
        return aSetName.makeStringAndClear();
    }
    else
    {
        return maBaseName;
    }
}

sal_Int32
GlyphSet::GetGlyphSetEncoding (sal_Int32 nGlyphSetID)
{
    if (meBaseType == fonttype::TrueType)
        return RTL_TEXTENCODING_DONTKNOW;
    else
    {
        if (mnBaseEncoding == RTL_TEXTENCODING_SYMBOL)
            return RTL_TEXTENCODING_SYMBOL;
        else
            return nGlyphSetID == 1 ? RTL_TEXTENCODING_MS_1252
                                    : RTL_TEXTENCODING_USER_START + nGlyphSetID;
    }
}

OString
GlyphSet::GetGlyphSetEncodingName (rtl_TextEncoding nEnc, const OString &rFontName)
{
    if (   nEnc == RTL_TEXTENCODING_MS_1252
        || nEnc == RTL_TEXTENCODING_ISO_8859_1)
    {
        return OString("ISO1252Encoding");
    }
    else
    if (nEnc >= RTL_TEXTENCODING_USER_START && nEnc <= RTL_TEXTENCODING_USER_END)
    {
        return  rFontName
                + OString("Enc")
                + OString::number ((nEnc - RTL_TEXTENCODING_USER_START));
    }
    else
    {
        return OString();
    }
}

OString
GlyphSet::GetGlyphSetEncodingName (sal_Int32 nGlyphSetID)
{
    return GetGlyphSetEncodingName (GetGlyphSetEncoding(nGlyphSetID), maBaseName);
}

void
GlyphSet::PSDefineReencodedFont (osl::File* pOutFile, sal_Int32 nGlyphSetID)
{
    // only for ps fonts
    if (meBaseType != fonttype::Type1)
        return;

    sal_Char  pEncodingVector [256];
    sal_Int32 nSize = 0;

    nSize += psp::appendStr ("(", pEncodingVector + nSize);
    nSize += psp::appendStr (GetReencodedFontName(nGlyphSetID).getStr(),
                                  pEncodingVector + nSize);
    nSize += psp::appendStr (") cvn (", pEncodingVector + nSize);
    nSize += psp::appendStr (maBaseName.getStr(),
                                  pEncodingVector + nSize);
    nSize += psp::appendStr (") cvn ", pEncodingVector + nSize);
    nSize += psp::appendStr (GetGlyphSetEncodingName(nGlyphSetID).getStr(),
                                  pEncodingVector + nSize);
    nSize += psp::appendStr (" psp_definefont\n",
                                  pEncodingVector + nSize);

    psp::WritePS (pOutFile, pEncodingVector, nSize);
}

OString
GlyphSet::GetReencodedFontName (rtl_TextEncoding nEnc, const OString &rFontName)
{
    if (   nEnc == RTL_TEXTENCODING_MS_1252
        || nEnc == RTL_TEXTENCODING_ISO_8859_1)
    {
        return rFontName
               + OString("-iso1252");
    }
    else
    if (nEnc >= RTL_TEXTENCODING_USER_START && nEnc <= RTL_TEXTENCODING_USER_END)
    {
        return rFontName
               + OString("-enc")
               + OString::number ((nEnc - RTL_TEXTENCODING_USER_START));
    }
    else
    {
        return OString();
    }
}

OString
GlyphSet::GetReencodedFontName (sal_Int32 nGlyphSetID)
{
    return GetReencodedFontName (GetGlyphSetEncoding(nGlyphSetID), maBaseName);
}

void GlyphSet::DrawGlyphs(
                          PrinterGfx& rGfx,
                          const Point& rPoint,
                          const sal_GlyphId* pGlyphIds,
                          const sal_Unicode* pUnicodes,
                          sal_Int16 nLen,
                          const sal_Int32* pDeltaArray,
                          const bool bUseGlyphs)
{
    unsigned char *pGlyphID    = static_cast<unsigned char*>(alloca (nLen * sizeof(unsigned char)));
    sal_Int32 *pGlyphSetID = static_cast<sal_Int32*>(alloca (nLen * sizeof(sal_Int32)));
    std::set< sal_Int32 > aGlyphSet;

    // convert unicode to font glyph id and font subset
    for (int nChar = 0; nChar < nLen; nChar++)
    {
        if (bUseGlyphs)
            GetGlyphID (pGlyphIds[nChar], pUnicodes[nChar], pGlyphID + nChar, pGlyphSetID + nChar);
        else
            GetCharID (pUnicodes[nChar], pGlyphID + nChar, pGlyphSetID + nChar);
        aGlyphSet.insert (pGlyphSetID[nChar]);
    }

    // loop over all glyph sets to detect substrings that can be shown together
    // without changing the postscript font
    sal_Int32 *pDeltaSubset = static_cast<sal_Int32*>(alloca (nLen * sizeof(sal_Int32)));
    unsigned char *pGlyphSubset = static_cast<unsigned char*>(alloca (nLen * sizeof(unsigned char)));

    std::set< sal_Int32 >::iterator aSet;
    for (aSet = aGlyphSet.begin(); aSet != aGlyphSet.end(); ++aSet)
    {
        Point     aPoint  = rPoint;
        sal_Int32 nOffset = 0;
        sal_Int32 nGlyphs = 0;
        sal_Int32 nChar;

        // get offset to first glyph
        for (nChar = 0; (nChar < nLen) && (pGlyphSetID[nChar] != *aSet); nChar++)
        {
            nOffset = pDeltaArray [nChar];
        }

        // loop over all chars to extract those that share the current glyph set
        for (nChar = 0; nChar < nLen; nChar++)
        {
            if (pGlyphSetID[nChar] == *aSet)
            {
                pGlyphSubset [nGlyphs] = pGlyphID [nChar];
                // the offset to the next glyph is determined by the glyph in
                // front of the next glyph with the same glyphset id
                // most often, this will be the current glyph
                while ((nChar + 1) < nLen)
                {
                    if (pGlyphSetID[nChar + 1] == *aSet)
                        break;
                    else
                        nChar += 1;
                }
                pDeltaSubset [nGlyphs] = pDeltaArray[nChar] - nOffset;

                nGlyphs += 1;
            }
        }

        // show the text using the PrinterGfx text api
        aPoint.Move (nOffset, 0);

        OString aGlyphSetName;
        if (bUseGlyphs)
            aGlyphSetName = GetGlyphSetName(*aSet);
        else
            aGlyphSetName = GetCharSetName(*aSet);

        rGfx.PSSetFont  (aGlyphSetName, GetGlyphSetEncoding(*aSet));
        rGfx.PSMoveTo   (aPoint);
        rGfx.PSShowText (pGlyphSubset, nGlyphs, nGlyphs, nGlyphs > 1 ? pDeltaSubset : NULL);
    }
}

void
GlyphSet::DrawText (PrinterGfx &rGfx, const Point& rPoint,
                    const sal_Unicode* pStr, sal_Int16 nLen, const sal_Int32* pDeltaArray)
{
    // dispatch to the impl method
    if (pDeltaArray == NULL)
        ImplDrawText (rGfx, rPoint, pStr, nLen);
    else
        ImplDrawText (rGfx, rPoint, pStr, nLen, pDeltaArray);
}

void
GlyphSet::ImplDrawText (PrinterGfx &rGfx, const Point& rPoint,
                        const sal_Unicode* pStr, sal_Int16 nLen)
{
    rGfx.PSMoveTo (rPoint);

    if( mbUseFontEncoding )
    {
        OString aPSName( OUStringToOString( rGfx.GetFontMgr().getPSName( mnFontID ), RTL_TEXTENCODING_ISO_8859_1 ) );
        OString aBytes( OUStringToOString( OUString( pStr, nLen ), mnBaseEncoding ) );
        rGfx.PSSetFont( aPSName, mnBaseEncoding );
        rGfx.PSShowText( reinterpret_cast<const unsigned char*>(aBytes.getStr()), nLen, aBytes.getLength() );
        return;
    }

    int nChar;
    unsigned char *pGlyphID    = static_cast<unsigned char*>(alloca (nLen * sizeof(unsigned char)));
    sal_Int32 *pGlyphSetID = static_cast<sal_Int32*>(alloca (nLen * sizeof(sal_Int32)));

    // convert unicode to glyph id and char set (font subset)
    for (nChar = 0; nChar < nLen; nChar++)
        GetCharID (pStr[nChar], pGlyphID + nChar, pGlyphSetID + nChar);

    // loop over the string to draw subsequent pieces of chars
    // with the same postscript font
    for (nChar = 0; nChar < nLen; /* atend */)
    {
        sal_Int32 nGlyphSetID = pGlyphSetID [nChar];
        sal_Int32 nGlyphs     = 1;
        for (int nNextChar = nChar + 1; nNextChar < nLen; nNextChar++)
        {
            if (pGlyphSetID[nNextChar] == nGlyphSetID)
                nGlyphs++;
            else
                break;
        }

        // show the text using the PrinterGfx text api
        OString aGlyphSetName(GetCharSetName(nGlyphSetID));
        rGfx.PSSetFont (aGlyphSetName, GetGlyphSetEncoding(nGlyphSetID));
        rGfx.PSShowText (pGlyphID + nChar, nGlyphs, nGlyphs);

        nChar += nGlyphs;
    }
}

void
GlyphSet::ImplDrawText (PrinterGfx &rGfx, const Point& rPoint,
                        const sal_Unicode* pStr, sal_Int16 nLen, const sal_Int32* pDeltaArray)
{
    if( mbUseFontEncoding )
    {
        OString aPSName( OUStringToOString( rGfx.GetFontMgr().getPSName( mnFontID ), RTL_TEXTENCODING_ISO_8859_1 ) );
        OString aBytes( OUStringToOString( OUString( pStr, nLen ), mnBaseEncoding ) );
        rGfx.PSMoveTo( rPoint );
        rGfx.PSSetFont( aPSName, mnBaseEncoding );
        rGfx.PSShowText( reinterpret_cast<const unsigned char*>(aBytes.getStr()), nLen, aBytes.getLength(), pDeltaArray );
        return;
    }

    DrawGlyphs( rGfx, rPoint, NULL, pStr, nLen, pDeltaArray, false);
}

bool
GlyphSet::PSUploadEncoding(osl::File* pOutFile, PrinterGfx &rGfx)
{
    // only for ps fonts
    if (meBaseType != fonttype::Type1)
        return false;
    if (mnBaseEncoding == RTL_TEXTENCODING_SYMBOL)
        return false;

    PrintFontManager &rMgr = rGfx.GetFontMgr();

    // loop through all the font subsets
    sal_Int32               nGlyphSetID = 0;
    char_list_t::iterator   aGlyphSet;
    for (aGlyphSet = maCharList.begin(); aGlyphSet != maCharList.end(); ++aGlyphSet)
    {
        ++nGlyphSetID;

        if (nGlyphSetID == 1) // latin1 page uses global reencoding table
        {
            PSDefineReencodedFont (pOutFile, nGlyphSetID);
            continue;
        }
        if ((*aGlyphSet).empty()) // empty set, doesn't need reencoding
        {
            continue;
        }

        // create reencoding table

        sal_Char  pEncodingVector [256];
        sal_Int32 nSize = 0;

        nSize += psp::appendStr ("/",
                                 pEncodingVector + nSize);
        nSize += psp::appendStr (GetGlyphSetEncodingName(nGlyphSetID).getStr(),
                                 pEncodingVector + nSize);
        nSize += psp::appendStr (" [ ",
                                 pEncodingVector + nSize);

        // need a list of glyphs, sorted by glyphid
        typedef std::map< sal_uInt8, sal_Unicode > ps_mapping_t;
        typedef ps_mapping_t::value_type ps_value_t;
        ps_mapping_t aSortedGlyphSet;

        char_map_t::const_iterator aUnsortedGlyph;
        for (aUnsortedGlyph  = (*aGlyphSet).begin();
             aUnsortedGlyph != (*aGlyphSet).end();
             ++aUnsortedGlyph)
        {
            aSortedGlyphSet.insert(ps_value_t((*aUnsortedGlyph).second,
                                             (*aUnsortedGlyph).first));
        }

        ps_mapping_t::const_iterator aSortedGlyph;
        // loop through all the glyphs in the subset
        for (aSortedGlyph  = (aSortedGlyphSet).begin();
             aSortedGlyph != (aSortedGlyphSet).end();
             ++aSortedGlyph)
        {
            nSize += psp::appendStr ("/",
                                     pEncodingVector + nSize);

            std::list< OString > aName( rMgr.getAdobeNameFromUnicode((*aSortedGlyph).second) );

            if( !aName.empty() )
                nSize += psp::appendStr ( aName.front().getStr(), pEncodingVector + nSize);
            else
                nSize += psp::appendStr (".notdef", pEncodingVector + nSize );
            nSize += psp::appendStr (" ",  pEncodingVector + nSize);
            // flush line
            if (nSize >= 70)
            {
                psp::appendStr ("\n", pEncodingVector + nSize);
                psp::WritePS (pOutFile, pEncodingVector);
                nSize = 0;
            }
        }

        nSize += psp::appendStr ("] def\n", pEncodingVector + nSize);
        psp::WritePS (pOutFile, pEncodingVector, nSize);

        PSDefineReencodedFont (pOutFile, nGlyphSetID);
    }

    return true;
}

struct EncEntry
{
    unsigned char  aEnc;
    long       aGID;

    EncEntry() : aEnc( 0 ), aGID( 0 ) {}

    bool operator<( const EncEntry& rRight ) const
    { return aEnc < rRight.aEnc; }
};

static void CreatePSUploadableFont( TrueTypeFont* pSrcFont, FILE* pTmpFile,
    const char* pGlyphSetName, int nGlyphCount,
    /*const*/ sal_uInt16* pRequestedGlyphs, /*const*/ unsigned char* pEncoding,
    bool bAllowType42, bool /*bAllowCID*/ )
{
    // match the font-subset to the printer capabilities
     // TODO: allow CFF for capable printers
    int nTargetMask = FontSubsetInfo::TYPE1_PFA | FontSubsetInfo::TYPE3_FONT;
    if( bAllowType42 )
        nTargetMask |= FontSubsetInfo::TYPE42_FONT;

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
        &aRequestedGlyphs[0], &aEncoding[0], nGlyphCount, NULL );
}

bool
GlyphSet::PSUploadFont (osl::File& rOutFile, PrinterGfx &rGfx, bool bAllowType42, std::list< OString >& rSuppliedFonts )
{
    // only for truetype fonts
    if (meBaseType != fonttype::TrueType)
        return false;

#if defined( UNX )
    TrueTypeFont *pTTFont;
    OString aTTFileName (rGfx.GetFontMgr().getFontFileSysPath(mnFontID));
    int nFace = rGfx.GetFontMgr().getFontFaceNumber(mnFontID);
    sal_Int32 nSuccess = OpenTTFontFile(aTTFileName.getStr(), nFace, &pTTFont);
    if (nSuccess != SF_OK)
        return false;

    utl::TempFile aTmpFile;
    aTmpFile.EnableKillingFile();
    FILE* pTmpFile = fopen(OUStringToOString(aTmpFile.GetFileName(), osl_getThreadTextEncoding()).getStr(), "w+b");
    if (pTmpFile == NULL)
        return false;

    // array of unicode source characters
    sal_Unicode pUChars[256];

    // encoding vector maps character encoding to the ordinal number
    // of the glyph in the output file
    unsigned char  pEncoding[256];
    sal_uInt16 pTTGlyphMapping[256];
    const bool bAllowCID = false; // TODO: nPSLanguageLevel>=3

    // loop through all the font subsets
    sal_Int32 nCharSetID;
    char_list_t::iterator aCharSet;
    for (aCharSet = maCharList.begin(), nCharSetID = 1;
         aCharSet != maCharList.end();
         ++aCharSet, nCharSetID++)
    {
        if ((*aCharSet).empty())
            continue;

        // loop through all the chars in the subset
        char_map_t::const_iterator aChar;
        sal_Int32 n = 0;
        for (aChar = (*aCharSet).begin(); aChar != (*aCharSet).end(); ++aChar)
        {
            pUChars [n]   = (*aChar).first;
            pEncoding [n] = (*aChar).second;
            n++;
        }
        // create a mapping from the unicode chars to the char encoding in
        // source TrueType font
        MapString (pTTFont, pUChars, (*aCharSet).size(), pTTGlyphMapping, mbVertical);

        // create the current subset
        OString aCharSetName = GetCharSetName(nCharSetID);
        fprintf( pTmpFile, "%%%%BeginResource: font %s\n", aCharSetName.getStr() );
        CreatePSUploadableFont( pTTFont, pTmpFile, aCharSetName.getStr(), (*aCharSet).size(),
                                pTTGlyphMapping, pEncoding, bAllowType42, bAllowCID );
        fprintf( pTmpFile, "%%%%EndResource\n" );
        rSuppliedFonts.push_back( aCharSetName );
    }

    // loop through all the font glyph subsets
    sal_Int32 nGlyphSetID;
    glyph_list_t::iterator aGlyphSet;
    for (aGlyphSet = maGlyphList.begin(), nGlyphSetID = 1;
         aGlyphSet != maGlyphList.end();
         ++aGlyphSet, nGlyphSetID++)
    {
        if ((*aGlyphSet).empty())
            continue;

        // loop through all the glyphs in the subset
        glyph_map_t::const_iterator aGlyph;
        sal_Int32 n = 0;
        for (aGlyph = (*aGlyphSet).begin(); aGlyph != (*aGlyphSet).end(); ++aGlyph)
        {
            pTTGlyphMapping [n] = (*aGlyph).first;
            pEncoding       [n] = (*aGlyph).second;
            n++;
        }

        // create the current subset
        OString aGlyphSetName = GetGlyphSetName(nGlyphSetID);
        fprintf( pTmpFile, "%%%%BeginResource: font %s\n", aGlyphSetName.getStr() );
        CreatePSUploadableFont( pTTFont, pTmpFile, aGlyphSetName.getStr(), (*aGlyphSet).size(),
                                pTTGlyphMapping, pEncoding, bAllowType42, bAllowCID );
        fprintf( pTmpFile, "%%%%EndResource\n" );
        rSuppliedFonts.push_back( aGlyphSetName );
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

    return true;
#else
    (void)rOutFile; (void)rGfx; (void)bAllowType42; (void)rSuppliedFonts;
#  warning FIXME: Missing OpenTTFontFile outside of Unix ...
    return false;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
