/*************************************************************************
 *
 *  $RCSfile: glyphset.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: cp $ $Date: 2001-08-27 11:03:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _PSPRINT_GLYPHSET_HXX_
#include <glyphset.hxx>
#endif
#ifndef _PSPRINT_PRINTERUTIL_HXX_
#include <psputil.hxx>
#endif
#ifndef _PSPRINT_PRINTERGFX_HXX_
#include <psprint/printergfx.hxx>
#endif
#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
#endif
#ifdef SOLARIS
#include <alloca.h>
#endif
#ifndef __SGI_STL_SET
#include <set>
#endif
#include <map>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef __SUBFONT_H
#define NO_LIST
#include <../fontsubset/sft.h>
#undef NO_LIST
#endif

using namespace psp ;

GlyphSet::GlyphSet ()
        : mnFontID   (-1),
          mbVertical (0)
{}

GlyphSet::GlyphSet (sal_Int32 nFontID, sal_Bool bVertical)
        : mnFontID (nFontID),
          mbVertical (bVertical)
{
    PrintFontManager &rMgr = PrintFontManager::get();
    meBaseType      = rMgr.getFontType (mnFontID);
    maBaseName      = ::rtl::OUStringToOString (rMgr.getPSName(mnFontID),
                                           RTL_TEXTENCODING_ASCII_US);
    mnBaseEncoding  = rMgr.getFontEncoding(mnFontID);
}

GlyphSet::~GlyphSet ()
{
    /* FIXME delete the glyphlist ??? */
}

sal_Int32
GlyphSet::GetFontID ()
{
    return mnFontID;
}

fonttype::type
GlyphSet::GetFontType ()
{
    return meBaseType;
}

sal_Bool
GlyphSet::IsVertical ()
{
    return mbVertical;
}

sal_Bool
GlyphSet::SetFont (sal_Int32 nFontID, sal_Bool bVertical)
{
    if (mnFontID != -1)
        return sal_False;

    mnFontID   = nFontID;
    mbVertical = bVertical;

    PrintFontManager &rMgr = PrintFontManager::get();
    meBaseType      = rMgr.getFontType (mnFontID);
    maBaseName      = ::rtl::OUStringToOString (rMgr.getPSName(mnFontID),
                                           RTL_TEXTENCODING_ASCII_US);
    mnBaseEncoding  = rMgr.getFontEncoding(mnFontID);

    return sal_True;
}

sal_Bool
GlyphSet::GetGlyphID (sal_Unicode nChar,
                      sal_uChar* nOutGlyphID, sal_Int32* nOutGlyphSetID)
{
    return    LookupGlyphID (nChar, nOutGlyphID, nOutGlyphSetID)
           || AddGlyphID    (nChar, nOutGlyphID, nOutGlyphSetID);
}

sal_Bool
GlyphSet::LookupGlyphID (sal_Unicode nChar,
                         sal_uChar* nOutGlyphID, sal_Int32* nOutGlyphSetID)
{
    glyphlist_t::iterator aGlyphSet;
    sal_Int32             nGlyphSetID;

    // loop thru all the font subsets
    for (aGlyphSet  = maGlyphList.begin(), nGlyphSetID = 1;
         aGlyphSet != maGlyphList.end();
         aGlyphSet++, nGlyphSetID++)
    {
        // check every subset if it contains the queried unicode char
        glyph_mapping_t::const_iterator aGlyph = (*aGlyphSet).find (nChar);
        if (aGlyph != (*aGlyphSet).end())
        {
            // success: found the unicode char, return the glyphid and the glyphsetid
            *nOutGlyphSetID = nGlyphSetID;
            *nOutGlyphID    = (*aGlyph).second;
            return sal_True;
        }
    }

    *nOutGlyphSetID = -1;
    *nOutGlyphID    =  0;
    return sal_False;
}

sal_uChar
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

    return nSize == 1 ? (sal_uChar)nAnsiChar : (sal_uChar)0;
}

sal_uChar
GlyphSet::GetSymbolMapping (sal_Unicode nUnicodeChar)
{
    if (0x0000 < nUnicodeChar && nUnicodeChar < 0x0100)
        return (sal_uChar)nUnicodeChar;
    if (0xf000 < nUnicodeChar && nUnicodeChar < 0xf100)
        return (sal_uChar)nUnicodeChar;

    return 0;
}

sal_Bool
GlyphSet::AddGlyphID (sal_Unicode nChar,
                      sal_uChar* nOutGlyphID, sal_Int32* nOutGlyphSetID)
{
    sal_uChar nMappedChar;

    // XXX important: avoid to reencode type1 symbol fonts
    if (mnBaseEncoding == RTL_TEXTENCODING_SYMBOL)
        nMappedChar = GetSymbolMapping (nChar);
    else
        nMappedChar = GetAnsiMapping (nChar);

    // create an empty glyphmap that is reserved for iso1252 encoded glyphs
    // (or -- unencoded -- symbol glyphs) and a second map that takes any other
    if (maGlyphList.empty())
    {
        glyph_mapping_t aMap, aMapp;

        maGlyphList.push_back (aMap);
        maGlyphList.push_back (aMapp);
    }
    // if the last map is full, create a new one
    if ((!nMappedChar) && (maGlyphList.back().size() == 255))
    {
        glyph_mapping_t aMap;
        maGlyphList.push_back (aMap);
    }

    // insert a new glyph in the font subset
    if (nMappedChar)
    {
        // always put iso1252 chars into the first map, map them on itself
        glyph_mapping_t& aGlyphSet = maGlyphList.front();
        aGlyphSet [nChar] = nMappedChar;
        *nOutGlyphSetID   = 1;
        *nOutGlyphID      = nMappedChar;
    }
    else
    {
        // other chars are just appended to the list
        glyph_mapping_t& aGlyphSet = maGlyphList.back();
        aGlyphSet [nChar] = aGlyphSet.size();
        *nOutGlyphSetID   = maGlyphList.size();
        *nOutGlyphID      = aGlyphSet [nChar];
    }

    return sal_True;
}

rtl::OString
GlyphSet::GetGlyphSetName (sal_Int32 nGlyphSetID)
{
    if (meBaseType == fonttype::TrueType)
    {
        return maBaseName
              + (mbVertical ? rtl::OString ("VSet") : rtl::OString ("HSet") )
              + rtl::OString::valueOf (nGlyphSetID);
    }
    else
    /* (meBaseType == fonttype::Type1 || meBaseType == fonttype::Builtin) */
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
    /* (meBaseType == fonttype::Type1 || meBaseType == fonttype::Builtin) */
        if (mnBaseEncoding == RTL_TEXTENCODING_SYMBOL)
            return RTL_TEXTENCODING_SYMBOL;
        else
            return nGlyphSetID == 1 ? RTL_TEXTENCODING_MS_1252
                                    : RTL_TEXTENCODING_USER_START + nGlyphSetID;
    }
}

rtl::OString
GlyphSet::GetGlyphSetEncodingName (rtl_TextEncoding nEnc, const rtl::OString &rFontName)
{
    if (   nEnc == RTL_TEXTENCODING_MS_1252
        || nEnc == RTL_TEXTENCODING_ISO_8859_1)
    {
        return rtl::OString("ISO1252Encoding");
    }
    else
    if (nEnc >= RTL_TEXTENCODING_USER_START && nEnc <= RTL_TEXTENCODING_USER_END)
    {
        return  rFontName
                + rtl::OString("Enc")
                + rtl::OString::valueOf ((sal_Int32)(nEnc - RTL_TEXTENCODING_USER_START));
    }
    else
    {
        return rtl::OString();
    }
}

rtl::OString
GlyphSet::GetGlyphSetEncodingName (sal_Int32 nGlyphSetID)
{
    return GetGlyphSetEncodingName (GetGlyphSetEncoding(nGlyphSetID), maBaseName);
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
    int nChar;

    sal_uChar *pGlyphID    = (sal_uChar*)alloca (nLen * sizeof(sal_uChar));
    sal_Int32 *pGlyphSetID = (sal_Int32*)alloca (nLen * sizeof(sal_Int32));

    // convert unicode to glyph id and glyphset (font subset)
    for (nChar = 0; nChar < nLen; nChar++)
        GetGlyphID (pStr[nChar], pGlyphID + nChar, pGlyphSetID + nChar);

    rGfx.PSMoveTo (rPoint);

    // loop over the string to draw subsequent pieces of chars with the same
    // postscript font
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
        rGfx.PSSetFont (GetGlyphSetName(nGlyphSetID), GetGlyphSetEncoding(nGlyphSetID));
        rGfx.PSShowText (pGlyphID + nChar, nGlyphs, nGlyphs);

        nChar += nGlyphs;
    }
}

void
GlyphSet::ImplDrawText (PrinterGfx &rGfx, const Point& rPoint,
                        const sal_Unicode* pStr, sal_Int16 nLen, const sal_Int32* pDeltaArray)
{
    sal_uChar *pGlyphID    = (sal_uChar*)alloca (nLen * sizeof(sal_uChar));
    sal_Int32 *pGlyphSetID = (sal_Int32*)alloca (nLen * sizeof(sal_Int32));
    std::set< sal_Int32 > aGlyphSet;

    // convert unicode to font glyph id and font subset
    for (int nChar = 0; nChar < nLen; nChar++)
    {
        GetGlyphID (pStr[nChar], pGlyphID + nChar, pGlyphSetID + nChar);
        aGlyphSet.insert (pGlyphSetID[nChar]);
    }

    // loop over all glyph sets to detect substrings that can be xshown together
    // without changing the postscript font
    sal_Int32 *pDeltaSubset = (sal_Int32*)alloca (nLen * sizeof(sal_Int32));
    sal_uChar *pGlyphSubset = (sal_uChar*)alloca (nLen * sizeof(sal_uChar));

    std::set< sal_Int32 >::iterator aSet;
    for (aSet = aGlyphSet.begin(); aSet != aGlyphSet.end(); aSet++)
    {
        Point     aPoint  = rPoint;
        sal_Int32 nOffset = 0;
        sal_Int32 nGlyphs = 0;

        // loop over all chars to extract those that share the current glyph set
        for (int nChar = 0; nChar < nLen; nChar++)
        {
            if (pGlyphSetID[nChar] == *aSet)
            {
                pGlyphSubset [nGlyphs] = pGlyphID [nChar];
                pDeltaSubset [nGlyphs] = nOffset;

                nOffset  = nChar < nLen - 1 ? pDeltaArray [nChar] : 0;
                nGlyphs += 1;
            }
            else
            {
                nOffset += nChar < nLen - 1 ? pDeltaArray [nChar] : 0;
            }
        }

        // show the text using the PrinterGfx text api
        aPoint.Move (pDeltaSubset[0], 0);

        rGfx.PSSetFont  (GetGlyphSetName(*aSet), GetGlyphSetEncoding(*aSet));
        rGfx.PSMoveTo   (aPoint);
        rGfx.PSShowText (pGlyphSubset, nGlyphs, nGlyphs, nGlyphs > 1 ? pDeltaSubset + 1 : NULL);
    }
}

sal_Bool
GlyphSet::PSUploadEncoding(osl::File* pOutFile, PrinterGfx &rGfx)
{
    // only for ps fonts
    if ((meBaseType != fonttype::Builtin) && (meBaseType != fonttype::Type1))
        return sal_False;

    PrintFontManager &rMgr = rGfx.GetFontMgr();

    // loop thru all the font subsets
    sal_Int32               nGlyphSetID = 0;
    glyphlist_t::iterator   aGlyphSet;
    for (aGlyphSet = maGlyphList.begin(); aGlyphSet != maGlyphList.end(); ++aGlyphSet)
    {
        ++nGlyphSetID;

        if (nGlyphSetID == 1) // iso1252 anyway, doesn't need reencoding
            continue;
        if ((*aGlyphSet).size() == 0) // empty set, doesn't need reencoding
            continue;

        sal_Char  pEncodingVector [256];
        sal_Int32 nSize = 0;

        nSize += psp::appendStr ("/",
                                 pEncodingVector + nSize);
        nSize += psp::appendStr (GetGlyphSetEncodingName(nGlyphSetID),
                                 pEncodingVector + nSize);
        nSize += psp::appendStr (" [ /.notdef ",
                                 pEncodingVector + nSize);

        // need a list of glyphs, sorted by glyphid
        typedef std::map< sal_uInt8, sal_Unicode > ps_mapping_t;
        typedef ps_mapping_t::value_type ps_value_t;
        ps_mapping_t aSortedGlypSet;

        glyph_mapping_t::const_iterator aUnsortedGlyph;
        for (aUnsortedGlyph  = (*aGlyphSet).begin();
             aUnsortedGlyph != (*aGlyphSet).end();
             aUnsortedGlyph++)
        {
            aSortedGlypSet.insert(ps_value_t((*aUnsortedGlyph).second,
                                             (*aUnsortedGlyph).first));
        }

        ps_mapping_t::const_iterator aSortedGlyph;
        // loop thru all the glyphs in the subset
        for (aSortedGlyph  = (aSortedGlypSet).begin();
             aSortedGlyph != (aSortedGlypSet).end();
             aSortedGlyph++)
        {
            nSize += psp::appendStr ("/",
                                     pEncodingVector + nSize);
            nSize += psp::appendStr (rMgr.getAdobeNameFromUnicode((*aSortedGlyph).second),
                                     pEncodingVector + nSize);
            nSize += psp::appendStr (" ",
                                     pEncodingVector + nSize);
            // flush line
            if (nSize >= 70)
            {
                nSize += psp::appendStr ("\n", pEncodingVector + nSize);
                psp::WritePS (pOutFile, pEncodingVector);
                nSize = 0;
            }
        }

        nSize += psp::appendStr ("] def\n", pEncodingVector + nSize);
        psp::WritePS (pOutFile, pEncodingVector);
    }

    return sal_True;
}

sal_Bool
GlyphSet::PSUploadFont (osl::File& rOutFile, PrinterGfx &rGfx, bool bAsType42 )
{
    // only for truetype fonts
    if (meBaseType != fonttype::TrueType)
        return sal_False;

    TrueTypeFont *pTTFont;
    rtl::OString aTTFileName (rGfx.GetFontMgr().getFontFileSysPath(mnFontID));
    int nFace = rGfx.GetFontMgr().getFontFaceNumber(mnFontID);
    sal_Int32 nSuccess = OpenTTFont(aTTFileName.getStr(), nFace < 0 ? 0 : nFace, &pTTFont);
    if (nSuccess != SF_OK)
        return sal_False;
    FILE* pTmpFile = tmpfile();
    if (pTmpFile == NULL)
        return sal_False;

    // array of unicode source characters
    sal_Unicode pUChars[256];

    // encoding vector maps character encoding to the ordinal number
    // of the glyph in the output file
    sal_uChar  pEncoding[256];
    sal_uInt16 pTTGlyphMapping[256];

    // loop thru all the font subsets
    sal_Int32 nGlyphSetID;
    glyphlist_t::iterator aGlyphSet;
    for (aGlyphSet = maGlyphList.begin(), nGlyphSetID = 1;
         aGlyphSet != maGlyphList.end();
         ++aGlyphSet, nGlyphSetID++)
    {
        if ((*aGlyphSet).size() == 0)
            continue;

        // loop thru all the glyphs in the subset
        glyph_mapping_t::const_iterator aGlyph;
        sal_Int32 n = 0;
        for (aGlyph = (*aGlyphSet).begin(); aGlyph != (*aGlyphSet).end(); aGlyph++)
        {
            pUChars [n]   = (*aGlyph).first;
            pEncoding [n] = (*aGlyph).second;
            n++;
        }
        // create a mapping from the unicode chars to the glyph encoding in
        // source TrueType font
        MapString (pTTFont, pUChars, (*aGlyphSet).size(), pTTGlyphMapping, mbVertical);

        // create the current subset
        if( bAsType42 )
            CreateT42FromTTGlyphs (pTTFont, pTmpFile, GetGlyphSetName(nGlyphSetID),
                                   pTTGlyphMapping, pEncoding, (*aGlyphSet).size() );
        else
            CreateT3FromTTGlyphs  (pTTFont, pTmpFile, GetGlyphSetName(nGlyphSetID),
                                   pTTGlyphMapping, pEncoding, (*aGlyphSet).size(),
                                   0 /* 0 = horizontal, 1 = vertical */ );
    }

    // copy the file into the page header
    rewind(pTmpFile);
    fflush(pTmpFile);

    sal_uChar  pBuffer[0x2000];
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

    return sal_True;
}


