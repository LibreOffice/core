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

#ifndef _PSPRINT_GLYPHSET_HXX_
#define _PSPRINT_GLYPHSET_HXX_

#include "vcl/fontmanager.hxx"

#include "osl/file.hxx"

#include "rtl/string.hxx"

#include "tools/gen.hxx"

#include <list>
#include <hash_map>

namespace psp {

class PrinterGfx;
class PrintFontManager;

class GlyphSet
{
private:

    sal_Int32           mnFontID;
    sal_Bool            mbVertical;
    rtl::OString        maBaseName;
    fonttype::type      meBaseType;
    rtl_TextEncoding    mnBaseEncoding;
    bool                mbUseFontEncoding;

    typedef std::hash_map< sal_Unicode, sal_uInt8 > char_map_t;
    typedef std::list< char_map_t > char_list_t;
    typedef std::hash_map< sal_uInt32, sal_uInt8 > glyph_map_t;
    typedef std::list< glyph_map_t > glyph_list_t;

    char_list_t     maCharList;
    glyph_list_t    maGlyphList;

    rtl::OString    GetGlyphSetName (sal_Int32 nGlyphSetID);
    rtl::OString    GetCharSetName (sal_Int32 nGlyphSetID);
    sal_Int32       GetGlyphSetEncoding (sal_Int32 nGlyphSetID);
    rtl::OString    GetGlyphSetEncodingName (sal_Int32 nGlyphSetID);

    rtl::OString    GetReencodedFontName (sal_Int32 nGlyphSetID);
    void            PSDefineReencodedFont (osl::File* pOutFile,
                                           sal_Int32 nGlyphSetID);

    sal_Bool        GetCharID (sal_Unicode nChar,
                                sal_uChar* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    sal_Bool        LookupCharID (sal_Unicode nChar,
                                   sal_uChar* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    sal_Bool        AddCharID (sal_Unicode nChar,
                                sal_uChar* nOutGlyphID,
                                sal_Int32* nOutGlyphSetID);
    sal_Bool        GetGlyphID (sal_uInt32 nGlyph, sal_Unicode nUnicode,
                                sal_uChar* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    sal_Bool        LookupGlyphID (sal_uInt32 nGlyph,
                                   sal_uChar* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    sal_Bool        AddGlyphID (sal_uInt32 nGlyph, sal_Unicode nUnicode,
                                sal_uChar* nOutGlyphID,
                                sal_Int32* nOutGlyphSetID);
    void            AddNotdef (char_map_t &rCharMap);
    void            AddNotdef (glyph_map_t &rGlyphMap);
    sal_uChar       GetAnsiMapping (sal_Unicode nUnicodeChar);
    sal_uChar       GetSymbolMapping (sal_Unicode nUnicodeChar);

    void            ImplDrawText (PrinterGfx &rGfx, const Point& rPoint,
                                  const sal_Unicode* pStr, sal_Int16 nLen);
    void            ImplDrawText (PrinterGfx &rGfx, const Point& rPoint,
                                  const sal_Unicode* pStr, sal_Int16 nLen,
                                  const sal_Int32* pDeltaArray);

public:

    GlyphSet ();
    GlyphSet (sal_Int32 nFontID, sal_Bool bVertical);
    ~GlyphSet ();

    sal_Int32       GetFontID ();
    fonttype::type  GetFontType ();
    static rtl::OString
    GetReencodedFontName (rtl_TextEncoding nEnc,
                          const rtl::OString &rFontName);
    static rtl::OString
    GetGlyphSetEncodingName (rtl_TextEncoding nEnc,
                             const rtl::OString &rFontName);
    sal_Bool        IsVertical ();

    sal_Bool        SetFont (sal_Int32 nFontID, sal_Bool bVertical);

    void            DrawText (PrinterGfx &rGfx, const Point& rPoint,
                              const sal_Unicode* pStr, sal_Int16 nLen,
                              const sal_Int32* pDeltaArray = NULL);
    void            DrawGlyphs (PrinterGfx& rGfx,
                                const Point& rPoint,
                                const sal_uInt32* pGlyphIds,
                                const sal_Unicode* pUnicodes,
                                sal_Int16 nLen,
                                const sal_Int32* pDeltaArray );
    sal_Bool        PSUploadEncoding(osl::File* pOutFile, PrinterGfx &rGfx);
    sal_Bool        PSUploadFont (osl::File& rOutFile, PrinterGfx &rGfx, bool bAsType42, std::list< rtl::OString >& rSuppliedFonts );
};


} /* namespace psp */

#endif

