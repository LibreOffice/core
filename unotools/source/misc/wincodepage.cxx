/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotools/wincodepage.hxx>
#include <rtl/string.h>
#include <rtl/textenc.h>

namespace{

struct LangEncodingDef
{
    const OUStringLiteral msLangStr;
    rtl_TextEncoding meTextEncoding;
};

// See https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756
rtl_TextEncoding impl_getWinTextEncodingFromLangStrANSI(const OUString& sLanguage)
{
    static constexpr LangEncodingDef aLanguageTab[] =
    {
        { "en",    RTL_TEXTENCODING_MS_1252 }, // Most used -> first in list
        { "th",    RTL_TEXTENCODING_MS_874 },
        { "ja",    RTL_TEXTENCODING_MS_932 },
        { "zh-cn", RTL_TEXTENCODING_MS_936 },  // Chinese (simplified) - must go before "zh"
        { "ko",    RTL_TEXTENCODING_MS_949 },
        { "zh",    RTL_TEXTENCODING_MS_950 },  // Chinese (traditional)
        { "bs",    RTL_TEXTENCODING_MS_1250 },
        { "cs",    RTL_TEXTENCODING_MS_1250 },
        { "hr",    RTL_TEXTENCODING_MS_1250 },
        { "hu",    RTL_TEXTENCODING_MS_1250 },
        { "pl",    RTL_TEXTENCODING_MS_1250 },
        { "ro",    RTL_TEXTENCODING_MS_1250 },
        { "sk",    RTL_TEXTENCODING_MS_1250 },
        { "sl",    RTL_TEXTENCODING_MS_1250 },
//        { "sr",    RTL_TEXTENCODING_MS_1250 },
        { "sq",    RTL_TEXTENCODING_MS_1250 },
        { "be",    RTL_TEXTENCODING_MS_1251 },
        { "bg",    RTL_TEXTENCODING_MS_1251 },
        { "mk",    RTL_TEXTENCODING_MS_1251 },
        { "ru",    RTL_TEXTENCODING_MS_1251 },
        { "sr",    RTL_TEXTENCODING_MS_1251 },
        { "uk",    RTL_TEXTENCODING_MS_1251 },
        { "es",    RTL_TEXTENCODING_MS_1252 },
        { "el",    RTL_TEXTENCODING_MS_1253 },
        { "tr",    RTL_TEXTENCODING_MS_1254 },
        { "he",    RTL_TEXTENCODING_MS_1255 },
        { "ar",    RTL_TEXTENCODING_MS_1256 },
        { "et",    RTL_TEXTENCODING_MS_1257 },
        { "lt",    RTL_TEXTENCODING_MS_1257 },
        { "lv",    RTL_TEXTENCODING_MS_1257 },
        { "vi",    RTL_TEXTENCODING_MS_1258 },
    };

    for (auto& def : aLanguageTab)
    {
        if (sLanguage.startsWithIgnoreAsciiCase(def.msLangStr))
            return def.meTextEncoding;
    }

    return RTL_TEXTENCODING_MS_1252;
}

/* ----------------------------------------------------------------------- */

// See https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756
// See http://shapelib.maptools.org/codepage.html
rtl_TextEncoding impl_getWinTextEncodingFromLangStrOEM(const OUString& sLanguage)
{
    static constexpr LangEncodingDef aLanguageTab[] =
    {
        { "de",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "en-us", RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "fi",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "fr-ca", RTL_TEXTENCODING_IBM_863 }, // OEM French Canadian; French Canadian (DOS)
        { "fr",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "it",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "nl",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "sv",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "el",    RTL_TEXTENCODING_IBM_737 }, // OEM Greek (formerly 437G); Greek (DOS)
        { "et",    RTL_TEXTENCODING_IBM_775 }, // OEM Baltic; Baltic (DOS)
        { "lt",    RTL_TEXTENCODING_IBM_775 }, // OEM Baltic; Baltic (DOS)
        { "lv",    RTL_TEXTENCODING_IBM_775 }, // OEM Baltic; Baltic (DOS)
        { "en",    RTL_TEXTENCODING_IBM_850 }, // OEM Multilingual Latin 1; Western European (DOS)
        { "bs",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "cs",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "hr",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "hu",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "pl",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "ro",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "sk",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "sl",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
//        { "sr",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "bg",    RTL_TEXTENCODING_IBM_855 }, // OEM Cyrillic (primarily Russian)
        { "mk",    RTL_TEXTENCODING_IBM_855 }, // OEM Cyrillic (primarily Russian)
        { "sr",    RTL_TEXTENCODING_IBM_855 }, // OEM Cyrillic (primarily Russian)
        { "tr",    RTL_TEXTENCODING_IBM_857 }, // OEM Turkish; Turkish (DOS)
        { "pt",    RTL_TEXTENCODING_IBM_860 }, // OEM Portuguese; Portuguese (DOS)
        { "is",    RTL_TEXTENCODING_IBM_861 }, // OEM Icelandic; Icelandic (DOS)
        { "he",    RTL_TEXTENCODING_IBM_862 }, // OEM Hebrew; Hebrew (DOS)
        { "ar",    RTL_TEXTENCODING_IBM_864 }, // OEM Arabic; Arabic (864)
        { "da",    RTL_TEXTENCODING_IBM_865 }, // OEM Nordic; Nordic (DOS)
        { "nn",    RTL_TEXTENCODING_IBM_865 }, // OEM Nordic; Nordic (DOS)
        { "be",    RTL_TEXTENCODING_IBM_866 }, // OEM Russian; Cyrillic (DOS)
        { "ru",    RTL_TEXTENCODING_IBM_866 }, // OEM Russian; Cyrillic (DOS)
        { "uk",    RTL_TEXTENCODING_IBM_866 }, // OEM Russian; Cyrillic (DOS)
        { "th",    RTL_TEXTENCODING_MS_874 },  // ANSI/OEM Thai (ISO 8859-11); Thai (Windows)
        { "ja",    RTL_TEXTENCODING_MS_932 },  // ANSI/OEM Japanese; Japanese (Shift-JIS)
        { "zh-cn", RTL_TEXTENCODING_MS_936 },  // ANSI/OEM Simplified Chinese (PRC, Singapore); Chinese Simplified (GB2312)
        { "ko",    RTL_TEXTENCODING_MS_949 },  // ANSI/OEM Korean (Unified Hangul Code)
        { "zh",    RTL_TEXTENCODING_MS_950 },  // ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC); Chinese Traditional (Big5)
        { "vi",    RTL_TEXTENCODING_MS_1258 }, // ANSI/OEM Vietnamese; Vietnamese (Windows)
    };

    for (auto& def : aLanguageTab)
    {
        if (sLanguage.startsWithIgnoreAsciiCase(def.msLangStr))
            return def.meTextEncoding;
    }

    return RTL_TEXTENCODING_IBM_850;
}

} // namespace

rtl_TextEncoding utl_getWinTextEncodingFromLangStr(const OUString& sLanguage, bool bOEM)
{
    return bOEM ?
        impl_getWinTextEncodingFromLangStrOEM(sLanguage) :
        impl_getWinTextEncodingFromLangStrANSI(sLanguage);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
