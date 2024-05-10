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

#include <sal/config.h>

#include <string_view>

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <config_options.h>

enum class SubsFontFlags
{
    ONLYONE    = 0x01,
    MS         = 0x02
};

namespace o3tl
{
    template<> struct typed_flags<SubsFontFlags> : is_typed_flags<SubsFontFlags, 0x03> {};
}

UNOTOOLS_DLLPUBLIC OUString GetSubsFontName( std::u16string_view rName, SubsFontFlags nFlags );

UNLESS_MERGELIBS(UNOTOOLS_DLLPUBLIC) void AddTokenFontName( OUString& rName, std::u16string_view rNewToken );


class UNLESS_MERGELIBS(UNOTOOLS_DLLPUBLIC) ConvertChar
{
public:
    const sal_Unicode*  mpCvtTab;
    OUString            maSubsFontName;
    sal_Unicode         (*mpCvtFunc)( sal_Unicode );
    sal_Unicode         RecodeChar( sal_Unicode c ) const;
    void                RecodeString( OUString& rStra, sal_Int32 nIndex, sal_Int32 nLen ) const;
    static const ConvertChar* GetRecodeData( std::u16string_view rOrgFontName, std::u16string_view rMapFontName );
};


enum class DefaultFontType
{
    SANS_UNICODE        = 1,
    SANS                = 2,
    SERIF               = 3,
    FIXED               = 4,
    SYMBOL              = 5,
    UI_SANS             = 1000,
    UI_FIXED            = 1001,
    LATIN_TEXT          = 2000,
    LATIN_PRESENTATION  = 2001,
    LATIN_SPREADSHEET   = 2002,
    LATIN_HEADING       = 2003,
    LATIN_DISPLAY       = 2004,
    LATIN_FIXED         = 2005,
    CJK_TEXT            = 3000,
    CJK_PRESENTATION    = 3001,
    CJK_SPREADSHEET     = 3002,
    CJK_HEADING         = 3003,
    CJK_DISPLAY         = 3004,
    CTL_TEXT            = 4000,
    CTL_PRESENTATION    = 4001,
    CTL_SPREADSHEET     = 4002,
    CTL_HEADING         = 4003,
    CTL_DISPLAY         = 4004,
};

UNOTOOLS_DLLPUBLIC std::u16string_view GetNextFontToken( std::u16string_view rTokenStr, sal_Int32& rIndex );
UNOTOOLS_DLLPUBLIC OUString GetEnglishSearchFontName( std::u16string_view rName );

/** Strip any "script font suffix" from the font name

    Related: fdo#49271 RTF files often contain weird-ass
    Win 3.1/Win95 style fontnames which attempt to put the
    charset encoding into the filename
    http://www.webcenter.ru/~kazarn/eng/fonts_ttf.htm
*/
UNOTOOLS_DLLPUBLIC OUString StripScriptFromName(const OUString& rName);

/** Determine if the font is the special Open|Star Symbol font

    @param rFontName
    The FontName to test for being Open|Star Symbol

    @return true if this is Open|Star Symbol
*/
// FIXME It's quite possible that code using this should instead check for RTL_TEXTENCODING_SYMBOL.
UNOTOOLS_DLLPUBLIC bool IsOpenSymbol(std::u16string_view rFontName);

#endif // INCLUDED_UNOTOOLS_FONTDEFS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
