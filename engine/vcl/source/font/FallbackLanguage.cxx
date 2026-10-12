/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <font/FallbackLanguage.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nutil/unicode.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <unicode/uchar.h>
#include <unicode/uscript.h>

namespace vcl::font
{
bool isEmoji(sal_uInt32 nChar) { return u_hasBinaryProperty(nChar, UCHAR_EMOJI); }

bool isImpossibleCodePointForLang(const LanguageTag& rLangTag, sal_uInt32 nChar)
{
    //a non-default script is set, let's believe it
    if (rLangTag.hasScript())
        return false;

    int32_t script = u_getIntPropertyValue(nChar, UCHAR_SCRIPT);
    UScriptCode eScript = static_cast<UScriptCode>(script);
    bool bIsImpossible = false;
    OUString sLang = rLangTag.getLanguage();
    switch (eScript)
    {
        //http://en.wiktionary.org/wiki/Category:Oriya_script_languages
        case USCRIPT_ORIYA:
            bIsImpossible = sLang != "or" && sLang != "kxv";
            break;
        //http://en.wiktionary.org/wiki/Category:Telugu_script_languages
        case USCRIPT_TELUGU:
            bIsImpossible = sLang != "te" && sLang != "gon" && sLang != "kfc";
            break;
        //http://en.wiktionary.org/wiki/Category:Bengali_script_languages
        case USCRIPT_BENGALI:
            bIsImpossible = sLang != "bn" && sLang != "as" && sLang != "bpy" && sLang != "ctg"
                            && sLang != "sa";
            break;
        // Chinese, Japanese and Korean fonts all claim to support the common
        // Western languages, so a Western language tag on CJK text cannot
        // tell them apart and leaves the choice to the order of the font list
        case USCRIPT_HAN:
            bIsImpossible = sLang != "zh" && sLang != "ja" && sLang != "ko" && sLang != "yue"
                            && sLang != "lzh";
            break;
        case USCRIPT_HANGUL:
            bIsImpossible = sLang != "ko";
            break;
        case USCRIPT_HIRAGANA:
        case USCRIPT_KATAKANA:
        case USCRIPT_KATAKANA_OR_HIRAGANA:
            bIsImpossible = sLang != "ja";
            break;
        case USCRIPT_BOPOMOFO:
            bIsImpossible = sLang != "zh";
            break;
        default:
            break;
    }
    SAL_WARN_IF(bIsImpossible, "vcl.fonts",
                "In glyph fallback throwing away the language property of "
                    << sLang << " because the detected script for '0x"
                    << OUString::number(nChar, 16) << "' is " << uscript_getName(eScript)
                    << " and that language doesn't make sense. Autodetecting instead.");
    return bIsImpossible;
}

OUString getExemplarLangTagForCodePoint(sal_uInt32 nChar)
{
    if (isEmoji(nChar))
        return u"und-zsye"_ustr;
    int32_t script = u_getIntPropertyValue(nChar, UCHAR_SCRIPT);
    UScriptCode eScript = static_cast<UScriptCode>(script);
    switch (eScript)
    {
        // the CJK languages are looked up by region, a bare "zh" is unknown to
        // fontconfig and to the default font lists
        case USCRIPT_HAN:
        {
            // the default Asian language of the configuration, which is
            // Simplified Chinese unless set to something else
            LanguageType eAsian = MsLangId::resolveSystemLanguageByScriptType(
                LANGUAGE_SYSTEM, css::i18n::ScriptType::ASIAN);
            return LanguageTag(eAsian).getBcp47();
        }
        case USCRIPT_HANGUL:
            return u"ko-KR"_ustr;
        case USCRIPT_HIRAGANA:
        case USCRIPT_KATAKANA:
        case USCRIPT_KATAKANA_OR_HIRAGANA:
            return u"ja-JP"_ustr;
        case USCRIPT_BOPOMOFO:
            return u"zh-TW"_ustr;
        default:
            break;
    }
    OStringBuffer aBuf(unicode::getExemplarLanguageForUScriptCode(eScript));
    if (const char* pScriptCode = uscript_getShortName(eScript))
        aBuf.append(OStringChar('-') + pScriptCode);
    return OStringToOUString(aBuf, RTL_TEXTENCODING_UTF8);
}

LanguageTag getLanguageForMissingChars(const LanguageTag& rLangTag, const OUString& rMissingChars)
{
    for (sal_Int32 nStrIndex = 0; nStrIndex < rMissingChars.getLength();)
    {
        const sal_uInt32 nChar = rMissingChars.iterateCodePoints(&nStrIndex);
        if (isImpossibleCodePointForLang(rLangTag, nChar))
            return LanguageTag(getExemplarLangTagForCodePoint(nChar));
    }
    return rLangTag;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
