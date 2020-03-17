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
#ifndef INCLUDED_EDITENG_SVXENUM_HXX
#define INCLUDED_EDITENG_SVXENUM_HXX

#include <com/sun/star/style/NumberingType.hpp>

enum class SvxCaseMap
{
    NotMapped,
    Uppercase,
    Lowercase,
    Capitalize,
    SmallCaps,
    End
};

enum class SvxEscapement
{
    Off,
    Superscript,
    Subscript,
    End
};

enum class SvxShadowLocation
{
    NONE,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    End
};

enum class SvxTabAdjust
{
    Left,
    Right,
    Decimal,
    Center,
    Default,
    End
};

enum class SvxLineSpaceRule
{
    Auto,
    Fix,
    Min
};

enum class SvxInterLineSpaceRule
{
    Off,
    Prop,
    Fix
};

enum class SvxAdjust
{
    Left,
    Right,
    Block,
    Center,
    BlockLine,
    End,
    LAST = End
};

enum class SvxBreak
{
    NONE,
    ColumnBefore,
    ColumnAfter,
    ColumnBoth,
    PageBefore,
    PageAfter,
    PageBoth,
    End
};

enum class SvxCellHorJustify
{
    Standard,
    Left,
    Center,
    Right,
    Block,
    Repeat
};

enum class SvxCellJustifyMethod
{
    Auto,
    Distribute
};

enum class SvxCellVerJustify
{
    Standard,
    Top,
    Center,
    Bottom,
    Block
};

enum class SvxCellOrientation
{
    Standard,
    TopBottom,
    BottomUp,
    Stacked
};

enum class SvxSpellArea
{
    Body = 0,
    BodyEnd,
    BodyStart,
    Other
};

/**
 * these must match the values in css::style::NumberingType
 */
enum SvxNumType : sal_Int16
{
    SVX_NUM_CHARS_UPPER_LETTER    = css::style::NumberingType::CHARS_UPPER_LETTER,    // Counts from a-z, aa - az, ba - bz, ...
    SVX_NUM_CHARS_LOWER_LETTER    = css::style::NumberingType::CHARS_LOWER_LETTER,
    SVX_NUM_ROMAN_UPPER           = css::style::NumberingType::ROMAN_UPPER,
    SVX_NUM_ROMAN_LOWER           = css::style::NumberingType::ROMAN_LOWER,
    SVX_NUM_ARABIC                = css::style::NumberingType::ARABIC,
    SVX_NUM_NUMBER_NONE           = css::style::NumberingType::NUMBER_NONE,
    SVX_NUM_CHAR_SPECIAL          = css::style::NumberingType::CHAR_SPECIAL,          // Bullet
    SVX_NUM_PAGEDESC              = css::style::NumberingType::PAGE_DESCRIPTOR,       // Numbering from the page template
    SVX_NUM_BITMAP                = css::style::NumberingType::BITMAP,
    SVX_NUM_CHARS_UPPER_LETTER_N  = css::style::NumberingType::CHARS_UPPER_LETTER_N,  // Counts from  a-z, aa-zz, aaa-zzz
    SVX_NUM_CHARS_LOWER_LETTER_N  = css::style::NumberingType::CHARS_LOWER_LETTER_N,
    SVX_NUM_TRANSLITERATION       = css::style::NumberingType::TRANSLITERATION,
    SVX_NUM_NATIVE_NUMBERING      = css::style::NumberingType::NATIVE_NUMBERING,
    SVX_NUM_FULL_WIDTH_ARABIC     = css::style::NumberingType::FULLWIDTH_ARABIC,
    SVX_NUM_CIRCLE_NUMBER         = css::style::NumberingType::CIRCLE_NUMBER,
    SVX_NUM_NUMBER_LOWER_ZH       = css::style::NumberingType::NUMBER_LOWER_ZH,
    SVX_NUM_NUMBER_UPPER_ZH       = css::style::NumberingType::NUMBER_UPPER_ZH,
    SVX_NUM_NUMBER_UPPER_ZH_TW    = css::style::NumberingType::NUMBER_UPPER_ZH_TW,
    SVX_NUM_TIAN_GAN_ZH           = css::style::NumberingType::TIAN_GAN_ZH,
    SVX_NUM_DI_ZI_ZH              = css::style::NumberingType::DI_ZI_ZH,
    SVX_NUM_NUMBER_TRADITIONAL_JA = css::style::NumberingType::NUMBER_TRADITIONAL_JA,
    SVX_NUM_AIU_FULLWIDTH_JA      = css::style::NumberingType::AIU_FULLWIDTH_JA,
    SVX_NUM_AIU_HALFWIDTH_JA      = css::style::NumberingType::AIU_HALFWIDTH_JA,
    SVX_NUM_IROHA_FULLWIDTH_JA    = css::style::NumberingType::IROHA_FULLWIDTH_JA,
    SVX_NUM_IROHA_HALFWIDTH_JA    = css::style::NumberingType::IROHA_HALFWIDTH_JA,
    SVX_NUM_NUMBER_UPPER_KO       = css::style::NumberingType::NUMBER_UPPER_KO,
    SVX_NUM_NUMBER_HANGUL_KO      = css::style::NumberingType::NUMBER_HANGUL_KO,
    SVX_NUM_HANGUL_JAMO_KO        = css::style::NumberingType::HANGUL_JAMO_KO,
    SVX_NUM_HANGUL_SYLLABLE_KO    = css::style::NumberingType::HANGUL_SYLLABLE_KO,
    SVX_NUM_HANGUL_CIRCLED_JAMO_KO = css::style::NumberingType::HANGUL_CIRCLED_JAMO_KO,
    SVX_NUM_HANGUL_CIRCLED_SYLLABLE_KO = css::style::NumberingType::HANGUL_CIRCLED_SYLLABLE_KO,
    SVX_NUM_CHARS_ARABIC          = css::style::NumberingType::CHARS_ARABIC,
    SVX_NUM_CHARS_THAI            = css::style::NumberingType::CHARS_THAI,
    SVX_NUM_CHARS_HEBREW          = css::style::NumberingType::CHARS_HEBREW,
    SVX_NUM_CHARS_NEPALI          = css::style::NumberingType::CHARS_NEPALI,
    SVX_NUM_CHARS_KHMER           = css::style::NumberingType::CHARS_KHMER,
    SVX_NUM_CHARS_LAO             = css::style::NumberingType::CHARS_LAO,
    SVX_NUM_CHARS_TIBETAN         = css::style::NumberingType::CHARS_TIBETAN,
    SVX_NUM_CHARS_CYRILLIC_UPPER_LETTER_BG = css::style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_BG,
    SVX_NUM_CHARS_CYRILLIC_LOWER_LETTER_BG = css::style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_BG,
    SVX_NUM_CHARS_CYRILLIC_UPPER_LETTER_N_BG = css::style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_BG,
    SVX_NUM_CHARS_CYRILLIC_LOWER_LETTER_N_BG = css::style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_BG,
    SVX_NUM_CHARS_CYRILLIC_UPPER_LETTER_RU = css::style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_RU,
    SVX_NUM_CHARS_CYRILLIC_LOWER_LETTER_RU = css::style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_RU,
    SVX_NUM_CHARS_CYRILLIC_UPPER_LETTER_N_RU = css::style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_RU,
    SVX_NUM_CHARS_CYRILLIC_LOWER_LETTER_N_RU = css::style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_RU,
    SVX_NUM_CHARS_PERSIAN         = css::style::NumberingType::CHARS_PERSIAN,
    SVX_NUM_CHARS_MYANMAR         = css::style::NumberingType::CHARS_MYANMAR,
    SVX_NUM_CHARS_CYRILLIC_UPPER_LETTER_SR = css::style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_SR,
    SVX_NUM_CHARS_CYRILLIC_LOWER_LETTER_SR = css::style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_SR,
    SVX_NUM_CHARS_CYRILLIC_UPPER_LETTER_N_SR = css::style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_SR,
    SVX_NUM_CHARS_CYRILLIC_LOWER_LETTER_N_SR = css::style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_SR,
    SVX_NUM_CHARS_GREEK_UPPER_LETTER = css::style::NumberingType::CHARS_GREEK_UPPER_LETTER,
    SVX_NUM_CHARS_GREEK_LOWER_LETTER = css::style::NumberingType::CHARS_GREEK_LOWER_LETTER,
    SVX_NUM_CHARS_ARABIC_ABJAD    = css::style::NumberingType::CHARS_ARABIC_ABJAD,
    SVX_NUM_CHARS_PERSIAN_WORD    = css::style::NumberingType::CHARS_PERSIAN_WORD,
    SVX_NUM_NUMBER_HEBREW         = css::style::NumberingType::NUMBER_HEBREW,
    SVX_NUM_NUMBER_ARABIC_INDIC   = css::style::NumberingType::NUMBER_ARABIC_INDIC,
    SVX_NUM_NUMBER_EAST_ARABIC_INDIC  = css::style::NumberingType::NUMBER_EAST_ARABIC_INDIC,
    SVX_NUM_NUMBER_INDIC_DEVANAGARI  = css::style::NumberingType::NUMBER_INDIC_DEVANAGARI,
    SVX_NUM_TEXT_NUMBER           = css::style::NumberingType::TEXT_NUMBER,
    SVX_NUM_TEXT_CARDINAL         = css::style::NumberingType::TEXT_CARDINAL,
    SVX_NUM_TEXT_ORDINAL          = css::style::NumberingType::TEXT_ORDINAL,
    SVX_NUM_SYMBOL_CHICAGO        = css::style::NumberingType::SYMBOL_CHICAGO,
    SVX_NUM_ARABIC_ZERO           = css::style::NumberingType::ARABIC_ZERO,
    SVX_NUM_ARABIC_ZERO3          = css::style::NumberingType::ARABIC_ZERO3,
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
