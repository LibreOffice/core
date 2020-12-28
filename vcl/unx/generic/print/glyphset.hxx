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

#include <osl/file.hxx>

#include <rtl/string.hxx>
#include <vcl/glyphitem.hxx>

#include <string_view>
#include <vector>
#include <unordered_map>

class Point;

namespace psp {

class PrinterGfx;
class PrintFontManager;

class GlyphSet
{
private:

    sal_Int32           mnFontID;
    bool                mbVertical;
    OString             maBaseName;

    typedef std::unordered_map< sal_GlyphId, sal_uInt8 > glyph_map_t;
    std::vector< glyph_map_t > maGlyphList;

    OString     GetGlyphSetName (sal_Int32 nGlyphSetID);

    void        GetGlyphID (sal_GlyphId nGlyphId,
                                unsigned char* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    bool        LookupGlyphID (sal_GlyphId nGlyphId,
                                   unsigned char* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    void        AddGlyphID (sal_GlyphId nGlyphId,
                                unsigned char* nOutGlyphID,
                                sal_Int32* nOutGlyphSetID);
    static void     AddNotdef (glyph_map_t &rGlyphMap);

public:

    GlyphSet (sal_Int32 nFontID, bool bVertical);
    /* FIXME delete the glyphlist in ~GlyphSet ??? */

    sal_Int32       GetFontID () const { return mnFontID;}
    static OString
    GetReencodedFontName (rtl_TextEncoding nEnc,
                          std::string_view rFontName);

    bool            IsVertical () const { return mbVertical;}

    void            DrawGlyph (PrinterGfx& rGfx,
                               const Point& rPoint,
                               const sal_GlyphId nGlyphId);
    void        PSUploadFont (osl::File& rOutFile, PrinterGfx &rGfx, bool bAsType42, std::vector< OString >& rSuppliedFonts );
};

} /* namespace psp */

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
