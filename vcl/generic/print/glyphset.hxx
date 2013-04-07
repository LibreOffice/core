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

#ifndef _PSPRINT_GLYPHSET_HXX_
#define _PSPRINT_GLYPHSET_HXX_

#include "vcl/fontmanager.hxx"

#include "osl/file.hxx"

#include "rtl/string.hxx"

#include <list>
#include <boost/unordered_map.hpp>

class Point;

namespace psp {

class PrinterGfx;
class PrintFontManager;

class GlyphSet
{
private:

    sal_Int32           mnFontID;
    sal_Bool            mbVertical;
    OString        maBaseName;
    fonttype::type      meBaseType;
    rtl_TextEncoding    mnBaseEncoding;
    bool                mbUseFontEncoding;

    typedef boost::unordered_map< sal_Unicode, sal_uInt8 > char_map_t;
    typedef std::list< char_map_t > char_list_t;
    typedef boost::unordered_map< sal_uInt32, sal_uInt8 > glyph_map_t;
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

    GlyphSet (sal_Int32 nFontID, sal_Bool bVertical);
    ~GlyphSet ();

    sal_Int32       GetFontID ();
    fonttype::type  GetFontType ();
    static OString
    GetReencodedFontName (rtl_TextEncoding nEnc,
                          const OString &rFontName);
    static OString
    GetGlyphSetEncodingName (rtl_TextEncoding nEnc,
                             const OString &rFontName);
    sal_Bool        IsVertical ();

    void            DrawText (PrinterGfx &rGfx, const Point& rPoint,
                              const sal_Unicode* pStr, sal_Int16 nLen,
                              const sal_Int32* pDeltaArray = NULL);
    void            DrawGlyphs (PrinterGfx& rGfx,
                                const Point& rPoint,
                                const sal_uInt32* pGlyphIds,
                                const sal_Unicode* pUnicodes,
                                sal_Int16 nLen,
                                const sal_Int32* pDeltaArray,
                                sal_Bool bUseGlyphs=sal_True);
    sal_Bool        PSUploadEncoding(osl::File* pOutFile, PrinterGfx &rGfx);
    sal_Bool        PSUploadFont (osl::File& rOutFile, PrinterGfx &rGfx, bool bAsType42, std::list< OString >& rSuppliedFonts );
};


} /* namespace psp */

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
