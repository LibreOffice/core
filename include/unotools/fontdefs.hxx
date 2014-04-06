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
#ifndef INCLUDED_UNOTOOLS_FONTDEFS_HXX
#define INCLUDED_UNOTOOLS_FONTDEFS_HXX

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <tools/solar.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

namespace utl {
  class FontSubstConfiguration;
  struct FontNameAttr;
}

// - SubsFontName -

#define SUBSFONT_ONLYONE    ((sal_uLong)0x00000001)
#define SUBSFONT_MS         ((sal_uLong)0x00000002)
#define SUBSFONT_PS         ((sal_uLong)0x00000004)
#define SUBSFONT_HTML       ((sal_uLong)0x00000008)

UNOTOOLS_DLLPUBLIC OUString GetSubsFontName( const OUString& rName, sal_uLong nFlags );

UNOTOOLS_DLLPUBLIC void AddTokenFontName( OUString& rName, const OUString& rNewToken );

struct UNOTOOLS_DLLPUBLIC FontNameHash { int operator()(const OUString&) const; };

// - ConvertChar -

class UNOTOOLS_DLLPUBLIC ConvertChar
{
public:
    const sal_Unicode*  mpCvtTab;
    const char*         mpSubsFontName;
    sal_Unicode         (*mpCvtFunc)( sal_Unicode );
    sal_Unicode         RecodeChar( sal_Unicode c ) const;
    void                RecodeString( OUString& rStra, sal_Int32 nIndex, sal_Int32 nLen ) const;
    static const ConvertChar* GetRecodeData( const OUString& rOrgFontName, const OUString& rMapFontName );
};

// Default-Font
#define DEFAULTFONT_SANS_UNICODE        ((sal_uInt16)1)
#define DEFAULTFONT_SANS                ((sal_uInt16)2)
#define DEFAULTFONT_SERIF               ((sal_uInt16)3)
#define DEFAULTFONT_FIXED               ((sal_uInt16)4)
#define DEFAULTFONT_SYMBOL              ((sal_uInt16)5)
#define DEFAULTFONT_UI_SANS             ((sal_uInt16)1000)
#define DEFAULTFONT_UI_FIXED            ((sal_uInt16)1001)
#define DEFAULTFONT_LATIN_TEXT          ((sal_uInt16)2000)
#define DEFAULTFONT_LATIN_PRESENTATION  ((sal_uInt16)2001)
#define DEFAULTFONT_LATIN_SPREADSHEET   ((sal_uInt16)2002)
#define DEFAULTFONT_LATIN_HEADING       ((sal_uInt16)2003)
#define DEFAULTFONT_LATIN_DISPLAY       ((sal_uInt16)2004)
#define DEFAULTFONT_LATIN_FIXED         ((sal_uInt16)2005)
#define DEFAULTFONT_CJK_TEXT            ((sal_uInt16)3000)
#define DEFAULTFONT_CJK_PRESENTATION    ((sal_uInt16)3001)
#define DEFAULTFONT_CJK_SPREADSHEET     ((sal_uInt16)3002)
#define DEFAULTFONT_CJK_HEADING         ((sal_uInt16)3003)
#define DEFAULTFONT_CJK_DISPLAY         ((sal_uInt16)3004)
#define DEFAULTFONT_CTL_TEXT            ((sal_uInt16)4000)
#define DEFAULTFONT_CTL_PRESENTATION    ((sal_uInt16)4001)
#define DEFAULTFONT_CTL_SPREADSHEET     ((sal_uInt16)4002)
#define DEFAULTFONT_CTL_HEADING         ((sal_uInt16)4003)
#define DEFAULTFONT_CTL_DISPLAY         ((sal_uInt16)4004)

UNOTOOLS_DLLPUBLIC OUString GetNextFontToken( const OUString& rTokenStr, sal_Int32& rIndex );

UNOTOOLS_DLLPUBLIC void GetEnglishSearchFontName( OUString& rName );

/** Determine if the font is the special Star|Open Symbol font

    @param rFontName
    The FontName to test for being Star|Open Symbol

    @return true if this is Star|Open Symbol
*/
// FIXME It's quite possible that code using this should instead check for RTL_TEXTENCODING_SYMBOL.
UNOTOOLS_DLLPUBLIC bool IsStarSymbol(const OUString &rFontName);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
