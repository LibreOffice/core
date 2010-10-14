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
#ifndef _UNOTOOLS_FONTDEFS_HXX
#define _UNOTOOLS_FONTDEFS_HXX

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <tools/string.hxx>

namespace utl {
  class FontSubstConfiguration;
  struct FontNameAttr;
}

// ----------------
// - SubsFontName -
// ----------------

#define SUBSFONT_ONLYONE    ((ULONG)0x00000001)
#define SUBSFONT_MS         ((ULONG)0x00000002)
#define SUBSFONT_PS         ((ULONG)0x00000004)
#define SUBSFONT_HTML       ((ULONG)0x00000008)

UNOTOOLS_DLLPUBLIC String GetSubsFontName( const String& rName, ULONG nFlags );

// -----------------
// - FontTokenName -
// -----------------

UNOTOOLS_DLLPUBLIC String GetFontToken( const String& rName, xub_StrLen nToken, xub_StrLen& rIndex );
inline String GetFontToken( const String& rName, xub_StrLen nToken )
{
    xub_StrLen nTempIndex = 0;
    return GetFontToken( rName, nToken, nTempIndex );
}

UNOTOOLS_DLLPUBLIC void AddTokenFontName( String& rName, const String& rNewToken );

struct UNOTOOLS_DLLPUBLIC FontNameHash { int operator()(const String&) const; };

// ---------------
// - ConvertChar -
// ---------------

class UNOTOOLS_DLLPUBLIC ConvertChar
{
public:
    const sal_Unicode*  mpCvtTab;
    const char*         mpSubsFontName;
    sal_Unicode         (*mpCvtFunc)( sal_Unicode );
    sal_Unicode         RecodeChar( sal_Unicode c ) const;
    void                RecodeString( String& rStra, xub_StrLen nIndex, xub_StrLen nLen ) const;
    static const ConvertChar* GetRecodeData( const String& rOrgFontName, const String& rMapFontName );
};


// Default-Font
#define DEFAULTFONT_SANS_UNICODE        ((USHORT)1)
#define DEFAULTFONT_SANS                ((USHORT)2)
#define DEFAULTFONT_SERIF               ((USHORT)3)
#define DEFAULTFONT_FIXED               ((USHORT)4)
#define DEFAULTFONT_SYMBOL              ((USHORT)5)
#define DEFAULTFONT_UI_SANS             ((USHORT)1000)
#define DEFAULTFONT_UI_FIXED            ((USHORT)1001)
#define DEFAULTFONT_LATIN_TEXT          ((USHORT)2000)
#define DEFAULTFONT_LATIN_PRESENTATION  ((USHORT)2001)
#define DEFAULTFONT_LATIN_SPREADSHEET   ((USHORT)2002)
#define DEFAULTFONT_LATIN_HEADING       ((USHORT)2003)
#define DEFAULTFONT_LATIN_DISPLAY       ((USHORT)2004)
#define DEFAULTFONT_LATIN_FIXED         ((USHORT)2005)
#define DEFAULTFONT_CJK_TEXT            ((USHORT)3000)
#define DEFAULTFONT_CJK_PRESENTATION    ((USHORT)3001)
#define DEFAULTFONT_CJK_SPREADSHEET     ((USHORT)3002)
#define DEFAULTFONT_CJK_HEADING         ((USHORT)3003)
#define DEFAULTFONT_CJK_DISPLAY         ((USHORT)3004)
#define DEFAULTFONT_CTL_TEXT            ((USHORT)4000)
#define DEFAULTFONT_CTL_PRESENTATION    ((USHORT)4001)
#define DEFAULTFONT_CTL_SPREADSHEET     ((USHORT)4002)
#define DEFAULTFONT_CTL_HEADING         ((USHORT)4003)
#define DEFAULTFONT_CTL_DISPLAY         ((USHORT)4004)

UNOTOOLS_DLLPUBLIC String GetNextFontToken( const String& rTokenStr, xub_StrLen& rIndex );

UNOTOOLS_DLLPUBLIC void GetEnglishSearchFontName( String& rName );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
