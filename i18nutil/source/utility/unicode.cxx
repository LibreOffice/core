/*************************************************************************
 *
 *  $RCSfile: unicode.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 12:26:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <i18nutil/unicode.hxx>
#include "unicode_data.h"

using namespace ::com::sun::star::i18n;

static ScriptTypeList defaultTypeList[] = {
    { UnicodeScript_kBasicLatin, UnicodeScript_kBasicLatin }, // 0,
    { UnicodeScript_kLatin1Supplement, UnicodeScript_kLatin1Supplement }, // 1,
    { UnicodeScript_kLatinExtendedA, UnicodeScript_kLatinExtendedA }, // 2,
    { UnicodeScript_kLatinExtendedB, UnicodeScript_kLatinExtendedB }, // 3,
    { UnicodeScript_kIPAExtension, UnicodeScript_kIPAExtension }, // 4,
    { UnicodeScript_kSpacingModifier, UnicodeScript_kSpacingModifier }, // 5,
    { UnicodeScript_kCombiningDiacritical, UnicodeScript_kCombiningDiacritical }, // 6,
    { UnicodeScript_kGreek, UnicodeScript_kGreek }, // 7,
    { UnicodeScript_kCyrillic, UnicodeScript_kCyrillic }, // 8,
    { UnicodeScript_kArmenian, UnicodeScript_kArmenian }, // 9,
    { UnicodeScript_kHebrew, UnicodeScript_kHebrew }, // 10,
    { UnicodeScript_kArabic, UnicodeScript_kArabic }, // 11,
    { UnicodeScript_kSyriac, UnicodeScript_kSyriac }, // 12,
    { UnicodeScript_kThaana, UnicodeScript_kThaana }, // 13,
    { UnicodeScript_kDevanagari, UnicodeScript_kDevanagari }, // 14,
    { UnicodeScript_kBengali, UnicodeScript_kBengali }, // 15,
    { UnicodeScript_kGurmukhi, UnicodeScript_kGurmukhi }, // 16,
    { UnicodeScript_kGujarati, UnicodeScript_kGujarati }, // 17,
    { UnicodeScript_kOriya, UnicodeScript_kOriya }, // 18,
    { UnicodeScript_kTamil, UnicodeScript_kTamil }, // 19,
    { UnicodeScript_kTelugu, UnicodeScript_kTelugu }, // 20,
    { UnicodeScript_kKannada, UnicodeScript_kKannada }, // 21,
    { UnicodeScript_kMalayalam, UnicodeScript_kMalayalam }, // 22,
    { UnicodeScript_kSinhala, UnicodeScript_kSinhala }, // 23,
    { UnicodeScript_kThai, UnicodeScript_kThai }, // 24,
    { UnicodeScript_kLao, UnicodeScript_kLao }, // 25,
    { UnicodeScript_kTibetan, UnicodeScript_kTibetan }, // 26,
    { UnicodeScript_kMyanmar, UnicodeScript_kMyanmar }, // 27,
    { UnicodeScript_kGeorgian, UnicodeScript_kGeorgian }, // 28,
    { UnicodeScript_kHangulJamo, UnicodeScript_kHangulJamo }, // 29,
    { UnicodeScript_kEthiopic, UnicodeScript_kEthiopic }, // 30,
    { UnicodeScript_kCherokee, UnicodeScript_kCherokee }, // 31,
    { UnicodeScript_kUnifiedCanadianAboriginalSyllabics, UnicodeScript_kUnifiedCanadianAboriginalSyllabics }, // 32,
    { UnicodeScript_kOgham, UnicodeScript_kOgham }, // 33,
    { UnicodeScript_kRunic, UnicodeScript_kRunic }, // 34,
    { UnicodeScript_kKhmer, UnicodeScript_kKhmer }, // 35,
    { UnicodeScript_kMongolian, UnicodeScript_kMongolian }, // 36,
    { UnicodeScript_kLatinExtendedAdditional, UnicodeScript_kLatinExtendedAdditional }, // 37,
    { UnicodeScript_kGreekExtended, UnicodeScript_kGreekExtended }, // 38,
    { UnicodeScript_kGeneralPunctuation, UnicodeScript_kGeneralPunctuation }, // 39,
    { UnicodeScript_kSuperSubScript, UnicodeScript_kSuperSubScript }, // 40,
    { UnicodeScript_kCurrencySymbolScript, UnicodeScript_kCurrencySymbolScript }, // 41,
    { UnicodeScript_kSymbolCombiningMark, UnicodeScript_kSymbolCombiningMark }, // 42,
    { UnicodeScript_kLetterlikeSymbol, UnicodeScript_kLetterlikeSymbol }, // 43,
    { UnicodeScript_kNumberForm, UnicodeScript_kNumberForm }, // 44,
    { UnicodeScript_kArrow, UnicodeScript_kArrow }, // 45,
    { UnicodeScript_kMathOperator, UnicodeScript_kMathOperator }, // 46,
    { UnicodeScript_kMiscTechnical, UnicodeScript_kMiscTechnical }, // 47,
    { UnicodeScript_kControlPicture, UnicodeScript_kControlPicture }, // 48,
    { UnicodeScript_kOpticalCharacter, UnicodeScript_kOpticalCharacter }, // 49,
    { UnicodeScript_kEnclosedAlphanumeric, UnicodeScript_kEnclosedAlphanumeric }, // 50,
    { UnicodeScript_kBoxDrawing, UnicodeScript_kBoxDrawing }, // 51,
    { UnicodeScript_kBlockElement, UnicodeScript_kBlockElement }, // 52,
    { UnicodeScript_kGeometricShape, UnicodeScript_kGeometricShape }, // 53,
    { UnicodeScript_kMiscSymbol, UnicodeScript_kMiscSymbol }, // 54,
    { UnicodeScript_kDingbat, UnicodeScript_kDingbat }, // 55,
    { UnicodeScript_kBraillePatterns, UnicodeScript_kBraillePatterns }, // 56,
    { UnicodeScript_kCJKRadicalsSupplement, UnicodeScript_kCJKRadicalsSupplement }, // 57,
    { UnicodeScript_kKangxiRadicals, UnicodeScript_kKangxiRadicals }, // 58,
    { UnicodeScript_kIdeographicDescriptionCharacters, UnicodeScript_kIdeographicDescriptionCharacters }, // 59,
    { UnicodeScript_kCJKSymbolPunctuation, UnicodeScript_kCJKSymbolPunctuation }, // 60,
    { UnicodeScript_kHiragana, UnicodeScript_kHiragana }, // 61,
    { UnicodeScript_kKatakana, UnicodeScript_kKatakana }, // 62,
    { UnicodeScript_kBopomofo, UnicodeScript_kBopomofo }, // 63,
    { UnicodeScript_kHangulCompatibilityJamo, UnicodeScript_kHangulCompatibilityJamo }, // 64,
    { UnicodeScript_kKanbun, UnicodeScript_kKanbun }, // 65,
    { UnicodeScript_kBopomofoExtended, UnicodeScript_kBopomofoExtended }, // 66,
    { UnicodeScript_kEnclosedCJKLetterMonth, UnicodeScript_kEnclosedCJKLetterMonth }, // 67,
    { UnicodeScript_kCJKCompatibility, UnicodeScript_kCJKCompatibility }, // 68,
    { UnicodeScript_k_CJKUnifiedIdeographsExtensionA, UnicodeScript_k_CJKUnifiedIdeographsExtensionA }, // 69,
    { UnicodeScript_kCJKUnifiedIdeograph, UnicodeScript_kCJKUnifiedIdeograph }, // 70,
    { UnicodeScript_kYiSyllables, UnicodeScript_kYiSyllables }, // 71,
    { UnicodeScript_kYiRadicals, UnicodeScript_kYiRadicals }, // 72,
    { UnicodeScript_kHangulSyllable, UnicodeScript_kHangulSyllable }, // 73,
    { UnicodeScript_kHighSurrogate, UnicodeScript_kHighSurrogate }, // 74,
    { UnicodeScript_kHighPrivateUseSurrogate, UnicodeScript_kHighPrivateUseSurrogate }, // 75,
    { UnicodeScript_kLowSurrogate, UnicodeScript_kLowSurrogate }, // 76,
    { UnicodeScript_kPrivateUse, UnicodeScript_kPrivateUse }, // 77,
    { UnicodeScript_kCJKCompatibilityIdeograph, UnicodeScript_kCJKCompatibilityIdeograph }, // 78,
    { UnicodeScript_kAlphabeticPresentation, UnicodeScript_kAlphabeticPresentation }, // 79,
    { UnicodeScript_kArabicPresentationA, UnicodeScript_kArabicPresentationA }, // 80,
    { UnicodeScript_kCombiningHalfMark, UnicodeScript_kCombiningHalfMark }, // 81,
    { UnicodeScript_kCJKCompatibilityForm, UnicodeScript_kCJKCompatibilityForm }, // 82,
    { UnicodeScript_kSmallFormVariant, UnicodeScript_kSmallFormVariant }, // 83,
    { UnicodeScript_kArabicPresentationB, UnicodeScript_kArabicPresentationB }, // 84,
    { UnicodeScript_kNoScript, UnicodeScript_kNoScript }, // 85,
    { UnicodeScript_kHalfwidthFullwidthForm, UnicodeScript_kHalfwidthFullwidthForm }, // 86,
    { UnicodeScript_kScriptCount, UnicodeScript_kNoScript } // 87,
};

sal_Int16 SAL_CALL
unicode::getUnicodeScriptType( const sal_Unicode ch, ScriptTypeList* typeList, sal_Int16 unknownType ) {

    if (!typeList) {
    typeList = defaultTypeList;
    unknownType = UnicodeScript_kNoScript;
    }

    sal_Int16 i = 0, type = typeList[0].from;
    while (type < UnicodeScript_kScriptCount && ch > UnicodeScriptType[type][UnicodeScriptTypeTo]) {
    type = typeList[++i].from;
    }

    return (type < UnicodeScript_kScriptCount &&
            ch >= UnicodeScriptType[type][UnicodeScriptTypeFrom]) ?
            typeList[i].to : unknownType;
}

sal_Bool SAL_CALL
unicode::isUnicodeScriptType( const sal_Unicode ch, sal_Int16 type) {
    return ch >= UnicodeScriptType[type][UnicodeScriptTypeFrom] &&
        ch <= UnicodeScriptType[type][UnicodeScriptTypeTo];
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

#define DIGITMASK   bit(UnicodeType::DECIMAL_DIGIT_NUMBER)|\
            bit(UnicodeType::LETTER_NUMBER)|\
            bit(UnicodeType::OTHER_NUMBER)

#define ALPHAMASK   UPPERMASK|LOWERMASK|TITLEMASK|\
            bit(UnicodeType::MODIFIER_LETTER)|\
            bit(UnicodeType::OTHER_LETTER)

#define BASEMASK    DIGITMASK|ALPHAMASK|\
            bit(UnicodeType::NON_SPACING_MARK)|\
            bit(UnicodeType::ENCLOSING_MARK)|\
            bit(UnicodeType::COMBINING_SPACING_MARK)

#define SPACEMASK   bit(UnicodeType::SPACE_SEPARATOR)|\
            bit(UnicodeType::LINE_SEPARATOR)|\
            bit(UnicodeType::PARAGRAPH_SEPARATOR)

#define PUNCTUATIONMASK bit(UnicodeType::DASH_PUNCTUATION)|\
            bit(UnicodeType::INITIAL_PUNCTUATION)|\
            bit(UnicodeType::FINAL_PUNCTUATION)|\
            bit(UnicodeType::CONNECTOR_PUNCTUATION)|\
            bit(UnicodeType::OTHER_PUNCTUATION)

#define SYMBOLMASK  bit(UnicodeType::MATH_SYMBOL)|\
            bit(UnicodeType::CURRENCY_SYMBOL)|\
            bit(UnicodeType::MODIFIER_SYMBOL)|\
            bit(UnicodeType::OTHER_SYMBOL)

#define PRINTMASK   BASEMASK|SPACEMASK|PUNCTUATIONMASK|SYMBOLMASK

#define CONTROLMASK bit(UnicodeType::CONTROL)|\
            bit(UnicodeType::FORMAT)|\
            bit(UnicodeType::LINE_SEPARATOR)|\
            bit(UnicodeType::PARAGRAPH_SEPARATOR)

#define IsType(func, mask)  \
sal_Bool SAL_CALL func( const sal_Unicode ch) {\
    return (bit(getUnicodeType(ch)) & (mask)) != 0;\
}

IsType(unicode::isUpper, UPPERMASK)
IsType(unicode::isLower, LOWERMASK)
IsType(unicode::isTitle, DIGITMASK)
IsType(unicode::isControl, CONTROLMASK)
IsType(unicode::isPrint, PRINTMASK)
IsType(unicode::isAlpha, ALPHAMASK)
IsType(unicode::isDigit, DIGITMASK)
IsType(unicode::isAlphaDigit, ALPHAMASK|DIGITMASK)
IsType(unicode::isSpace, SPACEMASK)
IsType(unicode::isBase, BASEMASK)
IsType(unicode::isPunctuation, PUNCTUATIONMASK)

#define CONTROLSPACE    bit(0x09)|bit(0x0a)|bit(0x0b)|bit(0x0c)|bit(0x0d)|\
            bit(0x1c)|bit(0x1d)|bit(0x1e)|bit(0x1f)

sal_Bool SAL_CALL unicode::isWhiteSpace( const sal_Unicode ch) {
    return (ch != 0xa0 && isSpace(ch)) || (ch <= 0x1F && (bit(ch) & (CONTROLSPACE)));
}

sal_Int32 SAL_CALL unicode::getCharType( const sal_Unicode ch )
{
    using namespace ::com::sun::star::i18n::KCharacterType;

    switch ( getUnicodeType( ch ) ) {
    // Upper
    case UnicodeType::UPPERCASE_LETTER :
        return UPPER|LETTER|PRINTABLE|BASE_FORM;

    // Lower
    case UnicodeType::LOWERCASE_LETTER :
        return LOWER|LETTER|PRINTABLE|BASE_FORM;

    // Title
    case UnicodeType::TITLECASE_LETTER :
        return TITLE_CASE|LETTER|PRINTABLE|BASE_FORM;

    // Letter
    case UnicodeType::MODIFIER_LETTER :
    case UnicodeType::OTHER_LETTER :
        return LETTER|PRINTABLE|BASE_FORM;

    // Digit
    case UnicodeType::DECIMAL_DIGIT_NUMBER:
    case UnicodeType::LETTER_NUMBER:
    case UnicodeType::OTHER_NUMBER:
        return DIGIT|PRINTABLE|BASE_FORM;

    // Base
    case UnicodeType::NON_SPACING_MARK:
    case UnicodeType::ENCLOSING_MARK:
    case UnicodeType::COMBINING_SPACING_MARK:
        return BASE_FORM|PRINTABLE;

    // Print
    case UnicodeType::SPACE_SEPARATOR:

    case UnicodeType::DASH_PUNCTUATION:
    case UnicodeType::INITIAL_PUNCTUATION:
    case UnicodeType::FINAL_PUNCTUATION:
    case UnicodeType::CONNECTOR_PUNCTUATION:
    case UnicodeType::OTHER_PUNCTUATION:

    case UnicodeType::MATH_SYMBOL:
    case UnicodeType::CURRENCY_SYMBOL:
    case UnicodeType::MODIFIER_SYMBOL:
    case UnicodeType::OTHER_SYMBOL:
        return PRINTABLE;

    // Control
    case UnicodeType::CONTROL:
    case UnicodeType::FORMAT:
        return CONTROL;

    case UnicodeType::LINE_SEPARATOR:
    case UnicodeType::PARAGRAPH_SEPARATOR:
        return CONTROL|PRINTABLE;

    // for all others
    default:
        return 0;
    }
}


