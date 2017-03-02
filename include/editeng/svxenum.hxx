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

enum SvxCaseMap
{
    SVX_CASEMAP_NOT_MAPPED,
    SVX_CASEMAP_VERSALIEN,
    SVX_CASEMAP_GEMEINE,
    SVX_CASEMAP_TITEL,
    SVX_CASEMAP_KAPITAELCHEN,
    SVX_CASEMAP_END
};

enum class SvxEscapement
{
    Off,
    Superscript,
    Subscript,
    End
};

enum SvxShadowLocation
{
    SVX_SHADOW_NONE,
    SVX_SHADOW_TOPLEFT,
    SVX_SHADOW_TOPRIGHT,
    SVX_SHADOW_BOTTOMLEFT,
    SVX_SHADOW_BOTTOMRIGHT,
    SVX_SHADOW_END
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

enum SvxAdjust
{
    SVX_ADJUST_LEFT,
    SVX_ADJUST_RIGHT,
    SVX_ADJUST_BLOCK,
    SVX_ADJUST_CENTER,
    SVX_ADJUST_BLOCKLINE,
    SVX_ADJUST_END
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

enum SvxCellHorJustify
{
    SVX_HOR_JUSTIFY_STANDARD,
    SVX_HOR_JUSTIFY_LEFT,
    SVX_HOR_JUSTIFY_CENTER,
    SVX_HOR_JUSTIFY_RIGHT,
    SVX_HOR_JUSTIFY_BLOCK,
    SVX_HOR_JUSTIFY_REPEAT
};

enum class SvxCellJustifyMethod
{
    Auto,
    Distribute
};

enum SvxCellVerJustify
{
    SVX_VER_JUSTIFY_STANDARD,
    SVX_VER_JUSTIFY_TOP,
    SVX_VER_JUSTIFY_CENTER,
    SVX_VER_JUSTIFY_BOTTOM,
    SVX_VER_JUSTIFY_BLOCK
};

enum SvxCellOrientation
{
    SVX_ORIENTATION_STANDARD,
    SVX_ORIENTATION_TOPBOTTOM,
    SVX_ORIENTATION_BOTTOMTOP,
    SVX_ORIENTATION_STACKED
};

enum SvxDrawAlignEnum
{
    SVX_OBJECT_ALIGN_LEFT,
    SVX_OBJECT_ALIGN_CENTER,
    SVX_OBJECT_ALIGN_RIGHT,
    SVX_OBJECT_ALIGN_UP,
    SVX_OBJECT_ALIGN_MIDDLE,
    SVX_OBJECT_ALIGN_DOWN
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
enum SvxExtNumType
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
    SVX_NUM_HANGUL_SYLLABLE_KO    = css::style::NumberingType::HANGUL_SYLLABLE_KO,
    SVX_NUM_HANGUL_JAMO_KO        = css::style::NumberingType::HANGUL_JAMO_KO,
    SVX_NUM_NUMBER_HANGUL_KO      = css::style::NumberingType::NUMBER_HANGUL_KO,
    SVX_NUM_NUMBER_UPPER_KO       = css::style::NumberingType::NUMBER_UPPER_KO,
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
