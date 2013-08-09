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
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <i18nutil/unicode.hxx>
#include "unicode_data.h"

// Workaround for glibc braindamage:
// glibc 2.4's langinfo.h does "#define CURRENCY_SYMBOL __CURRENCY_SYMBOL"
// which (obviously) breaks UnicodeType::CURRENCY_SYMBOL
#undef CURRENCY_SYMBOL

using namespace ::com::sun::star::i18n;

static const ScriptTypeList defaultTypeList[] = {
    { UnicodeScript_kBasicLatin,
      UnicodeScript_kBasicLatin,
      UnicodeScript_kBasicLatin },      // 0,
    { UnicodeScript_kLatin1Supplement,
      UnicodeScript_kLatin1Supplement,
      UnicodeScript_kLatin1Supplement },// 1,
    { UnicodeScript_kLatinExtendedA,
      UnicodeScript_kLatinExtendedA,
      UnicodeScript_kLatinExtendedA }, // 2,
    { UnicodeScript_kLatinExtendedB,
      UnicodeScript_kLatinExtendedB,
      UnicodeScript_kLatinExtendedB }, // 3,
    { UnicodeScript_kIPAExtension,
      UnicodeScript_kIPAExtension,
      UnicodeScript_kIPAExtension }, // 4,
    { UnicodeScript_kSpacingModifier,
      UnicodeScript_kSpacingModifier,
      UnicodeScript_kSpacingModifier }, // 5,
    { UnicodeScript_kCombiningDiacritical,
      UnicodeScript_kCombiningDiacritical,
      UnicodeScript_kCombiningDiacritical }, // 6,
    { UnicodeScript_kGreek,
      UnicodeScript_kGreek,
      UnicodeScript_kGreek }, // 7,
    { UnicodeScript_kCyrillic,
      UnicodeScript_kCyrillic,
      UnicodeScript_kCyrillic }, // 8,
    { UnicodeScript_kArmenian,
      UnicodeScript_kArmenian,
      UnicodeScript_kArmenian }, // 9,
    { UnicodeScript_kHebrew,
      UnicodeScript_kHebrew,
      UnicodeScript_kHebrew }, // 10,
    { UnicodeScript_kArabic,
      UnicodeScript_kArabic,
      UnicodeScript_kArabic }, // 11,
    { UnicodeScript_kSyriac,
      UnicodeScript_kSyriac,
      UnicodeScript_kSyriac }, // 12,
    { UnicodeScript_kThaana,
      UnicodeScript_kThaana,
      UnicodeScript_kThaana }, // 13,
    { UnicodeScript_kDevanagari,
      UnicodeScript_kDevanagari,
      UnicodeScript_kDevanagari }, // 14,
    { UnicodeScript_kBengali,
      UnicodeScript_kBengali,
      UnicodeScript_kBengali }, // 15,
    { UnicodeScript_kGurmukhi,
      UnicodeScript_kGurmukhi,
      UnicodeScript_kGurmukhi }, // 16,
    { UnicodeScript_kGujarati,
      UnicodeScript_kGujarati,
      UnicodeScript_kGujarati }, // 17,
    { UnicodeScript_kOriya,
      UnicodeScript_kOriya,
      UnicodeScript_kOriya }, // 18,
    { UnicodeScript_kTamil,
      UnicodeScript_kTamil,
      UnicodeScript_kTamil }, // 19,
    { UnicodeScript_kTelugu,
      UnicodeScript_kTelugu,
      UnicodeScript_kTelugu }, // 20,
    { UnicodeScript_kKannada,
      UnicodeScript_kKannada,
      UnicodeScript_kKannada }, // 21,
    { UnicodeScript_kMalayalam,
      UnicodeScript_kMalayalam,
      UnicodeScript_kMalayalam }, // 22,
    { UnicodeScript_kSinhala,
      UnicodeScript_kSinhala,
      UnicodeScript_kSinhala }, // 23,
    { UnicodeScript_kThai,
      UnicodeScript_kThai,
      UnicodeScript_kThai }, // 24,
    { UnicodeScript_kLao,
      UnicodeScript_kLao,
      UnicodeScript_kLao }, // 25,
    { UnicodeScript_kTibetan,
      UnicodeScript_kTibetan,
      UnicodeScript_kTibetan }, // 26,
    { UnicodeScript_kMyanmar,
      UnicodeScript_kMyanmar,
      UnicodeScript_kMyanmar }, // 27,
    { UnicodeScript_kGeorgian,
      UnicodeScript_kGeorgian,
      UnicodeScript_kGeorgian }, // 28,
    { UnicodeScript_kHangulJamo,
      UnicodeScript_kHangulJamo,
      UnicodeScript_kHangulJamo }, // 29,
    { UnicodeScript_kEthiopic,
      UnicodeScript_kEthiopic,
      UnicodeScript_kEthiopic }, // 30,
    { UnicodeScript_kCherokee,
      UnicodeScript_kCherokee,
      UnicodeScript_kCherokee }, // 31,
    { UnicodeScript_kUnifiedCanadianAboriginalSyllabics,
      UnicodeScript_kUnifiedCanadianAboriginalSyllabics,
      UnicodeScript_kUnifiedCanadianAboriginalSyllabics }, // 32,
    { UnicodeScript_kOgham,
      UnicodeScript_kOgham,
      UnicodeScript_kOgham }, // 33,
    { UnicodeScript_kRunic,
      UnicodeScript_kRunic,
      UnicodeScript_kRunic }, // 34,
    { UnicodeScript_kKhmer,
      UnicodeScript_kKhmer,
      UnicodeScript_kKhmer }, // 35,
    { UnicodeScript_kMongolian,
      UnicodeScript_kMongolian,
      UnicodeScript_kMongolian }, // 36,
    { UnicodeScript_kLatinExtendedAdditional,
      UnicodeScript_kLatinExtendedAdditional,
      UnicodeScript_kLatinExtendedAdditional }, // 37,
    { UnicodeScript_kGreekExtended,
      UnicodeScript_kGreekExtended,
      UnicodeScript_kGreekExtended }, // 38,
    { UnicodeScript_kGeneralPunctuation,
      UnicodeScript_kGeneralPunctuation,
      UnicodeScript_kGeneralPunctuation }, // 39,
    { UnicodeScript_kSuperSubScript,
      UnicodeScript_kSuperSubScript,
      UnicodeScript_kSuperSubScript }, // 40,
    { UnicodeScript_kCurrencySymbolScript,
      UnicodeScript_kCurrencySymbolScript,
      UnicodeScript_kCurrencySymbolScript }, // 41,
    { UnicodeScript_kSymbolCombiningMark,
      UnicodeScript_kSymbolCombiningMark,
      UnicodeScript_kSymbolCombiningMark }, // 42,
    { UnicodeScript_kLetterlikeSymbol,
      UnicodeScript_kLetterlikeSymbol,
      UnicodeScript_kLetterlikeSymbol }, // 43,
    { UnicodeScript_kNumberForm,
      UnicodeScript_kNumberForm,
      UnicodeScript_kNumberForm }, // 44,
    { UnicodeScript_kArrow,
      UnicodeScript_kArrow,
      UnicodeScript_kArrow }, // 45,
    { UnicodeScript_kMathOperator,
      UnicodeScript_kMathOperator,
      UnicodeScript_kMathOperator }, // 46,
    { UnicodeScript_kMiscTechnical,
      UnicodeScript_kMiscTechnical,
      UnicodeScript_kMiscTechnical }, // 47,
    { UnicodeScript_kControlPicture,
      UnicodeScript_kControlPicture,
      UnicodeScript_kControlPicture }, // 48,
    { UnicodeScript_kOpticalCharacter,
      UnicodeScript_kOpticalCharacter,
      UnicodeScript_kOpticalCharacter }, // 49,
    { UnicodeScript_kEnclosedAlphanumeric,
      UnicodeScript_kEnclosedAlphanumeric,
      UnicodeScript_kEnclosedAlphanumeric }, // 50,
    { UnicodeScript_kBoxDrawing,
      UnicodeScript_kBoxDrawing,
      UnicodeScript_kBoxDrawing }, // 51,
    { UnicodeScript_kBlockElement,
      UnicodeScript_kBlockElement,
      UnicodeScript_kBlockElement }, // 52,
    { UnicodeScript_kGeometricShape,
      UnicodeScript_kGeometricShape,
      UnicodeScript_kGeometricShape }, // 53,
    { UnicodeScript_kMiscSymbol,
      UnicodeScript_kMiscSymbol,
      UnicodeScript_kMiscSymbol }, // 54,
    { UnicodeScript_kDingbat,
      UnicodeScript_kDingbat,
      UnicodeScript_kDingbat }, // 55,
    { UnicodeScript_kBraillePatterns,
      UnicodeScript_kBraillePatterns,
      UnicodeScript_kBraillePatterns }, // 56,
    { UnicodeScript_kCJKRadicalsSupplement,
      UnicodeScript_kCJKRadicalsSupplement,
      UnicodeScript_kCJKRadicalsSupplement }, // 57,
    { UnicodeScript_kKangxiRadicals,
      UnicodeScript_kKangxiRadicals,
      UnicodeScript_kKangxiRadicals }, // 58,
    { UnicodeScript_kIdeographicDescriptionCharacters,
      UnicodeScript_kIdeographicDescriptionCharacters,
      UnicodeScript_kIdeographicDescriptionCharacters }, // 59,
    { UnicodeScript_kCJKSymbolPunctuation,
      UnicodeScript_kCJKSymbolPunctuation,
      UnicodeScript_kCJKSymbolPunctuation }, // 60,
    { UnicodeScript_kHiragana,
      UnicodeScript_kHiragana,
      UnicodeScript_kHiragana }, // 61,
    { UnicodeScript_kKatakana,
      UnicodeScript_kKatakana,
      UnicodeScript_kKatakana }, // 62,
    { UnicodeScript_kBopomofo,
      UnicodeScript_kBopomofo,
      UnicodeScript_kBopomofo }, // 63,
    { UnicodeScript_kHangulCompatibilityJamo,
      UnicodeScript_kHangulCompatibilityJamo,
      UnicodeScript_kHangulCompatibilityJamo }, // 64,
    { UnicodeScript_kKanbun,
      UnicodeScript_kKanbun,
      UnicodeScript_kKanbun }, // 65,
    { UnicodeScript_kBopomofoExtended,
      UnicodeScript_kBopomofoExtended,
      UnicodeScript_kBopomofoExtended }, // 66,
    { UnicodeScript_kEnclosedCJKLetterMonth,
      UnicodeScript_kEnclosedCJKLetterMonth,
      UnicodeScript_kEnclosedCJKLetterMonth }, // 67,
    { UnicodeScript_kCJKCompatibility,
      UnicodeScript_kCJKCompatibility,
      UnicodeScript_kCJKCompatibility }, // 68,
    { UnicodeScript_k_CJKUnifiedIdeographsExtensionA,
      UnicodeScript_k_CJKUnifiedIdeographsExtensionA,
      UnicodeScript_k_CJKUnifiedIdeographsExtensionA }, // 69,
    { UnicodeScript_kCJKUnifiedIdeograph,
      UnicodeScript_kCJKUnifiedIdeograph,
      UnicodeScript_kCJKUnifiedIdeograph }, // 70,
    { UnicodeScript_kYiSyllables,
      UnicodeScript_kYiSyllables,
      UnicodeScript_kYiSyllables }, // 71,
    { UnicodeScript_kYiRadicals,
      UnicodeScript_kYiRadicals,
      UnicodeScript_kYiRadicals }, // 72,
    { UnicodeScript_kHangulSyllable,
      UnicodeScript_kHangulSyllable,
      UnicodeScript_kHangulSyllable }, // 73,
    { UnicodeScript_kHighSurrogate,
      UnicodeScript_kHighSurrogate,
      UnicodeScript_kHighSurrogate }, // 74,
    { UnicodeScript_kHighPrivateUseSurrogate,
      UnicodeScript_kHighPrivateUseSurrogate,
      UnicodeScript_kHighPrivateUseSurrogate }, // 75,
    { UnicodeScript_kLowSurrogate,
      UnicodeScript_kLowSurrogate,
      UnicodeScript_kLowSurrogate }, // 76,
    { UnicodeScript_kPrivateUse,
      UnicodeScript_kPrivateUse,
      UnicodeScript_kPrivateUse }, // 77,
    { UnicodeScript_kCJKCompatibilityIdeograph,
      UnicodeScript_kCJKCompatibilityIdeograph,
      UnicodeScript_kCJKCompatibilityIdeograph }, // 78,
    { UnicodeScript_kAlphabeticPresentation,
      UnicodeScript_kAlphabeticPresentation,
      UnicodeScript_kAlphabeticPresentation }, // 79,
    { UnicodeScript_kArabicPresentationA,
      UnicodeScript_kArabicPresentationA,
      UnicodeScript_kArabicPresentationA }, // 80,
    { UnicodeScript_kCombiningHalfMark,
      UnicodeScript_kCombiningHalfMark,
      UnicodeScript_kCombiningHalfMark }, // 81,
    { UnicodeScript_kCJKCompatibilityForm,
      UnicodeScript_kCJKCompatibilityForm,
      UnicodeScript_kCJKCompatibilityForm }, // 82,
    { UnicodeScript_kSmallFormVariant,
      UnicodeScript_kSmallFormVariant,
      UnicodeScript_kSmallFormVariant }, // 83,
    { UnicodeScript_kArabicPresentationB,
      UnicodeScript_kArabicPresentationB,
      UnicodeScript_kArabicPresentationB }, // 84,
    { UnicodeScript_kNoScript,
      UnicodeScript_kNoScript,
      UnicodeScript_kNoScript }, // 85,
    { UnicodeScript_kHalfwidthFullwidthForm,
      UnicodeScript_kHalfwidthFullwidthForm,
      UnicodeScript_kHalfwidthFullwidthForm }, // 86,
    { UnicodeScript_kScriptCount,
      UnicodeScript_kScriptCount,
      UnicodeScript_kNoScript } // 87,
};

sal_Int16 SAL_CALL
unicode::getUnicodeScriptType( const sal_Unicode ch, const ScriptTypeList* typeList, sal_Int16 unknownType ) {

    if (!typeList) {
        typeList = defaultTypeList;
        unknownType = UnicodeScript_kNoScript;
    }

    sal_Int16 i = 0, type = typeList[0].to;
    while (type < UnicodeScript_kScriptCount && ch > UnicodeScriptType[type][UnicodeScriptTypeTo]) {
        type = typeList[++i].to;
    }

    return (type < UnicodeScript_kScriptCount &&
            ch >= UnicodeScriptType[typeList[i].from][UnicodeScriptTypeFrom]) ?
            typeList[i].value : unknownType;
}

sal_Unicode SAL_CALL
unicode::getUnicodeScriptStart( UnicodeScript type) {
    return UnicodeScriptType[type][UnicodeScriptTypeFrom];
}

sal_Unicode SAL_CALL
unicode::getUnicodeScriptEnd( UnicodeScript type) {
    return UnicodeScriptType[type][UnicodeScriptTypeTo];
}

sal_Int16 SAL_CALL
unicode::getUnicodeType( const sal_Unicode ch ) {
    static sal_Unicode c = 0x00;
    static sal_Int16 r = 0x00;

    if (ch == c) return r;
    else c = ch;

    sal_Int16 address = UnicodeTypeIndex[ch >> 8];
    return r = (sal_Int16)((address < UnicodeTypeNumberBlock) ? UnicodeTypeBlockValue[address] :
        UnicodeTypeValue[((address - UnicodeTypeNumberBlock) << 8) + (ch & 0xff)]);
}

sal_uInt8 SAL_CALL
unicode::getUnicodeDirection( const sal_Unicode ch ) {
    static sal_Unicode c = 0x00;
    static sal_uInt8 r = 0x00;

    if (ch == c) return r;
    else c = ch;

    sal_Int16 address = UnicodeDirectionIndex[ch >> 8];
    return r = ((address < UnicodeDirectionNumberBlock) ? UnicodeDirectionBlockValue[address] :
        UnicodeDirectionValue[((address - UnicodeDirectionNumberBlock) << 8) + (ch & 0xff)]);

}

#define bit(name)   (1 << name)

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
sal_Bool SAL_CALL func( const sal_Unicode ch) {\
    return (bit(getUnicodeType(ch)) & (mask)) != 0;\
}

IsType(unicode::isControl, CONTROLMASK)
IsType(unicode::isAlpha, ALPHAMASK)
IsType(unicode::isSpace, SPACEMASK)

#define CONTROLSPACE    bit(0x09)|bit(0x0a)|bit(0x0b)|bit(0x0c)|bit(0x0d)|\
            bit(0x1c)|bit(0x1d)|bit(0x1e)|bit(0x1f)

sal_Bool SAL_CALL unicode::isWhiteSpace( const sal_Unicode ch) {
    return (ch != 0xa0 && isSpace(ch)) || (ch <= 0x1F && (bit(ch) & (CONTROLSPACE)));
}

sal_Int16 SAL_CALL unicode::getScriptClassFromUScriptCode(UScriptCode eScript)
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

OString SAL_CALL unicode::getExemplerLanguageForUScriptCode(UScriptCode eScript)
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
#if (U_ICU_VERSION_MAJOR_NUM > 4)
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
    }
    return sRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
