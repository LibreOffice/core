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
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/languagetagicu.hxx>
#include <i18nutil/unicode.hxx>
#include <sal/log.hxx>
#include <unicode/numfmt.h>
#include "unicode_data.h"
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <rtl/character.hxx>
#include <memory>

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
bool SAL_CALL func( const sal_Unicode ch) {\
    return (bit(getUnicodeType(ch)) & (mask)) != 0;\
}

IsType(unicode::isControl, CONTROLMASK)
IsType(unicode::isAlpha, ALPHAMASK)
IsType(unicode::isSpace, SPACEMASK)

#define CONTROLSPACE    bit(0x09)|bit(0x0a)|bit(0x0b)|bit(0x0c)|bit(0x0d)|\
            bit(0x1c)|bit(0x1d)|bit(0x1e)|bit(0x1f)

bool SAL_CALL unicode::isWhiteSpace( const sal_Unicode ch) {
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

OString SAL_CALL unicode::getExemplarLanguageForUScriptCode(UScriptCode eScript)
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
            sRet = "mis";   // Jamo - Jamo subset of Hangul, ko-Jamo ?
            break;
        case USCRIPT_SYMBOLS_EMOJI:
            sRet = "mis";   // Zsye - Emoji variant
            break;
#endif
    }
    return sRet;
}

//Format a number as a percentage according to the rules of the given
//language, e.g. 100 -> "100%" for en-US vs "100 %" for de-DE
OUString SAL_CALL unicode::formatPercent(double dNumber,
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

    std::unique_ptr<NumberFormat> xF(
        NumberFormat::createPercentInstance(aLocale, errorCode));
    if(U_FAILURE(errorCode))
    {
        SAL_WARN("i18n", "NumberFormat::createPercentInstance failed");
        return OUString::number(dNumber) + "%";
    }

    UnicodeString output;
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
                    else if( isxdigit(uChar) )
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
        sIn = maInput.copy(2).toString();
        nUPlus = sIn.indexOf("U+");
    }
    else
        sIn = maInput.toString();
    while( nUPlus != -1 )
    {
        nUnicode = sIn.copy(0, nUPlus).toUInt32(16);
        //prevent creating control characters or invalid Unicode values
        if( !rtl::isUnicodeCodePoint(nUnicode) || nUnicode < 0x20  )
            maInput = sIn.copy(nUPlus);
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
