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
        { u"en",    RTL_TEXTENCODING_MS_1252 }, // Most used -> first in list
        { u"th",    RTL_TEXTENCODING_MS_874 },
        { u"ja",    RTL_TEXTENCODING_MS_932 },
        { u"zh-cn", RTL_TEXTENCODING_MS_936 },  // Chinese (simplified) - must go before "zh"
        { u"ko",    RTL_TEXTENCODING_MS_949 },
        { u"zh",    RTL_TEXTENCODING_MS_950 },  // Chinese (traditional)
        { u"bs",    RTL_TEXTENCODING_MS_1250 },
        { u"cs",    RTL_TEXTENCODING_MS_1250 },
        { u"hr",    RTL_TEXTENCODING_MS_1250 },
        { u"hu",    RTL_TEXTENCODING_MS_1250 },
        { u"pl",    RTL_TEXTENCODING_MS_1250 },
        { u"ro",    RTL_TEXTENCODING_MS_1250 },
        { u"sk",    RTL_TEXTENCODING_MS_1250 },
        { u"sl",    RTL_TEXTENCODING_MS_1250 },
//        { "sr",    RTL_TEXTENCODING_MS_1250 },
        { u"sq",    RTL_TEXTENCODING_MS_1250 },
        { u"be",    RTL_TEXTENCODING_MS_1251 },
        { u"bg",    RTL_TEXTENCODING_MS_1251 },
        { u"mk",    RTL_TEXTENCODING_MS_1251 },
        { u"ru",    RTL_TEXTENCODING_MS_1251 },
        { u"sr",    RTL_TEXTENCODING_MS_1251 },
        { u"uk",    RTL_TEXTENCODING_MS_1251 },
        { u"es",    RTL_TEXTENCODING_MS_1252 },
        { u"el",    RTL_TEXTENCODING_MS_1253 },
        { u"tr",    RTL_TEXTENCODING_MS_1254 },
        { u"he",    RTL_TEXTENCODING_MS_1255 },
        { u"ar",    RTL_TEXTENCODING_MS_1256 },
        { u"et",    RTL_TEXTENCODING_MS_1257 },
        { u"lt",    RTL_TEXTENCODING_MS_1257 },
        { u"lv",    RTL_TEXTENCODING_MS_1257 },
        { u"vi",    RTL_TEXTENCODING_MS_1258 },
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
        { u"de",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { u"en-us", RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { u"fi",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { u"fr-ca", RTL_TEXTENCODING_IBM_863 }, // OEM French Canadian; French Canadian (DOS)
        { u"fr",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { u"it",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { u"nl",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { u"sv",    RTL_TEXTENCODING_IBM_437 }, // OEM United States
        { u"el",    RTL_TEXTENCODING_IBM_737 }, // OEM Greek (formerly 437G); Greek (DOS)
        { u"et",    RTL_TEXTENCODING_IBM_775 }, // OEM Baltic; Baltic (DOS)
        { u"lt",    RTL_TEXTENCODING_IBM_775 }, // OEM Baltic; Baltic (DOS)
        { u"lv",    RTL_TEXTENCODING_IBM_775 }, // OEM Baltic; Baltic (DOS)
        { u"en",    RTL_TEXTENCODING_IBM_850 }, // OEM Multilingual Latin 1; Western European (DOS)
        { u"bs",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { u"cs",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { u"hr",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { u"hu",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { u"pl",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { u"ro",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { u"sk",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { u"sl",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
//        { "sr",    RTL_TEXTENCODING_IBM_852 }, // OEM Latin 2; Central European (DOS)
        { u"bg",    RTL_TEXTENCODING_IBM_855 }, // OEM Cyrillic (primarily Russian)
        { u"mk",    RTL_TEXTENCODING_IBM_855 }, // OEM Cyrillic (primarily Russian)
        { u"sr",    RTL_TEXTENCODING_IBM_855 }, // OEM Cyrillic (primarily Russian)
        { u"tr",    RTL_TEXTENCODING_IBM_857 }, // OEM Turkish; Turkish (DOS)
        { u"pt",    RTL_TEXTENCODING_IBM_860 }, // OEM Portuguese; Portuguese (DOS)
        { u"is",    RTL_TEXTENCODING_IBM_861 }, // OEM Icelandic; Icelandic (DOS)
        { u"he",    RTL_TEXTENCODING_IBM_862 }, // OEM Hebrew; Hebrew (DOS)
        { u"ar",    RTL_TEXTENCODING_IBM_864 }, // OEM Arabic; Arabic (864)
        { u"da",    RTL_TEXTENCODING_IBM_865 }, // OEM Nordic; Nordic (DOS)
        { u"nn",    RTL_TEXTENCODING_IBM_865 }, // OEM Nordic; Nordic (DOS)
        { u"be",    RTL_TEXTENCODING_IBM_866 }, // OEM Russian; Cyrillic (DOS)
        { u"ru",    RTL_TEXTENCODING_IBM_866 }, // OEM Russian; Cyrillic (DOS)
        { u"uk",    RTL_TEXTENCODING_IBM_866 }, // OEM Russian; Cyrillic (DOS)
        { u"th",    RTL_TEXTENCODING_MS_874 },  // ANSI/OEM Thai (ISO 8859-11); Thai (Windows)
        { u"ja",    RTL_TEXTENCODING_MS_932 },  // ANSI/OEM Japanese; Japanese (Shift-JIS)
        { u"zh-cn", RTL_TEXTENCODING_MS_936 },  // ANSI/OEM Simplified Chinese (PRC, Singapore); Chinese Simplified (GB2312)
        { u"ko",    RTL_TEXTENCODING_MS_949 },  // ANSI/OEM Korean (Unified Hangul Code)
        { u"zh",    RTL_TEXTENCODING_MS_950 },  // ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC); Chinese Traditional (Big5)
        { u"vi",    RTL_TEXTENCODING_MS_1258 }, // ANSI/OEM Vietnamese; Vietnamese (Windows)
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
