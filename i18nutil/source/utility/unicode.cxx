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

#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/languagetagicu.hxx>
#include <i18nutil/unicode.hxx>
#include <sal/log.hxx>
#include <unicode/numfmt.h>
#include <unicode/uchar.h>
#include "unicode_data.h"
#include <rtl/character.hxx>
#include <o3tl/string_view.hxx>
#include <memory>

// Workaround for glibc braindamage:
// glibc 2.4's langinfo.h does "#define CURRENCY_SYMBOL __CURRENCY_SYMBOL"
// which (obviously) breaks UnicodeType::CURRENCY_SYMBOL
#undef CURRENCY_SYMBOL

using namespace ::com::sun::star::i18n;

template<class L, typename T>
static T getScriptType( const sal_Unicode ch, const L* typeList, T unknownType ) {

    sal_Int16 i = 0;
    css::i18n::UnicodeScript type = typeList[0].to;
    while (type < UnicodeScript_kScriptCount && ch > UnicodeScriptType[static_cast<int>(type)][UnicodeScriptTypeTo]) {
        type = typeList[++i].to;
    }

    return (type < UnicodeScript_kScriptCount &&
            ch >= UnicodeScriptType[static_cast<int>(typeList[i].from)][int(UnicodeScriptTypeFrom)]) ?
            typeList[i].value : unknownType;
}

sal_Int16
unicode::getUnicodeScriptType( const sal_Unicode ch, const ScriptTypeList* typeList, sal_Int16 unknownType ) {
    return getScriptType(ch, typeList, unknownType);
}

sal_Unicode
unicode::getUnicodeScriptStart( UnicodeScript type) {
    return UnicodeScriptType[static_cast<int>(type)][UnicodeScriptTypeFrom];
}

sal_Unicode
unicode::getUnicodeScriptEnd( UnicodeScript type) {
    return UnicodeScriptType[static_cast<int>(type)][UnicodeScriptTypeTo];
}

sal_Int16
unicode::getUnicodeType(const sal_uInt32 ch)
{
    static sal_uInt32 c = 0x00;
    static sal_uInt32 r = 0x00;

    if (ch == c) return r;
    else c = ch;

    switch (u_charType(ch))
    {
        case U_UNASSIGNED:
            r = css::i18n::UnicodeType::UNASSIGNED;
            break;
        case U_UPPERCASE_LETTER:
            r = css::i18n::UnicodeType::UPPERCASE_LETTER;
            break;
        case U_LOWERCASE_LETTER:
            r = css::i18n::UnicodeType::LOWERCASE_LETTER;
            break;
        case U_TITLECASE_LETTER:
            r = css::i18n::UnicodeType::TITLECASE_LETTER;
            break;
        case U_MODIFIER_LETTER:
            r = css::i18n::UnicodeType::MODIFIER_LETTER;
            break;
        case U_OTHER_LETTER:
            r = css::i18n::UnicodeType::OTHER_LETTER;
            break;
        case U_NON_SPACING_MARK:
            r = css::i18n::UnicodeType::NON_SPACING_MARK;
            break;
        case U_ENCLOSING_MARK:
            r = css::i18n::UnicodeType::ENCLOSING_MARK;
            break;
        case U_COMBINING_SPACING_MARK:
            r = css::i18n::UnicodeType::COMBINING_SPACING_MARK;
            break;
        case U_DECIMAL_DIGIT_NUMBER:
            r = css::i18n::UnicodeType::DECIMAL_DIGIT_NUMBER;
            break;
        case U_LETTER_NUMBER:
            r = css::i18n::UnicodeType::LETTER_NUMBER;
            break;
        case U_OTHER_NUMBER:
            r = css::i18n::UnicodeType::OTHER_NUMBER;
            break;
        case U_SPACE_SEPARATOR:
            r = css::i18n::UnicodeType::SPACE_SEPARATOR;
            break;
        case U_LINE_SEPARATOR:
            r = css::i18n::UnicodeType::LINE_SEPARATOR;
            break;
        case U_PARAGRAPH_SEPARATOR:
            r = css::i18n::UnicodeType::PARAGRAPH_SEPARATOR;
            break;
        case U_CONTROL_CHAR:
            r = css::i18n::UnicodeType::CONTROL;
            break;
        case U_FORMAT_CHAR:
            r = css::i18n::UnicodeType::FORMAT;
            break;
        case U_PRIVATE_USE_CHAR:
            r = css::i18n::UnicodeType::PRIVATE_USE;
            break;
        case U_SURROGATE:
            r = css::i18n::UnicodeType::SURROGATE;
            break;
        case U_DASH_PUNCTUATION:
            r = css::i18n::UnicodeType::DASH_PUNCTUATION;
            break;
        case U_INITIAL_PUNCTUATION:
            r = css::i18n::UnicodeType::INITIAL_PUNCTUATION;
            break;
        case U_FINAL_PUNCTUATION:
            r = css::i18n::UnicodeType::FINAL_PUNCTUATION;
            break;
        case U_CONNECTOR_PUNCTUATION:
            r = css::i18n::UnicodeType::CONNECTOR_PUNCTUATION;
            break;
        case U_OTHER_PUNCTUATION:
            r = css::i18n::UnicodeType::OTHER_PUNCTUATION;
            break;
        case U_MATH_SYMBOL:
            r = css::i18n::UnicodeType::MATH_SYMBOL;
            break;
        case U_CURRENCY_SYMBOL:
            r = css::i18n::UnicodeType::CURRENCY_SYMBOL;
            break;
        case U_MODIFIER_SYMBOL:
            r = css::i18n::UnicodeType::MODIFIER_SYMBOL;
            break;
        case U_OTHER_SYMBOL:
            r = css::i18n::UnicodeType::OTHER_SYMBOL;
            break;
        case U_START_PUNCTUATION:
            r = css::i18n::UnicodeType::START_PUNCTUATION;
            break;
        case U_END_PUNCTUATION:
            r = css::i18n::UnicodeType::END_PUNCTUATION;
            break;
    }

    return r;
}

sal_uInt8
unicode::getUnicodeDirection( const sal_Unicode ch ) {
    static sal_Unicode c = 0x00;
    static sal_uInt8 r = 0x00;

    if (ch == c) return r;
    else c = ch;

    sal_Int16 address = UnicodeDirectionIndex[ch >> 8];
    r = (address < UnicodeDirectionNumberBlock)
            ? UnicodeDirectionBlockValue[address]
            : UnicodeDirectionValue[((address - UnicodeDirectionNumberBlock) << 8) + (ch & 0xff)];
    return r;
}

sal_uInt32 unicode::GetMirroredChar(sal_uInt32 nChar) {
    nChar = u_charMirror(nChar);
    return nChar;
}

#define bit(name)   (1U << name)

#define UPPERMASK   bit(UnicodeType::UPPERCASE_LETTER)

#define LOWERMASK   bit(UnicodeType::LOWERCASE_LETTER)

#define TITLEMASK   bit(UnicodeType::TITLECASE_LETTER)

#define ALPHAMASK   UPPERMASK|LOWERMASK|TITLEMASK|\
            bit(UnicodeType::MODIFIER_LETTER)|\
            bit(UnicodeType::OTHER_LETTER)

#define SPACEMASK   bit(UnicodeType::SPACE_SEPARATOR)|\
            bit(UnicodeType::LINE_SEPARATOR)|\
            bit(UnicodeType::PARAGRAPH_SEPARATOR)

#define CONTROLMASK bit(UnicodeType::CONTROL)|\
            bit(UnicodeType::FORMAT)|\
            bit(UnicodeType::LINE_SEPARATOR)|\
            bit(UnicodeType::PARAGRAPH_SEPARATOR)

#define IsType(func, mask)  \
bool func( const sal_uInt32 ch) {\
    return (bit(getUnicodeType(ch)) & (mask)) != 0;\
}

IsType(unicode::isControl, CONTROLMASK)
IsType(unicode::isAlpha, ALPHAMASK)
IsType(unicode::isSpace, SPACEMASK)

#define CONTROLSPACE    bit(0x09)|bit(0x0a)|bit(0x0b)|bit(0x0c)|bit(0x0d)|\
            bit(0x1c)|bit(0x1d)|bit(0x1e)|bit(0x1f)

bool unicode::isWhiteSpace(const sal_uInt32 ch)
{
    return (ch != 0xa0 && isSpace(ch)) || (ch <= 0x1F && (bit(ch) & (CONTROLSPACE)));
}

sal_Int16 unicode::getScriptClassFromUScriptCode(UScriptCode eScript)
{
    //See unicode/uscript.h
    sal_Int16 nRet;
    switch (eScript)
    {
        case USCRIPT_INVALID_CODE:
        case USCRIPT_COMMON:
        case USCRIPT_INHERITED:
        case USCRIPT_UNWRITTEN_LANGUAGES:
        case USCRIPT_UNKNOWN:
        case USCRIPT_MATHEMATICAL_NOTATION:
        case USCRIPT_SYMBOLS:
        case USCRIPT_CODE_LIMIT:
            nRet = ScriptType::WEAK;
            break;
        case USCRIPT_ARMENIAN:
        case USCRIPT_CHEROKEE:
        case USCRIPT_COPTIC:
        case USCRIPT_CYRILLIC:
        case USCRIPT_GEORGIAN:
        case USCRIPT_GOTHIC:
        case USCRIPT_GREEK:
        case USCRIPT_LATIN:
        case USCRIPT_OGHAM:
        case USCRIPT_OLD_ITALIC:
        case USCRIPT_RUNIC:
        case USCRIPT_CANADIAN_ABORIGINAL:
        case USCRIPT_BRAILLE:
        case USCRIPT_CYPRIOT:
        case USCRIPT_OSMANYA:
        case USCRIPT_SHAVIAN:
        case USCRIPT_KATAKANA_OR_HIRAGANA:
        case USCRIPT_GLAGOLITIC:
        case USCRIPT_CIRTH:
        case USCRIPT_OLD_CHURCH_SLAVONIC_CYRILLIC:
        case USCRIPT_OLD_HUNGARIAN:
        case USCRIPT_LATIN_FRAKTUR:
        case USCRIPT_LATIN_GAELIC:
            nRet = ScriptType::LATIN;
            break;
        case USCRIPT_BOPOMOFO:
        case USCRIPT_HAN:
        case USCRIPT_HANGUL:
        case USCRIPT_HIRAGANA:
        case USCRIPT_KATAKANA:
        case USCRIPT_YI:
        case USCRIPT_SIMPLIFIED_HAN:
        case USCRIPT_TRADITIONAL_HAN:
        case USCRIPT_JAPANESE:
        case USCRIPT_KOREAN:
        case USCRIPT_TANGUT:
        case USCRIPT_KHITAN_SMALL_SCRIPT:
            nRet = ScriptType::ASIAN;
            break;
        case USCRIPT_ARABIC:
        case USCRIPT_BENGALI:
        case USCRIPT_DESERET:
        case USCRIPT_DEVANAGARI:
        case USCRIPT_ETHIOPIC:
        case USCRIPT_GUJARATI:
        case USCRIPT_GURMUKHI:
        case USCRIPT_HEBREW:
        case USCRIPT_KANNADA:
        case USCRIPT_KHMER:
        case USCRIPT_LAO:
        case USCRIPT_MALAYALAM:
        case USCRIPT_MONGOLIAN:
        case USCRIPT_MYANMAR:
        case USCRIPT_ORIYA:
        case USCRIPT_SINHALA:
        case USCRIPT_SYRIAC:
        case USCRIPT_TAMIL:
        case USCRIPT_TELUGU:
        case USCRIPT_THAANA:
        case USCRIPT_THAI:
        case USCRIPT_TIBETAN:
        case USCRIPT_TAGALOG:
        case USCRIPT_HANUNOO:
        case USCRIPT_BUHID:
        case USCRIPT_TAGBANWA:
        case USCRIPT_LIMBU:
        case USCRIPT_LINEAR_B:
        case USCRIPT_TAI_LE:
        case USCRIPT_UGARITIC:
        case USCRIPT_BUGINESE:
        case USCRIPT_KHAROSHTHI:
        case USCRIPT_SYLOTI_NAGRI:
        case USCRIPT_NEW_TAI_LUE:
        case USCRIPT_TIFINAGH:
        case USCRIPT_OLD_PERSIAN:
        case USCRIPT_BALINESE:
        case USCRIPT_BATAK:
        case USCRIPT_BLISSYMBOLS:
        case USCRIPT_BRAHMI:
        case USCRIPT_CHAM:
        case USCRIPT_DEMOTIC_EGYPTIAN:
        case USCRIPT_HIERATIC_EGYPTIAN:
        case USCRIPT_EGYPTIAN_HIEROGLYPHS:
        case USCRIPT_KHUTSURI:
        case USCRIPT_PAHAWH_HMONG:
        case USCRIPT_HARAPPAN_INDUS:
        case USCRIPT_JAVANESE:
        case USCRIPT_KAYAH_LI:
        case USCRIPT_LEPCHA:
        case USCRIPT_LINEAR_A:
        case USCRIPT_MANDAEAN:
        case USCRIPT_MAYAN_HIEROGLYPHS:
        case USCRIPT_MEROITIC:
        case USCRIPT_NKO:
        case USCRIPT_ORKHON:
        case USCRIPT_OLD_PERMIC:
        case USCRIPT_PHAGS_PA:
        case USCRIPT_PHOENICIAN:
        case USCRIPT_PHONETIC_POLLARD:
        case USCRIPT_RONGORONGO:
        case USCRIPT_SARATI:
        case USCRIPT_ESTRANGELO_SYRIAC:
        case USCRIPT_WESTERN_SYRIAC:
        case USCRIPT_EASTERN_SYRIAC:
        case USCRIPT_TENGWAR:
        case USCRIPT_VAI:
        case USCRIPT_VISIBLE_SPEECH:
        case USCRIPT_CUNEIFORM:
        case USCRIPT_CARIAN:
        case USCRIPT_LANNA:
        case USCRIPT_LYCIAN:
        case USCRIPT_LYDIAN:
        case USCRIPT_OL_CHIKI:
        case USCRIPT_REJANG:
        case USCRIPT_SAURASHTRA:
        case USCRIPT_SIGN_WRITING:
        case USCRIPT_SUNDANESE:
        case USCRIPT_MOON:
        case USCRIPT_MEITEI_MAYEK:
        case USCRIPT_IMPERIAL_ARAMAIC:
        case USCRIPT_AVESTAN:
        case USCRIPT_CHAKMA:
        case USCRIPT_KAITHI:
        case USCRIPT_MANICHAEAN:
        case USCRIPT_INSCRIPTIONAL_PAHLAVI:
        case USCRIPT_PSALTER_PAHLAVI:
        case USCRIPT_BOOK_PAHLAVI:
        case USCRIPT_INSCRIPTIONAL_PARTHIAN:
        case USCRIPT_SAMARITAN:
        case USCRIPT_TAI_VIET:
        case USCRIPT_BAMUM:
        case USCRIPT_LISU:
        case USCRIPT_NAKHI_GEBA:
        case USCRIPT_OLD_SOUTH_ARABIAN:
        case USCRIPT_BASSA_VAH:
        case USCRIPT_DUPLOYAN_SHORTAND:
        case USCRIPT_ELBASAN:
        case USCRIPT_GRANTHA:
        case USCRIPT_KPELLE:
        case USCRIPT_LOMA:
        case USCRIPT_MENDE:
        case USCRIPT_MEROITIC_CURSIVE:
        case USCRIPT_OLD_NORTH_ARABIAN:
        case USCRIPT_NABATAEAN:
        case USCRIPT_PALMYRENE:
        case USCRIPT_SINDHI:
        case USCRIPT_WARANG_CITI:
        default:         // anything new is going to be pretty wild
            nRet = ScriptType::COMPLEX;
            break;
    }
    return nRet;
}

sal_Int16 unicode::getScriptClassFromLanguageTag( const LanguageTag& rLanguageTag )
{
    constexpr int32_t nBuf = 42;
    UScriptCode aBuf[nBuf];
    if (rLanguageTag.hasScript())
    {
        aBuf[0] = static_cast<UScriptCode>(u_getPropertyValueEnum( UCHAR_SCRIPT,
                OUStringToOString( rLanguageTag.getScript(), RTL_TEXTENCODING_ASCII_US).getStr()));
    }
    else
    {
        OUString aName;
        if (rLanguageTag.getCountry().isEmpty())
            aName = rLanguageTag.getLanguage();
        else
            aName = rLanguageTag.getLanguage() + "-" + rLanguageTag.getCountry();
        UErrorCode status = U_ZERO_ERROR;
        const int32_t nScripts = uscript_getCode(
                OUStringToOString( aName, RTL_TEXTENCODING_ASCII_US).getStr(),
                aBuf, nBuf, &status);
        // U_BUFFER_OVERFLOW_ERROR would be set with too many scripts for buffer
        // and required capacity returned, but really..
        if (nScripts == 0 || !U_SUCCESS(status))
            return css::i18n::ScriptType::LATIN;
    }
    return getScriptClassFromUScriptCode( aBuf[0]);
}

OString unicode::getExemplarLanguageForUScriptCode(UScriptCode eScript)
{
    OString sRet;
    switch (eScript)
    {
        case USCRIPT_CODE_LIMIT:
        case USCRIPT_INVALID_CODE:
            sRet = "zxx"_ostr;
            break;
        case USCRIPT_COMMON:
        case USCRIPT_INHERITED:
            sRet = "und"_ostr;
            break;
        case USCRIPT_MATHEMATICAL_NOTATION:
        case USCRIPT_SYMBOLS:
            sRet = "zxx"_ostr;
            break;
        case USCRIPT_UNWRITTEN_LANGUAGES:
        case USCRIPT_UNKNOWN:
            sRet = "und"_ostr;
            break;
        case USCRIPT_ARABIC:
            sRet = "ar"_ostr;
            break;
        case USCRIPT_ARMENIAN:
            sRet = "hy"_ostr;
            break;
        case USCRIPT_BENGALI:
            sRet = "bn"_ostr;
            break;
        case USCRIPT_BOPOMOFO:
            sRet = "zh"_ostr;
            break;
        case USCRIPT_CHEROKEE:
            sRet = "chr"_ostr;
            break;
        case USCRIPT_COPTIC:
            sRet = "cop"_ostr;
            break;
        case USCRIPT_CYRILLIC:
            sRet = "ru"_ostr;
            break;
        case USCRIPT_DESERET:
            sRet = "en"_ostr;
            break;
        case USCRIPT_DEVANAGARI:
            sRet = "hi"_ostr;
            break;
        case USCRIPT_ETHIOPIC:
            sRet = "am"_ostr;
            break;
        case USCRIPT_GEORGIAN:
            sRet = "ka"_ostr;
            break;
        case USCRIPT_GOTHIC:
            sRet = "got"_ostr;
            break;
        case USCRIPT_GREEK:
            sRet = "el"_ostr;
            break;
        case USCRIPT_GUJARATI:
            sRet = "gu"_ostr;
            break;
        case USCRIPT_GURMUKHI:
            sRet = "pa"_ostr;
            break;
        case USCRIPT_HAN:
            sRet = "zh"_ostr;
            break;
        case USCRIPT_HANGUL:
            sRet = "ko"_ostr;
            break;
        case USCRIPT_HEBREW:
            sRet = "hr"_ostr;
            break;
        case USCRIPT_HIRAGANA:
            sRet = "ja"_ostr;
            break;
        case USCRIPT_KANNADA:
            sRet = "kn"_ostr;
            break;
        case USCRIPT_KATAKANA:
            sRet = "ja"_ostr;
            break;
        case USCRIPT_KHMER:
            sRet = "km"_ostr;
            break;
        case USCRIPT_LAO:
            sRet = "lo"_ostr;
            break;
        case USCRIPT_LATIN:
            sRet = "en"_ostr;
            break;
        case USCRIPT_MALAYALAM:
            sRet = "ml"_ostr;
            break;
        case USCRIPT_MONGOLIAN:
            sRet = "mn"_ostr;
            break;
        case USCRIPT_MYANMAR:
            sRet = "my"_ostr;
            break;
        case USCRIPT_OGHAM:
            sRet = "pgl"_ostr;
            break;
        case USCRIPT_OLD_ITALIC:
            sRet = "osc"_ostr;
            break;
        case USCRIPT_ORIYA:
            sRet = "or"_ostr;
            break;
        case USCRIPT_RUNIC:
            sRet = "ang"_ostr;
            break;
        case USCRIPT_SINHALA:
            sRet = "si"_ostr;
            break;
        case USCRIPT_SYRIAC:
            sRet = "syr"_ostr;
            break;
        case USCRIPT_TAMIL:
            sRet = "ta"_ostr;
            break;
        case USCRIPT_TELUGU:
            sRet = "te"_ostr;
            break;
        case USCRIPT_THAANA:
            sRet = "dv"_ostr;
            break;
        case USCRIPT_THAI:
            sRet = "th"_ostr;
            break;
        case USCRIPT_TIBETAN:
            sRet = "bo"_ostr;
            break;
        case USCRIPT_CANADIAN_ABORIGINAL:
            sRet = "iu"_ostr;
            break;
        case USCRIPT_YI:
            sRet = "ii"_ostr;
            break;
        case USCRIPT_TAGALOG:
            sRet = "tl"_ostr;
            break;
        case USCRIPT_HANUNOO:
            sRet = "hnn"_ostr;
            break;
        case USCRIPT_BUHID:
            sRet = "bku"_ostr;
            break;
        case USCRIPT_TAGBANWA:
            sRet = "tbw"_ostr;
            break;
        case USCRIPT_BRAILLE:
            sRet = "en"_ostr;
            break;
        case USCRIPT_CYPRIOT:
            sRet = "ecy"_ostr;
            break;
        case USCRIPT_LIMBU:
            sRet = "lif"_ostr;
            break;
        case USCRIPT_LINEAR_B:
            sRet = "gmy"_ostr;
            break;
        case USCRIPT_OSMANYA:
            sRet = "so"_ostr;
            break;
        case USCRIPT_SHAVIAN:
            sRet = "en"_ostr;
            break;
        case USCRIPT_TAI_LE:
            sRet = "tdd"_ostr;
            break;
        case USCRIPT_UGARITIC:
            sRet = "uga"_ostr;
            break;
        case USCRIPT_KATAKANA_OR_HIRAGANA:
            sRet = "ja"_ostr;
            break;
        case USCRIPT_BUGINESE:
            sRet = "bug"_ostr;
            break;
        case USCRIPT_GLAGOLITIC:
            sRet = "ch"_ostr;
            break;
        case USCRIPT_KHAROSHTHI:
            sRet = "pra"_ostr;
            break;
        case USCRIPT_SYLOTI_NAGRI:
            sRet = "syl"_ostr;
            break;
        case USCRIPT_NEW_TAI_LUE:
            sRet = "khb"_ostr;
            break;
        case USCRIPT_TIFINAGH:
            sRet = "tmh"_ostr;
            break;
        case USCRIPT_OLD_PERSIAN:
            sRet = "peo"_ostr;
            break;
        case USCRIPT_BALINESE:
            sRet = "ban"_ostr;
            break;
        case USCRIPT_BATAK:
            sRet = "btk"_ostr;
            break;
        case USCRIPT_BLISSYMBOLS:
            sRet = "en"_ostr;
            break;
        case USCRIPT_BRAHMI:
            sRet = "pra"_ostr;
            break;
        case USCRIPT_CHAM:
            sRet = "cja"_ostr;
            break;
        case USCRIPT_CIRTH:
            sRet = "sjn"_ostr;
            break;
        case USCRIPT_OLD_CHURCH_SLAVONIC_CYRILLIC:
            sRet = "cu"_ostr;
            break;
        case USCRIPT_DEMOTIC_EGYPTIAN:
        case USCRIPT_HIERATIC_EGYPTIAN:
        case USCRIPT_EGYPTIAN_HIEROGLYPHS:
            sRet = "egy"_ostr;
            break;
        case USCRIPT_KHUTSURI:
            sRet = "ka"_ostr;
            break;
        case USCRIPT_SIMPLIFIED_HAN:
            sRet = "zh"_ostr;
            break;
        case USCRIPT_TRADITIONAL_HAN:
            sRet = "zh"_ostr;
            break;
        case USCRIPT_PAHAWH_HMONG:
            sRet = "blu"_ostr;
            break;
        case USCRIPT_OLD_HUNGARIAN:
            sRet = "ohu"_ostr;
            break;
        case USCRIPT_HARAPPAN_INDUS:
            sRet = "xiv"_ostr;
            break;
        case USCRIPT_JAVANESE:
            sRet = "kaw"_ostr;
            break;
        case USCRIPT_KAYAH_LI:
            sRet = "eky"_ostr;
            break;
        case USCRIPT_LATIN_FRAKTUR:
            sRet = "de"_ostr;
            break;
        case USCRIPT_LATIN_GAELIC:
            sRet = "ga"_ostr;
            break;
        case USCRIPT_LEPCHA:
            sRet = "lep"_ostr;
            break;
        case USCRIPT_LINEAR_A:
            sRet = "ecr"_ostr;
            break;
        case USCRIPT_MAYAN_HIEROGLYPHS:
            sRet = "myn"_ostr;
            break;
        case USCRIPT_MEROITIC:
            sRet = "xmr"_ostr;
            break;
        case USCRIPT_NKO:
            sRet = "nqo"_ostr;
            break;
        case USCRIPT_ORKHON:
            sRet = "otk"_ostr;
            break;
        case USCRIPT_OLD_PERMIC:
            sRet = "kv"_ostr;
            break;
        case USCRIPT_PHAGS_PA:
            sRet = "xng"_ostr;
            break;
        case USCRIPT_PHOENICIAN:
            sRet = "phn"_ostr;
            break;
        case USCRIPT_PHONETIC_POLLARD:
            sRet = "hmd"_ostr;
            break;
        case USCRIPT_RONGORONGO:
            sRet = "rap"_ostr;
            break;
        case USCRIPT_SARATI:
            sRet = "qya"_ostr;
            break;
        case USCRIPT_ESTRANGELO_SYRIAC:
            sRet = "syr"_ostr;
            break;
        case USCRIPT_WESTERN_SYRIAC:
            sRet = "tru"_ostr;
            break;
        case USCRIPT_EASTERN_SYRIAC:
            sRet = "aii"_ostr;
            break;
        case USCRIPT_TENGWAR:
            sRet = "sjn"_ostr;
            break;
        case USCRIPT_VAI:
            sRet = "vai"_ostr;
            break;
        case USCRIPT_VISIBLE_SPEECH:
            sRet = "en"_ostr;
            break;
        case USCRIPT_CUNEIFORM:
            sRet = "akk"_ostr;
            break;
        case USCRIPT_CARIAN:
            sRet = "xcr"_ostr;
            break;
        case USCRIPT_JAPANESE:
            sRet = "ja"_ostr;
            break;
        case USCRIPT_LANNA:
            sRet = "nod"_ostr;
            break;
        case USCRIPT_LYCIAN:
            sRet = "xlc"_ostr;
            break;
        case USCRIPT_LYDIAN:
            sRet = "xld"_ostr;
            break;
        case USCRIPT_OL_CHIKI:
            sRet = "sat"_ostr;
            break;
        case USCRIPT_REJANG:
            sRet = "rej"_ostr;
            break;
        case USCRIPT_SAURASHTRA:
            sRet = "saz"_ostr;
            break;
        case USCRIPT_SIGN_WRITING:
            sRet = "en"_ostr;
            break;
        case USCRIPT_SUNDANESE:
            sRet = "su"_ostr;
            break;
        case USCRIPT_MOON:
            sRet = "en"_ostr;
            break;
        case USCRIPT_MEITEI_MAYEK:
            sRet = "mni"_ostr;
            break;
        case USCRIPT_IMPERIAL_ARAMAIC:
            sRet = "arc"_ostr;
            break;
        case USCRIPT_AVESTAN:
            sRet = "ae"_ostr;
            break;
        case USCRIPT_CHAKMA:
            sRet = "ccp"_ostr;
            break;
        case USCRIPT_KOREAN:
            sRet = "ko"_ostr;
            break;
        case USCRIPT_KAITHI:
            sRet = "awa"_ostr;
            break;
        case USCRIPT_MANICHAEAN:
            sRet = "xmn"_ostr;
            break;
        case USCRIPT_INSCRIPTIONAL_PAHLAVI:
        case USCRIPT_PSALTER_PAHLAVI:
        case USCRIPT_BOOK_PAHLAVI:
        case USCRIPT_INSCRIPTIONAL_PARTHIAN:
            sRet = "xpr"_ostr;
            break;
        case USCRIPT_SAMARITAN:
            sRet = "heb"_ostr;
            break;
        case USCRIPT_TAI_VIET:
            sRet = "blt"_ostr;
            break;
        case USCRIPT_MANDAEAN: /* Aliased to USCRIPT_MANDAIC in icu 4.6. */
            sRet = "mic"_ostr;
            break;
        case USCRIPT_NABATAEAN:
            sRet = "mis-Nbat"_ostr;  // Uncoded with script
            break;
        case USCRIPT_PALMYRENE:
            sRet = "mis-Palm"_ostr;  // Uncoded with script
            break;
        case USCRIPT_BAMUM:
            sRet = "bax"_ostr;
            break;
        case USCRIPT_LISU:
            sRet = "lis"_ostr;
            break;
        case USCRIPT_NAKHI_GEBA:
            sRet = "nxq"_ostr;
            break;
        case USCRIPT_OLD_SOUTH_ARABIAN:
            sRet = "xsa"_ostr;
            break;
        case USCRIPT_BASSA_VAH:
            sRet = "bsq"_ostr;
            break;
        case USCRIPT_DUPLOYAN_SHORTAND:
            sRet = "fr"_ostr;
            break;
        case USCRIPT_ELBASAN:
            sRet = "sq"_ostr;
            break;
        case USCRIPT_GRANTHA:
            sRet = "ta"_ostr;
            break;
        case USCRIPT_KPELLE:
            sRet = "kpe"_ostr;
            break;
        case USCRIPT_LOMA:
            sRet = "lom"_ostr;
            break;
        case USCRIPT_MENDE:
            sRet = "men"_ostr;
            break;
        case USCRIPT_MEROITIC_CURSIVE:
            sRet = "xmr"_ostr;
            break;
        case USCRIPT_OLD_NORTH_ARABIAN:
            sRet = "xna"_ostr;
            break;
        case USCRIPT_SINDHI:
            sRet = "sd"_ostr;
            break;
        case USCRIPT_WARANG_CITI:
            sRet = "hoc"_ostr;
            break;
        case USCRIPT_AFAKA:
            sRet = "djk"_ostr;
            break;
        case USCRIPT_JURCHEN:
            sRet = "juc"_ostr;
            break;
        case USCRIPT_MRO:
            sRet = "cmr"_ostr;
            break;
        case USCRIPT_NUSHU:
            sRet = "mis-Nshu"_ostr;  // Uncoded with script
            break;
        case USCRIPT_SHARADA:
            sRet = "sa"_ostr;
            break;
        case USCRIPT_SORA_SOMPENG:
            sRet = "srb"_ostr;
            break;
        case USCRIPT_TAKRI:
            sRet = "doi"_ostr;
            break;
        case USCRIPT_TANGUT:
            sRet = "txg"_ostr;
            break;
        case USCRIPT_WOLEAI:
            sRet = "woe"_ostr;
            break;
        case USCRIPT_ANATOLIAN_HIEROGLYPHS:
            sRet = "hlu"_ostr;
            break;
        case USCRIPT_KHOJKI:
            sRet = "gu"_ostr;
            break;
        case USCRIPT_TIRHUTA:
            sRet = "mai"_ostr;
            break;
        case USCRIPT_CAUCASIAN_ALBANIAN:
            sRet = "xag"_ostr;
            break;
        case USCRIPT_MAHAJANI:
            sRet = "mwr"_ostr;
            break;
        case USCRIPT_AHOM:
            sRet = "aho"_ostr;
            break;
        case USCRIPT_HATRAN:
            sRet = "qly-Hatr"_ostr;
            break;
        case USCRIPT_MODI:
            sRet = "mr-Modi"_ostr;
            break;
        case USCRIPT_MULTANI:
            sRet = "skr-Mutl"_ostr;
            break;
        case USCRIPT_PAU_CIN_HAU:
            sRet = "ctd-Pauc"_ostr;
            break;
        case USCRIPT_SIDDHAM:
            sRet = "sa-Sidd"_ostr;
            break;
        case USCRIPT_ADLAM:
            sRet = "mis-Adlm"_ostr;   // Adlam for Fulani, no language code
            break;
        case USCRIPT_BHAIKSUKI:
            sRet = "mis-Bhks"_ostr;   // Bhaiksuki for some Buddhist texts, no language code
            break;
        case USCRIPT_MARCHEN:
            sRet = "bo-Marc"_ostr;
            break;
        case USCRIPT_NEWA:
            sRet = "new-Newa"_ostr;
            break;
        case USCRIPT_OSAGE:
            sRet = "osa-Osge"_ostr;
            break;
        case USCRIPT_HAN_WITH_BOPOMOFO:
            sRet = "mis-Hanb"_ostr;   // Han with Bopomofo, zh-Hanb ?
            break;
        case USCRIPT_JAMO:
            sRet = "ko"_ostr;   // Jamo - elements of Hangul Syllables
            break;
        case USCRIPT_SYMBOLS_EMOJI:
            sRet = "mis-Zsye"_ostr;   // Emoji variant
            break;
        case USCRIPT_MASARAM_GONDI:
            sRet = "gon-Gonm"_ostr;  // macro language code, could be wsg,esg,gno
            break;
        case USCRIPT_SOYOMBO:
            sRet = "mn-Soyo"_ostr;   // abugida to write Mongolian, also Tibetan and Sanskrit
            break;
        case USCRIPT_ZANABAZAR_SQUARE:
            sRet = "mn-Zanb"_ostr;   // abugida to write Mongolian
            break;
        case USCRIPT_DOGRA:
            sRet = "dgo"_ostr;       // Dogri proper
            break;
        case USCRIPT_GUNJALA_GONDI:
            sRet = "wsg"_ostr;       // Adilabad Gondi
            break;
        case USCRIPT_MAKASAR:
            sRet = "mak"_ostr;
            break;
        case USCRIPT_MEDEFAIDRIN:
            sRet = "dmf-Medf"_ostr;
            break;
        case USCRIPT_HANIFI_ROHINGYA:
            sRet = "rhg"_ostr;
            break;
        case USCRIPT_SOGDIAN:
            sRet = "sog"_ostr;
            break;
        case USCRIPT_OLD_SOGDIAN:
            sRet = "sog"_ostr;
            break;
        case USCRIPT_ELYMAIC:
            sRet = "arc-Elym"_ostr;
            break;
        case USCRIPT_NYIAKENG_PUACHUE_HMONG:
            sRet = "hmn-Hmnp"_ostr;  // macrolanguage code
            break;
        case USCRIPT_NANDINAGARI:
            sRet = "sa-Nand"_ostr;
            break;
        case USCRIPT_WANCHO:
            sRet = "nnp-Wcho"_ostr;
            break;
        case USCRIPT_CHORASMIAN:
            sRet = "xco-Chrs"_ostr;
            break;
        case USCRIPT_DIVES_AKURU:
            sRet = "dv-Diak"_ostr;
            break;
        case USCRIPT_KHITAN_SMALL_SCRIPT:
            sRet = "zkt-Kits"_ostr;
            break;
        case USCRIPT_YEZIDI:
            sRet = "kmr-Yezi"_ostr;
            break;
#if (U_ICU_VERSION_MAJOR_NUM >= 70)
        case USCRIPT_CYPRO_MINOAN:
            sRet = "mis-Cpmn"_ostr;  // Uncoded with script
            break;
        case USCRIPT_OLD_UYGHUR:
            sRet = "oui-Ougr"_ostr;
            break;
        case USCRIPT_TANGSA:
            sRet = "nst-Tnsa"_ostr;
            break;
        case USCRIPT_TOTO:
            sRet = "txo-Toto"_ostr;
            break;
        case USCRIPT_VITHKUQI:
            sRet = "sq-Vith"_ostr;   // macrolanguage code
            break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 72)
        case USCRIPT_KAWI:
            sRet = "mis-Kawi"_ostr;  // Uncoded with script
            break;
        case USCRIPT_NAG_MUNDARI:
            sRet = "unr-Nagm"_ostr;
            break;
#endif
    }
    return sRet;
}

//Format a number as a percentage according to the rules of the given
//language, e.g. 100 -> "100%" for en-US vs "100 %" for de-DE
OUString unicode::formatPercent(double dNumber,
    const LanguageTag &rLangTag)
{
    // get a currency formatter for this locale ID
    UErrorCode errorCode=U_ZERO_ERROR;

    LanguageTag aLangTag(rLangTag);

    // As of CLDR Version 24 these languages were not listed as using spacing
    // between number and % but are reported as such by our l10n groups
    // http://www.unicode.org/cldr/charts/24/by_type/numbers.number_formatting_patterns.html
    // so format using French which has the desired rules
    if (aLangTag.getLanguage() == "es" || aLangTag.getLanguage() == "sl")
        aLangTag.reset(u"fr-FR"_ustr);

    icu::Locale aLocale = LanguageTagIcu::getIcuLocale(aLangTag);

    std::unique_ptr<icu::NumberFormat> xF(
        icu::NumberFormat::createPercentInstance(aLocale, errorCode));
    if(U_FAILURE(errorCode))
    {
        SAL_WARN("i18n", "icu::NumberFormat::createPercentInstance failed");
        return OUString::number(dNumber) + "%";
    }

    icu::UnicodeString output;
    xF->format(dNumber/100, output);
    OUString aRet(reinterpret_cast<const sal_Unicode *>(output.getBuffer()),
        output.length());
    if (rLangTag.getLanguage() == "de")
    {
        //narrow no-break space instead of (normal) no-break space
        return aRet.replace(0x00A0, 0x202F);
    }
    return aRet;
}

bool ToggleUnicodeCodepoint::AllowMoreInput(sal_Unicode uChar)
{
    //arbitrarily chosen maximum length allowed - normal max usage would be around 30.
    if( maInput.getLength() > 255 )
        mbAllowMoreChars = false;

    if( !mbAllowMoreChars )
        return false;

    bool bPreventNonHex = false;
    if( maInput.indexOf("U+") != -1 )
        bPreventNonHex = true;

    switch ( unicode::getUnicodeType(uChar) )
    {
        case css::i18n::UnicodeType::SURROGATE:
            if( bPreventNonHex )
            {
                mbAllowMoreChars = false;
                return false;
            }

            if( rtl::isLowSurrogate(uChar) && maUtf16.isEmpty() && maInput.isEmpty()  )
            {
                maUtf16.append(uChar);
                return true;
            }
            if( rtl::isHighSurrogate(uChar) && maInput.isEmpty() )
                maUtf16.insert(0, uChar );
            //end of hex strings, or unexpected order of high/low, so don't accept more
            if( !maUtf16.isEmpty() )
                maInput.append(maUtf16);
            if( !maCombining.isEmpty() )
                maInput.append(maCombining);
            mbAllowMoreChars = false;
            break;

        case css::i18n::UnicodeType::NON_SPACING_MARK:
        case css::i18n::UnicodeType::COMBINING_SPACING_MARK:
            if( bPreventNonHex )
            {
                mbAllowMoreChars = false;
                return false;
            }

            //extreme edge case: already invalid high/low surrogates with preceding combining chars, and now an extra combining mark.
            if( !maUtf16.isEmpty() )
            {
                maInput = maUtf16;
                if( !maCombining.isEmpty() )
                    maInput.append(maCombining);
                mbAllowMoreChars = false;
                return false;
            }
            maCombining.insert(0, uChar);
            break;

        default:
            //extreme edge case: already invalid high/low surrogates with preceding combining chars, and now an extra character.
            if( !maUtf16.isEmpty() )
            {
                maInput = maUtf16;
                if( !maCombining.isEmpty() )
                    maInput.append(maCombining);
                mbAllowMoreChars = false;
                return false;
            }

            if( !maCombining.isEmpty() )
            {
                maCombining.insert(0, uChar);
                maInput = maCombining;
                mbAllowMoreChars = false;
                return false;
            }

            // 0 - 1f are control characters.  Do not process those.
            if( uChar < 0x20 )
            {
                mbAllowMoreChars = false;
                return false;
            }

            switch( uChar )
            {
                case 'u':
                case 'U':
                    // U+ notation found.  Continue looking for another one.
                    if( mbRequiresU )
                    {
                        mbRequiresU = false;
                        maInput.insert(0,"U+");
                    }
                    // treat as a normal character
                    else
                    {
                        mbAllowMoreChars = false;
                        if( !bPreventNonHex )
                            maInput.insertUtf32(0, uChar);
                    }
                    break;
                case '+':
                    // + already found: skip when not U, or edge case of +U+xxxx
                    if( mbRequiresU || (maInput.indexOf("U+") == 0) )
                        mbAllowMoreChars = false;
                    // hex chars followed by '+' - now require a 'U'
                    else if ( !maInput.isEmpty() )
                        mbRequiresU = true;
                    // treat as a normal character
                    else
                    {
                        mbAllowMoreChars = false;
                        if( !bPreventNonHex )
                            maInput.insertUtf32(0, uChar);
                    }
                    break;
                default:
                    // + already found. Since not U, cancel further input
                    if( mbRequiresU )
                        mbAllowMoreChars = false;
                    // maximum digits per notation is 8: only one notation
                    else if( maInput.indexOf("U+") == -1 && maInput.getLength() == 8 )
                        mbAllowMoreChars = false;
                    // maximum digits per notation is 8: previous notation found
                    else if( maInput.indexOf("U+") == 8 )
                        mbAllowMoreChars = false;
                    // a hex character. Add to string.
                    else if( rtl::isAsciiHexDigit(uChar) )
                    {
                        mbIsHexString = true;
                        maInput.insertUtf32(0, uChar);
                    }
                    // not a hex character: stop input. keep if it is the first input provided
                    else
                    {
                        mbAllowMoreChars = false;
                        if( maInput.isEmpty() )
                            maInput.insertUtf32(0, uChar);
                    }
            }
    }
    return mbAllowMoreChars;
}

OUString ToggleUnicodeCodepoint::StringToReplace()
{
    if( maInput.isEmpty() )
    {
        //edge case - input finished with incomplete low surrogate or combining characters without a base
        if( mbAllowMoreChars )
        {
            if( !maUtf16.isEmpty() )
                maInput = maUtf16;
            if( !maCombining.isEmpty() )
                maInput.append(maCombining);
        }
        return maInput.toString();
    }

    if( !mbIsHexString )
        return maInput.toString();

    //this function potentially modifies the input string.  Prevent addition of further characters
    mbAllowMoreChars = false;

    //validate unicode notation.
    OUString sIn;
    sal_uInt32 nUnicode = 0;
    sal_Int32 nUPlus = maInput.indexOf("U+");
    //if U+ notation used, strip off all extra chars added not in U+ notation
    if( nUPlus != -1 )
    {
        maInput.remove(0, nUPlus);
        sIn = maInput.copy(2).makeStringAndClear();
        nUPlus = sIn.indexOf("U+");
    }
    else
        sIn = maInput.toString();
    while( nUPlus != -1 )
    {
        nUnicode = o3tl::toUInt32(sIn.subView(0, nUPlus), 16);
        //prevent creating control characters or invalid Unicode values
        if( !rtl::isUnicodeCodePoint(nUnicode) || nUnicode < 0x20  )
            maInput = sIn.subView(nUPlus);
        sIn = sIn.copy(nUPlus+2);
        nUPlus =  sIn.indexOf("U+");
    }

    nUnicode = sIn.toUInt32(16);
    if( !rtl::isUnicodeCodePoint(nUnicode) || nUnicode < 0x20 )
       maInput.truncate().append( sIn[sIn.getLength()-1] );
    return maInput.toString();
}

sal_uInt32 ToggleUnicodeCodepoint::CharsToDelete()
{
    OUString sIn = StringToReplace();
    sal_Int32 nPos = 0;
    sal_uInt32 counter = 0;
    while( nPos < sIn.getLength() )
    {
        sIn.iterateCodePoints(&nPos);
        ++counter;
    }
    return counter;
}

OUString ToggleUnicodeCodepoint::ReplacementString()
{
    OUString sIn = StringToReplace();
    OUStringBuffer output = "";
    sal_Int32 nUPlus = sIn.indexOf("U+");
    // convert from hex notation to glyph
    if( nUPlus != -1 || (sIn.getLength() > 1 && mbIsHexString) )
    {
        sal_uInt32 nUnicode = 0;
        if( nUPlus == 0)
        {
            sIn = sIn.copy(2);
            nUPlus = sIn.indexOf("U+");
        }
        while( nUPlus > 0 )
        {
            nUnicode = o3tl::toUInt32(sIn.subView(0, nUPlus), 16);
            output.appendUtf32( nUnicode );

            sIn = sIn.copy(nUPlus+2);
            nUPlus = sIn.indexOf("U+");
        }
        nUnicode = sIn.toUInt32(16);
        output.appendUtf32( nUnicode );
    }
    // convert from glyph to hex notation
    else
    {
        sal_Int32 nPos = 0;
        while( nPos < sIn.getLength() )
        {
            OUStringBuffer aTmp = OUString::number(sIn.iterateCodePoints(&nPos),16);
            //pad with zeros - minimum length of 4.
            for( sal_Int32 i = 4 - aTmp.getLength(); i > 0; --i )
                aTmp.insert( 0,"0" );
            output.append( "U+" + aTmp );
        }
    }
    return output.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
