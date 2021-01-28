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
#include "unicode_data.h"
#include <rtl/character.hxx>
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
unicode::getUnicodeType( const sal_Unicode ch ) {
    static sal_Unicode c = 0x00;
    static sal_Int16 r = 0x00;

    if (ch == c) return r;
    else c = ch;

    sal_Int16 address = UnicodeTypeIndex[ch >> 8];
    r = static_cast<sal_Int16>(
            (address < UnicodeTypeNumberBlock)
            ? UnicodeTypeBlockValue[address]
            : UnicodeTypeValue[((address - UnicodeTypeNumberBlock) << 8) + (ch & 0xff)]);
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
bool func( const sal_Unicode ch) {\
    return (bit(getUnicodeType(ch)) & (mask)) != 0;\
}

IsType(unicode::isControl, CONTROLMASK)
IsType(unicode::isAlpha, ALPHAMASK)
IsType(unicode::isSpace, SPACEMASK)

#define CONTROLSPACE    bit(0x09)|bit(0x0a)|bit(0x0b)|bit(0x0c)|bit(0x0d)|\
            bit(0x1c)|bit(0x1d)|bit(0x1e)|bit(0x1f)

bool unicode::isWhiteSpace( const sal_Unicode ch) {
    return (ch != 0xa0 && isSpace(ch)) || (ch <= 0x1F && (bit(ch) & (CONTROLSPACE)));
}

sal_Int16 unicode::getScriptClassFromUScriptCode(UScriptCode eScript)
{
    //See unicode/uscript.h
    static const sal_Int16 scriptTypes[] =
    {
        ScriptType::WEAK, ScriptType::WEAK, ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::COMPLEX,
        ScriptType::ASIAN, ScriptType::LATIN, ScriptType::LATIN, ScriptType::LATIN, ScriptType::COMPLEX,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::LATIN, ScriptType::LATIN,
    // 15
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::ASIAN, ScriptType::ASIAN, ScriptType::COMPLEX,
        ScriptType::ASIAN, ScriptType::COMPLEX, ScriptType::ASIAN, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::LATIN,
    // 30
        ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::LATIN, ScriptType::ASIAN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    // 45
        ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::LATIN, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::LATIN,
        ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    // 60
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::LATIN, ScriptType::COMPLEX,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::ASIAN, ScriptType::ASIAN,
    // 75
        ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::LATIN, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    // 90
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::WEAK, ScriptType::WEAK, ScriptType::COMPLEX,
    // 105
        ScriptType::ASIAN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::ASIAN,
    // 120
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::WEAK, ScriptType::WEAK,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    // 135
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
        ScriptType::COMPLEX,
        ScriptType::WEAK
    };

    sal_Int16 nRet;
    if (eScript < USCRIPT_COMMON)
        nRet = ScriptType::WEAK;
    else if (static_cast<size_t>(eScript) >= SAL_N_ELEMENTS(scriptTypes))
        nRet = ScriptType::COMPLEX;         // anything new is going to be pretty wild
    else
        nRet = scriptTypes[eScript];
    return nRet;
}

OString unicode::getExemplarLanguageForUScriptCode(UScriptCode eScript)
{
    OString sRet;
    switch (eScript)
    {
        case USCRIPT_CODE_LIMIT:
        case USCRIPT_INVALID_CODE:
            sRet = "zxx";
            break;
        case USCRIPT_COMMON:
        case USCRIPT_INHERITED:
            sRet = "und";
            break;
        case USCRIPT_MATHEMATICAL_NOTATION:
        case USCRIPT_SYMBOLS:
            sRet = "zxx";
            break;
        case USCRIPT_UNWRITTEN_LANGUAGES:
        case USCRIPT_UNKNOWN:
            sRet = "und";
            break;
        case USCRIPT_ARABIC:
            sRet = "ar";
            break;
        case USCRIPT_ARMENIAN:
            sRet = "hy";
            break;
        case USCRIPT_BENGALI:
            sRet = "bn";
            break;
        case USCRIPT_BOPOMOFO:
            sRet = "zh";
            break;
        case USCRIPT_CHEROKEE:
            sRet = "chr";
            break;
        case USCRIPT_COPTIC:
            sRet = "cop";
            break;
        case USCRIPT_CYRILLIC:
            sRet = "ru";
            break;
        case USCRIPT_DESERET:
            sRet = "en";
            break;
        case USCRIPT_DEVANAGARI:
            sRet = "hi";
            break;
        case USCRIPT_ETHIOPIC:
            sRet = "am";
            break;
        case USCRIPT_GEORGIAN:
            sRet = "ka";
            break;
        case USCRIPT_GOTHIC:
            sRet = "got";
            break;
        case USCRIPT_GREEK:
            sRet = "el";
            break;
        case USCRIPT_GUJARATI:
            sRet = "gu";
            break;
        case USCRIPT_GURMUKHI:
            sRet = "pa";
            break;
        case USCRIPT_HAN:
            sRet = "zh";
            break;
        case USCRIPT_HANGUL:
            sRet = "ko";
            break;
        case USCRIPT_HEBREW:
            sRet = "hr";
            break;
        case USCRIPT_HIRAGANA:
            sRet = "ja";
            break;
        case USCRIPT_KANNADA:
            sRet = "kn";
            break;
        case USCRIPT_KATAKANA:
            sRet = "ja";
            break;
        case USCRIPT_KHMER:
            sRet = "km";
            break;
        case USCRIPT_LAO:
            sRet = "lo";
            break;
        case USCRIPT_LATIN:
            sRet = "en";
            break;
        case USCRIPT_MALAYALAM:
            sRet = "ml";
            break;
        case USCRIPT_MONGOLIAN:
            sRet = "mn";
            break;
        case USCRIPT_MYANMAR:
            sRet = "my";
            break;
        case USCRIPT_OGHAM:
            sRet = "pgl";
            break;
        case USCRIPT_OLD_ITALIC:
            sRet = "osc";
            break;
        case USCRIPT_ORIYA:
            sRet = "or";
            break;
        case USCRIPT_RUNIC:
            sRet = "ang";
            break;
        case USCRIPT_SINHALA:
            sRet = "si";
            break;
        case USCRIPT_SYRIAC:
            sRet = "syr";
            break;
        case USCRIPT_TAMIL:
            sRet = "ta";
            break;
        case USCRIPT_TELUGU:
            sRet = "te";
            break;
        case USCRIPT_THAANA:
            sRet = "dv";
            break;
        case USCRIPT_THAI:
            sRet = "th";
            break;
        case USCRIPT_TIBETAN:
            sRet = "bo";
            break;
        case USCRIPT_CANADIAN_ABORIGINAL:
            sRet = "iu";
            break;
        case USCRIPT_YI:
            sRet = "ii";
            break;
        case USCRIPT_TAGALOG:
            sRet = "tl";
            break;
        case USCRIPT_HANUNOO:
            sRet = "hnn";
            break;
        case USCRIPT_BUHID:
            sRet = "bku";
            break;
        case USCRIPT_TAGBANWA:
            sRet = "tbw";
            break;
        case USCRIPT_BRAILLE:
            sRet = "en";
            break;
        case USCRIPT_CYPRIOT:
            sRet = "ecy";
            break;
        case USCRIPT_LIMBU:
            sRet = "lif";
            break;
        case USCRIPT_LINEAR_B:
            sRet = "gmy";
            break;
        case USCRIPT_OSMANYA:
            sRet = "so";
            break;
        case USCRIPT_SHAVIAN:
            sRet = "en";
            break;
        case USCRIPT_TAI_LE:
            sRet = "tdd";
            break;
        case USCRIPT_UGARITIC:
            sRet = "uga";
            break;
        case USCRIPT_KATAKANA_OR_HIRAGANA:
            sRet = "ja";
            break;
        case USCRIPT_BUGINESE:
            sRet = "bug";
            break;
        case USCRIPT_GLAGOLITIC:
            sRet = "ch";
            break;
        case USCRIPT_KHAROSHTHI:
            sRet = "pra";
            break;
        case USCRIPT_SYLOTI_NAGRI:
            sRet = "syl";
            break;
        case USCRIPT_NEW_TAI_LUE:
            sRet = "khb";
            break;
        case USCRIPT_TIFINAGH:
            sRet = "tmh";
            break;
        case USCRIPT_OLD_PERSIAN:
            sRet = "peo";
            break;
        case USCRIPT_BALINESE:
            sRet = "ban";
            break;
        case USCRIPT_BATAK:
            sRet = "btk";
            break;
        case USCRIPT_BLISSYMBOLS:
            sRet = "en";
            break;
        case USCRIPT_BRAHMI:
            sRet = "pra";
            break;
        case USCRIPT_CHAM:
            sRet = "cja";
            break;
        case USCRIPT_CIRTH:
            sRet = "sjn";
            break;
        case USCRIPT_OLD_CHURCH_SLAVONIC_CYRILLIC:
            sRet = "cu";
            break;
        case USCRIPT_DEMOTIC_EGYPTIAN:
        case USCRIPT_HIERATIC_EGYPTIAN:
        case USCRIPT_EGYPTIAN_HIEROGLYPHS:
            sRet = "egy";
            break;
        case USCRIPT_KHUTSURI:
            sRet = "ka";
            break;
        case USCRIPT_SIMPLIFIED_HAN:
            sRet = "zh";
            break;
        case USCRIPT_TRADITIONAL_HAN:
            sRet = "zh";
            break;
        case USCRIPT_PAHAWH_HMONG:
            sRet = "blu";
            break;
        case USCRIPT_OLD_HUNGARIAN:
            sRet = "ohu";
            break;
        case USCRIPT_HARAPPAN_INDUS:
            sRet = "xiv";
            break;
        case USCRIPT_JAVANESE:
            sRet = "kaw";
            break;
        case USCRIPT_KAYAH_LI:
            sRet = "eky";
            break;
        case USCRIPT_LATIN_FRAKTUR:
            sRet = "de";
            break;
        case USCRIPT_LATIN_GAELIC:
            sRet = "ga";
            break;
        case USCRIPT_LEPCHA:
            sRet = "lep";
            break;
        case USCRIPT_LINEAR_A:
            sRet = "ecr";
            break;
        case USCRIPT_MAYAN_HIEROGLYPHS:
            sRet = "myn";
            break;
        case USCRIPT_MEROITIC:
            sRet = "xmr";
            break;
        case USCRIPT_NKO:
            sRet = "nqo";
            break;
        case USCRIPT_ORKHON:
            sRet = "otk";
            break;
        case USCRIPT_OLD_PERMIC:
            sRet = "kv";
            break;
        case USCRIPT_PHAGS_PA:
            sRet = "xng";
            break;
        case USCRIPT_PHOENICIAN:
            sRet = "phn";
            break;
        case USCRIPT_PHONETIC_POLLARD:
            sRet = "hmd";
            break;
        case USCRIPT_RONGORONGO:
            sRet = "rap";
            break;
        case USCRIPT_SARATI:
            sRet = "qya";
            break;
        case USCRIPT_ESTRANGELO_SYRIAC:
            sRet = "syr";
            break;
        case USCRIPT_WESTERN_SYRIAC:
            sRet = "tru";
            break;
        case USCRIPT_EASTERN_SYRIAC:
            sRet = "aii";
            break;
        case USCRIPT_TENGWAR:
            sRet = "sjn";
            break;
        case USCRIPT_VAI:
            sRet = "vai";
            break;
        case USCRIPT_VISIBLE_SPEECH:
            sRet = "en";
            break;
        case USCRIPT_CUNEIFORM:
            sRet = "akk";
            break;
        case USCRIPT_CARIAN:
            sRet = "xcr";
            break;
        case USCRIPT_JAPANESE:
            sRet = "ja";
            break;
        case USCRIPT_LANNA:
            sRet = "nod";
            break;
        case USCRIPT_LYCIAN:
            sRet = "xlc";
            break;
        case USCRIPT_LYDIAN:
            sRet = "xld";
            break;
        case USCRIPT_OL_CHIKI:
            sRet = "sat";
            break;
        case USCRIPT_REJANG:
            sRet = "rej";
            break;
        case USCRIPT_SAURASHTRA:
            sRet = "saz";
            break;
        case USCRIPT_SIGN_WRITING:
            sRet = "en";
            break;
        case USCRIPT_SUNDANESE:
            sRet = "su";
            break;
        case USCRIPT_MOON:
            sRet = "en";
            break;
        case USCRIPT_MEITEI_MAYEK:
            sRet = "mni";
            break;
        case USCRIPT_IMPERIAL_ARAMAIC:
            sRet = "arc";
            break;
        case USCRIPT_AVESTAN:
            sRet = "ae";
            break;
        case USCRIPT_CHAKMA:
            sRet = "ccp";
            break;
        case USCRIPT_KOREAN:
            sRet = "ko";
            break;
        case USCRIPT_KAITHI:
            sRet = "awa";
            break;
        case USCRIPT_MANICHAEAN:
            sRet = "xmn";
            break;
        case USCRIPT_INSCRIPTIONAL_PAHLAVI:
        case USCRIPT_PSALTER_PAHLAVI:
        case USCRIPT_BOOK_PAHLAVI:
        case USCRIPT_INSCRIPTIONAL_PARTHIAN:
            sRet = "xpr";
            break;
        case USCRIPT_SAMARITAN:
            sRet = "heb";
            break;
        case USCRIPT_TAI_VIET:
            sRet = "blt";
            break;
        case USCRIPT_MANDAEAN: /* Aliased to USCRIPT_MANDAIC in icu 4.6. */
            sRet = "mic";
            break;
#if (U_ICU_VERSION_MAJOR_NUM > 4) || (U_ICU_VERSION_MAJOR_NUM == 4 && U_ICU_VERSION_MINOR_NUM >= 4)
        case USCRIPT_NABATAEAN: //no language with an assigned code yet
            sRet = "mis";
            break;
        case USCRIPT_PALMYRENE: //no language with an assigned code yet
            sRet = "mis";
            break;
        case USCRIPT_BAMUM:
            sRet = "bax";
            break;
        case USCRIPT_LISU:
            sRet = "lis";
            break;
        case USCRIPT_NAKHI_GEBA:
            sRet = "nxq";
            break;
        case USCRIPT_OLD_SOUTH_ARABIAN:
            sRet = "xsa";
            break;
        case USCRIPT_BASSA_VAH:
            sRet = "bsq";
            break;
        case USCRIPT_DUPLOYAN_SHORTAND:
            sRet = "fr";
            break;
        case USCRIPT_ELBASAN:
            sRet = "sq";
            break;
        case USCRIPT_GRANTHA:
            sRet = "ta";
            break;
        case USCRIPT_KPELLE:
            sRet = "kpe";
            break;
        case USCRIPT_LOMA:
            sRet = "lom";
            break;
        case USCRIPT_MENDE:
            sRet = "men";
            break;
        case USCRIPT_MEROITIC_CURSIVE:
            sRet = "xmr";
            break;
        case USCRIPT_OLD_NORTH_ARABIAN:
            sRet = "xna";
            break;
        case USCRIPT_SINDHI:
            sRet = "sd";
            break;
        case USCRIPT_WARANG_CITI:
            sRet = "hoc";
            break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM > 4) || (U_ICU_VERSION_MAJOR_NUM == 4 && U_ICU_VERSION_MINOR_NUM >= 8)
        case USCRIPT_AFAKA:
            sRet = "djk";
            break;
        case USCRIPT_JURCHEN:
            sRet = "juc";
            break;
        case USCRIPT_MRO:
            sRet = "cmr";
            break;
        case USCRIPT_NUSHU: //no language with an assigned code yet
            sRet = "mis";
            break;
        case USCRIPT_SHARADA:
            sRet = "sa";
            break;
        case USCRIPT_SORA_SOMPENG:
            sRet = "srb";
            break;
        case USCRIPT_TAKRI:
            sRet = "doi";
            break;
        case USCRIPT_TANGUT:
            sRet = "txg";
            break;
        case USCRIPT_WOLEAI:
            sRet = "woe";
            break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 49)
        case USCRIPT_ANATOLIAN_HIEROGLYPHS:
            sRet = "hlu";
            break;
        case USCRIPT_KHOJKI:
            sRet = "gu";
            break;
        case USCRIPT_TIRHUTA:
            sRet = "mai";
            break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 52)
        case USCRIPT_CAUCASIAN_ALBANIAN:
            sRet = "xag";
            break;
        case USCRIPT_MAHAJANI:
            sRet = "mwr";
            break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 54)
        case USCRIPT_AHOM:
            sRet = "aho";
            break;
        case USCRIPT_HATRAN:
            sRet = "qly-Hatr";
            break;
        case USCRIPT_MODI:
            sRet = "mr-Modi";
            break;
        case USCRIPT_MULTANI:
            sRet = "skr-Mutl";
            break;
        case USCRIPT_PAU_CIN_HAU:
            sRet = "ctd-Pauc";
            break;
        case USCRIPT_SIDDHAM:
            sRet = "sa-Sidd";
            break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 58)
        case USCRIPT_ADLAM:
            sRet = "mis";   // Adlm - Adlam for Fulani, no language code
            break;
        case USCRIPT_BHAIKSUKI:
            sRet = "mis";   // Bhks - Bhaiksuki for some Buddhist texts, no language code
            break;
        case USCRIPT_MARCHEN:
            sRet = "bo-Marc";
            break;
        case USCRIPT_NEWA:
            sRet = "new-Newa";
            break;
        case USCRIPT_OSAGE:
            sRet = "osa-Osge";
            break;
        case USCRIPT_HAN_WITH_BOPOMOFO:
            sRet = "mis";   // Hanb - Han with Bopomofo, zh-Hanb ?
            break;
        case USCRIPT_JAMO:
            sRet = "ko";   // Jamo - elements of Hangul Syllables
            break;
        case USCRIPT_SYMBOLS_EMOJI:
            sRet = "mis";   // Zsye - Emoji variant
            break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 60)
        case USCRIPT_MASARAM_GONDI:
            sRet = "gon-Gonm";  // macro language code, could be wsg,esg,gno
            break;
        case USCRIPT_SOYOMBO:
            sRet = "mn-Soyo";   // abugida to write Mongolian, also Tibetan and Sanskrit
            break;
        case USCRIPT_ZANABAZAR_SQUARE:
            sRet = "mn-Zanb";   // abugida to write Mongolian
            break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 62)
        case USCRIPT_DOGRA:
            sRet = "dgo";       // Dogri proper
            break;
        case USCRIPT_GUNJALA_GONDI:
            sRet = "wsg";       // Adilabad Gondi
            break;
        case USCRIPT_MAKASAR:
            sRet = "mak";
            break;
        case USCRIPT_MEDEFAIDRIN:
            sRet = "mis-Medf";  // Uncoded with script
            break;
        case USCRIPT_HANIFI_ROHINGYA:
            sRet = "rhg";
            break;
        case USCRIPT_SOGDIAN:
            sRet = "sog";
            break;
        case USCRIPT_OLD_SOGDIAN:
            sRet = "sog";
            break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 64)
        case USCRIPT_ELYMAIC:
            sRet = "arc-Elym";
            break;
        case USCRIPT_NYIAKENG_PUACHUE_HMONG:
            sRet = "hmn-Hmnp";  // macrolanguage code
            break;
        case USCRIPT_NANDINAGARI:
            sRet = "sa-Nand";
            break;
        case USCRIPT_WANCHO:
            sRet = "nnp-Wcho";
            break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 66)
        case USCRIPT_CHORASMIAN:
            sRet = "xco-Chrs";
            break;
        case USCRIPT_DIVES_AKURU:
            sRet = "dv-Diak";
            break;
        case USCRIPT_KHITAN_SMALL_SCRIPT:
            sRet = "zkt-Kits";
            break;
        case USCRIPT_YEZIDI:
            sRet = "kmr-Yezi";
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
        aLangTag.reset("fr-FR");

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
        nUnicode = sIn.copy(0, nUPlus).toUInt32(16);
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
            nUnicode = sIn.copy(0, nUPlus).toUInt32(16);
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
            output.append( "U+" );
            output.append( aTmp );
        }
    }
    return output.toString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
