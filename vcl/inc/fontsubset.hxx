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

#ifndef INCLUDED_VCL_INC_FONTSUBSET_HXX
#define INCLUDED_VCL_INC_FONTSUBSET_HXX

#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <o3tl/typed_flags_set.hxx>

#include "sallayout.hxx"

namespace vcl { struct TrueTypeFont; }         ///< SFT's idea of a TTF font

enum class FontType {
    NO_FONT     = 0,
    SFNT_TTF    = 1<<1,                     ///< SFNT container with TrueType glyphs
    SFNT_CFF    = 1<<2,                     ///< SFNT container with CFF-container
    TYPE1_PFA   = 1<<3,                     ///< PSType1 Postscript Font Ascii
    TYPE1_PFB   = 1<<4,                     ///< PSType1 Postscript Font Binary
    CFF_FONT    = 1<<5,                     ///< CFF-container with PSType2 glyphs
    TYPE3_FONT  = 1<<6,                     ///< PSType3 Postscript font
    TYPE42_FONT = 1<<7,                     ///< PSType42 wrapper for an SFNT_TTF
    ANY_SFNT    = SFNT_TTF | SFNT_CFF,
    ANY_TYPE1   = TYPE1_PFA | TYPE1_PFB
};
namespace o3tl {
    template<> struct typed_flags<FontType> : is_typed_flags<FontType, (1<<8)-1> {};
}

class VCL_DLLPUBLIC FontSubsetInfo final
{
public:
    explicit    FontSubsetInfo();
                ~FontSubsetInfo();

    void        LoadFont( FontType eInFontType,
                    const unsigned char* pFontBytes, int nByteLength );
    void        LoadFont( vcl::TrueTypeFont* pSftTrueTypeFont );

    bool        CreateFontSubset( FontType nOutFontTypeMask,
                    FILE* pOutFile, const char* pOutFontName,
                    const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncodedIds,
                    int nReqGlyphCount, sal_Int32* pOutGlyphWidths = nullptr );

public: // TODO: make subsetter results private and provide accessor methods instead
        // subsetter-provided subset details needed by e.g. Postscript or PDF
    OUString                m_aPSName;
    int                     m_nAscent;          ///< all metrics in PS font units
    int                     m_nDescent;
    int                     m_nCapHeight;
    tools::Rectangle               m_aFontBBox;
    FontType                m_nFontType;        ///< font-type of subset result

private:
    // input-font-specific details
    unsigned const char*    mpInFontBytes;
    int                     mnInByteLength;
    FontType                meInFontType;       ///< allowed mask of input font-types
    vcl::TrueTypeFont*      mpSftTTFont;

    // subset-request details
    FontType                mnReqFontTypeMask;  ///< allowed subset-target font types
    FILE*                   mpOutFile;
    const char*             mpReqFontName;
    const sal_GlyphId*      mpReqGlyphIds;
    const sal_uInt8*        mpReqEncodedIds;
    int                     mnReqGlyphCount;

    bool    CreateFontSubsetFromCff( sal_Int32* pOutGlyphWidths );
    bool    CreateFontSubsetFromSfnt( sal_Int32* pOutGlyphWidths );
    static bool CreateFontSubsetFromType1( const sal_Int32* pOutGlyphWidths );
};

#endif // INCLUDED_VCL_INC_FONTSUBSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
