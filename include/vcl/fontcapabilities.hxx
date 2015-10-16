/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_FONTCAPABILITIES_HXX
#define INCLUDED_VCL_FONTCAPABILITIES_HXX

#include <boost/dynamic_bitset.hpp>
#include <vector>

#include <sal/types.h>

//See OS/2 table, i.e. http://www.microsoft.com/typography/otspec/os2.htm#ur
namespace vcl
{
    namespace UnicodeCoverage
    {
        enum UnicodeCoverageEnum
        {
            BASIC_LATIN                                 = 0,
            LATIN_1_SUPPLEMENT                          = 1,
            LATIN_EXTENDED_A                            = 2,
            LATIN_EXTENDED_B                            = 3,
            IPA_EXTENSIONS                              = 4,
            SPACING_MODIFIER_LETTERS                    = 5,
            COMBINING_DIACRITICAL_MARKS                 = 6,
            GREEK_AND_COPTIC                            = 7,
            COPTIC                                      = 8,
            CYRILLIC                                    = 9,
            ARMENIAN                                    = 10,
            HEBREW                                      = 11,
            VAI                                         = 12,
            ARABIC                                      = 13,
            NKO                                         = 14,
            DEVANAGARI                                  = 15,
            BENGALI                                     = 16,
            GURMUKHI                                    = 17,
            GUJARATI                                    = 18,
            ODIA                                        = 19,
            TAMIL                                       = 20,
            TELUGU                                      = 21,
            KANNADA                                     = 22,
            MALAYALAM                                   = 23,
            THAI                                        = 24,
            LAO                                         = 25,
            GEORGIAN                                    = 26,
            BALINESE                                    = 27,
            HANGUL_JAMO                                 = 28,
            LATIN_EXTENDED_ADDITIONAL                   = 29,
            GREEK_EXTENDED                              = 30,
            GENERAL_PUNCTUATION                         = 31,
            SUPERSCRIPTS_AND_SUBSCRIPTS                 = 32,
            CURRENCY_SYMBOLS                            = 33,
            COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS     = 34,
            LETTERLIKE_SYMBOLS                          = 35,
            NUMBER_FORMS                                = 36,
            ARROWS                                      = 37,
            MATHEMATICAL_OPERATORS                      = 38,
            MISCELLANEOUS_TECHNICAL                     = 39,
            CONTROL_PICTURES                            = 40,
            OPTICAL_CHARACTER_RECOGNITION               = 41,
            ENCLOSED_ALPHANUMERICS                      = 42,
            BOX_DRAWING                                 = 43,
            BLOCK_ELEMENTS                              = 44,
            GEOMETRIC_SHAPES                            = 45,
            MISCELLANEOUS_SYMBOLS                       = 46,
            DINGBATS                                    = 47,
            CJK_SYMBOLS_AND_PUNCTUATION                 = 48,
            HIRAGANA                                    = 49,
            KATAKANA                                    = 50,
            BOPOMOFO                                    = 51,
            HANGUL_COMPATIBILITY_JAMO                   = 52,
            PHAGS_PA                                    = 53,
            ENCLOSED_CJK_LETTERS_AND_MONTHS             = 54,
            CJK_COMPATIBILITY                           = 55,
            HANGUL_SYLLABLES                            = 56,
            NONPLANE_0                                  = 57,
            PHOENICIAN                                  = 58,
            CJK_UNIFIED_IDEOGRAPHS                      = 59,
            PRIVATE_USE_AREA_PLANE_0                    = 60,
            CJK_STROKES                                 = 61,
            ALPHABETIC_PRESENTATION_FORMS               = 62,
            ARABIC_PRESENTATION_FORMS_A                 = 63,
            COMBINING_HALF_MARKS                        = 64,
            VERTICAL_FORMS                              = 65,
            SMALL_FORM_VARIANTS                         = 66,
            ARABIC_PRESENTATION_FORMS_B                 = 67,
            HALFWIDTH_AND_FULLWIDTH_FORMS               = 68,
            SPECIALS                                    = 69,
            TIBETAN                                     = 70,
            SYRIAC                                      = 71,
            THAANA                                      = 72,
            SINHALA                                     = 73,
            MYANMAR                                     = 74,
            ETHIOPIC                                    = 75,
            CHEROKEE                                    = 76,
            UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS       = 77,
            OGHAM                                       = 78,
            RUNIC                                       = 79,
            KHMER                                       = 80,
            MONGOLIAN                                   = 81,
            BRAILLE_PATTERNS                            = 82,
            YI_SYLLABLES                                = 83,
            TAGALOG                                     = 84,
            OLD_ITALIC                                  = 85,
            GOTHIC                                      = 86,
            DESERET                                     = 87,
            BYZANTINE_MUSICAL_SYMBOLS                   = 88,
            MATHEMATICAL_ALPHANUMERIC_SYMBOLS           = 89,
            PRIVATE_USE_PLANE_15                        = 90,
            VARIATION_SELECTORS                         = 91,
            TAGS                                        = 92,
            LIMBU                                       = 93,
            TAI_LE                                      = 94,
            NEW_TAI_LUE                                 = 95,
            BUGINESE                                    = 96,
            GLAGOLITIC                                  = 97,
            TIFINAGH                                    = 98,
            YIJING_HEXAGRAM_SYMBOLS                     = 99,
            SYLOTI_NAGRI                                = 100,
            LINEAR_B_SYLLABARY                          = 101,
            ANCIENT_GREEK_NUMBERS                       = 102,
            UGARITIC                                    = 103,
            OLD_PERSIAN                                 = 104,
            SHAVIAN                                     = 105,
            OSMANYA                                     = 106,
            CYPRIOT_SYLLABARY                           = 107,
            KHAROSHTHI                                  = 108,
            TAI_XUAN_JING_SYMBOLS                       = 109,
            CUNEIFORM                                   = 110,
            COUNTING_ROD_NUMERALS                       = 111,
            SUNDANESE                                   = 112,
            LEPCHA                                      = 113,
            OL_CHIKI                                    = 114,
            SAURASHTRA                                  = 115,
            KAYAH_LI                                    = 116,
            REJANG                                      = 117,
            CHAM                                        = 118,
            ANCIENT_SYMBOLS                             = 119,
            PHAISTOS_DISC                               = 120,
            CARIAN                                      = 121,
            DOMINO_TILES                                = 122,
            RESERVED1                                   = 123,
            RESERVED2                                   = 124,
            RESERVED3                                   = 125,
            RESERVED4                                   = 126,
            RESERVED5                                   = 127,
            MAX_UC_ENUM                                 = 128
        };
    };

    namespace CodePageCoverage
    {
        enum CodePageCoverageEnum
        {
            CP1252       = 0,
            CP1250       = 1,
            CP1251       = 2,
            CP1253       = 3,
            CP1254       = 4,
            CP1255       = 5,
            CP1256       = 6,
            CP1257       = 7,
            CP1258       = 8,
            CP874        = 16,
            CP932        = 17,
            CP936        = 18,
            CP949        = 19,
            CP950        = 20,
            CP1361       = 21,
            CP869        = 48,
            CP866        = 49,
            CP865        = 50,
            CP864        = 51,
            CP863        = 52,
            CP862        = 53,
            CP861        = 54,
            CP860        = 55,
            CP857        = 56,
            CP855        = 57,
            CP852        = 58,
            CP775        = 59,
            CP737        = 60,
            CP780        = 61,
            CP850        = 62,
            CP437        = 63,
            MAX_CP_ENUM  = 64
        };
    };

    struct FontCapabilities
    {
        boost::dynamic_bitset<sal_uInt32> maUnicodeRange;
        boost::dynamic_bitset<sal_uInt32> maCodePageRange;
        std::vector< sal_uInt32 > maGSUBScriptTags;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
