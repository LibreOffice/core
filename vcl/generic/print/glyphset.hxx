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

#ifndef INCLUDED_VCL_GENERIC_PRINT_GLYPHSET_HXX
#define INCLUDED_VCL_GENERIC_PRINT_GLYPHSET_HXX

#include "fontmanager.hxx"

#include "osl/file.hxx"

#include "rtl/string.hxx"

#include <list>
#include <unordered_map>

class Point;

namespace psp {

class PrinterGfx;
class PrintFontManager;

class GlyphSet
{
private:

    sal_Int32           mnFontID;
    bool            mbVertical;
    OString        maBaseName;
    fonttype::type      meBaseType;
    rtl_TextEncoding    mnBaseEncoding;
    bool                mbUseFontEncoding;

    typedef std::unordered_map< sal_Unicode, sal_uInt8 > char_map_t;
    typedef std::list< char_map_t > char_list_t;
    typedef std::unordered_map< sal_GlyphId, sal_uInt8 > glyph_map_t;
    typedef std::list< glyph_map_t > glyph_list_t;

    char_list_t     maCharList;
    glyph_list_t    maGlyphList;

    OString    GetGlyphSetName (sal_Int32 nGlyphSetID);
    OString    GetCharSetName (sal_Int32 nGlyphSetID);
    sal_Int32       GetGlyphSetEncoding (sal_Int32 nGlyphSetID);
    OString    GetGlyphSetEncodingName (sal_Int32 nGlyphSetID);

    OString    GetReencodedFontName (sal_Int32 nGlyphSetID);
    void            PSDefineReencodedFont (osl::File* pOutFile,
                                           sal_Int32 nGlyphSetID);

    bool        GetCharID (sal_Unicode nChar,
                                unsigned char* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    bool        LookupCharID (sal_Unicode nChar,
                                   unsigned char* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    bool        AddCharID (sal_Unicode nChar,
                                unsigned char* nOutGlyphID,
                                sal_Int32* nOutGlyphSetID);
    bool        GetGlyphID (sal_GlyphId nGlyphId, sal_Unicode nUnicode,
                                unsigned char* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    bool        LookupGlyphID (sal_GlyphId nGlyphId,
                                   unsigned char* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    bool        AddGlyphID (sal_GlyphId nGlyphId, sal_Unicode nUnicode,
                                unsigned char* nOutGlyphID,
                                sal_Int32* nOutGlyphSetID);
    static void     AddNotdef (char_map_t &rCharMap);
    static void     AddNotdef (glyph_map_t &rGlyphMap);
    static unsigned char  GetAnsiMapping (sal_Unicode nUnicodeChar);
    static unsigned char  GetSymbolMapping (sal_Unicode nUnicodeChar);

    void            ImplDrawText (PrinterGfx &rGfx, const Point& rPoint,
                                  const sal_Unicode* pStr, sal_Int16 nLen);
    void            ImplDrawText (PrinterGfx &rGfx, const Point& rPoint,
                                  const sal_Unicode* pStr, sal_Int16 nLen,
                                  const sal_Int32* pDeltaArray);

public:

    GlyphSet (sal_Int32 nFontID, bool bVertical);
    ~GlyphSet ();

    sal_Int32       GetFontID () { return mnFontID;}
    fonttype::type  GetFontType () { return meBaseType;}
    static OString
    GetReencodedFontName (rtl_TextEncoding nEnc,
                          const OString &rFontName);
    static OString
    GetGlyphSetEncodingName (rtl_TextEncoding nEnc,
                             const OString &rFontName);
    bool            IsVertical () { return mbVertical;}

    void            DrawText (PrinterGfx &rGfx, const Point& rPoint,
                              const sal_Unicode* pStr, sal_Int16 nLen,
                              const sal_Int32* pDeltaArray = nullptr);
    void            DrawGlyphs (PrinterGfx& rGfx,
                                const Point& rPoint,
                                const sal_GlyphId* pGlyphIds,
                                const sal_Unicode* pUnicodes,
                                sal_Int16 nLen,
                                const sal_Int32* pDeltaArray,
                                bool bUseGlyphs=true);
    bool        PSUploadEncoding(osl::File* pOutFile, PrinterGfx &rGfx);
    bool        PSUploadFont (osl::File& rOutFile, PrinterGfx &rGfx, bool bAsType42, std::list< OString >& rSuppliedFonts );
};

} /* namespace psp */

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
