/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotools/wincodepage.hxx>
#include "rtl/string.h"
#include "rtl/textenc.h"

namespace{

// See https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756
rtl_TextEncoding impl_getWinTextEncodingFromLangStrANSI(const char* pLanguage)
{
    auto nLangLen = rtl_str_getLength(pLanguage);

    struct LangEncodingDef
    {
        const char*        mpLangStr;
        decltype(nLangLen) mnLangStrLen;
        rtl_TextEncoding   meTextEncoding;
    };
    static LangEncodingDef const aLanguageTab[] =
    {
        { "en",    2, RTL_TEXTENCODING_MS_1252 }, // Most used -> first in list
        { "th",    2, RTL_TEXTENCODING_MS_874 },
        { "ja",    2, RTL_TEXTENCODING_MS_932 },
        { "zh-cn", 5, RTL_TEXTENCODING_MS_936 },  // Chinese (simplified) - must go before "zh"
        { "ko",    2, RTL_TEXTENCODING_MS_949 },
        { "zh",    2, RTL_TEXTENCODING_MS_950 },  // Chinese (traditional)
        { "bs",    2, RTL_TEXTENCODING_MS_1250 },
        { "cs",    2, RTL_TEXTENCODING_MS_1250 },
        { "hr",    2, RTL_TEXTENCODING_MS_1250 },
        { "hu",    2, RTL_TEXTENCODING_MS_1250 },
        { "pl",    2, RTL_TEXTENCODING_MS_1250 },
        { "ro",    2, RTL_TEXTENCODING_MS_1250 },
        { "sk",    2, RTL_TEXTENCODING_MS_1250 },
        { "sl",    2, RTL_TEXTENCODING_MS_1250 },
//        { "sr",    2, RTL_TEXTENCODING_MS_1250 },
        { "sq",    2, RTL_TEXTENCODING_MS_1250 },
        { "be",    2, RTL_TEXTENCODING_MS_1251 },
        { "bg",    2, RTL_TEXTENCODING_MS_1251 },
        { "mk",    2, RTL_TEXTENCODING_MS_1251 },
        { "ru",    2, RTL_TEXTENCODING_MS_1251 },
        { "sr",    2, RTL_TEXTENCODING_MS_1251 },
        { "uk",    2, RTL_TEXTENCODING_MS_1251 },
        { "es",    2, RTL_TEXTENCODING_MS_1252 },
        { "el",    2, RTL_TEXTENCODING_MS_1253 },
        { "tr",    2, RTL_TEXTENCODING_MS_1254 },
        { "he",    2, RTL_TEXTENCODING_MS_1255 },
        { "ar",    2, RTL_TEXTENCODING_MS_1256 },
        { "et",    2, RTL_TEXTENCODING_MS_1257 },
        { "lt",    2, RTL_TEXTENCODING_MS_1257 },
        { "lv",    2, RTL_TEXTENCODING_MS_1257 },
        { "vi",    2, RTL_TEXTENCODING_MS_1258 },
    };

    for (auto& def : aLanguageTab)
    {
        if (rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(pLanguage, nLangLen,
                                                               def.mpLangStr, def.mnLangStrLen,
                                                               def.mnLangStrLen) == 0)
        {
            return def.meTextEncoding;
        }
    }

    return RTL_TEXTENCODING_MS_1252;
}

/* ----------------------------------------------------------------------- */

// See https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756
// See http://shapelib.maptools.org/codepage.html
rtl_TextEncoding impl_getWinTextEncodingFromLangStrOEM(const char* pLanguage)
{
    auto nLangLen = rtl_str_getLength(pLanguage);

    struct LangEncodingDef
    {
        const char*        mpLangStr;
        decltype(nLangLen) mnLangStrLen;
        rtl_TextEncoding   meTextEncoding;
    };
    static LangEncodingDef const aLanguageTab[] =
    {
        { "de",    2, RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "en-us", 5, RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "fi",    2, RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "fr-ca", 5, RTL_TEXTENCODING_IBM_863 }, // OEM French Canadian; French Canadian (DOS)
        { "fr",    2, RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "it",    2, RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "nl",    2, RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "sv",    2, RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { "el",    2, RTL_TEXTENCODING_IBM_737 }, // OEM Greek (formerly 437G); Greek (DOS)
        { "et",    2, RTL_TEXTENCODING_IBM_775 }, // OEM Baltic; Baltic (DOS)
        { "lt",    2, RTL_TEXTENCODING_IBM_775 }, // OEM Baltic; Baltic (DOS)
        { "lv",    2, RTL_TEXTENCODING_IBM_775 }, // OEM Baltic; Baltic (DOS)
        { "en",    2, RTL_TEXTENCODING_IBM_850 }, // OEM Multilingual Latin 1; Western European (DOS)
        { "bs",    2, RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "cs",    2, RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "hr",    2, RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "hu",    2, RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "pl",    2, RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "ro",    2, RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "sk",    2, RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "sl",    2, RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
//        { "sr",    2, RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { "bg",    2, RTL_TEXTENCODING_IBM_855 }, // OEM Cyrillic (primarily Russian)
        { "mk",    2, RTL_TEXTENCODING_IBM_855 }, // OEM Cyrillic (primarily Russian)
        { "sr",    2, RTL_TEXTENCODING_IBM_855 }, // OEM Cyrillic (primarily Russian)
        { "tr",    2, RTL_TEXTENCODING_IBM_857 }, // OEM Turkish; Turkish (DOS)
        { "pt",    2, RTL_TEXTENCODING_IBM_860 }, // OEM Portuguese; Portuguese (DOS)
        { "is",    2, RTL_TEXTENCODING_IBM_861 }, // OEM Icelandic; Icelandic (DOS)
        { "he",    2, RTL_TEXTENCODING_IBM_862 }, // OEM Hebrew; Hebrew (DOS)
        { "ar",    2, RTL_TEXTENCODING_IBM_864 }, // OEM Arabic; Arabic (864)
        { "da",    2, RTL_TEXTENCODING_IBM_865 }, // OEM Nordic; Nordic (DOS)
        { "nn",    2, RTL_TEXTENCODING_IBM_865 }, // OEM Nordic; Nordic (DOS)
        { "be",    2, RTL_TEXTENCODING_IBM_866 }, // OEM Russian; Cyrillic (DOS)
        { "ru",    2, RTL_TEXTENCODING_IBM_866 }, // OEM Russian; Cyrillic (DOS)
        { "uk",    2, RTL_TEXTENCODING_IBM_866 }, // OEM Russian; Cyrillic (DOS)
        { "th",    2, RTL_TEXTENCODING_MS_874 },  // ANSI/OEM Thai (ISO 8859-11); Thai (Windows)
        { "ja",    2, RTL_TEXTENCODING_MS_932 },  // ANSI/OEM Japanese; Japanese (Shift-JIS)
        { "zh-cn", 5, RTL_TEXTENCODING_MS_936 },  // ANSI/OEM Simplified Chinese (PRC, Singapore); Chinese Simplified (GB2312)
        { "ko",    2, RTL_TEXTENCODING_MS_949 },  // ANSI/OEM Korean (Unified Hangul Code)
        { "zh",    2, RTL_TEXTENCODING_MS_950 },  // ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC); Chinese Traditional (Big5)
        { "vi",    2, RTL_TEXTENCODING_MS_1258 }, // ANSI/OEM Vietnamese; Vietnamese (Windows)
    };

    for (auto& def : aLanguageTab)
    {
        if (rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(pLanguage, nLangLen,
                                                               def.mpLangStr, def.mnLangStrLen,
                                                               def.mnLangStrLen) == 0)
        {
            return def.meTextEncoding;
        }
    }

    return RTL_TEXTENCODING_IBM_850;
}

} // namespace

rtl_TextEncoding utl_getWinTextEncodingFromLangStr(const char* pLanguage, bool bOEM)
{
    return bOEM ?
        impl_getWinTextEncodingFromLangStrOEM(pLanguage) :
        impl_getWinTextEncodingFromLangStrANSI(pLanguage);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
