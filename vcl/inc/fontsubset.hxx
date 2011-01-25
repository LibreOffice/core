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

#ifndef _SV_FONTSUBSET_HXX
#define _SV_FONTSUBSET_HXX

#include <tools/gen.hxx>
#include <tools/string.hxx>
#include <cstdio>

#include "vcl/dllapi.h"

namespace vcl { struct _TrueTypeFont; } // SFT's idea of a TTF font

class VCL_DLLPUBLIC FontSubsetInfo
{
public:
    explicit    FontSubsetInfo( void );
    virtual     ~FontSubsetInfo( void );

    enum FontType {
        NO_FONT     = 0,
        SFNT_TTF    = 1<<1,     // SFNT container with TrueType glyphs
        SFNT_CFF    = 1<<2,     // SFNT container with CFF-container
        TYPE1_PFA   = 1<<3,     // PSType1 Postscript Font Ascii
        TYPE1_PFB   = 1<<4,     // PSType1 Postscript Font Binary
        CFF_FONT    = 1<<5,     // CFF-container with PSType2 glyphs
        TYPE3_FONT  = 1<<6,     // PSType3 Postscript font
        TYPE42_FONT = 1<<7,     // PSType42 wrapper for an SFNT_TTF
        ANY_SFNT    = SFNT_TTF | SFNT_CFF,
        ANY_TYPE1   = TYPE1_PFA | TYPE1_PFB,
        ANY_FONT    = 0xFF
    };

    bool        LoadFont( FontType eInFontType,
                    const unsigned char* pFontBytes, int nByteLength );
    bool        LoadFont( vcl::_TrueTypeFont* pSftTrueTypeFont );

    bool        CreateFontSubset( int nOutFontTypeMask,
                    FILE* pOutFile, const char* pOutFontName,
                    const long* pReqGlyphIds, const sal_uInt8* pEncodedIds,
                    int nReqGlyphCount, sal_Int32* pOutGlyphWidths = NULL );

public: // TODO: make subsetter results private and provide accessor methods instead
    // subsetter-provided subset details needed by e.g. Postscript or PDF
    String      m_aPSName;
    int         m_nAscent; // all metrics in PS font units
    int         m_nDescent;
    int         m_nCapHeight;
    Rectangle   m_aFontBBox;
    FontType    m_nFontType;    // font-type of subset result

private:
    // input-font-specific details
    unsigned const char*    mpInFontBytes;
    int                     mnInByteLength;
    FontType                meInFontType;   // allowed mask of input font-types
    vcl::_TrueTypeFont*     mpSftTTFont;

    // subset-request details
    int                     mnReqFontTypeMask;  // allowed subset-target font types
    FILE*                   mpOutFile;
    const char*             mpReqFontName;
    const long*             mpReqGlyphIds;
    const sal_uInt8*        mpReqEncodedIds;
    int                     mnReqGlyphCount;

protected:
    bool    CreateFontSubsetFromCff( sal_Int32* pOutGlyphWidths = NULL );
    bool    CreateFontSubsetFromSfnt( sal_Int32* pOutGlyphWidths = NULL );
    bool    CreateFontSubsetFromType1( sal_Int32* pOutGlyphWidths = NULL );
};

#endif // _SV_FONTSUBSET_HXX

